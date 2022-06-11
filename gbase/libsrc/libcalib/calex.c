#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "libcalib.h"
#include "logErrorMsg.h"
#include "libstring.h"

/*
 *    From the Program CALEX5a
 *
 * System calibration using arbitrary test signals
 * Simulation using impulse-invariant recursive filters
 * Reference: H.W.Schuessler, A signalprocessing approach to simulation,
 * Frequenz 35 (1981), 174-184
 *
 * -----------------------------------------------------------
 * The following comments refer to the original Fortran code:
 * 
 * recent revisions:
 *
 * 04 Dec 1998
 * partial systems defined by keywords hp1, lp1, hp2, bp2, lp2
 * anti-alias-filtered 'ausg' signal written into file 'ausf'
 * output in free format
 *
 * 22 Feb 1999
 * delay parameter alternatively renamed 'sub' and (ab)used for 
 * compensation of direct input-to-output coupling (for calibration 
 * of single-coil geophones in a half bridge).
 *
 * 06 Mar 2002
 * the delay parameter is alternatively used for the compensation of tilt
 * effects on a shake table.
 *
 * 12 Mar 2002
 * In place of using the delay parameter alternatively for the compensation
 * of galvanic coupling or tilt, the program now expects to find four 
 * parameters with fixed meaning: amp for the gain, del for the delay, sub 
 * for galvanic coupling, and til for the tilt. The sub parameter causes the 
 * fraction sub of the input signal to be subtracted from the output signal. 
 * The til parameter causes the twice-integrated signal to be added with 
 * amplitude til (actually this is done with the output signal, but could be 
 * done with the input signal as well, which would correspond better to the 
 * physical situation on a shake table).
 *
 * The parameter file is no longer compatible with that of the original 
 * version. It must contain two additional lines with 'sub' and 'til' after 
 * the 'del' line.
 *
 * 18 Sep 05
 * names of input and output files are read from calex5a.par
 * The input file must contain two additional lines with the file names
 * before all other parameters, and is no longer compatible with
 * earlier versions. File names should be enclosed in simple quotes
 * (apostrophs) especially when they contain spaces, or the line contains
 * comments after the file names. Lines starting with a space are ignored.
 */

#define ndi 8

static int CalibMini(CalibSys *s, CalibParam *p, double *x, double *q,
		double *d, double *ein);
static int CalibSysdef(CalibSys *s, CalibParam *p, double *x, double dt,
		double *delay, double *compens1, double *compens2);
static int CalibPartl(CalibSys *s);
static void CalibRekf1(double dt, double r, int n, double *ein, double *sum);
static void CalibRekf2(double sre, double sim, double rre, double rim, int n,
		double *ein, double *sum);
static void CalibPolyTrend(int mm, double *x, int n);
static void CalibGauss(double aik[ndi][ndi], int m, int n, double *rs,
		double *f);

int
CalibCompute(CalibParam *p, double dt, int npts, double *eing, double *ausg,
		CalibOut *co)
{
	int i, iter, noimp, ret = -1;
	double ein0, q, qq, qalt, gnorm, x[mpar];
	CalibParam p1;
	CalibSys s;
	CalibOut co_null = CALIB_OUT_NULL;

	*co = co_null;

	s.dt = dt;
	s.init = -1;

	p1 = *p;

	p1.ns1 = 1;
	p1.ns2 = npts;
	p1.m = 0;
	p1.m0i = 0;
	p1.m1i = 0;
	p1.m2i = 0;
	for(i = 0; i < 4; i++) {
	    p1.x0[i] = 0.0;
	    p1.rho[i] = 0.0;
	}
	p1.x0[0] = 1.0;

	/* corner period and order of the anti-alias filter
	 */
	CalibSetParam(&p1, s.dt);

	s.aus = NULL;
	s.sum = NULL;
	s.einf = NULL;
	s.sta = NULL;

	if(!(s.aus = (double *)mallocWarn(npts*sizeof(double))) ||
	   !(s.sum = (double *)mallocWarn(npts*sizeof(double))) ||
	   !(s.einf = (double *)mallocWarn(npts*sizeof(double))) ||
	   !(s.sta = (double *)mallocWarn(npts*sizeof(double))))
	{
	    goto ERROR_RETURN;
	}

	for(i = 0; i < mpar; i++) x[i] = 0.0;

	if(CalibQuadr(&s, &p1, x, &q, ausg)) goto ERROR_RETURN;

	for(i = 0; i < npts; i++) s.aus[i] = s.sum[i];

	co->npts = npts;
	co->dt = s.dt;
	if(!(co->ausf = (double *)mallocWarn(npts*sizeof(double)))) {
	    goto ERROR_RETURN;
	}
	memcpy(co->ausf, s.aus, npts*sizeof(double));

	ein0 = eing[0];
	for(i = 0; i < npts; i++) eing[i] -= ein0;

	if(CalibQuadr(&s, &p1, x, &qq, eing)) goto ERROR_RETURN;

	for(i = 0; i < npts; i++) s.einf[i] = s.sum[i];
	
	if(!(co->einf = (double *)mallocWarn(npts*sizeof(double)))) {
	    goto ERROR_RETURN;
	}
	memcpy(co->einf, s.einf, npts*sizeof(double));

	/* prepare iteration, set start parameters
	 */
	iter = 0;
	s.step = 1.0;
	noimp = 0;
	s.init = 0;

	CalibSetParam(p, s.dt);

	/* start model
	 */

	s.qn = 0.0;
	for(i = p->ns1-1; i < p->ns2; i++) {
	    s.qn += pow(s.aus[i], 2);
	}

	for(i = 0; i < npts; i++) s.sta[i] = 0.0;

	if(CalibQuadr(&s, p, x, &q, eing)) goto ERROR_RETURN;

	for(i = p->ns1-1; i < p->ns2; i++) {
	    s.sta[i] = s.aus[i] - s.sum[i];
	}
	
	if(!(co->synt1 = (double *)malloc(npts*sizeof(double)))) {
	    goto ERROR_RETURN;
	}
	memcpy(co->synt1, s.sta, npts*sizeof(double));

	co->initial_rms = sqrt(q);
	co->npar = p->m;

	/* iteration with the conjugate-gradient method
	 */

	for(iter = 1; iter <= p->maxit; iter++)
	{
	    qalt = q;

	    if(CalibCongrd(&s, p, x, &q, &gnorm, iter, eing)) goto ERROR_RETURN;

	    if(co->return_x) {
		int n = p->m + 1;
		if(iter == 1) {
		    if(!(co->x =(double *)mallocWarn(n*sizeof(double)))) {
			goto ERROR_RETURN;
		    }
		}
		else if(!(co->x = (double *)reallocWarn(co->x,
			(iter*n)*sizeof(double)))) goto ERROR_RETURN;
		co->x[(iter-1)*n] = sqrt(q);
		for(i = 0; i < p->m; i++) {
		    co->x[(iter-1)*n+1+i] = x[i];
		}
	    }

	/* the iteration stops when in m steps, the fit was improved by less
	 * than qac and the parameter vector was changed by less than finac.
	 * The rms error is relative to the rms amplitude of the output signal.
	 * Parameter corrections are relative to the specified search interval.
	 */

	    noimp++;

	    if((sqrt(qalt)-sqrt(q)) > p->qac || s.axi > p->finac) noimp = 0;
	    if(noimp >= p->m) break;
	}

	if(iter <= p->maxit) {
	    co->converged = 1;
	}
	else {
	    co->converged = 0;
	    iter = p->maxit;
	}

	co->iter = iter;
	co->final_rms = sqrt(q);
	for(i = 0; i < p->m; i++) {
	    co->final_par[i] = p->x00[i]+p->r00[i]*x[i];
	}

/*	if(iter < p->maxit) { */
	    for(i = 0; i < npts; i++) s.sta[i] = 0.0;
	    for(i = p->ns1-1; i < p->ns2; i++) {
		s.sta[i] = s.aus[i] - s.sum[i];
	    }
	    if(!(co->synt2 = (double *)malloc(npts*sizeof(double)))) {
		goto ERROR_RETURN;
	    }
	    memcpy(co->synt2, s.sta, npts*sizeof(double));
/*	} */

	for(i = 0; i < npts; i++) s.aus[i] = 0.0;
	for(i = p->ns1-1; i < p->ns2; i++) s.aus[i] = s.sum[i];

	if(!(co->rest = (double *)malloc(npts*sizeof(double)))) {
	    goto ERROR_RETURN;
	}
	memcpy(co->rest, s.aus, npts*sizeof(double));

	ret = 0;

 ERROR_RETURN:
	if(s.aus) free(s.aus);
	if(s.sum) free(s.sum);
	if(s.einf) free(s.einf);
	if(s.sta) free(s.sta);

	return ret;
}

void
CalibSetParam(CalibParam *p, double dt)
{
	int i, l, mali, malias;
	double wi;

	malias = (int)(6.0/log10(p->alias/dt)+1.0);
	mali = malias/2;

	l = 5 + p->m1i + 2*p->m2i;

	if(malias > 2*mali)
	{
	    for(i = l-1; i >= 5; i--) {
		p->x0[i] = p->x0[i-1];
		p->rho[i] = p->rho[i-1];
		strcpy(p->typ[i], p->typ[i-1]);
	    }
	    l++;
	    p->m1i++;
	    p->x0[4] = p->alias;
	    p->rho[4] = 0.0;
	    strcpy(p->typ[4], "lp1");
        }

	wi = (malias) ? 2.0*atan(1.0)/malias : 0.;
	p->m2i += mali;

	for(i = 1; i <= mali; i++) {
	    int l2i = l + 2*i - 2;
	    p->x0[l2i-1] = p->alias;
	    p->rho[l2i-1] = 0.0;
	    strcpy(p->typ[l2i-1], "lp2");
	    p->x0[l2i] = sin(wi*(2*i-1));
	    p->rho[l2i] = 0.0;
	    strcpy(p->typ[l2i], "lp2");
	}
}

/* Method of conjugate gradients according to Fletcher und Reeves (1964)
 */
int
CalibCongrd(CalibSys *s, CalibParam *p, double *x, double *q, double *gnorm,
		int iter, double *ein)
{
	int k;
	double dlen, g[mpar], dd[mpar];

	/* partial derivatives
	 */
	for(k = 0; k < p->m; k++)
	{
	    double qq, qqq;
	    x[k] += p->finac;
	    CalibQuadr(s, p, x, &qq, ein);
	    x[k] -= 2.*p->finac;
	    CalibQuadr(s, p, x, &qqq, ein);
	    x[k] += p->finac;
	    g[k] = (qq-qqq)/(2.0*p->finac);
	}

	/* determine new direction of descent
	 */
	if((int)((iter-1)/p->m)*p->m == iter-1)
	{
	    *gnorm = 0.0;
	    for(k = 0; k < p->m; k++) {
		*gnorm += pow(g[k], 2);
		s->d[k] = -g[k];
	    }
	}
	else {
	    double ga, beta;
	    ga = *gnorm;
	    *gnorm = 0.0;
	    for(k = 0; k < p->m; k++) {
		*gnorm += pow(g[k], 2);
	    }
	    beta = *gnorm/ga;
	    for(k = 0; k < p->m; k++) {
		s->d[k] = -g[k] + beta*s->d[k];
	    }
	}

	dlen = 0.0;
	for(k = 0; k < p->m; k++) {
	    dlen += pow(s->d[k], 2);
	}
	dlen = sqrt(dlen);

	for(k = 0; k < p->m; k++) {
	    dd[k] = s->d[k]/dlen;
	}

	/* search for minimum
	 */
	return CalibMini(s, p, x, q, dd, ein);
}

/*
 * x and q entered as start values, replaced by values of minimum
 * step size is between 1 and finac
 */
static int
CalibMini(CalibSys *s, CalibParam *p, double *x, double *q, double *d,
		double *ein)
{
	int k;
	double ql, qm, qr, xi;
	double xl[mpar], xm[mpar], xr[mpar];

	memcpy(xm, x, p->m*sizeof(double));
	qm = *q;

	for(k = 0; k < p->m; k++) {
	    xl[k] = x[k] - s->step*d[k];
	    xr[k] = x[k] + s->step*d[k];
	}

	CalibQuadr(s, p, xl, &ql, ein);
	CalibQuadr(s, p, xr, &qr, ein);

	for(;;)
	{
	    if(ql < qm && qr < qm) {
		logErrorMsg(LOG_ERR, "maximum encountered.");
		return -1;
	    }

	    if(ql < qm || qr < qm) break;

	    if(s->step < 8.*p->finac)
	    {
		/* interval too small
		 */
		xi = s->step*(ql-qr)/(ql-2.0*qm+qr)/2.0;
		s->axi = fabs(xi);

		for(k = 0; k < p->m; k++) x[k] = xm[k] + xi*d[k];

		CalibQuadr(s, p, x, q, ein);

		return 0;
	    }
	    /* divide interval
	     */
	    s->step /= 8.0;

	    for(k = 0; k < p->m; k++) {
		xl[k] = xm[k] - s->step*d[k];
		xr[k] = xm[k] + s->step*d[k];
	    }

	    CalibQuadr(s, p, xl, &ql, ein);
	    CalibQuadr(s, p, xr, &qr, ein);
	}

	if(ql < qr) {
	    /* reverse direction
	     */
	    memcpy(x, xl, p->m*sizeof(double));
	    *q = ql;
	    memcpy(xl, xr, p->m*sizeof(double));
	    ql = qr;
	    memcpy(xr, x, p->m*sizeof(double));
	    qr = *q;
	    for(k = 0; k < p->m; k++) d[k] = -d[k];
	}

	do {
	    if(s->step > 1.0) {
		/* don't interpolate */
		memcpy(x, xr, p->m*sizeof(double));
		*q = qr;
		s->axi = s->step;
		return 0;
	    }
	    s->step *= 2;
	    memcpy(xm, xr, p->m*sizeof(double));
	    qm = qr;
	    /* double the interval
	     */
	    for(k = 0; k < p->m; k++) {
		xr[k] = xm[k] + s->step*d[k];
	    }
	    CalibQuadr(s, p, xr, &qr, ein);

	} while(qr < qm);

	xi = s->step*(ql-qr)/(ql-2.0*qm+qr)/2.0;
	s->axi = fabs(xi);
	for(k = 0; k < p->m; k++) {
	    x[k] = xm[k] + xi*d[k];
	}
	CalibQuadr(s, p, x, q ,ein);

	return 0;
}

int
CalibQuadr(CalibSys *s, CalibParam *p, double *x, double *quad, double *ein)
{
	int i, ii, j, ns21;
	double delay, compens1, compens2, tau, comp, qua, offset;
	double grav = 9.81e-3;

	ns21 = p->ns2 - p->ns1 + 1;

	if(CalibSysdef(s, p, x, s->dt, &delay, &compens1, &compens2)) {
	    return -1;
	}
	if(CalibPartl(s)) return -1;

	for(i = 0; i < p->ns2; i++) s->sum[i] = 0.0;

	tau = delay;
	if(tau > 0.0) tau -= s->dt;

	for(j = 0; j < s->m1; j++) {
	    i = s->m0 + j;
	    CalibRekf1(s->dt*s->sr[i], s->resid[i]*exp(-tau*s->sr[i]), p->ns2,
			ein, s->sum);
	}

	for(j = 0; j < s->m2; j++)
	{
	    i = s->m0 + s->m1 + j;
	    if(s->si[i] == 0.0) {
		CalibRekf1(s->dt*s->sr[i], s->resid[i]*exp(-tau*s->sr[i]),
			p->ns2, ein, s->sum);
		CalibRekf1(s->dt*s->sr[i+s->m2],
			s->resid[i+s->m2]*exp(-tau*s->sr[i+s->m2]), p->ns2,
			ein, s->sum);
	    }
	    else {
		double sre = -tau*s->sr[i];
		double sim = -tau*s->si[i];
		double zabs = exp(sre);
		double zre = zabs*cos(sim);
		double zim = zabs*sin(sim);
		double rre = s->resid[i]*zre - s->residi[i]*zim;
		double rim = s->resid[i]*zim + s->residi[i]*zre;
		CalibRekf2(s->dt*s->sr[i], s->dt*s->si[i], rre, rim, p->ns2,
				ein, s->sum);
	    }
	}

	if(s->init < 0) return 0;

	if(delay > 0.0) {
	    int i2 = (p->ns1 > 2) ? p->ns1-1 : 1;
	    for(i = p->ns2-1; i >= i2; i--) {
		s->sum[i] = s->sum[i-1];
	    }
	}

	/*
	 * sum is now the synthetics. Correct for galvanic coupling or
	 * shake-table tilt if required, form difference with observed 
	 * output, and remove offset. 
	 * sub is the fraction of the input signal to be subtracted 
	 * from the output.
	 * til ist the tilt of the shake table in microradians per
	 * millimeter of motion. grav is the gravity in km/s^2. 
	 * Note: motion and tilt as seen by the sensor, so if the 
	 * motion is transverse, the tilt parameter may come out 
	 * very large!  
	 */

	if(compens2 != 0.0)
	{
	    double suma, sumn;
	    s->sta[0] = 0.5*s->sum[p->ns1-1];
	    for(i = p->ns1; i < p->ns2; i++) {
		ii = i - p->ns1 + 1;
		s->sta[ii] = s->sta[ii-1] + 0.5*(s->sum[i-1] + s->sum[i]);
	    }
	    CalibPolyTrend(1, s->sta, ns21);

	    suma = s->sta[0];
	    s->sta[0] = 0.5*suma;

	    for(i = p->ns1; i < p->ns2; i++) {
		ii = i - p->ns1 + 1;
		sumn = s->sta[ii];
		s->sta[ii] = s->sta[ii-1] + 0.5*(suma+sumn);
		suma =sumn;
	    }
	    CalibPolyTrend(2, s->sta, ns21);
	}

	comp = compens2*grav*s->dt*s->dt;

	for(i = p->ns1-1; i < p->ns2; i++) {
	    ii = i - p->ns1 + 1;
	    s->sum[i] = s->aus[i] - s->sum[i]
			- compens1*s->einf[i] + comp*s->sta[ii];
	}
	for(i = p->ns1-1; i < p->ns2; i++) {
	    ii = i - p->ns1 + 1;
	    s->sta[ii] = s->sum[i];
	}

	CalibPolyTrend(3, s->sta, ns21);

	for(i = p->ns1-1; i < p->ns2; i++) {
	    ii = i - p->ns1 + 1;
	    s->sum[i] = s->sta[ii];
	}

	offset = 0.0;
	qua = 0.0;
	for(i = p->ns1-1; i < p->ns2; i++) {
	    offset += s->sum[i];
	}
	offset /= ns21;

	for(i = p->ns1-1; i < p->ns2; i++) {
	    s->sum[i] -= offset;
	    qua = qua + pow(s->sum[i], 2);
	}
	*quad = qua/s->qn;

	return 0;
}

/*
 *  Definition of the transfer function (Laplace Transform):
 *  syspar(0 : m0): coefficients of the nominator polynomial, Grad m0
 *  syspar(m0+1 : m0+m1)  corner freq. of first-order partials
 *  syspar(m0+m1+1 : m0+m1+m2)  corner freq. of second-order partials
 *  syspar(m0+m1+m2+1 : m0+m1+2*m2)  numerical damping factors
 *
 *
 *                    Sum  syspar(i) * s**i
 *                  i=0,m0
 *  T(s) = --------------------------------------------------------
 *          Product (s+w1(i))*Product(s**2+2*s*w2(i)*h(i)+w2(i)**2)
 *           i=1,m1            i=1,m2
 *
 *
 *  where: w1(i) = syspar(m0+i)          , i = 1 ... m1
 *         w2(i) = syspar(m0+m1+i)       , i = 1 ... m2
 *          h(i) = syspar(m0+m1+m2+i)    , i = 1 ... m2
 *
 *  In addition the signal may be delayed by 'delay' (abs < +-dt),
 *  diminished by a fraction 'compens1' of the input signal to compensate
 *  for the voltage coupled into the coil of geophones
 *  by the calibration current, and a fraction 'compens2' of the twice-
 *  integrated signal may be removed in order to compensate for the tilt of 
 *  shake tables.
 *
 * current system parameters (order is different from parameter file!)
 */
static int
CalibSysdef(CalibSys *s, CalibParam *p, double *x, double dt, double *delay,
		double *compens1, double *compens2)
{
	int i, ipar, mp;
	double gain, zpi, xx[msys];
	char error[100];

	zpi = 8.0*atan(1.0);
	s->m0 = p->m0i;
	s->m1 = p->m1i;
	s->m2 = p->m2i;
	mp = 4 + s->m1 + 2*s->m2;
	ipar = 0;
	for(i = 0; i < mp; i++)
	{
	    if(p->rho[i] > 0.0) {
		xx[i] = p->x0[i] + x[ipar]*p->rho[i];
		ipar++;
	    }
	    else {
		xx[i] = p->x0[i];
	    }
	}

	gain = xx[0]*dt;
        *delay = xx[1];
        if(fabs(*delay) >= dt) {
	    snprintf(error, sizeof(error),
		"Delay(%.2lf) is greater that dt(%.2lf).", *delay, dt);
	    logErrorMsg(LOG_ERR, error);
	    return -1;
	}
        *compens1 = xx[2];
        *compens2 = xx[3];

	/* Numerator factor for first-order subsystems
	 */
	for(i = 4; i < s->m1+4; i++)
	{
	    if(!strcmp(p->typ[i], "hp1")) {
		s->m0++;
	    }
	    else if(!strcmp(p->typ[i], "lp1")) {
		gain = gain*zpi/xx[i];
	    }
	    else {
		snprintf(error, sizeof(error), "Wrong type: %s", p->typ[i]);
		logErrorMsg(LOG_ERR, error);
		return -1;
	    }
	}

	/* Numerator factor for second-order subsystems
	 */
	for(i = s->m1+4; i <= s->m1+2*s->m2+2; i += 2)
	{
	    if(!strcmp(p->typ[i], "hp2")) {
		s->m0 += 2;
	    }
	    else if(!strcmp(p->typ[i], "bp2")) {
		s->m0++;
		gain = gain*zpi/xx[i];
	    }
	    else if(!strcmp(p->typ[i], "lp2")) {
		gain = gain*pow(zpi/xx[i], 2);
	    }
	    else {
		snprintf(error, sizeof(error), "type error: %s", p->typ[i]);
		logErrorMsg(LOG_ERR, error);
		return -1;
	    }
	}

	if(s->m0 > s->m1 + 2*s->m2 - 2) {
	    logErrorMsg(LOG_ERR, "m0 is too large.");
	    return -1;
	}
	if(s->m0 < -1) {
	    logErrorMsg(LOG_ERR, "m0 is too small.");
	    return -1;
	}

	if(s->m0 >= 0) {
	    for(i = 0; i < s->m0; i++) s->syspar[i] = 0.;

	    s->syspar[s->m0] = gain;

	    for(i = 0; i < s->m1; i++) {
		s->syspar[s->m0+1+i] = zpi/xx[i+4];
	    }
	}
	else {
	    s->m0 = 0;
	    s->syspar[s->m0] = gain;
	    for(i = 0; i < s->m1; i++) {
		s->syspar[1+i] = zpi/xx[i+4];
	    }
	    s->m1++;
	    s->syspar[s->m1] = 0.;
	}

	for(i = 0; i < s->m2; i++) {
	    s->syspar[s->m0+1+s->m1+i] = zpi/xx[2*(i+1)+s->m1+2];
	    s->syspar[s->m0+1+s->m1+s->m2+i] = xx[2*(i+1)+s->m1+3];
	}
	return 0;
}

/*
 * factorization of the transfer function.
 * poles und residues (real and imag.parts) stored like system parameters
 * first-order partials: poles and residues are real and have indices
 * m0+i with i=1 .. m1
 * second-order partials: poles and residues are pairwise complex-conjugate
 * the pole pair is stored in s(m0+m1+i) und s(m0+m1+m2+i);
 * the real part of the residue is stored in resid(m0+m1+i),
 * the imag. part in residi(mo+m1+i), i=1 .. m2
 */
static int
CalibPartl(CalibSys *s)
{
	int i, k, m01;
	double h, w, wh, whh;
	double cresr, cresi, spowr, spowi, spowre;
	char error[100];

	/* determine the complex poles
	 */
	m01 = s->m0 + 1;
	for(i = 0; i < s->m1; i++) {
	    s->sr[s->m0+i]= -s->syspar[m01+i];
	    s->si[s->m0+i] = 0.0;
	}

	for(i = 0; i < s->m2; i++)
	{
	    w = s->syspar[m01+s->m1+i];
	    h = s->syspar[m01+s->m1+s->m2+i];

	    if(fabs(h-1.0) < 1.e-6) {
		snprintf(error, sizeof(error),
		    "CalibPartl: critical damping value # %d", i+1);
		logErrorMsg(LOG_ERR, error);
		return -1;
	    }

	    if(h < 0.0) {
		snprintf(error, sizeof(error),
	"CalibPartl: Negative damping encountered. Reduce range for # %d", i+1);
		logErrorMsg(LOG_ERR, error);
		return -1;
	    }

	    wh = w*h;
	    if(h < 1.) {
		whh = w*sqrt(1.0 - h*h);
		s->sr[s->m0+s->m1+i] = -wh;
		s->si[s->m0+s->m1+i] = whh;
		s->sr[s->m0+s->m1+s->m2+i] = -wh;
		s->si[s->m0+s->m1+s->m2+i] = -whh;
	    }
	    else {
		whh = w*sqrt(h*h - 1.0);
		s->sr[s->m0+s->m1+i] = -wh + whh;
		s->si[s->m0+s->m1+i] = 0.0;
		s->sr[s->m0+s->m1+s->m2+i] = -wh - whh;
		s->si[s->m0+s->m1+s->m2+i] = 0.0;
	    }
	}

	/*  calculate residues
	 */
	for(i = s->m0; i < s->m0+s->m1+2*s->m2; i++)
	{
	    cresr = s->syspar[0];
	    cresi = 0.0;
	    spowr = 1.0;
	    spowi = 0.0;
	    for(k = 0; k < s->m0; k++) {
		spowre = spowr*s->sr[i] - spowi*s->si[i];
		spowi =  spowr*s->si[i] + spowi*s->sr[i];
		spowr = spowre;
		cresr += s->syspar[1+k]*spowr;
		cresi += s->syspar[1+k]*spowi;
	    }
	    for(k = s->m0; k < s->m0+s->m1+2*s->m2; k++) {
		if(k != i) {
		    double sdr = s->sr[i] - s->sr[k];
		    double sdi = s->si[i] - s->si[k];
		    double sdq = sdr*sdr + sdi*sdi;
		    double cresre = (cresr*sdr + cresi*sdi)/sdq;
		    cresi = (cresi*sdr - cresr*sdi)/sdq;
		    cresr = cresre;
		}
	    }
	    s->resid[i] = cresr;
	    s->residi[i] = cresi;
	}
	return 0;
}

static void
CalibRekf1(double dt, double r, int n, double *ein, double *sum)
{
	int i;
	double aus0, z;

	z = exp(dt);
	aus0 = r*ein[0];
	sum[0] += aus0;

	for(i = 1; i < n; i++) {
	    aus0 = r*ein[i] + z*aus0;
	    sum[i] += aus0;
	}
}

static void
CalibRekf2(double sre, double sim, double rre, double rim, int n,
		double *ein, double *sum)
{
	int i;
	double zabs, zre, zim, f0, f1, g1, g2, aus0, aus1, aus2;

	zabs = exp(sre);
	zre = zabs*cos(sim);
	zim = zabs*sin(sim);
	f0 = 2.0*rre;
	f1 = -2.0*(rre*zre + rim*zim);
	g1 = 2.0*zre;
	g2 = -zabs*zabs;
	aus0 = f0*ein[0];
	sum[0] += aus0;
	aus1 = aus0;
	aus0 = f0*ein[1] + f1*ein[0] + g1*aus1;
	sum[1] += aus0;

	for(i = 2; i < n; i++) {
	    aus2 = aus1;
	    aus1 = aus0;
	    aus0 = f0*ein[i] + f1*ein[i-1] + g1*aus1 +g2*aus2;
	    sum[i] += aus0;
	}
}

/* remove the polynomial trend
 */
static void
CalibPolyTrend(int mm, double *x, int n)
{
	double b[ndi], c[ndi][ndi], a[ndi];
	double fnh, xpol, fac;
	int i, j, k, m;

	m = (mm < ndi-1) ? mm : ndi-1;
	fnh = (double)n/2.;

	for(j = 0; j <= m; j++)
	{
	    for(k = 0; k <= m; k++)
	    {
		c[j][k] = 0.0;

		for(i = 0; i < n; i++) {
		    c[j][k] += pow((double)(i+1)/fnh - 1.0, j+k);
		}
	    }
	    b[j] = 0.0;
	    for(i = 0; i < n; i++) {
		b[j] += pow((double)(i+1)/fnh - 1.0, j) * x[i];
	    }
	}

	CalibGauss(c, m+1, ndi, b, a);

	for(i = 0; i < n; i++)
	{
	    fac = (double)(i+1)/fnh - 1.0;
	    xpol = a[m];
	    for(j = m-1; j >= 0; j--) {
		xpol = xpol*fac + a[j];
	    }
	    x[i] -= xpol;
	}
}

/* solve linear equations
 */
static void
CalibGauss(double aik[ndi][ndi], int m, int n, double *rs, double *f)
{
	int j, k, l, ndex = 0;
	double h[ndi+1], imax[ndi];
	double aikmax, q;

	for(j = 0; j < m; j++)
	{
	    aikmax = 0.0;
	    for(k = 0; k < m; k++)
	    {
		h[k] = aik[j][k];
		if(fabs(h[k]) > aikmax) {
		    aikmax = fabs(h[k]);
		    ndex = k;
		}
	    }
	    h[m] = rs[j];
	    for(k = 0; k < m; k++) {
		q = aik[k][ndex]/h[ndex];
		for(l = 0; l < m; l++) {
		    aik[k][l] = aik[k][l] - q*h[l];
		}
		rs[k] = rs[k] - q*h[m];
	    }
	    for(k = 0; k < m; k++) {
		aik[j][k] = h[k];
	    }
	    rs[j] = h[m];
	    imax[j] = ndex;
	}
	for(j = 0; j < m; j++)
	{
	    ndex = (int)imax[j];
	    f[ndex] = rs[j]/aik[j][ndex];
	}
}

int
CalibOutput(char *file, char *text, double *x, int npts, double dt, double tmin,
		double tsec)
{
	int j, k, nvor, ndec;
	char error[MAXPATHLEN+1], format[20];
	double xmax;
	FILE *fp;

	if(!(fp = fopen(file, "w"))) {
	    snprintf(error, sizeof(error), "Cannot open file %s\n%s",
			file, strerror(errno));
	    logErrorMsg(LOG_ERR, error);
	    return -1;
	}
	xmax = 0.0;
	for(j = 0; j < npts; j++) {
	   if(fabs(x[j]) > xmax) xmax = fabs(x[j]);
	}
	nvor = (int)(log10(xmax > 2. ? xmax : 2.) + 1.);
	ndec = 10 - nvor;
	if(ndec < 0) ndec = 0;
	snprintf(format, sizeof(format), "%%13.%df", ndec);
	
	if(fprintf(fp, "%s\n", text) < 0) goto ERROR_RETURN;
	if(fprintf(fp, "%10d(5f13.%1d)            %10.6f%10.3f%10.3f\n",
		npts, ndec, dt, tmin, tsec) < 0) goto ERROR_RETURN;
	for(j = k = 0; j < npts; j++) {
	    if(fprintf(fp, format, x[j]) < 0) goto ERROR_RETURN;
	    if(++k == 5) {
		if(fprintf(fp, "\n") < 0) goto ERROR_RETURN;
		k = 0;
	    }
	}
	if(k != 0) if(fprintf(fp, "\n") < 0) goto ERROR_RETURN;

	fclose(fp);

	return 0;

    ERROR_RETURN:
	fclose(fp);
	return -1;
}
