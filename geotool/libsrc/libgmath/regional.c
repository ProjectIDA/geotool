#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libgmath.h"

#define N_FIRST_LOOP	25
#define MAX_ITER	100

/** Compute regional phase travel times.
 *  Returns travel time and derivatives for Px, Sx, Pg, Sg, P*, S*, Pn, Sn,
 *  PIP and PMP for a plane layered model consisting of two layers in the crust
 *  and a mantle velocity
 *  <pre>
 *  returns:	0 - success
 *		DEPTH_OUTSIDE_RANGE 	- depth out of range.
 *		DELTA_OUTSIDE_RANGE 	- delta out of range.
 *		PHASE_UNKNOWN 		- bad input phase
 *		CRUSTMODEL_BAD 		- bad input crust
 *
 *	CrustModel	*crust,		velocities in km/sec, depths in km
 *	char		*phase,
 *	float		delta,		degrees
 *	float		depth,		km
 *	float		*ttime,		sec
 *	Derivatives	*dd)		dd->dtdh : sec/km
 *					dd->dtdd : sec/deg
 *					dd->dpdd : (sec/deg)/deg
 *					dd->dpdh : (sec/deg)/km
 *					dd->dtds0 : d(t)/d(1/v0) : km
 *					dd->dtds1 : d(t)/d(1/v1) : km
 *					dd->dtds2 : d(t)/d(1/v2) : km
 *  </pre>
 */
int
regional(CrustModel *crust, const char *phase, double delta, double depth,
	float *ttime, Derivatives *dd)
{
	int i, iter;
        double dist, sini, sinj, cosi, cosj, tani, tanj,
                sin_limit, dsin, del, hp, a, b, vr3;
	double f, fp, theta, theta1;
	static double eps = .00001;
	static double half_pi = 1.570796326;
	float *v;

	if(phase == NULL || (int) strlen(phase) < 2)
	{
		return(PHASE_UNKNOWN);
	}

	if(delta == 0.)
	{
		return(DELTA_OUTSIDE_RANGE);
	}

	if(delta < 0) delta = -delta;

	for(i = 0; i < 3; i++)
	{
		if(crust->vp[i]<0. || crust->vs[i]<0.) return(CRUSTMODEL_BAD);
	}
	for(i = 0; i < 2; i++) if(crust->h[i] < 0.) return(CRUSTMODEL_BAD);

	if(phase[0] == 'P')
	{
		v = crust->vp;
	}
	else if(phase[0] == 'S')
	{
		v = crust->vs;
	}
	else
	{
		return(PHASE_UNKNOWN);
	}
	for(i = 1; i < 3; i++) if(v[i] < v[i-1]) return(CRUSTMODEL_BAD);

	if(phase[1] == 'x')
	{
		a = (double)delta;
		i = ttup(crust, phase, a, (double)depth, &b, dd);
		if(!i) *ttime = b;
		return(i);
	}

	delta *= DEG_TO_KM;

	if(phase[1] == 'g')
	{
		/* this is the same as ttup with depth < h[0]
		 */
		if(depth > crust->h[0]) return(DEPTH_OUTSIDE_RANGE);
		dist = sqrt(delta*delta + depth*depth);
		if(dist == 0.) return(DELTA_OUTSIDE_RANGE);

		*ttime = dist/v[0];

		dd->dtdd = delta/(v[0]*dist);
		dd->dtdh = depth/(v[0]*dist);
		dd->dtds0 = dist;
		dd->dtds1 = 0.;
		dd->dtds2 = 0.;
		dd->dtdh0 = 0.;
		dd->dtdh1 = 0.;
		vr3 = v[0]*pow(dist, 3.);
		dd->dpdd = depth*depth/vr3;
		dd->dpdh = -depth*delta/vr3;
	}
	else if(phase[1] == '*')
	{
		if(depth > crust->h[0]) return(DEPTH_OUTSIDE_RANGE);

		sini = v[0]/v[1];
		cosi = sqrt(1. - sini*sini);
		tani = sini/cosi;

		f = delta - (2.*crust->h[0]-depth)*tani;
		if(f < 0.)
		{
			return(DELTA_OUTSIDE_RANGE);
		}

		*ttime = (2.*crust->h[0] - depth)/(v[0]*cosi) + f/v[1];

		dd->dtdd = 1./v[1];
		dd->dtdh = tani/v[1] - 1./(v[0]*cosi);
                dd->dtds0 = (2.*crust->h[0] - depth)/cosi;
                dd->dtds1 = f;
                dd->dtds2 = 0.;
                dd->dtdh0 = 2./(v[0]*cosi) - 2.*tani/v[1];
                dd->dtdh1 = 0.;
		dd->dpdd = 0.;
		dd->dpdh = 0.;
	}
	else if(phase[1] == 'n')
	{
		if(depth > crust->h[0]+crust->h[1]) return(DEPTH_OUTSIDE_RANGE);

		sinj = v[1]/v[2];
		cosj = sqrt(1. - sinj*sinj);
		tanj = sinj/cosj;
		sini = v[0]/v[2];
		cosi = sqrt(1. - sini*sini);
		tani = sini/cosi;
		dd->dtdd = 1./v[2];
		dd->dpdd = 0.;
		dd->dpdh = 0.;
                dd->dtdh0 = 0.;
                dd->dtdh1 = 0.;
 
		if(depth < crust->h[0])
		{
			f = delta - (2.*crust->h[0]-depth)*tani -
					2.*crust->h[1]*tanj;
			if(f < 0.) return(DELTA_OUTSIDE_RANGE);

			*ttime = (2.*crust->h[0]-depth)/(v[0]*cosi)
					+ 2.*crust->h[1]/(v[1]*cosj) + f/v[2];
			dd->dtdh = tani/v[2] - 1./(v[0]*cosi);
                        dd->dtds0 = (2.*crust->h[0]-depth)/cosi;
                        dd->dtds1 = 2.*crust->h[1]/cosj;
                        dd->dtds2 = f;
                        dd->dtdh0 = 2./(v[0]*cosi) - 2.*tani/v[2];
                        dd->dtdh1 = 2./(v[1]*cosj) - 2.*tanj/v[2];
		}
		else
		{
			hp = depth - crust->h[0];
			f = delta - crust->h[0]*tani - (2.*crust->h[1]-hp)*tanj;
			if(f < 0.) return(DELTA_OUTSIDE_RANGE);

			*ttime = crust->h[0]/(v[0]*cosi) +
				(2.*crust->h[1] - hp)/(v[1]*cosj) + f/v[2];
			dd->dtdh = tanj/v[2] - 1./(v[1]*cosj);
                        dd->dtds0 = crust->h[0]/cosi;
                        dd->dtds1 = (2.*crust->h[1] - hp)/cosj;
                        dd->dtds2 = f;
                        dd->dtdh0 = 1./(v[0]*cosi) - tani/v[2];
                        dd->dtdh1 = 2./(v[1]*cosj) - 2.*tanj/v[2];
		}
	}
	else if(phase[1] == 'I')
	{
		if(depth > crust->h[0]) return(DEPTH_OUTSIDE_RANGE);

		hp = 2*crust->h[0] - depth;
		f =  sqrt(delta*delta + hp*hp);
		sini = delta/f;
		/* critical angle */
		if(sini > v[1]/v[0]) return(DELTA_OUTSIDE_RANGE);

		cosi = hp/f;
		*ttime = hp/(v[0]*cosi);
		dd->dtdh = - 1./(v[0]*cosi);
		dd->dtdd = delta*cosi/(hp*v[0]);
		dd->dpdd = (f - delta*delta/f)/(v[0]*f*f);
		dd->dpdh = -(f - hp*delta/f)/(v[0]*f*f);
                dd->dtdh0 = 2./(v[0]*cosi);
                dd->dtdh1 = 0.;
	}
	else if(phase[1] == 'M')
	{
		if(depth > crust->h[0]+crust->h[1]) return(DEPTH_OUTSIDE_RANGE);

		if(depth < crust->h[0])
		{
			a = 2.*crust->h[0] - depth;
			b = 2.*crust->h[1];
		}
		else
		{
			a = crust->h[0];
			b = 2.*crust->h[1] - (depth - crust->h[0]);
		}
		sin_limit = .99*v[0]/v[1];
	
		dsin = sin_limit/(N_FIRST_LOOP-1);
		for(i = 0; i < N_FIRST_LOOP; i++)
		{
			sini = i*dsin;
			cosi = sqrt(1. - sini*sini);
			tani = sini/cosi;
			sinj = sini*v[1]/v[0];
			cosj = sqrt(1. - sinj*sinj);
			tanj = sinj/cosj;
			del = a*tani + b*tanj;

			if(del > delta) break;
		}
		if(i == N_FIRST_LOOP) return(DELTA_OUTSIDE_RANGE);

		theta1 = asin(sini);
		if(theta1 >= half_pi)
		{
			return(DELTA_OUTSIDE_RANGE);
		}
		iter = 0;
		/* use newton's method to get root. */
		do
		{
			theta = theta1;
			sini = sin(theta);
			if(sini > sin_limit) return(DELTA_OUTSIDE_RANGE);
			cosi = sqrt(1. - sini*sini);
			tani = sini/cosi;
			sinj = sini*v[1]/v[0];
			cosj = sqrt(1. - sinj*sinj);
			tanj = sinj/cosj;
			f = a*tani + b*tanj - delta;
			fp = a/(cosi*cosi) + b/(cosj*cosj);
			theta1 = theta - f/fp;
			if(theta1 >= half_pi)
			{
				return(DELTA_OUTSIDE_RANGE);
			}
			iter++;
		}
		while(fabs((theta1-theta)/theta) > eps && iter < MAX_ITER);

		if(iter == MAX_ITER) return(DELTA_OUTSIDE_RANGE);
	
		*ttime = a/(v[0]*cosi) + b/(v[1]*cosj);
		dd->dtdh = (depth < crust->h[0]) ?
				-1./(v[0]*cosi) : -1./(v[1]*cosj);
/*
		dd->dtdd =
		dd->dpdd =
		dd->dpdh =
*/
		dd->dtdd = dd->dpdd = dd->dpdh = 0.;
	}
		
	dd->dtdd *= DEG_TO_KM;		 /*  sec/deg */
	dd->dpdd *= DEG_TO_KM*DEG_TO_KM; /* (sec/deg)/deg */
	dd->dpdh *= DEG_TO_KM;		 /* (sec/deg)/km */

	return(0);
}
