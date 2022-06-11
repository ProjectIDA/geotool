
/*
 * Copyright (c) 1993-1997 Science Applications International Corporation.

 * NAME
 *	LocSAT -- Event locator module

 * FILE    
 *	main.c

 * SYNOPSIS
 *	Compute event locations, confidence bounds, residuals and data
 *	importances using arrival times, azimuths, and slowness 
 *	measusrements from stations at regional and teleseismic distances.

 * DESCRIPTION
 *	Main program.  Determine event locations and related statistics via 
 *	least-squares inversion.  LocSAT runs in either a single event
 *	or batch mode.  LocSAT computes event locations, confidence bounds, 
 *	residuals and importances using arrival times, azimuths, and 
 *	slowness measusrements from stations at regional and teleseismic 
 *	distances.  Event locations are determined via an iterative 
 *	non-linear inverse technique.  This approach is essentially that of 
 *	Jordan and Sverdrup (1981) extended to include azimuth and slowness 
 *	data.  The key feature of this method is that it allows a priori
 *	knowledge of the data variances to be incorporated along with the 
 *	resultant a posteriori data residuals.  This makes calculation of 
 *	the error ellipses particularly robust.

 *	In order to better approximate event location in a truly heterogeneous
 *	media, while also maintaining the simplicity of travel-time tables
 *	based on a one-dimensional velocity model, various source-specific
 *	corrections are made, mostly with respect to travel-time.  These 
 *	source-specific station correction files are read in locate_event() 
 *	as specified in the local par file.  Travel-time corrections are 
 *	applied as follows.  First, if a test-site correction is requested, 
 *	travel-time corrections will be applied to all stations specified 
 *	within the user-specified region.  If no test-site correction is 
 *	requested or found, then LocSAT will look for a SSSC, if requested.
 *	If the current event location is within a SSSC region and a 
 *	correction exists for the specified station/phase pair, then this 
 *	correction will be applied and no SRST correction will be attempted,
 *	even if one exists.  If no SSSC exists and the user specifies SRST 
 *	corrections be applied, then LocSAT will search the SRST file to 
 *	identify whether the current event location is within one of its 
 *	convex geographic regions.  If so, an SRST correction will be 
 *	applied.  Finally, an elevation and ellipticity correction will
 *	be applied regardless of the correction type applied (or not applied).

 *	Numerous input variable controls can be used to control event 
 *	location.

 *	Usage:	LocSAT par=par.file [any other getpar options]

 *	Information on stations and detection parameters are read directly 
 *	from this LocSAT as specified in the par file or command line
 *	arguments, and are formated as follows:

 *	STATION FILE
 *	Each Line:	fscanf (fio, "%6s%lf%lf%lf%*[^\n]",
 *	----------------------------------------------------------------
 *	sites[].sta	%6s	Station code
 *	sites[].lat	%9.4f	Latitude (deg)
 *	sites[].lon	%9.4f	Longitude (deg)
 *	sites[].elev	%9.4f	Elevation (km)

 *	DATA FILE
 *	First line:	fscanf (fio, "%4d%2d%2d %2d%2d%lf%lf%lf%lf%lf%d%*[^\n]",
 *	----------------------------------------------------------------
 *	year		%4d	Year term on origin
 *	month		%2d	Month term of origin
 *	day		%2d	Day term of origin
 *	hour		%2d	Hour term of origin
 *	minute		%2d	Minute term of origin
 *	second		%lf	Seconds after minute of origin
 *	lat_init	%lf	First guess latitude (deg)
 *	lon_init	%lf	First guess longitude (deg)
 *	depth_init	%lf	Initial depth (km) 
 *	event_mag	%f	Event magnitude
 *	num_data	%3d	Number of observation in data file

 *	Note that lat_init, lon_init, and depth_init are part of the
 *	locator_params structure.  If lat_init or lon_init are set to
 *	+/-999.0, then LocSAT will determine its own best guess initial 
 *	location.  Also note effect of variable, use_location).

 *	Following lines: fscanf (fio, "%8d %6s %8s %4s%2s%lf%lf%*[^\n]",
 *	----------------------------------------------------------------
 *	arrival_id 	%8d 	Arrival ID (put into structure,
 *				assoc[].arid)
 *	station_name	%6s	Station name (put into structure,
 *				arrival[].sta)
 *      phase_type	%8s	Phase type (e.g., Pn, PKP, Lg) 
 *				(put into structure, assoc[].phase)
 *      data_type	%4s	Data type ([t]ime, [a]zim, or [s]low)
 *      arrival_type	%2s	Arrival usage (put into the appropriate 
 *				assoc structure, i.e., assoc[].timedef,
 *				assoc[].azdef and/or assoc[].slodef)
 *				  = d: Defining, used in location
 *				  = n: Non-defining, not used in location
 *	obs_data	%f	Observed datum (sec, deg, or sec/deg)
 *				(put into the appropriate arrival structure,
 *				i.e., arrival[].time, arrival[].azimuth
 *				or arrival[].slow)
 *	std_dev_data	%f	Standard deviation of observed datum (sec,
 *				deg, or sec/deg) (put into the appropriate 
 *				arrival structure, i.e., arrival[].deltim, 
 *				arrival[].delaz, or arrival[].delslo)

 *	A blank line must seperate events when run in batch mode.

 *	PAR FILE
 *	----------------------------------------------------------------
 *	vmodel_spec_file%s	Directory pathway and filename containing 
 *				velocity model information.
 *	list_of_phases	%vs	List of acceptable phases to be used in
 *				determining event locations (f.e.,
 *				"P,PKPdf,S,Pn,Lg,PcP")
 *	sasc_dir_prefix	%s	Name directory pathway and prefix name that
 *				define location of SASC tables.  If not
 *				defined, then no SASC can be applied as part
 *				of event location process.

 *	sssc_level 	%d	Desired level for SSSC

 *	use_location	%b	Use location given on the summary line of
 *				the data (phase) file; else let LocSAT
 *				determine it own best initial location
 *	fix_ot		%b	Constrain (fix) origin time
 *	fix_latlon	%b	Constrain (fix) lat/lon
 *	fix_depth	%b	Constrain (fix) depth
 *	verbose		%s	Degree of verbose output
 *				  = 0:	No output (Will not be passed here)
 *				  = 1:	Print only final location info
 *				  = 2:	Print final location and arrival info
 *				  = 3:	Do not print intermediate iterations
 *					and input station info
 *				  = 4:	Print all output
 *	conf_level	%lf	Confidence ellipse level
 *	damp		%lf	Percent damping relative to largest 
 *				singular value
 *	num_dof 	%d	Number of degrees of freedom in sig0
 *	max_iter	%d	Maximum number of location iterations

 *	ignore_big_res	%b	Ignore large (big) data residual during
 *				construction of system matrix, where the
 *				residual is > big_res_mult*data_std_err[]
 *	big_res_mult	%lf	Large (big) residual multiplier factor.  If
 *				this factor times the data_std_err[] for any
 *				phase is < data residual, then the phase will
 *				not be used in the location during that
 *				particular iteration.
 *	user_var_wgt	%lf	If > 0.0, use this user-defined variance 
 *				weighting in preference of all other variance 
 *				weighting.
 *	dist_var_wgt	%b	If set, pre-defined distance variance weighting
 *				will be applied to data for calculation of the
 *				error ellipses only.  This weighting will 
 *				over-ride the SRST variance weighting when
 *				calculating these error ellipses.
 *	use_srst	%b	Use (apply) SRST corrections for events with
 *				stations encompasing a valid SRST region
 *	srst_var_wgt	%b	Use (apply) SRST variance weighting for events 
 *				with stations encompasing a valid SRST region
 *				when use_srst is turned on.
 *	only_sta_w_corr	%b	Use only arrival data with SRST corrections 
 *				available.  Only meaningful when use_srst is
 *				turned on.  Data without SRST corrections
 *				will be ignored. 
 *	use_tscor	%b	Use (apply) a specified test-site travel-time
 *				correction.  A test-site correction will be
 *				applied as defined by variable, ts_region,
 *				and will over-ride SSSC and SRST corrections.
 *	ts_region	%s	Correction for this test-site region will be
 *				applied, if use_tscor is turned on.

 *	Almost all variables in the par file belong to the locator_params 
 *	structure.

 *	---- Functions called ----
 *	From libloc
 *		locate_event:		Main event location module
 *		setup_tt_facilities:	Read travel-time information

 *	From libpar
 *		setpar:		Initializes necessary parameters to libpar
 *		getpar:		Individual libpar parameters are obtained here
 *		mstspar:	Same as getpar, except that it terminates with
 *				an error message if parameter is not specified,
 *				and is restricted to strings only.

 * DIAGNOSTICS
 *	Complains when input data are bogus or poorly formatted.

 * FILES
 *	Reads control (par), data and station files here.

 * NOTES
 *	None.

 * SEE ALSO
 *	Bratt and Bache (1988).  "Locating events with a sparse network
 *	of regional arrays", BSSA, 78, 780-798.  The extended descriptions 
 *	for the database structures along with acceptable bounds is given 
 *	in the CSS database reference manual, "Center for Seismic Studies, 
 *	Version 3 Database: Schema Reference Manual" by Anderson, Farrell,
 *	Garcia, Given and Swanger, 1990.

 * AUTHOR
 *	Walter Nagy, July 23, 1993,	Created.
 *	Walter Nagy, Feb. 24, 1997,	Can now employ SASC by specifying new
 *					par file argument, sasc_dir_prefix.
 */


#include "config.h"
#include <unistd.h>
#include <sys/param.h>

#include "motif++/Application.h"
#include "locsat.h"
extern "C" {
#include "ibase/libloc.h"
#include "ibase/loc_params.h"
#include "ibase/site_Astructs.h"
#include "ibase/origin_Astructs.h"
#include "ibase/origerr_Astructs.h"
#include "ibase/arrival_Astructs.h"
#include "ibase/assoc_Astructs.h"

#include "libstring.h"
#include "libtime.h"
#include "logErrorMsg.h"
}

extern "C" {

int
locsat(CssTableClass *css, int num_phases, char **phases, cvector<CssSiteClass> &css_site,
		cvector<CssArrivalClass> &css_arrival, cvector<CssAssocClass> &css_assoc,
		CssOriginClass *css_origin, CssOrigerrClass *css_origerr, char **msg)
{
	LocSAT *params;
	Arrival	*arrival;
	Assoc	*assoc;
	Origerr	*origerr;
	Origin	*origin;
	Site	*sites;
	Ar_Info	*ar_info;
	int	i, fd, status;
	Locator_params *locator_params;
	char tmp_file[20];
	FILE *fp;
	struct stat buf;
	static char last_sasc_dir[MAXPATHLEN+1] = "";

 	params = (LocSAT *)css;

	*msg = NULL;
	locator_params	= UALLOC (Locator_params, 1);


	/* Specify static parameters */

	locator_params->refill_if_loc_fails = (int) FALSE;
	locator_params->use_elev_corr	    = (int) TRUE;
	locator_params->est_std_error	    = (double) 1.0;

	/* Specify parameters (as necessary for getpar) w/ default values */

	locator_params->verbose		    = '4';

	locator_params->use_location	    = (int) params->use_location;
	locator_params->fix_origin_time	    = (int) params->fix_ot;
	locator_params->fix_lat_lon	    = (int) params->fix_latlon;
	locator_params->fix_depth	    = (int) params->fix_depth;
	locator_params->origin_time_init    = (double)params->time_init;
	locator_params->lat_init	    = (double)params->lat_init;
	locator_params->lon_init	    = (double)params->lon_init;
	locator_params->depth_init	    = (double)params->depth_init;
	locator_params->damp		    = (double)params->damp;
	locator_params->conf_level	    = (double)params->conf_level;
	locator_params->num_dof		    = (int)params->num_dof;
	locator_params->max_iterations	    = (int)params->max_iter;
	locator_params->ignore_large_res    = (int)params->ignore_big_res;
	locator_params->large_res_mult	    = (double)params->big_res_mult;
	locator_params->ellip_cor_type	    = (int)params->ellip_cor_level;
	locator_params->sssc_level	    = (int)params->sssc_level;
	locator_params->use_test_site_corr  = (int) FALSE;
	locator_params->use_srst	    = (int)params->use_srst;
	locator_params->srst_var_wgt	    = (int)params->srst_var_wgt;
	locator_params->use_only_sta_w_corr = (int)params->only_sta_w_corr;
	locator_params->dist_var_wgt	    = (int)params->dist_var_wgt;
	locator_params->user_var_wgt	    = (double)params->user_var_wgt;
	strcpy (locator_params->test_site_region, "-");

	locator_params->prefix =(char *)quarkToString(params->vmodel_spec_file);

	snprintf(tmp_file, 20, "/tmp/geotlXXXXXX");
	fd = mkstemp(tmp_file);
	locator_params->outfile_name = tmp_file;

#ifdef _ZZZ_
	getpar ("verbose", "s", &locator_params->verbose);
	getpar ("fix_ot", "b", &locator_params->fix_origin_time);
	getpar ("fix_latlon", "b", &locator_params->fix_lat_lon);
	getpar ("fix_depth", "b", &locator_params->fix_depth);
	getpar ("use_location", "b", &locator_params->use_location);
	getpar ("damp", "F", &locator_params->damp);
	getpar ("conf_level", "F", &locator_params->conf_level);
	getpar ("num_dof", "d", &locator_params->num_dof);
	getpar ("max_iter", "d", &locator_params->max_iterations);
	getpar ("ignore_big_res", "b", &locator_params->ignore_large_res);
	getpar ("big_res_mult", "F", &locator_params->large_res_mult);
	getpar ("ellip_cor_level", "d", &locator_params->ellip_cor_type);
	getpar ("sssc_level", "d", &locator_params->sssc_level);
	getpar ("use_srst", "b", &locator_params->use_srst);
	getpar ("srst_var_wgt", "b", &locator_params->srst_var_wgt);
	getpar ("dist_var_wgt", "b", &locator_params->dist_var_wgt);
	getpar ("user_var_wgt", "F", &locator_params->user_var_wgt);
	getpar ("only_sta_w_corr", "b", &locator_params->use_only_sta_w_corr);

	getpar ("use_tscor", "b", &locator_params->use_test_site_corr);
	if (locator_params->use_test_site_corr)
	    getpar ("ts_region", "s", locator_params->test_site_region);
	locator_params->prefix = mstspar ("vmodel_spec_file");

	/*
	 * If loc_w_depth_every_X_km is > 0.0, then perform multiple event
	 * locations at depth increment specified by this variable.  For
	 * example, if loc_w_depth_every_X_km = 10.0, then 71 depth-
	 * constrained event locations will be computed at depths 
	 * between 0 and 700 km.
	 */

	getpar ("loc_w_depth_every_X_km", "F", &loc_w_depth_every_X_km);

	mio[TOKS].fnam = mstspar ("sta_file");

	/*
	 * Read list of acceptable phases
	 */

	sprintf (vstr, "vs[%d]", MAX_NUM_PHASES);
	phases = UALLOC (char *, MAX_NUM_PHASES);
	for (i = 0; i < MAX_NUM_PHASES; i++)
	    phases[i] = UALLOC (char, 9);
	num_phases = mstpar ("list_of_phases", vstr, (void *) phases);

	strcpy (input_string, "NULL");
	getpar ("sasc_dir_prefix", "s", input_string);
	sasc_dir_prefix = STRALLOC (input_string);

	endpar ();

	strcpy (summary_line, orid);
	strcat (summary_line, ".dat");
	mio[TOKD].fnam = STRALLOC (summary_line);
	strcpy (summary_line, orid);
	strcat (summary_line, ".out");
/***** output to a tmp file */
	locator_params->outfile_name = STRALLOC (summary_line);
#endif

	/* Allocate space for Site structures */
	sites	= UALLOC (Site, css_site.size());

	for(i = 0; i < css_site.size(); i++) {
	    stringcpy(sites[i].sta, css_site[i]->sta, sizeof(sites[i].sta));
	    sites[i].lat = css_site[i]->lat;
	    sites[i].lon = css_site[i]->lon;
	    sites[i].elev = css_site[i]->elev;
	}

	/*
	 * Read travel-time tables
	 */

	if (setup_tt_facilities (locator_params->prefix, phases, num_phases,
				 sites, css_site.size()) != OK)
	{
	    logErrorMsg(LOG_WARNING, "locsat: Problems reading T-T tables.");
	    UFREE (locator_params);
	    UFREE (sites);
	    close(fd);
	    return -1;
	}

	/*
	 * Read slowness/azimuth station correction (SASC) tables
	 */
	if(strcmp(last_sasc_dir, quarkToString(params->sasc_dir_prefix)))
	{
	    stringcpy(last_sasc_dir, quarkToString(params->sasc_dir_prefix),
			sizeof(last_sasc_dir));
	    if (read_sasc (last_sasc_dir) != OK)
	    {
		logErrorMsg(LOG_WARNING,
			"locsat: Problems while trying to read SASC tables.");
		UFREE (locator_params);
		UFREE (sites);
		close(fd);
		return -1;
	    }
	}

	/* Allocate space for Origin, Origerr structures */
	origin	= UALLOC (Origin, 1);
	origerr	= UALLOC (Origerr, 1);

	/* Initialize origin and origerr structures */

	origin->orid = css_origin->orid;
	origin->evid = css_origin->evid;

	origin->jdate = css_origin->jdate;
	origin->nass = css_origin->nass;
	origin->ndef = css_origin->ndef;
	origin->ndp = css_origin->ndp;
	origin->grn = css_origin->grn;
	origin->srn = css_origin->srn;
	strncpy(origin->etype, css_origin->etype, sizeof(origin->etype));
	origin->depdp = css_origin->depdp;
	strncpy(origin->dtype, css_origin->dtype, sizeof(origin->dtype));
	origin->mb = css_origin->mb;
	origin->mbid = css_origin->mbid;
	origin->ms = css_origin->ms;
	origin->msid = css_origin->msid;
	origin->ml = css_origin->ml;
	origin->mlid = css_origin->mlid;
	strncpy(origin->algorithm, css_origin->algorithm, sizeof(origin->algorithm));
	strncpy(origin->auth, css_origin->auth, sizeof(origin->auth));
	origin->commid = css_origin->commid;
	strncpy(origin->lddate, "-", sizeof(origin->lddate));

	origin->lat	= locator_params->lat_init;
	origin->lon	= locator_params->lon_init;
	origin->depth	= locator_params->depth_init;
	origin->time    = locator_params->origin_time_init;
	origerr->smajax	= -1.0; origerr->sminax	= -1.0; origerr->strike	= -1.0;
	origerr->stime	= -1.0; origerr->sdepth	= -1.0; origerr->sdobs	= -1.0;
	origerr->sxx	= -1.0; origerr->sxz	= -1.0; origerr->syz	= -1.0;
	origerr->syy	= -1.0; origerr->szz	= -1.0; origerr->sxy	= -1.0;
	origerr->stt	= -1.0; origerr->stx	= -1.0; origerr->sty	= -1.0;
	origerr->stz	= -1.0;
	origerr->orid   = css_origerr->orid;
	origerr->conf   = css_origerr->conf;
	origerr->commid = css_origerr->commid;
	strncpy(origerr->lddate, "-", sizeof(origerr->lddate));

	/* Allocate space for arrival, assoc and ar_info structures */

	arrival	= UALLOC (Arrival, css_arrival.size());
	assoc	= UALLOC (Assoc, css_arrival.size());
	ar_info	= UALLOC (Ar_Info, css_arrival.size());

	assoc[0].orid = 90000001;

	/* Read detection info, one detection at a time */

	for(i = 0; i < css_arrival.size(); i++)
	{
	    arrival[i].arid = css_arrival[i]->arid;
	    assoc[i].arid   = css_arrival[i]->arid;
	    assoc[i].orid   = css_origin->orid;
	    stringcpy(arrival[i].sta, css_arrival[i]->sta,
			sizeof(arrival[i].sta));
	    stringcpy(assoc[i].sta, css_arrival[i]->sta,
			sizeof(assoc[i].sta));
	    stringcpy(assoc[i].phase, css_assoc[i]->phase,
			sizeof(assoc[i].phase));
	    arrival[i].time	= css_arrival[i]->time;
	    arrival[i].azimuth	= css_arrival[i]->azimuth;
	    arrival[i].delaz	= css_arrival[i]->delaz;
	    arrival[i].slow  	= css_arrival[i]->slow;
	    arrival[i].delslo	= css_arrival[i]->delslo;
	    arrival[i].deltim	= css_arrival[i]->deltim;
	    stringcpy(assoc[i].timedef, css_assoc[i]->timedef,
			sizeof(assoc[i].timedef));
	    stringcpy(assoc[i].azdef, css_assoc[i]->azdef,
			sizeof(assoc[i].azdef));
	    stringcpy(assoc[i].slodef, css_assoc[i]->slodef,
			sizeof(assoc[i].slodef));
	    assoc[i].timeres = css_assoc[i]->timeres;
	    assoc[i].azres = css_assoc[i]->azres;
	    assoc[i].slores = css_assoc[i]->slores;
	    assoc[i].wgt = css_assoc[i]->wgt;
	    stringcpy(assoc[i].vmodel, css_assoc[i]->vmodel,
			sizeof(assoc[i].vmodel));
	    assoc[i].delta = css_assoc[i]->delta;
	    assoc[i].seaz = css_assoc[i]->seaz;
	    assoc[i].esaz = css_assoc[i]->esaz;
	}

	/*
	 * Execute location using travel-time tables, stations and detections 
	 * already read.  Send input data to LocSAT:
	 */

	status = locate_event (sites, css_site.size(), arrival, assoc, origin, 
		       origerr, locator_params, ar_info, css_arrival.size());

	for(i = 0; i < css_arrival.size(); i++)
	{
	    css_assoc[i]->timeres = assoc[i].timeres;
	    css_assoc[i]->azres = assoc[i].azres;
	    css_assoc[i]->slores = assoc[i].slores;
	    css_assoc[i]->wgt = assoc[i].wgt;
	    stringcpy(css_assoc[i]->vmodel, assoc[i].vmodel,
			sizeof(css_assoc[i]->vmodel));
	    css_assoc[i]->delta = assoc[i].delta;
	    css_assoc[i]->seaz = assoc[i].seaz;
	    css_assoc[i]->esaz = assoc[i].esaz;
	}
	css_origin->lat = origin->lat;
	css_origin->lon = origin->lon;
	css_origin->depth = origin->depth;
	css_origin->time = origin->time;
	css_origin->jdate = origin->jdate;
	css_origin->nass = origin->nass;
	css_origin->ndef = origin->ndef;
	stringcpy(css_origin->dtype, origin->dtype, sizeof(css_origin->dtype));
	css_origin->ndp = origin->ndp;
	css_origin->grn = origin->grn;
	css_origin->srn = origin->srn;
	stringcpy(css_origin->algorithm, origin->algorithm,
		sizeof(css_origin->algorithm));

	css_origerr->sxx = origerr->sxx;
	css_origerr->syy = origerr->syy;
	css_origerr->szz = origerr->szz;
	css_origerr->stt = origerr->stt;
	css_origerr->sxy = origerr->sxy;
	css_origerr->sxz = origerr->sxz;
	css_origerr->syz = origerr->syz;
	css_origerr->stx = origerr->stx;
	css_origerr->sty = origerr->sty;
	css_origerr->stz = origerr->stz;
	css_origerr->sdobs = origerr->sdobs;
	css_origerr->smajax = origerr->smajax;
	css_origerr->sminax = origerr->sminax;
	css_origerr->strike = origerr->strike;
	css_origerr->sdepth = origerr->sdepth;
	css_origerr->stime = origerr->stime;
	css_origerr->conf = origerr->conf;

	UFREE (origin);
	UFREE (origerr);
	UFREE (sites);
	UFREE (arrival);
	UFREE (assoc);
	UFREE (ar_info);

	close(fd);

	if(!stat(locator_params->outfile_name, &buf)
		&& (fp = fopen(locator_params->outfile_name, "r")) != NULL)
	{
	    char *c = (char *)malloc(buf.st_size+1);
	    for(i = 0; i < buf.st_size && (c[i] = fgetc(fp)) != EOF; i++);
	    fclose(fp);
	    c[i] = '\0';
	    *msg = c;
	}
	unlink(locator_params->outfile_name);

	UFREE (locator_params);

	return status;
}

}

#define offset(field) ((unsigned int)(((const char *)&a.field) - ((const char *)&a))), sizeof(a.field)

/** Define LocSAT table.
 */

static void defineLocSAT(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    LocSAT a(0);
    CssClassDescription des[] = {
    {1,     1,   "use_location",  "%1d",offset(use_location),0, CSS_BOOL,  "0"},
    {3,     3,         "fix_ot",  "%1d",offset(fix_ot),      0, CSS_BOOL,  "0"},
    {5,     5,     "fix_latlon",  "%1d",offset(fix_latlon),  0, CSS_BOOL,  "0"},
    {7,    15,      "fix_depth", "%.4f",offset(fix_depth),   0, CSS_BOOL,  "0"},
    {17,   33,      "time_init","%.5lf",offset(time_init),   0,CSS_TIME,"none"},
    {35,   43,       "lat_init", "%.4f",offset(lat_init),  0,CSS_FLOAT,"-999."},
    {45,   53,       "lon_init", "%.4f",offset(lon_init),  0,CSS_FLOAT,"-999."},
    {55,   63,     "depth_init", "%.4f",offset(depth_init),0,CSS_FLOAT,"-999."},
    {65,   69,           "damp", "%.3f",offset(damp),      0,CSS_FLOAT,  "-1."},
    {71,   75,     "conf_level", "%.3f",offset(conf_level),0,CSS_FLOAT,  ".90"},
    {77,   82,        "num_dof",   "%d",offset(num_dof),   0,CSS_INT,  "99999"},
    {84,   89,       "max_iter",   "%d",offset(max_iter),  0,CSS_INT,     "20"},
    {91,   91, "ignore_big_res",  "%1d",offset(ignore_big_res),0,CSS_BOOL, "0"},
    {93,   97,   "big_res_mult", "%.2f",offset(big_res_mult), 0,CSS_FLOAT,"3."},
    {99,   99,"ellip_cor_level",   "%d",offset(ellip_cor_level),0, CSS_INT,"2"},
    {101, 102,     "sssc_level",   "%d",offset(sssc_level),     0,CSS_INT, "1"},
    {104, 104,       "use_srst",   "%d",offset(use_srst),       0,CSS_BOOL,"0"},
    {106, 106,   "srst_var_wgt",   "%d",offset(srst_var_wgt),   0,CSS_BOOL,"0"},
    {108, 108,   "dist_var_wgt",   "%d",offset(dist_var_wgt),   0,CSS_BOOL,"1"},
    {110, 110,"only_sta_w_corr",  "%d",offset(only_sta_w_corr),0,CSS_BOOL,"0"},
    {112, 117,   "user_var_wgt","%.1f", offset(user_var_wgt),0,CSS_FLOAT,"-1."},
    {119, 126,           "orid","%.1f", offset(orid), 	       0,CSS_LONG,"-1"},
    {128, 144,         "lddate",  "%s", offset(lddate),      0,CSS_LDDATE, "-"},
    };
    CssClassExtra extra[] = {
	{"vmodel_spec_file", "%s", offset(vmodel_spec_file),  CSS_QUARK, ""},
	{"sasc_dir_prefix", "%s", offset(sasc_dir_prefix), CSS_QUARK, ""},
    };
    CssTableClass::define("LocSAT", sizeof(des)/sizeof(CssClassDescription), des,
		sizeof(extra)/sizeof(CssClassExtra), extra,
		LocSAT::createLocSAT, sizeof(LocSAT));
  }
}

/** Create a LocSAT object.
 */
LocSAT::LocSAT(void) : CssTableClass("LocSAT")
{
    defineLocSAT();
    initTable();
}

LocSAT::LocSAT(LocSAT &a) : CssTableClass("LocSAT")
{
    defineLocSAT();
    initTable();
    a.copyTo(this);
}

extern "C" {

void
initLocSAT(void)
{
        string prop;
	char *geotool_home=NULL;

        defineLocSAT();

	/* set vmodel_spec_file and sasc_dir_prefix from GEOTOOL_HOME
	 */
        if(!Application::getProperty("locate.LocSAT.vmodel_spec_file", prop) &&
	    (geotool_home = (char *)getenv("GEOTOOL_HOME")) )
	{
	    Application::putProperty("locate.LocSAT.vmodel_spec_file",
		geotool_home + string("/tables/data/TT/vmsf/idc.defs"));
        }

        if (!Application::getProperty("locate.LocSAT.sasc_dir_prefix", prop) &&
	    (geotool_home = (char *)getenv("GEOTOOL_HOME")) )
	{
	    Application::putProperty("locate.LocSAT.sasc_dir_prefix",
		geotool_home + string("/tables/data/SASC/sasc"));
	}
}

}
