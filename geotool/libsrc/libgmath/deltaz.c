#include "config.h"
#include <math.h>

#include "libgmath.h"

static double deg_to_radians;

static double azimuth(double rx, double ry, double rz, double sx, double sy,
			double sz);

/**
 * Routine for computing the source-receiver distance and azimuth.
 */
/* for input geographic coordinates slat, slon, rlat, rlon, compute
 * delta, az, baz
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

/** 
 *  Compute source-receiver distance, azimuth and back-azimuth.
 *  @param slat	Source latitude in degrees.
 *  @param slon Source longitude in degrees.
 *  @param rlat Receiver latitude in degrees.
 *  @param rlon Receiver longitude in degrees.
 *  @param delta Returned distance from source to receiver in degrees.
 *  @param az	Returned azimuth from source to receiver in degrees.
 *  @param baz	Returned azimuth from receiver to source in degrees.
 */
void
deltaz(double slat, double slon, double rlat, double rlon, double *delta,
	double *az, double *baz)
{
	double sx, sy, sz, rx, ry, rz, cx, cy, cz, dot, norm, theta, phi;

	deg_to_radians = acos(-1.)/180.;

	*delta = *az = *baz = 0.;
	if(slat == rlat && slon == rlon) return;

	phi = slon*deg_to_radians;
	theta = (90. - geocentric((double)slat))*deg_to_radians;
	sx = sin(theta)*cos(phi);
	sy = sin(theta)*sin(phi);
	sz = cos(theta);

	phi = rlon*deg_to_radians;
	theta = (90. - geocentric((double)rlat))*deg_to_radians;
	rx = sin(theta)*cos(phi);
	ry = sin(theta)*sin(phi);
	rz = cos(theta);

	dot = sx*rx + sy*ry + sz*rz;

	if(dot == 0.) return;

	/* delta = atan( |s x r| / (s * r) )
	 */
	cx = sy*rz - sz*ry;
	cy = sz*rx - sx*rz;
	cz = sx*ry - sy*rx;
	norm = sqrt(cx*cx + cy*cy + cz*cz);

	*delta = atan2(norm, dot)/deg_to_radians;

	*az  = azimuth(rx, ry, rz, sx, sy, sz);
	*baz = azimuth(sx, sy, sz, rx, ry, rz);

	return;
}

static double
azimuth(double rx, double ry, double rz, double sx, double sy, double sz)
{
	double az, nx, ny, nz, bx, by, bz, cx, cy, cz, dot, norm;

	/*
	 * azimuth = the angle between the pole to the great circle through
	 *	(0,0,1) and s and the pole to the great circle through
	 *	r and s.
	 *
	 *  n = north pole cross s (normalized)
	 *  c = r cross s (normalized)
	 *  az = atan( |c x n| / (c * n) )
	 */

	cx = ry*sz - rz*sy;
	cy = rz*sx - rx*sz;
	cz = rx*sy - ry*sx;
	norm = sqrt(cx*cx + cy*cy + cz*cz);

	/* normalize c
	 */
	cx /= norm;
	cy /= norm;
	cz /= norm;

	nx = -sy;
	ny = sx;
	nz = 0.;
	norm = sqrt(nx*nx + ny*ny);
	nx /= norm;
	ny /= norm;
	
	/* b = c x n
	 */
	bx = cy*nz - cz*ny;
	by = cz*nx - cx*nz;
	bz = cx*ny - cy*nx;

	norm = sqrt(bx*bx + by*by + bz*bz);

	dot = cx*nx + cy*ny + cz*nz;

	az = atan2(norm, dot)/deg_to_radians;

	/* if (s dot b) < 0. az = -az
	 */
	dot = sx*bx + sy*by + sz*bz;

	if(dot < 0.) az = -az;

	if(az < 0.) az += 360.;

	return(az);
}
