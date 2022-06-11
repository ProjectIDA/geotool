#ifndef _WAVEFORM_WINDOW_H
#define _WAVEFORM_WINDOW_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "motif++/Menu.h"
#include "WaveformView.h"
#include "gobject++/DataSource.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/TtPlot.h"

class CssFileDialog;
class TableFiles;
class MouseHelp;
class Scroll;
class TableViewer;
class TableQuery;
class PluginTable;
class Preferences;

/**
 *  @ingroup libgx
 */
class WaveformWindow : public Frame, public DataReceiver, public DataSource,
			public PrintClient
{
    public:
	WaveformWindow(const string &name, Component *parent,
		const string &title="", bool independent=true);
	~WaveformWindow(void);

	WaveformView	*wplot;
	PrintDialog	*print_window;

	void actionPerformed(ActionEvent *action_event);
        virtual void destroy(void);
	virtual void print(FILE *fp, PrintParam *p);

	void setDataSource(DataSource *ds) {
//		setVisible(true);
		wplot->setDataSource(ds); }
	void alignWaveforms(AlignType type);
	void alignWaveforms(AlignType type, const string &phase);
	void setAlignToggles(AlignType type);
	void clear(void);
	void deleteSelectedWaveforms(void);
	void copyData(void);
	void cutData(void);
	void pasteData(void);
	void travelTimeCurves(bool on);
	void sortByDistanceFrom(double lat, double lon);
	void appendRecentInput(RecentInput *r);
	void addRecentInput(RecentInput *r);
	void readFile(const string &file, const string &query,
		bool update_recent_input=true)
	{
	    vector<string> files;
	    files.push_back(file);
	    readFiles(files, query, update_recent_input);
	}
	void readFile(const string &file) {
	    readFile(file, "", true);
	}
	void readFiles(vector<string> &files, const string &query,
			bool update_recent_input=true);
	void readFiles(vector<string> &files) {
	    readFiles(files, "", true);
	}

        void selectWaveforms(SelectWaveformType type) {
	    wplot->selectWaveforms(type);
	}
#ifdef __STDC__
	ParseCmd putCmd(const char *format, ...);
#else
	ParseCmd putCmd(va_alist);
#endif
	void changeWindowMenu(const string &window_name);
	void windowManagerClose(void);

	// DataSource interface:
	virtual WaveformWindow *getWaveformWindowInstance(void) { return this; }
	virtual WaveformPlot *getWaveformPlotInstance(void) {
		return this->wplot; }
	bool reread(GTimeSeries *ts) { return ts->reread(); }
	int getWaveforms(gvector<Waveform *> &wvec,
		bool displayed_only=true) {
	    return wplot->getWaveforms(wvec, displayed_only); }
	int getPathInfo(PathInfo **path_info) {
	    return wplot->getPathInfo(path_info); }
	Waveform *getWaveform(int id) {
	    return wplot->getWaveform(id); }
	int getSelectedWaveforms(gvector<Waveform *> &wvec) {
	    return wplot->getSelectedWaveforms(wvec); }
	int copySelectedTimeSeries(gvector<GTimeSeries *> &tvec) {
	    return wplot->copySelectedTimeSeries(tvec); }
	int getSelectedWaveforms(const string &cursor_label,
		gvector<Waveform *> &wvec) {
	    return wplot->getSelectedWaveforms(cursor_label, wvec); }
	int getSelectedComponents(const string &cursor_label,
		const string &comps, gvector<Waveform *> &wvec) { 
	    return wplot->getSelectedComponents(cursor_label, comps, wvec); }
	int getSelectedComponents(const string &comps,
		gvector<Waveform *> &wvec){
	    return wplot->getSelectedComponents(comps, wvec); }
	int getSelectedComponents(vector<int> &num_cmpts,
		gvector<Waveform *> &wvec) {
	    return wplot->getSelectedComponents(num_cmpts, wvec); }
	int getSelectedComponents(const string &cursor_label,
		vector<int> &num_cmpts, gvector<Waveform *> &wvec) {
	    return wplot->getSelectedComponents(cursor_label,num_cmpts,wvec);
	}
	bool dataWindowIsDisplayed(const string &cursor_label) {
	    return wplot->dataWindowIsDisplayed(cursor_label); }
	CssOriginClass *getPrimaryOrigin(Waveform *w) {
	    return wplot->getPrimaryOrigin(w); }
	int getArrivalsOnWaveform(Waveform *w,
		cvector<CssArrivalClass> &arrivals) {
	    return wplot->getArrivalsOnWaveform(w, arrivals); }
	long getWorkingOrid(void) { return wplot->getWorkingOrid(); }
	void setWorkingOrid(long orid, bool do_callback=true) {
	    wplot->setWorkingOrid(orid, do_callback); }

	int getTable(cvector<CssAmpdescriptClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssAmplitudeClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssArrivalClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssAssocClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssOriginClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssOrigerrClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssParrivalClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssNetmagClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssStamagClass> &v) { return wplot->getTable(v); }
	int getTable(cvector<CssWftagClass> &v) { return wplot->getTable(v); }

	int getTable(cvector<CssAffiliationClass> &v) { return 0; }
	int getTable(cvector<CssHydroFeaturesClass> &v) { return 0; }
	int getTable(cvector<CssInfraFeaturesClass> &v) { return 0; }
	int getTable(cvector<CssStassocClass> &v) { return 0; }
	int getTable(cvector<CssSiteClass> &v) { return 0; }
	int getTable(cvector<CssSitechanClass> &v) { return 0; }
	int getTable(cvector<CssWfdiscClass> &v) { return 0; }
	int getTable(cvector<CssXtagClass> &v) { return 0; }

	int getSelectedTable(cvector<CssAffiliationClass> &v) { return 0; }
	int getSelectedTable(cvector<CssAmpdescriptClass> &v) { return 0; }
	int getSelectedTable(cvector<CssAmplitudeClass> &v) { return 0; }
	int getSelectedTable(cvector<CssAssocClass> &v) { return 0; }
	int getSelectedTable(cvector<CssHydroFeaturesClass> &v) { return 0; }
	int getSelectedTable(cvector<CssInfraFeaturesClass> &v) { return 0; }
	int getSelectedTable(cvector<CssNetmagClass> &v) { return 0; }
	int getSelectedTable(cvector<CssOriginClass> &v) { return 0; }
	int getSelectedTable(cvector<CssOrigerrClass> &v) { return 0; }
	int getSelectedTable(cvector<CssParrivalClass> &v) { return 0; }
	int getSelectedTable(cvector<CssStamagClass> &v) { return 0; }
	int getSelectedTable(cvector<CssStassocClass> &v) { return 0; }
	int getSelectedTable(cvector<CssSiteClass> &v) { return 0; }
	int getSelectedTable(cvector<CssSitechanClass> &v) { return 0; }
	int getSelectedTable(cvector<CssWfdiscClass> &v) { return 0; }
	int getSelectedTable(cvector<CssWftagClass> &v) { return 0; }
	int getSelectedTable(cvector<CssXtagClass> &v) { return 0; }

	int getSelectedTable(cvector<CssArrivalClass> &v) {
		return wplot->getSelectedTable(v); }

	int getTable(const string &cssTableName, gvector<CssTableClass *> &v) {
		return wplot->getTable(cssTableName, v); }
	int getSelectedTable(const string &cssTableName, gvector<CssTableClass *> &v)
	{
		return wplot->getSelectedTable(cssTableName, v); }

	void removeTable(CssTableClass *table, bool do_callback=true) {
		wplot->removeTable(table, do_callback); }

	void putTable(CssTableClass *table, bool do_callback) {
		wplot->putTable(table, do_callback);}

	void addDataListener(Component *comp) { wplot->addDataListener(comp); }
	void removeDataListener(Component *comp) {
		wplot->removeDataListener(comp); }
	void selectWaveform(Waveform *w, bool select, bool do_callbacks) {
		wplot->selectWaveform(w, select, do_callbacks); }
	bool selectRecord(CssTableClass *record, bool select,bool do_callback=false){
        	return wplot->selectRecord(record, select, do_callback); }
	void modifyWaveforms(gvector<Waveform *> &wvec) {
		wplot->modifyWaveforms(wvec); }
	void modifyWaveform(Waveform *w) {
		wplot->modifyWaveform(w); }
	bool deleteArrivals(cvector<CssArrivalClass> &arrivals) {
		return wplot->deleteArrivals(arrivals); }
	bool deleteOrigins(cvector<CssOriginClass> &origins) {
		return wplot->deleteOrigins(origins); }

	bool isSelected(CssTableClass *table) { return wplot->isSelected(table); }
	bool selectTable(CssTableClass *table, bool state) {
		return wplot->selectTable(table, state);
	}

	int numWaveforms(void) { return wplot->numWaveforms(); }
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd parseEditOrRemove(bool edit, const char *s, string &msg);
	bool parseSelectWaveforms(const string &cmd, string &msg, bool select);
	void parseHelp(const char *prefix);

	static void makeIcon(Display *display, Drawable d, int size);

    protected:
	CssFileDialog	*open_file;
	MouseHelp	*mouse_help_window;
	TableViewer	*tableviewer_window;
	TableQuery	*table_query;
	PluginTable	*plugin_table;
	Preferences	*preferences;
	vector<WaveformWindow *> windows;

	// File Menu
	Button	*open_db_button, *open_file_button,*output_button,*print_button;
	Button	*tv_button;
	Menu	*recent_files_menu;
	Button	*clear_recent_button, *preferences_button;
	Button	*waveform_setup_button, *close_button, *quit_button;
	Button	*table_files_button, *messages_button;
	Button	*new_window_button, *warnings_button;
	Button  *plugins_button;

	// Edit Menu
	Button	*clear_button, *delete_data_button, *copy_button, *cut_button;
	Button	*paste_button;
	Menu	*filter_menu;
	Button	*bfilter_button, *unfilter_button;
	Menu	*calib_menu, *demean_menu, *polarity_menu;
	Button	*display_calib_button, *display_counts_button;
	Button	*remove_mean_button, *restore_mean_button;
	Button	*normal_polarity_button, *reverse_polarity_button;
	UndoButton	*undo_button;

	// View Menu
	Menu	*align_menu;
	Menu	*components_menu;
	Menu	*cursors_menu;
	Button	*add_time_line, *add_time_limits, *add_crosshair,
			*remove_all_cursors;
	Menu	*data_movement_menu;
	Button	*data_values_button;
	Menu	*display_arrivals_menu;
	Menu	*display_data_menu;
	Button	*all_data, *selected_data;
	Button	*labels_button;
	Button	*magnify_button;
	Button	*promote_selected;
	Menu	*scale_menu;
	Button	*space_more, *space_less, *scale_up, *scale_down, *set_scale;
	Button	*scroll_button;
	Menu	*select_menu;
	Button	*select_all, *deselect_all, *select_visible, *select_visible_z,
			*select_visible_horz;
	Button	*set_default_order;
	Menu	*tags_menu;

	Menu	*sort_menu;

	Menu	*time_scale_menu;

	Button	*unzoom_all;
	Button	*waveform_color_button;

	// Option Menu
	Menu	*array_menu, *tables_menu;
	Button	*create_beam_button;
	Button  *diff_button;
	Button	*history_button;
	Button	*waveforms_button;
	Button	*waveform_tables_button;

	// Help Menu
	Button *help_button, *doc_button;
	Button *programmers_button, *version_button, *browser_button;

	WaveformPlotToggles wt;
	gvector<RecentInput *> recent_input;
	RecentInput	*current_input;

	Toggle  *align_toggle;
	Toggle  *time_scale_toggle;
	Toggle  *travel_times_toggle;

	void createInterface(void);
	void open(void);
	void setEditMenuSensitive(void);
	void setViewMenuSensitive(void);
        void getRecentInput(void);
	void addRecentInput(RecentInput *r, int position);
	void addRecentButton(RecentInput *r, int position);
	void clearRecentMenu(void);
	void saveRecentInput(void);

    private:
};

#endif
