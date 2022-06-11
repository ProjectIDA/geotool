/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * NAME
 *	find_sequences
 * 
 * FILE 
 *	sequences.c
 *
 * SYNOPSIS
 *
 *	void
 *	find_sequences (data, npts, thresh, start, end, nseg)
 *	float	*data;		(i) Data array                                 
 *	int	npts;		(i) Number of data points                      
 *	int	thresh;		(i) Number of consecutive equal values 
 *				    to be considered a sequence
 *	int	**start;	(o) Indices of start pts of sequence segments
 *	int	**end;		(o) Indices of end pts of sequence segments
 *	int	*nseg;		(o) Number of sequence segments
 *				
 * DESCRIPTION
 *
 * 	find_sequences () fills start[end] with the indices of those data 
 *	points that start[end] a segment which were thresh or more 
 *	consecutive equal valued points.  If consecutive segments
 *	are found, the segment indices are merged to form one segment.
 *	The arrays start, end must be deallocated by the calling routine.
 *	Assumes non-NULL data pointer on input.
 *
 * NOTES
 *
 *	Based on algorithm from SigPro program.
 *
 * AUTHOR
 *	Darrin Wahl
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dyn_array.h"
#include "libdataqcp.h"
#include "libstring.h"

#define FREE(a) if(a) {free(a); a = NULL;}


void
find_sequences(float *data, int npts, int thresh, int **start, int **end,
		int *nseg)
{
	Array	s, e = NULL;	/* Dynamic arrays for start, end */
	int	*st = NULL;
	int	*en = NULL;
	double	hold;		/* Current data value */
	int	n;		/* Number of consecutive equal points */
	int	im1;
	int	i, j;
	
	*nseg = 0;
	*start = NULL;
	*end = NULL;
	
	if(npts < thresh) return;

	/* 
	 * Dynamic arrays for start, end indices of sequences
	 */
	s = array_create (sizeof (int));
	e = array_create (sizeof (int));


	/* 
	 * Loop over all the data points.  If the current point
	 * is the equal to the hold value, increment the number 
	 * of sequence points.  If the current point is not equal to
	 * the hold value and the number of sequence points exceeds
	 * the threshold, add the index of the first point to the 
	 * start array, add the index of the end point to the end
	 * array, and reset the number of sequences and the
	 * hold value.  Otherwise, the current point is different
	 * and becomes the new hold value.
	 *
	 * Special case at the end of the loop if the last n
	 * points of data are equal because the indices were not yet added
	 * to the arrays.
	 */
        n = 1;
        hold = data[0];
	for (i = 1; i < npts; i++)
	{
	    if (data[i] == hold)
	    {
		n++;
	    }                        
	    else if (n >= thresh)
	    {
		j = i - n;
		array_add (s, (char *) &j);

		j = i - 1;
		array_add (e, (char *) &j);

		n = 1;
		hold = data[i];
	    }
	    else
	    {
		n = 1;
		hold = data[i];
	    }
	}
        if (n >= thresh)
	{
	    i = npts - n;
	    array_add (s, (char *) &i);

	    i = npts - 1;
	    array_add (e, (char *) &i);
	}
	
	/* Get number of sequences */
	n = array_count (s);

	/* Merge consecutive sequences into one sequence if possible */
	if (n > 2)	
	{
	    st = (int *) array_list (s);
	    en = (int *) array_list (e);

	    array_free (s);
	    array_free (e);

	    s = array_create (sizeof (int));
	    e = array_create (sizeof (int));

	    array_add (s, (char *) &st[0]);
	    for (i = 1; i < n; i++)
	    {
		im1 = i - 1;
			
		if (en[im1] != st[i] - 1)
		{
		    array_add (e, (char *) &en[im1]);
		    array_add (s, (char *) &st[i]);
		}
	    }
	    array_add (e, (char *) &en[n - 1]);
		
	    FREE(st);
	    FREE(en);
		
	    n = array_count (s);
	}
	
	*nseg = n;
	*start = (int *) array_list (s);
	*end = (int *) array_list (e);
	
	array_free (s);
	array_free (e);

	return;

}	/* find_sequences */



