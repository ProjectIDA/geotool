/** \file Ftrace.cpp
 *  \brief Defines class Ftrace.
 *  \author Vera Miljanovic
 */

/* Program to calculate and apply a time shift given a backazimuth (BAZ),
   phase velocity (PVEL) of a series of M array waveforms (up to narr=55).
   x and y offsets are calculated from station latitudes and longitudes
   relative to the array centre (CLAT, CLON).
   Bandpass filter data (using a recursive Butterworth filter)
   with corner frequencies at LF and HF, and NPOL order (must be even).
   (two-pass acausal filtering).
   Calculates F-trace using a moving window 2*SPTS+1 long.
   Probability (using the F-statistic) that a signal is present above
   bandlimited noise, with SNR equal to sqrt(signal/noise power)
   (see Douze and Laster, 1979. Geophysics. 44, pp.1999 and
   Blandford, 1974. Geophysics. 39, pp.633)
 */

#include "config.h"
#include <iostream>
#include <math.h>
#include <stdio.h>
using namespace std;

#include "Ftrace.h"
#include "Beam.h"
#include "motif++/MotifClasses.h"
#include "gobject++/GTimeSeries.h"

using namespace libgftrace;


Ftrace::Ftrace(const char *name, Component *parent, DataSource *ds)
		: WaveformWindow(name, parent)
{
    data_source = ds;
    if(data_source) {
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;

    createInterface();
}

void Ftrace::createInterface()
{
    int n;
    Arg args[20];

    // redo the WaveformWindow menubar and toolbar
    tool_bar->removeAllItems();

//    messages_button->setVisible(false);
    preferences_button->setVisible(false);
    plugins_button->setVisible(false);
    table_files_button->setVisible(false);
    tv_button->setVisible(false);
    warnings_button->setVisible(false);
    new_window_button->setLabel("New Window...");
    waveform_setup_button->setVisible(false);
    quit_button->setVisible(false);
    
    option_menu->setVisible(false);
    help_menu->setVisible(false);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    compute_button = new Button("Compute", option_menu, this);
    auto_beam_loc = new Toggle("Auto Beam Location", option_menu, this);
    auto_beam_loc->set(true);

    print_window = NULL;

    addPlugins("Ftrace", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(compute_button);
	tool_bar->add(clear_button);
    }
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    RowColumn *rc = new RowColumn("rc", frame_form, args, n);

    new Label("azimuth (deg)", rc);
    new Label("slowness (s/km)", rc);
    lat_label = new Label("beam latitude", rc);
    lat_label->setSensitive(false);
    lon_label = new Label("beam longitude", rc);
    lon_label->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;

    az_text = new TextField("az", rc, args, n);
    az_text->setString("300.6");
    slo_text = new TextField("slow", rc, args, n);
    slo_text->setString("0.0588");
    lat_text = new TextField("lat", rc, args, n);
    lat_text->setString("62.49320");
    lat_text->setSensitive(false);
    lon_text = new TextField("lon", rc, args, n);
    lon_text->setString("-114.60530");
    lon_text->setSensitive(false);

    new Label("window length (sec)", rc);
    new Label("signal-to-noise ratio", rc);
    new Label("low frequency", rc);
    new Label("high frequency", rc);

    len_text = new TextField("len", rc, args, n);
    len_text->setString("2.0");
    snr_text = new TextField("snr", rc, args, n);
    snr_text->setString("6.0");
    flo_text = new TextField("flo", rc, args, n);
    flo_text->setString("0.4");
    fhi_text = new TextField("fhi", rc, args, n);
    fhi_text->setString("3.0");

    n = 0;
    XtSetArg(args[n], XtNdataHeight, 50); n++;
    XtSetArg(args[n], XtNdataSeparation, 20); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    wplot->setValues(args, n);

    cursor.assign("a");

    string recipe_dir, recipe_dir2;
    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    recipe_dir.assign(c + string("/tables/recipes"));
	}
    }
    getProperty("recipeDir2", recipe_dir2);

    gbeam = new Beam(recipe_dir, recipe_dir2);
}

Ftrace::~Ftrace(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void Ftrace::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	}
	else wp = NULL;
    }
}

void Ftrace::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute") ) {
	compute();
    }
    else if(comp == auto_beam_loc) {
	bool state = !auto_beam_loc->state();
	lat_label->setSensitive(state);
	lon_label->setSensitive(state);
	lat_text->setSensitive(state);
	lon_text->setSensitive(state);
    }
    else if(!strcmp(cmd, "New Window...")) {
	newWindow();
    }
    else if(!strcmp(cmd, "Ftrace Help")) {
	showHelp(cmd);
    }
    else {
	WaveformWindow::actionPerformed(action_event);
    }
}

void Ftrace::compute(void)
{
    int i, spts, npol;
    bool zp;
    double az, slowness, beam_lat, beam_lon, flo, fhi, snr;
    double tmin, tmax, diff, window_len;
    GTimeSeries *beam_ts, *semb_ts, *fst_ts, *prob_ts;
    gvector<Waveform *> wvec;
  
    if(data_source == NULL) {
	showWarning("No data.");
	return;
    }
  
    if(!cursor.empty() && data_source->getSelectedWaveforms(cursor, wvec) > 0) {
	if(wvec[0]->num_dw > 0) {
	    tmin = wvec[0]->dw[0].d1->time();
	    tmax = wvec[0]->dw[0].d2->time();
	}
	else {
	    showWarning("Cannot get selected data.");
	    return;
	}
    }
    else if(data_source->getSelectedWaveforms(wvec) > 1) {
	tmin = wvec[0]->tbeg();
	tmax = wvec[0]->tend();
    }
    else if(wvec.size() == 1) {
	getArrayElements(wvec, &tmin, &tmax);
	if (tmin == tmax) {
	  showWarning("Failed to get array elements. Select more than 1 waveform.");
	  return;
	}
    }
    else {
	showWarning("No waveforms selected.");
	return;
    }
  
    for(i = 1; i < wvec.size(); i++) {
	diff = fabs(wvec[i]->segment(0)->tdel() -
			wvec[0]->segment(0)->tdel());
	if(diff/wvec[i]->segment(0)->tdel() > .01) {
	    showWarning("non constant sample rate.");
	    return;
	}
    }

    if(!az_text->getDouble(&az)) {
	showWarning("Cannot interpret azimuth");
	return;
    }
    if(!slo_text->getDouble(&slowness)) {
	showWarning("Cannot interpret slowness");
	return;
    }
    if( !auto_beam_loc->state() ) {
	if(!lat_text->getDouble(&beam_lat)) {
	    showWarning("Cannot interpret latitude");
	    return;
	}
	if(!lon_text->getDouble(&beam_lon)) {
	    showWarning("Cannot interpret longitude");
	    return;
	}
    }
    if(!flo_text->getDouble(&flo)) {
	showWarning("Cannot interpret low freq");
	return;
    }
    if(!fhi_text->getDouble(&fhi)) {
	showWarning("Cannot interpret high freq");
	return;
    }
    if(!len_text->getDouble(&window_len)) {
	showWarning("Cannot interpret window length");
	return;
    }
    if(!snr_text->getDouble(&snr)) {
	showWarning("Cannot interpret signal-to-noise");
	return;
    }

    spts = (int)(window_len/wvec[0]->segment(0)->tdel() + .5) + 1;
    spts = (spts-1)/2;
    if(2*(spts/2) != spts) spts++;

    npol = 2;
    zp = true;

    beam_ts = new GTimeSeries();
    semb_ts = new GTimeSeries();
    fst_ts = new GTimeSeries();
    prob_ts = new GTimeSeries();

    if( auto_beam_loc->state() ) {
	vector<double> tlags;
	if(Beam::getTimeLags(this, wvec, az, slowness, DNORTH_DEAST, tlags,
                        &beam_lat, &beam_lon) )
	{
	    Beam::ftrace(wvec, tmin, tmax, az, slowness, beam_lat, beam_lon,
			spts, npol, flo, fhi, zp, snr, beam_ts, semb_ts,
			fst_ts, prob_ts);
	    lat_text->setString("%.5f", beam_lat);
	    lon_text->setString("%.5f", beam_lon);
	}
    }
    else {
	Beam::ftrace(wvec, tmin, tmax, az, slowness, beam_lat, beam_lon, spts,
	    npol, flo, fhi, zp, snr, beam_ts, semb_ts, fst_ts, prob_ts);
    }

    beam_ts->tag.members.push_back(0);
    beam_ts->tag.ud_string.assign("beam");
    wplot->addWaveform(beam_ts);

    fst_ts->tag.members.push_back(0);
    fst_ts->tag.ud_string.assign("F-trace");
    wplot->addWaveform(fst_ts);

    semb_ts->tag.members.push_back(0);
    semb_ts->tag.ud_string.assign("semblance");
    wplot->addWaveform(semb_ts);

    prob_ts->tag.members.push_back(0);
    prob_ts->tag.ud_string.assign("probabilty");
    wplot->addWaveform(prob_ts);
}

int Ftrace::getArrayElements(gvector<Waveform *> &wlist, double *tmin,
				double *tmax)
{
    gvector<Waveform *> ws;
    Waveform *w;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    int i, j, nsta;

    *tmin = *tmax = 0.;

    if(!data_source) {
	wlist.clear();
	return 0;
    }

    w = wlist[0];
    wlist.clear();

    data_source->getWaveforms(ws, false);

    if( !gbeam->beamRecipe(w->net(), "cb", recipe) ) {
	showWarning("Cannot get beam elements for %s", w->net());
	return 0;
    }
    if((nsta = Beam::getGroup(recipe, beam_sta)) <= 0)
    {
	showWarning(GError::getMessage());
	return 0;
    }

    CssOriginClass *origin = data_source->getPrimaryOrigin(w);

    for(j = 0; j < nsta; j++)
    {
	for(i = 0; i < ws.size(); i++)
	{
	    if( !strcasecmp(ws[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(ws[i]->chan(), beam_sta[j].chan) &&
		    origin == data_source->getPrimaryOrigin(ws[i]))
	    {
		wlist.push_back(ws[i]);
		break;
	    }
	}
    }
    if(w->num_dw > 0) {
	*tmin = w->dw[0].d1->time();
	*tmax = w->dw[0].d2->time();

	// discard waveforms that are completely out of the window.
	gvector<Waveform *> v(wlist);
	wlist.clear();
	for(i = 0; i < v.size(); i++) {
	    if( !(v[i]->tbeg() > *tmax || v[i]->tend() < *tmin) )
	    {
		wlist.push_back(v[i]);
	    }
	}
    }

    return wlist.size();
}

void Ftrace::newWindow(void)
{
    int i;

    for(i = 0; i < (int)windows.size(); i++) {
	if( !windows[i]->isVisible() ) {
	    windows[i]->setVisible(true);
	    break;
	}
    }
    if(i == (int)windows.size()) {
	Ftrace *f = new Ftrace(getName(), this, data_source);
	windows.push_back(f);
	f->setVisible(true);
    }
}

ParseCmd Ftrace::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;
    char c[200];
    string s;

    for(int i = 0; i < 10; i++) {
	snprintf(c, sizeof(c), "%d", i+2);
	if(parseString(cmd, c, s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(s, msg);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		Ftrace *ft = new Ftrace(getName(), this, data_source);
		windows.push_back(ft);
		if(j == i) {
		    return ft->parseCmd(s, msg);
		}
	    }
	}
    }

    if((ret = parseParams(cmd, msg)) != COMMAND_PARSED) return ret;

    if(parseCompare(cmd, "Compute")) {
	compute();
    }
    else if(parseString(cmd, "compute", s)) {
	return parseCompute(s, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }   
    else {
	return WaveformWindow::parseCmd(cmd, msg);
    }
    return ret;
}

ParseCmd Ftrace::parseCompute(const string &cmd, string &msg)
{
    const char *args[] = {"az", "slow", "flo", "fhi", "lat", "lon", "wlen",
			"snr", "autoloc", "cursor"};

    int num = (int)sizeof(args)/(int)sizeof(const char *);

    if(unknownArgs(cmd, msg, num, args)) return ARGUMENT_ERROR;

    if( parseGetArg(cmd, "cursor", cursor) ) {
	compute();
	cursor.assign("a");
    }
    else {
	compute();
    }

    return COMMAND_PARSED;
}

ParseCmd Ftrace::parseParams(const string &cmd, string &msg)
{
    string s;
    ParseCmd ret;

    if( parseGetArg(cmd, "az", s) ) {
	az_text->setString(s);
    }
    if( parseGetArg(cmd, "slow", s) ) {
	slo_text->setString(s);
    }
    if( parseGetArg(cmd, "flo", s) ) {
	flo_text->setString(s);
    }
    if( parseGetArg(cmd, "fhi", s) ) {
	fhi_text->setString(s);
    }
    if( parseGetArg(cmd, "lat", s) ) {
	lat_text->setString(s);
    }
    if( parseGetArg(cmd, "lon", s) ) {
	lon_text->setString(s);
    }
    if( parseGetArg(cmd, "wlen", s) ) {
	len_text->setString(s);
    }
    if( parseGetArg(cmd, "snr", s) ) {
	snr_text->setString(s);
    }
    if( parseGetArg(cmd, "autoloc", s) ) {
	if((ret = auto_beam_loc->parseCmd(s, msg)) != COMMAND_PARSED) {
	    return ret;
	}
    }
    return COMMAND_PARSED;
}

ParseVar Ftrace::parseVar(const string &name, string &value)
{
    int i, j;
    double d;
    char c[200];
    string s;

    for(i = 0; i < 10; i++) {
	snprintf(c, sizeof(c), "%d", i+2);
	if(parseString(name, c, s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseVar(s, value);
	    }
	    for(j = (int)windows.size(); j <= i; j++) {
		Ftrace *ft;
		ft = new Ftrace(getName(), this, data_source);
		windows.push_back(ft);
		if(j == i) {
		    return ft->parseVar(s, value);
		}
	    }
	}
    }

    d = -1.;
    if(parseCompare(name, "az")) {
	az_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "slow")) {
	slo_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "flo")) {
	flo_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "fhi")) {
	fhi_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "lat")) {
	lat_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "lon")) {
	lon_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "wlen")) {
	len_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "snr")) {
	snr_text->getDouble(&d);
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "autoloc")) {
	value.assign(auto_beam_loc->state() ? "true" : "false");
	return STRING_RETURNED;
    }
    return WaveformWindow::parseVar(name, value);
}

void Ftrace::parseHelp(const char *prefix)
{
    printf("%scompute\n", prefix);
    printf("%scompute [az=DEG] [slow=SEC/KM] [flo=HZ] [fhi=HZ] [lat=DEG] [lon=DEG] [wlen=SEC] [snr=SNR] [autoloc={true,false}]\n", prefix);
    printf("%saz=DEG\n", prefix);
    printf("%sslow=SEC/KM\n", prefix);
    printf("%sflo=HZ\n", prefix);
    printf("%sfhi=HZ\n", prefix);
    printf("%slat=DEG\n", prefix);
    printf("%slon=DEG\n", prefix);
    printf("%swlen=SEC\n", prefix);
    printf("%ssnr=SNR\n", prefix);
    printf("%sautoloc={true,false}\n", prefix);
}
