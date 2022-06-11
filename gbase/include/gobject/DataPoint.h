#ifndef _DATA_POINT_H_
#define _DATA_POINT_H_

#include "gobject/TimeSeries.h"
#include "gobject/Segment.h"

/** 
 *  A GObject that holds a single data value. This structure holds a reference
 *  to a single data value of a TimeSeries.
 *
 *  @see new_DataPoint
 *  @see new_GObjectPart
 *  @see TimeSeries
 *  @see Segment
 *
 * </pre>
 */ /*<pre>*/

typedef struct
{
	GObjectPart		core;

	/*  DataPoint members */
	TimeSeries		timeSeries;
	Segment			segment;
	int			index;
	int			segmentIndex;
	char			label;
} _DataPoint, *DataPoint;

DataPoint new_DataPoint(TimeSeries timeSeries, Segment segment, int index);
#define DataPoint_time(dp) (dp->segment->tbeg + dp->index*dp->segment->tdel)
#define DataPoint_timeSeries(dp) (dp->timeSeries)
#define DataPoint_segment(dp) (dp->segment)
#define DataPoint_index(dp) (dp->index)
#define DataPoint_segmentIndex(dp) (dp->segmentIndex)
#define DataPoint_value(dp) (dp->segment->data[dp->index])

#endif
