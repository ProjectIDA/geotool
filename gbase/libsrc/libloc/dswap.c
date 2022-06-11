#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libloc.h"

/*
c NAME
c	dswap -- Interchange two vectors.

c FILE
c	dswap.f

c SYNOPSIS
c	LINPACK routine to interchange (swap) two vectors.

c DESCRIPTION
c	Subroutine.  Interchange (swap) vector dx() for dy() and
c	vice-a-versa.

c	---- On entry ----
c	n:	Length of vector
c	incx:	x-increment loop counter (= 1, if entire loop accessed)
c	incy:	y-increment loop counter (= 1, if entire loop accessed)
c	dx():	First  vector
c	dy():	Second vector

c	---- On return ----
c	dx():	New swaped vector, old dy() or second vector
c	dy():	New swaped vector, old dx() or first vector

c DIAGNOSTICS
c

c NOTES
c	Uses unrolled loops for increments equal to 1.

c SEE ALSO
c	LINPACK documentation by John Dongarra.

c AUTHOR
c	John Dongarra, March 1978.


      subroutine  dswap (n, dx, incx, dy, incy)
*/

#define mod(i,j) (i-((i)/(j))*(j))

void
dswap(int n, double *dx, int incx, double *dy, int incy)
{

/**
c     ---- On entry ----

      integer*4 incx, incy, n

c     ---- On entry and return ----

      real*8    dx(1), dy(1)

c     ---- Internal variables ----

      integer*4 i, ix, iy, m, mp1
      real*8    dtemp
 

      if (n.le.0) return
**/
	int	i, ix, iy, m;
	double	dtemp;

	if(n <= 0) return;

/**
      if (incx.eq.1 .and. incy.eq.1) goto 1010
 
c     Code for unequal increments or equal increments not equal to 1
 
      ix = 1
      iy = 1
      if (incx.lt.0) ix = (-n+1)*incx + 1
      if (incy.lt.0) iy = (-n+1)*incy + 1
      do 1000 i = 1, n
         dtemp = dx(ix)
         dx(ix) = dy(iy)
         dy(iy) = dtemp
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
		dtemp = dx[ix];
		dx[ix] = dy[iy];
		dy[iy] = dtemp;
		ix += incx;
		iy += incy;
	    }
	    return;
	}
 
/**
c     Code for both increments equal to 1
 
c     Clean-up loop
 
 1010 m = mod(n,3)
      if (m .eq. 0) goto 1030
      do 1020 i = 1, m
         dtemp = dx(i)
         dx(i) = dy(i)
         dy(i) = dtemp
 1020 continue
      if (n .lt. 3) return
 1030 mp1 = m + 1
      do 1040 i = mp1, n, 3
         dtemp     = dx(i)
         dx(i)     = dy(i)
         dy(i)     = dtemp
         dtemp     = dx(i + 1)
         dx(i + 1) = dy(i + 1)
         dy(i + 1) = dtemp
         dtemp     = dx(i + 2)
         dx(i + 2) = dy(i + 2)
         dy(i + 2) = dtemp
 1040 continue
**/

	m = mod(n, 3);
	if(m != 0)
	{
	    for(i = 0; i < m; i++)
	    {
		dtemp = dx[i];
		dx[i] = dy[i];
		dy[i] = dtemp;
	    }
	    if(n < 3) return;
	}
	for(i = m; i < n; i += 3)
	{
	    dtemp = dx[i];
	    dx[i] = dy[i];
	    dy[i] = dtemp;
	    dtemp = dx[i+1];
	    dx[i+1] = dy[i+1];
	    dy[i+1] = dtemp;
	    dtemp = dx[i+2];
	    dx[i+2] = dy[i+2];
	    dy[i+2] = dtemp;
	}
}
