/*
 * NAME
 * 
 *	find_peak_trough ()
 *
 * FILE 
 *
 *	find_peak_trough.c
 *
 * SYNOPSIS
 *
 *	int
 *	find_peak_trough (beam, npts, pts, maxdiff)
 *	float 	*beam;		(i)		
 *	int	npts;		(i)
 *	int	*pts;		(i/o)
 *	double	*maxdiff;	(o)
 *
 * DESCRIPTION
 *
 *	 Identify all peak and trough indices in a beam.
 *
 * DIAGNOSTICS
 *
 *	Returns OK if successful, ERR otherwise.
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
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

/* Local functions forward declaration */
static void set_peak_trough( float *beam, int npts, int *pts, int i );

int
find_peak_trough( float *beam, int npts, int *pts, double *maxdiff )
{
	int	maxpt = 0;     	/* Index of highest point in beam */
	int	minpt = 0;     	/* Index of lowest point in beam */
	int	i;		

	/* Check for error conditions */
	if (beam == NULL || pts == NULL || npts <= 0 ) 
	{
		return ERR;
	}

	/* Initialize */
	*maxdiff = 0.0;

	/* 
	 * Assign markers to all points in beam 
	 * By definition, first and last points cannot be peaks/troughs
	 */
	pts[0] = NEITHER;
	set_peak_trough (beam, npts, pts, 1);
	for (i = 2; i < npts; i++)      
	{
		set_peak_trough (beam, npts, pts, i);

		/* Save min,max points in beam for threshold determination */
		if (beam[i] > beam[maxpt])
		{
			maxpt = i;
		}
		else if (beam[i] < beam[minpt])  
		{
			minpt = i;
		}
		
	}
	pts[0] = NEITHER;
	pts[npts - 1] = NEITHER;
	
    	*maxdiff = fabs (beam[maxpt] - beam[minpt]);

        return OK;
}


static
void
set_peak_trough( float *beam, int npts, int *pts, int i )
{
	int	im1;
	im1 = i - 1;
	
	if (beam[i] > beam[im1])
	{
		pts[i] = PEAK;
		
		if (pts[im1] == PEAK)
		{
			pts[im1] = NEITHER; 
		}
		else if (pts[im1] == NEITHER)
		{
			pts[im1] = TROUGH;
		}
	}
	else if (beam[i] < beam[im1])
	{
		pts[i] = TROUGH;
		
		if (pts[im1] == TROUGH)
		{
			pts[im1] = NEITHER; 
		}
		else if (pts[im1] == NEITHER)
		{
			pts[im1] = PEAK; 
		}
	}
	else
	{
		pts[i] = NEITHER;
	}

	return;
}


