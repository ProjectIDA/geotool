/*
 * NAME
 * 
 *	compute_state
 *
 * FILE 
 *
 *	compute_state.c
 *
 * SYNOPSIS
 *
 *	int
 *	compute_state (norm, npts, thr, state)
 *	float  *norm;	(i) norm function for desired data state
 *	int	    npts;	(i) number of points in norm
 *	double  thr;	(i) threshold above which a norm's state is on
 *	int	  **state;  (i) state vector of norm
 *	
 * DESCRIPTION
 *
 *	compute_state() computes a state vector for an input norm 
 *	vector.  The norm values are compared to thr.  If the value
 *	is greater than or equal to thr, the state of the norm is
 *	set to one, otherwise the state is set to zero.  The state
 *	vector is allocated and returned.  Returns -1 on error, 
 *	0 on success.
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <stdio.h>
#include "libstring.h"       /* for mallocWarn  */
#include "aesir.h"
#include "libdetectP.h"      /* for prototypes etc. */

int
compute_state( float *norm, int npts, double thr, int **state)
{
	int	*stp = NULL;
	int	i;

	/* Initialize */
	*state = NULL;

	/* Error checks */
	if (!norm || npts < 1)
	{
		return (-1);
	}
	
	
	/* Allocate memory for state */
	if(!(stp = (int *)mallocWarn(npts*sizeof(int))));
	memset ((void *) stp, 0, sizeof (int) * npts);


	for (i = 0; i < npts; i++)
	{
		stp[i] = (norm[i] >= thr) ? 1 : 0;
	}

	*state = stp;

	/* where is stp FREE'd ? */

	return (0);
}

