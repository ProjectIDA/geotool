/** \file fit2d.cpp
 *  \brief Defines a parabolic interpolation
 */
#include "config.h"
#include <stdio.h>
#include <math.h>

#include "FKData.h"


/* basis = n**i, for i = 0,1,2
 */
#define basis(n,i) ( i == 0 ? 1. : (i == 1 ? n : n * n) )

/** Calculates parabolic refinement of the estimated peak location from grid
 *  values. Returns the location of the refined maximum in coordinates relative
 *  to the center grid point (-1. < x < 1. and -1. < y < 1.)
 *  @param[in] h a 3x3 grid with the maximum value at the center.
 *  @param[out] x the x-offset from the center grid point.
 *  @param[out] y the y-offset from the center grid point.
 */
void FKData::fit2d(double h[3][3], double *x, double *y)
{
	int	i, j, m, n;
	double	a, b, c, d, e, denom, phi[3][3];
	       
	/*
	 * construct correlations 
	 */
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
	{
		phi[i][j] = 0.;

		for(m = -1; m <= 1; m++)
			for(n = -1; n <= 1; n++)
		{
			phi[i][j] += h[m+1][n+1]*basis(m,i)*basis(n,j);
		}
	}
	a = phi[2][0]/2. - phi[0][0]/3.;
	b = phi[1][1]/4.;
	c = phi[0][2]/2. - phi[0][0]/3.;
	d = phi[1][0]/6.;
	e = phi[0][1]/6.;
	denom = b*b - 4.*a*c;
	if(fabs(denom) < 1.e-08)
	{
		*x = 0.0;
		*y = 0.0;
	}
	else
	{
		*x = (2.*c*d - b*e)/denom;
		*y = (2.*a*e - b*d)/denom;
	}
	return;
}
