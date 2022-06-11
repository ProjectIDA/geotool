
/*
 * Copyright (c) 1991-1997 Science Applications International Corporation.
 *

 * NAME
 * 	solve_via_svd -- Perform Singular Value Decomposition.

 * FILE
 * 	solve_via_svd.c

 * SYNOPSIS
 *	int
 *	solve_via_svd (icov, nd, np, maxp, at, d, damp, cnvgtst, condit, xsol,
 *		       covar, data_importances, applied_damping, rank)
 *	int	icov;		(i) Compute covariance matrix: 1 = No; 2 = Yes
 *	int	nd;		(i) Number of data (rows; observations)
 *	int	np;		(i) Number of parameters (columns)
 *	int	maxp;		(i) Leading dimension of at[] and covar[]
 *	double	*at;		(i) Transpose of the derviative (system) matrix
 *	double	*d;		(i) Data (residual) vector
 *	double	damp;		(i) User-specified percent damping to be applied
 *				    to diagonal elements of at[] [g[]].  That
 *				    is damping as a percent of the largest 
 *				    singular value.  If damp < 0.0, only damp 
 *				    when condition number > 30.0.
 *	double	*cnvgtst;	(o) Convergence test measure of Paige and
 *				    Saunders (1982).
 * 	double	condit[0];	(o) True condition number of non-zero singular 
 *				    values returned from dsvdc.  That is, 
 *				    largest sval/smallest sval calculated 
 *				    before scaling limit enforced.
 * 	double	condit[1];	(o) Effective condition number of non-zero 
 * 				    singular values.  In this case, the actual 
 *				    largest sval/smallest sval retained for 
 *				    use in obtaining solution.
 *	double	*xsol;		(o) Solution (adjustment) vector
 * 	double	covar[][];	(o) Model covariance matrix [square-symmetric]
 * 	double	*data_importances: (o) Hypocentral data importance vector
 * 	double	*applied_damping:  (o) Percent damping actually applied to the 
 *				       diagonal elements of at[] [g[]].  If
 *				       damp < 0.0, only apply damping when
 *				       condition number > 30.0.
 * 	double	rank:		(o) Effective rank of matrix

 * DESCRIPTION
 * 	Function.  Compute hypocentral solution vector by Singular Value 
 *	Decomposition (SVD) of a given system matrix.  We decompose an 
 *	arbitrary NxM rectangular matrix, G, via the method of SVD into its
 * 	component parts using a standard LINPACK routine.  This
 * 	mini-driver determines a solution of the form:

 * 	G =	U * sval * V-transpose, where U and V contain the left
 * 		and right singular vectors, respectively, while sval
 * 		holds the corresponding singular values.

 *	It is the rank of G that determines the maximum number of possible 
 *	singular values that are calculated.  So, if nd < np, then the 
 *	maximum rank of at[] is nd, and only nd singular values can be 
 *	calculated.  If the variable, info, is non-zero, then the number 
 *	of singular values will not be np.

 *	Given, k = MIN(nd, np)
 *	  Subr. dsvdc fills U with left singular vectors stored as,

 *			| vector 1   vector 2   ... vector k  |
 *			|   u1(1)      u2(1)    ...    uk(1)  |
 *			|   u1(2)      u2(2)    ...    uk(1)  |
 *	  u(np,np) =	|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|   u1(np)     u2(np)   ...    uk(np) |

 *	Given, k = MIN(nd, np)
 *	  Subr. dsvdc fills V with right singular vectors stored as,

 *			| vector 1   vector 2   ... vector k  |
 *			|   v1(1)      v2(1)    ...    vk(1)  |
 *			|   v1(2)      v2(2)    ...    vk(1)  |
 *	  v(nd,nd) =	|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|   v1(nd)     v2(nd)   ...    vk(nd) |

 *	  Then given,
 *	    G * m = d
 *	  Subr. dsvdc decomposes G as,
 *	    G  => ( V * LAMBDA * U-transpose )
 *	  So,
 *	    G * m = ( V * LAMBDA * U-transpose ) * m
 *	  And,
 *	    m = ( U * LAMBDA-inverse * V-transpose ) * d

 * 	  it's not,	Gm = (U * sval * V-transpos)m,
 * 	  but,		Gm = (V * sval * U-transpos)m
 * 	  so,		m  = (U * sval-inverse * V-Transpose) * d

 * 	---- Subroutines called ----
 * 	Local
 * 		dsvdc:	LINPACK Singular Value Decomposition routine

 * DIAGNOSTICS
 * 	Makes checks for invalid singular values and simply ignores them.

 * NOTES
 * 	Beware of problems associated with variable, info, from LINPACK
 * 	subroutine dsvdc.

 * SEE ALSO
 * 	LINPACK, John Dongarra for explanation of SVD application along
 * 	with corresponding subroutines; and NETLIB (netlib@ornl.gov) for
 * 	obtaining any LINPACK source code.

 * AUTHOR
 * 	Walter Nagy,  6/91,	Created.
 * 	Walter Nagy,  8/24/92,	Damping removed from calculation of covariance 
 *				matrix.  applied_damping argument also added.
 * 	Walter Nagy,  9/25/92,	Fixed divide-by-zero problems noted by Fred
 *				Dashiell at Inference.
 */


#include "config.h"
#include <math.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */
#include "locp.h"
#include "loc_defs.h"

#define MAX_DATA 800

static	int	maxdata	= MAX_DATA;

extern	int	dsvdc(double*,int,int,int, double*, double*, double*,
int, double*, int, double*, int, int*);


int
solve_via_svd (int icov, int nd, int np, int maxp, double *at, double *d, 
	       double damp, double *cnvgtst, double *condit, double *xsol,
	       double covar[][MAX_PARAM], double *data_importances, 
	       double *applied_damping, double *rank)
{
	int	job = 21;
	int	i, icnt, info, j, k, neig, norder;
	double	*e, *g, gscale[MAX_PARAM], gtr[MAX_PARAM], smax, sum;
	double	sval[MAX_PARAM+1], tmp[MAX_PARAM], u[MAX_PARAM][MAX_PARAM];
	double	*v, work[MAX_PARAM];
	double	dscale, frob, gtrnorm, rnorm;


	e = UALLOCA (double, nd);
	g = UALLOCA (double, MAX_PARAM * nd);
	v = UALLOCA (double, MAX_DATA * nd);

	/*
	 *  Variable, norder, limits the maximum possible number of singular
	 *  values.  Variable, job, controls the singular values sent back
	 *  from dsvdc.  Current job setting tells dsvdc to ignore bogus
	 *  values and pass both left and right singular vectors back.
	 */

	norder	= MIN(np, nd);

	/*
	 *  Unit-column normalize at[] matrix and stuff it into g[], since 
	 *  subr. dsvdc overwrites original matrix upon its return.
	 */

	for (j = 0; j < np; ++j)
	    gscale[j] = 0.0;

	for (i = 0; i < nd; ++i)
	{
	    for (j = 0; j < np; ++j)
	    {
		dscale = at[j + i*maxp];
		gscale[j] += dscale*dscale;
	    }
	}

	for (j = 0; j < np; ++j)
	{
	    if (gscale[j] > 0.0)
		gscale[j] = 1.0/sqrt(gscale[j]);
	    else
		gscale[j] = 1.0;
	}

	/*
	 *  Scale origin terms of similar dimension to spatial terms,
	 *  assuming a medium velocity of 8 km./sec.
	 */

	/* gscale[0] = 0.125*gscale[0]; */

	for (i = 0; i < nd; ++i)
	    for (j = 0; j < np; ++j)
		g[j + i*maxp] = at[j + i*maxp] * gscale[j];

	/*
	 *  Compute norms and undertake convergence test.
	 */

	frob	= 0.0;
	rnorm	= 0.0;
	gtrnorm	= 0.0;
	for (i = 0; i < nd; ++i)
	{
            rnorm = rnorm + d[i]*d[i];
	    for (j = 0; j < np; ++j)
	    {
		dscale = g[j + i*maxp];
		frob += dscale*dscale;
	    }
	}

	for (j = 0; j < np; ++j)
	{
	    gtr[j] = 0.0;
	    for (i = 0; i < nd; ++i)
		gtr[j] += g[j + i*maxp]*d[i];
	    gtrnorm += gtr[j]*gtr[j];
	}
	*cnvgtst = gtrnorm / (frob*rnorm);

	/* printf ("gtrnorm = %g  frob = %g\n", gtrnorm, frob); */
	/* printf ("rnorm = %g  cnvgtst = %g\n", rnorm, *cnvgtst); */

	/*
	 *  Decompose the matrix into its right and left singular vectors,
	 *  u[] and v[], respectively, as well as the diagonal matrix of
	 *  singular values, sval.  That is, perform an SVD.  LINPACK routine, 
	 *  dsvdc, of John Dongarra is chosen here.
	 */

	dsvdc(g, maxp, np, nd, sval, e, (double *)u, maxp, v, maxdata, work, job, &info);

	if (info >= norder)
	    return (GLerror6);

	/*
	 *  Adjust variable, neig, to control singular value cutoff, 
	 *  effectively determining which singular values to keep and/or 
	 *  ignore.
	 *      Note:	The good singular values are stored at the end of 
	 *		sval(), not the beginning.  smax is always sval(info+1)
	 *		with the descending values immediately following.
	 */

	neig = norder - info;

	/*
	 *  Avoid small singular values (limit condition number to ctol)
	 *  That is, set a singular value cutoff (i.e., singular values 
	 *  < pre-set limit, in order to obtain an effective condition number).
	 */

	smax = sval[info];
	for (j = info + 1; j < norder; ++j)
	{
	    if (smax > sval[j]*COND_NUM_LIMIT)
	    {
		neig = j - info;
		break;
	    }
	}

	/*
	 *  Construct the real (condit[0]) and effective (condit[1])
	 *  condition numbers from the given singular values
	 */

	if (sval[norder-1] > 0.0)
	    condit[0] = sval[info] / sval[norder-1];
	else
	    condit[0] = HUGE_VAL;
	condit[1] = sval[info] / sval[info+1 + (neig-1) - 1];
	if (isnan (condit[0]) || isnan (condit[1]))
	    return (GLerror6);

	/*
	 * If only the covariance matrix is desired, do NOT damp!!!
	 */

	if (icov > 1) 
	{
	    /* 
	     * Construct the parameter (model) covariance matrix, 
	     * if requested 
	     */

	    icnt = info+1 + (neig-1);
	    for (i = info; i < icnt; ++i)
		sval[i] = 1.0/(sval[i]*sval[i]);

	    for (i = 0; i < np; ++i)
	    {
		for (j = 0; j <= i; ++j)
		{
		    sum = 0.0;
		    icnt = info+1 + (neig-1);
		    for (k = info; k < icnt; ++k)
			sum += u[k][i] * u[k][j] * sval[k];
		    covar[j][i] = sum * gscale[j]*gscale[i];
		}
	    }

	    /*
	     * and then, the data importances (i.e., the diagonal 
	     * elements of the data resolution matrix)
	     */

	    *rank = 0.0;
	    for (i = 0; i < nd; ++i)
	    {
		sum = 0.0;
		for (j = 0; j < np; ++j)
		{
		    icnt = i + j*MAX_DATA;
		    sum += v[icnt]*v[icnt];
		}
		*rank += sum;
		data_importances[i] = sum;
	    }
	}
	else
	{
	    /* Apply damping, if necessary */

	    *applied_damping = 0.0;
	    if (damp < 0.0)
	    {
		if (condit[0] > 30.0)
		{
		    /*
		     * Apply damping of 1% largest singular value for 
		     * moderately ill-conditioned system.  Make this 5% 
		     * for more severely ill-conditioned system and 10% 
		     * for highly ill-conditioned problems.
		     */

		    icnt = info+1 + (neig-1);
		    *applied_damping = 0.01;
		    if (condit[0] > 300.0) 
			*applied_damping = 0.05;
		    if (condit[0] > 3000.0) 
			*applied_damping = 0.10;
		    for (i = info; i < icnt; ++i)
			sval[i] += smax * (*applied_damping);
		}
	    }
	    else
	    {
		*applied_damping = damp;
		icnt = info+1 + (neig-1);
		for (i = info; i < icnt; ++i)
		    sval[i] += smax * 0.01*(*applied_damping);
	    }

	    /*
	     * Find solution vector -- 
	     * First, compute (1/sval) * V-trans * d, 
	     */

	    icnt = info+1 + (neig-1);
	    for (j = info; j < icnt; ++j)
	    {
		sum = 0.0;
		for (i = 0; i < nd; ++i)
		    sum += v[i + j*MAX_DATA] * d[i];
		tmp[j] = sum/sval[j];
	    }

	    /*
	     * then, multiply by U, which yields the desired solution 
	     * vector, (i.e.,  xsol = U * (1/sval) * V-trans*d) = U * tmp
	     */

	    for (j = 0; j < np; ++j)
	    {
		sum = 0.0;
		icnt = info+1 + (neig-1);
		for (i = info; i < icnt; ++i)
		    sum += u[i][j] * tmp[i];
		xsol[j] = sum*gscale[j];
	    }
	}

	return (OK);
}
