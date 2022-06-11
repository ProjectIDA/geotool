/** \file TaperData.cpp
 *  \brief Defines class TaperData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>
#include <math.h>

#include "TaperData.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "tapers.h"
#include "libstring.h"
}

using namespace std;


/** A DataMethod for the application of a data taper. Several types of
 *  tapers are possible.
 *
 */

#ifndef PI
#define PI  3.14159265358979
#endif
#define TWO_PI (2.*PI)

/** Constructor with character string argument.  Create a TaperData object
 *  from a string description. The string description should contain the
 * parameter names and values in the form "name1=value1 name2=value". The order
 * of the name/value pairs is arbitrary. Example:
 * \code
 *     "type=cosine width=5 minpts=10 maxpts=100"
 * \endcode
 * @param[in] s the string taper description.
 * @throws GERROR_INVALID_ARGS
 */
TaperData::TaperData(const string &s) : DataMethod("TaperData"),
	width(0), minpts(0), maxpts(0)
{
    char *c;
    string taper_type;
    int taper_width, taper_minpts, taper_maxpts;

    if((c = stringGetArg(s.c_str(), "type")) != NULL) {
	if(strlen(c) >= 20) {
	    GError::setMessage("TaperData constructor: invalid type");
	    free(c);
	    throw GERROR_INVALID_ARGS;
	}
	taper_type.assign(c);
	free(c);
    }

    if(stringGetIntArg(s.c_str(), "width", &taper_width) == -2) {
	GError::setMessage("TaperData constructor: invalid width");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetIntArg(s.c_str(), "minpts", &taper_minpts) == -2) {
	GError::setMessage("TaperData constructor: invalid minpts");
	throw GERROR_INVALID_ARGS;
    }
    if(stringGetIntArg(s.c_str(), "maxpts", &taper_maxpts) == -2) {
	GError::setMessage("TaperData constructor: invalid maxpts");
	throw GERROR_INVALID_ARGS;
    }
    init(taper_type, taper_width, taper_minpts, taper_maxpts);
}

/** Constructor.
 *  @param[in] taper_type the type of taper. Valid types are
 *   - hamming
 *   - hanning
 *   - cosine  (apply a cosine taper to the beginning and end of the waveform)
 *   - cosineBeg (apply a cosine taper to the beginning only)
 *   - parzen
 *   - welch
 *   - blacknam
 *   - none
 *  @param[in] taper_width the width of the taper (as a % of the waveform
 *		length), if the type is "cosine" or "cosineBeg".
 *  @param[in] taper_minpts the minimum length of the taper, if the type is
 *		cosine or cosineBeg.
 *  @param[in] taper_maxpts The maximum length of the taper, if the type is
 *		cosine or cosineBeg.
 * @throws GERROR_INVALID_ARGS
 */
TaperData::TaperData(const string &taper_type, int taper_width,int taper_minpts,
		int taper_maxpts) : DataMethod("TaperData"),
		width(0), minpts(0), maxpts(0)
{
    init(taper_type, taper_width, taper_minpts, taper_maxpts);
}

Gobject * TaperData::clone(void)
{
    return (Gobject *) new TaperData(type, width, minpts, maxpts);
}

void TaperData::init(const string &taper_type, int taper_width, int taper_minpts,
		int taper_maxpts)
{
    if( taper_type.compare("hamming")  && taper_type.compare("hanning") && 
	taper_type.compare("cosine")   && taper_type.compare("cosineBeg") &&
	taper_type.compare("parzen")   && taper_type.compare("welch") &&
	taper_type.compare("blackman") && taper_type.compare("none"))
    {
	GError::setMessage("TaperData constructor: invalid Taper type.");
	throw GERROR_INVALID_ARGS;
    }
    type = taper_type;
    width = taper_width;
    minpts = taper_minpts;
    maxpts = taper_maxpts;
}

/** Destructor. */
TaperData::~TaperData(void)
{
}

bool TaperData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "TaperData.apply : ts=NULL");
	return false;
    }

    for(int i = 0; i < n; i++) {
	applyMethod(ts[i]);
    }
    return true;
}

bool TaperData::applyMethod(GTimeSeries *ts)
{	    
    int i;

    if(!type.compare("none")) {
	return false;
    }
    else if(!type.compare("hamming")) {
	for(i = 0; i < ts->size(); i++) {
	    Taper_hamm(ts->segment(i)->data, ts->segment(i)->length());
	}
    }
    else if(!type.compare("hanning")) {
	for(i = 0; i < ts->size(); i++) {
	    Taper_hann(ts->segment(i)->data, ts->segment(i)->length());
	}
    }
    else if(!type.compare("cosine") || !type.compare("cosineBeg"))
    {
	float wid = width/100., w_end;
	if(wid < 0.) wid = 0.;
	if(wid > 1.) wid = 1.;

	for(i = 0; i < ts->size(); i++)
	{
	    float w = wid;
	    /* this is the code which determines how many points will be
	     * used in the taper.  min of 5 points, max of 200 points,
	     * optimum is 5%
	     */
	    int npts = ts->segment(i)->length() * width;
	    if(maxpts > 0 && npts > maxpts) {
		w = (double) maxpts/ts->segment(i)->length();
	    }
	    else if (minpts > 0 && npts < minpts &&
			ts->segment(i)->length() > minpts)
	    {
		w = (double)minpts/ts->segment(i)->length();
	    }
	    w_end = (!type.compare("cosine")) ? w : 0.;
	    Taper_cosine(ts->segment(i)->data, ts->segment(i)->length(), w,
				w_end);
	}
    }
    else if(!type.compare("parzen")) {
	for(i = 0; i < ts->size(); i++) {
	    Taper_parzen(ts->segment(i)->data, ts->segment(i)->length());
	}
    }
    else if(!type.compare("welch")) {
	for(i = 0; i < ts->size(); i++) {
	    Taper_welch(ts->segment(i)->data, ts->segment(i)->length());
	}
    }
    else if(!type.compare("blackman")) {
	for(i = 0; i < ts->size(); i++) {
	    Taper_blackman(ts->segment(i)->data, ts->segment(i)->length());
	}
    }
    return true;
}

/* 
 *  Return a string representation of this taper.
 */
const char * TaperData::toString(void)
{
    char c[100];
    snprintf(c, sizeof(c), "TaperData: type=%s width=%d%% minpts=%d maxpts=%d",
		type.c_str(), width, minpts, maxpts);
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Create a TaperData instance from a string representation. This
 *  function uses the string argument constructor to create the
 *  TaperData from a string in the form produced by the toString()
 *  function.
 *  @param[in] args the parameters as a string.
 *  @returns an TaperData instance.
 */
TaperData * TaperData::create(const string &args)
{
    return new TaperData(args);
}
