#ifndef _HCANVASP_H
#define _HCANVASP_H

#include <Xm/Xm.h>
#include <Xm/PrimitiveP.h>
#include "widget/HCanvas.h"
#include "widget/MmTableP.h"
#include "widget/TCanvasP.h"

/// @cond

typedef struct
{
	Pixmap		image;
	Pixmap		drag_image;
	Pixmap		copy_image;
	GC		gc;
	int		copy_x;
	int		inside_column;
	int		mouse_down_column;
	int		mouse_menu_column;
	int		mouse_down_x;
	int		shift;
	int		drag_w;
	int		drag_h;
	int		depth;
	Boolean		moved_column;
	Boolean		moved_state;
	MmTableWidget	table;
	TCanvasWidget	canvas;
	Widget		find_text;
	Widget		find_form;
} HCanvasPart;

typedef struct
{
	int	empty;
} HCanvasClassPart;

typedef struct _HCanvasRec
{
	CorePart	core;
	XmPrimitivePart	primitive;
	HCanvasPart	hCanvas;
} HCanvasRec;

typedef struct _HCanvasClassRec
{
	CoreClassPart		core_class;
	XmPrimitiveClassPart	primitive_class;
	HCanvasClassPart	hCanvas_class;
} HCanvasClassRec;

extern HCanvasClassRec	hCanvasClassRec;

/* ******* from HCanvas.c for use in libwgets only ********/

void _HCanvasRedisplay(HCanvasWidget w);
void _HCanvas_mousePressedMenu(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);

/// @endcond

#endif
