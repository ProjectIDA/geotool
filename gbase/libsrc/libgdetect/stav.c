/*
 * NAME
 * 
 *	compute_stav
 *
 * FILE 
 *
 *	stav.c
 *
 * SYNOPSIS
 *
 *	int
 *	compute_stav (data, state, npts, method, meas, len, thresh, stav, 
 *		      stav_state)
 *	float	*data;		(i) data vector for computation
 *	int	*state;		(i) state of data vector
 *	int	npts;		(i) number of points in data, state
 *	char	*method;	(i) desired stav method ("recursive","sliding")
 *	char	*meas;		(i) desired stav measure ("square","absolute")
 *	int	len;		(i) desired short term average length samples
 *	int	thresh;		(i) number of samples in a valid stav window
 *	float	**stav;		(o) short term average vector
 *	int	**stav_state;	(o) state vector for stav
 *	
 * DESCRIPTION
 *
 *	compute_stav() computes an average of length stav_len
 *	on the input data.  If method is "recursive", then recursive_avg()
 *	is used, and if method is "sliding", then running_avg() is used.
 *	If meas is "square", the function applied to the average is squarex(),
 *	and if meas is "absolute", the function applied to the average is
 *	fabs().	The state vector is computed along with the average.
 *	The stav and stav_state vectors are allocated and returned.
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
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>	/* For fabs(),floor() */
#include "aesir.h"
#include "libaesir.h"
#include "libstring.h"  /* for mallocWarn  */
#include "libdetectP.h"	
#include "logErrorMsg.h"

/* formatted error messages */
char erreur[128];


int
compute_stav( float *data, int *state, int npts, char *method,  
			  char *meas, int len, int thresh, float **stav, int **stav_state)
{
	float	*sp = NULL;
	int	*stp = NULL;
	double	(*meas_fp)(double) = NULL;
	int	ret = -1;

	/* Initialize */
	*stav = NULL;
	*stav_state = NULL;

	/* Error checks */
	if (!data || !state || npts < 1 || !method || !meas || len < 1)
	{
		goto RETURN;
	}

	/* Allocate memory for stav, stav state */
	if(!(sp = (float *)mallocWarn( npts * sizeof(float))));
	memset ((void *) sp, 0, sizeof (float) * npts);
	if(!(stp = (int *)mallocWarn( npts * sizeof(int))));
	memset ((void *) stp, 0, sizeof (int) * npts);

	/* Retrieve the stav method function */
	if (STREQ (meas, "square"))
	{
		meas_fp = squarex;
	}
	else if (STREQ (meas, "absolute"))
	{
		meas_fp = fabs;
	}
	else
	{
	  snprintf( erreur, sizeof(erreur), "compute_stav: Bad stav measure: %s\n", meas);
	  logErrorMsg( DFX_VERBOSE0, erreur );
	  goto RETURN;
	}

	if (STREQ (method, "recursive"))
	{
		ret = recursive_avg (data, state, npts, 0, len, 
				     thresh, meas_fp, sp, stp);
	}
	else if (STREQ (method, "sliding"))
	{
		ret = running_avg (data, state, npts, len, thresh,
				   meas_fp, sp, stp);
	}
	else
	{
	  snprintf( erreur, sizeof(erreur), "compute_stav: Bad stav method: %s\n", method );
	  logErrorMsg( DFX_VERBOSE0, erreur );
	  goto RETURN;
	}
	if (ret < 0)
	{
	  logErrorMsg( DFX_VERBOSE0, "compute_stav: Error computing stav\n" );
	  goto RETURN;
	}
	
	*stav = sp;
	*stav_state = stp;
	
	ret = 0;
	
 RETURN:

	if (ret < 0)
	{
		FREE (sp);
		FREE (stp);
	}
	
	return (ret);
}
