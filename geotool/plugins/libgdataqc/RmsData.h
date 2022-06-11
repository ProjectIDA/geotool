#ifndef _RMS_DATA_H
#define _RMS_DATA_H

#include "DataMethod.h"

class GTimeSeries;
class GSegment;

namespace libgdataqc {

/** A DataMethod subclass that replaces the data with the rms value from a
 *	sliding window.
 *  GTimeSeries object.
 *  @ingroup libgmethod
 */
class RmsData : public DataMethod
{
    public:
	RmsData(const char *s);
	RmsData(int window_length, double start_time, double end_time);
	~RmsData(void);

	const char *toString(void);

        Gobject *clone(void);

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	/** Get the offset value.
 	 *  @returns the constant offset value.
 	 */
	int getWindowLength() { return window_length; }
	double getStartTime() { return start_time; }
	double getEndTime() { return end_time; }

	static RmsData *create(const char *args);


    protected:
	int	window_length; //!< the number of samples (odd) in the window
	double	start_time; //!< the start time
	double	end_time; //!< the end time

	void rmsData(float *data, float *data_out, int npts);

    private:

};

} // libgdataqc

#endif
