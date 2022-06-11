/*	SccsId:	%W%	%G%	*/

#ifndef _COPY_H_
#define _COPY_H_

#include "gobject/GMethod.h"

/** 
 *  A GMethod used to record a copy waveform operation.
 *
 *  @see new_GMethodPart
 *  @see new_GObjectPart
 *
 *
 */

typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	/* Copy members */
	double 		begSelect;
	double		endSelect;
} _Copy, *Copy;

Copy new_Copy(double begSelect, double endSelect);
void Copy_addMethod(void);

#endif
