#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libloc.h"

/*
c NAME
c	drot -- Apply a plane rotation.

c FILE
c	drot.f

c SYNOPSIS
c	LINPACK routine which applies a simple plane rotation.

c DESCRIPTION
c	Subroutine.  Given two vectors dx() and dy() of length n, simply 
c	apply a plane rotation and return in the same two vectors.

c	---- On entry ----
c	n:	Length of vector
c	incx:	x-increment loop counter (= 1, if entire loop accessed)
c	incy:	y-increment loop counter (= 1, if entire loop accessed)
c	dx():	Original unrotated vector
c	dy():	Original unrotated vector

c	---- On return ----
c	dx():	New vector dx() in rotated system
c	dy():	New vector dy() in rotated system

c DIAGNOSTICS
c

c NOTES
c	Uses unrolled loops for increments equal to 1.

c SEE ALSO
c	LINPACK documentation by John Dongarra.

c AUTHOR
c	John Dongarra, March 1978.


      subroutine drot (n, dx, incx, dy, incy, c, s)
*/

void
drot (int n, double *dx, int incx, double *dy, int incy, double c, double s)
{

/**
c     ---- On entry ----

      integer*4 incx, incy, n
      real*8    c, s
 
c     ---- On entry and return ----

      real*8    dx(1), dy(1)

c     ---- Internal variables ----

      integer*4 i, ix, iy
      real*8    dtemp

      if (n .le. 0) return
**/
	int	i, ix, iy;
	double	dtemp;

	if(n <= 0) return;
/**
      if (incx.eq.1 .and. incy.eq.1) goto 1010
 
c     Code for unequal increments or equal increments not equal to 1
 
      ix = 1
      iy = 1
      if (incx .lt. 0) ix = (-n+1)*incx + 1
      if (incy .lt. 0) iy = (-n+1)*incy + 1
      do 1000 i = 1, n
         dtemp  = c*dx(ix) + s*dy(iy)
         dy(iy) = c*dy(iy) - s*dx(ix)
         dx(ix) = dtemp
         ix = ix + incx
         iy = iy + incy
 1000 continue
      return
**/
	if(incx != 1 || incy != 1)
	{
	    ix = 0;
	    iy = 0;
	    if(incx < 0) ix = (-n+1)*incx;
	    if(incy < 0) iy = (-n+1)*incy;
	    for(i = 0; i < n; i++)
	    {
		dtemp  = c*dx[ix] + s*dy[iy];
		dy[iy] = c*dy[iy] - s*dx[ix];
		dx[ix] = dtemp;
		ix += incx;
		iy += incy;
	    }
	    return;
	} 

/**
c     Code for both increments equal to 1
 
 1010 do 1020 i = 1, n
         dtemp = c*dx(i) + s*dy(i)
         dy(i) = c*dy(i) - s*dx(i)
         dx(i) = dtemp
 1020 continue
      return
**/

	for(i = 0; i < n; i++)
	{
	    dtemp = c*dx[i] + s*dy[i];
	    dy[i] = c*dy[i] - s*dx[i];
	    dx[i] = dtemp;
	}
}
