/*
 * NAME
 * 
 *	fvec_funcs - Functions for manipulating float vectors
 *
 * FILE 
 *
 * 	fvec_funcs.c
 *
 * SYNOPSIS
 *
 *	int
 *	fvec_scal_init(x, n, scal)
 *	float	*x;	(i/o) Vector to initialize
 *	int	n;	(i) Number of elements in vector
 *	double	scal;	(i) Scalar initialization factor
 *
 *	int
 *	fvec_scal_mult(x, n, scal)
 *	float	*x;	(i/o) Vector to scale
 *	int	n;	(i) Number of elements in vector
 *	double	scal;	(i) Scale factor
 *
 *	int
 *	fvec_scal_add(x, n, scal)
 *	float	*x;	(i/o) Vector to scale
 *	int	n;	(i) Number of elements in vector
 *	double	scal;	(i) Scale factor
 *
 *	int
 *	fvec_scal_sub(x, n, scal)
 *	float	*x;	(i/o) Vector to scale
 *	int	n;	(i) Number of elements in vector
 *	double	scal;	(i) Scale factor
 *
 *	int
 *	fvec_scal_div(x, n, scal)
 *	float	*x;	(i/o) Vector to scale
 *	int	n;	(i) Number of elements in vector
 *	double	scal;	(i) Scale factor
 *
 *	int
 *	fvec_add(x, y, z, n)
 *	float 	*x, *y; (i) Vectors to add
 *	float	*z;	(o) Resultant vector
 *	int	n;	(i) Number of elements in vector
 *
 *	int
 *	fvec_mult(x, y, z, n)
 *	float 	*x, *y; (i) Vectors to multiply
 *	float	*z;	(o) Resultant vector
 *	int	n;	(i) Number of elements in vector
 *
 *	int
 *	fvec_div(x, y, z, n)
 *	float 	*x, *y; (i) Divide x elements by y elements
 *	float	*z;	(o) Resultant vector
 *	int	n;	(i) Number of elements in vector
 *
 *	int
 *	fvec_abs(x, z, n)
 *	float	*x;	(i/o) Vector to rectify
 *	float	*z;	(o) Resultant vector
 *	int	n;	(i) Number of elements in vector
 *
 *	int
 *	fvec_add_dly(x, z, xn, zn, xtime, ztime, xdly, samprate)
 *	float 	*x;	(i) Vector to delay
 *	float	*z;	(i/o) Resultant sum of delayed vector and input
 *	int	xn, zn;	(i) Number of elements in vectors
 *	double	xtime;	(i) Start time of x (sec)
 *	double	ztime;	(i) Start time of z (sec)
 *	double 	xdly;	(i) Time to delay x (sec)
 *	double	samprate; (i) Sampling rate (samples/sec)
 *
 *	int
 *	fvec_rot2d(x, y, angd, npts)
 *	float 	*x, *y; (i/o) Rotate vectors x,y in place
 *	double	angd;	(i) Angle of rotation in degrees
 *	int	npts;	(i) Number of elements in vectors
 *
 *	int
 *	fvec_rotrt(n, e, azid, npts)
 *	float 	*n, *e; (i/o) Rotate vectors n,e to r,t in place
 *	double	azid;	(i) Azimuth of rotation in degrees
 *	int	npts;	(i) Number of elements in vectors
 *
 *	int
 *	fvec_rotrtz (n, e, z, azid, incd, npts)
 *	float *n, *e, *z; (i/o) Rotate vectors n,e,z to r,t,z in place
 *	double azid;	  (i) Azimuth of rotation in degrees
 *	double incd;	  (i) Incidence angle of rotation in degrees
 *	int npts;	  (i) Number of elements in vectors
 *
 *	int
 *	fvec_sq(x, n)
 *	float	*x;	(i/o) Vector to square
 *	int	n;	(i) Number of elements in vector
 *
 *	int
 *	fvec_sqrt(x, n)
 *	float	*x;	(i/o) Vector to square root
 *	int	n;	(i) Number of elements in vector
 *
 *	int
 *	fvec_mean_var (x, n, mean, var)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double *mean;	(o) Mean value of x
 *	double	*var;	(o) Variance of values of x
 *
 *	int
 *	fvec_mean(x, n, mean)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double *mean;	(o) Mean value of x
 *
 *	int
 *	fvec_rms(x, n, rms)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double *rms;	(o) Root mean square value of x
 *
 *	int
 *	fvec_min_max (x, n, min, max)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double 	*min;	(o) Minimum value of x
 *	double	*max;	(o) Maximum value of x
 *	
 *	int
 *	fvec_max (x, n, max)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double	*max;	(o) Maximum value of x
 *	
 *	int
 *	fvec_abs_min_max (x, n, min, max)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double 	*min;	(o) Minimum absolute value of x
 *	double	*max;	(o) Maximum absolute value of x
 *
 *	int
 *	fvec_abs_max (x, n, min, max)
 *	float	*x;	(i) Vector to analyze
 *	int	n;	(i) Number of elements in vector
 *	double	*max;	(o) Maximum absolute value of x
 *
 * DESCRIPTION
 *
 *	fvec_scal_init() sets the elements of the vector to the init value.
 *	Returns 0 on success.
 *
 *	fvec_scal_mult() multiples the elements of the vector by a scalar.
 *	Returns 0 on success.
 *
 *	fvec_scal_add() adds a scalar to the elements of the vector.
 *	Returns 0 on success.
 *
 *	fvec_scal_sub() subtracts a scalar from the elements of the vector.
 *	Returns 0 on success.
 *
 *	fvec_scal_div() divides the elements of the vector by a scalar.
 *	Returns 0 on success.
 *
 *	fvec_add() adds two vectors and returns the third resultant vector.
 *	Returns 0 on success.
 *
 *	fvec_mult() multiplies two vectors and returns the third resultant 
 *	vector.	Returns 0 on success.
 *
 *	fvec_div() divides the elements in the first vector by the elements
 *	in the second vector and returns the third resultant vector.	
 *	Returns 0 on success.
 *
 *	fvec_add_dly() takes two time series vectors and adds the first to
 *	the second after shifting by a specified time delay. Returns 0
 *	on success.
 *
 *	fvec_rot2d() takes x and y vectors on input and rotates them
 *	in the direction specified by x-hat cross z-hat for positive
 *	rotation angles measures from the x-hat axis. The axis orientation
 *  is a right hand system where x-hat cross y-hat produces positive
 *  z-hat.
 *
 *	fvec_rotrt() takes north and east vectors on input and rotates them
 *	by the specified azimuth.  The two vectors are altered such that the
 *	north(1st) contains the radial component and the east(2nd) 
 *	contains the transverse component.  Directional sense for the radial 
 *	component is positive away from the source.  Transverse crossed 
 *	into radial is positive up.  The azimuth is measured positive 
 *	clockwise from north.
 *
 *	fvec_rotrtz() takes north,east,and vertical vectors on input and 
 *	rotates them by the specified azimuth and incidence angle.  The three
 *	vectors are altered such that the north(1st) contains the radial
 *	component, the east(2nd) contains the transverse component,
 *	and the vertical(3rd) contains the vertical component.  
 *	Directional sense for the radial component is positive away from 
 *	the source.  Transverse crossed into radial is positive up.  
 *	The azimuth is measured positive clockwise from north.  The incidence
 *	angle is measured positive from the vertical.  Returns 0 on success.
 *
 *	fvec_abs() takes the absolute value of the first vector
 *	and returns the result in the second vector.  Returns 0 on success.
 *
 *	fvec_sq() squares the elements of the vector. Returns 0 on
 *	success.
 *
 *	fvec_sqrt() takes the square root of the elements of the vector. 
 *	Returns 0 on success.  Assumes all data on input are > 0.
 *
 * 	fvec_mean_var() computes the mean and variance of the elements
 *	of the vector.  Returns -1 if n < 1, otherwise returns 0.
 *
 *	fvec_mean() computes the mean of the elements of the vector.
 *	Returns -1 if n < 1, otherwise returns 0.
 *
 *	fvec_rms() computes the root mean square of the vector. Returns 0 on
 *	success, -1 on failure.
 *
 *	fvec_min_max() computes the minimum and maximum values of the elements
 *	of the vector. Returns -1 if n < 1, otherwise returns 0.
 *
 *	fvec_max() computes the maximum value of the elements
 *	of the vector. Returns -1 if n < 1, otherwise returns index of max value.
 *
 *	fvec_abs_min_max() computes the minimum absolute and maximum absolue 
 *	values of the elements of the vector. Returns -1 if n < 1, otherwise 
 *	returns 0.
 *
 *	fvec_abs_max() computes the maximum absolue value of the elements 
 *	of the vector. Returns -1 if n < 1, otherwise returns index of abs max.
 *
 *
 * NOTES
 * 
 *	Functions have been written to take advantage of single precision
 *	floating point operations.  Using SUN cc -fsingle with optimization
 *	is recommended.
 *
 *	Input vectors are modified in those routines which don't have 
 *	separate output arguments (eg fvec_sq, fvec_scal_mult, etc.)
 *
 *      A test program is included at the bottom of the file.
 *
 * SEE ALSO
 *
 *	fwfm_funcs.c for waveform structure manipulation
 *
 * AUTHOR
 *
 *	Darrin Wahl, Jeff Given
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>	/* For rint(), pow(), sqrt(), fabs()             */

#define MAX(x,y)      (((x) > (y)) ? (x) : (y))
#define MIN(x,y)      (((x) < (y)) ? (x) : (y))
#define FREE(a)       if(a) {free(a); a = NULL;}

int
fvec_scal_init( float *x, int n, double scal)
{
	register float f = scal;
	register int i;
	
	for (i = 0; i < n; i++)
		x[i] = f;
	
	return(0);
}


int
fvec_scal_mult(float *x, int n, double scal)
{
	register float f = scal;
	register int i;

	if (f == 1.0)
		return (0);

	if (f == 0.0)
		return (fvec_scal_init (x, n, f));

	for (i = 0; i < n; i++)
		x[i] *= f;
	
	return(0);
}

int
fvec_scal_add( float *x, int n, double scal)
{
	register float f = scal;
	register int i;
	
	if (f == 0.0)
		return (0);

	for (i = 0; i < n; i++)
		x[i] += f;
	
	return(0);
}

int
fvec_scal_sub(float *x, int n, double scal)
{
	register float f = scal;
	register int i;
	
	if (f == 0.0)
		return (0);

	for (i = 0; i < n; i++)
		x[i] -= f;
	
	return(0);
}

int
fvec_scal_div(float *x, int n, double scal)
{
	register float f = scal;
	
	if (f == 0.0)
	{
		return (-1);
	}

	f = 1.0 / f;
	
	return (fvec_scal_mult (x, n, f));
}

int
fvec_add(float *x, float *y, float *z, int n)
{
	register int i;

	for (i = 0; i < n; i++) 
		z[i] = x[i] + y[i];
	
	return(0);
}

int
fvec_mult(float *x, float *y, float *z, int n)
{
	register int i;

	for (i = 0; i < n; i++) 
		z[i] = x[i] * y[i];
	
	return(0);
}

int
fvec_div(float *x, float *y, float *z, int n)
{
	register int i;

	for (i = 0; i < n; i++) 
		if (y[i] != 0.0)
			z[i] = x[i] / y[i];
		
	return(0);
}

int
fvec_abs(float *x, float *z, int n)
{
	register int i;

	for (i = 0; i < n; i++) 
		z[i] = (float) fabs ((double) x[i]);
	
	return(0);
}

int
fvec_sum(float *x, int n, double *sum)
{
	register float f;
	register int i;
	
	f = 0.0;
	for (i = 0; i < n; i++)
		f += x[i];
	
	*sum = f;
	
	return(0);
}

int
fvec_add_dly(float *x, float *z, int xn, int zn, 
             double xtime, double ztime, double xdly, double samprate)
{
	register int i;
	
	int x1, z1, n;

	double x_endtime, z_endtime, x_time = 0.0;
	double x_delay_time = xtime + xdly;

	x1 = 0;
	z1 = 0;

	if(x_delay_time < ztime) 
	{
		x1 = (int)rint((ztime - x_delay_time) * samprate);
		x += x1;
		x_time = ztime;
	}
	else if(ztime <= x_delay_time)
	{
		z1 = (int) rint ((x_delay_time - ztime) * samprate);
		z += z1;
		x_time = ztime + ((double) z1 / samprate);
	}
	
	n = xn - x1;

	x_endtime = x_time + ((double) n / samprate);
	z_endtime = ztime + ((double) zn / samprate);

	if(x_endtime > z_endtime)
	{
		n -= (int) rint ((x_endtime - z_endtime) * samprate);
	}
	
	for(i = 0; i < n; i++, x++, z++)
	{
		*z += *x;
	}
	
	return(0);
}

int
fvec_rot2d (float *x, float *y, double angd, int npts)
{
	int 	i;
	float 	angr;
	float	x0, y0;
	float	s, c;	

	/* Compute angle in radians */
	angr = M_PI * angd / 180.0;
	s = (float) sin (angr);
	c = (float) cos (angr);

	for (i = 0; i < npts; i++)
	{
		x0 = x[i];
		y0 = y[i];
		
		x[i] =  x0 * c + y0 * s;
		y[i] = -x0 * s + y0 * c;
        } 

	return (0);
}

int
fvec_rotrt (float *n, float *e, double azid, int npts)
{
	int 	i;
	float 	azir;
        float	s, c;	
	float 	east, north;

	/* Compute azimuth in radians measured positive away from source */
	azir = M_PI * azid / 180.0;
	s = (float) sin (azir - M_PI);
	c = (float) cos (azir - M_PI);

	for (i = 0; i < npts; i++)
	{
		east = e[i];
		north = n[i];

		/* Radial component into north */
		n[i] =   (east * s) + (north * c);

		/* Transverse component into east  */
		e[i] =   (east * c) - (north * s);
        } 

	return (0);
}

int
fvec_rotrtz (float *n, float *e, float *z, double azid, double incd, int npts)
{
	int 	i;
	float 	azir, incr;
	float	sa, ca;	
	float	si, ci;	
	float 	east, north, vert;

	/* Compute azimuth in radians measured positive away from source */
	azir = M_PI * azid / 180.0;
	sa = (float) sin (azir - M_PI);
	ca = (float) cos (azir - M_PI);

	/* Compute incidence in radians */
	incr = M_PI * incd / 180.0;
	si = (float) sin (incr);
	ci = (float) cos (incr);

	for (i = 0; i < npts; i++)
	{
		east = e[i];
		north = n[i];
	        vert = z[i];
		
		/* Radial component into north  */
		n[i] =   (east * ci * sa) + (north * ci * ca) - (vert * si);

	        /* Transverse component into east */		
		e[i] =   (east * ca) - (north * sa);

		/* Vertical component into z-vertical */
		z[i] =   (east * si * sa) + (north * si * ca) + (vert * ci); 
        } 

	return (0);
}


int
fvec_sq(float *x, int n)
{
	register int i;

	for (i = 0; i < n; i++) 
		x[i] *= x[i];
	
	return(0);
}

int
fvec_sqrt(float *x, int n)
{
	register int i;

	for (i = 0; i < n; i++) 
	{
		if (x[i] < 0.0)
		{
			return (-1);
		}
		x[i] = (float) sqrt ((double) x[i]);
	}	
	return(0);
}

int
fvec_mean( float *x, int n, double *mean )
{
	double sum;

	if (n < 1)
		return (-1);

	fvec_sum (x, n, &sum);

	*mean = sum / (double) (n);

	return(0);
}

int
fvec_mean_var (float *x, int n, double *mean, double *var)
{
	register int i;
	float sum, sqerr;

	if(n < 1)
		return(-1);
	
	if (fvec_mean (x, n, mean) < 0)
		return(-1);

	sum = *mean;
	for (i = 0, sqerr = 0; i < n; i++)
	{
		sqerr += (float) pow((double)(sum - x[i]), 2);
	}
	
	*var = sqerr / (double) n;
	
	return(0);
}

int
fvec_rms( float *x, int n, double *rms)
{
	register int i;
	float sum;

	if (n < 1)
		return (-1);

	for (i = 0, sum = 0 ; i < n; i++) 
		sum += x[i] * x[i];
		
	*rms = sqrt ((double) sum / (double) (n));

	return(0);
}


int
fvec_min_max (float *x, int n, double *min, double *max)
{
	register int i;
	float lmax, lmin;

	if (n < 1) 
		return(-1);

	lmin = lmax = x[0];
	for (i = 0; i < n; i++) 
	{
		lmax = MAX(lmax, x[i]);
		lmin = MIN(lmin, x[i]);
	}

	*max = lmax;
	*min = lmin;
	
	return(0);
}

int
fvec_max (float *x, int n, double *max)
{
	register int i;
	int imax;
	float lmax;

	if (n < 1) 
		return(-1);

	lmax = x[0];
	imax = 0;
	for (i = 0; i < n; i++) 
	{
		if (x[i] > lmax)
		{
			lmax = x[i];
			imax = i;
		}
	}

	*max = lmax;
	
	return(imax);
}

int
fvec_abs_min_max (float *x, int n, double *min, double *max)
{
	register int i;
	float lmax, lmin, t;

	if (n < 1) 
		return(-1);

	lmin = lmax = (float) fabs ((double) x[0]);
	for (i = 0; i < n; i++) 
	{
		t = (float) fabs ((double) x[i]);
	
		lmax = MAX(lmax, t);
		lmin = MIN(lmin, t);
	}

	*max = lmax;
	*min = lmin;
	
	return(0);
}

int
fvec_abs_max (float *x, int n, double *max)
{
	register int i;
	int imax;
	float lmax, t;

	if (n < 1) 
		return(-1);

	lmax = (float) fabs ((double) x[0]);
	imax = 0;
	for (i = 0; i < n; i++) 
	{
		t = (float) fabs ((double) x[i]);
		if (t > lmax)
		{
			lmax = t;
			imax = i;
		}
	}

	*max = lmax;
	
	return(imax);
}

	
/*
 * gcc -g -Wall -Wmissing-prototypes -Dlint fvec_funcs.c -o funcs.test ../../lib/libaesir.a -lm \
 *     -I. -I../../include -I../../include/db3 -D__svr4__ -DFUNCS_TEST
 */
#ifdef FUNCS_TEST

#include <stdio.h>

#define NSAMPS 10
#define NZSAMPS 14

void
main (argc, argv)
int	argc;
char	**argv;
{
	int	xn, yn, zn;
	double	xtime, ztime, xdly, samprate, mean, var, min, max, sum, rms;
	float	x[NSAMPS], y[NSAMPS], z[NZSAMPS];
	int	i;
	
	xn = yn = NSAMPS;
	zn = NZSAMPS;
	
	for (i = 0; i < NSAMPS; i++)
	{
		x[i] = i - 1;
		y[i] = i - 1;
		fprintf (stderr, "x[%d] = %f  y[%d] = %f\n", i, x[i],i,y[i]);
	}
	
	for (i = 0; i < NZSAMPS; i++)
	{
		z[i] = 0.0;
	}

	if (fvec_add (x, y, z, xn) < 0)
		fprintf (stderr, "error add x,y\n");
	else
		for (i = 0; i < xn; i++)		
			fprintf (stderr, "(x+y)[%d] = %f\n", i, z[i]);


	if (fvec_mult (x, y, z, xn) < 0)
		fprintf (stderr, "error mult x,y\n");
	else
		for (i = 0; i < xn; i++)		
			fprintf (stderr, "(x*y)[%d] = %f\n", i, z[i]);


	if (fvec_div (x, y, z, xn) < 0)
		fprintf (stderr, "error div x,y\n");
	else
		for (i = 0; i < xn; i++)		
			fprintf (stderr, "(x*y/y)[%d] = %f\n", i, z[i]);


	if (fvec_mean_var (y, yn, &mean, &var) < 0)
		fprintf (stderr, "error sqr y\n");
	else
		fprintf (stderr, "mean, var y= %f, %f\n", mean, var);

	if (fvec_min_max (x, xn, &min, &max) < 0)
		fprintf (stderr, "error min max x\n");
	else
		fprintf (stderr, "min, max x= %f, %f\n", min, max);

	if (fvec_abs_min_max (x, xn, &min, &max) < 0)
		fprintf (stderr, "error abs min max x\n");
	else
		fprintf (stderr, "abs min, abs max x= %f, %f\n", min, max);

	if ((i = fvec_max (x, xn, &max)) < 0)
		fprintf (stderr, "error max x\n");
	else
		fprintf (stderr, "max x[%d]= %f\n", i, max);


	if ((i = fvec_abs_max (x, xn, &max)) < 0)
		fprintf (stderr, "error abs max x\n");
	else
		fprintf (stderr, "abs max x[%d]= %f\n", i, max);


	if (fvec_rms (x, xn, &rms) < 0)
		fprintf (stderr, "error rms x\n");
	else
		fprintf (stderr, "rms x= %f\n", rms);


	if (fvec_abs (x, z, xn) < 0)
		fprintf (stderr, "error abs x\n");
	else
		for (i = 0; i < xn; i++)		
			fprintf (stderr, "abs(x)[%d] = %f\n", i, z[i]);


	if (fvec_sq (x, xn) < 0)
		fprintf (stderr, "error sqr x\n");
	else
		for (i = 0; i < xn; i++)		
			fprintf (stderr, "(x*x)[%d] = %f\n", i, x[i]);


	if (fvec_scal_add (z, zn, 1.0) < 0)
		fprintf (stderr, "error scal add z\n");
	else
		for (i = 0; i < zn; i++)		
			fprintf (stderr, "z[%d]+1 = %f\n", i, z[i]);

	if (fvec_scal_sub (z, zn, 10.0) < 0)
		fprintf (stderr, "error scal sub z\n");
	else
		for (i = 0; i < zn; i++)		
			fprintf (stderr, "z[%d]-10. = %f\n", i, z[i]);

	if (fvec_scal_div (z, zn, 2.0) < 0)
		fprintf (stderr, "error scal div z\n");
	else
		for (i = 0; i < zn; i++)		
			fprintf (stderr, "z[%d]/2.0 = %f\n", i, z[i]);

	if (fvec_sum (z, zn, &sum) < 0)
		fprintf (stderr, "error sum z\n");
	else
		fprintf (stderr, "sum z = %f\n", sum);
	

	if (fvec_scal_mult (z, zn, 0.0) < 0)
		fprintf (stderr, "error scal z\n");
	else
		for (i = 0; i < zn; i++)		
			fprintf (stderr, "zero * z[%d] = %f\n", i, z[i]);


	/* Test delay function */
	xtime = 0;
	ztime = -3;
	xdly = -2;
	samprate = 1.0;
	if (fvec_add_dly(x, z, xn, zn, xtime, ztime, xdly, samprate) < 0)
		fprintf (stderr, "error add delay\n");
	else
		for (i = 0; i < zn; i++)		
			fprintf (stderr, "z+xdly[%d] = %f\n", i, z[i]);
	

	exit(0);

}
#endif

