/** \file MeasureAmpPer.cpp
 *  \brief Defines class MeasureAmpPer
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <dlfcn.h>

using namespace std;

#include "MeasureAmpPer.h"
#include "libgx++.h"
#include "motif++/MotifClasses.h"
#include "SearchParam.h"
#include "IIRFilter.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
extern int measure_amplitude(float *beam, int npts, double beam_time,
		double samprate, char *type, double value, double thresh,
		double flo, double fhi, int ford, int zp, char *ftype,
		int interp_per, double *amp, double *per, double *time);

}

using namespace libgarrival;


MeasureAmpPer::MeasureAmpPer(const char *name, Component *parent,
		DataSource *ds, CPlotClass *cplot) : Frame(name, parent),
		DataReceiver(ds)
{
    cp = cplot;
    createInterface();
    init();
}

void MeasureAmpPer::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    net_toggle = new Toggle("Network Mode", edit_menu, this);
    net_toggle->set(true);
    save_all_button = new Button("Save All", edit_menu, this);
    save_selected_button = new Button("Save Selected", edit_menu, this);
    save_selected_button->setSensitive(false);
    create_amp_button = new Button("Create Amplitude", edit_menu, this);
    create_amp_button->setSensitive(false);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);
    clear_selected_button = new Button("Clear Selected Boxes", view_menu, this);
    clear_all_button = new Button("Clear All Boxes", view_menu, this);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);

    dfx_menu = new Menu("DFX Amplitude", option_menu);
    peak_trough_button = new Button("Peak Trough", dfx_menu, this);
    peak_trough_button->setCommandString("DFX peak_trough");
    zero_peak_button = new Button("Zero Peak", dfx_menu, this);
    zero_peak_button->setCommandString("DFX zero_peak");
    fm_a_button = new Button("First Motion A", dfx_menu, this);
    fm_a_button->setCommandString("DFX fm_a");
    fm_b_button = new Button("First Motion B", dfx_menu, this);
    fm_b_button->setCommandString("DFX fm_b");
    fm_button = new Button("First Motion", dfx_menu, this);
    fm_button->setCommandString("DFX fm");
    fm_sign_button = new Button("First Motion Sign", dfx_menu, this);
    fm_sign_button->setCommandString("DFX fm_sign");
    rms_button = new Button("RMS", dfx_menu, this);
    rms_button->setCommandString("DFX rms");
    mean_sqr_button = new Button("Mean Square", dfx_menu, this);
    mean_sqr_button->setCommandString("DFX mean_sqr");
    abs_max_button = new Button("Abs Max", dfx_menu, this);
    abs_max_button->setCommandString("DFX abs_max");
    avg_max_button = new Button("Avg Max", dfx_menu, this);
    avg_max_button->setCommandString("DFX avg_max");
    stav_button = new Button("Stav", dfx_menu, this);
    stav_button->setCommandString("DFX stav");

    dfx_interp_toggle = new Toggle("DFX Interpolate Period", option_menu, this);
    measure_all_button = new Button("Measure All", option_menu, this);
    measure_sel_button = new Button("Measure Selected", option_menu, this);
    if(cp->phaseLineIsDisplayed() || cp->lineIsDisplayed("A")) {
	measure_all_button->setSensitive(true);
	measure_sel_button->setSensitive(true);
    }
    else {
	measure_all_button->setSensitive(false);
	measure_sel_button->setSensitive(false);
    }
    cp->addActionListener(this, XtNlineCallback);
    cp->addActionListener(this, XtNphaseLineCallback);

    review_toggle = new Toggle("Review", option_menu, this);
    search_button = new Button("Search For Period", option_menu, this);
    search_param_button = new Button("Search Parameters...", option_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Measure Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNvisibleRows, 15); n++;
    XtSetArg(args[n], XtNwidth, 450); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    table = new Table("Amp Per", frame_form, args, n);
    table->setAttributes(
"amp(hpp_cnt),%.2f,amp(hpp_nm),%.2f,amp(hpp_Nnm),%.2f,amptype,%s,period,%.2f,\
amp(pp_cnt),%.2f, amp(pp_nm),%.2f, amp(pp_Nnm),%.2f,time,%t,sta,%s,chan,%s,\
wfid,%d,chanid,%d,nsamp,%d,samprate,%.4f,calib,%.4f,\
calper,%.4f,instype,%s,segtype,%s,datatype,%s,clip,%s,dir,%s,dfile,%s");
    table->setVisible(true);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    search_param_window = new SearchParam("Search Parameters", this);

    addPlugins("MeasureAmpPer", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(save_all_button, "Save All");
	tool_bar->add(create_amp_button, "Create Amplitude");
    }
    enableCallbackType(XmNactivateCallback);
}

void MeasureAmpPer::init(void)
{
    if( !getProperty("amp_type", amp_type) ) {
	amp_type.assign("A5/2");
	putProperty("amp_type", amp_type, false, this);
    }
    list();
}

MeasureAmpPer::~MeasureAmpPer(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void MeasureAmpPer::setVisible(bool visible)
{
    Frame::setVisible(visible);
    if(cp) {
	Arg args[1];
	if(visible) {
	    cp->addActionListener(this, XtNmeasureCallback);
	    XtSetArg(args[0], XtNmeasureBox, True);
	}
	else {
	    cp->removeAllMeasureBoxes();
	    cp->removeActionListener(this, XtNmeasureCallback);
	    XtSetArg(args[0], XtNmeasureBox, False);
	}
	cp->setValues(args, 1);
    }
}

void MeasureAmpPer::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if(!strcmp(cmd, "Attributes...")) {
        table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Search Parameters...")) {
        search_param_window->setVisible(true);
    }
    else if(!strcmp(reason, XtNmeasureCallback)) {
	measurement((CPlotMeasure *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNattributeChangeCallback)) {
	list();
    }
    else if(!strcmp(reason, XtNselectRowCallback)) {
	bool set = table->rowSelected();
	save_selected_button->setSensitive(set);
	create_amp_button->setSensitive(set);
    }
    else if(!strcmp(cmd, "Clear All Boxes")) {
	if(cp) cp->removeAllMeasureBoxes();
	list();
    }
    else if(!strcmp(cmd, "Clear Selected Boxes")) {
	clearBoxes();
    }
    else if(!strcmp(cmd, "Measure All")) {
	measure(false);
    }
    else if(!strcmp(cmd, "Measure Selected")) {
	measure(true);
    }
    else if(!strncmp(cmd, "DFX ", 4)) {
	DFXMeasureAmp(cmd+4);
    }
    else if(!strcmp(cmd, "Search For Period") ||
		!strcmp(cmd, "Search Parameters")) { // from Param window
	search();
    }
    else if(!strcmp(cmd, "Save All")) {
	save(false);
    }
    else if(!strcmp(cmd, "Save Selected")) {
	save(true);
    }
    else if(!strcmp(cmd, "Create Amplitude")) {
	createAmplitudes();
    }
    else if(!strcmp(cmd, "Review")) {
	if(cp && review_toggle->state()) cp->reviewSelectedMeasurements();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Measure Help")) {
	showHelp("Measure Amp Per Help");
    }
    else if(!strcmp(reason, XtNphaseLineCallback) ||
	    !strcmp(reason, XtNlineCallback))
    {
	measure_all_button->setSensitive(true);
	measure_sel_button->setSensitive(true);
    }
}

static int num_warned = 0;
static char **warned=NULL;

void MeasureAmpPer::measurement(CPlotMeasure *m)
{
    int num_columns = table->numColumns();
    char *msg;
    vector<string> row;

    for(int i = 0; i < 1000 && m[i].w != NULL; i++)
    {
	int j;
	//	printf("DEBUG: amp_nms = %d\n", m[i].amp_nms);
	if(m[i].amp_nms < 0) {
	    for(j = 0; j < num_warned && strcmp(m[i].w->sta(), warned[j]); j++);
	    if(j == num_warned) {
		if(!warned) warned = (char **)malloc(sizeof(char *));
		else warned = (char **)realloc(warned,
				(num_warned+1)*sizeof(char *));
		warned[num_warned++] = strdup(m[i].w->sta());
		if((msg = GError::getMessage())) {
		    showWarning(
			"Cannot convert %s/%s measuremnent to nanometers.\n%s",
			m[i].w->sta(), m[i].w->chan(), msg);
		}
		else {
		    showWarning(
			"Cannot convert %s/%s measuremnent to nanometers.",
			m[i].w->sta(), m[i].w->chan());
		}
	    }
	}
	getRow(&m[i], num_columns, row);
	if(net_toggle->state()) {
	    // only one box per net
	    for(j = 0; j < ws.size() &&
		strcasecmp(m[i].w->net(), ws[j]->net()); j++);
	    if(j < ws.size() && m[i].w != ws[j]) {
		cp->removeMeasureBox(ws[j]);
		ws.set(m[i].w, j);
	    }
	}
	else {
	    for(j = 0; j < ws.size() && m[i].w != ws[j]; j++);
	}
	if(j < ws.size()) {
	    table->setRow(j, row);
	}
	else {
	    table->addRow(row, false);
	    ws.push_back(m[i].w);
	}
    }
    table->adjustColumns();
}

void MeasureAmpPer::list(void)
{
    vector<string> row;
    gvector<Waveform *> wvec;

    if(!cp) return;

    cp->getWaveforms(wvec, false);

    table->removeAllRows();

    ws.clear();

    int num_columns = table->numColumns();

    for(int i = 0; i < wvec.size(); i++)
    {
	CPlotMeasure m;
	if(cp->getWaveformMeasureBox(wvec[i], &m))
	{
	    getRow(&m, num_columns, row);
	    table->addRow(row, false);
	    ws.push_back(wvec[i]);
	}
    }
    table->adjustColumns();
}

void MeasureAmpPer::getRow(CPlotMeasure *m, int num_columns,vector<string> &row)
{
    double time = m->w->tbeg() + m->left_side;
    CssWfdiscClass *wf = m->w->ts->getWfdisc(time);
    GSegment *seg = m->w->segment(time);
    char s[100];

    if( !seg ) seg = m->w->segment(0);

    row.clear();

    for(int j = 0; j < num_columns; j++)
    {
	TAttribute a = table->getAttribute(j);
	s[0] = '\0';

	if(!strcasecmp(a.name, "amptype"))
	{
	    row.push_back(amp_type);
	}
	else if(!strcasecmp(a.name, "amp(hpp_cnt)"))
	{
	    snprintf(s, sizeof(s), a.format, (double)m->amp_cnts/2.);
	    if(s[0] == '\0') {
		snprintf(s, sizeof(s), "%.2f", (double)m->amp_cnts/2.);
	    }
	    row.push_back(string(s));
        }
	else if(!strcasecmp(a.name, "amp(hpp_nm)"))
	{
	    if(m->amp_nms >= 0.) {
		snprintf(s, sizeof(s), a.format, m->amp_nms/2.);
		if(s[0] == '\0') {
		    snprintf(s, sizeof(s), "%.2f", m->amp_nms/2.);
		}
	    }
	    else {
		strcpy(s, "NA");
	    }
	    row.push_back(string(s));
        }
	else if(!strcasecmp(a.name, "amp(hpp_Nnm)"))
	{
	    snprintf(s, sizeof(s), a.format, m->amp_Nnms/2.);
	    if(s[0] == '\0') {
		snprintf(s, sizeof(s), "%.2f", m->amp_Nnms/2.);
	    }
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "amp(pp_cnt)"))
	{
	    snprintf(s, sizeof(s), a.format, m->amp_cnts);
	    if(s[0] == '\0') {
		snprintf(s, sizeof(s), "%.2f", m->amp_cnts);
	    }
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "amp(pp_nm)"))
	{
	    if(m->amp_nms >= 0.) {
		snprintf(s, sizeof(s), a.format, m->amp_nms);
		if(s[0] == '\0') {
		    snprintf(s, sizeof(s), "%.2f", m->amp_nms);
		}
	    }
	    else {
		strcpy(s, "NA");
	    }
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "amp(pp_Nnm)"))
	{
	    snprintf(s, sizeof(s), a.format, m->amp_Nnms);
	    if(s[0] == '\0') {
		snprintf(s, sizeof(s), "%.2f", m->amp_Nnms);
	    }
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "period"))
	{
	    snprintf(s, sizeof(s), a.format, m->period);
	    if(s[0] == '\0') {
		snprintf(s, sizeof(s), "%.2f", m->period);
	    }
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "time"))
	{
	    if(!strcmp(a.format, "%g")) {
		timeEpochToString(time, s, sizeof(s), GSE20);
	    }
	    else if(!strcmp(a.format, "%G")) {
		timeEpochToString(time, s, sizeof(s), GSE21);
	    }
	    else if(!strcmp(a.format, "%t")) {
		timeEpochToString(time, s, sizeof(s), YMONDHMS);
	    }
	    else {
		snprintf(s, sizeof(s), a.format, time);
	    }
	    if(s[0] == '\0') {
		snprintf(s, sizeof(s), a.format, time);
	    }
	    row.push_back(string(s));
        }
	else if(!strcasecmp(a.name, "sta")) {
	    row.push_back(string(wf->sta));
	}
	else if(!strcasecmp(a.name, "chan")) {
	    row.push_back(string(wf->chan));
	}
	else if(!strcasecmp(a.name, "net")) {
	    row.push_back(string(m->w->net()));
	}
	else if(!strcasecmp(a.name, "instype")) {
	    row.push_back(string(wf->instype));
	}
	else if(!strcasecmp(a.name, "segtype")) {
	    row.push_back(string(wf->segtype));
	}
	else if(!strcasecmp(a.name, "datatype")) {
	    row.push_back(string(wf->datatype));
	}
	else if(!strcasecmp(a.name, "clip")) {
	    row.push_back(string(wf->clip));
	}
	else if(!strcasecmp(a.name, "dir")) {
	    row.push_back(string(wf->dir));
	}
	else if(!strcasecmp(a.name, "dfile")) {
	    row.push_back(string(wf->dfile));
	}
	else if(!strcasecmp(a.name, "wfid")) {
	    snprintf(s, sizeof(s), a.format, wf->wfid);
	    if(s[0] == '\0') snprintf(s, sizeof(s), "%ld", wf->wfid);
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "chanid")) {
	    snprintf(s, sizeof(s), a.format, wf->chanid);
	    if(s[0] == '\0') snprintf(s, sizeof(s), "%ld", wf->wfid);
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "calib")) {
	    snprintf(s, sizeof(s), a.format, seg->calib());
	    if(s[0] == '\0') snprintf(s, sizeof(s), "%.2f", seg->calib());
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "calper")) {
	    snprintf(s, sizeof(s), a.format, seg->calper());
	    if(s[0] == '\0') snprintf(s, sizeof(s), "%.2f", seg->calper());
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "samprate")) {
	    snprintf(s, sizeof(s), a.format, wf->samprate);
	    if(s[0] == '\0') snprintf(s, sizeof(s), "%.2f", wf->samprate);
	    row.push_back(string(s));
	}
	else if(!strcasecmp(a.name, "nsamp")) {
	    snprintf(s, sizeof(s), a.format, wf->nsamp);
	    if(s[0] == '\0') snprintf(s, sizeof(s), "%ld", wf->nsamp);
	    row.push_back(string(s));
	}
	else {
	    row.push_back(string("NA"));
	}
    }
}

void MeasureAmpPer::clearBoxes(void)
{
    if(!cp) return;
    vector<int> rows;
    table->getSelectedRows(rows);

    for(int i = 0; i < (int)rows.size(); i++)  {
	cp->removeMeasureBox(ws[rows[i]]);
    }
    list();
}

void MeasureAmpPer::save(bool selected_only)
{
    vector<CssArrivalClass *> arrivals;

    if( !getMeasuredArrivals(arrivals, selected_only) ) return;

    char s[30], s2[30];
    if((int)arrivals.size() == 1) {
	snprintf(s, sizeof(s), "Change 1 Arrival");
	strcpy(s2, "Change Arrival");
    }
    else {
	snprintf(s, sizeof(s), "Change %d Arrivals", (int)arrivals.size());
	strcpy(s2, "Change Arrivals");
    }
    if(Question::askQuestion("Confirm Change Arrival",this,s,s2,"Cancel") == 1)
    {
	setCursor("hourglass");
	changeAmpPer(arrivals);
	doCallbacks(baseWidget(), (XtPointer)NULL, XmNactivateCallback);
	setCursor("default");
    }
}

int MeasureAmpPer::getMeasuredArrivals(vector<CssArrivalClass *> &arr,
			bool selected_only)
{
    int i, j, n;
    vector<bool> states;
    CPlotMeasure m;
    cvector<CssArrivalClass> arrivals;
    cp->getTable(arrivals);

    if(!arrivals.size()) {
	showWarning("No arrivals.");
	return 0;
    }

    int num_rows = table->getRowStates(states);

    for(i = 0; i < arrivals.size(); i++)
	if(cp->isSelected(arrivals[i]))
    {
	if((n = cp->getArrivalMeasureBox(arrivals[i], &m)) == 1) {
	    for(j = 0; j < num_rows && ws[j] != m.w; j++);
	    if(j < num_rows && (!selected_only || states[j]) ) {
		arr.push_back(arrivals[i]);
	    }
	}
	else if(n > 1) {
	    showWarning("Multiple boxes for: %s %s %s",
		    arrivals[i]->iphase, arrivals[i]->sta, arrivals[i]->chan);
	    return 0;
	}
    }

    if( !(int)arr.size() ) {
	showWarning(
		"No waveforms with a selected arrival and a measurement box.");
    }
    return (int)arr.size();
}

void MeasureAmpPer::changeAmpPer(vector<CssArrivalClass *> &arrivals)
{
    char *err_msg;
    CPlotMeasure m;

    errorMsg(); // clear last error message
    setPrintError(false);

    for(int i = 0; i < (int)arrivals.size(); i++)
    {
	if(cp->getArrivalMeasureBox(arrivals[i], &m))
	{
            DataSource *ds = arrivals[i]->getDataSource();

	    arrivals[i]->amp_cnts = m.amp_cnts;
	    arrivals[i]->amp_Nnms = m.amp_Nnms;
	    arrivals[i]->amp_nms = m.amp_nms;
	    arrivals[i]->zp_Nnms = m.zp_Nnms;
	    arrivals[i]->period = m.period;
	    arrivals[i]->box_location = True;
	    arrivals[i]->boxtime = m.w->tbeg() + m.left_side;
	    arrivals[i]->boxmin = m.bottom_side;

	    if(ds)
	    {
		double amp = arrivals[i]->amp;
		double per = arrivals[i]->per;

		arrivals[i]->amp = -1.;
		if(m.amp_nms > 0) arrivals[i]->amp = .5*m.amp_nms;
		arrivals[i]->per = arrivals[i]->period;

		ds->changeArrival(arrivals[i], m.w->ts, CHANGE_AMP_PER);

		if( (err_msg = errorMsg()) ) {
		    showWarning("Cannot save amp for %s/%s\n%s",
			arrivals[i]->sta, arrivals[i]->chan, err_msg);
		    arrivals[i]->amp = amp;
		    arrivals[i]->per = per;
		}
	    }
	}
    }
    setPrintError(true);
}

void MeasureAmpPer::createAmplitudes(void)
{
    vector<CssArrivalClass *> arrivals;
    CPlotMeasure m;

    if(!cp) return;

    if( !getMeasuredArrivals(arrivals, true) ) return;

    errorMsg(); // clear last error message

    for(int i = 0; i < (int)arrivals.size(); i++)
    {
	if(cp->getArrivalMeasureBox(arrivals[i], &m))
	{
	    CssAmplitudeClass *amp = new CssAmplitudeClass();
	    amp->arid = arrivals[i]->arid;
	    stringcpy(amp->chan, m.w->chan(), sizeof(amp->chan));
	    amp->per = m.period;
	    amp->amptime = m.w->tbeg() + m.left_side;
	    amp->start_time = amp->amptime;
	    amp->duration = m.period;
	    stringcpy(amp->amptype, amp_type.c_str(), sizeof(amp->amptype));

	    // always save amplitude as nanometers half peak-to-peak
	    if(m.amp_nms > 0) amp->amp = .5*m.amp_nms;
            DataSource *ds = arrivals[i]->getDataSource();

	    if(ds && ds->addAmplitude(arrivals[i], amp))
	    {
		cp->putTable(amp);
	    }
	    else {
		amp->deleteObject();
	    }
	    showErrorMsg();
	}
    }
}

void MeasureAmpPer::measure(bool selected)
{
    if(!cp) return;
    cp->removeAllMeasureBoxes();
    bool with_arrival = false;
    cp->measureAll(selected, with_arrival);
    list();
}

void MeasureAmpPer::search(void)
{
    if(!cp || !cp->numWaveforms()) return;

    cp->removeAllMeasureBoxes();

    double min_per, max_per;
    if( !search_param_window->getDouble("Minimum Period", &min_per) ) {
	showWarning("Invalid Minimum Period");
	return;
    }
    if( !search_param_window->getDouble("Maximum Period", &max_per) ) {
	showWarning("Invalid Maximum Period");
	return;
    }

    setCursor("hourglass");

    int ret;
    if((ret = cp->search(min_per, max_per, false)) == 1) {
	list();
    }
    else if(ret == 0) {
	showWarning(
	    "Cannot find period between %.2f and %.2f over specified interval",
		min_per, max_per);
    }
    else if(ret == -1) {
	showWarning(cp->getError());
    }
    setCursor("default");

    list();
}

void MeasureAmpPer::DFXMeasureAmp(const char *type)
{
    int i, j, j1, n, npts, windowed, interp_per=0, ford, zp;
    gvector<Waveform *> wvec;
    Waveform *w;
    char ftype[10];
    CPlotMeasure m;
    GSegment *segment;
    IIRFilter *iir = NULL;
    float *y;
    double samprate, tbeg, value=0., thresh=0., flo, fhi, amp, per, time;

    if(!cp) return;

    interp_per = dfx_interp_toggle->state();

    cp->getSelectedWaveforms("a", wvec);
    windowed = 1;
    if(wvec.size() <= 0 && !cp->doubleLineIsDisplayed("a"))
    {
	windowed = 0;
	cp->getSelectedWaveforms(wvec);
    }

    if(wvec.size() <= 0)
    {
	showWarning("No selected waveforms.");
	return;
    }

    for(i = 0; i < wvec.size(); i++)
    {
	w = wvec[i];
	if(windowed) {
	    GDataPoint *d1 = w->dw[0].d1;
	    GDataPoint *d2 = w->dw[0].d2;
	    y = d1->segment()->data + d1->index();
	    samprate = (d1->segment()->tdel() > 0) ?
				1./d1->segment()->tdel() : 1.;
	    tbeg = d1->segment()->tbeg() + d1->segment()->tdel()*d1->index();
	    segment = d1->segment();

	    if(d1->segment() != d2->segment()) {
		showWarning("%s/%s: processing only the first segment.",
			w->sta(), w->chan());
		npts = d1->segment()->length() - d1->index();
	    }
	    else {
		npts = d2->index() - d1->index() + 1;
	    }
	}
	else {
	    y = w->segment(0)->data;
	    npts = w->segment(0)->length();
	    samprate = (w->segment(0)->tdel() > 0) ?
				1./w->segment(0)->tdel() : 1.;
	    tbeg = w->segment(0)->tbeg();
	    segment = w->segment(0);
	    if(w->ts->size() > 1) {
		showWarning("%s/%s: processing only the first segment.",
			w->sta(), w->chan());
	    }
	}

	stringcpy(ftype, "NO", sizeof(ftype));
	flo = 0.;
	fhi = 1.;
	ford = 1;
	zp = 0;

	if( (iir = (IIRFilter *)w->ts->getMethod("IIRFilter")) &&
			strcmp(iir->getType(), "NA"))
	{
	    stringcpy(ftype, iir->getType(), sizeof(ftype));
	    flo = iir->getFlow();
	    fhi = iir->getFhigh();
	    ford = iir->getOrder();
	    zp = iir->getZeroPhase();
	}

	measure_amplitude(y, npts, tbeg, samprate, (char *)type, value, thresh,
		flo, fhi, ford, zp, ftype, interp_per, &amp, &per, &time);

	if(w->ts->getMethod("CalibData")) {
	    m.amp_Nnms = amp;
	    m.amp_cnts = (segment->calib() != 0.) ? amp/segment->calib() : amp;
	}
	else {
	    m.amp_cnts = amp;
	    m.amp_Nnms = (segment->calib() != 0.) ? amp*segment->calib() : amp;
	}
	m.w = w;
	m.amp_nms = 0;
	m.zp_Nnms = 0;
	m.period = per;
	m.left_side = time - w->tbeg();
	n = (int)(per*samprate) + 1;
	j1 = (int)((time - tbeg)*samprate);
	if(j1 < npts) m.bottom_side = y[j1];
	for(j = 0; j < n; j++) {
	    if(j1+j < npts && m.bottom_side > y[j1+j]) {
		m.bottom_side = y[j1+j];
	    }
	}
	cp->addMeasurement(&m, w);
    }

    list();
}
