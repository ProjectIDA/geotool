/*
 * NAME
 * 
 *	compute_snr
 *	compute_snr_aux
 *
 * FILE 
 *
 *	snr.c
 *
 * SYNOPSIS
 *
 *	int
 *	compute_snr (data, norm, npts, comp_thr, stav_len, ltav_len, stav_meas,
 *		     stav_method, stav_thresh, ltav_thresh, snr_method, 
 *		     stav, stav_state, ltav, ltav_state, snr, state)
 *	float	*data;		  (i) data vector for computation
 *	float	*norm;		  (i) normalization function for data (stack)
 *	int	npts;		      (i) number of points in data, norm
 *	double	comp_thr;	  (i) norm threshold above which state is 1
 *	int	stav_len;	      (i) desired short term average length samples
 *	int	ltav_len;	      (i) desired long term average length samples
 *	char	*stav_meas;	  (i) desired stav measure ("square","absolute")
 *	char	*stav_method; (i) desired stav method ("recursive","sliding")
 *	int	stav_thresh;	  (i) number of samples in a valid stav window
 *	int	ltav_thresh;	  (i) number of samples in a valid ltav window
 *	char	*snr_method;  (i) desired snr method ("standard","z","logz")
 *	float	**stav;		  (o) short term average vector
 *	int	**stav_state; 	  (o) short term average state vector
 *	float	**ltav;		  (o) long term average vector
 *	int	**ltav_state; 	  (o) long term average state vector
 *	float	**snr;		  (o) snr vector
 *	int	**state;	      (o) state vector for snr
 *	
 * DESCRIPTION
 *
 *	compute_snr() computes the signal-to-noise ratio of the data.
 *	
 * 	Algorithm:
 *
 *	1) Compute a state vector for the data using the norm.  Any points
 *      in the data whose norm is >= comp_thr have a state of one, otherwise
 *	their state is zero.
 *	2) Compute the short-term average (stav) and the state of the stav
 *	using the data. A set of points which has at least stav_thresh*stav_len
 *	points with data state == 1 has an stav state of one, otherwise the
 *	stav state is zero.
 *	3) Compute the long-term average (ltav) and the state of the stav
 *	using the stav. A set of points which has at least ltav_thresh*ltav_len
 *	points with stav state == 1 has an ltav state of one, otherwise the
 *	ltav state is zero.
 *	4) Compute the signal-to-noise ratio (snr) using compute_snr_aux().
 *	5) Set the stav, ltav, snr, and data state vectors for output.
 *	Return -1 on error, 0 otherwise.
 *
 *	compute_snr_aux() computes the signal-to-noise ratio for the
 *	given stav and ltav.
 *	
 * 	Algorithm:
 *
 *	1) Compute the signal-to-noise ratio (snr) from the stav, the ltav, 
 *	and their respective states. If snr_method is "z", use z_stat_snr().  
 *	If snr_method is "logz", compute the log of the stav, compute the 
 *	ltav of the log of the stav, and use z_stat_snr().  If snr_method 
 *	is "standard", use standard_snr().  The snr will be zero where
 *	the states of the stav or the ltav are zero.
 *	5) Set the stav, ltav, snr, and data state vectors for output.
 *	Return -1 on error, 0 otherwise.
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
 *
 * 	Data for output vectors is allocated within calls to routines
 *	in this routine.  These vectors should be free'd by calling
 *	routine.
 * 
 * SEE ALSO
 *
 *	sigpro(1) (file calcsnr.f in SigPro)
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "libstring.h"       /* for mallocWarn  */
#include "logErrorMsg.h"
#include "aesir.h"
#include "libdetectP.h"

int
compute_snr( float *data, float *norm, int npts, double comp_thr, 
			 int stav_len, int ltav_len, char *stav_meas, char *stav_method, 
			 int stav_thresh, int ltav_thresh, char *snr_method, 
			 float **stav, int **stav_state, float **ltav, int **ltav_state, 
			 float **snr, int **state )
{
  float *sp   = NULL;
  int	  *sstp = NULL;
  float *lp   = NULL;
  int	  *lstp = NULL;
  float *snp  = NULL;
  int	  *stp  = NULL;
  int	ret = -1;
	

  /* Initialize */
  *stav = NULL;
  *stav_state = NULL;
  *ltav = NULL;
  *ltav_state = NULL;
  *snr = NULL;
  *state = NULL;


  /* Error checks */
  if (!data || !norm || npts < 1 || stav_len < 1 || ltav_len <stav_len ||
	  !stav_meas || !stav_method || !snr_method)
	{
	  goto RETURN;
	}


  /* Compute state vector from norm function, comp_thr */
  ret = compute_state (norm, npts, comp_thr, &stp);
  if (ret < 0)
	{
	  logErrorMsg( DFX_VERBOSE0, "compute_snr: Error in compute_state\n" );
	  goto RETURN;
	}


  /* Compute stav and state of stav using method and measure */
  ret = compute_stav (data, stp, npts, stav_method,stav_meas,
					  stav_len, stav_thresh, &sp, &sstp);
  if (ret < 0)
	{
	  logErrorMsg( DFX_VERBOSE0,"compute_snr: Error in compute_stav\n" );
	  goto RETURN;
	}


  /* Compute ltav and state of ltav for given stav */
  ret = compute_ltav (sp, sstp, npts, stav_len, ltav_len, 
					  ltav_thresh, &lp, &lstp);
  if (ret < 0)
	{
	  logErrorMsg( DFX_VERBOSE0,"compute_snr: Error in compute_ltav\n" );
	  goto RETURN;
	}

	
  /* Compute snr */
  ret = compute_snr_aux (sp, sstp, lp, lstp, npts, stav_len, 
						 ltav_len, stav_meas, stav_method, stav_thresh,
						 ltav_thresh, snr_method, &snp);
  if (ret < 0)
	{
	  logErrorMsg( DFX_VERBOSE0, "compute_snr: Error computing snr on beam\n" );
	  goto RETURN;
	}


  *stav = sp;
  *stav_state = sstp;
  *ltav = lp;
  *ltav_state = lstp;
  *snr = snp;
  *state = stp;

  ret = 0;

 RETURN:	

  if (ret < 0)
	{
	  FREE (stp);
	  FREE (snp);
	  FREE (sp);
	  FREE (lp);
	  FREE (sstp);
	  FREE (lstp);
	}

  return (ret);
}


int
compute_snr_aux( float *stav, int *sstp, float *ltav, int *lstp, int npts, 
				 int stav_len, int ltav_len, char *stav_meas, char *stav_method, 
				 int stav_thresh, int ltav_thresh, char *snr_method, float **snr )
{
  float	*logsp = NULL;
  float	*loglp = NULL;
  int	*loglstp = NULL;
  float	*snp = NULL;
  int	ret = -1;
  int	i;

  /* Initialize */
  *snr = NULL;


  /* Error checks */
  if (!stav || !sstp || !ltav || !lstp || npts < 1 || stav_len < 1 
	  || ltav_len <stav_len || !stav_meas || !stav_method || !snr_method)
	{
	  goto RETURN;
	}
	
  /* Compute snr for particular snr method */
  if (STREQ (snr_method, "z"))
	{
	  i = 0;	/* log flag */
	  ret = z_stat_snr (stav, sstp, ltav, lstp, npts, stav_len, 
						ltav_len, ltav_thresh, i, &snp);
	  if (ret < 0)
		{
		  logErrorMsg( DFX_VERBOSE0, "compute_snr_aux: Error in z_stat_snr\n" );
		  goto RETURN;
		}
	}
  else if (STREQ (snr_method, "logz"))
	{
	  /* 
	   * If the log z detector is used, we need to compute
	   * the log of the stav for input to z detector.
	   */

	  if(!(logsp = (float *)mallocWarn( npts * sizeof(float))));
	  memset ((void *) logsp, 0, sizeof (float) * npts);

	  for (i = 0; i < npts; i++)
		{
		  if (stav[i] <= DFX_ZERO_VALUE)
			{
			  logsp[i] = DFX_NULL_LOG_VALUE;
			}
		  else
			{
			  logsp[i] = log10 (stav[i]);
			}
		}


	  /* Compute ltav and state of ltav for given log stav */
	  ret = compute_ltav (logsp, sstp, npts, stav_len, 
						  ltav_len, ltav_thresh, &loglp, &loglstp);
	  if (ret < 0)
		{
		  logErrorMsg( DFX_VERBOSE0,"compute_snr_aux: Error in compute_ltav for log z detector\n" );
		  goto RETURN;
		}


	  i = 1;	/* log flag */
	  ret = z_stat_snr (logsp, sstp, loglp, loglstp, npts,
						stav_len, ltav_len, ltav_thresh, i, &snp); 
	  if (ret < 0)
		{
		  logErrorMsg( DFX_VERBOSE0, "compute_snr_aux: Error in z_stat_snr for log z detector\n" );
		  goto RETURN;
		}

	}
  else if (STREQ (snr_method, "standard"))
	{
	  i = 0;	/* square flag */
	  if (STREQ (stav_meas, "square"))
		{
		  i = 1;
		}			
	  ret = standard_snr (stav, sstp, ltav, lstp, npts, 
						  stav_len, ltav_len, i, &snp);
	  if (ret < 0)
		{
		  logErrorMsg( DFX_VERBOSE0, "compute_snr_aux: Error in standard_snr\n" );
		  goto RETURN;
		}
	}
	
  *snr = snp;

  ret = 0;

 RETURN:	

  if (ret < 0)
	{
	  FREE (snp);
	}
  FREE (logsp);
  FREE (loglp);
  FREE (loglstp);

  return (ret);
}

