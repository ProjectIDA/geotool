/** \file ftrace.cpp
 *  \brief Defines functions of the Beam class for computing the F-trace.
 *  \author Ivan Henson
 *
 * Translated from a Fortran program written by Dave Bowers.
 *
 * Calculates F-trace using a moving window 2*spts+1 long.
 *
 * Probability (using the F-statistic) that a signal is present above
 * bandlimited noise, with SNR equal to sqrt(signal/noise power)
 * (see Douze and Laster, 1979. Geophysics. 44, pp.1999 and
 * Blandford, 1974. Geophysics. 39, pp.633)
 *
 * Dave Bowers 03/01/2001
 * (bowers@blacknest.gov.uk)
 */
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifdef HAVE_GSL
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_fit.h>
#include "gsl/gsl_fft_real.h"
#include "gsl/gsl_fft_halfcomplex.h"
#include "gsl/gsl_errno.h"
#endif

#include "gobject++/GTimeSeries.h"
#include "gobject++/GSegment.h"
#include "gobject++/GSegmentArray.h"
#include "Beam.h"
#include "motif++/Component.h"
#include "gobject++/GCoverage.h"
extern "C" {
#include "libmath.h"
}

#ifdef HAVE_GSL
static void ctaper(int npts, double *data, int taper_len);
static void ftaper(int npts, float *data, int taper_len);
#endif


void Beam::ftrace(gvector<GTimeSeries *> &ts, double tmin, double tmax,
		double az, double slowness, double beam_lat, double beam_lon,
		int spts, int npols, double flow, double fhigh, bool zp,
		double snr, GTimeSeries *beam_ts, GTimeSeries *semb_ts,
		GTimeSeries *fst_ts, GTimeSeries *prob_ts)
{
#ifdef HAVE_GSL
    double rad = M_PI/180., ang;
    double tdel, skx, sky, c0, c1, cov00, cov01, cov11, sumsq, x, y, dist, baz;
    double *xd=NULL, *time=NULL, *tau=NULL;
    int i, j, k, taper_len;
    float **data=NULL, *beam=NULL, *semb=NULL, *fst=NULL, *prob=NULL;
    gvector<GSegmentArray*> *v;
    GSegmentArray *sa;

    if( ts.size() <= 0) return;

    v = GCoverage::getArrays(ts, tmin, tmax);
    if (v == NULL) {
      fprintf(stderr, "Failed to get segments for time %f - %f.\n", tmin, tmax);
      return;
    }
    tdel = ts[0]->segment(0)->tdel();
    az *= rad;
    skx = slowness*sin(az);
    sky = slowness*cos(az);

    data = new float *[ts.size()];
    for(k = 0; k < ts.size(); k++) data[k] = NULL;
    tau = new double[ts.size()];

    for(k = 0; k < v->size(); k++) {
	sa = v->at(k);
  
	for(j = 0; j < sa->num_segments; j++) {
	    data[j] = (float *)realloc(data[j], sa->npts*sizeof(float));
	}
	time = (double *)realloc(time, sa->npts*sizeof(double));
	xd = (double *)realloc(xd, sa->npts*sizeof(double));
	beam = (float *)realloc(beam, sa->npts*sizeof(float));
	semb = (float *)realloc(semb, sa->npts*sizeof(float));
	fst = (float *)realloc(fst, sa->npts*sizeof(float));
	prob = (float *)realloc(prob, sa->npts*sizeof(float));

	for(j = 0; j < sa->num_segments; j++)
	{
	    // remove linear trend, 1% cosine taper, time shift
            // and load into data array

	    for(i = 0; i < sa->npts; i++) {
		time[i] = (double)(i*tdel);
		xd[i] = (double)sa->segments[j]->data[sa->beg_index[j]+i];
	    }

	    gsl_fit_linear(time, 1, xd, 1, sa->npts, &c0, &c1, &cov00, &cov01,
				&cov11, &sumsq);
	    x = 0.;
	    for(i = 0; i < sa->npts; i++) {
		xd[i] = xd[i] - c0 - c1*x;
		x += tdel;
	    }
	    taper_len = (int)(sa->npts*0.01);

	    ctaper(sa->npts, xd, taper_len);

	    if(ts[j]->lat() != beam_lat || ts[j]->lon() != beam_lon) {
		deltaz(beam_lat, beam_lon, ts[j]->lat(), ts[j]->lon(),
			&dist, &az, &baz);
		ang = az*rad;
		dist *= 111.19492664;
		x = dist * sin(ang);
		y = dist * cos(ang);
		tau[j] = x * skx + y * sky;
		// dimensionless units for tau
		tau[j] = tau[j] / tdel;
	    }
	    else {
		tau[j] = 0.;
	    }

	    // time shift each channel
	    shiftByFT(sa->npts, xd, tau[j]);

	    // load time shifted channels
	    for(i = 0; i < sa->npts; i++) data[j][i] = (float)xd[i];
	}

	// sum beam

	for(i = 0; i < sa->npts; i++) beam[i] = 0.0;

	for(j = 0; j < sa->num_segments; j++) {
	    for(i = 0; i < sa->npts; i++) {
		beam[i] = beam[i] + data[j][i];
	    }
	}
	for(i = 0; i < sa->npts; i++) beam[i] /= (float)sa->num_segments;

	// filter beam
	filter(beam, sa->npts, tdel, npols, flow, fhigh, zp);

	beam_ts->addSegment(new GSegment(beam, sa->npts, sa->tmin, tdel,1.,0.));

	// Filter: corner frequencies at lf and hf with npol butterworth filter

	for(j = 0; j < sa->num_segments; j++) {
	    filter(data[j], sa->npts, tdel, npols, flow, fhigh, zp);
	}

	fstuff(sa->num_segments, data, sa->npts, tdel, spts, snr, flow, fhigh,
		semb, fst, prob);

	// cosine taper and write out semblance, F, probability

	taper_len = 2*spts+1;
	ftaper(sa->npts, semb, taper_len);

	semb_ts->addSegment(new GSegment(semb, sa->npts, sa->tmin, tdel,1.,0.));
	fst_ts->addSegment(new GSegment(fst, sa->npts, sa->tmin, tdel, 1.,0.));
	prob_ts->addSegment(new GSegment(prob, sa->npts, sa->tmin, tdel,1.,0.));
    }

    for(i = 0; i < ts.size(); i++) free(data[i]);
    delete [] data;
    delete [] tau;
    free(time);
    free(xd);
    free(beam);
    free(semb);
    free(fst);
    free(prob);

    GTimeSeries *t[4];
    t[0] = beam_ts;
    t[1] = semb_ts;
    t[2] = fst_ts;
    t[3] = prob_ts;

    for(i = 0; i < 4; i++) {
	t[i]->setDataSource(ts[0]->getDataSource());
	t[i]->copyWfdiscPeriods(ts[0]); // to addArrivals
	t[i]->setSta(ts[0]->net());
	t[i]->setNet(ts[0]->net());
	t[i]->setLat(beam_lat);
	t[i]->setLon(beam_lon);
	t[i]->setComponent(1);
    }
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

void Beam::ftrace(gvector<GTimeSeries *> &ts, double tmin, double tmax,
	vector<double> &tlags, int spts, int npols, double flow, double fhigh,
	bool zp, double snr, GTimeSeries *beam_ts, GTimeSeries *semb_ts,
	GTimeSeries *fst_ts, GTimeSeries *prob_ts)
{
#ifdef HAVE_GSL
    double tdel, c0, c1, cov00, cov01, cov11, sumsq, x;
    double *xd=NULL, *time=NULL, *tau=NULL;
    int i, j, k, taper_len;
    float **data=NULL, *beam=NULL, *semb=NULL, *fst=NULL, *prob=NULL;
    gvector<GSegmentArray*> *v;
    GSegmentArray *sa;

    if(ts.size() <= 0) return;
    for(i = (int)tlags.size(); i < ts.size(); i++) tlags.push_back(0.);

    v = GCoverage::getArrays(ts, tmin, tmax);
    tdel = ts[0]->segment(0)->tdel();

    data = new float *[ts.size()];
    for(k = 0; k < ts.size(); k++) data[k] = NULL;

    // dimensionless units for tau
    tau = new double[ts.size()];
    for(i = 0; i < ts.size(); i++) tau[i] = tlags[i]/tdel;

    for(k = 0; k < v->size(); k++) {
	sa = v->at(k);
  
	for(j = 0; j < sa->num_segments; j++) {
	    data[j] = (float *)realloc(data[j], sa->npts*sizeof(float));
	}
	time = (double *)realloc(time, sa->npts*sizeof(double));
	xd = (double *)realloc(xd, sa->npts*sizeof(double));
	beam = (float *)realloc(beam, sa->npts*sizeof(float));
	semb = (float *)realloc(semb, sa->npts*sizeof(float));
	fst = (float *)realloc(fst, sa->npts*sizeof(float));
	prob = (float *)realloc(prob, sa->npts*sizeof(float));

	for(j = 0; j < sa->num_segments; j++)
	{
	    // remove linear trend, 1% cosine taper, time shift
            // and load into data array

	    for(i = 0; i < sa->npts; i++) {
		time[i] = (double)(i*tdel);
		xd[i] = (double)sa->segments[j]->data[sa->beg_index[j]+i];
	    }

	    gsl_fit_linear(time, 1, xd, 1, sa->npts, &c0, &c1, &cov00, &cov01,
				&cov11, &sumsq);
	    x = 0.;
	    for(i = 0; i < sa->npts; i++) {
		xd[i] = xd[i] - c0 - c1*x;
		x += tdel;
	    }
	    taper_len = (int)(sa->npts*0.01);

	    ctaper(sa->npts, xd, taper_len);

	    // time shift each channel
	    shiftByFT(sa->npts, xd, tau[j]);

	    // load time shifted channels
	    for(i = 0; i < sa->npts; i++) data[j][i] = (float)xd[i];
	}

	// sum beam

	for(i = 0; i < sa->npts; i++) beam[i] = 0.0;

	for(j = 0; j < sa->num_segments; j++) {
	    for(i = 0; i < sa->npts; i++) {
		beam[i] = beam[i] + data[j][i];
	    }
	}
	for(i = 0; i < sa->npts; i++) beam[i] /= (float)sa->num_segments;

	// filter beam
	filter(beam, sa->npts, tdel, npols, flow, fhigh, zp);

	beam_ts->addSegment(new GSegment(beam, sa->npts, sa->tmin, tdel,1.,0.));

	// Filter: corner frequencies at lf and hf with npol butterworth filter

	for(j = 0; j < sa->num_segments; j++) {
	    filter(data[j], sa->npts, tdel, npols, flow, fhigh, zp);
	}

	fstuff(sa->num_segments, data, sa->npts, tdel, spts, snr, flow, fhigh,
		semb, fst, prob);

	// cosine taper and write out semblance, F, probability

	taper_len = 2*spts+1;
	ftaper(sa->npts, semb, taper_len);

	semb_ts->addSegment(new GSegment(semb, sa->npts, sa->tmin, tdel,1.,0.));
	fst_ts->addSegment(new GSegment(fst, sa->npts, sa->tmin, tdel, 1.,0.));
	prob_ts->addSegment(new GSegment(prob, sa->npts, sa->tmin, tdel,1.,0.));
    }

    for(i = 0; i < ts.size(); i++) free(data[i]);
    delete [] data;
    delete [] tau;
    free(time);
    free(xd);
    free(beam);
    free(semb);
    free(fst);
    free(prob);

    GTimeSeries *t[4];
    t[0] = beam_ts;
    t[1] = semb_ts;
    t[2] = fst_ts;
    t[3] = prob_ts;

    for(i = 0; i < 4; i++) {
	t[i]->setDataSource(ts[0]->getDataSource());
	t[i]->copyWfdiscPeriods(ts[0]); // to addArrivals
	t[i]->setSta(ts[0]->net());
	t[i]->setNet(ts[0]->net());
	t[i]->setComponent(1);
    }
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

#ifdef HAVE_GSL
void Beam::filter(float *y, int npts, double tdel, int npols, double flow,
		double fhigh, bool zp)
{
    int i, n2;
    float f;

    if(zp) {
	//  two-pass filtering (load data in reverse order)
	n2 = npts/2;
	for(i = 0; i < n2; i++) {
	    f = y[i];
	    y[i] = y[npts-1-i];
	    y[npts-1-i] = f;
	}
    }

    recbut(y, npts, tdel, npols, flow, fhigh);

    if(zp) {
	for(i = 0; i < n2; i++) {
	    f = y[i];
	    y[i] = y[npts-1-i];
	    y[npts-1-i] = f;
	}
	recbut(y, npts, tdel, npols, flow, fhigh);
    }
}

/*
 * Routine to compute the recursive filter coefficients of a nrdr pole
 * Butterworth bandpass filter, with corner frequencies at lfr and hfr.
 * Apply filter to data in array XR (with N points and sampling rate DELR)
 * Return filtered data as array YR.
 *
 * The filter is normalised to a gain of one in the pass band. The theory
 * is given in "Time sequence analysis in geophysics" by E. R.
 * Kanasewich (pages 181-199).
 *
 * See AG/355 (Douglas 1993) for further details.
 */
void Beam::recbut(float *xr, int n, double dela, int nrdr, double lfr,
			double hfr)
{
    double a, b, c, PI2, wp, dw, con, tbd, theta, ang2, ang, xm1, xm2;
    gsl_complex p[50], pw;
    double h1[50], h2[50], fo1[50], fo2[50], fo3[50];
    int i, j, noby2, nlim;

    if(n < 3) return;

    PI2 = 2.0*M_PI;

    tbd = 2.0/dela;
    hfr = hfr*PI2;
    lfr = lfr*PI2;

    // Modify the cut-off frequencies.    

    hfr = tbd*tan(hfr/tbd);
    lfr = tbd*tan(lfr/tbd);
    dw = hfr-lfr;
    wp = hfr*lfr;
    noby2 = nrdr/2;
    ang = M_PI/(float)(nrdr*2);
    ang2 = ang*2.0;
    con = 1.0/pow(dw, nrdr);

    // Loop to set up poles and filter coefficients using equations 
    // 13.6.17 and 13.7.7.

    for(i = 0; i < noby2; i++)
    {
	j = nrdr-1-i;
	theta = i*ang2+ang;
	GSL_SET_COMPLEX(&pw, -cos(theta)*dw/2., sin(theta)*dw/2.);
	p[i] = gsl_complex_mul(pw, pw);
	p[i] = gsl_complex_sub_real(p[i], wp);
	p[i] = gsl_complex_sqrt(p[i]);

	p[nrdr+i] = gsl_complex_sub(pw, p[i]);
	p[i] = gsl_complex_add(pw, p[i]);
	p[j] = gsl_complex_conjugate(p[i]);
	pw = gsl_complex_add(p[i], p[j]);
	pw = gsl_complex_negative(pw);
	b = GSL_REAL(pw);
	pw = gsl_complex_mul(p[i], p[j]);
	c = GSL_REAL(pw);
	a = 2.0/dela + b + c*dela/2.0;
	h1[i] = (c*dela - 4.0/dela)/a;
	h2[i] = (2.0/dela - b + c*dela/2.0)/a;
	con = con*a;
	p[nrdr+j] = gsl_complex_conjugate(p[nrdr+i]);
	pw = gsl_complex_add(p[nrdr+i], p[nrdr+j]);
	pw = gsl_complex_negative(pw);
	b = GSL_REAL(pw);
	pw = gsl_complex_mul(p[nrdr+i], p[nrdr+j]);
	c = GSL_REAL(pw);
	a = 2.0/dela + b + c*dela/2.0;
	h1[noby2+i] = (c*dela-4.0/dela)/a;
	h2[noby2+i] = (2.0/dela-b+c*dela/2.0)/a;
	con *= a;
    }
    con = 1.0/con;
    /*
     * p[0] to p[nrdr-1] are the poles for the low frequency cut-off and 
     * p[nrdr] to p[2*nrdr-1] the poles for the high frequency cut-off.
     * a,b,c are equivalent to a(sub j), b(sub j) and c(sub j) for stage
     * j of the filter (see equation 13.7.7 of Kanasewich).
     * h1[j] and h2[j] are the filter coefficients for stage j. con is
     * the normalizing factor. 
     * 
     * 
     * Apply nrdr stage recursive filter to X array. fo1[j] is output of 
     * stage j at time i. fo2[j] is output at time i-1 and fo3[j] is output
     * at time i-2. The Y array is the filter output. 
     */

    nlim = nrdr+1;
    for(j = 0; j < nlim; j++) {
	fo1[j] = 0.0;
	fo2[j] = 0.0;
	fo3[j] = 0.0;
    }

    xm2 = xr[0];
    xm1 = xr[1];
    for(i = 2; i < n; i++) {
	fo1[0] = xr[i];
	fo2[0] = xm1;	// xr[i-1]
	fo3[0] = xm2;	// xr[i-2]
	for(j = 1; j < nlim; j++) {
	    fo1[j] = fo1[j-1] - fo3[j-1] - fo2[j]*h1[j-1] - fo3[j]*h2[j-1];
	}
	for(j = 1; j < nlim; j++) {
	    fo3[j] = fo2[j];
	    fo2[j] = fo1[j];
	}
	xm2 = xm1;
	xm1 = xr[i];
	xr[i] = fo1[nlim-1]*con;
    }
}

void Beam::fstuff(int num, float *data[], int npts, double tdel, int spts,
	float snr, float lf, float hf, float *semb, float *fst, float *prob)
{
    int i, j, k, smv, nwin, nn1, nn2, nc1, lambda, status;
    float fnn1, fprime, sum0, sum1, sum2, sum3;
    gsl_sf_result result;

    // calculate semblance for a moving 2*spts+1 sample window
    // and probability of the presence of a signal in band-limited white noise.
    // dof nn1 = 2BT, where B = hf - lf (Hz), T = time window (sec).
    //     nn2 = nn1(num-1)
    // non-centrality parameter, lambda = nn1 * (snr)**2 (Blandford 1974)
    if(num <= 0) return;
    
    gsl_error_handler_t *handler = gsl_set_error_handler_off();

    nwin = 2*spts;
    fnn1 = 2.*(hf-lf)*nwin*tdel;
    nn1 = (int)fnn1;
    if(nn1 < 1) nn1 = 1;
    nn2 = nn1*(num-1);

    // probability of a non-central F-distribution;
    // Abramowitz and Stegun (1964) P(F'|nn1,nn2,lambda) = P(F|nc1,nn2)
    // where F = (nn1 * F')/(nn1 + lambda), and 
    //     nc1 = (nn1 + lambda)**2/(nn1 + 2*lambda)

    lambda = (int)(fnn1*snr*snr);
    nc1 = (nn1+lambda)*(nn1+lambda)/(nn1+2*lambda);

    for(i = 0; i < npts; i++) {
	semb[i] = 0.;
	fst[i] = 0.;
	prob[i] = 0.;
    }

    // calculate the semblance.

    for(k = 0; k < npts-nwin; k++) {
	 smv = k + spts;

	// calculate semblance

	// top: sum1 = sum of square of sum
	// bottom: sum2 = sum of sum of squares
	sum1 = 0.0;
	sum3 = 0.0;
	for(i = k; i <= k+nwin; i++) {
	    sum0 = 0.0;
	    sum2 = 0.0;
	    for(j = 0; j < num; j++) {
		sum0 += data[j][i];
		sum2 += data[j][i]*data[j][i];
            }
	    sum1 += sum0*sum0;
	    sum3 += sum2;
	}
	// semblance
	semb[smv] = sum1 / ((float)num * sum3);

	// F-statistic F = S(num-1) / (1-S)
	if(semb[smv] != 0.) {
	    fst[smv] = semb[smv] * (float)(num-1) / (1. - semb[smv]);
	}
	else {
	    fst[smv] = semb[smv] * 1.e+30;
	}

	// probability of a non-central F-distribution.

	fprime = ((float)nn1*fst[smv])/(float)(nn1 + lambda);
	status = gsl_sf_beta_inc_e(0.5*nn2, 0.5*nc1, nn2/(nn2+nc1*fprime),
			&result);
	if(!status) {
	    prob[smv] = 1. - result.val;
	}
	else {
	    prob[smv] = 0.;
	}
    }
    gsl_set_error_handler(handler);
}

// t0 is the shift in sample intervals: time_shift = t0*dt.
// t0 = time_shift/dt

void Beam::shiftByFT(int npts, double *data, double t0)
{
    int i, n2, np2;
    double *f, re, im;
    gsl_complex e, e1, arg, c;

    for(np2 = 2; np2 < npts; np2 *= 2);
    f = new double[np2];

    for(i = 0; i < npts; i++) f[i] = data[i];
    for(i = npts; i < np2; i++) f[i] = 0.;
    n2 = np2/2;

    gsl_fft_real_radix2_transform(f, 1, np2);

    // shift in frequency domain by multiplying by exp(i*2*PI*f*t)
    // f = j*df
    // exp(i * 2*PI * j*df * t)
    // df = 1./(dt*np2); t = t0*dt;  df * t = t0/np2;
    // multiply by exp(i*2*PI*j*t0/np2)

    GSL_SET_COMPLEX(&arg, 0.0, -2.*M_PI*t0/np2);
    e1 = gsl_complex_exp(arg);
    e = e1;

    // f[0] not changed.
    for(i = 1; i < n2; i++) {
	re = f[i];
	im = f[np2-i];
	GSL_SET_COMPLEX(&c, re, im);
	c = gsl_complex_mul(c, e);
	e = gsl_complex_mul(e, e1);
	f[i] = GSL_REAL(c);
	f[np2-i] = GSL_IMAG(c);
    }
    GSL_SET_COMPLEX(&c, f[n2], 0.0);
    c = gsl_complex_mul(c, e);
    f[n2] = GSL_REAL(c);

    gsl_fft_halfcomplex_radix2_inverse(f, 1, np2);

    for(i = 0; i < npts; i++) data[i] = f[i];

    delete [] f;
}

static void
ctaper(int npts, double *data, int taper_len)
{
    double ang = M_PI/(2.*taper_len), cs;

    for(int i = 0; i < taper_len; i++) {
	cs = cos(i*ang);
	data[taper_len-1-i] *= cs*cs;
	data[npts-taper_len-1+i] *= cs*cs;
    }
}

static void
ftaper(int npts, float *data, int taper_len)
{
    double ang = M_PI/(2.*taper_len), cs;

    for(int i = 0; i < taper_len; i++) {
	cs = cos(i*ang);
	data[taper_len-1-i] *= cs*cs;
	data[npts-taper_len-1+i] *= cs*cs;
    }
}
#endif
