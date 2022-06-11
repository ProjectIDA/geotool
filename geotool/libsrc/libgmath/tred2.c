#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libgmath.h"

/** 
 *  Reduce a real symmetric matix to a tridiagonal matrix.
 *  <p>
 *  Copied from Lecture Notes in Computer Science
 *  Matrix Eigensystem Routines - EISPACK Guide, B. T. Smith et. al
 *  (Springer-Verlag 1974)
 *  @param n	dimension of matrix
 *  @param a	n by n real symmetric matrix a[i+j*n]
 *  @param d	n diagonal elememts of the output tridiagonal
 *  @param e	e[0]=0., e[1] to e[n-1] are subdiagonal elements
 *  @param z	n by n orthogonal transformation matrix
 */
void
tred2(int n, double *a, double *d, double *e, double *z)
{
	int i, j, k, l;
	double f, g, h, hh, scale;

	if(n <= 0)
	{
		return;
	}
	memcpy(z, a, n*n*sizeof(double));

	for(i = n-1; i > 1; i--)
	{
		l = i - 1;
		h = 0.0;
		scale = 0.0;
		/* scale row (algol tol then not needed) */
		for(k = 0; k <= l; k++)
		{
			scale += fabs(z[i+k*n]);
		}
		if(scale == 0.)
		{
			e[i] = z[i+l*n];
			d[i] = 0.;
		}
		else
		{
			for(k = 0; k <= l; k++)
			{
				z[i+k*n] /= scale;
				h += z[i+k*n]*z[i+k*n];
			}
			f = z[i+l*n];
			g = (f >= 0.) ? -sqrt(h) : sqrt(h);
			e[i] = scale * g;
			h = h - f * g;
			z[i+l*n] = f - g;
			f = 0.0;
			for(j = 0; j <= l; j++)
			{
				z[j+i*n] = z[i+j*n] / (scale * h);
				g = 0.0;
				/* form element of a*u */
				for(k = 0; k <= j; k++)
				{
					g += z[j+k*n] * z[i+k*n];
				}
				for(k = j+1; k <= l; k++)
				{
					g +=  z[k+j*n] * z[i+k*n];
				}
				/* form element of p */
				e[j] = g/h;
				f += e[j] * z[i+j*n];
			}
			hh = f/(h + h);
			/* form reduced a */
			for(j = 0; j <= l; j++)
			{
				f = z[i+j*n];
				g = e[j] - hh * f;
				e[j] = g;
				for(k = 0; k <= j; k++)
				{
					z[j+k*n] -= (f*e[k]+g*z[i+k*n]);
				}
			}
			for(k = 0; k <= l; k++)
			{
				z[i+k*n] *= scale;
			}
			d[i] = h;
		}
	}
	d[0] = 0.0;
	e[0] = 0.0;
	if(n > 1)
	{
		e[1] = z[1];
		d[1] = 0.;
	}
	/*
	 * accumulation of transformation matrices
	 */
	for(i = 0; i < n; i++)
	{
		l = i - 1;
		if(d[i] != 0.0)
		{
			for(j = 0; j <= l; j++)
			{
				for(k = 0, g = 0.0; k <= l; k++)
				{
					g += z[i+k*n] * z[k+j*n];
				}
				for(k = 0; k <= l; k++)
				{
					z[k+j*n] -= g*z[k+i*n];
				}
			}
		}
		d[i] = z[i+i*n];
		z[i+i*n] = 1.0;
		for(j = 0; j <= l; j++)
		{
			z[i+j*n] = z[j+i*n] = 0.0;
		}
	}
	return;
}
