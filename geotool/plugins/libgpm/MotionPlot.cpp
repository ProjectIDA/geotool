/** \file MotionPlot.cpp
 *  \brief Defines class MotionPlot.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "MotionPlot.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GTimeSeries.h"
#include "BasicMap.h"
#include "RotateData.h"

extern "C" {
#include "libgmath.h"
}

using namespace libgpm;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int mp_id = 0;

MotionPlot::MotionPlot(const char *name, Component *parent, DataSource *ds) :
		FormDialog(name, parent), DataReceiver(ds)
{
    if(data_source) {
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;

    createInterface();

    map = Application::getMap(this);
    if(map) {
	map->addActionListener(this, XtNsetVisibleCallback);
    }
    else {
	Application::addMapListener(this, getParent());
    }

    warned = false;
    id = mp_id++; // unique id for each MotionPlot instance to use with the Map.
    max_azimuth = 0.;
    max_incidence = 0.;
}

void MotionPlot::createInterface()
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    save_button = new Button("Save", controls, this);
    input_choice = new Choice("input", controls, this);
    input_choice->addItem("'a' cursor");
    input_choice->addItem("'b' cursor");
    input_choice->addItem("'c' cursor");
    input_choice->addItem("'d' cursor");
 
    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    long_arrow_toggle = new Toggle("Long Arrows", controls, this, args, n);
    map_toggle = new Toggle("Map", controls, this, args, n);
    new_pm_button = new Button("New PM Window...", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNisHomogeneous, False); n++;
    dial_rc = new RowColumn("dial_rc", this, args, n);

    label = new Label("Station to Source\nAzimuth", dial_rc);

    form = new Form("form", dial_rc);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    W_label = new Label("W", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    E_label = new Label("E", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, W_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, E_label->baseWidget()); n++;
    N_label = new Label("N", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, W_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, E_label->baseWidget()); n++;
    S_label = new Label("S", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, N_label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, S_label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, W_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, E_label->baseWidget()); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNmarkers, 24); n++;
    XtSetArg(args[n], XtNindicatorColor, stringToPixel("Red")); n++;
    XtSetArg(args[n], XtNarrow1Color, stringToPixel("Blue")); n++;
    XtSetArg(args[n], XtNarrow2Color, stringToPixel("Black")); n++;
    dial = new DialClass("dial", form, args, n);
    dial->addActionListener(this, XtNselect);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    rc = new RowColumn("rc", dial_rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNforeground, stringToPixel("Red")); n++;
    azimuth_label = new Label("Azimuth", rc, args, n);
    incidence_label = new Label("Incidence", rc);
    phase_choice = new Choice("phase_choice", rc, this);
    phase_choice->addItem("P-dist");
    phase_choice->addItem("S-dist");
    origin_button = new Button("Origin Az", rc, this);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    azimuth_text = new TextField("Azimuth", rc, this, args, n);
    azimuth_text->setString("0.0");

    incidence_text = new TextField("Incidence", rc, this, args, n);
    incidence_text->setString("90.0");

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    distance_text = new TextField("distance", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    maximize_button = new Button("Find Direction", rc, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, dial_rc->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_BOTTOM); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNsliderSize, 100); n++;
    XtSetArg(args[n], XmNincrement, 5); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    scrollbar = new ScrollBar("scrollbar", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, scrollbar->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("White")); n++;
    XtSetArg(args[n], XmNwidth, 500); n++;
    XtSetArg(args[n], XmNheight, 250); n++;
    pm = new ParticleMotionClass("pm", this, args, n);

    remove_cursor = false;

    iaspei = new Iaspei();

    getRayParameters(0.);

//    print_window = NULL;
}

void MotionPlot::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, XtNdoubleLineCallback);
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp) {
		wp->addActionListener(this, XtNdoubleLineCallback);
		wp->addActionListener(this, XtNdoubleLineDragCallback);
		wp->addActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
	else wp = NULL;
    }
}

MotionPlot::~MotionPlot(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void MotionPlot::setVisible(bool visible)
{
    FormDialog::setVisible(visible);
    if(visible) {
	displayDataCursors();
    }
    else {
	removeDataCursors();
    }
}

void MotionPlot::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(comp == dial) {
	dialAction((DialCallbackStruct *)action_event->getCalldata());
    }
    else if(comp == scrollbar) {
	scrollAction((XmScrollBarCallbackStruct *)action_event->getCalldata());
    }
    else if(comp == wp) {
	doubleLineCB((AxesCursorCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Azimuth")) {
	setAzimuth();
    }
    else if(!strcmp(cmd, "Incidence") || !strcmp(cmd, "phase_choice"))
    {
	setIncidence();
    }
    else if(!strcmp(cmd, "Origin Az")) {
	setOrigin();
    }
    else if(!strcmp(cmd, "Long Arrows")) {
	setArrowLength();
    }
    else if(!strcmp(cmd, "New PM Window...")) {
	newWindow();
    }
    else if(!strcmp(reason, XtNmapCallback)) {
	if(!map) {
	    map = (BasicMap *)comp;
	    map->addActionListener(this, XtNsetVisibleCallback);
	    drawOnMap(0);
	}
    }
    else if(!strcmp(reason, XtNsetVisibleCallback)) {
	if(comp == map && map->isVisible()) {
	    drawOnMap(0);
	}
    }
    else if(!strcmp(cmd, "Map")) {
	if(map) {
	    map->setVisible(true);
	    drawOnMap(0);
	}
	else if(wp) {
	    Button *b;
	    TopWindow *tp = wp->topWindowParent();
	    if(tp && (b = tp->findButton("Map")) ) {
		b->activate();
	    }
	}
    }
    else if(!strcmp(cmd, "Find Direction")) {
	rotateMax();
    }
    else if(!strcmp(cmd, "Save")) {
	saveAzInc();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Particle Motion Help");
    }
}

ParseCmd MotionPlot::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "Long_Arrows", c)) {
	return long_arrow_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Map", c)) {
	return map_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "azimuth", c)) {
	azimuth_text->setString(c,  true);
    }
    else if(parseArg(cmd, "incidence", c)) {
	incidence_text->setString(c,  true);
    }
    else if(parseCompare(cmd, "P-dist")) {
	phase_choice->setChoice("P-dist");
    }
    else if(parseCompare(cmd, "S-dist")) {
	phase_choice->setChoice("S-dist");
    }
    else if(parseCompare(cmd, "Origin_Az")) {
	origin_button->activate();
    }
    else if(parseCompare(cmd, "Find_Direction")) {
	maximize_button->activate();
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

void MotionPlot::parseHelp(const char *prefix)
{
    printf("%sazimuth=DEGREES\n", prefix);
    printf("%sincidence=DEGREES\n", prefix);
    printf("%sp-dist\n", prefix);
    printf("%ss-dist\n", prefix);
    printf("%sorigin_az\n", prefix);
    printf("%sfind_direction\n", prefix);
    printf("%slong_arrows=(true,false\n", prefix);
    printf("%smap=(true,false\n", prefix);
}

void MotionPlot::displayDataCursors(void)
{
    double xmin, xmax, x1, x2;

    if(!wp) return;

    wp->addActionListener(this, XtNdoubleLineCallback);
    wp->addActionListener(this, XtNdoubleLineDragCallback);
    wp->addActionListener(this, XtNdoubleLineScaleCallback);

    wp->getXLimits(&xmin, &xmax);

    const char *input = input_choice->getChoice();
    char cursor[2];
    cursor[0] = input[1];
    cursor[1] = '\0';

    x1 = xmin + (xmax-xmin)/7.;
    
    switch(cursor[0]) {
	case 'a':
	    x1 = xmin + (xmax-xmin)/7.;
	    break;
	case 'b':
	    x1 = xmin + 2*(xmax-xmin)/7.;
	    break;
	case 'c':
	    x1 = xmin + 3*(xmax-xmin)/7.;
	    break;
	case 'd':
	    x1 = xmin + 4*(xmax-xmin)/7.;
	    break;
    }
    if( !wp->doubleLineIsDisplayed(cursor) ) {
	remove_cursor = true;
	x2 = x1 + (xmax-xmin)/40.;
	if(x2 - x1 > 10.) x2 = x1 + 10.;
	wp->positionDoubleLine(cursor, x1, x2, false);
    }
    else {
	remove_cursor = false;
    }
}

void MotionPlot::removeDataCursors(void)
{
    if(!wp) return;

    wp->removeActionListener(this, XtNdoubleLineCallback);
    wp->removeActionListener(this, XtNdoubleLineDragCallback);
    wp->removeActionListener(this, XtNdoubleLineScaleCallback);

    if(remove_cursor) {
	string input_cursor;
	input_choice->getChoice(input_cursor);
	wp->deleteDoubleLine(input_cursor);
    }
}

/*
void MotionPlot::print(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print Particle Motion", this, this);
    }
    print_window->setVisible(true);
}
*/
void MotionPlot::dialAction(DialCallbackStruct *s)
{
    double az = s->position;

    dial->setPosition(s->position);

    azimuth_text->setString("%.1f", az);

    pm->setAzimuth(az);
    drawOnMap(0);
}

void MotionPlot::newWindow(void)
{
    MotionPlot *mp = NULL;

    for(int i = 0; i < windows.size(); i++) {
	if( !windows[i]->isVisible() ) {
	    mp = windows[i];
	    break;
	}
    }
    if(!mp) {
	mp = new MotionPlot(getName(), this, data_source);
	windows.add(mp);
    }
    const char *c = input_choice->getChoice();
    switch(c[1])
    {
	case 'a': mp->input_choice->setChoice("'b' cursor"); break;
	case 'b': mp->input_choice->setChoice("'c' cursor"); break;
	case 'c': mp->input_choice->setChoice("'d' cursor"); break;
	case 'd': mp->input_choice->setChoice("'a' cursor"); break;
    }
    mp->setVisible(true);
}

void MotionPlot::getRayParameters(double depth)
{
    int i, n;

    for(i = 0; i < 500; i++) {
	p_ray_p[i] = -1.;
	s_ray_p[i] = -1.;
    }

    n = 0;
    iaspei->getCurve("Pn", depth, &n, p_tt, p_dist, p_ray_p);
    for(i = 0; i < n; i++) {
	if(p_dist[i] > 20.) break;
    }
    n = i;
    iaspei->getCurve("P", depth, &p_npts, p_tt+n, p_dist+n, p_ray_p+n);
    p_npts += n;
   
    n = 0;
    iaspei->getCurve("Sn", depth, &n, s_tt, s_dist, s_ray_p);
    for(i = 0; i < n; i++) {
	if(s_dist[i] > 20.) break;
    }
    n = i;
    iaspei->getCurve("S", depth, &s_npts, s_tt+n, s_dist+n, s_ray_p+n);
    s_npts += n;
}

void MotionPlot::computeDistance(double incidence)
{
    double diff, dist, dmin, rayp;

    // find the ray parameters closest to this incidence
    dist = -1.;
    if(!strcmp(phase_choice->getChoice(), "P-dist"))
    {
	rayp = 6371.*sin(incidence*PI/180.)/5.80; 
	if(p_npts > 0) {
	    dmin = fabs(rayp - p_ray_p[0]);
	    dist = p_dist[0];
	}
	for(int i = 1; i < p_npts; i++) {
	    // isop_c get_seg() uses set_fnan to denote a break in the curve
//	    if(!fNaN(p_dist[i])) {
	    if(p_ray_p[i] >= 0.) {
		diff = fabs(rayp - p_ray_p[i]);
		if(dmin > diff) {
		    dmin = diff;
		    dist = p_dist[i];
		}
	    }
	}
    }
    else
    {
	rayp = 6371.*sin(incidence*PI/180.)/3.36;
	if(s_npts > 0) {
	    dmin = fabs(rayp - s_ray_p[0]);
	    dist = s_dist[0];
	}
	for(int i = 1; i < s_npts; i++) {
	    // isop_c get_seg() uses set_fnan to denote a break in the curve
//	    if(!fNaN(s_dist[i])) {
	    if(s_ray_p[i] >= 0.) {
		diff = fabs(rayp - s_ray_p[i]);
		if(dmin > diff) {
		    dmin = diff;
		    dist = s_dist[i];
		}
	    }
	}
    }
    if(dist > 0.) {
	distance_text->setString("%.2f", dist);
    }
    else {
	distance_text->setString("");
    }
    drawOnMap(0);
}

void MotionPlot::setAzimuth(void)
{
    double az;

    if(azimuth_text->getDouble(&az)) {
	dial->setPosition((int)az);
	pm->setAzimuth(az);
	drawOnMap(0);
    }
}

void MotionPlot::setIncidence(void)
{
    Arg args[3];
    int min, max, slider_size, value;
    double incidence;

    if(incidence_text->getDouble(&incidence)) {
	if(incidence < 0.) {
	    incidence_text->setString("0.");
	    incidence = 0.;
	}
	else if(incidence > 90.) {
	    incidence_text->setString("90.");
	    incidence = 90.;
	}
	XtSetArg(args[0], XmNminimum, &min);
	XtSetArg(args[1], XmNmaximum, &max);
	XtSetArg(args[2], XmNsliderSize, &slider_size);
	scrollbar->getValues(args, 3);

	value = 2*(int)((90.-incidence)*(max-slider_size-min)/180.) + min;
	scrollbar->setValue(value);

	pm->setIncidence(incidence);

	computeDistance(incidence);
    }
}

void MotionPlot::setOrigin(void)
{
    int ngroups;
    vector<int> ncmpts;
    double delta, az, baz;
    Arg args[3];
    CssOriginClass *origin;
    gvector<Waveform *> wvec;

    if(!wp) return;

    ngroups = wp->getSelectedComponents("a", ncmpts, wvec);
    if(ngroups == 0) {
	return;
    }

    if(ngroups != 1 || ncmpts[0] != 3) {
	showWarning("Select three components.");
	return;
    }

    if((origin = wp->getPrimaryOrigin(wvec[0])) != NULL
	    && origin->lat > -900. && origin->lon > -900.
	    && wvec[0]->lat() > -900. && wvec[0]->lon() > -900.)
    {
	deltaz(origin->lat, origin->lon, wvec[0]->lat(), wvec[0]->lon(),
			&delta, &az, &baz);
	if(baz < -998)
	{
	    XtSetArg(args[0], XtNarrow2Visible, False);
	    dial->setValues(args, 1);
	    return;
	}
	if(baz < 0) {
		int i = (int)(-baz/360) + 1;
		baz += i*360.;
	}
	else if(baz > 360.) {
		int i = (int)(baz/360);
		baz -= i*360.;
	}

	XtSetArg(args[0], XtNarrow2Visible, True);
	XtSetArg(args[1], XtNarrow2Position, (int)baz);
	XtSetArg(args[2], XtNposition, (int)baz);
	dial->setValues(args, 3);

	azimuth_text->setString("%.1f", baz);

	pm->setAzimuth(baz);
	drawOnMap(0);
    }
    else
    {
	XtSetArg(args[0], XtNarrow2Visible, FALSE);
	dial->setValues(args, 1);
    }
}

void MotionPlot::scrollAction(XmScrollBarCallbackStruct *s)
{
    int min, max, slider_size;
    Arg args[3];
    double incidence;

    XtSetArg(args[0], XmNminimum, &min);
    XtSetArg(args[1], XmNmaximum, &max);
    XtSetArg(args[2], XmNsliderSize, &slider_size);
    scrollbar->getValues(args, 3);

    incidence = (double)(s->value-min)*180./(max-slider_size-min);
    incidence = ((int)(incidence+.5))*.5;
    if(incidence < 0.) incidence = 0.;
    if(incidence > 90.) incidence = 90.;
    incidence = 90. - incidence;
    incidence_text->setString("%.1f", incidence);

    pm->setIncidence(incidence);
    computeDistance(incidence);
}

void MotionPlot::doubleLineCB(AxesCursorCallbackStruct *c)
{
    int	k, npts, ngroups;
    vector<int> ncmpts;
    float *e, *n, *z;
    double e_mean, n_mean, z_mean, alpha, beta, gamma;
    char cursor[2];
    GDataPoint *d1, *d2, *d3;
    gvector<Waveform *> wvec;

    if(!isVisible() || c->type != AXES_VER_DOUBLE_LINE) return;

    cursor[0] = *(input_choice->getChoice()+1);
    cursor[1] = '\0';
    if(cursor[0] != c->label[0]) return;

    ngroups = wp->getSelectedComponents(cursor, ncmpts, wvec);

    if(ngroups != 1 || ncmpts[0] != 3) {
	if(!warned) {
	  putWarning("Select three components to see particle motion display.");
	  warned = true;
	}
	return;
    }

    for(k = 0; k < wvec[0]->num_dw; k++) {
	if(wvec[0]->dw[k].label ==  c->label[0]) break;
    }
    if(k == wvec[0]->num_dw) return;

    d1 = wvec[0]->dw[k].d1;
    d2 = wvec[0]->dw[k].d2;
    if(d1->segmentIndex() != d2->segmentIndex()) {
	return;
    }

    npts = d2->index() - d1->index() + 1;

    for(int i = 0; i < 3; i++)
    {
	d1 = wvec[i]->dw[k].d1;
	d2 = wvec[i]->dw[k].d2;

	if(d1->segmentIndex() != d2->segmentIndex() ||
		d2->index() - d1->index()+1 != npts)
	{
	    return;
	}
    }
	
    d1 = wvec[0]->dw[k].d1;
    d2 = wvec[1]->dw[k].d1;
    d3 = wvec[2]->dw[k].d1;

    e = d1->segment()->data + d1->index();
    n = d2->segment()->data + d2->index();
    z = d3->segment()->data + d3->index();

    alpha = wvec[0]->currentAlpha();
    beta = wvec[0]->currentBeta();
    gamma = wvec[0]->currentGamma();

    if(alpha != 0. || beta != 0. || gamma != 0.) {
	unrotate(npts, &e, &n, &z, alpha, beta, gamma);
    }

    /* compute means
     */
    e_mean = d1->timeSeries()->mean();
    n_mean = d2->timeSeries()->mean();
    z_mean = d3->timeSeries()->mean();

    getRectilinearity(npts, e, e_mean, n, n_mean, z, z_mean);

    pm->input(npts, e, e_mean, n, n_mean, z, z_mean, wvec[0]->sta());

    if(alpha != 0. || beta != 0. || gamma != 0.) {
	Free(n);
	Free(e);
	Free(z);
    }
}

void MotionPlot::unrotate(int npts, float **e, float **n, float **z,
		double alpha, double beta, double gamma)
{
    float *pe, *pn, *pz;

    if( !(pe = (float *)mallocWarn(npts*sizeof(float))) ||
        !(pn = (float *)mallocWarn(npts*sizeof(float))) ||
        !(pz = (float *)mallocWarn(npts*sizeof(float)))) return;

    memcpy(pe, *e, npts*sizeof(float));
    memcpy(pn, *n, npts*sizeof(float));
    memcpy(pz, *z, npts*sizeof(float));

    *e = pe;
    *n = pn;
    *z = pz;

    RotateData::rotate(pe, pn, pz, npts, alpha, beta, gamma, 0., 0., 0.);
}

void MotionPlot::getRectilinearity(int npts, float *e, double e_mean, float *n,
			double n_mean, float *z, double z_mean)
{
    int i;
    double x, scale;
    double s[9], d[3], o[3], v[9];

    scale = 0.;
    for(i = 0; i < npts; i++)
    {
	e[i] -= e_mean;
	n[i] -= n_mean;
	z[i] -= z_mean;

        x = fabs(e[i]);
        if(x < scale) scale = x;
        x = fabs(n[i]);
        if(x < scale) scale = x;
        x = fabs(z[i]);
        if(x < scale) scale = x;
    }
    if(scale == 0.) scale = 1.;
    scale = 1./scale;
    for(i = 0; i < npts; i++) {
        e[i] *= scale;
        n[i] *= scale;
        z[i] *= scale;
    }

    covar(npts, e, n, z, s);
    tred2(3, s, d, o, v);
    tql2(3, d, o, v);

    for(i = 0; i < 3; i++) if(d[i] < 0.) d[i] = 0.;
    d[0] = sqrt(d[0]);      // make units amp instead of power
    d[1] = sqrt(d[1]);
    d[2] = sqrt(d[2]);

/*
    if(d[2] == 0.) {
        rectilinearity = 0.;
    }
    else {
        rectilinearity = 1. - .5*(d[0] + d[1])/d[2];
    }
*/
    i = (v[8] > 0.) ? -1 : 1;
    max_azimuth = 180.*atan2(i*v[6], i*v[7])/M_PI;
    max_incidence = 180.*acos(fabs(v[8]))/M_PI;

    scale = 1./scale;
    for(i = 0; i < npts; i++) {
        e[i] *= scale;
        n[i] *= scale;
        z[i] *= scale;

	e[i] += e_mean;
	n[i] += n_mean;
	z[i] += z_mean;
    }
}

void MotionPlot::rotateMax(void)
{
    azimuth_text->setString("%.1f", max_azimuth, true);
    incidence_text->setString("%.1f", max_incidence, true);
}

void MotionPlot::setArrowLength(void)
{
    Arg args[1];

    if(long_arrow_toggle->state()) {
	XtSetArg(args[0], XtNarrowLength, 100);
    }
    else {
	XtSetArg(args[0], XtNarrowLength, 6);
    }
    pm->setValues(args, 1);
}

void MotionPlot::drawOnMap(int reason)
{
    char    station[10], lab[50];
    int     i, nsta, display;
    double  az, dist;
    Boolean redisplay;
    MapPlotArc arc;
    MapPlotDelta delta;
    MapPlotStation *sta = NULL;
    LineInfo line_info_init = LINE_INFO_INIT;

    if( !map || !map->isVisible() ) return;

    /* get distance text widgetId */
    dist = -1.;
    distance_text->getDouble(&dist);

    display = MAP_ON;

    redisplay = (display == MAP_OFF) ? False : True;

    snprintf(lab, 50, "PM%d", id);
    arc.label = lab;
    delta.label = lab;

    pm->getSta(station);

    if(map_toggle->state())
    {
	az = pm->getAzimuth();

	if(!map->getStaArc(station, arc.label, &arc))
	{
	    nsta = map->getStations(&sta);
	    for(i = 0; i < nsta; i++) {
		if(sta[i].label != NULL && !strcmp(sta[i].label, station)) {
		    break;
		}
	    }
	    if(i == nsta) {
		Free(sta);
		return;
	    }

	    arc.lat = sta[i].lat;
	    arc.lon = sta[i].lon;
	    arc.az = az;
//	    arc.del = (dist > 0.) ? dist : 180.;
	    arc.del = 180.;
	    memcpy(&arc.line, &line_info_init, sizeof(LineInfo));

	    arc.line.fg = sta[i].sym.fg;
	    arc.line.xorr = (reason == 1) ? True : False;
	    arc.line.display = display;
	    map->assoc(map->addArc(&arc, redisplay), sta[i].id);
	    Free(sta);
	}
	else
	{
	    arc.az = az;
//	    arc.del = (dist > 0.) ? dist : 180.;
	    arc.del = 180.;
/*	    arc.line.fg = fg; */
	    arc.line.xorr = (reason == 1) ? True : False;
	    arc.line.display = display;
	    map->change((MapObject *)&arc, redisplay);
	}

	if(dist <= 0. || dist > 180) dist = 1.;

	if(!map->getStaDelta(station, delta.label, &delta))
	{
	    nsta = map->getStations(&sta);
	    for(i = 0; i < nsta; i++) {
		if(sta[i].label != NULL && !strcmp(sta[i].label, station)) {
		    break;
		}
	    }
	    if(i == nsta) {
		Free(sta);
		return;
	    }

	    delta.lat = sta[i].lat;
	    delta.lon = sta[i].lon;
	    delta.del = dist;
	    memcpy(&delta.line, &line_info_init, sizeof(LineInfo));

	    delta.line.fg = sta[i].sym.fg;
	    delta.line.xorr = (reason == 1) ? True : False;
	    delta.line.display = display;
	    map->assoc(map->addDelta(&delta, redisplay), sta[i].id);
	    Free(sta);
	}
	else
	{
	    delta.del = dist;
	    arc.line.xorr = (reason == 1) ? True : False;
	    arc.line.display = display;
	    map->change((MapObject *)&delta, redisplay);
	}
    }
    else {
	if(map->getStaArc(station, arc.label, &arc)) {
	    map->mapDelete(arc.id, true);
	}
	if(map->getStaDelta(station, delta.label, &delta)) {
	    map->mapDelete(delta.id, true);
	}
    }
}

void MotionPlot::saveAzInc(void)
{
    vector<int> sel;
    int changed[2] = {10, 14};
    vector<const char *> row;
    double azimuth, incidence;
    cvector<CssArrivalClass> arr;
    CssArrivalClass *a;
    DataSource *ds;

    if(!data_source) return;

    data_source->getSelectedTable(arr);
    if(arr.size() <= 0) {
        showWarning("No arrivals selected.");
        return;
    }
    else if(arr.size() > 1) {
        showWarning("More than one arrival selected.");
        return;
    }
    if( !(ds = arr[0]->getDataSource()) ) {
        showWarning("Cannot save changes.");
        return;
    }

    if(!azimuth_text->getDouble(&azimuth)) {
        showWarning("Invalid azimuth value.");
        return;
    }
    else if(!incidence_text->getDouble(&incidence)) {
        showWarning("Invalid incidence value.");
        return;
    }

    a = new CssArrivalClass(*arr[0]);

    a->azimuth = azimuth;
    a->ema = incidence;
    ds->changeTable(arr[0], a);
    arr[0]->azimuth = a->azimuth;
    arr[0]->ema = a->ema;
    TableListener::doCallbacks(arr[0], this, 2, changed);
}
