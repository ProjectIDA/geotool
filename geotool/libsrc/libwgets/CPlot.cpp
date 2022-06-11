/** \file CPlot.cpp
 *  \brief Defines widget CPlot.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      CPlot Widget -- widget for draw x-y graphs.
 *
 * SYNOPSIS
 *      #include "CPlot.h"
 *      Widget
 *      CPlotCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      CPlot.h
 *
 * BUGS
 *
 * NOTES
 *      To Do:
 *
 * AUTHOR
 *      I. Henson -- July 1990
 *	Teledyne Geotech
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <Xm/XmAll.h>

#ifdef HAVE_GSL
#include <gsl/gsl_spline.h>
#endif

extern "C" {
#include "libdrawx.h"
#include "libstring.h"
#include "libgmath.h"
}

#include "widget/Axes.h"
#include "widget/CPlotP.h"
#include "gobject++/CssTables.h"
#include "DataMethod.h"
#include "libgio.h"
#include "widget/CPlotClass.h"

#define MAPALF

#define insideClip(ax,event) (	((XButtonEvent *)event)->x >= ax->clipx1 && \
				((XButtonEvent *)event)->x <= ax->clipx2 && \
				((XButtonEvent *)event)->y >= ax->clipy1 && \
				((XButtonEvent *)event)->y <= ax->clipy2 \
			     )

#define XtRCPlotInt	(char *)"CPlotInt"

#define	offset(field)	XtOffset(CPlotWidget, c_plot.field)
static XtResource	resources[] = 
{
    {XtNallowPartialSelect, XtCAllowPartialSelect, XtRBoolean, sizeof(Boolean),
		offset(allow_partial_select),XtRString, (XtPointer)"True"},
    {XtNhighlightPoints, XtCHighlightPoints, XtRBoolean, sizeof(Boolean),
		offset(highlight_points),XtRString, (XtPointer)"True"},
    {XtNdisplayTags, XtCDisplayTags, XtRBoolean,sizeof(Boolean),
		offset(display_tags), XtRString, (XtPointer)"True"},
    {XtNtagFont, XtCTagFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(tag_font), XtRString, 
		(XtPointer)"-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*"},
    {XtNarrivalFont, XtCArrivalFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(arrival_font), XtRString, 
		(XtPointer)"-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"},
    {XtNassociatedArrivalFont, XtCAssociatedArrivalFont, XtRFontStruct,
		sizeof(XFontStruct *),offset(associated_arrival_font),XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*"},
    {XtNampFont, XtCAmpFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(amp_font), XtRString, 
		(XtPointer)"-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"},
    {XtNdataMovement, XtCDataMovement, XtRCPlotInt, sizeof(int),
		offset(data_movement), XtRString,
		(XtPointer)"CPLOT_NO_MOVEMENT"},
    {XtNscrollData, XtCScrollData, XtRBoolean, sizeof(Boolean),
		offset(scroll_data), XtRString, (XtPointer)"True"},
    {XtNadjustTimeLimits, XtCAdjustTimeLimits, XtRBoolean, sizeof(Boolean),
		offset(adjust_time_limits), XtRString, (XtPointer)"True"},
    {XtNlimitSelect, XtCLimitSelect, XtRBoolean,sizeof(Boolean),
		offset(limit_select), XtRString, (XtPointer)"False"},
    {XtNdisplayArrivals, XtCDisplayArrivals, XtRCPlotInt, sizeof(int),
		offset(display_arrivals), XtRString,
		(XtPointer)"CPLOT_ARRIVALS_OFF"},
    {XtNdisplayPredictedArrivals, XtCDisplayPredictedArrivals, XtRBoolean,
		sizeof(Boolean), offset(display_predicted_arrivals),
		XtRString, (XtPointer)"False"},
    {XtNdisplayAmplitudeScale, XtCDisplayAmplitudeScale, XtRBoolean,
		sizeof(Boolean), offset(display_amplitude_scale),
		XtRString, (XtPointer)"False"},
    {XtNdisplayAssocOnly, XtCDisplayAssocOnly, XtRBoolean, sizeof(Boolean),
		offset(display_assoc_only), XtRString, (XtPointer)"False"},
    {XtNcurvesOnly, XtCCurvesOnly, XtRBoolean,sizeof(Boolean),
		offset(curves_only), XtRString, (XtPointer)"False"},
    {XtNalignArrSelected, XtCAlignArrSelected, XtRBoolean, sizeof(Boolean),
		offset(align_arr_selected), XtRString, (XtPointer)"True"},
    {XtNampInterpolation, XtCAmpInterpolation, XtRBoolean, sizeof(Boolean),
		offset(amp_interpolation), XtRString, (XtPointer)"True"},
    {XtNthreeHalfCycles, XtCThreeHalfCycles, XtRBoolean, sizeof(Boolean),
		offset(three_half_cycles), XtRString, (XtPointer)"False"},
/*
    {XtNcomponentTimeLimit, XtCComponentTimeLimit, XtRDouble, sizeof(double),
		offset(component_time_limit), XtRString, (XtPointer)"600."},
*/
    {XtNmeasureBox, XtCMeasureBox, XtRBoolean, sizeof(Boolean),
		offset(measure_box), XtRString, (XtPointer)"False"},
    {XtNdataHeight, XtCDataHeight, XtRInt, sizeof(int),
		offset(dataHeight), XtRImmediate, (XtPointer)50},
    {XtNdataSeparation, XtCDataSeparation, XtRInt, sizeof(int),
		offset(dataSeparation), XtRImmediate, (XtPointer)2},
/*
    {XtNdataSpacing, XtCDataSpacing, XtRDouble, sizeof(double),
		offset(dataSpacing), XtRString, (XtPointer)"1."},
*/
    {XtNsingleArrSelect, XtCSingleArrSelect, XtRBoolean, sizeof(Boolean),
		offset(single_arr_select), XtRString, (XtPointer)"False"},
    {XtNjoinTimeSeries, XtCJoinTimeSeries, XtRBoolean, sizeof(Boolean),
		offset(join_timeSeries), XtRString, (XtPointer)"False"},
/*
    {XtNjoinTimeLimit, XtCJoinTimeLimit, XtRDouble, sizeof(double),
		offset(join_time_limit), XtRString, (XtPointer)"7200."},
    {XtNoverlapLimit, XtCOverlapLimit, XtRDouble,sizeof(double),
		offset(overlap_limit), XtRString, (XtPointer)"30."},
*/
    {XtNredrawSelectedData, XtCRedrawSelectedData, XtRBoolean, sizeof(Boolean),
		offset(redraw_selected_data), XtRString, (XtPointer)"True"},
    {XtNdisplayAddArrival, XtCDisplayAddArrival, XtRBoolean, sizeof(Boolean),
		offset(display_add_arrival), XtRString, (XtPointer)"False"},
/*
    {XtNinfoDelay, XtCInfoDelay, XtRDouble, sizeof(double),
		offset(info_delay), XtRString, (XtPointer)"1.0"},
*/
    {XtNsingleSelectDataCallback, XtCSingleSelectDataCallback, XtRCallback,
		sizeof(XtCallbackList), offset(single_select_data_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNselectDataCallback, XtCSelectDataCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_data_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNpositionCallback, XtCPositionCallback, XtRCallback,
		sizeof(XtCallbackList), offset(position_callbacks), XtRCallback,
		(XtPointer)NULL},
    {XtNpositionDragCallback, XtCPositionDragCallback, XtRCallback,
		sizeof(XtCallbackList), offset(position_drag_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNmeasureCallback, XtCMeasureCallback, XtRCallback,
		sizeof(XtCallbackList), offset(measure_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNampConvertCallback, XtCAmpConvertCallback, XtRCallback,
		sizeof(XtCallbackList), offset(amp_convert_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNretimeCallback, XtCRetimeCallback, XtRCallback,
		sizeof(XtCallbackList), offset(retime_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNretimeDragCallback, XtCRetimeDragCallback, XtRCallback,
		sizeof(XtCallbackList), offset(retime_drag_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNselectArrivalCallback, XtCSelectArrivalCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_arrival_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNwaveformMenuCallback, XtCWaveformMenuCallback,
		XtRCallback, sizeof(XtCallbackList),
		offset(waveform_menu_callbacks), XtRCallback, (XtPointer)NULL},
    {XtNarrivalMenuCallback, XtCArrivalMenuCallback,XtRCallback,
		sizeof(XtCallbackList), offset(arrival_menu_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNwaveformInfoCallback, XtCWaveformInfoCallback,
		XtRCallback, sizeof(XtCallbackList),
		offset(waveform_info_callbacks), XtRCallback, (XtPointer)NULL},
    {XtNarrivalInfoCallback, XtCArrivalInfoCallback,XtRCallback,
		sizeof(XtCallbackList), offset(arrival_info_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNmodifyWaveformCallback, XtCModifyWaveformCallback,
		XtRCallback, sizeof(XtCallbackList),
		offset(modify_waveform_callbacks), XtRCallback,(XtPointer)NULL},
    {XtNmenuCallback, XtCMenuCallback, XtRCallback,
		sizeof(XtCallbackList), offset(menu_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNaddArrivalCallback, XtCAddArrivalCallback, XtRCallback,
		sizeof(XtCallbackList), offset(add_arrival_callbacks),
		XtRCallback, (XtPointer)NULL},
    {XtNaddArrivalMenuCallback, XtCAddArrivalMenuCallback, XtRCallback,
		sizeof(XtCallbackList), offset(add_arrival_menu_callbacks),
		XtRCallback, (XtPointer)NULL},
};
#undef offset

/* Private functions */

static void CvtStringToCPlotInt(XrmValuePtr *args, Cardinal *num_args,
				XrmValuePtr fromVal, XrmValuePtr toVal);
static void ClassInitialize(void);
static void Initialize(Widget req, Widget nou);
static void Realize(Widget w, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static Boolean SetValues(CPlotWidget cur, CPlotWidget req, CPlotWidget nou);
static void CPlotRedraw(AxesWidget w, int type, double shift);
static void CPlotResize(AxesWidget w);
static void CPlotHardCopy(AxesWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, PrintParam *p);
static void _CPlotSelectData(AxesWidget w);
static void Destroy(Widget w);
static void SortEntries(CPlotWidget w);
static void DisplayNewCurve(CPlotWidget w, DataCurve *curve, Boolean redraw);
static Boolean InitDataEntry(CPlotClass *cplot, CPlotWidget w, DataEntry *entry,
			GTimeSeries *ts, CPlotInputStruct *input);
static void FindComponents(CPlotWidget w, DataEntry *entry);
static void DoDeci(CPlotWidget w, DataEntry *entry);
static void HardDrawPredArrivals(CPlotWidget w, FILE *fp, Boolean do_arrPlace,
			float font_scale, DrawStruct *d, DataEntry *entry,
			int fontsize);
static void HardDrawOneArrival(CPlotWidget w, FILE *fp, Boolean do_arrPlace,
			float font_scale, DrawStruct *d, int i, int j);
static void HardDrawScale(CPlotWidget w, FILE *fp, AxesParm *hard_a,
		DrawStruct *hard_d, SegArray *s, float font_scale,
		Boolean full_scale, DataEntry *entry, int tag_x, int tag_y);
static void fminmax(int n, float *a, double *min, double *max, double mean);
static void dminmax(int n, double *a, double *min, double *max, double mean);
static Boolean visibleMinMax(CPlotWidget w, DataEntry *entry);
static void Visible(CPlotWidget w);
static void DoClipping(CPlotWidget w, int nsegs, RSeg *segs, float x0, float y0,
			SegArray *m);
static void DrawLabel(CPlotWidget w, DataCurve *curve);
static void DisplayTag(CPlotWidget w, DataEntry *entry);
static Boolean chan_check(DataEntry *entry, int chan_type);
static void MagEntry(CPlotWidget w, DataEntry *we);
static void MagCurve(CPlotWidget w, DataCurve *we);
static void Motion(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void LeaveWindow(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void EnterWindow(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void MoveEntry(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void _CPlotMouseDownMeasure(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void ZCompOnly(CPlotWidget w, XEvent *event);
static void ECompOnly(CPlotWidget w, XEvent *event);
static void NCompOnly(CPlotWidget w, XEvent *event);
static void AllComp(CPlotWidget w, XEvent *event);
static void Components(CPlotWidget w, XEvent *event, int key);
static void FindMeasureBoxes(CPlotWidget w, int cursor_x, DataEntry *entry);
static Boolean GetBox(CPlotWidget w, GDataPoint *dp, double *left_side,
		double *right_side, double *bottom_side, double *top_side);
static void DrawMeasureBoxes(CPlotWidget w, DataEntry *entry);
static void DrawMeasureBox(CPlotWidget w, DataEntry *entry);
static void DoMeasureCallback(CPlotWidget w, int num, DataEntry **entry);
static void DoAmpConvertCallback(CPlotWidget w, int num, DataEntry **entry);
static void FillCursorStructs(CPlotWidget w, DataEntry *entry);
static void SetClipRects(CPlotWidget w, DataEntry *entry);
static void Select(CPlotWidget w, DataEntry *entry, Boolean callbacks);
static DataEntry *WhichEntry(CPlotWidget w, int cursor_x, int cursor_y);
static Boolean WhichMeasureBoxSide(CPlotWidget w, int cursor_x, int cursor_y);
static Boolean InRectangle(DataEntry *entry, int ix1, int iy1, int ix2,int iy2);
static Boolean in_rectangle(SegArray *s, int ix1, int iy1, int ix2, int iy2);
static void ResizeMeasureBox(CPlotWidget w, XEvent *event);
static void DeleteEntry(CPlotWidget w, DataEntry *entry);
static void OriginToData(CPlotWidget w, CssOriginClass *origin);
static void WftagToData(CPlotWidget w, CssWftagClass *wftag);
static void DrawArrivalLabel(CPlotWidget w, ArrivalEntry *a_entry);
static void FindArrivalsOnData(CPlotWidget w, DataEntry *entry);
static void ArrivalToData(CPlotWidget w, ArrivalEntry *a_entry);
static Boolean _CPlotSelectArrival(CPlotWidget w, CssArrivalClass *a,
			Boolean selected, Boolean doCallback);
static Boolean select_arrival(CPlotWidget w, XEvent *event);
static void DoSelectArrival(CPlotWidget w, ArrivalEntry *a_entry,
			Boolean do_callback);
static void MoveArrival(CPlotWidget w, XEvent *event);
static int WhichArrival(CPlotWidget w, int cursor_x, int cursor_y, int *ii,
			int *jj);
static void FindArrivals(CPlotWidget w, DataEntry *entry);
static void FindOneArrival(CPlotWidget w, DataEntry *entry, int j);
static void DrawArrivalsOnEntry(CPlotWidget w, DataEntry *entry);
static void DrawArrival(CPlotWidget w, ArrivalEntry *a_entry);
static void DrawOneArrival(CPlotWidget w, DataEntry *entry, int j,
			Boolean selected, Boolean moving);
static void FindPredArrivals(CPlotWidget w, DataEntry *entry);
static void FindOnePredArr(CPlotWidget w, DataEntry *entry,
			CPlotPredArr *pred_arr);
static void DrawOnePredArr(CPlotWidget w, DataEntry *entry,
			CPlotPredArr *pred_arr);
static void *MallocIt(CPlotWidget w, int nbytes);
static void DrawTag(CPlotWidget w, DataEntry *entry);
static void DrawScale(CPlotWidget w, DataEntry *entry, double yscale);
static int ComponentIndex(DataEntry *entry);
static void DrawArrivalRec(CPlotWidget w, GC gc);
static void DrawPointRec(CPlotWidget w, GC gc);
static void DrawLine(CPlotWidget w, GC gc, int x1, int y1, int x2, int y2);
static void position(CPlotWidget w);
static Boolean overlaps(double a1, double a2, double b1, double b2);
static void prepEntry(CPlotWidget w, DataEntry *entry);
static Boolean plot_ts(DataEntry *entry, double xscale, double yscale);
static void DrawAllArrivals(CPlotWidget w);
static void drawSelectLines(CPlotWidget w, DataEntry *entry);
static Boolean selectLimits(CPlotWidget w, DataEntry *entry, int *begSel,
			int *endSel);
static int maxTagWidth(CPlotWidget w);
static void AdjustHorizontalScrollbars(CPlotWidget w);
static void AdjustHorizontalLimits(CPlotWidget w);
static void moveOverEntry(CPlotWidget w, XEvent *event);
static void moveOverCurve(CPlotWidget w, XEvent *event);
extern "C" {
static int sort_by_y0(const void *A, const void *B);
static int sort_cd_by_y0(const void *A, const void *B);
static int sort_arrivals_by_time(const void *A, const void *B);
};
static void placeEntries(CPlotWidget w);

static void _CPlotMouseDownSelect(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void _CPlotMouseCtrlSelect(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void _CPlotMouseShiftSelect(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void _CPlotMouseDownScale(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseDownSelect(CPlotWidget w, DataEntry *selected_entry, int x);
static void mouseDownMove(CPlotWidget w, DataEntry *entry, int cursor_x,
			int cursor_y, Boolean manual);
static void _CPlotMouseDrag(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseDragSelect(CPlotWidget w, XEvent *event);
static void _CPlotMouseUp(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseUpSelect(CPlotWidget w, XEvent *event);
static DataEntry * getZoomEntry(CPlotWidget w, GTimeSeries *ts);
static void CPlot_AxesZoom(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void mouseCtrlSelect(CPlotWidget w, DataEntry *entry);
static void mouseShiftSelect(CPlotWidget w, DataEntry *entry, int x);
static void mouseDragMove(CPlotWidget w, XEvent *event, DataEntry *manual_entry,
			double manual_scaled_x0, double manual_scaled_y0);
static Boolean outside(CPlotWidget w, int cursor_x, int cursor_y);
static void changeScale(CPlotWidget w, double fac, Boolean all);
static void adjustLimits(CPlotWidget w);
static void printLeftLimit(AxesWidget widget);
static int FindRSeg(RSeg *r, int n, double findx);
static double getAmpNnms(DataEntry *entry, double amp);
static double getAmpTs(DataEntry *entry, double ampNnms);
static void arrivalInfo(XtPointer client_data, XtIntervalId id);
static void arrivalAddMenu(XtPointer client_data, XtIntervalId id);
static void waveformInfo(XtPointer client_data, XtIntervalId id);
static void destroyInfoPopup(CPlotWidget w);
static char *arrivalPhase(CssArrivalClass *a);
static void copyCD(Waveform *c1, Waveform *c2);
static void drawTimeSeries(GTimeSeries *ts, GDataPoint *beg, GDataPoint *end,
		double mean, double xscale, double yscale, RSegArray *r);
static int numEntriesOn(CPlotPart *cp);
static void DisplayOneArrival(CPlotWidget w, ArrivalEntry *a_entry);
static void CPlotKeyCommand(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params);
static void CPlotFreeEntry(CPlotPart *cp, DataEntry *entry);
static void CPlotPopupMenu(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params);
static void addArrivalCallback(CPlotWidget widget, bool shift);

static Boolean processKeyCommand(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params);
static bool putOrigin(CPlotWidget w, CssOriginClass *origin);

typedef struct {
	const char *proc_name;
	XtActionProc proc;
} CPlotKeyProc;

static CPlotKeyProc key_procs[] =
{
  {"AddCrosshair",	(XtActionProc)_AxesAddCrosshair},
  {"AddDoubleLine",	(XtActionProc)_AxesAddDoubleLine},
  {"AddLine",		(XtActionProc)_AxesAddLine},
  {"DeleteCursor",  	(XtActionProc)_AxesDeleteCursor},
  {"ZoomInHorizontal",	(XtActionProc)_AxesZoomHorizontal},
  {"ZoomInVertical",	(XtActionProc)_AxesZoomVertical},
  {"ZoomIn",		(XtActionProc)_AxesZoomPercentage},
  {"ZoomOutHorizontal",	(XtActionProc)_AxesUnzoomHorizontal},
  {"ZoomOutVertical",	(XtActionProc)_AxesUnzoomVertical},
  {"ZoomOut",		(XtActionProc)_AxesUnzoomPercentage},
  {"Page",   		(XtActionProc)AxesPage},
  {"MoveEntry",		(XtActionProc)MoveEntry}
};

static CPlotKeyAction default_key_actions[] =
{
  {"c",		"AddCrosshair"},
  {"l",		"AddDoubleLine"},
  {"L",		"AddLine"},
  {"d",		"DeleteCursor(one)"},
  {"D",		"DeleteCursor(all)"},
  {"h",		"ZoomInHorizontal(.20)"},
  {"v",		"ZoomInVertical(.20)"},
  {"z",		"ZoomIn(.20 .20)"},
  {"H",		"ZoomOutHorizontal(.20)"},
  {"V",		"ZoomOutVertical(.20)"},
  {"Z",		"ZoomOut(.20 .20)"},
  {"ctrl+f",	"Page(vertical DOWN)"},
  {"page_down",	"Page(vertical DOWN)"},
  {"ctrl+b",	"Page(vertical UP)"},
  {"page_up",	"Page(vertical UP)"},
  {"ctrl+l",	"Page(horizontal LEFT)"},
  {"ctrl+r",	"Page(horizontal RIGHT)"},
  {"ctrl+d",	"Page(vertical down)"},
  {"ctrl+u",	"Page(vertical up)"},
  {"arrow_down", "MoveEntry(down 1)"},
  {"arrow_up",	 "MoveEntry(up 1)"},
  {"arrow_right","MoveEntry(right 1)"},
  {"arrow_left", "MoveEntry(left 1)"}
};

static int num_key_actions = 0;
static CPlotKeyAction *key_actions = NULL;

void
CPlotSetKeyActions(int num, CPlotKeyAction *keys)
{
    Free(key_actions);
    num_key_actions = 0;
    if(num > 0) {
	key_actions = new CPlotKeyAction[num];
	memcpy(key_actions, keys, num*sizeof(CPlotKeyAction));
	num_key_actions = num;
    }
}

int
CPlotGetKeyActions(CPlotKeyAction **keys)
{
    CPlotKeyAction *k = NULL;
    *keys = NULL;
    if(num_key_actions <= 0) return 0;

    k = (CPlotKeyAction *)malloc(num_key_actions*sizeof(CPlotKeyAction));
    memcpy(k, key_actions, num_key_actions*sizeof(CPlotKeyAction));
    *keys = k;
    return num_key_actions;
}

int
CPlotGetDefaultKeyActions(CPlotKeyAction **keys)
{
    int num = sizeof(default_key_actions)/sizeof(CPlotKeyAction);
    CPlotKeyAction *k = NULL;
    *keys = NULL;
    if(num <= 0) return 0;

    k = (CPlotKeyAction *)malloc(num*sizeof(CPlotKeyAction));
    memcpy(k, default_key_actions, num*sizeof(CPlotKeyAction));
    *keys = k;
    return num;
}

/*
  ~Shift ~Ctrl<Key>c:	AddCrosshair() \n\
  ~Shift ~Ctrl<Key>l:	AddDoubleLine() \n\
   Shift ~Ctrl<Key>l:	AddLine() \n\
  ~Shift ~Ctrl<Key>d:	DeleteCursor(one) \n\
   Shift ~Ctrl<Key>d:	DeleteCursor(all) \n\
  ~Shift ~Ctrl<Key>h:	ZoomHorizontal() \n\
  ~Shift ~Ctrl<Key>v:	ZoomVertical() \n\
  ~Shift ~Ctrl<Key>z:	ZoomPercentage() \n\
   Shift ~Ctrl<Key>h:	UnzoomHorizontal() \n\
   Shift ~Ctrl<Key>v:	UnzoomVertical() \n\
   Shift ~Ctrl<Key>z:	UnzoomPercentage() \n\
  ~Shift ~Ctrl<Key>f:	Page(vertical DOWN) \n\
  ~Shift ~Ctrl<Key>b:	Page(vertical UP) \n\
  ~Shift ~Ctrl<Key>t:	Page(horizontal DOWN) \n\
  ~Shift ~Ctrl<Key>r:	Page(horizontal UP) \n\
   Shift ~Ctrl<Key>f:	Page(vertical down) \n\
   Shift ~Ctrl<Key>b:	Page(vertical up) \n\
   Shift ~Ctrl<Key>t:	Page(horizontal down) \n\
   Shift ~Ctrl<Key>r:	Page(horizontal up) \n\
  ~Shift ~Ctrl<Key>m:	MoveEntry(down 1) \n\
  ~Shift ~Ctrl<Key>i:	MoveEntry(up 1) \n\
  ~Shift ~Ctrl<Key>k:	MoveEntry(right 1) \n\
  ~Shift ~Ctrl<Key>j:	MoveEntry(left 1) \n\
   Shift ~Ctrl<Key>m:	MoveEntry(down 10) \n\
   Shift ~Ctrl<Key>i:	MoveEntry(up 10) \n\
   Shift ~Ctrl<Key>k:	MoveEntry(right 10) \n\
   Shift ~Ctrl<Key>j:	MoveEntry(left 10) \n\
  Button1<Key>1:	ZCompOnly() \n\
  Button1<Key>2:	ECompOnly() \n\
  Button1<Key>3:	NCompOnly() \n\
  Button1<Key>4:	AllComp() \n\
*/

static char	defTrans[] =
"~Shift ~Ctrl<Btn1Down>:MouseDownSelect() \n\
 ~Shift Ctrl<Btn1Down>:	MouseCtrlSelect() \n\
 ~Ctrl Shift<Btn1Down>:	MouseShiftSelect() \n\
 ~Shift ~Ctrl<Btn2Down>:Zoom() \n\
 ~Shift Ctrl<Btn2Down>:	Magnify() \n\
 ~Ctrl Shift<Btn2Down>:	ZoomBack() \n\
 ~Shift ~Ctrl<Btn3Down>:Zoom(horizontal) \n\
 ~Shift Ctrl<Btn3Down>:	MouseDownScale() \n\
 ~Ctrl Shift<Btn3Down>:	MouseDownMeasure() \n\
  Ctrl Shift<Btn1Down>:	PopupMenu(1) \n\
  Ctrl Shift<Btn2Down>:	PopupMenu(2) \n\
  Ctrl Shift<Btn3Down>:	PopupMenu(3) \n\
  <Btn1Motion>:		MouseDrag() \n\
  <Btn2Motion>:		MouseDrag() \n\
  <Btn3Motion>:		MouseDrag(horizontal) \n\
  <Btn1Up>:		MouseUp() \n\
  <Btn2Up>:		MouseUp() \n\
  <Btn3Up>:		MouseUp(horizontal) \n\
  <Motion>:		Motion() \n\
  ~Ctrl<KeyPress>:	KeyCommand()\n\
  Ctrl<Key>a:		KeyCommand(a)\n\
  Ctrl<Key>b:		KeyCommand(b)\n\
  Ctrl<Key>c:		KeyCommand(c)\n\
  Ctrl<Key>d:		KeyCommand(d)\n\
  Ctrl<Key>e:		KeyCommand(e)\n\
  Ctrl<Key>f:		KeyCommand(f)\n\
  Ctrl<Key>g:		KeyCommand(g)\n\
  Ctrl<Key>h:		KeyCommand(h)\n\
  Ctrl<Key>i:		KeyCommand(i)\n\
  Ctrl<Key>j:		KeyCommand(j)\n\
  Ctrl<Key>k:		KeyCommand(k)\n\
  Ctrl<Key>l:		KeyCommand(l)\n\
  Ctrl<Key>m:		KeyCommand(m)\n\
  Ctrl<Key>n:		KeyCommand(n)\n\
  Ctrl<Key>o:		KeyCommand(o)\n\
  Ctrl<Key>p:		KeyCommand(p)\n\
  Ctrl<Key>q:		KeyCommand(q)\n\
  Ctrl<Key>r:		KeyCommand(r)\n\
  Ctrl<Key>s:		KeyCommand(s)\n\
  Ctrl<Key>t:		KeyCommand(t)\n\
  Ctrl<Key>u:		KeyCommand(u)\n\
  Ctrl<Key>v:		KeyCommand(v)\n\
  Ctrl<Key>w:		KeyCommand(w)\n\
  Ctrl<Key>x:		KeyCommand(x)\n\
  Ctrl<Key>y:		KeyCommand(y)\n\
  Ctrl<Key>z:		KeyCommand(z)\n\
  <LeaveWindow>:	LeaveWindow() \n\
  <EnterWindow>:	EnterWindow() \n\
  <BtnDown>:		BtnDown()";

static XtActionsRec	actionsList[] =
{
	{(char *)"MouseDownSelect",	(XtActionProc)_CPlotMouseDownSelect},
	{(char *)"MouseCtrlSelect",	(XtActionProc)_CPlotMouseCtrlSelect},
	{(char *)"MouseShiftSelect",	(XtActionProc)_CPlotMouseShiftSelect},
	{(char *)"Zoom",		(XtActionProc)CPlot_AxesZoom},
	{(char *)"Magnify",		(XtActionProc)_AxesMagnify},
	{(char *)"ZoomBack",		(XtActionProc)_AxesZoomBack},
	{(char *)"MouseDownScale",	(XtActionProc)_CPlotMouseDownScale},
	{(char *)"MouseDownMeasure",	(XtActionProc)_CPlotMouseDownMeasure},
	{(char *)"MouseDrag",		(XtActionProc)_CPlotMouseDrag},
	{(char *)"MouseUp",		(XtActionProc)_CPlotMouseUp},
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
	{(char *)"KeyCommand",		(XtActionProc)CPlotKeyCommand},
	{(char *)"PopupMenu",		(XtActionProc)CPlotPopupMenu},

	{(char *)"MoveEntry",		(XtActionProc)MoveEntry},
	{(char *)"ZCompOnly",		(XtActionProc)ZCompOnly},
	{(char *)"ECompOnly",		(XtActionProc)ECompOnly},
	{(char *)"NCompOnly",		(XtActionProc)NCompOnly},
	{(char *)"AllComp",		(XtActionProc)AllComp},
	{(char *)"Motion",		(XtActionProc)Motion},
	{(char *)"LeaveWindow",		(XtActionProc)LeaveWindow},
	{(char *)"EnterWindow",		(XtActionProc)EnterWindow},
	{(char *)"BtnDown",		(XtActionProc)_AxesBtnDown},
};


CPlotClassRec	cPlotClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&axesClassRec),	/* superclass */
	(char *)"CPlot",		/* class_name */
	sizeof(CPlotRec),		/* widget_size */
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
		0,		/* empty */
	},
	{	/* CPlotClass fields */
		1,			/* nextid */
	},
};

WidgetClass cPlotWidgetClass = (WidgetClass)&cPlotClassRec;

#define alpha_size 7.

static const char *clist = "enz";
static const char *Clist = "ENZ";
static const char *rlist = "rtz";
static const char *Rlist = "RTZ";
static const char *vlist = "rtv";
static const char *nlist = "123";

static void
CvtStringToCPlotInt(XrmValuePtr *args, Cardinal *num_args, XrmValuePtr fromVal,
			XrmValuePtr toVal)
{
	char		*s = fromVal->addr;	/* string to convert */
	static int	val;			/* returned value */

	if(s == NULL)				   val = 0;
	else if(!strcasecmp(s, "CPLOT_X_MOVEMENT")) val = CPLOT_X_MOVEMENT;
	else if(!strcasecmp(s, "CPLOT_Y_MOVEMENT")) val = CPLOT_Y_MOVEMENT;
	else if(!strcasecmp(s, "CPLOT_XY_MOVEMENT")) val = CPLOT_XY_MOVEMENT;
	else if(!strcasecmp(s, "CPLOT_NO_MOVEMENT")) val = CPLOT_NO_MOVEMENT;
	else if(!strcasecmp(s, "CPLOT_ARRIVALS_OFF")) val = CPLOT_ARRIVALS_OFF;
	else if(!strcasecmp(s, "CPLOT_ARRIVALS_ONE_CHAN"))
	    val = CPLOT_ARRIVALS_ONE_CHAN;
	else if(!strcasecmp(s, "CPLOT_ARRIVALS_ALL_CHAN"))
	    val = CPLOT_ARRIVALS_ALL_CHAN;
	else if(!strcasecmp(s, "CPLOT_ALL_COMPONENTS"))
	    val = CPLOT_ALL_COMPONENTS;
	else if(!strcasecmp(s, "CPLOT_Z_COMPONENT"))	val = CPLOT_Z_COMPONENT;
	else if(!strcasecmp(s, "CPLOT_N_COMPONENT"))	val = CPLOT_N_COMPONENT;
	else if(!strcasecmp(s, "CPLOT_E_COMPONENT"))	val = CPLOT_E_COMPONENT;
	else {
	    printf("CvtStringToCPlotInt: unrecognized resource value: %s\n", s);
	    val = 0;
	}
	toVal->size = sizeof(val);
	toVal->addr = (char *)&val;
}

static void
ClassInitialize(void)
{
	XtAddConverter(XtRString, XtRCPlotInt,(XtConverter)CvtStringToCPlotInt,
		(XtConvertArgList)NULL, (Cardinal) 0);

	if( !key_actions ) {
	    int num = sizeof(default_key_actions)/sizeof(CPlotKeyAction);
	    CPlotSetKeyActions(num , default_key_actions);
	}
}

static void
Initialize(Widget w_req, Widget	w_new)
{
	CPlotWidget nou = (CPlotWidget)w_new;
	CPlotPart *cp = &nou->c_plot;
	/*XtTranslations translations; */

	cp->component_time_limit = 600.;
	cp->dataSpacing = 1.;
	cp->join_time_limit = 7200.;
	cp->overlap_limit = 30.;
	cp->info_delay = 1.0;

	/* no data displayed yet.
	 */
	cp->num_entries = 0;
	cp->entry = (DataEntry **)NULL;
	cp->size_entry = 0;
	cp->num_curves = 0;
	cp->curve = (DataCurve **)NULL;
	cp->size_curve = 0;
	cp->owner_object = new Gobject();

	cp->v = new Vectors();

	cp->working_orid = 0;

	cp->ref = (DataEntry *)NULL;
	cp->label_entry = (DataEntry *)NULL;
	cp->select_entry = (DataEntry *)NULL;
	cp->move_entry = (DataEntry *)NULL;
	cp->info_entry = (DataEntry *)NULL;

	cp->mov.moved = False;
	cp->mov.scaled = False;
	cp->mov.cutx1 = 0;
	cp->mov.cutx2 = 0;
	cp->mov.beg = NULL;
	cp->mov.end = NULL;
	cp->mov.xbeg = 0.;
	cp->mov.drag_yscale = 0.;
	cp->mov.y_diff = 0.;

	/*
	 * tag_font, arrival_font and amp_font are resources.
	 */
	cp->drag_scale_initial_y = 0;
	/*
	 * data_movement, x_axis, and y_axis are resources.
	 */
	nou->axes.last_cursor_x = 0;
	nou->axes.last_cursor_y = 0;
	cp->arrival_i = -1;
	cp->arrival_j = -1;
	cp->info_arrival_i = -1;
	cp->info_arrival_j = -1;
	cp->arrival_moving = False;
	cp->point_x = -1;
	cp->point_y = -1;
	cp->measure_entry = NULL;
	cp->resize_measure_box = -1;
	cp->find_arrivals = -1;
	cp->display_components = CPLOT_ALL_COMPONENTS;
	/*
	 * display_tags, real_time and time_scale are resources.
	 */
	/*
	 * scroll_data is a resource.
	 */
	cp->adjust_after_move = True;
	cp->scaling = False;
	cp->selecting = False;
	cp->deselecting = False;
	cp->adding_arrival = False;
	nou->axes.zooming = False;
        nou->axes.zoom_min = 1;
	nou->axes.moving_cursor = False;
	/*
	 * display_arrivals are resources.
	 */
	cp->draw_label_call = 0;
	cp->key_modifier = False;
	cp->need_sort = False;
	cp->retime_arrival = False;

/*
	translations = XtParseTranslationTable(defTrans);
	XtOverrideTranslations(nou, translations);
*/

	nou->axes.redraw_data_func = CPlotRedraw; 
	nou->axes.resize_func = CPlotResize;
	nou->axes.hard_copy_func = CPlotHardCopy;
	nou->axes.select_data_func = _CPlotSelectData;

	cp->first_position = True;
	cp->numPositions = 0;
	cp->tmin = 0.;
	cp->tmax = 300.;
	cp->uniform_scale = 100.;
	cp->max_tag_width = 0;
	if(cp->dataSpacing == 0.) cp->dataSpacing = 1.;
	cp->scale_independently = True;
	cp->tmp_selected = NULL;
	cp->data_shift = False;
	cp->data_min = 0.;
	cp->data_max = 0.;

	cp->info_popup = NULL;
	cp->curve_left_margin = .1;
	cp->curve_right_margin = .1;
	cp->curve_bottom_margin = .1;
	cp->curve_top_margin = .1;
	memset(cp->last_key_str, 0, sizeof(cp->last_key_str));
	cp->add_arrival_x = -1;
	cp->add_arrival_y = -1;
	cp->motion_cursor_x = -1;
	cp->motion_cursor_y = -1;
	cp->arrival_keys = new vector<ArrivalKey>;
}

void CPlotClass::setCurveMargins(double left_margin, double right_margin,
		double bottom_margin, double top_margin)
{
    CPlotPart *cp = &cw->c_plot;
    cp->curve_left_margin = left_margin;
    cp->curve_right_margin = right_margin;
    cp->curve_bottom_margin = bottom_margin;
    cp->curve_top_margin = top_margin;
}

void CPlotClass::setAppContext(XtAppContext app)
{
    cw->c_plot.app_context = app;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
	CPlotWidget w = (CPlotWidget)widget;
	CPlotPart *cp = &w->c_plot;
	int i;
	(*((cPlotWidgetClass->core_class.superclass)->core_class.realize))
		((Widget)w,  valueMask, attrs);

       	cp->labelGC   	  = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
       	cp->invertGC   	  = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
       	cp->arrivalGC  	  = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	
        XSetForeground(XtDisplay(w), cp->labelGC, w->axes.fg);
        XSetBackground(XtDisplay(w), cp->labelGC,
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), cp->arrivalGC, w->axes.fg);
        XSetBackground(XtDisplay(w), cp->arrivalGC,
		w->core.background_pixel);

	XSetFunction(XtDisplay(w), cp->invertGC, GXinvert);
	XSetPlaneMask(XtDisplay(w), cp->invertGC, w->axes.fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), cp->invertGC, w->axes.fg);
	XSetBackground(XtDisplay(w), cp->invertGC,
		w->core.background_pixel);
	XSetFont(XtDisplay(w), cp->invertGC, w->axes.font->fid);

	cp->dataGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	XSetFunction(XtDisplay(w), cp->dataGC, GXinvert);
	XSetPlaneMask(XtDisplay(w), cp->dataGC,
			w->axes.fg ^ w->core.background_pixel);
       	XSetForeground(XtDisplay(w), cp->dataGC, w->axes.fg);
        XSetBackground(XtDisplay(w), cp->dataGC,w->core.background_pixel);
	XSetFont(XtDisplay(w), cp->dataGC, w->axes.font->fid);
	XSetFont(XtDisplay(w), cp->labelGC, w->axes.font->fid);
	XSetFont(XtDisplay(w), cp->arrivalGC,cp->arrival_font->fid);

	if(cp->dataHeight <= 0)
	{
		cp->dataHeight = (w->axes.x_axis != LEFT) ?
			(int)(.2*(int)w->core.height) : (int)(.2*w->core.width);
	}
	if(cp->dataHeight <= 0) cp->dataHeight = 50;

	/* force dataHeight + dataSeparation >= 10 pixels
	 */
	if(cp->dataHeight + cp->dataSeparation < 10) {
	    cp->dataSeparation = 10 - cp->dataHeight;
	}
	position(w);

	for(i = 0; i < cp->num_entries; i++) {
	    DrawTag(w, cp->entry[i]);
	}
	for(i = 0; i < cp->v->arrival_entries.size(); i++) {
	    DrawArrivalLabel(w, cp->v->arrival_entries[i]);
	}

	w->axes.skip_limits_callback = True;
	AxesUnzoomAll((AxesWidget)w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);
	w->axes.skip_limits_callback = False;
}

static Boolean
SetValues(CPlotWidget cur, CPlotWidget req, CPlotWidget nou)
{
	CPlotPart *cur_cp = &cur->c_plot;
	CPlotPart *new_cp = &nou->c_plot;
	CPlotPart *req_cp = &req->c_plot;
	Boolean redisplay = False, redraw = False, redraw_z = False,
		zoom_zero = False;
	int i;
	CPlotWidget z;
	CPlotPart *zp;
	AxesPart *ax;

	ax = &nou->axes;
	z = (CPlotWidget)nou->axes.mag_to;
	zp = (z != NULL) ? &z->c_plot : NULL;

	if(cur->axes.font != req->axes.font) {
	    XSetFont(XtDisplay(nou), new_cp->invertGC, nou->axes.font->fid);
	    XSetFont(XtDisplay(nou), new_cp->dataGC, nou->axes.font->fid);
	    XSetFont(XtDisplay(nou), new_cp->labelGC, nou->axes.font->fid);
	    redraw = True;
	}
	if(cur_cp->tag_font != req_cp->tag_font) {
	    for(i = 0; i < new_cp->num_entries; i++) {
		DrawTag(nou, new_cp->entry[i]);
	    }
	    if(z != NULL) {
		for(i = 0; i < zp->num_entries; i++) {
		    DrawTag(z, zp->entry[i]);
		}
	    }
	    redraw = True;
	}
	if(cur_cp->arrival_font != req_cp->arrival_font) {
	    for(i = 0; i < req_cp->v->arrival_entries.size(); i++)
	    {
		CssArrivalClass *a = req_cp->v->arrival_entries[i]->a;
		XSetFont(XtDisplay(nou), new_cp->arrivalGC,
			new_cp->arrival_font->fid);
		CPlotRenameArrival(nou, a, arrivalPhase(a));
	    }
	}
	if(cur_cp->associated_arrival_font != req_cp->associated_arrival_font) {
	    for(i = 0; i < req_cp->v->arrival_entries.size(); i++)
	    {
		CssArrivalClass *a = req_cp->v->arrival_entries[i]->a;
		XSetFont(XtDisplay(nou), new_cp->arrivalGC,
			new_cp->associated_arrival_font->fid);
		CPlotRenameArrival(nou, a, arrivalPhase(a));
	    }
	}
	if(cur_cp->amp_font != req_cp->amp_font) {
	    redraw = True;
	}

	if(cur_cp->data_movement != req_cp->data_movement) {
	    if(req_cp->curves_only) {
		new_cp->data_movement = CPLOT_NO_MOVEMENT;
	    }
	    else {
		if(req_cp->data_movement != CPLOT_NO_MOVEMENT) {
		    _AxesSetCursor((AxesWidget)nou, "move");
		}
		else {
		    _AxesSetCursor((AxesWidget)nou, "default");
		}
		if(z != NULL) {
		    zp->data_movement = new_cp->data_movement;
		    if(zp->data_movement != CPLOT_NO_MOVEMENT) {
			_AxesSetCursor((AxesWidget)z, "move");
		    }
		    else {
			_AxesSetCursor((AxesWidget)z, "default");
		    }
		}
	    }
	}
	if(z != NULL && cur_cp->allow_partial_select !=
		req_cp->allow_partial_select) {
	    zp->allow_partial_select = new_cp->allow_partial_select;
	}
	if(z != NULL && cur_cp->three_half_cycles != req_cp->three_half_cycles){
	    zp->three_half_cycles = new_cp->three_half_cycles;
	}
	if(z != NULL && cur_cp->single_arr_select != req_cp->single_arr_select){
	    zp->single_arr_select = new_cp->single_arr_select;
	}
	if(cur_cp->measure_box != req_cp->measure_box)
	{
	    CPlotRemoveAllMeasureBoxes(nou);
	    if(z != NULL) zp->measure_box = new_cp->measure_box;
	}
        if(cur_cp->display_tags != req_cp->display_tags)
	{
	    redisplay = True;
	    if(z != NULL)
	    {
		zp->display_tags = req_cp->display_tags;
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
	if(cur_cp->display_arrivals != req_cp->display_arrivals)
	{
	    if(cur_cp->display_arrivals != CPLOT_ARRIVALS_OFF)
	    {
		new_cp->display_arrivals = cur_cp->display_arrivals;
		DrawAllArrivals(nou);
		if(z != NULL) {
		    zp->display_arrivals = cur_cp->display_arrivals;
		    DrawAllArrivals(z);
		}
	    }
	    new_cp->display_arrivals = req_cp->display_arrivals;
	    if(z != NULL) zp->display_arrivals = req_cp->display_arrivals;

	    if(req_cp->display_arrivals != CPLOT_ARRIVALS_OFF)
	    {
		if(req_cp->find_arrivals !=req_cp->display_arrivals)
		{
		    for(i = 0; i < new_cp->num_entries; i++) {
			FindArrivals(nou, new_cp->entry[i]);
		    }
		    if(z != NULL) {
			for(i = 0; i < zp->num_entries; i++) {
			    FindArrivals(z, zp->entry[i]);
			}
		    }
		}
		for(i = 0; i < new_cp->num_entries; i++) {
		    DrawArrivalsOnEntry(nou, new_cp->entry[i]);
		}
		if(z != NULL) {
		    for(i = 0; i < zp->num_entries; i++) {
			DrawArrivalsOnEntry(z, zp->entry[i]);
		    }
		}
	    }
	}
	if(cur_cp->display_predicted_arrivals !=
		req_cp->display_predicted_arrivals)
	{
	    new_cp->display_predicted_arrivals = True;
	    for(i = 0; i < new_cp->num_entries; i++) {
		_CPlotDrawPredArrivals(nou, new_cp->entry[i]);
	    }
	    new_cp->display_predicted_arrivals =
			req_cp->display_predicted_arrivals;
	    if(z != NULL)
	    {
		zp->display_predicted_arrivals = True;
		for(i = 0; i < zp->num_entries; i++) {
		    _CPlotDrawPredArrivals(z, zp->entry[i]);
		}
		zp->display_predicted_arrivals =
				req_cp->display_predicted_arrivals;
	    }
	}
	if(cur_cp->display_assoc_only != req_cp->display_assoc_only &&
	   cur_cp->display_arrivals == req_cp->display_arrivals &&
	   cur_cp->display_arrivals != CPLOT_ARRIVALS_OFF &&
		!redraw && !redisplay)
	{
	    DrawAllArrivals(cur);
	    DrawAllArrivals(nou);
	    if(z != NULL) {
		DrawAllArrivals(z);
		zp->display_assoc_only = req_cp->display_assoc_only;
		DrawAllArrivals(z);
	    }
	}		
	if(cur_cp->display_amplitude_scale != req_cp->display_amplitude_scale)
	{
	    if(!ax->auto_y_scale && ax->zoom == 1 && new_cp->num_entries > 0)
	    {
		zoom_zero = True;
	    }
	    else
	    {
		redraw = True;
		redraw_z = True;
		new_cp->display_amplitude_scale =
				req_cp->display_amplitude_scale;
		if(z != NULL)
		{
		    zp->display_amplitude_scale =
					req_cp->display_amplitude_scale;
		}
	    }
	}

	if(zoom_zero)
	{
	    redisplay = True;
	}
	else if(redraw)
	{
	    _AxesRedraw((AxesWidget)nou);
	    _CPlotRedraw(nou);
	    redisplay = True;
	}
	if(redraw_z && z != NULL)
	{
	    CPlotMagnifyWidget(nou, NULL, True);
	    CPlotMagnifyWidget(nou, z, True);
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	}
	if(redisplay) {
	    ax->setvalues_redisplay = True;
	    if(ax->redisplay_pending) redisplay = False;
	    ax->redisplay_pending = True;
	}
	return(redisplay);
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
 *  component_display_mode is different than the current
 *  component_display_mode. 
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] component_display_mode The component display mode.
 */
void CPlotClass::displayComponents(int component_display_mode)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	CPlotWidget z;

	if(cp->display_components != component_display_mode)
	{
	    cp->display_components = component_display_mode;
	    _CPlotComponentsOnOff(w, True);
	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw(w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _CPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);

	    if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	    {
		CPlotMagnifyWidget(w, NULL, True);
		CPlotMagnifyWidget(w, z, True);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
}

int CPlotClass::getComponentDisplayMode()
{
    return cw->c_plot.display_components;
}

static void
CPlotRedraw(AxesWidget widget, int type, double shift)
{
	CPlotWidget w = (CPlotWidget)widget;
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	if(cp->arrival_i >= 0) {
	    destroyInfoPopup(w);
	    cp->arrival_i = -1;
	    cp->arrival_j = -1;
	    cp->arrival_moving = False;
	}

	cp->point_x = -1;
	ax->use_screen = False;
	switch(type)
	{
	    case DATA_REDISPLAY :
		ax->use_screen = False;
		for(i = 0; i < cp->num_curves; i++) {
		    _CPlotRedisplayCurve(w, cp->curve[i]);
		}
		for(i = 0; i < cp->num_entries; i++) {
		    _CPlotRedisplayEntry(w, cp->entry[i]);
		}
		ax->use_screen = True;
		break;
	    case DATA_REDRAW :
		_CPlotRedraw(w);
		break;
	    case DATA_JUMP_HOR :
		_CPlotHorizontalScroll(w, shift, True);
		printLeftLimit(widget);
		break;
	    case DATA_JUMP_VERT :
		_CPlotVerticalScroll(w, shift, True);
		break;
	    case DATA_SCROLL_HOR :
		_CPlotHorizontalScroll(w, shift, False);
		printLeftLimit(widget);
		break;
	    case DATA_SCROLL_VERT :
		_CPlotVerticalScroll(w, shift, False);
		break;
	    case DATA_ZOOM_ZERO :
		CPlotZoomOut(w);
		printLeftLimit(widget);
		break;
	    case DATA_ZOOM_VERTICAL_ZERO :
		CPlotZoomOut(w);
		printLeftLimit(widget);
		break;
	}
	ax->use_screen = True;
}

static void
printLeftLimit(AxesWidget widget)
{
	AxesPart *ax = &widget->axes;

	if(ax->cursor_info2 != NULL) {
	    int n;
	    char tlab[100];
	    if(ax->x1[ax->zoom] > 86400) { /* 24*60*60 */
		timeEpochToString(ax->x1[ax->zoom], tlab, 100, GSE20);
	    }
	    else {
		timeEpochToString(ax->x1[ax->zoom], tlab, 100, HMS);
	    }
	    n = strlen(tlab);
	    if(n >= 99) return;
	    strcat(tlab, "    ");
	    n = strlen(tlab);
	    timeEpochToString(ax->x2[ax->zoom] - ax->x1[ax->zoom],
				tlab+n, 100-n, HMS);
	    InfoSetText(ax->cursor_info2, tlab);
	}
}

static void
CPlotResize(AxesWidget widget)
{
	CPlotWidget w = (CPlotWidget)widget;
	CPlotPart *cp = &w->c_plot;
	int	i;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	position(w);

	for(i = 0; i < cp->num_entries; i++) {
	    SetClipRects(w, cp->entry[i]);
	}
}

/** 
 *  
 */
void
_CPlotRedisplay(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;

	if(!XtIsRealized((Widget)w)) return;

	ax->use_screen = False;
	for(i = 0; i < cp->num_curves; i++) {
	    _CPlotRedisplayCurve(w, cp->curve[i]);
	}
	for(i = 0; i < cp->num_entries; i++) {
	    _CPlotRedisplayEntry(w, cp->entry[i]);
	}
	if(ax->use_pixmap) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	}
	ax->use_screen = True;
}

/** 
 *  
 */
 void
_CPlotRedisplayCurve(CPlotWidget w, DataCurve *curve)
{
	AxesPart *ax = &w->axes;

	if(!XtIsRealized((Widget)w) || curve == NULL) return;

	if(curve->on && curve->s.nsegs > 0)
	{
	    XSetForeground(XtDisplay(w), ax->axesGC, curve->fg);
	    _AxesDrawSegments((AxesWidget)w, ax->axesGC, curve->s.segs,
			curve->s.nsegs);
#ifndef MAPALF
	    if((lab_len = (int)strlen(curve->lab)) > 0) {
		DrawImageString(w, cp->labelGC, curve->lab_x, curve->lab_y,
				curve->lab, lab_len);
	    }
#endif
	    XSetForeground(XtDisplay(w), ax->axesGC, ax->fg);
	}
}

/** 
 *  
 */
void
_CPlotRedisplayEntry(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;

	if(!XtIsRealized((Widget)w)) return;

	if(entry == NULL || !entry->on || !entry->w->visible ||
		(entry->sel.nsegs <= 0 && entry->unsel.nsegs <= 0)) return;

	SetClipRects(w, entry);

	if(entry->sel.nsegs > 0)
	{
	    if(cp->redraw_selected_data) {
		XSetPlaneMask(XtDisplay(w), cp->dataGC,
				ax->select_fg ^ w->core.background_pixel);
		XSetForeground(XtDisplay(w), cp->dataGC, ax->select_fg);
	    }
	    else {
		XSetPlaneMask(XtDisplay(w), cp->dataGC,
				entry->w->fg ^ w->core.background_pixel);
		XSetForeground(XtDisplay(w), cp->dataGC, entry->w->fg);
	    }
	    _AxesDrawSegments2((AxesWidget)w, cp->dataGC, entry->sel.segs,
				entry->sel.nsegs);
	    drawSelectLines(w, entry);
	}
	if(entry->unsel.nsegs > 0)
	{
	    XSetPlaneMask(XtDisplay(w), cp->dataGC,
				entry->w->fg ^ w->core.background_pixel);
	    XSetForeground(XtDisplay(w), cp->dataGC, entry->w->fg);
	    _AxesDrawSegments2((AxesWidget)w, cp->dataGC, entry->unsel.segs,
				entry->unsel.nsegs);
	}

	DrawArrivalsOnEntry(w, entry);
	_CPlotDrawPredArrivals(w, entry);
	if(entry->n_measure_segs > 0) {
	    DrawMeasureBox(w, entry);
	}
	DisplayTag(w, entry);
	
	if(cp->scaling) {
	    double scale_change = cp->mov.drag_yscale/entry->drag_yscale;
	    DrawScale(w, entry, entry->yscale*scale_change);
	}
	else {
	    DrawScale(w, entry, entry->yscale);
	}
}

static void
Destroy(Widget widget)
{
	CPlotWidget w = (CPlotWidget)widget;
	CPlotPart *cp = &w->c_plot;
	int i;

	destroyInfoPopup(w);

	if(cp->labelGC != NULL) XFreeGC(XtDisplay(w), cp->labelGC);
	if(cp->dataGC != NULL) XFreeGC(XtDisplay(w), cp->dataGC);
	if(cp->invertGC != NULL) XFreeGC(XtDisplay(w), cp->invertGC);
	if(cp->arrivalGC != NULL) XFreeGC(XtDisplay(w), cp->arrivalGC);

//	XFreeFont(XtDisplay(w), cp->arrival_font);
//	XFreeFont(XtDisplay(w), cp->tag_font);
//	XFreeFont(XtDisplay(w), cp->amp_font);

	for(i = 0; i < cp->num_curves; i++) delete cp->curve[i];
	cp->num_curves = 0;
	Free(cp->curve);
	cp->size_curve = 0;

	for(i = 0; i < cp->num_entries; i++) {
	    CPlotFreeEntry(cp, cp->entry[i]);
	}
	cp->num_entries = 0;
	Free(cp->entry);
	cp->size_entry = 0;

	delete cp->v;

	Free(cp->tmp_selected);

	delete cp->owner_object;
	delete cp->arrival_keys;
}

static void
CPlotFreeEntry(CPlotPart *cp, DataEntry *entry)
{
	int i;

	if(entry->ts) entry->ts->removeOwner(cp->owner_object);
	entry->ts = NULL;

	for(i = 0; i < MAX_DECI; i++) {
	    if(entry->p[i].ts) {
		entry->p[i].ts->removeOwner(cp->owner_object);
	    }
	    entry->p[i].ts = NULL;
	}
	Free(entry->sel.segs);
	Free(entry->unsel.segs);
	Free(entry->r.segs);
	Free(entry->arrlab);
	Free(entry->pred_arr);
	Free(entry->tag_points);
	if(entry->beg) entry->beg->deleteObject();
	if(entry->end) entry->end->deleteObject();

	for(i = 0; i < entry->w->num_dp; i++) {
	    if(entry->dp[i]) entry->dp[i]->removeOwner(cp->owner_object);
		entry->dp[i] = NULL;
	}
	Free(entry->dp);
	for(i = 0; i < entry->w->num_dw; i++) {
	    if(entry->dw[i].d1) {
		entry->dw[i].d1->removeOwner(cp->owner_object);
		entry->dw[i].d1 = NULL;
	    }
	    if(entry->dw[i].d2) {
		entry->dw[i].d2->removeOwner(cp->owner_object);
		entry->dw[i].d2 = NULL;
	    }
	}
	Free(entry->dw);
	entry->w->removeOwner(cp->owner_object);
	entry->w = NULL;
	Free(entry);
}

#define mod(i,j) (i-((i)/(j))*(j))

/** 
 *  Add a curve to a CPlotWidget. The curve will be displayed immediately
 *  in the CPlotWidget window. The limits of the window will adjust according
 *  to the parameter adjust_limits. Possible values for adjust_limits are:
 *   - CPLOT_HOLD_LIMITS  Do not change the current limits of the axes.
 *   - CPLOT_ADJUST_X_LIMITS Adjust only the x-axis limits, if necessary.
 *   - CPLOT_ADJUST_Y_LIMITS Adjust only the y-axis limits, if necessary.
 *   - CPLOT_ADJUST_LIMITS Adjust the x-axis and y-axis limits, if necessary.
 *   - CPLOT_ADJUST_Y_GROW Allow the y-axis limits to grow, but do not shrink.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] npts The number of points on the curve.
 *  @param[in] x The x or horizontal coordinates of the curve.
 *  @param[in] y The y or vertical coordinates of the curve.
 *  @param[in] type The type can be <TT>CPLOT_CURVE</TT> for a regular curve,
 *	or <TT>CPLOT_TTCURVE</TT> for a travel time curve.
 *  @param[in] label A label that will be printed next to the curve.
 *  @param[in] mouse_label A label that will be printed in a separate text
 *		window when the mouse cursor is moved near to the curve.
 *  @param[in] on The initial visiblity of the curve. If on is True, the curve
 *		is drawn immediately, otherwise the curve is not drawn until a
 *		later function call changes this parameter.
 *  @param[in] adjust_limits Controls how the axes limits are adjusted to
 *		display this curve.
 *  @param[in] pixel The curve will be drawn using this color.
 *  @returns true for success.
 */
bool CPlotClass::addCurve(int npts, double *x, float *y, int type,
		const string &label, const string &mouse_label, bool on,
		short adjust_limits, Pixel pixel)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    DataCurve *curve;
    int i, num_e, num_c;
    double x_min=0., x_max=0., y_min=0., y_max=0., dx, dy;
    double cx_min=0., cx_max=0., cy_min=0., cy_max=0.;
    bool redraw;
	
    if(npts <= 0 || type == CPLOT_DATA) return false;
    if(x == NULL || y == NULL) {
	_AxesWarn((AxesWidget)cw, "CPlotAddCurve: NULL x,y pointers.");
	return false;
    }

    x_min = ax->x1[ax->zoom];
    x_max = ax->x2[ax->zoom];
    y_min = ax->y1[ax->zoom];
    y_max = ax->y2[ax->zoom];

    cp->num_curves++;
    ALLOC(cp->num_curves*sizeof(DataCurve *), 
		cp->size_curve, cp->curve, DataCurve *, 0);
    curve = new DataCurve();
    cp->curve[cp->num_curves-1] = curve;

    curve->npts = npts;
    curve->type = type;
    curve->fg = (pixel != (Pixel)NULL) ? pixel : ax->fg;
    curve->lab = label;
    curve->mouse_label = mouse_label;

    if( !(curve->y = (float *)MallocIt(cw, npts*sizeof(float))) ||
	!(curve->x = (double *)MallocIt(cw, npts*sizeof(double))) ) {
	cp->num_curves--;
	delete curve;
	return false;
    }
    memcpy(curve->y, y, npts*sizeof(float));
    memcpy(curve->x, x, npts*sizeof(double));
    dminmax(npts, x, &curve->xmin, &curve->xmax, 0.);
    fminmax(npts, y, &curve->ymin, &curve->ymax, 0.);

    curve->on = on;
    for(i = 0; i < cp->num_curves; i++) if(cp->curve[i]->on)
    {
	cx_min = cp->curve[i]->xmin;
	cx_max = cp->curve[i]->xmax;
	cy_min = cp->curve[i]->ymin;
	cy_max = cp->curve[i]->ymax;
	break;
    }
    for(i = 0; i < cp->num_curves; i++) if(cp->curve[i]->on)
    {
	if(cx_min > cp->curve[i]->xmin) cx_min = cp->curve[i]->xmin;
	if(cx_max < cp->curve[i]->xmax) cx_max = cp->curve[i]->xmax;
	if(cy_min > cp->curve[i]->ymin) cy_min = cp->curve[i]->ymin;
	if(cy_max < cp->curve[i]->ymax) cy_max = cp->curve[i]->ymax;
    }
    for(i = num_c = 0; i < cp->num_curves-1; i++) {
	if(cp->curve[i]->on) num_c++;
    }
    num_e = cp->num_entries;

    if(adjust_limits)
    {
	dx = cx_max - cx_min;
	dy = cy_max - cy_min;
	if(num_e == 0 && num_c == 0 && adjust_limits == CPLOT_ADJUST_LIMITS)
	{
		ax->zoom = 0;
	}
	if(adjust_limits == CPLOT_ADJUST_X_LIMITS ||
	   adjust_limits == CPLOT_ADJUST_LIMITS)
	{
		ax->x1[ax->zoom] = cx_min - cp->curve_left_margin*dx;
		ax->x2[ax->zoom] = cx_max + cp->curve_right_margin*dx;
	}
	if(adjust_limits == CPLOT_ADJUST_Y_LIMITS ||
	   adjust_limits == CPLOT_ADJUST_Y_GROW ||
	   adjust_limits == CPLOT_ADJUST_LIMITS)
	{
	    // Get the number of y-axis labels and if > 5, check if
	    // a small adjustment to the limits will reduce the number
	    // of labels.
	    int ndigit;
	    double ymin = cy_min - cp->curve_bottom_margin*dy;
	    double ymax = cy_max + cp->curve_top_margin*dy;
	    AxesParm a = ax->a;
	    nicex(ymin, ymax, a.min_ylab, a.max_ylab, &a.nylab, a.y_lab,
				&ndigit, &a.nydeci);
	    if(a.nylab > 5) {
		int imin = -1, nylab_min = a.max_ylab+1;
		for(i = 0; i < 4; i++) {
		    int nylab;
		    double y1 = cy_min - (.2+i*.1)*dy;
		    double y2 = cy_max + (.2+i*.1)*dy;
		    nicex(y1, y2, a.min_ylab, a.max_ylab, &nylab, a.y_lab,
				&ndigit, &a.nydeci);
		    if(nylab < nylab_min) {
			nylab_min = nylab;
			imin = i;
		    }
		}
		if(nylab_min < a.nylab) {
		    if(a.nylab > 8 || a.nylab - nylab_min > 3) {
			ymin = cy_min - (.2+imin*.1)*dy;
			ymax = cy_max + (.2+imin*.1)*dy;
		    }
		}
	    }
	    if(adjust_limits != CPLOT_ADJUST_Y_GROW) {
		ax->y1[ax->zoom] = ymin;
		ax->y2[ax->zoom] = ymax;
	    }
	    else if(fabs(ymax-ymin) > fabs(ax->y2[ax->zoom] - ax->y1[ax->zoom]))
	    {
		ax->y1[ax->zoom] = ymin;
		ax->y2[ax->zoom] = ymax;
	    }
	}
    }

    if(on)
    {
	redraw = ( x_min != ax->x1[ax->zoom] ||
		   x_max != ax->x2[ax->zoom] ||
		   y_min != ax->y1[ax->zoom] ||
		   y_max != ax->y2[ax->zoom]);

	DisplayNewCurve(cw, curve, redraw);
    }
    return true;
}

void CPlotClass::setCurveVisible(int index, bool visible, bool redisplay)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;

    if(index >= 0 && index < cp->num_curves) {
	cp->curve[index]->on = visible;
	if(redisplay) {
	    _AxesRedraw((AxesWidget)cw);
	    _CPlotRedraw(cw);
	    if(!ax->redisplay_pending) {
		_AxesRedisplayAxes((AxesWidget)cw);
		_CPlotRedisplay(cw);
		_AxesRedisplayXor((AxesWidget)cw);
	    }
	}
    }
}

int CPlotClass::numCurves()
{
    return cw->c_plot.num_curves;
}


/** Get all curves.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[out] curves an allocated array of CPlotCurve structures.
 *  @returns the number of curves.
 */
int CPlotClass::getCurves(vector<CPlotCurve> &curves)
{
    CPlotPart *cp = &cw->c_plot;
    CPlotCurve c;

    curves.clear();
    for(int i = 0; i < cp->num_curves; i++) {
	c.npts = cp->curve[i]->npts;
	c.x = cp->curve[i]->x;
	c.y = cp->curve[i]->y;
	c.type = cp->curve[i]->type;
	c.label = (char *)cp->curve[i]->lab.c_str();
	c.mouse_label = (char *)cp->curve[i]->mouse_label.c_str();
	c.pixel = cp->curve[i]->fg;
	curves.push_back(c);
    }
    return (int)curves.size();
}

/** 
 *  Add a GTimeSeries to a CPlotWidget. The CPlotInputStruct holds the
 *  parameters that control how the waveform will be displayed in the
 *  CPlotWidget window.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] ts A GTimeSeries object.
 *  @param[in] input input structure with waveform information.
 *  	- input->display_t0 The time at which to position the first sample of the TimeSeries.
 *  	- input->tag The tag or label to be displayed next to the TimeSeries.
 *  	- input->sta The station name for the TimeSeries.
 *  	- input->chan The channel name for the TimeSeries.
 *  	- input->net The network name for the TimeSeries.
 *  	- input->lat The station latitude in degrees. Can be set to -999.
 *  	- input->lon The station longitude in degrees. Can be set to -999.
 *  	- input->origin An optional origin to be associated with the TimeSeries. Can be NULL.
 *      - input->pixel The color used to draw the TimeSeries.
 *  	- input->on The initial visibility of the TimeSeries. If on is True,
 *			the TimeSeries is drawn immediately, otherwise it is
 *			not drawn until a later function call changes this
 *			parameter.
 *  @returns A Waveform object for the TimeSeries, or NULL if the
 *		TimeSeries has no samples or one of the
 *		CPlotInputStruct parameters is invalid.
 */
Waveform * CPlotClass::addTimeSeries(GTimeSeries *ts, CPlotInputStruct *input)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i;
    DataEntry *entry;
    CPlotPositionCallbackStruct c;
    CPlotWidget z = (CPlotWidget)ax->mag_to;
	
    if(ts == NULL || ts->length() <= 0 || ts->size() <= 0) return NULL;

    cp->num_entries++;

    ALLOC(cp->num_entries*sizeof(DataEntry *), cp->size_entry, cp->entry,
		DataEntry *, 0);

    if( !(cp->entry[cp->num_entries-1] =
			(DataEntry *)MallocIt(w, sizeof(DataEntry))) )
    {
	cp->num_entries--;
	return NULL;
    }
    entry = cp->entry[cp->num_entries-1];

    if(!InitDataEntry(this, w, entry, ts, input)) {
	DeleteEntry(w, entry);
	return NULL;
    }

    if(input->on) {
	if(cp->display_components == CPLOT_ALL_COMPONENTS) {
	    entry->on = true;
	}
	else if(cp->display_components == CPLOT_HORIZONTAL_COMPONENTS) {
	    entry->on = (entry==entry->comps[0] || entry==entry->comps[1]);
	}
	else {
	    entry->on = (entry == entry->comps[cp->display_components]);
	}
    }
    else {
	entry->on = False;
    }

    prepEntry(w, entry);

    if(input->on) {
	int num_on;
	for(i = num_on = 0; i < cp->num_entries; i++) {
	    if(cp->entry[i]->on) num_on++;
	}
	entry->w->scaled_y0 = num_on*cp->dataSpacing;
	for(i = 0; i < cp->num_entries; i++) {
	    if(!cp->entry[i]->on) cp->entry[i]->w->scaled_y0 += 1.;
	}
	adjustLimits(w);

	if( !cp->scale_independently && cp->num_entries == 1) {
	    cp->uniform_scale = entry->ymax - entry->ymin;
	}
    }
    else {
	entry->w->scaled_y0 = cp->num_entries*cp->dataSpacing;
    }

    FindComponents(w, entry);

    FindArrivalsOnData(w, entry);

    if(input->origin != (CssOriginClass *)NULL)
    {
	if(putOrigin(cw, input->origin)) {
	    entry->origin = input->origin;
	}
	else { // duplicate
	    for(i = 0; i < cp->v->origins.size(); i++)
		if(*input->origin ==  *cp->v->origins[i])
	    {
		entry->origin = cp->v->origins[i];
		break;
	    }
	}
    }
    else {
	entry->origin = dataToOrigin(ts);
    }

    cp->need_sort = True;

    if(_CPlotVisible(w, entry))
    {
	_CPlotRedrawEntry(w, entry);
	if(!ax->redisplay_pending)
	{
	    _CPlotRedisplayEntry(w, entry);
	}
	if(z != NULL)
	{
	    CPlotPart *zp = &z->c_plot;
	    int m;
	    MagEntry(w, entry);
	    Visible(z);
	     m = z->c_plot.num_entries-1;
	    _CPlotRedrawEntry(z, zp->entry[m]);
	    _CPlotRedisplayEntry(z, zp->entry[m]);
	}
	if(ax->use_pixmap && XtIsRealized((Widget)w) )
	{
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	}
    }
    else
    {
	MagEntry(w, entry);
    }
    c.reason = CPLOT_DATA_POSITION;
    c.wvec.push_back(entry->w);
    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);

    entry->w->default_order = numWaveforms() - 1;
    change.waveform = true;
    doDataChangeCallbacks();

    return(entry->w);
}


/** Get all curves.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[out] curves an allocated array of CPlotCurve structures.
 *  @returns the number of curves.
 */
int
CPlotGetCurves(CPlotWidget w, CPlotCurve **curves)
{
	CPlotPart *cp = &w->c_plot;
	int i;
	CPlotCurve *c = NULL;

	if(cp->num_curves <= 0) {
	    *curves = NULL;
	    return 0;
	}

	if( !(c = (CPlotCurve *)MallocIt(w, cp->num_curves*sizeof(CPlotCurve))))
	{
	    *curves = NULL;
	    return -1;
	}
	for(i = 0; i < cp->num_curves; i++) {
	    c[i].npts = cp->curve[i]->npts;
	    c[i].x = cp->curve[i]->x;
	    c[i].y = cp->curve[i]->y;
	    c[i].type = cp->curve[i]->type;
	    c[i].label = strdup(cp->curve[i]->lab.c_str());
	    c[i].mouse_label = strdup(cp->curve[i]->mouse_label.c_str());
	    c[i].pixel = cp->curve[i]->fg;
	}
	*curves = c;
	return cp->num_curves;
}


static void
adjustLimits(CPlotWidget w)
{
    CPlotPart	*cp = &w->c_plot;
    AxesPart	*ax = &w->axes;
    double	x_min, x_max, xmin, xmax, ymin, ymax, tmin, tmax;
    Boolean	need_adjustment, entry_on;
    int		i, n;

    /* Adjust limits and scrollbars, if necessary.
     */
    if(cp->adjust_time_limits)
    {
	tmin = tmax = 0.;
	if(cp->num_entries > 0) {
	    tmin = cp->entry[0]->w->scaled_x0;
	    tmax = tmin + cp->entry[0]->ts->duration();
	}
	for(i = 1; i < cp->num_entries; i++) {
	    double max = cp->entry[i]->w->scaled_x0 +
				cp->entry[i]->ts->duration();
	    if(tmin > cp->entry[i]->w->scaled_x0) {
		tmin = cp->entry[i]->w->scaled_x0;
	    }
	    if(tmax < max) tmax = max;
	}
	if(cp->num_entries == 1)
	{
	    if(tmin > cp->tmax || tmax < cp->tmin || (ax->zoom == ax->zoom_min
		&& (tmin < cp->tmin || tmax > cp->tmax)) )
	    {
		entry_on = cp->entry[0]->on;
		cp->entry[0]->on = False; // prevent redisplay of the new entry
		CPlotSetTimeLimits(w, tmin, tmax);
		cp->entry[0]->on = entry_on;
	    }
	}
	if(tmin < cp->tmin) cp->tmin = tmin;
	if(tmax > cp->tmax) cp->tmax = tmax;

	_CPlotGetXLimits(w, &x_min, &x_max);
    }

    need_adjustment = False;

    xmin = ax->x1[0];
    xmax = ax->x2[0];
    ymin = ax->y1[0];
    ymax = ax->y2[0];

    if(cp->adjust_time_limits)
    {
	if(xmin > x_min || xmax < x_max)
	{
            need_adjustment = True;
	    xmin = x_min;
	    xmax = x_max;
        }
	else if(ax->zoom > ax->zoom_min &&
		(ax->x1[ax->zoom] < xmin || ax->x1[ax->zoom] > xmax ||
		 ax->x2[ax->zoom] < xmin || ax->x2[ax->zoom] > xmax) )
	{
	    entry_on = cp->entry[0]->on;
	    cp->entry[0]->on = False; // prevent redisplay of the new entry
	    AdjustHorizontalLimits(w);
	    cp->entry[0]->on = entry_on;
            need_adjustment = True;
	    xmin = x_min;
	    xmax = x_max;
	}
    }
    n = numEntriesOn(cp);
    if(n > cp->numPositions) {
	need_adjustment = True;
	cp->numPositions = n;
	ymin = 0.01;
	ymax = (double)(cp->numPositions + .99)*cp->dataSpacing;
    }

    if(need_adjustment) {
	CPlotWidget z = (CPlotWidget)ax->mag_to;
	ax->x1[0] = xmin;
	ax->x2[0] = xmax;
	ax->y1[0] = ymin;
	ax->y2[0] = ymax;
	ax->zoom_min = 1;
	/* adjust scrollbars
	 */
	_Axes_AdjustScrollBars((AxesWidget)w);

	if(z != NULL) {
	    z->c_plot.numPositions = cp->numPositions;
	    z->axes.x1[0] = ax->x1[0];
	    z->axes.x2[0] = ax->x2[0];
	    z->axes.y1[0] = ax->y1[0];
	    z->axes.y2[0] = ax->y2[0];
	    z->axes.zoom_min = 1;
	    /* adjust scrollbars
	    */
	    _Axes_AdjustScrollBars((AxesWidget)z);
	}
    }
}

static int
numEntriesOn(CPlotPart *cp)
{
	int i, n;

	n = 0;
	for(i = 0; i < cp->num_entries; i++) {
	    if(cp->entry[i]->on) n++;
	}
	return n;
}

void CPlotClass::getTimeLimits(double *tmin, double *tmax)
{
    CPlotPart *cp = &cw->c_plot;
    *tmin = cp->tmin;
    *tmax = cp->tmax;
}

/** 
 *  Set the limits of the time axis. This function causes the data window to
 *  be immediately redrawn. The zoom history is not affected.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] tmin The minimum time on the horizontal axis.
 *  @param[in] tmax The maximum time on the horizontal axis.
 */
void CPlotSetTimeLimits(CPlotWidget w, double tmin, double tmax)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	CPlotWidget z = (CPlotWidget)ax->mag_to;
	double shift, xmin, xmax;
        int i;

	if(cp->tmin == tmin && cp->tmax == tmax) return;

	shift = tmax - cp->tmax;
	for(i = 0; i < ax->num_cursors; i++)
	{
	    if(ax->cursor[i].type == AXES_VER_DOUBLE_LINE
		&& ax->cursor[i].anchor_on)
	    {
		ax->cursor[i].scaled_x1 += shift;
		ax->cursor[i].scaled_x2 += shift;
	    }
	}

	cp->tmin = tmin;
	cp->tmax = tmax;
	_CPlotGetXLimits(w, &xmin, &xmax);

	ax->x1[0] = xmin;
	ax->x2[0] = xmax;
	ax->x1[1] = xmin;
	ax->x2[1] = xmax;

	ax->use_screen = False;
	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	if(!ax->redisplay_pending)
	{
	    _AxesRedisplayAxes((AxesWidget)w);
	    _CPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if(z != NULL)
	{
	    CPlotSetTimeLimits(z, tmin, tmax);
	}
}

static void
position(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int       i, height, n;
	double    xmin, xmax, ymin, ymax, fac=0.;
	Rectangle r;
	AxesParm a;
	DrawStruct d;

	if(w->core.width <= 0 || w->core.height <= 0) return;

	if(cp->curves_only) {
	    ax->zoom_min = 0;
	    return;
	}

	xmin = cp->tmin;
	xmax = cp->tmax;
	r = _AxesLayout((AxesWidget)w);

	a = ax->a;
	for(i = 0; i < 4; i++) {
	    a.axis_segs[i].n_segs = 0;
	    a.axis_segs[i].size_segs = 0;
	    a.axis_segs[i].segs = NULL;
	}
	for(i = 0; i < MAXLAB; i++) {
	    a.xlab[i] = NULL;
	    a.ylab[i] = NULL;
	}
	SetDrawArea(&d, r.x, r.y, r.x + r.width - 1, r.y - r.height + 1);
	SetScale(&d, 0., 0., 1., 1.);

	a.ymin = (ax->y1[0] < ax->y2[0]) ? ax->y1[0] : ax->y2[0];
	a.ymax = (ax->y2[0] > ax->y1[0]) ? ax->y2[0] : ax->y1[0];
	gdrawAxis(&a, &d, xmin, xmax, ax->x_axis, 9., 0., ax->y_axis,
                ax->tickmarks_inside, ax->display_axes_labels, ax->time_scale,
		_AxesTimeFactor((AxesWidget)w, fabs(xmax-xmin), False));

	for(i = 0; i < 4; i++) Free(a.axis_segs[i].segs);
	for(i = 0; i < MAXLAB; i++) {
	    Free(a.xlab[i]);
	    Free(a.ylab[i]);
	}

	height = abs(unscale_y(&d, 9.) - unscale_y(&d, 0.));
	cp->numPositions = height/(cp->dataHeight+cp->dataSeparation);
	if(cp->numPositions < 1) cp->numPositions = 1;
	n = numEntriesOn(cp);
	if(cp->numPositions < n) {
	    cp->numPositions = n;
	}

	ymin = 0.01;
	ymax = ((double)cp->numPositions + .99)*cp->dataSpacing;
	ax->y1[0] = ymin;
	ax->y2[0] = ymax;
        if(ax->zoom <= 0) {
	    ax->zoom = 1;
	    ax->zoom_max = 1;
	}
        ax->zoom_min = 1;

	if(cp->first_position) {
	    cp->first_position = False;
	    ymin = .01;
	}
	else {
	    ymin = ax->y1[1];
	}
	ymax = ymin + cp->dataSpacing*height/(cp->dataHeight+cp->dataSeparation)			- .02;

	if(ymax > ax->y2[0]) {
	    ymin -= (ymax - ax->y2[0]);
	    ymax = ax->y2[0];
	}

	if(ax->zoom > 1) {
	    fac = (ymax - ymin)/(ax->y2[1] - ax->y1[1]);
	}
	if(!cp->data_shift) {
	    ax->x1[1] = ax->x1[0];
	    ax->x2[1] = ax->x2[0];
	}
	ax->y1[1] = ymin;
	ax->y2[1] = ymax;

	for(i = 2; i <= ax->zoom; i++) {
	    double ydif = ax->y2[i] - ax->y1[i];
	    ax->y2[i] = ax->y1[i] + ydif*fac;
	    if(ax->y2[i] > ax->y2[0]){
		ax->y1[i] -= (ax->y2[i] - ax->y2[0]);
		ax->y2[i] = ax->y2[0];
	    }
	}
}

/** 
 *  Shift all waveforms and all line cursors by a constant time period.
 *  The limits of the axes will not be changed. This function does not
 *  cause the waveforms to be redrawn. This function must be followed by
 *  any other CPlot function that causes a waveform redraw.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] time_shift A positive or negative time shift in seconds.
 */
void CPlotClass::timeShift(double time_shift)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    int i;

    for(i = 0; i < cp->num_entries; i++) {
	cp->entry[i]->w->scaled_x0 += time_shift;
    }
    for(i = 0; i <= ax->zoom; i++) {
	ax->x1[i] += time_shift;
	ax->x2[i] += time_shift;
    }
    ax->x_min += time_shift;
    ax->x_max += time_shift;

    for(i = 0; i < ax->num_cursors; i++) {
	ax->cursor[i].scaled_x += time_shift;
	ax->cursor[i].scaled_x1 += time_shift;
	ax->cursor[i].scaled_x2 += time_shift;
    }
}

/** 
 *  Get the pixel waveform data height for a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @returns The data height in pixels.
 */
int CPlotClass::getDataHeight() {
    return cw->c_plot.dataHeight;
}

/** 
 *  Get the pixel waveform data separation for a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @returns The data separation in pixels.
 */
int CPlotClass::getDataSeparation() {
    return cw->c_plot.dataSeparation;
}

/** 
 *  Set the waveform data height and data separation for a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] dataHeight The data height in pixels.
 *  @param[in] dataSeparation The data separation in pixels.
 */
void CPlotClass::setDataHeight(int dataHeight, int dataSeparation)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	CPlotWidget z;
	int i;

	for(i = 0; i < cp->num_entries; i++) {
	    DataEntry *ze;
	    if(cp->entry[i]->drag_yscale != 1.) {
		cp->entry[i]->drag_yscale = 1.;
	    }
	    if((ze = (DataEntry *)cp->entry[i]->mag_entry) != NULL) {
		if(ze->drag_yscale != 1.) {
		    ze->drag_yscale = 1.;
		}
	    }
	}
	if(dataHeight < 1) dataHeight = 1;
	if(dataSeparation < 1) dataSeparation = 1;

	w->c_plot.dataHeight = dataHeight;
	w->c_plot.dataSeparation = dataSeparation;

	position(w);

	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);
	if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	{
	    z->axes.use_screen = False;
	    _AxesRedraw((AxesWidget)z);
	    _CPlotRedraw(z);
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	    z->axes.use_screen = True;
	}
}

/** 
 *  
 */
void
_CPlotGetXLimits(CPlotWidget w, double *xmin, double *xmax)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	double xlength;
	int i, label_width, min_x, max_x, tic, xdif;
	Rectangle r;
	AxesParm a;
	DrawStruct d;

	if(w->core.width <= 0 || w->core.height <= 0) return;

	r = _AxesLayout((AxesWidget)w);
	a = ax->a;
	for(i = 0; i < 4; i++) {
	    a.axis_segs[i].n_segs = 0;
	    a.axis_segs[i].size_segs = 0;
	    a.axis_segs[i].segs = NULL;
	}
	for(i = 0; i < MAXLAB; i++) {
	    a.xlab[i] = NULL;
	    a.ylab[i] = NULL;
	}
	SetDrawArea(&d, r.x, r.y, r.x + r.width - 1, r.y - r.height + 1);
//	SetScale(&d, 0., 0., 1., 1.);
	SetScale(&d, cp->tmin, 0., cp->tmax, 1.);

	*xmin = cp->tmin;
	*xmax = cp->tmax;
	a.ymin = (ax->y1[0] < ax->y2[0]) ? ax->y1[0] : ax->y2[0];
	a.ymax = (ax->y2[0] > ax->y1[0]) ? ax->y2[0] : ax->y1[0];
	gdrawAxis(&a, &d, *xmin, *xmax, ax->x_axis, ax->y2[ax->zoom],
		ax->y1[ax->zoom], ax->y_axis, ax->tickmarks_inside,
		ax->display_axes_labels, ax->time_scale,
		_AxesTimeFactor((AxesWidget)w, fabs(*xmax - *xmin), False));

	for(i = 0; i < 4; i++) Free(a.axis_segs[i].segs);
	for(i = 0; i < MAXLAB; i++) {
	    Free(a.xlab[i]);
	    Free(a.ylab[i]);
	}

	label_width = cp->max_tag_width;

	xlength = *xmax - *xmin;

	min_x = unscale_x(&d, *xmin);
	max_x = unscale_x(&d, *xmax);

	tic = (int)(a.ytic*d.unscalex);
	if(tic <= 0) tic = 1;
	if(cp->display_tags || cp->display_amplitude_scale) {
	    label_width += 5;
	    xdif = max_x - min_x - label_width - tic;
	    if(xdif <= 0) xdif = 1;
	    *xmin -= (double)label_width*xlength/xdif;
	}
	else {
	    xdif = max_x - min_x - 2*tic;
	    if(xdif <= 0) xdif = 1;
	    *xmin -= tic*xlength/xdif;
	}
	*xmax += tic*xlength/xdif;
}

/** 
 *  Get the maximum width of the waveform tags.
 *  @param[in] w A CPlotWidget pointer.
 *  @returns The maximum pixel width of the waveform tags.
 *  
 */
int CPlotClass::getMaxTagWidth()
{
    return cw->c_plot.max_tag_width;
}

int CPlotClass::getMaxAmpWidth()
{
    CPlotPart *cp = &cw->c_plot;
    int i, amp_width;

    amp_width = 0;
    for(i = 0; i < cp->num_entries; i++) {
	DataEntry *entry = cp->entry[i];
	if(cp->display_amplitude_scale && amp_width < entry->amp_width) {
	    amp_width = entry->amp_width;
	}
    }
    return amp_width;
}

static int
maxTagWidth(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	XCharStruct overall;
	int i, tag_width, ascent, descent, direction;

	XTextExtents(cp->tag_font, "0123456789", 10, &direction,
			&ascent, &descent, &overall);

	tag_width = 0;
	for(i = 0; i < cp->num_entries; i++) {
	    DataEntry *entry = cp->entry[i];
	    CPlotGetTagDimensions(w, entry->tag, &entry->tag_width,
				&entry->tag_height);

	    if(tag_width < entry->tag_width) tag_width = entry->tag_width;

	    if(cp->display_amplitude_scale && tag_width < entry->amp_width) {
		tag_width = entry->amp_width;
	    }
	}
	return tag_width;
}


/** 
 *  Set the maximum width of the waveform tags.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] tag_width The maximum tag width in pixels.
 *  
 */
void CPlotClass::setMaxTagWidth(int tag_width)
{
    cw->c_plot.max_tag_width = tag_width;
}

static void
SortEntries(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	int i, j, k, m, n;
	DataEntry **list = NULL;
	DataEntry *entry;

	if(cp->num_entries <= 0) return;

	cp->need_sort = False;
	n = cp->num_entries*sizeof(DataEntry *);
	if( !(list = (DataEntry **)MallocIt(w, n)) )
	{
		return;
	}
	memcpy(list, cp->entry, n);
	for(i = j = 0; i < cp->num_entries; i++) if(list[i] != NULL)
	{
		for(k = 0; k < NUM_COMPONENTS; k++)
			if(list[i]->comps[k] != NULL)
		{
			cp->entry[j++] = list[i]->comps[k];
		}
		entry = list[i];
		for(k = 0; k < NUM_COMPONENTS; k++)
		{
			for(m = 0; m < cp->num_entries; m++)
			{
				if(entry->comps[k] == list[m])
				{
					list[m] = NULL;
				}
			}
		}
	}
	Free(list);
}

static void
DisplayNewCurve(CPlotWidget w, DataCurve *curve, Boolean redraw)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;
	CPlotWidget z;

	z = (CPlotWidget)ax->mag_to;

	if(redraw)
	{
	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw(w);
	    if(!ax->redisplay_pending)
	    {
		_AxesRedisplayAxes((AxesWidget)w);
		_CPlotRedisplay(w);
		_AxesRedisplayXor((AxesWidget)w);
	    }
	    if(z != NULL)
	    {
		z->axes.zoom = 0;
		z->axes.x1[0] = ax->x1[ax->zoom];
		z->axes.x2[0] = ax->x2[ax->zoom];
		z->axes.y1[0] = ax->y1[ax->zoom];
		z->axes.y2[0] = ax->y2[ax->zoom];

		MagCurve(w, cp->curve[cp->num_curves-1]);

		_AxesRedraw((AxesWidget)z);
		_CPlotRedraw(z);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
	else
	{
	    _CPlotRedrawCurve(w, curve);
	    if(!ax->redisplay_pending)
	    {
		_CPlotRedisplayCurve(w, curve);
		if(ax->use_pixmap && XtWindow(w)) {
		    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
				0, 0, w->core.width, w->core.height, 0, 0);
		}
	    }
	    if(z != NULL)
	    {
		i = cp->num_curves-1;
		MagCurve(w, cp->curve[i]);
		_CPlotRedrawCurve(z, z->c_plot.curve[i]);
		_CPlotRedisplayCurve(z, z->c_plot.curve[i]);
		if(z->axes.use_pixmap && XtWindow(z)) {
		    XCopyArea(XtDisplay(z), z->axes.pixmap, XtWindow(z),
			z->axes.axesGC, 0, 0, z->core.width, z->core.height,
			0, 0);
		}
	    }
	}
}

static void
prepEntry(CPlotWidget w, DataEntry *entry)
{
	AxesPart *ax = &w->axes;

	DoDeci(w, entry);

	visibleMinMax(w, entry);

	entry->scale_width = ax->clipx2 - ax->clipx1;
	entry->scale_height = ax->clipy2 - ax->clipy1;
	entry->tbeg = entry->ts->tbeg();
	entry->tend = entry->ts->tend();

	entry->calib_applied = entry->ts->getMethod("CalibData") ? True : False;
}

static Boolean
InitDataEntry(CPlotClass *cplot, CPlotWidget w, DataEntry *entry,
		GTimeSeries *ts, CPlotInputStruct *input)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	double dmin, dmax, dmean;
	DataEntry data_entry_init = DATA_ENTRY_INIT;

	memcpy(entry, &data_entry_init, sizeof(DataEntry));
	entry->type = CPLOT_DATA;
	stringcpy(entry->tag, input->tag.c_str(), sizeof(entry->tag));

	entry->ts = ts;
	entry->w = new Waveform(ts);
	entry->w->addOwner(cp->owner_object);
	entry->w->cplot = cplot;
	entry->w->ts = entry->ts;
	entry->w->default_order = cp->num_entries-1;

	ts->addOwner(cp->owner_object);

	entry->w->channel = input->chan;

	entry->tbeg = ts->tbeg();
	entry->tend = ts->tend();
	entry->length = ts->duration();

//	entry->mean = ts->mean();
//	entry->ymin = ts->dataMin() - entry->mean;
//	entry->ymax = ts->dataMax() - entry->mean;

	ts->dataStats(&dmin, &dmax, &dmean);
	entry->mean = dmean;
	entry->ymin = dmin - entry->mean;
	entry->ymax = dmax - entry->mean;

	entry->xscale = entry->yscale = 0.;
	entry->drag_yscale = 1.;
		
	entry->w->selected = False;
	entry->w->fg = input->color;
	if((unsigned int)entry->w->fg == w->core.background_pixel) {
	    entry->w->fg = ax->fg;
	}

	DrawTag(w, entry);

	entry->arrlab = NULL;

	entry->display_time = input->display_t0;

	entry->w->scaled_x0 = input->display_t0;
	entry->w->scaled_y0 = 0.;

	entry->w->begSelect = 0.;
	entry->w->endSelect = 0.;


	entry->use_ts = entry->ts;

	entry->component = input->component;
	if(entry->component < 0 || entry->component > 3) {
	    entry->component = 0;
	}

/*
	x = (entry->length)/(x2-x1);
	for(i = 0; i < MAX_DECI; i++)
	{
		if(entry->p[i].ts.ntotal > 0 && x < entry->p[i].span)
		{
			entry->use_ts = &entry->p[i].ts;
			break;
		}
	}

	visibleMinMax(w, entry);
******/

	return(True);
}

static void
FindComponents(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	int i, j, ic;
	char c;
	const char *ch, *chan, *sta, *st;
	DataEntry *e;
	double t0;

	for(i = 0; i < NUM_COMPONENTS; i++)
	{
	    entry->comps[i] = (DataEntry *)NULL;
	    entry->w->c[i] = (Waveform *)NULL;
	}

	chan = entry->w->channel.c_str();
	sta  = entry->ts->sta();

	if( !entry->component ) // use chan to determine the component
	{
	    i = (int)strlen(chan);
	    c = (i > 0) ? chan[i-1] : '\0';

	    for(ic = 0; ic < NUM_COMPONENTS; ic++)
	    {
		if(c == clist[ic] || c == Clist[ic] || c == vlist[ic] ||
		   c == rlist[ic] || c == Rlist[ic] || c == nlist[ic]) break;
	    }
	    if(ic == NUM_COMPONENTS)
	    {
		entry->comps[0] = entry;
		entry->w->c[0] = entry->w;
		return;
	    }
	}
	else {	// the component was specified in the CPlotInputStruct
	    ic = entry->component - 1;
	}
	entry->comps[ic] = entry;
	entry->w->c[ic] = entry->w;

	t0 = entry->ts->tbeg();

	for(i = 0; i < cp->num_entries; i++)
	    if(cp->entry[i] != entry && cp->entry[i]->comps[ic] == NULL)
	{
	    e = cp->entry[i];
	    for(j = 0; j < NUM_COMPONENTS; j++)
	    {
		if(entry->comps[j] == NULL && e == e->comps[j] &&
		    fabs(t0 - e->ts->tbeg()) < cp->component_time_limit)
		{
		    break;
		}
	    }
	    if(j < NUM_COMPONENTS)
	    {
		ch = e->w->channel.c_str();
		st = e->ts->sta();
		if(!strcasecmp(sta, st) && (int)strlen(chan) == (int)strlen(ch)
			&& !strncasecmp(chan, ch, (int)strlen(ch)-1))
		{
		    entry->comps[j] = e;
		    entry->w->c[j] = e->w;
		    e->comps[ic] = entry;
		    e->w->c[ic] = entry->w;
		}
	    }
	}
}

/** 
 *  
 */
void
_CPlotComponentsOnOff(CPlotWidget w, Boolean place)
{
	CPlotPart *cp = &w->c_plot;
	int i, j1, j2, k;
	double dy, y_max=0.;
	DataEntry *e, *f, *g;

	/* if CPLOT_ALL_COMPONENTS, first place the z component,
	 * then put n and e beneath the z.
	 */
	if(cp->display_components == CPLOT_ALL_COMPONENTS) {
	    j1 = j2 = 2;
	}
	else if(cp->display_components == CPLOT_HORIZONTAL_COMPONENTS) {
	    j1 = 0; j2 = 1;
	}
	else if(cp->display_components >= 0 && cp->display_components <= 2) {
	    j1 = j2 = cp->display_components;
	}
	else {
	    return;
	}

	for(i = 0; i < cp->num_entries; i++)
	{
	    e = cp->entry[i];
	    if((e == e->comps[j1] || e == e->comps[j2]) && !e->on)
	    {
		for(k = NUM_COMPONENTS-1; k >= 0; k--) if(k != j1 && k != j2) {
		    if(e->comps[k] != NULL && e->comps[k]->on)break;
		}
		if(k >= 0)
		{
		    f = e->comps[k];
		    e->w->scaled_x0 = f->w->scaled_x0 +
			e->ts->tbeg() - f->ts->tbeg();
		    e->w->scaled_y0 = f->w->scaled_y0;
		    e->yscale = f->yscale;
		    e->on = True;
		}
	    }
	}
	for(i = 0; i < cp->num_entries; i++)
	{
	    if( cp->entry[i] != cp->entry[i]->comps[j1] &&
		cp->entry[i] != cp->entry[i]->comps[j2])
	    {
		cp->entry[i]->on = False;
		cp->entry[i]->w->selected = False;
	    }
	}
	for(i = 0; i < cp->num_entries; i++)
	{
	    e = cp->entry[i];
	    if((e == e->comps[j1] || e == e->comps[j2]) && !e->on)
	    {
		for(k = 0; k < cp->num_entries; k++) {
		    if(cp->entry[k]->on) {
			y_max = cp->entry[k]->w->scaled_y0;
			break;
		    }
		}
		for(k = 0; k < cp->num_entries; k++) {
		    if(cp->entry[k]->on) {
			if(y_max < cp->entry[k]->w->scaled_y0) {
			    y_max = cp->entry[k]->w->scaled_y0;
			}
		    }
		}
		e->on = True;
		e->w->scaled_y0 = y_max + 1.;
	    }
	}
	if(cp->display_components != CPLOT_ALL_COMPONENTS) {
	    if(place) placeEntries(w);
	    return;
	}

	for(i = 0; i < cp->num_entries; i++)
	{
	    e = cp->entry[i];
	    for(k = NUM_COMPONENTS-1; k >= 0; k--) {
		if(e->comps[k] != NULL && e->comps[k]->on) break;
	    }
	    if(k < 0)
	    {
		for(k = NUM_COMPONENTS-1; k >= 0; k--) {
		    if(e->comps[k] != NULL) break;
		}
		if(k >= 0) {
		    e->comps[k]->on = True;
		    for(k = 0; k < cp->num_entries; k++) {
			if(cp->entry[k]->on) {
			    y_max = cp->entry[k]->w->scaled_y0;
			    break;
			}
		    }
		    for(k = 0; k < cp->num_entries; k++) {
			if(cp->entry[k]->on) {
			    if(y_max < cp->entry[k]->w->scaled_y0) {
				y_max = cp->entry[k]->w->scaled_y0;
			    }
			}
		    }
		}
	    }
	}
	for(i = 0; i < cp->num_entries; i++)
	{
	    e = cp->entry[i];
	    if(!e->on)
	    {
		for(k = NUM_COMPONENTS-1; k >= 0; k--)
		{
		    if(e->comps[k] != NULL && e->comps[k]->on) break;
		}
		if(k >= 0)
		{
		    f = e->comps[k];
		    dy = .01;
		    for(int j = NUM_COMPONENTS-1; j >= 0; j--)
			if(j != k && e->comps[j] != NULL)
		    {
			g = e->comps[j];
			g->on = True;
			g->w->scaled_y0 = f->w->scaled_y0 + dy;
			dy += .1;
			g->w->scaled_x0 = f->w->scaled_x0 + 
			    g->ts->tbeg() - f->ts->tbeg();
			g->yscale = f->yscale;
		    }
		}
	    }
	}
	if(place) placeEntries(w);
}

static void
placeEntries(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	int i, n;
	DataEntry **entry;

	if(cp->num_entries <= 0) return;

	if( !(entry = (DataEntry **)MallocIt(w,
		cp->num_entries*sizeof(DataEntry *))) ) return;

	for(i = n = 0; i < cp->num_entries; i++) {
	    if(cp->entry[i]->on) entry[n++] = cp->entry[i];
	}

	qsort(entry, n, sizeof(DataEntry *), sort_by_y0);

	for(i = 0; i < n; i++) {
	    entry[i]->w->scaled_y0 = i + 1;
	}
	Free(entry);
}

static void
DoDeci(CPlotWidget w, DataEntry *entry)
{
	int i, n, npixels, npts, deci_min;
	double rate;
	GTimeSeries *last_ts;
	
	for(i = 0; i < MAX_DECI; i++) {
	    if(entry->p[i].ts != NULL) {
		entry->p[i].ts->removeOwner(w->c_plot.owner_object);
		entry->p[i].ts = NULL;
	    }
	}
	npixels = (w->core.width > w->core.height) ? w->core.width :
				w->core.height;
	if(npixels < 1000) npixels = 1000;

	npts = entry->ts->length();
	deci_min = 6;
	for(i = 0, n = deci_min;  2*npts/n > npixels; i++, n *= 2);
	if(i > MAX_DECI) i = MAX_DECI;
	if(i > 0)
	{
	    entry->num_deci = i;
	    entry->p[entry->num_deci-1].span = npts/(deci_min*npixels);

	    entry->p[entry->num_deci-1].ts =
			entry->ts->decimate(deci_min, False);

	    last_ts = entry->p[entry->num_deci-1].ts;

	    for(i = entry->num_deci-2, rate=2*deci_min; i >= 0; i--, rate *= 2)
	    {
		entry->p[i].ts = last_ts->decimate(4, False);
		entry->p[i].span = (int)(npts/(rate*npixels));
		last_ts = entry->p[i].ts;
	    }
	}
	else
	{
	    entry->num_deci = 0;
	}
	for(i = 0; i < entry->num_deci; i++) {
	    CPlotPart *cp = &w->c_plot;
	    entry->p[i].ts->addOwner(cp->owner_object);
	}
}

static void
CPlotHardCopy(AxesWidget widget, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, PrintParam *p)
{
	CPlotWidget w = (CPlotWidget)widget;
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i, j, k, m, x, y, ix1, ix2, iy1, iy2;
	int jbeg, jend, max_jbeg;
	int lab_x, lab_y, ntag_lines, pos;
	Boolean last_in;
	float fac;
	double x0, x1, y1, x2, y2, x_start, x_end, scaled_x, scaled_y, x_1, x_2;
	double scaley;
	DataEntry *entry;
	DataCurve *curve;
	SegArray segs;
	char *c, longest[100];
	int	tag_width;

	if(!ax->reverse_x) {
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	}
	else {
	    x1 = ax->x2[ax->zoom];
	    x2 = ax->x1[ax->zoom];
	}
	if(!ax->reverse_y) {
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	}
	else {
	    y1 = ax->y2[ax->zoom];
	    y2 = ax->y1[ax->zoom];
	}
	
	fac = (float)(d->ix2 - d->ix1)/1000.;
	d->s = &segs;
	d->s->segs = NULL;
	d->s->size_segs = 0;
	d->s->nsegs = 0;
	for(m = 0; m < cp->num_curves; m++)
	    if(cp->curve[m]->on && ( cp->curve[m]->npts > 0 ||
		(cp->curve[m]->type != CPLOT_CURVE &&
		cp->curve[m]->type != CPLOT_TTCURVE)))
	{
	    curve = cp->curve[m];
	    d->s->nsegs = 0;

	    if(p->color) cp->cplot_class->postColor(fp, curve->fg);

	    if(curve->type == CPLOT_CURVE || curve->type == CPLOT_TTCURVE)
	    {
		plotxyd(d, curve->npts, curve->x, curve->y, x1, x2, y1, y2);
		lab_x = unscale_x(d,scale_x(&ax->d,curve->lab_x));
		lab_y = unscale_y(d,scale_y(&ax->d,curve->lab_y));
		mapalf(d, lab_x, lab_y, fac*alpha_size, curve->lab_angle, 1,
			(char *)curve->lab.c_str());
		if(d->s->nsegs > 0)
		{
		    fprintf(fp, "%d %d M\n",d->s->segs[0].x1, d->s->segs[0].y1);
		    fprintf(fp, "%d %d D\n",d->s->segs[0].x2, d->s->segs[0].y2);
		}
		for(i = 1; i < d->s->nsegs; i++)
		{
		    if( d->s->segs[i].x1 != d->s->segs[i-1].x2 ||
			d->s->segs[i].y1 != d->s->segs[i-1].y2)
		    {
			fprintf(fp,"%d %d M\n",d->s->segs[i].x1,
				d->s->segs[i].y1);
		    }
		    fprintf(fp, "%d %d D\n",d->s->segs[i].x2, d->s->segs[i].y2);
		}
	    }
	    else
	    {
		plotsymd(d, curve->npts, curve->x, curve->y, curve->type,
			30, x1, x2, y1, y2);
		if(d->s->nsegs > 0)
		{
		    fprintf(fp, "%d %d M\n",d->s->segs[0].x1, d->s->segs[0].y1);
		    fprintf(fp, "%d %d D\n",d->s->segs[0].x2, d->s->segs[0].y2);
		}
		for(i = 1; i < d->s->nsegs; i++)
		{
		    if( d->s->segs[i].x1 != d->s->segs[i-1].x2 ||
			d->s->segs[i].y1 != d->s->segs[i-1].y2)
		    {
			fprintf(fp, "%d %d M\n", d->s->segs[i].x1,
						d->s->segs[i].y1);
		    }
		    fprintf(fp, "%d %d D\n",d->s->segs[i].x2, d->s->segs[i].y2);
		}
	    }
	}
	if(ax->x_axis != LEFT) {
	    x_1 = x1;
	    x_2 = x2;
	    scaley = ax->d.scaley;
	}
	else {
	    x_1 = y1;
	    x_2 = y2;
	    scaley = -ax->d.scalex;
	}
	ix1 = unscale_x(d, x1);
	ix2 = unscale_x(d, x2);
	iy1 = unscale_y(d, y1);
	iy2 = unscale_y(d, y2);

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


	tag_width = 99999;
	for(m = 0; m < cp->num_entries; m++) if(cp->entry[m]->on
		&& cp->entry[m]->ts->length() > 0
		&& cp->entry[m]->w->visible)
	{
	    GDataPoint *d1 = NULL, *d2 = NULL;

	    entry = cp->entry[m];
	    x_start = x_1 - entry->w->scaled_x0 + entry->tbeg;
	    x_end   = x_2 - entry->w->scaled_x0 + entry->tbeg;
	    if((d1 = entry->ts->lowerBound(x_start)) == NULL ||
	       (d2 = entry->ts->upperBound(x_end)) == NULL ||
	       (d1->segmentIndex() == d2->segmentIndex() &&
			d1->index() == d2->index()))
	    {
		if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
		continue;
	    }
	    if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
	    if(entry->tag_width < tag_width) tag_width = entry->tag_width;
	}

	for(m = 0; m < cp->num_entries; m++) if(cp->entry[m]->on
		&& cp->entry[m]->ts->length() > 0 && cp->entry[m]->w->visible)
	{
	    GDataPoint *d1 = NULL, *d2 = NULL;
	    entry = cp->entry[m];
	    x_start = x_1 - entry->w->scaled_x0 + entry->ts->tbeg();
	    x_end   = x_2 - entry->w->scaled_x0 + entry->ts->tbeg();
	    if((d1 = entry->ts->lowerBound(x_start)) == NULL ||
	       (d2 = entry->ts->upperBound(x_end)) == NULL ||
	       (d1->segmentIndex() == d2->segmentIndex() &&
			d1->index() == d2->index()))
	    {
		if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
		continue;
	    }

	    d->s->nsegs = 0;

	    last_in = False;
	    max_jbeg = 0;
	    for(k = d1->segmentIndex(); k <= d2->segmentIndex(); k++)
	    {
		GTimeSeries *ts = entry->ts;
		GSegment *s = ts->segment(k);

		jbeg = (k == d1->segmentIndex()) ? d1->index() : 0;
		if(jbeg > max_jbeg) max_jbeg = jbeg;
		jend = (k == d2->segmentIndex()) ? d2->index() : s->length()-1;

		x0 = entry->w->scaled_x0 + s->tbeg() - ts->tbeg();

		for(j = jbeg; j <= jend; j++)
		{
		    scaled_x = x0 + j*s->tdel();

		    scaled_y = entry->w->scaled_y0 - entry->yscale*
				(s->data[j]-entry->mean)*scaley;

		    if(ax->x_axis != LEFT)
		    {
			if(last_in)idraw(d, scaled_x, scaled_y);
			else	   imove(d, scaled_x, scaled_y);
		    }
		    else
		    {
			    if(last_in)idraw(d, scaled_y, scaled_x);
			    else	   imove(d, scaled_y, scaled_x);
		    }
		    last_in = True;
		}
		if(k < d2->segmentIndex() && !ts->continuous(k+1,0.,0.))
		{
		    last_in = False;
		}
	    }
	    if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
	    iflush(d);

	    if(p->color) {
		Pixel pix = (entry->w->selected && cp->redraw_selected_data)
				? ax->select_fg : entry->w->fg;
		cp->cplot_class->postColor(fp, pix);
	    }
	    fprintf(fp, "%d slw\n", p->data_linewidth);
	    if(d->s->nsegs > 0)
	    {
		fprintf(fp, "%d %d M\n",d->s->segs[0].x1,d->s->segs[0].y1);
		fprintf(fp, "%d %d D\n",d->s->segs[0].x2,d->s->segs[0].y2);
	    }
	    for(i = 1; i < d->s->nsegs; i++)
	    {
		if(d->s->segs[i].x1 != d->s->segs[i-1].x2 ||
				d->s->segs[i].y1 != d->s->segs[i-1].y2)
		{
		    fprintf(fp, "%d %d M\n",d->s->segs[i].x1, d->s->segs[i].y1);
		}
		fprintf(fp, "%d %d D\n", d->s->segs[i].x2, d->s->segs[i].y2);
	    }
	    fprintf(fp, "0 setlinewidth\n");

	    if(p->color) cp->cplot_class->postColor(fp, ax->fg);

	    if(cp->display_arrivals != CPLOT_ARRIVALS_OFF)
	    {
		fprintf(fp,"/%s findfont %d scalefont setfont\n",
				p->fonts.arr_font, p->fonts.arr_fontsize);
		for(j = 0; j < entry->n_arrlab; j++)
		{
                    if (entry->arrlab[j].a_entry->on)
                    {
                        cp->cplot_class->postColor(fp,
				entry->arrlab[j].a_entry->fg);
		        HardDrawOneArrival(w, fp, p->arrPlace, font_scale,
				d, m, j);
                    }
		}
	    }
	    if (cp->display_predicted_arrivals)
	    {
		fprintf(fp,"/%s findfont %d scalefont setfont\n",
				p->fonts.arr_font, p->fonts.arr_fontsize);
		HardDrawPredArrivals(w, fp, p->arrPlace,
				font_scale, d, entry, p->fonts.arr_fontsize);
	    }
	    if(ax->x_axis == BOTTOM || ax->y_axis == BOTTOM)
	    {
		if (max_jbeg > 0 || !p->tagPlace)
		{
		    x = unscale_x(d, scale_x(&ax->d, 15 + entry->tag_width -4));
		}
		else
		{
		    x = unscale_x(d, scale_x(&ax->d,
				entry->lab_x + entry->tag_width - 4)); 
		}
		/* entry->lab_x + tag_width - 4));  */
		pos = 2;
	    }
	    else
	    {
		if (max_jbeg > 0 || !p->tagPlace)
		{
		    x = unscale_x(d, scale_x(&ax->d, 15 + entry->tag_width/2));
		}
		else
		{
		    x = unscale_x(d, scale_x(&ax->d,
				entry->lab_x + entry->tag_width/2));
		}
		/* entry->lab_x + tag_width/2)); */
		pos = 1;
	    }
	    y = entry->tag_y + (int)(.5*entry->tag_height);
	    y = unscale_y(d, scale_y(&ax->d, y));

	    fprintf(fp, "/%s findfont %d scalefont setfont\n", 
			p->fonts.tag_font, p->fonts.tag_fontsize);

	    HardDrawScale(w, fp, a, d, d->s, font_scale, p->full_scale,
				entry, x, y);

	    if(cp->display_tags)
	    {
		/*
		 * get number of lines in the tag
		 */
		i = j = ntag_lines = 0;
		while(entry->tag[j] != '\0')
		{
		    ntag_lines++;
		    for(j = i; entry->tag[j] != '\0' &&
					entry->tag[j] != '\n'; j++);
		    i = j + 1;
		}

		/* put the limits on the stack for use later */
		fprintf(fp, "%f %f %f %f\n", (float)ix1/font_scale,
			(float)ix2/font_scale, (float)iy1/font_scale,
			(float)iy2/font_scale);


		fprintf(fp, "N %d %d M\n", x, y);

		fprintf(fp, "%.5f dup scale currentpoint\n",font_scale);

		/* find max width */
		fprintf(fp, "0\n");
		i = j = 0;
		while(entry->tag[j] != '\0')
		{
		    for(j = i; entry->tag[j] != '\0' &&
					entry->tag[j] != '\n'; j++);
		    strncpy(longest, entry->tag+i, j-i);
		    longest[j-i] = '\0';
		    c = _Axes_check_parentheses(longest);
		    i = j + 1;
		    fprintf(fp, "(%s) false charpath pathbbox\n",c);
		    Free(c);
		    fprintf(fp, "pop exch pop exch sub\n");
		    fprintf(fp, "2 copy gt {pop} {exch pop} ifelse\n");
		    fprintf(fp, "2 index 2 index N moveto\n");
		}

		/* find max height */
		fprintf(fp, "0\n");
		i = j = 0;
		while(entry->tag[j] != '\0')
		{
		    for(j = i; entry->tag[j] != '\0' &&
					entry->tag[j] != '\n'; j++);
		    strncpy(longest, entry->tag+i, j-i);
		    longest[j-i] = '\0';
		    c = _Axes_check_parentheses(longest);
		    i = j + 1;
		    fprintf(fp, "(%s) false charpath pathbbox\n",c);
		    Free(c);
		    fprintf(fp,"exch pop 3 -1 roll pop exch sub\n");
		    fprintf(fp, "2 copy gt {pop} {exch pop} ifelse\n");
		}

		/* get width  = max width + max height */
		fprintf(fp, "dup 2 index add\n");

		/* get height = (nlines+1)*max_h/2 + nlines*max_h */
		fprintf(fp, "1 index dup 2 div %d mul\n",ntag_lines+1);
		fprintf(fp, "exch %d mul add\n", ntag_lines);

		if(pos == 2)  /* right justified */
		{
		    fprintf(fp, "4 -1 roll pop 5 -1 roll\n");
		    fprintf(fp, "2 I sub 5 -1 roll 2 I\n");
		    fprintf(fp, "2 div add\n");
		}

		/* check if the rectangle is outside the limits */
		fprintf(fp, "1 I 9 I 2 copy\n");
		fprintf(fp, "lt {exch pop 3 -1 roll pop exch}\n");
		fprintf(fp, "{pop pop} ifelse\n");
		fprintf(fp, "1 I 8 I 5 I sub 2 copy\n");
		fprintf(fp, "gt {exch pop 3 -1 roll pop exch}\n");
		fprintf(fp, "{pop pop} ifelse\n");

		/* clean up the stack */
		fprintf(fp, "9 -1 roll pop 8 -1 roll pop\n");
		fprintf(fp, "7 -1 roll pop 6 -1 roll pop\n");

		fprintf(fp, "moveto currentpoint\n");

		fprintf(fp, "2 copy newpath moveto 3 I\n");
		fprintf(fp, "0 rlineto 2 I neg 0 exch rlineto\n");
		fprintf(fp, "4 -1 roll neg 0 rlineto\n");
		fprintf(fp, "3 -1 roll 0 exch rlineto closepath\n");
		/*
		 * erase the inside of the rectangle, then draw it.
		 */
		if(p->color) {
		    fprintf(fp, "gsave clip ");
		    if(entry->w->selected) {
			cp->cplot_class->postColor(fp, entry->w->fg);
			fprintf(fp, "fill grestore 1 setgray stroke\n");
		    }
		    else {
			fprintf(fp, "1 setgray fill grestore ");
			cp->cplot_class->postColor(fp, entry->w->fg);
			fprintf(fp, "stroke\n");
		    }
		}
		else {
		    fprintf(fp, "gsave clip 1 setgray fill grestore ");
		    fprintf(fp, "0 setgray stroke\n");
		}

		fprintf(fp, "exch 2 index 2 div add exch\n");
		fprintf(fp, "2 copy moveto pop exch dup 2 div add\n");
		i = j = 0;
		while(entry->tag[j] != '\0')
		{
		    fprintf(fp, "0 1 I neg rmoveto currentpoint\n");
		    for(j = i; entry->tag[j] != '\0' &&
					entry->tag[j] != '\n'; j++);
		    strncpy(longest, entry->tag+i, j-i);
		    longest[j-i] = '\0';
		    c = _Axes_check_parentheses(longest);
		    i = j + 1;
		    fprintf(fp, "(%s) show moveto\n",c);
		    Free(c);
		}
		fprintf(fp, "pop pop\n"); /* clean up */
			
		/* return to previous scale and clipping.
		 */
		fprintf(fp, "1 %.5f div dup s\n", font_scale);
		if(p->color) {
		    fprintf(fp, "0 setgray\n");
		}
	    }
	}
	fprintf(fp, "initclip\n");
	Free(d->s->segs);
}

static void
HardDrawPredArrivals(CPlotWidget w, FILE *fp, Boolean do_arrPlace,
	float font_scale, DrawStruct *d, DataEntry *entry, int fontsize)
{
	int j, x, y, entry_x0, entry_y0;
	AxesPart *ax = &w->axes;
	CPlotPredArr *pred_arr;

	if( !entry->beg ) return;

	entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);

	for(j = 0; j < entry->npred_arr; j++)
	{
		pred_arr = &entry->pred_arr[j];

		x = (int)(entry_x0 + pred_arr->x);
		x = unscale_x(d, scale_x(&w->axes.d, x));
		y = entry_y0 + pred_arr->y2;
		y = unscale_y(d, scale_y(&w->axes.d, y));
		if(do_arrPlace)
		{
			y -= 10;
			fprintf(fp, "%d %d M\n", x, y);
			fprintf(fp, "%d %d D\n", x, y-30);
			fprintf(fp, "%d %d M\n", x, y-30 -
				(int)(0.8*300*fontsize/72.));
		}
		else
		{
/* this makes the arrivals a little closer
			y -= 4;
			fprintf(fp, "%d %d M\n", x, y);
			fprintf(fp, "%d %d D\n", x, y-10);
			fprintf(fp, "%d %d M\n", x,
				y-10-(int)(0.8*300*fontsize/72.));
*/
			y -= 5;
			fprintf(fp, "%d %d M\n", x, y);
			fprintf(fp, "%d %d D\n", x, y-15);
			fprintf(fp, "%d %d M\n", x,
				y-15-(int)(0.8*300*fontsize/72.));
		}
		fprintf(fp, "(%s) %.5f PC\n", pred_arr->phase, font_scale);
	}
}

static void
HardDrawOneArrival(CPlotWidget w, FILE *fp, Boolean do_arrPlace,
		float font_scale, DrawStruct *d, int i, int j)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		x, y, y1, y2, entry_x0, entry_y0;
	float           percent;
	DataEntry	*entry;
	ArrivalLabel	*arrlab;

	entry = cp->entry[i];
	if(!entry->on || !entry->w->visible || !entry->beg) return;

	arrlab = &entry->arrlab[j];

	if(cp->display_arrivals == CPLOT_ARRIVALS_ONE_CHAN &&
		!DataSource::compareChan(entry->w->channel,
			arrlab->a_entry->a->chan))
	{
	    return;
	}
	entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);

	if(entry_x0 + arrlab->x > ax->clipx1 &&
	   entry_x0 + arrlab->x < ax->clipx2)
	{
	    x = (int)(entry_x0 + arrlab->x);
	    x = unscale_x(d, scale_x(&ax->d, x));
	    y = entry_y0 + arrlab->y2;
	    y = unscale_y(d, scale_y(&ax->d, y));

	    if (arrlab->a_entry->a->atype >= 0 &&
		arrlab->a_entry->a->atype < cp->v->arrival_types.size())
	    {
		CPlotArrivalType *arrtype =
		    cp->v->arrival_types[arrlab->a_entry->a->atype];

		fprintf(fp, "0 setlinecap\n");	
		fprintf(fp, "%d slw\n", arrtype->hardcopy_width_pix);	

		y1 = entry_y0;
		y1 = unscale_y(d, scale_y(&ax->d, y1));

		percent =(arrtype->max_display_value-arrlab->a_entry->a->value)/
		    (arrtype->max_display_value - arrtype->min_display_value);
 
		if (percent > 1.0) percent = 1.0;
		else if (percent < 0.0) percent = 0.0;
 
		y2 = y1 + (int) (arrtype->hardcopy_min_height_pix +
			(percent * (arrtype->hardcopy_max_height_pix -
				arrtype->hardcopy_min_height_pix)));

		fprintf(fp, "%d %d M\n", x, y1);
		fprintf(fp, "%d %d D\n", x, y2);

		if (do_arrPlace) {
		    fprintf(fp, "%d %d M\n", x, y2+10);
		}
		else {
		    fprintf(fp, "%d %d M\n", x, y2+5);
		}

		fprintf(fp, "1 setlinecap\n");	
		fprintf(fp, "0 slw\n");	
	    }
	    else if(do_arrPlace)
	    {
		y += 10;
		fprintf(fp, "%d %d M\n", x, y);
		fprintf(fp, "%d %d D\n", x, y+30);
		fprintf(fp, "%d %d M\n", x, y+40);
	    }
	    else
	    {
		y += 5;
		fprintf(fp, "%d %d M\n", x, y);
		fprintf(fp, "%d %d D\n", x, y+15);
		fprintf(fp, "%d %d M\n", x, y+20);
	    }
	    fprintf(fp, "(%s) %.5f PC\n",
		arrivalPhase(arrlab->a_entry->a), font_scale);
	}
}

static void
HardDrawScale(CPlotWidget w, FILE *fp, AxesParm *hard_a, DrawStruct *hard_d,
		SegArray *s, float font_scale, Boolean full_scale,
		DataEntry *entry, int tag_x, int tag_y)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
        int	i, j, lab_x, lab_y, tag_height, cut_off=0;
	int	x1, x2, x0, y0, iy1, iy2, ydif, height, ndigit;
	double	y, y1, y2, scaley;
	short	segs_y1;
	DrawStruct d;
	AxesParm a;
	DSegment *segs;
	AxesParm axes_parm_init = AXES_PARM_INIT;

	if(!cp->display_amplitude_scale || s->nsegs <= 0
		|| entry->ymin == entry->ymax)
	{
		return;
	}

	scaley = fabs(hard_d->scaley);

	x1 = unscale_x(hard_d, ax->x1[ax->zoom]);
	x2 = unscale_x(hard_d, ax->x2[ax->zoom]);
	if(x1 > x2)
	{
		x0 = x1;
		x1 = x2;
		x2 = x0;
	}
	if(s->segs[0].x1 >= x2 || s->segs[s->nsegs-1].x2 <= x1)
	{
		return;
	}
	iy1 = hard_d->iy1;
	iy2 = hard_d->iy2;
	if(iy1 < iy2)
	{
		y0  = iy1;
		iy1 = iy2;
		iy2 = y0;
	}
	for(i = 0; i < s->nsegs; i++)
	{
		if(s->segs[i].x1 >= x1 && s->segs[i].x1 <= x2)
		{
			if(iy1 > s->segs[i].y1) iy1 = s->segs[i].y1;
			if(iy2 < s->segs[i].y1) iy2 = s->segs[i].y1;
		}
		if(s->segs[i].x2 >= x1 && s->segs[i].x2 <= x2)
		{
			if(iy1 > s->segs[i].y2) iy1 = s->segs[i].y2;
			if(iy2 < s->segs[i].y2) iy2 = s->segs[i].y2;
		}
		if(s->segs[i].x1 < x1 && s->segs[i].x2 > x2)
		{
			if(iy1 > s->segs[i].y1) iy1 = s->segs[i].y1;
			if(iy1 > s->segs[i].y2) iy1 = s->segs[i].y2;
			if(iy2 < s->segs[i].y1) iy2 = s->segs[i].y1;
			if(iy2 < s->segs[i].y2) iy2 = s->segs[i].y2;
		}
	}
	ydif = abs(iy2 - iy1);
	if(entry->w->scaled_y0 < ax->y1[ax->zoom] - ydif*scaley ||
	   entry->w->scaled_y0 > ax->y2[ax->zoom] + ydif*scaley)
	{
		return;
	}
	if(ydif > .8*abs(hard_d->iy2 - hard_d->iy1))
	{
		ydif = (int)(.8*abs(hard_d->iy2 - hard_d->iy1));
	}

	memcpy(&a, &axes_parm_init, sizeof(AxesParm));
	a.fontMethod = hard_a->fontMethod;
	a.label_font_height = hard_a->label_font_height;
	a.label_font_width = hard_a->label_font_width;
	height = hard_a->label_font_height;

	tag_height = (int)fabs(entry->tag_height*ax->d.scaley/scaley);

	if(ydif < tag_height + 3*height)
	{
		ydif = tag_height + 3*height;
	}
	if(ydif < 3*height)
	{
		ydif = 3*height;
	}
	ydif /= 2;

	SetDrawArea(&d, 0, -ydif, 100, ydif);

	scaley = fabs(hard_d->scaley/ax->d.scaley)/entry->yscale;
	y1 = -ydif*scaley;
	y2 =  ydif*scaley;
	SetScale(&d, 0., y1, 1., y2);

	a.min_ylab = 3;
	i = (2*ydif -  tag_height)/height;
	if (i < 3) i = 3;
	if(i < a.max_ylab) a.max_ylab = i;
	if(a.max_ylab > 7) a.min_ylab = 5;

	nicex(y1, y2, a.min_ylab, a.max_ylab, &a.nylab, a.y_lab,
			&ndigit, &a.nydeci);

	a.top	 =  ydif;
	a.bottom = -ydif;
	a.ytic = .75*height*1./100.;

	gdrawYAxis(&a, &d, 2, 0., y1, y2, a.ytic, True, 0, 1, 0.);

	if (!full_scale)
	{
		cut_off = (int) a.nylab/2;
	}

	for(i = j = 0; i < a.nylab; i++)
		if (full_scale || i >= cut_off)
	{
		lab_x = a.r_ylab + tag_x;
		lab_y = unscale_y(&d, a.y_lab[i]) + tag_y;

		if(lab_y + .5*height < -.5*tag_height ||
		   lab_y - .5*height >  .5*tag_height)
		{
			fprintf(fp, "N %d %d M\n", lab_x, lab_y);
			fprintf(fp, "%.5f (%s) PR\n", font_scale, a.ylab[i]);
			j++;
		}
	}
	if(j <= 1)
	{
		y = (.5*tag_height + height)*scaley;
		a.min_ylab = 1;
		a.max_ylab = 1;
		nicex(y, y2, a.min_ylab, a.max_ylab, &a.nylab, a.y_lab,
				&ndigit, &a.nydeci);
		if(a.nylab > 0)
		{
			a.y_lab[2] = a.y_lab[0];
			a.y_lab[1] = 0.;
			a.y_lab[0] = -a.y_lab[0];
			a.nylab = 3;
			gdrawYAxis(&a, &d, 2, 0., y1, y2, a.ytic, True, 0,1,0.);
			lab_x = a.r_ylab + tag_x;
			for(i = 0; i < 3; i += 2)
				if (full_scale || i > 1)
			{
				lab_y = unscale_y(&d, a.y_lab[i]) + tag_y;
				fprintf(fp, "N %d %d M\n", lab_x, lab_y);
				fprintf(fp, "%.5f (%s) PR\n",
					font_scale, a.ylab[i]);
			}
		}
	}
	if(a.axis_segs[2].n_segs > 0 && a.nylab > 1)
	{
		segs = a.axis_segs[2].segs;
		if (full_scale)
			segs[0].y1 = unscale_y(&d, a.y_lab[0]);
		else
			segs[0].y1 = unscale_y(&d, a.y_lab[a.nylab-1]) -
				     ((unscale_y(&d, a.y_lab[a.nylab-1]) -
				       unscale_y(&d, a.y_lab[0]))/2);

		segs_y1 = segs[0].y1;

		segs[0].y2 = unscale_y(&d, a.y_lab[a.nylab-1]);
		for(j = 0; j < a.axis_segs[2].n_segs; j++)
		{
			if (!full_scale)
			{
				if (segs[j].y1 < segs_y1)
				{
					if (segs[j].y2 < segs_y1)
						continue;

					segs[j].y1 = segs_y1;
				}
				else if (segs[j].y2 < segs_y1)
				{
					if (segs[j].y1 < segs_y1)
						continue;

					segs[j].y2 = segs_y1;
				}
			}
			segs[j].x1 += tag_x;
			segs[j].x2 += tag_x;
			segs[j].y1 += tag_y;
			segs[j].y2 += tag_y;
			fprintf(fp, "%d %d M\n", segs[j].x1, segs[j].y1);
			fprintf(fp, "%d %d D\n", segs[j].x2, segs[j].y2);
		}
	}
	if(a.axis_segs[2].segs != NULL)
	{
		Free(a.axis_segs[2].segs);
	}
	for(i = 0; i < a.nylab; i++) Free(a.ylab[i]);
}

/** 
 *  Set the reference waveform. If a single waveform is selected, this function
 *  makes that waveform the reference waveform. (deprecated)
 *  @param[in] w A CPlotWidget pointer.
 *  
 */
void CPlotClass::setReference()
{
    CPlotPart *cp = &cw->c_plot;
    int i, n;
    DataEntry *ref=NULL, *old_ref;

    for(i = n = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w->selected) {
	    ref = cp->entry[i];
	    n++;
	}
    }
    if(n == 1 && ref == cp->ref) return;

    if((old_ref = cp->ref) != (DataEntry *)NULL &&
	old_ref->tag[(int)strlen(old_ref->tag)-1] == '*')
    {
	_CPlotRedisplayEntry(cw, old_ref);
	old_ref->tag[(int)strlen(old_ref->tag)-1] = '\0';
	DrawTag(cw, old_ref);
	_CPlotRedisplayEntry(cw, old_ref);
    }
    if(n == 0 || n > 1)
    {
	cp->ref = (DataEntry *)NULL;
	_AxesWarn((AxesWidget)cw, "One waveform must be selected.");
	return;
    }
    cp->ref = ref;
    _CPlotRedisplayEntry(cw, cp->ref);
    strcat(ref->tag, "*");
    DrawTag(cw, ref);
    _CPlotRedisplayEntry(cw, cp->ref);
}

/** 
 *  Get the reference waveform.  (deprecated)
 *  @param[in] w A CPlotWidget pointer.
 *  @returns The Waveform pointer corresponding to the reference waveform,
 *  or NULL if there is no reference waveform.
 */
Waveform * CPlotClass::getReference()
{
    if(cw->c_plot.ref != NULL) {
	return(cw->c_plot.ref->w);
    }
    return NULL;
}

/** 
 *  Find the waveform that an arrival is associated with.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival A CssArrivalClass that is current displayed in this
 *		CPlotWidget.
 *  @returns The Waveform pointer corresponding to the waveform that the
 * 		arrival is associated with. Returns NULL if the arrival is
 *		not found in this CPlotWidget.
 */
Waveform * CPlotClass::getArrivalData(CssArrivalClass *arrival)
{
    CPlotPart *cp = &cw->c_plot;
    DataEntry *entry;

    for(int i = 0; i < cp->num_entries; i++) {
	entry = cp->entry[i];
	
	for(int j = 0; j < entry->n_arrlab; j++) {
	    if(entry->arrlab[j].a_entry->a == arrival) return(entry->w);
	}
    }
    return((Waveform *)NULL);
}

int CPlotClass::getTable(const string &name, gvector<CssTableClass *> &v)
{
    CPlotPart *cp = &cw->c_plot;

    v.clear();
    if(!name.compare(cssAmpdescript))	     v.load(cp->v->ampdescripts);
    else if(!name.compare(cssAmplitude))     v.load(cp->v->amplitudes);
    else if(!name.compare(cssArrival))       v.load(cp->v->arrivals);
    else if(!name.compare(cssAssoc))         v.load(cp->v->assocs);
    else if(!name.compare(cssFilter))        v.load(cp->v->filters);
    else if(!name.compare(cssHydroFeatures)) v.load(cp->v->hydro_features);
    else if(!name.compare(cssInfraFeatures)) v.load(cp->v->infra_features);
    else if(!name.compare(cssNetmag))        v.load(cp->v->netmags);
    else if(!name.compare(cssOrigin))        v.load(cp->v->origins);
    else if(!name.compare(cssOrigerr))       v.load(cp->v->origerrs);
    else if(!name.compare(cssParrival))      v.load(cp->v->parrivals);
    else if(!name.compare(cssStamag))        v.load(cp->v->stamags);
    else if(!name.compare(cssStassoc))       v.load(cp->v->stassocs);
    else if(!name.compare(cssWftag))         v.load(cp->v->wftags);
    else if(!name.compare(cssWfdisc)) {
	for(int i = 0; i < cp->num_entries; i++)
	{
	    cvector<CssWfdiscClass> * cv = cp->entry[i]->w->ts->getWfdiscs();
	    if(cv) {
		for(int j = 0; j < cv->size(); j++) v.push_back(cv->at(j));
		delete cv;
	    }
	}
    }
    return v.size();
}

/** 
 *  Get all selected arrivals contained in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[out] arrivals An array of CssArrivalClass objects. The space for this
 *		array is allocated by this function.
 *  @returns the number of selected CssArrival objects.
 */
int CPlotClass::getSelectedTable(cvector<CssArrivalClass> &v)
{
    CPlotPart *cp = &cw->c_plot;

    v.clear();
    for(int i = 0; i < cp->v->arrival_entries.size(); i++) {
	ArrivalEntry *a_entry = cp->v->arrival_entries[i];
	if( a_entry->selected ) v.add(a_entry->a);
    }
    return v.size();
}

int CPlotClass::getSelectedTable(cvector<CssOriginClass> &v)
{
    CPlotPart *cp = &cw->c_plot;
    v.clear();
    for(int i = 0; i < cp->v->selected_tables.size(); i++) {
	if(cp->v->selected_tables[i]->nameIs(cssOrigin)) {
	    v.add((CssOriginClass *)cp->v->selected_tables[i]);
	}
    }
    return v.size();
}

cvector<CssArrivalClass> * CPlotClass::getArrivalRef() {
    return &cw->c_plot.v->arrivals;
}
cvector<CssOriginClass> * CPlotClass::getOriginRef() {
    return &cw->c_plot.v->origins;
}
cvector<CssOrigerrClass> * CPlotClass::getOrigerrRef() {
    return &cw->c_plot.v->origerrs;
}
cvector<CssAssocClass> * CPlotClass::getAssocRef() {
    return &cw->c_plot.v->assocs;
}
cvector<CssStamagClass> * CPlotClass::getStamagRef() {
    return &cw->c_plot.v->stamags;
}
cvector<CssStassocClass> * CPlotClass::getStassocRef() {
    return &cw->c_plot.v->stassocs;
}
cvector<CssNetmagClass> * CPlotClass::getNetmagRef() {
    return &cw->c_plot.v->netmags;
}
cvector<CssHydroFeaturesClass> * CPlotClass::getHydroFeaturesRef() {
    return &cw->c_plot.v->hydro_features;
}
cvector<CssInfraFeaturesClass> * CPlotClass::getInfraFeaturesRef() {
    return &cw->c_plot.v->infra_features;
}
cvector<CssWftagClass> * CPlotClass::getWftagRef() {
    return &cw->c_plot.v->wftags;
}
cvector<CssAmplitudeClass> * CPlotClass::getAmplitudeRef() {
    return &cw->c_plot.v->amplitudes;
}
cvector<CssAmpdescriptClass> * CPlotClass::getAmpdescriptRef() {
    return &cw->c_plot.v->ampdescripts;
}
cvector<CssParrivalClass> * CPlotClass::getParrivalRef() {
    return &cw->c_plot.v->parrivals;
}
cvector<CssFilterClass> * CPlotClass::getFilterRef() {
    return &cw->c_plot.v->filters;
}

gvector<CssTableClass *> * CPlotClass::getTableRef(const string &table_name)
{
    if(!table_name.compare(cssAmpdescript)) { return getAmpdescriptRef(); }
    else if(!table_name.compare(cssAmplitude)) { return getAmplitudeRef(); }
    else if(!table_name.compare(cssArrival)) { return getArrivalRef(); }
    else if(!table_name.compare(cssAssoc)) { return getAssocRef(); }
    else if(!table_name.compare(cssFilter)) { return getFilterRef(); }
    else if(!table_name.compare(cssHydroFeatures)) {
	return getHydroFeaturesRef();
    }
    else if(!table_name.compare(cssInfraFeatures)) {
   	return getInfraFeaturesRef();
    }
    else if(!table_name.compare(cssNetmag)) { return getNetmagRef(); }
    else if(!table_name.compare(cssOrigin)) { return getOriginRef(); }
    else if(!table_name.compare(cssOrigerr)) { return getOrigerrRef(); }
    else if(!table_name.compare(cssParrival)) { return getParrivalRef(); }
    else if(!table_name.compare(cssStamag)) { return getStamagRef(); }
    else if(!table_name.compare(cssStassoc)) { return getStassocRef(); }
    else if(!table_name.compare(cssWftag)) { return getWftagRef(); }

    fprintf(stderr, "CPlotClass::getTableRef: invalid name: %s\n",
                        table_name.c_str());
    return NULL;
}

/**
 *  Get all arrivals associated with a particular waveform.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd a Waveform object for a waveform in this CPlotClass.
 *  @param[out] arrivals An array of CssArrival objects. The space for this
 *		array is allocated by this function.
 *  @returns the number of CssArrival objects.
 */
int CPlotClass::getArrivalsOnWaveform(Waveform *cd,
			cvector<CssArrivalClass> &arrivals)
{
    CPlotPart *cp = &cw->c_plot;
    DataEntry *entry;
    int i, dc=0, id=0, copy=0;

    arrivals.clear();
    for(i = 0; i < cp->num_entries && cp->entry[i]->w != cd; i++);
    if(i == cp->num_entries) return 0;

    entry = cp->entry[i];
    if(cp->v->arrivals.size() <= 0) return 0;

    for(i = 0; i < cp->v->arrivals.size(); i++)
    {
	CssArrivalClass *a = cp->v->arrivals[i];
	if((!strcasecmp(entry->ts->sta(), a->sta) ||
	    !strcasecmp(entry->ts->net(), a->sta)) &&
		entry->tbeg <= a->time && entry->ts->tend() >= a->time)
	{
	    entry->ts->getValue("dc", &dc);
	    entry->ts->getValue("id", &id);
	    entry->ts->getValue("copy", &copy);

	    if(a->ts_dc > 0 && (dc != a->ts_dc || id != a->ts_id
			|| copy != a->ts_copy))
	    {
		continue;
	    }
	    arrivals.push_back(a);
	}
    }
    return arrivals.size();
}

/** 
 *  Align all waveforms with their first sample at time 0.
 *  @param[in] w A CPlotWidget pointer.
 *  
 */
void CPlotClass::alignFirstPt()
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    bool changed_one;
    CPlotWidget z;

    ax->use_screen = False;
    changed_one = False;
    for(int i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w->scaled_x0 != 0.) {
	    changed_one = true;
	    cp->entry[i]->w->scaled_x0 = 0.;
	}
    }
    if(changed_one) {
	zoomOut();
	if((z = (CPlotWidget)ax->mag_to) != NULL) {
	    CPlotMagnifyWidget(cw, NULL, True);
	    CPlotMagnifyWidget(cw, z, True);
	}
    }
    ax->use_screen = True;
}

/** 
 *  Set waveform colors. Specify the Pixel color of individual waveforms.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] num_waveforms The number of waveforms referenced in wvec.
 *  @param[in] wvec An array of Waveform pointers.
 *  @param[in] fg An array of num_waveforms Pixels.
 *  @param[in] redisplay If true, the data window is immediately redrawn.
 *		If False, the data is not redrawn until some other CPlot
 *		function causes a waveform redraw.
 */
void CPlotClass::colorWaveforms(gvector<Waveform *> &wvec, Pixel *fg,
			bool redisplay)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    int i, j;
    Pixel color;
    DataEntry	*entry;
    CPlotWidget	z;

    z = (CPlotWidget)ax->mag_to;

    ax->use_screen = False;

    for(i = 0; i < wvec.size(); i++)
    {
	color = (fg[i] > 0 && fg[i] != cw->core.background_pixel)
				? fg[i] : ax->fg;

	for(j = 0; j < cp->num_entries; j++) if(wvec[i] == cp->entry[j]->w)
	{
	    entry = cp->entry[j];
	    if(redisplay) {
		_CPlotRedisplayEntry(cw, entry);
		entry->w->fg = color;
		_CPlotRedisplayEntry(cw, entry);
	    }
	    else {
		entry->w->fg = color;
	    }
	    if(z != NULL)
	    {
		entry = z->c_plot.entry[j];
		if(redisplay) {
		    _CPlotRedisplayEntry(z, entry);
		    entry->w->fg = color;
		    _CPlotRedisplayEntry(z, entry);
		}
		else {
		    entry->w->fg = color;
		}
	    }
	}
    }
    ax->use_screen = True;
    if(ax->use_pixmap && XtWindow(cw)) {
	XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width, cw->core.height, 0, 0);
    }
}

/** 
 *  Change the color of selected waveforms. The data window will be
 *  immediately redrawn. The selected waveforms will remain selected, but
 *  their deselected color will be set to the input Pixel color.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] color The color to be assigned to all currently selected
 *	waveforms.
 */
void CPlotClass::changeColor(Pixel color)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    DataEntry *entry;
    CPlotWidget z;

    z = (CPlotWidget)ax->mag_to;

    for(int i = 0; i < cp->num_entries; i++) if(cp->entry[i]->w->selected)
    {
	entry = cp->entry[i];
	_CPlotRedisplayEntry(cw, entry);
	entry->w->fg = color;
	_CPlotRedisplayEntry(cw, entry);

	if(z != NULL)
	{
	    entry = z->c_plot.entry[i];
	    _CPlotRedisplayEntry(z, entry);
	    entry->w->fg = color;
	    _CPlotRedisplayEntry(z, entry);
	}
    }
}

/** 
 *  Align all waveforms on a phase-line-cursor.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] type The type of alignment:
 *	- CPLOT_SELECTED_WAVEFORM align all selected waveforms that contain the
 *			phase.
 *	- CPLOT_SELECTED_ARRIVAL align all waveforms that have the phase-line
 *		arrival selected.
 *	- CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM align all selected waveforms
 *		that have the phase-line arrival selected.
 *	If the resource XtNalignArrSelected is false, then this routine ignores
 *	the selected state of the waveforms and aligns all waveforms that
 *	contain the arrival (in the case of CPLOT_SELECTED_WAVEFORM) or contain
 *	the selected arrival (in the cases of CPLOT_SELECTED_ARRIVAL and
 *	CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM).
 * @returns the arrival phase name. Do not free it.
 */
const char * CPlotClass::alignOnArrival(int type)
{
	CPlotPart	*cp = &cw->c_plot;
	AxesPart	*ax = &cw->axes;
	int		i, j;
	double		x;
	char		*phase;
	DataEntry	*entry;
	Boolean		none_selected;
	CPlotWidget	z;
	CssArrivalClass	*a=NULL;

	cp->v->arrival_entries.sort(sort_arrivals_by_time);

	for(i = 0; i < ax->num_cursors; i++)
	{
	    if(ax->cursor[i].type == AXES_PHASE_LINE) break;
	}
	if(i == ax->num_cursors)
	{
	    _AxesWarning((AxesWidget)cw, "Need a phase line.");
	    return NULL;
	}
	phase = ax->cursor[i].label;
	x = (ax->x_axis != LEFT) ? ax->cursor[i].scaled_x :
					 ax->cursor[i].scaled_y;
	ax->use_screen = False;
	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    z->axes.use_screen = False;
	}

	if (type == CPLOT_SELECTED_WAVEFORM)
	{
	    none_selected = True;
	    for(i = 0; i < cp->num_entries; i++)
		if((cp->entry[i]->w->selected || !cp->align_arr_selected))
	    {
		none_selected = False;
		entry = cp->entry[i];
		for(j = 0; j < cp->v->arrival_entries.size(); j++)
		{
		    a = cp->v->arrival_entries[j]->a;

		    if((!strcasecmp(entry->ts->sta(), a->sta) ||
			!strcasecmp(entry->ts->net(), a->sta)) &&
			!strcmp(phase, arrivalPhase(a)) &&
			entry->ts->tbeg() <= a->time &&
			entry->ts->tend() >= a->time) break;
		}
		if(j == cp->v->arrival_entries.size()) continue;

		_CPlotRedisplayEntry(cw, entry);

		entry->w->scaled_x0 = x - (a->time - entry->ts->tbeg());

		_CPlotRedrawEntry(cw, entry);
		_CPlotRedisplayEntry(cw, entry);
		if((z = (CPlotWidget)ax->mag_to) != NULL)
		{
		    DataEntry *ze = z->c_plot.entry[i];
		    _CPlotRedisplayEntry(z, ze);
		    ze->w->scaled_x0 = entry->w->scaled_x0;
		    _CPlotRedrawEntry(z, ze);
		    _CPlotRedisplayEntry(z, ze);
		}
	    }
	    if(cp->align_arr_selected && none_selected && cp->num_entries > 0)
	    {
		_AxesWarning((AxesWidget)cw, "No waveforms selected.");
		return NULL;
	    }
	}
	else if (type == CPLOT_SELECTED_ARRIVAL)
	{
            none_selected = True;
 
            for(j = 0; j < cp->v->arrival_entries.size(); j++)
            {
		ArrivalEntry *a_entry = cp->v->arrival_entries[j];
		if(!a_entry->selected ) continue;
		none_selected = False;
 
		a = a_entry->a;
		for(i = 0; i < cp->num_entries; i++)
		{
		    entry = cp->entry[i];
 
		    if((strcasecmp(entry->ts->sta(), a->sta) &&
			strcasecmp(entry->ts->net(), a->sta)) ||
			strcmp(phase, arrivalPhase(a)) ||
			entry->ts->tbeg() >= a->time ||
			entry->ts->tend() <= a->time) continue;
 
		    _CPlotRedisplayEntry(cw, entry);
 
		    entry->w->scaled_x0 = x - (a->time - entry->ts->tbeg());
 
		    _CPlotRedrawEntry(cw, entry);
		    _CPlotRedisplayEntry(cw, entry);
		    if((z = (CPlotWidget)ax->mag_to) != NULL)
		    {
                        DataEntry *ze = z->c_plot.entry[i];
                        _CPlotRedisplayEntry(z, ze);
                        ze->w->scaled_x0 = entry->w->scaled_x0;
                        _CPlotRedrawEntry(z, ze);
                        _CPlotRedisplayEntry(z, ze);
		    }
		}
	    }
 
	    if(cp->align_arr_selected && none_selected && cp->num_entries > 0)
	    {
		_AxesWarning((AxesWidget)cw, "No arrivals selected.");
                return NULL;
	    }
	}
	else if (type == CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM)
	{
	    none_selected = True;
	    for(i = 0; i < cp->num_entries; i++)
		if((cp->entry[i]->w->selected || !cp->align_arr_selected))
	    {
		none_selected = False;
		entry = cp->entry[i];
		for(j = 0; j < cp->v->arrival_entries.size(); j++)
                {
		    ArrivalEntry *a_entry = cp->v->arrival_entries[j];
		    if(!a_entry->selected) continue;
 
		    a = a_entry->a;
		    if((!strcasecmp(entry->ts->sta(), a->sta) ||
			!strcasecmp(entry->ts->net(), a->sta)) &&
			!strcmp(phase, arrivalPhase(a)) &&
			entry->ts->tbeg() <= a->time &&
			entry->ts->tend() >= a->time) break;
		}
		if(j == cp->v->arrival_entries.size()) continue;
 
		_CPlotRedisplayEntry(cw, entry);
 
       		entry->w->scaled_x0 = x - (a->time - entry->ts->tbeg());
 
		_CPlotRedrawEntry(cw, entry);
		_CPlotRedisplayEntry(cw, entry);
		if((z = (CPlotWidget)ax->mag_to) != NULL)
		{
                        DataEntry *ze = z->c_plot.entry[i];
                        _CPlotRedisplayEntry(z, ze);
                        ze->w->scaled_x0 = entry->w->scaled_x0;
                        _CPlotRedrawEntry(z, ze);
                        _CPlotRedisplayEntry(z, ze);
		}
	    }
	    if(cp->align_arr_selected && none_selected && cp->num_entries > 0)
	    {
		_AxesWarning((AxesWidget)cw, "No waveforms selected.");
		return NULL;
	    }
	}

	if(ax->use_pixmap && XtWindow(cw))
	{
	    XCopyArea(XtDisplay(cw), ax->pixmap, XtWindow(cw), ax->axesGC, 0, 0,
			cw->core.width, cw->core.height, 0, 0);
	}
	ax->use_screen = True;
	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    if(ax->use_pixmap) {
		XCopyArea(XtDisplay(z), z->axes.pixmap, XtWindow(z),
    		    z->axes.axesGC, 0, 0, z->core.width, z->core.height, 0, 0);
	    }
	    z->axes.use_screen = True;
	    AdjustHorizontalScrollbars(z);
	}
	AdjustHorizontalScrollbars(cw);
	return phase;
}

/** 
 *  Align all waveforms on an arrival. All waveforms that are associated with
 *  the indicated arrival will be aligned on that arrival. All other waveforms
 *  are not repositioned.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] phase The phase name of the arrival.
 *  
 */
void CPlotClass::alignOnPhase(const string &phase)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	int i, j, num;
	char c1='\0', c2='\0';
	vector<double> scaled_x0;
	CssArrivalClass *a;
	gvector<Waveform *> wvec;

	if(cp->num_entries <= 0) return;

	cp->v->arrival_entries.sort(sort_arrivals_by_time);

	if(!phase.compare("FirstP")) {
	    c1 = 'P'; c2 = 'p';
	}
	else if(!phase.compare("FirstS")) {
	    c1 = 'S'; c2 = 's';
	}

	for(i = num = 0; i < cp->num_entries; i++)
	{
	    DataEntry *entry = cp->entry[i];
	    double tbeg = entry->ts->tbeg();
	    double tend = entry->ts->tend();


	    wvec.push_back(cp->entry[i]->w);
	    scaled_x0.push_back(0.);

	    if(!phase.compare("FirstP") || !phase.compare("FirstS"))
	    {
		for(j = 0; j < cp->v->arrival_entries.size(); j++)
		{
		    char *p;
		    a = cp->v->arrival_entries[j]->a;
		    p = arrivalPhase(a);

		    if((!strcasecmp(entry->ts->sta(), a->sta) ||
			!strcasecmp(entry->ts->net(), a->sta)) &&
			tbeg <= a->time && tend >= a->time &&
			(p[0] == c1 || p[0] == c2))
		    {
			scaled_x0[num] = tbeg - a->time;
			break;
		    }
		}
	    }
	    else
	    {
		for(j = 0; j < cp->v->arrival_entries.size(); j++)
		{
		    a = cp->v->arrival_entries[j]->a;

		    if((!strcasecmp(entry->ts->sta(), a->sta) ||
			!strcasecmp(entry->ts->net(), a->sta)) &&
			!phase.compare(arrivalPhase(a)) &&
			tbeg <= a->time && tend >= a->time)
		    {
			scaled_x0[num] = tbeg - a->time;
			break;
		    }
		}
	    }
	    num++;
	}
	positionX(wvec, scaled_x0);
}

/** 
 *  Remove a waveform amplitude measurement box. Only the measurement box
 *  displayed on the specified waveform is removed. Measurement boxes on other
 *  waveforms are not removed.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd The Waveform pointer associated with the waveform.
 *  
 */
void CPlotClass::removeMeasureBox(Waveform *cd)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    int		i;
    CPlotWidget	z;

    for(i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w == cd) break;
    }
    if(i == cp->num_entries) return;

    DrawMeasureBoxes(cw, cp->entry[i]);

    cp->entry[i]->n_measure_segs = 0;

    if((z = (CPlotWidget)ax->mag_to) == NULL) {
	z = (CPlotWidget)ax->mag_from;
    }
    if(z != NULL) {
	z->c_plot.entry[i]->n_measure_segs = 0;
    }
}

/** 
 *  Remove all amplitude measure boxes.
 *  @param[in] w A CPlotWidget pointer.
 *  
 */
void CPlotRemoveAllMeasureBoxes(CPlotWidget w)
{
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    CPlotWidget	z;

    if((z = (CPlotWidget)ax->mag_to) == NULL) {
	z = (CPlotWidget)ax->mag_from;
    }
    for(int i = 0; i < cp->num_entries; i++)
    {
	if( cp->entry[i]->n_measure_segs > 0 &&
	    cp->entry[i]->on && cp->entry[i]->w->visible)
	{
	    DrawMeasureBoxes(w, cp->entry[i]);
	}
	cp->entry[i]->n_measure_segs = 0;
	if(z != NULL) {
	    z->c_plot.entry[i]->n_measure_segs = 0;
	}
    }
}

static void
fminmax(int n, float *a, double *min, double *max, double mean)
{
	if(n <= 0) return;
	while(n > 0 && (!finite((*a) || fNaN(*a)) ))
	{
	    a++;
	    n--;
	}
	if(n <= 0) return;

	*min = *max = *a;

	while(n-- > 0)
	{
	    if(finite((*a) && !fNaN(*a)))
	    {
		if(*min > *a) *min = *a;
		if(*max < *a) *max = *a;
		a++;
	    }
	}
	*min -= mean;
	*max -= mean;
}

static void
dminmax(int n, double *a, double *min, double *max, double mean)
{
	if(n <= 0) return;
	while(n > 0 && (!finite((*a) || fNaN(*a)) ))
	{
	    a++;
	    n--;
	}
	if(n <= 0) return;

	*min = *max = *a;

	while(n-- > 0)
	{
	    if(finite((*a) && !fNaN(*a)))
	    {
		if(*min > *a) *min = *a;
		if(*max < *a) *max = *a;
		a++;
	    }
	}
	*min -= mean;
	*max -= mean;
}

/** 
 *  Get the visiblity of the waveforms.
 *  @param[in] w A CPlotWidget pointer.
 *  @returns True if there are any visible waveforms, otherwise return False.
 */
bool CPlotClass::dataVisible()
{
    CPlotPart *cp = &cw->c_plot;

    for(int i = 0; i < cp->num_entries; i++) if(cp->entry[i]->on) {
	if(visibleMinMax(cw, cp->entry[i])) return true;
    }
    return false;
}

/** 
 *  Scroll a waveform to the bottom. Position the vertical axis so that the
 *  indicated waveform is visible at the bottom of the data window.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd The Waveform pointer associated with the waveform.
 */
void CPlotClass::scrollBottom(Waveform *cd)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	DataEntry *e;
	int i, max;
	double ymax, ydif;
	/* scroll so that cd is at the bottom of the window
	 */
	for(i = 0; i < cp->num_entries; i++) {
	    if(cp->entry[i]->on && cd == cp->entry[i]->w) break;
	}
	if(i == cp->num_entries) return; /* not on or not found */

	e = cp->entry[i];
	if(!e->w->visible || (e->unsel.nsegs == 0 &&
		e->sel.nsegs == 0))
	{
	   e->w->visible = True;
	   _CPlotRedrawEntry(w, e);
	}

	max = 0;
	for(i = 0; i < e->unsel.nsegs; i++) {
	    if(e->unsel.segs[i].y1 > max) max = e->unsel.segs[i].y1;
	    if(e->unsel.segs[i].y2 > max) max = e->unsel.segs[i].y2;
	}
	for(i = 0; i < e->sel.nsegs; i++) {
	    if(e->sel.segs[i].y1 > max) max = e->sel.segs[i].y1;
	    if(e->sel.segs[i].y2 > max) max = e->sel.segs[i].y2;
	}
	ymax = scale_y(&ax->d, max);
	if(ymax < ax->y2[ax->zoom]) return;

	ydif = ymax - ax->y2[ax->zoom];

	ax->y1[ax->zoom] += ydif;
	ax->y2[ax->zoom] += ydif;

	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);
}
	
static Boolean
visibleMinMax(CPlotWidget w, DataEntry *entry)
{
	AxesPart *ax = &w->axes;
	int       i, j, n;
	double    x_start, x_end, min, max;
	GDataPoint *d1 = NULL, *d2 = NULL;
	double buffer_scaled_xmin = ax->x1[ax->zoom];
	double buffer_scaled_xmax = ax->x2[ax->zoom];

	entry->visible_ymin = entry->ymax;
	entry->visible_ymax = entry->ymin;

	x_start = buffer_scaled_xmin
		    - entry->w->scaled_x0 + entry->use_ts->tbeg();
	x_end   = buffer_scaled_xmax
		    - entry->w->scaled_x0 + entry->use_ts->tbeg();

	if( (d1 = entry->use_ts->lowerBound(x_start)) != NULL &&
	    (d2 = entry->use_ts->upperBound(x_end)) != NULL)
	{
	    GSegment *s = entry->use_ts->segment(d1->segmentIndex());

	    n = (d1->segmentIndex() != d2->segmentIndex()) ?
			s->length() : d2->index()+1;

	    min = entry->visible_ymin + entry->mean;
	    max = entry->visible_ymax + entry->mean;
	    for(i = d1->index(); i < n; i++) {
		float d = s->data[i];
		if(min > d) min = d;
		else if(max < d) max = d;
	    }

	    for(j = d1->segmentIndex()+1; j < d2->segmentIndex(); j++) {
		s = entry->use_ts->segment(j);
		n = s->length();
		float *d = s->data;
		for(i = 0; i < n; i++, d++) {
		    if(min > *d) min = *d;
		    else if(max < *d) max = *d;
		}
	    }
	    if(d2->segmentIndex() > d1->segmentIndex()) {
		s = entry->use_ts->segment(d2->segmentIndex());
		n = d2->index();
		float *d = s->data;
		for(i = 0; i <= n; i++, d++) {
		    if(min > *d) min = *d;
		    else if(max < *d) max = *d;
		}
	    }
	    if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
	    entry->visible_ymin = min - entry->mean;
	    entry->visible_ymax = max - entry->mean;
	    return True;
	}
	else
	{
	    if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
	    entry->visible_ymin = entry->ymin;
	    entry->visible_ymax = entry->ymax;
	    return False;
	}
}

/** 
 *  
 */
void
_CPlotRedraw(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;
	double y_scale_change;

	if(!XtIsRealized((Widget)w)) return;

	if(cp->need_sort) SortEntries(w);

	XSetClipRectangles(XtDisplay(w), ax->selectGC, 0, 0,
		&ax->clip_rect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(w), cp->labelGC, 0, 0,
		&ax->clip_rect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(w), cp->dataGC,
		0, 0, &ax->clip_rect, 1, Unsorted);

	for(i = 0; i < cp->num_curves; i++) {
	    if(cp->curve[i]->on) _CPlotRedrawCurve(w, cp->curve[i]); 
	}
	/*
	 * Check if each entry is visible.
	 */
	Visible(w);

	y_scale_change = (ax->mag_from == NULL) ? 1. :
			(double)((int)ax->clip_rect.height+2)/
			(double)((int)ax->old_clip.height+2);

	for(i = 0; i < cp->num_entries; i++) {
	    cp->entry[i]->yscale *= y_scale_change;
	    _CPlotRedrawEntry(w, cp->entry[i]);
	}
	if(cp->arrival_i >= 0) {
	    DrawArrivalRec(w, ax->mag_invertGC);
	    destroyInfoPopup(w);
	    cp->arrival_i = -1;
	    cp->arrival_j = -1;
	    cp->arrival_moving = False;
	}
	if(cp->point_x >= 0) {
	    DrawPointRec(w, ax->mag_invertGC);
	    cp->point_x = -1;
	}
	printLeftLimit((AxesWidget)w);
}

/** 
 *  
 */
void
_CPlotRedrawEntry(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int	i, i1, i2, j, begSel, endSel;
	int	entry_x0, entry_y0;
	float	f;
	double	xscale, x, xdif, yscale, ydif, r1, r2, tmin, tmax;
	double	x1, y1, x2, y2;
	Boolean	shift_only, partial_select;
	int	clipx1, clipx2;
	GTimeSeries *use_ts;
	DrawStruct *d = &ax->d;

	if(entry == NULL) return;

	entry->sel.nsegs = 0;
	entry->unsel.nsegs = 0;
	entry->sel.size_segs = 0;
	entry->unsel.size_segs = 0;
	Free(entry->sel.segs);
	Free(entry->unsel.segs);
	if(!entry->w->selected) {
	    entry->w->begSelect = 0.;
	    entry->w->endSelect = 0.;
	}

	if(entry->ts->length() <=0 || !entry->on || !entry->w->visible) return;

	if(ax->x_axis != LEFT) {
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	    xscale = ax->d.unscalex;
	}
	else {
	    x1 = ax->y1[ax->zoom];
	    x2 = ax->y2[ax->zoom];
	    y1 = ax->x1[ax->zoom];
	    y2 = ax->x2[ax->zoom];
	    xscale = ax->d.unscaley;
	}
	xscale = d->unscalex;

	ydif = (cp->scale_independently) ?
		entry->ymax - entry->ymin : cp->uniform_scale;
	if(ydif != 0.) {
	    yscale = (cp->dataHeight/ydif)*(ax->y2[1] - ax->y1[1])/(y2 - y1);
	}
	else yscale = 0.;
	yscale *= entry->drag_yscale;

	shift_only = False;

	if(!entry->modified)
	{
	    xdif = fabs((entry->xscale - xscale)/entry->xscale);
	    ydif = fabs((entry->yscale - yscale)/entry->yscale);

	    if(xdif < .001 && ydif < .001 &&
		ax->clipy2 - ax->clipy1 == entry->scale_height &&
		ax->clipx2 - ax->clipx1 == entry->scale_width)
	    {
	        GSegment *last_seg =
			entry->use_ts->segment(entry->use_ts->size()-1);
		double buffer_scaled_xmin = ax->x1[ax->zoom];
		double buffer_scaled_xmax = ax->x2[ax->zoom];
		double tbeg = entry->use_ts->tbeg();

                if(buffer_scaled_xmin < buffer_scaled_xmax) {
                    tmin = buffer_scaled_xmin - entry->w->scaled_x0 + tbeg;
                    tmax = buffer_scaled_xmax - entry->w->scaled_x0 + tbeg;
                }
                else {
                    tmin = buffer_scaled_xmax - entry->w->scaled_x0 + tbeg;
                    tmax = buffer_scaled_xmin - entry->w->scaled_x0 + tbeg;
                }

		/* if the entry hasn't been modified and the scale hasn't
		 * changed, and the complete waveform was drawn last time,
		 * then shift_only is true.
		 */
                if((entry->beg->time() <= tmin || entry->beg->index() ==0)
			&& (entry->end->time() >= tmax ||
			    (entry->end->segment() == last_seg &&
			    entry->end->index() == last_seg->length()-1)) )
                {
                    shift_only = True;
                }
	    }
	}

	use_ts = entry->ts;
	x = entry->length/(x2 - x1);
	for(i = 0; i < entry->num_deci; i++) {
	    if(entry->p[i].ts->length() > 0 && x < entry->p[i].span) {
		use_ts = entry->p[i].ts;
		break;
	    }
	}
	if(use_ts != entry->use_ts) {
	    entry->use_ts = use_ts;
	    shift_only = False;
	}

	if(!shift_only)
	{
	    x = entry->range*(x2-x1);

	    r1 = x1 - x;
	    r2 = x2 + x;

	    tmin = r1 - entry->w->scaled_x0 + entry->use_ts->tbeg();
	    if(entry->beg) entry->beg->deleteObject();
	    if((entry->beg = entry->use_ts->lowerBound(tmin)) == NULL)
		return;

	    tmax = r2 - entry->w->scaled_x0 + entry->use_ts->tbeg();
	    if(entry->end) entry->end->deleteObject();
	    if((entry->end = entry->use_ts->upperBound(tmax)) == NULL)
		return;

	    if(!plot_ts(entry, xscale, yscale))
		return;

	    entry->modified = False;
	    entry->r.start = 0;
	    entry->r.nshow = entry->r.nsegs;
	    entry->xscale = xscale;
	    entry->yscale = yscale;
	    entry->scale_width = ax->clipx2 - ax->clipx1;
	    entry->scale_height = ax->clipy2 - ax->clipy1;
	    visibleMinMax(w, entry);

	    FindArrivals(w, entry);
	    FindPredArrivals(w, entry);
	}
	else {
	    if(entry->need_find) FindArrivals(w, entry);
	    if(entry->need_pred_find) FindPredArrivals(w, entry);
	}

	if(ax->x_axis != LEFT)
	{
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    clipx1 = ax->clipx1;
	    clipx2 = ax->clipx2;
	}
	else
	{
	    entry_x0 = unscale_y(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    entry_y0 = unscale_x(&ax->d, entry->w->scaled_y0);
	    clipx1 = ax->clipy1;
	    clipx2 = ax->clipy2;
	    f = entry->r.xmin;
	    entry->r.xmin = entry->r.ymin;
	    entry->r.ymin = f;
	    f = entry->r.xmax;
	    entry->r.xmax = entry->r.ymax;
	    entry->r.ymax = f;
	}
	if(entry->tag_width > 0)
	{
	    if(ax->x_axis != LEFT) {
		entry->tag_x = entry_x0 - entry->tag_width - 4;
		entry->tag_y = entry_y0 - entry->tag_height/2;
	    }
	    else {
		entry->tag_x = entry_y0 - entry->tag_width/2;
		entry->tag_y = entry_x0 - entry->tag_height-4;
	    }
	}

	i1 = entry->r.start;
	while(i1 > 0 && entry_x0 + entry->r.segs[i1].x1 > clipx1) i1--;
	while(i1 < entry->r.nsegs-1 && entry_x0 +
		entry->r.segs[i1].x2 < clipx1) i1++;

	i2 = entry->r.start + entry->r.nshow - 1;
	while(i2 > 0 && entry_x0 + entry->r.segs[i2].x1 > clipx2) i2--;
	while(i2 < entry->r.nsegs-1 && entry_x0 +
		entry->r.segs[i2].x2 < clipx2) i2++;
	entry->r.start = i1;
	entry->r.nshow = i2 - i1 + 1;

	begSel = endSel = -1;
	partial_select = selectLimits(w, entry, &begSel, &endSel);

	SetClipRects(w, entry);

	if(entry_x0+entry->r.xmin< -S_LIM || entry_x0+entry->r.xmax > S_LIM ||
	   entry_y0+entry->r.ymin< -S_LIM || entry_y0+entry->r.ymax > S_LIM)
	{
	    SegArray *s = (entry->w->selected) ? &entry->sel : &entry->unsel;
	    /* do the clipping here. X can't handle it. */
	    DoClipping(w, entry->r.nshow, entry->r.segs+i1,entry_x0,entry_y0,s);
	}
	else if(partial_select)
	{
	    int k, l;
	    entry->sel.nsegs = 0;
	    entry->unsel.nsegs = 0;
	    for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
	    {
		if(entry->r.segs[j].x1 >=begSel && entry->r.segs[j].x2<=endSel){
		    entry->sel.nsegs++;
		}
		else {
		    entry->unsel.nsegs++;
		}
	    }
	    if(entry->sel.nsegs > 0) {
		ALLOC(entry->sel.nsegs*sizeof(DSegment), entry->sel.size_segs,
		    entry->sel.segs, DSegment, 0);
	    }
	    if(entry->unsel.nsegs > 0) {
		ALLOC(entry->unsel.nsegs*sizeof(DSegment),
		    entry->unsel.size_segs, entry->unsel.segs, DSegment, 0);
	    }
	    for(i = k = l = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
	    {
		if(entry->r.segs[j].x1 >=begSel && entry->r.segs[j].x2<=endSel)
		{
		    entry->sel.segs[k].x1 =
			(short)(entry_x0 + entry->r.segs[j].x1 + .5);
		    entry->sel.segs[k].y1 =
			(short)(entry_y0 + entry->r.segs[j].y1 + .5);
		    entry->sel.segs[k].x2 =
			(short)(entry_x0 + entry->r.segs[j].x2 + .5);
		    entry->sel.segs[k].y2 =
			(short)(entry_y0 + entry->r.segs[j].y2 + .5);
		    k++;
		}
		else {
		    entry->unsel.segs[l].x1 =
			(short)(entry_x0 + entry->r.segs[j].x1+.5);
		    entry->unsel.segs[l].y1 =
			(short)(entry_y0 + entry->r.segs[j].y1+.5);
		    entry->unsel.segs[l].x2 =
			(short)(entry_x0 + entry->r.segs[j].x2+.5);
		    entry->unsel.segs[l].y2 =
			(short)(entry_y0 + entry->r.segs[j].y2+.5);
		    l++;
		}
	    }
	}
	else {
	    SegArray *s = (entry->w->selected) ? &entry->sel : &entry->unsel;
	    s->nsegs = entry->r.nshow;
	    ALLOC(s->nsegs*sizeof(DSegment), s->size_segs, s->segs, DSegment,0);
	    for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
	    {
		s->segs[i].x1 = (short)(entry_x0 + entry->r.segs[j].x1 + .5);
		s->segs[i].y1 = (short)(entry_y0 + entry->r.segs[j].y1 + .5);
		s->segs[i].x2 = (short)(entry_x0 + entry->r.segs[j].x2 + .5);
		s->segs[i].y2 = (short)(entry_y0 + entry->r.segs[j].y2 + .5);
	    }
        }
}

void CPlotClass::zoomOnWaveforms(gvector<Waveform *> &wvec)
{
	CPlotWidget w =cw;
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	DataEntry *entry;
	CPlotWidget z;
	double x1, x2, y1, y2, xmin=0., xmax=0., ymin=0., ymax=0., ydif, yscale;
	double cp_tmin, cp_tmax;
	int i, j, entry_y0, iy1, iy2, cp_max_tag_width;
	bool first = true;

	for(j = 0; j < wvec.size(); j++)
	{
	    for(i = 0; i < cp->num_entries; i++) {
		if(cp->entry[i]->w == wvec[j]) break;
	    }
	    if(i == cp->num_entries) continue;

	    entry = cp->entry[i];

	    x1 = entry->w->scaled_x0;
	    x2 = x1 + entry->w->ts->duration();

	    ydif = (cp->scale_independently) ?
			entry->ymax - entry->ymin : cp->uniform_scale;
	    if(ydif != 0.) {
		yscale = (cp->dataHeight/ydif)*(ax->y2[1] - ax->y1[1])/
				(ax->y2[ax->zoom] - ax->y1[ax->zoom]);
	    }
	    else yscale = 0.;
	    yscale *= entry->drag_yscale;

	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);

	    iy1 = entry_y0 - (int)((entry->w->ts->dataMax()
				- entry->mean)*yscale + .5);
	    iy2 = entry_y0 + (int)((entry->mean
				- entry->w->ts->dataMin())*yscale + .5);

	    y1 = scale_y(&ax->d, iy1);
	    y2 = scale_y(&ax->d, iy2);
	    if(ax->zoom == 1) ax->zoom++;

	    cp_tmin = cp->tmin;
	    cp_tmax = cp->tmax;
	    cp->tmin = x1;
	    cp->tmax = x2;
	    cp_max_tag_width = cp->max_tag_width;
	    cp->max_tag_width = entry->tag_width;

	    _CPlotGetXLimits(w, &x1, &x2);

	    cp->tmin = cp_tmin;
	    cp->tmax = cp_tmax;
	    cp->max_tag_width = cp_max_tag_width;

	    if( first ) {
		first = false;
		xmin = x1;
	        xmax = x2;
	        ymin = y1;
	        ymax = y2;
	    }
	    else {
		if(x1 < xmin) xmin = x1;
		if(x2 > xmax) xmax = x2;
		if(y1 < ymin) ymin = y1;
		if(y2 > ymax) ymax = y2;
	    }
	}

	if(xmax > xmin)
	{
	    ax->x1[ax->zoom] = xmin;
	    ax->x2[ax->zoom] = xmax;
	    ax->y1[ax->zoom] = ymin - .1*(ymax - ymin);
	    ax->y2[ax->zoom] = ymax + .1*(ymax - ymin);

	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw(w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _CPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);

	    if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	    {
		CPlotMagnifyWidget(w, NULL, True);
		CPlotMagnifyWidget(w, z, True);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
}

static void
drawSelectLines(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int begSel, endSel;

	if(selectLimits(w, entry, &begSel, &endSel))
	{
	    DSegment seg;
	    int i, j, n;
	    int entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    int entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    int min_x = unscale_x(&ax->d, ax->x1[ax->zoom]);

	    XSetPlaneMask(XtDisplay(w), cp->dataGC,
			    ax->select_fg ^ w->core.background_pixel);
	    XSetForeground(XtDisplay(w), cp->dataGC,  ax->select_fg);
	    i = cp->dataHeight/2;
	    j = entry_x0 + begSel;
	    n = cp->display_tags ? entry->tag_width : 0;
	    if(j > min_x+n+2) {
		seg.x1 = j; seg.y1 = entry_y0-i;
		seg.x2 = j; seg.y2 = entry_y0+i;
		_AxesDrawSegments2((AxesWidget)w, cp->dataGC, &seg, 1);
	    }
	    j = entry_x0 + endSel;
	    if(j > min_x+n+2) {
		seg.x1 = j; seg.y1 = entry_y0-i;
		seg.x2 = j; seg.y2 = entry_y0+i;
		_AxesDrawSegments2((AxesWidget)w, cp->dataGC, &seg, 1);
	    }
	}
}

static Boolean
selectLimits(CPlotWidget w, DataEntry *entry, int *begSel, int *endSel)
{
	AxesPart *ax = &w->axes;
	double tbeg, tend;

	if(!entry->w->selected) return False;

	tbeg = entry->tbeg;
	tend = entry->tend;

	if(entry->w->begSelect != entry->w->endSelect &&
	    ((entry->w->begSelect > tbeg && entry->w->begSelect < tend) ||
	     (entry->w->endSelect > tbeg && entry->w->endSelect < tend)))
	{
	    int entry_x0 = unscale_x(&ax->d, entry->w->scaled_x0);
	    double x = entry->w->begSelect - entry->beg->time();
	    if(x < 0.) x = 0.;
	    x += entry->w->scaled_x0;
	    *begSel = unscale_x(&ax->d, x) - entry_x0;

	    x = (entry->w->endSelect < tend) ?  entry->w->endSelect : tend;
	    x += entry->w->scaled_x0 - entry->beg->time();
	    *endSel = unscale_x(&ax->d, x) - entry_x0;
	    return True;
	}
	entry->w->begSelect = 0.;
	entry->w->endSelect = 0.;
	return False;
}

static Boolean
plot_ts(DataEntry *entry, double xscale, double yscale)
{
	entry->r.nsegs = 0;
	entry->r.start = 0;
	entry->r.nshow = 0;

	if(entry->beg->timeSeries() != entry->end->timeSeries()) return False;

	drawTimeSeries(entry->beg->timeSeries(), entry->beg, entry->end,
		entry->mean, xscale, yscale, &entry->r);
	return True;
}

/** Change a curve.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] curve_index the index of the curve.
 *  @param[in] npts the number of values.
 *  @param[in] x the new x values.
 *  @param[in] y the new y values.
 *  @returns 1 for success.
 */
bool CPlotClass::changeCurve(int curve_index, int npts, double *x, float *y)
{
    CPlotPart *cp = &cw->c_plot;
    DataCurve *c;

    if(curve_index < 0 || curve_index >= cp->num_curves || npts <= 0
	    || !x || !y) return false;

    c = cp->curve[curve_index];

    if(npts != c->npts) {
	Free(c->x);
	Free(c->y);
	if( !(c->x = (double *)MallocIt(cw,npts*sizeof(double))) ) return false;
	if( !(c->y = (float *)MallocIt(cw, npts*sizeof(float))) ) return false;
	c->npts = npts;
    }
    memcpy(c->x, x, npts*sizeof(double));
    memcpy(c->y, y, npts*sizeof(float));
    dminmax(npts, x, &c->xmin, &c->xmax, 0.);
    fminmax(npts, y, &c->ymin, &c->ymax, 0.);

    if(c->on) {
	_AxesRedisplayAxes((AxesWidget)cw);
	_CPlotRedrawCurve(cw, c);
	_CPlotRedisplay(cw);
	_AxesRedisplayXor((AxesWidget)cw);
    }
    return true;
}

/** 
 *  
 */
void
_CPlotRedrawCurve(CPlotWidget w, DataCurve *curve)
{
	AxesPart *ax = &w->axes;
	double x1 = 0., y1 = 0., x2 = 0., y2 = 0.;
	
	if(curve == NULL ) return;

	curve->s.nsegs = 0;
	if(curve->npts <= 0)
	{
		return;
	}
	if(ax->x_axis != LEFT)
	{
		x1 = ax->x1[ax->zoom];
		x2 = ax->x2[ax->zoom];
		y1 = ax->y1[ax->zoom];
		y2 = ax->y2[ax->zoom];
	}
	else
	{
		x1 = ax->y1[ax->zoom];
		x2 = ax->y2[ax->zoom];
		y1 = ax->x1[ax->zoom];
		y2 = ax->x2[ax->zoom];
	}
	ax->d.s = &curve->s;

	SetClipArea(&ax->d, x1, y1, x2, y2);

	if(curve->type == CPLOT_CURVE || curve->type == CPLOT_TTCURVE)
	{
	    int last_nsegs = ax->d.s->nsegs;
	    plotxyd(&ax->d, curve->npts, curve->x, curve->y, x1, x2, y1, y2);
	    // check for a straight line and make sure it is straight. Rounding
	    // to the nearest int in plotxyd can make a line jump one pixel,
	    // when it the difference in y value is much less than a pixel
	    // height.
	    if(fabs((curve->ymax - curve->ymin)*ax->d.unscaley) < 1) {
		for(int i = last_nsegs; i < ax->d.s->nsegs; i++) {
		    ax->d.s->segs[i].y1 = ax->d.s->segs[last_nsegs+1].y1;
		    ax->d.s->segs[i].y2 = ax->d.s->segs[last_nsegs+1].y2;
		}
	    }
	}
	else
	{
		plotsymd(&ax->d, curve->npts, curve->x, curve->y,
			curve->type, 8, x1, x2, y1, y2);
	}
	if(curve->s.nsegs > 0)
	{
		DrawLabel(w, curve);
		mapalf(&ax->d, curve->lab_x, curve->lab_y, alpha_size,
			curve->lab_angle, 0, (char *)curve->lab.c_str());
	}
	curve->s.start = 0;
	curve->s.nshow = curve->s.nsegs;
}

/** 
 *  
 */
Boolean
_CPlotVisible(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	double ydif, yscale, scaled_ymin, scaled_ymax;
	double buffer_scaled_xmin = ax->x1[ax->zoom];
	double buffer_scaled_xmax = ax->x2[ax->zoom];
	double buffer_scaled_ymin = ax->y1[ax->zoom];
	double buffer_scaled_ymax = ax->y2[ax->zoom];

	ydif = entry->ymax - entry->ymin;
	if(ydif != 0.) {
	    double scaled_data_height = cp->dataSpacing*(double)cp->dataHeight/
			(double)(cp->dataHeight+cp->dataSeparation);
	    yscale = (scaled_data_height/ydif);
	}
	else yscale = 0.;
	yscale *= entry->drag_yscale;

	scaled_ymin = entry->w->scaled_y0 - entry->ymin*yscale;
	scaled_ymax = entry->w->scaled_y0 - entry->ymax*yscale;

	if(!overlaps(entry->w->scaled_x0, entry->w->scaled_x0+entry->length,
		    buffer_scaled_xmin, buffer_scaled_xmax) ||
	    !overlaps(scaled_ymin, scaled_ymax, buffer_scaled_ymin,
		    buffer_scaled_ymax))
	{
	    entry->w->visible = False;
	}
	else {
	    entry->w->visible = True; /* not necessarily visible */
	}
	return(entry->w->visible);
}

static Boolean
overlaps(double a1, double a2, double b1, double b2)
{
	if(b2 > b1) {
	    if(a2 > a1) {
		return((a1 >= b2 || a2 <= b1) ? False : True);
	    }
	    else {
		return((a2 >= b2 || a1 <= b1) ? False : True);
	    }
	}
	else {
	    if(a2 > a1) {
		return((a1 >= b1 || a2 <= b2) ? False : True);
	    }
	    else {
		return((a2 >= b1 || a1 <= b2) ? False : True);
	    }
	}
}



static void
Visible(CPlotWidget w)
{
	int i;
	CPlotPart *cp = &w->c_plot;
	for(i= 0; i < cp->num_entries; i++) _CPlotVisible(w, cp->entry[i]);
}

static void
DoClipping(CPlotWidget w, int nsegs, RSeg *segs, float x0, float y0,SegArray *m)
{
	AxesPart *ax = &w->axes;
	int i;
	DrawStruct d;

	if(nsegs <= 0) return;

        SetDrawArea(&d, ax->clipx1, ax->clipy1, ax->clipx2, ax->clipy2);

        SetScale(&d, (double)ax->clipx1, (double)ax->clipy1,
                     (double)ax->clipx2, (double)ax->clipy2);

	d.s = m;
	d.s->nsegs = 0;

	if(ax->x_axis != LEFT)
	{
	    for(i = 0; i < nsegs; i++) {
		imove(&d, (double)(x0 + segs[i].x1), (double)(y0 + segs[i].y1));
		idraw(&d, (double)(x0 + segs[i].x2), (double)(y0 + segs[i].y2));
	    }
	}
	else
	{
	    for(i = 0; i < nsegs; i++) {
		imove(&d, (double)(y0 - segs[i].y1), (double)(x0 + segs[i].x1));
		idraw(&d, (double)(y0 - segs[i].y2), (double)(x0 + segs[i].x2));
	    }
	}
	iflush(&d);
}

/** 
 *  Set the waveform vertical scaling factor. Set the data height of selected
 *  waveforms. This does not effect the initial data height for new waveforms,
 *  which is set by setDataHeight().
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] data_height The data height in pixels.
 *  @param[in] visible_only If True, set the data height for waveforms currently
 *			visible in the data window, subject to the 'all'
 *			parameter. If False, set the data height according
 *			to the 'all' parameter.
 *  @param[in] all If True, set the data height for all waveforms. If False, set
 *		the data height for selected waveforms only. (Subject to the
 *		'visible_only' parameter.)
 */
void CPlotClass::setScale(int data_height, bool visible_only, bool all)
{
    CPlotPart *cp = &cw->c_plot;
    bool changed_one = false;
    int i;
    CPlotWidget z;

    if(data_height == 0) return;
    data_height = abs(data_height);

    for(i = 0; i < cp->num_entries; i++) if(all || cp->entry[i]->w->selected)
    {
	DataEntry *entry = cp->entry[i];
	double drag_yscale = (double)data_height/(double)cp->dataHeight;

	if(visible_only  && visibleMinMax(cw, entry) &&
		entry->visible_ymin != entry->visible_ymax)
	{
	    // scale based on visible region
	    drag_yscale *= fabs((entry->ymax - entry->ymin)/
				(entry->visible_ymax - entry->visible_ymin));
	}
	if(entry->drag_yscale != drag_yscale)
	{
	    entry->drag_yscale = drag_yscale;
	    changed_one = True;
	}
    }
    if(changed_one)
    {
	_AxesRedraw((AxesWidget)cw);
	_CPlotRedraw(cw);
	_AxesRedisplayAxes((AxesWidget)cw);
	_CPlotRedisplay(cw);
	_AxesRedisplayXor((AxesWidget)cw);

	if((z = (CPlotWidget)cw->axes.mag_to) != NULL)
	{
	    CPlotMagnifyWidget(cw, NULL, True);
	    CPlotMagnifyWidget(cw, z, True);
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	}
    }
}

void CPlotClass::scaleUp(bool all)
{
    double fac = 1.5;
    changeScale(cw, fac, all);
}

void CPlotClass::scaleDown(bool all)
{
    double fac;
    fac = 2./3.;
    changeScale(cw, fac, all);
}

static void
changeScale(CPlotWidget w, double fac, Boolean all)
{
	CPlotPart *cp = &w->c_plot;
	int i;
	CPlotWidget z;

	for(i = 0; i < cp->num_entries; i++) 
	    if(all || cp->entry[i]->w->selected)
	{
	    cp->entry[i]->drag_yscale *= fac;
	}
	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);

	if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	{
	    CPlotMagnifyWidget(w, NULL, True);
	    CPlotMagnifyWidget(w, z, True);
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	}
}

bool CPlotClass::getUniformScale()
{
    return !cw->c_plot.scale_independently;
}

/** 
 *  Set the waveform scale to be uniform or independent.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] set If true, set the waveform amplitude scale to be uniform for
 * 		all waveforms.  If False, the waveform amplitude scales
 *		will be independent.
 *  
 */
void CPlotClass::setUniformScale(bool set)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	double max_ydif = 0.;
	int i;

	if(cp->scale_independently != !set) {
	    cp->scale_independently = !set;
	}

	for(i = 0; i < cp->num_entries; i++)
	{
	    DataEntry *entry = cp->entry[i];

	    if(max_ydif == 0.) {
		max_ydif = entry->ymax - entry->ymin;
	    }
	    else {
		if(entry->ymax - entry->ymin > max_ydif)
		    max_ydif = entry->ymax - entry->ymin;
	    }
	    if(entry->drag_yscale != 1.) {
		entry->drag_yscale = 1.;
	    }
	}
	if(max_ydif != 0.) {
	    CPlotWidget z;
	    if(cp->uniform_scale != max_ydif) {
		cp->uniform_scale = max_ydif;
	    }

	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw(w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _CPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);

	    if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	    {
		CPlotMagnifyWidget(w, NULL, True);
		CPlotMagnifyWidget(w, z, True);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
	    }
	}
}

#define mod(i,j) (i-((i)/(j))*(j))

static void
DrawLabel(CPlotWidget w, DataCurve *curve)
{
	CPlotPart *cp = &w->c_plot;
	int i, j, lab_len, beg;
#ifndef MAPALF
	float fac;
#endif
	double arg, xdif, ydif;
	Boolean ok;

	if((lab_len = (int)curve->lab.length()) == 0) return;

#ifndef MAPALF
	XTextExtents(ax->font, curve->lab, lab_len, &direction, &ascent,
			&descent, &overall);
#endif

	/* label the curve in the a different spot than the previous call,
	 * since curves of consecutive calls will probably be close together.
	 */
	cp->draw_label_call = mod(cp->draw_label_call+1, 8);
	if(curve->s.nsegs < 4)
	{
	    i = curve->s.nsegs - 1;
#ifdef MAPALF
	    xdif = curve->s.segs[i].x2 - curve->s.segs[i].x1;
	    ydif = curve->s.segs[i].y1 - curve->s.segs[i].y2;

	    if(xdif == 0. && ydif == 0.) {
		curve->lab_angle = 0.;
	    }
	    else {
		curve->lab_angle = 180.*atan2(ydif, xdif)/3.14159265;
		if(curve->lab_angle < -90) curve->lab_angle += 180.;
		else if(curve->lab_angle > 90) curve->lab_angle -= 180.;
	    }
	    arg = 3.14159265*curve->lab_angle/180.;
	    curve->lab_x = (int)(curve->s.segs[i].x1 - 4*sin(arg));
	    curve->lab_y = (int)(curve->s.segs[i].y1 - 4*cos(arg));
#else
	    fac = (cp->draw_label_call+1)/10.;
	    curve->lab_y = (int)(curve->s.segs[i].y1 + fac*(curve->s.segs[i].y2
					- curve->s.segs[i].y1));
	    curve->lab_x = (int)(curve->s.segs[i].x1 + fac*(curve->s.segs[i].x2
				- curve->s.segs[i].x1) - .5*overall.width);
#endif
	    return;
	}
	else if(curve->s.nsegs < 10)
	{
	    beg = curve->s.nsegs - 1 - cp->draw_label_call*curve->s.nsegs/10;
#ifndef MAPALF
	    fac = (cp->draw_label_call+1)/10.;
#endif
	}
	else
	{
	    beg = curve->s.nsegs - 1 - cp->draw_label_call*curve->s.nsegs/10;
#ifndef MAPALF
	    fac = .5;
#endif
	}

	for(i = beg, ok = False; i >= 0 && !ok; i--)
	{
	    ok = True;
#ifdef MAPALF
	    for(j = i; j < curve->s.nsegs-1; j++)
	    {
		if(abs(curve->s.segs[j].x1-curve->s.segs[i].x2) > 20 ||
		   abs(curve->s.segs[j].y1-curve->s.segs[i].y2) > 20) break;
	    }
	    curve->lab_x = curve->s.segs[i].x1;
	    curve->lab_y = curve->s.segs[i].y1;
	    xdif = curve->s.segs[j].x2 - curve->s.segs[i].x1;
	    ydif = curve->s.segs[i].y1 - curve->s.segs[j].y2;
	    if(xdif == 0. && ydif == 0.) {
		curve->lab_angle = 0.;
	    }
	    else
	    {
		curve->lab_angle = 180.*atan2(ydif, xdif)/3.14159265;
		if(curve->lab_angle < -90) curve->lab_angle += 180.;
		else if(curve->lab_angle > 90) curve->lab_angle -= 180.;
	    }
	    arg = 3.14159265*curve->lab_angle/180.;
	    curve->lab_x = (int)(curve->s.segs[i].x1 - 4*sin(arg));
	    curve->lab_y = (int)(curve->s.segs[i].y1 - 4*cos(arg));
#else
	    curve->lab_y = (int)(curve->s.segs[i].y1 + fac*(curve->s.segs[i].y2
				- curve->s.segs[i].y1));
	    if(curve->lab_y - 2*overall.ascent < ax->d->clipy1) {
		ok = False;
	    }
	    if(curve->lab_y+2*overall.ascent > ax->d->clipy2) {
		ok = False;
	    }
	    curve->lab_x = curve->s.segs[i].x1 + fac*(curve->s.segs[i].x2 -
				curve->s.segs[i].x1) - .5*overall.width;
	    if(curve->lab_x + .5*overall.width > ax->clipx2) {
		ok = False;
	    }
	    if(curve->lab_x - .5*overall.width < ax->clipx1) {
		ok = False;
	    }
#endif
	}
}

static void
DisplayTag(CPlotWidget w, DataEntry *entry)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i, x, y;
	GC		gc;
	static int	npoints = 0;
	static XPoint	*points = NULL;

	x = entry->tag_x;
	y = entry->tag_y;
	if(ax->x_axis != LEFT)
	{
	    if(entry->tag_x < ax->clipx1) {
		x = ax->clipx1;
	    }
	    else if(entry->tag_x + entry->tag_width > ax->clipx2) {
		x = ax->clipx2 - entry->tag_width;
	    }
	}
	else
	{
	    if(entry->tag_y < ax->clipy1) {
		y = ax->clipy1;
	    }
	    else if(entry->tag_y + entry->tag_height > ax->clipy2) {
		y = ax->clipy2 - entry->tag_height;
	    }
	}
	entry->lab_x = x;
	entry->lab_y = y;

	if(!cp->display_tags || entry->tag_width <= 0 || entry->tag_height <= 0)
	{
		return;
	}
	if( !XtIsRealized((Widget)w) ) return;

	if(entry->n_tag_points > npoints) {
	    Free(points);
	    points = (XPoint *)MallocIt(w, entry->n_tag_points*sizeof(XPoint));
	    npoints = entry->n_tag_points;
	}
	for(i = 0; i < entry->n_tag_points; i++) {
	    points[i].x = x + entry->tag_points[i].x;
	    points[i].y = y + entry->tag_points[i].y;
	}

	XSetPlaneMask(XtDisplay(w), ax->invert_clipGC, entry->w->fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), ax->invert_clipGC, entry->w->fg);
	gc = ax->invert_clipGC;
	if(!ax->use_pixmap || ax->use_screen)
	{
	    if(entry->w->selected) {
		XFillRectangle(XtDisplay(w), XtWindow(w), gc, x, y,
				entry->tag_width, entry->tag_height);
	    }
	    else {
		XDrawRectangle(XtDisplay(w), XtWindow(w), gc, x, y,
				entry->tag_width-1, entry->tag_height-1);
	    }
	    XDrawPoints(XtDisplay(w), XtWindow(w), gc, points,
				entry->n_tag_points, CoordModeOrigin);
	}
	if(ax->use_pixmap)
	{
	    if(entry->w->selected) {
		XFillRectangle(XtDisplay(w), ax->pixmap, gc, x, y,
				entry->tag_width, entry->tag_height);
	    }
	    else {
		XDrawRectangle(XtDisplay(w), ax->pixmap, gc, x, y,
				entry->tag_width-1, entry->tag_height-1);
	    }
	    XDrawPoints(XtDisplay(w), ax->pixmap, gc, points,
				entry->n_tag_points, CoordModeOrigin);
	}
	XSetPlaneMask(XtDisplay(w), ax->invert_clipGC, ax->fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), ax->invert_clipGC, ax->fg);
}

/** 
 *  Select/deselect waveforms. Select or deselect all waveforms, or
 *  only visible waveforms or only waveforms with a specific channel type.
 *  Possible values of chan_type are:
 *  <p><TT>CPLOT_ALL_COMPONENTS</TT> - Select all components.
 *  <p><TT>CPLOT_Z_COMPONENT</TT> - Select z components only.
 *  <p><TT>CPLOT_HORIZONTAL_COMPONENTS</TT> - Select horizontal components only.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] on_off If True, select waveforms. If False, deselect waveforms.
 *  @param[in] visible_only If True, select only visible waveforms, subject to
 *		'chan_type'.
 *  @param[in] chan_type The component selection parameter.
 */
void CPlotSelectAll(CPlotWidget w, bool on_off, bool visible_only,int chan_type)
{
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i, num_cursors;

    // deactivate the cursor callbacks. Do one callback below.
    ax->use_screen = False;

    num_cursors = ax->num_cursors;
    ax->num_cursors = 0;
    for(i = 0; i < cp->num_entries; i++) if(cp->entry[i]->on)
    {
	if(cp->entry[i]->w->selected != on_off &&
	    (!visible_only || cp->entry[i]->w->visible) &&
	    chan_check(cp->entry[i], chan_type))
	{
	    Select(w, cp->entry[i], True);
	}
    }
    ax->use_screen = True;
    if(ax->use_pixmap && XtIsRealized((Widget)w)) {
	XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
    }

    if(ax->mag_to != NULL) {
	CPlotSelectAll((CPlotWidget)ax->mag_to, on_off, visible_only,chan_type);
    }
    ax->num_cursors = num_cursors;
}

static Boolean
chan_check(DataEntry *entry, int chan_type)
{
	switch(chan_type)
	{
	    case CPLOT_ALL_COMPONENTS:
		return True;
		break;
	    case CPLOT_Z_COMPONENT:
		return (entry == entry->comps[2]) ? True : False;
		break;
	    case CPLOT_HORIZONTAL_COMPONENTS:
		return (entry != entry->comps[2]) ? True : False;
		break;
	    default:
		return True;
		break;
	}
	return true;
}

/** 
 *  Initialize the magnify window. This function is called each time the
 *  magnify window is popuped up. A magnifyCallback function will call this
 *  function with the widgetId, z, of the magnify CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] z Another CPlotWidget pointer to the magnify window.
 *  @param[in] redisplay If redisplay is False, the magnify window is not
 *			updated. Otherwise it is.
 *  
 */
void
CPlotMagnifyWidget(CPlotWidget w, CPlotWidget z, Boolean redisplay)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;
	CPlotWidget mag;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	if(z == NULL)
	{
	    _AxesRedisplayMagRect((AxesWidget)w);
	    ax->magnify_rect = False;
	    if((mag = (CPlotWidget)ax->mag_to) != NULL)
	    {
		for(i = 0; i < mag->c_plot.num_curves; i++) {
		    Free(mag->c_plot.curve[i]->s.segs);
		    Free(mag->c_plot.curve[i]);
		}
		Free(mag->c_plot.curve);
		mag->c_plot.num_curves = 0;
		for(i = 0; i < mag->c_plot.num_entries; i++)
		{
		    Free(mag->c_plot.entry[i]->sel.segs);
		    Free(mag->c_plot.entry[i]->unsel.segs);
		    Free(mag->c_plot.entry[i]->r.segs);
		    Free(mag->c_plot.entry[i]->arrlab);
		    Free(mag->c_plot.entry[i]->pred_arr);
		    Free(mag->c_plot.entry[i]->tag_points);
		    Free(mag->c_plot.entry[i]->w);
		    if(mag->c_plot.entry[i]->beg) {
			mag->c_plot.entry[i]->beg->deleteObject();
		    }
		    if(mag->c_plot.entry[i]->end) {
			mag->c_plot.entry[i]->end->deleteObject();
		    }
		    Free(mag->c_plot.entry[i]);
		}
		Free(mag->c_plot.entry);
		mag->c_plot.num_entries = 0;
	    }
	    ax->mag_to = NULL;
	    return;
	}
	if(ax->mag_to != NULL) {
	    return;
	}

	Initialize((Widget)z, (Widget)z);

	ax->mag_to = (Widget)z;

	z->axes.mag_from = (Widget)w;
	z->axes.mag_to = NULL;

	z->axes.zoom = 1;
	z->axes.zoom_min = 1;
	z->axes.zoom_max = 1;
	z->c_plot.tmin = cp->tmin;
	z->c_plot.tmax = cp->tmax;
	z->c_plot.scale_independently = cp->scale_independently;
	z->c_plot.uniform_scale = cp->uniform_scale;
	z->c_plot.max_tag_width = cp->max_tag_width;
	z->c_plot.arrival_font = cp->arrival_font;
	z->c_plot.associated_arrival_font = cp->associated_arrival_font;
	z->c_plot.tag_font = cp->tag_font;

	z->axes.x_min = ax->x_min;
	z->axes.x_max = ax->x_max;
	z->axes.y_min = ax->y_min;
	z->axes.y_max = ax->y_max;
	z->axes.x_axis = ax->x_axis;
	z->axes.y_axis = ax->y_axis;
	z->axes.reverse_x = ax->reverse_x;
	z->axes.reverse_y = ax->reverse_y;
	z->axes.unmark = ax->unmark;
	z->axes.time_scale = ax->time_scale;
	z->axes.display_axes_labels = ax->display_axes_labels;
	z->axes.horizontal_scroll = ax->horizontal_scroll;
	z->axes.vertical_scroll = ax->vertical_scroll;
	z->axes.last_x_mark = ax->last_x_mark;
	z->axes.last_y_mark = ax->last_y_mark;
	z->axes.display_axes_labels = ax->display_axes_labels;
	z->axes.a.y_label_int = ax->a.y_label_int;
	z->axes.num_cursors = ax->num_cursors;
	memcpy(z->axes.cursor, ax->cursor, ax->num_cursors*sizeof(AxesCursor));
	z->c_plot.dataHeight = cp->dataHeight;
	z->c_plot.dataSeparation = cp->dataSeparation;
	z->c_plot.dataSpacing = cp->dataSpacing;
	z->c_plot.data_movement = cp->data_movement;
	z->c_plot.display_components = cp->display_components;
	z->c_plot.display_tags = cp->display_tags;
	z->c_plot.redraw_selected_data = cp->redraw_selected_data;
	z->c_plot.display_arrivals = cp->display_arrivals;
	z->c_plot.display_predicted_arrivals = cp->display_predicted_arrivals;
	z->c_plot.display_assoc_only = cp->display_assoc_only;
	z->c_plot.display_amplitude_scale = cp->display_amplitude_scale;
	z->c_plot.three_half_cycles = cp->three_half_cycles;
	z->c_plot.measure_box = cp->measure_box;
	z->c_plot.single_arr_select = cp->single_arr_select;
	z->c_plot.last_arr_select = False;
	z->c_plot.retime_arrival = cp->retime_arrival;

	delete z->c_plot.v;
	z->c_plot.v = cp->v;

	z->c_plot.max_tag_width = cp->max_tag_width;
	z->c_plot.num_entries = 0;
	z->c_plot.num_curves = 0;

	position(z);

	_AxesRedraw((AxesWidget)z);
	for(i = 0; i < cp->num_entries; i++) {
	    MagEntry(w, cp->entry[i]);
	}
	for(i = 0; i < cp->num_curves; i++) {
	    MagCurve(w, cp->curve[i]);
	}
	_CPlotRedraw(z);

	if(redisplay) {
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	}
}

static void
MagEntry(CPlotWidget w, DataEntry *we)
{
	AxesPart *ax = &w->axes;
	CPlotWidget z;
	DataEntry *ze;
	DataEntry data_entry_init = DATA_ENTRY_INIT;

	if((z = (CPlotWidget)ax->mag_to) == NULL) return;

	z->c_plot.num_entries++;
	ALLOC(z->c_plot.num_entries*sizeof(DataEntry *),
		z->c_plot.size_entry, z->c_plot.entry, DataEntry *, 0);
	if( !(z->c_plot.entry[z->c_plot.num_entries-1] =
		(DataEntry *)MallocIt(w, sizeof(DataEntry))) ) return;

	ze = z->c_plot.entry[z->c_plot.num_entries-1];

	memcpy(ze, &data_entry_init, sizeof(DataEntry));
	ze->type = we->type;

	ze->w = new Waveform(we->ts);
	copyCD(ze->w, we->w);

	ze->ts = we->ts;
	ze->use_ts = ze->ts;
	memcpy(ze->p, we->p, MAX_DECI*sizeof(DeciArray));
	ze->n_arrlab = we->n_arrlab;

	if(ze->n_arrlab > 0) {
	    if( !(ze->arrlab = (ArrivalLabel *)MallocIt(w,
			ze->n_arrlab*sizeof(ArrivalLabel))) )
	    {
		delete z->c_plot.entry[z->c_plot.num_entries-1]->w;
		Free(z->c_plot.entry[z->c_plot.num_entries-1]);
		z->c_plot.num_entries--;
		return;
	    }
	    memcpy(ze->arrlab, we->arrlab, ze->n_arrlab*sizeof(ArrivalLabel));
	}

	ze->npred_arr = we->npred_arr;
	if(ze->npred_arr > 0) {
	    if( !( ze->pred_arr = (CPlotPredArr *)MallocIt(w,
			ze->npred_arr*sizeof(CPlotPredArr))) )
	    {
		Free(ze->arrlab);
		delete z->c_plot.entry[z->c_plot.num_entries-1]->w;
		Free(z->c_plot.entry[z->c_plot.num_entries-1]);
		z->c_plot.num_entries--;
		return;
	    }
	    memcpy(ze->pred_arr, we->pred_arr,
			ze->npred_arr*sizeof(CPlotPredArr));
	}
	ze->xscale = we->xscale;
	ze->yscale = we->yscale;
	ze->length = we->length;
	ze->mean = we->mean;
	ze->ymin = we->ymin;
	ze->ymax = we->ymax;
	ze->display_time = we->display_time;
	ze->on = we->on;
	ze->type = we->type;
	ze->tbeg = we->tbeg;
	ze->tend = we->tend;
	ze->modified = True;
	ze->n_measure_segs = we->n_measure_segs;
	ze->left_side = we->left_side;
	ze->bottom_side = we->bottom_side;
	ze->period = we->period;
	ze->amp_ts = we->amp_ts;
	ze->amp_cnts = we->amp_cnts;
	ze->amp_nms = we->amp_nms;
	ze->zp_ts = we->zp_ts;
	strncpy(ze->tag, we->tag, 99);
	ze->tag[99] = '\0';
	DrawTag(z, ze);
	ze->origin = we->origin;
	ze->w->begSelect = 0.;
	ze->w->endSelect = 0.;

	if(we == w->c_plot.ref)
	{
		z->c_plot.ref = ze;
	}

	FindComponents(z, ze);

	FindArrivalsOnData(z, ze);

	z->c_plot.need_sort = True;

	we->mag_entry = (void *)ze;
	ze->mag_entry = (void *)we;
}

static void
copyCD(Waveform *c1, Waveform *c2)
{
//    c1->id = c2->id;
    c1->cplot = c2->cplot;
    c1->ts = c2->ts;
    c1->channel = c2->channel;
    c1->visible = c2->visible;
    c1->selected = c2->selected;
    c1->scaled_x0 = c2->scaled_x0;
    c1->scaled_y0 = c2->scaled_y0;
    c1->fg = c2->fg;
    c1->num_dp = c2->num_dp;
    c1->dp = c2->dp;
    c1->num_dw = c2->num_dw;
    c1->dw = c2->dw;
    c1->begSelect = c2->begSelect;
    c2->endSelect = c2->endSelect;
    for(int i = 0; i < MAX_COMPONENTS; i++) {
	c1->c[i] = c2->c[i];
    }
    c1->file_order = c2->file_order;
    c1->distance = c2->distance;
}

static void
MagCurve(CPlotWidget w, DataCurve *we)
{
	AxesPart *ax = &w->axes;
	CPlotWidget z;
	DataCurve *ze;

	if((z = (CPlotWidget)ax->mag_to) == NULL)
	{
		return;
	}

	z->c_plot.num_curves++;
	ALLOC(z->c_plot.num_curves*sizeof(DataCurve *),
		z->c_plot.size_curve, z->c_plot.curve, DataCurve *, 0);
	ze = new DataCurve();
	z->c_plot.curve[z->c_plot.num_curves-1] = ze;
	ze->npts = we->npts;
	ze->type = we->type;
	ze->lab = we->lab;
	ze->x = we->x;
	ze->y = we->y;
	ze->x_orig = we->x_orig;
	ze->ray_p = we->ray_p;
	ze->xmin = we->xmin;
	ze->xmax = we->xmax;
	ze->ymin = we->ymin;
	ze->ymax = we->ymax;
	ze->on = we->on;
	ze->fg = we->fg;
}

/** 
 *  
 */
static void
_CPlotSelectData(AxesWidget widget)
{
	CPlotWidget w = (CPlotWidget)widget;
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;
	CPlotWidget z;

	z = (CPlotWidget)ax->mag_to;
	if(ax->zooming)
	{
	    for(i = 0; i < cp->num_entries; i++)
		if(cp->entry[i]->on && cp->entry[i]->w->visible)
	    {
		if(InRectangle(cp->entry[i], ax->zoom_x1, ax->zoom_y1,
				ax->zoom_x2, ax->zoom_y2))
		{
		    Select(w, cp->entry[i], True);
		    if(z != NULL) {
			Select(z, z->c_plot.entry[i], False);
		    }
		}
	    }
	}
}

/** 
 *  Return A CPlotWidget's axes limits to their original un-zoomed values.
 *  The horizontal axis limits will be adjusted to the minimum and maximum
 *  times of all waveforms. The vertical axis will be adjusted according
 *  to the initial data height and data separation parameters.
 *  @param[in] w A CPlotWidget pointer.
 */
void CPlotZoomOut(CPlotWidget w)
{
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    double tmin, tmax;

    if(ax->zoom > ax->zoom_min) {
	ax->zoom = ax->zoom_min;
    }
    cp->data_shift = False;
    if(cp->num_entries > 0) { // force limits change
	CPlotGetDataDuration(w, &tmin, &tmax);
	cp->tmin = tmax + .5*fabs(tmax-tmin);
    }

    AdjustHorizontalLimits(w);
}

static void
Motion(CPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, entry_x0, entry_y0;
	ArrivalLabel	*arrlab;

	_AxesMotion((AxesWidget)w, event, params, num_params);

	if(!cp->curves_only) {
	    moveOverEntry(w, event);
	}
	else {
	    moveOverCurve(w, event);
	}

	cp->motion_cursor_x = ((XButtonEvent *)event)->x;
	cp->motion_cursor_y = ((XButtonEvent *)event)->y;

	if(WhichArrival(w, cp->motion_cursor_x, cp->motion_cursor_y, &i, &j))
	{
	    if(i != cp->arrival_i || j != cp->arrival_j)
	    {
		unsigned long millisecs;

		if(cp->arrival_i >= 0) {
		    DrawArrivalRec(w, ax->mag_invertGC);
		    destroyInfoPopup(w);
		}
		arrlab = &cp->entry[i]->arrlab[j];

		entry_x0 = unscale_x(&ax->d, cp->entry[i]->w->scaled_x0
			    + cp->entry[i]->beg->time() - cp->entry[i]->tbeg);
		entry_y0 = unscale_y(&ax->d,cp->entry[i]->w->scaled_y0);
		cp->arrival_rec_x = entry_x0 +
			(int)(arrlab->x - .5*arrlab->a_entry->width -3);
		cp->arrival_rec_y = entry_y0 + (int)(arrlab->y - 3);
		cp->arrival_rec_width = arrlab->a_entry->width + 5;
		cp->arrival_rec_height = arrlab->a_entry->height + 5;
		DrawArrivalRec(w, ax->mag_invertGC);
		cp->arrival_i = i;
		cp->arrival_j = j;
		cp->info_arrival_i = i;
		cp->info_arrival_j = j;

		millisecs = (int)fabs(1000*cp->info_delay);
		if(cp->app_context) {
		    XtAppAddTimeOut(cp->app_context, millisecs,
			(XtTimerCallbackProc)arrivalInfo, (XtPointer)w);
		}
		else {
		    XtAddTimeOut(millisecs, (XtTimerCallbackProc)arrivalInfo,
			(XtPointer)w);
		}
	    }
	    _AxesSetCursor((AxesWidget)w, "hand");
	    return;
	}
	else
	{
	    if(cp->arrival_i >= 0) {
		DrawArrivalRec(w, ax->mag_invertGC);
		destroyInfoPopup(w);
	    }
	    cp->arrival_i = -1;
	    cp->arrival_j = -1;
	    cp->arrival_moving = False;
	}
	if(!WhichMeasureBoxSide(w, cp->motion_cursor_x, cp->motion_cursor_y))
	{
	    if(cp->label_entry == NULL) {
		if(cp->data_movement == CPLOT_NO_MOVEMENT) {
		    _AxesSetCursor((AxesWidget)w, "default");
		}
		else {
		    _AxesSetCursor((AxesWidget)w, "move");
		}
	    }
	}
}

static void
arrivalInfo(XtPointer client_data, XtIntervalId id)
{
	Widget w = (Widget)client_data;
	CPlotWidget widget = (CPlotWidget)client_data;
	CPlotPart *cp = &widget->c_plot;
	AxesPart *ax = &widget->axes;
	Widget z = (ax->mag_from) ? ax->mag_from : w;

	if(!cp->arrival_moving && cp->arrival_i >= 0 && cp->arrival_j >= 0
		&& cp->arrival_i == cp->info_arrival_i
		&& cp->arrival_j == cp->info_arrival_j)
	{
	    Position x, y;
	    CPlotInfoCallbackStruct c;

	    destroyInfoPopup(widget);

	    c.w = cp->entry[cp->arrival_i]->w;
	    c.a = cp->entry[cp->arrival_i]->arrlab[cp->arrival_j].a_entry->a;
	    c.label[0] = '\0';

	    XtCallCallbacks((Widget)z, XtNarrivalInfoCallback, (XtPointer)&c);

	    if(c.label[0] != '\0') {
		XmString xm =XmStringCreateLtoR(c.label,XmFONTLIST_DEFAULT_TAG);
		cp->info_popup = XtVaCreateWidget("infoShell",
				overrideShellWidgetClass,
				(Widget)widget, XmNborderWidth, 1, NULL);
		XtVaCreateManagedWidget("infoLabel", xmLabelWidgetClass,
				cp->info_popup, XmNlabelString, xm, NULL);
		XmStringFree(xm);
		XtTranslateCoords(w, 0, 0, &x, &y);
		x += cp->arrival_rec_x + cp->arrival_rec_width + 5;
		y += cp->arrival_rec_y + cp->arrival_rec_height + 5;
		XtMoveWidget(cp->info_popup, x, y);
		XtManageChild(cp->info_popup);
	    }
	}
}

static void
waveformInfo(XtPointer client_data, XtIntervalId id)
{
	Widget w = (Widget)client_data;
	CPlotWidget widget = (CPlotWidget)client_data;
	CPlotPart *cp = &widget->c_plot;
	AxesPart *ax = &widget->axes;
	Widget z = (ax->mag_from) ? ax->mag_from : w;

	if(cp->label_entry != NULL && cp->label_entry == cp->info_entry)
	{
	    Position x, y;
	    CPlotInfoCallbackStruct c;

	    destroyInfoPopup(widget);

	    c.w = cp->label_entry->w;
	    c.a = NULL;
	    c.label[0] = '\0';

	    XtCallCallbacks((Widget)z, XtNwaveformInfoCallback, (XtPointer)&c);

	    if(c.label[0] != '\0') {
		XmString xm =XmStringCreateLtoR(c.label,XmFONTLIST_DEFAULT_TAG);
		cp->info_popup = XtVaCreateWidget("infoShell",
				overrideShellWidgetClass,
				(Widget)widget, XmNborderWidth, 1, NULL);
		XtVaCreateManagedWidget("infoLabel", xmLabelWidgetClass,
				cp->info_popup, XmNlabelString, xm, NULL);
		XmStringFree(xm);
		XtTranslateCoords(w, 0, 0, &x, &y);
		x += cp->label_entry->lab_x + cp->label_entry->tag_width + 5;
		y += cp->label_entry->lab_y + cp->label_entry->tag_height + 5;
		XtMoveWidget(cp->info_popup, x, y);
		XtManageChild(cp->info_popup);
	    }
	}
}

static void
destroyInfoPopup(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;

	if(cp->info_popup)
	{
	    XtUnmanageChild(cp->info_popup);
	    XtDestroyWidget(cp->info_popup);
	    cp->info_popup = NULL;
	}
}

static void
LeaveWindow(CPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;

	if(cp->arrival_i >= 0) {
	    if(cp->arrival_moving) {
		XEvent  e;
		e.type = ButtonRelease;
		MoveArrival(w, &e);
	    }
	    else {
		DrawArrivalRec(w, ax->mag_invertGC);
		destroyInfoPopup(w);
	    }
	}
	cp->arrival_i = -1;
	cp->arrival_j = -1;
	cp->arrival_moving = False;
	if(cp->point_x >= 0) {
	    DrawPointRec(w, ax->mag_invertGC);
	}
	cp->point_x = -1;

/*
	cp->label_entry = NULL;
*/
	cp->info_entry = NULL;

	destroyInfoPopup(w);
}

static void
EnterWindow(CPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	Motion(w, event, params, num_params);
}

static void
CPlot_AxesZoom(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	CPlotWidget z = (ax->mag_from) ? (CPlotWidget)ax->mag_from : w;

	cp->info_entry = NULL;
	cp->info_arrival_i = -1;
	cp->info_arrival_j = -1;

	destroyInfoPopup(w);

	if(cp->retime_arrival) return;

	if(cp->measure_entry != NULL && cp->resize_measure_box >= 0) {
	    ResizeMeasureBox(w, event);
	}  
	else if(ax->cursor_mode != AXES_MOVE_MAGNIFY &&
		cp->arrival_i == -1 && cp->label_entry == NULL)
	{
	    if(cp->display_add_arrival && *num_params > 0
		&& !strcmp(params[0], "horizontal") && cp->add_arrival_x == -1)
	    {
                unsigned long millisecs = 300;
		if(cp->app_context) {
		    XtAppAddTimeOut(cp->app_context, millisecs,
			(XtTimerCallbackProc)arrivalAddMenu, (XtPointer)w);
		}
		else {
		    XtAddTimeOut(millisecs, (XtTimerCallbackProc)arrivalAddMenu,
			(XtPointer)w);
		}
		cp->add_arrival_x = ((XButtonEvent *)event)->x;
		cp->add_arrival_y = ((XButtonEvent *)event)->y;
		cp->add_arrival_event = *(XButtonEvent *)event;
	    }
	    _AxesZoom((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if(cp->arrival_i >= 0) {
	    CPlotMenuCallbackStruct c;
	    int i = cp->arrival_i;
	    int j = cp->arrival_j;
	    c.reason = CPLOT_ARRIVAL;
	    c.w = cp->entry[i]->w;
	    c.arrival = cp->entry[i]->arrlab[j].a_entry->a;
	    c.event = event;
	    c.main_plot = w;
	    destroyInfoPopup(w);
	    XtCallCallbacks((Widget)z, XtNarrivalMenuCallback, (XtPointer)&c);
	}
	else if(cp->label_entry != NULL) {
	    CPlotMenuCallbackStruct c;
	    c.reason = CPLOT_WAVEFORM;
	    c.w = cp->label_entry->w;
	    c.arrival = NULL;
	    c.event = event;
	    c.main_plot = w;
	    XtCallCallbacks((Widget)z, XtNwaveformMenuCallback, (XtPointer)&c);
	}
}

static void
arrivalAddMenu(XtPointer client_data, XtIntervalId id)
{
    CPlotWidget widget = (CPlotWidget)client_data;
    CPlotPart *cp = &widget->c_plot;
    AxesPart *ax = &widget->axes;

    if(!cp->display_add_arrival || cp->add_arrival_x < 0) return;

    if(ax->no_motion) {
 	addArrivalCallback(widget, false);
    }
    cp->add_arrival_x = -1;
} 

static void
addArrivalCallback(CPlotWidget widget, bool shift)
{
    CPlotPart *cp = &widget->c_plot;
    AxesPart *ax = &widget->axes;
    CPlotArrivalCallbackStruct p;

    DataEntry *entry = WhichEntry(widget, cp->add_arrival_x, cp->add_arrival_y);
    if(!entry) {
	cp->add_arrival_x = -1;
	return;
    }
    p.w = entry->w;
    p.shift = shift;
    memset(p.name, 0, sizeof(p.name));
    p.time = entry->ts->tbeg() + scale_x(&ax->d, cp->add_arrival_x)
			- entry->w->scaled_x0;
    p.event = (XEvent *)&cp->add_arrival_event;
    ax->undraw = False;
    ax->zooming = False;
    ax->hor_only = True;
    ax->no_motion = True;
    XtCallCallbacks((Widget)widget,XtNaddArrivalMenuCallback,(XtPointer)&p);
}

bool CPlotClass::retimeArrivalOn(CssArrivalClass *a, bool on)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    CPlotWidget z;
    ArrivalEntry *a_entry = NULL;
    bool ret = false;

    if( !cp->v->arrivals.contains(a) )  {
	    fprintf(stderr, "CPlotRetimeArrival: arrival not found.\n");
	    return False;
    }
    cp->retime_arrival = False;
    for(int i = 0; i < cp->v->arrival_entries.size(); i++)
    {
	a_entry = cp->v->arrival_entries[i];
	if( a_entry->a == a ) {
	    a_entry->retime = on;
	    ret = True;
	}
	if(a_entry->retime) cp->retime_arrival = True;
    }
    if((z = (CPlotWidget)ax->mag_to) != NULL) {
	z->c_plot.retime_arrival = cp->retime_arrival;
    }
    if(!ret) {
	fprintf(stderr, "CPlotRetimeArrival: arrival not found.\n");
    }
    return ret;
}

void CPlotClass::retimeArrivalAllOff()
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    CPlotWidget z;
    ArrivalEntry *a_entry = NULL;

    for(int i = 0; i < cp->v->arrival_entries.size(); i++) {
	a_entry = cp->v->arrival_entries[i];
	a_entry->retime = false;
    }
    cp->retime_arrival = False;

    if((z = (CPlotWidget)ax->mag_to) != NULL) {
	z->c_plot.retime_arrival = cp->retime_arrival;
    }
}

/** 
 *  
 */
static void
_CPlotMouseDownSelect(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int cursor_x, cursor_y;

	cp->info_entry = NULL;
	cp->info_arrival_i = -1;
	cp->info_arrival_j = -1;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(outside(w, cursor_x, cursor_y)) return;

	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;

	if(cp->arrival_i >= 0)
	{
	    if(cp->entry[cp->arrival_i]->arrlab[cp->arrival_j].a_entry->retime)
	    {
		MoveArrival(w, event);
	    }
	    else if(!cp->retime_arrival) {
		select_arrival(w, event);
	    }
	}
	else if(cp->measure_entry == NULL || cp->resize_measure_box < 0)
	{
	    if(cp->label_entry != NULL) {
		cp->select_entry = cp->label_entry;
	        mouseDownSelect(w, cp->select_entry, cursor_x);
	    }
	    else if(ax->cursor_mode == AXES_MOVE_MAGNIFY ||
		    ax->cursor_mode == AXES_SELECT_CURSOR)
	    {
		_AxesMove((AxesWidget)w, event, (const char **)params,
			num_params);
	    }
	    else if(cp->data_movement != CPLOT_NO_MOVEMENT) {
		cp->move_entry = WhichEntry(w, cursor_x, cursor_y);
	        mouseDownMove(w, cp->move_entry, cursor_x, cursor_y, False);
	    }
	    else {
		cp->select_entry = WhichEntry(w, cursor_x, cursor_y);
	        mouseDownSelect(w, cp->select_entry, cursor_x);
	    }
	}
	destroyInfoPopup(w);
}

static Boolean
outside(CPlotWidget w, int cursor_x, int cursor_y)
{
	AxesPart *ax = &w->axes;
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


/** 
 *  
 */
static void
_CPlotMouseCtrlSelect(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;

	ax->last_cursor_x = ((XButtonEvent *)event)->x;
	ax->last_cursor_y = ((XButtonEvent *)event)->y;

	if(outside(w, ax->last_cursor_x, ax->last_cursor_y)) return;

	destroyInfoPopup(w);

	if(cp->arrival_i >= 0) {
	    if(!cp->retime_arrival) {
		select_arrival(w, event);
	    }
	}
	else {
	    if(cp->label_entry != NULL) {
		cp->select_entry = cp->label_entry;
	        mouseCtrlSelect(w, cp->select_entry);
	    }
	    else if(ax->cursor_mode == AXES_MOVE_MAGNIFY ||
		    ax->cursor_mode == AXES_SELECT_CURSOR)
	    {
	        _AxesScale((AxesWidget)w, event, params, num_params);
	    }
	    else {
		cp->select_entry = WhichEntry(w, ax->last_cursor_x,
						ax->last_cursor_y);
		if(cp->select_entry != NULL) {
		    mouseCtrlSelect(w, cp->select_entry);
		}
	    }
	}
}

/** 
 *  
 */
static void
_CPlotMouseShiftSelect(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;

	ax->last_cursor_x = ((XButtonEvent *)event)->x;
	ax->last_cursor_y = ((XButtonEvent *)event)->y;

	if(outside(w, ax->last_cursor_x, ax->last_cursor_y)) return;

	destroyInfoPopup(w);

	if(cp->arrival_i >= 0) {
	    if(!cp->retime_arrival) {
		select_arrival(w, event);
	    }
	}
	else {
	    if(cp->label_entry != NULL) {
		cp->select_entry = cp->label_entry;
	    }
	    else if(ax->cursor_mode != AXES_MOVE_MAGNIFY &&
		    ax->cursor_mode != AXES_SELECT_CURSOR)
	    {
		cp->select_entry = WhichEntry(w, ax->last_cursor_x,
						ax->last_cursor_y);
	    }
	    if(cp->select_entry != NULL) {
		mouseShiftSelect(w, cp->select_entry, ax->last_cursor_x);
	    }
	}
}

static void
mouseDownSelect(CPlotWidget w, DataEntry *selected_entry, int x)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	Boolean one = False;
	int i;

	cp->tmp_selected = (Boolean *)AxesMalloc((Widget)w,
				cp->num_entries*sizeof(Boolean));
	for(i = 0; i < cp->num_entries; i++) {
	    cp->tmp_selected[i] = cp->entry[i]->w->selected;
	}

	cp->begSelect = scale_x(&ax->d, x);

	for(i = 0; i < cp->num_entries; i++) {
	    DataEntry *entry = cp->entry[i];
	    if(entry->w->selected && entry != selected_entry) {
		ax->use_screen = False;
		_CPlotRedisplayEntry(w, entry);
		entry->w->selected = False;
		entry->w->begSelect = 0.;
		entry->w->endSelect = 0.;
		_CPlotRedrawEntry(w, entry);
		_CPlotRedisplayEntry(w, entry);
		ax->use_screen = True;
	    }
	}
	cp->selecting = True;
	cp->deselecting = False;

	if(selected_entry != NULL)
	{
	    ax->use_screen = False;
	    _CPlotRedisplayEntry(w, selected_entry);
	    if(!cp->allow_partial_select && selected_entry->w->selected)
	    {
		Free(cp->tmp_selected);
		cp->selecting = False;
		cp->deselecting = True;
	    }
	    else
	    {
		if(cp->allow_partial_select)
		{
		    selected_entry->w->begSelect = selected_entry->ts->tbeg()
			    + cp->begSelect - selected_entry->w->scaled_x0;
		    selected_entry->w->endSelect =
			selected_entry->w->begSelect;
		}
	        cp->y0Select = unscale_y(&ax->d, selected_entry->w->scaled_y0);
	    }
	    selected_entry->w->selected = !selected_entry->w->selected;
	    _CPlotRedrawEntry(w, selected_entry);
	    _CPlotRedisplayEntry(w, selected_entry);
	    ax->use_screen = True;
	    one = True;
	}
	if(one) {
	    if(ax->use_pixmap) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	    }
	}
}

static void
mouseDownMove(CPlotWidget w, DataEntry *entry, int cursor_x, int cursor_y,
		Boolean manual)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;

	if(cp->data_movement == CPLOT_NO_MOVEMENT) return;
	cp->mov.moved = False;
	if(ax->x_axis != LEFT) {
	    cp->mov.cutx1 = ax->clipx1;
	    cp->mov.cutx2 = ax->clipx2;
	}
	else {
	    cp->mov.cutx1 = ax->clipy1;
	    cp->mov.cutx2 = ax->clipy2;
	}
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;

	if(!manual)
	{
	    CPlotWidget z;
	    if((z = (CPlotWidget)ax->mag_to) != NULL)
	    {
		cursor_x = unscale_x(&z->axes.d, scale_x(&ax->d, cursor_x));
		cursor_y = unscale_y(&z->axes.d, scale_y(&ax->d, cursor_y));
		mouseDownMove(z, (DataEntry *)entry->mag_entry, cursor_x,
				cursor_y, True);
	    }
	    if((z = (CPlotWidget)ax->mag_from) != NULL)
	    {
		cursor_x = unscale_x(&z->axes.d, scale_x(&ax->d, cursor_x));
		cursor_y = unscale_y(&z->axes.d, scale_y(&ax->d, cursor_y));
		mouseDownMove(z, (DataEntry *)entry->mag_entry, cursor_x,
				cursor_y, True);
	    }
	}
}


static void
mouseCtrlSelect(CPlotWidget w, DataEntry *entry)
{
	AxesPart *ax = &w->axes;

	ax->use_screen = False;
	_CPlotRedisplayEntry(w, entry);
	entry->w->selected = !entry->w->selected;
	entry->w->begSelect = 0.;
	entry->w->endSelect = 0.;
/*
	entry->ts->setSelectionStart(0.);
	entry->ts->setSelectionEnd(0.);
*/
	_CPlotRedrawEntry(w, entry);
	_CPlotRedisplayEntry(w, entry);
	if(ax->use_pixmap) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC, 0, 0,
			w->core.width, w->core.height, 0, 0);
	}
	ax->use_screen = True;
	XtCallCallbacks((Widget)w, XtNselectDataCallback, NULL);
}

/** 
 *  
 */
static void
_CPlotMouseDrag(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;

	if(cp->arrival_moving) {
	    MoveArrival(w, event);
	}
	else if(cp->selecting && cp->label_entry != NULL) {
	    mouseDragSelect(w, event);
	}
	else if( w->axes.cursor_mode == AXES_MOVE_MAGNIFY) {
	    _AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if( w->axes.cursor_mode == AXES_SELECT_CURSOR ||
	    w->axes.cursor_mode == AXES_STRETCH_MAGNIFY)
	{
	    _AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if(cp->selecting) {
	    mouseDragSelect(w, event);
	}
	else if(cp->scaling) {
	    _CPlotMouseDownScale(w, event, params, num_params);
	}
	else if(w->axes.zooming) {
	    _AxesZoom((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if(cp->move_entry != NULL) {
	    mouseDragMove(w, event, (DataEntry *)NULL, 0., 0.);
	}
	else if(cp->measure_entry != NULL && cp->resize_measure_box >= 0) {
	    ResizeMeasureBox(w, event);
	}  
}

static void
mouseShiftSelect(CPlotWidget w, DataEntry *entry, int x)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int	i, i1, i2, num;
	Boolean	select_total;
	DataEntry *entry_selected, **sorted = NULL;
	double	beg=0., end=0., t, tbeg, tend;

	if(entry == NULL) return;

	sorted = (DataEntry **)AxesMalloc((Widget)w,
			cp->num_entries*sizeof(DataEntry *));
	num = 0;
	for(i = 0; i < cp->num_entries; i++) {
	    if(cp->entry[i]->on) {
		sorted[num++] = cp->entry[i];
	    }
	}
	if(!num) {
	    Free(sorted);
	    return;
	}
	qsort(sorted, num, sizeof(DataEntry *), sort_by_y0);

	for(i = 0; i < num; i++) {
	    if(entry == sorted[i]) break;
	}
	if(i == num) return;

        for(i1 = i-1; i1 >= 0; i1--) {
	    if(sorted[i1]->w->selected) break;
	}
	for(i2 = i+1; i2 < num; i2++) {
	    if(sorted[i2]->w->selected) break;
	}
	if(i1 < 0 && i2 == num) {
	    ax->use_screen = False;
	    _CPlotRedisplayEntry(w, entry);
	    if(entry->w->selected &&
			entry->w->begSelect != entry->w->endSelect) {
		t = entry->ts->tbeg() + scale_x(&ax->d, x)
			- entry->w->scaled_x0;
		if(fabs(t-entry->w->begSelect) < fabs(t-entry->w->endSelect)){
		    entry->w->begSelect = t;
		}
		else {
		    entry->w->endSelect = t;
		}
	    }
	    else {
		entry->w->selected = !entry->w->selected;
		entry->w->begSelect = entry->w->endSelect = 0.;
	    }
	    _CPlotRedrawEntry(w, entry);
	    _CPlotRedisplayEntry(w, entry);
	    if(ax->use_pixmap) {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	    }
	    ax->use_screen = True;
	    XtCallCallbacks((Widget)w, XtNselectDataCallback, NULL);
            return;
        }
        if(i1 < 0) {
            i1 = i;
            entry_selected = sorted[i2];
            i2--;
        }
        else if(i2 == num) {
            entry_selected = sorted[i1];
            i1++;
            i2 = i;
        }
        else if(i-i1 < i2-i) {
            entry_selected = sorted[i1];
            i1++;
            i2 = i;
        }
        else {
            i1 = i;
            entry_selected = sorted[i2];
            i2--;
        }
	tbeg = entry_selected->ts->tbeg();
	tend = entry_selected->ts->tend();
	if((entry_selected->w->begSelect > tbeg &&
		entry_selected->w->begSelect < tend) ||
           (entry_selected->w->endSelect > tbeg &&
		entry_selected->w->endSelect < tend))
	{
	    select_total = False;

	    beg = entry_selected->w->scaled_x0 +
                    (entry_selected->w->begSelect - tbeg);
	    end = entry_selected->w->scaled_x0 +
                    (entry_selected->w->endSelect - tbeg);
        }
        else select_total = True;

	ax->use_screen = False;

	for(i = i1; i <= i2; i++) {
	    entry = sorted[i];
	    _CPlotRedisplayEntry(w, entry);
            entry->w->selected = True;
            if(select_total) {
                entry->w->begSelect = entry->ts->tbeg();
                entry->w->endSelect = entry->ts->tend();
            }
            else {
                entry->w->begSelect = entry->ts->tbeg() + beg
					- entry->w->scaled_x0;
                entry->w->endSelect = entry->ts->tbeg() + end
					- entry->w->scaled_x0;
            }
	    _CPlotRedrawEntry(w, entry);
	    _CPlotRedisplayEntry(w, entry);
        }
	Free(sorted);
        if(i2 >= i1) {
	    if(ax->use_pixmap) {
	        XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	    }
	    XtCallCallbacks((Widget)w, XtNselectDataCallback, NULL);
	}
	ax->use_screen = True;
}

static void
mouseDragSelect(CPlotWidget w, XEvent *event)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i;
	double t, beg, y0Select;
	int halfHeight = cp->dataHeight/2;
	int x = ((XButtonEvent *)event)->x;
	int y = ((XButtonEvent *)event)->y;

	if(cp->num_entries == 0) return;

	if(x == ax->last_cursor_x && y == ax->last_cursor_y) return;
	ax->last_cursor_x = x;
	ax->last_cursor_y = y;

	y0Select = scale_y(&ax->d, cp->y0Select);
	if(_AxesDragScroll((AxesWidget)w, event)) {
	    cp->y0Select = unscale_y(&ax->d, y0Select);
	}

	ax->use_screen = False;
	for(i = 0; i < cp->num_entries; i++) if(cp->entry[i]->on)
	{
	    DataEntry *entry = cp->entry[i];
	    int entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    double tbeg =  entry->ts->tbeg();

	    if(overlaps((double)cp->y0Select, (double)y,
		(double)(entry_y0-halfHeight), (double)(entry_y0+halfHeight)))
	    {
		_CPlotRedisplayEntry(w, entry);

		t = tbeg + scale_x(&ax->d, x) - entry->w->scaled_x0;
		beg = tbeg + cp->begSelect - entry->w->scaled_x0;
		if(cp->allow_partial_select &&
			fabs(t-beg) > 10*fabs(ax->d.scalex))
		{
		    if(t > beg) {
			entry->w->begSelect = beg;
			entry->w->endSelect = t;
		    }
		    else {
			entry->w->begSelect = t;
			entry->w->endSelect = beg;
		    }
		}
		else {
		    entry->w->begSelect = 0.;
		    entry->w->endSelect = 0.;
                }
		entry->w->selected = True;
		_CPlotRedrawEntry(w, entry);
		_CPlotRedisplayEntry(w, entry);
            }
            else if(entry->w->selected) {
		_CPlotRedisplayEntry(w, entry);
                entry->w->selected = False;
		entry->w->begSelect = 0.;
		entry->w->endSelect = 0.;
                _CPlotRedrawEntry(w, entry);
		_CPlotRedisplayEntry(w, entry);
            }
        }
	ax->use_screen = True;
	if(ax->use_pixmap) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC, 0, 0,
			w->core.width, w->core.height, 0, 0);
	}
}


/** 
 *  
 */
static void
_CPlotMouseUp(CPlotWidget w, XEvent *event, String *params,Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
        Widget z = (ax->mag_from) ? ax->mag_from : (Widget)w;

	cp->add_arrival_x = -1;
	if(cp->arrival_i >= 0 && cp->arrival_j >= 0 &&
	    cp->entry[cp->arrival_i]->arrlab[cp->arrival_j].a_entry->retime)
	{
	    MoveArrival(w, event);
	}
	else if(cp->selecting && cp->label_entry != NULL) {
	    mouseUpSelect(w, event);
	}
	else if(cp->adding_arrival && ax->m < ax->num_cursors) {
	    CPlotArrivalCallbackStruct p;
	    p.shift = false;
	    strncpy(p.name, ax->cursor[ax->m].label, sizeof(p.name));
            p.time = ax->cursor[ax->m].scaled_x
		    - cp->select_entry->w->scaled_x0 + cp->select_entry->tbeg;
	    p.w = cp->select_entry->w;
	    p.event = NULL;
	    ax->cursor_mode = AXES_ZOOM;
	    _AxesSetCursor((AxesWidget)w, "default");
	    XtCallCallbacks(z, XtNaddArrivalCallback, &p);
	    cp->cplot_class->deletePhaseLine();
	}
	else if( w->axes.cursor_mode == AXES_MOVE_MAGNIFY ||
	    w->axes.cursor_mode == AXES_SELECT_CURSOR)
	{
	    _AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if(w->axes.zooming) {
	    _AxesZoom((AxesWidget)w, event, (const char **)params, num_params);
	}
	else if(cp->selecting || cp->deselecting) {
	    mouseUpSelect(w, event);
	}
	else if(cp->scaling) {
	    _CPlotScale(w, event, params, num_params);
	}
	else if(cp->move_entry != NULL)
	{
	    CPlotPositionCallbackStruct c;
	    if(cp->adjust_after_move) {
		AdjustHorizontalScrollbars(w);
	    }
	    c.reason = CPLOT_DATA_POSITION;
	    c.wvec.push_back(cp->move_entry->w);
	    XtCallCallbacks(z, XtNpositionCallback, &c);
	}
	Free(cp->tmp_selected);
	cp->selecting = False;
	cp->deselecting = False;
	cp->move_entry = NULL;
	cp->adding_arrival = False;
	cp->last_key_str[0] = '\0';
}

static void
mouseUpSelect(CPlotWidget w, XEvent *event)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
        CPlotWidget z;
	int i;

	for(i = 0; i < cp->num_entries; i++)
	{
	    DataEntry *entry = cp->entry[i];
	    if(entry->w->selected &&
			entry->w->begSelect == entry->w->endSelect) {
		entry->w->begSelect = entry->ts->tbeg();
		entry->w->endSelect = entry->ts->tend();
	    }
/*
	    entry->ts->setSelectionStart(entry->w->beginSelect);
	    entry->ts->setSelectionEnd(entry->w->endSelect);
*/
        }
//	if(ax->mag_from == NULL && cp->tmp_selected)
	if(cp->tmp_selected)
	{
	    DataEntry *entry = NULL;
	    int n;
	    for(i = n = 0; i < cp->num_entries; i++) {
		if(cp->entry[i]->w->selected && !cp->tmp_selected[i]) {
		    entry = cp->entry[i];
		    n++;
		}
	    }
	    if(n == 1) {
		if(entry->w->selected) {
		    XtCallCallbacks((Widget)w, XtNsingleSelectDataCallback,
				entry->w);
		}
	    }
	}
	if(ax->mag_from == NULL) {
	    XtCallCallbacks((Widget)w, XtNselectDataCallback, NULL);
	}
	Free(cp->tmp_selected);

	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
            for(i = 0; i < cp->num_entries; i++) {
                DataEntry *entry = cp->entry[i];
                DataEntry *zentry = getZoomEntry(z, entry->ts);
                if(zentry != NULL) {
                    zentry->w->selected = entry->w->selected;
                    zentry->w->begSelect = entry->w->begSelect;
                    zentry->w->endSelect = entry->w->endSelect;
                }
            }
	    _CPlotRedraw(z);
	    z->axes.use_screen = True;
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
        }
	if((z = (CPlotWidget)ax->mag_from) != NULL)
	{
            for(i = 0; i < cp->num_entries; i++) {
                DataEntry *entry = cp->entry[i];
                DataEntry *zentry = getZoomEntry(z, entry->ts);
                if(zentry != NULL) {
                    zentry->w->selected = entry->w->selected;
                    zentry->w->begSelect = entry->w->begSelect;
                    zentry->w->endSelect = entry->w->endSelect;
                }
            }
	    _CPlotRedraw(z);
	    z->axes.use_screen = True;
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
        }
}

static DataEntry *
getZoomEntry(CPlotWidget w, GTimeSeries *ts)
{
	/* find corresponding entry in the zoom window
	 */
	int i;

	for(i = 0; i < w->c_plot.num_entries; i++) {
	    if(w->c_plot.entry[i]->ts == ts) {
                return w->c_plot.entry[i];
            }
        }
        return NULL;
}

static void
mouseDragMove(CPlotWidget w, XEvent *event, DataEntry *manual_entry,
		double manual_scaled_x0, double manual_scaled_y0)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	DataEntry *entry;
	Boolean partial_select;
	int cursor_x, cursor_y, delx, dely, entry_x0, entry_y0;
	int new_x0, new_y0;
	double clipx1, clipx2, new_scaled_x0, new_scaled_y0, scroll_shift = 0.;
	GSegment *s;
	CPlotPositionCallbackStruct c;

	if(cp->data_movement == CPLOT_NO_MOVEMENT) {
	    return;
	}
	ax->use_screen = False;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;


	new_scaled_x0 = 0.;
	new_scaled_y0 = 0.;

	if(manual_entry == NULL)
	{
	    if(ax->x_axis != LEFT) {
		delx = (cp->data_movement == CPLOT_X_MOVEMENT ||
			cp->data_movement == CPLOT_XY_MOVEMENT) ?
				cursor_x - ax->last_cursor_x : 0;
		dely = (cp->data_movement == CPLOT_Y_MOVEMENT ||
			cp->data_movement == CPLOT_XY_MOVEMENT) ?
				cursor_y - ax->last_cursor_y : 0;
	    }
	    else {
		delx = (cp->data_movement == CPLOT_X_MOVEMENT ||
			cp->data_movement == CPLOT_XY_MOVEMENT) ?
				cursor_y - ax->last_cursor_y : 0;
		dely = (cp->data_movement == CPLOT_Y_MOVEMENT ||
			cp->data_movement == CPLOT_XY_MOVEMENT) ?
				cursor_x - ax->last_cursor_x : 0;
	    }
	    if(abs(delx) < 1 && abs(dely) < 1) {
		ax->use_screen = True;
		return;
	    }

	    entry = cp->move_entry;
	    if(cp->data_movement == CPLOT_XY_MOVEMENT ||
		cp->data_movement == CPLOT_Y_MOVEMENT)
	    {
		double y2 = ax->y2[ax->zoom];
		if(_AxesDragScroll((AxesWidget)w, event)) {
		    scroll_shift = (ax->y2[ax->zoom] - y2);
		}
	    }
	    _CPlotRedisplayEntry(w, entry);

	    if(scroll_shift != 0.) {
		entry->w->scaled_y0 += scroll_shift;
		_CPlotRedrawEntry(w, entry);
	    }

	    cp->mov.moved = True;
	    entry_x0 = unscale_x(&ax->d, entry->w->scaled_x0);
	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    entry_x0 += delx;
	    entry_y0 += dely;
	    if(ax->x_axis != LEFT) {
		if(delx != 0) {
		    entry->w->scaled_x0 = scale_x(&ax->d, entry_x0);
		}
		if(dely != 0) {
		    entry->w->scaled_y0 = scale_y(&ax->d, entry_y0);
		}
	    }
	    else {
		if(delx != 0) {
		    entry->w->scaled_x0 = scale_y(&ax->d, entry_x0);
		}
		if(dely != 0) {
		    entry->w->scaled_y0 = scale_x(&ax->d, entry_y0);
		}
	    }
	    new_scaled_x0 = entry->w->scaled_x0;
	    new_scaled_y0 = entry->w->scaled_y0;
	}
	else
	{
	    entry = manual_entry;
	    entry_x0 = unscale_x(&ax->d, entry->w->scaled_x0);
	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    _CPlotRedisplayEntry(w, entry);
	    cp->mov.moved = True;
	    entry->w->scaled_x0 = manual_scaled_x0;
	    entry->w->scaled_y0 = manual_scaled_y0;
	    new_x0 = unscale_x(&ax->d, entry->w->scaled_x0);
	    new_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    delx = new_x0 - entry_x0;
	    dely = new_y0 - entry_y0;
	    entry_x0 = new_x0;
	    entry_y0 = new_y0;
	}
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;

	if(XtHasCallbacks((Widget)w, XtNpositionDragCallback) !=
				XtCallbackHasNone)
	{
	    c.reason = CPLOT_DATA_DRAG;
	    c.wvec.push_back(entry->w);
	    XtCallCallbacks((Widget)w, XtNpositionDragCallback, &c);
	}
	entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);

	clipx1 = cp->mov.cutx1 - entry_x0;
	clipx2 = cp->mov.cutx2 - entry_x0;

	s = entry->use_ts->segment(entry->use_ts->size()-1); /* last segment */

	if(entry->r.nsegs <= 0 || (entry->r.segs[0].x1 > clipx1 &&
	    (entry->beg->index() > 0 || entry->beg->segmentIndex() > 0)) ||
	    (entry->r.segs[entry->r.nsegs-1].x2 < clipx2 &&
	    (entry->end->segment() != s ||
		entry->end->index() < s->length()-1)))
	{
	    _CPlotRedrawEntry(w, entry);
	    _CPlotRedisplayEntry(w, entry);
	}
	else
	{
	    int i, j, i1, i2, begSel, endSel;
	    i1 = entry->r.start;
	    while(i1 > 0 && entry->r.segs[i1].x1 > clipx1) i1--;

	    while(i1 < entry->r.nsegs-1 && entry->r.segs[i1].x2 < clipx1) i1++;

	    i2 = entry->r.start + entry->r.nshow - 1;
	    while(i2 > 0 && entry->r.segs[i2].x1 > clipx2) i2--;

	    while(i2 < entry->r.nsegs-1 && entry->r.segs[i2].x2 < clipx2) i2++;

	    entry->r.start = i1;
	    entry->r.nshow = i2 - i1 + 1;

	    partial_select = selectLimits(w, entry, &begSel, &endSel);

	    if(	entry_x0 + entry->r.xmin < -S_LIM ||
		entry_x0 + entry->r.xmax > S_LIM ||
		entry_y0 + entry->r.ymin < -S_LIM ||
		entry_y0 + entry->r.ymax > S_LIM)
	    {
		/* do the clipping here. X can't handle it. */
		SegArray *sa = entry->w->selected ? &entry->sel:&entry->unsel;
		DoClipping(w, entry->r.nshow, entry->r.segs+entry->r.start,
				entry_x0, entry_y0, sa);
	    }
	    else if(partial_select)
	    {
		int k, l;
		Free(entry->sel.segs);
		Free(entry->unsel.segs);
		entry->sel.size_segs = entry->unsel.size_segs = 0;
		entry->sel.nsegs = 0;
		entry->unsel.nsegs = 0;
		for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		    if(entry->r.segs[j].x1 >= begSel
			&& entry->r.segs[j].x2 <= endSel)
		    {
			entry->sel.nsegs++;
		    }
		    else {
			entry->unsel.nsegs++;
		    }
		}
		if(entry->sel.nsegs > 0) {
		  ALLOC(entry->sel.nsegs*sizeof(DSegment), entry->sel.size_segs,
		    entry->sel.segs, DSegment, 0);
		}
		if(entry->unsel.nsegs > 0) {
		  ALLOC(entry->unsel.nsegs*sizeof(DSegment),
		    entry->unsel.size_segs, entry->unsel.segs, DSegment, 0);
		}
		for(i=k=l=0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		 if(entry->r.segs[j].x1 >=begSel && entry->r.segs[j].x2<=endSel)
		 {
		    entry->sel.segs[k].x1 = 
			(short)(entry_x0 + entry->r.segs[j].x1 + .5);
		    entry->sel.segs[k].y1 = 
			(short)(entry_y0 + entry->r.segs[j].y1 + .5);
		    entry->sel.segs[k].x2 = 
			(short)(entry_x0 + entry->r.segs[j].x2 + .5);
		    entry->sel.segs[k].y2 = 
			(short)(entry_y0 + entry->r.segs[j].y2 + .5);
		    k++;
		 }
		 else {
		    entry->unsel.segs[l].x1 = 
			(short)(entry_x0 + entry->r.segs[j].x1+.5);
		    entry->unsel.segs[l].y1 = 
			(short)(entry_y0 + entry->r.segs[j].y1+.5);
		    entry->unsel.segs[l].x2 = 
			(short)(entry_x0 + entry->r.segs[j].x2+.5);
		    entry->unsel.segs[l].y2 = 
			(short)(entry_y0 + entry->r.segs[j].y2+.5);
		    l++;
		 }
		}
	    }
	    else {
		SegArray *sa = entry->w->selected ? &entry->sel:&entry->unsel;
		sa->nsegs = entry->r.nshow;
		ALLOC(sa->nsegs*sizeof(DSegment), sa->size_segs, sa->segs,
			DSegment, 0);
		for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		    sa->segs[i].x1 = 
			(short)(entry_x0 + entry->r.segs[j].x1 + .5);
		    sa->segs[i].y1 = 
			(short)(entry_y0 + entry->r.segs[j].y1 + .5);
		    sa->segs[i].x2 = 
			(short)(entry_x0 + entry->r.segs[j].x2 + .5);
		    sa->segs[i].y2 = 
			(short)(entry_y0 + entry->r.segs[j].y2 + .5);
		}
	    }
	    if(entry->r.nshow == 0) {
		entry->w->visible = False;
	    }
	    else {
		entry->w->visible = True;
	    }
	
	    entry->tag_x += delx;
	    entry->tag_y += dely;
	    _CPlotRedisplayEntry(w, entry);
	    ax->use_screen = True;
	}

	if(ax->use_pixmap) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			 0, 0, w->core.width, w->core.height, 0, 0);
	}
	if(manual_entry == NULL)
	{
	    CPlotWidget z;
	    if((z = (CPlotWidget)ax->mag_to) != NULL)
	    {
		((XButtonEvent *)event)->x = unscale_x(&z->axes.d,
				scale_x(&ax->d, cursor_x));
		((XButtonEvent *)event)->y = unscale_y(&z->axes.d,
				scale_y(&ax->d, cursor_y));
		mouseDragMove(z, event, (DataEntry *)entry->mag_entry,
				new_scaled_x0, new_scaled_y0);
	    }
	    if((z = (CPlotWidget)ax->mag_from) != NULL)
	    {
		((XButtonEvent *)event)->x = unscale_x(&z->axes.d,
				scale_x(&ax->d, cursor_x));
		((XButtonEvent *)event)->y = unscale_y(&z->axes.d,
				scale_y(&ax->d, cursor_y));
		mouseDragMove(z, event, (DataEntry *)entry->mag_entry,
				new_scaled_x0, new_scaled_y0);
	    }
	}
}

static void
moveOverEntry(CPlotWidget w, XEvent *event)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	DataEntry *prev_label_entry = NULL;
	int cursor_x = ((XButtonEvent *)event)->x;
	int cursor_y = ((XButtonEvent *)event)->y;

	cp->select_entry = NULL;

	if(cursor_x < ax->clipx1 || cursor_x > ax->clipx2 ||
	   cursor_y < ax->clipy1 || cursor_y > ax->clipy2)
	{
	    destroyInfoPopup(w);
	    cp->label_entry = NULL;
	    cp->info_entry = NULL;
	    return;
	}

	prev_label_entry = cp->label_entry;

	cp->select_entry = WhichEntry(w, cursor_x, cursor_y);

	if(cp->label_entry != prev_label_entry) {
	    destroyInfoPopup(w);
	}

	if(cp->label_entry != NULL) {
	    _AxesSetCursor((AxesWidget)w, "hand");
	    if(cp->label_entry != prev_label_entry) {
		unsigned long millisecs = (int)fabs(1000*cp->info_delay);
		cp->info_entry = cp->label_entry;
		if(cp->app_context) {
		    XtAppAddTimeOut(cp->app_context, millisecs,
			(XtTimerCallbackProc)waveformInfo, (XtPointer)w);
		}
		else {
		    XtAddTimeOut(millisecs, (XtTimerCallbackProc)waveformInfo,
			(XtPointer)w);
		}
	    }
	}

	if(cp->select_entry != NULL && ax->cursor_info != NULL)
	{
	    DataEntry *entry = cp->select_entry;
	    char label[100];
	    int i, i1, i2, imin, x_min, y_min, last_x = -1, dx, dx_min = 10000;
	    int entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    int entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    double x, y, dist, dist_min;
	    double tbeg = entry->use_ts->tbeg();
	    double scaled_x = scale_x(&ax->d, cursor_x);
	    double time = tbeg + scaled_x - entry->w->scaled_x0;
	    double time_min;
	    GDataPoint *dp = entry->use_ts->nearest(time);
	    time = dp->segment()->time(dp->index());

	    i1 = dp->index() - 100;
	    if(i1 < 0) i1 = 0;
	    i2 = dp->index() + 100;
	    if(i2 >= dp->segment()->length()) i2 = dp->segment()->length()-1;
	    
	    imin = i1;
	    x = entry_x0 + floor((dp->segment()->time(i1) - entry->beg->time())
				*entry->xscale + .5);
	    y = entry_y0 - (dp->segment()->data[i1] -entry->mean)*entry->yscale;
	    x_min = (int)x;
	    y_min = (int)y;
	    last_x = (int)x;
	    time_min = time;
	    dist_min = pow(cursor_x - x, 2.) + pow(cursor_y - y, 2.);

	    for(i = i1; i <= i2; i++)
	    {
		time = dp->segment()->time(i);
		x = entry_x0 + floor((time - entry->beg->time())
				*entry->xscale + .5);
		y = entry_y0 - (dp->segment()->data[i] - entry->mean)*
					entry->yscale;
		dist = pow(cursor_x - x, 2.) + pow(cursor_y - y, 2.);
		if(dist < dist_min) {
		    dist_min = dist;
		    time_min = time;
		    x_min = (int)x;
		    y_min = (int)y;
		    imin = i;
		}
		dx = (int)(x - last_x);
		if(dx > 0 && dx < dx_min) dx_min = dx;
		last_x = (int)x;
	    }

	    snprintf(label, 100, "%s/%s  %s  %f",entry->ts->sta(),
			entry->w->channel.c_str(), timeEpochToGMT(time_min, 3),
			dp->segment()->data[imin]);
	    if(dp) dp->deleteObject();

	    InfoSetText(ax->cursor_info, label);

	    if(cp->highlight_points && entry->ts->length() > 2
			&& dx_min != 10000 && dx_min > 4)
	    {
		if(cp->point_x != x_min || cp->point_y != y_min)
		{
		    if(cp->point_x >= 0) {
			DrawPointRec(w, ax->mag_invertGC);
		    }
		    cp->point_rec_x = x_min - 1;
		    cp->point_rec_y = y_min - 1;
		    cp->point_rec_width = 3;
		    cp->point_rec_height = 3;
/*
		    cp->point_segs[0].x = x_min - 1;
		    cp->point_segs[0].y = y_min - 1;
		    cp->point_segs[0].width = 3;
		    cp->point_segs[0].height = 3;
		    cp->point_segs[1].x = x_min - 2;
		    cp->point_segs[1].y = y_min - 2;
		    cp->point_segs[1].width = 5;
		    cp->point_segs[1].height = 5;
*/
		    DrawPointRec(w, ax->mag_invertGC);
		    cp->point_x = x_min;
		    cp->point_y = y_min;
		}
	    }
	    else if(cp->point_x >= 0)
	    {
		DrawPointRec(w, ax->mag_invertGC);
		cp->point_x = -1;
	    }
	}
	else if(cp->point_x >= 0)
	{
	    DrawPointRec(w, ax->mag_invertGC);
	    cp->point_x = -1;
	}
}

static void
moveOverCurve(CPlotWidget w, XEvent *event)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int cursor_x = ((XButtonEvent *)event)->x;
	int cursor_y = ((XButtonEvent *)event)->y;
	int i, j, jmin=0;
	double x, y, xmin=0.;
	DataCurve *curve_min;

	if(ax->cursor_info == NULL) return;

	if(cursor_x < ax->clipx1 || cursor_x > ax->clipx2 ||
	   cursor_y < ax->clipy1 || cursor_y > ax->clipy2) return;

	x = scale_x(&ax->d, cursor_x);
	y = scale_y(&ax->d, cursor_y);

	curve_min = NULL;
	if(cp->num_curves == 1)
	{
	    curve_min = cp->curve[0];
	    jmin = -1;
	    for(j = 0; j < curve_min->npts; j++)
	    {
		double dist = fabs(curve_min->x[j] - x);
		if(jmin == -1) {
		    jmin = 0;
		    xmin = dist;
		}
		else if(dist < xmin) {
		    jmin = j;
		    xmin = dist;
		}
	    }
	}
	else {
	    for(i = 0; i < cp->num_curves; i++) if(cp->curve[i]->on) 
	    {
	 	DataCurve *curve = cp->curve[i];
		for(j = 0; j < curve->npts; j++)
		{
		    double dx = curve->x[j] - x;
		    double dy = curve->y[j] - y;
		    double dist = dx*dx + dy*dy;

		    if(curve_min == NULL) {
			curve_min = curve;
			jmin = 0;
			xmin = dist;
		    }
		    else if(dist < xmin) {
			curve_min = curve;
			jmin = j;
			xmin = dist;
		    }
		}
	    }
	}

	if(curve_min != NULL)
	{
	    char label[100];
	    snprintf(label, sizeof(label), "%s  %.4f  %.4f",
			curve_min->mouse_label.c_str(),
			curve_min->x[jmin], curve_min->y[jmin]);
	    InfoSetText(ax->cursor_info, label);
	}
}

static void
MoveEntry(CPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	XEvent	e;
	int	cursor_x, cursor_y;

        cursor_x = ((XButtonEvent *)event)->x;
        cursor_y = ((XButtonEvent *)event)->y;

	cp->move_entry = WhichEntry(w, cursor_x, cursor_y);

	if(*num_params < 2 || cp->move_entry == NULL ||
		cp->data_movement == CPLOT_NO_MOVEMENT) return;

	mouseDownMove(w, cp->move_entry, cursor_x, cursor_y, False);

	if(!strcmp(params[0], "up")) {
	    cursor_y -= atoi(params[1]);
	}
	else if(!strcmp(params[0], "down")) {
	    cursor_y += atoi(params[1]);
	}
	else if(!strcmp(params[0], "right")) {
	    cursor_x += atoi(params[1]);
	}
	else if(!strcmp(params[0], "left")) {
	    cursor_x -= atoi(params[1]);
	}
	else {
	    return;
	}
	e.type = MotionNotify;
	((XButtonEvent *)(&e))->x = cursor_x;
	((XButtonEvent *)(&e))->y = cursor_y;

	mouseDragMove(w, &e, (DataEntry *)NULL, 0., 0.);
}

/** 
 *  Place amplitude measurement boxes on waveforms at the line-cursor position.
 *  Amplitude measurement boxes are drawn on waveforms at the position of
 *  a phase-line-cursor created with AxesAddPhaseLine().
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] selected If True, boxes are drawn only on selected waveforms,
 *			otherwise they are drawn on all waveforms.
 *  @param[in] with_arrival If True, the first arrival of waveforms on which
 *			a box is drawn is selected.
 *  @returns True if any boxes were drawn.
 *  @see AxesAddPhaseLine
 */
bool CPlotClass::measureAll(bool selected, bool with_arrival)
{
	CPlotWidget	w = cw;
	CPlotPart	*cp = &cw->c_plot;
	AxesPart	*ax = &cw->axes;
	int		i, j, ix;
	Boolean		found_one;
	AxesCursor	*c;
	DataEntry	*entry;

	if(!XtIsRealized((Widget)cw)) return false;

	if(ax->mag_from != NULL) {
	    w = (CPlotWidget)ax->mag_from;
	}
	if(cp->resize_measure_box >= 0) return false;

	for(i = 0; i < ax->num_cursors; i++) {
	    if(ax->cursor[i].type == AXES_PHASE_LINE) break;
	}
	if(i == ax->num_cursors) {
	    /* look for vertical line */
	    for(i = 0; i < ax->num_cursors; i++) {
		if(ax->cursor[i].type == AXES_VERTICAL_LINE) break;
	    }
	    if(i == ax->num_cursors) {
		_AxesWarning((AxesWidget)w, "Need a vertical line.");
		return false;
	    }
	}
	c = &ax->cursor[i];

	found_one = false;
	for(i = 0; i < cp->num_entries; i++) if(cp->entry[i]->on
		&& (!selected || cp->entry[i]->w->selected))
	{
	    found_one = true;

	    entry = cp->entry[i];

	    if(with_arrival)
	    {
		ArrivalEntry *a_entry;
		CssArrivalClass *a;
		for(j = 0; j < cp->v->arrival_entries.size(); j++)
		{
		    a_entry = cp->v->arrival_entries[j];
		    a = a_entry->a;
			 
		    if((!strcasecmp(entry->ts->sta(), a->sta) ||
			!strcasecmp(entry->ts->net(), a->sta)) &&
			entry->tbeg > a->time &&
			entry->ts->tend() < a->time &&
			!a_entry->selected)
		    {
			DoSelectArrival(w, a_entry, True);
			break;
		    }
		}
	    }
	    DrawMeasureBoxes(w, entry);
	    ix = unscale_x(&ax->d, c->scaled_x);
	    FindMeasureBoxes(w, ix, entry);
	    DrawMeasureBoxes(w, entry);
	    DoAmpConvertCallback(w, 1, &entry);
	}
	if(!found_one) {
	    _AxesWarning((AxesWidget)w, "No waveforms selected.");
	    return false;
	}
	return true;
}

/** 
 *  Search selected waveforms for a cycle within period limits.
 *  Search waveforms within a double-line cursor to find the first occurrence
 *  of a cycle with a period that is between min_per and max_per.
 *  Draw a measurement box on the first qualifying cycle of each waveform.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] min_per The minimum period in seconds.
 *  @param[in] max_per The maximum period in seconds.
 *  @param[in] do_largest_amp If True, and no cycle is found with a qualifying
 *		period, then a measure box is drawn on the cycle with the
 *		largest amplitude. If False, no measure box is drawn on
 *		waveforms that do not have a qualifying period within the
 *		double-line cursors.
 *  @returns True if any boxes were drawn.
 */
int CPlotClass::search(double min_per,double max_per, bool do_largest_amp)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i, m, n, *is = NULL, num;
	AxesCursor *c;
	double sx1, sx2, left_side, right_side, top_side, bottom_side, period;
	float *amp = NULL, max_amp;
	CPlotWidget z;
	DataEntry *entry;

	if(cp->resize_measure_box >= 0) {
	    return 0;
	}
	for(i = 0; i < ax->num_cursors; i++) {
	    if(ax->cursor[i].type == AXES_VER_DOUBLE_LINE) break;
	}
	if(i < ax->num_cursors) {
	    c = &ax->cursor[i];
	}
	else {
	    c = NULL;
	}

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = w;
	}
	for(m = 0; m < z->c_plot.num_entries; m++)
	    if(z->c_plot.entry[m]->on && z->c_plot.entry[m]->w->selected)
	{
	    break;
	}
	if(m == z->c_plot.num_entries && z->c_plot.num_entries > 0) {
	    _AxesWarning((AxesWidget)w, "No waveforms selected.");
	    return -1;
	}

	n = 0;
	for(m = 0; m < z->c_plot.num_entries; m++)
	    if(z->c_plot.entry[m]->on && z->c_plot.entry[m]->w->selected
			&& z->c_plot.entry[m]->ts->length() > 0)
	{
	    GDataPoint *d1 = NULL, *d2 = NULL;
	    max_amp = 0.0;
	    entry = z->c_plot.entry[m];

	    if(c != NULL) {
		sx1 = c->scaled_x1 - entry->w->scaled_x0 + entry->tbeg;
		sx2 = c->scaled_x2 - entry->w->scaled_x0 + entry->tbeg;
	    }
	    else {
		sx1 = entry->tbeg;
		sx2 =  entry->ts->tend();
	    }

	    if((d1 = entry->ts->upperBound(sx1)) == NULL ||
	       (d2 = entry->ts->lowerBound(sx2)) == NULL ||
			d2->segmentIndex() != d1->segmentIndex())
	    {
		if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
		continue;
	    }

	    is = (int *)malloc(sizeof(int));
	    amp = (float *)malloc(sizeof(float));

	    for(num = 0, i = d1->index(); d1->index() <= d2->index();
		i += 2, d1->setIndex(i))
	    {
		if(GetBox(w, d1, &left_side, &right_side, &bottom_side,
			&top_side))
		{
		    is = (int *)realloc(is, (num+1)*sizeof(int));
		    is[num] = d1->index();

		    period = 2.*(right_side - left_side);
		    amp = (float *)realloc(amp, (num+1)*sizeof(float));
		    amp[num] = fabs(top_side - bottom_side);
		    if(period < min_per || period > max_per) {
			amp[num] = -amp[num];
		    }
		    num++;
		}
	    }
	    for(i = 0; i < num; i++) if(amp[i] > 0.) {
		d1->setIndex(is[i]);
		max_amp = amp[i];
		break;
	    }
	    if(i < num) {
		for(++i; i < num; i++) if(amp[i] > 0.)
		{
		    if(max_amp < amp[i]) {
			d1->setIndex(is[i]);
			max_amp = amp[i];
		    }
		}
	    }
	    else if(do_largest_amp)
	    {
		if(num > 0) {
		    d1->setIndex(is[0]);
		    max_amp = -amp[0];
		}
		for(i = 1; i < num; i++) {
		    if(max_amp < -amp[i]) {
			d1->setIndex(is[i]);
			max_amp = -amp[i];
		    }
		}
	    }

	    Free(is);
	    Free(amp);
	
	    DrawMeasureBoxes(z, entry);

	    if(max_amp > 0.)
	    {
		sx1 = entry->w->scaled_x0 + d1->time() - entry->tbeg;
		i = unscale_x(&z->axes.d, sx1);
		FindMeasureBoxes(z, i, entry);
		DrawMeasureBoxes(z, entry);
		DoAmpConvertCallback(w, 1, &entry);
		n++;
	    }
	    if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
	}
	return((n > 0) ? 1 : 0);
}

/** 
 *  
 */
static void
_CPlotMouseDownMeasure(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		cursor_x, cursor_y;
	Boolean		display_tags;
	DataEntry	*entry;
	CPlotWidget	z;

	if(event->type != ButtonPress) return;

	if(!cp->measure_box) {
	    cp->add_arrival_x = ((XButtonEvent *)event)->x;
	    cp->add_arrival_y = ((XButtonEvent *)event)->y;
	    cp->add_arrival_event = *(XButtonEvent *)event;
	    addArrivalCallback(w, true);
	    cp->add_arrival_x = -1;
	    cp->add_arrival_y = -1;
	    return;
	}

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;
	cp->measure_entry = NULL;

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}
	if(ax->x_axis == LEFT || cp->retime_arrival)
	{
	    return;
	}
	for(z = w; z->axes.mag_from != NULL; z = (CPlotWidget)z->axes.mag_from);

	if(XtHasCallbacks((Widget)z, XtNmeasureCallback) == XtCallbackHasNone) {
	    return;
	}
	/* ignore cursor and tags */
	display_tags = cp->display_tags;
	w->c_plot.display_tags = False;

	entry = WhichEntry(w, cursor_x, cursor_y);

	cp->display_tags = display_tags;
	if(entry != NULL && entry->ymin != entry->ymax)
	{
	    DrawMeasureBoxes(w, entry);
	    FindMeasureBoxes(w, cursor_x, entry);
	    DrawMeasureBoxes(w, entry);

	    DoAmpConvertCallback(w, 1, &entry);
	    DoMeasureCallback(w, 1, &entry);
	}
}

void CPlotClass::setCPlotArrivalKeys(vector<ArrivalKey> &keys)
{
    CPlotPart *cp = &cw->c_plot;
    cp->arrival_keys->clear();
    for(int i = 0; i < (int)keys.size(); i++) {
	cp->arrival_keys->push_back(keys[i]);
    }
}

static void
CPlotPopupMenu(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
    XtCallCallbacks((Widget)w, XtNmenuCallback, (XtPointer)event);
}

static void
CPlotKeyCommand(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
        CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	char str[10], phase[10];
	int i, len;
	KeySym key;
	XComposeStatus cs;

 	if(processKeyCommand(w, event, params, num_params)) return;

        if(XtHasCallbacks((Widget)w,XtNaddArrivalCallback) == XtCallbackHasNone)
	{
	    _AxesKeyCommand((AxesWidget)w, event, params, num_params);
	    return;
	}
	if(!cp->select_entry || (!cp->selecting && !cp->deselecting)
		|| cp->allow_partial_select)
	{
	    _AxesKeyCommand((AxesWidget)w, event, params, num_params);
	    return;
	}

	if(*num_params == 1) {
	    strcpy(str, "ctrl+");
	    strncpy(str+5, params[0], 1);
	    str[6] = '\0';
	}
	else {
	    len = XLookupString((XKeyEvent *)event, str, sizeof(str), &key,&cs);
	    if(len <= 0 || len >= (int)sizeof(str)) return;
	    str[len] = '\0';
	}

	if(!strncmp(str, cp->last_key_str, sizeof(cp->last_key_str)-1)) return;

	phase[0] = '\0';
	for(i = 0; i < (int)cp->arrival_keys->size(); i++) {
	    if(!cp->arrival_keys->at(i).key.compare(str)) {
		strncpy(phase, cp->arrival_keys->at(i).name.c_str(),
			sizeof(phase));
	    }
	}

	if(phase[0]  =='\0') {
	   fprintf(stderr, "Arrival key not found.\n");
	   return;
	}

	if(cp->adding_arrival) {
	    cp->cplot_class->renamePhaseLine(phase);
	    return;
	}
	int cursor_x = ((XButtonEvent *)event)->x;
	double x = scale_x(&ax->d, cursor_x);
	int ix1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
	int ix2 = unscale_x(&ax->d, ax->x2[ax->zoom]);

	if(ix1 > ix2) {
	    i = ix1;
	    ix1 = ix2;
	    ix2 = i;
	}

	if(x <= ix1 && x >= ix2) return;

	if(cp->deselecting) {
	    Select(w, cp->select_entry, False);
	}

	cp->cplot_class->positionPhaseLine(phase, x, False);

	if(ax->num_cursors <= 0) return;
	ax->m = ax->num_cursors-1;
	ax->cursor_mode = AXES_SELECT_CURSOR;
	_AxesSetCursor((AxesWidget)w, "move");
	cp->adding_arrival = True;
}

static Boolean
processKeyCommand(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	char str[10], proc[100], *c, *last;
	String p[2];
	Cardinal num;
	int i, j, len, num_key_procs;
	KeySym key;
	XComposeStatus cs;

	if(*num_params == 1) {
	    strcpy(str, "ctrl+");
	    strncpy(str+5, params[0], 1);
	    str[6] = '\0';
	}
	else {
	    len = XLookupString((XKeyEvent *)event, str, sizeof(str), &key,&cs);
	    if(key == XK_Up) {
		strcpy(str, "arrow_up");
	    }
	    else if(key == XK_Down) {
		strcpy(str, "arrow_down");
	    }
	    else if(key == XK_Left) {
		strcpy(str, "arrow_left");
	    }
	    else if(key == XK_Right) {
		strcpy(str, "arrow_right");
	    }
	    else if(key == XK_Page_Up) {
		strcpy(str, "page_up");
	    }
	    else if(key == XK_Page_Down) {
		strcpy(str, "page_down");
	    }
	    else if(len > 0 && len < (int)sizeof(str)) {
		str[len] = '\0';
	    }
	    else {
		return False;
	    }
	}
  	num_key_procs = sizeof(key_procs)/sizeof(CPlotKeyProc);
	for(i = 0; i < num_key_actions; i++) {
	    if(!strcmp(key_actions[i].key_code, str)) {
		strncpy(proc, key_actions[i].proc_call, sizeof(proc));
		if( (c = strtok_r(proc, "(", &last)) ) {
		    for(j = 0; j < num_key_procs &&
			strcmp(c, key_procs[j].proc_name); j++);
		    if(j == num_key_procs) {
			fprintf(stderr, "processKeyCommand: %s not found.\n",c);
			return True;
		    }
		    num = 0;
		    while( num < 2 && (c = strtok_r(NULL, " )", &last)) ) {
			p[num++] = c;
		    }
		    (*key_procs[j].proc)((Widget)w, event, p, &num);
		}

		return True;
	    }
	}
	return False;
}

static void
ZCompOnly(CPlotWidget w, XEvent *event)
{
	w->c_plot.key_modifier = True;
	Components(w, event, 0);
}
static void
ECompOnly(CPlotWidget w, XEvent *event)
{
	w->c_plot.key_modifier = True;
	Components(w, event, 1);
}
static void
NCompOnly(CPlotWidget w, XEvent *event)
{
	w->c_plot.key_modifier = True;
	Components(w, event, 2);
}
static void
AllComp(CPlotWidget w, XEvent *event)
{
	w->c_plot.key_modifier = True;
	Components(w, event, 3);
}
static void
Components(CPlotWidget w, XEvent *event, int key)
{
	AxesPart *ax = &w->axes;
	int i, cursor_x, cursor_y;
	Boolean redo_mag = False;
	double dy, yscale, scaled_y0;
	DataEntry *e, *f;
	CPlotWidget mag;

	if(ax->mag_from != NULL) return;

	AxesWaitCursor((Widget)w, True);
	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;
	if((f = WhichEntry(w, cursor_x, cursor_y)) != NULL)
	{
	    if(key >= 0 && key < NUM_COMPONENTS)
	    {
		if(f->comps[key] == NULL) {
		    AxesWaitCursor((Widget)w, False);
		    return;
		}
		for(i = 0; i < NUM_COMPONENTS; i++)
		    if(i != key && f->comps[i] != NULL)
		{
		    _CPlotRedisplayEntry(w, f->comps[i]);
		    f->comps[i]->on = False;
		    f->comps[i]->w->selected = False;
		    redo_mag = True;
		}
		e = f->comps[key];
		if(e != f)
		{
		    _CPlotRedisplayEntry(w, e);
		    e->w->scaled_x0 = f->w->scaled_x0 + e->tbeg - f->tbeg;
		    e->w->scaled_y0 = f->w->scaled_y0;
		    e->yscale = f->yscale;
		    e->on = True;
		    _CPlotRedrawEntry(w, e);
		    _CPlotRedisplayEntry(w, e);
		    redo_mag = True;
		}
	    }
	    else if(key == NUM_COMPONENTS)
	    {
		yscale =  1.5*(ax->y2[ax->zoom] -
				ax->y1[ax->zoom])/(float)w->core.height;
		dy = (ax->reverse_y) ? - f->tag_height*yscale
					: f->tag_height*yscale;
		scaled_y0 = f->w->scaled_y0;

		for(i = 0; i < NUM_COMPONENTS; i++) if(f->comps[i] != NULL)
		{
		    e = f->comps[i];
		    _CPlotRedisplayEntry(w, e);
		    e->w->scaled_x0 = f->w->scaled_x0 + e->tbeg - f->tbeg;
		    e->w->scaled_y0 = scaled_y0;
		    scaled_y0 -= dy;
		    e->yscale = f->yscale;
		    e->on = True;
		    _CPlotRedrawEntry(w, e);
		    _CPlotRedisplayEntry(w, e);
		    redo_mag = True;
		}
	    }
	}
	if(redo_mag && (mag = (CPlotWidget)ax->mag_to) != NULL)
	{
	    CPlotMagnifyWidget(w, NULL, True);
	    CPlotMagnifyWidget(w, mag, True);
	}
	AxesWaitCursor((Widget)w, False);
}

static void
FindMeasureBoxes(CPlotWidget w, int cursor_x, DataEntry *entry)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		k, ytop, ybottom, entry_y0;
	double		d, time, right_side, top_side, half_period;
	GDataPoint	*dp = NULL;
	CPlotWidget	z;

	entry->n_measure_segs = 0;

	time = scale_x(&ax->d, cursor_x) - entry->w->scaled_x0 + entry->tbeg;

	if((dp = entry->ts->nearest(time)) == NULL) return;

	if(!GetBox(w, dp, &entry->left_side, &right_side,
		&entry->bottom_side, &top_side))
	{
	    if(dp) dp->deleteObject();
	    return;
	}
	if(dp) dp->deleteObject();
	entry->zp_ts = entry->bottom_side;

	half_period = right_side - entry->left_side;

	if(cp->three_half_cycles) {
	    right_side -= half_period;
	    entry->left_side -= half_period;
	}
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	ytop = entry_y0 - (int)(entry->yscale*(top_side - entry->mean));
	ybottom = entry_y0 -
		(int)(entry->yscale*(entry->bottom_side - entry->mean));

	if(ytop > ybottom)
	{
	    k = ytop;
	    ytop = ybottom;
	    ybottom = k;
	    d = top_side;
	    top_side = entry->bottom_side;
	    entry->bottom_side = d;
	}
	entry->period = 2.*(right_side - entry->left_side);
	entry->amp_ts = top_side - entry->bottom_side;
	entry->amp_cnts = -1;
	entry->amp_nms = -1;

	entry->n_measure_segs = (cp->three_half_cycles) ? 6 : 5;

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}
	if(z != NULL)
	{
	    DataEntry *ze = (DataEntry *)entry->mag_entry;
	    ze->amp_cnts	= entry->amp_cnts;
	    ze->amp_nms		= entry->amp_nms;
	    ze->amp_ts		= entry->amp_ts;
	    ze->zp_ts		= entry->zp_ts;
	    ze->period		= entry->period;
	    ze->n_measure_segs	= entry->n_measure_segs;
	    ze->left_side	= entry->left_side;
	    ze->bottom_side	= entry->bottom_side;
	}

	for(z = w; z->axes.mag_from != NULL;
			z = (CPlotWidget)z->axes.mag_from);
}

static double
getAmpNnms(DataEntry *entry, double amp)
{
	if(entry->ts->getMethod("CalibData"))
	{
	    /* The GTimeSeries is counts*calib (Nnms), so return amp.
	     */
	    return amp;
	}
	else
	{
	    double tbeg = entry->ts->tbeg();
	    double amp_time = tbeg + entry->left_side;
	    GSegment *s = entry->ts->nearestSegment(amp_time);
	    double calib = (s->calib() != 0.) ? s->calib() : 1.;
	    /* The GTimeSeries is counts. Multiply by calib to get Nnms.
	     */
	    return amp*calib;
	}
}

static double
getAmpTs(DataEntry *entry, double ampNnms)
{
	if(entry->ts->getMethod("CalibData"))
	{
	    /* The GTimeSeries is counts*calib (Nnms), so return ampNnms.
	     */
	    return ampNnms;
	}
	else
	{
	    double tbeg = entry->ts->tbeg();
	    double amp_time = tbeg + entry->left_side;
	    GSegment *s = entry->ts->nearestSegment(amp_time);
	    double calib = (s->calib() != 0.) ? s->calib() : 1.;
	    /*The GTimeSeries is counts. Divide amp Nnms by calib to get counts.
	     */
	    return ampNnms/calib;
	}
}

static Boolean
GetBox(CPlotWidget w, GDataPoint *dp, double *left_side,
	double *right_side, double *bottom_side, double *top_side)
{
#ifdef HAVE_GSL
	CPlotPart *cp = &w->c_plot;
	int i, j, n;
	Boolean up, going_up=False;
	GSegment *s;
	float *y, spline_x, spline_y, dx, sx, sy;
	double x[11], yy[11];
	double tbeg = dp->timeSeries()->tbeg();

	s = dp->timeSeries()->segment(dp->segmentIndex());
	y = s->data;

	n = s->length();
	for(i = dp->index()+1; i < n; i++)
	{
	    if(y[i] != y[dp->index()]) {
		going_up = (y[i] > y[dp->index()]) ? True : False;
		break;
	    }
	}
	for(up = going_up; i < n; i++)
	{
	    if(y[i] != y[i-1])
	    {
		up = (y[i] > y[i-1]) ? True : False;
		if(up != going_up) {
		    if(i > 0) i--;
		    break;
		}
	    }
	}
	if(i >= s->length())
	{
	    return(False);
	}
	dp->setIndex(i);
	*right_side = dp->time() - tbeg;
	*top_side = y[i];
	if(cp->amp_interpolation && i > 4 && i < s->length()-5)
	{
	    gsl_spline *spline = gsl_spline_alloc(gsl_interp_cspline, 11);
	    gsl_interp_accel *acc = gsl_interp_accel_alloc();

	    /* cubic spline interpolation */
	    for(j = 0; j < 11; j++) {
		x[j] = dp->segment()->time(i-5+j) - tbeg;
		yy[j] = (double)y[i-5+j];
	    }
	    gsl_spline_init(spline, x, yy, 11);

	    spline_x = x[5];
	    spline_y = y[i];
	    dx = (x[6] - x[5])/100;
	    for(j = 1; j <= 100; j++)
	    {
		sx = x[5] + j*dx;
		sy = gsl_spline_eval(spline, sx, acc);
		if((going_up && sy < spline_y) || (!going_up && sy > spline_y))
		    break;
		spline_x = sx;
		spline_y = sy;
	    }

	    /* use a data value.
	     * top_side = spline_y;
	     */
	    *top_side = y[i];
	    *right_side = spline_x;
	    if(j == 1)
	    {
		spline_x = x[5];
		spline_y = y[i];
		dx = (x[5] - x[4])/100;
		for(j = 1; j <= 100; j++)
		{
		    sx = x[5] - j*dx;
		    sy = gsl_spline_eval(spline, sx, acc);
		    if( ( going_up && sy < spline_y) ||
			(!going_up && sy > spline_y)) break;
		    spline_x = sx;
		    spline_y = sy;
		}
		/* use a data value.
		 * top_side = spline_y;
		 */
		*top_side = y[i];
		*right_side = spline_x;
	    }
	    gsl_spline_free(spline);
	    gsl_interp_accel_free(acc);
	}
	for(i = dp->index()-1, up = going_up; i >= 0; i--)
	{
	    if(y[i+1] != y[i])
	    {
		up = (y[i+1] > y[i]) ? True : False;
		if(up != going_up) {
		    i++;
		    break;
		}
	    }
	}
	if(i < 0) return(False);

	*left_side = dp->segment()->time(i) - tbeg;
	*bottom_side = y[i];
	if(cp->amp_interpolation && i > 4 && i < s->length()-5)
	{
	    gsl_spline *spline = gsl_spline_alloc(gsl_interp_cspline, 11);
	    gsl_interp_accel *acc = gsl_interp_accel_alloc();

	    /* cubic spline interpolation */
	    for(j = 0; j < 11; j++) {
		x[j] = dp->segment()->time(i-5+j) - tbeg;
		yy[j] = (double)y[i-5+j];
	    }
	    gsl_spline_init(spline, x, yy, 11);

	    spline_x = x[5];
	    spline_y = y[i];
	    dx = (x[6] - x[5])/100;
	    for(j = 1; j <= 100; j++)
	    {
		sx = x[5] + j*dx;
		sy = gsl_spline_eval(spline, sx, acc);
		if(( going_up && sy > spline_y) ||
		   (!going_up && sy < spline_y)) break;
		spline_x = sx;
		spline_y = sy;
	    }
	    *left_side = spline_x;
	    /* use a data value.
	     * top_side = spline_y;
	     */
	    *bottom_side = y[i];
	    if(j == 1)
	    {
		spline_x = x[5];
		spline_y = y[i];
		dx = (x[5] - x[4])/100;
		for(j = 1; j <= 100; j++)
		{
		    sx = x[5] - j*dx;
		    sy = gsl_spline_eval(spline, sx, acc);
		    if( ( going_up && sy > spline_y) ||
			(!going_up && sy < spline_y)) break;
		    spline_x = sx;
		    spline_y = sy;
		}
		*left_side = spline_x;
		/* use a data value.
		 * bottom_side = spline_y;
		 */
		*bottom_side = y[i];
	    }
	    gsl_spline_free(spline);
	    gsl_interp_accel_free(acc);
	}
	return(True);
#else
	fprintf(stderr, "Operation unavailable without libgsl.\n");
	return false;
#endif
}

static void
DrawMeasureBoxes(CPlotWidget w, DataEntry *entry)
{
	AxesPart *ax = &w->axes;
	CPlotWidget z;

	DrawMeasureBox(w, entry);

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}
	if(z != NULL) {
	    DrawMeasureBox(z, (DataEntry *)entry->mag_entry);
	}
}

static void
DrawMeasureBox(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int xleft, xright, ytop, ybottom, width, len, entry_y0;
	XSegment *s;

	if(!entry->w->visible || entry->n_measure_segs<=0 || entry->w == NULL)
	{
		return;
	}
	xleft = unscale_x(&ax->d, entry->w->scaled_x0 + entry->left_side);
	xright= unscale_x(&ax->d, entry->w->scaled_x0 + entry->left_side
				+ .5*entry->period);
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	ybottom = entry_y0 -
		(int)(entry->yscale*(entry->bottom_side - entry->mean));
	ytop    = entry_y0 - (int)(entry->yscale*(entry->bottom_side
				- entry->mean + entry->amp_ts));

	s = entry->measure_segs;

	width = xright - xleft;
	len = (entry->n_measure_segs == 6) ? 2*width : width;
	s[0].x1 = xleft;	s[0].y1 = ybottom;
	s[0].x2 = xright + len; s[0].y2 = ybottom;

	s[1].x1 = xleft;        s[1].y1 = ytop;
	s[1].x2 = xright + len; s[1].y2 = ytop;

	s[2].x1 = xleft;  s[2].y1 = ytop + 1;
	s[2].x2 = xleft;  s[2].y2 = ybottom - 1;

	s[3].x1 = xright; s[3].y1 = ytop + 1;
	s[3].x2 = xright; s[3].y2 = ybottom - 1;
	
	s[4].x1 = xright + width;  s[4].y1 = ytop + 1;
	s[4].x2 = xright + width;  s[4].y2 = ybottom - 1;

	if(entry->n_measure_segs == 6) {
	    s[5].x1 = xright + 2*width; s[5].y1 = ytop + 1;
	    s[5].x2 = xright + 2*width; s[5].y2 = ybottom - 1;
	}

	SetClipRects(w, entry);

	XSetPlaneMask(XtDisplay(w), cp->invertGC, w->axes.fg ^
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), cp->invertGC, w->axes.fg);
	if(!ax->use_pixmap || ax->use_screen)
	{
	    XDrawSegments(XtDisplay(w), XtWindow(w), cp->invertGC, s,
				entry->n_measure_segs);
	}
	if(ax->use_pixmap)
	{
	    XDrawSegments(XtDisplay(w), ax->pixmap, cp->invertGC, s,
				entry->n_measure_segs);
	}
}

static void
DoMeasureCallback(CPlotWidget w, int num, DataEntry **entry)
{
	int		 i;
	CPlotMeasure	*m = NULL;
	CPlotWidget	 z;
	
	// printf("DEBUG: in CPlot->DoMeasureCallback\n");

	m = (CPlotMeasure *)AxesMalloc((Widget)w, (num+1)*sizeof(CPlotMeasure));

	for(i = 0; i < num; i++)
	{
	    m[i].w = entry[i]->w;
	    m[i].amp_Nnms = getAmpNnms(entry[i], entry[i]->amp_ts);
	    m[i].amp_cnts = entry[i]->amp_cnts;
	    m[i].amp_nms = entry[i]->amp_nms;
	    m[i].zp_Nnms = getAmpNnms(entry[i], entry[i]->zp_ts);
	    m[i].period = entry[i]->period;
	    m[i].left_side = entry[i]->left_side;
	    m[i].bottom_side = getAmpNnms(entry[i], entry[i]->bottom_side);
	}
	m[num].w = NULL;

	for(z = w; z->axes.mag_from != NULL; z = (CPlotWidget)z->axes.mag_from);
	XtCallCallbacks((Widget)z, XtNmeasureCallback, m);

	for(i = 0; i < num; i++) {
	    entry[i]->amp_cnts = m[i].amp_cnts;
	    entry[i]->amp_nms = m[i].amp_nms;
	}
	Free(m);
}

static void
DoAmpConvertCallback(CPlotWidget w, int num, DataEntry **entry)
{
	int		 i;
	CPlotMeasure	*m = NULL;
	CPlotWidget	 z;
	
	m = (CPlotMeasure *)AxesMalloc((Widget)w, (num+1)*sizeof(CPlotMeasure));

	for(i = 0; i < num; i++)
	{
	  // printf("DEBUG 1 amp_nms = %f\n", m[i].amp_nms);
	    m[i].w = entry[i]->w;
	    m[i].amp_Nnms = getAmpNnms(entry[i], entry[i]->amp_ts);
	    m[i].amp_cnts = entry[i]->amp_cnts;
	    m[i].amp_nms = entry[i]->amp_nms;
	    m[i].zp_Nnms = getAmpNnms(entry[i], entry[i]->zp_ts);
	    m[i].period = entry[i]->period;
	    m[i].left_side = entry[i]->left_side;
	    m[i].bottom_side = getAmpNnms(entry[i], entry[i]->bottom_side);
	 // printf("DEBUG 2 amp_nms = %f\n", m[i].amp_nms);
	}
	m[num].w = NULL;

	for(z = w; z->axes.mag_from != NULL; z = (CPlotWidget)z->axes.mag_from);
	XtCallCallbacks((Widget)z, XtNampConvertCallback, m);

	for(i = 0; i < num; i++)
	{
	  // printf("DEBUG 3 amp_nms = %f\n", m[i].amp_nms);
	    entry[i]->amp_cnts = m[i].amp_cnts;
	    entry[i]->amp_nms = m[i].amp_nms;
	}
	Free(m);
}

/** 
 *  Get a measurement box on a waveform that is associated with an arrival.
 *  Returns the measurement box currently displayed on a waveform that is
 *  associated with the specified arrival. If more than one waveform that is
 *  associated with the arrival has a measurement box, then the box
 *  information return will be for the last measurement box found.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival A CssArrivalClass associated with one or more waveforms in
 *			this CPlotWidget.
 *  @param[out] m A CPlotMeasure structure that will be filled.
 *  @returns The number of boxes found on waveforms associated with
 *		the specified arrival.
 *  @see CPlotMeasure
 */
int CPlotClass::getArrivalMeasureBox(CssArrivalClass *arrival, CPlotMeasure *m)
{
	CPlotWidget w = cw;
	CPlotPart *cp = &w->c_plot;
	int	  i, j, n;
	DataEntry *entry;

	/* look for entries with arrival and a measure box
	 */
	for(i = n = 0; i < cp->num_entries; i++)
	    if(cp->entry[i]->n_measure_segs > 0 &&
		(!strcasecmp(cp->entry[i]->ts->sta(), arrival->sta) ||
		 !strcasecmp(cp->entry[i]->ts->net(), arrival->sta))
		&& cp->entry[i]->tbeg <= arrival->time
		&& cp->entry[i]->ts->tend() >= arrival->time)
	{
	    entry = cp->entry[i];
	    for(j = 0; j < entry->n_arrlab; j++)
	    {
		if(entry->arrlab[j].a_entry->a == arrival)
		{
		    n++;
		    m->w = entry->w;
		    m->amp_Nnms = getAmpNnms(entry, entry->amp_ts);
		    m->amp_cnts = entry->amp_cnts;
		    m->amp_nms = entry->amp_nms;
		    m->zp_Nnms = getAmpNnms(entry, entry->zp_ts);
		    m->period = entry->period;
		    m->left_side = entry->left_side;
		    m->bottom_side = getAmpNnms(entry, entry->bottom_side);
		}
	    }
	}
	return(n);
}

Waveform * CPlotClass::getWaveform(int id)
{
    CPlotPart *cp = &cw->c_plot;
    for(int i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w->getId() == id) return cp->entry[i]->w;
    }
    return NULL;
}

/** 
 *  Get the measurement box information for the specified  waveform.  If a
 *  measurement box is displayed on the specified waveform, its dimensions
 *  are returned in the CPlotMeasure structure.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd A Waveform pointer for a waveform contained in this
 *  CPlotWidget.
 *  @param[out] m A CPlotMeasure structure that will be filled.
 *  @returns 1, if the waveform has a measurement box, otherwise returns 0.
 *  @see CPlotMeasure
 */
bool CPlotClass::getWaveformMeasureBox(Waveform *cd, CPlotMeasure *m)
{
    CPlotPart *cp = &cw->c_plot;
    int i;

    for(i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w == cd) break;
    }
    if(i == cp->num_entries) return false;

    if(cp->entry[i]->n_measure_segs > 0)
    {
	m->w = cp->entry[i]->w;
	m->amp_Nnms = getAmpNnms(cp->entry[i], cp->entry[i]->amp_ts);
	m->amp_cnts = cp->entry[i]->amp_cnts;
	m->amp_nms = cp->entry[i]->amp_nms;
	m->zp_Nnms = getAmpNnms(cp->entry[i], cp->entry[i]->zp_ts);
	m->period = cp->entry[i]->period;
	m->left_side = cp->entry[i]->left_side;
	m->bottom_side = getAmpNnms(cp->entry[i],cp->entry[i]->bottom_side);
	return true;
    }
    return false;
}

void CPlotClass::reviewMeasurements(bool selected, CssArrivalClass *arrival)
{
	CPlotWidget	w = cw;
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, k, num;
	DataEntry	**entry_list = NULL;
	ArrivalEntry	*arr_entry = NULL;
	CPlotWidget	z;

	if(!selected)
	{
	    for(i = 0; i < cp->v->arrival_entries.size(); i++)
	    {
	        arr_entry = cp->v->arrival_entries[i];
		if(arr_entry->a == arrival) break;
	    }
	    if(i == cp->v->arrival_entries.size()) return;
	}
	entry_list = (DataEntry **)AxesMalloc((Widget)w,
			cp->num_entries*sizeof(DataEntry *));

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}

	for(k = num = 0; k < cp->v->arrival_entries.size(); k++)
	{
	    ArrivalEntry *a_entry = cp->v->arrival_entries[k];
	    CssArrivalClass *a = a_entry->a;
	    if((!selected && a_entry == arr_entry) ||
		(selected && a_entry->selected))
	    {
	        for(i = 0; i < cp->num_entries; i++)
		    if((!strcasecmp(cp->entry[i]->ts->sta(), a->sta) ||
			!strcasecmp(cp->entry[i]->ts->net(), a->sta))
			&& cp->entry[i]->tbeg <= a->time
			&& cp->entry[i]->ts->tend() >= a->time)
		{
		    DataEntry *entry = cp->entry[i];
		    DataEntry *ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

		    for(j = 0; j < entry->n_arrlab; j++)
			if(entry->arrlab[j].a_entry == a_entry)
		    {
			removeMeasureBox(cp->entry[i]->w);
	
			if(a->amp_Nnms < 0. || a->period < 0.)
			{
				continue;
			}
			entry->amp_ts = getAmpTs(entry, a->amp_Nnms);
			entry->amp_cnts = a->amp_cnts;
			entry->amp_nms = a->amp_nms;
			entry->period = a->period;
			entry->zp_ts = getAmpTs(entry, a->zp_Nnms);
			if(a->box_location) {
			    entry->left_side = a->boxtime - entry->tbeg;
			    entry->bottom_side = a->boxmin;
			}
			else {
			    entry->left_side = a->time - entry->tbeg;
			    entry->bottom_side = -.5*entry->amp_ts;
			}

			entry->n_measure_segs = cp->three_half_cycles ?  6 : 5;

			if(ze != NULL)
			{
			    ze->amp_ts = entry->amp_ts;
			    ze->amp_cnts = entry->amp_cnts;
			    ze->amp_nms = entry->amp_nms;
			    ze->zp_ts = entry->zp_ts;
			    ze->amp_ts = entry->amp_ts;
			    ze->period = entry->period;
			    ze->left_side = entry->left_side;
			    ze->bottom_side = entry->bottom_side;
			    ze->n_measure_segs = entry->n_measure_segs;
			}
			DrawMeasureBoxes(w, entry);

			entry_list[num++] = entry;
			break;
		    }
		}
	    }
	}
	DoMeasureCallback(w, num, entry_list);

	Free(entry_list);
}

void CPlotClass::reviewMeasurementOnWaveform(CssAmplitudeClass *amplitude,
		Waveform *cd)
{
	CPlotWidget 	w = cw;
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	CPlotWidget	z;

	if(amplitude->amp_Nnms < 0. || amplitude->per < 0.) return;

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}

	for(i = 0; i < cp->num_entries && cd != cp->entry[i]->w; i++);

	if(i < cp->num_entries)
	{
	    DataEntry *entry = cp->entry[i];
	    DataEntry *ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

	    removeMeasureBox(cp->entry[i]->w);

	    entry->amp_ts = getAmpTs(entry, amplitude->amp_Nnms);
	    entry->amp_cnts = amplitude->amp_cnts;
	    entry->amp_nms = amplitude->amp_nms;
	    entry->period = amplitude->per;
	    entry->zp_ts = getAmpTs(entry, amplitude->zp_Nnms);
	    if(amplitude->box_location) {
		entry->left_side = amplitude->boxtime - entry->tbeg;
		entry->bottom_side = amplitude->boxmin;
	    }
	    else {
		entry->left_side = amplitude->amptime - entry->tbeg;
		entry->bottom_side = -.5*entry->amp_ts;
	    }

	    entry->n_measure_segs = cp->three_half_cycles ?  6 : 5;

	    if(ze != NULL)
	    {
		ze->amp_ts = entry->amp_ts;
		ze->amp_cnts = entry->amp_cnts;
		ze->amp_nms = entry->amp_nms;
		ze->zp_ts = entry->zp_ts;
		ze->amp_ts = entry->amp_ts;
		ze->period = entry->period;
		ze->left_side = entry->left_side;
		ze->bottom_side = entry->bottom_side;
		ze->n_measure_segs = entry->n_measure_segs;
	    }
	    DrawMeasureBoxes(w, entry);

	    DoMeasureCallback(w, 1, &entry);
	}
}

void CPlotClass::addMeasurement(CPlotMeasure *m, Waveform *cd)
{
	CPlotWidget	w = cw;
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	CPlotWidget	z;

	if(m->amp_Nnms < 0. || m->period < 0.) return;

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}

	for(i = 0; i < cp->num_entries && cd != cp->entry[i]->w; i++);

	if(i < cp->num_entries)
	{
	    DataEntry *entry = cp->entry[i];
	    DataEntry *ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

	    removeMeasureBox(cp->entry[i]->w);
	
	    entry->amp_ts = getAmpTs(entry, m->amp_Nnms);
	    entry->amp_cnts = m->amp_cnts;
	    entry->amp_nms = m->amp_nms;
	    entry->period = m->period;
	    entry->zp_ts = getAmpTs(entry, m->zp_Nnms);
	    entry->left_side = m->left_side;
	    entry->bottom_side = m->bottom_side;
	    entry->n_measure_segs = cp->three_half_cycles ?  6 : 5;
	    DoAmpConvertCallback(w, 1, &entry);

	    if(ze != NULL)
	    {
		ze->amp_ts = entry->amp_ts;
		ze->amp_cnts = entry->amp_cnts;
		ze->amp_nms = entry->amp_nms;
		ze->zp_ts = entry->zp_ts;
		ze->amp_ts = entry->amp_ts;
		ze->period = entry->period;
		ze->left_side = entry->left_side;
		ze->bottom_side = entry->bottom_side;
		ze->n_measure_segs = entry->n_measure_segs;
	    }
	    DrawMeasureBoxes(w, entry);
	    DoMeasureCallback(w, 1, &entry);
	}
}

/** 
 *  Get the Waveform structures for all waveforms in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[out] wvec Pointer to the address of an array of Waveform
 *	pointers. Space for this array is allocated by this function and
 *	should be freed when nolonger needed. Note that the members of the
 *	Waveform structures should not be altered, since they continue
 *	to be used by the CPlotWidget.
 *  @param displayed_only If True, only Waveform's for waveforms that are
 *	displayed are returned. A waveform can be hidden by
 *	displayComponents() or displayData().
 *  @returns The number of Waveform pointers in wvec.
 *  @see displayData
 *  @see displayComponents
 *  @see CPlotGetSelected
 *  @see CPlotGetComponentGroup
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedData
 *  @see CPlotGetWindowedComponents
 */
int CPlotClass::getWaveforms(gvector<Waveform *> &wvec, bool displayed_only)
{
    CPlotPart *cp = &cw->c_plot;

    wvec.clear();
    for(int i = 0; i < cp->num_entries; i++)
	if(!displayed_only || cp->entry[i]->on)
    {
	wvec.push_back(cp->entry[i]->w);
	FillCursorStructs(cw, cp->entry[i]);
    }
    wvec.sort(sort_cd_by_y0);
    return wvec.size();
}

/** 
 *  Get the Waveform structures for all selected waveforms in a
 *  CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[out] wvec Pointer to the address of an array of Waveform
 *	pointers. Space for this array is allocated by this function and
 *	should be freed when nolonger needed. Note that the members of the
 *	Waveform structures should not be altered, since they continue
 *	to be used by the CPlotWidget.
 *  @returns The number of Waveform pointers in wvec.
 *  @see CPlotGetWaveforms
 *  @see CPlotGetComponentGroup
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedData
 *  @see CPlotGetWindowedComponents
 */
int CPlotClass::getSelectedWaveforms(gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &cw->c_plot;

    wvec.clear();
    for(int i = 0; i < cp->num_entries; i++) if(cp->entry[i]->w->selected)
    {
	wvec.push_back(cp->entry[i]->w);
	FillCursorStructs(cw, cp->entry[i]);
    }
    wvec.sort(sort_cd_by_y0);
    return wvec.size();
}

/**
 *  Return the number of selected waveforms in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @returns The number of selected waveforms.
 *  @see CPlotGetSelected
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedData
 *  @see CPlotGetWindowedComponents
 */
int CPlotClass::numSelected()
{
    CPlotPart *cp = &cw->c_plot;
    int n;

    for(int i = n = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w->selected) n++;
    }
    return n;
}

int CPlotClass::numWaveforms()
{
    return cw->c_plot.num_entries;
}

/** 
 *  Get Waveform structures for stations that have selected components.
 *  Get the Waveform structures for selected waveforms that match the
 *  specified set of components. The component list can be one to three
 *  characters from "zn(r)e(t)". 'n' and 'r' are interchangeable and 'e' and
 *  't' are interchangeable, and the letters can be lower case or upper case. If
 *  waveforms for all specifed components are selected for a station, then
 *  Waveform pointers for those components are returned, in the same order
 *  as the input component list. For example, if the following waveforms are
 *  present and have the selected state indicated
 *  <pre>
 *  DBIC/BHE	selected
 *  DBIC/BHN	not selected
 *  DBIC/BHZ	not selected
 *
 *  KBZ/SHE	selected
 *  KBZ/SHN	selected
 *  KBZ/SHZ 	selected
 *
 *  BGCA/BHr	selected
 *  BGCA/BHt	selected
 *   </pre>
 *  and the component list is "ne", then Waveform pointers for
 *  <TT>KBZ/SHN</TT>, <TT>KBZ/SHE</TT>, <TT>BGCA/SHr</TT> and <TT>BGCA/SHt</TT>
 *  will be returned.
 *  
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] component_list A one two three character string specifying the
 *  			components requested.
 *  @param[out] wvec Pointer to the address of an array of Waveform
 *	pointers. Space for this array is allocated by this function and
 *	should be freed when nolonger needed. Note that the members of the
 *	Waveform structures should not be altered, since they continue
 *	to be used by the CPlotWidget.
 *  @returns The number of Waveform pointers in wvec.
 *  @see CPlotGetWaveforms
 *  @see CPlotGetSelected
 *  @see CPlotGetComponentGroup
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedData
 *  @see CPlotGetWindowedComponents
 */
int CPlotClass::getSelectedComponents(const string &component_list,
			gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j, num_cmpnts, cmpnts[NUM_COMPONENTS];

    wvec.clear();
    num_cmpnts = (int)component_list.length();
    if(num_cmpnts > NUM_COMPONENTS) num_cmpnts = NUM_COMPONENTS;

    for(i = 0; i < num_cmpnts; i++)
    {
	for(j = 0; j < NUM_COMPONENTS; j++)
	{
	    if( component_list[i] == clist[j] ||
		component_list[i] == Clist[j] ||
		component_list[i] == rlist[j] ||
		component_list[i] == Rlist[j] ||
		component_list[i] == vlist[j] ||
		component_list[i] == nlist[j]) break;
	}
	if(j == NUM_COMPONENTS) return(-1);
	cmpnts[i] = j;
    }

    for(i = 0; i < cp->num_entries; i++) cp->entry[i]->matched = False;

    for(i = 0; i < cp->num_entries; i++) if(!cp->entry[i]->matched)
    {
	DataEntry *entry = cp->entry[i];
	for(j = 0; j < num_cmpnts; j++) {
	    DataEntry *e = entry->comps[cmpnts[j]];
	    if(e == NULL || !e->on || !e->w->selected || e->matched) break;
	}
	if(j < num_cmpnts) continue;

	for(j = 0; j < num_cmpnts; j++) {
	    DataEntry *e = entry->comps[cmpnts[j]];
	    e->matched = True;
	    wvec.push_back(e->w);
	    FillCursorStructs(cw, e);
	}
    }
    return wvec.size();
}

int CPlotClass::getSelectedComponents(vector<int> &num,
			gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j;

    num.clear();
    wvec.clear();
    for(i = 0; i < cp->num_entries; i++) cp->entry[i]->matched = False;

    for(i = 0; i < cp->num_entries; i++) if(!cp->entry[i]->matched)
    {
	DataEntry *entry = cp->entry[i];
	// check that no components are already in wvec[].
	for(j = 0; j < 3 && (!entry->comps[j]
			|| !entry->comps[j]->matched); j++);
	if(j < 3) continue;

	// check that at least one component is selected
	for(j = 0; j < 3 && (!entry->comps[j]
			|| !entry->comps[j]->w->selected); j++);
	if(j == 3) continue;

	// check that at least the first two components (x,y) exist
	if(!entry->comps[0] || !entry->comps[0]->on ||
	   !entry->comps[1] || !entry->comps[1]->on) continue;

	for(j = 0; j < 3; j++) {
	    DataEntry *e = entry->comps[j];
	    if(!e || !e->on) break;
	    e->matched = True;
	    wvec.push_back(e->w);
	    FillCursorStructs(cw, e);
	}
	num.push_back(j);
    }
    return num.size();
}

int CPlotClass::getComponents(vector<int> &num, gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j;

    num.clear();
    wvec.clear();
    for(i = 0; i < cp->num_entries; i++) cp->entry[i]->matched = False;

    for(i = 0; i < cp->num_entries; i++) if(!cp->entry[i]->matched)
    {
	DataEntry *entry = cp->entry[i];
	// check that no components are already in wvec[].
	for(j = 0; j < 3 && (!entry->comps[j]
			|| !entry->comps[j]->matched); j++);
	if(j < 3) continue;

	// check that at least the first two components (x,y) exist
	if(!entry->comps[0] || !entry->comps[0]->on ||
	   !entry->comps[1] || !entry->comps[1]->on) continue;

	for(j = 0; j < 3; j++) {
	    DataEntry *e = entry->comps[j];
	    if(!e || !e->on) break;
	    e->matched = True;
	    wvec.push_back(e->w);
	    FillCursorStructs(cw, e);
	}
	num.push_back(j);
    }
    return num.size();
}

int CPlotClass::getSelectedComponents(const string &labels, vector<int> &num,
				gvector<Waveform *> &wvec)
{
    int i, j, k, n, m, num_groups, n_labels;
    vector<int> numg, tmp;
    gvector<Waveform *> ws;

    num.clear();
    wvec.clear();
    if((n_labels = (int)labels.length()) <= 0) return(0);

    if((num_groups = getSelectedComponents(numg, ws)) <= 0) return(0);

    n = 0;
    for(m = 0; m < num_groups; m++)
    {
	i = n;
	n += numg[m];

	for(k = 0; k < n_labels; k++)
	{
	    for(j = 0; j < ws[i]->num_dw; j++) {
		if(ws[i]->dw[j].label == labels[k]) break;
	    }
	    if(j == ws[i]->num_dw) break;
	}
	if(k < n_labels) {
	    tmp.push_back(-numg[m]);
	}
	else {
	    tmp.push_back(numg[m]);
	}
    }

    k = 0;
    for(m = 0; m < num_groups; m++) {
	if(tmp[m] > 0) {
	    num.push_back(tmp[m]);
	    for(j = 0; j < tmp[m]; j++) wvec.push_back(ws[k+j]);
	}
	k += abs(tmp[m]);
    }

    return num.size();
}

/** 
 *  Get Waveform structures for a particular station that has selected
 *  components. Get the Waveform structures for selected waveforms that
 *  match the specified set of components and the station or network name
 *  matches the specified station name, and the waveforms have data at
 *  time t0 and at time t0+length. See CPlotGetSelectedComps for an
 *  explanation of the component list.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] sta The station name.
 *  @param[in] t0 An epochal time at which there must be data.
 *  @param[in] length Window length in seconds. There must also be data at
 *		t0+length
 *  @param[in] component_list A one to three character string specifying the
 *  			components requested.
 *  @param[out] wvec Pointer to the address of an array of Waveform
 *	pointers. Space for this array is allocated by this function and
 *	should be freed when nolonger needed. Note that the members of the
 *	Waveform structures should not be altered, since they continue
 *	to be used by the CPlotWidget.
 *  @returns The number of Waveform pointers in wvec.
 *  @see CPlotGetWaveforms
 *  @see CPlotGetSelected
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedData
 *  @see CPlotGetWindowedComponents
 */
int CPlotClass::getCompGroup(const string &sta, double t0, double length,
		const string &component_list, gvector<Waveform *> &wvec)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j, num_cmpnts, cmpnts[NUM_COMPONENTS];

    wvec.clear();
    num_cmpnts = (int)component_list.length();
    if(num_cmpnts > NUM_COMPONENTS) num_cmpnts = NUM_COMPONENTS;

    for(i = 0; i < num_cmpnts; i++)
    {
	for(j = 0; j < NUM_COMPONENTS; j++)
	{
	    if(component_list[i] == clist[j] ||
	       component_list[i] == Clist[j] ||
	       component_list[i] == rlist[j] ||
	       component_list[i] == Rlist[j] ||
	       component_list[i] == vlist[j] ||
	       component_list[i] == nlist[j]) break;
	}
	if(j == NUM_COMPONENTS) return(-1);
	cmpnts[i] = j;
    }

    for(i = 0; i < cp->num_entries; i++) cp->entry[i]->matched = False;

    for(i = 0; i < cp->num_entries; i++)
	if(!cp->entry[i]->matched &&
	   (!strcasecmp(sta.c_str(), cp->entry[i]->ts->sta()) ||
	    !strcasecmp(sta.c_str(), cp->entry[i]->ts->net())))
    {
	DataEntry *entry = cp->entry[i];

	for(j = 0; j < num_cmpnts; j++) {
	    DataEntry *e = entry->comps[cmpnts[j]];
	    GDataPoint *d1 = NULL, *d2 = NULL;

	    if(e == NULL || e->matched) break;

	    if((d1 = e->ts->upperBound(t0+length)) == NULL ||
	       (d2 = e->ts->lowerBound(t0)) == NULL)
	    {
		if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
		break;
	    }
	    if(d1) d1->deleteObject(); if(d2) d2->deleteObject();
	}
	if(j < num_cmpnts) continue;

	for(j = 0; j < num_cmpnts; j++) {
	    DataEntry *e = entry->comps[cmpnts[j]];
	    e->matched = True;
	    wvec.push_back(e->w);
	    FillCursorStructs(cw, e);
	}
    }
    return wvec.size();
}

/** 
 *  Get Waveform structures for selected windowed waveforms.
 *  Structures are returned for all selected waveforms that have any data
 *  points between the specified double-line cursors. The double-line
 *  cursor are specified with the 'labels' string, which contains a single
 *  letter for each cursor. Cursor are labelled with a single lower case
 *  letter beginning with 'a'.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] labels One or more double-line cursor character-labels.
 *  @param[out] wvec Pointer to the address of an array of Waveform
 *	pointers. Space for this array is allocated by this function and
 *	should be freed when nolonger needed. Note that the members of the
 *	Waveform structures should not be altered, since they continue
 *	to be used by the CPlotWidget.
 *  @returns The number of Waveform pointers in wvec. Returns -1 if
 *	the cursors are not displayed.
 *  @see CPlotGetWaveforms
 *  @see CPlotGetSelected
 *  @see CPlotGetComponentGroup
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedComponents
 */
int CPlotClass::getSelectedWaveforms(const string &labels,
			gvector<Waveform *> &wvec)
{
    int i, j, k, m, n_labels;
    gvector<Waveform *> list;

    wvec.clear();
    if((n_labels = (int)labels.length()) <= 0) return -1;

    for(k = 0; k < n_labels; k++)
    {
	char lab[2];
	lab[0] =labels[k];
	lab[1] = '\0';
	if(doubleLineIsDisplayed(lab)) break;
    }
    if(k == n_labels) return -1;

    if((m = getSelectedWaveforms(list)) <= 0) return(0);

    for(i = 0; i < m; i++)
    {
	for(k = 0; k < n_labels; k++)
	{
	    for(j = 0; j < list[i]->num_dw; j++) {
		if(list[i]->dw[j].label == labels[k]) break;
	    }
	    if(j == list[i]->num_dw) break;
	}
	if(k == n_labels) {
	    wvec.push_back(list[i]);
	}
    }
    // sort dw by label
    for(i = 0; i < wvec.size(); i++)
    {
	Waveform *cd = wvec[i];
	GDataWindow *dw = new GDataWindow[cd->num_dw];

	for(j = 0; j < cd->num_dw; j++) dw[j] = cd->dw[j];

	m = 0;
	for(k = 0; k < n_labels; k++)
	{
	    for(j = 0; j < cd->num_dw; j++) {
		if(dw[j].label == labels[k]) {
		    cd->dw[m++] = dw[j];
		    dw[j].label = '\0';
		    break;
		}
	    }
	}
	for(j = 0; j < cd->num_dw; j++) {
	    if(dw[j].label != '\0') cd->dw[m++] = dw[j];
	}
	delete [] dw;
    }
    return wvec.size();
}

/** 
 *  Get Waveform structures for stations that have selected and windowed
 *  components. Get the Waveform structures for selected waveforms that
 *  match the specified set of components and have any data points between
 *  the specified double-line cursors. The component list can be one to
 *  three characters from "zn(r)e(t)". 'n' and 'r' are interchangeable
 *  and 'e' and 't' are interchangeable, and the letters can be lower
 *  case or upper case.  The double-line cursor are specified with the
 *  'labels' string, which contains a single letter for each cursor. Cursor
 *  are labelled with a single lower case letter beginning with 'a'.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] labels One or more double-line cursor character-labels.
 *  @param[in] component_list A one to three character string specifying the
 *  			components requested.
 *  @param[out] wvec Pointer to the address of an array of Waveform
 *	pointers. Space for this array is allocated by this function and
 *	should be freed when nolonger needed. Note that the members of the
 *	Waveform structures should not be altered, since they continue
 *	to be used by the CPlotWidget.
 *  @returns The number of Waveform pointers in wvec.
 *  @see CPlotGetWaveforms
 *  @see CPlotGetSelected
 *  @see CPlotGetComponentGroup
 *  @see CPlotGetSelectedComps
 *  @see CPlotGetWindowedData
 */
int CPlotClass::getSelectedComponents(const string &labels,
			const string &component_list,
			gvector<Waveform *> &wvec)
{
    int i, j, k, l, m, n_labels, num_cmpnts;
    gvector<Waveform *> list;
    Waveform **ws;

    wvec.clear();

    if((n_labels = (int)labels.length()) <= 0) return 0;

    if((m = getSelectedComponents(component_list, list)) <= 0) return 0;

    num_cmpnts = (int)component_list.length();
    if(num_cmpnts > NUM_COMPONENTS) num_cmpnts = NUM_COMPONENTS;

    ws = new Waveform * [num_cmpnts];

    for(i = 0; i < m; i += num_cmpnts)
    {
	for(l = 0; l < num_cmpnts; l++)
	{
	    for(k = 0; k < n_labels; k++)
	    {
		for(j = 0; j < list[i+l]->num_dw; j++) {
		    if(list[i+l]->dw[j].label == labels[k]) break;
		}
		if(j == list[i+l]->num_dw) break;
	    }
	    if(k == n_labels) {
		ws[l] = list[i+l];
	    }
	    else {
		break;
	    }
	}
	if(l == num_cmpnts) {
	    for(l = 0; l < num_cmpnts; l++) wvec.push_back(ws[l]);
	}
    }
    delete [] ws;
    return wvec.size();
}

static void
FillCursorStructs(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i, m, n;
	double t;
	AxesCursor *c;

	for(i = 0; i < entry->w->num_dp; i++) {
	    if(entry->dp[i]) {
		if(entry->dp[i]) entry->dp[i]->removeOwner(cp->owner_object);
		entry->dp[i] = NULL;
	    }
	}
	Free(entry->dp);
	entry->w->num_dp = 0;
	for(m = n = 0; m < ax->num_cursors; m++)
	    if( ax->cursor[m].type == AXES_HORIZONTAL_LINE || 
		ax->cursor[m].type == AXES_VERTICAL_LINE)
/*		ax->cursor[m].type == AXES_PHASE_LINE) */
	{
	    n++;
	}
	entry->dp = (GDataPoint **)MallocIt(w, n*sizeof(GDataPoint *));
	for(m = n = 0; m < ax->num_cursors; m++)
	    if( ax->cursor[m].type == AXES_HORIZONTAL_LINE || 
		ax->cursor[m].type == AXES_VERTICAL_LINE)
/*		ax->cursor[m].type == AXES_PHASE_LINE) */
	{
	    c = &ax->cursor[m];
	    t = c->scaled_x1 - entry->w->scaled_x0 + entry->tbeg;
	    if((entry->dp[n] = entry->ts->nearest(t)) == NULL) {
		continue;
	    }
	    entry->dp[n]->setLabel(c->label[0]);
	    entry->dp[n]->addOwner(cp->owner_object);
	    n++;
	}
	entry->w->num_dp = n;
	entry->w->dp = entry->dp;

	for(i = 0; i < entry->w->num_dw; i++) {
	    if(entry->dw[i].d1) entry->dw[i].d1->removeOwner(cp->owner_object);
	    if(entry->dw[i].d2) entry->dw[i].d2->removeOwner(cp->owner_object);
	    entry->dw[i].d1 = NULL;
	    entry->dw[i].d2 = NULL;
	}
	Free(entry->dw);
	entry->w->num_dw = 0;
	for(m = n = 0; m < ax->num_cursors; m++)
	    if( ax->cursor[m].type == AXES_HOR_DOUBLE_LINE || 
		ax->cursor[m].type == AXES_VER_DOUBLE_LINE)
	{
	    n++;
	}
	entry->dw = (GDataWindow *)MallocIt(w, n*sizeof(GDataWindow));
	for(m = n = 0; m < ax->num_cursors; m++)
	    if( ax->cursor[m].type == AXES_HOR_DOUBLE_LINE || 
		ax->cursor[m].type == AXES_VER_DOUBLE_LINE)
	{
	    c = &ax->cursor[m];
	    entry->dw[n].label = c->label[0];
	    t = c->scaled_x1 - entry->w->scaled_x0 + entry->tbeg;

	    if(t > entry->ts->tend() ||
	        (entry->dw[n].d1 = entry->ts->upperBound(t)) == NULL)
	    {
		continue;
	    }
	    entry->dw[n].d1->addOwner(cp->owner_object);

	    t = c->scaled_x2 - entry->w->scaled_x0 + entry->tbeg;

	    if(t < entry->ts->tbeg() ||
		(entry->dw[n].d2 = entry->ts->lowerBound(t)) == NULL)
	    {
		if(entry->dw[n].d1) {
		    entry->dw[n].d1->removeOwner(cp->owner_object);
		    entry->dw[n].d1 = NULL;
		}
		continue;
	    }
	    entry->dw[n].d2->addOwner(cp->owner_object);
	    n++;
	}
	entry->w->num_dw = n;
	entry->w->dw = entry->dw;
}

static void
SetClipRects(CPlotWidget w, DataEntry *entry)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		x, y, n;
	XRectangle	clip[3];

	if(!cp->display_tags || (ax->x_axis != LEFT &&
	    (entry->tag_x >= ax->clipx1 ||
	     entry->tag_y + entry->tag_height <= ax->clipy1 ||
	     entry->tag_y >= ax->clipy2)) ||
	    (ax->x_axis == LEFT && (entry->tag_y >= ax->clipy1 ||
	     entry->tag_x + entry->tag_width <= ax->clipx1 ||
	     entry->tag_x >= ax->clipx2)))
	{
	    clip[0].x = ax->clipx1;
	    clip[0].y = ax->clipy1;
	    clip[0].width = ax->clipx2 - ax->clipx1 + 1;
	    clip[0].height = ax->clipy2 - ax->clipy1 + 1;
	    n = 1;
	}
	else if(ax->x_axis != LEFT)
	{
	    y = entry->tag_y;

	    n = 0;
	    if(y > ax->clipy1+1) {
		clip[n].x = ax->clipx1;
		clip[n].y = ax->clipy1;
		clip[n].width = entry->tag_width;
		clip[n].height = y - ax->clipy1;
		n++;
	    }

	    clip[n].x = ax->clipx1 + entry->tag_width;
	    clip[n].y = ax->clipy1;
	    clip[n].width = ax->clipx2 - clip[n].x + 1;
	    clip[n].height = ax->clipy2 - ax->clipy1 + 1;
	    n++;
		
	    if(y + entry->tag_height < ax->clipy2-1)
	    {
		clip[n].x = ax->clipx1;
		clip[n].y = y + entry->tag_height;
		clip[n].width = entry->tag_width;
		clip[n].height = ax->clipy2 - clip[n].y + 1;
		n++;
	    }
	}
	else
	{
	    x = entry->tag_x;
	    n = 0;
	    if(x > ax->clipx1+1)
	    {
		clip[n].x = ax->clipx1;
		clip[n].y = ax->clipy1;
		clip[n].width = x - ax->clipx1;
		clip[n].height = entry->tag_height;
		n++;
	    }

	    if(x + entry->tag_width < ax->clipx2-1)
	    {
		clip[n].x = x + entry->tag_width;
		clip[n].y = ax->clipy1;
		clip[n].width = ax->clipx2 - clip[n].x + 1;
		clip[n].height = entry->tag_height;
		n++;
	    }

	    clip[n].x = ax->clipx1;
	    clip[n].y = ax->clipy1 + entry->tag_height;
	    clip[n].width = ax->clipx2 - ax->clipx1 + 1;
	    clip[n].height = ax->clipy2 - clip[n].y + 1;
	    n++;
	}
	if(XtIsRealized((Widget)w)) {
	    XSetClipRectangles(XtDisplay(w), cp->dataGC, 0, 0,clip,n,YXSorted);
	    XSetClipRectangles(XtDisplay(w), cp->invertGC,0,0,clip,n,YXSorted);
	}
}

/** 
 *  Select or deselect a waveform. The waveform associated with the
 *  Waveform structure is selected or deselected.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd A pointer to Waveform structure associated with a
 *	waveform in this CPlotWidget.
 *  @param[in] select If True, select the waveform. If False, deselect the
 *	waveform.
 *  @param[in] do_callbacks If True, all XtNselectDataCallback callback routines
 *		are called.
 */
void CPlotClass::selectWaveform(Waveform *cd, bool select, bool do_callbacks)
{
    CPlotPart *cp = &cw->c_plot;
    int i;
    CPlotWidget z;

    for(i = 0; i < cp->num_entries; i++) if(cp->entry[i]->w == cd) break;
    if(i < cp->num_entries)
    {
	if(select != cp->entry[i]->w->selected) {
	    Select(cw, cp->entry[i], do_callbacks);
	}
    }
    if((z = (CPlotWidget)cw->axes.mag_to) != NULL) {
	Select(z, z->c_plot.entry[i], False);
    }
}

static void
Select(CPlotWidget w, DataEntry *entry, Boolean callbacks)
{
	CPlotPart *cp = &w->c_plot;

	if(cp->redraw_selected_data) {
	    _CPlotRedisplayEntry(w, entry);
	}
	else if(entry->w->visible &&
		(entry->sel.nsegs > 0 || entry->unsel.nsegs > 0))
	{
	    DisplayTag(w, entry);
	}
	if(!entry->w->selected) {
	    entry->w->selected = True;
	}
	else {
	    entry->w->selected = False;
	    entry->w->begSelect = 0.;
	    entry->w->endSelect = 0.;
	}
	if(cp->redraw_selected_data) {
	    _CPlotRedrawEntry(w, entry);
	    _CPlotRedisplayEntry(w, entry);
	}
	else if(entry->w->visible &&
		(entry->sel.nsegs > 0 || entry->unsel.nsegs > 0))
	{
	    DisplayTag(w, entry);
	}

	if(callbacks) {
	    XtCallCallbacks((Widget)w, XtNselectDataCallback, entry->w);
	}
}

static DataEntry *
WhichEntry(CPlotWidget w, int cursor_x, int cursor_y)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int j, m, entry_x0, entry_y0;
	float d, dmin;
	int clipx1, clipx2;
	DataEntry *entry, *which_entry = NULL;

	cp->label_entry = NULL;
	/* First check if the cursor is inside an entry tag-rectangle.
	 * This overrides the distance tests below.
	 */
	if(cp->display_tags)
	{
	    for(m = 0; m < cp->num_entries; m++) if(cp->entry[m]->on)
	    {
		entry = cp->entry[m];
		if(entry->r.nsegs>0 && entry->on && entry->w->visible)
		{
		    if( cursor_x > entry->lab_x && cursor_x <
			entry->lab_x + entry->tag_width - 1 &&
			cursor_y > entry->lab_y &&
			cursor_y < entry->lab_y+entry->tag_height-1)
		    {
			cp->label_entry = entry;
			return entry;
		    }
		}
	    }
	}

	if(ax->x_axis != LEFT) {
	    clipx1 = ax->clipx1;
	    clipx2 = ax->clipx2;
	}
	else {
	    clipx1 = ax->clipy1;
	    clipx2 = ax->clipy2;
	}
	/* initialize dmin to a large value before seeking the minimim.
	 */
	dmin = 2*((int)w->core.width*(int)w->core.width +
		  (int)w->core.height*(int)w->core.height);

	for(m = 0; m < cp->num_entries; m++)
		if(cp->entry[m]->on && cp->entry[m]->beg)
	{
	    entry = cp->entry[m];
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    if(entry->r.nsegs > 0 && entry->on && entry->w->visible)
	    {
		if(entry_x0 + entry->r.segs[0].x1 > clipx2) {
		    j = 0;
	   	}
		else if(entry_x0+entry->r.segs[entry->r.nsegs-1].x2< clipx1) {
		    j = entry->r.nsegs-1;
		}
		else {
		    j = -1;
		}
		if(j >= 0)
		{
		    if(ax->x_axis != LEFT)
		    {
			d = AxesClass::pointToLine(
				(float)(entry_x0+ entry->r.segs[j].x1),
				(float)(entry_y0+ entry->r.segs[j].y1),
				(float)(entry_x0+ entry->r.segs[j].x2),
				(float)(entry_y0+ entry->r.segs[j].y2),
				(float)cursor_x, (float)cursor_y);
		    }
		    else
		    {
			d = AxesClass::pointToLine(
				(float)(entry_y0+ entry->r.segs[j].y1),
				(float)(entry_x0+ entry->r.segs[j].x1),
				(float)(entry_y0+ entry->r.segs[j].y2),
				(float)(entry_x0+ entry->r.segs[j].x2),
				(float)cursor_x, (float)cursor_y);
		    }
		    if(d < dmin) {
			dmin = d;
			which_entry = entry;
		    }
		}
	   	else
		{
		    for(j = 0; j < cp->entry[m]->unsel.nsegs; j++)
		    {
			d = AxesClass::pointToLine(
			    (float)entry->unsel.segs[j].x1,
			    (float)entry->unsel.segs[j].y1,
			    (float)entry->unsel.segs[j].x2,
			    (float)entry->unsel.segs[j].y2,
			    (float)cursor_x, (float)cursor_y);

			if(d < dmin) {
			    dmin = d;
			    which_entry = entry;
			}
		    }
		    for(j = 0; j < cp->entry[m]->sel.nsegs; j++)
		    {
			d = AxesClass::pointToLine(
			    (float)entry->sel.segs[j].x1,
			    (float)entry->sel.segs[j].y1,
			    (float)entry->sel.segs[j].x2,
			    (float)entry->sel.segs[j].y2,
			    (float)cursor_x, (float)cursor_y);

			if(d < dmin) {
			    dmin = d;
			    which_entry = entry;
			}
		    }
		}
	    }
	}
	return(which_entry);
}

static Boolean
WhichMeasureBoxSide(CPlotWidget w, int x, int y)
{
	CPlotPart *cp = &w->c_plot;
	int i;

	cp->measure_entry = NULL;
	cp->resize_measure_box = -1; 
	for(i = 0; i < cp->num_entries; i++)
	{
	    if(cp->entry[i]->n_measure_segs > 0)
	    {
		XSegment *s = cp->entry[i]->measure_segs;
		if(x > s[0].x1 && x < s[0].x2 && abs(y - s[0].y1) < 5)
		{
		    cp->measure_entry = cp->entry[i];
		    cp->resize_measure_box = 0;
		    _AxesSetCursor((AxesWidget)w, "resize down");
		    return True;
		}
		else if(x > s[1].x1 && x < s[1].x2 && abs(y - s[1].y1) < 5)
		{
		    cp->measure_entry = cp->entry[i];
		    cp->resize_measure_box = 1;
		    _AxesSetCursor((AxesWidget)w, "resize up");
		    return True;
		}
		else if(y > s[1].y1 && y < s[0].y1 && abs(x - s[2].x1) < 5)
		{
		    cp->measure_entry = cp->entry[i];
		    cp->resize_measure_box = 2;
		    _AxesSetCursor((AxesWidget)w, "resize left");
		    return True;
		}
		else if(y > s[1].y1 && y < s[0].y1 && abs(x - s[3].x1) < 5)
		{
		    cp->measure_entry = cp->entry[i];
		    cp->resize_measure_box = 3;
		    _AxesSetCursor((AxesWidget)w, "move");
		    return True;
		}
		else if(cp->entry[i]->n_measure_segs == 5 &&
		    y > s[1].y1 && y < s[0].y1 && abs(x - s[4].x1) < 5)
		{
		    cp->measure_entry = cp->entry[i];
		    cp->resize_measure_box = 4;
		    _AxesSetCursor((AxesWidget)w, "resize right");
		    return True;
		}
		else if(cp->entry[i]->n_measure_segs == 6 &&
		    y > s[1].y1 && y < s[0].y1 && abs(x - s[5].x1) < 5)
		{
		    cp->measure_entry = cp->entry[i];
		    cp->resize_measure_box = 5;
		    _AxesSetCursor((AxesWidget)w, "resize right");
		    return True;
		}
	    }
	}
	return False;
}

static Boolean
InRectangle(DataEntry *entry, int ix1, int iy1, int ix2, int iy2)
{
	if(in_rectangle(&entry->unsel, ix1, iy1, ix2, iy2)) return True;
	return in_rectangle(&entry->sel, ix1, iy1, ix2, iy2);
}

static Boolean
in_rectangle(SegArray *s, int ix1, int iy1, int ix2, int iy2)
{
	int i;
	short sx1, sy1, sx2, sy2;
	float slope, intercept, yleft, yrite, xbot, xtop;
	Boolean in1, in2, x1_in, y1_in;

	for(i = 0; i < s->nsegs; i++)
	{
	    sx1 = s->segs[i].x1;
	    sy1 = s->segs[i].y1;
	    sx2 = s->segs[i].x2;
	    sy2 = s->segs[i].y2;

	    x1_in = sx1 > ix1 && sx1 < ix2;
	    y1_in = sy1 > iy1 && sy1 < iy2;

	    in1 = x1_in && y1_in;

	    in2 = (sx2 > ix1 && sx2 < ix2) && (sy2 > iy1 && sy2 < iy2);

	    if(in1 || in2) {
		return(True);
	    }
	    /* Both points are outside the rectangle. Check if the 
	     * connecting line segment intersects the rectangle.
	     */
	    if(sx1 == sx2)
	    {
		/* vertical line segment.
		 */
		if(x1_in && ((sy1 <= iy1 && sy2 >= iy2)
			  || (sy1 >= iy2 && sy2 <= iy1)) )
		{
		    return(True);
		}
	    }
	    else if(sy1 == sy2)
	    {
		/* horizontal line segment.
		 */
		if(y1_in && ((sx1 <= ix1 && sx2 >= ix2)
			  || (sx1 >= ix2 && sx2 <= ix1)) )
		{
		    return(True);
		}
	    }
	    else
	    {
		slope = (float)(sy2 - sy1)/(float)(sx2 - sx1);
		intercept = .5*(sy1 + sy2 - slope*(sx1 + sx2));
		yleft = intercept + slope*ix1;

		if(yleft > iy1 && yleft < iy2) {
		    return(True);
		}
		yrite = intercept + slope*ix2;

		if(yrite > iy1 && yrite < iy2) {
		    return(True);
		}
		xbot  = (iy1 - intercept)/slope;

		if(xbot > ix1 && xbot < ix2) {
		    return(True);
		}
		xtop  = (iy2 - intercept)/slope;

		if(xtop > ix1 && xtop < ix2) {
		    return(True);
		}
	    }
	}
	return(False);
}

/** 
 *  
 */
static void
_CPlotMouseDownScale(CPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params)
{
	if(w->axes.cursor_mode != AXES_SELECT_CURSOR) {
	    _CPlotScale(w, event, params, num_params);
	}
}

/** 
 *  
 */
void
_CPlotScale(CPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i, j, cursor_x, cursor_y, init_diff, begSel, endSel;
	double xscale, start, end, y_factor, new_yscale;
	int clipx1, clipx2, n_measure_segs, entry_x0, entry_y0;
	Boolean partial_select;
	float f;
	DataEntry *entry;
	SegArray *s;
	CPlotWidget z;
	Boolean manual;

	if(ax->scaling_double_line)
	{
	    _AxesScale((AxesWidget)w, event, params, num_params);
	    return;
	}
	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(event->type == ButtonPress)
	{
	    cp->select_entry = WhichEntry(w, cursor_x, cursor_y);
	    if(cp->select_entry == NULL)
	    {
		_AxesScale((AxesWidget)w, event, params, num_params);
		return;
	    }
	    if(cp->select_entry != NULL)
	    {
		ax->last_cursor_x = cursor_x;
		ax->last_cursor_y = cursor_y;
		entry = cp->select_entry;
		if(!visibleMinMax(w, entry) ||
		    entry->visible_ymax == entry->visible_ymin)
		{
		    return;
		}
		cp->drag_scale_initial_y = (ax->x_axis == LEFT) ?
				cursor_x : cursor_y;
		cp->scaling = True;
		cp->mov.drag_yscale = entry->drag_yscale;
		cp->mov.y_diff = entry->visible_ymax - entry->visible_ymin;
		cp->mov.y_diff = (cp->mov.y_diff != 0.) ? 1./cp->mov.y_diff: 1.;
		/* find start and end points in data array which
		 * are currently being shown.
		 */
		if(entry->unsel.nsegs > 0) {
		    s = &entry->unsel;
		    start = scale_x(&ax->d, (int)s->segs[0].x1)
					- entry->w->scaled_x0;
		    end = scale_x(&ax->d, (int)s->segs[s->nsegs-1].x2+1)
					- entry->w->scaled_x0;
		    if(entry->sel.nsegs > 0) {
		        double t1, t2;
			s = &entry->sel;
			t1 = scale_x(&ax->d, (int)s->segs[0].x1)
					- entry->w->scaled_x0;
			t2 = scale_x(&ax->d, (int)s->segs[s->nsegs-1].x2+1)
					- entry->w->scaled_x0;
			if(t1 < start) start = t1;
			if(t2 > end) end = t2;
		    }
		}
		else if(entry->sel.nsegs > 0) {
		    s = &entry->sel;
		    start = scale_x(&ax->d, (int)s->segs[0].x1)
					- entry->w->scaled_x0;
		    end = scale_x(&ax->d, (int)s->segs[s->nsegs-1].x2+1)
					- entry->w->scaled_x0;
		}
		else {
		    return;
		}
		start += entry->use_ts->tbeg();
		end += entry->use_ts->tbeg();
		if(cp->mov.beg) cp->mov.beg->deleteObject();
		if(cp->mov.end) cp->mov.end->deleteObject();
		cp->mov.beg = entry->use_ts->lowerBound(start);
		cp->mov.end = entry->use_ts->lowerBound(end);
		if(cp->mov.beg == NULL || cp->mov.end == NULL) {
		    if(cp->mov.beg) cp->mov.beg->deleteObject();
		    if(cp->mov.end) cp->mov.end->deleteObject();
		    return;
		}
	    	cp->mov.npts = 0;
		for(i = cp->mov.beg->segmentIndex();
			i <= cp->mov.end->segmentIndex(); i++)
		{
		    GSegment *seg = cp->mov.end->segment();
		    int i1 = (i == cp->mov.beg->segmentIndex()) ?
				cp->mov.beg->index() : 0;
		    int i2 = (i == cp->mov.end->segmentIndex()) ?
				cp->mov.end->index() : seg->length()-1;
		    cp->mov.npts += i2 - i1 + 1;
		}
		cp->mov.scaled = False;
	    }
	    else
	    {
		cp->scaling = False;
	    }
	}
	else if(event->type == MotionNotify && cp->scaling)
	{
	    double ydif, yscale;
	    if(cursor_x == ax->last_cursor_x && cursor_y == ax->last_cursor_y)
		return;
	    ax->last_cursor_x = cursor_x;
	    ax->last_cursor_y = cursor_y;
	
	    ax->use_screen = False;
	    entry = cp->select_entry;
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    /*
	     * prevent redrawing of measure box.
	     */
	    n_measure_segs = entry->n_measure_segs;
	    entry->n_measure_segs = 0;

	    _CPlotRedisplayEntry(w, entry);

	    entry->w->visible = True;

	    ydif = (cp->scale_independently) ?
			entry->ymax - entry->ymin : cp->uniform_scale;
	    if(ydif != 0.) {
	        yscale = (cp->dataHeight/ydif)*(ax->y2[1] - ax->y1[1])/
				(ax->y2[ax->zoom] - ax->y1[ax->zoom]);
	    }
	    else yscale = 0.;

	    init_diff = abs(cp->drag_scale_initial_y - entry_y0);
	    if(init_diff == 0) init_diff = 1;
	    new_yscale = yscale*entry->drag_yscale*
			(double)abs(cursor_y - entry_y0)/(double)init_diff;

	    if(fabs(new_yscale) < cp->mov.y_diff) new_yscale = cp->mov.y_diff;
	    y_factor = (new_yscale/yscale)/cp->mov.drag_yscale;
	    cp->mov.drag_yscale = new_yscale/yscale;

	    if(cp->mov.npts < 1000)
	    {
	        xscale = (ax->x_axis != LEFT) ? ax->d.unscalex : ax->d.unscaley;
		drawTimeSeries(cp->mov.beg->timeSeries(), cp->mov.beg,
			cp->mov.end, entry->mean, xscale,
			cp->mov.drag_yscale*yscale, &entry->r);
		if(ax->x_axis == LEFT)
		{
		    f = entry->r.xmin;
		    entry->r.xmin = entry->r.ymin;
		    entry->r.ymin = f;
		    f = entry->r.xmax;
		    entry->r.xmax = entry->r.ymax;
		    entry->r.ymax = f;
		}
		entry->r.start = 0;
		entry->r.nshow = entry->r.nsegs;
		entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + cp->mov.beg->time() - entry->tbeg);
	    }
	    else
	    {
		for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		    entry->r.segs[j].y1 *= y_factor;
		    entry->r.segs[j].y2 *= y_factor;
		}
		if(ax->x_axis != LEFT)
		{
		    entry->r.ymin *= y_factor;
		    entry->r.ymax *= y_factor;
		}
		else
		{
		    entry->r.xmin *= y_factor;
		    entry->r.xmax *= y_factor;
		}
	    }

	    partial_select = selectLimits(w, entry, &begSel, &endSel);

	    if(	entry_x0+entry->r.xmin < -S_LIM ||
		entry_x0+entry->r.xmax >  S_LIM ||
	   	entry_y0+entry->r.ymin < -S_LIM ||
		entry_y0+entry->r.ymax >  S_LIM)
	    {
		/* do the clipping here. X can't handle it. */
		SegArray *sa = entry->w->selected ? &entry->sel:&entry->unsel;
		DoClipping(w, entry->r.nshow, entry->r.segs+entry->r.start,
				entry_x0, entry_y0, sa);
	    }
	    else if(partial_select)
	    {
		int k, l;
		Free(entry->sel.segs);
		Free(entry->unsel.segs);
		entry->sel.size_segs = entry->unsel.size_segs = 0;
		entry->sel.nsegs = 0;
		entry->unsel.nsegs = 0;
		for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		    if(entry->r.segs[j].x1 >= begSel
			&& entry->r.segs[j].x2 <= endSel)
		    {
			entry->sel.nsegs++;
		    }
		    else {
			entry->unsel.nsegs++;
		    }
		}
		if(entry->sel.nsegs > 0) {
		  ALLOC(entry->sel.nsegs*sizeof(DSegment), entry->sel.size_segs,
		    entry->sel.segs, DSegment, 0);
		}
		if(entry->unsel.nsegs > 0) {
		  ALLOC(entry->unsel.nsegs*sizeof(DSegment),
		    entry->unsel.size_segs, entry->unsel.segs, DSegment, 0);
		}
		for(i=k=l=0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		 if(entry->r.segs[j].x1 >=begSel && entry->r.segs[j].x2<=endSel)
		 {
		    entry->sel.segs[k].x1 =
				(short)(entry_x0 + entry->r.segs[j].x1 + .5);
		    entry->sel.segs[k].y1 =
				(short)(entry_y0 + entry->r.segs[j].y1 + .5);
		    entry->sel.segs[k].x2 =
				(short)(entry_x0 + entry->r.segs[j].x2 + .5);
		    entry->sel.segs[k].y2 =
				(short)(entry_y0 + entry->r.segs[j].y2 + .5);
		    k++;
		 }
		 else {
		    entry->unsel.segs[l].x1 =
				(short)(entry_x0 + entry->r.segs[j].x1+.5);
		    entry->unsel.segs[l].y1 =
				(short)(entry_y0 + entry->r.segs[j].y1+.5);
		    entry->unsel.segs[l].x2 =
				(short)(entry_x0 + entry->r.segs[j].x2+.5);
		    entry->unsel.segs[l].y2 =
				(short)(entry_y0 + entry->r.segs[j].y2+.5);
		    l++;
		 }
		}
	    }
	    else {
		SegArray *sa = entry->w->selected ? &entry->sel:&entry->unsel;
		sa->nsegs = entry->r.nshow;
		ALLOC(sa->nsegs*sizeof(DSegment), sa->size_segs, sa->segs,
			DSegment, 0);
		for(i = 0, j = entry->r.start; i < entry->r.nshow; i++, j++)
		{
		    sa->segs[i].x1 = (short)(entry_x0 + entry->r.segs[j].x1+.5);
		    sa->segs[i].y1 = (short)(entry_y0 + entry->r.segs[j].y1+.5);
		    sa->segs[i].x2 = (short)(entry_x0 + entry->r.segs[j].x2+.5);
		    sa->segs[i].y2 = (short)(entry_y0 + entry->r.segs[j].y2+.5);
		}
	    }
	    _CPlotRedisplayEntry(w, entry);
	    cp->mov.scaled = True;

	    entry->n_measure_segs = n_measure_segs;
	    if(ax->use_pixmap)
	    {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	    }
	    ax->use_screen = True;
	}
	else if(event->type == ButtonRelease  && cp->scaling)
	{
	    if(!cp->mov.scaled) {
		cp->scaling = False;
		return;
	    }
	    ax->use_screen = False;
	    entry = cp->select_entry;
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	    xscale = (ax->x_axis != LEFT) ?  ax->d.unscalex : ax->d.unscaley;

	    plot_ts(entry, xscale, cp->mov.drag_yscale*entry->yscale);

	    if(ax->x_axis == LEFT)
	    {
		f = entry->r.xmin;
		entry->r.xmin = entry->r.ymin;
		entry->r.ymin = f;
		f = entry->r.xmax;
		entry->r.xmax = entry->r.ymax;
		entry->r.ymax = f;
		clipx1 = ax->clipy1;
		clipx2 = ax->clipy2;
	    }
	    else {
		clipx1 = ax->clipx1;
		clipx2 = ax->clipx2;
	    }

	    for(entry->r.start = 0; entry->r.start < entry->r.nsegs &&
			entry_x0 + entry->r.segs[entry->r.start].x2 < clipx1;
			entry->r.start++);

	    for(i = entry->r.nsegs-1; i > entry->r.start && entry_x0 +
			entry->r.segs[i].x1 > clipx2; i--);
	    entry->r.nshow = i - entry->r.start + 1;

	    _CPlotRedisplayEntry(w, entry);

	    cp->scaling = False;
	    cp->mov.scaled = False;
	    entry->drag_yscale = cp->mov.drag_yscale;
	    _CPlotRedrawEntry(w, entry);
	    _CPlotRedisplayEntry(w, entry);
	    DrawArrivalsOnEntry(w, entry);
	    FindArrivals(w, entry);
	    DrawArrivalsOnEntry(w, entry);
	    _CPlotDrawPredArrivals(w, entry);
	    FindPredArrivals(w, entry);
	    _CPlotDrawPredArrivals(w, entry);

	    if(ax->use_pixmap)
	    {
		XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	    }
	    ax->use_screen = True;

	    manual = (*num_params > 0 && !strcmp(params[0], "manual"));
	    if(!manual && (z = (CPlotWidget)ax->mag_to) != NULL)
	    {
		DataEntry *ze = (DataEntry *)entry->mag_entry;
		z->axes.use_screen = False;
		_CPlotRedisplayEntry(z, ze);
		ze->drag_yscale = entry->drag_yscale;
		_CPlotRedrawEntry(z, ze);
		_CPlotRedisplayEntry(z, ze);
		DrawArrivalsOnEntry(z, ze);
		FindArrivals(z, ze);
		DrawArrivalsOnEntry(z, ze);
		_CPlotDrawPredArrivals(z, ze);
		FindPredArrivals(z, ze);
		_CPlotDrawPredArrivals(z, ze);
		if(z->axes.use_pixmap)
		{
		    XCopyArea(XtDisplay(z), z->axes.pixmap, XtWindow(z),
				z->axes.axesGC, 0, 0, z->core.width,
				z->core.height, 0, 0);
		}
		z->axes.use_screen = True;
	    }
	    if(!manual && (z = (CPlotWidget)ax->mag_from) != NULL)
	    {
		DataEntry *ze = (DataEntry *)entry->mag_entry;
		z->axes.use_screen = False;
		_CPlotRedisplayEntry(z, ze);
		ze->drag_yscale = entry->drag_yscale;
		_CPlotRedrawEntry(z, ze);
		_CPlotRedisplayEntry(z, ze);
		DrawArrivalsOnEntry(z, ze);
		FindArrivals(z, ze);
		DrawArrivalsOnEntry(z, ze);
		_CPlotDrawPredArrivals(z, ze);
		FindPredArrivals(z, ze);
		_CPlotDrawPredArrivals(z, ze);
		if(z->axes.use_pixmap)
		{
		    XCopyArea(XtDisplay(z), z->axes.pixmap, XtWindow(z),
				z->axes.axesGC, 0, 0, z->core.width,
				z->core.height, 0, 0);
		}
		z->axes.use_screen = True;
	    }
	}
}

/** 
 *  Select or deselect all CssArrivalClass in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] select If True, select all arrivals, otherwise deselect them.
 */
void CPlotClass::selectAllArrivals(bool select)
{
    CPlotPart *cp = &cw->c_plot;

    // select or deselect all arrivals
    for(int i = 0; i < cp->v->arrival_entries.size(); i++)
    {
	ArrivalEntry *a_entry = cp->v->arrival_entries[i];
	if(a_entry->selected != select) {
	    DoSelectArrival(cw, a_entry, False);
	}
    }
}

static void
ResizeMeasureBox(CPlotWidget w, XEvent *event)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		cursor_x, cursor_y, delx, dely;
	double		dx, dy, bottom;
	DataEntry	*entry, *ze, *ze0;
	CPlotWidget	z, z0;

	if(cp->measure_entry == NULL || cp->resize_measure_box < 0) return;

	entry = cp->measure_entry;

	if((z = (CPlotWidget)ax->mag_to) == NULL) {
	    z = (CPlotWidget)ax->mag_from;
	}
	ze = (z != NULL) ? (DataEntry *)cp->measure_entry->mag_entry : NULL;

	z0 = (ax->mag_from == NULL) ? w : (CPlotWidget)ax->mag_from;
/*
	if(z0 != NULL) ze0 = z0->c_plot.entry[cp->measure_entry];
*/
	if(z0 != NULL) ze0 = z0->c_plot.measure_entry;

	if(event->type == MotionNotify)
	{
	    cursor_x = ((XButtonEvent *)event)->x;
	    cursor_y = ((XButtonEvent *)event)->y;
	
	    delx = cursor_x - ax->last_cursor_x;
	    dely = cursor_y - ax->last_cursor_y;

	    ax->last_cursor_x = cursor_x;
	    ax->last_cursor_y = cursor_y;

	    DrawMeasureBoxes(w, cp->measure_entry);

	    bottom = entry->bottom_side;
	    if(cp->resize_measure_box == 0)
	    {
		dy = dely/entry->yscale;
		entry->bottom_side -= dy;
		entry->amp_ts += dy;
		if(entry->amp_ts < 0.) {
		    entry->amp_ts = -entry->amp_ts;
		    entry->bottom_side -= entry->amp_ts;
		    cp->resize_measure_box = 1;
		}
		if(entry->zp_ts == bottom) {
		    entry->zp_ts = entry->bottom_side;
		}
	    }
	    else if(cp->resize_measure_box == 1)
	    {
		dy = dely/entry->yscale;
		entry->amp_ts -= dy;
		if(entry->amp_ts < 0.) {
		    entry->amp_ts = -entry->amp_ts;
		    entry->bottom_side -= entry->amp_ts;
		    cp->resize_measure_box = 0;
		}
		if(entry->zp_ts != bottom) entry->zp_ts -= dy;
	    }
	    else if(cp->resize_measure_box == 2)
	    {
		dx = delx*ax->d.scalex;
		entry->left_side += dx;
		entry->period -= dx;
		if(entry->period < 0.) {
		    entry->period = -entry->period;
		    entry->left_side -= entry->period;
		    cp->resize_measure_box = 4;
		}
	    }
	    else if(cp->resize_measure_box == 3)
	    {
		entry->left_side += delx*ax->d.scalex;
		entry->bottom_side -= dely/entry->yscale;
	    }
	    else if(cp->resize_measure_box >= 4)
	    {
		dx = delx*ax->d.scalex;
		entry->period += dx;
		if(entry->period < 0.) {
		    entry->period = -entry->period;
		    entry->left_side -= entry->period;
		    cp->resize_measure_box = 2;
		}
	    }
	    if(ze != NULL)
	    {
		ze->left_side = entry->left_side;
		ze->bottom_side = entry->bottom_side;
		ze->amp_ts = entry->amp_ts;
		ze->zp_ts = entry->zp_ts;
		ze->period = entry->period;
		z->c_plot.resize_measure_box = cp->resize_measure_box;
	    }
	    DrawMeasureBoxes(w, cp->measure_entry);

	    if(XtHasCallbacks((Widget)z0,XtNmeasureCallback)!=XtCallbackHasNone)
	    {
		DoAmpConvertCallback(z0, 1, &ze0);
		DoMeasureCallback(z0, 1, &ze0);
	    }
	}
	else if(event->type == ButtonRelease)
	{
	    cp->measure_entry = NULL;
	    cp->resize_measure_box = -1;

	    if(z != NULL) {
		z->c_plot.measure_entry = NULL;
		z->c_plot.resize_measure_box = -1;
	    }
	}
}

/** 
 *  Remove the indicated waveforms from a CPlotWidget. The waveforms
 *  associated with the Waveform structure in 'wvec' are removed
 *  from the CPlotWidget. The corresponding TimeSeries objects are
 *  unregistered and freed, if they are not registered with another object
 *  or widget.  After the deletion, the remaining waveforms are moved towards
 *  the top of the data window to fill all gaps.
 *  <p>
 *  This function only removes waveforms. Any CssArrivalClass, CssOriginClasss, CssAssocClasss,
 *  etc. that are associated with the waveforms are not removed.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] num The number of waveforms to be deleted.
 *  @param[in] wvec The Waveform pointers for 'num' waveforms.
 */
void CPlotClass::deleteWaveforms(gvector<Waveform *> &wvec)
{
    if(wvec.size() > 0) {
	deleteWaveforms_NoReposition(wvec);
	position(cw);
	_AxesRedraw((AxesWidget)cw);
	_CPlotRedraw(cw);
	_AxesRedisplayAxes((AxesWidget)cw);
	_CPlotRedisplay(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	change.waveform = true;
	doDataChangeCallbacks();
    }
}

/** 
 *  Remove the indicated waveforms from a CPlotWidget. The waveforms
 *  associated with the Waveform structure in 'wvec' are removed
 *  from the CPlotWidget. The corresponding TimeSeries objects are
 *  unregistered and freed, if they are not registered with another object
 *  or widget. The remaining waveforms are not repositioned in the data
 *  window.
 *  <p>
 *  This function only removes waveforms. Any CssArrivalClass, CssOriginClasss, CssAssocClasss,
 *  etc. that are associated with the waveforms are not removed.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] num The number of waveforms to be deleted.
 *  @param[in] wvec The Waveform pointers for 'num' waveforms.
 */
void CPlotClass::deleteWaveforms_NoReposition(gvector<Waveform *> &wvec)
{

    CPlotPart *cp = &cw->c_plot;
    int i, j;

    if(wvec.size() == 0) return;
    for(i = 0; i < wvec.size(); i++)
    {
	for(j = 0; j < cp->num_entries; j++) {
	    if(wvec[i] == cp->entry[j]->w) break;
	}
	if(j < cp->num_entries) {
	    DeleteEntry(cw, cp->entry[j]);
	}
    }
    change.waveform = true;
    doDataChangeCallbacks();

}

/** 
 *  Remove all selected waveforms from a CPlotWidget. All selected
 *  TimeSeries objects are removed. No tables (CssArrivalClass, CssOriginClass, etc.)
 *  are removed.
 */
void CPlotClass::deleteSelectedWaveforms()
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    int m;

    if(ax->mag_from != NULL) {
	// can't delete data from a magnify CPlot widget
	return;
    }
    for(m = cp->num_entries-1; m >= 0; m--)
    {
	if(cp->entry[m]->w->selected) {
	    DeleteEntry(cw, cp->entry[m]);
	}
    }
    if(cp->num_entries == 0) cp->data_shift = False;

    position(cw);
    _AxesRedraw((AxesWidget)cw);
    _CPlotRedraw(cw);
    _AxesRedisplayAxes((AxesWidget)cw);
    _CPlotRedisplay(cw);
    _AxesRedisplayXor((AxesWidget)cw);
    change.waveform = true;
    change.wftag = true;
    doDataChangeCallbacks();

}

static void
DeleteEntry(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int i, j, k, m;
	CPlotWidget z;
	DataEntry *ze;

	if(ax->mag_from != NULL) {
	    /* can't delete entries from a magnify CPlot widget */
	    return;
	}
	if(entry == (DataEntry *)NULL || cp->num_entries <= 0) {
	    return;
	}
	for(m = 0; m < cp->num_entries; m++) {
	    if(entry == cp->entry[m]) break;
	}
	if(m == cp->num_entries) return;


	_CPlotRedisplayEntry(w, entry);
	if((z = (CPlotWidget)ax->mag_to) != NULL) {
	    ze = z->c_plot.entry[m];
	    _CPlotRedisplayEntry(z, ze);
	}

	if(entry == cp->ref) cp->ref = (DataEntry *)NULL;
	if(entry == cp->measure_entry) cp->measure_entry = (DataEntry *)NULL;
	if(entry == cp->label_entry) cp->label_entry = (DataEntry *)NULL;
	if(entry == cp->select_entry) cp->select_entry = (DataEntry *)NULL;
	if(entry == cp->move_entry) cp->move_entry = (DataEntry *)NULL;

	k = ComponentIndex(entry);
	for(j = 0; j < NUM_COMPONENTS; j++)
	{
	    if(j != k && entry->comps[j] != NULL) {
		entry->comps[j]->comps[k] = NULL;
		entry->w->c[j]->c[k] = NULL;
	    }
	}
	for(j = 0; j < NUM_COMPONENTS; j++)
	{
	    if(j != k && entry->comps[j] != NULL) break;
	}

	CPlotFreeEntry(cp, entry);

	for(i = m; i < cp->num_entries-1; i++)
	{
	    cp->entry[i] = cp->entry[i+1];
	}
	cp->num_entries--;
	cp->max_tag_width = maxTagWidth(w);
	if(z != NULL)
	{
	    ze = z->c_plot.entry[m];
	    if(ze == z->c_plot.ref) z->c_plot.ref = (DataEntry *)NULL;
	    z->c_plot.measure_entry = cp->measure_entry;

	    k = ComponentIndex(ze);
	    for(i = 0; i < NUM_COMPONENTS; i++)
	    {
		if(i != k && ze->comps[i] != NULL) {
		    ze->comps[i]->comps[k] = NULL;
		    ze->w->c[i]->c[k] = NULL;
		}
	    }
	    Free(ze->sel.segs);
	    Free(ze->unsel.segs);
	    Free(ze->r.segs);
	    Free(ze->arrlab);
	    Free(ze->pred_arr);
	    Free(ze->tag_points);
	    if(ze->beg) ze->beg->deleteObject();
	    if(ze->end) ze->end->deleteObject();
	    ze->w->ts = NULL;
	    ze->w->num_dp = 0;
	    ze->w->dp = NULL;
	    ze->w->num_dw = 0;
	    ze->w->dw = NULL;
	    delete ze->w;
	    Free(ze);
	    for(i = m; i < z->c_plot.num_entries-1; i++) {
		z->c_plot.entry[i] = z->c_plot.entry[i+1];
	    }
	    z->c_plot.num_entries--;
	    z->c_plot.max_tag_width = cp->max_tag_width;
	}
}

/** 
 * Remove all waveforms and tables from a CPlotWidget. All cursors are also
 * removed.
 */
void CPlotClass::clear(bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    if(do_callback) {
	change.waveform = (cp->num_entries > 0);
	change.arrival = (cp->v->arrivals.size() > 0);
	change.assoc = (cp->v->assocs.size() > 0);
	change.origin = (cp->v->origins.size() > 0);
	change.origerr = (cp->v->origerrs.size() > 0);
	change.stassoc = (cp->v->stassocs.size() > 0);
	change.stamag = (cp->v->stamags.size() > 0);
	change.netmag = (cp->v->netmags.size() > 0);
	change.hydro = (cp->v->hydro_features.size() > 0);
	change.infra = (cp->v->infra_features.size() > 0);
	change.wftag = (cp->v->wftags.size() > 0);
    }

    clearWaveforms(false);

    cp->v->selected_tables.clear();

    cp->v->ampdescripts.clear();
    cp->v->amplitudes.clear();
    clearArrivals(false);
    cp->v->assocs.clear();
    cp->v->filters.clear();
    cp->v->hydro_features.clear();
    cp->v->infra_features.clear();
    cp->v->netmags.clear();
    cp->v->origerrs.clear();
    cp->v->origins.clear();
    cp->v->parrivals.clear();
    cp->v->stamags.clear();
    cp->v->stassocs.clear();
    cp->v->wftags.clear();

    cp->working_orid = 0;

    deleteAllCursors();

    if(do_callback && (change.waveform || change.arrival || change.assoc ||
	change.origin || change.origerr || change.stassoc ||change.stamag ||
	change.netmag || change.hydro || change.infra || change.wftag))
    {
	change.amplitude = false;
	change.ampdescript = false;
	change.filter = false;
	change.parrival = false;
	change.working_orid = false;
	doDataChangeCallbacks();
    }
}

bool CPlotClass::empty()
{
    CPlotPart *cp = &cw->c_plot;

    for(int i = 0; i < cp->num_curves; i++) {
	if(cp->curve[i]->type != CPLOT_TTCURVE) return false;
    }
    if(cp->num_entries) return false;
    if(cp->v->wftags.size()) return false;
    if(cp->v->arrival_entries.size()) return false;
    if(cp->v->assocs.size()) return false;
    if(cp->v->origins.size()) return false;
    if(cp->v->origerrs.size()) return false;
    if(cp->v->stassocs.size()) return false;
    if(cp->v->stamags.size()) return false;
    if(cp->v->netmags.size()) return false;
    if(cp->v->hydro_features.size()) return false;
    if(cp->v->infra_features.size()) return false;
    if(cp->v->amplitudes.size()) return false;
    if(cp->v->ampdescripts.size()) return false;
    if(cp->v->filters.size()) return false;
    if(cp->v->parrivals.size()) return false;

    return true;
}

/** 
 *  Remove all waveforms from a CPlotWidget. All TimeSeries objects are
 *  unregistered. Only waveforms are removed. This function does not remove
 *  any CssArrivalClass, CssOriginClasss, etc.  All cursors are also removed.
 */
void CPlotClass::clearWaveforms(bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    int i, n;
    CPlotWidget z;

    cp->data_shift = False;
    if(ax->mag_from != NULL)
    {
	// can't clear a magnify CPlot widget
	return;
    }
    deleteAllCursors();

    n = cp->num_curves;
    for(i = 0; i < cp->num_curves; i++)
    {
	if(cp->curve[i]->type != CPLOT_TTCURVE)
	{
	    delete cp->curve[i];
	    n--;
	}
    }
    cp->num_curves = n;

    for(i = 0; i < cp->num_entries; i++) {
	CPlotFreeEntry(cp, cp->entry[i]);
    }

    for(i = 0; i < cp->v->wftags.size(); i++) {
	cp->v->selected_tables.remove(cp->v->wftags[i]);
    }
    cp->v->wftags.clear();

    cp->num_entries = 0;
    cp->label_entry = NULL;
    cp->select_entry = NULL;
    cp->move_entry = NULL;
    cp->measure_entry = NULL;
    cp->ref = NULL;
    ax->unmark = False;
    ax->mag_scaled_x1 = ax->x1[0];
    ax->mag_scaled_x2 = ax->x2[0];
    ax->mag_scaled_y1 = ax->y1[0];
    ax->mag_scaled_y2 = ax->y2[0];

    if(XtIsRealized((Widget)cw)) {

	position(cw);
	_AxesRedraw((AxesWidget)cw);

	ax->use_screen = False;
	_AxesClearArea((AxesWidget)cw, ax->d.ix1, ax->d.iy2,
		ax->d.ix2 - ax->d.ix1 + 1, ax->d.iy1 - ax->d.iy2 + 1);
	_AxesRedisplayAxes((AxesWidget)cw);
	_CPlotRedisplay(cw);
	_AxesRedisplayXor((AxesWidget)cw);
	ax->use_screen = True;

	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    z->axes.use_screen = False;
	    CPlotMagnifyWidget(cw, NULL, True);
	    CPlotMagnifyWidget(cw, z, True);
	    ax->magnify_rect = True;
	    z->axes.use_screen = True;
	}
    }
    if(do_callback) {
	change.waveform = true;
        change.wftag = true;
        doDataChangeCallbacks();
    }
}

/** 
 *  Display all data or only selected data. If 'mode' is 0, all waveforms will
 *  be displayed. This includes waveforms that are currently not displayed
 *  because they were hidden by a previous call to this function or a call to
 *  displayComponents. If 'mode' is 1, then only selected waveforms will
 *  be displayed. All deselected waveforms will be hidden, until another call
 *  to this function or a call to displayComponents.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] mode 0: display all waveforms. 1: display only selected waveforms.
 *  @see displayComponents
 */
void CPlotClass::displayData(int mode)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i;
    bool found_one;
    CPlotWidget z;

    if(mode <= 0)
    {
	found_one = False;
	for(i = 0; i < cp->num_entries; i++) if(!w->c_plot.entry[i]->on)
	{
	    found_one = true;
	    cp->entry[i]->on = True;
	}
	if(found_one && mode == 0)
	{
	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw(w);
	    if(!ax->redisplay_pending)
	    {
		_AxesRedisplayAxes((AxesWidget)w);
		_CPlotRedisplay(w);
		_AxesRedisplayXor((AxesWidget)w);
	    }
	}
	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    found_one = False;
	    for(i = 0; i < z->c_plot.num_entries; i++)
		    if(!z->c_plot.entry[i]->on)
	    {
		found_one = True;
		z->c_plot.entry[i]->on = True;
	    }
	    if(found_one && mode == 0)
	    {
		_AxesRedraw((AxesWidget)z);
		_CPlotRedraw(z);
		if(!z->axes.redisplay_pending) {
		    _AxesRedisplayAxes((AxesWidget)z);
		    _CPlotRedisplay(z);
		    _AxesRedisplayXor((AxesWidget)z);
		}
	    }
	}
    }
    else
    {
	for(i = 0; i < cp->num_entries; i++)
	    if(!cp->entry[i]->w->selected && cp->entry[i]->on)
	{
	    _CPlotRedisplayEntry(w, cp->entry[i]);
	    cp->entry[i]->on = False;
	}
	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    for(i = 0; i < z->c_plot.num_entries; i++)
		if(!z->c_plot.entry[i]->w->selected && z->c_plot.entry[i]->on)
	    {
		_CPlotRedisplayEntry(z, z->c_plot.entry[i]);
		z->c_plot.entry[i]->on = False;
	    }
	}
    }
}

/** 
 *  Set the display-flag for waveforms.
 *  @see displayComponents
 */
void CPlotClass::setDataDisplay(gvector<Waveform *> &wvec, vector<bool> &on,
			bool promote_visible, bool redisplay)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    CPlotWidget z = (CPlotWidget)ax->mag_to;
    int i, j;

    if((int)on.size() < wvec.size()) {
	cerr << "CPlotClass.setDataDisplay: on.size() < wvec.size()" << endl;
	return;
    }
    for(i = 0; i < wvec.size(); i++)
    {
	for(j = 0; j < cp->num_entries; j++) {
	    if(wvec[i] == cp->entry[j]->w) {
		w->c_plot.entry[j]->on = on[i];
		if(z  != NULL) z->c_plot.entry[j]->on = on[i];
		break;
	    }
	}
    }
    if(promote_visible) {
	placeEntries(w);
	position(w);
    }
	
    if(redisplay)
    {
	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);
	if(z != NULL)
	{
	    z->axes.use_screen = False;
	    _AxesRedraw((AxesWidget)z);
	    _CPlotRedraw(z);
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	    z->axes.use_screen = True;
	}
    }
}

/** 
 *  Set the display-flag for arrivals.
 *  @see displayComponents
 */
void CPlotClass::displayArrival(const string &sta, const string &chan,
			int arid, bool on)
{
    CPlotPart *cp = &cw->c_plot;

    for(int i = 0; i < cp->v->arrivals.size(); i++) {
	CssArrivalClass *a = cp->v->arrivals[i];
	if(!strcasecmp(sta.c_str(),a->sta) &&
		DataSource::compareChan(chan,a->chan) && arid == a->arid)
	{
	    setArrivalOn(a, on);
	}
    }
}

void CPlotClass::setArrivalOn(CssArrivalClass *a, bool on)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    CPlotWidget z = (CPlotWidget)ax->mag_to;
     ArrivalEntry *ae;
    int i, j, k;

    if(cp->display_arrivals == CPLOT_ARRIVALS_OFF) return;

    for(i = 0; i < cp->v->arrival_entries.size() &&
		cp->v->arrival_entries[i]->a != a; i++);
    if(i == cp->v->arrival_entries.size()) return;

    ae = cp->v->arrival_entries[i];
    if(ae->on == on) return;

    /* Redraw all occurrences of a. Since it is drawn with invertCG, it
     * will be hidden if it is currently drawn, or displayed if it is
     * currently not drawn.
     */
    if(on) ae->on = True;

    for(j = 0; j < cp->num_entries; j++) {
	DataEntry *entry = cp->entry[j];
	for(k = 0; k < entry->n_arrlab; k++) {
	    if(entry->arrlab[k].a_entry->a == a)
	    {
		DrawOneArrival(w, entry, k,
			entry->arrlab[k].a_entry->selected, False);
	    }
	}
    }
    // Do mag window
    if((z = (CPlotWidget)ax->mag_to) != NULL)
	for(j = 0; j < cp->num_entries; j++) {
	{
	    DataEntry *ze = z->c_plot.entry[j];
	    for(k = 0; k < ze->n_arrlab; k++) {
		if(ze->arrlab[k].a_entry->a == a) {
		    DrawOneArrival(z, ze, k,
				ze->arrlab[k].a_entry->selected, False);
		}
	    }
	}
    }
    if(!on) ae->on = False;
}

void CPlotClass::setArrivalsDisplay(cvector<CssArrivalClass> &a, bool *on)
{
    for(int i = 0; i < a.size(); i++) {
	setArrivalOn(a[i], on[i]);
    }
}

/** 
 *  Set the vertical axis labelling format to integer. If 'set' is True,
 *  the vertical axis labelling format is set to 'integer'. If 'set' is
 *  False, the vertical axis labelling format is set to 'regular'. When the
 *  labelling format is 'integer', no axis tickmarks will be drawn and only
 *  integer labels will be used.
 *  
 */
void CPlotClass::setYLabelInt(bool set, bool redisplay)
{
    CPlotWidget w = cw;
    AxesPart *ax = &w->axes;
    CPlotWidget z;

    ax->a.y_label_int = set;
    if((z = (CPlotWidget)ax->mag_to) != NULL) {
	z->axes.a.y_label_int = set;
    }
    if(redisplay)
    {
	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);
	if(z != NULL)
	{
	    z->axes.use_screen = False;
	    _AxesRedraw((AxesWidget)z);
	    _CPlotRedraw(z);
	    _AxesRedisplayAxes((AxesWidget)z);
	    _CPlotRedisplay(z);
	    _AxesRedisplayXor((AxesWidget)z);
	    z->axes.use_screen = True;
	}
    }
}

/** 
 *  Set the screen position of the specified waveform.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd A Waveform structure pointer associated with a waveform
 *		in this CPlotWidget.
 *  @param[in] scaled_x0 The x or epochal time position of the first data
 *		sample.
 *  @param[in] scaled_y0 The y or vertical position of the waveform mean.
 *  
 */
void CPlotClass::positionOne(Waveform *cd, double scaled_x0, double scaled_y0)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i;
    DataEntry *entry, *ze;
    CPlotWidget z;
    CPlotPositionCallbackStruct c;

    for(i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w == cd) break;
    }
    if(i == cp->num_entries) return;

    entry = cp->entry[i];
    if(entry->type != CPLOT_DATA) return;

    _CPlotRedisplayEntry(w, entry);
    entry->w->scaled_x0 = scaled_x0;
    entry->w->scaled_y0 = scaled_y0;
    _CPlotVisible(w, entry);
    _CPlotRedrawEntry(w, entry);
    _CPlotRedisplayEntry(w, entry);
    AdjustHorizontalLimits(w);
    if((z = (CPlotWidget)ax->mag_to) != NULL)
    {
	ze = z->c_plot.entry[i];
	_CPlotRedisplayEntry(z, ze);
	ze->w->scaled_x0 = entry->w->scaled_x0;
	ze->w->scaled_y0 = entry->w->scaled_y0;
	_CPlotVisible(z, ze);
	_CPlotRedrawEntry(z, ze);
	_CPlotRedisplayEntry(z, ze);
	AdjustHorizontalLimits(z);
    }
    c.reason = CPLOT_DATA_POSITION;
    c.wvec.push_back(cd);
    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);
}

/** 
 *  Set the x or time position of the indicated waveforms.
 */
void CPlotClass::positionX(gvector<Waveform *> &wvec, vector<double> &scaled_x0)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i, j;
    DataEntry *entry, *ze;
    CPlotWidget z = NULL;
    CPlotPositionCallbackStruct c;

    if((int)scaled_x0.size() < wvec.size()) {
       cerr << "CPlotClass.positionX: scaled_x0.size() < wvec.size()" << endl;
       return;
    }

    cp->data_shift = False;

    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < cp->num_entries; j++) {
	    if(cp->entry[j]->w == wvec[i]) break;
	}
	if(j == cp->num_entries) continue;

	entry = cp->entry[j];
	if(entry->type != CPLOT_DATA) continue;

	entry->w->scaled_x0 = scaled_x0[i];
	if((z = (CPlotWidget)ax->mag_to) != NULL) {
	    ze = z->c_plot.entry[j];
	    ze->w->scaled_x0 = entry->w->scaled_x0;
	}
    }
    AdjustHorizontalLimits(w);
	    
    if(z != NULL) AdjustHorizontalLimits(z);

    c.reason = CPLOT_DATA_POSITION;
    c.wvec.load(wvec);
    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);
}

/** 
 *  Set the x or time position of the indicated waveforms.
 */
void CPlotClass::positionX2(gvector<Waveform *> &wvec,vector<double> &scaled_x0,
				double x0_min, double x0_max)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i, j;
    DataEntry *entry, *ze;
    CPlotWidget z;
    CPlotPositionCallbackStruct c;

    if(wvec.size() <= 0) return;

    if((int)scaled_x0.size() < wvec.size()) {
       cerr << "CPlotClass.positionX2: scaled_x0.size() < wvec.size()" << endl;
       return;
    }

    cp->data_shift = True;
    cp->data_min = x0_min;
    cp->data_max = x0_max;

    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < cp->num_entries; j++) {
	    if(cp->entry[j]->w == wvec[i]) break;
	}
	if(j == cp->num_entries) continue;

	entry = cp->entry[j];
	if(entry->type != CPLOT_DATA) continue;

	entry->w->scaled_x0 = scaled_x0[i];
	if((z = (CPlotWidget)ax->mag_to) != NULL) {
	    ze = z->c_plot.entry[j];
	    ze->w->scaled_x0 = entry->w->scaled_x0;
	}
    }
    AdjustHorizontalLimits(w);
	    
    c.reason = CPLOT_DATA_POSITION;
    c.wvec.load(wvec);
    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);
}

/** 
 *  Set the y or vertical position of the indicated waveforms.
 */
void CPlotClass::positionY(gvector<Waveform *> &wvec, vector<double> &scaled_y0)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i, j;
    DataEntry *entry, *ze;
    CPlotWidget z = NULL;
    CPlotPositionCallbackStruct c;

    if(wvec.size() <= 0) return;

    if((int)scaled_y0.size() < wvec.size()) {
       cerr << "CPlotClass.positiony: scaled_y0.size() < wvec.size()" << endl;
       return;
    }

    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < cp->num_entries; j++) {
	    if(cp->entry[j]->w == wvec[i]) break;
	}
	if(j == cp->num_entries) continue;

	entry = cp->entry[j];
	if(entry->type != CPLOT_DATA) continue;

	entry->w->scaled_y0 = scaled_y0[i];
	if((z = (CPlotWidget)ax->mag_to) != NULL) {
	    ze = z->c_plot.entry[j];
	    ze->w->scaled_y0 = entry->w->scaled_y0;
	}
    }
    AdjustHorizontalLimits(w);
	    
    if(z != NULL) AdjustHorizontalLimits(z);

    c.reason = CPLOT_DATA_POSITION;
    c.wvec.load(wvec);
    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);
}

/** 
 *  Set the x,y position of the indicated waveforms.
 */
void CPlotClass::positionXY(gvector<Waveform *> &wvec,vector<double> &scaled_x0,
				vector<double> &scaled_y0)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i, j;
    DataEntry *entry, *ze;
    CPlotWidget z = NULL;
    CPlotPositionCallbackStruct c;

    if(wvec.size() <= 0) return;

    if((int)scaled_x0.size() < wvec.size()) {
       cerr << "CPlotClass.positionXY: scaled_x0.size() < wvec.size()" << endl;
       return;
    }
    if((int)scaled_y0.size() < wvec.size()) {
       cerr << "CPlotClass.positionXY: scaled_y0.size() < wvec.size()" << endl;
       return;
    }

    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < cp->num_entries; j++) {
		if(cp->entry[j]->w == wvec[i]) break;
	}
	if(j == cp->num_entries) continue;

	entry = cp->entry[j];
	if(entry->type != CPLOT_DATA) continue;

	entry->w->scaled_x0 = scaled_x0[i];
	entry->w->scaled_y0 = scaled_y0[i];
	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    ze = z->c_plot.entry[j];
	    ze->w->scaled_x0 = entry->w->scaled_x0;
	    ze->w->scaled_y0 = entry->w->scaled_y0;
	}
    }
    AdjustHorizontalLimits(w);
	    
    if(z != NULL) AdjustHorizontalLimits(z);

    c.reason = CPLOT_DATA_POSITION;
    c.wvec.load(wvec);
    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);
}

/** 
 *  Reorder the vertical positions of the waveforms. The vertical positions
 *  of the waveforms will ordered, from top down, by the order of the
 *  Waveform pointers in 'wvec'. Any waveforms not included in
 *  'wvec' will be appended at the bottom of the display.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] num The number of Waveform pointers in 'wvec'.
 *  @param[in] wvec An array of Waveform structure pointers associated
 *	with waveforms in this CPlotWidget.
 *  @param[in] redisplay If True, the data window is immediately updated. If
 *		False, the data window is not updated until a call to another
 *		CPlotWidget function that causes an update.
 */
void CPlotClass::setWaveformOrder(gvector<Waveform *> &wvec, bool redisplay)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    DataEntry **e;
    int i, j, n;

    if(cp->num_entries <= 0) return;

    if( !(e = (DataEntry **)MallocIt(w,
		cp->num_entries*sizeof(DataEntry *))) ) return;

    for(i = 0; i < cp->num_entries; i++) e[i] = cp->entry[i];

    n = 0;
    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < cp->num_entries; j++) {
	    if(e[j] != NULL && e[j]->w == wvec[i]) {
		cp->entry[n++] = e[j];
		e[j] = NULL;
	    }
	}
    }
    for(i = 0; i < cp->num_entries; i++) {
	if(e[i] != NULL) cp->entry[n++] = e[i];
    }
    Free(e);

    if(redisplay)
    {
	gvector<Waveform *> ws;
	for(i = 0; i < cp->num_entries; i++) {
	    double new_y0 = i + 1;
	    if(cp->entry[i]->w->scaled_y0 != new_y0) {
		cp->entry[i]->w->scaled_y0 = i+1;
		ws.push_back(cp->entry[i]->w);
	    }
	}
	if(ws.size() > 0)
	{
	    CPlotPositionCallbackStruct c;
	    CPlotWidget z;
	    _AxesRedraw((AxesWidget)w);
	    _CPlotRedraw(w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _CPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	    if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	    {
		z->axes.use_screen = False;
		_AxesRedraw((AxesWidget)z);
		_CPlotRedraw(z);
		_AxesRedisplayAxes((AxesWidget)z);
		_CPlotRedisplay(z);
		_AxesRedisplayXor((AxesWidget)z);
		z->axes.use_screen = True;
	    }
	    c.reason = CPLOT_DATA_POSITION;
	    c.wvec.load(ws);
	    XtCallCallbacks((Widget)w, XtNpositionCallback, &c);
	}
    }
}

static void
AdjustHorizontalLimits(CPlotWidget w)
{
	AxesPart *ax = &w->axes;

	AdjustHorizontalScrollbars(w);

	if(ax->zoom > 1 && (
	    (ax->x1[ax->zoom] < ax->x1[1] || ax->x1[ax->zoom] > ax->x2[1]) ||
	    (ax->x2[ax->zoom] < ax->x1[1] || ax->x2[ax->zoom] > ax->x2[1])))
	{
	    ax->zoom = 1;
	    ax->zoom_max = 1;
	}
	_AxesRedraw((AxesWidget)w);
	_CPlotRedraw(w);
	_AxesRedisplayAxes((AxesWidget)w);
	_CPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);

	if(ax->use_pixmap && XtWindow(w)) {
	    XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
	}
}

static void
AdjustHorizontalScrollbars(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	Boolean setZoomOne, curve_on;
	double x_min, x_max, xdif_old, xdif_new;
	double xmin, xmax, ymin, ymax;
	double curve_xmin=0., curve_xmax=0., curve_ymin=0., curve_ymax=0.;
	double tmin, tmax;
	int i;

	if(cp->num_entries <= 0) return;

	curve_on = False;
	for(i = 0; i < cp->num_curves; i++) if(cp->curve[i]->on) {
	    if(!curve_on) {
		curve_on = True;
		curve_xmin = cp->curve[i]->xmin;
		curve_xmax = cp->curve[i]->xmax;
		curve_ymin = cp->curve[i]->ymin;
		curve_ymax = cp->curve[i]->ymax;
	    }
	    else {
		if(curve_xmin > cp->curve[i]->xmin) {
		    curve_xmin = cp->curve[i]->xmin;
		}
		if(curve_xmax < cp->curve[i]->xmax) {
		    curve_xmax = cp->curve[i]->xmax;
		}
		if(curve_ymin > cp->curve[i]->ymin) {
		    curve_ymin = cp->curve[i]->ymin;
		}
		if(curve_ymax < cp->curve[i]->ymax) {
		    curve_ymax = cp->curve[i]->ymax;
		}
	    }
	}

	CPlotGetDataDuration(w, &tmin, &tmax);

	if(tmax < cp->tmin || tmin > cp->tmax || tmax < ax->x1[0]
		|| tmin > ax->x2[0])
	{
	    setZoomOne = True;
	}
	else {
	    setZoomOne = False;
	}

	cp->tmin = tmin;
	cp->tmax = tmax;
	_CPlotGetXLimits(w, &x_min, &x_max);

	if(curve_on) {
	    if(curve_xmin < x_min) x_min = curve_xmin;
	    if(curve_xmax > x_max) x_max = curve_xmax;
	}

	if(!setZoomOne) {
	    if(x_min > ax->x1[ax->zoom]) x_min = ax->x1[ax->zoom];
	    if(x_max < ax->x2[ax->zoom]) x_max = ax->x2[ax->zoom];
	}

	/* shift cursors */
	xdif_old = ax->x2[0] - ax->x1[0];
	xdif_new = (x_max - x_min);
	for(i = 0; i < ax->num_cursors; i++) {
	    AxesCursor *c = &ax->cursor[i];
	    if(c->scaled_x < x_min || c->scaled_x > x_max) {
		double f = (c->scaled_x - ax->x1[0])/xdif_old;
		double wd = c->scaled_x2 - c->scaled_x1;
		c->scaled_x = x_min + f*xdif_new;
		f = (c->scaled_x1 - ax->x1[0])/xdif_old;
		c->scaled_x1 = x_min + f*xdif_new;
		c->scaled_x2 = c->scaled_x1 + wd;
	    }
	}

	if(cp->move_entry == NULL) {
	    GetScale(&ax->d, &xmin, &ymin, &xmax, &ymax);
	    xmin += (x_min - ax->x1[0]);
	    xmax += (x_min - ax->x1[0]);
	    SetScale(&ax->d, xmin, ymin, xmax, ymax);
	}
	ax->x1[0] = x_min;
	ax->x2[0] = x_max;

	if(!cp->data_shift && setZoomOne) {
	    ax->x1[1] = x_min;
	    ax->x2[1] = x_max;
	}
	else {
	    _Axes_AdjustScrollBars((AxesWidget)w);
	}
}

/** 
 *  Get the minimum and maximum times for all data samples as they are
 *  currently displayed in the data window. That is, get the earliest
 *  beginning and the latest ending of all the waveforms.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] tmin The epochal minimum time.
 *  @param[in] tmax The epochal maximum time.
 *  @returns True if this CPlotWidget contains any waveforms, otherwise
 *		return False.
 *  
 */
bool CPlotGetDataDuration(CPlotWidget w, double *tmin, double *tmax)
{
    CPlotPart *cp = &w->c_plot;
    int i;
    bool data_on = false;

    if(cp->num_entries > 0) {
	*tmin = cp->entry[0]->w->scaled_x0;
	*tmax = *tmin + cp->entry[0]->ts->duration();
	data_on = True;
    }
    else {
	*tmin = 0.;
	*tmax = 300.;
    }
    for(i = 1; i < cp->num_entries; i++) {
	DataEntry *e= cp->entry[i];
	double t1 = e->w->scaled_x0;
	double t2 = t1 + e->ts->duration();
	if(*tmin > t1) *tmin = t1;
	if(*tmax < t2) *tmax = t2;
    }
    if(cp->data_shift) {
	if(*tmin > cp->data_min && *tmin < cp->data_max) {
	    *tmin = cp->data_min;
	}
	if(*tmax < cp->data_max && *tmax > cp->data_min) {
	    *tmax = cp->data_max;
	}
    }
    return data_on;
}

/** 
 *  Update the data window after modifying the sample values of one or
 *  more waveforms.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] num_waveforms The number of waveforms referenced in 'wvec'.
 *  @param[in] wvec An array of Waveform structure pointers associated
 *		to waveforms contained in this CPlotWidget.
 *  @param[in] new_tags 'num_waveforms' strings that will be the new waveform
 *		label. If not new tags are needed, 'new_tags' can be
 *		(char**)NULL. Also, an element of 'new_tags' can be
 *		(char *)NULL, indicating no new tag for the corresponding
 *		waveform.
 *  @param[in] redisplay if True, repaint the window.
 */
void CPlotClass::modify(gvector<Waveform *> &wvec, char **new_tags,
		bool redisplay)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    AxesPart *ax = &w->axes;
    int i, m;
    bool calib_applied;
    double old_ymin, old_ymax, y_factor;
    DataEntry *entry, *ze;
    CPlotWidget z;
    CPlotModifyCallbackStruct c;

    z = (CPlotWidget)ax->mag_to;

    ax->use_screen = False;
    for(i = 0; i < wvec.size(); i++)
    {
	for(m = 0; m < cp->num_entries; m++) {
		if(cp->entry[m]->w == wvec[i]) break;
	}
	if(m == cp->num_entries) continue;

	entry = cp->entry[m];
	if(entry->type != CPLOT_DATA) continue;

	_CPlotRedisplayEntry(w, entry);

	if(wvec[i]->ts != entry->ts) {
	    if(entry->use_ts==entry->ts) entry->use_ts = wvec[i]->ts;
	    if(entry->ts) entry->ts->removeOwner(cp->owner_object);
	    entry->ts = wvec[i]->ts;
	    entry->ts->addOwner(cp->owner_object);
	}

	entry->w->scaled_x0 -= entry->tbeg - entry->ts->tbeg();
	entry->tbeg = entry->ts->tbeg();
	entry->tend = entry->ts->tend();
	entry->length = entry->ts->tend() - entry->tbeg;
	entry->mean = entry->ts->mean();

	calib_applied = entry->calib_applied;

	prepEntry(w, entry);

	if(calib_applied != entry->calib_applied)
	{
	    double amp_time = entry->ts->tbeg() + entry->left_side;
	    GSegment *s = entry->ts->nearestSegment(amp_time);
	    double calib = (s->calib() != 0.) ? s->calib() : 1.;
	    if(entry->calib_applied) {
		entry->bottom_side *= calib;
		entry->amp_ts *= calib;
		entry->zp_ts *= calib;
	    }
	    else {
		entry->bottom_side /= calib;
		entry->amp_ts /= calib;
		entry->zp_ts /= calib;
	    }
	}

	if(new_tags != NULL && new_tags[i] != NULL)
	{
	    stringcpy(entry->tag, new_tags[i], sizeof(entry->tag));
	    DrawTag(w, entry);
	}
	old_ymin = entry->ymin;
	old_ymax = entry->ymax;
	entry->ymin = entry->ts->dataMin() - entry->mean;
	entry->ymax = entry->ts->dataMax() - entry->mean;
	y_factor = fabs(old_ymax - old_ymin)/fabs(entry->ymax - entry->ymin);
	entry->yscale *= y_factor;

	entry->modified = True;
	_CPlotRedrawEntry(w, entry);
	_CPlotRedisplayEntry(w, entry);
	if(z != NULL)
	{
	    ze = z->c_plot.entry[m];
	    _CPlotRedisplayEntry(z, ze);
	    if(new_tags != NULL && new_tags[i] != NULL)
	    {
		stringcpy(ze->tag, new_tags[i],sizeof(ze->tag));
		DrawTag(z, z->c_plot.entry[m]);
	    }
	    ze->ts = entry->ts;
	    memcpy(ze->p, entry->p, MAX_DECI*sizeof(DeciArray));
	    ze->tbeg = entry->tbeg;
	    ze->tend = entry->tend;
	    ze->length = entry->length;
	    ze->mean = entry->mean;
	    ze->ymin = entry->ymin;
	    ze->ymax = entry->ymax;
	    ze->yscale *= y_factor;
	    z->c_plot.entry[m]->modified = True;
	    _CPlotRedrawEntry(z, z->c_plot.entry[m]);
	    _CPlotRedisplayEntry(z, z->c_plot.entry[m]);
	}
    }
    if(redisplay && ax->use_pixmap && XtWindow(w)) {
	XCopyArea(XtDisplay(w), ax->pixmap, XtWindow(w), ax->axesGC,
			0, 0, w->core.width, w->core.height, 0, 0);
    }
    ax->use_screen = True;
    c.wvec.load(wvec);
    XtCallCallbacks((Widget)w, XtNmodifyWaveformCallback, &c);

    if(wvec.size() > 0) {
	change.waveform = true;
	doDataChangeCallbacks();
    }
}

/** 
 *  Add an origin to a CPlotWidget. The CssOriginClass is associated with any
 *  waveforms for which an appropriate CssWftagClass is found.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] origin A CssOriginClass to be added to this CPlotWidget.
 *  @returns True if this widget does not already contain the CssOriginClass,
 *		otherwise return False.
 */
static bool
putOrigin(CPlotWidget w, CssOriginClass *origin)
{
    CPlotPart *cp = &w->c_plot;

    // check for duplicate
    for(int i = 0; i < cp->v->origins.size(); i++) {
	if(*origin == *cp->v->origins[i]) return false;
    }
    cp->v->origins.push_back(origin);
    OriginToData(w, origin);
    if(cp->v->origins.size() == 1) {
	cp->working_orid = origin->orid;
    }
    return true;
}

/** 
 *  Get the CssOriginClass that would be associated with the indicated GTimeSeries.
 *  The GTimeSeries does not need to be a waveform in the CPlotWidget.
 *  The CssWftagClasss are searched to find an association.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] ts A TimeSeries object.
 *  @returns A CssOriginClass associated with 'ts' or NULL if none is found.
 */
CssOriginClass * CPlotClass::dataToOrigin(GTimeSeries *ts)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j, k;

    if( !ts->waveform_io ) return NULL;

    for(i = 0; i < (int)ts->waveform_io->wp.size(); i++)
    {
	for(j = 0; j < cp->v->wftags.size(); j++)
	{
	    if(cp->v->wftags[j]->wfid == ts->waveform_io->wp[i].wf.wfid)
	    {
		for(k = 0; k < cp->v->origins.size(); k++)
		{
		    if(cp->v->origins[k]->orid == cp->v->wftags[j]->tagid) {
			return cp->v->origins[k];
		    }
		}
	    }
	}
    }
    return (CssOriginClass *)NULL;
}

static void
OriginToData(CPlotWidget w, CssOriginClass *origin)
{
	CPlotPart *cp = &w->c_plot;
	vector<WfdiscPeriod> *wp;
	int i, j, k;
	long orid;

	orid = origin->orid;

	for(i = 0; i < cp->v->wftags.size(); i++)
	{
	    CssWftagClass *wftag = cp->v->wftags[i];
	    if(wftag->tagid == orid)
	    {
		for(j = 0; j < cp->num_entries; j++)
		{
		    DataEntry *entry = cp->entry[j];
		    wp = entry->ts->getWfdiscPeriods();
		    for(k = 0; wp && k < (int)wp->size(); k++)
		    {
			if(wftag->wfid == wp->at(k).wf.wfid) {
			    entry->origin = origin;
			    break;
			}
		    }
		}
	    }
	}
}

static void
WftagToData(CPlotWidget w, CssWftagClass *wftag)
{
	CPlotPart *cp = &w->c_plot;
	int i, j, k;
	vector<WfdiscPeriod> *wp;
	DataEntry *entry;

	for(i = 0; i < cp->v->origins.size(); i++)
	    if(wftag->tagid == cp->v->origins[i]->orid)
	{
	    CssOriginClass *origin = cp->v->origins[i];

	    for(j = 0; j < cp->num_entries; j++)
	    {
		entry = cp->entry[j];
		wp = entry->ts->getWfdiscPeriods();
		for(k = 0; wp && k < (int)wp->size(); k++)
		{
		    if(wftag->wfid == wp->at(k).wf.wfid) {
			entry->origin = origin;
			break;
		    }
		}
	    }
	}
}

/** 
 *  Set the primary origin association for the indicated waveform. The
 *  primary origin is used to compute source to station distance.
 *  This assocation overrides any CssWftagClass associations.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd The Waveform structure pointer for a waveform in this
 *		CPlotWidget.
 *  @param origin A CssOriginClass to be the primary origin for the waveform.
 *  @returns true For success. return false if the indicated Waveform structure
 *		'cd' does not refer to any waveforms in this CPlotWidget.
 */
bool CPlotClass::setPrimaryOrigin(Waveform *cd, CssOriginClass *origin)
{
    CPlotPart *cp = &cw->c_plot;
    int i;
    DataEntry *entry;

    for(i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->w == cd) break;
    }
    if(i == cp->num_entries) return false;
    entry = cp->entry[i];

    entry->origin = origin;

    putOrigin(cw, origin);

    change.primary_origin = true;
    doDataChangeCallbacks();

    return true;
}

void CPlotClass::setWorkingOrid(long orid, bool do_callback)
{
    cw->c_plot.working_orid = orid;
    change.working_orid = true;
    if(do_callback) doDataChangeCallbacks();
}

long CPlotClass::getWorkingOrid()
{
    return cw->c_plot.working_orid;
}

/** 
 *  Get the primary origin associated with the indicated waveform.
 *  @param[in] w A Waveform pointer.
 *  @param[in] cd The Waveform structure pointer for a waveform in this
 *		CPlotWidget.
 *  @returns A CssOriginClass object associated with 'cd', or return NULL if there
 *		if no GObject is associated with 'cd'.
 */
CssOriginClass * CPlotClass::getPrimaryOrigin(Waveform *cd)
{
    CPlotPart *cp = &cw->c_plot;
    CssOriginClass *primary_o, *working_origin;
    int i, j;

    for(i = 0; i < cp->num_entries && cd != cp->entry[i]->w; i++);
    if(i == cp->num_entries) return NULL;
    primary_o = cp->entry[i]->origin;

    if(cp->working_orid == 0) return primary_o;

    for(i = 0; i < cp->v->origins.size() &&
	cp->v->origins[i]->orid != cp->working_orid; i++);
    if(i == cp->v->origins.size()) {
        return primary_o;
    }
    working_origin = cp->v->origins[i];

    if(working_origin == primary_o) return primary_o;

    if(working_origin->time < cd->ts->tend() &&
        working_origin->time > cd->ts->tbeg() - 4*60*60) return working_origin;

    if(working_origin == dataToOrigin(cd->ts)) return working_origin;

    for(i = 0; i < cp->v->assocs.size() &&
	cp->v->assocs[i]->orid != working_origin->orid; i++);
    if(i < cp->v->assocs.size()) {
        for(j = 0; j < cp->v->arrivals.size() &&
		cp->v->arrivals[j]->arid != cp->v->assocs[i]->arid; j++);
        if(j < cp->v->arrivals.size()) {
            CssArrivalClass *a = cp->v->arrivals[j];
            if((!strcasecmp(cd->ts->sta(), a->sta)
		|| !strcasecmp(cd->ts->net(), a->sta))
                && cd->ts->tbeg() <= a->time && cd->ts->tend() >= a->time)
            {
                return working_origin;
            }
        }
    }

    return primary_o;
}

void CPlotClass::putTable(CssTableClass *table, bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    int i;

    if(table->nameIs(cssArrival)) {
	if(CPlotPutArrival((CssArrivalClass *)table, stringToPixel("black"))){
	    change.arrival = true;
	    if(do_callback) doDataChangeCallbacks();
	    TableListener::addListener(table, this);
	}
    }
    else if(table->nameIs(cssOrigin)) {
	if( putOrigin(cw, (CssOriginClass *)table) ) {
	    change.origin = true;
	    cvector<CssOriginClass> *o = getOriginRef();
	    if(o->size() == 1) {
		change.working_orid = true;
	    }
	    if(do_callback) doDataChangeCallbacks();
	}
    }
    else if(table->nameIs(cssAssoc)) {
	for(i = 0; i < cp->v->assocs.size(); i++) {
	    if(*((CssAssocClass *)table) == *cp->v->assocs[i]) return;
	}
	cp->v->assocs.push_back((CssAssocClass *)table);
	if(cp->v->assocs.size() == 1 && cp->v->origins.size() == 0) {
	    cp->working_orid = ((CssAssocClass *)table)->orid;
	    change.working_orid = true;
	}
	change.assoc = true;
	if(do_callback) doDataChangeCallbacks();
	TableListener::addListener(table, this);
    }
    else if(table->nameIs(cssOrigerr)) {
	// check for duplicate
	for(i = 0; i < cp->v->origerrs.size(); i++) {
	    if(*((CssOrigerrClass *)table) == *cp->v->origerrs[i]) return;
	}
	cp->v->origerrs.push_back((CssOrigerrClass *)table);
	if(cp->v->origerrs.size() == 1 && cp->v->origins.size() == 0) {
	    cp->working_orid = ((CssOrigerrClass *)table)->orid;
	    change.working_orid = true;
	}
	change.origerr = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssStamag)) {
	for(i = 0; i < cp->v->stamags.size(); i++) {
	    if(*((CssStamagClass *)table) == *cp->v->stamags[i]) return;
	}
	cp->v->stamags.push_back((CssStamagClass *)table);
	if(cp->v->stamags.size() == 1 && cp->v->origins.size() == 0) {
	    cp->working_orid = ((CssStamagClass *)table)->orid;
	    change.working_orid = true;
	}
	change.stamag = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssStassoc)) {
	for(i = 0; i < cp->v->stassocs.size(); i++) {
	    if(*((CssStassocClass *)table) == *cp->v->stassocs[i]) return;
	}
	cp->v->stassocs.push_back((CssStassocClass *)table);
	change.stassoc = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssAmplitude)) {
	for(i = 0; i < cp->v->amplitudes.size(); i++) {
	    if(*((CssAmplitudeClass *)table) == *cp->v->amplitudes[i]) return;
	}
	cp->v->amplitudes.push_back((CssAmplitudeClass *)table);
	change.amplitude = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssNetmag)) {
	for(i = 0; i < cp->v->netmags.size(); i++) {
	    if(*((CssNetmagClass *)table) == *cp->v->netmags[i]) return;
	}
	cp->v->netmags.push_back((CssNetmagClass *)table);
	if(cp->v->netmags.size() == 1 && cp->v->origins.size() == 0) {
	    cp->working_orid = ((CssNetmagClass *)table)->orid;
	    change.working_orid = true;
	}
	change.netmag = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssAmpdescript)) {
	for(i = 0; i < cp->v->ampdescripts.size(); i++) {
	    if(*((CssAmpdescriptClass *)table) == *cp->v->ampdescripts[i]) return;
	}
	cp->v->ampdescripts.push_back((CssAmpdescriptClass *)table);
	change.ampdescript = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssFilter)) {
	for(i = 0; i < cp->v->filters.size(); i++) {
	    if(*((CssFilterClass *)table) == *cp->v->filters[i]) return;
	}
	cp->v->filters.push_back((CssFilterClass *)table);
	change.filter = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssInfraFeatures)) {
	for(i = 0; i < cp->v->infra_features.size(); i++) {
	    if(*((CssInfraFeaturesClass *)table)== *cp->v->infra_features[i]) return;
	}
	cp->v->infra_features.push_back((CssInfraFeaturesClass *)table);
	change.infra = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssHydroFeatures)) {
	for(i = 0; i < cp->v->hydro_features.size(); i++) {
	    if(*((CssHydroFeaturesClass *)table)== *cp->v->hydro_features[i]) return;
	}
	cp->v->hydro_features.push_back((CssHydroFeaturesClass *)table);
	change.hydro = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssParrival)) {
	for(i = 0; i < cp->v->parrivals.size(); i++) {
	    if(*((CssParrivalClass *)table) == *cp->v->parrivals[i]) return;
	}
	cp->v->parrivals.push_back((CssParrivalClass *)table);
	if(cp->v->parrivals.size() == 1 && cp->v->origins.size() == 0) {
	    cp->working_orid = ((CssParrivalClass *)table)->orid;
	    change.working_orid = true;
	}
	change.parrival = true;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(table->nameIs(cssWftag)) {
	for(i = 0; i < cp->v->wftags.size(); i++) {
	    if(*((CssWftagClass *)table) == *cp->v->wftags[i]) return;
	}
	cp->v->wftags.push_back((CssWftagClass *)table);
	WftagToData(cw, (CssWftagClass *)table);
	change.wftag = true;
	if(do_callback) doDataChangeCallbacks();
    }
}

void CPlotClass::removeTable(CssTableClass *table, bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    const char *name = table->getName();
    int i;

    if(!strcmp(name, cssOrigin)) {
	if(cw->c_plot.working_orid == ((CssOriginClass *)table)->orid) {
	    if(do_callback) change.working_orid = true;
	}
    }
    else if(!strcmp(name, cssArrival)) {
	TableListener::removeListener(table, this);
    }

    cp->v->selected_tables.remove(table);

    for(i = 0; i < cp->v->arrivals.size(); i++) {
	if((CssArrivalClass *)table == cp->v->arrivals[i]) {
	    removeArrival((CssArrivalClass *)table);
	    if(do_callback) {
		change.arrival = true;
		doDataChangeCallbacks();
	    }
	    return;
	}
    }
    for(i = 0; i < cp->v->origins.size(); i++) {
	if((CssOriginClass *)table == cp->v->origins[i]) {
	    removeOrigin((CssOriginClass *)table, do_callback);
	    return;
	}
    }
    if(cp->v->origerrs.remove((CssOrigerrClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->assocs.remove((CssAssocClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->stassocs.remove((CssStassocClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->hydro_features.remove((CssHydroFeaturesClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->infra_features.remove((CssInfraFeaturesClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->stamags.remove((CssStamagClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->netmags.remove((CssNetmagClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->amplitudes.remove((CssAmplitudeClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->ampdescripts.remove((CssAmpdescriptClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->parrivals.remove((CssParrivalClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->filters.remove((CssFilterClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else if(cp->v->wftags.remove((CssWftagClass *)table)) {
	if(do_callback) change.origerr = true;
    }
    else {
	return;
    }
    if(do_callback) doDataChangeCallbacks();
}

/** 
 *  Remove an origin from a CPlotWidget.
 *  @param[in] w A Waveform pointer.
 *  @param[in] origin The CssOriginClass to be removed.
 */
void CPlotClass::removeOrigin(CssOriginClass *origin, bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;

    for(int i = 0; i < cp->num_entries; i++) {
	if(cp->entry[i]->origin == origin) {
	    cp->entry[i]->origin = NULL;
	}
    }
    cp->v->selected_tables.remove(origin);
    if(origin->orid == cp->working_orid) {
	cp->working_orid = 0;
    }
    if(cp->v->origins.remove(origin)) {
	if(do_callback) {
	    change.origin = true;
	    doDataChangeCallbacks();
	}
    }
}

void CPlotClass::clearTable(string &table_name, bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    int i;

    if(!table_name.compare(cssAmpdescript)) {
	if(do_callback) change.ampdescript = (cp->v->ampdescripts.size() > 0);
	for(i = 0; i < cp->v->ampdescripts.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->ampdescripts[i]);
	}
	cp->v->ampdescripts.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssAmplitude)) {
	if(do_callback) change.amplitude = (cp->v->amplitudes.size() > 0);
	for(i = 0; i < cp->v->amplitudes.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->amplitudes[i]);
	}
	cp->v->amplitudes.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssAssoc)) {
	if(do_callback) change.assoc = (cp->v->assocs.size() > 0);
	for(i = 0; i < cp->v->assocs.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->assocs[i]);
	}
	cp->v->assocs.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssFilter)) {
	if(do_callback) change.filter = (cp->v->filters.size() > 0);
	for(i = 0; i < cp->v->filters.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->filters[i]);
	}
	cp->v->filters.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssHydroFeatures)) {
	if(do_callback) change.hydro = (cp->v->hydro_features.size() > 0);
	for(i = 0; i < cp->v->hydro_features.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->hydro_features[i]);
	}
	cp->v->hydro_features.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssInfraFeatures)) {
	if(do_callback) change.infra = (cp->v->infra_features.size() > 0);
	for(i = 0; i < cp->v->infra_features.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->infra_features[i]);
	}
	cp->v->infra_features.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssNetmag)) {
	if(do_callback) change.netmag = (cp->v->netmags.size() > 0);
	for(i = 0; i < cp->v->netmags.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->netmags[i]);
	}
	cp->v->netmags.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssOrigerr)) {
	if(do_callback) change.origerr = (cp->v->origerrs.size() > 0);
	for(i = 0; i < cp->v->origerrs.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->origerrs[i]);
	}
	cp->v->origerrs.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssOrigin)) {
	if(do_callback) change.origin = (cp->v->origins.size() > 0);
	for(i = 0; i < cp->v->origins.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->origins[i]);
	}
	cp->v->origins.clear();
	 for(i = 0; i < cp->num_entries; i++) cp->entry[i]->origin = NULL;
	cp->working_orid = 0;
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssParrival)) {
	if(do_callback) change.parrival = (cp->v->parrivals.size() > 0);
	for(i = 0; i < cp->v->parrivals.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->parrivals[i]);
	}
	cp->v->parrivals.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssStamag)) {
	if(do_callback) change.stamag = (cp->v->stamags.size() > 0);
	for(i = 0; i < cp->v->stamags.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->stamags[i]);
	}
	cp->v->stamags.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssStassoc)) {
	if(do_callback) change.stassoc = (cp->v->stassocs.size() > 0);
	for(i = 0; i < cp->v->stassocs.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->stassocs[i]);
	}
	cp->v->stassocs.clear();
	if(do_callback) doDataChangeCallbacks();
    }
    else if(!table_name.compare(cssWftag)) {
	if(do_callback) change.wftag = (cp->v->wftags.size() > 0);
	for(i = 0; i < cp->v->wftags.size(); i++) {
	    cp->v->selected_tables.remove(cp->v->wftags[i]);
	}
	cp->v->wftags.clear();
	if(do_callback) doDataChangeCallbacks();
    }
}

/** 
 *  Add a CPlotArrivalType to a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] type A CPlotArrivalType to be added.
 *  @returns True, if the CPlotArrivalType is not already in the CPlotWidget,
 * 		otherwise return False.
 */
bool CPlotClass::addArrivalType(CPlotArrivalType *a)
{
    CPlotPart *cp = &cw->c_plot;

    // check for duplicate
    for(int i = 0; i < cp->v->arrival_types.size(); i++) {
	if(*a == *cp->v->arrival_types[i]) return false;
    }
    CPlotArrivalType *arr = new CPlotArrivalType();
    *arr = *a;
    cp->v->arrival_types.push_back(arr);
    return true;
}

/** 
 *  Add an arrival to a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival The CssArrivalClass to be added.
 *  @param[in] fg The pixel color of the arrival label.
 *  @returns True, if the CssArrivalClass is not already in the CPlotWidget,
 *  		otherwise returns False.
 */
bool CPlotClass::CPlotPutArrival(CssArrivalClass *arrival, Pixel fg)
{
    CPlotPart *cp = &cw->c_plot;
    ArrivalEntry *a_entry;
    CPlotWidget	z;

    // check for duplicate
    for(int i = 0; i < cp->v->arrivals.size(); i++) {
	if(*arrival == *cp->v->arrivals[i]) return false;
    }
    cp->v->arrivals.push_back(arrival);

    a_entry = new ArrivalEntry(arrival);
    cp->v->arrival_entries.push_back(a_entry);

    a_entry->fg = (fg != 0) ? fg : cw->axes.fg;

    DrawArrivalLabel(cw, a_entry);

    ArrivalToData(cw, a_entry);

    if((z = (CPlotWidget)cw->axes.mag_to) == NULL) {
	z = (CPlotWidget)cw->axes.mag_from;
    }
    if(z != NULL) {
	ArrivalToData(z, a_entry);
    }
    DisplayOneArrival(cw, a_entry);

    return true;
}

static void
DisplayOneArrival(CPlotWidget w, ArrivalEntry *a_entry)
{
    CPlotPart *cp = &w->c_plot;
    int i, j;
    CPlotWidget z = (CPlotWidget)w->axes.mag_to;

    if(cp->display_arrivals != CPLOT_ARRIVALS_OFF)
    {
	for(i = 0; i < cp->num_entries; i++)
	{
	    for(j = 0; j < cp->entry[i]->n_arrlab; j++)
		if(cp->entry[i]->arrlab[j].a_entry == a_entry)
	    {
		DrawOneArrival(w, cp->entry[i], j, a_entry->selected,False);
		if(z != NULL) {
		    DrawOneArrival(z, z->c_plot.entry[i], j, a_entry->selected,
				False);
		}
	    }
	}
    }
}

void CPlotClass::colorArrival(CssArrivalClass *arrival, Pixel fg)
{
    CPlotPart *cp = &cw->c_plot;
    int i;
    for(i = 0; i < cp->v->arrival_entries.size(); i++)
    {
	ArrivalEntry *a_entry = cp->v->arrival_entries[i];
	if(a_entry->a == arrival)
	{
	    a_entry->fg = (fg != 0) ? fg : cw->axes.fg;
	    DrawArrivalLabel(cw, a_entry);
	    DisplayOneArrival(cw, a_entry);
	    return;
	}
    }
}

void CPlotClass::setArrivalModified(CssArrivalClass *a, bool modified)
{
    CPlotPart *cp = &cw->c_plot;

    for(int i = 0; i < cp->v->arrival_entries.size(); i++) {
	ArrivalEntry *ae = cp->v->arrival_entries[i];
	if(ae->a == a) {
	    DisplayOneArrival(cw, ae);
	    ae->modified = modified;
	    DrawArrivalLabel(cw, ae);
	    DisplayOneArrival(cw, ae);
	}
    }
}

static void
DrawArrivalLabel(CPlotWidget w, ArrivalEntry *a_entry)
{
	CPlotPart	*cp = &w->c_plot;
	int		ascent, descent, direction, n, x, y, len;
	char		surrogate[9];
	XCharStruct	overall;
	Pixmap		pixmap;
	XImage		*image;
	CssArrivalClass	*a = a_entry->a;
	char		*p = arrivalPhase(a);

	if( a_entry->modified ) {
	    snprintf(cp->arrival_name, sizeof(cp->arrival_name), "%s*", p);
	    p = cp->arrival_name;
	}

	Free(a_entry->points);
	a_entry->npoints = 0;

	if( !XtIsRealized((Widget)w) ) return;

	len = (int)strlen(p);

	if (len > 0) {
	    if(!a_entry->associated) {
		XTextExtents(cp->arrival_font, p, (int)strlen(p),
			&direction, &ascent, &descent, &overall);
	    }
	    else {
		XSetFont(XtDisplay(w), cp->arrivalGC,
			cp->associated_arrival_font->fid);
		XTextExtents(cp->associated_arrival_font, p, (int)strlen(p),
			&direction, &ascent, &descent, &overall);
	    }
	}
	else {
	    stringcpy(surrogate, "P", sizeof(surrogate));
	    if(!a_entry->associated) {
		XTextExtents(cp->arrival_font, surrogate,(int)strlen(surrogate),
			&direction, &ascent, &descent, &overall);
	    }
	    else {
		XSetFont(XtDisplay(w), cp->arrivalGC,
			cp->associated_arrival_font->fid);
		XTextExtents(cp->associated_arrival_font, surrogate,
			(int)strlen(surrogate), &direction, &ascent,
			&descent, &overall);
	    }
	}

	a_entry->width  = overall.width;
	a_entry->height = overall.descent + overall.ascent;
	a_entry->ascent = overall.ascent;

	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
        pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
			a_entry->width+1, a_entry->height+1,
                        DefaultDepth(XtDisplay(w),DefaultScreen(XtDisplay(w))));
        XSetFunction(XtDisplay(w), cp->arrivalGC, GXclear);
        XFillRectangle(XtDisplay(w), pixmap, cp->arrivalGC, 0, 0,
			a_entry->width, a_entry->height);
        XSetFunction(XtDisplay(w), cp->arrivalGC, GXcopy);
        XSetForeground(XtDisplay(w), cp->arrivalGC, 1);

	if (len > 0)
        {
            XDrawString(XtDisplay(w), pixmap, cp->arrivalGC, 0,
			a_entry->ascent, p, (int)strlen(p));
	}
	XSetForeground(XtDisplay(w), cp->arrivalGC, a_entry->fg);

	image = XGetImage(XtDisplay(w), pixmap, 0, 0, a_entry->width,
			a_entry->height, AllPlanes, XYPixmap);

	a_entry->points = (XPoint *)MallocIt(w,
		a_entry->width*a_entry->height*sizeof(XPoint));

	for(x = n = 0; x < a_entry->width; x++)
	{
	    for(y = 0; y < a_entry->height; y++)
	    {
		if(XGetPixel(image, x, y)) {
		    a_entry->points[n].x = x;
		    a_entry->points[n].y = y;
		    n++;
		}
	    }
	}
	a_entry->npoints = n;
	XFreePixmap(XtDisplay(w), pixmap);
	XDestroyImage(image);

	if(a_entry->associated) {
	    XSetFont(XtDisplay(w), cp->arrivalGC, cp->arrival_font->fid);
	}
}

static void
FindArrivalsOnData(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	int	  i, j, dc=0, id=0, copy=0;

	for(i = 0; i < cp->v->arrival_entries.size(); i++)
	{
	    ArrivalEntry *a_entry = cp->v->arrival_entries[i];
	    CssArrivalClass *a = a_entry->a;

	    if((!strcasecmp(entry->ts->sta(), a->sta) ||
		!strcasecmp(entry->ts->net(), a->sta)) &&
		entry->tbeg <= a->time && entry->ts->tend() >= a->time)
	    {
		entry->ts->getValue("dc", &dc);
		entry->ts->getValue("id", &id);
		entry->ts->getValue("copy", &copy);
		if(a->ts_dc > 0 && (dc != a->ts_dc || id != a->ts_id
			|| copy != a->ts_copy))
		{
		    continue;
		}

		for(j = 0; j < entry->n_arrlab; j++) {
		    if(entry->arrlab[j].a_entry == a_entry) break;
		}
		if(j < entry->n_arrlab) continue;

		if(entry->arrlab == NULL) {
	    	    entry->arrlab = (ArrivalLabel *)
					MallocIt(w, sizeof(ArrivalLabel));
		}
		else {
		    entry->arrlab = (ArrivalLabel *)realloc(entry->arrlab,
			   (entry->n_arrlab+1)*sizeof(ArrivalLabel));
		}
		entry->arrlab[entry->n_arrlab].a_entry = a_entry;
		entry->arrlab[entry->n_arrlab].done = False;
		entry->arrlab[entry->n_arrlab].x = 0;
		entry->arrlab[entry->n_arrlab].y = 0;
		entry->arrlab[entry->n_arrlab].y1 = 0;
		entry->arrlab[entry->n_arrlab].y2 = 0;
		entry->n_arrlab++;

		if((cp->display_arrivals == CPLOT_ARRIVALS_ONE_CHAN &&
		    DataSource::compareChan(entry->w->channel, a->chan)) ||
		    cp->display_arrivals == CPLOT_ARRIVALS_ALL_CHAN)
		{
		    FindOneArrival(w, entry, entry->n_arrlab-1);
		}
	    }
	}
}

static void
ArrivalToData(CPlotWidget w, ArrivalEntry *a_entry)
{
	CPlotPart	*cp = &w->c_plot;
	int		i, j, dc=0, id=0, copy=0;
	DataEntry	*entry;
	CssArrivalClass	*a = a_entry->a;

	for(i = 0; i < cp->num_entries; i++)
	{
	    entry = cp->entry[i];

	    if((!strcasecmp(entry->ts->sta(), a->sta) ||
	   	!strcasecmp(entry->ts->net(), a->sta))
		&& entry->tbeg <= a->time && entry->ts->tend() >= a->time)
	    {
		entry->ts->getValue("dc", &dc);
		entry->ts->getValue("id", &id);
		entry->ts->getValue("copy", &copy);
		if(a->ts_dc > 0 && (dc != a->ts_dc || id != a->ts_id
			|| copy != a->ts_copy))
		{
		    continue;
		}

		for(j = 0; j < entry->n_arrlab; j++) {
		    if(entry->arrlab[j].a_entry == a_entry) break;
		}
		if(j < entry->n_arrlab) continue;

		if(entry->arrlab == NULL) {
		    entry->arrlab = (ArrivalLabel *)
					MallocIt(w, sizeof(ArrivalLabel));
		}
		else
		{
		    entry->arrlab = (ArrivalLabel *)realloc(entry->arrlab,
			   (entry->n_arrlab+1)*sizeof(ArrivalLabel));
		}
		entry->arrlab[entry->n_arrlab].a_entry = a_entry;
		entry->arrlab[entry->n_arrlab].done = False;
		entry->arrlab[entry->n_arrlab].x = 0;
		entry->arrlab[entry->n_arrlab].y = 0;
		entry->arrlab[entry->n_arrlab].y1 = 0;
		entry->arrlab[entry->n_arrlab].y2 = 0;
		entry->n_arrlab++;

		if((cp->display_arrivals == CPLOT_ARRIVALS_ONE_CHAN &&
		    DataSource::compareChan(entry->w->channel, a->chan)) ||
		    cp->display_arrivals == CPLOT_ARRIVALS_ALL_CHAN)
		{
		    FindOneArrival(w, entry, entry->n_arrlab-1);
		}
	    }
	}
}

/** 
 *  
 */
void
_CPlotPredArrivals(CPlotWidget w, Waveform *cd, CPlotPredArr *pred_arr,
			int npred_arr)
{
	CPlotPart	*cp = &w->c_plot;
	int		i;
	DataEntry	*entry;
	CPlotWidget	z;

	if((z = (CPlotWidget)w->axes.mag_to) == NULL) {
	    z = (CPlotWidget)w->axes.mag_from;
	}
	for(i = 0; i < cp->num_entries; i++) {
	    if(cd == cp->entry[i]->w) break;
	}
	if(i == cp->num_entries) return;

	entry = cp->entry[i];

	if(cp->display_predicted_arrivals)
	{
	    _CPlotDrawPredArrivals(w, entry);
	    if(z != NULL) _CPlotDrawPredArrivals(z, z->c_plot.entry[i]);
	}
	if(npred_arr <= 0)
	{
	    entry->npred_arr = 0;
	    if(z != NULL) z->c_plot.entry[i]->npred_arr = 0;
	    return;
	}

	if(entry->pred_arr == NULL) {
	    if( npred_arr && !(entry->pred_arr = (CPlotPredArr *)MallocIt(w,
			npred_arr*sizeof(CPlotPredArr))) ) return;
	}
	else {
	    if(npred_arr && (entry->pred_arr = (CPlotPredArr *)
		realloc(entry->pred_arr, npred_arr*sizeof(CPlotPredArr))) == 0)
			return;
	}
	memcpy(entry->pred_arr, pred_arr, npred_arr*sizeof(CPlotPredArr));
	entry->npred_arr = npred_arr;

	FindPredArrivals(w, entry);
	_CPlotDrawPredArrivals(w, entry);

	if(z != NULL)
	{
	    DataEntry *ze = z->c_plot.entry[i];

	    if(ze->pred_arr == NULL) {
		if( npred_arr && !(ze->pred_arr = (CPlotPredArr *)MallocIt(w,
			npred_arr*sizeof(CPlotPredArr))) ) return;
	    }
	    else {
		if(npred_arr && (ze->pred_arr = (CPlotPredArr *)
		    realloc(ze->pred_arr, npred_arr*sizeof(CPlotPredArr))) == 0)
			return;
	    }
	    memcpy(ze->pred_arr, pred_arr, npred_arr*sizeof(CPlotPredArr));
	    ze->npred_arr = npred_arr;

	    FindPredArrivals(z, ze);
	    _CPlotDrawPredArrivals(z, ze);
	}
	return;
}

void CPlotClass::arrivalAssociated(CssArrivalClass *arrival, bool associated)
{
    CPlotPart *cp = &cw->c_plot;
    ArrivalEntry *a_entry=NULL;
    int i;

    for(i = 0; i < cp->v->arrival_entries.size(); i++) {
	a_entry = cp->v->arrival_entries[i];
	if(arrival == a_entry->a) break;
    }
    if(i == cp->v->arrival_entries.size()) return;

    a_entry->associated = associated;
    CPlotRenameArrival(cw, arrival, arrival->phase);
}

/** 
 *  Rename a CssArrivalClass in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival The CssArrivalClass to be renamed.
 *  @param[in] phase The new phase name for 'arrival'.
 */
void
CPlotRenameArrival(CPlotWidget w, CssArrivalClass *arrival, const string &phase)
{
    CPlotPart *cp = &w->c_plot;
    int i, j;
    ArrivalEntry *a_entry=NULL;
    DataEntry *entry, *ze;
    CPlotWidget z;

    if((z = (CPlotWidget)w->axes.mag_to) == NULL) {
	z = (CPlotWidget)w->axes.mag_from;
    }

    for(i = 0; i < cp->v->arrival_entries.size(); i++) {
	if(arrival == cp->v->arrival_entries[i]->a) {
	    a_entry = cp->v->arrival_entries[i];
	    break;
	}
    }
    if( !a_entry ) return;

    for(i = 0; i < cp->num_entries; i++)
    {
	entry = cp->entry[i];
	ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

	for(j = 0; j < entry->n_arrlab; j++)
		if(entry->arrlab[j].a_entry == a_entry)
	{
	    DrawOneArrival(w, entry, j, a_entry->selected, False);
	    if(ze != NULL) {
		DrawOneArrival(z, ze, j, a_entry->selected, False);
	    }
	}
    }

    stringcpy(arrival->phase, phase.c_str(), sizeof(arrival->phase));

    DrawArrivalLabel(w, a_entry);

    for(i = 0; i < cp->num_entries; i++)
    {
	entry = cp->entry[i];
	ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

	for(j = 0; j < entry->n_arrlab; j++)
	    if(entry->arrlab[j].a_entry == a_entry)
	{
	    FindOneArrival(w, entry, j);
	    DrawOneArrival(w, entry, j, a_entry->selected, False);
	    if(ze != NULL) {
		FindOneArrival(w, ze, j);
		DrawOneArrival(w, ze, j, a_entry->selected, False);
	    }
	}
    }
}

/** 
 *  Set label color for a CssArrivalClass in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival The CssArrivalClass whose color is to be set.
 *  @param[in] col The new color for 'arrival'.
 */
void CPlotClass::arrivalSetColor(CssArrivalClass *arrival, Pixel col)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j;
    ArrivalEntry *a_entry=NULL;
    DataEntry *entry, *ze;
    CPlotWidget	z;

    if((z = (CPlotWidget)cw->axes.mag_to) == NULL) {
	z = (CPlotWidget)cw->axes.mag_from;
    }

    for(i = 0; i < cp->v->arrival_entries.size(); i++)
    {
	if(arrival == cp->v->arrival_entries[i]->a) {
	    a_entry = cp->v->arrival_entries[i];
	    break;
	}
    }
    if( !a_entry ) return;

    for(i = 0; i < cp->num_entries; i++)
    {
	entry = cp->entry[i];
	ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

	for(j = 0; j < entry->n_arrlab; j++)
		if(entry->arrlab[j].a_entry == a_entry)
	{
	    DrawOneArrival(cw, entry, j, a_entry->selected, False);
	    if(ze != NULL) {
		DrawOneArrival(z, ze, j, a_entry->selected, False);
	    }		
	}
    }

    a_entry->fg = col;

    DrawArrivalLabel(cw, a_entry);

    for(i = 0; i < cp->num_entries; i++)
    {
	entry = cp->entry[i];
	ze = (z != NULL) ? z->c_plot.entry[i] : NULL;

	for(j = 0; j < entry->n_arrlab; j++)
		if(entry->arrlab[j].a_entry == a_entry)
	{
	    FindOneArrival(cw, entry, j);
	    DrawOneArrival(cw, entry, j, a_entry->selected, False);
	    if(ze != NULL) {
		FindOneArrival(cw, ze, j);
		DrawOneArrival(cw, ze, j, a_entry->selected, False);
	    }
	}
    }
}

/** 
 *  Remove an single CssArrivalClass from a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival The CssArrivalClass to be removed.
 */
void CPlotClass::removeArrival(CssArrivalClass *arrival)
{
    CPlotWidget w = cw;
    CPlotPart *cp = &w->c_plot;
    int i, j, k;
    ArrivalEntry *a_entry=NULL;
    CssArrivalClass *a;
    DataEntry *entry;
    CPlotWidget	z;

    if((z = (CPlotWidget)w->axes.mag_to) == NULL) {
	z = (CPlotWidget)w->axes.mag_from;
    }

    for(i = 0; i < cp->v->arrival_entries.size(); i++) {
	if(arrival == cp->v->arrival_entries[i]->a) {
	    a_entry = cp->v->arrival_entries[i];
	    break;
	}
    }
    if(!a_entry) return;

    a = a_entry->a;

    for(i = 0; i < cp->num_entries; i++)
    {
	for(j = 0; j < cp->entry[i]->n_arrlab; j++)
		if(cp->entry[i]->arrlab[j].a_entry == a_entry)
	{
	    entry = cp->entry[i];
	    DrawOneArrival(w, entry, j, a_entry->selected, False);
	    entry->n_arrlab--;

	    for(k = j; k < entry->n_arrlab; k++) {
		entry->arrlab[k] = entry->arrlab[k+1];
	    }
	    if(z != NULL)
	    {
		entry = z->c_plot.entry[i];
		DrawOneArrival(z, entry, j, a_entry->selected, False);
		entry->n_arrlab--;
		for(k = j; k < entry->n_arrlab; k++) {
		    entry->arrlab[k] = entry->arrlab[k+1];
		}
	    }
	    break;
	}
    }
    cp->v->arrival_entries.remove(a_entry);
    cp->v->selected_tables.remove(a);
    cp->v->arrivals.remove(a);
}

/** 
 *  Remove all CssArrivalClasss from a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 */
void CPlotClass::clearArrivals(bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    if(do_callback) change.arrival = (cp->v->arrival_entries.size() > 0);
    for(int i = cp->v->arrival_entries.size()-1; i >= 0; i--) {
	removeArrival(cp->v->arrival_entries[i]->a);
    }
    if(do_callback) doDataChangeCallbacks();
}

/** 
 *  Select or deselect an arrival that is contained in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] arrival A CssArrivalClass contained in this CPlotWidget.
 *  @param[in] select If True, select 'arrival', otherwise deselect it.
 */
bool CPlotClass::selectArrival(CssArrivalClass *arrival, bool select)
{
    return _CPlotSelectArrival(cw, arrival, select, False);
}

bool CPlotClass::selectArrivalWithCB(CssArrivalClass *arrival, bool select)
{
    return _CPlotSelectArrival(cw, arrival, select, True);
}

/** 
 *  
 */
static Boolean
_CPlotSelectArrival(CPlotWidget w, CssArrivalClass *arrival, Boolean selected,
		Boolean doCallback)
{
    CPlotPart *cp = &w->c_plot;
    int i;

    for(i = 0; i < cp->v->arrival_entries.size(); i++)
    {
	ArrivalEntry *a_entry = cp->v->arrival_entries[i];
	if(a_entry->a == arrival)
	{
	    if(a_entry->selected != selected) {
		DoSelectArrival(w, a_entry, doCallback);
	    }
	    return true;
	}
    }
    return false; // arrival not found
}

bool CPlotClass::arrivalState(CssArrivalClass *arrival)
{
    CPlotPart *cp = &cw->c_plot;

    for(int i = 0; i < cp->v->arrival_entries.size(); i++) {
	ArrivalEntry *a_entry = cp->v->arrival_entries[i];
	if(a_entry->a == arrival) return a_entry->selected;
    }
    return false;
}

static Boolean
select_arrival(CPlotWidget w, XEvent *event)
{
	CPlotPart	*cp = &w->c_plot;
	int		i, j, k;
	CPlotWidget	z;

	/* if there are no arrivals, there is nothing to do */
	if (cp->v->arrival_entries.size() < 1)
	{
	    return(False);
	}

	if(event->type == ButtonPress)
	{
	    if(cp->arrival_i >= 0)
	    {
		i = cp->arrival_i;
		j = cp->arrival_j;
		if(cp->retime_arrival) {
		    return(cp->entry[i]->arrlab[j].a_entry->selected);
		}
		if(!cp->entry[i]->arrlab[j].a_entry->selected &&
				cp->single_arr_select)
		{
		    /* deselect all arrivals with callbacks */
		    for(k = 0; k < cp->v->arrival_entries.size(); k++) {
			if(cp->v->arrival_entries[k]->selected) {
			    DoSelectArrival(w, cp->v->arrival_entries[k], True);
			}
		    }
		}
		if((z = (CPlotWidget)w->axes.mag_to) != NULL)
		{
		    z->c_plot.last_arr_select = False;
		}
		if((z = (CPlotWidget)w->axes.mag_from) != NULL)
		{
		    z->c_plot.last_arr_select = False;
		}
		cp->last_arr_select = True;
		DoSelectArrival(w, cp->entry[i]->arrlab[j].a_entry, True);
		return(True);
	    }
	}
	else
	{
	    MoveArrival(w, event);
	}
	return(False);
}

static void
DoSelectArrival(CPlotWidget w, ArrivalEntry *a_entry, Boolean do_callback)
{
	CPlotPart	*cp = &w->c_plot;
	int		i, j;
	Boolean		selected;
	CPlotWidget	z;

	selected = a_entry->selected ? False : True;
	for(i = 0; i < cp->num_entries; i++)
	{
	    for(j = 0; j < cp->entry[i]->n_arrlab; j++)
		if(cp->entry[i]->arrlab[j].a_entry == a_entry)
	    {
		DrawOneArrival(w, cp->entry[i], j, a_entry->selected, False);
		DrawOneArrival(w, cp->entry[i], j, selected, False);
	    }
	}
	if((z = (CPlotWidget)w->axes.mag_to) != NULL)
	{
	    for(i = 0; i < z->c_plot.num_entries; i++)
	    {
		DataEntry *ze = z->c_plot.entry[i];
		for(j = 0; j < ze->n_arrlab; j++)
		   if(ze->arrlab[j].a_entry == a_entry)
		{
		    DrawOneArrival(z, ze, j, a_entry->selected, False);
		    DrawOneArrival(z, ze, j, selected, False);
		}
	    }
	}
	if((z = (CPlotWidget)w->axes.mag_from) != NULL)
	{
	    for(i = 0; i < z->c_plot.num_entries; i++)
	    {
		DataEntry *ze = z->c_plot.entry[i];
		for(j = 0; j < ze->n_arrlab; j++)
		   if(ze->arrlab[j].a_entry == a_entry)
		{
		    DrawOneArrival(z, ze, j, a_entry->selected, False);
		    DrawOneArrival(z, ze, j, selected, False);
		}
	    }
	}

	a_entry->selected = selected;
	if(selected) {
	    if( !cp->v->selected_tables.contains(a_entry->a)) {
		cp->v->selected_tables.push_back(a_entry->a);
	    }
	}
	else {
	    cp->v->selected_tables.remove(a_entry->a);
	}

	if(do_callback)
	{
	    for(z = w; z->axes.mag_from != NULL;
			z = (CPlotWidget)z->axes.mag_from);
	    XtCallCallbacks((Widget)z, XtNselectArrivalCallback, a_entry->a);
	}
}

bool CPlotClass::selectTable(CssTableClass *o, bool select)
{
    CPlotPart *cp = &cw->c_plot;

    // test if the table is a CssArrivalClass
    if(cp->v->arrivals.contains((CssArrivalClass *)o)) {
	return selectArrival((CssArrivalClass *)o, select);
    }
    else if(cp->v->origins.contains((CssOriginClass *)o) ||
	cp->v->origerrs.contains((CssOrigerrClass *)o) ||
	cp->v->assocs.contains((CssAssocClass *)o) ||
	cp->v->stassocs.contains((CssStassocClass *)o) ||
	cp->v->hydro_features.contains((CssHydroFeaturesClass *)o) ||
	cp->v->infra_features.contains((CssInfraFeaturesClass *)o) ||
	cp->v->stamags.contains((CssStamagClass *)o) ||
	cp->v->netmags.contains((CssNetmagClass *)o) ||
	cp->v->amplitudes.contains((CssAmplitudeClass *)o) ||
	cp->v->ampdescripts.contains((CssAmpdescriptClass *)o) ||
	cp->v->parrivals.contains((CssParrivalClass *)o) ||
	cp->v->filters.contains((CssFilterClass *)o) ||
	cp->v->wftags.contains((CssWftagClass *)o) )
    {
	if(select) {
	    if( !cp->v->selected_tables.contains(o) ) {
		cp->v->selected_tables.push_back(o);
	    }
	}
	else {
	    cp->v->selected_tables.remove(o);
	}
	return true;
    }
    return false; // table Not found
}

bool CPlotClass::isSelected(CssTableClass *table)
{
    if( !table ) return false;
    return cw->c_plot.v->selected_tables.contains(table);
}

static void
MoveArrival(CPlotWidget w, XEvent *event)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, k, cursor_x, cursor_y, delx, dely;
	int		entry_x0;
	double		x;
	DataEntry	*entry;
	ArrivalEntry	*a_entry;
	CPlotWidget	z;
	static CssArrivalClass *a;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	for(z = w; z->axes.mag_from != NULL; z = (CPlotWidget)z->axes.mag_from);

	if(event->type == ButtonPress)
	{
	    if(cp->arrival_i >= 0 && cp->arrival_j >= 0)
	    {
		cp->arrival_moving = True;
		i = cp->arrival_i;
		j = cp->arrival_j;
		entry = cp->entry[i];
		SetClipRects(w, entry);
		DrawArrivalRec(w, ax->mag_invertGC);
		destroyInfoPopup(w);
		DrawOneArrival(w, entry, j,
				entry->arrlab[j].a_entry->selected, False);
		DrawOneArrival(w, entry, j,
				entry->arrlab[j].a_entry->selected, True);
	    }
	    else {
		cp->arrival_moving = False;
	    }
	}
	else if(event->type == MotionNotify && cp->arrival_moving
		&& cp->arrival_i >= 0 && cp->arrival_j >= 0)
	{
	    i = cp->arrival_i;
	    j = cp->arrival_j;
	    entry = cp->entry[i];
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);

	    DrawOneArrival(w, entry, j,
			entry->arrlab[j].a_entry->selected, True);

	    if(ax->x_axis != LEFT) {
		delx = cursor_x - ax->last_cursor_x;
		entry->arrlab[j].x += delx;
	    }
	    else {
		dely = cursor_y - ax->last_cursor_y;
		entry->arrlab[j].y += dely;
	    }
	    if(ax->x_axis != LEFT) {
		x = scale_x(&ax->d, (int)(entry_x0 + entry->arrlab[j].x));
	    }
	    else {
		x = scale_y(&ax->d, (int)(entry_x0 + entry->arrlab[j].y));
	    }
	    entry->arrlab[j].a_entry->a->time =
				entry->tbeg + x - entry->w->scaled_x0;

	    FindOneArrival(w, cp->entry[i], j);
	    DrawOneArrival(w, entry, j,
			entry->arrlab[j].a_entry->selected, True);

	    a = entry->arrlab[j].a_entry->a;

	    XtCallCallbacks((Widget)z, XtNretimeDragCallback, a);
	}
	else if(event->type == ButtonRelease && cp->arrival_moving
		&& cp->arrival_i >= 0 && cp->arrival_j >= 0)
	{
	    cp->arrival_moving = False;
	    i = cp->arrival_i;
	    j = cp->arrival_j;
	    a_entry = cp->entry[i]->arrlab[j].a_entry;

	    for(i = 0; i < cp->num_entries; i++)
	    {
		entry = cp->entry[i];

		if(i == cp->arrival_i) {
		    DrawOneArrival(w, entry, j, a_entry->selected, True);
		    FindOneArrival(w, entry, j);
		    DrawOneArrival(w, entry, j, a_entry->selected, False);
		}
		else
		{
		    for(k = 0; k < entry->n_arrlab; k++) {
			if(entry->arrlab[k].a_entry == a_entry) break;
		    }
		    if(k < entry->n_arrlab)
		    {
			DrawOneArrival(w, entry, k, a_entry->selected, False);
			FindOneArrival(w, entry, k);
			DrawOneArrival(w, entry, k, a_entry->selected, False);
		    }
		}
	    }
	    if((z = (CPlotWidget)ax->mag_to) == NULL) {
		z = (CPlotWidget)ax->mag_from;
	    }
	    if(z != NULL)
	    {
		for(i = 0; i < z->c_plot.num_entries; i++)
		{
		    DataEntry  *ze = z->c_plot.entry[i];
		    for(j = 0; j < ze->n_arrlab; j++)
		    {
			if(ze->arrlab[j].a_entry == a_entry) break;
		    }
		    if(j < ze->n_arrlab)
		    {
			DrawOneArrival(z, ze, j, a_entry->selected, False);
			FindOneArrival(z, ze, j);
			DrawOneArrival(z, ze, j, a_entry->selected, False);
		    }
		}
	    }
	    cp->arrival_i = -1;
	    cp->arrival_j = -1;

	    for(z = w; z->axes.mag_from != NULL;
				z = (CPlotWidget)z->axes.mag_from);
	    XtCallCallbacks((Widget)z, XtNretimeCallback, a);
	}
	else if(cp->arrival_i < 0 || cp->arrival_j < 0) {
	    cp->arrival_moving = False;
	}
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;
}

/** 
 *  Change the time of a CssArrivalClass in a CPlotWidget.
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] a A CssArrivalClass contained in this CPlotWidget.
 *  @param[in] time the new time of the CssArrivalClass.
 *  @param[in] do_callback if True, do XtNretimeDragCallback.
 */
void CPlotClass::moveArrival(CssArrivalClass *a, double time, bool do_callback)
{
    CPlotPart *cp = &cw->c_plot;
    int i, j;
    ArrivalEntry *a_entry=NULL;
    DataEntry *entry, *ze;
    CPlotWidget z;

    for(i = 0; i < cp->v->arrival_entries.size(); i++) {
	if(cp->v->arrival_entries[i]->a == a) {
	    a_entry = cp->v->arrival_entries[i];
	    break;
	}
    }
    if( !a_entry ) return;

    a->time = time;

    if((z = (CPlotWidget)cw->axes.mag_to) == NULL) {
	z = (CPlotWidget)cw->axes.mag_from;
    }
    for(i = 0; i < cp->num_entries; i++)
    {
	entry = cp->entry[i];
	ze = (z != NULL) ? z->c_plot.entry[i] : NULL;
			
	for(j = 0; j < entry->n_arrlab; j++)
		if(entry->arrlab[j].a_entry == a_entry)
	{
	    Boolean selected = entry->arrlab[j].a_entry->selected;
	    DrawOneArrival(cw, entry, j, selected, False);
	    FindOneArrival(cw, entry, j);
	    DrawOneArrival(cw, entry, j, selected, False);

	    if(ze != NULL) {
		DrawOneArrival(z, ze, j, selected, False);
		FindOneArrival(z, ze, j);
		DrawOneArrival(z, ze, j, selected, False);
	    }
	}
    }
    if(do_callback) {
	XtCallCallbacks((Widget)cw, XtNretimeCallback, a);
    }
    change.arrival = true;
    doDataChangeCallbacks();
    if(do_callback) {
	TableListener::doCallbacks(a, this, a->memberIndex("time"));
    }
}

static int
WhichArrival(CPlotWidget w, int cursor_x, int cursor_y, int *ii, int *jj)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i, j;

	/* returns: ii, jj for the arrival containing the cursor:
	 *  cp->entry[ii]->arrlab[jj].a_entry
	 */
	*ii = *jj = -1;

	if(cp->display_arrivals == CPLOT_ARRIVALS_OFF) return(0);


	/* initialize dmin to a large value before seeking the minimim.
	*/
	for(i = 0; i < cp->num_entries; i++)
		if(cp->entry[i]->on && cp->entry[i]->w->visible
			&& cp->entry[i]->beg)
	{
	    DataEntry *entry = cp->entry[i];
	    int entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    int entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);

	    for(j = 0; j < entry->n_arrlab; j++)
	    {
		ArrivalEntry *a_entry = entry->arrlab[j].a_entry;

		if(a_entry->on && (
		    cp->display_arrivals == CPLOT_ARRIVALS_ALL_CHAN ||
			DataSource::compareChan(entry->w->channel,
				a_entry->a->chan)))
		{
		    int left = entry_x0 +
				(int)(entry->arrlab[j].x-.5*a_entry->width-2);
		    int rite = left + a_entry->width + 4;
		    int top = entry_y0 + (int)entry->arrlab[j].y - 2;
		    int bot = top + a_entry->height + 4;

		    if( cursor_x >= left && cursor_x <= rite && 
			cursor_y >= top  && cursor_y <= bot)
		    {
			*jj = j;
			*ii = i;
			return(1);
		    }
		}
	    }
	}
	return(0);
}

static void
FindArrivals(CPlotWidget w, DataEntry *entry)
{
	CPlotPart	*cp = &w->c_plot;
	int		j;
	ArrivalLabel	*arr;

	for(j = 0; j < entry->n_arrlab; j++) {
	    entry->arrlab[j].done = False;
	}
	for(j = 0; j < entry->n_arrlab; j++)
	{
	    arr = &entry->arrlab[j];
	    if((cp->display_arrivals == CPLOT_ARRIVALS_ONE_CHAN &&
		DataSource::compareChan(entry->w->channel,
			arr->a_entry->a->chan))
			|| cp->display_arrivals == CPLOT_ARRIVALS_ALL_CHAN)
	    {
		FindOneArrival(w, entry, j);
	    }
	}
	cp->find_arrivals = cp->display_arrivals;
}

static void
FindOneArrival(CPlotWidget w, DataEntry *entry, int j)
{
	AxesPart	*ax = &w->axes;
	int		i, l, m, entry_x0;
	int		min_y_top, max_y_bot, y2=0;
	double		t_phase;
	float		y_min;
	RSeg		*s;
	ArrivalLabel	*arrlab;

	if(!entry->on || !entry->w->visible || !entry->beg ||
		entry->r.nsegs <= 0 || entry->r.nshow <= 0)
	{
	    entry->need_find = True;
	    return;
	}
	entry->need_find = False;

	arrlab = &entry->arrlab[j];

	t_phase = entry->w->scaled_x0 + arrlab->a_entry->a->time-entry->tbeg;
	if(ax->x_axis != LEFT)
	{
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    arrlab->x = unscale_x(&ax->d, t_phase) - entry_x0;
	}
	else
	{
	    entry_x0 = unscale_y(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    arrlab->x = unscale_y(&ax->d, t_phase) - entry_x0;
	}

	arrlab->done = True;

	/* check if arrival label will overlap other arrival labels.
	 */
	min_y_top = 9999999;
	max_y_bot = -9999999;

	for(i = 0; i < entry->n_arrlab; i++)
	{
	    ArrivalLabel *a = &entry->arrlab[i];
	    if(a != arrlab && a->done && fabs(arrlab->x - a->x) <
		.5*(arrlab->a_entry->width + a->a_entry->width) + 4)
	    {
		if(min_y_top > a->y) min_y_top = (int)a->y;

		if(max_y_bot < a->y + a->a_entry->height) {
		    max_y_bot = (int)a->y + a->a_entry->height;
		    y2 = a->y2;
		}
	   }
	}
	if(min_y_top != 9999999)
	{
	    arrlab->y  = min_y_top - 4 - arrlab->a_entry->height;
	    arrlab->y1 = max_y_bot + 2;
	    arrlab->y2 = y2;
	}
	else
	{
	    s = entry->r.segs + entry->r.start;
	    l = FindRSeg(s, entry->r.nshow, (double)arrlab->x);
	    /*
	     * find y_min in label-width-wide region at s[k].x1
	     */
	    y_min = s[l].y1;
	    for(m = l; m>=0 && s[l].x1 - s[m].x1 <= arrlab->a_entry->width; m--)
	    {
		if(s[m].y1 < y_min) y_min = s[m].y1;
		if(s[m].y2 < y_min) y_min = s[m].y2;
	    }
	    for(m = l; m < entry->r.nshow && s[m].x1 - s[l].x1 <=
		arrlab->a_entry->width; m++)
	    {
		if(s[m].y1 < y_min) y_min = s[m].y1;
		if(s[m].y2 < y_min) y_min = s[m].y2;
	    }
	    arrlab->y2 = (int)y_min - 2;
	    arrlab->y1 = arrlab->y2 - 10;
	    arrlab->y  = arrlab->y1 - 2 - arrlab->a_entry->height;
	}
}

static void
DrawArrivalsOnEntry(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	int i, j;
	long orid;

	if(cp->display_arrivals == CPLOT_ARRIVALS_OFF || !entry->on ||
		!entry->w->visible)
	{
	    return;
	}
	if(cp->display_assoc_only)
	{
	    if(entry->origin == NULL || cp->v->assocs.size() == 0) return;

	    orid = entry->origin->orid;

	    for(i = 0; i < entry->n_arrlab; i++)
	    {
		long arid = entry->arrlab[i].a_entry->a->arid;
		for(j = 0; j < cp->v->assocs.size(); j++)
		{
		    CssAssocClass *a = cp->v->assocs[j];
		    if(a->orid == orid && a->arid == arid)
		    {
			DrawOneArrival(w, entry, i,
			    entry->arrlab[i].a_entry->selected, False);
			break;
		    }
		}
	    }
	}
	else {
	    for(i = 0; i < entry->n_arrlab; i++) {
		DrawOneArrival(w, entry, i,
				entry->arrlab[i].a_entry->selected, False);
	    }
	}
}

static void
DrawAllArrivals(CPlotWidget w)
{
	CPlotPart *cp = &w->c_plot;
	int i;

	for(i = 0; i < cp->v->arrival_entries.size(); i++)
	{
	    DrawArrival(w, cp->v->arrival_entries[i]);
	}
}

static void
DrawArrival(CPlotWidget w, ArrivalEntry *a_entry)
{
	CPlotPart *cp = &w->c_plot;
	int i, j;

	if(cp->display_arrivals != CPLOT_ARRIVALS_OFF)
	{
	    for(i = 0; i < cp->num_entries; i++)
	    {
		for(j = 0; j < cp->entry[i]->n_arrlab; j++)
		   if(cp->entry[i]->arrlab[j].a_entry == a_entry)
		{
		    DrawOneArrival(w, cp->entry[i], j, a_entry->selected,
				False);
		}
	    }
	}
}

static void
DrawOneArrival(CPlotWidget w, DataEntry *entry, int k, Boolean selected,
			Boolean  moving)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int	i, j, x, x0, x1, x2, y, y1, y2, bot, top, left, rite;
	int	entry_x0, entry_y0;
	long	arid, orid;
	ArrivalLabel	*arr;
	CssArrivalClass	*arrival;
	float		percent;
	static int	npoints = 0;
	static XPoint	*points = NULL;
	unsigned long   valuemask =
		GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle;
        XGCValues       values;

	arr = &entry->arrlab[k];

	if(!XtIsRealized((Widget)w) || !arr->a_entry->on) return;

	if(entry->beg) {
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	}
	else {
	    entry_x0 = unscale_x(&ax->d, entry->w->scaled_x0);
	}
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);

	if(cp->display_assoc_only)
	{
	    if(entry->origin == NULL || cp->v->assocs.size() == 0) return;

	    orid = entry->origin->orid;
	    arid = arr->a_entry->a->arid;

	    for(i = 0; i < cp->v->assocs.size(); i++)
	    {
		CssAssocClass *a = cp->v->assocs[i];
		if(a->orid == orid && a->arid == arid) break;
	    }
	    if(i == cp->v->assocs.size()) return;
	}
	if(arr->a_entry->npoints > npoints) {
	    Free(points);
	    points = (XPoint *)MallocIt(w,arr->a_entry->npoints*sizeof(XPoint));
	    npoints = arr->a_entry->npoints;
	}
	if(!entry->on || !entry->w->visible) {
	    return;
	}

	arrival = arr->a_entry->a;

	if(cp->display_arrivals == CPLOT_ARRIVALS_ONE_CHAN &&
	    !DataSource::compareChan(entry->w->channel, arrival->chan))
	{
	    return;
	}

    	XSetPlaneMask(XtDisplay(w), cp->invertGC,
			arr->a_entry->fg ^ w->core.background_pixel);
    	XSetForeground(XtDisplay(w), cp->invertGC, arr->a_entry->fg);

	if(ax->x_axis != LEFT)
	{
	    CPlotArrivalType *arrtype=NULL;
	    x = (int)(entry_x0 + arr->x);

	    if(arrival->atype >= 0 &&
		arrival->atype < cp->v->arrival_types.size())
	    {
		i = arrival->atype;
		arrtype = cp->v->arrival_types[i];
	    }
	    if(!arrtype) {
		y = (int)(entry_y0 + arr->y);
		y1 = entry_y0 + arr->y1;
		y2 = entry_y0 + arr->y2;
	    }
	    else
	    {
		y = (int)(entry_y0 + arr->y); /* ?? */
		y1 = entry_y0;
		percent = (arrtype->max_display_value - arrival->value)/
		  (arrtype->max_display_value - arrtype->min_display_value);

		if (percent > 1.0) percent = 1.0;
		else if (percent < 0.0) percent = 0.0;

		y2 = y1 - (int) (arrtype->display_min_height_pix + (percent * 
			  	 (arrtype->display_max_height_pix -
                               	  arrtype->display_min_height_pix)));
		y = y2 - arr->a_entry->height - arr->a_entry->ascent;
	    }

	    if(x > ax->clipx1 && x < ax->clipx2 &&
		y + arr->a_entry->height > ax->clipy1 && y < ax->clipy2)
	    {
		if (arrtype)
		{
		    XGetGCValues(XtDisplay(w), cp->invertGC, valuemask,&values);
		    XSetLineAttributes(XtDisplay(w), cp->invertGC,
				arrtype->display_width_pix, values.line_style,
				values.cap_style, values.join_style);
		}

		if(moving)
		{
		    bot = unscale_y(&ax->d, ax->y1[ax->zoom]) - 1;
		    DrawLine(w, cp->invertGC, x,y+arr->a_entry->height+5,x,bot);
		    top = unscale_y(&ax->d, ax->y2[ax->zoom]) + 1;
		    DrawLine(w, cp->invertGC, x, top, x, y-5);
		}
		else {
		    DrawLine(w, cp->invertGC, x, y1, x, y2);
		}
			
		if(arrtype)
		{
		    XSetLineAttributes(XtDisplay(w), cp->invertGC,
				values.line_width, values.line_style,
				values.cap_style, values.join_style);
		}

		x0 = (int)(x-.5*arr->a_entry->width);
		for(j = 0; j < arr->a_entry->npoints; j++)
		{
		    points[j].x = x0 + arr->a_entry->points[j].x;
		    points[j].y = y + arr->a_entry->points[j].y;
		}
		if(!ax->use_pixmap || ax->use_screen)
		{
		    if(selected) {
			XFillRectangle(XtDisplay(w),XtWindow(w), cp->invertGC,
			    x0-2, y-2, arr->a_entry->width+4,
			    arr->a_entry->height+4);
		    }
		    XDrawPoints(XtDisplay(w), XtWindow(w), cp->invertGC,
			    points, arr->a_entry->npoints, CoordModeOrigin);
		}
		if(ax->use_pixmap)
		{
		    if(selected) {
			XFillRectangle(XtDisplay(w), ax->pixmap, cp->invertGC,
			    x0-2, y-2, arr->a_entry->width+4,
			    arr->a_entry->height+4);
		    }
		    XDrawPoints(XtDisplay(w), ax->pixmap, cp->invertGC,
				points, arr->a_entry->npoints, CoordModeOrigin);
		}
	    }
	}
	else
	{
	    y = (int)(entry_x0 + arr->x);
	    x = (int)(entry_y0 + arr->y);
	    x1 = entry_y0 + arr->y1;
	    x2 = entry_y0 + arr->y2;
	    if(x > ax->clipx1 && x + arr->a_entry->width < ax->clipx2 &&
		y > ax->clipy1 && y < ax->clipy2)
	    {
		if(moving) {
		    left = unscale_x(&ax->d, ax->x1[ax->zoom]) - 1;
		    DrawLine(w, cp->invertGC, left, y, x-5,y);
		    rite = unscale_x(&ax->d, ax->x2[ax->zoom]) + 1;
		    DrawLine(w, cp->invertGC,x+arr->a_entry->width+5,y,rite,y);
		}
		else {
		    DrawLine(w, cp->invertGC, x1, y, x2, y);
		}
		x0 = (int)(y-.5*arr->a_entry->height);

		for(j = 0; j < arr->a_entry->npoints; j++) {
		    points[j].x = y + arr->a_entry->points[j].x;
		    points[j].y = x0 + arr->a_entry->points[j].y;
		}
		if(!ax->use_pixmap || ax->use_screen)
		{
		    if(selected) {
			XFillRectangle(XtDisplay(w),XtWindow(w), cp->invertGC,
			    x0-2, y-2, arr->a_entry->width+4,
			    arr->a_entry->height+4);
		    }
		    XDrawPoints(XtDisplay(w), XtWindow(w), cp->invertGC, points,
				arr->a_entry->npoints, CoordModeOrigin);
		}
		if(ax->use_pixmap)
		{
		    if(selected) {
			XFillRectangle(XtDisplay(w), ax->pixmap, cp->invertGC,
			    x0-2, y-2, arr->a_entry->width+4,
			    arr->a_entry->height+4);
		    }
		    XDrawPoints(XtDisplay(w), ax->pixmap, cp->invertGC, points,
				arr->a_entry->npoints, CoordModeOrigin);
		}
	    }
	}
}

static void
FindPredArrivals(CPlotWidget w, DataEntry *entry)
{
	int i;

	for(i = 0; i < entry->npred_arr; i++) {
	    entry->pred_arr[i].done = False;
	}
	for(i = 0; i < entry->npred_arr; i++) {
	    FindOnePredArr(w, entry, &entry->pred_arr[i]);
	}
}

static void
FindOnePredArr(CPlotWidget w, DataEntry *entry, CPlotPredArr *pred_arr)
{
	AxesPart	*ax = &w->axes;
	int		i, l, m, entry_x0;
	int		min_y_top, max_y_bot, y2=0;
	double		t_phase;
	float		y_max;
	RSeg		*s;
	CPlotPredArr	*a;

	if(!entry->on || !entry->w->visible ||
		entry->r.nsegs <= 0 || entry->r.nshow <= 0)
	{
	    entry->need_pred_find = True;
	    return;
	}
	entry->need_pred_find = False;
	pred_arr->done = True;

	t_phase = entry->w->scaled_x0 + pred_arr->time - entry->tbeg;
	if(ax->x_axis != LEFT)
	{
	    entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    pred_arr->x = unscale_x(&ax->d, t_phase) - entry_x0;
	}
	else
	{
	    entry_x0 = unscale_y(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	    pred_arr->x = unscale_y(&ax->d, t_phase) - entry_x0;
	}

	/* check if arrival label will overlap other arrival labels.
	 */
	min_y_top = 9999999;
	max_y_bot = -9999999;
	for(i = 0; i < entry->npred_arr; i++) 
	{
	    a = &entry->pred_arr[i];
	    if(a != pred_arr && a->done && fabs(pred_arr->x - a->x)
			< .5*(pred_arr->width + a->width)+4)
	    {
		if(min_y_top > a->y) {
		    min_y_top = (int)a->y;
		    y2 = a->y2;
		}
		if(max_y_bot < a->y + a->height) {
		    max_y_bot = (int)(a->y + a->height);
		}
	    }
	}
	if(min_y_top != 9999999)
	{
	    pred_arr->y = max_y_bot + 2;
	    pred_arr->y1 = min_y_top - 2;
	    pred_arr->y2 = y2;
	}
	else
	{
	    s = entry->r.segs + entry->r.start;
	    l = FindRSeg(s, entry->r.nshow, (double)pred_arr->x);
	    /*
	     * find y_max in label-width-wide region at s[k].x1
	     */
	    y_max = s[l].y1;
	    for(m = l; m >= 0 && s[l].x1 - s[m].x1 <= pred_arr->width; m--)
	    {
		if(s[m].y1 > y_max) y_max = s[m].y1;
		if(s[m].y2 > y_max) y_max = s[m].y2;
	    }
	    for(m = l; m < entry->r.nshow &&
			s[m].x1 - s[l].x1 <= pred_arr->width; m++)
	    {
		if(s[m].y1 > y_max) y_max = s[m].y1;
		if(s[m].y2 > y_max) y_max = s[m].y2;
	    }
	    pred_arr->y2 = (int)(y_max + 2);
	    pred_arr->y1 = pred_arr->y2 + 10;
	    pred_arr->y = pred_arr->y1 + 2;
	}
}

/** 
 *  
 */
void
_CPlotDrawPredArrivals(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	int i;

	if(!cp->display_predicted_arrivals || !entry->on || !entry->w->visible)
	{
	    return;
	}
	for(i = 0; i < entry->npred_arr; i++) {
	    DrawOnePredArr(w, entry, &entry->pred_arr[i]);
	}
}

static void
DrawOnePredArr(CPlotWidget w, DataEntry	*entry, CPlotPredArr *pred_arr)
{
	CPlotPart	*cp = &w->c_plot;
	AxesPart	*ax = &w->axes;
	int		i, x, x0, x1, x2, y, y1, y2, entry_x0, entry_y0;
	static int	npoints = 0;	
	static XPoint	*points = NULL;

	if( !entry->beg ) return;

	if(pred_arr->npoints > npoints)
	{
	    Free(points);
	    points = (XPoint *)MallocIt(w, pred_arr->npoints*sizeof(XPoint));
	    npoints = pred_arr->npoints;
	}

	entry_x0 = unscale_x(&ax->d,
		    entry->w->scaled_x0 + entry->beg->time() - entry->tbeg);
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);

	XSetPlaneMask(XtDisplay(w), cp->invertGC,
			    w->axes.fg ^ w->core.background_pixel);
	XSetForeground(XtDisplay(w), cp->invertGC, w->axes.fg);

	if(ax->x_axis != LEFT)
	{
	    x = (int)(entry_x0 + pred_arr->x);
	    y = (int)(entry_y0 + pred_arr->y);
	    y1 = entry_y0 + pred_arr->y1-1;
	    y2 = entry_y0 + pred_arr->y2;
	    if(x > ax->clipx1 && x < ax->clipx2 &&
		y + pred_arr->height > ax->clipy1 && y < ax->clipy2)
	    {
		DrawLine(w, cp->invertGC, x, y1, x, y2);
	    }
	    else {
		return;
	    }
	    x0 = (int)(x-.5*pred_arr->width);
	    for(i = 0; i < pred_arr->npoints; i++)
	    {
		points[i].x = x0 + pred_arr->points[i].x;
		points[i].y = y + pred_arr->points[i].y;
	    }
	    if(!ax->use_pixmap || ax->use_screen) {
		XSetPlaneMask(XtDisplay(w), cp->invertGC, pred_arr->fg ^
			w->core.background_pixel);
		XSetForeground(XtDisplay(w), cp->invertGC, pred_arr->fg);
		XFillRectangle(XtDisplay(w), XtWindow(w), cp->invertGC,
                            x0-1, y-1, pred_arr->width+2, pred_arr->height+2);
		XSetPlaneMask(XtDisplay(w), cp->invertGC, pred_arr->fg ^
			w->axes.fg);
		XSetForeground(XtDisplay(w), cp->invertGC, pred_arr->fg);
		XDrawPoints(XtDisplay(w), XtWindow(w), cp->invertGC, points,
			pred_arr->npoints, CoordModeOrigin);
	    }
	    if(ax->use_pixmap) {
		XSetPlaneMask(XtDisplay(w), cp->invertGC, pred_arr->fg ^
			w->core.background_pixel);
		XSetForeground(XtDisplay(w), cp->invertGC, pred_arr->fg);
		XFillRectangle(XtDisplay(w), ax->pixmap, cp->invertGC,
			x0-1, y-1, pred_arr->width+2, pred_arr->height+2);
		XSetPlaneMask(XtDisplay(w), cp->invertGC, pred_arr->fg ^
			w->axes.fg);
		XSetForeground(XtDisplay(w), cp->invertGC, pred_arr->fg);
		XDrawPoints(XtDisplay(w), ax->pixmap, cp->invertGC, points,
			pred_arr->npoints, CoordModeOrigin);
	    }
	}
	else
	{
		y = (int)(entry_x0 + pred_arr->x);
		x = (int)(entry_y0 + pred_arr->y);
		x1 = entry_y0 + pred_arr->y1;
		x2 = entry_y0 + pred_arr->y2;
		if(x > ax->clipx1 && x + pred_arr->width < ax->clipx2
			&& y > ax->clipy1 && y < ax->clipy2)
		{
			DrawLine(w, cp->invertGC, x1, y, x2, y);
		}
		else {
			return;
		}
	}
}

/** 
 *  Create a CPlotWidget.
 *  
 */
Widget
CPlotCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
	return (XtCreateWidget(name, cPlotWidgetClass, parent, 
		arglist, argcount));
}

/** 
 *  
 */
void
_CPlotHorizontalScroll(CPlotWidget w, double shift, Boolean jump)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int m;
	DataEntry *entry;

	cp->draw_label_call = 0;

	if(!cp->scroll_data)
	{
	    for(m = 0; m < cp->num_entries; m++)
	    {
		entry = cp->entry[m];
		if(ax->x_axis != LEFT) {
		    entry->w->scaled_x0 += shift;
		}
		else {
		    entry->w->scaled_y0 += shift;
		}
	    }
	}
	if(jump)
	{
	    _CPlotRedraw(w);
	    _CPlotRedisplay(w);
	    return;
	}
	for(m = 0; m < cp->num_curves; m++)
	{
	    if(cp->curve[m]->on) {
		_CPlotRedrawCurve(w, cp->curve[m]);
		_CPlotRedisplayCurve(w, cp->curve[m]);
	    }
	}
        /*
         * Check if each entry is visible.
         */
        Visible(w);

	if(ax->x_axis != LEFT)
	{
	    for(m = 0; m < cp->num_entries; m++)
	    {
		entry = cp->entry[m];
		_CPlotRedrawEntry(w, entry);
		_CPlotRedisplayEntry(w, entry);
	    }
	}
	else
	{
	    for(m = 0; m < cp->num_entries; m++) {
		_CPlotRedrawEntry(w, cp->entry[m]);
		_CPlotRedisplayEntry(w, cp->entry[m]);
	    }
	}
	cp->point_x = -1;
}

/** 
 *  
 */
void
_CPlotVerticalScroll(CPlotWidget w, double shift, Boolean jump)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
	int  m;
	DataEntry *entry;

	cp->draw_label_call = 0;

	if(!cp->scroll_data)
	{
	    for(m = 0; m < cp->num_entries; m++)
	    {
		entry = cp->entry[m];
		if(ax->x_axis != LEFT) {
		    entry->w->scaled_y0 += shift;
		}
		else {
		    entry->w->scaled_x0 += shift;
		}
	    }
	}
	if(jump)
	{
	    _CPlotRedraw(w);
	    _CPlotRedisplay(w);
	    return;
	}
	for(m = 0; m < cp->num_curves; m++)
	{
	    if(cp->curve[m]->on) {
		_CPlotRedrawCurve(w, cp->curve[m]);
		_CPlotRedisplayCurve(w, cp->curve[m]);
	    }
	}
	/*
	 * Check if each entry is visible.
	 */
	Visible(w);

	if(ax->x_axis != LEFT)
	{
	    for(m = 0; m < cp->num_entries; m++) {
		_CPlotRedrawEntry(w, cp->entry[m]);
		_CPlotRedisplayEntry(w, cp->entry[m]);
	    }
	}
	else
	{
	    for(m = 0; m < cp->num_entries; m++)
	    {
		entry = cp->entry[m];
		_CPlotRedrawEntry(w, entry);
	    }
	}
	cp->point_x = -1;
}

static void *
MallocIt(CPlotWidget w, int nbytes)
{
	CPlotWidget z;
	void *ptr;

	if(nbytes <= 0) {
	    return (void *)NULL;
	}
	else if((ptr = (void *)malloc(nbytes)) == (void *)0)
	{
	    for(z = w; z->axes.mag_from != NULL;
				z = (CPlotWidget)z->axes.mag_from);
	    _AxesWarn((AxesWidget)z, "Memory limitation:\nCannot allocate \
			an additional %d bytes.", nbytes);
	    return (void *)NULL;
	}
	return ptr;
}

/** 
 *  Change a waveform tag (label).
 *  @param[in] w A CPlotWidget pointer.
 *  @param[in] cd A Waveform structure pointer associated with a waveform
 *		in this CPlotWidget.
 *  @param[in] tag The character string for the new tag.
 *  @param[in] redraw If true, update the data display immediately, otherwise
 *		do not update now.
 */
void CPlotClass::changeTag(Waveform *cd, const string &tag, bool redraw)
{
    CPlotPart *cp = &cw->c_plot;
    AxesPart *ax = &cw->axes;
    int i;
    DataEntry *entry, *ze;
    CPlotWidget z;

    if((int)tag.length() <= 0) return;

    for(i = 0; i < cp->num_entries; i++) {
	if(cd == cp->entry[i]->w) break;
    }
    if(i == cp->num_entries) return;

    entry = cp->entry[i];
    if(tag.compare(entry->tag))
    {
	cp->max_tag_width = maxTagWidth(cw);
	if(redraw)
	{
	    _CPlotRedisplayEntry(cw, entry);
	    stringcpy(entry->tag, tag.c_str(), sizeof(entry->tag));
	    DrawTag(cw, entry);
	    _CPlotRedrawEntry(cw, entry);
	    if(!ax->redisplay_pending) {
		_CPlotRedisplayEntry(cw, entry);
	    }
	}
	else {
	    stringcpy(entry->tag, tag.c_str(), sizeof(entry->tag));
	    DrawTag(cw, entry);
	}
	if((z = (CPlotWidget)ax->mag_to) != NULL)
	{
	    ze = z->c_plot.entry[i];
	    if(redraw)
	    {
		_CPlotRedisplayEntry(z, ze);
		stringcpy(ze->tag, tag.c_str(), sizeof(ze->tag));
		DrawTag(z, ze);
		if(ze->on) {
		    _CPlotRedrawEntry(z, ze);
		}
		_CPlotRedisplayEntry(z, ze);
	    }
	    else {
		stringcpy(ze->tag, tag.c_str(), sizeof(ze->tag));
		DrawTag(z, ze);
	    }
	}
    }
}

static void
DrawTag(CPlotWidget w, DataEntry *entry)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
        int	i, j, ascent, descent, direction, last_descent, lab_x, lab_y, n;
        XCharStruct	overall;
	Pixmap		pixmap;
	XImage		*image;

	Free(entry->tag_points);
	entry->n_tag_points = 0;
	entry->tag_width = 0;
	entry->tag_height = 0;

	if(entry->tag[0] == '\0' || !XtIsRealized((Widget)w) ) return;

	CPlotGetTagDimensions(w, entry->tag, &entry->tag_width,
				&entry->tag_height);

	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
	pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w), entry->tag_width+1,
			entry->tag_height+1,
			DefaultDepth(XtDisplay(w),DefaultScreen(XtDisplay(w))));

	XSetFunction(XtDisplay(w), ax->axesGC, GXclear);
	XFillRectangle(XtDisplay(w), pixmap, ax->axesGC, 0, 0, entry->tag_width,			entry->tag_height);
	XSetFunction(XtDisplay(w), ax->axesGC, GXcopy);
	XSetForeground(XtDisplay(w), ax->axesGC, 1);

	lab_x = 2;
	lab_y = 0;
	last_descent = 0;
	i = j = 0;
	XSetFont(XtDisplay(w), ax->axesGC, cp->tag_font->fid);
	while(entry->tag[j] != '\0')
	{
	    for(j = i; entry->tag[j] != '\0' && entry->tag[j] != '\n'; j++);
	    XTextExtents(cp->tag_font, entry->tag+i, j-i,
			&direction, &ascent, &descent, &overall);
	    lab_y += last_descent + 2 + overall.ascent;
	    last_descent = overall.descent;
	    XDrawString(XtDisplay(w), pixmap, ax->axesGC,
			lab_x, lab_y, entry->tag+i, j-i);
	    i = j + 1;
	}
	XSetFont(XtDisplay(w), ax->axesGC, ax->font->fid);
	XSetFunction(XtDisplay(w), ax->axesGC, GXcopy);
	XSetForeground(XtDisplay(w), ax->axesGC, ax->fg);

	image = XGetImage(XtDisplay(w), pixmap, 0, 0, entry->tag_width,
		entry->tag_height, AllPlanes, XYPixmap);

	if( entry->tag_width*entry->tag_height > 0 &&
		!(entry->tag_points = (XPoint *)MallocIt(w,
		entry->tag_width*entry->tag_height*sizeof(XPoint))) ) return;

	for(i = n = 0; i < entry->tag_width; i++)
	{
	    for(j = 0; j < entry->tag_height; j++)
	    {
		if(XGetPixel(image, i, j)) {
		    entry->tag_points[n].x = i;
		    entry->tag_points[n].y = j;
		    n++;
		}
	    }
	}
	entry->n_tag_points = n;

	XFreePixmap(XtDisplay(w), pixmap);
	XDestroyImage(image);
}

/** 
 *  Get the pixel width and height that a waveform tag would if the indicated
 *  tag is added.
 */
void
CPlotGetTagDimensions(CPlotWidget w, const string &tag, int *width, int *height)
{
    CPlotPart *cp = &w->c_plot;
    int i, j, ascent, descent, direction;
    XCharStruct overall;

    *width = 0;
    *height = 2;
    i = j = 0;
    for(j = 0; j < (int)tag.length(); j++)
    {
	for(j = i; tag[j] != '\0' && tag[j] != '\n'; j++);

	XTextExtents(cp->tag_font, tag.c_str()+i, j-i, &direction, &ascent,
			&descent, &overall);
	if(*width < overall.width + 4) *width = overall.width + 4;
	*height += overall.ascent + overall.descent + 2;
	i = j + 1;
    }
}

static void
DrawScale(CPlotWidget w, DataEntry *entry, double yscale)
{
	CPlotPart *cp = &w->c_plot;
	AxesPart *ax = &w->axes;
        int i, j, ascent, descent, direction, min, max;
	int x1, x2, x0, y0, iy1, iy2, ydif, height, ndigit, entry_y0;
	double y, y1, y2;
        XCharStruct overall;
	DrawStruct d;
	SegArray *s;
	AxesParm a;
	AxesParm axes_parm_init = AXES_PARM_INIT;

	if(!cp->display_amplitude_scale
	    || (entry->sel.nsegs <= 0 && entry->unsel.nsegs <= 0)
	    || entry->ymin == entry->ymax || entry->yscale == 0.)
	{
	    return;
	}

	x1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
	x2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
	if(x1 > x2)
	{
	    x0 = x1;
	    x1 = x2;
	    x2 = x0;
	}
	if(entry->unsel.nsegs > 0) {
	    min = entry->unsel.segs[0].x1;
	    if(entry->sel.nsegs > 0 && entry->sel.segs[0].x1 < min) {
	        min = entry->sel.segs[0].x1;
	    }
	}
	else {
	    min = entry->sel.segs[0].x1;
	}
	if(entry->unsel.nsegs > 0) {
	    max = entry->unsel.segs[entry->unsel.nsegs-1].x1;
	    if(entry->sel.nsegs > 0 &&
		entry->sel.segs[entry->sel.nsegs-1].x1 > max) {
	        max = entry->sel.segs[entry->sel.nsegs-1].x1;
	    }
	}
	else {
	    max = entry->sel.segs[entry->sel.nsegs-1].x1;
	}

	if(min >= x2 || max <= x1) return;
	    
	iy1 = ax->d.iy1;
	iy2 = ax->d.iy2;
	if(iy1 < iy2)
	{
	    y0  = iy1;
	    iy1 = iy2;
	    iy2 = y0;
	}
	s = &entry->unsel;
	for(i = 0; i < s->nsegs; i++)
	{
	    if(s->segs[i].x1 >= x1 && s->segs[i].x1 <= x2) {
		if(iy1 > s->segs[i].y1) iy1 = s->segs[i].y1;
		if(iy2 < s->segs[i].y1) iy2 = s->segs[i].y1;
	    }
	    if(s->segs[i].x2 >= x1 && s->segs[i].x2 <= x2) {
		if(iy1 > s->segs[i].y2) iy1 = s->segs[i].y2;
		if(iy2 < s->segs[i].y2) iy2 = s->segs[i].y2;
	    }
	    if(s->segs[i].x1 < x1 && s->segs[i].x2 > x2) {
		if(iy1 > s->segs[i].y1) iy1 = s->segs[i].y1;
		if(iy1 > s->segs[i].y2) iy1 = s->segs[i].y2;
		if(iy2 < s->segs[i].y1) iy2 = s->segs[i].y1;
		if(iy2 < s->segs[i].y2) iy2 = s->segs[i].y2;
	    }
	}
	s = &entry->sel;
	for(i = 0; i < s->nsegs; i++)
	{
	    if(s->segs[i].x1 >= x1 && s->segs[i].x1 <= x2) {
		if(iy1 > s->segs[i].y1) iy1 = s->segs[i].y1;
		if(iy2 < s->segs[i].y1) iy2 = s->segs[i].y1;
	    }
	    if(s->segs[i].x2 >= x1 && s->segs[i].x2 <= x2) {
		if(iy1 > s->segs[i].y2) iy1 = s->segs[i].y2;
		if(iy2 < s->segs[i].y2) iy2 = s->segs[i].y2;
	    }
	    if(s->segs[i].x1 < x1 && s->segs[i].x2 > x2) {
		if(iy1 > s->segs[i].y1) iy1 = s->segs[i].y1;
		if(iy1 > s->segs[i].y2) iy1 = s->segs[i].y2;
		if(iy2 < s->segs[i].y1) iy2 = s->segs[i].y1;
		if(iy2 < s->segs[i].y2) iy2 = s->segs[i].y2;
	    }
	}
/*
	ydif = abs(iy2 - iy1);
*/
	entry_y0 = unscale_y(&ax->d, entry->w->scaled_y0);
	iy2 -= entry_y0;
	iy1 -= entry_y0;
	ydif = 2*(abs(iy2) > abs(iy1) ? abs(iy2) : abs(iy1));
ydif = (int)(.8*2*(abs(iy2) > abs(iy1) ? abs(iy2) : abs(iy1)));
	if(entry->w->scaled_y0 < ax->y1[ax->zoom] - ydif/ax->d.scaley ||
	   entry->w->scaled_y0 > ax->y2[ax->zoom] + ydif/ax->d.scaley)
	{
	    return;
	}
	if(ydif > .8*abs(ax->d.iy2 - ax->d.iy1))
	{
	    ydif = (int)(.8*abs(ax->d.iy2 - ax->d.iy1));
	}

	memcpy(&a, &axes_parm_init, sizeof(AxesParm));
	a.fontMethod = ax->a.fontMethod;
	a.font = cp->amp_font;
	if(a.fontMethod != NULL)  /* X fonts */
	{
	    XTextExtents((XFontStruct *)a.font, "0123456789", 10, &direction,
				&ascent, &descent, &overall);
	    height = overall.ascent + overall.descent;
	}
	else {
	    height = a.label_font_height;
	}

	if(ydif < entry->tag_height + 3*height) {
	    ydif = entry->tag_height + 3*height;
	}
	if(ydif < 3*height) {
	    ydif = 3*height;
	}
	ydif /= 2;

	SetDrawArea(&d, 0, ydif, 100, -ydif);

	y1 = entry->mean - ydif/yscale;
	y2 = entry->mean + ydif/yscale;

	SetScale(&d, 0., y1, 1., y2);

	a.min_ylab = 3;
	i = (2*ydif -  entry->tag_height)/height;
	if(i < 3) i = 3;
	if(i < a.max_ylab) a.max_ylab = i;
	if(a.max_ylab > 7) a.min_ylab = 5;

	nicex(y1, y2, a.min_ylab, a.max_ylab, &a.nylab, a.y_lab,
			&ndigit, &a.nydeci);

	a.top	 = -ydif;
	a.bottom =  ydif;
	a.ytic = .75*height*1./100.;

	gdrawYAxis(&a, &d, 2, 0., y1, y2, a.ytic, True, 0, 1, 0.);

	x0 = entry->lab_x + entry->tag_width;

	XSetFont(XtDisplay(w), ax->invert_clipGC, cp->amp_font->fid);
	entry->amp_width = 0;
	j = 0;
	if(a.nylab > 1)
	{
	    for(i = 0; i < a.nylab; i++)
	    {
		iy1 = a.y_ylab[i] + a.ylab_height[i] - a.ylab_ascent[i];
		iy2 = a.y_ylab[i] - a.ylab_ascent[i];
		if(iy1 < -.5*entry->tag_height || iy2 > .5*entry->tag_height)
		{
			j++;
		}
	    }
	    if(j > 1)
	    {
		for(i = j = 0; i < a.nylab; i++)
		{
		    iy1 = a.y_ylab[i] + a.ylab_height[i] - a.ylab_ascent[i];
		    iy2 = a.y_ylab[i] - a.ylab_ascent[i];
		    if(iy1 < -.5*entry->tag_height || iy2> .5*entry->tag_height)
		    {
			if(entry->amp_width < -a.x_ylab[i]) {
			    entry->amp_width = -a.x_ylab[i];
			}
			a.x_ylab[i] += x0;
			a.y_ylab[i] += entry_y0;
			_AxesDrawString2((AxesWidget)w, ax->invert_clipGC,
				a.x_ylab[i], a.y_ylab[i], a.ylab[i]);
			j++;
		    }
		}
	    }
	}
	if(j <= 1)
	{
	    a.nylab = 0;
	    a.min_ylab = 1;
	    a.max_ylab = 1;
	    y = entry->mean - (.5*entry->tag_height + height)/yscale;
	    nicex(y1, y, a.min_ylab, a.max_ylab, &i, a.y_lab, &ndigit,
			&a.nydeci);
	    y = entry->mean + (.5*entry->tag_height + height)/yscale;
	    nicex(y, y2, a.min_ylab, a.max_ylab, &j, a.y_lab+2, &ndigit,
			&a.nydeci);
	    if(i > 0 && j > 0)
	    {
		a.y_lab[1] = .5*(a.y_lab[0] + a.y_lab[2]);
		a.nylab = 3;
		gdrawYAxis(&a, &d, 2, 0., y1, y2, a.ytic, True, 0, 1, 0.);
		for(i = 0; i < 3; i += 2)
		{
		    if(entry->amp_width < -a.x_ylab[i]) {
			entry->amp_width = -a.x_ylab[i];
		    }
		    a.x_ylab[i] += x0;
		    a.y_ylab[i] += entry_y0;
		    _AxesDrawString2((AxesWidget)w, ax->invert_clipGC,
				a.x_ylab[i], a.y_ylab[i], a.ylab[i]);
		}
	    }
	}
	if(a.axis_segs[2].n_segs > 0 && a.nylab > 1)
	{
	    a.axis_segs[2].segs[0].y1 = unscale_y(&d, a.y_lab[0]);
	    a.axis_segs[2].segs[0].y2 = unscale_y(&d, a.y_lab[a.nylab-1]);
	    for(j = 0; j < a.axis_segs[2].n_segs; j++)
	    {
		a.axis_segs[2].segs[j].x1 += x0;
		a.axis_segs[2].segs[j].x2 += x0;
		a.axis_segs[2].segs[j].y1 += entry_y0;
		a.axis_segs[2].segs[j].y2 += entry_y0;
	    }
	    _AxesDrawSegments2((AxesWidget)w, ax->invert_clipGC,
			a.axis_segs[2].segs, a.axis_segs[2].n_segs);
	}

	if(a.axis_segs[2].segs != NULL) {
	    Free(a.axis_segs[2].segs);
	}
	for(i = 0; i < a.nylab; i++) Free(a.ylab[i]);
}

static int
ComponentIndex(DataEntry *entry)
{
    int i;
    for(i = 0; i < NUM_COMPONENTS; i++) {
	if(entry == entry->comps[i]) break;
    }
    return(i);
}

/** 
 *  Get the minimum and maximum amplitude of the visible part of a waveform.
 *  
 */
void CPlotClass::minMax(Waveform *cd, double *min, double *max)
{
    CPlotPart *cp = &cw->c_plot;
    int i;

    for(i = 0; i < cp->num_entries; i++) {
	if(cd == cp->entry[i]->w) break;
    }
    if(i < cp->num_entries) {
	*min = cp->entry[i]->visible_ymin;
	*max = cp->entry[i]->visible_ymax;
    }
}

static void
DrawArrivalRec(CPlotWidget w, GC gc)
{
    CPlotPart *cp = &w->c_plot;
    XSegment segs[8];
    int x = cp->arrival_rec_x;
    int y = cp->arrival_rec_y;
    int width = cp->arrival_rec_width;
    int height = cp->arrival_rec_height;

    /* Draw two rectangles for a double line. Don't use DrawRectangles,
     * since the point at the top left corner is drawn twice, so the xor
     * makes it disappear.
     */
    // inside rectangle
    segs[0].x1 = x;
    segs[0].y1 = y;
    segs[0].x2 = x + width;
    segs[0].y2 = y;
    segs[1].x1 = x;
    segs[1].y1 = y + height;
    segs[1].x2 = x + width;
    segs[1].y2 = y + height;
    segs[2].x1 = x;
    segs[2].y1 = y+1;
    segs[2].x2 = x;
    segs[2].y2 = y + height-1;
    segs[3].x1 = x + width;
    segs[3].y1 = y+1;
    segs[3].x2 = x + width;
    segs[3].y2 = y + height-1;

    // outside rectangle
    segs[4].x1 = x-1;
    segs[4].y1 = y-1;
    segs[4].x2 = x-1 + width+2;
    segs[4].y2 = y-1;
    segs[5].x1 = x-1;
    segs[5].y1 = y + height+1;
    segs[5].x2 = x-1 + width+2;
    segs[5].y2 = y + height+1;
    segs[6].x1 = x-1;
    segs[6].y1 = y;
    segs[6].x2 = x-1;
    segs[6].y2 = y + height+1;
    segs[7].x1 = x + width+1;
    segs[7].y1 = y;
    segs[7].x2 = x + width+1;
    segs[7].y2 = y + height+1;

    XDrawSegments(XtDisplay(w), XtWindow(w), gc, segs, 8);

    if(w->axes.use_pixmap)
    {
	XDrawSegments(XtDisplay(w), w->axes.pixmap, gc, segs, 8);
    }
}

static void
DrawPointRec(CPlotWidget w, GC gc)
{
    CPlotPart *cp = &w->c_plot;
    XSegment segs[8];
    int x = cp->point_rec_x;
    int y = cp->point_rec_y;
    int width = cp->point_rec_width;
    int height = cp->point_rec_height;

    /* Draw two rectangles for a double line. Don't use DrawRectangles,
     * since the point at the top left corner is drawn twice, so the xor
     * makes it disappear.
     */
    // inside rectangle
    segs[0].x1 = x;
    segs[0].y1 = y;
    segs[0].x2 = x + width;
    segs[0].y2 = y;
    segs[1].x1 = x;
    segs[1].y1 = y + height;
    segs[1].x2 = x + width;
    segs[1].y2 = y + height;
    segs[2].x1 = x;
    segs[2].y1 = y+1;
    segs[2].x2 = x;
    segs[2].y2 = y + height-1;
    segs[3].x1 = x + width;
    segs[3].y1 = y+1;
    segs[3].x2 = x + width;
    segs[3].y2 = y + height-1;

    // outside rectangle
    segs[4].x1 = x-1;
    segs[4].y1 = y-1;
    segs[4].x2 = x-1 + width+2;
    segs[4].y2 = y-1;
    segs[5].x1 = x-1;
    segs[5].y1 = y + height+1;
    segs[5].x2 = x-1 + width+2;
    segs[5].y2 = y + height+1;
    segs[6].x1 = x-1;
    segs[6].y1 = y;
    segs[6].x2 = x-1;
    segs[6].y2 = y + height+1;
    segs[7].x1 = x + width+1;
    segs[7].y1 = y;
    segs[7].x2 = x + width+1;
    segs[7].y2 = y + height+1;

    XDrawSegments(XtDisplay(w), XtWindow(w), gc, segs, 8);

    if(w->axes.use_pixmap)
    {
	XDrawSegments(XtDisplay(w), w->axes.pixmap, gc, segs, 8);
    }
}

static void
DrawLine(CPlotWidget w, GC gc, int x1, int y1, int x2, int y2)
{
	if(!w->axes.use_pixmap || w->axes.use_screen)
	{
		XDrawLine(XtDisplay(w), XtWindow(w), gc, x1, y1, x2, y2);
	}
	if(w->axes.use_pixmap)
	{
		XDrawLine(XtDisplay(w), w->axes.pixmap, gc, x1, y1, x2, y2);
	}
}

static int
sort_by_y0(const void *A, const void *B)
{
    DataEntry **a = (DataEntry **)A;
    DataEntry **b = (DataEntry **)B;

    if((*a)->w->scaled_y0 != (*b)->w->scaled_y0) {
	return(((*a)->w->scaled_y0 < (*b)->w->scaled_y0) ? -1 : 1);
    }
    return(0);
}

static int
sort_cd_by_y0(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;

    if((*a)->scaled_y0 != (*b)->scaled_y0) {
	return(((*a)->scaled_y0 < (*b)->scaled_y0) ? -1 : 1);
    }
    return(0);
}

static int
sort_arrivals_by_time(const void *A, const void *B)
{
    ArrivalEntry **a = (ArrivalEntry **)A;
    ArrivalEntry **b = (ArrivalEntry **)B;

    if((*a)->a->time != (*b)->a->time) {
	return(((*a)->a->time < (*b)->a->time) ? -1 : 1);
    }
    return(0);
}

/**
 *  Search for array elements that bound a value.
 *  For monotonically increasing r[j].x1, this function returns the index j,
 *  such that r[j].x1 < findx <= r[j+1].x1.
 *  @param[in] r An array of Rseg structures with monotonically increasing x1.
 *  @param[in] n The length of r.
 *  @param[in] findx The value to search for.
 *  returns j
 */
static int
FindRSeg(RSeg *r, int n, double findx)
{
	int jl = -1, ju = n, jm;

	if(n <= 0) return(0);

	if(findx <= r[0].x1) return(0);

	if(findx >= r[n-1].x1) return(n-1);

	while(ju - jl > 1)
	{
	    jm = (ju + jl)/2;

	    if(findx > r[jm].x1)
	    {
		jl = jm;
	    }
	    else
	    {
		ju = jm;
	    }
	}
	return(jl);
}

const char * CPlotClass::getTag(Waveform *cd)
{
    CPlotPart *cp = &cw->c_plot;
    int i;

    for(i = 0; i < cp->num_entries && cp->entry[i]->w != cd; i++);
    if(i == cp->num_entries) return NULL;
    return cp->entry[i]->tag;
}

static char *arrivalPhase(CssArrivalClass *a)
{
    return (a->phase[0] != '\0' && strcmp(a->phase, "-")) ?
		a->phase : a->iphase;
}
/*
 * NAME
 *      plotxd: compute non-overlapping XSegments
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

static void finish_repeat(RSegArray *r, double last_x, double y_min,
			double y_max);

static void
drawTimeSeries(GTimeSeries *ts, GDataPoint *beg, GDataPoint *end, double mean,
	double xscale, double yscale, RSegArray *r)
{
	int	i, k, i1, i2;
	bool	repeat;
	float	*data;
	double	xmid, ymid, xd, yd;
	double	last_x, last_y;
	double	next_x, next_y;
	double	y_min, y_max;
	double	dx, dy, D, yincr, dx2, t0, tbeg, tdel;
	GSegment *s;

	r->nsegs = 0;
	if(ts->size() <= 0) return;

	tbeg = beg->time();
	r->xmax = floor((end->time() - beg->time())*xscale);
	r->xmin = 0.;

	s = ts->segment(beg->segmentIndex());
	last_y = -(s->data[beg->index()] - mean)*yscale;
	y_min = y_max = last_y;
	repeat = False;
	last_x = r->xmin;
	i1 = beg->index() + 1;
	k = (i1 < s->length()) ? beg->segmentIndex() : beg->segmentIndex() + 1;

	for(; k <= end->segmentIndex(); k++)
	{
	    s = ts->segment(k);
	    data = s->data;
	    t0 = s->tbeg() - tbeg;
	    tdel = s->tdel();
	    if(k > beg->segmentIndex()) {
		if(ts->continuous(k, .001*tdel, .001*tdel)) {
		    i1 = 0;
		}
		else {
		    if(repeat) {
			finish_repeat(r, last_x, y_min, y_max);
		    }
		    last_x = floor(t0*xscale);
		    last_y = -(data[0] - mean)*yscale;
		    y_min = y_max = last_y;
		    repeat = False;
		    i1 = 1;
		}
	    }
	    i2 = (k != end->segmentIndex()) ? s->length() - 1 : end->index();
	    for(i = i1; i <= i2 ; i++)
	    {
		xd = floor((t0 + i*tdel)*xscale+.5);
		yd = -(data[i] - mean)*yscale;
			
		if(xd < last_x) {
		    continue;
		}
		else if(xd == last_x) {
		    if(y_min > yd) y_min = yd;
		    else if(y_max < yd) y_max = yd;
		    repeat = True;
		    last_x = xd;
		    last_y = yd;
		    continue;
		}
		if(repeat) {
		    /* xd should be > last_x */
		    dx = xd - last_x;
		    dy = fabs(yd - last_y);
		    if( dx >= dy )
		    {
			next_x = last_x + 1;
			D = 2*dy - dx;
			if(D >= 0) {
			    next_y = (yd > last_y) ? last_y + 1 : last_y - 1;
			}
			else {
			    next_y = last_y;
			}
		    }
		    else {
			yincr = (yd > last_y) ? 1 : -1;
			next_y = last_y;
			dx2 = 2*dx;
			D = dx2 - dy;
			if(D < 0 ) {
			    next_y += yincr*ceil(fabs(D)/dx2);
			    if(next_y < y_min) y_min = next_y;
			    else if(next_y > y_max) y_max = next_y;
			}
			next_x = last_x + 1;
			next_y += yincr;
		    }
		    ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs,
					r->segs, RSeg, 512);
		    r->segs[r->nsegs].x1 = last_x;
		    r->segs[r->nsegs].y1 = y_min;
		    r->segs[r->nsegs].x2 = last_x;
		    r->segs[r->nsegs].y2 = y_max;

		    if(r->nsegs == 0) {
			r->ymin = y_min;
			r->ymax = y_max;
		    }
		    else {
			if(y_min < r->ymin) r->ymin = y_min;
			if(y_max > r->ymax) r->ymax = y_max;
		    }
		    r->nsegs++;
		    xmid = floor(next_x);
		    ymid =       next_y;
		}
		else {
		    xmid = last_x;
		    ymid = last_y;
		}
		if(xd == xmid) {
		    y_min = (yd < ymid) ? yd : ymid;
		    y_max = (yd > ymid) ? yd : ymid;
		    repeat = True;
		}
		else {
		    /* xd should be > last_x */
		    dx = xd - last_x;
		    y_min = y_max = yd;
		    repeat = False;

		    dy = fabs(last_y - yd);
		    if( dx >= dy ) {
			next_x = xd - 1;
			D = 2*dy - dx;
			if(D >= 0) {
			    next_y = (last_y > yd) ?   yd+1 : yd-1;
			}
			else {
			    next_y = yd;
			}
		    }
		    else
		    {
			yincr = (last_y > yd) ? 1 : -1;
			next_y = yd;
			dx2 = 2*dx;
			D = dx2 - dy;
			if(D < 0 )
			{
			    repeat = True;
			    next_y += yincr*ceil(fabs(D)/dx2);
			    if(next_y < y_min) y_min = next_y;
			    else if(next_y > y_max) y_max = next_y;
			}
			next_x = xd - 1;
			next_y += yincr;
		    }
		    ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs,
					r->segs, RSeg, 512);
		    next_x = floor(next_x);
		    next_y =       next_y;
		    r->segs[r->nsegs].x1 = xmid;
		    r->segs[r->nsegs].y1 = ymid;
		    r->segs[r->nsegs].x2 = next_x;
		    r->segs[r->nsegs].y2 = next_y;
		    if(r->nsegs == 0) {
			if(ymid < next_y) {
			    r->ymin = ymid;
			    r->ymax= next_y;
			}
			else {
			    r->ymin = next_y;
			    r->ymax= ymid;
			}
		    }
		    else {
			if(ymid < next_y) {
			    if(ymid < r->ymin) r->ymin = ymid;
			    if(next_y > r->ymax) r->ymax = next_y;	
			}
			else {
			    if(next_y < r->ymin) r->ymin = next_y;
			    if(ymid > r->ymax) r->ymax = ymid;
			}
		    }
		    r->nsegs++;
		}
		last_x = xd;
		last_y = yd;
	    }
	}
        if(repeat)
	{
	    finish_repeat(r, last_x, y_min, y_max);
	}
}

static void
finish_repeat(RSegArray	*r, double last_x, double y_min, double y_max)
{
	ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs, r->segs, RSeg, 512);
	r->segs[r->nsegs].x1 = last_x;
	r->segs[r->nsegs].y1 = y_min;
	r->segs[r->nsegs].x2 = last_x;
	r->segs[r->nsegs].y2 = y_max;
	if(r->nsegs == 0)
	{
		r->ymin = y_min;
		r->ymax = y_max;
	}
	else
	{
		if(y_min < r->ymin) r->ymin = y_min;
		if(y_max > r->ymax) r->ymax = y_max;
	}
	r->nsegs++;
}

void CPlotSetClass(CPlotWidget w, CPlotClass *a)
{
    w->c_plot.cplot_class = a;
}
