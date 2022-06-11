#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libgmath.h"

static void eflat(double *h, double *v, int nlayer);
static int ray_p(double delta0, int n, double *vel, double *h, double *rayp,
			double *dpdd);
static double tt_star(double p, int n, double *v, double *h, Derivatives *dd);

#define TOL	.00001
#define MAXIT	500

/** 
 *  Compute travel time and derivatives for layered model. Compute travel time
 *  and derivatives for upgoing phases in 2 layers over a halfspace.
 *  The travel time derivatives are:
 *
 *  <pre>
 *	Derivatives	*dd	 dd->dtdh : sec/km
 *				 dd->dtdd : sec/deg
 *				 dd->dpdd : (sec/deg)/deg
 *				 dd->dpdh : (sec/deg)/km
 *				 dd->dtds0 : d(t)/d(1/v0) : km
 *				 dd->dtds1 : d(t)/d(1/v1) : km
 *				 dd->dtds2 : d(t)/d(1/v2) : km
 *				 dd->dtdh0 : sec/km
 *				 dd->dtdh1 : sec/km
 *  </pre>
 *
 * @param crust	 A CrustModel with velocities in km/sec, depths in km
 * @param phase The phase name "Px" or "Sx".
 * @param delta	The distance in degrees.
 * @param depth	The depth in kilometers.
 * @param tt (output) travel time in seconds.
 * @param dd (output) travel time derivatives.
 * @return 0 for success or a nonzero error code: PHASE_UNKNOWN, \
 *	DELTA_OUTSIDE_RANGE
 */
int
ttup(CrustModel *crust, const char *phase, double delta, double depth,
	double *tt, Derivatives *dd)
{
	int i, err, nlayer;
	double h[3], v[3], vel[3];
	double r, vr, vr3, dpdd, p, p1, p2, t1, t2, depth1, depth2;
	Derivatives d;

	if(!strcmp(phase, "Px"))
	{
		for(i = 0; i < 3; i++) v[i] = vel[i] = crust->vp[i];
	}
	else if(!strcmp(phase, "Sx"))
	{
		for(i = 0; i < 3; i++) v[i] = vel[i] = crust->vs[i];
	}
	else
	{
		return(PHASE_UNKNOWN);
	}
	delta *= DEG_TO_KM;

	if(depth <= crust->h[0])
	{
		/* source in the top layer.
		 */
		r = sqrt(delta*delta + depth*depth);
		*tt = r/v[0];
		vr = v[0]*r;
		dd->dtdd = delta/vr;
		dd->dtdh = depth/vr;
		dd->dtds0 = r;
		dd->dtds1 = 0.;
		dd->dtds2 = 0.;
		dd->dtdh0 = 0.;
		dd->dtdh1 = 0.;
		vr3 = v[0]*r*r*r;
		dd->dpdd = ((depth*depth)/vr3);
		dd->dpdh = (-delta*depth/vr3);
		return(0);
	}
	else
	{
		/* Source in 2nd layer or halfspace.
		 */
		if(depth <= crust->h[0] + crust->h[1])
		{
			h[0] = crust->h[0];
			h[1] = depth - crust->h[0];
			eflat(h, v, 2);
			nlayer = 2;
		}
		else
		{
			h[0] = crust->h[0];
			h[1] = crust->h[1];
			h[2] = depth - crust->h[0] - crust->h[1];
			eflat(h, v, 3);
			nlayer = 3;
		}

		if((err = ray_p(delta, nlayer, v, h, &p, &dpdd)) != 0)
		{
			return(err);
		}
		*tt = tt_star(p, nlayer, v, h, dd);

		dd->dpdd = dpdd;

		/*
		 * Compute numeric derivatives for dtdd, dtdh and dpdh.
		 */
		if(delta < .1 || ray_p(delta-.1, nlayer, v, h, &p1, &dpdd))
		{
			p1 = p;
			ray_p(delta+.1, nlayer, v, h, &p2, &dpdd);
		}
		else if(ray_p(delta+.1, nlayer, v, h, &p2, &dpdd))
		{
			p2  = p;
			ray_p(delta-.1, nlayer, v, h, &p1, &dpdd);
		}
		t1 = tt_star(p1, nlayer, v, h, &d);
		t2 = tt_star(p2, nlayer, v, h, &d);
		dd->dtdd = (t2 - t1)/.2;

		if(depth <= crust->h[0] + crust->h[1])
		{
			depth1 = depth - .5;
			if(depth1 < crust->h[0]) depth1 = crust->h[0];
			h[0] = crust->h[0];
			h[1] = depth1 - crust->h[0];
			v[0] = vel[0];
			v[1] = vel[1];
			eflat(h, v, 2);
			ray_p(delta, 2, v, h, &p1, &dpdd);
			t1 = tt_star(p1, 2, v, h, &d);

			depth2 = depth + .5;
			if(depth2 > crust->h[0] + crust->h[1])
			{
				depth2 = crust->h[0] + crust->h[1];
			}
			h[0] = crust->h[0];
			h[1] = depth2 - crust->h[0];
			v[0] = vel[0];
			v[1] = vel[1];
			eflat(h, v, 2);
			ray_p(delta, 2, v, h, &p2, &dpdd);
			t1 = tt_star(p2, 2, v, h, &d);
		}
		else
		{
			depth1 = depth - .5;
			if(depth1 < crust->h[0] + crust->h[1])
			{
				depth1 = crust->h[0] + crust->h[1];
			}
			h[0] = crust->h[0];
			h[1] = crust->h[1];
			h[2] = depth1 - crust->h[0] - crust->h[1];
			v[0] = vel[0];
			v[1] = vel[1];
			v[2] = vel[2];
			eflat(h, v, 3);
			ray_p(delta, 3, v, h, &p1, &dpdd);
			t1 = tt_star(p1, 3, v, h, &d);

			depth2 = depth + .5;

			h[0] = crust->h[0];
			h[1] = crust->h[1];
			h[2] = depth2 - crust->h[0] - crust->h[1];
			v[0] = vel[0];
			v[1] = vel[1];
			v[2] = vel[2];
			eflat(h, v, 3);
			ray_p(delta, 3, v, h, &p2, &dpdd);
			t1 = tt_star(p2, 3, v, h, &d);
		}
		dd->dtdh = (t2 - t1)/(depth2 - depth1);
		dd->dpdh = (p2 - p1)/(depth2 - depth1);
	}
	dd->dtdd *= DEG_TO_KM;		 /*  sec/deg */
	dd->dpdd *= DEG_TO_KM*DEG_TO_KM; /* (sec/deg)/deg */
	dd->dpdh *= DEG_TO_KM;		 /* (sec/deg)/km */

	return(0);
}

static void
eflat(
	double *h,
	double *v,
	int   nlayer)
{
	/* Simple earth-flattening transformation
	 */
	int i;
	double q, z, z2;

	for(i = 0, z = 0; i < nlayer; i++)
	{
		z2 = z + 0.5*h[i];
		z += h[i];
		q = 6371./(6371. - z2);
		v[i] *= q;
		h[i] *= q;
	}
}

static int
ray_p(double delta0, int n, double *vel, double *h, double *rayp, double *dpdd)
{
	int i, iter, j;
	double delta, p, p1, f, fmin, dfdp, eta[3], arg;

	/*
	 * Start at p = sin(45 degrees)/v and use Newton's method to
	 * find the root of f(p) = delta(p) - delta0 = 0;
	 */
	iter = 0;

	/* find acceptable starting angle
	 */
	fmin = 1.e+20;
	p1 = 0.;
	for(j = 1; j < 90; j++)
	{
		p = sin(j*DEG_TO_RADIANS)/vel[n-1];
		for(i = 0; i < n; i++)
		{
			arg = 1./(vel[i]*vel[i]) - p*p;
			if(arg < 0.) break;
			eta[i] = sqrt(arg);
		}
		if(i < n) continue;
		for(i = 0, delta = 0.; i < n; i++) delta += h[i]*p/eta[i];
		f = delta - delta0;
		if(fabs(f) < fmin)
		{
			fmin = fabs(f);
			p1 = p;
		}
	}
	if(p1 == 0.) return(DELTA_OUTSIDE_RANGE);

	do
	{
		p = p1;
		for(i = 0; i < n; i++)
		{
			arg = 1./(vel[i]*vel[i]) - p*p;
			if(arg < 0.) return(DELTA_OUTSIDE_RANGE);
			eta[i] = sqrt(arg);
		}
		delta = 0.;
		for(i = 0; i < n; i++) delta += h[i]*p/eta[i];
		f = delta - delta0;

		dfdp = 0.;
		for(i = 0; i < n; i++)
		{
			dfdp += h[i]*(1./eta[i] + p*p/pow(eta[i],3./2.));
		}

		p1 = p - f/dfdp;

	} while(fabs((p1-p)/p) > TOL && ++iter < MAXIT);

	if(iter == MAXIT) return(DELTA_OUTSIDE_RANGE);

	*rayp = p;
	*dpdd = 1./dfdp;
	return(0);
}

static double
tt_star(double p, int n, double *v, double *h, Derivatives *dd)
{
	double tt, a, b, c;

	if(n == 2)
	{
		a = h[0]/sqrt(1.-v[0]*v[0]*p*p);
		b = h[1]/sqrt(1.-v[1]*v[1]*p*p);
		tt = a/v[0] + b/v[1];
		dd->dtds0 = a;
		dd->dtds1 = b;
		dd->dtds2 = 0.;
		dd->dtdh0 = a/h[0];
		dd->dtdh1 = 0.;
	}
	else
	{
		a = h[0]/sqrt(1.-v[0]*v[0]*p*p);
		b = h[1]/sqrt(1.-v[1]*v[1]*p*p);
		c = h[2]/sqrt(1.-v[2]*v[2]*p*p);
		tt = a/v[0] + b/v[1] + c/v[2];
		dd->dtds0 = a;
		dd->dtds1 = b;
		dd->dtds2 = c;
		dd->dtdh0 = a/h[0];
		dd->dtdh1 = b/h[1];
	}
	return(tt);
}
