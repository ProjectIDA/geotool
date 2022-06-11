/*
 * NAME
 * 
 *	compute_ltav
 *
 * FILE 
 *
 *	ltav.c
 *
 * SYNOPSIS
 *
 *	int
 *	compute_ltav  (stav, stav_state, npts, stav_len,
 *		       ltav_len, ltav_thresh, ltav, ltav_state)
 *	float	*stav;		(i) short term average vector
 *	int	*stav_state;	(i) state vector for stav
 *	int	npts;		(i) number of points in stav, stav_state
 *	int	stav_len;	(i) short term average length in samples
 *	int	ltav_len;	(i) desired long term average length in samples
 *	int	ltav_thresh;	(i) number of samples in a valid ltav window
 *	float	**ltav;		(o) long term average vector
 *	int	**ltav_state;	(o) state vector for ltav
 *
 * DESCRIPTION
 *
 *	compute_ltav() computes a recursive average of length ltav_len
 *	on the input short-term average stav using 
 *	
 *		ltav[i] = ltav[i-1] + (stav[i-N] - ltav[i-1]) / M
 *
 *	for i=[N/2,npts-N/2], N=stav_len, M=ltav_len.
 *
 *	The state vector is computed along with the average.
 *	The ltav and ltav_state vectors are allocated and returned.
 *	Returns -1 on error, 0 otherwise.
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 *	sigpro(1) (file calcsnr.f)
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "libstring.h"       /* for mallocWarn  */
#include "aesir.h"
#include "libdetectP.h"
#include "logErrorMsg.h"



int
compute_ltav( float *stav, int *stav_state, int npts, int stav_len,
			  int ltav_len, int ltav_thresh, float **ltav, int **ltav_state )
{

	float *lp   = NULL;
	int	  *lstp = NULL;
	int	ret = -1;


	/* Initialize */
	*ltav = NULL;
	*ltav_state = NULL;
	

	/* Error checks */
	if (!stav || !stav_state || npts < 1 || stav_len < 1 || 
	    ltav_len < stav_len)
	{
		goto RETURN;
	}


	/* Allocate memory for ltav, ltav state */
	if(!(lp = (float *)mallocWarn( npts * sizeof(float))));
	memset ((void *) lp, 0, sizeof (float) * npts);
	if(!( lstp = (int *)mallocWarn( npts * sizeof(int))));
	memset ((void *) lstp, 0, sizeof (int) * npts);

	/* Compute a recursive average of the short term average */
	ret = recursive_avg( stav, stav_state, npts, 
			     stav_len, ltav_len, ltav_thresh, samex, lp, lstp );
	if (ret < 0)
	{
		logErrorMsg( DFX_VERBOSE0, "compute_ltav: Error in recursive_avg\n" );
		goto RETURN;
	}
	
	*ltav = lp;
	*ltav_state = lstp;

	ret = 0;
	
 RETURN:

	if (ret < 0)
	{
		FREE (lp);
		FREE (lstp);
	}
	
	return (ret);
}

