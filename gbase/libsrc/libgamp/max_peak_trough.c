/*
 * NAME
 * 
 *	max_peak_trough ()
 *
 * FILE 
 *
 *	max_peak_trough.c
 *
 * SYNOPSIS
 *
 *	int
 *	max_peak_trough (beam, npts, pts, max_amp, max_0, max_1)
 *	float	*beam;		(i)
 *	int	npts;		(i)
 *	int	*pts;		(i)
 *	double	*max_amp;	(o)
 *	int	*max_0;		(o)
 *	int	*max_1;		(o)
 *
 * DESCRIPTION
 *	Find the largest absolute peak to trough amplitude.
 *
 * DIAGNOSTICS
 *	Returns OK if successful, ERR otherwise.
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

/* double *max_amp - Largest peak to trough amp */
/* int    *max_0   - Max peak-trough pair       */

int
max_peak_trough( float *beam, int npts, int *pts, double *max_amp, int *max_0, int *max_1 )
{
	double	amp;		/* Current amplitude */
	double	maxamp;		/* Largest peak to trough amp so far */
	int	max0;		    /* Max peak-trough pair */
	int	max1;
	int	right_end_pt;  	/* Rightmost end-point in buffer */
	int	left_end_pt;    /* Leftmost end-point in buffer */
        int	first_i;
	int	last_i;
	int	ret = -1;
	int	i;
	

	/* Initialize */
	*max_amp = 0.0;
  	*max_0 = 0;	
	*max_1 = 0;
	maxamp = 0.0;
  	max0 = 0;	
	max1 = 0;


	/* Error checks */
	if (!beam || npts < 1 || !pts)
	{
		goto RETURN;
	}		


	/* Find first and last peak/trough points */
	for (i = 0; pts[i] == NEITHER && i < npts; i++);
	first_i = i;
	if (first_i == npts - 1)
	{		
		goto RETURN;
	}
	for (i = npts - 1; pts[i] == NEITHER && i > first_i; i--);
	last_i = i;	
	if (last_i == first_i)
	{
		goto RETURN;
	}
	

	/* Initialize first extrema as left_end_pt */
	left_end_pt = first_i;


	/* Loop thru remaining points and find max amplitude and indices */
	for (i = first_i + 1; i < last_i; i++)
	{
		/* Find next peak or trough, skip if neither */
		if (pts[i] != NEITHER)
		{
			/* Assign right_end_pt */
			right_end_pt = i;  
			

			/* Compute abs amplitude */
			amp = fabs (beam[left_end_pt] - beam[right_end_pt]);


			/* Find max amp and indices */
			if (amp > maxamp) 
			{
				max0 = left_end_pt;	
				max1 = right_end_pt;
				maxamp = amp; 
			}
				
			/* Re-assign left end point */
			left_end_pt = right_end_pt;
		}
	}

	*max_amp = maxamp;
  	*max_0 = max0;	
	*max_1 = max1;

	ret = 0;

 RETURN:

	return (ret);
}


