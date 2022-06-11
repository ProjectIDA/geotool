/** \file CopyData.cpp
 *  \brief Defines class CopyData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "CopyData.h"
#include "gobject++/GTimeSeries.h"
extern "C" {
#include "libstring.h"
}

using namespace std;

/** Constructor with character string argument. The parameters are specified in
 *  the form "beg_select=value end_select=value".
 *  @throws GERROR_INVALID_ARGS if a parameter is not found or cannot
 *      be parsed as a valid double.
 */
CopyData::CopyData(const string &s) : DataMethod("CopyData"),
	beg_select(0.), end_select(0.)
{
    if(stringGetDoubleArg(s.c_str(), "beg_select", &beg_select)) {
	GError::setMessage("CopyData: invalid beg_select");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetDoubleArg(s.c_str(), "end_select", &end_select)) {
	GError::setMessage("CopyData: invalid end_select");
	throw GERROR_INVALID_ARGS;
    }
}

/** Constructor.
 *  @param start_time The beginning epochal time of the copy.
 *  @param end_time The ending epochal time of the copy.
 */
CopyData::CopyData(double start_time, double end_time) : DataMethod("CopyData"),
	beg_select(start_time), end_select(end_time)
{
}

Gobject * CopyData::clone()
{
    return (Gobject *) new CopyData(beg_select, end_select);
}

CopyData::~CopyData(void)
{
}

bool CopyData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "CopyData.apply: ts=NULL");
	return false;
    }

    if(end_select > beg_select)
    {
	for(int i = 0; i < n; i++) {
	    ts[i]->truncate(beg_select, end_select);
	}
    }
    /* if end_select == beg_select, use entire ts. no action required here.
     */
    return true;
}

const char * CopyData::toString(void)
{
    char c[100];
    snprintf(c, sizeof(c),
	"CopyData: beg_select=%.2f end_select=%.2f", beg_select, end_select);
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create a CopyData instance from a string representation. This
 *  function uses the string argument constructor to create the
 *  CopyData from a string in the form produced by the toString()
 *  function.
 *  @param[in] args the parameters as a string.
 *  @returns an CopyData instance.
 */
CopyData * CopyData::create(const string &args)
{
    return new CopyData(args);
}
