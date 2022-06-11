/*	SccsId:	%W%	%G%	*/

#ifndef _AMP_FACTOR_H_
#define _AMP_FACTOR_H_

#include "gobject/GMethod.h"
#include "libstring.h"

/** 
 *  A GMethod used to record a waveform amplitude multiplicaton factor.
 *
 *  @see new_GMethodPart
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/

typedef struct
{
	GObjectPart	core;
	GMethodPart	gm;

	/* AmpFactor members */
	double 		factor;
	char	 	*comment;
} _AmpFactor, *AmpFactor;

AmpFactor new_AmpFactor(double factor, const char *comment);
void AmpFactor_addMethod(void);

#endif
