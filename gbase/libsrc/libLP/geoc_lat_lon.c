
/*
 * Copyright (c) 1995 Science Applications International Corporation.
 *

 * NAME
 *	geoc_lat_lon -- Given a lat/lon position & azi, find 2nd lat/lon point.

 * FILE
 *	geoc_lat_lon.c

 * SYNOPSIS
 *	Find a point on a sphere which is a given angular distance and 
 *	azimuth away from another point.  Input and output lat/lon data 
 *	are in a geocentric coordinate frame of reference.

 * DESCRIPTION
 *	Assuming a spherical Earth model, compute a second lat/lon position
 *	on the globe given the first position and an azimuth.  Both a
 *	FORTRAN and C interface exists, geoc_lat_lon_() and geoc_lat_lon(), 
 *	respectively.

 *	---- On entry ----
 *	alat1:	Geocentric latitude position of point 1 (radians)
 *	alon1:	Geocentric longitude position of point 1 (radians)
 *	delta:	Geocentric distance between points 1 and 2 (radians)
 *	azi:	Geocentric azimuth from north of point 2 w.r.t. point 1 
 *		(clockwise; radians)

 *	---- On return ----
 *	alat2:	Geocentric latitude position of point 2 (radians)
 *	alon2:	Geocentric longitude position of point 2 (radians)

 * DIAGNOSTICS
 *	None.

 * NOTES
 *	All arguments are in radians.  Input and output latitude are in 
 *	the geocentric coordinate system.  Latitude is zero at North Pole
 *	and positive to the south.  Longitude is positive toward the East 
 *	azi is measured clockwise from North.

 * SEE ALSO
 *	Complementary routine, geoc_distaz(), which calculates the angular
 *	distance, azimuth and  between two lat/lon points assuming a 
 *	spherical Earth.

 * AUTHOR
 *	Walter Nagy, June 23, 1995	Created.
 */

#include "config.h"
#include <math.h>
#include "libLP.h"

#define	TWO_PI		2.0*M_PI
#define	SIGN(a1, a2)	((a2) >= 0 ? -(a1) : (a1))


/*
 * This function is simply a FORTRAN interface to the C routine, geoc_lat_lon()
 */

void
geoc_lat_lon_ (double *flat1, double *flon1, double *delta, double *azi, double *flat2, double *flon2)

     /*double	*flat1, *flon1, *azi, *delta;*/
     /*double	*flat2, *flon2;*/

{
	geoc_lat_lon (*flat1, *flon1, *delta, *azi, flat2, flon2);
}


void
geoc_lat_lon (double alat1, double alon1, double delta, double azi, double *alat2, double *alon2)

     /*double  alat1, alon1, azi, delta;*/
/*double  *alat2, *alon2;*/

{
	double	alat, alon, b, c, coslat, dlon;
	double	r13, r13sq, sinlat, x1, x2, x3;
	/*	double	atan2(), cos(), sin();*/


	/*
	 * Convert a geographical location to geocentric cartesian 
	 * coordinates, assuming a spherical Earth.
	 */
 
	alat = M_PI_2 - delta;
	alon = M_PI - azi;
	r13  = cos(alat);

	/*
	 * x1:	Axis 1 intersects equator at  0 deg longitude  
	 * x2:	Axis 2 intersects equator at 90 deg longitude  
	 * x3:	Axis 3 intersects north pole
	 */

	x1 = r13*sin(alon);
	x2 = sin(alat);
	x3 = r13*cos(alon);

	/*
	 * Rotate in cartesian coordinates.  The cartesian coordinate system 
	 * is most easily described in geocentric terms.  The origin is at 
	 * the Earth's center.  Rotation by alat1 degrees southward, about 
	 * the 1-axis.
	 */

	sinlat = sin(alat1);
	coslat = cos(alat1);
	b      = x2;
	c      = x3;
	x2     = b*coslat - c*sinlat;
	x3     = b*sinlat + c*coslat;

	r13sq  = x3*x3 + x1*x1;
	r13    = sqrt(r13sq);
//	r123   = sqrt(r13sq + x2*x2);
	dlon   = atan2(x1, x3);
	*alat2 = M_PI_2 - atan2(x2, r13);
	*alon2 = alon1 + dlon;
	if (*alat2 > M_PI)
		*alat2 = TWO_PI - *alat2;
	if (*alat2 < 0.0)
		*alat2 = -(*alat2);
	if (fabs(*alon2) > M_PI)
		*alon2 = SIGN((TWO_PI-fabs(*alon2)), *alon2);
}

