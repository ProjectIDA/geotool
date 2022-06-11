#ifndef _DATA_LOOP_
#define	_DATA_LOOP_

#include "gobject/GObject.h"
#include "gobject/Segment.h"
#include "gobject/Vector.h"

/** This class represents a data buffer. It has methods for adding and
 *  retrieving data from a data buffer. The buffer or loop has a minimum
 *  duration. When a new data segment is added to the loop the oldest segment
 *  might be discarded. This is done in a manner that maintains a least
 *  <b>minDuration</b> seconds of data in the loop.
 */

/**
 *  A GObject that represents a data loop.
 *
 *  @member sta The station name.
 *  @member chan The channel name.
 *  @member sta_q The station name as a quark.
 *  @member stachan_q The station/channel string as a quark.
 *  @member start The index of the oldest segment.
 *  @member storage The number of bytes used for data storage.
 *  @member minDuration The minimum time duration of the data loop.
 *  @member duration The actual duration of the data loop.
 *  @member begTime The beginning time of the data loop.
 *  @member endTime The ending time of the data loop.
 *  @member maxOverlap	The maximum segment overlap (seconds) allowed.
 *  @member maxFutureTime The maximum segment time minus current time allowed.
 *  @member maxAge The maximum current time - segment time allowed.
 *  @member loopLength The storage size of the loop.
 *  @member numSegments The actual number of segments in the loop.
 *  @member *segments The array of Segments.
 */
typedef struct
{
	GObjectPart	core;

        char            net[10];
        char            sta[10];
	char		chan[10];
	int		sta_q;
	int		stachan_q;
	int		start;
	int		storage;
	double		minDuration;
	double		duration;
	double		begTime;
	double		endTime;
	double		maxOverlap;	/* seconds (initially off) */
	double		maxFutureTime;	/* seconds (initially off) */
	double		maxAge;		/* seconds (initially off) */
	int		loopLength;
	int		numSegments;
	Segment		*segments;
} _DataLoop, *DataLoop;

DataLoop new_DataLoop(const char *net, const char *sta, const char *chan,
			int loopLength, double minDuration);
void DataLoop_setMinDuration(DataLoop dl, double newMinDuration);
void DataLoop_setMaxOverlap(DataLoop dl, double maxOverlap);
void DataLoop_setMaxFutureTime(DataLoop dl, double maxFutureTime);
void DataLoop_setMaxAge(DataLoop dl, double maxAge);
void DataLoop_addSegment(DataLoop dl, Segment s);
int DataLoop_getData(DataLoop dl, Vector segments, Segment lasts);


#endif /* _DATA_LOOP_ */
