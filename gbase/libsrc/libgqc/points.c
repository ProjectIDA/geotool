/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * NAME
 *	find_points
 * 
 * FILE 
 *	points.c
 *
 * SYNOPSIS
 *
 *	void
 *	find_points (data, npts, thresh, start, end, nseg)
 *	float	*data;		(i) Data array                                 
 *	int	npts;		(i) Number of data points                      
 *	double	thresh;		(i) Spike point threshold multiplier
 *	int	**start;	(o) Indices of start pts of sequence segments
 *	int	**end;		(o) Indices of end pts of sequence segments
 *	int	*nseg;		(o) Number of sequence segments
 *				
 * DESCRIPTION
 *
 * 	find_points () determines single point zero gaps and single point
 *	spikes within a data sequence.  The data does not need to
 *	be demeaned on input.  If npts < 3, no action is taken.
 *	A single point zero gap is a data point whose value is < QC_ZERO_TOL
 *	and which has the same sign before and after it.
 *	A single point spike is a data point which has differences of
 *	opposite sign before and after it with the smaller of these 
 *	differences exceeding thresh times the larger of the differences 
 *	one sample away.
 *	The arrays start[end] are filled with the indices of those data 
 *	points that start[end] a segment which were found to be zero gaps
 *	or spikes, and nseg is set to the number of these segments.
 *	This routine assumes a non-NULL data pointer on input.
 *
 * NOTES
 *
 *	The arrays start, end must be deallocated by the calling routine.
 *
 *	Single point spike detection based on algorithm used by SigPro
 *	program routine spike5() written by Eric Chael.
 *
 * AUTHOR
 *	Darrin Wahl
 */

#include "config.h"
#include <stdio.h>
#include <math.h>
#include "dyn_array.h"
#include "libdataqcp.h"
#include "libstring.h"
#include	<sys/types.h>

#define ABS(x)		(fabs ((double)(x)))

#define CHECK_ZERO(d,dp1,dm1)	( (ABS((d))<QC_ZERO_TOL) && ((dp1)*(dm1)>0.0) )


void
find_points(float *data, int npts, double thresh, int **start, int **end,
		int *nseg)
{
        Array	s = NULL;
	Array	e = NULL;

	double	diff0, diff1;
	double	test0, test1;
	double	d0, d1;
	double	dp1, dp2, dm1, dm2;
	int	i;
	
	/* Initialize */
	*nseg = 0;
	*start = NULL;
	*end = NULL;

	/* Assume valid data pointer on input */
	if (npts < 3)
	{
		return;
	}

	/* 
	 * Create dynamic arrays for bad segment start, end indices
	 * if not fixing data
	 */
	s = array_create (sizeof (int));
	e = array_create (sizeof (int));

	/* 
	 * Look for single point zero gaps and 
	 * single point spikes.  First two points
	 * and last two points are not checked for 
	 * spike detection, so do these zero gap
	 * separately.  Make sure segments are
	 * ordered in time.
	 */

	i = 1;
	if (CHECK_ZERO (data[i],data[i+1],data[i-1]))
	{
	    array_add (s, (char *) &i);
	    array_add (e, (char *) &i);
	}	
	diff0 = data[1] - data[2];
	for (i = 2; i < npts - 2; i++)
	{
	    dm1 = data[i - 1];
	    dm2 = data[i - 2];
	    dp1 = data[i + 1];
	    dp2 = data[i + 2];
		
	    diff1 = data[i] - dp1;

	    /* 	
	     * Test for single point zero gaps
	     * and mask them if found.
	     *
	     * If no zero gap, test for single point spikes
	     * and mask them if found.
	     * 
	     */
	    if(CHECK_ZERO (data[i],dp1,dm1))
	    {
		array_add (s, (char *) &i);
		array_add (e, (char *) &i);
	    }	
	    else if (diff0 * diff1 < 0.0)
	    {
		d0 = ABS (diff0);
		d1 = ABS (diff1);
			
		test0 = MIN (d0, d1);

		d0 = ABS (dm2 - dm1);
		d1 = ABS (dp1 - dp2);
			
		test1 = MAX (d0, d1);
			
		if(test0 > thresh * test1)
		{
		    array_add (s, (char *) &i);
		    array_add (e, (char *) &i);
		}	
	    }
	    diff0 = diff1;
	}
	i = npts - 2;
	if(CHECK_ZERO(data[i],data[i+1],data[i-1]))
	{
	    array_add(s, (char *) &i);
	    array_add(e, (char *) &i);
	}	

	*nseg = array_count(s);
	*start = (int *)array_list(s);
	*end = (int *)array_list(e);	

	array_free(s);
	array_free(e);
	
	return;
 
}	/* find_points */
