/*
 * NAME
 *	splint_deriv -- Return a cubic spline interpolated value & derivatives.

 * FILE
 *	splint_deriv.c

 * SYNOPSIS
 *	This routine return a cubic spline interpolated value and derivatives 
 *	from an array.

 * DESCRIPTION
 *	Function.  Given the arrays xa[0..n-1] and ya[0..n-1] which tabulate
 *	a function (with the xa[i]'s in order), and given the array 
 *	y2a[0..n-1], which is the output from subroutine spline(), and given
 *	a value of x, this routine returns a cubic-spline interpolated
 *	value of y.  Also returns 1st and 2nd derivatives of y.

 * DIAGNOSTICS
 *

 * FILES
 *

 * NOTES
 *	The correct position in the table is obtained by means of bi-section.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */

#include "config.h"
#include "stdio.h"
#include "libinterp.h"


void
splint_deriv (float *xa, float *ya, float *y2a, int n, float x, float *y,
	      float *dy, float *d2y)
{
	int	klo, khi, k;
	float	h, b, a;

	klo = 0;
	khi = n-1;
	while (khi-klo > 1)
	{
		k = (khi + klo) >> 1;
		if (xa[k] > x)
			khi = k;
		else
			klo = k;
	}	/* klo and khi now bracket the input value of x */

	h = xa[khi] - xa[klo];
	if (h == 0)
		printf ("Bad xa input to routine splint");

	a    = (xa[khi] - x)/h;
	b    = (x - xa[klo])/h;
	*y   = a*ya[klo] + b*ya[khi] + ((a*a*a - a)*y2a[klo] 
	       + (b*b*b - b)*y2a[khi])*(h*h)/6.0;
	*dy  = ((ya[khi]-ya[klo])/h) - (((3.0*a*a-1.0)*h*y2a[klo])/6.0)
	       + (((3.0*b*b-1.0)*h*y2a[khi])/6.0);
	*d2y = a*y2a[klo] + b*y2a[khi];
}

