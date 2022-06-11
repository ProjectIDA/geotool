/** \file FKData.cpp
 *  \brief Defines class FKData.
 *  \author Ivan Henson
 */
#include <config.h>
#include <math.h>
#include <stdio.h>
#ifdef HAVE_GSL
#include "gsl/gsl_fft_real.h"
#endif

#include "FKData.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif
#ifndef HALF_PI
#define HALF_PI		1.57079632679489661923
#endif
#ifndef DEG_TO_KM
#define DEG_TO_KM	111.19492664	/* for R = 6371 */
#endif


#define CLEAN_UP \
	Free(fsum); \
	Free(f); \
	Free(t); \
	Free(csy); \
	Free(sny); \
	if(windows == 1) { \
	    for(int q = 1; q < wvec.size(); q++) { \
	 	d1[q]->deleteObject(); d2[q]->deleteObject(); \
	    } \
	} \
	delete [] d1; delete [] d2;

#define CLEAN_UP_FULL \
	Free(fsum); \
	Free(f); \
	Free(t); \
	Free(csy); \
	Free(sny); \
	for(int q = 0; q < wvec.size(); q++) { \
	    d1[q]->deleteObject(); d2[q]->deleteObject(); \
	} \
	delete [] d1; delete [] d2; \
	Free(lat); Free(lon);

#ifdef HAVE_GSL
static void demean(double *t, int npts);
#endif

/** Constructor. 
 *  Compute a single FK for one or more frequency bands.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 * 	of the Waveform objects specify a data window to be used. 
 *  @param[in] fk_args FK argument class with members:
 *  	- output_power: if true, output power db instead of amplitude dB.
 *  	- full_compute: if true, compute a single FK and save all frequencies.
 *  	- three_component: if true, compute a three-component FK.
 *  	- fine_grid: if true, compute a fine-grid.
 *  	- slowness_max: (sec/km) the maximum slowness value of the FK grid.
 *  	- num_slowness: the number of slowness values. (must be odd.)
 *  	- num_bands: the number of frequency bands.
 *  	- fmin: an array of minimum frequencies for the bands.
 *  	- fmax: an array of maximum frequencies for the bands.
 *  	- taper_type: the taper type
 *  	- taper_beg: for the cosine taper, the percentage of the waveform to
 *		taper at the beginning.
 *  	- taper_end: for the cosine taper, the percentage of the waveform to
 *		taper at the end.
 *  	- signal_slow_min: (sec/km) the minimum slowness value for the signal
 *		max. Ignored if < 0.
 *  	- signal_slow_max: (sec/km) the maximum slowness value for the signal
 *		max. Ignored if < 0.
 *  	- signal_az_min: (degrees) the minimum azimuth value for the signal max.
 *		Ignored if < 0.
 *  	- signal_az_max: (degrees) the maximum azimuth value for the signal max.
 *		Ignored if < 0.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *	- num_waveforms <= 0
 *	- num_bands <= 0 or num_bands > MAX_NBANDS
 *	- num_slowness <= 1
 *	- fmin[i] >= fmax[i]
 *	- windows == 0 and a waveform has more that one segment (wvec[i]->size() > 1)
 *	- windows > 0 and the window starts and ends in different segments
 *	- there is no station location for a waveform (wvec[i]->lat < -900. || wvec[i]->lon < -900.)
 *	- all the station locations are the same.
 *	- the sample time intervals differ by more than GTimeSeries::tdelTolerance()
 *	- dnorth/deast values are not available and station locations are not
 *              available.
 *  @throws GERROR_MALLOC_ERROR
 */
FKData::FKData(gvector<Waveform *> &wvec, int windows, FKArgs fk_args)
{
    init(wvec, fk_args);

    if( !fk_args.three_component ) {
	compute(wvec, windows, fk_args);
    }
    else {
	compute3C(wvec, windows, fk_args);
    }
}

/** Constructor with time limits.
 *  Compute a single FK for one or more frequency bands.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] tmin the start of the time window.
 *  @param[in] tmax the end of the time window.
 *  @param[in] fk_args FK argument class with members:
 *  	- output_power: if true, output power db instead of amplitude dB.
 *  	- full_compute: if true, compute a single FK and save all frequencies.
 *  	- three_component: if true, compute a three-component FK.
 *  	- fine_grid: if true, compute a fine-grid.
 *  	- slowness_max: (sec/km) the maximum slowness value of the FK grid.
 *  	- num_slowness: the number of slowness values. (must be odd.)
 *  	- num_bands: the number of frequency bands.
 *  	- fmin: an array of minimum frequencies for the bands.
 *  	- fmax: an array of maximum frequencies for the bands.
 *  	- taper_type: the taper type
 *  	- taper_beg: for the cosine taper, the percentage of the waveform to
 *		taper at the beginning.
 *  	- taper_end: for the cosine taper, the percentage of the waveform to
 *		taper at the end.
 *  	- signal_slow_min: (sec/km) the minimum slowness value for the signal
 *		max. Ignored if < 0.
 *  	- signal_slow_max: (sec/km) the maximum slowness value for the signal
 *		max. Ignored if < 0.
 *  	- signal_az_min: (degrees) the minimum azimuth value for the signal max.
 *		Ignored if < 0.
 *  	- signal_az_max: (degrees) the maximum azimuth value for the signal max.
 *		Ignored if < 0.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *	- num_waveforms <= 0
 *	- num_bands <= 0 or num_bands > MAX_NBANDS
 *	- num_slowness <= 1
 *	- fmin[i] >= fmax[i]
 *	- windows == 0 and a waveform has more that one segment (wvec[i]->size() > 1)
 *	- windows > 0 and the window starts and ends in different segments
 *	- there is no station location for a waveform (wvec[i]->lat < -900. || wvec[i]->lon < -900.)
 *	- all the station locations are the same.
 *	- the sample time intervals differ by more than GTimeSeries::tdelTolerance()
 *	- dnorth/deast values are not available and station locations are not
 *              available.
 *  @throws GERROR_MALLOC_ERROR
 */
FKData::FKData(gvector<Waveform *> &wvec, double tmin, double tmax,
		FKArgs fk_args)
{
    if( fk_args.three_component ) {
	init(wvec, fk_args);
	compute3C(wvec, tmin, tmax, fk_args);
    }
    else if( !fk_args.full_compute) {
	init(wvec, fk_args);
	compute(wvec, tmin, tmax, fk_args);
    }
    else {
	fullCompute(wvec, tmin, tmax, fk_args);
    }
}

/** A constructor that does not compute the FK. This constructor is used
 *  by the FKGram and FKGram3C classes.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 */
FKData::FKData(gvector<Waveform *> &wvec)
{
    FKArgs fk_args;
    init(wvec, fk_args);
}

/** Initialize the members.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] fk_args FK argument class.
 */
void FKData::init(gvector<Waveform *> &wvec, FKArgs fk_args)
{
    if(wvec.size() <= 0) {
	GError::setMessage("FKData: num_waveforms = %d", wvec.size());
	throw (GERROR_INVALID_ARGS);
    }
    args = fk_args;
    nwaveforms = wvec.size();
    shift = 0.;
    memset(net, 0, sizeof(net));
    windowed = 0;
    nt = 0;
    nf = 0;
    if1 = 0;
    if2 = 0;
    dt = 0.;
    df = 0.;
    tbeg = 0.;
    tend = 0.;
    d_slowness = 0.;
    for(int i = 0; i < MAX_NBANDS; i++) {
	scaling[i] = 1.;
	local_average[i] = 0.;
	jmax[i] = 0;
	kmax[i] = 0;
	xmax[i] = 0.;
        ymax[i] = 0.;
        fk_max[i] = 0.;
        restricted_fkmax[i] = 0.;
	total_power[i] = 0.;
	fstat[i] = -1.;
	fk[i] = NULL;
    }
    lon = NULL;
    lat = NULL;
    scan_spectrum = false;
    scan_bandw = 0.;
    memset(center_sta, 0, sizeof(center_sta));
    center_lat = 0.;
    center_lon = 0.;
    fcomplete = NULL;
    complete = NULL;
    waveform_id = NULL;

    if(wvec.size() > 0) {
	shift = wvec[0]->tbeg() - wvec[0]->scaled_x0;
	stringcpy(net, wvec[0]->net(), sizeof(net));
	waveform_id = new int[wvec.size()];
	for(int i = 0; i < wvec.size(); i++) {
	    waveform_id[i] = wvec[i]->getId();
	}
    }
}

/** Destructor. */
FKData::~FKData(void)
{
    for(int i = 0; i < args.num_bands; i++) Free(fk[i]);
    delete [] waveform_id;
    Free(lat);
    Free(lon);
    Free(fcomplete);
    Free(complete);
}

Gobject * FKData::clone(void)
{
    FKData *f = new FKData();

    f->args = args;
    f->nwaveforms = nwaveforms;
    f->waveform_id = NULL;
    if(nwaveforms > 0) {
	f->waveform_id = new int[nwaveforms];
    }

    for(int i = 0; i < nwaveforms; i++) {
	f->waveform_id[i] = waveform_id[i];
    }
    f->shift = shift;
    stringcpy(f->net, net, sizeof(f->net));
    f->shift = shift;
    f->windowed = windowed;
    f->d_slowness = d_slowness;
    stringcpy(f->center_sta, center_sta, sizeof(f->center_sta));
    f->center_lat = center_lat;
    f->center_lon = center_lon;

    int n = args.num_slowness * args.num_slowness;

    for(int i = 0; i < MAX_NBANDS; i++) {
        f->fk_max[i] = fk_max[i];
        f->restricted_fkmax[i] = restricted_fkmax[i];
        f->xmax[i] = xmax[i];
        f->ymax[i] = ymax[i];
	f->fk[i] = NULL;
	if(fk[i] != NULL) {
	    f->fk[i] = (float *)malloc(n*sizeof(float));
	    memcpy(f->fk[i], fk[i], n*sizeof(float));
	}
    }
    f->fine = fine;
    for(int i = 0; i < fine.nbands; i++) {
	f->fine.fk_fine[i] = (float *)malloc(fine.n_fine[i]*sizeof(float));
	memcpy(f->fine.fk_fine[i],fine.fk_fine[i],fine.n_fine[i]*sizeof(float));
    }
    return (Gobject *)f;
}

/** Compute a single FK for one or more frequency bands.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] tmin the start of the time window.
 *  @param[in] tmax the end of the time window.
 *  @param[in] fk_args FK argument class.
 */
void FKData::compute(gvector<Waveform *> &wvec, double tmin, double tmax,
			FKArgs fk_args)
{
    GDataWindow *dw, data_window;
    GDataPoint *dp1, *dp2;

    if(wvec.size() > 0) {
	gvector<Waveform *> ws;
	int num = 0;
	for(int i = 0; i < wvec.size(); i++) {
	    GTimeSeries *ts = wvec[i]->ts;
	    dp1 = ts->upperBound(tmin);
	    dp2 = ts->lowerBound(tmax);
	    if( ts->tbeg() <= tmin && ts->tend() >= tmax &&
		dp1->segmentIndex() == dp2->segmentIndex())
	    {
		ws.push_back(wvec[i]);
		waveform_id[num] = waveform_id[i];
		num++;
	    }
	    delete dp1;
	    delete dp2;
	}
	wvec.clear();
	wvec.load(ws);
	if(num < 4) return;

	dw = ws[0]->dw;
	ws[0]->dw = &data_window;
	ws[0]->dw[0].d1 = ws[0]->upperBound(tmin);
	ws[0]->dw[0].d2 = ws[0]->lowerBound(tmax);

	nwaveforms = num;
	compute(ws, 1, fk_args);

	delete ws[0]->dw[0].d1;
	delete ws[0]->dw[0].d2;
	ws[0]->dw = dw;
    }
}

/** Compute a single FK for one or more frequency bands.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 * 	of the Waveform objects specify a data window to be used. 
 *  @param[in] fk_args FK argument class.
 */
void FKData::compute(gvector<Waveform *> &wvec, int windows,
			FKArgs fk_args)
{
#ifdef HAVE_GSL
    int	i, j, k, l, b, i1, i2, npts, n, f0, if1_plus_1;
    int	*f_lo, *f_hi;
    double sx, sy, re, im;
    double t0, t0_min, csx, snx, argx, csx0, csx1, csx2=0., snx1,
		snx2=0., *csy=NULL, *sny=NULL, argy, csy0, sum, domega;
    FComplex *f = NULL;
    double *t = NULL, *fsum=NULL;
    double *fsum1, *fsum2, *fsum3, *fsum4, *fsum5, *fsum6, *fsum7, *fsum8;
    float *fk_b, *data, *taper;
    GDataPoint **d1, **d2;

    args = fk_args;

    if(args.num_bands <= 0 || args.num_bands > MAX_NBANDS) {
	GError::setMessage("FKData: num_bands = %d", args.num_bands);
	throw(GERROR_INVALID_ARGS);
    }
    if(args.num_slowness <= 1) {
	GError::setMessage("FKData: num_slowness = %d", args.num_slowness);
	throw(GERROR_INVALID_ARGS);
    }
    if(args.num_slowness % 2 == 0) args.num_slowness++;

    for(i = 0; i < args.num_bands; i++) {
	if(args.fmin[i] >= args.fmax[i])
	{
	    GError::setMessage("FKData: lo freq (%f) >= hi freq (%f)",
			args.fmin[i], args.fmax[i]);
	    throw(GERROR_INVALID_ARGS);
	}
    }
    if(wvec.size() <= 0) {
	GError::setMessage("FKData: num_waveforms = %d", wvec.size());
	throw(GERROR_INVALID_ARGS);
    }
    
    d1 = new GDataPoint *[wvec.size()];
    d2 = new GDataPoint *[wvec.size()];

    if(windows)
    {
	if(windows > 1) {
	    for(i = 0; i < wvec.size(); i++) {
		d1[i] = wvec[i]->dw[0].d1;
		d2[i] = wvec[i]->dw[0].d2;
	    }
	}
	else {
	    d1[0] = wvec[0]->dw[0].d1;
	    d2[0] = wvec[0]->dw[0].d2;
	    for(i = 1; i < wvec.size(); i++) {
		d1[i] = wvec[i]->lowerBound(d1[0]->time());
		d2[i] = wvec[i]->upperBound(d2[0]->time());
	    }
	}
	for(i = 0; i < wvec.size(); i++) {
	    if(d1[i]->segmentIndex() != d2[i]->segmentIndex()) {
		GError::setMessage("FKData: data gap: %s/%s",
			wvec[i]->sta(), wvec[i]->chan());
		CLEAN_UP;
		throw(GERROR_INVALID_ARGS);
	    }
	}
	dt = d1[0]->segment()->tdel();
	t0_min = d1[0]->time();
	i1 = d1[0]->index();
	i2 = d2[0]->index();
	npts = i2 - i1 + 1;
    }
    else if(wvec[0]->size() > 1) {
	GError::setMessage("FKData: data gap: %s/%s",
		wvec[0]->sta(), wvec[0]->chan());
	throw(GERROR_INVALID_ARGS);
    }
    else {
	dt = wvec[0]->segment(0)->tdel();
	t0_min = wvec[0]->tbeg();
	i1 = 0;
	i2 = wvec[0]->length()-1;
	npts = wvec[0]->length();
    }
    Free(lat);
    Free(lon);
    if(!(lat = (double *)malloc(wvec.size()*sizeof(double))) ||
       !(lon = (double *)malloc(wvec.size()*sizeof(double))) )
    {
	CLEAN_UP;
	Free(lat); Free(lon);
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }

    if( !getCoordinates(wvec, lat, lon) ) {
	CLEAN_UP;
	Free(lat); Free(lon);
	throw(GERROR_INVALID_ARGS);
    }
	
    /* find the earliest beginning time of the waveform segments.
     * check for constant dt and npts;
     */
    for(i = 1; i < wvec.size(); i++)
    {
	double tdel_tol = wvec[0]->tdelTolerance();
	double ddt = (wvec[i]->segment(0)->tdel() - dt)/dt;
	if(ddt > tdel_tol) {
	    GError::setMessage("FKData:Waveforms have different sample rates.");
	    CLEAN_UP;
	    Free(lat); Free(lon);
	    throw(GERROR_INVALID_ARGS);
	}
	if(windows)
	{
	    t0 = d1[i]->time();
	    i1 = d1[i]->index();
	    i2 = d2[i]->index();
	}
	else if(wvec[i]->size() > 1) {
	    GError::setMessage("FKData: Data gap: %s/%s",
				wvec[i]->sta(), wvec[i]->chan());
	    CLEAN_UP;
	    Free(lat); Free(lon);
	    throw(GERROR_INVALID_ARGS);
	}
	else {
	    t0 = wvec[i]->tbeg();
	    i1 = 0;
	    i2 = wvec[i]->length() - 1;
	}
	if(npts > i2 - i1 + 1) npts = i2 - i1 + 1;
	if(t0 < t0_min) t0_min = t0;
    }

    /* compute the fft's for all waveforms
     */
    for(n = 2; n < npts; n *= 2);
    nt = n;
    if(!(t = (double *)malloc(nt*sizeof(double)))){
	CLEAN_UP;
	Free(lat); Free(lon);
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }

    df = 1./((double)nt*dt);

    checkTaper(args.taper_type);

    this->windowed = windows;
    this->tbeg = t0_min;
    this->tend = t0_min + (npts-1)*dt;
    this->d_slowness = 2*args.slowness_max/(double)(args.num_slowness-1);

    for(b = 0; b < args.num_bands; b++)
    {
	if(!(fk[b] = (float *)malloc(
			args.num_slowness*args.num_slowness*sizeof(float))))
	{
	    CLEAN_UP;
	    Free(lat); Free(lon);
	    GError::setMessage("FKData: malloc failed.");
	    throw(GERROR_INVALID_ARGS);
	}
    }

    f_lo = new int[args.num_bands];
    f_hi = new int[args.num_bands];

    for(i = 0; i < args.num_bands; i++) {
	if1 = (int)(args.fmin[i]/df);
	if2 = (int)(args.fmax[i]/df);
	if(if2*df < args.fmax[i]) if2++;
	if(if1 < 0) if1 = 0;
	if(if1 > n/2) if1 = n/2;
	if(if2 < 0) if2 = 0;
	if(if2 > n/2) if2 = n/2;
	f_lo[i] = if1;
	f_hi[i] = if2;
    }

    if1 = f_lo[0];
    if2 = f_hi[0];
    for(i = 1; i < args.num_bands; i++) {
	if(if1 > f_lo[i]) if1 = f_lo[i];
	if(if2 < f_hi[i]) if2 = f_hi[i];
    }
    for(i = 0; i < args.num_bands; i++) {
	f_lo[i] -= if1;
	f_hi[i] -= if1;
    }

    nf = if2 - if1 + 1;
    if1_plus_1 = if1+1;
    domega = 2.*M_PI*df;

    if(!(fsum = (double *)malloc(8*nf*sizeof(double))))
    {
	CLEAN_UP;
	Free(lat); Free(lon);
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }
    fsum1 = fsum;
    fsum2 = fsum + nf;
    fsum3 = fsum + 2*nf;
    fsum4 = fsum + 3*nf;
    fsum5 = fsum + 4*nf;
    fsum6 = fsum + 5*nf;
    fsum7 = fsum + 6*nf;
    fsum8 = fsum + 7*nf;

    if(!(f = (FComplex *)malloc(wvec.size()*nf*sizeof(FComplex))))
    {
	CLEAN_UP;
	Free(lat); Free(lon);
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }

    if( !(csy = (double *)malloc(wvec.size()*nf*sizeof(double))) ||
        !(sny = (double *)malloc(wvec.size()*nf*sizeof(double))) ||
        !(taper = (float *)malloc(npts*sizeof(float))) )
    {
        CLEAN_UP;
	Free(lat); Free(lon);
        GError::setMessage("FKData: malloc failed.");
        throw(GERROR_INVALID_ARGS);
    }
    for(i = 0; i < npts; i++) taper[i] = 1.;
    applyTaper(taper, npts);

    for(i = 0, f0 = 0; i < wvec.size(); i++, f0 += nf)
    {
	if(windows) {
	    data = d1[i]->segment()->data + d1[i]->index();
	}
	else {
	    data = wvec[i]->segment(0)->data;
	}
	for(j = 0; j < npts; j++) t[j] = (double)data[j];
	demean(t, npts);
	for(j = 0; j < npts; j++) t[j] *= taper[j];
	for(j = npts; j < nt; j++) t[j] = 0.;

	gsl_fft_real_radix2_transform(t, 1, nt);

	// normalize by 1./nt
	int n2 = nt/2;
	for(j = if1, k = 0; j <= if2; j++, k++) {
	    if(j == 0 || j == n2) im = 0.;
	    else im = t[nt-j];
	    re = t[j];
	    f[f0+k].re = re/nt;
	    f[f0+k].im = im/nt;
	}
    }
    Free(t);
    Free(taper);

    for(b = 0; b < args.num_bands; b++)
    {
	total_power[b] = 0.0;
	for(j = 0, f0 = 0; j < wvec.size(); j++, f0 += nf) {
	    for(i = f_lo[b]; i <= f_hi[b]; i++) {
		total_power[b] += f[f0+i].re*f[f0+i].re + f[f0+i].im*f[f0+i].im;
	    }
	}
	if(total_power[b] == 0.) total_power[b] = 1.;
	scaling[b] = 1./(total_power[b]*wvec.size());
	fk_max[b] = 0.;
    }

    /*
     * Loop over sx and sy.
     * find the x and y coordinates of each station in a coordinate
     * system with the z-axis at theta0, phi0, and the y-axis north:
     * Euler angles to this new system are phi0, theta0, pi/2.
     * Then compute the time lag as minus the dot product of the horizontal
     * slowness vector with the local station coordinates(x,y)
     */
    int num_slow2 = (args.num_slowness+1)/2;
    int zero_index = (args.num_slowness-1)/2;

    for(j = 0; j < num_slow2; j++)
    {
	sy = j*d_slowness;
	for(i = 0, f0 = 0; i < wvec.size(); i++, f0 += nf)
	{
	    argy = domega*sy*lat[i];
	    csy0 = cos(argy);
	    csy[f0] = cos(if1*argy);
	    sny[f0] = sin(if1*argy);

	    if(nf > 1) {
		csy[f0+1] = cos(if1_plus_1*argy);
		sny[f0+1] = sin(if1_plus_1*argy);
	    }
	    for(l = 2; l < nf; l++) {
		csy[f0+l] = 2.*csy[f0+l-1]*csy0 - csy[f0+l-2];
		sny[f0+l] = 2.*sny[f0+l-1]*csy0 - sny[f0+l-2];
	    }
	}

	for(k = 0; k < num_slow2; k++)
	{
	    sx = k*d_slowness;
	    for(l = 0; l < nf; l++) {
		fsum1[l] = 0.;
		fsum2[l] = 0.;
		fsum3[l] = 0.;
		fsum4[l] = 0.;
		fsum5[l] = 0.;
		fsum6[l] = 0.;
		fsum7[l] = 0.;
		fsum8[l] = 0.;
	    }

	    for(i = 0, f0 = 0; i < wvec.size(); i++)
	    {
//		t_lag = sx*lon[i] + sy*lat[i];
//		t0 = (windows) ? d1[i]->time() : wvec[i]->tbeg();
//		t_lag += (t0 - t0_min);
//		arg = domega*t_lag;
		argx = domega*sx*lon[i];
		csx0 = cos(argx);
		csx1 = cos(if1*argx);
		snx1 = sin(if1*argx);

		fsum1[0] += f[f0].re*csx1*csy[f0];
		fsum2[0] += f[f0].re*snx1*sny[f0];
		fsum3[0] += f[f0].im*snx1*csy[f0];
		fsum4[0] += f[f0].im*csx1*sny[f0];

		fsum5[0] += f[f0].im*csx1*csy[f0];
		fsum6[0] += f[f0].im*snx1*sny[f0];
		fsum7[0] += f[f0].re*snx1*csy[f0];
		fsum8[0] += f[f0].re*csx1*sny[f0];
		f0++;

		if(nf > 1) {
		    csx2 = cos(if1_plus_1*argx);
		    snx2 = sin(if1_plus_1*argx);

		    fsum1[1] += f[f0].re*csx2*csy[f0];
		    fsum2[1] += f[f0].re*snx2*sny[f0];
		    fsum3[1] += f[f0].im*snx2*csy[f0];
		    fsum4[1] += f[f0].im*csx2*sny[f0];

		    fsum5[1] += f[f0].im*csx2*csy[f0];
		    fsum6[1] += f[f0].im*snx2*sny[f0];
		    fsum7[1] += f[f0].re*snx2*csy[f0];
		    fsum8[1] += f[f0].re*csx2*sny[f0];
		    f0++;
		}
		for(l = 2; l < nf; l++, f0++) {
		    csx = 2.*csx2*csx0 - csx1;
		    snx = 2.*snx2*csx0 - snx1;
		    csx1 = csx2;
		    csx2 = csx;
		    snx1 = snx2;
		    snx2 = snx;

		    fsum1[l] += f[f0].re*csx*csy[f0];
		    fsum2[l] += f[f0].re*snx*sny[f0];
		    fsum3[l] += f[f0].im*snx*csy[f0];
		    fsum4[l] += f[f0].im*csx*sny[f0];

		    fsum5[l] += f[f0].im*csx*csy[f0];
		    fsum6[l] += f[f0].im*snx*sny[f0];
		    fsum7[l] += f[f0].re*snx*csy[f0];
		    fsum8[l] += f[f0].re*csx*sny[f0];
		}
	    }
	    for(b = 0; b < args.num_bands; b++)
	    {
		fk_b = fk[b];
		// for sx >= 0 and sy >= 0
		for(l = f_lo[b], sum = 0.; l <= f_hi[b]; l++) {
		    re = fsum1[l] - fsum2[l] + fsum3[l] + fsum4[l];
		    im = fsum5[l] - fsum6[l] - fsum7[l] - fsum8[l];
		    sum += re*re + im*im;
		}
		sum *= scaling[b];
		fk_b[(zero_index+j)*args.num_slowness + zero_index+k] = sum;
		if(sum > fk_max[b]) fk_max[b] = sum;

		// for sx < 0 and sy >= 0
		if(k > 0) {
		    for(l = f_lo[b], sum = 0.; l <= f_hi[b]; l++) {
		        re = fsum1[l] + fsum2[l] - fsum3[l] + fsum4[l];
			im = fsum5[l] + fsum6[l] + fsum7[l] - fsum8[l];
			sum += re*re + im*im;
		    }
		    sum *= scaling[b];
		    fk_b[(zero_index+j)*args.num_slowness + zero_index-k] = sum;
		    if(sum > fk_max[b]) fk_max[b] = sum;
		}

		// for sx >= 0 and sy < 0
		if(j > 0) {
		    for(l = f_lo[b], sum = 0.; l <= f_hi[b]; l++) {
			re = fsum1[l] + fsum2[l] + fsum3[l] - fsum4[l];
			im = fsum5[l] + fsum6[l] - fsum7[l] + fsum8[l];
			sum += re*re + im*im;
		    }
		    sum *= scaling[b];
		    fk_b[(zero_index-j)*args.num_slowness + zero_index+k] = sum;
		    if(sum > fk_max[b]) fk_max[b] = sum;
		}

		// for sx < 0 and sy < 0
		if(j > 0 && k > 0) {
		    for(l = f_lo[b], sum = 0.; l <= f_hi[b]; l++) {
			re = fsum1[l] - fsum2[l] - fsum3[l] - fsum4[l];
			im = fsum5[l] - fsum6[l] + fsum7[l] + fsum8[l];
			sum += re*re + im*im;
		    }
		    sum *= scaling[b];
		    fk_b[(zero_index-j)*args.num_slowness + zero_index-k] = sum;
		    if(sum > fk_max[b]) fk_max[b] = sum;
		}
	    }
	}
    }

    /*
     * find restricted max fk.
     */

    bool *peak_mask = createPeakMask(args.signal_slow_min, args.signal_slow_max,
				args.signal_az_min, args.signal_az_max);

    for(b = 0; b < args.num_bands; b++) {
	findMax(fk[b], b, peak_mask);
    }
    Free(peak_mask);

    for(b = 0; b < args.num_bands; b++)
    {
	if(restricted_fkmax[b] > fk_max[b]) fk_max[b] = restricted_fkmax[b];
    }

    computeFineGrid(f_lo, f_hi, f, fsum1, fsum2);

    delete [] f_lo;
    delete [] f_hi;

    if( !args.output_power )
    {
	for(b = 0; b < args.num_bands; b++)
	{
	    n = args.num_slowness * args.num_slowness;
	    fk_b = fk[b];
	    for(i = 0; i < n; i++) {
		fk_b[i] /= (1. - fk_b[i] + 1.e-06);
	    }

	    fk_b = fine.fk_fine[b];
	    for(i = 0; i < fine.n_fine[b]; i++) {
		fk_b[i] /= (1. - fk_b[i] + 1.e-06);
	    }

	    fk_max[b] /= (1. - fk_max[b] + 1.e-06);
	    restricted_fkmax[b] /= (1. - restricted_fkmax[b] + 1.e-06);
	    fstat[b] = restricted_fkmax[b] * (double)(wvec.size() - 1);
	}
    }
    else {
	for(b = 0; b < args.num_bands; b++)
	{
	    fstat[b] = (double)(wvec.size()-1)*restricted_fkmax[b]/
			(1.-restricted_fkmax[b]+1.e-06);
	}
    }

    // Normalize by max fk value, return value in dB
    for(b = 0; b < args.num_bands; b++)
    {
	n = args.num_slowness * args.num_slowness;
	fk_b = fk[b];
	for(i = 0; i < n; i++) {
	    fk_b[i] = 10.0 - 10.0 * log10(fk_max[b]/(double)fk_b[i]);
	}
	fk_b = fine.fk_fine[b];
	for(i = 0; i < fine.n_fine[b]; i++) {
	    fk_b[i] = 10.0 - 10.0 * log10(fk_max[b]/(double)fk_b[i]);
	}
    }

    CLEAN_UP;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

void FKData::computeFineGrid(int *f_lo, int *f_hi, FComplex *f,
		double *fsum1, double *fsum2)
{
    int b, i, j, k, l, f0, imax;
    double sx, sy, arg, cs0, cs1, sn1, cs2=0., sn2=0., cs, sn, new_re, new_im;
    double x0, y0, sum, h[3][3];
    double domega = 2.*M_PI*df;
    int if1_plus_1 = if1+1;
    float *fk_b;

    fine.n_slowfine = 11;
    fine.nbands = args.num_bands;

    for(b = 0; b < args.num_bands; b++)
    {
	fine.n_fine[b] = 0;
	fine.fk_fine[b] = NULL;
    }
    if( !args.fine_grid ) return;

    for(b = 0; b < args.num_bands; b++)
    {
	if(jmax[b] > 0 && jmax[b] < args.num_slowness-1 &&
	   kmax[b] > 0 && kmax[b] < args.num_slowness-1 && fine.n_slowfine > 3)
	{
	    /* make a finer sx,sy grid centered at jmax,kmax
	     */
	    fine.n_fine[b] = fine.n_slowfine*fine.n_slowfine;
	    fine.fk_fine[b] = (float *)mallocWarn(fine.n_fine[b]*sizeof(float));
	    fine.d_slowfine[b] = 2.*d_slowness/(fine.n_slowfine-1);
	    fine.slowfine_ymin[b] = -args.slowness_max + (jmax[b]-1)*d_slowness;
	    fine.slowfine_xmin[b] = -args.slowness_max + (kmax[b]-1)*d_slowness;
	
	    for(j = 0; j < fine.n_slowfine; j++)
	    {
		sy = fine.slowfine_ymin[b] + j*fine.d_slowfine[b];
		for(k = 0; k < fine.n_slowfine; k++)
		{
		    sx = fine.slowfine_xmin[b] + k*fine.d_slowfine[b];

		    for(l = 0; l < nf; l++) fsum1[l] = fsum2[l] = 0.;

		    for(i = 0, f0 = 0; i < nwaveforms; i++)
		    {
//			t_lag = sx*lon[i] + sy*lat[i];
//			t0 = windows ? d1[i]->time() : wvec[i]->tbeg();
//			t_lag += (t0 - t0_min);
//			arg = domega*t_lag;
			arg = domega*(sx*lon[i] + sy*lat[i]);
			cs0 = cos(arg);
			cs1 = cos(if1*arg);
			sn1 = sin(if1*arg);
			new_re = f[f0].re*cs1 + f[f0].im*sn1;
			new_im = f[f0].im*cs1 - f[f0].re*sn1;
			f0++;
			fsum1[0] += new_re;
			fsum2[0] += new_im;
			if(nf > 1)
			{
			    cs2 = cos(if1_plus_1*arg);
			    sn2 = sin(if1_plus_1*arg);
			    new_re = f[f0].re*cs2 + f[f0].im*sn2;
			    new_im = f[f0].im*cs2 - f[f0].re*sn2;
			    fsum1[1] += new_re;
			    fsum2[1] += new_im;
			    f0++;
			}
			for(l = 2; l < nf; l++, f0++)
			{
			    cs = 2.*cs2*cs0 - cs1;
			    sn = 2.*sn2*cs0 - sn1;
			    cs1 = cs2;
			    cs2 = cs;
			    sn1 = sn2;
			    sn2 = sn;
			    new_re = f[f0].re*cs + f[f0].im*sn;
			    new_im = f[f0].im*cs - f[f0].re*sn;
			    fsum1[l] += new_re;
			    fsum2[l] += new_im;
			}
		    }
		    for(l = f_lo[b], sum = 0.; l <= f_hi[b]; l++) {
			sum += fsum1[l]*fsum1[l] + fsum2[l]*fsum2[l];
		    }
		    sum *= scaling[b];
		    fine.fk_fine[b][j*fine.n_slowfine + k] = sum;
		}
	    }

	    fk_b = fine.fk_fine[b];
	    imax = 0;
	    sum = fk_b[0];
	    for(i = 1; i < fine.n_fine[b]; i++)
	    {
		if(sum < fk_b[i]) {
		    imax = i;
		    sum = fk_b[i];
		}
	    }
	    if(sum > restricted_fkmax[b]) {
		restricted_fkmax[b] = sum;
	    }

	    fine.jmax[b] = imax/fine.n_slowfine;
	    fine.kmax[b] = imax - fine.jmax[b]*fine.n_slowfine;

	    /* use parabolic fit to refine the max x,y
	     */
		
	    if( fine.jmax[b] > 0 && fine.jmax[b] < fine.n_slowfine-1 &&
		fine.kmax[b] > 0 && fine.kmax[b] < fine.n_slowfine-1)
	    {
		for(j = 0; j < 3; j++)
		    for(k = 0; k < 3; k++)
		{
		    h[k][j] = fk_b[(fine.jmax[b]-1+j)*fine.n_slowfine
				+ fine.kmax[b]-1+k];
		}
		fit2d(h, &x0, &y0);
		xmax[b] = fine.slowfine_xmin[b] +
			fine.kmax[b]*fine.d_slowfine[b] + x0*fine.d_slowfine[b];
		ymax[b] = fine.slowfine_ymin[b] +
			fine.jmax[b]*fine.d_slowfine[b] + y0*fine.d_slowfine[b];
	    }
	    else {
		xmax[b] = fine.slowfine_xmin[b] + fine.kmax[b]*fine.d_slowfine[b];
		ymax[b] = fine.slowfine_ymin[b] + fine.jmax[b]*fine.d_slowfine[b];
	    }
	}
    }
}

bool * FKData::createPeakMask(double sig_slow_min, double sig_slow_max,
				double sig_az_min, double sig_az_max)
{
    if(sig_slow_min < 0. && sig_slow_max < 0. &&
	sig_az_min < 0. && sig_az_max < 0.)
    {
	return NULL;
    }
    int n = args.num_slowness * args.num_slowness;
    bool *peak_mask = (bool *)mallocWarn(n*sizeof(bool));

    if(sig_slow_min < 0.) sig_slow_min = 0.;
    if(sig_slow_max < 0.) {
	double x = -args.slowness_max + (args.num_slowness-1)*d_slowness;
	sig_slow_max = sqrt(x*x + x*x);
    }
    if(sig_az_min < 0.) sig_az_min = 0.;
    if(sig_az_max < 0.) sig_az_min = 360.;

    int k = 0;
    for(int i = 0; i < args.num_slowness; i++)
    {
	for(int j = 0; j < args.num_slowness; j++)
	{
	    double x_slow = -args.slowness_max + i*d_slowness;
	    double y_slow = -args.slowness_max + j*d_slowness;
	    double slow = sqrt(x_slow*x_slow + y_slow*y_slow);
	    double az = atan2(x_slow, y_slow);
	    az *= (180./M_PI);
	    if(az < 0.) az += 360.;

	    if(sig_slow_min <= slow && slow <= sig_slow_max &&
		sig_az_min <= az && az <= sig_az_max)
	    {
		peak_mask[k] = true;
	    }
	    else {
		peak_mask[k] = false;
	    }
	    k++;
	}
    }

    return peak_mask;
}

/** Compute a single FK and save all frequencies.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] tmin the start of the time window.
 *  @param[in] tmax the end of the time window.
 *  @param[in] fk_args FK argument class.
 *  @throws GERROR_INVALID_ARGS
 *  @throws GERROR_MALLOC_ERROR
 */
void FKData::fullCompute(gvector<Waveform *> &wvec,
		double tmin, double tmax, FKArgs fk_args)
{
#ifdef HAVE_GSL
    int	i, j, k, l, i1, i2, npts, n;
    int	if1_plus_1, f0;
    double t0, t0_min, csx, snx, argx, csx0, csx1, csx2=0.,
	snx1, snx2=0., *csy=NULL, *sny=NULL, argy, csy0, sx, sy, re, im, domega;
    FComplex *f=NULL;
    float *data=NULL, *taper=NULL;
    double *t=NULL, *fsum=NULL;
    double *fsum1, *fsum2, *fsum3, *fsum4, *fsum5, *fsum6, *fsum7, *fsum8;
    GDataPoint **d1, **d2;

    args = fk_args;

    if(args.num_slowness <= 1) {
	GError::setMessage("FKData: num_slowness = %d", args.num_slowness);
	throw(GERROR_INVALID_ARGS);
    }
    if(args.num_slowness % 2 == 0) args.num_slowness++;

    if(wvec.size() <= 0) {
	GError::setMessage("FKData: num_waveforms = %d. Make sure that you selected same station/channel.", wvec.size());
	throw(GERROR_INVALID_ARGS);
    }

    init(wvec, args);

    d1 = new GDataPoint *[wvec.size()];
    d2 = new GDataPoint *[wvec.size()];

    for(i = 0; i < wvec.size(); i++) {
	d1[i] = wvec[i]->nearest(tmin);
	d2[i] = wvec[i]->nearest(tmax);
	if(d1[i]->segmentIndex() != d2[i]->segmentIndex()) {
	    GError::setMessage("FKData: data gap: %s/%s",
			wvec[i]->sta(), wvec[i]->chan());
	    delete [] d1;
	    delete [] d2;
	    throw(GERROR_INVALID_ARGS);
	}
    }
    dt = d1[0]->segment()->tdel();
    t0_min = d1[0]->time();
    i1 = d1[0]->index();
    i2 = d2[0]->index();
    npts = i2 - i1 + 1;

    Free(lat);
    Free(lon);
    if(!(lat = (double *)malloc(wvec.size()*sizeof(double))) ||
       !(lon = (double *)malloc(wvec.size()*sizeof(double))) )
    {
	CLEAN_UP_FULL;
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }

    for(i = 0; i < wvec.size(); i++) lat[i] = lon[i] = 0.;

    if( !getCoordinates(wvec, lat, lon) ) {
	CLEAN_UP_FULL;
	throw(GERROR_INVALID_ARGS);
    }

    /* find the earliest beginning time of the waveform segments.
     * check for constant dt and npts;
     */
    for(i = 1; i < wvec.size(); i++)
    {
	double tdel_tol = wvec[0]->tdelTolerance();
	double ddt = (wvec[i]->segment(0)->tdel() - dt)/dt;
	if(ddt > tdel_tol) {
	    GError::setMessage("FKData:Waveforms have different sample rates.");
	    CLEAN_UP_FULL;
	    throw(GERROR_INVALID_ARGS);
	}
	t0 = d1[i]->time();
	i1 = d1[i]->index();
	i2 = d2[i]->index();
	if(npts > i2 - i1 + 1) npts = i2 - i1 + 1;
	if(t0 < t0_min) t0_min = t0;
    }

    /* compute the fft's for all waveforms
     */
    for(n = 2; n < npts; n *= 2);
    nt = n;
    if(!(t = (double *)malloc(nt*sizeof(double)))){
	CLEAN_UP_FULL;
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }

    df = 1./((double)nt*dt);

    checkTaper(args.taper_type);

    this->tbeg = t0_min;
    this->tend = t0_min + (npts-1)*dt;
    this->d_slowness = 2.*args.slowness_max/(double)(args.num_slowness-1);

    // compute the full spectrum
    if1 = 1;
    if2 = n/2;

    nf = if2 - if1 + 1;
    if1_plus_1 = if1+1;
    domega = 2.*M_PI*df;

    if(!(fsum = (double *)malloc(8*nf*sizeof(double))))
    {
	CLEAN_UP_FULL;
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }
    fsum1 = fsum;
    fsum2 = fsum + nf;
    fsum3 = fsum + 2*nf;
    fsum4 = fsum + 3*nf;
    fsum5 = fsum + 4*nf;
    fsum6 = fsum + 5*nf;
    fsum7 = fsum + 6*nf;
    fsum8 = fsum + 7*nf;

    Free(complete);
    Free(fcomplete);
    if(!(complete =(float *)malloc(nf*
	args.num_slowness*args.num_slowness*sizeof(float)))
	|| !(fcomplete = (float *)malloc(nf*sizeof(float))) )
    {
	CLEAN_UP_FULL;
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }
    for(j = 0; j < nf; j++) fcomplete[j] = 0.;

    if( !(f = (FComplex *)malloc(wvec.size()*nf*sizeof(FComplex))) ||
	!(csy = (double *)malloc(wvec.size()*nf*sizeof(double))) ||
        !(sny = (double *)malloc(wvec.size()*nf*sizeof(double))) ||
        !(taper = (float *)malloc(npts*sizeof(float))) )
    {
	CLEAN_UP_FULL;
	GError::setMessage("FKData: malloc failed.");
	throw(GERROR_INVALID_ARGS);
    }
    for(i = 0; i < npts; i++) taper[i] = 1.;
    applyTaper(taper, npts);

    for(i = 0, f0 = 0; i < wvec.size(); i++, f0 += nf)
    {
	data = d1[i]->segment()->data + d1[i]->index();
	for(j = 0; j < npts; j++) t[j] = (double)data[j];
	demean(t, npts);
	for(j = 0; j < npts; j++) t[j] *= taper[j];
	for(j = npts; j < nt; j++) t[j] = 0.;

	gsl_fft_real_radix2_transform(t, 1, n);

	// normalize by 1./n
	int n2 = n/2;
	for(j = if1, k = 0; j <= if2; j++, k++) {
	    if(j == 0 || j == n2) im = 0.;
	    else im = t[n-j];
	    re = t[j];
	    f[f0+k].re = re/n;
	    f[f0+k].im = im/n;
	    fcomplete[k] += f[f0+k].re*f[f0+k].re + f[f0+k].im*f[f0+k].im;
	}
    }
    Free(t);
    Free(taper);

    int num_slow2 = (args.num_slowness+1)/2;
    int zero_index = (args.num_slowness-1)/2;

    /*
     * Loop over sx and sy.
     * find the x and y coordinates of each station in a coordinate
     * system with the z-axis at theta0, phi0, and the y-axis north:
     * Euler angles to this new system are phi0, theta0, pi/2.
     * Then compute the time lag as minus the dot product of the horizontal
     * slowness vector with the local station coordinates(x,y)
     */
    for(j = 0; j < num_slow2; j++)
    {
	sy = j*d_slowness;
	for(i = 0, f0 = 0; i < wvec.size(); i++, f0 += nf)
	{
	    argy = domega*sy*lat[i];
	    csy0 = cos(argy);
	    csy[f0] = cos(if1*argy);
	    sny[f0] = sin(if1*argy);

	    if(nf > 1) {
		csy[f0+1] = cos(if1_plus_1*argy);
		sny[f0+1] = sin(if1_plus_1*argy);
	    }
	    for(l = 2; l < nf; l++) {
		csy[f0+l] = 2.*csy[f0+l-1]*csy0 - csy[f0+l-2];
		sny[f0+l] = 2.*sny[f0+l-1]*csy0 - sny[f0+l-2];
	    }
        }

	for(k = 0; k < num_slow2; k++)
	{
	    sx = k*d_slowness;
	    for(l = 0; l < nf; l++) {
		fsum1[l] = 0.;
		fsum2[l] = 0.;
		fsum3[l] = 0.;
		fsum4[l] = 0.;
		fsum5[l] = 0.;
		fsum6[l] = 0.;
		fsum7[l] = 0.;
		fsum8[l] = 0.;
	    }

	    for(i = 0, f0 = 0; i < wvec.size(); i++)
	    {
//		t_lag = sx*lon[i] + sy*lat[i];
//		t0 = d1[i]->time();
//		t_lag += (t0 - t0_min);
		argx = domega*sx*lon[i];
		csx0 = cos(argx);
		csx1 = cos(if1*argx);
		snx1 = sin(if1*argx);
		csx0 = cos(argx);
		csx1 = cos(if1*argx);
		snx1 = sin(if1*argx);

		fsum1[0] += f[f0].re*csx1*csy[f0];
		fsum2[0] += f[f0].re*snx1*sny[f0];
		fsum3[0] += f[f0].im*snx1*csy[f0];
		fsum4[0] += f[f0].im*csx1*sny[f0];

		fsum5[0] += f[f0].im*csx1*csy[f0];
		fsum6[0] += f[f0].im*snx1*sny[f0];
		fsum7[0] += f[f0].re*snx1*csy[f0];
		fsum8[0] += f[f0].re*csx1*sny[f0];
		f0++;

		if(nf > 1) {
		    csx2 = cos(if1_plus_1*argx);
		    snx2 = sin(if1_plus_1*argx);

		    fsum1[1] += f[f0].re*csx2*csy[f0];
		    fsum2[1] += f[f0].re*snx2*sny[f0];
		    fsum3[1] += f[f0].im*snx2*csy[f0];
		    fsum4[1] += f[f0].im*csx2*sny[f0];

		    fsum5[1] += f[f0].im*csx2*csy[f0];
		    fsum6[1] += f[f0].im*snx2*sny[f0];
		    fsum7[1] += f[f0].re*snx2*csy[f0];
		    fsum8[1] += f[f0].re*csx2*sny[f0];
		    f0++;
		}
		for(l = 2; l < nf; l++, f0++) {
		    csx = 2.*csx2*csx0 - csx1;
		    snx = 2.*snx2*csx0 - snx1;
		    csx1 = csx2;
		    csx2 = csx;
		    snx1 = snx2;
		    snx2 = snx;

		    fsum1[l] += f[f0].re*csx*csy[f0];
		    fsum2[l] += f[f0].re*snx*sny[f0];
		    fsum3[l] += f[f0].im*snx*csy[f0];
		    fsum4[l] += f[f0].im*csx*sny[f0];

		    fsum5[l] += f[f0].im*csx*csy[f0];
		    fsum6[l] += f[f0].im*snx*sny[f0];
		    fsum7[l] += f[f0].re*snx*csy[f0];
		    fsum8[l] += f[f0].re*csx*sny[f0];
		}
	    }
	    int m = args.num_slowness * args.num_slowness;
	    int mm = (zero_index+j)*args.num_slowness + zero_index+k;
	    for(l = 0; l < nf; l++) {
		re = fsum1[l] - fsum2[l] + fsum3[l] + fsum4[l];
                im = fsum5[l] - fsum6[l] - fsum7[l] - fsum8[l];
		complete[l*m + mm] = re*re + im*im;
	    }
	    // for sx < 0 and sy >= 0
	    if(k > 0) {
		mm = (zero_index+j)*args.num_slowness + zero_index-k;
		for(l = 0; l < nf; l++) {
		    re = fsum1[l] + fsum2[l] - fsum3[l] + fsum4[l];
		    im = fsum5[l] + fsum6[l] + fsum7[l] - fsum8[l];
		    complete[l*m + mm] = re*re + im*im;
		}
	    }
	    // for sx >= 0 and sy < 0
	    if(j > 0) {
		mm = (zero_index-j)*args.num_slowness + zero_index+k;
		for(l = 0; l < nf; l++) {
		    re = fsum1[l] + fsum2[l] + fsum3[l] - fsum4[l];
		    im = fsum5[l] + fsum6[l] - fsum7[l] + fsum8[l];
		    complete[l*m + mm] = re*re + im*im;
		}
	    }
	    // for sx < 0 and sy < 0
	    if(j > 0 && k > 0) {
		mm = (zero_index-j)*args.num_slowness + zero_index-k;
		for(l = 0; l < nf; l++) {
		    re = fsum1[l] - fsum2[l] - fsum3[l] - fsum4[l];
		    im = fsum5[l] - fsum6[l] + fsum7[l] + fsum8[l];
		    complete[l*m + mm] = re*re + im*im;
		}
	    }
	}
    }

    CLEAN_UP_FULL;
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

void FKData::searchFBands(double scanbandw, double flow, double fhigh,
		double sig_slow_min, double sig_slow_max, double sig_az_min,
		double sig_az_max)
{
    int ibandw, i, j, l, m, nn;
    double scale, fkmax, f_stat, total_pow;
    double max_fkmax=0., max_fstat=0.;
    float *fk_tmp=NULL;

    scan_bandw = scanbandw;
    // Find the maximum fstat for a sliding frequency band
    ibandw = (int)(scan_bandw/df+.5) + 1;
    if(ibandw < 1) ibandw = 1;
    if(ibandw >= nf) ibandw = nf-1;

    int l1 = (int)(flow/df + .5);
    int l2 = (int)(fhigh/df + .5);
    if(l1 < 1) l1 = 1;
    if(l1 > nf) l1 = nf;
    if(l2 < 1) l2 = 1;
    if(l2 > nf) l2 = nf;

    nn = args.num_slowness * args.num_slowness;

    fk_tmp = (float *)mallocWarn(nn*sizeof(float));
    for(i = 0; i < MAX_NBANDS; i++) Free(fk[i]);
    fk[0] = (float *)mallocWarn(nn*sizeof(float));
    args.num_bands = 1;

    for(l = l1; l < l2-ibandw; l++)
    {
	total_pow = 0.0;
	m = l + ibandw-1;
	for(j = l; j <= m; j++) {
	    total_pow += fcomplete[j-1]; // there is no freq=0.
	}
	if(total_pow == 0.) total_pow = 1.;
	scale = 1./(total_pow*nwaveforms);

	for(i = 0; i < nn; i++) fk_tmp[i] = 0.;

	for(j = l; j <= m; j++) {
	    for(i = 0; i < nn; i++) {
		fk_tmp[i] += complete[j*nn + i];
	    }
	}
	fkmax = 0.;
	for(i = 0; i < nn; i++) {
	    fk_tmp[i] *= scale;
	    if(fkmax < fk_tmp[i]) {
		fkmax = fk_tmp[i];
	    }
	}
	f_stat = (fkmax/(1. - fkmax + 1.e-06)) * (double)(nwaveforms - 1);
	if(f_stat > max_fstat) {
	    max_fkmax = fkmax;
	    max_fstat = f_stat;
	    for(i = 0; i < nn; i++) fk[0][i] = fk_tmp[i];
	    args.fmin[0] = (if1+l)*df;
	    args.fmax[0] = (if1+m)*df;
	}
    }
    Free(fk_tmp);

    fk_max[0] = max_fkmax;

    /*
     * find restricted max fk.
     */
    bool *peak_mask = createPeakMask(sig_slow_min, sig_slow_max, sig_az_min,
					sig_az_max);

    findMax(fk[0], 0, peak_mask);

    Free(peak_mask);

    if(restricted_fkmax[0] > fk_max[0]) {
	fk_max[0] = restricted_fkmax[0];
    }
    if( !args.output_power )
    {
	for(i = 0; i < nn; i++) {
	    fk[0][i] /= (1. - fk[0][i] + 1.e-06);
	}
	fk_max[0] /= (1. - fk_max[0] + 1.e-06);
	restricted_fkmax[0] /= (1. - restricted_fkmax[0] + 1.e-06);
	fstat[0] = restricted_fkmax[0]*(double)(nwaveforms - 1);
    }
    else {
	fstat[0] = (double)(nwaveforms-1)*restricted_fkmax[0]/
				(1. - restricted_fkmax[0] + 1.e-06);
    }
    for(i = 0; i < nn; i++) {
	fk[0][i] = 10.0 - 10.0 * log10(fk_max[0]/fk[0][i]);
    }
}

bool FKData::getCoordinates(gvector<Waveform *> &wvec, double *slat,
			double *slon)
{
    int i, j, k, n, n_unique;
    double d, x, y, z, x0, y0, z0, theta, phi, theta0, phi0, lat0, lon0, rad;
    double radius = 6371.;
    double dist;

    for(i = 0; i < wvec.size(); i++) slat[i] = slon[i] = 0.;

    bool have_dnorth = false;

    /* use dnorth,deast, if we have them
     */
    for(i = 0; i < wvec.size(); i++)
    {
	slon[i] = wvec[i]->deast();
	slat[i] = wvec[i]->dnorth();
	/* the null value for deast and dnorth is 0.0!  check if all the
	 * slat[] and slon[] are 0.0.  If so, generate dnorth and deast from
	 * coords
	 */
	if(fabs(slon[i]) > .000001 || fabs(slat[i]) > .000001) {
	    have_dnorth = true;
	}
    }
    if(have_dnorth)
    {
	if(wvec[0]->lat() < -900. || wvec[0]->lon() < -900.)
	{
	    GError::setMessage("No station location for %s.", wvec[0]->sta());
	    return false;
	}
	center_lat = wvec[0]->lat() - slat[0]/DEG_TO_KM;
	center_lon = wvec[0]->lon() - slon[0]/DEG_TO_KM;
	stringcpy(center_sta, wvec[0]->sta(), sizeof(center_sta));
    }

    if(!have_dnorth)
    {
	/* first check if we have lat, lon for all selected waveforms.
	 */
	for(i = n = n_unique = 0; i < wvec.size(); i++)
	{
	    if(wvec[i]->lat() < -900. || wvec[i]->lon() < -900.)
	    {
		GError::setMessage("No station location for %s.",
				wvec[i]->sta());
		return false;
	    }
	    else
	    {
		n++;
		for(j = 0; j < n_unique; j++) {
		    if(slat[j] == wvec[i]->lat()
			&& slon[j] == wvec[i]->lon()) break;
		}
		if(j == n_unique) {
		    slat[n_unique] = wvec[i]->lat();
		    slon[n_unique] = wvec[i]->lon();
		    n_unique++;
		}
	    }
	}
	if(n == 0) {
	    GError::setMessage("FKData: no station locations");
	    return false;
	}
	else if(n_unique <= 1) {
	    GError::setMessage(
		"FKData: Only one unique station for the selected waveforms.");
	    return false;
	}

	/* compute the geometrical center of the stations.
	 */
	rad = M_PI/180.;
	x0 = y0 = z0 = 0;
	for(i = 0; i < n_unique; i++)
	{
	    /* cartesian coordinates */
	    theta = HALF_PI - slat[i]*rad;
	    phi = slon[i]*rad;
	    x = sin(theta)*cos(phi);
	    y = sin(theta)*sin(phi);
	    z = cos(theta);
	    x0 += x;
	    y0 += y;
	    z0 += z;
	}
	x0 /= n_unique;
	y0 /= n_unique;
	z0 /= n_unique;
	lat0 = 90. - atan2(sqrt(x0*x0+y0*y0), z0)/rad;
	lon0 = atan2(y0, x0)/rad;
	/*
	 * Use the station lat,lon closest to the center as the reference sta
	 */
	k = 0;
	dist = (slat[0]-lat0)*(slat[0]-lat0) +(slon[0]-lon0)*(slon[0]-lon0);
	for(i = 1; i < n_unique; i++)
	{
	    d =(slat[i]-lat0)*(slat[i]-lat0) +(slon[i]-lon0)*(slon[i]-lon0);
	    if(d < dist) {
		k = i;
		dist = d;
	    }
	}
	for(i = 0; i < wvec.size(); i++)
	{
	    if(slat[k] == wvec[i]->lat() && slon[k] == wvec[i]->lon())
	    {
		stringcpy(center_sta, wvec[i]->sta(), sizeof(center_sta));
		center_lat = wvec[i]->lat();
		center_lon = wvec[i]->lon();
		break;
	    }
	}
	theta0 = rad*(90. - slat[k]);
	phi0 = rad*slon[k];
		
	/* get local lat,lon from center in km.
	 */
	for(i = 0; i < wvec.size(); i++)
	{
	    theta = HALF_PI - rad*wvec[i]->lat();
	    phi = rad*wvec[i]->lon();
	    euler(&theta, &phi, phi0, theta0, HALF_PI);
	    slon[i] = radius*sin(theta)*cos(phi);
	    slat[i] = radius*sin(theta)*sin(phi);
	}
    }
    return true;
}

/** Check for a valid the taper_type.
 *  @param[in] taper_type the taper type. HANN_TAPER, HAMM_TAPER, PARZEN_TAPER,
 *	WELCH_TAPER, BLACKMAN_TAPER, COSINE_TAPER, or NO_TAPER.
 */
void FKData::checkTaper(int taper_type)
{
    if (taper_type != HANN_TAPER &&
	taper_type != HAMM_TAPER &&
	taper_type != PARZEN_TAPER &&
	taper_type != WELCH_TAPER &&
	taper_type != BLACKMAN_TAPER &&
	taper_type != COSINE_TAPER)
    {
	args.taper_type = NO_TAPER;
    }
}

#ifdef HAVE_GSL
static void
demean(double *t, int npts)
{
    if(npts > 0) {
	double mean = 0.;
	for(int i = 0; i < npts; i++) mean += t[i];
	mean /= npts;
	for(int i = 0; i < npts; i++) t[i] -= mean;
    }
}
#endif

/** Apply the taper. Calls Taper_hann(), Taper_hamm(), Taper_parzen(),
 *	Taper_welch(), Taper_blackman(), or Taper_cosine().
 *  @param[in] t the data.
 *  @param[in] npts the number of values in t[].
 */
void FKData::applyTaper(float *t, int npts)
{
    if(args.taper_type == HANN_TAPER)
    {
	Taper_hann(t, npts);
    }
    else if(args.taper_type == HAMM_TAPER)
    {
	Taper_hamm(t, npts);
    }
    else if(args.taper_type == PARZEN_TAPER)
    {
	Taper_parzen(t, npts);
    }
    else if(args.taper_type == WELCH_TAPER)
    {
	Taper_welch(t, npts);
    }
    else if(args.taper_type == BLACKMAN_TAPER)
    {
	Taper_blackman(t, npts);
    }
    else if(args.taper_type == COSINE_TAPER)
    {
	Taper_cosine(t, npts, args.taper_beg, args.taper_end);
    }
}

/** Find the location of the maximum FK value.
 *  Search the FK grid for each frequency band and store the maximum values in
 *  fk_max[] and store the location in xmax[], ymax[]. The location is refined
 *  to a point interior to four grid points by using a parabolic interpolation.
 *  If peak_mask is non-NULL, then it contains a bool value for each grid point
 *  (num_slowness*num_slowness), and the search for a maximum is limited to grid
 *  points with a peak_mask is equal to true.
 *  @param[in] f the FK values.
 *  @param[in] b the frequency band index.
 *  @param[in] peak_mask either NULL or a grid-mask of allowable grid points.
 */
void FKData::findMax(float *f, int b, bool *peak_mask)
{
    int i, j, k, n, imax;
    double max;
    double h[3][3], x0, y0;

    n = args.num_slowness * args.num_slowness;
    max = f[0];
    imax = 0;
    if(peak_mask) {
	for(i = 0; i < n && !peak_mask[i]; i++);
	if(i < n) {
	    max = f[i];
	    imax = i;
	}
    }
    for(i = 0; i < n; i++)
    {
	if((max < f[i])) {
	    if(!peak_mask || peak_mask[i]) {
		imax = i;
		max = f[i];
	    }
	}
    }
    restricted_fkmax[b] = max;

    jmax[b] = imax/args.num_slowness;
    kmax[b] = imax - jmax[b]*args.num_slowness;
    xmax[b] = -args.slowness_max + kmax[b]*d_slowness;
    ymax[b] = -args.slowness_max + jmax[b]*d_slowness;

    /* use parabolic fit to refine the max x,y
     */

    if( jmax[b] > 0 && jmax[b] < args.num_slowness-1 &&
	kmax[b] > 0 && kmax[b] < args.num_slowness-1)
    {
	for(j = 0; j < 3; j++)
	    for(k = 0; k < 3; k++)
	{
	    h[k][j] = f[(jmax[b]-1+j)*args.num_slowness + kmax[b]-1+k];
	}
	fit2d(h, &x0, &y0);
	if(fabs(x0) <= 1. && fabs(y0) <= 1.) {
	    xmax[b] = -args.slowness_max + kmax[b]*d_slowness + x0*d_slowness;
	    ymax[b] = -args.slowness_max + jmax[b]*d_slowness + y0*d_slowness;
	}
    }
}

#define FWRITE(ptr,size,num,fp) if((int)fwrite(ptr,size,num,fp) != num) return false
#define FREAD(ptr,size,num,fp) if((int)fread(ptr,size,num,fp) != num) return false

/** Write the FK information in this FKData instance to a file.
 *  @param[in] fp a FILE pointer.
 *  @returns true for success. Returns false if an error occurred.
 */
bool FKData::write(FILE *fp)
{
    FWRITE(&nwaveforms, sizeof(int), 1, fp);
    FWRITE(&shift, sizeof(double), 1, fp);
    FWRITE(net, 1, 10, fp);
    FWRITE(&windowed, sizeof(int), 1, fp);
    FWRITE(&args.num_bands, sizeof(int), 1, fp);
    FWRITE(&tbeg, sizeof(double), 1, fp);
    FWRITE(&tend, sizeof(double), 1, fp);
    FWRITE(&args.slowness_max, sizeof(double), 1, fp);
    FWRITE(&args.num_slowness, sizeof(int), 1, fp);
    FWRITE(&d_slowness, sizeof(double), 1, fp);
    FWRITE(args.fmin, sizeof(double), args.num_bands, fp);
    FWRITE(args.fmax, sizeof(double), args.num_bands, fp);
    FWRITE(center_sta, 1, 10, fp);
    FWRITE(&center_lat, sizeof(double), 1, fp);
    FWRITE(&center_lon, sizeof(double), 1, fp);
    FWRITE(local_average, sizeof(double), args.num_bands, fp);
    FWRITE(xmax, sizeof(double), args.num_bands, fp);
    FWRITE(ymax, sizeof(double), args.num_bands, fp);
    FWRITE(fk_max, sizeof(double), args.num_bands, fp);
    FWRITE(&tend, sizeof(double), 1, fp);
    for(int i = 0; i < args.num_bands; i++)
    {
	FWRITE(fk[i], sizeof(float),args.num_slowness*args.num_slowness, fp);
    }
    return true;
}

/** Read the FK information into this FKData instance from a file.
 *  @param[in] fp a FILE pointer.
 *  @returns true for success. Returns false if an error occurred.
 */
bool FKData::read(FILE *fp)
{
    FREAD(&nwaveforms, sizeof(int), 1, fp);
    FREAD(&shift, sizeof(double), 1, fp);
    FREAD(net, 1, 10, fp);
    FREAD(&windowed, sizeof(int), 1, fp);
    FREAD(&args.num_bands, sizeof(int), 1, fp);
    FREAD(&tbeg, sizeof(double), 1, fp);
    FREAD(&tend, sizeof(double), 1, fp);
    FREAD(&args.slowness_max, sizeof(double), 1, fp);
    FREAD(&args.num_slowness, sizeof(int), 1, fp);
    FREAD(&d_slowness, sizeof(double), 1, fp);
    FREAD(args.fmin, sizeof(double), args.num_bands, fp);
    FREAD(args.fmax, sizeof(double), args.num_bands, fp);
    FREAD(center_sta, 1, 10, fp);
    FREAD(&center_lat, sizeof(double), 1, fp);
    FREAD(&center_lon, sizeof(double), 1, fp);
    FREAD(local_average, sizeof(double), args.num_bands, fp);
    FREAD(xmax, sizeof(double), args.num_bands, fp);
    FREAD(ymax, sizeof(double), args.num_bands, fp);
    FREAD(fk_max, sizeof(double), args.num_bands, fp);
    FREAD(&tend, sizeof(double), 1, fp);
    for(int i = 0; i < args.num_bands; i++)
    {
	FREAD(fk[i], sizeof(float),args.num_slowness*args.num_slowness, fp);
    }
    return true;
}
