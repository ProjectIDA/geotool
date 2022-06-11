/*
 * NAME
 * 
 *	first_motion ()
 *
 * FILE 
 *
 *	first_motion.c
 *
 * SYNOPSIS
 *
 *	int
 *	first_motion (beam, npts, pts, btime, samprate, type, amp, 
 *		      per, time)
 *	float	*beam;		(i)
 *	int	npts;		(i)
 *	int	*pts;		(i)
 *	double	btime;		(i)
 *	double	samprate;	(i)
 *	char	*type;		(i) 
 *	double	*amp;		(o)
 *	double	*per;		(o)
 *	double	*time;		(o)
 *
 * DESCRIPTION
 *
 * 	Determine first motion amplitude (fm), 	first motion absolute 
 *	amplitude (a) or second half-cycle of first motion (b).
 *
 * 	The first point that is a peak or trough is where to measure
 * 	the first motion.  This is a quarter cycle, so multiply time
 * 	by 4.0 to get period.  This is an absolute amplitude.
 *	The first motion amplitude is written as only the sign of the
 *	motion (+/- 1.0).
 *
 * 	The second half cycle is measured after first motion.
 * 	Multiply time by 2.0 to get period. This is a relative amplitude.
 *
 * DIAGNOSTICS
 *	Returns 0 if successful, -1 otherwise.
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
first_motion ( float *beam, int npts, int *pts, double btime, 
			   double samprate, char *type, double *amp, 
			   double *per, double *time)
{
	double	local_amp;
	double	local_per;
	double	local_time;
	int	i, j;
	int	ret = -1;

	/* Initialize */
	*amp  = 0.0;
  	*per  = 0;	
	*time = 0;
	local_amp  = 0.0;
	local_per  = 0.0;
	local_time = 0.0;

	/* Error checks */
	if (!beam || npts < 1 || !pts || samprate <= 0.0 || !type)
	{
		goto RETURN;
	}		

	/* Find first peak/trough point */
	for (i = 0; pts[i] == NEITHER && i < npts; i++);
	
	if( STREQ(type, AMP_FM)     || 
		STREQ(type, AMP_FM_A)   || 
	    STREQ(type, AMP_FM_SIGN)   )
	{
		if (STREQ(type, AMP_FM_A))
		{
			local_amp = fabs (beam[i]); 
		}
 		else if (STREQ(type, AMP_FM_SIGN))
		{
			local_amp = (beam[i] > 0.0) ? 1.0 : -1.0;
		}
		else
		{
			local_amp = beam[i];
		}
		local_per = 4.0 * (double) i / samprate;
		local_time = btime;
	}
	else if (STREQ(type, AMP_FM_B))
	{
		/* Find second peak/trough point */
		for (j = i + 1; pts[j] == NEITHER && j < npts; j++);

		local_amp = fabs (beam[j] - beam[i]); 
		local_per = 2.0 * (double) (j - i) / samprate;
		local_time = btime + (double) i / samprate;
	}
	else
	{
		goto RETURN;
	}


	*amp = local_amp;
	*per = local_per;
	*time = local_time;

	ret = 0;

 RETURN:
	
	return (ret);
}


