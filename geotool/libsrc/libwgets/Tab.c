/** \file Tab.c
 *  \brief Defines widget Tab.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      Tab Widget -- widget for displaying tabular data.
 *
 * SYNOPSIS
 *      #include "Tab.h"
 *      Widget
 *      TabCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      Tab.h
 *
 * AUTHOR
 *      I. Henson -- June 2003
 *	Scientific Computing
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "Xm/Label.h"
#include "Xm/ArrowB.h"

#include "widget/TabP.h"
#include "libstring.h"
#include "libdrawx.h" /* for StringToPixel only */


#define offset(field)	XtOffset(TabWidget, tab.field)
static XtResource	resources[] =
{
    {XtNtitle, XtCTitle, XtRString, sizeof(String),
	offset (title), XtRString, (XtPointer)NULL},
    {XtNtabLabels, XtCTabLabels, XmRStringArray, sizeof(String *),
	offset(tab_labels), XmRImmediate, (XtPointer)NULL},
    {XtNmargin, XtCMargin, XtRInt, sizeof(int),
	offset(margin), XtRImmediate, (XtPointer)4},
    {XtNtopTabColor, XtCTopTabColor, XtRPixel, sizeof(Pixel),
	offset(top_tab_fg), XtRString, (XtPointer)"Wheat"},
    {XtNuseTopTabColor, XtCUseTopTabColor, XtRBoolean, sizeof(Boolean),
	offset(use_top_tab_color), XtRString, (XtPointer)"False"},
    {XmNshadowThickness, XmCShadowThickness, XtRDimension, sizeof(Dimension),
	offset(shadow_thickness), XtRImmediate, (XtPointer)2},
    {XtNtabFont, XtCTabFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(font), XtRImmediate,(XtPointer) NULL},
    {XtNtabCallback, XtCTabCallback, XtRCallback, sizeof(XtCallbackList),
	offset(tab_callbacks), XtRCallback, (XtPointer)NULL},
    {XtNtabMenuCallback, XtCTabMenuCallback, XtRCallback,sizeof(XtCallbackList),
	offset(tab_menu_callbacks), XtRCallback, (XtPointer)NULL},
    {XtNinsensitiveTabCallback, XtCInsensitiveTabCallback, XtRCallback,
	sizeof(XtCallbackList), offset(insensitive_tab_callbacks), XtRCallback,
	(XtPointer)NULL},
};

/* Private functions */
static Boolean CvtStringToStringArray(Display *dpy, XrmValuePtr args,
		Cardinal *num_args, XrmValuePtr from, XrmValuePtr to,
		XtPointer *data);
static void ClassInitialize(void);
static void StringArrayDestructor(XtAppContext app, XrmValuePtr to,
		XtPointer converter_data, XrmValuePtr args, Cardinal *num_args);
static void Initialize(Widget req, Widget nou);
static void Realize(Widget widget, XtValueMask *valueMask,
		XSetWindowAttributes *attrs);
static Boolean SetValues(TabWidget cur,TabWidget req,TabWidget nou);
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
		XtWidgetGeometry *reply);
static void ChangeManaged(Widget w);
static void Destroy(Widget widget);
static void Resize(Widget widget);
static void doLayout(TabWidget widget);
static void Redisplay(Widget widget, XEvent *event, Region region);
static void *MallocIt(TabWidget w, int nbytes);
static void *ReallocIt(TabWidget w, void *ptr, int nbytes);
static XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *proposed,
		XtWidgetGeometry *answer);
static void mousePressedButton1(TabWidget w, XEvent *event, String *params,
                        Cardinal *num_params);
static void mousePressedMenu(TabWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Tab_drawTitle(TabWidget nou);
static void DrawTabs(TabWidget w);
static void GetTabWidths(TabWidget w);
static void GetFont(TabWidget w);
static int UpdateTabs(TabWidget w);
static void clearTabLabels(TabWidget w);
static void setOnTop(TabWidget widget);

static void destroyTab(XtPointer client_data, XtIntervalId id);
static void tabArrowCB(Widget widget, XtPointer client, XtPointer data);

static char defTrans[] =
" <Btn1Down>:  mousePressedButton1() \n\
  <Btn3Down>:  mousePressedMenu()";

static XtActionsRec actionsList[] =
{
    {"mousePressedButton1",	(XtActionProc)mousePressedButton1},
    {"mousePressedMenu",	(XtActionProc)mousePressedMenu},
};


TabClassRec	tabClassRec = 
{
    {	/* core fields */
	(WidgetClass)(&xmManagerClassRec),	/* superclass */
	"Tab",				/* class_name */
	sizeof(TabRec),			/* widget_size */
	ClassInitialize,		/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
        actionsList,			/* actions */
        XtNumber(actionsList),		/* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
	NULLQUARK,			/* xrm_class */
	TRUE,				/* compress_motion */
	TRUE,				/* compress_exposure */
	TRUE,				/* compress_enterleave */
	TRUE,				/* visible_interest */
	Destroy,			/* destroy */
	Resize,				/* resize */
	Redisplay,			/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
        defTrans,			/* tm_table */
	query_geometry,			/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL				/* extension */
    },
    {	/* composite_class fields */
	GeometryManager,		/* geometry_manager */
	ChangeManaged,			/* change_managed */
	XtInheritInsertChild,		/* insert_child */
	XtInheritDeleteChild,		/* delete_child */
	NULL				/* extension */
    },
    {   /* constraint_class fields */
	NULL,				/* constraint resource list */
	0,				/* number of constraints in list */
	0,				/* size of constraint record */
	NULL,				/* constraint initialization */
	NULL,				/* constraint destroy proc */
	NULL,				/* constraint set_values proc */
	NULL,				/* pointer to extension record */
    },
    {   /* manager_class fields */
	XtInheritTranslations,		/* translations */
	NULL,				/* syn_resources */
	0,				/* num_syn_resources */
	NULL,				/* syn_constraint_resources */
	0,				/* num_syn_constraint_resources */
	XmInheritParentProcess,		/* parent_process */
	NULL,				/* extension */
    },
    {	/* TabClass fields */
	0,		/* empty */
    },
};

WidgetClass tabWidgetClass = (WidgetClass)&tabClassRec;

static void
ClassInitialize(void)
{
    XtSetTypeConverter(XmRString, XmRStringArray,
	(XtTypeConverter)CvtStringToStringArray,
	NULL, 0, XtCacheAll | XtCacheRefCount, StringArrayDestructor);
}

static Boolean
CvtStringToStringArray(Display *dpy, XrmValuePtr args, Cardinal *num_args,
		XrmValuePtr from, XrmValuePtr to, XtPointer *data)
{
    static String *array;
    String s = from->addr;

    if(s == NULL || *s== '\0') {
	array = NULL;
    }
    else {
	String tok, c, buf;
	int n;

	buf = strdup(s);
	array = (String *)malloc(sizeof(String));
	n = 0;
	tok = buf;
	while((c = strtok(tok, ",")) != (String)NULL) {
	    tok = (String)NULL;
	    array = (String *)realloc(array, (n+1)*sizeof(String));
	    array[n++] = strdup(stringTrim(c));
	}
	array = (String *)realloc(array, (n+1)*sizeof(String));
	array[n++] = (String)NULL;
	free(buf);
    }
    if (to->addr == NULL) to->addr = (char*)(XtPointer) &array;
    else *(String **) to->addr = array;
    to->size = sizeof(String *);

    return True;
}

/*
 * Free the string array allocated by the String to StringArray converter
 */
static void
StringArrayDestructor(
    XtAppContext app,
    XrmValuePtr to,
    XtPointer converter_data,
    XrmValuePtr args,
    Cardinal *num_args)
{
    String *array = *(String **) to->addr;
    String *entry;

    if(array == NULL) return;

    for (entry = array; *entry != NULL; entry++) XtFree((char*)(XtPointer) *entry);

    XtFree((char*)(XtPointer) array);
}

static void
Initialize(Widget w_req, Widget w_new)
{
    TabWidget req = (TabWidget)w_req;
    TabWidget nou = (TabWidget)w_new;
    TabPart *t = &nou->tab;
    int	i, n;
    XmString xm;
    Size s;

    /* no data displayed yet.
     */
    t->title = (t->title != NULL) ? strdup(t->title) : strdup("");

    xm = XmStringCreateLtoR(t->title, XmSTRING_DEFAULT_CHARSET);

    t->label =  XtVaCreateWidget("Title", xmLabelWidgetClass, (Widget)nou,
		XmNmarginHeight, 0,
		XmNshadowThickness, 0,
		XmNhighlightThickness, 0,
		XmNlabelString, xm,
		NULL);
    XmStringFree(xm);

    t->arrowLeft =  XtVaCreateWidget("arrowLeft", xmArrowButtonWidgetClass,
		(Widget)nou,
		XmNshadowThickness, 0,
		XmNarrowDirection, XmARROW_LEFT,
		NULL);
    XtAddCallback(t->arrowLeft, XmNactivateCallback, tabArrowCB,
		(XtPointer)w_new);

    t->arrowRight =  XtVaCreateWidget("arrowRight", xmArrowButtonWidgetClass,
		(Widget)nou,
		XmNshadowThickness, 0,
		XmNarrowDirection, XmARROW_RIGHT,
		NULL);
    XtAddCallback(t->arrowRight, XmNactivateCallback, tabArrowCB,
		(XtPointer)w_new);

    n = 0;
    if(req->tab.tab_labels != (String *)NULL)
    {
    	for(n = 0; req->tab.tab_labels[n] != (String)NULL; n++);
	if(!(t->tab_labels = (char **)MallocIt(nou,
			(n+1)*sizeof(String))) ) return;

    	for(i = 0; req->tab.tab_labels[i] != (String)NULL; i++)
	{
	    t->tab_labels[i] = strdup(req->tab.tab_labels[i]);
	    stringTrim(t->tab_labels[i]);
	}
    }
    else {
	if( !(t->tab_labels = (char **)MallocIt(nou, sizeof(String))) ) return;
    }
    t->tab_labels[n] = (String)NULL;

    t->num_tab_colors = 1;
    t->tab_colors = (TabColor *)MallocIt(nou, sizeof(TabColor));
    t->tab_colors[0].label = strdup("");
    t->tab_colors[0].fg = StringToPixel((Widget)nou, "Light Blue");

    t->onTop = 0;
    t->tab_widths = (int *)NULL;
    t->label_y = (int *)NULL;
    t->tab_height = 0;
    t->shift = 0;
    t->max_shift = 0;
    t->ignore_change_managed = False;

    doLayout(nou);

    s = TabGetPreferredSize(nou);
    if(nou->core.width == 0) nou->core.width = s.width;
    if(nou->core.height == 0) nou->core.height = s.height;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    TabWidget w = (TabWidget)widget;
    TabPart *t = &w->tab;

    (*((tabWidgetClass->core_class.superclass)->core_class.realize))
	((Widget)w,  valueMask, attrs);

    t->gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

    XtManageChild(t->label);

    GetFont(w);

    doLayout(w);

    clearTabLabels(w);
    DrawTabs(w);
}

static void
GetFont(TabWidget w)
{
    TabPart *t = &w->tab;
    XmFontList fontList;
    Arg args[1];

    if(XtIsRealized((Widget)w) && t->font == NULL) {
	XtSetArg(args[0], XmNfontList, &fontList);
	XtGetValues(t->label, args, 1);

	if(fontList != NULL) {
	    XmFontContext context;
	    XmStringCharSet charset=NULL;
	    XmFontListInitFontContext(&context, fontList);
	    XmFontListGetNextFont(context, &charset, &t->font);
	    if(t->font != NULL) {
		XSetFont(XtDisplay(w), t->gc, t->font->fid);
	    }
	    if(charset) free(charset);
	    XmFontListFreeFontContext(context);
	}
   }
}

static Boolean
SetValues(TabWidget cur, TabWidget req, TabWidget nou)
{
    int j, n;
    TabPart *t = &nou->tab;
    Boolean redisplay = False;

    if(cur->tab.title != NULL && req->tab.title == NULL) {
	if(cur->tab.title[0] != '\0') {
	    nou->tab.title = strdup("");
	    doLayout(nou);
	    redisplay = True;
	}
	free(cur->tab.title);
    }
    else if(cur->tab.title == NULL && req->tab.title != NULL) {
	nou->tab.title = strdup(req->tab.title);
	if(req->tab.title[0] !='\0') {
	    doLayout(nou);
	    redisplay = True;
	}
    }
    else if(cur->tab.title != NULL && req->tab.title != NULL) {
	if(strcmp(cur->tab.title, req->tab.title)) {
	    nou->tab.title = strdup(req->tab.title);
	    if( (cur->tab.title[0] != '\0' && req->tab.title[0] =='\0')
	     || (cur->tab.title[0] == '\0' && req->tab.title[0] !='\0'))
	    {
		doLayout(nou);
		redisplay = True;
	    }
	    else {
                Tab_drawTitle(nou);
	    }
	    free(cur->tab.title);
	}
    }
    if(cur->tab.tab_labels != req->tab.tab_labels)
    {
	for(j = 0; t->tab_labels[j] != NULL; j++) free(t->tab_labels[j]);
	free(t->tab_labels);

	n = 0;
	if(req->tab.tab_labels != (String *)NULL)
	{
	    for(n = 0; req->tab.tab_labels[n] != (String)NULL; n++);
	    if( !(t->tab_labels = (char **)MallocIt(nou,
			(n+1)*sizeof(String))) ) return False;

	    for(j = 0; req->tab.tab_labels[j] != (String)NULL; j++)
	    {
		t->tab_labels[j] = strdup(req->tab.tab_labels[j]);
		stringTrim(t->tab_labels[j]);
	    }
	}
	else {
	    if( !(t->tab_labels = (char **)MallocIt(nou,
			sizeof(String))) ) return False;
	}
	t->tab_labels[n] = (String)NULL;
	redisplay = True;
    }
    return redisplay;
}

static void
Tab_drawTitle(TabWidget nou)
{
	XmString xm;
	Arg args[1];

	xm = XmStringCreateLtoR(nou->tab.title, XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[0], XmNlabelString, xm);
	XtSetValues(nou->tab.label, args, 1);
	XmStringFree(xm);
}

static XtGeometryResult
query_geometry(Widget wid, XtWidgetGeometry *proposed, XtWidgetGeometry *answer)
{
    TabWidget widget = (TabWidget)wid;
    Size s;

    proposed->request_mode &= CWWidth | CWHeight;
    if(proposed->request_mode == 0) {
	return XtGeometryYes;
    }

    s = TabGetPreferredSize(widget);

    answer->width = s.width;
    answer->height = s.height;
    answer->request_mode = CWWidth | CWHeight;

    if((proposed->request_mode & CWWidth) && proposed->width != s.width)
    {
	return XtGeometryNo;
    }
    else if((proposed->request_mode & CWHeight) && proposed->height != s.height)
    {
	return XtGeometryNo;
    }
    return XtGeometryYes;
}

Size
TabGetPreferredSize(TabWidget widget)
{
    TabPart *t = &widget->tab;
    CompositePart *c = &widget->composite;
    Arg args[1];
    Boolean mapped;
    Size s;
    int i, title_height, w, h;

    GetTabWidths(widget);

    title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;
    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);
    w = h = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped)
	{
	    if(w < c->children[i]->core.width) w = c->children[i]->core.width;
	    if(h < c->children[i]->core.height) h = c->children[i]->core.height;
	}
    }

    s.width = w + 1;
    s.height = title_height + t->tab_height + h + 1;

    return s;
}

static XtGeometryResult
GeometryManager(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *reply)
{
    return XtGeometryNo;
}

Boolean
TabMapChild(TabWidget widget, Widget child)
{
    CompositePart *c = &widget->composite;
    int i;

    for(i = 3; i < (signed int)c->num_children && child != c->children[i]; i++);
    if(i == (signed int)c->num_children) {
	fprintf(stderr, "TabMapChild: child widget not found.\n");
	return False;
    }
    XtSetMappedWhenManaged(child, True);
    GetFont(widget);
    doLayout(widget);
    clearTabLabels(widget);
    DrawTabs(widget);
    return True;
}

Boolean
TabUnmapChild(TabWidget widget, Widget child)
{
    CompositePart *c = &widget->composite;
    int i;

    for(i = 3; i < (signed int)c->num_children && child != c->children[i]; i++);
    if(i == (signed int)c->num_children) {
	fprintf(stderr, "TabUnmapChild: child widget not found.\n");
	return False;
    }
    XtSetMappedWhenManaged(child, False);
    ChangeManaged((Widget)widget);
    return True;
}

static void
ChangeManaged(Widget widget)
{
    TabWidget w = (TabWidget)widget;
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    Boolean first = True;
    Boolean mapped;
    Arg args[1];
    int i, j, h, title_height;

    if(!XtIsRealized((Widget)w) || t->ignore_change_managed) return;

    t->ignore_change_managed = True;

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    t->onTop = -1;

    for(i = (signed int)c->num_children-1; i >= 3; i--)
    {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped)
	{
	    if(XtIsManaged(c->children[i]) && first) {
		title_height = (t->title != NULL && t->title[0] != '\0') ?
				t->label->core.height + 2 : 0;
		h = title_height + t->tab_height;
		XtConfigureWidget(c->children[i], 0, h, widget->core.width,
			widget->core.height-h, 0);
		t->onTop = 0;
		for(j = 3; j < i; j++) {
		    XtGetValues(c->children[j], args, 1);
		    if(!c->children[j]->core.being_destroyed && mapped) {
			t->onTop++;
		    }
		}
		first = False;
	    }
	    else if(XtIsManaged(c->children[i])) {
		XtUnmanageChild(c->children[i]);
	    }
	}
    }
    if(t->onTop == -1) {
	for(i = 3; i < (signed int)c->num_children; i++)
	{
	    XtGetValues(c->children[i], args, 1);
	    if(!c->children[i]->core.being_destroyed && mapped)
	    {
		t->onTop = 0;
		title_height = (t->title != NULL && t->title[0] != '\0') ?
				t->label->core.height + 2 : 0;
		h = title_height + t->tab_height;
		XtConfigureWidget(c->children[i], 0, h, widget->core.width,
			widget->core.height-h, 0);
		break;
	    }
	}
    }

    GetFont(w);
    doLayout(w);
    clearTabLabels(w);
    DrawTabs(w);
    t->ignore_change_managed = False;
}

static void
Destroy(Widget widget)
{
    TabWidget w = (TabWidget)widget;
    TabPart *t = &w->tab;
    int j;

    for(j = 0; t->tab_labels[j] != NULL; j++) {
	free(t->tab_labels[j]);
    }
    free(t->tab_labels);
    if(t->title) free(t->title);
    for(j = 0; j < t->num_tab_colors; j++) free(t->tab_colors[j].label);
    free(t->tab_colors);

    if(t->gc != NULL) XFreeGC(XtDisplay(w), t->gc);
}

static void
Resize(Widget widget)
{
    TabWidget w = (TabWidget)widget;
    if(!XtIsRealized((Widget)w)) return;

    doLayout(w);
}

void
TabUpdate(TabWidget widget)
{
    doLayout(widget);
    clearTabLabels(widget);
    DrawTabs(widget);
}

static void
doLayout(TabWidget widget)
{
    TabPart *t = &widget->tab;
    CompositePart *c = &widget->composite;
    int i, title_height, numTabs, x1, x2;
    Arg args[1];
    Boolean mapped;

    if(widget->core.width <= 0 || widget->core.height <= 0) return;

    GetTabWidths(widget);

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped) numTabs++;
    }

    t->ignore_change_managed = True;

    title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;
    if(title_height > 0) {
	XtConfigureWidget(t->label, 0, 0, widget->core.width,
				t->label->core.height, 0);
    }

    if(title_height && !XtIsManaged(t->label)) {
	XtManageChild(t->label);
    }
    else if(!title_height && XtIsManaged(t->label)) {
	XtUnmanageChild(t->label);
    }

    setOnTop(widget);

    if(t->tab_height <= 0) {
	t->ignore_change_managed = False;
	return;
    }

    x1 = 4;
    x2 = x1;
    for(i = 0; i < numTabs; i++) {
	x2 = x1 + t->tab_widths[i];
	x1 += t->tab_widths[i] + 3;
    }
    t->shift = 0;
    if(x2 > widget->core.width+10) {
	int x, y, h;
	if(!XtIsManaged(t->arrowLeft)) XtManageChild(t->arrowLeft);
	if(!XtIsManaged(t->arrowRight)) XtManageChild(t->arrowRight);
	x = widget->core.width - t->arrowRight->core.width;
	y = 0;
	h = t->tab_height;
	XtConfigureWidget(t->arrowRight, x, y, t->arrowRight->core.width, h, 0);
	x = widget->core.width - t->arrowRight->core.width 
			- t->arrowLeft->core.width;
	y = 0;
	h = t->tab_height;
	XtConfigureWidget(t->arrowLeft, x, y, t->arrowLeft->core.width, h, 0);
	t->max_shift = x2 - (widget->core.width - t->arrowRight->core.width
			- t->arrowLeft->core.width);
	XtSetSensitive(t->arrowRight, True);
	XtSetSensitive(t->arrowLeft, False);
    }
    else {
	t->max_shift = 0;
	if(XtIsManaged(t->arrowLeft)) XtUnmanageChild(t->arrowLeft);
	if(XtIsManaged(t->arrowRight)) XtUnmanageChild(t->arrowRight);
    }
    t->ignore_change_managed = False;
}

static void
setOnTop(TabWidget widget)
{
    TabPart *t = &widget->tab;
    CompositePart *c = &widget->composite;
    int i, n, h, title_height;
    Arg args[1];
    Boolean mapped;

    title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;
    if(title_height > 0) {
	if(XtIsRealized((Widget)widget)) {
	    XtConfigureWidget(t->label, 0, 0, widget->core.width,
				t->label->core.height, 0);
	}
    }

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);
    t->ignore_change_managed = True;
    h = title_height + t->tab_height;
    n = 0;
    for(i = 3; i < (signed int)c->num_children; i++)
    {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped)
	{
	    if(n == t->onTop) {
		if(XtIsRealized((Widget)widget)) {
		    if(widget->core.height > h) {
			XtConfigureWidget(c->children[i], 0, h,
				widget->core.width, widget->core.height-h, 0);
		    }
		}
		if(!XtIsManaged(c->children[i])) {
		    XtManageChild(c->children[i]);
		}
	    }
	    else if(XtIsManaged(c->children[i])) {
		XtUnmanageChild(c->children[i]);
	    }
	    n++;
	}
    }
    t->ignore_change_managed = False;
}

static void
tabArrowCB(Widget widget, XtPointer client_data, XtPointer data)
{
    TabWidget w = (TabWidget)client_data;
    TabPart *t = &w->tab;

    if(widget == t->arrowRight)
    {
	if(t->shift < t->max_shift) {
	    t->shift += 20;
	    if(t->shift > t->max_shift) t->shift = t->max_shift;
	    if(t->shift == t->max_shift) {
		XtSetSensitive(t->arrowRight, False);
	    }
	    XtSetSensitive(t->arrowLeft, True);
	    clearTabLabels(w);
	    DrawTabs(w);
	}
    }
    else
    {
	if(t->shift > 0) {
	    t->shift -= 20;
	    if(t->shift < 0) t->shift = 0;
	    if(t->shift == 0) {
		XtSetSensitive(t->arrowLeft, False);
	    }
	    XtSetSensitive(t->arrowRight, True);
	    clearTabLabels(w);
	    DrawTabs(w);
	}
    }
}

static void
Redisplay(Widget widget, XEvent *event, Region region)
{
    TabWidget w = (TabWidget)widget;
    DrawTabs(w);
}

static void
GetTabWidths(TabWidget w)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int direction, ascent, descent;
    int i, h, numTabs, numLabels, thick;
    XCharStruct overall;
    Arg args[1];
    Boolean mapped;

    if(t->tab_widths != NULL) {
	free(t->tab_widths);
	t->tab_widths = NULL;
    }
    if(t->label_y != NULL) {
	free(t->label_y);
	t->label_y = NULL;
    }

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped) numTabs++;
    }
    if(numTabs == 0) return;

    if( !(t->tab_widths = (int *)MallocIt(w, numTabs*sizeof(int))) ) return;
    if( !(t->label_y = (int *)MallocIt(w, numTabs*sizeof(int))) ) return;

    for(numLabels = 0; t->tab_labels[numLabels] != NULL; numLabels++);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++)
    {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped)
	{
	    if(numTabs >= numLabels) {
		if(!(t->tab_labels = (char **)ReallocIt(w, t->tab_labels,
			(numTabs+2)*sizeof(String))) ) return;

		if(c->children[i]->core.name != NULL) {
		    t->tab_labels[numTabs] =
			strdup(c->children[i]->core.name);
		}
		else {
		    t->tab_labels[numTabs] = strdup("tab");
		}
		t->tab_labels[numTabs+1] = NULL;
	    }
	    numTabs++;
	}
    }
    for(i = numTabs; i < numLabels; i++) {
	if(t->tab_labels[i] != NULL) free(t->tab_labels[i]);
	t->tab_labels[i] = NULL;
    }

    if(!XtIsRealized((Widget)w)) return;

    thick = (t->shadow_thickness >= 1) ? t->shadow_thickness : 1;

    if(t->font == NULL) {
	GetFont(w);
	if(t->font == NULL) return;
    }

    t->tab_height = 0;
    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++)
    {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped)
	{
	    XTextExtents(t->font, t->tab_labels[numTabs],
			strlen(t->tab_labels[numTabs]),
			&direction, &ascent, &descent, &overall);
	    t->tab_widths[numTabs] = overall.width + 2*thick + 6;
	    h = 2*(overall.ascent + overall.descent);
	    if(t->tab_height < h) t->tab_height = h;
	    t->label_y[numTabs] = (int)(.5*(overall.ascent - overall.descent));
	    numTabs++;
	}
    }
    t->tab_height += 2*thick;
    for(i = 0; i < numTabs; i++)
    {
	t->label_y[i] += (int)(.5*t->tab_height);
    }
}

static int
UpdateTabs(TabWidget w)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, numTabs, numLabels;
    Arg args[1];
    Boolean mapped;

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped) numTabs++;
    }
    if(numTabs == 0) return 0;

    for(numLabels = 0; t->tab_labels[numLabels] != NULL; numLabels++);

    if(numLabels != numTabs) { /* tab has been added or deleted */
	GetTabWidths(w);
    }
    return numTabs;
}

static void
DrawTabs(TabWidget w)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, j, k, n, x1, y1, x2=0, y2, title_height, numTabs;
    Dimension thick;
    Pixel top_fg, bottom_fg, fg, bg;
    Arg args[3];
    Boolean mapped;

    if(!XtIsRealized((Widget)w)) return;

    numTabs = UpdateTabs(w);

    /* display tabs */

    XtSetArg(args[0], XmNtopShadowColor, &top_fg);
    XtSetArg(args[1], XmNbottomShadowColor, &bottom_fg);
    XtSetArg(args[2], XmNforeground, &fg);
    XtGetValues(t->label, args, 3);

    XtSetArg(args[0], XmNbackground, &bg);
    XtGetValues((Widget)w, args, 1);

    thick = (t->shadow_thickness >= 1) ? t->shadow_thickness : 1;

    title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    x1 = 4 - t->shift;
    y1 = title_height + 2;
    for(i = 0; i < numTabs; i++)
    {
	x2 = x1 + t->tab_widths[i];
	y2 = y1 + t->tab_height - 2;
	if(i == t->onTop && t->use_top_tab_color) {
	    XSetForeground(XtDisplay(w), t->gc, t->top_tab_fg);
	}
	else {
	    for(k = 0; k < t->num_tab_colors; k++) {
		if(!strcmp(t->tab_colors[k].label, t->tab_labels[i])) break;
	    }
	    if(k < t->num_tab_colors) {
		XSetForeground(XtDisplay(w), t->gc, t->tab_colors[k].fg);
	    }
	    else {
		XSetForeground(XtDisplay(w), t->gc, bg);
	    }
	}
	for(j = y1+1, n=4+thick; j <= y1+4; j++, n--) {
	    XDrawLine(XtDisplay(w), XtWindow(w), t->gc, x1+n,j,x2-n,j);
	}
	for(j = y1+5; j <= y2; j++) {
	    XDrawLine(XtDisplay(w), XtWindow(w), t->gc, x1, j, x2, j);
	}

	if(i == t->onTop) {
	    XSetForeground(XtDisplay(w), t->gc, top_fg);
	}
	else {
	    XSetForeground(XtDisplay(w), t->gc, bottom_fg);
	}
	for(j = 0; j < thick; j++)
	{
	    XDrawLine(XtDisplay(w), XtWindow(w),
			t->gc, x1+j, y2-1, x1+j, y1+5-j);
	    XDrawLine(XtDisplay(w), XtWindow(w),
			t->gc, x1+j, y1+5, x1+5+j, y1);
	    XDrawLine(XtDisplay(w), XtWindow(w),
			t->gc, x1+5-j, y1+j, x2-5, y1+j);
	}
	if(i == t->onTop) {
	    XSetForeground(XtDisplay(w), t->gc, bottom_fg);
	}
	else {
	    XSetForeground(XtDisplay(w), t->gc, top_fg);
	}
	for(j = 0; j < thick; j++)
	{
	    XDrawLine(XtDisplay(w), XtWindow(w),
			t->gc, x2-5-j, y1, x2-j, y1+5);
	    XDrawLine(XtDisplay(w), XtWindow(w),
			t->gc, x2-j, y1+5, x2-j, y2);
	}

	for(j = 3, n = 0; j < (signed int)c->num_children; j++) {
	    XtGetValues(c->children[j], args, 1);
	    if(!c->children[j]->core.being_destroyed && mapped) {
		if(n == i) break;
		n++;
	    }
	}
	if(j < (signed int)c->num_children && !XtIsSensitive(c->children[j])) {
	    XSetForeground(XtDisplay(w), t->gc, top_fg);
	}
	else {
	    XSetForeground(XtDisplay(w), t->gc, fg);
	}
	XDrawString(XtDisplay(w), XtWindow(w), t->gc, x1+thick+3,
		title_height + t->label_y[i],
		t->tab_labels[i], strlen(t->tab_labels[i]));

	x1 += t->tab_widths[i] + 3;
    }
    x1 = 4 - t->shift;
    y1 = title_height + 2;
    for(i = 0; i < numTabs; i++)
    {
	x2 = x1 + t->tab_widths[i];
	y2 = y1 + t->tab_height - 2;
	if(i == t->onTop)
	{
    	    XSetForeground(XtDisplay(w), t->gc, top_fg);
	    for(j = 0; j <= thick; j++)
	    {
		XDrawLine(XtDisplay(w), XtWindow(w), t->gc,
			0, y2-j, x1+thick-1, y2-j);
		XDrawLine(XtDisplay(w), XtWindow(w), t->gc,
			x2-thick+1+j, y2-j, w->core.width-1, y2-j);
	    }
	    if(t->use_top_tab_color) {
		XSetForeground(XtDisplay(w), t->gc, t->top_tab_fg);
	    }
	    else {
		XSetForeground(XtDisplay(w), t->gc, bg);
	    }
	    for(j = 0; j <= thick; j++)
	    {
		XDrawLine(XtDisplay(w), XtWindow(w), t->gc,
			x1+thick, y2-j, x2-thick, y2-j);
	    }
	}
	x1 += t->tab_widths[i] + 3;
    }
}

void
TabDeleteAllTabs(TabWidget w)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i;
    UpdateTabs(w);
    t->ignore_change_managed = True;
    for(i = (signed int)c->num_children-1; i >= 3; i--)
    {
	if(!c->children[i]->core.being_destroyed) {
	    XtUnmanageChild(c->children[i]);
	    XtDestroyWidget(c->children[i]);
	}
    }
    t->ignore_change_managed = False;
    for(i = 0; t->tab_labels[i] != NULL; i++) {
	free(t->tab_labels[i]);
	t->tab_labels[i] = NULL;
    }
}

int
TabDelete(TabWidget w, const char *name)
{
    int i, j, n, numTabs;
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    Boolean mapped;
    Arg args[1];

    UpdateTabs(w);

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped) numTabs++;
    }

    for(i = 0; t->tab_labels[i] != NULL && strcmp(t->tab_labels[i], name); i++);
    if(t->tab_labels[i] == NULL) return 0;

    if(t->onTop > i) t->onTop--;
    if(t->onTop >= numTabs-1) t->onTop--;
    if(t->onTop < 0) t->onTop = 0;

    free(t->tab_labels[i]);
    for(j = i; t->tab_labels[j] != NULL; j++) {
	t->tab_labels[j] = t->tab_labels[j+1];
    }

    n = 0;
    for(j = 3; j < (signed int)c->num_children; j++)
    {
	XtGetValues(c->children[j], args, 1);
	if(!c->children[j]->core.being_destroyed && mapped)
	{
	    if(n == i) {
		t->ignore_change_managed = True;
		XtUnmanageChild(c->children[j]);
		XtDestroyWidget(c->children[j]);
		if(t->app_context) {
		    XtAppAddTimeOut(t->app_context, 0,
			(XtTimerCallbackProc)destroyTab, (XtPointer)w);
		}
		else {
		    XtAddTimeOut(0, (XtTimerCallbackProc)destroyTab,
			(XtPointer)w);
		}
		t->ignore_change_managed = False;
		return 1;
	    }
	    n++;
	}
    }
    return 0;
}

static void
destroyTab(XtPointer client_data, XtIntervalId id)
{
    TabWidget w = (TabWidget)client_data;
    doLayout(w);
    clearTabLabels(w);
    DrawTabs(w);
}

int
TabNumTabs(TabWidget w)
{
    return UpdateTabs(w);
}

int
TabGetLabels(TabWidget w, String **tabLabels)
{
    TabPart *t = &w->tab;
    int i, numTabs;
    String *labels = (String *)NULL;

    numTabs = UpdateTabs(w);
    if(numTabs && !(labels = (char **)MallocIt(w, numTabs*sizeof(String *))) )
    {
	return -1;
    }
    for(i = 0; i < numTabs; i++) {
	labels[i] = t->tab_labels[i];
    }
    *tabLabels = labels;
    return numTabs;
}

void
TabSetLabel(TabWidget w, const char *oldLabel, const char *newLabel)
{
    TabPart *t = &w->tab;
    int i, numTabs;

    numTabs = UpdateTabs(w);

    for(i = 0; i < numTabs; i++) {
	if(t->tab_labels[i] && !strcasecmp(oldLabel, t->tab_labels[i])) break;
    }
    if(i < numTabs)
    {
	TabSetTabLabel(w, i, newLabel);
    }
}

void
TabSetTabColor(TabWidget w, const char *label, Pixel fg)
{
    TabPart *t = &w->tab;
    int i;

    for(i=0; i < t->num_tab_colors && strcmp(t->tab_colors[i].label,label);i++);
    if(i < t->num_tab_colors) {
	t->tab_colors[i].fg = fg;
    }
    else {
	t->tab_colors = (TabColor *)ReallocIt(w, t->tab_colors,
				(t->num_tab_colors+1)*sizeof(TabColor));
	t->tab_colors[t->num_tab_colors].label = strdup(label);
	t->tab_colors[t->num_tab_colors].fg = fg;
	t->num_tab_colors++;
    }
}

void
TabRemoveTabColors(TabWidget w)
{
    TabPart *t = &w->tab;
    int i;
    free(t->tab_colors[0].label);
    t->tab_colors[0].label = strdup("");
    for(i = 1; i < t->num_tab_colors; i++) {
	free(t->tab_colors[i].label);
    }
    t->num_tab_colors = 1;
}

void
TabSetTabLabel(TabWidget w, int i, const char *newLabel)
{
    TabPart *t = &w->tab;
    int numTabs;

    numTabs = UpdateTabs(w);

    if(i >= 0 && i < numTabs)
    {
	free(t->tab_labels[i]);
	t->tab_labels[i] = strdup(newLabel);
	clearTabLabels(w);
	doLayout(w);
	clearTabLabels(w);
	DrawTabs(w);
    }
}

static void
clearTabLabels(TabWidget w)
{
    TabPart *t = &w->tab;
    int title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;
    int y1;

    if(!XtIsRealized((Widget)w)) return;

    y1 = title_height + 2;

    XClearArea(XtDisplay(w), XtWindow(w), 0, y1, w->core.width, t->tab_height,
			False);
}


Widget
TabGetTab(TabWidget w, const char *tabLabel)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, j, n, numTabs;

    numTabs = UpdateTabs(w);

    for(i = 0; i < numTabs; i++) {
	if(t->tab_labels[i] && !strcasecmp(tabLabel, t->tab_labels[i])) break;
    }
    if(i < numTabs)
    {
	for(j = 3, n = 0; j < (signed int)c->num_children; j++)
        {
	    if(!c->children[j]->core.being_destroyed)
	    {
		if(n == i) return c->children[j];
		n++;
	    }
	}
    }
    return NULL;
}

void
TabOrder(TabWidget w, int num, String *orderedNames)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, j, numTabs;
    Widget *children = NULL;
    char **labels = NULL;

    numTabs = UpdateTabs(w);

    if(numTabs == 0) return;

    if( !(labels = (char **)MallocIt(w, numTabs*sizeof(String *))) ) return;
    if( !(children = (Widget *)MallocIt(w, numTabs*sizeof(Widget))) ) return;

    for(i = 0; i < numTabs; i++) labels[i] = NULL;

    for(j = 0; j < num; j++)
    {
	for(i = 0; i < numTabs; i++) {
	    if(t->tab_labels[i] != NULL
		&& !strcmp(orderedNames[j], t->tab_labels[i])) break;
	}
	if(i < numTabs)
	{
	    labels[j] = t->tab_labels[i];
	    t->tab_labels[i] = NULL;
	    children[j] = c->children[3+i];
	}
    }
    for(i = 0; i < numTabs; i++) {
	if(labels[i] == NULL) {
	    for(j = 0; j < numTabs && t->tab_labels[j] == NULL; j++);
	    if(j < numTabs) {
		labels[i] = t->tab_labels[j];
		t->tab_labels[j] = NULL;
		children[i] = c->children[3+j];
	    }
	}
    }
    for(i = 0; i < numTabs; i++) {
	c->children[3+i] = children[i];
	t->tab_labels[i] = labels[i];
    }
    if(children != NULL) free(children);
    if(labels != NULL) free(labels);
    doLayout(w);
    clearTabLabels(w);
    DrawTabs(w);
}

Widget
TabOnTop(TabWidget w)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    Arg args[1];
    Boolean mapped;
    int i, n;

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    for(i = 3, n = 0; i < (signed int)c->num_children; i++)
    {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped)
	{
	    if(t->onTop == n) {
		return c->children[i];
	    }
	    n++;
	}
    }
    return NULL;
}

char *
TabLabelOnTop(TabWidget w)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;

    if(t->onTop >= 0 && t->onTop < (signed int)(c->num_children-3)) {
	return t->tab_labels[t->onTop];
    }
    return NULL;
}

void
TabSetOnTop(TabWidget w, const char *tabLabel)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, j, n, numTabs;
    Arg args[1];
    Boolean mapped;

    numTabs = UpdateTabs(w);

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    for(i = 0; i < numTabs; i++) {
	if(t->tab_labels[i] && !strcasecmp(tabLabel, t->tab_labels[i])) break;
    }
    if(i < numTabs)
    {
	for(j = 3, n = 0; j < (signed int)c->num_children; j++)
        {
	    XtGetValues(c->children[j], args, 1);
	    if(!c->children[j]->core.being_destroyed && mapped)
	    {
		if(n == i) {
		    t->onTop = n;
		    setOnTop(w);
		    clearTabLabels(w);
		    DrawTabs(w);
		    break;
		}
		n++;
	    }
	}
    }
    else {
	fprintf(stderr, "TabSetOnTop: tab %s not found.\n", tabLabel);
    }
}

void
TabSetSensitive(TabWidget w, const char *tabLabel, Boolean state)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, j, n, numTabs;
    Arg args[1];
    Boolean mapped;

    numTabs = UpdateTabs(w);

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    for(i = 0; i < numTabs; i++) {
	if(t->tab_labels[i] && !strcasecmp(tabLabel, t->tab_labels[i])) break;
    }
    if(i < numTabs)
    {
	for(j = 3, n = 0; j < (signed int)c->num_children; j++)
        {
	    XtGetValues(c->children[j], args, 1);
	    if(!c->children[j]->core.being_destroyed && mapped)
	    {
		if(n == i) {
		    XtSetSensitive(c->children[j], state);
		    clearTabLabels(w);
		    DrawTabs(w);
		    break;
		}
		n++;
	    }
	}
    }
}

static void *
MallocIt(TabWidget w, int nbytes)
{
    char msg[128];
    void *ptr;

    if(nbytes <= 0) return (void *)NULL;

    if((ptr = (void *)malloc(nbytes)) == (void *)0) {
	snprintf(msg, 128,
	    "Memory limitation:\nCannot allocate an additional %d bytes.",
	    nbytes);
	fprintf(stderr, "%s\n", msg);
	return (void *)NULL;
    }
    return ptr;
}

static void *
ReallocIt(TabWidget w, void *ptr, int nbytes)
{
    char msg[128];
    void *pt;

    if((pt = (void *)realloc(ptr, nbytes)) == (void *)0) {
	snprintf(msg, 128,
	    "Memory limitation:\nCannot allocate an additional %d bytes.",
	    nbytes);
	fprintf(stderr, "%s\n", msg);
	return (void *)NULL;
    }
    return pt;
}

static void
mousePressedButton1(TabWidget w, XEvent *event, String *params,
                        Cardinal *num_params)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, j, x1, y1, x2, y2, title_height, n, numTabs;
//    Dimension thick;
    Arg args[1];
    Boolean mapped;
    int cursor_x = ((XButtonEvent *)event)->x;
    int cursor_y = ((XButtonEvent *)event)->y;

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped) numTabs++;
    }
    if(numTabs == 0) return;

//    thick = (t->shadow_thickness >= 1) ? t->shadow_thickness : 1;

    title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;

    x1 = 4 - t->shift;
    y1 = title_height + 2;
    for(i = 0; i < numTabs; i++)
    {
	x2 = x1 + t->tab_widths[i];
	y2 = y1 + t->tab_height - 2;

	if(cursor_x >= x1 && cursor_x <= x2 && cursor_y >= y1 && cursor_y <= y2)
	{
	    if(t->onTop != i) {
		for(j = 3, n = 0; j < (signed int)c->num_children; j++)
		{
		    XtGetValues(c->children[j], args, 1);
		    if(!c->children[j]->core.being_destroyed && mapped) {
			if(n == i) {
			    if(!XtIsSensitive(c->children[j])) {
				XtCallCallbacks((Widget)w,
					XtNinsensitiveTabCallback,
					(XtPointer)t->tab_labels[i]);
				return;
			    }
			}
			n++;
		    }
		}
		t->onTop = i;
		XtCallCallbacks((Widget)w, XtNtabCallback,
				(XtPointer)t->tab_labels[i]);
		setOnTop(w);
		DrawTabs(w);
		return;
	    }
	}
	x1 += t->tab_widths[i] + 3;
    }
}

static void
mousePressedMenu(TabWidget w, XEvent *event, String *params,
                        Cardinal *num_params)
{
    TabPart *t = &w->tab;
    CompositePart *c = &w->composite;
    int i, x1, y1, x2, y2, title_height, numTabs;
    char *label=NULL;
    Arg args[1];
    Boolean mapped;
    TabMenuCallbackStruct mc;
    int cursor_x = ((XButtonEvent *)event)->x;
    int cursor_y = ((XButtonEvent *)event)->y;

    XtSetArg(args[0], XmNmappedWhenManaged, &mapped);

    numTabs = 0;
    for(i = 3; i < (signed int)c->num_children; i++) {
	XtGetValues(c->children[i], args, 1);
	if(!c->children[i]->core.being_destroyed && mapped) numTabs++;
    }
    if(numTabs > 0) {
	title_height = (t->title != NULL && t->title[0] != '\0') ?
			t->label->core.height + 2 : 0;

	x1 = 4 - t->shift;
	y1 = title_height + 2;
	for(i = 0; i < numTabs; i++)
	{
	    x2 = x1 + t->tab_widths[i];
	    y2 = y1 + t->tab_height - 2;

	    if(cursor_x >= x1 && cursor_x <= x2 &&
		cursor_y >= y1 && cursor_y <= y2)
	    {
		label = t->tab_labels[i];
		break;
	    }
	    x1 += t->tab_widths[i] + 3;
	}
    }

    mc.tab_label = label;
    mc.event = event;

    XtCallCallbacks((Widget)w, XtNtabMenuCallback, (XtPointer)&mc);
}

Widget
TabCreate(Widget parent, const char *name, ArgList arglist, Cardinal argcount)
{
        return (XtCreateWidget(name, tabWidgetClass, parent,
			arglist, argcount));
}

void
TabSetAppContext(TabWidget w, XtAppContext app)
{
	TabPart *t = &w->tab;
	t->app_context = app;
}
