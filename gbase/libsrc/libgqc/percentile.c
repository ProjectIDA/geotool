/*
 * Copyright 1992 Science Applications International Corporation.
 *
 * NAME
 *	find_percentile
 * 
 * FILE 
 *	percentile.c
 *
 * SYNOPSIS
 *
 *	void
 *	find_percentile (percentile, data, npts, value)
 *	double	percentile;  	(i) Percentile of interest
 *	float	*data;	 	(i) Array of data    
 *	int	npts;		(i) Number of points in data
 *	double	*value;		(o) Percentile value from data 
 *
 *	static
 *	void
 *	merge (a, b, c, m, n)
 *	float	*a;	(i) First array of data
 *	float	*b;	(i) Second array of data
 *	float	*c;	(o) Output merged array of data in ascending order
 *	int	m	(i) Number of points in a
 *	int	n;	(i) Number of points in b
 *
 *
 * DESCRIPTION
 *
 *	find_percentile () returns the value of a specified percentile 
 *	of the input data.  For example, the median is the 50.0 percentile.
 *	This routine assumes a non-NULL data pointer is input.
 *
 *	merge () merges two arrays in ascending order.
 *	
 * NOTES
 *
 *
 * AUTHOR
 *	Darrin Wahl
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifdef HAVE_VALUES_H
#include <values.h> 
#endif /* HAVE_VALUES_H */
#include "libdataqcp.h" /* For prototypes */
#include "libstring.h"


#define WORK_SPACE 2048	/* static allocation for small vectors */

#define FREE(a) if(a) { free(a); a = NULL; }


/* Local functions forward definition */
static void merge(float *a, float *b, float *c, int m, int n);
	

void
find_percentile(double percentile, float *data, int npts, double *value)
{
	float	s[WORK_SPACE];
	float	w[WORK_SPACE];
	float	*sort = (float *) NULL;
	float   *work = (float *) NULL;
	int	i, j, k, m;
	
	/* Find largest power of two in npts */
	for (m = 1; m < npts; m *= 2);

	/* If necessary, allocate memory for sorted data, work space */
	if(m > WORK_SPACE)
	{
	    if(!(work = (float *)mallocWarn(m*sizeof(float)))) return;
	    memset((char *) work, 0, m*sizeof(float));
	    if(!(sort = (float *)mallocWarn(m*sizeof(float)))) return;
	    memset((char *) sort, 0, m*sizeof(float));
	}
	else
	{
	    work = w;
	    sort = s;
	}
	
	/* Copy data into sort array */
	memcpy((char *)sort, (char *)data,  npts*sizeof(float));

	/* Pad sort array out to m with largest float value */
	for (i = npts; i < m; sort[i++] = MAXFLOAT);

	/* Mergesort the data */
	for (k = 1; k < npts; k *= 2)
	{
	    for (j = 0; j < npts - k + 1; j += k * 2)
		merge(sort+j, sort+j+k, work+j, k, k);
			
	    for (j = 0; j < npts; j++) sort[j] = work[j];
	}

	/* Get percentile index and percentile value */
	i = (int) rint ((double) (percentile * npts) / 100.0) - 1;
	i = i < 0 ? 0 : i;
	*value = sort[i];

	if(m > WORK_SPACE)
	{
	    FREE(work);
	    FREE(sort);
	}
	
	return;
}	/* find_percentile */

static void	
merge(float *a, float *b, float *c, int m, int n)
{
	register int	i = 0;
	register int	j = 0;
	register int	k = 0;
	
	while (i < m && j < n)
	{
	    if (a[i] < b[j]) c[k++] = a[i++];
	    else c[k++] = b[j++];
	}
	while (i < m) c[k++] = a[i++];
	while (j < n) c[k++] = b[j++];
}
