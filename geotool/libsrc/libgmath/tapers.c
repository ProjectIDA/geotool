#include "config.h"
#include <math.h>

#include "tapers.h"
#include "libstring.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define TWO_PI (2.*M_PI)


/** 
 *  Apply a Hanning taper.
 */
double
Taper_hann(float *x, int n)
{
	int i;
	double dt, taper, norm=0.;

	if(n > 1)
	{
	    dt = TWO_PI/(double)(n-1);
	    for(i = 0; i < n; i++)
	    {
		taper = .5*(1. - cos(i*dt));
		x[i] *= taper;
		norm += taper;
	    }
	    if(norm) norm = (double)n/norm;
	}
	return (norm) ? norm : 1.;
}

/** 
 *  Apply a Hamming taper.
 */
double
Taper_hamm(float *x, int n)
{
	int i;
	double dt, taper, norm=0.;

	if(n > 1)
	{
	    dt = TWO_PI/(double)(n-1);
	    for(i = 0; i < n; i++)
	    {
		taper = (.54 - .46*cos(i*dt));
		x[i] *= taper;
		norm += taper;
	    }
	    if(norm) norm = (double)n/norm;
	}
	return (norm) ? norm : 1.;
}

/** 
 *  Apply a welch taper.
 */
double
Taper_welch(float *x, int n)
{
	int i;
	double fac, taper, norm=0., numerator, denominator;

	if(n > 0)
	{
	    numerator = .5*(n-1);
	    denominator = .5*(n+1);
	    for(i = 0; i < n; i++)
	    {
		fac = ((i - numerator)/denominator);
		taper = (1. - (fac * fac));
		x[i] *= taper;
		norm += taper;
	    }
	    if(norm) norm = (double)n/norm;
	}
	return (norm) ? norm : 1.;
}

/** 
 *  Apply a blackman taper.
 */
double
Taper_blackman(float *x, int n)
{
	int i;
	double taper, norm=0.;

	if(n > 0)
	{
	    for(i = 0; i < n; i++)
	    {
	        taper = (.42 - .5 * cos(i * TWO_PI/ (n - 1)) +
                       .08 * cos(i * 2. * TWO_PI/ (n - 1)));
		x[i] *= taper;
		norm += taper;
	    }
	    if(norm) norm = (double)n/norm;
	}
	return (norm) ? norm : 1.;
}

/** 
 *  Apply a Parzen taper.
 */
double
Taper_parzen(float *x, int n)
{
	int i;
	double half_n, fac, taper, norm=0.;

	if(n > 0)
	{
	    half_n = .5*n;
	    fac = 2./(n+1);
	    for(i = 0; i < n; i++)
	    {
		taper = (1. - fac*fabs((double)i - half_n));
		x[i] *= taper;
		norm += taper;
	    }
	    if(norm) norm = (double)n/norm;
	}
	return (norm) ? norm : 1.;
}

/** 
 *  Apply a cosine taper.
 *  @param x The data to be tapered.
 *  @param n The number of samples in data[].
 *  @param beg_len The length of the taper at the beginning of the data as a \
 *		   percentage of n.
 *  @param end_len The length of the taper at the end of the data as a \
 *		   percentage of n.
 */
double
Taper_cosine(float *x, int n, double beg_len, double end_len)
{
	int i, len;
	double arg, taper, unitArea=0., norm=0.;

	len = (int)(beg_len*n + .5);
	if(len > n) len = n;
	if(len > 0)
	{
	    arg = M_PI/(double)len;
	    for(i = 0; i < len; i++)
	    {
		taper = .5*(1. - cos(i*arg));
		x[i] *= taper;
		norm += taper;
		unitArea += 1.;
	    }
	}
	len = (int)(end_len*n + .5);
	if(len > n) len = n;
	if(len > 0)
	{
	    arg = M_PI/(double)len;
	    for(i = 0; i < len; i++)
	    {
		taper = .5*(1. - cos(i*arg));
		x[n-1-i] *= taper;
		norm += taper;
		unitArea += 1.;
	    }
	}
	if(norm) return unitArea/norm;
	return 1.;
}

