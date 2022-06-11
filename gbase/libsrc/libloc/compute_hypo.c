
/*
 * Copyright (c) 1990-1998 Science Applications International Corporation.
 *

 * NAME
 *	compute_hypo -- Compute a hypocentral location
 *	compute_hypo_lm -- Compute a hypocentral location by LM
 *	get_resids_and_derivs -- Calculate residuals and derivatives
 *	dfridr -- Calculate derivative of slowness regarding depth
 *	get_slow -- Calculate slowness
 *	mrqcof -- Evaluate the linearized fitting matrix and vector
 *	gaussj -- Linear equation solution by Gauss-Jordan elimination
 *	ivect -- Allocate an int vector
 *	dvect -- Allocate a double vector
 *	dmatr -- Allocate a double matrix
 *	free_ivect -- Free an int vector
 *	free_dvect -- Free a double vector
 *	free_dmatr -- free a double matrix

 * FILE
 *	compute_hypo.c

 * SYNOPSIS
 *	int
 *	compute_hypo (w_origin, w_origerr, arrival, w_assoc, locator_params, 
 *		      locator_info, ar_info_ptr, sites, num_sites, num_obs, 
 *		      ofp, verbose, torg)
 *	Origin	**w_origin;		(i/o) Local working DB3.0 origin table
 *	Origerr	**w_origerr;		(i/o) Local working DB3.0 origerr table
 *	Arrival	*arrival;		(i) Standard CSS DB3.0 arrival table
 *	Assoc	**w_assoc;		(i/o) Local working DB3.0 assoc table
 *	Locator_params *locator_params;	(i) Locator parameter info structure
 *	Locator_info   *locator_info;	(i/o) Locator info structure
 *	Ar_Info	**ar_info_ptr;		(i/o) Arrival-specific location info
 *	Site	*sites;			(i) Station structure
 *	int	num_sites;		(i) Number of stations in sites table
 *	int	num_obs;		(i) Number of arrival/assoc records
 *	FILE	*ofp;			(i) Output file pointer
 *	int	verbose;		(i) Degree of verbose output
 *					    =0:	No output
 *					    =1:	Print only final location info
 *					    =2:	Print final location and 
 *						arrival info
 *					    =3:	Do not print intermediate 
 *						iterations & input station info
 *					    =4:	Print all output
 *	int	*torg;			(o) Final estimate of event origin-time

 *	int
 *	compute_hypo_lm (origin, origerr, arrival, assoc, locator_params, 
 *		      	 locator_info, ar_info, sites, num_sites, num_obs,
 *		         ofp, verbose, torg, At, resid2, dsd2,
 *	      		 az_used_in_loc, init, svd_conv)
 *	Origin	       *origin;		(i/o) Local working DB3.0 origin table
 *	Origerr	       *origerr;	(i/o) Local working DB3.0 origerr table
 *	Arrival	       *arrival;	(i) Standard CSS DB3.0 arrival table
 *	Assoc	       *assoc;		(i/o) Local working DB3.0 assoc table
 *	Locator_params *locator_params;	(i) Locator parameter info structure
 *	Locator_info   *locator_info;	(i/o) Locator info structure
 *	Ar_Info	       *ar_info;	(i/o) Arrival-specific location info
 *	Site	       *sites;		(i) Station structure
 *	int	       num_sites;	(i) Number of stations in sites table
 *	int	       num_obs;		(i) Number of arrival/assoc records
 *	FILE	       *ofp;		(i) Output file pointer
 *	int	       verbose;		(i) Degree of verbose output
 *	double	       *torg;		(i/o) Origin time
 *	double	       *At;		(i/o) Derivative matrix
 *	double	       *resid2;		(i/o) Residual vector
 *	double	       *dsd2;		(i/o) Standart deviations
 *	double	       *az_used_in_loc;	(i/o) Number of azimuths
 *	double	       *init;		(i) Initial parameters for location
 *	Bool	       svd_conv;	(i) Divergence flag for SVD

 *	int
 *	get_resids_and_derivs (params, deriv)
 *	lm_params	*params;	(i/o) LM parameter structure
 *	Bool		deriv;		(i) Do we need derivatives ?

 *	double
 *	dfridr(x, h, err, n, p)
 *	double		x;		(i) Point for deriv calculating (depth)
 *	double		h;		(i) Estimated initial stepsize
 *	double		*err;		(o) Estimate of the error
 *	int		n;		(i) Station index
 *	lm_params 	*p;		(i) LM parameter structure

 *	double
 *	get_slow(depth, n, p)
 *	double		depth;		(i) Depth
 *	int		n;		(i) Station index
 *	lm_params	*p;		(i) LM parameter structure

 *	int
 *	mrqcof(pars, alpha, beta)
 *	lm_params	*pars;		(i/o) LM parameter structure
 *	double		**alpha;	(i/o) Working matrix
 *	double		*beta;		(i/o) Working vector

 *	int
 *	gaussj(a, n, b, m)
 *	double		**a;		(i/o) Input matrix
 *	int		n;		(i) Dimension of matrix 
 *	double		**b;		(i/o) Right-hand side vectors matrix
 *	int		m;		(i) Number of vectors

 *	int *
 *	ivect(nl, nh)
 *	int		nl;		(i) Lowermost range of vector
 *	int		nh;		(i) Highermost range of vector

 *	double *
 *	dvect(nl, nh)
 *	int		nl;		(i) Lowermost range of vector
 *	int		nh;		(i) Highermost range of vector

 *	double **
 *	dmatr(nrl, nrh, ncl, nch)
 *	int		nrl;		(i) Lowermost number of rows
 *	int		nrh;		(i) Highermost number of rows
 *	int		ncl;		(i) Lowermost number of cells
 *	int		nch;		(i) Highermost number of cells

 *	void
 *	free_ivect(v, nl, nh)
 *	int		*v;		(i) Pointer to vector
 *	int		nl;		(i) Lowermost range of vector
 *	int		nh;		(i) Highermost range of vector

 *	void
 *	free_dvect(v, nl, nh)
 *	double		*v;		(i) Pointer to vector
 *	int		nl;		(i) Lowermost range of vector
 *	int		nh;		(i) Highermost range of vector

 *	void
 *	free_dmatr(m, nrl, nrh, ncl, nch)
 *	double		**m;		(i) Pointer to matrix
 *	int		nrl;		(i) Lowermost number of rows
 *	int		nrh;		(i) Highermost number of rows
 *	int		ncl;		(i) Lowermost number of cells
 *	int		nch;		(i) Highermost number of cells

 * DESCRIPTION
 *	-- compute_hypo() is hypocenter inversion (event location) routine, done 
 *	as an iterative non-linear least squares inversion of travel-time, 
 *	azimuth and slowness data.  Computes event locations, confidence 
 *	bounds, residuals and importances using arrival time, azimuths 
 *	and slowness measurements from stations at regional and 
 *	teleseismic distances.

 *	ar_info[n].time_error_code:	Error code for n'th observation
 *		  .az_error_code:
 *		  .slow_error_code:
 *				  =  0,	No problem, normal interpolation
 *				  =  1,	No station information for datum
 *				  =  2,	No travel-time tables for datum
 *				  =  3,	Data type unknown
 *				  =  4,	S.D <= 0.0 for datum
 *				  =  5, Failed large residual criteria
 *				  =  6, Failed use only data with corr. criteria
 *				  = 11,	Distance-depth point (x0,z0) in 
 *					hole of T-T curve
 *				  = 12,	x0 < x(1)
 *				  = 13,	x0 > x(max)
 *				  = 14,	z0 < z(1)
 *				  = 15,	z0 > z(max)
 *				  = 16,	x0 < x(1) and z0 < z(1)
 *				  = 17,	x0 > x(max) and z0 < z(1)
 *				  = 18,	x0 < x(1) and z0 > z(max)
 *				  = 19,	x0 > x(max) and z0 > z(max)
 *				[NOTE:	If any of these codes is .le. 0 (e.g. 
 *					data_error_code = -17), the datum was 
 *					used to compute event location]

 *	origin->depth:		Initial educated guess of event focal depth (km)
 *				value, if < 0.0, only damp when condition 
 *				number > 30.0 

 *	ierr:			Solution error flag (return argument);
 *				  = 0:	No error
 *				  = 1:	Maximum number of iterations exceeded
 *				  = 2:	Divergent solution encountered
 *				  = 3:	Insufficient defining data before 
 *					location is attempted
 *				  = 4:	Insufficient defining data due to
 *					hole in T-T table(s)
 *				  = 5:	Insufficient defining data due to
 *					T-T table extrapolation
 *				  = 6:	SVD routine cannot decompose matrix
 *				  = 7:	Condition number of solution is too 
 *					great to continue
 *				  = 23: Failed large residual criteria
 *				  = 24: Failed use only data with corr. criteria

 *	-- compute_hypo_lm() is an alternative event location routine using 
 *	Levenberg-Marquardt method. Computes event locations using arrival time, 
 *	azimuths and slowness measurements from stations at regional and
 *	teleseismic distances.

 *	-- get_resids_and_derivs() calculates residuals and derivatives of travel
 *	times,azimuths and slowness. This is the function used by compute_hypo_lm().
 
 *	-- dfridr() returns the derivative of slowness regarding depth by Ridders
 *	method of polynomial extrapolation. The value h is input as an estimated
 *	initial stepsize; it need not be small, but rather should be an increment
 *	in x over which func changes substantially. An estimate of the error in
 *	the derivative is returned as err.

 *	-- get_slow() calculates slowness for dfridr().

 *	-- mrqcof() evaluates the linearized fitting matrix alpha and vector beta.

 *	-- gaussj() is linear equation solution by Gauss-Jordan elimination.
 *	a[1..n][1..n] is the input matrix. b[1..n][1..m] is input containing
 *	the m right-hand side vectors. On output, a is replaced by its matrix inverse,
 *	and b is replaced by the corresponding set of solution vectors.

 *	-- ivect() allocates an int vector with subscript range v[nl..nh].

 *	-- dvect() allocates a double vector with subscript range v[nl..nh].

 *	-- dmatr() allocates a double matrix with subscript range m[nrl..nrh][ncl..nch].

 *	-- free_ivect() frees an int vector allocated with ivect().

 *	-- free_dvect() frees a double vector allocated with dvect().

 *	-- free_dmatr() frees a double matrix allocated by dmatr().


 *	---- Functions called ----
 *	Local
 *		total_travel_time:	Compute total travel-time, slowness 
 *					along with derivatives
 *		solve_via_svd:		Perform least squares inversion via SVD

 *	From libgeog
 *		dist_azimuth: 	Determine the distance between between two 
 *				lat./lon. pairs
 *		lat_lon: 	Compute a second lat./lon. from the first,
 *				distance, and azimuth

 * DIAGNOSTICS
 *	See locator_error_table[] in file, loc_error_msg.c, for specific 
 *	global error code descriptions.

 * FILES
 *	None.

 * NOTES
 *	Note that the 2nd call to solve_via_svd() is only used to compute
 *	parameter covariance matrix and data importances.  The latter are
 *	defined as the diagonal elements of the data resolution matrix
 *	calculated within solve_via_svd().

 * SEE ALSO
 *	Bratt and Bache (1988) "Locating events with a sparse network of
 *	regional arrays", BSSA, 78, 780-798.  Also, Jordan and Sverdrup
 *	(1981) "Teleseismic location techniques and their application to
 *	earthquake clusters in the south-central Pacific", BSSA, 71,
 *	1105-1130.

 * AUTHORS
 *	Steve Bratt, 12/88,	Created.
 *	Walter Nagy,  7/90,	Major re-write.
 *	Walter Nagy,  8/18/92,	Major enhancement.  Converted to C.
 *	Walter Nagy,  1/ 6/93,	Added code to fix either lat/lon or origin time.
 *	Walter Nagy,  3/24/93,	Ellipticity and elevation corrections can now
 *				be incorporated in total T-T calculation.
 *	Walter Nagy,  3/28/93,	Added outlier screening functionality so that
 *				large residuals will be thrown out.
 *	Walter Nagy,  3/29/93,	Added function, apply_srst(), to apply SRST 
 *				corrections for ADSN project. 
 *	Walter Nagy,  4/23/93,	Added code to apply distance-variance weighting.
 *	Walter Nagy,  5/27/93,	Added code to apply test-site corrections for
 *				ADSN project.
 *	Walter Nagy, 12/ 1/93,	Moved variance and error ellipse calculations
 *				out of here and into file file, ellips.c.
 *	Walter Nagy,  3/ 3/94,	Added local prototype descriptions in new
 *				include file, 'locp.h'.  Prototypes included
 *				in file, 'libloc.h'.
 *	Walter Nagy,  6/15/94,	Replaced ackward GEOCENTRIC_COLAT define with 
 *				new function, lat_conv(), a general purpose 
 *				routine for converting back and forth between
 *				geographic and geocentric latitude.
 *	Walter Nagy,  7/ 1/94,	Cutover to new T-T handling facilities using 
 *				structure, tt_table. 
 *	Walter Nagy,  7/23/94,	Major change.  Now pass db arrival and assoc 
 *				structures instead of a whole gaggle of arrays.
 *	Walter Nagy,  7/25/94,	Travel-time residuals no longer need to be
 *				recalculated after orthogonalizing out origin
 *				time.  This speeds up the locator by about 5%.
 *	Walter Nagy,  9/21/95,	wgt attribute field of assoc table now filled
 *				as inverse of model + measurement error.
 *	Walter Nagy,  1/ 9/96,	Re-written to pass new structure, ar_info,
 *				which contains arrival-based locator info.
 */


#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libloc.h"
#include "locp.h"
#include "libgeog.h"
#include "origerr_Astructs.h"
#include "assoc_Astructs.h"
#include "loc_defs.h"
#include "blk_defs.h"

#define	CONVG_TOL	1.0e-8		/* Tolerance for convergence checks */
#define	MIN_ITER	4		/* Minimum # of iterations required */
#define	MAX_HISTORY	3		/* History of standard errors and */
					/* perturbation vectors used to */

#define	CONVG_LM	5.0 * 1.0e-4	/* Tolerance for LM convergence checks; for depth * 10  */
#define	FIX_D		0		/* Minimum # of start iterations with fixed depth for LM */
#define SL_DEPTH	0.5 		/* Initial step to calculate slow_deriv regarding depth for LM */
#define RMS_MULT	2.0 
#define RMS_MAX		3.2		/* Max wt_rms for LM convergence checks*/

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

static double maxarg1,maxarg2;
#define DMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))



int
compute_hypo (Origin **w_origin, Origerr **w_origerr, Arrival *arrival, 
	      Assoc **w_assoc, Locator_params *locator_params, 
	      Locator_info *locator_info, Ar_Info **ar_info_ptr, 
	      Site *sites, int num_sites, int num_obs, FILE *ofp, 
	      int verbose, double *torg)
{
	Origin	*origin;
	Origerr	*origerr;
	Origerr Na_Origerr = Na_Origerr_Init;
	Assoc	*assoc;
	Assoc	Na_Assoc = Na_Assoc_Init;
	Ar_Info	*ar_info;

	Bool	use_azimuth, use_slow, use_time;
	Bool	do_we_need_z_derivs;
	int	i, ip, iterr, k, m, n, nd_used, np, num_tt_used;
	int	nds[MAX_HISTORY];
	double	Asum=0., azi, esr, cnvg01, cnvg12, cnvgtst, condit[2];
	double	dacc, delta, deltim, dist, dmean=0., dxmax;
	double	dxn01, dxn12, dxnorm, residual, rt, scale, ssq, total_tt;
	double	unwt_rms, unwt_tt_sqd_sum, wt_rms;
	double	blk_radius = DEG_TO_KM;
	double	*At, *data_import, *dsd2, *resid2;
	double	*az_used_in_loc;
	double	Amean[MAX_PARAM], az_deriv[MAX_PARAM], cnvghats[MAX_HISTORY];
	double	covar[MAX_PARAM][MAX_PARAM], dxnrms[MAX_HISTORY];
	double	prin_deriv[MAX_PARAM], slow_deriv[MAX_PARAM];
	double	tt_deriv[MAX_PARAM], xsol[MAX_PARAM];
	double	yold[MAX_PARAM], ysol[MAX_PARAM];
	double	slodel, slow_vec_res, slow;
	double	dist_min, dist_max, az_min, az_max;
	double	init[4];

	/* Initializations */

	Bool	convergence = FALSE;
	Bool	divergence  = FALSE;
	Bool	ldenuis     = FALSE;
	Bool	fix_origin_time = FALSE;
	Bool	fix_depth_this_iter = FALSE;
	int	num_air_quakes	= 0;
	int	num_too_deep	= 0;
	int	ierr	 = 0;
	int	num_iter = 0;
	double	step	 = 1.0;
	double	cnvgold	 = 1.0;
	Bool	lm_solution = FALSE;
	Bool	return_err = FALSE;


	origin	= *w_origin;
	origerr	= *w_origerr;
	assoc	= *w_assoc;
	ar_info	= *ar_info_ptr;

	/* Allocate locally necessary memory */

	At		= UALLOCA (double, 3*num_obs*MAX_PARAM);
	resid2		= UALLOCA (double, 3*num_obs);
	dsd2		= UALLOCA (double, 3*num_obs);
	data_import	= UALLOCA (double, 3*num_obs);
	az_used_in_loc	= UALLOCA (double, num_obs);


	/*
	 * Main iterative loop
	 */

again:
	for (n = 0; n < num_obs; n++)
	{
	    assoc[n].timeres = Na_Assoc.timeres;
	    assoc[n].azres   = Na_Assoc.azres;
	    assoc[n].slores  = Na_Assoc.slores;
	    assoc[n].wgt     = Na_Assoc.wgt;
	    strcpy (assoc[n].vmodel, "-");
	}
	for (n = 0; n < 3*num_obs; n++)
	{
	    resid2[n] = -999.0;
	    dsd2[n] = 1.0;
	    data_import[n] = -1.0;
	    if( !lm_solution )
	    {
		for (i = 0; i < locator_info->num_params; i++)
		    At[i + n*MAX_PARAM] = 0.0;
	    }
	}

	/*
	 * Currently, never permit depth to go negative (airquake).  Keep track
	 * of number of times this occurs (num_air_quakes) for a single free
	 * depth location.  If this happens more than 4 times, constrain depth
	 * to the surface for the remaining iterations.  Analogously, never
	 * permit event depth to exceed MAX_DEPTH, or the deepest credible
	 * hypocentral depth.
	 */

	if (origin->depth < 0.0)
	{
	    ++num_air_quakes;
	    origin->depth = 0.0;
	}
	else if (origin->depth > MAX_DEPTH)
	{
	    ++num_too_deep;
	    origin->depth = MAX_DEPTH;
	}

	/* 
	 * save initial parameters for LM
	 */
	 
	if(! ldenuis)
	{
	    init[0] = *torg;
	    init[1] = origin->lon;
	    init[2] = origin->lat;
	    init[3] = origin->depth;
	}
	
	/*
	 * Set fix-depth flag and number of parameters.  Depth is always fixed
	 * during the first 3 iterations (Note: the first iteration counts as
	 * iteration 0).  If depth becomes negative an "airquake" results (as
	 * tested immediately above).  In this case, fix the depth at 0.0 km.
	 * during next iteration. 
	 */
 
	if (num_iter < MIN_ITER-1)
	    fix_depth_this_iter = TRUE;
	else if (num_air_quakes > 4)
	{
	    fix_depth_this_iter = TRUE;
	    origin->depth = 0.0;
	}
	else if (num_too_deep > 4)
	{
	    fix_depth_this_iter = TRUE;
	    origin->depth = MAX_DEPTH;
	}
	else
	{
	    if (locator_params->fix_depth)
		fix_depth_this_iter = TRUE;
	    else
		fix_depth_this_iter = FALSE;
	}

	/*
	 * If fixing depth on this iteration, then we need not compute depth
	 * derivatives (i.e., do_we_need_z_derivs = FALSE).
	 */

	if (fix_depth_this_iter)
	    do_we_need_z_derivs = FALSE;
	else
	    do_we_need_z_derivs = TRUE;

	fix_origin_time = FALSE;
	if (locator_params->fix_origin_time)
	    fix_origin_time = TRUE;

	/*
	 * How many model parameters?
	 */

	if (fix_depth_this_iter && !locator_params->fix_depth)
	    np = locator_info->num_params - 1;
	else
	    np = locator_info->num_params;

	if (! ldenuis)
	{
	    Asum  = 0.0;
	    dmean = 0.0;
	    for (m = 0; m < np; m++)
		Amean[m] = 0.0;
	}

	/*
	 * Compute distances and azimuths to stations with data (forward 
	 * problem for azimuths)
	 */

	for (m = 0, n = 0; n < num_obs; n++)
	{
	    if ((i = locator_info->sta_index[n]) < 0)
	    {
		assoc[n].delta = Na_Assoc.delta;
		assoc[n].seaz  = Na_Assoc.seaz;
		assoc[n].esaz  = Na_Assoc.esaz;
	    }
	    else
	    {
		dist_azimuth (sites[i].lat, sites[i].lon, origin->lat, 
			      origin->lon, &assoc[n].delta, &assoc[n].seaz,
			      &assoc[n].esaz, m);
		++m;
	    }
	}

	/* 
	 * Compute travel-times, slownesses and azimuths based on current 
	 * location hypothesis and determine partial derivatives.  Ignore 
	 * points with completely invalid data (i.e., data_error_code = 1,
	 * 2, 3, 4).
	 */

	unwt_tt_sqd_sum = 0.0;
	for (n = 0, nd_used = 0, num_tt_used = 0; n < num_obs; n++)
	{
	    use_time    = TRUE;
	    use_slow    = TRUE;
	    use_azimuth = TRUE;
	    if(ar_info[n].time_error_code > 0 && ar_info[n].time_error_code < 5)
		use_time = FALSE;
	    if(ar_info[n].slow_error_code > 0 && ar_info[n].slow_error_code < 5)
		use_slow = FALSE;
	    if(ar_info[n].az_error_code > 0 && ar_info[n].az_error_code < 5)
		use_azimuth = FALSE;
	    if(!convergence && !divergence && strncmp (assoc[n].timedef, "d", 1))
		use_time = FALSE;
	    if(!convergence && !divergence && strncmp (assoc[n].slodef, "d", 1))
		use_slow = FALSE;
	    if(!convergence && !divergence && strncmp (assoc[n].azdef, "d", 1))
		use_azimuth = FALSE;

	    if (use_time || use_slow)
	    {
		/*
		 * If this is a hydroacoustic phase, check for blockage and
		 * possibility that delta > 180 deg.  If the latter case
		 * is TRUE, then adjust delta, esaz and seaz variables of
		 * assoc table entry.
		 */

		if (STREQ (assoc[n].phase, "H") || STREQ (assoc[n].phase, "T")
		    || STREQ (assoc[n].phase, "HT") 
		    || STREQ (assoc[n].phase, "HO"))
		{
		    if (origerr->smajax > 0.0 && origerr->sminax > 0.0)
		    {
			blocked_ellipse_dist (assoc[n].sta, origin->lat, 
					      origin->lon, origerr->smajax, 
					      origerr->sminax, origerr->strike, 
					      &delta, &dist_min, &dist_max, 
					      &az_min, &az_max);
		    }
		    else
		    {
			blk_radius = get_blk_radius ();
			blocked_ellipse_dist (assoc[n].sta, origin->lat, 
					      origin->lon, blk_radius, 
					      blk_radius, 0.0, &delta, 
					      &dist_min, &dist_max, &az_min, 
					      &az_max);
		    }
		    
		    if (delta > 180.0 || (dist_min > 180.0 && dist_max > 180.0))
		    {
			assoc[n].delta = 360.0 - assoc[n].delta;
			assoc[n].seaz += 180.0;
			assoc[n].esaz += 180.0;
			if (assoc[n].seaz > 360.0)
			    assoc[n].seaz -= 360.0;
			if (assoc[n].esaz > 360.0)
			    assoc[n].esaz -= 360.0;
		    }
		}

		/*
		 * Determine travel time, azimuth and slowness along 
		 * with derivatives and any necessary corrections.
		 */

		total_tt = 
		total_travel_time (locator_params, sites, &ar_info[n], TRUE, 
				   do_we_need_z_derivs, origin->lat, 
				   origin->lon, origin->depth, assoc[n].delta, 
				   assoc[n].esaz, assoc[n].phase, 
				   locator_info->sta_index[n], 
				   locator_info->phase_index[n], 
				   locator_info->spm_index[n], 
				   arrival[n].deltim, prin_deriv,
				   tt_deriv, slow_deriv, az_deriv, &iterr);

		ar_info[n].orid = origin->orid;
		ar_info[n].arid = assoc[n].arid;
		if (total_tt > 0.0)
		{
		    /*
	 	     * Compute travel-time residual:
		     *
		     * data    = t    - t	  = obs_data - total_tt
		     *     res    obs    calc
		     *
		     * where, t     = t      + t
		     *         calc    table    corr
		     *
		     * and, t	is the sum of all travel-time
		     *       corr	corrections (i.e., SSSC or SRST
		     *		or test-site, and elevation and
		     *		ellipticity) as determined in
		     *		function, total_travel_time().
		     */

		    assoc[n].timeres = arrival[n].time - total_tt - *torg;

		    /*
		     * Compute slowness residual = (obs - calc) datum
		     */

		    assoc[n].slores = arrival[n].slow - prin_deriv[0];

		    strcpy (assoc[n].vmodel, ar_info[n].vmodel);
		}
		else
		{
		    assoc[n].timeres = Na_Assoc.timeres;
		    assoc[n].slores = Na_Assoc.slores;
		    strcpy (assoc[n].vmodel, "-");
		}

		assoc[n].wgt = 1.0/ar_info[n].model_plus_meas_error;

 		if (total_tt < 0.0)
                {
                        ar_info[n].time_error_code = 8;
                        ar_info[n].slow_error_code = 8;
			use_time = FALSE;
			use_slow = FALSE;
                }
		else if (num_iter < MIN_ITER)
		{
		    if (use_time)
			ar_info[n].time_error_code = 0;
		    if (use_slow)
			ar_info[n].slow_error_code = 0;
		}
		else
		{
		    if (use_time)
			ar_info[n].time_error_code = iterr;
		    if (use_slow)
			ar_info[n].slow_error_code = iterr;
		}

		/*
		 * If requested, use only travel-time data with SSSC, SRST, 
		 * or test-site corrections available.  Signify this by 
		 * setting data error code to 6 when correction not found.
		 * Make sure at least 2 iterations have already been performed.
		 */

		if (use_time && locator_params->use_only_sta_w_corr && 
		    num_iter > MIN_ITER-2 && 
		    ar_info[n].src_dpnt_corr_type == NO_SRC_DPNT_CORR)
		    ar_info[n].time_error_code = 6;

		if (use_slow && locator_params->use_only_sta_w_corr && 
		    num_iter > MIN_ITER-2)
		    ar_info[n].slow_error_code = 6;
		    		    
		/*
		 * If user sets locator_params->ignore_large_res = TRUE,
		 * then, do not include any data where data residual,
		 * data_residual[], is > pre-defined muliplier factor, 
		 * locator_params->large_res_mult, times the a priori
		 * data standard error, data_std_err[].  Do not apply
		 * this screening if depth is being freed for the first
		 * time.
		 */

		if (locator_params->ignore_large_res && 
		    ((fix_depth_this_iter && num_iter > MIN_ITER-2) || 
		     (!fix_depth_this_iter && num_iter >= MIN_ITER)))
		{
		    if (use_time && fabs(assoc[n].timeres) > 
			    locator_params->large_res_mult *
					ar_info[n].model_plus_meas_error)
			ar_info[n].time_error_code = 5;
		    if (use_slow && fabs(assoc[n].slores) > 
			    locator_params->large_res_mult*arrival[n].delslo)
			ar_info[n].slow_error_code = 5;
		}

		/*
		 * Store travel-time derivatives in A-transpose (At)
		 * Note that parameters in At are ordered as: origin-time; 
		 * longitude; latitude; depth.  Also normalize matrix and 
		 * residuals w.r.t. data variances.
		 */

		if (use_time && ar_info[n].time_error_code < 1 &&
		    STREQ (assoc[n].timedef, "d"))
		{
		  if( !lm_solution )
		  {
		    ip = 0;
		    if (! fix_origin_time)
		    {
			At[nd_used*MAX_PARAM] = tt_deriv[0];
			++ip;
		    }
		    if (! locator_params->fix_lat_lon)
		    {
			At[ip + nd_used*MAX_PARAM] = tt_deriv[1];
			++ip;
			At[ip + nd_used*MAX_PARAM] = tt_deriv[2];
			++ip;
		    }
		    if (! fix_depth_this_iter)
			At[ip + nd_used*MAX_PARAM] = tt_deriv[3];
		  }
		    residual = assoc[n].timeres;
		    if (! ldenuis && *torg == 0.0)
		    {
			deltim = arrival[n].deltim;
			dacc = 1.0/(deltim*deltim);
			for (m = 1; m < np; m++)
			    Amean[m] += dacc*At[m + nd_used*MAX_PARAM];
			Asum  += dacc;
			dmean += dacc*residual;
		    }

		    resid2[nd_used] = residual;
		    dsd2[nd_used] = ar_info[n].model_plus_meas_error;
		    az_used_in_loc[num_tt_used] = assoc[n].esaz;
		    unwt_tt_sqd_sum += residual*residual;

		    ++num_tt_used;
		    ++nd_used;	/* Actual # data to be used */
		}

		/*
		 * Store slowness derivatives in A-transpose (At)
		 */

		if (use_slow && ar_info[n].slow_error_code < 1 &&
			    STREQ (assoc[n].slodef, "d"))
		{
		  if( !lm_solution )
		  {
		    ip = 0;
		    if (! fix_origin_time)
		    {
			At[nd_used*MAX_PARAM] = slow_deriv[0];
			++ip;
		    }
		    if (! locator_params->fix_lat_lon)
		    {
			At[ip + nd_used*MAX_PARAM] = slow_deriv[1];
			++ip;
			At[ip + nd_used*MAX_PARAM] = slow_deriv[2];
			++ip;
		    }
		    if (! fix_depth_this_iter)
			At[ip + nd_used*MAX_PARAM] = slow_deriv[3];
		  }
		    resid2[nd_used] = assoc[n].slores;
		    dsd2[nd_used] = arrival[n].delslo;

		    ++nd_used;	/* Actual # data to be used */
		}
	    }

	    /*
	     * Deal with azimuth seperately in case only azimuth data is
	     * needed, then total_travel_time() need not be call at all.
	     * If total_travel_time() has been called already, then you
	     * have the azimuth derivatives already.
	     */

	    if (use_azimuth)
	    {
		/* Apply AZ SSSC adjustments, if necessary */

		/*
		 * Commented out until I (WCN) can resolve how to
		 * handle re-populating assoc[n].seaz & assoc[n].esaz.
		 * It really doesn't make sense to re-set this from
		 * the database description.  On the other hand, if
		 * one doesn't reset these variables than SSSC really
		 * can't be applied.
		 *

		if (num_iter > MIN_ITER-2 && locator_params->sssc_level > 0)
		{
		    azi = assoc[n].seaz;
		    baz = assoc[n].esaz;
		    i = locator_info->sta_index[n];
		    k = locator_info->phz_index[n];
		    sssc_found = apply_sssc (sssc, origin->lat, origin->lon, 
					     &correct, 1, i, k);
		    azi = azi + correct;
		    baz = baz + correct;
		    if (azi < 0.0)
			azi += 360.0;
		    if (baz < 0.0)
			baz += 360.0;
		    if (azi > 360.0)
			azi -= 360.0;
		    if (baz > 360.0)
			baz -= 360.0;
		    assoc[n].seaz = azi;
		    assoc[n].esaz = baz;

		     *
		     * Use only azimuth data with SSSC's?
		     * 

		    if (locator_params->use_only_sta_w_corr 
			&& num_iter > MIN_ITER-2 && !sssc_found)
			data_error_code[n] = 6;
		}
			 */

		if (! use_time && ! use_slow)
		{
			/*
			 * Special case: Only azimuth data available
			 */

			esr = assoc[n].esaz*DEG_TO_RAD;
			rt  = sin(assoc[n].delta*DEG_TO_RAD)*
			      RADIUS_EARTH*DEG_TO_RAD;
			if (rt == 0.0)
			    rt = 0.0001;
			az_deriv[0] = 0.0;
			az_deriv[1] = -cos(esr)/rt;
			az_deriv[2] = sin(esr)/rt;
			az_deriv[3] = 0.0;
		}

		/*
	 	 * Compute azimuth residual = 
		 *	(obs - calc) datum	[Note that azimuthal
		 *	SSSC has already been removed]
		 */

		assoc[n].azres = arrival[n].azimuth - assoc[n].seaz;
		if (fabs(assoc[n].azres) > 180.0)
		    assoc[n].azres = SIGN( (360.0-fabs(assoc[n].azres)),
						  assoc[n].azres );

		/*
		 * If user sets locator_params->use_only_sta_w_corr = TRUE
		 * then, do not include any azimuth data.
		 */

		if (locator_params->use_only_sta_w_corr && num_iter > MIN_ITER-2) 
		    ar_info[n].az_error_code = 6;

		/*
		 * If user sets locator_params->ignore_large_res = TRUE,
		 * then, do not include any azimuth data where its 
		 * residual, is > pre-defined muliplier factor, 
		 * locator_params->large_res_mult, times delaz.
		 */

		if (num_iter > MIN_ITER-2 && locator_params->ignore_large_res)
		{
		    if (fabs(assoc[n].azres) > 
		            locator_params->large_res_mult* arrival[n].delaz)
		    ar_info[n].az_error_code = 5;
		}

		/*
		 * Store azimuth derivatives in A-transpose (At)
		 */

		if (ar_info[n].az_error_code < 1 && STREQ (assoc[n].azdef, "d"))
		{
		  if( !lm_solution )
		  {
		    ip = 0;
		    if (! fix_origin_time)
		    {
			At[nd_used*MAX_PARAM] = az_deriv[0];
			++ip;
		    }
		    if (! locator_params->fix_lat_lon)
		    {
			At[ip + nd_used*MAX_PARAM] = az_deriv[1];
			++ip;
			At[ip + nd_used*MAX_PARAM] = az_deriv[2];
			++ip;
		    }
		    if (! fix_depth_this_iter)
			At[ip + nd_used*MAX_PARAM] = az_deriv[3];
		  }
		    resid2[nd_used] = assoc[n].azres;
		    dsd2[nd_used] = arrival[n].delaz;

		    ++nd_used;	/* Actual # data to be used */
		}
	    }
	}


	/*
	 * Check for insufficient data.  Flag with proper informative
	 * error message, if error is encountered.
	 * ierr = 4:  Insufficient defining data due to hole in T-T table(s)
	 * ierr = 5:  Insufficient defining data due to T-T table extrapolation
	 * ierr = 23: Insufficient defining data due to large res. restriction
	 * ierr = 24: Insufficient defining data due to use only stations with
	 *	      source-dependent corrections restriction
	 * ierr = 25: Insufficient defining data found while in iterative
	 *	      event location loop
	 */

	if (nd_used < np)
	{
	    ierr = GLerror16;
	    for (n = 0; n < num_obs; n++)
	    {
		if (ar_info[n].time_error_code == 5 ||
		    ar_info[n].az_error_code == 5 ||
		    ar_info[n].slow_error_code == 5)
		{
		    ierr = GLerror14;
		    break;
		}
		else if (ar_info[n].time_error_code == 6 ||
			 ar_info[n].az_error_code == 6 ||
			 ar_info[n].slow_error_code == 6)
		{
		    ierr = GLerror15;
		    break;
		}
		else if (ar_info[n].time_error_code == 11 ||
			 ar_info[n].slow_error_code == 11)
		{
		    ierr = GLerror4;
		    break;
		}
		else if (ar_info[n].time_error_code > 11 ||
			 ar_info[n].slow_error_code > 11)
		{
		    ierr = GLerror5;
		    break;
		}
	    }
	    return_err = TRUE;
	    goto LM;		/* try to use LM method */
	}

	/*
	 * If initial iteration and no original origin time is provided,
	 * then orthogonalize out origin-time term
	 */

	if (! ldenuis && *torg == 0.0)
	{
	    if (Asum > 0.0)
	    {
		for (m = 1; m < np; m++)
				Amean[m] /= Asum;
		*torg = dmean/Asum;
		init[0] = *torg;

		for (k = 0, n = 0; n < num_obs; n++)
		{
		    if (ar_info[n].time_error_code < 1 && 
			STREQ (assoc[n].timedef, "d"))
		    {
			for (m = 1; m < np; m++)
			{
			    if (At[m + k*MAX_PARAM] != 0.0)
				At[m + k*MAX_PARAM] -= Amean[m];
			}
			resid2[k] -= *torg;
			assoc[n].timeres = resid2[k];
			++k;
		    }
		    if (ar_info[n].slow_error_code < 1 && 
			    STREQ (assoc[n].slodef, "d"))
			++k;
		    if (ar_info[n].az_error_code < 1 && 
			    STREQ (assoc[n].azdef, "d"))
			++k;
		}
	    }
	    else
		*torg = 0.0;

	    ldenuis = TRUE;
	}
	ldenuis = TRUE;

	/*
	 * Normalize derivative matrix and residual vector by the data
	 * standard errors.  Also compute weighted and unweighted RMS 
	 * residual (dimensionless quantities).  
	 */

	unwt_rms = 0.0;
	wt_rms = 0.0;
	for (n = 0; n < nd_used; n++)
	{
	    unwt_rms += resid2[n]*resid2[n];
	    resid2[n] /= dsd2[n];
	    wt_rms += resid2[n]*resid2[n];
	    for (m = 0; m < np; m++)
		At[m + n*MAX_PARAM] /= dsd2[n];
	}
	unwt_rms = unwt_rms/nd_used;
	wt_rms = wt_rms/nd_used;
	unwt_rms = sqrt(unwt_rms);
	wt_rms   = sqrt(wt_rms);

	unwt_tt_sqd_sum = unwt_tt_sqd_sum/num_tt_used;
	if (num_tt_used > 0)
	    origerr->sdobs = sqrt(unwt_tt_sqd_sum);
	else
	    origerr->sdobs = Na_Origerr.sdobs;

	/*
	 * If convergence has been reached, break out of main iterative loop.
	 * This is where the real test is made to terminate iteration.
	 * Residuals must always be calculated one final time before the
	 * location can be thought of as complete.  This is true whether or
	 * not convergent, divergent or other errors are encountered.  This
	 * is the ONE and ONLY break point unless the SVD routine fails
	 * (i.e., ierr = GLerror6).
	 */

	if (convergence || divergence) 
	    goto wrap_it_up;


	for (m = 0; m < MAX_PARAM; m++)
	{
	    xsol[m] = 0.0;	/* Compacted solution vector from dsvdc() */
	    ysol[m] = 0.0;	/* Actual solution vector */
	}

	/*
	 * Determine the least squares solution
	 */

	if ((solve_via_svd (1, nd_used, np, 4, At, resid2, locator_params->damp,
			    &cnvgtst, condit, xsol, covar, data_import,
			    &locator_info->applied_damping, 
			    &locator_info->rank)) == 6)
	{
	    locator_info->nd_used = nd_used;
	    locator_params->num_iter = num_iter;
	    return (GLerror6);
	}

	/*
	 * Print solution information at each iterative step
	 */

	if (verbose == 3 || verbose == 4)
	{
	    fprintf (ofp, "\n\n- Iteration # %3d   Number of Obs. (Data): %3d\n", num_iter, nd_used);
	    fprintf (ofp, "- Lat:%8.3f   Lon:%9.3f   Depth:%9.3f   To:%10.3f\n", origin->lat, origin->lon, origin->depth, *torg);
	    fprintf (ofp, "- Unwt. RMS Res.:%8.4f   Wt. RMS Res.:%8.4f   CNVGTST:%12.5e\n\n", unwt_rms, wt_rms, cnvgtst);
	    fprintf (ofp, "                 Phase       TT(t)/Azi(a)/Slo(s)       Residuals       Delta\n");
	    fprintf (ofp, "     Arid Sta    Type        Observed Predicted      True  Normalzd    (deg)\n");
	    fprintf (ofp, "--------- ------ -------- -  -------- ---------  --------  --------  -------\n");
	    for (m = 0, n = 0; n < num_obs; n++)
	    {
		if (ar_info[n].time_error_code < 1 && 
		    STREQ (assoc[n].timedef, "d"))
		{
		    fprintf (ofp, "%9ld %-6s %-8s t%10.3f%10.3f%10.3f%10.3f%9.3f\n",
			     arrival[n].arid, assoc[n].sta, assoc[n].phase, 
			     arrival[n].time, arrival[n].time-assoc[n].timeres,
			     assoc[n].timeres, resid2[m], assoc[n].delta);
		    ++m;
		}
		if (ar_info[n].slow_error_code < 1 && 
		    STREQ (assoc[n].slodef, "d"))
		{
		    fprintf (ofp, "%9ld %-6s %-8s s%10.3f%10.3f%10.3f%10.3f%9.3f\n",
			     arrival[n].arid, assoc[n].sta, assoc[n].phase, 
			     arrival[n].slow, arrival[n].slow - assoc[n].slores,
			     assoc[n].slores, resid2[m], assoc[n].delta);
		    ++m;
		}
		if (ar_info[n].az_error_code < 1 && STREQ (assoc[n].azdef, "d"))
		{
		    fprintf (ofp, "%9ld %-6s %-8s a%10.3f%10.3f%10.3f%10.3f%9.3f\n",
			     arrival[n].arid, assoc[n].sta, assoc[n].phase, 
			     arrival[n].azimuth,
			     arrival[n].azimuth - assoc[n].azres,
			     assoc[n].azres, resid2[m], assoc[n].delta);
		    ++m;
		}
	    }
	}

	/*
	 * Compute norm of hypocenter perturbations
	 */

	ssq = 0.0;
	for (m = 0; m < np; m++)
	    ssq += xsol[m]*xsol[m];
	dxnorm = sqrt(ssq);

	/*
	 * Scale down hypocenter perturbations if they are very large.  Scale
	 * down even more for lat(t)er iterations.
	 */

	dxmax = 1500.0;
	if (num_iter < locator_params->max_iterations/5+1)
	    dxmax = 3000.0;
	if (dxnorm > dxmax)
	{
	    scale = dxmax/dxnorm;
	    for (m = 0; m < np; m++)
		xsol[m] *= scale;
	    dxnorm = dxmax;
	}


	/* 
	 * Uncompact solution vector
	 */

	ip = 0;
	if (! fix_origin_time)
	{
	    ysol[0] = xsol[0];
	    ++ip;
	}
	if (! locator_params->fix_lat_lon)
	{
	    ysol[1] = xsol[ip];
	    ++ip;
	    ysol[2] = xsol[ip];
	    ++ip;
	}
	if (! fix_depth_this_iter)
	    ysol[3] = xsol[ip];


	if (verbose == 3 || verbose == 4)
	{
	    fprintf (ofp, "\n> dLat:%9.3f   dLon:%9.3f   dZ:%9.3f  dOT:%9.3f\n", ysol[2], ysol[1], ysol[3], ysol[0]);
	    fprintf (ofp, "> True Cond. Num.:%10.3g   Effective Cond. Num.:%10.3g\n", condit[0], condit[1]);
	}

	/*
	 * Store the convergence test info from the 2 previous iterations
	 */

	for (i = MIN(MAX_HISTORY-1, num_iter); i > 0; i--)
	{
	    cnvghats[i] = cnvghats[i-1];
	    dxnrms[i]   = dxnrms[i-1];
	    nds[i]	    = nds[i-1];
	}

	/* Current convergence test information */

	nds[0]	    = nd_used;
	cnvghats[0] = cnvgtst;
	dxnrms[0]   = dxnorm;

	/*
	 * Convergence, divergence or just keep on iterating
	 */

	if (num_iter > MIN_ITER-1)
	{
	    if (dxnorm > 0.0 && cnvgtst > 0.0)
	    {
		if (dxnrms[1] <= 0.0 || dxnrms[2] <= 0.0)
		{
		    dxn01 = 1.05;
		    dxn12 = 1.05;
		}
		else
		{
		    dxn01 = dxnrms[0]/dxnrms[1];
		    dxn12 = dxnrms[1]/dxnrms[2];
		}
		if ( (dxn12 > 1.1 && dxn01 > dxn12) &&
		     (num_iter > MIN_ITER+2) && (dxnorm > 1000.0))
		{
		    divergence = TRUE;
		    goto slw;
		}
		if ((nds[0] == nds[1]) && (cnvgtst < CONVG_TOL || dxnorm < 0.5))
		{
		    convergence = TRUE;
		    goto slw;
		}
		if ((wt_rms < 0.001 || dxnorm < 0.001) && num_iter > MIN_ITER+2)
		{
		    convergence = TRUE;
		    goto slw;
		}

		if (cnvghats[1] <= 0.0 || cnvghats[2] <= 0.0)
		    cnvg01 = cnvgtst;
		else
		{
		    cnvg01 = cnvghats[0]/cnvghats[1];
		    cnvg12 = cnvghats[1]/cnvghats[2];
		    cnvg01 = fabs(cnvg12 - cnvg01);
		}
		cnvg12 = fabs(cnvghats[0]-cnvghats[2]);
		if ( ((cnvgtst < 1.01*cnvgold) && (cnvgtst < CONVG_TOL))
		     || (num_iter > 3*locator_params->max_iterations/4
			 && (cnvgtst < sqrt(CONVG_TOL)
		         || cnvg01 < CONVG_TOL || cnvg12 < sqrt(CONVG_TOL))) )
		{
		    convergence = TRUE;
		    goto slw;
		}
	    }
	    else
		convergence = TRUE;
	}

	/*
	 * Apply step-length weighting, if unweighted RMS residual 
	 * is increasing.  Steps are applied in half-lengths of the 
	 * previous solution vector.
	 */

slw:
	if (num_iter > MIN_ITER+2 && (cnvgtst > cnvgold ||
	    cnvghats[0]-cnvghats[2] == 0.0) && step > 0.05)
	{
	    step = 0.5*step;
	    if (step != 0.5)
	    {
		for (i = 0; i < np; i++)
		    ysol[i] = step*yold[i];
	    }
	    else
	    {
		for (i = 0; i < np; i++)
		{
		    ysol[i] = step*ysol[i];
		    yold[i] = ysol[i];
		}
	    }
	}
	else
	{
	    step    = 1.0;
	    cnvgold = cnvgtst;
	}

	/*
	 * Perturb (adjust) hypocentral location based on the solution derived
	 * during this iteration
	 */

	if (ysol[1] != 0.0 || ysol[2] != 0.0)
	{
	    azi	  = RAD_TO_DEG*atan2(ysol[1], ysol[2]);
	    dist  = ysol[1]*ysol[1] + ysol[2]*ysol[2];
	    dist  = sqrt(dist);
	    delta = RAD_TO_DEG*(dist/(RADIUS_EARTH-origin->depth));
	    lat_lon (origin->lat, origin->lon, delta, azi,
		     &origin->lat, &origin->lon);
	}
	*torg += ysol[0];
	if (! fix_depth_this_iter)
	    origin->depth -= ysol[3];

	/*
	 * End of main iterative loop!   Check for realized convergence or
	 * divergence of solution.  One final calculation for data residuals,
	 * distances, azimuths, etc. will be needed if either the convergence
	 * or divergence criterion is met.
	 */

	if (convergence)
	{
	    ierr = 0;
	    if (condit[0] > COND_NUM_LIMIT)
	    {
		ierr = GLerror7;
		divergence = TRUE;	/* Kludgy to exit properly */
	    }
	}
	else if (divergence)
	    ierr = GLerror2;
	else if (num_iter >= locator_params->max_iterations)
	{
	    ierr = GLerror1;
	    divergence = TRUE;	/* Kludgy to exit properly */
	}
	else
	    ++num_iter;


	/*
	 * Levenberg-Marquardt method
	 */
	
LM:
	if( return_err )
	    divergence = TRUE;
	
	if (convergence || divergence)
	{
	    lm_solution = compute_hypo_lm (origin, origerr, arrival, assoc, locator_params,
	    		  locator_info, ar_info, sites, num_sites, num_obs, ofp, verbose, torg, 
	      		  At, resid2, dsd2, az_used_in_loc, init, divergence);

	    if( lm_solution != 0)
	    {
		return_err = FALSE;
		if(lm_solution == 1)
		{
		    convergence = TRUE;
		    divergence = FALSE;
		    ierr = OK;
		}
		else
		{
		    convergence = FALSE;
		    divergence = TRUE;
		}
		if(origin->depth == 0.0 || origin->depth == MAX_DEPTH)
		{
		    fix_depth_this_iter = TRUE;
		    if(origin->depth == 0.0)
		    {
			num_air_quakes = 5;
			num_too_deep = 0;
		    }
		    if(origin->depth == MAX_DEPTH)
		    {
			num_too_deep = 5;
			num_air_quakes = 0;
		    }
		}
		else
		{
		    fix_depth_this_iter = FALSE;
		    num_too_deep = 0;
		    num_air_quakes = 0;
		}
	    }
	    else
	    {
		if( return_err )	/* LM as SVD could not solve problem */
		{
		    locator_info->nd_used = nd_used;
		    locator_info->np = np;
		    locator_params->num_iter = num_iter;
		    return (ierr);
	    	}
	    }
	}

	goto again;	/* Remember: Always re-calc residuals one last time */


wrap_it_up:

	/*
	 * Calculate the sum of weighted squared data residuals and store
	 * the number of parameters solved for here.
	 */

	locator_info->sum_sqrd_res = 0.0;
	for (n = 0; n < nd_used; n++)
	    locator_info->sum_sqrd_res += resid2[n]*resid2[n];
	locator_info->np = np;

	/*
	 * Compute covariance and data resolution matrices.  Only use diagonal
	 * of data resolution matrix (these are the data importances).
	 */

	if ((solve_via_svd (2, nd_used, np, 4, At, resid2, locator_params->damp,
			    &cnvgtst, condit, xsol, covar, data_import,
			    &locator_info->applied_damping,
			    &locator_info->rank)) == 6)
	{
	    locator_info->nd_used	 = nd_used;
	    locator_params->num_iter = num_iter;
	    return (GLerror6);
	}


	/*
	 * Uncompact and fill origerr structure, as necessary.
	 */

	origerr->stt = Na_Origerr.stt;
	origerr->stx = Na_Origerr.stx;
	origerr->sty = Na_Origerr.sty;
	origerr->stz = Na_Origerr.stz;
	origerr->sxx = Na_Origerr.sxx;
	origerr->sxy = Na_Origerr.sxy;
	origerr->sxz = Na_Origerr.sxz;
	origerr->syy = Na_Origerr.syy;
	origerr->syz = Na_Origerr.syz;
	origerr->szz = Na_Origerr.szz;
	if (! fix_origin_time)
	{
	    origerr->stt = covar[0][0];
	    if (! locator_params->fix_lat_lon)
	    {
		origerr->stx = covar[0][1];
		origerr->sty = covar[0][2];
		origerr->sxx = covar[1][1];
		origerr->sxy = covar[1][2];
		origerr->syy = covar[2][2];
		if (! fix_depth_this_iter)
		{
		    origerr->stz = covar[0][3];
		    origerr->sxz = covar[1][3];
		    origerr->syz = covar[2][3];
		    origerr->szz = covar[3][3];
		}
	    }
	    else
	    {
		if (! fix_depth_this_iter)
		{
		    origerr->stz = covar[0][1];
		    origerr->szz = covar[1][1];
		}
	    }
	}
	else
	{
	    if (! locator_params->fix_lat_lon)
	    {
		origerr->sxx = covar[0][0];
		origerr->sxy = covar[0][1];
		origerr->syy = covar[1][1];
		if (! fix_depth_this_iter)
		{
		    origerr->sxz = covar[0][2];
		    origerr->syz = covar[1][2];
		    origerr->szz = covar[2][2];
		}
	    }
	    else
		if (! fix_depth_this_iter)
		    origerr->szz = covar[0][0];
	}
	origerr->conf = locator_params->conf_level;


	/*
	 * Place input values back into original arrays
	 */

	for (m = 0, n = 0; n < num_obs; n++)
	{
	    if (ar_info[n].time_error_code < 1 && STREQ (assoc[n].timedef, "d"))
	    {
		ar_info[n].time_import = data_import[m];
		++m;
	    }
	    if (ar_info[n].slow_error_code < 1 && STREQ (assoc[n].slodef, "d"))
	    {
		ar_info[n].slow_import = data_import[m];
		++m;
	    }
	    if (ar_info[n].az_error_code < 1 && STREQ (assoc[n].azdef, "d"))
	    {
		ar_info[n].az_import = data_import[m];
		++m;
	    }
	}

	/*
	 * Compute azimuthal GAP (deg.) for this event
	 */

	/* simple sort routine */

        for (k = 0; k < num_tt_used-1; k++)
	{
            for (n = k+1; n < num_tt_used; n++)
	    {	
                if (az_used_in_loc[k] > az_used_in_loc[n])
                {
                    azi = az_used_in_loc[k];
                    az_used_in_loc[k] = az_used_in_loc[n];
                    az_used_in_loc[n] = azi;
                }
            } 
        }

	for (n = 1; n < num_tt_used; n++)
	    if ((az_used_in_loc[n]-az_used_in_loc[n-1] > 
						locator_info->azimuthal_gap))
		locator_info->azimuthal_gap = 
					az_used_in_loc[n]-az_used_in_loc[n-1];
	azi = 360.0 - az_used_in_loc[num_tt_used-1] + az_used_in_loc[0];
	if (azi > locator_info->azimuthal_gap)
	    locator_info->azimuthal_gap = azi;
	if (locator_info->azimuthal_gap < 0.0 || locator_info->azimuthal_gap > 360.0)
	    locator_info->azimuthal_gap = 360.0;

	/* Store true matrix condtion number */
	locator_info->condition_number = condit[0];    

	if (origin->depth < 0.0)	/* Don't return a depth value < 0.0 */
	    origin->depth = 0.0;

	/*
	 * Calculate and store in ar_info structure, the residual of
	 * vector slowness (i.e., ar_info[n].slow_vec_res).
	 */

	for (n = 0; n < num_obs; n++)
	{
	    ar_info[n].slow_vec_res = -999.0;
	    if (VALID_SEAZ(arrival[n].azimuth) && VALID_SLOW(arrival[n].slow))
	    {
		if (assoc[n].slores != -999.0)
		{
		    slow = arrival[n].slow;
		    slodel = slow - assoc[n].slores;
		    slow_vec_res = 
			slow*slow + slodel*slodel - 
			2.0*slow * slodel * cos(DEG_TO_RAD*assoc[n].azres);
		    ar_info[n].slow_vec_res = sqrt(fabs(slow_vec_res));
		}
	    }
	}

	/* Make final error checks */

	for (n = 0; n < num_obs; n++)
	{
	    if (ar_info[n].time_error_code > 0)
	    {
		strcpy (assoc[n].timedef, "n");
		if (ar_info[n].time_error_code < 4)
		{
		    assoc[n].timeres = Na_Assoc.timeres;
		    assoc[n].wgt = Na_Assoc.wgt;
		}
	    }
	    if (ar_info[n].az_error_code > 0)
	    {
		strcpy (assoc[n].azdef, "n");
		if (ar_info[n].az_error_code < 5)
		    assoc[n].azres = Na_Assoc.azres;
	    }
	    if (ar_info[n].slow_error_code > 0)
	    {
		strcpy (assoc[n].slodef, "n");
		if (ar_info[n].slow_error_code < 5)
		    assoc[n].slores = Na_Assoc.slores;
	    }
	}

	/*
	 * Make sure to set number of iterations and number of data actually
	 * used in their respective return structures.
	 */

	locator_info->nd_used	 = nd_used;
	locator_params->num_iter = num_iter;

	return (ierr);
}

int
compute_hypo_lm (Origin *origin, Origerr *origerr, Arrival *arrival, 
	      Assoc *assoc, Locator_params *locator_params, 
	      Locator_info *locator_info, Ar_Info *ar_info, 
	      Site *sites, int num_sites, int num_obs, FILE *ofp, 
	      int verbose, double *torg, double *At, double *resid2,
	      double *dsd2, double *az_used_in_loc, double *init, Bool svd_conv)
{
	lm_params	lm_par;
	int	j, k, l, ret, i, n, max_iter;
	double	*atry, *beta, *da, **oneda;
	double	delta=0, azi, dist;
	int	last_nd, nd_svd, iter;
	double	alamda=-0.5, alamda_last;

	double	**covar;
	double	**alpha;
	double	*a;

	double	wt_rms, unwt_rms, last_wt, res_wt;
	double	svd_rms, svd_unwt_rms, svd_sdobs;
	double orig[4], orig_svd[4], convg;
	double	depth[] = {-1.0, -1.0, 0.0, 380.0, MAX_DEPTH, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}; /* two first and two last are reserved */
	int	best_ind = 0, num_depth = 11, num_test_depth = 8;
	best_solution	best, depths[8];

	Bool	ignore_large_res;
	Bool	use_only_sta_w_corr;
	Bool	sec_test = FALSE;
	Bool	depth_test = FALSE;

	lm_par.origin = origin;
	lm_par.origerr = origerr;
	lm_par.arrival = arrival;
	lm_par.assoc = assoc;
	lm_par.locator_params = locator_params;
	lm_par.locator_info = locator_info;
	lm_par.ar_info = ar_info;
	lm_par.sites = sites;
	lm_par.num_sites = num_sites;
	lm_par.num_obs = num_obs;
	lm_par.torg = torg;
	lm_par.At = At;
	lm_par.resid2 = resid2;
	lm_par.dsd2 = dsd2;
	lm_par.az_used_in_loc = az_used_in_loc;
	lm_par.wt_rms = &wt_rms;
	lm_par.unwt_rms = &unwt_rms;
	lm_par.iter = MIN_ITER + 1;	/* only to save svd data */
	lm_par.fix_depth_this_iter = FALSE;

	/*
	 * save SVD location data
	 */

	lm_par.np = locator_info->num_params;

	orig_svd[0] = *torg;
	orig_svd[1] = origin->lon;
	orig_svd[2] = origin->lat;

	if(origin->depth <= 0.0)
	{
	    origin->depth = 0.0;
	    lm_par.np = locator_info->num_params - 1;
	    lm_par.fix_depth_this_iter = TRUE;
	}
	else
	    if(origin->depth >= MAX_DEPTH)
	    {
		origin->depth = MAX_DEPTH; 
		lm_par.np = locator_info->num_params - 1;
		lm_par.fix_depth_this_iter = TRUE;
	    }
	orig_svd[3] = origin->depth;

	if(get_resids_and_derivs(&lm_par, FALSE) == OK)
	{
	    svd_sdobs = origerr->sdobs;
	    svd_rms = wt_rms;
	    svd_unwt_rms = unwt_rms;
	    nd_svd = lm_par.nd;
	}
	else
	{
	    svd_sdobs = 999.0;
	    svd_rms = 999999.9;
	    svd_unwt_rms = 999.0;
	    nd_svd = 0;	
	}
	
	/* save two locating params */
	
	ignore_large_res = locator_params->ignore_large_res;
	locator_params->ignore_large_res = FALSE;
	use_only_sta_w_corr = locator_params->use_only_sta_w_corr;
	locator_params->use_only_sta_w_corr = FALSE;

	/* 
	 *Initialization
	 */

	atry = dvect(1, locator_info->num_params);
	beta = dvect(1, locator_info->num_params);
	da = dvect(1, locator_info->num_params);
	oneda = dmatr(1, locator_info->num_params, 1, 1);
	covar= dmatr(1, locator_info->num_params, 1, locator_info->num_params);
	alpha= dmatr(1, locator_info->num_params, 1, locator_info->num_params);
	a = dvect(1, locator_info->num_params);

	best.wt_rms = 999999.9;
	best.nd = 1;
	best.torg = init[0];
	best.lon = init[1];
	best.lat = init[2];
	best.depth = init[3];
	best.unwt_rms = -1.0;
	best.sdobs = -1.0;

	for(n=0; n < num_test_depth; n++)	/* initionalization */
	{
		depths[n].wt_rms = 999999.9;
		depths[n].nd = 1;
		depths[n].torg = init[0];
		depths[n].lon = init[1];
		depths[n].lat = init[2];
	}
	
	/* check initial depth */
	
	if(init[3] < 0.0)
	    init[3] = 0.0;
	else
	    if(init[3] > MAX_DEPTH)
		init[3] = MAX_DEPTH; 
	depth[1] = init[3];

	if( locator_params->fix_depth || ignore_large_res) /* we do not need depth search */
	    num_depth = 2;
	
	/*
	 * MAIN LOOP FOR DEPTHS
	 */

	for(n=0; n < num_depth; n++)
	{
	    max_iter = locator_params->max_iterations;
	    res_wt = 999999.9;

	    if(n > 1 && n < (num_test_depth+2))	/* fix depth */
		depth_test = TRUE;
	    else
		depth_test = FALSE;
		
	    if(n == 2 && best.wt_rms < 15) /* start depth test */
	    {
		init[0] = best.torg;
		init[1] = best.lon;
		init[2] = best.lat;
	    }
	     	     
	    if(n >= 3 && n <= (num_test_depth + 2))	/* find best test depth*/ 
	    {
		for(l=1, best_ind=0; l < n-2; l++)
		    if( (depths[best_ind].wt_rms > depths[l].wt_rms && depths[best_ind].nd <= depths[l].nd)
		      || (depths[best_ind].nd < depths[l].nd && depths[best_ind].wt_rms <= depths[l].wt_rms))
			best_ind = l;	/* best idex */

		if(n <= 5)
		{
		    init[0] = depths[best_ind].torg;
		    init[1] = depths[best_ind].lon;
		    init[2] = depths[best_ind].lat;
		}

		if(n == 5) /* test for 0, 380, 700 finished */
		{
		    if(depths[best_ind].depth == 0.0)
		    {
		        depth[5] = 33;
		        depth[6] = 75;
		        depth[7] = 170;
		    }
		    if(depths[best_ind].depth == 380.0)
		    {
		        depth[5] = 140;
		        depth[6] = 250;
		        depth[7] = 520;
		    }
		    if(depths[best_ind].depth == MAX_DEPTH)
		    {
		        depth[5] = 650;
		        depth[6] = 550;
		        depth[7] = 450;
		    }
		}

		/* try to skip last depth test*/
		
		if(n == 7 && depths[2].wt_rms < depths[3].wt_rms && depths[2].nd >= depths[3].nd
		    && depths[2].wt_rms < depths[4].wt_rms && depths[2].nd >= depths[4].nd 
		    && depths[2].wt_rms < depths[5].wt_rms && depths[2].nd >= depths[5].nd 
		    && depths[2].wt_rms < depths[6].wt_rms && depths[2].nd >= depths[6].nd )
		        continue;

		if(n == 7 && depths[3].wt_rms < depths[2].wt_rms && depths[3].nd >= depths[2].nd
		    && depths[3].wt_rms < depths[4].wt_rms && depths[3].nd >= depths[4].nd 
		    && depths[5].wt_rms < depths[6].wt_rms && depths[5].nd >= depths[6].nd 
		    && depths[6].wt_rms < depths[3].wt_rms && depths[6].nd >= depths[3].nd )
		        continue;

		if(n == 7 && depths[4].wt_rms < depths[3].wt_rms && depths[4].nd >= depths[3].nd
		    && depths[3].wt_rms < depths[2].wt_rms && depths[3].nd >= depths[2].nd 
		    && depths[4].wt_rms < depths[5].wt_rms && depths[4].nd >= depths[5].nd 
		    && depths[5].wt_rms < depths[6].wt_rms && depths[5].nd >= depths[6].nd )
		        continue;

		if(n == 8 && best.depth != 0.0 && best.depth != MAX_DEPTH && best.wt_rms < depths[best_ind].wt_rms && best.nd >= depths[best_ind].nd)
		{
		    if(best.depth > 33.0)
		    {
			depth[8]= best.depth - 10.0;
			depth[9]= best.depth + 10.0;
		    }	    
		    else
			depth[8]= 12.0;
		}		    		    
	    }
	    
	    if(n == (num_test_depth + 2))	/* depth tests have just finished */
	    {
		init[0] = depths[best_ind].torg;
		init[1] = depths[best_ind].lon;
		init[2] = depths[best_ind].lat;
		depth[num_test_depth+2] = depths[best_ind].depth;
	    }
	    if(n >= 1)
	    {
		*torg = init[0];
		origin->lon = init[1];
		origin->lat = init[2];
		origin->depth = depth[n];
	    } 
	    else	/* first depth is SVD solution */
	    {
		*torg = orig_svd[0];
		origin->lon = orig_svd[1];
		origin->lat = orig_svd[2];
		origin->depth = orig_svd[3];
	    }
	
	    if(origin->depth < 0.0)
	        continue;

	    orig[0] = *torg;
	    orig[1] = origin->lon;
	    orig[2] = origin->lat;
	    orig[3] = origin->depth;
	    lm_par.iter = 0;

	    if( locator_params->fix_depth )
	    {
	        lm_par.fix_depth_this_iter = TRUE;
		lm_par.np = locator_info->num_params;
	    }
	    else
		if( (lm_par.iter < FIX_D && n >= (num_test_depth+2)) || depth_test )
		{
		    lm_par.np = locator_info->num_params - 1;
		    lm_par.fix_depth_this_iter = TRUE;
		}
		else
		{
		    lm_par.np = locator_info->num_params;
		    lm_par.fix_depth_this_iter = FALSE;
		}

	    if( mrqcof(&lm_par, alpha, beta) != OK)
		continue;

	    last_nd = lm_par.nd;
	    last_wt = wt_rms;

	    if( n == 0)		/* save SVD solution as the best */
	    {
		best.torg = *torg;
		best.lon = origin->lon;
		best.lat = origin->lat;
		best.depth = origin->depth;
		best.wt_rms = wt_rms;
		best.unwt_rms = unwt_rms;
		best.sdobs = origerr->sdobs;
		best.nd = last_nd;
	    }

	    i = 1;
	    if (! locator_params->fix_origin_time)
	    {
		a[i] = *torg;
		++i;
	    }
	    if (! locator_params->fix_lat_lon)
	    {
		a[i] = 0.0;
		++i;
		a[i] = 0.0;
		++i;
	    }
	    if (! locator_params->fix_depth)
		a[i] = origin->depth;


	    if(wt_rms > RMS_MAX / 2.0)
		alamda = 0.00001;
	    else
		alamda = 0.001;

	    alamda_last = alamda * 10;
	  
	    for (j=1; j<=lm_par.np; j++)
		atry[j] = a[j];

	    /*
	     * INNER lOCATION LOOP
	     */

	    for(iter=0; lm_par.iter <= max_iter ; iter++)
	    {
		/*
		 * Alter linearized fitting matrix, by augmenting diagonal elements
		 */

		for (j=1; j<=lm_par.np; j++)
		{
		    for (k=1; k<=lm_par.np; k++)
			covar[j][k] = alpha[j][k];
			covar[j][j] = alpha[j][j] * (1.0 + alamda);
			oneda[j][1] = beta[j];
		}
		
		/* Matrix solution */
		
		if(gaussj(covar, lm_par.np, oneda, 1) != OK)
		    break;
	    
		for (j=1; j<=lm_par.np; j++)
		    da[j] = oneda[j][1];

		for (l=1; l<=lm_par.np; l++)
		    atry[l] = a[l] + da[l];

		if( locator_params->fix_depth )
		{
		    lm_par.fix_depth_this_iter = TRUE;
		    lm_par.np = locator_info->num_params;
		}
		else
		    if( (lm_par.iter < FIX_D && n >= (num_test_depth+2)) || depth_test )
		    {
			lm_par.np = locator_info->num_params - 1;
			lm_par.fix_depth_this_iter = TRUE;
		    }
		    else
		    {
			lm_par.np = locator_info->num_params;
			lm_par.fix_depth_this_iter = FALSE;
		    }

		i = 1;
		if (! locator_params->fix_origin_time)
		{
		    *torg = atry[i];
		    ++i;
		}
		if (! locator_params->fix_lat_lon)
		{
		    if (atry[i] != 0.0 || atry[i+1] != 0.0)
		    {
			azi = RAD_TO_DEG*atan2(atry[i], atry[i+1]);
			dist  = atry[i]*atry[i] + atry[i+1]*atry[i+1];
			dist  = sqrt(dist);
			delta = RAD_TO_DEG*(dist/(RADIUS_EARTH - orig[3]));
			lat_lon(orig[2], orig[1], delta, azi, &origin->lat, &origin->lon);
		    }
		    else
		    {
			azi = 0.0;
			dist  = 0.0;	    
		    }
		    i += 2;
		}
		if (! lm_par.fix_depth_this_iter)
		{ 
		    origin->depth = orig[3] - da[i];
		    atry[i] = origin->depth;

		    if(origin->depth < 0.0)
			origin->depth = 0.0;
		    else
		        if(origin->depth > MAX_DEPTH)
			    origin->depth = MAX_DEPTH; 
		}

		ret = 0;	/* flag of exit from loop */

		/* to save some time */
		
		if(delta >= 0.000001 && (fabs(origin->depth - orig[3]) >= 0.0001 || lm_par.fix_depth_this_iter))
		{
		    if(mrqcof(&lm_par, alpha, beta) != OK)
		    {
			*torg = orig[0];
			origin->lon = orig[1];
			origin->lat = orig[2];
			origin->depth = orig[3];
			mrqcof(&lm_par, alpha, beta);
			goto UNS;
		    }
		}	

		/* check for convg */

		convg = CONVG_LM;
		
		for(i=1; i<=lm_par.np; i++)
		{
		    if(i == lm_par.np && ! locator_params->fix_depth)
			convg *= 10.0;
		    if(fabs(atry[i] - a[i]) <= convg)
			ret = 1;
		    if(ret == 0)
			break;	
		}

		/*
		 * Did the trial succeed?
		 */
	
		if ((wt_rms < last_wt && lm_par.nd >= last_nd) || 
		    (last_nd < lm_par.nd && (wt_rms / last_wt) <= (RMS_MULT * (lm_par.nd - last_nd))) ||
		    (last_wt > wt_rms && last_nd > lm_par.nd && last_wt >= RMS_MAX && (last_wt / wt_rms) >= (RMS_MULT * (last_nd - lm_par.nd))))
		{
		    res_wt = last_wt - wt_rms;
		    last_wt = wt_rms;
		    last_nd = lm_par.nd;

		    alamda *= 0.1;

		    for (j=1; j<=lm_par.np; j++)
		    {
			for (k=1; k<=lm_par.np; k++)
			    alpha[j][k] = covar[j][k];
			beta[j] = da[j];
		    }

		    l = 1;
		    if (! locator_params->fix_origin_time)
		    {
			a[l] = atry[l];
			++l;
		    }
		    if (! locator_params->fix_lat_lon)
		    {
			a[l] = 0.0;
			++l;
			a[l] = 0.0;
			++l;
		    }
		    if (! locator_params->fix_depth)
			a[l] = atry[l];

		    orig[0] = *torg;
		    orig[1] = origin->lon;
		    orig[2] = origin->lat;
		    orig[3] = origin->depth;

		    lm_par.iter++;

		    /* for depth search only */
		    if(depth_test && depths[best_ind].nd > lm_par.nd && lm_par.iter >= 3)
			break;
		    if(depth_test  && lm_par.iter >= 5 && wt_rms > (depths[best_ind].wt_rms * 2.5) && depths[best_ind].nd >= lm_par.nd)
			break;
		} 
		else 
		{
UNS:		    /* Failure, increase alamda and return */

		    alamda *= 10.0;

		    if(alamda > 1.0e+15) /* against dead loop */
			break;
		}

		/* second check */
		
		if(ret == 1 && n > 1 && n < (num_test_depth+2))	/* fix depth*/
		     break;

		if(((ret == 1) && (alamda_last < alamda)) || (res_wt < 0.0000001) || (lm_par.iter == max_iter))
		{
		    if( ! sec_test)
		    {
			*torg = orig[0];
			origin->lon = orig[1];
			origin->lat = orig[2];
			origin->depth = orig[3];
		  
			if( ignore_large_res )
			    locator_params->ignore_large_res = TRUE;
			else
			    locator_params->ignore_large_res = FALSE;

			if( use_only_sta_w_corr )
			    locator_params->use_only_sta_w_corr = TRUE;
			else
			    locator_params->use_only_sta_w_corr = FALSE;

			lm_par.iter = MIN_ITER + 1;

			if( ignore_large_res || use_only_sta_w_corr)
			{
			    if(mrqcof(&lm_par, alpha, beta) != OK)
			        break;
				alamda = 0.001;
				last_wt = wt_rms;
				last_nd = lm_par.nd;
			}
			else
			{
			    if(wt_rms <= RMS_MAX && wt_rms <= ( best.wt_rms * 1.5) && lm_par.nd >= best.nd)
				alamda = 0.0001;
			    else

			        break;
			}
			sec_test = TRUE;
		    }
		    else
			if(lm_par.iter > MIN_ITER + 2)
			    break;	    
		}

		alamda_last = alamda;

	    }	/*  End of locate loop */

	    *torg = orig[0];
	    origin->lon = orig[1];
	    origin->lat = orig[2];
	    origin->depth = orig[3];

	    sec_test = FALSE;
	
	    if(!depth_test)	/* no fix depth*/
	    {
		if( ignore_large_res )
		    locator_params->ignore_large_res = TRUE;
		else
		    locator_params->ignore_large_res = FALSE;

		if( use_only_sta_w_corr )
		    locator_params->use_only_sta_w_corr = TRUE;
		else
		    locator_params->use_only_sta_w_corr = FALSE;
	    }
	    if(get_resids_and_derivs(&lm_par, TRUE) != OK)
		best.nd = 1;

	    locator_params->ignore_large_res = FALSE;
	    locator_params->use_only_sta_w_corr = FALSE;

	    if(depth_test)	/* fix depth*/
	    {
		depths[n-2].torg = *torg;
		depths[n-2].lon = origin->lon;
		depths[n-2].lat = origin->lat;
		depths[n-2].depth = origin->depth;
		depths[n-2].wt_rms = wt_rms;
		depths[n-2].nd = lm_par.nd;
		continue;
	    }

	    if( (n == 0) || (best.wt_rms > wt_rms && best.nd <= lm_par.nd) || 
		(best.wt_rms <= wt_rms && best.nd < lm_par.nd && (wt_rms / best.wt_rms) <= (RMS_MULT * (lm_par.nd - best.nd))) ||
		(best.wt_rms > wt_rms && best.nd > lm_par.nd && best.wt_rms >= RMS_MAX && (best.wt_rms / wt_rms) >= (RMS_MULT * (best.nd - lm_par.nd))) )
	    {
		best.torg = *torg;
		best.lon = origin->lon;
		best.lat = origin->lat;
		best.depth = origin->depth;
		best.wt_rms = wt_rms;
		best.unwt_rms = unwt_rms;
		best.sdobs = origerr->sdobs;
		best.nd = lm_par.nd;
	    }
	}	/* End of main loop */

	/* restore params */
	locator_params->ignore_large_res = ignore_large_res;
	locator_params->use_only_sta_w_corr = use_only_sta_w_corr;

	free_dmatr(oneda,1,locator_info->num_params,1,1);
	free_dvect(da,1,locator_info->num_params);
	free_dvect(beta,1,locator_info->num_params);
	free_dvect(atry,1,locator_info->num_params);
	free_dmatr(covar,1,locator_info->num_params,1,1);
	free_dmatr(alpha,1,locator_info->num_params,1,1);
	free_dvect(a,1,locator_info->num_params);

	if( (best.wt_rms > svd_rms && best.nd <= nd_svd) || 
	    (best.wt_rms <= svd_rms && best.nd < nd_svd && (svd_rms / best.wt_rms) <= (RMS_MULT * (nd_svd - best.nd))) ||
	    (best.wt_rms > svd_rms && best.nd > nd_svd && best.wt_rms >= RMS_MAX && (best.wt_rms / svd_rms) >= (RMS_MULT * (best.nd - nd_svd))) )
	{
	    *torg = orig_svd[0];
	    origin->lon = orig_svd[1];
	    origin->lat = orig_svd[2];
	    origin->depth = orig_svd[3];
	    lm_par.iter = -1;
	}
	else
	{
	    *torg = best.torg;
	    origin->lon = best.lon;
	    origin->lat = best.lat;
	    origin->depth = best.depth;

	    if( origin->depth == MAX_DEPTH || origin->depth == 0.0)
	    {
		if( !locator_params->fix_depth)
		{
		    lm_par.np = locator_info->num_params - 1;
	    	    lm_par.fix_depth_this_iter = TRUE;
		}
		else
		{
		    lm_par.np = locator_info->num_params;
	    	    lm_par.fix_depth_this_iter = TRUE;
		}
	    }
	    else
	    {
		if( !locator_params->fix_depth)
		{
		    lm_par.np = locator_info->num_params;
	    	    lm_par.fix_depth_this_iter = FALSE;
		}
		else
		{
		    lm_par.np = locator_info->num_params;
	    	    lm_par.fix_depth_this_iter = TRUE;
		}
	    }

	    if(get_resids_and_derivs(&lm_par, TRUE) != OK)
	    {
	    	*torg = orig_svd[0];
		origin->lon = orig_svd[1];
	  	origin->lat = orig_svd[2];
	    	origin->depth = orig_svd[3];
	    	lm_par.iter = -1;
	    }
	    else	
	        lm_par.iter = 0;
	}

	if ((verbose == 3 || verbose == 4) && lm_par.iter >= 0)
	{
	    fprintf (ofp, "\n----------------------------------------------------------------------------\n\n");
	    fprintf (ofp, "Number of Obs. (Data) -  SVD: %d   LM: %d\n\n", nd_svd, lm_par.nd);
	    fprintf (ofp, "SVD - Lat:%8.3f   Lon:%9.3f   Depth:%9.3f   To:%10.3f\n", orig_svd[2], orig_svd[1], orig_svd[3], orig_svd[0]);
	    fprintf (ofp, "LM  - Lat:%8.3f   Lon:%9.3f   Depth:%9.3f   To:%10.3f\n\n", best.lat, best.lon, best.depth, best.torg);
	    fprintf (ofp, "SVD - Unwt. RMS Res.:%8.4f   Wt. RMS Res.:%8.4f   RMS TT Res.:%8.4f\n", svd_unwt_rms, svd_rms, svd_sdobs);
	    fprintf (ofp, "LM  - Unwt. RMS Res.:%8.4f   Wt. RMS Res.:%8.4f   RMS TT Res.:%8.4f\n\n", best.unwt_rms, best.wt_rms, best.sdobs);
	}


	/* RETURN:  1 - LM converged; 2 - LM diverged; 0 - use SVD solution */

	if(lm_par.iter >= 0)
	    if(wt_rms > RMS_MAX)
		if(svd_conv)	/* in real it is diverg */
		    ret = 2;
		else
		    ret = 1;  /* if improvement and SVD converged , we too*/		
	    else
	        ret = 1;
	else
	    ret = 0;

	return ret;
}


int
get_resids_and_derivs (lm_params *params, Bool deriv)
{
	Bool	use_azimuth, use_slow, use_time;
	Bool	do_we_need_z_derivs;
	int	i, ip, iterr, m, n, nd_used, np, num_tt_used;
	double	esr, err;
	double	delta;
	double	residual, rt, total_tt;
	double	unwt_rms, unwt_tt_sqd_sum, wt_rms;
	double	blk_radius = DEG_TO_KM;
	double	*At, *dsd2, *resid2;
	double	*az_used_in_loc;
	double	az_deriv[MAX_PARAM];
	double	prin_deriv[MAX_PARAM], slow_deriv[MAX_PARAM];
	double	tt_deriv[MAX_PARAM];
	double	dist_min, dist_max, az_min, az_max, d;

	Origin	*origin;
	Origerr	*origerr;
	Assoc	*assoc;
	Ar_Info	*ar_info;
	Locator_params	*locator_params;
	Locator_info	*locator_info;
	Site	*sites;
	Arrival	*arrival;
	double	*torg;
	
	/* Initializations */

	Bool	convergence = FALSE;
	Bool	divergence  = FALSE;
	Bool	fix_origin_time = FALSE;
	int	ierr	 = 0;
	int	num_iter;
	int	num_obs;
        Assoc   Na_Assoc = Na_Assoc_Init;
        Origerr Na_Origerr = Na_Origerr_Init;

	At		= params->At;
	resid2		= params->resid2;
	dsd2		= params->dsd2;
	az_used_in_loc	= params->az_used_in_loc;
	num_obs		= params->num_obs;
	assoc		= params->assoc;
	locator_params	= params->locator_params;
	locator_info	= params->locator_info;
	sites		= params->sites;
	arrival		= params->arrival;
	origin		= params->origin;
	origerr		= params->origerr;
	ar_info		= params->ar_info;
	torg		= params->torg;
	num_iter	= params->iter;
	np		= params->np;

	for (n = 0; n < num_obs; n++)
	{
	    assoc[n].timeres = Na_Assoc.timeres;
	    assoc[n].azres   = Na_Assoc.azres;
	    assoc[n].slores  = Na_Assoc.slores;
	    assoc[n].wgt     = Na_Assoc.wgt;
	    strcpy (assoc[n].vmodel, "-");
	}
	for (n = 0; n < 3*num_obs; n++)
	{
	    resid2[n] = 0.0;
	    dsd2[n] = 1.0;
	    for (i = 0; i < locator_info->num_params; i++)
		At[i + n*MAX_PARAM] = 0.0;
	}

	/*
	 * If fixing depth on this iteration, then we need not compute depth
	 * derivatives (i.e., do_we_need_z_derivs = FALSE).
	 */

	if (params->fix_depth_this_iter || !deriv)
	    do_we_need_z_derivs = FALSE;
	else
	    do_we_need_z_derivs = TRUE;

	fix_origin_time = FALSE;
	if (locator_params->fix_origin_time)
	    fix_origin_time = TRUE;

	/*
	 * Compute distances and azimuths to stations with data (forward 
	 * problem for azimuths)
	 */

	for (m = 0, n = 0; n < num_obs; n++)
	{
	    if ((i = locator_info->sta_index[n]) < 0)
	    {
		assoc[n].delta = Na_Assoc.delta;
		assoc[n].seaz  = Na_Assoc.seaz;
		assoc[n].esaz  = Na_Assoc.esaz;
	    }
	    else
	    {
		dist_azimuth (sites[i].lat, sites[i].lon, origin->lat, 
			      origin->lon, &assoc[n].delta, &assoc[n].seaz,
			      &assoc[n].esaz, m);
		++m;
	    }
	}

	/* 
	 * Compute travel-times, slownesses and azimuths based on current 
	 * location hypothesis and determine partial derivatives.  Ignore 
	 * points with completely invalid data (i.e., data_error_code = 1,
	 * 2, 3, 4).
	 */

	unwt_tt_sqd_sum = 0.0;

	for (n = 0, nd_used = 0, num_tt_used = 0; n < num_obs; n++)
	{
	    use_time    = TRUE;
	    use_slow    = TRUE;
	    use_azimuth = TRUE;
	    if(ar_info[n].time_error_code > 0 && ar_info[n].time_error_code < 5)
		use_time = FALSE;
	    if(ar_info[n].slow_error_code > 0 && ar_info[n].slow_error_code < 5)
		use_slow = FALSE;
	    if(ar_info[n].az_error_code > 0 && ar_info[n].az_error_code < 5)
		use_azimuth = FALSE;
	    if(!convergence && !divergence && strncmp (assoc[n].timedef, "d", 1))
		use_time = FALSE;
	    if(!convergence && !divergence && strncmp (assoc[n].slodef, "d", 1))
		use_slow = FALSE;
	    if(!convergence && !divergence && strncmp (assoc[n].azdef, "d", 1))
		use_azimuth = FALSE;

	    if (use_time || use_slow)
	    {
		/*
		 * If this is a hydroacoustic phase, check for blockage and
		 * possibility that delta > 180 deg.  If the latter case
		 * is TRUE, then adjust delta, esaz and seaz variables of
		 * assoc table entry.
		 */

		if (STREQ (assoc[n].phase, "H") || STREQ (assoc[n].phase, "T")
		    || STREQ (assoc[n].phase, "HT") 
		    || STREQ (assoc[n].phase, "HO"))
		{
		    if (origerr->smajax > 0.0 && origerr->sminax > 0.0)
		    {
			blocked_ellipse_dist (assoc[n].sta, origin->lat, 
					      origin->lon, origerr->smajax, 
					      origerr->sminax, origerr->strike, 
					      &delta, &dist_min, &dist_max, 
					      &az_min, &az_max);
		    }
		    else
		    {
			blk_radius = get_blk_radius ();
			blocked_ellipse_dist (assoc[n].sta, origin->lat, 
					      origin->lon, blk_radius, 
					      blk_radius, 0.0, &delta, 
					      &dist_min, &dist_max, &az_min, 
					      &az_max);
		    }
		    
		    if (delta > 180.0 || (dist_min > 180.0 && dist_max > 180.0))
		    {
			assoc[n].delta = 360.0 - assoc[n].delta;
			assoc[n].seaz += 180.0;
			assoc[n].esaz += 180.0;
			if (assoc[n].seaz > 360.0)
			    assoc[n].seaz -= 360.0;
			if (assoc[n].esaz > 360.0)
			    assoc[n].esaz -= 360.0;
		    }
		}

		/*
		 * Determine travel time, azimuth and slowness along 
		 * with derivatives and any necessary corrections.
		 */

		total_tt = 
		total_travel_time (locator_params, sites, &ar_info[n], FALSE, 
				   do_we_need_z_derivs, origin->lat, 
				   origin->lon, origin->depth, assoc[n].delta, 
				   assoc[n].esaz, assoc[n].phase, 
				   locator_info->sta_index[n], 
				   locator_info->phase_index[n], 
				   locator_info->spm_index[n], 
				   arrival[n].deltim, prin_deriv,
				   tt_deriv, slow_deriv, az_deriv, &iterr);

		ar_info[n].orid = origin->orid;
		ar_info[n].arid = assoc[n].arid;
		if (total_tt > 0.0)
		{
		    /*
	 	     * Compute travel-time residual:
		     *
		     * data    = t    - t	  = obs_data - total_tt
		     *     res    obs    calc
		     *
		     * where, t     = t      + t
		     *         calc    table    corr
		     *
		     * and, t	is the sum of all travel-time
		     *       corr	corrections (i.e., SSSC or SRST
		     *		or test-site, and elevation and
		     *		ellipticity) as determined in
		     *		function, total_travel_time().
		     */

		    assoc[n].timeres = arrival[n].time - total_tt - *torg;

		    /*
		     * Compute slowness residual = (obs - calc) datum
		     */

		    assoc[n].slores = arrival[n].slow - prin_deriv[0];

		    strcpy (assoc[n].vmodel, ar_info[n].vmodel);
		}
		else
		{
		    assoc[n].timeres = Na_Assoc.timeres;
		    assoc[n].slores = Na_Assoc.slores;
		    strcpy (assoc[n].vmodel, "-");
		}

		assoc[n].wgt = 1.0/ar_info[n].model_plus_meas_error;

 		if (total_tt < 0.0)
                {
                        ar_info[n].time_error_code = 8;
                        ar_info[n].slow_error_code = 8;
			ar_info[n].az_error_code = 8;
			use_time = FALSE;
			use_slow = FALSE;
			use_azimuth = FALSE;
                }
		else
		{
		    if (use_time)
			ar_info[n].time_error_code = iterr;
		    if (use_slow)
			ar_info[n].slow_error_code = iterr;

		    ar_info[n].az_error_code = iterr;	/* added az */
		}

		/*
		 * If requested, use only travel-time data with SSSC, SRST, 
		 * or test-site corrections available.  Signify this by 
		 * setting data error code to 6 when correction not found.
		 * Make sure at least 2 iterations have already been performed.
		 */

		if (use_time && locator_params->use_only_sta_w_corr && 
		    num_iter > MIN_ITER-2 && 
		    ar_info[n].src_dpnt_corr_type == NO_SRC_DPNT_CORR)
		    ar_info[n].time_error_code = 6;

		if (use_slow && locator_params->use_only_sta_w_corr && 
		    num_iter > MIN_ITER-2)
		    ar_info[n].slow_error_code = 6;

		/*
		 * If user sets locator_params->ignore_large_res = TRUE,
		 * then, do not include any data where data residual,
		 * data_residual[], is > pre-defined muliplier factor, 
		 * locator_params->large_res_mult, times the a priori
		 * data standard error, data_std_err[].  Do not apply
		 * this screening if depth is being freed for the first
		 * time.
		 */

		if (locator_params->ignore_large_res && 
		    ((params->fix_depth_this_iter && num_iter > MIN_ITER-2) || 
		     (!params->fix_depth_this_iter && num_iter >= MIN_ITER)))
		{
		    if (use_time && fabs(assoc[n].timeres) > 
			    locator_params->large_res_mult *
					ar_info[n].model_plus_meas_error)
			ar_info[n].time_error_code = 5;
		    if (use_slow && fabs(assoc[n].slores) > 
			    locator_params->large_res_mult*arrival[n].delslo)
			ar_info[n].slow_error_code = 5;
		}

		/*
		 * Store travel-time derivatives in A-transpose (At)
		 * Note that parameters in At are ordered as: origin-time; 
		 * longitude; latitude; depth.  Also normalize matrix and 
		 * residuals w.r.t. data variances.
		 */

		if (use_time && ar_info[n].time_error_code < 1 &&
		    STREQ (assoc[n].timedef, "d"))
		{
		  if( deriv )
		  {
		    ip = 0;
		    if (! fix_origin_time)
		    {
			At[nd_used*MAX_PARAM] = tt_deriv[0];
			++ip;
		    }
		    if (! locator_params->fix_lat_lon)
		    {

			At[ip + nd_used*MAX_PARAM] = tt_deriv[1];
			++ip;
			At[ip + nd_used*MAX_PARAM] = tt_deriv[2];
			++ip;
		    }
		    if (! params->fix_depth_this_iter)
		    	At[ip + nd_used*MAX_PARAM] = tt_deriv[3];
		  }
		    residual = assoc[n].timeres;

		    resid2[nd_used] = residual;
		    dsd2[nd_used] = ar_info[n].model_plus_meas_error;
		    az_used_in_loc[num_tt_used] = assoc[n].esaz;
		    unwt_tt_sqd_sum += residual*residual;

		    ++num_tt_used;
		    ++nd_used;	/* Actual # data to be used */
		}

		/*
		 * Store slowness derivatives in A-transpose (At)
		 */

		if (use_slow && ar_info[n].slow_error_code < 1 &&
			    STREQ (assoc[n].slodef, "d"))
		{
		  if( deriv )
		  {
		    ip = 0;
		    if (! fix_origin_time)
		    {
			At[nd_used*MAX_PARAM] = slow_deriv[0];
			++ip;
		    }
		    if (! locator_params->fix_lat_lon)
		    {
			At[ip + nd_used*MAX_PARAM] = slow_deriv[1];
			++ip;
			At[ip + nd_used*MAX_PARAM] = slow_deriv[2];
			++ip;
		    }
		    if (! params->fix_depth_this_iter)
		    {
			d = origin->depth;
			if( d < SL_DEPTH)
	   		  d = SL_DEPTH;
			if( d > (MAX_DEPTH - SL_DEPTH))
	   		  d = MAX_DEPTH - SL_DEPTH;
			At[ip + nd_used*MAX_PARAM] = dfridr(d, SL_DEPTH, &err, n, params);
		    }
		  }
		    resid2[nd_used] = assoc[n].slores;
		    dsd2[nd_used] = arrival[n].delslo;

		    ++nd_used;	/* Actual # data to be used */
		}
	    }

	    /*
	     * Deal with azimuth seperately in case only azimuth data is
	     * needed, then total_travel_time() need not be call at all.
	     * If total_travel_time() has been called already, then you
	     * have the azimuth derivatives already.
	     */

	    if (use_azimuth)
	    {
		/* Apply AZ SSSC adjustments, if necessary */

		/* Copied from compute_hypo() !!!
		 * Commented out until I (WCN) can resolve how to
		 * handle re-populating assoc[n].seaz & assoc[n].esaz.
		 * It really doesn't make sense to re-set this from
		 * the database description.  On the other hand, if
		 * one doesn't reset these variables than SSSC really
		 * can't be applied.
		 *

		if (num_iter > MIN_ITER-2 && locator_params->sssc_level > 0)
		{
		    azi = assoc[n].seaz;
		    baz = assoc[n].esaz;
		    i = locator_info->sta_index[n];
		    k = locator_info->phz_index[n];
		    sssc_found = apply_sssc (sssc, origin->lat, origin->lon, 
					     &correct, 1, i, k);
		    azi = azi + correct;
		    baz = baz + correct;
		    if (azi < 0.0)
			azi += 360.0;
		    if (baz < 0.0)
			baz += 360.0;
		    if (azi > 360.0)
			azi -= 360.0;
		    if (baz > 360.0)
			baz -= 360.0;
		    assoc[n].seaz = azi;
		    assoc[n].esaz = baz;

		     *
		     * Use only azimuth data with SSSC's?
		     * 

		    if (locator_params->use_only_sta_w_corr 
			&& num_iter > MIN_ITER-2 && !sssc_found)
			data_error_code[n] = 6;
		}
			 */

		if (! use_time && ! use_slow)
		{
			/*
			 * Special case: Only azimuth data available
			 */

			esr = assoc[n].esaz*DEG_TO_RAD;
			rt  = sin(assoc[n].delta*DEG_TO_RAD)*
			      RADIUS_EARTH*DEG_TO_RAD;
			if (rt == 0.0)
			    rt = 0.0001;
			az_deriv[0] = 0.0;
			az_deriv[1] = -cos(esr)/rt;
			az_deriv[2] = sin(esr)/rt;
			az_deriv[3] = 0.0;
		}

		/*
	 	 * Compute azimuth residual = 
		 *	(obs - calc) datum	[Note that azimuthal
		 *	SSSC has already been removed]
		 */

		assoc[n].azres = arrival[n].azimuth - assoc[n].seaz;
		if (fabs(assoc[n].azres) > 180.0)
		    assoc[n].azres = SIGN( (360.0-fabs(assoc[n].azres)),
						  assoc[n].azres );

		/*
		 * If user sets locator_params->use_only_sta_w_corr = TRUE
		 * then, do not include any azimuth data.
		 */

		if (locator_params->use_only_sta_w_corr && num_iter > MIN_ITER-2) 
		    ar_info[n].az_error_code = 6;
		/*
		 * If user sets locator_params->ignore_large_res = TRUE,
		 * then, do not include any azimuth data where its 
		 * residual, is > pre-defined muliplier factor, 
		 * locator_params->large_res_mult, times delaz.
		 */

		if (num_iter > MIN_ITER-2 && locator_params->ignore_large_res)
		{
		    if (fabs(assoc[n].azres) > 
		            locator_params->large_res_mult* arrival[n].delaz)
		    ar_info[n].az_error_code = 5;
		}

		/*
		 * Store azimuth derivatives in A-transpose (At)
		 */

		if (ar_info[n].az_error_code < 1 && STREQ (assoc[n].azdef, "d"))
		{
		  if( deriv )
		  {
		    ip = 0;
		    if (! fix_origin_time)
		    {
			At[nd_used*MAX_PARAM] = az_deriv[0];
			++ip;
		    }
		    if (! locator_params->fix_lat_lon)
		    {
			At[ip + nd_used*MAX_PARAM] = az_deriv[1];
			++ip;
			At[ip + nd_used*MAX_PARAM] = az_deriv[2];
			++ip;
		    }
		    if (! params->fix_depth_this_iter)
			At[ip + nd_used*MAX_PARAM] = az_deriv[3];
		  }

		    resid2[nd_used] = assoc[n].azres;
		    dsd2[nd_used] = arrival[n].delaz;

		    ++nd_used;	/* Actual # data to be used */
		}
	    }
	}

	/*
	 * Check for insufficient data.
	*/
	if (nd_used < np)
	    ierr = 1;

	/*
	 * Compute weighted and unweighted RMS 
	 * residual (dimensionless quantities).  
	 */

	unwt_rms = 0.0;
	wt_rms = 0.0;

	for (n = 0; n < nd_used; n++)
	{
	    unwt_rms += resid2[n]*resid2[n];
	    residual = resid2[n] / dsd2[n];
	    wt_rms += residual*residual;
	}

	if (nd_used > 0)
	{
	    unwt_rms = unwt_rms/nd_used;
	    wt_rms = wt_rms/nd_used;
	    unwt_rms = sqrt(unwt_rms);
	    wt_rms   = sqrt(wt_rms);
	}

	unwt_tt_sqd_sum = unwt_tt_sqd_sum/num_tt_used;
	if (num_tt_used > 0)
	    origerr->sdobs = sqrt(unwt_tt_sqd_sum);
	else
	    origerr->sdobs = Na_Origerr.sdobs;

	locator_info->nd_used	 = nd_used;

	*params->wt_rms = wt_rms;
	*params->unwt_rms = unwt_rms;
	params->nd = nd_used;

	return (ierr);
}


double
dfridr(double x, double h, double *err, int n, lm_params *p)
{
	int 	i, j;
	double 	errt, fac, hh, **a, ans=0.0, f1, f2;
	double  con = 1.4;	/* Stepsize is decreased by con at each iteration. */
	int	ntab = 10; 	/* Sets maximum size of tableau. */
	double	safe = 2.0; 	/* Return when error is safe worse than the best so far. */
	double	big = 1.0e30;

	if(h == 0.0) 
	    return 0.0;

	a = dmatr(1, ntab, 1, ntab);
	hh = h;
MET_DR:	
	f1 = get_slow(x + hh, n, p);
	f2 = get_slow(x - hh, n, p);
	if((f1 < 0.0) || (f2 < 0.0))
	{
	    hh *= 0.75;
	    if(hh < 0.0000000001)
	        return 0.0;
	    goto MET_DR;
	}

	a[1][1] = (f1 - f2) / (2.0 * hh);
	*err = big;

	/* 
	 * Successive columns in the Neville tableau will go to smaller stepsizes and higher orders of extrapolation
	 */
	
	for (i=2; i <= ntab; i++)
	{
	    hh /= con;
	    a[1][i] = (get_slow(x + hh, n, p) - get_slow(x - hh, n, p)) / (2.0 * hh);	/* Try new, smaller stepsize */
	    fac=con * con;

	    /* 
	     * Compute extrapolations of various orders, requiring no new function evaluations
	     */

	    for (j=2; j<=i; j++) 
	    { 
		a[j][i] = (a[j-1][i] * fac - a[j-1][i-1]) / (fac - 1.0);
		fac = con * con * fac;
		errt = DMAX(fabs(a[j][i] - a[j-1][i]), fabs(a[j][i] - a[j-1][i-1]));
		
		/*
		 * The error strategy is to compare each new extrapolation to one order lower, both
		 * at the present stepsize and the previous one
		 */
		  
		if (errt <= *err)	/* If error is decreased, save the improved answer */
		{
		    *err = errt;
		    ans = a[j][i];
		}
	    }

	    /*
	     * If higher order is worse by a significant factor safe, then quit early
	     */

	    if (fabs(a[i][i] - a[i-1][i-1]) >= safe * (*err))
	       break;
	}
	free_dmatr(a, 1, ntab, 1, ntab);

	return ans;
}


double
get_slow(double depth, int n, lm_params *p)
{
	int	iterr;
	double	delta, seaz, esaz, total_tt;
	double	prin_deriv[MAX_PARAM], slow_deriv[MAX_PARAM];
	double	tt_deriv[MAX_PARAM], az_deriv[MAX_PARAM];

	dist_azimuth (p->sites[p->locator_info->sta_index[n]].lat, p->sites[p->locator_info->sta_index[n]].lon,
			p->origin->lat, p->origin->lon, &delta, &seaz, &esaz, 0);
	total_tt = 
	total_travel_time (p->locator_params, p->sites, &p->ar_info[n], TRUE, 
			FALSE, p->origin->lat, p->origin->lon, depth, delta, 
			esaz, p->assoc[n].phase, p->locator_info->sta_index[n], 
			p->locator_info->phase_index[n], p->locator_info->spm_index[n], 
			p->arrival[n].deltim, prin_deriv, tt_deriv, slow_deriv, az_deriv, &iterr);
			
	if(total_tt < 0.0)
	    return -1.0;
	else
	    return (prin_deriv[0] / DEG_TO_KM);
}


int
mrqcof(lm_params *pars, double **alpha, double *beta)
{
	int	i, j, k, l, m, np, nd, ierr;
	double	wt, sig2i, dy, *dyda;

	np = pars->np;
	dyda=dvect(1, np);

 	/*
	 * Initialize (symmetric) alpha, beta
	 */
	for (j=1; j<=np; j++)
	{
	    for (k=1; k<=j; k++)
		alpha[j][k] = 0.0;
	    beta[j] = 0.0;
	}
	
	ierr = get_resids_and_derivs(pars, TRUE);
	if( ierr != OK )
	    return (ierr);

	nd = pars->nd;

	/*
	 * Summation loop over all data
	 */
	for (i=0; i < nd; i++)
	{
	    sig2i = 1.0 / (pars->dsd2[i] * pars->dsd2[i]);
	    dy = pars->resid2[i];
		
	    for (j=0, l=1; l<=np; l++)
	    {
		for (m = 0; m < np; m++)
		    dyda[m+1] = pars->At[m + i*MAX_PARAM];

		wt = dyda[l] * sig2i;
		for (j++,k=0,m=1; m<=l; m++)
		    alpha[j][++k] += wt * dyda[m];
		beta[j] += dy * wt;
	    }
	}
	
	/*
	 * Fill in the symmetric side
	 */
	for (j=2; j<=np; j++)
	    for (k=1; k<j; k++)
		alpha[k][j] = alpha[j][k];

	free_dvect(dyda, 1, np);
	return OK;
}


int
gaussj(double **a, int n, double **b, int m)
{
	int	*indxc, *indxr, *ipiv;
	int	i, icol=0, irow=0, j, k, l, ll;
	double	big, dum, pivinv, temp;
	
	/*
	 * The integer arrays ipiv, indxr, and indxc are used for bookkeeping on the pivoting
	 */
	 
	indxc = ivect(1, n); 	
	indxr = ivect(1, n);
	ipiv = ivect(1, n);
	for(j=1; j<=n; j++)
	    ipiv[j]=0;
	    
 	/*
	 * This is the main loop over the columns to be reduced
	 */
	for(i=1; i<=n; i++)
	{
	    big=0.0;
	    
	    /*
	     * This is the outer loop of the search for a pivot element
	     */
	    for (j=1; j<=n; j++)
		if (ipiv[j] != 1)
		for (k=1; k<=n; k++)
		{
		    if (ipiv[k] == 0)
		    {
			if (fabs(a[j][k]) >= big)
			{
			    big = fabs(a[j][k]);
			    irow = j;
			    icol = k;
			}
		    }
		}
		++(ipiv[icol]);
	/*
	 * We now have the pivot element, so we interchange rows, if needed, to put the pivot
	 * element on the diagonal. The columns are not physically interchanged, only relabeled:
	 * indxc[i], the column of the ith pivot element, is the ith column that is reduced, while
	 * indxr[i] is the row in which that pivot element was originally located. If indxr[i] =
	 * indxc[i] there is an implied column interchange. With this form of bookkeeping, the
	 * solution b’s will end up in the correct order, and the inverse matrix will be scrambled
	 * by columns
	 */
	 
	if (irow != icol)
	{
	    for (l=1; l<=n; l++)
		SWAP(a[irow][l], a[icol][l])
	    for (l=1; l<=m; l++)
		SWAP(b[irow][l], b[icol][l])
	}
	indxr[i] = irow; 		/* We are now ready to divide the pivot row by the
				   	   pivot element, located at irow and icol */
	indxc[i] = icol;
	if (a[icol][icol] == 0.0)
	    return -1;

	pivinv = 1.0 / a[icol][icol];
	a[icol][icol] = 1.0;
	for (l=1; l<=n; l++)
	    a[icol][l] *= pivinv;
	for (l=1; l<=m; l++)
	    b[icol][l] *= pivinv;
	for (ll=1; ll<=n; ll++) 		/* Next, we reduce the rows... */
	    if (ll != icol)			/* ...except for the pivot one, of course */
	    {
		dum = a[ll][icol];
		a[ll][icol] = 0.0;
		for (l=1;l<=n;l++)
		    a[ll][l] -= a[icol][l] * dum;
		for (l=1;l<=m;l++)
		    b[ll][l] -= b[icol][l] * dum;
	    }
	}

	/*
	 * This is the end of the main loop over columns of the reduction. It only remains to unscramble
	 * the solution in view of the column interchanges. We do this by interchanging pairs of
	 * columns in the reverse order that the permutation was built up
	 */
	for (l=n; l>=1; l--)
	{
	    if (indxr[l] != indxc[l])
		for (k=1; k<=n; k++)
		    SWAP(a[k][indxr[l]], a[k][indxc[l]]);
	}
	
	free_ivect(ipiv, 1, n);
	free_ivect(indxr, 1, n);
	free_ivect(indxc, 1, n);

	return OK;
}


int *
ivect(int nl, int nh)
{
	int	*v;
	
	v = (int *)malloc((unsigned) ((nh - nl + 2) * sizeof(int)));
	if (!v) 
	{
	    fprintf(stderr, "libloc: allocation failure in ivect()");
	    exit(1);
	}
	return (v - nl + 1);
}


double *
dvect(int nl, int nh)
{
	double	*v;
	
	v = (double *)malloc((unsigned) ((nh - nl + 2) * sizeof(double)));
	if (!v)
	{
	    fprintf(stderr, "libloc: allocation failure in dvect()");
	    exit(1);
	}
	return (v - nl + 1);
}


double **
dmatr(int nrl, int nrh, int ncl, int nch)
{
	int 	i, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
	double	**m;
	
	/* allocate pointers to rows */
	m = (double **) malloc((unsigned)((nrow + 1) * sizeof(double*)));
	if (!m)
	{
	    fprintf(stderr, "libloc: allocation failure 1 in dmatr()");
	    exit(1);
	}

	m += 1;
	m -= nrl;
	
	/* allocate rows and set pointers to them */
	m[nrl] = (double *) malloc((unsigned)((nrow * ncol + 1) * sizeof(double)));
	if (!m[nrl])
	{
	    fprintf(stderr, "libloc: allocation failure 2 in dmatr()");
	    exit(1);
	}
	m[nrl] += 1;
	m[nrl] -= ncl;
	
	for(i=nrl+1; i<=nrh; i++)
	    m[i] = m[i-1] + ncol;
	
	return m;	/* return pointer to array of pointers to rows */
}


void
free_ivect(int *v, int nl, int nh)
{
	free((char*) (v + nl - 1));
}

void
free_dvect(double *v, int nl, int nh)
{
	free((char*) (v + nl - 1));
}


void
free_dmatr(double **m, int nrl, int nrh, int ncl, int nch)
{
	free((char*) (m[nrl] + ncl - 1));
	free((char*) (m + nrl - 1));
}
