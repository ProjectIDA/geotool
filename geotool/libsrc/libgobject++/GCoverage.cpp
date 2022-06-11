/** \file GCoverage.cpp
 *  \brief Defines class GCoverage.
 *  \author Ivan Henson
 *  A class for periods of data coverage for GTimeSeries objects. This class
 *  contains data for all the time windows for which all members of a set of
 *  GTimeSeries objects have data. The data for each period are available as
 *  an enumeration of GArray objects. The following example shows the usage of
 *  the GCoverage object in the rotation of two horizontal component
 *  GTimeSeries:
 *  \code
    GCoverage *o;
    GTimeSeries *ts[2];
    double e_mean, n_mean;

    ... // get the GTimeSeries objects

    e_mean = ts[0]->mean();
    n_mean = ts[1]->mean();

    for(o = new GCoverage(ts, 2); o->hasMoreElements(); )
    {
        GArray *array = (GArray *)o->nextElement();
        for(i = 0; i < array->data_length; i++) array->data[0][i] -= e_mean;
        for(i = 0; i < array->data_length; i++) array->data[1][i] -= n_mean;

        rotate(array->data[0], array->data[1], array->length(), angle);
        ...

        array->deleteObject();
    }
    o->deleteObject();
    \endcode
 */
#include "config.h"
#include <iostream>
#include <math.h>

using namespace std;

#include "gobject++/GCoverage.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GArray.h"
#include "gobject++/GSegmentArray.h"


/** Constructor. Determine the periods of complete data coverage for the input
 *	GTimeSeries objects.
 *  @param[in] t an array of GTimeSeries objects.
 *  @param[in] num the number of objects in t[].
 *  @throws GERROR_MALLOC_ERROR and GERROR_SAMPLE_RATE_EXCEPTION if the sample
 *   time intervals of the input segments differ by more than the
 *   tdelTolerance() of the first GTimeSeries object.
 */
GCoverage::GCoverage(gvector<GTimeSeries *> &t) throw(int) :
	ts(), tdel(0.), cov_segments(), cov_tmin(), cov_tmax(), e_seg(NULL)
{
    if(t.size() > 0) {
	double tmin = t[0]->tbeg();
	double tmax = t[0]->tend();

	init(t, tmin, tmax);
    }
}

/** Constructor with time limits. For the time window tmin to tmax, determine
 *  the periods of complete data coverage for the input GTimeSeries objects.
 *  @param[in] t an array of GTimeSeries objects.
 *  @param[in] num the number of GTimeSeries objects in t[].
 *  @param[in] tmin the minimum time.
 *  @param[in] tmax the maximum time.
 *  @throws GERROR_MALLOC_ERROR and GERROR_SAMPLE_RATE_EXCEPTION if the sample
 *   time intervals of the input segments differ by more than the
 *   tdelTolerance() of the first GTimeSeries object.
 */
GCoverage::GCoverage(gvector<GTimeSeries *> &t, double tmin, double tmax)
		throw(int) : ts(), tdel(0.), cov_segments(), cov_tmin(),
				cov_tmax(), e_seg(NULL)
{
    init(t, tmin, tmax);
}

GCoverage::GCoverage(GCoverage &c) : ts(), tdel(c.tdel),
	cov_segments(c.cov_segments), cov_tmin(c.cov_tmin),
	cov_tmax(c.cov_tmax), e_seg(NULL)
{
    for(int i = 0; i < c.ts.size(); i++) {
	ts.push_back(c.ts[i]);
    }
    e_seg = new genumeration<gvector<GSegment *> *>(&cov_segments);
    e_seg->addOwner(this);
}

GCoverage & GCoverage::operator=(GCoverage &c)
{
    if(this != &c) {
	clear();
	for(int i = 0; i < c.ts.size(); i++) {
	    ts.push_back(c.ts[i]);
	}
	tdel = c.tdel;
	cov_segments = c.cov_segments;
	cov_tmin = c.cov_tmin;
	cov_tmax = c.cov_tmax;
	e_seg = new genumeration<gvector<GSegment *> *>(&cov_segments);
	e_seg->addOwner(this);
    }
    return *this;
}

/** Initialization. For the time window tmin to tmax, determine the periods of
 *  complete data coverage for the input GTimeSeries objects.
 *  @param[in] t an array of GTimeSeries objects.
 *  @param[in] num the number of GTimeSeries objects in t[].
 *  @param[in] tmin the minimum time.
 *  @param[in] tmax the maximum time.
 *  @throws GERROR_MALLOC_ERROR and GERROR_SAMPLE_RATE_EXCEPTION if the sample
 *   time intervals of the input segments differ by more than the
 *   tdelTolerance() of the first GTimeSeries object.
 */
void GCoverage::init(gvector<GTimeSeries *> &t, double tmin, double tmax)
		throw(int)
{
    if(t.size() > 0) {
	GSegment **segments = NULL;

	for(int i = 0; i < t.size(); i++) {
	    ts.push_back(t[i]);
	}

	for(int i = 0; i < ts.size(); i++) {
	    if(tmin < ts[i]->tbeg()) tmin = ts[i]->tbeg();
	    if(tmax > ts[i]->tend()) tmax = ts[i]->tend();
	}
	segments = new GSegment * [ts.size()];

	getCoverage(0, segments, tmin, tmax);

	delete [] segments;

	e_seg = new genumeration<gvector<GSegment *> *>(&cov_segments);
	e_seg->addOwner(this);
    }
}

/** Destructor. */
GCoverage::~GCoverage(void)
{
    clear();
}

void GCoverage::clear(void)
{
    ts.clear();

    cov_segments.clear();
    if(e_seg) e_seg->removeOwner(this);

    cov_tmin.clear();
    cov_tmax.clear();
}

Gobject * GCoverage::clone(void)
{
    return (Gobject *) new GCoverage(ts);
}

/** Determine if the i'th GTimeSeries has data between tmin and tmax. This
 *  function is called recursively for all time series objects to determine if
 *  there is complete coverage of data between tmin and tmax.
 *  @param[in] i the index in the ts[] array.
 *  @param[in,out] segments the array of GSegments that have data between tmin
 *	and tmax.
 *  @param[in] tmin the start time of the time period.
 *  @param[in] tmax the end time of the time period.
 *  @throws GERROR_MALLOC_ERROR and GERROR_SAMPLE_RATE_EXCEPTION.
 */
void GCoverage::getCoverage(int i, GSegment **segments, double tmin,
			double tmax) throw(int)
{
    int j;

    if(i >= ts.size()) {
	gvector<GSegment *> *segs = new gvector<GSegment *>;
	for(j = 0; j < ts.size(); j++) {
	    segs->add(segments[j]);
	}
	cov_segments.add(segs);

	j = cov_segments.size();
	cov_tmin.push_back(tmin);
	cov_tmax.push_back(tmax);
        return;
    }

    for(j = 0; j < ts[i]->size(); j++)
    {
	GSegment *s = ts[i]->segment(j);
	if(s->tbeg() < tmax && s->tend() > tmin)
	{
	    double tbeg = (tmin > s->tbeg()) ? tmin : s->tbeg();
	    double tend = (tmax < s->tend()) ? tmax : s->tend();

	    if(i == 0) {
		tdel = s->tdel();
	    }
	    else {
		double ddt = fabs(tdel - s->tdel())/tdel;
		if(ddt > ts[0]->tdelTolerance()) {
		    GError::setMessage(
				"GCoverage.getCoverage: SampleRateException.");
		    throw GERROR_SAMPLE_RATE_EXCEPTION;
		}
	    }
	    segments[i] = s;
	    getCoverage(i+1, segments, tbeg, tend);
	}
    }
}

/** Check for more data coverage periods.
 * @returns true if there are more time periods of complete data coverage.
 */
bool GCoverage::hasMoreElements(void)
{
    return (e_seg) ? e_seg->hasMoreElements() : false;
}

/** Get the next data coverage period.
 *  @returns the next GArray object with the data.
 */
GArray * GCoverage::nextElement(void)
{
    if(!e_seg) return NULL;

    gvector<GSegment *> *v = e_seg->nextElement();

    if(!v) {
	return NULL;
    }
    else {
	double tmin = cov_tmin[e_seg->index()-1];
	double tmax = cov_tmax[e_seg->index()-1];
	double t_del = 0.;
	int i;
	int n = 0;

	for(i = 0; i < ts.size(); i++) {
	    int beg, end;
	    GSegment *s = v->at(i);
	    t_del = s->tdel();
	    beg = (int)((tmin - s->tbeg())/t_del + .5);
	    if(s->tbeg() + beg*t_del < tmin) beg++;
	    if(beg >= s->length()) beg = s->length()-1;
	    end = (int)((tmax - s->tbeg())/t_del + .5);
	    end = (int)((tmax - s->tbeg())/t_del);

	    if(i == 0) n = end - beg + 1;
	    else if(n > end - beg + 1) n = end - beg + 1;
	}

	GArray *a = new GArray(n, tmin, t_del);
	if(!a) return NULL;

	for(i = 0; i < ts.size(); i++) {
	    GSegment *s = v->at(i);
	    int beg = (int)((tmin - s->tbeg())/s->tdel() + .5);
	    a->add(s->data+beg);
	}

	return a;
    }
}

/** Get all time periods and segments where there is complete data coverage.
 *  @param[in] ts an array of GTimeSeries objects.
 *  @param[in] t_min the start time of the time period.
 *  @param[in] t_max the end time of the time period.
 *  @returns a gvector of GSegmentArray objects for each time period where
 *	there is complete data coverage.
 *  @throws GERROR_MALLOC_ERROR and GERROR_SAMPLE_RATE_EXCEPTION.
 */
gvector<GSegmentArray *> * GCoverage::getArrays(gvector<GTimeSeries *> &ts,
			 double t_min, double t_max) throw(int)
{
    int num;
    GCoverage *o = new GCoverage(ts, t_min, t_max);

    if( !(num = o->cov_segments.size()) ) {
	o->deleteObject();
	return NULL;
    }

    int *beg_index = new int[ts.size()];
    GSegment **segments = new GSegment * [ts.size()];
    if( !beg_index || !segments ) {
	GError::setMessage("GCoverage.getCoverage: malloc failed.");
	o->deleteObject();
	throw GERROR_MALLOC_ERROR;
    }
    gvector<GSegmentArray *> *arrays = new gvector<GSegmentArray *>(num);

    for(int i = 0; i < num; i++)
    {
	gvector<GSegment *> *v = o->cov_segments.at(i);
	double tmin = o->cov_tmin[i];
	double tmax = o->cov_tmax[i];
	int n = 0;

	v->copyInto(segments);

	for(int j = 0; j < ts.size(); j++) {
	    int beg, end;
	    GSegment *s = segments[j];
	    beg = (int)((tmin - s->tbeg())/s->tdel() + .5);
	    if(s->tbeg() + beg*s->tdel() < tmin) beg++;
	    if(beg >= s->length()) beg = s->length()-1;

	    end = (int)((tmax - s->tbeg())/s->tdel() + .5);
	    if(s->tbeg() + end*s->tdel() > tmax) end--;
	    if(end < 0) end = 0;
	    if(end >= s->length()) end = s->length()-1;
	    
	    if(j == 0) n = end - beg + 1;
	    else if(n > end - beg + 1) n = end - beg + 1;
	    beg_index[j] = beg;
	}
	arrays->add(new GSegmentArray(tmin, tmax, n, ts.size(),
				segments, beg_index));
    }
    delete [] beg_index;
    delete [] segments;
    o->deleteObject();
    return arrays;
}
