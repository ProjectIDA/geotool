/*
 * NAME
 * 
 *	peak_trough ()
 *
 * FILE 
 *
 *	peak_trough.c
 *
 * SYNOPSIS
 *
 *	int
 *	peak_trough (beam, npts, pts, btime, samprate, type, amp, per, time)
 *	float 	*beam;		
 *	int	npts;		
 *	int	*pts;
 *	double	btime;
 *	double	samprate;
 *	char	*type;
 *	double	*amp;
 *	double	*per;
 *	double	*time;
 *
 * DESCRIPTION
 *
 *	 Identify all peak and trough indices in a beam.
 *
 * DIAGNOSTICS
 *
 *	Returns OK if successful, ERR otherwise.
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 *
 *	Darrin Wahl	(derived from automb.c in DAP/libampmeas)
 */

#include "config.h"
#include <stdio.h>
#include <math.h>
#include "aesir.h"
#include "libampP.h"		/* For #defines, protos */

int
peak_trough ( float *beam, int npts, int *pts, double btime, double samprate, 
			  char *type, double *amp, double *per, double *time )
{
	double	maxamp;
	int	max0;
	int	max1;
	double	local_amp;
	double	local_per;
	double	local_time;
	int	ret;

	*amp = 0.0;
	*per = -1.0;
	*time = btime;

	/* Check for error conditions */
	if (beam == NULL || pts == NULL || npts <= 0 ) 
	{
		return ERR;
	}

	/* Find maximum abs peak to trough amplitude */
	ret = max_peak_trough( beam, npts, pts, &maxamp, &max0, &max1 );
	if (ret < 0)
	{
		goto RETURN;
	}

	local_amp = maxamp;
	if (STREQ (type, AMP_ZERO_PEAK))
	{
		local_amp /= 2.0;
	}
	local_per = 2.0 * (double) (max1 - max0) / samprate;
	local_time = btime + (double) max0 / samprate;

#if 0
This code for future use

	/* Find previous, next extrema */
	thresh = maxamp * side_peak_thresh;
	max_back = -1;
	for (i = max0 - 1; i > 0; i--)
	{
		if (pts[i] != NEITHER && fabs (beam[i] - beam[max0]) > thresh)
		{
			maxback = i;
		}
	}
	max_fwd = -1;
	for (i = max1 + 1; i < npts - 1; i--)
	{
		if (pts[i] != NEITHER && fabs (beam[i] - beam[max1]) > thresh)
		{
			max_fwd = i;
		}
	}

	/* Interpolate values around extrema */
	
	/* Calculate amp,time from central extrema */

	/* Calculate half periods updated for all extrema */

	/* Calculate best half period (avg) */

#endif

	*amp  = local_amp;
	*per  = local_per;
	*time = local_time;

	ret = 0;

 RETURN:
	
	return (ret);
}

