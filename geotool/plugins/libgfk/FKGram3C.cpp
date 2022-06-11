/** \file FKGram3C.cpp
 *  \brief Defines class FKComputeGram3C
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "FKGram3C.h"
#include "FKData.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/GCoverage.h"
#include "gobject++/GSegmentArray.h"
#include "IIRFilter.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
#include "tapers.h"
}

using namespace libgfk;

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif
#ifndef DEG_TO_KM
#define DEG_TO_KM	111.19492664	/* for R = 6371 */
#endif

static gvector<GSegmentArray*> *FKgetCoverage(gvector<Waveform*> &ws,
		double tmin, double tmax);
static bool getOrientation(Waveform *w, double *hang, double *vang);
static void matprod(double *A, double *x, int nrow, int ncol, double *y);


/** Constructor for a series of "three component FKs". The three-component FK
 *  is a power computation on an azimuth/incidence grid that resembles an FK.
 *  A series of FKData objects are computed for one or more frequency bands.
 *  FKData objects are computed for a sliding data window. The window's length
 *  and its overlap with the previous window are specified. An array of FKData
 *  objects is computed.
 *  @param[in] num_waveforms the number of waveforms in ws[]; must be 3.
 *  @param[in] ws the three component waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[in] max_slowness the maximum slowness value of the FK grid.
 *  @param[in] num_slowness the number of slowness values.
 *  @param[in] num_bands the number of frequency bands.
 *  @param[in] f_min an array of minimum frequencies for the bands.
 *  @param[in] f_max an array of maximum frequencies for the bands.
 *  @param[in] win_length the length of the sliding FK window in seconds.
 *  @param[in] win_overlap the overlap of one FK window with the previous
 *              window.
 *  @param[in] working_cb a callback function that reports the number of FK's
 *		computed. (or NULL)
 *  @param[in] taper the taper type
 *  @param[in] beg_taper for the cosine taper, the percentage of the waveform
 *      to taper at the beginning.
 *  @param[in] end_taper for the cosine taper, the percentage of the waveform
 *      to taper at the end.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *      - num_waveforms != 3
 *      - num_bands <= 0 or num_bands > MAX_NBANDS
 *      - num_slowness <= 1
 *      - f_min[i] >= f_max[i]
 *      - there are no hang and vang values for the waveforms.
 *      - the sample time intervals differ by more than GTimeSeries::tdelToleran
ce()
 *      - the window length is too short (the number of samples is < 5).
 *      - the window overlap is >= the window length.
 *      - there is no data coverage for any FK window. The waveforms are too
 *              short, or there are too many data gaps.
 *
 *  @throws GERROR_MALLOC_ERROR
 */
FKGram3C::FKGram3C(gvector<Waveform *> &ws, int windows,
	double max_slowness, int num_slowness,
	int num_bands, double *f_min, double *f_max, double win_length,
	double win_overlap, WorkingCB working_cb, int taper,
	double beg_taper, double end_taper)
{
    init(ws, windows, max_slowness,
	num_slowness, num_bands, f_min, f_max, win_length, win_overlap,
	working_cb, taper, beg_taper, end_taper);

    recompute(ws);
}

FKGram3C::~FKGram3C(void)
{
    Free(f);
    for(int i = 0; i < num_fks; i++) fkdata[i]->removeOwner(this);
    Free(fkdata);
}

/** Initilize the class members.
 *  @param[in] num_waveforms the number of waveforms in ws[]; must be 3.
 *  @param[in] ws the three component waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[in] max_slowness the maximum slowness value of the FK grid.
 *  @param[in] num_slowness the number of slowness values.
 *  @param[in] num_bands the number of frequency bands.
 *  @param[in] f_min an array of minimum frequencies for the bands.
 *  @param[in] f_max an array of maximum frequencies for the bands.
 *  @param[in] win_length the length of the sliding FK window in seconds.
 *  @param[in] win_overlap the overlap of one FK window with the previous
 *              window.
 *  @param[in] working_cb a callback function that reports the number of FK's
 *		computed. (or NULL)
 *  @param[in] taper the taper type
 *  @param[in] beg_taper for the cosine taper, the percentage of the waveform
 *      to taper at the beginning.
 *  @param[in] end_taper for the cosine taper, the percentage of the waveform
 *      to taper at the end.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *      - num_waveforms != 3
 *      - num_bands <= 0 or num_bands > MAX_NBANDS
 *      - num_slowness <= 1
 *      - f_min[i] >= f_max[i]
 *      - there are no hang and vang values for the waveforms.
 *  @throws GERROR_MALLOC_ERROR
 */
void FKGram3C::init(gvector<Waveform *> &ws, int windows,
	double max_slowness, int num_slowness,
	int num_bands, double *f_min, double *f_max, double win_length,
	double win_overlap, WorkingCB working_cb, int taper,
	double beg_taper, double end_taper)
{
    int i;

    /* check for valid input parameters
     */
    if(num_bands <= 0 || num_bands > MAX_NBANDS) {
	GError::setMessage("FKGram3C.init: num_bands = %d", num_bands);
	throw(GERROR_INVALID_ARGS);
    }
    if(num_slowness <= 1) {
	GError::setMessage("FKGram3C.init: num_slowness = %d", num_slowness);
	throw(GERROR_INVALID_ARGS);
    }
    if(ws.size() != 3) {
	GError::setMessage("FKGram3C.init: nwaveforms = %d", ws.size());
	throw(GERROR_INVALID_ARGS);
    }

    /* check the limits of each frequency band. must have f_max > f_min
     */
    for(i = 0; i < num_bands; i++) {
	if(f_min[i] >= f_max[i])
	{
	    GError::setMessage("FKGram3C.init: lo freq (%f) >= hi freq (%f)",
			f_min[i], f_max[i]);
	    throw(GERROR_INVALID_ARGS);
	}
    }

    for(i = 0; i < 3; i++) {
	waveform_ids[i] = ws[i]->getId();
    }
    windowed = windows;
    n_slowness = num_slowness;
    d_slowness = 2*max_slowness/(double)(n_slowness-1);
    slowness_max = max_slowness;
    nbands = num_bands;
    memcpy(fmin, f_min, nbands*sizeof(double));
    memcpy(fmax, f_max, nbands*sizeof(double));
    window_length = win_length;
    window_overlap = win_overlap;
    working_callback = working_cb;

    stringcpy(center_sta, ws[0]->sta(), sizeof(center_sta));
    center_lat = ws[0]->lat();
    center_lon = ws[0]->lon();

    for(i = 0; i < 3; i++) {
	if(!getOrientation(ws[i], &hang[i], &vang[i])) {
	    GError::setMessage("FKGram.init: no hang/vang for %s/%s",
		    ws[i]->sta(), ws[i]->chan());
	    throw(GERROR_INVALID_ARGS);
	}
    }
    for(i = 0; i < 3; i++)
    {
	double r, theta, phi;
	phi = DEG_TO_KM * vang[i];
	r = sin(phi);
	theta = (- hang[i] + 90) * DEG_TO_KM;
	p_site[i][0] = r * cos(theta);
	p_site[i][1] = r * sin(theta);
	p_site[i][2] = cos(phi);
    }

    window_width = 0;
    window_npts = 0;
    time0 = NULL_TIME;
    dt = 0.;
    fptr[0] = NULL;
    fptr[1] = NULL;
    fptr[2] = NULL;
    f_size = 0;
    f = NULL;
}

/** Calculate the number of FKs that will be computed.
 *  @param[in] v GSegmentArray objects that describe the data coverage.
 *  @param[in] window_overlap_npts the number of overlapping data values.
 *  @returns the number of FKs that will be computed.
 */
int FKGram3C::setup(gvector<GSegmentArray*> *v, int window_overlap_npts)
{
    int i, k, nfks;

    dk = window_npts - window_overlap_npts;

    /* count the number of FK's (nfks) that will be computed
     */
    nfks = 0;
    for(i = 0; i < v->size(); i++)
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
    return nfks;
}

/** Recompute the FKgram. Holding all other parameters constant, recompute
 *  the FKgram for the input waveforms. The waveform ids (wvec[i]->id)
 *  must be the same as when the FKGram was created. The data can be different.
 *  @param[in] num_waveforms the number of w
 *  @param[in] ws the waveforms as Waveform objects.
 */
int FKGram3C::recompute(gvector<Waveform *> &ws)
{
    int i, j, window_overlap_npts, nfks;
//    bool new_dt = false;
    double tmin, tmax;

    if(ws.size() != 3) {
	GError::setMessage("FKGram3C.compute: num_waveforms=%d", ws.size());
	throw(GERROR_INVALID_ARGS);
    }

    /* Put wvec in the original order and check that all elements are
     * present.
     */
    wvec.clear();
    for(i = 0; i < 3; i++) {
	for(j = 0; j < 3 && waveform_ids[i] != ws[j]->getId(); j++);
	if(j == 3) {
	    GError::setMessage("FKGram3C.compute invalid wvec.");
	    throw(GERROR_INVALID_ARGS);
	}
	wvec.push_back(ws[j]);
    }

    if(windowed)
    {
	tmin = wvec[0]->dw[0].d1->time();
	tmax = wvec[0]->dw[0].d2->time();
    }
    else {
	tmin = wvec[0]->tbeg();
	tmax = wvec[0]->tend();
	for(i = 1; i < 3; i++) {
	    if( tmin > wvec[i]->tbeg()) {
		tmin = wvec[i]->tbeg();
	    }
	    if( tmax < wvec[i]->tend()) {
		tmax = wvec[i]->tend();
	    }
	}
    }
    if(time0 == NULL_TIME) {
	double tdel_tol = wvec[0]->tdelTolerance();
	dt = wvec[0]->segment(0)->tdel();
	for(i = 1; i < 3; i++) {
	    double ddt = (wvec[i]->segment(0)->tdel()-dt)/dt;
	    if(ddt > tdel_tol) {
		GError::setMessage("Waveforms have different sample rates.");
		throw(GERROR_INVALID_ARGS);
            }
	}
    }
    else {
	/* Continue the windows */
	tmin = time0 + (window_length - window_overlap);
//	if(wvec[0]->segment(0)->tdel() != dt) new_dt = true;
	/* do something */
    }

    gvector<GSegmentArray*> *v = FKgetCoverage(wvec, tmin, tmax);

    if((int)v->size() == 0) {
	delete v;
	GError::setMessage("FKGram3C.recompute: no data coverage.");
	throw(GERROR_INVALID_ARGS);
    }

    /* loop over sliding fk-windows with window_overlap
     */
    window_width = (int)(window_length/dt + .5);
    window_npts = window_width + 1;
    if(window_npts < 5) {
	GError::setMessage("Window length too short for sample rate.");
	delete v;
	throw(GERROR_INVALID_ARGS);
    }
    window_overlap_npts = (int)(window_overlap/dt + .5);
    if(window_overlap_npts >= window_width) {
	GError::setMessage("Window overlap >= window length.");
	delete v;
	throw(GERROR_INVALID_ARGS);
    }
    if((nfks = setup(v, window_overlap_npts)) < 0) {
	delete v;
	throw(GERROR_INVALID_ARGS);
    }

    if((fkdata = allocateSpace(wvec, nfks)) == NULL)
    {
	delete v;
	GError::setMessage("FKGram3C.recompute: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }

    /* compute 'nfks' FKs for each of the 'nbands' frequency bands
     */
    computeFKData(v, &nfks, fkdata);

    delete v;

    num_fks = nfks;

    return num_fks;
}

static gvector<GSegmentArray*> *
FKgetCoverage(gvector<Waveform *> &ws, double tmin, double tmax)
{
    gvector<GSegmentArray*> *v;
    gvector<GTimeSeries *> ts;

    for(int i = 0; i < ws.size(); i++) {
	ts.push_back(ws[i]->ts);
    }

    v = GCoverage::getArrays(ts, tmin, tmax);

    return v;
}

/** Compute the FKs.
 *  @param[in] v a vector with GSegmentArray objects.
 *  @param[out] nfks the number of FKData objects computed.
 *  @param[out] fk_data an array of FKData objects.
 *  @returns true for success. Returns false if an error occurred.
 */
bool FKGram3C::computeFKData(gvector<GSegmentArray*> *v, int *nfks,
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
    for(i = 0; i < v->size(); i++)
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
bool FKGram3C::computeArray(GSegmentArray *sa, int windows, int *nfks,
			FKData **fk_data, int *nwork)
{
    int i, i1, j, k, l, b;
    double taper, t0, t1=0.;
    float *fk_b;
    IIRFilter *iir;

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

	/* Loop over all frequency bands
	 */
	for(b = 0; b < nbands; b++)
	{
	    /* Loop over the 3 components
	     */
	    for(i = 0; i < sa->num_segments; i++)
	    {
		GSegment *s = sa->segments[i];
		/* find the sample value nearest to time0 for each waveform.
		 */
		i1 = (int)((time0 - s->tbeg())/s->tdel() + .5);

		/* t0 = the time of this sample */
		t0 = s->tbeg() + i1*s->tdel();
		if(i == 0) t1 = t0;
		else if(t0 < t1) t1 = t0;
		
		/* copy the data for this waveform and this window into
		 * the f array.
		 */
		memcpy(fptr[i], s->data+i1, window_npts*sizeof(float));

		/* taper 10 points.
		 */
		taper = 10.1/(double)window_npts;
		Taper_cosine(fptr[i], window_npts, taper, taper);
		iir = new IIRFilter(3, "BP", fmin[b], fmax[b], dt,0);
		iir->applyMethod(fptr[i], window_npts, true);
		iir->deleteObject();

		/*
		if(hilbert && vang[i] > 45.0)
		{
		    Hilbert_data(npts, fptr[i]);
		}
		*/
	    }

	    tlen = dt*(window_npts-1);

	    for(j = 0, total_power = 0.; j < 3; j++)
	    {
		for(i = 0; i < window_npts; i++) {
		    total_power += fptr[j][i] * fptr[j][i];
		}
	    }
	    total_power /= tlen;

	    fk_b = fk_data[l]->fk[b];

	    slownessLoop(fk_b);
	}

	/* fill in the fk_data structure for this time window.
	 */
	fk_data[l]->nt = window_npts;
	fk_data[l]->dt = dt;
	    /* windows = true if the sliding fk windows are to be computed
	     * for only part of the waveforms specified by the 'a'
	     * double-line cursor in the main window.
	     */
	fk_data[l]->windowed = windows;
	    /* The time limits of the l'th window. */
	fk_data[l]->tbeg = t1;
	fk_data[l]->tend = t1 + window_width * dt;
	    /*  The slowness limits and the slowness sample rate. These
		are the same for all data windows. */
	fk_data[l]->args.slowness_max = slowness_max;
	fk_data[l]->args.num_slowness = n_slowness;
	fk_data[l]->d_slowness = d_slowness;
	    /*  the number of frequency bands. This is either 1 for the regular
		FK window, or 4 for the multi-band FK window */
	fk_data[l]->args.num_bands = nbands;
	for(b = 0; b < nbands; b++) {
	    fk_data[l]->args.fmin[b] = fmin[b];
	       fk_data[l]->args.fmax[b] = fmax[b];
	}
	/* Save the center of the array that will be used for beaming.
	 * This is either the reference location that dnorth and
	 * deast values refer to or in the absence of dnorth and deast
	 * values, it is the geometrical center of the array.
	 */
	stringcpy(fk_data[l]->center_sta, center_sta,
			sizeof(fk_data[l]->center_sta));
	fk_data[l]->center_lat = center_lat;
	fk_data[l]->center_lon = center_lon;

	for(b = 0; b < nbands; b++) {
	    fk_data[l]->findMax(fk_data[l]->fk[b]);
	}
    }
    *nfks = l;

    return true;
}

/** Allocate work space.
 *  @param[in] ws the waveforms as Waveform objects.
 *  @param[in] nfks the number of FKData objects that will be computed.
 */
FKData ** FKGram3C::allocateSpace(gvector<Waveform *> &ws, int nfks)
{
    int i, b, size;
    FKData **fk_data;

    size = 3*window_npts*sizeof(float);
    if(size > f_size) {
	Free(f);
	f_size = size;
	if(!(f = (float *)malloc(f_size))) return NULL;
    }
    for(i = 0; i < 3; i++) {
	fptr[i] = f + i*window_npts;
    }

    if(!(fk_data = (FKData **)malloc(nfks*sizeof(FKData *)))) return NULL;

    for(i = 0; i < nfks; i++)
    {
	fk_data[i] = new FKData(ws);
	fk_data[i]->addOwner(this);

	for(b = 0; b < nbands; b++)
	{
	    if(!(fk_data[i]->fk[b] = (float *)malloc(
			n_slowness*n_slowness*sizeof(float))))
	    {
		fk_data[i]->removeOwner(this);;
		while(--i >= 0) {
		    fk_data[i]->removeOwner(this);;
		}
		Free(fk_data);
		return NULL;
	    }
	}
    }

    return fk_data;
}

/** Loop over the slowness values.
 *  @param[out] fk_b an array of "fk" values.
 */
void FKGram3C::slownessLoop(float *fk_b)
{
    int i, j, k, l;
    double sx, sy, theta, t, p[3], r, sum, w[3], norm;

    for(j = 0, l = 0; j < n_slowness; j++)
    {
	sy = -slowness_max + j*d_slowness;
	for(k = 0; k < n_slowness; k++, l++)
	{
	    sx = -slowness_max + k*d_slowness;

	    theta = ((sy == 0.0 && sx == 0.0) ? 0.0 : atan2(sy,sx)) + M_PI;

	    if((r = sqrt(sx*sx + sy*sy)) <= 1.0)
	    {
		p[0] = r * cos(theta);
		p[1] = r * sin(theta);
		p[2] = cos(asin(r));

		matprod(&p_site[0][0], p, 3, 3, w);

		sum = 0.;
		for(i = 0; i < window_npts; i++)
		{
		    t = w[0]*fptr[0][i] + w[1]*fptr[1][i] + w[2]*fptr[2][i];
		    sum += t*t;
		}
		fk_b[l] = sum/tlen;
		/*
		norm = total_power - fkmap[l] + 1.e-05*total_power;
		*/
		norm = total_power;
		fk_b[l] /= norm;
	    }
	    else
	    {
		fk_b[l] = 0;
	    }
	}
    }
}

static bool
getOrientation(Waveform *w, double *hang, double *vang)
{
    if((*hang = w->hang()) > -900. && (*vang = w->vang()) > -900.)
    {
	return true;
    }
    return false;
}

static void
matprod(double *A, double *x, int nrow, int ncol, double *y)
{
    register int i, j, jstart;

    for(j = 0; j < nrow; j++)
    {
	y[j] = 0;

	jstart = j * ncol;

	for(i = 0; i < ncol; i++, jstart++)
	{
	    y[j] += A[jstart] * x[i];
	}
    }
}

