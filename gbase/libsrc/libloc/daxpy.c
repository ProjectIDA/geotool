#include "config.h"
#include <stdio.h>
#include <math.h>
#include "libloc.h"

/*
c NAME
c	daxpy -- Multiply constant to vector and add another vector.

c FILE
c	daxpy.f

c SYNOPSIS
c	LINPACK constant times a vector plus a vector routine.

c DESCRIPTION
c	Subroutine.  Given a vector dx() of length n, multiply a constant,
c	da, then add to vector dy().  Typically used in applying 
c	transformations, often in conjunction with subroutine ddot.

c	---- On entry ----
c	n: Length of vector
c	incx:	x-increment loop counter (= 1, if entire loop accessed)
c	incy:	y-increment loop counter (= 1, if entire loop accessed)
c	da:	Scalar constant ultimately multiplied to dx()
c	dx():	Vector to which constant is multiplied 
c	dy():	Vector added to dx()

c	---- On return ----
c	dy():	New vector to which original dy() is added

c DIAGNOSTICS
c       

c NOTES
c	Uses unrolled loops for increments equal to 1.

c SEE ALSO
c	LINPACK documentation by John Dongarra.

c AUTHOR
c	John Dongarra, March 1978.


      subroutine daxpy (n, da, dx, incx, dy, incy)
*/

#define mod(i,j) (i-((i)/(j))*(j))

void
daxpy(int n, double da, double *dx, int incx, double *dy, int incy)
{

/**
c     ---- On entry ----

      integer*4 incx, incy, n
      real*8    da, dx(1)
 
c     ---- On entry and return ----

      real*8    dy(1)

c     ---- Internal variables ----

      integer*4 i, ix, iy, m, mp1
**/
	int i, ix, iy, m;

	if(n <= 0) return;
	if(da == 0.0) return;
/**
      if (n  .le. 0)     return
      if (da .eq. 0.0d0) return
      if (incx.eq.1 .and. incy.eq.1) goto 1010
 
c     Code for unequal increments or equal increments not equal to 1 
 
      ix = 1
      iy = 1
      if (incx .lt. 0) ix = (-n+1)*incx + 1
      if (incy .lt. 0) iy = (-n+1)*incy + 1
      do 1000 i = 1, n
         dy(iy) = dy(iy) + da*dx(ix)
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
		dy[iy] += da*dx[ix];
		ix += incx;
		iy += incy;
	    }
	    return;
	}
 
/**
c     Code for both increments equal to 1
 
c     Clean-up loop
 
 1010 m = mod(n,4)
      if (m .eq. 0) goto 1030
      do 1020 i = 1, m
 1020 dy(i) = dy(i) + da*dx(i)
      if (n .lt. 4) return
 1030 mp1 = m + 1
      do 1040 i = mp1, n, 4
         dy(i)     = dy(i) + da*dx(i)
         dy(i + 1) = dy(i + 1) + da*dx(i + 1)
         dy(i + 2) = dy(i + 2) + da*dx(i + 2)
         dy(i + 3) = dy(i + 3) + da*dx(i + 3)
 1040 continue
      return
**/
	m = mod(n, 4);
	if(m != 0)
	{
	    for(i = 0; i < m; i++)
	    {
		dy[i] += da*dx[i];
	    }
	    if(n < 4) return;
	}
	for(i = m; i < n; i += 4)
	{
	    dy[i]   += da*dx[i];
	    dy[i+1] += da*dx[i+1];
	    dy[i+2] += da*dx[i+2];
	    dy[i+3] += da*dx[i+3];
	}
}
