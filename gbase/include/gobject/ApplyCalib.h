/*	SccsId:	%W%	%G%	*/

#ifndef _APPLY_CALIB_H_
#define _APPLY_CALIB_H_

#include "gobject/GMethod.h"
#include "libstring.h"

/** 
 *  A GMethod used to apply the calib factor to a TimeSeries.
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

	/* No ApplyCalib members needed. */
} _ApplyCalib, *ApplyCalib;

ApplyCalib new_ApplyCalib(void);
void ApplyCalib_addMethod(void);
GMethod ApplyCalib_methodCreate(TimeSeries ts, const char *s);

#endif
