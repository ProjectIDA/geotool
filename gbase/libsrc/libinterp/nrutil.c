
/*
 * Numerical Recipes Utility Functions
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include "libinterp.h"

/* Numerical Recipes standard error handler */

void nrerror (char error_text[])

     /*char	error_text[];*/
{
	fprintf (stderr, "Numerical Recipes run-time error...\n");
	fprintf (stderr, "%s\n", error_text);
	fprintf (stderr, "...now exiting system...\n");
	exit (1);
}


/* Allocate a float vector with range [nl..nh] */

float *vector (int nl, int nh)
{
	float	*v;

	v = (float *)malloc ((unsigned) (nh-nl+1)*sizeof(float));
	if (!v) nrerror ("Allocation failure in vector()");
	return	v-nl;
}


/* Allocate a double vector with range [nl..nh] */

double	*dvector(int nl, int nh)
{
	double	*v;

	v = (double *)malloc ((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) nrerror ("Allocation failure in dvector()");
	return	v-nl;
}


/* Allocate a float matrix with range [nrl..nrh][ncl..nch] */

float **matrix (int nrl, int nrh, int ncl, int nch)
{
	int	i;
	float	**m;

	/* Allocate pointers to rows */
	m = (float **)malloc ((unsigned) (nrh-nrl+1)*sizeof(float*));
	if (!m) nrerror ("Allocation failure 1 in matrix()");
	m -= nrl;

	/* Allocate rows and set pointers to them */
	for (i = nrl; i <= nrh; i++)
	{
		m[i] = (float *)malloc ((unsigned) (nch-ncl+1)*sizeof(float));
		if (!m[i]) nrerror ("Allocation failure 2 in matrix()");
		m[i] -= ncl;
	}

	/* Return pointer to array of pointers to rows */

	return	m;
}


/* Allocate a double matrix with range [nrl..nrh][ncl..nch] */

double	**dmatrix(int nrl, int nrh, int ncl, int nch)
{
	int	i;
	double	**m;

	/* Allocate pointers to rows */
	m = (double **)malloc ((unsigned) (nrh-nrl+1)*sizeof(double*));
	if (!m) nrerror ("Allocation failure 1 in dmatrix()");
	m -= nrl;

	/* Allocate rows and set pointers to them */
	for (i = nrl; i <= nrh; i++)
	{
		m[i] = (double *)malloc ((unsigned) (nch-ncl+1)*sizeof(double));
		if (!m[i]) nrerror ("Allocation failure 2 in dmatrix()");
		m[i] -= ncl;
	}

	/* Return pointer to array of pointers to rows */

	return	m;
}


/* Free a float vector allocated by vector() */

void
free_vector (float *v, int nl, int nh)
{
	free ((char*) (v + nl));
}


/* Free a double vector allocated by dvector() */

void
free_dvector (double *v, int nl, int nh)
{
	free ((char*) (v + nl));
}


/* Free a float matrix allocated by matrix() */

void
free_matrix (float **m, int nrl, int nrh, int ncl, int nch)
{
	int i;

	for (i = nrh; i >= nrl; i--) free ((char*) (m[i] + ncl));
	free ((char*) (m + nrl));
}


/* Free a double matrix allocated by dmatrix() */

void free_dmatrix (double **m, int nrl, int nrh, int ncl, int nch)
{
	int i;

	for (i = nrh; i >= nrl; i--) free ((char*) (m[i] + ncl));
	free ((char*) (m + nrl));
}


/* Allocate a float matrix m[nrl..nrh][ncl..nch] that points to the matrix, a,
 * declared in the standard C manner as a[nrow][ncol], where nrow = nrh-nrl+1
 * and ncol = nch-ncl+1.  The routine should be called with the address 
 * &a[0][0] as the first argument.
 */

float **convert_matrix (float *a, int nrl, int nrh, int ncl, int nch)
{
	int	i, j, nrow, ncol;
	float	**m;

	nrow = nrh - nrl + 1;
	ncol = nch - ncl + 1;

	/* Allocate pointers to rows */
	m = (float **) malloc ((unsigned) (nrow)*sizeof(float *));
	if (!m) nrerror ("Allocation failure in convert_matrix()");
	m -= nrl;
	for (i = 0, j = nrl; i <= nrow-1; i++, j++) m[j] = a + ncol*i - ncl;
	return	m;
}


/* Free a matrix allocated by convert_matrix() */

void
free_convert_matrix (float **b, int nrl, int nrh, int ncl, int nch)
{
	free ((char*) (b + nrl));
}


