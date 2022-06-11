/*
 * NAME
 *      euler() and euler2()
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include "libgmath.h"

#ifndef PI
#define PI 3.14159265358979323846 
#endif

#define even(i) ((i-2*((i)/2)==0) ? 1:0)
#define odd(i) ((i-2*((i)/2)==0) ? 0:1)

static void reduce(double *pth, double *pph);

/**
 * Routines for computing coordinates in a rotated coordinate system and the
 * rotation matrix.
 */

/** 
 *  Compute rotated coordinates.
 *  Compute the coordinates corresponding to the input theta,phi in a
 *  coordinate system rotated by Euler angles alpha,beta,gamma with
 *  respect to the original system. (see Edmonds p7 and p53)
 *  Input theta,phi,a,b,g  in radians.
 *  @return values for theta: 0. to PI, and for phi: -PI to PI.
 */
void
euler(double *theta, double *phi, double a, double b, double g)
{
	static int first = 1;
	double alpha, beta, gamma, x, y, z, xp, yp, zp;
	static double c[3][3], olda, oldb, oldg;

	if(first || a != olda || b != oldb || g != oldg)
	{
		first = 0; olda = a; oldb = b; oldg = g;
		/*
		 * Euler angles of old system from new system.
		 */
		alpha = -g;
		beta = -b;
		gamma = -a;
		/*
		 * Set up rotation matrix.
		 */
		rotation_matrix(alpha, beta, gamma, c);
	}
	x = sin(*theta)*cos(*phi);
	y = sin(*theta)*sin(*phi);
	z = cos(*theta);
	xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;
	zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	*theta = atan2(sqrt(xp*xp + yp*yp), zp);
	*phi = atan2(yp, xp);
	reduce(theta, phi);
	return;
}

/** 
 * Do two sets of euler angle rotations.
 * This is useful when rotation angle are always defined with respect to
 * the original coordinate system, and you therefore need to unrotate before
 * rotating with the new angles.
 * <p>
 * Rotate theta0, phi0 with alpha, beta, gamma:
 * <pre>
 * euler(&theta0, &phi0, alpha, beta, gamma);
 * </pre>
 * Rotate the new theta0, phi0 using alpha2, beta2, gamma2 with respect to the
 * original system.
 * <pre>
 * euler2(&theta0, &phi0, -gamma, -beta, -alpha, alpha2, beta2, gamma2);
 * </pre>
 */
void
euler2(double *theta, double *phi, double a1, double b1, double g1, double a2,
	double b2, double g2)
{
	static int first = 1;
	int i, j, k;
	double alpha, beta, gamma, x, y, z, xp, yp, zp, sum;
	double c1[3][3], c2[3][3];
	static double c[3][3], olda1, oldb1, oldg1, olda2, oldb2, oldg2;

	if(first || a1 != olda1 || b1 != oldb1 || g1 != oldg1 ||
			a2 != olda2 || b2 != oldb2 || g2 != oldg2)
	{
		first = 0; olda1 = a1; oldb1 = b1; oldg1 = g1;
			olda2 = a2; oldb2 = b2; oldg2 = g2;
		/*
		 * Euler angles of old system from new system.
		 */
		alpha = -g1;
		beta = -b1;
		gamma = -a1;
		/*
		 * Set up rotation matrices.
		 */
		rotation_matrix(alpha, beta, gamma, c1);
		alpha = -g2;
		beta = -b2;
		gamma = -a2;
		rotation_matrix(alpha, beta, gamma, c2);
		/*
		 * c = c2 * c1;
		 */
		for(i = 0; i < 3; i++)
		{
			for(j = 0; j < 3; j++)
			{	
				for(k = 0, sum = 0.; k < 3; k++)
					sum += c2[i][k]*c1[k][j];
				c[i][j] = sum;
			}
		}
	}
	x = sin(*theta)*cos(*phi);
	y = sin(*theta)*sin(*phi);
	z = cos(*theta);
	xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;
	zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	*theta = atan2(sqrt(xp*xp + yp*yp), zp);
	*phi = atan2(yp, xp);
	reduce(theta, phi);
	return;
}

/** 
 * Compute the rotation matrix. Compute the rotation matrix corresponding
 * to the three Euler angles.
 */
void
rotation_matrix(double alpha, double beta, double gamma, double c[3][3])
{
	int i, j, k;
	double sum;
	double ra[3][3], rb[3][3], rg[3][3], rbra[3][3];

	rg[0][0] = cos(gamma);
	rg[0][1] = sin(gamma);
	rg[1][0] = -rg[0][1];
	rg[1][1] = rg[0][0];
	rg[2][2] = 1.;
	rg[0][2] = rg[1][2] = rg[2][0] = rg[2][1] = 0.;
	rb[0][0] = cos(beta);
	rb[0][2] = -sin(beta);
	rb[2][0] = -rb[0][2];
	rb[1][1] = 1.;
	rb[2][2] = rb[0][0];
	rb[0][1] = rb[1][0] = rb[1][2] = rb[2][1] = 0.;
	ra[0][0] = cos(alpha);
	ra[0][1] = sin(alpha);
	ra[1][0] = -ra[0][1];
	ra[1][1] = ra[0][0];
	ra[2][2] = 1.;
	ra[0][2] = ra[1][2] = ra[2][0] = ra[2][1] = 0.;

	for(i = 0; i < 3; i++)	/* multiply rb * ra */
	{
		for(j = 0; j < 3; j++)
		{	
			for(k=0,sum=0.; k<3; k++)
				sum += rb[i][k]*ra[k][j];
			rbra[i][j] = sum;
		}
	}
	for(i = 0; i < 3; i++)	/* multiply rg * rb * ra */
	{
		for(j = 0; j < 3; j++)
		{
			for(k = 0,sum = 0.; k < 3; k++)
				sum += rg[i][k]*rbra[k][j];
			c[j][i] = sum;
		}
	}
	return;
}

/** 
 *  Force theta between 0. and PI, and phi between 0. and 2.*PI
 */
static void
reduce(double *pth, double *pph)
{
	int i;
	double th, ph;
	double two_pi = 2.*PI;

	th = *pth; ph = *pph;
	i = abs((int)(ph/two_pi));
	if(ph < 0.) ph += (i+1)*two_pi;
	else if(ph > two_pi) ph -= i*two_pi;
	*pph = ph;
	if(th < 0. || th > PI)
	{
		i = (int)(th/PI);
		if(th > 0.)
		{
			if(odd(i))
			{
				th = (i+1)*PI - th;
				ph = (ph < PI) ? ph + PI : ph - PI;
			}
			else th -= i*PI;
		}
		else
		{
			if(even(i))
			{
				th = -th + i*PI;
				ph = (ph < PI) ? ph + PI : ph - PI;
			}
			else th -= i*PI;
		}
		*pth = th; *pph = ph;
	}
	return;
}
