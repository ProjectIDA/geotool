/** \file IIRFilter.cpp
 *  \brief Defines class IIRFilter.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>
#include <math.h>

#include "IIRFilter.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libstring.h"
}

using namespace std;

/* These are the comments from the original fortran code.
 * NAME
 *        iirdes -- (filters) design iir digital filters from analog prototypes
 *
 *        Copyright 1988  regents of the university of california
 *
 *
 * SYNOPSIS
 *        call iirdes( iord, type, fl, fh, ts, sn, sd, nsects )
 *
 *
 *        integer iord     (i)    filter order (10 maximum)
 *        character*2 type (i)    filter type
 *                                lowpass (LP)
 *                                highpass (HP)
 *                                bandpass (BP)
 *                                bandreject (BR)
 *        real fl          (i)    low-frequency cutoff
 *        real fh          (i)    high-frequency cutoff
 *        real ts          (i)    sampling interval (in seconds)
 *        real sn(*)       (o)    array containing numerator coefficients of
 *                                second-order sections packed head-to-tail.
 *        real sd(*)       (o)    array containing denominator coefficients
 *                                of second-order sections packed head-to-tail.
 *        integer nsects   (o)    number of second-order sections.
 *
 * DESCRIPTION
 *        Subroutine to design iir digital filters from analog prototypes.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 *
 *	(415) 423-0617
 *	Translated from Fortran to C by:
 *	C. S. Lynnes
 *	Teledyne Geotech
 *	314 Montgomery
 *	Alexandria, VA  22314
 *	(703) 739-7316
 */

/** Construct an IIR filter object from a string description. The string
 *  description should contain the parameter names and values in the form
 *  "name1=value1 name2=value". The order of the name/value pairs is
 *  arbitrary. Example:
 *  \code
       "type=BP order=5 flow=1. fhigh=3. zero_phase=0"
 *  \endcode
 *  @param[in] time_interval the sample time interval.
 *  @param[in] s the string filter description.
 *  @throws GERROR_INVALID_ARGS and GERROR_MALLOC_ERROR
 */
IIRFilter::IIRFilter(double time_interval, const string &s) throw(int)
			: DataMethod("IIRFilter"),
	order(0), flow(0.), fhigh(0.), tdel(0.), zero_phase(0), nsects(0),
	nps(0), sn(NULL), sd(NULL), x1(NULL), x2(NULL), y1(NULL), y2(NULL)
{
    init(time_interval, s);
}

/** Construct an IIR filter object from a string description. The string
 *  description should contain the parameter names and values in the form
 *  "name1=value1 name2=value". The order of the name/value pairs is
 *  arbitrary. Example:
 *  \code
       "type=BP order=5 flow=1. fhigh=3. zero_phase=0"
 *  \endcode
 *  @param[in] ts a GTimeSeries object from which the sample time interval is
 *   obtained.
 *  @param[in] s the string filter description.
 *  @throws GERROR_INVALID_ARGS and GERROR_MALLOC_ERROR
 */
IIRFilter::IIRFilter(GTimeSeries *ts, const string &s) throw(int)
		: DataMethod("IIRFilter"),
	order(0), flow(0.), fhigh(0.), tdel(0.), zero_phase(0), nsects(0),
	nps(0), sn(NULL), sd(NULL), x1(NULL), x2(NULL), y1(NULL), y2(NULL)
{
    double t_del;

    if(!ts || ts->size() <= 0) {
	GError::setMessage("IIRFilter: invalid ts.");
	throw GERROR_INVALID_ARGS;
    }
    t_del = ts->segment(0)->tdel();
    init(t_del, s);
}

/** Constructor.
 *  @param[in] iord the filter order (0 to 10)
 *  @param[in] ftype the filter type: "BP", "BR", "LP", "HP
 *  @param[in] fl   the low frequency cut.
 *  @param[in] fh   the high frequency cut.
 *  @param[in] time_interval the sample time interval in seconds.
 *  @param[in] zp 1 for a zero phase filter, 0 for a causal filter.
 *  @throws GERROR_INVALID_ARGS and GERROR_MALLOC_ERROR
 */
IIRFilter::IIRFilter(int iord, const string &ftype, double fl, double fh,
		double time_interval, int zp) throw(int)
		: DataMethod("IIRFilter"),
	order(iord), flow(fl), fhigh(fh), tdel(time_interval), zero_phase(zp),
	nsects(0), nps(0), sn(NULL), sd(NULL), x1(NULL), x2(NULL), y1(NULL),
	y2(NULL)
{
    init(iord, ftype.c_str(), fl, fh, time_interval, zp);
}

IIRFilter::IIRFilter(const IIRFilter &f) : DataMethod(f),
	order(f.order), flow(f.flow), fhigh(f.fhigh), tdel(f.tdel),
	zero_phase(f.zero_phase), nsects(f.nsects), nps(f.nps),
	sn(NULL), sd(NULL), x1(NULL), x2(NULL), y1(NULL), y2(NULL)
{
    sn = (double *)malloc(nps*sizeof(double));
    sd = (double *)malloc(nps*sizeof(double));
    memcpy(sn, f.sn, nps*sizeof(double));
    memcpy(sd, f.sd, nps*sizeof(double));

    x1 = (double *)malloc(nsects*sizeof(double));
    x2 = (double *)malloc(nsects*sizeof(double));
    y1 = (double *)malloc(nsects*sizeof(double));
    y2 = (double *)malloc(nsects*sizeof(double));
    memcpy(x1, f.x1, nsects*sizeof(double));
    memcpy(x2, f.x2, nsects*sizeof(double));
    memcpy(y1, f.y1, nsects*sizeof(double));
    memcpy(y2, f.y2, nsects*sizeof(double));
}

IIRFilter & IIRFilter::operator=(const IIRFilter &f)
{
    if(this == &f) return *this;

    free(sn);
    free(sd);
    free(x1);
    free(x2);
    free(y1);
    free(y2);

    order = f.order;
    flow = f.flow;
    fhigh = f.fhigh;
    tdel = f.tdel;
    zero_phase = f.zero_phase;
    nsects = f.nsects;
    nps = f.nps;
    sn = (double *)malloc(nps*sizeof(double));
    sd = (double *)malloc(nps*sizeof(double));
    memcpy(sn, f.sn, nps*sizeof(double));
    memcpy(sd, f.sd, nps*sizeof(double));

    x1 = (double *)malloc(nsects*sizeof(double));
    x2 = (double *)malloc(nsects*sizeof(double));
    y1 = (double *)malloc(nsects*sizeof(double));
    y2 = (double *)malloc(nsects*sizeof(double));
    memcpy(x1, f.x1, nsects*sizeof(double));
    memcpy(x2, f.x2, nsects*sizeof(double));
    memcpy(y1, f.y1, nsects*sizeof(double));
    memcpy(y2, f.y2, nsects*sizeof(double));
    return *this;
}

Gobject * IIRFilter::clone(void)
{
    IIRFilter *iir = new IIRFilter(order, type, flow, fhigh, tdel, zero_phase);
    memcpy(iir->x1, x1, nsects*sizeof(double));
    memcpy(iir->x2, x2, nsects*sizeof(double));
    memcpy(iir->y1, y1, nsects*sizeof(double));
    memcpy(iir->y2, y2, nsects*sizeof(double));
    return (Gobject *) iir;
}

/** Initialize an IIR filter from a string representation. The string
 *  description should contain the parameter names and values in the form
 *  "name1=value1 name2=value". The order of the name/value pairs is
 *  arbitrary. Example:
 *  \code
       "type=BP order=5 flow=1. fhigh=3. zero_phase=0"
 *  \endcode
 *  @param[in] time_interval the sample time interval.
 *  @param[in] str the string filter description.
 *  @throws GERROR_MALLOC_ERROR
 */
void IIRFilter::init(double time_interval, const string &str) throw(int)
{
    char *c, ftype[3];
    int  iord, zp;
    double fl, fh;

    if((c = stringGetArg(str.c_str(), "type")) != NULL) {
	if(strlen(c) != 2) {
	    GError::setMessage("IIRFilter: invalid type");
	    free(c);
	    throw GERROR_INVALID_ARGS;
	}
	strncpy(ftype, c, 3);
	free(c);
    }
    if(stringGetIntArg(str.c_str(), "order", &iord)) {
	GError::setMessage("IIRFilter: invalid order");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetDoubleArg(str.c_str(), "flow", &fl)) {
	GError::setMessage("IIRFilter: invalid flow");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetDoubleArg(str.c_str(), "fhigh", &fh)) {
	GError::setMessage("IIRFilter: invalid fhigh");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetIntArg(str.c_str(), "zero_phase", &zp)) {
	GError::setMessage("IIRFilter: invalid zero_phase");
	throw GERROR_INVALID_ARGS;
    }
    init(iord, ftype, fl, fh, time_interval, zp);
}

/** 
 *  Initialize an IIR filter with the specified parameters.
 *  @param[in] iord the filter order (0 to 10)
 *  @param[in] ftype the filter type: "BP", "BR", "LP", "HP
 *  @param[in] fl   the low frequency cut.
 *  @param[in] fh   the high frequency cut.
 *  @param[in] time_interval the sample time interval in seconds.
 *  @param[in] zp 1 for a zero phase filter, 0 for a causal filter.
 *  @throws GERROR_MALLOC_ERROR
 */
void IIRFilter::init(int iord, const char *ftype, double fl, double fh,
			double time_interval, int zp)
{
    double	flw;
    double	fhw;
    Cmplx	p[10];
    char	ptype[10];

    order = iord;
    strncpy(type, ftype, 2);
    type[2] = '\0';
    flow = fl;
    fhigh = fh;
    tdel = time_interval;
    zero_phase = zp;

    if(!strcasecmp(type, "NA")) { /* No filter */
	nsects = 0;
	sn = NULL;
	sd = NULL;
	x1 = NULL;
	x2 = NULL;
	y1 = NULL;
	y2 = NULL;
	return;
    }

    nps = butterPoles(p, ptype, iord);

    sn = (double *)malloc(nps*6*sizeof(double));
    if(!sn) {
	GError::setMessage("IIRFilter.init malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
    sd = (double *)malloc(nps*6*sizeof(double));
    if(!sd) {
	free(sn);
	GError::setMessage("IIRFilter.init malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    if(!strcasecmp(type, "BP")) /* BAND_PASS */
    {
	fl = fl*tdel/2.;
	fh = fh*tdel/2.;
	flw = tangent_warp( fl, 2. );
	fhw = tangent_warp( fh, 2. );
	LPtoBP(p, ptype, nps, flw, fhw);
    }
    else if(!strcasecmp(type, "BR")) /* BAND_REJECT */
    {
	fl = fl*tdel/2.;
	fh = fh*tdel/2.;
	flw = tangent_warp( fl, 2. );
	fhw = tangent_warp( fh, 2. );
	LPtoBR(p, ptype, nps, flw, fhw);
    }
    else if(!strcasecmp(type, "LP")) /* LOW_PASS */
    {
	fh = fh*tdel/2.;
	fhw = tangent_warp( fh, 2. );
	lowpass(p, ptype, nps);
	cutoffAlter(fhw);
    }
    else if(!strcasecmp(type, "HP")) /* HIGH_PASS */
    {
	fl = fl*tdel/2.;
	flw = tangent_warp( fl, 2. );
	LPtoHP(p, ptype, nps);
	cutoffAlter(flw);
    }
    bilinear();

    x1 = (double *)malloc(nsects*sizeof(double));
    x2 = (double *)malloc(nsects*sizeof(double));
    y1 = (double *)malloc(nsects*sizeof(double));
    y2 = (double *)malloc(nsects*sizeof(double));
    if(!x1 || !x2 || !y1 || !y2) {
	free(sn);
	if(x1) free(x1);
	if(x2) free(x2);
	if(y1) free(y1);
	if(y2) free(y2);
	GError::setMessage("IIRFilter.init malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    Reset();
}

/** Destructor. */
IIRFilter::~IIRFilter(void)
{
    free(sn);
    free(sd);
    free(x1);
    free(x2);
    free(y1);
    free(y2);
}

/* These are the comments from the original fortran code.
 * NAME
 *         apiir  -- (filters) apply iir filter to a data sequence
 * 
 *         copyright 1988  regents of the university of california
 * 
 *  SYNOPSIS
 *         call apiir( data, nsamps, zp, sn, sd, nsects )
 *         real data       (i/o) array containing data
 *         integer nsamps  (i) number of data samples
 *         logical zp      (i) true for zero phase filtering,
 *                             false for single pass filtering
 *         real sn         (i) numerator polynomials for second
 *                             order sections.
 *         real sd         (i) denominator polynomials for second
 *                             order sections.
 *         integer nsects  (i) number of second-order sections
 * 
 *  DESCRIPTION
 *         Subroutine to apply an iir filter to a data sequence.
 *         The filter is assumed to be stored as second order sections.
 *         Filtering is in-place.
 *         Zero-phase (forward and reverse) is an option.
 * 
 *  WARNINGS
 *         Zero-phase filtering doubles the falloff rate outside of
 *         the band pass; number of poles is effectively doubled.
 * 
 *  AUTHOR
 *         Dave Harris
 *         Lawrence Livermore National Laboratory
 *         L-205
 *         P.O. Box 808
 *         Livermore, CA  94550
 *         (415) 423-0617
 */

bool IIRFilter::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "IIRFilter.apply: ts=NULL.");
	return false;
    }

    for(int i = 0; i < n; i++) {
	applyMethod(ts[i]);
    }
    return true;
}

const char * IIRFilter::toString(void)
{
    char c[100];
    snprintf(c, sizeof(c),
	"IIRFilter: type=%s order=%d flow=%.2f fhigh=%.2f zero_phase=%d",
		type, order, flow, fhigh, zero_phase);
    string_rep.assign(c);
    return string_rep.c_str();
}

/** 
 *  Apply the filter to a GTimeSeries object.
 *  @param[in] ts The GTimeSeries object to be filtered.
 */
void IIRFilter::applyMethod(GTimeSeries *ts)
{
    double tol;
    
    if(nsects == 0) { /* type = "NA", No filter */
	return;
    }

    for(int i = 0; i < ts->size(); i++) {
	tol = .001*ts->segment(i)->tdel();
	applyMethod(ts->segment(i), !ts->continuous(i, tol, tol));
    }
}

/** Filter a GSegment object.
 *  @param[in] s The GSegment object to be filtered.
 *  @param[in] reset if true, reset the coefficients of the recursive algorithm,
 *	otherwise continue with the last coefficients created.
 */
void IIRFilter::applyMethod(GSegment *s, bool reset)
{
    applyMethod(s->data, s->length(), reset);
}

/** Filter a float array.
 *  @param[in] data The float data to be filtered.
 *  @param[in] data_length the length of data[].
 *  @param[in] reset Reset the coefficents of the recursive algorithm. 
 */
bool IIRFilter::applyMethod(float *data, int data_length, bool reset)
{
    if(data_length <= 0) return true;
    if(data == NULL) return false;

    if(reset) Reset();

    applyFilter(data, data_length, nsects, sn, sd, x1, x2, y1, y2);
    if(zero_phase) reverse(data, data_length, 1);
    return true;
}

/** Apply filter to a float array.
 *  @param[in] data The float data to be filtered.
 *  @param[in] data_length the length of data[].
 *  @param[in] num_sections the number of second-order sections.
 *  @param[in] numerator numerator polynomials for second order sections.
 *  @param[in] denominator denominator polynomials for second order sections.
 *  @param[in,out] x1_coef num_sections coefficients needed for the recursive
 *		filter
 *  @param[in,out] x2_coef num_sections coefficients needed for the recursive
 *		filter
 *  @param[in,out] y1_coef num_sections coefficients needed for the recursive
 *		filter
 *  @param[in,out] y2_coef num_sections coefficients needed for the recursive
 *		filter
 */
void IIRFilter::
applyFilter(float *data, int data_length, int num_sections, double *numerator,
		double *denominator, double *x1_coef, double *x2_coef,
		double *y1_coef, double *y2_coef)
{
    int	i, j, jptr;
    double input, output;

    for(i = 0; i < data_length; i++) {
	jptr = 0;
	input = (double)data[i];
	output = input;
	for(j = 0; j < nsects; j++) {
	    output = numerator[jptr] * input
			+ numerator[jptr+1] * x1_coef[j]
			+ numerator[jptr+2] * x2_coef[j]
			- ( denominator[jptr+1] * y1_coef[j]
				+ denominator[jptr+2] * y2[j] );
	    y2_coef[j] = y1_coef[j];
	    y1_coef[j] = output;
	    x2_coef[j] = x1_coef[j];
	    x1_coef[j] = input;

	    jptr += 3;
	    input = output;
	}
	data[i] = (float)output;
    }
}

/** Do the reverse filter for zero-phase filters.
 *  @param[in] data a float array of length data_length.
 *  @param[in] data_length
 *  @param[in] reset = true to reset the recursive filter coefficients,
 *  = false to continue with the last coefficients.
 */
void IIRFilter::reverse(float *data, int data_length, bool reset)
{
    if(reset) Reset();
    doReverse(data, data_length, nsects, sn, sd, x1, x2, y1, y2);
}

/** Reverse Filter a float array for zero phase filtering.
 *  @param[in] data The float data to be filtered.
 *  @param[in] data_length the length of data[].
 *  @param[in] num_sections the number of second-order sections.
 *  @param[in] numerator numerator polynomials for second order sections.
 *  @param[in] denominator denominator polynomials for second order sections.
 *  @param[in,out] x1_coef num_sections coefficients needed for the recursive
 *		filter
 *  @param[in,out] x2_coef num_sections coefficients needed for the recursive
 *		filter
 *  @param[in,out] y1_coef num_sections coefficients needed for the recursive
 *		filter
 *  @param[in,out] y2_coef num_sections coefficients needed for the recursive
 *		filter
 */
void IIRFilter::
doReverse(float *data, int data_length, int num_sections, double *numerator,
		double *denominator, double *x1_coef, double *x2_coef,
		double *y1_coef, double *y2_coef)
{
    int jptr, ir;
    double input, output;

    for(int i = 0; i < data_length; i++) {
	jptr = 0;
	ir = data_length - 1 - i;
	input = (double)data[ir];
	output = input;
	for(int j = 0; j < num_sections; j++) {
	    output = numerator[jptr] * input
			+ numerator[jptr+1] * x1_coef[j]
			+ numerator[jptr+2] * x2_coef[j]
			- ( denominator[jptr+1] * y1[j]
				+ denominator[jptr+2] * y2_coef[j] );
	    y2_coef[j] = y1_coef[j];
	    y1_coef[j] = output;
	    x2_coef[j] = x1_coef[j];
	    x1_coef[j] = input;

	    jptr += 3;
	    input = output;
	}
	data[ir] = (float)output;
    }
}

/** Reset the recursive filter coefficients to zero.
 */
void IIRFilter::Reset()
{
    for(int i = 0; i < nsects; i++) {
	x1[i] = x2[i] = y1[i] = y2[i] = 0.;
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        bilin2  -- (filters) transforms an analog filter to a digital
 *        filter via the bilinear transformation. Assumes both are stored
 *        as second order sections.  The transformation is done in-place.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call bilin2( sn, sd, nsects )
 *        real sn(*) (i) Array containing numerator polynomial 
 *                       coefficients for second order sections.
 *                       Packed head-to-tail.
 *        real sd(*) (i) Array containing denominator polynomial 
 *                       coefficients for second order sections. 
 *                       Packed head-to-tail.
 *        integer nsects  (i) Number of second order sections.
 *
 * DESCRIPTION
 *        Transforms an analog filter to a digital filter via the bilinear
 *        transformation. Assumes both are stored as second order sections.  
 *        The transformation is done in-place.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *
 */
    
/** Transforms an analog filter to a digital filter via the bilinear
 *  transformation. Assumes both are stored as second order sections.
 *  The transformation is done in-place.
 */
void IIRFilter::bilinear(void)
{
    int i, iptr;
    double a0, a1, a2, scale;
    
    for(i = 0, iptr = 0; i < nsects; i++)
    {
	a0 = sd[iptr];
	a1 = sd[iptr+1];
	a2 = sd[iptr+2];
	scale = a2 + a1 + a0;
	sd[iptr]   = 1.;
	sd[iptr+1] = (2.*(a0 - a2)) / scale;
	sd[iptr+2] = (a2 - a1 + a0) / scale;
	a0 = sn[iptr];
	a1 = sn[iptr+1];
	a2 = sn[iptr+2];
	sn[iptr]   = (a2 + a1 + a0) / scale;
	sn[iptr+1] = (2.*(a0 - a2)) / scale;
	sn[iptr+2] = (a2 - a1 + a0) / scale;
	iptr = iptr + 3;
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        bupoles -- (filters) compute butterworth poles for lowpass filter
 *
 *        Copyright 1988  regents of the university of california
 *
 *
 * SYNOPSIS
 *        call  bupoles( p, type, n, iord )
 *        real p  (o) complex array containing poles contains only one 
 *                    from each complex conjugate pair, and all real poles
 *      character*1(1) type (o) character array indicating pole type:
 *                                'S' -- single real
 *                                'C' -- complex conjugate pair
 *        integer n  (o) number of second order sections
 *        iord       (i) desired number of poles
 *
 * DESCRIPTION
 *        bupoles -- subroutine to compute butterworth poles for
 *        normalized lowpass filter
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 * last modified:  august 4, 1988
 */

/** Compute butterworth poles for lowpass filter.
 *  @param[in] p complex array containing poles contains only one from each
 *	complex conjugate pair, and all real poles
 *  @param[in] ftype character array indicating pole type: 'S' for a single
 *	real, 'C' for a complex conjugate pair
 *  @param[in] iord desired number of poles
 */
int IIRFilter::
butterPoles(Cmplx *p, char *ptype, int iord)
{
    double angle;
    int half, k, n;

    half = iord/2;

    /* test for odd order, and add pole at -1 */

    n = 0;
    if ( 2*half < iord ) {
	p[0] = cmplx( -1., 0. );
	ptype[0] = 'S';
	n = 1;
    }
    for(k = 0; k < half; k++) {
	angle = M_PI * ( .5 + (double)(2*(k+1)-1) / (double)(2*iord) );
	p[n] = cmplx( cos(angle), sin(angle) );
	ptype[n] = 'C';
	n++;
    }
    return(n);
}

/* These are the comments from the original fortran code.
 * NAME
 *        cutoffs -- (filters) Subroutine to alter the cutoff of a filter.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call cutoffs( sn, sd, ns, f )
 *
 *        real sn   (i/o) numerator polynomials for second order sections.
 *        real sd   (i/o) denominator polynomials for second order sections.
 *        integer ns (i/o) number of second-order sections
 *        real f    (i)   new cutoff frequency
 *
 * DESCRIPTION
 *        Subroutine to alter the cutoff of a filter. Assumes that the
 *        filter is structured as second order sections.  Changes
 *        the cutoffs of normalized lowpass and highpass filters through
 *        a simple polynomial transformation.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 */

/** Function to alter the cutoff of a filter. Changes the cutoffs of normalized
 *  lowpass and highpass filters through a simple polynomial transformation.
 *  @param[in] f new cutoff frequency
 */
void IIRFilter::cutoffAlter(double f)
{
    int i, iptr;
    double scale;
    
    scale = 2.*M_PI*f;
    for(i = 0, iptr = 0; i < nsects; i++) {
	sn[ iptr + 1 ] = sn[ iptr + 1 ] / scale;
	sn[ iptr + 2 ] = sn[ iptr + 2 ] / (scale*scale);
	sd[ iptr + 1 ] = sd[ iptr + 1 ] / scale;
	sd[ iptr + 2 ] = sd[ iptr + 2 ] / (scale*scale);
	iptr += 3;
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        lpa -- (filters) generate second order sections from all-pole
 *        description for lowpass filters.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call lpa( p, ptype, np, sn, sd, ns )
 *        real p(*)            (i) array containing poles
 *        character*1 ptype(1) (i) character array containing type
 *                                    information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second order sections
 *
 * DESCRIPTION
 *
 *        Subroutine to generate second order section parameterization
 *        from an all-pole description for lowpass filters.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */
    
/** Generate second order sections from all-pole description for lowpass
 *  filters. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 */
void IIRFilter::lowpass(Cmplx *p, const char *ptype, int np)
{
    int i, iptr;

    nsects = 0;
    for(i = 0, iptr = 0; i < np; i++) {
	if(ptype[i] == 'C') {
	    sn[iptr]     = 1.;
	    sn[iptr + 1] = 0.;
	    sn[iptr + 2] = 0.;
	    sd[iptr] = real_part(cmplx_mul(p[i], cmplx_conjg(p[i])));
	    sd[iptr + 1] = -2. * real_part( p[i] );
	    sd[iptr + 2] = 1.;
	    iptr += 3;
	    nsects++;
	}
	else if(ptype[i] == 'S') {
	    sn[ iptr ]     = 1.;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 0.;
	    sd[ iptr ]     = -real_part( p[i] );
	    sd[ iptr + 1 ] = 1.;
	    sd[ iptr + 2 ] = 0.;
	    iptr += 3;
	    nsects++;
	}
    }
}
/* These are the comments from the original fortran code.
 * NAME
 *        lptbpa -- (filters)  convert all-pole lowpass to bandpass filter
 *        via the analog polynomial transformation.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call lptbpa( p, ptype, np, fl, fh, sn, sd, ns )
 *        real p(*)            (i)    array containing poles
 *        character*1(1) ptype (i)    array containing type information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real fl     (i) low-frequency cutoff
 *        real fh     (i) high-frequency cutoff
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second-order sections
 *
 * DESCRIPTION
 *
 *        Subroutine to convert an all-pole lowpass filter to a bandpass
 *        filter via the analog polynomial transformation. The lowpass
 *        filter is described in terms of its poles (as input to this
 *        routine). the output consists of the parameters for second order
 *        sections.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */

/** Convert all-pole lowpass to bandpass filter via the analog polynomial
 *  transformation. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 *  @param[in] fl low-frequency cutoff
 *  @param[in] fh high-frequency cutoff
 */
void IIRFilter::LPtoBP(Cmplx *p, const char *ptype, int np, double fl,
				double fh)
{
    Cmplx ctemp, p1, p2;
    double  twopi, a, b;
    int     i, iptr;

    twopi = 2.*M_PI;
    a = twopi*twopi*fl*fh;
    b = twopi*( fh - fl );
    nsects = 0;
    iptr = 0;
    for(i = 0; i < np; i++)
    {
	if(ptype[i] == 'C')
	{
	    ctemp = real_cmplx_mul(b, p[i]);
	    ctemp = cmplx_mul(ctemp, ctemp);
	    ctemp = cmplx_sub(ctemp, cmplx(4.*a, 0.));
	    ctemp = cmplx_sqrt( ctemp );
	    p1 = real_cmplx_mul(0.5, cmplx_add(real_cmplx_mul(b,p[i]), ctemp));
	    p2 = real_cmplx_mul(0.5, cmplx_sub(real_cmplx_mul(b,p[i]), ctemp));
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = b;
	    sn[ iptr + 2 ] = 0.;
	    sd[iptr] = real_part(cmplx_mul(p1, cmplx_conjg(p1)));
	    sd[ iptr + 1 ] = -2. * real_part( p1 );
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = b;
	    sn[ iptr + 2 ] = 0.;
	    sd[iptr] = real_part(cmplx_mul(p2, cmplx_conjg(p2)));
	    sd[ iptr + 1 ] = -2. * real_part( p2 );
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    nsects += 2;
	}
	if(ptype[i] == 'S')
	{
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = b;
	    sn[ iptr + 2 ] = 0.;
	    sd[ iptr ]     = a;
	    sd[ iptr + 1 ] = -b*real_part( p[i] );
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    nsects++;
	}
    }
}
/* These are the comments from the original fortran code.
 * NAME
 *        lptbra -- (filters)  convert all-pole lowpass to band reject
 *        filter via the analog polynomial transformation.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *
 *        call lptbra( p, ptype, np, fl, fh, sn, sd, ns )
 *        real p(*)            (i) array containing poles
 *        character*1(1) ptype (i) array containing type information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real fl     (i) low-frequency cutoff
 *        real fh     (i) high-frequency cutoff
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second-order sections
 *
 * DESCRIPTION
 *
 *        Subroutine to convert an all-pole lowpass filter to a band
 *        reject filter via the analog polynomial transformation. The
 *        lowpass filter is described in terms of its poles (as input to
 *        this routine). the output consists of the parameters for second
 *        order sections.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */
    
/** Convert all-pole lowpass to band reject filter via the analog polynomial
 *  transformation. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 *  @param[in] fl low-frequency cutoff
 *  @param[in] fh high-frequency cutoff
 */
void IIRFilter::LPtoBR(Cmplx *p, const char *ptype, int np, double fl,
			double fh)
{
    Cmplx pinv, ctemp, p1, p2;
    double  twopi, a, b;
    int     i, iptr;
    
    twopi = 2.*M_PI;
    a = twopi*twopi*fl*fh;
    b = twopi*( fh - fl );
    nsects = 0;
    iptr = 0;
    for(i = 0; i < np; i++)
    {
	if(ptype[i] == 'C')
	{
	    pinv = cmplx_div(cmplx(1.,0.), p[i]);
	    ctemp = cmplx_mul(real_cmplx_mul(b,pinv), real_cmplx_mul(b, pinv));
	    ctemp = cmplx_sub(ctemp, cmplx(4.*a, 0.));
	    ctemp = cmplx_sqrt( ctemp );
	    p1 = real_cmplx_mul(0.5, cmplx_add(real_cmplx_mul(b,pinv), ctemp));
	    p2 = real_cmplx_mul(0.5, cmplx_sub(real_cmplx_mul(b,pinv), ctemp));
	    sn[ iptr ]     = a;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[iptr] = real_part(cmplx_mul(p1, cmplx_conjg(p1)));
	    sd[ iptr + 1 ] = -2. * real_part(p1);
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    sn[ iptr ]     = a;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[iptr] = real_part(cmplx_mul(p2, cmplx_conjg(p2)));
	    sd[ iptr + 1 ] = -2. * real_part(p2);
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    nsects += 2;
	}
	else if(ptype[i] == 'S')
	{
	    sn[ iptr ]     = a;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[ iptr ]     = -a*real_part( p[i] );
	    sd[ iptr + 1 ] = b;
	    sd[ iptr + 2 ] = -real_part( p[i] );
	    iptr = iptr + 3;
	    nsects++;
	}
    }
}
/* These are the comments from the original fortran code.
 * NAME
 *        lpthpa -- (filters)  convert all-pole lowpass to high pass
 *        filter via the analog polynomial transformation.
 *
 *        Copyright 1988  regents of the university of california
 *
 *
 * SYNOPSIS
 *        call lpthpa( p, ptype, np, fl, fh, sn, sd, ns )
 *        real p(*)            (i) array containing poles
 *        character*1(1) ptype (i) array containing type information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real fl     (i) low-frequency cutoff
 *        real fh     (i) high-frequency cutoff
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second-order sections
 *
 * DESCRIPTION
 *        Subroutine to convert an all-pole lowpass filter to a high pass
 *        filter via the analog polynomial transformation. The lowpass
 *        filter is described in terms of its poles (as input to this
 *        routine). the output consists of the parameters for second
 *        order sections.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */
    
/** Convert all-pole lowpass to high pass filter via the analog polynomial
 *  transformation. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 */
void IIRFilter::LPtoHP(Cmplx *p, const char *ptype, int np)
{
    int i, iptr;

    nsects = 0;
    for(i = 0, iptr = 0; i < np; i++) {
	if(ptype[i] == 'C') {
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[ iptr ]     = 1.;
	    sd[ iptr + 1 ] = -2. * real_part( p[i] );
	    sd[ iptr + 2 ] = real_part(cmplx_mul(p[i], cmplx_conjg(p[i])));
	    iptr += 3;
	    nsects++;
	}
	else if(ptype[i] == 'S') {
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = 1.;
	    sn[ iptr + 2 ] = 0.;
	    sd[ iptr ]     = 1.;
	    sd[ iptr + 1 ] = -real_part( p[i] );
	    sd[ iptr + 2 ] = 0.;
	    iptr += 3;
	    nsects++;
	}
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        warp -- (filters) function, applies tangent frequency warping
 *        to compensate for bilinear analog -> digital transformation
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        real function warp(f,t)
 *
 *                f  original design frequency specification (hertz)
 *                t  sampling interval
 *
 *
 * DESCRIPTION
 *
 *        warp -- function, applies tangent frequency warping to
 *                compensate for bilinear analog -> digital transformation
 *
 * DIAGNOSTICS
 *        none
 *
 * RESTRICTIONS
 *        unknown.
 *
 * BUGS
 *        unknown.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */

/** Applies tangent frequency warping to compensate for bilinear analog to
 *  digital transformation.
 *  @param[in] f original design frequency specification (hertz)
 *  @param[in] t sampling interval
 */
double IIRFilter::
tangent_warp(double f, double t)
{
    double twopi;
    double angle;
    double warp;
    double fac;
    
    twopi = 2.*M_PI;
    fac = .5*f*t;
    if(fac >= .25) fac = .2499999;
    angle = fac*twopi;
    warp = 2.*tan(angle)/t;
    warp = warp/twopi;
    return(warp);
}

/** Complex addition.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 + c2
 */
Cmplx IIRFilter::cmplx_add(Cmplx c1, Cmplx c2)
{
    Cmplx	csum;
    csum.r = c1.r + c2.r;
    csum.i = c1.i + c2.i;
    return (csum);
}
/** Complex subtraction.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 - c2
 */
Cmplx IIRFilter::cmplx_sub(Cmplx c1, Cmplx c2)
{
    Cmplx	csum;
    csum.r = c1.r - c2.r;
    csum.i = c1.i - c2.i;
    return (csum);
}
/** Complex multiplication.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 * c2
 */
Cmplx IIRFilter::cmplx_mul(Cmplx c1, Cmplx c2)
{
    Cmplx	c;
    c.r = c1.r * c2.r - c1.i * c2.i;
    c.i = c1.i * c2.r + c1.r * c2.i;
    return (c);
}
/** Complex division.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 / c2
 */
Cmplx IIRFilter::cmplx_div(Cmplx c1, Cmplx c2)
{
    double	a, b, c, d;
    double	f;
    Cmplx	ratio;
    a = c1.r;
    b = c1.i;
    c = c2.r;
    d = c2.i;
    f = c*c + d*d;
    ratio.r = (a*c + b*d) / f;
    ratio.i = (b*c - a*d) / f;
    return (ratio);
}
/** Return the complex structure.
 */
Cmplx IIRFilter::cmplx(double fr, double fi)
{
    Cmplx	c;
    c.r = fr;
    c.i = fi;
    return (c);
}
/** Return the complex conjugate.
 */
Cmplx IIRFilter::cmplx_conjg(Cmplx c)
{
    c.i = -c.i;
    return (c);
}
/** Return the sqrt of a complex number.
 */
Cmplx IIRFilter::cmplx_sqrt(Cmplx c)
{
    double	radius, theta;
    double	c_r, c_i;
    c_r = c.r;
    c_i = c.i;
    radius = sqrt(sqrt(c_r*c_r + c_i*c_i));
    if (c_r == 0.)
    {
	theta = M_PI / 4.;
    }
    theta = 0.5 * atan2(c_i, c_r);
    c.r = radius * cos(theta);
    c.i = radius * sin(theta);
    return (c);
}
/** Multiply a complex  number by a real number.
 */
Cmplx IIRFilter::real_cmplx_mul(double r, Cmplx c)
{
    c.r *= r;
    c.i *= r;
    return (c);
}
/** Return the real part of a complex number.
 */
double IIRFilter::real_part(Cmplx c)
{
    return (c.r);
}

/** Create an IIRFilter instance from a string representation. This
 *  function uses the string argument constructor to create the
 *  IIRFilter from a string in the form produced by the toString()
 *  function.
 *  @param[in] tdel the sampling interval.
 *  @param[in] args the parameters as a string.
 *  @returns an IIRFilter instance.
 */
IIRFilter * IIRFilter::create(double tdel, const string &args)
{
    return new IIRFilter(tdel, args);
}
