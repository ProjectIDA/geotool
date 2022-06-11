
/*
 * Copyright (c) 1993-1997 Science Applications International Corporation.

 * NAME
 *	in_polygon -- Is a lat/lon point inside a polygon?
 
 * FILE 
 *	in_polygon.c

 * SYNOPSIS
 *	int
 *	in_polygon (sample_lat, sample_lon, poly_data, num_poly_pairs)
 *	double	sample_lat;
 *	double	sample_lon;
 *	double	poly_data[][2];
 *	int	num_poly_pairs;

 * DESCRIPTION
 *	Function.  Determine whether or not a sample lat, lon pair is 
 *	contained within a pre-defined geographic polygonal region.  This 
 *	is done by testing to see if the angles sum to +/-360 deg.  If 
 *	+360 deg, then the location is within a right handed polygon.  If 
 *	-360, then they are inside a left handed polygon.  Otherwise, the 
 *	sample lat/lon point is not inside the polygon.

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	Pre-defined polygonal region "must be" convex!

 * SEE ALSO
 *	Related routine, locinpoly.c, written by Darrin Wahl, is located 
 *	outside the /nmrd tree area.  This routine is derived from that 
 *	function.

 * AUTHOR
 *	Walter Nagy, March 1993.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "libgeog.h"

#define	RAD_TO_DEG	(180./M_PI)
#define	DEG_TO_RAD	(M_PI/180.)
#define	TINY		0.0001 


int	
in_polygon (double sample_lat, double sample_lon, double poly_data[][2], 
	    int num_poly_pairs)
{

	int	i, ret;
	double	a, b, c, alatr, alonr, clat, clon, slat, slon;
	double	l1, l2, sum, temp, x[3];	
	double  *vertices_lon;

	if (num_poly_pairs < 2) 
	    return(ERR);

	vertices_lon = UALLOC (double, num_poly_pairs);

	/* 
	 * Rotate the polygon lat/lon pair to coincide with the sample lat/lon
	 */

	for (i = 0; i < num_poly_pairs; i++)
	{
	    /*
	     * First, convert a geographic location to geocentric co-
	     * ordinates assuming a radius of unit length.
	     */

	    a    = DEG_TO_RAD*poly_data[i][0];
	    b    = DEG_TO_RAD*poly_data[i][1];
	    c    = cos(a);
	    x[0] = c*sin(b);     /* Intersects equator at  0 deg. lon. */
	    x[1] = sin(a);	     /* Intersects equator at 90 deg. lon. */
	    x[2] = c*cos(b);     /* Intersects north pole */

	    /*
	     * Now rotate x[], first by alon deg. westward about the 1-axis
	     * and then alat deg. southward about the 0-axis.  x[] on
	     * output can be thought of as the original vector in a
	     * rotated system.  Axis 0 points east; axis 1 points north;
	     * and axis 2 points up.
	     */

	    alatr = (sample_lat-90.0)*DEG_TO_RAD;
	    alonr = sample_lon*DEG_TO_RAD;
	    slat  = sin(alatr);
	    clat  = cos(alatr);
	    slon  = sin(alonr);
	    clon  = cos(alonr);
	    a     = x[0]*clon - x[2]*slon;
	    b     = x[1];
	    c     = x[0]*slon + x[2]*clon;
	    x[0]  = a;
	    x[1]  = b*clat - c*slat;
	    x[2]  = b*slat + c*clat;

	    /*
	     * Finally, convert the longitude component of the 
	     * geocentric system back to geographic coordinates.
	     */

	/*  r13 = sqrt(x[2]*x[2] + x[0]*x[0]);   Don't need lat 
	    vertices[0][i] = RAD_TO_DEG*atan2(x[1], r13); */
	    vertices_lon[i] = RAD_TO_DEG*atan2(x[0], x[2]);
	}
		

	/* Sum the longitudes of all the points in polygon */

	sum = 0.0;
	for (i = 0; i < num_poly_pairs-1; i++)
	{
	    l1 = vertices_lon[i];
	    l2 = vertices_lon[i+1];
		
	    if (l1 < 360.0) 
		l1 += 360.0;
	    if (l2 < 360.0) 
		l2 += 360.0;			
	    temp = l2 - l1;
	    if (temp > 180.0) 
		temp -= 360.0; 
	    if (temp < -180.0) 
		temp += 360.0; 
			
	    sum += temp;
	}
		

	/* Connect the last vertex longitude with the first */

	l1 = vertices_lon[num_poly_pairs-1];
	l2 = vertices_lon[0];
	if (l1 < 360.0) 
	    l1 += 360.0;
	if (l2 < 360.0) 
	    l2 += 360.0;			
	temp = l2 - l1;
	if (temp > 180.0) 
	    temp -= 360.0; 
	if (temp < -180.0) 
	    temp += 360.0; 
	sum += temp;
		
	/* 
	 * If the angles sum to +360 then the location is within
	 * a right handed polygon.  If they sum to -360, they are
	 * within a left handed polygon.  Otherwise, the location
	 * is not within the polygon.
	 */

	if (fabs(sum) > (360.0 - TINY)) 
	    ret =  TRUE;
	else
	    ret =  FALSE;
	
	UFREE (vertices_lon);
	
	return (ret);

}

