/** \file shade.c
 *  \brief Defines color contouring routines.
 */
/*
 * NAME
 *      color contouring.
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Xm/Xm.h>

#include "libdrawx.h"

#define Z(i,j) a->z[i+(j)*a->nx]

static void antiAlias(Matrx *a, int x, int *iy, int i, int width, int j1,
		int j2, Pixel *yColors, int num_colors, Pixel *colors,
		Pixel no_data_color, double *lines, XImage *image,
		Pixel lowerOutlier, Pixel upperOutlier);

static int ndex(double x, int n, double *xg);

/* assumes delx and dely are constant
 */
void
shade(Display *display, Drawable window, GC gc, DrawStruct *d, int num_colors, 
	Pixel *colors, double *lines, int num_stipples, Pixmap *stipples, 
	Matrx *a)
{
	int i, j, k, m, n, width=0, height=0;
	int i1, i2, j1, j2, x, y;
	float aij, amin, amax;

	if((num_colors <= 0 && num_stipples <= 0) ||
	   (num_colors > 0 && colors == NULL) ||
	   (num_stipples > 0 && stipples == NULL))
	{
		return;
	}
	/* get the matrix indices nearest to x1,y1 and x2,y2.
	*/
	i1 = ndex(a->x1, a->nx, a->x) - 1;
	i2 = ndex(a->x2, a->nx, a->x) - 1;
	j1 = ndex(a->y1, a->ny, a->y) - 1;
	j2 = ndex(a->y2, a->ny, a->y) - 1;
	if(a->x1 < a->x[i1] && i1 > 0)    i1 = i1 - 1;
	if(a->x2 > a->x[i2] && i2 < a->nx-1) i2 = i2 + 1;
	if(a->y1 < a->y[j1] && j1 > 0)    j1 = j1 - 1;
	if(a->y2 > a->y[j2] && j2 < a->ny-1) j2 = j2 + 1;

/*
	width  = abs(unscale_x(d, (double)a->x[1]) -
			unscale_x(d, (double)a->x[0])) + 1;
	height = abs(unscale_y(d, (double)a->y[1]) -
			unscale_y(d, (double)a->y[0])) + 1;
*/

	if(num_stipples > 0) {
	    n = num_stipples;
	    XSetFillStyle(display, gc, FillStippled);
	}
	else {
	    n = num_colors;
	    XSetFillStyle(display, gc, FillSolid);
	}

	amin = amax = Z(i1, j1);

	for(i = i1; i <= i2; i++)
	{
	    x = (int)(unscale_x(d, a->x[i]) - .5*width);
	    for(j = j1; j <= j2; j++)
	    {
		aij = Z(i,j);
		if(aij != a->exc)
		{
		    k = -1;
		    if(aij <= lines[0]) {
			k = 0;
		    }
		    if(aij >= lines[n]) {
			k = n-1;
		    }
		    else
		    {
			for(m = 0; m < n; m++)
			{
			    if(aij >= lines[m] && aij <= lines[m+1])
			    {
				k = m;
				break;
			    }
			}
		    }
		    y = (int)(unscale_y(d, a->y[j]) - .5*height);

		    if(num_stipples > 0)
		    {
			if(k > 0) {
			    /* assumes k = 0 is background
			     */
			    XSetStipple(display, gc, stipples[k]);
			    XFillRectangle(display, window, gc,
					x, y, width, height);
			}
		    }
		    else if(k >= 0) {
			XSetForeground(display, gc, colors[k]);
			XFillRectangle(display, window, gc, x, y, width,height);
		    }

		    if(amin > aij) {
			amin = aij;
			a->imin = i;
			a->jmin = j;
		    }
		    if(amax < aij) {
			amax = aij;
			a->imax = i;
			a->jmax = j;
		    }
		}
	    }
	}
	a->z_min = a->z[a->imin + a->jmin*a->nx];
	a->z_max = a->z[a->imax + a->jmax*a->nx];
	return;
}

void
shadeImage(Screen *screen, XImage *image, DrawStruct *d, Pixel no_data_color,
	Boolean *select_val, Boolean *dim_val, int num_colors, Pixel *colors,
	double *lines, Matrx *a, int *x0, int *y0, int *xwidth, int *yheight,
	Boolean applyAntiAlias, Boolean distinctOutliers, Pixel select_color)
{
	int i, j, k, l, width, height, klower, kupper, kmiddle;
	int i1, i2, j1, j2, x, y, last_x, last_y;
        Pixel  prev_pixel, this_pixel;
	int image_x1, image_x2, image_y1, image_y2;
	int *ix = NULL, *iy = NULL;
        Pixel *yColors = NULL;
	float aij, amin, amax, last_aij;
        Pixel lowerOutlier, upperOutlier;
	int o;

        /* if lowerOutlier and upperOutlier have valid pixel values, then 
           pixels with values outside the range of values given by colors[]
           will have the pixel color lowerOutlier or upperOutlier.  Otherwise,
	   outliers will be grouped with the highest or lowest colors[]
         */
        if (distinctOutliers)
        {
           lowerOutlier = WhitePixelOfScreen (screen);
           upperOutlier = BlackPixelOfScreen (screen);
        }
        else
        {
           lowerOutlier = colors[0];
           upperOutlier = colors[num_colors-1];
        }

	/* assumes that d->iy1 > d->iy2 */

	if((num_colors <= 0) || (num_colors > 0 && colors == NULL))
	{
	    return;
	}
	/* get the matrix indices nearest to x1,y1 and x2,y2.
	 */
	i1 = ndex(a->x1, a->nx, a->x) - 1;
	i2 = ndex(a->x2, a->nx, a->x) - 1;
	j1 = ndex(a->y1, a->ny, a->y) - 1;
	j2 = ndex(a->y2, a->ny, a->y) - 1;
	if(a->x1 < a->x[i1] && i1 > 0)    i1 = i1 - 1;
	if(a->x2 > a->x[i2] && i2 < a->nx-1) i2 = i2 + 1;
	if(a->y1 < a->y[j1] && j1 > 0)    j1 = j1 - 1;
	if(a->y2 > a->y[j2] && j2 < a->ny-1) j2 = j2 + 1;

	if((ix = (int *)malloc((i2-i1+2)*sizeof(int))) == NULL)
	{
	    return;
	}
	if((iy = (int *)malloc((j2-j1+2)*sizeof(int))) == NULL)
	{
	    if(ix != NULL) free(ix);
	    return;
	}
	if((yColors = (Pixel *)malloc((j2-j1+2)*sizeof(Pixel))) == NULL)
	{
	    if(ix != NULL) free(ix);
	    if(iy != NULL) free(iy);
	    return;
	}
	/* compute ix[i], the left side of a block centered on a->x[i]
	 */
	for(i = i1, k = 0; i <= i2; i++, k++)
	{
	    if(i == 0) {
		ix[k] = unscale_x(d, a->x[i] - .5*(a->x[i+1] - a->x[i]));
	    }
	    else {
		ix[k] = unscale_x(d, .5*(a->x[i-1] + a->x[i]));
	    }
	    if(ix[k] < 0) ix[k] = 0;
	    if(ix[k] >= image->width) ix[k] = image->width-1;
	}
	if(i2 == a->nx-1) {
	    ix[k] = unscale_x(d, a->x[i2] + .5*(a->x[i2]-a->x[i2-1]));
	}
	else {
	    ix[k] = unscale_x(d, .5*(a->x[i2] + a->x[i2+1]));
	}
	if(ix[k] < 0) ix[k] = 0;
	if(ix[k] >= image->width) ix[k] = image->width-1;

	/* compute iy[i], the top side of a block centered on a->y[i]
	 */
	for(j = j1, k = 0; j <= j2; j++, k++)
	{
	    if(j == 0) {
		iy[k] = unscale_y(d, a->y[j] - .5*(a->y[j+1] - a->y[j]));
	    }
	    else {
		iy[k] = unscale_y(d, .5*(a->y[j-1] + a->y[j]));
	    }
	    if(iy[k] < 0) iy[k] = 0;
	    if(iy[k] >= image->height) iy[k] = image->height-1;
	}
	if(j2 == a->ny-1) {
	    iy[k] = unscale_y(d, a->y[j2] + .5*(a->y[j2] - a->y[j2-1]));
	}
	else {
	    iy[k] = unscale_y(d, .5*(a->y[j2] + a->y[j2+1]));
	}
	if(iy[k] < 0) iy[k] = 0;
	if(iy[k] >= image->height) iy[k] = image->height-1;

	/* Compute the sub-rectangle of the image that will be drawn.
	 */
	image_x1 = ix[0];
	image_x2 = ix[i2-i1] + abs(ix[i2-i1+1] - ix[i2-i1]);
	image_y1 = iy[j2-j1+1];
	image_y2 = iy[1] + abs(iy[0] - iy[1]);
	*x0 = (image_x1 < image_x2) ? image_x1 : image_x2;
	*xwidth = (int)(fabs(image_x2 - image_x1));
	*y0 = (image_y1 < image_y2) ? image_y1 : image_y2;
	*yheight = (int)(fabs(image_y2 - image_y1));

	amin = amax = Z(i1, j1);
	a->imin = a->imax = i1;
	a->jmin = a->jmax = j1;

	last_x = -1;
	prev_pixel = None;
	last_aij = -1;

	for(i = i1; i <= i2; i++)
	{
	    x = ix[i-i1];
	    width = abs(ix[i-i1+1] - ix[i-i1]) + 1;
	    if(x == last_x && applyAntiAlias) 
	    {
		antiAlias(a, x, iy, i, width, j1, j2, yColors, num_colors,
			colors, no_data_color, lines, image,
			lowerOutlier, upperOutlier);
		continue;
	    }
	    last_x = x;

	    /* draw all no-data pixels
	     */
	    last_y = -1;
	    for(j = j1; j <= j2; j++) if((aij = Z(i,j)) == a->exc)
	    {
		y = iy[j-j1+1];
		if(y > iy[j-j1]) y = iy[j-j1];
		if(y == last_y) continue;
		last_y = y;
		height = abs(iy[j-j1] - iy[j-j1+1]) + 1;

		yColors[j-j1] = no_data_color;
		for(l = 0; l < height; l++) {
		    for(o = 0; o < width; o++) {
			XPutPixel(image, x+o, y+l, no_data_color);
		    }
		}
	    }

	    /* draw all data pixels
	     */
	    last_y = -1;
	    for(j = j1; j <= j2; j++) if((aij = Z(i,j)) != a->exc)
	    {
		y = iy[j-j1+1];
		if(y > iy[j-j1]) y = iy[j-j1];
		height = abs(iy[j-j1] - iy[j-j1+1]) + 1;
		if(y == last_y) continue;
		last_y = y;


		if(prev_pixel >= 0 && aij == last_aij) {
		    this_pixel = prev_pixel;
		}
		else
		{
		    this_pixel = None;
		    if(aij <= lines[0]) {
			this_pixel = lowerOutlier;
		    }
		    else if(aij >= lines[num_colors]) {
			this_pixel = upperOutlier;
		    }
		    else 
		    {
			klower = -1; kupper = num_colors+1;
			while(kupper - klower > 1)
			{
			    kmiddle = (klower + kupper)/2;
			    if(aij > lines[kmiddle]) {
				klower = kmiddle;
			    }
			    else {
				kupper = kmiddle;
			    }
			}
			this_pixel = colors[klower];
		    }
		}
		prev_pixel = this_pixel;
		last_aij = aij;

		if(this_pixel >= 0)
		{
		    yColors[j-j1] = this_pixel;

		    /* check if the entry should be selected
		    */
/*
		    if( select_val && select_val[i+j*a->nx] )
		    {
		        for(l = 0; l < height; l++) {
			    for(o = 0; o < width; o++) {
			        XPutPixel(image, x+o, y+l, select_color);
		 	    }
		        }
		    }
*/
		    /* check if the entry should be dimmed
		    */
//		    else if (dim_val && dim_val[i+j*a->nx])
		    if( (dim_val && dim_val[i+j*a->nx]) ||
			(select_val && select_val[i+j*a->nx]))
		    {
		        for(l = 0; l < height; l++) {
			    for(o = 0; o < width; o++) {
				if (l % 2 == 0 && o % 2 == 0)
				{
			            XPutPixel(image, x+o, y+l, select_color);
				}
				else
				{
			            XPutPixel(image, x+o, y+l, this_pixel);
				}
		 	    }
		        }
		    }
		    else
		    {
		        for(l = 0; l < height; l++) {
			    for(o = 0; o < width; o++) {
			        XPutPixel(image, x+o, y+l, this_pixel);
		 	    }
		        }
		    }
		}

		if(amin > aij) {
		    amin = aij;
		    a->imin = i;
		    a->jmin = j;
		}
		if(amax < aij) {
		    amax = aij;
		    a->imax = i;
		    a->jmax = j;
		}
	    }
	}
	a->z_min = a->z[a->imin + a->jmin*a->nx];
	a->z_max = a->z[a->imax + a->jmax*a->nx];
	if(ix != NULL) free(ix);
	if(iy != NULL) free(iy);
	if(yColors != NULL) free(yColors);
	
	return;
}

static void
antiAlias(Matrx *a, int x, int *iy, int i, int width, int j1, int j2,
	Pixel *yColors, int num_colors, Pixel *colors, Pixel no_data_color,
	double *lines, XImage *image, Pixel lowerOutlier, Pixel upperOutlier)
{
        Pixel   this_pixel;
	int	j;
	int	y, klower, kupper, kmiddle;
	int	o, l;
	int	height;
	int	last_y=0;
	float	aij;

	for(j = j1; j <= j2; j++) if((aij = Z(i,j)) != a->exc)
	{
	    if (yColors[j-j1] == no_data_color)
	    {
		y = iy[j-j1+1];
		if(y > iy[j-j1]) y = iy[j-j1];
		if(y == last_y) continue;
		last_y = y;
		height = abs(iy[j-j1] - iy[j-j1+1]) + 1;

		    this_pixel = None;
		    if(aij <= lines[0]) {
			this_pixel = lowerOutlier;
		    }
		    else if(aij >= lines[num_colors]) {
			this_pixel = upperOutlier;
		    }
		    else 
		    {
			klower = -1; kupper = num_colors+1;
			while(kupper - klower > 1)
			{
			    kmiddle = (klower + kupper)/2;
			    if(aij > lines[kmiddle]) {
				klower = kmiddle;
			    }
			    else {
				kupper = kmiddle;
			    }
			}
			this_pixel = colors[klower];
		    }

		if(this_pixel >= 0)
		{
		    yColors[j-j1] = this_pixel;

		    for(l = 0; l < height; l++) {
		        for(o = 0; o < width; o++) {
			    XPutPixel(image, x+o, y+l, this_pixel);
		        }
		    }
		}
	    }
	}
}

/*
   find the element of xg(1 to n) whose value is nearest to x.
*/
static int
ndex(double x, int n, double *xg)
{
   int i,nd;
   float difmin,dif;

   nd = 1;
   difmin = fabs(x-xg[0]);
   for(i=1; i<n; i++){
      dif = fabs(x-xg[i]);
      if(dif < difmin){
         difmin = dif;
         nd = i+1;
      }
   }
   return(nd);
}
