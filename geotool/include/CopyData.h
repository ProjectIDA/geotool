#ifndef _COPY_DATA_H
#define _COPY_DATA_H

#include "DataMethod.h"

class GTimeSeries;

/** A DataMethod subclass that copies all or part of a waveform.
 *  @ingroup libgmethod
 */
class CopyData : public DataMethod
{
    public:
	CopyData(const string &s);
	CopyData(double start_time, double end_time);
	~CopyData(void);

	const char *toString(void);

	Gobject *clone(void);

	virtual CopyData *getCopyDataInstance(void) { return this; }

	bool rotationCommutative(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	/** Get the start time of the copy.
	 *  @returns the epochal start time.
	 */
	double begSelect() { return beg_select; }
	/** Get the end time of the copy.
	 *  @returns the epochal end time.
	 */
	double endSelect() { return end_select; }

	static CopyData *create(const string &args);


    protected:
	double	beg_select; //!< the start time of the copy
	double	end_select; //!< the end time of the copy

    private:

};

#endif
