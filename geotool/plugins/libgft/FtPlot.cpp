/** \file FtPlot.cpp
 *  \brief Defines widget FtPlot.
 *  \author Ivan Henson
 */
/*
 * NAME
 *      FtPlot Widget -- widget for computing fft's
 *
 * FILES
 *      FtPlot.h
 *
 * BUGS
 *
 * NOTES
 *
 * AUTHOR
 *		Ivan Henson	8/91
 */
#include "config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */
#include "gsl/gsl_fft_real.h"

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "FtPlotP.h"
#include "widget/AxesClass.h"
#include "BasicSource.h"
#include "DataMethod.h"

extern "C"
{
#include "libgplot.h"
#include "libgmath.h"
#include "tapers.h"
}

#define alog10(x)	pow((double)10., (double)x)
#define Log10(x)	((x > 0.) ? log10(x) : log10(1.e-10))
#define DOTS_PER_INCH   300     /* PostScript dots per inch */
#define POINTS_PER_DOT  (72./300.)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void DoInstrumentCorr(FtPlotWidget w, FtEntry *entry, float *xPow);
static void DoSmoothing(FtPlotWidget w, FtEntry *entry, float *xPow);

FtEntry::FtEntry(FtPlotWidget widget, FtData *ftd)
{
    init(widget);
    setUp(ftd);
}

FtEntry::FtEntry(FtPlotWidget widget, FtData *ftd, const char *name,
		float *xp, float *ph, Pixel fg)
{
    init(widget);

    on = true;
    data_fg = fg;
    nf = ftd->nf;
    np2 = ftd->np2;
    df = ftd->df;
    dt = ftd->dt;
    time = ftd->time;
    rsp = ftd->rsp;
    strncpy(sta, ftd->sta, sizeof(sta));
    y = (float *)malloc(nf*sizeof(float));
    calib = ftd->calib;
    calper = ftd->calper;
    derived_spectrum = true;
    xPow = xp;
    phase = ph;
    strncpy(chan, name, sizeof(chan));
    scaleFt();
}

FtEntry::FtEntry(FtPlotWidget widget)
{
    init(widget);
}

void FtEntry::init(FtPlotWidget widget)
{
    FtPlotClassPart *cc = &ftPlotClassRec.ft_plot_class;

    w = widget;
    id = cc->nextid++;
    data_fg = 0;
    on = true;
    selected = false;
    selectable = true;
    fill = false;
    derived_spectrum = false;
    i1 = 0;
    i2 = 0;
    npts = 0;
    np2 = 0;
    nf = 0;
    dt = 0.;
    time = NULL_TIME;
    type = FT_TMP;
    t_length = 0;
    t = (float *)NULL;
    amp = (float *)NULL;
    phase = (float *)NULL;
    x = (float *)NULL;
    y = (float *)NULL;

    s.segs = (DSegment *)NULL;
    s.size_segs = 0;
    s.nsegs = 0;
    s.start = 0;
    s.nshow = 0;

    r.segs = (RSeg *)NULL;
    r.size_segs = 0;
    r.nsegs = 0;
    r.start = 0;
    r.nshow = 0;
    r.xmin = 0.;
    r.xmax = 0.;
    r.ymin = 0.;
    r.ymax = 0.;

    df = 0.;
    x_min = 0.;
    x_max = 0.;
    y_min = 0.;
    y_max = 0.;
    memset(taper, 0, sizeof(taper));
    smoothvalue = -1.;
    do_rsp = false;
    rsp = NULL;
}

FtEntry::~FtEntry(void)
{
    Free(amp);
    Free(x);
    Free(y);
    Free(s.segs);
    if(derived_spectrum) {
	Free(xPow);
	Free(phase);
    }
}

void FtEntry::setUp(FtData *ftd)
{
    type = FT_TMP;
    data_fg = ftd->fg;
    nf = ftd->nf;
    np2 = ftd->np2;
    df = ftd->df;
    dt = ftd->dt;
    time = ftd->time;
    xPow = ftd->xPow;
    phase = ftd->phase;
    rsp = ftd->rsp;
    strncpy(sta, ftd->sta, sizeof(sta));
    strncpy(chan, ftd->chan, sizeof(chan));
    y = (float *)malloc(nf*sizeof(float));
    calib = ftd->calib;
    calper = ftd->calper;

    scaleFt();
}

void FtEntry::scaleFt(void)
{
    scaleX();
    scaleY();
}

void FtEntry::scaleX(void)
{
    AxesPart *ax = &w->axes;
    int i;

    if(nf <= 0 || !on)
    {
	if (nf > 1 && df > 0.0)
	{
	    x_max = (nf-1)*df;	
	    if(ax->a.log_x) {
		x_max = Log10(x_max);
		x_min = Log10(.5*df);
	    }
	}
    }
    else if(w->ft_plot.xaxis_period) {
	if(x == NULL && !(x = (float *)AxesMalloc((Widget)w,
			nf*sizeof(float)))) return;
	for(i = 0; i < nf-1; i++) {
	    double freq = (nf-1-i)*df;
	    x[i] = 1./freq;
	}
	x[nf-1] = x[nf-2];
	if(ax->a.log_x)
	{
	    for(i = 0; i < nf; i++) {
		x[i] = Log10(x[i]);
	    }
	}
	x_min = x[0];
	x_max = x[nf-1];
    }
    else if(ax->a.log_x)
    {
	if(x == NULL && !(x = (float *)AxesMalloc((Widget)w,
			nf*sizeof(float)))) return;
	x[0] = Log10(.5*df);
	for(i = 1; i < nf; i++) {
	    x[i] = Log10(i*df);
	}
	x_min = x[0];
	x_max = x[nf-1];
    }
    else
    {
	x_min = 0.;
	x_max = (nf-1)*df;
	Free(x);
    }
}

void FtEntry::scaleY(void)
{
    AxesPart *ax = &w->axes;
    int i, i0;
    double fac, min, max, om, om2, om4;

    if(nf <= 1 || !on) return;

    memcpy(y, xPow, nf*sizeof(float));

    DoInstrumentCorr(w, this, y);

    if(w->ft_plot.mode == FT_AMP)
    {
	fac = np2/(2.*dt);
	for(i = 0; i < nf; i++) {
	    y[i] = dt*sqrt(fac*y[i]);
	}
	if(w->ft_plot.der == FT_VEL) {
	    for(i = 0; i < nf; i++) {
		om = 2.*M_PI*i*df;
		y[i] *= om;
	    }
	}
	else if(w->ft_plot.der == FT_ACC) {
	    for(i = 0; i < nf; i++) {
		om = 2.*M_PI*i*df;
		om2 = om*om;
		y[i] *= om2;
	    }
	}
    }
    else // power
    {
	if(w->ft_plot.der == FT_VEL) {
	    for(i = 0; i < nf; i++) {
		om = 2.*M_PI*i*df;
		om2 = om*om;
		y[i] *= om2;
	    }
	}
	else if(w->ft_plot.der == FT_ACC) {
	    for(i = 0; i < nf; i++) {
		om = 2.*M_PI*i*df;
		om4 = pow((double)om, (double)4.);
		y[i] *= om4;
	    }
	}
    }

    // db and units. The default is nanometers (units = FT_NM)

    ax->a.log_y = False;

    if(w->ft_plot.units == FT_M)
    {
	fac = (w->ft_plot.mode == FT_AMP) ? 1.e-9 : 1.e-18;
	for(i = 0; i < nf; i++) {
	    y[i] *= fac;
	}
    }
    else if(w->ft_plot.units == FT_LOG_NM)
    {
	ax->a.log_y = True;
	for(i = 0; i < nf; i++) {
	    y[i] = Log10(y[i]);
	}
    }
    else if(w->ft_plot.units == FT_LOG_M)
    {
	ax->a.log_y = True;
	fac = (w->ft_plot.mode == FT_AMP) ? 9 : 18;
	for(i = 0; i < nf; i++) {
	    y[i] = Log10(y[i]) - fac;
	}
    }
    else if(w->ft_plot.units == FT_DB_RE_NM) {
	fac = (w->ft_plot.mode == FT_AMP) ? 20. : 10.;
	for(i = 0; i < nf; i++) {
	    y[i] = fac*Log10(y[i]);
	}
    }
    else if (w->ft_plot.units == FT_DB_RE_M)
    {
	// meter = 1.e-9 nanometer
	fac = (w->ft_plot.mode == FT_AMP) ? 20. : 10.;
	for(i = 0; i < nf; i++) {
	    y[i] = fac*Log10(y[i]) - 180.;
	}
    }

    if(w->ft_plot.smoothing_width > 0.00001) {
	DoSmoothing(w, this, this->y);
    }

    if(w->ft_plot.xaxis_period) {
	float f;
	int n = nf/2;
	y[0] = y[1];
	for(i = 0; i < n; i++) {
	    f = y[i];
	    y[i] = y[nf-1-i];
	    y[nf-1-i] = f;
	}
    }

    i0 = (w->ft_plot.draw_dc && !w->ft_plot.xaxis_period) ? 0 : 1;
    if(i0 > nf-1) i0 = nf-1;
    min = max = y[i0];
    for(i = i0; i < nf; i++) 
    {
	if(min > y[i]) min = y[i];
	if(max < y[i]) max = y[i];
    }
    y_min = min;
    y_max = max;
}

class NoiseEntry : public FtEntry
{
    public :
    float *xnoise;

    NoiseEntry(FtPlotWidget widget) : FtEntry(widget)
    { 
	npts = 1;
	dt = 1.;
	df = 1.;
	type = FT_NOISE;
	xnoise = NULL;
    }
    ~NoiseEntry(void) {
	Free(xnoise);
    }

    void scaleFt(void) {
	scaleX();
	scaleY();
    }

    virtual void scaleX(void)
    {
	AxesPart *ax = &w->axes;
	int i;

	if(w->ft_plot.xaxis_period) {
	    for(i = 0; i < nf; i++) x[i] = 1./xnoise[nf-1-i];
	}
	else {
	    for(i = 0; i < nf; i++) x[i] = xnoise[i];
	}
	if(ax->a.log_x) {
	    for(i = 0; i < nf; i++) x[i] = Log10(x[i]);
	}
	x_min = x[0];
	x_max = x[nf-1];
    }

    virtual void scaleY(void)
    {
	AxesPart *ax = &w->axes;
	int i;
	double fac, min, max, om, om2, om4;

	if(nf <= 1 || !on) return;

	memcpy(y, xPow, nf*sizeof(float));

	if(w->ft_plot.mode == FT_AMP)
	{
	    for(i = 0; i < nf; i++) {
		y[i] = sqrt(y[i]);
	    }
	    if(w->ft_plot.der == FT_VEL) {
		for(i = 0; i < nf; i++) {
		    om = 2.*M_PI*xnoise[i];
		    y[i] *= om;
		}
	    }
	    else if(w->ft_plot.der == FT_ACC) {
		for(i = 0; i < nf; i++) {
		    om = 2.*M_PI*xnoise[i];
		    om2 = om*om;
		    y[i] *= om2;
		}
	    }
	}
	else // power
	{
	    if(w->ft_plot.der == FT_VEL) {
		for(i = 0; i < nf; i++) {
		    om = 2.*M_PI*xnoise[i];
		    om2 = om*om;
		    y[i] *= om2;
		}
	    }
	    else if(w->ft_plot.der == FT_ACC) {
		for(i = 0; i < nf; i++) {
		    om = 2.*M_PI*xnoise[i];
		    om4 = pow((double)om, (double)4.);
		    y[i] *= om4;
		}
	    }
	}

	// db and units. The default is nanometers (units = FT_NM)

	ax->a.log_y = False;

	if(w->ft_plot.units == FT_M) {
	    fac = (w->ft_plot.mode == FT_AMP) ? 1.e-9 : 1.e-18;
	    for(i = 0; i < nf; i++) y[i] *= fac;
	}
	else if(w->ft_plot.units == FT_LOG_NM) {
	    ax->a.log_y = True;
	    for(i = 0; i < nf; i++) y[i] = Log10(y[i]);
	}
	else if(w->ft_plot.units == FT_LOG_M) {
	    ax->a.log_y = True;
	    fac = (w->ft_plot.mode == FT_AMP) ? 9 : 18;
	    for(i = 0; i < nf; i++) y[i] = Log10(y[i]) - fac;
	}
	else if(w->ft_plot.units == FT_DB_RE_NM) {
	    fac = (w->ft_plot.mode == FT_AMP) ? 20. : 10.;
	    for(i = 0; i < nf; i++) y[i] = fac*Log10(y[i]);
	}
	else if (w->ft_plot.units == FT_DB_RE_M) {
	    // meter = 1.e-9 nanometer
	    fac = (w->ft_plot.mode == FT_AMP) ? 20. : 10.;
	    for(i = 0; i < nf; i++) y[i] = fac*Log10(y[i]) - 180.;
	}

	if(w->ft_plot.xaxis_period) {
	    float f;
	    int n = nf/2;
	    for(i = 0; i < n; i++) {
		f = y[i];
		y[i] = y[nf-1-i];
		y[nf-1-i] = f;
	    }
	}

	min = max = y[0];
	for(i = 1; i < nf; i++) {
	    if(min > y[i]) min = y[i];
	    if(max < y[i]) max = y[i];
	}
	y_min = min;
	y_max = max;
    }
};

FtPlotRec ftrec;

//#define	offset(field)		XtOffset(FtPlotWidget, ft_plot.field)
#define offset(field) ((Cardinal) ((char *)(&ftrec.ft_plot.field) - ((char *)(&ftrec))))
static XtResource	resources[] = 
{
	{XtNtaper, XtCTaper, XtRInt, sizeof(int),
		offset(taper), XtRImmediate, (XtPointer)HANN_TAPER},
	{XtNmode, XtCMode, XtRInt, sizeof(int),
		offset(mode), XtRImmediate, (XtPointer)FT_POWER},
	{XtNxAxisPeriod, XtCXAxisPeriod, XtRBoolean,
		sizeof(Boolean), offset(xaxis_period), XtRString,
		(XtPointer)"False"},
	{XtNdisplayArrayTraces, XtCDisplayArrayTraces, XtRBoolean,
		sizeof(Boolean), offset(display_array_traces), XtRString,
		(XtPointer)"True"},
	{XtNdisplayArrayMean, XtCDisplayArrayMean, XtRBoolean,
		sizeof(Boolean), offset(display_array_mean), XtRString,
		(XtPointer)"False"},
	{XtNdisplayArrayMedian, XtCDisplayArrayMedian, XtRBoolean,
		sizeof(Boolean), offset(display_array_median), XtRString,
		(XtPointer)"False"},
	{XtNdisplayArrayPercentile, XtCDisplayArrayPercentile, XtRBoolean,
		sizeof(Boolean), offset(display_array_percentile), XtRString,
		(XtPointer)"False"},
	{XtNdisplayArrayStdDev, XtCDisplayArrayStdDev, XtRBoolean,
		sizeof(Boolean), offset(display_array_std_dev), XtRString,
		(XtPointer)"False"},
	{XtNdisplayArrayNlnm, XtCDisplayArrayNlnm, XtRBoolean,
		sizeof(Boolean), offset(display_array_nlnm), XtRString,
		(XtPointer)"False"},
	{XtNdisplayArrayNhnm, XtCDisplayArrayNhnm, XtRBoolean,
		sizeof(Boolean), offset(display_array_nhnm), XtRString,
		(XtPointer)"False"},
	{XtNunits, XtCUnits, XtRInt, sizeof(int),
		offset(units), XtRImmediate, (XtPointer)FT_DB_RE_NM},
	{XtNder, XtCDer, XtRInt, sizeof(int),
		offset(der), XtRImmediate, (XtPointer)FT_DISP},
	{XtNgrouping, XtCGrouping, XtRInt, sizeof(int),
		offset(grouping), XtRImmediate, (XtPointer)FT_ALL},
        {XtNinstrumentCorr, XtCInstrumentCorr, XtRBoolean, sizeof(Boolean),
                offset(instrument_corr), XtRString, (XtPointer)"False"},
        {XtNdemean, XtCDemean, XtRBoolean, sizeof(Boolean),
                offset(demean), XtRString, (XtPointer)"True"},
        {XtNdrawDC, XtCDrawDC, XtRBoolean, sizeof(Boolean),
                offset(draw_dc), XtRString, (XtPointer)"False"},
        {XtNwindows, XtCWindows, XtRInt, sizeof(int),
                offset(windows), XtRImmediate, (XtPointer)1},
        {XtNoverlap, XtCOverlap, XtRInt, sizeof(int),
                offset(overlap), XtRImmediate, (XtPointer)0},
        {XtNpercentile1, XtCPercentile1, XtRInt, sizeof(int),
                offset(percentile1), XtRImmediate, (XtPointer)10},
        {XtNpercentile2, XtCPercentile2, XtRInt, sizeof(int),
                offset(percentile2), XtRImmediate, (XtPointer)90},
        {XtNselectCallback, XtCSelectCallback, XtRCallback,
                sizeof(XtCallbackList), offset(select_callbacks),
                XtRCallback, (XtPointer)NULL},
};
#undef offset

/* Private functions */

static void Initialize(Widget req, Widget nou);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static Boolean SetValues(FtPlotWidget cur, FtPlotWidget req, FtPlotWidget nou);
extern "C" {
static int cmp(const void *A, const void *B);
};
static void FtPlotRedraw(AxesWidget w, int type, double shift);
static void FtPlotHardCopy(AxesWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
			float font_scale, PrintParam  *p);
static void ComputeMean(FtPlotWidget w);
static void ComputeStdDev(FtPlotWidget w);
static void ComputeMedian(FtPlotWidget w);
static void ComputePercentiles(FtPlotWidget w);
static void ComputeNoise(FtPlotWidget w, const char *type);
static void DoFt(FtPlotWidget w, FtData *ftd);

static double get_mean(float *dB, int index, int nd2);
static void DrawEntry(FtPlotWidget w, DrawStruct *d, FtEntry *entry,
			SegArray *s);
static void FtPlotZoomZero(FtPlotWidget w);
static void Motion(FtPlotWidget w, XEvent *event, String *params,
                        Cardinal *num_params);
static FtEntry *WhichEntry(FtPlotWidget w, int cursor_x, int cursor_y);
static void
FtPlotMouseDownSelect(FtPlotWidget w, XEvent *event, const char **params,
			Cardinal *num_params);
static Boolean outside(FtPlotWidget w, int cursor_x, int cursor_y);
static void mouseDownSelect(FtPlotWidget w, FtEntry *selected_entry);
static void FtPlotRedisplayEntry(FtPlotWidget w, FtEntry *entry);
static void FillSegments(FtPlotWidget w, DrawStruct *d, FtEntry *entry,
			SegArray *seg_array, double x0, double y0);
static void DrawRectangles(FtPlotWidget w, GC gc, XRectangle *recs, int nrecs);
static void LeaveWindow(FtPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static int findx(float *x, int n, double fx);
static void Destroy(Widget w);


static char     defTrans[] =
"~Shift ~Ctrl<Btn2Down>:Zoom() \n\
  <Btn2Up>:		Zoom() \n\
  <Btn2Motion>:		Zoom() \n\
 ~Shift ~Ctrl<Btn3Down>:Zoom(horizontal) \n\
  <Btn3Up>:		Zoom(horizontal) \n\
  <Btn3Motion>:		Zoom(horizontal) \n\
 ~Ctrl Shift<Btn2Down>:	ZoomBack() \n\
 ~Shift Ctrl<Btn2Down>:	Magnify() \n\
 ~Shift ~Ctrl<Btn1Down>:MouseDownSelect() \n\
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
  <LeaveWindow>:	LeaveWindow() \n\
  <BtnDown>:		BtnDown()";

static XtActionsRec     actionsList[] =
{
    {(char *)"MouseDownSelect",	(XtActionProc)FtPlotMouseDownSelect},
    {(char *)"Zoom",		(XtActionProc)_AxesZoom},
    {(char *)"ZoomBack",	(XtActionProc)_AxesZoomBack},
    {(char *)"Magnify",		(XtActionProc)_AxesMagnify},
    {(char *)"Move",		(XtActionProc)_AxesMove},
    {(char *)"Scale",		(XtActionProc)_AxesScale},
    {(char *)"AddCrosshair",	(XtActionProc)_AxesAddCrosshair},
    {(char *)"AddLine",		(XtActionProc)_AxesAddLine},
    {(char *)"AddDoubleLine",	(XtActionProc)_AxesAddDoubleLine},
    {(char *)"DeleteCursor",	(XtActionProc)_AxesDeleteCursor},
    {(char *)"UnzoomPercentage",(XtActionProc)_AxesUnzoomPercentage},
    {(char *)"UnzoomHorizontal",(XtActionProc)_AxesUnzoomHorizontal},
    {(char *)"UnzoomVertical",	(XtActionProc)_AxesUnzoomVertical},
    {(char *)"ZoomPercentage",	(XtActionProc)_AxesZoomPercentage},
    {(char *)"ZoomHorizontal",	(XtActionProc)_AxesZoomHorizontal},
    {(char *)"ZoomVertical",	(XtActionProc)_AxesZoomVertical},
    {(char *)"Page",		(XtActionProc)AxesPage},
    {(char *)"KeyCommand",	(XtActionProc)_AxesKeyCommand},
    {(char *)"BtnDown",		(XtActionProc)_AxesBtnDown},
    {(char *)"Motion",		(XtActionProc)Motion},
    {(char *)"LeaveWindow",	(XtActionProc)LeaveWindow},
};


FtPlotClassRec	ftPlotClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&axesClassRec),	/* superclass */
	(char *)"FtPlot",		/* class_name */
	sizeof(FtPlotRec),		/* widget_size */
	NULL,				/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
        actionsList,                   	/* actions */
        XtNumber(actionsList),		/* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
	(XrmClass)0,			/* xrm_class */
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
	{	/* FtPlotClass fields */
		1,			/* nextid */
	},
};

WidgetClass ftPlotWidgetClass = (WidgetClass)&ftPlotClassRec;


static void
Initialize(Widget w_req, Widget	w_new)
{
    FtPlotWidget nou = (FtPlotWidget)w_new;

    nou->ft_plot.wait = False;
    nou->ft_plot.smoothing_width = 0.0;
    nou->ft_plot.first_call = True;
    nou->ft_plot.select_entry = NULL;
    nou->ft_plot.point_x = -1;
    nou->ft_plot.point_y = -1;

    nou->ft_plot.overlay_display = False;
    nou->ft_plot.overlay_npts = 0;
    nou->ft_plot.overlay_x = NULL;
    nou->ft_plot.overlay_y = NULL;
    nou->ft_plot.object_owner = new Gobject();

    nou->ft_plot.taper_length = .10;
    nou->ft_plot.taper_min_pts = 5;
    nou->ft_plot.taper_max_pts = 200;

    nou->axes.redraw_data_func = FtPlotRedraw;
    nou->axes.hard_copy_func = FtPlotHardCopy;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    FtPlotWidget w = (FtPlotWidget)widget;
    (*((ftPlotWidgetClass->core_class.superclass)->
			core_class.realize))((Widget)w,  valueMask, attrs);
    w->ft_plot.gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
    XSetBackground(XtDisplay(w), w->ft_plot.gc, w->core.background_pixel);

    w->ft_plot.gc_invert = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
    XSetFunction(XtDisplay(w), w->ft_plot.gc_invert, GXinvert);
    XSetBackground(XtDisplay(w), w->ft_plot.gc_invert,
				w->core.background_pixel);
    w->ft_plot.white = StringToPixel((Widget)w, (char *)"White");
    w->ft_plot.black = StringToPixel((Widget)w, (char *)"Black");

    _AxesRedraw((AxesWidget)widget);
    _FtPlotRedraw((FtPlotWidget)widget);
}

static Boolean
SetValues(FtPlotWidget cur, FtPlotWidget req, FtPlotWidget nou)
{
    int i;
    bool redisplay = false;
    bool do_limits = false;

    if( cur->ft_plot.display_array_traces != req->ft_plot.display_array_traces
     || cur->ft_plot.display_array_mean != req->ft_plot.display_array_mean
     || cur->ft_plot.display_array_median != req->ft_plot.display_array_median
     || cur->ft_plot.display_array_percentile !=
	req->ft_plot.display_array_percentile
     || cur->ft_plot.display_array_std_dev != req->ft_plot.display_array_std_dev
     || cur->ft_plot.xaxis_period != req->ft_plot.xaxis_period)
    {
	redisplay = true;
	do_limits = true;
    }
    if(cur->ft_plot.display_array_nlnm != req->ft_plot.display_array_nlnm)
    {
	if(!req->ft_plot.display_array_nlnm) {
	    for(i = (int)nou->ft_plot.entries.size()-1; i >= 0; i--) {
		if(nou->ft_plot.entries[i]->type == FT_NOISE
		    && !strcmp(nou->ft_plot.entries[i]->chan, "NLNM"))
		{
		    delete nou->ft_plot.entries[i];
		    nou->ft_plot.entries.erase(nou->ft_plot.entries.begin()+i);
		}
	    }
	}
	else {
	    do_limits = true;
	}
	redisplay = true;
    }
    if(cur->ft_plot.display_array_nhnm != req->ft_plot.display_array_nhnm)
    {
	if(!req->ft_plot.display_array_nhnm) {
	    for(i = (int)nou->ft_plot.entries.size()-1; i >= 0; i--) {
		if(nou->ft_plot.entries[i]->type == FT_NOISE
		    && !strcmp(nou->ft_plot.entries[i]->chan, "NHNM"))
		{
		    delete nou->ft_plot.entries[i];
		    nou->ft_plot.entries.erase(nou->ft_plot.entries.begin()+i);
		}
	    }
	}
	else {
	    do_limits = true;
	}
	redisplay = true;
    }

    if( cur->ft_plot.taper != req->ft_plot.taper ||
	cur->ft_plot.windows != req->ft_plot.windows ||
	cur->ft_plot.overlap != req->ft_plot.overlap ||
	cur->ft_plot.demean != req->ft_plot.demean)
    {
	for(i = 0; i < (int)nou->ft_plot.ftdata.size(); i++) {
	    DoFt(nou, nou->ft_plot.ftdata[i]);
	}
	redisplay = true;
    }
    if(cur->ft_plot.grouping != req->ft_plot.grouping)
    {
    }
    if(cur->ft_plot.instrument_corr != req->ft_plot.instrument_corr)
    {
	for(i = 0; i < (int)nou->ft_plot.entries.size(); i++) {
	    nou->ft_plot.entries[i]->scaleY();
	}
	redisplay = true;
	do_limits = true;
    }

    if(cur->axes.a.log_x != req->axes.a.log_x)
    {
	for(i = 0; i < (int)nou->ft_plot.entries.size(); i++) {
	    nou->ft_plot.entries[i]->scaleX();
	}
	redisplay = true;
	do_limits = true;
    }
    if(cur->ft_plot.units != req->ft_plot.units ||
	   cur->ft_plot.mode != req->ft_plot.mode ||
	   cur->ft_plot.der != req->ft_plot.der)
    {
	for(i = 0; i < (int)nou->ft_plot.entries.size(); i++) {
	    nou->ft_plot.entries[i]->scaleY();
	}
	redisplay = true;
	do_limits = true;
    }
    if(cur->ft_plot.draw_dc != req->ft_plot.draw_dc) {
	redisplay = true;
    }

    if(redisplay) {
	FtPlotCompute(nou, false, do_limits);

	nou->axes.setvalues_redisplay = True;
	if(nou->axes.redisplay_pending) redisplay = False;
	nou->axes.redisplay_pending = True;
    }
    return(redisplay);
}

void
FtPlotSmooth(FtPlotWidget w, double width)
{
    w->ft_plot.smoothing_width = width;

    FtPlotCompute(w, true, false);
}

double
FtPlotSmoothingWidth(FtPlotWidget w)
{
    return w->ft_plot.smoothing_width;
}

/* Input length as % of total length (1-100)
 */
void
FtPlotSetCosineTaper(FtPlotWidget w, double length, int min_pts, int max_pts)
{
    w->ft_plot.taper_length = length/100.;
    w->ft_plot.taper_min_pts = min_pts;
    w->ft_plot.taper_max_pts = max_pts;
    w->ft_plot.taper = COSINE_TAPER;
    for(int i = 0; i < (int)w->ft_plot.ftdata.size(); i++) {
	DoFt(w, w->ft_plot.ftdata[i]);
    }
    FtPlotCompute(w, true, false);
}

/* Return beg_length and end_length as % of total length (1-100)
 */
void
FtPlotGetCosineTaper(FtPlotWidget w, double *length, int *min_pts, int *max_pts)
{
    *length = w->ft_plot.taper_length*100;
    *min_pts = w->ft_plot.taper_min_pts;
    *max_pts = w->ft_plot.taper_max_pts;
}

static void
FtPlotRedraw(AxesWidget widget, int type, double shift)
{
    FtPlotWidget w = (FtPlotWidget)widget;

    w->ft_plot.point_x = -1;

    switch(type)
    {
	case DATA_REDISPLAY :
		_FtPlotRedisplay(w, False);
			break;
	case DATA_REDRAW :
		_FtPlotRedraw(w);
		break;
	case DATA_ZOOM_ZERO :
		FtPlotZoomZero(w);
		break;
	case DATA_JUMP_HOR :
	case DATA_JUMP_VERT :
	case DATA_SCROLL_HOR :
	case DATA_SCROLL_VERT :
		_FtPlotRedraw(w);
		_FtPlotRedisplay(w, False);
		break;
    }
}

/** 
 * 
 */
void
_FtPlotRedisplay(FtPlotWidget w, Boolean copy_area)
{
    FtPlotPart *ft = &w->ft_plot;
    int i, i0;

    for(i = 0; i < (int)ft->entries.size(); i++) if(ft->entries[i]->on)
    {
	FtPlotRedisplayEntry(w, ft->entries[i]);
    }
    if(w->ft_plot.overlay_display && w->ft_plot.overlay_npts > 0)
    {
	SegArray s;
	float *x = NULL;
	s.segs = NULL;
	s.nsegs = 0;
	s.size_segs = 0;
	w->axes.d.s = &s;
	if( !(x = (float *)AxesMalloc((Widget)w,
		ft->overlay_npts*sizeof(float))) ) return;

	if(w->axes.a.log_x) {
	    for(i = 0; i < ft->overlay_npts; i++) {
		x[i] = Log10(ft->overlay_x[i]);
	    }
	}
	else {
	    memcpy(x, ft->overlay_x, ft->overlay_npts*sizeof(float));
	}
	if(ft->overlay_x[0] > 0. || ft->draw_dc) {
	    i0 = 0;
	}
	else {
	    i0 = 1;
	}
	plotxy(&w->axes.d, ft->overlay_npts-i0, x+i0, ft->overlay_y+i0,
			w->axes.x1[w->axes.zoom], w->axes.x2[w->axes.zoom],
			w->axes.y1[w->axes.zoom], w->axes.y2[w->axes.zoom]);
	Free(x);
	_AxesDrawSegments((AxesWidget)w, ft->gc, s.segs, s.nsegs);
	Free(s.segs);
    }
    if(ft->point_x >= 0) {
	DrawRectangles(w, w->axes.mag_invertGC, ft->point_recs, 2);
    }
    if(w->axes.use_pixmap && copy_area && XtIsRealized((Widget)w))
    {
	XCopyArea(XtDisplay(w), w->axes.pixmap, XtWindow(w),
		    w->axes.axesGC, 0, 0, w->core.width, w->core.height, 0, 0);
    }
}

static void
FtPlotRedisplayEntry(FtPlotWidget w, FtEntry *entry)
{
    Pixel fg;

    if(!XtIsRealized((Widget)w)) return;

    fg = (entry->selected) ? w->axes.select_fg : entry->data_fg;

    if(entry->type == FT_TMP)
    {
	XSetPlaneMask(XtDisplay(w), w->ft_plot.gc_invert,
			fg ^ w->core.background_pixel);
	XSetForeground(XtDisplay(w), w->ft_plot.gc_invert, fg);
	_AxesDrawSegments((AxesWidget)w, w->ft_plot.gc_invert, entry->s.segs,
			entry->s.nsegs);
    }
    else
    {
	XSetForeground(XtDisplay(w), w->ft_plot.gc, fg);
	_AxesDrawSegments((AxesWidget)w, w->ft_plot.gc, entry->s.segs,
			entry->s.nsegs);
    }
}

static void
Destroy(Widget w)
{
    FtPlotClear((FtPlotWidget)w);
}

void
FtPlotClear(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;
    int i;

    for(i = (int)ft->entries.size()-1; i >= 0; i--)
	if(ft->entries[i]->type != FT_NOISE)
    {
	delete ft->entries[i];
	ft->entries.erase(ft->entries.begin()+i);
    }

    for(i = 0; i < (int)ft->ftdata.size(); i++) {
	delete ft->ftdata[i];
    }
    ft->ftdata.clear();
	    
    _AxesRedraw((AxesWidget)w);
    _FtPlotRedraw(w);
    if(!w->axes.redisplay_pending)
    {
	_AxesRedisplayAxes((AxesWidget)w);
	_FtPlotRedisplay(w, True);
	_AxesRedisplayXor((AxesWidget)w);
    }
}

void
FtPlotClearTmps(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;

    for(int i = (int)ft->entries.size()-1; i >= 0; i--)
	if(ft->entries[i]->type == FT_TMP)
    {
	delete ft->entries[i];
	ft->entries.erase(ft->entries.begin()+i);
    }
    for(int i = 0; i < (int)ft->ftdata.size(); i++) {
	delete ft->ftdata[i];
    }
    ft->ftdata.clear();

    _AxesRedisplayAxes((AxesWidget)w);
    _FtPlotRedisplay(w, True);
    _AxesRedisplayXor((AxesWidget)w);
}

void
FtPlotCompute(FtPlotWidget w, bool redisplay, bool do_limits)
{
    FtPlotPart *ft = &w->ft_plot;
    int	i;
    bool use_noise;

    for(i = (int)ft->entries.size()-1; i >= 0; i--)
	if(ft->entries[i]->type == FT_TMP)
    {
	delete ft->entries[i];
	ft->entries.erase(ft->entries.begin()+i);
    }

    if(ft->display_array_traces) {
	for(i = 0; i < (int)ft->ftdata.size(); i++) {
	    ft->entries.push_back(new FtEntry(w, ft->ftdata[i]));
	}
    }

    if(ft->display_array_mean) {
	ComputeMean(w);
    }
    if(ft->display_array_std_dev) {
	ComputeStdDev(w);
    }
    if(ft->display_array_median) {
	ComputeMedian(w);
    }
    if(ft->display_array_percentile) {
	ComputePercentiles(w);
    }
    if(ft->display_array_nlnm) {
	ComputeNoise(w, "NLNM");
    }
    if(ft->display_array_nhnm) {
	ComputeNoise(w, "NHNM");
    }

    use_noise = true;
    for(i = 0; i < (int)ft->entries.size(); i++) {
	if(ft->entries[i]->type != FT_NOISE && ft->entries[i]->on) {
	    use_noise = false;
	}
    }

    if(do_limits && (int)ft->entries.size() > 0)
    {
	w->axes.zoom = 0;
	for(i = 0; i < (int)ft->entries.size() && (!ft->entries[i]->on
		|| (ft->entries[i]->type == FT_NOISE && !use_noise)); i++);
	if(i < (int)ft->entries.size())
	{
	    double x_min = ft->entries[i]->x_min;
	    double x_max = ft->entries[i]->x_max;
	    double y_min = ft->entries[i]->y_min;
	    double y_max = ft->entries[i]->y_max;

	    for(; i < (int)ft->entries.size(); i++)
		if(ft->entries[i]->on &&
		    (ft->entries[i]->type != FT_NOISE || use_noise))
	    {
		FtEntry *entry = ft->entries[i];
		if(x_min > entry->x_min) x_min = entry->x_min;
		if(x_max < entry->x_max) x_max = entry->x_max;
		if(y_min > entry->y_min) y_min = entry->y_min;
		if(y_max < entry->y_max) y_max = entry->y_max;
	    }
	    w->axes.x1[0] = x_min - .1*(x_max - x_min);
	    w->axes.x2[0] = x_max + .1*(x_max - x_min);
	    w->axes.y1[0] = y_min - .1*(y_max - y_min);
	    w->axes.y2[0] = y_max + .1*(y_max - y_min);
	}
    }

    _AxesRedraw((AxesWidget)w);
    _FtPlotRedraw(w);

//    FtPlotZoomZero(w);
    if(redisplay)
    {
	_AxesRedisplayAxes((AxesWidget)w);
	_FtPlotRedisplay(w, True);
	_AxesRedisplayXor((AxesWidget)w);
    }
}

static void
ComputeMean(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;
    int nf, numft;
    vector<FtData *> v;
    FtData *f, *f0;
    float *mean, *phase;

    for(int i = 0; i < (int)ft->ftdata.size(); i++) {
	f = ft->ftdata[i];
	if((int)v.size() == 0) {
	    v.push_back(f);
	    f0 = f;
	}
	else if(f->nf == f0->nf) {
	    v.push_back(f);
	}
    }
    if((int)v.size() <= 1) return;
    nf = f0->nf;
    numft = (int)v.size();

    if( !(mean = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(phase = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;

    for(int i = 0; i < nf; i++) {
	double sum = 0., sump = 0.;
	for(int j = 0; j < numft; j++) {
	    f = v.at(j);
	    sum += f->xPow[i];
	    sump += f->phase[i];
	}
	mean[i] = sum/numft;
	phase[i] = sump/numft;
    }
    ft->entries.push_back(new FtEntry(w, f0, "mean", mean, phase,
		StringToPixel((Widget)w, (char *)"black")));
}

static void
ComputeStdDev(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;
    int nf, numft;
    vector<FtData *> v;
    FtData *f, *f0;
    float *sig1, *sig2, *phase;

    for(int i = 0; i < (int)ft->ftdata.size(); i++) {
	f = ft->ftdata[i];
	if((int)v.size() == 0) {
	    v.push_back(f);
	    f0 = f;
	}
	else if(f->nf == f0->nf) {
	    v.push_back(f);
	}
    }
    if((int)v.size() <= 1) return;
    nf = f0->nf;
    numft = (int)v.size();

    if( !(sig1 = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(sig2 = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(phase = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;

    for(int i = 0; i < nf; i++)
    {
	double mean = 0.;
	for(int j = 0; j < numft; j++) {
	    f = v.at(j);
	    mean += f->xPow[i];
	}
	mean /= numft;

	double sum = 0.;
	for(int j = 0; j < numft; j++) {
	    f = v.at(j);
	    sum += ((f->xPow[i] - mean) * (f->xPow[i] - mean));
	}
	double sigma = sqrt(sum/(double)(numft - 1));
	sig1[i] = mean + sigma;
	sig2[i] = mean - sigma;
	phase[i] = 0.;
    }

    ft->entries.push_back(new FtEntry(w, f0, "mean+std", sig1, phase,
		StringToPixel((Widget)w, (char *)"red")));
    ft->entries.push_back(new FtEntry(w, f0, "mean-std", sig2, phase,
		StringToPixel((Widget)w, (char *)"red")));
}

static void
ComputeMedian(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;
    int nf, numft, midpoint;
    vector<FtData *> v;
    FtData *f, *f0;
    float *median, *values, *phase;

    for(int i = 0; i < (int)ft->ftdata.size(); i++) {
	f = ft->ftdata[i];
	if((int)v.size() == 0) {
	    v.push_back(f);
	    f0 = f;
	}
	else if(f->nf == f0->nf) {
	    v.push_back(f);
	}
    }
    if((int)v.size() <= 1) return;
    nf = f0->nf;
    numft = (int)v.size();

    if( !(median = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(values = (float *)AxesMalloc((Widget)w, numft*sizeof(float)))) return;
    if( !(phase = (float *)AxesMalloc((Widget)w, numft*sizeof(float)))) return;

    midpoint = numft/2;
    for(int i = 0; i < nf; i++) {
	for(int j = 0; j < numft; j++) {
	    values[j] = v.at(j)->xPow[i];
	}
	qsort(values, numft, sizeof(float), cmp);
	median[i] = values[midpoint];
	phase[i] = 0.;
    }
    Free(values);

    ft->entries.push_back(new FtEntry(w, f0, "median", median, phase,
		StringToPixel((Widget)w, (char *)"grey")));
}

static void
ComputePercentiles(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;
    char label[20];
    int nf, numft, plo, phi;
    vector<FtData *> v;
    FtData *f, *f0;
    float *low, *high, *values, *phase;

    for(int i = 0; i < (int)ft->ftdata.size(); i++) {
	f = ft->ftdata[i];
	if((int)v.size() == 0) {
	    v.push_back(f);
	    f0 = f;
	}
	else if(f->nf == f0->nf) {
	    v.push_back(f);
	}
    }
    if((int)v.size() <= 1) return;
    nf = f0->nf;
    numft = (int)v.size();

    if( !(low = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(high = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(values = (float *)AxesMalloc((Widget)w, numft*sizeof(float)))) return;
    if( !(phase = (float *)AxesMalloc((Widget)w, numft*sizeof(float)))) return;

    plo = (int)(numft * (double)w->ft_plot.percentile1/100.);
    phi = (int)(numft * (double)w->ft_plot.percentile2/100.);
    if(plo < 0) plo = 0;
    if(plo >= numft) plo = numft-1;
    if(phi < 0) phi = 0;
    if(phi >= numft) phi = numft-1;

    for(int i = 0; i < nf; i++) {
	for(int j = 0; j < numft; j++) {
	    values[j] = v.at(j)->xPow[i];
	}
	qsort(values, numft, sizeof(float), cmp);
	low[i] = values[plo];
	high[i] = values[phi];
	phase[i] = 0.;
    }
    Free(values);

    snprintf(label, sizeof(label), "%d%%", w->ft_plot.percentile1);
    ft->entries.push_back(new FtEntry(w, f0, label, low, phase,
		StringToPixel((Widget)w, (char *)"green")));
    snprintf(label, sizeof(label), "%d%%", w->ft_plot.percentile2);
    ft->entries.push_back(new FtEntry(w, f0, label, high, phase,
		StringToPixel((Widget)w, (char *)"green")));
}

/* Noise models from Perterson, Observation and modeling of seismic background
 * noise, U.S. Geol. Surv. Tech. Rept., 93-322, 1-95, 1993.
 * http://ehp2-earthquake.wr.usgs.gov/regional/asl/pubs/files/ofr93-322.pdf
 *
 * NLNM: new low-noise model
 *	acceleration = a + b log10(p) dB referred to 1 (m/sec**2)**2 / Hz
 *      velocity = NLNM.acc + 20.0 log10(P/2pi) dB ref 1 (m/sec)**2 / Hz
 *      displacement = NLNM.acc + 20.0 log10(P*P/(4pi**2)) dB ref 1 m**2/Hz
 * NHNM: new high-noise model
 */
static void
ComputeNoise(FtPlotWidget w, const char *type)
{
    FtPlotPart *ft = &w->ft_plot;
    int i, j, nf;
    double db, per;
    float *noise, *xnoise;

    typedef struct
    {
	float p, a, b;
    } NoiseModel;

    int n_nlnm = 22;
    NoiseModel nlnm[22] = 
    {
	{0.10,           -162.36,        5.64},
	{0.17,           -166.7,         0.00},
	{0.40,           -170.00,        -8.30},
	{0.80,           -166.40,        28.90},
	{1.24,           -168.60,        52.48},
	{2.40,           -159.98,        29.81},
	{4.30,           -141.10,        0.00},
	{5.00,           -71.36,         -99.77},
	{6.00,           -97.26,         -66.49},
	{10.00,          -132.18,        -31.57},
	{12.00,          -205.27,        36.16},
	{15.60,          -37.65,         -104.33},
	{21.90,          -114.37,        -47.10},
	{31.60,          -160.58,        -16.28},
	{45.00,          -187.50,        0.00},
	{70.00,          -216.47,        15.70},
	{101.00,         -185.00,        0.00},
	{154.00,         -168.34,        -7.61},
	{328.00,         -217.43,        11.90},
	{600.00,         -258.28,        26.60},
	{10000.00,       -346.88,        48.75},
	{100000.00,      -346.88,        48.75}
    };

    int n_nhnm = 12;
    NoiseModel nhnm[12] = 
    {
	{0.10,           -108.73,        -17.23},
	{0.22,           -150.34,        -80.50},
	{0.32,           -122.31,        -23.87},
	{0.80,           -116.85,        32.51},
	{3.80,           -108.48,        18.08},
	{4.60,           -74.66,         -32.95},
	{6.30,             0.66,         -127.18},
	{7.90,           -93.37,         -22.42},
	{15.40,          73.54,          -162.98},
	{20.00,          -151.52,        10.01},
	{354.80,         -206.66,        31.63},
	{100000.,         -206.66,        31.63}
    };

    for(i = 0; i < (int)ft->entries.size(); i++) {
	if(ft->entries[i]->type == FT_NOISE
		&& !strcasecmp(ft->entries[i]->chan, type))
	{
	    ft->entries[i]->scaleFt();
	    return;
	}
    }

    nf = (!strcasecmp(type, "NLNM")) ? n_nlnm : n_nhnm;

    if( !(noise = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;
    if( !(xnoise = (float *)AxesMalloc((Widget)w, nf*sizeof(float))) ) return;

    if(!strcasecmp(type, "NLNM"))
    {
	for(i = 0; i < n_nlnm; i++)
	{
	    j = n_nlnm-1-i;
	    per = nlnm[j].p;
	    // displacement power dB. Add 180 [(10**9)**2] to convert m to nm
	    db = nlnm[j].a + nlnm[j].b * Log10(per) +
			20.*Log10(per*per / (4 * M_PI * M_PI)) + 180.;
	    noise[i] = pow((double)10., (double)0.1*db);
	    xnoise[i] = 1./per;
	}
    }
    else // NHNM
    {
	for(i = 0; i < n_nhnm; i++)
	{
	    j = n_nhnm-1-i;
	    per = nhnm[j].p;
	    // displacement power dB. Add 180 [(10**9)**2] to convert m to nm
	    db = nhnm[j].a + nhnm[j].b * Log10(per) +
			20.*Log10(per*per / (4 * M_PI * M_PI)) + 180.;
	    noise[i] = pow((double)10., (double)0.1*db);
	    xnoise[i] = 1./per;
	}
    }
    NoiseEntry *entry = new NoiseEntry(w);
    entry->data_fg = StringToPixel((Widget)w, (char *)"black");
    entry->nf = nf;
    entry->selectable = false;
    entry->derived_spectrum = true;
    entry->type = FT_NOISE;
    strncpy(entry->sta, "noise", sizeof(entry->sta));
    strncpy(entry->chan, type, sizeof(entry->chan));
    entry->x = (float *)malloc(nf*sizeof(float));
    entry->y = (float *)malloc(nf*sizeof(float));
    entry->xnoise = xnoise;
    entry->xPow = noise;
    entry->scaleFt();

    ft->entries.push_back(entry);
}

static int
cmp(const void *A, const void *B)
{
    float *i = (float *)A;
    float *j = (float *)B;
    if (*i < *j) {
	return(-1);
    }
    else if (*i > *j) {
	return(1);
    }
    else {
	return(0);
    }
}

void
FtPlotWait(FtPlotWidget w)
{
    w->ft_plot.wait = True;
}

void
FtPlotContinue(FtPlotWidget w)
{
    w->ft_plot.wait = False;
    if(w->axes.use_pixmap && XtIsRealized((Widget)w)) {
	XCopyArea(XtDisplay(w), w->axes.pixmap, XtWindow(w), w->axes.axesGC, 0,
			0, w->core.width, w->core.height, 0, 0);
    }
}

int
FtPlotNumSelected(FtPlotWidget w)
{
    int i, num;

    num = 0;
    for(i = 0; i < (int)w->ft_plot.entries.size(); i++) {
	if(w->ft_plot.entries[i]->selected) num++;
    }
    return num;
}

void
FtPlotInput(FtPlotWidget w, const char *sta, const char *chan, int npts,
		float *y, double dt, Pixel fg, GTimeSeries *ts, double time)
{
    if(npts <= 0 || y == NULL || dt <= 0.) {
	cerr << "FtPlotInput: Bad arguments." << endl;
	return;
    }

    vector<Response *> *v = BasicSource::getResponse(ts);

    bool remove_calib = (ts->getMethod("CalibData") && ts->segment(0)->calib());

    FtData *ftd = new FtData(sta, chan, npts, y, time, dt, remove_calib,
		ts->segment(0)->calib(), ts->segment(0)->calper(), fg, v);

    DoFt(w, ftd);

    w->ft_plot.ftdata.push_back(ftd);
}

void
FtPlotOverlay(FtPlotWidget w, int npts, float *x, float *y, Boolean display)
{
    Free(w->ft_plot.overlay_x);
    Free(w->ft_plot.overlay_y);
    w->ft_plot.overlay_npts = npts;
    if( !(w->ft_plot.overlay_x = (float *)AxesMalloc((Widget)w,
		w->ft_plot.overlay_npts*sizeof(float))) ) return;
    if( !(w->ft_plot.overlay_y = (float *)AxesMalloc((Widget)w,
		w->ft_plot.overlay_npts*sizeof(float))) ) return;
    memcpy(w->ft_plot.overlay_x, x, w->ft_plot.overlay_npts*sizeof(float));
    memcpy(w->ft_plot.overlay_y, y, w->ft_plot.overlay_npts*sizeof(float));
    w->ft_plot.overlay_display = display;
    if(display) {
	_AxesRedisplayAxes((AxesWidget)w);
	_FtPlotRedisplay(w, True);
	_AxesRedisplayXor((AxesWidget)w);
    }
}

int	
FtPlotGetData(FtPlotWidget w, FtPlotData **ft_list, Boolean selected_only)
{
    int i, n;
    FtEntry *entry;
    FtPlotData *ftl = NULL;
    char fstype[5];

    if( !(ftl = (FtPlotData *)AxesMalloc((Widget)w,
	w->ft_plot.entries.size()*sizeof(FtPlotData))) ) return(0);

    if (w->ft_plot.mode == FT_AMP) {
	stringcpy(fstype, "amp", sizeof(fstype));
    }
    else if (w->ft_plot.mode == FT_POWER) {
	stringcpy(fstype, "power", sizeof(fstype));
    }
    /* ZZ
    else if (w->ft_plot.mode == FT_PHASE)
	stringcpy(fstype, "phase", sizeof(fstype));
    ZZ */

    n = 0;
    for(i = 0; i < (int)w->ft_plot.entries.size(); i++)
    {
	entry = w->ft_plot.entries[i];

	if(!selected_only || entry->selected)
	{
	    ftl[n].id = entry->id;
	    stringcpy(ftl[n].sta, entry->sta, sizeof(ftl[n].sta));
	    stringcpy(ftl[n].chan, entry->chan, sizeof(ftl[n].chan));
	    stringcpy(ftl[n].taper, entry->taper, sizeof(ftl[n].taper));
	    stringcpy(ftl[n].fstype, fstype, sizeof(ftl[n].fstype));
	    if (entry->do_rsp == 0) {
		stringcpy(ftl[n].do_rsp, "0", sizeof(ftl[n].do_rsp));
	    }
	    else {
		stringcpy(ftl[n].do_rsp, "1", sizeof(ftl[n].do_rsp));
	    }
	    ftl[n].time = entry->time;
	    ftl[n].maxf = (entry->nf-1)*entry->df;
	    ftl[n].tlen = (entry->npts-1)*entry->dt;
	    ftl[n].nf = entry->nf;
	    ftl[n].df = entry->df;
	    ftl[n].taper_length = w->ft_plot.taper_length;
	    ftl[n].taper_min_pts = w->ft_plot.taper_min_pts;
	    ftl[n].taper_max_pts = w->ft_plot.taper_max_pts;
	    ftl[n].npts = entry->npts;
	    ftl[n].npts = entry->npts;
	    ftl[n].npts = entry->npts;
	    ftl[n].smoothvalue = entry->smoothvalue;

	    ftl[n].dB = entry->xPow;
	    ftl[n].phase = entry->phase;
	    ftl[n].t = entry->t;
	    n++;
	}
    }
    if(n == 0) {
	Free(ftl);
    }
    *ft_list = ftl;
    return(n);
}

/** 
 * 
 */
void
_FtPlotRedraw(FtPlotWidget w)
{
    if(XtIsRealized((Widget)w)) {
	XSetClipRectangles(XtDisplay(w), w->ft_plot.gc, 0, 0,
			&w->axes.clip_rect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(w), w->ft_plot.gc_invert, 0, 0,
			&w->axes.clip_rect, 1, Unsorted);
    }
    for(int i = 0; i < (int)w->ft_plot.entries.size(); i++)
    {
	DrawEntry(w, &w->axes.d, w->ft_plot.entries[i],
				&w->ft_plot.entries[i]->s);
    }
    if(w->ft_plot.point_x >= 0) {
	DrawRectangles(w, w->axes.mag_invertGC, w->ft_plot.point_recs, 2);
	w->ft_plot.point_x = -1;
    }
}

static void
DoFt(FtPlotWidget w, FtData *ftd)
{
    int i, j, shift, n2;
    double re=0., im=0., fac;
    double x, *data;
    float *taper;
    float overlap;
    double area_ratio;

    if(w->ft_plot.windows <= 0) return;

    overlap = 1.0 - w->ft_plot.overlap * 0.01;
    ftd->overlap = w->ft_plot.overlap;
    ftd->winpts = (int)(ftd->npts / (1 + overlap*(w->ft_plot.windows - 1)));

    if( !(taper = (float *)AxesMalloc((Widget)w, ftd->winpts*sizeof(float))) )
    {
	return;
    }

    for(i = 0; i < ftd->winpts; i++) taper[i] = 1.;

    if(w->ft_plot.taper == HANN_TAPER)
    {
	Taper_hann(taper, ftd->winpts);
	stringcpy(ftd->taper, "hanning", sizeof(ftd->taper));
    }
    else if(w->ft_plot.taper == HAMM_TAPER)
    {
	Taper_hamm(taper, ftd->winpts);
	stringcpy(ftd->taper, "hamming", sizeof(ftd->taper));
    }
    else if(w->ft_plot.taper == PARZEN_TAPER)
    {
	Taper_parzen(taper, ftd->winpts);
	stringcpy(ftd->taper, "parzen", sizeof(ftd->taper));
    }
    else if(w->ft_plot.taper == WELCH_TAPER)
    {
	Taper_welch(taper, ftd->winpts);
	stringcpy(ftd->taper, "welch", sizeof(ftd->taper));
    }
    else if(w->ft_plot.taper == BLACKMAN_TAPER)
    {
	Taper_blackman(taper, ftd->winpts);
	stringcpy(ftd->taper, "blackman", sizeof(ftd->taper));
    }
    else if(w->ft_plot.taper == COSINE_TAPER)
    {
	int taper_npts;
	double length = w->ft_plot.taper_length;

	for(i = 0; i < ftd->winpts; i++) taper[i] = 1.;

	taper_npts = (int)(ftd->winpts*w->ft_plot.taper_length);
	if(taper_npts > w->ft_plot.taper_max_pts) {
	    length = (double)w->ft_plot.taper_max_pts/ftd->winpts;
	}
	else if(taper_npts < w->ft_plot.taper_min_pts
		&& ftd->winpts > w->ft_plot.taper_min_pts) {
	    length = (double)w->ft_plot.taper_min_pts/ftd->winpts;
	}
	if(length > .50) length = .50;

	Taper_cosine(taper, ftd->winpts, length, length);
	stringcpy(ftd->taper, "cosine", sizeof(ftd->taper));
    }
    else {
	stringcpy(ftd->taper, "none", sizeof(ftd->taper));
    }
    ftd->taper_length = w->ft_plot.taper_length;
    ftd->taper_min_pts = w->ft_plot.taper_min_pts;
    ftd->taper_max_pts = w->ft_plot.taper_max_pts;

    area_ratio = 0.0;
    for(i = 0; i < ftd->winpts; i++) {
	area_ratio += taper[i];
    }
    area_ratio = (area_ratio) ? ftd->winpts/area_ratio : 1.;

    for(ftd->np2 = 2; ftd->np2 < ftd->winpts; ftd->np2 *= 2);
    ftd->nf = ftd->np2/2 + 1;
    ftd->df = 1./(double)(ftd->np2*ftd->dt);

    ftd->nfft = ftd->np2;

    Free(ftd->xPow);
    Free(ftd->phase);
    ftd->xPow =  (float *)malloc(ftd->nf*sizeof(float));
    ftd->phase = (float *)malloc(ftd->nf*sizeof(float));

    for(i = 0; i < ftd->nf; i++) {
	ftd->xPow[i] = 0.;
	ftd->phase[i] = 0.;
    }

    if( !(data = (double *)AxesMalloc((Widget)w, ftd->np2*sizeof(double))) ) {
	Free(taper);
	return;
    }
    fac = 1./(DEG_TO_RADIANS*w->ft_plot.windows);
    n2 = ftd->np2/2;

    for(j = 0; j < w->ft_plot.windows; j++)
    {
	shift = (int)(j * overlap*ftd->winpts);

	for(i = 0; i < ftd->winpts; i++) data[i] = ftd->t[ i + shift ];
	for(i = ftd->winpts; i < ftd->np2; i++) data[i] = 0.;

	/* demean and taper*/
	x = (w->ft_plot.demean) ? ftd->mean : 0.;

	for(i = 0; i < ftd->winpts; i++) {
	    data[i] = (data[i] - x)*taper[i];
	}

	gsl_fft_real_radix2_transform(data, 1, ftd->np2);

	for(i = 0; i < ftd->nf; i++)
	{
	    if(i == 0 || i == n2) im = 0.;
	    else im = data[ftd->np2-i];
	    re = data[i];
	    ftd->xPow[i] += (re*re + im*im);

	    /* the average phase */
	    ftd->phase[i] += (re != 0. && im != 0.) ? atan2(im,re)*fac : 0.;
	}
    }
    Free(data);
    Free(taper);

    /* Scale for power
     */
    x = 2.*ftd->dt/(ftd->np2*w->ft_plot.windows);
    for(i = 0; i < ftd->nf; i++) {
	ftd->xPow[i] *= x;
    }

    /* Compensate spectrum power for the taper loss.
     */
    for(i = 0; i < ftd->nf; i++) {
	ftd->xPow[i] *= area_ratio*area_ratio;
    }
}

static void
DoSmoothing(FtPlotWidget w, FtEntry *entry, float *xPow)
{
    int i, n, nd2;
    float *tmp = NULL;
    double d;

    entry->smoothvalue = w->ft_plot.smoothing_width;
    if((n = (int)(w->ft_plot.smoothing_width/entry->df + .5)) > 1)
    {
	if( !(tmp = (float *)AxesMalloc((Widget)w, entry->nf*sizeof(float))) ) {
	    return;
	}

	memcpy(tmp, xPow, entry->nf*sizeof(float));

	// make sure n is an odd number
	if (n%2 == 0) n++;

	nd2 = n/2;

	// for samps before full smoothing window can be applied 
	for(i = 1; i <= nd2; i++) {
	    xPow[i] = get_mean(tmp, i, i-1);
	}
	d = 1./(double)n;
	for(i = nd2+1; i < entry->nf - nd2; i++)
	{
	    xPow[i] = xPow[i-1] + d*(tmp[i+nd2] - tmp[i-nd2-1]);
	}
	entry->nf -= nd2;
	Free(tmp);
    }
    else
    {
	_AxesWarn((AxesWidget)w,
		"Mininum effective smoothing frequency for %s/%s is %.1f Hz.",
		entry->sta, entry->chan, 3*entry->df);
    }
}

static double
get_mean(float *dB, int index, int nd2)
{
    int i;
    double mean;

    mean = 0.0;
    for(i = index - nd2; i <= index + nd2; i++)
    {
	mean += dB[i];
    }
    return(mean/((nd2 * 2) + 1));
}

static void
DoInstrumentCorr(FtPlotWidget w, FtEntry *entry, float *xPow)
{
    int i;
    double *real = NULL, *imag = NULL;
    double freq1, freqn, calib;

    if(entry->type == FT_NOISE) return;

    entry->do_rsp = False;
    if(!w->ft_plot.instrument_corr) return;

    if(entry->rsp == NULL) {
	_AxesWarn((AxesWidget)w, "No instrument response for %s/%s\n",
			entry->sta, entry->chan);
	return;
    }

    freq1 = 0.0;
    freqn = entry->df * (entry->nf - 1);
    if( !(real = (double *)AxesMalloc((Widget)w, entry->nf*sizeof(double))) ) {
	return;
    }
    if( !(imag = (double *)AxesMalloc((Widget)w, entry->nf*sizeof(double))) ) {
	return;
    }

    Response::compute(entry->rsp, freq1, freqn, entry->calib, entry->calper,
			entry->nf, real, imag);

    entry->do_rsp = True;

    calib = (entry->calper > 0. && entry->calib != 0.) ? entry->calib : 1.;
    for(i = 0; i < entry->nf; i++)
    {
	double resp = real[i]*real[i] + imag[i]*imag[i];
	if(resp) xPow[i] *= calib*calib/resp;
    }
    Free(real);
    Free(imag);
}

static void
DrawEntry(FtPlotWidget w, DrawStruct *d, FtEntry *entry, SegArray *s)
{
    int i, i1, i2, n;
    double x0, y0;
    double xbeg;

    if(entry->nf <= 0 || !entry->on)
    {
	return;
    }
    if(entry->x == NULL)
    {
	i1 = (int)(w->axes.x1[w->axes.zoom]/entry->df);
	i2 = (int)(w->axes.x2[w->axes.zoom]/entry->df) + 1;
    }
    else
    {
	i1 = findx(entry->x, entry->nf, w->axes.x1[w->axes.zoom]);
	i2 = findx(entry->x, entry->nf, w->axes.x2[w->axes.zoom]);
	i2++;
    }
    if(i1 < 0) i1 = 0;
    else if(i1 >= entry->nf) i1 = entry->nf - 1;
    if(i1 == 0 && !w->ft_plot.draw_dc && entry->type != FT_NOISE) {
	i1++;
	if(i1 >= entry->nf) i1 = entry->nf - 1;
    }
    if(i2 < 0) i2 = 0;
    else if(i2 >= entry->nf) i2 = entry->nf - 1;
    n = i2 - i1 + 1;
    entry->i1 = i1;
    entry->i2 = i2;

    if(entry->x == NULL)
    {
	xbeg = i1*entry->df;
	plotxd(n, NULL, entry->y+i1, 0., d->unscalex, -d->unscaley, 0.,
		entry->df, &entry->r);
    }
    else
    {
	xbeg = entry->x[i1];
	plotxd(n, entry->x+i1, entry->y+i1, 0., d->unscalex, -d->unscaley,
		-xbeg, 0., &entry->r);
    }

    s->nsegs = entry->r.nsegs;
    ALLOC(s->nsegs*sizeof(DSegment), s->size_segs, s->segs, DSegment, 0);
    x0 = unscale_x(d, xbeg);
    y0 = unscale_y(d, 0.);
    if(entry->fill)
    {
	FillSegments(w, d, entry, s, x0, y0);
    }
    else if(x0 + entry->r.xmin < -S_LIM || x0 + entry->r.xmax > S_LIM ||
		y0 + entry->r.ymin < -S_LIM || y0 + entry->r.ymax > S_LIM)
    {
	/* do the clipping here. X can't handle it. */
	_AxesDoClipping((AxesWidget)w, entry->r.nsegs, entry->r.segs,
					x0, y0, s);
    }
    else
    {
	for(i = 0; i < entry->r.nsegs; i++)
	{
	    s->segs[i].x1 = (short)(x0 + entry->r.segs[i].x1);
	    s->segs[i].x2 = (short)(x0 + entry->r.segs[i].x2);
	    s->segs[i].y1 = (short)(y0 + entry->r.segs[i].y1);
	    s->segs[i].y2 = (short)(y0 + entry->r.segs[i].y2);
	}
    }
    Free(entry->r.segs);
    entry->r.segs = (RSeg *)NULL;
    entry->r.nsegs = 0;
}

static void
FillSegments(FtPlotWidget w, DrawStruct *d, FtEntry *entry, SegArray *seg_array,
		double x0, double y0)
{
	AxesPart *ax = &w->axes;
	Boolean y_reverse = False;
	int i, j, n, ybot, x, x1, x2, ix1, iy1, ix2, iy2;
	double slope, y, ymin=0., ymax=0., clipx1, clipx2, clipy1, clipy2;
	RSeg *segs, *seg = NULL, *s;

	ix1 = unscale_x(d, ax->x1[ax->zoom]);
	iy1 = unscale_y(d, ax->y1[ax->zoom]);
        ix2 = unscale_x(d, ax->x2[ax->zoom]);
	iy2 = unscale_y(d, ax->y2[ax->zoom]);
	if(iy2 < iy1)
	{
	    y_reverse = True;
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
	clipx1 = ix1+1 - x0;
	clipx2 = ix2-1 - x0;
	clipy1 = iy1+1 - y0;
	clipy2 = iy2-1 - y0;

	seg = (RSeg *)AxesMalloc((Widget)w, sizeof(RSeg));

	segs = entry->r.segs;
	n = 0;
	for(i = 0; i < entry->r.nsegs; i++)
	{
	    seg = (RSeg *)AxesRealloc((Widget)w, seg, (n+1)*sizeof(RSeg));
	    s = &seg[n];
	    if(segs[i].x1 == segs[i].x2)
	    {
		if(n > 0 && segs[i].x1 == seg[n-1].x1) {
		    if(seg[n-1].y1 > segs[i].y1) seg[n-1].y1 = segs[i].y1;
		    if(seg[n-1].y1 > segs[i].y2) seg[n-1].y1 = segs[i].y2;
		}
		else {
		    s->x1 = segs[i].x1;
		    s->y1 = (segs[i].y1 < segs[i].y2) ? segs[i].y1 : segs[i].y2;
		    s->x2 = segs[i].x1;
		    n++;
		}
	    }
	    else
	    {
		if(n > 0 && segs[i].x1 == seg[n-1].x2) {
		    if(seg[n-1].y1 > segs[i].y1) seg[n-1].y1 = segs[i].y1;
		}
		else {
		    s->x1 = segs[i].x1;
		    s->y1 = segs[i].y1;
		    s->x2 = segs[i].x1;
		    n++;
		}
		slope = (double)(segs[i].y2 - segs[i].y1)/
			(double)(segs[i].x2 - segs[i].x1);
		x1 = (int)(segs[i].x1 + .5);
		x2 = (int)(segs[i].x2 + .5);
		for(x = x1+1; x < x2; x++) {
		    if(x >= clipx1 && x <= clipx2) {
			seg = (RSeg *)AxesRealloc((Widget)w, seg,
					(n+1)*sizeof(RSeg));
			s = &seg[n++];
			s->x1 = x;
			y = segs[i].y1 + (x - segs[i].x1)*slope;
			s->y1 = y;
			s->x2 = x;
		    }
		}
		if(segs[i].x2 >= clipx1 && segs[i].x2 <= clipx2) {
		    seg = (RSeg *)AxesRealloc((Widget)w,seg,(n+1)*sizeof(RSeg));
		    s = &seg[n++];
		    s->x1 = segs[i].x2;
		    s->y1 = segs[i].y2;
		    s->x2 = segs[i].x2;
		}
	    }
	}

	/* find screen min,max for entry->y and use this for ybot
	 */
	if(entry->nf > 0) {
	    ymin = ymax = entry->y[0]*d->unscaley;
	}
	for(i = 1; i < entry->nf; i++) {
	    y = entry->y[i]*d->unscaley;
	    if(ymin > y) ymin = y;
	    if(ymax < y) ymax = y;
	}

	if(y_reverse) {
	    ybot = 0;
	    if(ymax > ybot) ybot = (int)ymax;
	}
	else {
	    ybot = iy2;
	    if(ymin < ybot) ybot = (int)ymin;
	}
	for(i = j = 0; i < n; i++) {
	    seg[i].y2 = ybot;
	    if(seg[i].x1 < clipx1 || seg[i].x1 > clipx2) continue;
	    if(seg[i].y1 < clipy1 && seg[i].y2 < clipy1) continue;
	    if(seg[i].y1 > clipy2 && seg[i].y2 > clipy2) continue;

	    if(seg[i].y1 < clipy1) seg[i].y1 = clipy1;
	    if(seg[i].y2 < clipy1) seg[i].y2 = clipy1;
	    if(seg[i].y1 > clipy2) seg[i].y1 = clipy2;
	    if(seg[i].y2 > clipy2) seg[i].y2 = clipy2;
	    seg[j++] = seg[i];
	}
	seg_array->nsegs = j;
	ALLOC(seg_array->nsegs*sizeof(DSegment), seg_array->size_segs,
		seg_array->segs, DSegment, 0);

	for(i = 0; i < seg_array->nsegs; i++)
	{
	    seg_array->segs[i].x1 = (short)(x0 + seg[i].x1);
	    seg_array->segs[i].x2 = (short)(x0 + seg[i].x2);
	    seg_array->segs[i].y1 = (short)(y0 + seg[i].y1);
	    seg_array->segs[i].y2 = (short)(y0 + seg[i].y2);
	}
	Free(seg);
}

Widget
FtPlotCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
    return (XtCreateWidget(name, ftPlotWidgetClass, parent, arglist, argcount));
}

static void
FtPlotZoomZero(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;
    int i;
    bool use_noise = true;
    double xmin, xmax, ymin, ymax;
    double x_min, x_max, y_min, y_max;
    FtEntry *entry;

    xmin = w->axes.x1[w->axes.zoom];
    xmax = w->axes.x2[w->axes.zoom];
    ymin = w->axes.y1[w->axes.zoom];
    ymax = w->axes.y2[w->axes.zoom];

    w->axes.zoom = 0;

    for(i = 0; i < (int)ft->entries.size(); i++) {
	if(ft->entries[i]->type != FT_NOISE && ft->entries[i]->on) {
	    use_noise = false;
	}
    }

    for(i = 0; i < (int)ft->entries.size() && (!ft->entries[i]->on
		|| (ft->entries[i]->type == FT_NOISE && !use_noise)); i++);
    if(i < (int)ft->entries.size())
    {
	x_min = ft->entries[i]->x_min;
	x_max = ft->entries[i]->x_max;
	y_min = ft->entries[i]->y_min;
	y_max = ft->entries[i]->y_max;
	for(; i < (int)ft->entries.size(); i++) if(ft->entries[i]->on
		&& (ft->entries[i]->type != FT_NOISE || use_noise))
	{
	    entry = ft->entries[i];
	    if(x_min > entry->x_min) x_min = entry->x_min;
	    if(x_max < entry->x_max) x_max = entry->x_max;
	    if(y_min > entry->y_min) y_min = entry->y_min;
	    if(y_max < entry->y_max) y_max = entry->y_max;
	}
	x_min -= .1*(x_max - x_min);
	x_max += .1*(x_max - x_min);
	y_min -= .1*(y_max - y_min);
	y_max += .1*(y_max - y_min);
    }
    else {
	x_min = w->axes.x_min;
	x_max = w->axes.x_max;
	y_min = w->axes.y_min;
	y_max = w->axes.y_max;
    }
    w->axes.x1[0] = x_min;
    w->axes.x2[0] = x_max;
    w->axes.y1[0] = y_min;
    w->axes.y2[0] = y_max;
    if( xmin != w->axes.x1[0] || xmax != w->axes.x2[0] ||
	ymin != w->axes.y1[0] || ymax != w->axes.y2[0])
    {
	_AxesRedraw((AxesWidget)w);
	_FtPlotRedraw(w);
	if(!w->axes.redisplay_pending)
	{
	    _AxesRedisplayAxes((AxesWidget)w);
	    _FtPlotRedisplay(w, True);
	    _AxesRedisplayXor((AxesWidget)w);
	}
    }
}

void
FtPlotSave(FtPlotWidget w)
{
    int i;
    FtEntry *entry;

    for(i = 0; i < (int)w->ft_plot.entries.size(); i++)
	if(w->ft_plot.entries[i]->type == FT_TMP
		&& w->ft_plot.entries[i]->selected)
    {
	entry = w->ft_plot.entries[i];
	entry->type = FT_PERM;
	entry->scaleX();
	DrawEntry(w, &w->axes.d, entry, &entry->s);
    }
    _AxesRedisplayAxes((AxesWidget)w);
    _FtPlotRedisplay(w, True);
    _AxesRedisplayXor((AxesWidget)w);
}

void
FtPlotDeleteSelected(FtPlotWidget w)
{
    FtPlotPart *ft = &w->ft_plot;

    for(int i = (int)ft->entries.size()-1; i >= 0; i--)
	if(ft->entries[i]->selected)
    {
	delete ft->entries[i];
	ft->entries.erase(ft->entries.begin()+i);
    }
    _AxesRedisplayAxes((AxesWidget)w);
    _FtPlotRedisplay(w, True);
    _AxesRedisplayXor((AxesWidget)w);
}

static void
FtPlotHardCopy(AxesWidget widget, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, PrintParam *p)
{
    FtPlotWidget w = (FtPlotWidget)widget;
    FtPlotPart *ft = &w->ft_plot;
    AxesPart *ax = &w->axes;
    int i, m;
    FtEntry *entry;
    SegArray s;

    s.segs = NULL;
    s.size_segs = 0;

    fprintf(fp, "N %d %d M\n",
		unscale_x(d, ax->x1[ax->zoom]),
		unscale_y(d, ax->y2[ax->zoom]));
    fprintf(fp, "%d %d d\n",
		unscale_x(d, ax->x2[ax->zoom]),
		unscale_y(d, ax->y2[ax->zoom]));
    fprintf(fp, "%d %d d\n",
		unscale_x(d, ax->x2[ax->zoom]),
		unscale_y(d, ax->y1[ax->zoom]));
    fprintf(fp, "%d %d d\n",
		unscale_x(d, ax->x1[ax->zoom]),
		unscale_y(d, ax->y1[ax->zoom]));
    fprintf(fp, "%d %d d\n",
		unscale_x(d, ax->x1[ax->zoom]),
		unscale_y(d, ax->y2[ax->zoom]));
    fprintf(fp, "closepath clip\n");

    for(m = 0; m < (int)ft->entries.size(); m++)
	if(ft->entries[m]->nf > 0 && ft->entries[m]->on)
    {
	s.nsegs = 0;
	entry = ft->entries[m];
	/* if DOTS_PER_INCH is more that 300, the short ints in the
	 * SegArray might be too small.
	 */
	DrawEntry(w, d, entry, &s);

	fprintf(fp, "%d slw\n", p->data_linewidth);
	if(p->color) {
	    Pixel fg = entry->selected ? w->axes.select_fg : entry->data_fg;
	    AxesPostColor((AxesWidget)widget, fp, fg);
	}
	if(s.nsegs > 0) {
	    fprintf(fp, "%d %d M\n", s.segs[0].x1, s.segs[0].y1);
	    fprintf(fp, "%d %d D\n", s.segs[0].x2, s.segs[0].y2);
	}
	for(i = 1; i < s.nsegs; i++)
	{
	    if(s.segs[i].x1 != s.segs[i-1].x2 || s.segs[i].y1 != s.segs[i-1].y2)
	    {
		fprintf(fp, "%d %d M\n", s.segs[i].x1, s.segs[i].y1);
	    }
	    fprintf(fp, "%d %d D\n", s.segs[i].x2, s.segs[i].y2);
	}
    }
    Free(s.segs);
    fprintf(fp, "initclip\n");
}

static void
Motion(FtPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    FtPlotPart *ft = &w->ft_plot;
    AxesPart *ax = &w->axes;
    char label[200];
    int i, i1, i2, cursor_x, cursor_y, imin, x_min=0, y_min=0, x1, x2,
		dx=0, x, y, deli, n;
    double d, dmin=0., freq, secs;
    FtEntry *entry = NULL;

    cursor_x = ((XButtonEvent *)event)->x;
    cursor_y = ((XButtonEvent *)event)->y;

    _AxesMotion((AxesWidget)w, event, params, num_params);

    if(ax->cursor_info == NULL) return;

    if((entry = WhichEntry(w, cursor_x, cursor_y)) == NULL || entry->nf <= 0)
    {
	if(ft->point_x >= 0) {
	    DrawRectangles(w, ax->mag_invertGC, ft->point_recs, 2);
	    ft->point_x = -1;
	}
	return;
    }

    if(ax->cursor_info2 != NULL && entry->time != NULL_TIME) {
	timeEpochToString(entry->time, label, 200, GSE20);
	strcat(label, "    ");
	i = strlen(label);
	timeEpochToString((entry->npts-1)*entry->dt, label+i, 200, HMS);
	InfoSetText(ax->cursor_info2, label);
    }

    i1 = entry->i1;
    i2 = entry->i2;

    n = i2 - i1 + 1;

    if(n > 2*(ax->d.ix2 - ax->d.ix1)) {
	deli = n/(ax->d.ix2 - ax->d.ix1);
    }
    else {
	deli = 1;
    }

    imin = -1;
    if(entry->x != NULL)
    {
	if(entry->nf > i1) {
	    x = ax->d.ix1 + nint((entry->x[i1]-ax->d.sx1)*ax->d.unscalex);
	    y = ax->d.iy1 + nint((entry->y[i1]-ax->d.sy1)*ax->d.unscaley);
	    x -= cursor_x;
	    y -= cursor_y;
	    dmin = x*x + y*y;
	    imin = i1;
	}
	for(i = i1+deli; i <= i2; i += deli) {
	    x = ax->d.ix1 + nint((entry->x[i]-ax->d.sx1)*ax->d.unscalex);
	    y = ax->d.iy1 + nint((entry->y[i]-ax->d.sy1)*ax->d.unscaley);
	    x -= cursor_x;
	    y -= cursor_y;
	    d = x*x + y*y;
	    if(d < dmin) {
		dmin = d;
		imin = i;
	    }
	}
	if(imin >= 0) {
	    x_min = unscale_x(&ax->d, entry->x[imin]);
	    y_min = unscale_y(&ax->d, entry->y[imin]);
	}
	if(imin > 0) {
	    x1 = unscale_x(&ax->d, entry->x[imin]);
	    x2 = unscale_x(&ax->d, entry->x[imin-1]);
	    dx = abs(x2 - x1);
	}
	else if(entry->nf > imin + 1) {
	    x1 = unscale_x(&ax->d, entry->x[imin]);
	    x2 = unscale_x(&ax->d, entry->x[imin+1]);
	    dx = abs(x2 - x1);
	}
    }
    else
    {
	if(entry->nf > i1) {
	    x = ax->d.ix1 + nint((i1*entry->df-ax->d.sx1)*ax->d.unscalex);
	    y = ax->d.iy1 + nint((entry->y[i1]-ax->d.sy1)*ax->d.unscaley);
	    x -= cursor_x;
	    y -= cursor_y;
	    dmin = x*x + y*y;
	    imin = i1;
	}
	for(i = i1+deli; i <= i2; i += deli) {
	    x = ax->d.ix1 + nint((i*entry->df-ax->d.sx1)*ax->d.unscalex);
	    y = ax->d.iy1 + nint((entry->y[i]-ax->d.sy1)*ax->d.unscaley);
	    x -= cursor_x;
	    y -= cursor_y;
	    d = x*x + y*y;
	    if(d < dmin) {
		dmin = d;
		imin = i;
	    }
	}
	if(imin >= 0) {
	    x_min = unscale_x(&ax->d, imin*entry->df);
	    y_min = unscale_y(&ax->d, entry->y[imin]);
	}
	if(imin > 0) {
	    x1 = unscale_x(&ax->d, imin*entry->df);
	    x2 = unscale_x(&ax->d, (imin-1)*entry->df);
	    dx = abs(x2 - x1);
	}
	else if(entry->nf > imin + 1) {
	    x1 = unscale_x(&ax->d, imin*entry->df);
	    x2 = unscale_x(&ax->d, (imin+1)*entry->df);
	    dx = abs(x2 - x1);
	}
    }

    if(imin >= 0)
    {
	freq = secs = 0.;
	if(entry->type != FT_NOISE) {
	    if( !ft->xaxis_period ) {
		freq = imin*entry->df;
		secs = (freq) ? 1./freq : 0.;
	    }
	    else if(entry->x && !ax->a.log_x) {
		secs = entry->x[imin];
		freq = (secs) ? 1./secs : 0.;
	    }
	    else if(entry->x && ax->a.log_x) {
		secs = pow((double)10., (double)entry->x[imin]);
		freq = (secs) ? 1./secs : 0.;
	    }
	}
	else {
	    if( !ft->xaxis_period ) {
		if(!ax->a.log_x) {
		    freq = entry->x[imin];
		    secs = (freq) ? 1./freq : 0.;
		}
		else {
		    freq = pow((double)10., (double)entry->x[imin]);
		    secs = (freq) ? 1./freq : 0.;
		}
	    }
	    else {
		if(!ax->a.log_x) {
		    secs = entry->x[imin];
		    freq = (secs) ? 1./secs : 0.;
		}
		else {
		    secs = pow((double)10., (double)entry->x[imin]);
		    freq = (secs) ? 1./secs : 0.;
		}
	    }
	}
	snprintf(label, 200, "%s/%s  %.4fHZ  %.4fsec  %.3f", entry->sta,
		    entry->chan, freq, secs, entry->y[imin]);

	InfoSetText(ax->cursor_info, label);

	if(dx > 4)
	{
	    if(ft->point_x != x_min || ft->point_y != y_min)
	    {
		if(ft->point_x >= 0) {
		    DrawRectangles(w, ax->mag_invertGC, ft->point_recs, 2);
		}
		ft->point_recs[0].x = x_min - 2;
		ft->point_recs[0].y = y_min - 1;
		ft->point_recs[0].width = 3;
		ft->point_recs[0].height = 3;
		ft->point_recs[1].x = x_min - 3;
		ft->point_recs[1].y = y_min - 2;
		ft->point_recs[1].width = 5;
		ft->point_recs[1].height = 5;
		DrawRectangles(w, ax->mag_invertGC, ft->point_recs, 2);
		ft->point_x = x_min;
		ft->point_y = y_min;
	    }
	}
	else if(ft->point_x >= 0)
	{
	    DrawRectangles(w, ax->mag_invertGC, ft->point_recs, 2);
	    ft->point_x = -1;
	}
    }
    else if(ft->point_x >= 0) {
	DrawRectangles(w, ax->mag_invertGC, ft->point_recs, 2);
	ft->point_x = -1;
    }
}

static void
DrawRectangles(FtPlotWidget w, GC gc, XRectangle *recs, int nrecs)
{
    if( !XtIsRealized((Widget)w) ) return;
    XDrawRectangles(XtDisplay(w), XtWindow(w), gc, recs, nrecs);

    if(w->axes.use_pixmap)
    {
	XDrawRectangles(XtDisplay(w), w->axes.pixmap, gc, recs, nrecs);
    }
}


static FtEntry *
WhichEntry(FtPlotWidget w, int cursor_x,  int cursor_y)
{
    int i, j;
    double dmin;
    FtEntry *which_entry = NULL;

    if((int)w->ft_plot.entries.size() == 1) return w->ft_plot.entries[0];

    /* initialize dmin to a large value before seeking the minimim.
     */
    dmin = 2*((int)w->core.width*(int)w->core.width +
		  (int)w->core.height*(int)w->core.height);

    for(i = 0; i < (int)w->ft_plot.entries.size(); i++)
	    if(w->ft_plot.entries[i]->on)
    {
	FtEntry *entry = w->ft_plot.entries[i];
	for(j = 0; j < entry->s.nsegs; j++)
	{
	    double d = AxesClass::pointToLine((float)entry->s.segs[j].x1,
				(float)entry->s.segs[j].y1,
				(float)entry->s.segs[j].x2,
				(float)entry->s.segs[j].y2,
				(float)cursor_x, (float)cursor_y);

	    if(d < dmin) {
		dmin = d;
		which_entry = entry;
	    }
	}
    }
    return which_entry;
}

void
FtPlotWrite(Widget widget, FILE *fp)
{
    FtPlotWidget w = (FtPlotWidget)widget;
    int i;
    FtEntry *entry;

    i = (int)w->ft_plot.entries.size();
    fwrite(&i, sizeof(int), 1, fp);

    for(i = 0; i < (int)w->ft_plot.entries.size(); i++)
    {
	entry = w->ft_plot.entries[i];

	if(!FtPlotWriteEntry(w, entry->id, fp)) return;
    }
}

#define FWRITE(a,b,c,d) \
if(fwrite(a,b,c,d) != (size_t)c) {perror("FtPlotWriteEntry"); return False;}

Boolean
FtPlotWriteEntry(FtPlotWidget w, int id, FILE *fp)
{
#ifdef _HO_HO_HO_
	int i, j, num_resp;
	Boolean have_it;
	FtEntry *entry;

	for(i = 0; i < (int)w->ft_plot.entries.size()
		&& w->ft_plot.entry[i]->id != id; i++);

	if(i < (int)w->ft_plot.entries.size())
	{
	    entry = w->ft_plot.entry[i];

	    FWRITE(&entry->data_fg, sizeof(Pixel), 1, fp);
	    FWRITE(&entry->on, sizeof(Boolean), 1, fp);
	    FWRITE(entry->sta, 1, 8, fp);
	    FWRITE(entry->chan, 1, 9, fp);
	    FWRITE(&entry->calper, sizeof(float), 1, fp);
	    if(entry->rsp == NULL && entry->ts != NULL) {
		vector<Response *> *v = BasicSource::getResponse(entry->ts);
		if(v) {
		    entry->rsp = new vector<Response *>(*v);
		}
	    }
	    num_resp = (entry->rsp != NULL) ? (int)entry->rsp->size() : 0;
	    FWRITE(&num_resp, sizeof(int), 1, fp);
	    for(j = 0; j < num_resp; j++)
	    {
		    Response *r = (Response *)entry->rsp->at(j);
		    FWRITE(r->source, 1, 13, fp);
		    FWRITE(&r->seq_num, sizeof(int), 1, fp);
		    FWRITE(r->des, 1, 13, fp);
		    FWRITE(r->type, 1, 7, fp);
		    FWRITE(r->units, 1, 2, fp);
		    FWRITE(r->author, 1, 45, fp);
		    FWRITE(&r->a0, sizeof(double), 1, fp);
		    FWRITE(&r->npoles, sizeof(int), 1, fp);
		    FWRITE(&r->nzeros, sizeof(int), 1, fp);
		    FWRITE(r->pole, sizeof(FComplex), r->npoles, fp);
		    FWRITE(r->pole_err, sizeof(FComplex), r->npoles, fp);
		    FWRITE(r->zero, sizeof(FComplex), r->nzeros, fp);
		    FWRITE(r->zero_err, sizeof(FComplex), r->nzeros, fp);
		    FWRITE(&r->nfap, sizeof(int), 1, fp);
		    FWRITE(r->fap_f, sizeof(float), r->nfap, fp);
		    FWRITE(r->fap_a, sizeof(float), r->nfap, fp);
		    FWRITE(r->fap_p, sizeof(float), r->nfap, fp);
		    FWRITE(r->amp_error, sizeof(float), r->nfap, fp);
		    FWRITE(r->phase_error, sizeof(float), r->nfap, fp);
		    FWRITE(&r->samprate, sizeof(float), 1, fp);
		    FWRITE(&r->num_n, sizeof(int), 1, fp);
		    FWRITE(&r->num_d, sizeof(int), 1, fp);
		    FWRITE(r->fir_n, sizeof(float), r->num_n, fp);
		    FWRITE(r->fir_n_error, sizeof(float), r->num_n, fp);
		    FWRITE(r->fir_d, sizeof(float), r->num_d, fp);
		    FWRITE(r->fir_d_error, sizeof(float), r->num_d, fp);

		    have_it = (r->cal != NULL) ? True : False;
		    FWRITE(&have_it, sizeof(Boolean), 1, fp);
		    if(have_it)
		    {
			FWRITE(r->cal, sizeof(GSE_CAL), 1, fp);
		    }
	    }
	    FWRITE(&entry->npts, sizeof(int), 1, fp);
	    FWRITE(&entry->np2, sizeof(int), 1, fp);
	    FWRITE(&entry->nf, sizeof(int), 1, fp);
	    FWRITE(&entry->type, sizeof(int), 1, fp);

	    FWRITE(&entry->t_length, sizeof(int), 1, fp);
	    FWRITE(entry->t, sizeof(float), entry->t_length, fp);

	    FWRITE(&entry->amp_length, sizeof(int), 1, fp);
	    FWRITE(entry->amp,sizeof(float), entry->amp_length, fp);

	    FWRITE(&entry->phase_length, sizeof(int), 1, fp);
	    FWRITE(entry->phase, sizeof(float), entry->phase_length, fp);

	    FWRITE(&entry->x_length, sizeof(int), 1, fp);
	    FWRITE(entry->x, sizeof(float), entry->x_length, fp);

	    j = 0;
	    if(entry->y == entry->amp) j = 1;
	    else if(entry->y == entry->phase) j = 2;
	    FWRITE(&j, sizeof(int), 1, fp);

	    FWRITE(&entry->db_length, sizeof(int), 1, fp);
	    FWRITE(entry->xPow, sizeof(float), entry->db_length, fp);

	    FWRITE(&entry->time, sizeof(double), 1, fp);
	    FWRITE(&entry->dt, sizeof(double), 1, fp);
	    FWRITE(&entry->df, sizeof(double), 1, fp);
	    FWRITE(&entry->x_min, sizeof(double), 1, fp);
	    FWRITE(&entry->x_max, sizeof(double), 1, fp);
	    FWRITE(&entry->y_min, sizeof(double), 1, fp);
	    FWRITE(&entry->y_max, sizeof(double), 1, fp);
	    FWRITE(&entry->amp_y_min, sizeof(double), 1, fp);
	    FWRITE(&entry->amp_y_max, sizeof(double), 1, fp);
	    FWRITE(&entry->phase_y_min, sizeof(double), 1, fp);
	    FWRITE(&entry->phase_y_max, sizeof(double), 1, fp);
	    FWRITE(entry->taper, 1, 9, fp);
	    FWRITE(&entry->taper_start, sizeof(int), 1, fp);
	    FWRITE(&entry->taper_end, sizeof(int), 1, fp);
	    FWRITE(&entry->winpts, sizeof(int), 1, fp);
	    FWRITE(&entry->overlap, sizeof(int), 1, fp);
	    FWRITE(&entry->nfft, sizeof(int), 1, fp);
	    FWRITE(&entry->smoothvalue, sizeof(float), 1, fp);
	    FWRITE(&entry->do_rsp, sizeof(Boolean), 1, fp);
	    FWRITE(&entry->contrib, sizeof(int), 1, fp);
	}
#endif
	return True;
}

void
FtPlotRead(Widget widget, FILE *fp)
{
#ifdef _HO_HO_HO_
	FtPlotWidget w = (FtPlotWidget)widget;
	int i, n;

	FtPlotClear(w);

	fread(&n, sizeof(int), 1, fp);

	w->ft_plot.num_entries = 0;

	for(i = 0; i < n; i++)
	{
	    if(!FtPlotReadEntry(w, fp)) return;
	}

	_AxesRedraw((AxesWidget)w);
	_FtPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_FtPlotRedisplay(w, True);
	_AxesRedisplayXor((AxesWidget)w);
#endif
}

#define FREAD(a,b,c,d) \
if(fread(a,b,c,d) != c) {perror("FtPlotReadEntry"); return False;}

Boolean
FtPlotReadEntry(FtPlotWidget w, FILE *fp)
{
#ifdef _HO_HO_HO_
	int i, j, num_resp;
	Boolean have_it;
	FtPlotClassPart *cc = &ftPlotClassRec.ft_plot_class;
	FtEntry ft_entry_init = FT_ENTRY_INIT;
	FtEntry *entry;

	ALLOC((w->ft_plot.num_entries+1)*sizeof(FtEntry *),
		w->ft_plot.size_entry, w->ft_plot.entry, FtEntry *, 0);

	i = w->ft_plot.num_entries;

	if( !(w->ft_plot.entry[i] = (FtEntry *)AxesMalloc((Widget)w,
		sizeof(FtEntry))) ) return False;

	entry = w->ft_plot.entry[i];
	memcpy(entry, &ft_entry_init, sizeof(FtEntry));

	entry->id = cc->nextid++;
	fread(&entry->data_fg, sizeof(Pixel), 1, fp);
	fread(&entry->on, sizeof(Boolean), 1, fp);
	fread(&entry->sta, 1, 8, fp);
	fread(&entry->chan, 1, 9, fp);
	fread(&entry->calper, sizeof(float), 1, fp);
	fread(&num_resp, sizeof(int), 1, fp);
	entry->rsp = NULL;
	for(j = 0; j < num_resp; j++)
	{
	    Response *r = new Response();
	    if(!entry->rsp) {
		entry->rsp = new vector<Response *>;
	    }
	    entry->rsp->push_back(r);
	    fread(r->source, 1, 13, fp);
	    fread(&r->seq_num, sizeof(int), 1, fp);
	    fread(r->des, 1, 13, fp);
	    fread(r->type, 1, 7, fp);
	    fread(r->units, 1, 2, fp);
	    fread(r->author, 1, 45, fp);
	    fread(&r->a0, sizeof(double), 1, fp);
	    fread(&r->npoles, sizeof(int), 1, fp);
	    fread(&r->nzeros, sizeof(int), 1, fp);
	    if( !(r->pole = (FComplex *)AxesMalloc((Widget)w,
		r->npoles*sizeof(FComplex))) ) return False;

	    fread(r->pole, sizeof(FComplex), r->npoles, fp);
	    if( !(r->pole_err = (FComplex *)AxesMalloc((Widget)w,
		r->npoles*sizeof(FComplex))) ) return False;

	    fread(r->pole_err, sizeof(FComplex), r->npoles, fp);
	    if( !(r->zero = (FComplex *)AxesMalloc((Widget)w,
		r->nzeros*sizeof(FComplex))) ) return False;
	    fread(r->zero, sizeof(FComplex), r->nzeros, fp);

	    if( !(r->zero_err = (FComplex *)AxesMalloc((Widget)w,
		r->nzeros*sizeof(FComplex))) ) return False;

	    fread(r->zero_err, sizeof(FComplex), r->nzeros, fp);
	    fread(&r->nfap, sizeof(int), 1, fp);

	    if(!(r->fap_f = (float *)AxesMalloc((Widget)w,
		r->nfap*sizeof(float))) ) return False;
	    if(!(r->fap_a = (float *)AxesMalloc((Widget)w,
		r->nfap*sizeof(float))) ) return False;
	    if(!(r->fap_p = (float *)AxesMalloc((Widget)w,
		r->nfap*sizeof(float))) ) return False;
	    if(!(r->amp_error = (float *)AxesMalloc((Widget)w,
		r->nfap*sizeof(float))) ) return False;
	    if(!(r->phase_error = (float *)AxesMalloc((Widget)w,
		r->nfap*sizeof(float))) ) return False;

	    fread(r->fap_f, sizeof(float), r->nfap, fp);
	    fread(r->fap_a, sizeof(float), r->nfap, fp);
	    fread(r->fap_p, sizeof(float), r->nfap, fp);
	    fread(r->amp_error, sizeof(float), r->nfap, fp);
	    fread(r->phase_error, sizeof(float), r->nfap, fp);
	    fread(&r->samprate, sizeof(float), 1, fp);
	    fread(&r->num_n, sizeof(int), 1, fp);
	    fread(&r->num_d, sizeof(int), 1, fp);
	    if(!(r->fir_n = (float *)AxesMalloc((Widget)w,
		r->num_n*sizeof(float))) ) return False;
	    fread(r->fir_n, sizeof(float), r->num_n, fp);
	    if(!(r->fir_n_error = (float *)AxesMalloc((Widget)w,
		r->num_n*sizeof(float))) ) return False;

	    fread(r->fir_n_error, sizeof(float), r->num_n, fp);
	    if(!(r->fir_d = (float *)AxesMalloc((Widget)w,
		r->num_d*sizeof(float))) ) return False;
	    fread(r->fir_d, sizeof(float), r->num_d, fp);
	    if(!(r->fir_d_error = (float *)AxesMalloc((Widget)w,
		r->num_d*sizeof(float))) ) return False;

	    fread(r->fir_d_error, sizeof(float), r->num_d, fp);

	    fread(&have_it, sizeof(Boolean), 1, fp);
	    if(have_it)
	    {
		if(!(r->cal = (GSE_CAL *)AxesMalloc((Widget)w,
			sizeof(GSE_CAL))) ) return False;
		fread(r->cal, sizeof(GSE_CAL), 1, fp);
	    }
	}
	fread(&entry->npts, sizeof(int), 1, fp);
	fread(&entry->np2, sizeof(int), 1, fp);
	fread(&entry->nf, sizeof(int), 1, fp);
	fread(&entry->type, sizeof(int), 1, fp);

	fread(&entry->t_length, sizeof(int), 1, fp);
	if( !(entry->t = (float *)AxesMalloc((Widget)w,
		entry->t_length*sizeof(float))) ) return False;
	fread(entry->t, sizeof(float), entry->t_length, fp);

	fread(&entry->amp_length, sizeof(int), 1, fp);
	if( !(entry->amp = (float *)AxesMalloc((Widget)w,
		entry->amp_length*sizeof(float))) ) return False;
	fread(entry->amp, sizeof(float), entry->amp_length, fp);

	fread(&entry->phase_length, sizeof(int), 1, fp);
	if( !(entry->phase = (float *)AxesMalloc((Widget)w,
		entry->phase_length*sizeof(float))) ) return False;
	fread(entry->phase, sizeof(float), entry->phase_length, fp);

	fread(&entry->x_length, sizeof(int), 1, fp);
	if(!(entry->x = (float *)AxesMalloc((Widget)w,
		entry->x_length*sizeof(float))) ) return False;
	fread(entry->x, sizeof(float), entry->x_length, fp);

	fread(&j, sizeof(int), 1, fp);
	if(j == 1) entry->y = entry->amp;
	else if(j == 2) entry->y = entry->phase;

	fread(&entry->db_length, sizeof(int), 1, fp);
	if(!(entry->xPow = (float *)AxesMalloc((Widget)w,
		entry->db_length*sizeof(float))) ) return False;
	fread(entry->xPow, sizeof(float), entry->db_length, fp);

	fread(&entry->time, sizeof(double), 1, fp);
	fread(&entry->dt, sizeof(double), 1, fp);
	fread(&entry->df, sizeof(double), 1, fp);
	fread(&entry->x_min, sizeof(double), 1, fp);
	fread(&entry->x_max, sizeof(double), 1, fp);
	fread(&entry->y_min, sizeof(double), 1, fp);
	fread(&entry->y_max, sizeof(double), 1, fp);
	fread(&entry->amp_y_min, sizeof(double), 1, fp);
	fread(&entry->amp_y_max, sizeof(double), 1, fp);
	fread(&entry->phase_y_min, sizeof(double), 1, fp);
	fread(&entry->phase_y_max, sizeof(double), 1, fp);
	fread(entry->taper, 1, 9, fp);
	fread(&entry->taper_start, sizeof(int), 1, fp);
	fread(&entry->taper_end, sizeof(int), 1, fp);
	fread(&entry->winpts, sizeof(int), 1, fp);
	fread(&entry->overlap, sizeof(int), 1, fp);
	fread(&entry->nfft, sizeof(int), 1, fp);
	fread(&entry->smoothvalue, sizeof(float), 1, fp);
	fread(&entry->do_rsp, sizeof(Boolean), 1, fp);
	fread(&entry->contrib, sizeof(int), 1, fp);

	w->ft_plot.num_entries++;

	if(w->ft_plot.num_entries == 1) {
	    AxesUnzoomAll((AxesWidget)w);
	}
#endif
	return True;
}

/**
 * 
 */
static void
FtPlotMouseDownSelect(FtPlotWidget w, XEvent *event, const char **params,
                Cardinal *num_params)
{
    FtPlotPart *ft = &w->ft_plot;
    AxesPart *ax = (w != NULL) ? &w->axes : NULL;
    int cursor_x, cursor_y;
    long num_selected;

    cursor_x = ((XButtonEvent *)event)->x;
    cursor_y = ((XButtonEvent *)event)->y;

    if(outside(w, cursor_x, cursor_y)) return;

    ax->last_cursor_x = cursor_x;
    ax->last_cursor_y = cursor_y;

    if(	ax->cursor_mode == AXES_MOVE_MAGNIFY ||
	ax->cursor_mode == AXES_SELECT_CURSOR)
    {
	_AxesMove((AxesWidget)w, event, params, num_params);
    }
    else {
	ft->select_entry = WhichEntry(w, cursor_x, cursor_y);
	if(ft->select_entry && !ft->select_entry->selectable) {
	    ft->select_entry = false;
	    return;
	}
	mouseDownSelect(w, ft->select_entry);
	if(ft->select_entry) {
	    int i;
	    num_selected = 0;
	    for(i = 0; i < (int)ft->entries.size(); i++) {
		if(ft->entries[i]->selected) num_selected++;
	    }
	    XtCallCallbacks((Widget)w, XtNselectCallback,
				(XtPointer)num_selected);
	}
    }
}

static Boolean
outside(FtPlotWidget w, int cursor_x, int cursor_y)
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

static void
mouseDownSelect(FtPlotWidget w, FtEntry *selected_entry)
{
    FtPlotPart *ft = (w != NULL) ? &w->ft_plot : NULL;
    int i;

    if((int)ft->entries.size() <= 0) return;

    for(i = 0; i < (int)ft->entries.size(); i++) {
	FtEntry *entry = ft->entries[i];
	if(entry->selected && entry != selected_entry) {
	    entry->selected = False;
	}
    }
    if(selected_entry != NULL) {
	selected_entry->selected = !selected_entry->selected;
    }
    _AxesRedisplayAxes((AxesWidget)w);
    _FtPlotRedisplay(w, True);
    _AxesRedisplayXor((AxesWidget)w);
}

void
FtPlotFillSelected(FtPlotWidget w)
{
    FtPlotPart *ft = (w != NULL) ? &w->ft_plot : NULL;
    Boolean found_one;
    int i;

    found_one = False;
    for(i = 0; i < (int)ft->entries.size(); i++) {
	FtEntry *entry = ft->entries[i];
	if(entry->selected) {
	    entry->fill = !entry->fill;
	    DrawEntry(w, &w->axes.d, entry, &entry->s);
	    found_one = True;
	}
    }
    if(found_one) {
	_AxesRedisplayAxes((AxesWidget)w);
	_FtPlotRedisplay(w, True);
	_AxesRedisplayXor((AxesWidget)w);
    }
}

static void
LeaveWindow(FtPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    if(w->ft_plot.point_x >= 0) {
	DrawRectangles(w, w->axes.mag_invertGC, w->ft_plot.point_recs, 2);
	w->ft_plot.point_x = -1;
    }
}

static int
findx(float *x, int n, double fx)
{
	int jl = -1, ju = n, jm;

	if(n <= 0) return(0);

	if(fx <= x[0]) return(0);

	if(fx >= x[n-1]) return(n-1);

	while(ju - jl > 1)
	{
	    jm = (ju + jl)/2;

	    if(fx > x[jm]) {
		jl = jm;
	    }
	    else {
		ju = jm;
	    }
	}
	return(jl);
}

int FtPlotNumEntries(FtPlotWidget w)
{
    return (int)w->ft_plot.entries.size();
}
