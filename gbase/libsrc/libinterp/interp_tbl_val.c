
/*
 * Copyright (c) 1993-1996 Science Applications International Corporation.
 *

 * NAME
 *	interpolate_table_value -- 2-D table interpolation routine.

 * FILE
 *	interp_tab_val.c

 * SYNOPSIS
 *	int
 *	interpolate_table_value (do_extrap, in_hole, nx, nz, x, z, xz,
 *				 dist, depth, value, x_1st_deriv, z_1st_deriv, 
 *				 x_2nd_deriv, z_2nd_deriv, interp_err)
 *	Bool	do_extrap;	(i) Do we want to extrapolate data? (T/F)
 *	Bool	in_hole;	(i) Are we in the hole of a T-T table? (T/F)
 *	Bool	do_we_need_z_derivs; (i) Do you need z derivatives? (T/F)
 *	int	nx;		(i) Dimension (length) of x(distance)-vector
 *	int	nz;		(i) Dimension (length) of z(depth)-vector
 *	float	*x;		(i) Distance samples for values of **xz
 *	float	*z;		(i) Depth samples for values of **xz
 *	float	**xz;		(i) Matrix of x-z samples for a given phase type
 *	float	dist;		(i) x-component distance to be bracketed (deg)
 *	float	depth;		(i) z-component depth to be bracketed (km)
 *	float	*value;		(o) Bi-cubicly interpolated value requested 
 *				    (i.e., travel-time)
 *	float	*x_1st_deriv;	(o) First derivative at variable, value, in 
 *				    x direction (horizontal slowness (sec/deg)) 
 *	float	*x_2nd_deriv;	(o) Second derivative at variable, value, in
 *				    x direction (1st derivative of horizontal 
 *				    slowness)
 *	float	*z_1st_deriv;	(o) First derivative at variable, value, in 
 *				    z direction (dt/dz)
 *	float	*z_2nd_deriv;	(o) Second derivative at variable, value, in 
 *				    z direction
 *	int	*interp_err;	(o) Interpolation error code (flag):
 *				     0:	No extrapolation was necessary!
 *				    11:	Interpolated point in hole of curve
 *				    12:	Interpolated point < first x point
 *				    13:	Interpolated point > last x point
 *				    14:	Interpolated point < first z point
 *				    15:	Interpolated point > last z point
 *				    16:	Interpolated point < first x point
 *						       and < first z point
 *				    17:	Interpolated point > first x point
 *						       and < first z point
 *				    18:	Interpolated point < first x point
 *						       and > first z point
 *				    18:	Interpolated point > first x point
 *						       and > first z point

 * DESCRIPTION
 *	Function.  Using bi-section, bracket a two-dimensional array by
 *	performing sucessive searches on two single arrays which are ordered 
 *	tables possessing monotonically increasing or decreasing values.
 *	Then using bi-cubic interpolation determine a value, and 1st and 2nd
 *	derivatives for a requested distance and depth.

 *	---- Local Important Variables ----
 *	ileft:		Left bracketed indice of x-component of array, xz
 *	iright:		Right bracketed indice of x-component of array, xz
 *	itop:		Left bracketed indice of y-component of array, xz
 *	ibottom:	Right bracketed indice of y-component of array, xz
 *	nx_req:		Number of desired distance samples for mini-table
 *	nz_req:		Number of desired depth samples for mini-table

 *	We can think of nx_req and nz_req being indexed (ordered) as follows:
 *	  o = Table node (an actual distance/depth in table)
 *	  x = Point of Interest (distance or depth)
 *	  l = Nearest left element (ileft in code)

 *		o     o     o     l x   o     o     o
 *		7     5     3     1     2     4     6

 * DIAGNOSTICS
 *	Will send back error message (ERR) if:
 *	     1)	single depth sampling exists, but requested depth is not 
 *		the same as that in the table.
 *	     2)	problems are encountered while doing rational function
 *		extrapolation (via function, ratint()).

 *	Will send back error message (-2) if:
 *	     1) Insufficient valid samples exist for a meaningful travel-
 *		time calculation.

 * FILES
 *	None.

 * NOTES
 *	By setting the Boolean input arguement, do_we_need_z_derivs, to true, 
 *	CPU efficiency is significantly improved, since splie2() only need
 *	be called once.  Also, should maybe pre-compute bi-cubic spline 
 *	2nd derivatives, currently done for every call made here using 
 *	function, splie2().

 * SEE ALSO
 *	Although now out-dated, the old Hermitian bi-variate interpolation 
 *	routine, holint2(), is still located in library, libinterp.

 * AUTHOR
 *	Walter Nagy, March 1993.
 */

#include "config.h"
#include <stdio.h>
#include "aesir.h"
#include "libinterp.h"

#define	MAX_DIST_SAMPLES	7
#define	MAX_DEPTH_SAMPLES	4
#define	BAD_SAMPLE	 	-1.0
#define	MIN_NUM_DIST_SAMPLES	3

#ifndef	MIN
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef	MAX
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))
#endif


int
interpolate_table_value (Bool do_extrap, Bool in_hole, Bool do_we_need_z_derivs,
			 int nx, int nz, float *x, float *z, float **xz, 
			 float dist, float depth, float *value,
			 float *x_1st_deriv, float *z_1st_deriv,
			 float *x_2nd_deriv, float *z_2nd_deriv, 
			 int *interp_err)
{

	int	i, j, k, kk, m, n;
	int	ileft, jleft, nx_req, nz_req;
	int	ilow, ihigh, itop, ibottom;
	int	num_extrap, num_samp;
	int	idist = 0, idepth = 0;
	float	mini_x[MAX_DIST_SAMPLES], mini_z[MAX_DEPTH_SAMPLES];
	float	**mini_table, **mini_table_trans;
	float	**deriv_2nd, **deriv_2nd_trans;
	float	dy, xshift, zshift;


	nx_req = MIN(MAX_DIST_SAMPLES, nx);	/* Requested # of samples 
						   in x-direction */
	nz_req = MIN(MAX_DEPTH_SAMPLES, nz);	/* Requested # of samples
						   in z-direction */

	*value	     = -1.0;
	*x_1st_deriv = -1.0;
	*z_1st_deriv = -1.0;
	*x_2nd_deriv = -1.0;
	*z_2nd_deriv = -1.0;


	if (nz == 1)	/* Special case: Only 1 depth sample available */
	{
	    itop	= 0;
	    ibottom	= 0;
	}
	else
	{
	    /* Bracket depth sample */

	    brack_one_vector (z, nz, depth, &jleft);

	    if (jleft < 0)		/* depth < min. table depth */
	    {
		if (depth == z[0])	/* Check if exactly equal */
		    jleft = 0;
		else
		    idepth--;
		itop	= 0;
		ibottom	= nz_req-1;
	    }
	    else if (jleft >= nz-1)	/* depth > max. table depth */
	    {
		idepth++;
		itop	= nz-nz_req;
		ibottom	= nz-1;
	    }
	    else			/* requested depth within valid range */
	    {
		ibottom = MIN(jleft + (nz_req/2), nz-1);
		itop    = MAX(ibottom-nz_req+1, 0);
		nz_req  = ibottom - itop + 1;
	    }
	}


	/* Bracket distance sample */

	brack_one_vector (x, nx, dist, &ileft);

	if (ileft < 0)			/* dist < minimum table dist */
	{
	    if (dist == x[0])		/* Check if exactly equal */
		ileft = 0;
	    else
		idist--;
	    ilow  = 0;
	    ihigh = nx_req-1;
	}
	else if (ileft >= nx-1)		/* dist > maximum table dist */
	{
	    idist++;
	    ilow  = nx-nx_req;
	    ihigh = nx-1;
	}

	/*
	 * Distance is within a valid table region, but may not have a
	 * valid value.  Interogate table in order to obtain as many 
	 * valid values as possible for either direct interpolation or
	 * eventual extrapolation.  This is determined by the ilow and 
	 * ihigh settings.
	 */

	else
	{
	    /*
	     * Make sure that high and low end requested does not run us
	     * off one side of the distance curve or the other.  We need
	     * to do this even before we check the actual values contained 
	     * in the 2-D (x-z) array.
	     */

	    ihigh = MIN(ileft + (nx_req/2), nx-1);
	    if (ihigh == nx-1)
		ilow = nx-nx_req;
	    ilow = MAX(ihigh-nx_req+1, 0);
	    if (ilow == 0)
		ihigh = nx_req-1;
	}

	if ((idist != 0 || idepth != 0 || in_hole != 0) && !do_extrap)
		goto done;

	/*
	 * If requested distance sample is within table bounds, then we 
	 * need to find as many valid samples as possible.  If none exists 
	 * shift ilow and ihigh closest to a valid curve.  On the other 
	 * hand, if the requested distance sample is located clearly 
	 * outside the valid table region, create an artificial mini-table 
	 * surrounding the requested sample distance value.
	 */

	if (in_hole)
	{
	    if (xz[itop][ihigh] == BAD_SAMPLE)
	    {
		for (i = 0; i < (nx_req-1)/2; i++)
		{
		    --ihigh;
		    if (xz[itop][ihigh] != BAD_SAMPLE)
			break;
		}
		ilow = ihigh - nx_req + 1;
	    }
	    else
	    {
		ihigh = 109;
		ilow  = ihigh - nx_req + 1;
	    }
	}
	else if (idist == 0)
	{
	    if (xz[itop][0] != BAD_SAMPLE && xz[itop][ihigh] == BAD_SAMPLE)
	    {
		idist = 1;
		for (i = 0; i < (nx_req-1)/2; i++)
		{
		    --ihigh;
		    if (xz[itop][ihigh] != BAD_SAMPLE)
		    {
			idist = 0;
			break;
		    }
		}
		ilow = ihigh - nx_req + 1;
	    }
	    else if (xz[itop][ilow] == BAD_SAMPLE)
	    {
		idist = -1;
		for (i = 0; i < (nx_req-1)/2; i++)
		{
		    ++ilow;
		    if (xz[itop][ilow] != BAD_SAMPLE)
		    {
			idist = 0;
			break;
		    }
		}
		ihigh = ilow + nx_req - 1;
	    }
	}


	/* printf ("ileft = %d  nx = %d  nx_req = %d\n", ileft, nx, nx_req); */
	/* printf ("idist = %d  dist = %f  ilow = %d  ihigh = %d\n",
		idist, dist, ilow, ihigh); */
	/*
	 * Up to now we have only inspected the 1st depth component on the
	 * distance vector.  Now we will build a complete mini-table which 
	 * will be used for actual inter/exptrapolation using rational 
	 * function and bi-cubic spline interpolation routines.
	 */

	mini_table = matrix (0, nz_req-1, 0, nx_req-1);
	deriv_2nd  = matrix (0, nz_req-1, 0, nx_req-1);

	for (i = 0; i < nx_req; i++)
	    mini_x[i] = x[ilow+i];
	for (i = 0; i < nz_req; i++)
	    mini_z[i] = z[itop+i];

	/*
	 * First, construct mini-table assuming no depth extrapolation is
	 * needed.  All distance extrapolation will be handled in this master
	 * "for loop".
	 */

	for (k = 0, kk = itop; k < nz_req; k++, kk++)
	{
	    /* First fill mini_table assuming all xz[][] values are valid */

	    for (i = 0; i < nx_req; i++)
		mini_table[k][i] = xz[kk][ilow+i];

	    /* Are we off the high end of the curve OR in a hole */

	    if (in_hole || idist > 0)
	    {
		if (idist > 0 && dist > x[ihigh])
		{
		    xshift = dist - x[ihigh-((nx_req-1)/2)];
		    for (j = 0; j < nx_req; j++)
		    {
			if (k < 1)
			    mini_x[j] = mini_x[j] + xshift;
			mini_table[k][j] = BAD_SAMPLE;
		    }
		    i = ilow;
		}
		else
		{
		    for (i = ihigh; i >= 0; i--)
		    {
			if (xz[kk][i] != BAD_SAMPLE)
			    break;
		    }
		    i = i - nx_req + 1;
		}
		for (j = 0; j < nx_req; j++)
		{
		    if (mini_table[k][j] == BAD_SAMPLE)
		    {
			if ((ratint (&x[i], &xz[kk][i], nx_req, mini_x[j], 
				     &mini_table[k][j], &dy)) != OK)
			{
			    free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			    free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
			    return (ERR);
			}
		    }
		}
	    }

	    /* Are we off the low end of the curve */

	    else if (idist < 0)
	    {
		if (dist < x[ilow])
		{
		    xshift = dist - x[ilow+((nx_req-1)/2)];
		    for (j = 0; j < nx_req; j++)
		    {
			if (k < 1)
			    mini_x[j] = mini_x[j] + xshift;
			mini_table[k][j] = BAD_SAMPLE;
		    }
		    i = ilow;
		}
		else
		{
		    for (i = ilow; i < nx; i++)
		    {
			if (xz[kk][i] != BAD_SAMPLE)
			    break;
		    }
		}
		for (j = 0; j < nx_req; j++)
		{
		    if (mini_table[k][j] == BAD_SAMPLE)
		    {
			if ((ratint (&x[i], &xz[kk][i], nx_req, mini_x[j], 
				     &mini_table[k][j], &dy)) != OK)
			{
			    free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			    free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
			    return (ERR);
			}
		    }
		}
	    }

	    /*
	     * Make sure there are no single BAD_SAMPLE entries.  If so, extrapolate
	     * as necessary.
	     */

	    else
	    {
		for (j = 0; j < nx_req; j++)
		{
		    if (mini_table[k][j] == BAD_SAMPLE)
		    {
			if (j > 0)
			{
			    /*
			     * Go back and get as many valid samples for this 
			     * depth as is possible for a good sample space.
			     */

			    num_extrap = nx_req - j;
			    i = ilow - num_extrap;
			    num_samp = nx_req;
			    while (i < 0 || xz[kk][i] == BAD_SAMPLE)
			    {
				++i;
				--num_samp;
				if (num_samp < MIN_NUM_DIST_SAMPLES)
				{
			    free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			    free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
				    return (-2); /* Insufficient samples */
				}
			    }
			    for (n = 0; n < num_extrap; n++)
			    {
				m = j + n;
				if (mini_table[k][m] == BAD_SAMPLE)
				{
				    if ((ratint (&x[i], &xz[kk][i], num_samp, 
						 mini_x[m], &mini_table[k][m], 
						 &dy)) != OK)
				    {
			free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
					return (-2);
				    }
				}
			    }
			}
			else
			{
			    /*
			     * Advance in distance and get a many valid samples for
			     * this depth as is possible for a good sample space.
			     */

			    num_extrap = 0;
			    num_samp = 0;
			    for (n = 0, i = ilow; i < ihigh; i++, n++)
			    {
				if (xz[kk][i] != BAD_SAMPLE)
				{
				    ilow = i;
				    num_extrap = n;
				    for (num_samp = 0, n = 0; n < nx_req; n++)
					if (xz[kk][ilow+n] != BAD_SAMPLE)
					    ++num_samp;
				    if (num_samp < MIN_NUM_DIST_SAMPLES)
				    {
			free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
					return (-2); /* Insufficient samples */
				    }
				    break;
				}
			    }
			    if (i == ihigh)
			    {
			free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
				return (-2);	/* Must have at least 1 sample */
			    }
			    for (n = 0; n < num_extrap; n++)
			    {
				if (mini_table[k][n] == BAD_SAMPLE)
				{
				    if ((ratint (&x[ilow], &xz[kk][ilow], num_samp,
						 mini_x[n], &mini_table[k][n], 
						 &dy)) != OK)
				    {
			    free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			    free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
					return (-2);
				    }
				}
			    }
			}
			break;
		    }
		}
	    }
	}
	/* printf ("x[%d] = %f  xz = %f  mini_x[%d] = %f  mini_table = %f\n",
		i, x[i], xz[kk][i], j, mini_x[j], mini_table[k][j]); */


	/* 
	 * If only one depth component exists wrap it up here.
	 */

	if (nz == 1)
	{
	    /*
	     * Perform a one-dimensional cubic spline interpolation on a
	     * single row of mini_table[][] to get the desired value for
	     * special case of only one depth available in the table.
	     */

	    spline (mini_x, mini_table[0], nx_req, 1.0e30, 1.0e30,
		    deriv_2nd[0]);
	    splint_deriv (mini_x, mini_table[0], deriv_2nd[0], nx_req,
		          dist, value, x_1st_deriv, x_2nd_deriv);

	    free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
	    free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);

	    goto done;
	}


	/*
	 * Now that the distance component of the mini-table is secure, 
	 * perform any necessary extrapolation for the depth component by
	 * re-constructing the mini-table.  Also, build transposed mini-
	 * table, mini_table_trans[][], to obtain distance derivatives
	 * from spline routines below.
	 */

	mini_table_trans = matrix (0, nx_req-1, 0, nz_req-1);
	deriv_2nd_trans  = matrix (0, nx_req-1, 0, nz_req-1);

	for (j = 0; j < nx_req; j++)
	{
	    /*
	     * Fill mini_table_trans[][] assuming all values from array,
	     * mini_table[][], are valid
	     */

	    for (i = 0; i < nz_req; i++)
		mini_table_trans[j][i] = mini_table[i][j];
	    /*
	    for (i = 0; i < nz_req; i++)
		printf ("mini_table_trans[%d][%d] = %f\n", j, i, mini_table_trans[j][i]);
		*/

	    /* Are we below the lowest depth component in the curve */

	    if (idepth > 0)
	    {
		zshift = depth - z[ibottom-((nz_req-1)/2)];
		if (j < 1)
		{
		    for (i = 0; i < nz_req; i++)
			mini_z[i] = mini_z[i] + zshift;
		}
		for (i = 0; i < nz_req; i++)
		{
		    if ((ratint (&z[itop], &mini_table_trans[j][0], nz_req, 
				 mini_z[i], &mini_table[i][j], &dy)) != OK)
		    {
			free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
			free_matrix (mini_table_trans, 0, nx_req-1, 0, nz_req-1);
			free_matrix (deriv_2nd_trans, 0, nx_req-1, 0, nz_req-1);
			return (ERR);
		    }
		}
		for (i = 0; i < nz_req; i++)
		    mini_table_trans[j][i] = mini_table[i][j];
		/*
		printf ("itop = %d  ibottom = %d  depth = %f  zshift = %f\n", itop, ibottom, depth, zshift);
		 */
	    }

	    else if (idepth < 0)
	    {
		zshift = depth - z[itop+((nz_req-1)/2)];
		if (j < 1)
		{
		    for (i = 0; i < nz_req; i++)
			mini_z[i] = mini_z[i] + zshift;
		}
		for (i = 0; i < nz_req; i++)
		{
		    if ((ratint (&z[itop], &mini_table_trans[j][0], nz_req, 
				 mini_z[i], &mini_table[i][j], &dy)) != OK)
		    {
			free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
			free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
			free_matrix (mini_table_trans, 0, nx_req-1, 0, nz_req-1);
			free_matrix (deriv_2nd_trans, 0, nx_req-1, 0, nz_req-1);
			return (ERR);
		    }
		}
		for (i = 0; i < nz_req; i++)
		    mini_table_trans[j][i] = mini_table[i][j];
	    }
		/*
		for (i = 0; i < nz_req; i++)
			printf ("mini_z[%d] = %f  mini_table_trans[%d][%d] = %f\n", i, mini_z[i], j, i, mini_table_trans[j][i]);
		*/
	}


	/*
	 * Now we have both mini-tables and can perform 2-D bi-cubic
	 * spline interpolations on our mini-tables to obtain our value
	 * of interest and 1st and 2nd derivatives in both the distance 
	 * and depth directions.  Note that the bi-cubic splines routines 
	 * need to be called twice in order to obtain the derivatives in
	 * first the distance direction and then the depth direction.
	 * If do_we_need_z_derivs is set FALSE, then depth derivatives
	 * do not need to be computed.
	 */

	splie2 (mini_x, mini_z, mini_table_trans, nx_req, nz_req,
		deriv_2nd_trans);
	splin2 (mini_x, mini_z, mini_table_trans, deriv_2nd_trans,
		nx_req, nz_req, dist, depth, value, x_1st_deriv, x_2nd_deriv);

	if (do_we_need_z_derivs)
	{
	    splie2 (mini_z, mini_x, mini_table, nz_req, nx_req, deriv_2nd);
	    splin2 (mini_z, mini_x, mini_table, deriv_2nd, nz_req, nx_req,
		    depth, dist, value, z_1st_deriv, z_2nd_deriv);
	}

	free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
	free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
	free_matrix (mini_table_trans, 0, nx_req-1, 0, nz_req-1);
	free_matrix (deriv_2nd_trans, 0, nx_req-1, 0, nz_req-1);

done:
	/* 
	 * All done here!  Set interpolation error flags, as necessary.
	 */

	if (in_hole)
	    *interp_err = 11;
	else if (idist < 0 && idepth == 0)
	    *interp_err = 12;
	else if (idist > 0 && idepth == 0)
	    *interp_err = 13;
	else if (idist == 0 && idepth < 0)
	    *interp_err = 14;
	else if (idist == 0 && idepth > 0)
	    *interp_err = 15;
	else if (idist < 0 && idepth < 0)
	    *interp_err = 16;
	else if (idist > 0 && idepth < 0)
	    *interp_err = 17;
	else if (idist < 0 && idepth > 0)
	    *interp_err = 18;
	else if (idist > 0 && idepth > 0)
	    *interp_err = 19;
	else
	    *interp_err = 0;

	return (OK);

}

