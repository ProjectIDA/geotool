/** \file Info.c
 *  \brief Defines widget Info.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      Info Widget -- widget for a displaying single line of text
 *
 * AUTHOR
 *      I. Henson -- March 2001
 *	Multimax, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "motif++/InfoP.h"

#define offset(field)	XtOffset(InfoWidget, info.field)
static XtResource	resources[] =
{
    {XmNvalue, XmCValue, XtRString, sizeof(String),
	offset(value), XtRString, (XtPointer)""},
    {XtNresizeWidth, XtCResizeWidth, XtRBoolean, sizeof(Boolean),
	offset(resize_width), XtRString, (XtPointer)"False"},
    {XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(font), XtRString,
	(XtPointer)"-adobe-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*"},
};
#undef offset

#define Offset(field) XtOffset(InfoWidget, drawing_area.field)
static XmSyntheticResource syn_resources[] =
{
    {
	XmNmarginWidth,
	sizeof(Dimension), Offset(margin_width),
	_XmFromHorizontalPixels, _XmToHorizontalPixels
    },
    {
	XmNmarginHeight,
	sizeof(Dimension), Offset(margin_height),
	_XmFromVerticalPixels, _XmToVerticalPixels
    }
};
#undef Offset


/* From DrawingArea.c for class part. */
static Boolean ConstraintSetValues(Widget curr, Widget req, Widget new_w,
        ArgList args, Cardinal *num_args);

/* Private functions */

static void Initialize(Widget req, Widget nou);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static Boolean SetValues(InfoWidget cur, InfoWidget req, InfoWidget nou);
static void Redisplay(Widget widget, XEvent *event, Region region);
static void Destroy(Widget w);

InfoClassRec	infoClassRec = 
{
    {	/* core fields */
	(WidgetClass)(&xmDrawingAreaClassRec),	/* superclass */
	"Info",				/* class_name */
	sizeof(InfoRec),		/* widget_size */
	NULL,				/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
	NULL,				/* actions */
	0,				/* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
	NULLQUARK,			/* xrm_class */
	TRUE,				/* compress_motion */
	TRUE,				/* compress_exposure */
	TRUE,				/* compress_enterleave */
	TRUE,				/* visible_interest */
	Destroy,			/* destroy */
	XtInheritResize,		/* resize */
	Redisplay,			/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
        XmInheritTranslations,		/* tm_table */
	NULL,				/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL,				/* extension */
    },
    {   /* composite_class fields */
	XtInheritGeometryManager,	/* geometry_manager */
	XtInheritChangeManaged,		/* change_managed */
	XtInheritInsertChild,           /* insert_child */
	XtInheritDeleteChild,           /* delete_child */
	NULL,				/* extension */
    },
    {   /* constraint_class fields */
	NULL,				/* constraint resource list */
	0,				/* number of constraints in list */
	0,				/* size of constraint record */
	NULL,				/* constraint initialization */
	NULL,				/* constraint destroy proc */
	ConstraintSetValues,		/* constraint set_values proc */
	NULL,				/* pointer to extension record */
    },
    {   /* manager_class fields */
	XtInheritTranslations,		/* translations */
	syn_resources,			/* syn_resources */
	XtNumber(syn_resources), 	/* num_syn_resources */
	NULL,				/* syn_constraint_resources */
	0,				/* num_syn_constraint_resources */
	XmInheritParentProcess,		/* parent_process */
	NULL,				/* extension */
    },
    {	/* XmDrawingArea fields */
	NULL,			/* extension */
    },
    {	/* InfoClass fields */
	NULL,			/* extension */
    },
};

WidgetClass infoWidgetClass = (WidgetClass)&infoClassRec;

static void
Initialize(Widget w_req, Widget w_new)
{
    InfoWidget nou = (InfoWidget)w_new;
    InfoPart *h = &nou->info;
    int ascent, descent, direction;
    XCharStruct overall;

    /* no data displayed yet.
     */
    h->value = (h->value != NULL) ? strdup(h->value) : strdup("");
    h->gc = (GC)NULL;
    XTextExtents(h->font, "ABCD", 4, &direction, &ascent, &descent, &overall);
    nou->core.height = overall.ascent + overall.descent + 8;
    if(h->resize_width) {
	XTextExtents(h->font, h->value, strlen(h->value), &direction,
		&ascent, &descent, &overall);
	nou->core.width = (overall.width > 0) ? overall.width + 10 : 1;
    }
}

static Boolean
SetValues(InfoWidget cur, InfoWidget req, InfoWidget nou)
{
    int ascent, descent, direction;
    XCharStruct overall;

    if(nou->info.font != cur->info.font) {
	XSetFont(XtDisplay(nou), nou->info.gc, nou->info.font->fid);
	XTextExtents(nou->info.font, "ABCD", 4, &direction, &ascent, &descent,
		&overall);
	nou->core.height = overall.ascent + overall.descent + 8;

	if(nou->info.resize_width) {
	    XTextExtents(nou->info.font,nou->info.value,
			strlen(nou->info.value), &direction,
			&ascent, &descent, &overall);
	    nou->core.width = (overall.width > 0) ? overall.width + 10 : 1;
	}
    }
    return True;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    InfoWidget w = (InfoWidget)widget;
    InfoPart *h = &w->info;

    (*((infoWidgetClass->core_class.superclass)->core_class.realize))
	((Widget)w,  valueMask, attrs);

    h->gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
    XSetBackground(XtDisplay(w), h->gc, w->core.background_pixel);
    XSetFont(XtDisplay(w), h->gc, h->font->fid);
}

static void
Destroy(Widget widget)
{
    InfoWidget w = (InfoWidget)widget;
    InfoPart *h = &w->info;

    if(h->gc != NULL) XFreeGC(XtDisplay(w), h->gc);
    if(h->value) free(h->value);
}

static void
Redisplay(Widget widget, XEvent *event, Region region)
{
    InfoWidget w = (InfoWidget)widget;
    InfoPart *h = &w->info;

    if(w->core.width <= 0 || w->core.height <= 0) return;

    XSetForeground(XtDisplay(w), h->gc, w->core.background_pixel);
    XFillRectangle(XtDisplay(w), XtWindow(w), h->gc, 0, 0,
		w->core.width, w->core.height);
    if(h->value != NULL && h->value[0] != '\0')
    {
	XSetForeground(XtDisplay(w), h->gc, w->manager.foreground);
	XDrawString(XtDisplay(w), XtWindow(w), h->gc, 4,
            w->core.height-5, h->value, (int)strlen(h->value));
    }
}

void
InfoSetText(Widget widget, char *text)
{
    InfoWidget w = (InfoWidget)widget;
    InfoPart *h = &w->info;

    if(h->value) free(h->value);
    h->value = (text != NULL) ? strdup(text) : strdup("");

    if(h->resize_width) {
	int ascent, descent, direction, width;
	XCharStruct overall;
	XTextExtents(h->font, h->value, strlen(h->value), &direction,
		&ascent, &descent, &overall);
	width = (overall.width > 0) ? overall.width + 10 : 1;
	XtResizeWidget(widget, width, widget->core.height, 0);
    }
    if(XtIsRealized(widget)) {
	Redisplay((Widget)w, (XEvent *)NULL, (Region)NULL);
    }
}

char *
InfoGetText(Widget widget)
{
    InfoWidget w = (InfoWidget)widget;
    InfoPart *h = &w->info;

    return (h->value != NULL) ? strdup(h->value) : strdup("");
}

extern void _XmMoveObject(Widget w, Position x, Position y);

static Boolean
ConstraintSetValues(Widget curr, Widget req, Widget new_w,
		ArgList args, Cardinal *num_args)
{
    if (XtX(curr) != XtX(new_w) || XtY(curr) != XtY(new_w)) {
	_XmMoveObject(new_w, XtX(new_w), XtY(new_w));
	return True;
    }
    return False;
}

