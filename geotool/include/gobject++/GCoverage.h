#ifndef _GOVERLAPS_H
#define _GOVERLAPS_H

#include "gobject++/gvector.h"
#include "gobject++/GSegmentArray.h"

class GTimeSeries;
class GSegment;
class GArray;

/**
 *  A class for determining time periods of data coverage. This class contains
 *  functions for determining the time periods for which all members of a set
 *  of GTimeSeries objects have data. The data for each time period can be
 *  accessed as a gvector of GSegmentArray objects and also as an enumeration
 *  of GArray objects. The following example shows the usage of the GCoverage
 *  object in the rotation of two horizontal component GTimeSeries:
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
 *  @ingroup libgobject
 */
class GCoverage : public Gobject
{
    public:
	GCoverage(gvector<GTimeSeries *> &ts) throw(int);
	GCoverage(gvector<GTimeSeries *> &ts, double tmin, double tmax)
		throw(int);
        GCoverage(GCoverage &c);
        GCoverage & operator=(GCoverage &c);

	~GCoverage(void);
	Gobject *clone(void);

	bool hasMoreElements(void);
	GArray *nextElement(void);

	static gvector<GSegmentArray *> *getArrays(gvector<GTimeSeries *> &ts,
			double t_min, double t_max) throw(int);

    protected:
	gvector<GTimeSeries *> ts; //!< The original GTimeSeries objects.
	double tdel;		   //!< The sample time interval.
	/** A gvector containing gvectors of GSegments which all have
		data for the time periods in cov_tmin[], cov_tmax[]. */
	gvector<gvector<GSegment *> *>	cov_segments;
		//! The beginning times for the periods of data coverage.
	vector<double>	cov_tmin;
		//! The ending times for the periods of data coverage.
	vector<double>	cov_tmax;
	/** A genumeration object for cov_segments. */
	genumeration<gvector<GSegment *> *> *e_seg;

	void init(gvector<GTimeSeries *> &ts, double tmin, double tmax)
		throw(int);
	void getCoverage(int i, GSegment **segments, double tmin, double tmax)
		throw(int);
	void clear(void);
};

#endif

