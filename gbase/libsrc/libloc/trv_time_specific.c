
/*
 * Copyright (c) 1990-1998 Science Applications International Corporation.
 *

 * NAME
 *	setup_tt_facilities -- Read travel-time tables & link their station info
 *	setup_tttables -- Given a list of phases, read travel-time tables
 *	total_travel_time -- Get travel time and derivatives w/ all corrections
 *	compute_ttime_w_corrs -- Get travel time w/ all corrections (no deriv.)
 *	trv_time_w_ellip_elev - Get travel time & derivatives (no src-dpnt corr)
 *	crude_but_quick_dist -- Get crude distance estimate from travel time
 *	crude_but_quick_trv_time -- Get crude travel time estimate from distance
 *	crude_tt_w_only_phase - Get crude travel time estimate with only phase
 *	initialize_ar_info -- Initialize ar_info structure to N/A values
 *	initialize_loc_params -- Initialize loc_params structure to N/A values
 *	get_tt_indexes -- Get travel-time indexes for phase of SPM
 *	get_default_phase_index -- Get default phase index (use no SPM info)
 *	get_model_error -- Get T-T modelling error for phase at distance/depth
 *	set_sta_pt -- Set station pointers for rapid search of associated phases
 *	get_sta_index -- Get station index

 * FILE
 *	trv_time_specific.c

 * SYNOPSIS
 *	int
 *	setup_tt_facilities (vmodel_filename, phase_types, num_phase_types,
 *			     sites, num_sites)
 *	char	*vmodel_filename;	(i) Velocity model file name location
 *	char	**phase_types;		(i) List of phase types desired
 *	int	num_phase_types;	(i) Number of phase types in above list
 *	Site	*sites;			(i) Station (site table) structure
 *	int	num_sites;		(i) Number of station (site) records

 *	int
 *	setup_tttables (vmodel_filename, phase_types, num_phase_types)
 *	char	*vmodel_filename;	(i) Velocity model file name location
 *	char	*phase_types;		(i) List of phase types desired
 *	int	num_phase_types;	(i) Number of phase types in above list

 *	double
 *	total_travel_time (locator_params, sites, ar_info, extrapolate,
 *			   do_we_need_z_derivs, ev_lat, ev_lon, ev_depth, 
 *			   distance, esaz, phase, sta_index, phase_index,
 *			   spm_index, data_std_err, prin_deriv, tt_deriv,
 *			   slow_deriv, az_deriv, interp_code)
 *	Locator_params *locator_params;	(i) Locator parameter info structure
 *	Site	*sites;			(i) Station structure
 *	Ar_Info	*ar_info;		(i) Arrival-based locator info
 *	Bool	extrapolate;		(i) Retrun T-T even if extrapolated
 *	Bool	do_we_need_z_derivs;	(i) Do we need depth derivatives ?
 *	double	ev_lat;			(i) Event latitude (deg.)
 *	double	ev_lon;			(i) Event longitude (deg.)
 *	double	ev_depth;		(i) Event depth (km.)
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	esaz;			(i) Event-to-station azimuth (deg.)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	int	sta_index;		(i) Station index of sites table
 *	int	phase_index;		(i) Phase index for T-T access
 *	int	spm_index;		(i) SPM index of SPM structure access
 *	double	data_std_err;		(i) A priori data standard error
 *	double	*prin_deriv;		(o) Principal derivatives
 *	double	*tt_deriv;		(o) Component travel-time derivatives
 *	double	*slow_deriv;		(o) Component slowness derivatives
 *	double	*az_deriv;		(o) Component azimuth derivatives
 *	int	*interp_code;		(o) Interpolation information code

 *	double
 *	compute_ttime_w_corrs (locator_params, sites, extrapolate, 
 *			       ev_lat, ev_lon, ev_depth, distance, esaz,
 *			       phase, sta_index, ar_info, horiz_slow, 
 *			       interp_code) 
 *	Locator_params *locator_params;	(i) Locator parameter info structure
 *	Site	*sites;			(i) Station structure
 *	Bool	extrapolate;		(i) Retrun T-T even if extrapolated
 *	double	ev_lat;			(i) Event latitude (deg.)
 *	double	ev_lon;			(i) Event longitude (deg.)
 *	double	ev_depth;		(i) Event depth (km.)
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	esaz;			(i) Event-to-station azimuth (deg.)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	int	sta_index;		(i) Station index of sites table
 *	Ar_Info	*ar_info;		(o) Arrival-based locator info
 *	double	*horiz_slow;		(o) Horizontal slowness (sec./deg.)
 *	int	*interp_code;		(o) Interpolation information code

 *	double
 *	trv_time_w_ellip_elev (extrapolate, do_we_need_z_derivs, ev_lat, 
 *			       distance, ev_depth, esaz, phase, sta_elev, 
 *			       phase_index, spm_index, prin_deriv, interp_code)
 *	Bool	extrapolate;		(i) Return T-T even if extrapolated
 *	Bool	do_we_need_z_derivs;	(i) Do we need depth derivatives ?
 *	double	ev_lat;			(i) Event latitude (deg.)
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	ev_depth;		(i) Event depth (km.)
 *	double	esaz;			(i) Event-to-station azimuth (deg.)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	double	sta_elev;		(i) Station elevation (km.)
 *	int	phase_index;		(i) Phase index for T-T access
 *	int	spm_index;		(i) SPM index for SPM structure access
 *	double	*prin_deriv;		(o) Principal derivatives
 *	int	*interp_code;		(o) Interpolation information code

 *	double
 *	crude_but_quick_dist (phase_index, trv_time)
 *	int	phase_index;		(i) Phase index
 *	double	trv_time;		(o) Travel time (sec.)

 *	double
 *	crude_but_quick_trv_time (phase_index, distance, depth)
 *	int	phase_index;		(i) Phase index
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	depth;			(i) Depth below Earth's surface (km.)

 *	double
 *	crude_tt_w_only_phase (phase, distance, depth)
 *	char	*phase;			(i) Phase name
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	depth;			(i) Depth below Earth's surface (km.)

 *	Ar_Info
 *	initialize_ar_info ()

 *	Locator_params
 *	initialize_loc_params ()

 *	int
 *	get_tt_indexes (phase, sta_index, spm_index)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)
 *	int	sta_index;		(i) Station index of site table
 *	int	*spm_index;		(o) SPM index for SPM structure access

 *	int
 *	get_default_phase_index (phase)
 *	char	*phase;			(i) Phase type (e.g., P, PKP, PcP)

 *	double
 *	get_model_error (phase_index, delta, depth)
 *	int	phase_index;		(i) Phase type index 
 *	double	delta;			(i) Event-to-station distance (deg.)
 *	double	depth;			(i) Depth below Earth's surface (km.)

 *	int
 *	set_sta_pt (site, num_sites)
 *	Site	*site;			(i) Site structure (station info)
 *	int	num_sites;		(i) Number of site table entries

 *	int
 *	get_sta_index (sta)
 *	char	*sta;			(i) Station name

 * DESCRIPTION
 *	Functions.  These routines provide the core travel-time table handling
 *	facilities for the SAIC location module.  All functionality local to
 *	the travel-time table structure, tt_table, is handled within this
 *	file.

 *	-- setup_tt_facilities() is the primary user interface used for 
 *	reading travel-time tables and linked station information to them.
 *	This routine needs to be called before any other travel-time
 *	manipulation facility is accessed.

 *	-- setup_tttables() is the main interface used for reading travel-
 *	time tables as formatted following the tt_table structure.  This 
 *	function reads and initializes the travel-time tables for various
 *	Locator-related routines (e.g., ARS, LocSAT, EvLoc, GAcons and ESAL).
 *	These tables are read from disk each time a change is detected in
 *	the list of tables being maintained and the list of tables input 
 *	through the argument list.  Travel-time table information is stored
 *	local to this routine using the tt_table structure (see include file,
 *	tt_table.h, for specifics).

 *	-- total_travel_time() is used to obtain travel-time, slowness and 
 *	derivative information with all corrections applied.  This is the
 *	function used by compute_hypo() to do actual iterative location.
 *	This is the most complete of all travel-time calculation functions
 *	in that it accesses all source-dependent, ellipticity and elevation
 *	corrections requested by the user.

 *	-- compute_ttime_w_corrs() is just like total_travel_time(), except
 *	that derivative information is not returned.  This is often desirable
 *	when location itself is not needed, but the travel time and slowness
 *	is needed.  This is used, for example, by ARS to "Align on Phase",
 *	which obviously has no need of derivatives.

 *	-- trv_time_w_ellip_elev() is used to obtain travel-time, slowness and 
 *	derivative information with ellipticity and elevation corrections 
 *	applied.

 *	-- crude_but_quick_dist() makes a crude distance estimate based on an
 *	given travel time, assumed to be at the Earth's surface.  This is
 *	currently used by function, best_guess(), only, where a rough estimate
 *	of distance for getting an initial location is sufficient.

 *	-- crude_but_quick_trv_time() makes a crude travel time estimate based 
 *	on a given distance and depth.  This is currently used by function, 
 *	best_guess(), only, where a rough estimate of travel time for getting 
 *	an initial location is sufficient.  I (WCN) envision this function
 *	being used for numerous application where only a crude estimate of
 *	travel time is needed (e.g., DFX, where only a rough time window is
 *	desired).  This function DOES NOT extrapolate beyond valid distance/
 *	depth ranges nor does it deal with path-dependent long-period LR and 
 *	LQ phases.

 *	-- crude_tt_w_only_phase() makes a crude travel time estimate based 
 *	only onb a given distance and depth and phase type (i.e., no station-
 *	dependency can be addressed).  This function DOES NOT extrapolate 
 *	beyond valid distance/depth ranges nor does it deal with path-dependent 
 *	long-period LR and LQ phases.

 *	-- initialize_ar_info(), initializes ar_info structure (type, Ar_Info)
 *	to N/A values.

 *	-- initialize_loc_params(), initializes loc_params structure (type, 
 *	Locator_params) to N/A values.

 *	-- get_tt_indexes(), as the name implies, is used to determine the
 *	travel-time related indexes for phase and station/phase/model info.

 *	-- get_default_phase_index() extracts the non-station/phase-dependent
 *	phase index for the given input phase.  That is, it simply get the
 *	index based on the phase type for the default set of travel-time 
 *	tables.

 *	-- get_model_error(), as the name implies, is used to extract the
 *	modelling error, which is stored with the travel-time tables.  Either
 *	a single-valued, distance-dependent, or distance/depth-depth 
 *	modelling error may exist.  These are optimized to expedite the
 *	access to these errors.  If the input distance and/or depth is 
 *	outside the specified range and domain, then the modelling error
 *	returned comes from the nearest edge.

 *	-- set_sta_pt(), links the station pointer structure (type, Sta_Pt)
 *	with a list of phases, corresponding to a given station, for rapid
 *	access.

 *	-- get_sta_index() extracts station index for the given input station.

 * DIAGNOSTICS
 *	See locator_error_table[] in file, loc_error_msg.c, for specific 
 *	global error code descriptions.

 *	-- trv_time_w_ellip_elev() will return with a value of -1.0 if a
 *	problem is encountered (f.e., an invalid phase type; problems 
 *	with interpolation routine).  Elsewise, the travel time (sec.) is
 *	returned if successful.

 *	-- get_tt_indexes() will return an error code of ERR (-1) if the
 *	requested input phase is invalid.  Else, it will return the integer
 *	phase index.

 *	-- get_default_phase_index() will return an error code of ERR (-1) 
 *	if the requested input phase is not part of the default set of
 *	travel-time tables list of phases.  Else, it will return the 
 *	integer phase index.

 *	-- initialize_ar_info(), returns a default ar_info structure (type, 
 *	Ar_Info) populated with N/A values.

 *	-- initialize_loc_params(), returns a default loc_params structure 
 *	(type, Locator_params) populated with N/A values.

 *	-- get_model_error(), returns the travel-time modelling error (sec.)
 *	for the given input phase.  If the phase does not have an associated
 *	modelling error, a default value of 1.0 sec. is returned.  If the
 *	input phase is an LR or LQ phase type, then a modelling error of
 *	30 sec. will be returned.

 *	-- set_sta_pt() will return an error code of ERR (-1) if memory
 *	cannot be allocated.  It will return an error code of -2 if no
 *	input site table exists OR the site table is empty.

 * FILES
 *	Travel-time table files are read by function, read_tt_tables().

 * NOTES
 *	The acronym, SPM, stands for the station_phase_model structure that
 *	contains station-specific information regarding the chosen 1-D
 *	velocity model (travel-time table), bulk station correction and 
 *	individual station sedimentary velocity.  VMSF is an acronym for
 *	velocity model specification file.  Two quick-and-dirty travel-time
 *	functions, crude_but_quick_trv_time() and crude_tt_w_only_phase(),
 *	do not extrapolate beyond valid distance/depth ranges.

 * SEE ALSO
 *	read_tt_tables() for reading of travel-time and model error info.

 * AUTHOR
 *	Walter Nagy, 5/94,	Created.
 *	Walter Nagy, 7/94,	Added functions, total_travel_time(), 
 *				crude_but_quick_dist() and 
 *				crude_but_quick_trv_time().
 *	Walter Nagy, 4/21/95,	A T-T derivative correction is now made to
 *				when SSSC's are employed.
 *	Walter Nagy, 8/25/95,	Added condition in total_travel_time() to
 *				calculate path-dependent LR/LQ travel times
 *				using function, LP_trace_ray().
 *	Walter Nagy, 8/29/95,	Boolean variable, do_we_need_z_derivs,
 *				added as 2nd argument to function, 
 *				trv_time_w_ellip_elev() and 5th argument
 *				to function, total_travel_time().  
 *	Walter Nagy, 9/95,	Added function, get_tt_index(), to determine 
 *				phase index given station specific knowledge.
 *	Walter Nagy, 9/8/95,	Ellipticity corrections are now directly a part
 *				of tt_table structure.
 *	Walter Nagy, 9/18/95,	Added function, set_sta_pt().
 *	Walter Nagy, 9/18/95,	Implemented static bulk station corrections.
 *				Also allow for SPM sedimentary velocities for
 *				calculating better elevation corrections.
 *	Walter Nagy, 9/20/95,	Added T-T modelling error functionality by 
 *				including these variances with travel-time 
 *				tables via new function, get_model_error().
 *	Walter Nagy, 10/27/95,	Added special handling of default SPM info as
 *				represented by phase designators, *P and *S.
 *	Walter Nagy, 11/30/95,	Accomodates new SSSC structure.
 *	Walter Nagy, 1/9/96,	New arrival-based location info structure,
 *				ar_info, now passed thru function, 
 *				total_travel_time().  Also added new function,
 *				initialize_ar_info() to set ar_info N/A values
 *	Walter Nagy, 2/12/96,	Added new function, setup_tt_facilities(),
 *				which guarantees that both the travel-time
 *				tables will be read and station info will be
 *				link to them for efficient access.
 *	Walter Nagy, 5/ 2/96,	Added new function, initialize_loc_params(), 
 *				to set loc_params to N/A values.
 *	Walter Nagy, 5/10/96,	Added new function, get_default_phase_index(), 
 *				to get phase index from default set of T-T
 *				tables only.
 *	Walter Nagy, 4/ 7/98,	Added new function, crude_tt_w_only_phase(), 
 *				compute a crude T-T with only a phase, distance
 *				and depth (requested by Ethan).
 *	Jerry Guern, 6/25/98,	Added interphase to module to seasonally varying
 *				2-D travel times, called radial-2D method.
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
#include "tt_info.h"

#define	SURF_P_VEL	5.8		/* Surface P-wave velocity (km/s) */
#define	SURF_S_VEL	SURF_P_VEL/1.73	/* Surface S-wave velocity (km/s) */

static	Bool	first_tt_table_read = TRUE;
static	char	*prev_vmodel_filename = (char *) NULL;

static	int	num_phases = 0;		/* Number of phases belonging to
					 * default set of T-T curves */
static	int	total_num_phases = 0;	/* Total number of phases belonging to
					 * to all sets of T-T curves */
static	int	num_models = 0;		/* # of different sets of T-T curves */
static	int	num_sta_phase_models = 0;

static	int	num_stations = 0;	/* # of sites */

static	TT_Table	*tt_table = (TT_Table *) NULL;
static	Model_Descrip	*model_descrip = (Model_Descrip *) NULL;
static	Sta_Phase_Model	*sta_phase_model = (Sta_Phase_Model *) NULL;
static	Sta_Pt		*sta_pt = (Sta_Pt *) NULL;
static	Ar_Info		tt_info;


int
setup_tt_facilities (char *vmodel_filename, char **phase_types, 
		     int num_phase_types, Site *sites, int num_sites)
{
	int	tt_read_error = OK;

	tt_read_error =
	setup_tttables (vmodel_filename, phase_types, num_phase_types);
	if (tt_read_error != OK)
	    return (tt_read_error);

	if (set_sta_pt (sites, num_sites) != OK)
	{
	    fprintf (stderr, "%s\n", loc_error_msg (TTerror6));
	    return (TTerror6);
	}

	return (OK);
}


int
setup_tttables (char *vmodel_filename, char **phase_types, int num_phase_types)
{
	Bool	lists_are_the_same;
	int	i, j;
	int	ierr=0;
	Sssc	*sssc_ptr;

	List_of_Phases	*ph, *next;


	if (num_phase_types == 0 || ! phase_types)
	{
		fprintf (stderr,"Error: setup_tttables: Null phase list!\n");
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
	    /* for (i = 0; i < num_phases; i++) */
	    for (i = 0; i < total_num_phases; i++)
	    {
		for (j = 0; j < tt_table[i].num_depths; j++)
		    UFREE (tt_table[i].trv_time[j]);
		UFREE (tt_table[i].trv_time);
		if (i < num_phases)
		{
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
			UFREE (tt_table[i].ec_table->dist_samples);
			UFREE (tt_table[i].ec_table->depth_samples);
			UFREE (tt_table[i].ec_table);
		    }
		}
		else
		    tt_table[i].ec_table = (EC_Table *) NULL;

		if (tt_table[i].model_error != NULL)
		{
		    UFREE(tt_table[i].model_error->depth_samples);
		    UFREE(tt_table[i].model_error->dist_var);
		    UFREE(tt_table[i].model_error->dist_samples);

		    if (tt_table[i].model_error->dist_depth_var != NULL)
		    {
			for (j = 0; j < tt_table[i].model_error->num_depths;j++)
			    UFREE (tt_table[i].model_error->dist_depth_var[j]);
			UFREE (tt_table[i].model_error->dist_depth_var);
		    }
		    UFREE (tt_table[i].model_error);
		}
		UFREE (tt_table[i].depth_samples);
		UFREE (tt_table[i].dist_samples);
	    }
	    UFREE (tt_table);

	    for (i = 0; i < num_sta_phase_models; i++)
	    {
		if (sta_phase_model[i].sssc != (Sssc *) NULL)
		{
		    sssc_ptr = sta_phase_model[i].sssc->finer_detail;
		    if (sssc_ptr != (Sssc *) NULL)
		    {
			UFREE (sssc_ptr->lat_grid_ref);
			UFREE (sssc_ptr->lon_grid_ref);
			for (j = 0; j < sssc_ptr->nlats; j++)
			{
			    UFREE (sssc_ptr->sta_cor[j]);
			    UFREE (sssc_ptr->sssc_2nd_deriv[j]);
			}
			UFREE (sssc_ptr->sta_cor);
			UFREE (sssc_ptr->sssc_2nd_deriv);
			UFREE (sssc_ptr->sssc_path);
			UFREE (sssc_ptr);
		    }
		    sssc_ptr = sta_phase_model[i].sssc;
		    UFREE (sssc_ptr->lat_grid_ref);
		    UFREE (sssc_ptr->lon_grid_ref);
		    for (j = 0; j < sssc_ptr->nlats; j++)
		    {
			UFREE (sssc_ptr->sta_cor[j]);
			UFREE (sssc_ptr->sssc_2nd_deriv[j]);
		    }
		    UFREE (sssc_ptr->sta_cor);
		    UFREE (sssc_ptr->sssc_2nd_deriv);
		    UFREE (sssc_ptr->sssc_path);
		    UFREE (sssc_ptr);
		}
	    }
	    UFREE (sta_phase_model);
	    sta_phase_model = (Sta_Phase_Model *) NULL;

	    for (i = 0; i < num_models; i++)
	    {
		ph = model_descrip[i].list_of_phases;
		while (ph != (List_of_Phases *) NULL)
		{
		    next = ph->next;
		    UFREE (ph);
		    ph = next;
		}
		model_descrip[i].list_of_phases = (List_of_Phases *) NULL;
		UFREE (model_descrip[i].dir_pathway);
	    }
	    UFREE (model_descrip);
	    model_descrip = (Model_Descrip *) NULL;
	}

	num_phases = num_phase_types;
	prev_vmodel_filename = STRALLOC (vmodel_filename);

	/*
	 * Do actual reading of travel-time tables here!
	 */

	ierr = read_tt_tables (FALSE, vmodel_filename, phase_types, 
				num_phase_types, &tt_table, 
				&total_num_phases, &model_descrip,
				&num_models, &sta_phase_model,
				&num_sta_phase_models);

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
	 * Return error status as determined by function, read_tt_tables(),
	 * if a serious problem was encountered.
	 */

	if (ierr != OK)
	    return (ierr);

	/*
	 * Read available SSSC tables!
	 */

	if (read_sssc (&model_descrip, num_models,
			&sta_phase_model, num_sta_phase_models) > 0)
	    return (SCerror1);

	/*
	 * Next, read available SRST tables.
	 */

	if (read_srst (model_descrip[0].dir_pathway, 
		       model_descrip[0].vmodel) != OK)
	    return (SRerror1);

	/*
	 * Finally, read any available test-site correction tables!
	 */

	if (read_ts_corr (model_descrip[0].dir_pathway, 
			  model_descrip[0].vmodel) != OK)
	    return (TSerror1);

	return (OK);
}

/* prin_deriv:	Principal radial and depth derivatives
 *		[0: dtdr;  1: dtdz; 2: d2tdr; 3: d2tdz]
 * tt_deriv:	Component travel-time derivatives
 *		[0: 1.0;  1: -dtdlon; 2: -dtdlat; 3: -dtdz]
 * slow_deriv:	Component slowness derivatives
 *		[0: 0.0;  1: -d2tdlon; 2: -d2tdlat; 3: -d2tdz]
 * az_deriv:	Component azimuth derivatives
 *		[0: 0.0;  1: -dazdlon; 2: -dazdlat; 3: 0.0]
 */
double
total_travel_time (Locator_params *locator_params, Site *sites, 
		   Ar_Info *ar_info, Bool extrapolate, 
		   Bool do_we_need_z_derivs, double ev_lat, 
		   double ev_lon, double ev_depth, double distance, 
		   double esaz, char *phase, int sta_index, int phase_index,
		   int spm_index, double data_std_err, double *prin_deriv, 
		   double *tt_deriv, double *slow_deriv, double *az_deriv, 
		   int *interp_code)
{
	Bool	tscor_found = FALSE;
	int	i;
/*	int	ierr;    JG removed  */
	double	trv_time = -1.0;
	double	src_dpnt_corr[3];
	double	me_factor, me_sssc;
	double	sta_lat, sta_lon;
	double	ev_geoc_co_lat, ev_geoc_lat;
	double	esr, cos_esr, sin_esr;
	double	az_corr, depth_corr;
	double	*LQ_trv_time = (double *) NULL ;	
        double  *LR_trv_time = (double *) NULL ;
	int     blocked;        /*Hydro phases only. JG*/


	static	double	dsdv[] = {	/* In degress */
		   0.,   10.,   20.,   30.,   40.,   50.,   60.,   70.,
		  80.,   90.,  100.,  180. };
	static  double  sdv[] = {      /* For a datum with an SRST correction */
		0.000, 0.444, 0.524, 0.274, 0.263, 0.372, 0.179, 0.202,
		0.137, 0.252, 0.682, 0.682 };
	static	double	period[] = { 20.0 };


	/*
	 * First, make sure depth is not negative.  If so, no valid travel
	 * time may be computed.
	 */

	if (ev_depth < 0.0)
	    return (-1.0);

	/*
	 * Check sta_index
	 */

	if( strcmp(sites[sta_index].sta, sta_pt[sta_index].sta) != 0)
	    fprintf (stderr, "libloc:total_travel_time: WARNING! Wrong sta_index!, %s station used instead of %s\n", sta_pt[sta_index].sta, sites[sta_index].sta);

	src_dpnt_corr[0] = 0.0;		/* Actual correction value */
	src_dpnt_corr[1] = 0.0;		/* Longitude derivative if SSSC found */
	src_dpnt_corr[2] = 0.0;		/* Latitude derivative if SSSC found */
	me_factor	 = 1.0;		/* SSSC modelling error factor */
	me_sssc		 = -1.0;	/* SSSC modelling error at source */

	tt_info = *ar_info;

	tt_info.meas_error = data_std_err;
	tt_info.model_plus_meas_error = data_std_err;
	tt_info.src_dpnt_corr_type = NO_SRC_DPNT_CORR;

	/*
	 * Grab travel time along with derivative information.  Handle path-
	 * dependent LR and LQ phases separately.
	 */

	if (phase_index == LR_PHASE_INDEX || phase_index == LQ_PHASE_INDEX)
	{
	    sta_lat = (double) sites[sta_index].lat;
	    sta_lon = (double) sites[sta_index].lon;
	    if (phase_index == LQ_PHASE_INDEX)
	    {
		LQ_trv_time = LP_trace_rays (ev_lat, ev_lon, sta_lat, sta_lon, 
				     LQ, period, 1);
                *interp_code = ERR;		
                if(LQ_trv_time != (double *) NULL)
		{
		    *interp_code = OK ;
		    trv_time = LQ_trv_time[0];
                    free(LQ_trv_time);		    
	        }
		strcpy (tt_info.vmodel, "path_dpnt_LQ");
		LQ_trv_time = (double *) NULL ;		
	    }
	    else
	    {
		LR_trv_time = LP_trace_rays (ev_lat, ev_lon, sta_lat, sta_lon, 
				     LR, period, 1);
                *interp_code = ERR;		
                if(LR_trv_time != (double *) NULL)
		{
		    *interp_code = OK ;				
		    trv_time = LR_trv_time[0];
                    free(LR_trv_time);
	        }
		strcpy (tt_info.vmodel, "path_dpnt_LR");
		LR_trv_time = (double *) NULL ;		
	    }

	    /* Get sub-set of derivatives pertainent to LP data */

	    esr	= esaz*DEG_TO_RAD;
	    sin_esr = sin(esr);
	    cos_esr = cos(esr);
	    depth_corr = DEG_TO_RAD*(RADIUS_EARTH-ev_depth);
	    az_corr = sin(distance*DEG_TO_RAD)*RADIUS_EARTH*DEG_TO_RAD;
	    if (az_corr == 0.0)
		az_corr = 0.0001;
	    for (i = 0; i < 4; i++)
	    {
		prin_deriv[i] = 0.0;
		tt_deriv[i]   = 0.0;
		slow_deriv[i] = 0.0;
		az_deriv[i]   = 0.0;
	    }

	    if (*interp_code != OK)
		return (-1.0);

	    prin_deriv[0] = trv_time/distance;		/* Average slowness */
	    az_deriv[1] = -cos_esr/az_corr;		/* Points East */
	    az_deriv[2] =  sin_esr/az_corr;		/* Points North */
	    tt_deriv[0] = 1.0;				/* Time */
	    tt_deriv[1] = -(prin_deriv[0]/depth_corr)*sin_esr;
	    tt_deriv[2] = -(prin_deriv[0]/depth_corr)*cos_esr;

	    tt_info.model_error	= 10.0;		/* For now, always 10 sec. */
	}


 
/* JG addition starts here. */
        else if ( phase_index==T_PHASE_INDEX || phase_index==H_O_PHASE_INDEX ||
		phase_index==I_PHASE_INDEX)
        {

            get_acoustic_tt(sites[sta_index].sta, ev_lat, ev_lon, &trv_time,
			&tt_info.model_error,&blocked);
	    if(phase_index==T_PHASE_INDEX)
		tt_info.model_error += (get_H_T_convert());
	    tt_info.tt_table_value = trv_time; /* No need for ellip/elev corrections*/
	    tt_info.total_travel_time = trv_time;
	    if (blocked>3)
	    	*interp_code = 20;
	    else
                *interp_code = 0;

            strcpy (tt_info.vmodel, (char *)(get_current_radial_2D_period_name(sites[sta_index].sta)));
 
            /* Get sub-set of derivatives pertainent to Hydro data */
 
            esr = esaz*DEG_TO_RAD;
            sin_esr = sin(esr);
            cos_esr = cos(esr);
            depth_corr = DEG_TO_RAD*(RADIUS_EARTH-ev_depth);
            az_corr = sin(distance*DEG_TO_RAD)*RADIUS_EARTH*DEG_TO_RAD;
            if (az_corr == 0.0)
                az_corr = 0.0001;
            for (i = 0; i < 4; i++)
            {
                prin_deriv[i] = 0.0;
                tt_deriv[i]   = 0.0;
                slow_deriv[i] = 0.0;
                az_deriv[i]   = 0.0;
            }
 
            /*prin_deriv[0] may change to ((1/1.48(km/s)) * KM_PER_DEG)  */
            prin_deriv[0] = trv_time/distance;          /* Average slowness */
            az_deriv[1] = -cos_esr/az_corr;             /* Points East */
            az_deriv[2] =  sin_esr/az_corr;             /* Points North */
            tt_deriv[0] = 1.0;                          /* Time */
            tt_deriv[1] = -(prin_deriv[0]/depth_corr)*sin_esr;
            tt_deriv[2] = -(prin_deriv[0]/depth_corr)*cos_esr;

            if (locator_params->dist_var_wgt)
            {
                if (tt_info.meas_error < 0.0)
                    tt_info.model_plus_meas_error = -1.0;
                else
                    tt_info.model_plus_meas_error =
                        sqrt (tt_info.model_error*tt_info.model_error +
                              tt_info.meas_error*tt_info.meas_error);
            }
        }
/* JG addition ends here. */
 



	else
	{
	    trv_time = 
	    trv_time_w_ellip_elev (extrapolate, do_we_need_z_derivs,
				   ev_lat, distance, ev_depth, esaz, phase, 
				   sites[sta_index].elev, phase_index,
				   spm_index, prin_deriv, interp_code);

	    if (trv_time < 0.0)
		return (-1.0);

	    /*
	     * Get any source-dependent corrections requested and found. 
	     * Let's start with test-site correction, if applicable,
	     * since this correction supercedes SSSC and SRST corrections.
	     */

	    if (locator_params->use_test_site_corr)
	    {
		if ((src_dpnt_corr[0] = 
		     get_ts_corr (locator_params->test_site_region, phase,
			     sites[sta_index].sta, &tscor_found)) == -999.0)
		{
		    /* Requested test-site region not found ! */
		    *interp_code = TSerror2;
		    return (-1.0);	
		}
		if (tscor_found)
		    tt_info.src_dpnt_corr_type = TEST_SITE_CORR;
	    }

	    /*
	     * Next, look for SSSC value if requested!
	     */

	    else if (locator_params->sssc_level > 0 && spm_index > -1)
	    {
		if (sta_phase_model[spm_index].sssc != (Sssc *) NULL)
		{
		    if (apply_sssc (ev_lat, ev_lon, sta_phase_model[spm_index], 
				    src_dpnt_corr, &me_factor, &me_sssc,
				    locator_params->sssc_level, locator_params->verbose))
			tt_info.src_dpnt_corr_type = SSSC_LEVEL_1_CORR;
		}
	    }

	    /*
	     * Finally, look for an SRST correction.  Here, 
	     * tt_info.model_error is the SRST data variance.  It's 
	     * inverse is the applied weight
	     */

	    else if (locator_params->use_srst)
	    {
		if (STREQ (phase, "P") || STREQ (phase, "PKP") || 
		    STREQ (phase, "PKPdf") || STREQ (phase, "PKPbc"))
		{
		    ev_geoc_co_lat = 
			lat_conv (ev_lat, TRUE, TRUE, FALSE, TRUE, FALSE);
		    ev_geoc_lat = 90.0-ev_geoc_co_lat*RAD_TO_DEG;
		    if (apply_srst (sites[sta_index].sta, ev_geoc_lat, 
				    ev_lon, ev_geoc_co_lat, ev_depth, 
				    &src_dpnt_corr[0], &tt_info.model_error))
		    {
			tt_info.src_dpnt_corr_type = SRST_CORR;
			if (locator_params->srst_var_wgt)
			{
			    for (i = 1; i < (signed int)NUM_ELEMENTS(dsdv); i++)
				if (distance <= dsdv[i])
				    break;
			    tt_info.model_error = 
				sdv[i] + ((distance-dsdv[i]) /
					 (dsdv[i-1]-dsdv[i]))*(sdv[i-1]-sdv[i]);
			}
		    }
		}
	    }

	    /*
	     * If user-specified variance weighting is requested, over-ride
	     * all other variance weighting.
	     */

	    if (locator_params->user_var_wgt > 0.0)
	    {
		tt_info.model_error = (double) locator_params->user_var_wgt;
		tt_info.model_plus_meas_error = tt_info.model_error;
	    }

	    if (tt_info.src_dpnt_corr_type > NO_SRC_DPNT_CORR)
	    {
		depth_corr = DEG_TO_RAD*(RADIUS_EARTH-ev_depth);
		src_dpnt_corr[1] /= depth_corr;
		src_dpnt_corr[2] /= depth_corr;
		trv_time += src_dpnt_corr[0];
	    }

	    tt_info.total_travel_time = trv_time;
	    tt_info.src_dpnt_corr = src_dpnt_corr[0];

	    /*
	     * Correct travel-time derivative relative to sum of all time
	     * corrections combined.
	     */

	    /*
	    tt_deriv_corr = (trv_time - tt_info.tt_table_value)/distance;
	    prin_deriv[0] += tt_deriv_corr;
	    printf ("horiz_slow: %8.4f  tt_deriv_corr: %8.4f\n",
		prin_deriv[0], tt_deriv_corr);
		*/

	    /*
	     * Get model error (tt_info.model_error) from travel-time tables
	     * if distance variance weighting has been requested.  Better yet,
	     * if a SSSC modeling error is available, then it takes precedance
	     * over all other modeling errors.
	     */

	    if (locator_params->dist_var_wgt)
	    {
		if (me_sssc > 0.0)
		    tt_info.model_error = me_sssc;
		else
		{
		    /* 
		     * The last two argument can be NULL, since when called from
		     * here, get_model_error is only getting 1-D models.
		     */
		    tt_info.model_error = 
			get_model_error (phase_index, distance, ev_depth, 0.0, NULL);
		    tt_info.model_error *= me_factor;
		}
		if (tt_info.meas_error < 0.0)
		    tt_info.model_plus_meas_error = -1.0;
		else
		    tt_info.model_plus_meas_error = 
			sqrt (tt_info.model_error*tt_info.model_error + 
			      tt_info.meas_error*tt_info.meas_error);
	    }

	    esr	= esaz*DEG_TO_RAD;
	    sin_esr = sin(esr);
	    cos_esr = cos(esr);
	    depth_corr = DEG_TO_RAD*(RADIUS_EARTH-ev_depth);
	    az_corr = sin(distance*DEG_TO_RAD)*RADIUS_EARTH*DEG_TO_RAD;
	    if (az_corr == 0.0)
		az_corr = 0.0001;

	    /*
	     * tt_deriv[] contains travel-time derivative information mapped 
	     * along the component time, latitude, longitude and depth axes 
	     * of deriv[].
	     */

	    tt_deriv[0] = 1.0;					/* Time */
	    tt_deriv[1] = -(prin_deriv[0]/depth_corr)*sin_esr;	/* Points E. */
	    tt_deriv[2] = -(prin_deriv[0]/depth_corr)*cos_esr;	/* Points N. */
	    tt_deriv[3] = -prin_deriv[1];			/* Points Up */

	    /*
	     * slow_deriv[] contains slowness derivative information mapped 
	     * along the component time, latitude, longitude and depth axes 
	     * of deriv[].
	     */

	    slow_deriv[0] = 0.0;				 /* Time */
	    slow_deriv[1] = -(prin_deriv[2]/depth_corr)*sin_esr; /* Points E. */
	    slow_deriv[2] = -(prin_deriv[2]/depth_corr)*cos_esr; /* Points N. */
	    slow_deriv[3] = -prin_deriv[3];			 /* Points Up */

	    /*
	     * az_deriv[] contains azimuth derivative information mapped 
	     * along the component time, latitude, longitude and depth axes 
	     * of deriv[].
	     */

	    az_deriv[0] = 0.0;			/* Time */
	    az_deriv[1] = -cos_esr/az_corr;	/* Points East */
	    az_deriv[2] =  sin_esr/az_corr;	/* Points North */
	    az_deriv[3] = 0.0;			/* Points Up */
	}

	*ar_info = tt_info;

	return (trv_time);
}


double
compute_ttime_w_corrs (Locator_params *locator_params, Site *sites, 
		       Bool extrapolate, double ev_lat, double ev_lon, 
		       double ev_depth, double distance, double esaz, 
		       char *phase, int sta_index, Ar_Info *ar_info,
		       double *horiz_slow) 
{

	Bool	do_we_need_z_derivs = FALSE;	/* We do no need depth deriv. */
	int	phase_index, spm_index;
	int	interp_code;
	double	trv_time = -1.0;
	double	prin_deriv[4], tt_deriv[4], slow_deriv[4], az_deriv[4];


	/*
	 * Initialize tt_info structure to N/A values.  Note that calls to
	 * function, total_travel_time(), have already been initialized,
	 * but since this function is typically called for computing 
	 * theoreticals, no guarantee is made that this has been initialized.
	 */

	tt_info = initialize_ar_info ();
	*ar_info = tt_info;

	/*
	 * Get phase and station/phase/model indexes given phase
	 */

	phase_index = get_tt_indexes (phase, sta_index, &spm_index);

	/*
	 * Grab travel time and horizontal slowness.
	 */

	trv_time = 
	total_travel_time (locator_params, sites, ar_info, extrapolate, 
			   do_we_need_z_derivs, ev_lat, ev_lon, ev_depth, 
			   distance, esaz, phase, sta_index, phase_index,
			   spm_index, 1.0, prin_deriv, tt_deriv, slow_deriv,
			   az_deriv, &interp_code);

	if (! extrapolate && interp_code != 0)
	    return (-1.0);

	*horiz_slow = prin_deriv[0];

	return (trv_time);
}

/* prin_deriv:Principal radial and depth derivatives
 *	[0: dtdr;  1: dtdz; 2: d2tdr; 3: d2tdz]
 */
double
trv_time_w_ellip_elev (Bool extrapolate, Bool do_we_need_z_derivs, 
			double ev_lat, double distance, double ev_depth, 
			double esaz, char *phase, double sta_elev, 
			int phase_index, int spm_index, 
			double *prin_deriv, int *interp_code)
{
	Bool	in_hole;
	int	fatal_err = 0;
	int	leg;
	float	value, x_1st_deriv, z_1st_deriv, x_2nd_deriv, z_2nd_deriv;
	double	bulk_static_sta_corr;
	double	el, elev_corr, ellip_corr;
	double	surf_vel;
	double	trv_time;

	static	double	save_ev_lat = 99.0;
	static	double	ev_geoc_co_lat = 0.0;


	tt_info.src_dpnt_corr_type = NO_SRC_DPNT_CORR;

	if (phase_index < 0)
	    return (-1.0);		/* Indicates phase index not found */

	if (phase_index > total_num_phases)
	{
	    fprintf (stderr, "trv_time_w_ellip_elev: phase_index is too large!!!\n");
	    return (-1.0);		/* Indicates phase index not found */
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
	interpolate_table_value (extrapolate, in_hole, do_we_need_z_derivs,
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


	/*
	 * prin_deriv[] contains distance/depth derivatives measured along 
	 * the principal distance/azimuth (radial) direction.
	 */

	prin_deriv[0] = (double)x_1st_deriv;   /* dtdr (horiz slow (sec/deg)) */
	prin_deriv[1] = (double)z_1st_deriv;   /* dtdz */
	prin_deriv[2] = (double)x_2nd_deriv;   /* d2tdr */
	prin_deriv[3] = (double)z_2nd_deriv;   /* d2tdz */


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
	    if (spm_index < 0)	/* Use default surface velocities */
	    {
		leg = last_leg (phase);
		if (leg == LAST_LEG_IS_P)
		    surf_vel = SURF_P_VEL;
		else if (leg == LAST_LEG_IS_S)
		    surf_vel = SURF_S_VEL;
		else
		    surf_vel = 0.0;
	    }
	    else		/* Get velocity from sta_phase_model */
	    {
		surf_vel = sta_phase_model[spm_index].sed_vel;
	    }

	    if (surf_vel > 0.0)
	    {
		el = surf_vel*KM_TO_DEG * prin_deriv[0];
		el = el*el;
		if (el > 1.0)
		    el = 1.0/el;
		el = 1.0 - el;
		el = sqrt(el);
		elev_corr = sta_elev*el/surf_vel;
	    }
	}

	/*
	 * Grab bulk static station correction term, if available!
	 */

	if (spm_index < 0)
	    bulk_static_sta_corr = 0.0;
	else
	    bulk_static_sta_corr = sta_phase_model[spm_index].bulk_sta_corr;

	trv_time = value + elev_corr + ellip_corr + bulk_static_sta_corr;

	/*
	 * Populate tt_info structure here
	 */

	tt_info.tt_table_value		= (double) value;
	tt_info.ellip_corr		= ellip_corr;
	tt_info.elev_corr		= elev_corr;
	tt_info.bulk_static_sta_corr	= bulk_static_sta_corr;

	strcpy (tt_info.vmodel, tt_table[phase_index].vmodel);

	return (trv_time);
}

char *
get_vmodel()
{
    return(tt_info.vmodel);
}

/* trv_time:Input travel-time (sec.) at Earth's surface
 */
double
crude_but_quick_dist (int phase_index, double trv_time)
{

	int	i;
	double	tmp;
	double	surf_dist;	/* Return crude distance estimate at surface */

	if (tt_table[phase_index].trv_time[0][0] > trv_time)
	    return (-1.0);	/* Input travel-time < lower distance bound */

	for (i = 1; i < tt_table[phase_index].num_dists; i++)
	{
	    if (tt_table[phase_index].trv_time[0][i] > trv_time)
	    {
		tmp = (trv_time - tt_table[phase_index].trv_time[0][i-1])/
			      (tt_table[phase_index].trv_time[0][i] -
			       tt_table[phase_index].trv_time[0][i-1]);
		surf_dist = tt_table[phase_index].dist_samples[i-1] +
			    tmp * (tt_table[phase_index].trv_time[0][i] -
				   tt_table[phase_index].trv_time[0][i-1]);
		return (surf_dist);
	    }
	}
	return (-1.0);		/* Input travel-time > upper distance bound */
}

/* distance:	Input distance (deg.)
 * depth:	Input depth (km.) below Earth's surface
 */
double
crude_tt_w_only_phase (char *phase, double distance, double depth)
{
	int	i;
	int	phase_index = -1;

	if (!phase || !*phase)
	    return (-1.0);

	for (i = 0; i < num_phases; i++)
	    if (STREQ (phase, tt_table[i].phase) && tt_table[i].num_dists > 0)
		phase_index = i;

	if (phase_index < 0)
	    return (-1.0);
	else
	    return (crude_but_quick_trv_time (phase_index, distance, depth));
}

/* distance:	Input distance (deg.)
 * depth:	Input depth (km.) below Earth's surface
 */
double
crude_but_quick_trv_time (int phase_index, double distance, double depth)
{

	Bool	max_depth_index_found = FALSE;
	int	i, j;
	double	tmp;
	double	depth_diff;
	double	tt_at_bottom, tt_at_top;
	double	trv_time;		/* Return crude T-T estimate */


	/*
	 * Make sure phase index is valid.  Also make sure depth is not 
	 * negative and number of depth samples is > 0
	 */

	if (phase_index < 0)
	    return (-1.0);
	if (depth < 0.0 || tt_table[phase_index].num_depths < 1)
	    return (-1.0);

	/*
	 * This is a quick-and-dirty routine for return a crude estimate 
	 * of travel-time (sec.) given and input distance (in arc deg.)
	 * and depth (in km.).  Bi-linear interpolation is used, so user's 
	 * should not assume that these times are better than about +/-2 sec. 
	 * 
	 * For radial_2D phases, 75.13 sec/deg is used. JG
	 */

	if (phase_index == LR_PHASE_INDEX || phase_index == LQ_PHASE_INDEX)
	   return (-1.0);

	if (phase_index == H_O_PHASE_INDEX || phase_index == T_PHASE_INDEX)
	   return (75.13 * distance);

	if (phase_index == I_PHASE_INDEX)
	   return (350.0 * distance);

	if (tt_table[phase_index].dist_samples[0] > distance)
	    return (-1.0);	/* Input distance < lower distance bound */

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


Ar_Info
initialize_ar_info (void)
{
	Ar_Info	Default_Ar_Info;

	Default_Ar_Info.time_error_code		= 0;
	Default_Ar_Info.az_error_code		= 0;
	Default_Ar_Info.slow_error_code		= 0;
	Default_Ar_Info.src_dpnt_corr_type	= NO_SRC_DPNT_CORR;
	Default_Ar_Info.total_travel_time	= -1.0;
	Default_Ar_Info.tt_table_value		= -1.0;
	Default_Ar_Info.ellip_corr		= 0.0;
	Default_Ar_Info.elev_corr		= 0.0;
	Default_Ar_Info.bulk_static_sta_corr	= 0.0;
	Default_Ar_Info.src_dpnt_corr		= 0.0;
	Default_Ar_Info.model_error		= -1.0;
	Default_Ar_Info.meas_error		= -1.0;
	Default_Ar_Info.model_plus_meas_error	= -1.0;
	Default_Ar_Info.time_import		= -1.0;
	Default_Ar_Info.az_import		= -1.0;
	Default_Ar_Info.slow_import		= -1.0;
	Default_Ar_Info.slow_vec_res		= -999.0;
	strcpy (Default_Ar_Info.vmodel, "-");
	strcpy (Default_Ar_Info.lddate, "-");

	return (Default_Ar_Info);
}


Locator_params
initialize_loc_params (void)
{
	Locator_params	Default_Loc_Params;

	Default_Loc_Params.max_iterations	= 20;
	Default_Loc_Params.num_iter		= 0;
	Default_Loc_Params.refill_if_loc_fails	= FALSE;
	Default_Loc_Params.fix_depth		= TRUE;
	Default_Loc_Params.fix_origin_time	= FALSE;
	Default_Loc_Params.fix_lat_lon		= FALSE;
	Default_Loc_Params.use_location		= TRUE;
	Default_Loc_Params.use_elev_corr	= TRUE;
	Default_Loc_Params.ellip_cor_type	= 2;
	Default_Loc_Params.sssc_level		= 0;
	Default_Loc_Params.use_test_site_corr	= FALSE;
	Default_Loc_Params.use_srst		= FALSE;
	Default_Loc_Params.srst_var_wgt		= FALSE;
	Default_Loc_Params.use_only_sta_w_corr	= FALSE;
	Default_Loc_Params.dist_var_wgt		= FALSE;
	Default_Loc_Params.ignore_large_res	= FALSE;
	Default_Loc_Params.num_dof		= 99999;
	Default_Loc_Params.est_std_error	= 1.0;
	Default_Loc_Params.conf_level		= 0.90;
	Default_Loc_Params.damp			= -1.0;
	Default_Loc_Params.user_var_wgt		= -1.0;
	Default_Loc_Params.lat_init		= -999.0;
	Default_Loc_Params.lon_init		= -999.0;
	Default_Loc_Params.depth_init		= -999.0;
	Default_Loc_Params.origin_time_init	= 0.0;
	Default_Loc_Params.large_res_mult	= 3.0;
	Default_Loc_Params.verbose		= '2';
	memset(Default_Loc_Params.test_site_region, 0, sizeof(Default_Loc_Params.test_site_region));
	Default_Loc_Params.outfile_name		= (char *) NULL;
	Default_Loc_Params.prefix		= (char *) NULL;

	return (Default_Loc_Params);
}


int
get_tt_indexes (char *phase, int sta_index, int *spm_index)
{
	int		i, j;
	int		leg;
	Phz_Pt		*phz;
	List_of_Phases	*list_phz;

	*spm_index = -1;

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

 
/*JG addition begins here*/
        if (STREQ (phase, "H") || STREQ (phase, "O"))
                if(station_in_radial_2D_tables(sta_pt[sta_index].sta))
                        return (H_O_PHASE_INDEX);
        if (STREQ (phase, "T") )
                if(station_in_radial_2D_tables(sta_pt[sta_index].sta))
                        return (T_PHASE_INDEX);
        if (STREQ (phase, "I") )
                if(station_in_radial_2D_tables(sta_pt[sta_index].sta))
                        return (I_PHASE_INDEX);
/*JG addition ends here*/
 





	phz = sta_pt[sta_index].phase_ptr;
	if (phz != (Phz_Pt *) NULL)
	{
	    /*
	     * If specific station/phase information has been specified set
	     * spm_index and phase index (i).
	     */

	    while (phz != (Phz_Pt *) NULL)
	    {
		if (STREQ (phase, phz->phase))
		{
		    *spm_index = phz->spm_index;
		    i = sta_phase_model[*spm_index].phase_index;
		    if (i >= 0 && tt_table[i].num_dists > 0)
		    	return (i);
		    else
			return (ERR);
		}
		phz = phz->next;
	    }

	    /*
	     * Check to see if any default station-specific information 
	     * exists.  If so, then look to see if a default P or S spm_index 
	     * exists.  If not, simply use default travel-time table.  This
	     * is a bit more complex since no unique phase_index can be
	     * specified in structure, sta_phase_model.
	     */

	    leg = last_leg (phase);
	    if (leg == LAST_LEG_IS_P)
	    {
		phz = sta_pt[sta_index].phase_ptr;
		while (phz != (Phz_Pt *) NULL)
		{
		    if (STREQ (phz->phase, "*P"))
		    {
			j = sta_phase_model[phz->spm_index].vel_index;
			if (j == 0)
			{
			    *spm_index = phz->spm_index;
			    break;
			}
			list_phz = model_descrip[j].list_of_phases;
			while (list_phz != (List_of_Phases *) NULL)
			{
			    if (STREQ (phase, list_phz->phase))
			    {
				*spm_index = phz->spm_index;
				return (list_phz->phase_index);
			    }
			    list_phz = list_phz->next;
			}
	    	    }
		    phz = phz->next;
		}
	    }
	    else if (leg == LAST_LEG_IS_S)
	    {
		phz = sta_pt[sta_index].phase_ptr;
		while (phz != (Phz_Pt *) NULL)
		{
		    if (STREQ (phz->phase, "*S"))
		    {
			j = sta_phase_model[phz->spm_index].vel_index;
			if (j == 0)
			{
			    *spm_index = phz->spm_index;
			    break;
			}
			list_phz = model_descrip[j].list_of_phases;
			while (list_phz != (List_of_Phases *) NULL)
			{
			    if (STREQ (phase, list_phz->phase))
			    {
				*spm_index = phz->spm_index;
				return (list_phz->phase_index);
			    }
			    list_phz = list_phz->next;
			}
	    	    }
		    phz = phz->next;
		}
	    }
	}

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


int
get_default_phase_index (char *phase)
{
	int	i;


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

	for (i = 0; i < num_phases; i++)
	    if (STREQ (phase, tt_table[i].phase) && tt_table[i].num_dists > 0)
		return (i);

	return (ERR);
}


double
get_model_error (int phase_index, double delta, double depth, double seaz,
		char *sta)
{

	Bool	max_depth_index_found = FALSE;
	int	i, k, m, n;
	int	num_dists, num_depths;
	double	ratio, zratio=0.;
	double	top, bot;
	double	model_error = 1.0;


	/*
	 * Make sure phase index is valid.  Also make sure depth is not 
	 * negative 
	 */

	if (phase_index < 0 || depth < 0.0)
	    return (1.0);

	/*
	 * First, check to see if this is a long-period phase.  If so, just
	 * return a 30 sec. modelling error for good measure.
	 */

	if (phase_index == LR_PHASE_INDEX || phase_index == LQ_PHASE_INDEX)
	   return (30.0);

	/*
	 * JG. Return modeling error for radial_2D stations.
	 */

	if ( phase_index == H_O_PHASE_INDEX || phase_index == I_PHASE_INDEX)
	    return get_2D_model_error(sta, delta, seaz);

	if ( phase_index == T_PHASE_INDEX )
	    return ( get_2D_model_error(sta, delta, seaz) +
			get_H_T_convert());

	/*
	 * If no modelling error is specified for this phase, just return
	 * the default error, 1.0 sec.
	 */

	if (tt_table[phase_index].model_error == (Model_Error *) NULL)
	    return (1.0);

	k = phase_index;
	num_dists  = tt_table[k].model_error->num_dists;
	num_depths = tt_table[k].model_error->num_depths;

	if (num_depths < 1)
	    return (1.0);

	if (num_depths == 1)
	{
	    if (num_dists == 1)
	    {
		/*
		 * Only a single-valued modelling error exists for this
		 * phase (bulk_var).
		 */

		 model_error = (double) tt_table[k].model_error->bulk_var;
	    }
	    else
	    {
		/*
		 * Only distance-dependent modelling errors exists for 
		 * this phase. Obtain modelling error via simple linear 
		 * interpolation.  If out of valid distance range,
		 * simply return modelling error from nearest end point.
		 */

		n = num_dists - 1;
		if (delta < tt_table[k].model_error->dist_samples[0])
		    model_error = (double) tt_table[k].model_error->dist_var[0];
		else if (delta > tt_table[k].model_error->dist_samples[n])
		    model_error = (double) tt_table[k].model_error->dist_var[n];
		else
		{
		    for (i = 1, n = 1; i < num_dists; i++)
		    {
			if (delta < tt_table[k].model_error->dist_samples[i])
			{
			    n = i;	/* High end */
			    break;
			}
		    }
		    ratio = (delta - tt_table[k].model_error->dist_samples[n-1])
			    / (tt_table[k].model_error->dist_samples[n] -
			       tt_table[k].model_error->dist_samples[n-1]);
		    model_error = 
			tt_table[k].model_error->dist_var[n-1] +
			ratio * (tt_table[k].model_error->dist_var[n] - 
				 tt_table[k].model_error->dist_var[n-1]);
		}
	    }
	}
	else
	{
	    /*
	     * Complete distance/depth-dependent modelling errors exist
	     * for this phase. Obtain modelling error via simple bi-linear 
	     * interpolation.  If out of valid distance or depth range,
	     * simply return modelling error from nearest end point.
	     */

	    for (i = 1, m = 1; i < num_depths; i++)
	    {
		if (tt_table[k].model_error->depth_samples[i] > depth)
		{
		    m = i;		/* m: Deepest depth index */
		    zratio = (depth - 
			      tt_table[k].model_error->depth_samples[i-1]) /
			     (tt_table[k].model_error->depth_samples[i] -
			      tt_table[k].model_error->depth_samples[i-1]);
		    max_depth_index_found = TRUE;
		    break;
		}
	    }
	    if (!max_depth_index_found)
		m = num_depths - 1;

	    n = num_dists - 1;
	    if (delta < tt_table[k].model_error->dist_samples[0])
	    {
		if (max_depth_index_found)
		    model_error = 
			tt_table[k].model_error->dist_depth_var[m-1][0] +
			zratio * 
			(tt_table[k].model_error->dist_depth_var[m][0] -
			 tt_table[k].model_error->dist_depth_var[m-1][0]);
		else
		    model_error = tt_table[k].model_error->dist_depth_var[m][0];
	    }
	    else if (delta > tt_table[k].model_error->dist_samples[n])
	    {
		if (max_depth_index_found)
		    model_error = 
			tt_table[k].model_error->dist_depth_var[m-1][n] +
			zratio * 
			(tt_table[k].model_error->dist_depth_var[m][n] -
			 tt_table[k].model_error->dist_depth_var[m-1][n]);
		else
		    model_error = tt_table[k].model_error->dist_depth_var[m][n];
	    }
	    else if (!max_depth_index_found)
	    {
		for (i = 1; i < num_dists; i++)
		{
		    if (delta < tt_table[k].model_error->dist_samples[i])
		    {
			n = i;
			break;
		    }
		}
		model_error = 
		    (double) tt_table[k].model_error->dist_depth_var[m][n];
	    }
	    else
	    {
		for (i = 1; i < num_dists; i++)
		{
		    if (delta < tt_table[k].model_error->dist_samples[i])
		    {
			ratio = (delta - 
				 tt_table[k].model_error->dist_samples[i-1]) /
				(tt_table[k].model_error->dist_samples[i] -
				 tt_table[k].model_error->dist_samples[i-1]);
			bot = tt_table[k].model_error->dist_depth_var[m][i-1] +
			      ratio *
			      (tt_table[k].model_error->dist_depth_var[m][i] -
			       tt_table[k].model_error->dist_depth_var[m][i-1]);
			top = tt_table[k].model_error->dist_depth_var[m-1][i-1] +
			      ratio *
			      (tt_table[k].model_error->dist_depth_var[m-1][i] -
			       tt_table[k].model_error->dist_depth_var[m-1][i-1]);
			model_error = top + zratio * (bot - top);
			break;
		    }
		}
	    }
	}

	return (model_error);
}


int
set_sta_pt (Site *site, int num_sites)
{
	Bool	refill_sta_pt = FALSE;
	int	i, j;

	Phz_Pt	*phz, *cur, *prev, *next;

	static	int	save_num_sites = -1;
	static	char	**save_site_list = (char **) NULL;


	/*
	 * First, make sure an input site table exists!
	 */

	if (site == (Site *) NULL || num_sites == 0)
	{
	    fprintf (stderr, "\nERROR: set_sta_pt: site table not specified or empty!\n");
	    fprintf (stderr, "       Function, set_sta_pt(), cannot be called until site table is avaiable!\n");
	    return (-2);
	}

	if (sta_pt != (Sta_Pt *) NULL)
	{
	    if (num_sites != save_num_sites)
		    refill_sta_pt = TRUE;
	    else
	    {
		for (i = 0; i < num_sites; i++)
		{
		    if (strcmp (site[i].sta, save_site_list[i]))
		    {
			refill_sta_pt = TRUE;
			break;
		    }
		}
	    }
	    if (!refill_sta_pt)
		return (OK);
	}

	/*
	 * If we need to re-allocate new memory, first free up previous
	 * arrays and structures (refill_sta_pt == TRUE).
	 */

	if (refill_sta_pt)
	{
	    for (i = 0; i < save_num_sites; i++)
	    {
		UFREE (save_site_list[i]);
		phz = sta_pt[i].phase_ptr;
		while (phz != (Phz_Pt *) NULL)
		{
		    next = phz->next;
		    UFREE (phz);
		    phz = next;
		}
	    }
	    UFREE (save_site_list);
	    UFREE (sta_pt);
	    sta_pt = (Sta_Pt *) NULL;
	}

	/*
	 * Allocate memory for sta_pt and phase_ptr structures as well as
	 * save_site_list array based on input site table.
	 */

	if( sta_pt == (Sta_Pt *)NULL )
	{
	    sta_pt = UALLOC (Sta_Pt, num_sites);
	    save_site_list = UALLOC (char *, num_sites);
	    for (i = 0; i < num_sites; i++)
	    {
		save_site_list[i] = UALLOC (char, 7);
		sta_pt[i].phase_ptr = (Phz_Pt *) NULL;
	    }
	}
	for (i = 0; i < num_sites; i++)
	{
	    strcpy (save_site_list[i], site[i].sta);
	    sta_pt[i].phase_ptr = (Phz_Pt *) NULL;
	}
	save_num_sites = num_sites;
	num_stations = save_num_sites;
	/*
	 * Now let's fill sta_pt and attached phz_pt structures based on
	 * the current contents of the station-specific phase/model info
	 * contained within structure, sta_phase_model.  We will do this,
	 * one station at-a-time, until all phases have been processed
	 * (i.e., added to sta_pt link list of phase pointer (phase_ptr)).
	 * If no phases are encountered for a given station, the sta_pt
	 * will contain a NULL pointer to Phz_Pt.  Do not include stations
	 * from sta_phase_model not contained in site table.
	 */

	for (i = 0; i < num_sites; i++)
	{
	    strcpy (sta_pt[i].sta, site[i].sta); /* JG */
	    for (j = 0; j < num_sta_phase_models; j++)
	    {
		if (STREQ (site[i].sta, sta_phase_model[j].sta))
		{
		    phz = sta_pt[i].phase_ptr;
		    if (phz == (Phz_Pt *) NULL)
		    {
			if ((sta_pt[i].phase_ptr = (Phz_Pt *)
				calloc (1, sizeof (Phz_Pt))) == NULL)
			{
			    fprintf (stderr, "set_sta_pt: calloc error\n");
			    return (ERR);
			}
			phz = sta_pt[i].phase_ptr;
		    }
		    else
		    {
			if ((cur = (Phz_Pt *)
				calloc (1, sizeof (Phz_Pt))) == NULL)
			{
			    fprintf (stderr, "set_sta_pt: calloc error\n");
			    return (ERR);
			}
			prev = phz;
			while (phz != (Phz_Pt *) NULL)
			{
			    prev = phz;
			    phz = phz->next;
			}
			prev->next = cur;
			phz = cur;
		    }
		    strcpy (phz->phase, sta_phase_model[j].phase);
		    phz->spm_index = j;
		    phz->next = NULL;
		}
	    }
	}

	return (OK);
}


int
get_sta_index (char *sta)
{
	int	i;
	int	sta_index = -1;

	for (i = 0; i < num_stations; i++)
	{
	    if( strcmp(sta, sta_pt[i].sta) == 0)
	    {
		sta_index = i;
		break;
	    }
	}

	if( sta_index == -1)
	{
	    fprintf (stderr, "get_sta_index: Could not find sta_index for station %s\n", sta);
	    sta_index = 0;
	}

	return (sta_index);
}
