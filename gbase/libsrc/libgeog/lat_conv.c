
/*
 * NAME
 *	lat_conv -- Convert from/to geographic to/from geocentric latitude.

 * FILE
 *	lat_conv.c

 * SYNOPSIS
 *	double
 *	lat_conv (double lat, Bool geog_to_geoc, Bool in_deg, Bool out_deg, 
 *		  Bool in_lat, Bool out_lat)

 * DESCRIPTION
 *	Functions.  This routine is used to convert between geographic and 
 *	geocentric latitudes.  Input and output latitude information can be
 *	geographic or geocentric, in radians or degrees and/or represented 
 *	as latitude or co-latitude.

 *	---- On entry ----
 *	lat:		Input latitude (deg/rad; lat/colat)
 *	geog_to_geoc:	Convert from geographic to geocentric latitude (TRUE)
 *			or geocentric to geographic latitude (FALSE)?
 *	in_deg:		Is input latitude in degrees (TRUE) or radians (FALSE)?
 *	out_deg:	Should output latitude be in degrees (TRUE) or 
 *			radians (FALSE)?
 *	in_lat:		Is input latitude represented as latitude (TRUE) or
 *			co-latitude (FALSE).
 *	out_lat:	Should returned latitude be represented as latitude 
 *			(TRUE) or co-latitude (FALSE).

 *	---- On return ----
 *	Return value:	Output latitude (geog/geoc; deg/rad; lat/colat)

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	Normal latitude is zero at equator and positive North.  Co-latitude 
 *	is zero at North Pole and positive to the south.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy, June 1994.
 */

#include "config.h"
#include <math.h>
#include "libgeog.h"

#define RAD_TO_DEG	(180.0/M_PI)
#define DEG_TO_RAD	(M_PI/180.0)

#define GEOCENTRIC_COLAT(x) \
	(x + (((0.192436*sin(x+x)) + (0.000323*sin(4.0*x)))*DEG_TO_RAD))
#define GEOGRAPHIC_COLAT(x) \
	(x - (((0.192436*sin(x+x)) - (0.000323*sin(4.0*x)))*DEG_TO_RAD))


double
lat_conv (double lat, Bool geog_to_geoc, Bool in_deg, Bool out_deg, 
Bool in_lat, Bool out_lat)

/*Bool	geog_to_geoc, in_deg, in_lat, out_deg, out_lat;*/
/*double	lat;*/

{
	double	glat, glat_gc;


	/*
	 * Is input latitude in degrees?  If so, convert to radians.
	 */

	if (in_deg)
		glat = lat*DEG_TO_RAD; /* Latitude (radians) */
	else
		glat = lat;	       /* Latitude (radians) */

	/*
	 * Is input geographic latitude or co-latitude?  Make sure it is
	 * converted to geocentric co-latitude (in radians), if not already
	 * true.
	 */

	if (in_lat)
		glat = M_PI_2 - glat;	/* Co-latitude (radians) */

	/*
	 * Do geographic to geocentric co-latitude conversion if geog_to_geoc 
	 * is set TRUE; else do geocentric to geographic co-latitude conversion.
	 */

	if (geog_to_geoc)
	{
		glat_gc	= GEOCENTRIC_COLAT(glat);
		glat	= glat_gc;	/* Geocentric co-latitude (radians) */
	}
	else
	{
		glat_gc	= GEOGRAPHIC_COLAT(glat);
		glat	= glat_gc;	/* Geographic co-latitude (radians) */
	}

	if (out_lat)
		glat = M_PI_2 - glat;  	/* Converted latitude (radians) */

	if (out_deg)
		glat = glat*RAD_TO_DEG;	/* Converted lat/co-lat (radians) */

	return (glat);
}

