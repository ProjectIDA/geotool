/** \file HCanvas.c
 *  \brief Defines widget HCanvas.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      HCanvas Widget -- widget for displaying tabular data.
 *
 * SYNOPSIS
 *      #include "HCanvas.h"
 *      Widget
 *      HCanvasCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      HCanvas.h
 *
 * AUTHOR
 *      I. Henson -- February 2000
 *	Multimax, Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "widget/HCanvasP.h"
#include "Xm/RowColumn.h"
#include "Xm/Label.h"
#include "Xm/Separator.h"
#include "Xm/PushB.h"
#include "Xm/Form.h"
#include "Xm/Text.h"
#include "widget/Table.h"

#define offset(field)	XtOffset(HCanvasWidget, hCanvas.field)
static XtResource	resources[] =
{
    {XtNtableWidget, XtCTableWidget, XtRWidget, sizeof(Widget),
	offset(table), XtRImmediate, NULL},
    {XtNcanvasWidget, XtCCanvasWidget, XtRWidget, sizeof(Widget),
	offset(canvas), XtRImmediate, NULL},
};
#undef offset

/* Private functions */

static void Initialize(Widget req, Widget new_index);
static Boolean SetValues(HCanvasWidget cur, HCanvasWidget req,
			HCanvasWidget new_index);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static void Resize(Widget widget);
static void Redisplay(Widget widget, XEvent *event, Region region);
static void redraw(HCanvasWidget w);
static void drawColumn(HCanvasWidget w, int j, int shift);
static void mousePressed(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseReleased(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void setImages(HCanvasWidget w, int j);
static void clearColumn(HCanvasWidget w, int j);
static void mouseDragged(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void moveColumn(HCanvasWidget w, XEvent *event);
static void mouseMoved(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void EnterWindow(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void LeaveWindow(HCanvasWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Destroy(Widget widget);
static int getBarMaximum(Widget w);
static void highlightColumn(HCanvasWidget w, int column, Boolean on);
static void HCanvasDisplayMenu(HCanvasWidget w, XEvent *event);
static void HCanvasCopyAllCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasCopyRowsCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasCopyColumnsCB(Widget w, XtPointer client, XtPointer data);
static void HCanvasCopyThisCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasPasteCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasSortCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasSortUniqueCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasShowAllCB	(Widget w, XtPointer client, XtPointer data);
static void HCanvasFindCB	(Widget w, XtPointer client, XtPointer data);
static void FindApplyCB		(Widget w, XtPointer client, XtPointer data);
static void FindCloseCB		(Widget w, XtPointer client, XtPointer data);
static void FindNextCB		(Widget w, XtPointer client, XtPointer data);
static void HCanvasSortSelectedCB(Widget w, XtPointer client, XtPointer data);
static void HCanvasReverseCB	(Widget w, XtPointer client, XtPointer data);

static Widget menu_popup = NULL;
static Widget copy_all_button = NULL;
static Widget copy_rows_button = NULL;
static Widget copy_columns_button = NULL;
static Widget copy_this_button = NULL;
static Widget paste_button = NULL;
static Widget sort_button = NULL;
static Widget sort_unique_button = NULL;
static Widget sort_selected_button = NULL;
static Widget reverse_button = NULL;
static Widget show_all_button = NULL;
static Widget find_button = NULL;

static char     defTrans[] =
" ~Shift ~Ctrl<Btn1Down>:	mousePressed() \n\
  ~Shift ~Ctrl<Btn2Down>:	mousePressed() \n\
  ~Shift ~Ctrl<Btn3Down>:	mousePressedMenu() \n\
  ~Shift Ctrl<Btn1Down>:	mousePressed(Ctrl) \n\
  ~Shift Ctrl<Btn2Down>:	mousePressed(Ctrl) \n\
  <Btn1Up>:		mouseReleased() \n\
  <Btn2Up>:		mouseReleased() \n\
  <Btn1Motion>:		mouseDragged() \n\
  <Btn2Motion>:		mouseDragged() \n\
  <MouseMoved>:		mouseMoved() \n\
  <EnterWindow>:	EnterWindow() \n\
  <LeaveWindow>:	LeaveWindow()";

static XtActionsRec	actionsList[] =

{
    {(char *)"mousePressed",	(XtActionProc)mousePressed},
    {(char *)"mousePressedMenu",(XtActionProc)_HCanvas_mousePressedMenu},
    {(char *)"mouseReleased",	(XtActionProc)mouseReleased},
    {(char *)"mouseDragged",	(XtActionProc)mouseDragged},
    {(char *)"mouseMoved",	(XtActionProc)mouseMoved},
    {(char *)"EnterWindow",	(XtActionProc)EnterWindow},
    {(char *)"LeaveWindow",	(XtActionProc)LeaveWindow},
};

HCanvasClassRec	hCanvasClassRec = 
{
    {	/* core fields */
	(WidgetClass)(&xmPrimitiveClassRec),	/* superclass */
	(char *)"HCanvas",			/* class_name */
	sizeof(HCanvasRec),		/* widget_size */
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
    {       /* primitive_class fields */
	(XtWidgetProc)_XtInherit,	/* Primitive border_highlight */
	(XtWidgetProc)_XtInherit,	/* Primitive border_unhighlight */
	NULL,				/* translations */
	NULL,				/* arm_and_activate */
	NULL,				/* get resources */
	0,				/* num get_resources */
	NULL,
    },
    {	/* HCanvasClass fields */
	0,			/* empty */
    },
};

WidgetClass hCanvasWidgetClass = (WidgetClass)&hCanvasClassRec;

static void
Initialize(Widget w_req, Widget w_new)
{
    HCanvasWidget new_index = (HCanvasWidget)w_new;
    HCanvasPart *h = &new_index->hCanvas;
    XtTranslations translations;

    if(new_index->core.width == 0)
	new_index->core.width =
		(Dimension)(XWidthOfScreen(XtScreen(new_index))/2.5);
    if(new_index->core.height == 0)
	new_index->core.height = XHeightOfScreen(XtScreen(new_index))/2;

    /* no data displayed yet.
     */
    h->image = (Pixmap)NULL;
    h->drag_image = (Pixmap)NULL;
    h->copy_image = (Pixmap)NULL;
    h->gc = (GC)NULL;
    h->copy_x = 0;
    h->mouse_down_column = -1;
    h->mouse_menu_column = -1;
    h->mouse_down_x = 0;
    h->shift = 0;
    h->drag_w = 0;
    h->drag_h = 0;
    h->inside_column = -1;
    h->moved_column = False;
    h->moved_state = False;
    h->find_form = NULL;
    h->find_text = NULL;

    translations = XtParseTranslationTable(defTrans);
    XtOverrideTranslations((Widget)new_index, translations);
}

static Boolean
SetValues(HCanvasWidget cur, HCanvasWidget req, HCanvasWidget new_index)
{
    /* nothing to do. */
    return False;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    HCanvasWidget w = (HCanvasWidget)widget;
    Pixel   white, black;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;
    XWindowAttributes window_attributes;

    (*((hCanvasWidgetClass->core_class.superclass)->core_class.realize))
	((Widget)w,  valueMask, attrs);

    XGetWindowAttributes(XtDisplay(w), XtWindow(w), &window_attributes);

    h->depth = window_attributes.depth;

    h->gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
    /*
     * check that no foreground colors are same as background
     */
    white = string_to_pixel(widget, "White");
    black = string_to_pixel(widget, "Black");
    if(w->primitive.foreground == w->core.background_pixel) {
	w->primitive.foreground = (w->core.background_pixel != black) ?
		black : white;
    }

    XSetBackground(XtDisplay(w), h->gc, w->core.background_pixel);

    XSetFont(XtDisplay(w), h->gc, t->font->fid);

    /* important to create pixmap with (width+1,height+1) for calls to
     * XFillRectangle(... width, height)
     * Otherwise get bad problems with some X-servers
     */
    h->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
		w->core.width+1, w->core.height+1, h->depth);
    XSetForeground(XtDisplay(w), h->gc, w->core.background_pixel);
    XFillRectangle(XtDisplay(w), h->image, h->gc, 0, 0,
		w->core.width, w->core.height);
    XSetForeground(XtDisplay(w), h->gc, w->primitive.foreground);
}

static void
Resize(Widget widget)
{
    HCanvasWidget w = (HCanvasWidget)widget;
    HCanvasPart *h = &w->hCanvas;

    if(!XtIsRealized((Widget)w)) return;

    if(h->image != (Pixmap)NULL) {
	XFreePixmap(XtDisplay(w), h->image);
	h->image = (Pixmap)NULL;
    }
    if(w->core.width <= 0 || w->core.height <= 0) return;
    /* important to create pixmap with (width+1,height+1) for calls to
     * XFillRectangle(... width, height)
     * Otherwise get bad problems with some X-servers
     */
    h->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
		w->core.width+1, w->core.height+1, h->depth);
    redraw(w);
}

static void
Redisplay(Widget widget, XEvent *event, Region region)
{
    HCanvasWidget w = (HCanvasWidget)widget;
    HCanvasPart *h = &w->hCanvas;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(!XtIsRealized((Widget)w)) return;

    if(h->image == (Pixmap)NULL) {
	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
        h->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
			w->core.width+1, w->core.height+1, h->depth);
	redraw(w);
    }
    if(h->image != (Pixmap)NULL) {
	XCopyArea(XtDisplay(w), h->image, XtWindow(w),
	    h->gc, 0, 0, w->core.width, w->core.height, 0, 0);
    }
}

static void
redraw(HCanvasWidget w)
{
    int j, left, right;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(w->core.width <= 0 || w->core.height <= 0) return;
    if(!XtIsRealized((Widget)w)) return;


    if(h->image == (Pixmap)NULL) {
	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
        h->image = XCreatePixmap(XtDisplay(w), XtWindow(w),
			w->core.width+1, w->core.height+1, h->depth);
    }

    XSetForeground(XtDisplay(w), h->gc, w->core.background_pixel);
    XFillRectangle(XtDisplay(w), h->image, h->gc, 0, 0,
		w->core.width, w->core.height);
    XSetForeground(XtDisplay(w), h->gc, w->primitive.foreground);

    if((left = MmTableGetBarValue(t->hbar)) < 0) return;
    if(left >= t->ncols) left = t->ncols-1;
    if((right = _TCanvasGetRight(h->canvas, left)) < 0) return;

    for(j = left; j <= right ; j++) {
	if(j != h->mouse_down_column) drawColumn(w, j, 0);
    }
    if(h->mouse_down_column >= 0) {
	drawColumn(w, h->mouse_down_column, h->shift);
    }
				
    if(right >= left) {
	int x;
	int width = t->col_end[right] - t->col_beg[left] + t->margin;
	int height = w->core.height;
	XSetForeground(XtDisplay(w), h->gc, w->primitive.foreground);
	XDrawRectangle(XtDisplay(w), h->image, h->gc, 0, 0, width, height-1);
	for(j = left+1; j <= right; j++) {
	    x = t->col_beg[j] - t->col_beg[left] + t->margin;
	    XDrawLine(XtDisplay(w), h->image, h->gc, x, 0, x, height-1);
	}
	x = t->col_end[right] - t->col_beg[left] + t->margin;
	if(x >= w->core.width) x = w->core.width-1;
	XDrawLine(XtDisplay(w), h->image, h->gc, x, 0, x, height-1);
    }
}

static void
drawColumn(HCanvasWidget w, int j, int shift)
{
    int x, xbeg, xend, fill_w;
    String s;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(j < 0 || j >= t->ncols) return;
    if(j == 0) {
	xbeg = 0;
	xend = t->col_width[t->col_order[j]] + t->margin;
    }
    else {
	xbeg = t->col_beg[j] - t->col_beg[t->left] + t->margin;
	xend = xbeg + t->col_width[t->col_order[j]];
    }
    fill_w = xend - xbeg - 3;
    xbeg += shift;
    if(t->column_state[t->col_order[j]]) {
	XSetForeground(XtDisplay(w), h->gc, w->primitive.foreground);
	XFillRectangle(XtDisplay(w), h->image, h->gc, xbeg+2, 2, fill_w,
		w->core.height-4);
	XSetForeground(XtDisplay(w), h->gc, w->core.background_pixel);
    }
    else {
	if(t->col_color[t->col_order[j]] == (Pixel)NULL) {
	    XSetForeground(XtDisplay(w), h->gc, w->core.background_pixel);
	}
	else {
	    XSetForeground(XtDisplay(w), h->gc, t->col_color[t->col_order[j]]);
	}
	XFillRectangle(XtDisplay(w), h->image, h->gc, xbeg+2, 2, fill_w,
		w->core.height-4);
	XSetForeground(XtDisplay(w), h->gc, w->primitive.foreground);
    }
    s = t->column_labels[t->col_order[j]];
    x = t->col_beg[j] - t->col_beg[t->left] + t->margin + 4;
    x += shift;
    XDrawString(XtDisplay(w), h->image, h->gc, x, t->max_ascent+2, s,
		(int)strlen(s));
}

void
_HCanvasRedisplay(HCanvasWidget w)
{
    redraw(w);
    Redisplay((Widget)w, NULL, NULL);
}

static void
mousePressed(HCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    int i, j, x, cursor_x;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(t->nrows == 0) return;

    _MmTableResetInfo(h->table);
    MmTableEditModeOff(h->table);
    h->moved_column = False;
    h->mouse_down_column = -1;
    if(!t->columnSelectable) return;

    cursor_x = ((XButtonEvent *)event)->x;
    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x >= x && cursor_x < x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
					t->col_width[t->col_order[j]];
    }
    if(j <= t->right) {
	h->mouse_down_column = j;
	h->moved_state = t->column_state[t->col_order[j]];
	h->mouse_down_x = cursor_x -
			(t->col_beg[j] - t->col_beg[t->left] + t->margin + 3);
	if(*num_params < 1 && t->columnSingleSelect) {	/* no modifiers */
	    for(i = 0; i < t->ncols; i++) {
		    if(i != j) t->column_state[t->col_order[i]] = False;
	    }
	}
	t->column_state[t->col_order[j]]= !t->column_state[t->col_order[j]];
	_HCanvasRedisplay(w);
	setImages(w, j);
    }
}

static void
mouseReleased(HCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    int x;
    HCanvasPart *h = &w->hCanvas;
    TCanvasPart *tc = &h->canvas->tCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(h->mouse_down_column == -1) return;
    if(h->moved_column) {
	t->column_state[t->col_order[h->mouse_down_column]] = h->moved_state;
    }

    XCopyArea(XtDisplay(w), h->copy_image, tc->image, h->gc,
		0, 0, h->drag_w, h->drag_h, h->copy_x, 0);

    x = (h->mouse_down_column == 0) ? 0 :
	t->col_beg[h->mouse_down_column] - t->col_beg[t->left] + t->margin;

    XCopyArea(XtDisplay(w), h->drag_image, tc->image, h->gc,
		0, 0, h->drag_w, h->drag_h, x, 0);

    _TCanvasRepaint(h->canvas);
    h->mouse_down_column = -1;
    _HCanvasRedisplay(w);
    h->shift = 0;

    if(h->moved_column) {
	XtCallCallbacks((Widget)h->table, XtNcolumnMovedCallback, t->col_order);
    }
    h->moved_column = False;

    XtCallCallbacks((Widget)h->table, XtNselectColumnCallback,
			(XtPointer)&h->mouse_down_column);
}

static void
mouseMoved(HCanvasWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    int j, x, cursor_x;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(t->ncols == 0 || !t->columnSelectable) return;

    cursor_x = ((XButtonEvent *)event)->x;
    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x >= x && cursor_x < x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
					t->col_width[t->col_order[j]];
    }
    if(j != h->inside_column) {
	highlightColumn(w, h->inside_column, False);
    }
    if(j <= t->right) {
	h->inside_column = j;
	highlightColumn(w, h->inside_column, True);
    }
    else h->inside_column = -1;
}

void
_HCanvas_mousePressedMenu(HCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    int j, x, cursor_x;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(t->nrows == 0) return;

    _MmTableResetInfo(h->table);
    MmTableEditModeOff(h->table);
    h->moved_column = False;
    h->mouse_down_column = -1;

    cursor_x = ((XButtonEvent *)event)->x;
    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x >= x && cursor_x < x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
					t->col_width[t->col_order[j]];
    }
    if(j <= t->right) {
	h->mouse_down_column = j;
	h->mouse_menu_column = j;
	HCanvasDisplayMenu(w, event);
    }
}

static void
HCanvasDisplayMenu(HCanvasWidget w, XEvent *event)
{
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	int i;

	if(menu_popup == NULL)
	{
	    int n;
	    Arg args[2];
	    Widget toplevel;
	    XtTranslations translations;
	    char trans[] =
"<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

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
	    menu_popup = XmCreatePopupMenu(toplevel, (char *)"popupMenu", args, n);
	    translations = XtParseTranslationTable(trans);
	    copy_rows_button = XtVaCreateManagedWidget("Copy Selected Rows",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    copy_all_button = XtVaCreateManagedWidget("Copy All Rows",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    copy_columns_button = XtVaCreateManagedWidget(
			"Copy Selected Columns", xmPushButtonWidgetClass,				menu_popup, XmNtranslations, translations,
			NULL);
	    copy_this_button = XtVaCreateManagedWidget(
			"Copy This Column", xmPushButtonWidgetClass,
			menu_popup, XmNtranslations, translations,
			NULL);
	    paste_button = XtVaCreateManagedWidget("Paste",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    sort_button = XtVaCreateManagedWidget("Sort",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    sort_unique_button = XtVaCreateManagedWidget("Sort Unique",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    sort_selected_button = XtVaCreateManagedWidget("Sort Selected",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    reverse_button = XtVaCreateManagedWidget("Reverse Order",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    show_all_button = XtVaCreateManagedWidget("Show All",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	    find_button = XtVaCreateManagedWidget("Find...",
			xmPushButtonWidgetClass, menu_popup,
			XmNtranslations, translations,
			NULL);
	}
	for(i = 0; i < t->nrows && !t->row_state[t->row_order[i]]; i++);

	if(i < t->nrows) {
	    XtRemoveAllCallbacks(copy_rows_button, XmNactivateCallback);
	    XtAddCallback(copy_rows_button, XmNactivateCallback,
		HCanvasCopyRowsCB, (XtPointer)w);
	    XtSetSensitive(copy_rows_button, True);
	}
	else {
	    XtSetSensitive(copy_rows_button, False);
	}

	for(i = 0; i < t->ncols && !t->column_state[t->col_order[i]]; i++);

	if(i < t->ncols) {
	    XtRemoveAllCallbacks(copy_columns_button, XmNactivateCallback);
	    XtAddCallback(copy_columns_button, XmNactivateCallback,
			HCanvasCopyColumnsCB, (XtPointer)w);
	    XtSetSensitive(copy_columns_button, True);
	}
	else {
	    XtSetSensitive(copy_columns_button, False);
	}
	XtRemoveAllCallbacks(copy_all_button, XmNactivateCallback);
	XtAddCallback(copy_all_button, XmNactivateCallback,
			HCanvasCopyAllCB, (XtPointer)w);
	XtRemoveAllCallbacks(copy_this_button, XmNactivateCallback);
	XtAddCallback(copy_this_button, XmNactivateCallback,
			HCanvasCopyThisCB, (XtPointer)w);
	XtRemoveAllCallbacks(paste_button, XmNactivateCallback);
	XtAddCallback(paste_button, XmNactivateCallback,
			HCanvasPasteCB, (XtPointer)w);
	XtRemoveAllCallbacks(sort_button, XmNactivateCallback);
	XtAddCallback(sort_button, XmNactivateCallback, HCanvasSortCB,
			(XtPointer)w);
	XtRemoveAllCallbacks(sort_unique_button, XmNactivateCallback);
	XtAddCallback(sort_unique_button, XmNactivateCallback,
			HCanvasSortUniqueCB, (XtPointer)w);
	XtRemoveAllCallbacks(sort_selected_button, XmNactivateCallback);
	XtAddCallback(sort_selected_button, XmNactivateCallback,
			HCanvasSortSelectedCB, (XtPointer)w);
	XtRemoveAllCallbacks(reverse_button, XmNactivateCallback);
	XtAddCallback(reverse_button, XmNactivateCallback,
			HCanvasReverseCB, (XtPointer)w);
	XtRemoveAllCallbacks(show_all_button, XmNactivateCallback);
	XtAddCallback(show_all_button, XmNactivateCallback, HCanvasShowAllCB,
			(XtPointer)w);
	XtRemoveAllCallbacks(find_button, XmNactivateCallback);
	XtAddCallback(find_button, XmNactivateCallback, HCanvasFindCB,
			(XtPointer)w);

	XmMenuPosition(menu_popup, (XButtonPressedEvent *)event);
	XtManageChild(menu_popup);
}

static void
HCanvasCopyRowsCB(Widget widget, XtPointer client_data, XtPointer data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	XmPushButtonCallbackStruct *c = (XmPushButtonCallbackStruct *)data;
	MmTableCopyPasteCallbackStruct cp;
	cp.selection_type = TABLE_COPY_ROWS;
	cp.time = ((XButtonEvent *)c->event)->time;
        cp.column = t->col_order[h->mouse_down_column];
	XtCallCallbacks((Widget)h->table, XtNcopyPasteCallback, (XtPointer)&cp);
}

static void
HCanvasCopyAllCB(Widget widget, XtPointer client_data, XtPointer data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	XmPushButtonCallbackStruct *c = (XmPushButtonCallbackStruct *)data;
	MmTableCopyPasteCallbackStruct cp;
	cp.selection_type = TABLE_COPY_ALL;
	cp.time = ((XButtonEvent *)c->event)->time;
        cp.column = t->col_order[h->mouse_down_column];
	XtCallCallbacks((Widget)h->table, XtNcopyPasteCallback, (XtPointer)&cp);
}

static void
HCanvasCopyColumnsCB(Widget widget, XtPointer client_data, XtPointer data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	XmPushButtonCallbackStruct *c = (XmPushButtonCallbackStruct *)data;
	MmTableCopyPasteCallbackStruct cp;
	cp.selection_type = TABLE_COPY_COLUMNS;
	cp.time = ((XButtonEvent *)c->event)->time;
        cp.column = t->col_order[h->mouse_down_column];
	XtCallCallbacks((Widget)h->table, XtNcopyPasteCallback, (XtPointer)&cp);
}

static void
HCanvasCopyThisCB(Widget widget, XtPointer client_data, XtPointer data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	XmPushButtonCallbackStruct *c = (XmPushButtonCallbackStruct *)data;
	MmTableCopyPasteCallbackStruct cp;
	cp.selection_type = TABLE_COPY_THIS_COLUMN;
	cp.time = ((XButtonEvent *)c->event)->time;
        cp.column = t->col_order[h->mouse_down_column];
	XtCallCallbacks((Widget)h->table, XtNcopyPasteCallback, (XtPointer)&cp);
}

static void
HCanvasPasteCB(Widget widget, XtPointer client_data, XtPointer data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	XmPushButtonCallbackStruct *c = (XmPushButtonCallbackStruct *)data;
	MmTableCopyPasteCallbackStruct cp;
	cp.selection_type = TABLE_PASTE;
	cp.time = ((XButtonEvent *)c->event)->time;
        cp.column = t->col_order[h->mouse_down_column];
	XtCallCallbacks((Widget)h->table, XtNcopyPasteCallback, (XtPointer)&cp);
}

static void
HCanvasSortCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;

	if(h->mouse_down_column >= 0 && h->mouse_down_column < t->ncols) {
	    t->table_class->sortByColumn(h->mouse_down_column);
	}
}

static void
HCanvasSortUniqueCB(Widget widget, XtPointer client, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;

	if(h->mouse_down_column >= 0 && h->mouse_down_column < t->ncols) {
	    vector<int> v;
	    v.push_back(t->col_order[h->mouse_down_column]);
	    t->table_class->sortUnique(v);
	}
}

static void
HCanvasSortSelectedCB(Widget widget, XtPointer client, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;

	if(h->mouse_down_column >= 0 && h->mouse_down_column < t->ncols) {
	    vector<int> v;
	    v.push_back(t->col_order[h->mouse_down_column]);
	    t->table_class->sortSelected(v);
	}
}

static void
HCanvasReverseCB(Widget widget, XtPointer client, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;

	t->table_class->reverseOrder();
}

static void
HCanvasShowAllCB(Widget widget, XtPointer client, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;

	t->table_class->showAll();
}

static void
destroyFindForm(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	HCanvasPart *h = (HCanvasPart *)client_data;

	h->find_text = NULL;
}

static void
HCanvasFindCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	XmString xm;
	Arg args[1];
	char find_label[200];
	Widget label, rc, close_button, next_button;
	int n;

	if(h->mouse_menu_column < 0 || h->mouse_menu_column >= t->ncols) {
	    return;
	}

	if(h->find_form != NULL) {
	    XtDestroyWidget(h->find_form);
	}

	xm = XmStringCreateLtoR((char *)"Find", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg(args[n], XmNdialogTitle, xm);
	h->find_form = XmCreateFormDialog(w->core.parent, (char *)"Find", args, 1);
	XtAddCallback(h->find_form, XtNdestroyCallback, destroyFindForm,
                        (XtPointer)h);

	XmStringFree(xm);
	snprintf(find_label, sizeof(find_label), "Find %s",
		t->column_labels[t->col_order[h->mouse_menu_column]]);
	xm = XmStringCreateLtoR(find_label, XmSTRING_DEFAULT_CHARSET);

	label = XtVaCreateManagedWidget("label", xmLabelWidgetClass,
			h->find_form,
			XmNlabelString, xm,
			XmNtopAttachment, XmATTACH_FORM,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			NULL);
	XmStringFree(xm);

	h->find_text = XtVaCreateManagedWidget("text", xmTextWidgetClass,
			h->find_form,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, label,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			XmNcolumns, 10,
			XmNvalue, "",
			NULL);
	XtAddCallback(h->find_text, XmNvalueChangedCallback, FindApplyCB,
				(XtPointer)w);

	rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass,
			h->find_form,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, h->find_text,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			XmNorientation, XmHORIZONTAL,
			NULL);
	close_button = XtVaCreateManagedWidget("Close",
				xmPushButtonWidgetClass, rc, NULL);
	XtAddCallback(close_button, XmNactivateCallback, FindCloseCB,
			(XtPointer)w);
	next_button = XtVaCreateManagedWidget("Next",
				xmPushButtonWidgetClass, rc, NULL);
	XtAddCallback(next_button, XmNactivateCallback, FindNextCB,
			(XtPointer)w);
	XtManageChild(h->find_form);
}

static void
FindApplyCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	char *text;
	int i, j, n, len;

	if(h->mouse_menu_column < 0 || h->mouse_menu_column >= t->ncols) {
	    return;
	}
	j = t->col_order[h->mouse_menu_column];

	text = XmTextGetString(h->find_text);
	len = (int)strlen(text);

	n = t->nrows + t->nhidden;

	for(i = 0; i < n &&
		strncasecmp(text, t->columns[j][t->row_order[i]], len); i++);

	if(i < n) {
	    t->table_class->moveToTop(t->row_order[i]);
	}
	XtFree(text);
}

static void
FindCloseCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;

	XtDestroyWidget(h->find_form);
	h->find_form = NULL;
	h->mouse_menu_column = -1;
}

static void
FindNextCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	HCanvasWidget w = (HCanvasWidget)client_data;
	HCanvasPart *h = &w->hCanvas;
	MmTablePart *t = &h->table->mmTable;
	char *text;
	int i, j, n, len;
	int	start;

	if(h->mouse_menu_column < 0 || h->mouse_menu_column >= t->ncols) {
	    return;
	}
	j = t->col_order[h->mouse_menu_column];

	text = XmTextGetString(h->find_text);
	len = (int)strlen(text);

	n = t->nrows + t->nhidden;

	start = MmTableGetBarValue(t->vbar) + 1;
	if(start < 0) start = 0;
	for(i = start; i < n &&
		strncasecmp(text, t->columns[j][t->row_order[i]], len); i++);

	if(i < n) {
	    t->table_class->moveToTop(t->row_order[i]);
	}
	XtFree(text);
}

static void
LeaveWindow(HCanvasWidget w, XEvent *event, String *params,Cardinal *num_params)
{
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(t->ncols > 0 && t->columnSelectable) {
        highlightColumn(w, h->inside_column, False);
	h->inside_column = -1;
    }
}

static void
EnterWindow(HCanvasWidget w, XEvent *event, String *params,Cardinal *num_params)
{
    int j, x, cursor_x;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(t->ncols == 0 || !t->columnSelectable) return;

    cursor_x = ((XButtonEvent *)event)->x;
    x = 0;
    for(j = t->left; j <= t->right; j++) {
	if(cursor_x >= x && cursor_x < x + t->col_width[t->col_order[j]]) break;
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
					t->col_width[t->col_order[j]];
    }
    if(j != h->inside_column) {
	highlightColumn(w, h->inside_column, False);
    }
    if(j <= t->right) {
	h->inside_column = j;
	highlightColumn(w, h->inside_column, True);
    }
    else h->inside_column = -1;
}

static void
highlightColumn(HCanvasWidget w, int column, Boolean on)
{
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;
    int j, x, width;

    if(column < 0 || column >= t->ncols || !XtIsRealized((Widget)w)) return;

    if(on) XSetForeground(XtDisplay(w), h->gc, w->primitive.foreground);
    else XSetForeground(XtDisplay(w), h->gc, w->core.background_pixel);

    x = 0;
    for(j = t->left; j < column; j++) {
	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
					t->col_width[t->col_order[j]];
    }

    width = (column == t->left) ?
		t->margin + t->col_width[t->col_order[column]] :
			t->col_width[t->col_order[column]];
    if(x + width > w->core.width) width = w->core.width - x;

    XDrawLine(XtDisplay(w), XtWindow(w), h->gc, x+1, 1, x+width-1, 1);
    XDrawLine(XtDisplay(w), XtWindow(w), h->gc, x+1, t->row_height,
		x+width-1, t->row_height);
    XDrawLine(XtDisplay(w), XtWindow(w), h->gc, x+1, 1, x+1, t->row_height-1);

    if(column != t->right || t->right == t->ncols-1) {
	XDrawLine(XtDisplay(w), XtWindow(w), h->gc, x+width-1, 1, x+width-1,
		t->row_height);
    }
}

static void
setImages(HCanvasWidget w, int j)
{
    int xbeg, xend;
    XWindowAttributes a;
    HCanvasPart *h = &w->hCanvas;
    TCanvasPart *tc = &h->canvas->tCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(j == 0) {
	xbeg = 0;
	xend = t->col_width[t->col_order[j]] + t->margin;
    }
    else {
	xbeg = t->col_beg[j] - t->col_beg[t->left] + t->margin;
	xend = xbeg + t->col_width[t->col_order[j]];
    }

    h->drag_w = xend - xbeg + 1;
    h->drag_h = h->canvas->core.height;
    if(h->drag_h > (t->bottom - t->top + 1)*t->row_height) {
	h->drag_h = (t->bottom - t->top + 1)*t->row_height;
    }

    XGetWindowAttributes(XtDisplay(w), XtWindow(w), &a);

    if(h->drag_image == (Pixmap)NULL || a.width < h->drag_w ||
	a.height < h->drag_h)
    {
	if(h->drag_image != (Pixmap)NULL) {
	    XFreePixmap(XtDisplay(w), h->drag_image);
	}
	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
	h->drag_image =  XCreatePixmap(XtDisplay(w), XtWindow(w),
				h->drag_w+1, h->drag_h+1, h->depth);
	if(h->copy_image != (Pixmap)NULL) {
	    XFreePixmap(XtDisplay(w), h->copy_image);
	}
	h->copy_image = XCreatePixmap(XtDisplay(w), XtWindow(w),
				h->drag_w+1, h->drag_h+1, h->depth);
    }
    XCopyArea(XtDisplay(w), tc->image, h->drag_image, h->gc,
		xbeg, 0, h->drag_w, h->drag_h, 0, 0);
    XCopyArea(XtDisplay(w), tc->image, h->copy_image, h->gc,
		xbeg, 0, h->drag_w, h->drag_h, 0, 0);

    XSetForeground(XtDisplay(w), h->gc, h->canvas->core.background_pixel);
    XFillRectangle(XtDisplay(w), h->copy_image, h->gc, 1, 1, h->drag_w-2,
		h->drag_h-2);
    h->copy_x = xbeg;
    XSetForeground(XtDisplay(w), h->gc, h->canvas->core.background_pixel);
    XFillRectangle(XtDisplay(w), tc->image, h->gc, xbeg, 0, h->drag_w,
		h->drag_h);
}

static void
clearColumn(HCanvasWidget w, int j)
{
    int xbeg, xend, width, height;
    HCanvasPart *h = &w->hCanvas;
    TCanvasPart *tc = &h->canvas->tCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(j == 0) {
	xbeg = 0;
	xend = t->col_width[t->col_order[j]] + t->margin;
    }
    else {
	xbeg = t->col_beg[j] - t->col_beg[t->left] + t->margin;
	xend = xbeg + t->col_width[t->col_order[j]];
    }
    width = xend - xbeg + 1;
    height = h->canvas->core.height;
    if(height > (t->bottom - t->top+1)*t->row_height) {
	height = (t->bottom - t->top+1)*t->row_height;
    }
    XSetForeground(XtDisplay(w), h->gc, h->canvas->core.background_pixel);
    XFillRectangle(XtDisplay(w), tc->image, h->gc, xbeg, 0, width, height);
}

static void
mouseDragged(HCanvasWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    int i, j, x, ex, dist;
    Boolean out;
    HCanvasPart *h = &w->hCanvas;
    TCanvasPart *tc = &h->canvas->tCanvas;
    MmTablePart *t = &h->table->mmTable;

    if(t->nrows == 0) return;

    if(h->mouse_down_column < 0) return;

    ex = ((XButtonEvent *)event)->x;

    x = 0;
    for(j = 0; j < t->left; j++) {
	x -= t->col_width[t->col_order[j]];
    }
    for(j = 0; j < t->ncols; j++) {
	if(ex > x && ex < x + t->col_width[t->col_order[j]]) break;

	x += (j == t->left) ? t->margin + t->col_width[t->col_order[j]] :
			t->col_width[t->col_order[j]];
    }
    dist = (j > h->mouse_down_column) ? ex - x :
				x + t->col_width[t->col_order[j]] - ex;
    out = False;
    if(ex < 0) {
	out = True;
	if(j == t->ncols) j = 0;
	else if(j == h->mouse_down_column && j > 0) j--;
    }
    else if(ex > w->core.width) {
	out = True;
	if(j == t->ncols) j = t->ncols-1;
	else if(j == h->mouse_down_column && j < t->ncols-1) j++;
    }

    if(j < t->ncols && j != h->mouse_down_column &&
	(out || dist > t->col_width[t->col_order[j]]/2))
    {
	int co;
	int *old_order = (int *)malloc(t->ncols*sizeof(int));
	memcpy(old_order, t->col_order, t->ncols*sizeof(int));
	h->moved_column = True;
	co = t->col_order[h->mouse_down_column];
	if(h->mouse_down_column > j) {
	    for(i = h->mouse_down_column; i > j; i--) {
		t->col_order[i] = t->col_order[i-1];
	    }
	}
	else {
	    for(i = h->mouse_down_column; i < j; i++) {
		t->col_order[i] = t->col_order[i+1];
	    }
	}
	t->col_order[j] = co;
	h->mouse_down_column = j;
	MmTableResetColumnLimits(h->table);

	if(j <= t->left) { /* scroll left */
	    if(t->left > 0) {
		MmTableSetBarValue(t->hbar, j > 0 ? j-1 : j);
    		_TCanvasRedraw(h->canvas);
		setImages(w, h->mouse_down_column);
		moveColumn(w, event);
    		_TCanvasRepaint(h->canvas);
		free(old_order);
		return;
	    }
	}
	else if(j >= t->right) { /* scroll right */
	    if(t->left < getBarMaximum(t->hbar)) {
		if(j < t->ncols-1) MmTableSetBarValue(t->hbar, t->left+1);
		else {
		    while(t->left < getBarMaximum(t->hbar) -
				MmTableGetBarVisibleAmount(t->hbar))
		    {
			t->left++;
			MmTableAdjustHScroll(h->table);
		    }
		    MmTableSetBarValue(t->hbar, t->left);
		}
    		_TCanvasRedraw(h->canvas);
		setImages(w, h->mouse_down_column);
		moveColumn(w, event);
    		_TCanvasRepaint(h->canvas);
		free(old_order);
		return;
	    }
	}
	MmTableAdjustHScroll(h->table);
	h->shift = 0;
	XCopyArea(XtDisplay(w), h->copy_image, tc->image, h->gc,
		0, 0, h->drag_w, h->drag_h, h->copy_x, 0);

	for(i = 0; i < t->ncols; i++) {
	    if(t->col_order[i] != old_order[i]) {
		clearColumn(w, i);
		_TCanvasDrawRows(h->canvas, t->top, t->bottom, i, i);
	    }
	}
	free(old_order);
	setImages(w, h->mouse_down_column);
	moveColumn(w, event);
	_TCanvasRepaint(h->canvas);
    }
    else {
	moveColumn(w, event);
	_TCanvasRepaint(h->canvas);
    }
}

static void
moveColumn(HCanvasWidget w, XEvent *event)
{
    int x, mar, ex;
    HCanvasPart *h = &w->hCanvas;
    TCanvasPart *tc = &h->canvas->tCanvas;
    MmTablePart *t = &h->table->mmTable;

    mar = (h->mouse_down_column == 0) ? 0 : t->margin;
    ex = ((XButtonEvent *)event)->x;
    x = ex - (t->col_beg[h->mouse_down_column] - t->col_beg[t->left]+mar+3);
    h->shift = x - h->mouse_down_x;
    if(h->mouse_down_column == 0 && h->shift < 0) h->shift = 0;
    else if(h->mouse_down_column == t->ncols-1 && h->shift > 0) h->shift = 0;

    _HCanvasRedisplay(w);

    x = t->col_beg[h->mouse_down_column] - t->col_beg[t->left] + mar;
    x += h->shift;
    XCopyArea(XtDisplay(w), h->copy_image, tc->image, h->gc,
		0, 0, h->drag_w, h->drag_h, h->copy_x, 0);
    XCopyArea(XtDisplay(w), tc->image, h->copy_image, h->gc,
		x, 0, h->drag_w, h->drag_h, 0, 0);
    h->copy_x = x;
    XCopyArea(XtDisplay(w), h->drag_image, tc->image, h->gc,
		0, 0, h->drag_w, h->drag_h, x, 0);
}

Size
HCanvasGetPreferredSize(HCanvasWidget w)
{
    int j;
    HCanvasPart *h = &w->hCanvas;
    MmTablePart *t = &h->table->mmTable;
    Size s;

    if(t->row_height == 0) {
        int direction;
        XCharStruct overall;
        XTextExtents(t->font, "M", 1, &direction, &t->max_ascent,
		&t->max_descent, &overall);
        t->row_height = t->max_ascent + t->max_descent + 4;
    }
    s.width = 0;
    for(j = 0; j < t->ncols; j++) s.width += t->col_width[t->col_order[j]];
    s.height = t->row_height + 2;
    return s;
}

static void
Destroy(Widget widget)
{
    HCanvasWidget w = (HCanvasWidget)widget;
    HCanvasPart *h = &w->hCanvas;

    if(h->gc != NULL) XFreeGC(XtDisplay(w), h->gc);

    if(h->image != (Pixmap)NULL) {
	XFreePixmap(XtDisplay(w), h->image);
    }
    if(h->drag_image != (Pixmap)NULL) {
	XFreePixmap(XtDisplay(w), h->drag_image);
    }
    if(h->copy_image != (Pixmap)NULL) {
	XFreePixmap(XtDisplay(w), h->copy_image);
    }
}


static int
getBarMaximum(Widget w)
{
    int max;
    Arg args[1];

    XtSetArg(args[0], XmNmaximum, &max);
    XtGetValues(w, args, 1);
    return max;
}
