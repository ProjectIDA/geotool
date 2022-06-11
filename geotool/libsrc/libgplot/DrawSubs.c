/*
 * NAME
 *      drawing subroutines, including scaling and clipping.
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */


#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "libgplot.h"
#include "alloc.h"

#ifndef HAVE_NINT
extern int	nint(double);
#endif /* HAVE_NINT */

#define between(a,b,c)  ( (b<=a && a<=c) || (c<=a && a<=b) )

static void Move(DrawStruct *d, double x, double y);
static void Draw(DrawStruct *d, double x, double y);

void
imove(DrawStruct *d, double x, double y)
{
	d->lastin = between(x, d->drawxmin, d->drawxmax) && 
			between(y, d->drawymin, d->drawymax);
	if(d->lastin)
	{
		Move(d, x, y);
	}
	d->lastx = x;
	d->lasty = y;
}

void
idraw(DrawStruct *d, double x, double y)
{
	int 	in, mov;
	double	xin, xout, yin, yout, xbot, xtop, yleft, yrite, cx=0., cy=0.,
		slope, intercept, xmin, xmax, ymin, ymax;
   
	xmin = d->drawxmin;  xmax = d->drawxmax;
	ymin = d->drawymin;  ymax = d->drawymax;

	in = between(x, xmin, xmax) && between(y, ymin, ymax);

	if(d->lastin && in) /* last point and this point are in */
	{ 
		Draw(d, x, y);
	}
	else if(d->lastin != in)
	{
		/* one point is in (last pt or this pt) and the other point 
		 * is out.
		 */
		if( !d->lastin )
		{
			xout = d->lastx;
			yout = d->lasty;
			xin = x;
			yin = y;
		}
		else 
		{
			xout = x;
			yout = y;
			xin = d->lastx;
			yin = d->lasty;
		}
		/* if the inside point is on the boundary, move it slightly 
		 * inside, so the logic below will work.
		 */
		if(xin == xmin)      xin += 1.e-5*(xmax-xmin);
		else if(xin == xmax) xin -= 1.e-5*(xmax-xmin);

		if(yin == ymin)      yin += 1.e-5*(ymax-ymin);
		else if(yin == ymax) yin -= 1.e-5*(ymax-ymin);
      

		/* find the point cx,cy where the line segment (xin,yin) to 
		 * (xout,yout) intersects the boundary.
		 */

		if(xin == xout) /* the line segment is vertical */
		{
			if( between(ymin, yout, yin) )
			{
				cx = xout; 
				cy = ymin;
			}
			else
			{
				cx = xout; 
				cy = ymax;
			}
		}
		else if(yin == yout)  /* the line segment is horizontal */
		{
			if( between(xmin, xout, xin) )
			{
				cx = xmin; 
				cy = yout;
			}
			else 
			{
				cx = xmax; 
				cy = yout;
			}
		}
		else 
		{
			slope = (yout - yin) / (xout - xin);
			intercept = .5*(yout + yin - slope*(xout + xin));
			yleft = intercept + slope*xmin;
			yrite = intercept + slope*xmax;
			xbot  = (ymin - intercept)/slope;
			xtop  = (ymax - intercept)/slope;
			if(between(yleft, ymin, ymax) && 
				between(xmin, xout, xin))
			{ 
				cx = xmin; 
				cy = yleft; 
			}
			else if(between(yrite, ymin, ymax) && 
				between(xmax, xout, xin))
			{
				cx = xmax;
				cy = yrite;
			}
			else if(between(xbot, xmin, xmax) && 
				between(ymin, yout, yin))
			{
				cx = xbot; 
				cy = ymin;
			}
			else if(between(xtop, xmin, xmax) && 
				between(ymax, yout, yin))
			{
				cx = xtop; 
				cy = ymax;
			}
		}
		if(d->lastin)
		{
			Draw(d, cx, cy);
		}
		else
		{
			Move(d, cx, cy);
			Draw(d, x, y);
		}
	}
	else
	{
		/* both the last point and this point are out. 
	 	 * check if the line segment joining them intersects 
		 * the plot region.
		 */

		if(x == d->lastx) /* the line segment is vertical */
		{
			if(between(x, xmin, xmax) && between(ymin, y, d->lasty))
			{
				Move(d, x, ymin); 
				Draw(d, x, ymax);
			}
		}
		else if(y == d->lasty) /* the line segment is horizontal */
		{
			if(between(y, ymin, ymax) && between(xmin, x, d->lastx))
			{ 
				Move(d, xmin, y);
				Draw(d, xmax, y);
			}
		}
		else 	/* line segment is not vertical and not horizontal */
		{
			slope = (y - d->lasty) / (x - d->lastx);
			intercept = .5*(y + d->lasty - slope*(x + d->lastx));
			yleft = intercept + slope*xmin;
			yrite = intercept + slope*xmax;
			xbot  = (ymin-intercept)/slope;
			xtop  = (ymax-intercept)/slope;
			mov = 0;
			if(between(yleft, ymin, ymax) && 
				between(xmin, x, d->lastx))
			{
				Move(d, xmin, yleft); 
				mov = 1;
			}
			if(between(yrite, ymin, ymax) && 
				between(xmax, x, d->lastx)){
				if( !mov )
				{
					Move(d, xmax, yrite); 
					mov = 1;
				}
				else
				{
					Draw(d, xmax, yrite);
				}
			}
			if(between(xbot, xmin, xmax) && 
				between(ymin, y, d->lasty))
			{
				if( !mov )
				{
					Move(d, xbot, ymin);
					mov = 1;
				}
				else
				{
					Draw(d, xbot, ymin);
				}
			}
			if(between(xtop, xmin, xmax) && 
				between(ymax, y, d->lasty))
			{
				Draw(d, xtop, ymax);
			}
		}
	}
	d->lastin = in;
	d->lastx = x;
	d->lasty = y;
}

static void
Move(DrawStruct *d, double x, double y)
{
	iflush(d);

	d->last_ix = unscale_x(d, x);
	d->last_iy = unscale_y(d, y);
	d->moved = True;
	d->repeating_x = False;
	d->repeating_y = False;
}

void
iflush(DrawStruct *d)
{
	if(d->repeating_x)
	{
		ALLOC((d->s->nsegs+1)*sizeof(DSegment), d->s->size_segs,
				d->s->segs, DSegment, 512);
		d->s->segs[d->s->nsegs].x1 = d->last_ix;
		d->s->segs[d->s->nsegs].y1 = d->repeat_ymin;
		d->s->segs[d->s->nsegs].x2 = d->last_ix;
		d->s->segs[d->s->nsegs].y2 = d->repeat_ymax;
		d->s->nsegs++;
	}
	else if(d->repeating_y)
	{
		ALLOC((d->s->nsegs+1)*sizeof(DSegment), d->s->size_segs,
				d->s->segs, DSegment, 512);
		d->s->segs[d->s->nsegs].x1 = d->repeat_xmin;
		d->s->segs[d->s->nsegs].y1 = d->last_iy;
		d->s->segs[d->s->nsegs].x2 = d->repeat_xmax;
		d->s->segs[d->s->nsegs].y2 = d->last_iy;
		d->s->nsegs++;
	}
	d->repeating_x = False;
	d->repeating_y = False;
}

static void
Draw(DrawStruct *d, double x, double y)
{
	short ix, iy;

	ix = unscale_x(d, x);
	iy = unscale_y(d, y);

	if(!d->moved && !d->repeating_y && ix == d->last_ix)
	{
		if(d->repeat_ymin > iy) d->repeat_ymin = iy;
		if(d->repeat_ymax < iy) d->repeat_ymax = iy;
		d->repeating_x = True;
		d->last_ix = ix;
		d->last_iy = iy;
		d->moved = False;
		return;
	}
	else if(d->repeating_x)
	{
		ALLOC((d->s->nsegs+1)*sizeof(DSegment), d->s->size_segs,
			d->s->segs, DSegment, 512);
		d->s->segs[d->s->nsegs].x1 = d->last_ix;
		d->s->segs[d->s->nsegs].y1 = d->repeat_ymin;
		d->s->segs[d->s->nsegs].x2 = d->last_ix;
		d->s->segs[d->s->nsegs].y2 = d->repeat_ymax;
		d->s->nsegs++;
	}
	else if(!d->moved && iy == d->last_iy)
	{
		if(d->repeat_xmin > ix) d->repeat_xmin = ix;
		if(d->repeat_xmax < ix) d->repeat_xmax = ix;
		d->repeating_y = True;
		d->last_ix = ix;
		d->last_iy = iy;
		d->moved = False;
		return;
	}
	else if(d->repeating_y)
	{
		ALLOC((d->s->nsegs+1)*sizeof(DSegment), d->s->size_segs,
			d->s->segs, DSegment, 512);
		d->s->segs[d->s->nsegs].x1 = d->repeat_xmin;
		d->s->segs[d->s->nsegs].y1 = d->last_iy;
		d->s->segs[d->s->nsegs].x2 = d->repeat_xmax;
		d->s->segs[d->s->nsegs].y2 = d->last_iy;
		d->s->nsegs++;
	}
	ALLOC((d->s->nsegs+1)*sizeof(DSegment), d->s->size_segs,
		d->s->segs, DSegment, 512);
	d->s->segs[d->s->nsegs].x1 = d->last_ix;
	d->s->segs[d->s->nsegs].y1 = d->last_iy;
	d->s->segs[d->s->nsegs].x2 = ix;
	d->s->segs[d->s->nsegs].y2 = iy;
	d->s->nsegs++;
	d->repeat_ymin = d->repeat_ymax = iy;
	d->repeat_xmin = d->repeat_xmax = ix;
	d->repeating_x = False;
	d->repeating_y = False;
	d->last_ix = ix;
	d->last_iy = iy;
	d->moved = False;
}

void
SetScale(DrawStruct *d, double x1, double y1, double x2, double y2)
{
	double xdif, ydif;

	d->sx1 = x1;
	d->sy1 = y1;
	d->sx2 = x2;
	d->sy2 = y2;
	xdif = d->sx2 - d->sx1;
	ydif = d->sy2 - d->sy1;
	d->unscalex = (xdif != 0) ? (d->ix2 - d->ix1)/xdif : 1.;
	d->unscaley = (ydif != 0) ? (d->iy2 - d->iy1)/ydif : 1.;
	d->scalex = (d->unscalex != 0.) ? 1./d->unscalex : 1.;
	d->scaley = (d->unscaley != 0.) ? 1./d->unscaley : 1.;
	d->drawxmin = d->sx1;
	d->drawxmax = d->sx2;
	d->drawymin = d->sy1;
	d->drawymax = d->sy2;
}

void
GetScale(DrawStruct *d, double *x1, double *y1, double *x2, double *y2)
{
	*x1 = d->sx1;
	*y1 = d->sy1;
	*x2 = d->sx2;
	*y2 = d->sy2;
}

void
ResetClipArea(DrawStruct *d)
{
	SetClipArea(d, 0., 0., 0., 0.);
}

void
SetClipArea(DrawStruct *d, double x1, double y1, double x2, double y2)
{
	if(x1 == x2 || y1 == y2)
	{
		/* return to the default draw limits.
		 */
		d->drawxmin = d->sx1;
		d->drawxmax = d->sx2;
		d->drawymin = d->sy1;
		d->drawymax = d->sy2;
	}
	else
	{
		d->drawxmin = x1;
		d->drawxmax = x2;
		d->drawymin = y1;
		d->drawymax = y2;
	}
	d->moved = True;
	d->repeating_x = False;
	d->repeating_y = False;
}

void
GetClipArea(DrawStruct *d, double *x1, double *y1, double *x2, double *y2)
{
	*x1 = d->drawxmin;
	*x2 = d->drawxmax;
	*y1 = d->drawymin;
	*y2 = d->drawymax;
}

void
SetDrawArea(DrawStruct *d, int ix1, int iy1, int ix2, int iy2)
{

	d->ix1 = ix1;
	d->iy1 = iy1;
	d->ix2 = ix2;
	d->iy2 = iy2;
	d->last_ix = ix1;
	d->last_iy = iy1;
	d->moved = True;
	d->repeating_x = False;
	d->repeating_y = False;

	d->sx1 = ix1;
	d->sx2 = ix2;
	d->sy1 = iy1;
	d->sy2 = iy2;

	d->scalex = 1.0;
	d->scaley = 1.0;
	d->unscalex = 1.0;
	d->unscaley = 1.0;

	d->drawxmin = d->sx1;
	d->drawxmax = d->sx2;
	d->drawymin = d->sy1;
	d->drawymax = d->sy2;

	d->lastin = False;
	d->lastx = d->sx1;
	d->lasty = d->sy1;

	d->last_ix = ix1;
	d->last_iy = iy1;
}

void
GetDrawArea(DrawStruct *d, int *ix1, int *iy1, int *ix2, int *iy2)
{
	*ix1 = d->ix1;
	*iy1 = d->iy1;
	*ix2 = d->ix2;
	*iy2 = d->iy2;
}

int
unscale_x(DrawStruct *d, double x)
{
	int ix;

	ix = d->ix1 + nint((x - d->sx1) * d->unscalex);
	return(ix);
}

int
unscale_y(DrawStruct *d, double y)
{
	int iy;

	iy = d->iy1 + nint((y - d->sy1) * d->unscaley);
	return(iy);
}

double
scale_x(DrawStruct *d, int x)
{
	double sx;

	sx = d->sx1 + (x - d->ix1)*d->scalex;
	return(sx);
}

double
scale_y(DrawStruct *d, int y)
{
	double sy;

	sy = d->sy1 + (y - d->iy1)*d->scaley;
	return(sy);
}
