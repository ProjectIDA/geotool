/** \file WaveformPlot.cpp
 *  \brief Defines class WaveformPlot.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "libgio.h"
#include "WaveformPlot.h"
#include "Amp.h"
#include "FKData.h"
#include "gobject++/DataSource.h"
#include "widget/PrintDialog.h"
#include "gobject++/SegmentInfo.h"
#include "WaveformConstraint.h"
#include "System.h"
#include "gobject++/WaveformTag.h"
#include "Waveform.h"
#include "Iaspei.h"
#include "motif++/MotifClasses.h"
#include "TableSource.h"
#include "TaperData.h"
#include "CalibData.h"
#include "OffsetData.h"
#include "AmpData.h"
#include "Demean.h"
#include "Beam.h"
#include "Response.h"
#include "SacSource.h"
#include "SeedSource.h"
#include "Scroll.h"
#include "GseSource.h"
#include "AscSource.h"
#include "DataValues.h"
#include "Filter.h"
#include "Output.h"
#include "AmplitudeScale.h"
#include "AxesLabels.h"
#include "History.h"
#include "CreateBeam.h"
#include "TagContents.h"
#include "WaveformColor.h"
#include "Waveforms.h"
#include "TableViewer.h"
#include "RotateData.h"
#include "Diff.h"

#include "gobject++/CssTables.h"
#include "cssio.h"

using namespace std;

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

class LoadedTableList : public TableViewer
{
    public:
	LoadedTableList(const string &name, Component *parent,
			DataSource *ds, const string &title) :
			TableViewer(name, parent, title)
	{
	    data_source = ds;
	    data_source->addDataListener(this);
	    open_button->setVisible(false);
	    add_row_button->setVisible(false);
	    clear_button->setVisible(false);
	    paste_button->setVisible(false);
	    delete_button->setVisible(false);
	    remove_tab->setVisible(false);
	}
	void actionPerformed(ActionEvent *action_event)
	{
	    const char *reason = action_event->getReason();

	    if(!strcmp(reason, XtNdataChangeCallback))
	    {
		DataChange *d = (DataChange *)action_event->getCalldata();
		if(d->arrival || d->assoc || d->origin || d->origerr ||
		    d->stassoc || d->stamag || d->netmag || d->hydro ||
		    d->infra || d->wftag || d->amplitude || d->ampdescript ||
		    d->filter || d->parrival || d->waveform)
		{
		    displayAllTables(data_source);
		}
	    }
	    else {
		TableViewer::actionPerformed(action_event);
	    }
	}
	~LoadedTableList(){ }

	DataSource *data_source;
};

class UndoDeleteOrigin : public UndoAction
{
    public:
	UndoDeleteOrigin(WaveformPlot *w) {
	    wplot = w;
	    error_msg = NULL;
	}
	~UndoDeleteOrigin() {
	}
	WaveformPlot *wplot;
	cvector<CssOriginClass> origins;
	cvector<CssOrigerrClass> origerrs;
	cvector<CssNetmagClass> netmags;
	cvector<CssAssocClass> assocs;
	cvector<CssStamagClass> stamags;
	cvector<CssWftagClass> wftags;

	bool undo(void) {
	    return wplot->undoDeleteOrigin(this);
	}
	void getLabel(string &label) {
	    if(origins.size() > 1) label.assign("Undo Delete Origins");
	    else label.assign("Undo Delete Origin");
	}
	bool errMsg(string &msg) {
	    if(error_msg) { msg.assign(error_msg); return true; }
	    else { msg.clear(); return false; }
	}
    protected:
	char *error_msg;
};

class UndoDeleteTables : public UndoAction
{
    public:
	UndoDeleteTables(WaveformPlot *w) {
	    wplot = w;
	    error_msg = NULL;
	}
	~UndoDeleteTables() { }
	WaveformPlot *wplot;
	gvector<CssTableClass *> tables;

	bool undo(void) { return wplot->undoDeleteTables(this); }
	void getLabel(string &label) {
	    const char *name= tables[0]->getName();
	    if(tables.size() > 1) {
		label.assign(string("Undo Delete ") + name + "s");
	    }
	    else {
		label.assign(string("Undo Delete ") + name);
	    }
	}
	bool errMsg(string &label) {
	    if(error_msg) { label.assign(error_msg); return true; }
	    else { label.clear(); return false; }
	}
    protected:
	char *error_msg;
};

/// @cond
typedef struct {
    cvector<CssArrivalClass>		*arrivals;
    cvector<CssOriginClass>		*origins;
    cvector<CssOrigerrClass>		*origerrs;
    cvector<CssAssocClass>		*assocs;
    cvector<CssStassocClass>		*stassocs;
    cvector<CssStamagClass>		*stamags;
    cvector<CssNetmagClass>		*netmags;
    cvector<CssHydroFeaturesClass>	*hydros;
    cvector<CssInfraFeaturesClass>	*infras;
    cvector<CssWftagClass>		*wftags;
    cvector<CssAmplitudeClass>		*amplitudes;
    cvector<CssAmpdescriptClass>	*ampdescripts;
    cvector<CssFilterClass>		*filters;
    cvector<CssParrivalClass>		*parrivals;
} AllTables;
/// @endcond

static int num_waveform_colors = 0;
static Pixel *waveform_colors = NULL;

WaveformPlot::WaveformPlot(const string &name, Component *parent, Arg *args,
		int n, DataSource *ds) : TtPlotClass(name, parent, args, n),
		DataReceiver()
{
    data_source = ds;
    if(data_source) data_source->addDataReceiver(this);
    init(name, parent, args, n);
}

WaveformPlot::WaveformPlot(const string &name, Component *parent,	
		InfoArea *infoarea, Arg *args, int n, DataSource *ds) :
		TtPlotClass(name, parent, infoarea, args, n), DataReceiver()
{
    data_source = ds;
    if(data_source) data_source->addDataReceiver(this);
    init(name, parent, args, n);
}

WaveformPlot::WaveformPlot(const string &name, Component *parent, Arg *args,
		int n) : TtPlotClass(name, parent, args, n), DataReceiver()
{
    data_source = NULL;
    init(name, parent, args, n);
}

WaveformPlot::WaveformPlot(const string &name, Component *parent,
		InfoArea *infoarea, Arg *args, int n) :
		TtPlotClass(name, parent, infoarea, args, n), DataReceiver()
{
    data_source = NULL;
    init(name, parent, args, n);
}

void WaveformPlot::init(const string &name, Component *parent, Arg *args, int n)
{
    int m;
    Arg a[20];

    // If the following resource are not input to the constructor, set them
    // as follows.
    m = 0;
    if( !inArgs(XtNyLabelInt, args, n) ) {
	XtSetArg(a[m], XtNyLabelInt, True); m++;
    }
    if( !inArgs(XtNdisplayTags, args, n) ) {
	XtSetArg(a[m], XtNdisplayTags, True); m++;
    }
    if( !inArgs(XtNdisplayAmplitudeScale, args, n) ) {
	XtSetArg(a[m], XtNdisplayAmplitudeScale, True); m++;
    }
    if( !inArgs(XtNtimeScale, args, n) ) {
	XtSetArg(a[m], XtNtimeScale, True); m++;
    }
    if( !inArgs(XtNextraXTickmarks, args, n) ) {
	XtSetArg(a[m], XtNextraXTickmarks, True); m++;
    }
    if( !inArgs(XtNextraYTickmarks, args, n) ) {
	XtSetArg(a[m], XtNextraYTickmarks, True); m++;
    }
    if( !inArgs(XtNautoYScale, args, n) ) {
	XtSetArg(a[m], XtNautoYScale, False); m++;
    }
    if( !inArgs(XtNdataMovement, args, n) ) {
	XtSetArg(a[m], XtNdataMovement, CPLOT_NO_MOVEMENT); m++;
    }
    if( !inArgs(XtNdisplayArrivals, args, n) ) {
	XtSetArg(a[m], XtNdisplayArrivals, CPLOT_ARRIVALS_ALL_CHAN); m++;
    }
    if( !inArgs(XtNtimeScale, args, n) ) {
	XtSetArg(a[m], XtNtimeScale, TIME_SCALE_HMS); m++;
    }
    if( !inArgs(XtNusePixmap, args, n) ) {
	XtSetArg(a[m], XtNusePixmap, True); m++;
    }
    if( !inArgs(XtNallowPartialSelect, args, n) ) {
	XtSetArg(a[m], XtNallowPartialSelect, False); m++;
    }
    if( !inArgs(XtNdisplayTtCurves, args, n) ) {
	XtSetArg(a[m], XtNdisplayTtCurves, False); m++;
    }
    if( !inArgs(XtNcurvesOnly, args, n) ) {
	XtSetArg(a[m], XtNcurvesOnly, False); m++;
    }
    if( !inArgs(XtNdrawYLabels, args, n) ) {
	XtSetArg(a[m], XtNdrawYLabels, True); m++;
    }
    if( !inArgs(XtNdisplayAddArrival, args, n) ) {
	XtSetArg(a[m], XtNdisplayAddArrival, True); m++;
    }

    if(m) setValues(a, m);

    XtAddCallback(base_widget, XtNampConvertCallback,
		ampConvertCallback, (XtPointer)this);

    enableCallbackType(XtNdataChangeCallback);
    enableCallbackType(XtNdataQCCallback);
    enableCallbackType(XtNalignCallback);
    enableCallbackType("measureArrivalCallback");
    enableCallbackType("measureAmplitudeCallback");

    if(!getProperty("color_code", color_code)) {
	color_code.assign("network");
    }

    sort_type = SORT_DEFAULT_ORDER;
    time_align = ALIGN_TRUE_TIME;

    next_color = -1;
    tt_curves_on = false;
    display_working = true;
    working_dialog_threshold = 3;

    sort_from_lat = -999.;
    sort_from_lon = -999.;

    time_scale_epoch = false;

    auto_vertical_scale = false;

    apply_calib = true;

    max_nyquist = 0.;

    for(int i = 0; i < 40; i++) tag_members[i] = 0;
    num_tag_members = 2;
    tag_members[0] = 1;
    tag_members[1] = 2;

    beam_index = 0;
    
    next_color = -1;
//    num_waveform_colors = getWaveformColors(comp_parent, &waveform_colors);

    arrival_color_mode = DEFAULT_COLOR;

    num_wave_plots = 1;
    wave_plots[0] = this;
    for(int i = 1; i < 100; i++) wave_plots[i] = NULL;

    menu_popup = NULL;
    arrival_add_popup = NULL;
    menu_made = false;

    print_window = NULL;
    amplitude_scale = NULL;
    bfilter_window = NULL;
    output_window = NULL;
    data_values_window = NULL;
    diff_window = NULL;
    tag_contents = NULL;
    labels_window = NULL;
    waveform_setup = NULL;
    history_window = NULL;
    waveform_window = NULL;
    loaded_tables_window = NULL;
    scroll_window = NULL;
    waveform_color_window = NULL;
    create_beam_window = NULL;

    hold_data_change_callbacks = false;

    outside_edit_menu = NULL;
    outside_view_menu = NULL;

    string recipe_dir, recipe_dir2;
    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    recipe_dir = c + string("/tables/recipes");
	}
    }
    getProperty("recipeDir2", recipe_dir2);

    gbeam = new Beam(recipe_dir, recipe_dir2);

    getProperty("add_arrival_menu", arrival_menu_phases);
    arrival_menu_sta = NULL;

    initArrivalKeys();

//    addDOwner(this);

    addActionListener(this, XtNmenuCallback);
    addActionListener(this, XtNaddArrivalCallback);
    addActionListener(this, XtNaddArrivalMenuCallback);

    makePopup();
    makePopupMenu();

    Application *app = Application::getApplication();
    app->addReservedName("auto_scale");
    app->addReservedName("display_tags");
    app->addReservedName("partial_select");
    app->addReservedName("uniform_scale");

    table_source = NULL;
}

void WaveformPlot::destroy(void)
{
    TtPlotClass::destroy();
    XtRemoveCallback(base_widget, XtNampConvertCallback,
	ampConvertCallback, (XtPointer)this);

}

void WaveformPlot::ampConvertCallback(Widget w, XtPointer client,XtPointer data)
{
    ((Component *)client)->setCursor("hourglass");
    ampConvert((CPlotMeasure *)data);
    ((Component *)client)->setCursor("default");
}

void WaveformPlot::ampConvert(CPlotMeasure *m)
{
    for(int i = 0; i < 1000 && m[i].w != NULL; i++)
    {
        GTimeSeries *ts = m[i].w->ts;
        GSegment *s = ts->nearestSegment(ts->tbeg() + m[i].left_side);
        double calib = (s->calib() != 0.) ? s->calib() : 1.;
        m[i].amp_cnts = m[i].amp_Nnms/fabs(calib);
        m[i].amp_nms = -1;

        if (calib < 0.0001) {
            fprintf(stderr, "suspicious calib value of %.3f for %s/%s\n",
                        calib, m[i].w->sta(), m[i].w->chan());
        }

        double amp_nms;
        if(BasicSource::cts2nms(ts, s, m[i].amp_cnts, m[i].period, &amp_nms))
        {
            m[i].amp_nms = amp_nms;
        }
    }
}

WaveformPlot::~WaveformPlot(void)
{
    delete gbeam;
    if(table_source) delete table_source;
}

void WaveformPlot::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Toggle *t = comp->getToggleInstance();
    Arg args[1];

    if(!strcmp(action_event->getReason(), XtNmenuCallback)) {
	menuPopup((XEvent *)action_event->getCalldata());
    }
    else if(!strcmp(action_event->getReason(), XtNaddArrivalCallback)) {
	addArrivalFromKey((CPlotArrivalCallbackStruct *)
			action_event->getCalldata());
    }
    else if(!strcmp(action_event->getReason(), XtNaddArrivalMenuCallback)) {
	addArrivalMenu((CPlotArrivalCallbackStruct *)
			action_event->getCalldata());
    }
    else if(!strncmp(cmd, "arrival-name ", 13)) {
	snprintf(arrival_cb.name, sizeof(arrival_cb.name), "%s", cmd+13);
	addArrivalFromKey(&arrival_cb);
    }
    else if(!strcmp(cmd, "Set Menu")) {
	setAddArrivalMenu();
    }
    else if(!strcmp(cmd, "Save as...") || !strcmp(cmd, "Output Waveforms...")) {
	output();
    }
    else if(!strcmp(cmd, "Arrival Parameters...")) {
	showArrivalParams();
    }
    else if(!strcmp(cmd, "Amplitude Parameters...")) {
	showAmplitudeParams();
    }
    else if(!strcmp(cmd, "Print...")) {
	printWindow();
    }
    else if(!strcmp(cmd, "Clear")) {
        clear();
    }
    else if(!strcmp(cmd, "Delete Data")) {
        deleteSelectedWaveforms();
    }
    else if(!strcmp(cmd, "Partial Select")) {
	if(t) {
	    XtSetArg(args[0], XtNallowPartialSelect, t->state());
	    setValues(args, 1);
	    syncToggles(t == wt.partial_select_toggle);
	}
    }
    else if(!strcmp(cmd, "Display Amplitude Scale")) {
	if(t) {
	    XtSetArg(args[0], XtNdisplayAmplitudeScale, t->state());
	    setValues(args, 1);
	    syncToggles(t == wt.display_amp_scale);
	}
    }
    else if(!strcmp(cmd, "Amplitude Scale")) {
	amplitudeScale((long)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Data Values...")) {
	if(!data_values_window) {
	    data_values_window = new DataValues("Data Values",
					formDialogParent(), this);
	}
	data_values_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Display counts*calib")) {
	displayCalib(true);
    }
    else if(!strcmp(cmd, "Display counts")) {
	displayCalib(false);
    }
    else if(!strcmp(cmd, "Remove Mean")) {
	removeMean(true);
    }
    else if(!strcmp(cmd, "Restore Mean")) {
	removeMean(false);
    }
    else if(!strcmp(cmd, "Normal")) {
	polarity(true);
    }
    else if(!strcmp(cmd, "Reverse")) {
	polarity(false);
    }
    else if(!strcmp(cmd, "Butterworth Filter...") || !strcmp(cmd, "Filter..."))
    {
	if(!bfilter_window) {
	    bfilter_window = new Filter("Butterworth Filter",
				formDialogParent(), this);
	}
	bfilter_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Unfilter All")) {
	unfilter(true);
    }
    else if(!strcmp(cmd, "On First Point")) {
	if(t && t->state()) {
	    alignWaveforms(ALIGN_FIRST_POINT);
	    syncToggles(t == wt.align_first_point);
	}
    }
    else if(!strcmp(cmd, "On True Time")) {
	if(t && t->state()) {
	    alignWaveforms(ALIGN_TRUE_TIME);
	    syncToggles(t == wt.align_true_time);
	}
    }
    else if(!strcmp(cmd, "Time Minus Origin")) {
	if(t && t->state()) {
	    alignWaveforms(ALIGN_ORIGIN);
	    syncToggles(t == wt.align_origin);
	}
    }
    else if(!strcmp(cmd, "Time Minus Predicted FirstP")) {
	if(t && t->state()) {
	    alignWaveforms(ALIGN_PREDICTED_P);
	    syncToggles(t == wt.align_predicted);
	}
    }
    else if(!strcmp(cmd, "Time Minus Observed FirstP")) {
	if(t && t->state()) {
	    alignWaveforms(ALIGN_OBSERVED_P);
	    syncToggles(t == wt.align_observed);
	}
    }
    else if(!strcmp(cmd, "All Components")) {
	if(t && t->state()) {
	    showComponents(ALL_COMPONENTS);
	    syncToggles(t == wt.all_components);
	}
    }
    else if(!strcmp(cmd, "Z Only")) {
	if(t && t->state()) {
	    showComponents(Z_COMPONENTS);
	    syncToggles(t == wt.z_only);
	}
    }
    else if(!strcmp(cmd, "N and E Only")) {
	if(t && t->state()) {
	    showComponents(N_AND_E_COMPONENTS);
	    syncToggles(t == wt.n_and_e_only);
	}
    }
    else if(!strcmp(cmd, "N Only")) {
	if(t && t->state()) {
	    showComponents(N_COMPONENTS);
	    syncToggles(t == wt.n_only);
	}
    }
    else if(!strcmp(cmd, "E Only")) {
	if(t && t->state()) {
	    showComponents(E_COMPONENTS);
	    syncToggles(t == wt.e_only);
	}
    }
    else if(!strcmp(cmd, "Add Time Line")) {
	addCursor(ADD_TIME_LINE);
    }
    else if(!strcmp(cmd, "Add Time Limits")) {
	addCursor(ADD_TIME_LIMITS);
    }
    else if(!strcmp(cmd, "Add Crosshair")) {
	addCursor(ADD_CROSSHAIR);
    }
    else if(!strcmp(cmd, "Remove All Cursors")) {
	addCursor(REMOVE_ALL_CURSORS);
    }
    else if(!strcmp(cmd, "no movement")) {
	if(t && t->state()) {
	    dataMovement(DATA_NO_MOVEMENT);
	    syncToggles(t == wt.no_movement);
	}
    }
    else if(!strcmp(cmd, "xy movement")) {
	if(t && t->state()) {
	    dataMovement(DATA_XY_MOVEMENT);
	    syncToggles(t == wt.xy_movement);
	}
    }
    else if(!strcmp(cmd, "x movement")) {
	if(t && t->state()) {
	    dataMovement(DATA_X_MOVEMENT);
	    syncToggles(t == wt.x_movement);
	}
    }
    else if(!strcmp(cmd, "y movement")) {
	if(t && t->state()) {
	    dataMovement(DATA_Y_MOVEMENT);
	    syncToggles(t == wt.y_movement);
	}
    }
    else if(!strcmp(cmd, "Off")) {
	if(t && t->state()) {
	    displayArrivals(ARRIVALS_OFF);
	    syncToggles(t == wt.arrivals_off);
	}
    }
    else if(!strcmp(cmd, "On One Channel")) {
	if(t && t->state()) {
	    displayArrivals(ON_ONE_CHANNEL);
	    syncToggles(t == wt.on_one_channel);
	}
    }
    else if(!strcmp(cmd, "On All Channels")) {
	if(t && t->state()) {
	    displayArrivals(ON_ALL_CHANNELS);
	    syncToggles(t == wt.on_all_channels);
	}
    }
    else if(!strcmp(cmd, "All Data")) {
	displayAllData(true);
    }
    else if(!strcmp(cmd, "Selected Only")) {
	displayAllData(false);
    }
    else if(!strcmp(cmd, "Display Tags")) {
	if(t) {
	    XtSetArg(args[0], XtNdisplayTags, t->state());
	    setValues(args, 1);
	    syncToggles(t == wt.display_tags);
	}
    }
    else if(!strcmp(cmd, "Promote Selected Waveforms")) {
	promoteWaveforms();
    }
    else if(!strcmp(cmd, "Space More")) {
	adjustDataHeight(SPACE_MORE);
    }
    else if(!strcmp(cmd, "Space Less")) {
	adjustDataHeight(SPACE_LESS);
    }
    else if(!strcmp(cmd, "Scale Up")) {
	adjustDataHeight(SCALE_UP);
    }
    else if(!strcmp(cmd, "Scale Down")) {
	adjustDataHeight(SCALE_DOWN);
    }
    else if(!strcmp(cmd, "Uniform Scale")) {
	if(t) {
	    setUniformAmpScale(t->state());
	    syncToggles(t == wt.uniform_scale);
	}
    }
    else if(!strcmp(cmd, "Auto Scale")) {
	if(t) {
	    setAutoScale(t->state());
	    syncToggles(t == wt.auto_scale);
	}
    }
    else if(!strcmp(cmd, "Set Scale...")) {
	setAmplitudeScale(true);
    }
    else if(!strcmp(cmd, "Select All")) {
	selectWaveforms(SELECT_ALL);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	selectWaveforms(DESELECT_ALL);
    }
    else if(!strcmp(cmd, "Select Visible")) {
	selectWaveforms(SELECT_VISIBLE);
    }
    else if(!strcmp(cmd, "Select Visible Z")) {
	selectWaveforms(SELECT_VISIBLE_Z);
    }
    else if(!strcmp(cmd, "Select Visible Horz")) {
	selectWaveforms(SELECT_VISIBLE_HORZ);
    }
    else if(!strcmp(cmd, "Set Default Order")) {
	setDefaultOrder();
    }
    else if(!strcmp(cmd, "Default Order")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_DEFAULT_ORDER);
	    syncToggles(t == wt.default_order);
	}
    }
    else if(!strcmp(cmd, "File Order")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_FILE_ORDER);
	    syncToggles(t == wt.file_order);
	}
    }
    else if(!strcmp(cmd, "Distance")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_DISTANCE);
	    syncToggles(t == wt.distance);
	}
    }
    else if(!strncmp(cmd, "Distance from", 13)) {
	if(t && t->state()) {
	    sortWaveforms(SORT_DISTANCE_FROM);
	    syncToggles(t == wt.distance_from);
	}
    }
    else if(!strcmp(cmd, "Time/Sta/Chan")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_TIME_STA_CHAN);
	    syncToggles(t == wt.time_sta_chan);
	}
    }
    else if(!strcmp(cmd, "Sta/Chan")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_STA_CHAN);
	    syncToggles(t == wt.sta_chan);
	}
    }
    else if(!strcmp(cmd, "Chan/Sta")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_CHAN2_STA);
	    syncToggles(t == wt.chan_sta);
	}
    }
    else if(!strcmp(cmd, "Back Azimuth")) {
	if(t && t->state()) {
	    sortWaveforms(SORT_BACK_AZIMUTH);
	    syncToggles(t == wt.back_azimuth);
	}
    }
    else if(!strcmp(cmd, "Tag Contents...")) {
	changeWaveformTag();
    }
    else if(!strcmp(cmd, "Tag Contents") && comp == tag_contents) {
	long dialog_state = *(long *)action_event->getCalldata();
	if(dialog_state == DIALOG_APPLY) {
	    int n, *order=NULL;
	    n = tag_contents->getOrder(&order);
	    char *text = tag_contents->getText();
	    setTagMembers(n, order, text, tag_contents->selectedOnly());
	    Free(order);
	    Free(text);
	}
    }
    else if(!strcmp(cmd, "Waveform Setup...")) {
	if(!waveform_setup) {
	    WaveformConstraint *c = &cons;
	    waveform_setup = new WaveformSetup("Waveform Setup",
		formDialogParent(), this, c->join_time_limit, c->overlap_limit,
		apply_calib);
	}
	waveform_setup->setVisible(true);
    }
    else if(!strcmp(cmd, "Waveform Setup")) {
	cons.join_time_limit = waveform_setup->joinTimeLimit();
	cons.overlap_limit = waveform_setup->overlapLimit();
	cons.window_length = waveform_setup->segmentLength();
	apply_calib = waveform_setup->applyCalib();
    }
    else if(!strcmp(cmd, "Labels...")) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Axes Labels", formDialogParent(),
					this);
	}
	labels_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Magnify Window...")) {
        showMagWindow();
    }
    else if(!strcmp(cmd, "History...")) {
	if(!history_window) {
	    history_window = new History("Method History", formDialogParent(),
					this);
	}
	history_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Diff...")) {
      if(!diff_window) {
	  diff_window = new Diff("Diff", formDialogParent(), this);
	}
	diff_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Create Beam...")) {
	if(!create_beam_window) {
	    create_beam_window = new CreateBeam("Create Beam",
					formDialogParent(), this);
	}
	create_beam_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Seconds")) {
	if(t && t->state()) {
	    setTimeScale(TIME_SECONDS);
	    syncToggles(t == wt.time_scale_seconds);
	}
    }
    else if(!strcmp(cmd, "Variable")) {
	if(t && t->state()) {
	    setTimeScale(TIME_VARIABLE);
	    syncToggles(t == wt.time_scale_variable);
	}
    }
    else if(!strcmp(cmd, "HMS")) {
	if(t && t->state()) {
	    setTimeScale(TIME_HMS);
	    syncToggles(t == wt.time_scale_hms);
	}
    }
    else if(!strcmp(cmd, "Epoch")) {
	if(t && t->state()) {
	    setTimeScale(TIME_EPOCH);
	    syncToggles(t == wt.time_scale_epoch_toggle);
	}
    }
    else if(!strcmp(cmd, "Unzoom All")) {
	unzoomAll();
    }
    else if(!strcmp(cmd, "Waveform Color...")) {
	if(!waveform_color_window) {
	    waveform_color_window = new WaveformColor("Waveform Color",
				formDialogParent(), this);
	}
	waveform_color_window->setVisible(true);
	waveform_color_window->setColor(getColorCode());
    }
    else if(comp == waveform_color_window) {
        setColorCode((char *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Scroll...")) {
	if(!scroll_window) {
	    scroll_window = new Scroll("Scroll", formDialogParent(), this);
	}
	scroll_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Waveforms...")) {
	if(!waveform_window) {
	    waveform_window = new Waveforms("Waveforms", formDialogParent(),
						this);
	}
	waveform_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Loaded Tables...")) {
	if(!loaded_tables_window) {
	    loaded_tables_window = new LoadedTableList("Loaded Tables",
				formDialogParent(), this, "Loaded Tables");
	}
	loaded_tables_window->setVisible(true);
	loaded_tables_window->displayAllTables(this);
    }
    else {
	TtPlotClass::actionPerformed(action_event);
    }
}

void WaveformPlot::addArrivalMenu(CPlotArrivalCallbackStruct *c)
{
    char s[20];
    if(!arrival_add_popup) {
	arrival_add_popup = createArrivalAddPopup();
    }
    if(c->shift) {
	snprintf(s, sizeof(s), "to: %s", c->w->sta());
    }
    else {
	snprintf(s, sizeof(s), "to: %s", c->w->net());
    }
    arrival_menu_sta->setLabel(s);
    arrival_add_popup->position((XButtonPressedEvent *)c->event);
    arrival_add_popup->setVisible(true);
    arrival_cb = *c;
}

PopupMenu * WaveformPlot::createArrivalAddPopup(void)
{
    Arg args[20];
    int n;
    Button *b;
    PopupMenu *popup;
    XtTranslations translations;
    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

    /* the default is button3, which will inactivate button3
     * in the AxesWidget, so set to 6 > all buttons
     */
    n = 0;
#if(XmVersion >= 2)
#ifndef XmNtearOffTitle
#define XmNtearOffTitle "tearOffTitle"
#endif
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
#endif
    XtSetArg(args[n], XmNwhichButton, 6); n++;
    XtSetArg(args[n], XmNradioBehavior, False); n++;
    popup = new PopupMenu("arrivalAddPopup", this, args, n);

    new Label("Add Arrival", popup);
    arrival_menu_sta = new Label("Sta", popup);
    new Separator("sep", popup);

    translations = XtParseTranslationTable(trans);
    n = 0;
    XtSetArg(args[n], XmNtranslations, translations); n++;

    if(arrival_menu_phases.empty()) {
	arrival_menu_phases.assign("P,Pn,S");
    }

    char *s = strdup(arrival_menu_phases.c_str());
    char *p, *tok, *last, name[50];
    tok = s;
    while((p = strtok_r(tok, ",", &last))) {
	tok = NULL;
	b = new Button(p, popup, args, n, this);
	snprintf(name, sizeof(name), "arrival-name %s", p);
	b->setCommandString(name);
    }
    Free(s);

    new Separator("sep", popup);
    b = new Button("Set Menu", popup, args, n, this);
    return popup;
}

void WaveformPlot::setAddArrivalMenu(void)
{
    string phases, ans;

    if( !TextQuestion::askTextQuestion("Set Add Arrival Menu",
		formDialogParent(), "Enter phases", ans, arrival_menu_phases,
		"Apply", "Cancel")) return; // Cancel

    if(!arrival_menu_phases.compare(ans)) {
	return;
    }
    arrival_menu_phases.assign(ans);
    putProperty("add_arrival_menu", ans);

    if(arrival_add_popup) {
	arrival_add_popup->destroy();
    }
    arrival_add_popup = createArrivalAddPopup();
}

ParseCmd WaveformPlot::parseCmd(const string &cmd, string &msg)
{
    bool err;
    string c;
    int id;
    ParseCmd ret;
    Waveform *w=NULL;
    gvector<Waveform *> wvec;
    Arg args[1];
    const char *cmds[] = {
        "deselect", "deselect_arrival", "filter", "output", "partial_select",
	"print_window", "rotate", "select", "select_arrival", "sort_waveforms",
        "tag_contents", "time_scale", "time_window", "time_zoom", "unfilter",
        "unrotate", "update_tag", "waveform_color",
        "waveform_setup", "zoom_on_waveform"
    };

    if( cmd.find('.') == string::npos ) {
	if(missingArgs(cmd, (int)sizeof(cmds)/(int)sizeof(const char *), cmds,
			msg) ) return ARGUMENT_ERROR;
    }
    if(parseGetArg(cmd, "", msg, "_wave_", &id)) {
	w = getWaveform(id);
	wvec.push_back(w);
    }

    if(parseString(cmd, "print_window", c)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print Waveforms",formDialogParent(),
					this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if((ret = parseOutput(w, cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseString(cmd, "output_window", c)) {
	if(!output_window) {
	    output_window = new Output("Save as", formDialogParent(), this);
	}
	return output_window->parseCmd(c, msg);
    }
    else if(parseFind(cmd, "clear", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
        clear();
    }
    else if(parseFind(cmd, "delete", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	if(!w) {
	    msg.assign("delete: missing wave object");
	    return ARGUMENT_ERROR;
	}
	deleteOne(w);
    }
    else if(parseFind(cmd, "delete_data", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
        deleteSelectedWaveforms();
    }
    else if(parseFind(cmd, "partial_select", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	XtSetArg(args[0], XtNallowPartialSelect, True);
	setValues(args, 1);
    }
    else if(parseFind(cmd, "total_select", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	XtSetArg(args[0], XtNallowPartialSelect, False);
	setValues(args, 1);
    }
    else if(parseString(cmd, "data_values", c)) {
        if(!data_values_window) {
            data_values_window = new DataValues("Data Values",
					formDialogParent(), this);
        }
        return data_values_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "axes_labels", c)) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Axes Labels", formDialogParent(),
						this);
	}
	return labels_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "set_axes_labels", c)) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Axes Labels", formDialogParent(),
						this);
	}
	return labels_window->parseCmd(c, msg);
    }
    else if(parseFind(cmd, "display_amplitude_scale", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	XtSetArg(args[0], XtNdisplayAmplitudeScale, True);
	setValues(args, 1);
    }
    else if(parseFind(cmd, "hide_amplitude_scale", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	XtSetArg(args[0], XtNdisplayAmplitudeScale, False);
	setValues(args, 1);
    }
    else if(parseString(cmd, "amplitude_scale", c)) {
        setAmplitudeScale(false);
        return amplitude_scale->parseCmd(c, msg);
    }
    else if(parseString(cmd, "set_layout", c) ||
		parseString(cmd, "set_current_scale", c)) {
        setAmplitudeScale(false);
        return amplitude_scale->parseCmd(cmd, msg);
    }
    else if(parseFind(cmd, "display_calib", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	if(w) {
	    if(changeCalib(w, true)) modifyWaveform(w);
	}
	else {
	    displayCalib(true);
	}
    }
    else if(parseFind(cmd, "display_counts", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	if(w) {
	    if(changeCalib(w, false)) modifyWaveform(w);
	}
	else {
	    displayCalib(false);
	}
    }
    else if(parseFind(cmd, "remove_mean", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	removeMean(true, w);
    }
    else if(parseFind(cmd, "restore_mean", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	removeMean(false, w);
    }
    else if(parseFind(cmd, "remove_all_methods", msg, &err, "_wave_", false))
    {
	if(err) return ARGUMENT_ERROR;
	if(w) {
	    w->ts->removeAllMethods();
	    modifyWaveform(w);
	}
	else if(getSelectedWaveforms(wvec) <= 0) {
	    msg.assign("No waveforms selected.");
	}
	else {
	    for(int i = 0; i < wvec.size(); i++) {
		wvec[i]->ts->removeAllMethods();
	    }
	    modifyWaveforms(wvec);
	}
    }
    else if(parseFind(cmd, "reverse_polarity", msg, &err, "_wave_", false))
    {
	if(err) return ARGUMENT_ERROR;
	polarity(false, w);
    }
    else if(parseFind(cmd, "normal_polarity", msg, &err, "_wave_", false))
    {
	if(err) return ARGUMENT_ERROR;
	polarity(true, w);
    }
    else if(parseFind(cmd, "position", msg, &err, "_wave_", false,
			"x", true, "y", true))
    {
	double x, y;
	if(err) return ARGUMENT_ERROR;
	if(!w) {
	    msg.assign("position: missing wave object");
	    return ARGUMENT_ERROR;
	}
	if(!parseGetArg(cmd, "position", msg, "x", &x)) {
	    msg.assign("position: cannot interpret x argument");
	    return ARGUMENT_ERROR;
	}
	if(!parseGetArg(cmd, "position", msg, "y", &y)) {
	    msg.assign("position: cannot interpret y argument");
	    return ARGUMENT_ERROR;
	}
	positionOne(w, x, y);
    }
    else if((ret = parseRotate(w, cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if((ret = parseUnrotate(w, cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseString(cmd, "butterworth_filter", c) ||
			parseString(cmd, "filter", c))
    {
	if(!bfilter_window) {
	    bfilter_window = new Filter("Butterworth Filter",
				formDialogParent(), this);
	}
	return bfilter_window->parseCmd(c, msg);
    }
    else if(parseFind(cmd, "unfilter", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	unfilter(false, w);
    }
    else if(parseFind(cmd, "unfilter_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	unfilter(true);
    }
    else if(parseFind(cmd, "align_on_phase", msg, &err, "phase", true)) {
	if(err) return ARGUMENT_ERROR;
	parseGetArg(cmd, "phase", c);
	alignOnPhase(c);
    }
    else if(parseFind(cmd, "align_on_predicted_phase", msg, &err, "phase",true))
    {
	if(err) return ARGUMENT_ERROR;
	parseGetArg(cmd, "phase", c);
	alignOnPredictedPhase(c);
    }
    else if(parseFind(cmd, "align_first_point", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	alignWaveforms(ALIGN_FIRST_POINT);
    }
    else if(parseFind(cmd, "align_true_time", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	alignWaveforms(ALIGN_TRUE_TIME);
    }
    else if(parseFind(cmd, "align_time_minus_origin", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	alignWaveforms(ALIGN_ORIGIN);
    }
    else if(parseFind(cmd, "align_predicted_firstP", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	alignWaveforms(ALIGN_PREDICTED_P);
    }
    else if(parseFind(cmd, "align_observed_firstP", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	alignWaveforms(ALIGN_OBSERVED_P);
    }
    else if(parseFind(cmd, "components_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	showComponents(ALL_COMPONENTS);
    }
    else if(parseFind(cmd, "components_z_only", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	showComponents(Z_COMPONENTS);
    }
    else if(parseFind(cmd, "components_n_and_e", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	showComponents(N_AND_E_COMPONENTS);
    }
    else if(parseFind(cmd, "components_n_only", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	showComponents(N_COMPONENTS);
    }
    else if(parseFind(cmd, "components_e_only", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	showComponents(E_COMPONENTS);
    }
    else if(parseFind(cmd, "add_time_line", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	addCursor(ADD_TIME_LINE);
    }
    else if(parseFind(cmd, "add_time_limits", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	addCursor(ADD_TIME_LIMITS);
    }
    else if(parseFind(cmd, "add_crosshair", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	addCursor(ADD_CROSSHAIR);
    }
    else if(parseFind(cmd, "remove_all_cursors", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	addCursor(REMOVE_ALL_CURSORS);
    }
    else if(parseFind(cmd, "movement_none", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	dataMovement(DATA_NO_MOVEMENT);
    }
    else if(parseFind(cmd, "movement_xy", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	dataMovement(DATA_XY_MOVEMENT);
    }
    else if(parseFind(cmd, "movement_x", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	dataMovement(DATA_X_MOVEMENT);
    }
    else if(parseFind(cmd, "movement_y", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	dataMovement(DATA_Y_MOVEMENT);
    }
    else if(parseFind(cmd, "arrivals_off", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	displayArrivals(ARRIVALS_OFF);
    }
    else if(parseFind(cmd, "arrivals_on_one_channel", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	displayArrivals(ON_ONE_CHANNEL);
    }
    else if(parseFind(cmd, "arrivals_on_all_channels", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	displayArrivals(ON_ALL_CHANNELS);
    }
    else if(parseFind(cmd, "display_all_data", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	displayAllData(true);
    }
    else if(parseFind(cmd, "display_selected_only", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	displayAllData(false);
    }
    else if(parseFind(cmd, "display_tags", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	XtSetArg(args[0], XtNdisplayTags, True);
	setValues(args, 1);
    }
    else if(parseFind(cmd, "hide_tags", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	XtSetArg(args[0], XtNdisplayTags, False);
	setValues(args, 1);
    }
    else if(parseFind(cmd, "promote_selected_waveforms", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	promoteWaveforms();
    }
    else if(parseFind(cmd, "space_more", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	adjustDataHeight(SPACE_MORE);
    }
    else if(parseFind(cmd, "space_less", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	adjustDataHeight(SPACE_LESS);
    }
    else if(parseFind(cmd, "scale_up", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	adjustDataHeight(SCALE_UP);
    }
    else if(parseFind(cmd, "scale_down", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	adjustDataHeight(SCALE_DOWN);
    }
    else if(parseFind(cmd, "uniform_scale", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setUniformAmpScale(True);
    }
    else if(parseFind(cmd, "independent_scale", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setUniformAmpScale(False);
    }
    else if(parseFind(cmd, "auto_scale_on", msg, &err)) {
	setAutoScale(true);
    }
    else if(parseFind(cmd, "auto_scale_off", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setAutoScale(false);
    }
    else if(parseFind(cmd, "scroll_left", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	scrollAxis("left");
    }
    else if(parseFind(cmd, "scroll_right", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	scrollAxis("right");
    }
    else if(parseFind(cmd, "scroll_up", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	scrollAxis("up");
    }
    else if(parseFind(cmd, "scroll_down", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	scrollAxis("down");
    }
    else if(parseString(cmd, "scroll_window", c)) {
	if(!scroll_window) {
	    scroll_window = new Scroll("Scroll", formDialogParent(), this);
	}
	return scroll_window->parseCmd(c, msg);
    }
    else if(parseFind(cmd, "select_all_arrivals", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectAllArrivals(true);
    }
    else if(parseFind(cmd, "deselect_all_arrivals", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectAllArrivals(false);
    }
    else if((ret = parseSelectArrivals(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseFind(cmd, "select_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectWaveforms(SELECT_ALL);
    }
    else if(parseFind(cmd, "deselect_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectWaveforms(DESELECT_ALL);
    }
    else if(parseFind(cmd, "select_visible", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectWaveforms(SELECT_VISIBLE);
    }
    else if(parseFind(cmd, "select_visible_z", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectWaveforms(SELECT_VISIBLE_Z);
    }
    else if(parseFind(cmd, "select_visible_horz", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	selectWaveforms(SELECT_VISIBLE_HORZ);
    }
    else if((ret = parseSelect(cmd, msg)) != COMMAND_NOT_FOUND){
	return ret;
    }
    else if(parseFind(cmd, "set_default_order", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setDefaultOrder();
    }
    else if(parseFind(cmd, "sort_waveforms", msg, &err, "order", true)) {
	if(err) return ARGUMENT_ERROR;
	string order;
	parseGetArg(cmd, "order", order);
	if(parseCompare(order, "default_order")) {
	    sortWaveforms(SORT_DEFAULT_ORDER);
	}
	else if(parseCompare(order, "file_order")) {
	    sortWaveforms(SORT_FILE_ORDER);
	}
	else if(parseCompare(order, "distance")) {
	    sortWaveforms(SORT_DISTANCE);
	}
/*
	else if(parseCompare(order, "distance_from", 13)) {
	    sortWaveforms(SORT_DISTANCE_FROM);
	}
*/
	else if(parseCompare(order, "time/sta/chan")) {
	    sortWaveforms(SORT_TIME_STA_CHAN);
	}
	else if(parseCompare(order, "sta/chan")) {
	    sortWaveforms(SORT_STA_CHAN);
	}
	else if(parseCompare(order, "chan/sta")) {
	    sortWaveforms(SORT_CHAN2_STA);
	}
	else if(parseCompare(order, "back_azimuth")) {
	    sortWaveforms(SORT_BACK_AZIMUTH);
	}
    }
    else if(parseString(cmd, "tag_contents", c)) {
	if(!tag_contents) {
	    tag_contents = new TagContents("Tag Contents", formDialogParent(),
			Waveform::numMemberNames(), Waveform::memberNames(),
			num_tag_members, tag_members, user_tag_string);
	    tag_contents->addActionListener(this);
	}
	return tag_contents->parseCmd(c, msg);
    }
    else if(parseFind(cmd, "update_tag", msg, &err, "_wave_", false)) {
	if(err) return ARGUMENT_ERROR;
	if(!w) {
	    msg.assign("update_tag: missing wave object");
	    return ARGUMENT_ERROR;
	}
	if(!tag_contents) {
	    tag_contents = new TagContents("Tag Contents", formDialogParent(),
			Waveform::numMemberNames(), Waveform::memberNames(),
			num_tag_members, tag_members, user_tag_string);
	    tag_contents->addActionListener(this);
        }
	updateTag(w);
    }
    else if(parseFind(cmd, "time_scale_seconds", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setTimeScale(TIME_SECONDS);
    }
    else if(parseFind(cmd, "time_scale_variable", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setTimeScale(TIME_VARIABLE);
    }
    else if(parseFind(cmd, "time_scale_hms", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setTimeScale(TIME_HMS);
    }
    else if(parseFind(cmd, "time_scale_epoch", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setTimeScale(TIME_EPOCH);
    }
    else if(parseString(cmd, "waveform_setup", c))
    {
	if(!waveform_setup) {
	    WaveformConstraint *wc = &cons;
	    waveform_setup = new WaveformSetup("Waveform Setup",
				formDialogParent(), this, wc->join_time_limit,
				wc->overlap_limit, apply_calib);
	}
	if((ret = waveform_setup->parseCmd(cmd, msg)) == COMMAND_PARSED) {
	    cons.join_time_limit = waveform_setup->joinTimeLimit();
	    cons.overlap_limit = waveform_setup->overlapLimit();
	    cons.window_length = waveform_setup->segmentLength();
	}
	return ret;
    }
    else if(parseString(cmd, "loaded_tables", c)) {
	if(!loaded_tables_window) {
	    loaded_tables_window = new LoadedTableList("Loaded Tables",
				formDialogParent(), this, "Loaded Tables");
	    loaded_tables_window->displayAllTables(this);
	}
	return loaded_tables_window->parseCmd(c, msg);
    }
    else if(parseFind(cmd, "unzoom_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	unzoomAll();
    }
    else if((ret = parseWaveformColor(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if((ret = parseTimeWindow(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseString(cmd, "create_beam", c)) {
	if(!create_beam_window) {
	    create_beam_window = new CreateBeam("Create Beam",
				formDialogParent(), this);
	}
	return create_beam_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "method_history", c)) {
	if(!history_window) {
	    history_window = new History("Method History", formDialogParent(),
					this);
	}
	return history_window->parseCmd(cmd, msg);
    }
    else if((ret = parseBeam(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseFind(cmd, "zoom_on_waveform", msg, &err, "_wave_", false))
    {
	if(err) return ARGUMENT_ERROR;
	if(w) {
	    zoomOnWaveforms(wvec);
	}
	else {
	    gvector<Waveform *> ws;
	    if(getSelectedWaveforms(ws) > 0) {
		zoomOnWaveforms(ws);
	    }
	}
    }
    else if(parseFind(cmd, "page_left", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	pageAxis("left");
    }
    else if(parseFind(cmd, "page_right", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	pageAxis("right");
    }
    else if(parseFind(cmd, "page_up", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	pageAxis("up");
    }
    else if(parseFind(cmd, "page_down", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	pageAxis("down");
    }
    else if(parseFind(cmd, "add_arrival", msg, &err,
		"phase", true, "time", true, "_wave_", false))
    {
	if(err) return ARGUMENT_ERROR;
	double time;
	if( !w ) {
	    msg.assign("add_arrival: missing wave object");
	    return ARGUMENT_ERROR;
	}
	string phase;
        parseGetArg(cmd, "phase", phase);
        if( !parseGetArg(cmd, "add_arrival", msg, "time", &time) ) {
	    msg.assign("add_arrival: invalid time argument");
	    return ARGUMENT_ERROR;
	}
	parseAddArrival(w, phase, time);
    }
    else if(parseFind(cmd, "set_working_orid", msg, &err, "orid", true)) {
	long orid;
	if(err) return ARGUMENT_ERROR;
	if(!parseGetArg(cmd, "set_working_orid", msg, "orid", &orid)) {
	    return ARGUMENT_ERROR;
	}
	setWorkingOrid(orid, true);
    }
    else {
	return TtPlotClass::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd WaveformPlot::parseOutput(Waveform *w, const string &cmd, string &msg)
{
    int i, n;
    string file, format, remark;
    bool append=true, raw=true, output_tables=true, err;
    double tmin = -1.e+60, tmax = 1.e+60;
    char wa[2];
    const char *c = "output";
    gvector<Waveform *> wvec, ws;

    if(!parseFind(cmd, "output", msg, &err, "_wave_", false,
	"file", true, "append", false, "raw", false, "output_tables", false,
	"format", false, "remark", false, "tmin", false, "tmax", false))
    {
	return COMMAND_NOT_FOUND;
    }
    else if(err) {
	return ARGUMENT_ERROR;
    }

    if( !parseGetArg(cmd, "file", file) ) {
	return ARGUMENT_ERROR;
    }
    parseGetArg(cmd, c, msg, "append", &append);
    if(!msg.empty()) return ARGUMENT_ERROR;

    parseGetArg(cmd, c, msg, "raw", &raw);
    if(!msg.empty()) return ARGUMENT_ERROR;

    parseGetArg(cmd, c, msg, "output_tables", &output_tables);
    if(!msg.empty()) return ARGUMENT_ERROR;

    if( parseGetArg(cmd, "format", format) ) {
	if(!parseCompare(format, "CSS") && !parseCompare(format, "SAC")
		&& !parseCompare(format, "ASCII"))
	{
	    msg.assign(string("output: invalid format: ") + format);
	    return ARGUMENT_ERROR;
	}
    }
    else {
	format.assign("css");
    }
    n = (int)file.length();
    if(n > 7 && !strcasecmp(file.c_str()+n-7, ".wfdisc")) {
	file.erase(n-7);
    }
    parseGetArg(cmd, "remark", remark);

    parseGetArg(cmd, c, msg, "tmin", &tmin);
    if(!msg.empty()) return ARGUMENT_ERROR;

    parseGetArg(cmd, c, msg, "tmax", &tmax);
    if(!msg.empty()) return ARGUMENT_ERROR;

    if(w) {
	ws.push_back(new Waveform(*w));
    }
    else if(getSelectedWaveforms(wvec) <= 0) {
	msg.assign("output: no waveforms selected.");
	return ARGUMENT_ERROR;
    }
    else {
	for(i = 0; i < wvec.size(); i++) {
	    ws.push_back(new Waveform(*wvec[i]));
	}
    }

    for(i = 0; i < ws.size(); i++) {
	if( (ws[i]->tbeg() < tmin && tmin < ws[i]->tend()) ||
	    (ws[i]->tbeg() < tmax && tmax < ws[i]->tend()))
	{
	    ws[i]->num_dw = 1;
	    ws[i]->dw = (GDataWindow *)malloc(sizeof(GDataWindow));
	    ws[i]->dw[0].d1 = ws[i]->nearest(tmin);
	    ws[i]->dw[0].d2 = ws[i]->nearest(tmax);
	}
    }
    wa[1] = '\0';
    wa[0] = (append) ? 'a' : 'w';

    if(parseCompare(format, "CSS")) {
	cssWriteCDC(file.c_str(), wa, ws, remark.c_str(), raw);
	if(output_tables) outputTables(file, wa);
    }
    else if(parseCompare(format, "SAC")) {
	CssOriginClass *origins[1];
	origins[0] = getPrimaryOrigin(ws[0]);

	sacWriteCDC(file.c_str(), wa, ws, remark.c_str(), raw, origins);
    }
    else {
        AscSource::output(file, wa, ws, remark, raw);
    }

    return COMMAND_PARSED;
}

void WaveformPlot::parseAddArrival(Waveform *w, const string &phase,
			double time)
{
    CssArrivalClass *arrival;
    double deltim = -1., delaz = -1., delslo = -1.;
    double azimuth = -1., slow = -1., ema = -1., rect = -1., snr = -1.;
    Password password = getpwuid(getuid());
    bool add_to_net = true;

    arrival = makeArrival(w, password, time, phase, deltim, azimuth,
                        delaz, slow, delslo, ema, rect, snr, add_to_net);
    if(arrival) {
        putArrivalWithColor(arrival, stringToPixel("black"));
    }
}

ParseCmd WaveformPlot::parseWaveformColor(const string &cmd, string &msg)
{
    bool err;
    string c, s;

    if(parseFind(cmd, "waveform_color", msg, &err,
		"code", false, "colors", false))
    {
	if(err) return ARGUMENT_ERROR;

	if(!waveform_color_window) {
	    waveform_color_window = new WaveformColor("Waveform Color",
					formDialogParent(), this);
	}
	waveform_color_window->setColor(getColorCode());
	if(parseGetArg(cmd, "code", c)) {
	    s.assign(string("select ") + c);
	    if(waveform_color_window->parseCmd(s, msg) != COMMAND_PARSED)
	    {
		msg.assign(string("waveform_color: unknow code: ") + c);
	    }
	}
	if(parseGetArg(cmd, "colors", c)) {
	    err = waveform_color_window->colorTable()->setColors(c, msg);
	    if(!err) return ARGUMENT_ERROR;
	}
	return COMMAND_PARSED;
    }
    else if(parseString(cmd, "waveform_color", c)) {
	if(!waveform_color_window) {
	    waveform_color_window = new WaveformColor("Waveform Color",
				formDialogParent(), this);
	}
	waveform_color_window->setColor(getColorCode());
	return waveform_color_window->parseCmd(c, msg);
    }
    return COMMAND_NOT_FOUND;
}

ParseCmd WaveformPlot::parseBeam(const string &cmd, string &msg)
{
    bool err, zp=false, use_filter=false, replace=false, found_slow=false;
    bool selected_only;
    int order=3, i, num;
    long addr;
    double slow, azimuth, low, high;
    string type, s;
    const char *command;
    ParseArg args[] =  {
	{"azimuth", true, false},	// 0
	{"slowdeg", false, false},	// 1
	{"slowkm", false, false},	// 2
	{"low", false, false},		// 3
	{"high", false, false},		// 4
	{"order", false, false},	// 5
	{"type", false, false},		// 6
	{"zp", false, false},		// 7
	{"replace", false, false},	// 8
	{"wlen", false, false},		// 9
	{"snr", false, false}		// 10
    };

    if(parseCompare(cmd, "beam ", 5)) {
	command = "beam";
    }
    else if(parseCompare(cmd, "ftrace ", 7)) {
	command = "ftrace";
    }
    else {
	return COMMAND_NOT_FOUND;
    }

    if(parseGetArg(cmd, command, msg, "_arrival_", &addr)) {
	replace = true;
	parseGetArg(cmd, command, msg, "replace", &replace);
	if(!msg.empty()) return ARGUMENT_ERROR;
	selected_only = false;
	parseGetArg(cmd, command, msg, "selected_only", &selected_only);
	if(!msg.empty()) return ARGUMENT_ERROR;
	cvector<CssArrivalClass> *arrivals = getArrivalRef();
	for(i = 0; i < arrivals->size() && (long)arrivals->at(i) != addr; i++);
	if(i < arrivals->size()) {
	    beamArrival(arrivals->at(i), selected_only, replace);
	    return COMMAND_PARSED;
	}
	msg.assign("arrival not found.");
	return ARGUMENT_ERROR;
    }

    num = (int)sizeof(args)/(int)sizeof(ParseArg);
    if(parseCheckArgs(cmd, command, num, args, msg, &err))
    {
	if(err) return ARGUMENT_ERROR;
	if(!create_beam_window) {
	    create_beam_window = new CreateBeam("Create Beam",
					formDialogParent(), this);
	}
	if(parseGetArg(cmd, command, msg, "slowkm", &slow) ) {
	    create_beam_window->setKm();
	    found_slow = true;
	}
	else if(!msg.empty()) {
	    return ARGUMENT_ERROR;
	}
	if(parseGetArg(cmd, command, msg, "slowdeg", &slow) ) {
	    if(found_slow) {
		msg.assign("beam: slowkm and slowdeg both entered");
		return ARGUMENT_ERROR;
	    }
	    create_beam_window->setDeg();
	    found_slow = true;
	}
	else if(!msg.empty()) {
	    return ARGUMENT_ERROR;
	}
	if(!found_slow) {
	    msg.assign("beam: neither slowkm nor slowdeg entered");
	    return ARGUMENT_ERROR;
	}

	create_beam_window->slownessText1()->setString("%.15g", slow);

	if( !parseGetArg(cmd, command, msg, "azimuth", &azimuth) ) {
	    return ARGUMENT_ERROR;
	}
        create_beam_window->azimuthText1()->setString("%.15g", azimuth);

	create_beam_window->setReplace(false);

	if(!args[6].found) { // type
	    if(args[3].found || args[4].found) {
		if(!args[6].found) type.assign("BP");
	    }
	    else if(args[5].found) { // order
		msg.assign("beam: order found without low or high");
		return ARGUMENT_ERROR;
	    }
	    else if(args[7].found) { // zp
		msg.assign("beam: zp found without low or high");
		return ARGUMENT_ERROR;
	    }
	}
	else {
	    parseGetArg(cmd, "type", type);
	}

	if(!type.empty())
	{
	    use_filter = true;
	    create_beam_window->setType(type);
	    if(parseCompare(type, "BP") || parseCompare(type, "BR")) {
		if(!parseGetArg(cmd, command, msg, "low", &low)) {
		    if(!msg.empty()) {
			msg.assign("beam: missing argument low");
		    }
		    return ARGUMENT_ERROR;
		}
		if(!parseGetArg(cmd, command, msg, "high", &high)) {
		    if(msg.empty()) {
			msg.assign("beam: missing argument high");
		    }
		    return ARGUMENT_ERROR;
		}
		create_beam_window->setLow(low);
		create_beam_window->setHigh(high);
	    }
	    else if(parseCompare(type, "LP")) {
		if(!parseGetArg(cmd, type, msg, "high", &high)) {
		    if(msg.empty()) {
			msg.assign("beam: missing argument high");
		    }
		    return ARGUMENT_ERROR;
		}
		create_beam_window->setHigh(high);
	    }
	    else if(parseCompare(type, "HP")) {
		if(!parseGetArg(cmd, command, msg, "low", &low)) {
		    if(msg.empty()) {
			msg.assign("beam: missing argument low");
		    }
		    return ARGUMENT_ERROR;
		}
		create_beam_window->setLow(low);
	    }
	    else if(parseCompare(type, "NA")) {
		use_filter = false;
	    }
	    else {
		msg.assign(string("unknown filter type: ") + type);
		return ARGUMENT_ERROR;
	    }
	    if(use_filter) {
		parseGetArg(cmd, command, msg, "order", &order);
		if(!msg.empty()) return ARGUMENT_ERROR;
		create_beam_window->setOrder(order);
		parseGetArg(cmd, command, msg, "zp", &zp);
		if(!msg.empty()) return ARGUMENT_ERROR;
		create_beam_window->setConstraint(zp);
	    }
	    parseGetArg(cmd, command, msg, "replace", &replace);
	    if(!msg.empty()) return ARGUMENT_ERROR;
	    create_beam_window->setReplace(replace);
	}
	else if(!msg.empty()) return ARGUMENT_ERROR;

	if(parseGetArg(cmd, "wlen", s)) {
	    create_beam_window->wlenText()->setString(s);
	}
	if(parseGetArg(cmd, "snr", s)) {
	    create_beam_window->snrText()->setString(s);
	}

	create_beam_window->useFilter(use_filter);
	create_beam_window->tab->setOnTop("Input Slowness");
	create_beam_window->makeBeam();

	return COMMAND_PARSED;
    }
/*
some detection beam recipes have az,slow. Perhaps don't make azimuth and
slowness required in parseFind(), and check that must have either
az,slow or a recipe with non-null az,slow. If input az,slow and recipe,
the input az,slow overrides the recipe az,slow.
*/
    return COMMAND_NOT_FOUND;
}

ParseVar WaveformPlot::parseVar(const string &name, string &value)
{
    string op, s, phase, net, sta;
    ostringstream os;
    double d, olat, olon, odepth, lat, lon, elev, delta, az, baz, slow;
    double stav_len, ltav_len;
    long addr;
    int id, n;
    CssOriginClass *o=NULL;

    if(parseCompare(name, "travelTime_ ", 12)) {
	if(!parseFuncArg(name, 1, "phase", phase, value)) {
	    return VARIABLE_ERROR;
	}
	if(parseGetArg(name, "arg2", s) && parseCompare(s,"_origin_=",9)
		&& stringToLong(s.c_str()+9, &addr))
	{
	    o = (CssOriginClass *)addr;
	    n = 3;
	}
	else {
	    if(!parseFuncArg(name, 2, "olat", &olat, value)) {
		return VARIABLE_ERROR;
	    }
	    if(!parseFuncArg(name, 3, "olon", &olon, value)) {
		return VARIABLE_ERROR;
	    }
	    if(!parseFuncArg(name, 4, "odepth", &odepth, value)) {
		return VARIABLE_ERROR;
	    }
	    n = 5;
	}
	os << "arg" << n;
	if(parseGetArg(name, os.str(), s) && parseCompare(s, "_wave_=", 7)
		&& stringToInt(s.c_str()+7, &id))
	{
	    Waveform *w = getWaveform(id);
	    if(!w) {
	        value.assign("travelTime: cannot find wave object.");
		return VARIABLE_ERROR;
	    }
	    lat = w->lat();
	    lon = w->lon();
	    elev = w->elev();
	    sta.assign(w->sta());
	    net.assign(w->net());
	}
	else {
	    if(!parseFuncArg(name, n, "lat", &lat, value)) {
		return VARIABLE_ERROR;
	    }
	    if(!parseFuncArg(name, n+1, "lon", &lon, value)) {
		return VARIABLE_ERROR;
	    }
	    if(!parseFuncArg(name, n+2, "elev", &elev, value)) {
		return VARIABLE_ERROR;
	    }
	    if(!parseFuncArg(name, n+3, "net", net, value)) {
		return VARIABLE_ERROR;
	    }
	    if(!parseFuncArg(name, n+4, "sta", sta, value)) {
		return VARIABLE_ERROR;
	    }
	}
	if(!o) {
	    o = new CssOriginClass();
	    o->lat = olat;
	    o->lon = olon;
	    o->depth = odepth;
	}
	d = getTravelTime(phase, o, lat, lon, elev, net, sta, &slow, op);
	o->deleteObject();
	parsePrintDouble(value, d);
	Application::putParseProperty("tt_phase", op);
	parsePrintDouble(s, slow);
	Application::putParseProperty("tt_slowness", s);

	return STRING_RETURNED;
    }
    else if(parseCompare(name, "distance_ ", 10) ||
		parseCompare(name, "azimuth_ ", 9) ||
		parseCompare(name, "backAzimuth_ ", 13))
    {
	if(!parseFuncArg(name, 1, "olat", &olat, value)) {
	    return VARIABLE_ERROR;
	}
	if(!parseFuncArg(name, 2, "olon", &olon, value)) {
	    return VARIABLE_ERROR;
	}
	if(!parseFuncArg(name, 3, "lat", &lat, value)) {
	    return VARIABLE_ERROR;
	}
	if(!parseFuncArg(name, 4, "lon", &lon, value)) {
	    return VARIABLE_ERROR;
	}
	deltaz(olat, olon, lat, lon, &delta, &az, &baz);
	if(parseCompare(name, "distance_ ", 10)) {
	    parsePrintDouble(value, delta);
	}
	else if(parseCompare(name, "azimuth_ ", 9)) {
	    parsePrintDouble(value, az);
	}
	else {
	    parsePrintDouble(value, baz);
	}
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "geocentric_ ", 12)) {
	if(!parseFuncArg(name, 1, "lat", &lat, value)) {
	    return VARIABLE_ERROR;
	}
	d = geocentric(lat);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "geographic_ ", 12)) {
	if(!parseFuncArg(name, 1, "lat", &lat, value)) {
	    return VARIABLE_ERROR;
	}
	d = geographic(lat);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "stalta_ ", 8)) {
	if(!parseFuncArg(name, 1, "ltav_len", &ltav_len, value)) {
	    return VARIABLE_ERROR;
	}
	if(!parseFuncArg(name, 2, "stav_len", &stav_len, value)) {
	    return VARIABLE_ERROR;
	}
    }
    else if(parseString(name, "filter", s))
    {
	if(!bfilter_window) {
	    bfilter_window = new Filter("Butterworth Filter",
				formDialogParent(), this);
	}
	return bfilter_window->parseVar(s, value);
    }
    else if(parseString(name, "print_window", s)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print Waveforms",formDialogParent(),
					this);
	}
	return print_window->parseVar(s, value);
    }
    else if(parseString(name, "output_window", s)) {
	if(!output_window) {
	    output_window = new Output("Save as", formDialogParent(), this);
	}
	return output_window->parseVar(s, value);
    }
    else if(parseString(name, "data_values", s)) {
	if(!data_values_window) {
	    data_values_window = new DataValues("Data Values",
					formDialogParent(), this);
	}
        return data_values_window->parseVar(s, value);
    }
    else if(parseString(name, "axes_labels", s)) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Axes Labels", formDialogParent(),
					this);
	}
	return labels_window->parseVar(s, value);
    }
    else if(parseString(name, "method_history", s)) {
	if(!history_window) {
	    history_window = new History("Method History", formDialogParent(),
					this);
	}
	return history_window->parseVar(s, value);
    }
    else if(parseString(name, "loaded_tables", s)) {
	if(!loaded_tables_window) {
	    loaded_tables_window = new LoadedTableList("Loaded Tables",
				formDialogParent(), this, "Loaded Tables");
	    loaded_tables_window->displayAllTables(this);
	}
	return loaded_tables_window->parseVar(s, value);
    }
    else if(parseCompare(name, "gap_limit")) {
	parsePrintDouble(value, cons.join_time_limit);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "overlap_limit")) {
	parsePrintDouble(value, cons.overlap_limit);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "segment_length")) {
	parsePrintDouble(value, cons.window_length);
	return STRING_RETURNED;
    }

    return TtPlotClass::parseVar(name, value);
}


void WaveformPlot::parseHelp(const char *prefix)
{
    makePopup();
    makePopupMenu();

    printf("\n");
    printf("%sadd_arrival wave[i] phase=PHASE time=TIME\n", prefix);
    printf("%salign_on_phase phase=PHASE\n", prefix);
    printf("%salign_on_predicted_phase phase=PHASE\n", prefix);
    printf("%salign_first_point\n", prefix);
    printf("%salign_true_time\n", prefix);
    printf("%salign_predicted_firstP\n", prefix);
    printf("%salign_observed_firstP\n", prefix);

    printf("%sadd_time_line\n", prefix);
    printf("%sadd_time_limits\n", prefix);
    printf("%sadd_crosshair\n", prefix);
    printf("%sauto_scale_on\n", prefix);
    printf("%sauto_scale_off\n", prefix);

    printf("%sbeam azimuth=DEGREES slowkm=SECS/KM or slowdeg=SECS/DEG [low=FREQ] \
[high=FREQ] [type=TYPE] [order=ORDER] [zp=(true,false)] [replace=(true,false)]\n", prefix);
    printf("%sbeam arrival[i] [selected_only=(true,false)] [replace=(true,false)]", prefix);

    printf("%sclear\n", prefix);
    printf("%scomponents_all\n", prefix);
    printf("%scomponents_z_only\n", prefix);
    printf("%scomponents_n_and_e\n", prefix);
    printf("%scomponents_n_only\n", prefix);
    printf("%scomponents_e_only\n\n", prefix);

    printf("%sconvolve [ wave[i] ] [ instrument[j] ] [inid=INID] [low=LOW] [high=HIGH] \
[data_taper=SECS] [remove_time_shift=(true,false)]\n", prefix);
    printf("%scopy [ wave[i] ] [tmin=TMIN tmax=TMAX]\n", prefix);
    printf("%scut [ wave[i] ] [tmin=TMIN tmax=TMAX]\n", prefix);
    printf("%sdeconvolve [ wave[i] ] [ instrument[j] ] [inid=INID] [low=LOW] [high=HIGH] \
[data_taper=SECS] [remove_time_shift=(true,false)] [amp_cutoff=CUTOFF]\n", prefix);

    printf("\n");
    printf("%smovement_none\n", prefix);
    printf("%smovement_xy\n", prefix);
    printf("%smovement_x\n", prefix);
    printf("%smovement_y\n", prefix);

    printf("%sdelete wave[i]\n", prefix);
    printf("%sdelete_data\n", prefix);
    printf("%sdelete_all_cursors\n", prefix);
    printf("%sdelete_crosshair\n", prefix);
    printf("%sdelete_line\n", prefix);
    printf("%sdelete_phase_line\n", prefix);
    printf("%sdelete_time_window [label=LABEL]\n\n", prefix);

    printf("%sdeselect origin[i]\n", prefix);
    printf("%sdeselect wave[i]\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%sdeselect_all_arrivals\n", prefix);
    printf("%sdeselect_arrival [phase=PHASE] [sta=STA] [chan=CHAN] [arid=ARID] [orid=ORID]\n\n", prefix);
    printf("%sdeselect NUM\n", prefix);
    printf("%sdeselect STA/CHAN\n", prefix);
    printf("%sdeselect [sta=STA] [chan=CHAN] [wfid=WFID] [orid=ORID]\n",prefix);
    printf("%sarrivals_off\n", prefix);
    printf("%sarrivals_on_one_channel\n", prefix);
    printf("%sarrivals_on_all_channels\n", prefix);

    printf("%sdisplay_all_data\n", prefix);
    printf("%sdisplay_selected_only\n", prefix);
    printf("%sdisplay_amplitude_scale\n", prefix);
    printf("%shide_amplitude_scale\n", prefix);
    printf("%sdisplay_calib [ wave[i] ]\n", prefix);
    printf("%sdisplay_counts [ wave[i] ]\n", prefix);
    printf("%sdisplay_tags\n", prefix);
    printf("%shide_tags\n", prefix);
    printf("%sdisplay_waveform wave[i]\n\n", prefix);

    printf("%spartial_select\n", prefix);
    printf("%stotal_select\n", prefix);
    printf("%snormal_polarity\n", prefix);
    printf("%sreverse_polarity\n", prefix);
    printf("%sposition_crosshair x=XCOORD y=YCOORD\n", prefix);
    printf("%sposition_line x=XCOORD [label=LABEL] [notify=(true,false)]\n", prefix);
    printf("%sposition_phase_line x=XCOORD phase=PHASE [notify=(true,false)]\n", prefix);
    printf("%sposition_time_window xmin=XMIN xmax=XMAX [label=LABEL] [notify=(true,false)]\n", prefix);
    printf("%spromote_selected_waveforms\n\n", prefix);

    printf("%sremove_all_cursors\n", prefix);
    printf("%sremove_mean\n", prefix);
    printf("%srestore_mean\n\n", prefix);

    printf("%sscale_up\n", prefix);
    printf("%sscale_down\n", prefix);
    printf("%sselect_all\n", prefix);
    printf("%sselect_visible\n", prefix);
    printf("%sselect_visible_z\n", prefix);
    printf("%sselect_visible_horz\n", prefix);
    printf("%sselect NUM\n", prefix);
    printf("%sselect STA/CHAN\n", prefix);
    printf("%sselect [sta=STA] [chan=CHAN] [wfid=WFID] [orid=ORID]\n", prefix);
    printf("%sset_default_order\n", prefix);
    printf("%sselect_all_arrivals\n", prefix);
    printf("%sselect_arrival [phase=PHASE] [sta=STA] [chan=CHAN] [arid=ARID] [orid=ORID]\n", prefix);

    printf("%ssort_waveforms order=\
(\"default_order\",\"file_order\",\"distance\",\"time/sta/chan\",\"sta/chan\",\"chan/sta\",\"back_azimuth\")\n", prefix);

    printf("%sspace_more\n", prefix);
    printf("%sspace_less\n\n", prefix);

    printf("%stime_window beg=TIME (end=TIME,duration=SECONDS) [letter=LETTER]\n", prefix);
    printf("%stime_window phase=PHASE [lead=TIME] [lag=TIME] [letter=LETTER]\n",
                prefix);
    printf("%stime_zoom beg=TIME (end=TIME,duration=SECONDS)\n", prefix);
    printf("%stime_zoom phase=PHASE [lead=TIME] [lag=TIME]\n", prefix);

    printf("%stime_scale_seconds\n", prefix);
    printf("%stime_scale_variable\n", prefix);
    printf("%stime_scale_hms\n", prefix);
    printf("%stime_scale_epoch\n", prefix);

    printf("%sfilter [ wave[i] ] low=LOW high=HIGH [type=TYPE] [order=ORDER] [zp=ZP]\n", prefix);
    printf("%sunfilter_all\n", prefix);
    printf("%soutput wave[i] file=FILE [tmin=TMIN] [tmax=TMAX] [append=(true,false)] \
[raw=(true,false)] [format=(css,sac,ascii)] [output_tables=(true,false)] [remark=REMARK]\n",
	prefix);

    printf("%suniform_scale\n", prefix);
    printf("%sindependent_scale\n", prefix);
    printf("%sunzoom\n", prefix);
    printf("%sunzoom_all\n", prefix);
    printf("%sunzoom_horizontal_all\n", prefix);
    printf("%sunzoom_vertical_all\n", prefix);
    printf("%szoom xmin=XMIN xmax=XMAX ymin=YMIN ymax=YMAX\n", prefix);
    printf("%szoom_on_waveform\n\n", prefix);

    printf("Attributes:\n");
    printf("crosshair_x, crosshair_y, crosshair_label, line_label, line_time,\n");
    printf("num_crosshairs, num_lines, num_time_windows,\n");
    printf("plot_tmin, plot_tmax, plot_xmin, plot_xmax, plot_ymin, plot_ymax\n");
    printf("time_window_a_min, time_window_a_max, time_window_a_width,\n");
    printf("time_window_b_min, time_window_b_max, time_window_b_width,\n");
    printf("time_window_c_min, time_window_c_max, time_window_c_width\n\n");

    printf("Attributes with wave and sel_wave:\n");
    printf("w.sta, w.chan, w.net, w.refsta, w.xchan, w.ychan, w.zchan, w.instype,\n");
    printf("w.segtype, w.datatype, w.clip, w.chanid, w.lat, w.lon, w.elev, w.dnorth,\n");
    printf("w.deast, w.hang, w.vang, w.jdate, w.tbeg, w.tend, w.dataMin, w.dataMax,\n");
    printf("w.mean, w.duration, w.nsegs, w.length, w.samprate, w.calib, w.calper, w.index\n\n");

    printf("%samplitude_scale.help\n", prefix);
    printf("%saxes_labels.help\n", prefix);
    printf("%sbutterworth_filter.help\n", prefix);
    printf("%screate_beam.help\n", prefix);
    printf("%sdata_values.help\n", prefix);
    printf("%sprint.help\n", prefix);
    printf("%stag_contents.help\n", prefix);
    printf("%swaveform_color.help\n", prefix);
    printf("%swaveform_setup.help\n\n", prefix);
}

ParseCmd WaveformPlot::parseTimeWindow(const string &cmd, string &msg)
{
    double tbeg, tend, tduration;
    double lead=1., lag=4.;
    ostringstream os;
    string c, phase;
    bool err, time_window=false;
    int i;
    long arid;

    if(parseFind(cmd, "time_window", msg, &err,
	"arid", false, "lead", false, "lag", false, "phase", false,
	"tbeg", false, "tend", false, "duration", false, "letter", false))
    {
	if(err) return ARGUMENT_ERROR;
	time_window = true;
	c.assign("time_window");
    }
    else if(parseFind(cmd, "time_zoom", msg, &err,
	"arid", false, "lead", false, "lag", false, "phase", false,
	"tbeg", false, "tend", false, "duration", false))
    {
	if(err) return ARGUMENT_ERROR;
	time_window = false;
	c.assign("time_zoom");
    }
    else {
	return COMMAND_NOT_FOUND;
    }

    if( parseGetArg(cmd, c, msg, "arid", &arid) ) {
	parseGetArg(cmd, c, msg, "lead", &lead);
	if(!msg.empty()) return ARGUMENT_ERROR;
	parseGetArg(cmd, c, msg, "lag", &lag);
	if(!msg.empty()) return ARGUMENT_ERROR;
	cvector<CssArrivalClass> *a = getArrivalRef();
	for(i = 0; i < a->size() && a->at(i)->arid != arid; i++);
	if(i == a->size()) {
	    os << "arid " << arid << " not found.";
	    msg.assign(os.str());
	    return ARGUMENT_ERROR;
	}
	tbeg = a->at(i)->time - lead;
	tend = a->at(i)->time + lag;
    }
    else if(!msg.empty()) {
	return ARGUMENT_ERROR; // could not read arid value
    }
    else if( parseGetArg(cmd, "phase", phase) ) {
	parseGetArg(cmd, c, msg, "lead", &lead);
	if(!msg.empty()) return ARGUMENT_ERROR;
	parseGetArg(cmd, c, msg, "lag", &lag);
	if(!msg.empty()) return ARGUMENT_ERROR;
	cvector<CssArrivalClass> *a = getArrivalRef();
	for(i = 0; i < a->size() && !parseCompare(phase, a->at(i)->phase); i++);
	if(i == a->size()) {
	    msg.assign(string("phase ") + phase + " not found.");
	    return ARGUMENT_ERROR;
	}
	tbeg = a->at(i)->time - lead;
	tend = a->at(i)->time + lag;
    }
    else
    {
	if(!parseGetArg(cmd, c, msg, "tbeg", &tbeg)) {
	    if(msg.empty()) {
		msg.assign(c + ": missing argument 'tbeg'");
	    }
	    return ARGUMENT_ERROR;
	}
	if(!parseGetArg(cmd, c, msg, "tend", &tend)) {
	    if(!msg.empty()) return ARGUMENT_ERROR;
	    if(!parseGetArg(cmd, c, msg, "duration", &tduration)) {
		if(msg.empty()) {
		    msg.assign(c + ": missing argument 'tbeg'");
		}
		return ARGUMENT_ERROR;
	    }
	    tend = tbeg + tduration;
	}
    }

    if(time_window) {
	string letter;
	if(!parseGetArg(cmd, "letter", letter)) letter.assign("a");
	positionDoubleLine(letter, tbeg, tend, true);
    }
    else {
	double ymin, ymax;
	getYLimits(&ymin, &ymax);
	zoom(tbeg, tend, ymin, ymax, false);
//	setTimeLimits(tbeg, tend);
    }
    return COMMAND_PARSED;
}

ParseCmd WaveformPlot::parseSelect(const string &cmd, string &msg)
{
    const char *b, *e;
    long orid, wfid, w_orid = -1;
    bool select, use_orid=false, use_wfid=false, wfid_match=false, err;
    gvector<Waveform *> wvec;
    ostringstream os;
    string c, sta, chan, s;
    int i, id;
    long addr;
    cvector<CssWfdiscClass> *v;

    if(parseCompare(cmd, "select", 6)) {
	select = true;
        c.assign("select");
    }
    else if(parseCompare(cmd, "deselect", 8)) {
        select = false;
        c.assign("deselect");
    }
    else {
        return COMMAND_NOT_FOUND;
    }
    if(parseCompare(cmd, "select") || parseCompare(cmd, "deselect")) {
	msg.assign(c + ": missing argument");
	return ARGUMENT_ERROR;
    }

    if(parseFind(cmd, c, msg, &err, "_arrival_", true) && !err)
    {
	if(!parseGetArg(cmd, c, msg, "_arrival_", &addr)) {
	    return ARGUMENT_ERROR;
	}
	cvector<CssArrivalClass> *arrivals = getArrivalRef();
	for(i = 0; i < arrivals->size() && (long)arrivals->at(i) != addr; i++);
	if(i < arrivals->size()) {
	    selectArrivalWithCB(arrivals->at(i), select);
	    return COMMAND_PARSED;
	}
	msg.assign(c + ": cannot find arrival object");
	return ARGUMENT_ERROR;
    }
    msg.clear();

    if(parseFind(cmd, c, msg, &err, "_wave_", true) && !err)
    {
	if(!parseGetArg(cmd, c, msg, "_wave_", &id)) {
	    return ARGUMENT_ERROR;
	}
	Waveform *w;
	if(!(w = getWaveform(id))) {
	    msg.assign(c + ": cannot find wave object");
	    return ARGUMENT_ERROR;
	}
	selectWaveform(w, select, true);
	return COMMAND_PARSED;
    }
    msg.clear();

    if(parseFind(cmd, c, msg, &err, "_origin_", true) && !err)
    {
	if(!parseGetArg(cmd, c, msg, "_origin_", &addr)) {
	    return ARGUMENT_ERROR;
	}
	selectOrigin((CssOriginClass *)addr, select);
	return COMMAND_PARSED;
    }
    msg.clear();

    b = cmd.c_str() + c.length();
    while(*b != '\0' && isspace((int)*b)) b++;
    e = b;
    s.clear();

    while(*e != '\0' && !isspace((int)*e)) s.append(1, *e++);

    if(stringToInt(s.c_str(), &i)) { // look for "select num"
	if(*e != '\0') {
	    while(*e != '\0' && isspace((int)*e)) e++;
	    if(*e != '\0') {
		msg.assign(c + ": unexpected argument(s): " + e);
		return ARGUMENT_ERROR;
	    }
	}
	getWaveforms(wvec);
	if(i > 0 && i <= wvec.size()) {
	    selectWaveform(wvec[i-1], select, true);
	    return COMMAND_PARSED;
	}
	os << c << ": invalid waveform index: " << i;
	msg.assign(os.str());
	return ARGUMENT_ERROR;
    }

    use_orid = parseGetArg(cmd, c, msg, "orid", &orid) ? true : false;
    if(!msg.empty()) return ARGUMENT_ERROR;
    use_wfid = parseGetArg(cmd, c, msg, "wfid", &wfid) ? true : false;
    if(!msg.empty()) return ARGUMENT_ERROR;

    parseGetArg(cmd, "sta", sta);
    parseGetArg(cmd, "chan", chan);

    if(sta.empty() && chan.empty() && !use_orid && !use_wfid) {
	msg.assign(
		"invalid select [#] [sta/chan] [sta=] [chan=] [wfid=] [orid=]");
	return ARGUMENT_ERROR;
    }

    getWaveforms(wvec);

    for(i = 0; i < wvec.size(); i++) {
	if(use_orid) {
	    CssOriginClass *o = getPrimaryOrigin(wvec[i]);
	    w_orid = (o) ? o->orid : -1;
	}
	if(use_wfid) {
	    wfid_match = false;
	    if((v = wvec[i]->ts->getWfdiscs())) {
		for(int j = 0; j < v->size(); j++) {
		    if(wfid == v->at(j)->wfid) {
			wfid_match = true;
			break;
		    }
		}
		delete v;
	    }
	}
	if((sta.empty() || parseCompare(sta, wvec[i]->sta())) &&
	    (chan.empty() || DataSource::compareChan(chan, wvec[i]->chan())) &&
	    (!use_orid || orid == w_orid) && (!use_wfid || wfid_match) )
	{
	    selectWaveform(wvec[i], select, true);
	}
    }
    return COMMAND_PARSED;
}

ParseCmd WaveformPlot::parseSelectArrivals(const string &cmd, string &msg)
{
    string c, sta, chan, phase;
    int i;
    long orid, arid, addr;
    bool select, use_orid = false, use_arid = false, err;

    if(parseCompare(cmd, "select_arrival", 14)) {
	select = true;
	c.assign("select_arrival");
    }
    else if(parseCompare(cmd, "deselect_arrival", 16)) {
	select = false;
	c.assign("deselect_arrival");
    }
    else {
	return COMMAND_NOT_FOUND;
    }

    if(parseFind(cmd, c, msg, &err, "_arrival_", true) &&
	parseGetArg(cmd, c, msg, "_arrival_", &addr))
    {
	cvector<CssArrivalClass> *arrivals = getArrivalRef();
	for(i = 0; i < arrivals->size() && (long)arrivals->at(i) != addr; i++);
	if(i < arrivals->size()) {
	    selectArrivalWithCB(arrivals->at(i), select);
	    return COMMAND_PARSED;
	}
	msg.assign(c + ": cannot find arrival object");
	return ARGUMENT_ERROR;
    }
    msg.clear();

    if(!parseFind(cmd, c, msg, &err, "arid", false, "orid", false,
		"sta", false, "chan", false, "phase", false))
    {
	return COMMAND_NOT_FOUND;
    }
    else if(err) {
	return ARGUMENT_ERROR;
    }

    use_arid = parseGetArg(cmd, c, msg, "arid", &arid) ? true : false;
    if(msg[0] != '\0') return ARGUMENT_ERROR;
    use_orid = parseGetArg(cmd, c, msg, "orid", &orid) ? true : false;
    if(msg[0] != '\0') return ARGUMENT_ERROR;

    parseGetArg(cmd, "sta", sta);
    parseGetArg(cmd, "chan", chan);
    parseGetArg(cmd, "phase", phase);

    if(sta.empty() && chan.empty() && phase.empty() && !use_arid && !use_orid) {
	msg.assign(
	    "invalid select_arrival [phase=] [sta=] [chan=] [arid=] [orid=]");
	return ARGUMENT_ERROR;
    }

    cvector<CssArrivalClass> *arrivals = getArrivalRef();
    cvector<CssAssocClass> *assocs = getAssocRef();
   
    for(i = 0; i < arrivals->size(); i++)
    {
	if(use_arid && arrivals->at(i)->arid != arid) continue;

	if(use_orid) {
	    bool orid_match = false;
	    for(int j = 0; j < assocs->size(); j++) {
		if(assocs->at(j)->arid == arrivals->at(i)->arid
			&& assocs->at(j)->orid == orid)
		{
		    orid_match = true;
		    break;
		}
	    }
	    if(!orid_match) continue;
	}
	char *p = (arrivals->at(i)->phase[0] != '\0' &&
		strcmp(arrivals->at(i)->phase, "-")) ? arrivals->at(i)->phase :
		arrivals->at(i)->iphase;

	if( (phase.empty() || parseCompare(phase, p)) &&
	    (sta.empty() || parseCompare(sta, arrivals->at(i)->sta)) &&
	    (chan.empty() || DataSource::compareChan(chan,
			arrivals->at(i)->chan)) )
	{
	    selectArrivalWithCB(arrivals->at(i), select);
	}
    }

    return COMMAND_PARSED;
}

void WaveformPlot::menuPopup(XEvent *event)
{
    makePopup();
    makePopupMenu();
    setToggleStates(&wt);
    menu_popup->position((XButtonPressedEvent *)event);
    menu_popup->setVisible(true);
}

void WaveformPlot::setToggleStates(WaveformPlotToggles *t)
{
    bool b;
    Arg args[1];

    XtSetArg(args[0], XtNallowPartialSelect, &b);
    getValues(args, 1);
    t->partial_select_toggle->set(b, false);
    XtSetArg(args[0], XtNdisplayAmplitudeScale, &b);
    getValues(args, 1);
    t->display_amp_scale->set(b, false);

    t->align_first_point->set(time_align == ALIGN_FIRST_POINT, false);
    t->align_true_time->set(time_align == ALIGN_TRUE_TIME, false);
    t->align_origin->set(time_align == ALIGN_ORIGIN, false);
    t->align_predicted->set(time_align == ALIGN_PREDICTED_ARRIVAL, false);
    t->align_observed->set(time_align == ALIGN_OBSERVED_ARRIVAL, false);

    int mode = getComponentDisplayMode();
    t->all_components->set(mode == CPLOT_ALL_COMPONENTS, false);
    t->z_only->set(mode == CPLOT_Z_COMPONENT, false);
    t->n_only->set(mode == CPLOT_N_COMPONENT, false);
    t->e_only->set(mode == CPLOT_E_COMPONENT, false);

    XtSetArg(args[0], XtNdataMovement, &mode);
    getValues(args, 1);
    t->no_movement->set(mode == CPLOT_NO_MOVEMENT, false);
    t->xy_movement->set(mode == CPLOT_XY_MOVEMENT, false);
    t->x_movement->set(mode == CPLOT_X_MOVEMENT, false);
    t->y_movement->set(mode == CPLOT_Y_MOVEMENT, false);

    XtSetArg(args[0], XtNdisplayArrivals, &mode);
    getValues(args, 1);
    t->arrivals_off->set(mode == CPLOT_ARRIVALS_OFF, false);
    t->on_one_channel->set(mode == CPLOT_ARRIVALS_ONE_CHAN, false);
    t->on_all_channels->set(mode == CPLOT_ARRIVALS_ALL_CHAN, false);

    t->default_order->set(sort_type == SORT_DEFAULT_ORDER, false);
    t->file_order->set(sort_type == SORT_FILE_ORDER, false);
    t->distance->set(sort_type == SORT_DISTANCE, false);
    t->distance_from->set(sort_type == SORT_DISTANCE_FROM, false);
    t->time_sta_chan->set(sort_type == SORT_TIME_STA_CHAN, false);
    t->sta_chan->set(sort_type == SORT_STA_CHAN, false);
    t->chan_sta->set(sort_type == SORT_CHAN2_STA, false);
    t->back_azimuth->set(sort_type == SORT_BACK_AZIMUTH, false);

    XtSetArg(args[0], XtNdisplayTags, &b);
    getValues(args, 1);
    t->display_tags->set(b, false);

    XtSetArg(args[0], XtNtimeScale, &mode);
    getValues(args, 1);
    t->time_scale_seconds->set(mode == TIME_SCALE_SECONDS && !time_scale_epoch,
		false);
    t->time_scale_variable->set(mode == TIME_SCALE_VARIABLE, false);
    t->time_scale_hms->set(mode == TIME_SCALE_HMS, false);
    t->time_scale_epoch_toggle->set(mode == TIME_SCALE_SECONDS &&
				time_scale_epoch, false);

    t->uniform_scale->set(getUniformScale(), false);
    t->auto_scale->set(auto_vertical_scale, false);
}

void WaveformPlot::makePopup(void)
{
    if(menu_popup) return;

    Arg args[2];
    /* the default is button3, which will inactivate button3
     * in the AxesWidget, so set to 6, which is > all buttons
     */
    int n = 0;
#if(XmVersion >= 2)
#ifndef XmNtearOffTitle
#define XmNtearOffTitle "tearOffTitle"
#endif
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
#endif
    XtSetArg(args[n], XmNwhichButton, 6); n++;
    menu_popup = new PopupMenu("menuPopup", this, args, n);
}

void WaveformPlot::makePopupMenu(void)
{
    if(menu_made) return;

    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";
    XtTranslations translations = XtParseTranslationTable(trans);
    Arg args[1];

    XtSetArg(args[0], XmNtranslations, translations);

    clear_button = new Button("Clear", menu_popup, args, 1, this);
    delete_data_button = new Button("Delete Data", menu_popup, args, 1, this);
    Menu *m = new Menu("Partial Select", menu_popup, false);
    wt.partial_select_toggle = new Toggle("Partial Select", m, this);
    new Button("Arrival Parameters...", menu_popup, args, 1, this);

    align_menu = new Menu("Align", menu_popup, true);
    wt.align_first_point = new Toggle("On First Point", align_menu, this, true);
    wt.align_first_point->set(false, false);
    wt.align_true_time = new Toggle("On True Time", align_menu, this, true);
    wt.align_true_time->set(true, false);
    wt.align_origin = new Toggle("Time Minus Origin", align_menu, this, true);
    wt.align_predicted = new Toggle("Time Minus Predicted FirstP", align_menu,
                                this, true);
    wt.align_observed = new Toggle("Time Minus Observed FirstP", align_menu,
                                this, true);

    components_menu = new Menu("Components", menu_popup, true);
    wt.all_components = new Toggle("All Components", components_menu,this,true);
    wt.all_components->set(true);
    wt.z_only = new Toggle("Z Only", components_menu, this, true);
    wt.n_and_e_only = new Toggle("N and E Only", components_menu, this, true);
    wt.n_only = new Toggle("N Only", components_menu, this, true);
    wt.e_only = new Toggle("E Only", components_menu, this, true);

    cursors_menu = new Menu("Cursors", menu_popup, false);
    add_time_line = new Button("Add Time Line", "L", cursors_menu, this);
    add_time_limits = new Button("Add Time Limits", "l", cursors_menu, this);
    add_crosshair = new Button("Add Crosshair", "c", cursors_menu, this);
    remove_all_cursors = new Button("Remove All Cursors","D",cursors_menu,this);

    data_movement_menu = new Menu("Data Movement", menu_popup, true);
    wt.no_movement = new Toggle("no movement", data_movement_menu, this);
    wt.no_movement->set(true);
    wt.xy_movement = new Toggle("xy movement", data_movement_menu, this);
    wt.x_movement = new Toggle("x movement", data_movement_menu, this);
    wt.y_movement = new Toggle("y movement", data_movement_menu, this);
    data_values_button = new Button("Data Values...", menu_popup, args, 1,this);

    m = new Menu("Display Amplitude Scale", menu_popup, false);
    wt.display_amp_scale = new Toggle("Display Amplitude Scale", m, this);
    wt.display_amp_scale->set(true);

    display_arrivals_menu = new Menu("Display Arrivals", menu_popup, true);
    wt.arrivals_off = new Toggle("Off", display_arrivals_menu, this);
    wt.on_one_channel = new Toggle("On One Channel",display_arrivals_menu,this);
    wt.on_all_channels = new Toggle("On All Channels", display_arrivals_menu,
				this);
    wt.on_all_channels->set(true);

    display_data_menu = new Menu("Display Data", menu_popup, true);
    all_data  = new Button("All Data", display_data_menu, this);
    selected_data  = new Button("Selected Only", display_data_menu, this);

    new Button("Filter...", menu_popup, args, 1, this);

    labels_button = new Button("Labels...", menu_popup, args, 1, this);

    magnify_button = new Button("Magnify Window...", menu_popup, args, 1, this);
    output_button = new Button("Save as...", menu_popup, args, 1, this);
    promote_selected = new Button("Promote Selected Waveforms", menu_popup,
				args, 1, this);
    scale_menu = new Menu("Scale", menu_popup, false);
    space_more  = new Button("Space More", scale_menu, this);
    space_less  = new Button("Space Less", scale_menu, this);
    scale_up  = new Button("Scale Up", scale_menu, this);
    scale_down  = new Button("Scale Down", scale_menu, this);
    set_scale  = new Button("Set Scale...", scale_menu, this);
    wt.uniform_scale = new Toggle("Uniform Scale", scale_menu, this);
    wt.auto_scale = new Toggle("Auto Scale", scale_menu, this);

    select_menu = new Menu("Select", menu_popup, false);
    select_all = new Button("Select All", select_menu, this);
    deselect_all = new Button("Deselect All", select_menu, this);
    select_visible = new Button("Select Visible", select_menu, this);
    select_visible_z = new Button("Select Visible Z", select_menu, this);
    select_visible_horz = new Button("Select Visible Horz", select_menu, this);

    set_default_order = new Button("Set Default Order", menu_popup, args, 1,
				this);

    sort_menu = new Menu("Sort", menu_popup, true);
    wt.default_order = new Toggle("Default Order", sort_menu, this);
    wt.default_order->set(true);
    wt.file_order = new Toggle("File Order", sort_menu, this);
    wt.distance = new Toggle("Distance", sort_menu, this);
    wt.distance_from = new Toggle("Distance from", sort_menu, this);
    wt.distance_from->setVisible(false);
    wt.time_sta_chan = new Toggle("Time/Sta/Chan", sort_menu, this);
    wt.sta_chan = new Toggle("Sta/Chan", sort_menu, this);
    wt.chan_sta = new Toggle("Chan/Sta", sort_menu, this);
    wt.back_azimuth = new Toggle("Back Azimuth", sort_menu, this);

    scroll_button = new Button("Scroll...", menu_popup, args, 1, this);

    tags_menu = new Menu("Tags", menu_popup, false);
    wt.display_tags = new Toggle("Display Tags", tags_menu, this);
    wt.display_tags->set(true, false);
    new Button("Tag Contents...", tags_menu, this);

    time_scale_menu = new Menu("Time Scale", menu_popup, true);
    wt.time_scale_seconds = new Toggle("Seconds", time_scale_menu, this, true);
    wt.time_scale_seconds->set(false, false);
    wt.time_scale_variable = new Toggle("Variable", time_scale_menu, this,true);
    wt.time_scale_variable->set(false, false);
    wt.time_scale_hms = new Toggle("HMS", time_scale_menu, this, true);
    wt.time_scale_hms->set(true, false);
    wt.time_scale_epoch_toggle = new Toggle("Epoch", time_scale_menu,this,true);
    wt.time_scale_epoch_toggle->set(false, false);

    unzoom_all = new Button("Unzoom All", menu_popup, args, 1, this);
    waveforms_button = new Button("Waveforms...", menu_popup, args, 1, this);
    waveform_color_button = new Button("Waveform Color...", menu_popup, args,
				1, this);
    menu_made = true;
}

void WaveformPlot::setDataSource(DataSource *ds)
{
    gvector<SegmentInfo *> *seginfo;

    setVisible(true);
    GError::getMessage();
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    data_source->removeOwner(this);
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    data_source->addOwner(this);
	}
    }
    if(data_source) {
	if((seginfo = ds->getSegmentList()) && (int)seginfo->size() > 0) {
	    addData(seginfo, 0, NULL, true, true);
	}
	if(seginfo) delete seginfo;
    }
    char *error = GError::getMessage();
    if(error) showWarning(error);
}

void WaveformPlot::inputData(gvector<SegmentInfo *> *seginfo, DataSource *ds,
		bool display_data, bool set_limits)
{
    if(ds != data_source) {
	if(data_source) data_source->removeDataReceiver(this);
	data_source = ds;
	if(data_source) data_source->addDataReceiver(this);
    }
    if(data_source) {
	addData(seginfo, 0, NULL, display_data, set_limits);
    }
}

void WaveformPlot::inputData(gvector<SegmentInfo *> *seginfo, DataSource *ds,
			bool display_data)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    data_source->removeOwner(this);
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    data_source->addOwner(this);
	}
    }
    if(data_source) {
	addData(seginfo, 0, NULL, display_data, true);
    }
}

void WaveformPlot::addData(gvector<SegmentInfo *> *seginfo, int pts,
			RecentInput *r, bool display_data, bool set_limits)
{
    const char *err_msg;
    int	i, j, k, workingUpdate=0, npts, num_selected, num_waveforms;
    int	num_inputs, max_tag_width, width;
    u_long low_swap = 0, swap;
    GTimeSeries *ts=NULL;
    cvector<CssOriginClass> *cur_origins = NULL;
    cvector<CssArrivalClass> *cur_arrivals = NULL;
    cvector<CssArrivalClass> arrivals;
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssAssocClass> assocs;
    cvector<CssStassocClass> stassocs;
    cvector<CssHydroFeaturesClass> hydro_features;
    cvector<CssInfraFeaturesClass> infra_features;
    cvector<CssStamagClass> stamags;
    cvector<CssNetmagClass> netmags;
    cvector<CssWftagClass> wftags;
    cvector<CssAmplitudeClass> amplitudes;
    cvector<CssAmpdescriptClass> ampdescripts;
    cvector<CssParrivalClass> parrivals;
    SegmentInfo **wav = NULL;
    vector<WaveInput *> *wav_inputs;
    double t1, t2, tmin, tmax;
    bool this_display_working, preview_arr = false;

    if(!data_source) {
	cerr << "WaveformPlot::addData called with null data_source" << endl;
	return;
    }
    resetDataChange(false);

    if(!tt_curves_on) setYLabelInt(True, False);

    num_selected = 0;
    for(i = 0; i < (int)seginfo->size(); i++)
    {
	if(seginfo->at(i)->selected) num_selected++;
    }
    if(num_selected <= 0) return;

    wav = (SegmentInfo **)mallocWarn(num_selected*sizeof(SegmentInfo *));

    for(i = j = 0; i < (int)seginfo->size(); i++) {
	if(seginfo->at(i)->selected) {
	    wav[j++] = seginfo->at(i);
	}
    }

    this_display_working = display_working;

    if(num_selected < working_dialog_threshold)
    {
	this_display_working = false;
    }
    if(this_display_working)
    {
/*
	WorkingDialog(widget, num_selected, 0, "Reading %d segments");
	if(!WorkingDialog(widget, 0, 1, NULL)) {
	    WorkingDialog(widget, 0, 2, NULL);
	    Free(wav);
	    return;
	}
*/
    }

    saveArrivalInfo(num_selected, wav);

    cur_origins = new cvector<CssOriginClass>;
    if(getOriginRef()) cur_origins->load(*getOriginRef());

    wav_inputs = cons.getTimeLimits(num_selected, wav, arrival_info);

    if(!wav_inputs || (num_inputs = (int)wav_inputs->size()) <= 0) {
	Free(wav);
	if(wav_inputs) delete wav_inputs;
//	if(this_display_working) WorkingDialog(widget, 0, 2, NULL);
	return;
    }

    if(this_display_working)
    {
	if(num_inputs < 100) workingUpdate = 1;
	if(num_inputs < 500) workingUpdate = 5;
	else if(num_inputs < 1000) workingUpdate = 10;
	else if(num_inputs < 5000) workingUpdate = 25;
	else workingUpdate = num_inputs/100;
	if(workingUpdate <= 0) workingUpdate = 1;
    }

    getPositionLimits(wav_inputs, &tmin, &tmax, &max_tag_width, r);

    width = getMaxTagWidth();
    k = getMaxAmpWidth();
    if(width < k) width = k;
    if(max_tag_width > width) {
	setMaxTagWidth(max_tag_width);
    }

    if(set_limits) {
	if(getDataDuration(&t1, &t2))
	{
	    if(tmin < t1 || tmax > t2) {
		if(t1 < tmin) tmin = t1;
		if(t2 > tmax) tmax = t2;
		setTimeLimits(tmin, tmax);
	    }
	}
	else {
	    setTimeLimits(tmin, tmax);
	}
    }

    /* If Auto vertical scale is on, adjust the data_height and
     * data_separation to display all waveforms.
     */
    if(num_inputs > 0 && auto_vertical_scale)
    {
	bool auto_x, auto_y;
	int left, right, top, bottom, height, data_height, data_separation;
	gvector<Waveform *> wvec;
	num_waveforms = getWaveforms(wvec);
	num_waveforms += num_inputs;

	getMargins(&auto_x, &auto_y, &left, &right, &top, &bottom);
	height = abs(top - bottom);

	data_height = height/(num_waveforms+1) - 2;
	if(data_height < 2) data_height = 2;

	data_separation = (data_height < 10) ? 0 : 2;
	data_height = height/(num_waveforms+1) - data_separation;
	if(data_height < 2) data_height = 2;
	if(data_height > 50) data_height = 50;

	setDataHeight(data_height, data_separation);
    }

    num_waveforms = 0;

    low_swap = (u_long)getProperty("low_swap", 0);

    preview_arr = getProperty("preview_arr", false);

//    setErrorWindowParent(widget);


    hold_data_change_callbacks = true;

    for(i = k = 0; i < num_inputs; i++)
    {
	if(low_swap > 0 &&
		(swap = System::getSwapSpace()) != 1 && swap < low_swap)
	{
	    showWarning("low swap space: %lu", swap);
	    for(j = i; j < num_inputs; j++) delete wav_inputs->at(j);
	    break;
	}

	arrivals.clear();
	origins.clear();
	origerrs.clear();
	assocs.clear();
	stassocs.clear();
	wftags.clear();
	hydro_features.clear();
	infra_features.clear();
	stamags.clear();
	netmags.clear();
	amplitudes.clear();
	ampdescripts.clear();
	parrivals.clear();
	ts = NULL;

	WaveInput *w = wav_inputs->at(i);

	npts = data_source->readData(&w->segments, w->start, w->end, pts,
			preview_arr, &ts, arrivals, origins, origerrs,
			assocs, stassocs, wftags, hydro_features,
			infra_features, stamags, netmags,
			amplitudes, ampdescripts, parrivals, &err_msg);

	if(err_msg != NULL) {
	    showWarning(err_msg);
	}
	if(npts <= 0 || !ts) {
	    if(ts) delete ts;
	    delete w;
	    continue;
	}
	num_waveforms++;
	setMaxNyquist(ts);
	ts->setDataSource(data_source);
		
	CPlotInputStruct input;
        input.chan.assign(w->segments[0]->chan);
	input.component = w->segments[0]->component;
	ts->setComponent(input.component);

	if(time_align == ALIGN_OBSERVED_ARRIVAL)
	{
	    double tbeg = ts->tbeg();
	    double tend = ts->tend();
	    int dc = 0, id = 0;
	    ts->getValue("dc", &dc);
	    ts->getValue("id", &id);

	    if(cur_arrivals == NULL) {
		cur_arrivals = new cvector<CssArrivalClass>(*getArrivalRef());
	    }
	    for(j = 0; j < cur_arrivals->size(); j++)
		if(!phase_align.compare(cur_arrivals->at(j)->phase) ||
		  (!phase_align.compare("FirstP") &&
			cur_arrivals->at(j)->phase[0] == 'P'))
	    {
		CssArrivalClass *a = cur_arrivals->at(j);
		if((!strcmp(ts->sta(),a->sta) || !strcmp(ts->net(),a->sta))
			&& tbeg <= a->time && tend >= a->time)
		{
		    if(a->ts_dc > 0 && (dc != a->ts_dc || id != a->ts_id))
//			    CSSTABLE_copy(t) != a->ts_copy))
		    {
			    continue;
		    }
	    	    w->t0 = a->time;
		    break;
		}
	    }
	    for(j = 0; j < arrivals.size(); j++)
	    {
		CssArrivalClass *a = arrivals[j];
		if(!phase_align.compare(a->phase) ||
		  (!phase_align.compare("FirstP") && a->phase[0]=='P'))
		{
		    if((!strcmp(ts->sta(),a->sta) || !strcmp(ts->net(),a->sta))
			    && tbeg <= a->time && tend >= a->time)
		    {
			w->t0 = a->time;
			break;
		    }
		}
	    }
	}

	input.display_t0 = ts->tbeg() - w->t0;

	ts->putValue("file order", w->segments[0]->file_order);

	addTables(*cur_origins, arrivals, origins, origerrs,
		assocs, stassocs, wftags, hydro_features, infra_features,
		stamags, netmags, amplitudes, ampdescripts, parrivals);

	input.origin = (r != NULL && r->origin != NULL) ?
		r->origin : dataToOrigin(ts);
	if(input.origin == NULL && origins.size() > 0) {
	    input.origin = origins[0];
	}

	setTag(ts, &input);

	setInputColor(ts, &input);
	input.on = display_data;

	applyCalib(ts);

	DataQCStruct dq;
	stringcpy(dq.sta, ts->sta(), sizeof(dq.sta));
	stringcpy(dq.chan, ts->chan(), sizeof(dq.chan));
	stringcpy(dq.net, ts->net(), sizeof(dq.net));
	dq.lat = ts->lat();
	dq.lon = ts->lon();
	dq.origin = input.origin;
	dq.ts = ts;

	doCallbacks(baseWidget(), (XtPointer)&dq, XtNdataQCCallback);

	// the DataQC method can alter ts.
	// if dq.ts == NULL, the DataQC failed and might skip this ts?
	if(dq.ts) ts = dq.ts;

	addTimeSeries(ts, &input);

/*
	    if(this_display_working && ++k == workingUpdate)
	    {
		k = 0;
		if(!WorkingDialog(widget, i+1, 1, NULL)) break;
	    }
*/
	delete w;
    }

    Free(wav);
    delete wav_inputs;

    delete cur_origins;
    delete cur_arrivals;

    if(num_waveforms == 0) {
	if(cons.start_time > NULL_TIME || cons.end_time > NULL_TIME) {
	    showWarning("No data. Check time limits in Waveform Layout.");
	}
    }
    else
    {
	if(!tt_curves_on) {
	    sortWaveforms(sort_type);
	}
    }
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

void WaveformPlot::addWaveforms(int num_waveforms, GTimeSeries **ts,
		Waveform **ws, Pixel *colors, bool display_data,
		bool set_limits)
{
    int	i, j, k, num;
    int	max_tag_width, width;
    Waveform *wf;
    cvector<CssOriginClass> cur_origins;
    cvector<CssArrivalClass> cur_arrivals;
    vector<WaveInput *> *wav_inputs;
    double t1, t2, tmin, tmax;

    if(num_waveforms <= 0) return;

    resetDataChange(false);

    if(!tt_curves_on) setYLabelInt(True, False);

    getTable(cur_origins);

    wav_inputs = new vector<WaveInput *>;
    for(i = 0; i < num_waveforms; i++) {
	WaveInput *w = new WaveInput(ts[i]->tbeg(), ts[i]->tend(), 0.);
	wav_inputs->push_back(w);
    }

    getPositionLimits(wav_inputs, num_waveforms, ts, &tmin, &tmax,
			&max_tag_width);

    width = getMaxTagWidth();
    k = getMaxAmpWidth();
    if(width < k) width = k;
    if(max_tag_width > width) {
	setMaxTagWidth(max_tag_width);
    }

    if(set_limits) {
	if(getDataDuration(&t1, &t2))
	{
	    if(tmin < t1 || tmax > t2) {
		if(t1 < tmin) tmin = t1;
		if(t2 > tmax) tmax = t2;
		setTimeLimits(tmin, tmax);
	    }
	}
	else {
	    setTimeLimits(tmin, tmax);
	}
    }

    /* If Auto vertical scale is on, adjust the data_height and
     * data_separation to display all waveforms.
     */
    if(auto_vertical_scale)
    {
	bool auto_x, auto_y;
	int left, right, top, bottom, height, data_height, data_separation;
	gvector<Waveform *> wvec;
	num = getWaveforms(wvec);
	num += num_waveforms;

	getMargins(&auto_x, &auto_y, &left, &right, &top, &bottom);
	height = abs(top - bottom);

	data_height = height/(num+1) - 2;
	if(data_height < 2) data_height = 2;

	data_separation = (data_height < 10) ? 0 : 2;
	data_height = height/(num+1) - data_separation;
	if(data_height < 2) data_height = 2;
	if(data_height > 50) data_height = 50;

	setDataHeight(data_height, data_separation);
    }

    hold_data_change_callbacks = true;

    for(i = 0; i < num_waveforms; i++)
    {
	CPlotInputStruct input;
	WaveInput *w = wav_inputs->at(i);

	setMaxNyquist(ts[i]);
        ts[i]->getChan(input.chan);
	input.component = ts[i]->component();

	if(time_align == ALIGN_OBSERVED_ARRIVAL)
	{
	    double tbeg = ts[i]->tbeg();
	    double tend = ts[i]->tend();
	    int dc = 0, id = 0;
	    ts[i]->getValue("dc", &dc);
	    ts[i]->getValue("id", &id);

	    if(cur_arrivals.size() == 0) {
		getTable(cur_arrivals);
	    }
	    for(j = 0; j < cur_arrivals.size(); j++)
		if(!phase_align.compare(cur_arrivals[j]->phase) ||
		  (!phase_align.compare("FirstP") &&
			cur_arrivals[j]->phase[0] == 'P'))
	    {
		CssArrivalClass *a = cur_arrivals[j];
		if((!strcmp(ts[i]->sta(),a->sta) ||!strcmp(ts[i]->net(),a->sta))
			&& tbeg <= a->time && tend >= a->time)
		{
		    if(a->ts_dc > 0 && (dc != a->ts_dc || id != a->ts_id)) {
			    continue;
		    }
	    	    w->t0 = a->time;
		    break;
		}
	    }
	}

	input.display_t0 = ts[i]->tbeg() - w->t0;

	input.origin = dataToOrigin(ts[i]);

	setTag(ts[i], &input);

	if(colors && colors[i] != UNDEFINED_PIXEL) {
	    input.color = colors[i];
	}
	else {
	    setInputColor(ts[i], &input);
	}
	input.on = display_data;

	applyCalib(ts[i]);

	DataQCStruct dq;
	stringcpy(dq.sta, ts[i]->sta(), sizeof(dq.sta));
	stringcpy(dq.chan, ts[i]->chan(), sizeof(dq.chan));
	stringcpy(dq.net, ts[i]->net(), sizeof(dq.net));
	dq.lat = ts[i]->lat();
	dq.lon = ts[i]->lon();
	dq.origin = input.origin;
	dq.ts = ts[i];

	doCallbacks(baseWidget(), (XtPointer)&dq, XtNdataQCCallback);

	// the DataQC method can alter ts.
	// if dq.ts == NULL, the DataQC failed and might skip this ts?
	if(dq.ts) ts[i] = dq.ts;

	wf = addTimeSeries(ts[i], &input);
	if(ws) ws[i] = wf;

	delete w;
    }

    delete wav_inputs;

    if(!tt_curves_on) {
	sortWaveforms(sort_type);
    }
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

void WaveformPlot::saveArrivalInfo(int num, SegmentInfo **s)
{
    ArrivalInfo a;

    for(int i = 0; i < num; i++)
    {
	for(int j = 0; j < (int)s[i]->arrivals.size(); j++)
	{
	    int k;
	    ArrivalInfo arr = s[i]->arrivals[j];
	    for(k = 0; k < (int)arrival_info.size(); k++) {
		if(arrival_info[k].arid == arr.arid) break;
	    }
	    if(k == (int)arrival_info.size()) {
	        a.arid = arr.arid;
		a.orid = arr.orid;
		a.time = arr.time;
		strcpy(a.net, arr.net);
		strcpy(a.chan, arr.chan);
		strcpy(a.phase, arr.phase);
		arrival_info.push_back(a);
            }
        }
    }
}

// don't need this. max_nyquist used in old Polar Filter
void WaveformPlot::setMaxNyquist(GTimeSeries *ts)
{
    for(int i = 0; i < ts->size(); i++)
    {
	if(ts->segment(i)->tdel() > 0.) {
	    double nyquist = .5/ts->segment(i)->tdel();
	    if(max_nyquist < nyquist) max_nyquist = nyquist;
	}
    }
}

bool WaveformPlot::applyCalib(GTimeSeries *ts)
{
    if(apply_calib)
    {
	if(!ts->getMethod("CalibData")) {
	    CalibData *cal = new CalibData();
	    cal->apply(ts);
	}
	return true;
    }
    return false;
}

void WaveformPlot::getPositionLimits(vector<WaveInput *> *wav_inputs,
	    double *t_min, double *t_max, int *max_tag_width, RecentInput *r)
{
    int i, width, height, time_scale;
    double tmin=0., tmax=0., tbeg, tend, epoch_min=0.;
    CssOriginClass *o = NULL;
    Arg args[1];

    if(!wav_inputs || wav_inputs->size() <= 0) return;

    if(time_align == ALIGN_TRUE_TIME)
    {
	XtSetArg(args[0], XtNtimeScale, &time_scale);
	getValues(args, 1);
	if(time_scale != TIME_SCALE_HMS && !time_scale_epoch)
	{
	    gvector<Waveform *> wvec;
	    int num = getWaveforms(wvec);
	    if(num > 0) epoch_min = wvec[0]->tbeg();
	    for(i = 1; i < num; i++) {
		double t = wvec[i]->tbeg();
		if(t < epoch_min) epoch_min = t;
	    }
	    if(num == 0) {
		epoch_min = wav_inputs->at(0)->start;
	    }
	    for(i = 0; i < (int)wav_inputs->size(); i++) {
		if(wav_inputs->at(i)->start < epoch_min) {
		    epoch_min = wav_inputs->at(i)->start;
		}
	    }
	    for(i = 0; i < num; i++) {
		double t = wvec[i]->tbeg();
		wvec[i]->scaled_x0 = t - epoch_min;
	    }
	}
    }

    /* Determine the min,max waveform limits after the new waveforms are
     * positioned according to the last align option. Also finds the
     * maximum tag width.
     */
    *max_tag_width = 0;
    if(r == NULL || r->origin == NULL) o = new CssOriginClass();

    for(i = 0; i < (int)wav_inputs->size(); i++)
    {
	CPlotInputStruct input;
	WaveInput *w = wav_inputs->at(i);
	SegmentInfo *s = w->segments[0];
	input.chan.assign(s->chan);
	if(r == NULL || r->origin == NULL) {
	    o->time = s->origin_time;
	    o->lat = s->origin_lat;
	    o->lon = s->origin_lon;
	    o->depth = s->origin_depth;
	    input.origin = o;
	}
	else {
	    input.origin = r->origin;
	    o = r->origin;
	}
	input.display_t0 = w->start;
	setTag(s->sta, s->station_lat, s->station_lon, &input);
	getTagDimensions(input.tag, &width, &height);
	if(width > *max_tag_width) *max_tag_width = width;

	if(time_align == ALIGN_TRUE_TIME) {
	    if(time_scale != TIME_SCALE_HMS && !time_scale_epoch) {
		w->t0 = epoch_min;
	    }
	    else {
		w->t0 = 0.;
	    }
	}
	else if(time_align == ALIGN_FIRST_POINT) {
	    w->t0 = w->start;
	}
	else if(time_align == ALIGN_ORIGIN)
	{
	    if(o->time > NULL_TIME_CHECK) {
		w->t0 = o->time;
	    }
	    else {
		w->t0 = w->start;
	    }
	}
	else if(time_align == ALIGN_PREDICTED_ARRIVAL ||
		time_align == ALIGN_OBSERVED_ARRIVAL)
	{
	    double tt, slow;
	    w->t0 = w->start;

	    if(o->time > -999999990.)
	    {
		string phase;
		if(!phase_align.compare("FirstP") ||
		   !phase_align.compare("FirstS"))
		{
		    char c = !phase_align.compare("FirstP") ? 'P' : 'S';

		    if(firstArrival(o, s->station_lat, s->station_lon,
			s->station_elev, s->net, s->sta, c, phase,
			&tt, &slow) == 1)
		    {
			w->t0 = o->time + tt;
		    }
		}
		else if((tt = getTravelTime(phase_align, o, s->station_lat,
			s->station_lon, s->station_elev, s->net, s->sta,
			&slow, phase)) > 0.)
		{
		    w->t0 = o->time + tt;
		}
	    }
	}
	else {
	    w->t0 = 0.;
	}
	tbeg = w->start - w->t0;
	tend = w->end - w->t0;

	if(i == 0) {
	    tmin = tbeg;
	    tmax = tend;
	}
	else {
	    if(tbeg < tmin) tmin = tbeg;
	    if(tend > tmax) tmax = tend;
	}
    }
    if(r == NULL || r->origin == NULL) o->deleteObject();

    *t_min = tmin;
    *t_max = tmax;
}

void WaveformPlot::getPositionLimits(vector<WaveInput *> *wav_inputs,
	int num_waveforms, GTimeSeries **ts, double *t_min, double *t_max,
	int *max_tag_width)
{
    int i, width, height, time_scale;
    double tmin=0., tmax=0., tbeg, tend, epoch_min=0.;
    CssOriginClass *o = NULL;
    Arg args[1];

    if(num_waveforms <= 0 || !wav_inputs
	|| (int)wav_inputs->size() != num_waveforms) return;

    if(time_align == ALIGN_TRUE_TIME)
    {
	XtSetArg(args[0], XtNtimeScale, &time_scale);
	getValues(args, 1);
	if(time_scale != TIME_SCALE_HMS && !time_scale_epoch)
	{
	    gvector<Waveform *> wvec;
	    int num = getWaveforms(wvec);
	    if(num > 0) epoch_min = wvec[0]->tbeg();
	    for(i = 1; i < num; i++) {
		double t = wvec[i]->tbeg();
		if(t < epoch_min) epoch_min = t;
	    }
	    if(num == 0) {
		epoch_min = wav_inputs->at(0)->start;
	    }
	    for(i = 0; i < (int)wav_inputs->size(); i++) {
		if(wav_inputs->at(i)->start < epoch_min) {
		    epoch_min = wav_inputs->at(i)->start;
		}
	    }
	    for(i = 0; i < num; i++) {
		double t = wvec[i]->tbeg();
		wvec[i]->scaled_x0 = t - epoch_min;
	    }
	}
    }

    /* Determine the min,max waveform limits after the new waveforms are
     * positioned according to the last align option. Also finds the
     * maximum tag width.
     */
    *max_tag_width = 0;
    o = new CssOriginClass();

    for(i = 0; i < num_waveforms; i++)
    {
	CPlotInputStruct input;
	WaveInput *w = wav_inputs->at(i);
	ts[i]->getChan(input.chan);
	input.display_t0 = ts[i]->tbeg();
	setTag(ts[i], &input);
	getTagDimensions(input.tag, &width, &height);
	if(width > *max_tag_width) *max_tag_width = width;

	if(time_align == ALIGN_TRUE_TIME) {
	    if(time_scale != TIME_SCALE_HMS && !time_scale_epoch) {
		w->t0 = epoch_min;
	    }
	    else {
		w->t0 = 0.;
	    }
	}
	else if(time_align == ALIGN_FIRST_POINT) {
	    w->t0 = w->start;
	}
	else if(time_align == ALIGN_ORIGIN)
	{
	    if(o->time > NULL_TIME_CHECK) {
		w->t0 = o->time;
	    }
	    else {
		w->t0 = w->start;
	    }
	}
	else if(time_align == ALIGN_PREDICTED_ARRIVAL ||
		time_align == ALIGN_OBSERVED_ARRIVAL)
	{
	    double tt, slow;
	    w->t0 = w->start;

	    if(o->time > -999999990.)
	    {
		string phase;
		if(!phase_align.compare("FirstP") ||
		   !phase_align.compare("FirstS"))
		{
		    char c = !phase_align.compare("FirstP") ? 'P' : 'S';

		    if(firstArrival(o, ts[i]->lat(), ts[i]->lon(),ts[i]->elev(),
			ts[i]->net(), ts[i]->sta(), c, phase, &tt, &slow) == 1)
		    {
			w->t0 = o->time + tt;
		    }
		}
		else if((tt = getTravelTime(phase_align, o, ts[i]->lat(),
			ts[i]->lon(), ts[i]->elev(), ts[i]->net(), ts[i]->sta(),
			&slow, phase)) > 0.)
		{
		    w->t0 = o->time + tt;
		}
	    }
	}
	else {
	    w->t0 = 0.;
	}
	tbeg = w->start - w->t0;
	tend = w->end - w->t0;

	if(i == 0) {
	    tmin = tbeg;
	    tmax = tend;
	}
	else {
	    if(tbeg < tmin) tmin = tbeg;
	    if(tend > tmax) tmax = tend;
	}
    }
    o->deleteObject();

    *t_min = tmin;
    *t_max = tmax;
}

void WaveformPlot::updateTag(Waveform *w)
{
    int i, num_selected, *selected=NULL;
    num_selected = tag_contents->getOrder(&selected);
    char *user_string = tag_contents->getText();

    num_tag_members = 0;
    for(i = 0; i < num_selected; i++)
    {
	if(selected[i] >= 0) {
	    tag_members[num_tag_members++] = selected[i];
	}
    }
    Free(selected);

    user_tag_string.clear();
    if(user_string) user_tag_string.assign(user_string);

    w->ts->tag.setMembers(num_tag_members, tag_members, user_string);

    char *new_tag = Waveform::getTag(w->ts->tag, w->sta(), w->chan(),
		getPrimaryOrigin(w), w->tbeg(), w->lat(), w->lon(), w->ts);

    changeTag(w, new_tag, true);
    
    Free(new_tag);
}

void WaveformPlot::setTagMembers(int num_selected, int *selected,
			const string &user_string, bool selected_only)
{
    gvector<Waveform *> wvec;

    num_tag_members = 0;
    for(int i = 0; i < num_selected; i++)
    {
	if(selected[i] >= 0) {
	    tag_members[num_tag_members++] = selected[i];
	}
    }
    user_tag_string.assign(user_string);
    
    if(selected_only) {
	if(getSelectedWaveforms(wvec) <= 0) return;
    }
    else {
	if(getWaveforms(wvec) <= 0) return;
    }

    for(int i = 0; i < wvec.size(); i++)
    {
	Waveform *w = wvec[i];

	w->ts->tag.setMembers(num_tag_members, tag_members, user_string);

	char *new_tag = Waveform::getTag(w->ts->tag, w->sta(), w->chan(),
		    getPrimaryOrigin(w), w->tbeg(), w->lat(), w->lon(), w->ts);

	changeTag(wvec[i], new_tag, true);

	Free(new_tag);
    }
}

void WaveformPlot::setTag(GTimeSeries *ts, CPlotInputStruct *input)
{
    char *c;

    if((int)ts->tag.members.size() == 0) {
	ts->tag.setMembers(num_tag_members, tag_members, user_tag_string);
    }
    c = Waveform::getTag(ts->tag, ts->sta(), input->chan, input->origin,
			ts->tbeg(), ts->lat(), ts->lon(), ts);
    input->tag.assign(c);
    free(c);
}

void WaveformPlot::setTag(const string &sta, double lat, double lon,
				CPlotInputStruct *input)
{
    WaveformTag tg;
    tg.setMembers(num_tag_members, tag_members, user_tag_string);
    char *c = Waveform::getTag(tg, sta, input->chan, input->origin,
			input->display_t0, lat, lon, NULL);
    input->tag.assign(c);
    free(c);
}

void WaveformPlot::setInputColor(GTimeSeries *ts, CPlotInputStruct *input)
{
    int	i;
    gvector<Waveform *> wvec;

    getWaveforms(wvec);

    if(!color_code.compare("uniform")) /* all the same color */
    {
	if(wvec.size() > 0) {
	    input->color = wvec[wvec.size()-1]->fg;
	}
	else {
	    input->color = nextColor(wvec);
	}
    }
    else if(!color_code.compare("unique")) /* all different */
    {
	input->color = nextColor(wvec);
    }
    else if(!color_code.compare("station"))
    {
	for(i = 0; i < wvec.size(); i++) {
	    if(!strcmp(ts->sta(), wvec[i]->sta())) {
	 	input->color = wvec[i]->fg;
		break;
	    }
	}
	if(i == wvec.size()) {
	    input->color = nextColor(wvec);
	}
    }
    else if(!color_code.compare("channel"))
    {
	for(i = 0; i < wvec.size(); i++) {
	    if(!input->chan.compare(wvec[i]->chan())) {
	 	input->color = wvec[i]->fg;
		break;
	    }
	}
	if(i == wvec.size()) {
	    input->color = nextColor(wvec);
	}
    }
    else if(!color_code.compare("network"))
    {
	for(i = 0; i < wvec.size(); i++) {
	    if(!strcmp(ts->net(), wvec[i]->net())) {
	 	input->color = wvec[i]->fg;
		break;
	    }
	}
	if(i == wvec.size()) {
	    input->color = nextColor(wvec);
	}
    }
    else if(!color_code.compare("origin"))
    {
	for(i = 0; i < wvec.size(); i++)
	{
	    CssOriginClass *o = getPrimaryOrigin(wvec[i]);
	    if(input->origin == NULL) {
		if(o == NULL) {
		    input->color = wvec[i]->fg;
		    break;
		}
	    }
	    else if(o != NULL && input->origin->getDC() == o->getDC()
		    && input->origin->orid == o->orid)
	    {
		input->color = wvec[i]->fg;
		break;
	    }
	}
	if(i == wvec.size()) {
	    input->color = nextColor(wvec);
	}
    }
    else {	/* uniform */
	if(wvec.size() > 0) {
	    input->color = wvec[wvec.size()-1]->fg;
	}
	else {
	    input->color = nextColor(wvec);
	}
    }
}

Pixel WaveformPlot::nextColor(gvector<Waveform *> &wvec)
{
    int i, j, num_colors;

    num_colors = numWaveformColors();
    /* find first color not being used in fg
     */
    for(i = 0; i < num_colors; i++)
    {
	for(j = 0; j < wvec.size() &&
		(Pixel)wvec[j]->fg != waveform_colors[i]; j++);
	if(j == wvec.size()) {
	    return(waveform_colors[i]);
	}
    }
    /* if all colors have been used, cycle through colors again
     */
    if(++next_color >= num_colors) next_color = 0;
    return(waveform_colors[next_color]);
}

Pixel WaveformPlot::nextColor(int num_waveforms, Pixel *fg)
{
    int i, j, num_colors;

    num_colors = numWaveformColors();

    /* find first color not being used in fg
     */
    for(i = 0; i < num_colors; i++)
    {
	for(j = 0; j < num_waveforms && fg[j] != waveform_colors[i]; j++);
	if(j == num_waveforms) {
	    return(waveform_colors[i]);
	}
    }
    /* if all colors have been used, cycle through colors again
     */
    if(++next_color >= num_colors) next_color = 0;
    return(waveform_colors[next_color]);
}

void WaveformPlot::addTables(cvector<CssOriginClass> &cur_origins,
	cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
	cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
	cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
	cvector<CssHydroFeaturesClass> &hydro_features,
	cvector<CssInfraFeaturesClass> &infra_features,
	cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
	cvector<CssAmplitudeClass> &amplitudes,
	cvector<CssAmpdescriptClass> &ampdescripts,
	cvector<CssParrivalClass> &parrivals)
{
    int i, j, k;
    AllTables *tables = NULL;

    /* first check all main_plot's for existing dc,id css,sac,gse, etc,
     * must give consistent dc,id's for repeated calls. Arrival/origin
     * changes must update all main_plots.
     */
    if(!(tables = (AllTables *)mallocWarn(num_wave_plots*sizeof(AllTables)))) {
	return;
    }
    for(i = 0; i < num_wave_plots; i++) {
	WaveformPlot *w = wave_plots[i];

	tables[i].arrivals = w->getArrivalRef();
	tables[i].origins  = w->getOriginRef();
	tables[i].origerrs = w->getOrigerrRef();
	tables[i].assocs   = w->getAssocRef();
	tables[i].stassocs = w->getStassocRef();
	tables[i].stamags  = w->getStamagRef();
	tables[i].netmags  = w->getNetmagRef();
	tables[i].hydros   = w->getHydroFeaturesRef();
	tables[i].infras   = w->getInfraFeaturesRef();
	tables[i].wftags   = w->getWftagRef();
	tables[i].amplitudes = w->getAmplitudeRef();
	tables[i].ampdescripts = w->getAmpdescriptRef();
	tables[i].parrivals = w->getParrivalRef();
    }

    for(j = 0; j < assocs.size(); j++) {
	CssAssocClass *assoc = assocs[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].assocs->size(); k++) {
		if(*assoc == *tables[i].assocs->at(k))
		{
		    assoc = tables[i].assocs->at(k);
		}
	    }
	}
	putTable(assoc);
    }
    for(j = 0; j < arrivals.size(); j++) {
	CssArrivalClass *arrival = arrivals[j];
	Pixel fg = getArrivalFg(arrival);
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].arrivals->size(); k++) {
		if(*arrival == *tables[i].arrivals->at(k))
		{
		    arrival = tables[i].arrivals->at(k);
		}
	    }
	}
	arrival->atype = 0;
	putArrivalWithColor(arrival, fg);
    }
    for(j = 0; j < origins.size(); j++)
    {
	CssOriginClass *origin = origins[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].origins->size(); k++) {
		if(*origin == *tables[i].origins->at(k))
		{
		    origin = tables[i].origins->at(k);
		}
	    }
	}
	putTable(origin);

	for(k = 0; k < cur_origins.size(); k++)
	    if(cur_origins[k]->orid==origin->orid && *cur_origins[k] != *origin)
	{
	    showWarning("warning! duplicate origins have same orid: '%d'",
			origin->orid);
	}
    }
    for(j = 0; j < origerrs.size(); j++) {
	CssOrigerrClass *origerr = origerrs[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].origerrs->size(); k++) {
		if(*origerr == *tables[i].origerrs->at(k))
		{
		    origerr = tables[i].origerrs->at(k);
		}
	    }
	}
	putTable(origerr);
    }
    for(j = 0; j < stassocs.size(); j++) {
	CssStassocClass *stassoc = stassocs[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].stassocs->size(); k++) {
		if(*stassoc == *tables[i].stassocs->at(k))
		{
		    stassoc = tables[i].stassocs->at(k);
		}
	    }
	}
	putTable(stassoc);
    }
    for(j = 0; j < stamags.size(); j++) {
	CssStamagClass *stamag = stamags[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].stamags->size(); k++) {
		if(*stamag == *tables[i].stamags->at(k))
		{
		    stamag = tables[i].stamags->at(k);
		}
	    }
	}
	putTable(stamag);
    }
    for(j = 0; j < netmags.size(); j++) {
	CssNetmagClass *netmag = netmags[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].netmags->size(); k++) {
		if(*netmag == *tables[i].netmags->at(k))
		{
		    netmag = tables[i].netmags->at(k);
		}
	    }
	}
	putTable(netmag);
    }
    for(j = 0; j < hydro_features.size(); j++) {
	CssHydroFeaturesClass *hydro= hydro_features[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].hydros->size(); k++) {
		if(*hydro == *tables[i].hydros->at(k))
		{
		    hydro = tables[i].hydros->at(k);
		}
	    }
	}
	putTable(hydro);
    }
    for(j = 0; j < infra_features.size(); j++) {
	CssInfraFeaturesClass *infra= infra_features[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].infras->size(); k++) {
		if(*infra == *tables[i].infras->at(k))
		{
		    infra = tables[i].infras->at(k);
		}
	    }
	}
	putTable(infra);
    }
    for(j = 0; j < wftags.size(); j++) {
	CssWftagClass *wftag = wftags[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].wftags->size(); k++) {
		if(*wftag == *tables[i].wftags->at(k))
		{
		    wftag = tables[i].wftags->at(k);
		}
	    }
	}
	putTable(wftag);
    }
    for(j = 0; j < amplitudes.size(); j++) {
	CssAmplitudeClass *amp = amplitudes[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].amplitudes->size(); k++) {
		if(*amp == *tables[i].amplitudes->at(k))
		{
		    amp = tables[i].amplitudes->at(k);
		}
	    }
	}
	putTable(amp);
    }
    for(j = 0; j < ampdescripts.size(); j++) {
	CssAmpdescriptClass *amp = ampdescripts[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].ampdescripts->size(); k++) {
		if(*amp == *tables[i].ampdescripts->at(k))
		{
		    amp = tables[i].ampdescripts->at(k);
		}
	    }
	}
	putTable(amp);
    }
    for(j = 0; j < parrivals.size(); j++) {
	CssParrivalClass *parrival = parrivals[j];
	for(i = 0; i < num_wave_plots; i++) {
	    for(k = 0; k < tables[i].parrivals->size(); k++) {
		if(*parrival == *tables[i].parrivals->at(k))
		{
		    parrival = tables[i].parrivals->at(k);
		}
	    }
	}
	putTable(parrival);
    }
    Free(tables);
}

Pixel WaveformPlot::getArrivalFg(CssArrivalClass *a)
{
    // see input.c version that allows author-color-coded arrivals
    Pixel pixel;

    if(arrival_color_mode == COLOR_BY_ASSOC) {
	cvector<CssAssocClass> *assocs = getAssocRef();
	int i;
	for(i = 0; i < assocs->size() && assocs->at(i)->arid != a->arid; i++);
	if(i < assocs->size()) {
	    pixel = stringToPixel("blue");
	}
	else {
	    pixel = stringToPixel("red");
	}
    }
    else {
	Arg args[1];
	XtSetArg(args[0], XtNforeground, &pixel);
	getValues(args, 1);
    }
    return pixel;
}

void WaveformPlot::alignWaveforms(AlignType type, const string &phase)
{
    if(type == ALIGN_PREDICTED_ARRIVAL) {
	phase_align.assign(phase);
    }
    alignWaveforms(type);
}

void WaveformPlot::alignWaveforms(AlignType type)
{
    char *phase;
    double x;

    if(type != ALIGN_TRUE_TIME)
    {
	Arg args[1];
	int time_scale;

	XtSetArg(args[0], XtNtimeScale, &time_scale);
	getValues(args, 1);
	if(time_scale != TIME_SCALE_SECONDS && time_scale != TIME_SCALE_VARIABLE
		&& time_scale != TIME_SCALE_HMS)
	{
	    XtSetArg(args[0], XtNtimeScale, TIME_SCALE_VARIABLE);
	    setValues(args, 1);
	}
    }

    if(type == ALIGN_TRUE_TIME) {
	alignTrueTime();
	time_align = ALIGN_TRUE_TIME;
    }
    else if(type == ALIGN_ORIGIN) {
	alignOrigin(true);
	time_align = ALIGN_ORIGIN;
    }
    else if(type == ALIGN_FIRST_POINT) {
	alignFirstPoint();
	time_align = ALIGN_FIRST_POINT;
    }
    else if(type == ALIGN_PREDICTED_P)
    {
	if(alignOnPredictedPhase("FirstP") == -1)
	{
	    showWarning(getError());
	}
	time_align = ALIGN_PREDICTED_ARRIVAL;
	phase_align.assign("FirstP");
	if(getPhaseLine(&x, &phase) && !strcmp(phase, "FirstP"))
	{
	    positionPhaseLine(phase, 0., False);
	}
    }
    else if(type == ALIGN_OBSERVED_P)
    {
	alignOnPhase("FirstP");
	time_align = ALIGN_OBSERVED_ARRIVAL;
	phase_align.assign("FirstP");
	if(getPhaseLine(&x, &phase) && !strcmp(phase,"FirstP"))
	{
	    positionPhaseLine(phase, 0., False);
	}
    }
    else if(type == ALIGN_PREDICTED_ARRIVAL)
    {
	if(alignOnPredictedPhase(phase_align) == -1)
	{
	    showWarning(getError());
	}
	time_align =  ALIGN_PREDICTED_ARRIVAL;
	if(getPhaseLine(&x, &phase) && !phase_align.compare(phase)){
	    positionPhaseLine(phase, 0., False);
	}
    }
    else if(type == ALIGN_OBSERVED_ARRIVAL)
    {
	alignOnPhase(phase_align);
	time_align = ALIGN_OBSERVED_ARRIVAL;
	if(getPhaseLine(&x, &phase) && !phase_align.compare(phase)){
	    positionPhaseLine(phase, 0., False);
	}
    }
    doCallbacks(baseWidget(), (XtPointer)&time_align, XtNalignCallback);
}

void WaveformPlot::alignTrueTime(void)
{
    int i, time_scale;
    Arg args[1];
    vector<double> scaled_x0;
    gvector<Waveform *> wvec;

    if(getWaveforms(wvec, false) <= 0) return;

    XtSetArg(args[0], XtNtimeScale, &time_scale);
    getValues(args, 1);

    if(time_scale != TIME_SCALE_HMS && !time_scale_epoch) {
	double epoch_min;
	if(wvec.size() > 0) epoch_min = wvec[0]->tbeg();
	for(i = 1; i < wvec.size(); i++) {
	    double t = wvec[i]->tbeg();
	    if(t < epoch_min) epoch_min = t;
	}
	for(i = 0; i < wvec.size(); i++) {
	    scaled_x0.push_back(wvec[i]->tbeg() - epoch_min);
	}
    }
    else {
	for(i = 0; i < wvec.size(); i++) {
	    scaled_x0.push_back(wvec[i]->tbeg());
	}
    }

    positionX(wvec, scaled_x0);
}

void WaveformPlot::alignFirstPoint(void)
{
    vector<double> scaled_x0;
    gvector<Waveform *> wvec;

    if(getWaveforms(wvec, false) <= 0) return;

    for(int i = 0; i < wvec.size(); i++) scaled_x0.push_back(0.);

    positionX(wvec, scaled_x0);
}

void WaveformPlot::alignOrigin(bool warn)
{
    int i;
    CssOriginClass *origin;
    vector<double> scaled_x0;
    gvector<Waveform *> wvec;

    if(getWaveforms(wvec, false) <= 0) return;

    for(i = 0; i < wvec.size(); i++)
    {
	if((origin = getPrimaryOrigin(wvec[i]))
		&& origin->time > NULL_TIME_CHECK)
	{
	    scaled_x0.push_back(wvec[i]->tbeg() - origin->time);
	}
	else
	{
	    scaled_x0.push_back(0.);
	    if(warn) {
		showWarning("%s/%s: no origin time",
			wvec[i]->sta(), wvec[i]->chan());
	    }
	}
    }
    positionX(wvec, scaled_x0);
}

void WaveformPlot::alignWaveforms(gvector<Waveform *> &wvec,
			vector<double> &t_lags)
{
    int i;
    double t0_min, x0_min, y0_min;
    vector<double> scaled_x0;

    if(wvec.size() <= 0) return;

    /* make lags relative to the top waveform.
     */

    t0_min = wvec[0]->tbeg() + t_lags[0];
    x0_min = wvec[0]->scaled_x0;
    y0_min = wvec[0]->scaled_y0;

    for(i = 1; i < wvec.size(); i++) {
	if(wvec[i]->scaled_y0 < y0_min) {
	    t0_min = wvec[i]->tbeg() + t_lags[i];
	    x0_min = wvec[i]->scaled_x0;
	    y0_min = wvec[i]->scaled_y0;
	}
    }

    for(i = 0; i < wvec.size(); i++)
    {
	scaled_x0.push_back(x0_min + wvec[i]->tbeg() + t_lags[i] - t0_min);
    }
    positionX(wvec, scaled_x0);

    /* if nothing is visible, zoomout
    */
    if(!dataVisible()) {
	zoomOut();
    }
}

void WaveformPlot::setTimeScale(TimeType set_time_scale)
{
    Arg args[1];
    int time_scale;

    XtSetArg(args[0], XtNtimeScale, &time_scale);
    getValues(args, 1);

    if(set_time_scale == TIME_SECONDS)
    {
	if(time_align == ALIGN_TRUE_TIME &&
	    (time_scale == TIME_SCALE_HMS || time_scale_epoch))
	{
	    shiftData(0);
	}
	if(time_scale == TIME_SCALE_SECONDS) {
	    redraw();
	}
	else {
	    XtSetArg(args[0], XtNtimeScale, TIME_SCALE_SECONDS);
	}
	time_scale_epoch = false;
    }
    else if(set_time_scale == TIME_VARIABLE)
    {
	if(time_align == ALIGN_TRUE_TIME &&
	    (time_scale == TIME_SCALE_HMS || time_scale_epoch))
	{
	    shiftData(0);
	}
	XtSetArg(args[0], XtNtimeScale, TIME_SCALE_VARIABLE);
	time_scale_epoch = false;
    }
    else if(set_time_scale == TIME_HMS)
    {
	if(time_align == ALIGN_TRUE_TIME && time_scale != TIME_SCALE_HMS
		&& !time_scale_epoch)
	{
	    shiftData(1);
	}
	XtSetArg(args[0], XtNtimeScale, TIME_SCALE_HMS);
	time_scale_epoch = false;
    }
    else if(set_time_scale == TIME_EPOCH)
    {
	if(time_align == ALIGN_TRUE_TIME && time_scale != TIME_SCALE_HMS)
	{
	    shiftData(1);
	}
	if(time_scale == TIME_SCALE_SECONDS) {
	    redraw();
	}
	else {
	    XtSetArg(args[0], XtNtimeScale, TIME_SCALE_SECONDS);
	}
	time_scale_epoch = true;
    }
    else {
	return;
    }
    setValues(args, 1);
}

void WaveformPlot::shiftData(int shift_forward)
{
    int		num;
    double	min, epoch_min;
    gvector<Waveform *> wvec;

    num = getWaveforms(wvec, false);
    if(num > 0) {
	min = wvec[0]->scaled_x0;
	epoch_min = wvec[0]->tbeg();
    }
    else {
	min = epoch_min = 0.;
    }

    for(int i = 1; i < num; i++)
    {
	if(wvec[i]->scaled_x0 < min) min = wvec[i]->scaled_x0;

	if(wvec[i]->tbeg() < epoch_min) {
	    epoch_min = wvec[i]->tbeg();
	}
    }

    if(!shift_forward) {
	timeShift(-min);
    }
    else {
	timeShift(epoch_min);
    }
}

void WaveformPlot::adjustDataHeight(DataHeightType type)
{
    int dataHeight = 0;
    int dataSeparation = getDataSeparation();

    if(type == SPACE_MORE) {
	dataHeight = (int)(1.5*getDataHeight() +.5);
	if(dataHeight < 1) dataHeight = 1;
	setDataHeight(dataHeight, dataSeparation);
    }
    else if(type == SPACE_LESS) {
	dataHeight = (int)(2.*getDataHeight()/3. +.5);
	if(dataHeight < 1) dataHeight = 1;
	setDataHeight(dataHeight, dataSeparation);
    }
    else if(type == SCALE_UP) {
	scaleUp(numSelected() == 0);
    }
    else if(type == SCALE_DOWN) {
	scaleDown(numSelected() == 0);
    }
    if((type == SPACE_MORE || type == SPACE_LESS) && amplitude_scale)
    {
	char text[20];
	snprintf(text, 20, "%d", getDataHeight());
	amplitude_scale->setDataHeight(text);

	snprintf(text, 20, "%d", getDataSeparation());
	amplitude_scale->setDataSeparation(text);
    }
}

void WaveformPlot::setAutoScale(bool set)
{
    auto_vertical_scale = set;
    if(set)
    {
	bool auto_x, auto_y;
	int left, right, top, bottom, height, data_height, data_separation;
	gvector<Waveform *> wvec;
	int num_waveforms = getWaveforms(wvec);

	getMargins(&auto_x, &auto_y, &left, &right, &top, &bottom);
	height = abs(top - bottom);

	data_height = height/(num_waveforms+1) - 2;
	if(data_height < 2) data_height = 2;

	data_separation = (data_height < 10) ? 0 : 2;
	data_height = height/(num_waveforms+1) - data_separation;
	if(data_height < 2) data_height = 2;
	if(data_height > 50) data_height = 50;

	setDataHeight(data_height, data_separation);
    }
    else
    {
	setDataHeight(50, 2);
    }
}

void WaveformPlot::showComponents(ComponentsType type)
{
    if(type == ALL_COMPONENTS) {
	displayComponents(CPLOT_ALL_COMPONENTS);
    }
    else if(type == Z_COMPONENTS) {
	displayComponents(CPLOT_Z_COMPONENT);
    }
    else if(type == N_AND_E_COMPONENTS) {
	displayComponents(CPLOT_HORIZONTAL_COMPONENTS);
    }
    else if(type == N_COMPONENTS) {
	displayComponents(CPLOT_N_COMPONENT);
    }
    else if(type == E_COMPONENTS) {
	displayComponents(CPLOT_E_COMPONENT);
    }
}

void WaveformPlot::addCursor(AddCursorType type)
{
    AxesCursorCallbackStruct *a;
    double xmin, xmax, xmid, ymin, ymax, ymid;
    
    getLimits(&xmin, &xmax, &ymin, &ymax);
    xmid = .5*(xmin+xmax);
    ymid = .5*(ymin+ymax);

    if(type == ADD_TIME_LINE) {
	if(getLines(&a) == 0) addLine();
	else if(a->scaled_x1 < xmin || a->scaled_x1 > xmax) {
	    positionLine(a->label, xmid, False);
	}
    }
    else if(type == ADD_TIME_LIMITS) {
	if(getDoubleLines(&a) == 0) addDoubleLine();
	else if(a->scaled_x1 < xmin || a->scaled_x2 > xmax) {
	    positionDoubleLine(a->label, xmid-.1*(xmax-xmin),
			xmid+.1*(xmax-xmin), False);
	}
    }
    else if(type == ADD_CROSSHAIR) {
	if(getCrosshairs(&a) == 0) addCrosshair();
	else if(a->scaled_x < xmin || a->scaled_x > xmax ||
		a->scaled_y < ymin || a->scaled_y > ymax)
	{
	    positionCrosshair(0, xmid, ymid, false);
	}
    }
    else if(type == REMOVE_ALL_CURSORS) {
	deleteAllCursors();
    }
}

void WaveformPlot::dataMovement(DataMovementType type)
{
    Arg args[1];

    if(type == DATA_NO_MOVEMENT) {
	XtSetArg(args[0], XtNdataMovement, CPLOT_NO_MOVEMENT);
    }
    else if(type == DATA_XY_MOVEMENT) {
	XtSetArg(args[0], XtNdataMovement, CPLOT_XY_MOVEMENT);
    }
    else if(type == DATA_X_MOVEMENT) {
	XtSetArg(args[0], XtNdataMovement, CPLOT_X_MOVEMENT);
    }
    else if(type == DATA_Y_MOVEMENT) {
	XtSetArg(args[0], XtNdataMovement, CPLOT_Y_MOVEMENT);
    }
    else {
	return;
    }
    setValues(args, 1);
}

void WaveformPlot::displayArrivals(DisplayArrivalsType type)
{
    Arg args[1];

    if(type == ARRIVALS_OFF) {
	XtSetArg(args[0], XtNdisplayArrivals, CPLOT_ARRIVALS_OFF);
    }
    else if(type == ON_ONE_CHANNEL) {
	XtSetArg(args[0], XtNdisplayArrivals, CPLOT_ARRIVALS_ONE_CHAN);
    }
    else if(type == ON_ALL_CHANNELS) {
	XtSetArg(args[0], XtNdisplayArrivals, CPLOT_ARRIVALS_ALL_CHAN);
    }
    else {
	return;
    }
    setValues(args, 1);
}

void WaveformPlot::displayAllData(bool all)
{
    int mode = (all) ? 0 : 1;
    displayData(mode);
}

void WaveformPlot::promoteWaveforms()
{
    int i;
    gvector<Waveform *> wvec, ws;

    if(getWaveforms(wvec) <= 0) return;

    Waveform::sortByY0(wvec);

    for(i = 0; i < wvec.size(); i++) {
	if(wvec[i]->selected) ws.push_back(wvec[i]);
    }
    for(i = 0; i < wvec.size(); i++) {
	if(!wvec[i]->selected) ws.push_back(wvec[i]);
    }

    setYLabelInt(True, False);
    setWaveformOrder(ws);
}

void WaveformPlot::selectWaveforms(SelectWaveformType type)
{
    if(type == SELECT_ALL) {
	selectAll(True, False, CPLOT_ALL_COMPONENTS);
    }
    else if(type == DESELECT_ALL) {
	selectAll(False, False, CPLOT_ALL_COMPONENTS);
    }
    else if(type == SELECT_VISIBLE) {
	selectAll(True, True, CPLOT_ALL_COMPONENTS);
    }
    else if(type == SELECT_VISIBLE_Z) {
	selectAll(True, True, CPLOT_Z_COMPONENT);
    }
    else if(type == SELECT_VISIBLE_HORZ) {
	selectAll(True, True, CPLOT_HORIZONTAL_COMPONENTS);
    }
}

void WaveformPlot::setDefaultOrder(void)
{
    gvector<Waveform *> wvec;

    if(getWaveforms(wvec) <= 0) return;

    Waveform::sortByY0(wvec);

    for(int i = 0; i < wvec.size(); i++) {
	wvec[i]->default_order = i;
    }

    setYLabelInt(true, false);
    setWaveformOrder(wvec);

//    refreshHistoryTable(widget);
//    ListWaveformInstruments(widget);
}

void WaveformPlot::sortWaveforms(SortWaveformType type)
{
    gvector<Waveform *> wvec;

    if( getWaveforms(wvec) <= 0) return;

    sortWaveforms(type, wvec);

    setYLabelInt(true, false);

    setWaveformOrder(wvec);

    sort_type = type;

    change.sort_waveforms = true;
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

void WaveformPlot::sortWaveforms(SortWaveformType type,
				gvector<Waveform *> &wvec)
{
    if(type == SORT_DEFAULT_ORDER) {
	Waveform::sortByDefaultOrder(wvec);
    }
    else if(type == SORT_FILE_ORDER) {
	Waveform::sortByFileOrder(wvec);
    }
    else if(type == SORT_DISTANCE) {
	CssOriginClass *o;
	double *lat = new double[wvec.size()];
	double *lon = new double[wvec.size()];
	for(int i = 0; i < wvec.size(); i++) {
	    if((o= getPrimaryOrigin(wvec[i])) != NULL) {
		lat[i] = o->lat;
		lon[i] = o->lon;
	    }
	    else {
		lat[i] = -999.;
		lon[i] = -999.;
	    }
	}
	Waveform::sortByDistance(wvec, lat, lon);
	delete [] lat;
	delete [] lon;
    }
    else if(type == SORT_DISTANCE_FROM) {
	double *lat = new double[wvec.size()];
	double *lon = new double[wvec.size()];
	for(int i = 0; i < wvec.size(); i++) {
	    lat[i] = sort_from_lat;
	    lon[i] = sort_from_lon;
	}
	Waveform::sortByDistance(wvec, lat, lon);
	delete [] lat;
	delete [] lon;
    }
    else if(type == SORT_TIME_STA_CHAN) {
	Waveform::sortByTime(wvec);
    }
    else if(type == SORT_STA_CHAN) {
	Waveform::sortBySta(wvec);
    }
    else if(type == SORT_CHAN2_STA) {
	Waveform::sortByChan2(wvec);
    }
    else if(type == SORT_CHAN_STA) {
	Waveform::sortByChan(wvec);
    }
    else if(type == SORT_BACK_AZIMUTH) {
	CssOriginClass *o;
	double *lat = new double[wvec.size()];
	double *lon = new double[wvec.size()];
	for(int i = 0; i < wvec.size(); i++) {
	    if((o= getPrimaryOrigin(wvec[i])) != NULL) {
		lat[i] = o->lat;
		lon[i] = o->lon;
	    }
	    else {
		lat[i] = -999.;
		lon[i] = -999.;
	    }
	}
	Waveform::sortByBaz(wvec, lat, lon);
	delete [] lat;
	delete [] lon;
    }
}

void WaveformPlot::clear(bool do_callbacks)
{
    hold_data_change_callbacks = true;
    CPlotClass::clear(true);
    hold_data_change_callbacks = false;
    if(do_callbacks) {
	doDataChangeCallbacks();
    }
    if(data_source) {
	data_source->removeDataReceiver(this);
	data_source->removeOwner(this);
	data_source = NULL;
    }
    Application::getApplication()->undoManager()->removeAll();
}

void WaveformPlot::deleteOne(Waveform *w)
{
    hold_data_change_callbacks = true;
    gvector<Waveform *> v(w);
    deleteWaveforms(v);
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

void WaveformPlot::deleteSelectedWaveforms(void)
{
    gvector<Waveform *> wvec;

    hold_data_change_callbacks = true;
    if(getSelectedWaveforms(wvec) > 0) {
        deleteWaveforms(wvec);
    }
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

void WaveformPlot::deleteWaveforms(gvector<Waveform *> &wvec)
{
    hold_data_change_callbacks = true;
    TtPlotClass::deleteWaveforms(wvec);

    if(time_align == ALIGN_TRUE_TIME)
    {
	Arg args[1];
	int time_scale;
	XtSetArg(args[0], XtNtimeScale, &time_scale);
	getValues(args, 1);
	if(time_scale != TIME_SCALE_HMS && !time_scale_epoch) {
	    double epoch_min=0.;
	    getWaveforms(wvec);
	    if(wvec.size() > 0) {
		epoch_min = wvec[0]->tbeg();
	    }
	    for(int i = 1; i < wvec.size(); i++) {
		double t = wvec[i]->tbeg();
		if(t < epoch_min) epoch_min = t;
	    }
	    for(int i = 0; i < wvec.size(); i++) {
		double t = wvec[i]->tbeg();
		wvec[i]->scaled_x0 = t - epoch_min;
	    }
	}
    }
    if(!tt_curves_on) {
	sortWaveforms(sort_type);
    }
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

// don't need this?? should be in the DataSource (like BasicSource)
void WaveformPlot::addStations(int num, SegmentInfo **s)
{
/*
    // should change gnetAddStations
    WaveformList **list=NULL, *wavelist=NULL;
    WaveformList waveform_list_null = WAVEFORM_LIST_NULL;

    wavelist = (WaveformList *)mallocWarn(num*sizeof(WaveformList));
    list = (WaveformList **)mallocWarn(num*sizeof(WaveformList *));

    for(int i = 0; i < num; i++)
    {
	list[i] = &wavelist[i];
	memcpy(list[i], &waveform_list_null, sizeof(WaveformList));
	strcpy(list[i]->sta, s[i]->sta);
	strcpy(list[i]->chan, s[i]->chan);
	strcpy(list[i]->net, s[i]->net);
	strcpy(list[i]->refsta, s[i]->refsta);
	list[i]->station_lat = s[i]->station_lat;
	list[i]->station_lon = s[i]->station_lon;
	list[i]->station_elev = s[i]->station_elev;
	list[i]->origin_time = s[i]->origin_time;
    }
    gnetAddStations(num, list);

    Free(list);
    Free(wavelist);
*/
}

void WaveformPlot::setTTCurves(bool on)
{
    Arg args[1];
    XtSetArg(args[0], XtNdisplayTtCurves, (bool)on);
    setValues(args, 1);
    tt_curves_on = on;
}

void WaveformPlot::modifyWaveforms(gvector<Waveform *> &wvec)
{
    int i;
    char **new_tags = NULL;

    if(wvec.size() <= 0) return;

    new_tags =(char **)malloc(wvec.size()*sizeof(char *));

    for(i = 0; i < wvec.size(); i++)
    {
	Waveform *w = wvec[i];
	new_tags[i] = Waveform::getTag(w->ts->tag, w->sta(), w->chan(),
			getPrimaryOrigin(w), w->tbeg(), w->lat(),
			w->lon(), w->ts);

    }
    modify(wvec, new_tags, true);
    for(i = 0; i < wvec.size(); i++) Free(new_tags[i]);
    Free(new_tags);
    if(wt.uniform_scale->state() && numWaveforms() == wvec.size()) { 
	setUniformScale(true);
    }
    hold_data_change_callbacks = false;
    doDataChangeCallbacks();
}

// static
void WaveformPlot::resetWaveformColors(void)
{
    Free(waveform_colors);
    num_waveform_colors = 0;
}

int WaveformPlot::numWaveformColors(void)
{
    if(num_waveform_colors <= 0) {
	Free(waveform_colors);
	num_waveform_colors = getWaveformColors(this, &waveform_colors);
    }
    return num_waveform_colors;
}

Pixel WaveformPlot::waveformColor(int i)
{
    if(i < 0) i = 0;
    i = i % numWaveformColors();

    return waveform_colors[i];
}

int WaveformPlot::getWaveformColors(Component *comp, Pixel **fg)
{
    string color;
    int num=0;

    if( (num = getProperty("num_waveform_colors", 0)) )
    {
	*fg = (Pixel *)mallocWarn(num*sizeof(Pixel));

	for(int i = 0; i < num; i++) {
	    char s[20];
	    snprintf(s, sizeof(s), "waveform_color%d", i+1);
	    if( getProperty(s, color) ) {
		(*fg)[i] = comp->stringToPixel(color);
	    }
	    else {
		(*fg)[i] = comp->stringToPixel("black");
	    }
	}
    }
    else {
	num = 12;
	*fg = (Pixel *)mallocWarn(num*sizeof(Pixel));
	(*fg)[0]  = comp->stringToPixel("forest green");
	(*fg)[1]  = comp->stringToPixel("sky blue");
	(*fg)[2]  = comp->stringToPixel("orange");
	(*fg)[3]  = comp->stringToPixel("brown");
	(*fg)[4]  = comp->stringToPixel("MediumOrchid");
	(*fg)[5]  = comp->stringToPixel("red");
	(*fg)[6]  = comp->stringToPixel("thistle");
	(*fg)[7]  = comp->stringToPixel("sea green");
	(*fg)[8]  = comp->stringToPixel("tan");
	(*fg)[9]  = comp->stringToPixel("maroon");
	(*fg)[10] = comp->stringToPixel("slate blue");
	(*fg)[11] = comp->stringToPixel("grey");
    }
    return num;
}

void WaveformPlot::setColorCode(const string &color)
{
    color_code.assign(color);

    int i, j;
    gvector<Waveform *> wvec;
    CssOriginClass **o = NULL;
    Pixel *fg = NULL;

    if(getWaveforms(wvec) <= 0) return;

    fg = (Pixel *)mallocWarn(wvec.size()*sizeof(Pixel));

    for(i = 0; i < wvec.size(); i++) fg[i] = wvec[i]->fg;

    if(!color_code.compare("uniform")) /* all the same color */
    {
	if((int)fg[0] == -1) {
	    fg[0] = nextColor(wvec.size(), fg);
	}
	for(i = 1; i < wvec.size(); i++) fg[i] = fg[0];

    }
    else if(!color_code.compare("unique")) /* all different */
    {
	if((int)fg[0] == -1) {
	    fg[0] = nextColor(wvec.size(), fg);
	}
	for(i = 1; i < wvec.size(); i++)
	{
	    for(j = 0; j < i; j++) if(fg[j] == fg[i])
	    {
		fg[i] = nextColor(wvec.size(), fg);
	    }
	}
    }
    else if(!color_code.compare("station"))
    {
	for(i = 0; i < wvec.size(); i++)
	{
	    for(j = 0; j < i; j++) {
		if(!strcmp(wvec[j]->sta(), wvec[i]->sta())) {
		    fg[i] = fg[j];
		    break;
		}
	    }
	    if(j == i) uniqueColor(i, wvec.size(), fg);
	}
    }
    else if(!color_code.compare("channel"))
    {
	for(i = 0; i < wvec.size(); i++)
	{
	    for(j = 0; j < i; j++) {
		if(!strcmp(wvec[j]->chan(), wvec[i]->chan())) {
		    fg[i] = fg[j];
		    break;
		}
	    }
	    if(j == i) uniqueColor(i, wvec.size(), fg);
	}
    }
    else if(!color_code.compare("network"))
    {
	for(i = 0; i < wvec.size(); i++)
	{
	    for(j = 0; j < i; j++) {
		if(!strcmp(wvec[j]->net(), wvec[i]->net())) {
		    fg[i] = fg[j];
		    break;
		}
	    }
	    if(j == i) uniqueColor(i, wvec.size(), fg);
	}
    }
    else if(!color_code.compare("origin"))
    {
	o = new CssOriginClass * [wvec.size()];

	for(i = 0; i < wvec.size(); i++)
	{
	    o[i] = getPrimaryOrigin(wvec[i]);
	    if(o[i] != NULL) {
		for(j = 0; j < i; j++) {
		    if(o[j] != NULL && o[j]->getDC() == o[i]->getDC()
				&& o[j]->orid == o[i]->orid) {
			fg[i] = fg[j];
			break;
		    }
		}
		if(j == i) uniqueColor(i, wvec.size(), fg);
	    }
	    else uniqueColor(i, wvec.size(), fg);
	}
	delete[] o;
    }

    /* Look for waveforms which didn't get matched.
     */
    for(i = 0; i > wvec.size(); i++) if((int)fg[i] == -1)
    {
	fg[i] = nextColor(wvec.size(), fg);
    }

    colorWaveforms(wvec, fg, true);

    Free(fg);
}

void WaveformPlot::uniqueColor(int i, int num_waveforms, Pixel *fg)
{
    int j;

    if((int)fg[i] == -1)
    {
	fg[i] = nextColor(num_waveforms, fg);
    }
    else
    {
	if(fg[i] == 1)
	{
	    fg[i] = nextColor(i, fg);
	    return;
	}

	for(j = 0; j < i; j++)
	{
	    if(fg[j] == fg[i]) break;
	}
	if(j < i)
	{
	    fg[i] = nextColor(i, fg);
	}
    }
}

void WaveformPlot::beamWaveforms(double az, double slowness, int order,
	const string &ftype, double flo, double fhi, int zp,
	BeamLocation beam_location, bool replace, WaveformPlot *dest)
{
    gvector<Waveform *> wvec;

    if(getSelectedWaveforms(wvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }
    beamWaveforms(wvec, az, slowness, order, ftype, flo, fhi, zp,
			beam_location, replace, dest);
}

void WaveformPlot::beamWaveforms(gvector<Waveform *> &wvec,
	double az, double slowness, int order, const string &ftype, double flo,
	double fhi, int zp, BeamLocation beam_location, bool replace,
	WaveformPlot *dest)
{
    vector<double> t_lags, weights;
    double beam_lat, beam_lon;

    for(int i = 0; i < wvec.size(); i++) weights.push_back(1.);

    if( !Beam::getTimeLags(this, wvec, az, slowness, beam_location, t_lags,
			&beam_lat, &beam_lon) )
    {
	showWarning(GError::getMessage());
	return;
    }

    IIRFilter *iir = NULL;

    if( strcasecmp(ftype.c_str(), "NA") ) {
	iir = new IIRFilter(order, ftype, flo, fhi,
			wvec[0]->segment(0)->tdel(), zp);
	iir->addOwner(this);
    }

    if( !dest ) dest = this;

    setCursor("hourglass");

    computeBeam(wvec, t_lags, beamChanName(), beam_lat, beam_lon, iir,
		weights, replace, false, dest);

    if(iir) iir->removeOwner(this);

    setCursor("default");
}

void WaveformPlot::computeFtrace(double az, double slowness, double window_len,
	double snr, int order, double flo, double fhi, bool zp,
	BeamLocation beam_location, bool replace, WaveformPlot *dest)
{
    gvector<Waveform *> wvec;

    if(getSelectedWaveforms(wvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }
    computeFtrace(wvec, az, slowness, window_len, snr, order, flo, fhi, zp,
			beam_location, replace, dest);
}

void WaveformPlot::computeFtrace(gvector<Waveform *> &wvec, double az,
	double slowness, double window_len, double snr, int order, double flo,
	double fhi, bool zp, BeamLocation beam_location, bool replace,
	WaveformPlot *dest)
{
    int spts;
    double beam_lat, beam_lon;
    vector<double> t_lags;
    GTimeSeries *ts, beam_ts, semb_ts, prob_ts;

    if(wvec.size() <= 0) return;
    if( !Beam::getTimeLags(this, wvec, az, slowness, beam_location, t_lags,
			&beam_lat, &beam_lon) )
    {
	showWarning(GError::getMessage());
	return;
    }
    spts = (int)(window_len/wvec[0]->segment(0)->tdel() + .5) + 1;
    spts = (spts-1)/2;
    if(2*(spts/2) != spts) spts++;
    ts = new GTimeSeries();
    Beam::ftrace(wvec, wvec[0]->tbeg(), wvec[0]->tend(), t_lags, spts,
		order, flo, fhi, zp, snr, &beam_ts, &semb_ts, ts, &prob_ts);
    ts->makeCopy();
    ts->setLat(beam_lat);
    ts->setLon(beam_lon);

    displayBeam(ts, wvec, "ftrace", NULL, replace, false, dest);

    ts->deleteObject();
}

const char * WaveformPlot::beamChanName(void)
{
    char s[50];
    beam_index++;
    snprintf(s, sizeof(s), "bm%02d", beam_index);
    beam_chan_name.assign(s);
    return beam_chan_name.c_str();
}

void WaveformPlot::computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		bool replace, bool scroll, WaveformPlot *dest)
{
    vector<double> weights;
    computeBeam(wvec, t_lag, beam_name, beam_lat, beam_lon,
		(IIRFilter *)NULL, weights, replace, scroll, dest);
}

void WaveformPlot::computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		IIRFilter *iir, vector<double> &weights, bool replace,
		bool scroll, WaveformPlot *dest)
{
    GTimeSeries *ts;

    if( !(ts = Beam::BeamTimeSeries(wvec, t_lag, weights, true)) ) {
	return;
    }
    ts->makeCopy();
    ts->setLat(beam_lat);
    ts->setLon(beam_lon);

    displayBeam(ts, wvec, beam_name, iir, replace, scroll, dest);

    ts->deleteObject();
}

void WaveformPlot::computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		double tbeg, double tend, bool replace,
		bool scroll, WaveformPlot *dest)
{
    vector<double> weights;
    computeBeam(wvec, t_lag, beam_name, beam_lat, beam_lon, tbeg, tend,
		(IIRFilter *)NULL, weights, replace, scroll, dest);
}

void WaveformPlot::computeBeam(gvector<Waveform *> &wvec, vector<double> &t_lag,
		const string &beam_name, double beam_lat, double beam_lon,
		double tbeg, double tend, IIRFilter *iir,
		vector<double> &weights, bool replace, bool scroll,
		WaveformPlot *dest)
{
    GTimeSeries *ts;

    if( !(ts = Beam::BeamSubSeries(wvec, t_lag, weights, tbeg, tend, true)) ) {
	return;
    }
    ts->makeCopy();
    ts->setLat(beam_lat);
    ts->setLon(beam_lon);

    displayBeam(ts, wvec, beam_name, iir, replace, scroll, dest);

    ts->deleteObject();
}

extern "C" {
static int sort_by_y(const void *A, const void *B);
};

void WaveformPlot::displayBeam(GTimeSeries *ts, gvector<Waveform *> &wvec,
		const string &beam_name, IIRFilter *iir, bool replace,
		bool scroll, WaveformPlot *dest)
{
    vector<bool> on;
    const char *net;
    int i, k, beam_pos = -1;
    gvector<Waveform *> ws_after, ws_before, v;
    Waveform *beam = NULL;
    Waveform *last_beam = NULL;
    CssOriginClass *origin = NULL;
    DataMethod *dm[3];

    if(wvec.size() <= 0) return;

    net = wvec[0]->net();
    if(net[0] == '\0') net = wvec[0]->sta();

    for(i = 0; i < wvec.size() &&
	    (origin = getPrimaryOrigin(wvec[i])) == NULL; i++);

    /* if replace beam, delete the last beam associated with this network
     * and this origin.
     */
    dest->getWaveforms(ws_before);
    if(replace)
    {
	for(i = 0; i < ws_before.size(); i++)
	{
	    if(ws_before[i]->ts->getValue("last_beam", &k) &&
		!strcasecmp(ws_before[i]->net(), net) && (origin == NULL ||
			origin == dest->getPrimaryOrigin(ws_before[i])))
	    {
		last_beam = ws_before[i];
		beam_pos = i;
		v.clear();
		v.push_back(last_beam);
		dest->deleteWaveforms_NoReposition(v);
		break;
	    }
	}
    }


    ts->setDataSource(wvec[0]->getDataSource());
    ts->copyWfdiscPeriods(wvec[0]->ts); // to addArrivals
    WfdiscPeriod *wp = ts->getWfdiscPeriod(-1.e+60);
    snprintf(wp->wf.sta, sizeof(wp->wf.sta), "%s", net);
    snprintf(wp->wf.chan, sizeof(wp->wf.chan), "%s", beam_name.c_str());
    ts->setSta(net);
    ts->setNet(net);
    ts->setChan(beam_name.c_str());
    ts->setComponent(1);

    beam = dest->addWaveform(ts);

    /* get all the waveforms, find the one just read in,
     * and position it correctly
     */
    dest->getWaveforms(ws_after, false);

    /* remove beam_flag from other waveforms in this network.
     */
    for(i = 0; i < ws_after.size()-1; i++)
    {
	if(!strcasecmp(net, ws_after[i]->net()) && (origin == NULL ||
		origin == dest->getPrimaryOrigin(ws_after[i])) &&
		ws_after[i]->ts->getValue("last_beam", &k))
	{
	    ws_after[i]->ts->removeValue("last_beam");
	}
    }
    beam->ts->putValue("last_beam", 1);
    beam->ts->beam_elements.clear();
    for(i = 0; i < (int)ts->beam_elements.size(); i++) {
	beam->ts->beam_elements.push_back(ts->beam_elements[i]);
    }

    if(origin != NULL) {
	dest->setPrimaryOrigin(beam, origin);
    }

    if(last_beam == NULL)
    {
	/* if the destination window is the same as the source, position beam.
	 *
	 * place the beam below the lowest waveform in the beam group
	 */
	ws_after.sort(sort_by_y);

	beam_pos = -1;
	for(k = ws_after.size()-2; k >= 0; k--)
	{
	    if(!strcasecmp(net, ws_after[k]->net()) && (origin == NULL ||
		origin == dest->getPrimaryOrigin(ws_after[k])))
	    {
		beam_pos = k+1;
		break;
	    }
	}
    }

    if(beam_pos >= 0)
    {
	ws_before.clear();
	for(i = 0; i < beam_pos; i++) {
	    ws_before.push_back(ws_after[i]);
	}
	ws_before.push_back(beam);
	// last element is the beam
	for(i = beam_pos; i < ws_after.size()-1; i++) {
	    ws_before.push_back(ws_after[i]);
	}
	dest->setWaveformOrder(ws_before);
	dest->setDefaultOrder();
    }
    ws_after.clear();
    ws_after.push_back(beam);
    on.push_back(true);

    dest->setDataDisplay(ws_after, on, false, true);

    if(iir != NULL) {
	dm[0] = new Demean();
	dm[1] = new TaperData("cosine", 10, 5, 200);
	dm[2] = iir;
	DataMethod::changeMethods(3, dm, beam);
    }
    dest->modifyWaveform(beam);

    if(scroll && !beam->visible && dest != this) {
	dest->scrollBottom(beam);
    }
}

static int
sort_by_y(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    if((*a)->scaled_y0  != (*b)->scaled_y0)
    {
	return(((*a)->scaled_y0 < (*b)->scaled_y0) ? -1 : 1);
    }
    return(0);
}

void WaveformPlot::readRecentInput(RecentInput *r)
{
    if(!strcasecmp(r->format.c_str(), "css") || r->format.empty())
    {
 	if( !table_source ) table_source = new TableSource("WaveformPlot");

	if( !table_source->openPrefix(r->file) ) {
	    return;
	}

	table_source->queryAllPrefixTables();

	if( !r->read_all ) {
	    table_source->selectTables(cssWfdisc, r->wavs);
	}

	gvector<SegmentInfo *> *seginfo = table_source->getSegmentList();

	if( !seginfo ) {
	    struct stat buf;
	    if(stat(r->file.c_str(), &buf) < 0) {
		showWarning("%s not found.", r->file.c_str());
	    }
	    return;
	}

	int n = 0;
	for(int i = 0; i < (int)seginfo->size(); i++) {
	    if(seginfo->at(i)->selected) n++;
	}

	if( n ) {
	    inputData(seginfo, table_source, true);
	}
	delete seginfo;
	table_source->clearTable("all");
    }
    else if(!strcasecmp(r->format.c_str(), "sac")) {
	SacSource *s = new SacSource("sac_source", r->file);
	setDataSource(s);
    }
    else if(!strcasecmp(r->format.c_str(), "seed")) {
	SeedSource *s = new SeedSource("seed_source", r->file);
	setDataSource(s);
    }
    else if(!strcasecmp(r->format.c_str(), "gse")) {
	GseSource *s = new GseSource("gse_source", r->file);
	setDataSource(s);
    }
    else if(stringCaseEndsWith(r->file.c_str(), ".sac")) {
	SacSource *s = new SacSource("sac_source", r->file);
	setDataSource(s);
    }
    else if(stringCaseEndsWith(r->file.c_str(), ".seed")) {
	SeedSource *s = new SeedSource("seed_source", r->file);
	setDataSource(s);
    }
    else if(stringCaseEndsWith(r->file.c_str(), ".msg") ||
		stringCaseEndsWith(r->file.c_str(),".gse"))
    {
	GseSource *s = new GseSource("gse_source", r->file);
	setDataSource(s);
    }
    else if(stringCaseEndsWith(r->file.c_str(), ".asc")) {
	AscSource *s = new AscSource("asc_source", r->file);
	setDataSource(s);
    }
    else if(SeedSource::isSeedFile(r->file)) {
	SeedSource *s = new SeedSource("seed_source", r->file);
	setDataSource(s);
    }
    else if(AscSource::isAscFile(r->file)) {
	AscSource *s = new AscSource("asc_source", r->file);
	setDataSource(s);
    }
    else {
	setPrintError(false);
	// try GSE message
	GseSource *s = new GseSource("gse_source", r->file);
	setDataSource(s);
	if( !numWaveforms() ) {
	    // try SAC
	    SacSource *ss = new SacSource("sac_source", r->file);
	    setDataSource(ss);
	}
	setPrintError(true);
    }
}

bool RecentInput::equals(RecentInput *r)
{
    if(read_all != r->read_all || chan_list.size() != r->chan_list.size()
	|| wavs.size() != r->wavs.size()) return false;

    if(file.compare(r->file)) return false;

    for(int i = 0; i < (int)chan_list.size(); i++) {
	if(strcasecmp(chan_list[i].c_str(), r->chan_list[i].c_str())) {
	    return false;
	}
    }
    for(int i = 0; i < (int)wavs.size(); i++) {
	if(wavs[i] != r->wavs[i]) return false;
    }
    return true;
}

void WaveformPlot::printWindow(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Waveforms", formDialogParent(),
					this);
    }
    print_window->setVisible(true);
}

void WaveformPlot::print(FILE *fp, PrintParam *p)
{
    AxesParm *a;

    p->right -= 1.;
    if((a = hardCopy(fp, p, NULL))) {
	free(a);
    }
}

void WaveformPlot::changeWaveformTag(void)
{
    if( !tag_contents ) {
	tag_contents = new TagContents("Tag Contents", formDialogParent(),
			Waveform::numMemberNames(), Waveform::memberNames(),
			num_tag_members, tag_members, user_tag_string);
	tag_contents->addActionListener(this);
    }
    tag_contents->setVisible(true);
}

void WaveformPlot::unfilter(bool all, Waveform *w)
{
    const char *methods[3];
    gvector<Waveform *> wvec, ws;

    if(w) {
	wvec.push_back(w);
    }
    else if(all) {
	getWaveforms(wvec);
    }
    else {
	getSelectedWaveforms(wvec);
    }

    methods[0] = "Demean";
    methods[1] = "TaperData";
    methods[2] = "IIRFilter";

    for(int i = 0; i < wvec.size(); i++) {
	if(DataMethod::remove(3, methods, wvec[i])) {
	    ws.push_back(wvec[i]);
	}
    }

    modifyWaveforms(ws);
}

void WaveformPlot::displayCalib(bool calib_on)
{
    gvector<Waveform *> wvec, ws;

    if( !getSelectedWaveforms(wvec) ) {
	showWarning("No waveforms selected.");
	return;
    }

    for(int i = 0; i < wvec.size(); i++) {
	if(changeCalib(wvec[i], calib_on))
	{
	    ws.push_back(wvec[i]);
	}
    }

    modifyWaveforms(ws);
}

bool WaveformPlot::changeCalib(Waveform *w, bool apply)
{
    if(apply)
    {
	if(!w->ts->getMethod("CalibData")) {
	    CalibData *cal = new CalibData();
	    cal->apply(w);
	    return true;
	}
    }
    else {
	if(w->ts->getMethod("CalibData")) {
	    gvector<Waveform *> v(w);
	    DataMethod::remove("CalibData", v);
	    return true;
	}
    }
    return false;
}

void WaveformPlot::removeMean(bool demean, Waveform *w)
{
    gvector<DataMethod *> *methods;
    gvector<Waveform *> wvec, ws, v;

    if(w) {
	wvec.push_back(w);
    }
    else if( getSelectedWaveforms(wvec) <= 0 ) {
        showWarning("No waveforms selected.");
        return;
    }

    if(demean)
    {
	for(int i = 0; i < wvec.size(); i++) {
	    methods = wvec[i]->dataMethods();
	    if((int)methods->size()==0 || !methods->back()->getDemeanInstance())
	    {
		Demean *dm = new Demean();
		dm->apply(wvec[i]);
		ws.push_back(wvec[i]);
	    }
	    delete methods;
	}
    }
    else
    {
	for(int i = 0; i < wvec.size(); i++) {
	    if(wvec[i]->ts->getMethod("Demean")) {
		v.clear();
		v.push_back(wvec[i]);
		DataMethod::remove("Demean", v);
		ws.push_back(wvec[i]);
	    }
	}
    }
    if(ws.size()) modifyWaveforms(ws);
}

void WaveformPlot::polarity(bool normal, Waveform *w)
{
    bool polarity_change=false;
    gvector<Waveform *> wvec;
    AmpData *amp;

    if(w) {
	wvec.push_back(w);
    }
    else if( getSelectedWaveforms(wvec) <= 0 ) {
        showWarning("No waveforms selected.");
        return;
    }

    for(int i = 0; i < wvec.size(); i++) {
	gvector<DataMethod *> *methods = wvec[i]->dataMethods();
        bool found_one = false;
        for(int j = 0; !found_one && j < (int)methods->size(); j++)
        {
            if( (amp = methods->at(j)->getAmpDataInstance()) )
            {
                if(!strcmp(amp->getComment(), "polarity")) {
		    if(normal) {
			wvec[i]->ts->removeMethod(amp);
			polarity_change = true;
		    }
                    found_one = true;
                }
            }
        }
	delete methods;

        if(!found_one && !normal) {
            amp = new AmpData(-1., "polarity");
            amp->apply(wvec[i]);
	    polarity_change = true;
        }
    }
    if(polarity_change) modifyWaveforms(wvec);
}

ParseCmd WaveformPlot::parseRotate(Waveform *w, const string &cmd, string &msg)
{
    bool err, set_hang;
    string errstr;
    int i, j, n, ngroups;
    vector<int> ncmpts;
    gvector<Waveform *> wvec;
    gvector<Waveform *> ws;
    double azimuth, incidence, reverse_azimuth;

    if(!parseFind(cmd, "rotate", msg, &err, "_wave_", false,
	"azimuth", true, "incidence", false, "set_hang"))
    {
	return COMMAND_NOT_FOUND;
    }
    else if(err) {
	return ARGUMENT_ERROR;
    }
    if(!w) {
	msg.assign("rotate: missing wave object");
	return ARGUMENT_ERROR;
    }
    if(!parseGetArg(cmd, "rotate", msg, "azimuth", &azimuth)) {
        return ARGUMENT_ERROR;
    }
    incidence = 90.;
    parseGetArg(cmd, "rotate", msg, "incidence", &incidence);
    if(msg[0] != '\0') return ARGUMENT_ERROR;

    set_hang = true;
    parseGetArg(cmd, "rotate", msg, "set_hang", &set_hang);
    if(msg[0] != '\0') return ARGUMENT_ERROR;

    reverse_azimuth = azimuth + 180.;
    if(reverse_azimuth > 360.) reverse_azimuth -= 360.;

    ngroups = getComponents(ncmpts, wvec);

    if( !RotateData::checkComponents(set_hang, incidence, ncmpts, wvec, errstr))
    {
	if( !errstr.empty() ) putWarning(errstr.c_str());
	return ARGUMENT_ERROR;
    }
    if( !errstr.empty() ) putWarning(errstr.c_str());
    ngroups = ncmpts.size();

    errstr.clear();
    n = 0;
    for(j = 0; j < ngroups; j++) {
	for(i = 0; i < ncmpts[j]; i++) {
	    if(wvec[n] == w) break;
	}
	if(i < ncmpts[j]) {
	    ws.clear();
	    ws.push_back(wvec[n]);
	    ws.push_back(wvec[n+1]);
	    if(ncmpts[j] == 2) {
		RotateData::rotateWaveforms(reverse_azimuth, wvec[n], wvec[n+1],
				errstr);
	    }
	    else if(ncmpts[j] == 3) {
		ws.push_back(wvec[n+2]);
		RotateData::rotateWaveforms(reverse_azimuth, incidence,
                                wvec[n], wvec[n+1], wvec[n+2], errstr);
	    }
	    if( !errstr.empty() ) putWarning(errstr.c_str());
	    modifyWaveforms(ws);
	    return COMMAND_PARSED;
	}
    }
    msg.assign("rotate: cannot find waveforms");
    return ARGUMENT_ERROR;
}

ParseCmd WaveformPlot::parseUnrotate(Waveform *w, const string &cmd,string &msg)
{
    bool err;
    int i, j, n, ngroups;
    vector<int> ncmpts;
    gvector<Waveform *> wvec, ws;

    if(!parseFind(cmd, "unrotate", msg, &err, "_wave_", false))
    {
	return COMMAND_NOT_FOUND;
    }
    else if(err) {
	return ARGUMENT_ERROR;
    }
    if(!w) {
        msg.assign("unrotate: missing wave object");
        return ARGUMENT_ERROR;
    }

    ngroups = getComponents(ncmpts, wvec);

    n = 0;
    for(j = 0; j < ngroups; j++) {
	for(i = 0; i < ncmpts[j]; i++) {
	    if(wvec[n] == w) break;
	}
	if(i < ncmpts[j]) {
	    ws.clear();
	    ws.push_back(wvec[n]);
	    ws.push_back(wvec[n+1]);
	    if(ncmpts[j] == 3) {
		ws.push_back(wvec[n+2]);
	    }

	    DataMethod::remove("RotateData", ws);

	    for(i = 0; i < ncmpts[j]; i++) {
		ws[i]->ts->getChan(ws[i]->channel);
	    }
	    modifyWaveforms(ws);

	    return COMMAND_PARSED;
	}
    }
    msg.assign("rotate: cannot find waveforms");
    return ARGUMENT_ERROR;
}

void WaveformPlot::setUniformAmpScale(bool set)
{
    if(amplitude_scale) {
	// synchronize with the Uniform toggle in the Amplitude Scale window
	amplitude_scale->uniformToggle()->set(set, false);
	amplitude_scale->independentToggle()->set(!set, false);
    }
    setUniformScale(set);
}

void WaveformPlot::setAmplitudeScale(bool visible)
{
    int dataHeight, dataSeparation;
    char text[20];

    if(!amplitude_scale) {
	amplitude_scale = new AmplitudeScale("Amplitude Scale",
					formDialogParent());
	amplitude_scale->addActionListener(this);
    }
    if(visible) {
	amplitude_scale->setVisible(true);
    }

    dataHeight = getDataHeight();
    dataSeparation = getDataSeparation();

    snprintf(text, sizeof(text), "%d", dataHeight);
    amplitude_scale->setDataHeight(text);

    snprintf(text, sizeof(text), "%d", dataSeparation);
    amplitude_scale->setDataSeparation(text);

    amplitude_scale->uniformToggle()->set(wt.uniform_scale->state(), false);
    amplitude_scale->independentToggle()->set(!wt.uniform_scale->state(),false);
}

// Callback from the Amplitude Scale Window
void WaveformPlot::amplitudeScale(int calldata)
{
    int data_height, data_separation;

    if(calldata == APPLY_LAYOUT)	// Apply Layout button
    {
	char *c = amplitude_scale->getDataHeight();
	if(!stringToInt(c, &data_height) || data_height <= 0) {
	    amplitude_scale->showWarning(
			"Enter a positive integer for the Data Height.");
	    Free(c);
	    return;
	}
	Free(c);
	c = amplitude_scale->getDataSeparation();
	if(!stringToInt(c, &data_separation) || data_separation <= 0) {
	    amplitude_scale->showWarning(
			"Enter a positive integer for the Data Separation.");
	    Free(c);
	    return;
	}
	Free(c);

	setDataHeight(data_height, data_separation);
    }
    else if(calldata == APPLY_SCALE)	// Apply Scale button
    {
	char *c = amplitude_scale->getScaleDataHeight();
	if(!stringToInt(c, &data_height) || data_height <= 0) {
	    amplitude_scale->showWarning(
			"Enter a positive integer for the Data Height.");
	    Free(c);
	    return;
	}
	Free(c);

	bool all = (bool)amplitude_scale->getAllState();
	bool visible = (bool)amplitude_scale->getVisibleState();

	setScale(data_height, visible, all);
    }
    else if(calldata == SET_TYPE_INDEPENDENT) { // Independent toggle set on
	wt.uniform_scale->set(false, true);
    }
    else if(calldata == SET_TYPE_UNIFORM) { // Uniform toggle set on
	wt.uniform_scale->set(true, true);
    }
}

void WaveformPlot::syncToggles(bool inside_to_outside)
{
    vector<Component *> *in, *out;

    if( !menu_popup || (!outside_edit_menu && !outside_view_menu)) return;

    if(inside_to_outside)
    {
	in = menu_popup->getChildren();

	if(outside_edit_menu) {
	    out = outside_edit_menu->getChildren();
	    syncToggleChildren(in, out);
	    delete out;
	}
	if(outside_view_menu) {
	    out = outside_view_menu->getChildren();
	    syncToggleChildren(in, out);
	    delete out;
	}

	delete in;
    }
    else {
	out = menu_popup->getChildren();

	if(outside_edit_menu) {
	    in = outside_edit_menu->getChildren();
	    syncToggleChildren(in, out);
	    delete in;
	}
	if(outside_view_menu) {
	    in = outside_view_menu->getChildren();
	    syncToggleChildren(in, out);
	    delete in;
	}

	delete out;
    }
}

void WaveformPlot::syncToggleChildren(vector<Component *> *in,
			vector<Component *> *out)
{
    Toggle *t_in, *t_out;
    Menu *menu;

    for(int i = 0; i < (int)in->size(); i++)
    {
	if( (t_in = in->at(i)->getToggleInstance()) )
	{
	    for(int j = 0; j < (int)out->size(); j++)
	    {
		if( (t_out = out->at(j)->getToggleInstance()) ) {
		    if(!strcmp(t_in->getName(), t_out->getName())) {
			t_out->set(t_in->state(), false);
			break;
		    }
		}
		else if( (menu = out->at(j)->getMenuInstance()) ) {
		    vector<Component *> *c = menu->getChildren();
		    syncToggleChildren(in, c);
		    delete c;
		}
	    }
	}
	else if( (menu = in->at(i)->getMenuInstance()) ) {
	    vector<Component *> *c = menu->getChildren();
	    syncToggleChildren(c, out);
	    delete c;
	}
    }
}

bool WaveformPlot::deleteOrigins(cvector<CssOriginClass> &origins)
{
    int i, j, dc;
    bool found_one = false;
    DataSource *ds;
    CssOriginClass *o;
    cvector<CssOrigerrClass> & origerrs = *getOrigerrRef();
    cvector<CssAssocClass>	& assocs = *getAssocRef();
    cvector<CssWftagClass>   & wftags = *getWftagRef();
    cvector<CssNetmagClass>  & netmags = *getNetmagRef();
    cvector<CssStamagClass>  & stamags = *getStamagRef();

    UndoDeleteOrigin *undo = new UndoDeleteOrigin(this);

    errorMsg(); // clear last error message
    bool err = false;

    for(i = 0; i < origins.size(); i++)
	if( (ds = origins[i]->getDataSource()) )
    {
	BasicSource::startBackup();

	o = origins[i];

	if(!ds->deleteOrigin(this, o, origerrs, assocs, wftags,netmags,stamags))
	{
	    err = true;
	    showErrorMsg();
	    continue;
	}

	undo->origins.push_back(o);
	removeTable(o, false);

	dc = o->getDC();

	for(j = 0; j < origerrs.size(); j++)
	    if(origerrs[j]->getDC() == dc && origerrs[j]->orid == o->orid)
	{
	    undo->origerrs.push_back(origerrs[j]);
	    removeTable(origerrs[j], false);
	}
	for(j = 0; j < assocs.size(); j++)
	    if(assocs[j]->getDC() == dc && assocs[j]->orid == o->orid)
	{
	    undo->assocs.push_back(assocs[j]);
	    removeTable(assocs[j], false);
	}
	for(j = 0; j < wftags.size(); j++)
	    if(wftags[j]->getDC() == dc &&
		!strcmp(wftags[j]->tagname, "orid") &&
		wftags[j]->tagid == o->orid)
	{
	    undo->wftags.push_back(wftags[j]);
	    removeTable(wftags[j], false);
	}
	for(j = 0; j < netmags.size(); j++)
	    if(netmags[j]->getDC() == dc && netmags[j]->orid == o->orid)
	{
	    undo->netmags.push_back(netmags[j]);
	    removeTable(netmags[j], false);
	}
	for(j = 0; j < stamags.size(); j++)
	    if(stamags[j]->getDC() == dc && stamags[j]->arid == o->orid)
	{
	    undo->stamags.push_back(stamags[j]);
	    removeTable(stamags[j], false);
	}
	found_one = true;
    }

    if(found_one) {
	Application::getApplication()->addUndoAction(undo);
    }
    else {
	delete undo;
	if( !err ) showWarning("No origin selected.");
    }
    return true;
}

bool WaveformPlot::undoDeleteOrigin(UndoDeleteOrigin *undo)
{
    int i, j, n, dc;
    DataSource *ds;

    n = 0;
    for(i = undo->origins.size()-1; i >= 0; i--)
	if( (ds = undo->origins[i]->getDataSource())
		&& ds->undoDeleteTable(undo->origins[i]))
    {
	long orid = undo->origins[i]->orid;
	dc = undo->origins[i]->getDC();

	putTable(undo->origins[i]);

	for(j = 0; j < undo->origerrs.size(); j++)
	    if(undo->origerrs[j]->getDC() ==dc && undo->origerrs[j]->orid==orid)
	{
	    putTable(undo->origerrs[j]);
	}
	for(j = 0; j < undo->assocs.size(); j++)
	    if(undo->assocs[j]->getDC() == dc && undo->assocs[j]->orid == orid)
	{
	    putTable(undo->assocs[j]);
	}
	for(j = 0; j < undo->wftags.size(); j++)
	    if(undo->wftags[j]->getDC() == dc && undo->wftags[j]->tagid == orid)
	{
	    putTable(undo->wftags[j]);
	}
	for(j = 0; j < undo->netmags.size(); j++)
	    if(undo->netmags[j]->getDC() == dc && undo->netmags[j]->orid ==orid)
	{
	    putTable(undo->netmags[j]);
	}
	for(j = 0; j < undo->stamags.size(); j++)
	    if(undo->stamags[j]->getDC() == dc && undo->stamags[j]->orid ==orid)
	{
	    putTable(undo->stamags[j]);
	}
	n++;
    }
    if( n != undo->origins.size() ) {
	showWarning("Error encountered while undoing delete arrival.");
    }
    if(n > 0) {
//	list();
    }
    return (n == undo->origins.size()) ? true : false;
}

void WaveformPlot::deleteTables(gvector<CssTableClass *> &t)
{
    DataSource *ds;

    if(t.size() <= 0) return;

    bool found_one=false;
    UndoDeleteTables *undo = new UndoDeleteTables(this);
    BasicSource::startBackup();
    for(int i = 0; i < t.size(); i++)
	if(( ds = t[i]->getDataSource()) )
    {
	found_one = true;
	TableListener::removeListener(t[i], this);
        ds->deleteTable(this, t[i]);
	undo->tables.push_back(t[i]);
	if(t[i]->nameIs("stamag") || t[i]->nameIs("netmag") ||
		t[i]->nameIs("amplitude"))
	{
	    removeTable(t[i], false);
	}
    }
    if(found_one) {
	Application::getApplication()->addUndoAction(undo);
    }
    else {
	delete undo;
    }
}

bool WaveformPlot::undoDeleteTables(UndoDeleteTables *undo)
{
    DataSource *ds;

    for(int i = undo->tables.size()-1; i >= 0; i--)
	if( (ds = undo->tables[i]->getDataSource())
		&& ds->undoDeleteTable(undo->tables[i]))
    {
	if(undo->tables[i]->nameIs("stamag")) {
	    putTable(undo->tables[i]);
	}
	else if(undo->tables[i]->nameIs("netmag")) {
	    putTable(undo->tables[i]);
	}
	else if(undo->tables[i]->nameIs("amplitude")) {
	    putTable(undo->tables[i]);
	}
    }
    return true;
}

void WaveformPlot::output(void)
{
    if(!output_window) {
	output_window = new Output("Save as", formDialogParent(), this);
    }
    output_window->setVisible(true);
}

void WaveformPlot::outputTables(const string &prefix, const string &access)
{
    output(prefix, access, *getArrivalRef());
    output(prefix, access, *getWftagRef());
    output(prefix, access, *getOriginRef());
    output(prefix, access, *getOrigerrRef());
    output(prefix, access, *getAssocRef());
    output(prefix, access, *getStassocRef());
    output(prefix, access, *getHydroFeaturesRef());
    output(prefix, access, *getInfraFeaturesRef());
    output(prefix, access, *getStamagRef());
    output(prefix, access, *getParrivalRef());
    output(prefix, access, *getNetmagRef());
    output(prefix, access, *getAmplitudeRef());
    output(prefix, access, *getAmpdescriptRef());
    output(prefix, access, *getFilterRef());
}

void WaveformPlot::output(const string &prefix, const string &access,
				gvector<CssTableClass *> &t)
{
    char path[MAXPATHLEN+1], *s;
    FILE *fp;

    if(t.size() <= 0) return;

    snprintf(path, sizeof(path), "%s.%s", prefix.c_str(), t[0]->getName());

    if( !(fp = fopen(path, access.c_str())) ) {
        showWarning("Cannot write to %s", path, strerror(errno));
	return;
    }
    for(int i = 0; i < t.size(); i++) {
	DateTime *dt = (DateTime *)t[i]->memberAddress("lddate");
	if(dt && t[i]->memberType("lddate") == CSS_LDDATE) {
	    timeEpochToDate(timeGetEpoch(), dt);
	}
	s = t[i]->toString();
	fprintf(fp, "%s\n", s);
	Free(s);
    }
    fclose(fp);
}
