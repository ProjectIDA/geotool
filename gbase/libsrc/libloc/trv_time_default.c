
/*
 * Copyright (c) 1996-1998 Science Applications International Corporation.
 *

 * NAME
 *	setup_default_tt_tables_only -- Setup default travel-time tables only
 *	fast_travel_time - Get travel time & slowness (no src-dpnt corr)
 *	super_fast_travel_time - Get a travel time estimate only
 *	get_phase_index -- Get phase index for given input phase 

 * FILE
 *	trv_time_default.c

 * SYNOPSIS
 *	int
 *	setup_default_tt_tables_only (vmodel_filename, phase_types, 
 *				      num_phase_types)
 *	char	*vmodel_filename;	(i) Velocity model file name location
 *	char	*phase_types;		(i) List of phase types desired
 *	int	num_phase_types;	(i) Number of phase types in above list

 *	double
 *	fast_travel_time (extrapolate, phase, ev_lat, ev_lon, sta_lat, sta_lon,
 *			  distance, ev_depth, esaz, sta_elev, horiz_slow, 
 *			  interp_code)
 *	Bool	extrapolate;		(i) Return T-T even if extrapolated
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	double	ev_lat;			(i) Event latitude (deg.)
 *	double	ev_lon;			(i) Event longitude (deg.)
 *	double	sta_lat;		(i) Station latitude (deg.)
 *	double	sta_lon;		(i) Station longitude (deg.)
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	ev_depth;		(i) Event depth (km.)
 *	double	esaz;			(i) Event-to-station azimuth (deg.)
 *	double	sta_elev;		(i) Station elevation (km.)
 *	double	*horiz_slow;		(o) Horizontal slowness (sec./deg.)
 *	int	*interp_code;		(o) Interpolation information code

 *	double
 *	super_fast_travel_time (phase, distance, depth)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	double	distance;		(i) Event-to-station distance (deg.
 *	double	depth;			(i) Event depth (km.)

 *	int
 *	get_phase_index (phase)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)

 *	double
 *	travel_time_wo_corr (extrapolate, phase, distance, ev_depth, esaz,
 *                            sta_elev, horiz_slow, interp_code)
 *	Bool	extrapolate;		(i) Return T-T even if extrapolated
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	ev_depth;		(i) Event depth (km.)
 *	double	sta_elev;		(i) Station elevation (km.)
 *	double	*horiz_slow;		(o) Horizontal slowness (sec./deg.)
 *	int	*interp_code;		(o) Interpolation information code
 *
 * DESCRIPTION
 *	Functions.  These routines are limited to reading phase-dependent-only
 *	travel-time table information.  That is, no station/phase-dependent
 *	information may be specified in the default velocity model 
 *	specification file (VMSF) for use by these routines.  This information
 *	may exist in the VMSF but will not be employed.  Note that the same
 *	travel-time structures are used, yet are not fully exploited for
 *	this limited capability.  No source-dependent knowledge may be 
 *	specified here including, SSSC's, SRST's, test-site correction, nor
 *	bulk station corrections.

 *	-- setup_default_tt_tables_only() is the main interface used for 
 *	reading the default phase-dependent-only travel-time tables as 
 *	formatted following the tt_table structure.  This function reads 
 *	and initializes the travel-time tables for various Locator-related 
 *	routines (e.g., ARS, LocSAT, EvLoc, GAcons and ESAL) indepedent of
 *	any station-related knowledge typically supplied by VMSF.  These 
 *	tables are read from disk each time a change is detected in the 
 *	list of current and previous phases being maintained by this 
 *	routine.  Travel-time table information is stored local to this 
 *	routine using the tt_table structure (see include file, tt_table.h, 
 *	for specifics).

 *	-- fast_travel_time() is used to obtain travel-time, slowness and 
 *	derivative information with ellipticity and elevation corrections 
 *	applied.  It does not apply source-dependent corrections, but will
 *	determine path-dependent travel-times.  Use bi-cubic spline inter-
 *	polation.

 *	-- super_fast_travel_time() is used to obtain a very quick estimate
 *	of travel-time based only on the default travel-time tables (i.e.,
 *	no station/phase-dependent info will be incorporated.  It is super-
 *	fast in that it will not allow extrapolation and performs a simple
 *	bi-linear interpolation without elevation or ellipticity corrrections.

 *	-- get_phase_index(), as the name implies, is used to determine the
 *	travel-time related phase index.

 * DIAGNOSTICS
 *	See locator_error_table[] in file, loc_error_msg.c, for specific 
 *	global error code descriptions.

 *	-- fast_travel_time() will return with a value of -1.0 if a
 *	problem is encountered (f.e., an invalid phase type; problems 
 *	with interpolation routine).  Elsewise, the travel time (sec.) is
 *	returned if successful.

 *	-- super_fast_travel_time() will return with a value of -1.0 if a
 *	problem is encountered (f.e., an invalid phase type; problems 
 *	with interpolation routine).  Elsewise, the travel time (sec.) is
 *	returned if successful.  No extrapolation and path-dependent curves
 *	are permitted.

 *	-- get_phase_index() will return an error code of ERR (-1) if the
 *	requested input phase is invalid.  Else, it will return the integer
 *	phase index.

 *
 *      -- travel_time_wo_corr() returns a travel time without ellipticity
 *      corrections.

 * FILES
 *	Travel-time table files are read by function, read_tt_tables().

 * NOTES
 *	These routines are limited to phase-dependent travel-time tables
 *	only.  That is, no station/phase-dependent information may be
 *	employed by these functions.

 * SEE ALSO
 *	read_tt_tables() for reading of travel-time and model error info.

 * AUTHOR
 *	Walter Nagy,  5/ 6/96,	Created.
 */

#include "config.h"
#include <stdio.h>
#include <math.h>
#include "libloc.h"
#include "locp.h"
#include "libLP.h"
#include "libinterp.h"
#include "libgeog.h"
#include "loc_defs.h"

#define	SURF_P_VEL	5.8		/* Surface P-wave velocity (km/s) */
#define	SURF_S_VEL	SURF_P_VEL/1.73	/* Surface S-wave velocity (km/s) */

static	Bool	first_tt_table_read = TRUE;
static	char	*prev_vmodel_filename = (char *) NULL;

static	int	num_phases = 0;
static	int	num_models = 0;
static	int	num_sta_phase_models = 0;

static	TT_Table	*tt_table = (TT_Table *) NULL;
static	Model_Descrip	*model_descrip = (Model_Descrip *) NULL;
static	Sta_Phase_Model	*sta_phase_model = (Sta_Phase_Model *) NULL;


int
setup_default_tt_tables_only (char *vmodel_filename, char **phase_types,
		int num_phase_types)
{
	Bool	lists_are_the_same;
	int	i, j;
	int	ierr;

	List_of_Phases	*ph, *next;


	if (num_phase_types == 0 || ! phase_types)
	{
		fprintf (stderr,"Error: setup_default_tt_tables_only: Null phase list!\n");
		return (TTerror1);
	}

	/*
	 * If not first read, then check to see if directory prefix of travel-
	 * time tables or phase list has changed.  If either of these has
	 * changed, then proceed to reading new tables.  In the case of
	 * identical phase lists and directory prefixes, then just return.
	 */

	if (! first_tt_table_read)
	{
	    if (STREQ(vmodel_filename, prev_vmodel_filename) &&
		    num_phases == num_phase_types)
	    {
		lists_are_the_same = TRUE;
		for (j = 0; j < num_phases; j++)
		{
		    if (strcmp (phase_types[j], tt_table[j].phase))
		    {
			lists_are_the_same = FALSE;
			break;
		    }
		}
		if (lists_are_the_same)
		    return (OK);	/* 2 lists are identical */
	    }

	    /*
	     * The current phase list is different from the previous phase
	     * list.  Therefore, free up previous travel-time table info.
	     */

	    UFREE (prev_vmodel_filename);
	    for (i = 0; i < num_phases; i++)
	    {
		for (j = 0; j < tt_table[i].num_depths; j++)
		    UFREE (tt_table[i].trv_time[j]);
		UFREE (tt_table[i].trv_time);
		if (tt_table[i].ec_table != NULL)
		{
		    for (j = 0; j < tt_table[i].ec_table->num_depths; j++)
		    {
			UFREE (tt_table[i].ec_table->t0[j]);
			UFREE (tt_table[i].ec_table->t1[j]);
			UFREE (tt_table[i].ec_table->t2[j]);
		    }
		    UFREE (tt_table[i].ec_table->t0);
		    UFREE (tt_table[i].ec_table->t1);
		    UFREE (tt_table[i].ec_table->t2);
		    UFREE (tt_table[i].ec_table);
		}
		if (tt_table[i].model_error != NULL)
		{
		    if (tt_table[i].model_error->dist_depth_var != NULL)
		    {
			for (j = 0; j < tt_table[i].model_error->num_depths;j++)
			    UFREE (tt_table[i].model_error->dist_depth_var[j]);
			UFREE (tt_table[i].model_error->dist_depth_var);
		    }
		    UFREE (tt_table[i].model_error);
		}
	    }
	    UFREE (tt_table);

	    for (i = 0; i < num_models; i++)
	    {
		ph = model_descrip[i].list_of_phases;
		while (ph != (List_of_Phases *) NULL)
		{
		    next = ph->next;
		    UFREE (ph);
		    ph = next;
		}
	    }
	    UFREE (model_descrip);
	}

	num_phases = num_phase_types;
	prev_vmodel_filename = STRALLOC (vmodel_filename);

	/*
	 * Do actual reading of travel-time tables here!
	 */

	ierr = read_tt_tables (TRUE, vmodel_filename, phase_types, 
				num_phase_types, &tt_table, &num_phases, 
				&model_descrip, &num_models, 
				&sta_phase_model, &num_sta_phase_models);

	/*
	 * If absolutely no VMSF found, then do not attempt tests above.
	 * On the other hand, if VMSF found, yet some other error was
	 * encountered while attempting to read T-T info, then we need
	 * to make sure to free any memory allocated within function,
	 * read_tt_tables(), so set first_tt_table_read = FALSE.
	 */

	if (ierr != TTerror5)
	    first_tt_table_read = FALSE;

	/*
	 * Return status as determined by function, read_tt_tables().
	 */

	return (ierr);
}


double
fast_travel_time (Bool extrapolate, char *phase, double ev_lat, double ev_lon,
		   double sta_lat, double sta_lon, double distance, 
		   double ev_depth, double esaz, double sta_elev, 
		   double *horiz_slow, int *interp_code)
/* horiz_slow;	Horizontal slowness (sec/deg): dtdr */ 
{

	Bool	in_hole;
	int	fatal_err = 0;
	int	phase_index = -1;
	int	leg;
	float	value, x_1st_deriv, z_1st_deriv, x_2nd_deriv, z_2nd_deriv;
	double	el, elev_corr, ellip_corr;
	double	surf_vel;
	double	trv_time;
	double	*LP_trv_time = (double *) NULL ;

	static	double	save_ev_lat = 99.0;
	static	double	ev_geoc_co_lat = 0.0;
	static	double	period[] = { 20.0 };


	*interp_code = 0;

	phase_index = get_phase_index (phase);

	if (phase_index < 0)
	    return (-1.0);		/* Indicates phase index not found */

	if (phase_index > num_phases)
	{
	    if (phase_index == LR_PHASE_INDEX)
	    {
		LP_trv_time = LP_trace_rays (ev_lat, ev_lon, sta_lat, sta_lon, LR, period, 1);
		trv_time = LP_trv_time[0];
		*horiz_slow = trv_time/distance;
                free(LP_trv_time);       	
		return (trv_time);
	    }
	    else if (phase_index == LQ_PHASE_INDEX)
	    {
		LP_trv_time = LP_trace_rays (ev_lat, ev_lon, sta_lat, sta_lon, LQ, period, 1);
		trv_time = LP_trv_time[0];
		*horiz_slow = trv_time/distance;
                free(LP_trv_time);		
		return (trv_time);
	    }
	    else
	    {
		fprintf (stderr, "fast_travel_time: phase_index: %d is too large for phase: %s!!!\n", phase_index, phase);
		return (-1.0);		/* Indicates phase index not found */
	    }
	}

	/*
	 * If a new event latitude (deg) is input, then compute a new event 
	 * geocetric co-latitude (radians).
	 */

	if (ev_lat != save_ev_lat)
	{
	    ev_geoc_co_lat = lat_conv (ev_lat, TRUE, TRUE, FALSE, TRUE, FALSE);
	    save_ev_lat = ev_lat;
	}

	/*
	 * If travel-time table for this phase at input distance has a hole,
	 * then set in_hole flag.
	 */

	if (distance > tt_table[phase_index].in_hole_dist[0] &&
	    distance < tt_table[phase_index].in_hole_dist[1])
	    in_hole = TRUE;
	else
	    in_hole = FALSE;

	/*
	 * Grab travel-time, slowness and derivatives
	 */

	fatal_err = 
	interpolate_table_value (extrapolate, in_hole, FALSE,
				 tt_table[phase_index].num_dists,
				 tt_table[phase_index].num_depths,
				 tt_table[phase_index].dist_samples,
				 tt_table[phase_index].depth_samples,
				 tt_table[phase_index].trv_time,
                                 (float)distance, (float)ev_depth, &value,
				 &x_1st_deriv, &z_1st_deriv,
				 &x_2nd_deriv, &z_2nd_deriv, interp_code);

	if (fatal_err == ERR)
	{
	    /*
	    fprintf (stderr, "Warning: Problem with extrapolation for %s at delta = %f & depth = %f\n", phase, distance, ev_depth);
	     */
	    return (-1.0);
	}

	if (fatal_err != OK)
	    return (-1.0);

	if (!extrapolate && *interp_code != 0)
	    return (-1.0);

	*horiz_slow = (double)x_1st_deriv;   /* dtdr (horiz slow (sec/deg)) */


	/*
	 * Determine an ellipticity correction!  If new-style (distance/depth
	 * dependent tables by phase) elliptiticity corrections exist, use
	 * these in preference to those simplified value stored in function,
	 * ellipticity_corr().  Currently (9/6/95), only new-style ellipticity
	 * correction tables exist for velocity model, ak135.
	 */

	if (tt_table[phase_index].ec_table != (EC_Table *) NULL)
	    ellip_corr = 
	    get_ec_from_table (tt_table[phase_index].ec_table, distance, esaz, 
			       ev_geoc_co_lat, ev_depth);
	else
	    ellip_corr = 
	    ellipticity_corr (distance, esaz, ev_geoc_co_lat, ev_depth, phase);

	/* 
	 * Calculate elevation correction 
	 */

	elev_corr = 0.0;
	if (sta_elev > -998.0)
	{
	    leg = last_leg (phase);
	    if (leg == LAST_LEG_IS_P)
		surf_vel = SURF_P_VEL;
	    else if (leg == LAST_LEG_IS_S)
		surf_vel = SURF_S_VEL;
	    else
		surf_vel = 0.0;

	    if (surf_vel > 0.0)
	    {
		el = surf_vel*KM_TO_DEG * (*horiz_slow);
		el = el*el;
		if (el > 1.0)
		    el = 1.0/el;
		el = 1.0 - el;
		el = sqrt(el);
		elev_corr = sta_elev*el/surf_vel;
	    }
	}

	trv_time = value + elev_corr + ellip_corr;

	return (trv_time);
}

/*
 * distance: Input distance (deg.)
 * depth: Input depth (km.) below Earth's surface
 */
double
super_fast_travel_time (char *phase, double distance, double depth)
{

	Bool	max_depth_index_found = FALSE;
	int	i, j;
	int	phase_index = -1;
	double	tmp;
	double	depth_diff;
	double	tt_at_bottom, tt_at_top;
	double	trv_time;		/* Return crude T-T estimate */


	/*
	 * This is a quick-and-dirty routine for return a crude estimate 
	 * of travel-time (sec.) given and input distance (in arc deg.)
	 * and depth (in km.).  Bi-linear interpolation is used, so user's 
	 * should not assume that these times are better than about +/-2 sec. 
	 * Will ignore path-dependent long-period travel-time information.
	 */

	phase_index = get_phase_index (phase);

	if (phase_index < 0 
	    || phase_index == LR_PHASE_INDEX || phase_index == LQ_PHASE_INDEX)
	   return (-1.0);

	if (tt_table[phase_index].dist_samples[0] > distance)
	    return (-1.0);	/* Input distance < lower distance bound */

	/*
	 * Event depth cannot be negative
	 */

	if (depth < 0.0)
	    return (-1.0);

	for (i = 1, j = 1; i < tt_table[phase_index].num_depths; i++)
	{
	    if (tt_table[phase_index].depth_samples[i] > depth)
	    {
		j = i;		/* j: Deepest depth index */
		max_depth_index_found = TRUE;
		break;
	    }
	}
	if (!max_depth_index_found)
	    j = tt_table[phase_index].num_depths - 1;

	for (i = 1; i < tt_table[phase_index].num_dists; i++)
	{
	    if (tt_table[phase_index].dist_samples[i] > distance)
	    {
		tmp = (distance - tt_table[phase_index].dist_samples[i-1])/
			      (tt_table[phase_index].dist_samples[i] -
			       tt_table[phase_index].dist_samples[i-1]);
		tt_at_bottom = tt_table[phase_index].trv_time[j][i-1] +
			       tmp * (tt_table[phase_index].trv_time[j][i] -
			              tt_table[phase_index].trv_time[j][i-1]);
		tt_at_top = tt_table[phase_index].trv_time[j][i-1] +
			    tmp * (tt_table[phase_index].trv_time[j-1][i] -
			           tt_table[phase_index].trv_time[j-1][i-1]);
		depth_diff = tt_table[phase_index].depth_samples[j] -
			     tt_table[phase_index].depth_samples[j-1];

		if (max_depth_index_found)
		{
		    trv_time = tt_at_top +
			(tt_at_bottom - tt_at_top) * 
			((depth - tt_table[phase_index].depth_samples[j-1]) / 
			  depth_diff);
		}
		else
		{	/* Extrapolate depth ! */
		    tmp = (tt_at_bottom - tt_at_top) / depth_diff;
		    trv_time = tt_at_bottom + tmp * (depth - tt_table[phase_index].depth_samples[j]);
		}

		return (trv_time);
	    }
	}
	return (-1.0);		/* Input distance > upper distance bound */
}


int
get_phase_index (char *phase)
{
	int i;


	if (!phase || !*phase)
	    return (ERR);

	/*
	 * Handle LR and LQ phases as special cases.  These tables are
	 * processed in a path-dependent manor, so we will assign a special
	 * phase_index for each here.  The phase index for LR will be
	 * LR_PHASE_INDEX, while LQ will be LQ_PHASE_INDEX.
	 */

	if (STREQ (phase, "LR") || !strncmp (phase, "nL", 2) 
				|| !strncmp (phase, "NL", 2))
	    return (LR_PHASE_INDEX);
	if (STREQ (phase, "LQ"))
	    return (LQ_PHASE_INDEX);

	/*
	 * No station-specific travel-time information available for 
	 * this phase type, so simply use default travel-time table for input 
	 * input phase, if available.  
	 */
	 
	for (i = 0; i < num_phases; i++)
	    if (STREQ (phase, tt_table[i].phase) && tt_table[i].num_dists > 0)
		return (i);

	return (ERR);
}

/* horiz_slow: Horizontal slowness (sec/deg): dtdr */ 
double
travel_time_wo_corr (Bool extrapolate, char *phase, 
		   double distance, double ev_depth, double sta_elev, 
		   double *horiz_slow, int *interp_code)
{

	Bool	in_hole;
	int	fatal_err = 0;
	int	phase_index = -1;
	int	leg;
	float	value, x_1st_deriv, z_1st_deriv, x_2nd_deriv, z_2nd_deriv;
	double	el, elev_corr;
	double	surf_vel;
	double	trv_time;

	*interp_code = 0;

	phase_index = get_phase_index (phase);

	if (phase_index < 0)
	    return (-1.0);		/* Indicates phase index not found */

	if (phase_index > num_phases)
	{
	    fprintf (stderr, "fast_travel_time: phase_index: %d is too large for phase: %s!!!\n", phase_index, phase);
	    return (-1.0);		/* Indicates phase index not found */
	}

	/*
	 * If travel-time table for this phase at input distance has a hole,
	 * then set in_hole flag.
	 */

	if (distance > tt_table[phase_index].in_hole_dist[0] &&
	    distance < tt_table[phase_index].in_hole_dist[1])
	    in_hole = TRUE;
	else
	    in_hole = FALSE;

	/*
	 * Grab travel-time, slowness and derivatives
	 */

	fatal_err = 
	interpolate_table_value (extrapolate, in_hole, FALSE,
				 tt_table[phase_index].num_dists,
				 tt_table[phase_index].num_depths,
				 tt_table[phase_index].dist_samples,
				 tt_table[phase_index].depth_samples,
				 tt_table[phase_index].trv_time,
                                 (float)distance, (float)ev_depth, &value,
				 &x_1st_deriv, &z_1st_deriv,
				 &x_2nd_deriv, &z_2nd_deriv, interp_code);

	if (fatal_err == ERR)
	{
	    /*
	    fprintf (stderr, "Warning: Problem with extrapolation for %s at delta = %f & depth = %f\n", phase, distance, ev_depth);
	     */
	    return (-1.0);
	}

	if (fatal_err != OK)
	    return (-1.0);

	if (!extrapolate && *interp_code != 0)
	    return (-1.0);

	*horiz_slow = (double)x_1st_deriv;   /* dtdr (horiz slow (sec/deg)) */

	/* 
	 * Calculate elevation correction 
	 */

	elev_corr = 0.0;
	if (sta_elev > -998.0)
	{
	    leg = last_leg (phase);
	    if (leg == LAST_LEG_IS_P)
		surf_vel = SURF_P_VEL;
	    else if (leg == LAST_LEG_IS_S)
		surf_vel = SURF_S_VEL;
	    else
		surf_vel = 0.0;

	    if (surf_vel > 0.0)
	    {
		el = surf_vel*KM_TO_DEG * (*horiz_slow);
		el = el*el;
		if (el > 1.0)
		    el = 1.0/el;
		el = 1.0 - el;
		el = sqrt(el);
		elev_corr = sta_elev*el/surf_vel;
	    }
	}

	trv_time = value + elev_corr;

	return (trv_time);
}
