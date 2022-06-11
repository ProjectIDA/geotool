
/*
 * NAME
 *	dist_azimuth -- Find distance and azimuth between 2 points on a sphere.

 * FILE
 *	dist_azimuth.c

 * SYNOPSIS
 *	Find the angular distance, azimuth and backazimuth between two 
 *	lat/lon points assuming a spherical Earth.  Geographic input
 *	version.

 * DESCRIPTION
 *	Function.  Assuming a spherical Earth model, compute the 
 *	angular distance, azimuth and back azimuth between two
 *	geographic latitude and longitude positions (typically measured 
 *	from a station to an event).  Both a FORTRAN and C interface
 *	exists, dist_azimuth_() and dist_azimuth(), respectively.

 *	---- On entry ----
 *	alat1:		Geographic latitude position of point 1 (deg)
 *	alon1:		Geographic longitude position of point 1 (deg)
 *	alat2:		Geographic latitude position of point 2 (deg)
 *	alon2:		Geographic longitude position of point 2 (deg)
 *	phase_index:	If > 0, 2nd pair of lat/lon coordinates have not 
 *			changed, so, some CPU efficiencies can be achieved 

 *	---- On return ----
 *	delta:		Geocentric distance between points 1 and 2 (deg)
 *	azi:		Geocentric azimuth from north of point 2 w.r.t. point 1 
 *			(clockwise)
 *	baz:		Geocentric back-azimuth from north (clockwise)

 * DIAGNOSTICS
 *	None.

 * NOTES
 *	All arguments are in degrees.  Latitude, longitude are geographic.  
 *	delta, azi and baz are geocentric.  Latitude is zero at equator and 
 *	positive North.  Longitude is positive toward the East azi is 
 *	measured clockwise from local North.  The first lat./lon. pair should 
 *	represent the station coordinates as a general rule.  Thus, the
 *	second pair would be the event coordinates.  If the 2nd pair of 
 *	lat/lon coordinates change with every event call, simply set 
 *	phase_index to 0 at every call to this function.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy, January 1994.
 */

#include "config.h"
#include <math.h>
#include "libgeog.h"

#define RAD_TO_DEG	(180.0/M_PI)
#define DEG_TO_RAD	(M_PI/180.0)

#define GEOCENTRIC_COLAT(x) \
	(x + (((0.192436*sin(x+x)) + (0.000323*sin(4.0*x)))*DEG_TO_RAD))


/* 
 * This function is simply a FORTRAN interface to C routine, dist_azimuth() 
 */

void
dist_azimuth_ (double *flat1, double *flon1, double *flat2, double *flon2,
		double *delta, double *azi, double *baz)
{
	dist_azimuth (*flat1, *flon1, *flat2, *flon2, delta, azi, baz, 0);
}


void
dist_azimuth (double alat1, double alon1, double alat2, double alon2,
		double *delta, double *azi, double *baz, int phase_index)
{
	double	clat1, cdlon, cdel, geoc_co_lat, geoc_lat1, geoc_lat2;
	double	geog_co_lat, rdlon, slat1, sdlon, xazi, xbaz, yazi, ybaz;
	static	double	clat2, slat2;


	/*
	 * Convert alat2 from geographic latitude to geocentric latitude 
	 * (radians).  This is optimized to avoid extra sine and cosine
	 * calls when alat2 doesn't change between calls (such as looping
	 * over a set of stations for a given event).
	 */

	if (phase_index == 0)
	{
		geog_co_lat = (90.0-(alat2))*DEG_TO_RAD;
		geoc_co_lat = GEOCENTRIC_COLAT(geog_co_lat);
		geoc_lat2 = 90.0*DEG_TO_RAD-geoc_co_lat;

		clat2 = cos(geoc_lat2);
		slat2 = sin(geoc_lat2);
	}

	/*
	 * Simple case when both sets of lat/lons are the same.
	 */

	if ((alat1 == alat2) && (alon1 == alon2))
	{
		*delta = 0.0;
		*azi = 0.0;
		*baz = 180.0;
		return;
	}

	/*
	 * Convert alat1 from geographic latitude to geocentric latitude 
	 * (radians).
	 */

	geog_co_lat = (90.0-(alat1))*DEG_TO_RAD;
	geoc_co_lat = GEOCENTRIC_COLAT(geog_co_lat);
	geoc_lat1 = 90.0*DEG_TO_RAD-geoc_co_lat;

	rdlon = DEG_TO_RAD * (alon2 - alon1);

	clat1 = cos(geoc_lat1);
	slat1 = sin(geoc_lat1);
	cdlon = cos(rdlon);
	sdlon = sin(rdlon);

	cdel = slat1*slat2 + clat1*clat2*cdlon;
	cdel = (cdel <  1.0) ? cdel :  1.0;
	cdel = (cdel > -1.0) ? cdel : -1.0;
	yazi = sdlon * clat2;
	xazi = clat1*slat2 - slat1*clat2*cdlon;
	ybaz = -sdlon * clat1;
	xbaz = clat2*slat1 - slat2*clat1*cdlon;

	*delta = RAD_TO_DEG * acos(cdel);
	*azi   = RAD_TO_DEG * atan2(yazi, xazi);
	*baz   = RAD_TO_DEG * atan2(ybaz, xbaz);

	if (*azi < 0.0)
		*azi += 360.0;
	if (*baz < 0.0)
		*baz += 360.0;
}

