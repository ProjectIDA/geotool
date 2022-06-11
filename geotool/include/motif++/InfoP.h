#ifndef _INFO_P_H
#define _INFO_P_H

#include <Xm/XmP.h>
#include "Xm/DrawingAP.h"
#include "motif++/Info.h"

/// @cond
/** 
 *  The Info Widget instance part structure
 */
typedef struct
{
	char		*value;
	Boolean		resize_width;
	XFontStruct	*font;
	GC		gc;
} InfoPart;

/** 
 *  The Info Widget class part structure
 */
typedef struct
{
	XtPointer	extension;
} InfoClassPart;

/** 
 *  The Info Widget full instance structure
 */
typedef struct _InfoRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmDrawingAreaPart	drawing_area;
	InfoPart		info;
} InfoRec;

/** 
 *  The Info Widget full class structure
 */
typedef struct _InfoClassRec
{
	CoreClassPart			core_class;
	CompositeClassPart		composite_class;
	ConstraintClassPart		constraint_class;
	XmManagerClassPart		manager_class;
	XmDrawingAreaClassPart		drawing_area_class;
	InfoClassPart			info_class;
} InfoClassRec;

extern InfoClassRec	infoClassRec;

/// @endcond

#endif
