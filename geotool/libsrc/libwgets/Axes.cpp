/** \file Axes.c
 *  \brief Defines widget Axes.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      Axes Widget -- widget for drawing axes. 
 *
 * SYNOPSIS
 *      #include "Axes.h"
 *      Widget
 *      AxesCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      Axes.h
 *
 * BUGS
 *
 * NOTES
 *      To Do:
 *
 * AUTHOR
 *      I. Henson -- July 1990
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/XmAll.h>

extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
}
#include "widget/AxesP.h"
#include "widget/AxesClass.h"

#define _MIDDLE_MOUSE_POPUP_

/*
#define NO_IMAGE
*/
#define XtRAxesInt	(char *)"AxesInt"


#define	offset(field)		XtOffset(AxesWidget, axes.field)
static XtResource	resources[] = 
{
        {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
                offset(fg), XtRString, (XtPointer)XtDefaultForeground},
	{XtNselectColor, XtCSelectColor, XtRPixel, sizeof(Pixel),
		offset(select_fg), XtRString, (XtPointer)"blue"},
        {XtNmagRectColor, XtCMagRectColor, XtRPixel, sizeof(Pixel),
                offset(mag_fg), XtRString, (XtPointer)"red"},
	{XtNaxesFont, XtCAxesFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(font), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*"},
	{XtNtitle, XtCTitle, XtRString, sizeof(String),
		offset (title), XtRString, (XtPointer)NULL},
	{XtNxLabel, XtCXLabel, XtRString, sizeof(String),
		offset (x_label), XtRString, (XtPointer)NULL},
	{XtNdisplayXLabel, XtCDisplayXLabel, XtRBoolean, sizeof(Boolean),
		offset(display_x_label), XtRString,(XtPointer)"True"},
	{XtNdisplayYLabel, XtCDisplayYLabel, XtRBoolean, sizeof(Boolean),
		offset(display_y_label), XtRString,(XtPointer)"True"},
	{XtNyLabel, XtCYLabel, XtRString, sizeof(String),
		offset (y_label), XtRString, (XtPointer)NULL},
	{XtNyLabelInt, XtCYLabelInt, XtRBoolean, sizeof(Boolean),
		offset(a.y_label_int), XtRString,(XtPointer)"False"},
	{XtNdrawYLabels, XtCDrawYLabels, XtRBoolean, sizeof(Boolean),
		offset(draw_y_labels), XtRString,(XtPointer)"True"},
	{XtNdisplayGrid, XtCDisplayGrid, XtRBoolean, sizeof(Boolean),
		offset(display_grid), XtRString, (XtPointer)"False"},
	{XtNlogX, XtCLogX, XtRBoolean, sizeof(Boolean),
		offset(a.log_x), XtRString,(XtPointer) "False"},
	{XtNlogY, XtCLogY, XtRBoolean, sizeof(Boolean),
		offset(a.log_y), XtRString,(XtPointer)"False"},
	{XtNminXLab, XtCMinXLab, XtRInt, sizeof(int),
		offset(a.min_xlab), XtRImmediate, (XtPointer)-1},
	{XtNmaxXLab, XtCMaxXLab, XtRInt, sizeof(int),
		offset(a.max_xlab), XtRImmediate, (XtPointer)-1},
	{XtNminYLab, XtCMinYLab, XtRInt, sizeof(int),
		offset(a.min_ylab), XtRImmediate, (XtPointer)-1},
	{XtNmaxYLab, XtCMaxYLab, XtRInt, sizeof(int),
		offset(a.max_ylab), XtRImmediate, (XtPointer)-1},
	{XtNminXSmall, XtCMinXSmall, XtRInt, sizeof(int),
		offset(a.min_xsmall), XtRImmediate, (XtPointer)-1},
	{XtNmaxXSmall, XtCMaxXSmall, XtRInt, sizeof(int),
		offset(a.max_xsmall), XtRImmediate, (XtPointer)-1},
	{XtNminYSmall, XtCMinYSmall, XtRInt, sizeof(int),
		offset(a.min_ysmall), XtRImmediate, (XtPointer)-1},
	{XtNmaxYSmall, XtCMaxYSmall, XtRInt, sizeof(int),
		offset(a.max_ysmall), XtRImmediate, (XtPointer)-1},
	{XtNxAxis, XtCXAxis, XtRAxesInt, sizeof(int),
		offset(x_axis), XtRString, (XtPointer)"BOTTOM"},
	{XtNyAxis, XtCYAxis, XtRAxesInt, sizeof(int),
		offset(y_axis), XtRString, (XtPointer)"LEFT"},
	{XtNleftMargin, XtCLeftMargin, XtRInt, sizeof(int),
		offset(left_margin), XtRImmediate, (XtPointer)0},
	{XtNrightMargin, XtCRightMargin, XtRInt, sizeof(int),
		offset(right_margin), XtRImmediate, (XtPointer)0},
	{XtNtopMargin, XtCTopMargin, XtRInt, sizeof(int),
		offset(top_margin), XtRImmediate, (XtPointer)0},
	{XtNbottomMargin, XtCBottomMargin, XtRInt, sizeof(int),
		offset(bottom_margin), XtRImmediate, (XtPointer)0},
	{XtNverticalScrollPosition, XtCVerticalScrollPosition, XtRAxesInt,
		sizeof(int), offset(vertical_scroll_position), XtRString,
		(XtPointer)"LEFT"},
	{XtNtimeScale, XtCTimeScale, XtRAxesInt, sizeof(int),
		offset(time_scale), XtRString, (XtPointer)"TIME_SCALE_SECONDS"},
	{XtNdisplayAxesLabels, XtCDisplayAxesLabels, XtRAxesInt, sizeof(int),
		offset(display_axes_labels), XtRString, (XtPointer)"AXES_XY"},
	{XtNdisplayAxes, XtCDisplayAxes, XtRBoolean, sizeof(Boolean),
		offset(display_axes), XtRString,(XtPointer)"True"},
	{XtNusePixmap, XtCUsePixmap, XtRBoolean, sizeof(Boolean),
		offset(use_pixmap), XtRString, (XtPointer)"True"},
	{XtNuniformScale, XtCUniformScale, XtRBoolean, sizeof(Boolean),
		offset(uniform_scale), XtRString,(XtPointer) "False"},
	{XtNautoYScale, XtCAutoYScale, XtRBoolean, sizeof(Boolean),
		offset(auto_y_scale), XtRString,(XtPointer) "True"},
	{XtNhorizontalScroll, XtCHorizontalScroll, XtRBoolean, sizeof(Boolean),
		offset(horizontal_scroll), XtRString, (XtPointer)"True"},
	{XtNverticalScroll, XtCVerticalScroll, XtRBoolean, sizeof(Boolean),
		offset(vertical_scroll), XtRString, (XtPointer)"True"},
	{XtNtickmarksInside, XtCTickmarksInside, XtRBoolean, sizeof(Boolean),
		offset(tickmarks_inside), XtRString,(XtPointer) "True"},
	{XtNclearOnScroll, XtCClearOnScroll, XtRBoolean, sizeof(Boolean),
		offset(clear_on_scroll), XtRString, (XtPointer)"True"},
	{XtNsaveSpace, XtCSaveSpace, XtRBoolean, sizeof(Boolean),
		offset(save_space), XtRString, (XtPointer)"False"},
	{XtNcursorLabelAlways, XtCCursorLabelAlways, XtRBoolean,sizeof(Boolean),
		offset(cursor_label_always), XtRString,(XtPointer)"False"},
	{XtNextraYTickmarks, XtCExtraYTickmarks, XtRBoolean, sizeof(Boolean),
		offset(extra_y_tics), XtRString, (XtPointer)"True"},
	{XtNextraXTickmarks, XtCExtraXTickmarks, XtRBoolean, sizeof(Boolean),
		offset(extra_x_tics), XtRString, (XtPointer)"True"},
	{XtNzoomControls, XtCZoomControls, XtRBoolean, sizeof(Boolean),
		offset(zoom_controls), XtRString, (XtPointer)"True"},
	{XtNmagnifyOnly, XtCMagnifyOnly, XtRBoolean, sizeof(Boolean),
		offset(magnify_only), XtRString,(XtPointer) "False"},
	{XtNallowCursorChange, XtCAllowCursorChange, XtRBoolean,sizeof(Boolean),
		offset(allow_cursor_change), XtRString,(XtPointer) "True"},
	{XtNlabelMode, XtCLabelMode, XtRBoolean, sizeof(Boolean),
		offset(label_mode), XtRString, (XtPointer)"False"},
	{XtNscrollbarAsNeeded, XtCScrollbarAsNeeded, XtRBoolean,sizeof(Boolean),
		offset(scrollbar_as_needed), XtRString,(XtPointer)"False"},
	{XtNdoubleLineAnchor, XtCDoubleLineAnchor, XtRBoolean, sizeof(Boolean),
		offset(double_line_anchor), XtRString,(XtPointer) "False"},
	{XtNredisplay, XtCRedisplay, XtRBoolean, sizeof(Boolean),
		offset(redisplay), XtRString, (XtPointer)"False"},
	{XtNzoomCallback, XtCZoomCallback, XtRCallback,
		sizeof(XtCallbackList), offset(zoom_callbacks), XtRCallback,
		(XtPointer)NULL},
	{XtNcrosshairCallback, XtCCrosshairCallback, XtRCallback,
		sizeof(XtCallbackList), offset(crosshair_callbacks),XtRCallback,
		(XtPointer)NULL},
	{XtNcrosshairDragCallback, XtCCrosshairDragCallback,XtRCallback,
		sizeof(XtCallbackList), offset(crosshair_drag_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNlineCallback, XtCLineCallback, XtRCallback, sizeof(XtCallbackList),
		offset(line_callbacks), XtRCallback, (XtPointer)NULL},
	{XtNlineDragCallback, XtCLineDragCallback, XtRCallback,
		sizeof(XtCallbackList), offset(line_drag_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNdoubleLineCallback, XtCDoubleLineCallback, XtRCallback,
		sizeof(XtCallbackList), offset(double_line_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNdoubleLineDragCallback, XtCDoubleLineDragCallback, XtRCallback,
		sizeof(XtCallbackList), offset(double_line_drag_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNdoubleLineScaleCallback, XtCDoubleLineScaleCallback, XtRCallback,
		sizeof(XtCallbackList), offset(double_line_scale_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNphaseLineCallback, XtCPhaseLineCallback, XtRCallback,
		sizeof(XtCallbackList), offset(phase_line_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNphaseLineDragCallback, XtCPhaseLineDragCallback, XtRCallback,
		sizeof(XtCallbackList), offset(phase_line_drag_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNlimitsCallback, XtCLimitsCallback, XtRCallback,
		sizeof(XtCallbackList), offset(limits_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNhorizontalScrollCallback, XtCHorizontalScrollCallback, XtRCallback,
		sizeof(XtCallbackList), offset(hor_scroll_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNmagnifyCallback, XtCMagnifyCallback, XtRCallback,
		sizeof(XtCallbackList), offset(magnify_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNwarningCallback, XtCWarningCallback, XtRCallback,
		sizeof(XtCallbackList), offset(warning_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNkeyPressCallback, XtCKeyPressCallback, XtRCallback,
		sizeof(XtCallbackList), offset(keypress_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNresizeCallback, XtCResizeCallback, XtRCallback,
		sizeof(XtCallbackList), offset(resize_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNinfoWidget, XtCInfoWidget, XtRWidget, sizeof(Widget),
		offset(cursor_info), XtRImmediate,(XtPointer) NULL},
	{XtNinfoWidget2, XtCInfoWidget2, XtRWidget, sizeof(Widget),
		offset(cursor_info2), XtRImmediate,(XtPointer) NULL},
};
#undef offset

/* Private functions */

static void CvtStringToAxesInt(XrmValuePtr *args, Cardinal *num_args,
				XrmValuePtr fromVal, XrmValuePtr toVal);
static void ClassInitialize(void);
static void Initialize(Widget req, Widget nou);
static void Realize(Widget w, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static Boolean SetValues(AxesWidget cur, AxesWidget req, AxesWidget nou);
static void Redisplay(Widget w, XEvent *event, Region region);
static void Resize(Widget w);
static void Redraw(AxesWidget w);
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
		XtWidgetGeometry *reply);
static void ChangeManaged(Widget w);
static void Destroy(Widget w);

static Boolean AxesFormMagRect(AxesWidget w, int *mag_x1, int *mag_x2,
			int *mag_y1, int *mag_y2);
static void DrawTics(AxesWidget w);
static void DoLayout(AxesWidget w);
static void flip_xy(AxesWidget w);
static void AxesMagApply(AxesWidget w, int mag_x1, int mag_x2, int mag_y1,
		int mag_y2);
static void DoMagFrom(AxesWidget w);
static void SetLimits(AxesWidget w, Boolean init);
static int AddCursor(AxesWidget w, int type, const char *phase,
		Boolean do_callback);
static void AxesPosition(AxesWidget w, int ic, Boolean do_callback);
static void DeleteCursor(AxesWidget w, int ic, Boolean manual);
static void ConstrainCursor(AxesWidget w, int ic);
static void CursorSegs(AxesWidget w, DrawStruct *d, AxesCursor *c);
static void CursorLabels(AxesWidget w, int ic);
static void ClearCursor(AxesWidget w, int m);
static void DoCursorCallback(AxesWidget widget, AxesCursor *c, int reason);
static void LastMark(AxesWidget w);
static void DrawTitles(AxesWidget w);
static void HorizontalScroll(Widget scroll, XtPointer client_data,
		XtPointer data);
static void VerticalScroll(Widget scroll, XtPointer client_data,
		XtPointer data);
static double time_factor(AxesWidget w, Boolean setLabel);
static void new_x_label(AxesWidget cur, AxesWidget nou, const char *label);
static void DrawCursor(AxesWidget w, int ic);
static double _AxesMagDistance(AxesWidget w, int cursor_x, int cursor_y);
static Boolean dragScroll(AxesWidget w, XEvent *event);
static void AddDoubleLineWithLabel(AxesWidget w, char label,
		double x1, double x2, Boolean draw_labels);
#ifdef _MIDDLE_MOUSE_POPUP_
static void AxesDeleteCB(Widget widget, XtPointer client, XtPointer data);
static void AxesWidthCB(Widget widget, XtPointer client, XtPointer data);
static void doCursorPopup(AxesWidget w, int x, int y, XEvent *event);
static void WidthApplyCB(Widget widget, XtPointer client, XtPointer data);
static void WidthCancelCB(Widget widget, XtPointer client, XtPointer data);
#endif
static Boolean SetCursorMode(AxesWidget w, int x, int y);
static void drawYLabels(AxesWidget w, AxesPart *ax);
static void drawXLabels(AxesWidget w, AxesPart *ax);
static void drawOneYLabel(AxesWidget w, AxesPart *ax, int i);
static void drawOneXLabel(AxesWidget w, AxesPart *ax, int i);
static Boolean haveCrosshair(AxesPart *ax);
static void _AxesHorizontalShift(AxesWidget w, double shift);
extern "C" {
static void fontSize(void *font, char *lab, int *width, int *height,
		int *ascent);
}
static int AxesFormOverlayRects(AxesWidget w, int pos, XRectangle *recs);
static Boolean AxesFormOverlay(AxesWidget w, int *o_x1, int *o_x2, int *o_y1,
                int *o_y2);
static void DrawGrid(AxesWidget w);
static void positionHorScroll(AxesPart *ax, Widget hor_scroll, int left,
		int right);

static char	defTrans[] =
"~Shift ~Ctrl<Btn2Down>:Zoom() \n\
  <Btn2Up>:		Zoom() \n\
  <Btn2Motion>:		Zoom() \n\
 ~Shift ~Ctrl<Btn3Down>:Zoom(horizontal) \n\
  <Btn3Up>:		Zoom(horizontal) \n\
  <Btn3Motion>:		Zoom(horizontal) \n\
 ~Ctrl Shift<Btn2Down>:	ZoomBack() \n\
 ~Shift Ctrl<Btn2Down>:	Magnify() \n\
 ~Shift ~Ctrl<Btn1Down>:Move() \n\
  <Btn1Motion>:		Move() \n\
  <Btn1Up>:		Move() \n\
  Ctrl<Btn1Down>:	Scale() \n\
  Ctrl<Btn1Up>:		Scale() \n\
  Ctrl<Btn1Motion>:	Scale() \n\
  ~Shift<Key>c:		AddCrosshair() \n\
  ~Shift<Key>l:		AddDoubleLine() \n\
  Shift<Key>l:		AddLine() \n\
  ~Shift<Key>d:		DeleteCursor(one) \n\
  Shift<Key>d:		DeleteCursor(all) \n\
  ~Shift<Key>h:		ZoomHorizontal() \n\
  ~Shift<Key>v:		ZoomVertical() \n\
  ~Shift<Key>z:		ZoomPercentage() \n\
  Shift<Key>h:		UnzoomHorizontal() \n\
  Shift<Key>v:		UnzoomVertical() \n\
  Shift<Key>z:		UnzoomPercentage() \n\
  ~Shift<Key>f:		Page(vertical down) \n\
  ~Shift<Key>b:		Page(vertical up) \n\
  ~Shift<Key>t:		Page(horizontal down) \n\
  ~Shift<Key>r:		Page(horizontal up) \n\
  Shift<Key>f:		Page(vertical DOWN) \n\
  Shift<Key>b:		Page(vertical UP) \n\
  Shift<Key>t:		Page(horizontal DOWN) \n\
  Shift<Key>r:		Page(horizontal UP) \n\
  <KeyPress>:		KeyCommand() \n\
  <Motion>:		Motion() \n\
  <BtnDown>:		BtnDown()\n\
  <EnterWindow>:	Motion()\n\
  <FocusIn>:		Motion()";

static XtActionsRec	actionsList[] =
{
	{(char *)"Zoom",		(XtActionProc)_AxesZoom},
	{(char *)"ZoomBack",		(XtActionProc)_AxesZoomBack},
	{(char *)"Magnify",		(XtActionProc)_AxesMagnify},
	{(char *)"Move",		(XtActionProc)_AxesMove},
	{(char *)"Scale",		(XtActionProc)_AxesScale},
	{(char *)"AddCrosshair",	(XtActionProc)_AxesAddCrosshair},
	{(char *)"AddLine",		(XtActionProc)_AxesAddLine},
	{(char *)"AddDoubleLine",	(XtActionProc)_AxesAddDoubleLine},
	{(char *)"DeleteCursor",	(XtActionProc)_AxesDeleteCursor},
	{(char *)"UnzoomPercentage",	(XtActionProc)_AxesUnzoomPercentage},
	{(char *)"UnzoomHorizontal",	(XtActionProc)_AxesUnzoomHorizontal},
	{(char *)"UnzoomVertical",	(XtActionProc)_AxesUnzoomVertical},
	{(char *)"ZoomPercentage",	(XtActionProc)_AxesZoomPercentage},
	{(char *)"ZoomHorizontal",	(XtActionProc)_AxesZoomHorizontal},
	{(char *)"ZoomVertical",	(XtActionProc)_AxesZoomVertical},
	{(char *)"Page",		(XtActionProc)AxesPage},
	{(char *)"KeyCommand",		(XtActionProc)_AxesKeyCommand},
	{(char *)"BtnDown",		(XtActionProc)_AxesBtnDown},
	{(char *)"Motion",		(XtActionProc)_AxesMotion},
};


AxesClassRec	axesClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&compositeClassRec),	/* superclass */
	(char *)"Axes",				/* class_name */
	sizeof(AxesRec),		/* widget_size */
	ClassInitialize,		/* class_initialize */
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
	(XtSetValuesFunc)SetValues,		/* set_values */
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
	{	/* composite_class fields */
	GeometryManager,		/* geometry_manager */
	ChangeManaged,			/* change_managed */
	XtInheritInsertChild,		/* insert_child */
	XtInheritDeleteChild,		/* delete_child */
	NULL				/* extension */
	},
	{	/* AxesClass fields */
		0,			/* empty */
	},
};

WidgetClass axesWidgetClass = (WidgetClass)&axesClassRec;

#define MARGIN 4 

static const char *double_line_label[MAX_CURSORS] =
{
	"a ", "b ", "c ", "d ", "e ", "f ", "g ", "h ", "i ", "j ",
};
static const char *line_label[MAX_CURSORS] =
{
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
};


static void
CvtStringToAxesInt(
	XrmValuePtr	*args,
	Cardinal	*num_args,
	XrmValuePtr	fromVal,
	XrmValuePtr	toVal)
{
	char		*s = fromVal->addr;	/* string to convert */
	static int	val;			/* returned value */
 
	if(s == NULL)					val = 0;
	else if(!strcasecmp(s, "BOTTOM"))		val = BOTTOM;
	else if(!strcasecmp(s, "TOP"))			val = TOP;
	else if(!strcasecmp(s, "LEFT"))			val = LEFT;
	else if(!strcasecmp(s, "RIGHT"))		val = RIGHT;
	else if(!strcasecmp(s, "AXES_X"))		val = AXES_X;
	else if(!strcasecmp(s, "AXES_Y"))		val = AXES_Y;
	else if(!strcasecmp(s, "AXES_XY"))		val = AXES_XY;
	else if(!strcasecmp(s, "AXES_NONE"))		val = AXES_NONE;
	else if(!strcasecmp(s, "TIME_SCALE_SECONDS"))val = TIME_SCALE_SECONDS;
	else if(!strcasecmp(s, "TIME_SCALE_VARIABLE"))val = TIME_SCALE_VARIABLE;
	else if(!strcasecmp(s, "TIME_SCALE_HMS"))	val = TIME_SCALE_HMS;
	else val = 0;

	toVal->size = sizeof(val);
	toVal->addr = (char*)(XtPointer)&val;
}

static void
ClassInitialize(void)
{
	XtAddConverter (XtRString, XtRAxesInt, (XtConverter)CvtStringToAxesInt,
		(XtConvertArgList) NULL, (Cardinal) 0);
}

static void
fontSize(void *font, char *lab, int *width, int *height, int *ascent)
{
	if(font == NULL) {
	    *width = 0;
	    *height = 0;
	    *ascent = 0;
	}
	else {
	    XFontStruct *fnt = (XFontStruct *)font;
	    XCharStruct overall;
	    int asc, descent, direction;

	    XTextExtents(fnt, lab, (int)strlen(lab), &direction, &asc,
			&descent, &overall);
	    *width = overall.width;
	    *height = overall.ascent + overall.descent;
	    *ascent = overall.ascent;
	}
}

static void
Initialize(Widget w_req, Widget w_new)
{
	AxesWidget req = (AxesWidget)w_req;
	AxesWidget nou = (AxesWidget)w_new;
	AxesPart *ax = (nou != NULL) ? &nou->axes : NULL;
	AxesCursor axes_cursor_init = AXES_CROSSHAIR_INIT;
	DrawStruct draw_struct_init = DRAW_STRUCT_INIT;
	AxesParm axes_parm_init = AXES_PARM_INIT;
	Boolean y_label_int;
	Arg args[20];
	Boolean log_x, log_y;
	int i, n, min_xlab, max_xlab, min_ylab, max_ylab;
	int min_xsmall, max_xsmall, min_ysmall, max_ysmall;
	static XtCallbackRec hor_callbacks[] = {
		{(XtCallbackProc)HorizontalScroll, (XtPointer)NULL},
		{(XtCallbackProc)NULL, (XtPointer) NULL}
	};
	static XtCallbackRec vert_callbacks[] = {
		{(XtCallbackProc)VerticalScroll, (XtPointer) NULL},
		{(XtCallbackProc)NULL, (XtPointer) NULL}
	};
	static char scrollbarTranslations[] = "#override\n\
		<Btn1Down>:		Select()\n\
		<Btn1Up>:		Release()\n\
		<Btn1Motion>:		Moved()\n\
		Button1<PtrMoved>:	Moved()";
	XtTranslations translations;

	if(req->core.width == 0) {
	    nou->core.width = (Dimension)(XWidthOfScreen(XtScreen(req))/2.5);
 	}
	if(req->core.height == 0) {
	    nou->core.height = XHeightOfScreen(XtScreen(req))/2;
	}

	/* no data displayed yet.
	 */
	ax->x_min = 0.;
	ax->x_max = 1.;
	ax->y_min = 0.;
	ax->y_max = 1.;

	ax->zoom_horizontal = .2;
	ax->zoom_vertical = .2;
	ax->double_line_width = 0.;

	ax->err_msg = strdup("");

	y_label_int = ax->a.y_label_int;
	log_x = ax->a.log_x;
	log_y = ax->a.log_y;
	min_xlab = ax->a.min_xlab;
	max_xlab = ax->a.max_xlab;
	min_ylab = ax->a.min_ylab;
	max_ylab = ax->a.max_ylab;
	min_xsmall = ax->a.min_xsmall;
	max_xsmall = ax->a.max_xsmall;
	min_ysmall = ax->a.min_ysmall;
	max_ysmall = ax->a.max_ysmall;

	memcpy(&ax->a, &axes_parm_init, sizeof(AxesParm));

	ax->a.fontMethod = fontSize;
	ax->a.y_label_int = y_label_int;
	ax->a.log_x = log_x;
	ax->a.log_y = log_y;

	if(min_xlab > -1)  ax->a.min_xlab = min_xlab;
	if(max_xlab > -1)  ax->a.max_xlab = max_xlab;
	if(min_ylab > -1)  ax->a.min_ylab = min_ylab;
	if(max_ylab > -1)  ax->a.max_ylab = max_ylab;
	if(min_xsmall > -1)  ax->a.min_xsmall = min_xsmall;
	if(max_xsmall > -1)  ax->a.max_xsmall = max_xsmall;
	if(min_ysmall > -1)  ax->a.min_ysmall = min_ysmall;
	if(max_ysmall > -1)  ax->a.max_ysmall = max_ysmall;

	ax->title = (req->axes.title != NULL) ?
			strdup(req->axes.title) : strdup("");
	ax->a.top_title = (!ax->save_space) ? ax->title : strdup("");
	ax->a.x_axis_label = (req->axes.x_label != NULL &&
		req->axes.display_x_label) ? strdup(req->axes.x_label) :
		strdup("");
	ax->x_label = ax->a.x_axis_label;
	ax->a.y_axis_label = (req->axes.y_label != NULL &&
		req->axes.display_y_label) ? strdup(req->axes.y_label) :
		strdup("");
	ax->y_label = ax->a.y_axis_label;
	ax->y_label_image = (XImage *)NULL;

	if(req->axes.time_scale == TIME_SCALE_HMS) {
	    if(ax->x_max - ax->x_min <= 40*3600) {
		new_x_label(nou, nou, "Time (hr:min:sec)");
	    }
	    else {
		new_x_label(nou, nou, "Time (yr/mo/day)");
	    }
	}

	ax->a.uniform_scale = ax->uniform_scale;
	/*
	 * fg, mag_fg, select_fg and font are resources.
	 */
	/*
	 * x_min, x_max, y_min, y_max are resources.
	 */
	SetLimits(nou, True);

	ax->x_spacing = 0.;
	ax->y_spacing = 0.;
	ax->zoom_x1 = 0;
	ax->zoom_y1 = 0;
	ax->zoom_x2 = 0;
	ax->zoom_y2 = 0;
	ax->mag_scaled_x1 = 0.;
	ax->mag_scaled_x2 = 0.;
	ax->mag_scaled_y1 = 0.;
	ax->mag_scaled_y2 = 0.;
	ax->last_x_mark = 0;
	ax->last_y_mark = 0;
	ax->label_xpos = -1;
	ax->label_ypos = -1;
	ax->wait_state = 0;
	ax->rt_margin = 0;
	ax->bm_margin = 0;

	for(i = 0; i < MAX_ZOOM; i++) {
	    ax->x1[i] = ax->x_min;
	    ax->x2[i] = ax->x_max;
	    ax->y1[i] = ax->y_min;
	    ax->y2[i] = ax->y_max;
	}

	/*
	 * x_axis and y_axis are resources.
	 */
	ax->cursor_mode = AXES_ZOOM;
	ax->last_cursor_x = 0;
	ax->last_cursor_y = 0;
	ax->overlay_rect = False;
	ax->n_ovy_grps = 0;
	for(i = 0; i < MAX_RECTS; i++)
	{
	    ax->ovy_grp[i].ovr_scaled_x1 = NULL;
	    ax->ovy_grp[i].ovr_scaled_x2 = NULL;
	    ax->ovy_grp[i].ovr_scaled_y1 = NULL;
	    ax->ovy_grp[i].ovr_scaled_y2 = NULL;
	    ax->ovy_grp[i].display = NULL;
	    ax->ovy_grp[i].nrecs = 0;
	    ax->ovy_grp[i].pixel = 0;
	    ax->ovy_grp[i].width = 0;
	}
	ax->undraw = False;
	ax->unmark = False;
	ax->zooming = False;
	ax->magnify = False;
	ax->magnify_rect = False;
	ax->scaling_double_line = 0;
	ax->moving_cursor = False;
	ax->num_cursors = 0;
	ax->num_crosshairs = 0;
	ax->num_lines = 0;
	ax->num_double_lines = 0;
	ax->num_phase_lines = 0;
	ax->crosshairs	= (AxesCursorCallbackStruct *)NULL;
	ax->lines	= (AxesCursorCallbackStruct *)NULL;
	ax->double_lines= (AxesCursorCallbackStruct *)NULL;
	ax->phase_lines	= (AxesCursorCallbackStruct *)NULL;
	ax->zoom = 0;
	ax->zoom_min = 0;
	ax->zoom_max = 0;
	ax->min_zoom_level = 0;
	ax->m = -1;
	ax->title_x = 0;
	ax->title_y = 0;
	ax->title_width = 0;
	ax->redisplay_pending = False;
	ax->expose = True;
	ax->use_screen = True;
	ax->setvalues_redisplay = False;
	ax->clear_on_redisplay = True;
	ax->manual_scroll = False;
	ax->check_expose_region = True;
	strncpy(ax->cursor_type, "default", 20);

	for(i = 0; i < MAX_CURSORS; i++) {
	    memcpy(&ax->cursor[i], &axes_cursor_init, sizeof(AxesCursor));
	}
	memcpy(&ax->d, &draw_struct_init, sizeof(DrawStruct));

	ax->pixmap = (Pixmap)NULL;

	ax->hor_scroll = (Widget)NULL;
	ax->vert_scroll = (Widget)NULL;
	ax->mag_to = (Widget)NULL;
	ax->mag_from = (Widget)NULL;

	ax->redraw_data_func = NULL;
	ax->resize_func = NULL;
	ax->cursor_func = NULL;
	ax->hard_copy_func = NULL;
	ax->select_data_func = NULL;
	ax->transform = NULL;
	ax->width_form = NULL;
	ax->width_text = NULL;
	ax->skip_limits_callback = False;

	ax->delx_min = 0.;
	ax->dely_min = 0.;

	ax->anchor_x = -1;
	ax->anchor_y = -1;
	ax->last_x = -1;
	ax->last_y = -1;
	ax->no_motion = False;
	ax->hor_only = False;

	ax->num_time_marks = 0;
	ax->time_marks = NULL;

	hor_callbacks[0].closure = (XtPointer)w_new;
	vert_callbacks[0].closure = (XtPointer)w_new;

	n = 0;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNincrement, 1); n++;
	XtSetArg(args[n], XmNincrementCallback, hor_callbacks); n++;
	XtSetArg(args[n], XmNdecrementCallback, hor_callbacks); n++;
	XtSetArg(args[n], XmNpageIncrementCallback, hor_callbacks); n++;
	XtSetArg(args[n], XmNpageDecrementCallback, hor_callbacks); n++;
	XtSetArg(args[n], XmNdragCallback, hor_callbacks); n++;
	XtSetArg(args[n], XmNvalueChangedCallback, hor_callbacks); n++;

	ax->hor_scroll = XmCreateScrollBar(w_new, (char *)"scrollHoriz",args,n);

	translations = XtParseTranslationTable(scrollbarTranslations);
	XtOverrideTranslations(ax->hor_scroll, translations);

	n = 0;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
	XtSetArg(args[n], XmNincrement, 1); n++;
	XtSetArg(args[n], XmNincrementCallback, vert_callbacks); n++;
	XtSetArg(args[n], XmNdecrementCallback, vert_callbacks); n++;
	XtSetArg(args[n], XmNpageIncrementCallback, vert_callbacks); n++;
	XtSetArg(args[n], XmNpageDecrementCallback, vert_callbacks); n++;
	XtSetArg(args[n], XmNdragCallback, vert_callbacks); n++;
	XtSetArg(args[n], XmNvalueChangedCallback, vert_callbacks); n++;

	ax->vert_scroll =  XmCreateScrollBar(w_new,(char *)"scrollVert",args,n);
	XtOverrideTranslations(ax->vert_scroll, translations);

	ax->outside_hor_scroll = NULL;
}

DrawStruct * AxesClass::drawStruct()
{
	return(&aw->axes.d);
}

void
AxesSetRedrawFunction(AxesWidget w, AxesRedrawFunction redraw_data_func)
{
	AxesPart *ax = &w->axes;
	ax->redraw_data_func = redraw_data_func;
}
void
AxesSetResizeFunction(AxesWidget w, AxesResizeFunction resize_func)
{
	AxesPart *ax = &w->axes;
	ax->resize_func = resize_func;
}
void
AxesSetCursorFunction(AxesWidget w, AxesCursorFunction cursor_func)
{
	AxesPart *ax = &w->axes;
	ax->cursor_func = cursor_func;
}
void
AxesSetHardCopyFunction(AxesWidget w, AxesHardCopyFunction hard_copy_func)
{
	AxesPart *ax = &w->axes;
	ax->hard_copy_func = hard_copy_func;
}
void
AxesSetSelectDataFunction(AxesWidget w, AxesSelectDataFunction select_data_func)
{
	AxesPart *ax = &w->axes;
	ax->select_data_func = select_data_func;
}
void
AxesSetTransformFunction(AxesWidget w, AxesTransformFunction transform)
{
	AxesPart *ax = &w->axes;
	ax->transform = transform;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
	AxesWidget w = (AxesWidget)widget;
	AxesPart *ax = &w->axes;
	Pixel	black, white;
	Widget	XmCreateScrollBar();
	Widget	children[10];
	int	num_children;
	XWindowAttributes window_attributes;
	int ascent, descent, direction;
	XCharStruct overall;

	(*((axesWidgetClass->core_class.superclass)->core_class.realize))
		((Widget)w,  valueMask, attrs);

	ax->axesGC	= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	ax->xorGC	= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	ax->selectGC	= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	ax->invertGC	= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	ax->invert_clipGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	ax->mag_invertGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	ax->eraseGC	= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	/*
	 * check that no foreground colors are same as background
	 */
	white = StringToPixel(widget, "White");
	black = StringToPixel(widget, "Black");
	if(ax->fg == w->core.background_pixel) {
	    ax->fg = (w->core.background_pixel != black) ? black : white;
	}
	if(ax->select_fg == w->core.background_pixel) {
	    ax->select_fg = (w->core.background_pixel != black) ? black : white;
	}
	if(ax->mag_fg == w->core.background_pixel) {
	    ax->mag_fg = (w->core.background_pixel != black) ? black : white;
	}

        XSetForeground(XtDisplay(w), ax->axesGC, ax->fg);
        XSetBackground(XtDisplay(w), ax->axesGC, w->core.background_pixel);
	XSetFont(XtDisplay(w), ax->axesGC, ax->font->fid);
	XSetFont(XtDisplay(w), ax->selectGC, ax->font->fid);

        XSetForeground(XtDisplay(w), ax->xorGC, ax->fg);
        XSetBackground(XtDisplay(w), ax->xorGC, w->core.background_pixel);
	XSetFont(XtDisplay(w), ax->xorGC, ax->font->fid);
	XSetFunction(XtDisplay(w), ax->xorGC, GXxor);
	XSetGraphicsExposures(XtDisplay(w), ax->xorGC, False);

	XSetFunction(XtDisplay(w), ax->invertGC, GXinvert);
	XSetPlaneMask(XtDisplay(w), ax->invertGC, ax->fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), ax->invertGC, ax->fg);
        XSetBackground(XtDisplay(w), ax->invertGC,w->core.background_pixel);
	XSetFont(XtDisplay(w), ax->invertGC, ax->font->fid);

	XSetFunction(XtDisplay(w), ax->selectGC, GXinvert);
	XSetPlaneMask(XtDisplay(w), ax->selectGC,
		ax->select_fg ^ w->core.background_pixel);
	XSetForeground(XtDisplay(w), ax->selectGC, ax->select_fg);
	XSetBackground(XtDisplay(w), ax->selectGC,
		w->core.background_pixel);

	XSetFunction(XtDisplay(w), ax->invert_clipGC, GXinvert);
	XSetPlaneMask(XtDisplay(w), ax->invert_clipGC, ax->fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), ax->invert_clipGC, ax->fg);
        XSetBackground(XtDisplay(w), ax->invert_clipGC,
		w->core.background_pixel);
	XSetFont(XtDisplay(w), ax->invert_clipGC, ax->font->fid);

	XSetFunction(XtDisplay(w), ax->mag_invertGC, GXinvert);
	XSetPlaneMask(XtDisplay(w), ax->mag_invertGC, ax->mag_fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), ax->mag_invertGC, ax->mag_fg);
        XSetBackground(XtDisplay(w), ax->mag_invertGC,
		w->core.background_pixel);

	XSetForeground(XtDisplay(w), ax->eraseGC, w->core.background_pixel);
	XSetBackground(XtDisplay(w), ax->eraseGC, w->core.background_pixel);

	ax->a.font = (void *)ax->font;

#ifndef NO_IMAGE
	if((int)strlen(ax->a.y_axis_label) > 0)
	{
	    ax->y_label_image = GetStrImage(XtDisplay(w), XtWindow(w),
			ax->axesGC, ax->font, ax->eraseGC, ax->a.y_axis_label,
			(int)strlen(ax->a.y_axis_label));
	}
#endif
	XTextExtents(ax->font, "01234", 5, &direction,
			&ascent, &descent, &overall);
	ax->a.axis_font_height = ascent;

	XClearWindow(XtDisplay(w), XtWindow(w));

	ax->last_width = w->core.width;
	ax->last_height = w->core.height;

	num_children = 0;

	if(ax->horizontal_scroll) {
	    children[num_children++] = ax->hor_scroll;
	}

	if(ax->vertical_scroll) {
	    children[num_children++] = ax->vert_scroll;
	}

	XGetWindowAttributes(XtDisplay(w), XtWindow(w), &window_attributes);
	ax->depth = window_attributes.depth;
	if(ax->use_pixmap)
	{
	    /* important to create pixmap with (width+1,height+1) for calls
	     * to XFillRectangle(... width, height)
	     * Otherwise get bad problems with some X-servers
	     */
	    ax->pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
				w->core.width+1, w->core.height+1, ax->depth);
	    if(ax->pixmap == (Pixmap)NULL)
	    {
		fprintf(stderr, "%s: XCreatePixmap failed.\n", w->core.name);
		ax->use_pixmap = False;
	    }
	    else {
		_AxesClearArea(w, 0, 0, w->core.width, w->core.height);
	    }
	}

	if(!ax->scrollbar_as_needed) {
	    XtManageChildren(children, num_children);
	}
	ax->skip_limits_callback = True;
	_AxesRedraw(w);
	ax->skip_limits_callback = False;
}

Widget AxesClass::getHorizontalScroll()
{
     return aw->axes.hor_scroll;
}

void AxesClass::setHorizontalScroll(Widget outside_hor_scroll)
{
    XtAddCallback(outside_hor_scroll, XmNvalueChangedCallback,
		HorizontalScroll, (XtPointer)aw);
    XtAddCallback(outside_hor_scroll, XmNdragCallback,
		HorizontalScroll, (XtPointer)aw);
    XtAddCallback(outside_hor_scroll, XmNincrementCallback,
		HorizontalScroll, (XtPointer)aw);
    XtAddCallback(outside_hor_scroll, XmNdecrementCallback,
		HorizontalScroll, (XtPointer)aw);
    XtAddCallback(outside_hor_scroll, XmNpageIncrementCallback,
		HorizontalScroll, (XtPointer)aw);
    XtAddCallback(outside_hor_scroll, XmNpageDecrementCallback,
		HorizontalScroll, (XtPointer)aw);

    aw->axes.outside_hor_scroll = outside_hor_scroll;
}

static Boolean
SetValues(AxesWidget cur, AxesWidget req, AxesWidget nou)
{
	AxesPart *cur_ax = &cur->axes;
	AxesPart *new_ax = &nou->axes;
	Boolean redisplay = False, redraw = False, set_limits = False;

        if(cur_ax->font != req->axes.font)
	{
	    if(new_ax->font == NULL) {
		new_ax->font = cur_ax->font;
	    }
	    else
	    {
		XSetFont(XtDisplay(nou), new_ax->axesGC, new_ax->font->fid);
		XSetFont(XtDisplay(nou), new_ax->selectGC, new_ax->font->fid);
		XSetFont(XtDisplay(nou), new_ax->xorGC, new_ax->font->fid);
		XSetFont(XtDisplay(nou), new_ax->invertGC, new_ax->font->fid);
		XSetFont(XtDisplay(nou), new_ax->invert_clipGC,
			new_ax->font->fid);
		new_ax->a.font = (void *)new_ax->font;
#ifndef NO_IMAGE
		if(cur_ax->x_axis != LEFT)
		{
		    if(cur_ax->y_label_image != (XImage *)NULL)
		    {
			XDestroyImage(cur_ax->y_label_image);
			new_ax->y_label_image = (XImage *)NULL;
		    }
		    if((int)strlen(new_ax->a.y_axis_label) > 0)
		    {
			if(XtIsRealized((Widget)cur)) {
			    new_ax->y_label_image = GetStrImage(XtDisplay(nou),
				XtWindow(nou), new_ax->axesGC, new_ax->font,
				new_ax->eraseGC, new_ax->a.y_axis_label,
				(int)strlen(new_ax->a.y_axis_label));
			}
		    }
		}
#endif
		redraw = True;
	    }
	}
        if(cur_ax->fg != req->axes.fg)
	{
	    if(XtIsRealized((Widget)cur)) {
                XSetForeground(XtDisplay(nou), new_ax->axesGC, new_ax->fg);
	    }
	    redisplay = True;
	}
	if(cur_ax->select_fg != req->axes.select_fg)
	{
	    if(XtIsRealized((Widget)cur)) {
		XSetForeground(XtDisplay(nou), new_ax->selectGC,
			new_ax->select_fg);
	    }
	}
        if(cur_ax->tickmarks_inside != req->axes.tickmarks_inside)
	{
	    redraw = True;
	}
        if(cur_ax->uniform_scale != req->axes.uniform_scale)
	{
		new_ax->a.uniform_scale = req->axes.uniform_scale;
		redraw = True;
	}
        if(req->axes.title == NULL && (int)strlen(cur_ax->a.top_title) > 0)
	{
		new_ax->a.top_title[0] = '\0';
		redraw = True;
	}
	else if(req->axes.title != NULL)
	{
	    if(!new_ax->save_space) {
		if(strcmp(req->axes.title, cur_ax->a.top_title))
		{
		    Free(cur_ax->a.top_title);
		    new_ax->a.top_title = strdup(req->axes.title);
		    new_ax->title = new_ax->a.top_title;
		    redraw = True;
		}
		else new_ax->title = new_ax->a.top_title;
	    }
	    else if(cur->axes.title && strcmp(req->axes.title, cur->axes.title))
	    {
		    redraw = True;
	    }
	}

        if(req->axes.x_label == NULL && (int)strlen(cur_ax->a.x_axis_label) > 0)
	{
		new_ax->a.x_axis_label[0] = '\0';
		redraw = True;
	}
	else if(req->axes.x_label != NULL &&
		strcmp(req->axes.x_label, cur_ax->a.x_axis_label))
	{
		Free(cur_ax->a.x_axis_label);
		new_ax->a.x_axis_label = strdup(req->axes.x_label);
		new_ax->x_label = new_ax->a.x_axis_label;
		redraw = True;
	}
	else {
		new_ax->x_label = new_ax->a.x_axis_label;
	}

        if(req->axes.y_label == NULL && (int)strlen(cur_ax->a.y_axis_label) > 0)
	{
		new_ax->a.y_axis_label[0] = '\0';
		redraw = True;
	}
	else if(req->axes.y_label != NULL &&
		strcmp(req->axes.y_label, cur_ax->a.y_axis_label))
	{
		Free(cur_ax->a.y_axis_label);
		new_ax->a.y_axis_label = strdup(req->axes.y_label);
		new_ax->y_label = new_ax->a.y_axis_label;

#ifndef NO_IMAGE
		if(cur_ax->x_axis != LEFT)
		{
		    if(cur_ax->y_label_image != (XImage *)NULL)
		    {
			XDestroyImage(cur_ax->y_label_image);
			new_ax->y_label_image = (XImage *)NULL;
		    }
		    if((int)strlen(new_ax->a.y_axis_label) > 0)
		    {
	    		if(XtIsRealized((Widget)cur)) {
			    new_ax->y_label_image = GetStrImage(XtDisplay(nou),
				XtWindow(nou), new_ax->axesGC, new_ax->font,
				new_ax->eraseGC, new_ax->a.y_axis_label,
				(int)strlen(new_ax->a.y_axis_label));
			}
		    }
		}
#endif
		redraw = True;
	}
	else {
		new_ax->y_label = new_ax->a.y_axis_label;
	}

        if(cur_ax->x_axis != req->axes.x_axis)
	{
		if(cur_ax->x_axis == LEFT || req->axes.x_axis == LEFT)
		{
			flip_xy(nou);
		}
		redraw = True;
	}
        if(cur_ax->a.y_label_int != req->axes.a.y_label_int)
	{
		redraw = True;
	}
        if(cur_ax->y_axis != req->axes.y_axis)
	{
		redraw = True;
	}
        if(cur_ax->x_min != req->axes.x_min ||
	   cur_ax->x_max != req->axes.x_max)
	{
		set_limits = True;
	}
        if(cur_ax->y_min != req->axes.y_min ||
           cur_ax->y_max != req->axes.y_max)
	{
		set_limits = True;
	}
        if(cur_ax->time_scale != req->axes.time_scale)
	{
		if(req->axes.time_scale == TIME_SCALE_SECONDS)
		{
		    new_x_label(cur, nou, "Time (seconds)");
		}
		else if(req->axes.time_scale == TIME_SCALE_HMS)
		{
		    if(new_ax->x2[new_ax->zoom] - new_ax->x1[new_ax->zoom]
				<= 40*3600) {
			new_x_label(cur, nou, "Time (hr:min:sec)");
		    }
		    else {
			new_x_label(cur, nou, "Time (yr/mo/day)");
		    }
		}
		redraw = True;
	}
	if(cur_ax->horizontal_scroll != req->axes.horizontal_scroll)
	{
		if(!req->axes.horizontal_scroll)
		{
			XtUnmanageChild(new_ax->hor_scroll);
		}
		redraw = True;
	}
	if(cur_ax->vertical_scroll != req->axes.vertical_scroll)
	{
		if(!req->axes.vertical_scroll)
		{
			XtUnmanageChild(new_ax->vert_scroll);
		}
		redraw = True;
	}
	if(cur_ax->vertical_scroll_position!=req->axes.vertical_scroll_position)
	{
	    DoLayout(nou);
	    redraw = True;
	}
	if(cur_ax->display_axes != req->axes.display_axes ||
		cur_ax->display_axes_labels != req->axes.display_axes_labels)
	{
		DoLayout(nou);
		redraw = True;
	}
	if(cur_ax->label_mode != req->axes.label_mode)
	{
		new_ax->label_xpos = new_ax->label_ypos = -1;
	}
        if(cur_ax->display_grid != req->axes.display_grid) {
	    redisplay = True;
	}
	if(cur_ax->a.log_x != req->axes.a.log_x) {
	    redisplay = True;
	}
	if(cur_ax->a.log_y != req->axes.a.log_y) {
	    redisplay = True;
	}

	if(set_limits)
	{
		SetLimits(nou, False);
		DoMagFrom(nou);
		redraw = True;
	}
	if(redraw)
	{
		Redraw(nou);
		redisplay = True;
	}
	if(new_ax->redisplay)	/* put a redisplay on the event queue */
	{
		new_ax->redisplay = False;
		redisplay = True;
	}
	if(redisplay)
	{
		new_ax->setvalues_redisplay = True;
		if(new_ax->redisplay_pending) redisplay = False;
		new_ax->redisplay_pending = True;
	}
	return(redisplay);
}

void
AxesSetYLimits(AxesWidget w, double ymin, double ymax, bool init)
{
    AxesPart *ax = &w->axes;

    if(ax->y_min == ymin && ax->y_max == ymax) return;

    ax->y_min = ymin;
    ax->y_max = ymax;

    SetLimits(w, init);
    if( !init ) {
	DoMagFrom(w);
	Redraw(w);
	Redisplay((Widget)w, NULL, NULL);
    }
}

void AxesClass::setXLimits(double xmin, double xmax, bool init)
{
    AxesPart *ax = &aw->axes;

    if(ax->x_min == xmin && ax->x_max == xmax) return;

    ax->x_min = xmin;
    ax->x_max = xmax;

    SetLimits(aw, init);
    if( !init ) {
	DoMagFrom(aw);
	Redraw(aw);
	Redisplay((Widget)aw, NULL, NULL);
    }
}

void AxesClass::setLimits(double xmin, double xmax, double ymin, double ymax,
		bool init)
{
    AxesPart *ax = &aw->axes;

    if( ax->x_min == xmin && ax->x_max == xmax &&
	ax->y_min == ymin && ax->y_max == ymax) return;

    ax->x_min = xmin;
    ax->x_max = xmax;
    ax->y_min = ymin;
    ax->y_max = ymax;

    SetLimits(aw, init);
    if( !init ) {
	DoMagFrom(aw);
	Redraw(aw);
	Redisplay((Widget)aw, NULL, NULL);
    }
}

void AxesClass::axesExpose(bool expose)
{
    AxesPart *ax = &aw->axes;
    ax->expose = expose;
    if(expose) {
	Redisplay((Widget)aw, NULL, NULL);
    }
    else {
	ax->redisplay_pending = True;
    }
    if(ax->mag_to != NULL) {
	ax = &((AxesWidget)ax->mag_to)->axes;
	ax->expose = expose;
	if(expose) {
	    Redisplay(ax->mag_to, NULL, NULL);
	}
	else {
	    ax->redisplay_pending = True;
	}
    }
}

static void
Redisplay(Widget widget, XEvent *event, Region region)
{
	AxesWidget w = (AxesWidget)widget;
	AxesPart *ax = &w->axes;
	int ix1, iy1, ix2, iy2, width, height;
	XEvent e;

	if(!ax->expose) return;

	if(ax->check_expose_region) {
	    GetDrawArea(&ax->d, &ix1, &iy1, &ix2, &iy2);
	    width = ix2 - ix1 + 1;
	    height = iy1 - iy2 + 1;
	    if(event != NULL && region != NULL &&
		XRectInRegion(region, ix1, iy2, width, height) == RectangleOut)
	    {
	    	return;
	    }
	}

	ax->redisplay_pending = False;

	if(event == NULL || !ax->use_pixmap || ax->setvalues_redisplay)
	{
	    if(!_AxesRedisplayAxes(w)) return;

	    ax->setvalues_redisplay = False;
	    ax->use_screen = False;
	    if(ax->redraw_data_func != NULL) {
		(*(ax->redraw_data_func))(w, DATA_REDISPLAY, True);
	    }
	    _AxesRedisplayXor(w);
	    ax->use_screen = True;
	}

	if(XtIsRealized(widget)) {
	    if(ax->use_pixmap) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	    }
	    while(XCheckWindowEvent(XtDisplay(w),XtWindow(w),ExposureMask,&e));
	}
}

/** 
 *  
 */
void
_AxesRedisplayXor(AxesWidget w)
{
	if(!XtIsRealized((Widget)w))
	{
		return;
	}
	_AxesRedisplayCursors(w);
	_AxesRedisplayMarks(w);
	_AxesRedisplayMagRect(w);
        _AxesRedisplayOverlays(w);
	_AxesRedisplayOverlays2(w);
}

/** 
 *  
 */
Boolean
_AxesRedisplayAxes(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int i, ix1, iy1, ix2, iy2, xmin, xmax, ymin, ymax, n;
	
	if(!XtIsRealized((Widget)w))
	{
		return(False);
	}
	GetDrawArea(&ax->d, &ix1, &iy1, &ix2, &iy2);

	if(ax->clear_on_redisplay)
	{
		_AxesClearArea(w, ix1, iy2, ix2-ix1+1, iy1-iy2+1);
	}
	else
	{
		xmin = unscale_x(&ax->d, ax->x1[ax->zoom]);
		xmax = unscale_x(&ax->d, ax->x2[ax->zoom]);
		ymin = unscale_y(&ax->d, ax->y1[ax->zoom]);
		ymax = unscale_y(&ax->d, ax->y2[ax->zoom]);
		_AxesClearArea(w, ix1, iy2, ix2-ix1+1, ymax-iy2+1);
		_AxesClearArea(w, ix1, ymin, ix2-ix1+1, iy1-ymin+1);
		_AxesClearArea(w, ix1, iy2, xmin-ix1+1, iy1-iy2+1);
		_AxesClearArea(w, xmax, iy2, ix2-xmax+1, iy1-iy2+1);
	}

	DrawTics(w);

	if(ax->display_grid) DrawGrid(w);

	if( ax->save_space && ax->title[0] != '\0') {
	    _AxesDrawString(w, ax->axesGC, ax->title_x, ax->title_y, ax->title);
	}

	if(ax->a.top_title != NULL && (int)strlen(ax->a.top_title) > 0)
	{
		_AxesDrawString(w, ax->axesGC, ax->a.title_x,
				ax->a.title_y, ax->a.top_title);
	}
	if(ax->a.x_axis_label != NULL &&
			(int)strlen(ax->a.x_axis_label) > 0)
	{
		_AxesDrawString(w, ax->axesGC, ax->a.xlab_x,
			ax->a.xlab_y, ax->a.x_axis_label);
	}
	if(ax->a.y_axis_label != NULL &&
		(int)strlen(ax->a.y_axis_label) > 0)
	{
#ifndef NO_IMAGE
		if(ax->y_label_image == NULL)
		{
		    ax->y_label_image = GetStrImage(XtDisplay(w),
				XtWindow(w), ax->axesGC, ax->font,
				ax->eraseGC, ax->a.y_axis_label,
				(int)strlen(ax->a.y_axis_label));
		}
		if(ax->use_pixmap)
		{
			XPutImage(XtDisplay(w), ax->pixmap, ax->axesGC,
			    ax->y_label_image, 0, 0, ax->a.ylab_x,
			    ax->a.ylab_y,ax->a.ylab_h,ax->a.ylab_w);
		}
		else
		{
			XPutImage(XtDisplay(w), XtWindow(w), ax->axesGC,
			    ax->y_label_image, 0, 0, ax->a.ylab_x,
			    ax->a.ylab_y,ax->a.ylab_h,ax->a.ylab_w);
		}
#endif
	}

	if( (n=ax->num_time_marks) > 0) {
	    XSegment *s = (XSegment *)malloc(4*n*sizeof(XSegment));
	    Pixel yellow = StringToPixel((Widget)w, (char *)"yellow");
	    xmin = unscale_x(&ax->d, ax->x1[ax->zoom]);
	    xmax = unscale_x(&ax->d, ax->x2[ax->zoom]);
	    iy1 = unscale_y(&ax->d, ax->y2[ax->zoom]) - 1;

	    for(i = n = 0; i < ax->num_time_marks; i++) {
		ix1 = unscale_x(&ax->d, ax->time_marks[i]);
		if(ix1 >= xmin && ix1 <= xmax) {
		    s[n].x1 = ix1-2; s[n].x2 = ix1-2;
		    s[n].y1 = iy1; s[n].y2 = iy1-7;
		    n++;
		    s[n].x1 = ix1-1; s[n].x2 = ix1-1;
		    s[n].y1 = iy1; s[n].y2 = iy1-7;
		    n++;
		    s[n].x1 = ix1+1; s[n].x2 = ix1+1;
		    s[n].y1 = iy1; s[n].y2 = iy1-7;
		    n++;
		    s[n].x1 = ix1+2; s[n].x2 = ix1+2;
		    s[n].y1 = iy1; s[n].y2 = iy1-7;
		    n++;
		}
	    }
	    if(n > 0) {
		XSetForeground(XtDisplay(w), ax->axesGC, yellow);
		if(ax->use_pixmap) {
		    XDrawSegments(XtDisplay(w), ax->pixmap, ax->axesGC, s, n);
		}
		else {
		    XDrawSegments(XtDisplay(w), XtWindow(w), ax->axesGC, s, n);
		}
	    }
	    for(i = n = 0; i < ax->num_time_marks; i++) {
		ix1 = unscale_x(&ax->d, ax->time_marks[i]);
		if(ix1 >= xmin && ix1 <= xmax) {
		    s[n].x1 = ix1; s[n].x2 = ix1;
		    s[n].y1 = iy1; s[n].y2 = iy1-7;
		    n++;
		}
	    }
	    if(n > 0) {
		XSetForeground(XtDisplay(w), ax->axesGC, ax->fg);
		if(ax->use_pixmap) {
		    XDrawSegments(XtDisplay(w), ax->pixmap, ax->axesGC, s, n);
		}
		else {
		    XDrawSegments(XtDisplay(w), XtWindow(w), ax->axesGC, s, n);
		}
	    }
	    free(s);
	}

	return(True);
}

void AxesClass::setTimeMarks(int num, double *t)
{
    AxesPart *ax = &aw->axes;
    Free(ax->time_marks);
    ax->num_time_marks = (num >= 0) ? num : 0;
    if(num > 0) {
	ax->time_marks = (double *)malloc(num*sizeof(double));
	memcpy(ax->time_marks, t, num*sizeof(double));
    }
    redraw();
}

void AxesClass::clearPlot()
{
    AxesPart *ax = &aw->axes;
    int ix1, iy1, ix2, iy2, x, y, width, height;
	
    ix1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
    ix2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
    iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
    iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);

    x = (ix1 < ix2) ? ix1+1 : ix2+1;
    y = (iy1 < iy2) ? iy1+1 : iy2+1;

    width  = abs(ix2 - ix1) - 1;
    height = abs(iy2 - iy1) - 1;
	
    if(width > 0 && height > 0) {
	_AxesClearArea(aw, x, y, width, height);
    }
}

/** 
 *  
 */
void
_AxesRedisplayCursors(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int i;

	if(!XtIsRealized((Widget)w))
	{
		return;
	}
	for(i = 0; i < ax->num_cursors; i++)
	{
		DrawCursor(w, i);
	}
}

/** 
 *  
 */
void
_AxesRedisplayMarks(AxesWidget w)
{
	if(!XtIsRealized((Widget)w))
	{
		return;
	}
	LastMark(w);
}

/** 
 *  
 */
void
_AxesRedisplayMagRect(AxesWidget w)
{
    AxesPart *ax = &w->axes;
    int mag_x1, mag_x2, mag_y1, mag_y2, width, height;

    if(!XtIsRealized((Widget)w)) return;

    if(AxesFormMagRect(w, &mag_x1, &mag_x2, &mag_y1, &mag_y2))
    {
	width  = (mag_x2 > mag_x1) ? mag_x2 - mag_x1 : 1;
	height = (mag_y2 > mag_y1) ? mag_y2 - mag_y1 : 1;
	ax->axes_class->drawRectangle(ax->mag_invertGC, mag_x1, mag_y1,
			width, height);
    }
}

static Boolean
AxesFormMagRect(
	AxesWidget w,
	int *mag_x1, int *mag_x2, int *mag_y1, int *mag_y2)
{
	AxesPart *ax = &w->axes;
	int m;
	double x1, x2, y1, y2;
	double sx1, sx2, sy1, sy2;

	if(!ax->magnify_rect)
	{
		return(False);
	}
	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	if(ax->mag_scaled_x1 >= x2 || ax->mag_scaled_x2 <= x1 ||
	   ax->mag_scaled_y1 >= y2 || ax->mag_scaled_y2 <= y1)
	{
		return(False);
	}
	sx1 = (ax->mag_scaled_x1 > x1) ?  ax->mag_scaled_x1 :
		x1 - .1*(x2-x1);
	sx2 = (ax->mag_scaled_x2 < x2) ?  ax->mag_scaled_x2 :
		x2 + .1*(x2-x1);
	sy1 = (ax->mag_scaled_y1 > y1) ?  ax->mag_scaled_y1 :
		y1 - .1*(y2-y1);
	sy2 = (ax->mag_scaled_y2 < y2) ?  ax->mag_scaled_y2 :
		y2 + .1*(y2-y1);
	*mag_x1 = unscale_x(&ax->d, sx1);
	*mag_x2 = unscale_x(&ax->d, sx2);
	*mag_y1 = unscale_y(&ax->d, sy1);
	*mag_y2 = unscale_y(&ax->d, sy2);
	if(*mag_x1 > *mag_x2)
	{
		m = *mag_x2;
		*mag_x2 = *mag_x1;
		*mag_x1 = m;
	}
	if(*mag_y1 > *mag_y2)
	{
		m = *mag_y2;
		*mag_y2 = *mag_y1;
		*mag_y1 = m;
	}
	return(True);
}

int AxesClass::setOverlayRect2(double ox1, double ox2, double oy1, double oy2,
		Pixel pixel, int width, bool display, int in_id, bool redisplay)
{
    AxesPart *ax = &aw->axes;
    Widget w = (Widget)aw;
    int	i, id;

    id = -1;

    if(in_id >= 0)
    {
	// add this retangle to the overlay group with the same pixel and width
	for(i = 0; i < ax->n_ovy_grps; i++)
	{
	    if(ax->ovy_grp[i].pixel == pixel && ax->ovy_grp[i].width == width)
	    {
		id = in_id;
		/* should verify that in_id < ax->ovy_grp[i].nrecs */
		ax->ovy_grp[i].ovr_scaled_x1[in_id] = ox1;
		ax->ovy_grp[i].ovr_scaled_x2[in_id] = ox2;
		ax->ovy_grp[i].ovr_scaled_y1[in_id] = oy1;
		ax->ovy_grp[i].ovr_scaled_y2[in_id] = oy2;
		ax->ovy_grp[i].display[in_id] = display;
	    }
	}
    }
    else
    {
	// add this retangle to the overlay group with the same pixel and width
	for(i = 0; i < ax->n_ovy_grps; i++)
	{
	    if(ax->ovy_grp[i].pixel == pixel && ax->ovy_grp[i].width == width)
	    {
		id = ax->ovy_grp[i].nrecs;	
		ax->ovy_grp[i].ovr_scaled_x1 = (double *)AxesRealloc(w,
			ax->ovy_grp[i].ovr_scaled_x1, (id+1)*sizeof(double));
		ax->ovy_grp[i].ovr_scaled_x2 = (double *)AxesRealloc(w,
			ax->ovy_grp[i].ovr_scaled_x2, (id+1)*sizeof(double));
		ax->ovy_grp[i].ovr_scaled_y1 = (double *)AxesRealloc(w,
			ax->ovy_grp[i].ovr_scaled_y1, (id+1)*sizeof(double));
		ax->ovy_grp[i].ovr_scaled_y2 = (double *)AxesRealloc(w,
			ax->ovy_grp[i].ovr_scaled_y2, (id+1)*sizeof(double));
		ax->ovy_grp[i].display = (Boolean *)AxesRealloc(w,
			ax->ovy_grp[i].display, (id+1)*sizeof(Boolean));
		ax->ovy_grp[i].ovr_scaled_x1[id] = ox1;
		ax->ovy_grp[i].ovr_scaled_x2[id] = ox2;
		ax->ovy_grp[i].ovr_scaled_y1[id] = oy1;
		ax->ovy_grp[i].ovr_scaled_y2[id] = oy2;
		ax->ovy_grp[i].display[id] = display;

		ax->ovy_grp[i].nrecs++;
		break;
	    }
	}

	// if there was no overlay group with the same pixel and width,
	// then add a new group
	if(i == ax->n_ovy_grps)
	{
	    // find the first group with no current members
	    for(i = 0; i < MAX_RECTS; i++)
	    {
		if(ax->ovy_grp[i].nrecs == 0)
		{
		    id = 0;
		    ax->ovy_grp[i].ovr_scaled_x1=(double *)AxesMalloc(w,
					(id+1)*sizeof(double));
		    ax->ovy_grp[i].ovr_scaled_x2=(double *)AxesMalloc(w,
					(id+1)*sizeof(double));
		    ax->ovy_grp[i].ovr_scaled_y1=(double *)AxesMalloc(w,
					(id+1)*sizeof(double));
		    ax->ovy_grp[i].ovr_scaled_y2=(double *)AxesMalloc(w,
					(id+1)*sizeof(double));
		    ax->ovy_grp[i].display = (Boolean *)AxesMalloc(w,
					(id+1)*sizeof(Boolean));
		    ax->ovy_grp[i].ovr_scaled_x1[id] = ox1;
		    ax->ovy_grp[i].ovr_scaled_x2[id] = ox2;
		    ax->ovy_grp[i].ovr_scaled_y1[id] = oy1;
		    ax->ovy_grp[i].ovr_scaled_y2[id] = oy2;
		    ax->ovy_grp[i].display[id] = display;
    
		    ax->ovy_grp[i].nrecs = 1;
		    ax->ovy_grp[i].pixel = pixel;
		    ax->ovy_grp[i].width = width;
		    ax->n_ovy_grps = i+1;
		    break;
		}
	    }
	}
    }

    if(redisplay)
    {
	Redisplay(w, NULL, NULL);
    }
    return id;
}

void AxesClass::removeAllOverlays()
{
    AxesPart *ax = &aw->axes;

    for(int i = 0; i < ax->n_ovy_grps; i++)
    {
	if(ax->ovy_grp[i].nrecs > 0)
	{
	   Free(ax->ovy_grp[i].ovr_scaled_x1);
	   Free(ax->ovy_grp[i].ovr_scaled_x2);
	   Free(ax->ovy_grp[i].ovr_scaled_y1);
	   Free(ax->ovy_grp[i].ovr_scaled_y2);
	   Free(ax->ovy_grp[i].display);
	}
	ax->ovy_grp[i].nrecs = 0;
	ax->ovy_grp[i].pixel = 0;
	ax->ovy_grp[i].width = 0;
    }
    ax->n_ovy_grps = 0;
    Redisplay((Widget)aw, NULL, NULL);
}

int AxesClass::removeOverlayRect2(Pixel pixel, int width, int in_id,
			bool redisplay)
{
    AxesPart *ax = &aw->axes;
    int	i, j;

    // add this retangle to the overlay group with the same pixel and width
    for (i=0; i<ax->n_ovy_grps; i++)
    {
	if (ax->ovy_grp[i].pixel == pixel && ax->ovy_grp[i].width == width)
	{
	    /* remove the rectangle at in_id position, and shift the rest down
		by one */
	    for (j=in_id; j<ax->ovy_grp[i].nrecs-1; j++)
	    {
		ax->ovy_grp[i].ovr_scaled_x1[j] =
				ax->ovy_grp[i].ovr_scaled_x1[j+1];
		ax->ovy_grp[i].ovr_scaled_x2[j] =
				ax->ovy_grp[i].ovr_scaled_x2[j+1];
		ax->ovy_grp[i].ovr_scaled_y1[j] =
				ax->ovy_grp[i].ovr_scaled_y1[j+1];
		ax->ovy_grp[i].ovr_scaled_y2[j] =
				ax->ovy_grp[i].ovr_scaled_y2[j+1];
		ax->ovy_grp[i].display[j] = ax->ovy_grp[i].display[j+1];
	    }
	    ax->ovy_grp[i].nrecs--;
	    if (ax->ovy_grp[i].nrecs == 0)
	    {
		Free(ax->ovy_grp[i].ovr_scaled_x1);
		Free(ax->ovy_grp[i].ovr_scaled_x2);
		Free(ax->ovy_grp[i].ovr_scaled_y1);
		Free(ax->ovy_grp[i].ovr_scaled_y2);
		Free(ax->ovy_grp[i].display);
	    }
	}
    }

    if (redisplay)
    {
	Redisplay((Widget)aw, NULL, NULL);
    }
    return 1;
}

void
_AxesRedisplayOverlays2(AxesWidget w)
{
    AxesPart *ax = &w->axes;
    int i, maxRec, nrec;
    XRectangle *recs;

    if(!XtIsRealized((Widget)w)) return;

    maxRec = 0;
    for (i=0; i<ax->n_ovy_grps; i++)
	    if (ax->ovy_grp[i].nrecs > maxRec)
    {
	maxRec = ax->ovy_grp[i].nrecs;
    }

    if (maxRec == 0) return;

    recs = (XRectangle *)AxesMalloc((Widget)w, (maxRec+1)*sizeof(XRectangle));

    for (i=0; i<ax->n_ovy_grps; i++)
	    if (ax->ovy_grp[i].nrecs > 0)
    {
	nrec = AxesFormOverlayRects(w, i, recs);
	if (nrec > 0)
	{
	    XSetClipRectangles(XtDisplay(w), ax->invertGC, 0, 0,
			&ax->clip_rect, 1, Unsorted);


	    XSetForeground(XtDisplay(w), ax->invertGC, ax->ovy_grp[i].pixel);
	    XSetPlaneMask(XtDisplay(w), ax->invertGC, ax->ovy_grp[i].pixel ^ 
			w->core.background_pixel);

	    XDrawRectangles(XtDisplay(w), XtWindow(w), ax->invertGC, recs,nrec);
	    if (ax->use_pixmap)
	    {
		XDrawRectangles(XtDisplay(w), ax->pixmap, ax->invertGC, recs,
			nrec);
	    }
	    XSetClipMask(XtDisplay(w), ax->invertGC, None);
	}
    }
    XSetForeground(XtDisplay(w), ax->invertGC, ax->fg);
    XSetPlaneMask(XtDisplay(w), ax->invertGC,ax->fg ^ w->core.background_pixel);

    Free(recs);
}

static int
AxesFormOverlayRects(AxesWidget w, int pos, XRectangle *recs)
{
    AxesPart *ax = &w->axes;
    int i, m, nrec;
    double x1, x2, y1, y2;
    double sx1, sx2, sy1, sy2;
    int o_x1, o_x2, o_y1, o_y2;

    x1 = ax->x1[ax->zoom];
    x2 = ax->x2[ax->zoom];
    y1 = ax->y1[ax->zoom];
    y2 = ax->y2[ax->zoom];

    nrec = 0;

    for (i=0; i<ax->ovy_grp[pos].nrecs; i++)
    {
	if(!ax->ovy_grp[pos].display[i]
	    || ax->ovy_grp[pos].ovr_scaled_x1[i] >= x2
	    || ax->ovy_grp[pos].ovr_scaled_x2[i] <= x1
	    || ax->ovy_grp[pos].ovr_scaled_y1[i] >= y2
	    || ax->ovy_grp[pos].ovr_scaled_y2[i] <= y1)
	{
	    continue;
	}
	sx1 = (ax->ovy_grp[pos].ovr_scaled_x1[i] > x1) ?
		ax->ovy_grp[pos].ovr_scaled_x1[i] : x1 - .1*(x2-x1);
	sx2 = (ax->ovy_grp[pos].ovr_scaled_x2[i] < x2) ?
		ax->ovy_grp[pos].ovr_scaled_x2[i] : x2 + .1*(x2-x1);
	sy1 = (ax->ovy_grp[pos].ovr_scaled_y1[i] > y1) ?
		ax->ovy_grp[pos].ovr_scaled_y1[i] : y1 - .1*(y2-y1);
	sy2 = (ax->ovy_grp[pos].ovr_scaled_y2[i] < y2) ?
		ax->ovy_grp[pos].ovr_scaled_y2[i] : y2 + .1*(y2-y1);

	o_x1 = unscale_x(&ax->d, sx1);
	o_x2 = unscale_x(&ax->d, sx2);
	o_y1 = unscale_y(&ax->d, sy1);
	o_y2 = unscale_y(&ax->d, sy2);
	if(o_x1 > o_x2)
	{
	    m = o_x2;
	    o_x2 = o_x1;
	    o_x1 = m;
	}
	if(o_y1 > o_y2)
	{
	    m = o_y2;
	    o_y2 = o_y1;
	    o_y1 = m;
	}

	recs[nrec].x = o_x1;
	recs[nrec].y = o_y1;
	recs[nrec].width = (o_x2 > o_x1) ? o_x2 - o_x1 : 1;
	recs[nrec].height = (o_y2 > o_y1) ? o_y2 - o_y1 : 1;

	nrec++;
    }
    return nrec;
}


void AxesClass::setOverlayRect(double ox1, double ox2, double oy1, double oy2)
{
    AxesPart *ax = &aw->axes;

    ax->overlay_rect = true;
    ax->ovr_scaled_x1 = ox1;
    ax->ovr_scaled_x2 = ox2;
    ax->ovr_scaled_y1 = oy1;
    ax->ovr_scaled_y2 = oy2;
    Redisplay((Widget)aw, NULL, NULL);
}

void AxesClass::removeOverlayRect()
{
    AxesPart *ax = &aw->axes;
    ax->overlay_rect = false;
    Redisplay((Widget)aw, NULL, NULL);
}

void
_AxesRedisplayOverlays(AxesWidget w)
{
    AxesPart *ax = &w->axes;
    int o_x1=0, o_x2=0, o_y1=0, o_y2=0, width, height;

    if(!XtIsRealized((Widget)w) || !ax->overlay_rect) return;

    if(AxesFormOverlay(w, &o_x1, &o_x2, &o_y1, &o_y2))
    {
	width  = (o_x2 > o_x1) ? o_x2 - o_x1 : 1;
	height = (o_y2 > o_y1) ? o_y2 - o_y1 : 1;
	XSetClipRectangles(XtDisplay(w), ax->invertGC, 0, 0,
			&ax->clip_rect, 1, Unsorted);
	ax->axes_class->drawRectangle(ax->invertGC, o_x1, o_y1, width, height);
	XSetClipMask(XtDisplay(w), ax->invertGC, None);
    }
}

static Boolean
AxesFormOverlay(AxesWidget w, int *o_x1, int *o_x2, int *o_y1, int *o_y2)
{
	AxesPart *ax = &w->axes;
	int m;
	double x1, x2, y1, y2;
	double sx1, sx2, sy1, sy2;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	if(ax->ovr_scaled_x1 >= x2 || ax->ovr_scaled_x2 <= x1 ||
	   ax->ovr_scaled_y1 >= y2 || ax->ovr_scaled_y2 <= y1)
	{
		return(False);
	}
	sx1 = (ax->ovr_scaled_x1 > x1) ?  ax->ovr_scaled_x1 :
		x1 - .1*(x2-x1);
	sx2 = (ax->ovr_scaled_x2 < x2) ?  ax->ovr_scaled_x2 :
		x2 + .1*(x2-x1);
	sy1 = (ax->ovr_scaled_y1 > y1) ?  ax->ovr_scaled_y1 :
		y1 - .1*(y2-y1);
	sy2 = (ax->ovr_scaled_y2 < y2) ?  ax->ovr_scaled_y2 :
		y2 + .1*(y2-y1);
	*o_x1 = unscale_x(&ax->d, sx1);
	*o_x2 = unscale_x(&ax->d, sx2);
	*o_y1 = unscale_y(&ax->d, sy1);
	*o_y2 = unscale_y(&ax->d, sy2);
	if(*o_x1 > *o_x2)
	{
		m = *o_x2;
		*o_x2 = *o_x1;
		*o_x1 = m;
	}
	if(*o_y1 > *o_y2)
	{
		m = *o_y2;
		*o_y2 = *o_y1;
		*o_y1 = m;
	}
	return(True);
}

void AxesClass::redraw()
{
    if(!XtIsRealized((Widget)aw)) return;

    _AxesRedraw(aw);
    _AxesRedisplayAxes(aw);
    if(aw->axes.redraw_data_func != NULL) {
	(*(aw->axes.redraw_data_func))(aw, DATA_REDRAW, False);
    }
    Redisplay((Widget)aw, NULL, NULL);
}

static void
Redraw(AxesWidget w)
{
	AxesPart *ax = &w->axes;

	if(!XtIsRealized((Widget)w)) return;

	_AxesRedraw(w);
	if(ax->redraw_data_func != NULL)
	{
		(*(ax->redraw_data_func))(w, DATA_REDRAW, False);
	}
}

/** 
 *  
 */
void
_AxesRedraw(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int i, ix1, ix2, iy1, iy2, left, right, top, bottom;
	double x_left, x_right, y_bottom, y_top, d;
	AxesLimitsCallbackStruct s;

	/* for purify */
	x_left = x_right = y_bottom = y_top = d = 0.0;

	_AxesClearArea(w, 0, 0, w->core.width, w->core.height);

	DoLayout(w);

	if(ax->x1[ax->zoom] == ax->x2[ax->zoom])
	{
		d = .1*fabs(ax->x1[ax->zoom]);
		if(d == 0.) d = .1;
		ax->x1[ax->zoom] -= d;
		ax->x2[ax->zoom] += d;
	}
	if(ax->y1[ax->zoom] == ax->y2[ax->zoom])
	{
		d = .1*fabs(ax->y1[ax->zoom]);
		if(d == 0.) d = .1;
		ax->y1[ax->zoom] -= d;
		ax->y2[ax->zoom] += d;
	}

	if(!ax->reverse_x)
	{
		x_left  = ax->x1[ax->zoom];
		x_right = ax->x2[ax->zoom];
	}
	else
	{
		x_left  = ax->x2[ax->zoom];
		x_right = ax->x1[ax->zoom];
	}
	if(!ax->reverse_y)
	{
		y_bottom = ax->y1[ax->zoom];
		y_top	 = ax->y2[ax->zoom];
	}
	else
	{
		y_bottom = ax->y2[ax->zoom];
		y_top	 = ax->y1[ax->zoom];
	}
	left   = ax->a.left;
	right  = ax->a.right;
	top    = ax->a.top;
	bottom = ax->a.bottom;

	ax->a.ymin = (ax->y1[0] < ax->y2[0]) ? ax->y1[0] : ax->y2[0];
	ax->a.ymax = (ax->y2[0] > ax->y1[0]) ? ax->y2[0] : ax->y1[0];
	ax->a.check_y_cursor = haveCrosshair(ax);
	gdrawAxis(&ax->a, &ax->d, x_left, x_right, ax->x_axis,
		y_bottom, y_top, ax->y_axis, ax->tickmarks_inside,
		ax->display_axes_labels, ax->time_scale,time_factor(w, True));
	if(ax->x_axis != LEFT)
	{
		ax->x_min = ax->x1[ax->zoom];
		ax->x_max = ax->x2[ax->zoom];
		if(!ax->reverse_y)
		{
			ax->y_min = ax->y1[ax->zoom];
			ax->y_max = ax->y2[ax->zoom];
		}
		else
		{
			ax->y_min = ax->y2[ax->zoom];
			ax->y_max = ax->y1[ax->zoom];
		}
	}
	else
	{
		ax->x_min = ax->y1[ax->zoom];
		ax->x_max = ax->y2[ax->zoom];
		if(!ax->reverse_x)
		{
			ax->y_min = ax->x1[ax->zoom];
			ax->y_max = ax->x2[ax->zoom];
		}
		else
		{
			ax->y_min = ax->x2[ax->zoom];
			ax->y_max = ax->x1[ax->zoom];
		}
	}
	if( !ax->skip_limits_callback ) {
		s.limits = True;
		s.x_margins = (left != ax->a.left || right != ax->a.right);
		s.y_margins = (top != ax->a.top || bottom != ax->a.bottom);
		s.left   = ax->a.left;
		s.right  = ax->a.right;
		s.top    = ax->a.top;
		s.bottom = ax->a.bottom;
		s.x_min = ax->x_min;
		s.x_max = ax->x_max;
		s.y_min = ax->y_min;
		s.y_max = ax->y_max;
		XtCallCallbacks((Widget)w, XtNlimitsCallback, &s);
	}

	_Axes_AdjustScrollBars(w);
	ix1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
	iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
	ix2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
	iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if(iy2 < iy1)
	{
		i = iy2;
		iy2 = iy1;
		iy1 = i;
	}
	if(ix2 < ix1)
	{
		i = ix2;
		ix2 = ix1;
		ix1 = i;
	}
	ax->old_clip.x = ax->clip_rect.x;
	ax->old_clip.y = ax->clip_rect.y;
	ax->old_clip.width = ax->clip_rect.width;
	ax->old_clip.height = ax->clip_rect.height;
	ax->clipx1 = ix1+1;
	ax->clipx2 = ix2-1;
	ax->clipy1 = iy1+1;
	ax->clipy2 = iy2-1;
	ax->clip_rect.x = ax->clipx1;
	ax->clip_rect.y = ax->clipy1;
	ax->clip_rect.width = ax->clipx2 - ax->clipx1 + 1;
	ax->clip_rect.height = ax->clipy2 - ax->clipy1 + 1;

	if( XtIsRealized((Widget)w) ) {
	    XSetClipRectangles(XtDisplay(w), ax->invert_clipGC, 0, 0,
			&ax->clip_rect, 1, Unsorted);
	    XSetClipRectangles(XtDisplay(w), ax->mag_invertGC, 0, 0,
			&ax->clip_rect, 1, Unsorted);
	    XSetClipRectangles(XtDisplay(w), ax->xorGC, 0, 0,
			&ax->clip_rect, 1, Unsorted);
	}
	DrawTitles(w);
}

static void
DrawTics(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int i;

	if(!ax->display_axes)
	{
	    return;
	}
	if(ax->display_axes_labels == AXES_X ||
	   ax->display_axes_labels == AXES_XY)
	{
	    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[0].segs,
					ax->a.axis_segs[0].n_segs);
	    if(!ax->save_space && ax->extra_x_tics) {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[1].segs,
					ax->a.axis_segs[1].n_segs);
	    }
	    else if(ax->a.axis_segs[1].n_segs > 0) {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[1].segs, 1);
	    }
	    drawXLabels(w, ax);
	}
	else
	{
	    for(i = 0; i < 2; i++) if(ax->a.axis_segs[i].n_segs > 0)
	    {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[i].segs, 1);
	    }
	}
	if(ax->display_axes_labels == AXES_Y ||
	   ax->display_axes_labels == AXES_XY)
	{
	    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[2].segs,
				ax->a.axis_segs[2].n_segs);
	    if(!ax->save_space && ax->extra_y_tics) {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[3].segs,
				ax->a.axis_segs[3].n_segs);
	    }
	    else if(ax->a.axis_segs[3].n_segs > 0) {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[3].segs, 1);
	    }
	    drawYLabels(w, ax);
	}
	else
	{
	    for(i = 2; i < 4; i++) if(ax->a.axis_segs[i].n_segs > 0)
	    {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[i].segs, 1);
	    }
	}
}

static void
DrawGrid(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int i, j, x, y, xmin, xmax, ymin, ymax;
	DSegment segs[10000];
	
	if(!ax->display_axes) return;

	xmin = unscale_x(&ax->d, ax->x1[ax->zoom]);
	xmax = unscale_x(&ax->d, ax->x2[ax->zoom]);
	if(xmin > xmax) {
	    x = xmin;
	    xmin = xmax;
	    xmax = x;
	}
	ymin = unscale_y(&ax->d, ax->y1[ax->zoom]);
	ymax = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if(ymin > ymax) {
	    y = ymin;
	    ymin = ymax;
	    ymax = y;
	}

	for(i = 0; i < ax->a.nylab; i++)
	{
	    y = unscale_y(&ax->d, ax->a.y_lab[i]);
	    if(y > ymin+1 && y < ymax-1) {
		x = xmin;
		for(j = 0; j < 10000 && x < xmax; j++, x += 4) {
		    segs[j].x1 = x;
		    segs[j].y1 = y;
		    segs[j].x2 = x;
		    segs[j].y2 = y;
		}
		_AxesDrawSegments(w, ax->axesGC, segs, j);
	    }
	}
	for(i = 0; i < ax->a.nxlab; i++)
	{
	    x = unscale_x(&ax->d, ax->a.x_lab[i]);
	    if(x > xmin+1 && x < xmax-1) {
		y = ymin;
		for(j = 0; j < 10000 && y < ymax; j++, y += 4) {
		    segs[j].x1 = x;
		    segs[j].y1 = y;
		    segs[j].x2 = x;
		    segs[j].y2 = y;
		}
		_AxesDrawSegments(w, ax->axesGC, segs, j);
	    }
	}
}

static void
drawYLabels(AxesWidget w, AxesPart *ax)
{
	AxesParm *a = &ax->a;
	int i, iy, iy2, ymin, ymax;
	double y;
	Boolean no_margins = False;

	if(!ax->draw_y_labels) return;

	ymin = unscale_y(&ax->d, ax->y1[ax->zoom]);
	ymax = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if(ymin > ymax) {
	    iy = ymin;
	    ymin = ymax;
	    ymax = iy;
	}

	if(ax->display_axes_labels != AXES_XY &&
		ax->display_axes_labels != AXES_X &&
           (a->top_title == NULL || (int)strlen(a->top_title) == 0) &&
           (a->x_axis_label == NULL || (int)strlen(a->x_axis_label) == 0))
        {
	    no_margins = True;
	}

	if(!a->log_y || ax->a.y_small_log || ax->a.y_medium_log)
	{
	    for(i = 0; i < a->nylab; i++) if(!a->ylab_off[i])
	    {
		y = a->y_lab[i];
		if(y >= ax->y1[ax->zoom] && y <= ax->y2[ax->zoom]) {
		    iy = a->y_ylab[i];
		    if(no_margins) {
			if(iy > ymax) {
			    iy = ymax;
			}
			else if(iy < ymin + a->axis_font_height) {
			    iy = ymin + a->axis_font_height;
			}
		    }
		    _AxesDrawString(w, ax->axesGC, a->x_ylab[i], iy,a->ylab[i]);
		}
	    }
	}
	else
	{
	    for(i = 0; i < a->nylab; i++) if(!a->ylab_off[i])
	    {
		y = a->y_lab[i];
		if(y >= ax->y1[ax->zoom] && y <= ax->y2[ax->zoom]) {
		    iy = a->y_ylab[i];
		    iy2 = a->y_ylab2[i];
		    if(no_margins) {
			if(iy > ymax) {
			    iy2 -= (iy-ymax);
			    iy = ymax;
			}
			else if(iy < ymin + a->axis_font_height) {
			    iy2 += (ymin + a->axis_font_height - iy);
			    iy = ymin + a->axis_font_height;
			}
		    }
		    if(a->ylab[i] != NULL && a->ylab[i][0] != '\0') {
			_AxesDrawString(w, ax->axesGC, a->x_ylab[i], iy, "10");
		    }
		    _AxesDrawString(w, ax->axesGC,a->x_ylab2[i],iy2,a->ylab[i]);
		}
	    }
	}
}

static void
drawOneYLabel(AxesWidget w, AxesPart *ax, int i)
{
	AxesParm *a = &ax->a;
	int iy, iy2, ymin, ymax;
	Boolean no_margins = False;

	if(!ax->draw_y_labels) return;

	ymin = unscale_y(&ax->d, ax->y1[ax->zoom]);
	ymax = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if(ymin > ymax) {
	    iy = ymin;
	    ymin = ymax;
	    ymax = iy;
	}

	if(ax->display_axes_labels != AXES_XY &&
		ax->display_axes_labels != AXES_X &&
           (a->top_title == NULL || (int)strlen(a->top_title) == 0) &&
           (a->x_axis_label == NULL || (int)strlen(a->x_axis_label) == 0))
        {
	    no_margins = True;
	}

	if(a->y_lab[i] >= ax->y1[ax->zoom] && a->y_lab[i] <= ax->y2[ax->zoom])
	{
	    if(!a->log_y || ax->a.y_small_log || ax->a.y_medium_log)
	    {
		iy = a->y_ylab[i];
		if(no_margins) {
		    if(iy > ymax) {
			iy = ymax;
		    }
		    else if(iy < ymin + a->axis_font_height) {
			iy = ymin + a->axis_font_height;
		    }
		}
		_AxesDrawString2(w, ax->axesGC, a->x_ylab[i], iy, a->ylab[i]);
	    }
	    else
	    {
		iy = a->y_ylab[i];
		iy2 = a->y_ylab2[i];
		if(no_margins) {
		    if(iy > ymax) {
			iy2 -= (iy-ymax);
			iy = ymax;
		    }
		    else if(iy < ymin + a->axis_font_height) {
			iy2 += (ymin + a->axis_font_height - iy);
			iy = ymin + a->axis_font_height;
		    }
		}
		if(a->ylab[i] != NULL && a->ylab[i][0] != '\0') {
		    _AxesDrawString2(w, ax->axesGC, a->x_ylab[i], iy, "10");
		}
		_AxesDrawString2(w, ax->axesGC, a->x_ylab2[i], iy2, a->ylab[i]);
	    }
	}
}
static void
drawXLabels(AxesWidget w, AxesPart *ax)
{
	int i;
	double x;

	if(!ax->a.log_x || ax->a.x_small_log || ax->a.x_medium_log)
	{
	    for(i = 0; i < ax->a.nxlab; i++) if(!ax->a.xlab_off[i])
	    {
		x = ax->a.x_lab[i];
		if(x >= ax->x1[ax->zoom] && x <= ax->x2[ax->zoom]) {
		    _AxesDrawString(w, ax->axesGC, ax->a.x_xlab[i], 
			ax->a.y_xlab + ax->a.xlab_ascent[i], ax->a.xlab[i]);
		}
	    }
	}
	else
	{
	    int y;
	    for(i = 0; i < ax->a.nxlab; i++) if(!ax->a.xlab_off[i])
	    {
		x = ax->a.x_lab[i];
		if(x >= ax->x1[ax->zoom] && x <= ax->x2[ax->zoom]) {
		    if(ax->a.xlab[i] != NULL && ax->a.xlab[i][0] != '\0') {
			y = (int)(ax->a.y_xlab + 1.6*ax->a.xlab_ascent[i]);
			_AxesDrawString(w, ax->axesGC, ax->a.x_xlab2[i], y,
					"10");
		    }
		    y = (int)(ax->a.y_xlab + .8*ax->a.xlab_ascent[i]);
		    _AxesDrawString(w, ax->axesGC, ax->a.x_xlab[i], y,
				ax->a.xlab[i]);
		}
	    }
	}
}

int AxesClass::getXLabels(double **values, char ***labels)
{
    AxesPart *ax = &aw->axes;
    int i, n;

    *values = NULL;
    *labels = NULL;
    if( ax->a.nxlab == 0 ) return 0;

    n = ax->a.nxlab;
    *values = (double *)AxesMalloc((Widget)aw, n*sizeof(double));
    *labels = (char **)AxesMalloc((Widget)aw, n*sizeof(char *));
    for(i = 0; i < n; i++) {
	(*values)[i] = ax->a.x_lab[i];
	(*labels)[i] = strdup(ax->a.xlab[i]);
    }
    return n;
}

static void
drawOneXLabel(AxesWidget w, AxesPart *ax, int i)
{
	if(ax->a.x_lab[i] >= ax->x1[ax->zoom]
		&& ax->a.x_lab[i] <= ax->x2[ax->zoom])
	{
	    if(!ax->a.log_x || ax->a.x_small_log || ax->a.x_medium_log)
	    {
	 	_AxesDrawString2(w, ax->axesGC, ax->a.x_xlab[i], 
			ax->a.y_xlab + ax->a.xlab_ascent[i], ax->a.xlab[i]);
	    }
	    else
	    {
		int y;
		if(ax->a.xlab[i] != NULL && ax->a.xlab[i][0] != '\0') {
		    y = (int)(ax->a.y_xlab + 1.6*ax->a.xlab_ascent[i]);
		    _AxesDrawString2(w, ax->axesGC, ax->a.x_xlab2[i], y, "10");
		}
		y = (int)(ax->a.y_xlab + .8*ax->a.xlab_ascent[i]);
		_AxesDrawString2(w, ax->axesGC, ax->a.x_xlab[i], y,
			ax->a.xlab[i]);
	    }
	}
}

static void
Resize(Widget widget)
{
	AxesWidget w = (AxesWidget)widget;
	AxesPart *ax = &w->axes;
	int	ymar, iy1, iy2;
	double	ydif;

	if(!XtIsRealized((Widget)w)) return;

	if(ax->pixmap != (Pixmap)NULL)
	{
		XFreePixmap(XtDisplay(w), ax->pixmap);
		ax->pixmap = (Pixmap)NULL;
	}
	
	if(ax->use_pixmap)
	{
		/* important to create pixmap with (width+1,height+1) for calls
		 * to XFillRectangle(... width, height)
		 * Otherwise get bad problems with some X-servers
		 */
		ax->pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
				w->core.width+1, w->core.height+1, ax->depth);
		if(ax->pixmap == (Pixmap)NULL)
		{
			fprintf(stderr, "%s: XCreatePixmap failed.\n",
					w->core.name);
			ax->use_pixmap = False;
		}
		else
		{
			_AxesClearArea(w, 0, 0, w->core.width, w->core.height);
		}
	}

	if(!ax->auto_y_scale)
	{
		iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
		iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);

		ymar = ax->last_height - abs(iy1 - iy2);
		if(ymar < 0.) ymar = 0;
		if(ymar > (int)w->core.height) ymar = 0;
		ydif = ax->y2[ax->zoom] - ax->y1[ax->zoom];

		ydif *= (float)((int)w->core.height-ymar)/
			(float)(ax->last_height-ymar);
		ax->y2[ax->zoom] = ax->y1[ax->zoom] + ydif;
	}
	if(!ax->a.auto_x)
	{
		ax->a.right = w->core.width - ax->rt_margin;
	}
	if(!ax->a.auto_y)
	{
		ax->a.bottom = w->core.height - ax->bm_margin;
	}
	if(ax->resize_func != NULL)
	{
		(*(ax->resize_func))(w);
	}

	ax->skip_limits_callback = True;
	Redraw(w);
	ax->skip_limits_callback = False;
	
	if(ax->use_pixmap)
	{
		_AxesRedisplayAxes(w);
		if(ax->redraw_data_func != NULL)
		{
			(*(ax->redraw_data_func))(w, DATA_REDISPLAY, False);
		}
		_AxesRedisplayXor(w);
	}
	ax->last_height = w->core.height;
	ax->last_width = w->core.width;
	XtCallCallbacks(widget, XtNresizeCallback, NULL);
}

static XtGeometryResult
GeometryManager(
	Widget 			w,
	XtWidgetGeometry	*request,
	XtWidgetGeometry	*reply)
{
	return XtGeometryNo;
}

static void
ChangeManaged(Widget w)
{
	DoLayout((AxesWidget)w);
}

static void
DoLayout(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	double xmin, xmax, ymin, ymax;

	Rectangle r = _AxesLayout(w);
	GetScale(&ax->d, &xmin, &ymin, &xmax, &ymax);
	SetDrawArea(&ax->d, r.x, r.y, r.x + r.width - 1, r.y - r.height + 1);
	SetScale(&ax->d, xmin, ymin, xmax, ymax);
}

/** 
 *  
 */
Rectangle
_AxesLayout(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	Rectangle r = {0, 0, 1, 1};
	Boolean display_hbar;
	Boolean display_vbar;

	display_vbar = True;
	if(ax->scrollbar_as_needed) {
	    int i;
	    display_vbar = False;
	    for(i = 0; i <= ax->zoom; i++) {
		if(ax->y1[i] != ax->y1[0] || ax->y2[i] != ax->y2[0]) break;
	    }
	    if(i <= ax->zoom) display_vbar = True;
	}

	if(ax->vertical_scroll && display_vbar)
	{
	    if(ax->vertical_scroll_position == LEFT) {
		r.x = ax->left_margin + ax->vert_scroll->core.width + MARGIN;
		r.width = w->core.width - ax->right_margin - r.x;
	    }
	    else {
		r.x = ax->left_margin + 10;
		r.width = w->core.width - ax->right_margin - r.x
				- ax->vert_scroll->core.width;
	    }
	    if(ax->vert_scroll && !XtIsManaged(ax->vert_scroll)) {
		 XtManageChild(ax->vert_scroll);
	    }
	}
	else {
	    if(ax->vert_scroll && XtIsManaged(ax->vert_scroll)) {
		 XtUnmanageChild(ax->vert_scroll);
	    }
	    r.x = ax->left_margin;
	    r.width = w->core.width - ax->right_margin - r.x;
	}
	display_hbar = (ax->horizontal_scroll &&
		(!ax->scrollbar_as_needed || ax->zoom > 0)) ? True : False;
	if(!display_hbar) {
	    if(ax->hor_scroll && XtIsManaged(ax->hor_scroll)) {
		 XtUnmanageChild(ax->hor_scroll);
	    }
	}
	else {
	    if(ax->hor_scroll && !XtIsManaged(ax->hor_scroll)) {
		 XtManageChild(ax->hor_scroll);
	    }
	}

	r.y = ax->top_margin;

	if(ax->display_x_label) {
	    r.height = (display_hbar) ?  w->core.height -
		ax->bottom_margin - ax->hor_scroll->core.height - MARGIN :
		w->core.height - ax->bottom_margin;
	}
	else {
	    r.height = (display_hbar) ?  w->core.height -
		ax->bottom_margin - ax->hor_scroll->core.height - 8 :
		w->core.height - ax->bottom_margin - 8;
	}
	if(ax->display_axes_labels != AXES_XY &&
		ax->display_axes_labels != AXES_X)
	{
	    if(ax->cursor_label_always) {
		int ascent, descent, direction;
		XCharStruct overall;
		XTextExtents(ax->font, "0123", 4, &direction, &ascent,
				&descent, &overall);
		r.height = w->core.height - overall.ascent - overall.descent -4;
		r.y = w->core.height - 1;
	    }
	    else {
		// This should be fixed in libgplot/DrawAxis.c instead of here.
		r.height = w->core.height-2;
		r.y = r.y + r.height - 1;
	    }
	}
	else {
	    r.y = r.y + r.height - 1;
	}
	return r;
}

static void
Destroy(Widget widget)
{
	AxesWidget w = (AxesWidget)widget;
	AxesPart *ax = &w->axes;
	int i;

	if(ax->axesGC != NULL) XFreeGC(XtDisplay(w), ax->axesGC);
	if(ax->xorGC != NULL) XFreeGC(XtDisplay(w), ax->xorGC);
	if(ax->selectGC != NULL) XFreeGC(XtDisplay(w), ax->selectGC);
	if(ax->invertGC != NULL) XFreeGC(XtDisplay(w), ax->invertGC);
	if(ax->invert_clipGC != NULL) XFreeGC(XtDisplay(w), ax->invert_clipGC);
	if(ax->mag_invertGC != NULL) XFreeGC(XtDisplay(w), ax->mag_invertGC);
	if(ax->eraseGC != NULL) XFreeGC(XtDisplay(w), ax->eraseGC);

	if(ax->pixmap != (Pixmap)NULL) {
	    XFreePixmap(XtDisplay(w), ax->pixmap);
	}
	if(ax->y_label_image != (XImage *)NULL) {
	    XDestroyImage(ax->y_label_image);
	}
	for(i = 0; i < MAXLAB; i++) {
	    Free(ax->a.xlab[i]);
	    Free(ax->a.ylab[i]);
	}
	Free(ax->a.top_title);
	Free(ax->a.x_axis_label);
	Free(ax->a.y_axis_label);
	for(i = 0; i < 4; i++) {
	    Free(ax->a.axis_segs[i].segs);
	}

	Free(ax->err_msg);
	Free(ax->a.top_title);
        Free(ax->a.x_axis_label);
        Free(ax->a.y_axis_label);
	if(ax->width_form != NULL) {
	   XtDestroyWidget(ax->width_form);
	}
	if(ax->width_text != NULL) {
	   XtDestroyWidget(ax->width_text);
	}
//	XFreeFont(XtDisplay(w), ax->font);

	for(i = 0; i < MAX_RECTS; i++)
	{
	    Free(ax->ovy_grp[i].ovr_scaled_x1);
	    Free(ax->ovy_grp[i].ovr_scaled_x2);
	    Free(ax->ovy_grp[i].ovr_scaled_y1);
	    Free(ax->ovy_grp[i].ovr_scaled_y2);
	    Free(ax->ovy_grp[i].display);
	}
	Free(ax->time_marks);
}

static void
flip_xy(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	char *s;
	int i;
	double d;

	if(ax->x_axis == LEFT)
	{
		ax->reverse_x = False;
		ax->reverse_y = True;
	}
	else
	{
		ax->reverse_y = True;
		ax->reverse_x = False;
	}
	for(i = 0; i <= ax->zoom; i++)
	{
		d = ax->x1[i];
		ax->x1[i] = ax->y1[i];
		ax->y1[i] = d;
		d = ax->x2[i];
		ax->x2[i] = ax->y2[i];
		ax->y2[i] = d;
	}

	s = ax->a.x_axis_label;
	ax->a.x_axis_label = ax->a.y_axis_label;
	ax->a.y_axis_label = s;

#ifndef NO_IMAGE
	if(XtIsRealized((Widget)w)) {
	    if(ax->y_label_image != (XImage *)NULL)
	    {
		XDestroyImage(ax->y_label_image);
		ax->y_label_image = (XImage *)NULL;
	    }
	    if((int)strlen(ax->a.y_axis_label) > 0)
	    {
		ax->y_label_image = GetStrImage(XtDisplay(w), XtWindow(w),
					ax->axesGC, ax->font, ax->eraseGC,
					ax->a.y_axis_label,
					(int)strlen(ax->a.y_axis_label));
	    }
	}
#endif
}

/** 
 *  
 */
void
_AxesMagnify(AxesWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	if(!ax->zoom_controls)
	{
		return;
	}
	AxesWaitCursor((Widget)w, True);
	ax->magnify = True;

	_AxesZoom(w, event, (const char **)params, num_params);
	AxesWaitCursor((Widget)w, False);
}

/** 
 *  
 */
void
_AxesZoom(AxesWidget w, XEvent *event, const char **params, Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int cursor_x, cursor_y, rect_x, rect_y, x1, y1, x2, y2;
	int delx, dely;
	GC gc;
 
	ax->hor_only = (*num_params > 0 && !strcmp(params[0],"horizontal"))
				? True : False;
	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(ax->cursor_mode == AXES_SELECT_CURSOR) {
	    if(ax->hor_only) {
		_AxesScale(w, event, (char **)params, num_params);
	    }
#ifdef _MIDDLE_MOUSE_POPUP_
	    else {
		doCursorPopup(w, cursor_x, cursor_y, event);
	    }
#endif
	    return;
	}

	if(!ax->zoom_controls) {
	    return;
	}

	if(ax->magnify_only && event->type == ButtonPress) {
	    ax->magnify = True;
	}

	gc = (ax->magnify) ? ax->mag_invertGC : ax->invert_clipGC;
	if(event->type == ButtonPress)
	{
/*
	    if(ax->magnify && ax->mag_to == NULL)
*/
	    if(ax->magnify && XtHasCallbacks((Widget)w, XtNmagnifyCallback)
			== XtCallbackHasNone)
	    {
		ax->magnify = False;
		ax->zooming = False;
		AxesWaitCursor((Widget)w, False);
		return;
	    }
	    if(!ax->magnify)
	    {
		if(ax->undraw)
		{
		    ax->axes_class->drawRectangle(ax->invert_clipGC, 
					ax->zoom_x1, ax->zoom_y1,
					ax->zoom_x2 - ax->zoom_x1, 
					ax->zoom_y2 - ax->zoom_y1);
		}
	    }
	    else {
		_AxesRedisplayMagRect(w);
	    }
	    ax->anchor_x = cursor_x;
	    ax->anchor_y = (ax->hor_only) ? unscale_y(&ax->d, ax->y1[ax->zoom])
					: cursor_y;
	    ax->undraw = False;
	    ax->zooming = True;
	    ax->no_motion = True;
	}
	else if(event->type == MotionNotify && ax->zooming)
	{
	    if(ax->hor_only) cursor_y = unscale_y(&ax->d, ax->y2[ax->zoom]);

	    if(ax->undraw) {
		rect_x = (ax->anchor_x < ax->last_x) ? ax->anchor_x :ax->last_x;
		rect_y = (ax->anchor_y < ax->last_y) ? ax->anchor_y :ax->last_y;
		ax->axes_class->drawRectangle(gc, rect_x, rect_y,
			abs(ax->anchor_x - ax->last_x),
			abs(ax->anchor_y - ax->last_y));
	    }
	    if(ax->a.uniform_scale)
	    {
		delx = abs((ax->d.ix2 - ax->d.ix1)*(cursor_y -  ax->anchor_y)/
				(ax->d.iy2 - ax->d.iy1));
		if(delx > abs(cursor_x - ax->anchor_x))
		{
		    if(cursor_x < ax->anchor_x) delx = -delx;
		    cursor_x = ax->anchor_x + delx;
		}
		else
		{
		    dely = abs((ax->d.iy2 - ax->d.iy1)*(cursor_x -  ax->anchor_x)/
					(ax->d.ix2 - ax->d.ix1));
		    if(cursor_y < ax->anchor_y) dely = -dely;
		    cursor_y = ax->anchor_y + dely;
		}
	    }

	    rect_x = (ax->anchor_x < cursor_x) ? ax->anchor_x : cursor_x;
	    rect_y = (ax->anchor_y < cursor_y) ? ax->anchor_y : cursor_y;
	    ax->axes_class->drawRectangle(gc, rect_x, rect_y, 
			abs(ax->anchor_x-cursor_x),abs(ax->anchor_y-cursor_y));
	    ax->undraw = True;
	    ax->no_motion = False;
	}
	else if(event->type == ButtonRelease && ax->zooming)
	{
	    AxesWaitCursor((Widget)w, True);
	    if(ax->no_motion)
	    {
		if(ax->magnify) {
		    _AxesRedisplayMagRect(w);
		}
		else if(!ax->hor_only) {
		    ax->axes_class->unzoom();
		}
		ax->undraw = False;
		ax->zooming = False;
		ax->magnify = False;
		AxesWaitCursor((Widget)w, False);
		return;
	    }
	    if(ax->anchor_x < ax->last_x) {
		x1 = ax->anchor_x;
		x2 = ax->last_x;
	    }
	    else {
		x1 = ax->last_x;
		x2 = ax->anchor_x;
	    }
	    if(ax->anchor_y > ax->last_y) {
		y1 = ax->anchor_y;
		y2 = ax->last_y;
	    }
	    else {
		y1 = ax->last_y;
		y2 = ax->anchor_y;
	    }
	    if(x1 == x2 || y1 == y2 || (abs(x1-x2) < 5 && abs(y1-y2) < 5))
	    {
		rect_x = (ax->anchor_x < ax->last_x) ? ax->anchor_x :ax->last_x;
		rect_y = (ax->anchor_y < ax->last_y) ? ax->anchor_y :ax->last_y;
		ax->axes_class->drawRectangle(gc, rect_x, rect_y,
			abs(ax->anchor_x - ax->last_x),
			abs(ax->anchor_y - ax->last_y));
		if(ax->magnify) {
		    _AxesRedisplayMagRect(w);
		}
		else if(!ax->hor_only) {
		    ax->axes_class->unzoom();
		}
		ax->undraw = False;
		ax->zooming = False;
		ax->magnify = False;
	    }
	    else if(!ax->magnify)
	    {
		ax->zoom_x1 = x1;
		ax->zoom_y1 = y2;
		ax->zoom_x2 = x2;
		ax->zoom_y2 = y1;
		if(XtHasCallbacks((Widget)w,XtNzoomCallback)==XtCallbackHasNone)
		{
		    AxesZoomApply(w,  0);
		}
		else {
		    XtCallCallbacks((Widget)w, XtNzoomCallback, NULL);
		}
	    }
	    else
	    {
		XtCallCallbacks((Widget)w, XtNmagnifyCallback, NULL);
		AxesMagApply(w, x1, x2, y2, y1);
		ax->magnify_rect = True;
		ax->undraw = False;
		ax->zooming = False;
	    }
	    ax->magnify = False;
	    AxesWaitCursor((Widget)w, False);
	}
	ax->last_x = cursor_x;
	ax->last_y = cursor_y;
}

void AxesClass::showMagWindow()
{
    AxesPart *ax = &aw->axes;
    double xmin, xmax, ymin, ymax;
    double mag_x1, mag_y1, mag_x2, mag_y2;

    xmin = ax->x1[ax->zoom];
    xmax = ax->x2[ax->zoom];
    ymin = ax->y1[ax->zoom];
    ymax = ax->y2[ax->zoom];
    mag_x1 = xmin + .25*(xmax-xmin);
    mag_y1 = ymin + .25*(ymax-ymin);
    mag_x2 = mag_x1 + .15*(xmax-xmin);
    mag_y2 = mag_y1 + .15*(ymax-ymin);
    setMagnifyWindow(mag_x1, mag_x2, mag_y1, mag_y2);
}

void AxesClass::setMagnifyWindow(double x_min, double x_max, double y_min,
		double y_max)
{
    AxesPart *ax = &aw->axes;
    int m, mag_x1, mag_x2, mag_y1, mag_y2, width, height;

    mag_x1 = unscale_x(&ax->d, x_min);
    mag_x2 = unscale_x(&ax->d, x_max);
    if(mag_x1 > mag_x2) {
	m = mag_x1;
	mag_x1 = mag_x2;
	mag_x2 = m;
    }
    mag_y1 = unscale_y(&ax->d, y_min);
    mag_y2 = unscale_y(&ax->d, y_max);
    if(mag_y1 > mag_y2) {
	m = mag_y1;
	mag_y1 = mag_y2;
	mag_y2 = m;
    }
    width = mag_x2 - mag_x1;
    height = mag_y2 - mag_y1;

    _AxesRedisplayMagRect(aw);
    drawRectangle(ax->mag_invertGC, mag_x1, mag_y1, width, height);

    XtCallCallbacks((Widget)aw, XtNmagnifyCallback, NULL);
    AxesMagApply(aw, mag_x1, mag_x2, mag_y1, mag_y2);
    ax->magnify_rect = True;
    ax->undraw = False;
    ax->zooming = False;
}

#ifdef _MIDDLE_MOUSE_POPUP_
static Widget double_line_popup = NULL;
static Widget cursor_popup = NULL;
static Widget delete_button, delete_button2;
static Widget set_width_button;

static void
doCursorPopup(AxesWidget w, int cursor_x, int cursor_y, XEvent *event)
{
	AxesPart *ax = &w->axes;
	float distance;
	int i;

	if((ax->m = _AxesWhichCursor(w, cursor_x, cursor_y, &distance,&i)) >= 0)
	{
	    Widget toplevel;

	    if(ax->cursor[ax->m].type == AXES_VER_DOUBLE_LINE)
	    {
		if(double_line_popup == NULL)
		{
		    int n;
		    Arg args[2];
		    XtTranslations translations;
		    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

		    /* the default is button3, which will inactivate button3
		     * in the AxesWidget, so set to button5
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
		    double_line_popup = XmCreatePopupMenu(toplevel,
				(char *)"popupMenu", args, n);
		    translations = XtParseTranslationTable(trans);
		    delete_button = XtVaCreateManagedWidget("Delete Cursor",
				xmPushButtonWidgetClass, double_line_popup,
				XmNtranslations, translations,
				NULL);
		    set_width_button = XtVaCreateManagedWidget("Set Width",
				xmPushButtonWidgetClass, double_line_popup,
				XmNtranslations, translations,
				NULL);
		}
		XtRemoveAllCallbacks(delete_button, XmNactivateCallback);
		XtAddCallback(delete_button, XmNactivateCallback, AxesDeleteCB,
				(XtPointer)w);
		XtRemoveAllCallbacks(set_width_button, XmNactivateCallback);
		XtAddCallback(set_width_button, XmNactivateCallback,AxesWidthCB,
				(XtPointer)w);

		XmMenuPosition(double_line_popup, (XButtonPressedEvent *)event);
		XtManageChild(double_line_popup);
	    }
	    else
	    {
		if(cursor_popup == NULL)
		{
		    int n;
		    Arg args[2];
		    XtTranslations translations;
		    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

		    /* the default is button3, which will inactivate button3
		     * in the AxesWidget, so set to 6, which is > all buttons
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
		    cursor_popup = XmCreatePopupMenu(toplevel,
					(char *)"popupMenu", args, n);
		    translations = XtParseTranslationTable(trans);
		    delete_button2 = XtVaCreateManagedWidget("Delete Cursor",
				xmPushButtonWidgetClass, cursor_popup,
				XmNtranslations, translations,
				NULL);
		}
		XtRemoveAllCallbacks(delete_button2, XmNactivateCallback);
		XtAddCallback(delete_button2, XmNactivateCallback, AxesDeleteCB,
				(XtPointer)w);

		XmMenuPosition(cursor_popup, (XButtonPressedEvent *)event);
		XtManageChild(cursor_popup);
	    }
	}
}

static void
AxesDeleteCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	AxesWidget w = (AxesWidget)client_data;
	AxesPart *ax = &w->axes;

	if(ax->m >= 0)
        {
	    DeleteCursor(w, ax->m, False);
	    ax->m = -1;
	}
	_AxesSetCursor(w, "default");
	ax->cursor_mode = AXES_ZOOM;
}

static void
destroyWidthForm(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	AxesPart *ax = (AxesPart *)client_data;

	ax->width_text = NULL;
}

static void
AxesWidthCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	AxesWidget w = (AxesWidget)client_data;
	AxesPart *ax = &w->axes;
	XmString xm;
	Arg args[1];
	char text[50];
	AxesCursor *c;
	Widget label, rc, button;
	int n;

	if(ax->m < 0) return;
	c = &ax->cursor[ax->m];
	ftoa(c->scaled_x2 - c->scaled_x1, c->nxdeci+4, 0, text, 50);

	if(ax->width_form != NULL) {
	    XtDestroyWidget(ax->width_form);
	}

	xm = XmStringCreateLtoR((char *)"Cursor Width", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg(args[n], XmNdialogTitle, xm);
	ax->width_form = XmCreateFormDialog(w->core.parent,
				(char *)"Cursor Width", args, 1);
	XtAddCallback(ax->width_form, XtNdestroyCallback, destroyWidthForm,
                        (XtPointer)ax);

	XmStringFree(xm);
	xm = XmStringCreateLtoR((char *)"Cursor Width (secs)",
			XmSTRING_DEFAULT_CHARSET);

	label = XtVaCreateManagedWidget("label", xmLabelWidgetClass,
			ax->width_form,
			XmNlabelString, xm,
			XmNtopAttachment, XmATTACH_FORM,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			NULL);
	XmStringFree(xm);

	ax->width_text = XtVaCreateManagedWidget("text", xmTextWidgetClass,
			ax->width_form,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, label,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			XmNcolumns, 10,
			XmNvalue, text,
			NULL);

	rc = XtVaCreateManagedWidget("text", xmRowColumnWidgetClass,
			ax->width_form,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, ax->width_text,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			XmNorientation, XmHORIZONTAL,
			NULL);
	button = XtVaCreateManagedWidget("Apply",
				xmPushButtonWidgetClass, rc, NULL);
	XtAddCallback(button, XmNactivateCallback, WidthApplyCB, (XtPointer)w);
	button = XtVaCreateManagedWidget("Cancel",
				xmPushButtonWidgetClass, rc, NULL);
	XtAddCallback(button, XmNactivateCallback, WidthCancelCB, (XtPointer)w);
	XtManageChild(ax->width_form);
}

static void
WidthApplyCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	AxesWidget w = (AxesWidget)client_data;
	AxesPart *ax = &w->axes;
	double width;
	char *text;

	text = XmTextGetString(ax->width_text);

	if(stringToDouble(text, &width) && ax->m >= 0)
	{
	    ClearCursor(w, ax->m);
	    ax->cursor[ax->m].scaled_x2 = ax->cursor[ax->m].scaled_x1 + width;
	    DrawCursor(w, ax->m);
	}
	XtFree(text);
	XtDestroyWidget(ax->width_form);
	ax->width_form = NULL;
}

static void
WidthCancelCB(Widget widget, XtPointer client_data, XtPointer callback_data)
{
	AxesWidget w = (AxesWidget)client_data;
	AxesPart *ax = &w->axes;
	XtDestroyWidget(ax->width_form);
	ax->width_form = NULL;
}

#endif

/** 
 *  
 */
void
_AxesZoomBack(AxesWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int i;

	AxesWaitCursor((Widget)w, True);
	if(ax->zoom < ax->zoom_max)
	{
	    i = ax->zoom + 1;
	    ax->zoom_x1 = unscale_x(&ax->d, ax->x1[i]);
	    ax->zoom_x2 = unscale_x(&ax->d, ax->x2[i]);
	    ax->zoom_y1 = unscale_y(&ax->d, ax->y2[i]);
	    ax->zoom_y2 = unscale_y(&ax->d, ax->y1[i]);
	    ax->zooming = True;
	    i = ax->zoom_max;
	    AxesZoomApply(w, 0);
	    ax->zoom_max = i;
	}
	AxesWaitCursor((Widget)w, False);
}

void
AxesZoomApply(AxesWidget w, int mode)	  /* 0 for zoom, 1 for select */
{
	AxesPart *ax = &w->axes;
	if(!ax->zooming) {
	    return;
	}
	if(ax->undraw) {
	    ax->axes_class->drawRectangle(ax->invert_clipGC,
			ax->zoom_x1, ax->zoom_y1,
			ax->zoom_x2 - ax->zoom_x1,
			ax->zoom_y2 - ax->zoom_y1);
	    ax->undraw = False;
	}
	if(!mode)
	{
	    if(ax->axes_class->doZoom(ax->zoom_x1, ax->zoom_y1,
			ax->zoom_x2, ax->zoom_y2))
	    {
		Redraw(w);
		Redisplay((Widget)w, NULL, NULL);
		DoMagFrom(w);
	    }
	}
	else
	{
	    if(ax->select_data_func != NULL) {
		(*(ax->select_data_func))(w);
	    }
	}
	ax->zooming = False;
}

void AxesClass::mag(double x_min, double x_max, double y_min, double y_max)
{
    AxesPart *ax = &aw->axes;
    int mag_x1, mag_x2, mag_y1, mag_y2;

    if(!ax->magnify_rect) return;

    mag_x1 = unscale_x(&ax->d, x_min);
    mag_x2 = unscale_x(&ax->d, x_max);
    mag_y1 = unscale_y(&ax->d, y_min);
    mag_y2 = unscale_y(&ax->d, y_max);

    _AxesRedisplayMagRect(aw);
    AxesMagApply(aw, mag_x1, mag_x2, mag_y1, mag_y2);
    _AxesRedisplayMagRect(aw);
}

static void
AxesMagApply(AxesWidget w, int mag_x1, int mag_x2, int mag_y1, int mag_y2)
{
	AxesPart *ax = &w->axes;
	int i, ix1, ix2, iy1, iy2;
	AxesWidget z;
	double sx1, sx2, sy1, sy2;

	if((z = (AxesWidget)ax->mag_to) == NULL) {
	    return;
	}

	ix1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
	ix2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
	iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
	iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);
	/*
	 * Don't allow mag limits to be outside of current limits.
	 */
	if(ix1 > ix2) {
	    i = ix1;
	    ix1 = ix2;
	    ix2 = i;
	}
	if(iy1 > iy2) {
	    i = iy1;
	    iy1 = iy2;
	    iy2 = i;
	}
	if(mag_x1 < ix1)      mag_x1 = ix1;
	else if(mag_x1 > ix2) mag_x1 = ix2;
	if(mag_x2 < ix1)      mag_x2 = ix1;
	else if(mag_x2 > ix2) mag_x2 = ix2;
	if(mag_y1 < iy1)      mag_y1 = iy1;
	else if(mag_y1 > iy2) mag_y1 = iy2;
	if(mag_y2 < iy1)      mag_y2 = iy1;
	else if(mag_y2 > iy2) mag_y2 = iy2;

	if(mag_x1 == mag_x2 || mag_y1 == mag_y2) return;

	sx1 = scale_x(&ax->d, mag_x1);
	sx2 = scale_x(&ax->d, mag_x2);
	sy1 = scale_y(&ax->d, mag_y1);
	sy2 = scale_y(&ax->d, mag_y2);
	ax->mag_scaled_x1 = (sx1 < sx2) ? sx1 : sx2;
	ax->mag_scaled_x2 = (sx1 < sx2) ? sx2 : sx1;
	ax->mag_scaled_y1 = (sy1 < sy2) ? sy1 : sy2;
	ax->mag_scaled_y2 = (sy1 < sy2) ? sy2 : sy1;

	z->axes.x1[0] = w->axes.x1[0];
	z->axes.x2[0] = w->axes.x2[0];
	z->axes.y1[0] = w->axes.y1[0];
	z->axes.y2[0] = w->axes.y2[0];
	z->axes.x1[2] = ax->mag_scaled_x1;
	z->axes.x2[2] = ax->mag_scaled_x2;
	z->axes.y1[2] = ax->mag_scaled_y1;
	z->axes.y2[2] = ax->mag_scaled_y2;
	z->axes.zoom = 2;
	z->axes.zoom_max = 2;
	Redraw(z);
	Redisplay((Widget)z, NULL, NULL);
}

void AxesClass::zoomCancel()
{
    AxesPart *ax = &aw->axes;
    if(ax->zooming)
    {
	if(ax->undraw)
	{
	    drawRectangle(ax->invert_clipGC,ax->zoom_x1, ax->zoom_y1,
			ax->zoom_x2 - ax->zoom_x1, ax->zoom_y2 - ax->zoom_y1);
	    ax->undraw = False;
	}
	ax->zooming = False;
    }
}

void AxesClass::getLimits(double *xmin, double *xmax, double *ymin,double *ymax)
{
    AxesPart *ax = &aw->axes;
    *xmin = ax->x1[ax->zoom];
    *ymin = ax->y1[ax->zoom];
    *xmax = ax->x2[ax->zoom];
    *ymax = ax->y2[ax->zoom];
}

void AxesClass::getMinLimits(double *xmin, double *xmax, double *ymin,
		double *ymax)
{
    AxesPart *ax = &aw->axes;
    *xmin = ax->x1[ax->zoom_min];
    *ymin = ax->y1[ax->zoom_min];
    *xmax = ax->x2[ax->zoom_min];
    *ymax = ax->y2[ax->zoom_min];
}

void AxesClass::setMinLimits(double xmin, double xmax, double ymin, double ymax)
{
    AxesPart *ax = &aw->axes;

    ax->zoom = ax->zoom_min;
    if(fabs(xmax-xmin) > ax->delx_min && fabs(ymax-ymin) > ax->dely_min) {
	ax->x1[ax->zoom] = xmin;
	ax->y1[ax->zoom] = ymin;
	ax->x2[ax->zoom] = xmax;
	ax->y2[ax->zoom] = ymax;
	Redraw(aw);
	axesExpose(true);
    }
}

void AxesClass::zoom(double xmin, double xmax, double ymin, double ymax,
			bool zoom_up)
{
    AxesPart *ax = &aw->axes;
    int ix1, ix2, iy1, iy2;
    int cur_ix1, cur_ix2, cur_iy1, cur_iy2;

    cur_ix1 = unscale_x(&ax->d, ax->x_min);
    cur_ix2 = unscale_x(&ax->d, ax->x_max);
    cur_iy1 = unscale_y(&ax->d, ax->y_min);
    cur_iy2 = unscale_y(&ax->d, ax->y_max);
		
    ix1 = unscale_x(&ax->d, xmin);
    ix2 = unscale_x(&ax->d, xmax);
    iy1 = unscale_y(&ax->d, ymin);
    iy2 = unscale_y(&ax->d, ymax);

    if(ix1 == cur_ix1 && ix2 == cur_ix2 && iy1 == cur_iy1 && iy2 == cur_iy2) {
	return;
    }
    if(!zoom_up) {
	ax->zoom = ax->zoom_min;
    }

    if(doZoom(ix1, iy1, ix2, iy2)) {
	Redraw(aw);
	Redisplay((Widget)aw, NULL, NULL);
	DoMagFrom(aw);
    }
}

void AxesClass::setMinZoomLevel(int min_zoom_level)
{
    if(aw->axes.zoom >= min_zoom_level) {
	aw->axes.min_zoom_level = min_zoom_level;
    }
}

int AxesClass::getZoomLevel()
{
    AxesPart *ax = &aw->axes;
    return ax->zoom - ax->zoom_min;
}

int AxesClass::getZooms(int *zoom_min, double **xmin, double **xmax,
		double **ymin, double **ymax)
{
    AxesPart *ax = &aw->axes;

    *xmin = (double *)AxesMalloc((Widget)aw, (ax->zoom+1)*sizeof(double));
    *xmax = (double *)AxesMalloc((Widget)aw, (ax->zoom+1)*sizeof(double));
    *ymin = (double *)AxesMalloc((Widget)aw, (ax->zoom+1)*sizeof(double));
    *ymax = (double *)AxesMalloc((Widget)aw, (ax->zoom+1)*sizeof(double));

    for(int i = 0; i <= ax->zoom; i++) {
	(*xmin)[i] = ax->x1[i];
	(*xmax)[i] = ax->x2[i];
	(*ymin)[i] = ax->y1[i];
	(*ymax)[i] = ax->y2[i];
    }
    *zoom_min = ax->zoom_min;
    return ax->zoom+1;
}

void AxesClass::setZooms(int zoom_min, int num, double *xmin, double *xmax,
		double *ymin, double *ymax)
{
    AxesPart *ax = &aw->axes;
    int i;

    if(num > MAX_ZOOM) num = MAX_ZOOM;
    if(num <= 0) return;
    if(zoom_min < 0 ||zoom_min >= num) zoom_min = 0;

    for(i = 0; i < num; i++) {
	ax->x1[i] = xmin[i];
	ax->x2[i] = xmax[i];
	ax->y1[i] = ymin[i];
	ax->y2[i] = ymax[i];
    }
    ax->zoom_min = zoom_min;
    ax->zoom = zoom_min;
    Redraw(aw);
    Redisplay((Widget)aw, NULL, NULL);
}

bool AxesClass::doZoom(int ix1, int iy1, int ix2, int iy2)
{
    AxesPart *ax = &aw->axes;
    int i, lx1, ly1, lx2, ly2;
    double x1 = 0.0,  y1 = 0.0, x2 = 0.0, y2 = 0.0;

    lx1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
    lx2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
    ly1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
    ly2 = unscale_y(&ax->d, ax->y2[ax->zoom]);
    /*
     * Don't allow zoom limits to be outside of current level limits.
     */
    if(lx1 > lx2) {
	i = lx1;
	lx1 = lx2;
	lx2 = i;
    }
    if(ly1 > ly2) {
	i = ly1;
	ly1 = ly2;
	ly2 = i;
    }
    if(ix1 < lx1)      ix1 = lx1;
    else if(ix1 > lx2) ix1 = lx2;
    if(ix2 < lx1)      ix2 = lx1;
    else if(ix2 > lx2) ix2 = lx2;
    if(iy1 < ly1)      iy1 = ly1;
    else if(iy1 > ly2) iy1 = ly2;
    if(iy2 < ly1)      iy2 = ly1;
    else if(iy2 > ly2) iy2 = ly2;

    if(ix1 == ix2 || iy1 == iy2) return false;

    x1 = scale_x(&ax->d, ix1);
    x2 = scale_x(&ax->d, ix2);
    y1 = scale_y(&ax->d, iy1);
    y2 = scale_y(&ax->d, iy2);

    /* check for float resolution
     */
    if(x1 == x2 || (float)y1 == (float)y2)
    {
	return false;
    }
    else if(fabs(x2 - x1) <= ax->delx_min || fabs(y2 - y1) <= ax->dely_min)
    {
	return false;
    }

    if(ax->zoom < MAX_ZOOM-1) ax->zoom++;
    ax->zoom_max = ax->zoom;

    ax->x1[ax->zoom] = (x1 < x2) ? x1 : x2;
    ax->y1[ax->zoom] = (y1 < y2) ? y1 : y2;
    ax->x2[ax->zoom] = (x2 > x1) ? x2 : x1;
    ax->y2[ax->zoom] = (y2 > y1) ? y2 : y1;

    return true;
}

static void
DoMagFrom(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	AxesWidget from;

	if(ax->mag_from != NULL)
	{
	    from = (AxesWidget)ax->mag_from;
	    _AxesRedisplayMagRect(from);
	    from->axes.mag_scaled_x1 = ax->x1[ax->zoom];
	    from->axes.mag_scaled_x2 = ax->x2[ax->zoom];
	    from->axes.mag_scaled_y1 = ax->y1[ax->zoom];
	    from->axes.mag_scaled_y2 = ax->y2[ax->zoom];
	    _AxesRedisplayMagRect(from);
	}
}

/** 
 *  
 */
Boolean
_AxesDragScroll(AxesWidget w, XEvent *event)
{
	if(dragScroll(w, event)) {
	    Redraw(w);
	    Redisplay((Widget)w, NULL, NULL);
	    return True;
	}
	return False;
}

static Boolean
dragScroll(AxesWidget w, XEvent *event)
{
	AxesPart *ax = &w->axes;
	int bottom, top;
	int y = ((XButtonEvent *)event)->y;

	bottom = unscale_y(&ax->d, ax->y2[ax->zoom]);
	top = unscale_y(&ax->d, ax->y1[ax->zoom]);

	if( (y > bottom && ax->y2[ax->zoom] < ax->y2[0]) ||
	    (y < top && ax->y1[ax->zoom] > ax->y1[0]))
	{
	    double y1 = ax->y1[ax->zoom];
	    double y2 = ax->y2[ax->zoom];
	    if(y > bottom) {
		y2 += .04*(ax->y2[ax->zoom] - ax->y1[ax->zoom]);
		if(y2 > ax->y2[0]) y2 =  ax->y2[0];
		y1 = y2 - (ax->y2[ax->zoom] - ax->y1[ax->zoom]);
	    }
	    else {
		y1 -= .04*(ax->y2[ax->zoom] - ax->y1[ax->zoom]);
		if(y1 < ax->y1[0]) y1 =  ax->y1[0];
		y2 = y1 + (ax->y2[ax->zoom] - ax->y1[ax->zoom]);
	    }
	    ax->y1[ax->zoom] = y1;
	    ax->y2[ax->zoom] = y2;

	    return True;
	}
	return False;
}


static void
SetLimits(AxesWidget w, Boolean init)
{
	AxesPart *ax = &w->axes;
	double d, xdif, ydif, xpermin, xpermax, ypermin, ypermax;
	double x_min, x_max, y_min, y_max;

	if(init) {
	    ax->zoom = 0;
	    ax->zoom_max = 0;
	}
	else
	{
	    if(ax->x_min > ax->x_max) {
		x_min = ax->x_max;
		x_max = ax->x_min;
	    }
	    else {
		x_max = ax->x_max;
		x_min = ax->x_min;
	    }
	    if(ax->y_min > ax->y_max) {
		y_min = ax->y_max;
		y_max = ax->y_min;
	    }
	    else {
		y_max = ax->y_max;
		y_min = ax->y_min;
	    }
	    if(ax->x_axis != LEFT)
	    {
		/* the +/- 0.1 is to take account of rounding */
/**/
/* OLD
		if(y_min < ax->y1[0] || y_max > ax->y2[0] ||
		   1.001*x_min < ax->x1[0] || 
		    .999*x_max > ax->x2[0] )
OLD */
		xdif = ax->x2[0] - ax->x1[0];
		ydif = ax->y2[0] - ax->y1[0];
		xpermin = (xdif != 0.) ? fabs((x_min - ax->x1[0])/xdif) : 1.;
		xpermax = (xdif != 0.) ? fabs((x_max - ax->x2[0])/xdif) : 1.;
		ypermin = (ydif != 0.) ? fabs((y_min - ax->y1[0])/ydif) : 1.;
		ypermax = (ydif != 0.) ? fabs((y_max - ax->y2[0])/ydif) : 1.;
		if(xpermin < .01 && xpermax < .01 && ypermin < .01
			&& ypermax < .01)
		{
		    ax->zoom = ax->zoom_min;
		    ax->zoom_max = ax->zoom_min;
		}
		else if(ax->zoom == ax->zoom_min) {
		    ax->zoom++;
		    ax->zoom_max = ax->zoom;
		}
/***/
	    }
	    else
	    {
/****/
		if(x_min < ax->y1[0] || x_max > ax->y2[0] ||
		   y_min < ax->x1[0] || y_max > ax->x2[0] )
		{
/***/
		    ax->zoom = ax->zoom_min;
		    ax->zoom_max = ax->zoom_min;
/***/
		}
		else if(ax->zoom == ax->zoom_min) {
		    ax->zoom++;
		    ax->zoom_max = ax->zoom;
		}
/***/
	    }
	}
	/*
	 * force x (time) axis to be increasing left to right or top to bottom.
	 */
	if(ax->x_axis != LEFT)
	{
	    if(ax->x_min > ax->x_max) {
		d = ax->x_min;
		ax->x_min = ax->x_max;
		ax->x_max = d;
	    }
	    ax->reverse_x = False;
	    ax->x1[ax->zoom] = ax->x_min;
	    ax->x2[ax->zoom] = ax->x_max;

	    if(ax->y_min < ax->y_max) {
		ax->reverse_y = False;
		ax->y1[ax->zoom] = ax->y_min;
		ax->y2[ax->zoom] = ax->y_max;
	    }
	    else {
		ax->reverse_y = True;
		ax->y1[ax->zoom] = ax->y_max;
		ax->y2[ax->zoom] = ax->y_min;
	    }
	}
	else
	{
	    if(ax->x_min > ax->x_max) {
		d = ax->x_min;
		ax->x_min = ax->x_max;
		ax->x_max = d;
	    }
	    ax->reverse_y = True;
	    ax->y1[ax->zoom] = ax->x_min;
	    ax->y2[ax->zoom] = ax->x_max;

	    if(ax->y_min < ax->y_max) {
		ax->reverse_x = False;
		ax->x1[ax->zoom] = ax->y_min;
		ax->x2[ax->zoom] = ax->y_max;
	    }
	    else {
		ax->reverse_x = True;
		ax->x1[ax->zoom] = ax->y_max;
		ax->x2[ax->zoom] = ax->y_min;
	    }
	}
	if(init) {
	    int i;
	    for(i = 0; i < MAX_ZOOM; i++) {
		ax->x1[i] = ax->x_min;
		ax->x2[i] = ax->x_max;
		ax->y1[i] = ax->y_min;
		ax->y2[i] = ax->y_max;
	    }
	}
}

void AxesClass::unzoom()
{
    AxesPart *ax = &aw->axes;
    if(( ax->auto_y_scale && ax->zoom > 0) ||
	   (!ax->auto_y_scale && ax->zoom > 1))
    {
	if(ax->zoom > ax->zoom_min && ax->zoom > ax->min_zoom_level) {
	    ax->zoom--;
	    Redraw(aw);
	    Redisplay((Widget)aw, NULL, NULL);
	    DoMagFrom(aw);
	}
    }
    else
    {
	unzoomAll();
    }
}

void AxesUnzoomAll(AxesWidget w)
{
    AxesPart *ax = &w->axes;
    if(!ax->auto_y_scale && ax->zoom == 0)
    {
	ax->x1[1] = ax->x1[0];
	ax->x2[1] = ax->x2[0];
	ax->y1[1] = ax->y1[0];
	ax->y2[1] = ax->y2[0];
	ax->zoom = 1;
    }
    if(ax->redraw_data_func != NULL)
    {
	(*(ax->redraw_data_func))(w, DATA_ZOOM_ZERO, False);
    }
    DoMagFrom(w);
}

void AxesClass::unzoomVerticalAll()
{
    AxesPart *ax = &aw->axes;
    if(!ax->auto_y_scale && ax->zoom == 0)
    {
	ax->x1[1] = ax->x1[0];
	ax->x2[1] = ax->x2[0];
	ax->y1[1] = ax->y1[0];
	ax->y2[1] = ax->y2[0];
	ax->zoom = 1;
    }

    if(ax->redraw_data_func != NULL)
    {
	(*(ax->redraw_data_func))(aw, DATA_ZOOM_VERTICAL_ZERO, False);
    }
    DoMagFrom(aw);
}

void AxesClass::unzoomHorizontalAll()
{
    AxesPart *ax = &aw->axes;
    if(!ax->auto_y_scale && ax->zoom == 0)
    {
	ax->x1[1] = ax->x1[0];
	ax->x2[1] = ax->x2[0];
	ax->y1[1] = ax->y1[0];
	ax->y2[1] = ax->y2[0];
	ax->zoom = 1;
    }
    if(ax->redraw_data_func != NULL)
    {
	(*(ax->redraw_data_func))(aw, DATA_ZOOM_HORIZONTAL_ZERO, False);
    }
    DoMagFrom(aw);
}

void AxesClass::setMargins(bool auto_x, bool auto_y, int left, int right,
			int top, int bottom)
{
    AxesPart *ax = &aw->axes;
    ax->a.auto_x = auto_x;
    ax->a.auto_y = auto_y;
    ax->a.left   = left;
    ax->a.right  = right;
    ax->a.top    = top;
    ax->a.bottom = bottom;
    if(!ax->a.auto_x) {
	ax->rt_margin = aw->core.width - right;
    }
    if(!ax->a.auto_y) {
	ax->bm_margin = aw->core.height - bottom;
    }
}

void AxesClass::getMargins(bool *auto_x, bool *auto_y, int *left,
		int *right, int *top, int *bottom)
{
    AxesPart *ax = &aw->axes;
    *auto_x = ax->a.auto_x;
    *auto_y = ax->a.auto_y;
    *left   = ax->a.left;
    *right  = ax->a.right;
    *top    = ax->a.top;
    *bottom = ax->a.bottom;
}

/** 
 *  
 */
void
_AxesUnzoomPercentage(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	double x1, x2, y1, y2, hor_zoom_fraction, ver_zoom_fraction;

	if(!ax->zoom_controls) return;

	AxesWaitCursor((Widget)w, True);
	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];

	if(*num_params != 2 || !stringToDouble(params[0], &hor_zoom_fraction)
		|| !stringToDouble(params[1], &ver_zoom_fraction))
	{
	   hor_zoom_fraction = ax->zoom_horizontal;
	   ver_zoom_fraction = ax->zoom_vertical;
	}

	ax->x1[ax->zoom] = x1 - .5*hor_zoom_fraction*(x2 - x1);
	ax->x2[ax->zoom] = x2 + .5*hor_zoom_fraction*(x2 - x1);
	ax->y1[ax->zoom] = y1 - .5*ver_zoom_fraction*(y2 - y1);
	ax->y2[ax->zoom] = y2 + .5*ver_zoom_fraction*(y2 - y1);

	ax->zoom++;
	ax->axes_class->unzoom();
	AxesWaitCursor((Widget)w, False);
}

/** 
 *  
 */
void
_AxesUnzoomHorizontal(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	double x1, x2, zoom_fraction;

	if(!ax->zoom_controls) return;

	AxesWaitCursor((Widget)w, True);

	if(*num_params != 1 || !stringToDouble(params[0], &zoom_fraction)) {
	    zoom_fraction = ax->zoom_horizontal;
	}

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	ax->x1[ax->zoom] = x1 - .5*zoom_fraction*(x2 - x1);
	ax->x2[ax->zoom] = x2 + .5*zoom_fraction*(x2 - x1);
	ax->zoom++;
	ax->axes_class->unzoom();
	AxesWaitCursor((Widget)w, False);
}

/** 
 *  
 */
void
_AxesUnzoomVertical(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	double y1, y2, zoom_fraction;

	if(!ax->zoom_controls) return;

	if(ax->a.y_label_int && fabs(ax->y2[ax->zoom] - ax->y1[ax->zoom]) >
		1000000) return;
	AxesWaitCursor((Widget)w, True);

	if(*num_params != 1 || !stringToDouble(params[0], &zoom_fraction)) {
	    zoom_fraction = ax->zoom_vertical;
	}

	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	ax->y1[ax->zoom] = y1 - .5*zoom_fraction*(y2 - y1);
	ax->y2[ax->zoom] = y2 + .5*zoom_fraction*(y2 - y1);
	ax->zoom++;
	ax->axes_class->unzoom();
	AxesWaitCursor((Widget)w, False);
}

/** 
 *  
 */
void
_AxesZoomHorizontal(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int i;
	double x1, x2, y1, y2, f, delx, zoom_fraction;

	if(!ax->zoom_controls) return;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];

	if(*num_params != 1 || !stringToDouble(params[0], &zoom_fraction)) {
	    zoom_fraction = ax->zoom_horizontal;
	}

	f = 1./(1. + zoom_fraction);
	delx = fabs(f*(x2-x1));
	if(delx > ax->delx_min)
	{
	    if(ax->zoom == 0)
	    {
		ax->zoom = 1;
	    }
	    i = ax->zoom;

	    AxesWaitCursor((Widget)w, True);
	    ax->x1[i] = .5*(x1+x2) - .5*f*(x2-x1);
	    ax->x2[i] = .5*(x1+x2) + .5*f*(x2-x1);
	    ax->y1[i] = y1;
	    ax->y2[i] = y2;
	    Redraw(w);
	    Redisplay((Widget)w, NULL, NULL);
	    DoMagFrom(w);
	    AxesWaitCursor((Widget)w, False);
	}
}

/** 
 *  
 */
void
_AxesZoomVertical(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int i;
	double x1, x2, y1, y2, f, dely, zoom_fraction;

	if(!ax->zoom_controls) return;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];

	if(*num_params != 1 || !stringToDouble(params[0], &zoom_fraction)) {
	    zoom_fraction = ax->zoom_vertical;
	}

	f = 1./(1. + zoom_fraction);
	dely = f*(y2-y1);
	if(fabs(dely) > ax->dely_min)
	{
	    if(ax->zoom == 0)
	    {
		ax->zoom = 1;
	    }
	    i = ax->zoom;
	    AxesWaitCursor((Widget)w, True);

	    ax->x1[i] = x1;
	    ax->x2[i] = x2;
	    f = 1./(1. + zoom_fraction);
	    ax->y1[i] = .5*(y1+y2) - .5*f*(y2-y1);
	    ax->y2[i] = .5*(y1+y2) + .5*f*(y2-y1);
	    Redraw(w);
	    Redisplay((Widget)w, NULL, NULL);
	    DoMagFrom(w);
	    AxesWaitCursor((Widget)w, False);
	}
}

/** 
 *  
 */
void
_AxesZoomPercentage(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int i;
	double x1, x2, y1, y2, f, delx, dely, hor_zoom_fraction,
		ver_zoom_fraction;

	if(!ax->zoom_controls) return;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];

	if(*num_params != 2 || !stringToDouble(params[0], &hor_zoom_fraction)
		|| !stringToDouble(params[1], &ver_zoom_fraction))
	{
	    hor_zoom_fraction = ax->zoom_horizontal;
	    ver_zoom_fraction = ax->zoom_vertical;
	}

	f = 1./(1. + hor_zoom_fraction);
	delx = fabs(f*(x2-x1));
	f = 1./(1. + ver_zoom_fraction);
	dely = fabs(f*(y2-y1));
	if(delx > ax->delx_min && dely > ax->dely_min)
	{
	    i = ax->zoom;

	    f = 1./(1. + hor_zoom_fraction);
	    ax->x1[i] = .5*(x1+x2) - .5*f*(x2-x1);
	    ax->x2[i] = .5*(x1+x2) + .5*f*(x2-x1);

	    f = 1./(1. + ver_zoom_fraction);
	    ax->y1[i] = .5*(y1+y2) - .5*f*(y2-y1);
	    ax->y2[i] = .5*(y1+y2) + .5*f*(y2-y1);

	    AxesWaitCursor((Widget)w, True);
	    Redraw(w);
	    Redisplay((Widget)w, NULL, NULL);
	    DoMagFrom(w);
	    AxesWaitCursor((Widget)w, False);
	}
}

int
AxesPage(AxesWidget widget, XEvent *event, const char **params,
		Cardinal *num_params)
{
	int n, value, slider_size, increment, page, min, max, sgn;
	unsigned char dir;
	int	end;
	Arg args[3];
	XmScrollBarCallbackStruct s;
	Widget w;

	end = False;
	if(*num_params <= 1) return(end);

	if(!strcmp(params[0], "vertical"))
	{
	    if(!widget->axes.vertical_scroll) return(end);
	    w = widget->axes.vert_scroll;
	}
	else if(!strcmp(params[0], "horizontal"))
	{
	    if(!widget->axes.horizontal_scroll) return(end);
	    w = widget->axes.hor_scroll;
	}
	else
	{
	    return(end);
	}
	XmScrollBarGetValues(w, &value, &slider_size, &increment, &page);
	n = 0;
	XtSetArg(args[n], XmNminimum, &min); n++;
	XtSetArg(args[n], XmNmaximum, &max); n++;
	XtSetArg(args[n], XmNprocessingDirection, &dir); n++;
	XtGetValues(w, args, n);

	sgn = (dir == XmMAX_ON_TOP || dir == XmMAX_ON_RIGHT) ? -1 : 1;

	if(!strcmp(params[1], "UP"))
	{
		s.value = value - sgn*page;
		s.reason = (sgn < 0) ? XmCR_PAGE_INCREMENT :XmCR_PAGE_DECREMENT;
	}
	else if(!strcmp(params[1], "DOWN"))
	{
		s.value = value + sgn*page;
		s.reason = (sgn > 0) ? XmCR_PAGE_INCREMENT :XmCR_PAGE_DECREMENT;
	}
	else if(!strcmp(params[1], "up"))
	{
		s.value = value - sgn*increment;
		s.reason = (sgn < 0) ? XmCR_INCREMENT : XmCR_DECREMENT;
	}
	else if(!strcmp(params[1], "down"))
	{
		s.value = value + sgn*increment;
		s.reason = (sgn > 0) ? XmCR_INCREMENT : XmCR_DECREMENT;
	}
	else
	{
		return(end);
	}

	if(s.value < min)
	{
		s.value = min;
		end = True;
	}
	if(s.value > max - slider_size)
	{
		s.value = max - slider_size;
		end = True;
	}

	n = 0;
	XtSetArg(args[n], XmNvalue, s.value); n++;
	XtSetValues(w, args, n);

	if(!strcmp(params[0], "vertical"))
	{
		VerticalScroll(w, (XtPointer)widget, (XtPointer)&s);
	}
	else
	{
		HorizontalScroll(w, (XtPointer)widget, (XtPointer)&s);
	}
	return(end);
}
	
/** 
 *  
 */
void
_AxesKeyCommand(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	char c, str[2];
	int ascent, descent, direction;
	XCharStruct overall;
	AxesKeyPressCallbackStruct	a;

	if(XtHasCallbacks((Widget)w, XtNkeyPressCallback) != XtCallbackHasNone)
	{
		a.x1 = ((XButtonEvent *)event)->x;
		a.y1 = ((XButtonEvent *)event)->y;
		a.x = scale_x(&ax->d, a.x1);
		a.y = scale_y(&ax->d, a.y1);
		a.len = XLookupString((XKeyEvent *)event, a.buf, 20, &a.key,
				&a.cs);

		/* ensure that the pointer is within the valid area */
		if (a.x >= ax->x1[ax->zoom] &&
		    a.x <= ax->x2[ax->zoom] &&
		    a.y >= ax->y1[ax->zoom] &&
		    a.y <= ax->y2[ax->zoom])
		{
		    XtCallCallbacks((Widget)w, XtNkeyPressCallback, &a);
		}

		return;
	}

	if(!ax->label_mode)
	{
		return;
	}
	XLookupString((XKeyEvent *)event, &c, 1, NULL, NULL);

	if(!ax->label_mode)
	{
		switch(c)
		{
		case 'c':
			_AxesAddCrosshair(w, event, params, num_params);
			break;
		case 'l':
			_AxesAddDoubleLine(w, event, params, num_params);
			break;
		case 'd':
			_AxesDeleteCursor(w, event, params, num_params);
			break;
		case 'h':
			_AxesUnzoomHorizontal(w, event, params, num_params);
			break;
		case 'v':
			_AxesUnzoomVertical(w, event, params, num_params);
			break;
		}
	}
	else if(c != '\n' && c != 13)
	{
		if(ax->label_xpos < 0)
		{
			ax->label_xpos = ((XButtonEvent *)event)->x;
			ax->label_ypos = ((XButtonEvent *)event)->y;
		}
		str[0] = c;
		str[1] = '\0';
		_AxesDrawString(w, ax->axesGC, ax->label_xpos,
			ax->label_ypos, str);
		XTextExtents(ax->font, &c, 1, &direction,
			&ascent, &descent, &overall);
		ax->label_xpos += overall.width;
	}
	else
	{
		ax->label_xpos = -1;
		ax->label_ypos = -1;
	}
}

/** 
 *  
 */
void
_AxesBtnDown(AxesWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	XButtonEvent *e = (XButtonEvent *)event;

	/* Specifying Btn4Up, Btn4Down, Btn5Up, etc does work in the translation
	 * table, so check for button 4 and 5 here.
	 */
	if(e->button == Button4) {
	    const char *p[] = {"vertical", "up"};
	    Cardinal num = 2;
	    AxesPage(w, event, p, &num);
	}
	else if(e->button == Button5) {
	    const char *p[] = {"vertical", "down"};
	    Cardinal num = 2;
	    AxesPage(w, event, p, &num);
	}
}

int AxesClass::addCrosshair()
{
    AxesPart *ax = &aw->axes;
    if(ax->num_cursors < MAX_CURSORS) {
	AddCursor(aw, AXES_CROSSHAIR, NULL, True);
	return(0);
    }
    else {
	return(-1);
    }
}

char AxesClass::addLine()
{
    AxesPart *ax = &aw->axes;
    if(ax->num_cursors < MAX_CURSORS) {
	AddCursor(aw, AXES_VERTICAL_LINE, NULL, False);
	return(ax->cursor[ax->num_cursors-1].label[0]);
    }
    else {
	return('\0');
    }
}

char AxesClass::addDoubleLine(bool draw_labels)
{
    AxesPart *ax = &aw->axes;
    if(ax->num_cursors < MAX_CURSORS) {
	AddCursor(aw, AXES_VER_DOUBLE_LINE, NULL, False);
	ax->cursor[ax->num_cursors-1].draw_labels = draw_labels;
	return(ax->cursor[ax->num_cursors-1].label[0]);
    }
    else {
	return('\0');
    }
}

void AxesClass::addHorDoubleLine()
{
    AxesPart *ax = &aw->axes;
    if(ax->num_cursors < MAX_CURSORS) {
	AddCursor(aw, AXES_HOR_DOUBLE_LINE, NULL, False);
	ax->cursor[ax->num_cursors-1].draw_labels = False;
    }
}

int AxesClass::renameCursor(const string &old_label, const string &new_label)
{
    AxesPart *ax = &aw->axes;
    int i;

    if((int)old_label.length() <= 0 || (int)new_label.length() <= 0 ) {
	return(-1);
    }
    for(i = 0; i < ax->num_cursors; i++)
    {
	if(!old_label.compare(ax->cursor[i].label))
	{
	    ClearCursor(aw, i);
	    strncpy(ax->cursor[i].label, new_label.c_str(), 50);
	    ax->cursor[i].label[9] = '\0';
	    DrawCursor(aw, i);
	    break;
	}
    }
    return ((i < ax->num_cursors) ? true : false);
}

int AxesClass::addPhaseLine(const string &phase)
{
    AxesPart *ax = &aw->axes;
    int i;
    AxesWidget z;

    for(i = 0; i < ax->num_cursors; i++)
    {
	if(ax->cursor[i].type == AXES_PHASE_LINE) break;
    }
    if(i < ax->num_cursors)
    {
	ClearCursor(aw, i);
	strncpy(ax->cursor[i].label, phase.c_str(), 50);
	ax->cursor[i].label[9] = '\0';
	DrawCursor(aw, i);

	if((z = (AxesWidget)ax->mag_to) != NULL)
	{
	    ClearCursor(z, i);
	    strncpy(z->axes.cursor[i].label, phase.c_str(), 50);
	    z->axes.cursor[i].label[9] = '\0';
	    DrawCursor(z, i);
	}
	if((z = (AxesWidget)ax->mag_from) != NULL)
	{
	    ClearCursor(z, i);
	    strncpy(z->axes.cursor[i].label, phase.c_str(), 50);
	    z->axes.cursor[i].label[9] = '\0';
	    DrawCursor(z, i);
	}
    }
    else if(ax->num_cursors < MAX_CURSORS)
    {
		
	AddCursor(aw, AXES_PHASE_LINE, phase.c_str(), False);
	return(0);
    }
    else
    {
	    return(-1);
    }
    return(0);
}

/** 
 *  
 */
void
_AxesAddCrosshair(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AddCursor(w, AXES_CROSSHAIR, NULL, True);
}

/** 
 *  
 */
void
_AxesAddDoubleLine(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AddCursor(w, AXES_VER_DOUBLE_LINE, NULL, True);
}

/** 
 *  
 */
void
_AxesAddLine(AxesWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	AddCursor(w, AXES_VERTICAL_LINE, NULL, True);
}

/** 
 *  
 */
void
_AxesDeleteCursor(AxesWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int i, ic, cursor_x, cursor_y;
	Boolean have_crosshair;
	float d_min;

	have_crosshair = haveCrosshair(ax);
	if(*num_params < 1 || !strcmp(params[0], "one"))
	{
	    cursor_x = ((XButtonEvent *)event)->x;
	    cursor_y = ((XButtonEvent *)event)->y;

	    if((ic = _AxesWhichCursor(w, cursor_x, cursor_y, &d_min, &i))>=0)
	    {
		if(ax->cursor[ic].type != AXES_PHASE_LINE) {
		    DeleteCursor(w, ic, False);
		    if(ax->cursor_mode == AXES_SELECT_CURSOR) {
			ax->cursor_mode = AXES_ZOOM;
			_AxesSetCursor(w, "default");
		    }
		}
	    }
	}
	else
	{
	    for(i = ax->num_cursors; i >= 0; i--)
		if(ax->cursor[i].type != AXES_PHASE_LINE)
	    {
		DeleteCursor(w, i, False);
	    }
	    if(ax->cursor_mode == AXES_SELECT_CURSOR) {
		ax->cursor_mode = AXES_ZOOM;
		_AxesSetCursor(w, "default");
	    }
	}
	if(have_crosshair && !haveCrosshair(ax)) {
	    Redraw(w);
	    Redisplay((Widget)w, NULL, NULL);
	}
}

/** 
 *  
 */
void
_AxesSetCursor(AxesWidget w, const char *type)
{
	AxesPart *ax = &w->axes;

	if(strcmp(ax->cursor_type, type)) {
	    if(!strcmp(type, "default")) {
		if(ax->cursor_mode == AXES_SELECT_CURSOR ||
		   ax->cursor_mode == AXES_MOVE_MAGNIFY)
		{
		    if(!strcmp(ax->cursor_type, "hand")) {
			return;
		    }
		    else if(strcmp(ax->cursor_type, "move")) {
			if(XtIsRealized((Widget)w)) {
			    XDefineCursor(XtDisplay(w), XtWindow(w),
				    GetCursor((Widget)w, "move"));
			}
	                strncpy(ax->cursor_type, "move", 20);
		    }
		}
		else {
		    if(XtIsRealized((Widget)w)) {
			XUndefineCursor(XtDisplay(w), XtWindow(w));
		    }
	            strncpy(ax->cursor_type, "default", 20);
		}
	    }
	    else {
		if(XtIsRealized((Widget)w)) {
		    XDefineCursor(XtDisplay(w), XtWindow(w),
			GetCursor((Widget)w, type));
		}
	        strncpy(ax->cursor_type, type, 20);
	    }
	    XFlush(XtDisplay(w));
	}
}

static int
AddCursor(AxesWidget w, int type, const char *phase, Boolean do_callback)
{
	AxesPart *ax = &w->axes;
	int i, j, ic;
	Boolean have_crosshair;
	AxesWidget z;
	AxesCursor *c;
	AxesCursor axes_cursor_init = AXES_CROSSHAIR_INIT;

	if(ax->num_cursors >= MAX_CURSORS) {
	    return -1;
	}
	have_crosshair = haveCrosshair(ax);
	ic = ax->num_cursors;

	ax->num_cursors++;

	c = &ax->cursor[ic];

	memcpy(c, &axes_cursor_init, sizeof(AxesCursor));

	c->scaled_x = ax->x1[ax->zoom] + (ax->x2[ax->zoom] - ax->x1[ax->zoom])*
		((float)(ic+1)/(MAX_CURSORS+1));
	c->scaled_y = ax->y2[ax->zoom] + (ax->y1[ax->zoom] - ax->y2[ax->zoom])*
		((float)(ic+1)/(MAX_CURSORS+1));
	c->type = type;
	c->scaled_x1 = c->scaled_x2 = c->scaled_x;
	c->scaled_y1 = c->scaled_y2 = c->scaled_y;

	if(type == AXES_VER_DOUBLE_LINE)
	{
	    for(i = 0; i < MAX_CURSORS; i++)
	    {
		for(j = 0; j < ax->num_cursors-1; j++)
		{
		    if(ax->cursor[j].type == AXES_VER_DOUBLE_LINE &&
			!strncmp(ax->cursor[j].label, double_line_label[i], 2))
		    {
			    break;
		    }
		}
		if(j == ax->num_cursors-1) {
		   strncpy(ax->cursor[j].label, double_line_label[i], 50);
		   break;
		}
	    }
	    if(ax->double_line_width > 0.) {
		c->scaled_x1 = c->scaled_x-.5*ax->double_line_width;
		c->scaled_x2 = c->scaled_x1 + ax->double_line_width;
	    }
	    else {
		c->scaled_x1 = c->scaled_x - (ax->x2[ax->zoom]
				- ax->x1[ax->zoom])/ (4.*MAX_CURSORS);
		c->scaled_x2 = c->scaled_x + (ax->x2[ax->zoom]
				- ax->x1[ax->zoom])/ (4.*MAX_CURSORS);
	    }
	}
	else if(type == AXES_HOR_DOUBLE_LINE)
	{
c->draw_labels = False;
	    if(ax->double_line_width > 0.) {
		c->scaled_y1 = c->scaled_y-.5*ax->double_line_width;
		c->scaled_y2 = c->scaled_y1 + ax->double_line_width;
	    }
	    else {
		c->scaled_y1 = c->scaled_y - (ax->y2[ax->zoom]
				- ax->y1[ax->zoom])/ (4.*MAX_CURSORS);
		c->scaled_y2 = c->scaled_y + (ax->y2[ax->zoom]
				- ax->y1[ax->zoom])/ (4.*MAX_CURSORS);
	    }
	}
	else if(type == AXES_HORIZONTAL_LINE || type == AXES_VERTICAL_LINE)
	{
	    for(i = j = 0; i < ax->num_cursors; i++)
	    {
		if(ax->cursor[i].type == AXES_HORIZONTAL_LINE ||
		   ax->cursor[i].type == AXES_VERTICAL_LINE)
		{
		    strncpy(ax->cursor[i].label, line_label[j++], 50);
		}
	    }
	}
	else if(type == AXES_PHASE_LINE)
	{
	    if(phase != NULL) strncpy(c->label, phase, 50);
	    else c->label[0] = '\0';
	}
	if(ax->mag_from == NULL)
	{
	    if(ax->cursor_func != NULL) {
		(*(ax->cursor_func))(w, NULL, c);
	    }
	}

	DrawCursor(w, ic);

	if(type == AXES_CROSSHAIR && !have_crosshair) {
	    Redraw(w);
	    Redisplay((Widget)w, NULL, NULL);
	}

	if((z = (AxesWidget)ax->mag_to) != NULL)
	{
	    z->axes.num_cursors++;
	    z->axes.cursor[ic].scaled_x = ax->cursor[ic].scaled_x;
	    z->axes.cursor[ic].scaled_y = ax->cursor[ic].scaled_y;
	    strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
	    z->axes.cursor[ic].type = type;
	    z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
	    z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
	    DrawCursor(z, ic);
	    if(!have_crosshair && type == AXES_CROSSHAIR) {
		Redraw(z);
		Redisplay((Widget)z, NULL, NULL);
	    }
	}
	if((z = (AxesWidget)ax->mag_from) != NULL)
	{
	    z->axes.num_cursors++;
	    z->axes.cursor[ic].scaled_x = ax->cursor[ic].scaled_x;
	    z->axes.cursor[ic].scaled_y = ax->cursor[ic].scaled_y;
	    strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
	    z->axes.cursor[ic].type = type;
	    z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
	    z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
	    if(z->axes.cursor_func != NULL)
	    {
		(*(z->axes.cursor_func))(z,NULL,&z->axes.cursor[ic]);
	    }
	    DrawCursor(z, ic);
	    if(!have_crosshair && type == AXES_CROSSHAIR) {
		Redraw(z);
		Redisplay((Widget)z, NULL, NULL);
	    }
	}
	if(do_callback) {
	    DoCursorCallback(w, c, 0);
	}
	return ic;
}

void AxesClass::deleteAllCursors()
{
    AxesPart *ax = &aw->axes;
    for(int i = ax->num_cursors-1; i >= 0; i--) DeleteCursor(aw, i, false);
}

static Boolean
haveCrosshair(AxesPart *ax)
{
	int i;

	for(i = 0; i < ax->num_cursors; i++)
	{
	    if(ax->cursor[i].type == AXES_CROSSHAIR) return True;
	}
	return False;
}

void AxesClass::deleteCrosshair()
{
    AxesPart *ax = &aw->axes;

    for(int i = ax->num_cursors-1; i >= 0; i--) {
	if(ax->cursor[i].type == AXES_CROSSHAIR) {
	    DeleteCursor(aw, i, False);
	    break;
	}
    }
}

void AxesClass::deleteLimitLines()
{
    AxesPart *ax = &aw->axes;

    for(int i = ax->num_cursors-1; i >= 0; i--)
    {
	if(ax->cursor[i].type == AXES_HOR_DOUBLE_LINE ||
	   ax->cursor[i].type == AXES_VER_DOUBLE_LINE)
	{
	    DeleteCursor(aw, i, False);
	    break;
	}
    }
}

void AxesClass::deleteDoubleLine(const string &label)
{
    AxesPart *ax = &aw->axes;

    if(label.empty()) return;
    for(int i = ax->num_cursors-1; i >= 0; i--)
    {
	if(ax->cursor[i].type == AXES_VER_DOUBLE_LINE
		&& ax->cursor[i].label[0] == label[0])
	{
	    DeleteCursor(aw, i, false);
	    return;
	}
    }
}

void AxesClass::deleteLine()
{
    AxesPart *ax = &aw->axes;

    for(int i = ax->num_cursors-1; i >= 0; i--)
    {
	if(ax->cursor[i].type == AXES_HORIZONTAL_LINE ||
	   ax->cursor[i].type == AXES_VERTICAL_LINE)
	{
	    DeleteCursor(aw, i, False);
	    break;
	}
    }
}

void AxesClass::deletePhaseLine()
{
    AxesPart *ax = &aw->axes;

    for(int i = ax->num_cursors-1; i >= 0; i--) {
	if(ax->cursor[i].type == AXES_PHASE_LINE) {
	    DeleteCursor(aw, i, False);
	    break;
	}
    }
}

bool AxesClass::getPhaseLine(double *x, char **phase)
{
    AxesPart *ax = &aw->axes;
    int i;

    for(i = 0; i < ax->num_cursors; i++) {
	if(ax->cursor[i].type == AXES_PHASE_LINE) break;
    }
    if(i < ax->num_cursors) {
	*x = ax->cursor[i].scaled_x;
	*phase = ax->cursor[i].label;
	return true;
    }
    else {
	return false;
    }
}

bool AxesClass::phaseLine(const string &label, AxesCursorCallbackStruct **p)
{
    AxesPart *ax = &aw->axes;

    getCursors(p);
    for(int i = 0; i < ax->num_phase_lines; i++)
    {
	if(!label.compare(ax->phase_lines[i].label))
	{
	    *p = &ax->phase_lines[i];
	    return true;
	}
    }
    *p = NULL;
    return false;
}

int AxesClass::positionCrosshair(int i, double x, double y, bool do_callback)
{
    AxesPart *ax = &aw->axes;
    int ic, j;

    j = 0;
    for(ic = 0; ic < ax->num_cursors; ic++)
    {
	if(ax->cursor[ic].type == AXES_CROSSHAIR) {
	    if(j == i) break;
	    j++;
	}
    }
    if(ic == ax->num_cursors)
    {
	if(ax->num_cursors < MAX_CURSORS) {
	    AddCursor(aw, AXES_CROSSHAIR, NULL, False);
	    ic = ax->num_cursors-1;
	}
	else {
	    return -1;
	}
    }
		
    ax->cursor[ic].scaled_x = x;
    ax->cursor[ic].scaled_y = y;
    ax->cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x2 = x;

    if(ax->mag_from == NULL)
    {
	if(ax->cursor_func != NULL) {
	    (*(ax->cursor_func))(aw, NULL, &ax->cursor[ic]);
	}
    }

    AxesPosition(aw, ic, do_callback);

    return(0);
}

static void
AxesPosition(AxesWidget w, int ic, Boolean do_callback)
{
	AxesPart *ax = &w->axes;
	AxesWidget z;

	ClearCursor(w, ic);
	DrawCursor(w, ic);
	if((z = (AxesWidget)ax->mag_from) != NULL)
	{
		ClearCursor(z, ic);
		z->axes.cursor[ic].scaled_x  = ax->cursor[ic].scaled_x;
		z->axes.cursor[ic].scaled_y  = ax->cursor[ic].scaled_y;
		z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
		z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
		strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
		DrawCursor(z, ic);
	}
	if((z = (AxesWidget)ax->mag_to) != NULL)
	{
		ClearCursor(z, ic);
		z->axes.cursor[ic].scaled_x  = ax->cursor[ic].scaled_x;
		z->axes.cursor[ic].scaled_y  = ax->cursor[ic].scaled_y;
		z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
		z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
		strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
		DrawCursor(z, ic);
	}
	if(do_callback)
	{
		DoCursorCallback(w, &ax->cursor[ic], 0);
	}
}

int AxesClass::positionLine(const string &label, double x, bool do_callback)
{
    AxesPart *ax = &aw->axes;
    int ic;

    if((int)label.length() == 0) return -1;

    for(ic = 0; ic < ax->num_cursors; ic++) {
	if(ax->cursor[ic].type == AXES_VERTICAL_LINE
		&& ax->cursor[ic].label[0] == label[0]) break;
    }
    if(ic == ax->num_cursors)
    {
	if(ax->num_cursors < MAX_CURSORS) {
	    AddCursor(aw, AXES_VERTICAL_LINE, NULL, False);
	    ic = ax->num_cursors-1;
	}
	else {
	    return -1;
	}
    }
		
    ax->cursor[ic].scaled_x = x;
    ax->cursor[ic].scaled_y = 0;
    ax->cursor[ic].scaled_x1 = x;
    ax->cursor[ic].scaled_x2 = x;

    AxesPosition(aw, ic, do_callback);

    return 0;
}

int AxesClass::positionLine(int i, double x, bool do_callback)
{
    AxesPart *ax = &aw->axes;
    int ic;

    int j = 0;
    for(ic = 0; ic < ax->num_cursors; ic++)
    {
	if(ax->cursor[ic].type == AXES_VERTICAL_LINE) {
	    if(j == i) break;
	    j++;
	}
    }
    if(ic == ax->num_cursors)
    {
	if(ax->num_cursors < MAX_CURSORS) {
	    AddCursor(aw, AXES_VERTICAL_LINE, NULL, False);
	    ic = ax->num_cursors-1;
	}
	else {
	    return -1;
	}
    }
		
    ax->cursor[ic].scaled_x = x;
    ax->cursor[ic].scaled_y = 0;
    ax->cursor[ic].scaled_x1 = x;
    ax->cursor[ic].scaled_x2 = x;

    AxesPosition(aw, ic, do_callback);

    return 0;
}

int AxesClass::positionDoubleLine(const string &label, double x1, double x2,
		bool do_callback, bool draw_labels)
{
    AxesPart *ax = &aw->axes;
    int ic;

    if((int)label.length() == 0) return -1;

    for(ic = 0; ic < ax->num_cursors; ic++) {
	if(ax->cursor[ic].type == AXES_VER_DOUBLE_LINE
		&& ax->cursor[ic].label[0] == label[0]) break;
    }
    if(ic == ax->num_cursors)
    {
	if(ax->num_cursors < MAX_CURSORS) {
	    AddDoubleLineWithLabel(aw, label[0], x1, x2, draw_labels);
	}
	else {
	    return -1;
	}
    }
    else {
	ax->cursor[ic].scaled_x = x1;
	ax->cursor[ic].scaled_y = 0;
	ax->cursor[ic].scaled_x1 = x1;
	ax->cursor[ic].scaled_x2 = x2;
    }

    AxesPosition(aw, ic, do_callback);

    return 0;
}

int AxesClass::positionDoubleLine(int i, double x1, double x2, 
		bool do_callback, bool draw_labels)
{
    AxesPart *ax = &aw->axes;
    int ic;

    int j = 0;
    for(ic = 0; ic < ax->num_cursors; ic++)
    {
	if(ax->cursor[ic].type == AXES_VER_DOUBLE_LINE) {
	    if(i == j) break;
	    j++;
	}
    }
    if(ic == ax->num_cursors)
    {
	if(ax->num_cursors < MAX_CURSORS) {
	    AddDoubleLineWithLabel(aw, 'a', x1, x2, draw_labels);
	}
	else {
	    return(-1);
	}
    }
    else {
	ax->cursor[ic].scaled_x = x1;
	ax->cursor[ic].scaled_y = 0;
	ax->cursor[ic].scaled_x1 = x1;
	ax->cursor[ic].scaled_x2 = x2;
    }

    AxesPosition(aw, ic, do_callback);

    return 0;
}

static void
AddDoubleLineWithLabel(AxesWidget w, char label, double x1, double x2,
			Boolean draw_labels)
{
	AxesPart *ax = &w->axes;
	int ic;
	AxesWidget z;
	AxesCursor *c;
	AxesCursor axes_cursor_init = AXES_CROSSHAIR_INIT;

	if(ax->num_cursors >= MAX_CURSORS) {
	    return;
	}
	ic = ax->num_cursors;

	ax->num_cursors++;

	c = &ax->cursor[ic];

	memcpy(c, &axes_cursor_init, sizeof(AxesCursor));

	c->type = AXES_VER_DOUBLE_LINE;
	c->scaled_x = x1;
	c->scaled_y = 0;
	c->scaled_x1 = x1;
	c->scaled_x2 = x2;

	c->label[0] = label;
	c->label[1] = ' ';
	c->label[2] = '\0';
	c->draw_labels = draw_labels;

	if(ax->mag_from == NULL)
	{
	    if(ax->cursor_func != NULL) {
		(*(ax->cursor_func))(w, NULL, c);
	    }
	}

	DrawCursor(w, ic);

	if((z = (AxesWidget)ax->mag_to) != NULL)
	{
	    z->axes.num_cursors++;
	    z->axes.cursor[ic].scaled_x = ax->cursor[ic].scaled_x;
	    z->axes.cursor[ic].scaled_y = ax->cursor[ic].scaled_y;
	    strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
	    z->axes.cursor[ic].type = AXES_VER_DOUBLE_LINE;
	    z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
	    z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
	    DrawCursor(z, ic);
	}
	if((z = (AxesWidget)ax->mag_from) != NULL)
	{
	    z->axes.num_cursors++;
	    z->axes.cursor[ic].scaled_x = ax->cursor[ic].scaled_x;
	    z->axes.cursor[ic].scaled_y = ax->cursor[ic].scaled_y;
	    strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
	    z->axes.cursor[ic].type = AXES_VER_DOUBLE_LINE;
	    z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
	    z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
	    if(z->axes.cursor_func != NULL)
	    {
		(*(z->axes.cursor_func))(z,NULL,&z->axes.cursor[ic]);
	    }
	    DrawCursor(z, ic);
	}
}

int AxesClass::positionPhaseLine(const string &phase, double x,bool do_callback)
{
    AxesPart *ax = &aw->axes;
    int ic;

    for(ic = 0; ic < ax->num_cursors
	    && ax->cursor[ic].type != AXES_PHASE_LINE; ic++);

    if(ic == ax->num_cursors) {
	if(addPhaseLine(phase)) return -1;
    }

    for(ic = 0; ic < ax->num_cursors
	    && ax->cursor[ic].type != AXES_PHASE_LINE; ic++);

    if(ic == ax->num_cursors) return -1;

    ax->cursor[ic].scaled_x = x;
    ax->cursor[ic].scaled_y = 0;
    ax->cursor[ic].scaled_x1 = x;
    ax->cursor[ic].scaled_x2 = x;

    AxesPosition(aw, ic, do_callback);

    renamePhaseLine(phase);

    if(x < ax->x1[0]) {
	ax->x1[0] = x;
	_Axes_AdjustScrollBars(aw);
    }
    else if(x > ax->x2[0]) {
	ax->x2[0] = x;
	_Axes_AdjustScrollBars(aw);
    }

    return 0;
}

int AxesClass::positionPhaseLine2(const string &phase, double x,
			bool do_callback)
{
    AxesPart *ax = &aw->axes;
    int ic;

    for(ic = 0; ic < ax->num_cursors &&
	(ax->cursor[ic].type != AXES_PHASE_LINE ||
		phase.compare(ax->cursor[ic].label)); ic++);

    if(ic == ax->num_cursors) {
	if((ic = addPhaseLine2(phase)) < 0) return -1;
    }

    ax->cursor[ic].scaled_x = x;
    ax->cursor[ic].scaled_y = 0;
    ax->cursor[ic].scaled_x1 = x;
    ax->cursor[ic].scaled_x2 = x;

    AxesPosition(aw, ic, do_callback);

    if(x < ax->x1[0]) {
	ax->x1[0] = x;
	_Axes_AdjustScrollBars(aw);
    }
    else if(x > ax->x2[0]) {
	ax->x2[0] = x;
	_Axes_AdjustScrollBars(aw);
    }

    return 0;
}

int AxesClass::addPhaseLine2(const string &phase)
{
    AxesPart *ax = &aw->axes;
    int i;

    for(i = 0; i < ax->num_cursors &&
	(ax->cursor[i].type != AXES_PHASE_LINE || 
		phase.compare(ax->cursor[i].label)); i++);

    if(i < ax->num_cursors) {
	return i;
    }
    else {
	return AddCursor(aw, AXES_PHASE_LINE, phase.c_str(), False);
    }
}

int AxesClass::renamePhaseLine(const string &phase)
{
    AxesPart *ax = &aw->axes;
    AxesWidget z;
    int ic;

    for(ic = 0; ic < ax->num_cursors
	    && ax->cursor[ic].type != AXES_PHASE_LINE; ic++);

    if(ic == ax->num_cursors) {
	    if(addPhaseLine(phase)) return -1;
    }

    for(ic = 0; ic < ax->num_cursors
	    && ax->cursor[ic].type != AXES_PHASE_LINE; ic++);

    if(ic == ax->num_cursors) return -1;

    if(!phase.empty()) {
	if( !phase.compare(ax->cursor[ic].label) ) return 0;
    }

    ClearCursor(aw, ic);

    if( !phase.empty() ) {
	strncpy(ax->cursor[ic].label, phase.c_str(), 50);
    }
    else {
	strncpy(ax->cursor[ic].label, "-", 50);
    }

    DrawCursor(aw, ic);
    if((z = (AxesWidget)ax->mag_from) != NULL)
    {
	ClearCursor(z, ic);
	z->axes.cursor[ic].scaled_x  = ax->cursor[ic].scaled_x;
	z->axes.cursor[ic].scaled_y  = ax->cursor[ic].scaled_y;
	z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
	z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
	strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
	DrawCursor(z, ic);
    }
    if((z = (AxesWidget)ax->mag_to) != NULL)
    {
	ClearCursor(z, ic);
	z->axes.cursor[ic].scaled_x  = ax->cursor[ic].scaled_x;
	z->axes.cursor[ic].scaled_y  = ax->cursor[ic].scaled_y;
	z->axes.cursor[ic].scaled_x1 = ax->cursor[ic].scaled_x1;
	z->axes.cursor[ic].scaled_x2 = ax->cursor[ic].scaled_x2;
	strncpy(z->axes.cursor[ic].label, ax->cursor[ic].label, 50);
	DrawCursor(z, ic);
    }
    return 0;
}

int AxesClass::getCursors(AxesCursorCallbackStruct **a)
{
    AxesPart *ax = &aw->axes;
    int i, j;

    /* order by AXES_CROSSHAIR, AXES_LINE, AXES_DOUBLE_LINE, AXES_PHASE 
     * set number of each and pointers to each
     */
    *a = ax->all_cursors;
    ax->crosshairs = ax->all_cursors;
    for(i = j = 0; i < ax->num_cursors; i++)
		if(ax->cursor[i].type == AXES_CROSSHAIR)
    {
	ax->all_cursors[j].type	 = ax->cursor[i].type;
	ax->all_cursors[j].x  	 = ax->cursor[i].x;
	ax->all_cursors[j].y  	 = ax->cursor[i].y;
	ax->all_cursors[j].x1	 = ax->cursor[i].x1;
	ax->all_cursors[j].x2	 = ax->cursor[i].x2;
	ax->all_cursors[j].y1	 = ax->cursor[i].y1;
	ax->all_cursors[j].y2	 = ax->cursor[i].y2;
	ax->all_cursors[j].scaled_x  = ax->cursor[i].scaled_x;
	ax->all_cursors[j].scaled_y  = ax->cursor[i].scaled_y;
	ax->all_cursors[j].scaled_x1 = ax->cursor[i].scaled_x1;
	ax->all_cursors[j].scaled_x2 = ax->cursor[i].scaled_x2;
	ax->all_cursors[j].scaled_y1 = ax->cursor[i].scaled_y1;
	ax->all_cursors[j].scaled_y2 = ax->cursor[i].scaled_y2;
	ax->all_cursors[j].index = j;
	strcpy(ax->all_cursors[j].label, ax->cursor[i].label);
	j++;
    }
    ax->num_crosshairs = j;
    ax->lines = ax->all_cursors + j;
    for(i = 0; i < ax->num_cursors; i++)
	if(ax->cursor[i].type == AXES_HORIZONTAL_LINE ||
	   ax->cursor[i].type == AXES_VERTICAL_LINE)
    {
	ax->all_cursors[j].type	 = ax->cursor[i].type;
	ax->all_cursors[j].x	 = ax->cursor[i].x;
	ax->all_cursors[j].y	 = ax->cursor[i].y;
	ax->all_cursors[j].x1	 = ax->cursor[i].x1;
	ax->all_cursors[j].x2	 = ax->cursor[i].x2;
	ax->all_cursors[j].y1	 = ax->cursor[i].y1;
	ax->all_cursors[j].y2	 = ax->cursor[i].y2;
	ax->all_cursors[j].scaled_x  = ax->cursor[i].scaled_x;
	ax->all_cursors[j].scaled_y  = ax->cursor[i].scaled_y;
	ax->all_cursors[j].scaled_x1 = ax->cursor[i].scaled_x1;
	ax->all_cursors[j].scaled_x2 = ax->cursor[i].scaled_x2;
	ax->all_cursors[j].scaled_y1 = ax->cursor[i].scaled_y1;
	ax->all_cursors[j].scaled_y2 = ax->cursor[i].scaled_y2;
	ax->all_cursors[j].label[0]  =  ax->cursor[i].label[0];
	ax->all_cursors[j].label[1]  = '\0';
	ax->all_cursors[j].index = j - ax->num_crosshairs;
	j++;
    }
    ax->num_lines = j - ax->num_crosshairs;
    ax->double_lines = ax->all_cursors + j;
    for(i = 0; i < ax->num_cursors; i++)
	if(ax->cursor[i].type == AXES_HOR_DOUBLE_LINE ||
	   ax->cursor[i].type == AXES_VER_DOUBLE_LINE)
    {
	ax->all_cursors[j].type	 = ax->cursor[i].type;
	ax->all_cursors[j].x	 = ax->cursor[i].x;
	ax->all_cursors[j].y	 = ax->cursor[i].y;
	ax->all_cursors[j].x1	 = ax->cursor[i].x1;
	ax->all_cursors[j].x2	 = ax->cursor[i].x2;
	ax->all_cursors[j].y1	 = ax->cursor[i].y1;
	ax->all_cursors[j].y2	 = ax->cursor[i].y2;
	ax->all_cursors[j].scaled_x  = ax->cursor[i].scaled_x;
	ax->all_cursors[j].scaled_y  = ax->cursor[i].scaled_y;
	ax->all_cursors[j].scaled_x1 = ax->cursor[i].scaled_x1;
	ax->all_cursors[j].scaled_x2 = ax->cursor[i].scaled_x2;
	ax->all_cursors[j].scaled_y1 = ax->cursor[i].scaled_y1;
	ax->all_cursors[j].scaled_y2 = ax->cursor[i].scaled_y2;
	ax->all_cursors[j].label[0]  =  ax->cursor[i].label[0];
	ax->all_cursors[j].label[1]  = '\0';
	ax->all_cursors[j].index = j -ax->num_crosshairs -ax->num_lines;
	j++;
    }
    ax->num_double_lines = j - ax->num_lines-ax->num_crosshairs;
    ax->phase_lines = ax->all_cursors + j;
    for(i = 0; i < ax->num_cursors; i++)
	if(ax->cursor[i].type == AXES_PHASE_LINE)
    {
	ax->all_cursors[j].type	 = ax->cursor[i].type;
	ax->all_cursors[j].x  	 = ax->cursor[i].x;
	ax->all_cursors[j].y	 = ax->cursor[i].y;
	ax->all_cursors[j].x1	 = ax->cursor[i].x1;
	ax->all_cursors[j].x2	 = ax->cursor[i].x2;
	ax->all_cursors[j].y1	 = ax->cursor[i].y1;
	ax->all_cursors[j].y2	 = ax->cursor[i].y2;
	ax->all_cursors[j].scaled_x  = ax->cursor[i].scaled_x;
	ax->all_cursors[j].scaled_y  = ax->cursor[i].scaled_y;
	ax->all_cursors[j].scaled_x1 = ax->cursor[i].scaled_x1;
	ax->all_cursors[j].scaled_x2 = ax->cursor[i].scaled_x2;
	ax->all_cursors[j].scaled_y1 = ax->cursor[i].scaled_y1;
	ax->all_cursors[j].scaled_y2 = ax->cursor[i].scaled_y2;
	ax->all_cursors[j].label[0]  = '\0';
	strncpy(ax->all_cursors[j].label, ax->cursor[i].label, 50);
	ax->all_cursors[j].index = j - ax->num_crosshairs - ax->num_lines
			- ax->num_double_lines;
	j++;
    }
    ax->num_phase_lines = j - ax->num_double_lines - 
			ax->num_lines - ax->num_crosshairs;
    return(ax->num_cursors);
}

int AxesClass::getCrosshairs(AxesCursorCallbackStruct **a)
{
    AxesPart *ax = &aw->axes;
    getCursors(a);
    *a = ax->crosshairs;
    return(ax->num_crosshairs);
}

int AxesClass::getLines(AxesCursorCallbackStruct **a)
{
    AxesPart *ax = &aw->axes;
    getCursors(a);
    *a = ax->lines;
    return(ax->num_lines);
}

int AxesClass::getDoubleLines(AxesCursorCallbackStruct **a)
{
    AxesPart *ax = &aw->axes;
    getCursors(a);
    *a = ax->double_lines;
    return(ax->num_double_lines);
}

int AxesClass::getPhaseLines(AxesCursorCallbackStruct **a)
{
    AxesPart *ax = &aw->axes;
    getCursors(a);
    *a = ax->phase_lines;
    return(ax->num_phase_lines);
}

bool AxesClass::crosshairIsDisplayed()
{
    AxesPart *ax = &aw->axes;

    for(int i = 0; i < ax->num_cursors; i++) {
	if(ax->cursor[i].type == AXES_CROSSHAIR) {
	    return true;
	}
    }
    return false;
}

bool AxesClass::lineIsDisplayed(const string &label)
{
    AxesPart *ax = &aw->axes;
    int i, j, n;

    n = (int)label.length();
    for(j = 0; j < n; j++)
    {
	for(i = 0; i < ax->num_cursors; i++)
	    if((ax->cursor[i].type == AXES_HORIZONTAL_LINE ||
		ax->cursor[i].type == AXES_VERTICAL_LINE) &&
		ax->cursor[i].label[0] == label[j])
	{
	    break;
	}
	if(i == ax->num_cursors) {
	    return false;
	}
    }
    return true;
}

bool AxesClass::doubleLineIsDisplayed(const string &label)
{
    AxesPart *ax = &aw->axes;
    int i, j, n;

    n = (int)label.length();
    for(j = 0; j < n; j++)
    {
	for(i = 0; i < ax->num_cursors; i++)
	    if((ax->cursor[i].type == AXES_HOR_DOUBLE_LINE ||
		ax->cursor[i].type == AXES_VER_DOUBLE_LINE) &&
		ax->cursor[i].label[0] == label[j])
	{
	    break;
	}
	if(i == ax->num_cursors) {
	    return false;
	}
    }
    return true;
}

bool AxesClass::phaseLineIsDisplayed()
{
    AxesPart *ax = &aw->axes;
    for(int i = 0; i < ax->num_cursors; i++) {
	if(ax->cursor[i].type == AXES_PHASE_LINE) return true;
    }
    return false;
}

static void
DeleteCursor(AxesWidget w, int ic, Boolean manual)
{
	AxesPart *ax = &w->axes;
	int i, j, ic_bit;
	AxesWidget z;
	AxesCursor *c;
	Boolean xlabels, ylabels;

	if(ic < 0 || ic >= ax->num_cursors) return;
	c = ax->cursor;

	if(!ax->display_axes) {
	    xlabels = ylabels = False;
	}
	else {
	    xlabels = (ax->display_axes_labels == AXES_X ||
			   ax->display_axes_labels == AXES_XY);
	    ylabels = (ax->display_axes_labels == AXES_Y ||
			   ax->display_axes_labels == AXES_XY);
	}

	if(c->type == AXES_CROSSHAIR)
	{
	    AxesCursorCallbackStruct p;
	    p.index	= ic;
	    p.type	= c[ic].type;
	    p.x		= c[ic].x;
	    p.y		= c[ic].y;
	    p.x1	= c[ic].x1;
	    p.x2	= c[ic].x2;
	    p.scaled_x	= c[ic].scaled_x;
	    p.scaled_y	= c[ic].scaled_y;
	    p.scaled_x1	= c[ic].scaled_x1;
	    p.scaled_x2	= c[ic].scaled_x2;
	    strcpy(p.label, c[ic].label);
	    p.reason = AXES_CROSSHAIR_DELETE;
	    if(XtHasCallbacks((Widget)w, XtNcrosshairCallback) != 
					XtCallbackHasNone)
	    {
		XtCallCallbacks((Widget)w, XtNcrosshairCallback, &p);
	    }
	}

	ClearCursor(w, ic);

	ic_bit = 1 << ic;
	for(j = 0; j < ax->a.nxlab; j++)
	{
		if(ax->a.xlab_off[j] == ic_bit && xlabels)
		{
		    drawOneXLabel(w, ax, j);
		}
		ax->a.xlab_off[j] &= (~ic_bit);
	}
	for(j = 0; j < ax->a.nylab; j++)
	{
		if(ax->a.ylab_off[j] == ic_bit && ylabels)
		{
		    drawOneYLabel(w, ax, j);
		}
		ax->a.ylab_off[j] &= (~ic_bit);
	}
	for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	{
		if(c[i].label_off == ic_bit && c[i].label[0] != '\0')
		{
			_AxesDrawString2(w, ax->axesGC, c[i].label_x,
				c[i].label_y, c[i].label);
		}
		c[i].label_off &= ~ic_bit;

		if(c[i].x_off == ic_bit && c[i].xlab[0] != '\0')
		{
			_AxesDrawString2(w, ax->axesGC, c[i].x_xlab,
					c[i].y_xlab, c[i].xlab);
		}
		c[i].x_off &= ~ic_bit;

		if(c[i].y_off == ic_bit && c[i].ylab[0] != '\0')
		{
			_AxesDrawString2(w, ax->axesGC, c[i].x_ylab,
				c[i].y_ylab, c[i].ylab);
		}
		c[i].y_off &= ~ic_bit;
	}
	for(i = ic+1; i < ax->num_cursors; i++)
	{
		memcpy(c+i-1, c+i, sizeof(AxesCursor));
	}
	ax->num_cursors--;

	if(!manual)
	{
		if((z = (AxesWidget)ax->mag_to) != NULL)
		{
			DeleteCursor(z, ic, True);
		}
		if((z = (AxesWidget)ax->mag_from) != NULL)
		{
			DeleteCursor(z, ic, True);
		}
	}
	for(i = j = 0; i < ax->num_cursors; i++)
		if(c[i].type == AXES_HOR_DOUBLE_LINE ||
		   c[i].type == AXES_VER_DOUBLE_LINE)
	{
		ClearCursor(w, i);
		strncpy(c[i].label, double_line_label[j++], 2);
		c[i].label[2] = '\0';
		DrawCursor(w, i);
	}
	for(i = j = 0; i < ax->num_cursors; i++)
		if(c[i].type == AXES_HORIZONTAL_LINE ||
		   c[i].type == AXES_VERTICAL_LINE)
	{
		ClearCursor(w, i);
		strncpy(c[i].label, line_label[j++], 50);
		DrawCursor(w, i);
	}
}

static void
ConstrainCursor(AxesWidget w, int ic)
{
	AxesPart *ax = &w->axes;
	int i, xmin, xmax, ymin, ymax;
	AxesCursor *c;

	c = &ax->cursor[ic];

	xmin = unscale_x(&ax->d, ax->x1[ax->zoom]);
	xmax = unscale_x(&ax->d, ax->x2[ax->zoom]);
	ymin = unscale_y(&ax->d, ax->y2[ax->zoom]);
	ymax = unscale_y(&ax->d, ax->y1[ax->zoom]);
	if(xmax < xmin)
	{
		i = xmin;
		xmin = xmax;
		xmax = i;
	}
	if(ymax < ymin)
	{
		i = ymin;
		ymin = ymax;
		ymax = i;
	}
	if(c->type == AXES_CROSSHAIR)
	{
		c->x = unscale_x(&ax->d, c->scaled_x);
		c->y = unscale_y(&ax->d, c->scaled_y);
		if(c->x <= xmin) c->x = xmin + 1;
		if(c->x >= xmax) c->x = xmax - 1;
		if(c->y <= ymin) c->y = ymin + 1;
		if(c->y >= ymax) c->y = ymax - 1;
		c->scaled_x = scale_x(&ax->d, c->x);
		c->scaled_y = scale_y(&ax->d, c->y);
	}
	else if(c->type == AXES_VERTICAL_LINE || c->type == AXES_PHASE_LINE)
	{
		c->x = unscale_x(&ax->d, c->scaled_x);
		if(c->x <= xmin) c->x = xmin + 1;
		if(c->x >= xmax) c->x = xmax - 1;
		c->scaled_x = scale_x(&ax->d, c->x);
	}
	else if(c->type == AXES_HORIZONTAL_LINE)
	{
		c->y = unscale_y(&ax->d, c->scaled_y);
		if(c->y <= ymin) c->y = ymin + 1;
		if(c->y >= ymax) c->y = ymax - 1;
		c->scaled_y = scale_y(&ax->d, c->y);
	}
/*
	else if(c->type == AXES_VER_DOUBLE_LINE)
	{
		c->x1 = unscale_x(&ax->d, c->scaled_x1);
		c->x2 = unscale_x(&ax->d, c->scaled_x2);
		if(c->x1 < xmin)  c->x1 = xmin;
		if(c->x1 >= xmax) c->x1 = xmax - 1;
		if(c->x2 <= xmin) c->x2 = xmin + 1;
		if(c->x2 > xmax)  c->x2 = xmax;
		c->scaled_x1 = scale_x(&ax->d, c->x1);
		c->scaled_x2 = scale_x(&ax->d, c->x2);
	}
*/
}

static void
DrawCursor(AxesWidget w, int ic)
{
	AxesPart *ax = &w->axes;
	CursorSegs(w, &ax->d, &ax->cursor[ic]);
	CursorLabels(w, ic);
}

static void
CursorSegs(AxesWidget w, DrawStruct *d, AxesCursor *c)
{
	AxesPart *ax = &w->axes;
	double x, x1, x2, y1, y2;
	int i, xmin, xmax, ymin, ymax;

	x = x1 = x2 = y1 = y2 = 0.0;
	i = xmin = xmax = ymin = ymax = 0;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	xmin = unscale_x(d, x1);
	xmax = unscale_x(d, x2);
	ymin = unscale_y(d, y2);
	ymax = unscale_y(d, y1);

	if(xmax < xmin) {
	    i = xmin;
	    xmin = xmax;
	    xmax = i;
	}
	if(ymax < ymin) {
	    i = ymin;
	    ymin = ymax;
	    ymax = i;
	}
	x = log10(fabs(ax->x2[ax->zoom] - ax->x1[ax->zoom]));
	c->nxdeci = (int)((x-4 > 0) ? 0 : -x+4);

	c->nsegs = 0;
	if(c->type == AXES_CROSSHAIR)
	{
	    if( (c->scaled_x < x1 && c->scaled_x < x2) ||
		(c->scaled_x > x1 && c->scaled_x > x2))
	    {
		c->x = c->x1 = -1;
	    }
	    else
	    {
		c->x = unscale_x(d, c->scaled_x);
		c->segs[0].x1 = c->x;
		c->segs[0].y1 = ymin;
		c->segs[0].x2 = c->x;
		c->segs[0].y2 = ymax;
		c->nsegs = 1;
	    }
	    if( (c->scaled_y < y1 && c->scaled_y < y2) ||
		(c->scaled_y > y1 && c->scaled_y > y2))
	    {
		c->y = -1;
	    }
	    else
	    {
		c->y = unscale_y(d, c->scaled_y);
		c->segs[c->nsegs].x1 = xmin;
		c->segs[c->nsegs].y1 = c->y;
		c->segs[c->nsegs].x2 = xmax;
		c->segs[c->nsegs].y2 = c->y;
		c->nsegs++;
	    }
	}
	else if(c->type == AXES_VERTICAL_LINE || c->type == AXES_PHASE_LINE)
	{
	    if( (c->scaled_x < x1 && c->scaled_x < x2) ||
		(c->scaled_x > x1 && c->scaled_x > x2))
	    {
		c->x = c->x1 = -1;
	    }
	    else
	    {
		c->x = unscale_x(d, c->scaled_x);
		c->segs[0].x1 = c->x;
		c->segs[0].y1 = ymin;
		c->segs[0].x2 = c->x;
		c->segs[0].y2 = ymax;
		c->nsegs = 1;
	    }
	    c->scaled_y = 0.;
	    c->y = -1;
	}
	else if(c->type == AXES_HORIZONTAL_LINE)
	{
	    c->scaled_x = 0;
	    c->x = c->x1 = -1;
	    if( (c->scaled_y < y1 && c->scaled_y < y2) ||
		(c->scaled_y > y1 && c->scaled_y > y2))
	    {
		c->y = -1;
	    }
	    else
	    {
		c->y = unscale_y(d, c->scaled_y);
		c->segs[0].x1 = xmin;
		c->segs[0].y1 = c->y;
		c->segs[0].x2 = xmax;
		c->segs[0].y2 = c->y;
		c->nsegs = 1;
	    }
	}
	else if(c->type == AXES_VER_DOUBLE_LINE)
	{
	    c->scaled_y = 0.;
	    c->y = -1;
	    if( (c->scaled_x1 < x1 && c->scaled_x1 < x2) ||
		(c->scaled_x1 > x1 && c->scaled_x1 > x2))
	    {
		c->x1 = -1;
	    }
	    else
	    {
		c->x1 = unscale_x(d, c->scaled_x1);
		if(c->x1 > xmin)
		{
		    c->segs[0].x1 = c->x1;
		    c->segs[0].x2 = c->x1;
		    c->segs[0].y1 = ymin;
		    c->segs[0].y2 = ymax;
		    c->nsegs++;
		}
		c->x = c->x1;
	    }
	    if( (c->scaled_x2 < x1 && c->scaled_x2 < x2) ||
		(c->scaled_x2 > x1 && c->scaled_x2 > x2))
	    {
		c->x2 = -1;
	    }
	    else
	    {
		c->x2 = unscale_x(d, c->scaled_x2);
		if(c->x2 == c->x1) c->x2++;
		if(c->x2 < xmax)
		{
		    c->segs[c->nsegs].x1 = c->x2;
		    c->segs[c->nsegs].x2 = c->x2;
		    c->segs[c->nsegs].y1 = ymin;
		    c->segs[c->nsegs].y2 = ymax;
		    c->nsegs++;
		}
	    }
	    ftoa(c->scaled_x2 - c->scaled_x1, c->nxdeci, 0, c->label+2, 48);

	    if(ax->double_line_anchor && c->x1 != -1)
	    {
		if(!c->anchor_on)
		{
		    c->segs[c->nsegs].x1 = c->x1-1;
		    c->segs[c->nsegs].x2 = c->x1-10;
		    c->segs[c->nsegs].y1 = ymin+1;
		    c->segs[c->nsegs].y2 = ymin+10;
		    c->nsegs++;

		    c->segs[c->nsegs].x1 = c->x1+1;
		    c->segs[c->nsegs].x2 = c->x1+10;
		    c->segs[c->nsegs].y1 = ymin+1;
		    c->segs[c->nsegs].y2 = ymin+10;
		    c->nsegs++;

		    c->segs[c->nsegs].x1 = c->x1-9;
		    c->segs[c->nsegs].x2 = c->x1-1;
		    c->segs[c->nsegs].y1 = ymin+10;
		    c->segs[c->nsegs].y2 = ymin+10;
		    c->nsegs++;

		    c->segs[c->nsegs].x1 = c->x1+1;
		    c->segs[c->nsegs].x2 = c->x1+9;
		    c->segs[c->nsegs].y1 = ymin+10;
		    c->segs[c->nsegs].y2 = ymin+10;
		    c->nsegs++;
		}
		else {
		    for(i = 0; i < 10; i++) {
			c->segs[c->nsegs].x1 = c->x1-(10-i);
			c->segs[c->nsegs].x2 = c->x1-(10-i);
			c->segs[c->nsegs].y1 = ymin+10;
			c->segs[c->nsegs].y2 = ymin+10-i;
			c->nsegs++;
		    }
		    for(i = 0; i < 10; i++) {
			c->segs[c->nsegs].x1 = c->x1+(10-i);
			c->segs[c->nsegs].x2 = c->x1+(10-i);
			c->segs[c->nsegs].y1 = ymin+10;
			c->segs[c->nsegs].y2 = ymin+10-i;
			c->nsegs++;
		    }
		}
	    }
	}
	else if(c->type == AXES_HOR_DOUBLE_LINE)
	{
	    c->scaled_x = 0.;
	    c->x = -1;
	    if( (c->scaled_y1 < x1 && c->scaled_y1 < y2) ||
		(c->scaled_y1 > x1 && c->scaled_y1 > y2))
	    {
		c->y1 = -1;
	    }
	    else
	    {
		c->y1 = unscale_y(d, c->scaled_y1);
		if(c->y1 > ymin)
		{
		    c->segs[0].y1 = c->y1;
		    c->segs[0].y2 = c->y1;
		    c->segs[0].x1 = xmin;
		    c->segs[0].x2 = xmax;
		    c->nsegs++;
		}
		c->y = c->y1;
	    }
	    if( (c->scaled_y2 < y1 && c->scaled_y2 < y2) ||
		(c->scaled_y2 > y1 && c->scaled_y2 > y2))
	    {
		c->y2 = -1;
	    }
	    else
	    {
		c->y2 = unscale_y(d, c->scaled_y2);
		if(c->y2 == c->y1) c->y2++;
		if(c->y2 < ymax)
		{
		    c->segs[c->nsegs].y1 = c->y2;
		    c->segs[c->nsegs].y2 = c->y2;
		    c->segs[c->nsegs].x1 = xmin;
		    c->segs[c->nsegs].x2 = xmax;
		    c->nsegs++;
		}
	    }
//	    ftoa(c->scaled_y2 - c->scaled_y1, c->nydeci, 0, c->label+2, 48);

	    if(ax->double_line_anchor && c->y1 != -1)
	    {
		if(!c->anchor_on)
		{
		    c->segs[c->nsegs].y1 = c->y1-1;
		    c->segs[c->nsegs].y2 = c->y1-10;
		    c->segs[c->nsegs].x1 = xmin+1;
		    c->segs[c->nsegs].x2 = xmin+10;
		    c->nsegs++;

		    c->segs[c->nsegs].y1 = c->y1+1;
		    c->segs[c->nsegs].y2 = c->y1+10;
		    c->segs[c->nsegs].x1 = xmin+1;
		    c->segs[c->nsegs].x2 = xmin+10;
		    c->nsegs++;

		    c->segs[c->nsegs].y1 = c->y1-9;
		    c->segs[c->nsegs].y2 = c->y1-1;
		    c->segs[c->nsegs].x1 = xmin+10;
		    c->segs[c->nsegs].x2 = xmin+10;
		    c->nsegs++;

		    c->segs[c->nsegs].y1 = c->y1+1;
		    c->segs[c->nsegs].y2 = c->y1+9;
		    c->segs[c->nsegs].x1 = xmin+10;
		    c->segs[c->nsegs].x2 = xmin+10;
		    c->nsegs++;
		}
		else {
		    for(i = 0; i < 10; i++) {
			c->segs[c->nsegs].y1 = c->y1-(10-i);
			c->segs[c->nsegs].y2 = c->y1-(10-i);
			c->segs[c->nsegs].x1 = xmin+10;
			c->segs[c->nsegs].x2 = xmin+10-i;
			c->nsegs++;
		    }
		    for(i = 0; i < 10; i++) {
			c->segs[c->nsegs].y1 = c->y1+(10-i);
			c->segs[c->nsegs].y2 = c->y1+(10-i);
			c->segs[c->nsegs].x1 = xmin+10;
			c->segs[c->nsegs].x2 = xmin+10-i;
			c->nsegs++;
		    }
		}
	    }
	}
}

static void
CursorLabels(AxesWidget w, int ic)
{
	AxesPart *ax = &w->axes;
	int i, lab_beg, lab_end, xbeg, xend, ybeg, yend, ic_bit, title_bit;
	int x_lab, y_lab, mar;
	int ascent, descent, direction, xmin, xmax, ymin, ymax;
	char *date;
	time_t clock;
	double y, x1, x2, y1, y2, scaled_y;
	Boolean xlabels, ylabels;
	XCharStruct overall;
	AxesCursor *c;

	c = ax->cursor;

	ic_bit = 1 << ic;
	title_bit = 1 << 30;

	if(!ax->display_axes) {
	    xlabels = ylabels = False;
	}
	else {
	    xlabels = ( ax->display_axes_labels == AXES_X ||
			ax->display_axes_labels == AXES_XY);
	    ylabels = ( ax->display_axes_labels == AXES_Y ||
			ax->display_axes_labels == AXES_XY);
	}

	c[ic].xlab[0] = '\0';
	c[ic].ylab[0] = '\0';
	c[ic].x_off = 0;
	c[ic].y_off = 0;
	c[ic].label_off = 0;

	if(c[ic].nsegs > 0)
	{
	    _AxesDrawSegments2(w, ax->invert_clipGC, (DSegment *)c[ic].segs,
				c[ic].nsegs);
	}
	if( !c[ic].draw_labels ) return;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	xmin = unscale_x(&ax->d, x1);
	xmax = unscale_x(&ax->d, x2);
	ymin = unscale_y(&ax->d, y2);
	ymax = unscale_y(&ax->d, y1);

	if(xmax < xmin) {
	    i = xmin;
	    xmin = xmax;
	    xmax = i;
	}
	if(ymax < ymin) {
	    i = ymin;
	    ymin = ymax;
	    ymax = i;
	}
	if((i=(int)strlen(c[ic].label)) > 0)
	{
	    XTextExtents(ax->font, c[ic].label, i, &direction, &ascent,
			&descent, &overall);
	    c[ic].label_width = overall.width;
	    c[ic].label_height = overall.ascent + overall.descent;
	    c[ic].label_ascent = overall.ascent;
	    if( c[ic].type == AXES_CROSSHAIR ||
		c[ic].type == AXES_VERTICAL_LINE ||
		c[ic].type == AXES_PHASE_LINE)
	    {
		x_lab = (int)((c[ic].x >= 0) ? c[ic].x-.5*overall.width : -1);
	    }
	    else
	    {
		if(c[ic].x1 < 0 && c[ic].x2 < 0) {
		    x_lab = -1;
		}
		else {
		    xbeg = (c[ic].x1 >= 0) ? c[ic].x1 : xmin;
		    xend = (c[ic].x2 >= 0) ? c[ic].x2 : xmax;
		    x_lab = (int)(.5*(xbeg + xend) - .5*overall.width);
		}
	    }
	    c[ic].label_x = x_lab;
	    if(c[ic].label_x > 0)
	    {
		mar = (int)((ax->tickmarks_inside) ?
			fabs(0.5*ax->a.xtic*ax->d.unscaley) :
			fabs(1.0*ax->a.xtic*ax->d.unscaley));
		if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM) {
		    y_lab = ymin - overall.descent - mar - 2;
		}
		else {
		    y_lab = ymax + overall.ascent + mar + 2;
		}
		if( !ax->save_space ) {
		    c[ic].label_y = y_lab;
		}
		else {
		    c[ic].label_y = ymin - overall.descent - 2;
		}
	    }
	}
	else
	{
	    c[ic].label_x = -1;
	}
	if(c[ic].type == AXES_CROSSHAIR)
	{
	    if(c[ic].x < 0) {
		c[ic].xlab[0] = '\0';
		
	    }
	    else if(ax->time_scale != TIME_SCALE_HMS)
	    {
		if(ax->a.log_x) {
		    ftoa(pow(10., c[ic].scaled_x), c[ic].nxdeci, 0,
				c[ic].xlab, 32);
		}
		else if(ax->transform != NULL)
		{
		    (*(ax->transform))(w, c[ic].scaled_x, c[ic].scaled_y,
				c[ic].xlab, c[ic].ylab);
		}
		else {
		    ftoa(c[ic].scaled_x, c[ic].nxdeci, 0, c[ic].xlab, 32);
		}
	    }
	    else if(c[ic].nxdeci > 0)
	    {
		strncpy(c[ic].xlab, timeEpochToGMT(c[ic].scaled_x,
				c[ic].nxdeci), 32);
	    }
	    else
	    {
		char buf[100];
		struct tm result;
		clock = (time_t)c[ic].scaled_x;
		clock = (c[ic].scaled_x < 0.0) ? (clock - 1) : clock;
		date = (char *)asctime_r(gmtime_r(&clock, &result), buf);
		strncpy(c[ic].xlab, date+11, 8);
		c[ic].xlab[8] = '\0';
	    }
	}
	else if(c[ic].type == AXES_VERTICAL_LINE ||
		c[ic].type == AXES_PHASE_LINE)
	{
	    if(c[ic].x < 0) {
		c[ic].xlab[0] = '\0';
	    }
	    else if(ax->time_scale != TIME_SCALE_HMS)
	    {
		if(ax->a.log_x)
		{
		    ftoa(pow(10., c[ic].scaled_x), c[ic].nxdeci, 0,
				c[ic].xlab, 32);
		}
		else if(ax->transform != NULL)
		{
		    (*(ax->transform))(w, c[ic].scaled_x, c[ic].scaled_y,
				c[ic].xlab, c[ic].ylab);
		}
		else {
		    ftoa(c[ic].scaled_x, c[ic].nxdeci, 0, c[ic].xlab, 32);
		}
	    }
	    else {
		strncpy(c[ic].xlab, timeEpochToGMT(c[ic].scaled_x,
				c[ic].nxdeci), 32);
	    }
	}
	else if(c[ic].type == AXES_VER_DOUBLE_LINE)
	{
	    if(c[ic].x1 < 0) {
		c[ic].xlab[0] = '\0';
	    }
	    else if(ax->time_scale != TIME_SCALE_HMS)
	    {
		if(ax->a.log_x) {
		    ftoa(pow(10., c[ic].scaled_x1), c[ic].nxdeci, 0,
				c[ic].xlab, 32);
		}
		else if(ax->transform != NULL) {
		    (*(ax->transform))(w, c[ic].scaled_x1, c[ic].scaled_y,
				c[ic].xlab, c[ic].ylab);
		}
		else {
		    ftoa(c[ic].scaled_x1, c[ic].nxdeci, 0, c[ic].xlab, 32);
		}
	    }
	    else {
		strncpy(c[ic].xlab, timeEpochToGMT(c[ic].scaled_x,
				c[ic].nxdeci), 32);
	    }
	}
	if(ax->display_axes_labels != AXES_X && 
	   ax->display_axes_labels != AXES_XY &&
	   ax->display_axes_labels != AXES_NO_XY)
	{
	    c[ic].xlab[0] = '\0';
	}
	if(c[ic].xlab[0] != '\0')
	{
	    XTextExtents(ax->font, c[ic].xlab, (int)strlen(c[ic].xlab),
				&direction, &ascent, &descent, &overall);
	    c[ic].x_width = overall.width;
	    c[ic].x_height = overall.ascent + overall.descent;
	    c[ic].x_xlab = (int)(c[ic].x - .5*c[ic].x_width);

	    if(c[ic].x_xlab < xmin) {
		c[ic].x_xlab = xmin;
	    }
	    if(c[ic].x_xlab + overall.width > xmax) {
		c[ic].x_xlab = xmax - overall.width;
	    }
	    c[ic].y_xlab = ax->a.y_xlab + overall.ascent;
	}
	if(c[ic].y >= 0)
	{
	    scaled_y = (ax->a.y_label_int) ?
				(int)(c[ic].scaled_y + .5) : c[ic].scaled_y;
	    if(ax->a.log_y)
	    {
		y = log10(fabs(pow(10., ax->y2[ax->zoom]) -
				pow(10., ax->y1[ax->zoom])));
		c[ic].nydeci = (int)((y-4 > 0) ? 0 : -y+4);
		ftoa(pow(10., scaled_y), c[ic].nydeci, 0, c[ic].ylab, 32);
	    }
	    else if(ax->transform != NULL)
	    {
		(*(ax->transform))(w, c[ic].scaled_x, scaled_y, c[ic].xlab,
				c[ic].ylab);
	    }
	    else
	    {
		y = log10(fabs(ax->y2[ax->zoom] - ax->y1[ax->zoom]));
		c[ic].nydeci = (int)((y-4 > 0) ? 0 : -y+4);
		ftoa(scaled_y, c[ic].nydeci, 0, c[ic].ylab, 32);
	    }

	    XTextExtents(ax->font, c[ic].ylab, (int)strlen(c[ic].ylab),
			&direction, &ascent, &descent, &overall);
	    c[ic].y_width = overall.width;
	    c[ic].y_height = overall.ascent + overall.descent;
	    c[ic].y_ylab = (int)(c[ic].y + .5*overall.ascent);
	    c[ic].x_ylab = ax->a.r_ylab - overall.width;
	}
	if(ax->display_axes_labels != AXES_Y && 
	   ax->display_axes_labels != AXES_XY &&
	   ax->display_axes_labels != AXES_NO_XY)
	{
	    c[ic].ylab[0] = '\0';
	}
	if(c[ic].label_x < 0)
	{
	    for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	    {
		if(c[i].label_off == ic_bit && c[i].label[0] != '\0')
		{
		    _AxesDrawString2(w, ax->axesGC,c[i].label_x, c[i].label_y,
				c[i].label);
		}
		c[i].label_off &= ~ic_bit;
	    }
	}
	else
	{
	    /*
	     * check if the cursor[ic].label is overlapping another
	     * crosshair label
	     */
	    lab_beg = c[ic].label_x;
	    lab_end = lab_beg + c[ic].label_width;
	    for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	    {
		xbeg = c[i].label_x;
		xend = xbeg + c[i].label_width;
		if(	(lab_beg < xbeg && lab_end > xend) ||
			(lab_beg >= xbeg && lab_beg <= xend) ||
			(lab_end >= xbeg && lab_end <= xend) )
		{
		    if(!c[i].label_off)
		    {
			_AxesClearArea2(w, xbeg-1,
				c[i].label_y - c[i].label_ascent,
				c[i].label_width+2, c[i].label_height);
		    }
		    c[i].label_off |= ic_bit;
		}
		else
		{
		    if(c[i].label_off == ic_bit && c[i].label[0] != '\0')
		    {
			_AxesDrawString2(w, ax->axesGC, c[i].label_x,
					c[i].label_y, c[i].label);
		    }
		    c[i].label_off &= ~ic_bit;
		}
	    }
	    /*
	     * check if the cursor[ic].label is overlapping the title
	     */
	    if( ax->save_space && ax->title[0] != '\0')
	    {
		if((c[ic].label_x > ax->title_x + ax->title_width) ||
		   (c[ic].label_x + c[ic].label_width < ax->title_x))
		{
		    _AxesDrawString2(w, ax->axesGC, c[ic].label_x,
					c[ic].label_y, c[ic].label);
		}
		else {
		    c[ic].label_off |= title_bit;
		}
	    }
	    else {
		_AxesDrawString2(w, ax->axesGC, c[ic].label_x,c[ic].label_y,
			c[ic].label);
	    }
	}
	if(c[ic].xlab[0] == '\0')
	{
	    for(i = 0; i < ax->a.nxlab; i++)
	    {
		if(ax->a.xlab_off[i] == ic_bit && xlabels)
		{
		    drawOneXLabel(w, ax, i);
		}
		ax->a.xlab_off[i] &= ~ic_bit;
	    }
	    for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	    {
		if(c[i].x_off == ic_bit && c[i].xlab[0] != '\0')
		{
		    _AxesDrawString2(w, ax->axesGC, c[i].x_xlab,c[i].y_xlab,
				c[i].xlab);
		}
		c[i].x_off &= ~ic_bit;
	    }
	}
	else
	{
	    /*
	     * check if the cursor[ic].xlab is overlapping an x_label.
	     */
	    lab_beg = c[ic].x_xlab;
	    lab_end = lab_beg + c[ic].x_width;
	    for(i = 0; i < ax->a.nxlab; i++)
	    {
		xbeg = ax->a.x_xlab[i];
		xend = xbeg + ax->a.xlab_width[i];
		if((lab_beg < xbeg && lab_end > xend) ||
		   (lab_beg >= xbeg && lab_beg <= xend) ||
		   (lab_end >= xbeg && lab_end <= xend) )
		{
		    if(!ax->a.xlab_off[i])
		    {
			_AxesClearArea2(w, xbeg, ax->a.y_xlab,
				ax->a.xlab_width[i], ax->a.max_xlab_height+1);
		    }
		    ax->a.xlab_off[i] |= ic_bit;
		}
		else
		{
		    if(ax->a.xlab_off[i] == ic_bit && xlabels)
		    {
			drawOneXLabel(w, ax, i);
		    }
		    ax->a.xlab_off[i] &= ~ic_bit;
		}
	    }
	    /*
	     * check if the cursor[ic].xlab is overlapping another
	     * crosshair x_label
	     */
	    lab_beg = c[ic].x_xlab;
	    lab_end = lab_beg + c[ic].x_width;
	    for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	    {
		xbeg = c[i].x_xlab;
		xend = xbeg + c[i].x_width;
		if( (lab_beg < xbeg && lab_end > xend) ||
		    (lab_beg >= xbeg && lab_beg <= xend) ||
		    (lab_end >= xbeg && lab_end <= xend) )
		{
		    if(!c[i].x_off)
		    {
			_AxesClearArea2(w, xbeg, c[i].y_xlab - c[i].x_height,
					c[i].x_width, c[i].x_height);
		    }
		    c[i].x_off |= ic_bit;
		}
		else
		{
		    if(c[i].x_off == ic_bit && c[i].xlab[0] != '\0')
		    {
			_AxesDrawString2(w, ax->axesGC, c[i].x_xlab,c[i].y_xlab,
						c[i].xlab);
		    }
		    c[i].x_off &= ~ic_bit;
		}
	    }
	    _AxesDrawString2(w, ax->axesGC, c[ic].x_xlab, c[ic].y_xlab,
			c[ic].xlab);
	}
	if(c[ic].type == AXES_VERTICAL_LINE || c[ic].type == AXES_PHASE_LINE ||
		c[ic].type == AXES_VER_DOUBLE_LINE)
	{
	    c[ic].y_off = 1 << MAX_CURSORS;
	    return;
	}
	if(c[ic].y < 0)
	{
	    c[ic].ylab[0] = '\0';
	    for(i = 0; i < ax->a.nylab; i++)
	    {
		if(ax->a.ylab_off[i] == ic_bit && ylabels)
		{
		    drawOneYLabel(w, ax, i);
		}
		ax->a.ylab_off[i] &= ~ic_bit;
	    }
	    for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	    {
		if(c[i].y_off == ic_bit && c[i].ylab[0] != '\0')
		{
		    _AxesDrawString2(w, ax->axesGC, c[i].x_ylab, c[i].y_ylab,
				c[i].ylab);
		}
		c[i].y_off &= ~ic_bit;
	    }
	}
	else if(c[ic].ylab[0] != '\0')
	{
	    /*
	     * check if the cursor[ic].ylab is overlapping a y_label.
	     */
	    XTextExtents(ax->font, c[ic].ylab, (int)strlen(c[ic].ylab),
				&direction, &ascent, &descent, &overall);
	    lab_beg = c[ic].y_ylab - overall.ascent;
	    lab_end = lab_beg + overall.ascent + overall.descent;
	    for(i = 0; i < ax->a.nylab; i++) if(ax->a.ylab[i] != NULL)
	    {
		ybeg = ax->a.y_ylab[i] - ax->a.ylab_ascent[i];
		yend = ybeg + ax->a.ylab_height[i];

		if( (lab_beg < ybeg && lab_end > yend) ||
		    (lab_beg >= ybeg && lab_beg <= yend) ||
		    (lab_end >= ybeg && lab_end <= yend) )
		{
		    if(!ax->a.ylab_off[i])
		    {
			_AxesClearArea2(w, ax->a.r_ylab - ax->a.max_ylab_width,
			    ybeg, ax->a.max_ylab_width, ax->a.ylab_height[i]);
		    }
		    ax->a.ylab_off[i] |= ic_bit;
		}
		else
		{
		    if(ax->a.ylab_off[i] == ic_bit && ylabels)
		    {
			drawOneYLabel(w, ax, i);
		    }
		    ax->a.ylab_off[i] &= ~ic_bit;
		}
	    }
	    /*
	     * check if the cursor[ic].ylab is overlapping another
	     * crosshair y_label
	     */
	    lab_beg = c[ic].y_ylab - overall.ascent;
	    lab_end = lab_beg +  c[ic].y_height;
	    for(i = 0; i < ax->num_cursors; i++) if(i != ic)
	    {
		ybeg = c[i].y_ylab - c[i].y_height;
		yend = ybeg + c[i].y_height;
		if( (lab_beg < ybeg && lab_end > yend) ||
		    (lab_beg >= ybeg && lab_beg <= yend) ||
		    (lab_end >= ybeg && lab_end <= yend) )
		{
		    if(!c[i].y_off)
		    {
			_AxesClearArea2(w, ax->a.r_ylab - c[i].y_width,
				    ybeg, c[i].y_width, c[i].y_height);
		    }
		    c[i].y_off |= ic_bit;
		}
		else
		{
		    if(c[i].y_off == ic_bit && c[i].ylab[0] != '\0')
		    {
			_AxesDrawString2(w, ax->axesGC, c[i].x_ylab,
					c[i].y_ylab, c[i].ylab);
		    }
		    c[i].y_off &= ~ic_bit;
		}
	    }
	    _AxesDrawString2(w, ax->axesGC, c[ic].x_ylab, c[ic].y_ylab,
			c[ic].ylab);
	}
}

static void
ClearCursor(AxesWidget w, int m)
{
	AxesPart *ax = &w->axes;
	AxesCursor *c;

	c = &ax->cursor[m];
	if(c->nsegs > 0)
	{
	    _AxesDrawSegments2(w, ax->invert_clipGC, (DSegment *)c->segs,
				c->nsegs);
	}
	if(c->label[0] != '\0' && !c->label_off)
	{
	    _AxesClearArea2(w, c->label_x-1, c->label_y - c->label_ascent,
			c->label_width+2, c->label_height);
	}
	if(c->xlab[0] != '\0' && !c->x_off)
	{
	    _AxesClearArea2(w, c->x_xlab, c->y_xlab - c->x_height,
			c->x_width, c->x_height);
	}
	if(c->ylab[0] != '\0' && !c->y_off)
	{
	    _AxesClearArea2(w, ax->a.r_ylab - c->y_width,
			c->y_ylab - c->y_height, c->y_width, c->y_height);
	}
}

/** 
 *  
 */
void
_AxesMove(AxesWidget w, XEvent *event, const char **params,Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int i, cursor_x, cursor_y, delx, dely, grab;
	static Boolean motion;
	AxesCursor *c;
	AxesWidget z;
	float d;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(event->type == ButtonPress)
	{
	    if(ax->cursor_mode == AXES_SELECT_CURSOR) {
		if(ax->double_line_anchor && !strcmp(ax->cursor_type, "hand"))
		{
		    i = _AxesWhichCursor(w, cursor_x, cursor_y, &d, &grab);
		    if(i >= 0) {
			DrawCursor(w, i);
			ax->cursor[i].anchor_on = !ax->cursor[i].anchor_on;
			DrawCursor(w, i);
		    }
		}
		else {
		    ax->m = _AxesWhichCursor(w, cursor_x, cursor_y, &d, &grab);
		    motion = False;
		}
	    }
	    else if(ax->cursor_mode == AXES_MOVE_MAGNIFY) {
		ax->last_cursor_x = cursor_x;
		ax->last_cursor_y = cursor_y;
	    }
	}
	else if(event->type == MotionNotify && ax->m >= 0)
	{
	    motion = True;
	    c = &ax->cursor[ax->m];

	    delx = cursor_x - ax->last_cursor_x;
	    dely = cursor_y - ax->last_cursor_y;

	    if(delx == 0 && dely == 0) {
		return;
	    }
	    ax->no_motion = False;

	    ClearCursor(w, ax->m);

	    if(c->type == AXES_CROSSHAIR) {
		c->scaled_x += delx*ax->d.scalex;
		c->scaled_y += dely*ax->d.scaley;
		c->scaled_x1 = c->scaled_x2 = c->scaled_x;
	    }
	    else if(c->type == AXES_VERTICAL_LINE ||
		    c->type == AXES_HORIZONTAL_LINE ||
		    c->type == AXES_PHASE_LINE)
	    {
		c->scaled_x += delx*ax->d.scalex;
		c->scaled_x1 = c->scaled_x2 = c->scaled_x;
	    }
	    else if(ax->scaling_double_line == 0) {
		if(c->type == AXES_VER_DOUBLE_LINE) {
		    c->scaled_x1 += delx*ax->d.scalex;
		    c->scaled_x2 += delx*ax->d.scalex;
		    c->scaled_x = c->scaled_x1;
		}
		else if(c->type == AXES_HOR_DOUBLE_LINE) {
		    c->scaled_y1 += dely*ax->d.scaley;
		    c->scaled_y2 += dely*ax->d.scaley;
		    c->scaled_y = c->scaled_y1;
		}
	    }
	    else if(ax->scaling_double_line == 1)
	    {
		if(c->type == AXES_VER_DOUBLE_LINE) {
		    c->scaled_x1 += delx*ax->d.scalex;
		    if(c->scaled_x1 > c->scaled_x2) {
			c->scaled_x1 = c->scaled_x2 - ax->d.scalex;
		    }
		    c->scaled_x = c->scaled_x1;
		}
		else if(c->type == AXES_HOR_DOUBLE_LINE) {
		    c->scaled_y1 += dely*ax->d.scaley;
		    if(c->scaled_y1 > c->scaled_y2) {
			c->scaled_y1 = c->scaled_y2 - ax->d.scaley;
		    }
		    c->scaled_y = c->scaled_y1;
		}
	    }
	    else if(ax->scaling_double_line == 2)
	    {
		if(c->type == AXES_VER_DOUBLE_LINE) {
		    c->scaled_x2 += delx*ax->d.scalex;
		    if(c->scaled_x2 < c->scaled_x1) {
			c->scaled_x2 = c->scaled_x1 + ax->d.scalex;
		    }
		}
		else if(c->type == AXES_HOR_DOUBLE_LINE) {
		    c->scaled_y2 += dely*ax->d.scaley;
		    if(c->scaled_y2 < c->scaled_y1) {
			c->scaled_y2 = c->scaled_y1 + ax->d.scaley;
		    }
		}
	    }
	    ConstrainCursor(w, ax->m);
	    DrawCursor(w, ax->m);

	    if((z = (AxesWidget)ax->mag_from) != NULL)
	    {
		ClearCursor(z, ax->m);
		z->axes.cursor[ax->m].scaled_x  = c->scaled_x;
		z->axes.cursor[ax->m].scaled_y  = c->scaled_y;
		z->axes.cursor[ax->m].scaled_x1 = c->scaled_x1;
		z->axes.cursor[ax->m].scaled_x2 = c->scaled_x2;
		strncpy(z->axes.cursor[ax->m].label,
			ax->cursor[ax->m].label, 50);
		DrawCursor(z, ax->m);
	    }
	    if((z = (AxesWidget)ax->mag_to) != NULL)
	    {
		ClearCursor(z, ax->m);
		z->axes.cursor[ax->m].scaled_x  = c->scaled_x;
		z->axes.cursor[ax->m].scaled_y  = c->scaled_y;
		z->axes.cursor[ax->m].scaled_x1 = c->scaled_x1;
		z->axes.cursor[ax->m].scaled_x2 = c->scaled_x2;
		strncpy(z->axes.cursor[ax->m].label,
			ax->cursor[ax->m].label, 50);
		DrawCursor(z, ax->m);
	    }
	    z = (ax->mag_from == NULL) ? w : (AxesWidget)ax->mag_from;

	    if(z->axes.cursor_func != NULL) {
		(*(z->axes.cursor_func))(z, event, c);
	    }

	    if(ax->scaling_double_line) {
		DoCursorCallback(w, c, 2);
	    }
	    else {
		DoCursorCallback(w, c, 1);
	    }

	    ax->last_cursor_x = cursor_x;
	    ax->last_cursor_y = cursor_y;
	}
	else if(event->type == MotionNotify &&
		ax->cursor_mode == AXES_MOVE_MAGNIFY)
	{
	    AxesWidget z = (AxesWidget)ax->mag_to;
	    double delx = (cursor_x - ax->last_cursor_x)*ax->d.scalex;
	    double dely = (cursor_y - ax->last_cursor_y)*ax->d.scaley;
	    double x, y, dx, dy;
	    if(z == NULL) return;

	    y = ax->y1[ax->zoom];
	    if(_AxesDragScroll(w, event)) {
		dy = ax->y1[ax->zoom] - y;
		y =  z->axes.y1[z->axes.zoom] + dy;
		if(y < z->axes.y1[0]) y = z->axes.y1[0];
		if(y > z->axes.y2[0]) y = z->axes.y2[0];
		dy = z->axes.y2[z->axes.zoom] - z->axes.y1[z->axes.zoom];
		z->axes.y1[z->axes.zoom] = y;
		z->axes.y2[z->axes.zoom] = y + dy;
	    }
	    x = z->axes.x1[z->axes.zoom] + delx;
	    if(x < z->axes.x1[0]) x = z->axes.x1[0];
	    if(x > z->axes.x2[0]) x = z->axes.x2[0];
	    y = z->axes.y1[z->axes.zoom] + dely;
	    if(y < z->axes.y1[0]) y = z->axes.y1[0];
	    if(y > z->axes.y2[0]) y = z->axes.y2[0];
	    
	    dx = z->axes.x2[z->axes.zoom] - z->axes.x1[z->axes.zoom];
	    dy = z->axes.y2[z->axes.zoom] - z->axes.y1[z->axes.zoom];
	    z->axes.x1[z->axes.zoom] = x;
	    z->axes.x2[z->axes.zoom] = x + dx;
	    z->axes.y1[z->axes.zoom] = y;
	    z->axes.y2[z->axes.zoom] = y + dy;
	    DoMagFrom(z);
	    Redraw(z);
	    Redisplay((Widget)z, NULL, NULL);
	}
	else if(event->type == MotionNotify &&
		ax->cursor_mode == AXES_STRETCH_MAGNIFY)
	{
	    AxesWidget z = (AxesWidget)ax->mag_to;
	    double delx = (cursor_x - ax->last_cursor_x)*ax->d.scalex;
	    double dely = (cursor_y - ax->last_cursor_y)*ax->d.scaley;
	    double x, y;
	    if(z == NULL) return;
	    switch(ax->stretch_magnify)
	    {
		case 0:
		    x = z->axes.x1[z->axes.zoom] + delx;
		    if(x < z->axes.x1[0]) x = z->axes.x1[0];
		    if(x > z->axes.x2[0]) x = z->axes.x2[0];
		    z->axes.x1[z->axes.zoom] = x;
		    y = z->axes.y1[z->axes.zoom] + dely;
		    if(y < z->axes.y1[0]) y = z->axes.y1[0];
		    if(y > z->axes.y2[0]) y = z->axes.y2[0];
		    z->axes.y1[z->axes.zoom] = y;
		    break;
		case 1:
		    x = z->axes.x2[z->axes.zoom] + delx;
		    if(x < z->axes.x1[0]) x = z->axes.x1[0];
		    if(x > z->axes.x2[0]) x = z->axes.x2[0];
		    z->axes.x2[z->axes.zoom] = x;
		    y = z->axes.y1[z->axes.zoom] + dely;
		    if(y < z->axes.y1[0]) y = z->axes.y1[0];
		    if(y > z->axes.y2[0]) y = z->axes.y2[0];
		    z->axes.y1[z->axes.zoom] = y;
		    break;
		case 2:
		    x = z->axes.x2[z->axes.zoom] + delx;
		    if(x < z->axes.x1[0]) x = z->axes.x1[0];
		    if(x > z->axes.x2[0]) x = z->axes.x2[0];
		    z->axes.x2[z->axes.zoom] = x;
		    y = z->axes.y2[z->axes.zoom] + dely;
		    if(y < z->axes.y1[0]) y = z->axes.y1[0];
		    if(y > z->axes.y2[0]) y = z->axes.y2[0];
		    z->axes.y2[z->axes.zoom] = y;
		    break;
		case 3:
		    x = z->axes.x1[z->axes.zoom] + delx;
		    if(x < z->axes.x1[0]) x = z->axes.x1[0];
		    if(x > z->axes.x2[0]) x = z->axes.x2[0];
		    z->axes.x1[z->axes.zoom] = x;
		    y = z->axes.y2[z->axes.zoom] + dely;
		    if(y < z->axes.y1[0]) y = z->axes.y1[0];
		    if(y > z->axes.y2[0]) y = z->axes.y2[0];
		    z->axes.y2[z->axes.zoom] = y;
		    break;
	    }
	    DoMagFrom(z);
	    Redraw(z);
	    Redisplay((Widget)z, NULL, NULL);
	}
	else if(event->type == ButtonRelease && ax->m >= 0)
	{
	    c = &ax->cursor[ax->m];
	    if(!motion)
	    {
		ClearCursor(w, ax->m);
		DrawCursor(w, ax->m);
		if((z = (AxesWidget)ax->mag_from) != NULL) {
		    ClearCursor(z, ax->m);
		    DrawCursor(z, ax->m);
		}
		if((z = (AxesWidget)ax->mag_to) != NULL)
		{
		    ClearCursor(z, ax->m);
		    DrawCursor(z, ax->m);
		}
	    }

	    z = (ax->mag_from == NULL) ? w : (AxesWidget)ax->mag_from;

	    if(z->axes.cursor_func != NULL) {
		(*(z->axes.cursor_func))(z, event, c);
	    }

	    DoCursorCallback(w, c, 0);
			
	    ax->scaling_double_line = 0;
	    ax->m = -1;
	}
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;
}

/** 
 *  
 */
Boolean
_AxesMotion(AxesWidget w, XEvent *event, String *params, Cardinal *num_params)
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
	    return False;
	}

	return SetCursorMode(w, x, y);
}

static Boolean
SetCursorMode(AxesWidget w, int x, int y)
{
	AxesPart *ax = &w->axes;
	int grab;
	float distance;

	if(x < ax->clipx1 || x > ax->clipx2 || y < ax->clipy1 || y > ax->clipy2)
	{
	    _AxesSetCursor(w, "default");
	    return False;
	}

	distance = _AxesMagDistance(w, x, y);
	if(distance >= 0 && distance < 5)  {
	    if(ax->cursor_mode != AXES_MOVE_MAGNIFY) {
		ax->cursor_mode = AXES_MOVE_MAGNIFY;
		_AxesSetCursor(w, "move");
		return True;
	    }
	    return False;
	}
	else if(ax->cursor_mode == AXES_MOVE_MAGNIFY) {
	    ax->cursor_mode = AXES_ZOOM;
	    _AxesSetCursor(w, "default");
	}

	if( !ax->allow_cursor_change ) return False;

	if(_AxesWhichCursor(w, x, y, &distance, &grab) >= 0)
	{
	    if(distance < 5)
	    {
		if(!ax->double_line_anchor || grab != 5 || y > ax->clipy1+10) {
		    if(ax->cursor_mode != AXES_SELECT_CURSOR
			 || !strcmp(ax->cursor_type, "hand"))
		    {
			ax->cursor_mode = AXES_SELECT_CURSOR;
			_AxesSetCursor(w, "move");
			return True;
		    }
		}
		else if(ax->double_line_anchor) {
		    ax->cursor_mode = AXES_SELECT_CURSOR;
		    _AxesSetCursor(w, "hand");
		    return True;
		}
	    }
	    else if(ax->cursor_mode == AXES_SELECT_CURSOR) {
		ax->cursor_mode = AXES_ZOOM;
		_AxesSetCursor(w, "default");
	    }
	}
	return False;
}

/** 
 *  
 */
static double
_AxesMagDistance(AxesWidget w, int cursor_x, int cursor_y)
{
	int mag_x1, mag_x2, mag_y1, mag_y2;
	float mx1, mx2, my1, my2;
	double d, dmin;
	float cx = (float)cursor_x;
	float cy = (float)cursor_y;

	if(!AxesFormMagRect(w, &mag_x1, &mag_x2, &mag_y1, &mag_y2)) return -1;

	mx1 = (float)mag_x1;
	mx2 = (float)mag_x2;
	my1 = (float)mag_y1;
	my2 = (float)mag_y2;
	dmin = AxesClass::pointToLine(mx1, my1, mx2, my1, cx, cy);
	d = AxesClass::pointToLine(mx1, my2, mx2, my2, cx, cy);
	if(d < dmin) {
	    dmin = d;
	}
	d = AxesClass::pointToLine(mx1, my1, mx1, my2, cx, cy);
	if(d < dmin) {
	    dmin = d;
	}
	d = AxesClass::pointToLine(mx2, my1, mx2, my2, cx, cy);
	if(d < dmin) {
	    dmin = d;
	}
	return dmin;
}

/** 
 *  
 */
void
_AxesScale(AxesWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int cursor_x, cursor_y, grab;
	double x, y, x1, x2, y1, y2;
	float d;

	if(event->type == ButtonPress)
	{
	    if(ax->cursor_mode == AXES_SELECT_CURSOR)
	    {
		cursor_x = ((XButtonEvent *)event)->x;
		cursor_y = ((XButtonEvent *)event)->y;
		ax->m = _AxesWhichCursor(w, cursor_x, cursor_y, &d, &grab);
		if(ax->m >= 0)
		{
		    if(ax->cursor[ax->m].type == AXES_VER_DOUBLE_LINE) {
			if( abs(cursor_x - ax->cursor[ax->m].x1) <
			    abs(cursor_x - ax->cursor[ax->m].x2))
			{
			    ax->scaling_double_line = 1;
			}
			else {
			    ax->scaling_double_line = 2;
			}
		    }
		    else if(ax->cursor[ax->m].type == AXES_HOR_DOUBLE_LINE) {
			if( abs(cursor_y - ax->cursor[ax->m].y1) <
			    abs(cursor_y - ax->cursor[ax->m].y2))
			{
			    ax->scaling_double_line = 1;
			}
			else {
			    ax->scaling_double_line = 2;
			}
		    }
		    ax->last_cursor_x = cursor_x;
		    ax->last_cursor_y = cursor_y;
		}
		else {
		    ax->scaling_double_line = 0;
		}
	    }
	    else if(ax->cursor_mode == AXES_MOVE_MAGNIFY)
	    {
		double dmin, dist;
		AxesWidget z = (AxesWidget)ax->mag_to;
		if(z == NULL) return;
		ax->cursor_mode = AXES_STRETCH_MAGNIFY;
		cursor_x = ((XButtonEvent *)event)->x;
		cursor_y = ((XButtonEvent *)event)->y;
		x = scale_x(&ax->d, cursor_x);
		y = scale_y(&ax->d, cursor_y);
		x1 = z->axes.x1[z->axes.zoom];
		x2 = z->axes.x2[z->axes.zoom];
		y1 = z->axes.y1[z->axes.zoom];
		y2 = z->axes.y2[z->axes.zoom];

		dmin = pow(x-x1, 2.) + pow(y-y1, 2.);
		ax->stretch_magnify = 0;

		dist = pow(x-x2, 2.) + pow(y-y1, 2.);
		if(dist < dmin) {
		    dmin = dist;
		    ax->stretch_magnify = 1;
		}
		dist = pow(x-x2, 2.) + pow(y-y2, 2.);
		if(dist < dmin) {
		    dmin = dist;
		    ax->stretch_magnify = 2;
		}
		dist = pow(x-x1, 2.) + pow(y-y2, 2.);
		if(dist < dmin) {
		    dmin = dist;
		    ax->stretch_magnify = 3;
		}
		ax->last_cursor_x = cursor_x;
		ax->last_cursor_y = cursor_y;
	    }
	}
	else
	{
	    _AxesMove(w, event, (const char **)params, num_params);
	    ax->no_motion = False;
	}
}

static void
DoCursorCallback(AxesWidget widget, AxesCursor *c, int reason)
{
	AxesCursorCallbackStruct p;
	AxesWidget w;
	int m;

	w = (widget->axes.mag_from == NULL) ?
		widget : (AxesWidget)widget->axes.mag_from;

	for(m = 0; m < w->axes.num_cursors; m++) {
		if(c == &w->axes.cursor[m]) break;
	}
	if(m == w->axes.num_cursors) return;

	p.index		= m;
	p.type		= c->type;
	p.x		= c->x;
	p.y		= c->y;
	p.x1		= c->x1;
	p.x2		= c->x2;
	p.y1		= c->y1;
	p.y2		= c->y2;
	p.scaled_x	= c->scaled_x;
	p.scaled_y	= c->scaled_y;
	p.scaled_x1	= c->scaled_x1;
	p.scaled_x2	= c->scaled_x2;
	p.scaled_y1	= c->scaled_y1;
	p.scaled_y2	= c->scaled_y2;

	if(c->type == AXES_CROSSHAIR)
	{
	    strcpy(p.label, c->label);
	    if(reason == 0)
	    {
		p.reason = AXES_CROSSHAIR_POSITION;
		if(XtHasCallbacks((Widget)w, XtNcrosshairCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNcrosshairCallback, &p);
		}
	    }
	    else if(reason == 1)
	    {
		p.reason = AXES_CROSSHAIR_DRAG;
		if(XtHasCallbacks((Widget)w, XtNcrosshairDragCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNcrosshairDragCallback,&p);
		}
	    }
	}
	else if(c->type == AXES_HORIZONTAL_LINE || c->type ==AXES_VERTICAL_LINE)
	{
	    p.label[0] =  c->label[0];
	    p.label[1] = '\0';
	    if(reason == 0)
	    {
		p.reason = AXES_LINE_POSITION;
		if(XtHasCallbacks((Widget)w, XtNlineCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNlineCallback, &p);
		}
	    }
	    else if(reason == 1)
	    {
		p.reason = AXES_LINE_DRAG;
		if(XtHasCallbacks((Widget)w, XtNlineDragCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNlineDragCallback, &p);
		}
	    }
	}
	else if(c->type == AXES_HOR_DOUBLE_LINE ||
		c->type == AXES_VER_DOUBLE_LINE)
	{
	    p.label[0] =  c->label[0];
	    p.label[1] = '\0';
	    if(reason == 0)
	    {
		p.reason = AXES_DOUBLE_LINE_POSITION;
		if(XtHasCallbacks((Widget)w, XtNdoubleLineCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNdoubleLineCallback, &p);
		}
	    }
	    else if(reason == 1)
	    {
		p.reason = AXES_DOUBLE_LINE_DRAG;
		if(XtHasCallbacks((Widget)w, XtNdoubleLineDragCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNdoubleLineDragCallback, &p);
		}
	    }
	    else if(reason == 2)
	    {
		p.reason = AXES_DOUBLE_LINE_SCALE;
		if(XtHasCallbacks((Widget)w, XtNdoubleLineScaleCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNdoubleLineScaleCallback, &p);
		}
	    }
	}
	else if(c->type == AXES_PHASE_LINE)
	{
	    strncpy(p.label, c->label, 50);
	    if(reason == 0)
	    {
		p.reason = AXES_PHASE_LINE_POSITION;
		if(XtHasCallbacks((Widget)w, XtNphaseLineCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNphaseLineCallback, &p);
		}
	    }
	    else if(reason == 1)
	    {
		p.reason = AXES_PHASE_LINE_DRAG;
		if(XtHasCallbacks((Widget)w, XtNphaseLineDragCallback) != 
					XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNphaseLineDragCallback, &p);
		}
	    }
	}
}

/** 
 *  
 */
int
_AxesWhichCursor(AxesWidget w, int cursor_x, int cursor_y, float *d_min,
		int *grab)
{
	AxesPart *ax = &w->axes;
	int	m, which, x, y;
	float	d, dmin;
	double	x1, x2, y1, y2;

	/* initialize dmin to a large value before seeking the minimim.
	 */
	dmin = 2.*((int)w->core.width*(int)w->core.width +
			(int)w->core.height*(int)w->core.height);
	which = -3;

	x1 = x2 = y1 = y2 = 0.0;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];

	*grab = 0;
	for(m = 0; m < ax->num_cursors; m++)
	{
	    if(ax->cursor[m].type == AXES_CROSSHAIR)
	    {
		if((x1 <= ax->cursor[m].scaled_x && 
		    x2 >= ax->cursor[m].scaled_x) ||
		   (x1 >= ax->cursor[m].scaled_x && 
		    x2 <= ax->cursor[m].scaled_x))
		{
		    x = unscale_x(&ax->d, ax->cursor[m].scaled_x);
		    d = abs(cursor_x - x);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 1;
		    }
		}
		if((y1 <= ax->cursor[m].scaled_y && 
		    y2 >= ax->cursor[m].scaled_y) ||
		   (y1 >= ax->cursor[m].scaled_y && 
		    y2 <= ax->cursor[m].scaled_y))
		{
		    y = unscale_y(&ax->d, ax->cursor[m].scaled_y);
		    d = abs(cursor_y - y);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 2;
		    }
		}
	    }
	    else if(ax->cursor[m].type == AXES_VERTICAL_LINE ||
		    ax->cursor[m].type == AXES_PHASE_LINE)
	    {
		if((x1 <= ax->cursor[m].scaled_x && 
		    x2 >= ax->cursor[m].scaled_x) ||
		   (x1 >= ax->cursor[m].scaled_x && 
		    x2 <= ax->cursor[m].scaled_x))
		{
		    x = unscale_x(&ax->d, ax->cursor[m].scaled_x);
		    d = abs(cursor_x - x);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 3;
		    }
		}
	    }
	    else if(ax->cursor[m].type == AXES_HORIZONTAL_LINE)
	    {
		if((y1 <= ax->cursor[m].scaled_y && 
		    y2 >= ax->cursor[m].scaled_y) ||
		   (y1 >= ax->cursor[m].scaled_y && 
		    y2 <= ax->cursor[m].scaled_y))
		{
		    y = unscale_y(&ax->d, ax->cursor[m].scaled_y);
		    d = abs(cursor_y - y);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 4;
		    }
		}
	    }
	    else if(ax->cursor[m].type == AXES_VER_DOUBLE_LINE)
	    {
		if((x1 <= ax->cursor[m].scaled_x1 && 
		    x2 >= ax->cursor[m].scaled_x1) ||
		   (x1 >= ax->cursor[m].scaled_x1 && 
		    x2 <= ax->cursor[m].scaled_x1))
		{
		    x = unscale_x(&ax->d, ax->cursor[m].scaled_x1);
		    d = abs(cursor_x - x);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 5;
		    }
		}
		if((x1 <= ax->cursor[m].scaled_x2 && 
		    x2 >= ax->cursor[m].scaled_x2) ||
		   (x1 >= ax->cursor[m].scaled_x2 && 
		    x2 <= ax->cursor[m].scaled_x2))
		{
		    x = unscale_x(&ax->d, ax->cursor[m].scaled_x2);
		    d = abs(cursor_x - x);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 6;
		    }
		}
	    }
	    else if(ax->cursor[m].type == AXES_HOR_DOUBLE_LINE)
	    {
		if((y1 <= ax->cursor[m].scaled_y1 && 
		    y2 >= ax->cursor[m].scaled_y1) ||
		   (y1 >= ax->cursor[m].scaled_y1 && 
		    y2 <= ax->cursor[m].scaled_y1))
		{
		    y = unscale_y(&ax->d, ax->cursor[m].scaled_y1);
		    d = abs(cursor_y - y);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 5;
		    }
		}
		if((y1 <= ax->cursor[m].scaled_y2 && 
		    y2 >= ax->cursor[m].scaled_y2) ||
		   (y1 >= ax->cursor[m].scaled_y2 && 
		    y2 <= ax->cursor[m].scaled_y2))
		{
		    y = unscale_y(&ax->d, ax->cursor[m].scaled_y2);
		    d = abs(cursor_y - y);
		    if(d < dmin) {
			dmin = d;
			which = m;
			*grab = 6;
		    }
		}
	    }
	}
	*d_min = dmin;
	return(which);
}

void AxesClass::mark(int x, int y)
{
    AxesPart *ax = &aw->axes;
    int last_x, last_y;
    DSegment segs[3];

    if(ax->unmark)
    {
	last_x = ax->last_x_mark;
	last_y = ax->last_y_mark;
	segs[0].x1 = last_x-8; segs[0].y1 = last_y;
	segs[0].x2 = last_x+8; segs[0].y2 = last_y;
	
	segs[1].x1 = last_x;   segs[1].y1 = last_y-8;
	segs[1].x2 = last_x;   segs[1].y2 = last_y-1;

	segs[2].x1 = last_x;   segs[2].y1 = last_y+1;
	segs[2].x2 = last_x;   segs[2].y2 = last_y+8;
	_AxesDrawSegments(aw, ax->invert_clipGC, segs, 2);
    }

    segs[0].x1 = x-8; segs[0].y1 = y;
    segs[0].x2 = x+8; segs[0].y2 = y;

    segs[1].x1 = x;   segs[1].y1 = y-8;
    segs[1].x2 = x;   segs[1].y2 = y-1;

    segs[2].x1 = x;   segs[2].y1 = y+1;
    segs[2].x2 = x;   segs[2].y2 = y+8;
    _AxesDrawSegments(aw, ax->invert_clipGC, segs,2);

    ax->last_x_mark = x;
    ax->last_y_mark = y;
    ax->unmark = true;
}

void AxesClass::drawLines(int  npts, double *x, double *y)
{
    AxesPart *ax = &aw->axes;
    int i;
    SegArray s;
    DrawStruct *d;
    SegArray seg_array_init = SEG_ARRAY_INIT;

    if(npts <= 0) return;

    d = &ax->d;
    SetClipArea(d, ax->x1[ax->zoom], ax->y1[ax->zoom],
	    ax->x2[ax->zoom], ax->y2[ax->zoom]);

    d->s = &s;
    s = seg_array_init;

    imove(d, x[0], y[0]);
    for(i = 1; i < npts; i++) idraw(d, x[i], y[i]);
    iflush(d);
    if(s.nsegs > 0) {
	if(XtIsRealized((Widget)aw)) {
	    XDrawSegments(XtDisplay(aw), XtWindow(aw), ax->axesGC,
			(XSegment *)s.segs, s.nsegs);
	}
    }
    Free(s.segs);
}

static void
LastMark(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int last_x, last_y;
	DSegment segs[3];

	if(ax->unmark)
	{
		last_x = ax->last_x_mark;
		last_y = ax->last_y_mark;
		segs[0].x1 = last_x-8; segs[0].y1 = last_y;
		segs[0].x2 = last_x+8; segs[0].y2 = last_y;
	
		segs[1].x1 = last_x;   segs[1].y1 = last_y-8;
		segs[1].x2 = last_x;   segs[1].y2 = last_y-1;
	
		segs[2].x1 = last_x;   segs[2].y1 = last_y+1;
		segs[2].x2 = last_x;   segs[2].y2 = last_y+8;
		_AxesDrawSegments(w, ax->invert_clipGC, segs, 2);
	}
}

Widget
AxesCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
	return (XtCreateWidget(name, axesWidgetClass, parent, 
		arglist, argcount));
}

static void
DrawTitles(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int x_mid, y_mid, ascent, descent, direction, x_pos, height;
	double d, x1, x2, y1, y2, xmin, ymin, ymax;
	XCharStruct overall;
	AxesParm *a;

	if( !XtIsRealized((Widget)w) ) return;

	a = &ax->a;
	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	xmin = ax->d.sx1;
	ymin = ax->d.sy1;
	ymax = ax->d.sy2;
	if(fabs(x1-xmin) > fabs(x2-xmin))
	{
	    d = x1;
	    x1 = x2;
	    x2 = d;
	}
	if(fabs(y1-ymin) > fabs(y2-ymin))
	{
	    d = y1;
	    y1 = y2;
	    y2 = d;
	}
	
	x_pos = (ax->x_axis != LEFT) ? ax->x_axis:ax->y_axis;
	/*
	 * Draw title.
	 */
	if(ax->save_space && ax->title[0] != '\0')
	{
	    XCharStruct overall;
	    XTextExtents((XFontStruct*)ax->a.font, ax->title,
			(int)strlen(ax->title), &direction, &ascent, &descent,
			&overall);
	    ax->title_x = unscale_x(&ax->d,.5*(x1+x2)) -(int)(.5*overall.width);
	    ax->title_y = unscale_y(&ax->d, y2) - overall.descent - 2;
	    ax->title_width = overall.width;
	}

	if(a->top_title != NULL && (int)strlen(a->top_title) > 0)
	{
	    XTextExtents((XFontStruct*)a->font, a->top_title,
			(int)strlen(a->top_title), &direction, &ascent,
			&descent, &overall);
	    x_mid = unscale_x(&ax->d, .5*(x1+x2));
	    a->title_x = (int)(x_mid - .5*overall.width);
	    if(x_pos == BOTTOM)
	    {
		a->title_y = (int)(.5*(ax->d.iy2 + unscale_y(&ax->d, y2)) - 2);
	    }
	    else
	    {
		a->title_y = (int)(.5*(unscale_y(&ax->d, y1) + ax->d.iy1)
				+ overall.ascent + 2);
	    }
	}
	if(a->x_axis_label != NULL && (int)strlen(a->x_axis_label) > 0)
	{
	    if(ax->time_scale == TIME_SCALE_HMS)
	    {
		if(ax->x2[ax->zoom] - ax->x1[ax->zoom] <= 40*3600) {
		    if(strcmp(a->x_axis_label, "Time (hr:min:sec)")) {
			new_x_label(w, w, "Time (hr:min:sec)");
		    }
		}
		else {
		    if(strcmp(a->x_axis_label, "Time (yr/mo/day)")) {
			new_x_label(w, w, "Time (yr/mo/day)");
		    }
		}
	    }
	    XTextExtents((XFontStruct*)a->font, a->x_axis_label,
			(int)strlen(a->x_axis_label), &direction, &ascent,
			&descent, &overall);
	    x_mid = unscale_x(&ax->d, .5*(x1+x2));
	    a->xlab_x = (int)(x_mid - .5*overall.width);
	    height = descent + ascent;
	    if(x_pos == BOTTOM)
	    {
		a->xlab_y =  (int)(a->y_xlab + .8*height + ascent);
		if(a->log_x) a->xlab_y += (int)(.5*ascent);
	    }
	    else
	    {
		a->xlab_y = unscale_y(&ax->d, ymax + .5*a->xtic)
					+ overall.ascent;
	    }
	    if(a->xlab_y + overall.descent > ax->d.iy1) {
		a->xlab_y = ax->d.iy1 - overall.descent;
	    }
	}
	if(a->y_axis_label != NULL && (int)strlen(a->y_axis_label) > 0)
	{
	    int space;
	    XTextExtents((XFontStruct*)a->font, a->y_axis_label,
			(int)strlen(a->y_axis_label), &direction, &ascent,
			&descent, &overall);

	    space = unscale_x(&ax->d, xmin + a->ytic) - unscale_x(&ax->d, xmin);

	    y_mid = unscale_y(&ax->d, .5*(y1+y2));
	    a->ylab_y = (int)(y_mid - .5*overall.width);
	    a->ylab_h = overall.descent + overall.ascent;
	    a->ylab_w = overall.width;
	    a->ylab_x = ax->a.r_ylab - a->maxcy - space - a->ylab_h;
	}
}

/** 
 *  
 */
void
_Axes_AdjustScrollBars(AxesWidget w)
{
	AxesPart *ax = &w->axes;
	int left, right, top, bot, x, y, width, height, value, slider_size, n;
	int increment;
	double a, b, a0, b0;
	Arg args[10];

/*
	left  = unscale_x(&ax->d, ax->x1[ax->zoom]);
	right = unscale_x(&ax->d, ax->x2[ax->zoom]);
	bot   = unscale_y(&ax->d, ax->y1[ax->zoom]);
	top   = unscale_y(&ax->d, ax->y2[ax->zoom]);
*/
	left = ax->a.left;
	right = ax->a.right;
	bot = ax->a.bottom;
	top = ax->a.top;

	if(left > right)
	{
		x = left;
		left = right;
		right = x;
	}
	if(top > bot)
	{
		y = top;
		top = bot;
		bot = y;
	}
	if(ax->vertical_scroll && ax->vert_scroll != (Widget)NULL
		&& (!ax->scrollbar_as_needed || ax->zoom > 0))
	{
		if(!XtIsManaged(ax->vert_scroll))
		{
			XtManageChild(ax->vert_scroll);
		}
		y = top;
		width = ax->vert_scroll->core.width;
		x = (ax->vertical_scroll_position == LEFT) ? 0 :
			w->core.width - width;
		height = bot - top + 1;
		XtResizeWidget(ax->vert_scroll, width, height, 0);
		XtMoveWidget(ax->vert_scroll, x, y);

		a = ax->y1[ax->zoom];
		b = ax->y2[ax->zoom];
		a0 = ax->y1[0];
		b0 = ax->y2[0];
		slider_size = (int)(fabs((b - a)/(b0 - a0)) * (bot - top));
		if(slider_size < 1) slider_size = 1;
		if(slider_size > bot - top) slider_size = bot - top;
	
		value = (int)((a - a0)/(b0 - a0) * (bot - top) + .5);
		if(value < 0) value = 0;
		if(value > bot - top - slider_size)
			value = bot - top - slider_size;
		/* Set XmNvalue first. Then set the other parameters. This
		 * avoids a bug in XmScrollBar that causes a warning about
		 * the value.
		 */
		n = 0;
		XtSetArg(args[n], XmNvalue, 0); n++;
		XtSetValues(ax->vert_scroll, args, n);
		n = 0;
		XtSetArg(args[n], XmNminimum, 0); n++;
		XtSetArg(args[n], XmNmaximum, bot - top); n++;
		XtSetArg(args[n], XmNsliderSize, slider_size); n++;
		increment = (slider_size/10 > 1) ? slider_size/10 : 1;
		XtSetArg(args[n], XmNincrement, increment); n++;
		XtSetArg(args[n], XmNpageIncrement, slider_size); n++;
		if(!ax->reverse_y)
		{
		 XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
		}
		else
		{
		 XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_BOTTOM);n++;
		}
		XtSetArg(args[n], XmNvalue, value); n++;
		XtSetValues(ax->vert_scroll, args, n);
		ax->vert_scroll_value = value;
	}
	else if(ax->vert_scroll != (Widget)NULL && XtIsManaged(ax->vert_scroll))
	{
	    XtUnmanageChild(ax->vert_scroll);
	}

	if(ax->horizontal_scroll && ax->hor_scroll != (Widget)NULL
		&& (!ax->scrollbar_as_needed || ax->zoom > 0))
	{
		if(!XtIsManaged(ax->hor_scroll))
		{
			XtManageChild(ax->hor_scroll);
		}
		positionHorScroll(ax, ax->hor_scroll, left, right);
	}
	else if(ax->hor_scroll != (Widget)NULL && XtIsManaged(ax->hor_scroll))
	{
	    XtUnmanageChild(ax->hor_scroll);
	}
	if(ax->outside_hor_scroll) {
	    positionHorScroll(ax, ax->outside_hor_scroll, left, right);
	}
}

static void
positionHorScroll(AxesPart *ax, Widget hor_scroll, int left, int right)
{
	int x, y, width, height, value, slider_size, n;
	int increment;
	double a, b, a0, b0;
	Arg args[10];

	x = left;
	y = XtParent(hor_scroll)->core.height - hor_scroll->core.height;
	width = right - left + 1;
	height = hor_scroll->core.height;
	XtResizeWidget(hor_scroll, width, height, 0);
	XtMoveWidget(hor_scroll, x, y);

	a = ax->x1[ax->zoom];
	b = ax->x2[ax->zoom];
	a0 = ax->x1[0];
	b0 = ax->x2[0];
	slider_size = (int)(fabs((b - a)/(b0 - a0)) * (right - left));
	if(slider_size < 1) slider_size = 1;
	if(slider_size > right - left) slider_size = right - left;

	value = (int)((a - a0)/(b0 - a0) * (right - left) + .5);
	if(value < 0) value = 0;
	if(value > right - left - slider_size) value = right-left-slider_size;

	n = 0;
	XtSetArg(args[n], XmNminimum, 0); n++;
	XtSetArg(args[n], XmNmaximum, right - left); n++;
	XtSetArg(args[n], XmNsliderSize, slider_size); n++;
	increment = (slider_size/10 > 1) ? slider_size/10 : 1;
	XtSetArg(args[n], XmNincrement, increment); n++;
	XtSetArg(args[n], XmNpageIncrement, slider_size); n++;
	if(!ax->reverse_x)
	{
	    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
	}
	else
	{
	    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_LEFT);n++;
	}
	XtSetArg(args[n], XmNvalue, value); n++;
	XtSetValues(hor_scroll, args, n);
	ax->hor_scroll_value = value;
}

static void
HorizontalScroll(Widget scroll, XtPointer client_data, XtPointer data)
{
	AxesWidget w = (AxesWidget)client_data;
	XmScrollBarCallbackStruct *call_data =(XmScrollBarCallbackStruct *)data;
	int n, min, max, value, left, right, slider_size;
	float fn;
	Arg args[10];
	double old_x1, old_x2, new_x1, new_x2, dif, shift;
	AxesLimitsCallbackStruct s;
	AxesPart *ax;

	ax = &w->axes;
	if(ax->hor_scroll == (Widget)NULL)
	{
	    return;
	}
	if(call_data->value == ax->hor_scroll_value)
	{
	    /* get this at the end of a drag */
	    s.limits = True;
	    s.x_min = ax->x1[ax->zoom];
	    s.x_max = ax->x2[ax->zoom];
	    s.y_min = ax->y1[ax->zoom];
	    s.y_max = ax->y2[ax->zoom];
	    s.x_margins = False;
	    s.y_margins = False;
	    s.left   = ax->a.left;
	    s.right  = ax->a.right;
	    s.top    = ax->a.top;
	    s.bottom = ax->a.bottom;
	    XtCallCallbacks((Widget)w, XtNlimitsCallback, &s);
	    return;
	}

	n = 0;
	XtSetArg(args[n], XmNminimum, &min); n++;
	XtSetArg(args[n], XmNmaximum, &max); n++;
	XtSetArg(args[n], XmNsliderSize, &slider_size); n++;
	XtGetValues(scroll, args, n);

	old_x1 = ax->x1[ax->zoom];
	old_x2 = ax->x2[ax->zoom];
	dif = old_x2 - old_x1;

	new_x1 = ax->x1[0] + ((double)call_data->value/(double)(max-min)) * 
		(ax->x2[0] - ax->x1[0]);
	new_x2 = new_x1 + dif;
	shift = new_x1 - old_x1;

	fn = (call_data->reason == XmCR_INCREMENT ||
			call_data->reason == XmCR_DECREMENT) ? .1 : 1.;

	if(call_data->reason != XmCR_DRAG && fabs(shift) > 1.01*fn*dif)
	{

	    if( call_data->reason == XmCR_INCREMENT ||
		call_data->reason == XmCR_PAGE_INCREMENT)
	    {
		new_x2 = old_x2 + fn*dif;
		if(new_x2 > ax->x2[0]) new_x2 = ax->x2[0];
		new_x1 = new_x2 - dif;
		if(new_x1 < ax->x1[0]) new_x1 = ax->x1[0];
	    }
	    else
	    {
		new_x1 = old_x1 - fn*dif;
		if(new_x1 < ax->x1[0]) new_x1 = ax->x1[0];
		new_x2 = new_x1 + dif;
		if(new_x2 > ax->x2[0]) new_x2 = ax->x2[0];
	    }
	    left  = unscale_x(&ax->d, new_x1);
	    right = unscale_x(&ax->d, new_x2);
	    value = (int)((new_x1 - ax->x1[0])/
			(ax->x2[0]-ax->x1[0])*abs(right-left) + .5);
	    if(value < 0) value = 0;
	    if(value > abs(right - left) - slider_size)
		value = abs(right-left)-slider_size;
		
	    n = 0;
	    XtSetArg(args[n], XmNvalue, value); n++;
	    XtSetValues(ax->hor_scroll, args, n);
	    ax->hor_scroll_value = value;
	    shift = new_x1 - old_x1;
	}
	else
	{
	    ax->hor_scroll_value = call_data->value;
	}
	ax->x1[ax->zoom] = new_x1;
	ax->x2[ax->zoom] = new_x2;

	if(ax->manual_scroll)
	{
	    if(ax->redraw_data_func != NULL)
	    {
		(*(ax->redraw_data_func))(w, DATA_SCROLL_HOR, shift);
		_AxesRedisplayXor(w);
	    }
	    return;
	}
	
	if(fabs(shift) > 10*dif || (ax->a.log_x && fabs(new_x2-new_x1) < 1.))
	{
	    _AxesRedraw(w);
	    _AxesRedisplayAxes(w);
	    if(ax->redraw_data_func != NULL)
	    {
		(*(ax->redraw_data_func))(w, DATA_JUMP_HOR, shift);
	    }
	    if(ax->use_pixmap && XtIsRealized((Widget)w)) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC, 0,
				0, w->core.width, w->core.height, 0, 0);
	    }
	    ax->use_screen = False;
	    _AxesRedisplayXor(w);
	    ax->use_screen = True;
	    DoMagFrom(w);
	    return;
	}
	if(call_data->reason != XmCR_DRAG)
	{
	    s.x_min = ax->x1[ax->zoom];
	    s.x_max = ax->x2[ax->zoom];
	    s.y_min = ax->y1[ax->zoom];
	    s.y_max = ax->y2[ax->zoom];
	    s.x_margins = False;
	    s.y_margins = False;
	    s.left   = ax->a.left;
	    s.right  = ax->a.right;
	    s.top    = ax->a.top;
	    s.bottom = ax->a.bottom;
	    XtCallCallbacks((Widget)w, XtNlimitsCallback, &s);
	}

	_AxesHorizontalShift(w, shift);

	if(call_data->reason == XmCR_DRAG) {
	    XtCallCallbacks((Widget)w, XtNhorizontalScrollCallback, &shift);
	}
}

static void
_AxesHorizontalShift(AxesWidget w, double shift)
{
	AxesPart *ax = &w->axes;
	float fn;
	double xmin, xmax, ymin, ymax;
	double x1, y1, x2, y2, xlo, xhi, yl, yn, spacing, factor;
	int i, n, height, width, xpos;

	if( ax->save_space && ax->title[0] != '\0') {
	    _AxesDrawString(w, ax->axesGC, ax->title_x, ax->title_y, ax->title);
	}
	for(i = 0; i < ax->num_cursors; i++)
	{
//	    if(ax->cursor[i].label_x >= 0 &&
	    if(ax->cursor[i].label_x >= 0 && !ax->cursor[i].label_off &&
		(n = (int)strlen(ax->cursor[i].label)) > 0)
	    {
		_AxesDrawString(w, ax->invertGC, ax->cursor[i].label_x,
				ax->cursor[i].label_y, ax->cursor[i].label);
	    }
	    if(ax->cursor[i].nsegs > 0)
	    {
		_AxesDrawSegments(w, ax->invert_clipGC,
			(DSegment *)ax->cursor[i].segs, ax->cursor[i].nsegs);
	    }
	    if(!ax->cursor[i].x_off && ax->cursor[i].xlab[0] !='\0')
	    {
		_AxesDrawString(w, ax->invertGC, ax->cursor[i].x_xlab,
				ax->cursor[i].y_xlab, ax->cursor[i].xlab);
	    }
	    if(!ax->cursor[i].y_off && ax->cursor[i].ylab[0] !='\0')
	    {
		_AxesDrawString(w, ax->invertGC, ax->cursor[i].x_ylab,
				ax->cursor[i].y_ylab, ax->cursor[i].ylab);
	    }
	}
	GetScale(&ax->d, &xmin, &ymin, &xmax, &ymax);
	xmin += shift;
	xmax += shift;
	SetScale(&ax->d, xmin, ymin, xmax, ymax);

	if( !ax->save_space) {
	    for(i = 0; i < ax->num_cursors; i++) 
		if(ax->cursor[i].label[0] != '\0')
	    {
		int x = ax->d.ix1 - (int)(.5*ax->cursor[i].label_width);
		int y = ax->cursor[i].label_y - ax->cursor[i].label_ascent;
		width = ax->d.ix2 + (int)(.5*ax->cursor[i].label_width) - x + 1;
		height = ax->cursor[i].label_height;
		_AxesClearArea(w, x, y, width, height);
		break;
	    }
	}

    if(ax->display_axes &&  (ax->display_axes_labels == AXES_X ||
		ax->display_axes_labels == AXES_XY))
    {
	if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
	{
	    if(ax->clear_on_scroll)
	    {
		if(ax->a.log_x && !ax->a.x_small_log && !ax->a.x_medium_log) {
		    height = (int)(ax->a.y_xlab + 1.6*ax->a.max_xlab_height
				- ax->clipy1);
		}
		else {
		    height = ax->a.y_xlab + ax->a.max_xlab_height - ax->clipy1;
		}
		width = ax->clipx2 - ax->clipx1 + 1;
		_AxesClearArea(w, ax->clipx1, ax->clipy1, width, height);
/*		_AxesClearArea(w, 0, ax->clipy1, w->core.width, height); */
	    }
	    else
	    {
		height = ax->a.y_xlab + ax->a.max_xlab_height - ax->clipy2 - 1;
/*		_AxesClearArea(w, ax->clipx1, ax->clipy2 + 1, width, height);*/
		_AxesClearArea(w, 0, ax->clipy2 + 1, w->core.width, height);
	    }
	    height = ax->a.max_xlab_height + 1;
	    if(ax->a.log_x && !ax->a.x_small_log && !ax->a.x_medium_log) {
		height += (int)(.6*ax->a.max_xlab_height);
	    }
	    width = ax->d.ix2 - ax->d.ix1 + 1;
/*	    _AxesClearArea(w, ax->d.ix1, ax->a.y_xlab,width,height); */
	    _AxesClearArea(w, 0, ax->a.y_xlab, w->core.width, height);
	    if(!ax->tickmarks_inside && !ax->save_space)
	    {
		width = ax->clipx2 - ax->clipx1 + 1;
		// get the tic marks on the edges 
		height = ax->a.y_xlab - ax->clipy2;
		width = ax->clipx2 - ax->clipx1 + 3;
		_AxesClearArea(w, ax->clipx1-1, ax->clipy2+1, width, height);
		height = (int)(fabs(ax->a.xtic*ax->d.unscaley) + 1);
		width = ax->clipx2 - ax->clipx1 + 3;
		_AxesClearArea(w, ax->clipx1-1, ax->clipy1-1-height, width,
				height);
	    }
	}
	else
	{
	    height = ax->clipy2 - ax->a.y_xlab + 1;
	    width = ax->clipx2 - ax->clipx1 + 1;
	    _AxesClearArea(w, ax->clipx1,ax->a.y_xlab,width,height);
	    height = ax->a.max_xlab_height;
	    width = ax->d.ix2 - ax->d.ix1 + 1;
	    _AxesClearArea(w, ax->d.ix1, ax->a.y_xlab,width,height);
	    if(!ax->tickmarks_inside)
	    {
		/* get the tic marks on the edges */
		height = ax->clipy1 - ax->a.y_xlab;
		width = ax->clipx2 - ax->clipx1 + 3;
		_AxesClearArea(w, ax->clipx1-1, ax->a.y_xlab, width, height);
		height = (int)(fabs(ax->a.xtic*ax->d.unscaley));
		width = ax->clipx2 - ax->clipx1 + 3;
		_AxesClearArea(w, ax->clipx1-1, ax->clipy2+1, width, height);
	    }
	}
    }
	if(!ax->reverse_x)
	{
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	}
	else
	{
	    x1 = ax->x2[ax->zoom];
	    x2 = ax->x1[ax->zoom];
	}
	if(!ax->reverse_y)
	{
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	}
	else
	{
	    y1 = ax->y2[ax->zoom];
	    y2 = ax->y1[ax->zoom];
	}

    if(ax->display_axes &&  (ax->display_axes_labels == AXES_X ||
		ax->display_axes_labels == AXES_XY))
    {
	factor = (ax->x_axis != LEFT) ? time_factor(w, True) : 1.;

	xlo = ax->x1[ax->zoom]/factor;
	xhi = ax->x2[ax->zoom]/factor;

	if(ax->a.nxlab > 1)
	{
	    spacing = (ax->a.x_lab[1] - ax->a.x_lab[0])/factor;
	}
	else
	{
	    spacing = ax->x_spacing;
	}
	if(spacing > 0.)
	{
	    ax->a.x_lab[0] /= factor;
	    for(i = 0, n = 1; i < ax->a.nxdeci; i++, n *= 10);
	    fn = (float)n;
	    for(i = 0; ax->a.x_lab[0]+(i-1)*spacing > xlo; i--);
	    for(     ; ax->a.x_lab[0]+    i*spacing < xlo; i++);
	    ax->a.x_lab[0] += i*spacing;
	    ax->a.x_lab[0] = floor(ax->a.x_lab[0]*n+.5)/fn;
	    for(i = 1; ax->a.x_lab[0] + i*spacing <= xhi && i+1 < MAXLAB; i++)
	    {
		ax->a.x_lab[i] = ax->a.x_lab[0] + i*spacing;
		ax->a.x_lab[i] = floor(ax->a.x_lab[i]*n+.5)/fn;
	    }
	    ax->a.nxlab = i;
	    ax->x_spacing = spacing;
	    for(i = 0; i < ax->a.nxlab; i++)
	    {
		ax->a.x_lab[i] *= factor;
	    }
	    if(ax->a.log_x && ax->a.nxlab == 1) {
		ax->a.x_lab[1] = ax->a.x_lab[0] + 1.;
		ax->a.nxlab++;
	    }
	}
	
	if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
	{
	    yl = y1;
	    yn = y2;
	    xpos = BOTTOM;
	}
	else
	{
	    yl = y2;
	    yn = y1;
	    xpos = TOP;
	}

	if(ax->time_scale == TIME_SCALE_HMS && ax->x_axis != LEFT)
	{
	    gdrawTimeAxis(&ax->a, &ax->d, 0, yl, x1, x2, ax->a.nxdeci,
			ax->a.xtic, xpos, ax->tickmarks_inside, True);
	    gdrawTimeAxis(&ax->a, &ax->d, 1, yn, x1, x2, ax->a.nxdeci,
			-ax->a.xtic, xpos, ax->tickmarks_inside, False);
	}
	else
	{
	    gdrawXAxis(&ax->a, &ax->d, 0, yl, x1, x2, ax->a.xtic, xpos,
			ax->tickmarks_inside, ax->a.ew, 1,factor);
	    gdrawXAxis(&ax->a, &ax->d, 1, yn, x1, x2, -ax->a.xtic, xpos,
			ax->tickmarks_inside, 0, 0, factor);
	}

	gdrawYAxis(&ax->a, &ax->d, 2, x1, y1, y2, ax->a.ytic,
		ax->tickmarks_inside, 0, 0, time_factor(w, True));
	gdrawYAxis(&ax->a, &ax->d, 3, x2, y1, y2, -ax->a.ytic,
		ax->tickmarks_inside, 0, 0, time_factor(w, True));
	if(ax->display_axes)
	{
	    if( ax->display_axes_labels == AXES_X ||
		ax->display_axes_labels == AXES_XY)
	    {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[0].segs,
					ax->a.axis_segs[0].n_segs);
		if( !ax->save_space && ax->extra_x_tics ) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[1].segs,
					ax->a.axis_segs[1].n_segs);
		}
		else if(ax->a.axis_segs[1].n_segs > 0) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[1].segs,1);
		}
		drawXLabels(w, ax);
	    }
	    else
	    {
		for(i = 0; i < 2; i++)
			if(ax->a.axis_segs[i].n_segs > 0)
		{
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[i].segs,1);
		}
	    }
	    if( ax->display_axes_labels == AXES_Y ||
		ax->display_axes_labels == AXES_XY)
	    {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[2].segs,
					ax->a.axis_segs[2].n_segs);
		if( !ax->save_space && ax->extra_y_tics ) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[3].segs,
					ax->a.axis_segs[3].n_segs);
		}
		else if(ax->a.axis_segs[3].n_segs > 0) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[3].segs,1);
		}
	    }
	    else
	    {
		for(i = 2; i < 4; i++)
			if(ax->a.axis_segs[i].n_segs > 0)
		{
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[i].segs,1);
		}
	    }
	}
    }
	if(ax->display_grid) {
	    DrawGrid(w);
	}
	if(ax->x_axis != LEFT)
	{
	    ax->x_min = x1;
	    ax->x_max = x2;
	    ax->y_min = y1;
	    ax->y_max = y2;
	}
	else
	{
	    ax->x_min = y1;
	    ax->x_max = y2;
	    ax->y_min = x1;
	    ax->y_max = x2;
	}
	if(ax->redraw_data_func != NULL)
	{
	    (*(ax->redraw_data_func))(w, DATA_SCROLL_HOR, shift);
	}
	if(ax->use_pixmap)
	{
	    ax->use_screen = False;
	    _AxesRedisplayXor(w);
	    ax->use_screen = True;
	    if(XtIsRealized((Widget)w)) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w),
			ax->axesGC, 0, 0, w->core.width,w->core.height,0,0);
	    }
	}
	else
	{
	    _AxesRedisplayXor(w);
	}
	DoMagFrom(w);
}

void AxesClass::horizontalShift(double shift, bool scroll)
{
    AxesPart *ax = &aw->axes;
    int i;

    for(i = 0; i < ax->num_cursors; i++)
    {
	if(ax->cursor[i].type == AXES_VER_DOUBLE_LINE
		&& ax->cursor[i].anchor_on)
	{
	    DrawCursor(aw, i);
	    ax->cursor[i].scaled_x += shift;
	    ax->cursor[i].scaled_x1 += shift;
	    ax->cursor[i].scaled_x2 += shift;
	    DrawCursor(aw, i);
	}
    }
    ax->x1[0] += shift;
    ax->x2[0] += shift;
    ax->x1[1] += shift;
    ax->x2[1] += shift;
    if(scroll) {
	int i;
	for(i = 2; i <= ax->zoom; i++) {
	    ax->x1[i] += shift;
	    ax->x2[i] += shift;
	}
    }

    if(ax->zoom <= 1 || scroll)
    {
	double dif = ax->x2[ax->zoom] - ax->x1[ax->zoom];

	if(fabs(shift) > 10*dif)
	{
	    _AxesRedraw(aw);
	    _AxesRedisplayAxes(aw);
	    if(ax->redraw_data_func != NULL)
	    {
		(*(ax->redraw_data_func))(aw, DATA_JUMP_HOR, shift);
	    }
	    if(ax->use_pixmap && XtIsRealized((Widget)aw)) {
		XCopyArea(XtDisplay(aw), ax->pixmap, XtWindow(aw), ax->axesGC,
				0, 0, aw->core.width, aw->core.height, 0, 0);
	    }
	    ax->use_screen = false;
	    _AxesRedisplayXor(aw);
	    ax->use_screen = true;
	    DoMagFrom(aw);
	}
	else {
		_AxesHorizontalShift(aw, shift);
	}
    }
    _Axes_AdjustScrollBars(aw);
}

static void
VerticalScroll(Widget scroll, XtPointer client_data, XtPointer data)
{
	AxesWidget w = (AxesWidget)client_data;
	XmScrollBarCallbackStruct *call_data =(XmScrollBarCallbackStruct *)data;
	int i, n, min, max, height, width, xpos, value, top, bot,
		slider_size, r_ylab;
	float fn;
	Arg args[10];
	double x1, y1, x2, y2, ylo, yhi, dif, shift, spacing, factor;
	double xmin, xmax, ymin, ymax;
	double old_y1, old_y2, new_y1, new_y2;
	AxesLimitsCallbackStruct s;
	AxesPart *ax;

	ax = &w->axes;
	if(ax->vert_scroll == (Widget)NULL)
	{
	    return;
	}
/*
	if(call_data->value == ax->vert_scroll_value)
	{
	    s.limits = True;
	    s.x_min = ax->x1[ax->zoom];
	    s.x_max = ax->x2[ax->zoom];
	    s.y_min = ax->y1[ax->zoom];
	    s.y_max = ax->y2[ax->zoom];
	    s.x_margins = False;
	    s.y_margins = False;
	    s.left   = ax->a.left;
	    s.right  = ax->a.right;
	    s.top    = ax->a.top;
	    s.bottom = ax->a.bottom;
	    XtCallCallbacks((Widget)w, XtNlimitsCallback, &s);
	    return;
	}
*/

	n = 0;
	XtSetArg(args[n], XmNminimum, &min); n++;
	XtSetArg(args[n], XmNmaximum, &max); n++;
	XtSetArg(args[n], XmNsliderSize, &slider_size); n++;
	XtGetValues(scroll, args, n);

	old_y1 = ax->y1[ax->zoom];
	old_y2 = ax->y2[ax->zoom];
	dif = old_y2 - old_y1;

	new_y1 = ax->y1[0] + ((double)call_data->value/(double)(max-min)) * 
		(ax->y2[0] - ax->y1[0]);
	new_y2 = new_y1 + dif;
	shift = new_y1 - old_y1;

	fn = (call_data->reason == XmCR_INCREMENT ||
			call_data->reason == XmCR_DECREMENT) ? .1 : 1.;

//	if(call_data->reason != XmCR_DRAG && fabs(shift) > 1.01*fn*dif)
	if(call_data->reason != XmCR_DRAG &&
		call_data->reason != XmCR_VALUE_CHANGED)
	{
	    if( call_data->reason == XmCR_INCREMENT ||
		call_data->reason == XmCR_PAGE_INCREMENT)
	    {
		new_y2 = old_y2 + fn*dif;
		if(new_y2 > ax->y2[0]) new_y2 = ax->y2[0];
		new_y1 = new_y2 - dif;
		if(new_y1 < ax->y1[0]) new_y1 = ax->y1[0];
	    }
	    else
	    {
		new_y1 = old_y1 - fn*dif;
		if(new_y1 < ax->y1[0]) new_y1 = ax->y1[0];
		new_y2 = new_y1 + dif;
		if(new_y2 > ax->y2[0]) new_y2 = ax->y2[0];
	    }
	    bot = unscale_y(&ax->d, new_y1);
	    top = unscale_y(&ax->d, new_y2);
	    value = (int)((new_y1 - ax->y1[0])/(ax->y2[0]-ax->y1[0])*
				abs(bot - top) + .5);
	    if(value < 0) value = 0;
	    if(value > abs(bot - top) - slider_size)
			value = abs(bot - top) - slider_size;
	    n = 0;
	    XtSetArg(args[n], XmNvalue, value); n++;
	    XtSetValues(ax->vert_scroll, args, n);
	    ax->vert_scroll_value = value;
	    shift = new_y1 - old_y1;
	}
	else
	{
	    ax->vert_scroll_value = call_data->value;
	}
	ax->y1[ax->zoom] = new_y1;
	ax->y2[ax->zoom] = new_y2;
	
	if(ax->manual_scroll)
	{
	    if(ax->redraw_data_func != NULL)
	    {
		(*(ax->redraw_data_func))(w, DATA_SCROLL_VERT, shift);
		_AxesRedisplayXor(w);
	    }
	    return;
	}
	
	if(fabs(shift) > 10*dif || (ax->a.log_y && fabs(new_y2-new_y1) < 1.))
	{
	    _AxesRedraw(w);
	    _AxesRedisplayAxes(w);
	    if(ax->redraw_data_func != NULL)
	    {
		(*(ax->redraw_data_func))(w, DATA_JUMP_VERT, shift);
	    }
	    if(ax->use_pixmap && XtIsRealized((Widget)w)) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC, 0,
				0, w->core.width, w->core.height,0,0);
	    }
	    ax->use_screen = False;
	    _AxesRedisplayXor(w);
	    ax->use_screen = True;
	    DoMagFrom(w);
	    return;
	}
	if(call_data->reason != XmCR_DRAG)
	{
	    s.x_min = ax->x1[ax->zoom];
	    s.x_max = ax->x2[ax->zoom];
	    s.y_min = ax->y1[ax->zoom];
	    s.y_max = ax->y2[ax->zoom];
	    s.x_margins = False;
	    s.y_margins = False;
	    s.left   = ax->a.left;
	    s.right  = ax->a.right;
	    s.top    = ax->a.top;
	    s.bottom = ax->a.bottom;
	    XtCallCallbacks((Widget)w, XtNlimitsCallback, &s);
	}
	for(i = 0; i < ax->num_cursors; i++)
	{
	    if(ax->cursor[i].label_x >= 0 &&
		(n = (int)strlen(ax->cursor[i].label)) > 0)
	    {
		_AxesDrawString(w, ax->invertGC, ax->cursor[i].label_x,
				ax->cursor[i].label_y, ax->cursor[i].label);
	    }
	    if(ax->cursor[i].nsegs > 0)
	    {
		_AxesDrawSegments(w, ax->invert_clipGC,
			(DSegment *)ax->cursor[i].segs, ax->cursor[i].nsegs);
	    }
	    if(!ax->cursor[i].x_off && ax->cursor[i].xlab[0] !='\0')
	    {
		_AxesDrawString(w, ax->invertGC, ax->cursor[i].x_xlab,
				ax->cursor[i].y_xlab, ax->cursor[i].xlab);
	    }
	    if(!ax->cursor[i].y_off && ax->cursor[i].ylab[0] !='\0')
	    {
		_AxesDrawString(w, ax->invertGC, ax->cursor[i].x_ylab,
				ax->cursor[i].y_ylab, ax->cursor[i].ylab);
	    }
	}

	r_ylab = (ax->a.max_ylab_width != 0) ? ax->a.r_ylab : 0;
		
	height = ax->clipy2 - ax->clipy1 + 1;
	if(ax->clear_on_scroll)
	{
	    width  = ax->clipx2 - r_ylab + ax->a.max_ylab_width + 1;
	    _AxesClearArea(w, r_ylab - ax->a.max_ylab_width, ax->clipy1, width,
				height);
	}
	else
	{
	    width  = ax->clipx1 -1 -r_ylab +ax->a.max_ylab_width +1;
	    _AxesClearArea(w, r_ylab - ax->a.max_ylab_width, ax->clipy1, width,
				height);
	}
	if(ax->a.max_ylab_width > 0)
	{
	    if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
	    {
		height = ax->a.y_xlab - ax->clipy1 + ax->a.max_ylab_height+1;
	    }
	    else
	    {
		height = ax->clipy2 - ax->a.y_xlab + 1;
	    }
	    width  = ax->a.max_ylab_width;

	    _AxesClearArea(w, r_ylab - ax->a.max_ylab_width,
		    ax->clipy1 - ax->a.max_ylab_height, width, height);

	    if(!ax->tickmarks_inside)
	    {
		/* get the tic marks on the edges */
		height = ax->clipy2 - ax->clipy1 + 3;
		width = ax->clipx1 - r_ylab;
		_AxesClearArea(w, r_ylab, ax->clipy1-1, width, height);
		height = ax->clipy2 - ax->clipy1 + 3;
		width = (int)(fabs(ax->a.ytic*ax->d.unscalex) + 2);
		_AxesClearArea(w, ax->clipx2+1, ax->clipy1-1, width, height);
	    }
	}

	if(!ax->reverse_x)
	{
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	}
	else
	{
	    x1 = ax->x2[ax->zoom];
	    x2 = ax->x1[ax->zoom];
	}
	if(!ax->reverse_y)
	{
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	}
	else
	{
	    y1 = ax->y2[ax->zoom];
	    y2 = ax->y1[ax->zoom];
	}

	GetScale(&ax->d, &xmin, &ymin, &xmax, &ymax);
	ymin += shift;
	ymax += shift;
	SetScale(&ax->d, xmin, ymin, xmax, ymax);

	if(ax->display_axes_labels == AXES_Y ||
	   ax->display_axes_labels == AXES_XY)
	{
	    factor = (ax->x_axis == LEFT) ? time_factor(w, True) : 1.;
	
	    ylo = ax->y1[ax->zoom]/factor;
	    yhi = ax->y2[ax->zoom]/factor;

	    if(ax->a.nylab > 1)
	    {
		spacing =(ax->a.y_lab[1]-ax->a.y_lab[0])/factor;
	    }
	    else
	    {
		spacing = ax->y_spacing;
	    }
	    ax->a.nylab = 0;
	    if(spacing > 0.)
	    {
		ax->a.y_lab[0] /= factor;
		for(i = 0, n = 1; i < ax->a.nydeci; i++, n *= 10);
		fn = (float)n;
		for(i = 0; ax->a.y_lab[0]+(i-1)*spacing > ylo; i--);
		for(     ; ax->a.y_lab[0]+    i*spacing < ylo; i++);
		ax->a.y_lab[0] += i*spacing;
		ax->a.y_lab[0] = floor(ax->a.y_lab[0]*n+.5)/fn;
		for(i = 1; ax->a.y_lab[0] + i*spacing <= yhi && i+1<MAXLAB; i++)
		{
		    ax->a.y_lab[i] = ax->a.y_lab[0] + i*spacing;
		    ax->a.y_lab[i] = floor(ax->a.y_lab[i]*n+.5)/fn;
		}
		ax->a.nylab = i;
		ax->y_spacing = spacing;
		for(i = 0; i < ax->a.nylab; i++)
		{
		    ax->a.y_lab[i] *= factor;
		}
		if(ax->a.log_y && ax->a.nylab == 1) {
		    ax->a.y_lab[1] = ax->a.y_lab[0] + 1.;
		    ax->a.nylab++;
		}
	    }

	    gdrawYAxis(&ax->a, &ax->d, 2, x1, y1, y2, ax->a.ytic,
			ax->tickmarks_inside, ax->a.ns, 1, factor);
	    gdrawYAxis(&ax->a, &ax->d, 3, x2, y1, y2,-ax->a.ytic,
			ax->tickmarks_inside, 0, 0, factor);
	}
	if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
	{
	    xpos = BOTTOM;
	}
	else
	{
	    xpos = TOP;
	}
	if(ax->time_scale == TIME_SCALE_HMS && ax->x_axis != LEFT)
	{
	    gdrawTimeAxis(&ax->a, &ax->d, 0, y1, x1, x2, ax->a.nxdeci,
			ax->a.xtic, xpos, ax->tickmarks_inside, True);
	    gdrawTimeAxis(&ax->a, &ax->d, 1, y2, x1, x2, ax->a.nxdeci,
			-ax->a.xtic, xpos, ax->tickmarks_inside, False);
	}
	else
	{
	    gdrawXAxis(&ax->a, &ax->d, 0, y1, x1, x2, ax->a.xtic, xpos,
			ax->tickmarks_inside, 0, 0, time_factor(w, True));
	    gdrawXAxis(&ax->a, &ax->d, 1, y2, x1, x2, -ax->a.xtic, xpos,
			ax->tickmarks_inside, 0, 0, time_factor(w, True));
	}
	if(ax->display_axes)
	{
	    if( ax->display_axes_labels == AXES_X ||
		ax->display_axes_labels == AXES_XY)
	    {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[0].segs,
					ax->a.axis_segs[0].n_segs);
                if( !ax->save_space && ax->extra_x_tics ) {

		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[1].segs,
					ax->a.axis_segs[1].n_segs);
		}
		else if(ax->a.axis_segs[1].n_segs > 0) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[1].segs,1);
                }
	    }
	    else
	    {
		for(i = 0; i < 2; i++)
		    if(ax->a.axis_segs[i].n_segs > 0)
		{
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[i].segs,1);
		}
	    }
	    if( ax->display_axes_labels == AXES_Y ||
		ax->display_axes_labels == AXES_XY)
	    {
		_AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[2].segs,
					ax->a.axis_segs[2].n_segs);
		if( !ax->save_space && ax->extra_y_tics ) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[3].segs,
					ax->a.axis_segs[3].n_segs);
		}
		else if(ax->a.axis_segs[3].n_segs > 0) {
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[3].segs,1);
		}
		drawYLabels(w, ax);
	    }
	    else
	    {
		for(i = 2; i < 4; i++)
			if(ax->a.axis_segs[i].n_segs > 0)
		{
		    _AxesDrawSegments(w, ax->axesGC, ax->a.axis_segs[i].segs,1);
		}
	    }
	}
	if(ax->display_grid) {
	    DrawGrid(w);
	}
	if(ax->x_axis != LEFT)
	{
	    ax->x_min = x1;
	    ax->x_max = x2;
	    ax->y_min = y1;
	    ax->y_max = y2;
	}
	else
	{
	    ax->x_min = y1;
	    ax->x_max = y2;
	    ax->y_min = x1;
	    ax->y_max = x2;
	}
	if(ax->redraw_data_func != NULL)
	{
	    (*(ax->redraw_data_func))(w, DATA_SCROLL_VERT, shift);
	}
	if(ax->use_pixmap)
	{
	    ax->use_screen = False;
	    _AxesRedisplayXor(w);
	    ax->use_screen = True;
	    if(XtIsRealized((Widget)w)) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w),
			ax->axesGC, 0, 0, w->core.width,w->core.height,0,0);
	    }
	}
	else
	{
	    _AxesRedisplayXor(w);
	}
	DoMagFrom(w);
}

/** 
 *  
 */
void
_AxesDoClipping(AxesWidget w, int nsegs, RSeg *segs, float x0, float y0,
		SegArray *m)
{
	AxesPart *ax = &w->axes;
	int i;
	DrawStruct d;

	if(nsegs <= 0)
	{
		return;
	}

        SetDrawArea(&d, ax->clipx1, ax->clipy1,
                	ax->clipx2, ax->clipy2);

        SetScale(&d, (double)ax->clipx1, (double)ax->clipy1,
                     (double)ax->clipx2, (double)ax->clipy2);

	d.s = m;
	d.s->nsegs = 0;

	if(ax->x_axis != LEFT)
	{
		for(i = 0; i < nsegs; i++)
		{
			imove(&d, (double)(x0 + segs[i].x1),
					(double)(y0 + segs[i].y1));
			idraw(&d, (double)(x0 + segs[i].x2),
					(double)(y0 + segs[i].y2));
		}
	}
	else
	{
		for(i = 0; i < nsegs; i++)
		{
			imove(&d, (double)(y0 - segs[i].y1),
					(double)(x0 + segs[i].x1));
			idraw(&d, (double)(y0 - segs[i].y2),
					(double)(x0 + segs[i].x2));
		}
	}
	iflush(&d);
}

AxesParm * AxesClass::hardCopy(FILE *fp, PrintParam *p, AxesParm *input_a)
{
	AxesPart *ax = &aw->axes;
	AxesParm axes_parm_init = AXES_PARM_INIT;
	int		i, j, x, y, n, ix1, ix2, iy1, iy2, nlines;
	float		font_scale;
	double		sx, x1, y1, x2, y2, pw, ph;
	double		left_margin, right_margin, top_margin, bottom_margin;
	DrawStruct	d;
	AxesParm	a, *return_a;
	DSegment	*s;
	char		*string, *t, *tok, *next;
	AxesCursor	c;
	Rectangle	r;

	if(ax->setvalues_redisplay) {
            Redisplay((Widget)aw, NULL, NULL);
	}

	/* font_scale to go back to the original 1/72 inch (point) scale for
	 * font printing
	 */
	font_scale = 1./POINTS_PER_DOT;

	memcpy(&a, &axes_parm_init, sizeof(AxesParm));
	a.axis_font_height = (int)(DOTS_PER_INCH*p->fonts.axis_fontsize/72.);
	a.axis_font_width = (int)(.5*a.axis_font_height);
	a.label_font_width = (int)(.5*a.label_font_height);
	a.label_font_height = (int)(DOTS_PER_INCH*p->fonts.label_fontsize/72.);
	a.title_font_width = (int)(.5*a.title_font_height);
	a.title_font_height = (int)(DOTS_PER_INCH*p->fonts.title_fontsize/72.);
	a.top_title = NULL;

	/* left,right,bottom,top = plot boundary in inches
	 */
	/* portrait  full page: left=.5, right=8., bottom=.5, top=10.5 
	 * landscape full page: left=.5, right=10.5, bottom=.5, top=8.
	 */
	r = _AxesLayout(aw);

	pw = p->right - p->left;
	ph = p->top - p->bottom;
	left_margin = pw*(double)ax->left_margin/r.width;
	right_margin = pw*(double)ax->right_margin/r.width;
	top_margin = ph*(double)ax->top_margin/r.height;
	bottom_margin = ph*(double)ax->bottom_margin/r.height;

	ix1 = (int)((p->left + left_margin)*DOTS_PER_INCH);
	ix2 = (int)((p->right - right_margin)*DOTS_PER_INCH);
	iy1 = (int)((p->bottom + bottom_margin)*DOTS_PER_INCH);
	iy2 = (int)((p->top - top_margin)*DOTS_PER_INCH);

	if(!ax->reverse_x)
	{
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	}
	else
	{
	    x1 = ax->x2[ax->zoom];
	    x2 = ax->x1[ax->zoom];
	}
	if(!ax->reverse_y)
	{
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	}
	else
	{
	    y1 = ax->y2[ax->zoom];
	    y2 = ax->y1[ax->zoom];
	}

	SetDrawArea(&d, ix1, iy1, ix2, iy2);

	if(!ax->save_space) {
	    if(!p->top_title_user &&
		    ax->a.top_title != NULL && strlen(ax->a.top_title) > 0) {
		a.top_title = strdup(ax->a.top_title);
	    }
	    else if(p->top_title != NULL) {
		a.top_title = strdup(p->top_title);
	    }
	}

	nlines = 0;
	if(p->top_title_user) {
	    nlines = p->top_title_lines;
	    a.top_title = strdup(" ");
	}
	else
	{
	    if(a.top_title != NULL && (int)strlen(a.top_title) > 0)
	    {
		t = strdup(a.top_title);
		tok = t;
		nlines = 0;
		for(i = 0; (next = strtok(tok, "\n")) != NULL; i++) {
		    tok = NULL;
		    if(strlen(next) > 0) nlines++;
		}
		Free(t);
	    }
	}
	if(nlines >= 2) {	/* increase top margin */
	    a.title_font_height *= (nlines/2);
	}

	if(ax->a.x_axis_label != NULL && strlen(ax->a.x_axis_label) > 0) {
	    a.x_axis_label = strdup(ax->a.x_axis_label);
	}
	else if(p->x_axis_label != NULL) {
	    a.x_axis_label = strdup(p->x_axis_label);
	}
	else a.x_axis_label = NULL;

	if(ax->a.y_axis_label != NULL && strlen(ax->a.y_axis_label) > 0) {
	    a.y_axis_label = strdup(ax->a.y_axis_label);
	}
	else if(p->y_axis_label != NULL) {
	    a.y_axis_label = strdup(p->y_axis_label);
	}
	else a.y_axis_label = NULL;

	a.log_x = ax->a.log_x;
	a.log_y = ax->a.log_y;
	a.x_small_log = ax->a.x_small_log;
	a.x_medium_log = ax->a.x_medium_log;
	a.y_small_log = ax->a.y_small_log;
	a.y_medium_log = ax->a.y_medium_log;

	a.y_label_int = ax->a.y_label_int;
	a.uniform_scale = ax->a.uniform_scale;
	a.center = p->center;
	a.ew = ax->a.ew;
	a.ns = ax->a.ns;
	a.min_xlab = ax->a.min_xlab;
	a.max_xlab = ax->a.max_xlab;
	a.min_ylab = ax->a.min_ylab;
	a.max_ylab = ax->a.max_ylab;

	if(p->min_xlab > -1) a.min_xlab = p->min_xlab;
	if(p->max_xlab > -1) a.max_xlab = p->max_xlab;
	if(p->min_ylab > -1) a.min_ylab = p->min_ylab;
	if(p->max_ylab > -1) a.max_ylab = p->max_ylab;
	if(p->min_xsmall > -1) a.min_xsmall = p->min_xsmall;
	if(p->max_xsmall > -1) a.max_xsmall = p->max_xsmall;
	if(p->min_ysmall > -1) a.min_ysmall = p->min_ysmall;
	if(p->max_ysmall > -1) a.max_ysmall = p->max_ysmall;

	if(input_a != NULL) {
	    a.auto_x = input_a->auto_x;
	    a.auto_y = input_a->auto_y;
	    a.left   = input_a->left;
	    a.right  = input_a->right;
	    a.top    = input_a->top;
	    a.bottom = input_a->bottom;
	}

	a.ymax = (ax->y1[0] < ax->y2[0]) ? ax->y1[0] : ax->y2[0];
	a.ymax = (ax->y2[0] > ax->y1[0]) ? ax->y2[0] : ax->y1[0];
	a.check_y_cursor = haveCrosshair(ax);
	gdrawAxis(&a, &d, x1, x2, ax->x_axis, y1, y2, ax->y_axis,
		ax->tickmarks_inside, ax->display_axes_labels,
		ax->time_scale, time_factor(aw, False));

	if(nlines >= 2) {	/* reset to correct value */
	    a.title_font_height /= (nlines/2);
	}

	if(p->color)
	{
	    Pixel white = StringToPixel((Widget)aw, "White");
	    if(aw->core.background_pixel != white) {
		postColor(fp, aw->core.background_pixel);
		fprintf(fp, "%d %d %d %d rectfill\n",
		    d.ix1, d.iy1, d.ix2 - d.ix1,  d.iy2 - d.iy1);
	    }
	    postColor(fp, ax->fg);
	}

	fprintf(fp, "2 slw\n");
	SetClipArea(&d, x1, y1, x2, y2);

	if(ax->hard_copy_func != NULL)
	{
	    (*(ax->hard_copy_func))(aw, fp, &d, &a, font_scale, p);
	}

	SetClipArea(&d, x1, y1, x2, y2);

	fprintf(fp, "2 slw\n");

	if(p->color)
	{
	    postColor(fp, ax->fg);
	}
	if(ax->display_axes_labels == AXES_X ||
	   ax->display_axes_labels == AXES_XY)
	{
	    s = a.axis_segs[0].segs;
	    for(j = 0; j < a.axis_segs[0].n_segs; j++) {
		fprintf(fp, "%d %d M\n", s[j].x1, s[j].y1);
		fprintf(fp, "%d %d D\n", s[j].x2, s[j].y2);
	    }
	    if( !ax->save_space && ax->extra_x_tics ) {
		s = a.axis_segs[1].segs;
		for(j = 0; j < a.axis_segs[1].n_segs; j++) {
		    fprintf(fp, "%d %d M\n", s[j].x1, s[j].y1);
		    fprintf(fp, "%d %d D\n", s[j].x2, s[j].y2);
		}
	    }
	    else if(ax->a.axis_segs[1].n_segs > 0) {
		s = a.axis_segs[1].segs;
		fprintf(fp, "%d %d M\n", s[0].x1, s[0].y1);
		fprintf(fp, "%d %d D\n", s[0].x2, s[0].y2);
	    }
	}
	else
	{
	    for(i = 0; i < 2; i++)
		if(a.axis_segs[i].n_segs > 0)
	    {
		s = a.axis_segs[i].segs;
		fprintf(fp, "%d %d M\n", s[0].x1, s[0].y1);
		fprintf(fp, "%d %d D\n", s[0].x2, s[0].y2);
	    }
	}
	if(ax->display_axes_labels == AXES_Y ||
	   ax->display_axes_labels == AXES_XY)
	{
	    s = a.axis_segs[2].segs;
	    for(j = 0; j < a.axis_segs[2].n_segs; j++) {
		fprintf(fp, "%d %d M\n", s[j].x1, s[j].y1);
		fprintf(fp, "%d %d D\n", s[j].x2, s[j].y2);
	    }
	    if( !ax->save_space && ax->extra_y_tics ) {
		s = a.axis_segs[3].segs;
		for(j = 0; j < a.axis_segs[3].n_segs; j++)
		{
		    fprintf(fp, "%d %d M\n", s[j].x1, s[j].y1);
		    fprintf(fp, "%d %d D\n", s[j].x2, s[j].y2);
		}
	    }
	    else if(ax->a.axis_segs[3].n_segs > 0) {
		s = a.axis_segs[3].segs;
		fprintf(fp, "%d %d M\n", s[0].x1, s[0].y1);
		fprintf(fp, "%d %d D\n", s[0].x2, s[0].y2);
	    }
	}
	else
	{
	    for(i = 2; i < 4; i++)
		if(a.axis_segs[i].n_segs > 0)
	    {
		s = a.axis_segs[i].segs;
		fprintf(fp, "%d %d M\n", s[0].x1, s[0].y1);
		fprintf(fp, "%d %d D\n", s[0].x2, s[0].y2);
	    }
	}

	fprintf(fp, "0 slw\n");

	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		p->fonts.axis_font, p->fonts.axis_fontsize);

	if(ax->display_axes_labels == AXES_X ||
	   ax->display_axes_labels == AXES_XY)
	{
	    for(i = 0; i < a.nxlab; i++) if(!ax->a.xlab_off[i])
	    {
		x = unscale_x(&d, a.x_lab[i]);
		if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
		{
		    y = (int)(a.y_xlab - .5*a.axis_font_height);
		}
		else
		{
		    y = (int)(a.y_xlab - 1.*a.axis_font_height);
		}
		fprintf(fp, "%d %d M\n", x, y);
		fprintf(fp, "(%s) %.5f PC\n", a.xlab[i], font_scale);
	    }
	}
	if(ax->draw_y_labels && (ax->display_axes_labels == AXES_Y ||
	   ax->display_axes_labels == AXES_XY))
	{
	    for(i = 0; i < a.nylab; i++)
		if(!ax->a.ylab_off[i] && a.ylab[i] != NULL)
	    {
		y = unscale_y(&d, a.y_lab[i]);
		if((y >= iy1 && y <= iy2) || (y >= iy2 && y <= iy1)) {
		    x = a.r_ylab;
		    if(!a.log_y || a.y_small_log || a.y_medium_log)
		    {
			fprintf(fp, "N %d %d M\n", x, y);
			fprintf(fp, "%.5f (%s) PR\n", font_scale, a.ylab[i]);
		    }
		    else {
			y += (int)(.5*a.axis_font_height);
			fprintf(fp, "N %d %d M\n", x, y);
			fprintf(fp, "%.5f (%s) PR\n", font_scale, a.ylab[i]);
			y -= a.axis_font_height;
			fprintf(fp, "N %d %d M\n", x, y);
			fprintf(fp, "%.5f dup scale\n", font_scale);
			fprintf(fp,
			    "(10%s) stringwidth pop neg 0 rmoveto\n",a.ylab[i]);
			fprintf(fp, "(10) show %.5f dup scale\n",1./font_scale);
		    }
		}
	    }
	}
	for(i = 0; i < 4; i++)
	{
	    if(a.axis_segs[i].segs != NULL)
	    {
		Free(a.axis_segs[i].segs);
	    }
	    a.axis_segs[i].size_segs = 0;
	    a.axis_segs[i].n_segs = 0;
	}
	for(i = 0; i < MAXLAB; i++)
	{
	    Free(a.xlab[i]);
	    Free(a.ylab[i]);
	}

	x = unscale_x(&d, .5*(x1+x2));

	if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM) {
	    if(ax->save_space) {
		y = (int)(unscale_y(&d, y2) + 1.5*a.axis_font_height);
	    }
	    else {
		y = (int)(unscale_y(&d, y2) + 2.5*a.axis_font_height);
	    }
	}
	else {
	    y = (int)(.5*(d.iy1 + unscale_y(&d, y1)));
	}

	if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM) {
	    y += (int)((nlines-1)*1.1*a.title_font_height);
	}
	p->top_title_x = x;
	p->top_title_y = y;

	if(p->top_title_user || (p->top_title && (int)strlen(p->top_title) > 0)
		|| (a.top_title != NULL && (int)strlen(a.top_title) > 0))
	{
	    fprintf(fp, "/%s findfont %d scalefont setfont\n",
		p->fonts.title_font, p->fonts.title_fontsize);

	    if(!p->top_title_user)
	    {
		char *title;
		if(p->top_title && (int)strlen(p->top_title) > 0) {
		    title = p->top_title;
		}
		else {
		    title = a.top_title;
		}
		t = strdup(title);
		tok = t;
		nlines = 0;
		for(i = 0; (next = strtok(tok, "\n")) != NULL; i++) {
		    tok = NULL;
		    if(strlen(next) > 0) {
			fprintf(fp, "%d %d M\n", x, y);
			string = _Axes_check_parentheses(next);
			fprintf(fp, "(%s) %.5f PC\n", string, font_scale);
			Free(string);
			y -= (int)(1.1*a.title_font_height);
			nlines++;
		    }
		}
		Free(t);
	    }
	}
	Free(a.top_title);

	x = unscale_x(&d, .5*(x1+x2));
	if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
	{
/*
	    y = .5*(a.y_xlab + d.iy1) - a.label_font_height;
*/
	    y = (int)(a.y_xlab - 2.*a.label_font_height);
	    if(a.log_x && fabs(ax->y2[ax->zoom]-ax->y1[ax->zoom]) >= 1.) {
		y -= a.axis_font_height;
	    }
	}
	else
	{
	    y = (int)(.5*(a.y_xlab + d.iy2) - a.label_font_height);
	}
	p->x_axis_x = x;
	p->x_axis_y = y;

	if(a.x_axis_label != NULL && (int)strlen(a.x_axis_label) > 0)
	{
	    fprintf(fp, "/%s findfont %d scalefont setfont\n",
			p->fonts.label_font, p->fonts.label_fontsize);
	    fprintf(fp, "%d %d M\n", x, y);
	    string = _Axes_check_parentheses(a.x_axis_label);
	    fprintf(fp, "(%s) %.5f PC\n", string, font_scale);
	    Free(string);
	}
	Free(a.x_axis_label);
	if(a.y_axis_label != NULL && (int)strlen(a.y_axis_label) > 0)
	{
	    /* x = d.ix1 + a.label_font_height; */
	    x = a.r_ylab - a.maxcy - 
			2*(unscale_x(&d, x1 + a.ytic) - unscale_x(&d, x1));
	    y = unscale_y(&d, .5*(y1+y2));
	    fprintf(fp, "/%s findfont %d scalefont setfont\n",
			p->fonts.label_font, p->fonts.label_fontsize);
	    fprintf(fp, "%d %d M\n", x, y);
	    fprintf(fp, "%.2f r\n", 90.);
	    string = _Axes_check_parentheses(a.y_axis_label);
	    fprintf(fp, "(%s) %.5f PC\n", string, font_scale);
	    Free(string);
	    fprintf(fp, "%.2f r\n", -90.);
	}
	Free(a.y_axis_label);
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		p->fonts.axis_font, p->fonts.axis_fontsize);

	ix1 = unscale_x(&d, x1);
	ix2 = unscale_x(&d, x2);
	iy1 = unscale_y(&d, y1);
	iy2 = unscale_y(&d, y2);

	for(i = 0; i < ax->num_cursors; i++)
	{
	    memcpy(&c, &ax->cursor[i], sizeof(AxesCursor));

	    CursorSegs(aw, &d, &c);

	    for(j = 0; j < c.nsegs; j++)
	    {
		fprintf(fp, "%d %d M\n", c.segs[j].x1, c.segs[j].y1);
		fprintf(fp, "%d %d D\n", c.segs[j].x2, c.segs[j].y2);
	    }
	    if(c.type == AXES_VER_DOUBLE_LINE)
	    {
		sx = .5*(c.scaled_x1 + c.scaled_x2);
	    }
	    else
	    {
		sx = c.scaled_x;
	    }

	    if( c.draw_labels )
	    {
		if(c.label_x > 0 && (n=(int)strlen(ax->cursor[i].label)) >0)
		{
		    x = unscale_x(&d, sx);
		    y = (int)((ax->x_axis == BOTTOM || ax->y_axis ==
			BOTTOM) ? iy2 + 1.*a.axis_font_height :
				  iy1 - 2.*a.axis_font_height);
		    fprintf(fp, "%d %d M\n", x, y);
		    fprintf(fp, "(%s) %.5f PC\n", c.label, font_scale);
		}
		if(c.xlab[0] != '\0' && !c.x_off)
		{
		    x = unscale_x(&d, c.scaled_x1);
		    y = (int)((ax->x_axis == BOTTOM || ax->y_axis ==
			BOTTOM)  ? a.y_xlab - .5*a.axis_font_height :
			a.y_xlab - 1.*a.axis_font_height);
		    fprintf(fp, "%d %d M\n", x, y);
		    fprintf(fp, "(%s) %.5f PC\n", c.xlab, font_scale);
		}
		if(c.ylab[0] != '\0' && !c.y_off)
		{
		    y = unscale_y(&d, ax->cursor[i].scaled_y);
		    x = a.r_ylab;
		    fprintf(fp, "N %d %d M\n", x, y);
		    fprintf(fp, "%.5f (%s) PR\n", font_scale, c.ylab);
		}
	    }
	}

	return_a = (AxesParm *)AxesMalloc((Widget)aw, sizeof(AxesParm));
	if(!return_a) return NULL;

	*return_a = a;

	return return_a;
}

void
AxesPostColor(AxesWidget w, FILE *fp, Pixel pixel)
{
    XColor color_info;

    color_info.pixel = pixel;
    XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
		DefaultScreen(XtDisplay(w))), &color_info);
    fprintf(fp, "%f %f %f setrgbcolor\n",
		(float)color_info.red/65535.,
		(float)color_info.green/65535.,
		(float)color_info.blue/65535.);
}

/** 
 *  
 */
char *
_Axes_check_parentheses(char *s)
{
	int		i, j, nc;
	char		*p;

	nc = (int)strlen(s);
	p = (char *)malloc(2*nc);

	for(i = j = 0; i < nc; i++, j++)
	{
	    if(s[i] == '(' || s[i] == ')') {
		p[j] = '\\';
		j++;
	    }
	    p[j] = s[i];
	}
	p[j] = '\0';
	return(p);
}

/** 
 *  
 */
void
_AxesClearArea(AxesWidget w, int x, int y, int width, int height)
{
	AxesPart *ax = &w->axes;
	if( !XtIsRealized((Widget)w) ) return;

	if(!ax->use_pixmap)
	{
		XClearArea(XtDisplay(w), XtWindow(w), x, y, width,height,False);
	}
	else 
	{
		XFillRectangle(XtDisplay(w), ax->pixmap, ax->eraseGC,
			x, y, width, height);
	}
}

/** 
 *  
 */
void
_AxesClearArea2(AxesWidget w, int x, int y, int width, int height)
{
	AxesPart *ax = &w->axes;
	if( !XtIsRealized((Widget)w) ) return;

	if(!ax->use_pixmap || ax->use_screen)
	{
		XClearArea(XtDisplay(w), XtWindow(w), x, y, width,height,False);
	}
	if(ax->use_pixmap)
	{
		XFillRectangle(XtDisplay(w), ax->pixmap, ax->eraseGC,
			x, y, width, height);
	}
}

/** 
 *  
 */
void
_AxesDrawSegments(AxesWidget w, GC gc, DSegment *segs, int nsegs)
{
	AxesPart *ax = &w->axes;
	if(nsegs <= 0 || !XtIsRealized((Widget)w) ) return;
	if(ax->use_pixmap)
	{
		XDrawSegments(XtDisplay(w), ax->pixmap, gc, (XSegment *)segs,
				nsegs);
	}
	else
	{
		XDrawSegments(XtDisplay(w), XtWindow(w), gc, (XSegment *)segs,
				nsegs);
	}
}

/** 
 *  
 */
void
_AxesDrawSegments2(AxesWidget w, GC gc, DSegment *segs, int nsegs)
{
	AxesPart *ax = &w->axes;
	if(nsegs <= 0 || !XtIsRealized((Widget)w) ) return;

	if(!ax->use_pixmap || ax->use_screen)
	{
		XDrawSegments(XtDisplay(w), XtWindow(w), gc, (XSegment *)segs,
				nsegs);
	}
	if(ax->use_pixmap)
	{
		XDrawSegments(XtDisplay(w), ax->pixmap, gc, (XSegment *)segs,
				nsegs);
	}
}

void AxesClass::drawRectangle(GC gc, int x1, int y1, int width, int height)
{
    AxesPart *ax = &aw->axes;
    if( !XtIsRealized((Widget)aw) ) return;

    XDrawRectangle(XtDisplay(aw), XtWindow(aw), gc, x1, y1, width,height);

    if(ax->use_pixmap) {
	XDrawRectangle(XtDisplay(aw), ax->pixmap, gc, x1, y1, width, height);
    }
}

/** 
 *  
 */
void
_AxesDrawString(AxesWidget w, GC gc, int x, int y, const char *str)
{
	AxesPart *ax = &w->axes;

	if( !XtIsRealized((Widget)w) || str == NULL || str[0] == '\0') return;
	if(ax->use_pixmap)
	{
		XDrawString(XtDisplay(w), ax->pixmap, gc, x, y,
				str, (int)strlen(str));
	}
	else
	{
		XDrawString(XtDisplay(w), XtWindow(w), gc, x, y,
				str, (int)strlen(str));
	}
}

/** 
 *  
 */
void
_AxesDrawString2(AxesWidget w, GC gc, int x, int y, const char *str)
{
	AxesPart *ax = &w->axes;

	if( !XtIsRealized((Widget)w) || str == NULL || str[0] == '\0') return;
	if(!ax->use_pixmap || ax->use_screen)
	{
		XDrawString(XtDisplay(w), XtWindow(w), gc, x, y, str,
				(int)strlen(str));
	}
	if(ax->use_pixmap)
	{
		XDrawString(XtDisplay(w), ax->pixmap, gc, x, y,
				str, (int)strlen(str));
	}
}

static double
time_factor(AxesWidget w, Boolean setLabel)
{
	AxesPart *ax = &w->axes;
	double d;
	if(ax->time_scale != TIME_SCALE_VARIABLE) return(1.);

	d = (ax->x_axis != LEFT) ?
		fabs(ax->x2[ax->zoom]-ax->x1[ax->zoom]):
		fabs(ax->y2[ax->zoom]-ax->y1[ax->zoom]);

	return _AxesTimeFactor(w, d, setLabel);
}

/** 
 *  
 */
double
_AxesTimeFactor(AxesWidget w, double d, Boolean setLabel)
{
	AxesPart *ax = &w->axes;
	int 	i;
	static int nrange = 10;
	static struct
	{
		const char	*label;
		double		factor;
		double		cutoff;
	} range[10] =
	{
		{ "Time (nanoseconds)", 1.e-9, 1.e-7},
		{ "Time (microseconds)", 1.e-6, .0001},
		{ "Time (milliseconds)", .001, .1},
		{ "Time (seconds)", 1., 120. },
		{ "Time (minutes)", 60., 7200. },
		{ "Time (hours)", 3600., 172800. },
		{ "Time (days)", 86400., 31536000. },
		{ "Time (years)", 31536000., 3.e+13},
		{ "Time (millions of years)", 3.1536e+13, 3.e+16},
		{ "Time (billions of years)", 3.1536e+16, 0.},
	};

	if(ax->time_scale != TIME_SCALE_VARIABLE)
	{
		return(1.);
	}
	for(i = 0; i < nrange-1; i++)
	{
	    if(d < range[i].cutoff)
	    {
		if(setLabel)
		{
		    if(ax->x_label == NULL ||
			strcasecmp(ax->x_label, range[i].label))
		    {
			new_x_label(w, w, range[i].label);
		    }
		}
		return(range[i].factor);
	    }
	}
	if(setLabel) {
	    if(ax->x_label == NULL ||
		strcasecmp(ax->x_label, range[nrange-1].label))
	    {
		new_x_label(w, w, range[nrange-1].label);
	    }
	}
	return(range[nrange-1].factor);
}

static void
new_x_label(AxesWidget cur, AxesWidget nou, const char *label)
{
	if(cur->axes.x_axis != LEFT)
	{
	    Free(cur->axes.a.x_axis_label);
	    nou->axes.a.x_axis_label = (label != NULL &&
		nou->axes.display_x_label) ? strdup(label) : strdup("");
	    nou->axes.x_label = nou->axes.a.x_axis_label;
	}
#ifndef NO_IMAGE
	else 
	{
	    Free(cur->axes.a.y_axis_label);
	    nou->axes.a.y_axis_label = (label != NULL &&
		nou->axes.display_y_label) ? strdup(label) : strdup("");
	    nou->axes.y_label = nou->axes.a.y_axis_label;

	    if( !XtIsRealized((Widget)cur) ) return;

	    if(cur->axes.y_label_image != (XImage *)NULL)
	    {
		XDestroyImage(cur->axes.y_label_image);
		nou->axes.y_label_image = (XImage *)NULL;
	    }
	    if((int)strlen(nou->axes.a.y_axis_label) > 0)
	    {
		nou->axes.y_label_image = GetStrImage(XtDisplay(nou),
			XtWindow(nou), nou->axes.axesGC, nou->axes.font,
			nou->axes.eraseGC, nou->axes.a.y_axis_label,
			(int)strlen(nou->axes.a.y_axis_label));
	    }
	}
#endif
}

/** 
 *  
 */
void
#ifdef __STDC__
_AxesWarn(AxesWidget w, const char *format, ...)
#else
_AxesWarn(va_alist) va_dcl
#endif
{
	va_list	va;
	char *warn = NULL;
	int  n;

#ifdef __STDC__
	va_start(va, format);
#else
	AxesWidget w;
	char *format;
	va_start(va);
	w = va_arg(va, Widget);
	format = va_arg(va, char *);
#endif
	if(w  == NULL || format == NULL || (n = (int)strlen(format)) <= 0)
	{
	    va_end(va);
	    return;
	}
	warn = (char *)malloc(n+5000);
	vsnprintf(warn, n+5000, format, va);

	if(XtHasCallbacks((Widget)w, XtNwarningCallback) == XtCallbackHasNone)
	{
	    fprintf(stderr, "%s\n", warn);
	}
	else {
	    XtCallCallbacks((Widget)w, XtNwarningCallback, warn);
	}
	Free(warn);
}

/** 
 *  
 */
void
#ifdef __STDC__
_AxesWarning(AxesWidget w, const char *format, ...)
#else
_AxesWarning(va_alist) va_dcl
#endif
{
	va_list	va;
	char *warn = NULL;
	int  n;

#ifdef __STDC__
	va_start(va, format);
#else
	AxesWidget w;
	char *format;
	va_start(va);
	w = va_arg(va, Widget);
	format = va_arg(va, char *);
#endif
	if(w  == NULL || format == NULL || (n = (int)strlen(format)) <= 0)
	{
	    va_end(va);
	    return;
	}
	warn = (char *)malloc(n+5000);
	vsnprintf(warn, n+5000, format, va);
	free(w->axes.err_msg);
	w->axes.err_msg = strdup(warn);
	Free(warn);
}

char * AxesClass::getError()
{
    return aw->axes.err_msg;
}

void
AxesWaitCursor(Widget w, Boolean on)
{
	AxesWidget widget = (AxesWidget)w;
	AxesPart *ax = &widget->axes;

	if(on)
	{
	    ax->wait_state++;
	    if( XtIsRealized(w) ) {
		XDefineCursor(XtDisplay(w), XtWindow(w),
				GetCursor(w, "hourglass"));
		XFlush(XtDisplay(w));
	    }
        }
        else if(--ax->wait_state <= 0)
        {
	    char type[50];
	    strncpy(type, ax->cursor_type, 50);
	    ax->cursor_type[0] = '\0';
	    _AxesSetCursor((AxesWidget)w, type);
            ax->wait_state = 0;
        }
}

void *
AxesMalloc(Widget w, int nbytes)
{
    void *ptr;

    if(nbytes <= 0) {
	return (void *)NULL;
    }
    else if((ptr = malloc(nbytes)) == (void *)0)
    {
	char error[200];
	if(errno > 0) {
	    snprintf(error, sizeof(error), "malloc error: %s", strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error), "malloc error on %d bytes", nbytes);
	}
	if(XtHasCallbacks(w, XtNwarningCallback) != XtCallbackHasNone) {
	    XtCallCallbacks(w, XtNwarningCallback, error);
	}
	else {
	    fprintf(stderr, "%s\n", error);
	}
	return (void *)NULL;
    }
    return ptr;
}

void *
AxesRealloc(Widget w, void *ptr, int nbytes)
{
    if(nbytes <= 0) {
	return ptr;
    }
    else if(ptr == (void *)NULL) {
	return(AxesMalloc(w, nbytes));
    }
    else if((ptr = realloc(ptr, nbytes)) == (void *)0)
    {
	char error[200];
	if(errno > 0) {
	    snprintf(error, sizeof(error), "malloc error: %s", strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error), "malloc error on %d bytes", nbytes);
	}
	if(XtHasCallbacks(w, XtNwarningCallback) != XtCallbackHasNone) {
	    XtCallCallbacks(w, XtNwarningCallback, error);
	}
	else {
	    fprintf(stderr, "%s\n", error);
	}
	return (void *)NULL;
    }
    return ptr;
}

#ifdef __ZZ__
static void
CvStringToDouble(XrmValuePtr *args, Cardinal *num_args, XrmValuePtr fromVal,
		XrmValuePtr toVal)
{
        char    *s = fromVal->addr;     /* string to convert */
        static double   val;            /* returned value */
	/* need the following, even though we have stdlib.h */

        if (s == NULL) val = 0.0;
        else val = atof (s);
        toVal->size = sizeof(val);
        toVal->addr = (char*)(XtPointer) &val;
}
#endif

/** 
 *  Find the distance from a point to a line.
 *  Find the minimum squared distance from point cx,cy to the
 *  line segment ax,ay to bx,by.
 *  
 */
float AxesClass::pointToLine(float ax, float ay, float bx, float by,
		float cx, float cy)
{
    float px, py, qx, qy, rx, ry, sx, sy, pr, qs, rr, pp, dd;
	
    /* vector definitions:
     *	p = b - a
     *	q = a - b
     *	r = c - a
     *	s = c - b
     */
    px = bx - ax;
    py = by - ay;
    qx = -px;
    qy = -py;
    rx = cx - ax;
    ry = cy - ay;
    sx = cx - bx;
    sy = cy - by;
	
    /* form dot products p*r and q*s */
    pr = px*rx + py*ry;
    qs = qx*sx + qy*sy;
	
    if(pr <= 0.) {
	dd = rx*rx + ry*ry;
    }
    else if(qs <= 0.) {
	dd = sx*sx + sy*sy;
    }
    else {
	rr = rx*rx + ry*ry;
	pp = px*px + py*py;
	dd = rr - pr*pr/pp;
    }
    return dd;
}

void AxesClass::setMinDelx(double delx_min)
{
    aw->axes.delx_min = delx_min;
}

void AxesClass::setMinDely(double dely_min)
{
    aw->axes.dely_min = dely_min;
}

void AxesSetClass(AxesWidget w, AxesClass *a)
{
    w->axes.axes_class = a;
}
