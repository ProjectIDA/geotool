#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libgmath.h"

#define sign(a,b) ((b >= 0.) ? a : -a)

/** 
 *  Convert geographic latitude to geocentric latitude.
 *  @param geographic_lat geographic latitude in degrees.
 *  @return geocentric latitude in degrees.
 */
double
geocentric(double geographic_lat)
{
	double geocentric_lat;
	double deg_to_radians;
	static double pi2 = 1.570796327;

	deg_to_radians = acos(-1.)/180.;

	geographic_lat *= deg_to_radians;

	geocentric_lat = (pi2 - fabs(geographic_lat) >= .05) ?
		atan(0.993277*sin(geographic_lat)/cos(geographic_lat)) :
		geographic_lat/0.993277 - sign(0.01063193, geographic_lat);

	return(geocentric_lat/deg_to_radians);
}

/** 
 *  Convert geocentric latitude to geographic latitude.
 *  @param geocentric_lat geocentric latitude in degrees.
 *  @return geographic latitude in degrees.
 */
double
geographic(double geocentric_lat)
{
	double geographic_lat;
	double deg_to_radians;
	static double pi2 = 1.570796327;

	deg_to_radians = acos(-1.)/180.;

	geocentric_lat *= deg_to_radians;

	geographic_lat = (pi2 - fabs(geocentric_lat) >= .05) ?
		atan(sin(geocentric_lat)/(0.993277*cos(geocentric_lat))) :
		0.993277*(geocentric_lat + sign(0.01063193, geocentric_lat));

	return(geographic_lat/deg_to_radians);
}
