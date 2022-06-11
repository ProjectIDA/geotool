/*
 * NAME
 *	lat_lon -- Given a lat/lon position and azi, find second lat/lon point.

 * FILE
 *	lat_lon.c

 * SYNOPSIS
 *	Find a point on a sphere which is a given angular distance and 
 *	azimuth away from another point.  Input and output lat/lon data 
 *	are in a geographic coordinate frame of reference.

 * DESCRIPTION
 *	Assuming a spherical Earth model, compute a second lat/lon position
 *	on the globe given the first position and an azimuth.  Both a
 *	FORTRAN and C interface exists, lat_lon_() and lat_lon(), 
 *	respectively.

 *	---- On entry ----
 *	alat1:	Geographic latitude position of point 1 (deg)
 *	alon1:	Geographic longitude position of point 1 (deg)
 *	delta:	Geoentric distance between points 1 and 2 (deg)
 *	azi:	Geocentric azimuth from north of point 2 w.r.t. point 1 
 *		(clockwise)

 *	---- On return ----
 *	alat2:	Geographic latitude position of point 2 (deg)
 *	alon2:	Geographic longitude position of point 2 (deg)

 * DIAGNOSTICS
 *	None.

 * NOTES
 *	All arguments are in degrees.  Input and output latitude and 
 *	longitude are geographic.  Input delta and azi are geocentric.
 *	Latitude is zero at equator and positive North.  Longitude is 
 *	positive toward the East azi is measured clockwise from local 
 *	North.  Also note that the conversion from geographic to geocentric
 *	coordinates are done different from the complementary function, 
 *	dist_azimuth().  These routines will be unified at a later date.

 * SEE ALSO
 *	Complementary routine, dist_azimuth(), which calculates the angular
 *	distance, azimuth and  between two lat/lon points assuming a 
 *	spherical Earth..

 * AUTHOR
 *	Walter Nagy, January 1994
 */

#include "config.h"
#include <math.h>
#include "libgeog.h"

#define RAD_TO_DEG	(180.0/M_PI)
#define DEG_TO_RAD	(M_PI/180.0)
#define	SIGN(a1, a2)	((a2) >= 0 ? -(a1) : (a1))

#define GEOCENTRIC_COLAT(x) \
	(x + (((0.192436*sin(x+x)) + (0.000323*sin(4.0*x)))*DEG_TO_RAD))
#define GEOGRAPHIC_LAT(x) \
	(90.0 - (x*RAD_TO_DEG - (0.192436*sin(x+x) - 0.000323*sin(4.0*x))))


/*
 * This function is simply a FORTRAN interface to the C routine, lat_lon()
 */

void
lat_lon_(double *flat1, double *flon1, double *delta, double *azi,
		double *flat2, double *flon2)
{
	lat_lon (*flat1, *flon1, *delta, *azi, flat2, flon2);
}


void
lat_lon(double alat1, double alon1, double delta, double azi,
		double *alat2, double *alon2)
{
	double	alat, alon, a, b, c, coslat, dlon;
	double	geoc_co_lat, geog_co_lat;
	double	r13, r13sq, sinlat, x1, x2, x3;
	/*	double	atan2(), cos(), sin();*/


	/*
	 * Convert a geographical location to geocentric cartesian 
	 * coordinates, assuming a spherical Earth.
	 */
 
	alat = 90.0 - delta;
	alon = 180.0 - azi;
	r13  = cos(DEG_TO_RAD*alat);

	/*
	 * x1:	Axis 1 intersects equator at  0 deg longitude  
	 * x2:	Axis 2 intersects equator at 90 deg longitude  
	 * x3:	Axis 3 intersects north pole
	 */

	x1 = r13*sin(DEG_TO_RAD*alon);
	x2 = sin(DEG_TO_RAD*alat);
	x3 = r13*cos(DEG_TO_RAD*alon);

	geog_co_lat = (90.0-alat1)*DEG_TO_RAD;		/* radians */
	geoc_co_lat = GEOCENTRIC_COLAT(geog_co_lat);	/* radians */

	/*
	 * Rotate in cartesian coordinates.  The cartesian coordinate system 
	 * is most easily described in geocentric terms.  The origin is at 
	 * the Earth's center.  Rotation by alat1 degrees southward, about 
	 * the 1-axis.
	 */

	sinlat = sin(geoc_co_lat);
	coslat = cos(geoc_co_lat);
	b      = x2;
	c      = x3;
	x2     = b*coslat - c*sinlat;
	x3     = b*sinlat + c*coslat;

	/*
	 * Finally, convert geocentric cartesian coordinates back to 
	 * a geographical location.
	 */
 
	r13sq  = x3*x3 + x1*x1;
	r13    = sqrt(r13sq);
//	r123   = sqrt(r13sq + x2*x2);
	dlon   = RAD_TO_DEG*atan2(x1, x3);
	a      = 90.0*DEG_TO_RAD - atan2(x2, r13);
	*alat2 = GEOGRAPHIC_LAT(a);
	*alon2 = alon1 + dlon;
	if (fabs(*alat2) > 90.0)
		*alat2 = SIGN((180.0-fabs(*alat2)), *alat2);
	if (fabs(*alon2) > 180.0)
		*alon2 = SIGN((360.0-fabs(*alon2)), *alon2);
}
