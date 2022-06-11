/*
 * NAME
 * 
 *	max_stav
 *
 * FILE 
 *
 *	max_stav.c
 *
 * SYNOPSIS
 *
 *	int
 *	max_stav (data, npts, nmax, amp, imax)
 *	float	*data;	(i) Data for analysis
 *	int	npts;	(i) Number of points in data
 *	int	nstav;	(i) Number of points in short-term average window
 *	double	*maxstav; (o) Output maximum short-term average
 *	int	*imax;	(o) Index of center of short-term average maximum
 *
 * DESCRIPTION
 *
 *	This routine computes the maximum short-term average value
 *	in a set of data.  The centered running average of the data is 
 *	computed, and the maximum value is determined along with the
 *	index of the maximum value.  Errors return -1, ok return 0.
 *
 * DIAGNOSTICS
 *
 *	Warning is printed if errors are encountered computing running
 *	average or determining max value.
 *
 * SEE ALSO
 *
 *	DFX libdetect for running_avg()
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <math.h>               /*for fabs */
#include "libaesir.h"
#include "libdetect.h"	/* for running_avg */
#include "libampP.h"	/* For local protos */
#include "logErrorMsg.h"
#include "libstring.h"  /* for mallocWarn   */

/* Local functions forward declaration */

static double absx( double x );

static double
absx ( double x )
{
	return (fabs(x));
}


int
max_stav( float *data, int npts, int nstav, double *maxstav, int *imax )
{
	float	*avg       = NULL;
	int	    *state     = NULL;
	int	    *avg_state = NULL;
	double	max;
	int	i;
	int	ret;

	/* Initialize */
	*maxstav = 0.0;
	*imax = 0;

	/* Error check */
	if( !data || npts < 1 || nstav < 1)
	{
		ret = -1;
		goto RETURN;
	}
	if (nstav > npts)
	{
		logErrorMsg( DFX_VERBOSE0,"max_stav: Short-term average length > data window\n" );
		ret = -1;
		goto RETURN;
	}

	/* Allocate local vectors */
	/* DFX_MALLOC_CHECK (state = UALLOC (int, npts)); */
	if(!(state = (int *)mallocWarn( npts * sizeof(int))));
	memset ((void *) state, 0, sizeof (int) * npts);

	/* DFX_MALLOC_CHECK (avg = UALLOC (float, npts)); */
	if(!(avg = (float *)mallocWarn( npts * sizeof(float))));
	memset ((void *) avg, 0, sizeof (float) * npts);

	/* DFX_MALLOC_CHECK (avg_state = UALLOC (int, npts)); */
	if(!(avg_state = (int *)mallocWarn( npts * sizeof(int))));
	memset ((void *) avg_state, 0, sizeof (int) * npts);
	

	/* Initialize state to 1 */
	for (i = 0; i < npts; i++)
		state[i] = 1;
	

	/* Compute running average of data */
	ret = running_avg( data, state, npts, nstav, nstav, 
					   absx, avg, avg_state);
	if (ret < 0)
	{
		logErrorMsg( DFX_VERBOSE0,"max_stav: Error computing short-term average\n" );
		goto RETURN;
	}
	

	/* Find maximum short-term average value */
	ret = fvec_max( avg, npts, &max );
	if (ret < 0)
	{
		logErrorMsg( DFX_VERBOSE0,"max_stav: Error finding maximum short-term average\n" );
		goto RETURN;
	}

	/* Set up return */
	*maxstav = max;
	*imax = ret;
	

	ret = 0;
	
 RETURN:

	FREE (state);
	FREE (avg_state);
	FREE (avg);
	
	return (ret);
}
