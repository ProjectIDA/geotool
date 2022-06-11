/*
 * NAME
 *      plotsym:	draw symbols
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <math.h>
#include <string.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */


#include "libgplot.h"

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
#
void
plotsym(
	DrawStruct *d,
	int n,
	float *x, float *y,
	int symbol,
	int symbol_size,
	double xmin, double xmax, double ymin, double ymax)
{
	int i, h;
	double scale_x(), scale_y();
	double x1, x2, y1, y2;
	float x_size, y_size;

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
			imove(d, (double)x[i]-x_size, (double)y[i]-y_size);
			idraw(d, (double)x[i]-x_size, (double)y[i]+y_size);
			idraw(d, (double)x[i]+x_size, (double)y[i]+y_size);
			idraw(d, (double)x[i]+x_size, (double)y[i]-y_size);
			idraw(d, (double)x[i]-x_size, (double)y[i]-y_size);
		}
	}
	else if(symbol == DIAMOND || symbol == FILLED_DIAMOND)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, (double)x[i],        (double)y[i]-y_size);
			idraw(d, (double)x[i]-x_size, (double)y[i]);
			idraw(d, (double)x[i],        (double)y[i]+y_size);
			idraw(d, (double)x[i]+x_size, (double)y[i]);
			idraw(d, (double)x[i],        (double)y[i]-y_size);
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
			imove(d, (double)x[i]-x_size, (double)y[i]-y_size);
			idraw(d, (double)x[i], 	(double)y[i]+y_size);
			idraw(d, (double)x[i]+x_size, (double)y[i]-y_size);
			idraw(d, (double)x[i]-x_size, (double)y[i]-y_size);
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
                        imove(d, (double)x[i]+x_size, (double)y[i]+y_size);
                        idraw(d, (double)x[i],  (double)y[i]-y_size);
                        idraw(d, (double)x[i]-x_size, (double)y[i]+y_size);
                        idraw(d, (double)x[i]+x_size, (double)y[i]+y_size);
                }
        }
	else if(symbol == PLUS)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i], (double)y[i]-y_size);
			idraw(d, x[i], (double)y[i]+y_size);
			imove(d, x[i]-x_size, (double)y[i]);
			idraw(d, x[i]+x_size, (double)y[i]);
		}
	}
	else if(symbol == EX)
	{
		for(i = 0; i < n; i++) if(finite(y[i]))
		{
			imove(d, x[i]-x_size, (double)y[i]-y_size);
			idraw(d, x[i]+x_size, (double)y[i]+y_size);
			imove(d, x[i]-x_size, (double)y[i]+y_size);
			idraw(d, x[i]+x_size, (double)y[i]-y_size);
		}
	}
	iflush(d);
}

int
DrawSymbolFromName(char *name)
{
	if(!strcasecmp(name, "square")) return SQUARE;
	else if(!strcasecmp(name, "triangle")) return TRIANGLE;
	else if(!strcasecmp(name, "plus")) return PLUS;
	else if(!strcasecmp(name, "ex")) return EX;
	else if(!strcasecmp(name, "inv-triangle")) return INV_TRIANGLE;
	else if(!strcasecmp(name, "inv_triangle")) return INV_TRIANGLE;
	else if(!strcasecmp(name, "diamond")) return DIAMOND;
	else if(!strcasecmp(name, "circle")) return CIRCLE;
	else if(!strcasecmp(name, "filled_square")) return FILLED_SQUARE;
	else if(!strcasecmp(name, "solid-square")) return FILLED_SQUARE;
	else if(!strcasecmp(name, "filled_triangle")) return FILLED_TRIANGLE;
	else if(!strcasecmp(name, "solid-triangle")) return FILLED_TRIANGLE;
	else if(!strcasecmp(name, "filled_inv_triangle")) 
			return FILLED_INV_TRIANGLE;
	else if(!strcasecmp(name, "solid-inv-tri")) return FILLED_INV_TRIANGLE;
	else if(!strcasecmp(name, "filled_diamond")) return FILLED_DIAMOND;
	else if(!strcasecmp(name, "solid-diamond")) return FILLED_DIAMOND;
	else if(!strcasecmp(name, "filled_circle")) return FILLED_CIRCLE;
	else if(!strcasecmp(name, "solid-circle")) return FILLED_CIRCLE;
	else return -1;
}
