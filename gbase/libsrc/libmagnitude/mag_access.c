
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * NAME
 *	setup_mag_facilities -- Read magnitude corr tables & link station info
 *	setup_mc_tables -- Read magnitude correction tables
 *	station_magnitude -- Compute station magnitude with all corrections
 *	abbrev_sta_mag -- Compute station magnitude (short version)
 *	initialize_sm_info -- Initialize sm_info structure to N/A values
 *	get_magtype_features -- Fill mag_cntrl structure for given magtype
 *	get_meas_error -- Get magnitude measure error based on SNR
 *	reset_algorithm -- Reset algorithm setting from MDF
 *	reset_min_dist -- Reset minimum distance setting for given magtype
 *	reset_max_dist -- Reset maximum distance setting for given magtype
 *	reset_amptypes -- Reset detection and event amptypes
 *	reset_sd_limits -- Reset lower/upper std dev bounds for a given magtype
 *	reset_sd_baseline -- Reset baseline std dev for a given magtype
 *	reset_wgt_ave_flag -- Reset weighted average flag for a given magtype
 *	revert_algorithm -- Revert to original algorithm setting in MDF
 *	revert_min_dist -- Revert to original minimum distance setting in MDF
 *	revert_max_dist -- Revert to original maximum distance setting in MDF
 *	revert_sd_limits -- Revert to original std dev bound settings in MDF
 *	revert_amptypes -- Revert to original detection and event amptypes
 *	revert_sd_baseline -- Revert to original std dev baseline setting in MDF
 *	revert_wgt_ave_flag -- Revert to original weighted average flag setting

 * FILE
 *	mag_access.c

 * SYNOPSIS
 *	int
 *	setup_mag_facilities (tl_model_filename, mdf_filename, list_of_magtypes,
 *			      num_magtypes, sites, num_sites)
 *	char	*tl_model_filename;	(i) TL model file name 
 *	char	*mdf_filename;		(i) MDF file name 
 *	char	**list_of_magtypes;	(i) Character array list of desired
 *					    magtypes to be read and accessed
 *	int	num_magtypes;		(i) Number of desired magtypes
 *	Site	*sites;			(i) Station (site table) structure
 *	int	num_sites;		(i) Number of station (site) records

 *	int
 *	setup_mc_tables (tl_model_filename, mdf_filename, list_of_magtypes,
 *			 num_magtypes)
 *	char	*tl_model_filename;	(i) TL model file name 
 *	char	*mdf_filename;		(i) MDF file name 
 *	char	**list_of_magtypes;	(i) Character array list of desired
 *					    magtypes to be read and accessed
 *	int	num_magtypes;		(i) Number of desired magtypes

 *	double
 *	station_magnitude (magtype, sta, phase, chan, extrapolate, ts_region, 
 *			   distance, ev_depth, amp, period, duration, snr, 
 *			   sm_info) 
 *	char	*magtype;		(i) Requested magnitude type (magtype)
 *	char	*sta;			(i) Station name
 *	char	*phase;			(i) Phase type
 *	char	*chan;			(i) Channel designator
 *	Bool	extrapolate;		(i) Return T-T even if extrapolated
 *	char	*ts_region;;		(i) Test-site corr region.  If NULL, no
 *					    test-site corr to be applied.
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	ev_depth;		(i) Event depth (km)
 *	double	amp;			(i) Amplitude (nm)
 *	double	period;			(i) Period (sec)
 *	double	duration;		(i) Coda duration, if available (sec)
 *	double	snr;			(i) Signal-to-noise ratio 
 *	SM_Info	*sm_info;		(o) Station magnitude info structure

 *	double
 *	abbrev_sta_mag (magtype, sta, phase, chan, distance, ev_depth, 
 *			amp, period, duration) 
 *	char	*magtype;		(i) Requested magnitude type (magtype)
 *	char	*sta;			(i) Station name
 *	char	*phase;			(i) Phase type
 *	char	*chan;			(i) Channel designator
 *	double	distance;		(i) Event-to-station distance (deg.)
 *	double	ev_depth;		(i) Event depth (km)
 *	double	amp;			(i) Amplitude (nm)
 *	double	period;			(i) Period (sec)
 *	double	duration;		(i) Coda duration, if available (sec)

 *	SM_Info
 *	initialize_sm_info ()

 *	Bool
 *	get_meas_error (snr)
 *	double	snr;			(i) Signal-to-noise ratio

 *	Bool
 *	get_magtype_features (magtype, Mag_Cntrl *mag_cntrl)
 *	char		*magtype;	(i) selected magtype
 *	Mag_Cntrl	*mag_cntrl;	(o) Populated mag_cntrl structure

 *	int
 *	reset_amptypes (magtype, det_amptype, ev_amptype)
 *	char	*magtype;		(i) selected magtype
 *	char	*det_amptype;		(i) reset detection amptype
 *	char	*ev_amptype;		(i) reset event amptype

 *	int
 *	reset_algorithm (magtype, algo_code)
 *	char	*magtype;		(i) selected magtype
 *	int	algo_code;		(i) reset algorithm code

 *	int
 *	reset_min_dist (magtype, dist_min)
 *	char	*magtype;		(i) selected magtype
 *	double	dist_min;		(i) reset minimum distance (deg)

 *	int
 *	reset_max_dist (magtype, dist_max)
 *	char	*magtype;		(i) selected magtype
 *	double	dist_max;		(i) reset maximum distance (deg)

 *	int
 *	reset_sd_limits (magtype, sglim1, sglim2)
 *	char	*magtype;		(i) selected magtype
 *	double	sglim1;			(i) reset lower bound limit of s.d.
 *	double	sglim2;			(i) reset upper bound limit of s.d.

 *	int
 *	reset_sd_baseline (magtype, sgbase)
 *	char	*magtype;		(i) selected magtype
 *	double	sgbase;			(i) reset baseline s.d.

 *	int
 *	reset_wgt_ave_flag (magtype, apply_wgt)
 *	char	*magtype;		(i) selected magtype
 *	Bool	apply_wgt;		(i) reset weighted average flag

 *	int
 *	revert_amptypes (magtype)
 *	char	*magtype;		(i) selected magtype

 *	int
 *	revert_algorithm (magtype)
 *	char	*magtype;		(i) selected magtype

 *	int
 *	revert_min_dist (magtype)
 *	char	*magtype;		(i) selected magtype

 *	int
 *	revert_max_dist (magtype)
 *	char	*magtype;		(i) selected magtype

 *	int
 *	revert_sd_limits (magtype)
 *	char	*magtype;		(i) selected magtype

 *	int
 *	revert_sd_baseline (magtype)
 *	char	*magtype;		(i) selected magtype

 *	int
 *	revert_wgt_ave_flag (magtype)
 *	char	*magtype;		(i) selected magtype

 * DESCRIPTION
 *	Functions.  These routines provide the core station magnitude handling
 *	facilities for the SAIC magnitude module.  All functionality local to
 *	the MC table structure, mc_table, is handled within this file.

 *	-- setup_mag_facilities() is the primary user interface used for 
 *	reading distance/depth-dependent magnitude-correction (MC) tables 
 *	and linked station information to them.  This routine needs to be 
 *	called before any other magnitude manipulation facility is accessed.

 *	-- setup_mc_tables() is the main interface used for reading MC tables
 *	as formatted following the mc_table structure.  This function reads 
 *	and initializes the MC tables for various magnitude-related routines 
 *	(e.g., ARS, EvLoc, GA and StaPro).  These tables are read from disk 
 *	each time a change is detected in the list of tables being maintained 
 *	and the list of tables input through the argument list.  MC table 
 *	information is stored local to this routine using the mc_table 
 *	structure (see include file, mc_table.h, for specifics).

 *	-- station_magnitude() is used to obtain total station magnitude 
 *	along with all applicable corrections and errors.  This is the
 *	main function for computing all station magnitudes.  It is capable
 *	of computing station magnitude that are station/phase/channel-
 *	dependent, with or with test-site corrections.  It is also capable
 *	of computing an SNR-based measurement error and modeling error.
 *	This latter information, in addition to derivatives and individual
 *	correction elements are returned in the sm_info structure defined
 *	in the include file, mag_descrip.h.  The input arguments, amp, per,
 *	chan and duration are all contained within the new amplitude table.

 *	-- abbrev_sta_mag() is a simplified version of the above function, 
 *	station_magnitude().  It does not permit inclusion of test-site
 *	corrections, calculation of SNR-based measurement error, and does
 *	not return sm_info structure.  (The sm_info structure contains alot
 *	of specifics concerning the given station magnitude calculation).
 *	No extrapolation for station magnitude is permitted by invoking this
 *	function call.  This is an auxilary function for computing station 
 *	magnitudes when the bare minimum is desired.

 *	-- initialize_sm_info(), initializes sm_info structure (type, SM_Info)
 *	to N/A values.

 *	-- get_magtype_features(), gets mag_cntrl control structure info
 *	given an input magtype.

 *	-- get_meas_error() determines the magnitude measurement error 
 *	based on an input signal-to-noise ratio (SNR).  Under the current 
 *	implementation, this function always returns 0.0.

 *	-- reset_amptypes() permits a calling application to reset the
 *	detection and event-based amptype settings originally established 
 *	from within the MDF for a given magtype.

 *	-- reset_algorithm() permits a calling application to reset the
 *	algorithm setting originally established from within the MDF 
 *	for a given magtype.

 *	-- reset_min_dist() permits a calling application to reset the
 *	minimum distance setting originaly established within the MDF for 
 *	a given magtype.

 *	-- reset_max_dist() permits a calling application to reset the
 *	maximum distance setting originaly established within the MDF for 
 *	a given magtype.

 *	-- reset_sd_limits() permits a calling application to reset the
 *	lower and upper bound limit settings of standard deviation for
 *	a given magtype.

 *	-- reset_sd_baseline() permits a calling application to reset the
 *	baseline standard deviation setting originally established
 *	within the MDF for a given magtype.

 *	-- reset_wgt_ave_flag() permits a calling application to reset the
 *	weighted average flag setting originally established within the MDF
 *	for a given magtype.

 *	-- revert_amptypes() reverts back to original detection and event-
 *	based amptype settings established in the MDF.  This is designed 
 *	to be employed after reset_amptypes() has been used to temporarily 
 *	effect the desired amptypes to be used for a given magtype.  Likely 
 *	to be needed when a new origin is selected, in say, ARS.

 *	-- revert_algorithm() reverts back to original algorithm settings
 *	established in the MDF.  This is designed to be employed after 
 *	reset_algorithm() has been used to temporarily effect the desired
 *	magnitude algorithm to be used.  Likely to be needed when a new 
 *	origin is selected, in say, ARS.

 *	-- revert_min_dist() reverts back to original minimum valid distance 
 *	range setting established in the MDF.  This is designed to be 
 *	employed after reset_min_dist() has been used to temporarily effect 
 *	the distance range.  This might be necessary when one wishes to 
 *	include/exclude station magnitude information in some distance range,
 *	yet have a mechanism to revert to the original valid distance range 
 *	setting, for say, when a new origin is selected.

 *	-- revert_max_dist() reverts back to original maximum valid distance 
 *	range setting established in the MDF.  This is designed to be 
 *	employed after reset_max_dist() has been used to temporarily effect 
 *	the distance range.  This might be necessary when one wishes to 
 *	include/exclude station magnitude information in some distance range,
 *	yet have a mechanism to revert to the original valid distance range 
 *	setting, for say, when a new origin is selected.

 *	-- revert_sd_limits() reverts back to original lower and upper bound
 *	limit settings of standard deviation established in the TTLF.  This 
 *	is designed to be employed after reset_sd_limits() has been used
 *	to temporarily effect the standard deviation limit bounds.

 *	-- revert_sd_baseline() reverts back to original baseline standard
 *	deviation setting established in the MDF.  This is designed to be
 *	employed after reset_sd_baseline() has been used to temporarily
 *	effect the baseline standard deviation.

 *	-- revert_wgt_ave_flag() reverts back to setting of the weighted
 *	average flag established in the MDF.  This is designed to be
 *	employed after reset_wgt_ave_flag() has been used to temporarily
 *	effect the weighted average flag.

 * DIAGNOSTICS
 *	See mag_error_table[] in file, mag_error_msg.c, for specific global
 *	magnitude error code descriptions.

 *	-- station_magnitude() will return with a value of -999.0 if a
 *	problem is encountered (f.e., an invalid TLtype; problems 
 *	with interpolation routine, etc.).  Elsewise, the station magnitude
 *	is returned, if successful.

 *	-- abbrev_sta_mag() will return with a value of -999.0 if a problem
 *	is encountered (f.e., an invalid TLtype; problems with interpolation 
 *	routine, etc.).  Elsewise, the station magnitude is returned, if 
 *	successful.

 *	-- initialize_sm_info(), returns a default sm_info structure (type, 
 *	SM_Info) populated with N/A values.

 *	-- get_magtype_features(), returns a TRUE condition if magtype mapping
 *	exists; else it returns FALSE.

 *	-- reset_amptypes() will return an error code of ERR (-1) if 
 *	input magtype has not be specified in MDF; else will return a 
 *	code of OK (0).

 *	-- reset_algorithm() will return an error code of ERR (-1) if 
 *	input magtype has not be specified in MDF; else will return a 
 *	code of OK (0).

 *	-- reset_min_dist() will return an error code of ERR (-1) if input
 *	magtype has not be specified in MDF; else it will return a code of 
 *	OK (0).

 *	-- reset_max_dist() will return an error code of ERR (-1) if input
 *	magtype has not be specified in MDF; else it will return a code of 
 *	OK (0).

 *	-- reset_sd_limits() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return 
 *	a code of OK (0).

 *	-- reset_sd_baseline() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return 
 *	a code of OK (0).

 *	-- reset_wgt_ave_flag() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return 
 *	a code of OK (0).

 *	-- revert_amptypes() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return a
 *	code of OK (0).

 *	-- revert_algorithm() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return a
 *	code of OK (0).

 *	-- revert_min_dist() will return an error code of ERR (-1) if input
 *	magtype has not be specified in MDF; else it will return a code of 
 *	OK (0).

 *	-- revert_max_dist() will return an error code of ERR (-1) if input
 *	magtype has not be specified in MDF; else it will return a code of 
 *	OK (0).

 *	-- revert_sd_limits() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return a
 *	code of OK (0).

 *	-- revert_sd_baseline() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return a
 *	code of OK (0).

 *	-- revert_wgt_ave_flag() will return an error code of ERR (-1) if
 *	input magtype has not be specified in MDF; else it will return a
 *	code of OK (0).

 * FILES
 *	Magnitude correction (MC) table files are read by function, 
 *	read_tl_table(), via function, read_tlsf(), as called here.

 * NOTES
 *	The acronym, STM, stands for the station_atten_model structure that
 *	that contains station-specific information regarding the chosen 1-D
 *	magnitude model (mmodel) and bulk static station correction.  TLSF 
 *	is an acronym for transmission loss specification file.

 * SEE ALSO
 *	Functions, read_tlsf() and read_tl_table() for reading of distance/
 *	depth-dependent magnitude corrections (transmission loss) and 
 *	modelling error info.

 * AUTHOR
 *	Walter Nagy, 8/21/97,	Created.
 *	Doug Brumbaugh, 12/18/1999,  Added functions reset_sd_baseline(),
 *				     reset_wgt_ave_flag(), revert_sd_baseline(),
 *				     revert_wgt_ave_flag().
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stamag_Astructs.h"
#include "libmagnitude.h"
#include "magp.h"
#include "libinterp.h"
#include "libgeog.h"

static	Bool	new_mc_tables_read = TRUE;
static	char	*prev_tl_model_filename = (char *) NULL;

static	int	num_md = 0;
static	int	num_mst = 0;

static	Mag_Descrip	*mag_descrip = (Mag_Descrip *) NULL;
static	Mag_Sta_TLType	*mag_sta_tltype = (Mag_Sta_TLType *) NULL;
static	SM_Info		SM_struct;


int
#ifdef __STDC__
setup_mag_facilities (char *tl_model_filename, char *mdf_filename, 
		      char **list_of_magtypes, int num_magtypes,
		      Site *sites, int num_sites)
#else
setup_mag_facilities (tl_model_filename, mdf_filename, list_of_magtypes,
		      num_magtypes, sites, num_sites)

char    *tl_model_filename;
char    *mdf_filename;
char    **list_of_magtypes;
int	num_magtypes;
Site	*sites;
int	num_sites;
#endif
{
	int	icode = OK;


	if ((icode = setup_mc_tables (tl_model_filename, mdf_filename,
				      list_of_magtypes, num_magtypes,
				      sites, num_sites)) != OK)
	    return (icode);

	if ((icode = set_sta_TL_pt (sites, num_sites)) != OK)
	{
	    fprintf (stderr, "%s\n", mag_error_msg (icode));
	    return (icode);
	}

	return (OK);
}


int
#ifdef __STDC__
setup_mc_tables (char *tl_model_filename, char *mdf_filename, 
		 char **list_of_magtypes, int num_magtypes, Site *sites,
		 int num_sites)
#else
setup_mc_tables (tl_model_filename, mdf_filename, list_of_magtypes, 
		 num_magtypes, sites, num_sites)

char    *tl_model_filename;
char    *mdf_filename;
char    **list_of_magtypes;
int	num_magtypes;
Site	*sites;
int	num_sites;
#endif
{
	int	icode;
	TL_Pt	*list_of_TLtypes = (TL_Pt *) NULL;
	TL_Pt	*tl, *tlnext;


	/*
	 * If not first read, then check to see if directory prefix of 
	 * magnitude-correction (MC) tables has changed.  In the case of
	 * identical directory prefixes, then just return.
	 */

	if (! new_mc_tables_read)
	{
	    if (STREQ (tl_model_filename, prev_tl_model_filename))
		return (OK);

	    UFREE (prev_tl_model_filename);
	}

	prev_tl_model_filename = STRALLOC (tl_model_filename);

	/*
	 * Read magnitude-specific information from Magnitude Description
	 * File (MDF).
	 */

	if ((icode = read_mdf (mdf_filename, list_of_magtypes, num_magtypes,
				&mag_descrip, &num_md, &mag_sta_tltype, 
				&num_mst, &list_of_TLtypes)) != OK)
	{
	    fprintf (stderr, "%s\n", mag_error_msg (icode));
	    return (icode);
	}

	/*
	 * Do actual reading of magnitude-correction (MC) tables here via
	 * reading of Transmission Loss Specificaton File (TLSF)!  Return
	 * error status as determined by function, read_tl_tables(), if a
	 * serious problem was encountered.
	 */

	if ((icode = read_tlsf (tl_model_filename, list_of_TLtypes, sites,
				num_sites)) != OK)
	{
	    fprintf (stderr, "%s\n", TL_error_msg (icode));
	    return (icode);
	}

	new_mc_tables_read = TRUE;

	/* free list_of_TLtypes */
	for (tl=list_of_TLtypes ; tl != (TL_Pt *)NULL ; tl=tlnext)
	{
		tlnext = tl->next;
		UFREE (tl);
	}

	return (OK);
}


double
#ifdef	__STDC__
station_magnitude (char *magtype, char *sta, char *phase, char *chan, 
		   Bool extrapolate, char *ts_region, double distance, 
		   double ev_depth, double amp, double period, double duration,
		   double snr, SM_Info *sm_info)
#else
station_magnitude (magtype, sta, phase, chan, extrapolate, ts_region, 
		   distance, ev_depth, amp, period, duration, snr, sm_info)
char	*magtype;
char	*sta;
char	*phase;
char	*chan;
Bool	extrapolate;
char	*ts_region;
double	distance;
double	ev_depth;
double	amp;
double	period;
double	duration;
double	snr;
SM_Info	*sm_info;
#endif
{

	int	i;
	int	interp_code = OK;
	int	sta_index, stm_index, mst_index;
	int	md_index, tlmd_index, tl_index;
	double	mag_corr;
	double	tot_mag_corr;
	double	dist_depth_corr;
	double	sta_magnitude;
	double	mag_cor_deriv[4];
	char	model[16];
        Stamag  Na_Stamag_rec = Na_Stamag_Init;


	/*
	 * Initialize SM_struct structure to N/A values.
	 */

	SM_struct = initialize_sm_info ();

	/*
	 * Get necessary indexes to calculate a station magnitude.  If no
	 * valid information exist for given input arguments, then tl_index
	 * will be populated with negative integer value, namely -1.  In
	 * this case, no valid station magnitude can be computed, so return
	 * with -999.0.
	 */

	if ((tl_index = 
		get_mag_indexes (magtype, sta, phase, chan, &sta_index, 
				 &stm_index, &tlmd_index, &md_index, 
				 &mst_index)) < 0)
	    return (Na_Stamag_rec.magnitude);

	/*
	 * Get distance/depth magnitude correction.
	 */

	dist_depth_corr = 
	interp_for_tl_value (distance, ev_depth, tl_index, extrapolate,
			     mag_cor_deriv, &interp_code);

	SM_struct.mc_table_value = dist_depth_corr;
	SM_struct.mag_error_code = interp_code;

	if (fabs(dist_depth_corr - Na_Stamag_rec.magnitude) < 0.1)
	    return (Na_Stamag_rec.magnitude);

	for (i = 0; i < 4; i++)
	    SM_struct.mag_cor_deriv[i] = mag_cor_deriv[i];

	/*
	 * Grab bulk static station correction and station correction
	 * error terms, if available!
	 */

	if (mst_index < 0)
	{
	    SM_struct.bulk_static_sta_corr =
		mag_descrip[md_index].def_sta_corr;
	    SM_struct.bulk_sta_corr_error =
		mag_descrip[md_index].def_sta_corr_error;
	}
	else
	{
	    SM_struct.bulk_static_sta_corr = 
		mag_sta_tltype[mst_index].bulk_sta_corr;
	    SM_struct.bulk_sta_corr_error =
		mag_sta_tltype[mst_index].bulk_sta_corr_error;
	}

	mag_corr = SM_struct.bulk_static_sta_corr;

	/*
	 * Get magnitude test-site correction, if requested (i.e., if
	 * character string, ts_region, is != NULL and a valid test-site
	 * table exists for given TLtype).  Cannot apply both test-site
	 * and bulk static magnitude station correction together.  Therefore,
	 * if test-site correction exists, it takes precedence.
	 */

	if (ts_region != (char *) NULL && strcmp (ts_region, ""))
	{
	    if (get_TL_ts_corr (ts_region, sta, mag_descrip[md_index].TLtype,
				tl_index, &mag_corr))
	    {
		SM_struct.src_dpnt_corr_type = MAG_TEST_SITE_CORR;
		SM_struct.src_dpnt_corr = mag_corr;
	    }
	}

	/*
	 *  Get modelling error from TL table (if it exists) and 
	 *  add to input measurement error and bulk station correction
	 *  error in a least-squares sense to get total error.
	 *  If no modelling error exists, then use the baseline
	 *  value in the MDF for the associated magtype as the
	 *  default modelling error.
	 */

	SM_struct.model_error = 
	    get_tl_model_error (tl_index, distance, ev_depth, model);
	if (SM_struct.model_error == NA_MODEL_ERROR)
	    SM_struct.model_error = mag_descrip[md_index].sgbase;

	SM_struct.meas_error = get_meas_error (snr);
	SM_struct.model_plus_meas_error = 
			sqrt (SM_struct.model_error*SM_struct.model_error + 
			      SM_struct.meas_error*SM_struct.meas_error +
			      SM_struct.bulk_sta_corr_error*
			      SM_struct.bulk_sta_corr_error);

	tot_mag_corr = dist_depth_corr + mag_corr;

	/*
	 * Finally, compute station magnitude as sum of log amplitude over
	 * period plus the total magnitude correction (mag_corr).  If no
	 * period dependency exists (i.e., period = -1.0), then simply
	 * calculate station magnitude (sta_magnitude) as log (amp) + corr.
	 * If neither an amplitude nor period dependency exists, then
	 * a duration magnitude is to be computed.  This is defined as:
	 * log (duration) + corr.
	 */

	if (amp < 0.0 && period < 0.0)
	    sta_magnitude = log10 (duration) + tot_mag_corr;
	else if (period < 0.0)
	    sta_magnitude = log10 (amp) + tot_mag_corr;
	else
	    sta_magnitude = log10 (amp/period) + tot_mag_corr;

	SM_struct.sta_magnitude = sta_magnitude;
	SM_struct.total_mag_corr = tot_mag_corr;
	strcpy (SM_struct.mmodel, model);

	*sm_info = SM_struct;

	return (sta_magnitude);
}


double
#ifdef	__STDC__
abbrev_sta_mag (char *magtype, char *sta, char *phase, char *chan, 
		double distance, double ev_depth, double amp, double period, 
		double duration)
#else
abbrev_sta_mag (magtype, sta, phase, chan, distance, ev_depth, amp, period,
		duration)
char	*magtype;
char	*sta;
char	*phase;
char	*chan;
double	distance;
double	ev_depth;
double	amp;
double	period;
double	duration;
#endif
{
	char	ts_region[9];
	double	sta_mag = -999.0;
	SM_Info	sm_null;

	strcpy (ts_region, "");

	sta_mag =
	station_magnitude (magtype, sta, phase, chan, FALSE, ts_region,
			   distance, ev_depth, amp, period, duration, 
			   -1.0, &sm_null);

	return (sta_mag);
}


SM_Info
#ifdef	__STDC__
initialize_sm_info ()
#else
initialize_sm_info ()
#endif
{
	SM_Info	Default_SM_Info;
        Stamag  Na_Stamag_rec = Na_Stamag_Init;

	Default_SM_Info.mag_error_code		= 0;
	Default_SM_Info.sta_magnitude		= Na_Stamag_rec.magnitude;
	Default_SM_Info.src_dpnt_corr_type	= NO_MAG_SRC_DPNT_CORR;
	Default_SM_Info.total_mag_corr		= -1.0;
	Default_SM_Info.mc_table_value		= -1.0;
	Default_SM_Info.bulk_static_sta_corr	= 0.0;
	Default_SM_Info.bulk_sta_corr_error	= 0.0;
	Default_SM_Info.src_dpnt_corr		= 0.0;
	Default_SM_Info.model_error		= -1.0;
	Default_SM_Info.meas_error		= -1.0;
	Default_SM_Info.model_plus_meas_error	= -1.0;
	Default_SM_Info.mag_cor_deriv[0]	= -1.0;
	Default_SM_Info.mag_cor_deriv[1]	= -1.0;
	Default_SM_Info.mag_cor_deriv[2]	= -1.0;
	Default_SM_Info.mag_cor_deriv[3]	= -1.0;
	strcpy (Default_SM_Info.mmodel, "-");
	strcpy (Default_SM_Info.lddate, "-");

	return (Default_SM_Info);
}


double
#ifdef	__STDC__
get_meas_error (double snr)
#else
get_meas_error (snr)
double	snr;
#endif
{
	return (0.0);	/* Temporary */
}


int
#ifdef	__STDC__
get_mag_indexes (char *magtype, char *sta, char *phase, char *chan,
		int *sta_index, int *stm_index, int *tlmd_index, 
		int *md_index, int *mst_index)
#else
get_mag_indexes (magtype, sta, phase, chan, sta_index, stm_index, tlmd_index,
		 md_index, mst_index)
char	*magtype;
char	*sta;
char	*phase;
char	*chan;
int	*sta_index;
int	*stm_index;
int	*tlmd_index;
int	*md_index;
int	*mst_index;
#endif
{
	Bool	magtype_found = FALSE;
	int	i;
	int	tl_index;
	char	TLtype[9];

	*md_index = -1;
	*mst_index = -1;

	if (!magtype || !*magtype)
	    return (ERR);

	/*
	 * First, determine default tl_index from tltype_model_descrip 
	 * definition.  Will need to inspect list_of_phz elements based
	 * on input phase specified to get specific tl_index.
	 */

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		magtype_found = TRUE;
		*md_index = i;
		strcpy (TLtype, mag_descrip[i].TLtype);
		break;
	    }
	}

	if (!magtype_found)
	    return (ERR);

	/*
	 * Now go out and get transmission loss (TL) indexes necessary to
	 * compute station magnitude information.
	 */

	tl_index =
	get_TL_indexes (TLtype, sta, phase, chan, sta_index, stm_index, 
			tlmd_index);

	/*
	 * Only get MST index (mst_index) if no STM information avaliable,
	 * since bulk static magnitude station corrections are only meaningful
	 * relative to tltype_model_descrip definitions.  If STM information
	 * is specified, then mst_index = -1.
	 */

	if (*stm_index < 0)
	{
	    for (i = 0; i < num_mst; i++)
	    {
		if (STREQ (sta, mag_sta_tltype[i].sta) &&
		    STREQ (TLtype, mag_sta_tltype[i].TLtype))
		{
		    *mst_index = i;
		    break;
		}
	    }
	}

	return (tl_index);
}


Bool
#ifdef	__STDC__
get_magtype_features (char *magtype, Mag_Cntrl *mag_cntrl) 
#else
get_magtype_features (magtype, *mag_cntrl)
char		*magtype;	/* selected input magtype */
Mag_Cntrl	*mag_cntrl;	/* mag_cntrl control settings */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		strcpy (mag_cntrl->magtype, mag_descrip[i].magtype);
		strcpy (mag_cntrl->TLtype, mag_descrip[i].TLtype);
		strcpy (mag_cntrl->det_amptype, mag_descrip[i].det_amptype);
		strcpy (mag_cntrl->ev_amptype, mag_descrip[i].ev_amptype);
		mag_cntrl->algo_code = mag_descrip[i].algo_code;
		mag_cntrl->dist_min = (double) mag_descrip[i].dist_min;
		mag_cntrl->dist_max = (double) mag_descrip[i].dist_max;
		mag_cntrl->sglim1 = (double) mag_descrip[i].sglim1;
		mag_cntrl->sglim2 = (double) mag_descrip[i].sglim2;
		mag_cntrl->sgbase = (double) mag_descrip[i].sgbase;
		mag_cntrl->apply_wgt = (Bool) mag_descrip[i].apply_wgt;

		return (TRUE);
	    }
	}

	return (FALSE);	/* No magtype definition exists */
}


int
#ifdef	__STDC__
reset_amptypes (char *magtype, char *det_amptype, char *ev_amptype)
#else
reset_amptypes (magtype, det_amptype, ev_amptype)
char	*magtype;	/* selected magtype */
char	*det_amptype;	/* reset detection-based amptype for given magtype */
char	*ev_amptype;	/* reset event-based amptype for given magtype */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		strcpy (mag_descrip[i].det_amptype, det_amptype);
		strcpy (mag_descrip[i].ev_amptype, ev_amptype);
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         amptype settings.  Values specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
reset_algorithm (char *magtype, int algo_code)
#else
reset_algorithm (magtype, algo_code)
char	*magtype;	/* selected magtype */
int	algo_code;	/* reset algorithm code (0: Net Avg; 1: MLE) */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].algo_code = algo_code;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         algorithm setting.  Value specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
reset_min_dist (char *magtype, double dist_min)
#else
reset_min_dist (magtype, dist_min)
char	*magtype;	/* selected magtype */
double	dist_min;	/* reset min valid distance range for magtype (deg) */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].dist_min = (float) dist_min;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         minimum distance setting.  Values specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
reset_max_dist (char *magtype, double dist_max)
#else
reset_max_dist (magtype, dist_max)
char	*magtype;	/* selected magtype */
double	dist_max;	/* reset max valid distance range for magtype (deg) */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].dist_max = (float) dist_max;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         maximum distance setting.  Values specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
reset_sd_limits (char *magtype, double sglim1, double sglim2)
#else
reset_sd_limits (magtype, sglim1, sglim2)
char	*magtype;	/* selected magtype */
double	sglim1;		/* reset lower bound limit of s.d. for magtype */
double	sglim2;		/* reset upper bound limit of s.d. for magtype */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].sglim1 = (float) sglim1;
		mag_descrip[i].sglim2 = (float) sglim2;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         lower/upper bound limit settings.  Values specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
reset_sd_baseline (char *magtype, double sgbase)
#else
reset_sd_baseline (magtype, sgbase)
char	*magtype;	/* selected magtype */
double	sgbase;		/* reset baseline s.d. for magtype */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].sgbase = (float) sgbase;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         baseline stdev setting.  Values specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
reset_wgt_ave_flag (char *magtype, Bool apply_wgt)
#else
reset_wgt_ave_flag (magtype, apply_wgt)
char	*magtype;	/* selected magtype */
Bool	apply_wgt;	/* reset flag indicating whether or not to compute
			 * weighted average for magtype */
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].apply_wgt = apply_wgt;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to reset\n", magtype);
	fprintf (stderr, "         weighted average flag.  Values specified in MDF will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_amptypes (char *magtype)
#else
revert_amptypes (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		strcpy (mag_descrip[i].det_amptype, 
			mag_descrip[i].orig_det_amptype);
		strcpy (mag_descrip[i].ev_amptype, 
			mag_descrip[i].orig_ev_amptype);
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         amptype settings.  Override values will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_algorithm (char *magtype)
#else
revert_algorithm (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].algo_code = mag_descrip[i].orig_algo_code;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         algorithm setting.  Override value will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_min_dist (char *magtype)
#else
revert_min_dist (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].dist_min = mag_descrip[i].orig_dist_min;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         minimum distance setting.  Override value will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_max_dist (char *magtype)
#else
revert_max_dist (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].dist_max = mag_descrip[i].orig_dist_max;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         maximum distance setting.  Override value will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_sd_limits (char *magtype)
#else
revert_sd_limits (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].sglim1 = mag_descrip[i].orig_sglim1;
		mag_descrip[i].sglim2 = mag_descrip[i].orig_sglim2;
		mag_descrip[i].sgbase = mag_descrip[i].orig_sgbase;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         lower/upper bound settings.  Override value will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_sd_baseline (char *magtype)
#else
revert_sd_baseline (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].dist_max = mag_descrip[i].orig_sgbase;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         baseline stdev setting.  Override value will be retained!\n");
	return (ERR);	/* magtype not found */
}


int
#ifdef	__STDC__
revert_wgt_ave_flag (char *magtype)
#else
revert_wgt_ave_flag (magtype)
char	*magtype;
#endif
{
	int	i;

	for (i = 0; i < num_md; i++)
	{
	    if (STREQ (mag_descrip[i].magtype, magtype))
	    {
		mag_descrip[i].dist_max = mag_descrip[i].orig_apply_wgt;
		return (OK);
	    }
	}

	fprintf (stderr, "Warning: Invalid magtype, %s, specified in attempt to revert\n", magtype);
	fprintf (stderr, "         weighted average flag setting.  Override value will be retained!\n");
	return (ERR);	/* magtype not found */
}
