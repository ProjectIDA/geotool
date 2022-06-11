/** \file CutData.cpp
 *  \brief Defines class CutData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "CutData.h"
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
CutData::CutData(const string &s) : DataMethod("CutData"),
	beg_select(0.), end_select(0.)
{
    if(stringGetDoubleArg(s.c_str(), "beg_select", &beg_select)) {
	GError::setMessage("CutData: invalid beg_select");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetDoubleArg(s.c_str(), "end_select", &end_select)) {
	GError::setMessage("CutData: invalid end_select");
	throw GERROR_INVALID_ARGS;
    }
}

/** Constructor.
 *  @param start_time The beginning epochal time of the cut.
 *  @param end_time The ending epochal time of the cut.
 */
CutData::CutData(double start_time, double end_time) : DataMethod("CutData"),
	beg_select(start_time), end_select(end_time)
{
}

Gobject * CutData::clone()
{
    return (Gobject *) new CutData(beg_select, end_select);
}

CutData::~CutData(void)
{
}

bool CutData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "CutData.apply: ts=NULL");
	return false;
    }

    if(end_select > beg_select)
    {
	for(int i = 0; i < n; i++)
	{
	    double tbeg = ts[i]->tbeg();
	    double tend = ts[i]->tend();
	    GTimeSeries *t;

	    t = ts[i]->subseries(tbeg, beg_select);
	    if(t) {
		t->join(ts[i]->subseries(end_select, tend));
	    }
	    else {
		t = ts[i]->subseries(end_select, tend);
	    }
	    if(!t) return false; /* ?? what happened to it. */
	    ts[i] = t;
	}
    }
    return true;
}

const char * CutData::toString(void)
{
    char c[100];
    snprintf(c, sizeof(c),
	"CutData: beg_select=%.2f end_select=%.2f", beg_select, end_select);
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create a CutData instance from a string representation. This
 *  function uses the string argument constructor to create the
 *  CutData from a string in the form produced by the toString()
 *  function.
 *  @param[in] args the parameters as a string.
 *  @returns an CutData instance.
 */
CutData * CutData::create(const string &args)
{
    return new CutData(args);
}
