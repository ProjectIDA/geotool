#ifndef _GSEGMENT_ARRAY_H
#define _GSEGMENT_ARRAY_H

#include "gobject++/Gobject.h"

class GSegment;

/** A class that holds complete data coverage GSegments for a specific time
 *  period.
 *  @ingroup libgobject
 */
class GSegmentArray : public Gobject
{
    public:
	GSegmentArray(double start_time, double end_time, int nsamples,
		int num_segs, GSegment **segs, int *first_sample);
	GSegmentArray(GSegmentArray &g) : tmin(g.tmin), tmax(g.tmax),
		npts(g.npts), num_segments(g.num_segments), segments(NULL),
		beg_index(NULL) { copy(g); }
	GSegmentArray & operator=(const GSegmentArray &rhs);
	~GSegmentArray(void);

	double	tmin; //!< The start time of the time period.
	double	tmax; //!< The end time of the time period.
	int	npts; //!< The number of sample points in the time period.
	int	num_segments; //!< The number of GSegments
	GSegment **segments;  //!< An array of length num_segments.
	/** An array of length num_segments that contains the index of the
	 *  first sample that is in the time period for each GSegment.
	 */
	int	*beg_index; 

    private:
	void copy(const GSegmentArray &g);
};

#endif

