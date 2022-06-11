/** \file QCParam.cpp
 *  \brief Defines class QCParam.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "QCParam.h"
#include "QCData.h"
#include "widget/Table.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}

using namespace libgdataqc;

QCParam::QCParam(const char *name, Component *parent, DataSource *ds)
		: Frame(name, parent), DataReceiver(ds)
{
    if(data_source) {
	data_source->addDataReceiver(this);
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;

    createInterface();

    if(wp) {
	wp->addActionListener(this, XtNdataQCCallback);
	wp->addActionListener(this, XtNdataChangeCallback);
    }
    addActionListener(this, XtNsetVisibleCallback);
}

void QCParam::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    save_button = new Button("Save", file_menu, this);
    save_button->setSensitive(false);
    close_button = new Button("Close", file_menu, this);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    apply_button = new Button("Apply QC to Selected Waveforms", option_menu,
			this);
    qc_input_toggle = new Toggle("QC Input", option_menu);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("QC Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableBackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XmNwidth, 500); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 2); n++;
    XtSetArg(args[n], XtNtableTitle, "QC Parameters"); n++;
    XtSetArg(args[n], XtNcolumns, 13); n++;
    const char *column_labels[] = {
	"type", "fix", "ntaper", "drop_thr", "single_trace_spike_thr", "niter",
	"nsamp", "nover", "spike_thr", "spike_stat ", "spike_val ",
	"spike_npwin ", "spike_dset"
    };
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    const char *column_choice[] = {
	"basic:extended", "0:1", "", "", "", "", "", "", "",
	"avg:per", "", "", "data:1diff:all"
    };
    XtSetArg(args[n], XtNcolumnChoice, column_choice); n++;
    table = new Table("table", frame_form, args, n);
    table->setVisible(true);
    table->addActionListener(this, XtNvalueChangedCallback);
    table->addActionListener(this, XtNchoiceChangedCallback);

    initParams();

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, table->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 20); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableBackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XtNtableTitle, "Spike and Gap Detection"); n++;
    XtSetArg(args[n], XtNcolumns, 9); n++;
    const char *col_labels[] = {
	"sta", "chan", "seg", "index", "start time", "start", "end", "length",
	"state"
    };
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    gap_table = new Table("gap_table", frame_form, info_area, args, n);
    gap_table->setVisible(true);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(save_button, "Save Parameters");
	tool_bar->add(apply_button, "Apply to Selected Waveforms");
    }
    addPlugins("QCParam", wp, NULL);
}

void QCParam::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
	if(wp) {
	    wp->removeActionListener(this, XtNdataQCCallback);
	    wp->removeActionListener(this, XtNdataChangeCallback);
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
            wp = data_source->getWaveformPlotInstance();
	    if(wp) {
		wp->addActionListener(this, XtNdataQCCallback);
		wp->addActionListener(this, XtNdataChangeCallback);
	    }
	}
	else wp = NULL;
    }
}


void QCParam::list(void)
{
    const char *row[13];
    char s[13][20];
    for(int i = 0; i < 13; i++) row[i] = &s[i][0];
    row[0] = extended ? "extended" : "basic";
    row[1] = !qcdef.fix ? (char *)"0" : (char *)"1";
    snprintf((char *)row[2], 20, "%d", qcdef.ntaper);
    snprintf((char *)row[3], 20, "%d", qcdef.drop_thr);
    snprintf((char *)row[4], 20, "%.1f", qcdef.single_trace_spike_thr);
    snprintf((char *)row[5], 20, "%d", qcdef.niter);
    snprintf((char *)row[6], 20, "%d", qcdef.nsamp);
    snprintf((char *)row[7], 20, "%d", qcdef.nover);
    snprintf((char *)row[8], 20, "%.1f", qcdef.spike_thr);
    row[9] = !qcdef.spike_stat ? (char *)"avg" : (char *)"per";
    snprintf((char *)row[10], 20, "%.1f", qcdef.spike_val);
    snprintf((char *)row[11], 20, "%d", qcdef.spike_npwin);
    if(qcdef.spike_dset == 0) {
	row[12] = "data";
    }
    else if(qcdef.spike_dset == 1) {
	row[12] = "1diff";
    }
    else {
	row[12] = "all";
    }
    table->removeAllRows();
    table->addRow(row, true);
}

QCParam::~QCParam(void)
{
}

void QCParam::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(action_event->getReason(), XtNsetVisibleCallback)) {
	list();
	close_button->setLabel("Close");
	save_button->setSensitive(false);
	apply_button->setSensitive(wp->numSelected() ? true : false);
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	apply_button->setSensitive(wp->numSelected() ? true : false);
    }
    else if(!strcmp(action_event->getReason(), XtNmodifyVerifyCallback)) {
	printf("XtNmodifyVerifyCallback\n");
    }
    else if(!strcmp(action_event->getReason(), XtNchoiceChangedCallback)) {
	close_button->setLabel("Cancel");
	save_button->setSensitive(true);
    }
    else if(!strcmp(action_event->getReason(), XtNvalueChangedCallback)) {
	close_button->setLabel("Cancel");
	save_button->setSensitive(true);
    }
    else if(!strcmp(cmd, "Save")) {
	if(getParam()) {
	    close_button->setLabel("Close");
	    save_button->setSensitive(false);
	    save();
	}
    }
    else if(!strncmp(cmd, "Apply QC", 8)) {
	apply();
    }
    else if(!strcmp(cmd, "QC Help")) {
	showHelp(cmd);
    }
    else if(!strcmp(action_event->getReason(), XtNdataQCCallback)) {
	if(qc_input_toggle->state()) {
	    apply((DataQCStruct *)action_event->getCalldata());
	}
    }
}

ParseCmd QCParam::parseCmd(const string &cmd, string &msg)
{
    string c;
    int id = -1;
    Waveform *cd;

    if(parseCompare(cmd, "apply")) {
	apply();
    }
    else if(parseCompare(cmd, "apply ", 6) && parseGetArg(cmd, "_wave_", &id))
    {
	if(!(cd = wp->getWaveform(id))) {
	    msg.assign("Waveform not found.");
	    return ARGUMENT_ERROR;
	}
	QCData *qcdata = new QCData(extended, &qcdef);
	gvector<Waveform *> v(cd);
        qcdata->apply(v);
        wp->modifyWaveforms(v);
    }
    else if(parseString(cmd, "table", c)) {
	return table->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void QCParam::parseHelp(const char *prefix)
{
    Table::parseHelp(prefix);
}

void QCParam::apply(void)
{
    if(!getParam()) return;

    gvector<Waveform *> wvec;
    wp->getSelectedWaveforms(wvec);

    if(wvec.size() > 0) {
	for(int i = 0; i < wvec.size(); i++) {
	    wvec[i]->ts->removeMethod("QCData");
	}
	QCData *qcdata = new QCData(extended, &qcdef);
	gap_table->removeAllRows();
	for(int i = 0; i < wvec.size(); i++) {
	    qcdata->applyqc(wvec[i]->ts, gap_table);
	    wvec[i]->ts->addDataMethod(qcdata);
	}
	wp->modifyWaveforms(wvec);
    }
}

void QCParam::apply(DataQCStruct *dq)
{
    QCData *qc = new QCData(extended, &qcdef);
    dq->ts = qc->apply(dq->ts);
}

bool QCParam::getParam(void)
{
    vector<const char *> row;
    QCDef def;

    table->getRow(0, row);
    if((int)row.size() == 0) {
	showWarning("QCParam: cannot get qc parameters.");
	return false;
    }
    extended = (!strcmp(row[0], "extended")) ? true : false;

    if(!stringToInt(row[1], &def.fix)) {
	showWarning("Invalid QC fix.");
	return false;
    }
    if(!stringToInt(row[2], &def.ntaper)) {
	showWarning("Invalid QC ntaper.");
	return false;
    }
    if(!stringToInt(row[3], &def.drop_thr)) {
	showWarning("Invalid QC drop_thr.");
	return false;
    }
    if(!stringToDouble(row[4], &def.single_trace_spike_thr)) {
	showWarning("Invalid single_trace_spike_thr.");
	return false;
    }
    if(!stringToInt(row[5], &def.niter) || def.niter <= 0) {
	showWarning("Invalid QC niter.");
	return false;
    }
    if(!stringToInt(row[6], &def.nsamp) || def.nsamp <= 0) {
	showWarning("Invalid QC nsamp.");
	return false;
    }
    if(!stringToInt(row[7], &def.nover)) {
	showWarning("Invalid QC nover.");
	return false;
    }
    if(!stringToDouble(row[8], &def.spike_thr)) {
	showWarning("Invalid QC spike_thr.");
	return false;
    }
    if(!strcmp(row[9], "avg")) {
	def.spike_stat = QC_SPIKE_STAT_AVG;
    }
    else if(!strcmp(row[9], "per")) {
	def.spike_stat = QC_SPIKE_STAT_PER;
    }
    else {
	showWarning("Invalid QC spike_stat.");
	return false;
    }
    if(!stringToDouble(row[10], &def.spike_val)) {
	showWarning("Invalid QC spike_val.");
	return false;
    }
    if(!stringToInt(row[11], &def.spike_npwin)) {
	showWarning("Invalid QC spike_npwin.");
	return false;
    }
    if(!strcmp(row[12], "data")) {
	def.spike_dset = QC_SPIKE_DSET_DATA;
    }
    else if(!strcmp(row[12], "1diff")) {
	def.spike_dset = QC_SPIKE_DSET_1DIFF;
    }
    else if(!strcmp(row[12], "all")) {
	def.spike_dset = QC_SPIKE_DSET_ALL;
    }
    else {
	showWarning("Invalid QC spike_dset.");
	return false;
    }
    qcdef = def;
    return true;
}


void QCParam::initParams()
{
    string s, c;

    extended = true;
    qcdef.fix = 1;      // Fix the data
    // Number of points to taper outside masked segments of length >= drop_thr
    qcdef.ntaper = 10;
    // Number of consecutive equal-valued samples to call a bad segment
    qcdef.drop_thr = 8;
    // Amplitude value threshold for single point spikes
    qcdef.single_trace_spike_thr = 10.0;
    qcdef.niter = 1; // Number of iterations to perform extended qc
    qcdef.nsamp = 10; // Number of samples in a time interval for extended qc
    qcdef.nover = 8;  // Number of overlap samples for extended qc
    qcdef.spike_thr = 10.0; // Amplitude value threshold for extended qc spikes
    // Statistic to use for measuring spikes across multiple data vectors
    qcdef.spike_stat = QC_SPIKE_STAT_AVG;
    qcdef.spike_val = 1.0;  // Value to use for spike_stat
    // Number of points to focus in on for single vector extended qc spike
    //detection
    qcdef.spike_npwin = 10;
    // Data set to use for extended qc across multiple data vectors
    qcdef.spike_dset = QC_SPIKE_DSET_DATA;

    if(!getProperty("qcparams", s)) return;

    if(parseGetArg(s, "type", c)) {
	if(parseCompare(c, "basic")) {
	    extended = false;
	}
	else if(parseCompare(c, "extended")) {
	    extended = true;
	}
	else {
	    logErrorMsg(LOG_WARNING, "Invalid qcparams type.");
	}
    }
    parseGetArg(s, "fix", &qcdef.fix);
    parseGetArg(s, "ntaper", &qcdef.ntaper);
    parseGetArg(s, "drop_thr", &qcdef.drop_thr);
    parseGetArg(s, "single_trace_spike_thr", &qcdef.single_trace_spike_thr);
    parseGetArg(s, "niter", &qcdef.niter);
    parseGetArg(s, "nsamp", &qcdef.nsamp);
    parseGetArg(s, "nover", &qcdef.nover);
    parseGetArg(s, "spike_thr", &qcdef.spike_thr);
    if(parseGetArg(s, "spike_stat", c)) {
	if(parseCompare(c, "avg")) {
	    qcdef.spike_stat = QC_SPIKE_STAT_AVG;
	}
	else if(parseCompare(c, "per")) {
	    qcdef.spike_stat = QC_SPIKE_STAT_PER;
	}
	else {
	    logErrorMsg(LOG_WARNING, "Invalid QCData spike_stat.");
	}
    }
    parseGetArg(s, "spike_val", &qcdef.spike_val);
    parseGetArg(s, "spike_npwin", &qcdef.spike_npwin);
    if(parseGetArg(s, "spike_dset", c)) {
	if(parseCompare(c, "data")) {
	    qcdef.spike_dset = QC_SPIKE_DSET_DATA;
	}
	else if(parseCompare(c, "1diff")) {
	    qcdef.spike_dset = QC_SPIKE_DSET_1DIFF;
	}
	else if(parseCompare(c, "all")) {
	    qcdef.spike_dset = QC_SPIKE_DSET_ALL;
	}
    }
}

void QCParam::save(void)
{
    ostringstream os;

    os.precision(2);
    os << "type=";
    if(extended) os << "extended";
    else os << "basic";

    os << " fix=" << qcdef.fix;
    os << " ntaper=" << qcdef.ntaper;
    os << " drop_thr=" << qcdef.drop_thr;
    os << " single_trace_spike_thr=" << qcdef.single_trace_spike_thr;
    os << " niter=" << qcdef.niter;
    os << " nsamp=" << qcdef.nsamp;
    os << " nover=" << qcdef.nover;
    os << " spike_thr=" << qcdef.spike_thr;
    os << " spike_stat=";
    if(qcdef.spike_stat == QC_SPIKE_STAT_AVG) os << "avg";
    else os << "per";
    os << "spike_val=" << qcdef.spike_val;
    os << "spike_npwin=" << qcdef.spike_npwin;
    os << "spike_dset=";
    if(qcdef.spike_dset == QC_SPIKE_DSET_DATA) os << "data";
    else if(qcdef.spike_dset == QC_SPIKE_DSET_1DIFF) os << "1diff";
    else os << "all";

    putProperty("qcparams", os.str());
}
