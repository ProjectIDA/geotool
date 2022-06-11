#ifndef _TABLE_QUERY_H
#define _TABLE_QUERY_H

#include "TableViewer.h"
#include "motif++/MotifDecs.h"
#include "motif++/Menu.h"
#include "OpenDB.h"
#include "Beam.h"
#include "WaveformWindow.h"

#ifdef HAVE_LIBODBC
#include "FixBool.h"
#include "libgdb.h"
#endif
extern "C" {
#include "libstring.h"
#include "libtime.h"
}

enum TQDisplayType {
    DISPLAY_ARRIVALS,
    DISPLAY_WAVEFORMS,
    REVIEW_ORIGINS
};

/** @ingroup libgx
 */

class CSSTable;
class SelectOrder;
class Working;
class CPlotClass;
class Waveform;
class GTimeSeries;
class WaveformPlot;
class GSourceInfo;
class QueryViews;

/** TableQuery window.
 *  @ingroup libgx
 */
class TableQuery : public TableViewer
{
    public:
	TableQuery(const string &name, Component *parent);
	TableQuery(const string &name, Component *parent, bool auto_connect);
	~TableQuery(void);

	void setWaveformReceiver(DataReceiver *dr);
	bool numSelectedRows(const string &tableName);
	void getAllTables(void);
	void getAllTables(bool static_tables);
	CSSTable *addTableTab(const string &tableName);
	CSSTable *addTableTab(const string &tableName, bool display_text_area);
	bool connectODBC(const string &odbc_source, const string &user,
		const string &password) {
	    return open_db->connectODBC(odbc_source, user, password);
	}
	void connectPrefix(const string &prefix) {
	    open_db->connectPrefix(prefix);
	}
	void disconnect(void) { open_db->disconnect(); }
	bool autoConnect(void) { return open_db->autoConnect(); }
	void runQuery(const string &tableName, char *query,bool raise_tab=true);
	void runQuery(const string &tableName, char *query, double tmin,
			double tmax, bool raise_tab=true);
	bool runQuery(const string &tableName, char *query,
			gvector<CssTableClass *> &records);
	void selectBeamChannels(const string &sta, const string &chan,
			int num_exclude=0, char **exclude_sta=NULL);
	void wfdiscsFromArrival(void);
	void wfdiscsFromTime(char *sta, double time);
	void setTimeWindow(double tmin, double tmax, bool selected_only=true);
	void setArrivalTimeWindow(void);
	void setArrivalTimeWindow(double time);
	void setArrivalTimeWindow(double time_before, double time_after);
	bool selectChannel(char *sta, char *chan);
	bool selectChannel(char *sta, char *chan, long arid,
		double display_start, double display_end);
	string getTopTabName(void);
	bool selectWfdiscs(const string &ontop, TQDisplayType display_type);
	void setNoRecordsWarn(bool nowarn) { no_records_warn = nowarn; }
	virtual void addRow(void);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseCmd parseConnect(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	void getLatLon(gvector<CssTableClass *> &sites, gvector<CssTableClass *> &wfdiscs,
                        double *lat, double *lon);
	void getNetworkTables(void);
	void getNetworkTables(bool show_working);
	void displayArrivals(void);
	void getArrivalNetworks(cvector<CssArrivalClass> &arrivals);
	bool readWaveform(GSourceInfo *s, Waveform *w, WaveformPlot *wp);
	void loadOneWaveform(Waveform *w, CPlotClass *cp, double tbeg,
		double tend, cvector<CssWfdiscClass> &wfdiscs, CssOriginClass *o);
	void loadBeam(Waveform *w, WaveformPlot *wp, double tbeg,
		double tend, cvector<CssWfdiscClass> &wfdiscs, BeamRecipe &recipe,
		vector<BeamSta> &beam_sta, CssOriginClass *o);
	void writeBeamDotw(GSourceInfo &s, char *net, char *chan,
		GTimeSeries *ts);
	int writeDotw(char *access, char *dotw, float *y, char *sta,
		char *chan, double time, int nsamp, double samprate,
		double calib, double calper, CssWfdiscClass *w);
	void makeBeam(char *net, char *chan, double tbeg, double tend,
		long arid, cvector<CssWfdiscClass> &w, BeamRecipe &recipe,
		vector<BeamSta> &beam_sta, CssOriginClass *o,
		cvector<CssWfdiscClass> &wfdiscs);

	//  DataSource interface
	bool readWaveform(Waveform *w, WaveformPlot *wp);
        void addDataListener(Component *comp) {
	    addActionListener(comp, XtNdataChangeCallback);
        }
	int getTable(const string &tableName, gvector<CssTableClass *> &v);

        int getTable(cvector<CssAffiliationClass> &v) { return getTable(cssAffiliation, v); }
        int getTable(cvector<CssAmpdescriptClass> &v) { return getTable(cssAmpdescript, v); }
        int getTable(cvector<CssAmplitudeClass> &v) { return getTable(cssAmplitude, v); }
        int getTable(cvector<CssArrivalClass> &v) { return getTable(cssArrival, v); }
        int getTable(cvector<CssAssocClass> &v) { return getTable(cssAssoc, v); }
        int getTable(cvector<CssHydroFeaturesClass> &v) { return getTable(cssHydroFeatures, v); }
        int getTable(cvector<CssInfraFeaturesClass> &v) { return getTable(cssInfraFeatures, v); }
        int getTable(cvector<CssNetmagClass> &v) { return getTable(cssNetmag, v); }
        int getTable(cvector<CssOrigerrClass> &v) { return getTable(cssOrigerr, v); }
        int getTable(cvector<CssOriginClass> &v) { return getTable(cssOrigin, v); }
        int getTable(cvector<CssGregionClass> &v) { return getTable(cssGregion, v); }
        int getTable(cvector<CssParrivalClass> &v) { return getTable(cssParrival, v); }
        int getTable(cvector<CssStamagClass> &v) { return getTable(cssStamag, v); }
        int getTable(cvector<CssStassocClass> &v) { return getTable(cssStassoc, v); }
        int getTable(cvector<CssSiteClass> &v) { return getTable(cssSite, v); }
        int getTable(cvector<CssSitechanClass> &v) { return getTable(cssSitechan, v); }
        int getTable(cvector<CssWfdiscClass> &v) { return getTable(cssWfdisc, v); }
        int getTable(cvector<CssWftagClass> &v) { return getTable(cssWftag, v); }
        int getTable(cvector<CssXtagClass> &v) { return getTable(cssXtag, v); }

	bool getRecordsFromIds(const string &tableName, const string &idName,
		int num_table_ids, long *table_ids,
		gvector<CssTableClass *> &records);
	CssSiteClass *getSite(const string &sta, int jdate);
	int getSelectedWaveforms(const string &cursor_label,
		gvector<Waveform *> &wvec) { return -1; }
	int getSelectedWaveforms(gvector<Waveform *> &wvec) {
		return -1; }
	bool dataWindowIsDisplayed(const string &cursor_label) { return false; }
	int getPathInfo(PathInfo **path_info);
	void displayWaveforms(TQDisplayType display_type) {
	    displayWaveforms(display_type, getTopTabName(), true);
	}
	void displayWaveforms(TQDisplayType display_type, bool visible) {
	    displayWaveforms(display_type, getTopTabName(), visible);
	}
	void displayWaveforms(TQDisplayType display_type, const string &onTop,
		bool visible=true);
	bool sourceIsODBC(void) {
	    return (open_db->sourceType() == ODBC_CONNECTION) ? true : false; }
	long getWorkingOrid(void) { return working_orid; }
	void setWorkingOrid(long orid, bool do_callback=true) {
		working_orid = orid; }

        gvector<SegmentInfo *> *getSegmentList(void);

	bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg);
	CssTableClass *createTable(CssTableClass *table, const string &table_name,
		const string &id_name, Password password);
	bool changeTable(CssTableClass *old_table, CssTableClass *new_table);
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
	bool addTable(CssTableClass *table);
	CssOrigerrClass *addOrigerr(CssOriginClass *origin);
	bool addAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude);
	bool addNetmag(CssNetmagClass *netmag, CssTableClass *table);
	bool addStamag(CssStamagClass *stamag, CssTableClass *table);
	CssOriginClass *getPrimaryOrigin(Waveform *w);
	ParseCmd parseWaveformConstraint(const string &cmd);

	// ODBC routines
#ifdef HAVE_LIBODBC
	CssTableClass *odbcCreateTable(CssTableClass *table, const string &table_name,
		const string &id_name, Password password,  bool output_source);
	bool odbcChangeTable(CssTableClass *old_table, CssTableClass *new_table);
	bool odbcDeleteTable(CssTableClass *table, const string &table_name);
	bool odbcAddArrival(CssArrivalClass *arrival, GTimeSeries *ts,
		bool output_source);
	bool odbcChangeArrival(CssArrivalClass *arrival, GTimeSeries *ts, int mask);
	bool odbcDeleteArrival(CssArrivalClass *arrival, cvector<CssAssocClass> &assocs,
		cvector<CssAmplitudeClass> &amps, cvector<CssStamagClass> &stamags,
		cvector<CssHydroFeaturesClass> &hydros,
		cvector<CssInfraFeaturesClass> &infras);
	bool odbcOriginCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *any, Password password, bool output_source);
	bool odbcDeleteOrigin(CssOriginClass *origin,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &wftags, cvector<CssNetmagClass> &netmags,
		cvector<CssStamagClass> &stamags);
	bool odbcAddAssoc(CssAssocClass *assoc, bool output_source);
	bool odbcAddTable(CssTableClass *table, bool output_source);
	CssOrigerrClass *odbcAddOrigerr(CssOriginClass *origin, bool output_source);
	bool odbcAddStamag(CssStamagClass *stamag, CssTableClass *table,
		bool output_source);
	bool odbcAddAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude,
		bool output_source);
	bool odbcAddNetmag(CssNetmagClass *netmag, CssTableClass *table,
		bool output_source);
	SQLHDBC connectToODBC(CssTableClass *table, int *account);
	SQLHDBC connectToODBC(GTimeSeries *ts, int *account);
	SQLHDBC odbcConnect(CssTableClass *table, string &tableName);
#endif

	// FFDB routines
	CssTableClass *ffdbTableCreate(CssTableClass *table, const string &table_name,
		const string &id_name, Password password, bool output_source);
	bool ffdbAddArrival(CssArrivalClass *arrival, GTimeSeries *ts,
		Password password, bool output_source);
	bool ffdbOriginCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *any, Password password, bool output_source);
        OpenDB *open_db;

	FileDialog *fileDialog;
	Button *query_button;
	Button *connection_button;
	Button *new_tq_button;

	Button *display_arrivals;
	Button *display_waveforms;
	Button *map_origins;
	Button *review_origins;
	Button *get_all_tables;
	Button *get_ampdescripts;
	Button *get_amplitudes;
	Button *get_aao;
	Button *get_aawo;
	Button *get_arrivals;
	Button *get_assocs;
	Button *get_filters;
	Button *get_fsdiscs;
	Button *get_fsrecipes;
	Button *get_fstags;
	Button *get_hydrofeatures;
	Button *get_infrafeatures;
	Button *get_instruments;
	Button *get_netmags;
	Button *get_origaux;
	Button *get_origerrs;
	Button *get_origins;
	Button *get_sensors;
	Button *get_sitechans;
	Button *get_stamags;
	Button *get_stassocs;
	Button *get_wfdiscs;
	Button *get_wftags;
	Button *locate_event;

	Button *change_author;

	Button *display_author_button;
    protected:

	typedef struct
	{
	    string	table_name;
	    string	id_name;
	    Button	*button;
	} TableId;

	vector<TableId> table_ids;

	Separator *option_sep1, *option_sep2, *option_sep3, *option_sep4;

        Menu		*views_menu;

	bool		last_arid_input;
	bool		last_display_waveforms;
	bool		waveform_subclass;
	bool		no_records_warn;
	char		*current_query;
	string		parse_start_phase;
	string		parse_end_phase;

	List		*sta_list;
	ScrolledWindow	*sta_sw;
	List		*chan_list;
	ScrolledWindow	*chan_sw;
	List		*ctype_list;
	ScrolledWindow	*ctype_sw;
	List		*jdate_list;
	ScrolledWindow	*jdate_sw;
	List		*origin_list;
	ScrolledWindow	*origin_sw;
	List		*start_phase_list;
	List		*end_phase_list;
	TextField	*global_start_time;
	TextField	*global_end_time;
	TextField	*azimuth_min;
	TextField	*azimuth_max;
	TextField	*distance_min;
	TextField	*distance_max;
	TextField	*time_before_phase;
	TextField	*time_after_phase;

	int		numSchema;
#ifdef HAVE_LIBODBC
	ODBC_Table	*schema;
#endif

	typedef struct
	{
	    RowColumn *rcbot;
	    ArrowButton *arrow;
	    Separator *sep2, *sep3, *sep4;
	    RowColumn *form2, *rcmid, *rctop;
	    Label *label;
	} WTStruct;

	WTStruct	wt;

	gvector<CssTableClass *> *assocs_sort;

	typedef struct
	{
	    char    sta[10];
	    char    chan[10];
	    bool    filter;
	    int     order;
	    char    type[3];
	    double  flow;
	    double  fhigh;
	    double  tdel;
	    int     zero_phase;
	} ReviewChannel;

	int num_review_channels;
	ReviewChannel *rc;

	QueryViews *query_views;
	Beam *gbeam;

	long working_orid;

	void createInterface(bool auto_connect);
	void setFileMenu(void);
	void setEditMenu(void);
	void setViewMenu(void);
	void setOptionMenu(void);
	void setHelpMenu(void);

        void init(void);
        void createSelectTables(void);
	void actionPerformed(ActionEvent *action_event);
	void open(void);
        Form *makeWfdiscTab(Form *topform);
	void readReviewChannels(void);
	bool getReviewFilter(ReviewChannel *rc, char *c, char **last);
        void tabSelect(const string &tab_name);
	void setDefaultViews(void);
	void queryView(char *view_query);
	void timeRunQuery(OpenDBStruct *op);
	void arrivalRunQuery(OpenDBStruct *op);
	void originRunQuery(OpenDBStruct *op);
	bool runOdbcQuery(gvector<CssTableClass *> &records, char *query,
		const string &tableName);
	void setIds(int i0, gvector<CssTableClass *> &records);
	bool runFlatQuery(gvector<CssTableClass *> &records, char *query,
		const string &tableName);
	void unspecifiedRunQuery(OpenDBStruct *op);
	void runUnspecifiedQuery(char *query);
	void tabQuery(void);
	void sortRows(gvector<CssTableClass *> &v, const string &tableName);
	char *tabName(void);
	void getWftagsFromOrigins(void);
	void wfdiscConstraintsInit(void);
	void getLinkedTable(const string &table1, const string &id_member,
			const string &table2);
	void getAllRows(const string &tableName);
	int getAAWO(bool get_wfdiscs, bool get_net_tables=false);
	void getAnotherTable(const string &tabName, const string &tableName);
	bool applyDisplayConstraints(void);
	void applyTimeConstraint(CSSTable *table, gvector<CssTableClass *> &v);
	void applyOriginConstraints(CSSTable *table, gvector<CssTableClass *> &v);
	CssOriginClass *getWfdiscOrigin(CssWfdiscClass *wfdisc,
		gvector<CssTableClass *> &origins, gvector<CssTableClass *> *wftags,
		gvector<CssTableClass *> &arrivals, gvector<CssTableClass *> *assocs);

	void storeOrids(gvector<CssTableClass *> &v);
	void applyFilters(WaveformPlot *wp, int n_old);

	static void loadArr(GTimeSeries *ts, SegmentInfo *seg,
		cvector<CssArrivalClass> &arr, cvector<CssArrivalClass> &arrivals);
	static void loadAssocs(cvector<CssArrivalClass> &arrivals,
		cvector<CssArrivalClass> &records, cvector<CssAssocClass> &assocs);
	static void loadAllAmplitudes(gvector<CssTableClass *> &records,
		gvector<CssTableClass *> &amps);
	static void loadAllStamags(gvector<CssTableClass *> &records,
		gvector<CssTableClass *> &stamags);
	static void loadAllParrivals(gvector<CssTableClass *> &records,
		gvector<CssTableClass *> &parrivals);
	static void loadAllNetmags(gvector<CssTableClass *> &records,
		gvector<CssTableClass *> &netmags);
	static void loadOrigerrs(gvector<CssTableClass *> &origins,
		gvector<CssTableClass *> &records, gvector<CssTableClass *> &origerrs);
	static void loadOrigin(GTimeSeries *ts, gvector<CssTableClass *> &origins,
		CssOriginClass *o);
	bool reviewChannel(char *sta, char *chan);
	bool reviewBeams(void);
	void reviewElements(gvector<CssTableClass *> &wfdiscs, gvector<CssTableClass *> &all);
	bool arrivalElements(void);
	void saveArrivalElements(gvector<CssTableClass *> &wfdiscs);
	long getOrid(CssTableClass *css);
	CssOriginClass *getOrigin(CssTableClass *css, gvector<CssTableClass *> &origins);
	void selectTableRow(const string &tableName);
	void setChange(CSSTable *table);
	void setChange(gvector<CssTableClass *> &v);
	void setChange(const string &name);
	void doRecentInput(void);


    private:
	char error_msg[200];
	bool ignore_getpath;
	void saveButton(const string &name, Button *b, const string &id_name);
	void resetAll(void);
	void selectConstraint(void);
	void arrowSelect(void);

	static void typeToPriorities(const string &type,int **priorities,
		int *np);
	static void defaultPriority(const string &chan, int *priority);
	static int checkCtype(const string &type, int priority);
	static void initStaPresent(int present);
	static void setStaPresent(int sindex, int cindex);
	static void cleanStaPresent(void);
	static int checkStaPresent(int sindex, int cindex);
	static int readPriorityFile(const string &priority_file, bool warn);
	static void getIndex(const string &_sta, const string &_chan, int *idx,
			int *eno, int *priority);
	static int findSta(int sta, gvector<CssTableClass *> &s);

};

#endif
