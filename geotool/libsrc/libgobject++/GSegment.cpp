/** \file GSegment.cpp
 *  \brief Defines class GSegment.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "gobject++/GSegment.h"

/** Constructor for float data. The data array is copied.
 *  @param[in] seg_data The float data.
 *  @param[in] seg_data_length The number of data samples.
 *  @param[in] tstart The time of the first data sample.
 *  @param[in] dt The sample time interval.
 *  @param[in] calibration The calibration factor for this data.
 *  @param[in] calperiod The calibration period for this data.
 *  @throws GERROR_MALLOC_ERROR
 */
GSegment::GSegment(float *seg_data, int seg_data_length, double tstart,
		double dt, double calibration, double calperiod) throw(int) :
	data(NULL), beg(0.), del(0.), data_length(0), initial_calib(0.),
	initial_calper(0.), Calib(0.), Calper(0.)
{
    init(seg_data_length, tstart, dt, calibration, calperiod);
    if(seg_data_length > 0) {
	memcpy(data, seg_data, seg_data_length*sizeof(float));
    }
}

/** Constructor for double data. The data array is copied and converted to
 *  float.
 *  @param[in] seg_data the double data.
 *  @param[in] seg_data_length The number of data samples.
 *  @param[in] tstart The time of the first data sample.
 *  @param[in] dt The sample time interval.
 *  @param[in] calibration The calibration factor for this data.
 *  @param[in] calperiod The calibration period for this data.
 *  @throws GERROR_MALLOC_ERROR
 */
GSegment::GSegment(double *seg_data, int seg_data_length, double tstart,
		double dt, double calibration, double calperiod) throw(int) :
	data(NULL), beg(0.), del(0.), data_length(0), initial_calib(0.),
	initial_calper(0.), Calib(0.), Calper(0.)
{
    init(seg_data_length, tstart, dt, calibration, calperiod);
    for(int i = 0; i < seg_data_length; i++) data[i] = (float)seg_data[i];
}

/** Constructor with data initialized to 0. Creates an array of data
 *  initialized to 0.
 *  @param[in] seg_data_length The number of data samples.
 *  @param[in] tstart The time of the first data sample.
 *  @param[in] dt The sample time interval.
 *  @param[in] calibration The calibration factor for this data.
 *  @param[in] calperiod The calibration period for this data.
 *  @throws GERROR_MALLOC_ERROR
 */
GSegment::GSegment(int seg_data_length, double tstart, double dt,
		double calibration, double calperiod) throw(int) :
	data(NULL), beg(0.), del(0.), data_length(0), initial_calib(0.),
	initial_calper(0.), Calib(0.), Calper(0.)
{
    init(seg_data_length, tstart, dt, calibration, calperiod);
}

/** Initialize.
 *  @param[in] seg_data_length The number of data samples.
 *  @param[in] tstart The time of the first data sample.
 *  @param[in] dt The sample time interval.
 *  @param[in] calib The calibration factor for this data.
 *  @param[in] calper The calibration period for this data.
 *  @throws GERROR_MALLOC_ERROR
 */
void GSegment::init(int seg_data_length, double tstart, double dt,
			double calibration, double calperiod) throw(int)
{
    beg = tstart;
    setTdel(dt);

    Calib = (calibration != 0.) ? calibration : 1.;
    Calper = calperiod;
    initial_calib = Calib;
    initial_calper = Calper;

    if(seg_data_length > 0) {
	data = (float *)malloc(seg_data_length*sizeof(float));
	data_length = seg_data_length;
    }
    else {
	data = (float *)malloc(sizeof(float));
	data_length = 0;
    }
    if(!data) {
	GError::setMessage("GSegment.init malloc failed.");
	cerr << GError::getMessage() << endl;
	throw GERROR_MALLOC_ERROR;
    }
}

/** Set the sample time interval of the segment.
 *  @param[in] dt the new sample time interval. It must be > 0.
 */
void GSegment::setTdel(double dt)
{
    if(dt <= 0.) {
	cerr <<  "GSegment.setTdel: invalid sample time interval." << endl;
	del = 1.;
    }
    else {
	del = dt;
    }
}

/** Creates a GSegment that is a subsegment of this GSegment.
 *  The subsegment begins at the specified begin_index and extends to
 *  the data at index end_index-1.
 *  @param[in] begin_index the beginning index. Must be >= 0 && < length().
 *  @param[in] end_index the ending index, exclusive. Must be >= 0 && <
 *   length() && > begin_index.
 *  throws GERROR_MALLOC_ERROR and GERROR_INVALID_ARGS
 */
GSegment * GSegment::subsegment(int begin_index, int end_index) throw(int)
{
    GSegment *seg;

    if(begin_index < 0 ) {
	GError::setMessage("GSegment.subsegment begin_index(%d) < 0.",
		begin_index);
	cerr << GError::getMessage() << endl;
	throw GERROR_INVALID_ARGS;
    }
    if(begin_index >= data_length) {
	GError::setMessage("GSegment.subsegment begin_index(%d) >= length().",
		begin_index);
	cerr << GError::getMessage() << endl;
	throw GERROR_INVALID_ARGS;
    }
    if(end_index <= begin_index) {
	GError::setMessage(
		"GSegment.subsegment end_index(%d) <= begin_index(%d).",
		begin_index, end_index);
	cerr << GError::getMessage() << endl;
	throw GERROR_INVALID_ARGS;
    }
    if(end_index > data_length) {
	GError::setMessage("GSegment.subsegment end_index(%d) > length().",
		end_index);
	cerr << GError::getMessage() << endl;
	throw GERROR_INVALID_ARGS;
    }

    seg = new GSegment(data+begin_index, end_index - begin_index,
		beg + begin_index*del, del, initial_calib, initial_calper);
    seg->Calib = Calib;
    seg->Calper = Calper;
    return seg;
}

GSegment::GSegment(GSegment &s) :
	data(NULL), beg(0.), del(0.), data_length(0), initial_calib(0.),
	initial_calper(0.), Calib(0.), Calper(0.)
{
    init(s.data_length, s.beg, s.del, s.initial_calib, s.initial_calper);
    Calib = s.Calib;
    Calper = s.Calper;
    if(s.data_length > 0) {
	memcpy(data, s.data, s.data_length*sizeof(float));
    }
}

GSegment::GSegment(GSegment *s) :
	data(NULL), beg(0.), del(0.), data_length(0), initial_calib(0.),
	initial_calper(0.), Calib(0.), Calper(0.)
{
    init(s->data_length, s->beg, s->del, s->initial_calib, s->initial_calper);
    Calib = s->Calib;
    Calper = s->Calper;
    if(s->data_length > 0) {
	memcpy(data, s->data, s->data_length*sizeof(float));
    }
}

GSegment & GSegment::operator=(const GSegment &s)
{
    if(this != &s) {
	if(data != NULL) free(data);
	init(s.data_length, s.beg, s.del, s.initial_calib, s.initial_calper);
	Calib = s.Calib;
	Calper = s.Calper;
	if(s.data_length > 0) {
	    memcpy(data, s.data, s.data_length*sizeof(float));
	}
    }
    return *this;
}

Gobject * GSegment::clone(void)
{
    GSegment *s = new GSegment(data, data_length, beg, del,
				initial_calib, initial_calper);
    s->Calib = Calib;
    s->Calper = Calper;
    return (Gobject *)s;

}

/** Destructor. */
GSegment::~GSegment(void)
{
    if(data != NULL) free(data);
}

/** Cut the beginning and ending of the data.
 *  Keep only data[i1] to data[i2], inclusive.
 *  @param[in] i1 the first sample to keep.
 *  @param[in] i2 the last sample to keep.
 *  throws GERROR_MALLOC_ERROR
 */
void GSegment::truncate(int i1, int i2) throw(int)
{
    if(i1 >= data_length || i2 < 0 || i1 > i2) return;
    if(i1 < 0) i1 = 0;
    if(i2 >= data_length) i2 = data_length-1;

    beg = beg + i1*del;
    data_length = i2 - i1 + 1;
    for(int i = i1, j = 0; i <= i2; i++) data[j++] = data[i];
    if(data_length > 0) {
	data = (float *)realloc(data, data_length*sizeof(float));
    }
    else {
	data = (float *)realloc(data, sizeof(float));
    }
    if( !data ) {
	GError::setMessage("GSegment.truncate realloc failed.");
cerr << GError::getMessage() << endl;
	throw GERROR_MALLOC_ERROR;
    }
}

/** Replace the data of this segment with the input array. The data is copied.
 *  The length of data[] must be >= length().
 *  @param[in] seg_data new data.
 */
void GSegment::setData(float *seg_data)
{
    if(data_length > 0) {
	memcpy(data, seg_data, data_length*sizeof(float));
    }
}

/** Set the calibration factor and the calibration period.
 *  @param[in] calib the new calibration factor. If calib is 0., it is set to 1.
 *  @param[in] calper the new calibration period.
 */
void GSegment::setCalibration(double calibration, double calperiod)
{
    Calib = (calibration != 0.) ? calibration : 1.;
    Calper = calperiod;
}
