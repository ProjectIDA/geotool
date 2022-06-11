#ifndef _TAB_H_
#define _TAB_H_
#include <Xm/Xm.h>

/// @cond
typedef struct _TabClassRec	*TabWidgetClass;
typedef struct _TabRec		*TabWidget;

extern WidgetClass	tabWidgetClass;
/// @endcond

#define XtNtabLabels			(char *)"tabLabels"
#define XtNtabFont			(char *)"tabFont"
#define XtNmargin			(char *)"margin"
#define XtNtabColor			(char *)"tabColor"
#define XtNuseTabColor			(char *)"useTabColor"
#define XtNtopTabColor			(char *)"topTabColor"
#define XtNuseTopTabColor		(char *)"useTopTabColor"
#define XtNtabCallback			(char *)"tabCallback"
#define XtNtabMenuCallback		(char *)"tabMenuCallback"
#define XtNinsensitiveTabCallback	(char *)"insensitiveTabCallback"

#define XtCTabLabels			(char *)"TabLabels"
#define XtCTabFont			(char *)"TabFont"
#define XtCTabColor			(char *)"TabColor"
#define XtCUseTabColor			(char *)"UseTabColor"
#define XtCTopTabColor			(char *)"TopTabColor"
#define XtCUseTopTabColor		(char *)"UseTopTabColor"
#define XtCTabCallback			(char *)"TabCallback"
#define XtCTabMenuCallback		(char *)"TabMenuCallback"
#define XtCInsensitiveTabCallback	(char *)"InsensitiveTabCallback"

#ifndef _Size_
#define _Size_
typedef struct {
	int	width;
	int	height;
} Size;
#endif

typedef struct
{
    char	*tab_label;
    XEvent	*event;
} TabMenuCallbackStruct;

int TabGetLabels(TabWidget w, String **tabLabels);
void TabSetLabel(TabWidget w, const char *oldLabel, const char *newLabel);
void TabSetTabLabel(TabWidget w, int i, const char *newLabel);
int TabDelete(TabWidget w, const char *name);
void TabOrder(TabWidget w, int num, String *orderedLabels);
Widget TabGetTab(TabWidget w, const char *tabLabel);
Widget TabOnTop(TabWidget w);
char *TabLabelOnTop(TabWidget w);
void TabSetOnTop(TabWidget w, const char *tabLabel);
void TabSetSensitive(TabWidget w, const char *tabLabel, Boolean state);
Size TabGetPreferredSize(TabWidget w);
void TabDeleteAllTabs(TabWidget w);
void TabUpdate(TabWidget w);
Boolean TabMapChild(TabWidget widget, Widget child);
Boolean TabUnmapChild(TabWidget widget, Widget child);
Widget TabCreate(Widget parent, const char *name, ArgList arglist,
			Cardinal argcount);
int TabNumTabs(TabWidget widget);
void TabSetAppContext(TabWidget w, XtAppContext app);
void TabSetTabColor(TabWidget w, const char *label, Pixel fg);
void TabRemoveTabColors(TabWidget w);

#endif
