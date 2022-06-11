#include "config.h"
/*
 * NAME
 *      cepstrum.c
 *	Algorithms from libhydro, Frank Graeber, PTS, 2005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_GSL
#include "gsl/gsl_fft_real.h"
#include "gsl/gsl_fft_halfcomplex.h"
#endif

#include "cepstrum.h"
#include "libgmath.h"
#include "libstring.h"
#include "logErrorMsg.h"

#define Log10(a) ((a) ? log10(a) : 1.e-20)
#define Free(a) if(a) {free(a); a=NULL;}

#ifdef HAVE_GSL
static int CepstrumSmooth(CepstrumParam *cp, float *data, int nf, double df);
static int noiseSpectrumEqual(float *data, int nf, int guard, int aveband,
			double tpass, int npass, int noise);
static void demean(double *data, int npts);
static void detrend(float *data, int npts);
static void taperHann(double *x, int n);
#endif

int
Cepstrum(float *signal, int sig_npts, float *noise, int noise_npts,
		double dt, CepstrumParam *cp, CepstrumOut *co)
{
#ifdef HAVE_GSL
	int i, num, npts, np2, nf, n2, imax;
	int if1, if2, i1, i2, guard, aveband;
	float *data=NULL, *noise_amp = NULL, min, max;
	double df, mean, sqerr, im, re, *r=NULL;
	CepstrumOut cepstrum_out_null = CEPSTRUM_OUT_NULL;

	*co = cepstrum_out_null;

	if(!signal || sig_npts <= 0 || dt <= 0) return -1;

	if(noise || noise_npts > 0) {
	    npts = (sig_npts > noise_npts) ? sig_npts : noise_npts;
	}
	else {
	    npts = sig_npts;
	}
	for(np2 = 2; np2 < npts; np2 *= 2);
	n2 = np2/2;
	nf = np2/2 + 1;
	co->nf = nf;
	df = 1./((double)np2*dt);
	co->dt = dt;
	co->df = df;

	if(!(r = (double *)mallocWarn(np2*sizeof(double)))) return -1;
	if(!(data = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	for(i = 0; i < sig_npts; i++) r[i] = signal[i];
	for(i = sig_npts; i < np2; i++) r[i] = 0.;

	demean(r, sig_npts);
	taperHann(r, sig_npts);

        gsl_fft_real_radix2_transform(r, 1, np2);

	for(i = 0; i < nf; i++) {
	    if(i == 0 || i == n2) im = 0.;
	    else im = r[np2-i];
	    re = r[i];
	    data[i] = sqrt(re*re + im*im);
	}
	if(cp->return_data1) {
	    if(!(co->data1 = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	    memcpy(co->data1, data, nf*sizeof(float));
	}

	/* smooth spectrum */
	if(CepstrumSmooth(cp, data, nf, df)) {
	  logErrorMsg(LOG_ERR, "Smoothing of spectrum failed\n");
	  return -1;
	}

	/* normalize */
	max = data[0];
	for(i = 1; i < nf; i++) {
	    if(data[i] > max) max = data[i];
	}
	if(max) {
	    for(i = 0; i < nf; i++) data[i] /= max;
	}
	if(cp->return_data2) {
	    if(!(co->data2 = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	    memcpy(co->data2, data, nf*sizeof(float));
	}

	if(noise) {
	    double *n = (double *)mallocWarn(np2*sizeof(double));
	    if(!n) return -1;
	    if(!(noise_amp = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	    for(i = 0; i < noise_npts; i++) n[i] = noise[i];
	    for(i = noise_npts; i < np2; i++) n[i] = 0.;

	    demean(n, noise_npts);
	    taperHann(n, noise_npts);

	    gsl_fft_real_radix2_transform(n, 1, np2);

	    for(i = 0; i < nf; i++) {
		if(i == 0 || i == n2) im = 0.;
		else im = n[np2-i];
		re = n[i];
		noise_amp[i] = sqrt(re*re + im*im);
	    }
	    if(cp->return_noise1) {
		if(!(co->noise1 = (float *)mallocWarn(nf*sizeof(float)))) {
		    return -1;
		}
		memcpy(co->noise1, noise_amp, nf*sizeof(float));
	    }

	    /* smooth noise */
	    if(CepstrumSmooth(cp, noise_amp, nf, df)) return -1;
	    if(max) {
		for(i = 0; i < nf; i++) noise_amp[i] /= max;
	    }

	    if(cp->return_noise2) {
		if(!(co->noise2 = (float *)mallocWarn(nf*sizeof(float)))) {
		    return -1;
		}
		memcpy(co->noise2, noise_amp, nf*sizeof(float));
	    }

	    for(i = 0; i < nf; i++) {
		data[i] -= noise_amp[i];
		if(data[i] < 0.) data[i] = 0.;
	    }
	    Free(n);
	    Free(noise_amp);

	    if(cp->return_data3) {
		if(!(co->data3 = (float *)mallocWarn(nf*sizeof(float)))) {
		    return -1;
		}
		memcpy(co->data3, data, nf*sizeof(float));
	    }
	}

	for(i = 0; i < nf; i++) {
	    data[i] = Log10(data[i]);
	}

	if1 = (int)(cp->flo/df + .5);
	if(if1 < 0) if1 = 0;
	if(if1 > nf-1) if1 = nf-1;
	if2 = (int)(cp->fhi/df + .5);
	if(if2 < 0) if1 = 0;
	if(if2 > nf-1) if2 = nf-1;

	min = data[if1];
	for(i = if1; i <= if2; i++) {
	    if(min > data[i]) min = data[i];
	}

/*
	if(min < cp->shift) {
	    logErrorMsg(LOG_ERR, "Cepstrum: shift too small.");
	}
	else if(cp->shift) {
	    for(i = if1; i <= if2; i++) {
		data[i] += cp->shift;
	    }
	}
*/

	guard = (int)rint(cp->guard1/df);
	aveband = (int)rint(cp->aveband1/df);

	num = if2 - if1 + 1;
	if(noiseSpectrumEqual(data+if1, num, guard, aveband,
			      cp->tpass, cp->npass, cp->noise_flag)) {
	  logErrorMsg(LOG_ERR, "First noise spectrum equalization failed\n");
	  return -1;
	}

	detrend(data+if1, num);

	if(cp->noise_flag) {
	    Taper_cosine(data+if1, num, 0.1, 0.1);
	}
	else {
	    Taper_cosine(data+if1, num, 0.2, 0.2);
	}

	for(i = 0; i < if1; i++) data[i] = data[if1];
	for(i = if2; i < nf; i++) data[i] = data[if2];

        /* remove mean to eliminate DC component */

	mean = 0.;
	for(i = 0; i < nf; i++) mean += data[i];
	if(nf) {
	    mean /= nf;
	    for(i = 0; i < nf; i++) data[i] -= mean;
	}

	if(cp->return_data4) {
	    if(!(co->data4 = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	    memcpy(co->data4, data, nf*sizeof(float));
	}

        /* inverse fft to cepstral time domain */

	for(i = nf-1; i >= 0; i--) {
	    r[i] = data[i]/dt;
	}
	for(i = nf; i < np2; i++) r[i] = 0.;

	gsl_fft_halfcomplex_radix2_inverse(r, 1, np2);

	for(i = 0; i < nf; i++) data[i] = r[i];

	if(cp->return_data5) {
	    if(!(co->data5 = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	    memcpy(co->data5, data, nf*sizeof(float));
	}

	/* reset DC component and start-up peak to aid noiseSpectrumEqual
	 */
	i1 = (int)rint(cp->pulse_delay_min/dt) + 1;
	if(i1 < 1./(cp->fhi*dt)) i1 = (int)(1./(cp->fhi*dt));

	i2 = (int)rint(cp->pulse_delay_max/dt);
	if(i2 > nf-1) i2 = nf-1;

	min = data[0];
	num = 0;
	for(i = 0; i < i1; i++) {
	    if(data[i] <= min ) {
		num = i;
		min = data[i];
	    }
	}
	for(i = 0; i < num; i++ ) data[i] = min;

	min = data[0];
	for(i = 0; i < nf; i++) {
	    if(min > data[i]) min = data[i];
	}

	for(i = 0; i < nf; i++) {
	    data[i] -= min;
	}

/*
	if(min < cp->shift) {
	    logErrorMsg(LOG_ERR, "Cepstrum: shift too small.");
	}
	else if(cp->shift) {
	    for(i = 0; i < nf; i++) {
		data[i] += cp->shift;
	    }
	}
*/

	guard = (int)rint(cp->guard2/dt);
	aveband = (int)rint(cp->aveband2/dt);

	if(noiseSpectrumEqual(data, nf, guard, aveband, cp->tpass, cp->npass,
			      cp->noise_flag)) {
	  logErrorMsg(LOG_ERR, "Second noise spectrum equalization failed\n");
	  return -1;
	}

	detrend(data, nf);

	for(i = 0; i < nf; i++) {
	    data[i] = fabs(data[i]);
	}

	if(cp->return_data6) {
	    if(!(co->data6 = (float *)mallocWarn(nf*sizeof(float)))) return -1;
	    memcpy(co->data6, data, nf*sizeof(float));
	}

	/* compute variance, delay time, and peak std
	 */
	
	max = data[i1];
	imax = 0;
	mean = 0.;
	for(i = i1; i <= i2; i++) {
	    if(max < data[i]) {
		max = data[i];
		imax = i;
	    }
	    mean += data[i];
	}
	co->delay_time = imax*dt;

	mean /= (double)(i2-i1+1);

	sqerr = 0.;
	for(i = i1; i < i2; i++) {
	    sqerr += pow(mean - data[i], 2);
	}
	co->variance = sqerr/(i2-i1+1);
	co->peak_std = (max - mean) / sqrt(co->variance);

	Free(r);
	Free(data);

	return(0);
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return -1;
#endif
}

#ifdef HAVE_GSL
static void
taperHann(double *x, int n)
{
	int i;
	double dt, taper;

	if(n > 1)
	{
	    dt = 2*M_PI/(double)(n-1);
	    for(i = 0; i < n; i++)
	    {
		taper = .5*(1. - cos(i*dt));
		x[i] *= taper;
	    }
	}
}

static int
CepstrumSmooth(CepstrumParam *cp, float *data, int nf, double df)
{
	double sum;
	int i, j, k, nbox, nbox2, istart, iend;
	float *tmp = NULL;

	nbox = (int)(cp->smoothing_width/(2.*df));
	nbox = 2*nbox + 1;

	if(nbox < 3 || nf < 2*nbox || nbox % 2 == 0) return -1;

	if(!(tmp = (float *)mallocWarn(nf*sizeof(float)))) return -1;

	for(k = 0; k < cp->smoothing_npass; k++)
	{
	    nbox2 = (nbox - 1)/2;

	    for(i = 0; i < nf; i++)
	    {
		sum = 0.0;
		istart = i - nbox;
		if(istart < 0) istart = 0;
		iend =   i + nbox2 + 1;
		if(iend > nf) iend = nf;
		for(j = istart; j < iend; j++) sum += data[j];
		sum /= (float) (iend - istart);
		tmp[i] = sum;
	    }

	    for( i = 0; i < nf; i++) data[i] = tmp[i];
	}
	Free(tmp);
	return 0;
}

/*
    noiseSpectrumEqual performs Noise Spectrum Equalization using a multi-pass
    split symmetric window
*/

static int
noiseSpectrumEqual(float *data, int nf, int guard, int aveband,
			double tpass, int npass, int noise)
{
	int     i, j, k, k1, k2;
	float   tempu=0., templ=0., dnoml=0., dnomu=0., anoise=0.;
	float   *buff = NULL, *snorm = NULL;

	if(!(buff = (float*)mallocWarn(nf*sizeof(float)))) return -1;
	if(!(snorm =(float*)mallocWarn(nf*sizeof(float)))) return -1;

	memset ((void *)snorm, 0, nf*sizeof(float));

	for(i = 0; i < nf; i++) buff[i] = data[i];

	for(i = 0; i < npass; i++)
	{
	    for(j = nf-1; j >= 0; j--)
	    {
		if(j < nf - guard -1)
		{
		    tempu = 0.0;
		    k2 = j + aveband + guard + 1;
		    if(k2 > nf) k2 = nf;
		    for(k = j + guard + 1; k < k2; k++) {
			tempu += buff[k];
		    }
		    dnomu = k2 - (j+guard+1);
		}
		else
		{
		    dnomu = 0.0;
		    tempu = 0.0;
		}

		if(j > guard)
		{
		    templ = 0.0;
		    k1 = j - (aveband + guard);
		    if(k1 < 0) k1 = 0;
		    for(k = k1; k < j - guard; k++) {
			templ += buff[k];
		    }
		    dnoml = j - guard - k1;
		}
		else
		{
		    dnoml = 0.0;
		    templ = 0.0;
		}
		anoise = (templ + tempu) / (dnoml + dnomu);
		snorm[j] = anoise;

		if(buff[j] > tpass*anoise)
		{
		    buff[j] = snorm[j];
		}
	    }
	    tpass /= 2.;
	}

	if(noise) {
	    for(i = 0; i < nf; i++)
	    {
		data[i] = (fabs(snorm[i]) > 1.e-13) ? snorm[i] : 1.e-13;
	    }
	}
	else {
	    for(i = 0; i < nf; i++)
	    {
		double n = (fabs(data[i]) > 1.0e-11) ? data[i] : 1.0e-11;
		double d = (fabs(snorm[i]) > 1.0e-11) ? snorm[i] : 1.0e-11;
		data[i] = n/d;
	    }
        }
	Free(buff);
	Free(snorm);
	return 0;
}

static void
detrend(float *data, int npts)
{
	int i;
	double sum, sumi, slope, intercept;

	sum = 0.;
	sumi = 0.;
	for(i = 0; i < npts; i++) {
	    sum += data[i];
	    sumi += (i + 1)*data[i];
	}

	/* Compute slope and intercept of linear least square fit
	 */
	slope = (12.*sumi - 6.*(npts + 1.)*sum)/(npts*(npts + 1.)*(npts - 1.));
	intercept = ((4.*npts + 2.)*sum - 6.*sumi)/(npts*(npts - 1.));

	/* Remove least square fit
	*/
	for(i = 0; i < npts; i++) {
	    data[i] -= (intercept + (i+1)*slope);
	}
}

static void
demean(double *data, int npts)
{
	int i;
	double mean;

	mean = 0.;
	for(i = 0; i < npts; i++) mean += data[i];
	if(npts) mean /= (double)npts;

	for(i = 0; i < npts; i++) data[i] -= mean;
}
#endif
