#ifndef _TCANVASP_H
#define _TCANVASP_H

#include <Xm/Xm.h>
#include <Xm/PrimitiveP.h>

#include "widget/TCanvas.h"
#include "widget/MmTableP.h"

/// @cond

typedef struct
{
	char		cursor_type[20];
	int		inside_row;
	int		mouse_down_row;
	int		mouse_drag_row;
	int		depth;
	int		choice_menu_row;
	int		choice_menu_column;
	Pixmap		image;
	Boolean		verticalLines;
	Boolean		horizontalLines;
	Boolean		selecting;
	Boolean		singleSelect;
	Boolean		radioSelect;
	Boolean		cntrlSelect;
	Boolean		dragSelect;
	GC		gc;
	MmTableSelectCallbackStruct select_data;
	MmTableWidget	table;
} TCanvasPart;

typedef struct
{
	int	empty;
} TCanvasClassPart;

typedef struct _TCanvasRec
{
	CorePart	core;
	XmPrimitivePart	primitive;
	TCanvasPart	tCanvas;
} TCanvasRec;

typedef struct _TCanvasClassRec
{
	CoreClassPart		core_class;
	XmPrimitiveClassPart	primitive_class;
	TCanvasClassPart	tCanvas_class;
} TCanvasClassRec;

extern TCanvasClassRec	tCanvasClassRec;

/* ******** from TCanvas.c for use in libwgets only ********/

void _TCanvasRepaint(TCanvasWidget w);
void _TCanvasRedraw(TCanvasWidget w);
void _TCanvasDrawRows(TCanvasWidget w, int i1, int i2, int j1, int j2);
void _TCanvasRedisplay(TCanvasWidget w);
void _TCanvasVScroll(TCanvasWidget w);
int  _TCanvasGetRight(TCanvasWidget w, int left);
void _TCanvasHScroll(TCanvasWidget w);
void _TCanvasUpdateRow(TCanvasWidget w, int i);
void _TCanvasSelectRow(TCanvasWidget w, int row, Boolean state);
void _TCanvasSelectRows(TCanvasWidget w, int num_rows, int *rows,
			Boolean *states);
void _TCanvasHighlightField(TCanvasWidget w, int row, int col, Boolean b);
void _TCanvas_drawToggle(TCanvasWidget w, int i);
void _TCanvasDrawRowLabels(TCanvasWidget w);
void _TCanvasDrawXLabels(TCanvasWidget w);

/// @endcond

#endif
