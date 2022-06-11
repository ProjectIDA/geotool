#ifndef _CUT_H_
#define _CUT_H_

#include "gobject/GMethod.h"

/** 
 *  A GMethod used to record a cut waveform operation.
 *
 *  @see new_GMethodPart
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/

typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	/* Cut members */
	double 		begSelect;
	double		endSelect;
} _Cut, *Cut;

Cut new_Cut(double begSelect, double endSelect);
void Cut_addMethod(void);

#endif
