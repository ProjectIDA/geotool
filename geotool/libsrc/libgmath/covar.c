#include "config.h"
#include "libgmath.h"

/*
 * NAME
 *      covar:	covariance matrix
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

/**
 * Compute the covariance matrix for 3 components.
 * <pre>
 * Input *e, *n, *z, all length n.
 * Output s = 3 by 3 symmetric matrix s[i+j*3].
 * 
 * e = x, n = y, z = z;
 *
 *	|ee en ez|
 * s = 	|ne nn nz|
 *	|ze zn zz|
 * </pre>
 */

/** 
 * Compute the covariance matrix for 3 components.
 */
void
covar(int npts, float *e, float *n, float *z, double *s)
{
	int i;
	double sum;

	if(npts <= 0) return;

	for(i = 0, sum = 0.; i < npts; i++)
	{
		sum += e[i]*e[i];
	}
	s[0] = sum/npts;
	for(i = 0, sum = 0.; i < npts; i++)
	{
		sum += n[i]*n[i];
	}
	s[4] = sum/npts;
	for(i = 0, sum = 0.; i < npts; i++)
	{
		sum += z[i]*z[i];
	}
	s[8] = sum/npts;

	for(i = 0, sum = 0.; i < npts; i++)
	{
		sum += e[i]*n[i];
	}
	s[1] = sum/npts;
	for(i = 0, sum = 0.; i < npts; i++)
	{
		sum += e[i]*z[i];
	}
	s[2] = sum/npts;
	for(i = 0, sum = 0.; i < npts; i++)
	{
		sum += n[i]*z[i];
	}
	s[5] = sum/npts;

	s[3] = s[1];
	s[6] = s[2];
	s[7] = s[5];
	return;
}
