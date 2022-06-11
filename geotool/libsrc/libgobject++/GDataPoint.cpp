/** \file GDataPoint.cpp
 *  \brief Defines class GDataPoint.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "gobject++/GDataPoint.h"
#include "gobject++/GTimeSeries.h"

/** 
 *  Constructor. A GDataPoint is a reference to one data value of a GSegment
 *  of a GTimeSeries.
 *  @param[in] time_series The GTimeSeries that contains the data point.
 *  @param[in] segment The GSegment that contains the data point. Must be a
 *		segment contained in time_series.
 *  @param[in] idx The index of the data point in the segment.
 *  @throws GERROR_INVALID_ARG
 */
GDataPoint::GDataPoint(GTimeSeries *time_series, GSegment *s, int idx)
		throw(int) : ts(time_series), seg(s), indx(idx),
		segment_index(0), label('\0')
{
    for(segment_index = 0; segment_index < ts->size(); segment_index++)
    {
	if(ts->segment(segment_index) == s) break;
    }
    if(segment_index == ts->size()) {
	GError::setMessage("GDataPoint: segment not contained in time_series.");
	throw GERROR_MALLOC_ERROR;
    }
    if(idx < 0 || idx >= s->length()) {
	GError::setMessage(
		"GDataPoint: index out of bounds %d, segment length: %d",
		idx, s->length());
	throw GERROR_MALLOC_ERROR;
    }
    ts->addOwner(this);
    seg->addOwner(this);
}

GDataPoint::GDataPoint(GDataPoint &g) : ts(g.ts), seg(g.seg), indx(g.indx),
		segment_index(g.segment_index), label(g.label)
{
    ts->addOwner(this);
    seg->addOwner(this);
}

GDataPoint & GDataPoint::operator=(const GDataPoint &g)
{
    if(this != &g) {
	ts->removeOwner(this);
	seg->removeOwner(this);
	ts = g.ts;
	seg = g.seg;
	indx = g.indx;
	segment_index = g.segment_index;
	label = g.label;
	ts->addOwner(this);
	seg->addOwner(this);
    }
    return *this;
}

/** Constructor. */
GDataPoint::~GDataPoint(void)
{
    ts->removeOwner(this);
    seg->removeOwner(this);
}

Gobject * GDataPoint::clone(void)
{
    GDataPoint *dp = new GDataPoint(ts, seg, indx);
    return (Gobject *)dp;
}
