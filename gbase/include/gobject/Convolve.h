/*	SccsId:	%W%	%G%	*/

#ifndef _CONVOLVE_H_
#define _CONVOLVE_H_

#include "response.h"
#include "gobject/GMethod.h"

/** 
 *  A GMethod used to record a convolution or deconvolution operation.
 *  The direction of the operation (convolution or deconvolution),
 *  the sequence of Response structures and the frequency band is stored.
 *
 *  @see new_GMethodPart
 *  @see new_GObjectPart
 *  @see Response
 *
 *
 */

typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	/* Convolve members */
	int		direction;
	Vector		responses;
	float 		flo;
	float		fhi;
	char		instype[7];
} _Convolve, *Convolve;

Convolve new_Convolve(int direction, Vector resp, char *instype, float flo,
			float fhi);
Convolve new_ConvolveOne(int direction, Response resp, char *instype, float flo,
			float fhi);
Convolve new_Convolve_fromString(char *s);
GMethod Deconvolve_methodCreate(TimeSeries ts, const char *s);

#endif
