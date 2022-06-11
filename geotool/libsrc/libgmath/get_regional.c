#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libgmath.h"

/**
 * Compute the regional travel times and derivative for a crust model.
 * @param crust Input CrustModel.
 * @param phase Input phase name.
 * @param depth Input depth in kilometers.
 * @param npts Output number of times computed.
 * @param t Output travel time values. t must hold up tp 400 values.
 * @param d Output distance values (degrees). d must hold up tp 400 values.
 */
void
get_regional(CrustModel *crust, const char *phase, double depth, int *npts,
		float *t, float *d)
{
	int i;
	double delta;
	float time;
	Derivatives der;

	*npts = 0;
	delta = .001;
	if(!regional(crust, phase, delta, depth, &time, &der))
	{
		t[*npts] = time;
		d[*npts] = delta;
		*npts = *npts + 1;
	}
	for(i = 1; i < 400; i++)
	{
		delta = .05*(double)i;
		if(!regional(crust, phase, delta, depth, &time, &der))
		{
			t[*npts] = time;
			d[*npts] = delta;
			*npts = *npts + 1;
		}
	}
}
