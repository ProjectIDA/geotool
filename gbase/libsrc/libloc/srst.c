
/*
 * Copyright (c) 1993-1996 Science Applications International Corporation.
 *

 * NAME
 *	read_srst -- Read SRST tables.
 *	apply_srst -- Apply SRST correction adjustments.
 *	set_srst_region_number -- Initialize SRST region number to NULL value.
 *	get_srst_region_number -- Get current SRST region number.

 * FILE
 *	srst.c

 * SYNOPSIS
 *	int
 *	read_srst (default_vmodel_pathway, default_vmodel)
 *	char	*default_vmodel_pathway;(i) Directory pathway where default
 *					    T-T tables exist
 *	char	*default_vmodel;	(i) Default velocity model name

 *	Bool
 *	apply_srst (sta, ev_geoc_lat, ev_lon, ev_geoc_co_lat, ev_depth, 
 *		    correct, var_wgt)
 *	char	*sta;		(i) Station name
 *	double	ev_geoc_lat;	(i) Geocentric event latitude (decmimal deg.)
 *	double	ev_lon;		(i) Event longitude (decmimal degrees)
 *	double	ev_geoc_co_lat;	(i) Geocentric event co-latitude (decmimal deg.)
 *	double	ev_depth;	(i) Event depth (km)
 *	double	*correct;	(o) SRST correction (sec)
 *	double	*var_wgt;	(o) SRST weighting factor

 *	void
 *	set_srst_region_number ()

 *	int
 *	get_srst_region_number ()

 * DESCRIPTION
 *	-- read_srst() reads source region/station time (SRST) tables
 *	from files into memory.  Each station encountered in an SRST file 
 *	will be read along with its coefficients, variance weighting 
 *	and type.  The geographical region boundaries are defined by 
 *	polygonal vertices (in the convex sense).  Each data line 
 *	contains the station name, 4 coeffients, its variance weighting 
 *	and SRST type.  This latter variable determines the equation 
 *	type to be applied.

 *	-- apply_srst() corrects for source region/station time (SRST) terms 
 *	in observed travel time for a given station.  Apply SRST correction 
 *	to partially compensate for lateral velocity heterogeneities.  
 *	Distinct corrections exist for different geographic regions and
 *	stations.  First, we must find out whether or not the event
 *	location is inside one of the region polygons.  If so, we need
 *	to find the appropriate station, if one exists, and extract its
 *	polynomial coefficients, weights and type descriptor.  From these
 *	terms we then calculate the SRST correction (seconds).

 *	-- set_srst_region_number() simply initializes current SRST region
 *	number to NULL value (-1).

 *	-- get_srst_region_number() gets the current SRST region number.
 *	This is currently only needed by ARS for display purposes.

 * DIAGNOSTICS
 *	-- read_srst() will return with an error code of ERR (-1) if input
 *	contains improperly formatted data.

 *	-- apply_srst() will complain if obvious incompatabilities are 
 *	encountered.  Will return Boolean variable as TRUE, if SRST 
 *	correction is found.

 *	-- get_srst_region_number() returns a NULL value (-1) if no SRST
 *	corrections are currently being applied.

 * FILES
 *	-- read_srst() reads input SRST correction file.  The file name MUST
 *	be "srst.operational", and MUST also be, located immediately underneath
 *	the default T-T table directory, in its own directory, SRST.  That 
 *	is, relative to default T-T table area, the file will be found in
 *	file, "SRST/srst.operational".

 * NOTES
 *	Under normal operating circumstances, a SSSC should be search for
 *	first, then if none exists, look for an SRST correction.  One day,
 *	should replace SRST approach with newer SSSC methodology!

 * SEE ALSO
 *	K.F. Vieth, 1975, BSSA, vol. 65, pp. 1199-1222, "Refined Hypocenters
 *	and Accurate Reliability Estimates," for explanation of SRST's.  

 * AUTHOR
 *	Walter Nagy, 3/93,	Created.
 *	Walter Nagy, 11/16/95,	File re-named, srst.c.
 *	Walter Nagy, 11/30/95,	Updated to use more memory-friendly structure.
 *				Functions to initialize and get SRST region 
 *				number added.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "locp.h"
#include "libgeog.h"
#include "loc_defs.h"
#include "srst.h"

static	Srst	*srst = (Srst *) NULL;
static	int	current_srst_region_number = -1;


int
read_srst (char *default_vmodel_pathway, char *default_vmodel)
{

	FILE	*srst_fp;
	int	i, j;
	double	ref_co_lat, ref_lon, ref_rot;
	char	srst_file_name[FILENAMELEN];


	strcpy (srst_file_name, default_vmodel_pathway);
	strcat (srst_file_name, "/");
	strcat (srst_file_name, default_vmodel);
	strcat (srst_file_name, ".srst");

	/*
	 * Open SRST file, if it is available!  To be read this file MUST
	 * exist under the default velocity model T-T area as file,
	 * default_vmodel_pathway/vmodel.srst.  It's OK to not have an 
	 * SRST file, but obviously, no SRST corrections can be applied 
	 * under this scenario.
	 */

	if ((srst_fp = fopen (srst_file_name, "r")) == NULL)
	{
	    fprintf (stderr, "Message: No SRST file exists in default T-T table area!\n");
	    srst = (Srst *) NULL;
	    return (OK);
	}

	/*
	 * How many SRST regions are there ? 
	 */

	if (fscanf (srst_fp, "%d%*[^\n]", &num_srst_regions) != 1) return(ERR);

	/*
	 * If a previous SRST file existed, free its memory, then re-allocate,
	 * as necessary, srst structure.
	 */

	if (srst != (Srst *) NULL)
	{
	    for (i = 0; i < num_srst_regions; i++)
	    {
		UFREE (srst[i].poly_lat_data);
		UFREE (srst[i].poly_lon_data);
		for (j = 0; j < srst[i].num_sta_w_srst; j++) {
		    UFREE (srst[i].sta[j]);
		}
		UFREE (srst[i].sta);
		UFREE (srst[i].type);
		UFREE (srst[i].c0);
		UFREE (srst[i].c1);
		UFREE (srst[i].c2);
		UFREE (srst[i].c3);
		UFREE (srst[i].weight);
	    }
	    UFREE (srst);
	}

	srst = UALLOC (Srst, num_srst_regions);

	/*
	 * First read polygonal boundary and reference coordinate system 
	 * information.  Then read the SRST corrections themselves.
	 */

	for (i = 0; i < num_srst_regions; i++)
	{
	    /*
	     * Read the SRST region number; reference coordinate 
	     * latitude, longitude, and the x-axis rotation angle;
	     * number of polygon points; and number of actual SRST
	     * corections included in the file.
	     */

	    if ((fscanf (srst_fp, "%d%lf%lf%lf%d%d%*[^\n]", &srst[i].reg_num, 
		   &ref_co_lat, &ref_lon, &ref_rot, &srst[i].num_poly_pairs, 
		   &srst[i].num_sta_w_srst)) != 6)
		return (ERR);

	    srst[i].ref_coord[0] = ref_co_lat*DEG_TO_RAD;
	    srst[i].ref_coord[1] = ref_lon*DEG_TO_RAD;
	    srst[i].ref_coord[2] = ref_rot*DEG_TO_RAD;

	    /* Allocate necessary memory for srst structure */

	    srst[i].poly_lat_data = UALLOC (double, srst[i].num_poly_pairs);
	    srst[i].poly_lon_data = UALLOC (double, srst[i].num_poly_pairs);

	    srst[i].sta = UALLOC (char *, srst[i].num_sta_w_srst);
	    for (j = 0; j < srst[i].num_sta_w_srst; j++)
		srst[i].sta[j] = UALLOC (char, 7);
	    srst[i].type   = UALLOC (int, srst[i].num_sta_w_srst);
	    srst[i].c0     = UALLOC (double, srst[i].num_sta_w_srst);
	    srst[i].c1     = UALLOC (double, srst[i].num_sta_w_srst);
	    srst[i].c2     = UALLOC (double, srst[i].num_sta_w_srst);
	    srst[i].c3     = UALLOC (double, srst[i].num_sta_w_srst);
	    srst[i].weight = UALLOC (double, srst[i].num_sta_w_srst);

	    /* Read polygon data */

	    for (j = 0; j < srst[i].num_poly_pairs; j++)
	    {
		if ((fscanf (srst_fp, "%lf%lf%*[^\n]", 
				&srst[i].poly_lat_data[j],
				&srst[i].poly_lon_data[j])) != 2)
		    return (ERR);
	    }

	    /* 
	     * Now read SRST data, station by station!
	     */

	    for (j = 0; j < srst[i].num_sta_w_srst; j++)
	    {
		if ((fscanf (srst_fp, "%6s%lf%lf%lf%lf%lf%d%*[^\n]",
				 srst[i].sta[j], &srst[i].c0[j], &srst[i].c1[j],
				 &srst[i].c2[j], &srst[i].c3[j], 
				 &srst[i].weight[j], &srst[i].type[j])) != 7)
		    return (ERR);
	    }
	}

	fclose (srst_fp);
	return (OK);
}


Bool
apply_srst (char *sta, double ev_geoc_lat, double ev_lon, 
	    double ev_geoc_co_lat, double ev_depth, double *correct,
	    double *var_wgt)
{

	int	i, ireg, j, k;
	double	a, b, cref, phi, rcor, rtmp, rot, sref;
	double	stheta, theta, x, y;
	double	poly_data[50][2];


	/*
	 * Is given event within any of our SRST regions?  If so, set region
	 * index (not region number), and look for station.
	 */

	for (ireg = -1, i = 0; i < num_srst_regions; i++)
	{
	    for (j = 0; j < srst[i].num_poly_pairs; j++)
	    {
		poly_data[j][0] = srst[i].poly_lat_data[j];
		poly_data[j][1] = srst[i].poly_lon_data[j];
	    }
	    if (in_polygon (ev_geoc_lat, ev_lon, poly_data,
			    srst[i].num_poly_pairs))
	    {
		ireg = i;
		break;
	    }
	}

	if (ireg < 0)
	    return (FALSE);		/* No valid region found, so return */

	/*
	 * Is input station (sta) contained within SRST file?  If so, srst
	 * array index has been found, therefore, grab its contents (i.e.,
	 * coefficients and weight) to compute actual SRST correction at
	 * current event location.
	 */

	for (k = -1, i = 0; i < srst[ireg].num_sta_w_srst; i++)
	{
	    if (STREQ (sta, srst[ireg].sta[i]))
	    {
		k = i;
		break;
	    }
	}
	if (k < 0)
	    return (FALSE);		/* No valid station found, so return */

	/*
	 * SRST row found for station/region pair.  Now determine correction
	 * based on type of equation related to coefficients.
	 */

	theta	= ev_geoc_co_lat;
	phi	= ev_lon*DEG_TO_RAD;
	stheta	= sin(theta);
	a	= theta - (double) srst[ireg].ref_coord[0];
	b	= phi - (double) srst[ireg].ref_coord[1];
	rot	= (double) srst[ireg].ref_coord[2];
	cref	= cos(rot);
	sref	= sin(rot);

	x = (a*cref - b*sref*stheta);	/* From eqn (11) of Vieth (1975) */
	y = (a*sref + b*cref*stheta);

	/* From eqn (12) of Vieth (1975) */
	rcor = srst[ireg].c0[k] + x*srst[ireg].c1[k] + y*srst[ireg].c2[k];

	if (srst[ireg].type[k] == 1)
	    rtmp = ev_depth*srst[ireg].c3[k];
	else if (srst[ireg].type[k] == 2)
	    rtmp = x*x*srst[ireg].c3[k];
	else
	    rtmp = y*y*srst[ireg].c3[k];

	*correct = rcor + rtmp;			/* SRST correction */
	*var_wgt = srst[ireg].weight[k];	/* Variance weighting */

	/*
	 * Save current SRST region number.  This is currently needed for
	 * display purposes, but could be used in other ways.
	 */

	current_srst_region_number = srst[ireg].reg_num;

	return (TRUE);
}


void
set_srst_region_number (void)
{
	current_srst_region_number = -1;
	return;
}


int
get_srst_region_number (void)
{
	return (current_srst_region_number);
}
