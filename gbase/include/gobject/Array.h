/*	SccsId:	%W%	%G%	*/

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "gobject/GObject.h"

/** 
 *  A GObject for holding 2-D arrays. This structure holds a two-dimensional
 *  array of data values that represent multiple time series that have the
 *  same beginning time, sample interval and duration. "size" time series,
 *  each of length "data_length" are stored in data[size][data_length].
 *
 *  @see new_Array
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/

/** 
 *  A GObject for holding 2-D arrays.
 *
 *  @member data A two dimensional array data[size][data_length].
 *  @member data_length The length of the time series.
 *  @member size The number of time series.
 *  @member tbeg The beginning time of the data.
 *  @member tend The ending time of the data.
 *  @member tdel The time interval of the data.
 */
typedef struct
{
	GObjectPart		core;

	/*  Array members */
	float			**data; 
	int			data_length;
	int			size;
	double			tbeg;
	double			tend;
	double			tdel;
} _Array, *Array;

Array new_Array(int data_length, double tbeg, double tdel);
int Array_add(Array a, float *data);

#endif
