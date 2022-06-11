#ifndef _OVERLAPS_H_
#define _OVERLAPS_H_

#include "gobject/GObject.h"
#include "gobject/TimeSeries.h"
#include "gobject/Segment.h"
#include "gobject/Vector.h"

/**
 *  A GObject for holding overlapping TimeSeries objects. This object contains
 *  data for all the time windows in which a set of TimeSeries objects overlap.
 *  The data for each period of data overlap are available as an enumeration
 *  of Array objects. The following example shows the usage of the Overlaps
 *  object in the rotation of two horizontal component TimeSeries:
 *  <pre>
 *  Overlaps o;
 *  TimeSeries ts[2];
 *  double e_mean, n_mean;
 *
 *  ...
 *
 *  for(o = new_Overlaps(ts, 2); Overlaps_hasMoreElements(o); )
 *  {
 *      Array array = (Array)Overlaps_nextElement(o);
 *      for(i = 0; i < array->data_length; i++) array->data[0][i] -= e_mean;
 *      for(i = 0; i < array->data_length; i++) array->data[1][i] -= n_mean;
 *
 *      Rotate_data(array->data[0], array->data[1], array->data_length, angle);
 *      ...
 *
 *      GObject_free(array);
 *  }
 *  GObject_free(o);
 *
 *  @see new_Overlaps
 *  @see TimeSeries
 *  @see Segment
 *  @see Vector
 *  @see Array
 *  @see Enumeration
 *  @see new_GObjectPart
 * </pre>
 */ /*<pre>*/

/** 
 *  The Hashtable structure definition.
 *
 *  @member ts The original TimeSeries objects.
 *  @member ts_length The number of TimeSeries objects.
 *  @member tdel The sample time interval.
 *  @member tdel_tolerance The tolerance for sample interval differences.
 *  @member lap_segments A Vector containing Vectors of Segments which overlap.
 *  @member lap_tmin The beginning times for the periods of overlap.
 *  @member lap_tmax The ending times for the periods of overlap.
 *  @member e_seg An Enuration object for lap_segments.
 */
typedef struct
{
	GObjectPart		core;

	/* Overlaps members */
	TimeSeries		*ts;
	int			ts_length;
	double			tdel;
	double			tdel_tolerance;
	Vector			lap_segments;
	double			*lap_tmin;
	double			*lap_tmax;
	Enumeration		e_seg;
} _Overlaps, *Overlaps;

typedef struct
{
	GObjectPart	core;

	double		tmin;
	double		tmax;
	int		num_segments;
	int		npts;
	Segment		*segments;
	int		*begIndex;
} _SegmentArray, *SegmentArray;


Overlaps new_Overlaps(TimeSeries *ts, int ts_length);
Overlaps new_Overlaps2(TimeSeries *ts, int ts_length, double tmin,
			double tmax);
bool Overlaps_hasMoreElements(Overlaps o);
GObject Overlaps_nextElement(Overlaps o);
Vector Overlaps_getArrays(TimeSeries *ts, int ts_length, double t_min,
			double t_max);
SegmentArray new_SegmentArray(double tmin, double tmax, int npts,
		int num_segments, Segment *segments, int *begIndex);

#define Overlaps_num(o) o->lap_segments->elementCount

#endif
