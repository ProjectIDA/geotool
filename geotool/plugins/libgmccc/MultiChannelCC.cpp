/** \file MultiChannelCC.cpp
 *  \brief Defines class Multi Channel Cross Correlation.
 *  \author Vera Miljanovic
 */

#include "config.h"
#include <iostream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_vector.h>

using namespace std;

#include "MultiChannelCC.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "tapers.h"
}

using namespace libgmccc;

MultiChannelCC::MultiChannelCC(const char *name, Component *parent,
			DataSource *ds) : Frame(name, parent), DataReceiver(ds)
{
    createInterface();
}

void MultiChannelCC::createInterface()
{
    int n;
    Arg args[30];

    setSize(500, 600);

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    print_button = new Button("Print...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);

    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    align_button = new Button("Align Waveforms", option_menu, this);
    copy_button = new Button("Copy to Main Window", option_menu, this);
    vandecar_crosson_button =
      new Button("Vandecar Crosson", option_menu, this);
    shearer_button = new Button("Shearer", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Multi Channel Cross Correlation Help",
			     help_menu, this);

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
    XtSetArg(args[n], XtNcolumns, 6); n++;
    const char *col_labels[] = {"sta", "chan", "max coef", "time", "abs corr lag", "rel corr lag"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    table = new Table("table", pane, args, n);
    table->setAttributes("sta,%s,chan,%s,max coef,%.8f,time,%t,abs corr lag,%.1f,rel corr lag,%.1f");
    int alignment[7] = {LEFT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY,
				LEFT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY};
    table->setAlignment(6, alignment);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    form = new Form("form", pane);

    n = 0;
    XtSetArg(args[n], XtNtitle, "Multi Channel Cross Correlation"); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_SECONDS); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    plot1 = new WaveformView("plot1", form, info_area, args, n);
    plot1->addActionListener(this, XtNselectDataCallback);

    setButtonsSensitive();

    print_window = NULL;
    num_mccc = 0;
    mccc = NULL;

    addPlugins("MultiChannelCC", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(vandecar_crosson_button, "Vandecar Crosson");
	tool_bar->add(shearer_button, "Shearer");
    }
}

MultiChannelCC::~MultiChannelCC(void)
{
    Free(mccc);
}

void MultiChannelCC::actionPerformed(ActionEvent *a)
{
    const char *cmd = a->getActionCommand();
    const char *reason = a->getReason();

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
    else if(!strcmp(cmd, "Delete")) {
	remove();
    }
    else if(!strcmp(cmd, "Vandecar Crosson")) {
	compute(VANDECAR_CROSSON);
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Shearer")) {
	compute(SHEARER);
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Align Waveforms")) {
	align();
    }
    else if(!strcmp(cmd, "Copy to Main Window")) {
	copyWaveform();
    }
    else if(!strcmp(cmd, "Multi Channel Cross Correlation Help")) {
	showHelp(cmd);
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

ParseCmd MultiChannelCC::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "Clear")) {
	clear_button->activate();
    }
    else if(parseCompare(cmd, "Delete")) {
	delete_button->activate();
    }
    else if(parseCompare(cmd, "Copy_to_Main_Window")) {
	copy_button->activate();
    }
    else if(parseCompare(cmd, "Vandecar_Crosson")) {
	vandecar_crosson_button->activate();
    }
    else if(parseCompare(cmd, "Shearer")) {
	shearer_button->activate();
    }
    else if(parseCompare(cmd, "Align_Waveforms")) {
	align_button->activate();
    }
    else if(parseString(cmd, "print_window", c)) {
	if(!print_window) {
	    print_window = new PrintDialog(
		"Print Multi Channel Cross Correlation", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "table", c)) {
	return table->parseCmd(c, msg);
    }
    else if(parseString(cmd, "wplot", c)) {
	return plot1->parseCmd(c, msg);
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

ParseVar MultiChannelCC::parseVar(const string &name, string &value)
{
    string c;

    if(parseString(name, "table", c)) {
        return table->parseVar(c, value);
    }
    else if(parseString(name, "wplot", c)) {
        return plot1->parseVar(c, value);
    }
    return Frame::parseVar(name, value);
}

void MultiChannelCC::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%smultiChannelCC\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%scopy_to_main_window\n", prefix);
    printf("%svandecar_crosson\n", prefix);
    printf("%sshearer\n", prefix);
    printf("%salign_waveforms\n", prefix);
    printf("%sprint.help\n", prefix);
    Table::parseHelp(prefix);
}

void MultiChannelCC::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Multi Channel Cross Correlation", this, this);
    }
    print_window->setVisible(true);
}

void MultiChannelCC::print(FILE *fp, PrintParam *p)
{
    p->top_title = NULL;
    p->x_axis_label = (char *)"time (seconds)";
    AxesParm *a = plot1->hardCopy(fp, p, NULL);
    Free(a);
}

void MultiChannelCC::compute(Method method)
{
    double lag;
    double d, duration = 0.;
    int i, num_waveforms = 0, select_mode = 0;
    GTimeSeries **ts_list, **mccc_ts_list, *ts=NULL, *mccc_ts;
    gvector<Waveform *> wvec;

    lag = 0.0;

    if (data_source == NULL) {
      showWarning("No data.");
      return;
    }

    if(!mccc) mccc = (MCCCStruct *)mallocWarn(sizeof(MCCCStruct));

    num_waveforms = data_source->getSelectedWaveforms("a", wvec);
    if (num_waveforms < 3) {
      num_waveforms = data_source->getSelectedWaveforms(wvec);
      select_mode = 2;
    }
    else {
      select_mode = 1;
    }
    if (num_waveforms < 3) {
      showWarning("Less than 3 waveforms selected.");
      return;
    }

    mccc =
      (MCCCStruct *)mallocWarn((num_mccc+num_waveforms)*sizeof(MCCCStruct));
    ts_list =
      (GTimeSeries **)mallocWarn(num_waveforms*sizeof(GTimeSeries *));

    setCursor("hourglass");

    Waveform::sortByY0(wvec);
    for(i = 0; i < num_waveforms; i++) {
      if (select_mode == 1) {
	double tbeg  = wvec[i]->dw[0].d1->time();
	double tend  = wvec[i]->dw[0].d2->time();
	ts = wvec[i]->ts->subseries(tbeg, tend);
      }
      else if (select_mode == 2) {
	ts = wvec[i]->ts;
      }

      ts_list[i] = ts;
    }

    if (!(mccc_ts_list = multiChannelCCTS(ts_list, num_waveforms,
					  method, lag, NULL, NULL))) { 
      showWarning("Failed to MCCC. See terminal for details.");
    }
    else {
      for(i = 0; i < num_waveforms; i++) {
	if (select_mode == 1) {
	  ts = ts_list[i];
	  ts->deleteObject();
	}

	mccc_ts = mccc_ts_list[i];

	mccc_ts->putValue("source_id", wvec[i]->getId());

	multiChannelCCAdd(wvec[i], mccc_ts);

	d = mccc_ts->tend() - mccc_ts->tbeg();
	if(duration < d) duration = d;
      }

      table->adjustColumns();

      plot1->setTimeLimits(-.2*duration, duration);

    }

    setCursor("default");
}

void MultiChannelCC::multiChannelCCAdd(Waveform *w, GTimeSeries *mccc_ts)
{
    const char *row[5];
    char maxMCCC[20], time[50], absCorrLag[50], relCorrLag[50];
    GDataPoint *dmin, *dmax, *dp;
    CPlotInputStruct input;
    
    input.color = w->fg;
    mccc_ts->setSta(w->sta());
    input.chan = w->chan() + string("C");
    mccc_ts->setNet(w->net());
    input.tag = w->sta() + string("/") + input.chan;
    plot1->addTimeSeries(mccc_ts, &input);

    dmin = mccc_ts->minPoint();
    dmax = mccc_ts->maxPoint();

    dp = (fabs(dmin->value()) > fabs(dmax->value())) ? dmin : dmax;

    strcpy(mccc[num_mccc].sta, w->sta());
    strcpy(mccc[num_mccc].chan, w->chan());
    mccc_ts->getValue("max_coef", &mccc[num_mccc].max_coef);
    mccc[num_mccc].time = dp->time();
    mccc[num_mccc].abs_corr_lag = dp->time();
    mccc[num_mccc].rel_corr_lag = dp->time() - mccc[0].time;

    int num_columns = table->numColumns();
    for(int i = 0; i < num_columns; i++)
    {
	TAttribute a = table->getAttribute(i);
	if(!strcmp(a.name, "sta")) {
	    row[i] = mccc[num_mccc].sta;
	}
	else if(!strcmp(a.name, "chan")) {
	    row[i] = mccc[num_mccc].chan;
	}
	else if(!strcmp(a.name, "max coef")) {
	    snprintf(maxMCCC, sizeof(maxMCCC), a.format, mccc[num_mccc].max_coef);
	    row[i] = maxMCCC;
	}
	else if(!strcmp(a.name, "time")) {
	    if(!strcmp(a.format, "%t")) {
		row[i] = timeEpochToString(mccc[num_mccc].time, time, sizeof(time),
					   YMONDHMS);
	    }
	    else {
		snprintf(time, sizeof(time), a.format, mccc[num_mccc].time);
	    }
	}
	else if (!strcmp(a.name, "abs corr lag")) {
	    strcpy(absCorrLag, "-999.9");
	    if(table->numRows() > 0) {
		snprintf(absCorrLag, sizeof(absCorrLag), a.format,
			 mccc[num_mccc].abs_corr_lag);
	    }
	    row[i] = absCorrLag;
	}
	else if (!strcmp(a.name, "rel corr lag")) {
	    strcpy(relCorrLag, "-999.9");
	    if(table->numRows() > 0) {
		snprintf(relCorrLag, sizeof(relCorrLag), a.format,
			 mccc[num_mccc].rel_corr_lag);
	    }
	    row[i] = relCorrLag;
	}


    }

    mccc_ts->putValue("mccc_max_coef", mccc[num_mccc].max_coef);
    mccc_ts->putValue("mccc_time", mccc[num_mccc].time);
    mccc_ts->putValue("mccc_abs_corr_lag", mccc[num_mccc].abs_corr_lag);
    mccc_ts->putValue("mccc_rel_corr_lag", mccc[num_mccc].rel_corr_lag);
    dmin->deleteObject();
    dmax->deleteObject();

    table->addRow(row, false);

    num_mccc++;

}

void MultiChannelCC::list(void)
{
    const char *row[5];
    char maxMCCC[20], time[50], timeLag[50];
    double t0 = mccc[0].time;

    table->removeAllRows();
    int num_columns = table->numColumns();
    for(int j = 0; j < num_mccc; j++) {
	for(int i = 0; i < num_columns; i++)
	{
	    TAttribute a = table->getAttribute(i);
	    if(!strcmp(a.name, "sta")) {
		row[i] = mccc[j].sta;
	    }
	    else if(!strcmp(a.name, "chan")) {
		row[i] = mccc[j].chan;
	    }
	    else if(!strcmp(a.name, "max coef")) {
		snprintf(maxMCCC, sizeof(maxMCCC), a.format, mccc[j].max_coef);
		row[i] = maxMCCC;
	    }
	    else if(!strcmp(a.name, "time")) {
		if(!strcmp(a.format, "%t")) {
		    row[i] = timeEpochToString(mccc[j].time, time, sizeof(time),
					YMONDHMS);
		}
		else {
		    snprintf(time, sizeof(time), a.format, mccc[j].time);
		}
	    }
	    else {
		snprintf(timeLag, sizeof(timeLag), a.format,
				mccc[j].time - t0);
		row[i] = timeLag;
	    }
	}
	table->addRow(row, false);
    }
    table->adjustColumns();
}

void MultiChannelCC::setButtonsSensitive(void)
{
    bool sensitive = (table->numRows() > 0) ? true : false;
    align_button->setSensitive(sensitive);
    clear_button->setSensitive(sensitive);

    sensitive = table->rowSelected();
    copy_button->setSensitive(sensitive);
    delete_button->setSensitive(sensitive);
}

void MultiChannelCC::copyWaveform(void)
{
    int i, num_waveforms;
    double d, tmin, tmax;
    GTimeSeries *mccc_ts;
    CPlotInputStruct input;
    gvector<Waveform *> wvec;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if(!(num_waveforms = plot1->getSelectedWaveforms(wvec))) {
	showWarning("No multi channel cross correlation waveforms selected.");
	return;
    }

    tmin = 0.;
    tmax = 0.;

    for(i = 0; i < num_waveforms; i++) {
	input.display_t0 = wvec[i]->tbeg();
	mccc_ts = new GTimeSeries(wvec[i]->ts);
	input.color = wvec[i]->fg;
	mccc_ts->setSta(wvec[i]->sta());
	mccc_ts->setChan(wvec[i]->chan());
	mccc_ts->setNet(wvec[i]->net());
	input.tag =  mccc_ts->sta() + string("/") + mccc_ts->chan();
	wp->addTimeSeries(mccc_ts, &input);
	d = mccc_ts->tbeg();
	if(tmin > d) tmin = d;
	d = mccc_ts->tend();
	if(tmax < d) tmax = d;
    }
}

void MultiChannelCC::clear(void)
{
    plot1->clear();
    table->removeAllRows();
    num_mccc = 0;
    Free(mccc);
    setButtonsSensitive();
}

void MultiChannelCC::remove(void)
{
    int i, j, n, num_waveforms, num_selected;
    vector<int> rows;
    gvector<Waveform *> wvec, cds;

    num_waveforms = plot1->getWaveforms(cds, false);

    if(!(num_selected = plot1->getSelectedWaveforms(wvec))) {
	showWarning("No multi channel cross correlation waveforms selected.");
	return;
    }

    plot1->deleteWaveforms(wvec);

    n = 0;
    for(i = 0; i < num_waveforms; i++) {
	for(j = 0; j < num_selected && cds[i] != wvec[j]; j++);
	if(j < num_selected) {
	    rows.push_back(i);
	    n++;
	}
    }
    table->removeRows(rows);

    for(i = num_mccc-1; i >= 0; i--) {
	for(j = 0; j < n && rows[j] != i; j++);
	if(j < n) {
	    // delete row i;
	    for(j = i; j < num_mccc-1; j++) {
		mccc[j] = mccc[j+1];
	    }
	    num_mccc--;
	}
    }

    if((num_waveforms = plot1->getWaveforms(wvec, false))) {
	vector<double> y;
	for(i = 0; i < num_waveforms; i++) {
	    y.push_back(i+1);
	}
	plot1->positionY(wvec, y);
    }
}

void MultiChannelCC::selectWaveform(void)
{
    vector<bool> states;
    int i, j, num_waveforms, num_selected;
    gvector<Waveform *> wvec, cds;

    if((num_waveforms = plot1->getWaveforms(cds, false)) <= 0) return;

    num_selected = plot1->getSelectedWaveforms(wvec);

    for(i = 0; i < num_waveforms; i++) {
	for(j = 0; j < num_selected && cds[i] != wvec[j]; j++);
	states.push_back((j < num_selected) ? true : false);
    }

    table->setRowStates(states);
}

void MultiChannelCC::selectRow(void)
{
    int i, j, num_waveforms, num_selected;
    vector<int> rows;
    gvector<Waveform *> wvec;
    bool select;

    num_selected = table->getSelectedRows(rows);

    num_waveforms = plot1->getWaveforms(wvec, false);

    for(i = 0; i < num_waveforms; i++) {
	for(j = 0; j < num_selected && rows[j] != i; j++);
	select = (j < num_selected) ? true : false;
	plot1->selectWaveform(wvec[i], select, false);
    }
}

void MultiChannelCC::align(void)
{
    double mccc_time=0., screen_mccc_time=0.;
    int id = -1, i, j, num, num_waveforms, n;
    vector<double> scaled_x0;
    gvector<Waveform *> wvec, cds, cv;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if(!(num = plot1->getWaveforms(cds, false))) return;

    if(!(num_waveforms = data_source->getWaveforms(wvec, false))) {
	return;
    }

    n = 0;
    for(i = 0; i < num; i++)
    {
	GTimeSeries *mccc_ts = cds[i]->ts;
	if( !mccc_ts->getValue("source_id", &id) ||
		!mccc_ts->getValue("mccc_time", &mccc_time)) return;

	for(j = 0; j < num_waveforms && n < num; j++) {
	    if(wvec[j]->getId() == id) {
		if(n == 0) {
		    screen_mccc_time = wvec[j]->scaled_x0 + mccc_time
					- wvec[j]->tbeg();
		}
		else {
		    scaled_x0.push_back(screen_mccc_time -
					(mccc_time - wvec[j]->tbeg()));
		    cv.push_back(wvec[j]);
		}
		n++;
	    }
	}
    }
    if(n > 1) {
	wp->positionX(cv, scaled_x0);
    }
}
