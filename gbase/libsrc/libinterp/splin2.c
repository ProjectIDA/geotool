/*
 * NAME
 *	splin2 -- Return values of a bi-cubic interpolated function.

 * FILE
 *	splin2.c

 * SYNOPSIS
 *	Return an interpolated function value by bi-cubic interpolation.

 * DESCRIPTION
 *	Function.  Given xs, xf, ysf, nslow, nfast as described in function 
 *	splie2() and y2sf as produced by that routine; and given a desired
 *	interpolating point rxs, rxf; this routine returns an interpolated 
 *	function value rysf by bi-cubic spline interpolation.  The 
 *	complemetary routine, splie2(), needs to be called once prior to 
 *	accessing this function to initializize natural splines and 2nd 
 *	derivatives.

 *	---- Functions called ----
 *		spline:	Return 2nd derivatives of an interpolating function
 *		splint:	Return a cubic spline interpolated value

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	The above is accomplished by constructing row, then column splines,
 *	one-dimension at a time.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */


#include "config.h"
#include "aesir.h"
#include "libinterp.h"

/* xs: Independent variable for slow index into matrix
 * xf: Independent variable for fast index into matrix
 * ysf: 2-D tabulated function of independent variables (xs, xf) with xf the
 *	fast index in the array (generally the last index in C)
 * y2sf: 2nd derivative w.r.t. fast index independent variable for all points
 *	in ysf
 * rxs: Interpolation point for slow index
 * rxf: Interpolation point for fast index
 * rysf: Interpolated value for point (rxs,rxf)
 * drysf: 1st derivative at value, rysf
 * d2rysf: 2nd derivative at value, rysf
 * nslow: Slow index in 2-D array
 * nfast: Fast index in 2-D array (accesses data memory sequentially)
 */
void
splin2 (float *xs, float *xf, float **ysf, float **y2sf,
	     int nslow, int nfast, float rxs, float rxf,
	     float *rysf, float *drysf, float *d2rysf)
{
	int	j;
	float	*ytmp, *y2tmp;

	if (nfast > nslow)
	{
		ytmp  = UALLOCA (float, nfast);
		y2tmp = UALLOCA (float, nfast);
	}
	else
	{
		ytmp  = UALLOCA (float, nslow);
		y2tmp = UALLOCA (float, nslow);
	}

	for (j = 0; j < nslow; j++)
		splint (xf, ysf[j], y2sf[j], nfast, rxf, &y2tmp[j]);
	spline (xs, y2tmp, nslow, 1.0e30, 1.0e30, ytmp);
	splint_deriv (xs, y2tmp, ytmp, nslow, rxs, rysf, drysf, d2rysf);
}

