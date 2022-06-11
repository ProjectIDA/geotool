#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "gobject/GObject.h"

/**
 *  A GObject that represents a segment of waveform data.
 *
 *  @see new_Segment
 *  @see new_GObjectPart
 */

/** 
 *  A GObject that represents a segment of waveform data.
 *
 *  @member tbeg The epoch time of the first sample.
 *  @member tdel The sample interval.
 *  @member data The array of data values.
 *  @member data_length The number of samples in data[].
 *  @member calib The data calibration factor.
 *  @member calper The data calibration period.
 */
typedef struct
{
	GObjectPart	core;

	double		tbeg;		/* time of the first data sample */
	double		tdel;		/* time increment */
	float		*data;		/* data samples */
	int		data_length;	/* number of samples in this segment */
	double		calib;		/* calibration factor */
	double		calper;		/* calibration period */
} _Segment, *Segment;

Segment new_Segment_n(int initialLength, double tbeg, double tdel,
			double calib, double calper);
Segment new_Segment(float *data, int data_length, double tbeg, double tdel,
			double calib, double calper);
Segment new_Segment_d(double *data, int data_length, double tbeg,
			double tdel, double calib, double calper);
Segment Segment_subsegment(Segment s, int beginIndex, int endIndex);
void _Segment_truncate(Segment s, int i1, int i2);
void Segment_setData(Segment s, float *data);
void Segment_setCalibration(Segment s, double calib, double calper);
#define Segment_tbeg(s) (s->tbeg)
#define Segment_tdel(s) (s->tdel)
#define Segment_tend(s) (s->tbeg + (s->data_length-1)*s->tdel)
#define Segment_time(s,i) (s->tbeg + (i)*s->tdel)

#endif
