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

#include "libgplot.h"

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
plotxy(DrawStruct *d, int n, float *x, float *y, double xmin, double xmax,
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
