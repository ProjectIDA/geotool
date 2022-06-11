/*
 * NAME
 *      routines for contouring
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "libgplot.h"
#include "libgmath.h"
#include "libstring.h"
#include "alloc.h"

static void contur2(DrawStruct *d, char *lab, float label_size, int n1, int nx,
			int ny, float *a, double *xg, double *yg, float exc,
			double cl, int nden);
static void trace2(int n1, int nx, int ny, float *a, double *xg, double *yg,
			float exc, int *hor, int *ver, int *isid1, int *is,
			int *js, double con, int nden, int maxn, int *n,
			double *x, double *y);
static void revers(double *x, int n);
static void pltcon(DrawStruct *d, char *lab, float label_size, int n,
			double *x, double *y);
static int ndex(double x, int n, double *xg);


void
condata(DrawStruct *d,
	float label_size,
	Matrx	*a,
	char auto_ci,
	float *ci, float cmin, float cmax)
{
	int i1, i2, j1, j2, mx, my;
	double xmin, xmax, ymin, ymax;

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
	mx = i2 - i1 + 1;
	my = j2 - j1 + 1;

	/* set the draw limits to prevent contour2 from drawing 
	 * outside x1,x2,y1,y2.
	 */
	xmin = a->x1 - 1.e-4*(a->x2 - a->x1);
	xmax = a->x2 + 1.e-4*(a->x2 - a->x1);
	ymin = a->y1 - 1.e-4*(a->y2 - a->y1);
	ymax = a->y2 + 1.e-4*(a->y2 - a->y1);
	SetClipArea(d, xmin, ymin, xmax, ymax);
   
	contour2(d, label_size, a->nx, mx, my, &a->z[i1+j1*a->nx], &a->x[i1],
		&a->y[j1], a->exc, auto_ci, ci, cmin, cmax,
		&a->imin, &a->jmin, &a->imax, &a->jmax);

	a->imin += i1;
	a->jmin += j1;
	a->imax += i1;
	a->jmax += j1;
	a->z_min = a->z[a->imin + a->jmin*a->nx];
	a->z_max = a->z[a->imax + a->jmax*a->nx];

	/* return to the default draw limits.
	 */
	SetClipArea(d, 0., 0., 0., 0.);

	return;
}

#define A(i,j) a[i+(j)*n1]

/*
   ------------------------------------------------------------------
   plot the contours of matrix with contour interval ci. the zero
   contour is not plotted.

   n1   = input first dimension of array a(i,j)
   nx   = input number of grid points in the i/x-direction.
   ny   = input number of grid points in the j/y-direction.
   a    = input matrix of values to contour. (a(i,j),i=1,nx),j=1,ny)
   xg   = input nx grid point x-coordinates.
   yg   = input ny grid point y-coordinates.
   exc  = input exclusion value: elements of a = exc are excluded.
   ci   = input contour interval.
   cmin = input: if cmin != cmax, draw contours only between the
   cmax = input: limits cmin to cmax.
   imin = output index of the minimum value of a in the region contoured.
   jmin = output index of the minimum value of a in the region contoured.
   imax = output index of the maximum value of a in the region contoured.
   jmax = output index of the maximum value of a in the region contoured.
   ------------------------------------------------------------------
*/
#define nsizes 6

void
contour2(DrawStruct *d, float label_size, int n1, int nx, int ny, float *a,
	double *xg, double *yg, float exc, char auto_ci, float *ci, float cmin,
	float cmax, int *imin, int *jmin, int *imax, int *jmax)
{
	int i, j, ndigit, ndeci, nmin, nden;
	char lab[80];
	static int   nsiz[nsizes] = { 2,  5, 20, 30, 40, 50};
	static int ndense[nsizes] = {20, 10,  5,  4,  3,  2};
	double tp[10], aij, c1, c2;
	double cstart, ci2, con, amin, amax;

	nmin = (nx < ny) ? nx : ny;
	if(nmin <= 1)
	{
		return;
	}

	/* Get the data min and max for the region to be contoured.
	 */
	for(j = 0, amin = amax = exc; j < ny; j++)
	{
		for(i = 0; i < nx; i++)
		{
			if(A(i,j) != exc)
			{
				amin = amax = A(i,j);
				*imin = *imax = i;
				*jmin = *jmax = j;
				break;
			}
		}
		if(i < nx) break;
	}
	for(j = 0; j < ny; j++)
	{
		for(i = 0; i < nx; i++)
		{
			if((aij = A(i,j)) != exc)
			{
				if(amin > aij)
				{
					amin = aij;
					*imin = i;
					*jmin = j;
				}
				if(amax < aij)
				{
					amax = aij;
					*imax = i;
					*jmax = j;
				}
			}
		}
	}
	if(amin == amax)
	{
		return;
	}

	if(cmin != cmax)
	{
		if(cmin < cmax)
		{
			c1 = cmin;
			c2 = cmax;
		}
		else
		{
			c1 = cmax;
			c2 = cmin;
		}
		if(amin > c1) c1 = amin;
		if(amax < c2) c2 = amax;
	}
	else
	{
		c1 = amin;
		c2 = amax;
	}

	if(auto_ci)
	{
		/* use nicex to get 5 to 10 evenly spaced contour levels 
		 * with nice values.
		 */
		nicex(c1, c2, 5, 10, &i, tp, &ndigit, &ndeci);
		*ci = tp[1] - tp[0];
		cstart = tp[0];
	}
	else
	{
		if(*ci <= 0)
		{
			return;
		}
		cstart = (int)(c1 / *ci)* *ci;
	}

	/* set the density of points drawn along the contours.
	 */
	for(i = 0, nden = 1; i < nsizes; i++)
	{
		if(nmin <= nsiz[i])
		{
			nden = ndense[i];
			break;
		}
	}

	ci2 = *ci/2.;
	for(i = 0, con = cstart; con <= c2; i++, con += *ci)
	{
/*
      if(con < 0) ipen(4);
      else ipen(6);
*/
		/* don't plot the zero contour */
		if(fabs(con) > ci2)
		{
			/* convert the contour value from float to char. */
			ftoa(con, 4, 0, lab, 80);
			contur2(d, lab, label_size, n1, nx, ny, a, xg, yg, exc,
				con, nden);
		}
	}
	return;
}

/*
   ------------------------------------------------------------------
   plot all contours at the level cl for a grid with varying grid
   point spacing in both x and y.

   n1    = input first dimension of array a(i,j)
   nx    = input number of grid points in the i/x-direction.
   ny    = input number of grid points in the j/y-direction.
   a     = input matrix of values to contour. (a(i,j),i=1,nx),j=1,ny)
   xg    = input nx grid point x-coordinates.
   yg    = input ny grid point y-coordinates.
   exc   = input exclusion value: elements of a = exc are excluded.
   cl    = input contour level.
   nden  = input density of contour points output. The density of the
           output contour points is approximately nden*nx and nden*ny.
   lab   = input character label for this contour
   label_size  = input label size for the character labels.
   ------------------------------------------------------------------
*/
#define MAXB 1600
#define MAXN 1000

static void
contur2(DrawStruct *d, char *lab, float label_size, int n1, int nx, int ny,
		float *a, double *xg, double *yg, float exc, double cl,
		int nden)
{
	int hor[MAXB], ver[MAXB];
	int nb, i, j, nx1, ny1, mx, my, nxblk, nyblk, ib, jb, i1, i2, j1, j2,
		ii, jj, iside, n, np, nx0, ny0;
	double x[MAXN], y[MAXN];
	double con;

	/* The program logic does not work if con==a[k]. Change it.
	 */
	con = cl;
	if(con == 0.) con = 1.e-60;
	for(j = 0; j < ny; j++)
	{
		for(i = 0; i < nx; i++) if(A(i,j) == con)
		{
			con *= .999999;
		}
	}

	/* check storage requirements for hor and ver.
	 */
	nb = nx*ny/32;
	if(nb*32 != nx*ny) nb++;
	nx1 = nx - 1;
	ny1 = ny - 1;
	if(nb > MAXB)
	{
		/* If there is not enough storage space to process the entire 
		 * matrix at once,  break it into blocks.
		 */
		mx = (int)(sqrt(32.*(float)MAXB));
		my = mx;
		nb = mx*my/32;
		if(nb*32 != mx*my) nb++;
		/*
		 * The blocks overlap one grid point. Thus the -1's here.
		 */
		nxblk = nx1/(mx-1);
		if(nxblk*(mx-1) != nx1) nxblk++;
		nyblk = ny1/(my-1);
		if(nyblk*(my-1) != ny1) nyblk++;
	}
	else
	{
		nxblk = 1;
		nyblk = 1;
		mx = nx;
		my = ny;
	}

   for(ib=0; ib<nxblk; ib++) for(jb=0; jb<nyblk; jb++){

      /* initialize hor and ver for this block */
      for(i=0; i<nb; i++) hor[i] = ver[i] = 0;

      /* set extreme indices for this block. the blocks overlap one point. */
      j1 = jb*(my-1);
      j2 = (j1+my-1 < ny) ? j1+my-1:ny1;
      i1 = ib*(mx-1);
      i2 = (i1+mx-1 < nx) ? i1+mx-1:nx1;
      nx0 = i2 - i1 + 1;
      ny0 = j2 - j1 + 1;
   
      /* look for all contours which cross i to i+1 segments. */
      for(j=j1; j<=j2; j++) for(i=i1; i<i2; i++){
         if(A(i,j)!=exc && A(i+1,j)!=exc && ((A(i,j)<con &&
            con<A(i+1,j)) || (A(i,j)>con && con>A(i+1,j)))){
            iside = 0;
            if(j<ny1 && A(i,j+1)!=exc && A(i+1,j+1)!=exc) iside = 1;
            else if(j>0 && A(i,j-1)!=exc && A(i+1,j-1)!=exc) iside = 3;
            if(iside!=0){
               ii = i-i1;
               jj = j-j1;
               do {
                  trace2(n1,nx0,ny0,&A(i1,j1),&xg[i1],&yg[j1],exc,hor,ver,
                      &iside,&ii,&jj,con,nden,MAXN,&n,x,y);
                  np = abs(n);
                  if(np>1)
		  {
			pltcon(d, lab, label_size, np, x, y);
		  }
                  /*
                  if n<0, then there are more points on this contour than 
                  we had space for in arrays x,y. go back to get the rest 
                  of them.
                  */
               } while(n < 0);
            }
         }
      }
   
      /* look for all contours which cross j to j+1 segments only. */
      for(j=j1; j<j2; j++) for(i=i1; i<=i2; i++){
         if(A(i,j)!=exc && A(i,j+1)!=exc && ((A(i,j)<con &&
            con<A(i,j+1)) || (A(i,j)>con && con>A(i,j+1)))){
            iside = 0;
            if(i<nx1 && A(i+1,j)!=exc && A(i+1,j+1)!=exc) iside = 4;
            else if(i>0 && A(i-1,j)!=exc && A(i-1,j+1)!=exc) iside = 2;
            if(iside!=0){
               ii = i-i1;
               jj = j-j1;
               do {
                  trace2(n1,nx0,ny0,&A(i1,j1),&xg[i1],&yg[j1],exc,hor,ver,
                         &iside,&ii,&jj,con,nden,MAXN,&n,x,y);
                  np = abs(n);
                  if(np>1) pltcon(d, lab, label_size, np, x, y);
               } while(n < 0);
            }
         }
      }
   }
}

/*
   ------------------------------------------------------------------
   trace the contour with value con, starting at side iside,is,js.
   return updated arrays hor,ver and n points tracing the contour in
   x,y.  con must not be equal to any elements of a.

   nx    = input number of elements in the i/x-direction.
   ny    = input number of elements in the j/y-direction.
   a     = input matrix to be contoured.
   xg    = input coordinates of elements in the i/x-direction.
   yg    = input coordinates of elements in the j/y-direction.
   exc   = input exclusion value: elements of a = exc are excluded.
   hor   = input,output integer array of length np which records all
           horizontal (i to i+1) segments crossed by the contour.
   ver   = input,output integer array of length np which records all
           vertical (j to j+1) segments crossed by the contour.
           np = the smallest integer >= nx*ny/32.
   isid1 = input starting side, 1,2,3 or 4.  (see comments below)
   is,js = input starting side indices.
           note that isid1,is,js must be pointers as they are also
           returned values if n is returned < 0.
   con   = input contour value.
   nden  = input density of contour points output. The density of the
           output contour points is approximately nden*nx and nden*ny.
   maxn  = input maximum storage available for hor and ver.
   n     = output number of points tracing the contour. if n < 0,
           then there was not enough space in arrays x,y for all the
           contour points. iabs(n) contour points are returned in x,y,
           and the contour can be continued with a to trace with
           is,js,iside unchanged from the output values.
   x     = output n x-coordinates of the contour points.
   y     = output n y-coordinates of the contour points.
   ------------------------------------------------------------------
*/
static void
trace2(int n1, int nx, int ny, float *a, double *xg, double *yg, float exc,
    int *hor, int *ver, int *isid1, int *is, int *js, double con,
    int nden, int maxn, int *n, double *x, double *y)
{
   /*
   hor and ver are used bitwise to record the crossing of a segment.
   bits 1 to 32 of each 4-byte element are used.
   ibit[i] : only the ith bit is set.
   */
   static unsigned int ibit[32] = 
      {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400,
       0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000,
       0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000,
       0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000 };

   int i,j,k,nx1,ny1,iside,n32,done,ifirst,ileft,irite,itop,ibot;
   float fnden,dx,dy,xx,yy,xi,yj,abot1,abot2,atop1,atop2,da1,da2,a1,a2,
         ddx,ddy,dx2,dy2;
   /*
      for the grid box below:

             (i,j+1) ---- (i+1,j+1)
               |             |
               |             |
             (i,j) ------ (i+1,j) 

      iside = 1 : the trace is entering the box between (i,j), (i+1,j).
      iside = 4 : the trace is entering the box between (i,j), (i,j+1).

      for the grid box below:

             (i-1,j+1) --- (i,j+1)
               |             |
               |             |
             (i-1,j) ----- (i,j)
                  
      iside = 2 : the trace is entering the box between (i,j), (i,j+1).

      for the grid box below:

             (i,j) ------- (i+1,j)
               |             |
               |             |
             (i,j-1) ----- (i+1,j-1)
                  
      iside = 3 : the trace is entering the box between (i,j), (i+1,j).
   */

   i = *is;
   j = *js;
   nx1 = nx - 1;
   ny1 = ny - 1;
   iside = *isid1;
   fnden = (float)nden;

   k = i + j*nx;
   n32 = k/32;
   k = k - n32*32;
   *n = 0;
   done = 0;
   if(iside==1 || iside==3){
      dx = xg[i+1] - xg[i];
      xx = (A(i,j)-con)*dx/(A(i,j)-A(i+1,j));
      yy = 0.;
      x[0] = xg[i] + xx;
      y[0] = yg[j];
      *n = *n + 1;
      if((hor[n32] & ibit[k]) != 0 ) done = 1;
      else hor[n32] = hor[n32] | ibit[k];
   }
   else {
      dy = yg[j+1] - yg[j];
      xx = 0.;
      yy = (A(i,j)-con)*dy/(A(i,j)-A(i,j+1));
      x[0] = xg[i];
      y[0] = yg[j] + yy;
      *n = *n + 1;
      if((ver[n32] & ibit[k]) != 0 ) done = 1;
      else ver[n32] = ver[n32] | ibit[k];
   }
   ifirst = 1;
   while(!done){
      xi = xg[i];
      yj = yg[j];
      if(iside==1 || iside==3){
         dx = xg[i+1] - xg[i];
         abot1 = A(i,j);
         abot2 = A(i+1,j);
         if(iside==1){
            dy = yg[j+1] - yg[j];
            ddy = dy/fnden;
            atop1 = A(i,j+1);
            atop2 = A(i+1,j+1);
         }
         else {
            dy = yg[j] - yg[j-1];
            ddy = -dy/fnden;
            atop1 = A(i,j-1);
            atop2 = A(i+1,j-1);
         }
         dx2 = dx/2.;
         dy2 = dy/2.;
         da1 = (atop1-abot1)/fnden;
         da2 = (atop2-abot2)/fnden;
         ileft = 0;
         irite = 0;
         k = 1;
         do {
            if(*n >= maxn){
               *n = -*n; *is = i; *js = j; *isid1 = iside;
               return;
            }
            a1 = abot1 + k*da1;
            a2 = abot2 + k*da2;
            if((con>abot1 && con<=a1) || (con<abot1 && con>=a1)){
               /* con crosses at left side */
               ileft = 1;
            }
            if((con>abot2 && con<=a2) || (con<abot2 && con>=a2)){
               /* con crosses at right side */
               if(ileft==1){
                  /* con crosses at left and right side. there are two contour
                     lines through this box. join up with the closest one. */
                  if(xx > dx2){
                     irite = 1;
                     ileft = 0;
                  }
               }
               else irite = 1;
            }
            if(ileft==1){
               /* con crosses at left side */
               xx = 0.;
               if(iside==1) yy = (con-abot1)*dy/(atop1-abot1);
               else {
                  yy = -(con-abot1)*dy/(atop1-abot1);
                  j = j - 1;
               }
               iside = 2;
            }
            else if(irite==1){
               /* con crosses at right side */
               xx = dx;
               if(iside==1) yy = (con-abot2)*dy/(atop2-abot2);
               else {
                  yy = -(con-abot2)*dy/(atop2-abot2);
                  j = j - 1;
               }
               i = i + 1;
               iside = 4;
            }
            else {
               xx = (con-a1)*dx/(a2-a1);
               yy = k*ddy;
            }
            x[*n] = xi + xx;
            y[*n] = yj + yy;
            *n = *n + 1;
            k++;
            /* the following check will not work if dx < 0. */
         } while(xx>0. && xx<dx && k<=nden);
         if(iside==1) j = j + 1;
         else if(iside==3) j = j - 1;
      }
      else {
         /* iside = 2 or iside = 4 */
         dy = yg[j+1] - yg[j];
         abot1 = A(i,j);
         abot2 = A(i,j+1);
         if(iside==4){
            dx = xg[i+1] - xg[i];
            ddx = dx/fnden;
            atop1 = A(i+1,j);
            atop2 = A(i+1,j+1);
         }
         else {
            dx = xg[i] - xg[i-1];
            ddx = -dx/fnden;
            atop1 = A(i-1,j);
            atop2 = A(i-1,j+1);
         }
         dx2 = dx/2.;
         dy2 = dy/2.;
         da1 = (atop1-abot1)/fnden;
         da2 = (atop2-abot2)/fnden;
         itop = 0;
         ibot = 0;
         k = 1;
         do {
            if(*n >= maxn){
               *n = -*n; *is = i; *js = j; *isid1 = iside;
               return;
            }
            a1 = abot1 + k*da1;
            a2 = abot2 + k*da2;
            if((con>abot1 && con<=a1) || (con<abot1 && con>=a1)){
               /* con crosses at bottom side */
               ibot = 1;
            }
            if((con>abot2 && con<=a2) || (con<abot2 && con>=a2)){
               /* con crosses at top side */
               if(ibot==1){
                  /* con crosses at bottom and top. there are two contour 
                     lines through this box. join up with the closest one. */
                  if(yy > dy2){
                     itop = 1;
                     ibot = 0;
                  }
               }
               else itop = 1;
            }
            if(ibot==1){
               /* con crosses at bottom side */
               if(iside==4) xx =  (con-abot1)*dx/(atop1-abot1);
               else {
                  xx = -(con-abot1)*dx/(atop1-abot1);
                  i = i - 1;
               }
               yy = 0.;
               iside = 3;
            }
            else if(itop==1){
               /* con crosses at top side */
               if(iside==4) xx =  (con-abot2)*dx/(atop2-abot2);
               else {
                  xx = -(con-abot2)*dx/(atop2-abot2);
                  i = i - 1;
               }
               yy = dy;
               j = j + 1;
               iside = 1;
            }
            else {
               xx = k*ddx;
               yy = (con-a1)*dy/(a2-a1);
            }
            x[*n] = xi + xx;
            y[*n] = yj + yy;
            *n = *n + 1;
            k++;
         } while(yy>0. && yy<dy && k<=nden);
         if(iside==2) i = i - 1;
         else if(iside==4) i = i + 1;
      }
      k = i + j*nx;
      n32 = k/32;
      k = k - n32*32;
      if(iside==1){
         if((hor[n32] & ibit[k]) != 0) done = 1;
         else {
            hor[n32] = hor[n32] | ibit[k];
            if(j==ny1 || A(i,j+1)==exc || A(i+1,j+1)==exc) done = 1;
         }
      }
      else if(iside==2){
         if((ver[n32] & ibit[k]) != 0) done = 1;
         else {
            ver[n32] = ver[n32] | ibit[k];
            if(i==0 || A(i-1,j)==exc || A(i-1,j+1)==exc) done = 1;
         }
      }
      else if(iside==3){
         if((hor[n32] & ibit[k]) != 0) done = 1;
         else {
            hor[n32] = hor[n32] | ibit[k];
            if(j==0 || A(i,j-1)==exc || A(i+1,j-1)==exc) done = 1;
         }
      }
      else {
         if((ver[n32] & ibit[k]) != 0) done = 1;
         else {
            ver[n32] = ver[n32] | ibit[k];
            if(i==nx1 || A(i+1,j)==exc || A(i+1,j+1)==exc) done = 1;
         }
      }
      if(done && ifirst==1 && (i!= *is || j!= *js)){
         ifirst = 0;
         iside = 0;
         if(*isid1==1 && *js!=0) iside = 3;
         else if(*isid1==2 && *is!=nx1) iside = 4;
         else if(*isid1==3 && *js!=ny1) iside = 1;
         else if(*isid1==4 && *is!=0)  iside = 2;
         if(iside!=0){
            i = *is;
            j = *js;
            revers(x,*n);
            revers(y,*n);
            done = 0;
         }
      }
   }
}

/*
   ------------------------------------------------------------------
   reverse the order of the n elements in x.
   ------------------------------------------------------------------
*/
static void
revers(double *x, int n)
{
   int i,n1,n2;
   double temp;

   n1 = n - 1;
   n2 = n/2;
   for(i=0; i<n2; i++){
      temp = x[i];
      x[i] = x[n1-i];
      x[n1-i] = temp;
   }
}

/*
   ------------------------------------------------------------------
   plot the points (x[i],y[i],i=0,n-1) connected by line segments.
   label the curve with the label lab, with font font.

   n     = input number of points to plot.
   x,y   = input n x and y coordinates respectively.
   lab   = input character label.
   nl    = input number of characters in lab.
   font  = input font number for the character labels.
   ------------------------------------------------------------------
*/
#define mod(i,j) (i-((i)/(j))*(j))
#define sign(x) (((x) >= 0.) ? 1 : -1)

static void
pltcon(DrawStruct	*d,
	char 		*lab,
	float		label_size,
	int		n,
	double		*x, 
	double		*y)
{
	static int call = 0;
	int i, a, char_width, label_width, ix1, iy1, ix2, iy2,
		ixmax, iymax, end, beg=0, nl;
	float dis, xdif, ydif, angle, ux, uy, du;
	double x2, y2;

	if( n <= 1)
	{
		return;
	}
   	nl = (int)strlen(lab);
	char_width = (int)(1.5*label_size);
	label_width = (nl+2)*char_width;

	/* if this is a small curve compared to the label, don't label it. */
	ix1 = unscale_x(d, x[0]);
	iy1 = unscale_y(d, y[0]);
	ix2 = unscale_x(d, x[1]);
	iy2 = unscale_y(d, y[1]);
	ixmax = abs(ix2 - ix1);
	iymax = abs(iy2 - iy1);
	for(i = 2; i < n; i++)
	{
		ix2 = unscale_x(d, x[i]);
		iy2 = unscale_y(d, y[i]);
		ixmax = (ixmax > (a = abs(ix2-ix1))) ? ixmax : a;
		iymax = (iymax > (a = abs(iy2-iy1))) ? iymax : a;
	}
	end = -1;
	if(ixmax >= 2*label_width || iymax >= 2*label_width)
	{
		/*
		 * label the curve in the a different spot than the previous
		 * call, since curves of consecutive calls are likely to be
		 * close.
		 */
		call = mod(call+1,4);
		/* beg = n/5, or 2*n/5, or 3*n/5, or 4*n/5 
		 */
		beg = (1 + call)*n/5;
		ix1 = unscale_x(d, x[beg]);
		iy1 = unscale_y(d, y[beg]);
		for(i = beg+1; i < n; i++)
		{
			ix2 = unscale_x(d, x[i]);
			iy2 = unscale_y(d, y[i]);
			dis = sqrt(pow((float)(ix2-ix1),2.) +
					pow((float)(iy2-iy1),2.));
			if(dis >= label_width)
			{
				end = i;
				break;
			}
		}
	}

	imove(d,x[0],y[0]);
	if(end>0)
	{
		/* get the angle, counter-clockwise from the x-axis.
		 */
		xdif = abs(ix2 - ix1)*sign(x[end]-x[beg]);
		ydif = abs(iy2 - iy1)*sign(y[end]-y[beg]);
		angle = 180.*atan2(ydif, xdif)/3.14159265;
		if(xdif < 0.)
		{
			i = ix1;
			ix1 = ix2;
			ix2 = i;
			i = iy1;
			iy1 = iy2;
			iy2 = i;
			angle = angle - 180.;
		}
		if(fabs(angle) < 5.) angle = 0.;
		else if(fabs(angle-90.) < 5.) angle = 90.;
		else if(fabs(angle+90.) < 5.) angle = -90.;
		else if(fabs(angle-180.) < 5.) angle = 180.;

		for(i = 1; i <= beg; i++) idraw(d, x[i], y[i]);
		/*
		 * shift ix1,iy1 a little in the direction of (ix2,iy2)
		 */
		ux = (ix2 - ix1);
		uy = (iy2 - iy1);
		du = sqrt(ux*ux + uy*uy);
		ux = ux/du;
		uy = uy/du;
		/* shift over one character width */
		ix1 = (int)(ix1 + char_width*ux);
		iy1 = (int)(iy1 + char_width*uy);
		ix2 = (int)(ix1 + (nl+1)*char_width*ux);
		iy2 = (int)(iy1 + (nl+1)*char_width*uy);
		x2 = scale_x(d, ix2);
		y2 = scale_y(d, iy2);
		if(xdif < 0.) idraw(d,x2,y2);
		ALLOC((d->l->n_ls+1)*sizeof(LabelStruct), d->l->size_ls,
			d->l->ls, LabelStruct, 0);
		stringcpy(d->l->ls[d->l->n_ls].label, lab,
			sizeof(d->l->ls[d->l->n_ls].label));
		d->l->ls[d->l->n_ls].x = ix1;
		d->l->ls[d->l->n_ls].y = iy1;
		d->l->ls[d->l->n_ls].size = label_size;
		d->l->ls[d->l->n_ls].angle = angle;
		d->l->n_ls++;
/*
	mapalf(XtDisplay(w), XtWindow(w), w->con_plot.labelGC, ix1, iy1,
		2, angle, lab);
*/

		if(xdif < 0.)
		{
			imove(d, x[end], y[end]);
			end++;
		}
		else imove(d, x2, y2);
		for(i = end; i < n; i++) idraw(d, x[i], y[i]);
 	}
	else
	{
		/* there is not enough room for a label. just plot the curve 
		 */
		for(i = 1; i < n; i++) idraw(d, x[i], y[i]);
	}
	iflush(d);
}   

/*      
   find the element of xg(1 to n) whose value is nearest to x.
*/
static int
ndex(double x, int n, double *xg)
{
    int i, nd;
    float difmin, dif;

    nd = 1;
    difmin = fabs(x-xg[0]);
    for(i=1; i < n; i++)
    {
	dif = fabs(x-xg[i]);
	if(dif < difmin) {
	    difmin = dif;
	    nd = i+1;
	}
    }
    return(nd);
}
