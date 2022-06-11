#ifndef _TIME_SERIES_H_
#define _TIME_SERIES_H_

#include "gobject/GObject.h"
#include "gobject/Segment.h"
#include "gobject/Hashtable.h"
#include "gobject/CssIO.h"

/**
 *  A GObject that holds waveform data. The time series can contain
 *  multiple segments that are separated by gaps and/or have different
 *  sample intervals.
 *
 *  @see new_TimeSeries
 *  @see new_Segment
 *  @see new_GObjectPart
 */

/** 
 *  A GObject that represents waveform data.
 *
 *  @member npts The total number of sample in this TimeSeries.
 *  @member s An array of Segment objects.
 *  @member size The number of Segment objects in s.
 *  @member tdel_tolerance Determines if adjacent segments will be joined. \
 *	    If the percent change in tdel is < tdel_tolerance (fabs(s2.tdel - \
 *          s1.tdel)/s1.tdel < tdel_tolerance) and the percent change in the \
 *          gap between segments is < tdel_tolerance (fabs(s2.tbeg - s1.tbeg \
 *	    + s1.data_length*s1.tdel)/s1.tdel), two adjacent segments will be \
 *	    combined into one segment. Otherwise, the two segments will not \
 *	    be joined.
 *  @member hashtable Holds information about the TimeSeries.
 *  @see Segment
 *  @see GObjectPart
 *  @see Hashtable
 */
typedef struct
{
	GObjectPart	core;

	int		npts;
	Segment		*s;
	int		size;
	double		tdel_tolerance;
	double		calib_tolerance;
	Hashtable	hashtable;
} _TimeSeries, *TimeSeries;

#define getFloat(s,a) ((Float)Hashtable_get(s->hashtable,a))
#define getCssTable(ts) ((CssTable)Hashtable_get(ts->hashtable,"CssTable"))
#define putCssTable(ts,a) Hashtable_put(ts->hashtable, "CssTable", (GObject)a)

TimeSeries new_TimeSeries(void);
TimeSeries new_TimeSeries_sl(Segment *s, int s_length);
TimeSeries new_TimeSeries_s(Segment s);
void TimeSeries_addSegment(TimeSeries ts, Segment newSeg);
void TimeSeries_addSegmentJoin(TimeSeries ts, Segment newSeg, bool join);
void TimeSeries_fillAllGaps(TimeSeries ts, int maxGap);
void TimeSeries_fillGap(TimeSeries ts, int segment_index, int maxGap);
void TimeSeries_splineSegments(TimeSeries ts, double maxDtChange, int maxGap);
void TimeSeries_join(TimeSeries ts, TimeSeries t);
bool TimeSeries_continuous(TimeSeries ts, int i, double tbegTol,
			double tdelTol);
void TimeSeries_copyInto(TimeSeries ts, float *data);
double TimeSeries_dataMax(TimeSeries ts);
double TimeSeries_dataMin(TimeSeries ts);
double TimeSeries_mean(TimeSeries ts);
Segment TimeSeries_segment(TimeSeries ts, double time);
Segment TimeSeries_nearestSegment(TimeSeries ts, double time);
TimeSeries TimeSeries_subseries(TimeSeries ts, double t1, double t2);
bool TimeSeries_truncate(TimeSeries ts, double t1, double t2);
void TimeSeries_setData(TimeSeries ts, float *data);
void TimeSeries_setData_t(TimeSeries ts, TimeSeries t);
void TimeSeries_removeAllSegments(TimeSeries ts);
void TimeSeries_setHashtable(TimeSeries ts, Hashtable h);
void TimeSeries_saveSegmentHistory(TimeSeries ts, bool save);
Segment TimeSeries_getSegment(TimeSeries ts, int index);
double TimeSeries_time(TimeSeries ts, int index);
TimeSeries SP_decimate(TimeSeries ts, int rate, bool clone_hashtable);
void SP_decimateData(float *data, int rate, int n, int remainder, float *sdata);
double SP_demean(TimeSeries ts);

#define TimeSeries_tbeg(ts) \
((ts->s != NULL && ts->size > 0) ? ts->s[0]->tbeg : 0.)

#define TimeSeries_tend(ts) \
( (ts->s!=NULL && ts->size>0) ? Segment_tend(ts->s[ts->size-1]) : 0. )

#ifndef _DATA_POINT_H_
#include "gobject/DataPoint.h"
DataPoint TimeSeries_minPoint(TimeSeries ts);
DataPoint TimeSeries_maxPoint(TimeSeries ts);
DataPoint TimeSeries_lowerBound(TimeSeries ts, double time);
DataPoint TimeSeries_nearest(TimeSeries ts, double time);
DataPoint TimeSeries_upperBound(TimeSeries ts, double time);
int TimeSeries_flagged_arrays(TimeSeries ts, DataPoint d1, DataPoint d2,
			float **pt, float **py);
bool TimeSeries_dataMinMax(TimeSeries ts, DataPoint d1, DataPoint d2,
                        double *min, double *max);

#endif

int TimeSeries_putValue(TimeSeries ts, const char *name,
			const char *member_type, void *member_addr);
int TimeSeries_getValue(TimeSeries ts, const char *name,
			const char *member_type, void *member_addr);
int TimeSeries_putBlock(TimeSeries ts, const char *name,
		const char *member_type, int member_size, void *member_addr);
int TimeSeries_getStringValue(TimeSeries ts, const char *name,
			const char *spec, char *buf, int buflen);
WfdiscIO TimeSeries_getWfdiscIO(TimeSeries ts);

/* tsCorrelate.c */
TimeSeries tsCorrelate(TimeSeries ref, TimeSeries target, int rank_order);
TimeSeries tsfCorrelate(TimeSeries ref, TimeSeries target, int ends);


#endif
