/** \file QCData.cpp
 *  \brief Defines class QCData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>

#include "QCData.h"
#include "motif++/Parse.h"
#include "widget/Table.h"
using namespace Parse;

extern "C" {
#include "libdataqc.h"
#include "libstring.h"
}
static void interpolate_segment(float *data, int npts, int start, int end);

using namespace libgdataqc;

/**
 * QCData contructor. QCData is a GMethod object that cuts out parts of a
 * waveform that have been flagged as bad data by a QC operator.
 * @return a QCData object, or NULL if malloc fails (logErrorMsg will be called)
 */
QCData::QCData(void) : DataMethod("QCData")
{
    extended = true;
    qcdef.fix = 0;
    qcdef.ntaper = 10;
    qcdef.drop_thr = 8;
    qcdef.single_trace_spike_thr = 10.0;
    qcdef.niter = 1;
    qcdef.nsamp = 10;
    qcdef.nover = 8;
    qcdef.spike_thr = 10.0;
    qcdef.spike_stat = QC_SPIKE_STAT_AVG;
    qcdef.spike_val = 1.;
    qcdef.spike_npwin = 10;
    qcdef.spike_dset = QC_SPIKE_DSET_DATA;
}

QCData::QCData(bool ex, QCDef *q) : DataMethod("QCData")
{
    extended = ex;
    qcdef = *q;
}

QCData::QCData(const char *s) : DataMethod("QCData")
{
    init(s);
}

Gobject * QCData::clone()
{
    return (Gobject *) new QCData(extended, &qcdef);
}

void QCData::init(const char *s)
{
    string c;

    extended = true;
    qcdef.fix = 1;
    qcdef.ntaper = 10;
    qcdef.drop_thr = 8;
    qcdef.single_trace_spike_thr = 10.0;
    qcdef.niter = 1;
    qcdef.nsamp = 10;
    qcdef.nover = 8;
    qcdef.spike_thr = 10.0;
    qcdef.spike_stat = QC_SPIKE_STAT_AVG;
    qcdef.spike_val = 1.;
    qcdef.spike_npwin = 10;
    qcdef.spike_dset = QC_SPIKE_DSET_DATA;


    if(parseGetArg(s, "type", c)) {
	if(parseCompare(c, "basic")) {
	    extended = false;
	}
	else if(parseCompare(c, "extended")) {
	    extended = true;
	}
	else {
	    logErrorMsg(LOG_WARNING, "Invalid QCData type.");
	}
    }
    if(stringGetIntArg(s, "fix", &qcdef.fix) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData fix.");
    }
    if(stringGetIntArg(s, "ntaper", &qcdef.ntaper) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData ntaper.");
    }
    if(stringGetIntArg(s, "drop_thr", &qcdef.drop_thr) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData drop_thr.");
    }
    if(stringGetDoubleArg(s, "single_trace_spike_thr",
		&qcdef.single_trace_spike_thr) == -2)
    {
	logErrorMsg(LOG_WARNING, "Invalid single_trace_spike_thr.");
    }
    if(stringGetIntArg(s, "niter", &qcdef.niter) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData niter.");
    }
    if(stringGetIntArg(s, "nsamp", &qcdef.nsamp) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData nsamp.");
    }
    if(stringGetIntArg(s, "nover", &qcdef.nover) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData nover.");
    }
    if(stringGetDoubleArg(s, "spike_thr", &qcdef.spike_thr) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData spike_thr.");
    }
    if(parseGetArg(s, "spike_stat", c)) {
	if(parseCompare(c, "avg")) {
	    qcdef.spike_stat = QC_SPIKE_STAT_AVG;
	}
	else if(parseCompare(c, "per")) {
	    qcdef.spike_stat = QC_SPIKE_STAT_PER;
	}
	else {
	    logErrorMsg(LOG_WARNING, "Invalid QCData spike_stat.");
	}
    }
    if(stringGetDoubleArg(s, "spike_val", &qcdef.spike_val) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData spike_val.");
    }
    if(stringGetIntArg(s, "spike_npwin", &qcdef.spike_npwin) == -2) {
	logErrorMsg(LOG_WARNING, "Invalid QCData spike_npwin.");
    }
    if(parseGetArg(s, "spike_dset", c)) {
	if(parseCompare(c, "data")) {
	    qcdef.spike_dset = QC_SPIKE_DSET_DATA;
	}
	else if(parseCompare(c, "1diff")) {
	    qcdef.spike_dset = QC_SPIKE_DSET_1DIFF;
	}
	else if(parseCompare(c, "all")) {
	    qcdef.spike_dset = QC_SPIKE_DSET_ALL;
	}
	else {
	    logErrorMsg(LOG_WARNING, "Invalid QCData spike_dset.");
	}
    }
}

QCData::~QCData(void)
{
}

bool QCData::applyMethod(int num_waveforms, GTimeSeries **ts)
{
    if(num_waveforms <= 0) return True;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "QCData::applyMethod: ts=NULL");
	return false;
    }

    for(int i = 0; i < num_waveforms; i++)
    {
	applyqc(ts[i]);
    }
    return true;
}

void QCData::applyqc(GTimeSeries *ts, Table *table)
{
    int i, i1, j, k, n, npts, seglen;
    const char *row[9];
    char r[9][25];
    vector<int> ndex;
    float mean;
    double tbeg, tdel;
    QCMask *m;
    GSegment *s, *seg;
    GTimeSeries *t = NULL;

    /* Apply QCData. If all segments of ts[i] are ok, then the new
     * TimeSeries t will not be created. If any segment of ts[i] is
     * altered, t will be created.
     */
    if(table) {
	for(i = 0; i < 9; i++) row[i] = &r[i][0];
    }

    for(j = 0; j < ts->size(); j++)
    {
	if(table) {
	    strncpy(&r[0][0], ts->sta(), 25);
	    strncpy(&r[1][0], ts->chan(), 25);
	    snprintf(&r[2][0], 25, "%d", j+1);
	}
	s = ts->segment(j);
	n = s->length();
	if(extended) {
	    qc_extended(&s->data, &n, 1, &qcdef, QC_EXTENDED_SS, &m);
	}
	else {
	    qc_basic(&s->data, &n, 1, &qcdef, &m);
	}

	if(m->nseg > 0 && qcdef.fix) {
	    qc_mean(s->data, n, m, &mean);
	    qc_demean(s->data, n, m, (double)mean);
	    for (k = 0; k < m->nseg; k++)
	    {
		seglen = m->end[k] - m->start[k] + 1;

		if(table) {
		    snprintf(&r[3][0], 25, "%d", k+1);
		    tbeg = s->tbeg() + m->start[k]*s->tdel();
		    timeEpochToString(tbeg, &r[4][0], 25, YMONDHMS);
		    snprintf(&r[5][0], 25, "%d", m->start[k]);
		    snprintf(&r[6][0], 25, "%d", m->end[k]);
		    snprintf(&r[7][0], 25, "%d", seglen);
		}
		/* If the segment length is smaller than the threshold,
		 * interpolate the segment, otherwise fill it with zeroes.
		 */
		if(seglen < qcdef.drop_thr) {
 		    interpolate_segment(s->data, n, m->start[k], m->end[k]);
		    if(table) strcpy(&r[8][0], "interpolated");
		}
		else {
		    if(table) strcpy(&r[8][0], "segment gap");
		    ndex.push_back(m->start[k]-1); // end of last good data
		    ndex.push_back(m->end[k]+1); // beginning o f next good data
		}
		if(table) table->addRow(row, false);
	    }

	    if((int)ndex.size() > 0) {
		if(t == NULL) {
		    t = new GTimeSeries();
		    /* If this is not the first segment of ts,
		     * copy the previous segments.
		     */
		    for(i = 0; i < j; i++) {
			t->addSegment(ts->segment(i));
		    }
		}
		if(ndex[0] > 0) {
		    seg = new GSegment(s->data, ndex[0]+1, s->tbeg(),
				s->tdel(), s->calib(), s->calper());
		    t->addSegment(seg);
		}
		for(i = 1; i < (int)ndex.size()-1; i++) {
		    tbeg = s->tbeg() + ndex[i]*s->tdel();
		    npts = ndex[i+1] - ndex[i] + 1;
		    if(npts > 1) {
			seg = new GSegment(s->data+ndex[i], npts, tbeg,
				s->tdel(), s->calib(), s->calper());
			t->addSegment(seg);
		    }
		}
		if(ndex[i] < s->length()-1) {
		    tbeg = s->tbeg() + ndex[i]*s->tdel();
		    npts = s->length() - ndex[i];
		    seg = new GSegment(s->data+ndex[i], npts, tbeg,
				s->tdel(), s->calib(), s->calper());
		    t->addSegment(seg);
		}
	    }
	}
	else if(m->nseg > 0) {
	    if(t == NULL) {
		t = new GTimeSeries();
		/* If this is not the first segment of ts,
		 * copy the previous segments.
		 */
		for(k = 0; k < j; k++) {
		    t->addSegment(ts->segment(k));
		}
	    }
	    if(m->start[0] > 0) {
		seg = new GSegment(s->data, m->start[0],s->tbeg(),
					s->tdel(), s->calib(), s->calper());
		t->addSegment(seg);
	    }
	    for(k = 1; k < m->nseg; k++)
	    {
		npts = m->start[k] - m->end[k-1];
		if(npts > 0) {
		    i1 = m->end[k-1] + 1;
		    tbeg = s->tbeg() + i1*s->tdel();
		    seg = new GSegment(s->data+i1, npts, tbeg,
					s->tdel(), s->calib(), s->calper());
		    t->addSegment(seg);
		}
	    }
	    if(m->end[m->nseg-1]+1 < s->length()) {
		npts = s->length() - (m->end[m->nseg-1]+1);
		i1 = m->end[m->nseg-1] + 1;
		tbeg = s->tbeg() + i1*s->tdel();
		seg = new GSegment(s->data+i1, npts, tbeg,
					s->tdel(), s->calib(), s->calper());
		t->addSegment(seg);
	    }
	    if(table) {
		for(k = 0; k < m->nseg; k++)
		{
		    seglen = m->end[k] - m->start[k] + 1;
		    snprintf(&r[3][0], 20, "%d", k+1);
		    tbeg = s->tbeg() + m->start[k]*s->tdel();
		    timeEpochToString(tbeg, &r[4][0], 20, YMONDHMS);
		    snprintf(&r[5][0], 20, "%d", m->start[k]);
		    snprintf(&r[6][0], 20, "%d", m->end[k]);
		    snprintf(&r[7][0], 20, "%d", seglen);
		    strcpy(&r[8][0], "segment gap");
		    table->addRow(row, false);
		}
	    }
	}
	else if(t != NULL) {
	    t->addSegment(s);
	}
	free(m->start);
	free(m->end);
    }
    if(table) table->adjustColumns();

    if(t && t->size() == 0) {
	/* The entire waveform has been masked as bad. Return a two-
	 * point "null" waveform.
	 */
	float data[2] = {0., 0.};
	tbeg = ts->tbeg();
	tdel = ts->tend() - tbeg;
	t->addSegment(new GSegment(data, 2, tbeg, tdel, 1., 1.));
    }
    /* Replace the original TimeSeries, if t was created.
     */
    if(t) {
	ts->removeAllSegments();
	for(j = 0; j < t->size(); j++) ts->addSegment(t->segment(j));
	t->deleteObject();
    }
}

const char * QCData::toString(void)
{
    char c[200];
    snprintf(c, sizeof(c),
"QCData: fix=%d ntaper=%d drop_thr=%d single_trace_spike_th=%.2f niter=%d \
nsamp=%d nover=%d spike_thr=%.2f spike_stat=%d spike_val=%.2f spike_npwin=%d \
spike_dset=%d",
	qcdef.fix, qcdef.ntaper, qcdef.drop_thr, qcdef.single_trace_spike_thr,
	qcdef.niter, qcdef.nsamp, qcdef.nover, qcdef.spike_thr,
	qcdef.spike_stat, qcdef.spike_val, qcdef.spike_npwin, qcdef.spike_dset);
    string_rep.assign(c);
    return string_rep.c_str();
}

static void
interpolate_segment(float *data, int npts, int start, int end)
{
    float   *d = NULL;
    double  d0, dn, delta;
    int     i, seglen, s, e, nm1, interp;

    /*
     * We need one point beyond each end of the masked segment
     * to interpolate.  Check if we have them.  If we have none,
     * zero out segment.  If we only have a point before the
     * segment, set the segment value to this value.
     * Likewise, if we only have a point after the segment,
     * set the segment value to this value.
     */
    interp = 0;
    e = end + 1;
    s = start - 1;
    nm1 = npts - 1;
    d0 = 0.0;
    if((e > nm1) && (s < 0)) {
	d0 = 0.0;
    }
    else if(e > nm1) {
	d0 = data[s];
    }
    else if(s < 0) {
	d0 = data[e];
    }
    else {
	interp = 1;
    }

    /* If we can't interpolate, fill the segment with a constant */
    if(!interp) {
	for (i = start; i <= end; i++) data[i] = d0;
    }
    else {
	dn = data[e];
	d0 = data[s];
	seglen = e - s + 1;
	delta = (dn - d0) / (double) (seglen - 1);

	d = &data[s];
	for (i = 1; i < seglen - 1; i++) d[i] = d0 + delta * i;
    }
}
