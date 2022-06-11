#ifndef _GSEGMENT_H_
#define _GSEGMENT_H_

#include "gobject++/Gobject.h"

class GTimeSeries;

/** A class that holds a segment of waveform data. The segment has a uniform
 *  sample time interval with no data gaps.
 *  @ingroup libgobject
 */
class GSegment : public Gobject
{
    // Allow GTimeSeries to access protected members
    friend class GTimeSeries;

    public:
	GSegment(float *seg_data, int seg_data_length, double tbeg, double tdel,
		double calibration, double calperiod) throw(int);
	GSegment(double *seg_data, int seg_data_length, double tbeg,double tdel,
		double calibration, double calperiod) throw(int);
	GSegment(int seg_data_length, double tbeg, double tdel,
		double calibration, double calperiod) throw(int);
	GSegment(GSegment &s);
	GSegment(GSegment *s);
	GSegment & operator=(const GSegment &g);
	GSegment *subsegment(int begin_index, int end_index) throw(int);
	~GSegment(void);
	Gobject *clone(void);

	void setData(float *seg_data);

	void setCalibration(double calibration, double calperiod);
	/** Get the beginning time of this GSegment.
	 *  @returns the epochal beginning time.
	 */
	double tbeg(void) { return beg; }
	/** Get the sample time increment for the data in this GSegment.
	 *  @returns the sample time increment in seconds.
	 */
	double tdel(void) { return del; }
	/** Get the end time of this GSegment.
	 *  @returns the epochal end time.
	 */
	double tend(void) { return beg + (data_length-1)*del; }
	/** Get the time of the i'th sample.
	 *  @returns the epochal time of the i'th sample.
	 */
	double time(int i) { return beg + i*del; }
	/** Get the calibration factor for this GSegment.
	 *  @returns the calib.
	 */
	double calib(void) { return Calib; }
	/** Get the calibration period for this GSegment.
	 *  @returns the calper.
	 */
	double calper(void) { return Calper; }
	/** Get the initial calibration factor for this GSegment.
	 *  @returns the initial calib.
	 */
	double initialCalib(void) {
	  //printf("DEBUG: in GSegment::initalCalib initial_calib = %f\n", initial_calib);
	  return initial_calib;
	} 
	/** Get the initial calibration period for this GSegment.
	 *  @returns the calper.
	 */
	double initialCalper(void) {
	  //printf("DEBUG: in GSegment::initalCalper initial_calper = %f\n", initial_calper);
	  return initial_calper;
	}
	/** Get the length of the segment.
	 *  @returns the number of sample values contained in the GSegment.
	 */
	int length(void) { return data_length; }

	float	*data;		//!< the data samples

    protected:
	double	beg;		//!< the epoch time of the first data sample
	double	del;		//!< the time increment
	int	data_length;	//!< the number of samples in this segment
	double	initial_calib;	//!< the initial calibration factor
	double	initial_calper;	//!< the initial calibration period
	double	Calib;		//!< the calibration factor
	double	Calper;		//!< the calibration period

	void init(int seg_data_length, double tbeg, double tdel,
		double calibration, double calperiod) throw(int);
	void setTdel(double tdel);
	void truncate(int i1, int i2) throw(int);
	/** Set the length of the segment.
	 *  @param[in] len the new length of the segment.
	 */
	void setLength(int len) { data_length = len; }
};

#endif
