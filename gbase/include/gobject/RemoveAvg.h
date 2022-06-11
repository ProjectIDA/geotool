#ifndef _REMOVEAVG_H_
#define _REMOVEAVG_H_

#include "gobject/GMethod.h"

/** 
 *  A GMethod used to remove the average from a waveform.
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
        double min_time;
        double max_time;
  
} _RemoveAvg, *RemoveAvg;

RemoveAvg new_RemoveAvg(double min_time, double max_time);
/* void RemoveAvg_addMethod(void);
 */
GMethod RemoveAvg_methodCreate(TimeSeries ts, const char *s);

#endif
