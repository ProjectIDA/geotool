/** \file GSegmentArray.cpp
 *  \brief Defines class GSegmentArray.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "gobject++/GSegmentArray.h"
#include "gobject++/GSegment.h"

/** Constructor.
 *  @param[in] start_time the start time of the time period.
 *  @param[in] end_time the end time of the time period.
 *  @param[in] nsamples the number of data samples in the time period.
 *  @param[in] num_segs the number of GSegments that have data for this
 *	time period.
 *  @param[in] segs an array of length num_segments that contains the
 *	GSegment objects.
 *  @param[in] first_sample an array of length num_segments that contains the
 *	index of the first sample that is in the time period for each GSegment.
 */
GSegmentArray::GSegmentArray(double start_time, double end_time, int nsamples,
		int num_segs, GSegment **segs, int *first_sample) :
		tmin(start_time), tmax(end_time), npts(nsamples),
		num_segments(num_segs), segments(NULL), beg_index(NULL)
{
    segments = new GSegment * [num_segments];
    memcpy(segments, segs, num_segments*sizeof(GSegment *));

    beg_index = new int [num_segments];
    memcpy(beg_index, first_sample, num_segments*sizeof(int));

    for(int i = 0; i < num_segments; i++) segments[i]->addOwner(this);
}

/** Destructor */
GSegmentArray::~GSegmentArray(void)
{
    for(int i = 0; i < num_segments; i++) segments[i]->removeOwner(this);
    delete [] segments;
    delete [] beg_index;
}

GSegmentArray & GSegmentArray::operator=(const GSegmentArray &rhs)
{
    if(this != &rhs) {
	for(int i = 0; i < num_segments; i++) segments[i]->removeOwner(this);
	delete [] segments;
	delete [] beg_index;
	copy(rhs);
    }
    return *this;
}

void GSegmentArray::copy(const GSegmentArray &g)
{
    tmin = g.tmin;
    tmax = g.tmax;
    npts = g.npts;
    num_segments = g.num_segments;

    segments = new GSegment * [num_segments];
    memcpy(segments, g.segments, num_segments*sizeof(GSegment *));

    beg_index = new int [num_segments];
    memcpy(beg_index, g.beg_index, num_segments*sizeof(int));

    for(int i = 0; i < num_segments; i++) segments[i]->addOwner(this);
}
