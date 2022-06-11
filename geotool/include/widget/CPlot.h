#ifndef	_CPLOT_H_
#define	_CPLOT_H_

#include <stdio.h>
#include <sys/param.h>
#include "gobject++/cvector.h"
#include "widget/Axes.h"
#include "gobject++/CssTables.h"
#include "gobject++/gvector.h"

class GTimeSeries;
class Waveform;

#define XtNallowPartialSelect		(char *)"allowPartialSelect"
#define XtNdisplayTags			(char *)"displayTags"
#define XtNdataMovement			(char *)"dataMovement"
#define XtNscrollData			(char *)"scrollData"
#define XtNdisplayArrivals		(char *)"displayArrivals"
#define XtNdisplayPredictedArrivals	(char *)"displayPredictedArrivals"
#define XtNdisplayAssocOnly		(char *)"displayAssocOnly"
#define XtNdisplayAmplitudeScale	(char *)"displayAmplitudeScale"
#define XtNcurvesOnly			(char *)"curvesOnly"
#define XtNalignArrSelected		(char *)"alignArrSelected"
#define XtNarrivalFont			(char *)"arrivalFont"
#define XtNassociatedArrivalFont	(char *)"associatedArrivalFont"
#define XtNtagFont			(char *)"tagFont"
#define XtNampFont			(char *)"ampFont"
#define XtNdistanceUnits		(char *)"distanceUnits"
//#define XtNcomponentTimeLimit		(char *)"componentTimeLimit"
#define XtNampInterpolation		(char *)"ampInterpolation"
#define XtNthreeHalfCycles		(char *)"threeHalfCycles"
#define XtNlimitSelect			(char *)"limitSelect"
#define XtNmeasureBox			(char *)"measureBox"
#define XtNdataHeight			(char *)"dataHeight"
//#define XtNdataSpacing			(char *)"dataSpacing"
#define XtNdataSeparation		(char *)"dataSeparation"
#define XtNsingleArrSelect		(char *)"singleArrSelect"
#define XtNredrawSelectedData		(char *)"redrawSelectedData"
#define XtNsingleSelectDataCallback	(char *)"singleSelectDataCallback"
#define XtNselectDataCallback		(char *)"selectDataCallback"
#define XtNpositionCallback		(char *)"positionCallback"
#define XtNpositionDragCallback		(char *)"positionDragCallback"
#define XtNselectArrivalCallback	(char *)"selectArrivalCallback"
#define XtNmeasureCallback		(char *)"measureCallback"
#define XtNampConvertCallback		(char *)"ampConvertCallback"
#define XtNretimeCallback		(char *)"retimeCallback"
#define XtNretimeDragCallback		(char *)"retimeDragCallback"
#define XtNwaveformMenuCallback		(char *)"waveformMenuCallback"
#define XtNarrivalMenuCallback		(char *)"arrivalMenuCallback"
#define XtNwaveformInfoCallback		(char *)"waveformInfoCallback"
#define XtNarrivalInfoCallback		(char *)"arrivalInfoCallback"
#define XtNmodifyWaveformCallback	(char *)"modifyWaveformCallback"
#define XtNmenuCallback			(char *)"menuCallback"
#define XtNjoinTimeSeries		(char *)"joinTimeSeries"
//#define XtNjoinTimeLimit		(char *)"joinTimeLimit"
//#define XtNoverlapLimit			(char *)"overlapLimit"
#define XtNhighlightPoints		(char *)"hightlightPoints"
//#define XtNinfoDelay			(char *)"infoDelay"
#define XtNadjustTimeLimits		(char *)"adjustTimeLimits"
#define XtNaddArrivalCallback		(char *)"addArrivalCallback"
#define XtNaddArrivalMenuCallback	(char *)"addArrivalMenuCallback"

#define XtCAllowPartialSelect		(char *)"AllowPartialSelect"
#define XtCDisplayTags			(char *)"DisplayTags"
#define XtCDataMovement			(char *)"DataMovement"
#define XtCScrollData			(char *)"ScrollData"
#define XtCCrosshairs			(char *)"Crosshairs"
#define XtCDisplayArrivals		(char *)"DisplayArrivals"
#define XtCDisplayPredictedArrivals	(char *)"DisplayPredictedArrivals"
#define XtCDisplayAssocOnly		(char *)"DisplayAssocOnly"
#define XtCDisplayAmplitudeScale	(char *)"DisplayAmplitudeScale"
#define XtCCurvesOnly			(char *)"CurvesOnly"
#define XtCAlignArrSelected		(char *)"AlignArrSelected"
#define XtCArrivalFont			(char *)"ArrivalFont"
#define XtCAssociatedArrivalFont	(char *)"AssociatedArrivalFont"
#define XtCTagFont			(char *)"TagFont"
#define XtCAmpFont			(char *)"AmpFont"
#define XtCDistanceUnits		(char *)"DistanceUnits"
//#define XtCComponentTimeLimit		(char *)"ComponentTimeLimit"
#define XtCAmpInterpolation		(char *)"AmpInterpolation"
#define XtCThreeHalfCycles		(char *)"ThreeHalfCycles"
#define XtCLimitSelect			(char *)"LimitSelect"
#define XtCSingleArrSelect		(char *)"SingleArrSelect"
#define XtCRedrawSelectedData		(char *)"RedrawSelectedData"
#define XtCMeasureBox			(char *)"MeasureBox"
#define XtCDataHeight			(char *)"DataHeight"
//#define XtCDataSpacing			(char *)"DataSpacing"
#define XtCDataSeparation		(char *)"DataSeparation"
#define XtCSingleSelectDataCallback	(char *)"SingleSelectDataCallback"
#define XtCSelectDataCallback		(char *)"SelectDataCallback"
#define XtCPositionCallback		(char *)"PositionCallback"
#define XtCPositionDragCallback		(char *)"PositionDragCallback"
#define XtCSelectArrivalCallback	(char *)"SelectArrivalCallback"
#define XtCMeasureCallback		(char *)"MeasureCallback"
#define XtCAmpConvertCallback		(char *)"AmpConvertCallback"
#define XtCRetimeCallback		(char *)"RetimeCallback"
#define XtCWaveformMenuCallback		(char *)"WaveformMenuCallback"
#define XtCArrivalMenuCallback		(char *)"ArrivalMenuCallback"
#define XtCWaveformInfoCallback		(char *)"WaveformInfoCallback"
#define XtCArrivalInfoCallback		(char *)"ArrivalInfoCallback"
#define XtCRetimeDragCallback		(char *)"RetimeDragCallback"
#define XtCModifyWaveformCallback	(char *)"ModifyWaveformCallback"
#define XtCMenuCallback			(char *)"MenuCallback"
#define XtCJoinTimeSeries		(char *)"JoinTimeSeries"
//#define XtCJoinTimeLimit		(char *)"JoinTimeLimit"
//#define XtCOverlapLimit			(char *)"OverlapLimit"
#define XtCHighlightPoints		(char *)"HightlightPoints"
//#define XtCInfoDelay			(char *)"InfoDelay"
#define XtCAdjustTimeLimits		(char *)"AdjustTimeLimits"
#define XtCAddArrivalCallback		(char *)"AddArrivalCallback"
#define XtCAddArrivalMenuCallback	(char *)"AddArrivalMenuCallback"

#ifndef XtRDouble
#define XtRDouble	(char *)"double"
#endif

/// @cond
typedef struct _CPlotClassRec	*CPlotWidgetClass;
typedef struct _CPlotRec	*CPlotWidget;

extern WidgetClass		cPlotWidgetClass;
/// @endcond


#define MAX_COMPONENTS  3
#define NUM_COMPONENTS  3

#ifndef NULL_TIME_CHECK
#define NULL_TIME_CHECK -999999999.
#endif

#define TIME_TYPES "Time values must be in one of these formats:\n\
1992/12/30 18:34:09    (hh:mm:ss is optional)\n\
92/12/30 18:34:09    (hh:mm:ss is optional)\n\
1992/326 18:34:09    (hh:mm:ss is optional)\n\
1992326 18:34:09    (hh:mm:ss is optional)\n\
3737626262.539       (epoch time)\n\
now\n\
now - 30m \n\
[in addition to minutes, time can also be added or\n\
subtracted in days (d), hours (h), minutes (m), or seconds (s)]\n\
The slash in the dates may be replaced with a dash -.\n\
A dash representing subtraction must be preceeded by a blank."


/** 
 *  A structure for CPlotAddTimeSeries and TtPlotAddTimeSeries.
 *  <pre>
 *	CPlotInputStruct input;
 *	CPlotWidget w;
 *	TimeSeries ts;
 *	...
 *
 *	CPlotAddTimeSeries(w, ts, &input);
 *  </pre>
 *  @see CPlotAddTimeSeries
 *  @see TtPlotAddTimeSeries
 */
class CPlotInputStruct
{
    public:
	double		display_t0;	/* display start time */
	string		tag;		/* waveforms tag */
	string		chan;		/* channel name */
	CssOriginClass	*origin;	/* associated origin */
	Pixel		color;		/* waveform color */
	bool		on;		/* initially on or off */
	int		component;	/* component */
    CPlotInputStruct(void) {
	display_t0 = 0.;
	origin = NULL;
	color = 0;
	on = true;
	component = 0;
    }
};

/** 
 *  Structure to specify arrival hardcopy characteristics.
 *  @see CPlotAddArrivalType
 */
class CPlotArrivalType : public Gobject
{
    public:
        float   min_display_value;
        float   max_display_value;
        int     display_width_pix;
        int     hardcopy_width_pix;
        int     display_min_height_pix;
        int     display_max_height_pix;
        int     hardcopy_min_height_pix;
        int     hardcopy_max_height_pix;
        CPlotArrivalType(void) {
            min_display_value = 0.;
            max_display_value = 0.;
            display_width_pix = 0;
            hardcopy_width_pix = 0;
            display_min_height_pix = 0;
            display_max_height_pix = 0;
            hardcopy_min_height_pix = 0;
            hardcopy_max_height_pix = 0;
        }
        bool operator==(const CPlotArrivalType &rhs) { return (
            min_display_value == rhs.min_display_value &&
            max_display_value == rhs.max_display_value &&
            display_width_pix == rhs.display_width_pix &&
            hardcopy_width_pix == rhs.hardcopy_width_pix &&
            display_min_height_pix == rhs.display_min_height_pix &&
            display_max_height_pix == rhs.display_max_height_pix &&
            hardcopy_min_height_pix == rhs.hardcopy_min_height_pix &&
            hardcopy_max_height_pix == rhs.hardcopy_max_height_pix)
                ? true : false;
        }
};

class ArrivalKey
{
    public:
    ArrivalKey() { }
    string name;
    string key;
};

/** 
 *  The positionCallback resource callback structure.  This structure
 *  is the callback data to functions that are registered with a
 *  CPlotWidget with the positionCallback resource.
 */
typedef struct
{
	gvector<Waveform	*> wvec;
	int		reason;	/* DATA_DRAG or DATA_POSITION */
} CPlotPositionCallbackStruct;

typedef struct
{
	CPlotWidget	main_plot;
	int		reason;	/* CPLOT_WAVEFORM or CPLOT_ARRIVAL */
	Waveform	*w;
	CssArrivalClass	*arrival;
	XEvent		*event;
} CPlotMenuCallbackStruct;

typedef struct
{
	gvector<Waveform *> wvec;
} CPlotModifyCallbackStruct;

typedef struct
{
	Waveform	*w;
	CssArrivalClass	*a;
	char		label[500];
} CPlotInfoCallbackStruct;

typedef struct
{
	Waveform	*w;
	bool		shift;
	char		name[10];
	double		time;
	XEvent		*event;
} CPlotArrivalCallbackStruct;

#define CPLOT_WAVEFORM			0
#define CPLOT_ARRIVAL			1

#define CPLOT_DATA			0
#define CPLOT_CURVE			1
#define CPLOT_TTCURVE			2

#define CPLOT_DATA_DRAG			0
#define CPLOT_DATA_POSITION		1

#define CPLOT_INDEPENDENT		0
#define CPLOT_UNIFORM_PIXEL		1
#define CPLOT_UNIFORM_CONV		2

#define CPLOT_XY_MOVEMENT		0
#define CPLOT_NO_MOVEMENT		1
#define CPLOT_X_MOVEMENT		2
#define CPLOT_Y_MOVEMENT		3

#define CPLOT_ALL_COMPONENTS		-1
#define CPLOT_E_COMPONENT		0
#define CPLOT_N_COMPONENT		1
#define CPLOT_Z_COMPONENT		2
#define CPLOT_HORIZONTAL_COMPONENTS	3

#define CPLOT_HOLD_LIMITS		0
#define CPLOT_ADJUST_X_LIMITS		1
#define CPLOT_ADJUST_Y_LIMITS		2
#define CPLOT_ADJUST_LIMITS		3
#define CPLOT_ADJUST_Y_GROW		4


#define CPLOT_ARRIVALS_OFF		0
#define CPLOT_ARRIVALS_ONE_CHAN		1
#define CPLOT_ARRIVALS_ALL_CHAN		2

#define CPLOT_SELECTED_ARRIVAL        0
#define CPLOT_SELECTED_WAVEFORM       1
#define CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM 2

typedef struct
{
	int	npts;
	double	*x;
	float	*y;
	int	type;
	char	*label;
	char	*mouse_label;
	Pixel	pixel;
} CPlotCurve;

/**
 *  Information about a waveform amplitude and period measurement.
 *  @see CPlotGetArrivalMeasureBox
 */
typedef struct
{
	Waveform *w;		/* CPlotData structure for measured waveform */
	double  amp_cnts;	/* measured amplitude */
	double  amp_Nnms;	/* measured amplitude (nominal) */
	double  amp_nms;	/* measured amplitude (corrected) */
	double  zp_Nnms;	/* zp measured amplitude (nominal) */
	double  period;		/* measured period */
	double  left_side;	/* position (time) of the left side of the box*/
	double  bottom_side;	/* position (amp) of the bottom side */
} CPlotMeasure;

typedef struct {
	char key_code[20];
	char proc_call[50];
} CPlotKeyAction;

int CPlotGetCurves(CPlotWidget w, CPlotCurve **curves);
void CPlotSelectAll(CPlotWidget w, bool on_off, bool visible_only,
			int chan_type);
void CPlotMagnifyWidget(CPlotWidget w, CPlotWidget z, Boolean redisplay);
Widget CPlotCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);
void CPlotZoomOut(CPlotWidget w);
bool CPlotGetDataDuration(CPlotWidget w, double *tmin, double *tmax);
void CPlotSetTimeLimits(CPlotWidget w, double tmin, double tmax);
void CPlotGetTagDimensions(CPlotWidget w, const string &tag, int *width,
			int *height);
void CPlotSetKeyActions(int num, CPlotKeyAction *keys);
void CPlotRemoveAllMeasureBoxes(CPlotWidget w);
int CPlotGetKeyActions(CPlotKeyAction **keys);
int CPlotGetDefaultKeyActions(CPlotKeyAction **keys);
void CPlotRenameArrival(CPlotWidget w, CssArrivalClass *arrival,const string &phase);
void CPlotZoomOut(CPlotWidget w);
bool CPlotGetDataDuration(CPlotWidget w, double *tmin, double *tmax);
class CPlotClass;
void CPlotSetClass(CPlotWidget w, CPlotClass *a);

#endif	/* _CPLOT_H_ */
