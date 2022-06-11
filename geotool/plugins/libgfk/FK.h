#ifndef _FK_H
#define _FK_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "widget/Table.h"
#include "Waveform.h"
#include "widget/ConPlotClass.h"
#include "DataReceiver.h"
#include "Beam.h"
#include "widget/Parser.h"

class AxesClass;
class AxesLabels;
class BasicMap;
class CPlotClass;
class Waveform;
class DataSource;
class FKData;
class FKFine;
class GTimeSeries;
class RefSta;
class TaperWindow;

namespace libgfk {

class FKParam;
class FkParamDialog;
class FkSignal;
class BeamWaveformView;

enum FKType {
    FK_SINGLE_BAND,
    FK_MULTI_BAND
};

enum RTDmode {
    RTD_OFF,
    RTD_START,
    RTD_COMPUTING,
    RTD_WAITING
};

/** @defgroup libgfk plugin FK
 *  FK and FK Multi-band window.
 *  @ingroup libgfk
 */
class FK : public Frame, public PrintClient, public Parser, public DataReceiver
{
    friend class FkSignal;
    friend class BeamWaveformView;

    public:
	FK(const char *, Component *, DataSource *);
	FK(const char *, Component *, FKType, DataSource *);
	~FK(void);

	virtual void setVisible(bool visible);
	virtual void print(FILE *fp, PrintParam *p);
	void setDataSource(DataSource *ds);

	bool compute(void) { return compute(true, true); }
	bool autoCompute(void) { return computeAll(true, true); }
	FKData *currentFK(void) { return current_fk; }
	FKParam *getFKParam(void) { return p; }
	Table *bandTable(void) { return table; }
	void positionFK(double time);
	FKType type() { return (nbands == 1) ? FK_SINGLE_BAND : FK_MULTI_BAND; }
	void setWindowLength(double window_length);
	void setWindowOverlap(double window_overlap);
	void makeFK(int plot_index, int if1, int if2);
	double getFMin(int band);
	double getFMax(int band);
	Matrx *getData(int band);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

	// Parser and DataSource interface
	int getWaveforms(gvector<Waveform *> &ws,
		bool displayed_only=true)
	{
	    ws.clear();
	    ws.load(wvec);
	    return ws.size();
	}
	int getSelectedWaveforms(gvector<Waveform *> &ws) {
	    return getWaveforms(ws);
	}
	int getSelectedWaveforms(const string &cursor_label,
			gvector<Waveform *> &ws)
	{ ws.clear(); return 0; }

	int getTable(const string &name, gvector<CssTableClass *> &v) {
	    v.clear();
	    if(!name.compare(cssArrival)) {
		v.load((gvector<CssTableClass *> &)arrivals);
		return v.size();
	    }
	    else if(!name.compare(cssAssoc)) {
		v.load((gvector<CssTableClass *> &)assocs);
		return v.size();
	    }
	    return 0;
	}
	int getTable(cvector<CssAmpdescriptClass> &v) { return 0; }
	int getTable(cvector<CssAmplitudeClass> &v) { return 0; }
	int getTable(cvector<CssArrivalClass> &v) {
		v.clear();
		v.load(arrivals);
		return v.size();
	}
	int getTable(cvector<CssAssocClass> &v) {
		v.clear();
		v.load(assocs);
		return v.size();
	}
	int getTable(cvector<CssOriginClass> &v) { return 0; }
	int getTable(cvector<CssOrigerrClass> &v) { return 0; }
	int getTable(cvector<CssParrivalClass> &v) { return 0; }
	int getTable(cvector<CssNetmagClass> &v) { return 0; }
	int getTable(cvector<CssStamagClass> &v) { return 0; }
	int getTable(cvector<CssWftagClass> &v) { return 0; }
	int getTable(cvector<CssAffiliationClass> &v) { return 0; }
	int getTable(cvector<CssHydroFeaturesClass> &v) { return 0; }
	int getTable(cvector<CssInfraFeaturesClass> &v) { return 0; }
	int getTable(cvector<CssStassocClass> &v) { return 0; }
	int getTable(cvector<CssSiteClass> &v) { return 0; }
	int getTable(cvector<CssSitechanClass> &v) { return 0; }
	int getTable(cvector<CssWfdiscClass> &v) { return 0; }
	int getTable(cvector<CssXtagClass> &v) { return 0; }

	bool addArrival(CssArrivalClass *a, GTimeSeries *ts, Password pw) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->addArrival(a, ts, pw, -1);
	    }
	    return false;
	}
	bool addAssoc(CssAssocClass *a) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->addAssoc(a);
	    }
	    return false;
	}
	bool addStamag(CssStamagClass *s, CssTableClass *t) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->addStamag(s, t);
	    }
	    return false;
	}
	bool addAmplitude(CssArrivalClass *a, CssAmplitudeClass *amp) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->addAmplitude(a,amp);
	    }
	    return false;
	}
	bool addNetmag(CssNetmagClass *n, CssTableClass *t) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->addNetmag(n,t);
	    }
	    return false;
	}
	bool changeArrival(CssArrivalClass *a, GTimeSeries *ts, int change_type) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
	       return wvec[0]->ts->getDataSource()->changeArrival(a,ts,change_type);
	    }
	    return false;
	}
	bool deleteArrival(Component *caller, CssArrivalClass *arrival, 
	    cvector<CssAssocClass> &asscs, cvector<CssAmplitudeClass> &amps,
	    cvector<CssStamagClass> &stamags, cvector<CssHydroFeaturesClass> &hydros,
	    cvector<CssInfraFeaturesClass> &infras)
	{
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
	       return wvec[0]->ts->getDataSource()->deleteArrival(caller,
			arrival, asscs, amps, stamags, hydros, infras);
	    }
	    return false;
	}
	bool addTable(CssTableClass *css) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->addTable(css);
	    }
	    return false;
	}
	bool deleteTable(Component *caller, CssTableClass *t,
			const string &table_name)
	{
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->deleteTable(caller, t,
				table_name);
	    }
	    return false;
	}
	bool changeTable(CssTableClass *o, CssTableClass *n) {
	    if(wvec.size() > 0 && wvec[0]->ts->getDataSource()) {
		return wvec[0]->ts->getDataSource()->changeTable(o,n);
	    }
	    return false;
	}
	void saveSignals(const char *prefix);
	void display(int i);
	void saveAzSlow(void);

	bool show_single;
	double *slowness;
	float fkmin[4], fkmax[4];
	WaveformPlot	*wp;

    protected:
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	ConPlotClass	*plot1, *plot2, *plot3, *plot4;
	AxesLabels	*labels_window;
	FkParamDialog	*parameter_window;
	RefSta		*ref_sta_window;
	TaperWindow	*taper_window;
	FkSignal	*signal_window;
	PrintDialog	*print_window;

	char net[10];
	int nbands;
	double fmin[4], fmax[4];
	bool auto_display;
	bool ignore_callback;
	int ids[8];
	int callback_reason;
	ConPlotClass *conplot[4];
	BeamWaveformView *beam_plot;
	int count;
	RTDmode rtd_mode;
	char rtd_sta[10];
	char rtd_chan[10];
	DataSource *rtd_data_source;
	double rtd_save_secs;
	bool redraw;
	bool update_ftrace;
	bool display_data[2];
	bool display_grid[2];
	AxesClass *line_drag;
	BasicMap *map;
	gvector<Waveform *> wvec;
	GTimeSeries *beams[4];
	cvector<CssArrivalClass> arrivals;
	cvector<CssAssocClass> assocs;
	vector <FK *> windows;
	char status_msg[200];
	double snr[4], fkmax_snr[4];
	double auto_tbeg, auto_tend;
	Beam *gbeam;

//	RTDWindow rtd;

	FKParam		*p;
	FKData		*current_fk;

	// File menu
	Button *compute_button, *auto_compute_button, *print_button;
	Button *new_fk_button, *close_button;

	// Edit menu
	UndoButton *undo_button;

	// View menu
	Toggle *grid_toggle, *fine_grid_toggle, *wide_grid_toggle;
	Menu *units_menu;
	Toggle *units_km_toggle, *units_deg_toggle, *predicted_toggle;
	Button *labels_button, *start_button, *back_button, *forward_button;

	// Option menu
	Toggle *auto_cursor_toggle, *rtd_compute_toggle, *align_toggle;
	Button *rtd_save_button;
	Button *beam_button, *d3view_button, *freq_bands_button;
	Menu *location_menu;
	Toggle *dnorth_deast_toggle, *ref_sta_toggle, *array_center_toggle;
	Button *ref_sta_button, *peaks_button, *param_button, *save_slow_button;
	Button *sig_meas_button;
	Menu *taper_menu;
	Toggle *hanning_toggle, *hamming_toggle, *cosine_toggle, *parzen_toggle;
	Toggle *welch_toggle, *blackman_toggle, *none_toggle;
	Button *taper_per_button, *ftrace_button;

	// Help menu
	Button *fk_help_button;

	Pane *pane;
	Table *table;

	// single fk
	Form *freq_form, *form1, *form2, *form3, *form4;
	Label *lo_label[4], *hi_label[4];
	Scale *lo_scale[4], *hi_scale[4];
	TextField *lo_text[4], *hi_text[4];

	// multi fk
	Form *form;

	void createInterface(FKType);
	void createSingleInterface(void);
	void createMultiInterface(void);

	void init(FKType);
	void actionPerformed(ActionEvent *action_event);
	void print(void);
	void setButtonsSensitive(void);
	void showRefSta(bool visible);
	void startDisplay(void);
	void stopDisplay(void);
	void displayFirst(void);
	double getDataShift(FKData *fk);
	void autoStep(bool forward);
	void autoStart(void);
	void wideGrid(void);
	void changeUnits(void);
	void setLabels();
	void tableGetAzSlow(int fk_units, const char *slow_text,
		const char *az_text, double *azimuth, double *sec_per_km);
	void tableEdit(MmTableEditCallbackStruct *);
	bool getBands(void);
	void tieLimits(Component *comp, AxesLimitsCallbackStruct *s);
	void GetResources(XtPointer sub_data);
	void drawFK(bool draw_conplot);
	void windowInfo(void);
	bool compute(bool show_warning, bool redraw);
	bool computeAll(bool show_warning, bool redraw);
	int FkGetWaveforms(int *windowed, double *tmin, double *tmax,
		bool warning);
	void getParams(double *slowness_max, int *n_slowness,
		bool *display_data, bool *display_grid,
		double *signal_slow_min, double *signal_slow_max,
		double *signal_az_min, double *signal_az_max,
		int *taper, double *begTaper, double *endTaper);
	void returnTaper(int *taper, double *begTaper, double *endTaper);
	void rtdContinue(void);
	int addToGram(void);
	void setGramButtonsOn(bool on);
	void updateSignalMeasurements(void);
	int addToGramGetWaveforms(gvector<Waveform *> &wvec);
	bool haveData(void);
	int allGetWaveforms(int *windowed, bool show_warning);
	bool allGetParam(int nbands, double dt,
		double *slowness_max, int *n_slowness, bool *display_data,
		bool *display_grid, double *window_length,
		double *window_overlap, double *signal_slow_min,
                double *signal_slow_max, double *signal_az_min,
                double *signal_az_max, int *taper, double *begTaper,
		double *endTaper, double *stav_len, double *ltav_len,
		bool *scan_freqs, double *bandwidth);
	int getArrayElements(gvector<Waveform *> &ws, bool show_warning) {
		double tmin, tmax;
		return getArrayElements(ws, &tmin, &tmax, show_warning);
	}
	int getArrayElements(gvector<Waveform *> &wvec, double *tmin,
		double *tmax, bool show_warning);
	void displayFKs(bool *display_data, bool *display_grid, FKFine *fine,
		const char *sta, bool redraw);
	void crosshairCB(ConPlotClass *cp, AxesCursorCallbackStruct *p);
	void grid(void);
	void fineGrid(void);
	void doubleLine(AxesCursorCallbackStruct *axes_cursor);
	void windowDrag(AxesCursorCallbackStruct *c);
	void computeButtonsOn(bool on);
	void lineDrag(AxesClass *comp, AxesCursorCallbackStruct *c);
	void positionLine(CPlotClass *cp, double x);
	void getPhase(FKData *fk, char *phase_lab, int len);
	void drawSingleFK(void);
	void show3D(void);
	void crosshairAction(double slowness, double az, int reason, int row);
	void redrawMap(void);
	void drawOnMap(double az, int reason, int row);
	bool getLags(gvector<Waveform *> &wvec, double az, double slowness,
		vector<double> &t_lags, double *beam_lat=NULL,
		double *beam_lon=NULL);
	bool getBeamCursor(AxesCursorCallbackStruct **a);
	void fkBeam(void);
	void makeBeam(double az, double sec_per_km, const char *chan);
	void makeBeam(double az, double sec_per_km, const char *chan,
		double time, double endtime);
	void displayBeams(double sec_per_km, double az, int row);
	void setFreqScales(FKData *fkdata);
	static bool workingUpdate(int count, int status,
		const char *label_format);
	void tableSelect(void);
	void align(double slow, double az);
	void doFtrace(void);
	ParseVar parseFK(FKData *fkd, int i, const char *a, string &value);

    private:
	static void nextFK(XtPointer client_data, XtIntervalId id);

};

} // namespace libgfk

#endif
