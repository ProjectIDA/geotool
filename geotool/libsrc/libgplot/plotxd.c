/*
 * NAME
 *      plotxd: compute non-overlapping XSegments
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <math.h>
#ifdef HAVE_IEEEFP_H 
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */
#include <stdlib.h>

#include "libgplot.h"
#include "alloc.h"

static void plotXY(int n, float *x, float *y, double mean, double xscale,
			double yscale, double xbeg, RSegArray *r);
static void plotY(int n, float *y, double mean, double xscale, double yscale,
			double xbeg, double x_incr, RSegArray *r);
static void finish_repeat(RSegArray *r, double last_x, double y_min,
			double y_max);

void
plotxd(int n, float *x, float *y, double mean, double xscale, double yscale,
	double xbeg, double x_incr, RSegArray *r)
{
	if(r->segs != NULL)
	{
		free(&r->segs);
		r->segs = NULL;
	}
	r->size_segs = 0;
	r->nsegs = 0;
	if(x_incr <= 0.)
	{
		plotXY(n, x, y, mean, fabs(xscale), yscale, xbeg, r);
	}
	else
	{
		plotY(n, y, mean, fabs(xscale), yscale, xbeg, x_incr, r);
	}
}
/*   ------------------------------------------------------------------
 *   plot the points in x,y
 *
 *   n         = (i) number of points.
 *   x         = (i) n x-coordinates.
 *   y         = (i) n y-coordinates.
 *   ------------------------------------------------------------------
 */
static void
plotXY(int n, float *x, float *y, double mean, double xscale, double yscale,
	double xbeg, RSegArray *r)
{
	double	xmid, ymid, xd, yd;
        double	last_x=0., last_y=0.;
        double	next_x, next_y;
        double	y_min=0., y_max=0.;
        double	dx, dy, D, xincr, yincr, dx2;
	int	i;
        char	repeat=False;

	if(n <= 0) return;

	r->nsegs = 0;
	for(i = n-1; i >= 0 && !finite(y[i]); i--);
	if(i >= 0) r->xmax = floor((xbeg + x[i]) * xscale);

	for(i = 0; i < n && !finite((*y)); i++, x++, y++);
	if(i < n)
	{
	    last_x = floor((xbeg + *x++) * xscale);
            last_y =  - (*y++ - mean) * yscale;
	    y_min = y_max = last_y;
	    repeat = False;
	    i++;
	    r->xmin = last_x;
	}
	if(r->xmin > r->xmax)
	{
	    D = r->xmin;
	    r->xmin = r->xmax;
	    r->xmax = D;
	}

	for(; i < n; i++, x++, y++)
	{
	    if(!finite((*y)))
	    {
		for(; i < n && !finite((*y)); i++, x++, y++);
		if(i < n)
		{
		    last_x = floor((xbeg + *x) * xscale);
		    last_y =     - (*y - mean) * yscale;
		    y_min = y_max = last_y;
		    repeat = False;
		}
		continue;
	    }
			
            xd = floor((xbeg + *x) * xscale);
            yd =     - (*y - mean) * yscale;

            if(xd == last_x) {
		if(y_min > yd) y_min = yd;
		else if(y_max < yd) y_max = yd;
		repeat = True;
            }
            else
            {
		if(repeat) {
		    /* xd should be > last_x */
		    dx = fabs(xd - last_x);
		    xincr = (xd > last_x) ? 1 : -1;
		    dy = fabs(yd - last_y);
		    if( dx >= dy ) {
			next_x = last_x + xincr;
			D = 2*dy - dx;
			if(D >= 0) {
			    next_y = (yd > last_y) ? last_y + 1 : last_y - 1;
			}
			else {
			    next_y = last_y;
			}
		    }
		    else {
			yincr = (yd > last_y) ? 1 : -1;
			next_y = last_y;
			dx2 = 2*dx;
			D = dx2 - dy;
			if(D < 0 ) {
			    next_y += yincr*ceil(fabs(D)/dx2);
			    if(next_y < y_min) y_min = next_y;
			    else if(next_y > y_max) y_max = next_y;
			}
			next_x = last_x + xincr;
			next_y += yincr;
		    }
		    ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs,
					r->segs, RSeg, 512);
		    r->segs[r->nsegs].x1 = last_x;
		    r->segs[r->nsegs].y1 = y_min;
		    r->segs[r->nsegs].x2 = last_x;
		    r->segs[r->nsegs].y2 = y_max;
		    if(r->nsegs == 0) {
			r->ymin = y_min;
			r->ymax = y_max;
		    }
		    else {
			if(y_min < r->ymin) r->ymin = y_min;
			if(y_max > r->ymax) r->ymax = y_max;
		    }
		    r->nsegs++;
		    xmid = floor(next_x);
		    ymid =       next_y;
		}
		else {
		    xmid = last_x;
		    ymid = last_y;
		}
		if(xd == xmid) {
		    y_min = (yd < ymid) ? yd : ymid;
		    y_max = (yd > ymid) ? yd : ymid;
		    repeat = True;
		}
		else {
		    y_min = y_max = yd;
		    repeat = False;

		    /* xd should be > last_x */
		    dx = fabs(xd - last_x);
		    xincr = (xd > last_x) ? 1 : -1;
		    dy = fabs(last_y - yd);
		    if( dx >= dy )
		    {
			next_x = xd - xincr;
			D = 2*dy - dx;
			if(D >= 0) {
			    next_y = (last_y > yd) ? yd+1 : yd-1;
			}
			else {
			    next_y = yd;
			}
		    }
		    else
		    {
			yincr = (last_y > yd) ? 1 : -1;
			next_y = yd;
			dx2 = 2*dx;
			D = dx2 - dy;
			if(D < 0 ) {
			    repeat = True;
			    next_y += yincr*ceil(fabs(D)/dx2);
			    if(next_y < y_min) y_min = next_y;
			    else if(next_y > y_max) y_max = next_y;
			}
			next_x = xd - xincr;
			next_y += yincr;
		    }
		    ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs,
					r->segs, RSeg, 512);
		    next_x = floor(next_x);
		    next_y =       next_y;
		    r->segs[r->nsegs].x1 = xmid;
		    r->segs[r->nsegs].y1 = ymid;
		    r->segs[r->nsegs].x2 = next_x;
		    r->segs[r->nsegs].y2 = next_y;
		    if(r->nsegs == 0)
		    {
			if(ymid < next_y) {
			    r->ymin = ymid; r->ymax= next_y;
			}
			else {
			    r->ymin = next_y; r->ymax= ymid;
			}
		    }
		    else
		    {
			if(ymid < next_y) {
			    if(ymid < r->ymin) r->ymin = ymid;
			    if(next_y > r->ymax) r->ymax = next_y;	
			}
			else {
			    if(next_y < r->ymin) r->ymin = next_y;
			    if(ymid > r->ymax) r->ymax = ymid;
			}
		    }
		    r->nsegs++;
		}
	    }
	    last_x = xd;
	    last_y = yd;
	}
        if(repeat)
	{
	    finish_repeat(r, last_x, y_min, y_max);
	}
}

static void
plotY(int n, float *y, double mean, double xscale, double yscale, double xbeg,
	double x_incr, RSegArray *r)
{
	double	xmid, ymid, xd, yd;
        double	last_x=0., last_y=0.;
        double	next_x, next_y;
        double	y_min=0., y_max=0.;
        double	dx, dy, D, yincr, dx2;
	int	i;
        char	repeat=False;

	if(n <= 0) return;

	for(i = n-1; i >= 0 && !finite(y[i]); i--);
	if(i >= 0) r->xmax = floor(i*x_incr * xscale);
	
	r->nsegs = 0;
	for(i = 0; i < n && !finite(*y); i++, y++);
	if(i < n)
	{
	    last_x = floor((xbeg+i*x_incr) * xscale);
            last_y =     - (*y++ - mean) * yscale;
	    y_min = y_max = last_y;
	    repeat = False;
	    i++;
	    r->xmin = last_x;
	}

	for(; i < n; i++, y++)
	{
	    if(!finite((*y)))
	    {
		for(; i < n && !finite((*y)); i++, y++);
		if(i < n) {
		    last_x = floor((xbeg+i*x_incr) * xscale);
		    last_y =     - (*y - mean) * yscale;
		    y_min = y_max = last_y;
		    repeat = False;
		}
		continue;
	    }
			
	    xd = floor((xbeg+i*x_incr) * xscale);
	    yd = - (*y - mean) * yscale;

            if(xd == last_x) {
		if(y_min > yd) y_min = yd;
		else if(y_max < yd) y_max = yd;
		repeat = True;
            }
            else if(xd > last_x)
            {
		if(repeat)
		{
		    /* xd should be > last_x */
		    dx = xd - last_x;
		    dy = fabs(yd - last_y);
		    if( dx >= dy )
		    {
			next_x = last_x + 1;
			D = 2*dy - dx;
			if(D >= 0) {
			    next_y = (yd > last_y) ? last_y + 1 : last_y - 1;
			}
			else {
			    next_y = last_y;
			}
		    }
		    else
		    {
			yincr = (yd > last_y) ? 1 : -1;
			next_y = last_y;
			dx2 = 2*dx;
			D = dx2 - dy;
			if(D < 0 ) {
			    next_y += yincr*ceil(fabs(D)/dx2);
			    if(next_y < y_min) y_min = next_y;
			    else if(next_y > y_max) y_max = next_y;
			}
			next_x = last_x + 1;
			next_y += yincr;
		    }
		    ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs,
					r->segs, RSeg, 512);
		    r->segs[r->nsegs].x1 = last_x;
		    r->segs[r->nsegs].y1 = y_min;
		    r->segs[r->nsegs].x2 = last_x;
		    r->segs[r->nsegs].y2 = y_max;
		    if(r->nsegs == 0) {
			r->ymin = y_min;
			r->ymax = y_max;
		    }
		    else {
			if(y_min < r->ymin) r->ymin = y_min;
			if(y_max > r->ymax) r->ymax = y_max;
		    }
		    r->nsegs++;
		    xmid = floor(next_x);
		    ymid =       next_y;
		}
		else {
		    xmid = last_x;
		    ymid = last_y;
		}
		if(xd == xmid) {
		    y_min = (yd < ymid) ? yd : ymid;
		    y_max = (yd > ymid) ? yd : ymid;
				repeat = True;
		}
		else 
		{
		    /* xd should be > last_x */
		    dx = xd - last_x;
		    y_min = y_max = yd;
		    repeat = False;

		    dy = fabs(last_y - yd);
		    if( dx >= dy ) {
			next_x = xd - 1;
			D = 2*dy - dx;
			if(D >= 0) {
			    next_y = (last_y > yd) ?   yd+1 : yd-1;
			}
			else {
			    next_y = yd;
			}
		    }
		    else {
			yincr = (last_y > yd) ? 1 : -1;
			next_y = yd;
			dx2 = 2*dx;
			D = dx2 - dy;
			if(D < 0 ) {
			    repeat = True;
			    next_y += yincr*ceil(fabs(D)/dx2);
			    if(next_y < y_min) y_min = next_y;
			    else if(next_y > y_max) y_max = next_y;
			}
			next_x = xd - 1;
			next_y += yincr;
		    }
		    ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs,
					r->segs, RSeg, 512);
		    next_x = floor(next_x);
		    next_y =       next_y;
		    r->segs[r->nsegs].x1 = xmid;
		    r->segs[r->nsegs].y1 = ymid;
		    r->segs[r->nsegs].x2 = next_x;
		    r->segs[r->nsegs].y2 = next_y;
		    if(r->nsegs == 0) {
			if(ymid < next_y) {
			    r->ymin = ymid; r->ymax= next_y;
			}
			else {
			    r->ymin = next_y; r->ymax= ymid;
			}
		    }
		    else {
			if(ymid < next_y) {
			    if(ymid < r->ymin) r->ymin = ymid;
			    if(next_y > r->ymax) r->ymax = next_y;	
			}
			else {
			    if(next_y < r->ymin) r->ymin = next_y;
			    if(ymid > r->ymax) r->ymax = ymid;
			}
		    }
		    r->nsegs++;
		}
	    }
	    last_x = xd;
	    last_y = yd;
	}
        if(repeat)
	{
		finish_repeat(r, last_x, y_min, y_max);
	}
}

static void
finish_repeat(RSegArray	*r, double last_x, double y_min, double y_max)
{
	ALLOC((r->nsegs+1)*sizeof(RSeg), r->size_segs, r->segs, RSeg, 512);
	r->segs[r->nsegs].x1 = last_x;
	r->segs[r->nsegs].y1 = y_min;
	r->segs[r->nsegs].x2 = last_x;
	r->segs[r->nsegs].y2 = y_max;
	if(r->nsegs == 0)
	{
		r->ymin = y_min;
		r->ymax = y_max;
	}
	else
	{
		if(y_min < r->ymin) r->ymin = y_min;
		if(y_max > r->ymax) r->ymax = y_max;
	}
	r->nsegs++;
}
