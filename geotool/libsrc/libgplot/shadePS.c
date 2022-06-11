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

#include "libgplot.h"

#define Z(i,j) a->z[i+(j)*a->nx]

static int ndex(double x, int n, double *xg);

void
shadePS(FILE *fp, DrawStruct *d, int num_colors, double *lines, Matrx *a,
	int xmin, int xmax, int ymin, int ymax)
{
	int i, j, k, width, height, klower, kupper, kmiddle, last_k;
	int i1, i2, j1, j2, x, y, *ix = NULL, *iy = NULL, last_x;
	float aij, amin, amax;

	if(num_colors <= 0) {
	    return;
	}
	fprintf(fp, "/f { x 4 1 roll rectfill } def\n");

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
	for(i = i1, k = 0; i <= i2; i++, k++)
	{
	    if(i == 0) {
		ix[k] = unscale_x(d, (a->x[i] -.5*(a->x[i+1]-a->x[i])));
	    }
	    else {
		ix[k] = unscale_x(d, .5*(a->x[i-1] + a->x[i]));
	    }
	    if(ix[k] < xmin) ix[k] = xmin;
	    if(ix[k] > xmax) ix[k] = xmax;
	}
	if(i2 == a->nx-1) {
	    ix[k] = unscale_x(d, (a->x[i2] + .5*(a->x[i2]-a->x[i2-1])));
	}
	else {
	    ix[k] = unscale_x(d, .5*(a->x[i2] + a->x[i2+1]));
	}
	if(ix[k] < xmin) ix[k] = xmin;
	if(ix[k] > xmax) ix[k] = xmax;

	for(j = j1, k = 0; j <= j2; j++, k++)
	{
	    if(j == 0) {
		iy[k] = unscale_y(d, a->y[j] - .5*(a->y[j+1]-a->y[j]));
	    }
	    else {
		iy[k] = unscale_y(d, .5*(a->y[j-1] + a->y[j]));
	    }
	    if(iy[k] < ymin) iy[k] = ymin;
	    if(iy[k] > ymax) iy[k] = ymax;
	}
	if(j2 == a->ny-1) {
	    iy[k] = unscale_y(d, a->y[j2] + .5*(a->y[j2]-a->y[j2-1]));
	}
	else {
	    iy[k] = unscale_y(d, .5*(a->y[j2] + a->y[j2+1]));
	}
	if(iy[k] < ymin) iy[k] = ymin;
	if(iy[k] > ymax) iy[k] = ymax;

	fprintf(fp, "c%d ", num_colors);
	fprintf(fp, "%d %d %d %d rectfill\n", xmin, ymin, xmax-xmin, ymax-ymin);

	amin = amax = Z(i1, j1);
	a->imin = a->imax = i1;
	a->jmin = a->jmax = j1;

	last_k = -1;
	last_x = -1;
	for(i = i1; i <= i2; i++)
	{
	    x = ix[i-i1];
	    if(x != last_x) {
		fprintf(fp, "/x %d def\n", x);
		last_x = x;
	    }
	    width = abs(ix[i-i1+1] - ix[i-i1]);
	    if ((i + 1) >= i2) width += 1;
	    for(j = j1; j <= j2; j++) if((aij = Z(i,j)) != a->exc)
	    {
		k = -1;
		if(aij <= lines[0]) {
		    k = 0;
		}
		else if(aij >= lines[num_colors]) {
		    k = num_colors-1;
		}
		else 
		{
		    if(aij >= lines[0] && aij <= lines[num_colors])
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
			k = klower;
		    }
		}
		if(k >= 0 && k < num_colors)
		{
		    y = iy[j-j1+1];
		    if(y > iy[j-j1]) y = iy[j-j1];
		    height = abs(iy[j-j1] - iy[j-j1+1]) + 1;

		    if(k != last_k) {
			fprintf(fp, "c%d ", k);
			last_k = k;
		    }
		    fprintf(fp, "%d %d %d f\n", y, width, height);
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
	
	return;
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
