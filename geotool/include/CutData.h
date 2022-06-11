#ifndef _CUT_DATA_H
#define _CUT_DATA_H

#include "DataMethod.h"

class GTimeSeries;

/** A DataMethod subclass that cuts all or part of a waveform.
 *  @ingroup libgmethod
 */
class CutData : public DataMethod
{
    public:
	CutData(const string &s);
	CutData(double start_time, double end_time);
	~CutData(void);

	const char *toString(void);

	Gobject *clone(void);

	virtual CutData *getCutDataInstance(void) { return this; }

	bool canAppend(void) { return true; }
	bool rotationCommutative(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	void applyToSegment(GSegment *s) {}

	/** Get the start time of the cut.
	 *  @returns the epochal start time.
	 */
	double begSelect() { return beg_select; }
	/** Get the end time of the cut.
	 *  @returns the epochal end time.
	 */
	double endSelect() { return end_select; }

	static CutData *create(const string &args);


    protected:
	double  beg_select; //!< the start time of the cut
	double  end_select; //!< the end time of the cut

    private:

};

#endif
