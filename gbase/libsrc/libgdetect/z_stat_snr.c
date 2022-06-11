/*
 * NAME
 * 
 *	z_stat_snr
 *
 * FILE 
 *
 *	z_stat_snr.c
 *
 * SYNOPSIS
 *	
 *	int
 *	z_stat_snr (stav, stav_state, ltav, ltav_state, npts, stav_len, 
 *		    ltav_len, ltav_thresh, log_flag, snr)
 *	float	*stav;		(i) short term average vector
 *	int	*stav_state;	(i) state of stav
 *	float	*ltav;		(i) long term average vector
 *	int	*ltav_state;	(i) state of ltav
 *	int	npts;		(i) number of points in data, norm
 *	int	stav_len;	(i) short term average length samples
 *	int	ltav_len;	(i) long term average length samples
 *	int	ltav_thresh;	(i) number of samples in a valid ltav window
 *	int	log_flag;	(i) data is log of data, log_flag=1
 *	float	**snr;		(o) signal-to-noise ratio
 *	
 * DESCRIPTION
 *
 *	z_stat_snr() computes the z-statistic ratio of the stav and the
 *	ltav.  The z-statistic is defined as 
 *
 *		snr[i] = (stav[i] - ltav[i]) / sqrt (var (stav[i]))
 * 
 *	where
 *
 *		var(stav[i]) = var(stav[i-1])+
 *			       ((stav[i]-ltav[i])**2 - var(stav[i-1])) / N
 *	
 *	for i=[N/2,npts-N/2], N=ltav_len, M=stav_len.
 *
 *	Algorithm:
 *	
 *	1) Compute the vector of differences of stav and ltav, and
 *	difference state vector.
 *	2) Compute recursive average of squared differences using
 *	a recursive lookback of 1 for an average window length of ltav_len.
 *	This average vector now contains the variances of the stav
 *	about the ltav.
 *	3) Compute the z-statistic snr as the ratio of the difference
 *	of the stav and the ltav to the square root of the variance
 * 	of the stav about the ltav.
 *	4) The snr is allocated and returned.  Return -1 on error, 0 otherwise.
 *	
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
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
#include <stdlib.h>
#include <math.h>	/* For fabs(),floor() */
#include "aesir.h"
#include "libstring.h"       /* for mallocWarn  */
#include "libdetectP.h"


int
z_stat_snr( float *stav, int *stav_state, float *ltav, 
			int *ltav_state, int npts, int stav_len,  
			int ltav_len, int ltav_thresh, int log_flag, float **snr)
{
	float	*snp  = NULL;
	float	*diff = NULL;
	int	   *state = NULL;
	int	    *sntp = NULL;
	double	stdv;
	int	start,end,k;
	int	ims;
	int	ret;
	int	i;

	/* Initialize */
	*snr = NULL;

	/* Allocate memory for snr, diff, snr state */
	if(!(snp = (float *)mallocWarn( npts * sizeof(float))));
	memset ((void *) snp, 0, sizeof (float) * npts);

	if(!(diff = (float *)mallocWarn( npts * sizeof(float))));
	memset ((void *) diff, 0, sizeof (float) * npts);

	if(!(state = (int *)mallocWarn( npts * sizeof(int))));
	memset ((void *) state, 0, sizeof (int) * npts);

	if(!( sntp = (int *)mallocWarn( npts * sizeof(int))));
	memset ((void *) sntp, 0, sizeof (int) * npts);

	/* Fill diff with difference stav-ltav */
	start = end = ltav_len / 2;
	if (ltav_len % 2 == 0)
	{
		end -= 1;
	}
/* 	k = MAX(start,prev_len) + 1; ????????????*/
	k = start + 1;
	for (i = 0; i < k; i++)
	{
		state[i] = stav_state[i] * ltav_state[i];
		diff[i] = stav[i] - ltav[i];
	}		
	for (i = k; i < npts; i++)
	{
		ims = i - stav_len;

		state[i] = stav_state[ims] * ltav_state[i];
		diff[i] = stav[ims] - ltav[i];
	}		


	/* Compute recursive avg of squared differences */
	ret = recursive_avg (diff, state, npts, 1, ltav_len, ltav_thresh,
			     squarex, snp, sntp);
	if (ret < 0)
	{
		goto RETURN;
	}


	/* Compute the snr */
	for (i = 0; i < npts; i++)
	{
		stdv = sqrt (snp[i]);
		
		if (!sntp[i])
		{
			snp[i] = 0;
		}
		else if (log_flag && stdv <= DFX_ZERO_VALUE)
		{
			snp[i] = 0;
		}
		else if (stdv <= (ltav[i] * DFX_ZERO_VALUE))
		{
			snp[i] = 0.0;
		}
		else
		{
			snp[i] = (stav[i] - ltav[i]) / stdv;
		}
	}
	
	*snr = snp;

	ret = 0;

 RETURN:	

	if (ret < 0)
	{
		FREE (snp);
	}
	FREE (diff);
	FREE (state);
	FREE (sntp);

	return (ret);
}


