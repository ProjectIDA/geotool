/** \file ConPlot.c
 *  \brief Defines widget ConPlot.
 *  \author Ivan Henson
 */
#include "config.h"

/*
 * NAME
 *      ConPlot Widget -- widget for drawing contours.
 *
 * FILES
 *      ConPlot.h
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
#ifdef HAVE_GSL
#include "gsl/gsl_interp.h"
#endif

extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
}

#include "widget/Axes.h"
#include "widget/ConPlotP.h"
#include "widget/ConPlotClass.h"

#define BAR_WIDTH 10
#define DOTS_PER_INCH   300     /* PostScript dots per inch */
#define POINTS_PER_DOT  (72./300.)

#define	offset(field)		XtOffset(ConPlotWidget, con_plot.field)
static XtResource	resources[] = 
{
	{XtNcontourLabelSize, XtCContourLabelSize, XtRInt, sizeof(int),
		offset(contour_label_size), XtRImmediate, (caddr_t)5},
	{XtNcontourLabelColor, XtCContourLabelColor, XtRPixel, sizeof(Pixel),
		offset(contour_label_fg), XtRString,(XtPointer) "red"},
	{XtNplotBackgroundColor, XtCPlotBackgroundColor, XtRPixel,sizeof(Pixel),
		offset(plot_background_pixel), XtRString,(XtPointer) "grey80"},
	{XtNdataColor, XtCDataColor, XtRPixel, sizeof(Pixel),
		offset(data_fg), XtRString, (XtPointer)"medium blue"},
	{XtNautoInterval, XtCAutoInterval, XtRBoolean, sizeof(Boolean),
		offset(auto_interval), XtRString,(XtPointer)"True"},
	{XtNapplyAntiAlias, XtCApplyAntiAlias, XtRBoolean, sizeof(Boolean),
		offset(apply_anti_alias), XtRString, (XtPointer)"True"},
	{XtNmarkMax, XtCMarkMax, XtRBoolean, sizeof(Boolean),
		offset(mark_max), XtRString, (XtPointer)"False"},
	{XtNmode, XtCMode, XtRInt, sizeof(int),
		offset(mode), XtRImmediate,(XtPointer) CONTOURS_ONLY},
	{XtNbarTitle, XtCBarTitle, XtRString, sizeof(String),
                offset(bar_title), XtRString, (XtPointer)NULL},
	{XtNshortBarTitle, XtCShortBarTitle, XtRString, sizeof(String),
                offset(short_bar_title), XtRString, (XtPointer)NULL},
        {XtNselectDataCallback, XtCSelectDataCallback, XtRCallback,
                sizeof(XtCallbackList), offset(select_data_callbacks),
                XtRCallback, (XtPointer)NULL},
        {XtNmouseOverCallback, XtCMouseOverCallback, XtRCallback,
                sizeof(XtCallbackList), offset(mouse_over_callbacks),
                XtRCallback, (XtPointer)NULL},
        {XtNselectBarCallback, XtCSelectBarCallback, XtRCallback,
                sizeof(XtCallbackList), offset(select_bar_callbacks),
                XtRCallback, (XtPointer)NULL},
};
#undef offset

/* private functions */

//static void ClassInitialize(void);
static void Initialize(Widget req, Widget nou);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static void InitImage(ConPlotWidget w);
static Boolean SetValues(ConPlotWidget cur, ConPlotWidget req,
			ConPlotWidget nou);
static void ConPlotRedraw(AxesWidget w, int type, double shift);
static void DrawGrid(ConPlotWidget w, ConEntry *entry);
static void DrawMaximum(ConPlotWidget w);
static void Destroy(Widget w);
static void SetClipping(ConPlotWidget w);
static void DrawEntry(ConPlotWidget w, ConEntry *entry);
static void LabelCrosshair(AxesWidget w, XEvent *event, AxesCursor *c);
static Boolean GetColorCells(ConPlotWidget w, int num_colors);
static Boolean ComputeBar(ConPlotWidget w);
static void ConPlotHardCopy(AxesWidget w, FILE *fp, DrawStruct *d,
			AxesParm *a, float font_scale, PrintParam *p);
static void HardDrawBar(ConPlotWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
			float font_scale, AxesFontStruct *fonts);
static void Motion(ConPlotWidget w, XEvent *event, String *params,
                        Cardinal *num_params);
static void ConPlotZoomOut(ConPlotWidget w);
//static void CvStringToDouble(XrmValuePtr *args, Cardinal *num_args,
//				XrmValuePtr fromVal, XrmValuePtr toVal);
static void putSegments(FILE *fp, SegArray *s);
static void ConPlotMouseDownSelect(ConPlotWidget w, XEvent *e, String *params,
                        Cardinal *num_params);
static void ConPlotMouseDragSelect(ConPlotWidget w, XEvent *e,
			const char **params, Cardinal *num_params);
static Boolean outside(ConPlotWidget w, int cursor_x, int cursor_y);
static Boolean inside_bar(ConPlotWidget w, int cursor_x, int cursor_y);
static void ConPlotBtn1Motion(ConPlotWidget w, XEvent *event, String *param,
			Cardinal *num);
static void ConPlotBtn2Motion(ConPlotWidget w, XEvent *event,const char **param,
			Cardinal *num);
static void ConPlotBtn3Motion(ConPlotWidget w, XEvent *event,const char **param,
			Cardinal *num);
static void getBarTitleDimensions(ConPlotWidget w);


static char     defTrans[] =
"~Shift ~Ctrl<Btn1Down>:MouseDownSelect() \n\
 <Btn1Motion>:		Btn1Motion() \n\
 ~Shift Ctrl<Btn1Down>: MouseDownSelect(ctrl1) \n\
 ~Ctrl Shift<Btn1Down>: MouseDownSelect(shift1) \n\
  Ctrl<Btn2Down>:       MouseDownSelect(ctrl2) \n\
  Ctrl<Btn3Down>:       MouseDownSelect(ctrl3) \n\
 ~Shift ~Ctrl<Btn2Down>:Zoom() \n\
  <Btn2Up>:		Zoom() \n\
  <Btn2Motion>:		Zoom() \n\
 ~Shift ~Ctrl<Btn3Down>:Zoom(horizontal) \n\
  <Btn3Up>:		Zoom(horizontal) \n\
  <Btn3Motion>:		Zoom(horizontal) \n\
 Shift<Btn2Down>:	ZoomBack() \n\
 Ctrl<Btn2Down>:	Magnify() \n\
  <Btn1Up>:		Move() \n\
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
  <BtnDown>:		BtnDown()";

static XtActionsRec     actionsList[] =
{
        {(char *)"MouseDownSelect",     (XtActionProc)ConPlotMouseDownSelect},
	{(char *)"Btn1Motion",		(XtActionProc)ConPlotBtn1Motion},
	{(char *)"Btn2Motion",		(XtActionProc)ConPlotBtn2Motion},
	{(char *)"Btn3Motion",		(XtActionProc)ConPlotBtn3Motion},
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
	{(char *)"Motion",		(XtActionProc)Motion},
};

ConPlotClassRec	conPlotClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&axesClassRec),	/* superclass */
	(char *)"ConPlot",		/* class_name */
	sizeof(ConPlotRec),		/* widget_size */
	NULL,		/* class_initialize */
//	ClassInitialize,		/* class_initialize */
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
	XtInheritExpose,		/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
        defTrans,			/* tm_table */
	NULL,				/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL,				/* extension */
	},
	{       /* composite_class fields */
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

WidgetClass conPlotWidgetClass = (WidgetClass)&conPlotClassRec;

/*
static void
ClassInitialize(void)
{
	XtAddConverter (XtRString, XtRDouble, (XtConverter)CvStringToDouble,
		(XtConvertArgList) NULL, (Cardinal) 0);
}
*/

static void
Initialize(Widget w_req, Widget	w_new)
{
	ConPlotWidget	nou = (ConPlotWidget)w_new;
	/* no data displayed yet.
	 */
	nou->con_plot.contour_interval = 0.;
	nou->con_plot.contour_min = 0.;
	nou->con_plot.contour_max = 0.;
	nou->con_plot.num_entries = 0;
	nou->con_plot.entry = (ConEntry **)NULL;
	nou->con_plot.size_entry = 0;

	nou->con_plot.pixels = (Pixel *)NULL;
	nou->con_plot.cmap = (Colormap) NULL;
	nou->con_plot.plane_masks = NULL;
	nou->con_plot.num_colors = 0;
	nou->con_plot.num_cells = 0;
	nou->con_plot.colors = NULL;
	nou->con_plot.num_stipples = 0;
	nou->con_plot.stipples = NULL;
	nou->con_plot.num_lines = 0;
	nou->con_plot.lines = NULL;
	nou->con_plot.ndeci = 0;
	nou->con_plot.bar_x = 0;
	nou->con_plot.bar_width = 0;
	nou->con_plot.image = (XImage *)NULL;
	nou->con_plot.image_width = 0;
	nou->con_plot.image_height = 0;
 	if(nou->axes.top_margin == 0) {
	    nou->axes.top_margin = 2;
	}
	nou->con_plot.ctrl_select_reason = -1;
	nou->con_plot.num_bar_labels = 0;
	nou->con_plot.bar_values = NULL;
	nou->con_plot.bar_labels = NULL;

	if(nou->con_plot.bar_title && (int)strlen(nou->con_plot.bar_title) > 0)
	{
	    nou->con_plot.bar_title = strdup(nou->con_plot.bar_title);
	}
	else nou->con_plot.bar_title = NULL;

	if(nou->con_plot.short_bar_title &&
			(int)strlen(nou->con_plot.short_bar_title) > 0)
	{
	    nou->con_plot.short_bar_title=strdup(nou->con_plot.short_bar_title);
	}
	else nou->con_plot.short_bar_title = NULL;
/*
	if(nou->con_plot.bar_title && (int)strlen(nou->con_plot.bar_title) == 0)
	{
	    nou->con_plot.bar_title = NULL;
	}
	if(nou->con_plot.short_bar_title &&
		(int)strlen(nou->con_plot.short_bar_title) == 0)
	{
	    nou->con_plot.short_bar_title = NULL;
	}
*/
	nou->con_plot.bar_title_x = 0;
	nou->con_plot.bar_title_y = 0;
	nou->con_plot.bar_title_w = 0;
	nou->con_plot.bar_title_h = 0;
	nou->con_plot.bar_title_image = NULL;
	nou->con_plot.long_bar_title_image = NULL;
	nou->con_plot.short_bar_title_image = NULL;

	/* this is so DrawBar will be exposed when only it is exposed */
	nou->axes.check_expose_region = False;

	nou->axes.clear_on_redisplay = False;
	nou->axes.redraw_data_func = ConPlotRedraw;
	nou->axes.cursor_func = LabelCrosshair;
	nou->axes.hard_copy_func = ConPlotHardCopy;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
	ConPlotWidget w = (ConPlotWidget)widget;
	ConPlotPart *cp = &w->con_plot;
	w->axes.right_margin = cp->bar_width + 2 + cp->bar_title_h;

	(*((conPlotWidgetClass->core_class.superclass)->
			core_class.realize))((Widget)w,  valueMask, attrs);

       	cp->dataGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
       	cp->contour_labelGC= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	cp->colorGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	cp->barGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

	if(cp->data_fg == w->core.background_pixel)
	{
		cp->data_fg = w->axes.fg;
	}
        XSetForeground(XtDisplay(w), cp->dataGC, cp->data_fg);
        XSetBackground(XtDisplay(w), cp->dataGC,
		w->core.background_pixel);
        XSetForeground(XtDisplay(w), cp->contour_labelGC,
		cp->contour_label_fg);
        XSetBackground(XtDisplay(w), cp->contour_labelGC,
		w->core.background_pixel);

        XSetForeground(XtDisplay(w), cp->colorGC, w->axes.fg);
        XSetBackground(XtDisplay(w), cp->colorGC,
		w->core.background_pixel);
        XSetForeground(XtDisplay(w), cp->barGC, w->axes.fg);
        XSetBackground(XtDisplay(w), cp->barGC,
		w->core.background_pixel);
	XSetFont(XtDisplay(w), cp->barGC, w->axes.font->fid);

	InitImage(w);

	if(cp->bar_title)
	{
	    cp->long_bar_title_image = GetStrImage(XtDisplay(w), XtWindow(w),
			w->axes.axesGC, w->axes.font, w->axes.eraseGC,
			cp->bar_title, (int)strlen(cp->bar_title));
	}
	if(cp->short_bar_title)
	{
	    cp->short_bar_title_image = GetStrImage(XtDisplay(w), XtWindow(w),
			w->axes.axesGC, w->axes.font, w->axes.eraseGC,
			cp->short_bar_title, (int)strlen(cp->short_bar_title));
	}

	w->axes.skip_limits_callback = True;
	_AxesRedraw((AxesWidget)w);
	_ConPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_ConPlotRedisplayData(w);
	_ConPlotDrawBar(w);
	_AxesRedisplayXor((AxesWidget)w);
	w->axes.skip_limits_callback = False;
	
}

static void
InitImage(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	Display	*display;
	char	*data;
	int	num_colors, depth;

	if( !XtIsRealized((Widget)w) ) return;

	num_colors = XDisplayCells(XtDisplay(w), DefaultScreen(XtDisplay(w)));
	if(cp->image == NULL && cp->mode != CONTOURS_ONLY && num_colors > 20)
	{
	    cp->image_width = w->core.width;
	    cp->image_height = w->core.height;

	    display = XtDisplay(w);
	    depth = DefaultDepth(display, DefaultScreen(display));

	    if( !(data = (char *)AxesMalloc((Widget)w,
			cp->image_width*cp->image_height*8)) ) return;

	    cp->image = XCreateImage(display,
				DefaultVisual(display, DefaultScreen(display)),
				depth, ZPixmap, 0, data, cp->image_width,
				cp->image_height,  8, 0);
	}
}

static Boolean
SetValues(ConPlotWidget	cur, ConPlotWidget req, ConPlotWidget nou)
{
	ConPlotPart *cur_cp = &cur->con_plot;
	ConPlotPart *req_cp = &req->con_plot;
	Boolean redisplay = False, redraw = False;

	if(!XtIsRealized((Widget)cur)) return(False);

	if(cur->axes.font != req->axes.font) {
	    XSetFont(XtDisplay(nou), req_cp->barGC, nou->axes.font->fid);
	    redraw = True;
	}
	if(cur_cp->auto_interval != req_cp->auto_interval)
	{
		redraw = True;
	}
	if(cur_cp->contour_interval != req_cp->contour_interval
		&& req_cp->auto_interval)
	{
		redraw = True;
	}
	if(cur_cp->contour_min != req_cp->contour_min ||
	   cur_cp->contour_max != req_cp->contour_max)
	{
		redraw = True;
	}
	if(cur_cp->mode != req_cp->mode)
	{
		InitImage(nou);
		redraw = True;
	}
	
	if(redraw)
	{
		_ConPlotRedraw(nou);
		redisplay = True;
	}
	
	if(redisplay)
	{
		if(nou->axes.redisplay_pending) redisplay = False;
		nou->axes.redisplay_pending = True;
	}
	return(redisplay);
}

void ConPlotClass::setParams(double contour_interval, double contour_min,
			double contour_max)
{
    ConPlotPart *cp = &cw->con_plot;

    if(	contour_interval != cp->contour_interval ||
	contour_min != cp->contour_min || contour_max != cp->contour_max)
    {
	cp->contour_interval = contour_interval;
	cp->contour_min = contour_min;
	cp->contour_max = contour_max;

	_AxesRedraw((AxesWidget)cw);
	_ConPlotRedraw(cw);
	_AxesRedisplayAxes((AxesWidget)cw);
	_ConPlotRedisplayData(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	_ConPlotDrawBar(cw);
    }
}

static void
ConPlotRedraw(AxesWidget widget, int type, double shift)
{
	ConPlotWidget w = (ConPlotWidget)widget;
	switch(type)
	{
	    case DATA_REDISPLAY :
		if(!w->axes.redisplay_pending)
		{
		    _ConPlotRedisplayData(w);
		    _ConPlotDrawBar(w);
		}
		break;
	    case DATA_REDRAW :
		_ConPlotRedraw(w);
		break;
	    case DATA_JUMP_HOR :
	    case DATA_JUMP_VERT :
	    case DATA_SCROLL_HOR :
	    case DATA_SCROLL_VERT :
		_ConPlotRedraw(w);
		_ConPlotRedisplayData(w);
		break;
	    case DATA_ZOOM_ZERO :
		ConPlotZoomOut(w);
		break;
	}
}

static void
ConPlotZoomOut(ConPlotWidget w)
{
	AxesPart *ax = &w->axes;

	ax->zoom = 0;
	_AxesRedraw((AxesWidget)w);
	_AxesRedisplayAxes((AxesWidget)w);
	_ConPlotRedraw(w);
	_ConPlotRedisplayData(w);
	_AxesRedisplayXor((AxesWidget)w);
	_ConPlotDrawBar(w);
	if(ax->use_pixmap && XtIsRealized((Widget)w)) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w),
		ax->axesGC, 0, 0, w->core.width,  w->core.height, 0, 0);
	}
}

/** 
 *  
 */
void
_ConPlotRedisplayData(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	AxesPart *ax = &w->axes;
	int	 i, n, ix1, ix2, iy1, iy2;
	ConEntry *entry;
	Drawable drawable = w->axes.use_pixmap ? w->axes.pixmap : XtWindow(w);

	if( !XtIsRealized((Widget)w) ) return;

	XSetClipRectangles(XtDisplay(w), w->axes.axesGC, 0, 0,
			&ax->clip_rect, 1, Unsorted);

	ix1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
	ix2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
	iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
	iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if(ix1 > ix2)
	{
		i = ix1;
		ix1 = ix2;
		ix2 = i;
	}
	if(iy1 > iy2)
	{
		i = iy1;
		iy1 = iy2;
		iy2 = i;
	}
	XSetForeground(XtDisplay(w), ax->axesGC, cp->plot_background_pixel);
	XFillRectangle(XtDisplay(w), drawable, w->axes.axesGC,
			ix1+1, iy1+1, ix2-ix1-1, iy2-iy1-1);
	XSetForeground(XtDisplay(w), ax->axesGC, ax->fg);

	for(i = 0; i < cp->num_entries; i++)
		if(cp->entry[i]->display_data)
	{
	    entry = cp->entry[i];
	    if(cp->mode != COLOR_ONLY)
	    {
		XDrawSegments(XtDisplay(w), drawable, cp->dataGC,
			(XSegment *)entry->s.segs, entry->s.nsegs);
		XDrawSegments(XtDisplay(w), drawable, cp->contour_labelGC,
			(XSegment *)entry->t.segs, entry->t.nsegs);
	    }
	    if(cp->mode != CONTOURS_ONLY)
	    {
		if(cp->image != NULL)
		{
		    XPutImage(XtDisplay(w), drawable, w->axes.axesGC,
			    cp->image, cp->image_x0, cp->image_y0,
			    cp->image_x0, cp->image_y0,
			    cp->image_display_width, cp->image_display_height);
		}
		else
		{
		    entry->m.x1 = w->axes.x1[w->axes.zoom];
		    entry->m.x2 = w->axes.x2[w->axes.zoom];
		    entry->m.y1 = w->axes.y1[w->axes.zoom];
		    entry->m.y2 = w->axes.y2[w->axes.zoom];
		    n = (cp->num_stipples > 0) ?
				cp->num_stipples : cp->num_colors;
		    if(n >= cp->num_lines) n = cp->num_lines-1;
		    shade(XtDisplay(w), drawable, cp->colorGC, &w->axes.d,
			n, cp->pixels, cp->lines, cp->num_stipples,
			cp->stipples, &entry->m);
		}
	    }
	    if(entry->display_grid) {
		DrawGrid(w, entry);
	    }
	}

	XSetClipMask(XtDisplay(w), w->axes.axesGC, None);

	DrawMaximum(w);
}

static void
DrawGrid(ConPlotWidget w, ConEntry *entry)
{
	int i, j, x, y;
	double f, x1, x2, y1, y2;
	XSegment s[2];
	Drawable drawable = w->axes.use_pixmap ? w->axes.pixmap : XtWindow(w);

	x1 = w->axes.x1[w->axes.zoom];
	x2 = w->axes.x2[w->axes.zoom];
	if(x1 > x2)
	{
		f = x1;
		x1 = x2;
		x2 = f;
	}
	y1 = w->axes.y1[w->axes.zoom];
	y2 = w->axes.y2[w->axes.zoom];
	if(y1 > y2)
	{
		f = y1;
		y1 = y2;
		y2 = f;
	}

	for(i = 0; i < entry->m.nx; i++)
		if(entry->m.x[i] >= x1 && entry->m.x[i] <= x2)
	{
	    x = unscale_x(&w->axes.d, (double)entry->m.x[i]);
	    for(j = 0; j < entry->m.ny; j++)
		if(entry->m.y[j] >= y1 && entry->m.y[j] <= y2)
	    {
		y = unscale_y(&w->axes.d,(double)entry->m.y[j]);

		s[0].x1 = x-2;
		s[0].y1 = y;
		s[0].x2 = x+2;
		s[0].y2 = y;
		s[1].x1 = x;
		s[1].y1 = y-2;
		s[1].x2 = x;
		s[1].y2 = y+2;
		if( XtIsRealized((Widget)w) ) {
		    XDrawSegments(XtDisplay(w), drawable, w->axes.axesGC,s,2);
		}
	    }
	}
}

static void
DrawMaximum(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	int m, x, y;
	ConEntry *entry;
	Drawable drawable = w->axes.use_pixmap ? w->axes.pixmap : XtWindow(w);

	if(!cp->mark_max || !XtIsRealized((Widget)w))
	{
		return;
	}
	XSetClipRectangles(XtDisplay(w), w->axes.axesGC,
				0, 0, &w->axes.clip_rect, 1, Unsorted);
	for(m = 0; m < cp->num_entries; m++)
	{
	    entry = cp->entry[m];
	    x = unscale_x(&w->axes.d, entry->xmax);
	    y = unscale_y(&w->axes.d, entry->ymax);
	    XDrawRectangle(XtDisplay(w), drawable, w->axes.axesGC,
			x-3, y-3, 6, 6);
	}
	XSetClipMask(XtDisplay(w), w->axes.axesGC, None);
}
		
static void
Destroy(Widget widget)
{
	ConPlotWidget w = (ConPlotWidget)widget;
	ConPlotPart *cp = &w->con_plot;
	int i;
	ConPlotClear(w);
	for(i = 0; i < cp->num_bar_labels; i++) {
	    Free(cp->bar_labels[i]);
	}
	Free(cp->bar_values);
	Free(cp->bar_labels);
	if(cp->image != (XImage *)NULL) {
	    XDestroyImage(cp->image);
	    cp->image = NULL;
	}
}

void
ConPlotClear(ConPlotWidget w)
{
	int i, ix1, ix2, iy1, iy2;

	ConPlotClearEntries(w);

	ix1 = unscale_x(&w->axes.d, w->axes.x1[w->axes.zoom]);
	ix2 = unscale_x(&w->axes.d, w->axes.x2[w->axes.zoom]);
	iy1 = unscale_y(&w->axes.d, w->axes.y1[w->axes.zoom]);
	iy2 = unscale_y(&w->axes.d, w->axes.y2[w->axes.zoom]);
	if(ix1 > ix2)
	{
		i = ix1;
		ix1 = ix2;
		ix2 = i;
	}
	if(iy1 > iy2)
	{
		i = iy1;
		iy1 = iy2;
		iy2 = i;
	}
	_AxesRedisplayXor((AxesWidget)w);
	_AxesClearArea((AxesWidget)w, ix1+1, iy1+1, ix2-ix1-1, iy2-iy1-1);
	_AxesRedisplayXor((AxesWidget)w);

	ConPlotZoomOut(w);
}

void ConPlotClearEntries(ConPlotWidget w)
{
    ConPlotPart *cp = &w->con_plot;
    ConEntry *entry;

    for(int i = 0; i < cp->num_entries; i++)
    {
	entry = cp->entry[i];
	Free(entry->s.segs);
	Free(entry->t.segs);
	Free(entry->l.ls);
	Free(entry->m.x);
	Free(entry->m.y);
	Free(entry->m.z);
	Free(entry->label);
	Free(entry->select_val);
	Free(entry->dim_val);
	Free(entry);
    }
    cp->num_entries = 0;
}

int ConPlotClass::input(const char *label, int nx, int ny, double *x, double *y,
	float *z, float no_data_flag, int num_lines, double *lines,
	double xmax, double ymax, bool display_data, bool display_grid,
	bool redraw_axes, bool distinctOutliers, Pixel select_color)
{
	ConPlotPart	*cp = &cw->con_plot;
	AxesPart	*ax = &cw->axes;
	Widget		w = (Widget)cw;
	int		i, j;
	ConEntry	*entry;
	double		x_min, x_max, y_min, y_max, min=0, max=0;
	ConEntry	con_entry_init = CON_ENTRY_INIT;
	static int	id = 0;

	if(nx <= 1 || ny <= 1 || x == NULL || y == NULL || z == NULL)
	{
		return(0);
	}
	cp->num_entries++;

	ALLOC(cp->num_entries*sizeof(ConEntry *),
		cp->size_entry, cp->entry, ConEntry *, 0);

	if( !(cp->entry[cp->num_entries-1] = (ConEntry *)AxesMalloc(w,
		sizeof(ConEntry))) ) return(0);

	entry = cp->entry[cp->num_entries-1];

	memcpy(entry, &con_entry_init, sizeof(ConEntry));

        entry->distinctOutliers = distinctOutliers;
        entry->select_color = select_color;

	entry->label = (label != NULL) ? strdup(label) : strdup("");

	if( !(entry->m.x = (double *)AxesMalloc(w, nx*sizeof(double))) )
	{
		Free(entry);
		cp->num_entries--;
		return(0);
	}
	if( !(entry->m.y = (double *)AxesMalloc(w, ny*sizeof(double))) )
	{
		Free(entry->m.x);
		Free(entry);
		cp->num_entries--;
		return(0);
	}
	if( !(entry->m.z = (float *)AxesMalloc(w, nx*ny*sizeof(float))))
	{
		Free(entry->m.x);
		Free(entry->m.y);
		Free(entry);
		cp->num_entries--;
		return(0);
	}
	
	memcpy(entry->m.x, x, nx*sizeof(double));
	memcpy(entry->m.y, y, ny*sizeof(double));
	memcpy(entry->m.z, z, nx*ny*sizeof(float));

	entry->m.exc = no_data_flag;

	entry->xmax = xmax;
	entry->ymax = ymax;
	entry->display_data = display_data;
	entry->display_grid = display_grid;
	if(++id <= 0) id = 1;
	entry->id = id;

	if(redraw_axes) ax->zoom = 0;

	if(num_lines > 0)
	{
		Free(cp->lines);
		if( !(cp->lines = (double *)AxesMalloc(w,
			num_lines*sizeof(double))) )
		{
			Free(entry->m.x);
			Free(entry->m.y);
			Free(entry->m.z);
			Free(entry);
			cp->num_entries--;
			return(0);
		}
		memcpy(cp->lines, lines, num_lines*sizeof(double));
		cp->num_lines = num_lines;
		if(cp->mode != CONTOURS_ONLY)
		{
			if(ComputeBar(cw))
			{
				redraw_axes = True;
			}
		}
	}
		
	entry->m.nx = nx;
	entry->m.ny = ny;
	entry->auto_ci = cp->auto_interval;
	entry->ci = cp->contour_interval;
	entry->c_min = cp->contour_min;
	entry->c_max = cp->contour_max;
	entry->m.imin = entry->m.jmin = 0;
	entry->m.imax = entry->m.jmax = 0;

	if(nx > 0)
	{
		min = max = z[0];
	}
	for(i = 0; i < nx; i++)
		for(j = 0; j < ny; j++)
	{
		if(z[i+j*nx] < min)
		{
			min = z[i+j*nx];
			entry->m.imin = i;
			entry->m.jmin = j;
		}
		if(z[i+j*nx] > max)
		{
			max = z[i+j*nx];
			entry->m.imax = i;
			entry->m.jmax = j;
		}
	}
	entry->m.z_min = min;
	entry->m.z_max = max;

	entry = cp->entry[0];
	if(cp->mode != CONTOURS_ONLY)
	{
		x_min = entry->m.x[0] - .5*(entry->m.x[1] - entry->m.x[0]);
		x_max = entry->m.x[entry->m.nx-1] + .5*(
			entry->m.x[entry->m.nx-1] - entry->m.x[entry->m.nx-2]);
		y_min = entry->m.y[0] - .5*(entry->m.y[1] - entry->m.y[0]);
		y_max = entry->m.y[entry->m.ny-1] + .5*(
			entry->m.y[entry->m.ny-1] - entry->m.y[entry->m.ny-2]);
		for(i = 1; i < cp->num_entries; i++)
		{
			entry = cp->entry[i];
			min = entry->m.x[0] - .5*(entry->m.x[1]-entry->m.x[0]);
			max = entry->m.x[entry->m.nx-1] + .5*(
					entry->m.x[entry->m.nx-1]
					- entry->m.x[entry->m.nx-2]);
			if(x_min > min) x_min = min;
			if(x_max < max) x_max = max;
			min = entry->m.y[0] - .5*(entry->m.y[1]-entry->m.y[0]);
			max = entry->m.y[entry->m.ny-1] + .5*(
					entry->m.y[entry->m.ny-1]
					- entry->m.y[entry->m.ny-2]);
			if(y_min > min) y_min = min;
			if(y_max < max) y_max = max;
		}
	}
	else
	{
		x_min = entry->m.x[0];
		x_max = entry->m.x[entry->m.nx-1];
		y_min = entry->m.y[0];
		y_max = entry->m.y[entry->m.ny-1];
		for(i = 1; i < cp->num_entries; i++)
		{
			entry = cp->entry[i];
			if(x_min > entry->m.x[0]) x_min = entry->m.x[0];
			if(x_max < entry->m.x[entry->m.nx-1])
				x_max = entry->m.x[entry->m.nx-1];
			if(y_min > entry->m.y[0]) y_min = entry->m.y[0];
			if(y_max < entry->m.y[entry->m.ny-1])
				y_max = entry->m.y[entry->m.ny-1];
		}
	}
	if(!ax->zoom &&
		(x_min != ax->x1[0] || x_max != ax->x2[0] ||
		 y_min != ax->y1[0] || y_max != ax->y2[0]))
	{
		redraw_axes = True;
	}
	ax->x1[0] = x_min;
	ax->x2[0] = x_max;
	ax->y1[0] = y_min;
	ax->y2[0] = y_max;

	if(!display_data && !display_grid) return(entry->id);

	if(redraw_axes)
	{
		_AxesRedraw((AxesWidget)w);
		_ConPlotRedraw(cw);
		if(!cw->axes.redisplay_pending)
		{
			_AxesRedisplayAxes((AxesWidget)w);
			_ConPlotRedisplayData(cw);
			_AxesRedisplayXor((AxesWidget)w);
			_ConPlotDrawBar(cw);
		}
	}
	else if(!cw->axes.redisplay_pending)
	{
		_AxesRedisplayXor((AxesWidget)w);
		_ConPlotRedraw(cw);
		_ConPlotRedisplayData(cw);
		_AxesRedisplayXor((AxesWidget)w);
		_ConPlotDrawBar(cw);
	}
	if(ax->use_pixmap && XtIsRealized(w)) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC, 0, 0,
			cw->core.width,  cw->core.height, 0, 0);
	}
	return(entry->id);
}

Matrx * ConPlotClass::getMatrix()
{
    ConPlotPart *cp = &cw->con_plot;

    for(int i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->display_data) {
	    return &cp->entry[i]->m;
	}
    }
    return NULL;
}

void ConPlotClass::dimPixels(int nx, int ny, bool *dim)
{
    ConPlotPart	*cp = &cw->con_plot;
    AxesPart *ax = &cw->axes;

    for(int i = 0; i < cp->num_entries; i++)
    {
	if(cp->entry[i]->dim_val == NULL)
	{
	    cp->entry[i]->dim_val =(Boolean *)malloc(nx*ny*sizeof(Boolean));
	}
	else
	{
	    cp->entry[i]->dim_val =(Boolean *)realloc(cp->entry[i]->dim_val,
						nx*ny*sizeof(Boolean));
        }
	memcpy(cp->entry[i]->dim_val, dim, nx*ny*sizeof(Boolean));
    }

    _ConPlotRedraw(cw);
        
    if(cp->image != NULL) {
	_ConPlotRedisplayData(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	_ConPlotDrawBar(cw);
    }
    if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
	XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width,  cw->core.height, 0, 0);
    }
}

void ConPlotClass::selectCells(int nx, int ny, bool *select)
{
    ConPlotPart *cp = &cw->con_plot;
    AxesPart *ax = &cw->axes;

    for(int i = 0; i < cp->num_entries; i++)
    {
	if(cp->entry[i]->select_val == NULL)
	{
	    cp->entry[i]->select_val = (Boolean *)malloc(nx*ny*sizeof(Boolean));
	}
	else
	{
	    cp->entry[i]->select_val =
		(Boolean *)realloc(cp->entry[i]->select_val,
						nx*ny*sizeof(Boolean));
	}
	memcpy(cp->entry[i]->select_val, select, nx*ny*sizeof(Boolean));
    }

    _ConPlotRedraw(cw);
        
    if(cp->image != NULL) {
	_ConPlotRedisplayData(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	_ConPlotDrawBar(cw);
    }
    if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
	XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width,  cw->core.height, 0, 0);
    }
}

void ConPlotClass::selectAllCells(bool select, bool redisplay)
{
    ConPlotPart *cp = &cw->con_plot;
    AxesPart *ax = &cw->axes;

    for(int i = 0; i < cp->num_entries; i++) {
	Free(cp->entry[i]->select_val);
    }
    if(redisplay) {
	_ConPlotRedraw(cw);
        
	if(cp->image != NULL) {
	    _ConPlotRedisplayData(cw);
	    _AxesRedisplayXor((AxesWidget)cw);
	    _ConPlotDrawBar(cw);
	}
	if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
	    XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width,  cw->core.height, 0, 0);
	}
    }
}

void ConPlotClass::dimEntry(int x, int y, bool value)
{
    ConPlotPart *cp = &cw->con_plot;

    for(int i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->dim_val != NULL) {
	    cp->entry[i]->dim_val[x+(y)*cp->entry[i]->m.nx] = value;
	}
    }

    cw->axes.redisplay_pending = true;

    _AxesRedraw((AxesWidget)cw);
    _ConPlotRedraw(cw);
    _AxesRedisplayAxes((AxesWidget)cw);
    _ConPlotRedisplayData(cw);
    _AxesRedisplayXor((AxesWidget)cw);
    _ConPlotDrawBar(cw);
}

void ConPlotClass::dimOff()
{
    ConPlotPart *cp = &cw->con_plot;
    AxesPart *ax = &cw->axes;
    int	i, j;

    for(i = 0; i < cp->num_entries; i++)
    {
	if(cp->entry[i]->dim_val != NULL) {
	    for(j = 0; j < cp->entry[i]->m.nx * cp->entry[i]->m.ny; j++) {
		cp->entry[i]->dim_val[j] = False;
	    }
	}
    }
    _ConPlotRedraw(cw);
        
    if(cp->image != NULL) {
	_ConPlotRedisplayData(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	_ConPlotDrawBar(cw);
    }
    if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
	XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
		cw->core.width,  cw->core.height, 0, 0);
    }
}

void ConPlotClass::display(int num, int *ids, bool *display_data,
		bool *display_grid)
{
    ConPlotPart *cp = &cw->con_plot;
    AxesPart *ax = &cw->axes;
    int i, j;
    bool  redisplay = false;
    ConEntry *entry;

    for(i = 0; i < num; i++)
    {
	for(j = 0; j < cp->num_entries; j++) if(ids[i] == cp->entry[j]->id)
	{
	    entry = cp->entry[j];
	
	    if(	entry->display_data != display_data[i] ||
		entry->display_grid != display_grid[i])
	    {
		redisplay = true;
	    }

	    entry->display_data = display_data[i];
	    entry->display_grid = display_grid[i];
	}
    }
    if(redisplay)
    {
	_AxesRedraw((AxesWidget)cw);
	_ConPlotRedraw(cw);
	_AxesRedisplayAxes((AxesWidget)cw);
	_ConPlotRedisplayData(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	_ConPlotDrawBar(cw);
    }
    if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
	XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width,  cw->core.height, 0, 0);
    }
}

bool ConPlotClass::output(int *nx, int *ny, double **x, double **y, float **z)
{
    ConPlotPart *cp = &cw->con_plot;
    ConEntry *entry;

    *nx = *ny = 0;
    *x = *y = NULL;
    *z = NULL;
    if(cp->num_entries <= 0) return false;

    entry = cp->entry[0];
    *nx = entry->m.nx;
    *ny = entry->m.ny;

    if( !(*x = (double *)AxesMalloc((Widget)cw, *nx*sizeof(double))) ||
	!(*y = (double *)AxesMalloc((Widget)cw, *ny*sizeof(double))) ||
	!(*z = (float *)AxesMalloc((Widget)cw, *nx* *ny*sizeof(float))) )
    {
	Free(*x); Free(*y); Free(*z);
	return false;
    }
    memcpy(*x, entry->m.x, *nx*sizeof(double));
    memcpy(*y, entry->m.y, *ny*sizeof(double));
    memcpy(*z, entry->m.z, *nx* *ny*sizeof(float));

    return true;
}

bool ConPlotClass::colors(int num_colors, int *r, int *g, int *b,
		float dim_percent)
{
    ConPlotPart *cp = &cw->con_plot;
    AxesPart *ax = &cw->axes;
    int num_colors_needed;
    double red, green, blue;
    int i;

    num_colors_needed = num_colors;

    // if dim_colors are needed, we need twice the number of colors
    if(dim_percent > 0.0) {
	num_colors_needed = num_colors + num_colors;
    }

    if(num_colors_needed <= 0) return false;

    if(XDisplayCells(XtDisplay(cw), DefaultScreen(XtDisplay(cw)))
		< num_colors_needed) return false;

    if(num_colors_needed > cp->num_cells &&
		!GetColorCells(cw, num_colors_needed)) return false;

    cp->num_colors = num_colors;
    cp->dim_percent = dim_percent;

    if(cp->private_cells)
    {
	for(i = 0; i < cp->num_colors; i++) {
	    cp->colors[i].red   = r[i];
	    cp->colors[i].green = g[i];
	    cp->colors[i].blue  = b[i];
	    XStoreColor(XtDisplay(cw), cp->cmap, &cp->colors[i]);
	}

	/* add the dim colors if needed. Note that i is set from the loop
	 * just above
	 */
	if (cp->dim_percent > 0.0)
	{
	    for( ; i < num_colors_needed; i++) {
		// the rgb's still need to be corrected to make them dim
		red = (double) r[i - cp->num_colors];
		green = (double) g[i - cp->num_colors];
		blue = (double) b[i - cp->num_colors];
		rgbBrighten(&red, &green, &blue, (double) dim_percent);
		cp->colors[i].red   = (int) red;
		cp->colors[i].green = (int) green;
		cp->colors[i].blue  = (int) blue;
		XStoreColor(XtDisplay(cw), cp->cmap, &cp->colors[i]);
	    }
	}
    }
    else
    {
	for(i = 0; i < cp->num_colors; i++)
	{
	    cp->colors[i].red   = r[i];
	    cp->colors[i].green = g[i];
	    cp->colors[i].blue  = b[i];

	    if(!XAllocColor(XtDisplay(cw), cp->cmap, &cp->colors[i])) {
		return false;
	    }
	    cp->pixels[i] = cp->colors[i].pixel;
	}
	/* add the dim colors if needed. Note that i is set from the loop
	 * just above
	 */
	if(cp->dim_percent > 0.0)
	{
	    for( ; i < num_colors_needed; i++)
	    {
		// the rgb's still need to be corrected to make them dim
		red = (double) r[i - cp->num_colors];
		green = (double) g[i - cp->num_colors];
		blue = (double) b[i - cp->num_colors];
		rgbBrighten(&red, &green, &blue, (double) dim_percent);
		cp->colors[i].red   = (int) red;
		cp->colors[i].green = (int) green;
		cp->colors[i].blue  = (int) blue;

		if(!XAllocColor(XtDisplay(cw), cp->cmap, &cp->colors[i])) {
		    return false;
		}
		cp->pixels[i] = cp->colors[i].pixel;
	    }
	}
	_ConPlotRedraw(cw);
    }
    if(cp->image != NULL) {
	_ConPlotRedisplayData(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	_ConPlotDrawBar(cw);
    }
    if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
	XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width,  cw->core.height, 0, 0);
    }
    return true;
}

/* It appears that spectrogram only calls this when the normal color allocation
 * fails.  So, if this is called, chances are that dim colors cannot be used.
 */
bool ConPlotClass::setColors(Pixel *pixels, int num_colors)
{
    ConPlotPart *cp = &cw->con_plot;
    int	i, num_colors_needed;

    num_colors_needed = num_colors;

    // if dim_colors are needed, we need twice the number of colors
    if(cp->dim_percent > 0.0) {
	num_colors_needed = num_colors + num_colors;
    }

    if(num_colors < 1) return false;

    if(cp->pixels == NULL) {
	cp->pixels = (Pixel *)malloc(num_colors_needed*sizeof(Pixel));
    }
    else {
	cp->pixels = (Pixel *)realloc(cp->pixels,
					num_colors_needed*sizeof(Pixel));
    }
    Free(cp->colors);
    if( !(cp->colors = (XColor *)AxesMalloc((Widget)cw,
			num_colors_needed*sizeof(XColor))) ) return false;

    Free(cp->plane_masks);
    if( !(cp->plane_masks = (unsigned int *)AxesMalloc((Widget)cw,
		num_colors_needed*sizeof(unsigned int)))) return false;

    if(cp->cmap == (Colormap) NULL)
    {
	Widget toplevel;
	for(toplevel = XtParent(cw); XtParent(toplevel) != NULL;
		toplevel = XtParent(toplevel));

	if((cp->cmap = DefaultColormap(XtDisplay(toplevel),
		DefaultScreen(XtDisplay(toplevel)))) == (Colormap)NULL)
	{
		return false;
	}
    }

    for(i = 0; i < num_colors; i++) {
	cp->colors[i].pixel = cp->pixels[i] = pixels[i];
	cp->colors[i].flags = DoRed | DoGreen | DoBlue;
    }
 
    /* add the dim colors if needed. Note that i is set from the loop
     * just above
     */
    if(cp->dim_percent > 0.0)
    {
	for( ; i < num_colors_needed; i++)
	{
	  cp->colors[i].pixel = cp->pixels[i-num_colors] = pixels[i-num_colors];
	  cp->colors[i].flags = DoRed | DoGreen | DoBlue;
	}
    }

    cp->num_cells = num_colors;
    cp->num_colors = num_colors;
	
    return true;
}

/*
	stipple_width	in bits
	stipple_height	in bits
	all stipples have same width and height
*/
void ConPlotClass::setStipples(int num_stipples, char *stipple_bits,
			int stipple_width, int stipple_height)
{
    ConPlotPart *cp = &cw->con_plot;
    int i, bytes_per_stipple;

    if(num_stipples <= 0) return;

    if(!XtIsRealized((Widget)cw)) {
	_AxesWarn((AxesWidget)cw, "ConPlotStipples called before Realize.");
	return;
    }
    for(i = 0; i < cp->num_stipples; i++)
    {
	XFreePixmap(XtDisplay(cw), cp->stipples[i]);
    }
    Free(cp->stipples);
    if( !(cp->stipples = (Pixmap *)AxesMalloc((Widget)cw,
		num_stipples*sizeof(Pixmap))) ) return;

    cp->num_stipples = num_stipples;
    bytes_per_stipple = stipple_width*stipple_height/8;
    if(8*bytes_per_stipple < stipple_width*stipple_height)
    {
	bytes_per_stipple++;
    }

    for(i = 0; i < num_stipples; i++)
    {
	cp->stipples[i] = XCreateBitmapFromData(XtDisplay(cw), XtWindow(cw),
				stipple_bits + i*bytes_per_stipple,
				stipple_width, stipple_height);
    }
}

void ConPlotClass::colorLines(int num_lines, double *lines)
{
	ConPlotPart *cp = &cw->con_plot;
	AxesPart *ax = &cw->axes;
	int  n;
	ConEntry *entry;
	Drawable drawable =cw->axes.use_pixmap ? cw->axes.pixmap : XtWindow(cw);

	if(num_lines <= 0) return;

	Free(cp->lines);
	if( !(cp->lines = (double *)AxesMalloc((Widget)cw,
		num_lines*sizeof(double))) ) return;

	memcpy(cp->lines, lines, num_lines*sizeof(double));
	cp->num_lines = num_lines;
	if(cp->mode == CONTOURS_ONLY || cp->num_entries <= 0) {
	    return;
	}
	if(ComputeBar(cw) || cp->num_stipples > 0)
	{
	    _AxesRedraw((AxesWidget)cw);
	    _ConPlotRedraw(cw);
	    if(!cw->axes.redisplay_pending)
	    {
		_AxesRedisplayAxes((AxesWidget)cw);
		_ConPlotRedisplayData(cw);
		_AxesRedisplayXor((AxesWidget)cw);
		_ConPlotDrawBar(cw);
		if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
		    XCopyArea(XtDisplay(cw), ax->pixmap,XtWindow(cw),ax->axesGC,
				0, 0, cw->core.width, cw->core.height, 0, 0);
		}
	    }
	}
	else
	{
	    entry = cp->entry[0];
	    if(!cw->axes.redisplay_pending) {
		_AxesRedisplayXor((AxesWidget)cw);
	    }
	    entry->m.x1 = cw->axes.x1[cw->axes.zoom];
	    entry->m.x2 = cw->axes.x2[cw->axes.zoom];
	    entry->m.y1 = cw->axes.y1[cw->axes.zoom];
	    entry->m.y2 = cw->axes.y2[cw->axes.zoom];
	    if(cp->image != NULL)
	    {
		n = (cp->num_colors < cp->num_lines) ?
			cp->num_colors : cp->num_lines-1;
		shadeImage(XtScreen(cw), cp->image, &cw->axes.d,
			cp->plot_background_pixel, entry->select_val,
			entry->dim_val, n, cp->pixels, cp->lines, &entry->m,
			&cp->image_x0, &cp->image_y0, &cp->image_display_width,
			&cp->image_display_height, cp->apply_anti_alias,
                        entry->distinctOutliers, entry->select_color);
		if(!cw->axes.redisplay_pending && XtIsRealized((Widget)cw))
		{
		    XSetClipRectangles(XtDisplay(cw), cw->axes.axesGC, 0,0,
				&ax->clip_rect, 1, Unsorted);
		    XPutImage(XtDisplay(cw), drawable, cw->axes.axesGC,
			cp->image, cp->image_x0, cp->image_y0, cp->image_x0,
			cp->image_y0, cp->image_display_width,
			cp->image_display_height);
		    XSetClipMask(XtDisplay(cw), cw->axes.axesGC, None);
		}
	    }
	    else if(!cw->axes.redisplay_pending)
	    {
		n = (cp->num_stipples > 0) ?  cp->num_stipples : cp->num_colors;
		if(n >= cp->num_lines) n = cp->num_lines-1;
		shade(XtDisplay(cw), drawable, cp->colorGC,
			&cw->axes.d, n, cp->pixels, cp->lines,
			cp->num_stipples, cp->stipples, &entry->m);
	    }
	    if(!cw->axes.redisplay_pending) {
		_AxesRedisplayXor((AxesWidget)cw);
		_ConPlotDrawBar(cw);
		if(ax->use_pixmap && XtIsRealized((Widget)cw)) {
		    XCopyArea(XtDisplay(cw), ax->pixmap,XtWindow(cw),ax->axesGC,
				0, 0, cw->core.width, cw->core.height, 0, 0);
		}
	    }
	}
}

static void
SetClipping(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;

	if( XtIsRealized((Widget)w) ) {
	    XSetClipRectangles(XtDisplay(w), cp->dataGC,
				0, 0, &w->axes.clip_rect, 1, Unsorted);
	    XSetClipRectangles(XtDisplay(w), cp->contour_labelGC,
				0, 0, &w->axes.clip_rect, 1, Unsorted);
	    XSetClipRectangles(XtDisplay(w), cp->colorGC,
				0, 0, &w->axes.clip_rect, 1, Unsorted);
	}
}

/** 
 *  
 */
void
_ConPlotRedraw(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	int i, width, height, depth;
	char *data;
	Display *display;
	
	if(cp->mode != CONTOURS_ONLY)
	{
	    width = w->core.width;
	    height = w->core.height;

	    if(cp->image != NULL &&
		(width != cp->image_width || height != cp->image_height))
	    {
		XDestroyImage(cp->image);

		display = XtDisplay(w);
		depth = DefaultDepth(display, DefaultScreen(display));

		
		if( !(data = (char *)AxesMalloc((Widget)w, width*height*8)))
			 return;

		cp->image = XCreateImage(display,
			DefaultVisual(display, DefaultScreen(display)),
			depth, ZPixmap, 0, data, width, height, 8, 0);
		cp->image_width = width;
		cp->image_height = height;
	    }
	}
	SetClipping(w);
	for(i = 0; i < cp->num_entries; i++)
		if(cp->entry[i]->display_data)
	{
		DrawEntry(w, cp->entry[i]);
	}
}

static void
DrawEntry(ConPlotWidget w, ConEntry *entry)
{
	ConPlotPart *cp = &w->con_plot;
	int i, n;
	LabelArray *l;

	if(entry->m.nx <= 0 || entry->m.ny <= 0)
	{
	    return;
	}
	
	Free(entry->s.segs);
	Free(entry->l.ls);
	entry->s.size_segs = 0;
	entry->s.nsegs = 0;
	entry->l.size_ls = 0;
	entry->l.n_ls = 0;
	Free(entry->t.segs);
	entry->t.size_segs = 0;
	entry->t.nsegs = 0;
	if(cp->mode != COLOR_ONLY)
	{
	    w->axes.d.s = &entry->s;
	    w->axes.d.l = &entry->l;
	    entry->m.x1 = w->axes.x1[w->axes.zoom];
	    entry->m.x2 = w->axes.x2[w->axes.zoom];
	    entry->m.y1 = w->axes.y1[w->axes.zoom];
	    entry->m.y2 = w->axes.y2[w->axes.zoom];
	    condata(&w->axes.d, (float)cp->contour_label_size, &entry->m,
		    entry->auto_ci, &entry->ci, entry->c_min, entry->c_max);
	    w->axes.d.s = &entry->t;
	    l = &entry->l;
	    for(i = 0; i < l->n_ls; i++)
	    {
		mapalf(&w->axes.d, l->ls[i].x, l->ls[i].y, l->ls[i].size,
			l->ls[i].angle, 0, l->ls[i].label);
	    }
	}
	if(cp->mode != CONTOURS_ONLY && cp->image != NULL)
	{
	    entry->m.x1 = w->axes.x1[w->axes.zoom];
	    entry->m.x2 = w->axes.x2[w->axes.zoom];
	    entry->m.y1 = w->axes.y1[w->axes.zoom];
	    entry->m.y2 = w->axes.y2[w->axes.zoom];
	    n = (cp->num_colors < cp->num_lines) ?
			cp->num_colors : cp->num_lines-1;
	    if(n > 0) {
		shadeImage(XtScreen(w), cp->image, &w->axes.d,
		    cp->plot_background_pixel, entry->select_val,entry->dim_val,
		    n, cp->pixels, cp->lines, &entry->m, &cp->image_x0,
		    &cp->image_y0, &cp->image_display_width,
		    &cp->image_display_height, cp->apply_anti_alias,
		    entry->distinctOutliers, entry->select_color);
	    }
	}
}

Widget
ConPlotCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
	return (XtCreateWidget(name, conPlotWidgetClass, parent, 
		arglist, argcount));
}

static void
LabelCrosshair(AxesWidget widget, XEvent *event, AxesCursor *c)
{
	ConPlotWidget w = (ConPlotWidget)widget;
	ConPlotPart *cp = &w->con_plot;
	int	i, j, k, ndeci;
	double	x, y, value, d, dist;
	ConEntry *entry, *e;

	if(cp->num_entries <= 0) return;
	if(c->type == AXES_CROSSHAIR)
	{
		e = cp->entry[0];
		value = 0.;
		dist = 1.e+40;
		for(k = 0; k < cp->num_entries; k++)
		{
			entry = cp->entry[k];

			for(i = 0; i < entry->m.nx; i++)
				for(j = 0; j < entry->m.ny; j++)
			{
				x = c->scaled_x - entry->m.x[i];
				y = c->scaled_y - entry->m.y[j];
				d = x*x + y*y;
				if(d < dist)
				{
					dist = d;
					value = entry->m.z[i+j*entry->m.nx];
					e = entry;
				}
			}
		}
		if(e->m.z_max != e->m.z_min)
		{
			x = log10(fabs(e->m.z_max - e->m.z_min));
			ndeci = (int)((x-4 > 0) ? 0 : -x+4);
		}
		else
		{
			ndeci = 2;
		}
		ftoa(value, ndeci, 0, c->label, 50);
	}
}

static Boolean 
GetColorCells(ConPlotWidget w, int num_colors)
{
	ConPlotPart *cp = &w->con_plot;
	int i, need;
	
	if(num_colors > XDisplayCells(XtDisplay(w),DefaultScreen(XtDisplay(w))))
	{
		return False;
	}
	need = num_colors - cp->num_cells;
	if(need <= 0)
	{
		return True;
	}
	if(cp->pixels == NULL)
	{
		cp->pixels = (Pixel *)malloc(num_colors*sizeof(Pixel));
	}
	else
	{
		cp->pixels = (Pixel *)realloc(cp->pixels,
						num_colors*sizeof(Pixel));
	}
	Free(cp->colors);
	if( !(cp->colors = (XColor *)AxesMalloc((Widget)w,
			num_colors*sizeof(XColor))) ) return False;

	Free(cp->plane_masks);
	if( !(cp->plane_masks = (unsigned int *)AxesMalloc((Widget)w,
		num_colors*sizeof(unsigned int))) ) return False;

	if(cp->cmap == (Colormap) NULL)
	{
	    Widget toplevel;
	    for(toplevel = XtParent(w); XtParent(toplevel) != NULL;
		toplevel = XtParent(toplevel));

	    if((cp->cmap = DefaultColormap(XtDisplay(toplevel),
		DefaultScreen(XtDisplay(toplevel)))) == (Colormap)NULL)
	    {
		return False;
	    }
	}

	if(XAllocColorCells(XtDisplay(w), cp->cmap, False, 
		(unsigned long *)cp->plane_masks, 0,
		cp->pixels+cp->num_cells, need) != 0)
	{
	    for(i = 0; i < need; i++)
	    {
		cp->colors[cp->num_cells+i].pixel = cp->pixels[i];
		cp->colors[cp->num_cells+i].flags = DoRed | DoGreen | DoBlue;
	    }
	    cp->num_cells = num_colors;
	    cp->private_cells = True;
	}
	else
	{
	    cp->private_cells = False;
	}
	return True;
}

int ConPlotClass::getPixels(Pixel **p_pixels)
{
    ConPlotPart *cp = &cw->con_plot;
    int num_pixels;
    Pixel *pixels = NULL;

    if(cp->num_colors > 0) {
	pixels = (Pixel *)AxesMalloc((Widget)cw,
				cp->num_colors*sizeof(Pixel));
	memcpy(pixels, cp->pixels, cp->num_colors*sizeof(Pixel));
	num_pixels = cp->num_colors;
    }
    else {
	num_pixels = 0;
    }
    *p_pixels = pixels;
    return num_pixels;
}

int ConPlotClass::getStipples(Pixmap **p_stipples)
{
    ConPlotPart *cp = &cw->con_plot;
    int num_stipples;
    Pixmap *stipples = NULL;

    if(cp->num_stipples > 0)
    {
	stipples = (Pixmap *)AxesMalloc((Widget)cw,
				cp->num_stipples*sizeof(Pixmap));
	memcpy(stipples, cp->stipples, cp->num_stipples*sizeof(Pixmap));
	num_stipples = cp->num_stipples;
    }
    else {
	num_stipples = 0;
    }
    *p_stipples = stipples;
    return num_stipples;
}

static Boolean
ComputeBar(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	int i, new_bar_width, max_width, n;
	int ascent, descent, direction;
	XCharStruct overall;
	char lab[20], last_lab[20];
	double range, dif;

	n = (cp->num_stipples > 0) ?  cp->num_stipples : cp->num_colors;

	if(n >= cp->num_lines) n = cp->num_lines-1;

	if(n <= 0) {
	    return(False);
	}
	dif = cp->lines[n] - cp->lines[0];

	if(dif == 0.) {
	    return(False);
	}
	range = log10(fabs(dif));
	cp->ndeci = (int)((range-2 > 0) ? 0 : -range+2);
	if(cp->num_bar_labels == 0)
	{
	    last_lab[0] = '\0';
	    for(;;)
	    {
		for(i = max_width = 0; i <= n; i++)
		{
		    ftoa(cp->lines[i], cp->ndeci, 1, lab, 20);
		    if(!strcmp(lab, last_lab)) {
			cp->ndeci++;
			last_lab[0] = '\0';
			break;
		    }
		    stringcpy(last_lab, lab, sizeof(last_lab));
		    XTextExtents(w->axes.font, lab, (int)strlen(lab),
				&direction, &ascent, &descent, &overall);
		    if(max_width < overall.width) {
			max_width = overall.width;
		    }
		}
		if(i > n || cp->ndeci >= 10) break;
	    }
	}
	else
	{
	    max_width = 0;
	    for(i = 0; i < cp->num_bar_labels; i++) {
		XTextExtents(w->axes.font, cp->bar_labels[i],
			(int)strlen(cp->bar_labels[i]), &direction, &ascent,
			&descent, &overall);
		if(max_width < overall.width) {
		    max_width = overall.width;
		}
	    }
	}

	getBarTitleDimensions(w);

//	new_bar_width = BAR_WIDTH + 8 + max_width;
	new_bar_width = BAR_WIDTH + 4 + max_width;
	if(new_bar_width > (int)w->core.width) {
	    return(False);
	}
	if(new_bar_width > cp->bar_width || cp->bar_width - new_bar_width > 10)
	{
	    cp->bar_width = new_bar_width;
	    w->axes.right_margin = cp->bar_width + 2 + cp->bar_title_h;
	    return(True);
	}
	else
	{
	    return(False);
	}
}

static void
getBarTitleDimensions(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	AxesPart *ax = &w->axes;
	int ascent, descent, direction;
	XCharStruct overall;
	int iy1, iy2, y_mid;

	cp->bar_title_image = NULL;
	if( !XtIsRealized((Widget)w) ) return;

	if(cp->bar_title)
	{
	    iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
	    iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);

	    XTextExtents((XFontStruct*)ax->a.font, cp->bar_title,
			(int)strlen(cp->bar_title), &direction, &ascent,
			&descent, &overall);
	    if( overall.width < abs(iy2-iy1) || !cp->short_bar_title )
	    {
		y_mid =unscale_y(&ax->d,.5*(ax->y1[ax->zoom]+ax->y2[ax->zoom]));
		cp->bar_title_x = unscale_x(&ax->d, ax->x2[ax->zoom]) + 4;
		cp->bar_title_y = (int)(y_mid - .5*overall.width);
//		cp->bar_title_h = overall.descent + overall.ascent;
		cp->bar_title_h = ascent + descent;
		cp->bar_title_w = overall.width;
		cp->bar_title_image = cp->long_bar_title_image;
	    }
	}
	if( !cp->bar_title_image && cp->short_bar_title )
	{
	    XTextExtents((XFontStruct*)ax->a.font, cp->short_bar_title,
			(int)strlen(cp->short_bar_title), &direction, &ascent,
			&descent, &overall);
	    y_mid = unscale_y(&ax->d,.5*(ax->y1[ax->zoom]+ax->y2[ax->zoom]));
	    cp->bar_title_x = unscale_x(&ax->d, ax->x2[ax->zoom]) + 4;
	    cp->bar_title_y = (int)(y_mid - .5*overall.width);
	    cp->bar_title_h = ascent + descent;
	    cp->bar_title_w = overall.width;
	    cp->bar_title_image = cp->short_bar_title_image;
	}
}

void ConPlotClass::setBarValues(int num_bar_labels, double *bar_values,
			char **bar_labels, bool redraw)
{
    ConPlotPart *cp = &cw->con_plot;
    int i;

    Free(cp->bar_values);
    for(i = 0; i < cp->num_bar_labels; i++) {
	Free(cp->bar_labels[i]);
    }
    Free(cp->bar_labels);
    cp->num_bar_labels = 0;

    if(num_bar_labels > 0)
    {
	cp->num_bar_labels = num_bar_labels;
	cp->bar_values = (double *)malloc(num_bar_labels*sizeof(double));
	memcpy(cp->bar_values, bar_values, num_bar_labels*sizeof(double));
	cp->bar_labels = (char **)malloc(num_bar_labels*sizeof(char *));
	for(i = 0; i < num_bar_labels; i++) {
	    cp->bar_labels[i] = strdup(bar_labels[i]);
	}
	if(redraw && XtIsRealized((Widget)cw))
	{
	    _AxesRedraw((AxesWidget)cw);
	    _ConPlotRedraw(cw);
	    if(!cw->axes.redisplay_pending) {
		_AxesRedisplayAxes((AxesWidget)cw);
		_ConPlotRedisplayData(cw);
		_AxesRedisplayXor((AxesWidget)cw);
		_ConPlotDrawBar(cw);
	    }
	}
    }
}

/** 
 *  
 */
void
_ConPlotDrawBar(ConPlotWidget w)
{
	ConPlotPart *cp = &w->con_plot;
	AxesPart *ax = &w->axes;
	int i, n, x, y, idif, ix2, iy1, iy2, last_y, incr;
	int ascent, descent, direction, margin;
	XCharStruct overall;
	char lab[20];
	float dely;
	double del;
	Drawable drawable = w->axes.use_pixmap ? w->axes.pixmap : XtWindow(w);

	if( !XtIsRealized((Widget)w) || cp->mode == CONTOURS_ONLY )
	{
		return;
	}
	n = (cp->num_stipples > 0) ?
		cp->num_stipples : cp->num_colors;
	if(n >= cp->num_lines) n = cp->num_lines-1;

	if(cp->mode == CONTOURS_ONLY || n <= 0 ||
		cp->num_lines <= 0)
	{
		return;
	}
	XTextExtents(w->axes.font, "0123456789", 10, &direction,
				&ascent, &descent, &overall);
	margin = overall.descent + overall.ascent;

	iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
	iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if(iy1 < iy2) {
	    i = iy1;
	    iy1 = iy2;
	    iy2 = i;
	}

	idif = iy1 - iy2;
	if(idif <= 0)
	{
		return;
	}
	ix2 = unscale_x(&w->axes.d, w->axes.x2[w->axes.zoom]);

	if(ax->display_axes_labels == AXES_XY ||
		ax->display_axes_labels == AXES_X)
	{
	    x = ix2 + (int)(.5*ax->a.maxcx);
	}
	else {
	    x = ix2 + 4;
	}

	if(cp->bar_title) {
	    getBarTitleDimensions(w);

	    if(x < cp->bar_title_x + cp->bar_title_h + 4) {
		x = cp->bar_title_x + cp->bar_title_h + 4;
	    }
	    cp->bar_x = x;
	    _AxesClearArea((AxesWidget)w, cp->bar_title_x, w->axes.d.iy2,
			cp->bar_title_w + cp->bar_width + 4,
			w->axes.d.iy1-w->axes.d.iy2+1);
	}
	else {
	    cp->bar_x = x;
	    _AxesClearArea((AxesWidget)w, x, w->axes.d.iy2, cp->bar_width,
			w->axes.d.iy1-w->axes.d.iy2+1);
	}

	if(cp->bar_title_image) {
	    XPutImage(XtDisplay(w), drawable, ax->axesGC,
		cp->bar_title_image, 0, 0, cp->bar_title_x,
		cp->bar_title_y, cp->bar_title_h, cp->bar_title_w);
	}

	dely = (float)idif/n;
	incr = 1;
	if(dely < 3*margin)
	{
	    int m;
	    m = (int)((float)idif/(3.*margin) + 1);
	    incr = n/m;
	    if(incr*m < n) incr++;
	    if(incr < 0) incr = 1;
	}

	last_y = iy1;
	if(cp->num_stipples > 0)
	{
		XSetFillStyle(XtDisplay(w), cp->barGC, FillStippled);
	}
	else
	{
		XSetFillStyle(XtDisplay(w), cp->barGC, FillSolid);
	}

	for(i = 0; i < n; i++)
	{
	    if(cp->num_stipples > 0) {
		XSetStipple(XtDisplay(w), cp->barGC, cp->stipples[i]);
	    }
	    else
	    {
		XSetForeground(XtDisplay(w), cp->barGC, cp->pixels[i]);
	    }
	    y = (int)(iy1 - (i+1)*dely);
	    XFillRectangle(XtDisplay(w), drawable, cp->barGC,
				x, y, BAR_WIDTH, last_y - y);
	    last_y = y;
	}
	XSetForeground(XtDisplay(w), cp->barGC, w->axes.fg);
	x += BAR_WIDTH + 4;

	if(cp->num_bar_labels == 0)
	{
	    for(i = 0; i <= n-incr; i += incr)
	    {
		y = (int)(iy1 - i*dely);
		ftoa(cp->lines[i], cp->ndeci, 1, lab, 20);
		XTextExtents(w->axes.font, lab, (int)strlen(lab), &direction,
				&ascent, &descent, &overall);
		XDrawString(XtDisplay(w), drawable, cp->barGC,
			x, y + overall.ascent/2, lab, (int)strlen(lab));
	    }
	    y = (int)(iy1 - n*dely);
	    ftoa(cp->lines[n], cp->ndeci, 1, lab, 20);
	    XTextExtents(w->axes.font, lab, (int)strlen(lab), &direction,
				&ascent, &descent, &overall);
	    XDrawString(XtDisplay(w), drawable, cp->barGC,
			x, y + overall.ascent/2, lab, (int)strlen(lab));
	}
	else
	{
	    del = cp->lines[n]- cp->lines[0];
	    for(i = 0; i < cp->num_bar_labels; i++) {
		XTextExtents(w->axes.font, cp->bar_labels[i],
			(int)strlen(cp->bar_labels[i]), &direction, &ascent,
			&descent, &overall);
		if((cp->num_bar_labels==2 || cp->num_bar_labels==3) && i==0) {
		    y = iy1 - overall.descent;
		}
		else if((cp->num_bar_labels==2 && i == 1) || 
			(cp->num_bar_labels==3 && i == 2))
		{
		    y = iy2 + overall.ascent;
		}
		else {
		    y = (int)(iy1 - (cp->bar_values[i]-cp->lines[0])*idif/del);
		    y += overall.ascent/2;
		}
		XDrawString(XtDisplay(w), drawable, cp->barGC, x, y,
			cp->bar_labels[i], (int)strlen(cp->bar_labels[i]));
	    }
	}
}

static void
ConPlotHardCopy(AxesWidget widget, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, PrintParam *p)
{
	ConPlotWidget w = (ConPlotWidget)widget;
	ConPlotPart *cp = &w->con_plot;
	AxesPart *ax = &w->axes;
	int i, j, m, n, xmin, xmax, ymin, ymax;
	float fac;
	double x1, x2, y1, y2;
	ConEntry *entry;
	SegArray segs;
	LabelArray l;

	xmin = unscale_x(d, w->axes.x1[w->axes.zoom]) + 1;
	xmax = unscale_x(d, w->axes.x2[w->axes.zoom]) - 1;
	ymin = unscale_y(d, w->axes.y1[w->axes.zoom]) + 1;
	ymax = unscale_y(d, w->axes.y2[w->axes.zoom]) - 1;

	d->s = &segs;
	d->s->segs = NULL;
	d->s->size_segs = 0;
	d->s->nsegs = 0;
	d->l = &l;
	d->l->ls = NULL;
	d->l->size_ls = 0;
	d->l->n_ls = 0;
	fac = (float)(d->ix2 - d->ix1)/600.;

	for(m = 0; m < cp->num_entries; m++)
		if(cp->entry[m]->display_data)
	{
	    entry = cp->entry[m];
	    if(cp->mode == COLOR_ONLY)
	    {
		XColor color_info;
		for(i = 0; i < cp->num_colors; i++) {
		    color_info.pixel = cp->pixels[i];
		    XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color_info);

		    fprintf(fp, "/c%d { %f %f %f setrgbcolor } def\n", i,
			(float)color_info.red/65535.,
			(float)color_info.green/65535.,
			(float)color_info.blue/65535.);
		}
		color_info.pixel = cp->plot_background_pixel;
		XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color_info);

		fprintf(fp,"/c%d { %f %f %f setrgbcolor } def\n",cp->num_colors,
			(float)color_info.red/65535.,
			(float)color_info.green/65535.,
			(float)color_info.blue/65535.);

		fprintf(fp,
	    "/F { moveto lineto lineto lineto closepath fill newpath } def\n");

		fprintf(fp, "\nnewpath\n");
		n = (cp->num_stipples > 0) ?  cp->num_stipples : cp->num_colors;
		if(n >= cp->num_lines) n = cp->num_lines-1;

		fprintf(fp, "N %d %d M\n", unscale_x(d, ax->x1[ax->zoom]),
		    unscale_y(d, ax->y2[ax->zoom]));
		fprintf(fp, "%d %d d\n", unscale_x(d, ax->x2[ax->zoom]),
		    unscale_y(d, ax->y2[ax->zoom]));
		fprintf(fp, "%d %d d\n", unscale_x(d, ax->x2[ax->zoom]),
		    unscale_y(d, ax->y1[ax->zoom]));
		fprintf(fp, "%d %d d\n", unscale_x(d, ax->x1[ax->zoom]),
		    unscale_y(d, ax->y1[ax->zoom]));
		fprintf(fp, "%d %d d\n", unscale_x(d, ax->x1[ax->zoom]),
		    unscale_y(d, ax->y2[ax->zoom]));
		fprintf(fp, "closepath clip\n");

		fprintf(fp, "gsave clip ");
		fprintf(fp, "1 setgray\n");
		fprintf(fp, "fill grestore 1 setgray stroke\n");

		shadePS(fp, d, n, cp->lines, &entry->m, xmin, xmax, ymin, ymax);
		fprintf(fp, "initclip\n");

		HardDrawBar((ConPlotWidget)widget, fp, d, a, font_scale,
				&p->fonts);

		fprintf(fp, "0 setgray\n");
	    }
	    else
	    {
		fprintf(fp, "%d slw\n", p->data_linewidth);

		entry = cp->entry[m];
		d->s->nsegs = 0;
		d->l->n_ls = 0;
		entry->m.x1 = w->axes.x1[w->axes.zoom];
		entry->m.x2 = w->axes.x2[w->axes.zoom];
		entry->m.y1 = w->axes.y1[w->axes.zoom];
		entry->m.y2 = w->axes.y2[w->axes.zoom];
		if(entry->display_grid)
		{
		    for(i = 0; i < entry->m.nx; i++)
			if(entry->m.x[i] >= w->axes.x1[w->axes.zoom] &&
			   entry->m.x[i] <= w->axes.x2[w->axes.zoom])
		    {
			for(j = 0; j < entry->m.ny; j++)
			{
			    if( entry->m.y[j] >= w->axes.y1[w->axes.zoom] &&
				entry->m.y[j] <= w->axes.y2[w->axes.zoom])
			    {
				x1 = entry->m.x[i] - 2*w->axes.d.scalex;
				x2 = entry->m.x[i] + 2*w->axes.d.scalex;
				y1 = entry->m.y[j] - 2*w->axes.d.scaley;
				y2 = entry->m.y[j] + 2*w->axes.d.scaley;
				imove(d, x1, .5*(y1+y2));
				idraw(d, x2, .5*(y1+y2));
				imove(d, .5*(x1+x2), y1);
				idraw(d, .5*(x1+x2), y2);
				iflush(d);
			    }
			}
		    }
		}
		if(p->color) ax->axes_class->postColor(fp, w->axes.fg);
		putSegments(fp, d->s);

		Free(d->s->segs);
		d->s->size_segs = 0;
		d->s->nsegs = 0;

		condata(d, (float)(fac*cp->contour_label_size),
			&entry->m, entry->auto_ci, &entry->ci, entry->c_min,
			entry->c_max);

		if(p->color) ax->axes_class->postColor(fp, cp->data_fg);
		putSegments(fp, d->s);

		Free(d->s->segs);
		d->s->size_segs = 0;
		d->s->nsegs = 0;

		for(i = 0; i < d->l->n_ls; i++)
		{
		    mapalf(d, d->l->ls[i].x, d->l->ls[i].y, d->l->ls[i].size,
				d->l->ls[i].angle, 1, d->l->ls[i].label);
		}

		if(p->color) {
		    ax->axes_class->postColor(fp, cp->contour_label_fg);
		}
		putSegments(fp, d->s);

		Free(d->s->segs);
		d->s->size_segs = 0;
		d->s->nsegs = 0;

		for(i = 0; i < d->l->n_ls; i++)
		if(cp->mark_max)
		{
		    x1 = entry->xmax - 3*w->axes.d.scalex;
		    x2 = entry->xmax + 3*w->axes.d.scalex;
		    y1 = entry->ymax - 3*w->axes.d.scaley;
		    y2 = entry->ymax + 3*w->axes.d.scaley;
		    imove(d, x1, y1);
		    idraw(d, x1, y2);
		    idraw(d, x2, y2);
		    idraw(d, x2, y1);
		    idraw(d, x1, y1);
		    iflush(d);
		}
	  	if(p->color) ax->axes_class->postColor(fp, w->axes.fg);
		putSegments(fp, d->s);

		Free(d->s->segs);
		Free(d->l->ls);
	    }
	}
}

static void
putSegments(FILE *fp, SegArray *s)
{
	int i;

	if(s->nsegs > 0)
	{
	    fprintf(fp, "%d %d M\n", s->segs[0].x1, s->segs[0].y1);
	    fprintf(fp, "%d %d D\n", s->segs[0].x2, s->segs[0].y2);
	}
	for(i = 1; i < s->nsegs; i++)
	{
	    if( s->segs[i].x1 != s->segs[i-1].x2 ||
		s->segs[i].y1 != s->segs[i-1].y2)
	    {
		fprintf(fp, "%d %d M\n", s->segs[i].x1, s->segs[i].y1);
	    }
	    fprintf(fp, "%d %d D\n", s->segs[i].x2, s->segs[i].y2);
	}
}

static void
HardDrawBar(ConPlotWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, AxesFontStruct *fonts)
{
	ConPlotPart *cp = &w->con_plot;
	int i, n, x, y, idif, iy1, iy2, last_y, margin, bar_width, incr;
	char lab[200], *bar_title;
	float dely;

	if(cp->mode == CONTOURS_ONLY) {
		return;
	}
	n = (cp->num_stipples > 0) ? cp->num_stipples : cp->num_colors;
	if(n >= cp->num_lines) n = cp->num_lines-1;

	if(cp->mode == CONTOURS_ONLY || n <= 0 || cp->num_lines <= 0) {
		return;
	}
	margin = a->axis_font_height;
	iy1 = unscale_y(d, w->axes.y1[w->axes.zoom]);
	iy2 = unscale_y(d, w->axes.y2[w->axes.zoom]);
	if(iy1 > iy2) {
	    i = iy1;
	    iy1 = iy2;
	    iy2 = i;
	}
	idif = iy2 - iy1;
	if(idif <= 0) return;

	x = unscale_x(d, w->axes.x2[w->axes.zoom]) + (int)(.05*DOTS_PER_INCH);

	if(cp->bar_title_image)
	{
	    if(cp->bar_title_image == cp->long_bar_title_image) {
		bar_title = _Axes_check_parentheses(cp->bar_title);
	    }
	    else {
		bar_title = _Axes_check_parentheses(cp->short_bar_title);
	    }
	    fprintf(fp, "/%s findfont %d scalefont setfont\n",
		fonts->axis_font, fonts->axis_fontsize);
	    fprintf(fp, "0 setgray\n");

	    y = (iy1 + iy2)/2;
	    fprintf(fp, "%d %d M\n", x + a->axis_font_height, y);
	    fprintf(fp, "%.2f r\n", 90.);
	    fprintf(fp, "(%s) %.5f PC\n", bar_title, font_scale);
	    Free(bar_title);
	    fprintf(fp, "%.2f r\n", -90.);

	    x += (int)(a->axis_font_height + .05*DOTS_PER_INCH);
	}

	bar_width = (int)(.2*DOTS_PER_INCH);

	dely = (float)idif/n;
	incr = 1;
	if(dely < 3*margin)
	{
	    int m;
	    m = (int)((float)idif/(3.*margin) + 1);
	    incr = n/m;
	    if(incr*m < n) incr++;
	    if(incr < 0) incr = 1;
	}

	last_y = iy1;

	for(i = 0; i < n; i++)
	{
	    fprintf(fp, "c%d ", i);

	    y = (int)(iy1 + (i+1)*dely);
	    fprintf(fp, "%d %d %d %d rectfill\n", x, y, bar_width, last_y - y);
	    last_y = y;
	}
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		fonts->axis_font, fonts->axis_fontsize);
	fprintf(fp, "0 setgray\n");

	x += (int)(bar_width + .05*DOTS_PER_INCH);

	if(cp->num_bar_labels == 0)
	{
	    for(i = 0; i <= n-incr; i += incr)
	    {
		y = (int)(iy1 + i*dely - .5*a->axis_font_height);
		fprintf(fp, "N %d %d M\n", x, y);

		ftoa(cp->lines[i], cp->ndeci, 1, lab, 200);
		fprintf(fp, "(%s) %.5f P\n", lab, font_scale);
	    }
	    y = (int)(iy1 + n*dely - .5*a->axis_font_height);
	    fprintf(fp, "N %d %d M\n", x, y);

	    ftoa(cp->lines[n], cp->ndeci, 1, lab, 200);
	    fprintf(fp, "(%s) %.5f P\n", lab, font_scale);
	}
	else
	{
	    double del = cp->lines[n]- cp->lines[0];
	    for(i = 0; i < cp->num_bar_labels; i++)
	    {
		if(cp->num_bar_labels == 2 && i == 0) {
		    y = iy1;
		}
		else if(cp->num_bar_labels == 2 && i == 1) {
		    y = iy2 - a->axis_font_height;
		}
		else {
		    y = (int)(iy1 + (cp->bar_values[i]-cp->lines[0])*idif/del);
		    y -= a->axis_font_height/2;
		}
		fprintf(fp, "N %d %d M\n", x, y);
		fprintf(fp, "(%s) %.5f P\n", cp->bar_labels[i], font_scale);
            }
	}
}

static void
Motion(ConPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	ConPlotPart *cp = &w->con_plot;
	AxesPart *ax = &w->axes;
	int i, j, cursor_x, cursor_y, n;
	char label[200];
	char tlab[100];
	double x, y;
	ConEntry *entry = NULL;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(XtHasCallbacks((Widget)w, XtNselectBarCallback) != XtCallbackHasNone
	    && inside_bar(w, cursor_x, cursor_y))
	{
	    _AxesSetCursor((AxesWidget)w, "hand");

	    if(ax->cursor_info != NULL && cp->num_lines > 1)
	    {
		x = (double)(w->axes.d.iy1 - cursor_y)/
			(double)(w->axes.d.iy1-w->axes.d.iy2);
		n = cp->num_lines - 1;
		y = cp->lines[0] + x*(cp->lines[n] - cp->lines[0]);

		snprintf(label, 200, "%.2f", y);
		InfoSetText(ax->cursor_info, label);
	    }
	    return;
	}

	_AxesMotion((AxesWidget)w, event, params, num_params);

	if(cp->num_entries <= 0) return;

	entry = cp->entry[0];
	if(entry->m.nx <= 0 || entry->m.ny <= 0) return;
	x = scale_x(&ax->d, cursor_x);
	y = scale_y(&ax->d, cursor_y);

#ifdef HAVE_GSL
	i = gsl_interp_bsearch(entry->m.x, x, 0, entry->m.nx-1);
	j = gsl_interp_bsearch(entry->m.y, y, 0, entry->m.ny-1);
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return;
#endif

	if(XtHasCallbacks((Widget)w, XtNmouseOverCallback) != XtCallbackHasNone
		&& ax->cursor_mode != AXES_SELECT_CURSOR)
	{
	    ConPlotMotionCallbackStruct c;
	    c.x = entry->m.x[i];
	    c.y = entry->m.y[j];
	    c.z = entry->m.z[i+j*entry->m.nx];
	    c.i = i;
	    c.j = j;
	    c.pos = i+j*entry->m.nx;
	    c.selectable = False;
	    XtCallCallbacks((Widget)w, XtNmouseOverCallback, &c);
	    if(c.selectable) {
		_AxesSetCursor((AxesWidget)w, "hand");
	    }
	    else {
		_AxesSetCursor((AxesWidget)w, "default");
	    }
	}

	if(ax->cursor_info != NULL)
	{
	    if (entry->m.x[i] > 86400)
	    {
		timeEpochToString(entry->m.x[i], tlab, 100,YMONDHMS2 );
	    }
	    else
	    {
		snprintf(tlab, 100, "%.4f", entry->m.x[i]);	
	    }
	    if( !ax->a.log_y ) {
		y = entry->m.y[j];
	    }
	    else {
		y = pow(10., entry->m.y[j]);
	    }

	    if(entry->m.z[i+j*entry->m.nx] != entry->m.exc)
	    {
		snprintf(label, 200, "%s  %s  %.4f  %.3f", entry->label,
			tlab, y, entry->m.z[i+j*entry->m.nx]);
	    }
	    else
	    {
		snprintf(label, 200, "%s  %s  %.4f  NA", entry->label, tlab, y);
	    }
	    InfoSetText(ax->cursor_info, label);
	}
}

bool ConPlotClass::privateCells()
{
    ConPlotPart *cp = &cw->con_plot;
    return cp->private_cells;
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

static void
ConPlotBtn1Motion(ConPlotWidget w, XEvent *event, String *param, Cardinal *num)
{
	ConPlotPart *cp = &w->con_plot;
	Cardinal n = 1;

	if( cp->ctrl_select_reason == -1 ) {
	    Cardinal n = 0;
	    ConPlotMouseDragSelect(w, event, NULL, &n);
	}
	else if( cp->ctrl_select_reason == CONPLOT_DOWN_CTRL1 ) {
	    const char * p[1] = {"ctrl1"};
	    ConPlotMouseDragSelect(w, event, p, &n);
	}
	else if( cp->ctrl_select_reason == CONPLOT_DOWN_SHIFT1 ) {
	    const char * p[1] = {"shift1"};
	    ConPlotMouseDragSelect(w, event, p, &n);
	}
}

static void
ConPlotBtn2Motion(ConPlotWidget w, XEvent *event, const char **param,
		Cardinal *num)
{
	ConPlotPart *cp = (w != NULL) ? &w->con_plot : NULL;
	Cardinal n = 1;

	if( cp->ctrl_select_reason == -1 ) {
	    _AxesZoom((AxesWidget)w, event, param, num);
	}
	else if( cp->ctrl_select_reason == CONPLOT_DOWN_CTRL2 ) {
	    const char * p[1] = {"ctrl2"};
	    ConPlotMouseDragSelect(w, event, p, &n);
	}
	else if( cp->ctrl_select_reason == CONPLOT_DOWN_SHIFT2 ) {
	    const char * p[1] = {"shift2"};
	    ConPlotMouseDragSelect(w, event, p, &n);
	}
}

static void
ConPlotBtn3Motion(ConPlotWidget w, XEvent *event, const char **param,
		Cardinal *num)
{
	ConPlotPart *cp = (w != NULL) ? &w->con_plot : NULL;
	Cardinal n = 1;

	if( cp->ctrl_select_reason == -1 ) {
	    const char * p[1] = {"horizontal"};
	    _AxesZoom((AxesWidget)w, event, p, &n);
	}
	else if( cp->ctrl_select_reason == CONPLOT_DOWN_CTRL3 ) {
	    const char * p[1] = {"ctrl3"};
	    ConPlotMouseDragSelect(w, event, p, &n);
	}
	else if( cp->ctrl_select_reason == CONPLOT_DOWN_SHIFT3 ) {
	    const char *p[1] = {"shift3"};
	    ConPlotMouseDragSelect(w, event, p, &n);
	}
}

/** 
 *  
 */
static void
ConPlotMouseDownSelect(ConPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	ConPlotPart *cp = (w != NULL) ? &w->con_plot : NULL;
	AxesPart *ax = (w != NULL) ? &w->axes : NULL;
	int cursor_x, cursor_y;
	ConPlotSelectCallbackStruct c;

	int i, j;
	double x, y;
	ConEntry *entry = NULL;

	cp->ctrl_select_reason = -1;

	if(ax->cursor_mode == AXES_SELECT_CURSOR ||
		ax->cursor_mode == AXES_SELECT_CURSOR) {
	    _AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	    return;
	}

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(inside_bar(w, cursor_x, cursor_y)) 
	{
		XtCallCallbacks((Widget)w, XtNselectBarCallback, NULL);
		return;
	}
	if(outside(w, cursor_x, cursor_y)) return;

	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;

	if(cp->num_entries <= 0) return;

	entry = cp->entry[0];
	x = scale_x(&ax->d, cursor_x);
	y = scale_y(&ax->d, cursor_y);

#ifdef HAVE_GSL
	i = gsl_interp_bsearch(entry->m.x, x, 0, entry->m.nx-1);
	j = gsl_interp_bsearch(entry->m.y, y, 0, entry->m.ny-1);
	c.entry_id = entry->id;
	c.x = entry->m.x[i];
	c.y = entry->m.y[j];
	c.z = entry->m.z[i+j*entry->m.nx];
	c.i = i;
	c.j = j;
	c.pos = i+j*entry->m.nx;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return;
#endif
	c.reason = CONPLOT_DOWN;

	if(*num_params > 0)
	{
	    if(!strcmp(params[0], "ctrl1")) {
		c.reason = CONPLOT_DOWN_CTRL1;
	    }
	    else if(!strcmp(params[0], "ctrl2")) {
		c.reason = CONPLOT_DOWN_CTRL2;
	    }
	    else if(!strcmp(params[0], "ctrl3")) {
		c.reason = CONPLOT_DOWN_CTRL3;
	    }
	    else if(!strcmp(params[0], "shift1")) {
		c.reason = CONPLOT_DOWN_SHIFT1;
	    }
	    else if(!strcmp(params[0], "shift2")) {
		c.reason = CONPLOT_DOWN_SHIFT2;
	    }
	    else if(!strcmp(params[0], "shift3")) {
		c.reason = CONPLOT_DOWN_SHIFT3;
	    }
	}
	cp->ctrl_select_reason = c.reason;
	cp->select_callback = c;

	XtCallCallbacks((Widget)w, XtNselectDataCallback, &c);
}
/** 
 *  
 */
static void
ConPlotMouseDragSelect(ConPlotWidget w, XEvent *event, const char **params,
		Cardinal *num_params)
{
	ConPlotPart *cp = (w != NULL) ? &w->con_plot : NULL;
	AxesPart *ax = (w != NULL) ? &w->axes : NULL;
	int cursor_x, cursor_y;
	ConPlotSelectCallbackStruct c;

	int i, j;
	double x, y;
	ConEntry *entry = NULL;

	if(ax->cursor_mode == AXES_SELECT_CURSOR ||
		ax->cursor_mode == AXES_SELECT_CURSOR) {
	    _AxesMove((AxesWidget)w, event, params, num_params);
	    return;
	}

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(inside_bar(w, cursor_x, cursor_y)) 
	{
		XtCallCallbacks((Widget)w, XtNselectBarCallback, NULL);
		return;
	}
	if(outside(w, cursor_x, cursor_y)) return;

	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;

	if(cp->num_entries <= 0) return;

	entry = cp->entry[0];
	if(entry->m.nx <= 0 || entry->m.ny <= 0) return;
	x = scale_x(&ax->d, cursor_x);
	y = scale_y(&ax->d, cursor_y);

#ifdef HAVE_GSL
	i = gsl_interp_bsearch(entry->m.x, x, 0, entry->m.nx-1);
	j = gsl_interp_bsearch(entry->m.y, y, 0, entry->m.ny-1);
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return;
#endif

	c.entry_id = entry->id;
	c.x = entry->m.x[i];
	c.y = entry->m.y[j];
	c.z = entry->m.z[i+j*entry->m.nx];
	c.i = i;
	c.j = j;
	c.pos = i+j*entry->m.nx;

	c.reason = CONPLOT_DRAG;

	if(*num_params > 0)
	{
	    if(!strcmp(params[0], "ctrl1")) {
		c.reason = CONPLOT_DRAG_CTRL1;
	    }
	    else if(!strcmp(params[0], "ctrl2")) {
		c.reason = CONPLOT_DRAG_CTRL2;
	    }
	    else if(!strcmp(params[0], "ctrl3")) {
		c.reason = CONPLOT_DRAG_CTRL3;
	    }
	    else if(!strcmp(params[0], "shift1")) {
		c.reason = CONPLOT_DRAG_SHIFT1;
	    }
	    else if(!strcmp(params[0], "shift2")) {
		c.reason = CONPLOT_DRAG_SHIFT2;
	    }
	    else if(!strcmp(params[0], "shift3")) {
		c.reason = CONPLOT_DRAG_SHIFT3;
	    }
	}

	// only do callback if the cell has changed.
	if(c.entry_id != cp->select_callback.entry_id ||
	    c.i != cp->select_callback.i || c.j != cp->select_callback.j)
	{
	    cp->select_callback = c;
	    XtCallCallbacks((Widget)w, XtNselectDataCallback, &c);
	}
}

static Boolean
outside(ConPlotWidget w, int cursor_x, int cursor_y)
{
	AxesPart *ax = (w != NULL) ? &w->axes : NULL;
	int xmin = unscale_x(&ax->d, ax->x1[ax->zoom]);
	int xmax = unscale_x(&ax->d, ax->x2[ax->zoom]);
	int ymin = unscale_y(&ax->d, ax->y1[ax->zoom]);
	int ymax = unscale_y(&ax->d, ax->y2[ax->zoom]);

	if(xmin > xmax) {
	    int x = xmin;
	    xmin = xmax;
	    xmax = x;
	}
	if(ymin > ymax) {
	    int y = ymin;
	    ymin = ymax;
	    ymax = y;
	}
	
	return (cursor_x < xmin || cursor_x > xmax ||
		cursor_y < ymin || cursor_y > ymax) ? True : False;
}

static Boolean
inside_bar(ConPlotWidget w, int cursor_x, int cursor_y)
{
	ConPlotPart *cp = &w->con_plot;
	int xmin = cp->bar_x;
	int xmax = cp->bar_x + cp->bar_width + 1;
	int ymin = w->axes.d.iy2;
	int ymax = w->axes.d.iy1-w->axes.d.iy2+1;

	if(xmin > xmax) {
	    int x = xmin;
	    xmin = xmax;
	    xmax = x;
	}
	if(ymin > ymax) {
	    int y = ymin;
	    ymin = ymax;
	    ymax = y;
	}
	
	return (cursor_x >= xmin && cursor_x <= xmax &&
		cursor_y >= ymin && cursor_y <= ymax) ? True : False;
}
