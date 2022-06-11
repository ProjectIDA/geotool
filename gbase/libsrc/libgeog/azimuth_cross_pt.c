
/*
 * NAME
 *	azimuth_cross_pt -- Find lat/lon crossing point from 2 great circles.

 * FILE
 *	azimuth_cross_pt.c

 * SYNOPSIS
 *	Given 2 reference points with great-circle azimuths, compute their
 *	intersection.

 * DESCRIPTION
 *	Function.  Given the locations of 2 reference points and 2 azimuths 
 *	of great circles passing through those points, this routine computes 
 *	the location of the crossing of 2 great circles and the distances 
 *	from two reference points.  Assuming a spherical Earth model, 
 *	compute a second lat/lon position on the globe given the first 
 *	position and an azimuth. 

 *	---- On entry ----
 *	alat1:	Latitudinal position of point 1 (deg.)
 *	alon1:	Longitudinal position of point 1 (deg.)
 *	alat2:	Latitudinal position of point 2 (deg.)
 *	alon2:	Longitudinal position of point 2 (deg.)
 *	aza:	Azimuth from north of first  great circle path (deg.)
 *	azb:	Azimuth from north of second great circle path (deg.)

 *	---- On return ----
 *	dista:	Distance from  first reference point to crossing point (deg.)
 *	distb:	Distance from second reference point to crossing point (deg.)
 *	alat:	Latitudinal crossing point (deg.)
 *	alon:	Longitudinal crossing point (deg.)

 *	Return:	= 0, All OK
 *		= 1, Lines do not cross within a reasonable distance

 *	---- Functions called ----
 *	Local
 *		dist_azimuth:	Determine the distance between between two
 *				lat./lon. pairs
 *		lat_lon:	Compute a second lat./lon. from first
 *				distance and azimuth

 * DIAGNOSTICS
 *	If either azimuths or distances will not allow the paths to cross, 
 *	then an error message will be returned.

 * NOTES
 *	All arguments are in degrees.  Input latitude and longitude are 
 *	geographic.  Latitude is zero at equator and positive North.
 *	Longitude is positive toward the East azi is measured clockwise 
 *	from local North.  Necessary geocentric conversions are made in 
 *	function, dist_azimuth().

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy, January 1994.
 */

#include "config.h"
#include <math.h>
#include "libgeog.h"

#define	RAD_TO_DEG	(180.0/M_PI)
#define DEG_TO_RAD	(M_PI/180.0)
#define	SIGN(a1, a2)	((a2) >= 0 ? -(a1) : (a1))


void
azimuth_cross_pt_ (double *flat1, double *flon1, double *fza, double *flat2,
double *flon2, double *fzb, double *dista, double *distb, double *alat, 
double *alon, int *ierr)

     /*int	*ierr;*/
     /*double	*flat1, *flat2, *flon1, *flon2, *fza, *fzb;*/
     /*double	*alat, *alon, *dista, *distb;*/
{
	*ierr = azimuth_cross_pt (*flat1, *flon1, *fza, *flat2, *flon2, *fzb,
				  dista, distb, alat, alon);
}


int
azimuth_cross_pt (double alat1,double alon1, double aza, double alat2, 
double alon2, double azb,double *dista, double *distb, double *alat, 
double *alon)

     /*double	alat1, alat2, alon1, alon2, aza, azb;*/
     /*double	*alat, *alon, *dista, *distb;*/

{
	double	alatin, alonin, az, azi, baz, c1, c2, c3, c4, c5; 
        double  delta, dist, e, f, fa, fb, g, h, ra, rb, rc;


	/* Find azimuth, back azimuth and radial distance between stations */

	dist_azimuth (alat1, alon1, alat2, alon2, &delta, &azi, &baz, 0);

	/* Find angle measured from line between two stations to aza and azb */

	fa = aza - azi;
	fb = azb - baz;
	ra = fa;
	rb = fb;

	if (fabs(ra) > 180.0)
		ra = SIGN((360.0-fabs(ra)), ra);
	if (fabs(rb) > 180.0)
		rb = SIGN((360.0-fabs(rb)), rb);

	/*
	 * If the signs of ra and rb are the same, the great circles along
	 * those azimuths will not cross within a "reasonable" distance.
	 */

	if (SIGN(1.0, ra) == SIGN(1.0, rb))
		return (1);

	ra = fabs(ra);
	rb = fabs(rb);

	/*
	 * If the sum of ra and rb is > 180., there will be no crossing
	 * within a reasonable distance
	 */

	if ((ra + rb) > 180.0)
		return (1);

	ra = ra * DEG_TO_RAD;
	rb = rb * DEG_TO_RAD;
	rc = delta * DEG_TO_RAD;

	c1 = tan(0.5*rc);
	c2 = 0.5 * (ra - rb);
	c3 = 0.5 * (ra + rb);

	/* Construct equations for solving for the distances */

	f = c1 * sin(c2);
	g = sin(c3);
	h = c1 * cos(c2);
	e = cos(c3);

	c4 = atan(f/g);
	c5 = atan(h/e);

	/* Compute distances (lengths of the triangle) */

	*distb = (c4 + c5) * RAD_TO_DEG;
	*dista = (c5 - c4) * RAD_TO_DEG;

	if ((*dista < 0.0) || (*distb < 0.0))
		return (1);

	if (*dista < *distb)
	{
		dist   = *dista;
		az     = aza;
		alatin = alat1;
		alonin = alon1;
	}
	else
	{
		dist   = *distb;
		az     = azb;
		alatin = alat2;
		alonin = alon2;
	}

	lat_lon (alatin, alonin, dist, az, alat, alon);

	return (0);
}

