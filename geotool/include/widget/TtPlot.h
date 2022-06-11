#ifndef	_TTPLOT_H_
#define	_TTPLOT_H_

#include "widget/CPlot.h"
#include "TravelTime.h"
extern "C" {
#include "crust.h"

/* lat,lon: degrees
 * depth: km
 * elev: meters
 * tt: seconds
 * slowness: sec/deg
 */
typedef int (*ComputeTTMethod)(double origin_time, double origin_lat,
			double origin_lon, double origin_depth, double delta,
			double esaz, const char *net, double lat, double lon,
			double elev, int num_phases, const char **phases,
			double *tt, double *slowness);
}

#define XtNgeoTableDir			(char *)"geoTableDir"
#define XtNdisplayTtCurves		(char *)"displayTtCurves"
#define XtNdisplayTtLabels		(char *)"displayTtLabels"
#define XtNreducedTime			(char *)"reducedTime"
#define XtNselectedPhases		(char *)"selectedPhases"
#define XtNiaspeiColor			(char *)"iaspeiColor"
#define XtNiaspeiCurveColor		(char *)"iaspeiCurveColor"
#define XtNjbColor			(char *)"jbColor"
#define XtNjbTable			(char *)"jbTable"
#define XtNiaspeiTable			(char *)"iaspeiTable"
#define XtNregionalColor		(char *)"regionalColor"
#define XtNplotRay			(char *)"plotRay"
#define XtNalignPredSelected		(char *)"alignPredSelected"
#define XtNuseCelerity			(char *)"useCelerity"

#define XtCGeoTableDir			(char *)"GeoTableDir"
#define XtCDisplayTtCurves		(char *)"DisplayTtCurves"
#define XtCDisplayTtLabels		(char *)"DisplayTtLabels"
#define XtCReducedTime			(char *)"ReducedTime"
#define XtCSelectedPhases		(char *)"SelectedPhases"
#define XtCIaspeiColor			(char *)"IaspeiColor"
#define XtCIaspeiCurveColor		(char *)"IaspeiCurveColor"
#define XtCJbColor			(char *)"JbColor"
#define XtCIaspeiTable			(char *)"IaspeiTable"
#define XtCJbTable			(char *)"JbTable"
#define XtCRegionalColor		(char *)"RegionalColor"
#define XtCPlotRay			(char *)"PlotRay"
#define XtCAlignPredSelected		(char *)"AlignPredSelected"
#define XtCDisplayCurves		(char *)"DisplayCurves"
#define XtCUseCelerity			(char *)"UseCelerity"

#define IASPEI		0
#define	JB		1
#define	REGIONAL	2
#define	SURFACE		3
#define DEGREES		0
#define KILOMETERS	1

/// @cond
typedef struct _TtPlotClassRec	*TtPlotWidgetClass;
typedef struct _TtPlotRec	*TtPlotWidget;

extern WidgetClass		ttPlotWidgetClass;
/// @endcond

typedef struct
{
	char		sta[7];
	char		phase[9];
	int		arid;
	int		group_id;
	int		in_use;
	Pixel		fg;
	double		time;
} InputPredArr;

Widget TtPlotCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);

#endif	/* _TTPLOT_H_ */
