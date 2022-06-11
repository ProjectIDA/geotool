/** \file FKCompute3C.cpp
 *  \brief Defines routine FKCompute3C
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "FKData.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "gobject++/GDataPoint.h"
#include "IIRFilter.h"
#include "libmath.h"
extern "C" {
#include "libstring.h"
}


#ifndef PI
#define PI 3.14159265358979323846
#endif

#define CLEAN_UP \
	if(windowed == 1) { \
	    for(i = 1; i < wvec.size(); i++) { \
		d1[i]->deleteObject(); d2[i]->deleteObject(); \
	    } \
	} \
	Free(f)

static int getOrientation(GTimeSeries *ts, double *hang, double *vang);
static void matprod(double *A, double *x, int nrow, int ncol, double *y);

void FKData::compute3C(gvector<Waveform *> &wvec, double tmin, double tmax,
			FKArgs fk_args)
{
    GDataWindow *dw, data_window;

    if(wvec.size() > 0) {
	gvector<Waveform *> ws;
	for(int i = 0; i < wvec.size(); i++) {
	    if( wvec[i]->tbeg() <= tmin &&
		wvec[i]->tend() >= tmax) ws.push_back(wvec[i]);
	}

	dw = ws[0]->dw;
	ws[0]->dw = &data_window;
	ws[0]->dw[0].d1 = ws[0]->lowerBound(tmin);
	ws[0]->dw[0].d2 = ws[0]->upperBound(tmax);

	compute3C(ws, 1, fk_args);

	delete ws[0]->dw[0].d1;
	delete ws[0]->dw[0].d2;
	ws[0]->dw = dw;
    }
}

/** Compute the 3C power grid.
 *  Takes three-component data and computes a power computation for an
 *  azimuth/incidence grid that resembles an FK computation.
 *  @param[in] wvec the three component waveforms as Waveform objects.
 *  @param[in] windows if 0, use the entire waveform. If 1, the dw[] members
 *      of the Waveform objects specify a data window to be used.
 *  @param[in] fk_args FK argument class with members:
 *	- output_power: if true, output power db instead of amplitude dB.
 *	- full_compute: if true, compute a single FK and save all frequencies.
 *	- three_component: if true, compute a three-component FK.
 *	- fine_grid: if true, compute a fine-grid.
 *	- slowness_max: (sec/km) the maximum slowness value of the FK grid.
 *	- num_slowness: the number of slowness values. (must be odd.)
 *	- num_bands: the number of frequency bands.
 *	- fmin: an array of minimum frequencies for the bands.
 *	- fmax: an array of maximum frequencies for the bands.
 *	- taper_type: the taper type
 *	- taper_beg: for the cosine taper, the percentage of the waveform to
 *		taper at the beginning.
 *	- taper_end: for the cosine taper, the percentage of the waveform to
 *		taper at the end.
 *	- signal_slow_min: (sec/km) the minimum slowness value for the signal
 *		max. Ignored if < 0.
 *	- signal_slow_max: (sec/km) the maximum slowness value for the signal
 *		max. Ignored if < 0.
 *	- signal_az_min: (degrees) the minimum azimuth value for the signal max.
 *		Ignored if < 0.
 *	- signal_az_max: (degrees) the maximum azimuth value for the signal max.
 *		Ignored if < 0.
 *  @throws GERROR_INVALID_ARGS if one of the conditions below is true:
 *	- wvec.size() != 3
 *	- num_bands <= 0 or num_bands > MAX_NBANDS
 *	- num_slowness <= 1
 *	- fmin[i] >= fmax[i]
 *	- there are no hang and vang values for the waveforms.
 *	- windows == 0 and a waveform has more that one segment (wvec[i]->size() > 1)
 *	- windows > 0 and the window starts and ends in different segments
 *	- the sample time intervals differ by more than GTimeSeries::tdelTolerance()
 *  @throws GERROR_MALLOC_ERROR
 */
void FKData::compute3C(gvector<Waveform *> &wvec, int windows, FKArgs fk_args)
{
	int	i, j, k, l, npts, i1, i2, b, imax;
	float	*f = NULL, *fptr[3], *fk_b;
	double	vang[3], hang[3];
	double	theta, phi, r, p_site[3][3], tlen, sum, sx, sy;
	double p[3], w[3], h[3][3], norm, delt, t0_min, t0, t, sum_coarse;
	double	dtor = PI / 180.;
	GDataPoint *d1[3];
	GDataPoint *d2[3];

	args = fk_args;

	if(args.num_bands <= 0) {
	    GError::setMessage("FKData.compute3C: num_bands = %d",
		args.num_bands);
	    throw(GERROR_INVALID_ARGS);
	}
	if(args.num_slowness <= 1) {
	    GError::setMessage("FKData.compute3C: num_slowness = %d",
			args.num_slowness);
	    throw(GERROR_INVALID_ARGS);
	}

	for(i = 0; i < args.num_bands; i++) {
	    if(args.fmin[i] >= args.fmax[i])
	    {
		GError::setMessage("FT: lo freq (%f) >= hi freq (%f)",
			args.fmin[i], args.fmax[i]);
		throw(GERROR_INVALID_ARGS);
	    }
	}
	if(wvec.size() != 3) {
	    GError::setMessage("FKData.compute3C: num_waveforms = %d",
			wvec.size());
	    throw(GERROR_INVALID_ARGS);
	}
	for(i = 0; i < 3; i++) {
	    if(!getOrientation(wvec[i]->ts, &hang[i], &vang[i])) {
		GError::setMessage("FKData.compute3C: no hang/vang for %s/%s",
			wvec[i]->sta(), wvec[i]->chan());
		throw(GERROR_INVALID_ARGS);
	    }
	}

	if(windows)
	{
	    if(windows > 1) {
		for(i = 0; i < 3; i++) {
		    d1[i] = wvec[i]->dw[0].d1;
		    d2[i] = wvec[i]->dw[0].d2;
		}
	    }
	    else {
		d1[0] = wvec[0]->dw[0].d1;
		d2[0] = wvec[0]->dw[0].d2;
		for(i = 1; i < 3; i++) {
		    d1[i] = wvec[i]->lowerBound(d1[0]->time());
		    d2[i] = wvec[i]->upperBound(d2[0]->time());
		}
	    }
	    for(i = 0; i < 3; i++) {
		if(d1[i]->segmentIndex() != d2[i]->segmentIndex()) {
		    GError::setMessage("Data gap: %s/%s",
			wvec[i]->sta(), wvec[i]->chan());
		    CLEAN_UP;
		    throw(GERROR_INVALID_ARGS);
		}
	    }
	    delt = d1[0]->segment()->tdel();
	    t0_min = d1[0]->time();
	    i1 = d1[0]->index();
	    i2 = d2[0]->index();
	    npts = i2 - i1 + 1;
	}
	else if(wvec[0]->size() > 1) {
	    GError::setMessage("Data gap: %s/%s",
			wvec[0]->sta(), wvec[0]->chan());
	    throw(GERROR_INVALID_ARGS);
	}
	else {
	    delt = wvec[0]->segment(0)->tdel();
	    t0_min = wvec[0]->tbeg();
	    i1 = 0;
	    i2 = wvec[0]->length()-1;
	    npts = wvec[0]->length();
	}
	tlen = delt*(npts-1);

	/* find the earliest beginning time of the waveform segments.
	 * check for constant delt and npts
	 */
	for(i = 1; i < 3; i++)
	{
	    double tdel_tol = wvec[0]->tdelTolerance();
	    double ddt = (wvec[i]->segment(0)->tdel() - delt)/delt;
	    if(ddt > tdel_tol) {
		GError::setMessage("Waveforms have different sample rates.");
		CLEAN_UP;
		throw(GERROR_INVALID_ARGS);
	    }
	    if(windows)
	    {
		t0 = d1[i]->time();
		i1 = d1[i]->index();
		i2 = d2[i]->index();
	    }
	    else if(wvec[i]->size() > 1) {
		GError::setMessage("Data gap: %s/%s",
				wvec[i]->sta(), wvec[i]->chan());
		CLEAN_UP;
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

	if(!(f = (float *)malloc(3*npts*sizeof(float)))) {
	    CLEAN_UP;
	    GError::setMessage("FKData.compute3C: malloc failed.");
	    throw(GERROR_MALLOC_ERROR);
	}
	for(i = 0; i < 3; i++) {
	    fptr[i] = f + i*npts;
	}

	windowed = windows;
	tbeg = t0_min;
	tend = t0_min + (npts-1)*delt;
	this->d_slowness = 2.*args.slowness_max/(double)(args.num_slowness-1);
	stringcpy(this->center_sta,wvec[0]->sta(),sizeof(this->center_sta));
	this->center_lat = wvec[0]->lat();
	this->center_lon = wvec[0]->lon();

	for(b = 0; b < args.num_bands; b++)
	{
	    if(!(fk[b] = (float *)malloc(
			args.num_slowness*args.num_slowness*sizeof(float))))
	    {
		CLEAN_UP;
		GError::setMessage("FKData: malloc failed.");
		throw(GERROR_INVALID_ARGS);
	    }
	}

	for(i = 0; i < 3; i++)
	{
	    phi = dtor * vang[i];
	    r = sin(phi);
	    theta = (- hang[i] + 90) * dtor;
	    p_site[i][0] = r * cos(theta);
	    p_site[i][1] = r * sin(theta);
	    p_site[i][2] = cos(phi);
	}

	for(b = 0; b < args.num_bands; b++)
	{
	    for(i = 0; i < 3; i++)
	    {
		if(windowed) {
		    memcpy(fptr[i], d1[i]->segment()->data+d1[i]->index(),
				npts*sizeof(float));
		}
		else {
		    memcpy(fptr[i], wvec[i]->segment(0)->data,
				npts*sizeof(float));
		}
		/* taper 10 points.
		 */
/*
		taper = 10.1/(double)npts;

		Taper_cosine(fptr[i], npts, taper, taper);
*/
		IIRFilter *iir = new IIRFilter(3, "BP", args.fmin[b],
				args.fmax[b], delt, 0);
		iir->applyMethod(fptr[i], npts, true);
		iir->deleteObject();

		/*
		if(hilbert && vang[i] > 45.0)
		{
		    Hilbert_data(npts, fptr[i]);
		}
		*/
	    }

	    total_power[b] = 0.;
	    for(j = 0; j < 3; j++)
	    {
		for(i = 0; i < npts; i++) {
		    total_power[b] += fptr[j][i] * fptr[j][i];
		}
	    }
	    total_power[b] /= tlen;

	    fk_b = fk[b];

	    for(j = 0, l = 0; j < args.num_slowness; j++)
	    {
		sy = -args.slowness_max + j*d_slowness;
		for(k = 0; k < args.num_slowness; k++, l++)
		{
		    sx = -args.slowness_max + k*d_slowness;

		    theta = ((sy == 0.0 && sx == 0.0) ? 0.0 : atan2(sy,sx))+PI;

		    if((r = sqrt(sx*sx + sy*sy)) <= 1.0)
		    {
			p[0] = r * cos(theta);
			p[1] = r * sin(theta);
			p[2] = cos(asin(r));

			matprod(&p_site[0][0], p, 3, 3, w);

			sum = 0.;
			for(i = 0; i < npts; i++)
			{
			    t = w[0] * fptr[0][i] +
				w[1] * fptr[1][i] +
				w[2] * fptr[2][i];
			    sum += t*t;
			}
			fk_b[l] = sum/tlen;
			/*
			norm = total_power - fkmap[l] + 1.e-05 * total_power;
			*/
			norm = total_power[b];
			fk_b[l] /= norm;
		    }
		    else
		    {
			fk_b[l] = 0;
		    }
		}
	    }
	}

	/*
	 * find max fk.
	 */
	sum_coarse = 0.;
	for(b = 0; b < args.num_bands; b++)
	{
	    int n = args.num_slowness * args.num_slowness;
	    fk_b = fk[b];
	    sum_coarse = fk_b[0];
	    imax = 0;
	    for(i = 1; i < n; i++)
	    {
		if(sum_coarse < fk_b[i]) {
		    imax = i;
		    sum_coarse = fk_b[i];
		}
	    }
	    fk_max[b] = sum_coarse;
/*	    for(i = 0; i < n; i++) fk_b[i] /= sum_coarse; */

	    jmax[b] = imax/args.num_slowness;
	    kmax[b] = imax - jmax[b]*args.num_slowness;
	    xmax[b] = -args.slowness_max + kmax[b]*d_slowness;
	    ymax[b] = -args.slowness_max + jmax[b]*d_slowness;
	}

	fine.n_slowfine = 11;

	for(b = 0; b < args.num_bands; b++)
	{
	    fine.n_fine[b] = 0;
	    fine.fk_fine[b] = NULL;

	    if( jmax[b] > 0 && jmax[b] < args.num_slowness-1 &&
		kmax[b] > 0 && kmax[b] < args.num_slowness-1 && fine.n_slowfine > 3)
	    {
		/* make a finer sx,sy grid centered at jmax,kmax
		 */
		fine.n_fine[b] = fine.n_slowfine*fine.n_slowfine;
		fine.fk_fine[b] = (float *)mallocWarn(
				fine.n_fine[b]*sizeof(float));
		fine.d_slowfine[b] = 2.*d_slowness/(fine.n_slowfine-1);
		fine.slowfine_ymin[b] = -args.slowness_max + (jmax[b]-1)*d_slowness;
		fine.slowfine_xmin[b] = -args.slowness_max + (kmax[b]-1)*d_slowness;
	
		for(j = 0, l = 0; j < fine.n_slowfine; j++)
		{
		    sy = fine.slowfine_ymin[b] + j*fine.d_slowfine[b];
		    for(k = 0; k < fine.n_slowfine; k++, l++)
		    {
			sx = fine.slowfine_xmin[b] + k*fine.d_slowfine[b];

			theta = ((sy==0.0 && sx==0.0) ? 0. : atan2(sy,sx)) + PI;

			if((r = sqrt(sx*sx + sy*sy)) <= 1.0)
			{
			    p[0] = r * cos(theta);
			    p[1] = r * sin(theta);
			    p[2] = cos(asin(r));

			    matprod(&p_site[0][0], p, 3, 3, w);

			    sum = 0.;
			    for(i = 0; i < npts; i++)
			    {
				t = w[0] * fptr[0][i] +
				    w[1] * fptr[1][i] +
				    w[2] * fptr[2][i];
				sum += t*t;
			    }
			    fine.fk_fine[b][l] = sum/tlen;
			    /*
			    norm = total_power - fkmap[l] + 1.e-05*total_power;
			    */
			    norm = total_power[b];
			    fine.fk_fine[b][l] /= norm;
			}
			else
			{
			    fine.fk_fine[b][l] = 0;
			}
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
		if(sum > sum_coarse) {
		    fk_max[b] = sum;
		}
/*
		if(sum > sum_coarse) {
		    fk_max[b] = sum_coarse;
		    sum_coarse /= sum;
		    n = n_slowness*n_slowness;
		    for(i = 0; i < n; i++) fk[b][i] *= sum_coarse;
		}
		else {
		    sum = sum_coarse;
		}
		for(i = 0; i < fine.n_fine[b]; i++) fk_b[i] /= sum;
*/

		jmax[b] = imax/fine.n_slowfine;
		kmax[b] = imax - jmax[b]*fine.n_slowfine;

		/* use parabolic fit to refine the max x,y
		 */
		
		if( jmax[b] > 0 && jmax[b] < fine.n_slowfine-1 &&
		    kmax[b] > 0 && kmax[b] < fine.n_slowfine-1)
		{
		    double x0, y0;
		    for(j = 0; j < 3; j++)
			for(k = 0; k < 3; k++)
		    {
			h[k][j] = fk_b[(jmax[b]-1+j)*fine.n_slowfine +
					kmax[b]-1+k];
		    }
		    fit2d(h, &x0, &y0);
		    xmax[b] = fine.slowfine_xmin[b]
					+ kmax[b]*fine.d_slowfine[b]
					+ x0*fine.d_slowfine[b];
		    ymax[b] = fine.slowfine_ymin[b]
					+ jmax[b]*fine.d_slowfine[b]
					+ y0*fine.d_slowfine[b];
		}
		else {
		    xmax[b] = fine.slowfine_xmin[b]
					+ kmax[b]*fine.d_slowfine[b];
		    ymax[b] = fine.slowfine_ymin[b]
					+ jmax[b]*fine.d_slowfine[b];
		}
	    }
	}

        CLEAN_UP;
}

static int
getOrientation(GTimeSeries *ts, double *hang, double *vang)
{
    *hang = ts->hang();
    *vang = ts->vang();
    if(*hang > -900. && *vang > -900.) return 1;
    return 0;
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

