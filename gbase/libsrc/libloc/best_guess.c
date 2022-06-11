
/*
 * Copyright (c) 1990-1996 Science Applications International Corporation.
 *

 * NAME
 *	best_guess -- Make an initial location guess or prediction.

 * FILE
 *	best_guess.c

 * SYNOPSIS
 *	int
 *	best_guess (sites, num_sites, w_arrival, w_assoc, ar_info, 
 *		    num_obs, locator_info, ofp, verbose, lat_init, lon_init)
 *	Site	*sites;			(i) Station structure
 *	int	num_sites;		(i) Number of stations in sites table
 *	Arrival	*w_arrival;		(i) Local CSS DB3.0 arrival table
 *	Assoc	*w_assoc;		(i) Local CSS DB3.0 assoc table
 *	Ar_Info *ar_info; 		(i) Arrival-based location info
 *	int	num_obs;		(i) Number of arrival/assoc records
 *	Locator_info   *locator_info;	(i) Local locator info structure
 *	FILE	*ofp;			(i) Output file pointer;
 *	int	verbose;		(i) Level of verbose output:
 *					    0:	No output
 *					    1:	Print only final location info
 *					    2:	Print final location and 
 *						arrival info
 *					    3:	Do not print intermediate 
 *						iterations & input station info
 *					    4:	Print all output 
 *	double	*lat_init;		(o) Best-guess (initial) latitude (deg)
 *	double	*lon_init;		(o) Best-guess (initial) longitude (deg)

 * DESCRIPTION
 *	Function.  This function takes a first-cut at the epicentral
 *	location using one of the following techniques, in the order 
 *	tried,

 *	  1.	Attempts to compute an initial location using S-P times 
 *	      	for the closest station and the best-determined defining 
 *		azimuth for that station (based on the smallest S.D.).
 *	  2.	Attempts to compute an intial location from various combi-
 *	  	nations of S-P times and P-wave arrival times.  The 
 *		searches are preformed in the following order of 
 *		importance:
 *	  	  A.	Uses the 3 least S-P times and finds a common 
 *			crossing point.
 *	  	  B.	Uses the smallest P-wave travel time between the 
 *			common crossing points of 2 S-P times.
 *	  	  C.	Uses a single S-P time to obtain a crude origin 
 *			time and then find the nearest crossing points 
 *			from two P-wave arrival times.
 *	  3.	Attempts to compute an initial location from various combi-
 *		nations of azimuth and P-wave arrival time data.  When the
 *		difference between azimuths is < 10 deg., then the inter-
 *		section will be poorly determined, and is therefore, 
 *		ignored.  The searches are preformed in the following order 
 *		of importance:
 *		  A.	Computes the intersection of 2 great circles given 
 *			2 stations with azimuth data from those points.
 *			The location formed using the best-defining 
 *			azimuths from those stations closest to the 
 *			location is then chosen.  
 *		  B.	Minimizes two P-wave arrival times constrained by a 
 *			single azimuth datum, preferably one with an 
 *			arrival time.
 *	  4.	Attempts to obtain an initial epicentral location based on
 *		an approximate minimization procedure using three or more 
 *		P-wave arrival times as data.
 *	  5.	Attempts to compute an initial location using a P slowness 
 *		and the best-determined azimuth datum at the station with 
 *		the P-slowness with the smallest standard error between some 
 *		bounds.
 *	  6.	Looks for the closest station based of arrival times and 
 *		uses an associated azimuth (if available).  
 *	  7.	Looks for a station with a good slowness (based on the
 *		slowness datum with the smallest a priori data standard 
 *		error) and places the initial location near that station.
 *	  8.	It looks for a station with an azimuth and places the 
 *		initial location near that station at the associated 
 *		azimuth. 

 *	---- Functions called ----
 *	From libgeog
 *		azimuth_cross_pt:	Determine crossing points of 2 great 
					circles 
 *		small_circle_cross_pts:	Determine crossing points of 2 small 
					circles
 *		dist_azimuth:		Determine the distance between between 
 *					2 lat./lon. pairs
 *		lat_lon:		Compute a second lat./lon. from the 
 *					1st, a distance and an azimuth

 *	From libloc
 *		crude_but_quick_trv_time: Make a crude travel-time determination
 *					  from a given distance
 *		crude_but_quick_dist:	  Make a crude distance determination 
 *					  from a given travel time

 *	Internal
 *		order_data_asc:	Re-order a data array in ascending order
 *		print_guess:	Just print out the guess solution

 * DIAGNOSTICS
 *	Returns if it cannot find an initial educated guess given the 
 *	above search conditions.

 * NOTES
 *	Currently under-going substanitive additions.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy, July 1990.
 *	Converted to C by Walter Nagy, August 1992.
 *	Now passes db structures, Walter Nagy,	July 1994.
 */


#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libgeog.h"
#include "libloc.h"
#include "locp.h"

static void print_guess(	double	*alat,
					double	*alon,
					FILE	*ofp);

static void order_data_asc(	int	index,
					int	*array_index,
					double	*order_array);

#define	MAX_SMINUSP	12	/* Maximum number of S minus P elements */
#define	MAX_DIST	19	/* Maximum number of permissable elements 
				   for distance(), slowness() arrays */
#define	DEFAULT_DIST	0.01	/* Default distance (deg.) from the 
				   "guessed" event to the station */
#define	DEFAULT_AZIM	0.0	/* Default azimuth (deg.) from the 
				   "guessed" event to the station */
#define	FMAXAZCROSS	80.0	/* Maximum cross-azimuth setting (deg.) 
				   for initialization purposes */
#define	FMIN_SLOW	0.02	/* Minimum allowable slowness */
#define	FMAX_SLOW	19.16	/* Maximum allowable slowness */


int
best_guess (Site *sites, int num_sites, Arrival *w_arrival, Assoc *w_assoc, 
	    Ar_Info *ar_info, int num_obs, Locator_info *locator_info, 
	    FILE *ofp, int verbose, double *lat_init, double *lon_init)
{

	int	ierr = 0;
	Bool	first_azim, *goodazim, *goodcompr, *goodslow, *goodsminusp;
	int	*indexcompr, *indexdsd, *indexdsd2, *indexsminusp, *iwave;
	int	i, i1, i1s, i2, i2s, i3s, iazim, ic2, icerr, icompr, icross;
	int	ierrx, islow, isminusp, iusesta, j, j1, j2, k;
	int	n, n1, n2;
	double	*azimsd, *bestazim, *bestslow, *comprtime, *ordercompr;
	double	*orderdsd, *orderdsd2, *ordersminusp, *sheartime, *slowsd;
	double	alat0x, alon0x, azi, azisav, baz, bestazcross;
	double	*crosslat, *crosslon, delcross, delta, *dis;
	double	dist1, dist1x, dist2, dist2x, fmaxtime, res1, res2;
	double	sminusptime, smallest, tcalc, tmp;
	double	torg = 0.0, useazim = 0.0, useslow = 0.0;


	/*
	 * Slowness and S-P time as a function of distance for a surface event.
	 * Taken from IASPEI Seismological Tables of B.L.N. Kennett (1991).
	 */

	static	double	distance[] = {
		  0.0,  10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0,  80.0,
		 90.0, 100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 170.0,
		180.0 };
	static	double	slowness[] = {
		 19.17, 13.70, 10.90, 8.85, 8.30, 7.60, 6.88, 6.15, 5.40,
		  4.66,  4.44,  1.96, 1.91, 1.88, 1.79, 1.57, 1.14, 0.59,
		  0.01 };
	static	double	sminusp[] = {
		  0.00, 114.20, 226.76, 300.00, 367.49, 432.64,
		494.45, 552.32, 605.82, 654.47, 695.75, 734.60 };


	/* Allocate locally necessary memory */

	azimsd		= UALLOCA (double, num_sites);
	slowsd		= UALLOCA (double, num_sites);
	bestazim	= UALLOCA (double, num_sites);
	bestslow	= UALLOCA (double, num_sites);
	comprtime	= UALLOCA (double, num_sites);
	sheartime	= UALLOCA (double, num_sites);
	goodcompr	= UALLOCA (Bool, num_sites);
	goodsminusp	= UALLOCA (Bool, num_sites);
	goodazim	= UALLOCA (Bool, num_sites);
	goodslow	= UALLOCA (Bool, num_sites);
	indexcompr	= UALLOCA (int, num_sites);
	indexsminusp	= UALLOCA (int, num_sites);
	indexdsd	= UALLOCA (int, num_sites);
	indexdsd2	= UALLOCA (int, num_sites);
	iwave		= UALLOCA (int, num_sites);
	ordercompr	= UALLOCA (double, num_sites);
	ordersminusp	= UALLOCA (double, num_sites);
	orderdsd	= UALLOCA (double, num_sites);
	orderdsd2	= UALLOCA (double, num_sites);


	if (verbose > 2) 
	{
	    fprintf (ofp, " --------------------------------------------------\n");
	    fprintf (ofp, " best_guess: Initial location information:\n");
	    fprintf (ofp, " --------------------------------------------------\n");
	}

	/* Local initializations */

	iusesta		= 0;
	sminusptime	= 690.0;
	fmaxtime	= 888888.0;
	bestazcross	= FMAXAZCROSS;
	first_azim	= TRUE;

	for (i = 0; i < num_sites; i++)
	{
	    azimsd[i]      = 40.0;
	    slowsd[i]      = FMAX_SLOW;
	    bestazim[i]    = -fmaxtime;
	    bestslow[i]    = -fmaxtime;
	    goodcompr[i]   = FALSE;
	    goodsminusp[i] = FALSE;
	    goodazim[i]	   = FALSE;
	    goodslow[i]	   = FALSE;
	    comprtime[i]   = fmaxtime;
	    sheartime[i]   = fmaxtime;
	}

	/*
	 * Load valid P-wave and S-wave arrival times, P-slownesses and 
	 * azimuths (use P-type phase or phase with the smallest azimuth 
	 * standard deviation) for each arrival/assoc record.
	 */

	for (n = 0; n < num_obs; n++)
	{
	    i = locator_info->sta_index[n];

	    /* ---- Arrival Times ---- */
	    if (STREQ (w_assoc[n].timedef, "d") && 
		ar_info[n].time_error_code == 0)
	    {
		/*
		 * Load valid S and P arrival times into arrays 
		 */

		if ((STREQ (w_assoc[n].phase, "P")  ||
		     STREQ (w_assoc[n].phase, "Pn") ||
		     STREQ (w_assoc[n].phase, "Pg") ||
		     STREQ (w_assoc[n].phase, "Pb")) &&
			    w_arrival[n].time < comprtime[i])
		{
		    comprtime[i] = w_arrival[n].time;
		    iwave[i] = locator_info->phase_index[n];
		}
		else if ((STREQ (w_assoc[n].phase, "S")  ||
			  STREQ (w_assoc[n].phase, "Sn") ||
			  STREQ (w_assoc[n].phase, "Sg") ||
			  STREQ (w_assoc[n].phase, "Lg") ||
			  STREQ (w_assoc[n].phase, "Sb")) &&
				 w_arrival[n].time < sheartime[i])
		{
		    sheartime[i] = w_arrival[n].time;
		}
	    }

	    /* ---- Azimuths ---- */
	    if (STREQ (w_assoc[n].azdef, "d") && ar_info[n].az_error_code == 0)
	    {
		/*
		 * Always save at least one azimuth datum, just in 
		 * case the event only contains a single time, 
		 * single azimuth and single slowness.
		 */

		if (first_azim == TRUE || w_arrival[n].delaz < azimsd[i])
		{
		    azimsd[i]   = w_arrival[n].delaz;
		    bestazim[i] = w_arrival[n].azimuth;
		    first_azim  = FALSE;
		}
	    }

	    /* ---- Slownesses ---- */
	    if (STREQ (w_assoc[n].slodef, "d") && 
		ar_info[n].slow_error_code == 0)
	    {
		if (! strncmp (w_assoc[n].phase, "P", 1)
		    && w_arrival[n].delslo > 0.0
		    && w_arrival[n].slow > FMIN_SLOW
		    && w_arrival[n].slow < FMAX_SLOW)
		{
		    if (w_arrival[n].delslo < slowsd[i])
		    {
			slowsd[i]   = w_arrival[n].delslo;
			bestslow[i] = w_arrival[n].slow;
		    }
		}
	    }
	}

	/*
	 * Define good (valid) P-wave and S-wave arrival times, S-P times, 
	 * azimuths and slownesses.  Load these valid data in the order() 
	 * array and save their station index in the index() array.
	 */

	icompr = 0; isminusp = 0; iazim = 0; islow = 0;
	for (i = 0; i < num_sites; i++)
	{
	    if (comprtime[i] < fmaxtime)
		goodcompr[i] = TRUE;
	    if (goodcompr[i])
	    {
		indexcompr[icompr] = i;
		ordercompr[icompr] = comprtime[i];
		++icompr;
	    }

	    if (goodcompr[i] && (sheartime[i] <  fmaxtime) &&
		   (sheartime[i] > comprtime[i]))
		goodsminusp[i] = TRUE;
	    if (goodsminusp[i])
	    {
		indexsminusp[isminusp] = i;
		ordersminusp[isminusp] = sheartime[i] - comprtime[i];
		++isminusp;
	    }

	    if (bestazim[i] >= -180.0 && bestazim[i] <= 360.0)
		goodazim[i] = TRUE;
	    if (goodazim[i])
	    {
		indexdsd[iazim] = i;
		orderdsd[iazim] = azimsd[i];
		++iazim;
	    }

	    if (bestslow[i] > FMIN_SLOW && bestslow[i] < FMAX_SLOW)
		goodslow[i] = TRUE;
	    if (goodslow[i])
	    {
		indexdsd2[islow] = i;
		orderdsd2[islow] = slowsd[i];
		++islow;
	    }
	}

	dis	 = UALLOCA (double, isminusp);
	crosslat = UALLOCA (double, 2*isminusp);
	crosslon = UALLOCA (double, 2*isminusp);

	/*
	 * Re-order P-wave arrival times, S-P times, azimuths and slownesses
	 * in ascending order.  For arrival time and S-P time data sort by
	 * earliest times first.  For azimuth and slowness data order by
	 * smallest standard errors first.
	 */

	if (icompr > 1)
	    order_data_asc (icompr, indexcompr, ordercompr);

	if (isminusp > 1)
	    order_data_asc (isminusp, indexsminusp, ordersminusp);

	if (iazim > 1)
	    order_data_asc (iazim, indexdsd, orderdsd);

	if (islow > 1)
	    order_data_asc (islow, indexdsd2, orderdsd2);


	/*
	 * Find closest station with smallest S-P time and an azimuth.
	 * Compute the location from the S-P time and azimuth.
	 * First and preferred search procedure !
	 */

	if (isminusp < 1) 
	    goto no_s_minus_p;

	for (i = 0; i < isminusp; i++)
	{
	    n = indexsminusp[i];
	    if (goodazim[n])
	    {
		sminusptime = ordersminusp[i];

		/* Interpolate slowness to get distance */
		for (j = 0; j < MAX_SMINUSP-1; j++)
		{
		    if (sminusptime > sminusp[j] && sminusptime <= sminusp[j+1])
		    {
			dis[0] = (distance[j+1] - distance[j]) *
				 (sminusptime - sminusp[j]) /
				 (sminusp[j+1] - sminusp[j]) + distance[j];
			useazim	= bestazim[n];
			iusesta	= n;

			/* Done!  Let's find a lat/lon pair */
			lat_lon (sites[iusesta].lat, sites[iusesta].lon, 
				 dis[0], useazim, lat_init, lon_init);

			if (verbose > 2) 
			{
			    fprintf (ofp, "    Method: S-P time w/ azimuth at 1 station\n");
			    fprintf (ofp, "   Station: %-6s\n", sites[iusesta].sta);
			    fprintf (ofp, "  S-P time: %7.2f sec.\n", sminusptime);
			    fprintf (ofp, "  Distance: %7.2f deg.\n", dis[0]);
			    fprintf (ofp, "   Azimuth: %7.2f deg.\n", useazim);
			}
			goto done;
		    }
		}
	    }
	}

	/*
	 * Look here for multiple S-P times and compute distances
	 */

	if (icompr < 2) 
	    goto no_s_minus_p;

	for (i = 0; i < isminusp; i++)
	{
	    sminusptime = ordersminusp[i];

	    /* Interpolate slowness(es) to get distance(s) */
	    for (j = 0; j < MAX_SMINUSP-1; j++)
	    {
		if (sminusptime > sminusp[j] && sminusptime <= sminusp[j+1])
		    dis[i] = (distance[j+1] - distance[j]) *
			     (sminusptime - sminusp[j]) /
			     (sminusp[j+1] - sminusp[j]) + distance[j];
	    }
	}

	/*
	 * Compute the approximate origin time
	 */

	j = indexsminusp[0];
	n = iwave[j];
	if ((tcalc = crude_but_quick_trv_time (n, dis[0], 0.0)) < 0.0)
	    goto only_one_s_minus_p;
	torg = ordercompr[j] - tcalc;

	/*
	 * Determine the necessary crossing points
	 */

	icross	= 0;
	icerr	= 0;
	if (isminusp > 1)
	{
	    i1s = i2s = i3s = 0;
	    for (i1 = 1; i1 < isminusp; i1++)
	    {
		for (i2 = 0; i2 < i1; i2++)
		{
		    n1	= indexsminusp[i1];
		    n2	= indexsminusp[i2];
		    ++icross;
		    ic2	= (2*icross) - 1;
		    icerr =
		    small_circle_cross_pts (sites[n2].lat, sites[n2].lon, 
				sites[n1].lat, sites[n1].lon, dis[i2], dis[i1],
				&crosslat[ic2-1], &crosslon[ic2-1],
				&crosslat[ic2], &crosslon[ic2]);
		    if (icerr != 0)
		    {
			icross = icross - 1;
			continue;
		    }
		    if (icross > 1)
		    {
			if (n2 != i1s && n2 != i2s)
			    i2s = n2;
			else
			    i2s = n1;
		    }
		    else
		    {
			i1s = n2;
			i2s = n1;
			i3s = n1;
		    }

		    /*
		     * Find the best crossing from 3 S-P times ?
		     */

		    if (icross > 1)
		    {
			smallest = 888888.0;
			azisav = 0.0;
			for (i = 0; i < 2; i++)
			{
			    for (j = 2; j < 4; j++)
			    {
				dist_azimuth (crosslat[i], crosslon[i],
					      crosslat[j], crosslon[j], 
					      &delcross, &azi, &baz, 0);
				if (delcross < smallest)
				{
				    smallest = delcross;
				    azisav   = azi;
				    i1 = i;
				    i2 = j;
				}
			    }
			}
			smallest = 0.5*smallest;
			lat_lon (crosslat[i1], crosslon[i1], smallest, 
				 azisav, lat_init, lon_init);

			if (verbose > 2) 
			{
			    fprintf (ofp, "    Method: Nearest crossing of 3 S-P times\n");
			    fprintf (ofp, "  Stations: %-6s  %-6s  %-6s\n", sites[i1s].sta, sites[i2s].sta, sites[i3s].sta);
			    fprintf (ofp, " Distances: %7.2f%7.2f%7.2f deg.\n", dis[0], dis[1], dis[2]);

			}
			goto done;
		    }
		}
	    }

	    if (icross == 0 || icompr < 3) 
		goto only_one_s_minus_p;

	    /*
	     * Use 2 S-P times and the shortest independent arrival time
	     * to determine initial location
	     */

	    for (i = 0; i < icompr; i++)
	    {
		j = indexcompr[i];
		if (j != indexsminusp[0] && j != indexsminusp[1])
		    break;
	    }
	    n = iwave[j];

	    /* Calculate theoretical travel times */
    
	    dist_azimuth (sites[j].lat, sites[j].lon, crosslat[0], crosslon[0],
			  &dis[0], &azi, &baz, 0);
	    if ((tcalc = crude_but_quick_trv_time (n, dis[0], 0.0)) < 0.0)
		goto only_one_s_minus_p;
	    res1 = fabs(comprtime[j] - torg - tcalc);
	    dist_azimuth (sites[j].lat, sites[j].lon, crosslat[1], crosslon[1],
			  &dis[1], &azi, &baz, 0);
	    if ((tcalc = crude_but_quick_trv_time (n, dis[1], 0.0)) < 0.0)
		goto only_one_s_minus_p;
	    res2 = fabs(comprtime[j] - torg - tcalc);

	    /* Choose travel time with the smallest residual */

	    if (res1 < res2)
	    {
		*lat_init = crosslat[0];
		*lon_init = crosslon[0];
	    }
	    else
	    {
		*lat_init = crosslat[1];
		*lon_init = crosslon[1];
	    }

	    if (verbose > 2) 
	    {
		fprintf (ofp, "    Method: S-P crossings and nearest arrival time\n");
		fprintf (ofp, "  Stations: %-6s  %-6s  %-6s\n", sites[i1s].sta, sites[i2s].sta, sites[j].sta);
		fprintf (ofp, " Distances: %7.2f%7.2f deg.\n", dis[0], dis[1]);
	    }
	    goto done;
	}

	else
	{

only_one_s_minus_p:

	    /*
	     * Try to determine initial location using 1 S-P time and
	     * 2 nearest independent arrival times
	     */
	
	    icross = 0;
	    n1     = indexsminusp[0];
	    for (k = 0, i1 = 0, j1 = 0; k < icompr; k++)
	    {
		j = indexcompr[k];
		if (j == n1) 
		    continue;
		++icross;
		n	= iwave[j];

		/*
		 * Calculate distance from station to the origin,
		 * and then, find the crossing points
		 */

		tcalc = ordercompr[k] - torg;
		if ((dist1 = crude_but_quick_dist (n, tcalc)) < 0.0)
		    continue;

		icerr = 0;
		ic2   = (2*icross) - 1;
		icerr =
		small_circle_cross_pts (sites[n1].lat, sites[n1].lon,
				sites[j].lat, sites[j].lon, dis[0], dist1,
				&crosslat[ic2-1], &crosslon[ic2-1],
				&crosslat[ic2], &crosslon[ic2]);
		if (icerr == 0)
		{
		    dis[icross] = dist1;
		    if (icross > 1)
		    {
			j2 = j;
			smallest = 888888.0;
			azisav = 0.0;
			for (i = 0; i < 2; i++)
			{
			    for (j = 2; j < 4; j++)
			    {
				dist_azimuth (crosslat[i], crosslon[i],
					      crosslat[j], crosslon[j],
					      &delcross, &azi, &baz, 0);
				if (delcross < smallest)
				{
				    smallest = delcross;
				    azisav   = azi;
				    i1 = i;
				    i2 = j;
				}
			    }
			}
			smallest = 0.5*smallest;
			lat_lon (crosslat[i1], crosslon[i1], smallest, 
				 azisav, lat_init, lon_init);

			if (verbose > 2) 
			{
			    fprintf (ofp, "    Method: Nearest crossing of 1 S-P & 2 P- times\n");
			    fprintf (ofp, "  Stations: %-6s  %-6s  %-6s\n", sites[n1].sta, sites[j1].sta, sites[j2].sta);
			    fprintf (ofp, " Distances: %7.2f%7.2f%7.2f deg.\n", dis[0], dis[1], dis[2]);
			}
			goto done;
		    }
		    else
			j1 = j;
		}
		else
		    --icross;
	    }
	}


no_s_minus_p:

	/* 
	 * Try to find crossing point of 2 defining azimuths.  Take the 
	 * location * that is closest (on average) to the 2 stations
	 */
      
	if (iazim > 1)
	{
	    i1s = i2s = 0;
	    for (i1 = 1, dist1 = 0.0, dist2 = 0.0; i1 < iazim; i1++)
	    {
		for (i2 = 0; i2 < i1; i2++)
		{
		    n1 = indexdsd[i1];
		    n2 = indexdsd[i2];

		    /*
		     * If the difference between 2 azimuths is < 10 deg., then
		     * the confidence one might put put into the computed 
		     * crossing point would be uncertain, so ignore!
		     */

		    tmp = fabs(bestazim[n1] - bestazim[n2]);
		    if (tmp > 350.0) 
			tmp = 360.0 - tmp;
		    if (tmp < 10.0) 
			continue;
		    ierrx = 
		    azimuth_cross_pt (sites[n1].lat, sites[n1].lon, 
				      bestazim[n1], sites[n2].lat, 
				      sites[n2].lon, bestazim[n2], 
				      &dist1x, &dist2x, &alat0x, &alon0x);
		    if (ierrx == 0)
		    {
			delta = 0.5*(dist1x + dist2x);
			if (delta < bestazcross)
			{
			    bestazcross = delta;
			    i1s	= n1;
			    i2s	= n2;
			    dist1 = dist1x;
			    dist2 = dist2x;
			    *lat_init = alat0x;
			    *lon_init = alon0x;
			}
		    }
		}
	    }

	    if (bestazcross < FMAXAZCROSS)
	    {
		if (verbose > 2) 
		{
		    fprintf (ofp, "    Method: Crossing of azimuths from 2 stations\n");
		    fprintf (ofp, "  Stations: %-6s  %-6s\n", sites[i1s].sta, sites[i2s].sta);
		    fprintf (ofp, "  Azimuths: %7.2f  %7.2f deg.\n", bestazim[i1s], bestazim[i2s]);
		    fprintf (ofp, " Distances: %7.2f%7.2f deg.\n", dist1, dist2);
		}
		goto done;
	    }
	}

	/* Look here for 1 azimuth and the 2 closest arrvial times */

/*
	if (iazim == 1 && icompr > 1)
	{
	    n1 = indexdsd[0];

	    Preferably with an arrival time from the azimuth datum

	    for (i = 0; i < icompr; i++)
	    {
		if (n1 == indexcompr[i])
		{
		    ADD CODE HERE
		}
	    }
	}
*/

	/*
	 * Find station with slowness and azimuth, then try to compute a 
	 * location from these data
	 */

	if (islow > 0 && iazim > 0)
	{
	    iusesta	= indexdsd2[0];
	    useslow	= bestslow[iusesta];
	    useazim	= bestazim[iusesta];

	    /*
	    for (i = 0; i < num_sites; i++)
	    {
		if (goodslow[i] && goodazim[i] && goodslow[i] > useslow)
		{
		    useslow = bestslow[i];
		    useazim = bestazim[i];
		    iusesta = i;
		}
	    }
	    */

	    for (j = 0; j < MAX_DIST-1; j++)
	    {
		if (useslow < slowness[j] && useslow >= slowness[j+1])
		{
		    dis[0] = (distance[j+1] - distance[j]) *
			      (useslow - slowness[j]) /
			      (slowness[j+1] - slowness[j]) + distance[j];

		    lat_lon (sites[iusesta].lat, sites[iusesta].lon,
			     dis[0], useazim, lat_init, lon_init);

		    if (verbose > 2) 
		    {
			fprintf (ofp, "    Method: P slowness with azimuth at 1 station\n");
			fprintf (ofp, "   Station: %-6s\n", sites[iusesta].sta);
			fprintf (ofp, "  Slowness: %7.2f sec./deg.\n", useslow);
			fprintf (ofp, "   Azimuth: %7.2f deg.\n", useazim);
			fprintf (ofp, "  Distance: %7.2f deg.\n", dis[0]);
		    }
		    goto done;
		}
	    }
	}

	/*
	 * # Look here for 3 closest arrvial times
	 */

	/*
	 * Use point near station with earlist arrival time.  Use the azimuth
	 * at that station, if there is one.  Probably will become unneccesary!
	 */

	if (icompr > 0)
	{
	    iusesta = indexcompr[0];
	    if (goodazim[iusesta])
	    {
		dis[0]	= DEFAULT_DIST;
		useazim	= bestazim[iusesta];
	    }
	    else
	    {
		dis[0]	= DEFAULT_DIST;
		useazim	= DEFAULT_AZIM;
	    }

	    if (verbose > 2) 
	    {
		fprintf (ofp, "    Method: Station with earliest arrival time\n");
		fprintf (ofp, "   Station: %-6s\n", sites[iusesta].sta);
		fprintf (ofp, "      Time: %7.2f sec.\n", ordercompr[0]);
		fprintf (ofp, "   Azimuth: %7.2f deg.\n", useazim);
		fprintf (ofp, "  Distance: %7.2f deg.\n", dis[0]);
	    }
	}

	/*
	 * Find the best determied azimuth 
	 */

	else if (iazim > 0)
	{
	    iusesta = indexdsd[0];
	    dis[0]  = DEFAULT_DIST;
	    if (goodazim[iusesta])
		useazim	= bestazim[iusesta];
	    else
		useazim	= DEFAULT_AZIM;

	    if (verbose > 2) 
	    {
		fprintf (ofp, "    Method: Station with smallest azimuth S.D.\n");
		fprintf (ofp, "   Station: %-6s\n", sites[iusesta].sta);
		fprintf (ofp, " Best S.D.: %7.2f deg.\n", bestazim[iusesta]);
		fprintf (ofp, "   Azimuth: %7.2f deg.\n", useazim);
		fprintf (ofp, "  Distance: %7.2f deg.\n", dis[0]);
	    }
	}

	/*
	 * Finally try station with the best slowness (as defined by the
	 * smallest s.d.) and the default azimuth
	 */

	else if (islow > 0)
	{
	    iusesta	= indexdsd2[0];
	    useslow	= bestslow[iusesta];
	    for (j = 0; j < MAX_DIST -1; j++)
	    {
		if (useslow < slowness[j] && useslow >= slowness[j+1])
			dis[0] = (distance[j+1] - distance[j]) *
				 (useslow - slowness[j]) /
				 (slowness[j+1] - slowness[j]) + distance[j];
	    }
	    useazim = DEFAULT_AZIM;

	    if (verbose > 2) 
	    {
		fprintf (ofp, "    Method: Station with largest slowness\n");
		fprintf (ofp, "   Station: %-6s\n", sites[iusesta].sta);
		fprintf (ofp, "  Slowness: %7.2f sec./deg.\n", useslow);
		fprintf (ofp, "   Azimuth: %7.2f deg.\n", useazim);
		fprintf (ofp, "  Distance: %7.2f deg.\n", dis[0]);
	    }
	}
                     

	/*
	 * If all else fails, bail out!
	 */

	else
	{
	    if (verbose > 2) 
	    {
		fprintf (ofp, " Initial location procedure failed - Bailing out\n");
		ierr = 1;
	    }
	}

	lat_lon (sites[iusesta].lat, sites[iusesta].lon, dis[0], useazim,
		 lat_init, lon_init);


done:
	if (verbose > 2)
	    print_guess (lat_init, lon_init, ofp);

	return (ierr);
}


static void
print_guess (double *alat, double *alon, FILE *ofp)
{
	char	ns = 'N';
	char	ew = 'E';

	if (*alat < 0.0)
	    ns = 'S';
	if (*alon < 0.0)
	    ew = 'W';
	fprintf (ofp, "  Latitude: %7.2f deg. %c\n Longitude: %7.2f deg. %c\n",
		fabs(*alat), ns, fabs(*alon), ew);
	fprintf (ofp, " --------------------------------------------------\n");

	return;
}


static void
order_data_asc (int index, int *array_index, double *order_array)
{

	int	i, j, n;
	double	tmp;

	/* Special case if index = 2 */

	if (index == 2)
	{
	    if (order_array[0] > order_array[1])
	    {
		tmp		= order_array[0];
		n		= array_index[0];
		order_array[0]	= order_array[1];
		array_index[0]	= array_index[1];
		order_array[1]	= tmp;
		array_index[1]	= n;
	    }
	    return;
	}

	/* Re-order data in ascending order */

	for (i = ((index+1)/2)-1; i >= 0; i--)
	{
	    for (j = 0; j < index-i; j++)
	    {
		if (order_array[j] > order_array[j+i])
		{
		    tmp = order_array[j];
		    n   = array_index[j];
		    order_array[j] = order_array[j+i];
		    array_index[j] = array_index[j+i];
		    order_array[j+i] = tmp;
		    array_index[j+i] = n;
		}
	    }
	}
	return;
}

