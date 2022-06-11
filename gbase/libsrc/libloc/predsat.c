/*
 * Copyright (c) 1990-1997 Science Applications International Corporation.

 * NAME
 *	predsat -- Predict Slowness, Azimuth and Travel-time bounds.

 * FILE    
 *	predsat.c
 
 * SYNOPSIS
 *	Computes maximun and minimum bounds on the travel-time, slowness, 
 *	and azimuth for a set of data (station-phase-datatype) based on 
 *	an event location, confidence ellipse and travel-time curves.
 
 * DESCRIPTION
 *	Function.  Information on travel-time tables, stations, event 
 *	and confidence parameters, the desired stations and phases are 
 *	passed to and from PredSAT via the argument list.  The phase and 
 *	station names given for each datum (dstaid,dwavid) must match in 
 *	the phase and station lists (staid,phase_type).  If all that you desire 
 *	are the predictions from a point, just input 0.0 for all of the 
 *	confidence bounds parameters.  

 *	---- Indexing ----
 *	i = 0, num_sites-1;	n = 0, ndata-1;

 *	---- On entry ----
 *	num_sites:	Number of stations

 *	out_file:	Output file name
 *	data_sta_id:	Name of station for which prediction to be made  
 *	data_phase_type:Name of phase for which prediction to be made

 *	slow_flag:	Compute slowness bounds (y or n)
 *	verbose:	Verbose printout (y or n)

 *	---- On return ----
 *	dist_min:	Minimum distance from the ellipse to the station (deg)
 *	dist_max:	Maximum distance from the ellipse to the station (deg)
 *	dist_center:	Distance from the center of ellipse to the station (deg)
 *	tt_min:  	Minimum travel-time for data_phase_type from the
 *			ellipse to the station (sec)
 *	tt_max:  	Maximum travel-time for data_phase_type from the
 *			ellipse to the station (sec)
 *	tt_center: 	Travel-time for data_phase_type from the center of the 
 *			ellipse to the station (sec)
 *	az_min:  	Minimum azimuth from event to the to the station (deg)
 *	az_max:  	Maximum azimuth from data_sta_id ellipse (deg)
 *	az_center: 	Maximum azimuth from data_sta_id to the center of the 
 *			ellipse (deg)
 *	slow_min: 	Minimum slowness for data_phase_type from the 
 *			ellipse to the station (sec/deg)
 *	slow_max: 	Maximum slowness for data_phase_type from the ellipse 
 *			to the station (sec/deg)
 *	slow_center:	Slowness for data_phase_type from the center of the
 *			ellipse to the station (sec/deg)
 *	tt_err_min, tt_err_max, tt_err_center: Error codes for travel-times 
 *	slow_err_min, slow_err_max, slow_err_center: Error codes for slownesses
 *			  =  0,	No problem, normal prediction
 *			  =  1,	No station information for datum
 *			  =  2,	No travel-time tables for datum
 *			  =  3, Data type unknown
 *			  =  4, S.D <= 0.0 for datum
 *			  =  5, Data residual too large for datum
 *			  =  6, Only datum w SRST correction are
 *				included in this location
 *			  = 11,	Interpolation point in hole of travel-time curve
 *			  = 12,	Interpolation point less than first distance
 *				point in curve
 *			  = 13, Interpolation point greater than last 
 *				distance point in curve
 *			  = 14,	Interpolation point less than first depth 
 *				point in curve
 *			  = 15,	Interpolation point greater than last depth 
 *				point in curve
 *			  = 16,	Interpolation point less than first distance 
 *				point in curve and less than first depth 
 *				point in curve
 *			  = 17,	Interpolation point greater than last 
 *				distance point in curve and less than first 
 *				depth point in curve
 *			  = 18,	Interpolation point less than first distance 
 *				point in curve and greater than first depth 
 *				point in curve
 *			  = 19,	Interpolation point greater than last 
 *				distance point in curve and greater than 
 *				first depth point in curve

 *	---- Functions called ----
 *	Internal
 *		order_wrt_center:	Order data w.r.t. given input center 
 *					value.

 *	Local
 *		slow_calc:		Compute horizontal slownesses and 
 *					partials
 *		compute_ttime_w_corrs:	Compute travel time (sec.) with any
 *					corrections requested.

 *	From libgeog
 *		dist_azimuth:		Determine the distance between between 
 *					two lat./lon. pairs
 *		lat_lon:		Compute a second lat./lon. from first
 *					distance and an azimuth
 
 * DIAGNOSTICS
 *	Complains when input data are bad ...
 
 * FILES
 *	Files are read by calling routine.
 
 * NOTES
 *	This could be done alot better.  Anybody want to take a crack at it?
 
 * SEE ALSO
 *	Bratt and Bache (1988) Locating events with a sparse network of
 *      regional arrays, BSSA, 78, 780-798.
 
 * AUTHORS
 *	Steve Bratt, 12/88,	Created.
 *	Walter Nagy,  5/91,	Major enhancement.  Converted to C.
 *	Walter Nagy,  9/26/92,	float arguments promoted to doubles.
 *	Walter Nagy, 10/ 5/93,	Now if value for slowness or travel-time is 
 *				< minimum or > maximum value, those extremes 
 *				will be set equal to center value.
 *	Walter Nagy, 10/ 8/93,	Added locator_params controls to apply 
 *				T-T corrections.
 *      RLB           3/14/97   
 */
static void order_wrt_center (double *array_to_order, int *index_array,
		double *center_value);


#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libloc.h"
#include "libgeog.h"
#include "loc_defs.h"

#define MAX_SLOW_BOUND 1000

void
predsat (Locator_params *locator_params, Site *sites, int num_sites, 
	 Origin *origin, Origerr *origerr, char *data_sta_id,
	 char *data_phase_type, int num_data, int slow_flag, int verbose, 
	 char *out_file, double *dist_min, double *dist_max, 
	 double *dist_center, double *tt_min, double *tt_max, 
	 double *tt_center, double *az_min, double *az_max, double *az_center, 
	 double *slow_min, double *slow_max, double *slow_center, 
	 int *tt_err_min, int *tt_err_max, int *tt_err_center, 
	 int *slow_err_min, int *slow_err_max, int *slow_err_center)
{

	FILE	*ofp = NULL;
	Bool	station_found = FALSE;
	int	i, indx[3], n, num_times;
	int	lprt[20];
	int	sta_index;
	int	spm_index;
	double	angle, az_diff, azim[2], azimuth, az_neg, az_pos;
	double	back_azimuth, dist[3], distance, e, epj, epn, epr, epr_add;
	double	esaz[3], ev_lat, ev_lon, plat[4], plon[4];
	double	sl_calc[3], sta_lat, sta_lon;
	double	travel_time, tt_calc[3], x, x1, xp, y, y1, yp, z[3];
	double	horiz_slow;
	Ar_Info	ar_info[3];

	tt_calc[0] = -999.0; tt_calc[1]	= -999.0; tt_calc[2] = -999.0;
	sl_calc[0] = -999.0; sl_calc[1]	= -999.0; sl_calc[2] = -999.0;
	azim[0]    =  999.0; azim[1]    = -999.0; 
	dist[1]    =  999.0; dist[2]    = -999.0;
	*dist_min  = -999.0; *dist_max  = -999.0;
	*tt_min    = -999.0; *tt_max    = -999.0;
	*az_min    = -999.0; *az_max    = -999.0;
	*slow_min  = -999.0; *slow_max  = -999.0;
	*tt_center = -999.0; *slow_center = -999.0;
	az_neg     =  999.0; az_pos = -999.0;

	n  = 0;
	ev_lat = (double) origin->lat;
	ev_lon = (double) origin->lon;
	set_epoch_travel_time(origin->time);

	/* Open output file */
 
	if (verbose)
	    ofp = fopen (out_file, "a");

	/*
	 * Check that datum's station and phase are valid. 
	 * If so, assign a pointer!
	 */

	for (i = 0; i < num_sites; i++)
	{
	    if (! strcmp (data_sta_id, sites[i].sta))
	    {
		station_found = TRUE;
		break;
	    }
	}

	if (station_found)
	{
	    sta_index = i;

	    if (get_tt_indexes (data_phase_type, sta_index, &spm_index) < 0)
	    {
		if (verbose)
		    fprintf (ofp, "? PredSAT: Phase %.8s unknown\n",
				     data_phase_type);
		else
		    fprintf (stderr, "? PredSAT: Phase %.8s unknown\n",
				     data_phase_type);

		ar_info[0].time_error_code = 2;
		ar_info[1].time_error_code = 2;
		ar_info[2].time_error_code = 2;
	    }
	    else
	    {
		/* Compute distance and azimuth between station and event */

		sta_lat = sites[sta_index].lat;
		sta_lon = sites[sta_index].lon;
		dist_azimuth (sta_lat, sta_lon, ev_lat, ev_lon, dist_center, 
			      az_center, &back_azimuth, 0);

		dist[0] = *dist_center;
		esaz[0] = back_azimuth;
		z[0]    = origin->depth;

		/*
		 * If no confidence ellipse,
		 * compute travel-time from station to point
		 */
 
		epj = origerr->smajax/DEG_TO_KM;
		epn = origerr->sminax/DEG_TO_KM;
		epr = origerr->strike;
		num_times = 3;

		if (origerr->smajax <= 0.0 || origerr->sminax <= 0.0)
		    num_times = 1;

		/*
		 * If this is a hydroacoustic phase, check for blockage and
		 * possibility that delta > 180 deg.  If the latter case
		 * is TRUE, then adjust distance, azimuth and back azimuth.
		 */

		else if (   STREQ (data_phase_type, "H") 
			 || STREQ (data_phase_type, "T")
			 || STREQ (data_phase_type, "HT") 
			 || STREQ (data_phase_type, "HO"))
		{
		    blocked_ellipse_dist (data_sta_id, ev_lat, ev_lon, 
					  origerr->smajax, origerr->sminax, 
					  origerr->strike, dist_center,
					  dist_min, dist_max, az_min, az_max);
		    if (*dist_center > 180.0 || 
			(*dist_min > 180.0 && *dist_max > 180.0))
		    {
			*dist_center = 360.0 - dist[0];
			*az_center += 180.0;
			back_azimuth += 180.0;
			if (*az_center > 360.0)
			    *az_center -= 360.0;
			if (back_azimuth > 360.0)
			    back_azimuth -= 360.0;
		    }
/* 
RLB 3/14/97 No need to change dist_center. In the case the center of the ellipse is blocked
and the ellipse is partially unblocked, dist_center = -1 was returned.
		    dist[0] = *dist_center;
*/
/* 
If center was blocked, fall back onto previously computed distance.
*/
		    if(*dist_center < 0) *dist_center = dist[0] ;
		    dist[1] = *dist_min;
		    dist[2] = *dist_max;
		    azim[0] = *az_min;
		    azim[1] = *az_max;
		    esaz[0] = back_azimuth;
		    esaz[1] = back_azimuth;
		    esaz[2] = back_azimuth;
		}
		else
		{
		    /*
		     * Find points on confidence ellipse that are the minimum 
		     * and maximum distance from the station.  Assume that 
		     * these points are at one of the tips of the axes of 
		     * the confidence ellipse.
		     */

		    lat_lon (ev_lat, ev_lon, epj, epr, &plat[0], &plon[0]);
		    epr_add = epr + 180.0;
		    lat_lon (ev_lat, ev_lon, epj, epr_add, &plat[1], &plon[1]);
		    epr_add = epr + 90.0;
		    lat_lon (ev_lat, ev_lon, epn, epr_add, &plat[2], &plon[2]);
		    epr_add = epr + 270.0;
		    lat_lon (ev_lat, ev_lon, epn, epr_add, &plat[3], &plon[3]);

		    for (n = 0; n < 4; n++)
		    {
			dist_azimuth (sta_lat, sta_lon, plat[n], plon[n],
				      &distance, &azimuth, &back_azimuth, 0);

			/* Find min/max distance with associated azimuth */

			if (distance < dist[1])
			{
			    dist[1] = distance;
			    esaz[1] = back_azimuth;
			}
			if (distance > dist[2])
			{
			    dist[2] = distance;
			    esaz[2] = back_azimuth;
			}

			/* Find minimum and maximum azimuth */

			az_diff = azimuth - *az_center;
			if (fabs(az_diff) > 180.0) 
			    az_diff = SIGN(360.0-fabs(az_diff), az_diff);
			if (az_diff < az_neg)
			{
			    azim[0] = azimuth;
			    az_neg  = az_diff;
			}
			else if (az_diff > az_pos)
			{
			    azim[1] = azimuth;
			    az_pos  = az_diff;
			}
		    }
		}
 
		if (num_times > 1)
		{
		    /*
		     * If station is within ellipse, set minimum distance
		     * to 0.0 and azimuth bounds to 0.0 and 360.0 degrees
		     */

		    if (*az_center >= 0.0 && *az_center <= 90.0)
			angle = 90.0 - *az_center;
		    else if (*az_center >= 90.0 && *az_center <= 180.0)
			angle = 180.0 + *az_center;
		    else if (*az_center >= 180.0 && *az_center <= 270.0)
			angle = *az_center;
		    else
			angle = *az_center - 180.0;

		    x = *dist_center*DEG_TO_KM*cos(angle*DEG_TO_RAD);
		    y = *dist_center*DEG_TO_KM*sin(angle*DEG_TO_RAD);

		    /*
		     * Rotate into ellipsoid coordinate system and check 
		     * if station is within ellipse
		     */

		    angle = 90.0 - origerr->strike;
		    xp    = x*cos(angle) + y*sin(angle);
		    yp    = -x*sin(angle) + y*cos(angle);
		    x1    = xp/(origerr->smajax);
		    y1    = yp/(origerr->sminax);
		    e     = x1*x1 + y1*y1;
		    if (e <= 1.0)
		    {
			dist[1] = 0.0;
			azim[0] = 0.0;
			azim[1] = 360.0;
		    }

		    /* 
		     * Minimum time path will be the deepest epicenter for all 
		     * but (a) depth phases, where the shallower depth will 
		     * produce earlier arrivals, and (b) phases from events 
		     * who's epicenter-station distance is less than its depth.
		     */

		    if (origerr->sdepth < 0.0)
		    {
			z[1] = origin->depth;
			z[2] = origin->depth;
		    }
		    else
		    {
			if (! strncmp (data_phase_type, "p", 1) || 
			    ! strncmp (data_phase_type, "s", 1) || 
			    (dist[1]*DEG_TO_KM < origin->depth))
			{
			    z[1] = origin->depth - origerr->sdepth;
			    z[2] = origin->depth + origerr->sdepth;
			}
			else
			{
			    z[1] = origin->depth + origerr->sdepth;
			    z[2] = origin->depth - origerr->sdepth;
			}
		    }
      
		    if (z[1] < 0.0)
			z[1] = 0.0;
		    if (z[2] < 0.0)
			z[2] = 0.0;
		}

		for (n = 0; n < num_times; n++)
		{
		    travel_time = 
		    compute_ttime_w_corrs (locator_params, sites, TRUE, 
				ev_lat, ev_lon, z[n], dist[n], esaz[n], 
				data_phase_type, sta_index, &ar_info[n],
				&horiz_slow);

		    /*
		     * Use only sucessfully interpolated and extrapolated 
		     * results
		     * - Don't use if in a hole (time_error_code = 11)
		     */

		    if (ar_info[n].time_error_code == 0 && travel_time < 0.0)
		    {
			ar_info[n].time_error_code = 10;
			tt_calc[n] = -999.0;
		    }
		    else if (ar_info[n].time_error_code == 11)
			tt_calc[n] = -999.0;
		    else
			tt_calc[n] = travel_time;

		    if (slow_flag)
		    {
			if (ar_info[n].slow_error_code != 11 && 
			    horiz_slow < MAX_SLOW_BOUND )
			    sl_calc[n] = horiz_slow;
			else
			    sl_calc[n] = -999.0;
		    }
		}
	    }
	}
	else
	{
	    /* Station not found !!!! */
	    if (verbose)
		fprintf (ofp, "? PredSAT: Station %.6s unknown\n",
				 data_sta_id);
	    else
		fprintf (stderr, "? PredSAT: Station %.6s unknown\n",
				 data_sta_id);
	    ar_info[0].time_error_code = 1;
	    ar_info[1].time_error_code = 1;
	    ar_info[2].time_error_code = 1;
	}

	/*
	 * Load array data into arguments for return to calling routine
	 * The most distant point will have the smallest slowness 
	 */

	if (slow_flag)
	{
	    *slow_center = sl_calc[0];
	    *slow_err_center = ar_info[0].slow_error_code;

	    order_wrt_center (sl_calc, indx, slow_center);

	    *slow_min = sl_calc[indx[1]];
	    *slow_max = sl_calc[indx[2]];
	    *slow_err_min = ar_info[indx[1]].slow_error_code;
	    *slow_err_max = ar_info[indx[2]].slow_error_code;
	}

	*tt_center	= tt_calc[0];
	*tt_err_center	= ar_info[0].time_error_code;

	order_wrt_center (tt_calc, indx, tt_center);

	*tt_min = tt_calc[indx[1]];
	*tt_max = tt_calc[indx[2]];
	*tt_err_min = ar_info[indx[1]].time_error_code;
	*tt_err_max = ar_info[indx[2]].time_error_code;

	*az_min	  = azim[0];
	*az_max	  = azim[1];
	*dist_min = dist[1];
	*dist_max = dist[2];


	/* Print location results if desired */

	if (verbose)
	{
	    for (i = 0; i < 20; i++)
		lprt[i] = FALSE;

	    fprintf (ofp, "== EVENT INFORMATION ============================================\n");
	    fprintf (ofp, " Lat:%9.3f deg   Lon:%9.3f deg   Depth:%9.3f km\n", 
			  origin->lat, origin->lon, origin->depth);

	    fprintf (ofp, "Semi-major axis:%9.1f km   Semi-minor axis:%9.1f km\n", origerr->smajax, origerr->sminax);
             fprintf (ofp, "Major-axis strike:%7.1f deg clockwise from North\n", origerr->strike);
             fprintf (ofp, "Depth error:%8.1f km   Orig. time error:%8.1f sec\n", origerr->sdepth, origerr->stime);

	    fprintf (ofp, "== PREDICTED TIME/AZIMUTH/SLOWNESS BOUNDS =======================\n");

	    fprintf (ofp, " Arrival ID = %6d   Station = %s   Phase = %s\n", 
			  num_data, data_sta_id, data_phase_type);

	    fprintf (ofp, "Min Cntr Max Azim (deg)    : %12.2f%12.2f%12.2f\n", 
			  *az_min, *az_center, *az_max);
	    fprintf (ofp, "Min Cntr Max Dist (deg)    : %12.2f%12.2f%12.2f\n", 
			  *dist_min, *dist_center, *dist_max);
	    fprintf (ofp, "Min Cntr Max Time (sec)    : %12.2f%12.2f%12.2f\n", 
			  *tt_min, *tt_center, *tt_max);
	    fprintf (ofp, "Min Cntr Max Slow (sec/deg): %12.2f%12.2f%12.2f\n", 
			  *slow_min, *slow_center, *slow_max);
	    fprintf (ofp, "Time Errors       (sec)    : %12d%12d%12d\n", 
			 *tt_err_min, *tt_err_center, *tt_err_max);
	    fprintf (ofp, "Slowness Errors   (sec/deg): %12d%12d%12d\n", 
			 *slow_err_min, *slow_err_center, *slow_err_max);

	    if (*tt_err_min > 0)  lprt[*tt_err_min]  = TRUE;
	    if (*tt_err_center > 0) lprt[*tt_err_center] = TRUE;
	    if (*tt_err_max > 0)  lprt[*tt_err_max]  = TRUE;
	    if (*slow_err_min > 0)  lprt[*slow_err_min]  = TRUE;
	    if (*slow_err_center > 0) lprt[*slow_err_center] = TRUE;
	    if (*slow_err_max > 0)  lprt[*slow_err_max]  = TRUE;

	    fprintf (ofp, " =  0, No problem, normal interpolation\n");
	    if (lprt[1])  fprintf (ofp, " =  1, No station information\n");
	    if (lprt[2])  fprintf (ofp, " =  2, No travel-time tables\n");
	    if (lprt[3])  fprintf (ofp, " =  3, Data type unknown\n");
	    if (lprt[4])  fprintf (ofp, " =  4, S.D. <= 0.0\n");
	    if (lprt[5])  fprintf (ofp, " =  5, Data residual too large for datum\n");
	    if (lprt[6])  fprintf (ofp, " =  6, Only data w/ corrections can be used in this location\n");
	    if (lprt[11]) fprintf (ofp, " = 11, Distance-depth point '(x0,z0) in hole of travel-time curve\n");
	    if (lprt[12]) fprintf (ofp, " = 12, x0 < x(1)\n");
	    if (lprt[13]) fprintf (ofp, " = 13, x0 > x(max)\n");
	    if (lprt[14]) fprintf (ofp, " = 14, z0 < z(1)\n");
	    if (lprt[15]) fprintf (ofp, " = 15, z0 > z(max)\n");
	    if (lprt[16]) fprintf (ofp, " = 16, x0 < x(1) & z0 < z(1)\n");
	    if (lprt[17]) fprintf (ofp, " = 17, x0 > x(max) & z0 < z(1)\n");
	    if (lprt[18]) fprintf (ofp, " = 18, x0 < x(1) & z0 > z(max)\n");
	    if (lprt[19]) fprintf (ofp, " = 19, x0 > x(max) & z0 > z(max)\n");
	    fprintf (ofp, "\n");

	    fclose (ofp);
	}
}


/*
 * This function is used to order data with respect to (wrt) the given input
 * center value.  Here this must always be index 0 (indx[0]) upon return.
 * If one of the other index values is < or > the center value, its value
 * will be set equal to the center value.
 */

static void
order_wrt_center (double *array_to_order, int *index_array,
		double *center_value)
{

	index_array[0] = 0;		/* Center index is always 0 */
	*center_value = array_to_order[0];

	if (array_to_order[1] > array_to_order[2])
	{
	    if (array_to_order[1] > *center_value)
		index_array[2] = 1;
	    else
		index_array[2] = 0;
	    if (array_to_order[2] < *center_value)
		index_array[1] = 2;
	    else
		index_array[1] = 0;
	}
	else
	{
	    if (array_to_order[1] < *center_value)
		index_array[1] = 1;
	    else
		index_array[1] = 0;
	    if (array_to_order[2] > *center_value)
		index_array[2] = 2;
	    else
		index_array[2] = 0;
	}
}

