/*
 * NAME
 * 
 *	avg_max()
 *
 * FILE 
 *
 *	avg_max.c
 *
 * SYNOPSIS
 *
 *	int
 *	avg_max (data, npts, nmax, amp, imax)
 *	float	*data;	(i) Data for analysis
 *	int	npts;	(i) Number of points in data
 *	int	nmax;	(i) Number of maxima to average
 *	double	*avmax;	(o) Output averaged maxima
 *	int	*imax;	(o) Index of maximum value in data
 *
 * DESCRIPTION
 *
 *	This routine computes the average of the nmax largest values
 *	in a set of data.  The rms value of the data is computed.
 *	Then for each point in the data, the rms is subtracted from
 *	the abs value of the point.  The index of the largest difference
 *	is kept. The differences are sorted.  The nmax largest differences
 *	are summed.  If fewer than nmax values are > 0, only these
 *	are averaged.  Errors return -1, ok return 0.
 *
 * DIAGNOSTICS
 *
 *	Warning is printed if no or fewer than nmax maxima are found.
 *
 * SEE ALSO
 *
 *	SONSECA amplitude code
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <math.h>		/*for fabs */
#include "libaesir.h"		    /* For what ?  */
#include "aesir.h"		    /* For what ?  */
#include "logErrorMsg.h"
#include "libstring.h"      /* for mallocWarn   */
#include "libampP.h"		/* For local protos */

/* Local functions forward declaration */
static int d_compare( const void *el1, const void *el2 );

/* Order from largest to smallest */
static int 
d_compare ( const void *el1, const void *el2 )
{
	double	*f = (double *) el1;
	double 	*g = (double *) el2;

	if (*f > *g) 
		return (-1);
	else if (*f < *g) 
		return (1);
	else 
		return (0);
}



int
avg_max( float *data, int npts, int nmax, double *avmax, int *imax )
{
	double	*diff = NULL;
	double	max_sum;
	double	max_diff;
	int	max_i;
	double	rms;
	double	d;
	int	cnt;
	int	ret;
	int	i;
        char    msg[128];

	/* Initialize */
	*avmax = 0.0;
	*imax = 0;

	/* Error check */
	if (!data || npts < 1 || nmax < 1)
	{
		ret = -1;
		goto RETURN;
	}
	
	/* Compute rms value of data */
	ret = fvec_rms (data, npts, &rms);
	if (ret < 0)
	{
		goto RETURN;
	}

	/* Compute differences and keep track of largest */
	if(!(diff = (double *)mallocWarn(npts*sizeof(double))));
	memset ((void *) diff, 0, sizeof (double) * npts);
	max_diff = 0.0;
	max_i = 0;
	for (i = 0; i < npts; i++)
	{
		d = fabs ((double) data[i]) - rms;
		diff[i] = (d > 0.0) ? d : 0.0;
		if (diff[i] > max_diff)
		{
			max_diff = diff[i];
			max_i = i;
		}
	}	

	/* Sort differences */
	qsort ((char *) diff, npts, sizeof (double), d_compare);

	/* Sum the nmax largest differences */
	max_sum = 0.0;
	cnt = 0;
	for (i = 0; i < nmax; i++)
	{
		if (diff[i] > 0.0)
		{
			max_sum += diff[i];
			cnt++;
		}
	}
	if (cnt < 1)
	  {
		logErrorMsg( DFX_VERBOSE0,"avg_max: Flat data.  No maxima found.\n" );
		ret = 0;
		goto RETURN;
	}
	else if (cnt < nmax)
	{
	  snprintf( msg, sizeof(msg),
				"avg_max: Only %d maxima found < %d maxima desired.\n", cnt, nmax );
	  logErrorMsg( DFX_VERBOSE0, msg );
	}
	
	/* Set up return */
	*avmax = (max_sum / (double) cnt) + rms;
	*imax = max_i;

 RETURN:

	FREE (diff);
	return (ret);
}
