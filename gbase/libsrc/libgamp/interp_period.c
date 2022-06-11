/*
 * NAME
 * 
 *  interp_period ()
 *
 * FILE 
 *
 *  interp_period.c
 *
 * SYNOPSIS
 *
 *  int
 *  interp_period (beam, npts, pt_vals, max_0, max_1, samprate,
 *                 flo, fhi, ford, zp, period)
 *  float *beam =>(i) Time series.      
 *  int npts    =>(i) Number of points in time series.
 *  int pt_vals =>(i) Extremum values for beam points: PEAK|TROUGH|NIETHER.
 *                    Note:  Assumes "small" extrema have already been culled.
 *  int max_0   =>(i) Index of earliest extremum in max peak - trough.
 *  int max_1   =>(i) Index of latest extremum in max peak - trough.
 *  double  samprate=>(i) Sample rate in samples/second.
 *  double  flo =>(i) Filter low cutoff.
 *  double  fhi =>(i) Filter hi cutoff.
 *  int ford    =>(i) Filter order.
 *  int zp      =>(i) Zero-phase flag.
 *  double *per =>(o) Computed period.
 *
 * DESCRIPTION
 *
 *  Uses LaGrangian or Cosine curve interpolation to compute more accurate
 *  period.  
 *
 * DIAGNOSTICS
 *
 *  Returns OK if successful, ERR otherwise.
 *
 * ALGORITHM
 *
 *  (1) Identify "side" extrema--extrema to either side of the max_0, max_1
 *  extrema pair.
 *
 *  (2) Use Lagrangian or Cosine fit to three points centered on 
 *  extrema max_0, max_1, and previous and post extrema if they exist.
 *  Store position of extrema position estimated from curve fit.
 *
 *  (3) Use extrema positions from (2) to compute up to three half-period
 *  values.
 *
 *  (4) Check half-periods for validity against (1) Nyquist frequency,
 *      (2) processing window length, (3) band-pass corner frequencies,
 *      (4) allowable filter response correction.
 *
 *  (5) Use "reasonable" half-periods to calculate final half-period 
 *  as the average of valid half-periods.
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 *
 *  Petr Firbas, Incorporated into libamp by Ethan Brown
 *
 *  Changes made by Yan Jia:
 *  1. Corrected the unbounded searching of the preious and next extrema.
 *  2. corrected the mistakes in error messages for period ratios.
 *  3. Improved the criteria for choosing the best half-period if there are 3 
 *     valid half-periods:
 *         (a). If the ratio of the maximum half-period to the middle half-period 
 *              exceeds the max_half_period_ratio, discard the maximum.
 *         (b). If the ratio of the middle half-period to the minimum half-period 
 *              exceeds the max_half_period_ratio, discard the minimum.
 *  4. Added an additional extremum if the maximum peak-trough pair is the first
 *     or the last pair in the data window and caused only two valid half-periods
 *     for the final period estimation.
 *  5. Added 3 verbose4 level messages for testing plan.
 *  Last Modification on Feb. 04, 2005
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libstring.h"      /* for mallocWarn   */
#include "logErrorMsg.h"
#include "aesir.h"
#include "libampP.h"        /* For #defines, protos */

#define P_NUM_EXTREMA   4
#define P_NUM_H_PERIODS (P_NUM_EXTREMA - 1)
#define DB_ERRORMSG_SIZE_BIG 1024

static char * var_get(char *type);

static int apply_filt_resp_new2( 
                                int filter_order,
                                int zero_phase,
                                double  freq,
                                double  hicut,
                                double  lowcut,
                                double  samprate,
                                double  *amp, 
                                double  max_allowed_filter_correction );

static int cos_parameters( 
                          float  y_1,
                          float  y0,
                          float  y1,
                          float *shift_samples,
                          float *amp,           
                          float *half_period_samples );

static int lagrange3( float  y_1,
                      float  y0,
                      float  y1,
                      float *xmax,
                      float *ymax );

static int make_cosine_interp( 
                              int   *extrema_idxs,
                              int    n_ex,
                              float *beam,
                              int    npts,
                              float *out_extrema_vals,
                              float *out_extrema_indxs);

static int make_lagrange_interp( int   *extrema_idxs,
                                 int    n_ex,
                                 float *beam,
                                 int    npts,
                                 float *out_extrema_vals,
                                 float *out_extrema_indxs );


static int screen_half_periods( float  *half_periods,
                                int     n_hp,
                                double  window_length,
                                double  samprate,
                                double  flo,
                                double  fhi,
                                int     ford,
                                int     zp    );

static float get_mid_half_period( float *half_periods,
                                  int    n_hp         );

static float get_max_half_period( float *half_periods,
                                  int    n_hp         );

static float get_min_half_period( float *half_periods,
                                  int   n_hp           );

static int calc_best_period( float  *half_periods,
                             int     n_hp,
                             double *out_period    );

static int compare_float( const void *, const void * );


static int
compare_float( const void * fp1, const void *fp2)
{
  float   *f1 = (float *)fp1;
  float   *f2 = (float *)fp2;
    
  if( *f1 > *f2 )
	return(1);
  else if( *f1 == *f2 )
	return(0);
  else
	return(-1);
}

            
int
interp_period( float *beam, int npts, int *pt_vals, 
               int max_0, int max_1, double samprate,
               double flo, double fhi, int ford, int zp, double *period )
{
  double  maxamp;     /* peak-trough amplitude from indicies max_0,1*/
  double  thresh;     /* side extrema threshold. */
  int prev_extrem;    /* Index of extrema before max_0. */
  int next_extrem;    /* Index of extrema after max_1. */
  char    *s = (char *)NULL;
  double  side_peak_thresh = 0.25;
  int data_extrema[P_NUM_EXTREMA];
  int i;
  double  window_length;
  double  local_period = -1.0;
  int use_lagrange_interpolation = 0;
    
  float   interp_extrem_vals[P_NUM_EXTREMA];
  float   interp_extrem_indxs[P_NUM_EXTREMA];
  float   half_periods[P_NUM_H_PERIODS];
  char    msg[128];

  *period = -1.0;
    
  /* Check for bogus input */
    
  if (beam == NULL || pt_vals == NULL || npts <= 0 ) 
    {
      logErrorMsg( DFX_VERBOSE0, "interp_period: Error in input!\n" );        
      return ERR;
    }
    
  maxamp = fabs ((double)(beam[max_0] - beam[max_1]));
    
  /* Get side peak threshold from cvar parameter. */
    
  if ((s = var_get ("period-side-peak-threshold")))
	side_peak_thresh = atof (s);

  /* Find previous, next extrema */
    
  thresh = maxamp * side_peak_thresh;
  prev_extrem = -1;
  for (i = max_0 - 1; i > 0; i--)
    {
	  if (pt_vals[i] != NEITHER && fabs (beam[i] - beam[max_0])
		  > thresh)
        {
		  prev_extrem = i;
		  break;      /* Added by YJ */
        }
    }
  next_extrem = -1;
  for (i = max_1 + 1; i < npts - 1; i++)
    {
	  if (pt_vals[i] != NEITHER && fabs (beam[i] - beam[max_1])
		  > thresh)
        {
		  next_extrem = i;
		  break;      /* Added by YJ */
        }
    }

  /* Interpolate values around extrema */
  /* YJ's change for adding one more extremum if nothing found 
	 before max_0 or after max_1 */

  if (prev_extrem == -1 )
    {
	  for (i = next_extrem + 1; i < npts - 1; i++)
		{
		  if (pt_vals[i] != NEITHER && fabs (beam[i] - beam[next_extrem])
			  > thresh)
			{
			  prev_extrem = i;
			  break;         
			}
		}
	  data_extrema[0] = max_0;
	  data_extrema[1] = max_1;
	  data_extrema[2] = next_extrem;
	  data_extrema[3] = prev_extrem;
	  logErrorMsg(DFX_VERBOSE4, "One more extremum is added!\n");
    }
  else if (next_extrem == -1)
    {
	  for (i = prev_extrem - 1; i > 0; i--)
		{
		  if (pt_vals[i] != NEITHER && fabs (beam[i] - beam[prev_extrem])
			  > thresh)
			{
			  next_extrem = i;
			  break;
			}
		}
	  data_extrema[0] = next_extrem;
	  data_extrema[1] = prev_extrem;
	  data_extrema[2] = max_0;
	  data_extrema[3] = max_1;
	  logErrorMsg(DFX_VERBOSE4, "One more extremum is added!\n");
    }
  else
    {
	  data_extrema[0] = prev_extrem;
	  data_extrema[1] = max_0;
	  data_extrema[2] = max_1;
	  data_extrema[3] = next_extrem;
    }
  /* End of YJ's change */

  /* Get interpolation-type flag. */
    
  if ((s = var_get ("period-use-lagrange-interpolation")))
	use_lagrange_interpolation = atoi (s);
    
  if (use_lagrange_interpolation)
    {
	  if (make_lagrange_interp(&(data_extrema[0]), P_NUM_EXTREMA,
							   beam, npts,
							   &(interp_extrem_vals[0]),
							   &(interp_extrem_indxs[0]))
		  == ERR)
        {
		  logErrorMsg(DFX_VERBOSE0, "interp_period: Error in lagrange interp\n" );      
		  return ERR;
        }
    }
  else if (make_cosine_interp(&(data_extrema[0]),
							  P_NUM_EXTREMA,
							  beam, npts,
							  &(interp_extrem_vals[0]),
							  &(interp_extrem_indxs[0]))
		   == ERR)
    {
	  logErrorMsg(DFX_VERBOSE0, "interp_period: Error in cosine interp\n" );      
	  return ERR;
    }

            

  /* Calculate half periods updated for all extrema */

  for (i = 0; i < P_NUM_H_PERIODS; i++)
    {

	  /* If either are null, set difference to null. */
        
	  if (interp_extrem_indxs[i] < 0.0 ||
		  interp_extrem_indxs[i + 1] < 0.0)

		half_periods[i] = -1.0;
	  else
		half_periods[i] =
		  (interp_extrem_indxs[i + 1] -
		   interp_extrem_indxs[i]) / samprate;
    }
    

  /* Screen half-periods using various criteria. */

  window_length = (float)npts / samprate;
  if (screen_half_periods(half_periods,
						  P_NUM_H_PERIODS, window_length,
						  samprate, flo, fhi, ford, zp) == ERR)
    {
	  logErrorMsg(DFX_VERBOSE0, "interp_period: Error screening half_periods.\n" );      
	  return ERR;
    }
                

  /* Calculate best half period (avg) from screened half periods */

  qsort((void *)half_periods, P_NUM_H_PERIODS, sizeof(float),
		compare_float);   

  if (calc_best_period(half_periods, P_NUM_H_PERIODS, &local_period)
	  == ERR)
    {
      snprintf( msg, sizeof(msg),  "interp_period: Error calculating period.\n" );
	  logErrorMsg(DFX_VERBOSE0, msg);      
	  return ERR;
    }


  *period = local_period;
    
  return (OK);
}


/*
 * NAME
 * 
 *  calc_best_period
 *
 * FILE 
 *
 *  interp_period.c
 *
 * SYNOPSIS
 *
 *  int
 *  calc_best_period (half_periods, n_hp, out_period)
 *  float   *half_periods   =>(i) Array of sorted half period values.
 *  int n_hp        =>(i) Number of half periods.
 *  double  *local_period   =>(o) calculated period.
 *
 * DESCRIPTION
 *
 *  Calculates final period from valid half periods.
 *
 *  (1) If there are no valid periods, return error.
 *
 *  (2) If there is one valid half-period, and the minimum number
 *    threshold is <= 1, return that period.
 *
 *  (3) If there are two valid half periods check the ratio of
 *    max/min.  If the ratio exceeds a threshold, select the
 *    minimum of the two as the period.  Otherwise take the average.
 *
 *  (4) If there are three valid half periods, (a) compare the minimum
 *    period with the middle period.  If this value exceeds a threshold
 *    throw out the minimum.  Then check the ratio of the middle to the
 *    maximum period.  If this ratio exceeds the threshold, use the
 *    middle value as the period.  Otherwise average the valid periods.
 *  
 *
 * DIAGNOSTICS
 *
 *  Returns OK if successful, ERR otherwise.
 *
 *
 * AUTHOR
 *
 *  Petr Firbas, Incorporated into libamp by Ethan Brown
 */

static int
calc_best_period( float *half_periods, int n_hp, double *out_period )
{
  float   max_half_period, mid_half_period, min_half_period;
  int     num_valid_half_periods = 0;
  int     min_valid_half_periods = 1;
  double  max_half_period_ratio = 2.0;
  char    *s = (char *)NULL;
  int i;
  char    msg[128];
    

  /* Get threshold parameters, if set. */
    
  if ((s = var_get ("period-min-half-periods")))
	min_valid_half_periods = atoi (s);

  if ((s = var_get ("period-max-half-period-ratio")))
	max_half_period_ratio = atof (s);

  for (i = 0; i < n_hp; i++)
	if (half_periods[i] >= 0.0)
	  num_valid_half_periods++;

  if (num_valid_half_periods == 0)
    {
	  logErrorMsg(DFX_VERBOSE0,
				  "calc_best_period: Error: No valid half_periods found!\n" );
	  return ERR;
    }
  else if (num_valid_half_periods < min_valid_half_periods)
    {
	  snprintf( msg, sizeof( msg ), 
				"calc_best_period: Error: only %d valid periods. Below threshold of %d\n",
				num_valid_half_periods,
				min_valid_half_periods );
	  logErrorMsg(DFX_VERBOSE0, msg );
	  return ERR;
    }

  switch (num_valid_half_periods)
    {
    case(1):
	  *out_period = 2.0 * get_max_half_period(half_periods, n_hp);
	  return (OK);

    case(2):
	  min_half_period = get_min_half_period(half_periods, n_hp);
	  max_half_period = get_max_half_period(half_periods, n_hp);

	  /* Check threshold. */
        
	  if (max_half_period / min_half_period > max_half_period_ratio)
        {
		  /* Use min value for period. */
            
		  snprintf( msg, sizeof( msg ),
					"calc_best_period: Ratio of max and min half periods %.3f exceeds threshold of %.3f\n",
					max_half_period / min_half_period,
					max_half_period_ratio );
		  logErrorMsg(DFX_VERBOSE1, msg );
		  *out_period = 2.0 * min_half_period;
		  return(OK);
        }
	  else
        {
		  /* Use average of min and max */

		  *out_period = min_half_period + max_half_period;
		  return(OK);
        }
        
    case(3):
	  min_half_period = get_min_half_period(half_periods, n_hp);
	  max_half_period = get_max_half_period(half_periods, n_hp);
	  mid_half_period = get_mid_half_period(half_periods, n_hp);

	  /* Check min and mid.If min too small move on with mid & max.*/

	  if (mid_half_period / min_half_period  > max_half_period_ratio)
        {
		  snprintf( msg, sizeof(msg),
					"calc_best_period: Ratio of mid and min half periods %.3f exceeds threshold of %.3f. Discarding min period.\n",
					mid_half_period / min_half_period,
					max_half_period_ratio); /* Error message corrected by YJ */
		  logErrorMsg(DFX_VERBOSE1, msg );
		  /* Check mid and max */

		  if (max_half_period / mid_half_period
			  >  max_half_period_ratio)
            {
			  /* Use mid value only as period. */

			  snprintf( msg, sizeof( msg ),
						"calc_best_period: Ratio of max and mid half periods %.3f exceeds threshold of %.3f. "
						"Using mid period as final period.\n",
						max_half_period / mid_half_period,
						max_half_period_ratio);
			  logErrorMsg(DFX_VERBOSE1, msg );
			  *out_period = mid_half_period * 2.0;
			  return(OK);
            }
		  else
            {
			  /* mid AND max are OK.  Use average. */

			  *out_period =
				mid_half_period + max_half_period;
			  return(OK);
            }
        }
        
	  /*
	   *  Min and mid are OK.  Now check min and max.
	   *  If max too big, use min and mid.
	   */
    
	  /* YJ's change started here */  
	  else if (max_half_period / mid_half_period
			   >  max_half_period_ratio)
        {
		  snprintf( msg, sizeof( msg ),
					"calc_best_period: Ratio of max and mid half periods %.3f exceeds threshold of %.3f. Discarding the max period.\n",
					max_half_period / mid_half_period, max_half_period_ratio);
		  logErrorMsg(DFX_VERBOSE1, msg );
		  *out_period = min_half_period + mid_half_period;
		  logErrorMsg(DFX_VERBOSE4, "Used the ratio of max and mid half periods to discard the extremely large max period.\n");
		  return(OK);
        } 
	  /* End of YJ's change */
        
	  /* All three values are OK.  Average them. */
        
	  else
        {
		  *out_period = 2.0 * (min_half_period +
							   mid_half_period +
							   max_half_period) / 3.0;
            
		  return(OK);
        }
    default:
        
	  snprintf( msg, sizeof(msg),
				"calc_best_period: Error: Strange number of half_periods: %d!\n",
				num_valid_half_periods);
	  logErrorMsg(DFX_VERBOSE0, msg );
	  return ERR;
    }
}


static float
get_min_half_period( float *half_periods, int n_hp)
{
  int i;
  float  min_per = 99999999.9;

  for(i = 0; i < n_hp; i++)
	if (half_periods[i] >= 0.0
		&& half_periods[i] < min_per)
	  min_per = half_periods[i];
    
  if (min_per == 99999999.9)
    {
	  logErrorMsg(DFX_VERBOSE0,"get_min_half_period: Error: No valid half periods for min find!\n");
	  return (-1.0);
    }
  return (min_per);
}

static float
get_max_half_period( float *half_periods, int n_hp)
{
  int i;
  float max_per = -1.0;

  for(i = 0; i < n_hp; i++)
	if (half_periods[i] >= 0.0
		&& half_periods[i] > max_per)
	  max_per = half_periods[i];
    
  if (max_per < 0.0)
    {
	  logErrorMsg(DFX_VERBOSE0, "get_max_half_period: Error: No valid half periods for max find!\n" );
	  return (-1.0);
    }
    
  return (max_per);
}

/* Assumes sorted ascending. */
static float
get_mid_half_period( float *half_periods, int n_hp)
{
  int i;
        
  for(i = 0; i < n_hp; i++)
	if (half_periods[i] < 0.0)
	  {
		logErrorMsg(DFX_VERBOSE0, "get_mid_half_period: Error: Null half period.  No mid found!\n" );
		return (-1.0);
	  }

  return (half_periods[n_hp - 2]);
}


/*
 * NAME
 * 
 *  screen_half_periods
 *
 * FILE 
 *
 *  interp_period.c
 *
 * SYNOPSIS
 *
 *  int
 *  screen_half_periods (half_periods, n_hp, window_length, samprate,
 *          flo, fhi, ford, zp)
 *  float   *half_periods   =>(i) Array of half period values.
 *  int n_hp        =>(i) Number of half periods.
 *  double  window_length   =>(i) Length of data window in seconds.
 *  double  samprate    =>(i) Sample rate.
 *  double  flo     =>(i) Filter low cutoff.
 *  double  fhi     =>(i) Filter hi cutoff.
 *  int ford        =>(i) Filter order.
 *  int zp      =>(i) Zero-phase flag.
 *
 * DESCRIPTION
 *
 *  Screens half periods by checking against Nyquist frequency,
 *  window length, distance from corner frequencies, and filter
 *  response correction.
 *  
 *
 * DIAGNOSTICS
 *
 *  Returns OK if successful, ERR otherwise.
 *
 *
 * AUTHOR
 *
 *  Petr Firbas, Incorporated into libamp by Ethan Brown
 */

static int
screen_half_periods( float *half_periods, int n_hp, 
					 double window_length, double samprate,
					 double flo, double fhi, int ford, int zp )
{
  int i;
  double  max_nyquist_percentage = 0.8;
  double  max_window_length_percentage = .25;
  double  max_hi_cut_percentage = 2.0;
  double  min_lo_cut_percentage = 0.833;
  double  max_filter_correction = 100.0;
  double  expected_correction;
  char    *s = NULL;
  float   nyquist =  samprate / 2.0;
  char    msg[128];
    
  /* Get nyquist-percentage threshold from cvar parameter. */
    
  if ((s = var_get ("period-max-nyquist-percentage")))
	max_nyquist_percentage = atof (s);

  /* Get window-percentage threshold from cvar parameter. */
    
  if ((s = var_get ("period-window-length-percentage")))
	max_window_length_percentage = atof (s);

  /* Get corner-frequency thresholds from cvar parameter. */
    
  if ((s = var_get ("period-max-hi-cut-percentage")))
	max_hi_cut_percentage = atof (s);

  if ((s = var_get ("period-min-lo-cut-percentage")))
	min_lo_cut_percentage = atof (s);

  /* Get maximum filter correction from cvar parameter. */
    
  if ((s = var_get ("period-max-filter-correction")))
	max_filter_correction = atof (s);

  for(i = 0; i < n_hp; i++)
    {
	  float   this_period    = 2.0 * half_periods[i];
	  float   this_frequency =
		1.0 / ((this_period == 0.0) ? .0000001 : this_period);
            
	  /*  Skip of this half period is already null. */

	  if (half_periods[i] < 0.0)
		continue;

	  /* Nyquist test */

	  if (this_frequency >= nyquist * max_nyquist_percentage)
        {
		  snprintf( msg, sizeof(msg),
					"screen_half_periods: Info: frequency %.2f exceeds the %.2f nyquist threshold.\n",
					this_frequency, nyquist * max_nyquist_percentage );
		  logErrorMsg(DFX_VERBOSE1, msg );

		  /* Break to next half_period */

		  half_periods[i] = -1.0;
		  continue;
        }

	  /* Window length check */
        
	  if (this_period >=
		  window_length * max_window_length_percentage)
        {
		  snprintf( msg, sizeof(msg),
					"screen_half_periods: Info: period %.2f exceeds the %.2f second window length percentage threshold.\n",
					this_period, window_length * max_window_length_percentage );
		  logErrorMsg( DFX_VERBOSE1, msg );

		  /* Break to next half_period */

		  half_periods[i] = -1.0;
		  continue;
        }

	  /* Corner frequency check. */

	  if ((this_frequency >= fhi * max_hi_cut_percentage)
		  || (this_frequency <= flo * min_lo_cut_percentage))
        {
		  snprintf( msg, sizeof(msg),
					"screen_half_periods: Info: frequency %.2f out of filter band range %.3f - %.3f.\n",
					this_frequency, flo * min_lo_cut_percentage, fhi * max_hi_cut_percentage );
		  logErrorMsg( DFX_VERBOSE1, msg );

		  /* Break to next half_period */

		  half_periods[i] = -1.0;
		  continue;
        }

	  /* Filter response correction check. */

	  expected_correction = 1.0;
	  if (apply_filt_resp_new2(ford, zp,
							   this_frequency,
							   fhi, flo,
							   samprate,
							   &expected_correction,
							   max_filter_correction)
		  == ERR)
        {
		  snprintf( msg, sizeof(msg),
					"screen_half_periods: Error: in apply_filt_resp_new2 for frequency %.3f.\n",
					this_frequency );
		  logErrorMsg(DFX_VERBOSE0, msg );
		  return ERR;
        }
	  else if (expected_correction >= max_filter_correction)
        {
		  snprintf( msg, sizeof(msg),
					"screen_half_periods: frequency %.3f causes filter correction to exceed maximum value of %.3f .\n",
					 this_frequency, max_filter_correction );
		  logErrorMsg(DFX_VERBOSE1, msg );
            
		  half_periods[i] = -1.0;
		  continue;
        }
    }

  return (OK);

}


/*
 * NAME
 * 
 *  make_lagrange_interp
 *
 * FILE 
 *
 *  interp_period.c
 *
 * SYNOPSIS
 *
 *  int
 *  make_lagrange_interp (extrema_idxs, n_ex, beam, npts, out_extrem_vals,
 *              out_extrema_indxs)
 *  int *extrema_idxs   =>(i) Indexes of beam extrema.
 *  int n_ex        =>(i) Number of extrema.
 *  float   *beam       =>(i) Time series.      
 *  int npts        =>(i) Number of points in time series.
 *  float   *out_extrema_vals=>(o) Values of interpolated extrema.
 *  float   *out_extrema_indxs=>(o) Indexes of interpolated extrema.
 *
 * DESCRIPTION
 *
 *  Uses LaGrangian curve interpolation to compute more accurate
 *  period.  
 *
 * DIAGNOSTICS
 *
 *  Returns OK if successful, ERR otherwise.
 *
 * AUTHOR
 *
 *  Petr Firbas, Incorporated into libamp by Ethan Brown
 */

static int
make_lagrange_interp( int *extrema_idxs, int n_ex, float *beam, int npts,
					  float *out_extrema_vals, float *out_extrema_indxs  )
{
  int i, this_extrema;
  float   f_extrema;
  float   f_amp;
  char    msg[128];

  /* Make lagrange interpolation for each extrema */
    
  for(i = 0; i < n_ex; i++)
    {
	  /* Don't process "null" extrema. */
        
	  if (extrema_idxs[i] < 0)
        {
		  out_extrema_vals[i]  = -1.0;
		  out_extrema_indxs[i] = -1.0;
		  continue;
        }

	  f_extrema = this_extrema = extrema_idxs[i];

	  /* This should never happen, but... */
        
	  if (this_extrema >= npts)
        {
		  snprintf( msg, sizeof(msg),
					"make_lagrange_interp: Extrema index %d out of range. Max is %d\n",
					 this_extrema, npts - 1);     
		  logErrorMsg( DFX_VERBOSE0, msg );
		  return ERR;
        }
        
	  f_amp = beam[this_extrema];
        
	  /*
	   *  Make 3 point lagrange interpretation.  (Fatal error if center
	   *  point not extrema.)
	   */

	  if (lagrange3(beam[this_extrema - 1],
					beam[this_extrema],
					beam[this_extrema + 1],
					&f_extrema, &f_amp)
		  == ERR)
        {
		  logErrorMsg( DFX_VERBOSE0, "make_lagrange_interp: Error in lagrange interpolation!\n" );
		  return ERR;
        }
        
	  out_extrema_vals[i] = f_amp;
	  out_extrema_indxs[i] = f_extrema;
    }
    
  return(OK);
}


/*
 * NAME
 * 
 *  make_cosine_interp
 *
 * FILE 
 *
 *  interp_period.c
 *
 * SYNOPSIS
 *
 *  int
 *  make_cosine_interp (extrema_idxs, n_ex, beam, npts, out_extrema_vals,
 *              out_extrema_indxs)
 *  int *extrema_idxs   =>(i) Indexes of beam extrema.
 *  int n_ex        =>(i) Number of extrema.
 *  float   *beam       =>(i) Time series.      
 *  int npts        =>(i) Number of points in time series.
 *  float   *out_extrema_vals=>(o) Values of interpolated extrema.
 *  float   *out_extrema_indxs=>(o) Indexes of interpolated extrema.
 *
 * DESCRIPTION
 *
 *  Uses LaGrangian curve interpolation to compute more accurate
 *  period.  
 *
 * DIAGNOSTICS
 *
 *  Returns OK if successful, ERR otherwise.
 *
 * AUTHOR
 *
 *  Petr Firbas, Incorporated into libamp by Ethan Brown
 */

static int
make_cosine_interp( int *extrema_idxs, int n_ex, float *beam, int npts,
				    float *out_extrema_vals, float *out_extrema_indxs  )
{
  int i, this_extrema;
  float   f_extrema;
  float   cos_maxamp, cos_shift, cos_half_period;
  char    msg[128];

  /* Make cosine interpolation for each extrema */
    
  for(i = 0; i < n_ex; i++)
    {
	  /* Don't process "null" extrema. */
        
	  if (extrema_idxs[i] < 0)
        {
		  out_extrema_vals[i]  = -1.0;
		  out_extrema_indxs[i] = -1.0;
		  continue;
        }

	  f_extrema = this_extrema = extrema_idxs[i];

	  /* This should never happen, but... */
        
	  if (this_extrema >= npts)
        {
		  snprintf( msg, sizeof(msg),
					"make_cosine_interp: Extrema index %d out of range. Max is %d\n",
					 this_extrema, npts - 1);     
		  logErrorMsg( DFX_VERBOSE0, msg );
		  return ERR;
        }
        
	  cos_maxamp      = beam[this_extrema];
	  cos_shift       = 0.0;
	  cos_half_period = -1.0;
        
	  /*
	   *  Make 3 point cosine interpretation.  (Fatal error if center
	   *  point not extrema.)
	   */

	  if (cos_parameters(beam[this_extrema - 1],
						 beam[this_extrema],
						 beam[this_extrema + 1],
						 &cos_shift, &cos_maxamp, &cos_half_period)
		  == ERR)
        {
		  logErrorMsg(DFX_VERBOSE0, "make_cosine_interp: Error in cosine interpolation!\n" );
		  return ERR;
        }
        
	  out_extrema_vals[i] = cos_maxamp;
	  out_extrema_indxs[i] = f_extrema + cos_shift;
    }
    
  return(OK);
}

/*
 *
 * NAME
 *  lagrange3
 * 
 * FILE 
 *  amp.c
 *
 * SYNOPSIS
 *      int lagrange3( y_1, y0 , y1 , *xmax , *ymax )
 *
 *      float   y_1         (i)     value for point at -1
 *      float   y0          (i)     value for point at 0
 *      float   y1          (i)     value for point at 1
 *      float   *xmax       (i/o)   updated position of maximum found
 *      float   *ymax       (o)     value at maximum found
 *
 * DESCRIPTION
 *  Compute better aproximation of the maxima/minima time/position
 *  from three points. Use robust Lagrange approach. Time coordinates
 *  are considered one sample interval apart (i.e. one unit).
 *  Compute also the value at maximum/minimum
 *  found. This subroutine should be used preferably for PEAKS/TROUGHS
 *  detected at the central point.
 *  
 *
 * DIAGNOSTICS
 *
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 *
 *  Petr Firbas     Feb 1995    test version
 *
 ***************************************************************************
 *
 */


static int
lagrange3( 
		  float     y_1,
		  float     y0,
		  float     y1,
		  float *xmax,
		  float *ymax )
{

  float   shift;      /* shift of maximum in sample intervals */
  char    msg[128];

  shift = 0.0;
  *ymax = y0;

  /*
	With current PEAK and TROUGH definition this should not ever happen
  */ 
  if ( ( y_1 - 2 * y0 + y1 ) == 0.0 )
    {
	  logErrorMsg( LOG_WARNING, "lagrange3: PEAK or TROUGH? Collinear samples detected." );
	  return ERR; 
    }

  /*
	Else determine PEAK/TROUGH position
  */
  else
    {
	  shift = ( y_1 - y1 ) / ( y_1 - 2 * y0 + y1 ) / 2.0;
    }

  /*
	Test again: Is the "shift" is in bounds - 
	i.e. less than 1 sample interval?
  */
  if ( fabs( shift ) >= 1.0 ) 
    {
	  snprintf( msg, sizeof(msg), "lagrange3: Values: %.2g %.2g %.2g do not define PEAK or TROUGH.",
			   y_1,y0,y1 );
	  logErrorMsg( LOG_WARNING, msg );

	  *ymax = y0;

	  return ERR; 
    }

  /*
	Compute the approximation of the PEAK/TROUGH value 
  */
  *ymax = 
	( shift * ( shift - 1.0 ) ) * y_1 / 2.0 -
	( ( shift + 1.0 ) * ( shift - 1.0 ) ) * y0 +
	( shift * ( shift + 1.0 ) ) * y1 / 2.0;

  /*
	calculate maximum/minimum position
  */
  *xmax = *xmax + shift;


  return OK;
}

/*
 *
 * NAME
 *  cos_parameters
 * 
 * FILE 
 *  amp.c
 *
 * SYNOPSIS
 *      int cos_parameters( y_1, y0, y1, *shift_samples, *amp, *half_period_samples ) 
 *
 *      float   y_1                 (i)     value for point at -1
 *      float   y0                  (i)     value for point at 0
 *      float   y1                  (i)     value for point at 1
 *      float   *shift_samples      (o)     position of maximum found 
 *                                          (with fraction of sample interval)
 *      float   *amp                (o)     value at maximum found
 *      float   *half_period_samples(o)     half period found 
 *                                          (with fraction of sample interval)
 *
 * DESCRIPTION
 *  Compute better aproximation of the maxima/minima time/position
 *  from three points. Fit a cosine curve. Time coordinates
 *  are considered one sample interval apart (i.e. one unit).
 *  Compute also the value at maximum/minimum
 *  found. This subroutine should be used preferably for PEAKS/TROUGHS
 *  detected at the central point.
 *  
 *
 * DIAGNOSTICS
 *
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 *
 *  Petr Firbas     March 1995  test version
 *
 ***************************************************************************
 *
 */


static int
cos_parameters( float    y_1,
			    float    y0,
			    float    y1,
			    float    *shift_samples,
			    float    *amp,           
			    float    *half_period_samples )

{

  float   b, c;
  double  temp;
  int     peak = 1;

  /*
	With current PEAK and TROUGH definition this should not ever happen
  */ 
  if ( ( y_1 - 2 * y0 + y1 ) == 0.0 )
    {
	  logErrorMsg(LOG_WARNING, "cos_parameters: PEAK or TROUGH? Collinear samples detected." ); 
	  return ERR; 
    }

  /* 
	 For trough use input values * -1.0 and set variable "peak"
  */
  if ( y_1 > y0 )
    {
	  peak = 0;
    }
  else if( y_1 == y0 )
    {
	  if( y1 > y0)
        {
		  peak = 0;
        }
    }

  if( peak == 0 )
    {
	  y_1 = -1.0 * y_1;
	  y0  = -1.0 * y0;
	  y1  = -1.0 * y1;
    }

  /*
	compute the half period in samples
  */
  temp = ( (double)y1 + (double)y_1 ) / 2.0 / (double)y0;
  if ( fabs(temp) >= 1.0 )
    {

	  return ERR; 
    }
  b = acos( temp );
  *half_period_samples = M_PI / b;
    
  /*
	compute time shift of peak/trough as a fraction of sample interval
  */
  c = atan( 1.0 / tan ( b ) * ( y1 - y_1 ) / ( y1 + y_1 ) );
  *shift_samples = fabs( *half_period_samples / M_PI * c );

  /* 
	 determine the sign
  */
  if (y_1 > y1 ) 
    {
	  *shift_samples = -1.0 * *shift_samples;
    }

  /*
	determine the amplitude
  */
  *amp = fabs ( y0 / cos( c ) );
  if ( peak == 0 ) *amp = -1.0 * *amp;

  return OK;  

}


/*
 *
 * NAME
 *  apply_filt_resp_new2
 * 
 * FILE 
 *  amp.c
 *
 * SYNOPSIS
 *      int apply_filt_resp_new2 ( filter_order, zero_phase, freq, hicut, lowcut,
 *      samprate, amp, max_allowed_filter_correction )
 *
 *      int     filter_order    (i)     filter order
 *      int     zero_phase      (i)     true for non causal
 *      double  freq            (i)     frequency in cycles/sec
 *      double  hicut           (i)     band cut off frequency in cycles/sec
 *      double  lowcut          (i)     band cut off frequency in cycles/sec
 *      double  samprate        (i)     sampling frequncy in cycles/sec 
 *      double  *amp            (i/o)   adjusted for filter response
 *      double  max_allowed_filter_correction 
 *                              (i)     Maximum allowed filter correction 
 *
 *
 *
 * DESCRIPTION
 *
 *      Applies the butterworth filter response corresponding to the input
 *      frequency and band cut off frequency(ies).
 *      It does not allow the correction to grow over a specified limit.
 *
 *      Used in the libampmeas library of the DAP program
 *      for body wave amplitude measurements.
 *
 *
 * DIAGNOSTICS
 * 
 *      Warning message for no filter requested.
 *
 *      
 * FILES
 *
 *
 * NOTES
 *
 * 
 * SEE ALSO
 *  
 *
 * AUTHOR
 *
 *  Petr Firbas     Feb  1995   test version
 *                  Jan  1996   samplerate changed to double
 *
 ***************************************************************************
 *
 */


/* these defines should be put out of this function */
#define MAX_FILTER_ORDER 10 /* introduce this limit to avoid erroneous input */


static int
apply_filt_resp_new2( int filter_order, int zero_phase, double freq, double hicut, double lowcut, 
					  double samprate, double *amp, double max_allowed_filter_correction          )

	 /* int     filter_order -  filter order */
	 /* int     zero_phase - flag for zero phase filter */
	 /* double  freq;        signal frequency */
	 /* double  hicut;       high cut frequency of the filter */
	 /* double  lowcut;      low cut frequency of the filter */
	 /* double  samprate;    signal sample rate */
	 /* double  *amp;        adjusted for instrument response */
	 /* double  max_allowed_filter_correction;  
	  * This limit should avoid filter corrections (to be applied later)
	  * which are excessively large. In these cases mostly either period estimation
	  * is in error or the period is too far outside of the filter band. Mostly there
	  * could not be enough energy in signal to allow for as high correction 
   	  */

{

  double  sample_int; 		/* sample interval */
  double  butterworth = 1.0; 	/* filter correction */
  double  freq_a;     		/* signal frequency transformed to analog domain */
  double  hicut_a;    		/* hicut frequency transformed to analog domain */
  double  lowcut_a;   		/* lowcut frequency transformed to analog domain */
  double  omega_norm_LP; 	/* frequency normalized to standard LP */
  char    msg[128];
    
  /*
   *  Test on input frequency ... this test may go somewhere up in
   *  the program
   */
  if ( samprate <= 0.0 ) 
    {
	  snprintf( msg, sizeof(msg), "apply_filt_resp_new2: Wrong signal sampling rate: %f.", samprate );	  
	  logErrorMsg( LOG_WARNING, msg ); 

	  return ERR; 
    }

  /*
   *  Test on input frequency ... this test may go somewhere
   *  up in the program
   */
  if ( freq <= 0.0 || freq >= samprate / 2.0 ) 
    {
	  snprintf( msg, sizeof(msg), "apply_filt_resp_new2: Wrong signal frequency: %f.", freq );
	  logErrorMsg( LOG_WARNING, msg );

	  return ERR; 
    }
        

  /*
	Test on filter order ... this test may go somewhere up in the program
  */
  if ( 
	  filter_order > MAX_FILTER_ORDER || 
	  ( filter_order < 1 && ( lowcut > 0.0 || hicut > 0.0 ) ) ||
	  ( filter_order < 0 && ( lowcut > 0.0 && hicut > 0.0 ) ) )
    {
	  snprintf( msg, sizeof(msg),
			   "apply_filt_resp_new2: Filter order: %d out of limits. Max order allowed is %d", filter_order, MAX_FILTER_ORDER );
	  logErrorMsg( LOG_WARNING, msg );

	  return ERR; 
    }



  /*
	Use sample interval
  */
  sample_int = 1.0 / samprate;

    
  /*
	For a bandpass filter
  */    
  if ( ( lowcut > 0.0 ) && ( hicut > 0.0 ) )
    {
	  /* 
		 Convert the "digital frequencies to "analog" ones 
	  */
	  hicut_a = tan ( (double)( M_PI * hicut * sample_int ) ) / 
		( M_PI * sample_int );
	  lowcut_a = tan ( (double)( M_PI * lowcut * sample_int ) ) / 
		( M_PI * sample_int );
	  freq_a = tan ( (double)( M_PI * freq * sample_int ) ) / 
		( M_PI * sample_int );

	  /* 
		 Convert the signal "analog" frequency to that 
		 of standard (normalized) LP filter 
	  */ 
	  omega_norm_LP = ( freq_a * freq_a - hicut_a * lowcut_a ) / 
		freq_a / ( hicut_a - lowcut_a );

	  /* 
		 Compute the amplitude adjustment factor 
	  */
	  butterworth = sqrt( fabs( 1.0 / 
								( 1.0 + (double)pow( omega_norm_LP, (double)( 2.0 * filter_order )))));
    }

  /*
	For a highpass filter
  */    
  else if ( ( lowcut > 0.0 ) && ( hicut <= 0.0 ) )
    {
	  /* 
		 Convert the "digital frequencies to "analog" ones 
	  */
	  lowcut_a = tan ( (double)( M_PI * lowcut * sample_int ) ) / 
		( M_PI * sample_int );
	  freq_a = tan ( (double)( M_PI * freq * sample_int ) ) / 
		( M_PI * sample_int );

	  /* 
		 Convert the signal frequency to that of standard LP filter 
	  */ 
	  omega_norm_LP = -1.0 * lowcut_a / freq_a;

	  /* 
		 Compute the amplitude adjustment factor 
	  */
	  butterworth = sqrt( fabs( 1.0 / 
								( 1.0 + (double)pow( omega_norm_LP, (double)( 2.0 * filter_order )))));
    }


  /*
	For a lowpass filter
  */    
  else if ( ( lowcut <= 0.0 ) && ( hicut > 0.0 ) )
    {
	  /* 
		 Convert the "digital" frequencies to "analog" ones 
	  */
	  hicut_a = tan ( (double)( M_PI * hicut * sample_int ) ) / 
		( M_PI * sample_int );
	  freq_a = tan ( (double)( M_PI * freq * sample_int ) ) / 
		( M_PI * sample_int );

	  /* 
		 Convert the signal frequency to that of standard LP filter 
	  */ 
	  omega_norm_LP = freq_a / hicut_a;

	  /* 
		 Compute the amplitude adjustment factor 
	  */
	  butterworth =
		sqrt( fabs( 1.0 / 
					( 1.0 +
					  (double)pow(omega_norm_LP,
								  (double)(2.0 * filter_order)))));
    }

  /*
	Test on "no filter requested" 
	- not considered error here, but warning is issued
  */

  else
	/*
	  i.e. if ( ( lowcut <= 0.0 ) && ( hicut <= 0.0 ) )
	*/
    {
	  butterworth = 1.0;
        
	  logErrorMsg( LOG_WARNING, "apply_filt_resp_new2: No filter correction requested." );
    }
    
  /*
	For zero phase filter use the square of the correction coefficient
  */
  if ( zero_phase )
    {
	  butterworth = butterworth * butterworth;
    }

  if ( 1.0 / butterworth > max_allowed_filter_correction )
    {
	  return ERR;
    }

  /* 
	 Apply amplitude adjustment 
  */
  *amp = *amp / butterworth;
        
  return OK;
    
}

/*

  if ((s = var_get ("period-side-peak-threshold")))
  if ((s = var_get ("period-use-lagrange-interpolation")))
  if ((s = var_get ("period-min-half-periods")))
  if ((s = var_get ("period-max-half-period-ratio")))
  if ((s = var_get ("period-max-nyquist-percentage")))
  if ((s = var_get ("period-window-length-percentage")))
  if ((s = var_get ("period-max-hi-cut-percentage")))
  if ((s = var_get ("period-min-lo-cut-percentage")))
  if ((s = var_get ("period-max-filter-correction")))

from Changes.DFX:
Parameters related to this modification:
amp-use-interp-period={0|1}
        is the switch to enable the period estimator.  Default = 0 (off).

period-side-peak-threshold=<float>
        is the minimum fraction of the maximum peak-to-trough amplitude for
        extrema preceding and following the maximum to be included for
        the half_period estimation.  Default = 0.25.

period-use-lagrange-interpolation={0|1}
        is the switch to use lagrange or cosine curve fitting.
        Default = 0 (use cosine fit).

period-min-half-periods=<integer>
        is the minimum allowable number of valid half-periods.
        Default = 1.

period-max-half-period-ratio=<float>
        is the maximum ratio between half-periods (see (4) above).
        Default = 2.0.

period-max-nyquist-percentage=<float>
        is the maximum fraction of the nyquist frequency allowable for
        the frequency-equivalent half-period (see (3) above).
        Default = 0.8.

period-window-length-percentage=<float>
        is the maximum fraction of the window length allowable for
        the period-equivalent half-period (see (3) above).
        Default = 0.25.

period-max-hi-cut-percentage=<float>
        is the maximum fraction of the filter pass-band high cutoff
        allowable for the frequency-equivalent half-period (see (3) above).
        Default = 2.0.

period-min-lo-cut-percentage=<float>
        is the minimum fraction of the filter pass-band low cutoff
        allowable for the frequency-equivalent half-period (see (3) above).
        Default = 0.833.

period-max-filter-correction=<float>
        is the maximum filter response correction for the
        frequency-equivalent half-period.  Default = 100.0.

*/



static char * 
var_get(char *type)
{
	static char	val[16];

	if (!strcmp(type, "period-side-peak-threshold"))
	{
		strcpy(val, "0.25");
	}
	else if (!strcmp(type, "period-use-lagrange-interpolation"))
	{
		strcpy(val, "0");
	}
	else if (!strcmp(type, "period-min-half-periods"))
	{
		strcpy(val, "1");
	}
	else if (!strcmp(type, "period-max-half-period-ratio"))
	{
		strcpy(val, "2.0");
	}
	else if (!strcmp(type, "period-max-nyquist-percentage"))
	{
		strcpy(val, "0.8");
	}
	else if (!strcmp(type, "period-window-length-percentage"))
	{
		strcpy(val, "0.25");
	}
	else if (!strcmp(type, "period-max-hi-cut-percentage"))
	{
		strcpy(val, "2.0");
	}
	else if (!strcmp(type, "period-lo-cut-percentage"))
	{
		strcpy(val, "0.833");
	}
	else if (!strcmp(type, "period-max-filter-correction"))
	{
		strcpy(val, "100.0");
	}

	return val;
}
