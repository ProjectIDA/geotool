#ifndef _CRUST_H_
#define _CRUST_H_

typedef struct
{
	char	name[5];
	char	full_name[64];
	float	h[2];
	float	vp[3];
	float	vs[3];
} CrustModel;

typedef struct
{
	float	dtdd;
	float	dtdh;
	float	dpdd;
	float	dpdh;
	float   dtds0;
	float   dtds1;
	float   dtds2;
	float   dtdh0;
	float   dtdh1;
} Derivatives;

#ifndef PI
#define PI		3.14159265358979323846
#endif
#define HALF_PI         1.57079632679489661923
#define DEG_TO_KM       111.19492664            /* for R = 6371 */
#define DEG_TO_RADIANS  0.01745329252

#ifndef TIME_RESIDUAL_TOO_BIG
#include "tterror.h"
#endif

#endif
