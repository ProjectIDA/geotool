#ifndef _DATA_SOURCE_H
#define _DATA_SOURCE_H

#include <iostream>
#include <vector>

using namespace std;
#include "gobject++/cvector.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}
 
class DataReceiver;
class Component;
class SegmentInfo;
class WaveformWindow;
class WaveformPlot;
class TableViewer;
class Waveform;
class GTimeSeries;
class Response;

#define XtNdataChangeCallback   (char *)"dataChangeCallback"

#define CHANGE_TIME		1
#define CHANGE_AMP_PER		2
#define CHANGE_PHASE_NAME	4
#define CHANGE_STASSID		8
#define CHANGE_AZIMUTH		16
#define CHANGE_SLOW		32

class DataChange
{
    public:
	DataChange() :
	    select_waveform(false),
	    sort_waveforms(false),
	    select_arrival(false),
	    select_origin(false),
	    waveform(false),
	    arrival(false),
	    assoc(false),
	    origin(false),
	    origerr(false),
	    stassoc(false),
	    stamag(false),
	    netmag(false),
	    hydro(false),
	    infra(false),
	    wftag(false),
	    amplitude(false),
	    ampdescript(false),
	    filter(false),
	    parrival(false),
	    unknown(false),
	    unknown_select(false),
	    primary_origin(false),
	    working_orid(false)
	{}

	void setAll(bool state)
	{
	    select_waveform = state;
	    sort_waveforms = state;
	    select_arrival = state;
	    select_origin = state;
	    waveform = state;
	    arrival = state;
	    assoc = state;
	    origin = state;
	    origerr = state;
	    stassoc = state;
	    stamag = state;
	    netmag = state;
	    hydro = state;
	    infra = state;
	    wftag = state;
	    amplitude = state;
	    ampdescript = state;
	    filter = state;
	    parrival = state;
	    unknown = state;
	    unknown_select = state;
	    primary_origin = state;
	    working_orid = state;
	}

	bool select_waveform;
	bool sort_waveforms;
	bool select_arrival;
	bool select_origin;
	bool waveform;
	bool arrival;
	bool assoc;
	bool origin;
	bool origerr;
	bool stassoc;
	bool stamag;
	bool netmag;
	bool hydro;
	bool infra;
	bool wftag;
	bool amplitude;
	bool ampdescript;
	bool filter;
	bool parrival;
	bool unknown;
	bool unknown_select;
	bool primary_origin;
	bool working_orid;
};

typedef struct
{
    int		sta;
    int		net;
    int		refsta;
    int		*chan;
    int		nchan;
    double	lat;
    double	lon;
    double	elev;
} GStation;

typedef struct
{
    char	sta[7];
    double	lat;
    double	lon;
    CssOriginClass	*origin;
    int		fg;
} PathInfo;

typedef struct passwd * Password;

#define DataSource_warn(a) cerr <<"DataSource warning: no "<<a<<" routine\n"

/** A virtual interface for classes that support data I/O.
 *  @ingroup libgmethod
 */
class DataSource
{
    public:
	virtual ~DataSource(void);

	/** Get the Waveform objects for waveforms that are displayed.
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @param[in] displayed_only if true, return only waveforms that are
	 *	displayed, otherwise return all waveforms.
	 *  @returns the number of objects in wvec[].
	 */
	virtual int getWaveforms(gvector<Waveform *> &wvec,
		bool displayed_only=true) {
		DataSource_warn("getWaveforms"); return 0;}
	/** Get the PathInfo for all waveforms.
	 *  @param[out] path_info an allocated array of PathInfo structures.
	 *	- p->lat the station latitude
	 *	- p->lon the station longitude
	 *	- p->origin the associated origin
	 *	- p->fg the waveform color.
	 */
	virtual int getPathInfo(PathInfo **path_info) {
		DataSource_warn("getPathInfo"); return 0;}
	/** Get the Waveform object for the specified id.
	 *  @param[in] id the id of the waveform.
	 *  @returns a Waveform object or NULL.
	 */
	virtual Waveform *getWaveform(int id) {
		DataSource_warn("getWaveform"); return NULL;}
	/** Get Waveform objects for all selected waveforms.
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @returns the number of objects in wvec[].
	 */
	virtual int getSelectedWaveforms(gvector<Waveform *> &wvec) {
		DataSource_warn("getSelectedWaveforms"); return 0;}
	/** Get cloned GTimeSeries objects for selected waveforms.
	 *  @param[out] ts_list an allocated array of GTimeSeries objects.
	 *	Free space with ts_list[i]->deleteObject and free(ts_list).
	 *  @returns the number of objects in ts_list[].
	 */
	virtual int copySelectedTimeSeries(gvector<GTimeSeries *> &tvec) {
		DataSource_warn("copySelectedTimeSeries"); return 0;}
	/** Get Waveform objects for selected waveforms within a window.
	 *  @param[in] cursor_label the character label of the cursor window.
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @returns the number of objects in wvec[].
	 */
	virtual int getSelectedWaveforms(const string &cursor_label,
			gvector<Waveform *> &wvec) {
		DataSource_warn("getSelectedWaveforms"); return -1;}
	/** Get Waveform objects for selected components within a window.
	 *  @param[in] cursor_label the character label of the cursor window.
	 *  @param[in] comps the components to return (eg. "zne").
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @returns the number of objects in wvec[].
	 */
	virtual int getSelectedComponents(const string &cursor_label,
			const string &comps, gvector<Waveform *> &wvec) {
		DataSource_warn("getSelectedComponents 1"); return -1;}
	/** Get Waveform objects for selected components.
	 *  @param[in] comps the components to return (eg. "zne").
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @returns the number of objects in wvec[].
	 */
	virtual int getSelectedComponents(const string &comps,
		gvector<Waveform *> &wvec) {
		DataSource_warn("getSelectedComponents 2"); return 0;}
	/** Get Waveform objects for selected components.
	 *  @param[in] num_cmpts the number of components (2 or 3) in each group
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @returns the number of groups (elements in num_cmpts).
	 */
	virtual int getSelectedComponents(vector<int> &num_cmpts,
			gvector<Waveform *> &wvec) {
		DataSource_warn("getSelectedComponents 3"); return 0;}
	/** Get Waveform objects for selected components within a window.
	 *  @param[in] cursor_label the character label of the cursor window.
	 *  @param[out] num_cmpts the number of components (2 or 3) in each group
	 *  @param[out] wvec an allocated array of Waveform objects.
	 *  @returns the number of groups (elements in num_cmpts).
	 */
	virtual int getSelectedComponents(const string &cursor_label,
		vector<int> &num_cmpts, gvector<Waveform *> &wvec) {
		DataSource_warn("getSelectedComponents 4"); return 0;}
	/** Check if a cursor window is displayed.
	 *  @param[in] cursor_label the label of the cursor time window.
	 *  @returns true if the cursor window is displayed.
	 */
	virtual bool dataWindowIsDisplayed(const string &cursor_label) { 
		DataSource_warn("dataWindowIsDisplayed"); return false; }
	/** Get the origin that is associated with the specified waveform.
	 *  @param[in] w the Waveform object for the waveform.
	 *  @returns the associated origin or NULL.
	 */
	virtual CssOriginClass * getPrimaryOrigin(Waveform *w) { 
		DataSource_warn("getPrimaryOrigin"); return NULL; }
	/** Get the working orid.
	 *  @returns the working orid or -1 if none is set.
	 */
	virtual long getWorkingOrid(void) {
		DataSource_warn("getWorkingOrid"); return -1; }
	/** Set the working orid.
	 *  @param[in] orid the new working orid.
	 *  @param[in] do_callback If true, do a DataChange callback.
	 */
	virtual void setWorkingOrid(long orid, bool do_callback=true) {
		DataSource_warn("setWorkingOrid"); }

	/** Get the arrivals that are associated with the waveform.
	 *  @param[in] w the Waveform object for the waveform.
	 *  @param[out] arrivals an allocated array of CssArrival objects.
	 *  @returns the number of objects in arrivals[].
	 */
	virtual int getArrivalsOnWaveform(Waveform *w,
		cvector<CssArrivalClass> &arrivals){
		DataSource_warn("getArrivalsOnWaveform"); return 0;}

	// getTable routines

	/** Get a Vector of CssTableClass objects.
	 *  @param[in] cssTableName the table name.
	 *  @param[in] copy If true, allocate a new Vector for return. If false,
	 *	return an internal Vector.
	 *  @returns a Vector or NULL.
	 */
	virtual cvector<CssAffiliationClass> * getAffiliationTable(void) {
	    DataSource_warn("getAffiliationTable"); return NULL;}
	virtual cvector<CssSiteClass> * getSiteTable(void) {
	    DataSource_warn("getSiteTable"); return NULL;}
	virtual cvector<CssSitechanClass> * getSitechanTable(void) {
	    DataSource_warn("getSitechanTable"); return NULL;}

	virtual int getTable(cvector<CssAffiliationClass> &v) {
	    DataSource_warn("getTable(cvector<CssAffiliationClass> &v)"); return -1;}
	virtual int getTable(cvector<CssAmpdescriptClass> &v) {
	    DataSource_warn("getTable(cvector<CssAmpdescript> &v)"); return -1;}
	virtual int getTable(cvector<CssAmplitudeClass> &v) {
	    DataSource_warn("getTable(cvector<CssAmplitude> &v)"); return -1;}
	virtual int getTable(cvector<CssArrivalClass> &v) {
	    DataSource_warn("getTable(cvector<CssArrivalClass> &v)"); return -1;}
	virtual int getTable(cvector<CssAssocClass> &v) {
	    DataSource_warn("getTable(cvector<CssAssocClass> &v)"); return -1;}
	virtual int getTable(cvector<CssInfraFeaturesClass> &v) {
	    DataSource_warn("getTable(cvector<CssInfraFeaturesClass> &v)"); return -1;}
	virtual int getTable(cvector<CssHydroFeaturesClass> &v) {
	    DataSource_warn("getTable(cvector<CssHydroFeaturesClass> &v)"); return -1;}
	virtual int getTable(cvector<CssOriginClass> &v) {
	    DataSource_warn("getTable(cvector<CssOriginClass> &v)"); return -1;}
	virtual int getTable(cvector<CssOrigerrClass> &v) {
	    DataSource_warn("getTable(cvector<CssOrigerrClass> &v)"); return -1;}
	virtual int getTable(cvector<CssParrivalClass> &v) {
	    DataSource_warn("getTable(cvector<CssParrivalClass> &v)"); return -1;}
	virtual int getTable(cvector<CssNetmagClass> &v) {
	    DataSource_warn("getTable(cvector<CssNetmagClass> &v)"); return -1;}
	virtual int getTable(cvector<CssSiteClass> &v) {
	    DataSource_warn("getTable(cvector<CssSiteClass> &v)"); return -1;}
	virtual int getTable(cvector<CssSitechanClass> &v) {
	    DataSource_warn("getTable(cvector<CssSitechanClass> &v)"); return -1;}
	virtual int getTable(cvector<CssStamagClass> &v) {
	    DataSource_warn("getTable(cvector<CssStamagClass> &v)"); return -1;}
	virtual int getTable(cvector<CssStassocClass> &v) {
	    DataSource_warn("getTable(cvector<CssStassocClass> &v)"); return -1;}
	virtual int getTable(cvector<CssWfdiscClass> &v) {
	    DataSource_warn("getTable(cvector<CssWfdiscClass> &v)"); return -1;}
	virtual int getTable(cvector<CssWftagClass> &v) {
	    DataSource_warn("getTable(cvector<CssWftagClass> &v)"); return -1;}
	virtual int getTable(cvector<CssXtagClass> &v) {
	    DataSource_warn("getTable(cvector<CssXtagClass> &v)"); return -1;}

	virtual int getTable(const string &cssTableName, gvector<CssTableClass *> &v) {
	    DataSource_warn("getTable(gvector<CssTableClass *> &v)"); return -1;}

	virtual int getSelectedTable(cvector<CssAffiliationClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssAffiliationClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssAmpdescriptClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssAmpdescriptClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssAmplitudeClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssAmplitudeClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssArrivalClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssArrivalClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssAssocClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssAssocClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssOriginClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssOriginClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssOrigerrClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssOrigerrClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssParrivalClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssParrivalClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssNetmagClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssNetmagClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssSiteClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssSiteClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssSitechanClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssSitechanClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssStamagClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssStamagClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssStassocClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssStassocClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssWfdiscClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssWfdiscClass> &v)"); return -1;}
	virtual int getSelectedTable(cvector<CssWftagClass> &v) {
	    DataSource_warn("getSelectedTable(cvector<CssWftagClass> &v)"); return -1;}

	virtual int getSelectedTable(const string &cssTableName,
			gvector<CssTableClass *> &v) {
		DataSource_warn("getSelectedTable(gvector<CssTableClass *> &v)");
		return -1; }

	/** Remove a CssTableClass record.
	 *  @param[in] a the record object to remove.
	 *  @param[in] do_callback If true, do a DataChange callback.
	 */
	virtual void removeTable(CssTableClass *table, bool do_callback=true) {
		DataSource_warn("removeTable(CssTableClass)"); }
	/** Remove a CssAssoc record.
	 *  @param[in] a the record object to remove.
	 *  @param[in] do_callback If true, do a DataChange callback.
	 */

	/** Add a CssTableClass record.
	 *  @param[in] a the record object to add.
	 *  @param[in] do_callback If true, do a DataChange callback.
	 */
	virtual void putTable(CssTableClass *table, bool do_callback=true) {
		DataSource_warn("putTable(CssTableClass)"); }

	/** Return the selected state of a table object.
	 *  @param[in] table a CssTableClass object
	 *  @returns true if the object is selected in the DataSource.
	 */
	virtual bool isSelected(CssTableClass *table) {
		DataSource_warn("isSelected(CssTableClass)");
		return false;
	}
	/** Set the selected state of a table object.
	 *  @param[in] table set the selected state of the table object.
	 *  @param[in] state a CssTableClass object
	 *  @returns true if the record is found in the DataSource.
	 */
	virtual bool selectTable(CssTableClass *table, bool state) {
		DataSource_warn("selectTable(CssTableClass, bool)");
		return false;
	}

	/** Get CssTableClass objects from table ids.
	 *  @param[in] tableName the CSS table name.
	 *  @param[in] idName the id name ("arid", "orid", etc.")
	 *  @param[in] num_table_ids the number of table ids.
	 *  @param[in] table_ids an array of ids.
	 *  @param[in,out] objects a Vector to receive the CssTableClass objects.
	 *  @returns true if the tableName is recognized; false otherwise.
	 */
	virtual bool getRecordsFromIds(const string &tableName,
		const string &idName, int num_table_ids, long *table_ids,
		gvector<CssTableClass *> & objects) {
		DataSource_warn("getRecordsFromIds"); return false; }

	/** Get a list of segments that are available to read.
	 *  @returns a vector of SegmentInfo objects for the segments.
 	 */
	virtual gvector<SegmentInfo *> *getSegmentList(void){ 
		DataSource_warn("getSegmentList"); return NULL; }
	/** Read waveform data.
	 *  @param[in] seginfo a vector of the SegmentInfo objects.
	 *  @param[in] start_time truncate waveforms to [start_time, end_time].
	 *  @param[in] end_time truncate waveforms to [start_time, end_time].
	 *  @param[in] pts if != 0, then decimate the data by taking every pts sample
	 *  @param[in] preview_arr if true, preview arrivals only.
	 *  @param[out] ts an array of GTimeSeries objects.
	 *  @param[out] arrivals the CssArrival's associated with the waveforms.
	 *  @param[out] origins the CssOrigin's associated with the waveforms.
	 *  @param[out] origerrs the CssOrigerr's associated with the waveforms.
	 *  @param[out] assocs the CssAssoc's associated with the waveforms.
	 *  @param[out] stassocs the CssStassocs's associated with the waveforms.
	 *  @param[out] wftags the CssWftag's associated with the waveforms.
	 *  @param[out] hydro_features the CssHydroFeatures associated with the waveforms.
	 *  @param[out] infra_features the CssInfraFeatures associated with the waveforms.
	 *  @param[out] stamags the CssStamag's associated with the waveforms.
	 *  @param[out] netmags the CssNetmag's associated with the waveforms.
	 *  @param[out] amplitudes the CssAmplitude's associated with the waveforms.
	 *  @param[out] ampdescripts the CssAmdescript's associated with the waveforms.
	 *  @param[out] parrivals the CssParrival's associated with the waveforms.
	 *  @param[out] err_msg a pointer to an error message or NULL.
	 *  @returns the number of waveforms in ts[].
 	 */
	virtual int readData(gvector<SegmentInfo *> *seginfo, double start_time,
		double end_time, int pts, bool preview_arr, GTimeSeries **ts,
		cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
		cvector<CssHydroFeaturesClass> &hydro_features,
		cvector<CssInfraFeaturesClass> &infra_features,
		cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
		cvector<CssAmplitudeClass> &amplitudes,
		cvector<CssAmpdescriptClass> &ampdescripts,
		cvector<CssParrivalClass> &parrivals, const char **err_msg) { 
			DataSource_warn("readData"); return 0; }
	/** Make a GTimeSeries object from CssSegmenInfo objects. This function
	 *  is called initially with *ts == NULL and the first SegmentInfo
	 *  object. It is called repeatedly with a non-null *ts and each
 	 *  SegmentInfo object that is to be included in the waveform.
	 *  @param[in] s the segment to add to the waveform.
	 *  @param[in] tbeg the minimum waveform start time.
	 *  @param[in] tend the maximum waveform end time.
	 *  @param[in] pts the decimation factor (enter 0 for no decimation).
	 *  @param[in,out] ts GTimeSeries object. If *ts == NULL, it is created.
	 *  @param[out] err_msg a pointer to an error message or NULL.
	 *  @returns true for success and false to indicate an error.
	 */
	virtual bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
                int pts, GTimeSeries **ts, const char **err_msg) {
		DataSource_warn("makeTimeSeries"); return 0; }
	/** Reread a GTimeSeries
	 *  @param[in,out] ts the GTimeSeries object to re-read.
	 *  @returns true for success or false for failure.
	 */
	virtual bool reread(GTimeSeries *ts) { 
		DataSource_warn("reread"); return false; }
/*
	virtual bool readTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg) {
		DataSource_warn("readTimeSeries"); return false; }
*/
	/** Get the channel id.
	 *  @param[in] sta the station name.
	 *  @param[in] chan the channel name.
	 *  @param[out] chanid the channel id.
	 *  @returns true for success and false if the id is not found.
	 */
	virtual bool getChanid(const string &sta, const string &chan,
		long *chanid){ DataSource_warn("getChanid"); return false; }
	/** Get the stations in a network.
	 *  @param[in] net the network name.
	 *  @param[out] stations an array of GStation structures.
	 *	- sta the station quark.
	 *	- net the network quark.
	 *	- refsta the reference station quark.
	 *	- chan an array of channel quarks.
	 *	- nchan the number of channel quarks in chan[].
	 *	- lat the station latitude.
	 *	- lon the station longitude.
	 *	- elev the station elevation.
	 *  @returns the number of elements in stations[].
	 */
	virtual int getNetwork(const string &net, GStation ***stations) {
		*stations = NULL; DataSource_warn("getNetwork"); return 0; }

	/** Get the network name.
	 *  @param[in] sta the station name.
	 *  @returns the name of the network that contains the input station.
	 */
	virtual string getNet(const string &sta) {
		DataSource_warn("getNetwork"); return NULL; }

	/** Save reference stations. The input stations are made the new
	 *  reference stations for their respective arrays.
	 *  @param[in] num the number of reference stations.
	 *  @param[in] refsta an array of reference stations to save.
	 */
	virtual bool saveRefSta(int num, const char **refsta) { 
		DataSource_warn("saveRefSta"); return false; }
	/** Get the site table for a station.
	 *  @param[in] sta the station name.
	 *  @param[in] jdate the date (yyyyddd).
	 */
	virtual CssSiteClass * getSite(const string &sta, int jdate) {
		DataSource_warn("getSite"); return NULL; }
	/** Get the sitechan table for a station.
	 *  @param[in] sta the station name.
	 *  @param[in] chan the channel name.
	 *  @param[in] jdate the date (yyyyddd).
	 */
	virtual CssSitechanClass * getSitechan(const string &sta, const string &chan,
		int jdate) { DataSource_warn("getSitechan"); return NULL; }

	/** Add a data listener. The listener is informed when data is changed.
	 *  @param[in] comp the listener component.
	 */
	virtual void addDataListener(Component *comp) {
		DataSource_warn("addDataListener"); }
	/** Remove a data listener.
	 *  @param[in] comp the listener component.
	 */
	virtual void removeDataListener(Component *comp) {
		DataSource_warn("removeDataListener"); }

	/** Select a waveform.
	 *  @param[in] w the Waveform object for the waveform.
	 *  @param[in] select the selected state.
	 */
	virtual void selectWaveform(Waveform *w, bool select,
			bool callbacks=false) {
		DataSource_warn("selectWaveform"); }
	/** Select a table record.
	 *  @param[in] record the CssTableClass record.
	 *  @param[in] select the selected state.
	 *  @param[in] do_callback if true, do callbacks
	 */
	virtual bool selectRecord(CssTableClass *record, bool select,
			bool do_callback=false) {
		DataSource_warn("selectRecord"); return false; }
	/** Create a table record.
	 *  @param[in] table the CssTableClass record to create.
	 *  @param[in] table_name the name of the table.
	 *  @param[in] id_name a table member that must be incremented (eg.
	 *	"arid", "orid", etc.)
	 *  @param[in] password the user password structure.
	 */
	virtual CssTableClass *createTable(CssTableClass *table, const string &table_name,
		const string &id_name, Password password) {
		DataSource_warn("createTable"); return NULL; }
	/** Add an arrival.
	 *  @param[in] arrival the arrival object to add.
	 *  @param[in] ts the waveform.
	 *  @param[in] password the user password structure.
	 *  @returns true for success.
	 */
	virtual bool addArrival(CssArrivalClass *arrival, GTimeSeries *ts,
		Password password, long max_arid) {
		DataSource_warn("addArrival"); return false; }
	/** Add an assoc.
	 *  @param[in] assoc the assoc object to add.
	 *  @returns true for success.
	 */
	virtual bool addAssoc(CssAssocClass *assoc) {
		DataSource_warn("addAssoc"); return false; }
	/** Add an origerr.
	 *  @param[in] origin the origin object associated with the origerr.
	 *  @returns the origerr object or NULL.
	 */
	virtual CssOrigerrClass * addOrigerr(CssOriginClass *origin) { 
		DataSource_warn("addOrigerr"); return NULL; }
	/** Create an origin.
	 *  @param[in] origin the origin object.
	 *  @param[in] origerr the origerr object.
	 *  @param[in] table any object from the save data source used to get
	 *	the source information.
	 *  @param[in] password the user password structure.
	 *  @returns true for success.
	 */
	virtual bool originCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *table, Password password) { 
		DataSource_warn("originCreate"); return false; }
	/** Add a stamag.
	 *  @param[in] stamag the stamag object to add.
	 *  @param[in] table any object from the save data source used to get
	 *	the source information.
	 *  @returns true for success.
	 */
	virtual bool addStamag(CssStamagClass *stamag, CssTableClass *table) {
		DataSource_warn("addStamag"); return false; }
	/** Add an amplitude.
	 *  @param[in] arrival the associated arrival record.
	 *  @param[in] amplitude the amplitude object to add.
	 *  @returns true for success.
	 */
	virtual bool addAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude){
		DataSource_warn("addAmplitude"); return false; }
	/** Add a netmag.
	 *  @param[in] netmag the netmag object to add.
	 *  @param[in] table any object from the save data source used to get
	 *	the source information.
	 *  @returns true for success.
	 */
	virtual bool addNetmag(CssNetmagClass *netmag, CssTableClass *table) {
		DataSource_warn("addNetmag"); return false; }

	/** Delete one origin record. Origerr, assoc, wftag, stamag and netmag
	 *  object withs the same orid are also deleted.
	 *  @param[in] origin the origin that is to deleted.
	 *  @param[in] n_origerrs the number of origerr objects in origerrs[]
	 *  @param[in] origerrs origerr objects.
	 *  @param[in] n_assocs the number of assoc objects in assocs[]
	 *  @param[in] assocs assoc objects.
	 *  @param[in] n_wftags the number of wftag objects in wftags[]
	 *  @param[in] wftags wftag objects.
	 *  @param[in] n_stamags the number of stamag objects in stamags[]
	 *  @param[in] stamags stamag objects.
	 *  @param[in] n_netmags the number of netmag objects in netmags[]
	 *  @param[in] netmags netmag objects.
	 *  @returns true for success.
	 */
	virtual bool deleteOrigin(Component *caller, CssOriginClass *origin,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &wftags, cvector<CssNetmagClass> &netmags,
		cvector<CssStamagClass> &stamags) {
		    DataSource_warn("deleteOrigin"); return false; }
	/** Delete origins. Associated assocs and netmags are also deleted.
	 *  @returns true for success.
	 */
	virtual bool deleteOrigins(cvector<CssOriginClass> &origins) { 
		DataSource_warn("deleteOrigins"); return false; }
	/** Change an arrival record.
	 *  @param[in] arrival the arrival object to change.
	 *  @param[in] ts the associated GTimeSeries object.
	 *  @param[in] type the type of change:
	 *	- CHANGE_TIME
	 *	- CHANGE_AMP_PER
	 *	- CHANGE_PHASE_NAME
	 *	- CHANGE_STASSID
	 *	- CHANGE_AZIMUTH
	 *	- CHANGE_SLOW
	 *  @returns true for success.
	 */
	virtual bool changeArrival(CssArrivalClass *arrival, GTimeSeries *ts,
		int type)
	{ DataSource_warn("changeArrival"); return false; }

	/** Delete one arrival record. Assoc, amplitude, stamag, hydrofeatures,
	 *  and infrafeatures objects with the same arid are also deleted.
	 *  @param[in] arrival the arrival that is to be deleted.
	 *  @param[in] n_assocs the number of assoc objects in assocs[]
	 *  @param[in] assocs assoc objects.
	 *  @param[in] n_amps the number of amplitude objects in amps[]
	 *  @param[in] amps amplitude objects.
	 *  @param[in] n_stamags the number of stamag objects in stamags[]
	 *  @param[in] stamags stamag objects.
	 *  @param[in] n_hydros the number of hydrofeatures objects in hydros[]
	 *  @param[in] hydros hydrofeatures objects.
	 *  @param[in] n_infras the number of infrafeatures objects in infras[]
	 *  @param[in] infras infrafeatures objects.
	 *  @returns true for success.
	 */
	virtual bool deleteArrival(Component *caller, CssArrivalClass *arrival,
		cvector<CssAssocClass> &assocs, cvector<CssAmplitudeClass> &amps,
		cvector<CssStamagClass> &stamags, cvector<CssHydroFeaturesClass> &hydros,
		cvector<CssInfraFeaturesClass> &infras) { 
		    DataSource_warn("deleteArrival"); return false; }
	/** Delete arrivals. Associated assocs objects are also deleted.
	 *  @param[in] n_arrivals the number of arrivals in arrivals[]
	 *  @param[in] arrivals arrival objects.
	 *  @returns true for success.
	 */
	virtual bool deleteArrivals(cvector<CssArrivalClass> &arrivals) { 
		DataSource_warn("deleteArrivals"); return false; }
	/** Add a CssTableClass record.
	 *  @param[in] css the CssTableClass object to add.
	 *  @returns true for success.
	 */
	virtual bool addTable(CssTableClass *css) {
		DataSource_warn("addTable"); return false; }
	/** Delete one CssTableClass record.
	 *  @param[in] table 
	 *  @returns true for success.
	 */
	virtual bool deleteTable(Component *caller, CssTableClass *table,
		const string &table_name="") {
		DataSource_warn("deleteTable"); return false;}
	/** Change a CssTableClass record.
	 *  @param[in] old_table the original record.
	 *  @param[in] new_table the new record.
	 *  @returns true for success.
	 */
	virtual bool changeTable(CssTableClass *old_table, CssTableClass *new_table) { 
		DataSource_warn("changeTable"); return false; }
	/** Re-read a waveform.
	 *  @param[in] w the Waveform object for the waveform.
	 *  @param[in] wp the WaveformPlot that displays the waveform.
	 */
	virtual bool readWaveform(Waveform *w, WaveformPlot *wp) {
		DataSource_warn("readWaveform"); return false; }
	/** Modify waveforms.
	 *  @param[in] num_waveforms the number of waveforms to modify.
	 *  @param[in] wvec the Waveform objects for the waveforms.
	 */
	virtual void modifyWaveforms(gvector<Waveform *> &wvec) {}
	/** Modify a waveform.
	 *  @param[in] w the Waveform object for the waveform.
	 */
	virtual void modifyWaveform(Waveform *w) {}
	/** Get the beam elements.
	 *  @param[in] sta the station name.
	 *  @param[in] chan the channel name.
	 *  @param[out] wvec an array of Waveform objects for the
	 *		beam elements.
	 *  @returns the number of beam elements.
	 */
	virtual int getBeamElements(const string &sta, const string &chan,
			gvector<Waveform *> &wvec) { return -1; }

	/** Undo the deletion of a table. Insert the table object into its
	 *  source location (disk file or database account).
	 *  @param[in] table a table object that has been deleted.
	 */
	virtual bool undoDeleteTable(CssTableClass *table) {
		DataSource_warn("undoDeleteTable"); return false; }

	/** Check the source type.
	 *  @returns true if the source type is ODBC.
	 */
	virtual bool sourceIsODBC(void) { return false; }
	/** Get a WaveformWindow instance of this DataSource. This avoids class
	 *  casting.
	 *  returns a WaveformWindow class pointer if this DataSource object is
	 *  a subclass of WaveformWindow, otherwise returns NULL.
	 */
	virtual WaveformWindow *getWaveformWindowInstance(void) { return NULL; }
	/** Get a WaveformPlot instance of this DataSource. This avoids class
	 *  casting.
	 *  returns a WaveformPlot class pointer if this DataSource object is
	 *  a subclass of WaveformPlot, otherwise returns NULL.
	 */
	virtual WaveformPlot *getWaveformPlotInstance(void) { return NULL; }
	/** Get a TableViewer instance of this DataSource. This avoids class
	 *  casting.
	 *  returns a TableViewer class pointer if this DataSource object is
	 *  a subclass of TableViewer, otherwise returns NULL.
	 */
	virtual TableViewer *getTableViewerInstance(void) { return NULL; }
	/** Check real-time state.
	 *  @returns true if this DataSource is real-time.
	 */
	virtual bool isRealTime(void) { return false; }

	virtual void addDataReceiver(DataReceiver *receiver);
	virtual void removeDataReceiver(DataReceiver *owner);

	virtual vector<Response *> * channelResponse(GTimeSeries *ts,
			bool print_err=true) { return NULL; }

	static void copySourceInfo(CssTableClass *table, GTimeSeries *ts);
	static void copySourceInfo(GTimeSeries *ts, CssTableClass *table);
	static void copySourceInfo(CssTableClass *table_dest, CssTableClass *table_src);
        static void putDataSource(CssTableClass *table, DataSource *ds);
	static DataSource *getDataSource(CssTableClass *table);
	static bool compareChan(const string &c1, const string &c2);
	/* Compare channel names input as quarks.
	 * @param[in] c1_quark a channel name as a quark.
	 * @param[in[ c2_quark a channel name as a quark.
	 */
	static bool compareChan(int c1_quark, int c2_quark) {
		return compareChan(quarkToString(c1_quark),
				quarkToString(c2_quark));
	}
	/* Compare channel names (string and quark).
	 * @param[in] c1 a channel name.
	 * @param[in[ c2_quark a channel name as a quark.
	 */
	static bool compareChan(const string &c1, int c2_quark) {
		return compareChan(c1, quarkToString(c2_quark));
	}
	/* Compare channel names (quark and string).
	 * @param[in] c1_quark a channel name as a quark.
	 * @param[in[ c2 a channel name.
	 */
	static bool compareChan(int c1_quark, const string &c2) {
		return compareChan(quarkToString(c1_quark), c2);
	}

	void addOwner(Gobject *owner);
	void removeOwner(Gobject *owner);
	virtual void deleteSource() { if(!hasOwners()) delete this; }
	bool hasOwners() { return (int)owners.size() > 0 ? true : false; }

    protected:
	// can only be constructed by a subclass
	DataSource(void) : change(), hold_data_change_callbacks(false),
		allow_owners(false), receivers(), owners()
	{ }

#ifdef __STDC__
	virtual void ShowWarning(const char *format, ...);
	virtual void PutWarning(const char *format, ...);
#else
	virtual void ShowWarning(va_alist);
	virtual void PutWarning(va_alist);
#endif

	/** Reset the data change fields.
	  * @param[in] set the state of all data change fields.
	  */
	void resetDataChange(bool state) {
	    change.setAll(state);
	}
	bool dataChange(void);

	DataChange change;
	bool hold_data_change_callbacks;
	bool allow_owners;
	vector<DataReceiver *> receivers;
	vector<Gobject *> owners;
};

#endif
