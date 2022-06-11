/** \file OffsetData.cpp
 *  \brief Defines class OffsetData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "OffsetData.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libstring.h"
}

using namespace std;

/** Constructor with character string argument. The parameters offset
 * is input as a character string in the form "offset=value"
 * @throws GERROR_INVALID_ARGS if the offset parameter is not found or cannot
 *      be parsed as a valid double.
 */
OffsetData::OffsetData(const string &s) : DataMethod("OffsetData"), value(0.)
{
    if(stringGetDoubleArg(s.c_str(), "offset", &value)) {
	GError::setMessage("OffsetData constructor: invalid offset");
 	throw GERROR_INVALID_ARGS;
    }
}

/** Constructor with offset argument.
 *  @param[in] offset the GTimeSeries data values are offset by this value.
 */
OffsetData::OffsetData(double offset) : DataMethod("OffsetData"), value(0.)
{
    value = offset;
}

Gobject * OffsetData::clone()
{
    return (Gobject *) new OffsetData(value);
}

/** Destructor. */
OffsetData::~OffsetData(void)
{
}

bool OffsetData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "OffsetData.apply: ts=NULL");
	return false;
    }

    for(int i = 0; i < n; i++)
    {
	for(int j = 0; j < ts[i]->size(); j++) {
	    offsetSegment(ts[i]->segment(j), value);
	}
    }
    return true;
}

// static
/** Apply the offset to a GSegment.
 *  @param[in] s the GSegment
 *  @param[in] offset the GSegment data values are offset by this value.
 */
void OffsetData::offsetSegment(GSegment *s, double offset)
{
    int n = s->length();
    for(int i = 0; i < n; i++) {
	s->data[i] += offset;
    }
}

const char * OffsetData::toString(void)
{
    char c[100];
    snprintf(c, sizeof(c), "Offset: %.4e", value);
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create an OffsetData instance from a string representation. This
 *  function uses the string argument constructor to create the OffsetData
 *  from a string in the form produced by the toString() function.
 *  @param[in] args the parameters as a string.
 *  @returns an OffsetData instance.
 */
OffsetData * OffsetData::create(const string &args)
{
    return new OffsetData(args);
}
