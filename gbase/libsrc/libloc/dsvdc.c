#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libloc.h"

/*
c NAME
c	dsvdc -- Perform singular value decomposition.

c FILE
c	dsvdc.f

c SYNOPSIS
c	Decompose an arbitrary matrix into its left and right singular
c	vectors along with their singular values via singular value
c	decomposition.

c DESCRIPTION
c	Subroutine.  dsvdc is a LINPACK subroutine designed to reduce 
c	a real*8 NxP matrix x() by orthogonal transformations u() and v() 
c	to diagonal form.  The diagonal elements s(i) are the singular 
c	values of x().  The columns of u() are the corresponding left 
c	singular vectors, and the columns of v() the right singular 
c	vectors.
 
c	---- On entry ----
c	ldx:		Leading dimension of x()
c	x(ldx,p):	ldx.ge.n; x() contains the matrix whose singular 
c			value decomposition is to be computed.  x() is 
c			destroyed.
c	n:		The number of columns (parameters) of x()
c	p:		The number of rows (data) of x()
c	ldu:		Leading dimension of u()
c	ldv:		Leading dimension of v()
c	work():		Scratch array
c	job:		Control the computation of the singular vectors.
c			decimal expansion ab has the following meaning:
c			a.eq.0: Do not compute the left singular vectors.
c			a.eq.1: Return the n left singular vectors in u().
c			a.ge.2: Return the first min(n,p) singular vectors 
c				in u().
c			b.eq.0: Do not compute the right singular vectors.
c			b.eq.1: Return the right singular vectors in v().

c	---- On return ----
c	s(n):		Singular values in descrending order of magnitude, 
c			where the array size is min(n+1,p)
c       e(p):		e() ordinarily contains zeros.  However see the
c			discussion of info for exceptions.
c	u(ldu,k):	Matrix of left singular vectors, where ldu.ge.n.
c			If joba.eq.1 then k.eq.n,  If joba.ge.2, then 
c			k.eq.min(n,p).  u() is not referenced if joba.eq.0.
c			If n.le.p or if joba.eq.2, then u() may be 
c			indentified with x() in the subroutine call.
c	v(ldv,p):	Matrix of right singular vectors, where ldv.ge.p. 
c			v() is not referenced if job.eq.0.  If p.le.n, then 
c			v() may be identified with x() in subroutine call.
c	info:		The singular values (and their corresponding singular 
c			vectors) s(info+1), s(info+2),..., s(m) are correct 
c			(here m = min(n,p)).  Thus, if info.eq.0, all the 
c			singular values and their vectors are correct.  In 
c			any event, the matrix b() = trans(u)*x*v is the 
c			bidiagonal matrix with the elements of s() on its 
c			diagonal and the elements of e() on its super-diagonal 
c			(trans(u) is the transpose of u()).  Thus the singular 
c			values of x() and b() are the same.

c	---- Subroutines called ----
c	Local
c		daxpy:	LINPACK constant times vector plus vector routine
c		drot: 	LINPACK routine to apply a simple plane rotation
c		drotg:	LINPACK routine to apply a Givens plane rotation
c		dscal:	LINPACK routine which scales a vector by a constant
c		dswap:	LINPACK routine to interchange (swap) 2 vectors

c	---- Functions called ----
c	Local
c		ddot:	LINPACK function to compute dot product of 2 vectors
c		dnrm2:	LINPACK function to compute Euclidean norm

c DIAGNOSTICS
c	See variable info above.

c NOTES
c

c SEE ALSO
c	LINPACK documentation by John Dongarra.

c AUTHOR
c	G.W. Stewart, U. of Maryland, Argonne National Lab., March 1979.


      subroutine dsvdc (x, ldx, n, p, s, e, u, ldu, v, ldv, work, job,
     &                  info)
*/

#define mod(i,j) (i-((i)/(j))*(j))

#define x(i,j)	px[(i-1)+(j-1)*ldx]
#define u(i,j)	pu[(i-1)+(j-1)*ldu]
#define v(i,j)	pv[(i-1)+(j-1)*ldv]
#define s(i)	ps[i-1]
#define e(i)	pe[i-1]
#define work(i)	pwork[i-1]

static double dmax(double a, double b, double c, double d, double f);

void
dsvdc(double *px, int ldx, int n, int p, double *ps, double *pe, double *pu,
	int ldu, double *pv, int ldv, double *pwork, int job, int *info)
{

/**
c     ---- On entry ----

      integer*4 job, ldu, ldv, ldx, n, p
      real*8    work(1), x(ldx,1)
     
c     ---- On return ----

      integer*4 info
      real*8    e(1), s(1), u(ldu,1), v(ldv,1)
 
c     ---- Internal variables ----
 
      integer*4 i, iter, j, jobu, k, kase, kk, l, ll, lls, lm1, lp1, ls
      integer*4 lu, m, maxit, mm, mm1, mp1, nct, nctp1, ncu, nrt, nrtp1
      real*8    ddot, t
      real*8    b, c, cs, el, emm1, f, g, dnrm2, scale, shift, sl, sm
      real*8    sn, smm1, t1, test, ztest
      logical   wantu, wantv
**/
	int	i, iter, j, jobu, k, kase, kk, l, ll, lls, lm1, lp1, ls=0;
	int	lu, m, maxit, mm, mm1, mp1, nct, nctp1, ncu, nrt, nrtp1;
	double	t;
	double	b, c, cs, el, emm1, f, g, scale, shift, sl, sm;
	double	sn, smm1, t1, test, ztest;
	int	wantu, wantv;

	extern double dnrm2(int, double*,int), ddot(int, double*,int, double*,int);
	
/**
c     Set the maximum number of iterations
 
      maxit = 30
 
c     Determine what is to be computed
 
      wantu = .false.
      wantv = .false.
      jobu  = mod(job,100)/10
      ncu   = n
      if (jobu.gt.1) ncu   = min0(n,p)
      if (jobu.ne.0) wantu = .true.
      if (mod(job,10).ne.0) wantv = .true.
**/
	maxit = 30;

	wantu = 0;
	wantv = 0;
	jobu  = mod(job,100)/10;
	ncu   = n;
	if(jobu > 1) ncu = (n < p) ? n : p;
	if(jobu != 0) wantu = 1;
	if(mod(job,10) != 0) wantv = 1;
 
/**
c     Reduce x to bidiagonal form, storing the diagonal elements in s
c     and the super-diagonal elements in e()
 
      info = 0
      nct  = min0(n-1,p)
      nrt  = max0(0,min0(p-2,n))
      lu   = max0(nct,nrt)
**/
	*info = 0;
	nct  = (n-1 < p) ? n-1 : p;
	nrt  = (p-2 < n) ? p-2 : n;
	if(nrt < 0) nrt = 0;
	lu   = (nct > nrt) ? nct : nrt;

/**
      if (lu.lt.1) goto 1140
      do 1130 l = 1, lu
         lp1 = l + 1
         if (l.gt.nct) goto 1010
 
c        Compute the transformation for the l-th column and place the
c        l-th diagonal in s(l)
 
         s(l) = dnrm2(n-l+1,x(l,l),1)
         if (s(l).eq.0.0d0) goto 1000
         if (x(l,l).ne.0.0d0) s(l) = dsign(s(l),x(l,l))
         call dscal (n-l+1, 1.0d0/s(l), x(l,l), 1)
         x(l,l) = 1.0d0 + x(l,l)
 1000    s(l)   = -s(l)
 1010    if (p.lt.lp1) goto 1040
         do 1030 j = lp1, p
            if (l.gt.nct) goto 1020
            if (s(l).eq.0.0d0) goto 1020
 
c           Apply the transformation
 
            t = -ddot(n-l+1, x(l,l), 1, x(l,j),1)/x(l,l)
            call daxpy (n-l+1, t, x(l,l), 1, x(l,j), 1)
 
c           Place the l-th row of x into  e for the subsequent 
c           calculation of the row transformation.
 
 1020       e(j) = x(l,j)
 1030    continue
**/
	if(lu >= 1)
	{
	    for(l = 1; l <= lu; l++)
	    {
		lp1 = l + 1;
		if(l <= nct)
		{
		    s(l) = dnrm2(n-l+1, &x(l,l), 1);
		    if(s(l) != 0.)
		    {
			if(x(l,l) != 0.)
			{
			    s(l) = fabs(s(l));
			    if(x(l,l) < 0.) s(l) = -s(l);
		        }
		        dscal(n-l+1, 1./s(l), &x(l,l), 1);
		        x(l,l) += 1.;
		    }
		    s(l) = -s(l);
	        }

	        for(j = lp1; j <= p; j++)
	        {
		    if(l <= nct && s(l) != 0.)
		    {
		        t = -ddot(n-l+1, &x(l,l), 1, &x(l,j), 1)/x(l,l);
		        daxpy(n-l+1, t, &x(l,l), 1, &x(l,j), 1);
		    }
		    e(j) = x(l,j);
	        }

/**
 1040    if (.not.wantu .or. l.gt.nct) goto 1060
 
c        Place the transformation in u() for subsequent back multiplication
 
         do 1050 i = l, n
 1050    u(i,l) = x(i,l)
**/
		if(wantu && l <= nct)
		{
		    for(i = l; i <= n; i++) u(i,l) = x(i,l);
		}
	
/**
 1060    if (l.gt.nrt) goto 1130
 
c	 Compute the l-th row transformation and place the l-th 
c	 super-diagonal in e(l)
 
         e(l) = dnrm2(p-l,e(lp1),1)
         if (e(l).eq.0.0d0) goto 1070
         if (e(lp1).ne.0.0d0) e(l) = dsign(e(l),e(lp1))
         call dscal (p-l, 1.0d0/e(l), e(lp1), 1)
         e(lp1) = 1.0d0 + e(lp1)
 1070    e(l)   = -e(l)
         if (lp1.gt.n .or. e(l).eq.0.0d0) goto 1110
 
c        Apply the transformation
 
         do 1080 i = lp1, n
 1080    work(i) = 0.0d0
         do 1090 j = lp1, p
 1090    call daxpy (n-l, e(j), x(lp1,j), 1, work(lp1), 1)
         do 1100 j = lp1, p
 1100    call daxpy (n-l, -e(j)/e(lp1), work(lp1), 1, x(lp1,j), 1)
 1110    if (.not.wantv) goto 1130
 
c	 Place the transformation in v() for subsequent back multiplication
 
         do 1120 i = lp1, p
 1120    v(i,l) = e(i)
 1130 continue
**/
		if(l <= nrt)
		{
		    e(l) = dnrm2(p-l, &e(lp1), 1);
		    if(e(l) != 0.)
		    {
		        if(e(lp1) != 0.)
		        {
			    e(l) = fabs(e(l));
			    if(e(lp1) < 0.) e(l) = -e(l);
		        }
		        dscal(p-l, 1./e(l), &e(lp1), 1);
		        e(lp1) += 1.;
		    }
		    e(l) = -e(l);

		    if(lp1 <= n && e(l) != 0.)
		    {
		        for(i = lp1; i <= n; i++) work(i) = 0.;
		        for(j = lp1; j <= p; j++)
		        {
			    daxpy(n-l, e(j), &x(lp1,j), 1, &work(lp1), 1);
		        }
		        for(j = lp1; j <= p; j++)
		        {
			    daxpy(n-l, -e(j)/e(lp1), &work(lp1), 1, &x(lp1,j), 1);
		        }
		    }
		    if(wantv)
		    {
		        for(i = lp1; i <= p; i++)
		        {
			    v(i,l) = e(i);
		        }
		    }
	        }
	    }
	}
		
 
/**
c     Set up the final bidiagonal matrix or order m
 
 1140 m     = min0(p,n+1)
      nctp1 = nct + 1
      nrtp1 = nrt + 1
      if (nct.lt.p)   s(nctp1) = x(nctp1,nctp1)
      if (n.lt.m)     s(m)     = 0.0d0
      if (nrtp1.lt.m) e(nrtp1) = x(nrtp1,m)
      e(m) = 0.0d0
**/
	m	= (p < n+1) ? p : n+1;
	nctp1	= nct + 1;
	nrtp1	= nrt + 1;
	if(nct < p)	s(nctp1) = x(nctp1,nctp1);
	if(n < m)	s(m)     = 0.;
	if(nrtp1 < m)	e(nrtp1) = x(nrtp1,m);
	e(m) = 0.;

/**
c     If required, generate u()
 
      if (.not.wantu) goto 1240
      if (ncu.lt.nctp1) goto 1170
      do 1160 j = nctp1, ncu
         do 1150 i = 1, n
 1150    u(i,j) = 0.0d0
         u(j,j) = 1.0d0
 1160 continue
 1170 if (nct.lt.1) goto 1240
      do 1230 ll = 1, nct
         l = nct - ll + 1
         if (s(l).eq.0.0d0) goto 1210
         lp1 = l + 1
         if (ncu.lt.lp1) goto 1190
         do 1180 j = lp1, ncu
            t = -ddot(n-l+1, u(l,l), 1, u(l,j),1)/u(l,l)
            call daxpy (n-l+1, t, u(l,l), 1, u(l,j), 1)
 1180    continue
 1190    call dscal (n-l+1, -1.0d0, u(l,l), 1)
         u(l,l) = 1.0d0 + u(l,l)
         lm1    = l - 1
         if (lm1.lt.1) goto 1230
         do 1200 i = 1, lm1
 1200    u(i,l) = 0.0d0
         goto 1230
 1210    do 1220 i = 1, n
 1220    u(i,l) = 0.0d0
         u(l,l) = 1.0d0
 1230 continue
**/
	if(wantu)
	{
	    for(j = nctp1; j <= ncu; j++)
	    {
		for(i = 1; i <= n; i++) u(i,j) = 0.;
		u(j,j) = 1.;
	    }
	    for(ll = 1; ll <= nct; ll++)
	    {
		l = nct - ll + 1;
		if(s(l) != 0.)
		{
		    lp1 = l + 1;
		    for(j = lp1; j <= ncu; j++)
		    {
			t = -ddot(n-l+1, &u(l,l), 1, &u(l,j), 1)/u(l,l);
			daxpy(n-l+1, t, &u(l,l), 1, &u(l,j), 1);
		    }
		    dscal(n-l+1, -1., &u(l,l), 1);
		    u(l,l) += 1.;
		    lm1 = l - 1;
		    for(i = 1; i <= lm1; i++) u(i,l) = 0.;
		}
		else
		{
		    for(i = 1; i <= n; i++) u(i,l) = 0.;
		    u(l,l) = 1.;
		}
	    }
	}
 
/**
c     If it is required, generate v()
 
 1240 if (.not.wantv) goto 1290
      do 1280 ll = 1, p
         l    = p - ll + 1
         lp1 = l + 1
         if (l.gt.nrt) goto 1260
         if (e(l).eq.0.0d0) goto 1260
         do 1250 j = lp1, p
            t = -ddot(p-l, v(lp1,l), 1, v(lp1,j),1)/v(lp1,l)
            call daxpy (p-l, t, v(lp1,l), 1, v(lp1,j), 1)
 1250    continue
 1260    do 1270 i = 1, p
 1270    v(i,l) = 0.0d0
         v(l,l) = 1.0d0
 1280 continue
**/
	if(wantv)
	{
	    for(ll = 1; ll <= p; ll++)
	    {
		l = p - ll + 1;
		lp1 = l + 1;
		if(l <= nrt && e(l) != 0.)
		{
		    for(j = lp1; j <= p; j++)
		    {
			t = -ddot(p-l, &v(lp1,l), 1, &v(lp1,j),1)/v(lp1,l);
			daxpy(p-l, t, &v(lp1,l), 1, &v(lp1,j), 1);
		    }
		}
		for(i = 1; i <= p; i++) v(i,l) = 0.;
		v(l,l) = 1.;
	    }
	}
 
/**
c     Main iteration loop for the singular values
 
 1290 mm   = m
      iter = 0

c     Quit if all the singular values have been found
 
c     Exit
 1300 if (m.eq.0) goto 9900
 
c     If too many iterations have been performed, set flag and return
 
      if (iter.lt.maxit) goto 1310
      *info = m

c     Exit
      goto 9900
 
c     This section of the program inspects for negligible elements in 
c     the s and e arrays.  On completion the variables kase and l are 
c     set as follows.
 
c	kase = 1	if s(m) and e(l-1) are negligible and l.lt.m
c	kase = 2	if s(l) is negligible and l.lt.m
c	kase = 3	if e(l-1) is negligible, l.lt.m, and
c			s(l), ..., s(m) are not negligible (qr step)
c	kase = 4	if e(m-1) is negligible (convergence)
**/
	mm = m;
	iter = 0;

	while(m > 0)
	{
	    if(iter >= maxit)
	    {
		*info = m;
		return;
	    }
 
/**
 1310 do 1320 ll = 1, m
         l = m - ll

c        Exit
         if (l.eq.0) goto 1330
         test  = dabs(s(l)) + dabs(s(l+1))
         ztest = test + dabs(e(l))
         if (ztest.ne.test) goto 1320
         e(l) = 0.0d0

c        Exit
         goto 1330
 1320 continue
**/
	    for(ll = 1; ll <= m; ll++)
	    {
		l = m - ll;
		if(l == 0) break;
		test = fabs(s(l)) + fabs(s(l+1));
		ztest = test + fabs(e(l));
		if(ztest == test)
		{
		    e(l) = 0.;
		    break;
		}
	    }

/**
 1330 if (l.ne.m-1) goto 1340
      kase = 4
      goto 1390
 1340 lp1 = l + 1
      mp1 = m + 1
      do 1350 lls = lp1, mp1
         ls = m - lls + lp1

c        Exit
         if (ls.eq.l) goto 1360
         test = 0.0d0
         if (ls.ne.m) test = test + dabs(e(ls))
         if (ls.ne.l+1) test = test + dabs(e(ls-1))
         ztest = test + dabs(s(ls))
         if (ztest.ne.test) goto 1350
            s(ls) = 0.0d0

c           Exit
            goto 1360
 1350 continue
 1360 if (ls.ne.l) goto 1370
      kase = 3
      goto 1390
 1370 if (ls.ne.m) goto 1380
      kase = 1
      goto 1390
 1380 kase = 2
      l = ls
 1390 l = l + 1
**/
	    if(l == m-1)
	    {
		kase = 4;
	    }
	    else
	    {
		lp1 = l + 1;
		mp1 = m + 1;
		for(lls = lp1; lls <= mp1; lls++)
		{
		    ls = m - lls + lp1;
		    if(ls == l) break;
		    test = 0.;
		    if(ls != m) test += fabs(e(ls));
		    if(ls != l+1) test += fabs(e(ls-1));
		    ztest = test + fabs(s(ls));
		    if(ztest == test)
		    {
			s(ls) = 0.;
			break;
		    }
		}
		if(ls == l)
		{
		    kase = 3;
		}
		else if(ls == m)
		{
		    kase = 1;
		}
		else
		{
		    kase = 2;
		    l = ls;
		}
	    }
	    l++;
 
/**
c     Perform the task indicated by kase
 
      goto (1400, 1430, 1450, 1480), kase
 
c     Deflate negligible s(m)
 
 1400 mm1    = m - 1
      f      = e(m-1)
      e(m-1) = 0.0d0
      do 1420 kk = l, mm1
         k  = mm1 - kk + l
         t1 = s(k)
         call drotg (t1, f, cs, sn)
         s(k) = t1
         if (k.eq.l) goto 1410
         f      = -sn*e(k-1)
         e(k-1) = cs*e(k-1)
 1410    if (wantv) call drot (p, v(1,k), 1, v(1,m), 1, cs, sn)
 1420 continue
      goto 1300
**/
	    if(kase == 1)
	    {
		mm1 = m - 1;
		f = e(m-1);
		e(m-1) = 0.;
		for(kk = l; kk <= mm1; kk++)
		{
		    k = mm1 - kk + l;
		    t1 = s(k);
		    drotg(&t1, &f, &cs, &sn);
		    s(k) = t1;
		    if(k != l)
		    {
			f = -sn*e(k-1);
			e(k-1) = cs*e(k-1);
		    }
		    if(wantv) drot(p, &v(1,k), 1, &v(1,m), 1, cs, sn);
		}
	    }
 
/**
c     Split at negligible s(l)
 
 1430 f      = e(l-1)
      e(l-1) = 0.0d0
      do 1440 k = l, m
         t1 = s(k)
         call drotg (t1, f, cs, sn)
         s(k) = t1
         f    = -sn*e(k)
         e(k) = cs*e(k)
         if (wantu) call drot (n, u(1,k), 1, u(1,l-1), 1, cs, sn)
 1440 continue
      goto 1300
**/
	    else if(kase == 2)
	    {
		f = e(l-1);
		e(l-1) = 0.;
		for(k = l; k <= m; k++)
		{
		    t1 = s(k);
		    drotg(&t1, &f, &cs, &sn);
		    s(k) = t1;
		    f = -sn*e(k);
		    e(k) = cs*e(k);
		    if(wantu) drot(n, &u(1,k), 1, &u(1,l-1), 1, cs, sn);
		}
	    }
 
/**
c     Perform one QR step
 
c     Calculate the shift
 
 1450 scale = dmax1(dabs(s(m)),dabs(s(m-1)),dabs(e(m-1)),
     &              dabs(s(l)),dabs(e(l)))
      sm    = s(m)/scale
      smm1  = s(m-1)/scale
      emm1  = e(m-1)/scale
      sl    = s(l)/scale
      el    = e(l)/scale
      b     = ((smm1 + sm)*(smm1 - sm) + emm1**2)/2.0d0
      c     = (sm*emm1)**2
      shift = 0.0d0
      if (b.eq.0.0d0 .and. c.eq.0.0d0) goto 1460
      shift = dsqrt(b**2+c)
      if (b.lt.0.0d0) shift = -shift
      shift = c/(b + shift)
 1460 f = (sl + sm)*(sl - sm) - shift
      g = sl*el
 
c     Chase zeros
 
      mm1 = m - 1
      do 1470 k = l, mm1
         call drotg (f, g, cs, sn)
         if (k.ne.l) e(k-1) = f
         f      = cs*s(k) + sn*e(k)
         e(k)   = cs*e(k) - sn*s(k)
         g      = sn*s(k+1)
         s(k+1) = cs*s(k+1)
         if (wantv) call drot (p, v(1,k), 1, v(1,k+1), 1, cs, sn)
         call drotg (f, g, cs, sn)
         s(k)   = f
         f      = cs*e(k) + sn*s(k+1)
         s(k+1) = -sn*e(k) + cs*s(k+1)
         g      = sn*e(k+1)
         e(k+1) = cs*e(k+1)
         if (wantu .and. k .lt. n)
     &      call drot (n, u(1,k), 1, u(1,k+1), 1, cs, sn)
 1470 continue
      e(m-1) = f
      iter   = iter + 1
      goto 1300
**/
	    else if(kase == 3)
	    {
		scale = dmax(fabs(s(m)), fabs(s(m-1)), fabs(e(m-1)),
				fabs(s(l)), fabs(e(l)));
		sm    = s(m)/scale;
		smm1  = s(m-1)/scale;
		emm1  = e(m-1)/scale;
		sl    = s(l)/scale;
		el    = e(l)/scale;
		b     = ((smm1 + sm)*(smm1 - sm) + emm1*emm1)/2.0;
		c     = pow(sm*emm1, 2.);
		shift = 0.0;
		if(b != 0. || c != 0.)
		{
		    shift = sqrt(b*b+c);
		    if(b < 0.) shift = -shift;
		    shift = c/(b + shift);
		}
		f = (sl + sm)*(sl - sm) - shift;
		g = sl*el;

		mm1 = m - 1;
		for(k = l; k <= mm1; k++)
		{
		    drotg(&f, &g, &cs, &sn);
		    if(k != l) e(k-1) = f;
		    f	   = cs*s(k) + sn*e(k);
		    e(k)   = cs*e(k) - sn*s(k);
		    g	   = sn*s(k+1);
		    s(k+1) = cs*s(k+1);
		    if(wantv) drot(p, &v(1,k), 1, &v(1,k+1), 1, cs, sn);
		    drotg(&f, &g, &cs, &sn);
		    s(k)   = f;
		    f	   =  cs*e(k) + sn*s(k+1);
		    s(k+1) = -sn*e(k) + cs*s(k+1);
		    g	   = sn*e(k+1);
		    e(k+1) = cs*e(k+1);
		    if(wantu && k < n)
			drot(n, &u(1,k), 1, &u(1,k+1), 1, cs, sn);
		}
		e(m-1) = f;
		iter++;
	    }
 
/**
c     Convergence
 
c     Make the singular value  positive
 
 1480 if (s(l).ge.0.0d0) goto 1490
      s(l) = -s(l)
      if (wantv) call dscal (p, -1.0d0, v(1,l), 1)
 
c     Order the singular value
 
 1490 if (l.eq.mm) goto 1500

c     Exit
      if (s(l).ge.s(l+1)) goto 1500
      t      = s(l)
      s(l)   = s(l+1)
      s(l+1) = t
      if (wantv .and. l.lt.p) call dswap (p, v(1,l), 1, v(1,l+1), 1)
      if (wantu .and. l.lt.n) call dswap (n, u(1,l), 1, u(1,l+1), 1)
      l = l + 1
      goto 1490
 1500 iter = 0
      m = m - 1
      goto 1300
 9900 return
      end
**/
	    else
	    {
		if(s(l) < 0)
		{
		    s(l) = -s(l);
		    if(wantv) dscal(p, -1., &v(1,l), 1);
		}
		while(l != mm && s(l) < s(l+1))
		{
		    t = s(l);
		    s(l) = s(l+1);
		    s(l+1) = t;
		    if(wantv && l < p) dswap(p, &v(1,l), 1, &v(1,l+1), 1);
		    if(wantu && l < n) dswap(n, &u(1,l), 1, &u(1,l+1), 1);
		    l++;
		}
		iter = 0;
		m--;
	    }
	}
}

static double
dmax(double a, double b, double c, double d, double f)
{
	double max;

	max = a;
	if(b > max) max = b;
	if(c > max) max = c;
	if(d > max) max = d;
	if(f > max) max = f;
	return(max);
}
