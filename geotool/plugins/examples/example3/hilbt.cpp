/** \file hilbt.cpp
 *  \brief Defines class hilbert.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include "hilbt.h"
#include "gobject++/GTimeSeries.h"
extern "C" {
#include "libmath.h"
}

hilbert::hilbert(void) : DataMethod("hilbert")
{
}

hilbert::~hilbert(void)
{
}

Gobject * hilbert::clone()
{
    return (Gobject *) new hilbert();
}

bool hilbert::applyMethod(int num_waveforms, GTimeSeries **ts)
{
    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "hilbert.apply: ts=NULL");
	return false;
    }
    for(int i = 0; i < num_waveforms; i++)
    {
	for(int j = 0; j < ts[i]->size(); j++) {
	    applySegment(ts[i]->segment(j));
	}
    }
    return true;
}

// static
void hilbert::applySegment(GSegment *s)
{
    Hilbert_data(s->length(), s->data);
}

const char * hilbert::toString(void)
{
    string_rep.assign("hilbert transform.");
    return string_rep.c_str();
}
