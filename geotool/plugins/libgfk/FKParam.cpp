/** \file FKParam.cpp
 *  \brief Defines FK signal analysis functions.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "FKParam.h"
#include "FKGram.h"
#include "FKGram3C.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"
#include "TaperData.h"
#include "IIRFilter.h"
#include "Beam.h"
#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
#include "fnan.h"
}

using namespace libgfk;

static void unwrapAzimuth(float *az, int n);
static void unwrap(float *p, int n);

FKParam::FKParam(void)
{
    memset(net, 0, sizeof(net));
    memset(sta, 0, sizeof(sta));
    memset(chan, 0, sizeof(chan));
    three_component = false;
    nbands = 0;
    fk_units = 0;

    num_fkdata = 0;
    size_fkdata = 0;
    fkdata = NULL;
    single_fk = NULL;
    num_waveforms = 0;
    waveform_ids = NULL;
    for(int i = 0; i < MAX_NBANDS; i++) { 
	min_fk[i] = 0.;
        max_fk[i] = 0.;
    }
    window_length = 0.;
    window_overlap = 0.;
    beam_input = false;

    x = NULL;
    fg = NULL;
    fg3C = NULL;
}

FKParam::~FKParam(void)
{
    freeSpace();
}

void FKParam::freeSpace(void)
{
//    delete single_fk;

    Free(x);

    for(int i = 0; i < 4; i++) {
	Free(sig[i].snr);
	Free(sig[i].fstat);
	Free(sig[i].appvel);
	Free(sig[i].azimuth);
	Free(sig[i].slowness);
	Free(sig[i].delaz);
	Free(sig[i].delslo);
    }
    size_fkdata = 0;
    num_fkdata = 0;
    fkdata = NULL;
    num_waveforms = 0;
}

void FKParam::findMinMax(void)
{
    int b, i, k, n;

    if(num_fkdata <= 0) return;

    for(b = 0; b < nbands; b++)
    {
	float minfk, maxfk;
	float *fk = &fkdata[0]->fk[b][0];

	minfk = fk[0];
	maxfk = fk[0];
	for(k = 0; k < num_fkdata; k++)
	{
	    n = fkdata[k]->args.num_slowness*fkdata[k]->args.num_slowness;
	    fk = &fkdata[k]->fk[b][0];
	    for(i = 0; i < n; i++){ 
		if(minfk > fk[i]) minfk = fk[i];
		if(maxfk < fk[i]) maxfk = fk[i];
	    }
	}
	this->min_fk[b] = minfk;
	this->max_fk[b] = maxfk;
    }
}

void FKParam::computeAzSlow(int fk_units, double x, double y, double fstat,
			double cfreq, float *az, float *app_vel,
			float *slowness, float *delaz, float *delslo)
{
    double sec_per_km, d_az;

    if(fk_units == FK_SEC_PER_DEG) {
	crosshair_to_slow_az(fk_units, x*DEG_TO_KM, y*DEG_TO_KM,
				&sec_per_km, &d_az);
    }
    else {
	crosshair_to_slow_az(fk_units, x, y, &sec_per_km, &d_az);
    }

    *az = (float)d_az;

    *slowness = sec_per_km;
//    if(*slowness == 0.) *slowness = 1.e-06;

    if(*slowness != 0.) {
	*app_vel = 1./ *slowness;
    }
    else {
	set_fnan(*app_vel);
    }

    if (fk_units == FK_SEC_PER_DEG) {
	*slowness = sec_per_km*DEG_TO_KM;
    }

/*
      slowness = sqrt(fk->xmax[imax]*fk->xmax[imax] +
                                fk->ymax[imax]*fk->ymax[imax]);
        arrival->azimuth = atan2(fk->xmax[imax], fk->ymax[imax]);
        arrival->azimuth *= (180./M_PI);
        if(arrival->azimuth < 0.) arrival->azimuth += 360;
*/
    double fk_dk = 0.017; // get this from arrival parameters
    *delslo = fk_dk/(sqrt(fstat)*cfreq);
    if(*slowness != 0.) {
	*delaz = *delslo/(2.* *slowness);
	if(*delaz < 1.0) {
	    *delaz = 2.*asin(*delaz)*180./M_PI;
	}
	else {
	    *delaz = 180;
	}
    }
    else {
	set_fnan(*delaz);
    }
//    arrival->slow = slowness * DEG_TO_KM;
//    arrival->delaz = delaz;
}

void FKParam::crosshair_to_slow_az(int fk_units, double scaled_x,
		double scaled_y, double *sec_per_km, double *az)
{
    double  slowness, a;

    slowness = sqrt(scaled_x*scaled_x + scaled_y*scaled_y);
    a = atan2(scaled_x, scaled_y);
    a *= (180./M_PI);
    if(a < 0.) a += 360;

    *az = a;
    *sec_per_km = (fk_units == FK_SEC_PER_KM)
				? slowness : slowness/DEG_TO_KM;
}
void FKParam::allocSignal(void)
{
    if( !fkdata || num_fkdata < size_fkdata) return;

    int n = num_fkdata*sizeof(float);

    if( !x )
    {
	if(!(x = (double *)mallocWarn(num_fkdata*sizeof(double)))) return;

	for(int j = 0; j < nbands; j++)
	{
	    if(!(sig[j].snr = (float *)mallocWarn(n))) return;
	    if(!(sig[j].fstat = (float *)mallocWarn(n))) return;
            if(!(sig[j].appvel = (float *)mallocWarn(n))) return;
            if(!(sig[j].azimuth = (float *)mallocWarn(n))) return;
            if(!(sig[j].slowness = (float *)mallocWarn(n))) return;
            if(!(sig[j].delaz = (float *)mallocWarn(n))) return;
            if(!(sig[j].delslo = (float *)mallocWarn(n))) return;
	}
    }
    else {
	if(!(x = (double *)reallocWarn(x, num_fkdata*sizeof(double)))) return;

	for(int j = 0; j < nbands; j++)
	{
	    if(!(sig[j].snr = (float *)reallocWarn(sig[j].snr, n))) return;
	    if(!(sig[j].fstat = (float *)reallocWarn(sig[j].fstat, n))) return;
            if(!(sig[j].appvel =(float *)reallocWarn(sig[j].appvel,n))) return;
            if(!(sig[j].azimuth = (float *)reallocWarn(sig[j].azimuth, n))) return;
            if(!(sig[j].slowness = (float *)reallocWarn(sig[j].slowness, n))) return;
            if(!(sig[j].delaz = (float *)reallocWarn(sig[j].delaz, n))) return;
            if(!(sig[j].delslo = (float *)reallocWarn(sig[j].delslo, n))) return;
	}
    }
    size_fkdata = num_fkdata;
}

void FKParam::updateSignalMeasurements(DataSource *ds, bool unwrapAz,
		BeamLocation beam_location, double stav_len, double ltav_len,
		gvector<Waveform *> &wvec)
{
    int i, j;
    double slow_sec_per_km, arrival_time;

    if(!fkdata || num_fkdata <= 0) return;

    allocSignal();

    for(i = 0; i < num_fkdata; i++) {
	x[i] = fkdata[i]->tbeg;
    }
    for(j = 0; j < nbands; j++)
    {
	for(i = 0; i < num_fkdata; i++)
	{
	    sig[j].fstat[i] = fkdata[i]->fstat[j];
	    double cfreq = .5*(fkdata[i]->args.fmin[j]+fkdata[i]->args.fmax[j]);
	    computeAzSlow(fk_units, fkdata[i]->xmax[j], fkdata[i]->ymax[j],
		    fkdata[i]->fstat[j], cfreq, &sig[j].azimuth[i],
		    &sig[j].appvel[i], &sig[j].slowness[i], &sig[j].delaz[i],
		    &sig[j].delslo[i]);

	    if(sig[j].appvel[i] > 0.) {
		slow_sec_per_km = 1./sig[j].appvel[i];
//		arrival_time = fkdata[i]->tend - stav_len;
		arrival_time = fkdata[i]->tbeg;
		computeSnr(ds, arrival_time, stav_len, ltav_len, beam_location,
			wvec, sig[j].azimuth[i], slow_sec_per_km,
			fkdata[i]->args.fmin[j], fkdata[i]->args.fmax[j],
			&sig[j].snr[i]);
	    }
	    else {
		sig[j].snr[i] = -1.;
	    }
	}

	// unwrap azimuth
	if(unwrapAz)
	{
	    unwrapAzimuth(sig[j].azimuth, num_fkdata);
	}
    }
}

bool FKParam::computeSnr(DataSource *ds, double time, double stav_len,
		double ltav_len, BeamLocation beam_location,
		gvector<Waveform *> &wvec, double az,
		double slow_sec_per_km, double fmin, double fmax, float *snr)
{
    double beam_lat, beam_lon;
    vector<double> t_lags;
    GTimeSeries *ts;
    IIRFilter *iir = NULL;
    TaperData *taper;

    *snr = 0.;

    if( !Beam::getTimeLags(ds, wvec, az, slow_sec_per_km,
		beam_location, t_lags, &beam_lat, &beam_lon) ) return false;

    ts = Beam::BeamSubSeries(wvec, t_lags, time-ltav_len-1.,
			time+window_length+stav_len, true);

    if(!ts) {
	*snr = -1.;
	return false;
    }
    try {
	iir = new IIRFilter(3, "BP", fmin, fmax, ts->segment(0)->tdel(), 0);
    }
    catch(...) {
	cerr << GError::getMessage() << endl;
	return false;
    }
    taper = new TaperData("cosine", 10, 5, 200);
    taper->apply(ts);
    iir->apply(ts);

    ts->addOwner(this);

    *snr = getSnr(ts, time, window_length, stav_len, ltav_len);

    ts->removeOwner(this);

    return true;
}

double FKParam::getSnr(GTimeSeries *ts, double time, double window_length,
			double stav_len, double ltav_len)
{
    double stav, ltav, max_snr, tdel, snr, tmin;
    GDataPoint *d1, *d2;
    int n;

    d1 = ts->upperBound(time - ltav_len);
    d2 = ts->lowerBound(time);
    if(d2->time() - d1->time() < ltav_len-1.) {
	delete d1;
	delete d2;
	return -1.; // not enough data for the ltav window
    }
    ltav = 0.;
    n = 0;
    for(int j = d1->segmentIndex(); j <= d2->segmentIndex(); j++) {
        GSegment *s = ts->segment(j);
        int i1 = (j == d1->segmentIndex()) ? d1->index() : 0;
        int i2 = (j < d2->segmentIndex()) ? s->length()-1 : d2->index();
        for(int i = i1; i <= i2; i++) {
            ltav += fabs(s->data[i]);
            n++;
        }
    }
    if(n) ltav /= n;
    delete d1;
    delete d2;

    if(ltav == 0.) return -1.;

    d1 = ts->upperBound(time);
    tdel = d1->segment()->tdel();
    delete d1;

    max_snr = 0.;

    for(tmin = time; tmin+tdel < time+window_length+.5; tmin += tdel)
    {
	d1 = ts->upperBound(tmin);
	d2 = ts->lowerBound(tmin + stav_len);
	stav = 0.;
	n = 0;
	for(int j = d1->segmentIndex(); j <= d2->segmentIndex(); j++) {
	    GSegment *s = ts->segment(j);
	    int i1 = (j == d1->segmentIndex()) ? d1->index() : 0;
	    int i2 = (j < d2->segmentIndex()) ? s->length()-1 : d2->index();
	    for(int i = i1; i <= i2; i++) {
		stav += fabs(s->data[i]);
		n++;
	    }
	}
	if(n) {
	    stav /= n;
	    snr = stav/ltav;
	    if(snr > max_snr) max_snr = snr;
	}
	delete d1;
	delete d2;
    }
    return max_snr;
}

static void
unwrapAzimuth(float *az, int n)
{
    int i;

    for(i = 0; i < n; i++) {
	az[i] *= DEG_TO_RADIANS;
    }
    unwrap(az, n);
    for(i = 0; i < n; i++) {
	az[i] /= DEG_TO_RADIANS;
    }
}

static void
unwrap(float *p, int n)
{
    int i;
    float this_p, last_p;
    double shift, two_pi;
    double pi = 3.14159265358979;

    if(n <= 0) return;

    two_pi = 2.*pi;
    shift = 0.;
    last_p = atan2(sin(p[0]), cos(p[0])); /* force p between +/- PI */
    p[0] = last_p;
    for(i = 1; i < n; i++)
    {
	this_p = atan2(sin(p[i]), cos(p[i]));
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
