#ifndef _Rotate_H_
#define _Rotate_H_

#include "gobject/GMethod.h"

/**
 *  A GMethod object that specifies a rotation of two-component data.
 *
 *  @see new_Rotate
 *  @see TimeSeries
 *  @see new_GObjectPart
 *  @see new_GMethodPart
 */

/** 
 *  A GMethod object that specifies a rotation of two-component data.
 *  This object is stored in the hashtable of a TimeSeries object after
 *  a rotation has been applied. The mate member is the other component
 *  TimeSeries that was used to rotate.
 *
 *  @member angle The angle of the rotation in degrees.
 *  @member TimeSeries The other component TimeSeries of the rotation.
 *  @member mate_is_x True is the mate is the x-component (usually East), False otherwise.
 */
typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	double			angle;
	TimeSeries		mate;
	bool			mate_is_x;
} _Rotate, *Rotate;

Rotate new_Rotate(double angle, TimeSeries mate, bool mate_is_x);
void Rotate_timeSeries(TimeSeries ts1, TimeSeries ts2, double angle,int update);
void Rotate_data(float *x, float *y, int npts, double angle);
int Rotate_max(TimeSeries *ts, double tmin, double tmax, double *max_theta);

#endif
