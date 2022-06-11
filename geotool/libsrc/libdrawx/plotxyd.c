/*
 * NAME
 *      plotxy: draw x-y plot.
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */

#include "libdrawx.h"

/*   ------------------------------------------------------------------
 *   plot the points in x,y
 *
 *   n         = (i) number of points.
 *   x         = (i) n x-coordinates.
 *   y         = (i) n y-coordinates.
 *   xmin,xmax = (i) limits of the plot region for x-values.
 *   ymin,ymax = (i) limits of the plot region for y-values.
 *               if xmin=xmax=0. xmin and xmax are computed here.
 *               if ymin=ymax=0. ymin and ymax are computed here.
 *   ------------------------------------------------------------------
 */
void
plotxyd(DrawStruct *d, int n, double *x, float *y, double xmin, double xmax,
		double ymin, double ymax)
{
	int i, imov;
	double x1, x2, y1, y2;

	x1 = xmin;
	x2 = xmax;
	y1 = ymin;
	y2 = ymax;
	if(x1==0. && x2==0.) /* find xmin, xmax */
	{
		x1 = x[0];
		x2 = x[0];
		for(i = 1; i < n; i++)
		{
			x1 = (x1 < x[i]) ? x1 : x[i]; 
			x2 = (x2 > x[i]) ? x2 : x[i];
		}
	}
	if(y1==0. && y2==0.) /* find ymin, ymax */
	{
		y1 = y[0];
		y2 = y[0];
		for(i = 1; i < n; i++)
		{
			y1 = (y1 < y[i]) ? y1 : y[i];
			y2 = (y2 > y[i]) ? y2 : y[i];
		}
	}
 
	SetClipArea(d, x1, y1, x2, y2);

	imov = 1;
	for(i = 0; i < n; i++)
	{
		if(finite(y[i]) && !fNaN(y[i]))
		{
			if(imov)
			{
				imove(d, x[i], y[i]);
				imov = 0;
			}
			else idraw(d, x[i], y[i]);
		}
		else imov = 1;
	}
	/* flush the draw buffer. */
	if( n > 0 )
	{
		iflush(d);
	}
}

/*
 * NAME
 *      plotsym:	draw symbols
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

/*   ------------------------------------------------------------------
 *   plot symbols at the points in x,y
 *
 *   n         = (i) number of points.
 *   x         = (i) n x-coordinates.
 *   y         = (i) n y-coordinates.
 *   xmin,xmax = (i) limits of the plot region for x-values.
 *   ymin,ymax = (i) limits of the plot region for y-values.
 *               if xmin=xmax=0. xmin and xmax are computed here.
 *               if ymin=ymax=0. ymin and ymax are computed here.
 *   ------------------------------------------------------------------
 */
void
plotsymd(DrawStruct *d, int n, double *x, float *y, int symbol,
	int symbol_size, double xmin, double xmax, double ymin, double ymax)
{
	int i, h;
	double scale_x(), scale_y();
	double x1, x2, y1, y2, x_size;
	float y_size;

	if(n <= 0)
	{
		return;
	}
	/* get scaled symbol size.
	 */
	x_size = fabs(scale_x(d, symbol_size/2) - scale_x(d, 0));
	y_size = fabs(scale_y(d, symbol_size/2) - scale_y(d, 0));
	x1 = xmin;
	x2 = xmax;
	y1 = ymin;
	y2 = ymax;
	if(x1==0. && x2==0.) /* find xmin, xmax */
	{
		x1 = x[0];
		x2 = x[0];
		for(i = 1; i < n; i++)
		{
			x1 = (x1 < x[i]) ? x1 : x[i]; 
			x2 = (x2 > x[i]) ? x2 : x[i];
		}
		x1 -= x_size;
		x2 += x_size;
	}
	if(y1==0. && y2==0.) /* find ymin, ymax */
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			y1 = y[i];
			y2 = y[i];
			break;
		}
		for(; i < n; i++) if(finite(y[i]))
		{
			y1 = (y1 < y[i]) ? y1 : y[i];
			y2 = (y2 > y[i]) ? y2 : y[i];
		}
		y1 -= y_size;
		y2 += y_size;
	}
 
	SetClipArea(d, x1, y1, x2, y2);

	if(symbol == SQUARE || symbol == FILLED_SQUARE)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i]-x_size, (double)(y[i]-y_size));
			idraw(d, x[i]-x_size, (double)(y[i]+y_size));
			idraw(d, x[i]+x_size, (double)(y[i]+y_size));
			idraw(d, x[i]+x_size, (double)(y[i]-y_size));
			idraw(d, x[i]-x_size, (double)(y[i]-y_size));
		}
	}
	else if(symbol == DIAMOND || symbol == FILLED_DIAMOND)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i],        (double)(y[i]-y_size));
			idraw(d, x[i]-x_size, (double)y[i]);
			idraw(d, x[i],        (double)(y[i]+y_size));
			idraw(d, x[i]+x_size, (double)y[i]);
			idraw(d, x[i],        (double)(y[i]-y_size));
		}
	}
	else if(symbol == TRIANGLE || symbol == FILLED_TRIANGLE)
	{
		/* symbol_size = the length of each side.
		 * Find the distance from the center to the bottom side.
		 */
		h = (int)(symbol_size*tan(3.14159*30./180.));
		y_size = fabs(scale_y(d, h) - scale_y(d, 0));
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i]-x_size, (double)(y[i]-y_size));
			idraw(d, x[i], 	      (double)(y[i]+y_size));
			idraw(d, x[i]+x_size, (double)(y[i]-y_size));
			idraw(d, x[i]-x_size, (double)(y[i]-y_size));
		}
	}
        else if(symbol == INV_TRIANGLE || symbol == FILLED_INV_TRIANGLE)
        {
                /* symbol_size = the length of each side.
                 * Find the distance from the center to the bottom side.
                 */
                h = (int)(symbol_size*tan(3.14159*30./180.));
                y_size = fabs(scale_y(d, h) - scale_y(d, 0));
                for(i = 0; i < n; i++) if(finite(y[i]))
                {
                        imove(d, x[i]+x_size, (double)(y[i]+y_size));
                        idraw(d, x[i],        (double)(y[i]-y_size));
                        idraw(d, x[i]-x_size, (double)(y[i]+y_size));
                        idraw(d, x[i]+x_size, (double)(y[i]+y_size));
                }
        }
	else if(symbol == PLUS)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i], (double)(y[i]-y_size));
			idraw(d, x[i], (double)(y[i]+y_size));
			imove(d, x[i]-x_size, (double)y[i]);
			idraw(d, x[i]+x_size, (double)y[i]);
		}
	}
	else if(symbol == EX)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i]-x_size, (double)(y[i]-y_size));
			idraw(d, x[i]+x_size, (double)(y[i]+y_size));
			imove(d, x[i]-x_size, (double)(y[i]+y_size));
			idraw(d, x[i]+x_size, (double)(y[i]-y_size));
		}
	}
	iflush(d);
}
