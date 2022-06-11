#ifndef _GDATA_POINT_H
#define _GDATA_POINT_H

#include "gobject++/Gobject.h"
#include "gobject++/GSegment.h"

class GTimeSeries;

/** A reference to a single data value of a GTimeSeries.
 *  @ingroup libgobject
 */
class GDataPoint : public Gobject
{
    public:
	GDataPoint(GTimeSeries *ts, GSegment *segment, int index) throw(int);
	GDataPoint(GDataPoint &g);
	GDataPoint & operator=(const GDataPoint &g);
	~GDataPoint(void);
	Gobject *clone(void);

	/** Get the GTimeSeries that contains the data value.
	 *  @returns the GTimeSeries instance.
 	 */
	GTimeSeries *timeSeries(void) { return ts; }
	/** Get the GSegment that contains the data value.
	 *  @returns the GSegment instance.
 	 */
	GSegment *segment(void) { return seg; }
	/** Get the index of the data value in the GSegment.
	 *  @returns the data value index
 	 */
	int index(void) { return indx; }
	/** Get the GSegment index in the GTimeSeries.
	 *  @returns the index of the GSegment that contains the data value.
	 */
	int segmentIndex(void) { return segment_index; }
	/** Get the data value.
	 *  @returns the data value.
	 */
	float data(void) { return seg->data[indx]; }
	/** Get the data value.
	 *  @returns the data value.
	 */
	float value(void) { return seg->data[indx]; }
	/** Get the time of the data value.
	 *  @returns the time of the data value.
	 */
	double time(void) { return seg->tbeg() + indx*seg->tdel(); }
	void setIndex(int i) { indx = i; }
	/** Set the GDataPoint label.
	 *  @param[in] new_label
	 */
	void setLabel(char new_label) { label = new_label; }
	char getLabel(void) { return label; }

    protected:
	GTimeSeries	*ts; //!< the timeSeries that contains the data value.
	GSegment	*seg; //!< the segment that contains the data value.
	int		indx; //!< the index of the data value in seg.
	int		segment_index; //!< the index of seg in ts.
	char		label; //!< the label.
};

#endif

