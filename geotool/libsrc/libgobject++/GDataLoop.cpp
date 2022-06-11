/** \file GDataLoop.cpp
 *  \brief Defines class GDataLoop.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>

#include "gobject++/GDataLoop.h"
#include "gobject++/GSegment.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
}

/*  A class that implements a simple data buffer for the GSegment class. It
 *  has methods for adding and retrieving GSegment objects from the data
 *  buffer. The loop size (in seconds) is the sum of the lengths of the
 *  individual segments. The loop storage size is the loop size * sample rate
 *  * sizeof(float). The loop stores the segments in the order in which they
 *  are added. The segments are not sorted by time. The storage for the loop
 *  grows until the loop size (in seconds) is greater than or equal to
 *  the parameter min_duration. When a segment is added to the loop, the
 *  "oldest" segment will be dropped from the loop only if the remaining
 *  loop size is >= min_duration.
 */

/** Constructor. The loop size will automatically increase from it's initial
 *  size, loop_length, to insure that at least min_duration seconds of data
 *  are held in the loop.
 *  @param[in] net_name The network name.
 *  @param[in] sta_name The station name.
 *  @param[in] chan_name The channel name.
 *  @param[in] initial_array_length The initial array size of this loop
 *  	(number of segments).
 *  @param[in] minimum_duration The minimum duration in seconds of this loop.
 *  @throws GERROR_MALLOC_ERROR
 */
GDataLoop::GDataLoop(const string &net_name, const string &sta_name,
		const string &chan_name, int initial_array_length,
		double minimum_duration) throw(int) :
	sta_q(0), stachan_q(0), start(0), storage(0),
	min_duration(minimum_duration), duration(0.), beg_time(0.),
	end_time(0.), max_overlap(-1.), max_future_time(-1.),
	max_age(-1.), loop_length(initial_array_length), num_segments(0),
	segments(NULL)
	
{
    char sc[25];

    stringcpy(net, net_name.c_str(), sizeof(net));
    stringToUpper(net);
    stringcpy(sta, sta_name.c_str(), sizeof(sta));
    stringToUpper(sta);
    stringcpy(chan, chan_name.c_str(), sizeof(chan));
    stringToLower(chan);
    snprintf(sc, sizeof(sc), "%s/%s", sta, chan);
    sta_q = stringUpperToQuark(sta);
    stachan_q = stringUpperToQuark(sc);

    segments = (GSegment **)malloc(loop_length*sizeof(GSegment *));
    if( !segments ) {
	GError::setMessage("GDataLoop constructor: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
}

GDataLoop::GDataLoop(const GDataLoop &g) :
	sta_q(0), stachan_q(0), start(0), storage(0),
	min_duration(0.), duration(0.), beg_time(0.),
	end_time(0.), max_overlap(-1.), max_future_time(-1.),
	max_age(-1.), loop_length(0), num_segments(0),
	segments(NULL)
{
    copy(g);
}

GDataLoop & GDataLoop::operator=(const GDataLoop &g)
{
    copy(g);
    return *this;
}

void GDataLoop::copy(const GDataLoop &g)
{
    stringcpy(sta, g.sta, sizeof(sta));
    stringcpy(chan, g.chan, sizeof(chan));
    stringcpy(net, g.net, sizeof(net));
    sta_q = g.sta_q;
    stachan_q = g.stachan_q;
    start = g.start;
    storage = g.storage;
    min_duration = g.min_duration;
    duration = g.duration;
    beg_time = g.beg_time;
    end_time = g.end_time;
    max_overlap = g.max_overlap;
    max_future_time = g.max_future_time;
    max_age = g.max_age;
    loop_length = g.loop_length;
    num_segments = g.num_segments;
    segments = (GSegment **)malloc(loop_length*sizeof(GSegment *));
    for(int i = 0; i < num_segments; i++) {
	segments[i] = g.segments[i];
	segments[i]->addOwner(this);
    }
}

/** Destructor */
GDataLoop::~GDataLoop(void)
{
    for(int i = 0; i < num_segments; i++) {
	segments[i]->removeOwner(this);
    }
    Free(segments);
}

/** Set the loop min_duration. The loop will automatically adjust it's size
 *  as new segments are added to insure that at least new_min_duration
 *  seconds of data are held.
 *  @param[in] new_min_duration the new loop minimum duration.
 *  @throws GERROR_MALLOC_ERROR
 */
void GDataLoop::setMinDuration(double new_min_duration) throw(int)
{
    if(new_min_duration == min_duration) return;

    if(new_min_duration < min_duration && num_segments > 1) {
	// might have to throw out some segments
	GSegment *s = NULL;
	double d = 0.;
	int end_index = start + num_segments - 1;
	int i;

	min_duration = new_min_duration;

	for(i = end_index; i >= start; i--) {
	    if(i >= loop_length) i -= loop_length;
	    s = segments[i];
	    d += s->tend() - s->tbeg();
	    if(d > new_min_duration) break;
	}
	if(i > start) {
	    int n = end_index - i;
	    int k = 0;
	    GSegment **new_s = (GSegment **)malloc(n*sizeof(GSegment *));

	    if( !new_s ) {
		GError::setMessage("GDataLoop.setMinDuration: malloc failed.");
		throw GERROR_MALLOC_ERROR;
	    }

	    for(; i <= end_index; i++) {
		if(i >= loop_length) i -= loop_length;
		new_s[k++] = segments[i];
	    }
	    num_segments = k;
	    Free(segments);
	    segments = new_s;
	    start = 0;
	}
    }
    // else num_segments will increase in addSegment()
}

/** Set the limit for overlapping segments. If max_overlap >= 0., a segment can
 *  not be added to the loop if its start_time < loop_end_time - max_overlap.
 *  The GDataLoop does not check for overlapping segments if max_overlap is
 *  negative. This is the initial state.
 *  @param[in] maximum_overlap The maximum amount that a new segment can
 *  overlap the current end time of the data loop. Set to -1 to allow any
 *  amount of overlap.
 */
void GDataLoop::setMaxOverlap(double maximum_overlap)
{
    max_overlap = maximum_overlap;
}

/** Set the start_time limit for future segments. A segment can not be
 *  added to the loop if its start_time > cpu time + max_future_time
 *  The GDataLoop does not check for future segment start times if
 *  max_future_time is negative. This is the initial state.
 *  @param[in] maximum_future_time The maximum amount that the start time of a
 *  new segment can be greater that the current cpu time.
 *  Set to -1 to allow all start times.
 */
void GDataLoop::setMaxFutureTime(double maximum_future_time)
{
    max_future_time = fabs(maximum_future_time);
}

/** Set the limit for old segments. A segment can not be added to the loop
 *  if the cpu time - segment_start_time > max_age. The GDataLoop does not
 *  check for old segment start times if max_age is negative. This is the
 *  initial state.
 *  @param[in] maximum_age The maximum value allowed for cpu time -
 *	segment_start_time. Set to -1 to allow all start times.
 */
void GDataLoop::setMaxAge(double maximum_age)
{
    max_age = maximum_age;
}

/** Add a data segment to the loop.
 *  @param[in] s the segment
 *  @throws GERROR_MALLOC_ERROR
 */
void GDataLoop::addSegment(GSegment *s) throw(int)
{
    if(max_overlap >= 0. && num_segments > 0)
    {
	int end_index = start + num_segments - 1;
	if(end_index >= loop_length) end_index -= loop_length;
	if(s->tbeg() < segments[end_index]->tend() - max_overlap) return;
    }

    if(max_future_time > 0.)
    {
	double now = timeGetEpoch();
	if(s->tbeg() > now + max_future_time) return;
    }

    if(max_age >= 0.)
    {
	double now = timeGetEpoch();
	if(now - s->tbeg() > max_age) return;
    }

    /* check if we need to increase the loop size
     */
    if(num_segments == loop_length)
    {
	double new_duration = duration + s->tend() - s->tbeg();

	if(num_segments > 1) {
	    new_duration -= (segments[start]->tend() - segments[start]->tbeg());
	}
	if(duration < min_duration || new_duration < min_duration)
	{
	    int i;
	    GSegment **new_segments = (GSegment **)malloc(
				(loop_length+1)*sizeof(GSegment **));
	    if( !new_segments ) {
		GError::setMessage("GDataLoop.addSegment: malloc failed.");
		throw GERROR_MALLOC_ERROR;
	    }

	    for(i = 0; i < num_segments; i++) {
		int j = start + i;
		if(j >= loop_length) j -= loop_length;
		new_segments[i] = segments[j];
	    }
	    start = 0;
	    Free(segments);
	    segments = new_segments;
	    loop_length++;
	    segments[num_segments++] = s;
	    s->addOwner(this);
	    end_time = s->tend();
	    duration += (s->tend() - s->tbeg());
	    storage += s->length()*sizeof(float);
/*
printf("data loop %s/%s: increasing loop size to %d  loop duration: %.2f\n",
sta, chan, loop_length, duration);
*/
	}
	else {
	    duration -= (segments[start]->tend() - segments[start]->tbeg());
	    storage -= segments[start]->length()*sizeof(float);
	    segments[start]->removeOwner(this);
	    segments[start] = s;
	    s->addOwner(this);
	    start++;
	    if(start >= loop_length) start = 0;
	    beg_time = segments[start]->tbeg();
	    end_time = s->tend();
	    duration += (s->tend() - s->tbeg());
	    storage += s->length()*sizeof(float);
/*
printf("data loop %s/%s: replacing segment %d  loop duration: %.2f\n",
sta, chan, start, duration);
*/
	}
    }
    else {
	int end_index = start + num_segments - 1;
	end_index++;
	if(end_index >= loop_length) end_index -= loop_length;
	segments[end_index] = s;
	s->addOwner(this);
	if(num_segments == 0) beg_time = s->tbeg();
	end_time = s->tend();
	num_segments++;
	duration += (s->tend() - s->tbeg());
	storage += s->length()*sizeof(float);
/*
printf("data loop %s/%s: adding segment %d  loop duration: %.2f\n",
sta, chan, end_index, duration);
*/
    }
}

/** Get segments from the loop. Returns segments in the loop that have
 *  been received since the segment last_seg was received. If
 *  last_seg is null, all segments in the loop are returned.
 *  @param[in,out] segs the segments are appended to this gvector.
 *  @param[in] last_seg the last segment retrieved the previous time this
 * 	function was called, or NULL.
 */
int GDataLoop::getData(gvector<GSegment *> *segs, GSegment *last_seg)
{
    int i, j, end_index, n;

    if(num_segments == 0) return 0;

    end_index = start + num_segments - 1;
	
    for(i = end_index; i >= start; i--) {
	j = i;
	if(j >= loop_length) j -= loop_length;
	if(segments[j] == last_seg) break;
    }
    n = 0;
    for(++i; i <= end_index; i++) {
	j = i;
	if(j >= loop_length) j -= loop_length;
	segs->add(segments[j]);
	n++;
    }
    return n;
}
