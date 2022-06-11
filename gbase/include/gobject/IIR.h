#ifndef _IIR_H_
#define _IIR_H_

#include "gobject/GMethod.h"

/**
 *  A GMethod object that specifies an IIR filter. This object contains all the
 *  parameters needed to completely specify an IIR filter. This includes
 *  the coefficients from the last data window needed to continue the
 *  recursive filtering on additional contiguous data.
 *
 *  @see new_Hashtable
 *  @see Vector
 *  @see new_GObjectPart
 *  @see new_GMethodPart
 */

/** 
 *  A GMethod object that specifies an IIR filter.
 *
 *  @member order The filter order.
 *  @member type The filter type (LP, HP, BP or BR).
 *  @member flow The low frequency filter cutoff (Hz).
 *  @member fhigh The high frequency filter cutoff (Hz).
 *  @member tdel The sample interval of the data.
 *  @member zero_phase 1 for zero phase filtering or 0 or single pass filtering.
 *  @member nsects The number of second-order sections.
 *  @member sn The numerator polynomials for second order sections.
 *  @member sd The denominator polynomials for second order sections
 *  @member x1 nsects recursion coeficients (initially zero)
 *  @member x2 nsects recursion coeficients (initially zero)
 *  @member y1 nsects recursion coeficients (initially zero)
 *  @member y2 nsects recursion coeficients (initially zero)
 */
typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	int			order;
	char			type[3];
	double			flow;
	double			fhigh;
	double			tdel;
	int			zero_phase;
	int			nsects;
	double			*sn, *sd;
	double			*x1, *x2, *y1, *y2;
} _IIR, *IIR;

IIR new_IIR_fromString(double tdel, const char *s);
IIR new_IIR(int iord, const char *type, double fl, double fh, double tdel,
			int zero_phase);
void IIR_filter_ts(IIR iir, TimeSeries ts);
void IIR_filter_segment(IIR iir, Segment s, int Reset);
void IIR_filter_data(IIR iir, float *data, int data_length, int Reset);
GMethod IIR_methodCreate(TimeSeries ts, const char *s);

#endif
