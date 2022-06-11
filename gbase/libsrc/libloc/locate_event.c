
/*
 * Copyright (c) 1990-2003 Science Applications International Corporation.
 *

 * NAME
 *	locate_event -- Determine a hypocentral event location

 * FILE
 *	locate_event.c

 * SYNOPSIS
 *	int
 *	locate_event (sites, num_sites, arrival, assoc, origin, origerr, 
 *		      locator_params, ar_info, num_obs)
 *	Site	*sites;			(i) Station structure
 *	int	num_sites;		(i) Number of stations in sites table
 *	Arrival	*arrival;		(i) Standard CSS DB3.0 arrival table
 *	Assoc	*assoc;			(i/o) Standard CSS DB3.0 assoc table
 *	Origin	*origin;		(i/o) Standard CSS DB3.0 origin table
 *	Origerr	*origerr;		(i/o) Standard CSS DB3.0 origerr table
 *	Locator_params *locator_params; (i) Locator parameter info structure
 *	Ar_Info *ar_info;		(o) Arrival-based location info
 *	int	num_obs;		(i) Number of arrival/assoc records

 * DESCRIPTION
 *	Function.  This routine serves as the interface for calculating 
 *	single event hypocentral locations from the applications: ARS, 
 *	EvLoc, ESAL, GA, StaPro, LocSAT, WaveExpert and Beamer.

 * DIAGNOSTICS
 *	See *locator_error_table[] character string in file, loc_error_msg.c,
 *	for global-type errors and ar_info[] structure for local errors 
 *	(i.e., errors found in individual datum's).

 * FILES
 *	Travel-time table information is read by function, read_tt_tables(),
 *	as called by function, setup_tt_tables(), which provides memory
 *	management.

 * NOTES
 *	Since the arrival table is only used as input, we can correct for
 *	azimuth and slowness within the arrival attributes, azimuth and
 *	slowness, locally.  We will also added the modeling errors to the
 *	delaz and delslo fields.

 * SEE ALSO
 *	user_locate() function calls this routine in ARS.  main() calls 
 *	this routine in LocSAT and EvLoc.  Users need to make sure travel-
 *	time tables have been read via function, setup_tttables(), prior
 *	to invoking this function.

 * AUTHORS
 *	18 Aug 1992 (Walter Nagy)
 *	    Replaced FORTRAN subroutine call, locsat0, with this new C code.
 *	    Code structure was dramatically upgraded.  Exploits new C 
 *	    function calls to best_guess(), compute_hypo() and 
 *	    print_loc_results().
 *	 6 Oct 1993 (Walter Nagy)
 *	    Implemented code, namely, compute_ttime_w_corrs(), to apply 
 *	    source-dependent travel-time corrections.  This was primarily
 *	    needed by the ARS function, Align and Compute Theoreticals
 *	 4 Mar 1994 (Walter Nagy)
 *	    setup_sites() is no longer needed here.
 *	 4 Jul 1994 (Walter Nagy)
 *	    Major change.  All functions (re)moved except locate_event().
 *	    New travel-time handling facilities implemented using new
 *	    tt_table structure (see include file, tt_table.h).
 *	 9 Jan 1996 (Walter Nagy)
 *	    network argument no longer needed in call to locate_event().
 *	    Also, substituted locator_errors structure argument for new 
 *	    ar_info structure which contains contents of locator_errors
 *	    plus some additional arrival-based location information.
 *	26 Jan 1998 (Jerry Guern)
 *	    Y2K remediation.
 *	02 Jul 2001 (Doug Brumbaugh)
 *	    Fixed CMRva00813 - the original Assoc structure was being used
 *	    to decrement the number of defining data, when the working Assoc
 *	    structure should have been used.
 *	14 Nov 2003 (Walter Nagy)
 *	    Added station/phase-dependent azimuth/slowness computation
 *	    output, including SASCs, as requested in IDC CR P1 (AWST).
 */


#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "libloc.h"
#include "locp.h"
#include "libgeog.h"
#include "libtime.h"
#include "origin_Astructs.h"
#include "loc_defs.h"
#include "ar_sasc.h"


#define	VALID_TIME(x)	((x) > -9999999999.000)

static	Bool	first_call = TRUE;

Locator_info	*locator_info;


int
locate_event (Site *sites, int num_sites, Arrival *arrival, Assoc *assoc, 
	      Origin *origin, Origerr *origerr, Locator_params *locator_params,
	      Ar_Info *ar_info, int num_obs)
{

	int	i, ierr, n, ndef, ndp;
	int	verbose;
	int	num_data	  = 0,
		num_def_data	  = 0,
		num_time_def_data = 0;

	double	time_offset, torg;

	Origin	*w_origin = NULL;
	Origerr	*w_origerr = NULL;
	Arrival	*w_arrival = NULL;
	Assoc	*w_assoc = NULL;
	char str[100];
	FILE	*ofp = (FILE *) NULL;
	Origin Na_Origin_rec = Na_Origin_Init;


	/* Check input */

	if ((num_obs == 0) || (! arrival))
	{
	    fprintf (stderr, "\nEvID: %ld  In_OrID: %ld\n", 
				origin->evid, origin->orid);
	    fprintf (stderr, "%s\n", loc_error_msg (GLerror9));
	    return (GLerror9);
	}
	if (! assoc)
	{
	    fprintf (stderr, "\nEvID: %ld  In_OrID: %ld\n", 
				origin->evid, origin->orid);
	    fprintf (stderr, "%s\n", loc_error_msg (GLerror10));
	    return (GLerror10);
	}
	else if (! origin)
	{
	    fprintf (stderr, "%s\n", loc_error_msg (GLerror11));
	    return (GLerror11);
	}
	else if (! origerr)
	{
	    fprintf (stderr, "\nEvID: %ld  In_OrID: %ld\n", 
				origin->evid, origin->orid);
	    fprintf (stderr, "%s\n", loc_error_msg (GLerror12));
	    return (GLerror12);
	}

	/* Check alignment of arrival/assoc pointers */

	for (i = 0; i < num_obs; i++)
	    if (arrival[i].arid != assoc[i].arid)
	    {
		fprintf (stderr, "\nEvID: %ld  In_OrID: %ld  ArID: %ld\n", 
				origin->evid, origin->orid, arrival[i].arid);
		fprintf (stderr, "%s\n", loc_error_msg (GLerror13));
		return (GLerror13);
	    }
	
	/*
	 * Make sure to initialize SRST region number to NULL value (-1).
	 */

	set_srst_region_number ();

	/*
	 * Let's allocate temporary storage for local working structures, 
	 * w_origin, w_origerr, w_assoc and w_arrival.  Then, copy input 
	 * structures into working structures.
	 */

	w_origin  = UALLOCA (Origin, 1);
	w_origerr = UALLOCA (Origerr, 1);
	w_arrival = UALLOCA (Arrival, num_obs);
	w_assoc   = UALLOCA (Assoc, num_obs);

	MCOPY (w_origin, origin, sizeof (Origin));
	MCOPY (w_origerr, origerr, sizeof (Origerr));
	for (i = 0; i < num_obs; i++)
	{
	    MCOPY (&w_arrival[i], &arrival[i], sizeof (Arrival));
	    MCOPY (&w_assoc[i], &assoc[i], sizeof (Assoc));
	}

	/* Desired level of verbose output */

	if (locator_params->verbose == '4' || locator_params->verbose == 'y' ||
	    locator_params->verbose == 't')
	    verbose = 4;	/* All output */
	else if (locator_params->verbose == '3')
	    verbose = 3;	/* Do not output STAS and intermediate steps */
	else if (locator_params->verbose == '2')
	    verbose = 2;	/* Only output final location with arrivals */
	else if (locator_params->verbose == '1')
	    verbose = 1;	/* Only output final location */
	else
	    verbose = 0;	/* No output */

	/*
	 * Use current lat/lon as initial guess.  If locator_params structure
	 * contains default initial values (or those just outside the normal
	 * range of lat/lon values), then use origin structure, else use
	 * locator_params structure to override.  User must fill both
	 * locator_params->lat_init and locator_params->lon_init, if they
	 * wish to read from the locator_params structure.  Finally, make sure
	 * event depth is not permitted above Earth's surface.
	 */

	if (locator_params->fix_lat_lon)
	{
	    w_origin->lat = locator_params->lat_init;
	    w_origin->lon = locator_params->lon_init;
	}
	if (locator_params->depth_init >= 0.0) 
	    w_origin->depth = locator_params->depth_init;
	if (w_origin->depth < 0.0)
	    w_origin->depth = 0.0;

	/*
	 * If the use_location flag equals zero than set the lat and lon
	 * init values to something the locator knows to ignore (-999.0)
	 */

	if (! locator_params->use_location)
	{
	    w_origin->lat = Na_Origin_rec.lat;
	    w_origin->lon = Na_Origin_rec.lon;
	}
	
	for (i = 0; (! VALID_TIME(arrival[i].time)) && i < num_obs; i++);

	if (i < num_obs)				/* Find offset time */
	    time_offset = arrival[i].time;
	else
	    time_offset = 0.0;

	for (i = 0; i < num_obs; i++)
	    if (VALID_TIME(arrival[i].time))
		time_offset = MIN(time_offset, arrival[i].time);

	set_epoch_travel_time(time_offset);


	/*
	 * If origin time is to be constrained, then we MUST insist that 
	 * relative time before the first arrival (torg) be the difference 
	 * between the user-fixed origin time and first arrival.  Otherwise,
	 * torg is simply the difference between the input origin time and
	 * the first arrival.  If the origin->time field contains an N/A
	 * value, then set torg = 0.0 and orthogonalize the origin time 
	 * term in function, compute_hypo() .
	 */

	if (locator_params->fix_origin_time)
	    torg = locator_params->origin_time_init - time_offset;
	else if (w_origin->time == Na_Origin_rec.time) 
	    torg = 0.0;
	else
	    torg = w_origin->time - time_offset;

	/*
	 * Extract the observations from the arrival/assoc structures.
         * Assume that these are stored in same order in both structures.
	 * num_obs is the number of arrival records sent to the locator.
	 * Each arrival record may have up to 3 datums (these individually
	 * incrementing the variable, num_data).  The following loop,
	 * therefore, looks for valid travel-time, azimuth and slowness 
	 * data, incrementing num_data accordingly.  The number of defining
	 * data, indicated by variable, num_def_data, is also determined
	 * here.  Depth phases are automatically made non-defining for
	 * fixed (constrained) depth locations.  Note that only the working 
	 * structure, w_assoc, is modified in this way. 
	 */
	
	for (i = 0; i < num_obs; i++)
	{
	    ar_info[i].orid = w_origin->orid;
	    ar_info[i].arid = arrival[i].arid;
	    w_arrival[i].time = arrival[i].time-time_offset;

	    /*
	     * Don't allow depth phases to be used when restrained (fixed)
	     * depth location is requested.
	     */

	    if (locator_params->fix_depth &&
	        (!strncmp (w_assoc[i].phase, "p", 1) ||
	         !strncmp (w_assoc[i].phase, "s", 1)))
	    {
		strcpy (w_assoc[i].timedef, "n");
		strcpy (w_assoc[i].azdef, "n");	/* May re-evaluate in future */
		strcpy (w_assoc[i].slodef, "n");
		continue;
	    }
	    if (STREQ (w_assoc[i].timedef, "d"))
	    {
		++num_def_data;
		++num_time_def_data;
	    }
	    ++num_data;

	    if (VALID_SEAZ(arrival[i].azimuth))
	    {
		if (STREQ (w_assoc[i].azdef, "d"))
		    ++num_def_data;
		++num_data;
	    }
	    else
	    {
		strcpy (assoc[i].azdef, "n");
		strcpy (w_assoc[i].azdef, "n");
	    }

	    if (VALID_SLOW(arrival[i].slow))
	    {
		if (STREQ (w_assoc[i].slodef, "d"))
		    ++num_def_data;
		++num_data;
	    }
	    else
	    {
		strcpy (assoc[i].slodef, "n");
		strcpy (w_assoc[i].slodef, "n");
	    }
	}		


	/*
	 * If not already done, allocate memory for typedef structure, 
	 * locator_info.  Always initialize its local variables.
	 */

	if (first_call)
	{
	    locator_info = UALLOC (Locator_info, 1);
	    first_call = FALSE;
	}

	locator_info->sta_index   = UALLOCA (int, num_obs);
	locator_info->phase_index = UALLOCA (int, num_obs);
	locator_info->spm_index   = UALLOCA (int, num_obs);

	locator_info->nd_used = 0;
	locator_info->tot_ndf = 0;
	locator_info->azimuthal_gap = -1.0;
	locator_info->sigma_hat	= -1.0;
	locator_info->snssd = -1.0;
	locator_info->sum_sqrd_res = -1.0;
	locator_info->applied_damping = -1.0;
	locator_info->rank = -1.0;
	for (n = 0; n < num_obs; n++)
	{
	    locator_info->sta_index[n]   = -1;
	    locator_info->phase_index[n] = -1;
	    locator_info->spm_index[n]   = -1;
	}

	/*
	 * Initialize ar_info array to N/A values, except for arid.
	 */

	for (n = 0; n < num_obs; n++)
	{
	    ar_info[n] = initialize_ar_info ();
	    ar_info[n].orid = w_origin->orid;
	    ar_info[n].arid = arrival[n].arid;
	}

	/*
	 * Determine how many parameters to solve for.  
	 */

	locator_info->num_params = 0;
	if (! locator_params->fix_lat_lon) 
	    locator_info->num_params = 2;
	if (! locator_params->fix_origin_time) 
	    ++locator_info->num_params;
	if (! locator_params->fix_depth) 
	    ++locator_info->num_params;

	/*
	 * Check for valid data and load station, phase and data 
	 * type pointers
	 */

	for (n = 0; n < num_obs; n++)
	{
	    /* Check that the datum's station is valid */

	    for (i = 0; i < num_sites; i++)
	    {
		if (STREQ(w_arrival[n].sta, sites[i].sta))
		{
		    locator_info->sta_index[n] = i;
		    locator_info->phase_index[n] = 
			get_tt_indexes (w_assoc[n].phase, i,
					&locator_info->spm_index[n]);
		    if (locator_info->phase_index[n] < 0)
		    {
			/*
			 * Phase type is invalid for time and slowness
			 */

			ar_info[n].time_error_code = 2;
			ar_info[n].slow_error_code = 2;
			if (w_arrival[n].delaz < 0.0)
			    ar_info[n].az_error_code = 4;
		    }
		    else
		    {
			if (w_arrival[n].deltim < 0.0)
			    ar_info[n].time_error_code = 4;
			if (w_arrival[n].delaz < 0.0)
			    ar_info[n].az_error_code = 4;
			if (w_arrival[n].delslo < 0.0)
			    ar_info[n].slow_error_code = 4;
		    }
		    break;
		}
	    }
	    if (locator_info->sta_index[n] < 0)
	    {
		ar_info[n].time_error_code = 1;
		ar_info[n].az_error_code   = 1;
		ar_info[n].slow_error_code = 1;
	    }
	}

	/*
	 * Correct for azimuth and slowness here, if not already done.  
	 * The function, apply_sasc_adj_in_locator(), will return a 
	 * boolean of TRUE if the SASC adjustments needs to be made here.
	 * If FALSE, this implies SASC adjustments have already been
	 * applied to arrival table.  Modeling error contributions as 
	 * an RMS measure are now added to the delaz and delslo fields
	 * of arrival.  This is OK since we will not return the arrival 
	 * table in this function.  Also, decrement number of defining 
	 * data variable (num_def_data) in cases where an association-
	 * based error code has already been determined.
	 */

	for (n = 0; n < num_obs; n++)
	{
	    if (apply_sasc_adj_in_locator())
		correct_az_slow (w_assoc[n].arid, w_assoc[n].sta, 
				 &w_arrival[n].azimuth, &w_arrival[n].slow, 
				 w_arrival[n].delaz, w_arrival[n].delslo,
				 &w_arrival[n].delaz, &w_arrival[n].delslo);

	    if (STREQ(w_assoc[n].timedef, "d") && ar_info[n].time_error_code > 0)
	    {
		strcpy (w_assoc[n].timedef, "n");
		--num_def_data;
	    }
	    if (STREQ(w_assoc[n].azdef, "d") && ar_info[n].az_error_code > 0)
	    {
		strcpy (w_assoc[n].azdef, "n");
		--num_def_data;
	    }
	    if (STREQ(w_assoc[n].slodef, "d") && ar_info[n].slow_error_code > 0)
	    {
		strcpy (w_assoc[n].slodef, "n");
		--num_def_data;
	    }
	}


	/*
	 * Open main output file, unless only screen output is desired
	 */

	if (verbose > 0)
	{
	    if (! STREQ(locator_params->outfile_name, ""))
	    {
		if ((ofp = fopen(locator_params->outfile_name, "w")) == NULL)
		{
		    fprintf (stderr, "ERR: opening file: <%s>\n",
				     locator_params->outfile_name);
		    verbose = 0;
		    ofp = stderr;
		}
	    }
	    else
		ofp = stdout;
	}

	/* Print station and data (observation) information */

	if (verbose == 3 || verbose == 4)
	{
            fprintf (ofp, "\n\n LOCATION RESULTS FOR EvID: %ld\n",origin->evid);
            fprintf (ofp, " ======================================================== \n");

	    timeEpochToString(time_offset, str, 100, GSE21);
	    fprintf (ofp, "\n First arrival detected at:  %s \n", str);
/*
            sprintf (buf, "%s", stdtime_format (time_offset, "%o"));
            fprintf (ofp, "\n First arrival detected at:  %s on %s \n\n", buf, stdtime_format (time_offset, "%b %e %Y") );
*/
            fprintf (ofp, " ============================================ ============ \n\n");
	}

	if (verbose == 4)
	{
	    for (i = 0; i < num_sites; i++)
		fprintf (ofp, " %-6s %9.4f %9.4f %9.4f\n",
				sites[i].sta, sites[i].lat, sites[i].lon,
				sites[i].elev);
	    fprintf (ofp, "\n ======================================================== \n\n");
	}

	if (verbose == 3 || verbose == 4)
	{
	    fprintf (ofp, " Ariv ID Statn  Phase    Type Atype  Observed    S.D. Err\n");
	    for (n = 0; n < num_obs; n++)
	    {
		fprintf (ofp, "%8ld %-6s %-8s t    %s%14.3f%8.3f%4ld\n",
				w_arrival[n].arid, w_arrival[n].sta,
				w_assoc[n].phase, w_assoc[n].timedef,
				w_arrival[n].time, w_arrival[n].deltim,
				ar_info[n].time_error_code);
		if (VALID_SEAZ(arrival[n].azimuth))
		    fprintf (ofp, "%8ld %-6s %-8s a    %s%14.3f%8.3f%4ld\n",
				  w_arrival[n].arid, w_arrival[n].sta,
				  w_assoc[n].phase, w_assoc[n].azdef,
				  w_arrival[n].azimuth, w_arrival[n].delaz,
				  ar_info[n].az_error_code);
		if (VALID_SLOW(arrival[n].slow))
		    fprintf (ofp, "%8ld %-6s %-8s s    %s%14.3f%8.3f%4ld\n",
				  w_arrival[n].arid, w_arrival[n].sta,
				  w_assoc[n].phase, w_assoc[n].slodef,
				  w_arrival[n].slow, w_arrival[n].delslo,
				  ar_info[n].slow_error_code);
	    }
	    fprintf (ofp, " ======================================================== \n\n\n");
	}


	/*
	 * Is there sufficient definining data for determining a solution ?
	 */

	if (num_def_data < locator_info->num_params || 
	     (! locator_params->fix_origin_time && num_time_def_data == 0))
	{
	    locator_params->num_iter = 0;
	    if (verbose > 0)
	    {
		fprintf (ofp, "\nEvID: %ld  In_OrID: %ld\n", 
				origin->evid, origin->orid);
		fprintf (ofp, "%s\n", loc_error_msg (GLerror3));
	    }
	    if (apply_sasc_adj_in_locator())
		free_active_ar_sasc ();

	    if(ofp && ofp != stderr && ofp != stdout) fclose(ofp);
	    return (GLerror3);
	}


	/*
	 * Determine a best guess initial first-cut location
	 */

	if (fabs(w_origin->lat) > 90.0 || fabs(w_origin->lon) > 180.0)
	{
	    if (locator_params->fix_lat_lon)
	    {
		locator_params->num_iter = 0;
		if (verbose > 0)
		{
		    fprintf (ofp, "\nEvID: %ld  In_OrID: %ld\n", 
					origin->evid, origin->orid);
		    fprintf (ofp, "%s\n", loc_error_msg (GLerror8));
		}
		if (apply_sasc_adj_in_locator())
		    free_active_ar_sasc ();

		if(ofp && ofp != stderr && ofp != stdout) fclose(ofp);
		return (GLerror8);
	    }

	    if (best_guess (sites, num_sites, w_arrival, w_assoc, 
			    ar_info, num_obs, locator_info, ofp,
			    verbose, &w_origin->lat, &w_origin->lon) != OK)
	    {
		fprintf (ofp, "best_guess: To few data to get an initial location\n");
		fprintf (ofp, "            Try Tonga-Fiji !!\n");
		w_origin->lat = -20.0;	w_origin->lon = -175.0;
	    }
	}


	/*
	 * Compute a hypocentral location
	 */

	ierr =
	compute_hypo (&w_origin, &w_origerr, w_arrival, &w_assoc,
		      locator_params, locator_info, &ar_info, 
		      sites, num_sites, num_obs, ofp, verbose, &torg);

	w_origin->time = torg + time_offset;

	/*
	 * Compute the location confidence bounds if location is successful,
	 * maximum number of iterations has been exceeded or solution is 
	 * found to be divergent.
	 */

	if (ierr < 3)
	    ellips (locator_info, &w_origerr, locator_params->conf_level,
		    locator_params->num_dof, locator_params->est_std_error);


	/* Fill structures given new location, if desired */

	if (ierr == 0 || locator_params->refill_if_loc_fails)
	{
            MCOPY (origin, w_origin, sizeof (Origin));
            MCOPY (origerr, w_origerr, sizeof (Origerr));
	    for (i = 0; i < num_obs; i++)
		MCOPY (&assoc[i], &w_assoc[i], sizeof (Assoc));

	    origin->jdate = timeEpochToJDate(origin->time);

	    /* Compute geographic and seismic region numbers. */

	    origin->grn = nmreg (origin->lat, origin->lon);
	    origin->srn = gtos (origin->grn);
	    w_origin->grn = origin->grn;
	    w_origin->srn = origin->srn;

	    /*
	     * Count number of defining phases (ndef) and number of
	     * depth phases (ndp) 
	     */

	    for (i = 0, ndef = 0, ndp = 0; i < num_obs; i++)
	    {
		if (STREQ (assoc[i].timedef, "d"))
		{
		    ndef++;
		    if (!strncmp (assoc[i].phase, "p", 1) || 
			!strncmp (assoc[i].phase, "s", 1))
			ndp++;
		}
	    }

	    origin->ndef = ndef;
	    origin->nass = num_obs;
	    origin->ndp  = ndp;

	    strcpy (origin->algorithm, "LocSAT");

	    /*
	     * Update dtype (depth type) attribute in origin table.  dtype 
	     * is a single-character flag that indicates the method by 
	     * which the depth was determined or constrained during the 
	     * location process.  Only the following 4 settings are 
	     * permitted: f (free); d (from depth phases); r (restrained 
	     * by location program); or g (restrained by geophysicist).
	     */

	    if (locator_params->fix_depth)
		strcpy (origin->dtype, "g");
	    else
	    {
		if (ndp > 0)
		    strcpy (origin->dtype, "d");
		else if (origin->depth > 0.0 && origin->depth < MAX_DEPTH)
		    strcpy (origin->dtype, "f");
		else
		    strcpy (origin->dtype, "r");
	    }
	}

	if (verbose > 0 && ierr < 3)
	    print_loc_results (w_origin, w_origerr, w_arrival, w_assoc,
				locator_params, locator_info, ar_info, 
				num_obs, torg, ofp, verbose, ierr);
	
	/* Print out the locator-specific error codes here */

	if (ierr > 0 && verbose > 0)
	{
	    fprintf (ofp, "\nEvID: %ld  In_OrID: %ld\n",
			  origin->evid, origin->orid);
	    fprintf (ofp, "%s\n", loc_error_msg (ierr));
	}

	if (apply_sasc_adj_in_locator())
	    free_active_ar_sasc ();

	if(ofp && ofp != stderr && ofp != stdout) fclose(ofp);
	return (ierr);
}
