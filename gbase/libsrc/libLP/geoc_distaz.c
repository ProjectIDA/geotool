
/*
 * Copyright (c) 1995 Science Applications International Corporation.
 *

 * NAME
 *	geoc_distaz -- Find distance and azimuth between 2 points on a sphere.

 * FILE
 *	geoc_distaz.c

 * SYNOPSIS
 *	Find the angular distance, azimuth and backazimuth between two 
 *	lat/lon points assuming a spherical Earth.  Based on assumption
 *	that input latitudes are in geocentric co-latitude (radians).

 * DESCRIPTION
 *	Function.  Assuming a spherical Earth model, compute the 
 *	angular distance, azimuth and back azimuth between two
 *	geocentric co-latitude and longitude positions (typically measured 
 *	from a station to an event).  Both a FORTRAN and C interface
 *	exists, geoc_distaz_() and geoc_distaz(), respectively.

 *	---- On entry ----
 *	alat1:		Geocentric co-latitude position of point 1 (radians)
 *	alon1:		Geographic longitude position of point 1 (radians)
 *	alat2:		Geocentric co-latitude position of point 2 (radians)
 *	alon2:		Geographic longitude position of point 2 (radians)
 *	phase_index:	If > 0, 2nd pair of lat/lon coordinates have not 
 *			changed, so, some CPU efficiencies can be achieved 

 *	---- On return ----
 *	delta:		Geocentric distance between points 1 and 2 (radians)
 *	azi:		Geocentric azimuth from north of point 2 w.r.t. point 1 
 *			(clockwise)
 *	baz:		Geocentric back-azimuth from north (clockwise)

 * DIAGNOSTICS
 *	None.

 * NOTES
 *	All arguments are in geocentric radians.  Co-latitude is zero at 
 *	North Pole and positive south.  Longitude is positive toward the 
 *	East azi is measured clockwise from local North.  The first 
 *	lat./lon. pair should represent the station coordinates as a 
 *	general rule.  Thus, the second pair would be the event 
 *	coordinates.  If the 2nd pair of lat/lon coordinates change with 
 *	every event call, simply set phase_index to 0 at every call to 
 *	this function.

 * SEE ALSO
 *	Taken from a similar function, dist_azimuth(), located in 
 *	library, libgeog(3).

 * AUTHOR
 *	Walter Nagy, June 23, 1995.	Created.
 */

#include "config.h"
#include <math.h>
#include "libLP.h"


/* 
 * This function is simply a FORTRAN interface to C routine, geoc_distaz() 
 */

void
geoc_distaz_ (double *flat1, double *flon1, double *flat2, double *flon2, double *delta, double *azi, double *baz, int *ph_index)

     /* int  *ph_index;*/
     /* double	*flat1, *flon1, *flat2, *flon2;*/
     /* double	*azi, *baz, *delta;*/
{
	geoc_distaz (*flat1, *flon1, *flat2, *flon2, delta, azi, baz, 
		     *ph_index);
}


void
geoc_distaz ( double alat1,double  alon1, double alat2, double alon2, double *delta, double *azi,double *baz, int phase_index)

     /*int	phase_index;*/
       /*double	alat1, alon1, alat2, alon2;*/
      /*double	*azi, *baz, *delta;*/
{
	double	clat1, cdlon, cdel, rdlon;
	double	slat1, sdlon, xazi, xbaz, yazi, ybaz;
	static	double	clat2, slat2;


	/*
	 * This is optimized to avoid extra sine and cosine calls when 
	 * alat2 doesn't change between calls (such as looping over a set 
	 * of stations for a given event).
	 */

	if (phase_index == 0)
	{
		clat2 = cos(alat2);
		slat2 = sin(alat2);
	}

	/*
	 * Simple case when both sets of lat/lons are the same.
	 */

	if ((alat1 == alat2) && (alon1 == alon2))
	{
		*delta = 0.0;
		*azi = 0.0;
		*baz = M_PI;
		return;
	}

	rdlon = alon2 - alon1;

	clat1 = cos(alat1);
	slat1 = sin(alat1);
	cdlon = cos(rdlon);
	sdlon = sin(rdlon);

	cdel = clat1*clat2 + slat1*slat2*cdlon;
	cdel = (cdel <  1.0) ? cdel :  1.0;
	cdel = (cdel > -1.0) ? cdel : -1.0;
	yazi = sdlon * slat2;
	xazi = slat1*clat2 - clat1*slat2*cdlon;
	ybaz = -sdlon * slat1;
	xbaz = slat2*clat1 - clat2*slat1*cdlon;

	*delta = acos(cdel);
	*azi   = atan2(yazi, xazi);
	*baz   = atan2(ybaz, xbaz);

	if (*azi < 0.0)
		*azi += 2.0*M_PI;
	if (*baz < 0.0)
		*baz += 2.0*M_PI;
}

