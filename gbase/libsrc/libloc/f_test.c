
/*
 * Copyright (c) 1991-1996 Science Applications International Corporation.
 *

 * NAME
 *	f_test -- F-distribution test.

 * FILE
 *	f_test.c

 * SYNOPSIS
 *	void 
 *	f_test (m, n, p, x)
 *	int	m;	(i) Number of parameters (must be 1, 2, or 3)
 *	int	n;	(i) Degrees of freedom (must be greater than 1)
 *	double	p;	(i) Confidence level (must be 0.80, 0.90, 0.95 or 0.99)
 *	double	*x;	(o) F(x) = P for m parameters and n degrees of freedom

 * DESCRIPTION
 *	Function.  Make an Snedecor's F-distribution test for M parameters
 *	and N degrees of freedom equal to the given confidence limit.

 * DIAGNOSTICS
 *	Only allowable confidence limits are 80%, 90%, 95% and 99%.

 * FILES
 *	None.

 * NOTES
 *	ns[] represents the number of degrees of freedom in the our 
 *	denominator expression.  xs[] represents the same further sub-
 *	divided by the number of parameters (1, 2 or 3) and the confidence
 *	level (0.80, 0.90, 0.95 or 0.99).  x is done with a crude table
 *	and interpolation and is accurate only to about +/- 0.01.

 * SEE ALSO
 *	Tables are copied from Biometrika Tables for Statistics, Vol. 1,
 *	E.S. Pearson and H.O. Hartley, eds., Biometrika Trust, London, 1976.

 * AUTHORS
 *	Walter Nagy,  2/91,	Created.
 *	Walter Nagy,  7/92,	Expanded table values provided by David 
 *				Taylor, ENSCO.
 *	Walter Nagy, 11/18/93,	Added 80th percentile ellipse table for 
 *				computing confidence regions. 
 */

#include "config.h"
#include <math.h>
#include "locp.h"

#define	MAX_TABLE	34
#define	MAX_P		3

void 
f_test (int m, int n, double p, double *x)
{

	int	i;
	int	ip = 1, j = 0;
	double	an, an1, an2, dp, x1, x2, y ,y1, y2;

	static	int	ns[] = {  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
				 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
				 23, 24, 25, 26, 27, 28, 29, 30, 40, 60,
				120, 99999 };
	static	double	ps[] = { 0.80, 0.90, 0.95, 0.99 };
	static	double	xs[][MAX_P][MAX_TABLE] =
	{

				/* p = 80th percentile */
	    { { 9.47,  3.56,  2.68,  2.35,  2.18,  2.07,  2.00,  1.95,  1.91,
		1.88,  1.86,  1.84,  1.82,  1.81,  1.80,  1.79,  1.78,  1.77, 
		1.763, 1.757, 1.751, 1.746, 1.741, 1.737, 1.733, 1.729, 1.726,
		1.723, 1.720, 1.717, 1.698, 1.679, 1.661, 1.642},

	      {12.00,  4.00,  2.89,  2.47,  2.26,  2.13,  2.04,  1.98,  1.93,
		1.90,  1.87,  1.85,  1.83,  1.81,  1.80,  1.78,  1.77,  1.76,
		1.754, 1.746, 1.739, 1.733, 1.728, 1.722, 1.718, 1.713, 1.709,
		1.706, 1.702, 1.699, 1.676, 1.653, 1.631, 1.609},

	      {13.06,  4.16,  2.94,  2.48,  2.25,  2.11,  2.02,  1.95,  1.90,
		1.86,  1.83,  1.80,  1.78,  1.76,  1.75,  1.74,  1.72,  1.71,
		1.704, 1.696, 1.688, 1.682, 1.676, 1.670, 1.665, 1.660,
		1.656, 1.652, 1.648, 1.645, 1.620, 1.595, 1.571, 1.547}},

				/* p = 90th percentile */
	    { {39.86,  8.53,  5.54,  4.54,  4.06,  3.78,  3.59,  3.46,  3.36,
		3.29,  3.23,  3.18,  3.14,  3.10,  3.07,  3.05,  3.03,  3.01, 
		2.99,  2.97,  2.96,  2.95,  2.94,  2.93,  2.92,  2.91,  2.90,
		2.89,  2.89,  2.88,  2.84,  2.79,  2.75,  2.71},

	      {49.50,  9.00,  5.46,  4.32,  3.78,  3.46,  3.26,  3.11,  3.01,
		2.92,  2.86,  2.81,  2.76,  2.73,  2.70,  2.67,  2.64,  2.62,
		2.61,  2.59,  2.57,  2.56,  2.55,  2.54,  2.53,  2.52,  2.51,
		2.50,  2.50,  2.49,  2.44,  2.39,  2.35,  2.30},

	      {53.59,  9.16,  5.39,  4.19,  3.62,  3.29,  3.07,  2.92,  2.81,
		2.73,  2.66,  2.61,  2.56,  2.52,  2.49,  2.46,  2.44,  2.42,
		2.40,  2.38,  2.36,  2.35,  2.34,  2.33,  2.32,  2.31,  2.30,
		2.29,  2.28,  2.28,  2.23,  2.18,  2.13,  2.08}},

				/* p = 95th percentile */
	    {{161.4,  18.51, 10.13,  7.71,  6.61,  5.99,  5.59,  5.32,  5.12,
		4.96,  4.84,  4.75,  4.67,  4.60,  4.54,  4.49,  4.45,  4.41,
		4.38,  4.35,  4.32,  4.30,  4.28,  4.26,  4.24,  4.23,  4.21,
		4.20,  4.18,  4.17,  4.08,  4.00,  3.92,  3.84},

	     {199.5,  19.00,  9.55,  6.94,  5.79,  5.14,  4.74,  4.46,  4.26,
		4.10,  3.98,  3.89,  3.81,  3.74,  3.68,  3.63,  3.59,  3.55,
		3.52,  3.49,  3.47,  3.44,  3.42,  3.40,  3.39,  3.37,  3.35,
		3.34,  3.33,  3.32,  3.23,  3.15,  3.07,  3.00},

	     {215.7,  19.16,  9.28,  6.59,  5.41,  4.76,  4.35,  4.07,  3.86,
		3.71,  3.59,  3.49,  3.41,  3.34,  3.29,  3.24,  3.20,  3.16,
		3.13,  3.10,  3.07,  3.05,  3.03,  3.01,  2.99,  2.98,  2.96,
		2.95,  2.93,  2.92,  2.84,  2.76,  2.68,  2.60}},

				/* p = 99th percentile */
	   {{4052.0,  98.50, 34.12, 21.20, 16.26, 13.75, 12.25, 11.26, 10.56,
	       10.04,  9.65,  9.33,  9.07,  8.86,  8.68,  8.53,  8.40,  8.29,
		8.18,  8.10,  8.02,  7.95,  7.88,  7.82,  7.77,  7.72,  7.68,
		7.64,  7.60,  7.56,  7.31,  7.08,  6.85,  6.63},

	    {4999.5,  99.00, 30.82, 18.00, 13.27, 10.92,  9.55,  8.65,  8.02,
		7.56,  7.21,  6.93,  6.70,  6.51,  6.36,  6.23,  6.11,  6.01,
		5.93,  5.85,  5.78,  5.72,  5.66,  5.61,  5.57,  5.53,  5.49,
		5.45,  5.42,  5.39,  5.18,  4.98,  4.79,  4.61},

	    {5403.0,  99.17, 29.46, 16.69, 12.06,  9.78,  8.45,  7.59,  6.99,
		6.55,  6.22,  5.95,  5.74,  5.56,  5.42,  5.29,  5.18,  5.09,
		5.01,  4.94,  4.87,  4.82,  4.76,  4.72,  4.68,  4.64,  4.60,
		4.57,  4.54,  4.51,  4.31,  4.13,  3.95,  3.78}},
	};
 
	*x = 0.0;
	if (m < 1 || m > MAX_P)
	    return;
	if (n < 1)
	{
	    *x = 1000.0; 
	    return;
	}
	for (i = 0; i < 4; i++)
	{
	    dp = fabs(p-ps[i]);
	    if (dp < 0.001)
		ip = i;
	}
 
	for (i = MAX_TABLE-1; i >= 0; i--)
	    if (n >= ns[i])
	    {
		j = i;
		break;
	    }
 
	if (n == ns[j] || j == MAX_TABLE-1)
	    *x = *(xs[ip][m-1] + j);
	else
	{
	    an1 = ns[j];
	    an2 = ns[j+1];
	    an  = n;
	    y1  = an1/(1.0 + an1);
	    y2  = an2/(1.0 + an2);
	    y   = an/(1.0 + an);
	    x1  = *(xs[ip][m-1] + j);
	    x2  = *(xs[ip][m-1] + j+1);
	    *x  = x1 + (x2-x1)*((y-y1)/(y2-y1));
	}
}

