/** \file Demean.cpp
 *  \brief Defines class Demean.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "Demean.h"
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
Demean::Demean(void) : DataMethod("Demean")
{
}

Gobject * Demean::clone()
{
    return (Gobject *) new Demean();
}

/** Destructor. */
Demean::~Demean(void)
{
}

bool Demean::applyMethod(int n, GTimeSeries **ts)
{
    double mean;

    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "Demean.apply: ts=NULL");
	return false;
    }

    for(int k = 0; k < n; k++)
    {
	mean = ts[k]->mean();
	for(int j = 0; j < ts[k]->size(); j++) {
	    int npts = ts[k]->segment(j)->length();
	    for(int i = 0; i < npts; i++) {
		ts[k]->segment(j)->data[i] -= mean;
	    }
	}
    }
    return true;
}

const char * Demean::toString(void)
{
    char c[100];
    snprintf(c, sizeof(c), "Demean");
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create an Demean instance from a string representation. This
 *  function uses the string argument constructor to create the Demean
 *  from a string in the form produced by the toString() function.
 *  @param[in] args the parameters as a string.
 *  @returns an Demean instance.
 */
Demean * Demean::create(const string &args)
{
    return new Demean();
}

