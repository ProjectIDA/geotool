#ifndef _HCANVAS_H_
#define _HCANVAS_H_

/// @cond
typedef struct _HCanvasClassRec	*HCanvasWidgetClass;
typedef struct _HCanvasRec	*HCanvasWidget;

extern WidgetClass hCanvasWidgetClass;
/// @endcond

#define XtNtableWidget	(char *)"tableWidget"
#define XtNcanvasWidget	(char *)"canvasWidget"

#define XtCTableWidget	(char *)"TableWidget"
#define XtCCanvasWidget	(char *)"CanvasWidget"

#ifndef _Size_
#define _Size_
typedef struct {
    int width;
    int height;
} Size;
#endif

Size HCanvasGetPreferredSize(HCanvasWidget w);

#endif
