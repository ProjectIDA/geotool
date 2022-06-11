/*
 * Copyright (c) 1994-1997 Science Applications International Corporation.
 *

 * NAME
 *	ellip_dist -- Find distance and azimuth between 2 points on an ellipsoid.

 * FILE
 *	ellip_dist.c

 * SYNOPSIS
 *	int
 *	ellip_dist (lat1, lon1, lat2, lon2, dist, faz, baz, flag)
 *	double	lat1;		(i) Geographic latitude of point 1 (deg)
 *	double	lon1;		(i) Geographic longitude of point 1 (deg)
 *	double	lat2;		(i) Geographic latitude of point 2 (deg)
 *	double	lon2;		(i) Geographic longitude of point 2 (deg)
 *	double	*dist;		(o) distance in between 1 & 2 (deg/km)
 *	double	*faz;		(o) azimuth from north of 2 w.r.t. 1 
 *				    (clockwise; deg)
 *	double	*baz;		(o) Geocentric back-azimuth from north (deg)
 *	int	flag		(i) Flag specifying if dist should be in
 *				    psuedo-degrees (0) or km (1)

 * DESCRIPTION
 *   Function ellip_dist. 
 *
 *	Finds the distance, azimuth and backazimuth between two lat/lon points
 *	along the surface of a reference ellipsoid.
 *
 *	If flag is false than the distance is returned in degrees
 *	by setting 1 deg = 111.12 km (60 nautical miles * 1.852 km/nm)
 *	This means the distance between the equator and the north pole
 *	is not 90 degrees, but instead 90.01039 degrees
 *
 *	uses GRS80 / WGS84 (NAD83) earth model (ellipsoid)
 *
 *	The meat of the algorithm was taken from the code "inverse.for"
 *	written by L.Pfeifer and John G Gergen of the National Geodetic
 *	Survey (NGS), Rockville MD
 *
 *	The algorithm follows T. Vincenty's modified Rainsford's method 
 *	with Helmert's elliptical terms.
 *
 * DIAGNOSTICS
 *	
 *	Returns  1 on successful completion
 *	Returns  0 if invalid latitudes are entered (i.e. fabs(lat) > 90)
 *	Returns -1 if the two points are identical (sets dist=0,faz=0,baz=180)
 *	Returns -2 if fails to converge (computes dist,faz and baz using dist_azimuth)
 *
 * NOTES
 *
 * SEE ALSO
 *	dist_azimuth.c
 *
 * AUTHOR
 *	Jeffrey A. Hanson, August 1999
 */
#include "config.h"
#include <stdio.h>
#include <math.h>
#include "aesir.h"
#include "libgeog.h"

/*
void   dist_azimuth(double lat1,
                    double lon1,
		    double lat2,
		    double lon2,
		    double *delta,
		    double *azi,
		    double *baz,
		    int    phase_index);
*/


/* we are defining the number of km in a degree such that 
 * 1 degree = 60 nautical miles which is somewhat different 
 * than using the earth's average radius (i.e. 111.195) */
#define	KM_PER_DEG	(111.12) 
#define DEG_PER_RAD	(57.295779513082321004)

#define CONVERGE	(0.5e-13)
#define	INF_LOOP	(100)

/* GRS80 or WGS84 (NAD83) earth model (ellipsoid) */
	/* equatorial radius (meters) */
#define EQ_RADIUS	(6378137.0)
	/* earth flattening = (EQ_RADIUS-POLAR_RADIUS)/EQ_RADIUS */
#define FLATTENING	(1.0/298.25722210088)

int
ellip_dist(double lat1, double lon1, double lat2, double lon2, double *dist,
		double *faz, double *baz, int flag)
{

char	*fname		= "ellip_dist";

long	success		= 0;
long	cnt		= 0;

double	a		= EQ_RADIUS;
double	f		= FLATTENING;

double	r, tu1, tu2, cu1, su1, cu2, s;
double	d, x, sx, cx, sy, cy, y, sa, c2a, cz, e, c ;
double	fazi, bazi;

/* can't be at earth's pole of rotation */
if (lat1 > 90.0 || lat1 < -90.0) {
	fprintf(stderr,"%s: invalid latitude\n",fname);
	goto RETURN;
} else if (lat1 > 89.9999) {
	lat1 = 89.9999;
} else if (lat1 < -89.9999) {
	lat1 = -89.9999;
}
if (lat2 > 90.0 || lat2 < -90.0) {
	fprintf(stderr,"%s: invalid latitude\n",fname);
	goto RETURN;
} else if (lat2 > 89.9999) {
	lat2 = 89.9999;
} else if (lat2 < -89.9999) {
	lat2 = -89.9999;
}

/* make longitudes run between -180 and 180 (not really necessary but makes
   it easier to test if the points are at the same location)
*/
if (lon1 < -180.0) lon1 += 360.0;
if (lon1 >  180.0) lon1 -= 360.0;
if (lon2 < -180.0) lon1 += 360.0;
if (lon2 >  180.0) lon1 -= 360.0;

/* check if points are equal */
if (lat1==lat2 && lon1==lon2) {
	*dist = 0.0;
	*faz = 0.0;
	*baz = 180.0;
	success = -1;
	goto RETURN;
}

/* convert lat and lon from degrees into radians */
lat1 /= DEG_PER_RAD;
lon1 /= DEG_PER_RAD;
lat2 /= DEG_PER_RAD;
lon2 /= DEG_PER_RAD;

/*
C *** SOLUTION OF THE GEODETIC INVERSE PROBLEM AFTER T.VINCENTY
C *** MODIFIED RAINSFORD'S METHOD WITH HELMERT'S ELLIPTICAL TERMS
C *** EFFECTIVE IN ANY AZIMUTH AND AT ANY DISTANCE SHORT OF ANTIPODAL
C *** STANDPOINT/FOREPOINT MUST NOT BE THE GEOGRAPHIC POLE
C
C *** A IS THE SEMI-MAJOR AXIS OF THE REFERENCE (EARTH) ELLIPSOID
C *** F IS THE FLATTENING (NOT RECIPROCAL) OF THE REFERNECE ELLIPSOID
C *** LATITUDES AND LONGITUDES IN RADIANS POSITIVE NORTH AND EAST
C *** FORWARD AZIMUTHS AT BOTH POINTS RETURNED IN RADIANS FROM NORTH
C
C *** PROGRAMMED FOR CDC-6600 BY LCDR L.PFEIFER NGS ROCKVILLE MD 18FEB75
C *** MODIFIED FOR IBM SYSTEM 360 BY JOHN G GERGEN NGS ROCKVILLE MD 7507
C
*/
/* The following code is ugly because it's converted from fortran code 
   written in 1975. */

r 	= 1.0 - f;
tu1 	= r*sin(lat1)/cos(lat1);
tu2 	= r*sin(lat2)/cos(lat2);
cu1 	= 1.0/sqrt(tu1*tu1+1.0);
su1 	= cu1*tu1;
cu2 	= 1.0/sqrt(tu2*tu2+1.0);
s 	= cu1*cu2;
bazi 	= s*tu2;
fazi	= bazi*tu1;

x	= lon2-lon1;

do {

	sx	= sin(x);
	cx	= cos(x);
	tu1	= cu2 * sx;
	tu2	= bazi-su1*cu2*cx;
	sy	= sqrt(tu1*tu1+tu2*tu2);
	cy	= s*cx+fazi;
	y	= atan2(sy,cy);
	sa	= s*sx/sy;
	c2a	= 1.0-sa*sa;
	cz	= fazi+fazi;
	if (c2a > 0.0) cz = cy-cz/c2a;
	e	= 2.0*cz*cz-1.0;
	c	= ((-3.0*c2a+4.0)*f+4.0)*c2a*f/16.0;
	d	= x;
	x	= ((e*cy*c+cz)*sy*c+y)*sa;
	x	= (1.0-c)*x*f+lon2-lon1;

	/* if points are near anti-podes this may fail to converge 
	 * We'll stop the loop and use the more robust but less
	 * accurate dist_azimuth routine 
	 */
	if (++cnt > INF_LOOP) {
		fprintf(stderr,"%s: Failed to converge, using 'dist_azimuth' instead\n",fname);
		dist_azimuth(lat1*DEG_PER_RAD,lon1*DEG_PER_RAD,
			     lat2*DEG_PER_RAD,lon2*DEG_PER_RAD,
			     &s,&fazi,&bazi,0);

		if (flag) {
			*dist = s*KM_PER_DEG;
		} else {
			*dist = s;
		}
		*faz  = fazi;
		*baz  = bazi;
		success = -2;

		goto RETURN;
	}
} while (fabs(d-x) > CONVERGE);

fazi	= atan2(tu1,tu2);
bazi	= atan2(cu1*sx,bazi*cx-su1*cu2)+M_PI;
x	= sqrt((1.0/r/r-1.0)*c2a+1.0)+1.0;
x	= (x-2.0)/x;
c	= 1.0-x;
c	= (x*x/4.0+1.0)/c;
d	= (0.375*x*x-1.0)*x;
x	= e*cy;
s	= 1.0-e*e;
s	= ((((sy*sy*4.0-3.0)*s*cz*d/6.0-x)*d/4.0+cz)*sy*d+y)*c*a*r;

/* convert fazi and bazi into degrees */
fazi *= DEG_PER_RAD;
bazi *= DEG_PER_RAD;

/* put answers into return parameters */
*dist	= s/1000.0;	/* convert to km */

if (!flag) {
	*dist /= KM_PER_DEG;
}

*faz	= fazi;
*baz	= bazi;

if (*faz < 0.0) *faz += 360.0;
if (*baz < 0.0) *baz += 360.0;

success = 1;

RETURN:

	return(success);

}
