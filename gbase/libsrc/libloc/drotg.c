#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libloc.h"

/*
c NAME
c	drotg -- Construct a Givens plane rotation.

c FILE
c	drotg.f

c SYNOPSIS
c	LINPACK routine applies a Givens plane rotation.

c DESCRIPTION
c	Subroutine.  Construct a Givens plane rotation, a scalar, da and
c	db, at a time.  Called in SVD routine, dsvdc, prior to application
c	of normal plane rotation (subr. drot).

c DIAGNOSTICS
c

c NOTES
c

c SEE ALSO
c	LINPACK documentation by John Dongarra.

c AUTHOR
c	John Dongarra, March 1978.


      subroutine drotg (da, db, c, s)
*/

void
drotg(double *da, double *db, double *c, double *s)
{

/**
c     ---- On entry and return ----

      real*8 c, da, db, s

c     ---- Internal variables ----

      real*8 r, roe, scale, z
**/
	double	r, roe, scale, z;
 
/**
      roe = db
      if ( dabs(da) .gt. dabs(db) ) roe = da
      scale = dabs(da) + dabs(db)
      if (scale .ne. 0.0d0) goto 1000
      c = 1.0d0
      s = 0.0d0
      r = 0.0d0
      goto 1010
 1000 r = scale*dsqrt((da/scale)**2 + (db/scale)**2)
      r = dsign(1.0d0, roe)*r
      c = da/r
      s = db/r
 1010 z = 1.0d0
      if (dabs(da) .gt. dabs(db)) z = s
      if (dabs(db) .ge. dabs(da) .and. c .ne. 0.0d0) z = 1.0d0/c
      da = r
      db = z
**/
	roe = *db;
	if(fabs(*da) > fabs(*db)) roe = *da;
	scale = fabs(*da) + fabs(*db);
	if(scale == 0.)
	{
	    *c = 1.0;
	    *s = 0.0;
	    r = 0.0;
	}
	else
	{
	    r = scale*sqrt(pow(*da/scale, 2.) + pow(*db/scale, 2.));
	    if(roe < 0) r = -r;
	    *c = *da/r;
	    *s = *db/r;
	}
	z = 1.0;
	if(fabs(*da) >  fabs(*db)) z = *s;
	if(fabs(*db) >= fabs(*da) && *c != 0.0) z = 1.0/(*c);
	*da = r;
	*db = z;
}
