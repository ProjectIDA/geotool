/*
 * NAME
 * 
 * 	smooth_peak_trough ()
 * FILE 
 *
 * 	smooth_peak_trough.c
 *
 * SYNOPSIS
 *
 *	int
 *	smooth_peak_trough (beam, npts, pts, maxdiff, thresh)
 *	float 	*beam;		(i)
 *	int	npts;		(i)
 *	int	*pts;		(i)
 *	double	maxdiff;	(i)
 *	double	thresh;		(i)
 *
 * DESCRIPTION
 *	Iteratively, find the smallest peak to trough amplitude and
 *      eliminate it from from the peak-trough index list.
 *	Perform this until all peak-trough index pairs that had an 
 *	amplitude less than a threshold % of max amplitude have been
 *	eliminated from the peak-trough index list.
 *
 * DIAGNOSTICS
 *
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
smooth_peak_trough( float *beam, int npts, int *pts, double maxdiff, double thresh )
{
	double	amp;		    /* Current amplitude */
	double  minamp;         /* Current minimum peak-trough amp */
	int	min0;		        /* Min peak-trough pair indices */
	int	min1;		
	double	min_amp_thresh;	/* Smallest allowable peak-trough amp */
	int	right_end_pt;       /* Rightmost end-point in buffer during smoothing */
	int	left_end_pt;        /* Leftmost end-point in buffer during smoothing */
	int	more_mins;
	int	first_i;
	int	last_i;
	int	internal_i;
	int	i, j;
	int	ret = -1;

	/* Error checks */
	if (!beam || npts < 1 || !pts || thresh < 0.0 || maxdiff <= 0.0)
	{
		goto RETURN;
	}
	if (thresh == 0.0)
	{
		ret = 0;
		goto RETURN;
	}
	

	/* Compute minimum amplitude threshold */
	min_amp_thresh = maxdiff * thresh;
       

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

	/* Loop to find unimportant peak-trough pairs and annihilate them */
	more_mins = TRUE;
	while (more_mins)
	{
		/* Reset minamp to find new minimum this pass */
		minamp = maxdiff;  

		/* 
		 * Find first "internal" peak or trough point and
		 * initialize its index
		 */
		for (i = first_i + 1; pts[i] == NEITHER && i < last_i; i++);
		left_end_pt = internal_i = i;


		/* Loop thru remaining points and find min amplitude indices*/
		min0 = 1;
		min1 = 1;
		for (j = internal_i + 1; j < last_i; j++)
		{
			/* Find next peak or trough, skip if neither */
			if (pts[j] != NEITHER)
			{
				right_end_pt = j;  
			

				/* Check abs amplitude to see if its an acceptable min*/
				amp = fabs (beam[left_end_pt] - 
							beam[right_end_pt]);


				/* Found new acceptable min */
				if (amp < min_amp_thresh && amp < minamp) 
				{
					min0 = left_end_pt;   
					min1 = right_end_pt;
					minamp = amp;
				}


				/* Re-assign FirstPt and end point */
				left_end_pt = right_end_pt;
			}
		}


		/* 
		 * Annihilate current minimum or break out if no more mins 
		 * qualify 
		 */
		if (minamp < min_amp_thresh)
		{
			pts[min0] = NEITHER;
			pts[min1] = NEITHER;	
		}
		else /* No more mins */
		{
			more_mins = FALSE;
		}
	}

	ret = 0;

 RETURN:

	return (ret);
}
