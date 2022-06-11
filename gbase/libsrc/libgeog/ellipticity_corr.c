 
/*
 * Copyright (c) 1993-1997 Science Applications International Corporation.
 *

 * NAME
 *	ellipticity_corr -- Determine an ellipticity correction.

 * FILE
 *	ellipticity_corr.c

 * SYNOPSIS
 *	double
 *	ellipticity_corr (delta, esaz, ecolat, depth, phase)
 *	double	delta;
 *	double	esaz;
 *	double	ecolat;
 *	double	depth;
 *	char	*phase;

 * DESCRIPTION
 *	Function.  Determine an ellipticity correction (sec) for a given 
 *	phase with a distance (delta) and azimuth from the event to the 
 *	station (esaz), and an event depth (depth).  Uses IASPEI tables 
 *	as originally formulated by Dziewonski and Gilbert (1976) for the
 *	following phases:
 *		P, PcP, PKPab, PKPbc, PKPdf (PKP), PKIKP and S

 *	---- On entry ----
 *	delta:	Event to station distance (deg)
 *	esaz:	Event to station azimuth (deg)
 *	ecolat:	Event co-latitude (radians)
 *	depth:	Event depth (km)
 *	iphs:	Phase-type index (determined externally by find_phase())

 *	---- On return ----
 *	ellip_corr:	Ellipticity correction (sec)

 * DIAGNOSTICS
 *	Only allows the phase type described above.

 * FILES
 *	None.

 * NOTES
 *	The ellipticity correction, as presented here, is a correction in 
 *	time, and not distance.  So, delta is the distance along a great 
 *	circle path assuming a spherical (not elliptical) Earth.  The
 *	correction provided here is "added" to the travel time outside
 *	this routine.  Adapted from anonymously authored FORTRAN code 
 *	contained in B.L.N. Kennett's IASPEI package.

 * SEE ALSO
 *	Tables constants are copied from "IASPEI 1991 Seismological Tables",
 *	compiled by B.L.N. Kennett (1991),

 * AUTHOR
 *	Walter Nagy, March 1993.
 */

#include "config.h"
#include <math.h>
#include "libgeog.h"

#define	MAX_CONSTANTS	10
#define	MAX_P		8
#define	DEG_TO_RAD	0.017453293
#define	EARTH_RADIUS	6371.0
#define	SQRT3_OVER2	0.866025404


double
ellipticity_corr (double delta, double esaz, double ecolat, double depth, 
		  char *phase)
{

	int	iphs;
	double	adepth, azim, edist, ellip_corr = 0.0;
	double	sc0, sc1, sc2, t0, t1, t2;

	static	double	t[][MAX_P][MAX_CONSTANTS] =
	{
				/* t0 constants */
	    {
		 {-0.01711, -1.7791,   0.0000,   0.0000,  0.0000, -0.9630,
		-13.2326,  13.7390,   0.0000,   0.0000},
		 {-0.08291, -2.1455,   2.4538,  -0.7907,  0.0000,  2.0258,
		-12.9357,   2.1287,   5.2668,  -0.9229},
		 {-1.5022,  -0.0943,   1.9655,  -1.1661,  0.1393,  3.4920,
		 -9.9051,  -0.3875,   5.3581,  -0.0686},
	 	 { 2.9971,  -2.9549,   0.4082,   0.0000,  0.0000, 28.1650,
		  9.2160, -17.9030,  -5.2995,   3.2029},
		 { 3.6775,  -2.2221,   0.0000,   0.0000,  0.0000, -1.3127,
		 -6.2476,   1.6684,   0.0000,   0.0000},
		 {-10.6238,  15.4993,  -7.4840,   1.0673,  0.0000,  3.2763,
		 -6.4596,  -0.4923,   0.0000,   0.0000},
		 {-0.01332, -3.2777,  -1.2243,   7.5246,  0.0000, -3.4856,
		-10.3187,  43.4834, -70.5341, -50.2287},
		 {-0.07859, -4.0924,   4.6116,  -1.4760,  0.0000,  2.9104,
		-17.8661,   4.6262,   7.1486,  -1.9154},
	    },
				/* t1 constants */
	    {
		 { 0.0040,  -0.7841,   6.0441, -17.5535,  0.0000, -0.2549,
		  2.0519, -19.0605, -37.8235,  54.5110},
		 {-0.0048,   0.0839,  -2.2705,   2.4137, -0.5957, -2.4241,
		 -4.2792,   1.9728,   3.5644,  -0.5285},
		 { 0.0033,  -1.3485,   0.1735,   1.1583, -0.4162, -0.1096,
		  0.2576,  -0.5978,   0.1888,   0.1600},
		 { 2.6249,  -0.0025,  -0.2086,  -0.0184,  0.0000, -1.5077,
		  0.9904,   0.3513,   0.0000,   0.0000},
		 { 3.4213,  -0.9359,   0.0000,   0.0000,  0.0000,  0.0000,
		  0.0000,   0.0000,   0.0000,   0.0000},
		 {-8.0633,   8.0238,  -1.7407,   0.0000,  0.0000,  0.0000,
		  0.0000,   0.0000,   0.0000,   0.0000},
		 { 0.0109,  -1.2300,   8.9145, -27.5847,  0.0000, -0.6951,
		  5.6201, -33.0908, -83.8233, 102.4333},
		 {-0.0311,   0.1896,  -4.0694,   4.2599, -1.0387, -3.9368,
		 -8.4379,   2.6814,   6.9535,  -0.6086},
	    },
				/* t2 constants */
	    {
		 { 0.0107,   0.0275,  -0.6912,   0.0347,  0.1157, -0.1836,
		  0.0000,   0.0296,   0.0000,   0.0000},
		 { 0.0107,   0.0275,  -0.6912,   0.0347,  0.1157, -0.1836,
		  0.0000,   0.0296,   0.0000,   0.0000},
		 { 0.0005,  -0.01231, -1.0156,   0.4396,  0.0000,  0.0000,
		  0.0000,   0.0000,   0.0000,   0.0000},
		 {-3.5838,   2.1474,  -0.3548,   0.0000,  0.0000, -1.3369,
		 -5.4889,   0.6809,   1.5096,  -0.0763},
		 {-2.9912,   1.0313,   0.0000,   0.0000,  0.0000,  0.0000,
		  0.0000,   0.0000,   0.0000,   0.0000},
		 { 3.2814,  -7.1224,   3.5418,  -0.5115,  0.0000,  0.0000,
		  0.0000,   0.0000,   0.0000,   0.0000},
		 { 0.00025,  0.1685,  -2.2435,   3.3433,  0.0000, -0.0503,
		  0.5353,   1.5362, -14.3118,  -3.2938},
		 { 0.0843,  -0.2917,  -0.6767,  -0.2934,  0.2779, -0.4336,
		  0.0306,  0.07113,   0.0000,   0.0000},
	    },
	};

	/*
	 * First, determine phase-type index
	 */

	if (! strcmp (phase, "P"))
	{
	    if (delta < 15.0)
		iphs = 0;
	    else if (delta < 110.0)
		iphs = 1;
	    else
		iphs = 5;	/* Use the PKPdf branch */
	}
	else if (! strcmp (phase, "PcP"))
	{
	    iphs = 2;
	    if (delta > 90.0)	/* Correction not valid at < 90 deg. */
		return (0.0);
	}
	else if (! strcmp (phase, "PKPab"))
	{
	    iphs = 3;
	    if (delta < 140.0)	/* Correction not valid at < 140 deg. */
		return (0.0);
	}
	else if (! strcmp (phase, "PKPbc"))
	{
	    iphs = 4;
	    if (delta < 140.0 || delta > 160.0)	
		/* Correction not valid except between 140 & 160 deg. */
		return (0.0);
	}
	else if ((! strncmp (phase, "PKP", 3)) || (! strcmp (phase, "PKIKP")))
	{
	    iphs = 5;
	    if (delta < 110.0)	/* Correction not valid at < 110 deg. */
		return (0.0);
	}
	else if (! strcmp (phase, "S"))
	{
	    if (delta < 15.0)
		iphs = 6;
	    else if (delta < 110.0)
		iphs = 7;
	    else			/* Correction not valid at > 110 deg. */
		return (0.0);
	}
	else
	    return (0.0);	/* No ellipt. correction exists for this phase*/


	edist  = delta*DEG_TO_RAD;		/* Event to station distance */
	azim   = esaz*DEG_TO_RAD;		/* Event to station azimuth */

	/*
	 * Set up reference constants
	 */

	sc0 = 0.25*(1.0 + 3.0*cos(2.0*ecolat));
	sc1 = SQRT3_OVER2*sin(2.0*ecolat);
	sc2 = SQRT3_OVER2*sin(ecolat)*sin(ecolat);

	adepth = depth/EARTH_RADIUS;

	/*
	 * Compute tau coefficients of Dziewonski and Gilbert (1976).
	 */

	t0 = t[0][iphs][0] + edist*(t[0][iphs][1] + edist*(t[0][iphs][2]
		+ edist*(t[0][iphs][3] + edist*t[0][iphs][4])))
		+ adepth*(t[0][iphs][5] + adepth*t[0][iphs][6])
		+ adepth*edist*(t[0][iphs][7] + t[0][iphs][8]*adepth
		+ t[0][iphs][9]*edist);
	t1 = t[1][iphs][0] + edist*(t[1][iphs][1] + edist*(t[1][iphs][2]
		+ edist*(t[1][iphs][3] + edist*t[1][iphs][4])))
		+ adepth*(t[1][iphs][5] + adepth*t[1][iphs][6])
		+ adepth*edist*(t[1][iphs][7] + t[1][iphs][8]*adepth
		+ t[1][iphs][9]*edist);
	t2 = t[2][iphs][0] + edist*(t[2][iphs][1] + edist*(t[2][iphs][2]
		+ edist*(t[2][iphs][3] + edist*t[2][iphs][4])))
		+ adepth*(t[2][iphs][5] + adepth*t[2][iphs][6])
		+ adepth*edist*(t[2][iphs][7] + t[2][iphs][8]*adepth
		+ t[2][iphs][9]*edist);

	/*
	 * Compute ellipticity correction via equations (22) and (26) 
	 * of Dziewonski and Gilbert (1976).
	 */

	ellip_corr = sc0*t0 + sc1*cos(azim)*t1 + sc2*cos(2.0*azim)*t2;

/*
	printf ("dist = %5.1f  t0 = %7.3f  t1 = %7.3f  t2 = %7.3f  ellip_corr = %7.3f\n", delta, t0, t1, t2, ellip_corr);
 */

	return (ellip_corr);
}

