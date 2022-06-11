/** \file RmsData.cpp
 *  \brief Defines class RmsData.
 */
#include "config.h"
#include <iostream>
#include <sstream>
#include <math.h>

#include "RmsData.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
}

using namespace std;
using namespace libgdataqc;

/** Constructor with character string argument. The parameters offset is input
 *  as a character string in the form
 * "window_length=value start_time=value end_time=value".
 * @param[in] s the string arguments.
 * @throws GERROR_INVALID_ARGS if the offset parameter is not found or cannot
 *      be parsed as a valid double.
 */
RmsData::RmsData(const char *s) : DataMethod("RmsData")
{
    if(stringGetIntArg(s, "window_length", &window_length)) {
	GError::setMessage("RmsData constructor: invalid window_length");
 	throw GERROR_INVALID_ARGS;
    }
    if(window_length != 2*(window_length/2)) window_length++;

    if(stringGetDoubleArg(s, "start_time", &start_time)) {
	GError::setMessage("RmsData constructor: invalid start_time");
 	throw GERROR_INVALID_ARGS;
    }
    if(stringGetDoubleArg(s, "end_time", &end_time)) {
	GError::setMessage("RmsData constructor: invalid end_time");
 	throw GERROR_INVALID_ARGS;
    }
}

/** Constructor with arguments.
 *  @param[in] wlength the window length in samples.
 *  @param[in] stime the start time of the data to be processed.
 *  @param[in] etime the end time of the data to be processed.
 */
RmsData::RmsData(int wlength, double stime, double etime)
		: DataMethod("RmsData")
{
    if(wlength != 2*(wlength/2)) wlength++;
    window_length = wlength;
    start_time = stime;
    end_time = etime;
}

Gobject * RmsData::clone()
{
    return (Gobject *) new RmsData(window_length, start_time, end_time);
}

/** Destructor. */
RmsData::~RmsData(void)
{
}

bool RmsData::applyMethod(int num_waveforms, GTimeSeries **ts)
{
    if(num_waveforms <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "RmsData.apply: ts=NULL");
	return false;
    }
    float *data = NULL, *data_out = NULL;

    for(int l = 0; l < num_waveforms; l++)
    {
	GDataPoint *beg = ts[l]->upperBound(start_time);
	GDataPoint *end = ts[l]->lowerBound(end_time);
	int i2 = (end->segmentIndex() == beg->segmentIndex()) ?
			end->index() : beg->segment()->length()-1;
	int npts = i2 - beg->index() + 1;
	for(int j = beg->segmentIndex()+1; j < end->segmentIndex(); j++) {
	    npts += ts[l]->segment(j)->length();
	}
	if(end->segmentIndex() > beg->segmentIndex()) {
	    npts += end->segmentIndex()+1;
	}
	if(!data) {
	    data = (float *)mallocWarn(npts*sizeof(float));
	    data_out = (float *)mallocWarn(npts*sizeof(float));
	}
	else {
	    data = (float *)reallocWarn(data, npts*sizeof(float));
	    data_out = (float *)reallocWarn(data, npts*sizeof(float));
	}

	int k = 0;
	for(int i = beg->index(); i <= i2; i++) {
	    data[k++] = beg->segment()->data[i];
	}
	for(int j = beg->segmentIndex()+1; j < end->segmentIndex(); j++) {
	    for(int i = 0; i < ts[l]->segment(j)->length(); i++) {
		data[k++] = ts[l]->segment(j)->data[i];
	    }
	}
	if(end->segmentIndex() > beg->segmentIndex()) {
	    for(int i = 0; i <= end->segmentIndex(); i++) {
		data[k++] = end->segment()->data[i];
	    }
	}

	rmsData(data, data_out, npts);

	k = 0;
	for(int i = beg->index(); i <= i2; i++) {
	    beg->segment()->data[i] = data_out[k++];
	}
	for(int j = beg->segmentIndex()+1; j < end->segmentIndex(); j++) {
	    for(int i = 0; i < ts[l]->segment(j)->length(); i++) {
		ts[l]->segment(j)->data[i] = data_out[k++];
	    }
	}
	if(end->segmentIndex() > beg->segmentIndex()) {
	    for(int i = 0; i <= end->segmentIndex(); i++) {
		end->segment()->data[i] = data_out[k++];
	    }
	}

	beg->deleteObject();
	end->deleteObject();
    }
    Free(data);
    Free(data_out);

    return true;
}

// static
/** Apply the rms to a data array.
 *  @param[in] data
 *  @param[in,out] data_out
 *  @param[in] npts the length of data[] and data_out[]
 */
void RmsData::rmsData(float *data, float *data_out, int npts)
{
    if(npts <= 0) return;

    double mean = 0;
    for(int i = 0; i < npts; i++) mean += data[i];
    mean /= npts;

    int m = window_length/2;
    int i1 = window_length - window_length/2 - 1;
    int i2 = npts - i1 -1;

    for(int i = 0; i < i1; i++) data_out[i] = data[i];
    for(int i = i1; i <= i2; i++) {
	double rms = 0.;
	for(int j = 0; j < window_length; j++) {
	    rms += (data[i-m+j] - mean)*(data[i-m+j] - mean);
	}
	data_out[i] = sqrt(rms/window_length);
    }
    for(int i = i2+1; i < npts; i++) data_out[i] = data[i];
}

const char * RmsData::toString(void)
{
    char start[100], end[100], c[200];
    timeEpochToString(start_time, start, sizeof(start), YMONDHMS);
    timeEpochToString(end_time, end, sizeof(end), YMONDHMS);
    snprintf(c, sizeof(c),
	"RMS: window_length: %d start: %s end: %s", window_length, start, end);
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create an RmsData instance from a string representation. This
 *  function uses the string argument constructor to create the RmsData
 *  from a string in the form produced by the toString() function.
 *  @param[in] args the parameters as a string.
 *  @returns an RmsData instance.
 */
RmsData * RmsData::create(const char *args)
{
    return new RmsData(args);
}

