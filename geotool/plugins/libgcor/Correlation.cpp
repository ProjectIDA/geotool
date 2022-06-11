/** \file Correlation.cpp
 *  \brief Defines class Correlation.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "Correlation.h"
#include "MinCorrOverlap.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/GTimeSeries.h"
#include "TableMenu.h"

extern "C" {
#include "libstring.h"
}

using namespace libgcor;

#define MAX_COLUMNS 55


Correlation::Correlation(const char *name, Component *parent, DataSource *ds)
			: Frame(name, parent, true), DataReceiver(ds)
{
    createInterface();
}

void Correlation::createInterface()
{
    int n;
    Arg args[30];

    setSize(500, 600);
    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    print_button = new Button("Print...", file_menu, this);
    output_table_button = new Button("Output Table...", file_menu, this);
    output_traces_button = new Button("Output Traces...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);

    table_menu = new TableMenu("View", menu_bar);

    option_menu = new Menu("Option", menu_bar);
    align_button = new Button("Align Waveforms", option_menu, this);
    copy_button = new Button("Copy to Main Window", option_menu, this);
    correlate_button = new Button("Correlate", option_menu, this);
    max_corr_menu = new Menu("Maximum Coefficent", option_menu, true);
    pos_max_toggle = new Toggle("Maximum Positive Value", max_corr_menu, this);
    pos_max_toggle->set(true);
    abs_max_toggle = new Toggle("Maximum Absolute Value", max_corr_menu, this);
    norm_type_menu = new Menu("Normalization Type", option_menu, true);
    local_mean_toggle = new Toggle("Local Mean", norm_type_menu, this);
    local_mean_toggle->set(false);
    global_mean_toggle = new Toggle("Global Mean", norm_type_menu, this);
    global_mean_toggle->set(false);
    total_amp_toggle = new Toggle("Total Amp", norm_type_menu, this);
    total_amp_toggle->set(true);
    set_ref_button = new Button("Set Reference", option_menu, this);
    min_overlap_button = new Button("Minimum Overlap...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Correlation Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNvisibleRows, 5); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNcolumns, 5); n++;
    const char *col_labels[] = {"sta", "chan", "max coef", "time", "time lag"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    table = new Table("table", pane, args, n);
    table->setAttributes("sta,%s,chan,%s,max coef,%.2f,time,%t,time lag,%.3f");
    int alignment[5] = {LEFT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY,
				LEFT_JUSTIFY, RIGHT_JUSTIFY};
    table->setAlignment(5, alignment);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);
    if(abs_max_toggle->state()) {
	table->setColumnLabel(2, "max abs coef");
    }
    else {
	table->setColumnLabel(2, "max pos coef");
    }

    table_menu->setTable(table);

    form = new Form("form", pane);

    n = 0;
    XtSetArg(args[n], XtNtitle, "Reference Trace"); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    XtSetArg(args[n], XtNheight, 150); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    plot1 = new WaveformView("plot1", form, info_area, args, n);

    n = 0;
    XtSetArg(args[n], XtNtitle, "Correlation Traces"); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_SECONDS); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, plot1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    plot2 = new WaveformView("plot2", form, info_area, args, n);
    plot2->addActionListener(this, XtNselectDataCallback);

    setButtonsSensitive();

    print_window = NULL;
    file_dialog = NULL;
    cor = new gvector<CorClass *>;

    min_corr_overlap = new MinCorrOverlap("Minimum Correlation Overlap", this);
    min_corr_overlap->addActionListener(this, XmNactivateCallback);

    addPlugins("Correlation", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(set_ref_button, "Set Reference");
	tool_bar->add(correlate_button, "Correlate");
	tool_bar->add(copy_button, "Copy to main window");
    }
}

Correlation::~Correlation(void)
{
    cor->clear();
    delete cor;
}

void Correlation::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Clear")) {
	clear();
    }
    else if(!strcmp(cmd, "Minimum Overlap...")) {
	min_corr_overlap->setVisible(true);
    }
    else if(comp == min_corr_overlap) {
	min_corr_overlap->setVisible(false);
    }
    else if(!strcmp(cmd, "Output Table...")) {
	save();
    }
    else if(!strcmp(cmd, "Output Traces...")) {
	action_event->setCommandString("Output...");
	plot2->actionPerformed(action_event);
    }
    else if(!strcmp(cmd, "Delete")) {
	remove();
    }
    else if(!strcmp(cmd, "Set Reference")) {
	setReference();
    }
    else if(!strcmp(cmd, "Correlate")) {
	compute();
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Align Waveforms")) {
	align();
    }
    else if(!strcmp(cmd, "Copy to Main Window")) {
	copyWaveform();
    }
    else if(!strcmp(cmd, "Correlation Help")) {
	showHelp("Correlation Help");
    }
    else if(comp == pos_max_toggle && pos_max_toggle->state()) {
	list();
	selectWaveform();
	setButtonsSensitive();
    }
    else if(comp == abs_max_toggle && abs_max_toggle->state()) {
	list();
	selectWaveform();
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNattributeChangeCallback)) {
	list();
	selectWaveform();
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNselectRowCallback)) { // select row
	selectRow();
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNselectDataCallback)) { // select waveform
	selectWaveform();
	setButtonsSensitive();
    }
}

ParseCmd Correlation::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "Clear")) {
	clear_button->activate();
    }
    else if(parseCompare(cmd, "Correlate")) {
	correlate_button->activate();
    }
    else if(parseCompare(cmd, "Delete")) {
	delete_button->activate();
    }
    else if(parseCompare(cmd, "Copy_to_Main_Window")) {
	copy_button->activate();
    }
    else if(parseCompare(cmd, "Set_Reference")) {
	set_ref_button->activate();
    }
    else if(parseCompare(cmd, "Align_Waveforms")) {
	align_button->activate();
    }
    else if(parseString(cmd, "plot1", c)) {
	return plot1->parseCmd(c, msg);
    }
    else if(parseString(cmd, "plot2", c)) {
	return plot2->parseCmd(c, msg);
    }
    else if(parseString(cmd, "print_window", c)) {
	if(!print_window) {
	    print_window = new PrintDialog("Print Correlation", this, this);
	}
	return print_window->parseCmd(c, msg);
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

ParseVar Correlation::parseVar(const string &name, string &value)
{
    string c;

    if(parseString(name, "table", c)) {
	return table->parseVar(c, value);
    }
    else if(parseString(name, "plot1", c)) {
	return plot1->parseVar(c, value);
    }
    else if(parseString(name, "plot2", c)) {
	return plot2->parseVar(c, value);
    }
    return Frame::parseVar(name, value);
}

void Correlation::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scorrelate\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%scopy_to_main_window\n", prefix);
    printf("%sset_reference\n", prefix);
    printf("%salign_waveforms\n", prefix);
    printf("%sprint.help\n", prefix);
    Table::parseHelp(prefix);
}

void Correlation::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Correlation", this, this);
    }
    print_window->setVisible(true);
}

void Correlation::print(FILE *fp, PrintParam *p)
{
//    char title[200], tlab[100];
    int total_height;
    AxesParm *a, *a2;
    double f, bottom, top;
    Dimension height;
    Arg args[1];

    XtSetArg(args[0], XtNheight, &height);

    total_height = 0;
    plot1->getValues(args, 1);
    total_height += height;
    plot2->getValues(args, 1);
    total_height += height;
/*
    snprintf(title, 200,
        "%s  %s\nwindow-length %.1fsecs  window-overlap %.1fsecs",
        fk->p.net, tlab, fk->p.window_length, fk->p.window_overlap);
    p->top_title = title;
*/
    p->top_title = NULL;

    plot1->getValues(args, 1);
    f = (double)height/(double)total_height;
    bottom = p->bottom;
    top = p->top;
    p->bottom = top - f*(top - bottom);

    if( !(a = plot1->hardCopy(fp, p, NULL)) ) {
        return;
    }

    p->top_title = NULL;
    a->auto_x = False;

    p->top = p->bottom;
    plot2->getValues(args, 1);
    f += height/(double)total_height;
    p->bottom = bottom;
    p->x_axis_label = (char *)"time (seconds)";
    if((a2 = plot2->hardCopy(fp, p, a)) == NULL)
    {
	free(a);
	return;
    }
    free(a2);
    free(a);
}

void Correlation::setReference(void)
{
    GTimeSeries *ts;
    gvector<Waveform *> wvec;
    Waveform *w;

    if(data_source->getSelectedWaveforms("a", wvec) > 1)
    {
	showWarning("More than one waveform is selected.");
	return;
    }
    else if(wvec.size() == 1) {
	double tbeg  = wvec[0]->dw[0].d1->time();
	double tend  = wvec[0]->dw[0].d2->time();
	ts = wvec[0]->ts->subseries(tbeg, tend);
    }
    else if(data_source->getSelectedWaveforms(wvec) <= 0)
    {
	showWarning("No waveforms selected.");
	return;
    }
    else if(wvec.size() != 1) {
	showWarning("More than one waveform is selected.");
	return;
    }
    else if( !(ts = wvec[0]->ts->subseries(wvec[0]->begSelect,
					wvec[0]->endSelect)) )
    {
	ts = wvec[0]->ts;
    }
    w = wvec[0];

    plot1->clear();

    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    cvector<CssOriginClass> origins;
    data_source->getArrivalsOnWaveform(w, arrivals);
    data_source->getTable(assocs);
    data_source->getTable(origins);
    for(int i = 0; i < arrivals.size(); i++) {
	plot1->putArrivalWithColor(arrivals[i], stringToPixel("black"));
    }
    for(int i = 0; i < assocs.size(); i++) {
	plot1->putTable(assocs[i], false);
    }
    for(int i = 0; i < origins.size(); i++) {
	plot1->putTable(origins[i], false);
    }

    plot1->addWaveform(ts, stringToPixel("black"));
}

void Correlation::compute(void)
{
    double duration = 0.;
    int i;
    bool partial_ts;
    NormType norm_type;
    GTimeSeries *ts, *ref_ts;
    gvector<Waveform *> wvec;

    if(!plot1->getWaveforms(wvec, true)) {
	showWarning("No reference waveform.");
	return;
    }
    ref_ts = wvec[0]->ts;
    wvec.clear();

    if(local_mean_toggle->state()) {
	norm_type = LOCAL_MEAN;
    }
    else if(global_mean_toggle->state()) {
	norm_type = GLOBAL_MEAN;
    }
    else {
	norm_type = TOTAL_AMP;
    }

    if(data_source->getSelectedWaveforms("a", wvec) > 0)
    {
	setCursor("hourglass");
	Waveform::sortByY0(wvec);
	for(i = 0; i < wvec.size(); i++) {
	    double tbeg  = wvec[i]->dw[0].d1->time();
	    double tend  = wvec[i]->dw[0].d2->time();
	    ts = wvec[i]->ts->subseries(tbeg, tend);
	    correlate(ref_ts, wvec[i], ts, norm_type, &duration);
	    ts->deleteObject();
	}
	setCursor("default");
    }
    else if(data_source->getSelectedWaveforms(wvec) > 0)
    {
	setCursor("hourglass");
	Waveform::sortByY0(wvec);
	for(i = 0; i < wvec.size(); i++) {
	    ts = wvec[i]->ts->subseries(wvec[i]->begSelect,
					wvec[i]->endSelect);
	    if(ts) {
		partial_ts = true;
	    }
	    else {
		partial_ts = false;
		ts = wvec[i]->ts;
	    }
	    correlate(ref_ts, wvec[i], ts, norm_type, &duration);
	    if(partial_ts) ts->deleteObject();
	}
	setCursor("default");
    }
    else
    {
	showWarning("No waveforms selected.");
	return;
    }

    table->adjustColumns();
}

void Correlation::correlate(GTimeSeries *ref_ts, Waveform *w,
		GTimeSeries *ts, NormType norm_type, double *duration)
{
    double d;
    GTimeSeries *cor_ts;
    if(!(cor_ts = fftCorrelate(ref_ts, ts, 0, norm_type))) {
//    if(!(cor_ts = timeCorrelate(ref_ts, ts, 1, norm_type))){
	return;
    }

    if(min_corr_overlap->min_overlap > 0.) {
	double tmin = cor_ts->tbeg() + min_corr_overlap->min_overlap;
	double tmax = cor_ts->tend() - min_corr_overlap->min_overlap;
	GTimeSeries *cor_min = cor_ts->subseries(tmin, tmax);
	cor_ts->deleteObject();
	if(cor_min && cor_min->length() > 0) {
	    cor_min->putValue("source_id", w->getId());
	    correlateAdd(w, cor_min);
	    d = cor_min->duration();
	    if(*duration < d) *duration = d;
	}
    }
    else {
	cor_ts->putValue("source_id", w->getId());
	correlateAdd(w, cor_ts);
	d = cor_ts->duration();
	if(*duration < d) *duration = d;
    }
}

void Correlation::correlateAdd(Waveform *w, GTimeSeries *cor_ts)
{
    const char *row[50];
    GDataPoint *dmin, *dmax, *dp;
    double t0;
    cvector<CssArrivalClass> arrivals;

    cor_ts->setSta(w->sta());
    cor_ts->setChan(w->chan());
    cor_ts->setNet(w->net());
    plot2->addWaveform(cor_ts, w->fg);

    dmin = cor_ts->minPoint();
    dmax = cor_ts->maxPoint();

    CorClass *cc = new CorClass(w->sta(), w->chan(), dmin, dmax);
    cor->add(cc);

    if( data_source->getArrivalsOnWaveform(w, arrivals) > 0 ) {
	for(int i = 0; i < arrivals.size(); i++) {
	    if(arrivals[i]->time >= cor_ts->tbeg() &&
		arrivals[i]->time <= cor_ts->tend())
	    {
		cc->arrivals.push_back(arrivals[i]);
	    }
	}
    }

    if(cor->size() == 1) {
	setupTable(cc);
    }

    if(abs_max_toggle->state()) {
	if(fabs(dmin->value()) > fabs(dmax->value())) {
	    dp = dmin;
	    t0 = cor->at(0)->dmin->time();
	}
	else {
	    dp = dmax;
	    t0 = cor->at(0)->dmax->time();
	}
    }
    else {
	dp = dmax;
	t0 = cor->at(0)->dmax->time();
    }

    int num_columns = table->numColumns();
    CorClass *cc0 = cor->at(0);

    getRow(num_columns, t0, cc0, cc, row);

    cor_ts->putValue("max_cor", dp->value());
    cor_ts->putValue("cor_lag", dp->time() - t0);
    cor_ts->putValue("cor_time", dp->time());

    table->addRow(row, false);

    for(int i = 0; i < num_columns; i++) Free(row[i]);
}

void Correlation::list(void)
{
    table->removeAllRows();
    if(cor->size() == 0) return;

    CorClass *cc0, *cc;
    const char *row[MAX_COLUMNS];
    double t0;

/*
    if(abs_max_toggle->state()) {
	table->setColumnLabel(2, "max abs coef");
    }
    else {
	table->setColumnLabel(2, "max pos coef");
    }
*/

    cc0 = cor->at(0);
    setupTable(cc0);

    if(abs_max_toggle->state()) {
	if(fabs(cc0->dmin->value()) > fabs(cc0->dmax->value())) {
	    t0 = cc0->dmin->time();
	}
	else {
	    t0 = cc0->dmax->time();
	}
    }
    else {
	t0 = cc0->dmax->time();
    }

    int num_columns = table->numColumns();

    for(int j = 0; j < cor->size(); j++) {
	cc = cor->at(j);
	getRow(num_columns, t0, cc0, cc, row);

	table->addRow(row, false);

	for(int i = 0; i < num_columns; i++) Free(row[i]);
    }
    table->adjustColumns();
}

void Correlation::getRow(int num_columns, double t0, CorClass *cc0,
			CorClass *cc, const char **row)
{
    char maxCor[20], time[50], timeLag[50], arrlag[50];
    double d, lag, arr_time=0.;
    GDataPoint *dp;
    int i, k;

    if(abs_max_toggle->state()) {
	if(fabs(cc->dmin->value()) > fabs(cc->dmax->value())) {
	    dp = cc->dmin;
	}
	else {
	    dp = cc->dmax;
	}
    }
    else {
	dp = cc->dmax;
    }

    for(i = 0; i < num_columns; i++)
    {
	TAttribute a = table->getAttribute(i);
	if(!strcmp(a.name, "sta")) {
	    row[i] = strdup(cc->sta);
	}
	else if(!strcmp(a.name, "chan")) {
	    row[i] = strdup(cc->chan);
	}
	else if(!strcmp(a.name, "max abs coef") ||
		!strcmp(a.name, "max pos coef"))
	{
	    snprintf(maxCor, sizeof(maxCor), a.format, dp->value());
	    row[i] = strdup(maxCor);
	}
	else if(!strcmp(a.name, "time")) {
	    if(!strcmp(a.format, "%t")) {
		timeEpochToString(dp->time(), time, sizeof(time), YMONDHMS);
	    }
	    else {
		snprintf(time, sizeof(time), a.format, dp->time());
	    }
	    row[i] = strdup(time);
	}
	else if(!strcmp(a.name, "time lag")) {
	    d = 0.;
	    if(table->numRows() > 0) {
		d = dp->time() - t0;
	    }
	    snprintf(timeLag, sizeof(timeLag), a.format, d);
	    row[i] = strdup(timeLag);
	}
	else {
	    // compute arrival time lags:
	    // The lag associated with the P arrival on the i'th waveform is
	    // waveform_lag(i) + arrival_time(i, P) - arrival_time(0, P)
	    row[i] = NULL;
	    for(k = 0; k < cc0->arrivals.size(); k++) {
		if(!strcmp(a.name, cc0->arrivals[k]->phase)) {
		    arr_time = cc0->arrivals[k]->time;
		    break;
		}
	    }
	    if(k < cc0->arrivals.size()) {
		if(cc == cc0) {
		    snprintf(arrlag, sizeof(arrlag), a.format, 0.0);
		    row[i] = strdup(arrlag);
		}
		else {
		    for(k = 0; k < cc->arrivals.size(); k++) {
			if(!strcmp(a.name, cc->arrivals[k]->phase)) {
			    lag = cc->arrivals[k]->time + dp->time() - t0 - arr_time;
			    snprintf(arrlag, sizeof(arrlag), a.format, lag);
			    row[i] = strdup(arrlag);
			    break;
			}
		    }
		}
	    }
	    if(!row[i]) row[i] = strdup("-");
	}
    }
}

void Correlation::setButtonsSensitive(void)
{
    bool sensitive = (table->numRows() > 0) ? true : false;
    align_button->setSensitive(sensitive);
    clear_button->setSensitive(sensitive);

    sensitive = table->rowSelected();
    copy_button->setSensitive(sensitive);
    delete_button->setSensitive(sensitive);
}

void Correlation::copyWaveform(void)
{
    GTimeSeries **cor_ts = NULL;
    gvector<Waveform *> wvec;
    Pixel *pixels = NULL;
    WaveformPlot *wp;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if( plot2->getSelectedWaveforms(wvec) <= 0 ) {
	showWarning("No correlations selected.");
	return;
    }

    cor_ts = (GTimeSeries **)mallocWarn(wvec.size()*sizeof(GTimeSeries *));
    pixels = (Pixel *)mallocWarn(wvec.size()*sizeof(Pixel));
    for(int i =  0; i < wvec.size(); i++) {
	cor_ts[i] = new GTimeSeries(wvec[i]->ts);
	pixels[i] = wvec[i]->fg;
    }
    wp->addWaveforms(wvec.size(), cor_ts, NULL, pixels);

    Free(pixels);
    Free(cor_ts);
}

void Correlation::clear(void)
{
    plot2->clear();
    table->removeAllRows();
    cor->clear();
    setButtonsSensitive();
}

void Correlation::remove(void)
{
    int i, j, n, num_waveforms, num_selected;
    vector<int> rows;
    gvector<Waveform *> wvec, ws;

    num_waveforms = plot2->getWaveforms(ws, false);

    if(!(num_selected = plot2->getSelectedWaveforms(wvec))) {
	showWarning("No correlations selected.");
	return;
    }

    plot2->deleteWaveforms(wvec);

    n = 0;
    for(i = 0; i < num_waveforms; i++) {
	for(j = 0; j < num_selected && ws[i] != wvec[j]; j++);
	if(j < num_selected) {
	    rows.push_back(i);
	    n++;
	}
    }
    table->removeRows(rows);

    for(i = num_selected-1; i >= 0; i--) {
	cor->removeAt(rows[i]);
    }

    wvec.clear();
    if((num_waveforms = plot2->getWaveforms(wvec, false))) {
	vector<double> y;
	for(i = 0; i < num_waveforms; i++) {
	    y.push_back(i+1);
	}
	plot2->positionY(wvec, y);
    }
}

void Correlation::selectWaveform(void)
{
    vector<bool> states;
    int i, j, num_waveforms, num_selected;
    gvector<Waveform *> wvec, ws;

    if((num_waveforms = plot2->getWaveforms(ws, false)) <= 0) return;

    num_selected = plot2->getSelectedWaveforms(wvec);

    for(i = 0; i < num_waveforms; i++) {
	for(j = 0; j < num_selected && ws[i] != wvec[j]; j++);
	states.push_back((j < num_selected) ? true : false);
    }

    table->setRowStates(states);
}

void Correlation::selectRow(void)
{
    int i, j, num_waveforms, num_selected;
    vector<int> rows;
    gvector<Waveform *> wvec;
    bool select;

    num_selected = table->getSelectedRows(rows);

    num_waveforms = plot2->getWaveforms(wvec, false);

    for(i = 0; i < num_waveforms; i++) {
	for(j = 0; j < num_selected && rows[j] != i; j++);
	select = (j < num_selected) ? true : false;
	plot2->selectWaveform(wvec[i], select, false);
    }
}

void Correlation::align(void)
{
    double cor_time=0., screen_cor_time=0.;
    int id = -1, i, j, num_cor, num_waveforms, n;
    gvector<Waveform *> wvec, ws, c;
    vector<double> scaled_x0;
    WaveformPlot *wp;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if(!(num_cor = plot2->getWaveforms(ws, false))) return;

    if(!(num_waveforms = data_source->getWaveforms(wvec, false))) {
	return;
    }

    n = 0;
    for(i = 0; i < num_cor; i++)
    {
	GTimeSeries *cor_ts = ws[i]->ts;
	if( !cor_ts->getValue("source_id", &id) ||
		!cor_ts->getValue("cor_time", &cor_time)) return;

	for(j = 0; j < num_waveforms && n < num_cor; j++) {
	    if(wvec[j]->getId() == id) {
		if(n == 0) {
		    screen_cor_time = wvec[j]->scaled_x0 + cor_time
					- wvec[j]->tbeg();
		}
		else {
		    scaled_x0.push_back(screen_cor_time -
					(cor_time - wvec[j]->tbeg()));
		    c.push_back(wvec[j]);
		}
		n++;
	    }
	}
    }
    if(n > 1) {
	wp->positionX(c, scaled_x0);
    }
}

void Correlation::setupTable(CorClass *cc)
{
    char att[1000] = "";
    int alignment[MAX_COLUMNS];
    Arg args[20];
    TAttribute a;
    int i, n, num_columns;

    alignment[0] = LEFT_JUSTIFY;
    alignment[1] = LEFT_JUSTIFY;
    alignment[2] = RIGHT_JUSTIFY;
    alignment[3] = LEFT_JUSTIFY;
    alignment[4] = RIGHT_JUSTIFY;

    for(i = 0; i < 2; i++) {
	a = table->getAttribute(i);
	n = (int)strlen(att);
	snprintf(att+n, sizeof(att)-n, "%s,%s,", a.name, a.format);
    }

    a = table->getAttribute(2);
    if(abs_max_toggle->state()) {
	n = (int)strlen(att);
	snprintf(att+n, sizeof(att)-n, "max abs coef,%s", a.format);
    }
    else {
	n = (int)strlen(att);
	snprintf(att+n, sizeof(att)-n, "max pos coef,%s", a.format);
    }
    for(i = 3; i < 5; i++) {
	a = table->getAttribute(i);
	n = (int)strlen(att);
	snprintf(att+n, sizeof(att)-n, ",%s,%s", a.name, a.format);
    }
    num_columns = 5;

    for(i = 0; i < cc->arrivals.size() && i < MAX_COLUMNS-5; i++) {
	alignment[num_columns] = RIGHT_JUSTIFY;
	n = (int)strlen(att);
	snprintf(att+n, sizeof(att)-n, ",%s,%%.3f", cc->arrivals[i]->phase);
    }

    n = 0;
    XtSetArg(args[n], XtNcolumns, num_columns); n++;
    table->setValues(args, n);
    table->setAttributes(att);
    table->setAlignment(num_columns, alignment);
}

void Correlation::save(void)
{
    char *file;

    if(file_dialog == NULL) {
	file_dialog = new FileDialog("Save Correlation Table", this, FILE_ONLY,					".", (char *)"*", "Save");
    }
    file_dialog->setVisible(true);

    if((file = file_dialog->getFile()) != NULL) {
	table->save(file);
	XtFree(file);
    }
    file_dialog->setVisible(false);
}
