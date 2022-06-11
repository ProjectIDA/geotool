#ifndef _TCANVAS_H_
#define _TCANVAS_H_

#ifndef _Size_
#define _Size_
typedef struct {
    int width;
    int height;
} Size;
#endif

/// @cond
typedef struct _TCanvasClassRec	*TCanvasWidgetClass;
typedef struct _TCanvasRec	*TCanvasWidget;

extern WidgetClass tCanvasWidgetClass;
/// @endcond

#define XtNtableWidget		(char *)"tableWidget"
#define XtNverticalLines	(char *)"verticalLines"
#define XtNhorizontalLines	(char *)"horizontalLines"
#define XtNsingleSelect		(char *)"singleSelect"
#define XtNradioSelect		(char *)"radioSelect"
#define XtNcntrlSelect		(char *)"cntrlSelect"
#define XtNdragSelect		(char *)"dragSelect"

#define XtCTableWidget		(char *)"TableWidget"
#define XtCVerticalLines	(char *)"VerticalLines"
#define XtCHorizontalLines	(char *)"HorizontalLines"
#define XtCCntrlSelect		(char *)"CntrlSelect"
#define XtCDragSelect		(char *)"DragSelect"

Size TCanvasGetPreferredSize(TCanvasWidget w);
void TCanvasSelectRow(TCanvasWidget w, int row, Boolean state);
void _TCanvasSetTranslations(TCanvasWidget widget);

#endif
