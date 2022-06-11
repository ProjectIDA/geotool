/*
 * NAME
 *      DrawAxis:	axis drawing routines
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "libgplot.h"
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
#include "alloc.h"

/*
 *      -----------------------------------------------------------------
 *      plot a set of axis from x1 to x2 and from y1 to y2 and set the
 *      window scale to allow room for axis labels.
 *       	
 *      win   = input window number.
 *      x1,y1 = input minimum x and y coordinates.
 *      x2,y2 = input maximum x and y coordinates.
 *      ew    = input flag. = 1: write x-labels as longitudes.
 *      			    put e after positive x-labels, and
 *                                  put w after after negative x-labels.
 *                             = 0: write x-labels as regular coordinates.
 *      ns    = input flag. = 1: write y-labels as latitudes
 *                                  put n after positive y-labels, and
 *                                  put s after negative y-labels.
 *                             = 0: write y-labels as regular coordinates.
 *      title = input title character string of length ntl
 *      ntl   = input number of characters in the title.
 *      -----------------------------------------------------------------
 *
 *      min_xlab=2 = minimum number of labels on x-axis
 *      max_xlab=5 = maximum number of labels on x-axis
 *      min_ylab=2 = minimum number of labels on y-axis
 *      max_ylab=5 = maximum number of labels on y-axis
 *
 *      min_xsmall=1 = minimum number of x small ticmarks
 *      max_xsmall=4 = maximum number of x small ticmarks
 *      min_ysmall=1 = minimum number of y small ticmarks
 *      max_ysmall=2 = maximum number of y small ticmarks
 */

static void checkInput(AxesParm *a, double *x1, double *x2, double *y1,
			double *y2);
static void getYLabels(AxesParm *a, int xpos, double y1, double y2,
			double time_factor);
static void getXLabels(AxesParm *a, int xpos, double x1, double x2,
			double time_factor, int time_scale);
static void getMargins(AxesParm *a, DrawStruct *d, int display_axes_labels,
		int x_pos, char ticks_in, int width, int height, int *left,
		int *rite, int *top,int *bot);
static void getScaledLimits(AxesParm *a, DrawStruct *d, int left, int rite,
		int top, int bot, double x1, double x2, double y1, double y2,
		double *xmin, double *xmax, double *ymin, double *ymax);
static void getTicmarks(AxesParm *a, int xpos, int time_scale,
			double time_factor);
static void drawXAxes(AxesParm *a, DrawStruct *d, int xpos, int x_pos,
		double time_factor, int time_scale, char ticks_in,
		double x1, double x2, double y1, double y2);
static void drawYAxes(AxesParm *a, DrawStruct *d, int xpos, double time_factor,
		char ticks_in, double x1, double x2, double y1, double y2);
static void doLogYSmallTics(AxesParm *A, AxisSegs *a, DrawStruct *d,
		double ymin, double ymax, int ix1, int ix2);
static void doLogXSmallTics(AxesParm *A, AxisSegs *a, DrawStruct *d,
		double xmin, double xmax, int iy1, int iy2);
static void getWidestXWidth(AxesParm *a, int xpos, double time_factor,
		char log_x, char medium_log);
static void getWidestYWidth(AxesParm *a, char log_y);
static void getRegularXLabels(AxesParm *a, int xpos, double x1, double x2,
		double time_factor);
static void getHMS_Labels(AxesParm *a, double x1, double x2);
static void checkYCursor(AxesParm *a, char log_y, double y1, double y2);
static void getMediumLog(double x, char *lab);

#define Free(a) if(a) {free(a); a = NULL;}

void
gdrawAxis(AxesParm *a, DrawStruct *d, double  x1, double x2, int x_pos,
	double y1, double y2, int y_pos, char ticks_in,
	int display_axes_labels, int time_scale, double time_factor)
{
	int i, j, xlen, ylen, xpos, left, rite, bot, top, width, height, 
		ascent, ix1, iy1, ix2, iy2;
	double xmin, xmax, ymin, ymax;

	/* check for valid input arguments
	 */
	checkInput(a, &x1, &x2, &y1, &y2);

	/* 
	 * compute the margins required.
	 */
	/* get height of numerals */
	if(a->fontMethod != NULL)  /* X fonts */
	{
	    (*a->fontMethod)(a->font, "0123456789", &width, &height, &ascent);
	    width /= 10;
	}
	else {
	    height = 0;
	    width = 0;
	}

	xpos = x_pos;

	if(x_pos == LEFT)
	{
	    x_pos = y_pos;
	    y_pos = LEFT;
	    if(time_scale == TIME_SCALE_HMS) {
		time_scale = TIME_SCALE_SECONDS;
	    }
	    if(display_axes_labels == AXES_X) {
		display_axes_labels = AXES_Y;
	    }
	}

	/* get the unscaled window dimensions
	 */
	GetDrawArea(d, &ix1, &iy1, &ix2, &iy2);
		
	xlen = abs(ix2 - ix1);
	ylen = abs(iy2 - iy1);

	if(display_axes_labels == AXES_Y || display_axes_labels == AXES_XY ||
		display_axes_labels == AXES_NO_XY)
	{
	    /* get y-coordinates of the labels on the y-axis.
	     */
	    getYLabels(a, xpos, y1, y2, time_factor);
	}
	else {
	    a->nylab = 0;
	    a->maxcy = 0;
	}

	/* get x-coordinates of the labels on the x-axis.
	 */
	getXLabels(a, xpos, x1, x2, time_factor, time_scale);

	getMargins(a, d, display_axes_labels, x_pos, ticks_in, width, height,
			&left, &rite, &top, &bot);
	if(display_axes_labels != AXES_XY && display_axes_labels != AXES_X
	   && (a->top_title == NULL || (int)strlen(a->top_title) == 0)
	   && (a->x_axis_label == NULL || (int)strlen(a->x_axis_label) == 0))
	{
	    top = bot = 0;
	    a->top = iy2;
	    a->bottom = iy1;
	}

	/* get the scaled window limits including these margins.
	 */
	getScaledLimits(a, d, left, rite, top, bot, x1, x2, y1, y2,
			&xmin, &xmax, &ymin, &ymax);
	
	SetScale(d, xmin, ymin, xmax, ymax);

	if(time_scale != TIME_SCALE_HMS)
	{
	    /* check if there is room for all the x-axis labels */
	    while(abs(unscale_x(d, a->x_lab[1]) - unscale_x(d, a->x_lab[0]))
			< 1.5*a->maxcx && a->nxlab > 2)
	    {
		for(i = j = 0; i < a->nxlab; i += 2)
		{
		    a->x_lab[j++] = a->x_lab[i];
		}
		a->nxlab = j;
	    }
	}
	else
	{
	    i = unscale_x(d, x1 + a->time_interval) - unscale_x(d, x1);
	    i = abs(i);
	    a->maxcx += (int)(a->nxdeci*(a->maxcx/8.));
	    while(i > 0 && a->maxcx > .9*i)
	    {
		a->time_interval *= 2;
		i *= 2;
	    }
	}

	/* get the scaled ticmark lengths */
	height = (a->fontMethod != NULL) ? height : a->axis_font_height;
	a->ytic = .5*height*(xmax-xmin)/(double)xlen;
	a->xtic = .5*height*(ymax-ymin)/(double)ylen;

	getTicmarks(a, xpos, time_scale, time_factor);

	drawXAxes(a, d, xpos, x_pos, time_factor, time_scale, ticks_in,
			x1, x2, y1, y2);

	drawYAxes(a, d, xpos, time_factor, ticks_in, x1, x2, y1, y2);

	if(display_axes_labels == AXES_NO_XY)
	{
	    a->nxlab = a->nylab = 0;
	}
}

static void
drawXAxes(AxesParm *a, DrawStruct *d, int xpos, int x_pos, double time_factor,
		int time_scale, char ticks_in, double x1, double x2,
		double y1, double y2)
{
	double y_labl, y_no_labl;

	if(x_pos == BOTTOM) {
	    y_labl = y1;
	    y_no_labl = y2;
	}
	else {
	    y_labl = y2;
	    y_no_labl = y1;
	    a->xtic = -a->xtic;
	}
	if(time_scale != TIME_SCALE_HMS)
	{
	    /* draw and label the x-axis.
	     */
	    if(xpos != LEFT && time_factor > 0.) {
		gdrawXAxis(a, d, 0, y_labl, x1, x2, a->xtic, x_pos, ticks_in,
			a->ew, 1, time_factor);
	    }
	    else {
		gdrawXAxis(a, d, 0, y_labl, x1, x2, a->xtic, x_pos, ticks_in,
			a->ew, 1, 0.);
	    }
	    /* draw an x-axis without labels (font=0) at y=y2.
	     */
	    gdrawXAxis(a, d, 1, y_no_labl, x1, x2, -a->xtic, x_pos, ticks_in,
				0, 0, 0.);
	}
	else
	{
	    gdrawTimeAxis(a, d, 0, y_labl, x1, x2, a->nxdeci, a->xtic, x_pos,
			ticks_in, True);
	    gdrawTimeAxis(a, d, 1, y_no_labl, x1, x2, a->nxdeci, -a->xtic,
			x_pos, ticks_in, False);
	}
}

static void
drawYAxes(AxesParm *a, DrawStruct *d, int xpos, double time_factor,
		char ticks_in, double x1, double x2, double y1, double y2)
{
	/* 
	 * draw and label the y-axis at x=x1.
	 */
	if(xpos == LEFT && time_factor > 0.) {
	    gdrawYAxis(a, d, 2, x1, y1, y2, a->ytic, ticks_in, a->ns, 1,
			time_factor);
	}
	else {
	    gdrawYAxis(a, d, 2, x1, y1, y2, a->ytic, ticks_in, a->ns, 1, 0.);
	}
	/* 
	 * draw a y-axis without labels at x=x2.
	 */
	gdrawYAxis(a, d, 3, x2, y1, y2, -a->ytic, ticks_in, 0, 0, 0.);
}

static void
checkInput(AxesParm *a, double *x1, double *x2, double *y1, double *y2)
{
	/* 
	 * check for x1==x2, y1==y2
	 */
	if(*x1 == *x2)
	{
	    double x = .1*fabs(*x1);
	    if(x == 0.) x = .1;
	    *x1 -= x;
	    *x2 += x;
	}
	if(*y1 == *y2)
	{
	    double y = .1*fabs(*y1);
	    if(y == 0.) y = .1;
	    *y1 -= y;
	    *y2 += y;
	}

	/* 
	 * check that requested labels are within allowed bounds
	 */
	if(a->min_xlab < 2 || a->min_xlab > MAXLAB) a->min_xlab = 2;
	if(a->max_xlab < 2 || a->max_xlab > MAXLAB) a->max_xlab = MAXLAB;
	if(a->min_ylab < 2 || a->min_ylab > MAXLAB) a->min_ylab = 2;
	if(a->max_ylab < 2 || a->max_ylab > MAXLAB) a->max_ylab = MAXLAB;
	if(a->min_xsmall < a->max_xsmall && a->max_xsmall > MAXLAB) 
			a->max_xsmall = MAXLAB;
	if(a->min_ysmall < a->max_ysmall && a->max_ysmall > MAXLAB) 
			a->max_ysmall = MAXLAB;

	a->nxlab = 0;
	a->nylab = 0;
	a->nxdeci = 0;
	a->nydeci = 0;
	a->r_ylab = 0;
	a->max_xlab_height = 0;
	a->max_ylab_width = 0;
}

static void
getScaledLimits(AxesParm *a, DrawStruct *d, int left, int rite, int top,
	int bot, double x1, double x2, double y1,
	double y2, double *xmin, double *xmax, double *ymin, double *ymax)
{
	int ixmar, iymar;
	int ix1, iy1, ix2, iy2, xlen, ylen;
	double xscale, yscale, xleft, xrite, ytop, ybot;

	/* get the unscaled window dimensions */
	GetDrawArea(d, &ix1, &iy1, &ix2, &iy2);
		
	xlen = abs(ix2 - ix1);
	ylen = abs(iy2 - iy1);

	ixmar = left + rite;
	xscale = fabs(x2-x1)/(double)(xlen-ixmar);
	xleft = left*xscale;
	xrite = rite*xscale;
	if(x2 > x1)
	{
	    *xmin = x1 - xleft;
	    *xmax = x2 + xrite;
	}
	else 
	{
	    *xmin = x1 + xleft;
	    *xmax = x2 - xrite;
	}
	
	iymar = bot + top;
	yscale = fabs(y2-y1)/(double)(ylen-iymar);
	ybot = bot*yscale;
	ytop = top*yscale;
	if(y2 > y1)
	{
	    *ymin = y1 - ybot;
	    *ymax = y2 + ytop;
	}
	else 
	{
	    *ymin = y1 + ybot;
	    *ymax = y2 - ytop;
	}
	if(a->uniform_scale)
	{
	    double delx, dely;

	    delx = fabs((double)(d->ix2-d->ix1)*
				(*ymax - *ymin)/(d->iy2-d->iy1));
	    if(delx > fabs(*xmax - *xmin))
	    {
		if(*xmin > *xmax) delx = -delx;
		if(a->center) {
		    double x = .5*(*xmin + *xmax);
		    *xmin = x - .5*delx;
		    *xmax = x + .5*delx;
		}
		else {
		    *xmin = *xmax - delx;
		}
	    }
	    else
	    {
		dely = fabs((double)(d->iy2-d->iy1)*
				(*xmax - *xmin)/(d->ix2-d->ix1));
		if(*ymin > *ymax) dely = -dely;
		if(a->center) {
		    double y = .5*(*ymin + *ymax);
		    *ymin = y - .5*dely;
		    *ymax = y + .5*dely;
		}
		else {
		    *ymin = *ymax - dely;
		}
	    }
	}
}
	
static void
getYLabels(AxesParm *a, int xpos, double y1, double y2, double time_factor)
{
	int i, j, lastn=0, n, ndigit;
	double y;
	char log_y = a->log_y;

	a->y_small_log = False;
	a->y_medium_log = False;
	if(a->log_y && fabs(y2 - y1) < 1.) {
	    y1 = pow(10., y1);
	    y2 = pow(10., y2);
	    log_y = False;
	    a->y_small_log = True;
	}
	else if(a->log_y && fabs(y1) < 6. && fabs(y2) < 6.) {
	    a->y_medium_log = True;
	}

	if(xpos == LEFT && time_factor > 0.)
	{
	    nicex(y1/time_factor, y2/time_factor, a->min_ylab, a->max_ylab,
			&a->nylab, a->y_lab, &ndigit, &a->nydeci);
	    for(i = 0; i < a->nylab; i++) a->y_lab[i] *= time_factor;
	}
	else
	{
	    nicex(y1, y2, a->min_ylab, a->max_ylab, &a->nylab, a->y_lab,
			&ndigit, &a->nydeci);
	}
	if(a->nylab > 1 && a->y_lab[0] > a->y_lab[1])
	{
	    /* force increasing y_lab[] */
	    for(i = 0, j = a->nylab-1; i < a->nylab/2; i++, j--)
	    {
		y = a->y_lab[i];
		a->y_lab[i] = a->y_lab[j];
		a->y_lab[j] = y;
	    }
	}
	/* force labels to be integers
	 */
	if(a->y_label_int || log_y)
	{
	    for(i = j = 0; i < a->nylab; i++) {
		if(a->y_lab[i] >= 0.) n = (int)(a->y_lab[i]+.5);
		else n = -(int)(-a->y_lab[i]+.5);
		if(i == 0 || n != lastn) {
		    a->y_lab[j++] = n;
		    lastn = n;
		}
	    }
	    a->nylab = j;
	}
	/*
	 * get the width of the widest y-axis label.
	 */
	getWidestYWidth(a, log_y);

	if(a->check_y_cursor)
	{
	    checkYCursor(a, log_y, y1, y2);
	}

	if(a->y_small_log) {
	    for(i = j = 0; i < a->nylab; i++) {
		if(a->y_lab[i] > 0.) {
		    a->y_lab[j++] = log10(a->y_lab[i]);
		}
	    }
	    a->nylab = j;
	}
}

static void
checkYCursor(AxesParm *a, char log_y, double y1, double y2)
{
	char lab[80];
	int nydeci, width, height, ascent;
	double y;

	/* check if crosshair width is greater */
	if(log_y || a->y_label_int)
	{
	    nydeci = (int)(log10(fabs(y2)>fabs(y1) ? fabs(y2):fabs(y1))+1);
	}
	else
	{
	    y = log10(fabs(y2 - y1));
	    nydeci = (int)((y-4 > 0) ? 0 : -y+4);
	}
	if(a->fontMethod != NULL)
	{
	    ftoa(y1 + .0013456*(y2-y1), nydeci, 0, lab, 80);
	    if(!a->y_label_int && !log_y)
	    {
		strcat(lab, "0"); /* one more to make sure */
	    }
	    (*a->fontMethod)(a->font, lab, &width, &height, &ascent);
	    if(a->maxcy < width) a->maxcy = width;
	    ftoa(y2 - .0013456*(y2-y1), nydeci, 0, lab, 80);
	    (*a->fontMethod)(a->font, lab, &width, &height, &ascent);
	    if(a->maxcy < width) a->maxcy = width;
	}
	else
	{
	    ftoa(y1 + .0013456*(y2-y1), nydeci, 0, lab, 80);
	    if(!a->y_label_int && !log_y)
	    {
		strcat(lab, "0"); /* one more to make sure */
	    }
	    width = a->axis_font_width*(int)strlen(lab);
	    if(a->maxcy < width) a->maxcy = width;
	    ftoa(y2 - .0013456*(y2-y1), nydeci, 0, lab, 80);
	    width = a->axis_font_width*(int)strlen(lab);
	    if(a->maxcy < width) a->maxcy = width;
	}
}

static void
getWidestYWidth(AxesParm *a, char log_y)
{
	int i, n, width, height, ascent;
	char use_exp = False;
	double y[MAXLAB+2];
	char lab[80], fmt[10];

	for(i = 0; i < a->nylab; i++) {
	    y[i] = a->y_lab[i];
	}
	n = a->nylab;
	if(a->y_label_int || log_y) {
	    y[n++] = (int)a->ymin;
	    y[n++] = (int)(a->ymax + .5);
	}
	else {
	    double max=0.;
	    if(n > 0) max = fabs(y[0]);
	    for(i = 0; i < n; i++) if(fabs(y[i]) > max) max = fabs(y[i]);
	    stringcpy(fmt, "%.2e", sizeof(fmt));
	    if(max < 1.e-06) use_exp = True; /* force exponential format */
	    if(!log_y && (fabs(a->ymin) >= 1.e+07 || fabs(a->ymax) >= 1.e+07))
	    {
		char *c, lab2[80];
		int ndigit;
		use_exp = True;
		/* determine the digits needed for distinct labels.
		 */
		for(ndigit = 2; ndigit < 10; ndigit++) {
		    snprintf(fmt, 10, "%%.%de", ndigit);
		    c = lab;
		    for(i = 0; i < n; i++)
		    {
			snprintf(c, 80, fmt, y[i]);
			if(i > 0 && !strcmp(lab, lab2)) break;
			c = (c == lab) ? lab2 : lab;
		    }
		    if(i == n) break;
		}
	    }
	}

	for(i = a->maxcy = 0; i < n; i++)
	{
	    if(use_exp) {
		snprintf(lab, 80, fmt, y[i]);
	    }
	    else if(a->y_medium_log) {
		getMediumLog(y[i], lab);
	    }
	    else {
		ftoa(y[i], a->nydeci, !a->y_label_int, lab, 80);
	    }
	    if(!a->y_medium_log && (log_y || a->y_label_int)
			&& strchr(lab, '.') != NULL) {
		continue;
	    }
	    if(log_y && !a->y_small_log && !a->y_medium_log) strcat(lab, "10");
	    if(a->fontMethod != NULL) {
		(*a->fontMethod)(a->font, lab, &width, &height, &ascent);
	    }
	    else {
		width = a->axis_font_width*(int)strlen(lab);
	    }
	    if(a->maxcy < width) a->maxcy = width;
	}
}

static void
getXLabels(AxesParm *a, int xpos, double x1, double x2, double time_factor,
		int time_scale)
{
	if(time_scale != TIME_SCALE_HMS)
	{
	    getRegularXLabels(a, xpos, x1, x2, time_factor);
	}
	else {
	    getHMS_Labels(a, x1, x2);
	}
}

static void
getRegularXLabels(AxesParm *a, int xpos, double x1, double x2,
		double time_factor)
{
	int i, j, lastn=0, n, ndigit;
	double x;
	char log_x = a->log_x;

	a->x_small_log = False;
	a->x_medium_log = False;
	if(a->log_x && fabs(x2 - x1) < 1.) {
	    x1 = pow(10., x1);
	    x2 = pow(10., x2);
	    log_x = False;
	    a->x_small_log = True;
	}
	else if(a->log_x && fabs(x1) < 6. && fabs(x2) < 6.) {
	    a->x_medium_log = True;
	}
	/* 
	 * get x-coordinates of the labels on the x-axis.
	 */
	if(xpos != LEFT && time_factor > 0.)
	{
	    nicex(x1/time_factor, x2/time_factor, a->min_xlab, a->max_xlab,
			&a->nxlab, a->x_lab, &ndigit, &a->nxdeci);
	    if(a->nxlab >= MAXLAB) a->nxlab = MAXLAB-1;
	    if(a->nylab >= MAXLAB) a->nylab = MAXLAB-1;
	    for(i = 0; i < a->nxlab; i++) a->x_lab[i] *= time_factor;
	}
	else
	{
	    nicex(x1, x2, a->min_xlab, a->max_xlab, &a->nxlab, a->x_lab,
			&ndigit, &a->nxdeci);
	    if(a->nxlab >= MAXLAB) a->nxlab = MAXLAB-1;
	    if(a->nylab >= MAXLAB) a->nylab = MAXLAB-1;
	}
	if(a->nxlab > 1 && a->x_lab[0] > a->x_lab[1])
	{
	    /* force increasing x_lab[] */
	    for(i = 0, j = a->nxlab-1; i < a->nxlab/2; i++, j--)
	    {
		x = a->x_lab[i];
		a->x_lab[i] = a->x_lab[j];
		a->x_lab[j] = x;
	    }
	}

	/* force labels to be integers
	 */
	if(log_x)
	{
	    for(i = j = 0; i < a->nxlab; i++) {
		if(a->x_lab[i] >= 0.) n = (int)(a->x_lab[i]+.5);
		else n = -(int)(-a->x_lab[i]+.5);
		if(i == 0 || n != lastn) {
		    a->x_lab[j++] = n;
		    lastn = n;
		}
	    }
	    a->nxlab = j;
	}
	getWidestXWidth(a, xpos, time_factor, log_x, a->x_medium_log);

	if(a->x_small_log) {
	    for(i = j = 0; i < a->nxlab; i++) {
		if(a->x_lab[i] > 0.) {
		    a->x_lab[j++] = log10(a->x_lab[i]);
		}
	    }
	    a->nxlab = j;
	}
}

static void
getHMS_Labels(AxesParm *a, double x1, double x2)
{
	int i, ndigit, width, height, ascent;
	double duration;
	static int n_cuts = 30;
	static struct { double limit, interval, small; } cut_offs[] =
	{
	    {1., .2, .05}, {3., .5, .1}, {5., 1., .5}, {30., 5., 1.},
	    {60., 10., 2.}, {90., 15., 5.}, {120., 20., 10.},
	    {240., 30., 10.}, {360., 60., 20.}, {720., 120., 30.},
	    {900., 150., 30.}, {1800., 300., 60.}, {3600., 600., 120.},
	    {5400., 900., 300.}, {7800., 1200., 300.},
	    {10800., 1800., 600.}, {5*3600., 3600., 600.},
	    {25*3600., 2*3600., 1800.},		/* duration <= 24 hours */
//	    {10*3600., 2*3600., 1200.},		/* duration <= 10 hours */
//	    {20*3600., 4*3600., 3600.},		/* duration <= 20 hours */
	    {40*3600., 8*3600., 2*3600.},	/* duration <= 40 hours */
	    {5*24*3600., 24*3600., 12*3600.},	/* duration <= 5 days */
	    {10*24*3600., 2*24*3600., 24*3600.},/* duration <= 10 days */
	    {20*24*3600., 4*24*3600., 24*3600.},/* duration <= 20 days */
	    {40*24*3600., 8*24*3600., 2*24*3600.},/* duration <= 40 days */
	    {80*24*3600., 16*24*3600., 4*24*3600.},/* duration <= 80 days */
	    {160*24*3600., 32*24*3600., 8*24*3600.},/* duration <= 160 days */
	    {320*24*3600., 64*24*3600., 16*24*3600.},/* duration <= 320 days */
	    {5*365*24*3600., 365*24*3600.,31*24*3600.},/* duration <= 5 years */
	    {10*365*24*3600., 2*365*24*3600.,365*24*3600.},/* <= 10 years */
	    {20*365*24*3600., 4*365*24*3600., 365*24*3600.},/* <= 20 years */
	};

	duration = x2 - x1;
	if(duration >= 1.)
	{
	    for(i = 0; i < n_cuts; i++)
	    {
		if(duration <= cut_offs[i].limit)
		{
		    a->time_interval = cut_offs[i].interval;
		    a->small_interval = cut_offs[i].small;
		    break;
		}
	    }
	    if(i == n_cuts)
	    {
		int yr = 365*24*3600;
		a->time_interval = (int)((duration/6.)/yr)*yr;
		a->small_interval = 0.;
	    }
	    a->nxdeci = (a->time_interval > 1.) ? 0 : 1;
	}
	else
	{
	    nicex(x1, x2, a->min_xlab, a->max_xlab, &a->nxlab,
				a->x_lab, &ndigit, &a->nxdeci);
	    if(a->nxlab >= MAXLAB) a->nxlab = MAXLAB-1;
	    a->time_interval = a->x_lab[1] - a->x_lab[0];
	    a->small_interval = 0.;
	}

	if(duration < 20*3600.) {
	    if(a->fontMethod != NULL) {
		(*a->fontMethod)(a->font, "00:00:00", &width, &height, &ascent);
	    }
	    else {
		width = 8*a->axis_font_width;
	    }
	}
	else {
	    if(a->fontMethod != NULL) {
		(*a->fontMethod)(a->font, "00:00", &width, &height, &ascent);
	    }
	    else {
		width = 5*a->axis_font_width;
	    }
	}
	a->maxcx = width;
}

static void
getWidestXWidth(AxesParm *a, int xpos, double time_factor, char log_x,
		char medium_log)
{
	int i, n, ndeci, width, height, ascent;
	double x[MAXLAB+2];
	char lab[80], fmt[10];

	/* 
	 * get the width of the widest x-axis label.
	 */
	ndeci = a->nxdeci;

	for(i = 0; i < a->nxlab; i++) {
	    x[i] = a->x_lab[i];
	}
	n = a->nxlab;
	if(log_x) {
	    x[n++] = (int)a->xmin;
	    x[n++] = (int)(a->xmax + .5);
	}
	else {
	    double max=0.;
	    if(n > 0) max = fabs(x[0]);
	    for(i = 0; i < n; i++) if(fabs(x[i]) > max) max = fabs(x[i]);
	    if(max < 1.e-06) ndeci = -1; /* force exponential format */
	    stringcpy(fmt, "%.2e", sizeof(fmt));
	    if(fabs(a->xmin)>1.e+06 || fabs(a->xmax)>1.e+06) {
		char *c, lab2[80];
		int ndigit;
		/* determine the digits needed for distinct labels.
		 */
		for(ndigit = 2; ndigit < 10; ndigit++) {
		    snprintf(fmt, 10, "%%.%de", ndigit);
		    c = lab;
		    for(i = 0; i < a->nxlab; i++)
		    {
			snprintf(c, 80, fmt, a->x_lab[i]);
			if(i > 0 && !strcmp(lab, lab2)) break;
			c = (c == lab) ? lab2 : lab;
		    }
		    if(i == a->nxlab) break;
		}
	    }
	}
	for(i = a->maxcx = 0; i < n; i++)
	{
	    if(xpos != LEFT && time_factor > 0. && time_factor != 1.)
	    {
		ftoa(x[i]/time_factor, ndeci, 1, lab, 80);
	    }
	    else if(ndeci < 0) {
		snprintf(lab, 80, fmt, x[i]);
	    }
	    else if(medium_log) {
		getMediumLog(x[i], lab);
	    }
	    else {
		ftoa(x[i], ndeci, 1, lab, 80);
	    }
	    if(!medium_log && log_x && strchr(lab, '.') != NULL) {
		continue;
	    }
	    if(a->fontMethod != NULL) {
		(*a->fontMethod)(a->font, lab, &width, &height, &ascent);
	    }
	    else {
		width = a->axis_font_width*(int)strlen(lab);
	    }
	    if(a->maxcx < width) a->maxcx = width;
	}
}

static void
getMediumLog(double x, char *lab)
{
	char *loglab[] = {"0.00001", "0.0001", "0.001", "0.01", "0.1", 
			"1", "10", "100", "1000", "10000", "100000"};
	int lg = (x > 0) ? (int)(x+.5) : -(int)(-x+.5);
	lg += 5;
	lab[0] = '\0';
	if(lg >= 0 && lg < 11) {
	    strcpy(lab, loglab[lg]);
	}
}

static void
getMargins(AxesParm *a, DrawStruct *d, int display_axes_labels, int x_pos,
	char ticks_in, int width, int height, int *left, int *rite, int *top,
	int *bot)
{
	int exponent_space;
	int ix1, iy1, ix2, iy2, w, h, ascent;
	/*
	 * set the left margin to allow for maxcy length labels, plus a
	 * space, plus a ticmark with length = 4, + a little.
	 * set the right margin to allow for space for the last x-axis label,
	 * in case it is on the end of the x-axis.
	 */

	/* get the unscaled window dimensions */
	GetDrawArea(d, &ix1, &iy1, &ix2, &iy2);

	/* margins:  */
	if(a->fontMethod == NULL)
	{
	    height = a->label_font_height;
	    width = a->label_font_width;
	}
	/* space between tick labels and y-axis = .5*height
	 * width of tick labels = a->maxcy (includes crosshair width if
	 *			requested)
	 * space between tick labels and y-axis label = .5*height
	 * height of y-axis label = height
	 * space before y-axis label = height
	 */
	*left = (int)(1.0*height + a->maxcy);
	if(a->y_axis_label != NULL && (int)strlen(a->y_axis_label) > 0)
	{
	    *left += (int)(1.5*height);
	}
	if(!ticks_in && a->maxcy != 0)
	{
	    *left += (int)(.5*height);
	}
	if(a->maxcy == 0)
	{
	    *left += (int)(.5*a->maxcx);
	}
	if(display_axes_labels == AXES_XY || display_axes_labels ==AXES_X) {
	    *rite = (int)(.5*a->maxcx);
	}
	else {
	    *rite = width;
	}
	if(a->auto_x)
	{
	    a->left = ix1 + *left;
	    a->right = ix2 - *rite;
	}
	else
	{
	    *left = a->left - ix1;
	    *rite = ix2 - a->right;
	}
	if(a->top_title != NULL && (int)strlen(a->top_title) > 0)
	{
	    /* get height of title */
	    if(a->fontMethod != NULL) {
		(*a->fontMethod)(a->font, a->top_title, &w, &h, &ascent);
		*top = (int)(h*3.5);
	    }
	    else {
		*top = (int)(a->title_font_height*3.5);
	    }
	}
	else
	{
	    *top = (int)((a->fontMethod) ? 1.5*height : 2.*a->axis_font_height);
	}
	exponent_space = (a->log_x && !a->x_small_log && !a->x_medium_log)
				? height : 0;
	if(a->x_axis_label != NULL && (int)strlen(a->x_axis_label) > 0)
	{
	    /* get height of x axis label */
	    if(a->fontMethod != NULL) {
		(*a->fontMethod)(a->font, a->x_axis_label, &w, &h, &ascent);
		*bot = 2*ascent +  exponent_space + h;
	    }
	    else {
		*bot = (int)(3.5*height + exponent_space);
	    }
	}
	else {
	    *bot = (int)(1.5*height + exponent_space);
	}
	if(x_pos == TOP)
	{
	    int i = *bot;
	    *bot = *top;
	    *top = i;
	}
	if(a->auto_y)
	{
	    a->top = iy2 + *top;
	    a->bottom = iy1 - *bot;
	}
	else
	{
	    *top = a->top - iy2;
	    *bot = iy1 - a->bottom;
	}
}

static void
getTicmarks(AxesParm *a, int xpos, int time_scale, double time_factor)
{
	int i, j, n;
	double z[MAXLAB];

	if(time_scale != TIME_SCALE_HMS)
	{
	    if(a->min_xsmall < a->max_xsmall)
	    {
		/* 
		 * get number of nice x small ticmarks.
		 */
		if(xpos != LEFT && time_factor > 0.)
		{
		    nicex(a->x_lab[0]/time_factor, a->x_lab[1]/time_factor,
			a->min_xsmall+2, a->max_xsmall+2, &n, z, &i, &j);
		    if(n <= 2) a->xsmall = 0.;
		    else a->xsmall = fabs(a->x_lab[1]-a->x_lab[0])/(n-1);
		}
		else
		{
		    nicex(a->x_lab[0], a->x_lab[1], a->min_xsmall+2,
			a->max_xsmall+2, &n, z, &i, &j);
		    if(n <= 2) a->xsmall = 0.;
		    else a->xsmall = fabs(a->x_lab[1]-a->x_lab[0])/(n-1);
		}
	    }
	    else
	    {
		a->xsmall = fabs(a->x_lab[1]-a->x_lab[0])/(a->min_xsmall+1);
	    }
	}

	if(a->min_ysmall < a->max_ysmall)
	{
	    /* 
	     * get number of nice y small ticmarks.
	     */
	    if(xpos == LEFT && time_factor > 0.)
	    {
		nicex(a->y_lab[0]/time_factor, a->y_lab[1]/time_factor,
			a->min_ysmall+2, a->max_ysmall+2, &n, z, &i,&j);
		if(n <= 2) a->ysmall = 0.;
		else a->ysmall = fabs(a->y_lab[1] - a->y_lab[0])/(n-1);
	    }
	    else
	    {
		nicex(a->y_lab[0], a->y_lab[1], a->min_ysmall+2,
			a->max_ysmall+2, &n, z, &i, &j);
		if(n <= 2) a->ysmall = 0.;
		else a->ysmall = fabs(a->y_lab[1] - a->y_lab[0])/(n-1);
	    }
	}
	else a->ysmall = fabs(a->y_lab[1] - a->y_lab[0])/(a->min_ysmall + 1);
}

/*
   -----------------------------------------------------------------
   plot an x-axis at y from xmin to xmax.

   y     = input y-coordinate at which to draw the axis.
   xmin  = input minimum x-coordinate of the axis.
   xmax  = input maximum x-coordinate of the axis.
   nlab  = input number of labeled ticmarks.
   small = input spacing between small tic marks (between labeled tic marks).
   xlab  = input nlab x-coordinates of the ticmarks.
   ndec  = input number of digits to the right of the decimal point
           to print in the labels.
   xtic  = input ticmark length.
   ew    = input flag. = 1: write labels as longitudes.
                            put e after positive x labels, and
                            put w after after negative labels.
                       = 0: write labels as regular coordinates
   font  = input font number for the character labels on the axis.
   -----------------------------------------------------------------
*/
void
gdrawXAxis(AxesParm *A, DrawStruct *d, int ia, double y, double xmin,
	double xmax, double xtic, int x_pos, char ticks_in, int ew, int fnt,
	double fac)
{
	char lab[80], fmt[10];
	char use_exp;
	int i, j, ix1, ix2, iy1, iy2, iy3;
	double dif=0., x, label_top;
	int width, height, ascent, sgn;
	AxisSegs *a;

	a = &A->axis_segs[ia];
	sgn = (xtic > 0.) ? -1 : 1;
	ix1 = unscale_x(d, xmin);
	ix2 = unscale_x(d, xmax);

	iy1 = unscale_y(d, y);
	if(ticks_in)
	{
	    iy2 = unscale_y(d, y + .5*xtic);
	    iy3 = unscale_y(d, y + xtic);
	}
	else
	{
	    iy2 = unscale_y(d, y - .5*xtic);
	    iy3 = unscale_y(d, y - xtic);
	}

	if(a->segs != NULL)
	{
	    free(a->segs);
	    a->segs = NULL;
	}
	a->n_segs = 0;
	a->size_segs = 0;
	ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs, a->segs,DSegment,
		128);
	a->segs[a->n_segs].x1 = ix1;
	a->segs[a->n_segs].y1 = iy1;
	a->segs[a->n_segs].x2 = ix2;
	a->segs[a->n_segs].y2 = iy1;
	a->n_segs++;

	A->x_small_log = (fabs(xmax-xmin) < 1.) ? True : False;
	A->x_medium_log = False;
	if(A->log_x && !A->x_small_log && fabs(xmin) < 6. && fabs(xmax) < 6.) {
	    A->x_medium_log = True;
	}

	if(A->nxlab > 1 && A->xsmall > 0. && !A->log_x)
	{
	    dif = (A->x_lab[1] > A->x_lab[0]) ? A->xsmall : -A->xsmall;
	    for(j = 1; j < 100; j++)
	    {
		x = A->x_lab[0] - j*dif;
		if((x < xmin && x < xmax) || (x > xmin && x > xmax)) break;
		ix1 = unscale_x(d, x);
			
		ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		a->segs[a->n_segs].x1 = ix1;
		a->segs[a->n_segs].y1 = iy1;
		a->segs[a->n_segs].x2 = ix1;
		a->segs[a->n_segs].y2 = iy2;
		a->n_segs++;
	    }
	}
	else if(A->nxlab == 1 && A->xsmall > 0. && !A->log_x)
	{
	    for(i = -1; i <= 1; i += 2)
	    {
		for(j = 1; j < 100; j++)
		{
		    x = A->x_lab[0] + j*i*A->xsmall;
		    if((x < xmin && x < xmax) || (x > xmin && x > xmax)) break;
		    ix1 = unscale_x(d, x);
		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
			a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix1;
		    a->segs[a->n_segs].y2 = iy2;
		    a->n_segs++;
		}
	    }
	}

	label_top = (ticks_in) ? y - 1.*xtic : y - 1.5*xtic;
	if(A->nxlab > 0 && fnt != 0)
	{
	    if(A->fontMethod != NULL)
	    {
		(*A->fontMethod)(A->font, "0123456789", &width,&height,&ascent);
		A->max_xlab_height = height;
	    }
	    else
	    {
		A->max_xlab_height = A->axis_font_height;
	    }
	    x = fabs(scale_y(d, A->max_xlab_height) - scale_y(d, 0));
	    A->y_xlab = (x_pos == BOTTOM) ? unscale_y(d, label_top) :
				unscale_y(d, label_top + sgn*x);
	}
	stringcpy(fmt, "%.2e", sizeof(fmt));
	use_exp = False;
	/* force exponential format for small numbers. */
	if(!A->log_x && (fac <= 0. || fac == 1.) &&
		fabs(xmin) < 1.e-06 && fabs(xmax) < 1.e-06) use_exp = True;
	if(!A->log_x && (fac <= 0. || fac == 1.) &&
		(fabs(xmin)>=1.e+07 || fabs(xmax)>=1.e+07))
	{
	    char *c, lab2[80];
	    int ndigit;
	    use_exp = True;
	    /* determine the digits needed for distinct labels.
	     */
	    for(ndigit = 2; ndigit < 10; ndigit++) {
		snprintf(fmt, 10, "%%.%de", ndigit);
		c = lab;
		for(i = 0; i < A->nxlab; i++)
		{
		    snprintf(c, 80, fmt, A->x_lab[i]);
		    if(i > 0 && !strcmp(lab, lab2)) break;
		    c = (c == lab) ? lab2 : lab;
		}
		if(i == A->nxlab) break;
	    }
	}

	for(i = 0; i < A->nxlab; i++)
	{
	    if(fnt != 0)
	    {
		if(use_exp) {
		    snprintf(lab, 80, fmt, A->x_lab[i]);
		    if(!strncmp(lab, "-0.0e", 5)) {
			stringcpy(lab, "0", sizeof(lab));
		    }
		}
		else if(A->log_x && A->x_small_log) {
		    x = pow(10., A->x_lab[i]);
		    ftoa(x, A->nxdeci, 1, lab, 80);
		}
		else if(A->log_x && A->x_medium_log) {
		    getMediumLog(A->x_lab[i], lab);
		}
		else if(fac > 0.)
		{
		    if(ew == 1) gdrawCVlon(A->x_lab[i]/fac,A->nxdeci,lab,80);
		    else if(ew==2) 
			gdrawCVlon(A->x_lab_tran[i]/fac, A->nxdeci,lab,80);
		    else ftoa(A->x_lab[i]/fac, A->nxdeci, 1, lab, 80);
		}
		else
		{
		    if(ew == 1) gdrawCVlon(A->x_lab[i], A->nxdeci, lab, 80);
		    else if(ew == 2)
			gdrawCVlon(A->x_lab_tran[i], A->nxdeci, lab,80);
		    else ftoa(A->x_lab[i], A->nxdeci, 1, lab, 80);
		}
		Free(A->xlab[i]);
		A->xlab[i] = NULL;
		if(A->log_x && !A->x_small_log && !A->x_medium_log
			&& strstr(lab, ".") != NULL)
		{
		    continue;
		}
		A->xlab[i] = stringCopy(lab);
		if(A->fontMethod != NULL)
		{
		    (*A->fontMethod)(A->font, lab, &width, &height, &ascent);
	
		    A->xlab_width[i] = width;
		    A->xlab_ascent[i] = ascent;
		    A->x_xlab[i] = (int)(unscale_x(d, A->x_lab[i]) - .5*width);
		    if(A->log_x && !A->x_small_log && !A->x_medium_log) {
			int w, h, asc;
			(*A->fontMethod)(A->font, "10", &w, &h, &asc);
                        A->x_xlab[i] += (int)(.5*w);
                        A->x_xlab2[i] = (int)(unscale_x(d, A->x_lab[i])
					-.5*(width + w));
			A->xlab_width[i] += w;
		    }
		}
		/* this is for
		    XDrawString(XtDisplay(w), XtWindow(w), gc,
			w->c_plot.x_xlab[i],
			w->c_plot.a.y_xlab + w->c_plot.xlab_ascent[i],
			lab, (int)strlen(lab));
		*/
		A->xlab_off[i] = 0;
	    }

	    if(	(A->x_lab[i] > xmin && A->x_lab[i] < xmax) ||
	    	(A->x_lab[i] > xmax && A->x_lab[i] < xmin))
	    {
		ix1 = unscale_x(d, A->x_lab[i]);
		ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs, a->segs,
				DSegment, 128);
		a->segs[a->n_segs].x1 = ix1;
		a->segs[a->n_segs].y1 = iy1;
		a->segs[a->n_segs].x2 = ix1;
		a->segs[a->n_segs].y2 = iy3;
		a->n_segs++;
	    }

	    if(i < A->nxlab-1 && A->xsmall > 0. && !A->log_x)
	    {
		for(j = 1; j < 100; j++)
		{
		    x = A->x_lab[i] + j*dif;
		    if(fabs(A->x_lab[i+1]-x) < .5*A->xsmall) break;
		    ix1 = unscale_x(d, x);
		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix1;
		    a->segs[a->n_segs].y2 = iy2;
		    a->n_segs++;
		}
	    }
	}
	if(A->log_x && !A->x_small_log) {
	    doLogXSmallTics(A, a, d, xmin, xmax, iy1, iy2);
	}
	if(A->nxlab > 1 && A->xsmall > 0. && !A->log_x)
	{
	    for(j = 1; j < 100; j++)
	    {
		x = A->x_lab[A->nxlab-1] + j*dif;
		if((x < xmin && x < xmax) || (x > xmin && x > xmax)) break;
		ix1 = unscale_x(d, x);
		ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		a->segs[a->n_segs].x1 = ix1;
		a->segs[a->n_segs].y1 = iy1;
		a->segs[a->n_segs].x2 = ix1;
		a->segs[a->n_segs].y2 = iy2;
		a->n_segs++;
	    }
	}
}

/*
   -----------------------------------------------------------------
   plot an y-axis at x from ymin to ymax.

   x     = input x-coordinate at which to draw the axis.
   ymin  = input minimum y-coordinate of the axis.
   ymax  = input maximum y-coordinate of the axis.
   nlab  = input number of labeled ticmarks.
   small = input number of small tic marks between labeled tic marks.
   ylab  = input nlab y-coordinates of the ticmarks.
   ndec  = input number of digits to the right of the decimal point
           to print in the labels.
   ytic  = input ticmark length. can be negative.
   ns    = input flag. = 1: write labels as latitudes.
                            put n after positive y labels, and
                            put s after after negative labels.
                       = 0: write labels as regular coordinates
   font  = input font number for the character labels on the axis.
   -----------------------------------------------------------------
*/
void
gdrawYAxis(AxesParm *A, DrawStruct *d, int ia, double x, double ymin,
	double ymax, double ytic, char ticks_in, int ns, int fnt, double fac)
{
	char lab[80], fmt[10];
	char use_exp;
	int i, j, ix1, ix2, iy1, iy2, ix3;
	double dif=0., y;
	int width, height, ascent;
	AxisSegs *a;

	a = &A->axis_segs[ia];
	iy1 = unscale_y(d, ymin);
	iy2 = unscale_y(d, ymax);

	ix1 = unscale_x(d, x);
	if(ticks_in)
	{
	    ix2 = unscale_x(d, x + .5*ytic);
	    ix3 = unscale_x(d, x + ytic);
	}
	else
	{
	    ix2 = unscale_x(d, x - .5*ytic);
	    ix3 = unscale_x(d, x - ytic);
	}

	if(a->segs != NULL)
	{
	    free(a->segs);
	    a->segs = NULL;
	}
	a->n_segs = 0;
	a->size_segs = 0;
	ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs, a->segs, DSegment,
		128);
	a->segs[a->n_segs].x1 = ix1;
	a->segs[a->n_segs].y1 = iy1;
	a->segs[a->n_segs].x2 = ix1;
	a->segs[a->n_segs].y2 = iy2;
	a->n_segs++;

	A->y_small_log = (fabs(ymax-ymin) < 1.) ? True : False;
	A->y_medium_log = False;
	if(A->log_y && !A->y_small_log && fabs(ymin) < 6. && fabs(ymax) < 6.) {
	    A->y_medium_log = True;
	}

	if(A->nylab > 1 && A->ysmall > 0 && !A->y_label_int && !A->log_y)
	{
	    dif = (A->y_lab[1] > A->y_lab[0]) ? A->ysmall : -A->ysmall;
	    for(j = 1; j < 100 ; j++)
	    {
		y = A->y_lab[0] - j*dif;
		if((y < ymin && y < ymax) || (y > ymin && y > ymax)) break;
		iy1 = unscale_y(d, y);

		ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		a->segs[a->n_segs].x1 = ix1;
		a->segs[a->n_segs].y1 = iy1;
		a->segs[a->n_segs].x2 = ix2;
		a->segs[a->n_segs].y2 = iy1;
		a->n_segs++;
	    }
	}
	else if(A->nylab == 1 && A->ysmall > 0. && !A->y_label_int && !A->log_y)
	{
	    for(i = -1; i <= 1; i += 2)
	    {
		for(j = 1; j < 100; j++)
		{
		    y = A->y_lab[0] + j*i*A->ysmall;
		    if((y < ymin && y < ymax) || (y > ymin && y > ymax)) break;
		    iy1 = unscale_y(d, y);
		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix2;
		    a->segs[a->n_segs].y2 = iy1;
		    a->n_segs++;
		}
	    }
	}
	if(A->nylab > 0 && fnt != 0)
	{
	    A->r_ylab = (ticks_in) ? unscale_x(d, x - 1*ytic) :
				unscale_x(d, x - 1.5*ytic);
	    A->max_ylab_width = A->maxcy;
	    A->max_ylab_height = 0;
	}
	if(A->log_y && A->nylab > 0 && !A->y_small_log) {
	    for(i = 0; i < A->nylab; i++)
	    {
		ftoa(A->y_lab[i], A->nydeci, !A->y_label_int, lab, 80);
		if((A->y_label_int || A->log_y) && strchr(lab, '.') == NULL) {
		    break;
		}
	    }
	    if(i == A->nylab) {
		A->y_lab[0] = (int)A->y_lab[0];
		A->y_lab[1] = A->y_lab[0] + 1.;
		A->nylab = 2;
	    }
	}

	/* force exponential format for small and large numbers. */
	stringcpy(fmt, "%.2e", sizeof(fmt));
	use_exp = False;
	if(!A->log_y && (fac <= 0. || fac == 1.) &&
		fabs(ymin) < 1.e-06 && fabs(ymax) < 1.e-06) use_exp = True;
	if(!A->log_y && (fac <= 0. || fac == 1.) &&
		(fabs(ymin)>=1.e+07 || fabs(ymax)>=1.e+07))
	{
	    char *c, lab2[80];
	    int ndigit;
	    use_exp = True;
	    /* determine the digits needed for distinct labels.
	     */
	    for(ndigit = 2; ndigit < 10; ndigit++) {
		snprintf(fmt, 10, "%%.%de", ndigit);
		c = lab;
		for(i = 0; i < A->nylab; i++)
		{
		    snprintf(c, 80, fmt, A->y_lab[i]);
		    if(i > 0 && !strcmp(lab, lab2)) break;
		    c = (c == lab) ? lab2 : lab;
		}
		if(i == A->nylab) break;
	    }
	}

	for(i = 0; i < A->nylab; i++)
	{
	    if(use_exp) {
		snprintf(lab, 80, fmt, A->y_lab[i]);
		if(!strncmp(lab, "-0.0e", 5)) {
		    stringcpy(lab, "0", sizeof(lab));
		}
	    }
	    else if(A->log_y && A->y_small_log) {
		y = pow(10., A->y_lab[i]);
		ftoa(y, A->nydeci, !A->y_label_int, lab, 80);
	    }
	    else if(A->log_y && A->y_medium_log) {
		getMediumLog(A->y_lab[i], lab);
	    }
	    else if(fac > 0.)
	    {
		if(ns == 1) gdrawCVlat(A->y_lab[i]/fac, A->nydeci, lab, 80);
		else if(ns == 2)
			gdrawCVlat(A->y_lab_tran[i]/fac, A->nydeci, lab,80);
		else ftoa(A->y_lab[i]/fac, A->nydeci, !A->y_label_int, lab, 80);
	    }
	    else
	    {
		if(ns == 1) gdrawCVlat(A->y_lab[i], A->nydeci, lab, 80);
		else if(ns == 2)
			gdrawCVlat(A->y_lab_tran[i], A->nydeci, lab, 80);
		else ftoa(A->y_lab[i], A->nydeci, !A->y_label_int, lab, 80);
	    }
	    if(fnt != 0)
	    {
		Free(A->ylab[i]);
		A->ylab[i] = NULL;
	    }
	    if((A->y_label_int || A->log_y) && !A->y_small_log
		&& !A->y_medium_log && strchr(lab, '.') != NULL)
	    {
		continue;
	    }
	    if(!A->y_label_int || A->log_y)
	    {
		y = A->y_lab[i];
		if((y > ymin && y < ymax) || (y > ymax && y < ymin)) {
		    iy1 = unscale_y(d, A->y_lab[i]);
		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix3;
		    a->segs[a->n_segs].y2 = iy1;
		    a->n_segs++;
		}
	    }
	    if(fnt != 0)
	    {
		A->ylab[i] = stringCopy(lab);
		if(A->fontMethod != NULL)
		{
		    (*A->fontMethod)(A->font, lab, &width, &height, &ascent);
		    A->ylab_height[i] = height;
		    A->ylab_ascent[i] = ascent;
		    A->y_ylab[i] = (int)(unscale_y(d, A->y_lab[i]) +.5*ascent);
		    A->x_ylab[i] = A->r_ylab - width;
		    if(A->log_y && !A->y_small_log && !A->y_medium_log) {
			int w, h, asc;
			A->x_ylab2[i] = A->x_ylab[i];
			(*A->fontMethod)(A->font, "10", &w, &h, &asc);
			A->x_ylab[i] -= w;
			A->y_ylab2[i] = A->y_ylab[i] - ascent;
			width += w;
			A->ylab_height[i] += h;
			A->ylab_ascent[i] += h;
		    }
		    if(width > A->max_ylab_width)
		    {
			A->max_ylab_width = width;
		    }
		    if(A->ylab_height[i] > A->max_ylab_height) {
		        A->max_ylab_height = A->ylab_height[i];
		    }
		}
		/* this is for
		    XDrawString(XtDisplay(w), XtWindow(w), gc,
			w->c_plot.a.x_ylab[i], w->c_plot.a.y_ylab[i], lab,
			(int)strlen(lab));
		*/
		A->ylab_off[i] = 0;
	    }
	    if(i < A->nylab-1 && A->ysmall > 0. && !A->y_label_int && !A->log_y)
	    {
		for(j = 1; j < 100; j++)
		{
		    y = A->y_lab[i] + j*dif;
		    if(fabs(A->y_lab[i+1]-y) < .5*A->ysmall) break;
		    iy1 = unscale_y(d, y);
		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix2;
		    a->segs[a->n_segs].y2 = iy1;
		    a->n_segs++;
		}
	    }
	}
	if(A->log_y && !A->y_small_log) {
	    doLogYSmallTics(A, a, d, ymin, ymax, ix1, ix2);
	}

	if(A->nylab > 1 && A->ysmall > 0. && !A->y_label_int && !A->log_y)
	{
	    for(j = 1; j < 100; j++)
	    {
		y = A->y_lab[A->nylab-1] + j*dif;
		if((y < ymin && y < ymax) || (y > ymin && y > ymax)) break;
		iy1 = unscale_y(d, y);
		ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		a->segs[a->n_segs].x1 = ix1;
		a->segs[a->n_segs].y1 = iy1;
		a->segs[a->n_segs].x2 = ix2;
		a->segs[a->n_segs].y2 = iy1;
		a->n_segs++;
	    }
	}
}

static void
doLogYSmallTics(AxesParm *A, AxisSegs *a, DrawStruct *d, double ymin,
		double ymax, int ix1, int ix2)
{
	int i, j, n, iy1;
	double ylab[MAXLAB+2], y;

	if(A->nylab <= 0) return;

	n = 0;
	if(A->nylab > 1) {
	    if(A->y_lab[0] < A->y_lab[1]) {
		for(i = 0; i < A->nylab; i++) {
		    if(A->ylab[i] != NULL) ylab[n++] = A->y_lab[i];
		}
	    }
	    else {
		for(i = A->nylab-1; i >= 0; i--) {
		    if(A->ylab[i] != NULL) ylab[n++] = A->y_lab[i];
		}
	    }
	}
	if(n == 0)
	{
	    ylab[0] = (int)A->y_lab[0];
	    ylab[1] = ylab[0] + 1;
	    n = 2;
	}
	else if(n == 1) {
	    ylab[1] = ylab[0];
	    ylab[0] = ylab[1] - 1.;
	    ylab[2] = ylab[1] + 1.;
	    n = 3;
	}
	else if(n > 1) {
	    y = ylab[1] - ylab[0];
	    for(i = n; i > 0; i--) ylab[i] = ylab[i-1];
	    ylab[0] = ylab[1] - y;
	    ylab[n+1] = ylab[n] + y;
	    n += 2;
	}
	for(i = 1; i < n; i++)
	{
	    int diff = (int)(ylab[i] - ylab[i-1] + .5);
	    if(diff == 1)
	    {
		double f = pow(10., ylab[i-1]);
		for(j = 2; j <= 9; j++) {
		    y = log10(j*f);
		    if((y < ymin && y < ymax) || (y > ymin && y > ymax)) {
			   continue;
		    }
		    iy1 = unscale_y(d, y);

		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix2;
		    a->segs[a->n_segs].y2 = iy1;
		    a->n_segs++;
		}
	    }
	    else if(diff <= 10) {
		for(j = 1; j < diff; j++) {
		    y = ylab[i-1] + j;
		    if((y < ymin && y < ymax) || (y > ymin && y > ymax)) break;
		    iy1 = unscale_y(d, y);

		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix2;
		    a->segs[a->n_segs].y2 = iy1;
		    a->n_segs++;
		}
	    }
	}
}

static void
doLogXSmallTics(AxesParm *A, AxisSegs *a, DrawStruct *d, double xmin,
		double xmax, int iy1, int iy2)
{
	int i, j, n, ix1;
	double xlab[MAXLAB+2], x;

	if(A->nxlab <= 0) return;

	n = 0;
	if(A->nxlab > 1) {
	    if(A->x_lab[0] < A->x_lab[1]) {
		for(i = 0; i < A->nxlab; i++) {
		    if(A->xlab[i] != NULL) xlab[n++] = A->x_lab[i];
		}
	    }
	    else {
		for(i = A->nxlab-1; i >= 0; i--) {
		    if(A->xlab[i] != NULL) xlab[n++] = A->x_lab[i];
		}
	    }
	}
	if(n == 0)
	{
	    xlab[0] = (int)A->x_lab[0];
	    xlab[1] = xlab[0] + 1;
	    n = 2;
	}
	else if(n == 1) {
	    xlab[1] = xlab[0];
	    xlab[0] = xlab[1] - 1.;
	    xlab[2] = xlab[1] + 1.;
	    n = 3;
	}
	else if(n > 1) {
	    x = xlab[1] - xlab[0];
	    for(i = n; i > 0; i--) xlab[i] = xlab[i-1];
	    xlab[0] = xlab[1] - x;
	    xlab[n+1] = xlab[n] + x;
	    n += 2;
	}
	for(i = 1; i < n; i++)
	{
	    int diff = (int)(xlab[i] - xlab[i-1] + .5);
	    if(diff == 1)
	    {
		double f = pow(10., xlab[i-1]);
		for(j = 2; j <= 9; j++) {
		    x = log10(j*f);
		    if((x < xmin && x < xmax) || (x > xmin && x > xmax)) {
			   continue;
		    }
		    ix1 = unscale_x(d, x);

		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix1;
		    a->segs[a->n_segs].y2 = iy2;
		    a->n_segs++;
		}
	    }
	    else if(diff <= 10) {
		for(j = 1; j < diff; j++) {
		    x = xlab[i-1] + j;
		    if((x < xmin && x < xmax) || (x > xmin && x > xmax)) break;
		    ix1 = unscale_x(d, x);

		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix1;
		    a->segs[a->n_segs].y2 = iy2;
		    a->n_segs++;
		}
	    }
	}
}

void
gdrawTimeAxis(AxesParm *A, DrawStruct *d, int ia, double y, double xmin,
	double xmax, int nxdeci, double xtic, int x_pos, char ticks_in,
	char label_it)
{
	char	lab[80];
	int	i, ix1, ix2, iy1, iy2, iy3, h, decimal, p, sgn;
	double	t_line, t_line2, g, limit, t_small=0., label_top, x, duration;
	double 	t0, shift=0.;
	int	width, height, ascent;
	time_t	clock;
	char	*date;
	AxisSegs *a;

	a = &A->axis_segs[ia];
	sgn = (xtic > 0.) ? -1 : 1;
	ix1 = unscale_x(d, xmin);
	ix2 = unscale_x(d, xmax);

	iy1 = unscale_y(d, y);
	if(ticks_in)
	{
	    iy2 = unscale_y(d, y + .5*xtic);
	    iy3 = unscale_y(d, y + xtic);
	}
	else
	{
	    iy2 = unscale_y(d, y - .5*xtic);
	    iy3 = unscale_y(d, y - xtic);
	}

	if(a->segs != NULL)
	{
	    free(a->segs);
	    a->segs = NULL;
	}
	a->n_segs = 0;
	a->size_segs = 0;
	ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs, a->segs, DSegment,
		128);
	a->segs[a->n_segs].x1 = ix1;
	a->segs[a->n_segs].y1 = iy1;
	a->segs[a->n_segs].x2 = ix2;
	a->segs[a->n_segs].y2 = iy1;
	a->n_segs++;

	duration = xmax - xmin;

	if(xmax > xmin)
	{
	    limit = xmax;
	    if(duration < 365*24*3600.) {
		g = ceil(xmin/A->time_interval);
		shift = 0.;
	    }
	    else { /* duration > a year, start on Jan 1 */
		DateTime dt;
		timeEpochToDate(xmin, &dt);
		dt.month = 1;
		dt.day = 1;
		dt.hour = 0;
		dt.minute = 0;
		dt.second = 0.;
		t0 = timeDateToEpoch(&dt);
		if(t0 < xmin) {
		    dt.year++;
		    t0 = timeDateToEpoch(&dt);
		}

		g = ceil(t0/A->time_interval);
		shift = t0 - g*A->time_interval;
	    }
	}
	else
	{
	    limit = xmin;
	    g = ceil(xmax/A->time_interval);
	}

	label_top = (ticks_in) ? y - 1.*xtic : y - 1.5*xtic;
	if(label_it)
	{
	    A->nxlab = 0;
	    A->max_xlab_height = 0;
	}
	if(A->small_interval > 0.)
	{
	    t_small = shift + (g - 1.)*A->time_interval;
	    if(xmax > xmin)
	    {
		while(t_small < xmin) t_small += A->small_interval;
	    }
	    else
	    {
		while(t_small < xmax) t_small += A->small_interval;
	    }
	    t_small -= A->small_interval;
	}

	for(i=0; i<MAXLAB && (t_line=shift+g*A->time_interval) <=limit; i++,g++)
	{
	    if(A->small_interval > 0.)
	    {
		for(t_small += A->small_interval; t_small < t_line;
			t_small += A->small_interval)
		{
		    ix1 = unscale_x(d, t_small);
	
		    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs, a->segs,
				 DSegment, 128);
		    a->segs[a->n_segs].x1 = ix1;
		    a->segs[a->n_segs].y1 = iy1;
		    a->segs[a->n_segs].x2 = ix1;
		    a->segs[a->n_segs].y2 = iy2;
		    a->n_segs++;
		}
	    }
		
	    ix1 = unscale_x(d, t_line);

	    ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
			a->segs, DSegment, 128);
	    a->segs[a->n_segs].x1 = ix1;
	    a->segs[a->n_segs].y1 = iy1;
	    a->segs[a->n_segs].x2 = ix1;
	    a->segs[a->n_segs].y2 = iy3;
	    a->n_segs++;

	    if(label_it)
	    {
		/* print time as either hh:mm:ss.ss or yr/mo/da
		 */
		if(duration <= 40*3600) /* <= 40 hours print hr:mm:ss format */
		{
		    char buf[50];
		    struct tm result;
		    /*
		     * date is a 26 character string:
		     * Sun Sep 16 01:03:52 1973\n\0
		     */
		    p = (int)(pow(10., (double)nxdeci));
		    t_line2 = floor(t_line*p+.5);
		    clock = (time_t)(t_line2/p);
		    clock = (t_line2 < 0.0 && clock > (time_t)0) ? 
				(clock - 1) : clock;
		    date = asctime_r(gmtime_r(&clock, &result), buf);
		    if(duration < 20*3600) { // print hr:mm:ss
			strncpy(lab, date+11, 8);
			lab[8] = '\0';
			decimal = (int)(t_line2 - p*floor(t_line));
			if(decimal != 0) // print  hr:mm:ss.ss
			{
			    if(lab[6] == '0')
			    {
				lab[6] = lab[7];
				snprintf(lab+7, 80, ".%s",
					gdrawFracSec(nxdeci,decimal));
			    }
			    else 
			    {
				snprintf(lab+8, 80, ".%s",
					gdrawFracSec(nxdeci,decimal));
			    }
			}
		    }
		    else { // print hr:mm
			strncpy(lab, date+11, 5);
			lab[5] = '\0';
		    }
		}
		else {	/* > 40 hours print yr/mo/day format */
		    DateTime dt;
		    int yr;
		    timeEpochToDate(t_line, &dt);
		    if(duration > 365*24*3600.) { /* force Jan 1 date */
			if(dt.month > 6)  dt.year++;
			dt.month = 1;
			dt.day = 1;
		    }
		    yr = dt.year - (dt.year/100)*100;
		    snprintf(lab, 80, "%02d/%02d/%02d", yr, dt.month, dt.day);
		}
		A->x_lab[A->nxlab] = t_line;

		Free(A->xlab[A->nxlab]);
		A->xlab[A->nxlab] = stringCopy(lab);
		A->xlab_off[A->nxlab] = 0;
		A->nxlab++;
	    }
	}
	if(label_it)
	{
	    for(i = A->nxlab-1; i >= 0; i--)
	    {
		if(A->fontMethod != NULL)
		{
		    (*A->fontMethod)(A->font, A->xlab[i], &width, &height,
					&ascent);
		    h = height;
		}
		else
		{
		    h = A->axis_font_height;
		    width = (int)strlen(A->xlab[i])*A->axis_font_width;
		    ascent = 0;
		}

		A->xlab_width[i] = width;
		A->xlab_ascent[i] = ascent;
		A->x_xlab[i] = (int)(unscale_x(d, A->x_lab[i]) - .5*width);

		if(h > A->max_xlab_height)
		{
		    A->max_xlab_height = h;
		}
	    }
	    x = fabs(scale_y(d, A->max_xlab_height) - scale_y(d, 0));
	    A->y_xlab = (x_pos == BOTTOM) ? unscale_y(d, label_top) :
			unscale_y(d, label_top + sgn*x);
	}
	if(A->small_interval > 0.)
	{
	    for(t_small += A->small_interval; t_small < limit;
		t_small += A->small_interval)
	    {
		ix1 = unscale_x(d, t_small);
		ALLOC((a->n_segs+1)*sizeof(DSegment), a->size_segs,
				a->segs, DSegment, 128);
		a->segs[a->n_segs].x1 = ix1;
		a->segs[a->n_segs].y1 = iy1;
		a->segs[a->n_segs].x2 = ix1;
		a->segs[a->n_segs].y2 = iy2;
		a->n_segs++;
	    }
	}
}

char *
gdrawFracSec(int n, int val)
{
        static char str[16];
        char spec[8];
 
        if (n == 1)
                stringcpy(spec, "%01d", sizeof(spec));
        else if (n == 2)
                stringcpy(spec, "%02d", sizeof(spec));
        else if (n == 3)
                stringcpy(spec, "%03d", sizeof(spec));
        else if (n == 4)
                stringcpy(spec, "%04d", sizeof(spec));
        else if (n == 5)
                stringcpy(spec, "%05d", sizeof(spec));
        else if (n == 6)
                stringcpy(spec, "%06d", sizeof(spec));
        else if (n == 7)
                stringcpy(spec, "%07d", sizeof(spec));
        else if (n == 8)
                stringcpy(spec, "%08d", sizeof(spec));
        else
                stringcpy(spec, "%d", sizeof(spec));
 
        if (val >= (int) pow(10., (float)n))
        {
                snprintf(str, sizeof(str), spec, 0);
        }
        else
        {
                snprintf(str, sizeof(str), spec, val);
        }
        return(str);
}
 


#define mod(i,j) (i-((i)/(j))*(j))

/*
   -----------------------------------------------------------------
   convert an x-coordinate value (=0. at longitude=0.) to a string
   with longitude notation. (eg. xcoord= -25. returns lab="25W")
   -----------------------------------------------------------------
*/
void
gdrawCVlon(double xcoord, int ndec, char *lab, int lab_size)
{
	char dir[2];
	int npi;
	double x=0.;

	dir[1] = '\0';
	if(xcoord != 0.)
	{
	    if(xcoord  >  0.)
	    {
		npi = (int)((xcoord - .00001)/180.);
		if(mod(npi, 2) == 0)
		{
		    x = xcoord - npi*180.;
		    dir[0] = 'E';
		}
		else
		{
		    x = (npi + 1)*180. - xcoord;
		    dir[0] = 'W';
		}
	    }
	    else if(xcoord  <  0.)
	    {
		npi = (int)((-xcoord - .00001)/180.);
		if(mod(npi, 2) == 0)
		{
		    x = -xcoord - npi*180.;
		    dir[0] = 'W';
		}
		else
		{
		    x = (npi + 1)*180. + xcoord;
		    dir[0] = 'E';
		}
	    }
	    ftoa(fabs(x), ndec, 1, lab, lab_size);
	    strncat(lab, dir, lab_size-strlen(lab));
	    lab[lab_size-1] = '\0';
	}
	else stringcpy(lab, "0", lab_size);
}

/*;
   -----------------------------------------------------------------
   convert an y-coordinate value (=0. at equator) to a string
   with latitude notation. (eg. ycoord= 25. returns lab="25N")
   -----------------------------------------------------------------
*/
void
gdrawCVlat(double ycoord, int ndec, char *lab, int lab_size)
{
	if(ycoord == 0.)
	{
	    stringcpy(lab, "EQ", lab_size);
	}
	else
	{
	    ftoa(fabs(ycoord), ndec, 1, lab, lab_size);
	    if(ycoord  <  0.)
	    {
		strncat(lab, "S", lab_size-strlen(lab));
	    }
	    else
	    {
		strncat(lab, "N", lab_size-strlen(lab));
	    }
	    lab[lab_size-1] = '\0';
	}
}
