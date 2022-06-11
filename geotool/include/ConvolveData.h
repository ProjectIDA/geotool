#ifndef _CONVOLVE_DATA_H
#define _CONVOLVE_DATA_H

#include <vector>
using namespace std;

#include "DataMethod.h"

class GTimeSeries;
class Response;

/** A DataMethod subclass that performs a convolution or deconvolution operation
 *  on the data values of a GTimeSeries object. The operation can include
 *  several instrument responses, specified by Response class objects.
 *  @ingroup libgmethod
 */
class ConvolveData : public DataMethod
{
    public:
	ConvolveData(const string &s) throw(int);
	ConvolveData(int operation_direction, vector<Response *> &resp,
		const string &instrument_type, double flow, double fhigh,
		double amp_cutoff, double calib, double calper,
		bool remove_fir_time_shift);
	ConvolveData(int operation_direction, Response *resp,
		const string &instrument_type, double flow, double fhigh,
		double amp_cutoff, double calib, double calper,
		bool remove_fir_time_shift);
	~ConvolveData(void);

	const char *toString(void);

	Gobject *clone(void);

	virtual ConvolveData *getConvolveDataInstance(void) { return this; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	bool applyMethod(GTimeSeries *ts);
	void applyToSegment(GSegment *s);
	/** Get the instrument type.
	 *  @returns the instrument type.
	 */
	char *getInstype() { return instype; }
	/** Get the direction of the operation.
	 *   - -1 for deconvolution
	 *   - 1 for convolution
	 *  @returns the direction.
	 */
	int getDirection() { return direction; }
	/** Get the filter low frequency cutoff.
	 *  @returns the low frequency cutoff.
	 */
	double getFlow() { return flo; }
	/** Get the filter high frequency cutoff.
	 *  @returns the high frequency cutoff.
	 */
	double getFhigh() { return fhi; }
	/** Get the calib.
	 *  @returns the calib.
	 */
	double getCalib() { return ncalib; }
	/** Get the calper.
	 *  @returns the calper.
	 */
	double getCalper() { return ncalper; }

	vector<Response *> responses; //!< the instrument Response objects

//	static ConvolveData *create(const char *args);

    protected:
	char	instype[7]; //!< the instrument type
	/** The direction of the operation.
	 *   - -1 for deconvolution
	 *   - 1 for convolution
	 */
	int	direction; 
	double	flo; //!< the filter low frequency cutoff
	double	fhi; //!< the filter high frequency cutoff
	double	cutoff; //!< the low amplitude cutoff (fraction) for deconvolution
	double	ncalib; //!< normalize the response amplitude to ncalib at period ncalper
	double	ncalper; //!< normalize the response amplitude to ncalib at period ncalper
	bool	remove_time_shift;

    private:

};

#endif
