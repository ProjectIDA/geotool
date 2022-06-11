#ifndef _BASIC_SOURCE_H
#define _BASIC_SOURCE_H

#include <string.h>
#include <string>
#include "widget/Parser.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/cvector.h"
#include "gobject++/CssTables.h"
using namespace std;

extern "C" {
#include "libstring.h"
}
class WaveformPlot;

/** An interface for subclasses that support CSS table I/O.
 *  @ingroup libgx
 */
class BasicSource : public Parser
{
    public:
	~BasicSource(void);

	gvector<SegmentInfo *> *getSegmentList(void);
	int readData(gvector<SegmentInfo *> *seginfo,
		double start_time, double end_time,
		int pts, bool preview_arr, GTimeSeries **ts,
		cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
		cvector<CssHydroFeaturesClass> &hydro_features,
		cvector<CssInfraFeaturesClass> &infra_features,
		cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
		cvector<CssAmplitudeClass> &amplitudes,
		cvector<CssAmpdescriptClass> &ampdescripts,
		cvector<CssParrivalClass> &parrivals, const char **err_msg);
	int getWaveforms(gvector<Waveform *> &wvec, bool displayed_only=true);
	int getPathInfo(PathInfo **path_info);
	Waveform *getWaveform(int id);
	int getSelectedWaveforms(gvector<Waveform *> &wvec) {
		return getWaveforms(wvec);
	}
	int getSelectedWaveforms(const string &cursor_label,
                        gvector<Waveform *> &wvec) {
	    return getSelectedWaveforms(wvec);
	}
	int getArrivalsOnWaveform(Waveform *w, cvector<CssArrivalClass> &arrivals);
	bool reread(GTimeSeries *ts);
        virtual bool getChanid(const string &sta, const string &chan,
				long *chanid);
	int getNetwork(const string &net, GStation ***stations);
	string getNet(const string &sta);
	bool changeTable(CssTableClass *old_table, CssTableClass *new_table);
	bool addTable(CssTableClass *css);
	bool addAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude);
	GTimeSeries *readTimeSeries(cvector<CssWfdiscClass> &wfdiscs, double tbeg,
			double tend);
	bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg);
	CssSiteClass *getSite(const string &sta, int jdate);
	CssSitechanClass *getSitechan(const string &sta, const string &chan,
		int jdate);

	CssTableClass *createTable(CssTableClass *table, const string &table_name,
		const string &id_name, Password password);
	bool deleteTable(Component *caller, CssTableClass *table,
		const string &table_name="");
	bool addArrival(CssArrivalClass *arrival, GTimeSeries *ts, Password password,
		long max_arid);
	bool changeArrival(CssArrivalClass *arrival, GTimeSeries *ts, int type);
	bool deleteArrival(Component *caller, CssArrivalClass *arrival,
		cvector<CssAssocClass> &assocs, cvector<CssAmplitudeClass> &amps,
		cvector<CssStamagClass> &stamags, cvector<CssHydroFeaturesClass> &hydros,
		cvector<CssInfraFeaturesClass> &infras);
	bool originCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *table, Password password);
	bool deleteOrigin(Component *caller, CssOriginClass *origin,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &wftags, cvector<CssNetmagClass> &netmags,
		cvector<CssStamagClass> &stamags);
	bool addAssoc(CssAssocClass *assoc);
	CssOrigerrClass *addOrigerr(CssOriginClass *origin);
	bool addNetmag(CssNetmagClass *netmag, CssTableClass *table);
	bool addStamag(CssStamagClass *stamag, CssTableClass *table);
	virtual bool undoDeleteTable(CssTableClass *table);

	virtual vector<Response *> * channelResponse(GTimeSeries *ts,
		bool print_err=true) {
		return getInstrumentResponse(ts, print_err); }

	static GSegment *readSegment(CssWfdiscClass *wfdisc,
		const string &working_dir, double start_time, double end_time,
		int pts_wanted);
	static FILE * openBackupFile(void);
	/** Get the instrument response for a waveform. Do not warn if it
	 *  cannot be found.
	 *  @param[in] ts a GTimeSeries object.
	 *  @returns a vector of Response objects, one for each of the
	 *  components of the cascaded response. Returns NULL if ts->length()
 	 *  <= 2 or the response cannot be found.
	 */
	static vector<Response *> *getResponse(GTimeSeries *ts,
			bool print_err=true);
	static vector<Response *> *getInstrumentResponse(GTimeSeries *ts,
			bool print_err=true);
	static bool getResponseFile(GTimeSeries *ts, string &insname,
		string &file, int *inid, const char **err_msg);
	static bool checkForBeam(GTimeSeries *ts, string &sta, string &chan);

	/** Convert an amplitude from counts to nanometers.
	 *  @param[in] ts a GTimeSeries object.
	 *  @param[in] s a GSegment member of ts used for calib.
	 *  @param[in] ampcts the amplitude in counts.
	 *  @param[in] period the period in seconds.
	 *  @param[out] ampnms the amplitude in nanometers.
	 *  @param[in] warn if true, warn about missing instrument response.
	 *  @returns true for success, returns false if the response cannot be
	 *  found, or the calper is <= 0.
	 */
	static bool cts2nms(GTimeSeries *ts, GSegment *s, double ampcts,
			double period, double *ampnms, bool warn=true);

	/** Convert an amplitude from nanometers to counts. Do not warn if the
	 *	response cannot be found.
	 *  @param[in] ts a GTimeSeries object.
	 *  @param[in] s a GSegment member of ts used for calib.
	 *  @param[in] ampnms the amplitude in nanometers.
	 *  @param[in] period the period in seconds.
	 *  @param[out] ampcts the amplitude in counts.
	 *  @returns true for success, returns false if the response cannot be
	 *  found, or the calper is <= 0.
	 */
	static bool nms2cts(GTimeSeries *ts, GSegment *s, double ampnms,
			double period, double *ampcts, bool warn=true);

	static bool newMissingResp(int sta, int chan);
	static bool startBackup(void);
	static bool undoFileModification(void);
	static bool isArray(const string &net);
	static bool isArrayChannel(const string &net, const string &sta,
		const string &chan);
	static bool isThreeComponentChannel(const string &sta,
		const string &chan);
	static void copyTable(CssTableClass *t2, CssTableClass *t1) {
		t1->copyTo(t2, true);
		t2->setIds(t1->getDC(), 1);
	}

    protected:
	BasicSource(const string &name);

	void loadAllTables(GTimeSeries *ts, SegmentInfo *s,
		cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
		cvector<CssXtagClass> &xtags,
		cvector<CssHydroFeaturesClass> &hydro_features,
		cvector<CssInfraFeaturesClass> &infra_features,
		cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
		cvector<CssAmplitudeClass> &amplitudes,
		cvector<CssAmpdescriptClass> &ampdescripts,
		cvector<CssParrivalClass> &parrivals);
	void loadTsDatabase(GTimeSeries *ts, SegmentInfo *s);
	void loadArrivals(GTimeSeries *ts, const string &sta, const string &net,
		cvector<CssArrivalClass> &arrivals);
	void loadAssocs(cvector<CssArrivalClass> &arrivals,
		cvector<CssAssocClass> &assocs);
	void loadStassocs(cvector<CssArrivalClass> &arrivals,
		cvector<CssStassocClass> &stassocs);
	void loadHydroFeatures(cvector<CssArrivalClass> &arrivals,
		cvector<CssHydroFeaturesClass> &hydro_features);
	void loadInfraFeatures(cvector<CssArrivalClass> &arrivals,
		cvector<CssInfraFeaturesClass> &infra_features);
	void loadStamags(cvector<CssArrivalClass> &arrivals,
		cvector<CssStamagClass> &stamags);
	void loadOridWftags(SegmentInfo *s, cvector<CssWftagClass> &wftags);
	void loadXtags(SegmentInfo *s, cvector<CssXtagClass> &xtags);
	void loadOrigins(GTimeSeries *ts, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &wftags, cvector<CssXtagClass> &xtags,
		cvector<CssOriginClass> &origins);
	void loadOrigerrs(cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs);
	void loadNetmags(cvector<CssOriginClass> &origins,
		cvector<CssNetmagClass> &netmags);
	void loadParrivals(cvector<CssOriginClass> &origins,
		cvector<CssParrivalClass> &parrivals);
	void loadAmplitudes(cvector<CssArrivalClass> &arrivals,
		cvector<CssAmplitudeClass> &amplitudes,
		cvector<CssAmpdescriptClass> &ampdescripts);
	void getDir(const string &full_path, string &dir, string &prefix);
	void getNetworks(gvector<SegmentInfo *> *segs);
	void getSites(gvector<SegmentInfo *> *segs);
	void getSitechans(gvector<SegmentInfo *> *segs);
	void resetLoaded(void);
	void resetLoaded(const string &name);
	void addStations(gvector<SegmentInfo *> *segs);
	int networkStations(const string &net, const char ***elements);
	int getChannels(const string &sta, const char ***channels);
	void fillStations(const string &net, int jdate);
	static void getComponentOrientation(SegmentInfo *s, CssSitechanClass **sc,
		CssSitechanClass *this_sc);

        /** Returns true if orid wftags should be created when necessary to link
         *  origins to waveforms. This is generally true when reading file
         *  prefix data sources.
         */
        virtual bool createOridWftags(void) { return false; }

	virtual double getStartTime(void) { return NULL_TIME; }
	virtual double getEndTime(void) { return NULL_TIME; }
	virtual bool sourceIsODBC(void) { return false; }
	virtual void query(const string &) {}
	virtual void getNetworkTables(void) {}

	gvector<Waveform *> waveforms;
	int nextid;

    private:

#define MAX_MSG 51200
	char error_msg[MAX_MSG+1];

};

#endif
