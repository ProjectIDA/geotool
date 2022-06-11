/** \file TCanvas.c
 *  \brief Defines widget TCanvas.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      TCanvas Widget -- widget for displaying tabular data.
 *
 * SYNOPSIS
 *      #include "TCanvas.h"
 *      Widget
 *      TCanvasCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      TCanvas.h
 *
 * AUTHOR
 *      I. Henson -- February 2000
 *	Multimax, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include "Xm/RowColumn.h"
#include "Xm/PushB.h"

extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
#include "libstring.h"
#include "libtime.h"
#include "motif++/Info.h"
}
#include "widget/TCanvasP.h"
#include "widget/Table.h"

#define offset(field)	XtOffset(TCanvasWidget, tCanvas.field)
static XtResource	resources[] =
{
    {XtNtableWidget, XtCTableWidget, XtRWidget, sizeof(Widget),
	offset(table), XtRImmediate, (XtPointer)NULL},
    {XtNverticalLines, XtCVerticalLines, XtRBoolean,
	sizeof(Boolean), offset(verticalLines), XtRString, (XtPointer)"True"},
    {XtNhorizontalLines, XtCHorizontalLines, XtRBoolean,
	sizeof(Boolean), offset(horizontalLines), XtRString, (XtPointer)"True"},
    {XtNsingleSelect, XtCSingleSelect, XtRBoolean,
	sizeof(Boolean), offset(singleSelect), XtRString, (XtPointer) "False"},
    {XtNradioSelect, XtCRadioSelect, XtRBoolean,
	sizeof(Boolean), offset(radioSelect), XtRString, (XtPointer)"False"},
    {XtNcntrlSelect, XtCCntrlSelect, XtRBoolean,
	sizeof(Boolean), offset(cntrlSelect), XtRString, (XtPointer)"False"},
    {XtNdragSelect, XtCDragSelect, XtRBoolean,
	sizeof(Boolean), offset(dragSelect), XtRString, (XtPointer)"True"},
};
#undef offset

/* private functions */

static void Initialize(Widget req, Widget new_index);
static Boolean SetValues(TCanvasWidget cur, TCanvasWidget req,
			TCanvasWidget new_index);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static void Resize(Widget widget);
static void Redisplay(Widget widget, XEvent *event, Region region);
static void Destroy(Widget widget);
static void fillSides(TCanvasWidget w);
static void mouseMoved(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseDraggedButton3(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseDraggedButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mousePressedButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseActionButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mousePressedButton2(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mousePressedButton3(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static Boolean mousePressed3(TCanvasWidget w, XEvent *event, Boolean action);
static void mouseReleasedButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseReleasedButton3(TCanvasWidget w, XEvent *event, String *params,
			 Cardinal *num_params);
static void LeaveWindow(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void EnterWindow(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void GainedFocus(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void LostFocus(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void KeyPressed(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void TCanvasMotion(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void positionEditCursor(TCanvasWidget w, XEvent *event, int col,int row);
static void editCursor(XtPointer client_data, XtIntervalId id);
static void drawEditCursor(TCanvasWidget w, int x, int on, int col, int row);
static void highlightRow(TCanvasWidget w, int row, Boolean on);
static int stringWidth(TCanvasWidget w, String s);
static int stringWidthLen(TCanvasWidget w, String s, int len);
static String substring(TCanvasWidget w, String s, int i1, int i2);
static void printInfo(TCanvasWidget w, Boolean scroll);
static void *MallocIt(TCanvasWidget w, int nbytes);
static void *ReallocIt(TCanvasWidget w, void *ptr, int nbytes);
static void TCanvasScroll(TCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params);
static void flipChoice(MmTablePart *t, char *choice, int i, int j);
static void SetCursorMode(TCanvasWidget w, int x, int y);
static void TCanvasSetCursor(TCanvasWidget w, char *type);
static void ChoiceSelectCB(Widget widget, XtPointer client, XtPointer data);
static void TCanvasDisplayChoiceMenu(TCanvasWidget w, XEvent *event,
			char *choice, int row, int column);
static void TCanvasCellSelect(TCanvasWidget w, XEvent *event, int row, int col);
static void TCanvasCellEnter(TCanvasWidget w, XEvent *event, int row, int col);
static int getFirstLine(TCanvasWidget w);

enum CellDir
{
    CELL_RIGHT,
    CELL_LEFT,
    CELL_DOWN,
    CELL_UP
};
static void nextCell(TCanvasWidget w, enum CellDir dir);
static void page(TCanvasWidget w, KeySym keysym);
static void Page(TCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params);
static void Scroll(TCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params);


static char defTrans[] =
" ~Shift ~Ctrl<Btn1Down>:mousePressedButton1() \n\
  ~Ctrl Shift<Btn1Down>:mousePressedButton1(Shift) \n\
  ~Shift Ctrl<Btn1Down>:mousePressedButton1(Ctrl) \n\
  <Btn1Up>:		mouseReleasedButton1() \n\
  <Btn1Motion>:		mouseDraggedButton1() \n\
  <Btn2Down>:		mousePressedButton2() \n\
  <Btn3Down>:		mousePressedButton3() \n\
  <Btn3Up>:		mouseReleasedButton3() \n\
  <Btn3Motion>:		mouseDraggedButton3() \n\
  <MouseMoved>:		mouseMoved() \n\
  <EnterWindow>:	EnterWindow() \n\
  <LeaveWindow>:	LeaveWindow() \n\
  <FocusIn>:		GainedFocus() \n\
  <FocusOut>:		LostFocus() \n\
  Ctrl<Key>f:		Page(down)\n\
  Ctrl<Key>b:		Page(up)\n\
  Ctrl<Key>d:		KeyScroll(down)\n\
  Ctrl<Key>u:		KeyScroll(up)\n\
  <KeyDown>:		KeyPressed() \n\
  <BtnDown>:		Scroll()";

static char singleSelectTrans[] =
" ~Shift ~Ctrl<Btn1Down>:mousePressedButton1(Ctrl) \n\
  ~Ctrl Shift<Btn1Down>:mousePressedButton1(Ctrl) \n\
  ~Shift Ctrl<Btn1Down>:mousePressedButton1(Ctrl) \n\
  <Btn1Up>:		mouseReleasedButton1() \n\
  <Btn2Down>:		mousePressedButton2() \n\
  <Btn3Down>:		mousePressedButton3() \n\
  <Btn3Up>:		mouseReleasedButton3() \n\
  <Btn3Motion>:		mouseDraggedButton3() \n\
  <MouseMoved>:		mouseMoved() \n\
  <EnterWindow>:	EnterWindow() \n\
  <LeaveWindow>:	LeaveWindow() \n\
  <FocusIn>:		GainedFocus() \n\
  <FocusOut>:		LostFocus() \n\
  Ctrl<Key>f:		Page(down)\n\
  Ctrl<Key>b:		Page(up)\n\
  Ctrl<Key>d:		KeyScroll(down)\n\
  Ctrl<Key>u:		KeyScroll(up)\n\
  <KeyDown>:		KeyPressed() \n\
  <BtnDown>:		Scroll()";

static char cntrlSelectTrans[] =
" ~Shift ~Ctrl<Btn1Down>:mousePressedButton1(Ctrl) \n\
  ~Ctrl Shift<Btn1Down>:mousePressedButton1(Shift) \n\
  ~Shift Ctrl<Btn1Down>:mousePressedButton1(Ctrl) \n\
  <Btn1Up>:		mouseReleasedButton1() \n\
  <Btn1Motion>:		mouseDraggedButton1() \n\
  <Btn2Down>:		mousePressedButton2() \n\
  <Btn3Down>:		mousePressedButton3() \n\
  <Btn3Up>:		mouseReleasedButton3() \n\
  <Btn3Motion>:		mouseDraggedButton3() \n\
  <MouseMoved>:		mouseMoved() \n\
  <EnterWindow>:	EnterWindow() \n\
  <LeaveWindow>:	LeaveWindow() \n\
  <FocusIn>:		GainedFocus() \n\
  <FocusOut>:		LostFocus() \n\
  Ctrl<Key>f:		Page(down)\n\
  Ctrl<Key>b:		Page(up)\n\
  Ctrl<Key>d:		KeyScroll(down)\n\
  Ctrl<Key>u:		KeyScroll(up)\n\
  <KeyDown>:		KeyPressed() \n\
  <BtnDown>:		Scroll()";

static XtActionsRec actionsList[] =
{
    {(char *)"mousePressedButton1",	(XtActionProc)mousePressedButton1},
    {(char *)"mouseReleasedButton1",	(XtActionProc)mouseReleasedButton1},
    {(char *)"mouseDraggedButton1",	(XtActionProc)mouseDraggedButton1},
    {(char *)"mousePressedButton3",	(XtActionProc)mousePressedButton3},
    {(char *)"mousePressedButton2",	(XtActionProc)mousePressedButton2},
    {(char *)"mouseReleasedButton3",	(XtActionProc)mouseReleasedButton3},
    {(char *)"mouseDraggedButton3",	(XtActionProc)mouseDraggedButton3},
    {(char *)"mouseMoved",		(XtActionProc)mouseMoved},
    {(char *)"EnterWindow",		(XtActionProc)EnterWindow},
    {(char *)"LeaveWindow",		(XtActionProc)LeaveWindow},
    {(char *)"GainedFocus",		(XtActionProc)GainedFocus},
    {(char *)"LostFocus",		(XtActionProc)LostFocus},
    {(char *)"Page",			(XtActionProc)Page},
    {(char *)"KeyScroll",		(XtActionProc)Scroll},
    {(char *)"KeyPressed",		(XtActionProc)KeyPressed},
    {(char *)"Scroll",			(XtActionProc)TCanvasScroll},
};


TCanvasClassRec	tCanvasClassRec = 
{
    {	/* core fields */
	(WidgetClass)(&xmPrimitiveClassRec),	/* superclass */
	(char *)"TCanvas",			/* class_name */
	sizeof(TCanvasRec),		/* widget_size */
	NULL,				/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
        actionsList,                    /* actions */
        XtNumber(actionsList),          /* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
	NULLQUARK,			/* xrm_class */
	TRUE,				/* compress_motion */
	TRUE,				/* compress_exposure */
	TRUE,				/* compress_enterleave */
	TRUE,				/* visible_interest */
	Destroy,			/* destroy */
	Resize,				/* resize */
	Redisplay,			/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
        defTrans,                       /* tm_table */
	NULL,				/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL				/* extension */
    },
    {	/* primitive_class fields */
	(XtWidgetProc)_XtInherit,	/* Primitive border_highlight */
	(XtWidgetProc)_XtInherit,	/* Primitive border_unhighlight */
	NULL,				/* translations */
	NULL,				/* arm_and_activate */
	NULL,				/* get resources */
	0,				/* num get_resources */
	NULL,
    },
    {	/* TCanvasClass fields */
	0,		/* empty */
    },
};

WidgetClass tCanvasWidgetClass = (WidgetClass)&tCanvasClassRec;

static Widget choice_popup=NULL;

static void
Initialize(Widget w_req, Widget w_new)
{
    TCanvasWidget new_index = (TCanvasWidget)w_new;
    TCanvasPart *tc = &new_index->tCanvas;

    if(new_index->core.width == 0)
	new_index->core.width = (Dimension)(XWidthOfScreen(XtScreen(new_index))/2.5);
    if(new_index->core.height == 0)
	new_index->core.height = XHeightOfScreen(XtScreen(new_index))/2;

    tc->inside_row = -1;
    tc->mouse_down_row = -1;
    tc->mouse_drag_row = -1;
    tc->image = (Pixmap)NULL;
    tc->depth = -1;
    tc->select_data.states = (Boolean *)NULL;
    tc->select_data.changed_rows = (int *)NULL;
    tc->selecting = False;
    tc->gc = (GC)NULL;
    stringcpy(tc->cursor_type, "default", sizeof(tc->cursor_type));

    if(tc->radioSelect) tc->singleSelect = True;

    if(tc->singleSelect || !tc->dragSelect) {
	XtTranslations translations;
	translations = XtParseTranslationTable(singleSelectTrans);
	XtOverrideTranslations(w_new, translations);
    }
    else if(tc->cntrlSelect) {
	XtTranslations translations;
	translations = XtParseTranslationTable(cntrlSelectTrans);
	XtOverrideTranslations(w_new, translations);
    }
}

void
_TCanvasSetTranslations(TCanvasWidget widget)
{
    TCanvasPart *tc = &widget->tCanvas;
    XtTranslations translations;

    if(!tc->singleSelect && tc->dragSelect && !tc->cntrlSelect) {
	translations = XtParseTranslationTable(defTrans);
	XtOverrideTranslations((Widget)widget, translations);
    }
    else if(tc->singleSelect || !tc->dragSelect) {
	translations = XtParseTranslationTable(singleSelectTrans);
	XtOverrideTranslations((Widget)widget, translations);
    }
    else if(tc->cntrlSelect) {
	translations = XtParseTranslationTable(cntrlSelectTrans);
	XtOverrideTranslations((Widget)widget, translations);
    }
}

static Boolean
SetValues(TCanvasWidget cur, TCanvasWidget req, TCanvasWidget new_index)
{
    /* nothing to do. */
    return False;
}


static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    TCanvasWidget w = (TCanvasWidget)widget;
    Pixel   white, black;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    XWindowAttributes window_attributes;

    (*((tCanvasWidgetClass->core_class.superclass)->core_class.realize))
	((Widget)w,  valueMask, attrs);

    XGetWindowAttributes(XtDisplay(w), XtWindow(w), &window_attributes);

    tc->depth = window_attributes.depth;

    tc->gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
    /*
     * check that no foreground colors are same as background
     */
    white = string_to_pixel(widget, "White");
    black = string_to_pixel(widget, "Black");
    if(w->primitive.foreground == t->table_background) {
	w->primitive.foreground = (t->table_background != black) ?
		black : white;
    }

    XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
    XSetBackground(XtDisplay(w), tc->gc, w->core.background_pixel);


    XSetFont(XtDisplay(w), tc->gc, t->font->fid);

    /* important to create pixmap with (width+1,height+1) for calls to
     * XFillRectangle(... width, height)
     * Otherwise get bad problems with some X-servers
     */
    tc->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
		w->core.width+1, w->core.height+1, tc->depth);
    XSetForeground(XtDisplay(w), tc->gc, w->core.background_pixel);
    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 0, 0,
                w->core.width, w->core.height);
    XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
}

static void
Resize(Widget widget)
{
    TCanvasWidget w = (TCanvasWidget)widget;
    TCanvasPart *tc = &w->tCanvas;

    if(!XtIsRealized((Widget)w)) return;

    if(tc->image != (Pixmap)NULL) {
	XFreePixmap(XtDisplay(w), tc->image);
	tc->image = (Pixmap)NULL;
    }
    if(w->core.width <= 0 || w->core.height <= 0) return;

    /* important to create pixmap with (width+1,height+1) for calls to
     * XFillRectangle(... width, height)
     * Otherwise get bad problems with some X-servers
     */
    tc->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
		w->core.width+1, w->core.height+1, tc->depth);
    XSetForeground(XtDisplay(w), tc->gc, w->core.background_pixel);
    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 0, 0,
                w->core.width, w->core.height);
    _TCanvasRedraw(w);
}

static void
Redisplay(Widget widget, XEvent	*event, Region	region)
{
    TCanvasWidget w = (TCanvasWidget)widget;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(!XtIsRealized(widget) || w->core.width <= 0 || w->core.height <= 0) {
	return;
    }

    if(tc->image == (Pixmap)NULL) {
	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
        tc->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
			w->core.width+1, w->core.height+1, tc->depth);
	XSetForeground(XtDisplay(w), tc->gc, w->core.background_pixel);
	XFillRectangle(XtDisplay(w), tc->image, tc->gc, 0, 0,
                w->core.width, w->core.height);
	_TCanvasRedraw(w);
    }
    if(t->needAdjustColumns == 1) {
	if(t->ncols > 0) {
	    MmTableAdjustColumnWidths(tc->table, 0, t->ncols-1);
	}
    }
    _TCanvasRepaint(w);
}

/** 
 *  
 */
void
_TCanvasRepaint(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(tc->image != (Pixmap)NULL) {
	XCopyArea(XtDisplay(w), tc->image, XtWindow(w),
	    tc->gc, 0, 0, w->core.width, w->core.height, 0, 0);
    }
    _TCanvasDrawRowLabels(w);
    if(t->color_only) {
	XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
		w->core.height-1);
    }
}

void
_TCanvasDrawRowLabels(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    XRectangle r;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(w->core.x > 0) {
	XSetForeground(XtDisplay(w), t->gc, w->core.background_pixel);
	XFillRectangle(XtDisplay(w), XtWindow(tc->table), t->gc, 0, 0,
		w->core.x, w->core.y + w->core.height);
    }
    r.x = 0;
    r.y = w->core.y;
    r.width = XtWidth(tc->table);
    r.height = w->core.height;
    XSetClipRectangles(XtDisplay(w), t->gc, 0, 0, &r, 1, Unsorted);

    if(t->select_toggles)
    {
    	int i;
	for(i = t->top; i <= t->bottom && i < t->nrows; i++)
	{
	    _TCanvas_drawToggle(w, i);
	}
	XSetForeground(XtDisplay(w), t->gc, t->fg);
    }

    if(t->displayRowLabels)
    {
	int i, x;
	XSetForeground(XtDisplay(w), t->gc, w->primitive.foreground);
	for(i = t->top; i <= t->bottom; i++) {
	    int ro = t->row_order[i];
	    int y = w->core.y + (i - t->top)*t->row_height;
	    char *label = t->row_labels[ro];
	    if(label != NULL && label[0] != '\0') {
		x = w->core.x - t->row_label_width + 2;
	        XDrawString(XtDisplay(tc->table), XtWindow(tc->table), t->gc,
		    x, y + t->max_ascent+3, label, (int)strlen(label));
	    }
	}
	XSetForeground(XtDisplay(w), t->gc, t->fg);
    }
    if(t->toggle_label != NULL && strlen(t->toggle_label) > 0)
    {
	int x;
	r.x = 0;
	r.y = 0;
	r.width = t->toggle_label_width + 10;
	r.height = t->row_height;
	XSetClipRectangles(XtDisplay(w), t->gc, 0, 0, &r, 1, Unsorted);

	x = (w->core.x + 1 - t->toggle_label_width)/2;
	XSetForeground(XtDisplay(w), t->gc, w->primitive.foreground);
	XDrawString(XtDisplay(tc->table), XtWindow(tc->table), t->gc,
		x, t->toggle_label_y,
		t->toggle_label, (int)strlen(t->toggle_label));
	XSetForeground(XtDisplay(w), t->gc, t->fg);
    }
}


void
_TCanvas_drawToggle(TCanvasWidget w, int i)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    XRectangle r;
    int ro, x, y;

    if(i < 0 || i >= t->nrows) return;
    r.x = 0;
    r.y = w->core.y;
    r.width = XtWidth(tc->table);
    r.height = w->core.height;
    XSetClipRectangles(XtDisplay(w), t->gc, 0, 0, &r, 1, Unsorted);

    if(t->toggle_label_width > 0 && t->row_label_width == 0)
    {
	x = (w->core.x +1)/2 - 8;
	if(x < 0) x = 0;
    }
    else {
	x = w->core.x - t->row_label_width - 16;
    }
    y = w->core.y + (i - t->top)*t->row_height;
    y += (int)(.5*t->row_height - 6);
    ro = t->row_order[i];

    if(!t->row_state[ro]) {
	XSetForeground(XtDisplay(w), t->gc, w->core.background_pixel);
	XFillRectangle(XtDisplay(tc->table), XtWindow(tc->table),
		t->gc, x, y, 12, 12);
	XSetForeground(XtDisplay(w), t->gc, w->primitive.top_shadow_color);
    }
    else {
	XSetForeground(XtDisplay(w), t->gc, w->primitive.bottom_shadow_color);
	XFillRectangle(XtDisplay(tc->table), XtWindow(tc->table),
			t->gc, x, y, 12, 12);
	XSetForeground(XtDisplay(w), t->gc, w->primitive.highlight_color);
    }
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table),t->gc,x,  y,  x+12,y);
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table),t->gc,x,  y+1,x+11,y+1);
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table),t->gc,x,  y,  x+0,y+12);
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table),t->gc,x+1,y,  x+1,y+11);
    if(!t->row_state[ro]) {
	XSetForeground(XtDisplay(w), t->gc, w->primitive.highlight_color);
    }
    else {
	XSetForeground(XtDisplay(w), t->gc, w->primitive.top_shadow_color);
    }
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table), t->gc,
			x+1, y+12, x+12, y+12);
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table), t->gc,
			x+2, y+11, x+12, y+11);
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table), t->gc,
			x+12, y+1, x+12, y+12);
    XDrawLine(XtDisplay(tc->table), XtWindow(tc->table), t->gc,
			x+11, y+2, x+11, y+11);
}

static void
Destroy(Widget widget)
{
    TCanvasWidget w = (TCanvasWidget)widget;
    TCanvasPart *tc = &w->tCanvas;

    if(tc->gc != NULL) XFreeGC(XtDisplay(w), tc->gc);

    if(tc->image != (Pixmap)NULL) {
        XFreePixmap(XtDisplay(w), tc->image);
    }
    Free(tc->select_data.states);
    Free(tc->select_data.changed_rows);
}

static void
fillSides(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    Pixel bg = w->core.parent->core.background_pixel;

    if(t->ncols > 0) {
	int width = t->col_end[t->right] - t->col_beg[t->left] + t->margin + 1;
	if(width < w->core.width) {
	    XSetForeground(XtDisplay(w), tc->gc, bg);
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc, width, 0,
		w->core.width - width, w->core.height);
	}
    }
    if(t->nrows > 0) {
	int y = (t->bottom - t->top+1)*t->row_height + 1;
	if(y < w->core.height) {
	    XSetForeground(XtDisplay(w), tc->gc, bg);
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 0, y,
		w->core.width, w->core.height - y);
	}
    }
    if(t->ncols == 0 || t->nrows == 0) {
	XSetForeground(XtDisplay(w), tc->gc, bg);
	XFillRectangle(XtDisplay(w), tc->image, tc->gc, 0, 0,
                w->core.width, w->core.height);
    }
}

/** 
 *  
 */
void
_TCanvasRedraw(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(!XtIsRealized((Widget)w)) return;

    if(t->row_height == 0) {
        int direction;
        XCharStruct overall;
        XTextExtents(t->font, "M", 1, &direction, &t->max_ascent,
                &t->max_descent, &overall);
        t->row_height = t->max_ascent + t->max_descent + 4;
    }

    t->top = MmTableGetBarValue(t->vbar);
    t->bottom = t->top + MmTableGetBarVisibleAmount(t->vbar);
    if(t->bottom >= t->nrows-1) t->bottom = t->nrows-1;
    t->left = MmTableGetBarValue(t->hbar);
    if(t->left >= t->ncols) t->left = t->ncols-1;
    if(t->left < 0) t->left = 0;
    t->right = _TCanvasGetRight(w, t->left);
    fillSides(w);

    _TCanvasDrawRows(w, t->top, t->bottom, t->left, t->right);
}

#define mod(a,b) (((a)/(b))*(b)==a)

/** 
 *  
 */
void
_TCanvasDrawRows(TCanvasWidget w, int i1, int i2, int j1, int j2)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    XmPrimitivePart *p = &w->primitive;
    Display *dp = XtDisplay(w);

    int i, j, j0, fill_x, fill_width, select_fill_x, select_fill_width;

    if(	i1 < 0 || i1 >= t->nrows || i2 < 0 || i2 >= t->nrows ||
	j1 < 0 || j1 >= t->ncols || j2 < 0 || j2 >= t->ncols) return;

    if(j1 == t->left) {
	if(j1 == 0) {
	    fill_x = 2;
	    fill_width = t->col_end[j2] - t->col_beg[j1] + t->margin - 3;
	}
	else {
	    fill_x = 0;
	    fill_width = t->col_end[j2] - t->col_beg[j1] + t->margin - 1;
	}
    }
    else {
	fill_x = t->col_beg[j1] - t->col_beg[t->left] + t->margin + 1;
	fill_width = t->col_end[j2] - t->col_beg[j1] - 2;
    }
    if(fill_x + fill_width > w->core.width) fill_width = w->core.width - fill_x;

    select_fill_x = (j1 == 0) ? fill_x-1 : fill_x;
    select_fill_width = (j2 == t->ncols-1) ? fill_width : fill_width+1;

    j0 = (t->color_only) ? getFirstLine(w) : t->left;

    for(i = i1; i <= i2; i++)
    {
	int ro = t->row_order[i];
	int y = (i - t->top)*t->row_height;
	Boolean on = t->row_state[ro];
	XSetForeground(dp, tc->gc, t->table_background);
	if(tc->verticalLines && !t->color_only) {
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc,
		select_fill_x, y+1, fill_width+1, t->row_height-1);
	}
	else if(t->color_only) {
	    XFillRectangle(dp, tc->image, tc->gc,
		select_fill_x, y+1, fill_width+1, t->row_height);
	}
	else {
	    XFillRectangle(dp, tc->image, tc->gc,
		select_fill_x, y, fill_width+1, t->row_height);
	}
	if(on && !t->select_toggles) {
	    XSetForeground(dp, tc->gc, p->foreground);
	    XFillRectangle(dp, tc->image, tc->gc,
			fill_x, y+2, select_fill_width, t->row_height-3);
	    XSetForeground(dp, tc->gc, t->table_background);
	}
	else {
	    XSetForeground(dp, tc->gc, p->foreground);
	}

	for(j = j1; j <= j2; j++)
	{
	    int co = t->col_order[j];
	    int x = 0;
	    String s = t->columns[co][ro];
	    if(s == (String)NULL) s = (char *)"";
	    if(t->col_alignment[co] == LEFT_JUSTIFY) {
		x = t->col_beg[j] - t->col_beg[t->left] + t->margin + 3;
	    }
	    else if(t->col_alignment[co] == RIGHT_JUSTIFY) {
		x = t->col_end[j] - t->col_beg[t->left] + t->margin - 3
			- stringWidth(w, s);
	    }
	    else {	/* CENTER */
		x = t->col_beg[j] - t->col_beg[t->left] + t->margin + 3
			+ (t->col_end[j]-t->col_beg[j]-stringWidth(w, s))/2;
	    }
	    if(i == t->select_row && j == t->select_col &&
		t->select_char1 != t->select_char2)
	    {
		String sub;
		int f1, width;
		int start = x;
		int p1, p2, ex1, ex2;
		if(t->select_x1 < t->select_x2) {
		    p1 = t->select_char1;
		    p2 = t->select_char2;
		    ex1 = start + t->select_x1;
		    ex2 = start + t->select_x2;
		}
		else {
		    p1 = t->select_char2;
		    p2 = t->select_char1;
		    ex1 = start + t->select_x2;
		    ex2 = start + t->select_x1;
		}
		XSetForeground(dp, tc->gc, t->table_background);
		f1 = t->col_beg[j] - t->col_beg[t->left] + t->margin + 1;
		width = t->col_end[j] - t->col_beg[j];
    		XFillRectangle(dp, tc->image, tc->gc, f1, y+2, width,
				t->row_height-3);
		sub = substring(w, s, 0, p1);
		XSetForeground(dp, tc->gc, p->foreground);
		if(sub != NULL) {
		    XDrawString(dp, tc->image, tc->gc, x,
			y + t->max_ascent+3, sub, (int)strlen(sub));
		}
		Free(sub);
		width = ex2 - ex1 + 1;
    		XFillRectangle(dp, tc->image, tc->gc, ex1, y+2, width,
				t->row_height-3);
		XSetForeground(dp, tc->gc, t->table_background);
		sub = substring(w, s, p1, p2);
		if(sub != NULL) {
		    XDrawString(dp, tc->image, tc->gc, ex1,
			y + t->max_ascent + 3, sub, (int)strlen(sub));
		}
		XSetForeground(dp, tc->gc, p->foreground);
		Free(sub);
		sub = substring(w, s, p2, (int)strlen(s));
		if(sub != NULL) {
		    XDrawString(dp, tc->image, tc->gc, ex2,
			y + t->max_ascent + 3, sub, (int)strlen(sub));
		}
		Free(sub);
	    }
	    else  {
		if(t->cell_fill[co][ro] == CELL_TOGGLE_ON ||
		    t->cell_fill[co][ro] == CELL_TOGGLE_OFF)
		{
		    int hx = t->col_beg[j] - t->col_beg[t->left] + t->margin+3;
		    int hy = y+3;
		    int hw = t->col_end[j] - t->col_beg[j] - 6;
		    int hh = t->row_height-6;
		    XSetForeground(dp, tc->gc, w->core.background_pixel);
		    XDrawLine(dp, tc->image, tc->gc, hx-1, hy-1, hx-1, hy+hh+2);
		    XDrawLine(dp, tc->image, tc->gc, hx-1, hy-1, hx+hw+2, hy-1);
		    XDrawLine(dp, tc->image, tc->gc, hx-2, hy-2, hx-2, hy+hh+2);
		    XDrawLine(dp, tc->image, tc->gc, hx-2, hy-2, hx+hw+3, hy-2);
		    XDrawLine(dp, tc->image, tc->gc,hx,hy+hh+1,hx+hw+3,hy+hh+1);
		    XDrawLine(dp, tc->image, tc->gc,hx,hy+hh+2,hx+hw+3,hy+hh+2);
		    XDrawLine(dp, tc->image, tc->gc,hx+hw+1,hy,hx+hw+1,hy+hh);
		    XDrawLine(dp, tc->image, tc->gc,hx+hw+2,hy,hx+hw+2,hy+hh);
		    if(t->cell_fill[co][ro] == CELL_TOGGLE_OFF) {
			XSetForeground(dp, tc->gc, w->core.background_pixel);
			XFillRectangle(dp, tc->image, tc->gc, hx, hy, hw, hh);
			XSetForeground(dp, tc->gc, p->top_shadow_color);
		    }
		    else {
			XSetForeground(dp, tc->gc, p->bottom_shadow_color);
			XFillRectangle(dp, tc->image, tc->gc, hx, hy, hw, hh);
			XSetForeground(dp, tc->gc, p->highlight_color);
		    }
		    XDrawLine(dp, tc->image,tc->gc,hx,  hy,  hx+hw,hy);
		    XDrawLine(dp, tc->image,tc->gc,hx,  hy+1,hx+hw-1,hy+1);
		    XDrawLine(dp, tc->image,tc->gc,hx,  hy,  hx+0, hy+hh);
		    XDrawLine(dp, tc->image,tc->gc,hx+1,hy,  hx+1, hy+hh-1);
		    if(t->cell_fill[co][ro] == CELL_TOGGLE_OFF) {
			XSetForeground(dp, tc->gc, p->highlight_color);
		    }
		    else {
			XSetForeground(dp,tc->gc, p->top_shadow_color);
		    }
		    XDrawLine(dp, tc->image,tc->gc,hx+1, hy+hh, hx+hw, hy+hh);
		    XDrawLine(dp, tc->image,tc->gc,hx+2,hy+hh-1,hx+hw,hy+hh-1);
		    XDrawLine(dp, tc->image,tc->gc,hx+hw, hy+1, hx+hw, hy+hh);
		    XDrawLine(dp,tc->image,tc->gc,hx+hw-1,hy+2,hx+hw-1,hy+hh-1);
		    XSetForeground(dp, tc->gc, (on && !t->select_toggles) ?
			t->table_background : p->foreground);
		}
		else if(t->cell_fill[co][ro] != CELL_NO_FILL_FLAG) {
		    /* color fill the cell background */
		    int hx = t->col_beg[j] - t->col_beg[t->left] + t->margin+2;
		    int hy = y+2;
		    int hw = t->col_end[j] - t->col_beg[j] - 3;
		    int hh = t->row_height-3;
		    if(!tc->verticalLines) { hy--; hh++; }
		    if(t->color_only) { hy--; hh += 2; }
		    if(!tc->horizontalLines) { hx--; hw += 2; }
		    if(j == 0) {hx -= t->margin; hw += t->margin;}
		    XSetForeground(dp, tc->gc, t->cell_fill[co][ro]);
		    if(t->color_only) hw++;
		    XFillRectangle(dp, tc->image, tc->gc,hx,hy,hw,hh);
		    XSetForeground(dp, tc->gc, (on && !t->select_toggles) ?
			t->table_background : p->foreground);
		    XDrawString(dp, tc->image, tc->gc, x, y + t->max_ascent+3,
				s, (int)strlen(s));
		}
		else {
		    XDrawString(dp, tc->image, tc->gc, x, y + t->max_ascent+3,
				s, (int)strlen(s));
		}

		if(t->highlight[co][ro]) {
		    int hx = t->col_beg[j] - t->col_beg[t->left] + t->margin+1;
		    int hy = y+1;
		    int hw = t->col_end[j] - t->col_beg[j] - 2;
		    int hh = t->row_height-2;
		    if(j == 0) {hx -= t->margin; hw += t->margin;}
		    XSetForeground(dp, tc->gc, t->highlight_color);
		    XDrawRectangle(dp, tc->image, tc->gc,hx,hy,hw,hh);
		    XSetForeground(dp, tc->gc, (on && !t->select_toggles) ?
			t->table_background : p->foreground);
		}
	    }
	    if(tc->verticalLines) {
		if(j > 0 && mod(j-j0, t->n_small_tdel)) {
		    x = t->col_beg[j] - t->col_beg[t->left] + t->margin;
		    if(!t->color_only) {
			XDrawLine(dp, tc->image, tc->gc, x, y+1, x,
				y + t->row_height-1);
		    }
		    else {
			int y2 = (i < t->nrows-1) ? y + t->row_height+1 :
					w->core.height-1;
			XDrawLine(dp, tc->image, tc->gc, x, y, x, y2);
		    }
		}
		if(j == j2 && j2 < t->right && mod(j+1-j0,t->n_small_tdel)) {
		    x = t->col_beg[j+1] - t->col_beg[t->left] + t->margin;
		    if(!t->color_only) {
			XDrawLine(dp, tc->image, tc->gc, x, y+1,
				x, y + t->row_height-1);
		    }
		    else {
			int y2 = (i < t->nrows-1) ? y + t->row_height+1 :
					w->core.height-1;
			XDrawLine(dp, tc->image, tc->gc, x, y, x, y2);
		    }
		}
	    }
	}
	if(tc->horizontalLines) {
	    int h = t->row_height;
	    XSetForeground(dp, tc->gc, p->foreground);
	    XDrawLine(dp, tc->image, tc->gc, fill_x-1, y,
			fill_x+fill_width+1, y);
	    XDrawLine(dp, tc->image, tc->gc, fill_x-1, y+h,
			fill_x+fill_width+1, y+h);
	}
    }
    if(tc->verticalLines) {
	int y = (i1 - t->top)*t->row_height;
	XSetForeground(dp, tc->gc, p->foreground);
	if(j1 == 0) {
	    if(!t->color_only) {
		XDrawLine(dp, tc->image, tc->gc, 0, y, 0,
			y+(i2-i1+1)*t->row_height);
	    }
	    else {
		XDrawLine(dp, tc->image, tc->gc, 0, y, 0,
			y+(i2-i1+1)*t->row_height+2);
	    }
	}
	if(j2 == t->ncols-1 && mod(j2-j0,t->n_small_tdel)) {
	    int x = t->col_end[j2] - t->col_beg[t->left] + t->margin;
	    if(x < w->core.width) {
		if(!t->color_only) {
		    XDrawLine(dp, tc->image, tc->gc, x, y, x,
			y+(i2-i1+1)*t->row_height);
		}
		else {
		    XDrawLine(dp, tc->image, tc->gc, x, y, x,
			y+(i2-i1+1)*t->row_height+2);
		}
	    }
	}
    }
    if(t->color_only) {
	XSetForeground(dp, tc->gc, p->foreground);
	XDrawRectangle(dp, XtWindow(w), tc->gc, 0, 0, w->core.width-1,
			w->core.height-1);
	_TCanvasDrawXLabels(w);
    }
}

static int
getFirstLine(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int j, j0;

    j0 = t->left;
    for(j = t->left; j <= t->right; j++) {
	double time = t->epoch_time + j*t->small_tdel;
	if(fabs(time - ((int)(time/t->large_tdel))*t->large_tdel)
		< t->small_tdel) {
	    j0 = j;
	    break;
	}
    }
    return j0;
}

void
_TCanvasDrawXLabels(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    Widget table = (Widget)tc->table;
    char s[20];
    DateTime dt;
    int j, j0, direction, ascent, descent, x, y, last_end;
    XCharStruct overall;
    XRectangle r;

    if(!t->color_only) return;

    r.x = 0;
    r.y = 0;
    r.width = table->core.width;
    r.height = table->core.height;
    XSetClipRectangles(XtDisplay(table), t->gc, 0, 0, &r, 1, Unsorted);

    y = w->core.y + w->core.height;

    strcpy(s, "00:00:00");
    XTextExtents(t->font, s, (int)strlen(s), &direction, &ascent, &descent,
			&overall);
    XClearArea(XtDisplay(table), XtWindow(table), 0, y,
		XtWidth(table), y + overall.ascent+overall.ascent+4, False);

    j0 = (t->color_only) ? getFirstLine(w) : 0;

    last_end = -1000;

    for(j = j0; j <= t->right; j++) if(mod(j-j0, t->n_small_tdel)) {
	double time = t->epoch_time + j*t->small_tdel;
	timeEpochToDate(time, &dt);
	snprintf(s, sizeof(s), "%02d:%02d:%02d", dt.hour, dt.minute,
		(int)(dt.second+.5));
        XTextExtents(t->font, s, (int)strlen(s), &direction, &ascent,
                &descent, &overall);
	y = w->core.y + w->core.height + ascent + 2;
	x = w->core.x + t->col_beg[j] - t->col_beg[t->left] + t->margin;
	x -= (int)(.5*overall.width);
	if(x > last_end) {
	    XDrawString(XtDisplay(table), XtWindow(table), t->gc,
		x, y, s, (int)strlen(s));
	    last_end = x + overall.width;
	}
    }
}

/** 
 *  
 */
void
_TCanvasRedisplay(TCanvasWidget w)
{
    _TCanvasRedraw(w);
    Redisplay((Widget)w, NULL, NULL);
}

static void
TCanvasScroll(TCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    XButtonEvent *e = (XButtonEvent *)event;
    int cursor_y = e->y;
    int row, top;

    top = MmTableGetBarValue(t->vbar);

    if(e->button == Button4) {
	if(top > 0) {
	    MmTableSetBarValue(t->vbar, top-1);
	    _TCanvasVScroll(w);
	}
    }
    else if(e->button == Button5) {
	row = t->bottom + 1;
	if(row <= t->nrows) {
	    MmTableSetBarValue(t->vbar, top+1);
	    _TCanvasVScroll(w);
	}
    }

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row > t->nrows-1) row = t->nrows-1;

    if(!t->select_toggles && t->selectable && t->rowSelectable) {
	highlightRow(w, row, True);
    }
    tc->inside_row = row;
    if(t->color_only) {
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
			w->core.height-1);
    }

    printInfo(w, False);
}


/** 
 *  
 */
void
_TCanvasVScroll(TCanvasWidget w)
{
    int new_top, new_bottom, new_left, new_right;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(!XtIsRealized((Widget)w)) return;

    new_top = MmTableGetBarValue(t->vbar);
    new_bottom = new_top + MmTableGetBarVisibleAmount(t->vbar);
    if(new_bottom >= t->nrows) new_bottom = t->nrows-1;
    new_left = MmTableGetBarValue(t->hbar);
    if(new_left >= t->ncols) new_left = t->ncols-1;
    if(new_left < 0) t->left = 0;
    new_right = _TCanvasGetRight(w, new_left);
    if(new_top >= t->bottom || new_bottom <= t->top ||
	new_left != t->left || new_right != t->right)
    {
	_TCanvasRedisplay(w);
	_TCanvasDrawRowLabels(w);
	return;
    }
    if(new_top > t->top) {
	int i1, i2;
	int y = (new_top - t->top)*t->row_height;
	int height = w->core.height - y;
	XCopyArea(XtDisplay(w), tc->image, tc->image, tc->gc,
			0, y, w->core.width, height, 0, 0);
	XSetForeground(XtDisplay(w), tc->gc, t->table_background);
	y = height;
	height = w->core.height - height + 1;

	i1 = t->bottom;
	i2 = new_bottom + 1;
	if(i2 > t->nrows-1) i2 = t->nrows-1;
	t->top = new_top;
	t->bottom = new_bottom;

	_TCanvasDrawRows(w, i1, i2, t->left, t->right);
    }
    else {
	int i1, i2;
	int y = (t->top - new_top)*t->row_height;
	int height = w->core.height - y;
	XCopyArea(XtDisplay(w), tc->image, tc->image, tc->gc,
			0, 0, w->core.width, height, 0, y);
	XSetForeground(XtDisplay(w), tc->gc, w->core.background_pixel);

	i1 = new_top;
	i2 = t->top;
	t->top = new_top;
	t->bottom = new_bottom;

	_TCanvasDrawRows(w, i1, i2, t->left, t->right);
    }
    fillSides(w);
    XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	w->core.width, w->core.height, 0, 0);
    _TCanvasDrawRowLabels(w);
    if(t->color_only) {
	XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
		w->core.height-1);
    }

    printInfo(w, True);
}

/** 
 *  
 */
int
_TCanvasGetRight(TCanvasWidget w, int left)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int right;

    if(t->ncols <= 0) return -1;
    for(right = left; right < t->ncols-1; right++) {
	int x = t->col_beg[right+1] - t->col_beg[left] + t->margin;
	if(x >= w->core.width) break;
    }
    return right;
}

/** 
 *  
 */
void
_TCanvasHScroll(TCanvasWidget w)
{
    int new_top, new_bottom, new_left, new_right;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(!XtIsRealized((Widget)w)) return;

    new_top = MmTableGetBarValue(t->vbar);
    new_bottom = new_top + MmTableGetBarVisibleAmount(t->vbar);
    if(new_bottom >= t->nrows) new_bottom = t->nrows-1;
    new_left = MmTableGetBarValue(t->hbar);
    if(new_left >= t->ncols) new_left = t->ncols-1;
    if(new_left < 0) t->left = 0;
    new_right = _TCanvasGetRight(w, new_left);
    if(new_left >= t->right || new_right <= t->left ||
	new_top != t->top || new_bottom != t->bottom)
    {
	_TCanvasRedisplay(w);
	return;
    }
    if(new_left > t->left) {
	int j1, j2;
	int x = t->col_beg[new_left] - t->col_beg[t->left];
	int width = w->core.width - x - 1;
	XCopyArea(XtDisplay(w), tc->image, tc->image, tc->gc,
			x, 0, width, w->core.height, 0, 0);
	XSetForeground(XtDisplay(w), tc->gc, w->core.background_pixel);
	x = width;
	width = w->core.width - width + 1;

	j1 = t->right;
	j2 = new_right;
	t->left = new_left;
	t->right = new_right;

	_TCanvasDrawRows(w, t->top, t->bottom, j1, j2);
    }
    else {
	int j1, j2;
	int x = t->col_beg[t->left] - t->col_beg[new_left];
	int width = w->core.width - x;
	XCopyArea(XtDisplay(w), tc->image, tc->image, tc->gc,
			0, 0, width, w->core.height, x, 0);
	XSetForeground(XtDisplay(w), tc->gc, w->core.background_pixel);

	j1 = new_left;
	j2 = t->left;
	t->left = new_left;
	t->right = new_right;

	_TCanvasDrawRows(w, t->top, t->bottom, j1, j2);
    }
    fillSides(w);
    XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	w->core.width, w->core.height, 0, 0);

    MmTableAdjustHScroll(tc->table);
    if(t->color_only) {
	XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
		w->core.height-1);
    }
}

Size
TCanvasGetPreferredSize(TCanvasWidget w)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int nrows;
    Size s;

    if(t->row_height == 0) {
	int direction;
	XCharStruct overall;
	XTextExtents(t->font, "M", 1, &direction, &t->max_ascent,
		&t->max_descent, &overall);
	t->row_height = t->max_ascent + t->max_descent + 4;
    }
    s.width = (t->ncols > 0) ? t->margin + t->col_end[t->ncols-1] + 1 : 50;
    nrows = (t->visible_rows > t->nrows) ? t->visible_rows : t->nrows;
    s.height = nrows*t->row_height + 1 + t->bottom_margin;
    return s;
}

/** 
 *  
 */
void
_TCanvasUpdateRow(TCanvasWidget w, int i)
{
    int y, width;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(i < 0 || i >= t->nrows) return;
    y = (i - t->top)*t->row_height;
    XSetForeground(XtDisplay(w), tc->gc, t->table_background);
    width = t->col_end[t->right] - t->col_beg[t->left] + t->margin - 3;
    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
		t->row_height-3);
    _TCanvasDrawRows(w, i, i, t->left, t->right);
    fillSides(w);
    XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	w->core.width, w->core.height, 0, 0);
    if(t->color_only) {
	XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
		w->core.height-1);
    }
}

static void
mouseMoved(TCanvasWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    int top, row, column, j, x;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int cursor_x = ((XButtonEvent *)event)->x;
    int cursor_y = ((XButtonEvent *)event)->y;

    if(t->nrows <= 0) return;

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;

    if(!t->select_toggles && t->selectable && t->rowSelectable) {
	if(row != tc->inside_row) {
	    highlightRow(w, tc->inside_row, False);
	}
	if(row < t->nrows) highlightRow(w, row, True);
    }
    tc->inside_row = (row < t->nrows) ? row : -1;

    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x >= x && cursor_x < x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
                                        t->col_width[t->col_order[j]];
    }
    column = (j <= t->right) ? j : -1;

    printInfo(w, False);

    TCanvasMotion(w, event, params, num_params);

    if(row >= 0 && row < t->nrows && column >= 0 && column < t->ncols &&
		(row != t->cursor_row || column != t->cursor_col))
    {
	t->cursor_row = row;
	t->cursor_col = column;
	TCanvasCellEnter(w, event, row, column);
    }
}

static void
printInfo(TCanvasWidget w, Boolean scroll)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(t->cursor_info != NULL)
    {
	char label[100];
	if(!scroll) {
	    snprintf(label, 100, "top row: %d  cursor: %d",
			t->top+1, tc->inside_row+1);
	}
	else {
	    snprintf(label, 100, "top row: %d  cursor:", t->top+1);
	}
	InfoSetText(t->cursor_info, label);
    }
}

static void
mouseDraggedButton3(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    int old_edit_pos, y, x1, x2, y1, y2, width, height;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(t->select_row < 0 || t->select_col < 0) return;

    old_edit_pos = t->edit_pos;
    positionEditCursor(w, event, t->select_col, t->select_row);
    if(t->edit_pos == old_edit_pos) return;
    t->select_x2 = t->edit_x;
    t->select_char2 = t->edit_pos;

    _TCanvasDrawRows(w, t->select_row, t->select_row, t->select_col,
			t->select_col);
    y = (t->select_row - t->top)*t->row_height;
    x1 = t->col_beg[t->select_col] - t->col_beg[t->left] + t->margin;
    x2 = x1 + t->col_width[t->col_order[t->select_col]] - 1;
    y1 = y+2;
    y2 = y + t->row_height-1;
    width = x2 - x1 + 1;
    height = y2 - y1 + 1;
    XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc,
    		x1, y1, width, height, x1, y1);
    if(t->color_only) {
	XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
		w->core.height-1);
    }
}

static void
mouseDraggedButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    int i, top, row, width, y;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int cursor_y = ((XButtonEvent *)event)->y;
	
    if(t->editable &&
	(t->select_toggles || !t->selectable || !t->rowSelectable))
    {
	mouseDraggedButton3(w, event, params, num_params);
    }

    if(t->select_toggles || !t->selectable || !t->rowSelectable
	|| tc->mouse_down_row < 0) return;

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row < 0) row = 0;
    if(row > t->nrows-1) row = t->nrows-1;
    if(row == tc->mouse_drag_row) return;

    width = t->col_end[t->right] - t->col_beg[t->left] + t->margin - 3;

    for(i = row+1; i <= tc->mouse_drag_row; i++) {
	if(i > tc->mouse_down_row) {
	    t->row_state[t->row_order[i]] = False;
	    y = (i-top)*t->row_height;
	    XSetForeground(XtDisplay(w), tc->gc, t->table_background);
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
			t->row_height-3);
	    _TCanvasDrawRows(w, i, i, t->left, t->right);
	}
    }
    for(i = tc->mouse_drag_row; i < row; i++) {
	if(i < tc->mouse_down_row) {
	    t->row_state[t->row_order[i]] = False;
	    y = (i-top)*t->row_height;
	    XSetForeground(XtDisplay(w), tc->gc, t->table_background);
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
			t->row_height-3);
	    _TCanvasDrawRows(w, i, i, t->left, t->right);
	}
    }
    for(i = tc->mouse_drag_row+1; i <= row; i++) {
	if(i > tc->mouse_down_row) {
	    t->row_state[t->row_order[i]] = True;
	    y = (i-top)*t->row_height;
	    XSetForeground(XtDisplay(w), tc->gc, t->table_background);
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
			t->row_height-3);
	    _TCanvasDrawRows(w, i, i, t->left, t->right);
	}
    }
    for(i = row; i < tc->mouse_drag_row; i++) {
	if(i < tc->mouse_down_row) {
	    t->row_state[t->row_order[i]] = True;
	    y = (i-top)*t->row_height;
	    XSetForeground(XtDisplay(w), tc->gc, t->table_background);
	    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
			t->row_height-3);
	    _TCanvasDrawRows(w, i, i, t->left, t->right);
	}
    }

    XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	w->core.width, w->core.height, 0, 0);

    tc->mouse_drag_row = row;
    tc->inside_row = row;
    highlightRow(w, row, True);

    if(tc->mouse_drag_row >= t->bottom) {
	row = tc->mouse_drag_row - (t->bottom - top) + 1;
	if(row > t->nrows-1) row = t->nrows-1;
	MmTableSetBarValue(t->vbar, row);
	_TCanvasVScroll(w);
    }
    else if(tc->mouse_drag_row < top && tc->mouse_drag_row >= 0) {
	MmTableSetBarValue(t->vbar, tc->mouse_drag_row);
	_TCanvasVScroll(w);
    }
    if(t->color_only) {
	XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0, w->core.width-1,
		w->core.height-1);
    }
    printInfo(w, False);
}

static void
mousePressedButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    int i, j, top, row, column, x;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int cursor_x = ((XButtonEvent *)event)->x;
    int cursor_y = ((XButtonEvent *)event)->y;
    static Time last_time = 0;

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row > t->nrows-1) return;

    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x >= x && cursor_x < x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
                                        t->col_width[t->col_order[j]];
    }
    column = (j <= t->right) ? j : -1;

    TCanvasCellSelect(w, event, row, column);

    if(t->nrows <= 0 || (!t->editable &&
		(!t->selectable || !t->rowSelectable)) ) return;

    _MmTableResetInfo(tc->table);

    if(t->editable &&
	(t->select_toggles || !t->selectable || !t->rowSelectable))
    {
	mousePressedButton3(w, event, params, num_params);
	return;
    }

    tc->selecting = True;

    if(t->select_row >= 0 && t->select_char2 > t->select_char1) {
	t->select_row = t->select_col = t->select_char1 = t->select_char2 = -1;
	_TCanvasRedisplay(w);
    }
    t->select_row = t->select_col = t->select_char1 = t->select_char2 = -1;

    MmTableEditModeOff(tc->table);

    tc->select_data.states = (Boolean *)ReallocIt(w, tc->select_data.states,
		(t->nrows+t->nhidden)*sizeof(Boolean));
    tc->select_data.changed_rows = (int *)ReallocIt(w,
		tc->select_data.changed_rows,(t->nrows+t->nhidden)*sizeof(int));

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	tc->select_data.states[i] = t->row_state[i];
    }

    if(((XButtonEvent *)event)->time - last_time < 300) {
	mouseActionButton1(w, event, params, num_params);
    }
    else last_time = ((XButtonEvent *)event)->time;

    tc->mouse_down_row = row;
    tc->mouse_drag_row = row;
			
    if(*num_params > 0 && !strcmp(params[0], "Ctrl"))
    {
	_TCanvasSelectRow(w,t->row_order[row],!t->row_state[t->row_order[row]]);
    }
    else if(*num_params > 0 && !strcmp(params[0], "Shift")) {
	int i1, i2;
	for(i1 = row-1; i1 >= 0; i1--) {
	    if(t->row_state[t->row_order[i1]]) break;
	}
	for(i2 = row+1; i2 < t->nrows; i2++) {
	    if(t->row_state[t->row_order[i2]]) break;
	}
	if(i1 < 0 && i2 == t->nrows) {
	    _TCanvasSelectRow(w, t->row_order[row],
		!t->row_state[t->row_order[row]]);
	}
	else {
	    if(i1 < 0) {
		i1 = row;
		i2--;
	    }
	    else if(i2 == t->nrows) {
		i1++;
		i2 = row;
	    }
	    else if(row-i1 < i2-row) {
		i1++;
		i2 = row;
	    }
	    else {
		i1 = row;
		i2--;
	    }
	    for(i = i1; i <= i2; i++) {
		t->row_state[t->row_order[i]] = True;
	    }
	    if(i2 >= i1) _TCanvasRedisplay(w);
	}
    }
    else {
	int n = 0;
	for(i = 0; i < t->nrows; i++) {
	    if(i != row && t->row_state[t->row_order[i]]) {
		t->row_state[t->row_order[i]] = False;
		n++;
	    }
	}
	if(n == 0) {
	    _TCanvasSelectRow(w, t->row_order[row],
		!t->row_state[t->row_order[row]]);
	}
	else {
	    t->row_state[t->row_order[row]] = !t->row_state[t->row_order[row]];
	    _TCanvasRedisplay(w);
	}
    }
    highlightRow(w, row, True);
}

static void
TCanvasCellSelect(TCanvasWidget w, XEvent *event, int row, int col)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    MmTableCellCallbackStruct c;
    int ro, co;

    if(row >= 0 && row < t->nrows && col >= 0 && col < t->ncols)
    {
	c.row = row;
	c.column = col;
	ro = t->row_order[row];
	co = t->col_order[col];
	c.string = t->columns[co][ro];
	if(t->cell_fill[co][ro] == CELL_TOGGLE_ON) {
	    t->cell_fill[co][ro] = CELL_TOGGLE_OFF;
	    _TCanvasDrawRows(w, row, row, col, col);
	    _TCanvasRedisplay(w);
	}
	else if(t->cell_fill[co][ro] == CELL_TOGGLE_OFF) {
	    t->cell_fill[co][ro] = CELL_TOGGLE_ON;
	    _TCanvasDrawRows(w, row, row, col, col);
	    _TCanvasRedisplay(w);
	}
	c.pixel = t->cell_fill[co][ro];
	c.event = event;
	XtCallCallbacks((Widget)tc->table, XtNcellSelectCallback, &c);
    }
}

static void
TCanvasCellEnter(TCanvasWidget w, XEvent *event, int row, int col)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    MmTableCellCallbackStruct c;

    if(row >= 0 && row < t->nrows && col >= 0 && col < t->ncols)
    {
	c.row = row;
	c.column = col;
	c.string = t->columns[t->col_order[col]][t->row_order[row]];
	c.pixel = t->cell_fill[t->col_order[col]][t->row_order[row]];
	c.event = event;
	XtCallCallbacks((Widget)tc->table, XtNcellEnterCallback, &c);
    }
}


static void *
ReallocIt(TCanvasWidget w, void *ptr, int nbytes)
{   
    TCanvasPart *tc = &w->tCanvas;
    char msg[128];
    void *pt;

    if((pt = (void *)realloc(ptr, nbytes)) == (void *)0) {
	snprintf(msg, 128,
            "Memory limitation:\nCannot allocate an additional %d bytes.",
            nbytes);
	if(XtHasCallbacks((Widget)tc->table, XtNwarningCallback) !=
		XtCallbackHasNone)
	{
	    XtCallCallbacks((Widget)tc->table, XtNwarningCallback, msg);
	}
	else {
	    fprintf(stderr, "%s\n", msg);
	}
	return (void *)NULL;
    }
    return pt;
}

static void *
MallocIt(TCanvasWidget w, int nbytes)
{   
    TCanvasPart *tc = &w->tCanvas;
    char msg[128];
    void *ptr;

    if(nbytes <= 0) return (void *)NULL;

    if((ptr = (void *)malloc(nbytes)) == (void *)0) {
	snprintf(msg, 128,
            "Memory limitation:\nCannot allocate an additional %d bytes.",
            nbytes);
	if(XtHasCallbacks((Widget)tc->table, XtNwarningCallback) !=
		XtCallbackHasNone)
	{
	    XtCallCallbacks((Widget)tc->table, XtNwarningCallback, msg);
	}
	else {
	    fprintf(stderr, "%s\n", msg);
	}
	return (void *)NULL;
    }
    return ptr;
}

static void
mouseActionButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    int row, top;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int cursor_y = ((XButtonEvent *)event)->y;

    if(t->select_toggles || !t->selectable || !t->rowSelectable) return;

    tc->mouse_down_row = -1;
    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;

/*
    if(tableListeners.size() > 0) {
	String[] values = new String[t->ncols];
	for(i = 0; i < t->ncols; i++) {
	    values[i] = t->columns[i][t->row_order[row]];
	}
	for(int i = 0; i < tableListeners.size(); i++) {
	    TableListener t = (TableListener)tableListeners.elementAt(i);
	    t.rowDoubleClicked(w->core.parent, row,
		t->row_order[row], t->column_labels, values);
	}
    }
*/
    highlightRow(w, row, True);
}

static void
mousePressedButton2(		/* paste */
    TCanvasWidget	w,
    XEvent		*event,
    String		*params,
    Cardinal		*num_params)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(!t->editable) return;

    MmTablePaste(tc->table, (XButtonEvent *)event);
    _MmTableResetInfo(tc->table);
}

static void
mousePressedButton3(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    Boolean action = False;
    int top, row;
    int cursor_y = ((XButtonEvent *)event)->y;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    static Time last_time = 0;

    if(t->nrows > 0 && !t->editable) {
	_HCanvas_mousePressedMenu(t->header, event, params, num_params);
	return;
    }

    if(t->nrows <= 0 || (!t->selectable && !t->editable)) return;

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row > t->nrows-1) return;

    if(((XButtonEvent *)event)->time - last_time < 300) {
	action = True;
    }
    else last_time = ((XButtonEvent *)event)->time;

    if(!t->editable && t->rowSelectable) {
	mousePressedButton1(w, event, params, num_params);
    }
    else if(mousePressed3(w, event, action)) {
	_TCanvasRedisplay(w);
    }
    _MmTableResetInfo(tc->table);
}

static Boolean
mousePressed3(TCanvasWidget w, XEvent *event, Boolean action)
{
    char *choice=NULL;
    int j, x, top, row;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    XButtonEvent *e = (XButtonEvent *)event;
    int cursor_x = ((XButtonEvent *)event)->x;
    int cursor_y = ((XButtonEvent *)event)->y;

    if(t->nrows <= 0 || t->ncols <= 0) return False;
    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x > x && cursor_x <= x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
		t->col_width[t->col_order[j]];
    }
    if(j > t->right) return False;
    if(!t->col_editable[t->col_order[j]]) return False;

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row >= 0 && row < t->nrows
	&& !t->row_editable[t->row_order[row]]) return False;

    if(!t->table_class->cellEditable(t->row_order[row], t->col_order[j])) {
	return False;
    }

    if((choice = t->table_class->getCellChoice(row, j)) != NULL ||
	t->column_choice[t->col_order[j]][0] != '\0')
    {
	if(e->button == Button1) {
	    MmTableEditCallbackStruct callback;
	    callback.old_string =
		strdup(t->columns[t->col_order[j]][t->row_order[row]]);
	
	    if(choice) {
		flipChoice(t, choice, row, j);
	    }
	    else {
		flipChoice(t, t->column_choice[t->col_order[j]], row, j);
	    }
	    _TCanvasDrawRows(w, row, row, t->left, t->right);
	    _TCanvasRedisplay(w);
	    XFlush(XtDisplay(w));

	    callback.column = j;
	    callback.row = row;
	    callback.new_string =t->columns[t->col_order[j]][t->row_order[row]];
	    XtCallCallbacks((Widget)tc->table, XtNchoiceChangedCallback,
				&callback);
	    free(callback.old_string);
	    return True;
	}
	else {
	    /* Display a popup menu of choices. */
	   if(choice) {
		TCanvasDisplayChoiceMenu(w, event, choice, row, j);
	   }
	   else {
		TCanvasDisplayChoiceMenu(w, event,
			t->column_choice[t->col_order[j]], row, j);
	   }
	}
    }

/*
    for(i = 0; i < t->nrows; i++) t->row_state[i] = False;

    if(t->edit_row >= 0 && t->edit_x >= 0) {
	drawEditCursor(w, t->edit_x, 0, t->edit_col, t->edit_row);
    }
*/
    t->edit_x = t->edit_pos = t->select_char1 = t->select_char2 = -1;
    t->select_row = t->select_col = -1;
    t->start_edit_cursor = False;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    if(row < 0 || row >= t->nrows) {
	t->edit_row = -1;
	t->edit_col = -1;
	_TCanvasRedisplay(w);
	return False;
    }

    if(j <= t->right) {
	t->select_row = row;
	t->select_col = j;
	if(action)	/* double click */
	{
	    t->select_char1 = 0;
	    t->select_char2 = (int)strlen(
			t->columns[t->col_order[j]][t->row_order[row]]);
	    t->select_x1 = 0;
	    t->select_x2 = stringWidth(w, 
			t->columns[t->col_order[j]][t->row_order[row]]);
	    t->edit_x = t->select_x2;
	    t->edit_pos = t->select_char2;
	    _TCanvasRedisplay(w);
	    if(t->edit_col == j && t->edit_row == row) {
		drawEditCursor(w, t->edit_x, 1, j, row);
		return True;
	    }
	}
	else {
	    positionEditCursor(w, event, j, row);
	}
	if(t->edit_col == j && t->edit_row == row) {
	    t->select_x1 = t->edit_x;
	    t->select_char1 = t->select_char2 = t->edit_pos;
	    drawEditCursor(w, t->edit_x, 1, j, row);
	}
	else {
	    if(!t->select_toggles) {
		if(row != t->edit_row) {
		    Boolean r_state = t->row_state[t->row_order[row]];
		    t->row_state[t->row_order[row]] = False;
	    	    MmTableEditModeOff(tc->table);
		    t->edit_row_state = r_state;
		}
		else {
		    MmTableEditModeOff(tc->table);
		    t->row_state[t->row_order[row]] = False;
		    _TCanvasDrawRows(w, row, row, t->left, t->right);
		    XCopyArea(XtDisplay(w), tc->image, XtWindow(w),
			tc->gc, 0, 0, w->core.width, w->core.height, 0, 0);
		    if(t->color_only) {
			XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0,
			    w->core.width-1,w->core.height-1);
		    }
		}
	    }
	    t->edit_col = j;
	    t->edit_row = row;
	    Free(t->edit_string);
	    t->edit_string = strdup(
	      t->columns[t->col_order[t->edit_col]][t->row_order[t->edit_row]]);
	    if(!action) {  /* a single click */
		t->select_x1 = t->edit_x;
		t->select_char1 = t->select_char2 = t->edit_pos;
	    }
	    t->start_edit_cursor = True;
	    if(t->edit_row >= 0 && t->edit_x >= 0) {
		drawEditCursor(w, t->edit_x, 0, t->edit_col, t->edit_row);
	    }
	}
    }
    return True;
}

static void
TCanvasDisplayChoiceMenu(TCanvasWidget w, XEvent *event, char *choice,
		int row, int column)
{
    TCanvasPart *tc = &w->tCanvas;
    Widget toplevel, button;
    char *buf, *s, *tok;
    Arg args[2];
    int n;
    XtTranslations translations;
    char trans[] =
"<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

    if(!choice) return;
    buf = strdup(choice);
    n = 0;
    tok = buf;
    while(strtok(tok, ":") != NULL) {
	tok = NULL;
	n++;
    }
    free(buf);
    if(!n) return;

    if(choice_popup)
    {
	XtDestroyWidget(choice_popup);
    }
	
    /* the default is button3, which will inactivate button3
     * in the HCanvasWidget, so set to button5
     */
    n = 0;
#if(XmVersion >= 2)
#ifndef XmNtearOffModel
#define XmNtearOffModel "tearOffModel"
#endif
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED);n++;
#endif
    XtSetArg(args[n], XmNwhichButton, 6); n++;

    for(toplevel = XtParent(w); XtParent(toplevel) != NULL;
			toplevel = XtParent(toplevel));
    choice_popup = XmCreatePopupMenu(toplevel, (char *)"Choice", args, n);
    translations = XtParseTranslationTable(trans);

    buf = strdup(choice);
    tok = buf;
    while((s = strtok(tok, ":")) != NULL)
    {
	tok = NULL;
	button = XtVaCreateManagedWidget(s,
			xmPushButtonWidgetClass, choice_popup,
                        XmNtranslations, translations,
                        NULL);
	XtAddCallback(button, XmNactivateCallback,
			ChoiceSelectCB, (XtPointer)w);
    }
    free(buf);

    tc->choice_menu_row = row;
    tc->choice_menu_column = column;
    XmMenuPosition(choice_popup, (XButtonPressedEvent *)event);
    XtManageChild(choice_popup);
}

static void
ChoiceSelectCB(Widget widget, XtPointer client_data, XtPointer data)
{
    TCanvasWidget w = (TCanvasWidget)client_data;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    MmTableEditCallbackStruct callback;
    int i, j;
    char *c;

    i = tc->choice_menu_row;
    j = tc->choice_menu_column;
    if(i < 0 || i >= t->nrows || j < 0 || j >= t->ncols) return;

    c = XtName(widget);
    Free(t->columns[t->col_order[j]][t->row_order[i]]);
    t->columns[t->col_order[j]][t->row_order[i]] = strdup(c);
    _TCanvasDrawRows(w, tc->choice_menu_row, tc->choice_menu_column,
			t->left, t->right);
    _TCanvasRedisplay(w);
    XFlush(XtDisplay(w));

    callback.old_string =
		strdup(t->columns[t->col_order[j]][t->row_order[i]]);
	
    callback.column = j;
    callback.row = i;
    callback.new_string = t->columns[t->col_order[j]][t->row_order[i]];
    XtCallCallbacks((Widget)tc->table, XtNchoiceChangedCallback, &callback);
    free(callback.old_string);
}

/* toggle between two values for a field. Example: column_choice="true:false"
 */

static void
flipChoice(MmTablePart *t, char *choice, int i, int j)
{
    char *c, *s[1000], *tok, *buf;
    int k, n;

    c = t->columns[t->col_order[j]][t->row_order[i]];

    buf = strdup(choice);

    n = 0;
    if(buf[0] == ':') {
	s[0] = (char *)""; // blank
	n = 1;
    }
    tok = buf;
    while(n < 1000 && (s[n] = strtok(tok, ":")) != NULL) {
	tok = NULL;
	n++;
    }
    if(!n) {
	Free(buf);
	return;
    }

    for(k = 0; k < n && strcasecmp(c, s[k]); k++);
    k++;
    if(k >= n) k = 0;
    
    Free(t->columns[t->col_order[j]][t->row_order[i]]);
    t->columns[t->col_order[j]][t->row_order[i]] = strdup(s[k]);
    Free(buf);
}

static void
mouseReleasedButton1(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    int i;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    MmTableSelectCallbackStruct *c;

    if((t->select_toggles || !t->selectable || !t->rowSelectable) &&
		t->editable && t->start_edit_cursor)
    {
	mouseReleasedButton3(w, event, params, num_params);
	return;
    }
    if(!tc->selecting) return;
    tc->selecting = False;

    tc->mouse_down_row = -1;

    c = &tc->select_data;
    c->nrows = t->nrows;
    c->nchanged_rows = 0;

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	if(c->states[i] != t->row_state[i]) {
	    c->changed_rows[c->nchanged_rows++] = i;
	}
	c->states[i] = t->row_state[i];
    }
    if(c->nchanged_rows > 0) {
	XtCallCallbacks((Widget)tc->table, XtNselectRowCallback, c);
    }
}

static void
mouseReleasedButton3(TCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    long *client_data;

    if(!t->selectable && !t->editable) return;

    if(t->start_edit_cursor) {
	t->start_edit_cursor = False;
	client_data = (long *)malloc(4*sizeof(long));
	client_data[0] = t->edit_col;
	client_data[1] = t->edit_row;
	client_data[2] = 1;
	client_data[3] = (long)w;
	editCursor((XtPointer)client_data, 0);
    }
    if(t->edit_row >= 0) {
	Boolean on = True;
	XtCallCallbacks((Widget)tc->table, XtNeditModeCallback, &on);
    }
    MmTableCopy(tc->table, (XButtonEvent *)event);
}

static void
LeaveWindow(TCanvasWidget w, XEvent *event, String *params,Cardinal *num_params)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

//    MmTableEditModeOff(tc->table);
    if(t->selectable && t->rowSelectable) {
	highlightRow(w, tc->inside_row, False);
	printInfo(w, True);
    }
    XtCallCallbacks((Widget)tc->table, XtNleaveWindowCallback, NULL);
}

static void
EnterWindow(TCanvasWidget w, XEvent *event, String *params,Cardinal *num_params)
{
    int top, row;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int cursor_y = ((XButtonEvent *)event)->y;

    if(t->edit_row >= 0) {
	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    }
    if(t->select_toggles || !t->selectable || !t->rowSelectable) return;
    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row != tc->inside_row) {
	highlightRow(w, tc->inside_row, False);
    }
    if(row < t->nrows) {
	highlightRow(w, row, True);
	tc->inside_row = row;
	printInfo(w, False);
    }
    else {
	tc->inside_row = -1;
    }
}

static void
GainedFocus(TCanvasWidget w, XEvent *event, String *params,Cardinal *num_params)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    t->have_focus = True;
}

static void
LostFocus(TCanvasWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    t->have_focus = False;
}

static void
Page(TCanvasWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    if( *num_params < 1 ) return;

    if(!strcmp(params[0], "down")) {
	page(w, XK_Page_Down);
    }
    else {
	page(w, XK_Page_Up);
    }
}

static void
Scroll(TCanvasWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    if( *num_params < 1 ) return;

    if(!strcmp(params[0], "down")) {
	page(w, XK_Down);
    }
    else {
	page(w, XK_Up);
    }
}

static void
KeyPressed(TCanvasWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    int i, j, len, fill_x, fill_width, y, max_w, width;
    Boolean ascii;
    char buffer[10], *buf, c;
    String s;
    KeySym keysym;
    XComposeStatus compose;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    MmTableEditCallbackStruct callback;

    if(t->edit_col < 0 || t->edit_row < 0) {
	len = XLookupString((XKeyEvent *)event, buffer, 10, &keysym, &compose);
	if(len <= 0) {
	    page(w, keysym);
	}
	return;
    }

    s = t->columns[t->col_order[t->edit_col]][t->row_order[t->edit_row]];

    if(XLookupString((XKeyEvent *)event, buffer, 10, &keysym, &compose) <= 0) {
	if(keysym == XK_Left) {
	    if(t->edit_pos > 0) {
		t->edit_pos--;
		drawEditCursor(w, t->edit_x, 0, t->edit_col, t->edit_row);
		t->edit_x = stringWidthLen(w, s, t->edit_pos);
		drawEditCursor(w, t->edit_x, 1, t->edit_col, t->edit_row);
	    }
	}
	else if(keysym == XK_Right) {
	    len = strlen(s);
	    if(t->edit_pos < len) {
		t->edit_pos++;
		drawEditCursor(w, t->edit_x, 0, t->edit_col, t->edit_row);
		t->edit_x = stringWidthLen(w, s, t->edit_pos);
		drawEditCursor(w, t->edit_x, 1, t->edit_col, t->edit_row);
	    }
	}
	else if(keysym == XK_Up)
	{
	    nextCell(w, CELL_UP);
	}
	else if(keysym == XK_Down)
	{
	    nextCell(w, CELL_DOWN);
	}
	else if(keysym == XK_ISO_Left_Tab) // shift tab
	{
	    nextCell(w, CELL_LEFT);
	}
	return;
    }
    c = buffer[0];

    if(c == '\t') {
	nextCell(w, CELL_RIGHT);
	return;
    }
    else if(c == '\r') {
	nextCell(w, CELL_DOWN);
	return;
    }

    ascii = ((keysym >= XK_KP_Space) && (keysym <= XK_KP_9)) ||
	((keysym >= XK_space) && (keysym <= XK_asciitilde)) ? True : False;

    if(!ascii && keysym != XK_BackSpace && keysym != XK_Delete
	&& keysym != XK_Linefeed && keysym != XK_Return) return;

    callback.old_string = strdup(s);
    len = (int)strlen(s);
    buf = (String)malloc(len+1);
    stringcpy(buf, s, len+1);

    if(keysym == XK_Delete) {
	if(t->edit_pos >= len) return;
	t->edit_pos++;
	c = '\b';
    }
    if(c == '\b') { /* backspace */
	if(t->edit_pos == 0) return;
	if(t->select_row == t->edit_row && t->select_char1 != t->select_char2)
	{
	    MmTableCut(tc->table, (XButtonEvent *)event);
	    return;
	}
	for(i = t->edit_pos-1; i < len; i++) buf[i] = buf[i+1];
	if(len > 1) buf[len-1] = '\0';
	t->edit_pos--;
    }
    else if(c != ' ' && (isspace((int)c) || iscntrl((int)c)))
    {
	i = t->edit_row;
	j = t->edit_col;
	MmTableEditModeOff(tc->table);
	t->edit_row = t->edit_col = t->select_char1 = t->select_char2 = -1;
	drawEditCursor(w, t->edit_x, 0, j, i);
	return;
    }
    else {
	if(t->edit_pos > len) t->edit_pos = len; /* should never be */
	buf = (String)realloc(buf, len+2);
	for(i = len; i > t->edit_pos; i--) buf[i] = buf[i-1];
	buf[t->edit_pos] = c;
	buf[len+1] = '\0';
	t->edit_pos++;
    }
    if(t->backup[t->col_order[t->edit_col]][t->row_order[t->edit_row]] == 
		(String)NULL)
    {
	t->backup[t->col_order[t->edit_col]][t->row_order[t->edit_row]] = s;
    }
    else {
	Free(s);
    }
    t->edit_x = stringWidthLen(w, buf, t->edit_pos);
    t->columns[t->col_order[t->edit_col]][t->row_order[t->edit_row]] = buf;
    fill_x = t->col_beg[t->edit_col] - t->col_beg[t->left] + t->margin;
    fill_width = t->col_width[t->col_order[t->edit_col]] - 1;
    y = (t->edit_row - t->top)*t->row_height;
    XSetForeground(XtDisplay(w), tc->gc, t->table_background);
    XFillRectangle(XtDisplay(w), tc->image, tc->gc, fill_x, y+1, fill_width,
			t->row_height-5);
    t->select_row = t->select_char1 = t->select_char2 = -1;
    _TCanvasDrawRows(w, t->edit_row, t->edit_row, t->edit_col, t->edit_col);

    t->needAdjustColumns = 0;

    j = t->edit_col;
    max_w = stringWidth(w, t->column_labels[t->col_order[j]]) + t->cellMargin;

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	width = stringWidth(w, t->columns[t->col_order[j]][i]) + t->cellMargin;
	if(max_w < width) max_w = width;
    }
    if(max_w < t->min_col_width) max_w = t->min_col_width;
    t->col_width[t->col_order[j]] = max_w;

    callback.column = t->edit_col;
    callback.row = t->edit_row;
    callback.new_string = buf;

    XtCallCallbacks((Widget)tc->table, XtNvalueChangedCallback, &callback);
    free(callback.old_string);

    MmTableResetColumnLimits(tc->table);
    if(t->header) _HCanvasRedisplay(t->header);
    _TCanvasRedisplay(t->canvas);
}

static void
page(TCanvasWidget w, KeySym keysym)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int top = MmTableGetBarValue(t->vbar);
    int row, visible_rows;

    if(keysym == XK_Up) {
	if(top > 0) {
	    MmTableSetBarValue(t->vbar, top-1);
	    _TCanvasVScroll(w);
	    row = tc->inside_row-1;
	}
	else return;
    }
    else if(keysym == XK_Down) {
	row = t->bottom + 1;
	if(row <= t->nrows) {
	    MmTableSetBarValue(t->vbar, top+1);
	    _TCanvasVScroll(w);
	    if(MmTableGetBarValue(t->vbar) != top) {
		row = tc->inside_row+1;
	    }
	    else row = tc->inside_row;
	}
	else return;
    }
    else if(keysym == XK_Page_Up) {
	if(top > 0) {
	    visible_rows = MmTableGetBarVisibleAmount(t->vbar);
	    if(visible_rows <= top) {
		top -= visible_rows;
		row = tc->inside_row - visible_rows;
	    }
	    else {
		row = tc->inside_row - top;
		top = 0;
	    }
	    MmTableSetBarValue(t->vbar, top);
	    _TCanvasVScroll(w);
	}
	else return;
    }
    else if(keysym == XK_Page_Down) {
	visible_rows = MmTableGetBarVisibleAmount(t->vbar);
	row = t->bottom + visible_rows;
	if(row < t->nrows) {
	    MmTableSetBarValue(t->vbar, top+visible_rows);
	    _TCanvasVScroll(w);
	    row = tc->inside_row+visible_rows;
	}
	else if(t->bottom < t->nrows-1) {
	    int n = t->nrows - visible_rows;
	    row = tc->inside_row + (n - top);
	    top = n;
	    MmTableSetBarValue(t->vbar, top);
	    _TCanvasVScroll(w);
	}
	else return;
    }
    else {
	return;
    }

    if(row >= 0 && row < t->nrows) {
	if(!t->select_toggles && t->selectable && t->rowSelectable) {
	    highlightRow(w, row, True);
	}
	tc->inside_row = row;
	if(t->color_only) {
	    XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0,
			w->core.width-1, w->core.height-1);
	}
    }
    printInfo(w, False);
}

static void
nextCell(TCanvasWidget w, enum CellDir dir)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int k, pos, col = -1, row = -1, oi, oj, n, sign=1, top;
    long *client_data;
    char *s;
    Boolean r_state;

    n = t->nrows * t->ncols;

    if(dir == CELL_RIGHT || dir == CELL_DOWN) {
	sign = 1;
    }
    else if(dir == CELL_LEFT || dir == CELL_UP) {
	sign = -1;
    }
    if(dir == CELL_LEFT || dir == CELL_RIGHT)
    {
	pos = t->edit_row*t->ncols + t->edit_col;
	pos += sign;
	for(k = 0; k < n; k++, pos += sign) {
	    if(pos < 0) pos = n-1;
	    if(pos >= n) pos = 0;
	    row = pos/t->ncols;
	    col = pos - row*t->ncols;
	    oi = t->row_order[row];
	    oj = t->col_order[col];
	    if(	t->row_editable[oi] && t->col_editable[oj] &&
		t->table_class->cellEditable(oi, oj) &&
		!t->table_class->getCellChoice(oi, oj) &&
		t->column_choice[oj][0] == '\0' )
	    {
		break;
	    }
	}
    }
    else
    {
	pos = t->edit_col*t->nrows + t->edit_row;
	pos += sign;
	for(k = 0; k < n; k++, pos += sign) {
	    if(pos < 0) pos = n-1;
	    if(pos >= n) pos = 0;
	    col = pos/t->nrows;
	    row = pos - col*t->nrows;
	    oi = t->row_order[row];
	    oj = t->col_order[col];
	    if(	t->row_editable[oi] && t->col_editable[oj] &&
		t->table_class->cellEditable(oi, oj) &&
		!t->table_class->getCellChoice(oi, oj) &&
		t->column_choice[oj][0] == '\0' )
	    {
		break;
	    }
	}
    }
    if(row == -1 || col == -1) return;

    top = MmTableGetBarValue(t->vbar);
    if(row < top) {
	MmTableSetBarValue(t->vbar, row);
	_TCanvasVScroll(w);
    }
    if(row > t->bottom) {
	int value = row - (t->top - t->bottom);
	MmTableSetBarValue(t->vbar, value);
	_TCanvasVScroll(w);
    }
    if(col < t->left && t->hbar && XtIsManaged(t->hbar)) {
	int n, value, visible, min, max;
	Arg args[4];
	n = 0;
	XtSetArg(args[n], XmNvalue, &value); n++;
	XtSetArg(args[n], XmNsliderSize, &visible); n++;
	XtSetArg(args[n], XmNminimum, &min); n++;
	XtSetArg(args[n], XmNmaximum, &max); n++;
	XtGetValues(t->hbar, args, n);

	value = col;
	MmTableSetBarValue(t->hbar, value);
	_TCanvasHScroll(t->canvas);
	n = 0;
	XtSetArg(args[n], XmNvalue, &value); n++;
	XtGetValues(t->hbar, args, n);
	t->last_hbar_value = value;
	if(t->header) _HCanvasRedisplay(t->header);
    }
    if(col >= t->right && t->hbar && XtIsManaged(t->hbar)) {
	int n, value, visible, min, max;
	Arg args[4];
	n = 0;
	XtSetArg(args[n], XmNvalue, &value); n++;
	XtSetArg(args[n], XmNsliderSize, &visible); n++;
	XtSetArg(args[n], XmNminimum, &min); n++;
	XtSetArg(args[n], XmNmaximum, &max); n++;
	XtGetValues(t->hbar, args, n);

	n = col + 1;
	if(n >= t->ncols) n = t->ncols-1;
	while(t->right <= n) {
	    value++;
	    if(value > max-visible) break;
	    MmTableSetBarValue(t->hbar, value);
	    _TCanvasHScroll(t->canvas);
	    XtSetArg(args[0], XmNvalue, &value);
	    XtGetValues(t->hbar, args, 1);
	    t->last_hbar_value = value;
	    if(t->header) _HCanvasRedisplay(t->header);
	}
    }

    if(row >= 0 && row < t->nrows &&
	col >= 0 && col < t->ncols)
    {
	r_state = t->row_state[t->row_order[row]];
	t->row_state[t->row_order[row]] = False;
	MmTableEditModeOff(tc->table);
	t->edit_row_state = r_state;
	t->edit_row = row;
	t->edit_col = col;
	s = t->columns[t->col_order[t->edit_col]][t->row_order[t->edit_row]];
	t->edit_pos = strlen(s);
	t->edit_x = stringWidthLen(w, s, t->edit_pos);
	Free(t->edit_string);
	t->edit_string = strdup(s);
	drawEditCursor(w, t->edit_x, 0, t->edit_col, t->edit_row);
	client_data = (long *)malloc(4*sizeof(long));
	client_data[0] = t->edit_col;
	client_data[1] = t->edit_row;
	client_data[2] = 1;
	client_data[3] = (long)w;
	editCursor((XtPointer)client_data, 0);
	_MmTableResetInfo(tc->table);
    }
}

static void
positionEditCursor(TCanvasWidget w, XEvent *event, int col, int row)
{
    int i, pos, dmin, start, ex, len, x;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int cursor_x = ((XButtonEvent *)event)->x;

    String s = t->columns[t->col_order[col]][t->row_order[row]];
    if(s == NULL) return;
    pos = -1;
    dmin = 100000;
    start = 0;
    if(t->col_alignment[t->col_order[col]] == LEFT_JUSTIFY) {
	start = t->col_beg[col] - t->col_beg[t->left] + t->margin + 3;
    }
    else if(t->col_alignment[t->col_order[col]] == RIGHT_JUSTIFY) {
	start = t->col_end[col] - t->col_beg[t->left] + t->margin - 3
		-stringWidth(w, s);
    }
    ex = 0;
    len = (int)strlen(s);
    for(i = -1; i < len; i++) {
	x = (i == -1) ? start : start + stringWidthLen(w, s, i+1);
	if(abs(cursor_x - x) < dmin) {
	    pos = i;
	    dmin = abs(cursor_x - x);
	    ex = x - start;
	}
    }
    t->edit_pos = pos+1;
    t->edit_x = ex;
}

static void
editCursor(XtPointer client_data, XtIntervalId id)
{

    int col, row, on;
    long *data;
    unsigned long interval;
    TCanvasWidget w;
    TCanvasPart *tc;
    MmTablePart *t;

    data = (long *)client_data;
    col = data[0];
    row = data[1];
    on  = data[2];
    w = (TCanvasWidget)data[3];
    tc = &w->tCanvas;
    t = &tc->table->mmTable;
    
    if(t->edit_col != col || t->edit_row != row) {
	free(client_data);
	return;
    }

    if(on || t->have_focus) {
	drawEditCursor(w, t->edit_x, on, col, row);
    }
    data[2] = !on;

    interval = (on) ? 1300 : 700;
    if(t->app_context) {
	XtAppAddTimeOut(t->app_context, interval,
				(XtTimerCallbackProc)editCursor, client_data);
    }
    else {
	XtAddTimeOut(interval, (XtTimerCallbackProc)editCursor, client_data);
    }
}

static void
drawEditCursor(TCanvasWidget w, int x, int on, int col, int row)
{
    int start, y;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(x < 0 || col < 0 || row < 0) return;

    start = 0;
    if(t->col_alignment[t->col_order[col]] == LEFT_JUSTIFY) {
	start = t->col_beg[col] - t->col_beg[t->left] + t->margin + 3;
    }
    else if(t->col_alignment[t->col_order[col]] == RIGHT_JUSTIFY) {
	start = t->col_end[col] - t->col_beg[t->left] + t->margin - 3
	    - stringWidth(w, t->columns[t->col_order[col]][t->row_order[row]]);
    }
    x += start;
    if(on) XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
    else XSetForeground(XtDisplay(w), tc->gc, t->table_background);
    y = (row - t->top)*t->row_height;
    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, x, y+3, x, y+3+t->max_ascent);
    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, x-2, y+3, x+2, y+3);
    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, x-2, y+4+t->max_ascent,
		x+2, y+4+t->max_ascent);
}

/** 
 *  
 */
void
_TCanvasSelectRow(TCanvasWidget w, int row, Boolean state)
{
    int i, irow, y, width;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(row < 0 || row >= t->nrows + t->nhidden) return;
    if(state == t->row_state[row]) return;
    if(t->radioSelect && t->row_state[row]) return;
    t->row_state[row] = state;
    if(!XtIsRealized((Widget)w)) return;

    irow = t->display_order[row];

    if(!t->select_toggles)
    {
	y = (irow - t->top)*t->row_height;
	width = t->col_end[t->right] - t->col_beg[t->left] + t->margin - 3;
	XSetForeground(XtDisplay(w), tc->gc, t->table_background);
	XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
	    t->row_height-3);
	_TCanvasDrawRows(w, irow, irow, t->left, t->right);

	if(t->singleSelect && state)
	{
	    for(i = 0; i < t->nrows; i++)
		if(i != irow && t->row_state[t->row_order[i]])
	    {
		t->row_state[t->row_order[i]] = False;
		if(i >= t->top && i <= t->bottom)
		{
		    y = (i - t->top)*t->row_height;
		    width = t->col_end[t->right] - t->col_beg[t->left]
				+ t->margin - 3;
		    XSetForeground(XtDisplay(w), tc->gc,
				t->table_background);
		    XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2,
				width, t->row_height-3);
		    _TCanvasDrawRows(w, i, i, t->left, t->right);
		}
	    }
	}
	XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	    w->core.width, w->core.height, 0, 0);
	if(t->color_only) {
	    XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0,
		w->core.width-1, w->core.height-1);
	}
    }
    else
    {
	_TCanvas_drawToggle(t->canvas, irow);

	if(t->singleSelect && state)
	{
	    for(i = 0; i < t->nrows; i++)
		if(i != irow && t->row_state[t->row_order[i]])
	    {
		t->row_state[t->row_order[i]] = False;
		if(i >= t->top && i <= t->bottom) {
		    _TCanvas_drawToggle(t->canvas, i);
		}
	    }
	}
    }
}

/** 
 *  
 */
void
_TCanvasSelectRows(TCanvasWidget w, int num_rows, int *rows, Boolean *states)
{
    int i, irow, y, width;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    for(i = 0; i < num_rows; i++) {
	if(rows[i] >= 0 && rows[i] < t->nrows + t->nhidden) {
	    t->row_state[rows[i]] = states[i];
	}
    }
    if(!XtIsRealized((Widget)w)) return;

    if(!t->select_toggles)
    {
	for(i = 0; i < num_rows; i++) {
	    irow = t->display_order[rows[i]];
	    if(irow >= t->top && irow <= t->bottom) {
		y = (irow - t->top)*t->row_height;
		width = t->col_end[t->right] - t->col_beg[t->left]
				+ t->margin - 3;
		XSetForeground(XtDisplay(w), tc->gc, t->table_background);
		XFillRectangle(XtDisplay(w), tc->image, tc->gc, 2, y+2, width,
				t->row_height-3);
		_TCanvasDrawRows(w, irow, irow, t->left, t->right);
	    }
	}
	XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	    w->core.width, w->core.height, 0, 0);
	if(t->color_only) {
	    XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0,
		w->core.width-1, w->core.height-1);
	}
    }
    else {
	for(i = 0; i < num_rows; i++) {
	    irow = t->display_order[rows[i]];
	    if(irow >= t->top && irow <= t->bottom) {
		_TCanvas_drawToggle(t->canvas, irow);
	    }
	}
    }
}

static void
highlightRow(TCanvasWidget w, int row, Boolean on)
{
    int j, jmin, top, x, y, width;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(row < 0 || row >= t->nrows  + t->nhidden || t->select_toggles
		|| !XtIsRealized((Widget)w)) return;

    if(on) XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
    else XSetForeground(XtDisplay(w), tc->gc, t->table_background);

    top = MmTableGetBarValue(t->vbar);
    y = (row - t->top)*t->row_height;
    width = t->col_end[t->right] - t->col_beg[t->left] + t->margin;

    if(t->left == 0) {
	if(tc->verticalLines) {
	    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 1, y+1, width-1, y+1);
	}
	else {
	    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 1, y, width-1, y);
	}
        XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 1, y+t->row_height-1,
		width-1, y+t->row_height-1);
        XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 1, y+1, 1,
		y+t->row_height-1);
    }
    else {
	width -= 2;
	if(tc->verticalLines) {
	    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 0, y+1, width, y+1);
	}
	else {
	    XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 0, y, width, y);
	}
        XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, 0, y+t->row_height-1,
		width, y+t->row_height-1);
    }
    if(t->right == t->ncols-1) {
	x = t->col_end[t->right] - t->col_beg[t->left] + t->margin - 1;
        XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, x, y+1, x,
		y+t->row_height-1);
    }
    if(!on && tc->verticalLines) {
	if(!t->row_state[t->row_order[row]] || t->select_toggles) {
	    XSetForeground(XtDisplay(w), tc->gc, w->primitive.foreground);
	}
	jmin = (t->left > 0) ? t->left : 1;
	for(j = jmin; j <= t->right; j++) {
	    x = t->col_beg[j] - t->col_beg[t->left] + t->margin;
	    if(mod(t->left, t->n_small_tdel)) {
		XDrawLine(XtDisplay(w), XtWindow(w), tc->gc, x, y+1, x,
			y+t->row_height-1);
	    }
	}
    }
    if(!on) {
	int ro = t->row_order[row];
	for(j = t->left; j <= t->right; j++) {
	    if(t->highlight[t->col_order[j]][ro]) {
		int hx = t->col_beg[j] - t->col_beg[t->left] + t->margin + 1;
		int hy = (row-top)*t->row_height + 1;
		int hw = t->col_end[j] - t->col_beg[j] - 2;
		int hh = t->row_height-2;
		XSetForeground(XtDisplay(w), tc->gc, t->highlight_color);
		XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, hx,hy,hw,hh);
	    }
	}
    }
}

/** 
 *  
 */
void
_TCanvasHighlightField(TCanvasWidget w, int row, int col, Boolean highlight)
{
    int ro, co, hx, hy, hw, hh;
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;

    if(row < 0 || row >= t->nrows + t->nhidden
	|| col < 0 || col >= t->ncols) return;

    ro = t->display_order[row];
    for(co = 0; co < t->ncols; co++) {
	if(t->col_order[co] == col) break;
    }

    t->highlight[col][row] = highlight;
    hx = t->col_beg[co] - t->col_beg[t->left] + t->margin + 1;
    hy = (ro - t->top)*t->row_height + 1;
    hw = t->col_end[co] - t->col_beg[co] - 2;
    hh = t->row_height-2;

    if(XtIsRealized((Widget)w))
    {
	if(highlight) {
	    XSetForeground(XtDisplay(w), tc->gc, t->highlight_color);
	    XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, hx, hy, hw, hh);
	    XDrawRectangle(XtDisplay(w), tc->image, tc->gc, hx, hy, hw, hh);
	}
	else {
	    _TCanvasDrawRows(w, row, row, col, col);
	    XCopyArea(XtDisplay(w), tc->image, XtWindow(w),
		tc->gc, 0, 0, w->core.width, w->core.height, 0, 0);
	    if(t->color_only) {
		XDrawRectangle(XtDisplay(w), XtWindow(w), tc->gc, 0, 0,
		    w->core.width-1, w->core.height-1);
	    }
	}
    }
}

static int
stringWidth(TCanvasWidget w, String s)
{
    if(s == (String)NULL || (int)strlen(s) == 0) return 0;
    return XTextWidth(w->tCanvas.table->mmTable.font, s, (int)strlen(s));
}

static int
stringWidthLen(TCanvasWidget w, String s, int len)
{
    return XTextWidth(w->tCanvas.table->mmTable.font, s, len);
}

static String
substring(TCanvasWidget w, String s, int i1, int i2)
{
    String sub;
    if(i2 < i1 || !(sub = (char *)MallocIt(w, i2-i1+1)) ) return (String)NULL;

    strncpy(sub, s+i1, i2-i1);
    sub[i2-i1] = '\0';

    return sub;
}

/**
 *  
 */
static void
TCanvasMotion(TCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    int x, y;

    if(event->type == MotionNotify) {
	x = ((XButtonEvent *)event)->x;
	y = ((XButtonEvent *)event)->y;
    }
    else if(event->type == EnterNotify) {
	x = ((XCrossingEvent *)event)->x;
	y = ((XCrossingEvent *)event)->y;
    }
    else {
	return;
    }

    SetCursorMode(w, x, y);
}

static void
SetCursorMode(TCanvasWidget w, int cursor_x, int cursor_y)
{
    TCanvasPart *tc = &w->tCanvas;
    MmTablePart *t = &tc->table->mmTable;
    int j, x, top, row;

    if(t->nrows <= 0 || t->ncols <= 0) return;

    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x > x && cursor_x <= x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
		t->col_width[t->col_order[j]];
    }
    if(j > t->right || !t->col_editable[t->col_order[j]]) {
	TCanvasSetCursor(w, (char *)"default");
	return;
    }

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;

    if(row < 0 || row >= t->nrows ||
	!t->table_class->cellEditable(t->row_order[row], t->col_order[j]) ||
	!t->row_editable[t->row_order[row]])
    {
	TCanvasSetCursor(w, (char *)"default");
	return;
    }

    if(t->column_choice[t->col_order[j]][0] != '\0' ||
	t->table_class->getCellChoice(t->row_order[row], t->col_order[j]) )
    {
	TCanvasSetCursor(w, (char *)"hand");
    }
    else {
	TCanvasSetCursor(w, (char *)"default");
    }
}

static void
TCanvasSetCursor(TCanvasWidget w, char *type)
{
    TCanvasPart *tc = &w->tCanvas;

    if(strcmp(tc->cursor_type, type)) {
	if(!strcmp(type, "default")) {
	     XUndefineCursor(XtDisplay(w), XtWindow(w));
	}
	else {
	    XDefineCursor(XtDisplay(w), XtWindow(w), GetCursor((Widget)w,type));
	}
	stringcpy(tc->cursor_type, type, sizeof(tc->cursor_type));
	XFlush(XtDisplay(w));
    }
}
