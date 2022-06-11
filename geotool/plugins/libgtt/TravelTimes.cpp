/** \file TravelTimes.cpp
 *  \brief Defines class TravelTimes.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>
#include <errno.h>
#include <math.h>

#include "TravelTimes.h"
#include "motif++/MotifClasses.h"
#include "WaveformWindow.h"
#include "OridList.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "crust_def.h"
}
using namespace libgtt;

TravelTimes::TravelTimes(const char *name, Component *parent, DataSource *ds)
		: FormDialog(name, parent, false), DataReceiver(ds)
{
    int n;
    Arg args[20];

    ww = ds->getWaveformWindowInstance();
    wplot = ds->getWaveformPlotInstance();
    if(wplot) wplot->addDataListener(this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    form1 = new Form("form1", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNnumColumns, 1); n++;
    XtSetArg(args[n], XmNmarginHeight, 10); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    r1 = new RowColumn("r1", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    labels_toggle = new Toggle("Labels", r1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    curves_toggle = new Toggle("Curves", r1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    degrees_toggle = new Toggle("Degrees", r1, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    kilometers_toggle = new Toggle("Kilometers", r1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    scroll_toggle = new Toggle("Scroll Data", r1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, r1->baseWidget()); n++;
    orid_list = new OridList("orid list", form1, args, n, ds);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, orid_list->baseWidget()); n++;
    sep = new Separator("sep", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form2 = new Form("form2", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    rc_select = new RowColumn("rc_select", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    select_all_button = new Button("Select All", rc_select, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    deselect_all_button = new Button("Deselect All", rc_select, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc_select->baseWidget()); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    r2 = new RowColumn("r2", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    lab1 = new Label("IASPEI", r2, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw1 = new ScrolledWindow("sw1", r2, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmMULTIPLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 8); n++;
    iaspei_list = new List("iaspei_list", sw1, args, n, this);

/*
    char *iaspei_items[] = {
	"LR", "LQ", "P", "T", "PKPdf", "PKPab", "PKiKP", "pPKPdf", "pPKPab",
	"pP", "pPn", "pPb", "pPg", "sPKPdf", "sPKPab", "sP", "sPn", "sPb",
	"sPg", "pPKiKP", "sPKiKP", "PcP", "ScP", "SKPdf", "SKPab", "SKiKP",
	"PKKPdf", "PKKPab", "SKKPdf", "SKKPab", "P'P'df", "P'P'ab", "PP",
	"PnPn", "PbPb", "PgPg", "S", "SKSdf", "SKSac", "pSKSdf", "pSKSac", "pS",
	"sSKSdf", "sSKSac", "sS", "sSn", "sSb", "sSg", "ScS", "PcS", "PKSdf",
	"PKSab", "PKKSdf", "PKKSab", "SKKSdf", "SKKSac", "S'S'df", "S'S'ac",
	"SS", "SnSn", "SbSb", "SgSg", "SP",  "SPn", "SPg", "PS", "PnS", "PgS", 
	"Pdiff", "Sdiff",  NULL, 
    };

    int num;
    for(num = 0; iaspei_items[num] != NULL && num < 100; num++);
    iaspei_list->addItems(iaspei_items, num);
    iaspei_list->select("P");
    iaspei_list->select("S");
*/

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, r2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc_select->baseWidget()); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    r3 = new RowColumn("r3", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    lab2 = new Label("JB", r3, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    sw2 = new ScrolledWindow("sw2", r3, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmMULTIPLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 8); n++;
    jb_list = new List("jb_list", sw2, args, n, this);

    const char *jb_items[] = {
	"P", "PP", "S", "SS", "PcP", "pP", "sP", "ScS", "PKPab", "PKPbc",
	"PKPdf", "SKSac", "SKSdf"
    };
    jb_list->addItems(jb_items, 13);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    sep1 = new Separator("sep1", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNspacing, 10); n++;
    XtSetArg(args[n], XmNbottomWidget, rc_select->baseWidget()); n++;
    rc2 = new RowColumn("rc2", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    r4 = new RowColumn("r4", rc2, args, n);

    lab3 = new Label("Crust Models", r4);

    n = 0;
    XtSetArg(args[n], XmNheight, 120); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw3 = new ScrolledWindow("sw3", r4, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 10); n++;
    crust_list = new List("crust_list", sw3, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    r5 = new RowColumn("r5", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNforeground, stringToPixel("blue")); n++;
    lab4 = new Label("Regionals", r5, args, n);

    n = 0;
    XtSetArg(args[n], XmNheight, 120); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw4 = new ScrolledWindow("sw4", r5, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmMULTIPLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 10); n++;
    regional_list = new List("regional_list", sw4, args, n, this);

    const char *regional_items[] = {
        "Lg", "Rg", "Pn", "Sn", "Pg", "Sg", "P*", "S*", "Px", "Sx", "PIP", "PMP"
    };
    regional_list->addItems(regional_items, 12);

    n = 0;
    XtSetArg(args[n], XmNrows, 7); n++;
    XtSetArg(args[n], XmNskipAdjust, True); n++;
    XtSetArg(args[n], XmNwordWrap, False); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, False); n++;
    crust_text = new TextField("text", rc, args, n);

    sep2 = new Separator("sep2", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    reduced = new RowColumn("reduced", rc, args, n);

    reduced_time = new Toggle("Reduced Time Velocity (km/s)", reduced, this,
				args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    velocity_text = new TextField("velocity", reduced, this, args, n);
    reduced_apply = new Button("Apply", reduced, this);

    reduced_time->setSensitive(false);
    velocity_text->setSensitive(false);
    reduced_apply->setSensitive(false);

    sep3 = new Separator("sep3", rc);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    depth_rc = new RowColumn("depth_rc", rc, args, n);
    depth_rc->setSensitive(false);

    lab5 = new Label("Depth", depth_rc);
    depth_apply = new Button("Apply", depth_rc, this);
    depth_apply->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    depth_text = new TextField("depth_text", depth_rc, this, args, n);
    depth_text->setString("0.");
    depthApply();

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    XtSetArg(args[n], XmNscaleHeight, 20); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    depth_scale = new Scale("depth_scale", depth_rc, this, args, n);

    sep4 = new Separator("sep4", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", rc, args, n);
    close_button = new Button("Close", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    save_selected = true;
    ln_max = 4.;
    scale_depth_max = 800.;

    initPhaseSelections();

    crustInit();

    Application::addPropertyListener(this);
}

TravelTimes::~TravelTimes(void)
{
}

void TravelTimes::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
        if(data_source) {
            data_source->removeDataListener(this);
	    data_source->removeDataReceiver(this);
        }
        data_source = ds;
	ww = NULL;
	wplot = NULL;
        if(data_source) {
	    data_source->addDataReceiver(this);
	    data_source->addDataListener(this);
	    ww = data_source->getWaveformWindowInstance();
	    wplot = data_source->getWaveformPlotInstance();
        }
    }
}

void TravelTimes::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Arg args[10];

    if(comp == labels_toggle) {
	if(wplot) {
	    XtSetArg(args[0], XtNdisplayTtLabels, labels_toggle->state());
	    wplot->setValues(args, 1);
	}
    }
    else if(comp == curves_toggle) {
	curves(curves_toggle->state());
    }
    else if(comp == iaspei_list || comp == jb_list || comp == regional_list)
    {
	selectPhase();
    }
    else if(comp == crust_list) {
	crustSelect();
    }
    else if(comp == depth_scale) {
	depthScale();
    }
    else if(comp == depth_text) {
	depthText();
    }
    else if(comp == depth_apply) {
	depthApply();
    }
    else if(comp == close_button) {
	setVisible(false);
    }
    else if(comp == select_all_button) {
	selectAllPhases();
    }
    else if(comp == deselect_all_button) {
	deselectAllPhases();
    }
    else if(comp == scroll_toggle) {
	bool set = scroll_toggle->state();
	XtSetArg(args[0], XtNscrollData, (Boolean)set);
	if(wplot) wplot->setValues(args, 1);
    }
    else if(!strcmp(cmd, "Degrees")) {
	if(degrees_toggle->state()) {
	    kilometers_toggle->set(false);
	    XtSetArg(args[0], XtNdistanceUnits, DEGREES);
	    if(wplot) wplot->setValues(args, 1);
	}
	else {
	    kilometers_toggle->set(true, true);
	}
    }
    else if(!strcmp(cmd, "Kilometers")) {
	if(kilometers_toggle->state()) {
	    degrees_toggle->set(false);
	    XtSetArg(args[0], XtNdistanceUnits, KILOMETERS);
	    if(wplot) wplot->setValues(args, 1);
	}
	else {
	    degrees_toggle->set(true, true);
	}
    }
    else if(comp == reduced_time) {
	bool on = reduced_time->state();
	velocity_text->setSensitive(on);
	reduced_apply->setSensitive(on);
	if(on) {
	    double reduction_velocity;
	    if(velocity_text->getDouble(&reduction_velocity)) {
		reducedTime(reduction_velocity);
	    }
	}
	else {
	    XtSetArg(args[0], XtNreducedTime, False);
	    if(wplot) wplot->setValues(args, 1);
	}
    }
    else if(comp == velocity_text) {
	double d;
	reduced_apply->setSensitive(velocity_text->getDouble(&d));
    }
    else if(comp == reduced_apply) {
	double reduction_velocity;
	if(velocity_text->getDouble(&reduction_velocity)) {
	    reducedTime(reduction_velocity);
	}
    }
    else if(comp == help_button) {
	showHelp("Travel Times Help");
    }
    else if(!strcmp(action_event->getReason(), XtNchangePropertyCallback)) {
	if(!strcmp((char *)action_event->getCalldata(), "crustModels")) {
	    loadCrustModels();
	}
    }
}

ParseCmd TravelTimes::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "Select_All")) {
	select_all_button->activate();
    }
    else if(parseCompare(cmd, "Deselect_All")) {
	deselect_all_button->activate();
    }
    else if(parseArg(cmd, "Labels", c)) {
	return labels_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Curves", c)) {
	return curves_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Units", c)) {
	if(parseCompare(c, "degrees")) {
	    degrees_toggle->set(true);
	    kilometers_toggle->set(false);
	}
	else if(parseCompare(c, "kilometers")) {
	    kilometers_toggle->set(true);
	    degrees_toggle->set(false);
	}
	else return COMMAND_NOT_FOUND;
    }
    else if(parseArg(cmd, "Iaspei", c)) {
	return iaspei_list->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "JB", c)) {
	return jb_list->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Crust", c)) {
	return crust_list->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Regional", c)) {
	return regional_list->parseCmd(c, msg);
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

void TravelTimes::parseHelp(const char *prefix)
{
    printf("%sselect_all\n", prefix);
    printf("%sdeselect_all\n", prefix);

    printf("%slabels=(true,false)\n", prefix);
    printf("%scurves=(true,false)\n", prefix);
    printf("%sunits=(degrees,kilometers)\n", prefix);
    char p[200];
    snprintf(p, sizeof(p), "%siaspei.", prefix);
    iaspei_list->parseHelp(p);
    snprintf(p, sizeof(p), "%sjb.", prefix);
    jb_list->parseHelp(p);
    snprintf(p, sizeof(p), "%scrust.", prefix);
    crust_list->parseHelp(p);
    snprintf(p, sizeof(p), "%sregional.", prefix);
    regional_list->parseHelp(p);
}

void TravelTimes::curves(bool on)
{
    if(on != curves_toggle->state()) {
	curves_toggle->set(on);
    }
    if(ww) {
	ww->travelTimeCurves(on);
	reduced_time->setSensitive(on);
	if(reduced_time->state()) {
	    velocity_text->setSensitive(on);
	    reduced_apply->setSensitive(on);
	}
	depth_rc->setSensitive(on);
    }
}

void TravelTimes::selectPhase(void)
{
    /*
     * get selected phase names
     */
    int total_selected = 0;
    char **selected, *selected_phases[100], label[50];
    int num;

    num = iaspei_list->getSelectedItems(&selected);
    for(int i = 0; i < num; i++) {
	selected_phases[total_selected++] = selected[i];
    }
    Free(selected);

    num = jb_list->getSelectedItems(&selected);
    for(int i = 0; i < num; i++) {
	snprintf(label, sizeof(label), "J%s", selected[i]);
	Free(selected[i]);
	selected_phases[total_selected++] = strdup(label);
    }
    Free(selected);

    num = regional_list->getSelectedItems(&selected);
    for(int i = 0; i < num; i++) {
	if(!strcmp(selected[i], "Lg")) {
	    selected_phases[total_selected++] = selected[i];
	}
	else {
	    snprintf(label, sizeof(label), "R%s", selected[i]);
	    Free(selected[i]);
	    selected_phases[total_selected++] = strdup(label);
	}
    }
    Free(selected);

    selected_phases[total_selected] = NULL;

/*
    curves = XmToggleButtonGetState(w);
    g->tt_curves_on = (curves && total_selected > 0);
*/
    Arg args[1];
    XtSetArg(args[0], XtNselectedPhases, selected_phases);
    if(wplot) wplot->setValues(args, 1);

    saveSelectedPhases(total_selected, selected_phases);

    for(int i = 0; i < total_selected; i++) {
	Free(selected_phases[i]);
    }
}

void TravelTimes::saveSelectedPhases(int total_selected, char **selected_phases)
{
    int i, len;
    char *prop = NULL;

    if(!save_selected || total_selected <= 0) return;

    len = 0;
    for(i = 0; i < total_selected; i++) {
	len += strlen(selected_phases[i]) + 1;
    }
    if(!(prop = (char *)mallocWarn(len+1))) return;

    prop[0] = '\0';
    for(i = 0; i < total_selected; i++) {
	strcat(prop, selected_phases[i]);
	if(i < total_selected-1) strcat(prop, ",");
    }
    putProperty("Travel_Time_Phases", prop);
    Application::writeApplicationProperties();
    Free(prop);
}

void TravelTimes::crustInit(void)
{
    num_crusts = NUM_DEFAULT_CRUSTS;
    crust_models = (CrustModel *)mallocWarn(num_crusts * sizeof(CrustModel));
    for(int i = 0; i < num_crusts; i++) {
	crust_models[i] = default_crusts[i];
    }

    loadCrustModels();
}

void TravelTimes::loadCrustModels(void)
{
    string crust_file;
    CrustModel *cm = NULL;
    int i, num;

    if( getProperty("crustModels", crust_file) ) 
    {
	FILE *fp;
	if((fp = fopen(crust_file.c_str(), "r")) == NULL)
	{
	    if(errno > 0) {
		fprintf(stderr, "Cannot open: %s\n%s\n",
			crust_file.c_str(), strerror(errno));
	    }
	    else {
		fprintf(stderr, "Cannot open: %s\n", crust_file.c_str());
	    }
	}
	else
	{
	    readCrustModels(fp, &cm, &num);
	    fclose(fp);

	    if(num <= 0) {
		fprintf(stderr, "No crust models found in %s\n",
			crust_file.c_str());
	    }
	    else {
		Free(crust_models);
		crust_models = cm;
		num_crusts = num;
	    }
	}
    }

    crust_list->deselectAll();
    crust_list->deleteAll();
    for(i = 0; i < num_crusts; i++) {
	crust_list->addItem(crust_models[i].full_name);
    }
    crust_list->selectPos(1, true);
}

extern "C" {
static int
sort_phases(const void *A, const void *B)
{
    char *a = *(char **)A;
    char *b = *(char **)B;
    return strcasecmp(a, b);
}
};

void TravelTimes::initPhaseSelections(void)
{
    if(!wplot) return;
    save_selected = false;

    vector<const char *> phase_list;
    int num = wplot->getIaspeiPhases(phase_list);
    const char **phases = (const char **)mallocWarn((num+4)*sizeof(char *));

    for(int i = 0; i < num; i++) {
	phases[i] = phase_list[i];
    }

    phases[num] = "LR";
    phases[num+1] = "LQ";
    phases[num+2] = "T";
    phases[num+3] = "I";
    num += 4;
    qsort(phases, num, sizeof(char *), sort_phases);

    for(int i = 0; i < num; i++) {
	iaspei_list->addItem(phases[i]);
    }
    Free(phases);

    selectPhases(iaspei_list);
    selectPhases(regional_list);
    selectPhases(jb_list);

    save_selected = True;
}

void TravelTimes::selectPhases(List *list)
{
    char *prop=NULL, **items=NULL;

    int num = list->getItems(&items);

    if( (prop = getProperty("Travel_Time_Phases")) )
    {
	int j = (list == regional_list || list == jb_list) ? 1 : 0;

	char *phase, *last, *tok = prop;
	while((phase = strtok_r(tok, ", ", &last)) != NULL)
	{
	    tok = NULL;

	    if( (list == regional_list && phase[0] != 'R') ||
		(list == jb_list && phase[0] != 'J')) continue;

	    for(int i = 0; i < num; i++)
	    {
		if(!strcmp(items[i], phase+j))
		{
		    list->selectPos(i+1, true);
		}
	    }
	}
	Free(prop);
    }
    else if(list == iaspei_list)
    {
	/* always select P and S */
	for(int i = 0; i < num; i++)
	{
	    if(!strcmp(items[i], "P") || !strcmp(items[i], "S"))
	    {
		list->selectPos(i+1, true);
	    }
	}
    }

    for(int i = 0; i < num; i++) Free(items[i]);
    Free(items);
}

void TravelTimes::crustSelect(void)
{
    int		num;
    char	text[4096];
    char	**selected=NULL;
    CrustModel	cm;

    if((num = crust_list->getSelectedItems(&selected)) <= 0) return;

    if(!crust_model(num_crusts, crust_models, selected[0], &cm))
    {
        snprintf(text, sizeof(text),
"Model = %s\n\n\
                          Upper Crust     Lower Crust     Mantle\n\
Thickness (km)          %3.1f              %3.1f               --\n\
P velocity (km/s)       %3.2f              %3.2f              %3.2f\n\
S velocity (km/s)       %3.2f              %3.2f              %3.2f",
	    cm.name, cm.h[0], cm.h[1], cm.vp[0], cm.vp[1], cm.vp[2],
	    cm.vs[0], cm.vs[1], cm.vs[2]);

	crust_text->setString(text);

	if(wplot) wplot->crust(&cm);
    }
    Free(selected);
}

void TravelTimes::depthScale(void)
{
    int n, val, max;
    double depth, ln;
    char text[50];
    Arg args[2];

    n = 0;
    XtSetArg(args[n], XmNvalue, &val); n++;
    XtSetArg(args[n], XmNmaximum, &max); n++;
    depth_scale->getValues(args, n);

    ln = val*ln_max/(float)max;
    depth = scale_depth_max*(exp(ln)-1.)/(exp(ln_max)-1.);
    snprintf(text, sizeof(text), "%d", (int)(depth+.5));
    depth_text->setString(text, true);
}

void TravelTimes::depthApply(void)
{
    double depth;

    if(!depth_text->getDouble(&depth)) {
	showWarning("Invalid depth: %s", depth_text->getString());
	return;
    }
    if(wplot) wplot->setSourceDepth(depth);
}

void TravelTimes::depthText(void)
{
    double depth;

    if(!depth_text->getDouble(&depth)) {
	depth_apply->setSensitive(false);
	return;
    }

    double current_depth = (wplot) ? wplot->getSourceDepth() : 0.;

    if(depth != current_depth) {
	depth_apply->setSensitive(true);
    }
    else {
	depth_apply->setSensitive(false);
    }
}

void TravelTimes::selectAllPhases(void)
{
    iaspei_list->deselectAll();
    jb_list->deselectAll();
    regional_list->deselectAll();

    iaspei_list->selectAll();
    regional_list->selectAll();

    selectPhase();
}

void TravelTimes::deselectAllPhases(void)
{
    iaspei_list->deselectAll();
    jb_list->deselectAll();
    regional_list->deselectAll();

    selectPhase();
}

void TravelTimes::reducedTime(double reduction_velocity)
{

    if(reduction_velocity == 0.) {
	Arg args[1];
	XtSetArg(args[0], XtNreducedTime, False);
	if(wplot) wplot->setValues(args, 1);
    }
    else
    {
	char xlabel[50], v[30];
	int lg, ndeci;
	lg = (int)log10(fabs(reduction_velocity));
	ndeci = (lg-3 > 0) ? 0 : -lg+3;
	ftoa(reduction_velocity, ndeci, 0, v, 30);
	snprintf(xlabel, sizeof(xlabel), "t - r/%s", v);

	if(wplot) wplot->setReductionVelocity(reduction_velocity, true);
    }
}
