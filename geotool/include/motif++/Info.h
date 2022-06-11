#ifndef _INFO_H_
#define _INFO_H_
#include <Xm/Xm.h>

#define XtNresizeWidth		(char *)"resizeWidth"
#define XtCResizeWidth		(char *)"ResizeWidth"

/// @cond
typedef struct _InfoClassRec	*InfoWidgetClass;
typedef struct _InfoRec		*InfoWidget;

extern WidgetClass infoWidgetClass;
/// @endcond

void InfoSetText(Widget widget, char *text);
char *InfoGetText(Widget widget);

#endif
