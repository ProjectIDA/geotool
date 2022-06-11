/*
 * NAME
 *      nicex:	find nice numbers for labels
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fnan.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */

#include "libgmath.h"

/**
 * Return 'nice' evenly spaced numbers. Return evenly spaced numbers between
 * ax1 and ax2 for axis labeling, or contour values.
 * 
 * @param ax1 range limit.
 * @param ax2 range limit. ax2 can be greater than or less than ax1.
 * @param minlb the minimum number of numbers returned.
 * @param maxlb the maximum number of numbers returned.
 * @param nlab (output) the number of numbers returned.
 * @param x (output) the evenly spaced numbers.
 * @param ndigit (output) the total number of digits needed to distinguish \
 *			each label x[].
 * @param ndeci (output) the number digits to the right of the decimal needed \
 *			to distinguish each label x[].
 */
void
nicex(double ax1, double ax2, int minlb, int maxlb, int *nlab, double *x,
	int *ndigit, int *ndeci)
{
	int i, j, jinc, i1, i2, ishift, count;
	double x1, x2, xdif, xmin, xmax, f, ixmin, ixmax, xval;

	if(!finite(ax1) || !finite(ax2))
	{
		x[0] = ax1;
		x[1] = ax2;
		*nlab = 2;
		*ndeci = 8;
		*ndigit = 8;
		return;
	}
	/* Expand the range (ax1,ax2) just a little, in case x1 and/or x2 is
	 * very near to a nice number (eg. ax1 = 0., ax2 = .9999).
	 */
	if(ax2 > ax1)
	{
		f = .0001*(ax2 - ax1);
		x1 = ax1 - f;
		x2 = ax2 + f;
		xmin = x1;
		xmax = x2;
	}
	else
	{
		f = .0001*(ax1 - ax2);
		x1 = ax1 + f;
		x2 = ax2 - f;
		xmin = x2;
		xmax = x1;
	}
	xdif = xmax - xmin;
	if(xdif == 0.)
	{
		x[0] = xmin;
		*nlab = 1;
		*ndeci = 1;
		*ndigit = 6;
		return;
	}
	ishift = 0;
	while(xdif < 1.)
	{
		ishift++;
		xdif *= 10.;
		xmin *= 10.;
		xmax *= 10.;
	}
	if(ishift >= 13)
	{
		x[0] = ax1;
		x[1] = ax2;
		*nlab = 2;
		*ndeci = 13;
		*ndigit = 13;
		return;
	}
		
	while(xdif > 10.)
	{
		ishift--;
		xdif *= .1;
		xmin *= .1;
		xmax *= .1;
	}

	jinc = 1;

	count = 0;
	do
	{
		ixmin = ceil(xmin);
/*		ixmax = floor(xmax) + .5*jinc; */
		ixmax = floor(xmax) + .5;
		*nlab = 0;
		for(xval = ixmin; xval < ixmax && *nlab < maxlb+1; xval += jinc)
		{
			*nlab = *nlab + 1;
		}
		if(*nlab < minlb)
		{
			ishift++;
			xmin *= 10.;
			xmax *= 10.;
		}
		else if(*nlab > maxlb)
		{
			jinc++;
		}
	} while((*nlab < minlb || *nlab > maxlb) && ++count < 1000);

	if(count >= 1000) {
		/* ax1 and/or ax2 probably NaN */
		x[0] = ax1;
		x[1] = ax2;
		*nlab = 2;
		*ndeci = 8;
		*ndigit = 8;
		return;
	}

	f = pow(10.,(double)(-ishift));
	*nlab = 0;
	for(xval = ixmin; xval < ixmax && *nlab < maxlb+1; xval += jinc)
	{
		x[*nlab] = xval*f;
		*nlab = *nlab + 1;
	}

	*ndeci = (ishift > 0) ? ishift : 0;
	for(i1 = 0, xmin = fabs(x[0]); xmin > 1; i1++) xmin *= .1;
	for(i2 = 0, xmax = fabs(x[*nlab-1]); xmax > 1; i2++) xmax *= .1;
	i1 += ishift;
	i2 += ishift;
	*ndigit = (i1 > i2) ? i1:i2;

	if(x1*x2 < 0 && *nlab > 1)  /* force zero to be a label */
	{
		for(i = 0; i < *nlab && x[i] != 0.; i++);

		if(i == *nlab)  /* zero is not a label */
		{
			xdif = (x[1] - x[0]);
			i1 = abs((int)(x1/xdif));
			for(i = 0; i <= i1; i++)
			{
				x[i] = -(i1-i)*xdif;
			}
			i1 = abs((int)(x2/xdif));
			for(j = 1; j <= i1; j++, i++)
			{
				x[i] = j*xdif;
			}
			/* *nlab might be < minlb, but will never be > maxlb */
			*nlab = i;
		}
	}
	return;
}
