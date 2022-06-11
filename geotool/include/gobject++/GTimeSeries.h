#ifndef _GTIME_SERIES_H_
#define _GTIME_SERIES_H_

#include "gobject++/Gobject.h"
#include "gobject++/GSegment.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/ghashtable.h"
#include "gobject++/gvector.h"
#include "gobject++/GSourceInfo.h"
#include "gobject++/DataSource.h"
#include "DataMethod.h"
#include "BeamSta.h"
#include "SegmentInfo.h"
#include "gobject++/WaveformTag.h"
#include "gobject++/CssTables.h"

class ResponseFile;
class Response;

/** A class that holds waveform data as a sequence of data segments. The
 *  GTimeSeries object is built up from GSegment objects. At any time, a new
 *  GSegment object can be added to the GTimeSeries object. Internally, two
 *  adjacent GSegment objects are joined into one GSegment object, if the data
 *  gap between them is equal to one sample time interval, and the adjacent
 *  GSegment objects have the same sample time interval, the same calibration
 *  factor, and the same calibration period. The data from a new GSegment
 *  object replaces any data that it overlaps.
 *
 *  The sample time increment tolerance, tdel_tolerance, determines if
 *  adjacent segments will be joined when their sample time increments, tdel,
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
 *   .99.
 *
 *  The calibration factor tolerance, calib_tolerance, also determines if
 *  adjacent segments will be joined when their calibration factors are not the
 *  same. To adjacent segments will be joined only if the percent change in
 *  the calibration factor is less than calib_tolerance. The default value of
 *  calib_tolerance is .01.
 *  @ingroup libgmethod++
 */
class GTimeSeries : public Gobject
{
    public:
	GTimeSeries(void);
	GTimeSeries(double sample_time_tolerance, double calibration_tolerance);
	GTimeSeries(GSegment **segments, int segments_length);
	GTimeSeries(GSegment *segment);
	GTimeSeries(const GTimeSeries &ts);
	GTimeSeries(const GTimeSeries *ts);
	GTimeSeries & operator=(const GTimeSeries &ts);

	~GTimeSeries(void);
	
	virtual Gobject *clone(void);

	virtual void addSegment(GSegment *seg, bool join=true) throw(int);
	void joinSegments(void) throw(int);
	void fillAllGaps(int maxGap);
	void fillGap(int segment_index, int maxGap);
	void join(GTimeSeries *t);
	bool continuous(int i, double tbeg_tol, double tdel_tol);
	void copyInto(float *data);
	double dataMax(void);
	GDataPoint *maxPoint(void);
	double dataMin(void);
	GDataPoint *minPoint(void);
	bool dataMinMax(GDataPoint *d1, GDataPoint *d2,double *min,double *max);
	double dataMin(double t1, double t2);
	double dataMax(double t1, double t2);
	void dataStats(double *dmin, double *dmax, double *dmean);
	GDataPoint *lowerBound(double time);
	double mean(void);
	GDataPoint *nearest(double time);
	GSegment *segment(double time);
	GSegment *nearestSegment(double time);
	GTimeSeries *subseries(double t1, double t2);
	bool truncate(double t1, double t2);
	void makeCopy(void);

	/** Get the time of the first data sample.
	 *  @returns the epochal time of the first data sample or returns 0., if
 	 *  there is no data in the GTimeSeries object.
	 */
	double tbeg() { return (nsegs > 0) ? s[0]->tbeg() : 0.; }
	/** Get the time of the last data sample.
	 *  @returns the epochal time of the last data sample or returns 0., if
 	 *  there is no data in the GTimeSeries object.
	 */
	double tend() { return (nsegs > 0) ? s[nsegs-1]->tend() : 0.; }
	/** Get the time duration.
	 *  @returns the time duration, tend() - tbeg().
	 */
	double duration() {
	    return (nsegs > 0) ? s[nsegs-1]->tend() - s[0]->tbeg() : 0.; }

	GDataPoint *upperBound(double time);
	void setData(float *data);
	void removeAllSegments(void);
	void setData(GTimeSeries *ts);
	GSegment *getSegment(int index);
	double time(int index);
	int flaggedArrays(GDataPoint *d1, GDataPoint *d2,float **pt,float **py)
		throw(int);

	/** Store a character string in the hashtable. The string is copied.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the character string.
	 */
	void putValue(const string &name, char *value) {
	    string string_value(value);
	    shashtable.put(name, &string_value);
	}
	/** Store a float value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the float value.
	 */
	void putValue(const string &name, float value) {
	    dhashtable.put(name, (double)value);
	}
	/** Store a double value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the double value.
	 */
	void putValue(const string &name, double value) {
	    dhashtable.put(name, value);
	}
	/** Store a int value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the int value.
	 */
	void putValue(const string &name, int value) {
	    ihashtable.put(name, value);
	}
	/** Store a long value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the long value.
	 */
	void putValue(const string &name, long value) {
	    lhashtable.put(name, value);
	}
	/** Store a Gobject in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the Gobject.
	 */
	void putValue(const string &name, Gobject *value) {
	    vhashtable.put(name, value);
	}

	/** Get a string value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the string value.
	 *  @param[in] len the length of value[].
	 *  @returns true if the name was found in the hashtable.
	 *  Returns false if not.
	 */
	bool getValue(const string &name, char *value, int len)
	{
	    string *string_value;
	    if(shashtable.get(name, &string_value)) {
		strncpy(value, string_value->c_str(), len);
		value[len-1] = '\0';
		return true;
	    }
	    return false;
	}

	/** Get a float value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the float value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, float *value)
	{
	    double d;
	    if(dhashtable.get(name, &d)) {
		*value = (float)d;
		return true;
	    }
	    return false;
	}

	/** Get a double value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the double value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, double *value)
	{
	    return dhashtable.get(name, value);
	}

	/** Get a int value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the int value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, int *value)
	{
	    return ihashtable.get(name, value);
	}

	/** Get a long value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the long value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, long *value)
	{
	    return lhashtable.get(name, value);
	}

	/** Get a Gobject from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @returns the Gobject or NULL if the name is not in the hashtable.
	 */
	Gobject *getValue(const string &name)
	{
	    Gobject *o;
	    if(vhashtable.get(name, &o)) {
		return o;
	    }
	    return NULL;
	}

	/** Remove a value from the hastable.
	 *  @param[in] name the hashtable key.
	 */
	void removeValue(const string &name) {
	    shashtable.remove(name);
	    dhashtable.remove(name);
	    ihashtable.remove(name);
	    lhashtable.remove(name);
	    vhashtable.remove(name);
 	}

	/** Get the number of GSegments.
	 *  @returns the number of GSegment objects in the GTimeSeries.
	 */
	int size(void) { return nsegs; }

	/** Get the number of samples.
	 *  @returns the total number of samples in the GTimeSeries.
	 */
	int length(void) { return npts; }

	/** Get the i'th GSegment.
	 *  @returns the i'th GSegment or NULL, if i < 0 || i >= size().
	 */
	GSegment *segment(int i) {
	   return (i >= 0 && i < nsegs) ? s[i] : NULL;
	}

	/** Get the sample time increment tolerance.
	 *  @returns tdel_tolerance.
	 */
	double tdelTolerance(void) { return tdel_tolerance; }

	/** Get the calibration factor tolerance.
	 *  @returns calib_tolerance.
	 */
	double calibTolerance(void) { return calib_tolerance; }

	GTimeSeries *decimate(int rate, bool clone_hashtable=false);
	double demean(void);
	CssWfdiscClass *getWfdisc(void);
	CssWfdiscClass *getWfdisc(double sample_time) {
	    WfdiscPeriod *wp = getWfdiscPeriod(sample_time);
	    return (wp) ? &wp->wf : NULL;
	}
	WfdiscPeriod *getWfdiscPeriod(double time);
	vector<WfdiscPeriod> *getWfdiscPeriods(void) {
	    if(waveform_io) return &waveform_io->wp;
	    return NULL;
	}
	void copyWfdiscPeriods(GTimeSeries *ts);
	long getWfid(void);
	cvector<CssWfdiscClass> * getWfdiscs(void);

	const char *sta(void) { return station_name.c_str(); }
	bool getSta(string &station) {
	    if(!station_name.empty()) {
		station.assign(station_name);
		return true;
	    }
	    else {
		station.assign("-");
		return false;
	    }
 	}
	const char *chan(void) { return channel_name.c_str(); }
	bool getChan(string &channel) {
	    if(!channel_name.empty()) {
		channel.assign(channel_name);
		return true;
	    }
	    else {
		channel.assign("-");
		return false;
	    }
 	}
	const char *net(void) { return network_name.c_str(); }
	bool getNet(string &network) {
	    if(!network_name.empty()) {
		network.assign(network_name);
		return true;
	    }
	    else {
		network.assign("-");
		return false;
	    }
 	}
	const char *refsta(void) { return refsta_name.c_str(); }
	bool getRefsta(string &refstation) {
	    if(!refsta_name.empty()) {
		refstation.assign(refsta_name);
		return true;
	    }
	    else {
		refstation.assign("-");
		return false;
	    }
 	}
	const char *xChan(void) { return x_chan_name.c_str(); }
	bool getXChan(string &x_chan) {
	    if(!x_chan_name.empty()) {
		x_chan.assign(x_chan_name);
		return true;
	    }
	    else {
		x_chan.assign("-");
		return false;
	    }
 	}
	const char *yChan(void) { return y_chan_name.c_str(); }
	bool getYChan(string &y_chan) {
	    if(!y_chan_name.empty()) {
		y_chan.assign(y_chan_name);
		return true;
	    }
	    else {
		y_chan.assign("-");
		return false;
	    }
 	}
	const char *zChan(void) { return z_chan_name.c_str(); }
	bool getZChan(string &z_chan) {
	    if(!z_chan_name.empty()) {
		z_chan.assign(z_chan_name);
		return true;
	    }
	    else {
		z_chan.assign("-");
		return false;
	    }
 	}
	const char *instype(void) { return instrument_code.c_str(); }
	bool getInstype(string &instrument_type) {
	    if(!instrument_code.empty()) {
		instrument_type.assign(instrument_code);
		return true;
	    }
	    else {
		instrument_type.assign("-");
		return false;
	    }
 	}
	const char *segtype(void) { return seg_type.c_str(); }
	bool getSegtype(string &segment_type) {
	    if(!seg_type.empty()) {
		segment_type.assign(seg_type);
		return true;
	    }
	    else {
		segment_type.assign("-");
		return false;
	    }
 	}
	const char *datatype(void) { return data_type.c_str(); }
	bool getDatatype(string &dataType) {
	    if(!data_type.empty()) {
		dataType.assign(data_type);
		return true;
	    }
	    else {
		dataType.assign("-");
		return false;
	    }
 	}
	const char *clip(void) { return clip_type.c_str(); }
	bool getClip(string &clipType) {
	    if(!clip_type.empty()) {
		clipType.assign(clip_type);
		return true;
	    }
	    else {
		clipType.assign("-");
		return false;
	    }
 	}

	void setSta(string station) {
	    station_name = station;
	}
	void setChan(string channel) {
	    channel_name = channel;
	}
	void setNet(string network) {
	    network_name = network;
	}
	void setRefsta(string refstation) {
	    refsta_name = refstation;
	}
	void setXChan(string xchan) {
	    x_chan_name = xchan;
	}
	void setYChan(string ychan) {
	    y_chan_name = ychan;
	}
	void setZChan(string zchan) {
	    z_chan_name = zchan;
	}
	void setInstype(string instrument_type) {
	    instrument_code = instrument_type;
	}
	void setSegtype(string segment_type) {
	    seg_type = segment_type;
	}
	void setDatatype(string dataType) {
	    data_type = dataType;
	}
	void setClip(string clipType) {
	    clip_type = clipType;
	}

	int chanid(void) { return station_chanid; }
	double lat(void) { return station_lat; }
	double lon(void) { return station_lon; }
	double elev(void) { return station_elev; }
	double dnorth(void) { return station_dnorth; }
	double deast(void) { return station_deast; }
	double hang(void) { return station_hang; }
	double vang(void) { return station_vang; }
	double alpha(void) { return station_alpha; }
	double beta(void) { return station_beta; }
	double gamma(void) { return station_gamma; }
	double currentAlpha(void) { return current_alpha; }
	double currentBeta(void) { return current_beta; }
	double currentGamma(void) { return current_gamma; }
	int component(void) { return component_code; }

	void setChanid(int channel_id) { station_chanid = channel_id; }
	void setLat(double latitude) { station_lat = latitude; }
	void setLon(double longitude) { station_lon = longitude; }
	void setElev(double elevation) { station_elev = elevation; }
	void setDnorth(double sta_dnorth) { station_dnorth = sta_dnorth;}
	void setDeast(double sta_deast) { station_deast = sta_deast; }
	void setHang(double sta_hang) { station_hang = sta_hang; }
	void setVang(double sta_vang) { station_vang = sta_vang; }
	void setAlpha(double sta_alpha) { station_alpha = sta_alpha;}
	void setBeta(double sta_beta) { station_beta = sta_beta; }
	void setGamma(double sta_gamma) { station_gamma = sta_gamma;}
	void setCurrentAlpha(double cur_alpha) {current_alpha = cur_alpha;}
	void setCurrentBeta(double cur_beta) { current_beta = cur_beta; }
	void setCurrentGamma(double cur_gamma) { current_gamma = cur_gamma;}
	void setComponent(int comp) { component_code = comp;}

	double originalStart(void) { return original_tbeg; }
	double originalEnd(void) { return original_tend; }
	double selectionStart(void) { return selection_start; }
	double selectionEnd(void) { return selection_end; }

	void setOriginalStart(double start) { original_tbeg = start; }
	void setOriginalEnd(double end) { original_tend = end; }
	void setSelectionStart(double start) { selection_start = start; }
	void setSelectionEnd(double end) { selection_end = end; }

	long jdate(void) { return julian_date; }
	void setJDate(long jDate) { julian_date = jDate; }

	gvector<DataMethod *> *dataMethods(void) {
	    return new gvector<DataMethod *>(data_methods); }
	void addDataMethod(DataMethod *dm) { data_methods.add(dm); }
	void setDataMethods(gvector<DataMethod *> *new_methods);
	DataMethod *getMethod(const char *method_name);
	int getMethods(const char *method_name, gvector<DataMethod *> &v);
	bool removeMethod(DataMethod *dm, bool reapply=true);
	bool removeMethod(const char *method_name, bool reapply=true);
	bool removeAllMethods();
	bool applyMethods();

	void copyInfo(const GTimeSeries &ts, bool clone_hashtable=true);
	virtual bool reread(void);

	void setDataSource(DataSource *ds);
	DataSource * getDataSource() { return _data_source; }

	ResponseFile	*response_file;
	vector<Response *> response; //!< Response objects
	bool		no_response;
	int		inid;

	vector<BeamSta> beam_elements;

	WaveformIO	*waveform_io;

	gvector<SegmentInfo *> array_elements;

	WaveformTag	tag;

	GSourceInfo	source_info;

    protected:
	/** The total number of data samples in this GTimeSeries. */
	int		npts;
	GSegment	**s;	//!< An array of GSegment objects.
	int		nsegs;  //!< The number of GSegment objects in s[].
	double		tdel_tolerance;//!< The sample time increment tolerance.
	double		calib_tolerance;//!< The calibration factor tolerance.
	double		time_tolerance;//!< Tolerance for time inputs.
	GTimeSeries	*copy;

	ghashtable<string *>	shashtable;
	ghashtable<double>	dhashtable;
	ghashtable<int>		ihashtable;
	ghashtable<long>	lhashtable;
	ghashtable<Gobject *>	vhashtable;

	string		station_name; //!< Station name
	string		channel_name; //!< Channel name
	string		network_name; //!< Network name
	string		refsta_name;  //!< Array reference station name
	string		x_chan_name;  //!< Channel name of the x-axis component
	string		y_chan_name;  //!< Channel name of the y-axis component
	string		z_chan_name;  //!< Channel name of the z-axis component
	string		instrument_code; //!< Instrument code
	string		seg_type;  //!< segtype
	string		data_type; //!< datatype
	string		clip_type; //!< clip

	int		station_chanid; //!< Station channel id
	double		station_lat;    //!< Station latitude
	double		station_lon;    //!< Station latitude
	double		station_elev;   //!< Station elevation
	double		station_dnorth; //!< Array element dnorth offset
	double		station_deast;  //!< Array element deast offset
	double		station_hang;   //!< Horizontal orientation angle
	double		station_vang;   //!< Vertical orientation angle
	double		station_alpha;  //!< Station orientation Euler angle
	double		station_beta;   //!< Station orientation Euler angle
	double		station_gamma;  //!< Station orientation Euler angle
	double		current_alpha;  //!< Current orientation Euler angle
	double		current_beta;   //!< Current orientation Euler angle
	double		current_gamma;  //!< Current orientation Euler angle
	int		component_code; //!< 1: x, 2: y, 3: z

	double		original_tbeg; //!< Original start time
	double		original_tend; //!< Original end time

	double		selection_start; //!< Selection start time
	double		selection_end;   //!< Selection end time

	long		julian_date; //!< Julian date

	/** True if this waveform was created from other waveforms. */
	bool		derived;

	gvector<DataMethod *> data_methods; //!< DataMethod objects

	DataSource	*_data_source;

	void init(void);
};
#endif
