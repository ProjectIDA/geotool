/** \file CreateBeam.cpp
 *  \brief Defines class CreateBeam
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

#include "CreateBeam.h"
#include "motif++/MotifClasses.h"
#include "widget/TabClass.h"
#include "CSSTable.h"
#include "MultiTable.h"
#include "widget/Table.h"
#include "Beam.h"
#include "Iaspei.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
static int sort_phases(const void *A, const void *B);
static int sort_by_time(const void *A, const void *B);
}

CreateBeam::CreateBeam(const string &name, Component *parent,
		WaveformPlot *wave_plot) : FormDialog(name, parent)
{
    wp = wave_plot;
    createInterface();
}

void CreateBeam::createInterface(void)
{
    int n;
    Arg args[20];
    Component *parent;

    // Input Slowness tab
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNheight, 250); n++;
    tab = new TabClass("tab", this, args, n);
    tab->addActionListener(this, XtNtabCallback);
    parent = tab;

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    input_rc = new RowColumn("Input Slowness", parent, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    input_rc2 = new RowColumn("input_rc2", input_rc, args, n);

    label1 = new Label("azimuth (deg)", input_rc2);
    label2 = new Label("slowness", input_rc2);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    azimuth_text1 = new TextField("azimuth_text1", input_rc2, this, args, n);
    slowness_text1 = new TextField("slowness_text1", input_rc2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    input_rc3 = new RowColumn("input_rc3", input_rc, args, n);

    label3 = new Label("slowness units", input_rc3);
    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb = new RadioBox("Units", input_rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    s_km = new Toggle("s/km", rb, args, n);
    s_km->set(true);
    s_deg = new Toggle("s/deg", rb, args, n);
    s_deg->set(false);

    new Separator("sep", input_rc);

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb2 = new RadioBox("type", input_rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    beam_toggle = new Toggle("beam", rb2, this, args, n);
    beam_toggle->set(true);
    ftrace_toggle = new Toggle("ftrace", rb2, this, args, n);
    ftrace_toggle->set(false);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    input_rc4 = new RowColumn("input_rc4", input_rc, args, n);

    label4 = new Label("window length (secs)", input_rc4);
    label5 = new Label("signal-to-noise ratio", input_rc4);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    window_text = new TextField("window_text", input_rc4, this, args, n);
    window_text->setString("2.0");
    snr_text = new TextField("snr_text", input_rc4, this, args, n);
    snr_text->setString("4.0");
    input_rc4->setSensitive(false);

    // Origins tab
    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    origin_rc = new RowColumn("Origins", tab, args, n);
    tab->setOnTop("Input Slowness");
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form = new Form("form", origin_rc);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    origin_rc2 = new RowColumn("origin_rc2", form, args, n);

    phase_label = new Label("Phase", origin_rc2);
    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("sw", origin_rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    phase_list = new List("phase_list", sw, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, origin_rc2->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableBackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    origin_table = new MultiTable("origin_table", form, args, n);
    origin_table->setVisible(true);
    origin_table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    RowColumn *rco = new RowColumn("rco", origin_rc, args, n);

    slowness_label = new Label("slowness (s/km)", rco);
    azimuth_label = new Label("azimuth (deg)", rco);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    slowness_text2 = new TextField("slowness_text2", rco, this, args, n);
    azimuth_text2 = new TextField("azimuth_text2", rco, this, args, n);

    // Arrivals tab
    Form *arrival_form = new Form("Arrivals", tab);
    tab->setOnTop("Input Slowness");
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    arrival_rc = new RowColumn("arrival_rc", arrival_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    recipe_waveforms = new Toggle("Recipe Waveforms", arrival_rc, this, args,n);
    recipe_waveforms->set(true);
    new Space("space", arrival_rc, XmHORIZONTAL, 10);
    selected_waveforms = new Toggle("Selected Waveforms", arrival_rc,
			this, args,n);
    selected_waveforms->set(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, arrival_rc->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XtNtableBackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    arrival_table = new ctable<CssArrivalClass>("arrival_table", arrival_form,
					args, n);
    arrival_table->setVisible(true);
    arrival_table->addActionListener(this, XtNselectRowCallback);

    // Below the tab.
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, tab->baseWidget()); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    compute_button = new Button("Compute", controls, this);
    compute_button->setSensitive(false);
//    align_button = new Button("Align Waveforms", controls, this);
//    align_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    replace_toggle = new Toggle("Replace", controls, args, n);
    replace_toggle->set(true);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    hide_form = new Form("hide_form", this, args, n);
    hide_form->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    waveform_label = new Label("Beam Selected Waveforms", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, waveform_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    filter_rc = new RowColumn("filter_rc", this, args, n);

    label6 = new Label("Beam Filter", filter_rc);
    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb3 = new RadioBox("Filter", filter_rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    filter_on = new Toggle("On", rb3, args, n);
    filter_on->set(true);
    filter_off = new Toggle("Off", rb3, args, n);
    filter_off->set(false);

    const char *column_labels[] = {"Low", "High", "Order", "Type","Constraint"};
    const char *column_choice[] = {"","","0:1:2:3:4:5:6:7:8:9:10","BP:BR:LP:HP",
                                "zero phase:causal"};
    const char *cells[] = {
        "2.0", "4.0", "3", "BP", "causal", (char *)NULL};

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, filter_rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNcolumns, 5); n++;
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNcolumnChoice, column_choice); n++;
    XtSetArg(args[n], XtNcells, cells); n++;
    XtSetArg(args[n], XtNborderWidth, 0); n++;
    table = new Table("table", this, args, n);

    initPhaseList();
    initOriginTable();
    listArrivals();

    wp->addDataListener(this);
}

void CreateBeam::initPhaseList(void)
{
    vector<const char *> plist;
    int num = wp->getIaspeiPhases(plist);
    const char **phases = (const char **)mallocWarn((num+4)*sizeof(char *));

    for(int i = 0; i < num; i++) {
        phases[i] = plist[i];
    }

    phases[num] = "LR";
    phases[num+1] = "LQ";
    phases[num+2] = "T";
    phases[num+3] = "I";
    num += 4;
    qsort(phases, num, sizeof(char *), sort_phases);

    for(int i = 0; i < num; i++) {
	phase_list->addItem(phases[i]);
    }
    Free(phases);
}

void CreateBeam::initOriginTable(void)
{
    string attributes;
    const char *extra[] = {"file", "file_index"};
    const char *extra_format[] = {"%s", "%d"};

    const char *names[] = {"origin", "origerr"};

    if(getProperty("originAttributes", attributes)) {
        origin_table->setType(2, names, 2, extra, extra_format, attributes);
    }
    else {
        origin_table->setType(2, names, 2, extra, extra_format);
    }

    listOrigins();
}

CreateBeam::~CreateBeam(void)
{
}

void CreateBeam::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(comp == tab) {
	setSensitive();
    }
    else if(comp == slowness_text1 || comp == azimuth_text1 ||
	    comp == slowness_text2 || comp == azimuth_text2)
    {
	setSensitive();
    }
    else if(!strcmp(cmd, "Compute")) {
	makeBeam();
    }
    else if(!strcmp(cmd, "Recipe Waveforms")) {
	selected_waveforms->set(!recipe_waveforms->state(), false);
    }
    else if(!strcmp(cmd, "Selected Waveforms")) {
	recipe_waveforms->set(!selected_waveforms->state(), false);
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	DataChange *c = (DataChange *)action_event->getCalldata();
	if(c->origin || c->origerr) {
	    listOrigins();
	}
	if(c->arrival) {
	    listArrivals();
	}
	slowness_text2->setString("");
	azimuth_text2->setString("");
	computeSlowAz();
	setSensitive();
    }
    else if(!strcmp(cmd, "phase_list") || !strcmp(cmd, "origin_table")) {
	slowness_text2->setString("");
	azimuth_text2->setString("");
	computeSlowAz();
	setSensitive();
    }
    else if(!strcmp(cmd, "arrival_table")) {
	setSensitive();
    }
/*
    else if(!strcmp(cmd, "Align Waveforms")) {
	alignWaveforms();
    }
*/
    else if(!strcmp(cmd, "Help")) {
	showHelp("Create Beam Help");
    }
    else if(comp == beam_toggle || comp == ftrace_toggle) {
	input_rc4->setSensitive(ftrace_toggle->state());
    }
}

void CreateBeam::listOrigins(void)
{
    if(!wp) return;

    int i, j, l, n;
    int num_rows;
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;

    wp->getTable(origins);
    wp->getTable(origerrs);

    origin_table->removeAllRecords();

    origins.sort(sort_by_time);

    num_rows = 0;

    for(i = num_rows = 0; i < origins.size(); i++)
    {
	int dc = origins[i]->getDC();
	for(j = n = 0; j < origerrs.size(); j++) {
	    if(origerrs[j]->getDC() == dc
			&& origerrs[j]->orid == origins[i]->orid) n++;
	}
	num_rows += (n > 0 ? n : 1);
    }
    for(i = 0; i < origins.size(); i++)
    {
	int dc = origins[i]->getDC();
	for(l = n = 0; l < origerrs.size(); l++)
		if(origerrs[l]->getDC() == dc
			&& origerrs[l]->orid == origins[i]->orid)
	{
	    n++;
	    origin_table->addRecord(new TableRow(origins[i],origerrs[l]),false);
	}
	if(n == 0)
	{
	    origin_table->addRecord(new TableRow(origins[i]), false);
	}
    }

    origin_table->list();
}

void CreateBeam::listArrivals(void)
{
    if(!wp) return;

    cvector<CssArrivalClass> arrivals;

    wp->getTable(arrivals);

    arrival_table->removeAllRecords();

    arrivals.sortByMember("sta");
    arrival_table->addRecords(arrivals);

    arrival_table->list();
}

bool CreateBeam::getAzimuth(double *azimuth)
{
    if( !strcmp(tab->labelOnTop(), "Input Slowness") ) {
	if(!azimuth_text1->getDouble(azimuth)) {
	    showWarning("Cannot interpret azimuth");
	    return false;
	}
    }
    else {
	if(!azimuth_text2->getDouble(azimuth)) {
	    showWarning("Cannot interpret azimuth");
	    return false;
	}
    }
    return true;
}

bool CreateBeam::getSlowness(double *slowness)
{
    if( !strcmp(tab->labelOnTop(), "Input Slowness") ) {
	if(!slowness_text1->getDouble(slowness)) {
	    showWarning("Cannot interpret slowness");
	    return false;
	}
	if(s_deg->state()) {
	    // convert s from sec/deg to sec/km.
	    *slowness *= 180./(M_PI*6371.);
	}
    }
    else {
	if(!slowness_text2->getDouble(slowness)) {
	    showWarning("Cannot interpret slowness");
	    return false;
	}
    }
    return true;
}

void CreateBeam::computeSlowAz(void)
{
    char s[50];
    string op;
    char **phases;
    double lon, lat, delta, az, baz, slowness;
    BeamLocation beam_location = DNORTH_DEAST;
    gvector<Waveform *> wvec;
    gvector<TableRow*> v;

    origin_table->getSelectedRecords(v);
    if(v.size() <= 0) {
	return;
    }
    TableRow *t = v[0];
    CssOriginClass *origin = (CssOriginClass *)t->tables[0];

    // check for null values
    if(origin->lat < -900. || origin->lon < -900. || origin->depth < -900) {
	return;
    }

    if( phase_list->getSelectedItems(&phases) <= 0) return;

    int num = wp->getSelectedWaveforms(wvec);
    if( num < 2 ) {
	return;
    }
    DataSource *ds = wvec[0]->getDataSource();

    if((beam_location = Beam::getLocation(ds, beam_location, wvec, &lon,				&lat)) == BEAM_LOCATION_ERROR)
    {
	showWarning(GError::getMessage());
	return;
    }

    deltaz(origin->lat, origin->lon, lat, lon, &delta, &az, &baz);

    snprintf(s, sizeof(s), "%.2f", baz);
    azimuth_text2->setString(s);

    wp->getTravelTime(phases[0], origin, lat, lon, wvec[0]->elev(), 
	wvec[0]->net(), wvec[0]->sta(), &slowness, op);
    // convert slowness from sec/deg to sec/km.
    slowness *= 180./(M_PI*6371.);

    snprintf(s, sizeof(s), "%.4f", slowness);
    slowness_text2->setString(s);
}

bool CreateBeam::makeBeam(void)
{
    double azimuth, slowness, window_len, snr;
    char type[3];
    bool repl;
    int order, zero_phase;
    double flo, fhi;
    BeamLocation beam_location = DNORTH_DEAST;
    WaveformPlot *dest = wp;

    repl = replace_toggle->state();

    if(tab && !strcmp(tab->labelOnTop(), "Arrivals") ) {
	cvector<CssArrivalClass> v;
	arrival_table->getSelectedTable(v);
	if(v.size() > 0) {
	    wp->beamArrival(v[0], selected_waveforms->state(), repl);
	    return true;
	}
	return false;
    }

    if( !getAzimuth(&azimuth) || !getSlowness(&slowness) ) return false;

    if(filter_on->state()) {
	if( !getFilter(type, &order, &flo, &fhi, &zero_phase) ) return false;
    }
    else {
	strcpy(type, "NA");
    }

    if(beam_toggle->state()) {
	wp->beamWaveforms(azimuth, slowness, order, type, flo, fhi, zero_phase,
		beam_location, repl, dest);
    }
    else {
	if(!window_text->getDouble(&window_len)) {
	    showWarning("Cannot interpret window length value");
	    return false;
	}
	if(!snr_text->getDouble(&snr)) {
	    showWarning("Cannot interpret snr value");
	    return false;
	}
	wp->computeFtrace(azimuth, slowness, window_len, snr, order, flo, fhi,
		zero_phase, beam_location, repl, dest);
    }
    return true;
}

bool CreateBeam::getFilter(char *type, int *order, double *flo, double *fhi,
		int *zero_phase)
{
    vector<const char *> row;

    table->getRow(0, row);

    if(!strcasecmp(row[3], "BP")) {
	strcpy(type, "BP");
    }
    else if(!strcasecmp(row[3], "LP")) {
	strcpy(type, "LP");
    }
    else if(!strcasecmp(row[3], "HP")) {
	strcpy(type, "HP");
    }
    else if(!strcasecmp(row[3], "BR")) {
	strcpy(type, "BR");
    }
    else {
	showWarning("Cannot interpret filter type: %s", row[3]);
	return false;
    }
    if(!stringToDouble(row[0], flo) || *flo < 0.) {
	showWarning("Cannot interpret Filter Low Frequency.");
	return false;
    }
    if(strcmp(type, "HP"))
    {
	if(!stringToDouble(row[1], fhi) || *fhi < 0.  || *fhi <= *flo)
	{
	    showWarning("Cannot interpret Filter High Frequency.");
	    return false;
	}
    }
    if(!stringToInt(row[2], order) || *order < 1 || *order > 10) {
	showWarning("Cannot interpret Filter Order");
	return false;
    }
    if(!strncasecmp(row[4], "zero", 4)) {
	*zero_phase = 1;
    }
    else if(!strncasecmp(row[4], "causal", 6)) {
	*zero_phase = 0;
    }
    else {
	showWarning("Cannot interpret Filter Constraint");
	return false;
    }
    return true;
}

void CreateBeam::setSensitive(void)
{
    double az, slow;
    bool set = false;

    if(!tab) {
	set = (azimuth_text1->getDouble(&az) &&
			slowness_text1->getDouble(&slow)) ? true : false;
    }
    else if(!strcmp(tab->labelOnTop(), "Input Slowness"))
    {
	set = (azimuth_text1->getDouble(&az) &&
			slowness_text1->getDouble(&slow)) ? true : false;
	if( wp->numSelected() < 2 ) set = false;
	hide_form->setVisible(false);
    }
    else if(!strcmp(tab->labelOnTop(), "Origins"))
    {
	set = (azimuth_text2->getDouble(&az) &&
			slowness_text2->getDouble(&slow)) ? true : false;
	if( wp->numSelected() < 2 ) set = false;
	hide_form->setVisible(false);
    }
    else if(!strcmp(tab->labelOnTop(), "Arrivals") ) {
	set = arrival_table->rowSelected();
	hide_form->setVisible(true);
    }

    compute_button->setSensitive(set);
//    align_button->setSensitive(set);
}

static int
sort_by_time(const void *A, const void *B)
{
    CssOriginClass *a = *(CssOriginClass **)A;
    CssOriginClass *b = *(CssOriginClass **)B;

    if (fabs(a->time - b->time) < 0.000009) return(0);

    return((a->time < b->time) ? -1 : 1);
}

static int
sort_phases(const void *A, const void *B)
{
    char *a = *(char **)A;
    char *b = *(char **)B;
    return strcasecmp(a, b);
}

ParseCmd CreateBeam::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;
    string c;

    if(parseArg(cmd, "slowness", c)) {
	slowness_text1->setString(c);
    }
    else if(parseArg(cmd, "azimuth", c)) {
	azimuth_text1->setString(c);
    }
    else if(parseArg(cmd, "wlen", c)) {
	window_text->setString(c);
    }
    else if(parseArg(cmd, "snr", c)) {
	snr_text->setString(c);
    }
    else if(parseArg(cmd, "beam_filter", c)) {
	if(parseCompare(c, "on")) {
	    filter_on->set(true, true);
	}
	else if(parseCompare(c, "off")) {
	    filter_off->set(true, true);
	}
	else ret = ARGUMENT_ERROR;
    }
    else if(parseArg(cmd, "slowness_units", c)) {
	if(parseCompare(c, "s/km")) {
	    s_km->set(true, true);
	}
	else if(parseCompare(c, "s/deg")) {
	    s_deg->set(true, true);
	}
	else ret = ARGUMENT_ERROR;
    }
    else if(parseArg(cmd, "low", c)) {
	table->setField(0, 0, c, true);
    }
    else if(parseArg(cmd, "high", c)) {
	table->setField(0, 1, c, true);
    }
    else if(parseArg(cmd, "order", c)) {
	table->setField(0, 2, c, true);
    }
    else if(parseArg(cmd, "type", c)) {
	table->setField(0, 3, c, true);
    }
    else if(parseArg(cmd, "constraint", c)) {
	table->setField(0, 4, c, true);
    }
    else if(parseArg(cmd, "replace", c)) {
	ret = replace_toggle->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "beam")) {
	beam_toggle->set(true, true);
	makeBeam();
    }
    else if(parseCompare(cmd, "ftrace")) {
	ftrace_toggle->set(true, true);
	makeBeam();
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	ret = FormDialog::parseCmd(cmd, msg);
    }
    return ret;
}

void CreateBeam::parseHelp(const char *prefix)
{
    printf("\n");
    printf("%sazimuth=NUM\n", prefix);
    printf("%sslowness=NUM\n", prefix);
    printf("%sslowness_units=(s/km,s/deg)\n", prefix);
    printf("%sbeam_filter=(on,off)\n", prefix);
    printf("%slow=NUM\n", prefix);
    printf("%shigh=NUM\n", prefix);
    printf("%sorder=NUM\n", prefix);
    printf("%stype=(bp,br,lp,hp)\n", prefix);
    printf("%sconstraint=(causal,zero phase)\n", prefix);
    printf("%sreplace=(true,false)\n", prefix);
    printf("%sbeam\n", prefix);
    FormDialog::parseHelp(prefix);
}
