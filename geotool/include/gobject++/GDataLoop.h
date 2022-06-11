#ifndef _GDATA_LOOP_
#define	_GDATA_LOOP_

#include "gobject++/Gobject.h"
#include "gobject++/GSegment.h"
#include "gobject++/gvector.h"

/** A class that implements a simple data buffer for the GSegment class. It
 *  has methods for adding and retrieving GSegment instances from the data
 *  buffer. The buffer is a loop that maintains a minimum duration. When a
 *  new data segment is added to the loop the oldest segment might be discarded.
 *  This is done in a manner that maintains a least min_duration seconds
 *  of data in the loop.
 *
 *  The loop size (in seconds) is the sum of the lengths of the individual
 *  segments. The loop storage size is the loop size * sample rate *
 *  sizeof(float). The loop stores the segments in the order in which they
 *  are added. The segments are not sorted by time. The storage for the loop
 *  grows until the loop size (in seconds) is greater than or equal to
 *  the parameter min_duration. When a segment is added to the loop, the
 *  "oldest" segment will be dropped from the loop only if the remaining
 *  loop size is >= min_duration.
 *  @ingroup libgobject
 */
class GDataLoop : public Gobject
{
    public:
	GDataLoop(const string &net_name, const string &sta_name,
		const string &chan_name, int initial_array_length,
		double minimum_duration) throw(int);
        GDataLoop(const GDataLoop &g);
        GDataLoop & operator=(const GDataLoop &g);
	~GDataLoop(void);

	void setMinDuration(double new_min_duration) throw(int);
	void setMaxOverlap(double maximum_overlap);
	void setMaxFutureTime(double maximum_future_time);
	void setMaxAge(double maximum_age);
	void addSegment(GSegment *s) throw(int);
	int getData(gvector<GSegment *> *segs, GSegment *last_seg);
	
        char	sta[10];      //!< The station name.
	char	chan[10];     //!< The channel name.
        char	net[10];      //!< The network name.
	int	sta_q;	      //!< The station name as a quark.
	int	stachan_q;    //!< The station/channel string as a quark.
	int	start;	      //!< The index of the oldest segment.
	int	storage;      //!< The number of bytes used for data storage.
	double	min_duration; //!< The minimum time duration of the data loop.
	double	duration;     //!< The actual duration of the data loop.
	double	beg_time;     //!< The beginning time of the data loop.
	double	end_time;     //!< The ending time of the data loop.
	double	max_overlap; //!< The maximum segment overlap (seconds) allowed.
	/** The maximum segment time minus current time allowed. */
	double	max_future_time;
	double	max_age;   //!< The maximum current time - segment time allowed.
	int	loop_length;  //!< The storage size of the loop.
	int	num_segments; //!< The actual number of GSegments in the loop.
	GSegment **segments;  //!< The array of GSegments.

    protected:
	void copy(const GDataLoop &g);
};

#endif /* _GDATA_LOOP_ */
