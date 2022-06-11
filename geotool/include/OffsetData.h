#ifndef _OFFSET_DATA_H
#define _OFFSET_DATA_H

#include "DataMethod.h"

class GTimeSeries;
class GSegment;

/** A DataMethod subclass that adds an offset value to the data values of a
 *  GTimeSeries object.
 *  @ingroup libgmethod
 */
class OffsetData : public DataMethod
{
    public:
	OffsetData(const string &s);
	OffsetData(double value);
	~OffsetData(void);

	const char *toString(void);

        Gobject *clone(void);

	virtual OffsetData *getOffsetDataInstance(void) { return this; }

	bool canAppend(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	void applyToSegment(GSegment *s) { offsetSegment(s, value); }

	/** Get the offset value.
 	 *  @returns the constant offset value.
 	 */
	double getOffset() { return value; }

	static OffsetData *create(const string &args);


    protected:
	double	value; //!< the offset value.

	static void offsetSegment(GSegment *s, double value);

    private:

};

#endif
