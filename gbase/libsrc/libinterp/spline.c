/*
 * NAME
 *	spline -- Return the second derivatives of an interpolating function.

 * FILE
 *	spline.c

 * SYNOPSIS
 *	Construct an array of second derivatives based on an interpolating
 *	function at given tabulated points.

 * DESCRIPTION
 *	Function.  Given arrays x[0..n-1] and y[0..n-1] containing a 
 *	tabulated function, i.e., y[i] = f(x[i]), with x[0] < x[1] < x[n-1],
 *	and given values yp1 and ypn for the first derivative of the 
 *	interpolating function at points 0 and n-1, respectively, this 
 *	routine returns an array y2[0..n-1] that contains the 2nd i
 *	derivatives of the interpolating function at the tabulated points 
 *	x[i].  

 * DIAGNOSTICS
 *	If yp1 and/or ypn are equal to 1.0e30 or larger, the routine is 
 *	signalled to set the corresponding boundary condition for a natural 
 *	spline, with zero second derivative on that boundary.

 * FILES
 *

 * NOTES
 *	Note that this routine only need be called once to process the
 *	entire tabulated function in x and y arrays.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */


#include "config.h"
#include "aesir.h"
#include "libinterp.h"

void
spline (float *x, float *y, int n, float yp1, float ypn, float *y2)
{
	int	i, k;
	float	p, qn, sig, un, *u;

	u = UALLOCA (float, n);

	if (yp1 > 0.99e30)
		y2[0] = u[0] = 0.0;
	else
	{
		y2[0] = -0.5;
		u[0]  = (3.0/(x[1]-x[0])) * ((y[1]-y[0])/(x[1]-x[0])-yp1);
	}

	/* Decomposition loop for tridiagonal algorithm */

	for (i = 1; i < n-1; i++)
	{
		sig   = (x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p     = sig*y2[i-1] + 2.0;
		y2[i] = (sig-1.0)/p;
		u[i]  = (y[i+1]-y[i])/(x[i+1]-x[i]) 
			- (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]  = (6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	if (ypn > 0.99e30)
		qn = un = 0.0;
	else
	{
		qn = 0.5;
		un = (3.0/(x[n-1] - x[n-2]))*(ypn - (y[n-1] - y[n-2])/
		     (x[n-1] - x[n-2]));
	}
	y2[n-1] = (un - qn*u[n-2])/(qn*y2[n-2] + 1.0);

	/* Back substituition loop of tridiagonal algorithm */

	for (k = n-2; k >= 0; k--)
		y2[k] = y2[k]*y2[k+1] + u[k];
}

