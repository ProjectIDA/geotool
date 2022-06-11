/** \file ParticleMotion.c
 *  \brief Defines widget ParticleMotion.
 *  \author Ivan Henson
 */
/*
 * NAME
 *      ParticleMotion Widget -- widget for drawing particle motion plots.
 *
 * FILES
 *      ParticleMotion.h
 *
 * BUGS
 *
 * NOTES
 *
 * AUTHOR
 *		Ivan Henson	8/91
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <X11/Shell.h>
#include <Xm/Form.h>

#include "ParticleMotionP.h"
#include "libgplot.h"
#include "libdrawx.h"
#include "libmath.h"
#include "libstring.h"

#define Free(a) {if(a) free((void *)a); a = NULL; }

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define	offset(field)	XtOffset(ParticleMotionWidget, particle_motion.field)
static XtResource	resources[] = 
{
	{XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(font), XtRString, 
		"-adobe-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*"},
	{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
		offset(fg), XtRString, "medium blue"},
	{XtNarrowLength, XtCArrowLength, XtRInt, sizeof(int),
		offset(arrow_length), XtRString, (caddr_t)"6"},
};
#undef offset

/* private functions */

static void ClassInitialize(void);
static void Initialize(Widget req, Widget new);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static Boolean SetValues(ParticleMotionWidget cur, ParticleMotionWidget req,
			ParticleMotionWidget new);
static void Redisplay(Widget w, XEvent *event, Region region);
static void Resize(Widget widget);
static void DoLayout(ParticleMotionWidget w);
static void Destroy(Widget widget);
static void DrawData(ParticleMotionWidget w);
static void drawLabels(ParticleMotionWidget w);
static void DrawSrcRecPlane(ParticleMotionWidget w);
static void DrawOrthogonalPlane(ParticleMotionWidget w);
static void *ReallocIt(void *ptr, int nbytes);
static void CvStringToDouble(XrmValuePtr *args, Cardinal *num_args,
				XrmValuePtr fromVal, XrmValuePtr toVal);


ParticleMotionClassRec	particleMotionClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&xmPrimitiveClassRec),	/* superclass */
	"ParticleMotion",			/* class_name */
	sizeof(ParticleMotionRec),		/* widget_size */
	ClassInitialize,		/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
        NULL,                    	/* actions */
        0,          			/* num_actions */
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
        NULL,				/* tm_table */
	NULL,				/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL				/* extension */
	},
        {       /* primitive_class fields */
        (XtWidgetProc)_XtInherit,       /* Primitive border_highlight */
        (XtWidgetProc)_XtInherit,       /* Primitive border_unhighlight */
        NULL,                           /* translations */
        NULL,                           /* arm_and_activate */
        NULL,                           /* get resources */
        0,                              /* num get_resources */
        NULL,
	}
};

WidgetClass particleMotionWidgetClass = (WidgetClass)&particleMotionClassRec;

static void
ClassInitialize(void)
{
	XtAddConverter (XtRString, XtRDouble, (XtConverter)CvStringToDouble,
		(XtConvertArgList) NULL, (Cardinal) 0);
}

static void
Initialize(Widget w_req, Widget	w_new)
{
	ParticleMotionWidget new = (ParticleMotionWidget)w_new;
	ParticleMotionPart *pm = &new->particle_motion;

	if(new->core.width == 0)
		new->core.width = XWidthOfScreen(XtScreen(new))/10;
	if(new->core.height == 0)
		new->core.height = XHeightOfScreen(XtScreen(new))/5;

	/* no data displayed yet.
	 */
	pm->npts = 0;
	pm->nsegs = 0;
	pm->x = NULL;
	pm->y = NULL;
	pm->z = NULL;
	pm->rx = NULL;
	pm->ry = NULL;
	pm->rz = NULL;
	pm->segs = NULL;
	pm->pixmap = (Pixmap)NULL;
	pm->sta[0] = '\0';

	/* euler angles of the rotation to the azimuth,incidence
	 */
	pm->alpha = (-90./180.)*M_PI; /* default to backazimuth = 0. */
	pm->beta = 0.;
	pm->gamma = 0.;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
	ParticleMotionWidget w = (ParticleMotionWidget)widget;
	ParticleMotionPart *pm = &w->particle_motion;
	Pixel black, white;
	XWindowAttributes window_attributes;

	(*((particleMotionWidgetClass->core_class.superclass)->
			core_class.realize))((Widget)w,  valueMask, attrs);

       	pm->gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	pm->eraseGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

	white = StringToPixel(widget, "White");
	black = StringToPixel(widget, "Black");

	if(pm->fg == w->core.background_pixel)
	{
	    pm->fg = (w->core.background_pixel != black) ? black : white;
	}
        XSetForeground(XtDisplay(w), pm->gc, pm->fg);
        XSetBackground(XtDisplay(w), pm->gc, w->core.background_pixel);
	XSetFont(XtDisplay(w), pm->gc, pm->font->fid);

/*
	XSetLineAttributes(XtDisplay(w), pm->gc,
		0, LineOnOffDash, CapButt, JoinMiter);
	XSetDashes(XtDisplay(w), pm->gc, 0, dash_list, 2);
*/

	XSetForeground(XtDisplay(w), pm->eraseGC, w->core.background_pixel);
	XSetBackground(XtDisplay(w), pm->eraseGC, w->core.background_pixel);

	XGetWindowAttributes(XtDisplay(w), XtWindow(w), &window_attributes);
	pm->depth = window_attributes.depth;

	DoLayout(w);
}

static Boolean
SetValues(
	ParticleMotionWidget cur,
	ParticleMotionWidget req,
	ParticleMotionWidget new)
{
	ParticleMotionPart *new_pm = &new->particle_motion;
	ParticleMotionPart *cur_pm = &cur->particle_motion;
	Boolean redisplay = False;

	if (!XtIsRealized((Widget)cur))
	{
	    return(False);
	}
        if(cur_pm->font != new_pm->font)
        {
            if(new_pm->font == NULL) {
                new_pm->font = cur_pm->font;
            }
            else
            {
		XSetFont(XtDisplay(new), new_pm->gc, new_pm->font->fid);
		DrawData(new);
		redisplay = True;
	    }
	}
	if(cur_pm->fg != req->particle_motion.fg)
	{
	    XSetForeground(XtDisplay(new), new_pm->gc, new_pm->fg);
	    redisplay = True;
	}
        if(cur_pm->arrow_length != req->particle_motion.arrow_length)
	{
	    DrawData(new);
	    redisplay = True;
	}
	return (redisplay);
}

static void
Redisplay(Widget widget, XEvent	*event, Region region)
{
	ParticleMotionWidget w = (ParticleMotionWidget)widget;
	ParticleMotionPart *pm = &w->particle_motion;

	XCopyArea(XtDisplay(w), pm->pixmap, XtWindow(w), pm->gc, 0, 0,
		w->core.width, w->core.height, 0, 0);
}

static void
Resize(Widget widget)
{
	ParticleMotionWidget w = (ParticleMotionWidget)widget;

	if(!XtIsRealized(widget)) return;

	DoLayout(w);
	DrawData(w);
}

static void
DoLayout(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	int w2, width, height;
	int ascent, descent, direction;
	XCharStruct overall;

	XTextExtents(pm->font, "source->receiver plane", 10, &direction,
                        &ascent, &descent, &overall);

	height = w->core.height - (overall.ascent + overall.descent + 2);
	w2 = w->core.width/2;
	width = (w2 < height) ? w2 : height;

	pm->x0 = w->core.width/4;
	pm->y0 = height/2;
	pm->r_scale = width/2;

	pm->x1 = 3*w->core.width/4;
	pm->y1 = height/2;

	if(pm->pixmap != (Pixmap)NULL)
	{
	    XFreePixmap(XtDisplay(w), pm->pixmap);
	    pm->pixmap = (Pixmap)NULL;
	}

	/* important to create pixmap with (width+1,height+1) for calls
	 * to XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
	pm->pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
			w->core.width+1, w->core.height+1, pm->depth);
	if(pm->pixmap == (Pixmap)NULL)
	{
	    fprintf(stderr, "%s: XCreatePixmap failed.\n", w->core.name);
	}
	else
	{
	    XFillRectangle(XtDisplay(w), pm->pixmap, pm->eraseGC,
		0, 0, w->core.width, w->core.height);
	    XDrawLine(XtDisplay(w), pm->pixmap, pm->gc, w->core.width/2, 0, 
		w->core.width/2, w->core.height-1);
	    drawLabels(w);
	}
}

static void
drawLabels(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	char *left_label = "source->receiver plane";
	char *right_label = "orthogonal plane";
        int x, y;
        int ascent, descent, direction;
        XCharStruct overall;

	XTextExtents(pm->font, left_label, (int)strlen(left_label),
			&direction, &ascent, &descent, &overall);
	x = pm->x0 - overall.width/2;
	y = w->core.height-2 - overall.descent;
	XDrawString(XtDisplay(w), pm->pixmap, pm->gc, x, y,
		left_label, (int)strlen(left_label));

	XTextExtents(pm->font, right_label, (int)strlen(right_label),
			&direction, &ascent, &descent, &overall);
	x = pm->x1 - overall.width/2;
	y = w->core.height-2 - overall.descent;
	XDrawString(XtDisplay(w), pm->pixmap, pm->gc, x, y,
		right_label, (int)strlen(right_label));
}

static void
Destroy(Widget widget)
{
	ParticleMotionWidget w = (ParticleMotionWidget)widget;
	ParticleMotionPart *pm = &w->particle_motion;

	Free(pm->x);
	Free(pm->y);
	Free(pm->z);
	Free(pm->rx);
	Free(pm->ry);
	Free(pm->rz);
	Free(pm->segs);
	XFreeGC(XtDisplay(w), pm->gc);
	XFreeGC(XtDisplay(w), pm->eraseGC);

	if(pm->pixmap != (Pixmap)NULL)
	{
	    XFreePixmap(XtDisplay(w), pm->pixmap);
	    pm->pixmap = (Pixmap)NULL;
	}
}

void
ParticleMotionInput(ParticleMotionWidget w, int npts, float *x, double x_mean,
	float *y, double y_mean, float *z, double z_mean, const char *sta)
{
	ParticleMotionPart *pm = &w->particle_motion;
	int i;
	double max_amp;

	pm->npts = 0;
	pm->nsegs = 0;

	if( !(pm->x = (float *)ReallocIt(pm->x, npts*sizeof(float)) ) ||
	    !(pm->y = (float *)ReallocIt(pm->y, npts*sizeof(float)) ) ||
	    !(pm->z = (float *)ReallocIt(pm->z, npts*sizeof(float)) ) ||
	    !(pm->rx = (float *)ReallocIt(pm->rx, npts*sizeof(float)) ) ||
	    !(pm->ry = (float *)ReallocIt(pm->ry, npts*sizeof(float)) ) ||
	    !(pm->rz = (float *)ReallocIt(pm->rz, npts*sizeof(float)) ) ||
	    !(pm->segs = (XSegment *)ReallocIt(pm->segs,
			3*npts*sizeof(XSegment))) )
	{
		Free(pm->x); Free(pm->y); Free(pm->z);
		Free(pm->rx); Free(pm->ry); Free(pm->rz);
		return;
	}

	stringcpy(pm->sta, sta, sizeof(pm->sta));
	pm->npts = npts;

	max_amp = 0.;
	for(i = 0; i < npts; i++) {
	    double xm = x[i] - x_mean;
	    double ym = y[i] - y_mean;
	    double zm = z[i] - z_mean;
	    double amp = sqrt(xm*xm + ym*ym + zm*zm);
	    if(max_amp < amp) max_amp = amp;
	}
	if(max_amp != 0.) {
	    for(i = 0; i < npts; i++) {
		pm->x[i] = (x[i] - x_mean)/max_amp;
		pm->y[i] = (y[i] - y_mean)/max_amp;
		pm->z[i] = (z[i] - z_mean)/max_amp;
	    }
	}

	DrawData(w);

	XCopyArea(XtDisplay(w), pm->pixmap, XtWindow(w), pm->gc, 0, 0,
		w->core.width, w->core.height, 0, 0);
}

void
ParticleMotionAzimuth(ParticleMotionWidget w, double angle)
{
	ParticleMotionPart *pm = &w->particle_motion;

	/* angle = the horizontal angle clockwise from North,
	 *	 from the station to the source, in degrees.
 	 *
	 * rotate coordinate system so that the x-axis is pointing
	 * from source to station.
	 */

	pm->alpha = -((90. + angle)/180.)*M_PI;

	DrawData(w);

	XCopyArea(XtDisplay(w), pm->pixmap, XtWindow(w), pm->gc, 0, 0,
		w->core.width, w->core.height, 0, 0);
}

void
ParticleMotionIncidence(ParticleMotionWidget w, double angle)
{
	ParticleMotionPart *pm = &w->particle_motion;

	/* angle = the incidence angle from vertical "down", in degrees.
	 */

	pm->beta = -((90. - angle)/180.)*M_PI;

	DrawData(w);

	XCopyArea(XtDisplay(w), pm->pixmap, XtWindow(w), pm->gc, 0, 0,
		w->core.width, w->core.height, 0, 0);
}

double
ParticleMotionGetIncidence(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	return 90. + (pm->beta/M_PI)*180.;
}

double
ParticleMotionGetAzimuth(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	return -90. - pm->alpha*(180./M_PI);
}

void
ParticleMotionGetSta(ParticleMotionWidget w, char *sta)
{
	ParticleMotionPart *pm = &w->particle_motion;
	stringcpy(sta, pm->sta, sizeof(pm->sta));
}

static void
DrawData(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	int i;
	double c[3][3];

	XFillRectangle(XtDisplay(w), pm->pixmap, pm->eraseGC,
		0, 0, w->core.width, w->core.height);

	/* rotate axis so that x is pointing from source to receiver
	 */
	rotation_matrix(-pm->gamma, -pm->beta, -pm->alpha, c);

	for(i = 0; i < pm->npts; i++)
	{
	    pm->rx[i] = c[0][0]*pm->x[i] + c[0][1]*pm->y[i] + c[0][2]*pm->z[i];
	    pm->ry[i] = c[1][0]*pm->x[i] + c[1][1]*pm->y[i] + c[1][2]*pm->z[i];
	    pm->rz[i] = c[2][0]*pm->x[i] + c[2][1]*pm->y[i] + c[2][2]*pm->z[i];
	}

	XDrawLine(XtDisplay(w), pm->pixmap, pm->gc, w->core.width/2, 0, 
		w->core.width/2, w->core.height-1);

	/* plot motion in the source-receiver plane (x, z)
	 */
	DrawSrcRecPlane(w);

	/* plot motion in the plane perpendicular to the source-receiver plane
	 * (y, z)
	 */
	DrawOrthogonalPlane(w);

	drawLabels(w);
}

static void
DrawSrcRecPlane(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	int i, j, n;
	double x, y, dx, dy, norm, arrow_length, len;

	n = 0;
	for(i = 1; i < pm->npts; i++)
	{
	    for(j = i; j < pm->npts
		&& pm->rx[j] == pm->rx[i-1] && pm->rz[j] == pm->rz[i-1]; j++);
	    if(j == pm->npts) return;

	    x = pm->x0 + pm->rx[i-1]*pm->r_scale;
	    y = pm->y0 - pm->rz[i-1]*pm->r_scale; /* Y axis is top to bottom */

	    dx = pm->rx[j] - pm->rx[i-1];
	    dy = -pm->rz[j] + pm->rz[i-1];
	    i = j;

	    norm = sqrt(dx*dx + dy*dy);
	    len = norm*pm->r_scale;
	    arrow_length = (pm->arrow_length < len) ? pm->arrow_length : len;
	    if(norm != 0.) {
		dx /= norm;
		dy /= norm;
	    }

	    pm->segs[n].x1 = x;
	    pm->segs[n].y1 = y;
	    pm->segs[n].x2 = x + dx*arrow_length;
	    pm->segs[n].y2 = y + dy*arrow_length;
	    n++;
	    x = pm->segs[n-1].x2;
	    y = pm->segs[n-1].y2;
	    pm->segs[n].x1 = x;
	    pm->segs[n].y1 = y;
	    pm->segs[n].x2 = x - 4*dx + 3*dy;
	    pm->segs[n].y2 = y - 4*dy - 3*dx;
	    n++;
	    pm->segs[n].x1 = x;
	    pm->segs[n].y1 = y;
	    pm->segs[n].x2 = x - 4*dx - 3*dy;
	    pm->segs[n].y2 = y - 4*dy + 3*dx;
	    n++;
	}
	pm->nsegs = n;

	XDrawSegments(XtDisplay(w), pm->pixmap, pm->gc, pm->segs, pm->nsegs);
}

static void
DrawOrthogonalPlane(ParticleMotionWidget w)
{
	ParticleMotionPart *pm = &w->particle_motion;
	int i, j, n;
	double x, y, dx, dy, norm, arrow_length, len;

	n = 0;
	for(i = 1; i < pm->npts; i++)
	{
	    for(j = i; j < pm->npts
		&& pm->ry[j] == pm->ry[i-1] && pm->rz[j] == pm->rz[i-1]; j++);
	    if(j == pm->npts) return;

	    x = pm->x1 + pm->ry[i-1]*pm->r_scale;
	    y = pm->y1 - pm->rz[i-1]*pm->r_scale; /* Y axis is top to bottom */

	    dx =  pm->ry[j] - pm->ry[i-1];
	    dy =  -pm->rz[j] + pm->rz[i-1];
	    i = j;

	    norm = sqrt(dx*dx + dy*dy);
	    len = norm*pm->r_scale;
	    arrow_length = (pm->arrow_length < len) ? pm->arrow_length : len;
	    if(norm != 0.) {
		dx /= norm;
		dy /= norm;
	    }

	    pm->segs[n].x1 = x;
	    pm->segs[n].y1 = y;
	    pm->segs[n].x2 = x + dx*arrow_length;
	    pm->segs[n].y2 = y + dy*arrow_length;
	    n++;
	    x = pm->segs[n-1].x2;
	    y = pm->segs[n-1].y2;
	    pm->segs[n].x1 = x;
	    pm->segs[n].y1 = y;
	    pm->segs[n].x2 = x - 4*dx + 3*dy;
	    pm->segs[n].y2 = y - 4*dy - 3*dx;
	    n++;
	    pm->segs[n].x1 = x;
	    pm->segs[n].y1 = y;
	    pm->segs[n].x2 = x - 4*dx - 3*dy;
	    pm->segs[n].y2 = y - 4*dy + 3*dx;
	    n++;
	}
	pm->nsegs = n;

	XDrawSegments(XtDisplay(w), pm->pixmap, pm->gc, pm->segs, pm->nsegs);
}

Widget
ParticleMotionCreate(
	Widget		parent,
	String		name,
	ArgList		arglist,
	Cardinal	argcount)
{
	return (XtCreateWidget(name, particleMotionWidgetClass, parent, 
		arglist, argcount));
}

void
ParticleMotionGetData(ParticleMotionWidget w, int *npts, float **x, float **y,
		float **z, double *az, double *incidence, char *sta,
		int sta_size)
{
	ParticleMotionPart *pm = &w->particle_motion;

	*npts = pm->npts;
	*x = pm->x;
	*y = pm->y;
	*z = pm->z;
	*az = -90. - pm->alpha*(180./M_PI);
	*incidence = 90. + pm->beta*(180./M_PI);
	stringcpy(sta, pm->sta, sta_size);
}

static void *
ReallocIt(void *ptr, int nbytes)
{
	if(nbytes <= 0) {
	    return ptr;
	}
	else if(ptr == (void *)NULL)
        {
	    if((ptr = (void *)malloc(nbytes)) == (void *)0)
	    {
		if(errno > 0) {
		    fprintf(stderr, "malloc error: %s", strerror(errno));
		}
		else {
		    fprintf(stderr, "malloc error on %d bytes", nbytes);
		}
		return NULL;
	    }
	}
	else if((ptr = (void *)realloc(ptr, nbytes)) == (void *)0)
	{
	    if(errno > 0){
		fprintf(stderr, "malloc error: %s", strerror(errno));
	    }
	    else {
		fprintf(stderr, "malloc error on %d bytes", nbytes);
	    }
	    return NULL;
	}
	return ptr;
}

static void
CvStringToDouble(XrmValuePtr *args, Cardinal *num_args, XrmValuePtr fromVal,
		XrmValuePtr toVal)
{
        char    *s = fromVal->addr;     /* string to convert */
        static double   val;            /* returned value */
	/* need the following, even though we have stdlib.h */
        extern double atof ();

        if (s == NULL) val = 0.0;
        else val = atof (s);
        toVal->size = sizeof(val);
        toVal->addr = (XtPointer) &val;
}
