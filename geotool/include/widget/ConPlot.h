#ifndef	CONPLOT_H_
#define	CONPLOT_H_

#include "widget/Axes.h"

#define XtNdataColor 			(char *)"dataColor"
#define XtNcontourLabelColor 		(char *)"contourLabelColor"
#define XtNcontourLabelSize 		(char *)"contourLabelSize"
#define XtNmarkMax			(char *)"markMax"
#define XtNautoInterval			(char *)"autoInterval"
#define XtNmode				(char *)"mode"
#define XtNnumColors			(char *)"numColors"
#define XtNcontourColors		(char *)"contourColors"
#define XtNselectDataCallback           (char *)"selectDataCallback"
#define XtNselectBarCallback           	(char *)"selectBarCallback"
#define XtNmouseOverCallback		(char *)"mouseOverCallback"
#define XtNplotBackgroundColor		(char *)"plotBackgroundColor"
#define XtNapplyAntiAlias		(char *)"applyAntiAlias"
#define XtNbarTitle			(char *)"barTitle"
#define XtNshortBarTitle		(char *)"shortBarTitle"

#define XtCDataColor 			(char *)"DataColor"
#define XtCContourLabelColor 		(char *)"ContourLabelColor"
#define XtCContourLabelSize 		(char *)"ContourLabelSize"
#define XtCMarkMax			(char *)"MarkMax"
#define XtCAutoInterval			(char *)"AutoInterval"
#define XtCMode				(char *)"Mode"
#define XtCNumColors			(char *)"NumColors"
#define XtCContourColors		(char *)"ContourColors"
#define XtCSelectDataCallback           (char *)"SelectDataCallback"
#define XtCSelectBarCallback           	(char *)"SelectBarCallback"
#define XtCMouseOverCallback		(char *)"MouseOverCallback"
#define XtCPlotBackgroundColor		(char *)"PlotBackgroundColor"
#define XtCApplyAntiAlias		(char *)"ApplyAntiAlias"
#define XtCBarTitle			(char *)"BarTitle"
#define XtCShortBarTitle		(char *)"ShortBarTitle"

#ifndef XtRDouble
#define XtRDouble	(char *)"double"
#endif

typedef struct _ConPlotClassRec	*ConPlotWidgetClass;
typedef struct _ConPlotRec	*ConPlotWidget;

extern WidgetClass		conPlotWidgetClass;

#define	CONTOURS_ONLY		0
#define	COLOR_ONLY		1
#define	CONTOURS_AND_COLOR	2

typedef struct
{
	int		entry_id;
	float		x;
	float		y;
	float		z;
	int		i;
	int		j;
	int		pos;
        int             reason; 
} ConPlotSelectCallbackStruct;

typedef struct
{
	float		x;
	float		y;
	float		z;
	int		i;
	int		j;
	int		pos;
	Boolean		selectable;
} ConPlotMotionCallbackStruct;

/* these are the reasons in ConPlotSelectCallbackStruct */
#define CONPLOT_DOWN		1
#define CONPLOT_DRAG		2
#define CONPLOT_DOWN_CTRL1	3
#define CONPLOT_DRAG_CTRL1	4
#define CONPLOT_DOWN_CTRL2	5
#define CONPLOT_DRAG_CTRL2	6
#define CONPLOT_DOWN_CTRL3	7
#define CONPLOT_DRAG_CTRL3	8
#define CONPLOT_DOWN_SHIFT1	9
#define CONPLOT_DRAG_SHIFT1	10
#define CONPLOT_DOWN_SHIFT2	11
#define CONPLOT_DRAG_SHIFT2	12
#define CONPLOT_DOWN_SHIFT3	13
#define CONPLOT_DRAG_SHIFT3	14

void ConPlotClear(ConPlotWidget w);
void ConPlotClearEntries(ConPlotWidget w);
Widget ConPlotCreate(Widget parent, String name, ArgList arglist,
		Cardinal argcount);

#endif	/* CONPLOT_H_ */
