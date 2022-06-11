/** \file GArray.cpp
 *  \brief Defines class GArray.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "gobject++/GArray.h"

/** 
 *  Constructor. A GArray holds one or more float[] arrays. The function
 *  GArray.add is called to add arrays.  All array lengths must be the same.
 *  @param[in] len The length of the arrays that will be added.
 *  @param[in] tstart The start time of the arrays that will be added.
 *  @param[in] dt The sample time interval.
 */
GArray::GArray(int len, double tstart, double dt) : data(NULL),
	data_length(len), num(0), beg(tstart), end(tstart+(len-1)*dt), del(dt)
{
    data = (float **)malloc(sizeof(float *));
    if( !data ) {
	GError::setMessage("GArray: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
}

/** Add an array to an GArray object.
 *  @param f a float[] array of length a->data_length.
 */
void GArray::add(float *f)
{
    if(!(data = (float **)realloc(data, (num+1)*sizeof(float *)))) {
	GError::setMessage("GArray.add: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
    if(!(data[num] = (float *)malloc(data_length*sizeof(float)))) {
	GError::setMessage("GArray.add: malloc failed.");
	Free(data);
	throw GERROR_MALLOC_ERROR;
    }
    memcpy(data[num], f, data_length*sizeof(float));
    num++;
}

Gobject * GArray::clone(void)
{
    GArray *array;

    array = new GArray(data_length, beg, del);
    if(!array) return NULL;

    for(int i = 0; i < num; i++) {
	array->add(data[i]);
    }
    return (Gobject *)array;
}

GArray::~GArray(void)
{
    for(int i = 0; i < num; i++) free(data[i]);
    free(data);
}
