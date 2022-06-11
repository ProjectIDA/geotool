/*****************************************************
 * Dial.h: Public header file for Dial Widget Class
 *****************************************************/
#ifndef  DIAL_H
#define  DIAL_H

#include <X11/IntrinsicP.h>

/// @cond
extern WidgetClass XsdialWidgetClass;
typedef struct _XsDialClassRec * XsDialWidgetClass;
typedef struct _XsDialRec      * XsDialWidget;
/// @endcond
/*
 * Define resource strings for the Dial widget.
 */
#define XtNselect         (char *)"select"
#define XtNmarkers        (char *)"markers"
#define XtNminimum        (char *)"minimum"
#define XtNmaximum        (char *)"maximum"
#define XtNindicatorColor (char *)"indicatorColor"
#define XtNposition       (char *)"position"
#define XtNmarkerLength   (char *)"markerLength"
#define XtNarrow1Visible  (char *)"arrow1Visible"
#define XtNarrow1Color	  (char *)"arrow1Color"
#define XtNarrow1Position (char *)"arrow1Position"
#define XtNarrow2Visible  (char *)"arrow2Visible"
#define XtNarrow2Color	  (char *)"arrow2Color"
#define XtNarrow2Position (char *)"arrow2Position"

#define XtCMarkers        (char *)"Markers"
#define XtCMin            (char *)"Min"
#define XtCMax            (char *)"Max"
#define XtCArrow1Visible  (char *)"Arrow1Visible"
#define XtCArrow1Color	  (char *)"Arrow1Color"
#define XtCArrow1Position (char *)"Arrow1Position"
#define XtCArrow2Visible  (char *)"Arrow2Visible"
#define XtCArrow2Color	  (char *)"Arrow2Color"
#define XtCArrow2Position (char *)"Arrow2Position"

typedef struct
{
	XEvent	*event;
	int	position;
} DialCallbackStruct;

#endif /* DIAL_H */
