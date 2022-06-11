/** \file Response.cpp
 *  \brief Defines class Response.
 *  \author Ivan Henson
 */
#include "config.h"
#include <strings.h>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <sys/param.h>
#ifdef HAVE_GSL
#include <gsl/gsl_spline.h>
#include "gsl/gsl_fft_real.h"
#include "gsl/gsl_fft_halfcomplex.h"
#include "gsl/gsl_errno.h"
#endif
using namespace std;


#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#include "Response.h"
#include "ResponseFile.h"
#include "DataMethod.h"
extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

static void cmplx_mult(int n, double *ar, double *ai, double *br, double *bi);
#ifdef HAVE_GSL
static double interpResp(gsl_interp *interp, gsl_interp_accel *acc,
			double *f, double *a, int n, double freq);
#endif

/** Constructor. Initially the Response object does not contain any response
 *  information.
 */
Response::Response(void) :
    stage(0), a0(1.), b58_sensitivity(1.), b58_frequency(-1.), npoles(0),
    nzeros(0), pole(NULL), pole_err(NULL), zero(NULL), zero_err(NULL), nfap(0),
    fap_f(NULL), fap_a(NULL), fap_p(NULL), amp_error(NULL), phase_error(NULL),
    input_samprate(0.), output_samprate(0.), delay(0.), correction(0.),
    num_n(0), num_d(0), fir_n(NULL), fir_n_error(NULL), fir_d(NULL),
    fir_d_error(NULL), cal(NULL)
{
}

Response::Response(const Response &r) :
    stage(r.stage), a0(r.a0), b58_sensitivity(1.), b58_frequency(-1.),
    npoles(r.npoles),nzeros(r.nzeros), pole(NULL), pole_err(NULL), zero(NULL),
    zero_err(NULL), nfap(r.nfap), fap_f(NULL), fap_a(NULL), fap_p(NULL),
    amp_error(NULL), phase_error(NULL), input_samprate(r.input_samprate),
    output_samprate(r.output_samprate), delay(r.delay),correction(r.correction),
    num_n(r.num_n), num_d(r.num_d), fir_n(NULL), fir_n_error(NULL), fir_d(NULL),
    fir_d_error(NULL), cal(NULL)
{
    copy(r);
}

Response & Response::operator=(const Response &r)
{
    source = r.source;
    stage = r.stage;
    des = r.des;
    type = r.type;
    input_units = r.input_units;
    response_units = r.response_units;
    output_units = r.output_units;
    author = r.author;
    a0 = r.a0;
    b58_sensitivity = r.b58_sensitivity;
    b58_frequency = r.b58_frequency;
    npoles = r.npoles;
    nzeros = r.nzeros;
    nfap = r.nfap;
    input_samprate = r.input_samprate;
    output_samprate = r.output_samprate;
    delay = r.delay;
    correction = r.correction;
    num_n = r.num_n;
    num_d = r.num_d;

    copy(r);
    return *this;
}

void Response::copy(const Response &r)
{
    pole = (FComplex *)malloc(npoles*sizeof(FComplex));
    memcpy(pole, r.pole, npoles*sizeof(FComplex));
    pole_err = (FComplex *)malloc(npoles*sizeof(FComplex));
    memcpy(pole_err, r.pole_err, npoles*sizeof(FComplex));

    zero = (FComplex *)malloc(nzeros*sizeof(FComplex));
    memcpy(zero, r.zero, nzeros*sizeof(FComplex));
    zero_err = (FComplex *)malloc(nzeros*sizeof(FComplex));
    memcpy(zero_err, r.zero_err, nzeros*sizeof(FComplex));

    fap_f = (float *)malloc(nfap*sizeof(float));
    memcpy(fap_f, r.fap_f, nfap*sizeof(float));
    fap_a = (float *)malloc(nfap*sizeof(float));
    memcpy(fap_a, r.fap_a, nfap*sizeof(float));
    fap_p = (float *)malloc(nfap*sizeof(float));
    memcpy(fap_p, r.fap_p, nfap*sizeof(float));
    amp_error = (float *)malloc(nfap*sizeof(float));
    memcpy(amp_error, r.amp_error, nfap*sizeof(float));
    phase_error = (float *)malloc(nfap*sizeof(float));
    memcpy(phase_error, r.phase_error, nfap*sizeof(float));

    fir_n = (float *)malloc(num_n*sizeof(float));
    memcpy(fir_n, r.fir_n, num_n*sizeof(float));
    fir_d = (float *)malloc(num_d*sizeof(float));
    memcpy(fir_d, r.fir_d, num_d*sizeof(float));

    if(r.cal) {
	cal = (GSE_CAL *)malloc(sizeof(GSE_CAL));
	cal = r.cal;
    }
    else cal = NULL;
}

/** Destructor. */
Response::~Response(void)
{
    Free(pole);
    Free(pole_err);
    Free(zero);
    Free(zero_err);
    Free(fap_f);
    Free(fap_a);
    Free(fap_p);
    Free(amp_error);
    Free(phase_error);
    Free(fir_n);
    Free(fir_d);
    Free(fir_n_error);
    Free(fir_d_error);
    Free(cal);
}

/*
 * NAME
 * AUTHOR
 *	Ivan Henson --- Nov 1991
 *	Teledyne Geotech
 */

/** Convolve or deconvolve instrument responses. Convolve or deconvolve one or
 *  more instrument responses with the input data array. For deconvolution,
 *  the amplitude spectrum is cosine tapered from frequencies 0 to flo and
 *  from fhi to the nyquist frequency, and low amplitude values are restricted
 *  to be > amp_cutoff*max_amp before the response is inverted.
 *  @param[in] resp a Vector of one or more Response objects.
 *  @param[in] direction 1: convolve, -1: deconvolve.
 *  @param[in] data an array of float data values.
 *  @param[in] npts the number of data values.
 *  @param[in] dt the sample time interval.
 *  @param[in] flo the low frequence.
 *  @param[in] fhi the high frequence.
 *  @param[in] amp_cutoff for deconvolution, the lowest response amplitude 
 *  	to be included in the inverse response. Before the response is
 *	inverted, amplitudes values lower than amp_cutoff * the maximum
 *	amplitude are set to this value.
 *  @param[in] calib The calibration factor. Not used if calper <= 0.
 *  @param[in] calper The calibration period or <= 0. if not used.
 *  @param[in] remove_time_shift Remove the response time shift.
 *  @returns true for success, false for error.
 *  @throws GERROR_MALLOC_ERROR
 */
bool Response::convolve(vector<Response *> *resp, int direction, float *data,
		int npts, double dt, double flo, double fhi, double amp_cutoff,
		double calib, double calper, bool remove_time_shift)
{
#ifdef HAVE_GSL
    int i, nf, np2, n, n2;
    double df, re, im, nyquist, den;
    double *real=NULL, *imag=NULL, *f=NULL;

    if(dt <= 0. || npts <= 0) return true;

    /*
     * allow space for a 1 minute impulse response length
     * need time domain space for n = npts + 60./dt
     */
    n = (int)(npts + 60./dt);
    for(np2 = 2; np2 < n; np2 *= 2);

    df = 1./(dt*np2);
    nf = np2/2 + 1;
    n2 = np2/2;

    if(!(real = (double *)mallocWarn(nf*sizeof(double)))) return false;
    if(!(imag = (double *)mallocWarn(nf*sizeof(double)))) return false;

    nyquist = 1./(2.*dt);

    if( !compute(resp, 0., nyquist, calib, calper, nf, real, imag) )
    {
	Free(real);
	Free(imag);
	return false;
    }
    if(remove_time_shift) {
	removeTimeShift(nf, real, imag);
    }

    if(direction == -1) { // deconvolve
	ampCutoff(real, imag, nf, amp_cutoff);
	for(i = 0; i < nf; i++) {
	    den = real[i]*real[i] + imag[i]*imag[i];
	    if(den != 0.) {
		real[i] =  real[i]/den;
		imag[i] = -imag[i]/den;
	    }
	}
    }
    taperAmp(real, imag, df, nf, flo, fhi);

    if(!(f = (double *)mallocWarn(np2*sizeof(double)))) return false;

    for(i = 0; i < npts; i++) f[i] = (double)data[i];
    for(i = npts; i < np2; i++) f[i] = 0.;

    gsl_fft_real_radix2_transform(f, 1, np2);

    // convolve
    f[0] = f[0]*real[0];
    f[n2] = f[n2]*real[n2];
    for(i = 1; i < n2; i++)
    {
	re = f[i];
	im = f[np2-i];
	f[i] = re*real[i] - im*imag[i];
	f[np2-i] = re*imag[i] + im*real[i];
    }
    Free(real);
    Free(imag);

    /*
     * perform the inverse fft
     */
    gsl_fft_halfcomplex_radix2_inverse(f, 1, np2);

    for(i = 0; i < npts; i++) data[i] = (float)f[i];

    Free(f);

    return true;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return false;
#endif
}

/** FIR frequency domain convolution. A convolution or deconvolution is
 *  performed by multiplying or dividing the input data spectrum by the FIR
 *  response.
 *  @param[in] direction 1: convolve, -1: deconvolve.
 *  @param[in] f the complex data spectrum.
 *  @param[in] np2 the number of complex values in f[].
 *  @param[in] dt the sample time interval.
 *  @param[in] remove_time_shift if true, remove any time shift.
 *  @returns true for success; returns false if np2 <= 0 or num_n <= 0 or
 *	dt <= 0.
 *  @throws GERROR_MALLOC_ERROR
 */
bool Response::firConvolve(int direction, FComplex *f, int np2, double dt,
			bool remove_time_shift)
{
    int i, nf;
    double nyquist, real, imag, *re, *im;

    if(np2 <= 0 || num_n <= 0 || dt <= 0.) return false;

    nf = np2/2 + 1;
    if(!(re = (double *)mallocWarn(nf*sizeof(double)))) {
	GError::setMessage("Response.firConvolve: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
    if(!(im = (double *)mallocWarn(nf*sizeof(double)))) {
	GError::setMessage("Response.firConvolve: malloc failed.");
	Free(re);
	throw GERROR_MALLOC_ERROR;
    }

    nyquist = 1./(2.*dt);

    computeFIR(0., nyquist, nf, re, im);

    if(remove_time_shift) {
	removeTimeShift(nf, re, im);
    }

    if(direction == 1)
    {
	    /* convolve */
	for(i = 0; i < nf; i++)
	{
	    real = f[i].re;
	    imag = f[i].im;
	    f[i].re = real*re[i] - imag*im[i];
	    f[i].im = real*im[i] + imag*re[i];
	}
    }
    else
    {
	double den;
	/* deconvolve */
	for(i = 0; i < nf; i++)
	{
	    real = f[i].re;
	    imag = f[i].im;
	    f[i].re = real*re[i] + imag*im[i];
	    f[i].im = imag*re[i] - real*im[i];
	    den = re[i]*re[i] + im[i]*im[i];

	    if(den != 0.) {
		f[i].re /= den;
		f[i].im /= den;
	    }
	}
    }
    Free(re); Free(im);
    return true;
}

/** FAP frequency domain convolution. A convolution or deconvolution is
 *  performed by multiplying or dividing the input data spectrum by the FAP
 *  response.
 *  @param[in] direction 1: convolve, -1: deconvolve.
 *  @param[in] y the complex data spectrum.
 *  @param[in] np2 the number of complex values in f[].
 *  @param[in] dt the sample time interval.
 */
bool Response::fapConvolve(int direction, FComplex *y, int np2, double dt)
{
#ifdef HAVE_GSL
/*
 * NAME
 *	fapconvolve -- interpolate amp & phase from FAP data.  Multiply or
 *			divide f[] by response.
 *
 *	DESCRIPTION
 *	fapConvolve() interpolates an amplitude-phase frequency response
 *	(e.g., generated from FAP files) and multiplies or divides f[] by
 *	this response. The amplitude interpolation is done logarithmically,
 *	while the phase is done linearly.  The output f[] amplitude is
 *	cosine tapered on both of the response limits.
 *
 *	AUTHOR
 *	Ivan Henson --- Nov 1991
 *	Teledyne Geotech
 */
    int i, j, n, nf;
    double *f = NULL, *a = NULL, *p = NULL;
    double freq, amp, phase, df;
    double real, imag, fre, fim;
    double degrees_to_rad = M_PI/180.;
    gsl_interp *interp;
    gsl_interp_accel *acc;

    nf = np2/2 + 1;
    if(nf <= 0 || nfap <= 1) {
	return true;
    }
    f = new double[nfap];
    a = new double[nfap];
    p = new double[nfap];

    if(fap_f[0] == 0.) {
	fap_f[0] = 1.e-30;
	fap_a[0] = 1.e-30;
    }
    for(i = 0; i < nfap; i++)
    {
	f[i] = log10((double)fap_f[i]);
	a[i] = log10((double)fap_a[i]);
    }
    /* unwrap the phase */
    for(i = 0; i < nfap; i++) p[i] = (double)fap_p[i]*degrees_to_rad;
    unwrap(p, nfap);

    df = 1./(dt*np2);

    if(nfap >= 4) {
	interp = gsl_interp_alloc(gsl_interp_cspline, 4);
	n = 4;
    }
    else {
	interp = gsl_interp_alloc(gsl_interp_linear, nfap);
	n = nfap;
    }
    acc = gsl_interp_accel_alloc();

    for(i = 0; i < nf; i++)
    {
	freq = (i > 0) ? log10((double)(i*df)) : -50.;

	if(nfap >= 4) {
	    for(j = 0; j < nfap; j++) {
		if(f[j] > freq) break;
	    }
	    j -= 2;
	    if(j < 0) {
		j = 0;
	    }
	    else if(j > nfap - 4) {
		j = nfap - 2;
	    }
	}
	else {
	    j = 0;
	}
	amp = interpResp(interp, acc, f+j, a+j, n, freq);
	phase = interpResp(interp, acc, f+j, p+j, n, freq);

	amp = pow((double)10., (double)amp);
	real = amp*cos(phase);
	imag = amp*sin(phase);

	fre = y[i].re;
	fim = y[i].im;

	if(direction == 1)	/* convolve */
	{
	    y[i].re = real*fre - imag*fim;
	    y[i].im = real*fim + imag*fre;
	}
	else		/* deconvolve */
	{
	    if(amp != 0.)
	    {
		amp = amp*amp;
		y[i].re = (real*fre + imag*fim)/amp;
		y[i].im = (real*fim - imag*fre)/amp;
	    }
	    else
	    {
		y[i].re = y[i].im = 0.;
	    }
	}
    }
    gsl_interp_free(interp);
    gsl_interp_accel_free(acc);

    delete[] f;
    delete[] a;
    delete[] p;

    return true;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return false;
#endif
}

/** PAZ frequency domain convolution. A convolution or deconvolution is
 *  performed by multiplying or dividing the input data spectrum by the PAZ
 *  response.
 *  @param[in] direction 1: convolve, -1: deconvolve.
 *  @param[in,out] f the complex data spectrum.
 *  @param[in] np2 the number of complex values in f[].
 *  @param[in] dt the sample time interval.
 */
bool Response::pazConvolve(int direction, FComplex *f, int np2, double dt)
{
    int i, nf, np, nz;
    float twopi, re, im, omega, df;
    double real, imag, cnorm;
    FComplex *p, *z;

    twopi = 2.*M_PI;
    cnorm = a0;
    if(cnorm == 0.) cnorm = 1.;

    if(direction == -1) {
	p = zero;
	np = nzeros;
	z = pole;
	nz = npoles;
	cnorm = 1./cnorm;
    }
    else {
	p = pole;
	np = npoles;
	z = zero;
	nz = nzeros;
    }

    nf = np2/2 + 1;
    df = 1./(dt*np2);

    for(i = 0; i < nf; i++)
    {
	omega = i*df*twopi;
	computePAZ(omega, p, np, z, nz, cnorm, &real, &imag);
	re = f[i].re;
	im = f[i].im;
	f[i].re = re*real - im*imag;
	f[i].im = re*imag + im*real;
    }
    return true;
}

void Response::ampCutoff(double *real, double *imag, int nf, double amp_cutoff)
{
//    int i, mid;
    int i;
    double amp_max;

    if(amp_cutoff > 0.) {
	double *amps = new double[nf];
	for(i = 0; i < nf; i++) {
	    amps[i] = sqrt(real[i]*real[i] + imag[i]*imag[i]);
	}
	amp_max = 0.;
	for(i = 0; i < nf; i++) {
	    if(amp_max < amps[i]) amp_max = amps[i];
	}
	amp_cutoff *= amp_max;

 	for(i = 1; i < nf; i++) {
	    if(amps[i] < amp_cutoff) {
		if(amps[i] > 0.) {
		    real[i] *= amp_cutoff/amps[i];
		    imag[i] *= amp_cutoff/amps[i];
		}
		else {
		    real[i] = amp_cutoff;
		    imag[i] = 0.;
		}
	    }
	}
/*
	mid = nf/2;
	for(i = mid; i >= 0; i--) {
	    if(amps[i] < amp_cutoff && amps[i] > 0.) {
		real[i] *= amps[i+1]/amps[i];
		imag[i] *= amps[i+1]/amps[i];
		amps[i] = amps[i+1];
	    }
	}
	for(i = mid+1; i < nf; i++) {
	    if(amps[i] < amp_cutoff && amps[i] > 0.) {
		real[i] *= amps[i-1]/amps[i];
		imag[i] *= amps[i-1]/amps[i];
		amps[i] = amps[i-1];
	    }
	}
*/
	delete [] amps;
    }
}

void Response::taperAmp(double *real, double *imag, double df, int nf,
			double flo, double fhi)
{
    int i, taper_beg, taper_end;
    double arg, amp, phase, re, im, taper;

    // taper the amplitude spectrum

    if(flo > 0. && flo < (nf-1)*df)
    {
	taper_end = (int)(flo/df + .5);
	if(taper_end >= nf) taper_end = nf-1;
	re = real[taper_end];
	im = imag[taper_end];
	amp = sqrt(re*re + im*im);
	arg = M_PI/taper_end;
	for(i = 0; i < taper_end; i++)
	{
	    taper = .5*(1. - cos(i*arg));
	    re = real[i];
	    im = imag[i];
	    phase = (re == 0. && im == 0.) ? 0. : atan2(im, re);
	    real[i] = taper*amp*cos(phase);
	    imag[i] = taper*amp*sin(phase);
	}
    }
    if(fhi > 0. && fhi < (nf-1)*df)
    {
	taper_beg = (int)(fhi/df + .5);
	if(taper_beg >= nf) taper_beg = nf-1;
	re = real[taper_beg];
	im = imag[taper_beg];
	amp = sqrt(re*re + im*im);
	arg = M_PI/(nf-taper_beg);
	for(i = taper_beg+1; i < nf; i++)
	{
	    taper = .5*(1. - cos((nf-i)*arg));
	    re = real[i];
	    im = imag[i];
	    phase = (re == 0. && im == 0.) ? 0. : atan2(im, re);
	    real[i] = taper*amp*cos(phase);
	    imag[i] = taper*amp*sin(phase);
	}
    }
}

/*
 * NAME
 *	Taken from the fortran routine
 *	Source: geotech/src/libsrc/response/pazresp.f
 *		by T. W. McElfresh
 *	pazconvolve --  compute complex response from poles & zeros
 *			multiply or divide f[] by the response.
 * SYNOPSIS
 *	pazconvolve(direction,pole,npoles,zero,nzeros,cnorm,df,f,nf,calper)
 *	int  direction	(i) -1: deconvolve (divide f by the response)
 *			     1: convolve (multiply f by the response)
 *	FComplex *pole	(i) array of poles
 *	int	npoles	(i) number of poles
 *	FComplex *zero	(i) array of zeros
 *	int	nzeros	(i) number of zeros
 *	double	 cnorm	(i) normalization
 *	float df	(i) input frequency increment for f[]
 *	float *f	(i) input tranform: f[2*i]=real, f[2*i+1]=imag
 *	int   nf	(i) number of real,imaginary pairs in f[]
 *	float calper	(i) normalize response to be 1. at 1/calper frequency.
 *
 * DESCRIPTION
 * 	pazconvolve() computes the complex instrument response (real,imag)
 * 	for a given set of poles and zeros and convolves or deconvolves with
 *	the input tranform f[].
 *
 * 	Compute complex frequency response from poles and zeros. 
 *
 *			  M
 *		        _____
 *			 | |  ( iw  - zero[j] )
 *		  	 j=1
 *
 *	H(w)  = a0 * ------------------
 *			  N
 *			_____
 *			 | |  ( iw - pole[j] )
 *			 j=1
 *	where:
 *	H = complex response at frequency w = 2*pi*f
 *	iw = i*omega = i * 2 * pi * f
 *	a0 = normalizing constant
 *	M = number of zeros
 *	N = number of poles
 *
 * AUTHOR
 *	Ivan Henson --- Nov 1990
 *	Teledyne Geotech
 */
/** Compute a poles and zeros (PAZ) response. The PAZ response
 * \f$H(\omega)\f$ is
\f{eqnarray*}
H(\omega) = a_{0} \frac {\prod_{j=1}^{M} (i \omega - z_j )}
{\prod_{j=1}^{N} (i \omega - p_j ) }
\f}
 * where
 *
 * - \f$a_0\f$ = the normalization constant.
 * - \f$\omega\f$ = the frequency in radians (2\f$\pi f\f$).
 * - \f$M\f$ = num_zeros, the number of zeros.
 * - \f$N\f$ = num_poles, the number of poles.
 * - \f$z_{j}\f$ = zeros[j], the complex zeros.
 * - \f$p_{j}\f$ = poles[j], the complex poles.
 *  @param[in] omega the frequency in radians.
 *  @param[in] poles the poles.
 *  @param[in] num_poles the number of poles.
 *  @param[in] zeros the zeros.
 *  @param[in] num_zeros the number of zeros.
 *  @param[in] norm the normalization constant.
 *  @param[out] real the real part of the response at omega.
 *  @param[out] imag the imaginary part of the response at omega.
 */
void Response::computePAZ(double omega, FComplex *poles, int num_poles,
	FComplex *zeros, int num_zeros, double norm, double *real, double *imag)
{
    computepaz(omega, poles, num_poles, zeros, num_zeros, norm, real, imag);

    if(b58_frequency >= 0.) {
	double re, im, fac, omega;
	omega = 2.*M_PI*b58_frequency;
	computepaz(omega, poles, num_poles, zeros, num_zeros, norm, &re, &im);
	// normalize response amplitude to be b58_sensitivity at b58_frequency
	fac = sqrt(re*re + im*im);
	if(fac != 0.) {
	    fac = b58_sensitivity/fac;
	    *real *= fac;
	    *imag *= fac;
	}
    }
}

void Response::computepaz(double omega, FComplex *poles, int num_poles,
		FComplex *zeros, int num_zeros, double norm, double *real,
		double *imag)
{
    int j, nmin;
    double pre, pim, zre, zim, re, im, den;

    *real = 1.;
    *imag = 0.;

    if(num_poles <= 0 && num_zeros <= 0) return;

/**/
    nmin = (num_poles < num_zeros) ? num_poles : num_zeros;

    for(j = 0; j < nmin; j++)
    {
	pre = -poles[j].re;
	pim = omega - poles[j].im;
	den = pre*pre + pim*pim;
	if(den == 0.)
	{
	    *real = *imag = 0.;
	    break;
	}
	zre = -zeros[j].re;
	zim = omega - zeros[j].im;

	re = (pre*zre + pim*zim)/den;
	im = (pre*zim - pim*zre)/den;
	pre = *real;
	pim = *imag;

	*real = pre*re - pim*im;
	*imag = pre*im + pim*re;
    }
    for(j = nmin; j < num_zeros; j++)
/* */
//    for(j = 0; j < num_zeros; j++)
    {
	zre = -zeros[j].re;
	zim = omega - zeros[j].im;
	re = *real;
	im = *imag;
	*real = re*zre - im*zim;
	*imag = re*zim + im*zre;
    }
    for(j = nmin; j < num_poles; j++)
//    for(j = 0; j < num_poles; j++)
    {
	pre = -poles[j].re;
	pim = omega - poles[j].im;
	den = pre*pre + pim*pim;
	if(den == 0.)
	{
	    *real = *imag = 0.;
	    break;
	}
	re = *real;
	im = *imag;
	*real = (pre*re + pim*im)/den;
	*imag = (pre*im - pim*re)/den;
    }
    *real *= norm;
    *imag *= norm;
}

/**
 * Compute a frequency and amplitude response curve. Polynomial interpolation
 * (Neville's algorithm) is used to compute the response at the desired
 * frequencies from the FAP response values. Four FAP amplitude values (fap_a),
 * two below the desired frequency and two above the desired frequency, are used
 * in the polynomial interpolation. The frequency is converted to
 * log10(frequency) and the amplitude is converted to log10(amplitude) before
 * the interpolation is performed.
 *
 * The FAP phase values are first unwraped, then four log10(frequency) and the
 * four corresponding phase values are used in the interpolation of the desired
 * phase values.
 *
 * If lofreq == 0., it is set to 1.e-30. If fap_f[0] == 0., it is set to 1.e-30.
 * If fap_a[i] = 0, it is set to 1.e-30, before the log10() is taken. The
 * complex response \f$H(f)\f$ is computed from the intepolated amplitude
 * \f$a\f$ and the interpolated phase \f$\theta\f$ for nf frequency values from
 * lofreq to hifreq, where the frequency interval is (hifreq-lofreq)/(nf-1).
 *
\f{eqnarray*}
H(f) = a \left( cos ( \theta ) + i sin ( \theta ) \right)
\f}
 *
 * If nfap <= 1, an error message is printed, the nf real and imag values are
 * set to 0., and the function returns.
 * 
 * @param[in] lofreq The low frequency cutoff.
 * @param[in] hifreq The high frequency cutoff.
 * @param[in] nf The number of frequency value between flo and fhi to compute.
 * @param[out] real An array to hold the nf real response values.
 * @param[out] imag An array to hold the nf imaginary response values.
 */
void Response::computeFAP(double lofreq, double hifreq, int nf, double *real, double *imag)
{
    computefap(lofreq, hifreq, nf, real, imag);

    if(b58_frequency >= 0.) {
	double re, im, norm;
	computefap(b58_frequency, b58_frequency, 1, &re, &im);
	// normalize response amplitude to be b58_sensitivity at b58_frequency
	norm = sqrt(re*re + im*im);
	if(norm != 0.) {
	    norm = b58_sensitivity/norm;
	    for(int i = 0; i < nf; i++) {
		real[i] *= norm;
		imag[i] *= norm;
	    }
	}
    }
}

void Response::computefap(double lofreq, double hifreq, int nf, double *real,
			double *imag)
{
#ifdef HAVE_GSL
    int i, j, n;
    double freq, df;
    double *f = NULL, *a = NULL, *p = NULL;
    double amp, phase;
    gsl_interp *interp;
    gsl_interp_accel *acc;

    if(nfap <= 1) {
	for(i = 0; i < nf; i++) {
	    real[i] = imag[i] = 0.;
	}
	cerr << "Response.computeFAP error: nfap = " << nfap << endl;
	return;
    }
    f = new double[nfap];
    a = new double[nfap];
    p = new double[nfap];

    if(lofreq == 0.)
    {
	lofreq = 1.e-30;
    }
    if(fap_f[0] == 0.)
    {
	fap_f[0] = 1.e-30;
    }
    for(i = 0; i < nfap; i++)
    {
	f[i] = log10((double)fap_f[i]);
	a[i] = (fap_a[i] != 0.) ? log10((double)fap_a[i]) : log10(1.e-30);
    }
    // unwrap the phase
    for(i = 0; i < nfap; i++) p[i] = (double)fap_p[i];
    unwrap(p, nfap);

    df = (nf > 1) ? (hifreq-lofreq)/(nf-1) : 0.;

    if(nfap >= 4) {
	interp = gsl_interp_alloc(gsl_interp_cspline, 4);
	n = 4;
    }
    else {
	interp = gsl_interp_alloc(gsl_interp_linear, nfap);
	n = nfap;
    }
    acc = gsl_interp_accel_alloc();

    for(i = 0; i < nf; i++)
    {
	freq = log10(lofreq + i*df);

	if(nfap >= 4) {
	    for(j = 0; j < nfap; j++) {
		if(f[j] > freq) break;
	    }
	    j -= 2;
	    if(j < 0) {
		j = 0;
	    }
	    else if(j > nfap - 4) {
		j = nfap - 4;
	    }
	}
	else {
	    j = 0;
	}
	amp = interpResp(interp, acc, f+j, a+j, n, freq);
	phase = interpResp(interp, acc, f+j, p+j, n, freq);

	amp = pow((double)10., amp);
	real[i] = amp*cos(phase);
	imag[i] = amp*sin(phase);
    }
    gsl_interp_free(interp);
    gsl_interp_accel_free(acc);

    delete[] f;
    delete[] a;
    delete[] p;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

/*
 *  Compute a linear filter response (FIR or IIR)
 *
 *		 M
 *		----
 *		\           -2*PI*i*k*(f*dt)
 *		/     c  * e
 *		----   k
 *		k=0
 *  H(f) =   ------------------------------------
 *		 N
 *		----
 *		\           -2*PI*i*j*(f*dt)
 *		/     d  * e
 *		----   j
 *		j=0
 *
 * M = resp->num_n must be > 0.
 * N = resp->num_d can be >= 0.
 * dt = 1./resp->samprate
 * c[k] = resp->fir_n[k]
 * d[j] = resp->fir_d[j]
 */

/**
 * Compute a linear filter (FIR) instrument response curve. The FIR response
 * \f$H(f)\f$ is
\f{eqnarray*}
H(f) = \frac { \sum_{k=0}^{M} {c_{k}   e^{-2\pi i k (f dt)}} }
{ \sum_{j=0}^{N} {d_{j}   e^{-2\pi i j (f dt)}} }
\f}
 * where
 *
 * - \f$f\f$ = frequency, the interval is (hifreq-lofreq)/(nf-1).
 * - \f$M\f$ = num_n, the number of numerator coefficients, must be > 0.
 * - \f$N\f$ = num_d, the number of denominator coefficients, can be >= 0.
 * - \f$dt\f$ = 1./samprate, the sample time interval.
 * - \f$c_{k}\f$ = fir_n[k], the numerator coefficients
 * - \f$d_{j}\f$ = fir_d[j], the denominator coefficients
 * 
 * @param[in] lofreq The low frequency cutoff.
 * @param[in] hifreq The high frequency cutoff.
 * @param[in] nf The number of frequency value between flo and fhi to compute.
 * @param[out] real An array to hold the nf real response values.
 * @param[out] imag An array to hold the nf imaginary response values.
 * @return true for success, false for if num_n <= 0 or lofreq < 0.
 *    or lofreq > hifreq or samprate <= 0. or hifreq > (samprate/2.)*1.0001
 */
bool Response::computeFIR(double lofreq, double hifreq, int nf, double *real, double *imag)
{
    if( !computefir(lofreq, hifreq, nf, real, imag) ) return false;

    if(b58_frequency >= 0.) {
	double re, im, norm;
	if(computefir(b58_frequency, b58_frequency, 1, &re, &im)) {
	    // normalize amplitude to be b58_sensitivity at b58_frequency
	    norm = sqrt(re*re + im*im);
	    if(norm != 0.) {
		norm = b58_sensitivity/norm;
		for(int i = 0; i < nf; i++) {
		    real[i] *= norm;
		    imag[i] *= norm;
		}
	    }
	}
    }
    return true;
}

bool Response::computefir(double lofreq, double hifreq, int nf, double *real,
		double *imag)
{
    int i, j, k;
    double arg, den, freq, df, dt, cs, csa, cs1, cs2, sn, sn1, sn2;
    double numerator_real, numerator_imag;
    double denominator_real, denominator_imag;

    if(nf <= 0) return true;

    if(num_n <= 0 || lofreq < 0. || lofreq > hifreq ||
	input_samprate <= 0. || hifreq > (input_samprate/2.)*1.0001)
    {
	return false;
    }

    df = (nf > 1) ? (hifreq - lofreq)/(double)(nf-1) : 0.;
    dt = 1./input_samprate;

    for(i = 0; i < nf; i++)
    {
	freq = lofreq + i*df;

	arg = 2.*M_PI*freq*dt;

	/* compute the numerator
	 */
	numerator_real = 0.;
	numerator_imag = 0.;
	if(num_n < 5) {
	    for(k = 0; k < num_n; k++) {
		numerator_real += fir_n[k]*cos(k*arg);
		numerator_imag -= fir_n[k]*sin(k*arg);
	    }
	}
	else {
	    cs1 = 1.;
	    sn1 = 0.;
	    numerator_real += fir_n[0];
	    cs2 = cos(arg);
	    sn2 = sin(arg);
	    csa = cs2;
	    numerator_real += fir_n[1]*cs2;
	    numerator_imag -= fir_n[1]*sn2;
	    for(k = 2; k < num_n; k++) {
		cs = 2.*cs2*csa - cs1;
		sn = 2.*sn2*csa - sn1;
		numerator_real += fir_n[k]*cs;
		numerator_imag -= fir_n[k]*sn;
		cs1 = cs2;
		cs2 = cs;
		sn1 = sn2;
		sn2 = sn;
	    }
	}

	if(num_d > 0)
	{
	    /* compute the denomiator
	     */
	    denominator_real = 0.;
	    denominator_imag = 0.;
	    for(j = 0; j < num_d; j++) {
		denominator_real += fir_d[j]*cos(j*arg);
		denominator_imag -= fir_d[j]*sin(j*arg);
	    }

	    /* complex division
	     */
	    den = denominator_real*denominator_real +
		    denominator_imag*denominator_imag;
	    if(den == 0.) den = 1.;
	    real[i] = (numerator_real*denominator_real +
			numerator_imag*denominator_imag)/den;
	    imag[i] = (numerator_imag*denominator_real -
			numerator_real*denominator_imag)/den;
	}
	else
	{
	    real[i] = numerator_real;
	    imag[i] = numerator_imag;
	}
    }

    return true;
}

/**
 * Compute an instrument response curve for PAZ, FAP and FIR formats.
 * If calper > 0., the response is normalized to 1./calib at period calper.
 * @param[in] resp A Vector containing Response objects.
 * @param[in] lofreq The low frequency cutoff.
 * @param[in] hifreq The high frequency cutoff.
 * @param[in] calib The calibration factor. Not used if calper <= 0.
 * @param[in] calper The calibration period or <= 0. if not used.
 * @param[in] nf The number of frequency value between flo and fhi to compute.
 * @param[out] real An array to hold the nf real response values.
 * @param[out] imag An array to hold the nf imaginary response values.
 * @return true for success, false for failure.
 * @throws GERROR_MALLOC_ERROR
 */
bool Response::compute(vector<Response *> *resp, double lofreq, double hifreq,
		double calib, double calper, int nf, double *real, double *imag)
{
    int i, j;
    double calfreq;
    double df, omega, re, im, calre=1., calim=0., *fr=NULL, *fi=NULL;

    if((int)resp->size() <= 0 || nf <= 0) return false;

    fr = (double *)mallocWarn(nf*sizeof(double));
    fi = (double *)mallocWarn(nf*sizeof(double));
    if( !fr || !fi ) {
	Free(fr);
	GError::setMessage("Response.compute: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    for(j = 0; j < (int)resp->size(); j++)
    {
	Response *r = resp->at(j);

	if(!r->type.compare("paz"))
	{
	    df = (nf > 1) ? (hifreq - lofreq)/(nf-1) : 0.;
	    for(i = 0; i < nf; i++)
	    {
		omega = 2.*M_PI*(lofreq + i*df);
		r->computePAZ(omega, r->pole, r->npoles, r->zero, r->nzeros,
				r->a0, fr+i, fi+i);
	    }
	    if(calper > 0.)
	    {
		omega = 2.*M_PI/calper;
		r->computePAZ(omega, r->pole, r->npoles, r->zero, r->nzeros,
			r->a0, &re, &im);
		cmplx_mult(1, &calre, &calim, &re, &im);
	    }
	}
	else if(!r->type.compare("fap"))
	{
	    r->computeFAP(lofreq, hifreq, nf, fr, fi);
	    if(calper > 0.)
	    {
		calfreq = 1./calper;
		r->computeFAP(calfreq, calfreq, 1, &re, &im);
		cmplx_mult(1, &calre, &calim, &re, &im);
	    }
	}
	else if(!r->type.compare("fir"))
	{
	    r->computeFIR(lofreq, hifreq, nf, fr, fi);
	    if(calper > 0.)
	    {
		calfreq = 1./calper;
		r->computeFIR(calfreq, calfreq, 1, &re, &im);
		cmplx_mult(1, &calre, &calim, &re, &im);
	    }
	}
	else
	{
	    cerr << "Response:compute: unknown type: " << r->type << endl;
	    Free(fr);
	    Free(fi);
	    return false;
	}

	if(j == 0) {
	    memcpy(real, fr, nf*sizeof(double));
	    memcpy(imag, fi, nf*sizeof(double));
	}
	else {
	    cmplx_mult(nf, real, imag, fr, fi);
	}
    }
    if(calper > 0.)
    {
	/* Normalize the response to be 1./calib at period calper. Then, to
	 * convert from counts to nanometers, you will multiply the counts by
	 * calib.
	 */
	double norm = sqrt(calre*calre + calim*calim);
	if(norm != 0. && calib != 0.) norm = 1./(calib*norm);
	for(i = 0; i < nf; i++)
	{
	    real[i] *= norm;
	    imag[i] *= norm;
	}
    }
    Free(fr);
    Free(fi);
    return true;
}

/** Multiply to arrays of complex numbers.
 *  @param[in] n the number of complex numbers.
 *  @param[in,out] ar real part of a*b (length n)
 *  @param[in,out] ai imagninary part of a*b (length n)
 *  @param[in] br real part (length n)
 *  @param[in] bi imagninary part (length n)
 */
static void
cmplx_mult(int n, double *ar, double *ai, double *br, double *bi)
{
    int i;
    double real, imag;

    for(i = 0; i < n; i++)
    {
	real = ar[i];
	imag = ai[i];

	ar[i] = real*br[i] - imag*bi[i];
	ai[i] = real*bi[i] + imag*br[i];
    }
}

/**
 * Unwrap a phase curve.
 * @param p The phase values in radians.
 * @param n The number of phase values.
 */
void Response::unwrap(double *p, int n)
{
    int i;
    double this_p, last_p;
    double shift, two_pi;

    if(n <= 0) return;

    two_pi = 2.*M_PI;
    shift = 0.;
    // force phase between -PI and PI
    if(p[0] > M_PI) last_p = p[0] - ((int)((p[0]+M_PI)/two_pi))*two_pi;
    else if(p[0] < -M_PI) last_p = p[0] - ((int)((p[0]-M_PI)/two_pi))*two_pi;
    else last_p = p[0];
    p[0] = last_p;
    for(i = 1; i < n; i++)
    {
	if(p[i] > M_PI) this_p = p[i] - ((int)((p[i]+M_PI)/two_pi))*two_pi;
	else if(p[i] < -M_PI) this_p = p[i] - ((int)((p[i]-M_PI)/two_pi))*two_pi;
	else this_p = p[i];

	if(this_p > last_p)
	{
	    if(this_p - last_p > fabs(this_p-two_pi - last_p))
	    {
		shift -= two_pi;
	    }
	}
	else
	{
	    if(last_p - this_p > fabs(this_p+two_pi - last_p))
	    {
		shift += two_pi;
	    }
	}
	last_p = this_p;
	p[i] = this_p + shift;
    }
}

/** Remove any time shift from the response. Remove the linear trend from
 *  the phase of the complex response.
 *  @param[in] nf the number of complex values
 *  @param[in] re the real values
 *  @param[in] im the imaginary values
 */
void Response::removeTimeShift(int nf, double *re, double *im)
{
    int i;
    double *phase = NULL;
    double amp, a, b, d, sumxy, sumx, sumy, sumxsq;

    if(nf <= 0) return;

    phase = (double *)mallocWarn(nf*sizeof(double));
    for(i = 0; i < nf; i++) {
	phase[i] = atan2(im[i], re[i]);
    }
    unwrap(phase, nf);

    sumxy = 0.;
    sumx = 0.;
    sumy = 0.;
    sumxsq = 0.;
    for(i = 0; i < nf; i++) {
	sumxy += (double)i*phase[i]; 
	sumx += (double)i;
	sumy += phase[i];
	sumxsq += (double)(i*i);
    }

    d = nf*sumxsq - sumx*sumx;
    if(d != 0.) {
	b = (nf*sumxy - sumx*sumy)/d;
	a = sumy/(double)nf - b*sumx/(double)nf;

	for(i = 0; i < nf; i++) {
	    phase[i] -= (a + b*(double)i);
	    amp = sqrt(re[i]*re[i] + im[i]*im[i]);
	    re[i] = amp*cos(phase[i]);
	    im[i] = amp*sin(phase[i]);
	}
    }
    Free(phase);
}

#ifdef HAVE_GSL
static double
interpResp(gsl_interp *interp, gsl_interp_accel *acc, double *f, double *a,
		int n, double freq)
{
    if(freq <= f[0]) {
	return a[0];
    }
    else if(freq >= f[n-1]) {
	return a[n-1];
    }
    gsl_interp_init(interp, f, a, n);
    return gsl_interp_eval(interp, f, a, freq, acc);
}
#endif
