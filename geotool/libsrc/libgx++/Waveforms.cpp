/** \file Waveforms.cpp
 *  \brief Defines class Waveforms.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "Waveforms.h"
#include "motif++/MotifClasses.h"
#include "CSSTable.h"
#include "Waveform.h"
#include "WaveformWindow.h"
#include "WaveformPlot.h"
#include "TableMenu.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libtime.h"
#include "libstring.h"
}

class SegmentTable : public Frame
{
    public:
	SegmentTable(const string &, Component *, GTimeSeries *);

	Button		*close_button;
	Label		*label;
	TableMenu 	*table_menu;
	Table		*table;
	GTimeSeries	*ts;

	void actionPerformed(ActionEvent *action_event);
	void list(void);
	void setTimeSeries(GTimeSeries *timeseries) {
	    if(ts) ts->removeOwner(this);
	    ts = timeseries;
	    if(ts) ts->addOwner(this);
	    list();
	}
	void clear(void) {
	    table->removeAllRows();
	    if(ts) ts->removeOwner(this);
	    ts = NULL;
	}
};
    
SegmentTable::SegmentTable(const string &name, Component *parent,
		GTimeSeries *timeseries) : Frame(name, parent)
{
    Arg args[20];
    int n;

    ts = timeseries;
    if(ts) ts->addOwner(this);

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toobar", this, menu_bar);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    table_menu = new TableMenu("View", menu_bar);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("sta/chan", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNcolumns, 7); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XtNwidth, 450); n++;
    const char *column_labels[] = {"index", "time", "endtime", "nsamp",
				"samprate", "calib", "calper"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    table = new Table("table", frame_form, args, n);

    table->setAttributes(
"index,%d,time,%t,endtime,%t,nsamp,%d,samprate,%.3f,calib,%.3f,calper,%.3f");
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    table_menu->setTable(table);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
    }

    list();
}

void SegmentTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(action_event->getReason(), XtNattributeChangeCallback)) {
        list();
    }
}

void SegmentTable::list(void)
{
    table->removeAllRows();

    int num_columns = table->numColumns();
    if( !num_columns || !ts) return;

    char lab[50];
    snprintf(lab, sizeof(lab), "%s/%s", ts->sta(), ts->chan());
    label->setLabel(lab);

    const char **row = (const char **)mallocWarn(num_columns*sizeof(char *));
    char ndex[10], time[50], endtime[50], nsamp[20], samprate[20], calib[20],
		calper[20];

    for(int i = 0; i < ts->size(); i++) {
	GSegment *s = ts->segment(i);
	for(int j = 0; j < num_columns; j++) {
	    TAttribute a = table->getAttribute(j);
	    if(!strcmp(a.name, "index")) {
		snprintf(ndex, sizeof(ndex), a.format, i+1);
		row[j] = ndex;
	    }
	    else if(!strcmp(a.name, "time")) {
		if(!strcmp(a.format, "%t")) {
		    timeEpochToString(s->tbeg(), time, sizeof(time), YMONDHMS);
		}
		else {
		    snprintf(time, sizeof(time), a.format, s->tbeg());
		}
		row[j] = time;
            }
	    else if(!strcmp(a.name, "endtime")) {
		if(!strcmp(a.format, "%t")) {
		    timeEpochToString(s->tend(), endtime, sizeof(endtime),
				YMONDHMS);
		}
		else {
		    snprintf(endtime, sizeof(endtime), a.format, s->tend());
		}
		row[j] = endtime;
            }
	    else if(!strcmp(a.name, "nsamp")) {
		snprintf(nsamp, sizeof(nsamp), a.format, s->length());
		row[j] = nsamp;
	    }
	    else if(!strcmp(a.name, "samprate")) {
		if(s->tdel() != 0.) {
		    snprintf(samprate, sizeof(samprate), a.format,1./s->tdel());
		}
		else {
		    snprintf(samprate, sizeof(samprate), "-");
		}
		row[j] = samprate;
	    }
	    else if(!strcmp(a.name, "calib")) {
		snprintf(calib, sizeof(calib), a.format, s->calib());
		row[j] = calib;
	    }
	    else if(!strcmp(a.name, "calper")) {
		snprintf(calper, sizeof(calper), a.format, s->calper());
		row[j] = calper;
	    }
	}
	table->addRow(row, false);
    }
    table->adjustColumns();
    Free(row);
}

class WaveformTable : public CSSTable
{
    public:
	WaveformTable(const string &name, Component *parent, Waveforms *w,
			Arg *args, int n) : CSSTable(name, parent, args, n)
	{
	    waveforms = w;
	}
	~WaveformTable() {}

    protected:
	Waveforms *waveforms;
	bool getExtra(int i, CssTableClass *o, TAttribute *a, string &value);
};

bool WaveformTable::getExtra(int i, CssTableClass *o, TAttribute *a, string &value)
{
    char s[50];
    Waveform *w = waveforms->ws[i];
    GTimeSeries *ts = w->ts;

    value.assign("-");

    if(!strcmp(a->name, "net")) {
	value.assign(w->net());
    }
    else if(!strcmp(a->name, "lat")) {
	snprintf(s, sizeof(s), a->format, w->lat());
	value.assign(s);
    }
    else if(!strcmp(a->name, "lon")) {
	snprintf(s, sizeof(s), a->format, w->lon());
	value.assign(s);
    }
    else if(!strcmp(a->name, "elev")) {
	snprintf(s, sizeof(s), a->format, w->elev());
	value.assign(s);
    }
    else if(!strcmp(a->name, "orid")) {
	CssOriginClass *origin = waveforms->data_source->getPrimaryOrigin(w);
	if(!origin) return false;
	snprintf(s, sizeof(s), a->format, origin->orid);
	value.assign(s);
    }
    else if(!strcasecmp(a->name, "distance(km)") ||
		!strcasecmp(a->name, "distance(deg)") ||
		!strcasecmp(a->name, "azimuth") ||
		!strcasecmp(a->name, "baz"))
    {
	double delta, az, baz;
	CssOriginClass *origin = waveforms->data_source->getPrimaryOrigin(w);
	if(!origin) return false;
 	if(w->lat() > -900. && w->lon() > -900.)
	{
	    deltaz(origin->lat, origin->lon, w->lat(), w->lon(),
			&delta, &az, &baz);
	    if(!strcasecmp(a->name, "distance(km)")) {
		delta *= DEG_TO_KM;
	    }
	    else if(!strcasecmp(a->name, "azimuth")) {
		delta = az;
	    }
	    else if(!strcasecmp(a->name, "baz")) {
		delta = baz;
	    }
	    snprintf(s, sizeof(s), a->format, delta);
	    value.assign(s);
	}
    }
    else if(!strcmp(a->name, "refsta")) {
	ts->getRefsta(value);
    }
    else if(!strcmp(a->name, "x_chan")) {
	ts->getXChan(value);
    }
    else if(!strcmp(a->name, "y_chan")) {
	ts->getYChan(value);
    }
    else if(!strcmp(a->name, "z_chan")) {
	ts->getZChan(value);
    }
    else if(!strcmp(a->name, "hang")) {
	snprintf(s, sizeof(s), a->format, ts->hang());
	value.assign(s);
    }
    else if(!strcmp(a->name, "vang")) {
	snprintf(s, sizeof(s), a->format, ts->vang());
	value.assign(s);
    }
    else if(!strcmp(a->name, "dnorth")) {
	snprintf(s, sizeof(s), a->format, ts->dnorth());
	value.assign(s);
    }
    else if(!strcmp(a->name, "deast")) {
	snprintf(s, sizeof(s), a->format, ts->deast());
	value.assign(s);
    }
    else if(!strcmp(a->name, "alpha")) {
	snprintf(s, sizeof(s), a->format, ts->alpha());
	value.assign(s);
    }
    else if(!strcmp(a->name, "beta")) {
	snprintf(s, sizeof(s), a->format, ts->beta());
	value.assign(s);
    }
    else if(!strcmp(a->name, "gamma")) {
	snprintf(s, sizeof(s), a->format, ts->gamma());
	value.assign(s);
    }
    else if(!strcmp(a->name, "current_alpha")) {
	snprintf(s, sizeof(s), a->format, ts->currentAlpha());
	value.assign(s);
    }
    else if(!strcmp(a->name, "current_beta")) {
	snprintf(s, sizeof(s), a->format, ts->currentBeta());
	value.assign(s);
    }
    else if(!strcmp(a->name, "current_gamma")) {
	snprintf(s, sizeof(s), a->format, ts->currentGamma());
	value.assign(s);
    }
    else {
	return false;
    }
    return true;
}


Waveforms::Waveforms(const string &name, Component *parent, DataSource *ds) :
		Frame(name, parent), DataReceiver(ds)
{
    if(data_source) {
	data_source->addDataListener(this);
    }
    createInterface();
}

void Waveforms::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
//    remove_button = new Button("Remove", edit_menu, this);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);
    deselect_all_button = new Button("Deselect All", view_menu, this);
    promote_rows_button = new Button("Promote Selected Rows", view_menu, this);
    promote_cols_button = new Button("Promote Selected Columns",view_menu,this);
    sort_button = new Button("Sort by Selected Columns", view_menu, this);
    sort_selected_button = new Button("Sort Selected Rows Only",view_menu,this);
    sort_unique_button = new Button("Sort Unique", view_menu, this);
    expand_button = new Button("Expand", view_menu, this);
    reverse_button = new Button("Reverse Order", view_menu, this);
    segments_button = new Button("Segment Information...", view_menu, this);
    segments_button->setSensitive(false);
    show_all_button = new Button("Show All", view_menu, this);
    if(data_source && data_source->getWaveformPlotInstance()) {
	apply_sort_button = new Button("Apply Sort to Waveforms", view_menu,
					this);
    }
    else {
	apply_sort_button = NULL;
    }

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Waveforms Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableTitle, "Waveforms"); n++;
    XtSetArg(args[n], XtNtableInfoWidget, info_area->leftInfo()); n++;
    XtSetArg(args[n], XtNcursorInfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XtNwidth, 400); n++;
    table = new WaveformTable("waveforms", frame_form, this, args, n);
    const char *extra[] = {"net", "orid", "lat", "lon", "elev", "distance(km)",
		"distance(deg)", "azimuth", "baz", "refsta", "hang", "vang",
		"dnorth", "deast", "alpha", "beta", "gamma",
		"current_alpha", "current_beta", "current_gamma",
		"x_chan", "y_chan", "z_chan", "file", "file_index",};
    const char *extra_formats[] = {"%s", "%d", "%.1f", "%.1f", "%.1f", "%.1f",
		"%.1f", "%.1f", "%.1f", "%s", "%.3f", "%.3f",
		"%.3f", "%.3f", "%.1f", "%.1f", "%.1f",
		"%.1f", "%.1f", "%.1f",
		"%s", "%s", "%s", "%s", "%d"};
/*
    const char *display_list =
"sta,%s, chan,%s, net,%s, orid,%d,time,%t, wfid,%d, chanid,%d, jdate,%d,\
endtime,%t, nsamp,%d, samprate,%.2f, calib,%.4f, calper,%.2f, instype,%s,\
segtype,%s, datatype,%s, clip,%s, dir,%s, dfile,%s, foff,%d, commid,%d,\
lddate,%s, lat,%.2f, lon,%.2f, distance(km), %.2f, distance(deg),%.2f,\
azimuth,%.2f, baz,%.2f, file,%s, file_index,%d";

    table->setType(cssWfdisc, 10, extra, extra_formats, display_list);
*/
    table->setType(cssWfdisc, 24, extra, extra_formats);
    table->adjustColumns();

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNselectColumnCallback);

    if(!tool_bar->loadDefaults()) {
	tool_bar->add(close_button);
	tool_bar->add(segments_button);
    }

    segment_table = new SegmentTable("Segment Information", this, NULL);

    list();

    setButtonsSensitive();
}

Waveforms::~Waveforms(void)
{
}

void Waveforms::actionPerformed(ActionEvent *action_event)
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
    else if(!strcmp(cmd, "Promote Selected Rows")) {
	table->promoteSelectedRows();
    }
    else if(!strcmp(cmd, "Promote Selected Columns")) {
	table->promoteSelectedColumns();
    }
    else if(!strcmp(cmd, "Sort by Selected Columns")) {
	vector<int> columns;
	table->getSelectedColumns(columns);
	table->sortByColumns(columns);
    }
    else if(!strcmp(cmd, "Sort Selected Rows Only")) {
	vector<int> columns;
	table->getSelectedColumns(columns);
	table->sortSelected(columns);
    }
    else if(!strcmp(cmd, "Sort Unique")) {
	vector<int> columns;
	table->getSelectedColumns(columns);
	table->sortUnique(columns);
    }
    else if(!strcmp(cmd, "Expand")) {
	vector<int> columns;
	table->getSelectedColumns(columns);
	if((int)columns.size() > 0) {
	    table->expand(columns[0]);
        }
    }
    else if(!strcmp(cmd, "Reverse Order")) {
	table->reverseOrder();
    }
    else if(!strcmp(cmd, "Show All")) {
	table->showAll();
    }
    else if(!strcmp(cmd, "Apply Sort to Waveforms") && data_source)
    {
	applySort();
	list();
    }
    else if(comp == table) { // select row or column
	setButtonsSensitive();
	if(!strcmp(reason, XtNselectRowCallback)) {
	  selectRow((MmTableSelectCallbackStruct *)action_event->getCalldata());
	}
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	DataChange *d = (DataChange *)action_event->getCalldata();
	if(d->select_waveform) {
	    selectWaveform();
	}
	else if(d->waveform || d->sort_waveforms || d->primary_origin) {
	    list();
	}
    }
    else if(!strcmp(cmd, "Segment Information...")) {
	gvector<Waveform *> wvec;
	data_source->getSelectedWaveforms(wvec);
	if(wvec.size() > 0) {
	    segment_table->setTimeSeries(wvec[0]->ts);
	    segment_table->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "Remove")) {
//	remove();
    }
    else if(!strcmp(cmd, "Waveforms Help")) {
	showHelp("Waveforms Help");
    }
}

void Waveforms::applySort(void)
{
    int i, j, num;
    vector<int> order;
    vector<double> y;
    gvector<Waveform *> wvec, w_new;
    vector<bool> on;
    WaveformPlot *wp = NULL;

    if(!data_source || !(wp = data_source->getWaveformPlotInstance())) return;

    data_source->getWaveforms(wvec);

    num = table->getRowOrder(order);

    for(i = 0; i < num; i++) {
	for(j = 0; j < w_new.size(); j++) {
	    if(w_new[j] == ws[order[i]]) break;
	}
	if(j == w_new.size()) {
	     w_new.push_back(ws[order[i]]);
	}
    }
    for(i = 0; i < w_new.size(); i++) y.push_back(i+1);

    for(i = 0; i < wvec.size(); i++) {
	for(j = 0; j < w_new.size(); j++) {
	    if(wvec[i] == w_new[j]) break;
	}
	on.push_back((j < w_new.size()) ? true : false);
    }

    wp->setDataDisplay(wvec, on, false, false);

    wp->setYLabelInt(true, false);

    wp->positionY(w_new, y);
}

void Waveforms::list(void)
{
    segment_table->clear();

    table->removeAllRecords();

    ws.clear();

    if(!data_source) return;

    gvector<Waveform *> wvec;
    data_source->getWaveforms(wvec);
    Waveform::sortByY0(wvec);

    for(int i = 0; i < wvec.size(); i++) {
	cvector<CssWfdiscClass> *v = wvec[i]->ts->getWfdiscs();
	if(v) {
	    for(int j = 0; j < v->size(); j++) {
		table->addRecord(v->at(j), false);
		ws.push_back(wvec[i]);
	    }
	}
	else {
	    string s;
	    GTimeSeries *ts = wvec[i]->ts;
	    CssWfdiscClass *wfdisc = new CssWfdiscClass();
	    ts->getSta(s);
	    stringcpy(wfdisc->sta, s.c_str(), sizeof(wfdisc->sta));
	    ts->getChan(s);
	    stringcpy(wfdisc->chan, s.c_str(), sizeof(wfdisc->chan));
	    wfdisc->time = ts->tbeg();
	    wfdisc->endtime = ts->tend();
	    wfdisc->nsamp = ts->length();
	    if(ts->size() > 0) {
		if(ts->segment(0)->tdel() != 0.) {
		    wfdisc->samprate = 1./ts->segment(0)->tdel();
		}
		wfdisc->calib = ts->segment(0)->calib();
		wfdisc->calper = ts->segment(0)->calper();
	    }
	    wfdisc->jdate = ts->jdate();
	    table->addRecord(wfdisc, false);
	    ws.push_back(wvec[i]);
	}
    }

    if(ws.size() > 0) table->list();
    selectWaveform();
}

void Waveforms::setButtonsSensitive(void)
{
    bool row_selected = table->rowSelected();
    bool col_selected = table->columnSelected();

    deselect_all_button->setSensitive(row_selected);
    promote_rows_button->setSensitive(row_selected);
    promote_rows_button->setSensitive(row_selected);
//    remove_button->setSensitive(row_selected);

    promote_cols_button->setSensitive(col_selected);
    sort_button->setSensitive(col_selected);
    sort_unique_button->setSensitive(col_selected);

    sort_selected_button->setSensitive(row_selected && col_selected);
    expand_button->setSensitive(row_selected && col_selected);

    if(table->numSelectedRows() == 1) {
	segments_button->setSensitive(true);
    }
    else {
	segments_button->setSensitive(false);
    }
}

void Waveforms::selectWaveform(void)
{
    vector<bool> states;

    table->getRowStates(states);

    for(int i = 0; i < ws.size() && i < (int)states.size(); i++)
    {
	if(ws[i]->selected != states[i])
	{
	    table->selectRow(i, ws[i]->selected);
	}
    }
}

void Waveforms::selectRow(MmTableSelectCallbackStruct *c)
{
    if(!data_source) return;
    int *rows = (int *)mallocWarn(c->nrows*sizeof(int));
    bool *states = (bool *)mallocWarn(c->nrows*sizeof(bool));

    for(int i = 0; i < c->nrows; i++) rows[i] = -1;

    for(int j = 0; j < c->nchanged_rows; j++)
    {
	int row = c->changed_rows[j];
	if(ws[row]->selected != c->states[row]) {
	    data_source->selectWaveform(ws[row], c->states[row]);
	}

	for(int i = 0; i < ws.size(); i++)
	{
	    if(i != row && ws[i] == ws[row])
	    {
		rows[i] = i;
		states[i] = c->states[row];
	    }
	}
    }
    int n = 0;
    for(int i = 0; i < c->nrows; i++) {
	if(rows[i] != -1) {
	    rows[n] = rows[i];
	    states[n] = states[i];
	    n++;
	}
    }
    table->selectRows(n, rows, states);
    Free(rows);
    Free(states);
}
