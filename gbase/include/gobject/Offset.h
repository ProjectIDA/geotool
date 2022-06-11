#ifndef _OFFSET_H_
#define _OFFSET_H_

#include "gobject/GMethod.h"

/** 
 *  A GMethod used to record a waveform amplitude offset.
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

	/* Offset members */
	double 		value;
} _Offset, *Offset;

Offset new_Offset(double value);
void Offset_addMethod(void);
GMethod Offset_methodCreate(TimeSeries ts, const char *s);

#endif
