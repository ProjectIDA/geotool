/*
 * NAME
 * 
 *	standard_snr
 *
 * FILE 
 *
 *	standard_snr.c
 *
 * SYNOPSIS
 *
 *	int
 *	standard_snr  (stav, stav_state, ltav, ltav_state, npts, stav_len, 
 *		       ltav_len, sqr_flag, snr)
 *	float	*stav;		(i) short term average vector
 *	int	*stav_state;	(i) state of stav
 *	float	*ltav;		(i) long term average vector
 *	int	*ltav_state;	(i) state of ltav
 *	int	npts;		(i) number of points in data, norm
 *	int	stav_len;	(i) short term average length samples
 *	int	ltav_len;	(i) long term average length samples
 *	int	sqr_flag;	(i) data is squared, sqr_flag==1
 *	float	**snr;		(o) signal-to-noise ratio
 *	
 * DESCRIPTION
 *
 *	standard_snr() computes the ratio of the stav and the ltav.
 *	The snr vector is allocated and returned.  If the ltav is greater 
 *	than zero, the snr is calculated as the stav divided by the ltav
 *	and modulated by the product of the stav state and ltav state.
 *	If either state is zero, the snr is zero.  If the ltav is zero,
 *	the snr is set to zero.  If the data is squared, the snr is square
 *	rooted.
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
#include "aesir.h"
#include "libaesir.h"
#include "libstring.h"       /* for mallocWarn  */
#include "libdetectP.h"

int
standard_snr( float *stav, int *stav_state, float *ltav, int *ltav_state, 
			  int npts, int stav_len, int ltav_len, int sqr_flag, float **snr )
{
	float	*snp = NULL;
	int	ret;
	int	i;


	/* Initialize */
	*snr = NULL;


	/* Allocate memory for snr */
	if(!(snp = (float *)mallocWarn( npts * sizeof(float))));
	memset ((void *) snp, 0, sizeof (float) * npts);


	/* Compute the snr */
	for (i = 0; i < npts; i++)
	{
		if (ltav[i] <= DFX_ZERO_VALUE)
		{
			snp[i] = 0.0;
		}
		else
		{
			snp[i] = stav_state[i]*ltav_state[i]*stav[i]/ltav[i];
		}
	}

	/* Root snr if data is squared */
	if (sqr_flag)
	{
		ret = fvec_sqrt (snp, npts);
		if (ret < 0)
		{
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
	
	return (ret);
}

