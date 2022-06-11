/** \file AmpMag.cpp
 *  \brief Defines class AmpMag
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <pwd.h>
#include <dlfcn.h>
#include <string.h>

#include "AmpMag.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "MeasureAmpPer.h"
#include "gobject++/GTimeSeries.h"
#include "BasicSource.h"
#include "CalibData.h"
#include "IIRFilter.h"
#include "TaperData.h"
#include "libgio.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"

#include "ibase/libmagnitude.h"
#include "ibase/db_arrival.h"
#include "ibase/db_assoc.h"
#include "ibase/db_stamag.h"
#include "ibase/db_amplitude.h"
#include "ibase/db_netmag.h"
}

using namespace libgarrival;

static void fixAuth(char *auth, Password password, int len);
static void copyOrigin(Origin *origin, CssOriginClass *o);
static void copyNetmag(Netmag *netmag, CssNetmagClass *n);
static CssNetmagClass *createNetmag(Netmag *netmag);
static void copyStamag(Stamag *stamag, CssStamagClass *s);
static CssStamagClass *createStamag(Stamag *stamag);
static void copyAmplitude(Amplitude *amp, CssAmplitudeClass *a);
static void copyAssoc(Assoc *assoc, CssAssocClass *a);
static void copyParrival(Parrival *parrival, CssParrivalClass *p);
static int getMagTypes(char **magTypes);

static bool mag_lib_ready = false;
static Site *sites = NULL;

static void setTimeField(char *field, int len, char *format, double value);

class ReviewSource : public DataSource
{
    public:
	ReviewSource(void) {}
	~ReviewSource(void) {
	    clear();
	}
	void addData(GTimeSeries *ts)
	{
	    GTimeSeries *clone = new GTimeSeries(ts);
	    data.push_back(clone);
	    ts->putValue("review_source_id", clone);
	}
	void clear(void) {
	    for(int i = 0; i < (int)data.size(); i++) {
		data.at(i)->deleteObject();
	    }
	    data.clear();
	}
	bool reread(GTimeSeries *ts)
	{
	    GTimeSeries *clone;

	    if( (clone = (GTimeSeries *)ts->getValue("review_source_id")) ) {
		ts->removeAllSegments();
		for(int i = 0; i < clone->size(); i++) {
		    GSegment *s = new GSegment(clone->segment(i));
		    ts->addSegment(s);
		}
		return true;
	    }
	    cerr << "ReviewSource.reread: cannot re-read data." << endl;
	    return false;
	}

        vector<GTimeSeries *> data;
};

AmpMag::AmpMag(const char *name, Component *parent, DataSource *ds,
	WaveformPlot *wplot) : Frame(name, parent, true), DataReceiver(ds)
{
    if(data_source) {
	data_source->addDataListener(this);
    }
    wp = wplot;
    createInterface();
    init();
}

void AmpMag::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    print_button = new Button("Print", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    edit_button = new Button("Edit", edit_menu, this);
    cancel_button = new Button("Cancel Edit", edit_menu, this);
    cancel_button->setSensitive(false);
    delete_button = new Button("Delete Selected Records", edit_menu, this);
    filter_button = new Button("Filter...", edit_menu);
    save_button = new Button("Save Edit", edit_menu, this);
    save_button->setSensitive(false);
    undo_button = new UndoButton("Undo", edit_menu);
    undo_button->setSensitive(false);


    // View Menu
    view_menu = new Menu("View", menu_bar);
    amp_attributes_button = new Button("Amplitude Attributes...", view_menu,
				this);
    stamag_attributes_button = new Button("Stamag Attributes...", view_menu,
				this);
    netmag_attributes_button = new Button("Netmag Attributes...", view_menu,
				this);

    deselect_button = new Button("Deselect All", view_menu, this);

    sort_menu = new Menu("Sort", view_menu);
    amp_sort_button = new Button("Amplitude by amptype", sort_menu, this);
    stamag_sort_button = new Button("Stamag by sta,magtype,magid", sort_menu,
				this);
    netmag_sort_button = new Button("Netmag by amptype,magid", sort_menu, this);

    sort_cols_button = new Button("Sort on Selected Column(s)", view_menu,this);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    compute_button = new Button("Compute Magnitudes", option_menu, this);
    measure_ml_button = new Button("Measure ml Amplitude", option_menu, this);
    measure_mb_button = new Button("Measure mb Amplitude", option_menu, this);
    manual_button = new Button("Manual Measurement...", option_menu, this);
    review_button = new Button("Review Amplitude", option_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Amplitude Help", help_menu, this);

    delete_button->setSensitive(false);
    edit_button->setSensitive(false);
    compute_button->setSensitive(false);
    review_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNtableTitle, "amplitudes"); n++;
    XtSetArg(args[n], XtNvisibleRows, 5); n++;
    XtSetArg(args[n], XmNpaneMinimum, 200); n++;
    amp_table = new ctable<CssAmplitudeClass>("amp_table", pane, args, n);
//    amp_table->setType(cssAmplitude);
    amp_table->displayEditControls(false);
    amp_table->setVisible(true);
    amp_table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNtableTitle, "stamags"); n++;
    XtSetArg(args[n], XtNvisibleRows, 5); n++;
    XtSetArg(args[n], XmNpaneMinimum, 200); n++;
    stamag_table = new ctable<CssStamagClass>("stamag_table", pane, args, n);
//    stamag_table->setType(cssStamag);
    stamag_table->displayEditControls(false);
    stamag_table->setVisible(true);
    stamag_table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNtableTitle, "netmags"); n++;
    XtSetArg(args[n], XtNvisibleRows, 5); n++;
    XtSetArg(args[n], XmNpaneMinimum, 200); n++;
    netmag_table = new ctable<CssNetmagClass>("netmag_table", pane, args, n);
//    netmag_table->setType(cssNetmag);
    netmag_table->displayEditControls(false);
    netmag_table->setVisible(true);
    netmag_table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XtNheight, 250); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_SECONDS); n++;
    XtSetArg(args[n], XtNmeasureBox, True);
    plot1 = new WaveformView("plot1", pane, info_area, args, n);

    plot1->addActionListener(this, XtNmeasureCallback);
    plot1->addActionListener(this, XtNmodifyWaveformCallback);
    plot1->alignWaveforms(ALIGN_FIRST_POINT);
    string msg;
    plot1->parseCmd("Tag_Contents.Filter(low high)", msg);
    plot1->parseCmd("Tag_Contents.apply", msg);
    filter_button->addActionListener(plot1);

    measure_window = NULL;

    addPlugins("AmpMag", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(cancel_button, "Cancel");
	tool_bar->add(save_button, "Save");
	tool_bar->add(review_button, "Review Amplitude");
	tool_bar->add(compute_button, "Compute Magnitudes");
    }
}

void AmpMag::init(void)
{
    cvector<CssArrivalClass> a;
    data_source->getTable(a);
    bool set= (a.size() > 0);

    measure_ml_button->setSensitive(set);
    measure_mb_button->setSensitive(set);
    list();
    ignore_measure_cb = false;
    force_edit_on = false;
    review_source = new ReviewSource();

    string recipe_dir, recipe_dir2;
    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	char path[MAXPATHLEN+1];
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    snprintf(path, sizeof(path), "%s/tables/recipes", c);
	    recipe_dir.assign(path);
        }
    }
    getProperty("recipeDir2", recipe_dir2);

    gbeam = new Beam(recipe_dir, recipe_dir2);
}

AmpMag::~AmpMag(void)
{
    delete gbeam;
}

void AmpMag::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Amplitude Attributes...")) {
        amp_table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Stamag Attributes...")) {
        stamag_table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Netmag Attributes...")) {
        netmag_table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Compute Magnitudes")) {
	computeMagnitudes();
    }
    else if(!strcmp(cmd, "Measure ml Amplitude")) {
        measureAmplitude(ML_MEASURE);
    }
    else if(!strcmp(cmd, "Measure mb Amplitude")) {
        measureAmplitude(MB_MEASURE);
    }
    else if(!strcmp(cmd, "Edit")) {
	editOn();
    }
    else if(!strcmp(cmd, "Cancel Edit")) {
	editOff(false);
    }
    else if(!strcmp(cmd, "Save Edit")) {
	editOff(true);
    }
    else if(!strcmp(cmd, "Manual Measurement...")) {
	if(!measure_window) {
	    measure_window = new MeasureAmpPer("Measure Amp Per", this,
				data_source, wp);
	}
	measure_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Delete Selected Records")) {
	deleteSelected();
    }
    else if(!strcmp(reason, XtNselectRowCallback)) {
	selectRow((CSSTable *)comp);
    }
    else if(!strcmp(cmd, "Sort on Selected Column(s)")) {
	vector<int> cols;
	amp_table->getSelectedColumns(cols);
	if((int)cols.size()) amp_table->sortByColumns(cols);
	cols.clear();
	stamag_table->getSelectedColumns(cols);
	if((int)cols.size()) stamag_table->sortByColumns(cols);
	cols.clear();
	netmag_table->getSelectedColumns(cols);
	if((int)cols.size()) netmag_table->sortByColumns(cols);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	amp_table->selectAllRows(false);
	stamag_table->selectAllRows(false);
	netmag_table->selectAllRows(false);
    }
    else if(!strcmp(cmd, "Amplitude by amptype")) {
	amp_table->sortByColumnLabels("amptype");
    }
    else if(!strcmp(cmd, "Stamag by sta,magtype,magid")) {
	stamag_table->sortByColumnLabels("sta,magtype,magid");
    }
    else if(!strcmp(cmd, "Netmag by magtype,magid")) {
	stamag_table->sortByColumnLabels("magtype,magid");
    }
    else if(!strcmp(cmd, "Review Amplitude")) {
	amplitudeReview();
    }
    else if(!strcmp(reason, XtNmeasureCallback)) {
	if(!ignore_measure_cb) {
	    ampMeasure((CPlotMeasure *)action_event->getCalldata());
	}
    }
    else if(!strcmp(reason, XtNmodifyWaveformCallback)) {
	modifyReview((CPlotModifyCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNdataChangeCallback)) {
	DataChange *d = (DataChange *)action_event->getCalldata();
	if(d->stamag || d->netmag || d->amplitude) {
	    list();
	}
	if(d->select_arrival) {
	    cvector<CssArrivalClass> a;
	    data_source->getTable(a);
	    bool set= (a.size() > 0);
	    measure_ml_button->setSensitive(set);
	    measure_mb_button->setSensitive(set);
	}
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
	plot1->clear();
    }
    else if(!strcmp(cmd, "Amplitude Help")) {
	showHelp("Amplitude Help");
    }
}

ParseCmd AmpMag::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "compute_magnitudes")) {
	computeMagnitudes();
    }
    else if(parseCompare(cmd, "measure_amplitude")) {
        measureAmplitude(AUTO_MEASURE);
    }
    else if(parseCompare(cmd, "measure_ml")) {
        measureAmplitude(ML_MEASURE);
    }
    else if(parseCompare(cmd, "measure_mb")) {
        measureAmplitude(MB_MEASURE);
    }
    else if(parseString(cmd, "amplitudes", c)) {
        return amp_table->parseCmd(c, msg);
    }
    else if(parseString(cmd, "stamags", c)) {
        return stamag_table->parseCmd(c, msg);
    }
    else if(parseString(cmd, "netmags", c)) {
        return netmag_table->parseCmd(c, msg);
    }
    else if(parseString(cmd, "wplot", c)) {
	return plot1->parseCmd(c, msg);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar AmpMag::parseVar(const string &name, string &value)
{
    string c;

    if(parseString(name, "amplitudes", c)) {
        return amp_table->parseVar(c, value);
    }
    else if(parseString(name, "stamags", c)) {
        return stamag_table->parseVar(c, value);
    }
    else if(parseString(name, "netmags", c)) {
        return netmag_table->parseVar(c, value);
    }
    else if(parseString(name, "wplot", c)) {
        return plot1->parseVar(c, value);
    }
    return Frame::parseVar(name, value);
}

void AmpMag::editOn(void)
{
    cancel_button->setSensitive(true);
    save_button->setSensitive(true);
    
    amp_table->editModeOn();
    stamag_table->editModeOn();
    netmag_table->editModeOn();
}

void AmpMag::editOff(bool save)
{
    cancel_button->setSensitive(false);
    save_button->setSensitive(false);

    // do this to prevent the pane children from resizing
    frame_form->setVisible(false);
    pane->setVisible(false);
    
    if(save) {
	amp_table->editSave();
	stamag_table->editSave();
	netmag_table->editSave();
    }
    else {
	amp_table->editCancel();
	stamag_table->editCancel();
	netmag_table->editCancel();
    }
    if(!save) {
	cancelAmplitudes();
    }
    pane->setVisible(true);
    frame_form->setVisible(true);

    if(force_edit_on) {
	amp_table->selectAllRows(false);
    }
}

void AmpMag::selectRow(CSSTable *table)
{
    bool set;
    if(table == amp_table)
    {
	vector<bool> states;
	amp_table->getRowStates(states);
	cvector<CssAmplitudeClass> amps;
	data_source->getTable(amps);
	set = false;
	for(int i = 0; i < (int)states.size(); i++) if(states[i])
	{
	    if(!strcasecmp(amps[i]->amptype, "A5/2") ||
		!strcasecmp(amps[i]->amptype, "hpp_cnt"))
	    {
		set = true;
		break;
	    }
	}
	review_button->setSensitive(set);
	force_edit_on = false;
    }
    set = (amp_table->rowSelected() || stamag_table->rowSelected()
		|| netmag_table->rowSelected()) ? true : false;
    delete_button->setSensitive(set);
    edit_button->setSensitive(set);
}

void AmpMag::list(void)
{
    amplitudeList();
    stamagList();
    netmagList(); 

    updateAmpPlot();
}

void AmpMag::amplitudeList(void)
{
    cvector<CssAmplitudeClass> a;
    cvector<CssAmplitudeClass> v;

    amp_table->getSelected(v);
    amp_table->removeAllRecords();

    if(!data_source || data_source->getTable(a) <= 0)
    {
	compute_button->setSensitive(false);
	return;
    }
    compute_button->setSensitive(true);

    amp_table->addRecords(a);

    vector<int> selected;
    vector<bool> states;
    for(int i = 0; i < a.size(); i++) {
	for(int j = 0; j < v.size(); j++) {
	    if(a[i] == v[j]) {
		selected.push_back(i);
		states.push_back(true);
		break;
	    }
	}
    }
    amp_table->selectRows(selected, states);

//    amp_table->sortByColumnLabels("ampid");
}

void AmpMag::stamagList(void)
{
    cvector<CssStamagClass> s;
    cvector<CssStamagClass> v;

    stamag_table->getSelectedRecords(v);
    stamag_table->removeAllRecords();

    if(!data_source || data_source->getTable(s) <= 0) return;

    stamag_table->addRecords(s);

    int *selected = new int[s.size()];
    bool *states = new bool[s.size()];
    int n_selected = 0;
    for(int i = 0; i < s.size(); i++) {
	for(int j = 0; j < v.size(); j++) {
	    if(s[i] == v[j]) {
		selected[n_selected] = i;
		states[n_selected] = true;
		n_selected++;
		break;
	    }
	}
    }
    stamag_table->selectRows(n_selected, selected, states);
    delete [] selected;
    delete [] states;

    stamag_table->sortByColumnLabels("ampid");
}

void AmpMag::netmagList(void)
{
    cvector<CssNetmagClass> n;
    cvector<CssNetmagClass> v;

    netmag_table->getSelected(v);
    netmag_table->removeAllRecords();

    if(!data_source || data_source->getTable(n) <= 0) return;

    netmag_table->addRecords(n);

    int *selected = new int[n.size()];
    bool *states = new bool[n.size()];
    int n_selected = 0;
    for(int i = 0; i < n.size(); i++) {
	for(int j = 0; j < v.size(); j++) {
	    if(n[i] == v[j]) {
		selected[n_selected] = i;
		states[n_selected] = true;
		n_selected++;
		break;
	    }
	}
    }
    netmag_table->selectRows(n_selected, selected, states);
    delete [] selected;
    delete [] states;

    netmag_table->sortByColumnLabels("magid");
}

void AmpMag::measureAmplitude(AmpMeasureMode mode)
{
    cvector<CssArrivalClass> arrivals;

    if(data_source->getSelectedTable(arrivals) <= 0)
    {
	showWarning("No selected arrivals.");
	return;
    }

    for(int i = 0; i < arrivals.size(); i++) {
	wp->measureAmps(arrivals[i], NULL, mode);
    }
}

void AmpMag::computeMagnitudes(void)
{
    int i, j, k, l, num_mags, num_magtypes;
    cvector<CssAssocClass> assocs;
    cvector<CssOriginClass> origins;
    CssOriginClass *o;
    cvector<CssNetmagClass> netmags;
    CssNetmagClass *tmp_netmag;
    cvector<CssStamagClass> stamags;
    CssStamagClass *tmp_stamag;
    cvector<CssAmplitudeClass> amps;
    cvector<CssParrivalClass> parrivals;
    char *magtypes[50];
    MAGNITUDE *mag;
    Mag_Params mag_params;
    Password password = getpwuid(getuid());
    DataSource *ds;

    int num_netmag=0, num_stamag=0, num_det_amp=0, num_ev_amp=0;
    int num_assoc=0, num_parrival=0;

    Origin origin;
    Amplitude *det_amp = NULL, *ev_amp = NULL;
    Netmag *netmag = NULL;
    Stamag *stamag = NULL;
    Assoc *assoc = NULL;
    Parrival *parrival = NULL;

    if( !getSelectedAmps(amps) && !data_source->getTable(amps) )
    {
	info_area->setText(INFO_AREA_LEFT, "No amplitudes.");
	return;
    }
    if( !data_source->getTable(origins) ){
	info_area->setText(INFO_AREA_LEFT, "No origins.");
	return;
    }

    initMagLib(amps[0]);

    num_magtypes = getMagTypes(magtypes);

    mag_params = initialize_mag_params();
    mag_params.verbose = 0;
    strcpy(mag_params.net, "AA_IDC");
    strcpy(mag_params.magtype_to_origin_mb, "mb");
    strcpy(mag_params.magtype_to_origin_ms, "ms");
    strcpy(mag_params.magtype_to_origin_ml, "ml");
    mag_params.list_of_mb_magtypes = magtypes;
    mag_params.num_mb_magtypes = 1;

    data_source->getTable(netmags);
    data_source->getTable(stamags);
    data_source->getTable(assocs);
    data_source->getTable(parrivals);

    det_amp = (Amplitude *)mallocWarn(amps.size()*sizeof(Amplitude));
    ev_amp = (Amplitude *)mallocWarn(amps.size()*sizeof(Amplitude));
    netmag = (Netmag *)mallocWarn(netmags.size()*sizeof(Netmag));
    stamag = (Stamag *)mallocWarn(stamags.size()*sizeof(Stamag));
    assoc = (Assoc *)mallocWarn(assocs.size()*sizeof(Assoc));
    parrival = (Parrival *)mallocWarn(parrivals.size()*sizeof(Parrival));

    for(l = 0; l < origins.size(); l++)
    {
	o = origins[l];
	copyOrigin(&origin, origins[l]);

	// collect assocs and parrivals for this origin
	num_assoc = 0;
	for(i = 0; i < assocs.size(); i++) {
	    if(assocs[i]->orid == origin.orid) {
		copyAssoc(&assoc[num_assoc++], assocs[i]);
	    }
	}
	num_parrival = 0;
	for(i = 0; i < parrivals.size(); i++) {
	    if(parrivals[i]->orid == origin.orid) {
		copyParrival(&parrival[num_parrival++], parrivals[i]);
	    }
	}

	/* amplitudes are divided into two groups:
	 * amplitudes with arid > 0 (detection amplitudes)
	 * amplitudes with arid = -1 (so called event amplitudes),
	 *	which have a parid (predicted arrival) value instead
	 */
	num_det_amp = 0;
	for(i = 0; i < amps.size(); i++) if(amps[i]->arid > 0) 
	{
	    for(j = 0; j < num_assoc; j++) {
		if(amps[i]->arid == assoc[j].arid) break;
	    }
	    if(j < num_assoc) {
		copyAmplitude(&det_amp[num_det_amp], amps[i]);
		num_det_amp++;
	    }
	}

	num_ev_amp = 0;
	for(i = 0; i < amps.size(); i++) if (amps[i]->arid < 0)
	{
	    for(j = 0; j < parrivals.size(); j++) {
		if(amps[i]->parid == parrival[j].parid) break;
	    }
	    if(j < parrivals.size()) {
		copyAmplitude(&ev_amp[num_ev_amp], amps[i]);
		num_ev_amp++;
	    }
	}

	if(num_det_amp == 0 && num_ev_amp == 0) continue;

	// collect netmags
	num_netmag = 0;
	for(i = 0; i < netmags.size(); i++) if(netmags[i]->evid == origin.evid)
	{
	    copyNetmag(&netmag[num_netmag], netmags[i]);
	    num_netmag++;
	}

	// collect stamags
	num_stamag = 0;
	for(i = 0; i < stamags.size(); i++) if(stamags[i]->evid == origin.evid)
	{
	    copyStamag(&stamag[num_stamag], stamags[i]);
	    // if there is no amp for the stamag, set magdef = 'n'
	    for(j = 0; j < amps.size() &&
			amps[j]->ampid != stamags[i]->ampid; j++);
	    if(j == amps.size()) {
		if(stamag[num_stamag].magdef[0] != 'n') {
		    strcpy(stamag[num_stamag].magdef, "n");

		    ds =stamags[i]->getDataSource();
		    tmp_stamag = new CssStamagClass(*stamags[i]);
		    strcpy(tmp_stamag->magdef, "n");
		    ds->changeTable(stamags[i], tmp_stamag);
		    tmp_stamag->deleteObject();
		    strcpy(stamags[i]->magdef, "n");
		}
	    }
	    else {
		strncpy(stamag[num_stamag].magdef, "d",
			sizeof(stamag[num_stamag].magdef));
	    }
	    num_stamag++;
	}

	mag = build_mag_obj(magtypes, num_magtypes, &origin, netmag, num_netmag,
		stamag, num_stamag, det_amp, num_det_amp, ev_amp, num_ev_amp, 
		assoc, num_assoc, parrival, num_parrival);

	if((num_mags = calc_mags(mag,num_magtypes,&origin,&mag_params)) == ERR)
	{
	    fprintf(stdout,
	    "Error: Problem encountered while trying to compute magnitudes.\n");
	}

	copyOrigin(&origin, o);

	for(i = 0; i < num_magtypes; i++)
	{
	    if(mag[i].mag_computed)
	    {
		for(j = 0; j < netmags.size(); j++)
		{
		    if(netmags[j]->magid == mag[i].netmag.magid && 
			!strcmp(netmags[j]->magtype, mag[i].netmag.magtype))
		    {
			tmp_netmag = createNetmag(&mag[i].netmag);

			if( !netmags[j]->strictlyEquals(*tmp_netmag) )
			{
			    ds = netmags[j]->getDataSource();
			    ds->changeTable(netmags[j], tmp_netmag);
			    tmp_netmag->copyTo(netmags[j], false);
			}
			tmp_netmag->deleteObject();
			break;
		    }
	        }
		if(j == netmags.size()) {
		    fixAuth(mag[i].netmag.auth, password,
				sizeof(mag[i].netmag.auth));
		    tmp_netmag = createNetmag(&mag[i].netmag);
		    ds = o->getDataSource();
		    ds->addNetmag(tmp_netmag, o);
		    data_source->putTable(tmp_netmag);
		    mag[i].netmag.magid = tmp_netmag->magid;
		    tmp_netmag->deleteObject();
		}
	    }

	    for(j = 0; j < mag[i].count; j++) if(mag[i].stamag[j])
	    {
		for(k = 0; k < stamags.size(); k++) {
		    if( stamags[k]->ampid == mag[i].stamag[j]->ampid && 
			!strcmp(stamags[k]->magtype, mag[i].stamag[j]->magtype))
		    {
			tmp_stamag = createStamag(mag[i].stamag[j]);

			if(!stamags[k]->strictlyEquals(*tmp_stamag))
			{
			    ds =stamags[k]->getDataSource();
			    ds->changeTable(stamags[k], tmp_stamag);
			    tmp_stamag->copyTo(stamags[k], false);
			}
			tmp_stamag->deleteObject();
			break;
		    }
		}
		if(k == stamags.size()) {
		    fixAuth(mag[i].stamag[j]->auth, password,
				sizeof(mag[i].stamag[j]->auth));
		    mag[i].stamag[j]->magid = mag[i].netmag.magid;
		    tmp_stamag = createStamag(mag[i].stamag[j]);
		    ds = o->getDataSource();
		    ds->addStamag(tmp_stamag, o);
		    data_source->putTable(tmp_stamag);
		    tmp_stamag->deleteObject();
		}
		free(mag[i].stamag[j]);
	    }

	    free(mag[i].stamag);
	    for(j = 0; j < mag[i].count; j++) Free(mag[i].amplitude[j]);
	    free(mag[i].amplitude);
	    free(mag[i].sm_aux);
	}
	Free(mag);
    }
    for(i = 0; i < num_magtypes; i++) free(magtypes[i]);

    Free(det_amp);
    Free(ev_amp);
    Free(netmag);
    Free(stamag);
    Free(assoc);
    Free(parrival);

    updateTables();
}

void AmpMag::initMagLib(CssTableClass *css)
{
    string tlModelFile, magDescripFile;
    int i, num_magtypes;
    char *magTypes[50];
    cvector<CssSiteClass> site;

    // can only call setup_mag_facilities once(?) per execution. This means
    // we must use the first site list.
    if(mag_lib_ready) return;

    if(!(num_magtypes = getMagTypes(magTypes))) {
	showWarning("No magTypes.");
	return;
    }

    if(!getProperty("tlModelFile", tlModelFile)) {
	showWarning("No tlModelFile.");
	for(i = 0; i < num_magtypes; i++) free(magTypes[i]);
	return;
    }
    getProperty("magDescripFile", magDescripFile);

    DataSource *ds = css->getDataSource();

    if(!ds) {
	showWarning("No data source for site table");
	return;
    }

    if( !ds->getTable(site) ) {
	showWarning("No site table.");
	return;
    }
	
    Free(sites);
    sites = (Site *)mallocWarn(site.size()*sizeof(Site));

    for(i = 0; i < site.size(); i++) {
	stringcpy(sites[i].sta, site[i]->sta, sizeof(sites[i].sta));
	sites[i].lat = site[i]->lat;
	sites[i].lon = site[i]->lon;
	sites[i].elev = site[i]->elev;
    }

    // not sure if you can call this routine more than once.
    setup_mag_facilities((char *)tlModelFile.c_str(),
		(char *)magDescripFile.c_str(), magTypes, num_magtypes, sites,
		site.size());

    for(i = 0; i < num_magtypes; i++) free(magTypes[i]);

    mag_lib_ready = true;
}

int AmpMag::getSelectedAmps(cvector<CssAmplitudeClass> &amplitudes)
{
    int i, j;
    vector<int> rows;
    cvector<CssAmplitudeClass> amps;

    amplitudes.clear();
    if(!amp_table->getSelectedRows(rows)) return 0;

    data_source->getTable(amps);

    for(i = 0; i < amps.size(); i++) {
	for(j = 0; j < (int)rows.size() && rows[j] != i; j++);
	if(j < (int)rows.size()) {
	    amplitudes.add(amps[i]);
	}
    }
    return amplitudes.size();
}

int AmpMag::getSelectedNetmags(cvector<CssNetmagClass> &netmags)
{
    int i, j;
    vector<int> rows;
    cvector<CssNetmagClass> nets;

    netmags.clear();
    if(!netmag_table->getSelectedRows(rows)) return 0;

    data_source->getTable(nets);

    for(i = 0; i < nets.size(); i++) {
	for(j = 0; j < (int)rows.size() && rows[j] != i; j++);
	if(j < (int)rows.size()) {
	    netmags.add(nets[i]);
	}
    }
    return netmags.size();
}

int AmpMag::getSelectedStamags(cvector<CssStamagClass> &stamags)
{
    int i, j;
    vector<int> rows;
    cvector<CssStamagClass> stas;

    stamags.clear();
    if(!stamag_table->getSelectedRows(rows)) return 0;

    data_source->getTable(stas);

    for(i = 0; i < stas.size(); i++) {
	for(j = 0; j < (int)rows.size() && rows[j] != i; j++);
	if(j < (int)rows.size()) {
	    stamags.add(stas[i]);
	}
    }
    return stamags.size();
}

void AmpMag::updateTables(void)
{
    stamagList();
    stamag_table->moveToTop(stamag_table->numRows()-1);
    netmagList(); 
    netmag_table->moveToTop(netmag_table->numRows()-1);
}

void AmpMag::deleteSelected(void)
{
    cvector<CssNetmagClass> netmags;
    cvector<CssStamagClass> stamags;
    cvector<CssAmplitudeClass> amps;

    getSelectedStamags(stamags);
    wp->deleteTables(stamags);

    getSelectedNetmags(netmags);
    wp->deleteTables(netmags);

    getSelectedAmps(amps);
    wp->deleteTables(amps);

    list();
}

static int
getMagTypes(char **magTypes)
{
    char *c, *prop, *tok, *last;
    int num;

    if(!(prop = Application::getProperty("magTypes"))) {
	prop = strdup(
//    "mb,mb_ave,mb_mle,ms,ms_ave,ms_mle,ml,mlppn,mb1,mb1mle,ms1,ms1mle,mb_tmp");
//    "mb,mb_ave,mb_mle,ms,ms_ave,ms_mle,ml,mlppn,mb1,mb1mle,ms1,ms1mle");
    "mb,ml");
    }
    tok = prop;
    num = 0;
    while(num < 50 && (c = strtok_r(tok, ",", &last)) != NULL)
    {
	tok = NULL;
	magTypes[num++] = strdup(stringTrim(c));
    }
    free(prop);
    return num;
}

static void
fixAuth(char *auth, Password password, int len)
{
    if (!strcmp(auth, "-") || !strcmp(auth, "build_mag_obj")) {
	stringcpy(auth, password->pw_name, len);
    }
}

static void
copyOrigin(Origin *origin, CssOriginClass *o)
{
    origin->lat = o->lat;
    origin->lon = o->lon;
    origin->depth = o->depth;
    origin->time = o->time;
    origin->orid = o->orid;
    origin->evid = o->evid;
    origin->jdate = o->jdate;
    origin->nass = o->nass;
    origin->ndef = o->ndef;
    origin->ndp = o->ndp;
    origin->grn = o->grn;
    origin->srn = o->srn;
    memcpy(origin->etype, o->etype, sizeof(origin->etype));
    origin->depdp = o->depdp;
    memcpy(origin->dtype, o->dtype, sizeof(origin->dtype));
    origin->mb = o->mb;
    origin->mbid = o->mbid;
    origin->ms = o->ms;
    origin->msid = o->msid;
    origin->ml = o->ml;
    origin->mlid = o->mlid;
    memcpy(origin->algorithm, o->algorithm, sizeof(origin->algorithm));
    memcpy(origin->auth, o->auth, sizeof(origin->auth));
    origin->commid = o->commid;
    strcpy(origin->lddate, timeDateString(&o->lddate));
}

static void
copyNetmag(Netmag *netmag, CssNetmagClass *n)
{
    netmag->magid = n->magid;
    memcpy(netmag->net, n->net, sizeof(netmag->net));
    netmag->orid = n->orid;
    netmag->evid = n->evid;
    memcpy(netmag->magtype, n->magtype, sizeof(netmag->magtype));
    netmag->nsta = n->nsta;
    netmag->magnitude = n->magnitude;
    netmag->uncertainty = n->uncertainty;
    memcpy(netmag->auth, n->auth, sizeof(netmag->auth));
    netmag->commid = n->commid;
    strcpy(netmag->lddate, timeDateString(&n->lddate));
}

static CssNetmagClass *
createNetmag(Netmag *netmag)
{
    double t = NULL_TIME;
    CssNetmagClass *n = new CssNetmagClass();
    n->magid = netmag->magid;
    memcpy(n->net, netmag->net, sizeof(n->net));
    n->orid = netmag->orid;
    n->evid = netmag->evid;
    memcpy(n->magtype, netmag->magtype, sizeof(n->magtype));
    n->nsta = netmag->nsta;
    n->magnitude = netmag->magnitude;
    n->uncertainty = netmag->uncertainty;
    memcpy(n->auth, netmag->auth, sizeof(n->auth));
    n->commid = netmag->commid;
    timeParseString(netmag->lddate, &t);
    timeEpochToDate(t, &n->lddate);
    return n;
}

static void
copyStamag(Stamag *stamag, CssStamagClass *s)
{
    stamag->magid = s->magid;
    stamag->ampid = s->ampid;
    memcpy(stamag->sta, s->sta, sizeof(stamag->sta));
    stamag->arid = s->arid;
    stamag->orid = s->orid;
    stamag->evid = s->evid;
    memcpy(stamag->phase, s->phase, sizeof(stamag->phase));
    stamag->delta = s->delta;
    memcpy(stamag->magtype, s->magtype, sizeof(stamag->magtype));
    stamag->magnitude = s->magnitude;
    stamag->uncertainty = s->uncertainty;
    stamag->magres = s->magres;
    memcpy(stamag->magdef, s->magdef, sizeof(stamag->magdef));
    memcpy(stamag->mmodel, s->mmodel, sizeof(stamag->mmodel));
    stamag->commid = s->commid;
    memcpy(stamag->auth, s->auth, sizeof(stamag->auth));
    strcpy(stamag->lddate, timeDateString(&s->lddate));
}

static CssStamagClass *
createStamag(Stamag *stamag)
{
    double t = NULL_TIME;
    CssStamagClass *s = new CssStamagClass();
    s->magid = stamag->magid;
    s->ampid = stamag->ampid;
    memcpy(s->sta, stamag->sta, sizeof(s->sta));
    s->arid = stamag->arid;
    s->orid = stamag->orid;
    s->evid = stamag->evid;
    memcpy(s->phase, stamag->phase, sizeof(s->phase));
    s->delta = stamag->delta;
    memcpy(s->magtype, stamag->magtype, sizeof(s->magtype));
    s->magnitude = stamag->magnitude;
    s->uncertainty = stamag->uncertainty;
    s->magres = stamag->magres;
    memcpy(s->magdef, stamag->magdef, sizeof(s->magdef));
    memcpy(s->mmodel, stamag->mmodel, sizeof(s->mmodel));
    s->commid = stamag->commid;
    memcpy(s->auth, stamag->auth, sizeof(s->auth));
    timeParseString(stamag->lddate, &t);
    timeEpochToDate(t, &s->lddate);
    return s;
}

static void
copyAmplitude(Amplitude *amp, CssAmplitudeClass *a)
{
    amp->ampid = a->ampid;
    amp->arid = a->arid;
    amp->parid = a->parid;
    memcpy(amp->chan, a->chan, sizeof(amp->chan));
    amp->amp = a->amp;
    amp->per = a->per;
    amp->snr = a->snr;
    amp->amptime = a->amptime;
    amp->start_time = a->start_time;
    amp->duration = a->duration;
    amp->bandw = a->bandw;
    memcpy(amp->amptype, a->amptype, sizeof(amp->amptype));
    memcpy(amp->units, a->units, sizeof(amp->units));
    memcpy(amp->clip, a->clip, sizeof(amp->clip));
    memcpy(amp->inarrival, a->inarrival, sizeof(amp->inarrival));
    memcpy(amp->auth, a->auth, sizeof(amp->auth));
    strcpy(amp->lddate, timeDateString(&a->lddate));
}

static void
copyAssoc(Assoc *assoc, CssAssocClass *a)
{
    assoc->arid = a->arid;
    assoc->orid = a->orid;
    memcpy(assoc->sta, a->sta, sizeof(assoc->sta));
    memcpy(assoc->phase, a->phase, sizeof(assoc->phase));
    assoc->belief = a->belief;
    assoc->delta = a->delta;
    assoc->seaz = a->seaz;
    assoc->esaz = a->esaz;
    assoc->timeres = a->timeres;
    memcpy(assoc->timedef, a->timedef, sizeof(assoc->timedef));
    assoc->azres = a->azres;
    memcpy(assoc->azdef, a->azdef, sizeof(assoc->azdef));
    assoc->slores = a->slores;
    memcpy(assoc->slodef, a->slodef, sizeof(assoc->slodef));
    assoc->emares = a->emares;
    assoc->wgt = a->wgt;
    memcpy(assoc->vmodel, a->vmodel, sizeof(assoc->vmodel));
    assoc->commid = a->commid;
    strcpy(assoc->lddate, timeDateString(&a->lddate));
}

static void
copyParrival(Parrival *parrival, CssParrivalClass *p)
{
    parrival->parid = p->parid;
    parrival->orid = p->orid;
    parrival->evid = p->evid;
    memcpy(parrival->sta, p->sta, sizeof(parrival->sta));
    parrival->time = p->time;
    parrival->azimuth = p->azimuth;
    parrival->slow = p->slow;
    memcpy(parrival->phase, p->phase, sizeof(parrival->phase));
    parrival->delta = p->delta;
    memcpy(parrival->vmodel, p->vmodel, sizeof(parrival->vmodel));
    strcpy(parrival->lddate, timeDateString(&p->lddate));
}


void AmpMag::amplitudeReview(void)
{
    int i;
    vector<bool> states;
    cvector<CssAmplitudeClass> amps;
    cvector<CssArrivalClass> arrivals;

    amp_table->editModeOff();

    if( !amp_table->getRowStates(states) ) return;

    for(i = 0; i < (int)states.size() && !states[i]; i++);
    if(i == (int)states.size()) {
	showWarning("No amplitude records selected.");
	return;
    }

    data_source->getTable(amps);
    data_source->getTable(arrivals);

    setCursor("hourglass");

    plot1->clear();

    review_source->clear();

    review(states, amps, arrivals);

    setCursor("default");
}

void AmpMag::review(vector<bool> &states, cvector<CssAmplitudeClass> &amps,
		cvector<CssArrivalClass> &arrivals)
{
    int i, j, k;
    gvector<Waveform *> wvec;
    Waveform *main_w;
    double duration, max_duration;
    double tmin, tmax, tbeg, tend;
    CssAmplitudeClass *gamp;
    CssArrivalClass *garr;
    BeamRecipe recipe;

    data_source->getWaveforms(wvec, false);

    max_duration = 0.;

    for(i = 0; i < (int)states.size(); i++) if(states[i])
    {
	gamp = amps[i];

	for(k = 0; k < arrivals.size() && gamp->arid != arrivals[k]->arid; k++);
	if(k == arrivals.size()) {
	    showWarning("Error getting CssArrivalClass object.");
	    continue;
	}
	garr = arrivals[k];

	/* first look for the waveform that the amplitude belongs to.
	 */
	for(j = 0; j < wvec.size(); j++) {
	    if((!strcmp(wvec[j]->net(), garr->sta) || 
		!strcmp(wvec[j]->sta(), garr->sta)) &&
		!strcmp(wvec[j]->ts->chan(), gamp->chan)) break;
	}
	if(j == wvec.size()) {
	    // find any waveform from the network
	    for(j = 0; j < wvec.size(); j++) {
		if((!strcmp(wvec[j]->net(), garr->sta) || 
		    !strcmp(wvec[j]->sta(), garr->sta)) &&
		    wvec[j]->tbeg() <= garr->time &&
		    wvec[j]->tend() >= garr->time) break;
	    }
	}
	if(j == wvec.size()) {
	    showWarning("Cannot find waveforms for arrival %s/%s %d",
			garr->sta, garr->chan, garr->arid);
	    continue;
	}
	main_w = wvec[j];

	if(!strcmp(gamp->amptype, "-")) {
	    string amptype;
	    if( getProperty("amp_type", amptype) ) {
		stringcpy(gamp->amptype, amptype.c_str(),sizeof(gamp->amptype));
	    }
	}

	if(!strcmp(gamp->amptype, "A5/2"))
	{
	    GSegment *s = main_w->ts->nearestSegment(gamp->amptime);

	    gamp->amp_nms = 2*gamp->amp;
	    if(!BasicSource::nms2cts(main_w->ts, s, gamp->amp_nms, gamp->per,
					&gamp->amp_cnts))
	    {
		showWarning("Cannot convert amplitude nms to counts.");
		continue;
	    }
	}
	else if(!strcmp(gamp->amptype, "hpp_cnt"))
	{
	    gamp->amp_cnts = 2*gamp->amp;
	}
	else {
	    showWarning("Cannot review amptype %s", gamp->amptype);
	    continue;
	}
	gamp->box_location = True;
	gamp->boxtime = gamp->amptime;
	    
	duration = gamp->duration > gamp->per ? gamp->duration : gamp->per;
	if(duration <= 0.) {
	    showWarning("Duration and period <= 0.");
	    continue;
	}

	/* check if this is a beam
	 */
	if( gbeam->beamRecipe(main_w->net(), gamp->chan, recipe) )
	{
	    reviewBeamAmp(wvec, main_w, gamp, garr, recipe,
			&tmin, &tmax, &max_duration);
	}
	else if(DataSource::compareChan(main_w->ts->chan(), gamp->chan))
	{
	    reviewRegularAmp(main_w, gamp, garr, &tmin, &tmax, &max_duration);
    	}
	else {
	    showWarning("Cannot find waveforms for arrival %s/%s %d",
			garr->sta, garr->chan, garr->arid);
	}
    }

    if(max_duration > 0.) {
	plot1->setTimeLimits(tmin, tmax);
	tbeg = -4.*max_duration;
	tend = 4.*max_duration;
	plot1->setXLimits(tbeg, tend);
	plot1->setScale(plot1->getDataHeight(), true, true);
    }
}

void AmpMag::reviewBeamAmp(gvector<Waveform *> &wvec, Waveform *main_w,
		CssAmplitudeClass *gamp, CssArrivalClass *garr, BeamRecipe &recipe,
		double *tmin, double *tmax, double *max_duration)
{
    int i, n;
    gvector<Waveform *> ws;
    vector<BeamSta> beam_sta;
    GTimeSeries *ts=NULL;
    CssOriginClass *origin = NULL;
    cvector<CssAssocClass> assocs;
    CssAssocClass *assoc=NULL;

    /* the amplitude refers to a beam.
     */
    n = data_source->getTable(assocs);
    for(i = 0; i < n && garr->arid != assocs[i]->arid; i++);
    if(i < n) {
        assoc = assocs[i];
    }
    if( !assoc ) {
	showWarning("Cannot find assoc for arrival.arid=%ld", garr->arid);
	return;
    }

    /* select the beam components.
     */
    if(Beam::getGroup(recipe, beam_sta) <= 0)
    {
	showWarning(GError::getMessage());
	return;
    }
    n = getCDs(beam_sta, wvec, ws);

    if(n == 0) {
	return;
    }
    else if(n == 1) {
	ts = new GTimeSeries(ws[0]->ts);
    }
    else {
	for(i = 0; i < n; i++) {
	    if( (origin = data_source->getPrimaryOrigin(ws[i]))
		&& origin->lat > -900. && origin->lon > -900.) break;
	}
	if( !origin ) {
	    showWarning("No origin information.");
	    return;
	}

        ts = wp->getAmpBeam(ws, garr, origin, assoc->phase, true);
	if (ts) {
	  ts->setNet(ws[0]->net());
	  ts->setSta(ws[0]->net());
	  ts->setChan(recipe.name);
	  review_source->addData(ts);
	  ts->setDataSource(review_source);
	}
    }
    if(ts) {
	loadTs(ts, gamp, garr, assoc, ws[0], tmin, tmax, max_duration);
    }
    return;
}

int AmpMag::getCDs(vector<BeamSta> &beam_sta, gvector<Waveform *> &wvec,
			gvector<Waveform *> &ws)
{
    int j, l, n, mlen;
    char missing[500];

    ws.clear();
    mlen = 1;
    missing[0] = '\0';
    for(j = n = 0; j < (int)beam_sta.size(); j++) if(beam_sta[j].wgt != 0.)
    {
	for(l = 0; l < wvec.size(); l++)
	{
	    if( !strcasecmp(wvec[l]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(wvec[l]->ts->chan(), beam_sta[j].chan))
	    {
		ws.push_back(wvec[l]);
		n++;
		break;
	    }
	}
	if(l == wvec.size()) {
	    int m = 3 + (int)strlen(beam_sta[j].sta) +
				(int)strlen(beam_sta[j].chan);
	    if(mlen + m < 500) {
		if(missing[0] != '\0') strcat(missing, ", ");
		strcat(missing, beam_sta[j].sta);
		strcat(missing, "/");
		strcat(missing, beam_sta[j].chan);
		mlen = (int)strlen(missing);
	    }
	}
    }
    if(missing[0] != '\0') {
	showWarning("Missing beam channels: %s.", missing);
	return 0;
    }
    return n;
}

void AmpMag::loadTs(GTimeSeries *ts, CssAmplitudeClass *gamp, CssArrivalClass *garr,
		CssAssocClass *assoc, Waveform *main_w,
		double *tmin, double *tmax, double *max_duration)
{
    Waveform *new_w;
    double duration, tbeg, tend, calib;
    GSegment *s;

    ts->putValue("amplitude", gamp);
    ts->putValue("ts", main_w->ts);

    tbeg = ts->tbeg() - gamp->amptime;
    tend = tbeg + ts->tend() - ts->tbeg();

    new_w = plot1->addWaveform(ts, nextColor());
    gvector<Waveform *> v(new_w);
    vector<double> d;
    d.push_back(tbeg);
    plot1->positionX(v, d);

    duration = gamp->per;

    s = ts->nearestSegment(gamp->amptime);
    calib = (s->calib() != 0.) ? s->calib() : 1.;

    BasicSource::nms2cts(ts, s, 2.*gamp->amp, gamp->per, &gamp->amp_cnts);

    gamp->amp_Nnms = gamp->amp_cnts*fabs(calib);

    gamp->boxmin = ts->dataMin(gamp->amptime, gamp->amptime + duration);

    plot1->putTable(gamp);
    plot1->putTable(assoc);
    stringcpy(garr->chan, new_w->chan(), sizeof(garr->chan));
    plot1->putArrivalWithColor(garr, stringToPixel("black"));
    ignore_measure_cb = true;
    plot1->reviewMeasurementOnWaveform(gamp, new_w);
    ignore_measure_cb = false;

    if(*max_duration == 0) {
	*tmin = tbeg;
	*tmax = tend;
    }
    else {
	if(tbeg < *tmin) *tmin = tbeg;
	if(tend > *tmax) *tmax = tend;
    }
    if(*max_duration < duration) *max_duration = duration;
}

void AmpMag::reviewRegularAmp(Waveform *main_w, CssAmplitudeClass *gamp,
		CssArrivalClass *garr, double *tmin, double *tmax,
		double *max_duration)
{
    Waveform *new_w;
    double duration;
    double tbeg, tend, calib;
    GTimeSeries *ts;
    GSegment *s;
    AmplitudeParams *ap = WaveformPlot::amplitudeParams();

    tbeg = gamp->start_time;
    tend = gamp->start_time + gamp->duration;
    ts = main_w->ts->subseries(tbeg, tend);
    ts->setOriginalStart(tbeg);
    ts->setOriginalEnd(tend);
    ts->removeAllMethods();

    TaperData *taper = new TaperData("cosine", (int)(ap->mb_taper_frac*100),
					5, 200);
    taper->apply(ts);
    taper->deleteObject();

    IIRFilter *filter = new IIRFilter(ap->mb_filter_order, ap->mb_filter_type,
		ap->mb_filter_locut, ap->mb_filter_hicut,
		ts->segment(0)->tdel(), ap->mb_filter_zp);
    filter->apply(ts);
    filter->deleteObject();

    ts->putValue("amplitude", gamp);
    ts->putValue("ts", main_w->ts);

    tbeg = ts->tbeg() - gamp->amptime;
    tend = tbeg + ts->tend() - ts->tbeg();

    new_w = plot1->addWaveform(ts, nextColor());
    gvector<Waveform *> v(new_w);
    vector<double> d;
    d.push_back(tbeg);
    plot1->positionX(v, d);

    duration = gamp->per;

    s = main_w->ts->nearestSegment(gamp->amptime);

    calib = (s->calib() != 0.) ? s->calib() : 1.;
    gamp->amp_Nnms = gamp->amp_cnts*fabs(calib);

    gamp->boxmin = ts->dataMin(gamp->amptime, gamp->amptime + gamp->per);

    plot1->putTable(gamp);
    plot1->putArrivalWithColor(garr, stringToPixel("black"));
    ignore_measure_cb = true;
    plot1->reviewMeasurementOnWaveform(gamp, new_w);
    ignore_measure_cb = false;

    if(*max_duration == 0) {
	*tmin = tbeg;
	*tmax = tend;
    }
    else {
	if(tbeg < *tmin) *tmin = tbeg;
	if(tend > *tmax) *tmax = tend;
    }
    if(*max_duration < duration) *max_duration = duration;
}

void AmpMag::getColumns(int *amp_col, int *per_col,
		int *amptime_col, int *start_time_col)
{
    vector<const char *> labels;

    amp_table->getColumnLabels(labels);
    *amp_col = -1;
    *per_col = -1;
    *amptime_col = -1;
    *start_time_col = -1;

    for(int i = 0; i < (int)labels.size(); i++)
    {
	if(!strcmp(labels[i], "amp")) {
	    *amp_col = i;
	}
	else if(!strcmp(labels[i], "per")) {
	    *per_col = i;
	}
	else if(!strcmp(labels[i], "amptime")) {
	    *amptime_col = i;
	}
	else if(!strcmp(labels[i], "start_time")) {
	    *start_time_col = i;
	}
    }
}

void AmpMag::ampMeasure(CPlotMeasure *m)
{
    AmpFormats formats;
    CssAmplitudeClass *gamp;
    GTimeSeries *ts=NULL;

    if(m->w == NULL ||
	!(gamp = (CssAmplitudeClass *)m->w->ts->getValue("amplitude")) ||
	!(ts = (GTimeSeries *)m->w->ts->getValue("ts")) )
    {
	return;
    }
    getAmpFormats(&formats);

    updateAmpTable(&formats, gamp, m, ts);
}

void AmpMag::updateAmpTable(AmpFormats *formats, CssAmplitudeClass *gamp,
			CPlotMeasure *m, GTimeSeries *ts)
{
    int		i, amp_col, per_col, amptime_col, start_time_col;
    int		num_fields, col[4], row[4];
    double	calib, amp_nms;
    char	*field[4];
    char	fields[400];
    GSegment	*s;
    vector<const char *> r;

    field[0] = fields;
    field[1] = fields+100;
    field[2] = fields+200;
    field[3] = fields+300;

    getColumns(&amp_col, &per_col, &amptime_col, &start_time_col);

    s = ts->nearestSegment(ts->tbeg() + m->left_side);
    calib = (s->calib() != 0.) ? s->calib() : 1.;
    m->amp_cnts = m->amp_Nnms/calib;

    int num_rows = amp_table->numRows();
    for(i = 0; i < num_rows && gamp != amp_table->getRecord(i); i++);

    if(i == num_rows) return;

    if(!strcasecmp(gamp->amptype, "A5/2")) {
	if( !BasicSource::cts2nms(ts, s, m->amp_cnts, m->period, &amp_nms) ) {
	    return;
	}
    }
    else if(strcasecmp(gamp->amptype, "hpp_cnt")) {
	return;
    }

    if( !amp_table->editMode() ) {
	amp_table->selectAllRows(true);
	editOn();
	force_edit_on = true;
    }
    else {
	vector<bool> editable;
        amp_table->getRowEditable(editable);
	bool b = ((int)editable.size() > i ) ? editable[i] : false;
	if(!b) return;
    }

    amp_table->getRow(i, r);

    num_fields = 0;
    if(amp_col >= 0) {
	double amp;
	if(!strcasecmp(gamp->amptype, "A5/2")) amp = .5*amp_nms;
	else amp = m->amp_cnts; // hpp_cts
	snprintf(field[num_fields], 100, formats->amp_format, amp);
	row[num_fields] = i;
	col[num_fields] = amp_col;
	num_fields++;
    }
    if(per_col >= 0) {
	snprintf(field[num_fields], 100, formats->per_format,m->period);
	row[num_fields] = i;
	col[num_fields] = per_col;
	num_fields++;
    }
    if(amptime_col >= 0) {
	setTimeField(field[num_fields], 100,formats->amptime_format,
				m->w->tbeg() + m->left_side);
	row[num_fields] = i;
	col[num_fields] = amptime_col;
	num_fields++;
    }
    if(start_time_col >= 0) {
	setTimeField(field[num_fields], 100, formats->start_time_format,
				m->w->tbeg() + m->left_side);
	row[num_fields] = i;
	col[num_fields] = start_time_col;
	num_fields++;
    }

    if(num_fields) {
	amp_table->setFields(num_fields, row, col,(const char **)field);
    }
}

void AmpMag::getAmpFormats(AmpFormats *formats)
{
    strcpy(formats->amp_format, "%.4f");
    strcpy(formats->per_format, "%.2f");
    strcpy(formats->amptime_format, "%t");
    strcpy(formats->start_time_format, "%t");

    TAttribute a;
    a = amp_table->getAttribute("amp");
    if(a.format[0] != '\0')  {
	stringcpy(formats->amp_format, a.format, sizeof(formats->amp_format));
    }
    a = amp_table->getAttribute("per");
    if(a.format[0] != '\0')  {
	stringcpy(formats->per_format, a.format, sizeof(formats->per_format));
    }
    a = amp_table->getAttribute("amptime");
    if(a.format[0] != '\0')  {
	stringcpy(formats->amptime_format, a.format,
			sizeof(formats->amptime_format));
    }
    a = amp_table->getAttribute("start_time");
    if(a.format[0] != '\0')  {
	stringcpy(formats->start_time_format, a.format,
			sizeof(formats->start_time_format));
    }
}

static void
setTimeField(char *field, int len, char *format, double value)
{
    if(!strcmp(format, "%g")) {
	timeEpochToString(value, field, len, GSE20);
    }
    else if(!strcmp(format, "%G")) {
	timeEpochToString(value, field, len, GSE21);
    }
    else if(!strcmp(format, "%t")) {
	timeEpochToString(value, field, len, YMONDHMS);
    }
    else {
	snprintf(field, len, format, value);
    }
    if(field[0] == '\0') {
	snprintf(field, len, format, value);
    }
}

Pixel AmpMag::nextColor()
{
    gvector<Waveform *> wvec;

    plot1->getWaveforms(wvec, true);
    Pixel color = plot1->nextColor(wvec);

    return color;
}

void AmpMag::updateAmpPlot(void)
{
    gvector<Waveform *> wvec;
    cvector<CssAmplitudeClass> amps;

    amp_table->getTableRecords(amps);

    plot1->getWaveforms(wvec, false);

    for(int j = 0; j < wvec.size(); j++)
    {
	CssAmplitudeClass *gamp;
	if( (gamp = (CssAmplitudeClass *)wvec[j]->ts->getValue("amplitude")) )
	{
	    int i;
	    for(i = 0; i < amps.size() && amps[i] != gamp; i++);
	    if(i == amps.size()) {
		gvector<Waveform *> v(wvec[j]);
		plot1->deleteWaveforms(v);
	    }
	    else {
		CPlotMeasure m;
		plot1->getWaveformMeasureBox(wvec[j], &m);
		ignore_measure_cb = true;
		ampMeasure(&m);
		ignore_measure_cb = false;
	    }
	}
    }
}

void AmpMag::cancelAmplitudes(void)
{
    gvector<Waveform *> wvec;
    cvector<CssAmplitudeClass> amps;
    CssAmplitudeClass *gamp;
    GTimeSeries *ts;

    plot1->getWaveforms(wvec, false);
    data_source->getTable(amps);

    ignore_measure_cb = true;
    for(int i = 0; i < amps.size(); i++) {
	for(int j = 0; j < wvec.size(); j++) {
	    ts = wvec[j]->ts;
	    if( (gamp = (CssAmplitudeClass *)ts->getValue("amplitude")) )
	    {
		if(gamp == amps[i]) {
		    gamp->boxmin = ts->dataMin(gamp->amptime,
						gamp->amptime + gamp->per);
		    plot1->reviewMeasurementOnWaveform(amps[i], wvec[j]);
		}
	    }
	}
    }
    ignore_measure_cb = false;
}

void AmpMag::modifyReview(CPlotModifyCallbackStruct *c)
{
    CPlotMeasure m;

    ignore_measure_cb = true;

    for(int i = 0; i < c->wvec.size(); i++) {
	if(plot1->getWaveformMeasureBox(c->wvec[i], &m)) {
	    double time = c->wvec[i]->tbeg() + m.left_side;
	    m.bottom_side = c->wvec[i]->ts->dataMin(time, time + m.period);
	    plot1->removeMeasureBox(c->wvec[i]);
	    plot1->addMeasurement(&m, c->wvec[i]);
	}
    }
    ignore_measure_cb = false;
}
