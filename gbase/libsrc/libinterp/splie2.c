/*
 * NAME
 *      splie2 -- Construct natural splines and second derivatives.

 * FILE
 *      splie2.c

 * SYNOPSIS
 *      Perform 1-D natural cubic splines on rows and return the second
 *	derivatives.
 
 * DESCRIPTION
 *      Function.  Given a tabulated function ysf[0..nslow-1][0..nfast-1],
 *	and tabulated independent variables xs[0..nslow-1] and xf[0..nfast-1], 
 *	this routine constructs one-dimensional natural cubic splines of
 *	the rows of ysf and returns the second derivatives in the array 
 *	y2sf[0..nslow-1][0..nfast-1].  This routine only needs to be called
 *	once, and then, any number of bi-cubic spline interpolations can
 *	be performed by successive calls to splin2.

 *	---- Functions called ----
 *		spline:	Return 2nd derivatives of an interpolating function

 * DIAGNOSTICS
 *	Values returned larger than 1.0e30 signal a natual spline.

 * FILES
 *	None.

 * NOTES
 *	See variable descriptions below.

 * SEE ALSO
 *      Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */

#include "config.h"
#include "libinterp.h"

/* xs: Independent variable for slow index into matrix 
 * xf: Independent variable for fast index into matrix
 * ysf: 2-D tabulated function of independent variables (xs, xf) with xf the
 * 	fast index in the array
 * (generally the last index in C)
 * nslow: Slow index in 2-D array
 * nfast: Fast index in 2-D array (accesses data memory sequentially)
 * y2sf: 2nd derivative w.r.t. fast index independent variable for all points
 * in ysf
 */
void
splie2 (float *xs, float *xf, float **ysf, int nslow, int nfast, float **y2sf)
{
	int	j;

	for (j = 0; j < nslow; j++)
		spline (xf, ysf[j], nfast, 1.0e30, 1.0e30, y2sf[j]);
}
