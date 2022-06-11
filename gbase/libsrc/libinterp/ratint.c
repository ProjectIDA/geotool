
/*
 * NAME
 *	ratint -- Rational function interpolating function routine.

 * FILE
 *	ratint.c

 * SYNOPSIS
 *	A rational function is constructed so as to go through a chosen
 *	set of tabulated function values.

 * DESCRIPTION
 *	Function.  Given arrays xa[0..n-1] and ya[0..n-1], and given a 
 *	value, x, this routine returns a value of y and an accuracy 
 *	estimate, dy.  The value returned is that of the diagonal rational 
 *	function, evaluated at x, which passes through the n points 
 *	(xa[i], ya[i]), i = 0..n-1.

 * DIAGNOSTICS
 *	An error will be return if an interpolating function has a pole at
 *	the requested value of x.

 * FILES
 *	None.

 * NOTES
 *	Note that this routine only need be called once to process the
 *	entire tabulated function in x and y arrays.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.  Use in a
 *	analogous way to the routine, polint, page 90.

 * AUTHOR
 *
 */

#include "config.h"
#include <math.h>
#include "libinterp.h"

#ifdef	__svr4__
# include <float.h>
#else
# define  FLT_EPSILON	1.0E-06
#endif

int 
ratint (float *xa, float *ya, int n, float x, float *y, float *dy)
{
	int	i, m, ns;
	double	*c, *d, dd, h, hh, t, w;

	c  = UALLOCA (double, n);
	d  = UALLOCA (double, n);
	hh = fabs(x - xa[0]);

	for (ns = 0, i = 0; i < n; i++)
	{
		h = fabs(x - xa[i]);
		if (h == 0.0)
		{
			*y  = ya[i];
			*dy = 0.0;
			return (0);
		}
		else if (h < hh)
		{
			ns = i;
			hh = h;
		}
		c[i] = ya[i];
		d[i] = ya[i] + FLT_EPSILON; /* Needed to prevent a rare */
	}				    /* zero-over-zero condition */

	*y = ya[ns--];

	for (m = 0; m < n-1; m++)
	{
		for (i = 0; i < n-1-m; i++)
		{
			w  = c[i+1] - d[i];
			h  = xa[i+m+1] - x;
			t  = (xa[i] - x) * d[i]/h;
			dd = t - c[i+1];
			if (dd == 0.0)
				/* Interpolating function has a pole at the
				   requested value of x.  Return error */
				return (-1);
			dd = w/dd;
			d[i] = c[i+1] * dd;
			c[i] = t * dd;
		}
		*y += (*dy = (2*(ns+1) < (n-m-1) ? c[ns+1] : d[ns--]) );
	}
	return (0);
}

