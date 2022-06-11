/** \file Resp.cpp
 *  \brief Defines class Resp.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <algorithm>
using namespace std;

#include "Resp.h"
#include "motif++/MotifClasses.h"
#include "Demean.h"
#include "libgx++.h"
#include "ResponseFile.h"
#include "RespTapers.h"
#include "ConvolveData.h"
#include "CalibData.h"
#include "TaperData.h"
#include "widget/Tab.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}
static void setResponseAlignment(Table *table);
static bool hasOriginalResponse(GTimeSeries *ts);

using namespace libgrsp;

Resp::Resp(const char *name, Component *parent, DataSource *ds) :
		Frame(name, parent, true), DataReceiver(ds)
{
    createInterface();
    init();
}

void Resp::createInterface()
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    open_button = new Button("Open...", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    view_menu = new Menu("View", menu_bar);
    cascade_toggle = new Toggle("Cascade", view_menu, this);
    cascade_toggle->set(true);
    grid_toggle = new Toggle("Grid", view_menu, this);
    grid_toggle->set(true);
    axes_menu = new Menu("Axes", view_menu, true);
    log_xy_toggle = new Toggle("Log x-y", axes_menu, this, true);
    log_x_toggle = new Toggle("Log x", axes_menu, this, true);
    log_y_toggle = new Toggle("Log y", axes_menu, this, true);
    log_y_toggle->set(true);
    linear_xy_toggle = new Toggle("Linear x-y", axes_menu, this, true);
    display_menu = new Menu("Display", view_menu, true);
    amp_toggle = new Toggle("Amplitude", display_menu, this, true);
    amp_toggle->set(true);
    power_toggle = new Toggle("Power", display_menu, this, true);

    option_menu = new Menu("Option", menu_bar);

    convolve_button = new Button("Convolve", option_menu, this);
    convolve_button->setSensitive(false);
    deconvolve_button = new Button("Deconvolve", option_menu, this);
    deconvolve_button->setSensitive(false);

    remove_time_shift_toggle = new Toggle("Remove Time Shift", option_menu,
					this);
    remove_time_shift_toggle->set(true);
    original_button = new Button("Return to Original", option_menu, this);
    original_button->setSensitive(false);
    tapers_button = new Button("Tapers...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    resp_help_button = new Button("Response Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XtNheight, 600); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNheight, 300); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    tab = new TabClass("tab", pane, args, n);

    n = 0;
    XtSetArg(args[n], XtNheight, 300); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    XtSetArg(args[n], XtNyLabelInt, False); n++;
    XtSetArg(args[n], XtNextraXTickmarks, True); n++;
    XtSetArg(args[n], XtNextraYTickmarks, True); n++;
    XtSetArg(args[n], XtNxLabel, "Frequency (Hz)"); n++;
    amp_plot = new CPlotClass("Amplitude", tab, info_area, args, n);

    XtSetArg(args[n], XtNyLabel, "Phase"); n++;
    phase_plot = new CPlotClass("Phase", tab, info_area, args, n);
    n--;

    inverse_plot = new CPlotClass("Tapered Inverse", tab, info_area, args, n);
    n--;

    XtSetArg(args[n], XtNxLabel, "Time (secs)"); n++;
    time_plot = new CPlotClass("Time", tab, info_area, args, n);
    tab->setOnTop("Amplitude");

    n = 0;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNheight, 300); n++;
    table_tab = new TabClass("table_tab", pane, args, n);
    table_tab->addActionListener(this, XtNtabCallback);

    rps.form = new Form("Instruments", table_tab);
    all.form = new Form("All Instruments", table_tab);
    table_tab->setOnTop("Instruments");

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 5); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNtableTitle, "Responses"); n++;
    XtSetArg(args[n], XtNcolumns, 23); n++;
    const char *resp_labels[] = {"no.", "source", "stage", "description",
	"type", "input", "output", "author", "npoles", "nzeros","nfap",
	"samprate", "numerator", "denominator", "gse-sta", "gse-chan",
	"gse-auxid", "instype", "calib", "calper", "calfreq", "ondate",
	"offdate"};

    XtSetArg(args[n], XtNcolumnLabels, resp_labels); n++;
    rps.response_table = new Table("Responses", rps.form, args, n);
    rps.response_table->addActionListener(this, XtNselectRowCallback);

    all.response_table = new Table("Responses", all.form, args, n);
    all.response_table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rps.response_table->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    rps.instrument_table = new ctable<CssInstrumentClass>("Instruments", rps.form,
		args, n);
    const char *extra[] = {"no.", "sta", "chan"};
    const char *formats[] = {"%d", "%s", "%s"};
    const char *display_list = "no.,sta,chan,inid,insname,instype,band,digital,\
samprate,ncalib,ncalper,dir,dfile,rsptype,lddate";
    rps.instrument_table->setType(cssInstrument, 3, extra,formats,display_list);
    rps.instrument_table->addActionListener(this, XtNselectRowCallback);


    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, all.response_table->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    all.instrument_table = new ctable<CssInstrumentClass>("Instruments", all.form,
		args, n);
    const char *all_extra[] = {"no."};
    const char *fmts[] = {"%d"};
    const char *display= "no.,dfile,inid,insname,instype,band,digital,\
samprate,ncalib,ncalper,dir,rsptype,lddate";
    all.instrument_table->setType(cssInstrument, 1, all_extra, fmts, display);
    all.instrument_table->addActionListener(this, XtNselectRowCallback);

    ignore_data_change = false;
    print_window = NULL;
    resp_tapers = new RespTapers("Response Tapers", this, 5., .01, 0., -6.);

    addPlugins("Resp", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(convolve_button, "Convolve");
	tool_bar->add(deconvolve_button, "Deconvolve");
    }
}

Resp::~Resp(void)
{
}

void Resp::init(void)
{
    listAllInstruments();

    listWaveformInstruments();
    if(data_source) {
	data_source->addDataListener(this);
    }
}

void Resp::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    DataChange *c;
    RespStruct *rs;

    rs = (!strcasecmp(table_tab->labelOnTop(), "Instruments")) ? &rps : &all;

    if(comp == table_tab) {
//	waveformResponse(rs, CPLOT_ADJUST_Y_GROW);
	waveformResponse(rs, CPLOT_ADJUST_LIMITS);
	setButtonsSensitive(rs);
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Open...")) {
	open();
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Tapers...")) {
	resp_tapers->setVisible(true);
    }
    else if(!strcmp(cmd, "Grid") || !strcmp(cmd, "Cascade")) {
//	waveformResponse(rs, CPLOT_ADJUST_Y_GROW);
	waveformResponse(rs, CPLOT_ADJUST_LIMITS);
    }
    else if(!strncmp(cmd, "Log", 3) || !strncmp(cmd, "Linear", 6) ||
	!strcmp(cmd, "Amplitude") || !strcmp(cmd, "Power"))
    {
	Toggle *t = comp->getToggleInstance();
	if(t && t->state()) waveformResponse(rs, CPLOT_ADJUST_LIMITS);
    }
    else if(comp == rps.instrument_table) { // select row
	listResponses(&rps);
	setButtonsSensitive(rs);
    }
    else if(comp == all.instrument_table) { // select row
	listResponses(&all);
	setButtonsSensitive(rs);
    }
    else if(!strcmp(cmd, "Remove Time Shift") ||
	    !strcmp(cmd, "Convert to Displacement"))
    {
	waveformResponse(rs, CPLOT_ADJUST_LIMITS);
    }
    else if(comp == rs->response_table) { // select row
//	waveformResponse(rs, CPLOT_ADJUST_Y_GROW);
	waveformResponse(rs, CPLOT_ADJUST_LIMITS);
    }
    else if(comp == resp_tapers) { // apply response tapers
//	waveformResponse(rs, CPLOT_ADJUST_Y_GROW);
	waveformResponse(rs, CPLOT_ADJUST_LIMITS);
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	if(!ignore_data_change) {
	    c = (DataChange *)action_event->getCalldata();
	    if(c->waveform) {
		listWaveformInstruments();
	    }
	    if(c->waveform || c->select_waveform) {
		setButtonsSensitive(rs);
	    }
	}
    }
    else if(comp == convolve_button) {
	convolve(1);
    }
    else if(comp == deconvolve_button) {
	convolve(-1);
    }
    else if(comp == original_button) {
	gvector<Waveform *> wvec;
	if( !data_source->getSelectedWaveforms(wvec) ) {
	    showWarning("No waveforms selected.");
	}
	else {
	    removeMethods(wvec);
	}
    }
    else if(!strcmp(cmd, "Response Help")) {
	showHelp("Instrument Response Help");
    }
}

void Resp::setButtonsSensitive(RespStruct *rs)
{
    gvector<Waveform *> wvec;
    bool state=false;
    if(data_source->getSelectedWaveforms(wvec) > 0) {
	if(rs->response_table->rowSelected()) {
	    state = true;
	}
	original_button->setSensitive(true);
    }
    else {
	original_button->setSensitive(false);
    }
    convolve_button->setSensitive(state);
    deconvolve_button->setSensitive(state);
}

ParseCmd Resp::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if(parseArg(cmd, "Cascade", c)) {
	return cascade_toggle->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Convolve")) {
	convolve(1);
    }
    else if(parseCompare(cmd, "Convolve ", 9)) {
	return parseConvolve(1, cmd.substr(9), msg);
    }
    else if(parseCompare(cmd, "Deconvolve", 10)) {
	return parseConvolve(-1, cmd.substr(10), msg);
    }
    else if(parseCompare(cmd, "Return_to_Original") ||
		parseCompare(cmd, "remove_con_decon")) {
	original_button->activate();
    }
    else if(parseCompare(cmd, "Return_to_Original ", 19)) {
	return parseConvolve(0, cmd.substr(19), msg);
    }
    else if(parseCompare(cmd, "remove_con_decon ", 17)) {
	return parseConvolve(0, cmd.substr(17), msg);
    }
    else if(parseArg(cmd, "Grid", c)) {
	return grid_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Axes", c)) {
	return axes_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Display", c)) {
	return display_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "instruments", c)) {
	table_tab->setOnTop("Instruments");
	return rps.instrument_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "all_instruments", c)) {
	table_tab->setOnTop("All Instruments");
	return all.instrument_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "instruments.responses", c)) {
	table_tab->setOnTop("Instruments");
	return rps.response_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "all_instruments.responses", c)) {
	table_tab->setOnTop("All Instruments");
	return all.response_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Data_Taper", c)) {
	resp_tapers->data_taper_text->setString(c, true);
    }
    else if(parseArg(cmd, "Low_Pass", c)) {
	resp_tapers->low_pass_text->setString(c, true);
    }
    else if(parseArg(cmd, "High_Pass", c)) {
	resp_tapers->high_pass_text->setString(c, true);
    }
    else if(parseString(cmd, "print_window", c)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print Response", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = all.instrument_table->parseCmd(cmd, msg))
		!= COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else if((ret = Frame::parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd Resp::parseConvolve(int direction, const string &s, string &msg)
{
    string sta, chan;
    int i, j, id = -1, num_resp;
    vector<int> selected;
    bool instr;
    ParseCmd ret;
    gvector<Waveform *> wvec;
    Waveform *w=NULL;

    if((ret = parseParams(s, &id, &instr, msg)) != COMMAND_PARSED) {
	return ret;
    }
    if(id >= 0 && !(w = data_source->getWaveform(id)) ) {
	msg.assign("Waveform not found.");
	return ARGUMENT_ERROR;
    }

    if(w) {
	wvec.push_back(w);
    }
    else {
	if(data_source->getSelectedWaveforms(wvec) <= 0) {
	    showWarning("No waveforms selected.");
	    return COMMAND_PARSED;
	}
    }
    if(direction == 0) {
	removeMethods(wvec);
	return COMMAND_PARSED;
    }

    if(instr) { // instrument both specified
	if( !(num_resp = all.response_table->getSelectedRows(selected)) ) {
	    showWarning("No responses found.");
	    return COMMAND_PARSED;
	}
	table_tab->setOnTop("All Instruments");
	conDecon(direction, num_resp, selected, wvec);
	return COMMAND_PARSED;
    }

    if(direction == 1) { // convolve
	table_tab->setOnTop("All Instruments");
	if(!all.instrument_table->rowSelected()) {
	    msg.assign("No instrument selected.");
	    return ARGUMENT_ERROR;
	}
	if( !(num_resp = all.response_table->getSelectedRows(selected)) ) {
	    showWarning("No responses found.");
	    return COMMAND_PARSED;
	}
	conDecon(1, num_resp, selected, wvec);
	return COMMAND_PARSED;
    }

    table_tab->setOnTop("Instruments");

    // deconvolve
    rps.instrument_table->deselectAllRows();
    cvector<CssInstrumentClass> v;
    rps.instrument_table->getRecords(v);

    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < v.size(); j++) {
	    if(v[j]->getValue("sta", sta) && v[j]->getValue("chan", chan)
		&& !strcasecmp(sta.c_str(), wvec[i]->sta())
		&& DataSource::compareChan(chan, wvec[i]->chan()))
	    {
		rps.instrument_table->deselectAllRows();
		rps.instrument_table->selectRowWithCB(j, true);
		if(!(num_resp=rps.response_table->getSelectedRows(selected))) {
		    showWarning("No responses not found for %s/%s",
				sta.c_str(), chan.c_str());
		}
		else {
		    table_tab->setOnTop("Instruments");
		    gvector<Waveform *> ws(wvec[i]);
		    conDecon(-1, num_resp, selected, ws);
		}
		break;
	    }
	}
	if(j == v.size()) {
	    showWarning("No response found for %s/%s", wvec[i]->sta(),
			wvec[i]->chan());
	}
    }

    return COMMAND_PARSED;
}

ParseCmd Resp::parseParams(const string &cmd, int *id, bool *instr, string &msg)
{
    int i;
    bool b;
    long addr, inid;
    string c;
    const char *args[] = {"_wave_", "_instrument_", "inid", "low", "high",				"data_taper", "remove_time_shift", "amp_cutoff"};

    parseGetArg(cmd, "convolve", msg, "_wave_", id);

    *instr = false;
    if(parseGetArg(cmd, "convolve", msg, "_instrument_", &addr)) {
	cvector<CssInstrumentClass> v;
	all.instrument_table->getRecords(v);
	if(!v.contains((CssInstrumentClass *)addr)) {
	    msg.assign("instrument not found");
	    return ARGUMENT_ERROR;
	}
	all.instrument_table->deselectAllRows();
	all.instrument_table->selectRecord((CssTableClass *)addr, true, true);
	*instr = true;
    }
    if(parseGetArg(cmd, "convolve", msg, "inid", &inid)) {
	cvector<CssInstrumentClass> v;
	all.instrument_table->getRecords(v);
	for(i = 0; i < v.size(); i++) {
	    if(v[i]->inid == inid) {
		all.instrument_table->deselectAllRows();
		all.instrument_table->selectRecord(v[i], true, true);
		*instr = true;
		break;
	    }
	}
	if(i == v.size()) {
	    ostringstream os;
	    os << "inid not found: " << inid;
	    msg.assign(os.str());
	    return ARGUMENT_ERROR;
	}
    }
    if(parseGetArg(cmd, "low", c)) {
	resp_tapers->low_pass_text->setString(c, true);
    }
    if(parseGetArg(cmd, "high", c)) {
	resp_tapers->high_pass_text->setString(c, true);
    }
    if(parseGetArg(cmd, "data_taper", c)) {
	resp_tapers->data_taper_text->setString(c, true);
    }
    if(parseGetArg(cmd, "amp_cutoff", c)) {
	resp_tapers->amp_cutoff_text->setString(c, true);
    }
    if(parseGetArg(cmd, "convolve", msg, "remove_time_shift", &b)) {
	remove_time_shift_toggle->set(b, true);
    }
    if(resp_tapers->params_ok) {
	resp_tapers->apply();
    }
    else {
	resp_tapers->warn->getLabel(msg);
	return ARGUMENT_ERROR;
    }
    return unknownArgs(cmd, msg, 8, args) ? ARGUMENT_ERROR : COMMAND_PARSED;
}

ParseVar Resp::parseVar(const string &name, string &value)
{
    ParseVar ret;
    string c;
/*
    RespStruct *rs;

    if((ret =all.instrument_table->parseVar(name, value)) != VARIABLE_NOT_FOUND)
    {
	return ret;
    }
	
    rs = (!strcasecmp(table_tab->labelOnTop(), "Instruments")) ? &rps : &all;
*/

    if(parseArg(name, "instruments.responses", c)) {
	return rps.response_table->parseVar(c, value);
    }
    else if(parseArg(name, "all_instruments.responses", c)) {
	return all.response_table->parseVar(c, value);
    }
    else if(parseArg(name, "instruments", c)) {
	return rps.instrument_table->parseVar(c, value);
    }
    else if(parseArg(name, "all_instruments", c)) {
	return all.instrument_table->parseVar(c, value);
    }
    else if((ret = Frame::parseVar(name, value)) != VARIABLE_NOT_FOUND)
    {
	return ret;
    }
    else {
	return FormDialog::parseVar(name, value);
    }
    return STRING_RETURNED;
}

void Resp::parseHelp(const char *prefix)
{
    printf("%scascade=(true,false)\n", prefix);
    printf("%sconvolve\n", prefix);
    printf("%sdeconvolve\n", prefix);
    printf("%sgrid=(true,false)\n", prefix);
    printf("%sreturn_to_original\n", prefix);
    printf("%saxes=(log x-y,log x,log y,linear x-y)\n", prefix);
    printf("%sdisplay=(amplitude,power,phase)\n", prefix);

    char p[200];
    snprintf(p, sizeof(p), "%sinstruments.", prefix);
    Table::parseHelp(p);
    snprintf(p, sizeof(p), "%sresponses.", prefix);
    Table::parseHelp(p);

    printf("%sdata_taper=SECONDS\n", prefix);
    printf("%slow_pass=FREQ\n", prefix);
    printf("%shigh_pass=FREQ\n", prefix);
    printf("%sprint.help\n\n", prefix);
}

void Resp::print(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print Response", this, this);
    }
    print_window->setVisible(true);
}

void Resp::print(FILE *fp, PrintParam *ps)
{
    AxesParm *a=NULL;
    char *s = tab->labelOnTop();

    if(s && !strcasecmp(s, "Time")) {
	a = time_plot->hardCopy(fp, ps, NULL);
    }
    else if(s && !strcasecmp(s, "Amplitude")) {
	a = amp_plot->hardCopy(fp, ps, NULL);
    }
    else if(s && !strcasecmp(s, "Phase")) {
	a = phase_plot->hardCopy(fp, ps, NULL);
    }
    else {
	a = inverse_plot->hardCopy(fp, ps, NULL);
    }
    Free(a);
}

void Resp::listWaveformInstruments(void)
{
    gvector<Waveform *> wvec;
    CssInstrumentClass *ins;

    if(data_source) {
	data_source->getWaveforms(wvec, false);
    }

    rps.instrument_table->removeAllRecords();

    for(int i = 0; i < wvec.size(); i++)
	if(wvec[i]->length() > 2)
    {
	vector<Response *> *r = BasicSource::getResponse(wvec[i]->ts, true);
	ResponseFile *rf = wvec[i]->ts->response_file;
	if(r != NULL && rf != NULL)
	{
	    if( (ins = ResponseFile::getInstrument(rf, wvec[i]->ts->inid)) )
	    {
		ins->putValue("sta", (char *)wvec[i]->sta());
		ins->putValue("chan", (char *)wvec[i]->chan());
		rps.instrument_table->addRecord(ins, false);
	    }
	}
	else if(r != NULL && (int)r->size() > 0) { // Response from Seed file
	    ins = new CssInstrumentClass();
	    ins->putValue("sta", (char *)wvec[i]->sta());
	    ins->putValue("chan", (char *)wvec[i]->chan());
	    ins->putValue("seed", wvec[i]->ts);
	    ins->samprate = r->back()->output_samprate;
	    ins->ncalib = wvec[i]->segment(0)->calib();
	    ins->ncalper = wvec[i]->segment(0)->calper();
	    snprintf(ins->insname, sizeof(ins->insname), "%s",
		r->front()->insname.c_str());
	    snprintf(ins->dfile, sizeof(ins->dfile), "%s",
		r->front()->author.c_str());
	    rps.instrument_table->addRecord(ins, false);
	}
    }

    rps.instrument_table->list();

    rps.response_table->removeAllRows();
    rps.response_table->adjustColumns();

    if(!strcasecmp(table_tab->labelOnTop(), "Instruments")) {
	amp_plot->clearWaveforms();
	phase_plot->clearWaveforms();
	inverse_plot->clearWaveforms();
	time_plot->clearWaveforms();
    }
}

void Resp::listAllInstruments(void)
{
    long no;
    cvector<CssInstrumentClass> v;

    ResponseFile::getInstruments(v);
    v.sortByMember("dfile");

    for(int i = 0; i < v.size(); i++) {
	no = i+1;
	v[i]->putValue("no.", no);
    }
    all.instrument_table->removeAllRecords();
    all.instrument_table->addRecords(v);

    if(!strcasecmp(table_tab->labelOnTop(), "All Instruments")) {
	amp_plot->clearWaveforms();
	phase_plot->clearWaveforms();
	inverse_plot->clearWaveforms();
	time_plot->clearWaveforms();
    }
}

static void
setResponseAlignment(Table *table)
{
    int alignment[22] =
	{RIGHT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY, LEFT_JUSTIFY,
	LEFT_JUSTIFY, LEFT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY,
	RIGHT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY,
	RIGHT_JUSTIFY, RIGHT_JUSTIFY, LEFT_JUSTIFY, LEFT_JUSTIFY,
	LEFT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY,
	RIGHT_JUSTIFY, RIGHT_JUSTIFY};

	table->setAlignment(22, alignment);
}

void Resp::listResponses(RespStruct *rs)
{
    int i, j, m;
    long no=0;
    cvector<CssInstrumentClass> v;
    GTimeSeries *ts;
    ResponseFile *rf;

    amp_plot->clearWaveforms();
    phase_plot->clearWaveforms();
    inverse_plot->clearWaveforms();
    time_plot->clearWaveforms();

    rs->response_table->removeAllRows();
    setResponseAlignment(rs->response_table);

    rs->instrument_table->getSelectedRecords(v);

    if( !v.size() ) {
	rs->response_table->adjustColumns();
	return;
    }

    rs->response_table->adjustColumns();

    rs->responses.clear();
    rs->response_files.clear();
    rs->instrument_index.clear();

    for(i=0, m=0; i < v.size(); i++) 
    {
	if((ts = (GTimeSeries *)v[i]->getValue("seed"))) {
	    vector<Response *> *r = BasicSource::getResponse(ts, true);
	    rf = new ResponseFile();
	    for(j = 0; r && j < (int)r->size(); j++) {
		rf->responses.push_back(r->at(j));
	    }
	    for(j = 0; r && j < (int)r->size(); j++)
	    {
		addResponseRow(rs->response_table, r->at(j), m++);

		rs->responses.push_back(r->at(j));
		rs->response_files.push_back(rf);
		rs->instrument_index.push_back(no);
	    }
	}
	else {
	    try {
		rf = ResponseFile::readFile(v[i]);
	    }
	    catch(...) { rf= NULL; }
	    if(rf) {
		v[i]->getValue("no.", &no);

		for(j = 0; j < (int)rf->responses.size(); j++)
		{
		    addResponseRow(rs->response_table, rf->responses[j], m++);
		    rs->responses.push_back(rf->responses[j]);
		    rs->response_files.push_back(rf);
		    rs->instrument_index.push_back(no);
		}
	    }
	    else {
		showWarning(GError::getMessage());
	    }
	}
    }

    amp_plot->clearWaveforms();
    phase_plot->clearWaveforms();
    inverse_plot->clearWaveforms();
    time_plot->clearWaveforms();

    rs->response_table->adjustColumns();
    rs->response_table->selectAllRows(true);
    waveformResponse(rs, CPLOT_ADJUST_LIMITS);
}

void Resp::addResponseRow(Table *table, Response *r, int row_num)
{
    const char *row[23];
    char samprate[100], stage[100], npoles[100], nzeros[100],
		nfap[100], num_n[100], num_d[100], calib[100], calper[100],
		ondate[100], offdate[100], rowNum[20];

    snprintf(rowNum, sizeof(rowNum), "%d", row_num+1);
    row[0] = rowNum;
    row[1] = r->source.c_str();
    snprintf(stage, sizeof(stage), "%d", r->stage);
    row[2] = stage;
    row[3] = r->des.c_str();
    row[4] = r->type.c_str();
    row[5] = r->response_units.c_str();
    row[6] = r->output_units.c_str();
    row[7] = r->author.c_str();
    snprintf(npoles, sizeof(npoles), "%d", r->npoles);
    row[8] = npoles;
    snprintf(nzeros, sizeof(nzeros), "%d", r->nzeros);
    row[9] = nzeros;
    snprintf(nfap, sizeof(nfap), "%d", r->nfap);
    row[10] = nfap;
    snprintf(samprate, sizeof(samprate), "%.7g", r->output_samprate);
    row[11] = samprate;
    snprintf(num_n, sizeof(num_n), "%d", r->num_n);
    row[12] = num_n;
    snprintf(num_d, sizeof(num_d), "%d", r->num_d);
    row[13] = num_d;

    for(int i = 14; i < 23; i++) row[i] = "-";

    if(r->cal != NULL) {
	row[14] = r->cal->sta;
	row[15] = r->cal->chan;
	row[16] = r->cal->auxid;
	row[17] = r->cal->instype;
	snprintf(calib, sizeof(calib), "%.6g", r->cal->calib);
	row[18] = calib;
	snprintf(calper, sizeof(calper), "%.6g", r->cal->calper);
	row[19] = calper;
	snprintf(ondate, sizeof(ondate), "%04d-%02d-%02d", r->cal->ondate.year,
		r->cal->ondate.month, r->cal->ondate.day);
	row[21] = ondate;
	snprintf(offdate, sizeof(offdate), "%04d-%02d-%02d",
	    r->cal->offdate.year, r->cal->offdate.month, r->cal->offdate.day);
	row[22] = offdate;
    }
    else  if(!r->source.compare(0, 9, "blockette")) {
	snprintf(calib, sizeof(calib), "%.6g", r->b58_sensitivity);
	row[18] = calib;
	snprintf(calper, sizeof(calper), "%.6g", r->b58_frequency);
	row[20] = calper; // really frequency
    }
    table->addRow(row, false);
}

void Resp::waveformResponse(RespStruct *rs, short adjust_limits)
{
    int num_selected, *selected = NULL;

    amp_plot->clearWaveforms();
    phase_plot->clearWaveforms();
    inverse_plot->clearWaveforms();
    time_plot->clearWaveforms();
    if( !(num_selected = rs->response_table->getSelectedRows(&selected)) ) {
	return;
    }

    int  i, j, l, n, num_waveform_colors;
    int  inid = -1;
    double samprate, calper, calib;
    Pixel *waveform_fg = NULL;
    CssInstrumentClass *ins;
    Pixel fg;

    num_waveform_colors = WaveformPlot::getWaveformColors(this, &waveform_fg);

    /* make sure all selected indices are between 0 and elementCount
     */
    for(i = j = 0; i < num_selected; i++) {
	if(selected[i] >= 0 && selected[i] < (int)rs->responses.size()) {
	    selected[j++] = selected[i];
	}
    }
    num_selected = j;
    vector<Response *> v;

    if(cascade_toggle->state())
    {
	for(i = l = 0; i < num_selected; i++, l++)
	{
	    Response *r = rs->responses[selected[i]];
	    ResponseFile *rf = rs->response_files[selected[i]];
	    if( (ins = ResponseFile::getInstrument(rf, inid)) ) {
		samprate = ins->samprate;
		calper = ins->ncalper;
		calib = ins->ncalib;
	    }
	    else {
		// seed file: find lowest sample rate from all stages
		samprate = r->output_samprate;
		for(j = 0; j < (int)rf->responses.size(); j++) {
		    if(rf->responses[j]->output_samprate < samprate) {
			samprate = rf->responses[j]->output_samprate;
		    }
		}
		calper = 0.;
		calib = 0.;
	    }
	    v.clear();
	    v.push_back(r);
	    for(j = i+1; j < num_selected &&
			rf == rs->response_files[selected[j]]; j++)
	    {
		v.push_back(rs->responses[selected[j]]);
		i++;
	    }
	    fg = waveform_fg[l % num_waveform_colors];
	    n = (i < num_selected-1) ? rs->instrument_index[i] : -1;
	    drawResponse(v, fg, n, samprate, calper, calib, adjust_limits);
	}
    }
    else {
	for(i = 0; i < num_selected; i++)
	{
	    Response *r = rs->responses[selected[i]];
	    ResponseFile *rf = rs->response_files[selected[i]];
	    samprate = 0.;
	    calper = 0.;
	    calib = 0.;
	    if( (ins = ResponseFile::getInstrument(rf, inid)) ) {
		samprate = ins->samprate;
		if((int)rf->responses.size() == 1) {
		    calper = ins->ncalper;
		    calib = ins->ncalib;
		}
	    }
	    else {
		// seed file: find lowest sample rate from all stages
		samprate = r->output_samprate;
		for(j = 0; j < (int)rf->responses.size(); j++) {
		    if(rf->responses[j]->output_samprate < samprate) {
			samprate = rf->responses[j]->output_samprate;
		    }
		}
		calper = 0.;
		calib = 0.;
	    }
	    v.clear();
	    v.push_back(r);
	    fg = waveform_fg[i % num_waveform_colors];
	    drawResponse(v, fg, selected[i]+1, samprate, calper, calib,
				adjust_limits);
	}
    }
    Free(selected);
    Free(waveform_fg);
}

void Resp::drawResponse(vector<Response *> &resp, Pixel fg, int selected,
	double samprate, double calper, double calib, short adjust_limits)
{
    char lab[20];
    Arg	args[10];
    bool remove_time_shift;
    int i, nf, np2;
    float *amp = NULL, *phase = NULL, *inverse = NULL;
    double *freq = NULL, *real=NULL, *imag=NULL, *re=NULL, *im=NULL, *p=NULL;
    double df, fmax=40., taper_secs, flo, fhi, amp_cutoff, ymin, ymax;
    double rad_to_degree = 180./M_PI;
     
    if((int)resp.size() == 0) return;

    np2 = 16384;
    nf = np2/2 + 1;

    if(!(real = (double *)mallocWarn(nf*sizeof(double)))) return;
    if(!(imag = (double *)mallocWarn(nf*sizeof(double)))) return;
    if(!(re = (double *)mallocWarn(nf*sizeof(double)))) return;
    if(!(im = (double *)mallocWarn(nf*sizeof(double)))) return;
    if(!(freq = (double *)mallocWarn(nf*sizeof(double)))) return;
    if(!(amp = (float *)mallocWarn(nf*sizeof(float)))) return;
    if(!(inverse = (float *)mallocWarn(nf*sizeof(float)))) return;
    if(!(phase = (float *)mallocWarn(nf*sizeof(float)))) return;
    if(!(p = (double *)mallocWarn(nf*sizeof(double)))) return;


    resp_tapers->getParams(&taper_secs, &flo, &fhi, &amp_cutoff);

    if(samprate > 0.) {
	fmax = .5*samprate;
    }
    else {
	for(i = 0; i < (int)resp.size(); i++) {
	    if(!strcasecmp(resp[i]->type.c_str(), "fap") && resp[i]->nfap > 1) {
		fmax = resp[i]->fap_f[resp[i]->nfap-1];
		break;
	    }
	}
	if(fmax < 0.) fmax = 40.;
    }
    df = fmax/(nf-1);

    if(calper < 0.) calper = 0.;
    if(calib == 0.) calib = 1.;

    Response::compute(&resp, 0., fmax, calib, calper, nf, real, imag);

    remove_time_shift = remove_time_shift_toggle->state();

    if(remove_time_shift) {
	Response::removeTimeShift(nf, real, imag);
    }

    memcpy(re, real, nf*sizeof(double));
    memcpy(im, imag, nf*sizeof(double));

//    Response::taperAmp(real, imag, df, nf, flo, fhi);

    for(i = 0; i < nf; i++)
    {
	amp[i] = sqrt(real[i]*real[i] + imag[i]*imag[i]);
	if(imag[i] == 0. && real[i] == 0.) {
	    phase[i] = 0.;
	}
	else {
	    phase[i] = atan2(imag[i], real[i]);
	}
	freq[i] = i*df;
    }

    if(amp_cutoff > 0.) {
	Response::ampCutoff(re, im, nf, amp_cutoff);
    }

    for(i = 0; i < nf; i++) {
	double den = re[i]*re[i] + im[i]*im[i];
	if(den != 0.) {
	    re[i] =  re[i]/den;
	    im[i] = -im[i]/den;
	}
    }
    Response::taperAmp(re, im, df, nf, flo, fhi);
    
    for(i = 0; i < nf; i++) {
	inverse[i] = sqrt(re[i]*re[i] + im[i]*im[i]);
    }

    if(power_toggle->state()) {
	for(i = 0; i < nf; i++) {
	    amp[i] *= amp[i];
	    inverse[i] *= inverse[i];
	}
    }
    if(freq[0] == 0.) {
	if(log_x_toggle->state() || log_xy_toggle->state() ||
	    !strcasecmp(resp[0]->response_units.c_str(), "M/S") ||
	    !strncasecmp(resp[0]->response_units.c_str(), "v", 1) ||
	    !strcasecmp(resp[0]->response_units.c_str(), "M/S**2") ||
	    !strncasecmp(resp[0]->response_units.c_str(), "a", 1) )
	{
	    // don't draw freq=0. point
	    nf--;
	    for(i = 0; i < nf; i++) {
		freq[i] = freq[i+1];
		amp[i] = amp[i+1];
		phase[i] = phase[i+1];
		inverse[i] = inverse[i+1];
	    }
	}
    }

    int n = 0;
    if(log_x_toggle->state() || log_xy_toggle->state())
    {
	for(i = 0; i < nf; i++) {
	    freq[i] = log10(freq[i]);
	}
	XtSetArg(args[n], XtNlogX, True); n++;
    }
    else {
	XtSetArg(args[n], XtNlogX, False); n++;
    }
    phase_plot->setValues(args, n);
    if(log_y_toggle->state() || log_xy_toggle->state())
    {
	LogData(nf, amp);
	LogData(nf, inverse);
	if(amp_toggle->state()) {
	    XtSetArg(args[n], XtNyLabel, "Log Amplitude"); n++;
	}
	else if(power_toggle->state()) {
	    XtSetArg(args[n], XtNyLabel, "Log Power"); n++;
	}
    }
    else {
	if(amp_toggle->state()) {
	    XtSetArg(args[n], XtNyLabel, "Amplitude"); n++;
	}
	else if(power_toggle->state()) {
	    XtSetArg(args[n], XtNyLabel, "Power"); n++;
	}
    }
    XtSetArg(args[n], XtNdisplayGrid, grid_toggle->state()); n++;
    amp_plot->setValues(args, n);
    inverse_plot->setValues(args, n);

    lab[0] = '\0';
/*
    if(selected >= 0) {
	snprintf(lab, sizeof(lab), "%d", selected);
    }
*/
    if(amp_toggle->state() || power_toggle->state())
    {
	amp_plot->addCurve(nf, freq, amp, CPLOT_CURVE, lab, lab, true,
			adjust_limits, fg);
    }

    for(i = 0; i < nf; i++) p[i] = (double)phase[i];
    Response::unwrap(p, nf);
    for(i = 0; i < nf; i++) phase[i] = (float)p[i]*rad_to_degree;
    phase_plot->addCurve(nf, freq, phase, CPLOT_CURVE, lab, lab, true,
			CPLOT_ADJUST_LIMITS, fg);
    phase_plot->getYLimits(&ymin, &ymax);
    if(-0.1 < ymin && ymin < 1.0 && -0.1 < ymin && ymin < 1.0) {
	phase_plot-> setYLimits(-0.1, 0.1);
    }

    inverse_plot->addCurve(nf, freq, inverse, CPLOT_CURVE, lab, lab, true,
			adjust_limits, fg);

    Free(real);
    Free(imag);
    Free(freq);
    Free(amp);
    Free(inverse);
    Free(phase);
    Free(p);
    Free(re);
    Free(im);


    int npts;
    double *t=NULL, tlen;
    float *data=NULL;
    double dt = (samprate > 0.) ? 1./samprate : 1./40.;

/**/
    flo = 0.; // turn off frequency domain tapering
    fhi = 0.;
/**/

    samprate = 1./dt;
    if( samprate > 10000.) {
	tlen = .1;
    }
    else if( samprate > 1000.) {
	tlen = 1.;
    }
    else if( samprate > 99. ) {
	tlen = 5.;
    }
    else if( samprate > 19.) {
	tlen = 10.;
    }
    else if( samprate > 3. ) {
	tlen = 20.;
    }
    else {
	tlen = 50.;
    }
    // plot -tlen to tlen seconds
    n = (int)(tlen/dt + .5);
    npts = 2*n + 1;
    
    if(!(t = (double *)mallocWarn(npts*sizeof(double)))) return;
    if(!(data = (float *)mallocWarn(npts*sizeof(float)))) return;

    for(i = 1; i <= n; i++) {
	t[n+i] =  i*dt;
	t[n-i] = -i*dt;
	data[n+i] = 0.;
	data[n-i] = 0.;
    }
    t[n] = 0.;
    data[n] = 1.;
    lab[0] = '\0';
    Response::convolve(&resp, 1, data, npts, dt, flo, fhi, amp_cutoff,
			calib, calper, remove_time_shift);
    time_plot->addCurve(npts, t, data, CPLOT_CURVE, lab, lab, true,
			CPLOT_ADJUST_LIMITS, fg);
    Free(t);
    Free(data);
}

void Resp::convolve(int direction)
{
    int num_resp;
    vector<int> selected;
    gvector<Waveform *> wvec;
    RespStruct *rs;
    rs = (!strcasecmp(table_tab->labelOnTop(), "Instruments")) ? &rps : &all;

    if( !(num_resp = rs->response_table->getSelectedRows(selected)) ) {
	showWarning("No response selected.");
	return;
    }
    if( data_source->getSelectedWaveforms(wvec) <= 0 ) {
	showWarning("No waveforms selected.");
	return;
    }
    setCursor("hourglass");

    conDecon(direction, num_resp, selected, wvec);

    setCursor("default");
}

static bool
hasOriginalResponse(GTimeSeries *ts)
{
    gvector<DataMethod *> *methods = ts->dataMethods();
    for(int i = 0; i < (int)methods->size(); i++) {
	if(methods->at(i)->getConvolveDataInstance()) {
	    delete methods;
	    return false;
	}
    }
    delete methods;
    return true;
}

void Resp::conDecon(int direction, int num_resp, vector<int> &selected,
		gvector<Waveform *> &wvec)
{
    int i, j, k, npts, taper_pts;
    double taper_secs, flo, fhi, calib, calper, cutoff, nyquist;
    gvector<DataMethod *> *methods;
    const char *instype;
    DataMethod *dm;
    Working *working = NULL;
    CssInstrumentClass *ins;
    RespStruct *rs;
    rs = (!strcasecmp(table_tab->labelOnTop(), "Instruments")) ? &rps : &all;
    bool remove_time_shift = remove_time_shift_toggle->state();

    if(num_resp <= 0) return;

    resp_tapers->getParams(&taper_secs, &flo, &fhi, &cutoff);

    vector<Response *> resp;
    for(i = 0; i < num_resp; i++) {
	resp.push_back(rs->responses[selected[i]]);
    }

    k = rs->instrument_index[selected[0]];
    if( (ins = all.instrument_table->getRecord(k-1)) ) {
	instype = ins->instype;
    }
    else {
	instype = resp[0]->insname.c_str();
    }

    npts = 0;
    for(i = 0; i < wvec.size(); i++) {
        npts += wvec[i]->length();
    }
    int working_threshold = getProperty("working_dialog_threshold", 100000);
    if(npts > working_threshold) {
	char title[100];
	snprintf(title, sizeof(title), "Processing %d waveforms",wvec.size());
	working = new Working("Working", this, title, "waveforms processed");
	working->setVisible(true);
    }

    for(i = 0; i < wvec.size(); i++)
    {
	methods = wvec[i]->dataMethods();

	for(j = (int)methods->size()-1; j >= 0; j--) {
	    if( methods->at(j)->getCalibDataInstance() ) {
		methods->removeAt(j);
	    }
	}
	// demean if not already done
	if((int)methods->size() == 0 || !methods->back()->getDemeanInstance())
	{
	    Demean *demean = new Demean();
	    methods->add(demean);
	}

	if(taper_secs > 0.) {
	    taper_pts = (int)(taper_secs/wvec[i]->segment(0)->tdel()+.5);
	    dm = new TaperData("cosine", 0, taper_pts, taper_pts);
	}
	else {
	    dm = new TaperData("none", 0, 0, 0);
	}
	methods->add(dm);

	if(direction == -1 && hasOriginalResponse(wvec[i]->ts)) {
	    calib = wvec[i]->segment(0)->calib();
	    calper = wvec[i]->segment(0)->calper();
	}
	else if(ins) {
	    calib = ins->ncalib;
	    calper = ins->ncalper;
	}
	else {
	    calib = 0.;
	    calper = 0.;
	}
	if(fhi == 0.) {
	    nyquist = .5/wvec[i]->segment(0)->tdel();
	    fhi = nyquist - 1.;
	}
	dm = new ConvolveData(direction, resp, instype, flo, fhi, cutoff,
				calib, calper, remove_time_shift);
	methods->add(dm);

	wvec[i]->setDataMethods(methods);
	delete methods;

	DataMethod::update(wvec[i]);

	if(working && !working->update(i+1)) break;
    }
    ignore_data_change = true;
    data_source->modifyWaveforms(wvec);
    ignore_data_change = false;

    if(working) {
	working->destroy();
    }
}

void Resp::removeMethods(gvector<Waveform *> &wvec)
{
    int i, j;

    /* remove all Convolve methods
     */

    for(i = 0; i < wvec.size(); i++)
    {
	gvector<DataMethod *> *methods = wvec[i]->dataMethods();

	for(j = (int)methods->size()-1; j >= 0; j--) {
	    DataMethod *dm = methods->at(j);
	    if(dm->getConvolveDataInstance()) {
		methods->removeAt(j);
		if(j > 0) {
		    dm = methods->at(j-1);
		    if(dm->getTaperDataInstance()) {
			methods->removeAt(j-1);
			j--;
		    }
		}
	    }
	    else if(dm->getDemeanInstance()) {
		methods->removeAt(j);
	    }
	}
	wvec[i]->setDataMethods(methods);

	// put calib method back in
	for(j = 0; j < (int)methods->size() &&
		!methods->at(j)->getCalibDataInstance(); j++);
	if(j == (int)methods->size()) {
	    CalibData *cal = new CalibData();
	    gvector<Waveform *> v(wvec[i]);
	    cal->apply(v);
	}
	delete methods;
	DataMethod::update(wvec[i]);
    }
    ignore_data_change = true;
    data_source->modifyWaveforms(wvec);
    ignore_data_change = false;
}

/* Get the name of a response file. If it is a dfile of a current instrument,
 * simply select that instrument in the table. If not, make an instrument
 * object for the dfile, add the object to the instrument table and select it.
 */
void Resp::open(void)
{
    char *file;
    FileDialog *file_dialog = new FileDialog("Response Open", this,
				EXISTING_FILE, "./", "*");
    file_dialog->setVisible(true);

    if( (file = file_dialog->getFile()) ) {
	CssInstrumentClass *o=NULL;
	int i;
	long q, file_q = stringToQuark(file);
	cvector<CssInstrumentClass> v;
	all.instrument_table->getRecords(v);

	for(i = 0; i < v.size(); i++) {
	    // get the absolute path to the dfile
	    if(v[i]->getValue("dfile", &q) && file_q == q) break;
	}

	if(i < v.size()) {
	    table_tab->setOnTop("All Instruments");
	    all.instrument_table->deselectAllRows();
	    all.instrument_table->selectRecord(o, true, true);
	    all.instrument_table->moveToTop(i);
//	    listResponses(&all);
	    setButtonsSensitive(&all);
	}
	else {
	    // make a new instrument record for this dfile
	    o = ResponseFile::addDFile(file);
	    listAllInstruments();
	    table_tab->setOnTop("All Instruments");
	    all.instrument_table->deselectAllRows();
	    all.instrument_table->selectRecord(o, true, true);
	    all.instrument_table->getRecords(v);
	    for(i = 0; i < v.size() && v[i] != o; i++);
	    if(i < v.size()) {
		all.instrument_table->moveToTop(i);
	    }
	    
//	    listResponses(&all);
	    setButtonsSensitive(&all);
	}
	Free(file);
    }
    file_dialog->destroy();
}

void Resp::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
        if(data_source) data_source->removeDataReceiver(this);
        data_source = ds;
        if(data_source) {
	    data_source->addDataReceiver(this);
	    data_source->addDataListener(this);
	}
    }
    listWaveformInstruments();
}
