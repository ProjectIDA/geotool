/*
 * NAME - measure_amplitude ()
 * 
 * FILE 
 *  amp.c
 *
 * SYNOPSIS
 *
 *  int
 *  measure_amplitude (beam, npts, beam_time, samprate, type, value,
 *             thresh, flo, fhi, ford, zp, ftype, interp_per,
 *             amp, per, time)
 *  float   *beam;       (i)
 *  int npts;            (i)
 *  double   beam_time;  (i)
 *  double   samprate;   (i)
 *  char    *type;       (i)
 *  double   value;      (i)
 *  double   thresh;     (i)
 *  double   flo;        (i)
 *  double   fhi;        (i)
 *  int      ford;       (i)
 *  int      zp;         (i)
 *  char    *ftype;      (i)
 *  int      interp_per; (i)
 *  double  *amp;        (o)
 *  double  *per;        (o)
 *  double  *time;       (o)
 *  
 * DESCRIPTION
 *  Obtains a measure of the amplitude, period, and time in a beam.  
 *
 *  For the given beam wave form, the following algorithm is executed:
 *
 *  (1) Identify all peak and trough indices.
 *  (2) Iteratively, find the smallest peak to trough amplitude and
 *      eliminate it from from the peak-trough index list.
 *  (3) Perform (2) until all peak-trough index pairs that had an 
 *      amplitude less than a threshold % of max amplitude have been
 *      eliminated from the peak-trough index list.
 *  (4) Measure amplitude, period, and time of amplitude measure.
 *
 *
 * DIAGNOSTICS
 *  Returns OK if successful, ERR otherwise.
 *
 * SEE ALSO
 *
 *  "Event Classification for the Seismic Technique", and
 *  AFTAC document.
 *
 * AUTHOR
 *
 *  Mari Mortell    May 1993    Derived from automb.c (DAP/libampmeas)
 *  Darrin Wahl                 significantly modified for DFX
 */

#include "config.h"
#include <stdio.h>
#include <math.h>
#include "aesir.h"
#include "libaesir.h"
#include "logErrorMsg.h"
#include "libstring.h"      /* for mallocWarn   */
#include "libampP.h"        /* For PEAK,TROUGH,NEITHER */


/* Local functions forward declaration */
#if 0
static int count_peak_trough( int *pts, int npts );
#endif
static int get_per_from_flo_fhi( double flo, double fhi, char *ftype, double samprate, double *per );

int
measure_amplitude( float *beam, int npts, double beam_time, 
                   double samprate, char *type, double value,
                   double thresh, double flo, double fhi, 
                   int ford, int zp, char *ftype, int interp_per,
                   double *amp, double *per, double *time )
{
  int    *pts = NULL;
  double  maxdiff;
  double  maxamp;
  int     max0, max1;
  int     nstav;
  int     nmax;
  double  local_amp;
  double  local_time; 
  double  local_per;
  int     ret = -1;
  int     i;  
  char	  msg[128];


  /* Initialize */
  *amp = 0.0;
  *per = -1.0;
  *time = beam_time;

  /* Error checks */
  if( !beam || npts < 1 ) {
    logErrorMsg( DFX_VERBOSE0, "measure_amplitude: No beam input\n" );
    goto RETURN;
  }       

  if( samprate <= 0.0 ) {
	snprintf( msg, sizeof(msg), "measure_amplitude: Bad samprate %f\n", samprate );
    logErrorMsg( DFX_VERBOSE0, msg );
    goto RETURN;
  }
       
  if( thresh < 0.0 || thresh > 1.0 ) {
	snprintf( msg, sizeof(msg),"measure_amplitude: Bad smoothing threshold %f\n", thresh );
    logErrorMsg( DFX_VERBOSE0, msg );
    goto RETURN;
  }       

  if( flo < 0.0 || fhi <= flo ) {
	snprintf( msg, sizeof(msg),"measure_amplitude: Bad filter band %f to %f\n", flo, fhi );
    logErrorMsg( DFX_VERBOSE0, msg );
    goto RETURN;
  }       

  if( !type) {
    logErrorMsg( DFX_VERBOSE0, "measure_amplitude: Need a measurement type\n" );
    goto RETURN;
  }       

  /* Allocate array for extrema labels */
  if( !( pts = (int *)mallocWarn( npts*sizeof(int) ) ) );
  memset( (void *) pts, 0, npts * sizeof(int) );

  /* Find peaks and troughs in the beam */
    
  ret = find_peak_trough (beam, npts, pts, &maxdiff);
  if( ret < 0) {
    logErrorMsg( DFX_VERBOSE0, "measure_amplitude: Error in find_peak_trough\n" );        
    goto RETURN;
  }

  if( maxdiff == 0.0) {
	snprintf( msg, sizeof(msg),"measure_amplitude: Largest amplitude excursion too small %f\n", maxdiff );
    logErrorMsg( DFX_VERBOSE0, msg );
    ret = -1;
    goto RETURN;
  }

#if 0
  /* This code for future use */
  nextrema = count_peak_trough (pts, npts);
  if( nextrema < 4) {
	snprintf( msg, sizeof(msg),
			  "measure_amplitude : Number of extrema found (%d) < number required (%d).\n", nextrema, 4 );
    logErrorMsg( DFX_VERBOSE0, msg );
    goto RETURN;
  }
#endif  

  /* Smooth the peaks and troughs according to thresh */
  ret = smooth_peak_trough (beam, npts, pts, maxdiff, thresh);
  if( ret < 0) {
    logErrorMsg( DFX_VERBOSE0, "measure_amplitude: Error in smooth_peak_trough\n" );        
    goto RETURN;
  }

#if 0
  /* This code for future use */
  nextrema = count_peak_trough (pts, npts);
  if( nextrema < 4) {
	snprintf( msg, sizeof(msg),
			  "measure_amplitude: Number of extrema found after smoothing (%d) < number required (%d).\n", nextrema, 4 );
    logErrorMsg( DFX_VERBOSE0, msg );
    return = -1;
    goto RETURN;
  }
#endif

  /* Compute the maximum peak-to-trough excursion of the beam */
  ret = max_peak_trough (beam, npts, pts, &maxamp, &max0, &max1);
  if( ret < 0) {
    logErrorMsg( DFX_VERBOSE0, "measure_amplitude: Error in max_peak_trough\n" );        
    goto RETURN;
  }

  /* Measure amplitude, period, time of measurement */
  if( STREQ(type, AMP_PEAK_TROUGH) || 
      STREQ(type, AMP_ZERO_PEAK  )   ) {

    /* 
     * If the measurement type is peak-to-trough, set the
     * amplitude.  If the measurement type is zero-to-peak, 
     * compute the amplitude.  Use the period at the time of 
     * the maximum peak-to-trough amplitude.  
     */
    local_amp = maxamp;
    if( STREQ (type, AMP_ZERO_PEAK)) {
      local_amp *= 0.5;
    }
    local_per = 2.0 * (double) (max1 - max0) / samprate;
    local_time = beam_time + (double) max0 / samprate;
    
    /* If specified, revise period using interpolated maxima */
    
    if( interp_per) {
      double  really_local_period;

      if( interp_period(beam, npts, pts, max0, max1,
                        samprate, flo, fhi, ford, zp,
                        &really_local_period )  == ERR) {

        logErrorMsg( DFX_VERBOSE0,
                     "measure_amplitude: Error in interp_period.  Using period from peak-trough computation\n" );        

      } else if( really_local_period <= 0.0) {

		snprintf( msg, sizeof(msg),
				  "measure_amplitude: Invalid period %.3f returned from interp_period.  Using period from peak-trough computation\n",
				  really_local_period );
        logErrorMsg( DFX_VERBOSE0, msg );

      } else {
        local_per = really_local_period;
      }
    }

  } else if( STREQ(type, AMP_FM_A )   || 
             STREQ(type, AMP_FM_B )   ||
             STREQ(type, AMP_FM   )   || 
             STREQ(type, AMP_FM_SIGN )   ) {

    /* Compute the amplitude, period, time time of first motion */
    ret = first_motion (beam, npts, pts, beam_time,
                        samprate, type, &local_amp, 
                        &local_per, &local_time);
    if( ret < 0) {
      logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in first_motion\n" );     
      goto RETURN;
    }
        
  } else if( STREQ(type, AMP_RMS) || STREQ(type, AMP_MEAN_SQR)) {

    /* 
     * If the measurement type is either rms or mean square,
     * compute the rms value of the beam as the amplitude.
     * Square the rms value if the measurement type is mean square.
     * Use the inverse center band frequency as the period.
     */
    ret = fvec_rms (beam, npts, &local_amp);
    if( ret < 0) {
      logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in fvec_rms\n" );     
      goto RETURN;
    }

    if( STREQ(type, AMP_MEAN_SQR)) {
      local_amp *= local_amp;
    }
    ret = get_per_from_flo_fhi 
      (flo, fhi, ftype, samprate, &local_per);
    if( ret < 0) {
      logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in get_per_from_flo_fhi\n" );     
      goto RETURN;
    }
    local_time = beam_time;
  
  } else if( STREQ(type, AMP_ABS_MAX)) {

    /* 
     * If the measurement type is absolute maximum, compute 
     * the absolute maximum value of the beam as the amplitude.
     * Use the inverse center band frequency as the period.
     */
    i = fvec_abs_max (beam, npts, &local_amp);
    if( i < 0) {
      logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in fvec_abs_max\n" );     
      ret = -1;
      goto RETURN;
    }
    ret = get_per_from_flo_fhi 
      (flo, fhi, ftype, samprate, &local_per);
    if( ret < 0) {
      logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in get_per_from_flo_fhi\n" );     
      goto RETURN;
    }
    local_time = beam_time + (double) i / samprate;

  } else if( STREQ(type, AMP_AVG_MAX) || STREQ(type, AMP_STAV)) {

    /* 
     * For these measurement types, we need a valid associated
     * measurement value.  Check for NULL (-1.0).
     */
    if( value < 0.0) {
	  snprintf( msg, sizeof(msg),
				"measure_amplitude: Bad measurement value %f for measurement type %s\n", value, type );
      logErrorMsg( DFX_VERBOSE0, msg );     
      ret = -1;
      goto RETURN;
    }
            
    if( STREQ (type, AMP_AVG_MAX)) {

      /* 
       * If the measurement type is average maximum, compute 
       * the average maximum value of the beam as amplitude.
       * Use the inverse center band frequency as the period.
       */
      nmax = (int) rint (value);
      ret = avg_max (beam, npts, nmax, &local_amp, &i);
      if( ret < 0) {
        logErrorMsg( DFX_VERBOSE0, "measure_amplitude: Error in avg_max\n" );
        goto RETURN;
      }
      ret = get_per_from_flo_fhi 
        (flo, fhi, ftype, samprate, &local_per);
      if( ret < 0) {
        logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in get_per_from_flo_fhi\n" );     
        goto RETURN;
      }
      local_time = beam_time + (double) i / samprate;
    } else {
      /* 
       * If the measurement type is short-term average, 
       * compute the max stav of the beam as amplitude.
       * Use the inverse center band frequency as the period.
       */
      nstav = (int) rint (samprate * value);
      ret = max_stav (beam, npts, nstav, &local_amp, &i);
      if( ret < 0) {
        logErrorMsg( DFX_VERBOSE0, "measure_amplitude: Error in max_stav\n" );
        goto RETURN;
      }
      ret = get_per_from_flo_fhi 
        (flo, fhi, ftype, samprate, &local_per);
      if( ret < 0) {
        logErrorMsg( DFX_VERBOSE0,"measure_amplitude: Error in get_per_from_flo_fhi\n" );     
        goto RETURN;
      }
      local_time = beam_time + (double) i / samprate;
    }           

  }  else  {

	snprintf( msg, sizeof(msg),
			  "measure_amplitude: Invalid measurement type %s\n", type );
    logErrorMsg( DFX_VERBOSE0, msg );
    ret = -1;
    goto RETURN;

  }    
    
  *amp = local_amp;
  *per = local_per;
  *time = local_time;

  ret = 0;
    
 RETURN:

  FREE (pts);

  return (ret);
}


#if 0
static int 
count_peak_trough( int *pts, int npts)
{
  int nextrema = 0;   
  int i;

  for (i = 0; i < npts; i++) {
    if( pts[i] == PEAK || pts[i] == TROUGH) {
      nextrema++;
    }
  }
  return (nextrema);
}
#endif

static int
get_per_from_flo_fhi( double flo, double fhi, char *ftype, double samprate, double *per )
{
  double  nyquist;
  char	  msg[128];

  *per = -1.0;

  if( samprate <= 0.0) {
    logErrorMsg( DFX_VERBOSE0, "get_per_from_flo_fhi: Bad samprate\n" );
    return (-1);
  }

  /* Test on reasonable frequency band wrt to Nyquist frequency */
  nyquist = 0.5 * samprate;
  if( BANDPASS_FILTER (ftype) || 
      LOWPASS_FILTER (ftype)  || 
      BANDREJECT_FILTER (ftype) ) {
    if( fhi >= nyquist) {
	  snprintf( msg, sizeof(msg),
				"get_per_from_flo_fhi: High frequency cutoff %f >= Nyquist %f\n", fhi, nyquist );
      logErrorMsg( DFX_VERBOSE0, msg );
      return (-1);
    } else if( BANDPASS_FILTER (ftype)) {
      *per = 1.0 / (flo + 0.5 * (fhi - flo));
    } else if( LOWPASS_FILTER (ftype)) {
      *per = 2.0 / fhi;
    } else if( BANDREJECT_FILTER (ftype)) {
      *per = 1.0 / samprate;
      if( BETWEEN (samprate, flo, fhi))
        *per = 1.0 / fhi;
    }       
  } else if( HIGHPASS_FILTER (ftype)) {
    if( flo >= nyquist) {
	  snprintf( msg, sizeof(msg),
				"get_per_from_flo_fhi: Low frequency cutoff %f >= Nyquist %f\n", flo, nyquist );
      logErrorMsg( DFX_VERBOSE0, msg );
      return (-1);
    } else {
      *per =  1.0 / (flo + 0.5 * (nyquist - flo));
    }       
  } else if( NO_FILTER (ftype)) {
    *per = 1.0 / nyquist;
  
    logErrorMsg( DFX_VERBOSE2, "get_per_from_flo_fhi: No filter applied.  Period set to inverse Nyquist frequency\n" );
  } else {
	snprintf( msg, sizeof(msg),
			  "get_per_from_flo_fhi: Bad filter type %s\n", ftype );
    logErrorMsg( DFX_VERBOSE0, msg );
    return (-1);
  }
  
  return (0);
}
