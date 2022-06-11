
/*
 * Copyright (c) 1990-2004 Science Applications International Corporation.
 *

 * NAME
 *	print_loc_results_ -- Print out location results.

 * FILE
 *	print_loc_results_.c

 * SYNOPSIS
 *	void
 *	print_loc_results (origin, origerr, arrival, assoc, locator_params, 
 *			   locator_info, ar_info, num_obs, torg, ofp, 
 *			   verbose, ierr)
 *	Origin		*origin;	(i) Local working DB3.0 origin table
 *	Origerr		*origerr;	(i) Local working DB3.0 origerr table
 *	Arrival		*arrival;	(i) Standard CSS DB3.0 arrival table
 *	Assoc		*assoc;		(i) Local working DB3.0 assoc table
 *	Locator_params	*locator_params;(i) Locator parameter info structure
 *	Locator_info	*locator_info;	(i) Locator info structure
 *	Ar_Info		*ar_info;	(i) Arrival-specific location info
 *	int		num_obs;	(i) Number of arrival/assoc records
 *	double		torg;		(i) Final estimate of origin time (sec)
 *	FILE		*ofp;		(i) Output file pointer
 *	int		verbose;	(i) Degree of verbose output
 *					    =0: No output
 *					    =1: Print only final location info
 *					    =2: Print final location and
 *						arrival info
 *					    =3: Do not print intermediate
 *						iterations & input station info
 *					    =4: Print all output
 *	int		ierr;		(i) Solution error flag (code)
 *					    =0:	No error
 *					    =1:	Maximum # of iterations exceeded
 *					    =2:	Divergent solution encountered

 * DESCRIPTION
 *	Function.  Print out the final location results once a solution 
 *	has been determined.

 *	ar_info[n].time_error_code:	Error code for n'th observation
 *	          .az_error_code:
 *	          .slow_error_code:
 *				  =  0, No problem, normal interpolation
 *				  =  1, No station information for datum
 *				  =  2, No travel-time tables for datum
 *				  =  3, Data type unknown
 *				  =  4, S.D <= 0.0 for datum
 *				  =  5, Data residual too large for datum
 *				  =  6, Only datum w SRST correction are
 *					included in this location
 *				  =  8, Trv_time too far off table.
 *				  = 11, Distance-depth point (x0,z0) in 
					hole of T-T curve
 *				  = 12, x0 < x(1)
 *				  = 13, x0 > x(max)
 *				  = 14, z0 < z(1)
 *				  = 15, z0 > z(max)
 *				  = 16, x0 < x(1) and z0 < z(1)
 *				  = 17, x0 > x(max) and z0 < z(1)
 *				  = 18, x0 < x(1) and z0 > z(max)
 *				  = 19, x0 > x(max) and z0 > z(max)
 *				  = 20, radial_2D tables show path blocked.
 *				[NOTE:	If any of these codes is .le. 0 (e.g. 
 *					data_err_code = -17), the datum was 
 *					used to compute event location]

 * DIAGNOSTICS
 *	None.

 * FILES
 *	Output is written to user-specified file (or the screen) at the level
 *	specified by the verbose flag.

 * NOTES
 *	Current usage of ar_sasc structure is local to libloc and, by design,
 *	not available at the public level.

 * SEE ALSO
 *	Driver routine locate_event() for input values.

 * AUTHORS
 *	Walter Nagy,  8/92,	Created.
 *	Walter Nagy,  1/96,	Significantly enhanced to display arrival-based
 *				location info via new ar_info structure.
 *      Walter Nagy, 11/03,	Added station/phase-dependent azimuth/slowness 
 *				computation output, including SASCs, as requested 
 *				in IDC CR P1 (AWST).  Call to new function,
 *				get_ar_sasc() added.
 */


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "locp.h"
#include "loc_defs.h"
#include "libtime.h"
#include "libgeog.h"
#include "ar_sasc.h"

#define	NUM_PRINT_MSG_ERRS	21

struct	delta_time_index
{
	double	delta;
	double	time;
	double	index;
};

static int sort_delta_compare(const void *elm1, const void *elm2);
static int sort_time_compare(const void *elm1, const void *elm2);


void
print_loc_results (Origin *origin, Origerr *origerr, Arrival *arrival, 
		   Assoc *assoc, Locator_params *locator_params, 
		   Locator_info *locator_info, Ar_Info *ar_info, 
		   int num_obs, double torg, FILE *ofp, int verbose, 
		   int ierr)
{
	Bool	print_this_error[NUM_PRINT_MSG_ERRS];
	int	i, j, n;
	double	cur_delta, dsd_norm[3]; 
	char	ew, ns;
	char	grname[37];
	char	src_type[3];
	char	plus_or_minus[5];
	struct	delta_time_index *delta_time;
	struct	delta_time_index *head, *tail, *ptr;
   	char	str[100];
	Ar_SASC	ar_sasc;
	Arrival Na_Arrival = Na_Arrival_Init;


	/* Allocate local date and distance/time sort structures */

	delta_time = UALLOCA (struct delta_time_index, num_obs);


	fprintf (ofp, "\n ==============================================================================\n");

	if (ierr == 0)
		fprintf (ofp, "\n Location ran for%3d iterations ... Converged!\n\n", locator_params->num_iter);
	else if (ierr == 1)
		fprintf (ofp, "\n Location ran for%3d iterations ... Exhausted!\n\n", locator_params->num_iter);
	else if (ierr == 2)
		fprintf (ofp, "\n Location ran for%3d iterations ... Diverged!\n\n", locator_params->num_iter);

	greg (origin->grn, grname);
	fprintf (ofp, " EvID: %ld  In_OrID: %ld  GRN: %s\n\n", 
		      origin->evid, origin->orid, grname);

	/*
	 * Print out location information
	 */

	ns = 'N';
	ew = 'E';
	if (origin->lat < 0.0)
	    ns = 'S';
	if (origin->lon < 0.0)
	    ew = 'W';
	if (origerr->sxx <= 0.0 && origerr->syy <= 0.0) 
	{
	    fprintf (ofp, " Final location estimate:\n");
	    fprintf (ofp, "      Latitude: %13.3f deg. %c\n",
			  fabs(origin->lat), ns);
	    fprintf (ofp, "     Longitude: %13.3f deg. %c\n",
			  fabs(origin->lon), ew);
	}
	else
	{
	    fprintf (ofp, " Final location estimate (+/- S.D.):\n");
	    fprintf (ofp, "      Latitude: %13.3f deg. %c +/- %9.3f km.\n",
			  fabs(origin->lat), ns, sqrt(origerr->syy));
	    fprintf (ofp, "     Longitude: %13.3f deg. %c +/- %9.3f km.\n",
			  fabs(origin->lon), ew, sqrt(origerr->sxx));
	}

	if (origerr->szz <= 0.0)
	    fprintf (ofp, "         Depth: %13.3f  km.\n", origin->depth);
	else
	    fprintf (ofp, "         Depth: %13.3f  km.   +/- %9.3f km.\n",
			  origin->depth, sqrt(origerr->szz));

	if (origerr->stt <= 0.0)
	{
	    fprintf (ofp, " Relative O.T.: %13.3f sec.\n", torg);
	    fprintf (ofp, " Absolute O.T.: %13.3f sec.\n", origin->time);
	}
	else
	{
	    fprintf (ofp, " Relative O.T.: %13.3f sec.   +/- %9.3f sec.\n",
			  torg, sqrt(origerr->stt));
	    fprintf (ofp, " Absolute O.T.: %13.3f sec.   +/- %9.3f sec.\n",
			  origin->time, sqrt(origerr->stt));
	}

	/*
	 * Convert epoch time back to human time, and output
	 */


/*
        sprintf (buf, "%s", stdtime_format (origin->time, "%o"));

        fprintf (ofp, "              : %s %s \n", stdtime_format (origin->time, "%b %e %Y"), buf);
*/
	timeEpochToString(origin->time, str, 100, GSE21);
	fprintf (ofp, "              : %s \n", str);

	if (verbose == 1)
		return;

	/*
	 * Print out location error-related information
	 */

	fprintf (ofp, "\n Confidence region at %4.2f level:\n",
		      locator_params->conf_level);
	fprintf (ofp, "   Semi-major axis:%8.1f  km. =%6.2f deg.\n",
		 origerr->smajax, origerr->smajax*KM_TO_DEG);
	fprintf (ofp, "   Semi-minor axis:%8.1f  km. =%6.2f deg.\n",
		 origerr->sminax, origerr->sminax*KM_TO_DEG);
	fprintf (ofp, " Major-axis strike:%8.1f deg. clockwise from North\n",
		 origerr->strike);
	if (locator_params->fix_depth != TRUE)
	    fprintf (ofp, "       Depth error:%8.1f  km.\n", origerr->sdepth);
	fprintf (ofp, "  Orig. time error:%8.1f sec.\n", origerr->stime);

	fprintf (ofp, "\n Standard errors (sigma):\n");
	fprintf (ofp, "              Prior:%6.2f (%6d deg. of freedom)\n",
		      locator_params->est_std_error, locator_params->num_dof);
	fprintf (ofp, "          Posterior:%6.2f (%6d deg. of freedom)\n",
		      locator_info->sigma_hat, locator_info->tot_ndf);
	fprintf (ofp, "          Posterior:%6.2f (Normalized sample S.D.)\n\n",
		      locator_info->snssd);

	fprintf (ofp, "   RMS travel-time residual: %-.3f sec.\n", origerr->sdobs);
	fprintf (ofp, "      Maximum azimuthal GAP: %-.1f deg.\n",
		      locator_info->azimuthal_gap);
	fprintf (ofp, "   Effective rank of matrix: %-.2f\n", locator_info->rank);
	fprintf (ofp, " Condition number of matrix: %-.4g\n", locator_info->condition_number);

	if (locator_info->applied_damping > 0.0)
	    fprintf (ofp, "    Percent applied damping: %-.2f %%\n",
			  100.0*locator_info->applied_damping);
	else
	    fprintf (ofp, "  - No damping required !\n");

	/*
	 * Sort data by distance, then by time
	 */

	for (n = 0; n < num_obs; n++)
	{
	    delta_time[n].delta = assoc[n].delta;
	    delta_time[n].time  = arrival[n].time;
	    delta_time[n].index = n;
	}

	/* Sort by distance (delta) first */

	qsort (delta_time, (size_t) num_obs,
		sizeof (struct delta_time_index), sort_delta_compare);

	/* Now sort by time for deltas with the same value */

	head = ptr = delta_time;
	tail = delta_time + num_obs;
	while (head < tail)
	{
	    cur_delta = (*head).delta;
	    while ((ptr < tail) && (cur_delta == (*ptr).delta))
		ptr++;
	    if (ptr > head && ptr <= tail)
	    {
		qsort ((void *) head, (size_t) (ptr - head),
			sizeof (struct delta_time_index), sort_time_compare);
		head = ptr++;
	    }
	}

	/*
	 * Write out the final summary of results
	 */

	for (i = 0; i < NUM_PRINT_MSG_ERRS; i++)
	    print_this_error[i] = FALSE;

	fprintf (ofp, "\n ==============================================================================\n");
	fprintf (ofp, "                                 Residuals   A Priori Dist.  Azimuth  Data\n");
	fprintf (ofp, " Ariv ID Statn  Phase    Def   True   Normal   Error  (deg.)  (deg.) Import Err\n");
	fprintf (ofp, " ==============================================================================\n");

	for (n = 0; n < num_obs; n++)
	{
	    j = (int)(delta_time[n].index);

	    /*  Calculate normalized residual */

	    if (assoc[j].timeres != -999.0)
		dsd_norm[0] = assoc[j].timeres*assoc[j].wgt;
	    else
		dsd_norm[0] = -999.0;
	    if (assoc[j].azres != -999.0)
		dsd_norm[1] = assoc[j].azres/arrival[j].delaz;
	    else
		dsd_norm[1] = -999.0;
	    if (assoc[j].slores != -999.0)
		dsd_norm[2] = assoc[j].slores/arrival[j].delslo;
	    else
		dsd_norm[2] = -999.0;

	    if (verbose > 2 && ar_info[j].tt_table_value > 0.0)
	    {
		if (ar_info[j].src_dpnt_corr_type == NO_SRC_DPNT_CORR)
		    strcpy (src_type, "");
		else if (ar_info[j].src_dpnt_corr_type == SSSC_LEVEL_1_CORR)
		    strcpy (src_type, "SS");
		else if (ar_info[j].src_dpnt_corr_type == SSSC_LEVEL_2_CORR)
		    strcpy (src_type, "S2");
		else if (ar_info[j].src_dpnt_corr_type == TEST_SITE_CORR)
		    strcpy (src_type, "TS");
		else if (ar_info[j].src_dpnt_corr_type == SRST_CORR)
		    strcpy (src_type, "SR");
		else
		    fprintf (stderr, "Warning: Unknown SRC_CORR_TYPE: %ld !\n",
				ar_info[j].src_dpnt_corr_type);

		strcpy (plus_or_minus, "++++");
		if (ar_info[j].ellip_corr < 0.0)
		    plus_or_minus[0] = '-';
		if (ar_info[j].elev_corr < 0.0)
		    plus_or_minus[1] = '-';
		if (ar_info[j].bulk_static_sta_corr < 0.0)
		    plus_or_minus[2] = '-';
		if (ar_info[j].src_dpnt_corr < 0.0)
		    plus_or_minus[3] = '-';

		fprintf (ofp, "(Vel. Model  Total TT = Table   + Ellip +  Elev +  Bulk + SRC_CORR; Model  Meas)\n");
		fprintf (ofp, "(%-12s%8.3f =%8.3f %c%6.3f %c%6.3f %c%6.3f %c%6.3f%-2s ;%6.2f%6.2f)\n",
			ar_info[j].vmodel, ar_info[j].total_travel_time, 
			ar_info[j].tt_table_value,
			plus_or_minus[0], fabs(ar_info[j].ellip_corr),
			plus_or_minus[1], fabs(ar_info[j].elev_corr),
			plus_or_minus[2], fabs(ar_info[j].bulk_static_sta_corr),
			plus_or_minus[3], fabs(ar_info[j].src_dpnt_corr), 
			src_type, ar_info[j].model_error, 
			ar_info[j].meas_error);
	    }

	    /* Print one arrival on each line */

	    fprintf (ofp, "%8ld %-6s %-7s", assoc[j].arid, assoc[j].sta, 
			  assoc[j].phase); 
	    fprintf (ofp, "t  %s%9.3f%9.3f%7.3f%8.3f%7.2f%8.3f%3ld\n",
			  assoc[j].timedef, assoc[j].timeres, dsd_norm[0],
			  ar_info[j].model_plus_meas_error, assoc[j].delta, 
			  assoc[j].esaz, ar_info[j].time_import,
			  ar_info[j].time_error_code);

	    ar_sasc = get_ar_sasc (assoc[j].arid);
	    if (VALID_SLOW(arrival[j].slow) && VALID_SEAZ(arrival[j].azimuth) && ar_sasc.arid != Na_Arrival.arid)
	    {
		fprintf (ofp, "Slow_Vec_Res");
		fprintf (ofp, "  Az_Res = Raw_Az - AzCor - SEAZ");
		if (fabs(assoc[j].slores) < 999.)
		    fprintf (ofp, "    Sl_Res = Raw_Sl - SlCor - HSlow\n");
		else
		    fprintf (ofp, "\n");
		fprintf (ofp, "%9.3f", ar_info[j].slow_vec_res);
		fprintf (ofp, "  %9.3f%9.3f%8.3f%8.3f",
			 assoc[j].azres, ar_sasc.raw_azimuth, 
			 ar_sasc.azimuth_corr, assoc[j].seaz);
		if (fabs(assoc[j].slores) < 999.)
		    fprintf (ofp, "%9.3f%9.3f%8.3f%9.3f\n",
			     assoc[j].slores, ar_sasc.raw_slow, ar_sasc.slow_corr, 
			     ar_sasc.raw_slow - ar_sasc.slow_corr - assoc[j].slores);
		else
		    fprintf (ofp, "\n");
		fprintf (ofp, "%8ld %-6s %-7s", 
			 assoc[j].arid, assoc[j].sta, assoc[j].phase); 
		fprintf (ofp, "s  %s%9.3f%9.3f%7.3f               %8.3f%3ld\n",
			 assoc[j].slodef, assoc[j].slores, dsd_norm[2],
			 arrival[j].delslo, ar_info[j].slow_import,
			 ar_info[j].slow_error_code);
		fprintf (ofp, "%8ld %-6s %-7s", 
			 assoc[j].arid, assoc[j].sta, assoc[j].phase); 
		fprintf (ofp, "a  %s%9.3f%9.3f%7.3f               %8.3f%3ld\n",
			 assoc[j].azdef, assoc[j].azres, dsd_norm[1],
			 arrival[j].delaz, ar_info[j].az_import,
			 ar_info[j].az_error_code);
	    }

	    if (ar_info[j].time_error_code > 0)
		print_this_error[ar_info[j].time_error_code] = TRUE;
	    if (ar_info[j].slow_error_code > 0)
		print_this_error[ar_info[j].slow_error_code] = TRUE;
	    if (ar_info[j].az_error_code > 0)
		print_this_error[ar_info[j].az_error_code] = TRUE;
	}


	/*
	 * Print out pertainent error messages
	 */

	fprintf (ofp, " ==============================================================================\n");
	fprintf (ofp, " =  0, No problem, normal interpolation\n");

	if (print_this_error[1])
	    fprintf (ofp, " =  1, No station information\n");
	if (print_this_error[2])
	    fprintf (ofp, " =  2, No travel-time tables\n");
	if (print_this_error[3])
	    fprintf (ofp, " =  3, Data type unknown\n");
	if (print_this_error[4])
	    fprintf (ofp, " =  4, Data standard error <= 0.0\n");
	if (print_this_error[5])
	    fprintf (ofp, " =  5, Data residual too large for datum\n");
	if (print_this_error[6])
	    fprintf (ofp, " =  6, Only data w/ corrections can be used in this location\n");
	if (print_this_error[11])
	    fprintf (ofp, " =  8, Travel time too far off of table to extrapolate\n");
	if (print_this_error[11])
	    fprintf (ofp, " = 11, Distance-depth point (x0,z0) in hole of travel-time curve\n");
	if (print_this_error[12])
	    fprintf (ofp, " = 12, x0 < x(1)\n");
	if (print_this_error[13])
	    fprintf (ofp, " = 13, x0 > x(max)\n");
	if (print_this_error[14])
	    fprintf (ofp, " = 14, z0 < z(1)\n");
	if (print_this_error[15])
	    fprintf (ofp, " = 15, z0 > z(max)\n");
	if (print_this_error[16])
	    fprintf (ofp, " = 16, x0 < x(1) & z0 < z(1)\n");
	if (print_this_error[17])
	    fprintf (ofp, " = 17, x0 > x(max) & z0 < z(1)\n");
	if (print_this_error[18])
	    fprintf (ofp, " = 18, x0 < x(1) & z0 > z(max)\n");
	if (print_this_error[19])
	    fprintf (ofp, " = 19, x0 > x(max) & z0 > z(max)\n");
	if (print_this_error[20])
	    fprintf (ofp, " = 20, radial_2D tables show path blocked.\n");

	return;
}


static	int
sort_delta_compare (const void *elm1, const void *elm2)
{
	struct	delta_time_index	*s1 = (struct delta_time_index *) elm1;
	struct	delta_time_index	*s2 = (struct delta_time_index *) elm2;

	if (s1 == NULL || s2 == NULL)
	    return (0);

	if (s1->delta > s2->delta)
	    return (1);
	if (s1->delta < s2->delta)
	    return (-1);
	else
	    return (0);		/* Two samples are the same */
}


static	int
sort_time_compare (const void *elm1, const void *elm2)
{
	struct	delta_time_index	*s1 = (struct delta_time_index *) elm1;
	struct	delta_time_index	*s2 = (struct delta_time_index *) elm2;

	if (s1 == NULL || s2 == NULL)
	    return (0);

	if (s1->time > s2->time)
	    return (1);
	if (s1->time < s2->time)
	    return (-1);
	else
	    return (0);		/* Two samples are the same */
}


