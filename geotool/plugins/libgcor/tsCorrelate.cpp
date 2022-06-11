/** \file tsCorrelate.cpp
 *  \brief Defines correlation routines.
 *  \author Ivan Henson
 */
#include "config.h"
#include <math.h>
#include <string.h>

#include "gsl/gsl_fft_real.h"
#include "gsl/gsl_fft_halfcomplex.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Correlation.h"
#include "gobject++/GTimeSeries.h"
extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "tapers.h"
}

using namespace libgcor;

static float *getData(GTimeSeries *ts, int *npts);
static void rankArray(int n, float *f);
static void sortupf(float *values, int n, int *sort_order);

/* Compute the linear correlation waveform:
 *
 *	                  sum [ (x[i]-xm) * (y[i]-ym) ]
 *  r[j] =  ---------------------------------------------------------------
 *          sqrt{sum[(x[i]-xm)*(x[i]-xm)]} * sqrt{sum[(y[i]-ym)*(y[i]-ym)]}
 *
 *  xm = mean of x[i]
 *  ym = mean of y[i]
 */

/* Compute a correlation waveform.
 * @param[in] ref The reference waveform.
 * @param[in] target The target waveform.
 * @param[in] ends Set to 0 or 1 to ignore or include the correlation points
 *	at the beginning and ending that are computed using only part of the
 *	ref waveform.
 * @param[in] norm_type LOCAL_MEAN: the mean is recomputed for the data points
 *	that contribute to each correlation. GLOBAL_MEAN: global means are
 *	used.  TOTAL_AMP: the correlation values are normalized by the total
 *	amplitudes of the waveforms.
 * @return Returns the correlation waveform. Returns NULL if the length of the
 *	ref is 0 or the length of the target waveform is 0.
 */
GTimeSeries * Correlation::timeCorrelate(GTimeSeries *ref, GTimeSeries *target,
			int ends, NormType norm_type)
{
    int i, j, j1, n, nr, nt, nc;
    float *r, *t, *c = NULL, *f;
    double tmean, rmean, tbeg, tdel, calib, calper, sum_rr, sum_tt, sum_rt, d;
    GTimeSeries *ts = NULL;
    bool reverse = false;

    if(ref->size() > 1) {
	r = getData(ref, &nr);
    }
    else {
	nr = ref->length();
	r = ref->segment(0)->data;
    }
    if(target->size() > 1) {
	t = getData(target, &nt);
    }
    else {
	nt = target->length();
	t = target->segment(0)->data;
    }
    if(nr <= 0 || nt <= 0) return NULL;

    if(nr > nt) { // reverse r and t if nr < nt
	n = nr;
	nr = nt;
	nt = n;
	f = r;
	r = t;
	t = f;
	reverse = true;
    }

    rmean = 0.;
    for(i = 0; i < nr; i++) rmean += r[i];
    if(nr) rmean /= nr;

    tmean = 0.;
    for(i = 0; i < nt; i++) tmean += t[i];
    if(nt) tmean /= nt;

    if(ends) {
	nc = nr + nt - 1;
	c = (float *)mallocWarn(nc*sizeof(float));
	j1 = 0;
    }
    else {
	nc = nt - nr + 1;
	c = (float *)mallocWarn(nc*sizeof(float));
	j1 = nr-1;
    }

    nc = 0;
    for(j = j1; j < nt; j++)
    {
	n = (j < nr) ? j+1 : nr;
	sum_rt = sum_tt = sum_rr = 0.;

	if( norm_type == LOCAL_MEAN ) {
	    rmean = 0.;
	    for(i = 0; i < n; i++) rmean += r[nr-1-i];
	    if(n) rmean /= n;
	    tmean = 0.;
	    for(i = 0; i < n; i++) tmean += t[j-i];
	    if(n) tmean /= n;
	}

	for(i = 0; i < n; i++) {
	    sum_rt += (t[j-i]-tmean)*(r[nr-1-i]-rmean);
	    sum_rr += (r[nr-1-i]-rmean)*(r[nr-1-i]-rmean);
	    sum_tt += (t[j-i]-tmean)*(t[j-i]-tmean);
	}
	d = (norm_type != TOTAL_AMP) ? sqrt(sum_rr*sum_tt) : 1.;
	c[nc++] = (d != 0.) ? sum_rt/d : 0.;
    }
    if(ends)
    {
	for(j = nr-2; j >= 0; j--) {
	    n = (j < nt) ? j+1 : nt;
	    sum_rt = sum_tt = sum_rr = 0.;

	    if( norm_type == LOCAL_MEAN )
	    {
		rmean = 0.;
		tmean = 0.;
		for(i = 0; i < n; i++) {
		    rmean += r[j-i];
		    tmean += t[nt-1-i];
		}
		if(n) {
		    rmean /= n;
		    tmean /= n;
		}
	    }
	    for(i = 0; i < n; i++) {
		sum_rt += (t[nt-1-i]-tmean)*(r[j-i]-rmean);
		sum_rr += (r[j-i]-rmean)*(r[j-i]-rmean);
		sum_tt += (t[nt-1-i]-tmean)*(t[nt-1-i]-tmean);
	    }
	    d = (norm_type != TOTAL_AMP) ? sqrt(sum_rr*sum_tt) : 1.;
	    c[nc++] = (d != 0.) ? sum_rt/d : 0.;
	}
    }
    if(norm_type == TOTAL_AMP)
    {
	sum_rr = 0.;
	for(i = 0; i < nr; i++) sum_rr += (r[i]-rmean)*(r[i]-rmean);
	sum_tt = 0.;
	for(i = 0; i < nt; i++) sum_tt += (t[i]-tmean)*(t[i]-tmean);

	d = sum_rr * sum_tt;
	if(d > 0.) {
	    nc = (ends) ? nr + nt - 1 : nt - nr + 1;
	    d = 1./sqrt(d);
	    for(i = 0; i < nc; i++) c[i] *= d;
	}
    }
    if( reverse ) {
	float fc;
	// reverse c[]
	n = nc/2;
	for(i = 0; i < n; i++) {
	    fc = c[i];
	    c[i] = c[nc-1-i];
	    c[nc-1-i] = fc;
	}
	f = r;
	r = t;
	t = f;
    }

    if(ref->size() > 1) free(r);
    if(target->size() > 1) free(t);

    tdel = target->segment(0)->tdel();
    if(ends) {
	tbeg = target->tbeg() - (nr-1)*tdel/2.;
    }
    else {
	tbeg = target->tbeg() + (nr-1)*tdel/2.;
    }
    calib = 1.;
    calper = target->segment(0)->calper();
    ts = new GTimeSeries(new GSegment(c, nc, tbeg, tdel, calib, calper));
    if(c) free(c);
    return ts;
}

/* Compute a correlation waveform using the FFT. This is much faster than
 * the routine tsCorrelate.
 * @param[in] ref The reference waveform. Must be shorter than the target.
 * @param[in] target The target waveform.
 * @param[in] rank_order If 1, compute the nonparametric or rank correlation.
 * @param[in] norm_type LOCAL_MEAN: the mean is recomputed for the data points
 *	that contribute to each correlation. GLOBAL_MEAN: global means are
 *	used.  TOTAL_AMP: the correlation values are normalized by the
 *	total amplitudes of the waveforms.
 * @return Returns the correlation waveform. Returns NULL if the length of the
 *	ref waveform or the length of the target wveform is less than or equal
 *	to 0.
 */
GTimeSeries * Correlation::fftCorrelate(GTimeSeries *ref, GTimeSeries *target,
			int rank_order, NormType norm_type)
{
    int nr, nt, nc;
    float *r=NULL, *t=NULL, *c=NULL;
    double tbeg, tdel, calib, calper;
    GTimeSeries *ts = NULL;

    if(ref->size() > 1) {
	r = getData(ref, &nr);
    }
    else {
	nr = ref->length();
	r = (float *)mallocWarn(nr*sizeof(float));
	memcpy(r, ref->segment(0)->data, nr*sizeof(float));
    }
    if(target->size() > 1) {
	t = getData(target, &nt);
    }
    else {
	nt = target->length();
	t = (float *)mallocWarn(nt*sizeof(float));
	memcpy(t, target->segment(0)->data, nt*sizeof(float));
    }
    if(nt <= 0 || nr <= 0) {
	Free(r);
	Free(t);
	return NULL;
    }

    if(rank_order) {
	rankArray(nr, r);
	rankArray(nt, t);
    }

    tdel = target->segment(0)->tdel();
    nc = nt + nr - 1;
    c = (float *)mallocWarn(nc*sizeof(float));

    fftCorrelate(r, nr, t, nt, c, norm_type);

    if(r) free(r);
    if(t) free(t);

    calib = 1.;
    calper = target->segment(0)->calper();
    tbeg = target->tbeg() - (nr-1)*tdel/2.;
    // c[nr-1] = lag of zero. Assign time tbeg to lag zero.
//    tbeg = target->tbeg() - (nr-1)*tdel;
    tdel = target->segment(0)->tdel();
    ts = new GTimeSeries(new GSegment(c, nc, tbeg, tdel, calib, calper));
    if(c) free(c);
    return ts;
}

/*
 * @param[in] r The reference waveform. Must be shorter than the target.
 * @param[in] nr The length of r[].
 * @param[in] t The target waveform.
 * @param[in] nt The length of t[].
 * @param[in] rank_order If 1, compute the nonparametric or rank correlation.
 * @param[out] c The correlation waveform. c must be length nc = nt + nr - 1 to
 *	receive the output correlation trace.
 * @param[in] norm_type LOCAL_MEAN: the mean is recomputed for the data points
 *	that contribute to each correlation. GLOBAL_MEAN: global means are
 *	used.  TOTAL_AMP: the correlation values are normalized by the total
 *	amplitudes of the data arrays.
 */
void Correlation::fftCorrelate(float *r, int nr, float *t, int nt, float *c,
			NormType norm_type)
{
    int i, n, nc, np2;
    double tmean, rmean;
    float *data1 = NULL;
    float *data2 = NULL;
    float *f = NULL;
    bool reverse = false;

    if(nt <= 0 || nr <= 0) return;
    if(nr > nt) { // reverse r and t if nr < nt
	n = nr;
	nr = nt;
	nt = n;
	f = r;
	r = t;
	t = f;
	reverse = true;
    }
    nc = nr + nt - 1;
    for(np2 = 2; np2 < nc; np2 *= 2);

    data1 = (float *)mallocWarn(np2*sizeof(float));
    data2 = (float *)mallocWarn(np2*sizeof(float));
    f = (float *)mallocWarn(2*np2*sizeof(float));

    for(i = 0; i < np2; i++) {
	data1[i] = 0.;
	data2[i] = 0.;
    }

    // demean r
    rmean = 0.;
    for(i = 0; i < nr; i++) rmean += r[i];
    if(nr) rmean /= nr;
    for(i = 0; i < nr; i++) data1[i] = r[i] - rmean;

    // demean t
    tmean = 0.;
    for(i = 0; i < nt; i++) tmean += t[i];
    if(nt) tmean /= nt;
    for(i = 0; i < nt; i++) data2[i] = t[i] - tmean;

    // compute unnormalized cross-correlation
    correl(data2, data1, np2, f);

    // Only return lags -(nr-1) to nt.

    // The negative lags are at the end of f[].
    int i1 = np2 - (nr - 1);
    for(i = 0; i < nr-1; i++) {
	c[i] = f[i1+i];
    }
    // The positive lags.
    for(i = 0; i < nt; i++) {
	c[nr-1+i] = f[i];
    }
    Free(f);

    if( norm_type == GLOBAL_MEAN ) {
	norm1(data1, nr, data2, nt, c);
    }
    else if( norm_type == LOCAL_MEAN ) {
	norm2(data1, nr, data2, nt, c);
    }
    else {
	norm3(data1, nr, data2, nt, c);
    }
    Free(data1);
    Free(data2);

    if( reverse ) {
	float fc;
	// reverse c[]
	n = nc/2;
	for(i = 0; i < n; i++) {
	    fc = c[i];
	    c[i] = c[nc-1-i];
	    c[nc-1-i] = fc;
	}
    }
}

/* Normalize by sqrt( sum((t-tm)*(t-tm)) * sum((r-rm)*(r-rm)) ),
 * where tm and rm are global means and the sum is over the overlapping
 * data values.
 */
void Correlation::norm1(float *data1, int nr, float *data2, int nt, float *c)
{
    int i, k;
    double sum_rr, sum_tt, d;

    if(nr <= 0 || nt <= 0) return;

    sum_tt = 0.;
    sum_rr = 0.;
    k = 0;

    // normalize for lags -(nr-2) to 0 
    for(i = 0; i < nr; i++) {
	sum_tt += data2[i]*data2[i];
	sum_rr += data1[nr-1-i]*data1[nr-1-i];
	d = sum_tt*sum_rr;
	if(d > 0.) c[k] /= sqrt(d);
	else c[k] = 0.;
	k++;
    }

    // normalize for lags from 1 to nt-nr
    for(i = 1; i <= nt-nr; i++) {
	sum_tt -= data2[i-1]*data2[i-1];
	sum_tt += data2[nr+i-1]*data2[nr+i-1];
	d = sum_tt*sum_rr;
	if(d > 0.) c[k] /= sqrt(d);
	else c[k] = 0.;
	k++;
    }

    // normalize for lags nt-nr+1 to nt-2
    for(i = 0; i < nr-1; i++) {
	sum_tt -= data2[nt-nr+i]*data2[nt-nr+i];
	sum_rr -= data1[nr-1-i]*data1[nr-1-i];
	d = sum_tt*sum_rr;
	if(d > 0.) c[k] /= sqrt(d);
	else c[k] = 0.;
	k++;
    }
}

/* Normalize by sqrt( sum((t-tm)*(t-tm)) * sum((r-rm)*(r-rm)) ),
 * where tm and rm are local means and the sum is over the overlapping data
 * values.
 */
void Correlation::norm2(float *data1, int nr, float *data2, int nt, float *c)
{
    int i, k, n=0, nc;
    double tmean, rmean=0., rsum, tsum, sum_rr, sum_tt, d;

    if(nr <= 0 || nt <= 0) return;

    nc = nr + nt - 1;

    c[0] = 0.;
    tsum = data2[0];
    rsum = data1[nr-1];
    sum_tt = data2[0]*data2[0];
    sum_rr = data1[nr-1]*data1[nr-1];
    k = 1;

    // normalize for lags -(nr-2) to 0 
    for(i = 1; i < nr; i++) {
	n = i+1;
	tsum += data2[i];
	tmean = tsum/n;
	rsum += data1[nr-1-i];
	rmean = rsum/n;
	c[k] -= n*tmean*rmean;

	sum_tt += data2[i]*data2[i];
	sum_rr += data1[nr-1-i]*data1[nr-1-i];
	d = (sum_tt - n*tmean*tmean)*(sum_rr - n*rmean*rmean);
	if(d > 0.) c[k] /= sqrt(d);
	else c[k] = 0.;
	k++;
    }

    // normalize for lags from 1 to nt-nr
    for(i = 1; i <= nt-nr; i++) {
	tsum -= data2[i-1];
	tsum += data2[nr+i-1];
	tmean = tsum/nr;
	c[k] -= n*tmean*rmean;
	sum_tt -= data2[i-1]*data2[i-1];
	sum_tt += data2[nr+i-1]*data2[nr+i-1];
	d = (sum_tt - n*tmean*tmean)*(sum_rr - n*rmean*rmean);
	if(d > 0.) c[k] /= sqrt(d);
	else c[k] = 0.;
	k++;
    }

    // normalize for lags nt-nr+1 to nt-2
    for(i = 0; i < nr-2; i++) {
	tsum -= data2[nt-nr+i];
	n = nr-1-i;
	tmean = tsum/n;
	rsum -= data1[nr-1-i];
	rmean = rsum/n;
	c[k] -= n*tmean*rmean;
	sum_tt -= data2[nt-nr+i]*data2[nt-nr+i];
	sum_rr -= data1[nr-1-i]*data1[nr-1-i];
	d = (sum_tt - n*tmean*tmean)*(sum_rr - n*rmean*rmean);
	if(d > 0.) c[k] /= sqrt(d);
	else c[k] = 0.;
	k++;
    }
    if(nc > 0) {
	c[nc-1] = 0.;
    }
}

/* Normalize by sqrt( sum((t-tm)*(t-tm)) * sum((r-rm)*(r-rm)) ),
 * where tm and rm are global means and the sum is over all data values.
 */
void Correlation::norm3(float *data1, int nr, float *data2, int nt, float *c)
{
    double sum_rr, sum_tt, d;

    if(nr <= 0 || nt <= 0) return;

    sum_rr = 0.;
    for(int i = 0; i < nr; i++) {
	sum_rr += data1[i]*data1[i];
    }
    sum_tt = 0.;
    for(int i = 0; i < nt; i++) {
	sum_tt += data2[i]*data2[i];
    }

    d = sum_rr * sum_tt;
    if(d > 0.) {
	int nc = nr + nt - 1;
	d = 1./sqrt(d);
	for(int i = 0; i < nc; i++) {
	    c[i] *= d;
	}
    }
}

static void
rankArray(int n, float *f)
{
	int i, *sort_order;

	if(!(sort_order = (int *)mallocWarn(n*sizeof(int)))) return;

	sortupf(f, n, sort_order);

	for(i = 0; i < n; i++) {
	    f[sort_order[i]] = (float)i;
	}
	free(sort_order);
}

static float *
getData(GTimeSeries *ts, int *npts)
{
	int i, j, n, np;
	double tdel = ts->segment(0)->tdel();
	float *r, *data;

	n = 0;
	for(j = 0; j < ts->size(); j++) {
	    if(j  > 0) {
		double gap = ts->segment(j)->tbeg() - ts->segment(j-1)->tend();
		int ngap = (int)(gap/tdel + .5) - 1;
		if(ngap > 0) {
		    n += ngap;
		}
	    }
	    n += ts->segment(j)->length();
	}
	r = (float *)mallocWarn(n*sizeof(float));
	n = 0;
	for(j = 0; j < ts->size(); j++) {
	    if(j  > 0) {
		double gap = ts->segment(j)->tbeg() - ts->segment(j-1)->tend();
		int ngap = (int)(gap/tdel + .5) - 1;
		for(i = 0; i < ngap; i++) {
		    r[n++] = 0.;
		}
	    }
	    np = ts->segment(j)->length();
	    data = ts->segment(j)->data;
	    for(i = 0; i < np; i++, data++) {
		r[n++] = *data;
	    }
	}
	*npts = n;
	return r;
}

static void
sortupf(float *values, int n, int *sort_order)
{
	int gap, i, j, itemp;
	float temp;

	for(i = 0; i < n; i++) sort_order[i] = i;

	for(gap = n/2; gap > 0; gap /= 2) {
	    for(i = gap; i < n; i++) {
		for(j = i-gap; j >= 0 && values[j] > values[j+gap]; j -= gap) {
		    temp = values[j];
		    values[j] = values[j+gap];
		    values[j+gap] = temp;
		    itemp = sort_order[j];
		    sort_order[j] = sort_order[j+gap];
		    sort_order[j+gap] = itemp;
		}
	    }
	}
}

/** Calculates the correlation of two data arrays. Computes the correlation
 *  of two real data sets data1[] and data2[], each of length n (including
 *  any user-supplied zero padding). n must be an integer power of 2. The
 *  correlation values are not normalized. The correlation values are returned
 *  as the n points in c[] stored in wraparound order, i.e. correlations at
 *  increasing negative lags are in c[n-1] down to c[n/2], while correlations
 *  at increasing positive lags are in c[0] (zero lag) on up to c[n/2-1]. Sign
 *  convention of this routine: if data1[] lags data2[], i.e. is shifted to the
 *  right of it, then c[] will show a peak at position lags.
 *  @param[in] data1 first data array of length n.
 *  @param[in] data2 second data array of length n.
 *  @param[in] n the length of the data arrays.
 *  @param[out] c an array of length n with correlations.
 */
void Correlation::correl(float *data1, float *data2, int n, float *c)
{
    int n2 = n/2;
    double *d1, *d2, re, im;

    d1 = (double *)mallocWarn(n*sizeof(double));
    d2 = (double *)mallocWarn(n*sizeof(double));

    for(int i = 0; i < n; i++) {
	d1[i] = (double)data1[i];
	d2[i] = (double)data2[i];
    }

    gsl_fft_real_radix2_transform(d1, 1, n);
    gsl_fft_real_radix2_transform(d2, 1, n);

    d2[0] = d2[0]*d1[0];
    d2[n2] = d2[n2]*d1[n2];
    for(int i = 1; i < n2; i++) {
	re = d2[i];
	im = d2[n-i];
	d2[i] = (d1[i]*re + d1[n-i]*im);
	d2[n-i] = (d1[n-i]*re - d1[i]*im);
    }

    gsl_fft_halfcomplex_radix2_inverse(d2, 1, n);
    for(int i = 0; i < n; i++) c[i] = d2[i];

    Free(d1);
    Free(d2);
}
