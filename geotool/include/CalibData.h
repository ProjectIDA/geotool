#ifndef _CALIB_DATA_H
#define _CALIB_DATA_H

#include "DataMethod.h"

class GTimeSeries;
class GSegment;

/** A DataMethod subclass that multiplies the data values of a GTimeSeries
 *  object by the calibration factor. The calibration factor is store in the
 *  GSegment object.
 *  @ingroup libgmethod
 */
class CalibData : public DataMethod
{
    public:
	CalibData(const string &s);
	CalibData();
	~CalibData(void);

	const char *toString(void);

	Gobject *clone(void);

	virtual CalibData *getCalibDataInstance(void) { return this; }

	bool canAppend(void) { return true; }
	bool rotationCommutative(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	void applyToSegment(GSegment *s) { calibSegment(s); }

	bool Equals(CalibData *c) { return true; }

	static CalibData *create(const string &args);

    protected:
	static void calibSegment(GSegment *s);

    private:

};

#endif
