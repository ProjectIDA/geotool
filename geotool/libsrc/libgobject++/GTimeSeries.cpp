#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#ifdef HAVE_GSL
#include <gsl/gsl_spline.h>
#endif

#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libtime.h"
#include "fnan.h"
static int sortSegments(const void *A, const void *B);
}

/*  A class that holds waveform data as a sequence of data segments. The
 *  GTimeSeries object is built up from GSegment objects. At any time, a new
 *  GSegment object can be added to the GTimeSeries object. Internally, two
 *  adjacent GSegment objects are joined into one GSegment object, if the data
 *  gap between them is equal to one sample time interval, and the adjacent
 *  GSegment objects have the same sample time interval, the same calibration
 *  factor, and the same calibration period. The data from a new GSegment
 *  object replaces any data that it overlaps.
 *
 *  The sample time interval tolerance, tdel_tolerance, determines if
 *  adjacent segments will be joined when their sample time intervals, tdel,
 *  are not the same, or the gap between the segments is not exactly tdel. If
 *  the percent change in tdel is < tdel_tolerance
 *  \code
 *  (fabs(s2->tdel() - s1->tdel())/s1->tdel() < tdel_tolerance)
 *  \endcode
 *  and the percent change in the gap between segments is < tdel_tolerance
 *  \code
 *  (fabs(s2->tbeg() - s1->tbeg() + s1->length()*s1->tdel())/s1->tdel()) < tdel_tolerance
 *  \endcode
 *   the two adjacent segments will be combined into one segment. Otherwise,
 *   the two segments will not be joined. The default value of tdel_tolerance is
 *   0.99.
 *
 *  The calibration factor tolerance, calib_tolerance, also determines if
 *  adjacent segments will be joined when their calibration factors are not the
 *  same. To adjacent segments will be joined only if the percent change in
 *  the calibration factor is less than calib_tolerance. The default value of
 *  calib_tolerance is .01.
 */

static void decimateData(float *data, int rate, int n, int remainder,
		float *sdata);

/** Construct an empty GTimeSeries.
 */
GTimeSeries::GTimeSeries(void) :
	response_file(NULL), response(), no_response(false), inid(-1),
	beam_elements(), waveform_io(NULL), array_elements(), tag(),
	source_info(), npts(0), s(NULL), nsegs(0), tdel_tolerance(0.99),
	calib_tolerance(.01), time_tolerance(.01), copy(NULL),
	shashtable(), dhashtable(), ihashtable(), lhashtable(),
	vhashtable(), station_name(), channel_name(),
	network_name(), refsta_name(), x_chan_name(), y_chan_name(),
	z_chan_name(), instrument_code(), seg_type(), data_type(), clip_type(),
	station_chanid(-1), station_lat(-999.), station_lon(-999.),
	station_elev(-999.), station_dnorth(0.0), station_deast(0.0),
	station_hang(-999.), station_vang(-999.), station_alpha(0.),
	station_beta(0.), station_gamma(0.), current_alpha(0.),
	current_beta(0.), current_gamma(0.), component_code(0),
	original_tbeg(NULL_TIME), original_tend(NULL_TIME),
	selection_start(NULL_TIME), selection_end(NULL_TIME),
	julian_date(-1), derived(false), data_methods(), _data_source(NULL)
{
    init();
}

/** Construct an empty GTimeSeries.
 *  @param[in] sample_time_tolerance the sample time interval tolerance.
 *  @param[in] calibration_tolerance the calibration factor tolerance.
 */
GTimeSeries::GTimeSeries(double sample_time_tolerance,
			double calibration_tolerance) :
	response_file(NULL), response(), no_response(false), inid(-1),
	beam_elements(), waveform_io(NULL), array_elements(), tag(),
	source_info(), npts(0), s(NULL), nsegs(0),
	tdel_tolerance(sample_time_tolerance),
	calib_tolerance(calibration_tolerance), time_tolerance(.01), copy(NULL),
	shashtable(), dhashtable(), ihashtable(), lhashtable(),
	vhashtable(), station_name(), channel_name(),
	network_name(), refsta_name(), x_chan_name(), y_chan_name(),
	z_chan_name(), instrument_code(), seg_type(), data_type(), clip_type(),
	station_chanid(-1), station_lat(-999.), station_lon(-999.),
	station_elev(-999.), station_dnorth(0.0), station_deast(0.0),
	station_hang(-999.), station_vang(-999.), station_alpha(0.),
	station_beta(0.), station_gamma(0.), current_alpha(0.),
	current_beta(0.), current_gamma(0.), component_code(0),
	original_tbeg(NULL_TIME), original_tend(NULL_TIME),
	selection_start(NULL_TIME), selection_end(NULL_TIME),
	julian_date(-1), derived(false), data_methods(), _data_source(NULL)
{
    init();
}

/** Construct a GTimeSeries from an array of GSegment objects. The function
 *  addSegment() is called for each object in the GSegment array.
 *  @param[in] segments an array of GSegment objects.
 *  @param[in] segments_length the number of GSegment objects in segments[].
 */
GTimeSeries::GTimeSeries(GSegment **segments, int segments_length) :
	response_file(NULL), response(), no_response(false), inid(-1),
	beam_elements(), waveform_io(NULL), array_elements(), tag(),
	source_info(), npts(0), s(NULL), nsegs(0), tdel_tolerance(0.99),
	calib_tolerance(.01), time_tolerance(.01), copy(NULL),
	shashtable(), dhashtable(), ihashtable(), lhashtable(),
	vhashtable(), station_name(), channel_name(),
	network_name(), refsta_name(), x_chan_name(), y_chan_name(),
	z_chan_name(), instrument_code(), seg_type(), data_type(), clip_type(),
	station_chanid(-1), station_lat(-999.), station_lon(-999.),
	station_elev(-999.), station_dnorth(0.0), station_deast(0.0),
	station_hang(-999.), station_vang(-999.), station_alpha(0.),
	station_beta(0.), station_gamma(0.), current_alpha(0.),
	current_beta(0.), current_gamma(0.), component_code(0),
	original_tbeg(NULL_TIME), original_tend(NULL_TIME),
	selection_start(NULL_TIME), selection_end(NULL_TIME),
	julian_date(-1), derived(false), data_methods(), _data_source(NULL)
{
    init();
    for(int i = 0; i < segments_length; i++) {
	addSegment(segments[i]);
    }
}

/** Construct a GTimeSeries from a single GSegment object. The function
 *  addSegment() is called to add the segment.
 */
GTimeSeries::GTimeSeries(GSegment *seg) :
	response_file(NULL), response(), no_response(false), inid(-1),
	beam_elements(), waveform_io(NULL), array_elements(), tag(),
	source_info(), npts(0), s(NULL), nsegs(0), tdel_tolerance(0.99),
	calib_tolerance(.01), time_tolerance(.01), copy(NULL),
	shashtable(), dhashtable(), ihashtable(), lhashtable(),
	vhashtable(), station_name(), channel_name(),
	network_name(), refsta_name(), x_chan_name(), y_chan_name(),
	z_chan_name(), instrument_code(), seg_type(), data_type(), clip_type(),
	station_chanid(-1), station_lat(-999.), station_lon(-999.),
	station_elev(-999.), station_dnorth(0.0), station_deast(0.0),
	station_hang(-999.), station_vang(-999.), station_alpha(0.),
	station_beta(0.), station_gamma(0.), current_alpha(0.),
	current_beta(0.), current_gamma(0.), component_code(0),
	original_tbeg(NULL_TIME), original_tend(NULL_TIME),
	selection_start(NULL_TIME), selection_end(NULL_TIME),
	julian_date(-1), derived(false), data_methods(), _data_source(NULL)
{
    init();
    addSegment(seg);
}

GTimeSeries::GTimeSeries(const GTimeSeries &ts) :
	response_file(ts.response_file), response(ts.response),
	no_response(ts.no_response), inid(ts.inid),
	beam_elements(ts.beam_elements), waveform_io(NULL),
	array_elements(ts.array_elements), tag(ts.tag),
	source_info(ts.source_info), npts(ts.npts), s(NULL),
	nsegs(0), tdel_tolerance(ts.tdel_tolerance),
	calib_tolerance(.01), time_tolerance(.01), copy(NULL),
	shashtable(ts.shashtable), dhashtable(ts.dhashtable),
	ihashtable(ts.ihashtable), lhashtable(ts.lhashtable),
	vhashtable(ts.vhashtable),
	station_name(ts.station_name), channel_name(ts.channel_name),
	network_name(ts.network_name), refsta_name(ts.refsta_name),
	x_chan_name(ts.x_chan_name), y_chan_name(ts.y_chan_name),
	z_chan_name(ts.z_chan_name), instrument_code(ts.instrument_code),
	seg_type(ts.seg_type), data_type(ts.data_type), clip_type(ts.clip_type),
	station_chanid(ts.station_chanid), station_lat(ts.station_lat),
	station_lon(ts.station_lon), station_elev(ts.station_elev),
	station_dnorth(ts.station_dnorth), station_deast(ts.station_deast),
	station_hang(ts.station_hang), station_vang(ts.station_vang),
	station_alpha(ts.station_alpha), station_beta(ts.station_beta),
	station_gamma(ts.station_gamma), current_alpha(ts.current_alpha),
	current_beta(ts.current_beta), current_gamma(ts.current_gamma),
	component_code(ts.component_code), original_tbeg(ts.original_tbeg),
	original_tend(ts.original_tend), selection_start(ts.selection_start),
	selection_end(ts.selection_end), julian_date(ts.julian_date),
	derived(ts.derived), data_methods(ts.data_methods),
	_data_source(ts._data_source)
{
    if(ts.waveform_io) waveform_io = (WaveformIO *)ts.waveform_io->clone();
    for(int i = 0; i < ts.nsegs; i++) {
	addSegment(new GSegment(ts.s[i]));
    }
    if(ts.copy) {
	copy = new GTimeSeries((const GTimeSeries)*ts.copy);
    }
    if(_data_source) _data_source->addOwner(this);
}

GTimeSeries::GTimeSeries(const GTimeSeries *ts) :
	response_file(ts->response_file), response(ts->response),
	no_response(ts->no_response), inid(ts->inid),
	beam_elements(ts->beam_elements), waveform_io(NULL),
	array_elements(ts->array_elements), tag(ts->tag),
	source_info(ts->source_info), npts(ts->npts), s(NULL),
	nsegs(0), tdel_tolerance(ts->tdel_tolerance),
	calib_tolerance(.01), time_tolerance(.01), copy(NULL),
	shashtable(ts->shashtable), dhashtable(ts->dhashtable),
	ihashtable(ts->ihashtable), lhashtable(ts->lhashtable),
	vhashtable(ts->vhashtable),
	station_name(ts->station_name), channel_name(ts->channel_name),
	network_name(ts->network_name), refsta_name(ts->refsta_name),
	x_chan_name(ts->x_chan_name), y_chan_name(ts->y_chan_name),
	z_chan_name(ts->z_chan_name), instrument_code(ts->instrument_code),
	seg_type(ts->seg_type), data_type(ts->data_type),
	clip_type(ts->clip_type),
	station_chanid(ts->station_chanid), station_lat(ts->station_lat),
	station_lon(ts->station_lon), station_elev(ts->station_elev),
	station_dnorth(ts->station_dnorth), station_deast(ts->station_deast),
	station_hang(ts->station_hang), station_vang(ts->station_vang),
	station_alpha(ts->station_alpha), station_beta(ts->station_beta),
	station_gamma(ts->station_gamma), current_alpha(ts->current_alpha),
	current_beta(ts->current_beta), current_gamma(ts->current_gamma),
	component_code(ts->component_code), original_tbeg(ts->original_tbeg),
	original_tend(ts->original_tend), selection_start(ts->selection_start),
	selection_end(ts->selection_end), julian_date(ts->julian_date),
	derived(ts->derived), data_methods(ts->data_methods),
	_data_source(ts->_data_source)
{
    if(ts->waveform_io) waveform_io = (WaveformIO *)ts->waveform_io->clone();
    for(int i = 0; i < ts->nsegs; i++) {
	addSegment(new GSegment(ts->s[i]));
    }
    if(ts->copy) {
	copy = new GTimeSeries((const GTimeSeries)*ts->copy);
    }
    if(_data_source) _data_source->addOwner(this);
}

GTimeSeries & GTimeSeries::operator=(const GTimeSeries &ts)
{
    if(this == &ts) return *this;

    for(int i = 0; i < nsegs; i++) s[i]->removeOwner(this);
    free(s);

    data_methods.clear();
    if(waveform_io) delete waveform_io;
    array_elements.clear();

    if(copy) delete copy;
    copy = NULL;

    copyInfo(ts, true);

    for(int i = 0; i < ts.nsegs; i++) {
	addSegment(new GSegment(ts.s[i]));
    }

    if(ts.copy) {
	copy = new GTimeSeries();
	for(int i = 0; i < ts.copy->nsegs; i++) {
	    copy->addSegment(new GSegment(ts.copy->s[i]));
	}
    }
    if(_data_source) _data_source->addOwner(this);

    return *this;
}

/** Initialize.
 */
void GTimeSeries::init(void)
{
    s = (GSegment **)malloc(sizeof(GSegment *));
    if( !s ) {
	GError::setMessage("GTimeSeries.init: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
}

/** Destructor. */
GTimeSeries::~GTimeSeries(void)
{
    for(int i = 0; i < nsegs; i++) s[i]->removeOwner(this);
    free(s);

    data_methods.clear();
    if(waveform_io) delete waveform_io;
    array_elements.clear();
    if(copy) delete copy;
    if(_data_source) _data_source->removeOwner(this);
}

/** Add a GSegment to this GTimeSeries with optional join argument. If the
 *  argument join_segments is true, the input GSegment will be joined to an
 *  existing adjacent GSegment, if possible. It join_segments is false, the
 *  input GSegment will not be joined to another GSegment.
 *  @param[in] seg the GSegment object.
 *  @param[in] join_segments
 *  throws GERROR_MALLOC_ERROR
 */
void GTimeSeries::addSegment(GSegment *seg, bool join_segments) throw(int)
{
    int i1, i2;
    int i, j, num_segs;
    double  d1, d2;
    GSegment *tmp = NULL;

    if(seg == NULL || seg->length() == 0) return;

    num_segs = nsegs;

    /* new data overrides existing data. check for overlaps
     */
    for(i = num_segs-1; i >= 0; i--)
    {
	if(s[i]->tbeg() <= seg->tend() &&
		s[i]->tend() + tdel_tolerance*s[i]->tdel() >= seg->tbeg())
	{
	    // new segment overlaps s[i]

	    d1 = floor((seg->tbeg() - s[i]->tbeg())/s[i]->tdel());
	    d2 = floor((seg->tend() + tdel_tolerance*s[i]->tdel()
			- s[i]->tbeg())/s[i]->tdel()) + 1;

	    if(d1 <= 0. && d2 >= (double)s[i]->length()) {
		// new segment completely covers s[i]
		s[i]->removeOwner(this);
		for(j = i; j < num_segs-1; j++) s[j] = s[j+1];
		s[num_segs-1] = NULL;
		num_segs--;
	    }
	    else if(d1 <= 0.) {
		// new segment overlaps the beginning of s[i]
		i1 = (int)(d2+.5) + 1;
		s[i]->truncate(i1, s[i]->length()-1);
	    }
	    else if(d2 >= (double)s[i]->length()) {
		// new segment overlaps the end of s[i]
		i2 =  (int)(d1+.5);
		s[i]->truncate(0, i2);
	    }
	    else {
		// new segment overlaps middle of s[i]
		tmp = new GSegment(s[i]);
		i1 =  (int)(d1+.5);
		s[i]->truncate(0, i1);
		i2 =  (int)(d2+.5) + 1;
		if(i2 < tmp->length()) {
		    tmp->truncate(i2, tmp->length());
		}
		else {
		    tmp->deleteObject();
		    tmp = NULL;
		}
		break;
	    }
	}
    }
    if(tmp != NULL) {
	s = (GSegment **)realloc(s, (num_segs+2)*sizeof(GSegment *));
	if( !s ) {
	    GError::setMessage("GTimeSeries.addSegment: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	s[num_segs++] = seg;
	seg->addOwner(this);
	s[num_segs++] = tmp;
	tmp->addOwner(this);
    }
    else {
	s = (GSegment **)realloc(s, (num_segs+1)*sizeof(GSegment *));
	if( !s ) {
	    GError::setMessage("GTimeSeries.addSegment: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	s[num_segs++] = seg;
	seg->addOwner(this);
    }
    nsegs = num_segs;

    qsort(s, num_segs, sizeof(GSegment *), sortSegments);

    if(join_segments) {
	joinSegments();
    }

    for(i = 0, npts = 0; s != NULL && i < nsegs; i++) {
	npts += s[i]->length();
    }
}

/** Join segments where possible. Check if any adjacent GSegments can be
 *  combined into one GSegment.
 *  throws GERROR_MALLOC_ERROR
 */
void GTimeSeries::joinSegments(void) throw(int)
{
    int j, j1, k, n;
    double ddt, dgap, dcalib;
    GSegment *new_s;

    if(nsegs <= 1) return;

    /* set the tdel for segments with length() == 1
     */
    for(j = 0; j < nsegs; j++) {
	if(s[j]->length() == 1) {
	    if(j == 0) {
		if(nsegs > 1) {
		    s[0]->setTdel(s[1]->tbeg() - s[0]->tbeg());
		}
	    }
	    else {
		s[j]->setTdel(s[j]->tbeg() - s[j-1]->tend());
	    }
	}
    }

    j1 = 0;
    for(j = 0; j < nsegs; j++)
    {
	if(j != nsegs-1 && s[j]->length() > 1 && s[j+1]->length() > 1)
	{
	    ddt = fabs(s[j+1]->tdel() - s[j]->tdel())/s[j1]->tdel();
	    if(s[j1]->calib() != 0.) {
		dcalib = fabs((s[j+1]->calib() - s[j]->calib())/s[j1]->calib());
	    }
	    else {
		dcalib = fabs(s[j+1]->calib() - s[j]->calib());
	    }
	    dgap = fabs(s[j+1]->tbeg() - s[j]->tend() - s[j1]->tdel())/
			s[j1]->tdel();
	    if(ddt <= tdel_tolerance && dgap <= tdel_tolerance
			&& dcalib <= calib_tolerance)
	    {
		continue;
	    }
	}

	if(j > j1)
	{
	    for(k=j1, n=0; k <= j; k++) n += s[k]->length();
	    new_s = new GSegment(n, s[j1]->tbeg(), s[j1]->tdel(),
				s[j1]->calib(), s[j1]->calper());
	    new_s->addOwner(this);

	    for(k = j1, n = 0; k <= j; k++) {
		memcpy(new_s->data+n, s[k]->data, s[k]->length()*sizeof(float));
                n += s[k]->length();
	    }
	    s[j1]->removeOwner(this);
	    s[j1] = new_s;
	    for(k = j1+1; k <= j; k++) {
		s[k]->removeOwner(this);
		s[k] = NULL;
	    }
	}
	j1 = j + 1;
    }
    for(j = n = 0; j < nsegs; j++) {
	if(s[j] != NULL) s[n++] = s[j];
    }
    if(n < nsegs) {
	s = (GSegment **)realloc(s, n*sizeof(GSegment *));
	if( !s ) {
	    GError::setMessage("TimeSeries.joinSegments: malloc error.");
	    throw GERROR_MALLOC_ERROR;
	}
	nsegs = n;
    }
    for(j = npts = 0; j < nsegs; j++) {
	npts += s[j]->length();
    }
}

/** Fill all data gaps that are <= max_gap sample intervals. Gaps are filled
 *  by cubic spline interpolation. A gap will not be filled if the
 *  change in sample intervals between segments exceeds the tdel_tolerance.
 *  @param[in] max_gap the maximum size of a gap in sample intervals that will
 *  be filled by interpolated values.
 */
void GTimeSeries::fillAllGaps(int max_gap)
{
    fillGap(-1, max_gap);    /* fill all gaps */
}

/** Fill a single data gap. Fill the gap before a segment if it is <= max_gap
 *  sample intervals. The gap is filled by cubic spline interpolation.
 *  The gap will not be filled if the change in sample intervals between
 *  segments exceeds the tdel_tolerance.
 *  @param[in] segment_index the index of the segment. Fill the gap between this
 *  segment and the previous segment. Input -1 to fill the gaps between all
 *  segments.
 *  @param[in] max_gap the maximum size of a gap in sample intervals that will
 *  be filled by interpolated values.
 */
void GTimeSeries::fillGap(int segment_index, int max_gap)
{
#ifdef HAVE_GSL
    int i, j, k, l, len, missing, m, n;
    int j1 = 0, total_missing = 0;
    double *xa=NULL, *ya=NULL;
    double ddt, dgap, dcalib;
    GSegment *new_s;
    gsl_spline *spline;
    gsl_interp_accel *acc;

    if(s == NULL || nsegs <= 1) return;

    xa = (double *)malloc((20+max_gap)*sizeof(double));
    ya = (double *)malloc((20+max_gap)*sizeof(double));
    if(!xa || !ya) {
	GError::setMessage("GTimeSeries.fillGap: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    for(j = 0; j < nsegs; j++)
    {
	if(j != nsegs-1) {
	    ddt = fabs(s[j+1]->tdel() - s[j]->tdel())/s[j1]->tdel();
	    if(s[j1]->calib() != 0.) {
		dcalib = fabs((s[j+1]->calib() - s[j]->calib())/s[j1]->calib());
	    }
	    else {
		dcalib = fabs(s[j+1]->calib() - s[j]->calib());
	    }
	    dgap = fabs(s[j+1]->tbeg() - s[j]->tend() - s[j1]->tdel())/
			s[j1]->tdel();
	    missing = (int)rint(dgap); /* round to nearest integer */
	    total_missing += missing;
	    dgap = fabs(dgap - missing);
	    if(segment_index < 0 || j+1 == segment_index) {
		if(ddt <= tdel_tolerance && missing <= max_gap &&
		    dgap <= tdel_tolerance &&
		    dcalib <= calib_tolerance) continue;
	    }
	    total_missing -= missing;
	}
	if(j > j1) {
	    for(k=j1, n=0; k <= j; k++) n += s[k]->length();
	    n += total_missing;

	    new_s = new GSegment(n, s[j1]->tbeg(), s[j1]->tdel(),
				s[j1]->calib(), s[j1]->calper());
	    if(!new_s) {
		GError::setMessage("GTimeSeries.fillGap: malloc failed.");
		throw GERROR_MALLOC_ERROR;
	    }
	    new_s->addOwner(this);

	    double t0 = s[j1]->tbeg();

	    for(k=j1, n=0; k < j; k++) {
		len = s[k]->length();
		memcpy(new_s->data+n, s[k]->data, n*sizeof(float));
		n += len;
		m = 0;
		l = (len > 10) ? 10 : len;
		for(i = 0; i < l; i++) {
		    xa[m] = s[k]->tbeg() - t0 +(len-l+i)*s[k]->tdel();
		    ya[m++] = (double)s[k]->data[len-l+i];
		}

		len = s[k+1]->length();
		l = (len > 10) ? 10 : len;
		for(i = 0; i < l; i++) {
       		    xa[m] = s[k+1]->tbeg() - t0 + i*s[k]->tdel();
		    ya[m++] = (double)s[k+1]->data[i];
		}
		spline = gsl_spline_alloc(gsl_interp_cspline, m);
		acc = gsl_interp_accel_alloc();
		gsl_spline_init(spline, xa, ya, m);

		dgap = fabs(s[k+1]->tbeg() - s[k]->tend() - s[j1]->tdel())/
				s[j1]->tdel();
		missing = (int)rint(dgap);
		dgap = fabs(dgap - missing);

		len = s[k]->length();
		for(i = 0; i < missing; i++) {
		    double x = s[k]->tbeg() - t0 + (len+i)*s[k]->tdel();
		    new_s->data[n++] = (float)gsl_spline_eval(spline, x, acc);
		}
		gsl_spline_free(spline);
		gsl_interp_accel_free(acc);
	    }
	    len = s[j]->length();
	    memcpy(new_s->data+n, s[j]->data, len*sizeof(float));

	    s[j1]->removeOwner(this);
	    s[j1] = new_s;
	    for(k = j1+1; k <= j; k++) {
		s[k]->removeOwner(this);
		s[k] = NULL;
	    }
	}
	j1 = j+1;
	total_missing = 0;
    }

    Free(xa);
    Free(ya);

    for(j = n = 0; j < nsegs; j++) if(s[j] != NULL) n++;

    if(n < nsegs) {
	s = (GSegment **)realloc(s, n*sizeof(GSegment *));
	if(!s) {
	    GError::setMessage("GTimeSeries.fillGap: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	nsegs = n;
    }
    for(j = npts = 0; j < nsegs; j++) {
	npts += s[j]->length();
    }
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

/** Combine this GTimeSeries with the input GTimeSeries. The time period of
 *  the input GTimeSeries can be before, after or overlapping with the time
 *  period of this GTimeSeries.
 *  @param[in] ts the GSegment to combine with this GTimeSeries.
 */
void GTimeSeries::join(GTimeSeries *ts)
{
    if(ts == NULL || ts == this) return;

    for(int i = 0; i < ts->size(); i++) {
	addSegment(ts->s[i]);
    }
}

Gobject * GTimeSeries::clone(void)
{
    GTimeSeries *ts;
 
    if(nsegs > 0) {
	ts = new GTimeSeries(new GSegment(s[0]));
	for(int i = 1; i < nsegs; i++) {
	    ts->addSegment(new GSegment(s[i]));
	}
    }
    else {
	ts = new GTimeSeries();
    }
    ts->tdel_tolerance = tdel_tolerance;
    ts->calib_tolerance = calib_tolerance;
    ts->time_tolerance = time_tolerance;

    ts->copyInfo(*this, true);

    if(copy) {
	ts->copy = new GTimeSeries();
	for(int i = 0; i < copy->nsegs; i++) {
	    ts->copy->addSegment(new GSegment(copy->s[i]));
	}
    }

    return (Gobject *)ts;
}
    
void GTimeSeries::copyInfo(const GTimeSeries &ts, bool clone_hashtable)
{
//    if(!ts || ts == this) return;
    if(&ts == this) return;

    setSta(ts.station_name);
    setChan(ts.channel_name);
    setNet(ts.network_name);
    setRefsta(ts.refsta_name);
    setXChan(ts.x_chan_name);
    setYChan(ts.y_chan_name);
    setZChan(ts.z_chan_name);
    setInstype(ts.instrument_code);
    setSegtype(ts.seg_type);
    setDatatype(ts.data_type);
    setClip(ts.clip_type);

    station_chanid = ts.station_chanid;
    station_lat = ts.station_lat;
    station_lon = ts.station_lon;
    station_elev = ts.station_elev;
    station_dnorth = ts.station_dnorth;
    station_deast = ts.station_deast;
    station_hang = ts.station_hang;
    station_vang = ts.station_vang;
    station_alpha = ts.station_alpha;
    station_beta = ts.station_beta;
    station_gamma = ts.station_gamma;
    current_alpha = ts.current_alpha;
    current_beta = ts.current_beta;
    current_gamma = ts.current_gamma;
    component_code = ts.component_code;

    original_tbeg = ts.original_tbeg;
    original_tend = ts.original_tend;

    selection_start = ts.selection_start;
    selection_end = ts.selection_end;

    julian_date = ts.julian_date;

    derived = ts.derived;

    if(clone_hashtable) {
	shashtable = ts.shashtable;
	dhashtable = ts.dhashtable;
	ihashtable = ts.ihashtable;
	lhashtable = ts.lhashtable;
	vhashtable = ts.vhashtable;

	data_methods = ts.data_methods;
	waveform_io = (ts.waveform_io) ? (WaveformIO *)ts.waveform_io->clone()
			: NULL;

	response_file = ts.response_file;
	response = ts.response;
	no_response = ts.no_response;
	beam_elements = ts.beam_elements;
	array_elements = ts.array_elements;
    }
    setDataSource(ts._data_source);
    tag = ts.tag;
}

/** 
 *  Returns true if the i'th GSegment of this GTimeSeries is
 *  continuous with the i-1 GSegment. Returns false otherwise.
 *  The first GSegment is 0. Returns false for i = 0.
 *  A GSegment is continuous with the previous GSegment, if the
 *  shift in tbeg < tbegTol and the shift in tdel is < tdelTol.
 *  @param i The GSegment index. Begins at 0.
 *  @param tbeg_tol Tolerance for shifted tbeg.
 *  @param tdel_tol Tolerance for shifted tdel.
 */
bool GTimeSeries::continuous(int i, double tbeg_tol, double tdel_tol)
{
    double sampdif, timedif;
    if(i <= 0 || i >= nsegs) return false;

    sampdif = s[i]->tdel() - s[i-1]->tdel();
    timedif = s[i]->tbeg() - (s[i-1]->tend() + s[i-1]->tdel());
    return  (fabs(sampdif) < tdel_tol && fabs(timedif) < tbeg_tol) ? true:false;
}

/** Copy the data from this GTimeSeries into a float array. All data values
 *  from this GTimeSeries are copied to the array data[].
 *  param[in,out] data a float array of length at least length() samples.
 */
void GTimeSeries::copyInto(float *data)
{
    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) {
	    *data++ = *d;
	}
    }
}

/** Get the maximum data value.
 *  @returns the maximum data value of the GTimeSeries. Returns 0., if this
 *  GTimeSeries has length() equal to zero.
 */
double GTimeSeries::dataMax(void)
{
    if(nsegs < 1) return 0.;

    double dmax = (s[0]->length() > 0) ? s[0]->data[0] : 0.;

    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) {
	    if(s[j]->data[i] > dmax) dmax = *d;
	}
    }
    return(dmax);
}

/** Get the maximum data value with a time period.
 *  param[in] t1 the beginning of the time period.
 *  param[in] t2 the end of the time period.
 *  @returns the maximum data value of the GTimeSeries between t1 and t2.
 *	Returns 0., if this GTimeSeries has length() equal to zero.
 */
double GTimeSeries::dataMax(double t1, double t2)
{
    double dmin=0., dmax=0.;
    GDataPoint *d1 = nearest(t1);
    GDataPoint *d2 = nearest(t2);

    dataMinMax(d1, d2, &dmin, &dmax);

    d1->deleteObject();
    d2->deleteObject();
    
    return(dmax);
}

/** Get the minimum data value with a time period.
 *  param[in] t1 the beginning of the time period.
 *  param[in] t2 the end of the time period.
 *  @returns the minimum data value of the GTimeSeries between t1 and t2.
 *	Returns 0., if this GTimeSeries has length() equal to zero.
 */
double GTimeSeries::dataMin(double t1, double t2)
{
    double dmin=0., dmax=0.;
    GDataPoint *d1 = nearest(t1);
    GDataPoint *d2 = nearest(t2);

    dataMinMax(d1, d2, &dmin, &dmax);

    d1->deleteObject();
    d2->deleteObject();
    
    return(dmin);
}

/** Get the maximum data value as a GDataPoint object.
 *  @returns a GDataPoint object for the maximum data value of this GTimeSeries.
 *  Returns GDataPoint(0.) if this GTimeSeries has no data.
 */
GDataPoint * GTimeSeries::maxPoint(void)
{
    if(nsegs < 1) return NULL;

    double dmax = (s[0]->length() > 0) ? s[0]->data[0] : 0.;
    int jmax = 0;
    int imax = 0;

    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) {
	    if(*d > dmax) {
		dmax = *d;
		jmax = j;
		imax = i;
	    }
	}
    }
    return new GDataPoint(this, s[jmax], imax);
}

/** Get the minimum data value.
 *  @returns the minimum data value of the GTimeSeries. Returns 0., if this
 *  GTimeSeries has length() equal to zero.
 */
double GTimeSeries::dataMin(void)
{
    if(nsegs < 1) return 0.;

    double dmin = (s[0]->length() > 0) ? s[0]->data[0] : 0.;

    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) {
	    if(s[j]->data[i] < dmin) dmin = *d;
	}
    }
    return(dmin);
}

/** Get the minimum data value as a GDataPoint object.
 *  @returns a GDataPoint object for the minimum data value of this GTimeSeries.
 *  Returns GDataPoint(0.) if this GTimeSeries has no data.
 */
GDataPoint * GTimeSeries::minPoint(void)
{
    if(nsegs < 1) return NULL;

    double dmin = (s[0]->length() > 0) ? s[0]->data[0] : 0.;
    int jmin = 0;
    int imin = 0;

    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) {
	    if(*d < dmin) {
		dmin = *d;
		jmin = j;
		imin = i;
	    }
	}
    }
    return new GDataPoint(this, s[jmin], imin);
}

/** Get the minimum and maximum data values for a time window. The time window
 *  is specified by two GDataPoint objects.
 *  @param[in] d1 the GDataPoint for the data value at the beginning of the time
 *	window.
 *  @param[in] d2 the GDataPoint for the data value at the end of the time
 *	window.
 *  @param[out] min the data minimum for the time window.
 *  @param[out] max the data maximum for the time window.
 *  @returns true for success, false for invalid d1 or d2.
 */
bool GTimeSeries::dataMinMax(GDataPoint *d1, GDataPoint *d2, double *min,
			double *max)
{
    int i, j, n;
    GSegment *seg = NULL;

    if(nsegs < 1) return false;

    if(d1->segmentIndex() < 0 || d1->segmentIndex() >= nsegs) return false;
    if(d2->segmentIndex() < 0 || d2->segmentIndex() >= nsegs) return false;
    if(d1->index() < 0 || d1->index() >= s[d1->segmentIndex()]->length())
	return false;
    if(d2->index() < 0 || d2->index() >= s[d2->segmentIndex()]->length())
	return false;

    *min = *max = s[d1->segmentIndex()]->data[d1->index()];

    seg = s[d1->segmentIndex()];

    n = d1->segmentIndex() != d2->segmentIndex() ? seg->length() :d2->index()+1;

    for(i = d1->index(); i < n; i++) {
	float d = seg->data[i];
	if(*min > d) *min = d;
	if(*max < d) *max = d;
    }

    for(j = d1->segmentIndex()+1; j < d2->segmentIndex(); j++) {
	seg = s[j];
	int npts = seg->length();
	for(i = 0; i < npts; i++) {
	    float d = seg->data[i];
	    if(*min > d) *min = d;
	    if(*max < d) *max = d;
	}
    }
    if(d2->segmentIndex() > d1->segmentIndex()) {
	seg = s[d2->segmentIndex()];
	for(i = 0; i <= d2->index(); i++) {
	    float d = seg->data[i];
	    if(*min > d) *min = d;
	    if(*max < d) *max = d;
	}
    }
    return true;
}

/** Get the GDataPoint lower bound to the input time. Returns NULL if this
 *  GTimeSeries object contains no data.
 *  @param[in] epoch the epochal time.
 *  @returns a GDataPoint for the data value whose time is <= the input time.
 * 	Returns the first GDataPoint if time < tbeg(). Returns the last
 *	GDataPoint if time > tend().
 */
GDataPoint * GTimeSeries::lowerBound(double epoch)
{
    if(nsegs <= 0) return NULL;

    if(epoch >= tend()) {
	return(new GDataPoint(this, s[nsegs-1], s[nsegs-1]->length()-1));
    }
    else {
	int ndex, i;

	for(i = nsegs-1; i >= 0; i--) {
	    if(epoch >= s[i]->tbeg()) break;
	}
	if(i >= 0) {
	    ndex = (int)((epoch-s[i]->tbeg())/s[i]->tdel());
	    if(epoch < s[i]->tbeg() + ndex*s[i]->tdel()) ndex--;
	    if(ndex >= s[i]->length()) ndex = s[i]->length()-1;
	    if(ndex < 0) ndex = 0;
	}
	else {
	    i = 0;
	    ndex = 0;
	}
	return(new GDataPoint(this, s[i], ndex));
    }
}

/** Get the arithmetic mean of the data in this GTimeSeries.
 *  @returns the data mean value.
 */
double GTimeSeries::mean(void)
{
    if(nsegs < 1 || npts <= 0) return 0.;

    double m = 0.;
    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) m += *d;
    }
    m /= npts;

    return(m);
}

/** Get the minimum, maximum, and mean data values.
 */
void GTimeSeries::dataStats(double *dmin, double *dmax, double *dmean)
{
    if(nsegs < 1 || npts <= 0) return;

    *dmin = (s[0]->length() > 0) ? s[0]->data[0] : 0.;
    *dmax = *dmin;
    double m = 0.;

    for(int j = 0; j < nsegs; j++) {
	int n = s[j]->length();
	float *d = s[j]->data;
	for(int i = 0; i < n; i++, d++) {
	    m += *d;
	    if(*d < *dmin) *dmin = *d;
	    else if(*d > *dmax) *dmax = *d;
	}
    }
    *dmean = m / npts;
}

/** Get the nearest GDataPoint to the input time. Returns NULL if this
 *  GTimeSeries contains no data.
 *  @param[in] epoch the epochal time.
 *  @returns a GDataPoint object for the data value whose time is closest to
 *  the input time.
 */
GDataPoint * GTimeSeries::nearest(double epoch)
{
    GDataPoint *l;

    if(nsegs <= 0) return NULL;

    if(epoch <= tbeg()) {
	return(new GDataPoint(this, s[0], 0));
    }
    else if(epoch >= tend()) {
	return(new GDataPoint(this, s[nsegs-1], s[nsegs-1]->length()-1));
    }
    l = lowerBound(epoch);

    if(l->index() < l->segment()->length()-1)
    {
	if(epoch - l->time() > .5*l->segment()->tdel()) {
	    GDataPoint *dp = new GDataPoint(this, l->segment(), l->index()+1);
	    l->deleteObject();
	    return dp;
	}
	return l;
    }
    else
    {
	if(epoch - l->time() > s[l->segmentIndex()+1]->tbeg() - epoch) {
	    GDataPoint *dp = new GDataPoint(this, s[l->segmentIndex()+1], 0);
	    l->deleteObject();
	    return dp;
	}
	return l;
    }
}

/** Get the segment that contains a time point. Returns NULL if
 *  no segment contains the time.
 *  @param[in] epoch the epochal time.
 *  @returns the GSegment whose time period contains the input time value.
 */
GSegment * GTimeSeries::segment(double epoch)
{
    for(int i = 0; i < nsegs; i++)
    {
	if(epoch >= s[i]->tbeg() && epoch <= s[i]->tend()) return s[i];
    }
    return NULL;
}

/** Get the segment nearest to a time point. If no segment contains the
 *  time, returns the nearest segment.
 *  @param[in] epoch the epochal time.
 *  @returns the GSegment whose time period contains the input time value.
 *  Returns the first GSegment if epoch < tbeg(). Returns the last GSegment if
 *  epoch > tend().
 */
GSegment * GTimeSeries::nearestSegment(double epoch)
{
    GSegment *seg;

    if((seg = segment(epoch)) != NULL) return seg;

    if(nsegs <= 0) return NULL;

    double min = fabs(epoch - s[0]->tbeg());
    seg = s[0];

    for(int i = 0; i < nsegs; i++)
    {
	double dt = fabs(epoch - s[i]->tbeg());
	if(min > dt) {
	    min = dt;
	    seg = s[i];
	}

	dt = fabs(epoch - s[i]->tend());
	if(min > dt) {
	    min = dt;
	    seg = s[i];
	}
    }
    return(seg);
}

/** 
 *  Creates a new GTimeSeries from this GTimeSeries including only data
 *  between times t1 and t2.
 *  @return New GTimeSeries with samples between time t1 to time t2. Returns \
 *          NULL, if t1 > t2 or t1 >= tend(ts) or t2 < tbeg(ts) or no samples \
 *	    are between t1 and t2.
 */
GTimeSeries *GTimeSeries::subseries(double t1, double t2)
{
    if(size() <= 0) return NULL;
    double del = time_tolerance*s[0]->tdel();

    if(t2 < tbeg()+del || t1 > t2+del || t1 > tend()-del) return NULL;

    if(t1 < tbeg()+del && t2 > tend()-del) {
	return new GTimeSeries(*this);
    }

    bool has_owners;
    if(!(has_owners = hasOwners())) {
	// add this as an owner of this, so we don't get freed when d1 and d2
	// are freed.
	addOwner(this);
    }
    GDataPoint *d1 = upperBound(t1-del);
    GDataPoint *d2 = lowerBound(t2+del);
    if(d1->segmentIndex() == d2->segmentIndex() && d1->index() > d2->index())
    {
	d1->deleteObject();
	d2->deleteObject();
	if(!has_owners) removeOwner(this, false); // remove owner,do not delete
	return NULL;
    }
    GTimeSeries *ts = new GTimeSeries();

    ts->copyInfo(*this, true);

    int i1 = d1->segmentIndex();
    int i2 = d2->segmentIndex();

    if(i1 == i2) {
	GSegment *seg = s[i1]->subsegment(d1->index(), d2->index()+1);
	ts->addSegment(seg);
    }
    else {
	GSegment *seg = s[i1]->subsegment(d1->index(), s[i1]->length());
	ts->addSegment(seg);
	for(int i = i1+1; i < i2; i++) {
	    seg = s[i]->subsegment(0, s[i]->length());
	    ts->addSegment(seg);
	}
	seg = s[i2]->subsegment(0, d2->index()+1);
	ts->addSegment(seg);
    }
    d1->deleteObject();
    d2->deleteObject();
    if(!has_owners) {
	removeOwner(this, false); // remove owner, do not delete
    }
    return ts;
}

/** Cut both ends of a GTimeSeries. Saves data between times t1 and t2.
 *  @param[in] t1 the beginning time of the data window to save.
 *  @param[in] t2 the end time of the data window to save.
 *  @returns true if any segments were actually truncated. Returns false if
 *  t1 > t2 or there are no data times between t1 and t2.
 */
bool GTimeSeries::truncate(double t1, double t2)
{
    if(size() <= 0) return true;

    double del = time_tolerance*s[0]->tdel();
    if(t1 <  tbeg()+del && t2 > tend()-del) {
	return true;
    }
    GTimeSeries *ts = subseries(t1, t2);

    if(ts == NULL) return false;

    removeAllSegments();

    for(int i = 0; i < ts->size(); i++) {
	addSegment(ts->s[i]);
    }
    ts->deleteObject();

    return true;
}

static int
sortSegments(const void *A, const void *B)
{
    GSegment **a = (GSegment **)A;
    GSegment **b = (GSegment **)B;
    return (*a)->tbeg() < (*b)->tbeg() ? -1 : 1;
}

/*
static void
sortSegments(GSegment **s, int i1, int i2)
{
	int lo = i1;
	int hi = i2;
	double mid;
	if(lo >= hi) return;

	mid = s[(lo+hi)/2]->tbeg();

	while(lo < hi) {
	    while(lo < hi && s[lo]->tbeg() < mid) lo++;
	    while(lo < hi && s[hi]->tbeg() >= mid) hi--;

	    if(lo < hi){
		GSegment *seg = s[lo];
		s[lo] = s[hi];
		s[hi] = seg;
	    }
	}
	if(hi < lo) {
	    int i = hi;
	    hi = lo;
	    lo = i;
	}
	sortSegments(s, i1, lo);
	sortSegments(s, lo == i1 ? lo+1 : lo, i2);
}
*/

/** Get the GDataPoint upper bound to the input time. Returns NULL if this
 *  GTimeSeries object contains no data.
 *  @param[in] epoch the epochal time.
 *  @returns a GDataPoint for the data value whose time is >= the input time.
 * 	Returns the first GDataPoint if epoch < tbeg(). Returns the last
 *	GDataPoint if epoch > tend().
 */
GDataPoint * GTimeSeries::upperBound(double epoch)
{
    if(epoch <= tbeg()) {
	return(nsegs > 0 ? new GDataPoint(this, s[0], 0) : NULL);
    }
    else {
	int i, ndex;

	for(i = 0; i < nsegs; i++) {
	    if(epoch <= s[i]->tend()) break;
	}
	if(i < nsegs) {
	    ndex = (int)((epoch - s[i]->tbeg())/s[i]->tdel());
	    if(epoch > s[i]->tbeg() + ndex*s[i]->tdel()) ndex++;
	    if(ndex >= s[i]->length()) ndex = s[i]->length()-1;
	    if(ndex < 0) ndex = 0;
	}
	else
	{
	    i = nsegs-1;
	    ndex = s[i]->length()-1;
	}
	return(new GDataPoint(this, s[i], ndex));
    }
}

/** Replace the data of this GTimeSeries with the input data[] array.
 *  @param[in] data an array of length() floats.
 */
void GTimeSeries::setData(float *data)
{
    int j, k;
    for(j = 0, k = 0; j < nsegs; j++) {
	int len = s[j]->length();
	s[j]->setData(data+k);
	k += len;
    }
}

/** Remove all GSegments from the GTimeSeries.
 */
void GTimeSeries::removeAllSegments()
{
    for(int i = 0; i < nsegs; i++) {
	s[i]->removeOwner(this);
    }
    npts = 0;
    nsegs = 0;
}

/** Replace the data of this GTimeSeries with the data from another GTimeSeries.
 *  @param[in] ts
 */
void GTimeSeries::setData(GTimeSeries *ts)
{
    for(int i = 0; i < nsegs; i++) {
	s[i]->removeOwner(this);
    }
    s = (GSegment **)realloc(s, ts->size()*sizeof(GSegment));
    nsegs = ts->size();
    for(int i = 0; i < nsegs; i++) {
	s[i] = new GSegment(ts->s[i]);
	s[i]->addOwner(this);
    }
    npts = ts->npts;
}

/** Get the GSegment containing the data at the specified index,
 *  counting from the beginning of the time series. Returns NULL,
 *  if index < 0 or index >= length().
 *  @param[in] index the index of the data sample.
 *  @returns the GSegment that contains the specified data sample.
 */
GSegment * GTimeSeries::getSegment(int index)
{
    if(index < 0 || index >= npts) return NULL;

    int n = 0;
    for(int i = 0; i < nsegs; i++) {
	if(index >= n && index < n + s[i]->length()) return s[i];
	n += s[i]->length();
    }
    return NULL;
}

/** Get the time corresponding to the index of a data point. If the index is
 *  < 0 the time is computed using the tdel() of the first GSegment. If the
 *  index is >= length(), the time is computed using the tdel() of the last
 *  GSegment.
 *  @param[in] index
 *  @returns the time corresponding to the data sample at index.
 */
double GTimeSeries::time(int index)
{
    if(!nsegs) return NULL_TIME;

    if(index < 0) {
	return s[0]->tbeg() - abs(index)*s[0]->tdel();
    }

    int n = 0;
    for(int i = 0; i < nsegs; i++) {
	if(index >= n && index < n + s[i]->length())
	{
	    return s[i]->tbeg() + (index-n)*s[i]->tdel();
	}
	n += s[i]->length();
    }
    return s[nsegs-1]->tbeg() + (index-n)*s[nsegs-1]->tdel();
}

/** Return an array of data values for the input time window. Return an array
 *  of data values between GDataPoints d1 and d2. At data gaps, insert an NAN
 *  value. Free the pointers *pt and *py when no longer needed.
 *  @param[in] d1 the first data point.
 *  @param[in] d2 the last data point.
 *  @param[out] pt an array of time values.
 *  @param[out] py an array of data values.
 *  @returns the number of values in *pt and in *py.
 */
int GTimeSeries::flaggedArrays(GDataPoint *d1, GDataPoint *d2, float **pt,
			float **py) throw(int)
{
    int i, j, n;
    float *t=NULL, *y=NULL;

    *pt = (float *)NULL;
    *py = (float *)NULL;

    double t0 = tbeg();

    if(d1->segmentIndex() == d2->segmentIndex())
    {
	n = d2->index() - d1->index() + 1;
	if( !(t = (float *)malloc(n*sizeof(float))) ||
	    !(y = (float *)malloc(n*sizeof(float))) )
	{
	    Free(t);
	    GError::setMessage("GTimeSeries.flaggedArrays: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	memcpy(y, d1->segment()->data+d1->index(), n*sizeof(float));
	for(i = 0; i < n; i++) {
	    t[i] =  d1->segment()->time(d1->index()+i) - t0;
	}
    }
    else
    {
	n = d1->segment()->length() - d1->index();
	for(i = d1->segmentIndex()+1; i < d2->segmentIndex(); i++) {
	    n += 1;  /* for fnan point */
	    n += s[i]->length();
	}
	n += 1;  /* for fnan point */
	n += d2->index() + 1;
	if( !(t = (float *)malloc(n*sizeof(float))) ||
	    !(y = (float *)malloc(n*sizeof(float))) )
	{
	    Free(t);
	    GError::setMessage("GTimeSeries.flaggedArrays: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	n = d1->segment()->length() - d1->index();
	memcpy(y, d1->segment()->data+d1->index(), n*sizeof(float));

	for(j = 0; j < n; j++) {
	    t[j] = d1->segment()->time(d1->index()+j) - t0;
	}

	for(i = d1->segmentIndex()+1; i < d2->segmentIndex(); i++)
	{
	    set_fnan(t[n]);
	    set_fnan(y[n++]);
		
	    int npts = s[i]->length();
	    memcpy(y+n, s[i]->data, npts*sizeof(float));
	    for(j = 0; j < npts; j++) {
		t[n+j] = s[i]->time(j) - t0;
	    }
	    n += s[i]->length();
	}
	set_fnan(t[n]);
	set_fnan(y[n++]);

	memcpy(y+n, d2->segment()->data, (d2->index()+1)*sizeof(float));
	for(j = 0; j <= d2->index(); j++) {
	    t[n+j] = d2->segment()->time(j) - t0;
	}
	n += d2->index() + 1;
    }
    *pt = t;
    *py = y;

    return n;
}

/**
 * Create a decimated GTimeSeries. Each segment is individually
 * decimated at the same rate. For a rate of 1, there is no
 * decimation.  For a rate of 2, every second value is returned. For
 * a rate of 3, every third value is returned, etc. Set clone_hashtable
 * to false, if the this GTimeSeries will be deleted after this function is
 * called. Set clone_hashtable true otherwise.
 * @param[in] rate The decimation rate.
 * @param[in] clone_hashtable If true, the returned GTimeSeries will have a
 *  new ghashtable. If false, it will use the ghashtable of this GTimeSeries.
 */
GTimeSeries * GTimeSeries::decimate(int rate, bool clone_hashtable)
{
    if(rate <= 2 || npts <= 2) return NULL;

    /* decimate all segments at the same rate. Would be better to
     * use an individual rate for each segment depending on its
     * percent of the total length. (Only when segments have different
     * tdel, one rate will not be appropriate for all.)
     */
    GTimeSeries *ds = new GTimeSeries();

    ds->copyInfo(*this, clone_hashtable);

    for(int k = 0; k < nsegs; k++) {
	int n = (int)(s[k]->length()/rate);
	int remainder = s[k]->length() - (int)(n*rate);
	n *= 2;
	int m = n + ((remainder > 0) ? 2 : 0);
	GSegment *seg = new GSegment(m, s[k]->tbeg(), .5*rate*s[k]->tdel(),
				s[k]->calib(), s[k]->calper());

	decimateData(s[k]->data, rate, n, remainder, seg->data);
	ds->addSegment(seg);
    }
    return(ds);
}

/**
 * Decimate an array.
 */
static void
decimateData(float *data, int rate, int n, int remainder, float *sdata)
{
    int i, j, xmin, xmax, x;
    float ymin, ymax;

    xmin = xmax = x = 0;
    for(i = 0; i < n; i += 2) {
	xmin = xmax = x++;
	ymin = ymax = *data++;

	for(j = 1; j < rate; j++, x++, data++) {
	    if(ymin > *data) {
		ymin = *data;
		xmin = x;
	    }
	    if(ymax < *data) {
		ymax = *data;
		xmax = x;
	    }
	}
	if(xmin < xmax) {
	    *sdata++ = ymin;
	    *sdata++ = ymax;
	}
	else {
	    *sdata++ = ymax;
	    *sdata++ = ymin;
	}
    }
    if(remainder > 0)
    {
	xmin = xmax = x++;
	ymin = ymax = *data++;

	for(j = 1; j < remainder; j++, x++, data++) {
	    if(ymin > *data) {
		ymin = *data;
		xmin = x;
	    }
	    if(ymax < *data) {
		ymax = *data;
		xmax = x;
	    }
	}
	if(xmin < xmax) {
	    *sdata++ = ymin;
	    *sdata++ = ymax;
	}
	else {
	    *sdata++ = ymax;
	    *sdata++ = ymin;
	}
    }
}

/** Demean the data. Subtract the mean value from the data.
 *  @returns the computed mean value.
 */
double GTimeSeries::demean(void)
{
    if(npts <= 0) return 0.;

    double mean_value = 0.;
    for(int i = 0; i < nsegs; i++) {
	int n = s[i]->length();
	for(int j = 0; j < n; j++) mean_value += s[i]->data[j];
    }
    mean_value /= (double)npts;
    for(int i = 0; i < nsegs; i++) {
	int n = s[i]->length();
	for(int j = 0; j < n; j++) s[i]->data[j] -= mean_value;
    }
    return mean_value;
}

/** Get the first WFDISC30 structure associated with this GTimeSeries.
 *  @returns the first WFDSIC30 structure associated with this GTimeSeries, or
 *  returns NULL if no structure is found.
 */
CssWfdiscClass * GTimeSeries::getWfdisc(void)
{
    if( waveform_io && (int)waveform_io->wp.size() > 0)
    {
	return &waveform_io->wp[0].wf;
    }
    return NULL;
}

/** Get the WfdiscPeriod structure associated with this GTimeSeries at the input
 *  time.
 *  @param[in] epoch
 *  @returns the WfdiscPeriod structure associated with this GTimeSeries at the
 *  specified time. Returns NULL if no structure is found.
 */
WfdiscPeriod * GTimeSeries::getWfdiscPeriod(double epoch)
{
    if( waveform_io && (int)waveform_io->wp.size() > 0)
    {
	if(epoch < waveform_io->wp[0].tbeg) {
	    return &waveform_io->wp[0];
	}
	for(int i = 0; i < (int)waveform_io->wp.size(); i++) {
	    if(epoch < waveform_io->wp[i].tend) {
		return &waveform_io->wp[i];
	    }
	}
	int i = (int)waveform_io->wp.size()-1;
	return &waveform_io->wp[i];
    }
    return NULL;
}

/** Copy the WfdiscPeriods from another GTimeSeries. This is done for beams and
 *  other derived waveforms. The derived flag will be set to true.
 */
void GTimeSeries::copyWfdiscPeriods(GTimeSeries *ts)
{
    if(waveform_io) return;

    if(ts->waveform_io)
    {
	waveform_io = new WaveformIO();
	for(int i = 0; i < (int)ts->waveform_io->wp.size(); i++) {
	    waveform_io->wp.push_back(ts->waveform_io->wp.at(i));
	}
    }
    derived = true;
}

/** Get the wfid from the first WFDISC30 associated with this GTimeSeries.
 *  @returns the wfid from the first WFDISC30 structure.
 */
long GTimeSeries::getWfid(void)
{
    if( waveform_io && (int)waveform_io->wp.size() > 0)
    {
	return waveform_io->wp[0].wf.wfid;
    }
    return -1;
}

/** Get all CssWfdiscClass structures associated with this GTimeSeries.
 *  @returns a Vector of CssWfdiscClass structures.
 */
cvector<CssWfdiscClass> * GTimeSeries::getWfdiscs(void)
{
    if( waveform_io && (int)waveform_io->wp.size() > 0)
    {
	cvector<CssWfdiscClass> *v = new cvector<CssWfdiscClass>;
	for(int i = 0; i < (int)waveform_io->wp.size(); i++) {
	    CssWfdiscClass *w = new CssWfdiscClass();
	    waveform_io->wp[i].wf.copyTo(w);

	    w->setFile(waveform_io->wp[i].wfdisc_file);
	    w->setDir(waveform_io->wp[i].dir);
	    w->setPrefix(waveform_io->wp[i].prefix);
	    int offset = waveform_io->wp[i].wfdisc_index*(w->getLineLength()+1);
	    w->setFileOffset(offset);

	    v->push_back(w);
 	}
	return v;
    }
    return NULL;
}

bool GTimeSeries::reread(void)
{
    if( copy ) {
	double d;
	removeAllSegments();
	for(int i = 0; i < copy->size(); i++) {
	    addSegment(new GSegment(copy->s[i]));
	}
	d = alpha();
	if(d < -900.) d = 0.;
	setCurrentAlpha(d);

	d = beta();
	if(d < -900.) d = 0.;
	setCurrentBeta(d);

	d = gamma();
	if(d < -900.) d = 0.;
	setCurrentGamma(d);
	return true;
    }
    else if( !_data_source ) {
	GError::setMessage("GTimeSeries::reread: no DataSource.");
	cerr << "GTimeSeries::reread: no DataSource." << endl;
	return false;
    }
    return _data_source->reread(this);
}

/** Make an internal copy of the data. This copy will be used when reread()
 *  is called, instead re-reading the data.
 */
void GTimeSeries::makeCopy(void)
{
    if( !copy ) {
	copy = new GTimeSeries();
    }
    else {
	copy->removeAllSegments();
    }
    for(int i = 0; i < nsegs; i++) {
	copy->addSegment(new GSegment(s[i]));
    }
}

void GTimeSeries::setDataSource(DataSource *ds)
{
    if(ds == _data_source) return;

    if(_data_source) {
	_data_source->removeOwner(this);
    }
    _data_source = ds;
    if(_data_source) {
	_data_source->addOwner(this);
    }
}

void GTimeSeries::setDataMethods(gvector<DataMethod *> *new_methods)
{
    data_methods.clear();
    for(int i = 0; i < (int)new_methods->size(); i++) {
	data_methods.add(new_methods->at(i));
    }
}

DataMethod * GTimeSeries::getMethod(const char *method_name)
{
    for(int i = data_methods.size()-1; i >= 0; i--) {
	if(!strcmp(data_methods[i]->methodName(), method_name)) {
	    return data_methods[i];
	}
    }
    return (DataMethod *)NULL;
}

int GTimeSeries::getMethods(const char *method_name, gvector<DataMethod *> &v)
{
    v.clear();
    for(int i = 0; i < data_methods.size(); i++) {
	if(!strcmp(data_methods[i]->methodName(), method_name)) {
	    v.push_back(data_methods[i]);
	}
    }
    return (int)v.size();
}

/** Remove a DataMethod object from the sequence of DataMethods that have
 *  been applied to this GTimeSeries.
 *  @param[in] dm the DataMethod object to remove.
 *  @returns true if the method was successfully removed. Returns false if the
 *  method could not be removed from the waveform.
 */
bool GTimeSeries::removeMethod(DataMethod *dm, bool reapply)
{
    for(int i = 0; i < data_methods.size(); i++) {
	if(data_methods[i] == dm) {
	    data_methods.removeAt(i);
	    if(reapply) {
		reread();
		return applyMethods();
	    }
	    return true;
	}
    }
    return false;
}

/** Remove a DataMethod from the sequence of DataMethods that have
 *  been applied to this GTimeSeries.
 *  @param[in] dm the DataMethod object to remove.
 *  @returns true if the method was successfully removed. Returns false if the
 *  method could not be removed from the waveform.
 */
bool GTimeSeries::removeMethod(const char *method_name, bool reapply)
{
    for(int i = 0; i < data_methods.size(); i++) {
	if(!strcmp(data_methods[i]->methodName(), method_name)) {
	    data_methods.removeAt(i);
	    if(reapply) {
		reread();
		return applyMethods();
	    }
	    return true;
	}
    }
    return false;
}

bool GTimeSeries::applyMethods()
{
    GTimeSeries *ts[1]; ts[0] = this;
    for(int i = 0; i < (int)data_methods.size(); i++) {
	if(!data_methods[i]->applyMethod(1, ts)) return false;
    }
    return true;
}

/** Remove all DataMethod objects. This will force the raw data to be reread
 *  if there are any existing DataMethods.
 *  @returns true if the methods were removed.
 */
bool GTimeSeries::removeAllMethods()
{
    if(data_methods.size() > 0) {
        data_methods.clear();
        return reread();
    }
    return false;
}

