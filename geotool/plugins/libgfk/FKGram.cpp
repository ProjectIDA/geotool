/** \file FKGram.cpp
 *  \brief Defines routine FKComputeGram.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gsl/gsl_fft_real.h"

#include "FKGram.h"
#include "FKData.h"
#include "gobject++/GCoverage.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "gobject++/GSegmentArray.h"
#include "gobject++/gvector.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
#include "tapers.h"
}

using namespace libgfk;

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2	1.57079632679489661923
#endif
#ifndef DEG_TO_KM
#define DEG_TO_KM	111.19492664	/* for R = 6371 */
#endif

static bool useDnorthDeast(gvector<Waveform *> &wvec,
		double *lat, double *lon, double *center_lat,
		double *center_lon, char *center_sta, int len);
static bool useCenter(gvector<Waveform *> &wvec,
		double *lat, double *lon, double *center_lat,double *center_lon,
		char *center_sta, int len);
static gvector<GSegmentArray*> *FKgetCoverage(gvector<Waveform*> &wvec,
		double tmin, double tmax);
static void demean(double *t, int npts);

/** Constructor. */
FKWorkSpace::FKWorkSpace(void)
{
    f_lo = NULL;
    f_hi = NULL;
    t_size = 0;
    taper_size = 0;
    fsum_size = 0;
    f_size = 0;
    t0_size = 0;
    csx_size = 0;
    lags_size = 0;
    t = NULL;
    taper = NULL;
    fsum = NULL;
    fsum1 = NULL;
    fsum2 = NULL;
    fsum3 = NULL;
    fsum4 = NULL;
    fsum5 = NULL;
    fsum6 = NULL;
    fsum7 = NULL;
    fsum8 = NULL;
    complete = NULL;
    fk = NULL;
    f = NULL;
    lat = NULL;
    lon = NULL;
    t0 = NULL;
    csx = NULL;
    snx = NULL;
    csy = NULL;
    sny = NULL;
    lags = NULL;
}

FKWorkSpace::~FKWorkSpace(void)
{
    freeSpace();
}

/** Free all space allocated. */
void FKWorkSpace::freeSpace(void)
{
    Free(f_lo);
    Free(f_hi);
    Free(t);
    Free(taper);
    Free(fsum);
    Free(complete);
    Free(fk);
    Free(f);
    Free(lat);
    Free(lon);
    Free(t0);
    Free(csx);
    Free(snx);
    Free(csy);
    Free(sny);
    Free(lags);
}

/** Destructor */
FKGram::~FKGram(void)
{
    Free(peak_mask);
    for(int i = 0; i < num_fks; i++) fkdata[i]->removeOwner(this);
    Free(fkdata);
}

/** Constructor.
 *  Compute a series of FKData objects for one or more frequency bands. FK's are
 *  computed for a sliding data window. The window's length and its overlap with
 *  the previous window are specified. An array of FKData objects is computed.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[in] max_slowness the maximum slowness value of the FK grid.
 *  @param[in] num_slowness the number of slowness values.
 *  @param[in] num_bands the number of frequency bands.
 *  @param[in] f_min an array of minimum frequencies for the bands.
 *  @param[in] f_max an array of maximum frequencies for the bands.
 *  @param[in] win_length the length of the sliding FK window in seconds.
 *  @param[in] win_overlap the overlap of one FK window with the previous
 *		window.
 *  @param[in] working_cb a callback function that reports the number of FK's
 *		computed. (or NULL)
 *  @param[in] taper_type the taper type
 *  @param[in] taper_beg for the cosine taper, the percentage of the waveform
 *      to taper at the beginning.
 *  @param[in] taper_end for the cosine taper, the percentage of the waveform
 *      to taper at the end.
 *  @param[in] scan_frequencies if true, use a sliding frequency window of width
 *	scan_bandwith to search for the maximum FK value.
 *  @param[in] scan_bandwidth used if scan_frequencies is true.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *      - num_waveforms <= 0
 *      - num_bands <= 0 or num_bands > MAX_NBANDS
 *      - num_slowness <= 1
 *	- f_min[i] >= f_max[i]
 *      - there is no station location for a waveform (wvec[i]->lat < -900. || wvec[i]->lon < -900.)
 *      - all the station locations are the same.
 *      - the sample time intervals differ by more than GTimeSeries::tdelTolerance()
 *	- the window length is too short (the number of samples is < 5).
 *	- the window overlap is >= the window length.
 *	- there is no data coverage for any FK window. The waveforms are too
 *		short, or there are too many data gaps.
 *	- dnorth/deast values are not available and station locations are not
 *		available.
 *		
 *  @throws GERROR_MALLOC_ERROR
 */
FKGram::FKGram(gvector<Waveform *> &wvec, int windows,
	double max_slowness,int num_slowness,int num_bands,
	double *f_min, double *f_max, double win_length, double win_overlap,
	WorkingCB working_cb, int taper_type, double taper_beg,double taper_end,
	bool scan_frequencies, double scan_bandwidth)
{
    init(wvec, windows, max_slowness, num_slowness, num_bands, f_min, f_max,
	win_length, win_overlap, working_cb, -1., -1., -1., -1., taper_type,
	taper_beg, taper_end, scan_frequencies, scan_bandwidth);

    compute(wvec);
}

/** Constructor.
 *  Compute a series of FKData objects for one or more frequency bands. FK's are
 *  computed for a sliding data window. The window's length and its overlap with
 *  the previous window are specified. An array of FKData objects is computed.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[in] max_slowness the maximum slowness value of the FK grid.
 *  @param[in] num_slowness the number of slowness values.
 *  @param[in] num_bands the number of frequency bands.
 *  @param[in] f_min an array of minimum frequencies for the bands.
 *  @param[in] f_max an array of maximum frequencies for the bands.
 *  @param[in] win_length the length of the sliding FK window in seconds.
 *  @param[in] win_overlap the overlap of one FK window with the previous
 *		window.
 *  @param[in] working_cb a callback function that reports the number of FK's
 *		computed. (or NULL)
 *  @param[in] sig_slow_min (sec/km) the minimum slowness value for the
 *	signal max. Ignored if < 0.
 *  @param[in] sig_slow_max (sec/km) the maximum slowness value for the
 *	signal max. Ignored if < 0.
 *  @param[in] sig_az_min (degrees) the minimum azimuth value for the
 *	signal max. Ignored if < 0.
 *  @param[in] sig_az_max (degrees) the maximum azimuth value for the
 *	signal max. Ignored if < 0.
 *  @param[in] taper_type the taper type
 *  @param[in] taper_beg for the cosine taper, the percentage of the waveform
 *      to taper at the beginning.
 *  @param[in] taper_end for the cosine taper, the percentage of the waveform
 *      to taper at the end.
 *  @param[in] scan_frequencies if true, use a sliding frequency window of width
 *	scan_bandwith to search for the maximum FK value.
 *  @param[in] scan_bandwidth used if scan_frequencies is true.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *      - num_waveforms <= 0
 *      - num_bands <= 0 or num_bands > MAX_NBANDS
 *      - num_slowness <= 1
 *	- f_min[i] >= f_max[i]
 *      - there is no station location for a waveform (wvec[i]->lat < -900. || wvec[i]->lon < -900.)
 *      - all the station locations are the same.
 *      - the sample time intervals differ by more than GTimeSeries::tdelTolerance()
 *	- the window length is too short (the number of samples is < 5).
 *	- the window overlap is >= the window length.
 *	- there is no data coverage for any FK window. The waveforms are too
 *		short, or there are too many data gaps.
 *	- dnorth/deast values are not available and station locations are not
 *		available.
 *		
 *  @throws GERROR_MALLOC_ERROR
 */
FKGram::FKGram(gvector<Waveform *> &wvec, int windows,
	double max_slowness,int num_slowness,int num_bands,
	double *f_min, double *f_max, double win_length, double win_overlap,
	WorkingCB working_cb, double sig_slow_min, double sig_slow_max,
	double sig_az_min, double sig_az_max, int taper_type,
	double taper_beg, double taper_end, bool scan_frequencies,
	double scan_bandwidth)
{
    init(wvec, windows, max_slowness, num_slowness, num_bands,
	f_min, f_max, win_length, win_overlap, working_cb, sig_slow_min,
	sig_slow_max, sig_az_min, sig_az_max, taper_type, taper_beg, taper_end,
	scan_frequencies, scan_bandwidth);

    compute(wvec);
}

void FKGram::initMembers(void)
{
    windowed = 0;
    n_slowness = 0;
    d_slowness = 0.;
    slowness_max = 0.;
    nbands = 0;
    for(int i = 0; i < MAX_NBANDS; i++) fmin[i] = 0.;
    for(int i = 0; i < MAX_NBANDS; i++) fmax[i] = 0.;
    window_length = 0.;
    window_overlap = 0.;
    n = 0;
    nf = 0;
    if1 = 0;
    if2 = 0;
    dk = 0;
    window_width = 0;
    window_npts = 0;
    taper = 0;
    time0 = NULL_TIME;
    t0_min = 0.;
    dt = 0.;
    df = 0.;
    domega = 0.;
    center_lat = -999.;
    center_lon = -999.;
    memset(center_sta, 0, sizeof(center_sta));
    working_callback = NULL;
    peak_mask = NULL;
    full_compute = false;
    num_fks = 0;
    fkdata = NULL;
    scan_spectrum = false;
    scan_bandw = 0.;
}

/** Initialize class members.
 *  @param[in] num_waveforms the number of waveforms in wvec[].
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[in] max_slowness the maximum slowness value of the FK grid.
 *  @param[in] num_slowness the number of slowness values.
 *  @param[in] num_bands the number of frequency bands.
 *  @param[in] f_min an array of minimum frequencies for the bands.
 *  @param[in] f_max an array of maximum frequencies for the bands.
 *  @param[in] win_length the length of the sliding FK window in seconds.
 *  @param[in] win_overlap the overlap of one FK window with the previous
 *		window.
 *  @param[in] working_cb a callback function that gives the number of FK's
 *		computed. (or NULL)
 *  @param[in] sig_slow_min (sec/km) the minimum slowness value for the
 *	signal max. Ignored if < 0.
 *  @param[in] sig_slow_max (sec/km) the maximum slowness value for the
 *	signal max. Ignored if < 0.
 *  @param[in] sig_az_min (degrees) the minimum azimuth value for the
 *	signal max. Ignored if < 0.
 *  @param[in] sig_az_max (degrees) the maximum azimuth value for the
 *	signal max. Ignored if < 0.
 *  @param[in] taper_type the taper type
 *  @param[in] taper_beg for the cosine taper, the percentage of the waveform
 *      to taper at the beginning.
 *  @param[in] taper_end for the cosine taper, the percentage of the waveform
 *      to taper at the end.
 *  @param[in] scan_frequencies if true, use a sliding frequency window of width
 *	scan_bandwith to search for the maximum FK value.
 *  @param[in] scan_bandwidth used if scan_frequencies is true.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *      - num_waveforms <= 0
 *      - num_bands <= 0 or num_bands > MAX_NBANDS
 *      - num_slowness <= 1
 *	- f_min[i] >= f_max[i]
 *	- dnorth/deast values are not available and station locations are not
 *		available.
 *		
 *  @throws GERROR_MALLOC_ERROR
 */
void FKGram::init(gvector<Waveform *> &wvec, int windows,
	double max_slowness,int num_slowness,int num_bands,
	double *f_min, double *f_max, double win_length, double win_overlap,
	WorkingCB working_cb, double sig_slow_min, double sig_slow_max,
	double sig_az_min, double sig_az_max, int taper_type, double taper_beg,
	double taper_end, bool scan_frequencies, double scan_bandwidth)
{
    int i;

    initMembers();

    /* check for valid input parameters
     */
    if(num_bands <= 0 || num_bands > MAX_NBANDS) {
	GError::setMessage("FKGram.init: num_bands = %d", num_bands);
	throw(GERROR_INVALID_ARGS);
    }
    if(num_slowness <= 1) {
	GError::setMessage("FKGram.init: num_slowness = %d", num_slowness);
	throw(GERROR_INVALID_ARGS);
    }
    if(wvec.size() <= 0) {
	GError::setMessage("FKGram.init: nwaveforms = %d", wvec.size());
	throw(GERROR_INVALID_ARGS);
    }

    /* check the limits of each frequency band. must have f_max > f_min
     */
    for(i = 0; i < num_bands; i++) {
	if(f_min[i] >= f_max[i])
	{
	    GError::setMessage("FKGram.init: lo freq (%f) >= hi freq (%f)",
			f_min[i], f_max[i]);
	    throw(GERROR_INVALID_ARGS);
	}
    }

    waveforms.load(wvec);

    this->windowed = windows;
    this->n_slowness = num_slowness;
    this->d_slowness = 2*max_slowness/(double)(n_slowness-1);
    this->slowness_max = max_slowness;
    this->nbands = num_bands;
    memcpy(this->fmin, f_min, nbands*sizeof(double));
    memcpy(this->fmax, f_max, nbands*sizeof(double));
    this->window_length = win_length;
    this->window_overlap = win_overlap;
    this->working_callback = working_cb;
    this->signal_slow_min = sig_slow_min;
    this->signal_slow_max = sig_slow_max;
    this->signal_az_min = sig_az_min;
    this->signal_az_max = sig_az_max;
    this->taper = taper_type;
    this->beg_taper = taper_beg;
    this->end_taper = taper_end;
    this->scan_spectrum = scan_frequencies;
    this->scan_bandw = scan_bandwidth;
    if(!(peak_mask = (bool *)malloc(n_slowness*n_slowness*sizeof(bool))))
    {
	GError::setMessage("FKGram.init: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }

    if(!(ws.lat = (double *)malloc(wvec.size()*sizeof(double))))
    {
	GError::setMessage("FKGram.init: malloc failed.");
	Free(peak_mask);
	throw(GERROR_MALLOC_ERROR);
    }
    if(!(ws.lon = (double *)mallocWarn(wvec.size()*sizeof(double))))
    {
	GError::setMessage("FKGram.init: malloc failed.");
	Free(peak_mask);
	Free(ws.lat);
	throw(GERROR_MALLOC_ERROR);
    }

    /* use dnorth,deast, if we have them, else use the station that is the
     * closest to the geometrical center of the array.
     */
    if( ! useDnorthDeast(wvec, ws.lat, ws.lon, &center_lat, &center_lon,
		center_sta, sizeof(center_sta))  &&
	! useCenter(wvec, ws.lat, ws.lon, &center_lat, &center_lon,
		center_sta, sizeof(center_sta)) )
    {
	Free(peak_mask);
	Free(ws.lat); Free(ws.lon);
	throw(GERROR_MALLOC_ERROR);
    }

    if( !(ws.f_lo = (int *)malloc(nbands*sizeof(int))) ||
	!(ws.f_hi = (int *)malloc(nbands*sizeof(int))))
    {
	GError::setMessage("FKGram.init: malloc failed.");
	Free(peak_mask);
	Free(ws.lat); Free(ws.lon);
	Free(ws.f_lo); Free(ws.f_hi);
	throw(GERROR_MALLOC_ERROR);
    }

    num_fks = 0;
    fkdata = NULL;
}

/** Calculate the number of FKs that will be computed.
 *  @param[in] v GSegmentArray objects that describe the data coverage.
 *  @param[in] window_overlap_npts the number of overlapping data values.
 *  @returns the number of FKs that will be computed.
 */
int FKGram::setup(gvector<GSegmentArray*> *v, int window_overlap_npts)
{
    int i, j, k, nfks;
    double az, slow, x, y;

    /* compute the fft's for all waveforms
     */
    for(n = 2; n < window_npts; n *= 2);

    df = 1./((float)n*dt);

    dk = window_npts - window_overlap_npts;

    /* count the number of FK's (nfks) that will be computed
     */
    nfks = 0;
    for(i = 0; i < (int)v->size(); i++)
    {
	for(k = 0; k + window_width < v->at(i)->npts; k += dk) nfks++;
    }

    /* if window_width > data length, compute one FK
     */
/*
    if(nfks == 0) {
	window_npts = sa->npts;
	nfks = 1;
    }
*/

    /* Get the indices (f_lo[i], f_hi[i]) of the frequency bands. Each
     * band-limited FK will include energy for fk.f_lo[i] <= fk.f_hi[i].
     * The minimum frequency index for all bands is fk.if1 and the
     * max is fk.if2;
     */
    getBands();

    /* nf = the number of frequency values between the minimum and
     * the maximum frequencies the will be included in all bands.
     */
    nf = if2 - if1 + 1;
    domega = 2.*M_PI*df;

    if(signal_slow_min < 0.) signal_slow_min = 0.;
    if(signal_slow_max < 0.) {
        x = -slowness_max + (n_slowness-1)*d_slowness;
        signal_slow_max = x;
    }
    if(signal_az_min < 0.) signal_az_min = 0.;
    if(signal_az_max < 0.) signal_az_max = 360.;

    // 250 < az < 25 : 250 < az < 385
    if(signal_az_max < signal_az_min) {
	signal_az_max += 360.;
    }

    for(i=0, k=0; i < n_slowness; i++)
    {
	for(j = 0; j < n_slowness; j++)
	{
	    x = -slowness_max + i*d_slowness;
	    y = -slowness_max + j*d_slowness;
	    slow = sqrt(x*x + y*y);
	    az = atan2(x, y);
	    az *= (180./M_PI);
	    if(az < 0.) az += 360.;

	    if(signal_slow_min <= slow && slow <= signal_slow_max &&
		((signal_az_min <= az && az <= signal_az_max) ||
		 (signal_az_min <= az+360 && az+360 <= signal_az_max)) )
	    {
		peak_mask[k] = true;
	    }
	    else {
		peak_mask[k] = false;
	    }
	    k++;
	}
    }
    return nfks;
}

/** Recompute the FKgram. Holding all other parameters constant, compute
 *  the FKgram for the input waveforms. The waveform objects in wvec
 *  must be the same as when the FKGram was created, although they can be in
 *  a different order. The data can be different.
 *  @param[in] num_waveforms the number of w
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] append if true, FKData objects are computed starting at time
 * 	time0 are appended to the fkdata[] array.
 *  @param[in] save_time_secs If append is true and save_time_secs > 0.,
 *	FKData objects in fkdata[] that have a tbeg < the most recent
 *	fkdata[] - save_time_secs are discarded. If append is true and
 *	save_time_secs <= 0., FKData objects in fkdata[] that have
 *	tbeg < the current data minimum time are discarded.
 */
int FKGram::compute(gvector<Waveform *> &wvec, bool append,
			double save_time_secs)
{
    int i, j, k, window_overlap_npts, nfks;
    FKData **fk_data = NULL;
//    bool new_dt = false;
    double tmin, tmax;
    gvector<GSegmentArray*> *v;

    nfks = 0;

    if(wvec.size() != waveforms.size()) {
        GError::setMessage("FKGram: invalid num_waveforms=%d", wvec.size());
        throw(GERROR_INVALID_ARGS);
    }

    /* Put wvec in the original order and check that all elements are
     * present.
     */
    ws.wvec.clear();
    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < wvec.size() && waveforms[i] != wvec[j]; j++);
	if(j == wvec.size()) {
	    GError::setMessage("FKGram: invalid wvec.");
	    throw(GERROR_INVALID_ARGS);
	}
	ws.wvec.push_back(wvec[j]);
    }

    if(windowed)
    {
	tmin = ws.wvec[0]->dw[0].d1->time();
	tmax = ws.wvec[0]->dw[0].d2->time();
    }
    else {
	tmin = ws.wvec[0]->tbeg();
	tmax = ws.wvec[0]->tend();
	for(i = 1; i < ws.wvec.size(); i++) {
	    if(tmin > ws.wvec[i]->tbeg()) {
		tmin = ws.wvec[i]->tbeg();
	    }
	    if(tmax < ws.wvec[i]->tend()) {
		tmax = ws.wvec[i]->tend();
	    }
	}
    }

    if(!append || time0 == NULL_TIME) {
	double tdel_tol = ws.wvec[0]->tdelTolerance();
	dt = ws.wvec[0]->segment(0)->tdel();
	for(i = 1; i < ws.wvec.size(); i++) {
	    double ddt = (ws.wvec[i]->segment(0)->tdel() - dt)/dt;
	    if(ddt > tdel_tol) {
		GError::setMessage(
			"FKGram: waveforms have different sample rates.");
		throw(GERROR_INVALID_ARGS);
	    }
	}
    }
    else {
	/* Continue the windows */
	tmin = time0 + (window_length - window_overlap);
//	if(ws.wvec[0]->segment(0)->tdel() != dt) new_dt = true;
	/* do something */
    }

    tstart = tmin;
    tend = tmax;
    v = FKgetCoverage(ws.wvec, tmin, tmax);

    if((int)v->size() == 0) {
	delete v;
	GError::setMessage("FKGram: no data coverage.");
	throw(GERROR_INVALID_ARGS);
    }

    /* loop over sliding fk-windows with window_overlap
     */
    window_width = (int)(window_length/dt + .5);
    window_npts = window_width + 1;
    if(window_npts < 5) {
	GError::setMessage("FKGram: window length too short for sample rate.");
	v->deleteObject();
	throw(GERROR_INVALID_ARGS);
    }
    window_overlap_npts = (int)(window_overlap/dt + .5);
    if(window_overlap_npts >= window_width) {
	GError::setMessage("FKGram: window overlap >= window length.");
	v->deleteObject();
	throw(GERROR_INVALID_ARGS);
    }
    if((nfks = setup(v, window_overlap_npts)) < 0) {
	delete v;
	GError::setMessage("FKGram: data length too short.");
	throw(GERROR_INVALID_ARGS);
    }

    if(nfks)
    {
	if((fk_data = allocateSpace(ws.wvec, nfks)) == NULL)
	{
	    v->deleteObject();
	    GError::setMessage("FKGram.compute: malloc failed.");
	    throw(GERROR_MALLOC_ERROR);
	}

	computeTaper(window_npts, ws.taper);

	/* compute 'nfks' FKs for each of the 'nbands' frequency bands
	 */
	computeFKData(v, &nfks, fk_data);

	if( !append ) {
	    for(i = 0; i < num_fks; i++) fkdata[i]->removeOwner(this);
	    Free(fkdata);
	    fkdata = fk_data;
	    num_fks = nfks;
	}
	else
	{
	    /* If save_time_secs > 0., discard FKs that are older than the
	     * most recent FK - save_time_secs.
	     * If save_time_secs <= 0, discard FKs that are older than the
	     * oldest data.
	     */
	    if(save_time_secs > 0.) {
		tmax = ws.wvec[0]->tbeg();
		for(i = 1; i < ws.wvec.size(); i++) {
		    if(tmax < ws.wvec[i]->tbeg()) {
			tmax = ws.wvec[i]->tbeg();
		    }
		}
		tmin = tmax - save_time_secs;
	    }
	    else {
		tmin = ws.wvec[0]->tbeg();
		for(i = 1; i < ws.wvec.size(); i++) {
		    if(tmin > ws.wvec[i]->tbeg()) {
			tmin = ws.wvec[i]->tbeg();
		    }
		}
	    }
	    for(i = 0; i < num_fks; i++) {
		if(fkdata[i]->tbeg < tmin) {
		    fkdata[i]->removeOwner(this);
		    fkdata[i] = NULL;
		}
	    }
	    for(i = k = 0; i < num_fks; i++) {
		if(fkdata[i]) {
		    fkdata[k++] = fkdata[i];
		}
	    }
	    num_fks = k;

	    /* append new FKs
	     */
	    if((fkdata = (FKData **)reallocWarn(fkdata,
				(num_fks+nfks)*sizeof(FKData *))))
	    {
		for(i = 0; i < nfks; i++) {
		    fkdata[num_fks++] = fk_data[i];
		}
		Free(fk_data);
	    }
	}
    }
    delete v;

    return nfks;
}

int FKGram::fullCompute(gvector<Waveform *> &wvec, int index)
{
    int i, j, window_overlap_npts, nfks;
    FKData **fk_data = NULL;
    double tmin, tmax;
    gvector<GSegmentArray*> *v;

    if(index < 0 || index >= num_fks) return 0;

    nfks = 0;

    if(wvec.size() != waveforms.size()) {
        GError::setMessage("FKGram: invalid num_waveforms=%d", wvec.size());
        throw(GERROR_INVALID_ARGS);
    }

    /* Put wvec in the original order and check that all elements are
     * present.
     */
    ws.wvec.clear();
    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < wvec.size() && waveforms[i] != wvec[j]; j++);
	if(j == wvec.size()) {
	    GError::setMessage("FKGram: invalid wvec.");
	    throw(GERROR_INVALID_ARGS);
	}
	ws.wvec.push_back(wvec[j]);
    }

    /* Continue the index window only */
    tmin = fkdata[index]->tbeg;
    tmax = tmin + window_length;

    v = FKgetCoverage(ws.wvec, tmin, tmax);

    if((int)v->size() == 0) {
	delete v;
	GError::setMessage("FKGram: no data coverage.");
	throw(GERROR_INVALID_ARGS);
    }

    /* loop over sliding fk-windows with window_overlap
     */
    window_width = (int)(window_length/dt + .5);
    window_npts = window_width + 1;
    if(window_npts < 5) {
	GError::setMessage("FKGram: window length too short for sample rate.");
	delete v;
	throw(GERROR_INVALID_ARGS);
    }
    window_overlap_npts = (int)(window_overlap/dt + .5);
    if(window_overlap_npts >= window_width) {
	GError::setMessage("FKGram: window overlap >= window length.");
	delete v;
	throw(GERROR_INVALID_ARGS);
    }
    if((nfks = setup(v, window_overlap_npts)) < 0) {
	delete v;
	GError::setMessage("FKGram: data length too short.");
	throw(GERROR_INVALID_ARGS);
    }

    if(nfks)
    {
	full_compute = true;
	if((fk_data = allocateSpace(ws.wvec, nfks)) == NULL)
	{
	    delete v;
	    full_compute = false;
	    GError::setMessage("FKGram.compute: malloc failed.");
	    throw(GERROR_MALLOC_ERROR);
	}

	computeTaper(window_npts, ws.taper);

	/* compute 'nfks' FKs for each of the 'nbands' frequency bands
	 */
	computeFKData(v, &nfks, fk_data);

	fkdata[index]->removeOwner(this);
	fkdata[index] = fk_data[0];

	for(i = 1; i < nfks; i++) { // should not happen
	    fk_data[i]->removeOwner(this);
	}
 	Free(fk_data);
	full_compute = false;
    }
    v->deleteObject();

    return nfks;
}

static gvector<GSegmentArray*> *
FKgetCoverage(gvector<Waveform *> &wvec, double tmin, double tmax)
{
    gvector<GSegmentArray*> *v;
    gvector<GTimeSeries *> ts;

    for(int i = 0; i < wvec.size(); i++) {
	ts.push_back(wvec[i]->ts);
    }

    v = GCoverage::getArrays(ts, tmin, tmax);

    return v;
}


/** Compute the FKs.
 *  @param[in] v a gvector with GSegmentArray objects.
 *  @param[out] nfks the number of FKData objects computed.
 *  @param[out] fk_data an array of FKData objects.
 *  @returns true for success. Returns false if an error occurred.
 */
bool FKGram::computeFKData(gvector<GSegmentArray*> *v, int *nfks,
			FKData **fk_data)
{
    int i, nwork;

    if(working_callback) {
	(*working_callback)(*nfks, 0, "Computing %d FKs");
	if(!(*working_callback)(0, 1, NULL)) {
	    (*working_callback)(0, 2, NULL);
	    return false;
	}
    }

    nwork = 0;
    *nfks = 0;
    for(i = 0; i < (int)v->size(); i++)
    {
	if(!computeArray(v->at(i), windowed, nfks, fk_data, &nwork)) {
	    break;
	}
    }
    if(working_callback) {
	(*working_callback)(0, 2, NULL);
    }

    return true;
}

/** Compute the FKs for a GSegmentArray object.
 *  @param[in] sa a GSegmentArray object.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[out] nfks the number of FKData objects computed for this call.
 *  @param[out] fk_data an array of FKData objects.
 *  @param[in,out] nwork the total number of FKData objects computed.
 *  @returns true for success. Returns false if an error occurred.
 */
bool FKGram::computeArray(GSegmentArray *sa, int windows, int *nfks,
			FKData **fk_data, int *nwork)
{
    int i, i1, j, k, l, b, f0, m, n2;
    double t1=0., re, im;

    /* look over the sliding fk-windows 
     * sa->npts = total number of time samples
     * window_width = number of time samples in each window.
     * dk = window_npts - window_overlap_npts;
     */
    l = *nfks;
    for(k = 0; k + window_width < sa->npts; k += dk, l++)
    {
	/* time0 = the beginning time of this window.  */
	time0 = sa->tmin + k*dt;

	/* update the working dialog window after every fifth FK computed
	 */
	if(working_callback && ++(*nwork) == 5) {
	    *nwork = 0;
	    if(!(*working_callback)(l+1, 1, NULL)) {
		*nfks = l;
		return false;
	    }
	}

	/* Loop over all waveforms included in the FK computation.
	 * Compute the Fourier transform of each window and save the
	 * spectra in ws.f[i][m], where i = the time-window index and
	 * m is the frequency index for frequencies if1*df to
	 * if2*df.
	 */
	for(i = 0, f0=0; i < sa->num_segments; i++, f0 += nf)
	{
	    GSegment *s = sa->segments[i];
	    /* find the sample value nearest to time0 for each waveform.
	     */
	    i1 = (int)((time0 - s->tbeg())/s->tdel() + .5);

	    /* t0 = the time of this sample */
	    ws.t0[i] = s->tbeg() + i1*s->tdel();
		
	    /* copy the data for this waveform and this window into
	     * the ws.t array.
	     *
	     * n is the length of the Fourier Transform, which can be
	     * a little longer than the window length. Pad the time series
	     * with zeros from window_npts to n.
	     */
	    for(j = 0; j < window_npts; j++) ws.t[j] = (double)s->data[i1+j];
	    for(j = window_npts; j < n; j++) ws.t[j] = 0.;

	    /* demean and multiply the time samples by the taper.
	     */
	    demean(ws.t, window_npts);
	    if(taper != NO_TAPER) {
		for(j = 0; j < window_npts; j++) ws.t[j] *= ws.taper[j];
	    }

	    /* Fourier transform ws.t.
	     */
	    gsl_fft_real_radix2_transform(ws.t, 1, n);

	    /* save the spectra between if1 and if2, the global
	     * limits of all freqency bands. Scale by 1/n.
	     */
	    n2 = n/2;
	    for(j = if1, m = 0; j <= if2; j++, m++) {
		if(j == 0 || j == n2) im = 0.;
		else im = ws.t[n-j];
		re = ws.t[j];
		ws.f[f0+m].re = re/n;
		ws.f[f0+m].im = im/n;
	    }
	    if(i == 0) t1 = ws.t0[i];
	    else if(ws.t0[i] < t1) t1 = ws.t0[i];
	}
	/* fill in the fk_data structure for this time window.
	 */
	fk_data[l]->nt = n;
	fk_data[l]->dt = dt;
	    /* windows = true if the sliding fk windows are to be computed
	     * for only part of the waveforms specified by the 'a'
	     * double-line cursor in the main window.
	     */
	fk_data[l]->windowed = windows;
	    /* The time limits of the l'th window. */
	fk_data[l]->tbeg = ws.t0[0];
	fk_data[l]->tend = ws.t0[0] + window_width * dt;
	    /*  The slowness limits and the slowness sample rate. These
		are the same for all data windows. */
	fk_data[l]->args.slowness_max = slowness_max;
	fk_data[l]->args.num_slowness = n_slowness;
	fk_data[l]->d_slowness = d_slowness;
	    /*  the number of frequency bands. This is either 1 for
		the regular FK window, or 4 for the multi-band FK window
	     */
	fk_data[l]->args.num_bands = nbands;
	for(b = 0; b < nbands; b++) {
	    fk_data[l]->args.fmin[b] = fmin[b];
	    fk_data[l]->args.fmax[b] = fmax[b];
	}
	/* Save the center of the array that will be used for beaming.
	 * This is either the reference location that dnorth and deast values
 	 * refer to or in the absence of dnorth and deast values, it is the
	 * station closest to the geometrical center of the array.
	 */
	stringcpy(fk_data[l]->center_sta, center_sta,
			sizeof(fk_data[l]->center_sta));
	fk_data[l]->center_lat = center_lat;
	fk_data[l]->center_lon = center_lon;

	fk_data[l]->args.taper_type = taper;
	fk_data[l]->args.taper_beg = beg_taper;
	fk_data[l]->args.taper_end = end_taper;
	fk_data[l]->scan_spectrum = scan_spectrum;
	fk_data[l]->scan_bandw = scan_bandw;

	computeScaling(sa);

	/* compute the sines and cosines of the time lags needed in the
	 * FK computations.
	 */

	/* t1 = the minimum time of this window for all wavewforms.
	 * ws.t0[i] = the beginning time of this window for each waveform.
	 * Normally all the ws.t0[i] are the same, but they don't
	 * have to be, if the waveforms were not sampled uniformly from
	 * one waveform to another. Compute the data time lag of each
	 * waveform from time t1. This "data" time lag is added to the
	 * slowness time lags.
	 */
	if(l == 0) {
	    /* if k == 0, this is the first window and we must compute the
	     * sines and cosines of the time lags.
	     */
	    for(i = 0; i < ws.wvec.size(); i++) {
		ws.lags[i] = ws.t0[i] - t1;
	    }
	    computeSines(fk_data[l]);
	}
	else {
	    /* if k > 0, we might not have to compute the sines and
	     * cosines of the time lags. The slowness time lags computed in
	     * computeSines() do not change for each window. We only need
	     * to recompute if one of the "data" time lags is different
	     * for this window. For uniformly sample waveforms, the data
	     * time lags will always be zero, and we don't need to
	     * recompute.
	     */ 
#ifdef _OLD__
	    for(i = 0; i < nwaveforms; i++) {
		if(ws.lags[i] != ws.t0[i] - t1) break;
	    }
	    if(i < nwaveforms) {
		/* a data time lag has changed; need to recompute lags.
		 */
		for(i = 0; i < nwaveforms; i++) {
		    ws.lags[i] = ws.t0[i] - t1;
		}
		computeSines(fk_data[l]);
	    }
#endif
	}

	/* use the sines and cosines of all the slowness time lags, and the
	 * waveform spectra to compute the FK for this window.
	 */
	if( !scan_spectrum ) {
	    slownessLoop(fk_data[l]);
	}
	else {
	    slownessLoopSearch(sa, fk_data[l]);
	}
    }
    *nfks = l;

    return true;
}

static bool
useDnorthDeast(gvector<Waveform *> &wvec, double *lat,
		double *lon, double *center_lat, double *center_lon,
		char *center_sta, int len)
{
    int i;

    for(i = 0; i < wvec.size(); i++) lat[i] = lon[i] = 0.;

    /* use dnorth,deast, if we have them
     */
    for(i = 0; i < wvec.size(); i++)
    {
	lon[i] = wvec[i]->deast();
	lat[i] = wvec[i]->dnorth();
    }
    if(i == wvec.size())
    {
	if(wvec[0]->lat() < -900. || wvec[0]->lon() < -900.)
	{
	    GError::setMessage("No station location for %s.", wvec[0]->sta());
	    return false;
	}
	*center_lat = wvec[0]->lat() - lat[0]/DEG_TO_KM;
	*center_lon = wvec[0]->lon() - lon[0]/DEG_TO_KM;
	stringcpy(center_sta, wvec[0]->sta(), len);
    }
    /* the null value for deast and dnorth is 0.0!  check if all the
     * lat[] and lon[] are 0.0.  If so, generate dnorth and deast from
     * coords
     */
    for(i = 0; i < wvec.size(); i++) {
	if (lat[i] < -0.000001 || lat[i] > 0.000001 ||
	    lon[i] < -0.000001 || lon[i] > 0.000001) return true;
    }
    GError::setMessage("No deast/dnorth values.");
    return false;
}

static bool
useCenter(gvector<Waveform *> &wvec, double *lat, double *lon,
	double *center_lat, double *center_lon, char *center_sta, int len)
{
    int		i, j, k, n, n_unique;
    double	x, y, z, x0, y0, z0, theta, phi, theta0, phi0, lat0, lon0;
    double	rad, d, dist;
    double	radius = 6371.;

    /* first check if we have lat, lon for all selected waveforms.
     */
    for(i = n = n_unique = 0; i < wvec.size(); i++)
    {
	if(wvec[i]->lat() < -900. || wvec[i]->lon() < -900.)
	{
	    GError::setMessage("No station location for %s.", wvec[i]->sta());
	    return false;
	}
	else
	{
	    n++;
	    for(j = 0; j < n_unique; j++) {
		if(lat[j] == wvec[i]->lat() &&
		lon[j] == wvec[i]->lon()) break;
	    }
	    if(j == n_unique) {
		lat[n_unique] = wvec[i]->lat();
		lon[n_unique] = wvec[i]->lon();
		n_unique++;
	    }
	}
    }
    if(n == 0) {
	GError::setMessage("No station locations.");
	return false;
    }
    else if(n_unique == 1) {
	GError::setMessage(
		"Only one unique station for the selected waveforms.");
	return false;
    }
    /* compute the geometrical center of the stations.
     */
    rad = M_PI/180.;
    x0 = y0 = z0 = 0;
    for(i = 0; i < n_unique; i++)
    {
	/* cartesian coordinates */
	theta = M_PI_2 - lat[i]*rad;
	phi = lon[i]*rad;
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
     * Use the station lat,lon closest to the center as ref station
     */
    k = 0;
    dist = (lat[0]-lat0)*(lat[0]-lat0) +(lon[0]-lon0)*(lon[0]-lon0);
    for(i = 1; i < n_unique; i++)
    {
	d =(lat[i]-lat0)*(lat[i]-lat0) +(lon[i]-lon0)*(lon[i]-lon0);
	if(d < dist) {
	    k = i;
	    dist = d;
	}
    }
    for(i = 0; i < wvec.size(); i++)
    {
	if(lat[k] == wvec[i]->lat() && lon[k] == wvec[i]->lon())
	{
	    stringcpy(center_sta, wvec[i]->sta(), len);
	    *center_lat = wvec[i]->lat();
	    *center_lon = wvec[i]->lon();
	    break;
	}
    }
    theta0 = rad*(90. - lat[k]);
    phi0 = rad*lon[k];
		
    /* get local lat,lon from center in km.
     */
    for(i = 0; i < wvec.size(); i++)
    {
	theta = M_PI_2 - rad*wvec[i]->lat();
	phi = rad*wvec[i]->lon();
	euler(&theta, &phi, phi0, theta0, M_PI_2);
	lon[i] = radius*sin(theta)*cos(phi);
	lat[i] = radius*sin(theta)*sin(phi);
    }
    return true;
}

/** Compute the discrete frequencies that correspond to the frequency band
 *  limits.
 */
void FKGram::getBands(void)
{
    for(int i = 0; i < nbands; i++)
    {
	if1 = (int)(fmin[i]/df);
	if2 = (int)(fmax[i]/df + .5);
	if(if1 < 0) if1 = 0;
	if(if1 > n/2) if1 = n/2;
	if(if2 < 0) if2 = 0;
	if(if2 > n/2) if2 = n/2;
	ws.f_lo[i] = if1;
	ws.f_hi[i] = if2;
    }

    if1 = ws.f_lo[0];
    if2 = ws.f_hi[0];
    for(int i = 1; i < nbands; i++) {
	if(if1 > ws.f_lo[i]) if1 = ws.f_lo[i];
	if(if2 < ws.f_hi[i]) if2 = ws.f_hi[i];
    }
    for(int i = 0; i < nbands; i++) {
	ws.f_lo[i] -= if1;
	ws.f_hi[i] -= if1;
    }
}

/** Allocate work space.
 *  @param[in] wvec the waveforms as Waveform objects.
 *  @param[in] nfks the number of FKData objects that will be computed.
 */
FKData ** FKGram::allocateSpace(gvector<Waveform *> &wvec, int nfks)
{
    int i, b, nslow, size;
    FKData **fk_data;

    size = n*sizeof(double);
    if(size > ws.t_size) {
	Free(ws.t);
	ws.t_size = size;
	if(!(ws.t = (double *)malloc(ws.t_size))) return NULL;
    }
    size = 8*nf*sizeof(float);
    if(size > ws.fsum_size) {
	Free(ws.fsum);
	ws.fsum_size = size;
	if(!(ws.fsum = (float *)malloc(ws.fsum_size))) return NULL;
	ws.fsum1 = ws.fsum;
	ws.fsum2 = ws.fsum + nf;
	ws.fsum3 = ws.fsum + 2*nf;
	ws.fsum4 = ws.fsum + 3*nf;
	ws.fsum5 = ws.fsum + 4*nf;
	ws.fsum6 = ws.fsum + 5*nf;
	ws.fsum7 = ws.fsum + 6*nf;
	ws.fsum8 = ws.fsum + 7*nf;
    }
    size = waveforms.size()*sizeof(double);
    if(size > ws.t0_size) {
	Free(ws.t0);
	ws.t0_size = size;
	if(!(ws.t0 = (double *)malloc(ws.t0_size))) return NULL;
    }

    size = waveforms.size()*nf*sizeof(FComplex);
    if(size > ws.f_size) {
	Free(ws.f);
	ws.f_size = size;
	if(!(ws.f = (FComplex *)malloc(ws.f_size))) return NULL;
    }

    if(scan_spectrum) {
	Free(ws.complete);
	ws.complete = (float *)malloc(nf*n_slowness*n_slowness*sizeof(float));
	Free(ws.fk)
	ws.fk = (float *)malloc(n_slowness*n_slowness*sizeof(float));
    }

    if(!(fk_data = (FKData **)malloc(nfks*sizeof(FKData *)))) return NULL;

    for(i = 0; i < nfks; i++)
    {
	fk_data[i] = new FKData(wvec);
	fk_data[i]->addOwner(this);

	for(b = 0; b < nbands; b++)
	{
	    if(!(fk_data[i]->fk[b] = (float *)malloc(
			n_slowness*n_slowness*sizeof(float))))
	    {
		fk_data[i]->removeOwner(this);
		while(--i >= 0) {
		    fk_data[i]->removeOwner(this);
		}
		Free(fk_data);
		return NULL;
	    }
	}
	if(full_compute) {
	    if(!(fk_data[i]->complete = (float *)malloc(
			nf*n_slowness*n_slowness*sizeof(float))))
	    {
		fk_data[i]->removeOwner(this);
		while(--i >= 0) {
		    fk_data[i]->removeOwner(this);
		}
		Free(fk_data);
		return NULL;
	    }
	}
    }

    nslow = n_slowness*n_slowness;
    size = nslow*waveforms.size()*nf*sizeof(double);
    if(size > ws.csx_size) {
	Free(ws.csx);
	Free(ws.snx);
	Free(ws.csy);
	Free(ws.sny);
	ws.csx_size = size;
	if( !(ws.csx = (double *)malloc(ws.csx_size)) ||
	    !(ws.snx = (double *)malloc(ws.csx_size)) ||
	    !(ws.csy = (double *)malloc(ws.csx_size)) ||
	    !(ws.sny = (double *)malloc(ws.csx_size))) return NULL;
    }
    size = waveforms.size()*sizeof(double);
    if(size > ws.lags_size) {
	Free(ws.lags);
	ws.lags_size = size;
	if(!(ws.lags = (double *)malloc(ws.lags_size))) return NULL;
    }
    size = window_npts*sizeof(float);
    if(size > ws.taper_size) {
	Free(ws.taper);
	ws.taper_size = size;
	if(!(ws.taper = (float *)malloc(ws.taper_size))) return NULL;
    }

    return fk_data;
}

void FKGram::computeTaper(int window_len, float *tp)
{
    for(int i = 0; i < window_len; i++) tp[i] = 1.;

    if (taper == HANN_TAPER)
    {
	Taper_hann(tp, window_len);
    }
    else if (taper == HAMM_TAPER)
    {
	Taper_hamm(tp, window_len);
    }
    else if (taper == PARZEN_TAPER)
    {
	Taper_parzen(tp, window_len);
    }
    else if (taper == WELCH_TAPER)
    {
	Taper_welch(tp, window_len);
    }
    else if (taper == BLACKMAN_TAPER)
    {
	Taper_blackman(tp, window_len);
    }
    else if (taper == COSINE_TAPER)
    {
	Taper_cosine(tp, window_len, beg_taper, end_taper);
    }
}

void FKGram::computeSines(FKData *fkd)
{
    int	i, j, k, l, m, if1_plus_1;
    double sx, sy;
    double csx, snx, argx, csx0, csx1, csx2=0., snx1, snx2=0.;
    double csy, sny, argy, csy0, csy1, csy2=0., sny1, sny2=0.;

/*
    if(fkd->nwaveforms > 0) t0min = ws.t0[0];

    for(i = 1; i < fkd->nwaveforms; i++)
    {
	if(ws.t0[i] < t0min) t0min = ws.t0[i];
    }
*/
    if1_plus_1 = if1 + 1;
    /*
     * Loop over sx and sy and compute the time lags for each waveform.
     * Then compute the time lag as minus the dot product of the horizontal
     * slowness vector with the local station coordinates(x,y)
     */

    int nslow2 = (fkd->args.num_slowness+1)/2;
    m = 0;
    for(j = 0; j < nslow2; j++)
    {
	sy = j*fkd->d_slowness;
	for(k = 0; k < nslow2; k++)
	{
	    sx = k*fkd->d_slowness;
	    for(i = 0; i < fkd->nwaveforms; i++)
	    {
//		t_lag = sx*ws.lon[i] + sy*ws.lat[i];
//		t_lag += (ws.t0[i] - t0min);

		/* Compute the sines and cosines of the time lags.
		 * For speed, use recursion.
		 */
		argy = domega*sy*ws.lat[i];
		csy0 = cos(argy);
		csy1 = cos(if1*argy);
		sny1 = sin(if1*argy);
		ws.csy[m] = csy1;
		ws.sny[m] = sny1;

		argx = domega*sx*ws.lon[i];
		csx0 = cos(argx);
		csx1 = cos(if1*argx);
		snx1 = sin(if1*argx);
		ws.csx[m] = csx1;
		ws.snx[m] = snx1;
		m++;

		if(nf > 1) {
		    csy2 = cos(if1_plus_1*argy);
		    sny2 = sin(if1_plus_1*argy);
		    ws.csy[m] = csy2;
		    ws.sny[m] = sny2;

		    csx2 = cos(if1_plus_1*argx);
		    snx2 = sin(if1_plus_1*argx);
		    ws.csx[m] = csx2;
		    ws.snx[m] = snx2;
		    m++;
		}
		for(l = 2; l < nf; l++) {
		    csy = 2.*csy2*csy0 - csy1;
		    sny = 2.*sny2*csy0 - sny1;
		    csy1 = csy2;
		    csy2 = csy;
		    sny1 = sny2;
		    sny2 = sny;
		    ws.csy[m] = csy;
		    ws.sny[m] = sny;

		    csx = 2.*csx2*csx0 - csx1;
		    snx = 2.*snx2*csx0 - snx1;
		    csx1 = csx2;
		    csx2 = csx;
		    snx1 = snx2;
		    snx2 = snx;
		    ws.csx[m] = csx;
		    ws.snx[m] = snx;
		    m++;
		}
	    }
	}
    }
}

void FKGram::computeScaling(GSegmentArray *sa)
{
    int b, i, j, f0;

    for(b = 0; b < nbands; b++)
    {
	total_power[b] = 0.0;
	for(i = 0, f0 = 0; i < sa->num_segments; i++, f0 += nf) {
	    for(j = ws.f_lo[b]; j <= ws.f_hi[b]; j++) {
		total_power[b] += ws.f[f0+j].re*ws.f[f0+j].re
				+ ws.f[f0+j].im*ws.f[f0+j].im;
	    }
	}
	if(total_power[b] == 0.) total_power[b] = 1.;
	scaling[b] = 1./(total_power[b]*sa->num_segments);
    }
}

void FKGram::slownessLoop(FKData *fkd)
{
    int i, j, k, l, m, b, nslow2, zero_index, f0, nslow;
    double re, im, sum;

    /*
     * Loop over slowness sx and sy. Add the contribution of each
     * waveform to the FK.
     */

    nslow = fkd->args.num_slowness;
    nslow2 = (fkd->args.num_slowness+1)/2;
    zero_index = (fkd->args.num_slowness-1)/2;

    for(b = 0; b < fkd->args.num_bands; b++) fkd->fk_max[b] = 0.;

    m = 0;
    for(j = 0; j < nslow2; j++)
    {
	for(k = 0; k < nslow2; k++)
	{
	    for(l = 0; l < nf; l++) {
		ws.fsum1[l] = 0.;
		ws.fsum2[l] = 0.;
		ws.fsum3[l] = 0.;
		ws.fsum4[l] = 0.;
		ws.fsum5[l] = 0.;
		ws.fsum6[l] = 0.;
		ws.fsum7[l] = 0.;
		ws.fsum8[l] = 0.;
	    }

	    for(i = 0, f0 = 0; i < fkd->nwaveforms; i++)
	    {
		/* for each frequency from l=0 to nf, add up the
		 * contributions for each waveform.
		 */
		for(l = 0; l < nf; l++, m++, f0++) {
		    re = ws.f[f0].re;
		    im = ws.f[f0].im;
		    ws.fsum1[l] += re*ws.csx[m]*ws.csy[m];
		    ws.fsum2[l] += re*ws.snx[m]*ws.sny[m];
		    ws.fsum3[l] += im*ws.snx[m]*ws.csy[m];
		    ws.fsum4[l] += im*ws.csx[m]*ws.sny[m];

		    ws.fsum5[l] += im*ws.csx[m]*ws.csy[m];
		    ws.fsum6[l] += im*ws.snx[m]*ws.sny[m];
		    ws.fsum7[l] += re*ws.snx[m]*ws.csy[m];
		    ws.fsum8[l] += re*ws.csx[m]*ws.sny[m];
		}
	    }
	    /* for each frequency band, add up the frequency contributions
	     */
	    for(b = 0; b < fkd->args.num_bands; b++)
	    {
		// for sx >= 0 and sy >= 0
		for(l = ws.f_lo[b], sum = 0.; l <= ws.f_hi[b]; l++) {
		    re = ws.fsum1[l] - ws.fsum2[l] + ws.fsum3[l] + ws.fsum4[l];
		    im = ws.fsum5[l] - ws.fsum6[l] - ws.fsum7[l] - ws.fsum8[l];
		    sum += re*re + im*im;
		}
		sum *= scaling[b];
		fkd->fk[b][(zero_index+j)*nslow + zero_index+k] = sum;
		if(sum > fkd->fk_max[b]) fkd->fk_max[b] = sum;

		// for sx < 0 and sy >= 0
		if(k > 0) {
		    for(l = ws.f_lo[b], sum = 0.; l <= ws.f_hi[b]; l++) {
		     re = ws.fsum1[l] + ws.fsum2[l] - ws.fsum3[l] + ws.fsum4[l];
		     im = ws.fsum5[l] + ws.fsum6[l] + ws.fsum7[l] - ws.fsum8[l];
		     sum += re*re + im*im;
		    }
		    sum *= scaling[b];
		    fkd->fk[b][(zero_index+j)*nslow + zero_index - k]=sum;
		    if(sum > fkd->fk_max[b]) fkd->fk_max[b] = sum;
		}

		// for sx >= 0 and sy < 0
		if(j > 0) {
		    for(l = ws.f_lo[b], sum = 0.; l <= ws.f_hi[b]; l++) {
		     re = ws.fsum1[l] + ws.fsum2[l] + ws.fsum3[l] - ws.fsum4[l];
		     im = ws.fsum5[l] + ws.fsum6[l] - ws.fsum7[l] + ws.fsum8[l];
		     sum += re*re + im*im;
		    }
		    sum *= scaling[b];
		    fkd->fk[b][(zero_index-j)*nslow + zero_index + k]=sum;
		    if(sum > fkd->fk_max[b]) fkd->fk_max[b] = sum;
		}

		// for sx < 0 and sy < 0
		if(j > 0 && k > 0) {
		    for(l = ws.f_lo[b], sum = 0.; l <= ws.f_hi[b]; l++) {
		     re = ws.fsum1[l] - ws.fsum2[l] - ws.fsum3[l] - ws.fsum4[l];
		     im = ws.fsum5[l] - ws.fsum6[l] + ws.fsum7[l] + ws.fsum8[l];
		     sum += re*re + im*im;
		    }
		    sum *= scaling[b];
		    fkd->fk[b][(zero_index-j)*nslow + zero_index - k]=sum;
		    if(sum > fkd->fk_max[b]) fkd->fk_max[b] = sum;
		}
	    }
	    if(full_compute)
	    {
		/* save the slowness grid for all frequencies
		 */
		int nn = nslow*nslow;
		// for sx >= 0 and sy >= 0
		int jk = (zero_index+j)*nslow + zero_index+k;
		for(l = 0; l < nf; l++) {
		    re = ws.fsum1[l] - ws.fsum2[l] + ws.fsum3[l] + ws.fsum4[l];
		    im = ws.fsum5[l] - ws.fsum6[l] - ws.fsum7[l] - ws.fsum8[l];
		    fkd->complete[l*nn + jk] = re*re + im*im;
		}
		// for sx < 0 and sy >= 0
		if(k > 0) {
		    jk = (zero_index+j)*nslow + zero_index-k;
		    for(l = 0; l < nf; l++) {
		     re = ws.fsum1[l] + ws.fsum2[l] - ws.fsum3[l] + ws.fsum4[l];
		     im = ws.fsum5[l] + ws.fsum6[l] + ws.fsum7[l] - ws.fsum8[l];
		     fkd->complete[l*nn + jk] = re*re + im*im;
		    }
		}
		// for sx >= 0 and sy < 0
		if(j > 0) {
		    jk = (zero_index-j)*nslow + zero_index+k;
		    for(l = 0; l < nf; l++) {
		     re = ws.fsum1[l] + ws.fsum2[l] + ws.fsum3[l] - ws.fsum4[l];
		     im = ws.fsum5[l] + ws.fsum6[l] - ws.fsum7[l] + ws.fsum8[l];
		     fkd->complete[l*nn + jk] = re*re + im*im;
		    }
		}
		// for sx < 0 and sy < 0
		if(j > 0 && k > 0) {
		    jk = (zero_index-j)*nslow + zero_index-k;
		    for(l = 0; l < nf; l++) {
		     re = ws.fsum1[l] - ws.fsum2[l] - ws.fsum3[l] - ws.fsum4[l];
		     im = ws.fsum5[l] - ws.fsum6[l] + ws.fsum7[l] + ws.fsum8[l];
		     fkd->complete[l*nn + jk] = re*re + im*im;
		    }
		}
	    }
	}
    }
    /*
     * find max fk.
     */

    m = nslow*nslow;
    for(b = 0; b < fkd->args.num_bands; b++)
    {
	fkd->findMax(fkd->fk[b], b, peak_mask);

	if(fkd->restricted_fkmax[b] > fkd->fk_max[b]) {
	    fkd->fk_max[b] = fkd->restricted_fkmax[b];
	}
	if( !fkd->args.output_power )
	{
	    for(i = 0; i < m; i++) {
		fkd->fk[b][i] /= (1. - fkd->fk[b][i] + 1.e-06);
	    }
	    fkd->fk_max[b] /= (1. - fkd->fk_max[b] + 1.e-06);
	    fkd->restricted_fkmax[b] /= (1. - fkd->restricted_fkmax[b] +1.e-06);
	    fkd->fstat[b] = fkd->restricted_fkmax[b] *
				(double)(fkd->nwaveforms - 1);
	}
	else {
	    fkd->fstat[b] = (double)(fkd->nwaveforms-1)*fkd->restricted_fkmax[b]
				/ (1. - fkd->restricted_fkmax[b] + 1.e-06);
	}
	for(i = 0; i < m; i++) {
	    fkd->fk[b][i] = 10.0 - 10.0 * log10(fkd->fk_max[b]/fkd->fk[b][i]);
	}
    }
}

void FKGram::slownessLoopSearch(GSegmentArray *sa, FKData *fkd)
{
    int i, j, k, l, m, nslow2, nslow, zero_index, f0, nn;
    double re, im, scale, fkmax, max_fstat=0., max_fkmax=0.;

    /*
     * Loop over slowness sx and sy. Add the contribution of each
     * waveform to the FK.
     */
    nslow = fkd->args.num_slowness;
    nn = nslow*nslow;

    nslow2 = (fkd->args.num_slowness+1)/2;
    zero_index = (fkd->args.num_slowness-1)/2;

    m = 0;
    for(j = 0; j < nslow2; j++)
    {
	for(k = 0; k < nslow2; k++)
	{
	    for(l = 0; l < nf; l++) {
		ws.fsum1[l] = 0.;
		ws.fsum2[l] = 0.;
		ws.fsum3[l] = 0.;
		ws.fsum4[l] = 0.;
		ws.fsum5[l] = 0.;
		ws.fsum6[l] = 0.;
		ws.fsum7[l] = 0.;
		ws.fsum8[l] = 0.;
	    }

	    for(i = 0, f0 = 0; i < fkd->nwaveforms; i++)
	    {
		/* for each frequency from l=0 to nf, add up the
		 * contributions for each waveform.
		 */
		for(l = 0; l < nf; l++, m++, f0++) {
		    re = ws.f[f0].re;
		    im = ws.f[f0].im;
		    ws.fsum1[l] += re*ws.csx[m]*ws.csy[m];
		    ws.fsum2[l] += re*ws.snx[m]*ws.sny[m];
		    ws.fsum3[l] += im*ws.snx[m]*ws.csy[m];
		    ws.fsum4[l] += im*ws.csx[m]*ws.sny[m];

		    ws.fsum5[l] += im*ws.csx[m]*ws.csy[m];
		    ws.fsum6[l] += im*ws.snx[m]*ws.sny[m];
		    ws.fsum7[l] += re*ws.snx[m]*ws.csy[m];
		    ws.fsum8[l] += re*ws.csx[m]*ws.sny[m];
		}
	    }
	    /* save the slowness grid for all frequencies
	     */
	    // for sx >= 0 and sy >= 0
	    int jk = (zero_index+j)*nslow + zero_index+k;
	    for(l = 0; l < nf; l++) {
		re = ws.fsum1[l] - ws.fsum2[l] + ws.fsum3[l] + ws.fsum4[l];
		im = ws.fsum5[l] - ws.fsum6[l] - ws.fsum7[l] - ws.fsum8[l];
		ws.complete[l*nn + jk] = re*re + im*im;
	    }
	    // for sx < 0 and sy >= 0
	    if(k > 0) {
		jk = (zero_index+j)*nslow + zero_index-k;
		for(l = 0; l < nf; l++) {
		    re = ws.fsum1[l] + ws.fsum2[l] - ws.fsum3[l] + ws.fsum4[l];
		    im = ws.fsum5[l] + ws.fsum6[l] + ws.fsum7[l] - ws.fsum8[l];
		    ws.complete[l*nn + jk] = re*re + im*im;
		}
	    }
	    // for sx >= 0 and sy < 0
	    if(j > 0) {
		jk = (zero_index-j)*nslow + zero_index+k;
		for(l = 0; l < nf; l++) {
		    re = ws.fsum1[l] + ws.fsum2[l] + ws.fsum3[l] - ws.fsum4[l];
		    im = ws.fsum5[l] + ws.fsum6[l] - ws.fsum7[l] + ws.fsum8[l];
		    ws.complete[l*nn + jk] = re*re + im*im;
		}
	    }
	    // for sx < 0 and sy < 0
	    if(j > 0 && k > 0) {
		jk = (zero_index-j)*nslow + zero_index-k;
		for(l = 0; l < nf; l++) {
		    re = ws.fsum1[l] - ws.fsum2[l] - ws.fsum3[l] - ws.fsum4[l];
		    im = ws.fsum5[l] - ws.fsum6[l] + ws.fsum7[l] + ws.fsum8[l];
		    ws.complete[l*nn + jk] = re*re + im*im;
		}
	    }
	}
    }

    // Find the maximum fstat for a sliding frequency band
    int ibandw = (int)(scan_bandw/df+.5) + 1;
    if(ibandw < 1) ibandw = 1;
    if(ibandw >= nf) ibandw = nf-1;

    for(l = 0; l < nf-ibandw; l++)
    {
	double fstat, total_pow = 0.0;
	m = l + ibandw-1;
	for(i = 0, f0 = 0; i < sa->num_segments; i++, f0 += nf) {
	    for(j = l; j <= m; j++) {
		total_pow += ws.f[f0+j].re*ws.f[f0+j].re
				+ ws.f[f0+j].im*ws.f[f0+j].im;
	    }
	}
	if(total_pow == 0.) total_pow = 1.;
	scale = 1./(total_pow*sa->num_segments);

	for(i = 0; i < nn; i++) ws.fk[i] = 0.;

	for(j = l; j <= m; j++) {
	    for(i = 0; i < nn; i++) {
		ws.fk[i] += ws.complete[j*nn + i];
	    }
	}
	fkmax = 0.;
	for(i = 0; i < nn; i++) {
	    ws.fk[i] *= scale;
	    if(fkmax < ws.fk[i]) {
		fkmax = ws.fk[i];
	    }
	}
	fstat = (fkmax/(1. - fkmax + 1.e-06)) * (double)(sa->num_segments - 1);
	if(fstat > max_fstat) {
	    max_fkmax = fkmax;
	    max_fstat = fstat;
	    for(i = 0; i < nn; i++) fkd->fk[0][i] = ws.fk[i];
	    fkd->args.fmin[0] = (if1+l)*df;
	    fkd->args.fmax[0] = (if1+m)*df;
	}
    }
    fkd->fk_max[0] = max_fkmax;

    fkd->findMax(fkd->fk[0], 0, peak_mask);

    if(fkd->restricted_fkmax[0] > fkd->fk_max[0]) {
	fkd->fk_max[0] = fkd->restricted_fkmax[0];
    }
    if( !fkd->args.output_power )
    {
	for(i = 0; i < nn; i++) {
	    fkd->fk[0][i] /= (1. - fkd->fk[0][i] + 1.e-06);
	}
	fkd->fk_max[0] /= (1. - fkd->fk_max[0] + 1.e-06);
	fkd->restricted_fkmax[0] /= (1. - fkd->restricted_fkmax[0] + 1.e-06);
	fkd->fstat[0] = fkd->restricted_fkmax[0]*(double)(sa->num_segments - 1);
    }
    else {
	fkd->fstat[0] = (double)(sa->num_segments-1)*fkd->restricted_fkmax[0]/
				(1. - fkd->restricted_fkmax[0] + 1.e-06);
    }
    for(i = 0; i < nn; i++) {
	fkd->fk[0][i] = 10.0 - 10.0 * log10(fkd->fk_max[0]/fkd->fk[0][i]);
    }
}

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
