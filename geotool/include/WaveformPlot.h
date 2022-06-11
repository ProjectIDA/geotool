#ifndef WAVEFORM_PLOT_H
#define WAVEFORM_PLOT_H

#include "widget/TtPlotClass.h"
#include "gobject++/DataSource.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "WaveformSetup.h"
#include "AddStation.h"
#include "WaveformConstraint.h"
#include "motif++/MotifDecs.h"
#include "Beam.h"
#include "IIRFilter.h"
#include "ArrivalParams.h"
#include "AmplitudeParams.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "crust.h"
}

#ifndef UNDEFINED_PIXEL
#define UNDEFINED_PIXEL 0x80000000
#endif


enum AlignType {
    ALIGN_FIRST_POINT,
    ALIGN_TRUE_TIME,
    ALIGN_ORIGIN,
    ALIGN_PREDICTED_P,
    ALIGN_OBSERVED_P,
    ALIGN_PREDICTED_ARRIVAL,
    ALIGN_OBSERVED_ARRIVAL
};

enum TimeType {
    TIME_SECONDS,
    TIME_VARIABLE,
    TIME_HMS,
    TIME_EPOCH
};

enum DataHeightType {
    SPACE_MORE,
    SPACE_LESS,
    SCALE_UP,
    SCALE_DOWN
};

enum ComponentsType {
    ALL_COMPONENTS,
    Z_COMPONENTS,
    N_AND_E_COMPONENTS,
    N_COMPONENTS,
    E_COMPONENTS
};

enum AddCursorType {
    ADD_TIME_LINE,
    ADD_TIME_LIMITS,
    ADD_CROSSHAIR,
    REMOVE_ALL_CURSORS
};

enum DataMovementType {
    DATA_NO_MOVEMENT,
    DATA_XY_MOVEMENT,
    DATA_X_MOVEMENT,
    DATA_Y_MOVEMENT
};

enum DisplayArrivalsType {
    ARRIVALS_OFF,
    ON_ONE_CHANNEL,
    ON_ALL_CHANNELS
};

enum SelectWaveformType {
    SELECT_ALL,
    DESELECT_ALL,
    SELECT_VISIBLE,
    SELECT_VISIBLE_Z,
    SELECT_VISIBLE_HORZ
};

enum SortWaveformType {
    SORT_DEFAULT_ORDER,
    SORT_FILE_ORDER,
    SORT_DISTANCE,
    SORT_DISTANCE_FROM,
    SORT_TIME_STA_CHAN,
    SORT_STA_CHAN,
    SORT_CHAN_STA,
    SORT_CHAN2_STA,
    SORT_BACK_AZIMUTH
};

enum ArrivalColorMode {
    DEFAULT_COLOR,
    COLOR_BY_ASSOC
};

enum AmpMeasureMode {
    AUTO_MEASURE,
    ML_MEASURE,
    MB_MEASURE
};

typedef struct {
    char	sta[7];
    char	chan[9];
    char	net[9];
    double	lat;
    double	lon;
    CssOriginClass	*origin;
    GTimeSeries	*ts;
} DataQCStruct;

typedef struct {
    double stav_len;
    double ltav_len;
    double min_snr;
    double max_snr;
    double min_deltim;
    double max_deltim;
    double deltim_diff;
} TimeParams;

class Button;
class FKArgs;

/**
 *  @ingroup libgx
 */
class RecentInput : public Gobject
{
    public :
	RecentInput(void) {
	    read_all = false;
	    origin = NULL;
	    button = NULL;
	}
	~RecentInput(void) { }

	bool equals(RecentInput *r);

	bool		read_all;
	string		format;
	vector<string>	chan_list;
	string		file;
	vector<int>	wavs;
	CssOriginClass	*origin;
	Button		*button;
};

typedef struct
{
    Toggle *partial_select_toggle;
    Toggle *display_amp_scale;

    Toggle *align_first_point;
    Toggle *align_true_time;
    Toggle *align_origin;
    Toggle *align_predicted;
    Toggle *align_observed;
    Toggle *align_predicted_phase;

    Toggle *all_components;
    Toggle *z_only;
    Toggle *n_and_e_only;
    Toggle *n_only;
    Toggle *e_only;

    Toggle *no_movement;
    Toggle *xy_movement;
    Toggle *x_movement;
    Toggle *y_movement;

    Toggle *arrivals_off;
    Toggle *on_one_channel;
    Toggle *on_all_channels;

    Toggle *default_order;
    Toggle *file_order;
    Toggle *distance;
    Toggle *distance_from;
    Toggle *time_sta_chan;
    Toggle *sta_chan;
    Toggle *chan_sta;
    Toggle *back_azimuth;

    Toggle *display_tags;

    Toggle *time_scale_seconds;
    Toggle *time_scale_variable;
    Toggle *time_scale_hms;
    Toggle *time_scale_epoch_toggle;

    Toggle *uniform_scale;
    Toggle *auto_scale;
} WaveformPlotToggles;


#define XtNdataQCCallback (char *)"dataQCCallback"
#define XtNalignCallback (char *)"alignCallback"

class AmplitudeScale;
class AxesLabels;
class CreateBeam;
class DataValues;
class Diff;
class Filter;
class History;
class LoadedTableList;
class Output;
class PrintDialog;
class Scroll;
class SegmentInfo;
class TableSource;
class TagContents;
class UndoDeleteArrival;
class UndoDeleteOrigin;
class UndoDeleteTables;
class Waveforms;
class WaveformColor;
class WaveformWindow;


/**
 *  @ingroup libgx
 */
class WaveformPlot : public TtPlotClass, public DataReceiver, public PrintClient
{
    friend class WaveformWindow;
    friend class UndoDeleteArrival;
    friend class UndoDeleteOrigin;

    public:

	WaveformPlot(const string &name, Component *parent, Arg *args=NULL,
			int n=0);
	WaveformPlot(const string &, Component *, InfoArea *, Arg *, int);
	WaveformPlot(const string &, Component *, Arg *, int, DataSource *ds);
	WaveformPlot(const string &, Component *, InfoArea *, Arg *, int,
			DataSource *ds);
	~WaveformPlot(void);

	virtual void destroy(void);
	void actionPerformed(ActionEvent *action_event);

	// PrintClient interface
	virtual void print(FILE *fp, PrintParam *p);

	// DataReceiver interface
	void setDataSource(DataSource *ds);

	// DataSource interface
	void modifyWaveforms(gvector<Waveform *> &wvec);
	void modifyWaveform(Waveform *w) { 
	    gvector<Waveform *> v(w);
	    modifyWaveforms(v);
	}
	bool reread(GTimeSeries *ts) {
	    return ts->reread();
	}
	bool deleteArrivals(cvector<CssArrivalClass> &arrivals);
	bool deleteOrigins(cvector<CssOriginClass> &origins);
	virtual WaveformPlot *getWaveformPlotInstance(void) { return this; }

	void setTagMembers(int num_selected, int *selected,
			const string &user_string, bool selected_only);
	void alignWaveforms(AlignType align_type);
	void alignWaveforms(AlignType type, const string &phase);
	void alignWaveforms(gvector<Waveform *> &wvec, vector<double> &t_lags);
	AlignType alignment(void) { return time_align; }
	void setTimeScale(TimeType time_type);
        void adjustDataHeight(DataHeightType type);
	void setAutoScale(bool set);
	void turnOffAutoScale(void) { auto_vertical_scale = false; }
        void showComponents(ComponentsType type);
        void addCursor(AddCursorType type);
        void dataMovement(DataMovementType type);
	void displayArrivals(DisplayArrivalsType type);
	void displayAllData(bool all);
        void promoteWaveforms();
        void selectWaveforms(SelectWaveformType type);
        void setDefaultOrder(void);
	void sortWaveforms(SortWaveformType type);
	void sortWaveforms(SortWaveformType type, gvector<Waveform *> &);
	void alignTrueTime(void);
	void alignFirstPoint(void);
	void alignOrigin(bool warn);
	void clear(bool do_callback=true);
	void deleteOne(Waveform *w);
	void deleteSelectedWaveforms(void);
	void deleteWaveforms(gvector<Waveform *> &wvec);
	void setSyncMenus(Menu *out_edit_menu, Menu *out_view_menu) {
	    outside_edit_menu = out_edit_menu;
	    outside_view_menu = out_view_menu;
 	}

	static u_long getSwapSpace(void);

	void setWaveformLimit(int num) { cons.waveform_limit = num; }
	void setWaveformMaxLength(double secs) { cons.max_length = secs; }

	void inputData(gvector<SegmentInfo *> *seginfo, bool display_data) {
	    addData(seginfo, 0, (RecentInput *)NULL, display_data, true);
	}
	void inputData(gvector<SegmentInfo *> *seginfo, DataSource *ds,
			bool display_data);
	void inputData(gvector<SegmentInfo *> *seginfo, DataSource *ds,
			bool display_data, bool set_limits);
	void addWaveforms(int num_waveforms, GTimeSeries **ts,
			Waveform **ws=NULL, Pixel *colors=NULL,
			bool display_data=true, bool set_limits=true);
	Waveform *addWaveform(GTimeSeries *ts,Pixel color=UNDEFINED_PIXEL,
			bool display_data=true, bool set_limits=true)
	{
	    Waveform *ws[1] = {NULL};
	    addWaveforms(1, &ts, ws, &color, display_data, set_limits);
	    return ws[0];
	}

	void setTTCurves(bool on);

	void setApplyCalib(bool apply) { apply_calib = apply; }
	bool getApplyCalib(void) { return apply_calib; }

	string getColorCode(void) { return color_code; }
	void setColorCode(const string &color);
	Pixel nextColor(gvector<Waveform *> &wvec);
	Pixel nextColor(int num_waveforms, Pixel *fg);

	void beamWaveforms(double az, double slowness, int order,
		const string &ftype, double flo, double fhi, int zp,
		BeamLocation beam_location, bool replace, WaveformPlot *dest);
	void beamWaveforms(gvector<Waveform *> &wvec,
		double az, double slowness, int order, const string &ftype,
		double flo, double fhi, int zp, BeamLocation beam_location,
		bool replace, WaveformPlot *dest);
	void computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		bool replace, bool scroll, WaveformPlot *dest);
	void computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		IIRFilter *iir, vector<double> &weights,
		bool replace, bool scroll, WaveformPlot *dest);
	void computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		double tbeg, double tend, bool replace, bool scroll,
		WaveformPlot *dest);
	void computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		double tbeg, double tend, IIRFilter *iir,
		vector<double> &weights, bool replace, bool scroll,
		WaveformPlot *dest);
	void computeFtrace(double az, double slowness, double window_len,
		double snr, int order, double flo, double fhi, bool zp,
		BeamLocation beam_location, bool replace, WaveformPlot *dest);
	void computeFtrace(gvector<Waveform *> &wvec, double az,
		double slowness, double window_len, double snr, int order,
		double flo, double fhi, bool zp, BeamLocation beam_location,
		bool replace, WaveformPlot *dest);
	void displayBeam(GTimeSeries *ts, gvector<Waveform *> &wvec,
		const string &beam_name, IIRFilter *iir, bool replace,
		bool scroll, WaveformPlot *dest);
	void displayCalib(bool calib_on);
	void removeMean(bool demean, Waveform *w=NULL);
	bool changeCalib(Waveform *w, bool apply);
	void polarity(bool normal, Waveform *w=NULL);
	void unfilter(bool all, Waveform *w=NULL);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);
	ParseCmd parseOutput(Waveform *w, const string &cmd, string &msg);
	ParseCmd parseTimeWindow(const string &cmd, string &msg);
	ParseCmd parseSelectArrivals(const string &cmd, string &msg);
	ParseCmd parseSelect(const string &cmd, string &msg);
	void parseAddArrival(Waveform *w, const string &phase, double time);
	ParseCmd parseRotate(Waveform *w, const string &cmd, string &msg);
	ParseCmd parseUnrotate(Waveform *w, const string &cmd, string &msg);
	ParseCmd parseWaveformColor(const string &cmd, string &msg);
	ParseCmd parseBeam(const string &cmd, string &msg);
	void putTable(CssTableClass *table, bool do_callback=true) {
	    if(!strcmp(table->getName(), cssArrival)) {
		Pixel fg = getArrivalFg((CssArrivalClass *)table);
		putArrivalWithColor((CssArrivalClass *)table, fg);
	    }
	    else {
		TtPlotClass::putTable(table, do_callback);
	    }
	}
	bool ttCurvesOn(void) { return tt_curves_on; }
	void setToggleStates(WaveformPlotToggles *t);

	vector<ArrivalKey> *getArrivalKeys(void) {
	    return new vector<ArrivalKey>(arrival_keys);
	}
	void setArrivalKeys(vector<ArrivalKey> &keys) {
	    arrival_keys.clear();
	    for(int i = 0; i < (int)keys.size(); i++) {
		arrival_keys.push_back(keys[i]);
	    }
	    setCPlotArrivalKeys(arrival_keys);
	}
	void output(void);

	static int getWaveformColors(Component *, Pixel **fg);
	static void ampConvert(CPlotMeasure *m);
	static void resetWaveformColors(void);

	CssArrivalClass *makeArrival(Waveform *w, Password password,
		double time, const string &phase, double deltim, double azimuth,
		double delaz, double slow, double delslo, double ema,
		double rect, double snr, bool single_arrival);
	bool measureAmplitudes(gvector<Waveform *> &wvec,
		CssArrivalClass *arrival, CssAmplitudeClass **amp_mb,
		CssAmplitudeClass **amp_ml, AmpMeasureMode mode=AUTO_MEASURE);
	bool measureMBAmp(GTimeSeries *ts, double time, CssAmplitudeClass *amp);
	bool saveAmp(DataSource *ds, CssArrivalClass *arrival, CssAmplitudeClass *amp);
	bool measureAmps(CssArrivalClass *arrival, Waveform *w,
		AmpMeasureMode mode=AUTO_MEASURE);
	void measureArrayAmp(gvector<Waveform *> &wvec, CssArrivalClass *arrival,
		CssOriginClass *origin, const string &phase,
		bool measure_mb, CssAmplitudeClass *amp);
	GTimeSeries *getAmpBeam(gvector<Waveform *> &wvec,
		CssArrivalClass *arrival, CssOriginClass *origin, const string &phase,
		bool measure_mb);
	void mlppn(float *databuf, int dim, int atPoint, double sps,
		double *amp, double *snr);
	bool mbppc(float *databuf, int dim, int atPoint, double sps,
		double *amplitude, double *period, int *amp_pos);
	void deleteTables(gvector<CssTableClass *> &t);
	bool undoDeleteTables(UndoDeleteTables *undo);
	void outputTables(const string &prefix, const string &access);
	void output(const string &prefix, const string &access,
		gvector<CssTableClass *> &t);
	void updateTag(Waveform *w);
	bool beamArrival(CssArrivalClass *arrival,bool selected_waveforms_only=false,
			bool replace=true);
	void addArrivalMenu(CPlotArrivalCallbackStruct *c);
	void setArrivalMenuPhases(const string &phases) {
	    if(arrival_menu_phases.empty()) {
		arrival_menu_phases.assign(phases);
	    }
	}
	bool arrivalFK(CssArrivalClass *arrival, Waveform *w, bool fk_mb);
	bool arrivalPolar(CssArrivalClass *arrival, Waveform *w);

	PrintDialog *print_window;

	static ArrivalParams *arrivalParams(void);
	static AmplitudeParams *amplitudeParams(void);
	static void showArrivalParams(void);
	static void showAmplitudeParams(void);

    protected:

	TableSource *table_source;

	typedef struct
	{
	    int		nfiles;
	    char	**files;
	    char	format[50];
	} RecentListing;
	
	typedef struct
	{
	    char	*file;
	} RecentProject;

	WaveformConstraint cons;

        PopupMenu	*menu_popup;
        PopupMenu	*arrival_add_popup;
	AmplitudeScale	*amplitude_scale;
	Filter		*bfilter_window;
	Output		*output_window;
        DataValues	*data_values_window;
	Diff            *diff_window;
	TagContents	*tag_contents;
	AxesLabels	*labels_window;
	WaveformSetup	*waveform_setup;
        CreateBeam	*create_beam_window;
        History		*history_window;
	Waveforms	*waveform_window;
	LoadedTableList	*loaded_tables_window;
	Scroll		*scroll_window;
	WaveformColor	*waveform_color_window;

	string	color_code;
	string	arrival_menu_phases;
	Label	*arrival_menu_sta;

	SortWaveformType sort_type;
	AlignType	time_align;
	string		phase_align;

	int	predicted_phase;

	bool	tt_curves_on;
	bool	display_working;
	int	working_dialog_threshold;

	double sort_from_lat;
	double sort_from_lon;

	vector<ArrivalInfo> arrival_info;

	bool time_scale_epoch;
	bool auto_vertical_scale;
	bool apply_calib;
	bool menu_made;

	Beam *gbeam;

	double	waveform_duration;
	double max_nyquist;

	int	num_tag_members;
	int	tag_members[40];
	string	user_tag_string;
	
	int	beam_index;
	string	beam_chan_name;

	int	next_color;
	ArrivalColorMode arrival_color_mode;

	int	num_wave_plots;
	WaveformPlot *wave_plots[100];

	Menu *outside_edit_menu;
	Menu *outside_view_menu;

	WaveformPlotToggles wt;

	vector<ArrivalKey> arrival_keys;
	CPlotArrivalCallbackStruct arrival_cb;

	// Menu
	Button *clear_button;
	Button *delete_data_button;

        Menu    *align_menu;
        Menu    *components_menu;
        Menu    *cursors_menu;
        Button  *add_time_line, *add_time_limits, *add_crosshair,
                        *remove_all_cursors;
        Menu    *data_movement_menu;
        Button  *data_values_button;
        Menu    *display_arrivals_menu;
        Menu    *display_data_menu;
        Button  *all_data, *selected_data;
        Button  *labels_button;
        Button  *magnify_button;
        Button  *output_button;
        Button  *waveforms_button;
        Button  *promote_selected;
        Menu    *scale_menu;
        Button  *space_more, *space_less, *scale_up, *scale_down, *set_scale;
        Button  *scroll_button;
        Menu    *select_menu;
        Button  *select_all, *deselect_all, *select_visible, *select_visible_z,
                        *select_visible_horz;
        Button  *set_default_order;
        Menu    *tags_menu;

        Menu    *sort_menu;

        Menu    *time_scale_menu;
        Button  *unzoom_all;
	Button  *waveform_color_button;

	virtual void addData(gvector<SegmentInfo *> *seginfo, int pts,
			RecentInput *r, bool display_data, bool set_limits);

	void init(const string &name, Component *parent, Arg *args, int n);
	void saveArrivalInfo(int num, SegmentInfo **s);
	void setMaxNyquist(GTimeSeries *ts);
	bool applyCalib(GTimeSeries *ts);
	void getPositionLimits(vector<WaveInput *> *wav_inputs, double *t_min,
		double *t_max, int *max_tag_width, RecentInput *r);
	void getPositionLimits(vector<WaveInput *> *wav_inputs,
		int num_waveforms, GTimeSeries **ts, double *t_min,
		double *t_max, int *max_tag_width);

	virtual void setInputColor(GTimeSeries *ts, CPlotInputStruct *input);
	virtual void setTag(GTimeSeries *ts, CPlotInputStruct *input);
	virtual void setTag(const string &sta, double lat, double lon,
				CPlotInputStruct *input);
	void addTables(cvector<CssOriginClass> &cur_origins,
	    cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
	    cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
	    cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
	    cvector<CssHydroFeaturesClass> &hydro_features,
	    cvector<CssInfraFeaturesClass> &infra_features,
	    cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
	    cvector<CssAmplitudeClass> &amplitudes,
	    cvector<CssAmpdescriptClass> &ampdescripts,
	    cvector<CssParrivalClass> &parrivals);
	Pixel getArrivalFg(CssArrivalClass *a);
	void shiftData(int time_scale);
	void addStations(int num, SegmentInfo **s);
	void uniqueColor(int i, int num_waveforms, Pixel *fg);
	void readRecentInput(RecentInput *r);
	void menuPopup(XEvent *event);
	void makePopup(void);
	virtual void makePopupMenu(void);
	void printWindow(void);
	int numWaveformColors(void);
	Pixel waveformColor(int i);

	void setUniformAmpScale(bool set);
	void setAmplitudeScale(bool visible);
	void amplitudeScale(int calldata);
	void changeWaveformTag(void);
	virtual const char *beamChanName(void);
	void initArrivalKeys(void);
	virtual CssArrivalClass *addArrivalFromKey(CPlotArrivalCallbackStruct *p);
	void getArrayStats(gvector<Waveform *> &wvec, CssArrivalClass *arrival,
		double *fmin, double *fmax);
	int getArrayElements(Waveform *w, double time,
		gvector<Waveform *> &wlist, bool show_warning);
	bool getThreeComponents(Waveform *w, gvector<Waveform *> &wvec);
	void getThreeCompStats(Waveform *w, gvector<Waveform *> &wvec,
		double time, CssArrivalClass *arrival, double *tmax);
	void getSnr(GTimeSeries *ts, double time, double *snr,
		double *deltim);
	GTimeSeries *makeBeam(gvector<Waveform *> &wvec,
		double azimuth, double slowness, double tbeg, double tend,
		int order, double flo, double fhi, bool zp, bool coherent);

	TimeParams getTimeParams(void);
	void getFkParams(double *fk_lead, double *fk_lag, double *fk_dk,
		FKArgs *args=NULL, double *bandw=NULL);
	bool associateWithOrigin(DataSource *ds, CssArrivalClass *arrival,
			CssOriginClass *origin , double lat, double lon);
	bool undoDeleteArrival(UndoDeleteArrival *undo);
	bool undoDeleteOrigin(UndoDeleteOrigin *undo);
	PopupMenu *createArrivalAddPopup(void);
	void setAddArrivalMenu(void);

    private:
	void syncToggles(bool inside_to_outside);
	void syncToggleChildren(vector<Component *> *in,
		vector<Component *> *out);
	static void ampConvertCallback(Widget, XtPointer, XtPointer);

};

#endif
