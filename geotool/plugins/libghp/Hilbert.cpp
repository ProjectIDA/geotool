/** \file Hilbert.cpp
 *  \brief Defines class Hilbert.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include "Hilbert.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libgmath.h"
}

namespace libghp {

/**
 *
 *
 */
Hilbert::Hilbert(void) : DataMethod("Hilbert")
{
}

Hilbert::~Hilbert(void)
{
}

Gobject * Hilbert::clone()
{
    return (Gobject *) new Hilbert();
}

bool Hilbert::applyMethod(int num_waveforms, GTimeSeries **ts)
{
    int i, j;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "Hilbert.apply: ts=NULL");
	return false;
    }
    for(i = 0; i < num_waveforms; i++)
    {
	for(j = 0; j < ts[i]->size(); j++) {
	    applySegment(ts[i]->segment(j));
	}
    }
    return true;
}

// static
void Hilbert::applySegment(GSegment *s)
{
    Hilbert_data(s->length(), s->data);
}

const char * Hilbert::toString(void)
{
    string_rep.assign("Hilbert transform.");
    return string_rep.c_str();
}

} // namespace libghp
