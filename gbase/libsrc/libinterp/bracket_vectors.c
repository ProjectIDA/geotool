
/*
 * NAMES
 *	brack_one_vector -- Find left index of given value in 1-D float array.
 *	brack_one_dvector -- Find left index of given value in 1-D double array.

 * FILE
 *	bracket_vectors.c

 * SYNOPSIS
 *	brack_one_vector (float *xx, int n, float x, int *j)
 *	brack_one_dvector (double *xx, int n, double x, int *j)

 * DESCRIPTION
 *	Functions.  These 2 complementary function, take a given an float or
 *	double array xx[0...n-1], and a given value x, and returns a value j, 
 *	such that x is between xx[j] and xx[j+1].  xx must be monotonic, 
 *	either increasing or decreasing.

 * DIAGNOSTICS
 *	 j = -1 or j = n-1 is returned to indicate that x is out of range.

 * FILES
 *	None.

 * NOTES
 *	Borrowed from Numerical Recipes in C routine, locate().

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 98-99.

 * AUTHOR
 *	Indexed from 0, n-1 and prototyped by W. Nagy.
 */

#include "config.h"
#include "libinterp.h"

void
brack_one_vector (float *xx, int n, float x, int *j)
{

	int	ascnd, ju, jm, jl;

	/* Initialize lower and upper limits */

	jl = -1;
	ju = n;

	ascnd = xx[n-1] > xx[0];
	while (ju-jl > 1)
	{
		jm = (ju+jl) >> 1;		/* Compute a midpoint */
		if ((x > xx[jm]) == ascnd)
			jl = jm;		/* Replace a lower limit */
		else
			ju = jm;		/* Replace an upper limit */
	}
	*j = jl;
}


void
#ifdef __STDC__
brack_one_dvector (double *xx, int n, double x, int *j)
#else
brack_one_dvector (xx, n, x, j)
int	*j, n;
double	x, *xx;
#endif

{

	int	ascnd, ju, jm, jl;

	/* Initialize lower and upper limits */

	jl = -1;
	ju = n;

	ascnd = xx[n-1] > xx[0];
	while (ju-jl > 1)
	{
		jm = (ju+jl) >> 1;		/* Compute a midpoint */
		if ((x > xx[jm]) == ascnd)
			jl = jm;		/* Replace a lower limit */
		else
			ju = jm;		/* Replace an upper limit */
	}
	*j = jl;
}

