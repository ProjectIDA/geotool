/** \file Slowness.cpp
 *  \brief Defines class Slowness
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <algorithm>

using namespace std;

#include "Slowness.h"
#include "WaveformPlot.h"
#include "motif++/MotifClasses.h"
#include "widget/TabClass.h"
#include "MultiTable.h"
#include "widget/Table.h"
#include "Beam.h"
#include "TravelTime.h"
#include "gobject++/CssTables.h"
#include "libgmath.h"

extern "C" {
#include "libgmath.h"
static int sort_by_time(const void *A, const void *B);
}
static bool sort_phases(const char * a, const char * b);

using namespace libgbm;

Slowness::Slowness(const char *name, Component *parent, DataSource *ds)
		: FormDialog(name, parent, false)
{
    wp = ds->getWaveformPlotInstance();

    if( !wp ) {
	cerr << "Slowness: null WaveformPlot data source." << endl;
    }

    createInterface();
}

void Slowness::createInterface(void)
{
    int n;
    Arg args[20];

    // Input Slowness tab
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNheight, 250); n++;
    tab = new TabClass("tab", this, args, n);
    tab->addActionListener(this, XtNtabCallback);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    input_rc = new RowColumn("Input Slowness", tab, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    rc1 = new RowColumn("rc1", input_rc, args, n);

    label1 = new Label("slowness", rc1);
    label2 = new Label("azimuth (deg)", rc1);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    slowness_text1 = new TextField("slowness_text1", rc1, this, args, n);
    azimuth_text1 = new TextField("azimuth_text1", rc1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc2 = new RowColumn("rc2", input_rc, args, n);

    label3 = new Label("slowness units", rc2);
    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb = new RadioBox("Units", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    s_km = new Toggle("s/km", rb, args, n);
    s_km->set(true);
    s_deg = new Toggle("s/deg", rb, args, n);
    s_deg->set(false);

    // Origins tab
    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    origin_rc = new RowColumn("Origins", tab, args, n);
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
    RowColumn *rc = new RowColumn("rc", form, args, n);

    phase_label = new Label("Phase", rc);
    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("sw", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    phase_list = new List("phase_list", sw, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, rc->baseWidget()); n++;
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

    new Label("slowness (s/km)", rco);
    new Label("azimuth (deg)", rco);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    slowness_text2 = new TextField("slowness_text2", rco, this, args, n);
    azimuth_text2 = new TextField("azimuth_text2", rco, this, args, n);

    tab->setOnTop("Input Slowness");

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
    beam_button = new Button("Beam", controls, this);
    beam_button->setSensitive(false);
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
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc3 = new RowColumn("rc3", this, args, n);

    label4 = new Label("Beam Filter", rc3);
    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb2 = new RadioBox("Filter", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    filter_on = new Toggle("On", rb2, args, n);
    filter_on->set(true);
    filter_off = new Toggle("Off", rb2, args, n);
    filter_off->set(false);

    const char *column_labels[] = {"Low", "High", "Order", "Type","Constraint"};
    const char *column_choice[] = {"","","0:1:2:3:4:5:6:7:8:9:10","BP:BR:LP:HP",
                                "zero phase:causal"};
    const char *cells[] = {
        "2.0", "4.0", "3", "BP", "causal", (char *)NULL};

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc3->baseWidget()); n++;
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

    travel_time = NULL;

    if(wp) wp->addDataListener(this);
}

void Slowness::initPhaseList(void)
{
    vector<const char *> phases;

    if(wp) wp->getIaspeiPhases(phases);

    phases.push_back("LR");
    phases.push_back("LQ");
    phases.push_back("T");
    phases.push_back("I");
    sort(phases.begin(), phases.end(), sort_phases);

    for(int i = 0; i < (int)phases.size(); i++) {
	phase_list->addItem(phases[i]);
    }
}

void Slowness::initOriginTable(void)
{
    string prop;
    const char *extra[] = {"file", "file_index"};
    const char *extra_format[] = {"%s", "%d"};

    const char *names[] = {cssOrigin, cssOrigerr};

    if(getProperty("originAttributes", prop)) {
        origin_table->setType(2, names, 2, extra, extra_format, prop);
    }
    else {
        origin_table->setType(2, names, 2, extra, extra_format);
    }

    list();
}

Slowness::~Slowness(void)
{
}

void Slowness::actionPerformed(ActionEvent *action_event)
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
    else if(!strcmp(cmd, "Beam")) {
	makeBeam();
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	DataChange *c = (DataChange *)action_event->getCalldata();
	if(c->origin || c->origerr) {
	    list();
        }
	slowness_text2->setString("");
	azimuth_text2->setString("");
	computeSlowAz();
	setSensitive();
    }
    else if(!strcmp(cmd, "phase_list") | !strcmp(cmd, "origin_table")) {
	slowness_text2->setString("");
	azimuth_text2->setString("");
	computeSlowAz();
	setSensitive();
    }
/*
    else if(!strcmp(cmd, "Align Waveforms")) {
	alignWaveforms();
    }
*/
    else if(!strcmp(cmd, "Help")) {
	showHelp("Slowness Help");
    }
}

void Slowness::list(void)
{
    int i, j, l, n;
    int num_rows;
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;

    origin_table->removeAllRecords();

    if( !wp ) return;

    wp->getTable(origins);

    if(origins.size() == 0) {
	return;
    }
    wp->getTable(origerrs);

    origins.sort(sort_by_time);

    num_rows = 0;

    if(origins.size() > 0)
    {
        for(i = num_rows = 0; i < origins.size(); i++)
        {
            int dc = origins[i]->getDC();
            long orid = origins[i]->orid;
            for(j = n = 0; j < origerrs.size(); j++) {
                if(origerrs[j]->getDC() == dc && origerrs[j]->orid==orid) n++;
            }
            num_rows += (n > 0 ? n : 1);
        }
        for(i = 0; i < origins.size(); i++)
        {
            int dc = origins[i]->getDC();
            long orid = origins[i]->orid;
            for(l = n = 0; l < origerrs.size(); l++)
                if(origerrs[l]->getDC() == dc && origerrs[l]->orid == orid)
            {
                n++;
                origin_table->addRecord(new TableRow(origins[i],
                                origerrs[l]), false);
            }
            if(n == 0)
            {
                origin_table->addRecord(new TableRow(origins[i]), false);
            }
        }
    }

    origin_table->list();
}

bool Slowness::getAzimuth(double *azimuth)
{
    if( !strcmp(tab->labelOnTop(), "Input Slowness") ) {
	if(!azimuth_text1->getDouble(azimuth)) {
	    showWarning("Invalid azimuth");
	    return false;
	}
    }
    else {
	if(!azimuth_text2->getDouble(azimuth)) {
	    showWarning("Invalid azimuth");
	    return false;
	}
    }
    return true;
}

bool Slowness::getSlowness(double *slowness)
{
    if( !strcmp(tab->labelOnTop(), "Input Slowness") ) {
	if(!slowness_text1->getDouble(slowness)) {
	    showWarning("Invalid slowness");
	    return false;
	}
	if(s_deg->state()) {
	    // convert s from sec/deg to sec/km.
	    *slowness *= 180./(3.14159265*6371.);
	}
    }
    else {
	if(!slowness_text2->getDouble(slowness)) {
	    showWarning("Invalid slowness");
	    return false;
	}
    }
    return true;
}

void Slowness::computeSlowAz(void)
{
    char s[50], **phases;
    double lon, lat, delta, az, baz, slowness;
    BeamLocation beam_location = DNORTH_DEAST;
    gvector<Waveform *> wvec;

    gvector<TableRow *> v;
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

    int num = (wp) ? wp->getSelectedWaveforms(wvec) : 0;
    if( num < 2 ) {
	return;
    }
    DataSource *ds = wvec[0]->getDataSource();

    if((beam_location = Beam::getLocation(ds, beam_location, wvec,
			&lon, &lat)) == BEAM_LOCATION_ERROR)
    {
	showWarning(GError::getMessage());
	return;
    }

    deltaz(origin->lat, origin->lon, lat, lon, &delta, &az, &baz);

    snprintf(s, sizeof(s), "%.2f", baz);
    azimuth_text2->setString(s);

    if(!travel_time) {
	travel_time = new TravelTime();
    }
    string op;
    double tt = travel_time->getTravelTime(phases[0], origin, lat, lon,
			wvec[0]->elev(), wvec[0]->net(), wvec[0]->sta(),
			&slowness, op);
    if(tt < 0.)
    {
	showWarning("Cannot compute slowness for phase %s.", phases[0]);
	return;
    }
    // convert slowness from sec/deg to sec/km.
    slowness *= 180./(3.14159265*6371.);

    snprintf(s, sizeof(s), "%.4f", slowness);
    slowness_text2->setString(s);
}

bool Slowness::makeBeam(void)
{
    double azimuth, slowness;
    char type[3];
    bool replace;
    int order, zero_phase;
    double flo, fhi;
    BeamLocation beam_location = DNORTH_DEAST;
    WaveformPlot *dest = wp;

    if( !wp || !getAzimuth(&azimuth) || !getSlowness(&slowness) ) return false;

    if(filter_on->state()) {
	if( !getFilter(type, &order, &flo, &fhi, &zero_phase) ) return false;
    }
    else {
	strcpy(type, "NA");
    }

    replace = replace_toggle->state();

    wp->beamWaveforms(azimuth, slowness, order, type, flo, fhi, zero_phase,
		beam_location, replace, dest);
    return true;
}

bool Slowness::getFilter(char *type, int *order, double *flo, double *fhi,
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
	showWarning("Invalid filter type: %s", row[3]);
	return false;
    }
    if(!stringToDouble(row[0], flo) || *flo < 0.) {
	showWarning("Invalid Filter Low Frequency.");
	return false;
    }
    if(strcmp(type, "HP"))
    {
	if(!stringToDouble(row[1], fhi) || *fhi < 0.  || *fhi <= *flo)
	{
	    showWarning("Invalid Filter High Frequency.");
	    return false;
	}
    }
    if(!stringToInt(row[2], order) || *order < 1 || *order > 10) {
	showWarning("Invalid Filter Order");
	return false;
    }
    if(!strncasecmp(row[4], "zero", 4)) {
	*zero_phase = 1;
    }
    else if(!strncasecmp(row[4], "causal", 6)) {
	*zero_phase = 0;
    }
    else {
	showWarning("Invalid Filter Constraint");
	return false;
    }

    return true;
}

void Slowness::setSensitive(void)
{
    double az, slow;
    bool set;
    if( !wp ) return;

    if( !strcmp(tab->labelOnTop(), "Input Slowness") ) {
	set = (azimuth_text1->getDouble(&az) &&
			slowness_text1->getDouble(&slow)) ? true : false;
    }
    else {
	set = (azimuth_text2->getDouble(&az) &&
			slowness_text2->getDouble(&slow)) ? true : false;
    }
    if( wp->numSelected() < 2 ) set = false;

  
    beam_button->setSensitive(set);
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

static bool
sort_phases(const char * a, const char * b)
{
    return strcasecmp(a, b) > 0 ? true : false;
}
