
/*
 * Copyright (c) 1995 Science Applications International Corporation.

 * NAME
 *	dist_given_2angles_plus_side -- Self explanatory.

 * FILE
 *	non_Naperian.c

 * SYNOPSIS
 *	double
 *	dist_given_2angles_plus_side (angle_wrt_to_baz, angle_wrt_to_azi,
 *				      side_between_angles)
 *	double	angle_wrt_to_baz;	(i) radians
 *	double	angle_wrt_to_azi;	(i) radians
 *	double	side_between_angles;	(i) radians

 * DESCRIPTION
 *	Assuming a spherical Earth model, compute lat/lon positions,
 *	distance and/or azimuths (forward and back) using oblique-
 *	angled ("non-Naperian") trigonometric relations.

 *	-- dist_given_2angles_plus_side() will calculate a distance on
 *	a spherical Earth using non-Naperian trigonometry, given one
 *	side and the 2 nearest angles of an oblique-angled triangle 
 *	mapped on sphere.  The resultant distance (radians) will be 
 *	opposite side_between_angles nearest angle, angle_wrt_to_azi.

 * DIAGNOSTICS
 *	-- dist_given_2angles_plus_side() will return the distance 
 *	(radians) of the side common to angle_wrt_to_azi, but opposite 
 *	side_between_angles.

 * FILES
 *	None.

 * NOTES
 *	All arguments are in radians.  All input arguments are assumed
 *	to be relative to a geocentric coordinate system.  Other 
 *	functions will be added to this file as needed.

 * SEE ALSO
 *	Formulas taken from "Stereographic Projection in Structural
 *	Geology", by F.C. Phillips (1955), pp 69-70.

 * AUTHOR
 *	Walter Nagy, June 27, 1995	Created.
 */

#include "config.h"
#include <math.h>
#include <float.h>
#include "libLP.h"


double
dist_given_2angles_plus_side (double angle_wrt_to_baz, double angle_wrt_to_azi,
			      double side_between_angles)
{

	double	A_minus_B_over_2, A_plus_B_over_2, delta_over_2;
	double	a_plus_b, a_minus_b;


	A_minus_B_over_2 = (angle_wrt_to_baz - angle_wrt_to_azi) / 2.0;
	A_plus_B_over_2 = (angle_wrt_to_baz + angle_wrt_to_azi) / 2.0;
	delta_over_2 = side_between_angles / 2.0;

	if (A_plus_B_over_2 == 0.0 || A_plus_B_over_2 == M_PI_2)
	    A_plus_B_over_2 += DBL_EPSILON;

	a_plus_b = atan ((cos(A_minus_B_over_2)*tan(delta_over_2))/
				cos(A_plus_B_over_2));
	a_minus_b = atan ((sin(A_minus_B_over_2)*tan(delta_over_2))/
				sin(A_plus_B_over_2));

	return (a_plus_b + a_minus_b);
}
