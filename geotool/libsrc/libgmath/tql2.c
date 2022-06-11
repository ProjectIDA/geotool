#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libgmath.h"

#define sign(a,b)	((b >= 0.) ? fabs(a) : -fabs(a))

/** 
 * Compute eigenvalues and eigenvectors of a symmetric tridiagonal matrix.
 * <p>
 * From Lecture Notes in Computer Science
 * Matrix Eigensystem Routines - EISPACK Guide, B. T. Smith et. al
 * (Springer-Verlag 1974)
 * <pre>
 *	int n 		dimension of the symmetric tridiagonal matrix
 *	double *d 	(i) diagonal elements of the tridiagonal matrix.
 *			(o) eigenvalues
 *	double *e 	e[0] is not used, e[1] to e[n-1] = subdiagonal.
 *	double *z 	n by n transformation matrix to tridiagonal form
 * </pre>
 *  @return 0 for success, nonzero for error.
 */
int
tql2(int n, double *d, double *e, double *z)

{
	int i, j, k, l, m, ii, l1;
	double b, c, f, g, h, p, r, s, machep;
/*
 * machep is a machine dependent parameter specifying
 * the relative precision of floating point arithmetic.
 *
 *	machine epsilon can be found for a particular system by the
 *	following algorithm:  (Forsythe, Computer Methods for Mathe-
 *	matical Computatons, Prentice-Hall 1977)
 *		double machep, mach1;
 *		machep = 1.0;
 *		do
 *		{
 *			machep *= 0.5;
 *			mach1 = machep + 1.0;
 *		} while(mach1 > 1.0);
 */
	machep = .1e-16;

	if(n == 1)
	{
		return(0);
	}

	for(i = 1; i < n; i++)
	{
		e[i-1] = e[i];
	}
	f = 0.0;
	b = 0.0;
	e[n-1] = 0.0;

	for(l = 0; l < n; l++)
	{
		j = 0;
		h = machep * (fabs(d[l]) + fabs(e[l]));
		if(b < h) b = h;
		/*
		 * look for small sub-diagonal element.
		 * e[n-1] is always zero, so there is no exit
		 * through the bottom of the loop.
		 */
		for(m = l; m < n && fabs(e[m]) > b; m++);

		if(m != l) do
		{
			if(j++ >= 30)
			{
				/* set error -- no convergence to an
				 * eigenvalue after 30 iterations 
				 */
				return(l);
			}
			/* form shift */
			l1 = l + 1;
			g = d[l];
			p = (d[l1] - g) / (2.0 * e[l]);
			r = sqrt(p*p+1.0);
			d[l] = e[l]/(p + sign(r, p));
			h = g - d[l];
			for(i = l1; i < n; i++)
			{
				d[i] -= h;
			}
			f += h;
			/*
			 * ql transformation
			 */
			p = d[m];
			c = 1.0;
			s = 0.0;
			for(i = m-1; i >= l; i--)
			{
				g = c * e[i];
				h = c * p;
				if(fabs(p) >= fabs(e[i]))
				{
					c = e[i]/p;
					r = sqrt(c*c+1.0);
					e[i+1] = s*p*r;
					s = c/r;
					c = 1.0/r;
				}
				else
				{
					c = p/e[i];
					r = sqrt(c*c+1.0);
					e[i+1] = s * e[i] * r;
					s = 1.0/r;
					c = c * s;
				}
				p = c * d[i] - s * g;
				d[i+1] = h + s * (c * g + s * d[i]);
				/* form vector */
				for(k = 0; k < n; k++)
				{
					h = z[k+(i+1)*n];
					z[k+(i+1)*n] = s*z[k+i*n] + c*h;
					z[k+i*n] = c*z[k+i*n] - s*h;
				}
			}
			e[l] = s * p;
			d[l] = c * p;
		} while(fabs(e[l]) > b);
		d[l] += f;
	}
	/*
	 * order eigenvalues and eigenvectors
	 */
	for(ii = 1; ii < n; ii++)
	{
		i = ii - 1;
		k = i;
		p = d[i];
		for(j = ii; j < n; j++)
		{
			if(d[j] < p)
			{
				k = j;
				p = d[j];
			}
		}
		if(k != i)
		{
			d[k] = d[i];
			d[i] = p;
			for(j = 0; j < n; j++)
			{
				p = z[j+i*n];
				z[j+i*n] = z[j+k*n];
				z[j+k*n] = p;
			}
		}
	}
	return(0);
}
