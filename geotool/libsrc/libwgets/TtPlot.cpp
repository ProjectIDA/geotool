/** \file TtPlot.cpp
 *  \brief Defines widget TtPlot.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      TtPlot Widget -- widget for drawing Travel-Time curves.
 *
 * SYNOPSIS
 *      #include "TtPlot.h"
 *      Widget
 *      TtPlotCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      TtPlot.h
 *
 * AUTHOR
 *      I. Henson -- July 1990
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */
#include <sys/param.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "widget/TtPlotP.h"
#include "widget/TtPlotClass.h"
#include "widget/CPlot.h"
extern "C" {
#include "libgplot.h"
#include "libgmath.h"
#include "libisop.h"
#include "libstring.h"
}

#define MALLOC(ptr, Size, type)				\
	if( (ptr = (type *)malloc(Size)) <= (type *)0)	\
	{						\
		perror("TtPlot: malloc error");	\
		printf("   size=%ld\n",(long)(Size));		\
		exit(1);				\
	}

#define XtRTtPlotInt	(char *)"TtPlotInt"
#define DGR2KM  111.1954        /* kilometers per degree */

#define	offset(field)		XtOffset(TtPlotWidget, tt_plot.field)
static XtResource	resources[] = 
{
	{XtNgeoTableDir, XtCGeoTableDir, XtRString, sizeof(String),
		offset(geo_table_dir), XtRString, (XtPointer)NULL},
	{XtNdisplayTtCurves, XtCDisplayTtCurves, XtRBoolean, sizeof(Boolean),
		offset(display_tt_curves), XtRString, (XtPointer)"False"},
	{XtNdisplayTtLabels, XtCDisplayTtLabels, XtRBoolean, sizeof(Boolean),
		offset(display_tt_labels), XtRString, (XtPointer)"False"},
	{XtNdistanceUnits, XtCDistanceUnits, XtRTtPlotInt, sizeof(int),
		offset(distance_units), XtRString, (XtPointer)"DEGREES"},
        {XtNselectedPhases, XtCSelectedPhases, XtRPointer, sizeof(char **),
                offset(selected_phases), XtRImmediate, (XtPointer)NULL},
	{XtNreducedTime, XtCReducedTime, XtRBoolean, sizeof(Boolean),
		offset(reduced_time), XtRString, (XtPointer)"False"},
	{XtNiaspeiCurveColor, XtCIaspeiCurveColor, XtRPixel, sizeof(Pixel),
		offset(iaspei_curve_color), XtRString, (XtPointer)"black"},
	{XtNiaspeiColor, XtCIaspeiColor, XtRPixel, sizeof(Pixel),
		offset(iaspei_color), XtRString, (XtPointer)"yellow"},
	{XtNjbColor, XtCJbColor, XtRPixel, sizeof(Pixel),
		offset(jb_color), XtRString, (XtPointer)"orange"},
	{XtNiaspeiTable, XtCIaspeiTable, XtRString, sizeof(String),
		offset(iaspei_table), XtRString, (XtPointer)NULL},
	{XtNjbTable, XtCJbTable, XtRString, sizeof(String),
		offset(jb_table), XtRString, (XtPointer)NULL},
	{XtNregionalColor, XtCRegionalColor, XtRPixel, sizeof(Pixel),
		offset(regional_color), XtRString, (XtPointer)"sky blue"},
	{XtNalignPredSelected, XtCAlignPredSelected, XtRBoolean,sizeof(Boolean),
		offset(align_pred_selected), XtRString, (XtPointer)"False"},
	{XtNuseCelerity, XtCUseCelerity, XtRBoolean,sizeof(Boolean),
		offset(use_celerity), XtRString, (XtPointer)"True"},
};
#undef offset

/* Private functions */

static void CvtStringToTtPlotInt(XrmValuePtr *args, Cardinal *num_args,
			XrmValuePtr fromVal, XrmValuePtr toVal);
static Boolean CvtStringToStringArray(Display *dpy, XrmValuePtr args,
			Cardinal *num_args, XrmValuePtr from, XrmValuePtr to,
			XtPointer *data);
static void StringArrayDestructor(XtAppContext app, XrmValuePtr to,
			XtPointer converter_data, XrmValuePtr args,
			Cardinal *num_args);
static void ClassInitialize(void);
static void Initialize(Widget req, Widget New);
static Boolean SetValues(TtPlotWidget cur, TtPlotWidget req, TtPlotWidget New);
static Boolean SelectPhases(TtPlotWidget w);
static Boolean SelectedPhases(TtPlotWidget w);
static void Destroy(Widget w);
static void ComputeCurve(TtPlotWidget w, int m, int table);
static void DoAllPredLabels(TtPlotWidget w);
static void DoPredLabel(TtPlotWidget w, DataEntry *entry, TrTm *tr);
static void get_label_points(TtPlotWidget w, CPlotPredArr *pred_arr);
static void check_table(TtPlotWidget w, DataEntry *entry, float *tt, int num,
		PhaseList *phases, int *npred, CPlotPredArr *pred_arr);
static void tlp(TtPlotWidget w, const string &phase, int *npts, float *tt,
		float *delta);
static Boolean SetLimits(TtPlotWidget w);
static void fminmax(int n, float *a, double *min, double *max);
static void dminmax(int n, double *a, double *min, double *max);
extern "C" {
static int sort_by_depth(const void *A, const void *B);
}
static void JBcurve(TtPlotWidget w, const string &phase, float depth, int *npts,
			float *y, float *x);
static void SaveState(TtPlotWidget w, CPlotState *state);
static void RestoreState(TtPlotWidget w, CPlotState *state, Boolean redraw);
static void positionEntry(TtPlotWidget w, Waveform *cd, Boolean on);
static void DeleteSaved(TtPlotWidget w, DataEntry *entry);
static Boolean updateCurves(TtPlotWidget w, Boolean mag_redo);
static void TtPlotRedisplay(TtPlotWidget w);


TtPlotClassRec	ttPlotClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&cPlotClassRec),	/* superclass */
	(char *)"TtPlot",		/* class_name */
	sizeof(TtPlotRec),		/* widget_size */
	ClassInitialize,		/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	XtInheritRealize,		/* realize */
        NULL,				/* actions */
        0,				/* num_actions */
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
        XtInheritTranslations,		/* tm_table */
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
        {       /* AxesClass fields */
		0,		/* empty */
        },
        {       /* CPlotClass fields */
		0,		/* empty */
        },
        {       /* TtPlotClass fields */
		(char *)NULL,		/* jb_table */
		(char *)NULL,		/* iaspei_table */
        },
};

WidgetClass ttPlotWidgetClass = (WidgetClass)&ttPlotClassRec;


static void
CvtStringToTtPlotInt(XrmValuePtr *args, Cardinal *num_args, XrmValuePtr fromVal,
			XrmValuePtr toVal)
{
	char		*s = fromVal->addr;	/* string to convert */
	static int	val;			/* returned value */

	if(s == NULL)					val = 0;
	else if(!strcasecmp(s, "DEGREES"))		val = DEGREES;
	else if(!strcasecmp(s, "KILOMETERS"))		val = KILOMETERS;
	toVal->size = sizeof(val);
	toVal->addr = (char *)&val;
}

static void
ClassInitialize(void)
{
    XtAddConverter(XtRString,XtRTtPlotInt,(XtConverter)CvtStringToTtPlotInt,
		(XtConvertArgList)NULL, (Cardinal) 0);
    XtSetTypeConverter(XmRString, XmRStringArray,
	(XtTypeConverter)CvtStringToStringArray,
	NULL, 0, XtCacheAll | XtCacheRefCount, StringArrayDestructor);
}

static Boolean
CvtStringToStringArray(Display *dpy, XrmValuePtr args, Cardinal *num_args,
		XrmValuePtr from, XrmValuePtr to, XtPointer *data)
{
    static String *array;
    String s = from->addr;

    if(s == NULL || *s== '\0') {
	array = NULL;
    }
    else {
	String tok, c, buf;
	char *last;
	int n;

	buf = strdup(s);
	array = (String *)malloc(sizeof(String));
	n = 0;
	tok = buf;
	while((c = strtok_r(tok, ",", &last)) != (String)NULL) {
	    tok = (String)NULL;
	    array = (String *)realloc(array, (n+1)*sizeof(String));
	    array[n++] = strdup(stringTrim(c));
	}
	array = (String *)realloc(array, (n+1)*sizeof(String));
	array[n++] = (String)NULL;
	free(buf);
    }
    if (to->addr == NULL) to->addr = (char *) &array;
    else *(String **) to->addr = array;
    to->size = sizeof(String *);

    return True;
}

/*
 * Free the string array allocated by the String to StringArray converter
 */
static void
StringArrayDestructor(XtAppContext app, XrmValuePtr to,
	XtPointer converter_data, XrmValuePtr args, Cardinal *num_args)
{
    String *array = *(String **) to->addr;
    String *entry;

    if(array == NULL) return;

    for (entry = array; *entry != NULL; entry++) XtFree((char *) *entry);

    XtFree((char *) array);
}

static void
Initialize(Widget w_req, Widget w_new)
{
	TtPlotWidget New = (TtPlotWidget)w_new;
	TtPlotPart *tp = &New->tt_plot;
	CPlotPart *c_plot = &New->c_plot;
	int	i, n;
	char	path[MAXPATHLEN+1];
        struct stat buf;

	tp->source_depth = 0.;
	tp->reduction_velocity = 0.;
	tp->stop_Pdiff = 120.;
	tp->lg_vel = 3.4;
	tp->lq_vel = 3.2;
	tp->lr_vel = 3.0;
	tp->rg_vel = 3.0;
	tp->t_vel = 1.485;
	tp->infra_vel = .320;
	tp->infra_tt = -1.;

	if(tp->iaspei_table == NULL)
	{
	    tp->iaspei_table = strdup("");
	    if(tp->geo_table_dir != NULL)
	    {
		snprintf(path, MAXPATHLEN, "%s/models/iasp91.hed",
			tp->geo_table_dir);
		if(stat(path, &buf) == 0) {
		    snprintf(path, MAXPATHLEN, "%s/models/iasp91",
			tp->geo_table_dir);
		    tp->iaspei_table = strdup(path);
		}
	    }
	}
	else {
	    tp->iaspei_table = strdup(tp->iaspei_table);
	}

	if(tp->jb_table == NULL)
	{
	    tp->jb_table = strdup("");
	    if(tp->geo_table_dir != NULL)
	    {
		snprintf(path, MAXPATHLEN, "%s/models/jbtable",
				tp->geo_table_dir);
		if(stat(path, &buf) == 0) {
		    free(tp->jb_table);
		    tp->jb_table = strdup(path);
		}
	    }
	}
	else {
	    tp->jb_table = strdup(tp->jb_table);
	}

	tp->ray_widget = (Widget)NULL;
	tp->crust.h[0] = 0.;
	tp->crust.h[1] = 0.;
	tp->crust.vp[0] = -1.;
	tp->crust.vp[1] = -1.;
	tp->crust.vp[2] = -1.;
	tp->crust.vs[0] = -1.;
	tp->crust.vs[1] = -1.;
	tp->crust.vs[2] = -1.;
	stringcpy(tp->crust.name, "", sizeof(tp->crust.name));
	tp->crust.full_name[0] = '\0';

	tp->iaspei_depth = -999.;
	tp->num_labels = 0;
	tp->pred_labels = NULL;
	tp->predicted_first_warn = True;
	tp->predictedPhase_first_warn = True;
	tp->jb_first_warn = True;
	tp->prev_n = 0;

	tp->travel_time = new TravelTime(tp->iaspei_table, tp->jb_table);

	tp->travel_time->setLgVelocity(tp->lg_vel);
	tp->travel_time->setLqVelocity(tp->lq_vel);
	tp->travel_time->setLrVelocity(tp->lr_vel);
	tp->travel_time->setRgVelocity(tp->rg_vel);
	tp->travel_time->setTVelocity(tp->t_vel);
	tp->travel_time->setUseCelerity(tp->use_celerity);
	tp->travel_time->setStopPdiff(tp->stop_Pdiff);

	MALLOC(tp->iaspei_phases, tp->travel_time->num_iaspei*sizeof(PhaseList),
		PhaseList);
	MALLOC(tp->jb_phases, tp->travel_time->num_jb*sizeof(PhaseList),
		PhaseList);
	MALLOC(tp->regional_phases, tp->travel_time->num_regional*
			sizeof(PhaseList), PhaseList);
	MALLOC(tp->surface_phases, tp->travel_time->num_surface*
			sizeof(PhaseList), PhaseList);

	for(i = 0; i < tp->travel_time->num_iaspei; i++) {
	    tp->iaspei_phases[i].phase = tp->travel_time->iaspeiPhase(i);
	    tp->iaspei_phases[i].made = false;
	    tp->iaspei_phases[i].selected = false;
	}
	for(i = 0; i < tp->travel_time->num_jb; i++) {
	    tp->jb_phases[i].phase = tp->travel_time->jbPhase(i);
	    tp->jb_phases[i].made = false;
	    tp->jb_phases[i].selected = false;
	}
	for(i = 0; i < tp->travel_time->num_regional; i++) {
	    tp->regional_phases[i].phase = tp->travel_time->regionalPhase(i);
	    tp->regional_phases[i].made = false;
	    tp->regional_phases[i].selected = false;
	}
	for(i = 0; i < tp->travel_time->num_surface; i++) {
	    tp->surface_phases[i].phase = tp->travel_time->surfacePhase(i);
	    tp->surface_phases[i].made = false;
	    tp->surface_phases[i].selected = false;
	}

	c_plot->num_curves = tp->travel_time->num_iaspei +
			tp->travel_time->num_jb +
			tp->travel_time->num_regional +
			tp->travel_time->num_surface;
	c_plot->size_curve = c_plot->num_curves;
	MALLOC(c_plot->curve, c_plot->num_curves*sizeof(DataCurve *),
		DataCurve *);
	for(i = 0; i < c_plot->num_curves; i++)
	{
	    c_plot->curve[i] = new DataCurve();
	    c_plot->curve[i]->type = CPLOT_TTCURVE;
	    c_plot->curve[i]->on = False;
	}

	for(i = 0; i < tp->travel_time->num_iaspei; i++) {
	    c_plot->curve[i]->lab.assign(tp->iaspei_phases[i].phase);
	}
	n = tp->travel_time->num_iaspei;
	for(i = 0; i < tp->travel_time->num_jb; i++) {
	    c_plot->curve[n+i]->lab.assign(tp->jb_phases[i].phase+1);
	}
	n += tp->travel_time->num_jb;
	for(i = 0; i < tp->travel_time->num_regional; i++) {
	    c_plot->curve[n+i]->lab.assign(tp->regional_phases[i].phase+1);
	}
	n += tp->travel_time->num_regional;
	for(i = 0; i < tp->travel_time->num_surface; i++) {
	    c_plot->curve[n+i]->lab.assign(tp->surface_phases[i].phase);
	}

	tp->last_depth = -999.;

	tp->state.num_saved = 0;
	tp->state.saved = NULL;
	tp->state.zoom = 1;
	tp->state.zoom_max = 1;
	tp->restore_last_state = False;
	tp->last_state.num_saved = 0;
	tp->last_state.saved = NULL;
	tp->last_state.zoom = 1;
	tp->last_state.zoom_max = 1;
	if(tp->display_tt_curves) c_plot->adjust_after_move = False;
	if(!tp->reduced_time) tp->reduction_velocity = 0.;
	c_plot->display_predicted_arrivals = tp->display_tt_labels;

	AxesSetYLimits((AxesWidget)w_new, 1., 0., True);
}

void TtPlotClass::setComputeTTMethod(ComputeTTMethod method)
{
    TtPlotPart *tp = &tw->tt_plot;
    delete tp->travel_time;
    tp->travel_time = new TravelTime(tp->iaspei_table, tp->jb_table,method);
}

double TtPlotClass::getTravelTime(const string &phase, CssOriginClass *origin,
	double lat, double lon, double elev, const string &net,
	const string &sta, double *slowness, string &op)
{
    TtPlotPart *tp = &tw->tt_plot;
    return tp->travel_time->getTravelTime(phase, origin, lat, lon, elev, net,
		sta, slowness, op);
}

bool TtPlotClass::firstArrival(CssOriginClass *origin, double lat, double lon,
	double elev, const string &net, const string &sta, char type,
	string &phase, double *time, double *slowness)
{
    TtPlotPart *tp = &tw->tt_plot;
    return tp->travel_time->firstArrival(origin, lat, lon, elev, net, sta,
		type, phase, time, slowness);
}

double TtPlotClass::getSourceDepth()
{
    return tw->tt_plot.source_depth;
}

void TtPlotClass::setReductionVelocity(double vel, bool reduced_time)
{
    TtPlotPart *tp = &tw->tt_plot;
    int i;

    if(tp->reduction_velocity == vel && tp->reduced_time == reduced_time) {
	return;
    }

    tp->reduction_velocity = vel;
    tp->reduced_time = reduced_time;

    for(i = 0; i < tp->travel_time->num_iaspei; i++) {
	tp->iaspei_phases[i].made = False;
    }
    for(i = 0; i < tp->travel_time->num_jb; i++) {
	tp->jb_phases[i].made = False;
    }
    for(i = 0; i < tp->travel_time->num_regional; i++) {
	tp->regional_phases[i].made = False;
    }
    if(tp->display_tt_curves)
    {
	bool new_limits, redraw = False;

	if( updateCurves(tw, False) ) redraw = True;

	new_limits = SetLimits(tw);

	if(tp->display_tt_curves && redraw)
	{
	    TtPlotWidget z = (TtPlotWidget)tw->axes.mag_to;
	    _CPlotRedraw((CPlotWidget)tw);
	    _AxesRedisplayAxes((AxesWidget)tw);
	    _CPlotRedisplay((CPlotWidget)tw);
	    _AxesRedisplayXor((AxesWidget)tw);

	    if(z != NULL && !new_limits)
	    {
		_AxesRedraw((AxesWidget)z);
		_CPlotRedraw((CPlotWidget)z);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay((CPlotWidget)z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
    }
}

int TtPlotClass::getIaspeiPhases(vector<const char *> &phase_list)
{
    TtPlotPart *tp = &tw->tt_plot;

    phase_list.clear();
    for(int i = 0; i < tp->travel_time->num_iaspei; i++) {
	phase_list.push_back(tp->iaspei_phases[i].phase);
    }
    return (int)phase_list.size();
}

static Boolean
SetValues(TtPlotWidget cur, TtPlotWidget req, TtPlotWidget New)
{
	TtPlotPart *cur_tp = &cur->tt_plot;
	TtPlotPart *new_tp = &New->tt_plot;
	TtPlotPart *req_tp = &req->tt_plot;
	Boolean redisplay = False, new_limits = False;
	Boolean  redraw = False;
	Boolean mag_redo = False;
	Boolean label_model_change = False;
	TtPlotWidget z;
	int i, j, n;

	z = (TtPlotWidget)New->axes.mag_to;

	if((req_tp->iaspei_table==NULL && cur_tp->iaspei_table!=NULL)
	|| (req_tp->iaspei_table!=NULL && cur_tp->iaspei_table==NULL)
	|| (req_tp->iaspei_table!=NULL && cur_tp->iaspei_table!=NULL
	    && strcmp(req_tp->iaspei_table,cur_tp->iaspei_table)))
	{
	    Free(cur_tp->iaspei_table);
	    new_tp->iaspei_table = (req_tp->iaspei_table != NULL)
		    ? strdup(req_tp->iaspei_table) : strdup("");
	    redraw = True;
	}
	else {
	    new_tp->iaspei_table = cur_tp->iaspei_table;
	}
	if((req_tp->jb_table == NULL && cur_tp->jb_table != NULL)
	|| (req_tp->jb_table != NULL && cur_tp->jb_table == NULL)
	|| (req_tp->jb_table != NULL && cur_tp->jb_table != NULL
	    && strcmp(req_tp->jb_table, cur_tp->jb_table)))
	{
	    Free(cur_tp->jb_table);
	    new_tp->jb_table = (req_tp->jb_table != NULL)
		    ? strdup(req_tp->jb_table) : strdup("");
	    redraw = True;
	}
	else {
	    new_tp->jb_table = cur_tp->jb_table;
	}
	if(cur_tp->reduced_time != req_tp->reduced_time)
        {
	    for(i = 0; i < new_tp->travel_time->num_iaspei; i++) {
		new_tp->iaspei_phases[i].made = False;
	    }
	    for(i = 0; i < new_tp->travel_time->num_jb; i++) {
		new_tp->jb_phases[i].made = False;
	    }
	    for(i = 0; i < new_tp->travel_time->num_regional; i++) {
		new_tp->regional_phases[i].made = False;
	    }
	}
	if(cur_tp->selected_phases != req_tp->selected_phases)
	{
	    label_model_change = redraw = SelectPhases(New);
	    new_tp->selected_phases = NULL;
	}
	if(cur_tp->display_tt_curves != new_tp->display_tt_curves)
	{
	    New->c_plot.adjust_after_move = !new_tp->display_tt_curves;
	    redraw = True;
	    n = new_tp->travel_time->num_iaspei + new_tp->travel_time->num_jb
		 + new_tp->travel_time->num_regional +
		 + new_tp->travel_time->num_surface;
	    for(i = 0; i < n; i++)
	    {
		New->c_plot.curve[i]->on = False;
		if(z != NULL) {
		    z->c_plot.curve[i]->on = False;
		}
	    }
	    if(z != NULL) {
		CPlotMagnifyWidget((CPlotWidget)New, NULL, True);
		mag_redo = True;
	    }
	}
	if(new_tp->display_tt_curves)
	{
	    if( updateCurves(New, mag_redo) ) redraw = True;

	    if(!cur_tp->display_tt_curves) {
		SaveState(New, &New->tt_plot.state);
		if(New->tt_plot.restore_last_state &&
			New->tt_plot.last_state.zoom > 1) {
		    RestoreState(New, &New->tt_plot.last_state, True);
		}
		else {
		    new_limits = SetLimits(New);
		}
	    }
	    else if(cur_tp->reduced_time != req_tp->reduced_time)
	    {
		new_limits = SetLimits(New);
	    }
	}
	else if(cur_tp->display_tt_curves) {	/* cur on, new off */
	    SaveState(New, &New->tt_plot.last_state);
	    RestoreState(New, &New->tt_plot.state, True);
	    redisplay = True;
	}
	if(cur_tp->distance_units != req_tp->distance_units)
	{
	    double convert = (new_tp->distance_units == KILOMETERS) ?
			DGR2KM : 1./DGR2KM;
	    for(i = 0; i < New->c_plot.num_curves; i++)
	    {
		DataCurve *curve = New->c_plot.curve[i];
		for(j = 0; j < curve->npts; j++) 
		    if(curve->ray_p[j] >= 0.) curve->y[j] *= convert;

		if(curve->npts > 0)
		{
		    fminmax(curve->npts, curve->y, &curve->ymin, &curve->ymax);
		    if(z != NULL && !mag_redo) {
			z->c_plot.curve[i]->ymin = curve->ymin;
			z->c_plot.curve[i]->ymax = curve->ymax;
		    }
		}
	    }
	    if(New->tt_plot.display_tt_curves)
	    {
		for(i = 0; i < New->c_plot.num_entries; i++) {
		    New->c_plot.entry[i]->w->scaled_y0 *= convert;
		}
		for(i = 0; i <= New->axes.zoom; i++) {
		    New->axes.y1[i] *= convert;
		    New->axes.y2[i] *= convert;
		}
		New->c_plot.dataSpacing *= DGR2KM;
		_AxesRedraw((AxesWidget)New);
		_CPlotRedraw((CPlotWidget)New);

		if(z != NULL && !mag_redo) {
		    for(i = 0; i < z->c_plot.num_entries; i++) {
			z->c_plot.entry[i]->w->scaled_y0 *= convert;
		    }
		    for(i = 0; i <= z->axes.zoom; i++) {
			z->axes.y1[i] *= convert;
			z->axes.y2[i] *= convert;
		    }
		    z->c_plot.dataSpacing *= DGR2KM;
		    _AxesRedraw((AxesWidget)z);
		    _CPlotRedraw((CPlotWidget)z);
		    _AxesRedisplayAxes((AxesWidget)z);
		    _CPlotRedisplay((CPlotWidget)z);
		    _AxesRedisplayXor((AxesWidget)z);
		}
		redisplay = True;
	    }
	}

	if((cur_tp->display_tt_curves || new_tp->display_tt_curves) && redraw)
	{
	    New->c_plot.display_predicted_arrivals = req_tp->display_tt_labels;
	    redisplay = True;
	    _CPlotRedraw((CPlotWidget)New);
	    if(z != NULL  && !mag_redo && !new_limits)
	    {
		_AxesRedraw((AxesWidget)z);
		_CPlotRedraw((CPlotWidget)z);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay((CPlotWidget)z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
	if(cur_tp->display_tt_labels != req_tp->display_tt_labels)
	{
	    New->c_plot.display_predicted_arrivals = True;
	    for(i = 0; i < New->c_plot.num_entries; i++)
	    {
		_CPlotDrawPredArrivals((CPlotWidget)New, New->c_plot.entry[i]);
	    }
	    New->c_plot.display_predicted_arrivals = req_tp->display_tt_labels;
	    if(z != NULL && !mag_redo)
	    {
		z->c_plot.display_predicted_arrivals = True;
		for(i = 0; i < z->c_plot.num_entries; i++)
		{
		    _CPlotDrawPredArrivals((CPlotWidget)z, z->c_plot.entry[i]);
		}
		z->c_plot.display_predicted_arrivals =req_tp->display_tt_labels;
		z->tt_plot.display_tt_labels = req_tp->display_tt_labels;
	    }
	    label_model_change = SelectedPhases(New);
	}
	if(req_tp->display_tt_labels && label_model_change)
	{
	    New->c_plot.display_predicted_arrivals = req_tp->display_tt_labels;
	    DoAllPredLabels(New);
	}

	if(redisplay) { 
	    New->axes.setvalues_redisplay = True;
	    if(New->axes.redisplay_pending) redisplay = False;
	    New->axes.redisplay_pending = True;
	}
	if(cur_tp->display_tt_curves != new_tp->display_tt_curves && z != NULL)
	{
	    CPlotMagnifyWidget((CPlotWidget)New, (CPlotWidget)z, True);
	}
	return(redisplay);
}

static Boolean
updateCurves(TtPlotWidget w, Boolean mag_redo)
{
	TtPlotPart *tp = &w->tt_plot;
	int i, n;
 	Boolean redraw = False;
	TtPlotWidget z = (TtPlotWidget)w->axes.mag_to;

	for(i = 0; i < tp->travel_time->num_iaspei; i++)
	{
	    if(w->c_plot.curve[i]->on != tp->iaspei_phases[i].selected)
	    {
		redraw = True;
	    }
	    w->c_plot.curve[i]->on = tp->iaspei_phases[i].selected;
	    if(z != NULL && !mag_redo) {
		z->c_plot.curve[i]->on = tp->iaspei_phases[i].selected;
	    }
	    if(!tp->iaspei_phases[i].made && w->c_plot.curve[i]->on)
	    {
		ComputeCurve(w, i, IASPEI);
		tp->iaspei_phases[i].made = True;
		redraw = True;
	    }
	}
	n = tp->travel_time->num_iaspei;
	for(i = 0; i < tp->travel_time->num_jb; i++)
	{
	    if(w->c_plot.curve[n+i]->on != tp->jb_phases[i].selected)
	    {
		redraw = True;
	    }
	    w->c_plot.curve[n+i]->on = tp->jb_phases[i].selected;
	    if(z != NULL && !mag_redo)
	    {
		z->c_plot.curve[n+i]->on = tp->jb_phases[i].selected;
	    }
	    if(!tp->jb_phases[i].made && w->c_plot.curve[n+i]->on)
	    {
		ComputeCurve(w, n+i, JB);
		tp->jb_phases[i].made = True;
		redraw = True;
	    }
	}
	n = tp->travel_time->num_iaspei + tp->travel_time->num_jb;
	for(i = 0; i < tp->travel_time->num_regional; i++)
	{
	    if(w->c_plot.curve[n+i]->on != tp->regional_phases[i].selected)
	    {
		redraw = True;
	    }
	    w->c_plot.curve[n+i]->on = tp->regional_phases[i].selected;
	    if(z != NULL && !mag_redo)
	    {
		z->c_plot.curve[n+i]->on = tp->regional_phases[i].selected;
	    }
	    if(!tp->regional_phases[i].made && w->c_plot.curve[n+i]->on)
	    {
		ComputeCurve(w, n+i, REGIONAL);
		tp->regional_phases[i].made = True;
		redraw = True;
	    }
	}
	n = tp->travel_time->num_iaspei + tp->travel_time->num_jb +
		tp->travel_time->num_regional;
	for(i = 0; i < tp->travel_time->num_surface; i++)
	{
	    if(w->c_plot.curve[n+i]->on != tp->surface_phases[i].selected)
	    {
		redraw = True;
	    }
	    w->c_plot.curve[n+i]->on = tp->surface_phases[i].selected;
	    if(z != NULL && !mag_redo)
	    {
		z->c_plot.curve[n+i]->on = tp->surface_phases[i].selected;
	    }
	    if(!tp->surface_phases[i].made && w->c_plot.curve[n+i]->on)
	    {
		ComputeCurve(w, n+i, SURFACE);
		tp->surface_phases[i].made = True;
		redraw = True;
	    }
	}

	return redraw;
}

static Boolean
SelectPhases(TtPlotWidget w)
{
	TtPlotPart *tp = &w->tt_plot;
	int i, j;
	Boolean redraw = False;

	for(i = 0; i < tp->travel_time->num_iaspei; i++)
	{
	    for(j = 0; tp->selected_phases[j] != NULL; j++)
	    {
		if(!strcmp(tp->iaspei_phases[i].phase, tp->selected_phases[j]))
			break;
	    }
	    if(tp->selected_phases[j] != NULL)
	    {
		if(!tp->iaspei_phases[i].selected) {
		    redraw = True;
		    tp->iaspei_phases[i].selected = True;
		}
	    }
	    else if(tp->iaspei_phases[i].selected) {
		redraw = True;
		tp->iaspei_phases[i].selected = False;
	    }
	}
	for(i = 0; i < tp->travel_time->num_jb; i++)
	{
	    for(j = 0; tp->selected_phases[j] != NULL; j++)
	    {
		if(!strcmp(tp->jb_phases[i].phase, tp->selected_phases[j]))
			break;
	    }
	    if(tp->selected_phases[j] != NULL)
	    {
		if(!tp->jb_phases[i].selected) {
		    redraw = True;
		    tp->jb_phases[i].selected = True;
		}
	    }
	    else if(tp->jb_phases[i].selected) {
		redraw = True;
		tp->jb_phases[i].selected = False;
	    }
	}
	for(i = 0; i < tp->travel_time->num_regional; i++)
	{
	    for(j = 0; tp->selected_phases[j] != NULL; j++)
	    {
		if(!strcmp(tp->regional_phases[i].phase,tp->selected_phases[j]))
			break;
	    }
	    if(tp->selected_phases[j] != NULL)
	    {
		if(!tp->regional_phases[i].selected) {
		    redraw = True;
		    tp->regional_phases[i].selected = True;
		}
	    }
	    else if(tp->regional_phases[i].selected) {
		redraw = True;
		tp->regional_phases[i].selected = False;
	    }
	}
	for(i = 0; i < tp->travel_time->num_surface; i++)
	{
	    for(j = 0; tp->selected_phases[j] != NULL; j++)
	    {
		if(!strcmp(tp->surface_phases[i].phase, tp->selected_phases[j]))			break;
	    }
	    if(tp->selected_phases[j] != NULL)
	    {
		if(!tp->surface_phases[i].selected) {
		    redraw = True;
		    tp->surface_phases[i].selected = True;
		}
	    }
	    else if(tp->surface_phases[i].selected) {
		redraw = True;
		tp->surface_phases[i].selected = False;
	    }
	}
	return(redraw);
}

static Boolean
SelectedPhases(TtPlotWidget w)
{
	TtPlotPart *tp = &w->tt_plot;
	int i;

	for(i = 0; i < tp->travel_time->num_iaspei; i++) {
	    if(tp->iaspei_phases[i].selected) return(True);
	}
	for(i = 0; i < tp->travel_time->num_jb; i++) {
	    if(tp->jb_phases[i].selected) return(True);
	}
	for(i = 0; i < tp->travel_time->num_regional; i++) {
	    if(tp->regional_phases[i].selected) return(True);
	}
	for(i = 0; i < tp->travel_time->num_surface; i++) {
	    if(tp->surface_phases[i].selected) return(True);
	}
	return(False);
}

void TtPlotClass::selectIaspeiPhases(bool select)
{
    TtPlotPart *tp = &tw->tt_plot;

    for(int i = 0; i < tp->travel_time->num_iaspei; i++) {
	tp->iaspei_phases[i].selected = True;
    }
}

static void
Destroy(Widget widget)
{
	TtPlotWidget w = (TtPlotWidget)widget;
	TtPlotPart *tp = &w->tt_plot;
	int i;

//	Free(tp->geo_table_dir);
	Free(tp->jb_table);
	Free(tp->iaspei_table);

	Free(tp->iaspei_phases);
	Free(tp->jb_phases);
	Free(tp->regional_phases);
	Free(tp->surface_phases);

	for(i = 0; i < tp->num_labels; i++) {
	    Free(tp->pred_labels[i].phase);
	    Free(tp->pred_labels[i].points);
	}
	tp->num_labels = 0;
}

static void
ComputeCurve(TtPlotWidget w, int m, int table)
{
	TtPlotPart *tp = &w->tt_plot;
	int i, n_branch;
	float x[500], y[500], ray_p[500];
	DataCurve *curve;
	TtPlotWidget z;
	
	curve = w->c_plot.curve[m];

	Free(curve->x);
	Free(curve->x_orig);
	Free(curve->y);
	Free(curve->ray_p);

	if(table == JB) {
	    JBcurve(w, curve->lab, (float)tp->source_depth, &curve->npts, y, x);
	    curve->fg = tp->jb_color;
	}
	else if(table == SURFACE) {
	    tlp(w, curve->lab, &curve->npts, x, y);
	    curve->fg = tp->iaspei_curve_color;
	}
	else if(table == REGIONAL) {
	    get_regional(&tp->crust, curve->lab.c_str(),(float)tp->source_depth,
			&curve->npts, x, y);
	    curve->fg = tp->regional_color;
	}
	else if(table == IASPEI)
	{
	    if(!TravelTime::openIaspei(tp->iaspei_table)) return;

	    if(tp->source_depth != tp->iaspei_depth) {
		tp->iaspei_depth = tp->source_depth;
		depset1((float)tp->iaspei_depth);
	    }
	    for(i = 0; i < 500; i++) ray_p[i] = -1.;
	    get_seg((char *)curve->lab.c_str(), &curve->npts, x, y, ray_p,
			&n_branch);
	    curve->fg = tp->iaspei_curve_color;
	}
	if(curve->fg == w->core.background_pixel) {
	    curve->fg = w->axes.fg;
	}

	if(tp->distance_units == KILOMETERS)
	{
//	    for(i = 0; i < curve->npts; i++) if(!fNaN(y[i])) y[i] *= DGR2KM;
	    for(i = 0; i < curve->npts; i++) if(ray_p[i]>=0.) y[i] *= DGR2KM;
	}
	if(tp->reduced_time && tp->reduction_velocity != 0.) {
	    double d;
	    if(tp->distance_units == KILOMETERS) {
		d = 1./tp->reduction_velocity;
	    }
	    else {
		d = DGR2KM/tp->reduction_velocity;
	    }
//	    for(i = 0; i < curve->npts; i++) if(!fNaN(y[i])) x[i] -= y[i]*d;
	    for(i = 0; i < curve->npts; i++) if(ray_p[i]>=0.) x[i] -= y[i]*d;
	}
	curve->x = (double *)AxesMalloc((Widget)w, curve->npts*sizeof(double));
	curve->y = (float *)AxesMalloc((Widget)w, curve->npts*sizeof(float));
	curve->x_orig = (float *)AxesMalloc((Widget)w,
				curve->npts*sizeof(float));
	curve->ray_p = (float *)AxesMalloc((Widget)w,curve->npts*sizeof(float));
	for(i = 0; i < curve->npts; i++) {
	    curve->x[i] = x[i];
	    curve->x_orig[i] = x[i];
	}
	memcpy(curve->y,         y,	curve->npts*sizeof(float));
	memcpy(curve->ray_p, ray_p,	curve->npts*sizeof(float));

	dminmax(curve->npts, curve->x, &curve->xmin, &curve->xmax);
	fminmax(curve->npts, curve->y, &curve->ymin, &curve->ymax);

	if((z = (TtPlotWidget)w->axes.mag_to) != NULL)
	{
	    z->c_plot.curve[m]->x = curve->x;
	    z->c_plot.curve[m]->y = curve->y;
	    z->c_plot.curve[m]->x_orig = curve->x_orig;
	    z->c_plot.curve[m]->ray_p = curve->ray_p;
	    z->c_plot.curve[m]->npts = curve->npts;
	    z->c_plot.curve[m]->xmin = curve->xmin;
	    z->c_plot.curve[m]->xmax = curve->xmax;
	    z->c_plot.curve[m]->ymin = curve->ymin;
	    z->c_plot.curve[m]->ymax = curve->ymax;
	    z->c_plot.curve[m]->fg = curve->fg;
	}
}

int TtPlotClass::getCurve(const string &phase, int *npts, float *tt,
			float *dist, float *ray_p)
{
    TtPlotPart *tp = &tw->tt_plot;
    int n_branch;

    if(!TravelTime::openIaspei(tp->iaspei_table)) return 0;

    if(tp->iaspei_depth != 0.) {
	tp->iaspei_depth = 0.;
	depset1((float)tp->iaspei_depth);
    }
    n_branch = 0;
    get_seg((char *)phase.c_str(), npts, tt, dist, ray_p, &n_branch);
    return n_branch;
}

static void
DoAllPredLabels(TtPlotWidget w)
{
	TtPlotPart *tp = &w->tt_plot;
        CPlotPart *cp = &w->c_plot;
	int i, num_entries;
	DataEntry **entry = NULL;
	CssOriginClass *o;
	TrTm tr;

	entry = (DataEntry **)AxesMalloc((Widget)w,
			w->c_plot.num_entries*sizeof(DataEntry *));

	num_entries = 0;
	for(i = 0; i < w->c_plot.num_entries; i++)
	    if( (o = cp->cplot_class->getPrimaryOrigin(cp->entry[i]->w)) )
	{
	    w->c_plot.entry[i]->depth = o->depth;
	    entry[num_entries++] = w->c_plot.entry[i];
	}
	qsort(entry, (unsigned)num_entries, sizeof(DataEntry *), sort_by_depth);

	for(i = 0; i < num_entries; i++)
	    if( (o = cp->cplot_class->getPrimaryOrigin(entry[i]->w)) )
	{

	    tp->travel_time->getAllTimes(&tr, o, entry[i]->ts->lat(),
				entry[i]->ts->lon(), entry[i]->ts->elev(),
				entry[i]->ts->net(), entry[i]->ts->sta());
	    DoPredLabel(w, entry[i], &tr);
	}
	Free(entry);
}

void TtPlotClass::useCelerity(bool use_celerity)
{
    TtPlotPart *tp = &tw->tt_plot;

    if(tp->use_celerity != use_celerity)
    {
	tp->use_celerity = use_celerity;
	tp->surface_phases[4].made = False;

	tp->travel_time->setUseCelerity(use_celerity);

	if(tp->display_tt_labels) {
	    DoAllPredLabels(tw);
	    TtPlotRedisplay(tw);
	}
    }
}
	    
void TtPlotClass::setCelerity(double celerity)
{
    TtPlotPart *tp = &tw->tt_plot;

    if(tp->infra_vel != celerity)
    {
	if(celerity <= 0.) {
	    fprintf(stderr, "TtPlotSetCelerity: invalid celerity: %f",celerity);
	    return;
	}
	tp->infra_vel = celerity;

	tp->surface_phases[4].made = False;

	tp->travel_time->setCelerity(celerity);

	if(tp->display_tt_labels) {
	    DoAllPredLabels(tw);
	    TtPlotRedisplay(tw);
	}
    }
}

static void
DoPredLabel(TtPlotWidget w, DataEntry *entry, TrTm *tr)
{
	TtPlotPart *tp = &w->tt_plot;
        CPlotPart *cp = &w->c_plot;
	int i, npred;
	CPlotPredArr pred_arr[200];
	CssOriginClass *origin = cp->cplot_class->getPrimaryOrigin(entry->w);

	if(tr == NULL || origin == NULL || origin->time <= NULL_TIME_CHECK)
	{
	    return;
	}
	if( !XtIsRealized((Widget)w) ) return;

	npred = 0;
	check_table(w, entry, tr->iaspei_tt, tp->travel_time->num_iaspei,
			tp->iaspei_phases, &npred, pred_arr);
	check_table(w, entry, tr->jb_tt, tp->travel_time->num_jb,
			tp->jb_phases, &npred, pred_arr);
	check_table(w, entry, tr->regional_tt, tp->travel_time->num_regional,
			tp->regional_phases, &npred, pred_arr);
	check_table(w, entry, tr->surface_tt, tp->travel_time->num_surface,
			tp->surface_phases, &npred, pred_arr);

	for(i = 0; i < npred; i++) {
	    get_label_points(w, &pred_arr[i]);
	}
	_CPlotPredArrivals((CPlotWidget)w, entry->w, pred_arr, npred);
}

static void
get_label_points(TtPlotWidget w, CPlotPredArr *pred_arr)
{
	TtPlotPart *tp = &w->tt_plot;
	int 		i, n, ascent, descent, direction, x, y;
	XCharStruct	overall;
	Pixmap		pixmap;
	XImage		*image;
	PredictedLabel	*l;

	for(i = 0; i < tp->num_labels; i++)
	{
	    if(!strcmp(tp->pred_labels[i].phase, pred_arr->phase))
	    {
		pred_arr->width = tp->pred_labels[i].width;
		pred_arr->height = tp->pred_labels[i].height;
		pred_arr->ascent = tp->pred_labels[i].ascent;
		pred_arr->npoints = tp->pred_labels[i].npoints;
		pred_arr->points = tp->pred_labels[i].points;
		return;
	    }
	}
	if(tp->pred_labels == NULL) {
	    tp->pred_labels = (PredictedLabel *)AxesMalloc((Widget)w,
				sizeof(PredictedLabel));
	}
	else {
	    tp->pred_labels = (PredictedLabel *)AxesRealloc((Widget)w,
		tp->pred_labels, (tp->num_labels+1)*sizeof(PredictedLabel));
	}
	l = &tp->pred_labels[tp->num_labels];
	tp->num_labels++;

	l->phase = strdup(pred_arr->phase);
	XTextExtents(w->c_plot.arrival_font, l->phase, (int)strlen(l->phase),
			&direction, &ascent, &descent, &overall);
	l->width = overall.width;
	l->height = overall.descent + overall.ascent;
	l->ascent = overall.ascent;

	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
	pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),l->width+1,l->height+1,
		DefaultDepth(XtDisplay(w), DefaultScreen(XtDisplay(w))));
	XSetFunction(XtDisplay(w), w->c_plot.arrivalGC, GXclear);
	XFillRectangle(XtDisplay(w), pixmap, w->c_plot.arrivalGC,
		0, 0, l->width, l->height);
	XSetFunction(XtDisplay(w), w->c_plot.arrivalGC, GXcopy);
	XSetForeground(XtDisplay(w), w->c_plot.arrivalGC, 1);
	XDrawString(XtDisplay(w), pixmap, w->c_plot.arrivalGC, 0,
		l->ascent, l->phase, (int)strlen(l->phase));
	XSetForeground(XtDisplay(w), w->c_plot.arrivalGC, w->axes.fg);

	image = XGetImage(XtDisplay(w), pixmap, 0, 0, l->width, l->height,
				AllPlanes, XYPixmap);
	l->points = (XPoint *)AxesMalloc((Widget)w,
				l->width*l->height*sizeof(XPoint));
 
	for(x = n = 0; x < l->width; x++)
	{
	    for(y = 0; y < l->height; y++)
	    {
		if(XGetPixel(image, x, y))
		{
		    l->points[n].x = x;
		    l->points[n].y = y;
		    n++;
		}
	    }
	}
	l->npoints = n;
	XFreePixmap(XtDisplay(w), pixmap);
	XDestroyImage(image);

	pred_arr->width = l->width;
	pred_arr->height = l->height;
	pred_arr->ascent = l->ascent;
	pred_arr->npoints = l->npoints;
	pred_arr->points = l->points;
}

static void
check_table(TtPlotWidget w, DataEntry *entry, float *tt, int num,
		PhaseList *phases, int *npred, CPlotPredArr *pred_arr)
{
	TtPlotPart *tp = &w->tt_plot;
        CPlotPart *cp = &w->c_plot;
	int i;
	double time, tbeg, tend;
	CssOriginClass *origin = cp->cplot_class->getPrimaryOrigin(entry->w);
	CPlotPredArr cplot_pred_arr_null = CPlot_Pred_Arr_NULL;

	tbeg = entry->ts->tbeg();
	tend = entry->ts->tend();
	
	for(i = 0; i < num; i++) if(phases[i].selected)
	{
	    if(tt[i] > 0.)
	    {
		/* check if time is on waveform
		 */
		time = origin->time + tt[i];
		if(time >= tbeg && time <= tend)
		{
		    memcpy(&pred_arr[*npred], &cplot_pred_arr_null,
					sizeof(CPlotPredArr));

		    if(phases[i].phase[0] == 'R' &&
			    strcmp(phases[i].phase, "Rg")) {
			stringcpy(pred_arr[*npred].phase, phases[i].phase + 1,
					sizeof(pred_arr[*npred].phase));
			pred_arr[*npred].fg = tp->regional_color;
		    }
		    else if(phases[i].phase[0] == 'J') {
			stringcpy(pred_arr[*npred].phase, phases[i].phase + 1,
				sizeof(pred_arr[*npred].phase));
			pred_arr[*npred].fg = tp->jb_color;
		    }
		    else {
			stringcpy(pred_arr[*npred].phase, phases[i].phase,
				sizeof(pred_arr[*npred].phase));
			pred_arr[*npred].fg = tp->iaspei_color;
		    }
		    pred_arr[*npred].time = time;
		    (*npred)++;
		}
	    }
	}
}

/** 
 *  Set the crustal model. This will be used in regional travel time
 *  computations.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] crust The crustal model.
 */
void TtPlotClass::crust(CrustModel *crust)
{
    TtPlotPart *tp = &tw->tt_plot;
    int i, n;

    /* check if this is a new crust model
     */
    if(	tp->crust.h[0] == crust->h[0] && tp->crust.h[1] == crust->h[1] &&
	tp->crust.vp[0] == crust->vp[0] && tp->crust.vp[1] == crust->vp[1] &&
	tp->crust.vp[2] == crust->vp[2] && tp->crust.vs[0] == crust->vs[0] &&
	tp->crust.vs[1] == crust->vs[1] && tp->crust.vs[2] == crust->vs[2])
    {
	// just in case name changed
	memcpy(&tp->crust, crust, sizeof(CrustModel));
	return;
    }

    memcpy(&tp->crust, crust, sizeof(CrustModel));

    n = tp->travel_time->num_iaspei + tp->travel_time->num_jb;
    for(i = 0; i < tp->travel_time->num_regional; i++)
    {
	if(tw->c_plot.curve[n+i]->on) {
	    ComputeCurve(tw, n+i, REGIONAL);
	}
    }

    tp->travel_time->setCrust(crust);

    DoAllPredLabels(tw);

    TtPlotRedisplay(tw);
}

static void
TtPlotRedisplay(TtPlotWidget w)
{
    TtPlotWidget z;

    _AxesRedraw((AxesWidget)w);
    _CPlotRedraw((CPlotWidget)w);
    _AxesRedisplayAxes((AxesWidget)w);
    _CPlotRedisplay((CPlotWidget)w);
    _AxesRedisplayXor((AxesWidget)w);
    if((z = (TtPlotWidget)w->axes.mag_to) != NULL)
    {
	_AxesRedraw((AxesWidget)z);
	_CPlotRedraw((CPlotWidget)z);
	_AxesRedisplayAxes((AxesWidget)z);
	_CPlotRedisplay((CPlotWidget)z);
	_AxesRedisplayXor((AxesWidget)z);
    }
}

static void
tlp(TtPlotWidget w, const string &phase, int *npts, float *tt, float *delta)
{
    TtPlotPart *t = &w->tt_plot;
    float velocity;

    *npts = 2;
    delta[0] = 0.;
    tt[0] = 0.;
    delta[1] = 180.;

    if(!phase.compare("LR")) {
	velocity = t->lr_vel;
    }
    else if(!phase.compare("Rg")) {
	velocity = t->rg_vel;
    }
    else if(!phase.compare("LQ")) {
	velocity = t->lq_vel;
    }
    else if(!phase.compare("T")) {
	velocity = t->t_vel;
    }
    else if(!phase.compare("I")) {
	velocity = t->infra_vel;
	delta[1] = 40.;
    }
    else if(!phase.compare("Lg"))
    {
	velocity = t->lg_vel;
	delta[1] = 20.;
    }
    else {
	*npts = 0;
	return;
    }
    tt[1] = (delta[1] * DGR2KM) / velocity;
    return;
}

/** 
 *  Align waveforms on the arrival time of a phase.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] phase The phase name.
 *  
 */
int TtPlotClass::alignOnPredictedPhase(const string &phase)
{
    TtPlotPart *tp = &tw->tt_plot;
    AxesPart *ax = &tw->axes;
    CPlotPart *cp = &tw->c_plot;
    int	i, j, ret;
    double arr_time, tt, x0_min, x0_max, x=0., slowness;
    vector<double> scaled_x0;
    bool found_one;
    int	num_entries;
    CssOriginClass *origin;
    gvector<Waveform *> wvec;
    DataEntry **entry = NULL;
    string op;

    if(tw->c_plot.num_entries <= 0) return 0;

    entry = (DataEntry **)AxesMalloc((Widget)tw,
			tw->c_plot.num_entries*sizeof(DataEntry *));
    num_entries = 0;
    for(i = 0; i < tw->c_plot.num_entries; i++)
    {
	origin = getPrimaryOrigin(cp->entry[i]->w);
	if(origin) {
	    cp->entry[i]->depth = origin->depth;
	    entry[num_entries++] = cp->entry[i];
	}
    }
    if(num_entries == 0)
    {
	_AxesWarning((AxesWidget)tw, "No origin information.");
	Free(entry);
	return -1;
    }

    qsort(entry, (unsigned)num_entries, sizeof(DataEntry *), sort_by_depth);

    x0_min = cp->tmin;
    x0_max = cp->tmax;
    found_one = false;
    for(i = 0; i < ax->num_cursors; i++)
    {
	if(ax->cursor[i].type == AXES_PHASE_LINE &&
		!phase.compare(ax->cursor[i].label))
	{
	    // shift origin to cursor position
	    x = (ax->x_axis != LEFT) ? ax->cursor[i].scaled_x :
					 ax->cursor[i].scaled_y;

	    if(x > ax->x1[ax->zoom] && x < ax->x2[ax->zoom]) {
		found_one = true;
		break;
	    }
	}
    }
    if(!found_one) {
	x = ax->x1[ax->zoom]+.2*(ax->x2[ax->zoom]-ax->x1[ax->zoom]);
    }
    ax->x1[ax->zoom] -= x;
    ax->x2[ax->zoom] -= x;
    for(j = ax->zoom-1; j > 0; j--) {
	if(ax->x1[j+1] < ax->x1[j] || ax->x2[j+1] > ax->x2[j]) {
	    ax->x1[j] -= x;
	    ax->x2[j] -= x;
	}
    }
    if(cp->tmin > ax->x2[1]) {
	x0_max = ax->x2[1];
    }
    if(ax->x1[1] < ax->x1[0]) {
	x0_min = ax->x1[1];
    }
    if(ax->x2[1] > ax->x2[0]) {
	x0_max = ax->x2[1];
    }

    ret = 1;
    for(i = 0; i < num_entries; i++)
    {
	DataEntry *e = entry[i];
	origin = getPrimaryOrigin(e->w);

	if(origin == NULL || origin->time <= NULL_TIME_CHECK)
	{
	    if(tp->predictedPhase_first_warn) {
		tp->predictedPhase_first_warn = False;
		_AxesWarning((AxesWidget)tw,
			"No origin information for some stations.");
		ret = -1;
	    }
	    continue;
	}
	tt = tp->travel_time->getTravelTime(phase, origin, e->ts->lat(),
			e->ts->lon(), e->ts->elev(), e->ts->net(),
			e->ts->sta(), &slowness, op);

	if(tt > 0.) {
	    arr_time = origin->time + tt;
	    scaled_x0.push_back(e->tbeg - arr_time);
	    wvec.push_back(e->w);
	}
    }
    Free(entry);

    for(i = 0; i < tw->c_plot.num_entries; i++)
    {
	for(j = 0; j < wvec.size(); j++) {
	    if(wvec[j] == tw->c_plot.entry[i]->w) break;
	}
	if(j == wvec.size()) {
	    wvec.push_back(tw->c_plot.entry[i]->w);
	    scaled_x0.push_back(0.);
	}
    }

    positionX2(wvec, scaled_x0, x0_min, x0_max);

    return ret;
}

static void
SaveState(TtPlotWidget w, CPlotState *state)
{
	AxesPart *ax = &w->axes;
	CPlotPart *cp = &w->c_plot;
	int i, j;

	for(i = 0; i <= ax->zoom_max; i++)
	{
	    state->x1[i] = ax->x1[i];
	    state->x2[i] = ax->x2[i];
	    state->y1[i] = ax->y1[i];
	    state->y2[i] = ax->y2[i];
	}
	state->tmin = cp->tmin;
	state->tmax = cp->tmax;
	state->zoom = ax->zoom;
	state->zoom_max = ax->zoom_max;
	state->dataSpacing = cp->dataSpacing;
	state->dataHeight = cp->dataHeight;
	state->dataSeparation = cp->dataSeparation;
	state->tmin = cp->tmin;
	state->tmax = cp->tmax;
	state->y_label_int = ax->a.y_label_int;
	state->num_saved = cp->num_entries;

	Free(state->saved);
	if( !(state->saved = (SavedEntry *)AxesMalloc((Widget)w,
			state->num_saved*sizeof(SavedEntry))) ) return;

	for(i = 0; i < cp->num_entries; i++) {
	    DataEntry *e = cp->entry[i];
	    SavedEntry *s = &state->saved[i];
	    s->entry = e;
	    s->on = e->on;
	    s->scaled_x0 = e->w->scaled_x0;
	    s->scaled_y0 = e->w->scaled_y0;
	    s->drag_yscale = e->drag_yscale;
	    s->begSelect = e->w->begSelect;
	    s->endSelect = e->w->endSelect;
	    s->tag_x = e->tag_x;
	    s->tag_y = e->tag_y;
	    s->lab_x = e->lab_x;
	    s->lab_y = e->lab_y;
	    s->n_measure_segs = e->n_measure_segs;
	    for(j = 0; j < s->n_measure_segs; j++) {
		s->measure_segs[j] = e->measure_segs[j];
	    }
	    s->left_side = e->left_side;
	    s->bottom_side = e->bottom_side;
	    s->zp_ts = e->zp_ts;
	    s->period = e->period;
	    s->amp_cnts = e->amp_cnts;
	    s->amp_ts = e->amp_ts;
	    s->amp_nms = e->amp_nms;
	}
}

static void
RestoreState(TtPlotWidget w, CPlotState *state, Boolean redraw)
{
	TtPlotPart *tp = &w->tt_plot;
	AxesPart *ax = &w->axes;
	CPlotPart *cp = &w->c_plot;
	TtPlotWidget z;
	int i, j;

	for(i = 0; i <= state->zoom_max; i++)
	{
	    ax->x1[i] = state->x1[i];
	    ax->x2[i] = state->x2[i];
	    ax->y1[i] = state->y1[i];
	    ax->y2[i] = state->y2[i];
	}
	cp->tmin = state->tmin;
	cp->tmax = state->tmax;
	ax->zoom = state->zoom;
	ax->zoom_max = state->zoom_max;
	cp->dataSpacing = state->dataSpacing;
	cp->dataHeight = state->dataHeight;
	cp->dataSeparation = state->dataSeparation;
	cp->tmin = state->tmin;
	cp->tmax = state->tmax;
	ax->a.y_label_int = state->y_label_int;

	for(i = 0; i < state->num_saved; i++) {
	    DataEntry *e = cp->entry[i];
	    SavedEntry *s = &state->saved[i];
	    e->on = s->on;
	    e->w->scaled_x0 = s->scaled_x0;
	    e->w->scaled_y0 = s->scaled_y0;
	    e->drag_yscale = s->drag_yscale;
	    e->w->begSelect = s->begSelect;
	    e->w->endSelect = s->endSelect;
	    e->tag_x = s->tag_x;
	    e->tag_y = s->tag_y;
	    e->lab_x = s->lab_x;
	    e->lab_y = s->lab_y;
	    e->n_measure_segs = s->n_measure_segs;
	    for(j = 0; j < s->n_measure_segs; j++) {
		e->measure_segs[j] = s->measure_segs[j];
	    }
	    e->left_side = s->left_side;
	    e->bottom_side = s->bottom_side;
	    e->zp_ts = s->zp_ts;
	    e->period = s->period;
	    e->amp_cnts = s->amp_cnts;
	    e->amp_ts = s->amp_ts;
	    e->amp_nms = s->amp_nms;
	}

	if(redraw) {
	    ax->num_cursors = 0;
	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw((CPlotWidget)w);
	    if((z = (TtPlotWidget)ax->mag_to) != NULL)
	    {
		int n = tp->travel_time->num_iaspei + tp->travel_time->num_jb +
		   tp->travel_time->num_regional + tp->travel_time->num_surface;
		for(i = 0; i < n; i++) {
		    Free(z->c_plot.curve[i]->s.segs);
		}
		for(i = 0; i < z->c_plot.num_entries; i++)
		{
		    Free(z->c_plot.entry[i]->sel.segs);
		    Free(z->c_plot.entry[i]->unsel.segs);
		    Free(z->c_plot.entry[i]->r.segs);
		    Free(z->c_plot.entry[i]);
		}
		CPlotMagnifyWidget((CPlotWidget)w, (CPlotWidget)z, True);
	    }
	}
}

static Boolean
SetLimits(TtPlotWidget w)
{
	AxesPart *ax = &w->axes;
	CPlotPart *cp = &w->c_plot;
	TtPlotPart *tp = &w->tt_plot;
	Boolean *position_filled = NULL;
	CssOriginClass *origin;
	int	i, j, m, n, npos, height;
	double	xmin, xmax, ymin, ymax, dx, dy, data_ymin=0., data_ymax=0.,
			tmin=0., tmax=0.;
	TtPlotWidget z;

	/* initialize using longest curve, T */

	dx = 180.*DGR2KM/tp->t_vel;
	xmin = 0.;
	xmax = 1.05*dx;
	ymin = 0.;
	ymax = 1.05*180.;
	if(tp->distance_units != DEGREES) ymax *= DGR2KM;

	n = tp->travel_time->num_iaspei + tp->travel_time->num_jb +
		tp->travel_time->num_regional + tp->travel_time->num_surface;

	for(m = 0; m < n; m++) if(cp->curve[m]->npts > 0)
	{
	    DataCurve *curve = cp->curve[m];
	    dx = curve->xmax - curve->xmin;
	    dy = curve->ymax - curve->ymin;
	    if(xmin > curve->xmin-.05*dx) xmin = curve->xmin - .05*dx;
	    if(xmax < curve->xmax+.05*dx) xmax = curve->xmax + .05*dx;
	    if(ymin > curve->ymin-.05*dy) ymin = curve->ymin - .05*dy;
	    if(ymax < curve->ymax+.05*dy) ymax = curve->ymax + .05*dy;
	}
	if(xmin > 0.) xmin = 0.;
	if(ymin > 0.) ymin = 0.;
	dy = (tp->distance_units == DEGREES) ? 180. : 180.*DGR2KM;
	if(ymax < dy) ymax = dy;

	ax->x_axis = BOTTOM;
	ax->y_axis = LEFT;
	ax->reverse_x = False;
	ax->reverse_y = True;
	ax->a.y_label_int = False;

	cp->dataSpacing = 2.;	/* degrees */
	if(tp->distance_units != DEGREES) {
	    cp->dataSpacing *= DGR2KM;
	}

	npos = (int)(ymax/cp->dataSpacing + .5) + 1;
	if(npos < cp->num_entries) npos = cp->num_entries;

	position_filled = (Boolean *)AxesMalloc((Widget)w,npos*sizeof(Boolean));

	for(i = 0; i < npos; i++) position_filled[i] = False;

	for(i = 0; i < cp->num_entries; i++)
	{
	    DataEntry *entry = cp->entry[i];
	    origin = cp->cplot_class->getPrimaryOrigin(entry->w);
	    if(origin != NULL && origin->time > NULL_TIME_CHECK)
	    {
		CssOriginClass *o = origin;
		entry->w->scaled_x0 = entry->ts->tbeg() - o->time;

		if(entry->ts->lat() > -900. && entry->ts->lon() > -900. &&
		    o->lat > -900. && o->lon > -900.)
		{
		    double delta, az, baz;
		    deltaz(o->lat, o->lon, entry->ts->lat(), entry->ts->lon(),
			 &delta, &az, &baz);
		    entry->w->scaled_y0 = delta;
		    if(tp->distance_units != DEGREES) {
		        entry->w->scaled_y0 *= DGR2KM;
		    }

		    if(tp->reduced_time && tp->reduction_velocity != 0.) {
			if(tp->distance_units != DEGREES) {
			    entry->w->scaled_x0 -= entry->w->scaled_y0/
						tp->reduction_velocity;
			}
			else {
			    entry->w->scaled_x0 -=
				DGR2KM*entry->w->scaled_y0/
					tp->reduction_velocity;
			}
		    }
		    n = (int)(delta/cp->dataSpacing + .5);
		    if(n >= npos) n = npos-1;
		    position_filled[n] = True;
		}
		else {
		    entry->w->scaled_y0 = -100.;
		}
	    }
	    else {
		entry->w->scaled_x0 = 0.;
		entry->w->scaled_y0 = -100.;
	    }
	}

	/* place entries which had no origin
	 */
	m = npos;
	for(i = 0; i < cp->num_entries; i++)
	{
	    DataEntry *entry = cp->entry[i];
	    if(entry->w->scaled_y0 == -100.) {
		for(j = 0; j < npos; j++) if(!position_filled[j])
		{
		    entry->w->scaled_y0 = j*cp->dataSpacing;
		    position_filled[j] = True;
		    break;
		}
		if(j == npos) {
		    entry->w->scaled_y0 = cp->dataSpacing*m++;
		}
	    }
	    if(i == 0) {
		data_ymin = entry->w->scaled_y0;
		data_ymax = data_ymin;
		tmin = entry->w->scaled_x0;
		tmax = tmin + entry->ts->duration();
	    }
	    else  {
		double length;
		if(data_ymin > entry->w->scaled_y0) {
		    data_ymin = entry->w->scaled_y0;
		}
		if(data_ymax < entry->w->scaled_y0) {
		    data_ymax = entry->w->scaled_y0;
		}
		if(tmin > entry->w->scaled_x0) tmin = entry->w->scaled_x0;
		length = entry->ts->duration();
		if(tmax < entry->w->scaled_x0 + length) {
		    tmax = entry->w->scaled_x0 + length;
		}
	    }
	}
	Free(position_filled);

	if(cp->num_entries > 0 && xmax < tmax) {
	    xmax = tmax;
	}

	height = abs(unscale_y(&ax->d, ax->y1[ax->zoom]) - 
			unscale_y(&ax->d, ax->y2[ax->zoom]));

	n = (int)(cp->dataSpacing*height/(ymax - ymin));
	cp->dataHeight = n-1;
	cp->dataSeparation = 1;

	ax->x1[1] = xmin;
	ax->x2[1] = xmax;
	ax->y1[1] = ymin;
	ax->y2[1] = ymax;

	if(cp->num_entries > 0) {
	    if(cp->num_entries > 1) {
		double dmax = (data_ymax < ax->y2[1]) ? data_ymax : ax->y2[1];
		double d = fabs((dmax-data_ymin)/(ax->y2[1]-ax->y1[1]));
		if(d > .2) d = 1.;
		else d *= 2.;
		if(d <= 0.) d = 1.;

		for(i = 0; i < cp->num_entries; i++) {
		    cp->entry[i]->drag_yscale = d;
		}
		data_ymin -= 2*d*.99*cp->dataSpacing;
		data_ymax += 2*d*.99*cp->dataSpacing;
	    }
	    else {
		data_ymin -= .99*cp->dataSpacing;
		data_ymax += .99*cp->dataSpacing;
	    }
	    if(ymin > data_ymin) {
		ymin = data_ymin;
	    }
	    if(ymax < data_ymax) {
		ymax = data_ymax;
	    }
	}
	ax->x1[0] = xmin;
	ax->x2[0] = xmax;
	ax->y1[0] = ymin;
	ax->y2[0] = ymax;

	ax->zoom_min = 1;
	if(cp->num_entries > 0) {
	    ax->y1[2] = data_ymin;
	    ax->y2[2] = (data_ymax < ax->y2[1]) ? data_ymax : ax->y2[1];
	    cp->tmin = tmin;
	    cp->tmax = tmax;
	    _CPlotGetXLimits((CPlotWidget)w, &ax->x1[2], &ax->x2[2]);
	    if(ax->x1[2] < ax->x1[1]) {
		ax->x1[0] = ax->x1[1] = ax->x1[2];
	    }
	    if(ax->x2[2] > ax->x2[1]) {
		ax->x2[0] = ax->x2[1] = ax->x2[2];
	    }
	    ax->zoom = 2;
	    ax->zoom_max = 2;
	}
	else {
	    ax->zoom = 1;
	    ax->zoom_max = 1;
	}

	ax->num_cursors = 0;
	_AxesRedraw((AxesWidget)w);
	 _CPlotRedraw((CPlotWidget)w);
	if((z = (TtPlotWidget)ax->mag_to) != NULL)
	{
	    n = tp->travel_time->num_iaspei + tp->travel_time->num_jb +
		tp->travel_time->num_regional + tp->travel_time->num_surface;
	    for(m = 0; m < n; m++) {
		Free(z->c_plot.curve[m]->s.segs);
	    }
	    for(m = 0; m < z->c_plot.num_entries; m++)
	    {
		Free(z->c_plot.entry[m]->sel.segs);
		Free(z->c_plot.entry[m]->unsel.segs);
		Free(z->c_plot.entry[m]->r.segs);
		Free(z->c_plot.entry[m]);
	    }
	    CPlotMagnifyWidget((CPlotWidget)w, (CPlotWidget)z, True);
	}
	w->tt_plot.restore_last_state = True;
	return True;
}

/** 
 *  Get the minimum and maximum times for all data samples as they are
 *  currently displayed in the data window. That is, get the earliest
 *  beginning and the latest ending of all the waveforms.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotGetDataDuration.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[out] tmin The epochal minimum time.
 *  @param[out] tmax The epochal maximum time.
 *  @returns True if this CPlotWidget contains any waveforms, otherwise
 *		return False.
 *  
 */
bool TtPlotClass::getDataDuration(double *tmin, double *tmax)
{
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;
    int i;
    bool data_on = false;

    if(tp->display_tt_curves)
    {
	if(state->num_saved > 0) {
	    *tmin = state->saved[0].scaled_x0;
	    *tmax = *tmin + state->saved[0].entry->ts->duration();
	    data_on = true;
	}
	for(i = 1; i < state->num_saved; i++) {
	    SavedEntry *s= &state->saved[i];
	    double t1 = s->scaled_x0;
	    double t2 = t1 + s->entry->ts->duration();
	    if(*tmin > t1) *tmin = t1;
	    if(*tmax < t2) *tmax = t2;
	}
    }
    else {
	data_on = CPlotClass::getDataDuration(tmin, tmax);
    }
    return data_on;
}

/** 
 *  Set the limits of the time axis. This function causes the data window to
 *  be immediately redrawn. The zoom history is not affected.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotSetTimeLimits.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] tmin The minimum time on the horizontal axis.
 *  @param[in] tmax The maximum time on the horizontal axis.
 */
void TtPlotClass::setTimeLimits(double tmin, double tmax)
{
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;
    AxesPart *ax = &tw->axes;
    Boolean y_label_int;
    int zoom;
    double xmin, xmax, y1, y2;

    if(tp->display_tt_curves)
    {
	double t1 = tw->c_plot.tmin;
	double t2 = tw->c_plot.tmax;
	state->tmin = tmin;
	state->tmax = tmax;
	tw->c_plot.tmin = tmin;
	tw->c_plot.tmax = tmax;
	zoom = ax->zoom;
	ax->zoom = state->zoom;
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];
	ax->y1[ax->zoom] = state->y1[ax->zoom];
	ax->y2[ax->zoom] = state->y2[ax->zoom];
	y_label_int = ax->a.y_label_int;
	ax->a.y_label_int = state->y_label_int;
	    
	_CPlotGetXLimits((CPlotWidget)tw, &xmin, &xmax);

	tw->c_plot.tmin = t1;
	tw->c_plot.tmax = t2;
	state->x1[0] = xmin;
	state->x2[0] = xmax;
	state->x1[1] = xmin;
	state->x2[1] = xmax;

	ax->y1[ax->zoom] = y1;
	ax->y2[ax->zoom] = y2;
	ax->zoom = zoom;
	ax->a.y_label_int = y_label_int;
    }
    else {
	CPlotClass::setTimeLimits(tmin, tmax);
    }
}

/** 
 *  Remove the indicated waveforms from a CPlotWidget. The waveforms
 *  associated with the Waveform structure in 'wvec' are removed
 *  from the CPlotWidget. The corresponding TimeSeries objects are
 *  unregistered and freed if they are not registered with another object
 *  or widget.  After the deletion, the remaining waveforms are moved towards
 *  the top of the data window to fill all gaps.
 *  <p>
 *  This function only removes waveforms. Any CssArrivalClasss, CssOriginClasss,
 *   CssAssocs, etc. that are associated with the waveforms are not removed.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotDeleteWaveforms.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] num The number of waveforms to be deleted.
 *  @param[in] wvec The Waveform pointers for 'num' waveforms.
 */
void TtPlotClass::deleteWaveforms(gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &tw->c_plot;
    int i, j;

    for(i = 0; i < wvec.size(); i++)
    {
	for(j = 0; j < cp->num_entries; j++) {
	    if(wvec[i] == cp->entry[j]->w) break;
	}
	if(j < cp->num_entries) {
	    DeleteSaved(tw, cp->entry[j]);
	}
    }
    CPlotClass::deleteWaveforms(wvec);
    tw->tt_plot.restore_last_state = False;
}

/** 
 *  Remove the indicated waveforms from a CPlotWidget. The waveforms
 *  associated with the Waveform structure in 'wvec' are removed
 *  from the CPlotWidget. The corresponding TimeSeries objects are
 *  unregistered and freed if they are not registered with another object
 *  or widget. The remaining waveforms are not repositioned in the data
 *  window.
 *  <p>
 *  This function only removes waveforms. Any CssArrivalClasss, CssOriginClasss,
 *  CssAssocs, etc. that are associated with the waveforms are not removed.
 *  <p>
 *  Use this function for TtPlotWidgets instead of
 *  CPlotDeleteWaveforms_NoReposition.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] num The number of waveforms to be deleted.
 *  @param[in] wvec The Waveform pointers for 'num' waveforms.
 */
void TtPlotClass::deleteWaveforms_NoReposition(gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &tw->c_plot;
    int i, j;

    for(i = 0; i < wvec.size(); i++)
    {
	for(j = 0; j < cp->num_entries; j++) {
	    if(wvec[i] == cp->entry[j]->w) break;
	}
	if(j < cp->num_entries) {
	    DeleteSaved(tw, cp->entry[j]);
	}
    }
    CPlotClass::deleteWaveforms_NoReposition(wvec);
    tw->tt_plot.restore_last_state = False;
}

/** 
 *  Remove all selected waveforms from a CPlotWidget. All selected
 *  TimeSeries objects are unregistered. Only waveforms are removed. This
 *  function does not remove any CssArrivalClasss, CssOriginClasss, etc.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotDeleteData.
 */
void TtPlotClass::deleteSelectedWaveforms()
{
    CPlotPart *cp = &tw->c_plot;

    if(!XtIsRealized((Widget)tw)) return;

    for(int i = cp->num_entries-1; i >= 0; i--)
    {
	if(cp->entry[i]->w->selected) DeleteSaved(tw, cp->entry[i]);
    }
    CPlotClass::deleteSelectedWaveforms();
    tw->tt_plot.restore_last_state = False;
}

static void
DeleteSaved(TtPlotWidget w, DataEntry *entry)
{
        TtPlotPart *tp = &w->tt_plot;
	CPlotState *state = &tp->state;
	int i, j;

	for(i = 0; i < state->num_saved; i++) {
	    if(state->saved[i].entry == entry) {
		for(j = i; j < state->num_saved-1; j++) {
		    state->saved[i] = state->saved[i+1];
		}
		state->num_saved--;
	 	return;
	    }
	}
}

/** 
 *  Remove all waveforms (TimeSeries), CssArrivalClasss, CssAssocs, CssOriginClasss,
 *  CssOrigerrs, and CssWftags from a CPlotWidget. All cursors are also removed.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotClear.
 */
void TtPlotClass::clear(bool do_callback)
{
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;
    CPlotClass::clear();
    state->num_saved = 0;
    Free(state->saved);
    tw->tt_plot.restore_last_state = False;
}

/** 
 *  Remove all waveforms from a CPlotWidget. All TimeSeries objects are
 *  unregistered. Only waveforms are removed. This function does not remove
 *  any CssArrivalClasss, CssOriginClasss, etc.  All cursors are also removed.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotClearWaveforms.
 */
void TtPlotClass::clearWaveforms(bool do_callback)
{
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;
    CPlotClass::clearWaveforms(do_callback);
    state->num_saved = 0;
    Free(state->saved);
    tw->tt_plot.restore_last_state = False;
}

/** 
 *  Set the y or vertical position of the indicated waveforms.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotPositionY.
 */
void TtPlotClass::positionY(gvector<Waveform *> &wvec,vector<double> &scaled_y0)
{
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;

    if(!tp->display_tt_curves) {
	CPlotClass::positionY(wvec, scaled_y0);
    }
    else
    {
	for(int i = 0; i < wvec.size(); i++) {
	    for(int j = 0; j < state->num_saved; j++) {
		if(state->saved[j].entry->w == wvec[i]) {
		    if(state->saved[j].entry->type == CPLOT_DATA) {
			state->saved[j].scaled_y0 = scaled_y0[i];
		    }
		    break;
		}
	    }
	}
    }
}

/** 
 *  Reorder the vertical positions of the waveforms. The vertical positions
 *  of the waveforms will be ordered, from top down, by the order of the
 *  Waveform pointers in 'wvec'. Any waveforms not included in
 *  'wvec' will be appended at the bottom of the display.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] num The number of Waveform pointers in 'wvec'.
 *  @param[in] wvec An array of Waveform structure pointers associated
 *			with waveforms in this TtPlotWidget.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotSetWaveformOrder.
 */
void TtPlotClass::setWaveformOrder(gvector<Waveform *> &wvec, bool redisplay)
{
    CPlotPart *cp = &tw->c_plot;
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;

    if(!tp->display_tt_curves) {
	CPlotClass::setWaveformOrder(wvec, redisplay);
    }
    else
    {
	CPlotClass::setWaveformOrder(wvec, false);

	for(int i = 0; i < cp->num_entries; i++) {
	    for(int j = 0; j < state->num_saved; j++) {
		if(state->saved[j].entry == cp->entry[i]) {
		    if(state->saved[j].entry->type == CPLOT_DATA) {
			state->saved[j].scaled_y0 = i+1;
		    }
		    break;
		}
	    }
	}
    }
}

/** 
 *  Set the display-flag for waveforms.
 *  @see CPlotDisplayComponents
 */
void TtPlotClass::setDataDisplay(gvector<Waveform *> &wvec, vector<bool> &on,
			bool promote_visible, bool redisplay)
{
    CPlotPart *cp = &tw->c_plot;
    TtPlotPart *tp = &tw->tt_plot;
    CPlotState *state = &tp->state;

    if(!tp->display_tt_curves) {
	CPlotClass::setDataDisplay(wvec, on, promote_visible, redisplay);
    }
    else
    {
	CPlotClass::setDataDisplay(wvec, on, false, false);

	for(int i = 0; i < cp->num_entries; i++) {
	    for(int j = 0; j < state->num_saved; j++) {
		if(state->saved[j].entry == cp->entry[i]) {
		    if(state->saved[j].entry->type == CPLOT_DATA) {
			state->saved[j].on = cp->entry[i]->on;
		    }
		    break;
		}
	    }
	}
    }
}

/** 
 *  Set the waveform data height and data separation for a CPlotWidget.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] dataHeight The data height in pixels.
 *  @param[in] dataSeparation The data separation in pixels.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotSetDataHeight.
 */
void TtPlotClass::setDataHeight(int dataHeight, int dataSeparation)
{
    TtPlotPart *tp = &tw->tt_plot;

    if(dataHeight < 1) dataHeight = 1;
    if(dataSeparation < 1) dataSeparation = 1;

    if(!tp->display_tt_curves) {
	CPlotClass::setDataHeight(dataHeight, dataSeparation);
    }
    else
    {
	tw->c_plot.dataHeight = dataHeight;
	_CPlotRedraw((CPlotWidget)tw);
	_AxesRedisplayAxes((AxesWidget)tw);
	_CPlotRedisplay((CPlotWidget)tw);
	_AxesRedisplayXor((AxesWidget)tw);
    }
}

static void
fminmax(int n, float *a, double *min, double *max)
{
	if(n <= 0) return;
	*min = *max = *a;
	/* isop_c get_seg() uses set_fnan to denote a break in the curve
	 */
	while(n > 0 && fNaN((*a))) {
	    a++;
	    n--;
	}
	
	if(n > 0) *min = *max = *a;

	while(n-- > 0) {
	    if(!fNaN((*a))) {
		*min = (*min < *a) ? *min : *a;
		*max = (*max > *a) ? *max : *a;
	    }
	    a++;
	}
}

static void
dminmax(int n, double *a, double *min, double *max)
{
	if(n <= 0) return;
	*min = *max = *a;
	while(n-- > 0) {
	    if(*min > *a) *min = *a;
	    if(*max < *a) *max = *a;
	    a++;
	}
}

/** 
 *  Create a TtPlotWidget.
 *  
 */
Widget
TtPlotCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
	return (XtCreateWidget(name, ttPlotWidgetClass, parent, 
		arglist, argcount));
}

static int
sort_by_depth(const void *A, const void *B)
{
	DataEntry **a = (DataEntry **)A;
	DataEntry **b = (DataEntry **)B;
	if((*a)->depth > (*b)->depth) return 1;
	if((*a)->depth < (*b)->depth) return -1;
	return 0;
}

static void
JBcurve(TtPlotWidget w, const string &phase, float depth, int *npts, float *y,
		float *x)
{
	TtPlotPart *tp = &w->tt_plot;
	int	i, n, code, err, num_jb=13;
	float	delta, ddel, min, max, ttime;
	Derivatives der;
	struct
	{
	    const char  *phase;
	    float delta_min, delta_max;
	} jb[] =
	{
	    {"P",       0.,105.}, {"PP",      0.,210.}, {"S",       0.,107.},
	    {"SS",      0.,214.}, {"PcP",     0.,100.}, {"pP",      0.,105.},
	    {"sP",      0.,105.}, {"ScS",     0.,100.}, {"PKPab", 141.,180.},
	    {"PKPbc", 141.,180.}, {"PKPdf", 109.,180.}, {"SKSac",  62.,133.},
	    {"SKSdf",  99.,180.},
	};
	int jb_code[] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 17,
	};

	if(!TravelTime::openjb(tp->jb_table)) return;

	*npts = 0;

	for(i = 0; i < num_jb; i++) {
	    if(!phase.compare(jb[i].phase)) break;
	}

	if(i == num_jb) return;

	code = jb_code[i];
	min = jb[i].delta_min;
	max = jb[i].delta_max;
	ddel = (max - min)/199;

	for(i = n = 0; i < 200; i++)
	{
	    delta = min + i*ddel;
		
	    if(!(err = jbsim(code, delta, depth, &ttime, &der)))
	    {
		x[n] = ttime;
		y[n] = delta;
		n++;
	    }
	    else if(err == CANNOT_OPEN_JBTABLE)
	    {
		if(w->tt_plot.jb_first_warn) {
		    w->tt_plot.jb_first_warn = False;
		    _AxesWarn((AxesWidget)w, "jbTable file is not open.");
		}
		return;
	    }
	    else if(err == JBTABLE_READ_ERROR)
	    {
		if(w->tt_plot.jb_first_warn) {
		    w->tt_plot.jb_first_warn = False;
		    _AxesWarn((AxesWidget)w, "JB table read error.");
		}
		return;
	    }
	    else if(n > 0 && finite(y[n-1]))
	    {
		set_fnan(y[n++]);
	    }
	}
	*npts = n;
}

/** 
 *  Add a GTimeSeries to a TtPlotWidget. The CPlotInputStruct holds the
 *  parameters that control how the waveform will be displayed in the
 *  CPlotWidget window.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] ts A GTimeSeries object.
 *  @param[in] input input structure with waveform information.
 *  	- input->display_t0 The time at which to position the first sample of the TimeSeries.
 *  	- input->tag The tag or label to be displayed next to the TimeSeries.
 *  	- input->sta The station name for the TimeSeries.
 *  	- input->chan The channel name for the TimeSeries.
 *  	- input->net The network name for the TimeSeries.
 *  	- input->lat The station latitude in degrees. Can be set to -999.
 *  	- input->lon The station longitude in degrees. Can be set to -999.
 *  	- input->elev The station elevation in meters. Can be set to -999.
 *  	- input->origin An optional origin to be associated with the TimeSeries. Can be NULL.
 *      - input->pixel The color used to draw the TimeSeries.
 *  	- input->on The initial visibility of the TimeSeries. If on is True,
 *			the TimeSeries is drawn immediately, otherwise it is
 *			not drawn until a later function call changes this
 *			parameter.
 *  @returns A Waveform object for the TimeSeries, or NULL if the
 *		TimeSeries as no samples or one of the
 *		CPlotInputStruct parameters is invalid.
 */
Waveform * TtPlotClass::addTimeSeries(GTimeSeries *ts, CPlotInputStruct *input)
{
    TtPlotPart *tp = &tw->tt_plot;
    bool on=false;
    int i;
    CPlotState state;
    Waveform *cd;

    if(tp->display_tt_curves) {
	state.saved = NULL;
	state.num_saved = 0;
	SaveState(tw, &state);
	RestoreState(tw, &tp->state, False);
	on = input->on;
	input->on = False;
    }
    if( (cd = CPlotClass::addTimeSeries(ts, input)) )
    {
	DataEntry *e;
	for(i = 0; i < tw->c_plot.num_entries; i++) {
	    if(tw->c_plot.entry[i]->w == cd) break;
	}
	e = tw->c_plot.entry[i];

	if(tp->display_tt_curves) {
	    e->on = on;
	    SaveState(tw, &tp->state);
	    RestoreState(tw, &state, False);
	    Free(state.saved);
	    e->on = False;
	    positionEntry(tw, cd, on);
	}

	if(tp->display_tt_labels && SelectedPhases(tw) && e->origin)
	{
	    TrTm tr;
	    tp->travel_time->getAllTimes(&tr, e->origin, e->ts->lat(),
			e->ts->lon(), e->ts->elev(), e->ts->net(),e->ts->sta());
	    DoPredLabel(tw, e, &tr);
	}
    }
    tw->tt_plot.restore_last_state = false;
    return(cd);
}

static void
positionEntry(TtPlotWidget w, Waveform *cd, Boolean on)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	CPlotWidget z = (CPlotWidget)ax->mag_to;
	Boolean *position_filled = NULL;
	int i, m, n, npos;
	double xmin, xmax, ymax, tmin, tmax;
	DataEntry *entry;
	CssOriginClass *o;

	for(i = 0; i < cp->num_entries; i++) {
	    if(cp->entry[i]->w == cd) break;
	}
	if(i == cp->num_entries) return;
	entry = cp->entry[i];

	/* erase entry at current position */
	_CPlotRedisplayEntry((CPlotWidget)w, entry);
	if(z != NULL)
	{
	    CPlotPart *zp = &z->c_plot;
	    DataEntry *ze = zp->entry[z->c_plot.num_entries-1];
	    _CPlotRedisplayEntry(z, ze);
	}

	entry->on = on;

	ymax = ax->y2[1];

	npos = (int)(ymax/cp->dataSpacing + .5) + 1;
	if(npos < cp->num_entries) npos = cp->num_entries;

	position_filled = (Boolean *)AxesMalloc((Widget)w,npos*sizeof(Boolean));

	for(i = 0; i < npos; i++) position_filled[i] = False;

	for(i = 0; i < cp->num_entries-1; i++)
	{
	    n = (int)(cp->entry[i]->w->scaled_y0/cp->dataSpacing + .5);
	    if(n >= npos) n = npos-1;
	    position_filled[n] = True;
	}

	o = cp->cplot_class->getPrimaryOrigin(entry->w);

	if(o != NULL && o->time > NULL_TIME_CHECK)
	{
	    entry->w->scaled_x0 = entry->ts->tbeg() - o->time;

	    if(entry->ts->lat() > -900. && entry->ts->lon() > -900. &&
		    o->lat > -900. && o->lon > -900.)
	    {
		double delta, az, baz;
		deltaz(o->lat, o->lon, entry->ts->lat(), entry->ts->lon(),
			 &delta, &az, &baz);
		entry->w->scaled_y0 = delta;

		n = (int)(delta/cp->dataSpacing + .5);
		if(n >= npos) n = npos-1;
		position_filled[n] = True;
	    }
	    else {
		entry->w->scaled_y0 = -100.;
	    }
	}
	else {
	    entry->w->scaled_x0 = 0.;
	    entry->w->scaled_y0 = -100.;
	}

	/* place entries which had no origin
	 */
	m = npos;
	if(entry->w->scaled_y0 == -100.) {
	    for(i = 0; i < npos; i++) if(!position_filled[i])
	    {
		entry->w->scaled_y0 = i*cp->dataSpacing;
		position_filled[i] = True;
		break;
	    }
	    if(i == npos) {
		entry->w->scaled_y0 = cp->dataSpacing*m++;
	    }
	}
	tmin = entry->w->scaled_x0;
	tmax = tmin + entry->ts->duration();

	if(tmin < cp->tmin) cp->tmin = tmin;
	if(tmax > cp->tmax) cp->tmax = tmax;

	_CPlotGetXLimits((CPlotWidget)w, &xmin, &xmax);

	if(xmin < ax->x1[0]) ax->x1[0] = xmin;
	if(xmax > ax->x2[0]) ax->x2[0] = xmax;

	if(ax->zoom > 1) {
	    if(xmin < ax->x1[1]) ax->x1[1] = xmin;
	    if(xmax > ax->x2[1]) ax->x2[1] = xmax;
	}
	_Axes_AdjustScrollBars((AxesWidget)w);

	if(_CPlotVisible((CPlotWidget)w, entry))
	{
	    _CPlotRedrawEntry((CPlotWidget)w, entry);
	    if(!ax->redisplay_pending)
	    {
		_CPlotRedisplayEntry((CPlotWidget)w, entry);
	    }
	    if(z != NULL)
	    {
		CPlotPart *zp = &z->c_plot;
		DataEntry *ze = zp->entry[z->c_plot.num_entries-1];
		_CPlotVisible((CPlotWidget)z, ze);
		_CPlotRedrawEntry(z, ze);
		_CPlotRedisplayEntry(z, ze);
	    }
	    if(ax->use_pixmap && XtIsRealized((Widget)w))
	    {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w),
			ax->axesGC, 0, 0, w->core.width, w->core.height, 0, 0);
	    }
	}
}

/** 
 *  
 */
void TtPlotClass::changePredArr(int n, InputPredArr *in_pred_arr,int action)
{
    TtPlotPart *tp = &tw->tt_plot;
    int i, j, npred, num_entries, n_in_use;
    DataEntry **entry = NULL;
    CPlotPredArr pred_arr[200];

    entry = (DataEntry **)AxesMalloc((Widget)tw,
		tw->c_plot.num_entries*sizeof(DataEntry *));

    num_entries = 0;
    for(i = 0; i < tw->c_plot.num_entries; i++)
    {
	entry[num_entries++] = tw->c_plot.entry[i];
    }

    /* add */
    if (action == 0)
    {
	for(j = tp->prev_n, i = 0; i < n; j++, i++) {
	    memcpy(&tp->prev_pred_arr[j], &in_pred_arr[i],sizeof(InputPredArr));
	}
	tp->prev_n += n;
    }
    /* delete */
    else if (action == 1)
    {
	for(i =0 ; i < n; i++)
	{
	    for(j = 0; j < tp->prev_n; j++) {
		if(tp->prev_pred_arr[j].arid == in_pred_arr[i].arid &&
		   tp->prev_pred_arr[j].group_id == in_pred_arr[i].group_id &&
			tp->prev_pred_arr[j].in_use)
		{
		    tp->prev_pred_arr[j].in_use = False;
		    break;
		}
	    }
	}
    }
    /* update */
    else if (action == 2)
    {
	for(j = tp->prev_n-n, i = 0; i < n; j++, i++) {
	    memcpy(&tp->prev_pred_arr[j],&in_pred_arr[i],sizeof(InputPredArr));
	}
    }

    n_in_use = 0;
    for(i = 0; i < num_entries; i++)
    {
	npred = 0;
	for (j=0; j<tp->prev_n; j++) {
	    if(tp->prev_pred_arr[j].in_use &&
		   (!strcasecmp(tp->prev_pred_arr[j].sta, entry[i]->ts->net())
		|| !strcasecmp(tp->prev_pred_arr[j].sta, entry[i]->ts->sta())) )
	    {
		stringcpy(pred_arr[npred].phase, tp->prev_pred_arr[j].phase,
			sizeof(pred_arr[npred].phase));
		pred_arr[npred].fg = tp->prev_pred_arr[j].fg;
		pred_arr[npred].time = tp->prev_pred_arr[j].time;
		npred++;	
	    }
	}

	for(j = 0; j < npred; j++) {
		get_label_points(tw, &pred_arr[j]);
	}
	n_in_use += npred;
	_CPlotPredArrivals((CPlotWidget)tw, entry[i]->w, pred_arr, npred);
    }
    if(n_in_use == 0) tp->prev_n = 0;
    Free(entry);
}

/** 
 *  Set the component display mode. Choices for the mode are (from CPlot.h):
 * <p> <TT>
 *  CPLOT_ALL_COMPONENTS </TT>- display all components of three-component data.
 * <p> <TT>
 *  CPLOT_Z_COMPONENT </TT>- display only the z-component of three-component
 *				data.
 * <p> <TT>
 *  CPLOT_N_COMPONENT </TT>- display only the n-component of three-component
 *				data.
 * <p> <TT>
 *  CPLOT_E_COMPONENT </TT>- display only the e-component of three-component
 *				data.
 * <p> <TT>
 *  CPLOT_HORIZONTAL_COMPONENTS </TT>- display only data for which an
 *				n-component and an e-component are present.
 * <p>
 *  This function will cause the data window to be redrawn, if the input
 *  component_display_mode is different than the current component_display_mode.
 *  <p>
 *  Use this function for TtPlotWidgets instead of CPlotDisplayComponents.
 *
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] component_display_mode The component display mode.
 */
void TtPlotClass::displayComponents(int component_display_mode)
{
    CPlotPart *cp = &tw->c_plot;
    TtPlotPart *tp = &tw->tt_plot;
    CPlotWidget z;

    if(cp->display_components != component_display_mode)
    {
	if(!tp->display_tt_curves) {
	    CPlotClass::displayComponents(component_display_mode);
	    tw->tt_plot.restore_last_state = False;
	}
	else {
	    CPlotState state;
	    state.saved = NULL;
	    state.num_saved = 0;
	    SaveState(tw, &state);
	    RestoreState(tw, &tp->state, False);
	    cp->display_components = component_display_mode;
	    _CPlotComponentsOnOff((CPlotWidget)tw, True);
	    SaveState(tw, &tp->state);
	    RestoreState(tw, &state, False);
	    Free(state.saved);

	    _CPlotComponentsOnOff((CPlotWidget)tw, False);
	    _AxesRedraw((AxesWidget)tw);
	    _CPlotRedraw((CPlotWidget)tw);
	    _AxesRedisplayAxes((AxesWidget)tw);
	    _CPlotRedisplay((CPlotWidget)tw);
	    _AxesRedisplayXor((AxesWidget)tw);
	    if((z = (CPlotWidget)tw->axes.mag_to) != NULL)
	    {
		CPlotMagnifyWidget((CPlotWidget)tw, NULL, True);
		CPlotMagnifyWidget((CPlotWidget)tw, z, True);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
    }
}

void TtPlotClass::updatePredicted()
{
    CPlotPart *cp = &tw->c_plot;
    TtPlotPart *tp = &tw->tt_plot;
    AxesPart *ax = &tw->axes;

    if(tp->display_tt_curves)
    {
	int i;
	for(i = 0; i < cp->num_entries; i++) {
	    positionEntry(tw, cp->entry[i]->w, cp->entry[i]->on);
	}
    }
    if(tp->display_tt_labels)
    {
	DoAllPredLabels(tw);

	if(ax->use_pixmap && XtIsRealized((Widget)tw)) {
	    XCopyArea(XtDisplay(tw), ax->pixmap, XtWindow(tw),
		ax->axesGC, 0, 0, tw->core.width, tw->core.height, 0, 0);
	}
    }
}

void TtPlotClass::setSourceDepth(double depth)
{
    TtPlotPart *tp = &tw->tt_plot;
    int i;

    if(tp->source_depth == depth) return;

    tp->source_depth = depth;

    for(i = 0; i < tp->travel_time->num_iaspei; i++) {
	tp->iaspei_phases[i].made = False;
    }
    for(i = 0; i < tp->travel_time->num_jb; i++) {
	tp->jb_phases[i].made = False;
    }
    for(i = 0; i < tp->travel_time->num_regional; i++) {
	tp->regional_phases[i].made = False;
    }

    if(tp->display_tt_curves)
    {
	if( updateCurves(tw, False) )
	{
	    TtPlotWidget z = (TtPlotWidget)tw->axes.mag_to;

	    _CPlotRedraw((CPlotWidget)tw);
	    _AxesRedisplayAxes((AxesWidget)tw);
	    _CPlotRedisplay((CPlotWidget)tw);
	    _AxesRedisplayXor((AxesWidget)tw);

	    if(z != NULL)
	    {
		_AxesRedraw((AxesWidget)z);
		_CPlotRedraw((CPlotWidget)z);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay((CPlotWidget)z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
    }
}

void TtPlotClass::updateForOriginChange()
{
    if(tw->tt_plot.display_tt_labels)
    {
	DoAllPredLabels(tw);

	_AxesRedisplayAxes((AxesWidget)tw);
	_CPlotRedisplay((CPlotWidget)tw);
	_AxesRedisplayXor((AxesWidget)tw);
    }
}
