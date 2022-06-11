#include "config.h"
#include <stdio.h>
#include <math.h>

#include "locp.h"

/*
c NAME
c	dnrm2 -- Compute the Euclidean norm of a vector.

c FILE
c	dnrm2.f

c SYNOPSIS
c	LINPACK routine to calculate the L2 (Euclidean) norm of a vector.

c DESCRIPTION
c	Function.  Given a vector dx() of length n, compute the Euclidean
c	norm and return as a scalar in dnrm2.

c	---- On entry ----
c	n:	Length of vector;  If n <= 0, return n = 0; else n = 1
c	incx:	x-storage increment counter (= 1, if entire loop accessed)
c	dx():	Vector of interest

c	---- On return ----
c	dnrm2:	Scalar Eucildean norm of dx()

c DIAGNOSTICS
c	Bases norm solution on precision of machine constants.

c NOTES
c	Uses unrolled loops for increments equal to 1.

c SEE ALSO
c	LINPACK documentation by John Dongarra.

c AUTHOR
c	C.L. Lawson, Jan. 1978.


      real*8 function dnrm2 (n, dx, incx)
*/

double
dnrm2 (int n, double *dx, int incx)
{

/**
c     ---- On entry ----

      integer*4 incx, n
      real*8    dx(1)

c     ---- On return ----

c     real*8 function dnrm2

c     ---- Internal variables ----

      integer*4 i, j, next, nn
      real*8    cuthi, cutlo, hitest, one, sum, xmax, zero

      data  zero, one /0.0d0, 1.0d0/
**/
	int	i, j, next, nn;
	double	cuthi, cutlo, hitest, one, sum, xmax=0., zero;
 
/** 
c	Four phase method     Using two built-in constants that are
c	hopefully applicable to all machines.
c	  cutlo = Maximum of  dsqrt(u/eps)	over all known machines
c	  cuthi = Minimum of  dsqrt(v)  	over all known machines
c	where,
c	  eps = Smallest no. such that eps + 1. .gt. 1.
c	  u   = Smallest positive no.	(underflow limit)
c	  v   = Largest  no.    	(overflow  limit)
 
c	Brief outline of algorithm:
 
c	Phase 1	Scans zero components
c	Move to phase 2 when a component is nonzero and .le. cutlo
c	Move to phase 3 when a component is .gt. cutlo
c	Move to phase 4 when a component is .ge. cuthi/m
c	  where, m = n for x() real and m = 2*n for complex
 
c	Values for cutlo and cuthi:
c	From the environmental parameters listed in the IMSL converter
c	document the limiting values are as follows:
c	cutlo, S.P.   u/eps = 2**(-102) for  HONEYWELL.  Close seconds 
c	are:
c		UNIVAC and DEC at 2**(-103)
c		thus cutlo = 2**(-51) = 4.44089e-16
c	cuthi, S.P.   v = 2**127 for UNIVAC, HONEYWELL, and DEC
c		thus cuthi = 2**(63.5) = 1.30438e19
c	cutlo, D.P.   u/eps = 2**(-67) for HONEYWELL and DEC
c		thus cutlo = 2**(-33.5) = 8.23181d-11
c	cuthi, D.P.   same as S.P.  cuthi = 1.30438d19
c	data cutlo, cuthi / 8.232d-11,  1.304d19 /
c	data cutlo, cuthi / 4.441e-16,  1.304e19 /

c     For SUN system computers
      data cutlo, cuthi / 8.232d-11,  1.304d19 /
**/
	cutlo = 8.232e-11;
	cuthi = 1.304e+19;
	zero = 0.0;
	one = 1.0;
 
/**
      if (n .gt. 0) goto 1000
      dnrm2 = zero
      goto 1130
**/
	if(n <= 0) return(zero);

/**
 1000 assign 1020 to next
      sum = zero
      nn  = n * incx
**/
	next = 1020;
	sum = zero;
	nn = n*incx;

/**
c     Begin main loop

      i = 1
 1010 goto next, (1020, 1030, 1060, 1070)
**/
	i = 0;
 L1010:	if(next == 1020) goto L1020;
	else if(next == 1030) goto L1030;
	else if(next == 1060) goto L1060;
	else if(next == 1070) goto L1070;

/**
 1020 if (dabs(dx(i)) .gt. cutlo) goto 1100
      assign 1030 to next
      xmax = zero
**/
 L1020:	if(fabs(dx[i]) > cutlo) goto L1100;
	next = 1030;
	xmax = zero;
 
/**
c     Phase 1.  sum is zero
 
 1030 if (dx(i) .eq. zero) goto 1120
      if (dabs(dx(i)) .gt. cutlo) goto 1100
**/
 L1030:	if(dx[i] == zero) goto L1120;
	if(fabs(dx[i]) > cutlo) goto L1100;
 
/**
c     Prepare for phase 2

      assign 1060 to next
      goto 1050
**/
	next = 1060;
	goto L1050;
 
/**
c     Prepare for phase 4
 
 1040 i = j
      assign 1070 to next
      sum  = (sum/dx(i)) / dx(i)
 1050 xmax = dabs(dx(i))
      goto 1080
**/
 L1040:	i = j;
	next = 1070;
	sum = (sum/dx[i]) / dx[i];
 L1050:	xmax = fabs(dx[i]);
	goto L1080;
 
/**
c     Phase 2.  Sum is small
c        Scale to avoid destructive underflow
 
 1060 if (dabs(dx(i)) .gt. cutlo) goto 1090
 
c     Common code for phases 2 and 4
c     In phase 4, sum is large, so scale to avoid overflow
 
 1070 if (dabs(dx(i)) .le. xmax) goto 1080
         sum  = one + sum * (xmax / dx(i))**2
         xmax = dabs(dx(i))
         goto 1120
 
 1080 sum = sum + (dx(i)/xmax)**2
      goto 1120
**/
 L1060:	if(fabs(dx[i]) > cutlo) goto L1090;

 L1070:	if(fabs(dx[i]) <= xmax) goto L1080;
	sum = one + sum * pow(xmax / dx[i], 2.);
	xmax = fabs(dx[i]);
	goto L1120;
 L1080:	sum += pow(dx[i]/xmax, 2.);
	goto L1120;
 
/**
c     Prepare for phase 3
 
 1090 sum = (sum * xmax) * xmax
 
c     For real or D.P. set hitest = cuthi/n
c     For complex      set hitest = cuthi/(2*n)
 
 1100 hitest = cuthi/float(n)
**/
 L1090:	sum = (sum * xmax) * xmax;
 L1100:	hitest = cuthi/(double)n;
 
/**
c     Phase 3:  sum is mid-range, no scaling
 
      do 1110 j = i, nn, incx
         if (dabs(dx(j)) .ge. hitest) goto 1040
 1110 sum   = sum + dx(j)**2
      dnrm2 = dsqrt(sum)
      goto 1130
 
 1120 i = i + incx
      if (i .le. nn) goto 1010
 
c     Compute square root and adjust for scaling
 
      dnrm2 = xmax*dsqrt(sum)

 1130 return
      end
**/
	for(j = i; j < nn; j += incx)
	{
	    if(fabs(dx[j]) >= hitest) goto L1040;
	    sum += dx[j]*dx[j];
	}
	return(sqrt(sum));

 L1120:	i += incx;
	if(i < nn) goto L1010;

	return(xmax*sqrt(sum));
}
