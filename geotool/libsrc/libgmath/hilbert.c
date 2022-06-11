#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_GSL
#include "gsl/gsl_fft_real.h"
#include "gsl/gsl_fft_halfcomplex.h"
#endif
#include "libgmath.h"

/**
 * Hilbert tranformation.
 */

int
Hilbert_data(int npts, float *data)
{
#ifdef HAVE_GSL
	int	i, np2, n2;
	double	re, im, *x=NULL;

	for(np2 = 2; np2 < npts; np2 *= 2);
	n2 = np2/2;

	if( !(x = (double *)malloc(np2*sizeof(double))) ) return -1;

	for(i = 0; i < npts; i++) x[i] = data[i];
	for(i = npts; i < np2; i++) x[i] = 0.;

	gsl_fft_real_radix2_transform(x, 1, np2);

	x[0] = x[np2-1] = 0.;
	for(i = 1; i < n2; i++) {
	    re = x[i];
	    im = x[np2-1-i];
	    x[i] = im;
	    x[np2-1-i] = -re;
	}

	gsl_fft_halfcomplex_radix2_inverse(x, 1, np2);

	for(i = 0; i < npts; i++) data[i] = x[i];

	if(x) free(x);

	return 0;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return -1;
#endif
}
