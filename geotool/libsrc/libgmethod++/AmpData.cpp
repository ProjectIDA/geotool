/** \file AmpData.cpp
 *  \brief Defines class AmpData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "AmpData.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libstring.h"
}

using namespace std;

/** Constructor with character string argument. The parameters factor and
 *  comment are input as a character string in the form "factor=value
 *  comment=value". The comment is optional.
 *  @param s the arguments as a character string.
 *  @throws GERROR_INVALID_ARGS if the factor parameter is not found or cannot
 *	be parsed as a valid double.
 */
AmpData::AmpData(const string &s) throw(int) : DataMethod("AmpData"),
		factor(0.), comment()
{
    char *c;
    if(stringGetDoubleArg(s.c_str(), "factor", &factor)) {
	GError::setMessage("AmpData constructor: invalid or missing factor.");
	throw GERROR_INVALID_ARGS;
    }
    if( (c= stringGetArg(s.c_str(), "comment")) ) {
	comment.assign(c);
	free(c);
    }
}

Gobject * AmpData::clone()
{
    return (Gobject *) new AmpData(factor, comment);
}

/** Destructor. */
AmpData::~AmpData(void)
{
}

bool AmpData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "AmpData.apply: ts=NULL");
	return false;
    }

    for(int i = 0; i < n; i++)
    {
	for(int j = 0; j < ts[i]->size(); j++) {
	    ampSegment(ts[i]->segment(j), factor);
	}
    }
    return true;
}

/** Apply the factor to a GSegment.
 *  @param[in] s the GSegment
 *  @param[in] factor the GSegment data values are multiplied by this value.
 */
void AmpData::ampSegment(GSegment *s, double factor)
{
    int n = s->length();
    for(int i = 0; i < n; i++) {
	s->data[i] *= factor;
    }
}

const char * AmpData::toString(void)
{
    char c[100];
    if(factor == -1.) {
	snprintf(c, sizeof(c), "Polarity factor = -1");
    }
    else {
	snprintf(c, sizeof(c), "AmpData: factor = %.4e %s",
			factor, comment.c_str());
    }
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create an AmpData instance from a string representation. This
 *  function uses the string argument constructor to create the
 *  AmpData from a string in the form produced by the toString()
 *  function.
 *  @param[in] args the parameters as a string.
 *  @returns an AmpData instance.
 */
AmpData * AmpData::create(const string &args)
{
    return new AmpData(args);
}
