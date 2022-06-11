/** \file Histgm.c
 *  \brief Defines widget Histgm.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      Histgm Widget -- widget for drawing histograms.
 *
 * FILES
 *      Histgm.h
 *
 * BUGS
 *
 * NOTES
 *
 * AUTHOR
 *		Ivan Henson	8/91
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "widget/HistgmP.h"
extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
}

#define Free(ptr) if(ptr) {free(ptr); ptr=NULL;}

#define	offset(field)		XtOffset(HistgmWidget, histgm.field)
static XtResource	resources[] = 
{
	{XtNdataColor, XtCDataColor, XtRPixel, sizeof(Pixel),
		offset(data_fg), XtRString, (XtPointer)XtDefaultForeground},
	{XtNselectCallback, XtCSelectCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_callback),XtRCallback,
		(caddr_t)NULL},
	{XtNstretchCallback, XtCStretchCallback, XtRCallback,
		sizeof(XtCallbackList), offset(stretch_callback),XtRCallback,
		(caddr_t)NULL},
};
#undef offset

/* Private functions */

//static void ClassInitialize(void);
static void Initialize(Widget req, Widget new_index);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static Boolean SetValues(HistgmWidget cur, HistgmWidget req, HistgmWidget new_index);
static void Redisplay(Widget w, XEvent *event, Region region);
static void HistgmRedraw(AxesWidget w, int type, double shift);
static void DrawBoxes(HistgmWidget w);
static void Destroy(Widget w);
static void Locate(HistgmWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Stretch(HistgmWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static int WhichLine(HistgmWidget w, int cursor_x, int cursor_y);
static void SelectBox(HistgmWidget w);
/*
static void CvStringToDouble(XrmValuePtr *args, Cardinal *num_args,
				XrmValuePtr fromVal, XrmValuePtr toVal);
*/

static char	defTrans[] = "#override\n\
 ~Shift ~Ctrl<Btn1Down>:Locate() \n\
 ~Shift Ctrl<Btn1Down>:	Stretch() \n\
  <Btn1Motion>:		Stretch() \n\
  <Btn1Up>:		Stretch()";

static XtActionsRec     actionsList[] =
{
	{(char *)"Locate",	(XtActionProc)Locate},
	{(char *)"Stretch",	(XtActionProc)Stretch},
};

HistgmClassRec	histgmClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&axesClassRec),	/* superclass */
	(char *)"Histgm",		/* class_name */
	sizeof(HistgmRec),		/* widget_size */
//	ClassInitialize,		/* class_initialize */
	NULL,		/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
        actionsList,                   	/* actions */
        XtNumber(actionsList),		/* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
	NULLQUARK,			/* xrm_class */
	TRUE,				/* compress_motion */
	TRUE,				/* compress_exposure */
	TRUE,				/* compress_enterleave */
	TRUE,				/* visible_interest */
	Destroy,			/* destroy */
	XtInheritResize,		/* resize */
	Redisplay,			/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
        XtInheritTranslations,		/* tm_table */
	NULL,				/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL				/* extension */
	},
	{	/* composite_class fields */
	XtInheritGeometryManager,	/* geometry_manager */
	XtInheritChangeManaged,		/* change_managed */
	XtInheritInsertChild,		/* insert_child */
	XtInheritDeleteChild,		/* delete_child */
	NULL,				/* extension */
	},
	{	/* AxesClass fields */
	0,			/* empty */
	},
};

WidgetClass histgmWidgetClass = (WidgetClass)&histgmClassRec;

/*
static void
ClassInitialize(void)
{
	XtAddConverter(XtRString, XtRDouble, (XtConverter)CvStringToDouble,
		(XtConvertArgList) NULL, (Cardinal) 0);
}
*/

static void
Initialize(Widget w_req, Widget	w_new)
{
	HistgmWidget new_index = (HistgmWidget)w_new;
	XtTranslations translations;

	/* no data displayed yet.
	 */
	new_index->histgm.s.nsegs = 0;
	new_index->histgm.s.segs = (DSegment *)NULL;

	new_index->histgm.num_bins = 0;
	new_index->histgm.bins = NULL;
	new_index->histgm.num_colors = 0;
	new_index->histgm.colors = NULL;
	new_index->histgm.num_stipples = 0;
	new_index->histgm.stipples = NULL;
	new_index->histgm.line = NULL;
	new_index->histgm.ratio = NULL;
	new_index->histgm.selected = 0;
	new_index->histgm.bar_height = 50;
	new_index->histgm.which_line = -1;
	new_index->histgm.ctrl_motion = False;

	translations = XtParseTranslationTable(defTrans);
	XtOverrideTranslations((Widget)new_index, translations);

	new_index->axes.redraw_data_func = HistgmRedraw;

	new_index->axes.vertical_scroll = False;
	new_index->axes.horizontal_scroll = False;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
	HistgmWidget w = (HistgmWidget)widget;
	Pixel black, white;

	w->axes.bottom_margin = w->histgm.bar_height;

	(*((histgmWidgetClass->core_class.superclass)->
			core_class.realize))((Widget)w,  valueMask, attrs);

       	w->histgm.dataGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	w->histgm.selectGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	w->histgm.colorGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

	white = StringToPixel(widget, "White");
	black = StringToPixel(widget, "Black");
	if(w->histgm.data_fg == w->core.background_pixel)
	{
		w->histgm.data_fg = (w->core.background_pixel != black) ?
					black : white;
	}

        XSetForeground(XtDisplay(w), w->histgm.dataGC, w->histgm.data_fg);
        XSetBackground(XtDisplay(w), w->histgm.dataGC,
		w->core.background_pixel);

        XSetForeground(XtDisplay(w), w->histgm.colorGC, w->axes.fg);
	XSetBackground(XtDisplay(w), w->histgm.colorGC,
			w->core.background_pixel);
	XSetForeground(XtDisplay(w), w->histgm.selectGC, w->axes.fg);
	XSetBackground(XtDisplay(w), w->histgm.selectGC,
			w->core.background_pixel);

	w->histgm.last_x_min = unscale_x(&w->axes.d, w->axes.x1[w->axes.zoom]);
	w->histgm.last_x_max = unscale_x(&w->axes.d, w->axes.x2[w->axes.zoom]);
	_HistgmRedraw(w);
}

static Boolean
SetValues(HistgmWidget cur, HistgmWidget req, HistgmWidget new_index)
{
	Boolean redisplay = False;

	if (!XtIsRealized((Widget)cur))
	{
		return(False);
	}

	return (redisplay);
}

static void
Redisplay(Widget widget, XEvent *event, Region region)
{
	HistgmWidget w = (HistgmWidget)widget;
	if(_AxesRedisplayAxes((AxesWidget)w))
	{
	    HistgmRedraw((AxesWidget)w, DATA_REDISPLAY, 0.);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	else
	{
	    DrawBoxes(w);
	    SelectBox(w);
	}
}

static void
HistgmRedraw(AxesWidget widget, int type, double shift)
{
	HistgmWidget w = (HistgmWidget)widget;
	switch(type)
	{
		case DATA_REDISPLAY :
			_HistgmRedisplay(w);
			break;
		case DATA_REDRAW :
			_HistgmRedraw(w);
			break;
		case DATA_JUMP_HOR :
			break;
		case DATA_JUMP_VERT :
			break;
		case DATA_SCROLL_HOR :
			break;
		case DATA_SCROLL_VERT :
			break;
	}
}

/** 
 *
 */
void
_HistgmRedisplay(HistgmWidget w)
{
	if(w->histgm.s.nsegs > 0)
	{
	    XDrawSegments(XtDisplay(w), XtWindow(w), w->histgm.dataGC,
			(XSegment *)w->histgm.s.segs, w->histgm.s.nsegs);
	}

	DrawBoxes(w);
	SelectBox(w);
}

static void
DrawBoxes(HistgmWidget w)
{
	int i, x, y, width;

	if(w->histgm.num_colors > 0)
	{
		XSetFillStyle(XtDisplay(w), w->histgm.colorGC, FillSolid);
	}
	y = w->core.height - w->histgm.bar_height + 20;
	for(i = 0; i < w->histgm.num_colors; i++)
	{
		x = unscale_x(&w->axes.d, w->histgm.line[i]);
		width = unscale_x(&w->axes.d, w->histgm.line[i+1]) - x;
		XSetForeground(XtDisplay(w), w->histgm.colorGC,
			w->histgm.colors[i]);
		XFillRectangle(XtDisplay(w), XtWindow(w), w->histgm.colorGC,
			x, y, width, w->histgm.bar_height - 20);
	}
	if(w->histgm.num_stipples > 0)
	{
		XSetFillStyle(XtDisplay(w), w->histgm.colorGC, FillStippled);
		x = unscale_x(&w->axes.d, w->histgm.line[0]);
		width = unscale_x(&w->axes.d,
				w->histgm.line[w->histgm.num_stipples]) - x;
		XClearArea(XtDisplay(w), XtWindow(w), x, y, width,
				w->histgm.bar_height - 20, False);
	}
	for(i = 0; i < w->histgm.num_stipples; i++)
	{
		x = unscale_x(&w->axes.d, w->histgm.line[i]);
		width = unscale_x(&w->axes.d, w->histgm.line[i+1]) - x;
		XSetStipple(XtDisplay(w), w->histgm.colorGC,
                                        w->histgm.stipples[i]);
		XFillRectangle(XtDisplay(w), XtWindow(w), w->histgm.colorGC,
			x, y, width, w->histgm.bar_height - 20);
	}
}

static void
Destroy(Widget widget)
{
	HistgmWidget w = (HistgmWidget)widget;
	if(w->histgm.colorGC != NULL) XFreeGC(XtDisplay(w), w->histgm.colorGC);
}

void
HistgmInputData(HistgmWidget w, int npts, float *z, float no_data_flag,
	int num_bins, int num_colors, Pixel *colors, int num_stipples,
	Pixmap *stipples, double *lines)
{
	int i, j, n;
	double min=0, max=0;

	if(npts <= 0 || z == NULL || num_bins <= 0)
	{
	    return;
	}

	Free(w->histgm.bins);
	w->histgm.bins = (float *)malloc((num_bins+1)*sizeof(float));
	w->histgm.num_bins = num_bins;

	for(i = 0; i < num_bins; i++)
	{
	    w->histgm.bins[i] = 0.;
	}

	for(i = 0; i < npts && z[i] == no_data_flag; i++);
	if(i < npts) {
	    min = max = z[i];
	}
	for(i = 0; i < npts; i++) if(z[i] != no_data_flag)
	{
	    if(min > z[i]) min = z[i];
	    if(max < z[i]) max = z[i];
	}

	w->histgm.dz = (max - min)/num_bins;

	if(w->histgm.dz == 0.)
	{
	    return;
	}

	w->axes.zoom = 0;
	w->axes.x1[0] = min;
	w->axes.x2[0] = max;

	w->histgm.bins[num_bins] = 0;
	for(i = 0; i < npts; i++) if(z[i] != no_data_flag)
	{
	    j = (int)((z[i]-min)/w->histgm.dz);
	    w->histgm.bins[j] += 1.;
	}
	w->histgm.bins[num_bins-1] += w->histgm.bins[num_bins];

	for(i = 0; i < num_bins; i++) if(w->histgm.bins[i] != 0.)
	{
	    w->histgm.bins[i] = log10(w->histgm.bins[i]);
	}

	for(i = 0, max = 0.; i < num_bins; i++)
	{
	    if(max < w->histgm.bins[i]) max = w->histgm.bins[i];
	}

	w->axes.y1[0] = 0.;
	w->axes.y2[0] = max;

	n = (num_colors > 0) ? num_colors : num_stipples;
	Free(w->histgm.line);
	w->histgm.line = (double *)malloc((n+1)*sizeof(double));
	memcpy(w->histgm.line, lines, (n+1)*sizeof(double));
	Free(w->histgm.ratio);
	w->histgm.ratio = (double *)malloc((n+1)*sizeof(double));

	w->histgm.num_colors = num_colors;
	w->histgm.num_stipples = num_stipples;
	Free(w->histgm.colors);
	Free(w->histgm.stipples);
	w->histgm.colors = (Pixel *)malloc(num_colors*sizeof(Pixel));
	w->histgm.stipples = (Pixmap *)malloc(num_stipples*sizeof(Pixmap));
	memcpy(w->histgm.colors, colors, num_colors*sizeof(Pixel));
	memcpy(w->histgm.stipples, stipples, num_stipples*sizeof(Pixmap));
	
	if(XtIsRealized((Widget)w))
	{
	    _AxesRedraw((AxesWidget)w);
	    _HistgmRedraw(w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _HistgmRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if(w->histgm.selected >= 0) {
	    XtCallCallbacks((Widget)w, XtNselectCallback, &w->histgm.selected);
	}
}

void
HistgmInput(HistgmWidget w, int num_colors, Pixel *colors, int num_stipples,
	Pixmap *stipples, double *lines, int num_bins, float *bins,
	float data_min, float data_max)
{
	int i, n;
	double max;

	if(num_bins <= 0) {
	    return;
	}

	Free(w->histgm.bins);
	w->histgm.bins = (float *)malloc(num_bins*sizeof(float));
	memcpy(w->histgm.bins, bins, num_bins*sizeof(float));
	w->histgm.num_bins = num_bins;

	w->histgm.dz = (data_max - data_min)/num_bins;

	if(w->histgm.dz == 0.) {
	    return;
	}

	w->axes.zoom = 0;
	w->axes.x1[0] = data_min;
	w->axes.x2[0] = data_max;

	for(i = 0, max = 0.; i < num_bins; i++)
	{
	    if(max < w->histgm.bins[i]) max = w->histgm.bins[i];
	}

	w->axes.y1[0] = 0.;
	w->axes.y2[0] = max;

	n = (num_colors > 0) ? num_colors : num_stipples;
	Free(w->histgm.line);
	w->histgm.line = (double *)malloc((n+1)*sizeof(double));
	memcpy(w->histgm.line, lines, (n+1)*sizeof(double));
	Free(w->histgm.ratio);
	w->histgm.ratio = (double *)malloc((n+1)*sizeof(double));

	w->histgm.num_colors = num_colors;
	w->histgm.num_stipples = num_stipples;
	Free(w->histgm.colors);
	Free(w->histgm.stipples);
	w->histgm.colors = (Pixel *)malloc(num_colors*sizeof(Pixel));
	w->histgm.stipples = (Pixmap *)malloc(num_stipples*sizeof(Pixmap));
	memcpy(w->histgm.colors, colors, num_colors*sizeof(Pixel));
	memcpy(w->histgm.stipples, stipples, num_stipples*sizeof(Pixmap));
	
	if(XtIsRealized((Widget)w))
	{
	    _AxesRedraw((AxesWidget)w);
	    _HistgmRedraw(w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _HistgmRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if(w->histgm.selected >= 0) {
	    XtCallCallbacks((Widget)w, XtNselectCallback, &w->histgm.selected);
	}
}

void
HistgmLines(HistgmWidget w, int num_colors, int num_stipples, double *lines)
{
	int n;

	n = (num_colors > 0) ? num_colors : num_stipples;
	Free(w->histgm.line);
	w->histgm.line = (double *)malloc((n+1)*sizeof(double));
	memcpy(w->histgm.line, lines, (n+1)*sizeof(double));
	w->histgm.num_colors = num_colors;
	if(w->histgm.colors) {
	    w->histgm.colors = (Pixel *)realloc(w->histgm.colors,
				num_colors*sizeof(Pixel));
	}
	else {
	    w->histgm.colors = (Pixel *)malloc(num_colors*sizeof(Pixel));
	}
	if(w->histgm.ratio) {
	    w->histgm.ratio = (double *)realloc(w->histgm.ratio,
				(n+1)*sizeof(double));
	}
	else {
	    w->histgm.ratio = (double *)malloc((n+1)*sizeof(double));
	}
	w->histgm.num_stipples = num_stipples;

	DrawBoxes(w);
	SelectBox(w);
	if(w->histgm.selected >= 0) {
	    XtCallCallbacks((Widget)w, XtNselectCallback, &w->histgm.selected);
	}
}

void
HistgmColors(HistgmWidget w, int num_colors, Pixel *pixels)
{
	int i;

	for(i = 0; i < num_colors && i < w->histgm.num_colors; i++) {
	    w->histgm.colors[i] = pixels[i];
	}
	DrawBoxes(w);
	SelectBox(w);
}

/** 
 *
 */
void
_HistgmRedraw(HistgmWidget w)
{
	int i, x_min, x_max, bar_y;
	double x, y;
	XRectangle clip_rect;
	
	XClearArea(XtDisplay(w), XtWindow(w), 0, w->core.height -
		w->histgm.bar_height, w->core.width, 20, False);

	Free(w->histgm.s.segs);
	w->histgm.s.nsegs = 0;
	w->histgm.s.size_segs = 0;
	w->axes.d.s = &w->histgm.s;

	SetClipArea(&w->axes.d,
		w->axes.x1[w->axes.zoom], w->axes.y1[w->axes.zoom],
		w->axes.x2[w->axes.zoom], w->axes.y2[w->axes.zoom]);

	for(i = 0; i < w->histgm.num_bins; i++)
	{
		x = w->axes.x1[0] + i*w->histgm.dz;
		imove(&w->axes.d, x, 0.);
		y = w->histgm.bins[i];
		idraw(&w->axes.d, x, y);
		x += w->histgm.dz;
		idraw(&w->axes.d, x, y);
		y = 0.;
		idraw(&w->axes.d, x, y);
	}
	iflush(&w->axes.d);
	SetClipArea(&w->axes.d, 0., 0., 0., 0.);

	XSetClipRectangles(XtDisplay(w), w->histgm.dataGC, 0, 0,
			&w->axes.clip_rect, 1, Unsorted);

	clip_rect.x = w->axes.clip_rect.x;
	clip_rect.y = w->core.height - w->histgm.bar_height;
	clip_rect.width = w->axes.clip_rect.width;
	clip_rect.height = w->histgm.bar_height;
	XSetClipRectangles(XtDisplay(w), w->histgm.colorGC, 0, 0,
			&clip_rect, 1, Unsorted);

	x_min = unscale_x(&w->axes.d, w->axes.x1[w->axes.zoom]);
	x_max = unscale_x(&w->axes.d, w->axes.x2[w->axes.zoom]);
	bar_y = w->core.height - w->histgm.bar_height + 20;
	if(x_min > w->histgm.last_x_min)
	{
	    XClearArea(XtDisplay(w), XtWindow(w), w->histgm.last_x_min,
			bar_y, x_min - w->histgm.last_x_min + 1,
			w->histgm.bar_height - 20, False);
	}
	if(x_max < w->histgm.last_x_max)
	{
	    XClearArea(XtDisplay(w), XtWindow(w), x_max,
			bar_y, w->histgm.last_x_max - x_max,
			w->histgm.bar_height - 20, False);
	}
	w->histgm.last_x_min = x_min;
	w->histgm.last_x_max = x_max;
}

Widget
HistgmCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
	return (XtCreateWidget(name, histgmWidgetClass, parent, 
		arglist, argcount));
}

static void
Locate(HistgmWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	int i, cursor_x, cursor_y, height, x1, x2, y;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	y = w->core.height - w->histgm.bar_height + 20;
	height = w->histgm.bar_height - 20;

	if(event->type == ButtonPress)
	{
	    if(cursor_y < (int)w->core.height - w->histgm.bar_height)
	    {
		_AxesMove((AxesWidget)w, event, (const char **)params,
				num_params);
		return;
	    }
	    for(i = 0; i < w->histgm.num_colors; i++)
	    {
		x1 = unscale_x(&w->axes.d, w->histgm.line[i]);
		x2 = unscale_x(&w->axes.d, w->histgm.line[i+1]);
		if(cursor_x > x1 && cursor_x < x2 &&
		   cursor_y > y && cursor_y < y + height)
		{
		    break;
		}
	    }
	    if(i < w->histgm.num_colors && i != w->histgm.selected)
	    {
		w->histgm.selected = i;
		SelectBox(w);
		XtCallCallbacks((Widget)w, XtNselectCallback, &i);
	    }
	}
}

static void
Stretch(HistgmWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	int i, cursor_x, cursor_y, delx, x, left, right, n;
	double d;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	n = (w->histgm.num_colors > 0) ?
			w->histgm.num_colors : w->histgm.num_stipples;

	if(event->type == ButtonPress)
	{
	    w->histgm.ctrl_motion = True;
	    if(cursor_y < (int)w->core.height - w->histgm.bar_height)
	    {
		_AxesMove((AxesWidget)w, event, (const char **)params,
				num_params);
		return;
	    }
	    if((w->histgm.which_line=WhichLine(w, cursor_x, cursor_y)) < 0)
	    {
		return;
	    }
	    d = w->histgm.line[n-1] - w->histgm.line[1];
	    if(d <= 0.) d = 1.;
	    for(i = 1; i < n-1; i++)
	    {
		w->histgm.ratio[i] = (w->histgm.line[i+1] -w->histgm.line[i])/d;
	    }
	}
	else if(!w->histgm.ctrl_motion)
	{
	    _AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if(event->type == MotionNotify)
	{
	    if(w->histgm.which_line < 0)
	    {
		_AxesScale((AxesWidget)w, event, params, num_params);
		return;
	    }
	    delx = cursor_x - w->histgm.last_cursor_x;
	    x = unscale_x(&w->axes.d, w->histgm.line[w->histgm.which_line]);
	    x += delx;
	    if(w->histgm.which_line == 1)
	    {
		left = unscale_x(&w->axes.d, w->histgm.line[0]);
		right = unscale_x(&w->axes.d, w->histgm.line[n-1]);
	    }
	    else
	    {
		left = unscale_x(&w->axes.d, w->histgm.line[1]);
		right = unscale_x(&w->axes.d, w->histgm.line[n]);
	    }
	    if(x < left + 1)
	    {
		x = left + 1;
	    }
	    else if(x > right - 1)
	    {
		x = right - 1;
	    }
	    w->histgm.line[w->histgm.which_line] = scale_x(&w->axes.d, x);
		
	    d = w->histgm.line[n-1] - w->histgm.line[1];
	    for(i = 1; i < n-1; i++)
	    {
		w->histgm.line[i+1] = w->histgm.line[i] + d*w->histgm.ratio[i];
	    }
	    DrawBoxes(w);
	    SelectBox(w);
	}
	else if(event->type == ButtonRelease)
	{
	    if(w->histgm.which_line < 0)
	    {
		_AxesScale((AxesWidget)w, event, params, num_params);
		return;
	    }
	    w->histgm.which_line = -1;
	    XtCallCallbacks((Widget)w, XtNstretchCallback, w->histgm.line);
	    w->histgm.ctrl_motion = False;
	}
	w->histgm.last_cursor_x = cursor_x;
	w->histgm.last_cursor_y = cursor_y;
}

static int
WhichLine(HistgmWidget w, int cursor_x, int cursor_y)
{
	int left_dis, right_dis, which, x, n;

	n = (w->histgm.num_colors > 0) ?
		w->histgm.num_colors : w->histgm.num_stipples;
	if(n < 2)
	{
	    return(-1);
	}
	x = unscale_x(&w->axes.d, w->histgm.line[1]);
	left_dis = abs(x - cursor_x);
	x = unscale_x(&w->axes.d, w->histgm.line[n-1]);
	right_dis = abs(x - cursor_x);
	which = (left_dis < right_dis) ? 1 : n - 1;
	return(which);
}

static void
SelectBox(HistgmWidget w)
{
	int i, x1, x2, x, y;
	double x_min, x_max;

	y = w->core.height - w->histgm.bar_height + 1;
	XClearArea(XtDisplay(w), XtWindow(w), 0, y, w->core.width, 19, False);

	if(w->histgm.selected < 0 || w->histgm.selected >= w->histgm.num_colors
		|| w->histgm.num_colors <= 0)
	{
	    return;
	}

	i = w->histgm.selected;
	if(w->histgm.line[i] < w->axes.x2[w->axes.zoom] &&
		w->histgm.line[i+1] > w->axes.x1[w->axes.zoom])
	{
	    x_min = (w->histgm.line[i] > w->axes.x1[w->axes.zoom]) ?
				w->histgm.line[i] : w->axes.x1[w->axes.zoom];
	    x_max = (w->histgm.line[i+1] < w->axes.x2[w->axes.zoom]) ?
				w->histgm.line[i+1] : w->axes.x2[w->axes.zoom];
	    x1 = unscale_x(&w->axes.d, x_min);
	    x2 = unscale_x(&w->axes.d, x_max);
	    x = (int)(.5*(x1 + x2));

	    y = w->core.height - w->histgm.bar_height + 10;
	    XDrawLine(XtDisplay(w), XtWindow(w), w->histgm.selectGC,
				x1, y, x2, y);
	    y = w->core.height - w->histgm.bar_height + 1;
	    XDrawLine(XtDisplay(w), XtWindow(w), w->histgm.selectGC,
				x, y, x, y + 18);
	}
}

#ifdef _ZZZ_
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
