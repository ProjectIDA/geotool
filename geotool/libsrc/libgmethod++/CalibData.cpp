/** \file CalibData.cpp
 *  \brief Defines class CalibData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "CalibData.h"
#include "gobject++/GTimeSeries.h"

using namespace std;

/** Constructor with character string argument. Since there are no parameters
 *  for the CalibData method, the input string can be anything.
 *  @param[in] s not used.
 */
CalibData::CalibData(const string &s) : DataMethod("CalibData")
{
    // nothing to do.
}

/** Constructor. */
CalibData::CalibData(void) : DataMethod("CalibData")
{
}

Gobject * CalibData::clone()
{
    return (Gobject *) new CalibData();
}

/** Destructor. */
CalibData::~CalibData(void)
{
}

bool CalibData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "CalibData.apply: ts=NULL");
	return false;
    }

    for(int i = 0; i < n; i++)
    {
	for(int j = 0; j < ts[i]->size(); j++) {
	    calibSegment(ts[i]->segment(j));
	}
    }
    return true;
}

// static
/** Apply the calibration factor to a GSegment.
 *  @param[in] s the GSegment
 */
void CalibData::calibSegment(GSegment *s)
{
    double cal = s->calib();
    if(cal != 0.0 && cal != 1.0) {
	int n = s->length();
	float *d = s->data;
	for(int i = 0; i < n; i++, d++) {
	    *d *= cal;
	}
    }
}

const char * CalibData::toString()
{
    char c[100];
    snprintf(c, sizeof(c), "Calibration factor applied.");
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create an CalibData instance from a string representation. This
 *  function uses the string argument constructor to create the
 *  CalibData from a string in the form produced by the toString()
 *  function.
 *  @param[in] args the string. Since CalibData has no parameters, the string
 * 	is not used.
 *  @returns an CalibData instance.
 */
CalibData * CalibData::create(const string &args)
{
    return new CalibData(args);
}
