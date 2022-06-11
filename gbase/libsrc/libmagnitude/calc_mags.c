
/*
 * Copyright (c) 1994-2000 Science Applications International Corporation.
 *

 * NAME 
 *	calc_mags -- Primary magnitude determination interface
 *	mag_set_compute_upper_bounds -- Does user want to compute ub mags?
 *	mag_get_compute_upper_bounds -- Get users desire to compute ub mags

 * FILE 
 *	calc_mags.c

 * SYNPOSIS 
 *	int
 *	calc_mags (magn_ptr, num_magns, origin, mag_params)
 *	MAGNITUDE	*magn_ptr;	(i)   Array of primary magnitude
 *					      structures (objects)
 *	int		num_magns;	(i)   Number of elements in magn_ptr
 *	Origin		*origin;	(i/o) CSS 3.0 origin table
 *	Mag_Params	*mag_params;	(i)   Magnitude parameter controls

 *	void
 *	mag_set_compute_upper_bounds (compute_ubs)
 *	Bool		compute_ubs;	(i)   Set boolean state to indicate
 *					      whether user desires magnitudes
 *					      whose data are solely represented
 *					      by event-based amplitudes (i.e.,
 *					      those that result in upper-bound
 *					      magnitude determinations).

 *	Bool
 *	mag_get_compute_upper_bounds ()

 * DESCRIPTIONS
 *	Computes maximum likelihood and network-averaged magnitudes as defined
 *	in the Magnitude Description File (MDF).  Also compute error estimates
 *	and associated standard deviation from station magnitude data.  Some 
 *	station magnitude data may represent upper or lower bounds.
 
 *	calc_mags() is the primary functional interface for performing 
 *	magnitude calculations within this library.  Will attempt to compute
 *	magnitudes for every magtype requested in magn_ptr pointer array
 *	structure.

 *	mag_set_compute_upper_bounds () allows the user to request that upper- 
 *	bound magnitudes be determined.  If set to FALSE, then purely event-
 *	based upper-bound magnitudes will NOT be determined.

 *	mag_get_compute_upper_bounds () informs local function whether the 
 *	user desires that upper-bound magnitudes be determined.  If FALSE, then 
 *	purely event-based upper-bound magnitudes will NOT be determined.

 *	---- Functions called ----
 *	Local:
 *		station_magnitude:	Compute individual station magnitudes
 *		network_mag:		Compute network-averaged and maximum-
 *					likelihood estimated (MLE) magnitudes
 *		mag_boot_strap:		Compute MLE magnitude via bootstrap
 *					method
 *		mag_get_compute_upper_bounds:	Compute strictly upper-bound
 *						magnitudes?

 * REFERENCES 
 *	The functions that perform the ML estimation have largely based on the
 *	original FORTRAN code of Keith MacLaughlin and adjusted to accomodate
 *	SAIC software. General references to methods employed:
 *	    - Ringdal, F. (1976). "Maximum-Likelihood Estimation of Seismic
 *	      Magnitude", Bull. Seism. Soc. Am., v. 66, pp. 789-902.
 *	      extended to handle non-detections due to clipping
 *	    - von Seggern, D. H., and D. W. Rivers (1978). "Comments on the
 *	      Use of Truncated Distribution Theory for Improved Magnitude 
 *	      Estimation", Bull. Seism. Soc. Am., v. 68, pp. 1543 - 1546.

 *	    - Use the EM algorithm to speed up the program with procedure 
 *	      by Dempster et al (1977)      

 *	    - Bootstrap for uncertainty estimates according to principles by
 *	      Efron (1983), Keith McLaughlin (1988). "Maximum-Likelihood 
 *	      Event Magnitude Estimation with Bootstrapping for Uncertainty
 *	      Estimation", Bull. Seism. Soc. Am., 78: 855-862.

 *	    - Extended to allow for a fixed "sdmag", weights, and station
 *	      corrections by Keith McLaughlin

 *	    - Subroutine for finding region type from program described in
 *	      Swedish working paper to the GSE: "IA SYSTEM" (1988). GSE/SW/62.

 * DIAGNOSTICS
 *	Will return the number of magnitudes determined (i.e., netmag records),
 *	even if they contain N/A values for magnitude and uncertainty.
 *	A return code of -1 means a problem was encountered while trying to
 *	read the test-site correction file, if requested by user.

 *	Default state of function, mag_set_compute_upper_bounds (), is FALSE.

 * FILES
 *	All magnitude info is read by function, setup_mag_facilities(),
 *	located within parallel source code file, mag_access.c.

 * NOTES
 * 	Note that 3 types of amplitude data can be processed.  These 3 
 *	amplitude types are delineated by the sm_aux.sig_type attribute.
 *	They are: 1) an actual measured signal; 2) a measured event-based
 *	(usually noise) amplitude (a lower bound); and 3) a clipped signal 
 *	(an upper bound).  The latter 2 of these can only be used in 
 *	determining an MLE magnitudes.  Network magnitude may be computed
 *	with only upper or lower bound amplitude measures.

 * SEE ALSO
 *	Calling routines in ARS(1) and EvLoc(1).  Descriptions for structure,
 *	magn, can be found in the include file, mag_descrip.h.  Descriptions
 *	for the structure, mag_params, can be found in the include file, 
 *	mag_params.h.

 * AUTHOR
 *	Walter Nagy, 10/ 6/97,	Created
 *	Walter Nagy,  7/24/98,	We now ensure that at least 2 detection-based
 *				amplitudes exist before attempting an MLE 
 *				calculation WITH bootstrap resampling.
 *	Walter Nagy,  2/ 7/00,	Added functionality that will permit user to
 *				control whether upper-bound magnitudes should
 *				be determined.  Required addition of new
 *				functions, mag_set_compute_upper_bounds()
 *				and mag_get_compute_upper_bounds()
 *
 *	Modified by YJ for stopping EvLoc over-riding non-defining magnitudes 
 *	set by the analysts on April 4, 2005
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "origin_Astructs.h"
#include "netmag_Astructs.h"
#include "stamag_Astructs.h"
#include "libmagnitude.h"
#include "magp.h"

static	Bool	compute_upper_bounds = FALSE;

int
#ifdef UsePrototypes
calc_mags (MAGNITUDE *magn_ptr, int num_magns, Origin *origin, 
	   Mag_Params *mag_params) 
#else
calc_mags (magn_ptr, num_magns, origin, mag_params)

MAGNITUDE	*magn_ptr;
int		num_magns;
Origin		*origin;
Mag_Params	*mag_params;
#endif
{
	Bool	use_this_sta = FALSE;
	Bool	boot_stat = FALSE;
	Bool	re_compute_mag = FALSE;
	int	i, j, l, n;
	int	ierr = 0;
	int	num_amps_used = 0;
	int	num_detect_based_amps = 0;
	int	num_def_detect_based_amps = 0;
	int	num_event_based_amps = 0;
	int	num_def_event_based_amps = 0;
	int	num_magdef = 0;
	int	mag_count = 0;
	double	mag, sdav, sigma;
	double	magb, sigmab, sdml, sdsdml;
	char	ts_region[9];
	char	sm_out[80];
	char	*nondef_sm_out = (char *) NULL;
	char	*def_sm_out = (char *) NULL;

	Amplitude	*ampl = (Amplitude *) NULL;
	Stamag		*sm = (Stamag *) NULL;
	SM_Sub		*sm_sub = (SM_Sub *) NULL;
	MAGNITUDE	*magn = (MAGNITUDE *) NULL;
	Mag_Cntrl	*mcntrl = (Mag_Cntrl *) NULL;
	SM_Info		sm_info;

        Netmag          Na_Netmag_rec = Na_Netmag_Init;
        Stamag          Na_Stamag_rec = Na_Stamag_Init;
        Origin          Na_Origin_rec = Na_Origin_Init;
/*
	Assoc  		Na_Assoc_rec = Na_Assoc_Init;
*/
	/*
	 * Main loop over each magnitude to be computed
	 */

	for (n = 0; n < num_magns; n++)
	{
	    magn = &magn_ptr[n];
	    magn->mag_computed = FALSE;
	    magn->mag_write = FALSE;
	    if (magn->count == 0)
		continue;	/* No data available to compute a magnitude */

	    sm_sub = UALLOC (SM_Sub, magn->count);
	    mcntrl = &magn->mag_cntrl;
	    num_magdef = 0;
	    num_detect_based_amps = 0;
	    num_def_detect_based_amps = 0;
	    num_event_based_amps = 0;
	    num_def_event_based_amps = 0;
	    nondef_sm_out = UALLOCA (char, sizeof(sm_out)*magn->count);
	    def_sm_out = UALLOCA (char, sizeof(sm_out)*(magn->count+2));
	    strcpy (nondef_sm_out, "");
	    strcpy (def_sm_out, "");
	    for (i = 0; i < magn->count; i++)
	    {
		ampl = magn->amplitude[i];
		sm = magn->stamag[i];

		/*
		 * If mode for given station magnitude records is automatic
		 * (i.e., magn->sm_aux[i].manual_override = FALSE), then
		 * make sure to initialize local magdef state to magnitude
		 * defining (i.e., TRUE).  In so doing, this algorithm will 
		 * internally evaluate all amplitude information on a 
		 * station magnitude-by-station magnitude basis as 
		 * established by the user's magnitude control settings.
		 */

		/* Commented out by YJ. */
	/*	if (! magn->sm_aux[i].manual_override)
		    strcpy (sm->magdef, "d"); */

		/*
		 * Apply test-site correction, if available ?
		 */
		
		if (mag_params->use_ts_corr)
		    strcpy (ts_region, mag_params->ts_region);
		else
		    strcpy (ts_region, "");

		/*
		 * Make sure delta field of stamag has been updated.  This is
		 * likely to be in needed triple location/magnitude mode and
		 * interactive processing.
		 */

		sm->delta =
		get_delta_for_sta (sm->sta, origin->lat, origin->lon);

		/*
		 * Compute a single station magnitude
		 */

		sm->magnitude = 
		station_magnitude (mcntrl->magtype, sm->sta, sm->phase, 
				   ampl->chan, FALSE, ts_region, sm->delta, 
				   origin->depth, ampl->amp, ampl->per, 
				   ampl->duration, ampl->snr, &sm_info);

		/*
		 * If station magnitude calculation was unsuccessful, then
		 * set magnitude defining switch to magnitude non-defining.
		 */

		if (fabs(sm->magnitude - Na_Stamag_rec.magnitude) < 0.1)
		    strcpy (sm->magdef, "n");

		/*
		 * If station to event distance is not within pre-defined 
		 * bounds for given magtype, then make magnitude non-defining.
		 */

		if (! magn->sm_aux[i].manual_override &&
		    (sm->delta < mcntrl->dist_min || 
		     sm->delta > mcntrl->dist_max))
		    strcpy (sm->magdef, "n");

		/*
		 * If only sub-station list is permissable, then test current 
		 * associated station against the sub-station list.  If test 
		 * fails, then set magnitude non-defining.
		 */

		if (! magn->sm_aux[i].manual_override &&
		    mag_params->sub_sta_list_only)
		{
		    use_this_sta = FALSE;
		    for (l = 0; l < mag_params->num_sub_sta_list; l++)
		    {
			if (STREQ (mag_params->sub_sta_list[l], sm->sta))
			{
			    use_this_sta = TRUE;
			    break;
			}
		    }
		    if (!use_this_sta)
			strcpy (sm->magdef, "n");
		}

		/*
		 * If only stations with test-site corrections options are
		 * set, then screen out those station magnitudes not 
		 * satisfying this condition.
		 */

		if (mag_params->use_only_sta_w_corr &&
		    mag_params->use_ts_corr && 
		    ! magn->sm_aux[i].manual_override &&
		    sm_info.src_dpnt_corr_type == NO_MAG_SRC_DPNT_CORR)
		    strcpy (sm->magdef, "n");

		if (STREQ (sm->magdef, "d"))
		    ++num_magdef;

		if (mag_params->verbose > 1)
		{

		    /*
		     *  Separate the non-defining and defining station
		     *  magnitude output.
		     */
		    if (STREQ (sm->magdef, "n"))
		    {
		        sprintf (sm_out, "%-6s  %-8s %-8s%-8s%7.2f%8.2f%7.2f%7.2f%9.3f%3d\n",
				     sm->sta, sm->phase, ampl->amptype,
				     ampl->chan, sm->delta, ampl->amp,
				     ampl->per, ampl->duration, sm->magnitude,
				     ierr);
			strcat (nondef_sm_out, sm_out);
		    }
		    else	/*  (STREQ (sm->magdef, "d"))  */
		    {
			if (num_magdef == 1)
			{
			    strcpy (sm_out, "\nStation Phase    Amptype Chan      Delta    Amp    Per    Dur   Sta_Mag Code\n");
			    strcat (def_sm_out, sm_out);
			    strcpy (sm_out,   "------- -------- ------- -------- ------   -----  -----  -----  ------- ----\n");
			    strcat (def_sm_out, sm_out);
			}
		        sprintf (sm_out, "%-6s  %-8s %-8s%-8s%7.2f%8.2f%7.2f%7.2f%9.3f%3d\n",
				     sm->sta, sm->phase, ampl->amptype,
				     ampl->chan, sm->delta, ampl->amp,
				     ampl->per, ampl->duration, sm->magnitude,
				     ierr);
			strcat (def_sm_out, sm_out);
		    }
		}

		/*
		 *  Store inverse of combined measurement plus modeling error 
		 *  as a weighting to be applied in magnitude determinations,
		 *  if magnitude distance variance weighting is to be employed.
		 *  In order for a weight to be applied, the sum of the
		 *  modelling and station correction errors must be larger
		 *  than 0.
		 *
		 *  If station weighting is desired, but model and station
		 *  correction errors are zero for an arrival that is
		 *  defining, then exit the program with an error message.
		 */

		if (mcntrl->apply_wgt)
		{
		    if (sm_info.model_plus_meas_error > 0.0)
		    {
			sm->uncertainty = sm_info.model_plus_meas_error;
			magn->sm_aux[i].wt = sm_info.model_plus_meas_error;
		    }
		    else
		    {
			if (STREQ (sm->magdef, "n"))
			{
			    sm->uncertainty = Na_Stamag_rec.uncertainty;
			    magn->sm_aux[i].wt = 0.0;
			}
			else	/*  sm->magdef is "d"  */
		    	{
			    fprintf (stdout, "\n\n%s weighted average desired, but no %s station weights available!\n", sm->magtype, sm->sta);
			    return (ERR);
			}
		    }
		}
		else
		{
		    sm->uncertainty = Na_Stamag_rec.uncertainty;
		    magn->sm_aux[i].wt = 0.0;
		}

		strcpy (sm->mmodel, sm_info.mmodel);
			
		strcpy (sm_sub[i].magdef, sm->magdef);
		sm_sub[i].magnitude = sm->magnitude;
		sm_sub[i].wt = magn->sm_aux[i].wt;
		sm_sub[i].sig_type = magn->sm_aux[i].sig_type;
	    }

	    if (mag_params->verbose > 1)
	    {
		/*
		 *  Write the station magnitude results to stdout
		 */
		fprintf (stdout, nondef_sm_out);
		fprintf (stdout, def_sm_out);
		fprintf (stdout, "\n");
	    }

	    while (num_magdef > 0)
	    {
		/*
		 * Compute a network magnitude
		 */

		ierr =
		network_mag (sm_sub, mcntrl, magn->count, mag_params->verbose,
			     &mag, &sigma, &sdav, &num_amps_used);

		if (mag_params->verbose > 0)
		{
		    if (mcntrl->algo_code == NET_AVG)
		    {
			fprintf (stdout, "============================================================================\n\n");
			fprintf (stdout, "Network-Averaged Results: Mag:%6.2f  Sigma:%6.2f  Sdav:%6.2f  Num:%3d\n",
					 mag, sigma, sdav, num_amps_used);
			fprintf (stdout, "\n============================================================================\n");
		    }
		    else
		    {
			fprintf (stdout, "============================================================================\n\n");
			fprintf (stdout, "MLE Results: Mag:%6.2f  Sigma:%6.2f  Sdav:%6.2f  Num:%3d\n",
					 mag, sigma, sdav, num_amps_used);
			fprintf (stdout, "\n============================================================================\n");
		    }
		}

		/*
		 * Should we perform bootstrapping?  If only 1 detection-
		 * based datum (amplitude) exists, then bootstrap 
		 * re-sampling is meaningless.  Thus, our conditional test
		 * is num_def_detect_based_amps > 1, instead of simply > 0.
		 */

		boot_stat = FALSE;
		if (mcntrl->algo_code == MLE_W_BOOTS && 
		    mag_params->num_boots > 0 && num_amps_used > 1)
		{
		    ierr =
		    mag_boot_strap (sm_sub, mcntrl, magn->count,
				    mag_params->num_boots, mag, sigma,
				    &magb, &sigmab, &sdml, &sdsdml,
				    mag_params->verbose);

		    if (mag_params->verbose > 0)
		    {
			fprintf (stdout, "\n============================================================================\n\n");
			fprintf (stdout, "MLE Bootstrap Results: num_boots: %d  sglim1: %7.2f  sglim2: %7.2f\n", mag_params->num_boots, mcntrl->sglim1, mcntrl->sglim2);
			fprintf (stdout, "  Mag:%6.2f  Sigma:%6.2f  Sdml:%6.2f  Sdsdml:%6.2f  Err: %d\n", magb, sigmab, sdml, sdsdml, ierr);
			fprintf (stdout, "\n============================================================================\n\n");
		    }

		    boot_stat = TRUE;
		}

		/*
		 * Compute a posteriori magnitude residuals and store in
		 * stamag table.  If no valid station magnitude could be
		 * computed, then make sure to set magres and mmodel to
		 * N/A values.
		 */

		for (j = 0; j < magn->count; j++)
		{
		    sm = magn->stamag[j];
		    if (sm->magnitude != Na_Stamag_rec.magnitude)
			sm->magres = sm->magnitude - mag;
		    else
		    {
			sm->magres = Na_Stamag_rec.magres;
			strcpy (sm->mmodel, Na_Stamag_rec.mmodel);
			if (STREQ (sm->magdef, "d"))
			{
			    strcpy (sm->magdef, "n");
			    --num_magdef;
			}
		    }
		}

		/*
		 * If residual screening option is selected, inspect 
		 * magnitude residuals for outliers.  If outliers are 
		 * found, then remove the associated amplitude and 
		 * re-compute network magnitude.
		 */

		re_compute_mag = FALSE;
		if (mag_params->ignore_large_res)
		{
		    for (j = 0; j < magn->count; j++)
		    {
			if (! magn->sm_aux[j].manual_override)
			{
			    sm = magn->stamag[j];
			    if (STREQ (sm->magdef, "d") && 
				fabs(sm->magres) > 
						mag_params->large_res_mult*sdav)
			    {
				re_compute_mag = TRUE;
				--num_magdef;
				strcpy (sm->magdef, "n");
				strcpy (sm_sub[j].magdef, sm->magdef);
			    }
			}
		    }
		}
		if (!re_compute_mag)
		    break;
	    }

	    /*
	     * (Re)set counters since outlier screening may have made some
	     * stamags magnitude non-defining.
	     */

	    num_detect_based_amps = 0;
	    num_def_detect_based_amps = 0;
	    num_event_based_amps = 0;
	    num_def_event_based_amps = 0;
	    for (j = 0; j < magn->count; j++)
	    {
		sm = magn->stamag[j];
		if (sm_sub[j].sig_type == MEAS_SIGNAL)
		{
		    ++num_detect_based_amps;
		    if (!strcmp (sm->magdef, "d"))
			++num_def_detect_based_amps;
		}
		else
		{
		    ++num_event_based_amps;
		    if (!strcmp (sm->magdef, "d"))
			++num_def_event_based_amps;
		}
	    }

	    magn->netmag.magnitude = Na_Netmag_rec.magnitude;
	    magn->netmag.uncertainty = Na_Netmag_rec.uncertainty;
	    strcpy (magn->netmag.net, mag_params->net);

	    if (mcntrl->algo_code == NET_AVG)
	    {
		/*
		 * Update key database attribute fields in netmag and stamag
		 * for network-averaged magnitude determinations.
		 */

		if (num_magdef > 0)
		{
		    ++mag_count;
		    magn->mag_computed = TRUE;
		    magn->mag_write = TRUE;
		    magn->netmag.magnitude = mag;
		    magn->netmag.uncertainty = sdav;
		}
		else if (num_detect_based_amps > 0)
		{
		    ++mag_count;
		    magn->mag_write = TRUE;
		}
	    }
	    else
	    {
		/*
		 * Update key database attribute fields in netmag and 
		 * stamag for MLE magnitude determinations.

		 * If the user has requested that upper-bound magnitude 
		 * determinations be suppressed (i.e., return code of 
		 * function, mag_get_compute_upper_bounds (), is FALSE)
		 * AND num_def_detect_based_amps = 0, then do not allow 
		 * associated netmag and stamag records to be written to
		 * the database.
		 */

		if (num_magdef > 0 && 
		    (mag_get_compute_upper_bounds () || 
		     num_def_detect_based_amps > 0))
		{
		    ++mag_count;
		    magn->mag_computed = TRUE;
		    magn->mag_write = TRUE;
		    magn->netmag.magnitude = mag;
		    if (boot_stat)	/* MLE; w/ bootstrapping */
			magn->netmag.uncertainty = sigmab;
		    else		/* MLE; w/o bootstrapping */
			magn->netmag.uncertainty = sigma;
		}
		else if (num_detect_based_amps > 0 ||
			 (mag_get_compute_upper_bounds () && 
			  num_event_based_amps > 0))
		{
		    ++mag_count;
		    magn->mag_write = TRUE;
		}
	    }

	    magn->netmag.nsta = num_magdef;
	    magn->netmag.orid = origin->orid;
	    for (j = 0; j < magn->count; j++)
		magn->stamag[j]->orid = origin->orid;

	    /*
	     * Populate origin magnitude and related ID fields if magtype 
	     * match is found for mb, Ms or ML.  If no network magnitude
	     * could be computed, then populate with N/A values.
	     */

	    if (STREQ (mcntrl->magtype, mag_params->magtype_to_origin_mb))
	    {
		if (num_magdef > 0)
		{
		    origin->mb   = magn->netmag.magnitude;
		    origin->mbid = magn->netmag.magid;
		}
		else
		{
		    origin->mb   = Na_Origin_rec.mb;
		    origin->mbid = Na_Origin_rec.mbid;
		}
	    }
	    else if (STREQ (mcntrl->magtype, mag_params->magtype_to_origin_ms))
	    {
		if (num_magdef > 0)
		{
		    origin->ms   = magn->netmag.magnitude;
		    origin->msid = magn->netmag.magid;
		}
		else
		{
		    origin->ms   = Na_Origin_rec.ms;
		    origin->msid = Na_Origin_rec.msid;
		}
	    }
	    else if (STREQ (mcntrl->magtype, mag_params->magtype_to_origin_ml))
	    {
		if (num_magdef > 0)
		{
		    origin->ml   = magn->netmag.magnitude;
		    origin->mlid = magn->netmag.magid;
		}
		else
		{
		    origin->ml   = Na_Origin_rec.ml;
		    origin->mlid = Na_Origin_rec.mlid;
		}
	    }

	    UFREE (sm_sub);

	}	/* End of main magnitude loop */

	return (mag_count);
}


void
#ifdef UsePrototypes
mag_set_compute_upper_bounds (Bool compute_ub)
#else
mag_set_compute_upper_bounds (compute_ub)
Bool    compute_ub;
#endif
{
	/*
	 * Set boolean state indicating calling functions desire to compute
	 * upper-bound (purely event-based) magnitudes.  Default state is
	 * FALSE.
	 */

	compute_upper_bounds = compute_ub;
	return;
}


Bool
#ifdef UsePrototypes
mag_get_compute_upper_bounds ()
#else
mag_get_compute_upper_bounds ()
#endif
{
	return (compute_upper_bounds);
}

