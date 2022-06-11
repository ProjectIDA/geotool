/** \file PolarFilter.cpp
 *  \brief Defines class PolarFilter.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>

using namespace std;

#include "PolarFilter.h"
#include "motif++/MotifClasses.h"
#include "libgio.h"
#include "WaveformPlot.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GArray.h"
#include "gobject++/GCoverage.h"
#include "gobject++/GSourceInfo.h"
#include "IIRFilter.h"
#include "cssio.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

using namespace libghp;


PolarFilter::PolarFilter(const char *name, Component *parent, DataSource *ds)
		: FormDialog(name, parent, false), DataReceiver(NULL)
{
    createInterface();
    setDataSource(ds);
}

void PolarFilter::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    rtd_compute_toggle = new Toggle("RTD Compute", controls, this);
    rtd_compute_toggle->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 3); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    rc = new RowColumn("rc", this, args, n);

    label1 = new Label("Order", rc);
    label2 = new Label("Low Cut (Hz)", rc);
    label3 = new Label("High Cut (Hz)", rc);
    label4 = new Label("Frequency Cycles", rc);
    label5 = new Label("Time Cycles", rc);
    label6 = new Label("Azimuth", rc);
    label7 = new Label("Incidence", rc);
    label8 = new Label("Aperture", rc);
    label9 = new Label("Rectilinearity Scaling", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 10); n++;
    XtSetArg(args[n], XmNvalue, 4); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale1 = new Scale("scale1", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale2 = new Scale("scale2", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale3 = new Scale("scale3", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 2); n++;
    XtSetArg(args[n], XmNmaximum, 6); n++;
    XtSetArg(args[n], XmNvalue, 4); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale4 = new Scale("scale4", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 3); n++;
    XtSetArg(args[n], XmNmaximum, 8); n++;
    XtSetArg(args[n], XmNvalue, 4); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale5 = new Scale("scale5", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 360); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale6 = new Scale("scale6", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 90); n++;
    XtSetArg(args[n], XmNvalue, 10); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale7 = new Scale("scale7", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 8); n++;
    XtSetArg(args[n], XmNvalue, 2); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale8 = new Scale("scale8", rc, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 8); n++;
    XtSetArg(args[n], XmNvalue, 2); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    scale9 = new Scale("scale9", rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "4"); n++;
    order = new TextField("order", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "0"); n++;
    low_cut = new TextField("low_cut", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "0"); n++;
    hi_cut = new TextField("hi_cut", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "4"); n++;
    freq_cycles = new TextField("freq_cycles", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "4"); n++;
    time_cycles = new TextField("time_cycles", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "0"); n++;
    azimuth = new TextField("azimuth", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "10"); n++;
    incidence = new TextField("incidence", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "2"); n++;
    aperture = new TextField("aperture", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 7); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "2"); n++;
    recti_scaling = new TextField("recti_scaling", rc, args, n);

    ln_max = getProperty("polar_ln_max", 3.);
    scale_hi_freq = getProperty("scale_hi_freq", 150.);
}

PolarFilter::~PolarFilter(void)
{
}

void PolarFilter::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
        if(data_source) {
            data_source->removeDataReceiver(this);
            if(wp) {
                wp->removeActionListener(this, "RTDUpdate");
            }
        }
        data_source = ds;
        if(data_source) {
            data_source->addDataReceiver(this);
            wp = data_source->getWaveformPlotInstance();
            if(wp) {
                if(wp->isRealTime()) {
                    wp->addActionListener(this, "RTDUpdate");
                    rtd_compute_toggle->setVisible(true);
                }
            }
        }
        else wp = NULL;
    }
}

void PolarFilter::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
        setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
    }
    else if(!strcmp(cmd, "Polarization Filter Help")) {
	apply();
    }
    else if(!strncmp(cmd, "scale", 5)) {
	scale((Scale *)action_event->getSource());
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Polarization Filter Help");
    }
    else if(!strcmp(action_event->getReason(), "RTDUpdate")) {
	if(rtd_compute_toggle->state()) {
	    apply();
	}

    }
}

ParseCmd PolarFilter::parseCmd(const string &cmd, string &msg)
{
    string c;
    int value;
    double d;

    if(parseCompare(cmd, "Apply")) {
	apply_button->activate();
    }
    else if(parseArg(cmd, "Order", c)) {
	if(!stringToInt(c.c_str(), &value) || value <= 0 || value > 10) {
	    msg.assign("Invalid order (1-10)");
	    return ARGUMENT_ERROR;
	}
	order->setString(c, true);
    }
    else if(parseArg(cmd, "Low_Cut", c)) {
	if(!stringToDouble(c.c_str(), &d) || d < 0.) {
	    msg.assign("Invalid low cut");
	    return ARGUMENT_ERROR;
	}
	low_cut->setString(c, true);
    }
    else if(parseArg(cmd, "High_Cut", c)) {
	if(!stringToDouble(c.c_str(), &d) || d < 0.) {
	    msg.assign("Invalid high cut");
	    return ARGUMENT_ERROR;
	}
	hi_cut->setString(c, true);
    }
    else if(parseArg(cmd, "Frequency_Cycles", c)) {
	if(!stringToInt(c.c_str(), &value) || value < 2 || value > 6) {
	    msg.assign("Invalid frequency cycles (2-6)");
	    return ARGUMENT_ERROR;
	}
	freq_cycles->setString(c, true);
    }
    else if(parseArg(cmd, "Time_Cycles", c)) {
	if(!stringToInt(c.c_str(), &value) || value < 3 || value > 8) {
	    msg.assign("Invalid time cycles (3-8)");
	    return ARGUMENT_ERROR;
	}
	time_cycles->setString(c, true);
    }
    else if(parseArg(cmd, "Azimuth", c)) {
	if(!stringToDouble(c.c_str(), &d) || d < 0 || d > 360) {
	    msg.assign("Invalid azimuth (0-360)");
	    return ARGUMENT_ERROR;
	}
	azimuth->setString(c, true);
    }
    else if(parseArg(cmd, "Incidence", c)) {
	if(!stringToDouble(c.c_str(), &d) || d < 0 || d > 90) {
	    msg.assign("Invalid incidence (0-90)");
	    return ARGUMENT_ERROR;
	}
	incidence->setString(c, true);
    }
    else if(parseArg(cmd, "Aperture", c)) {
	if(!stringToInt(c.c_str(), &value) || value < 1 || value > 8) {
	    msg.assign("Invalid aperture (1-8)");
	    return ARGUMENT_ERROR;
	}
	aperture->setString(c, true);
    }
    else if(parseArg(cmd, "Scaling", c)) {
	if(!stringToInt(c.c_str(), &value) || value < 1 || value > 8) {
	    msg.assign("Invalid scaling (1-8)");
	    return ARGUMENT_ERROR;
	}
	recti_scaling->setString(c, true);
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

void PolarFilter::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%sorder=(1-10)\n", prefix);
    printf("%slow_cut=FREQ\n", prefix);
    printf("%shigh_cut=FREQ\n", prefix);
    printf("%sfrequency_cycles=(2-6)\n", prefix);
    printf("%stime_cycles=(3-8)\n", prefix);
    printf("%sazimuth=DEGREES\n", prefix);
    printf("%sincidence=DEGREES\n", prefix);
    printf("%saperture=(1-8)\n", prefix);
    printf("%sscaling=(1-8)\n", prefix);
}

void PolarFilter::scale(Scale *sc)
{
    if(sc == scale1) { // order
	order->setString(sc->getValue());
    }
    if(sc == scale2) { // low cut
	double max = sc->getMaximum();
	double ln = sc->getValue()*ln_max/(float)max;
	double max_nyquist = getMaxNyquist();
	double freq_max = scale_hi_freq;
	if(freq_max < max_nyquist) freq_max = max_nyquist;
	double value = freq_max*(exp(ln)-1.)/(exp(ln_max)-1.);
	char s[100];
	ftoa(value, 2, 0, s, 100);
	low_cut->setString(s);
    }
    else if(sc == scale3) { // high cut
	double max = sc->getMaximum();
	double ln = sc->getValue()*ln_max/(float)max;
	double max_nyquist = getMaxNyquist();
	double freq_max = scale_hi_freq;
	if(freq_max < max_nyquist) freq_max = max_nyquist;
	double value = freq_max*(exp(ln)-1.)/(exp(ln_max)-1.);
	char s[100];
	ftoa(value, 2, 0, s, 100);
	hi_cut->setString(s);
    }
    else if(sc == scale4) { // frequency cycles
	freq_cycles->setString(sc->getValue());
    }
    else if(sc == scale5) { // time cycles
	time_cycles->setString(sc->getValue());
    }
    else if(sc == scale6) { // azimuth
	azimuth->setString(sc->getValue());
    }
    else if(sc == scale7) { // incidence
	incidence->setString(sc->getValue());
    }
    else if(sc == scale8) { // aperture
	aperture->setString(sc->getValue());
    }
    else if(sc == scale9) { // rectilinear scaling
	recti_scaling->setString(sc->getValue());
    }
}

double PolarFilter::getMaxNyquist()
{
    gvector<Waveform *> wvec;
    double nyquist, max = 0.;
    data_source->getWaveforms(wvec, true);

    for(int i = 0; i < wvec.size(); i++) {
	for(int j = 0; j < wvec[i]->size(); j++) {
	    if(wvec[i]->segment(j)->tdel() > 0.) {
		nyquist = .5/wvec[i]->segment(j)->tdel();
		if(max < nyquist) max = nyquist;
	    }
	}
    }
    return max;
}

void PolarFilter::apply(void)
{
    int		i, k, npts, norder, num_waveforms, num;
    gvector<Waveform *> wvec, ws;
    Waveform	*polar, *last_polar = NULL;
    float	*y = NULL;
    double	dt, tcycles, fc1, fc2, fcycles, apert, rect, thni, phi;
    double	nyquist;
    vector<double> scaled_y0;
    gvector<GTimeSeries *> ts;
    GTimeSeries	*polar_ts;
    GSegment	*s;
    double	calib, calper;
    CssOriginClass	*origin = NULL;
    GCoverage	*o;

    if( !wp ) return;

    if(!order->getInt(&norder) || norder < 1 || norder > 10) {
	showWarning("Invalid Order");
	return;
    }
    if(!freq_cycles->getDouble(&fcycles) || fcycles < 2 || fcycles > 6) {
	showWarning("Invalid Number of Frequency Cycles.");
	return;
    }
    if(!time_cycles->getDouble(&tcycles) || tcycles < 3 || tcycles > 8) {
	showWarning("Invalid Number of Time Cycles.");
	return;
    }
    if(!aperture->getDouble(&apert) || apert < 1. || apert > 10.) {
	showWarning("Invalid Aperture.");
	return;
    }
    if(!recti_scaling->getDouble(&rect) || rect < 1. || rect > 10.) {
	showWarning("Invalid Rectilinearity Scaling");
	return;
    }
    if(!azimuth->getDouble(&thni)) {
	showWarning("Invalid Azimuth.");
	return;
    }
    if(!incidence->getDouble(&phi)) {
	showWarning("Invalid Incidence.");
	return;
    }

    if((num_waveforms = wp->getSelectedComponents("zne", wvec)) <= 0)
    {
	showWarning("No 3-component waveform set selected.");
	return;
    }
    else if(num_waveforms > 3)
    {
	showWarning("More than one 3-component set selected.");
	return;
    }

    nyquist = 1./(2.*wvec[0]->segment(0)->tdel());
	
    if(!low_cut->getDouble(&fc1) || fc1 < 0. || fc1 > nyquist) {
	char buf[100];
	stringcpy(buf, "Invalid filter low cut.", sizeof(buf));
	if(fc1 > nyquist) {
	    snprintf(buf+strlen(buf), 100, "\nNyquist = %6.3f",nyquist);
	}
	showWarning(buf);
	return;
    }
    if(!hi_cut->getDouble(&fc2) || fc2 < 0. || fc2 > nyquist) {
	char buf[100];
	stringcpy(buf, "Invalid filter high cut.", sizeof(buf));
	if(fc2 > nyquist) {
	    snprintf(buf+strlen(buf), 100, "\nNyquist = %6.3f",nyquist);
	}
	showWarning(buf);
	return;
    }

    if(fc1 == 0.) {
	fc1 = .1*nyquist;
	low_cut->setString("%.2f", fc1, false);
    }
    if(fc2 == 0.) {
	fc2 = nyquist;
	hi_cut->setString("%.2f", fc2, false);
    }

    calib = calper = 0.0;
    /* compute average calib/calper
     */
    for(i = 0; i < num_waveforms; i++)
    {
	calper += wvec[i]->segment(0)->calper();
	calib += (wvec[i]->segment(0)->calib() != 0.) ?
			wvec[i]->segment(0)->calib() : 1.;
	if(origin == NULL) {
	    origin = data_source->getPrimaryOrigin(wvec[i]);
	}
    }
    calper /= (double)num_waveforms;
    calib /= (double)num_waveforms;
    if(calib == 0.) calib = 1.;

    setCursor("hourglass");

    /* delete last polar-waveform, if found
     */
    num = data_source->getWaveforms(ws, false);
    for(i = 0; i < num; i++) {
	if(!strcmp(ws[i]->sta(), wvec[0]->sta()) &&
		ws[i]->ts->getValue("polarization"))
	{
	    last_polar = ws[i];
	    break;
	}
    }
    if(last_polar != NULL) {
	for(int j = 0; j < num_waveforms; j++) {
	    if(wvec[j]->scaled_y0 > last_polar->scaled_y0) {
		wvec[j]->scaled_y0 -= 1.;
	    }
	}
	if(wp) {
	    gvector<Waveform *> v(last_polar);
	    wp->deleteWaveforms_NoReposition(v);
	}
    }

    /* process segments which have data for all three components
     */
    ts.push_back(wvec[0]->ts);	/* z */
    ts.push_back(wvec[1]->ts);	/* n */
    ts.push_back(wvec[2]->ts);	/* e */

    dt = wvec[0]->segment(0)->tdel();

    polar_ts = new GTimeSeries();
    polar_ts->source_info.setSource(ts[0]->source_info);

    for(o = new GCoverage(ts); o->hasMoreElements(); )
    {
	GArray *array = (GArray *)o->nextElement();

	y = (float *)mallocWarn(array->length()*sizeof(float));

	polarfilt(array->data[0], array->data[2], array->data[1],
		array->length(), dt, tcycles, fc1, fc2, fcycles, norder,
		apert, rect, thni, phi, y);

	npts = array->length();
	for(i = 0; i < npts; i++) y[i] /= calib;

	s = new GSegment(y, npts, array->tbeg(), array->tdel(), calib, calper);
	Free(y);
	polar_ts->addSegment(s);

	array->deleteObject();
    }
    o->deleteObject();

    if(polar_ts->length() <= 0) {
	delete polar_ts;
	setCursor("default");
	return;
    }

    polar_ts->makeCopy();
    polar_ts->setDataSource(ts[0]->getDataSource());
    polar_ts->copyWfdiscPeriods(ts[0]); // to addArrivals
    WfdiscPeriod *wpd = polar_ts->getWfdiscPeriod(-1.e+60);
    snprintf(wpd->wf.sta, sizeof(wpd->wf.sta), "%s", wvec[0]->sta());
    snprintf(wpd->wf.chan, sizeof(wpd->wf.chan), "%s", "polar");
    polar_ts->setSta(wvec[0]->sta());
    polar_ts->setNet(wvec[0]->net());
    polar_ts->setChan("polar");
    polar_ts->setComponent(1);

    polar = wp->addWaveform(polar_ts);

    polar->ts->putValue("polarization", 1);

    /* get all the waveforms, find the one just read in,
     * and position it correctly
     */
    ws.clear();
    num_waveforms = wp->getWaveforms(ws, false);

    if(origin != NULL) {
	if(wp) {
	    wp->setPrimaryOrigin(polar, origin);
	    wp->updatePredicted();
	}
    }

    /* take polar out */
    for(i = 0; i < num_waveforms-1; i++) {
	if(ws[i]->scaled_y0 > polar->scaled_y0) {
	    ws[i]->scaled_y0 -= 1.;
	}
    }

    /* place the polar below the lowest selected waveform
     */
    Waveform::sortByY0(ws);
    for(k = num_waveforms-2; k >= 0; k--) if(ws[k]->selected) break;

    if(k < 0) k = num_waveforms-2;  /* none selected */

    wvec.clear();
    for(i = 0; i <= k; i++) wvec.push_back(ws[i]);
    wvec.push_back(polar);
    for(i = k+1; i < num_waveforms-1; i++) wvec.push_back(ws[i]);

    scaled_y0.resize(num_waveforms, 0.);

    for(i = 0; i <= k; i++) scaled_y0[i] = ws[i]->scaled_y0;
    for(i = num_waveforms-1; i >= k+2; i--) {
	scaled_y0[i] = ws[i]->scaled_y0 + 1;
    }
    scaled_y0[k+1] = ws[k]->scaled_y0 + 1;

    if(wp) {
	wp->positionY(wvec, scaled_y0);
	wp->setWaveformOrder(wvec);
    }

    setCursor("default");
}

#define ldimwndo 10000
#define ZERO_PHASE 1

static void unitvect(double east, double azim, double colat, float *r);
static void ellipse(float *z, float *n, float *e, int npts, double *values,
			double vectors[3][3]);
static void costaper(float *y, int npts, int ntaper);

/*
 *	author: Andy Jurkevics, CSS, Aug 1986
 *
 *	translated to C: Ivan Henson, Teledyne Geotech, March 1993
 */

/** 
 *
 *	Polarization filter 3-component seismograms. This function filters
 *	3-component seismograms with a polarization filter to pass rectilinear
 *	motion with any orientation.
 * <p>
 *	author: Andy Jurkevics, CSS, Aug 1986
 * <pre>
 *	input:
 *	fc1, fc2	frequency interval for analysis, fc1 < fc2
 *			This inverval will be broken into a number of
 *			sub-bands depending on the freq resolution.
 *			fcycles	resolution of frequency bands in # of cycles, 
 *			use 2-6
 *			**** fc1 must be > 0. ****
 *	norder		order of butterworth for bandpassing, use
 *			even order, 4 recommended 
 *			(2 passes of filter made for zero phase)	
 *	seglen		length of time segment in seconds
 *	tcycles		resolution of time window in # of cycles, 
 *			use 3-8
 *	apert		aperture parameter of form cos**apert
 *			for controling width of 'opening' to pass
 *			rectilinear motion
 *	rect		parameter for scaling output according to
 *			degree of rectilinearity: (1-ev1/ev2)**rect
 *	thni		azimuth of pass rectilinear motion from north 
 *			in degrees
 *	phi		incidence of pass rectilinear motion from
 *			vertical in degrees
 * </pre>
 */
int PolarFilter::
polarfilt(float *sz, float *se, float *sn, int npts, double dt, double tcycles,
	double fc1, double fc2, double fcycles, int norder, double apert,
	double rect, double thni, double phi, float *y)
{
	int i, j, last, iwndo, lwndo, nwndo, ltaper, iband, nbands, ibgn, ifin,
		ntheta, imax, itheta;
	float *gz, *gn, *ge, *hz, *hn, *he, *buf;
	double direction[100][3], vectors[3][3], values[3];
	double r[3][3], rot[3];
	double flo[15], fhi[15];
	double rad, ftemp, fcenter, width, taper, theta, dtheta, ampmax,
		sign0, sign1, amplit, sum, cross, rectilin;
	IIRFilter *iir;

	if(npts <= 0 || fc1 <= 0. || fc2 <= fc1) return(-1);

	if((buf = (float *)malloc(7*npts*sizeof(float))) == NULL)
	{
	    logErrorMsg(LOG_ERR, "polarfilt: malloc error.");
	    return(-1);
	}
	gz = buf +   npts;
	gn = buf + 2*npts;
	ge = buf + 3*npts;
	hz = buf + 4*npts;
	hn = buf + 5*npts;
	he = buf + 6*npts;

	rad = 180./M_PI;
	/*
	 * compute number of bands and the corner frequencies
	 * note upper band limit may change somewhat from input value
	 */
	nbands	= 0;
	ftemp	= fc1;
	for(iband = 0; iband < 15; iband++)
	{
	    flo[iband] = ftemp;
	    fcenter	= flo[iband]/(1.-1./(2.*fcycles));
	    width	= fcenter/fcycles;
	    fhi[iband] = flo[iband] + width;
	    ftemp	= fhi[iband];
	    if(fcenter > fc2) break;
	    nbands	= nbands + 1;
	}
	fc2 = fhi[nbands-1];

	/*
	 * read 3-component traces beginning at correct time lag for each
	 * station.  recall 1 = z, 2 = n, 3 = e
	 */

	/*
	 * rotate to more natural coordinates so that 
	 * component 1 = desired rectilinear component (p)
	 * component 2 = 90 degrees from 1 on same azimuth (sv)
	 * component 3 = 90 degrees from azimuth of 1 and 2 (sh)
	 * r is unit vector for doing rotation
	 */

	unitvect(1., thni, phi, buf);
	for(i = 0; i < 3; i++) r[i][0] = buf[i];
	unitvect(1., thni, phi+90., buf);
	for(i = 0; i < 3; i++) r[i][1] = buf[i];
	unitvect(1., thni+90., 90., buf);
	for(i = 0; i < 3; i++) r[i][2] = buf[i];

	for(i = 0; i < npts; i++)
	{
	    for(j = 0; j < 3; j++)
	    {
		rot[j] = r[0][j]*sz[i] + r[1][j]*sn[i] + r[2][j]*se[i];
	    }
	    sz[i] = rot[0];
	    sn[i] = rot[1];
	    se[i] = rot[2];
	}
	/*
	 * zero the output trace
	 */
	for(i = 0; i < npts; i++) y[i] = 0.;

	/*
	 * main loop over frequencies
	 */
	for(iband = 0; iband < nbands; iband++)
	{
	    /*
	     * bandpass
	     */
	    memcpy(gz, sz, npts*sizeof(float));
	    memcpy(gn, sn, npts*sizeof(float));
	    memcpy(ge, se, npts*sizeof(float));

	    iir = new IIRFilter(norder, "HP", flo[iband], fhi[iband], dt,
				ZERO_PHASE);
	    iir->applyMethod(gz, npts, true);
	    iir->applyMethod(gn, npts, true);
	    iir->applyMethod(ge, npts, true);
	    iir->deleteObject();

	    iir = new IIRFilter(norder, "LP", flo[iband], fhi[iband], dt,
				ZERO_PHASE);
	    iir->applyMethod(gz, npts, true);
	    iir->applyMethod(gn, npts, true);
	    iir->applyMethod(ge, npts, true);
	    iir->deleteObject();

	    costaper(gz, npts, 12);
	    costaper(gn, npts, 12);
	    costaper(ge, npts, 12);
	    /*
	     * establish windows 
	     */
	    taper   = .5;
	    fcenter = (flo[iband]+fhi[iband])/2.;
	    lwndo   = (int)(tcycles/(dt*fcenter));
	    ltaper  = (int)(lwndo*taper);
	    ifin    = ltaper + 1;
	    last    = 0;
	    /*
	     * loop over time windows
	     */
	    for(iwndo = 0; iwndo < ldimwndo; iwndo++)
	    {
		if(last) break;
		ibgn = ifin - ltaper;
		ifin = ibgn + lwndo - 1;
		if(ifin > npts)
		{
		    last = 1;
		    ifin = npts;
		    if((double)(ifin-ibgn) < 1.5*ltaper) break;
		}
		nwndo	= ifin - ibgn + 1;
		/*
		 * copy window segment and cosine taper
		 */
		memcpy(hz, gz+ibgn-1, nwndo*sizeof(float));
		memcpy(hn, gn+ibgn-1, nwndo*sizeof(float));
		memcpy(he, ge+ibgn-1, nwndo*sizeof(float));
		costaper(hz, nwndo, ltaper);
		costaper(hn, nwndo, ltaper);
		costaper(he, nwndo, ltaper);
		/*
		 * find polarization ellipse 
		 */
		ellipse(hz, hn, he, nwndo, values, vectors);
		/*
		 * print orientation of largest evector, etc.
		 *
 		 * t = dt*(ibgn+ifin)/2.
		 * sgn	= sign(1.,vectors(1,1))
		 * temp1 = atan2(sgn*vectors(3,1),sgn*vectors(2,1))*rad
		 * temp2 = acos(sgn*vectors(1,1))*rad
		 */
		/*
		 * do the vector filtering. subtract smallest eigenvalue
		 * from others to remove noise contribution. only
		 * largest two eigenvalues used from now on.
		 */
		for(j = 0; j < 3; j++)
		{
		    values[j] = sqrt(values[j]*values[j] - values[2]*values[2]);
		}

		/*
		 * Search along ellipse from minor axis to major axis
		 * for maximum of (ellipse value)*(aperture function).
		 * Bisect angle between minor and major axis ntheta
		 * times in search.  Begin search at minor axis.
		 */
		ntheta	= 11;
		dtheta	= 90./(ntheta-1);
		ampmax	= 0.;
		imax = 0;
		for(itheta = 0; itheta < ntheta; itheta++)
		{
		    theta = itheta*dtheta/rad;
		    /*
		     * compute test vector 'direction' in trace
		     * coordinates using ellipse axis directions
		     * which point upwards
		     */
		    sign0 = (vectors[0][0] >= 0.) ? 1 : -1;
		    sign1 = (vectors[0][1] >= 0.) ? 1 : -1;
		    for(j = 0; j < 3; j++)
		    {
			direction[j][itheta] = vectors[j][0]*sin(theta)*sign0 +
				vectors[j][1]*cos(theta)*sign1;
		    }
		    /*
		     * compute test vector in ellipse coordinates
		     */
		    rot[0] = sin(theta);
		    rot[1] = cos(theta);
		    /*
		     *compute amplitude of ellipse in this direction
		     */
		    for(j = 0, amplit = 0.; j < 2; j++)
		    {
			amplit += pow(rot[j]/(values[j]+1.e-6), 2.);
		    }
		    amplit = sqrt(1./amplit);
		    /*
		     * multiply ellipse amplitude by aperture
		     * The aperture cosine is the dot product of
		     * test vector with pass direction which is
		     * pass(1)*direction(1,itheta)
		     */
		    amplit *= pow(direction[0][itheta], apert);
		    /*
		     * find direction to rectilinear motion to pass 
		     */
		    if(amplit > ampmax)
		    {
			ampmax	= amplit;
			imax	= itheta;
		    }
		}
		/*
		 * construct the pass component as linear combination
		 * of data in the direction of direction(j,imax) 
		 */
		for(i = 0, sum = cross = 0.; i < nwndo; i++)
		{
		    buf[i] = direction[0][imax]*hz[i] + direction[1][imax]*hn[i]
				+ direction[2][imax]*he[i];
		    sum += buf[i]*buf[i];
		    cross += hz[i]*buf[i];
		}
		sum = sqrt(sum/nwndo) + 1.e-7;
		/*
		 * scale to amplitude of ellipse*(cos)**n
		 * and multiply by degree of rectilinearity
		 */
		rectilin = pow(1.-values[1]/values[0], rect);
		if(cross < 0.) ampmax *= -1;
		for(i = 0; i < nwndo; i++)
		{
		    buf[i] *= ampmax/sum*rectilin;
		}
		/*
		 * add contribution to output trace
		 */
		for(i = 0; i < nwndo; i++)
		{
		    y[ibgn+i-1] +=  buf[i];
		}
	    }
	}
	free(buf);
	return(0);
}

/** 
 * Convert from angles to unit vector r.
 *
 * <pre>
 * convention is:
 *	direction 1 is up
 *	direction 2 is north
 *	direction 3 is east
 *	if angle from north to east is +ve, set east = 1.
 *	if angle from north to east is -ve, set east = -1.
 *
 * input:
 *	east	+1. if angle(north,east) is +ve
 *	 	-1. if angle(north,east) is -ve 
 *	azim	azimuth in degrees between north and vector r
 *	colat	angle in degrees between up and vector r
 *	all angles input in degrees
 *
 * output:
 *	r(1..3)	unit vector  relative to up, north, east
 * </pre>
 */
static void
unitvect(double east, double azim, double colat, float *r)
{
	double rad, th2r, th1r;

	rad = 180./M_PI;

	if(east != -1 && east != 1.)
	{
	    logErrorMsg(LOG_WARNING, "polarfilt: invalid value for east.");
	    return;
	}

	th2r = azim/rad*east;
	th1r = colat/rad;
	r[0] = cos(th1r);
	r[1] = cos(th2r)*sin(th1r);
	r[2] = sin(th2r)*sin(th1r);
}

static void
ellipse(float *z, float *n, float *e, int npts, double *values, 
	double vectors[3][3])
{
	/*
	 * Computes the polarization ellipse as eigenvectors and eigenvalues.
	 *	values(3)	eigenvalues
	 *	vectors(3,3)	eigenvectors
	 */
	int i, j;
	double s[9], d[3], o[3], v[9];

	covar(npts, z, n, e, s);

	/* 
	 * compute eigenvectors and eigenvalues
	 */
	tred2(3, s, d, o, v);
	/*
	 * eigenvalues are returned from tql2 in ascending order
	 */
	tql2(3, d, o, v);

	/* order by descending eigenvalue and take sqrt
	 */
	for(j = 0; j < 3; j++)
	{
	    values[j] = sqrt(d[2-j]);
	    for(i = 0; i < 3; i++)
	    {
		vectors[i][j] = v[(2-j)*3 + i];
	    }
	}
}

static void
costaper(float *y, int npts, int ntaper)
{
	int i, nt;
	double arg, taper;

	if(ntaper < 0) return;
	nt = (ntaper <= npts) ? ntaper : npts;

	arg = 2.*M_PI/(double)nt;

	for(i = 0; i < nt; i++)
	{
	    taper = 0.54 - 0.46*cos((double)i*arg);
	    y[i] *= taper;
	    y[npts-1-i] *= taper;
	}
}
