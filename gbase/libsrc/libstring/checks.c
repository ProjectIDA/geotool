#include "config.h"
#include <stdlib.h> 
#include <string.h> 
#include <math.h> 
#include <ctype.h>  

#include "libstring.h"

#ifndef True
#define	True	1
#endif
#ifndef False
#define	False	0
#endif

/**
 * Utility routines for converting a string to numbers within a specific range.
 */

/**
 * Convert string to latitude (in degrees). The latitude must be between
 * -90.0 and 90.0.
 * @param str Input string.
 * @val Returned latitude.
 * @return 1 for success, 0 for invalid latitude.
 */
int	
stringToLatitude(const char *str, double *val)
{
	double	tval;

	if (!stringIsNumber(str)) return(False);
	tval= atof(str);
	if (tval < -90.0 || tval > 90.0) 
		return(False);
	else
		*val = tval;
	return(True);
}

/**
 * Convert string to longitude (in degrees). The longitude must be between
 * -180.00001 and 180.00001.
 * @param str Input string.
 * @val Returned longitude.
 * @return 1 for success, 0 for invalid longitude.
 */
int
stringToLongitude(const char *str, double *val)
{
	if (!stringIsNumber(str)) return(False);
	*val = atof(str);
	return((*val >= -180.00001 && *val <= 180.00001) ? True : False);
}

/**
 * Convert string to depth (in km). The depth must be between -900.0 and 0.0.
 * @param str Input string.
 * @val Returned depth.
 * @return 1 for success, 0 for invalid depth.
 */
int	
stringToDepth(const char *str, double *val)
{
	double	tval;

	if (!stringIsNumber(str)) return(False);
	tval= atof(str);
	if (tval < -900.0 || tval > 0.0) 
		return(False);
	else
		*val = tval;
	return(True);
}

/**
 * Test if a string represents a number. Does not test for exponential format.
 * @ param str Input string.
 * @return 1 if the string represents a number, 0 if not.
 */
int
stringIsNumber(const char *str)
{
	int i;

	for (i=0; i<(int)strlen(str); i++)
	{
	    if (!isdigit((int)str[i]) && !isspace((int)str[i]) && str[i] != '.'
			&& str[i] != '-') return(False);
	}
	return(True);
}

/**
 * Test if a string is only spaces and periods ('.').
 * @ param str Input string.
 * @return 1 if the string is only spaces and periods, 0 otherwise.
 */
int
stringIsOnlySpaces(const char *str)
{
	int i;

	for (i=0; i<(int)strlen(str); i++)
	{
		if (!isspace((int)str[i]) && str[i] != '.') return(False);
	}
	return(True);
}

/**
 * Convert string to a distance in degrees. The number must be between
 * 0.0 and 180.
 * @param str Input string.
 * @val Returned number.
 * @return 1 for success, 0 for invalid number.
 */
int	
stringToDistanceDeg(const char *str, double *val)
{
	if (!stringIsNumber(str)) return(False);
	*val = atof(str);
	if (*val >= 0.0 && *val < 180.0) 
		return(True);
	return(False);
}

/**
 * Convert string to a distance in kilometers. The distance must be between
 * 0.0 and 20015.10.
 * @param str Input string.
 * @val Returned distance.
 * @return 1 for success, 0 for invalid distance.
 */
int	
stringToDistanceKm(const char *str, double *val)
{
	if (!stringIsNumber(str)) return(False);
	*val = atof(str);
	if (*val > 0.0 && *val < 20015.10) 
		return(True);
	return(False);
}

/**
 * Convert string to a azimuth in degrees. The azimuth must be between
 * -360.0 and 360.0.
 * @param str Input string.
 * @val Returned azimuth.
 * @return 1 for success, 0 for invalid azimuth.
 */
int	
stringToAzimuth(const char *str, double *val)
{
	if (!stringIsNumber(str)) return(False);
	*val = atof(str);
	if (*val > -360.0 && *val < 360.0) 
		return(True);
	return(False);
}
