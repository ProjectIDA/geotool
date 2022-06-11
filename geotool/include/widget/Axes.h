#ifndef	_AXES_H_
#define	_AXES_H_

#include <stdio.h>
#include <Xm/Xm.h>
extern "C" {
#include "libgplot.h"
}
#include "PrintParam.h"
#include <string>
using namespace std;

#define XtNmagRectColor 		(char *)"magRectColor"
#define XtNreverseX			(char *)"reverseX"
#define XtNreverseY			(char *)"reverseY"
#define XtNxLabel			(char *)"xLabel"
#define XtNyLabel			(char *)"yLabel"
#define XtNyLabelInt			(char *)"yLabelInt"
#define XtNlogX				(char *)"logX"
#define XtNlogY				(char *)"logY"
#define XtNtimeScale			(char *)"timeScale"
#define XtNhorizontalScroll		(char *)"horizontalScroll"
#define XtNverticalScroll		(char *)"verticalScroll"
#define XtNverticalScrollPosition	(char *)"verticalScrollPosition"
#define XtNtickmarksInside		(char *)"tickmarksInside"
#define XtNclearOnScroll		(char *)"clearOnScroll"
#define XtNzoomControls			(char *)"zoomControls"
#define XtNmagnifyOnly			(char *)"magnifyOnly"
#define XtNallowCursorChange		(char *)"allowCursorChange"
#define XtNlabelMode			(char *)"labelMode"
#define XtNdisplayAxesLabels		(char *)"displayAxesLabels"
#define XtNdisplayAxes			(char *)"displayAxes"
#define XtNxAxis			(char *)"xAxis"
#define XtNyAxis			(char *)"yAxis"
#define XtNdataLinewidth		(char *)"dataLinewidth"
#define XtNredisplay			(char *)"redisplay"
#define XtNselectColor			(char *)"selectColor"
#define XtNusePixmap			(char *)"usePixmap"
#define XtNuniformScale			(char *)"uniformScale"
#define XtNautoYScale			(char *)"autoYScale"
#define XtNzoomCallback			(char *)"zoomCallback"
#define XtNcrosshairCallback		(char *)"crosshairCallback"
#define XtNcrosshairDragCallback	(char *)"crosshairDragCallback"
#define XtNlineCallback			(char *)"lineCallback"
#define XtNlineDragCallback		(char *)"lineDragCallback"
#define XtNdoubleLineCallback		(char *)"doubleLineCallback"
#define XtNdoubleLineDragCallback	(char *)"doubleLineDragCallback"
#define XtNdoubleLineScaleCallback	(char *)"doubleLineScaleCallback"
#define XtNphaseLineCallback		(char *)"phaseLineCallback"
#define XtNphaseLineDragCallback	(char *)"phaseLineDragCallback"
#define XtNlimitsCallback		(char *)"limitsCallback"
#define XtNhorizontalScrollCallback	(char *)"horizontalScrollCallback"
#define XtNmagnifyCallback		(char *)"magnifyCallback"
#define XtNwarningCallback		(char *)"warningCallback"
#define XtNkeyPressCallback		(char *)"keyPressCallback"
#define XtNresizeCallback		(char *)"resizeCallback"
#define XtNinfoWidget			(char *)"infoWidget"
#define XtNinfoWidget2			(char *)"infoWidget2"
#define XtNaxesFont			(char *)"axesFont"
#define XtNleftMargin			(char *)"leftMargin"
#define XtNrightMargin			(char *)"rightMargin"
#define XtNtopMargin			(char *)"topMargin"
#define XtNbottomMargin			(char *)"bottomMargin"
#define XtNscrollbarAsNeeded		(char *)"scrollbarAsNeeded"
#define XtNdrawYLabels			(char *)"drawYLabels"
#define XtNdoubleLineAnchor		(char *)"doubleLineAnchor"
#define XtNdisplayGrid			(char *)"displayGrid"
#define XtNsaveSpace			(char *)"saveSpace"
#define XtNcursorLabelAlways		(char *)"cursorLabelAlways"
#define XtNextraYTickmarks		(char *)"extraYTickmarks"
#define XtNextraXTickmarks		(char *)"extraXTickmarks"
#define XtNdisplayXLabel		(char *)"displayXLabel"
#define XtNdisplayYLabel		(char *)"displayYLabel"
#define XtNminXLab			(char *)"minXLab"
#define XtNmaxXLab			(char *)"maxXLab"
#define XtNminYLab			(char *)"minYLab"
#define XtNmaxYLab			(char *)"maxYLab"
#define XtNminXSmall			(char *)"minXSmall"
#define XtNmaxXSmall			(char *)"maxXSmall"
#define XtNminYSmall			(char *)"minYSmall"
#define XtNmaxYSmall			(char *)"maxYSmall"
#define XtNdisplayAddArrival		(char *)"displayAddArrival"

#define XtCMagRectColor 		(char *)"MagRectColor"
#define XtCReverseX			(char *)"ReverseX"
#define XtCReverseY			(char *)"ReverseY"
#define XtCXLabel			(char *)"XLabel"
#define XtCYLabel			(char *)"YLabel"
#define XtCYLabelInt			(char *)"YLabelInt"
#define XtCLogX				(char *)"LogX"
#define XtCLogY				(char *)"LogY"
#define XtCTimeScale			(char *)"TimeScale"
#define XtCHorizontalScroll		(char *)"HorizontalScroll"
#define XtCVerticalScroll		(char *)"VerticalScroll"
#define XtCVerticalScrollPosition	(char *)"VerticalScrollPosition"
#define XtCTickmarksInside		(char *)"TickmarksInside"
#define XtCClearOnScroll		(char *)"ClearOnScroll"
#define XtCZoomControls			(char *)"ZoomControls"
#define XtCMagnifyOnly			(char *)"MagnifyOnly"
#define XtCAllowCursorChange		(char *)"AllowCursorChange"
#define XtCLabelMode			(char *)"LabelMode"
#define XtCDisplayAxesLabels		(char *)"DisplayAxesLabels"
#define XtCDisplayAxes			(char *)"DisplayAxes"
#define XtCXAxis			(char *)"XAxis"
#define XtCYAxis			(char *)"YAxis"
#define XtCDataLinewidth		(char *)"DataLinewidth"
#define XtCRedisplay			(char *)"Redisplay"
#define XtCSelectColor			(char *)"SelectColor"
#define XtCUsePixmap			(char *)"UsePixmap"
#define XtCUniformScale			(char *)"UniformScale"
#define XtCAutoYScale			(char *)"AutoYScale"
#define XtCZoomCallback			(char *)"ZoomCallback"
#define XtCCrosshairCallback		(char *)"CrosshairCallback"
#define XtCCrosshairDragCallback	(char *)"CrosshairDragCallback"
#define XtCLineCallback			(char *)"LineCallback"
#define XtCLineDragCallback		(char *)"LineDragCallback"
#define XtCDoubleLineCallback		(char *)"DoubleLineCallback"
#define XtCDoubleLineDragCallback	(char *)"DoubleLineDragCallback"
#define XtCDoubleLineScaleCallback	(char *)"DoubleLineScaleCallback"
#define XtCPhaseLineCallback		(char *)"PhaseLineCallback"
#define XtCPhaseLineDragCallback	(char *)"PhaseLineDragCallback"
#define XtCLimitsCallback		(char *)"LimitsCallback"
#define XtCHorizontalScrollCallback	(char *)"HorizontalScrollCallback"
#define XtCMagnifyCallback		(char *)"MagnifyCallback"
#define XtCWarningCallback		(char *)"WarningCallback"
#define XtCKeyPressCallback		(char *)"KeyPressCallback"
#define XtCResizeCallback		(char *)"ResizeCallback"
#define XtCInfoWidget			(char *)"InfoWidget"
#define XtCInfoWidget2			(char *)"InfoWidget2"
#define XtCAxesFont			(char *)"AxesFont"
#define XtCLeftMargin			(char *)"LeftMargin"
#define XtCRightMargin			(char *)"RightMargin"
#define XtCTopMargin			(char *)"TopMargin"
#define XtCBottomMargin			(char *)"BottomMargin"
#define XtCScrollbarAsNeeded		(char *)"ScrollbarAsNeeded"
#define XtCDrawYLabels			(char *)"DrawYLabels"
#define XtCDoubleLineAnchor		(char *)"DoubleLineAnchor"
#define XtCDisplayGrid			(char *)"DisplayGrid"
#define XtCSaveSpace			(char *)"SaveSpace"
#define XtCCursorLabelAlways		(char *)"CursorLabelAlways"
#define XtCExtraYTickmarks		(char *)"ExtraYTickmarks"
#define XtCExtraXTickmarks		(char *)"ExtraXTickmarks"
#define XtCDisplayXLabel		(char *)"DisplayXLabel"
#define XtCDisplayYLabel		(char *)"DisplayYLabel"
#define XtCMinXLab			(char *)"MinXLab"
#define XtCMaxXLab			(char *)"MaxXLab"
#define XtCMinYLab			(char *)"MinYLab"
#define XtCMaxYLab			(char *)"MaxYLab"
#define XtCMinXSmall			(char *)"MinXSmall"
#define XtCMaxXSmall			(char *)"MaxXSmall"
#define XtCMinYSmall			(char *)"MinYSmall"
#define XtCMaxYSmall			(char *)"MaxYSmall"
#define XtCDisplayAddArrival		(char *)"DisplayAddArrival"


/// @cond
typedef struct _AxesClassRec	*AxesWidgetClass;
typedef struct _AxesRec		*AxesWidget;
/// @endcond

extern WidgetClass		axesWidgetClass;

typedef struct
{
	int	index;			/* internal cursor index */
        int     type;			/* AXES_CROSSHAIR, AXES_LINE, etc */
	char	label[50];		/* label */
	int	reason;			/* AXES_CROSSHAIR_POSITION, etc. */
        int     x, y;			/* cursor position in pixels */
        double	scaled_y, scaled_x;	/* scaled cursor position */
	int	x1, x2;			/* AXES_DOUBLE_LINE pixel positions */
	int	y1, y2;			/* AXES_DOUBLE_LINE pixel positions */
	double	scaled_x1, scaled_x2;	/* AXES_DOUBLE_LINE scaled positions */
	double	scaled_y1, scaled_y2;	/* AXES_DOUBLE_LINE scaled positions */
} AxesCursorCallbackStruct;

typedef struct
{
	Boolean limits;			/* true if plot limits have changed */
	Boolean x_margins;		/* true if x margins have changed */
	Boolean y_margins;		/* true if y margins have changed */
	double x_min, x_max, y_min, y_max; /* current plot limits */
	int left, right, top, bottom;	/* current axes limits in pixels */
} AxesLimitsCallbackStruct;

typedef struct
{
	int left, right, top, bottom;	/* current axes margins in pixels */
} AxesMarginsCallbackStruct;

typedef struct
{
	int		x, y, nxdeci, nydeci, type, x1, x2, y1, y2;
	int		x_xlab, y_xlab, x_width, x_height, x_off;
	int		x_ylab, y_ylab, y_width, y_height, y_off;
	int		label_x, label_y, label_width, label_height;
	int		label_ascent, label_off;
	char		xlab[32], ylab[32];
	char		label[50];
	Boolean		anchor_on;
	Boolean		draw_labels;
	int		nsegs;
	XSegment	segs[22];
	double		scaled_x, scaled_y, scaled_x1, scaled_x2;
	double		scaled_y1, scaled_y2;
} AxesCursor;

typedef struct
{
	int		x1, y1;
	double		x, y;
	char		buf[20];
	int		len;
	KeySym		key;
	XComposeStatus	cs;
} AxesKeyPressCallbackStruct;


#define AXES_CROSSHAIR_INIT \
{ \
	0,0,0,0,	/* x, y, nxdeci, nydeci */ \
	0,0,0,0,0,	/* type, x1, x2, y1, y2 */ \
	0,0,0,0,0,	/* x_xlab, y_xlab, x_width, x_height, x_off */ \
	0,0,0,0,0,	/* x_ylab, y_ylab, y_width, y_height, y_off */ \
	0,0,0,0,  	/* label_x, label_y, label_width, label_height */ \
	0,0,		/* label_ascent, label_off */ \
	"", "",		/* xlab, ylab */ \
	"",		/* label */ \
	False,		/* anchor_on */ \
	True,		/* draw_labels */ \
	0,		/* nsegs */ \
	{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}, \
	 {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}, \
	 {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}, \
	 {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}, \
	 {0,0,0,0},{0,0,0,0}},	/* segs[22] */ \
	0.,0.,0.,0.,	/* scaled_x, scaled_y, scaled_x1, scaled_x2 */ \
	0., 0.,		/* scaled_y1, scaled_y2 */ \
}

#define	N_DATA_COLORS	6

#define AXES_CROSSHAIR		0
#define AXES_HORIZONTAL_LINE	1
#define AXES_VERTICAL_LINE	2
#define AXES_HOR_DOUBLE_LINE	3
#define AXES_VER_DOUBLE_LINE	4
#define AXES_PHASE_LINE		5

#define AXES_CROSSHAIR_POSITION 	0
#define AXES_CROSSHAIR_DRAG 		1
#define AXES_CROSSHAIR_DELETE 		2
#define AXES_LINE_POSITION 		3
#define AXES_LINE_DRAG 			4
#define AXES_DOUBLE_LINE_POSITION	5
#define AXES_DOUBLE_LINE_DRAG		6
#define AXES_DOUBLE_LINE_SCALE		7
#define AXES_PHASE_LINE_POSITION	8
#define AXES_PHASE_LINE_DRAG		9

typedef void (*AxesRedrawFunction)(AxesWidget w, int type, double shift);
typedef void (*AxesResizeFunction)(AxesWidget w);
typedef void (*AxesCursorFunction)(AxesWidget w, XEvent *e, AxesCursor *c);
typedef void (*AxesHardCopyFunction)(AxesWidget w, FILE *fp, DrawStruct *d,
			AxesParm *a, float font_scale, PrintParam *p);
typedef void (*AxesSelectDataFunction)(AxesWidget w);
typedef void (*AxesTransformFunction)(AxesWidget w, double x, double y,
			char *xlab, char *ylab);

void AxesSetRedrawFunction(AxesWidget w, AxesRedrawFunction redraw_data_func);
void AxesSetResizeFunction(AxesWidget w, AxesResizeFunction resize_func);
void AxesSetCursorFunction(AxesWidget w, AxesCursorFunction cursor_func);
void AxesSetHardCopyFunction(AxesWidget w, AxesHardCopyFunction hard_copy_func);
void AxesSetSelectDataFunction(AxesWidget w,
			AxesSelectDataFunction select_data_func);
void AxesSetTransformFunction(AxesWidget w, AxesTransformFunction transform);

void AxesZoomApply(AxesWidget w, int mode);
void AxesSetYLimits(AxesWidget w, double ymin, double ymax, bool init);
void AxesPostColor(AxesWidget w, FILE *fp, Pixel pixel);
int AxesPage(AxesWidget w, XEvent *event, const char **params,
			Cardinal *num_params);
void AxesUnzoomAll(AxesWidget w);
Widget AxesCreate(Widget parent, String name, ArgList arglist, Cardinal n);
class AxesClass;
void AxesSetClass(AxesWidget w, AxesClass *a);

#endif
