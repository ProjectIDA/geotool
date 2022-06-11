/** \file Spectro.cpp
 *  \brief Defines class Spectro.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
#include <dirent.h>
#include "gsl/gsl_fft_real.h"

using namespace std;

#include "Spectro.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "libgio.h"
#include "SpectroParam.h"
#include "widget/ConPlotClass.h"
#include "gobject++/GTimeSeries.h"
#include "DataMethod.h"
#include "BasicSource.h"
#include "Response.h"
#include "gobject++/CssTables.h"
#include "cssio.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "tapers.h"
}

using namespace libgspectro;

namespace libgspectro {

class SpectroWaveformView : public WaveformView
{
    public:
	SpectroWaveformView(const char *name, Component *parent, Spectro *spo,
		InfoArea *ia, Arg *args, int n) :
		WaveformView(name, parent, ia, args, n)
	{
	    sp = spo;
        }

    protected:
	Spectro *sp;

	CssArrivalClass * addArrivalFromKey(CPlotArrivalCallbackStruct *p)
	{
            Password password = getpwuid(getuid());
	    bool made_it = false;
            CssArrivalClass * a=NULL;
            double deltim = -1., delaz = -1., delslo = -1.;
            double azimuth = -1., slow = -1., snr = -1.;
	    double incidence = -1., rect = -1.;

	    if(sp->wp) {
		gvector<Waveform *> ws;
		int i;
		sp->wp->getWaveforms(ws);
		for(i = 0; i < ws.size(); i++) {
		    if(!strcasecmp(ws[i]->sta(), p->w->sta()) &&
			!strcasecmp(ws[i]->chan(), p->w->chan()) &&
			ws[i]->segment(p->time) ) break;
		}
		if(i < ws.size()) {
		    a = sp->wp->makeArrival(ws[i], password, p->time,
				p->name, deltim, azimuth, delaz, slow, delslo,
				incidence, rect, snr, true);
		    if(a) {
			sp->wp->putArrivalWithColor(a, stringToPixel("black"));
		    }
		    made_it = true;
		}
	    }
	    if(!made_it) {
		a = makeArrival(p->w, password, p->time, p->name,
				deltim, azimuth, delaz, slow, delslo,
				incidence, rect, snr, true);
		if(a) {
		    putArrivalWithColor(a, stringToPixel("black"));
		}
	    }
	    return a;
        }
};

} // namespace libgspectro

Spectro::Spectro(const char *name, Component *parent, DataSource *ds) :
		Frame(name, parent, true), DataReceiver(NULL)
{
    init();
    createInterface();
    setDataSource(ds);
}

Spectro::Spectro(const char *name, Component *parent, DataSource *ds,
	bool set_main_window) : Frame(name, parent, true, set_main_window)
{
    init();
    createInterface();
    setDataSource(ds);
}

void Spectro::createInterface(void)
{
    setSize(540, 650);

    fileDialog = NULL;
    saveDialog = NULL;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    compute_button = new Button("Compute", file_menu, this);
//    open_button = new Button("Open...", file_menu, this);
//    save_button = new Button("Save...", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    new_spectro = new Button("New Spectrogram Window...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    view_menu = new Menu("View", menu_bar);

    colors = new Button("Colors...", view_menu, this);
    normalize_toggle = new Toggle("Normalize", view_menu, this, true);
    labels = new Button("Labels...", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    auto_cursor = new Toggle("Auto Cursor", option_menu, this);
    rtd_compute = new Toggle("RTD Compute", option_menu, this);
    rtd_compute->setVisible(false);

    parameters = new Button("Parameters...", option_menu, this);
    bin_average_toggle = new Toggle("Bin Average", option_menu, this);
    bin_average_toggle->setCommandString("Compute");
    log_data_toggle = new Toggle("Log", option_menu, this);
    log_data_toggle->set(true, false);
    log_data_toggle->setCommandString("Compute");
    instrument_toggle = new Toggle("Instrument Corr", option_menu, this);
    instrument_toggle->setCommandString("Compute");

    int n;
    Arg args[30];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNmode, 1); n++;
    XtSetArg(args[n], XtNtickmarksInside, False); n++;
    XtSetArg(args[n], XtNheight, 400); n++;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_HMS); n++;
    XtSetArg(args[n], XtNextraXTickmarks, False); n++;
    XtSetArg(args[n], XtNextraYTickmarks, False); n++;
    XtSetArg(args[n], XtNclearOnScroll, False); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    plot1 = new ConPlotClass("plot1", pane, args, n);

    plot1->addActionListener(this, XtNphaseLineDragCallback);
    plot1->addActionListener(this, XtNphaseLineCallback);
    plot1->addActionListener(this, XtNlimitsCallback);
    plot1->addActionListener(this, XtNselectBarCallback);

    n = 0;
    XtSetArg(args[n], XtNdisplayTags, False); n++;
    XtSetArg(args[n], XtNdrawYLabels, False); n++;
    plot2 = new SpectroWaveformView("plot2", pane, this, info_area, args, n);

    plot2->addActionListener(this, XtNdoubleLineDragCallback);
    plot2->addActionListener(this, XtNdoubleLineCallback);
    plot2->addActionListener(this, XtNlimitsCallback);

    help_menu = new Menu("Help", menu_bar);
    help = new Button("Spectro Help...", help_menu, this);
    menu_bar->setHelpMenu(help_menu);

    parameter_window = new SpectroParam("Spectrogram Parameters", this);
    parameter_window->setCommandString("Compute");
    parameter_window->addActionListener(this);
    print_window = NULL;
    colors_window = NULL;
    labels_window = NULL;

    addPlugins("Spectro", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
    }
}

void Spectro::init(void)
{
    data_source = NULL;
    wp = NULL;
    init_colors = true;
    n_windows = 0;
    nf = 0;
    x = NULL;
    y = NULL;
    data = NULL;

    window_length = 0.;
    window_overlap = 0.;
    f_min = 0.;
    f_max = 0.;
    min = 0.;
    max = 0.;
    samprate = 0.;
    exc = -1.e+30;
    normalize = false;
    auto_color_limits = false;
    bin_average = false;
    instrument_corr = false;
    add_callbacks = true;

    memset(sta, 0, sizeof(sta));
    memset(chan, 0, sizeof(sta));
    memset(net, 0, sizeof(sta));
    time = 0.;
    tlen = 0.;
    winpts = 0;
    overlap = 0;
    lofreq = 0.0;
    hifreq = 0.0;

    memset(ts_label, 0, sizeof(ts_label));
    w_id = 0;
    selection_warning = 0;
    ignore_limits_callback = false;
}

Spectro::~Spectro(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void Spectro::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    void *cb_data = action_event->getCalldata();

    if(!strcmp(cmd, "plot1")) {
	if(!strcmp(reason, XtNphaseLineDragCallback) ||
	    !strcmp(reason, XtNphaseLineCallback))
	{
	    cursorPlot1((AxesCursorCallbackStruct *)cb_data);
	}
	else if(!strcmp(reason, XtNlimitsCallback)) {
//	    tieLimits(plot2, (AxesLimitsCallbackStruct *)cb_data);
	}
	else if(!strcmp(reason,XtNselectBarCallback)) {
	    if(!colors_window) {
		colors_window = new ColorSelection("Spec Color Selection",
					this, this);
	    }
	    colors_window->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "plot2")) {
	if(!strcmp(reason, XtNdoubleLineCallback) ||
	    !strcmp(reason, XtNdoubleLineDragCallback))
	{
	    cursorPlot2((AxesCursorCallbackStruct *)cb_data);
	}
	else if(!strcmp(reason, XtNlimitsCallback)) {
//	    tieLimits(plot1, (AxesLimitsCallbackStruct *)cb_data);
	}
    }
#ifdef _OLD__
    else if(!strcmp(cmd, "Open...")) {
	open();
    }
    else if(!strcmp(cmd, "Save...")) {
	char *file;

	if(fileDialog == NULL) {
	    fileDialog = new FileDialog("Spectrogram Save", this, FILE_ONLY,
				".", (char *)"*.spdisc", "Save");
	}
	else {
	    fileDialog->setVisible(true);
	}
	if((file = fileDialog->getFile()) != NULL) {
	    save(file);
	    XtFree(file);
	}
	fileDialog->setVisible(false);
    }
#endif
    else if(!strcmp(cmd, "Colors...")) {
	if(!colors_window) {
	    colors_window =
		new ColorSelection("Spec Color Selection", this, this);
	}
	colors_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Labels...")) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Spectrogram Labels", this, plot1);
	}
	labels_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Parameters...")) {
	parameter_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
	if(auto_cursor->state()) {
	    auto_cursor->set(false, false);
	}
	if(rtd_compute->state()) {
	    rtd_compute->set(false, false);
	}
    }
    else if(!strcmp(cmd, "Compute")) {
//	if(isVisible()) {
	    selection_warning = 0;
	    compute();
//	}
    }
    else if(!strcmp(cmd, "Auto Cursor")) {
	if(wp) {
	    if(auto_cursor->state()) {
		wp->addActionListener(this, XtNdoubleLineCallback);
		wp->addActionListener(this, XtNdoubleLineScaleCallback);

	    }
	    else {
		wp->removeActionListener(this, XtNdoubleLineCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
    }
    else if(!strcmp(reason, "RTDUpdate")) {
        if(rtd_compute->state()) {
            compute(false);
        }
    }
    else if(action_event->getSource() == wp) {
        if( (!strcmp(reason, XtNdoubleLineCallback) ||
                !strcmp(reason, XtNdoubleLineScaleCallback))
                && auto_cursor->state())
        {
	    AxesCursorCallbackStruct *a = (AxesCursorCallbackStruct *)cb_data;
	    if(a->label[0] == 'a') {
		compute(false);
	    }
        }
    }
    else if(!strcmp(cmd, "Normalize")) {
//	if(isVisible()) {
	    if(!colors_window) {
		colors_window =
		    new ColorSelection("Spec Color Selection", this, this);
	    }
	    colors_window->setAutoLimits(true);
	    selection_warning = 0;
	    compute();
//	}
    }
    else if(!strcmp(cmd, "New Spectrogram Window...")) {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    Spectro *s = new Spectro(getName(), this, data_source);
	    windows.push_back(s);
	    s->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "RTD Compute")) {
	selection_warning = 0;
/*
	if((w = localWidget(s->w, "*_RTD_Form*main_plot"))) {
//	    if(XmToggleButtonGetState(widget)) {
	    if(rtd_toggle->state()) {
		XtAddCallback(w, XtNrtdComputeCallback,
                        (XtCallbackProc)Spectro_rtdCompute, (XtPointer)this);
	    }
	    else {
		XtRemoveCallback(w, XtNrtdComputeCallback,
                        (XtCallbackProc)Spectro_rtdCompute, (XtPointer)this);
	    }
	}
*/
    }
    else if(!strcmp(cmd, "Spec Color Selection")) {
	if( ((ColorSelectionStruct *)cb_data)->reason == LIMITS_APPLY) {
	    compute();
	}
	else {
	    plot1->setColors(colors_window->getPixels(),
			colors_window->numColors());
	    plot1->colorLines(colors_window->numLines(),
			colors_window->colorLines());
	}
    }
    else if(!strcmp(cmd, "Spectro Help...")) {
	showHelp("Spectrogram Help");
    }
}

ParseCmd Spectro::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    for(int i = 0; i < 10; i++) {
	ostringstream os;
	os << i+2;
	if(parseString(cmd, os.str(), c)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(c, msg);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		Spectro *sp = new Spectro(getName(), this, data_source);
		windows.push_back(sp);
		if(j == i) {
		    return sp->parseCmd(c, msg);
		}
	    }
	}
    }

    if(parseArg(cmd, "Colors", c)) {
	if(!colors_window) {
	    colors_window =
		new ColorSelection("Spec Color Selection", this, this);
	}
	return colors_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "Labels", c)) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Spectrogram Labels", this, plot1);
	}
	return labels_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "print_window", c)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print Spectrogram", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "Parameters", c) || parseString(cmd,"Param",c))
    {
	return parameter_window->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Auto_Window_Parameters", c)) {
	return parameter_window->parseCmd(cmd, msg);
    }
    else if(parseArg(cmd, "lo_freq", c)) {
	return parameter_window->parseCmd(cmd, msg);
    }
    else if(parseArg(cmd, "hi_freq", c)) {
	return parameter_window->parseCmd(cmd, msg);
    }
    else if(parseArg(cmd, "window_length", c)) {
	return parameter_window->parseCmd(cmd, msg);
    }
    else if(parseArg(cmd, "window_overlap", c)) {
	return parameter_window->parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "Close")) {
	setVisible(false);
	if(auto_cursor->state()) {
	    auto_cursor->set(false, false);
	}
	if(rtd_compute->state()) {
	    rtd_compute->set(false, false);
	}
    }
    else if(parseCompare(cmd, "compute")) {
	selection_warning = 0;
	compute();
    }
    else if(parseArg(cmd, "normalize", c)) {
	normalize_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "bin_average", c)) {
	bin_average_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "log", c)) {
	log_data_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "instrument_corr", c) ||
		parseArg(cmd, "instrument", c))
    {
	instrument_toggle->parseCmd(c, msg);
    }
    else if(parseString(cmd, "plot1", c)) {
	return plot1->parseCmd(c, msg);
    }
    else if(parseString(cmd, "plot2", c)) {
	return plot2->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = pluginParse(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else {
	return Frame::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar Spectro::parseVar(const string &name, string &value)
{
    ParseVar ret;
    string c;

    if(parseCompare(name, "n_windows")) {
	parsePrintInt(value, n_windows);
    }
    else if(parseCompare(name, "nf")) {
	parsePrintInt(value, nf);
    }
    else if(parseCompare(name, "exc")) {
	parsePrintDouble(value, exc);
    }
    else if(parseArray(name, "x", n_windows, x, value, &ret) ) {
	return ret;
    }
    else if(parseArray(name, "y", nf, y, value, &ret) ) {
	return ret;
    }
    else if(parseArray(name, "pow", n_windows*nf, data, value, &ret)) {
	return ret;
    }
    else if(parseString(name, "plot1", c)) {
        return plot1->parseVar(c, value);
    }
    else if(parseString(name, "wplot", c)) {
        return plot2->parseVar(c, value);
    }
    else {
	return Frame::parseVar(name, value);
    }
    return STRING_RETURNED;
}

void Spectro::parseHelp(const char *prefix)
{
    char p[200];
    printf("%scompute\n", prefix);
    printf("%snormalize=(true,false)\n", prefix);
    printf("%sbin_average=(true,false)\n", prefix);
    printf("%slog=(true,false)\n", prefix);
    printf("%sinstrument_corr=(true,false)\n", prefix);

    printf("%sauto_window_parameters=(true,false)\n", prefix);
    printf("%slo_freq=HZ\n", prefix);
    printf("%shi_freq=HZ\n", prefix);
    printf("%swindow_overlap=SECONDS\n", prefix);
    printf("%swindow_length=SECONDS\n", prefix);
    printf("%ssave_file=FILENAME\n", prefix);
    snprintf(p, sizeof(p), "%slabels.", prefix);
    AxesLabels::parseHelp(p);
    printf("%sprint.help\n", prefix);
}

void Spectro::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, XtNdoubleLineCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
		wp->removeActionListener(this, "RTDUpdate");
            }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp) {
		if(auto_cursor->state()) {
		    wp->addActionListener(this, XtNdoubleLineCallback);
		    wp->addActionListener(this, XtNdoubleLineScaleCallback);
		}
		if(wp->isRealTime()) {
		    wp->addActionListener(this, "RTDUpdate");
		    rtd_compute->setVisible(true);
		}
	    }
	}
	else wp = NULL;
    }
}

void Spectro::compute(bool warning)
{
    int num_waveforms, windowed;
    GTimeSeries *ts;
    gvector<Waveform *> wvec;
    Waveform *w;

    if(!data_source) {
        showWarning("Spectro compute: no data source.");
        return;
    }
    if(!colors_window) {
	colors_window = new ColorSelection("Spec Color Selection", this, this);
    }

    Free(data);
    Free(x);
    Free(y);
    nf = 0;
    n_windows = 0;

    bin_average = bin_average_toggle->state();
    instrument_corr = instrument_toggle->state();
    normalize = normalize_toggle->state();

    if(plot1->getZoomLevel()) {
	plot1->unzoom();
    }
    plot1->clearEntries();

/*
	if(sp->add_callbacks) {
	    XtAddCallback((Widget)main_plot, XtNdoubleLineCallback,
			(XtCallbackProc)DoubleLine, sp->name);
	    XtAddCallback((Widget)main_plot, XtNdoubleLineScaleCallback,
			(XtCallbackProc)DoubleLine, sp->name);
	    sp->add_callbacks = false;
	}
*/

    num_waveforms = data_source->getSelectedWaveforms("a", wvec);
    windowed = 1;
    if(num_waveforms <= 0)
    {
	windowed = 0;
	num_waveforms = data_source->getSelectedWaveforms(wvec);
    }

    if(num_waveforms <= 0)
    {
	if(warning) showWarning("No selected waveforms.");
	selection_warning++;
	return;
    }
    else if(num_waveforms > 1 && warning)
    {
	showWarning("More than one waveform selected.");
	selection_warning++;
    }

    w = wvec[0];
    w_id = w->getId();

    if(windowed) {
	ts = w->ts->subseries(w->dw[0].d1->time(), w->dw[0].d2->time());
    }
    else {
	ts = new GTimeSeries(w->ts);
    }
    fg = w->fg;

    ts->setSta(w->sta());
    ts->setChan(w->chan());

    vector<Response *> *rsp = NULL;

    if(instrument_corr) {
	if( !(rsp = BasicSource::getResponse(w->ts, true)) ) {
	    ts->deleteObject();
	    return;
	}
    }

    if(!spectro(w, ts, windowed, rsp)) {
	ts->deleteObject();
    }
}

bool Spectro::spectro(Waveform *w, GTimeSeries *ts, bool windowed,
			vector<Response *> *rsp)
{
    int i, j, k, l, dk, n, n2, npts, if1, if2, window_pts, i1, window_width;
    double a, df, tbeg, range, calper, calib, taper_norm, re, im, fre, fim;
    double *f=NULL, *real=NULL, *imag=NULL;
    float *s = NULL, *t = NULL, *taper=NULL;
    char msg[20];
    bool auto_param;

    i1 = 0;
    for(i = 1; i < ts->size(); i++)
    {
	if(fabs(ts->segment(i)->tdel() - ts->segment(0)->tdel())/
		ts->segment(0)->tdel() > .001)
	{
	    showWarning("Sample rate change in %s/%s.", w->sta(), w->chan());
	    return false;
	}
    }

    samprate = 1./ts->segment(0)->tdel();
    if(!(t = (float *)mallocWarn(ts->length()*sizeof(float)))) return false;

    ts->demean();

    npts = 0;
    for(i = 0; i < ts->size(); i++)
    {
	memcpy(t+npts, ts->segment(i)->data,
		ts->segment(i)->length()*sizeof(float));
	npts += ts->segment(i)->length();
    }
    tbeg = ts->segment(0)->tbeg();

    if(npts < 16) {
	showWarning("Spectrogram: time window too short.");
	Free(t);
	return false;
    }

    a = fabs(t[0]);
    for(j = 1; j < npts; j++) {
	if(fabs(t[j]) > a) a = fabs(t[j]);
    }
    if(a == 0.) {
	showWarning("No data.");
	Free(t);
	return false;
    }

    for(j = 0; j < npts; j++) t[j] /= a;

    parameter_window->getBool("Auto Window Parameters", &auto_param);

    parameter_window->ignoreTextInput(true); /* ignore textInput */

    if(auto_param || !parameter_window->getDouble("window_length_text", &a)
		|| (int)(a*samprate+.5) < 4)
    {
	a = (npts > 20) ? npts/20 : 4;
	if(a < 4) a = 4;
	a /= samprate;
	ftoa((double)a, 2, 0, msg, 50);
	parameter_window->setString("window_length_text", msg, false);
    }
    window_length = a;
    window_width = (int)(a*samprate+.5);

    if(auto_param || !parameter_window->getDouble("window_overlap_text", &a)
		|| a < 0.)
    {
	a = .9*window_width/samprate;
	ftoa((double)a, 2, 0, msg, 50);
	parameter_window->setString("window_overlap_text", msg, false);
    }
    overlap = (int)(a*samprate+.5);
    if(overlap >= window_width)
    {
	overlap = window_width - 1;
	a = overlap/samprate;
	ftoa((double)a, 2, 0, msg, 50);
	parameter_window->setString("window_overlap_text", msg, false);
    }
    window_overlap = a;

    if(auto_param || !parameter_window->getDouble("lo_freq_text", &a))
    {
	a = 0.;
	parameter_window->setString("lo_freq_text", "0.", false);
    }

    f_min = a;
    if(auto_param || !parameter_window->getDouble("hi_freq_text", &a))
    {
	a = .5*samprate;
	ftoa((double)a, 2, 0, msg, 50);
	parameter_window->setString("hi_freq_text", msg, false);
    }
    f_max = a;

    window_pts = window_width + 1;
    for(n = 2; n < window_pts; n *= 2);
    n2 = n/2;
    df = samprate/(float)n;
    if1 = (int)(f_min/df);
    if2 = (int)(f_max/df + .5);
    if(if1 < 0) if1 = 0;
    if(if1 > n/2) if1 = n/2;
    if(if2 < 0) if2 = 0;
    if(if2 > n/2) if2 = n/2;
    nf = if2 - if1 + 1;

    if(!(y = (double *)mallocWarn(nf*sizeof(double)))) {
	Free(t);
	return false;
    }
    for(i = 0; i < nf; i++) y[i] = (if1+i)*df;

    if(!(f = (double *)mallocWarn(n*sizeof(double))) ||
	!(taper = (float *)mallocWarn(window_pts*sizeof(float))) )
    {
	Free(t); Free(y); Free(f);
	return false;
    }
    dk = window_pts - overlap;
    for(k = n_windows = 0; k + window_width < npts; k += dk) n_windows++;

    if(!(s = (float *)mallocWarn(n_windows*nf*sizeof(float)))) {
	Free(f); Free(t); Free(y); Free(taper);
	return false;
    }
    if(!(x = (double *)mallocWarn(n_windows*sizeof(double)))) {
	Free(f); Free(s); Free(t); Free(y); Free(taper);
	return false;
    }
    for(i = 0; i < n_windows*nf; i++) s[i] = exc;

    calper = ts->segment(0)->calper();
    calib = ts->segment(0)->calib();
    if(!calib) calib = 1.;

    if(rsp) {
	if(!(real = (double *)mallocWarn(nf*sizeof(double))) ||
	   !(imag = (double *)mallocWarn(nf*sizeof(double))))
	{
	    Free(f); Free(s); Free(t); Free(y); Free(real); Free(taper);
	    return false;
	}
	Response::compute(rsp, f_min, f_max, calib, calper, nf, real, imag);
    }
    bool remove_calib = ts->getMethod("CalibData") ? true : false;

    for(i = 0; i < window_pts; i++) taper[i] = 1.;
    taper_norm = Taper_hann(taper, window_pts);

    setCursor("hourglass");
    for(k = l = 0; k + window_width < npts; k += dk, l++)
    {
	i = i1 + k;
	x[l] =  ts->time((int)(i+.5*window_width));

	if(ts->getSegment(i) != ts->getSegment(i+window_width))
	{
	    /* this window crosses boundary between two segments. */
	    continue;
	}

	for(i = 0; i < window_pts && t[k+i] == 0.; i++);
	if(i == window_pts) {
	    /* skip zero data window */
	    continue;
	}

	for(i = 0; i < window_pts; i++) f[i] = t[k+i];
	for(i = window_pts; i < n; i++) f[i] = 0.;

	if(remove_calib && calib) {
	    for(i = 0; i < window_pts; i++) f[i] /= calib;
	}

	for(i = 0; i < window_pts; i++) f[i] *= taper[i];

	gsl_fft_real_radix2_transform(f, 1, n);

	/* compute power */
	if(rsp) {
	    // divide by the response
	    for(j = if1, i = 0; j <= if2; j++, i++) {
		if(j == 0 || j == n2) fim = 0.;
		else fim = f[n-j];
		fre = f[j];

		a = real[i]*real[i] + imag[i]*imag[i];
		if(a == 0.) a = 1.;
		re = (fre*real[i] + fim*imag[i])/a;
		im = (fim*real[i] - fre*imag[i])/a;
		f[j] = re*re + im*im;
	    }
	}
	else {
	    for(j = if1, i = 0; j <= if2; j++, i++) {
		if(j == 0 || j == n2) fim = 0.;
		else fim = f[n-j];
		fre = f[j];
		f[j] = fre*fre + fim*fim;
	    }
	}
	/* compensate for taper and scale for power */
	double scale = taper_norm*2.*ts->segment(0)->tdel()/n;
	if(bin_average)
	{
	    if(if1 <= if2) {
		j = if1;
		min = scale*f[j];
		max = min;
	    }
	    for(j = if1, i = 0; j <= if2; j++, i++)
	    {
		s[i*n_windows+l] = scale*f[j];
		if(s[i*n_windows+l] < min) min = s[i*n_windows+l];
		if(s[i*n_windows+l] > max) max = s[i*n_windows+l];
	    }
	    range = max - min;

	    for(j = if1, i = 0; j <= if2; j++, i++) {
		s[i*n_windows+l] = (s[i*n_windows+l] - min)/range;
	    }
	}
	else
	{
	    for(j = if1, i = 0; j <= if2; j++, i++)
	    {
		s[i*n_windows+l] = scale*f[j];

		/* fix spurious values */
		if (s[i*n_windows+l] == 0.0 && l > 0) {
		    s[i*n_windows+l] = s[i*n_windows+l-1];
		}
	    }
	}
    }
    Free(f); Free(t); Free(taper);
    Free(real); Free(imag);

    for(i = 0; i < n_windows*nf && s[i] == exc; i++);
    if(i == n_windows*nf) {
	/* no data in any windows */
	showWarning("No data.");
	setCursor("default");
	return false;
    }

    if(log_data_toggle->state()) {
	/* dB relative meters**2
	 */
	for(i = 0; i < n_windows*nf; i++) if (s[i] != 0 && s[i] != exc) {
	    s[i] = 10.*log10(s[i]) - 180.;
	}
    }

    sMinMax(s, n_windows*nf, exc, &min, &max);

    if(normalize)
    {
	range = max - min;
	for(i = 0; i < n_windows*nf; i++) if(s[i] != exc) {
	    s[i] = (s[i] - min)/range;
	}
	sMinMax(s, n_windows*nf, exc, &min, &max);
    }

    colors_window->updateLimits(&min, &max, 3);

    if( !colors_window->autoLimits() )
    {
	for(i = 0; i < n_windows*nf; i++) {
	    if(s[i] < min) s[i] = exc;
	    if(s[i] > max) s[i] = exc;
	}
    }

/*
    n = (num_colors > 0) ? num_colors : num_stipples;

    num_lines = n+1;
*/

    stringcpy(sta, w->sta(), sizeof(sta));
    stringcpy(chan, w->chan(), sizeof(chan));
    stringcpy(net, w->net(), sizeof(net));
    time = tbeg;
    winpts = window_width;
    overlap = (int) ((overlap*100.)/window_width);
    lofreq = f_min;
    hifreq = f_max;
    tlen = npts/samprate;

    data = s;

    snprintf(ts_label, sizeof(ts_label), "%s/%s", w->sta(), w->chan());

    parameter_window->ignoreTextInput(false);
    colors_window->ignoreTextInput(false);

    drawSpectro(w, ts);

    setCursor("default");

    return true;
}

void Spectro::drawSpectro(Waveform *w, GTimeSeries *ts)
{
    int i, n;
    double x_min, x_max, cursor_x, d;

    if(!data_source) {
        cerr << "Spectro.drawSpectro: no data source." << endl;
        return;
    }
    if(!colors_window) {
	colors_window = new ColorSelection("Spec Color Selection", this, this);
    }
//	makeLines(sp, lines);

    if(plot1->getZoomLevel()) {
	plot1->unzoom();
    }
    plot1->clearEntries();

    plot1->setColors(colors_window->getPixels(), colors_window->numColors());
    n = n_windows*nf;
    colors_window->computeBins(2*colors_window->numColors(), n, data, exc);
/*** if log_y, set sp->y */
    plot1->input(ts_label, n_windows, nf, x, y, data, exc,
		colors_window->numLines(), colors_window->colorLines(),
		0., 0., true, false, true, false, 0);

    plot1->getXLimits(&x_min, &x_max);

    plot2->clear();
	
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    cvector<CssOriginClass> origins;
    data_source->getArrivalsOnWaveform(w, arrivals);
    data_source->getTable(assocs);
    data_source->getTable(origins);
    for(i = 0; i < arrivals.size(); i++) {
	plot2->putArrivalWithColor(arrivals[i], stringToPixel("black"));
    }
    for(i = 0; i < assocs.size(); i++) {
	plot2->putTable(assocs[i], false);
    }
    for(i = 0; i < origins.size(); i++) {
	plot2->putTable(origins[i], false);
    }

    plot2->setTimeLimits(x_min, x_max);

    CPlotInputStruct input;
    input.display_t0 = ts->tbeg();
    input.color = fg;
    input.chan.assign(chan);
    input.tag = sta + string("/") + chan;
    plot2->addTimeSeries(ts, &input);

    n = (int)(.2*n_windows);
    cursor_x = x[n];
    plot1->positionPhaseLine("", cursor_x, false);
    d = .5*window_length;
    plot2->positionDoubleLine("b", cursor_x-d,cursor_x+d, false);

    if(wp) {
	wp->positionDoubleLine("b", cursor_x-d, cursor_x+d, true);
    }
}

void Spectro::open(void)
{
    char *file;

    if(fileDialog == NULL) {
	fileDialog = new FileDialog("Open file", this, EXISTING_FILE, ".",
				(char *)"*.spdisc");
    }
    else {
	fileDialog->setVisible(true);
    }
    if((file = fileDialog->getFile()) != NULL)
    {
//	readFile(file);
	XtFree(file);
    }
}

void Spectro::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Spectrogram", this, this);
    }
    print_window->setVisible(true);
}

void Spectro::print(FILE *fp, PrintParam *p)
{
    char title[200], xlab[200], tlab[200];
    double f, bot;
    AxesParm *a, *a2;
    Dimension h1, h2;
    Arg args[1];

    p->right -= 1.;        /* leave space for the color bar */
   
    XtSetArg(args[0], XtNheight, &h1);
    plot1->getValues(args, 1);

    XtSetArg(args[0], XtNheight, &h2);
    plot2->getValues(args, 1);

    f = (double)h1/(double)(h1+h2);

    bot = p->bottom;
    p->bottom = p->top - f*(p->top - p->bottom);

    timeEpochToString(time, tlab, 200, GSE20);

    snprintf(title, 200, "%s/%s %s\nwindow-length %.1f  overlap %.1f",
	sta, chan, tlab, window_length, window_overlap);
    p->top_title = title;

    xlab[0] = '\0';
    if(bin_average) {
	strcat(xlab, " bin-averaged");
    }
    if(instrument_corr) {
	strcat(xlab, " instrument-corrected");
    }
    p->x_axis_label = NULL;

    p->y_axis_label = (char *)"Frequency (Hz)";
    a = plot1->hardCopy(fp, p, NULL);

    if(a == NULL) {
	return;
    }
    a->auto_x = false;
    p->top = p->bottom;
    p->bottom = bot;
    p->top_title = NULL;
    p->y_axis_label = NULL;
    p->x_axis_label = xlab;
    snprintf(xlab, 200, "Time (s) from %s", tlab);
    if((a2 = plot2->hardCopy(fp, p, a)) == NULL)
    {
	free(a);
	return;
    }
    free(a);
    free(a2);
}

void Spectro::sMinMax(float *s, int npts, float exc, double *min, double *max)
{
    int i;

    for(i = 0; i < npts && s[i] == exc; i++);
    if(i < npts) {
	*min = s[i];
	*max = s[i];
    }
    for(i = 0; i < npts; i++) if(s[i] != exc) {
	if(*min > s[i]) *min = s[i];
	if(*max < s[i]) *max = s[i];
    }
}

void Spectro::cursorPlot1(AxesCursorCallbackStruct *p)
{
    double d, scaled_x, dmin;
    int i;

    scaled_x = p->scaled_x;
    if(n_windows > 0) {
	scaled_x = x[0];
	dmin = fabs(p->scaled_x - x[0]);
	for(i = 0; i < n_windows; i++) {
	    d = fabs(p->scaled_x - x[i]);
	    if(d < dmin) {
		dmin = d;
		scaled_x = x[i];
	    }
	}
    }

    d = .5*window_length;
    if(p->reason != AXES_PHASE_LINE_DRAG)
    {
	plot1->positionPhaseLine("", scaled_x, false);
    }
    plot2->positionDoubleLine("b", scaled_x-d, scaled_x+d, false);
    if(wp) {
	wp->positionDoubleLine("b",scaled_x-d, scaled_x+d, true);
    }
}

void Spectro::cursorPlot2(AxesCursorCallbackStruct *p)
{
    double d, scaled_x, dmin;
    int i;

    scaled_x = p->scaled_x + .5*window_length;
    if(n_windows > 0) {
	scaled_x = x[0];
	dmin = fabs(p->scaled_x + .5*window_length - x[0]);
	for(i = 0; i < n_windows; i++) {
	    d = fabs(p->scaled_x + .5*window_length - x[i]);
	    if(d < dmin) {
		dmin = d;
		scaled_x = x[i];
	    }
	}
    }

    d = .5*window_length;
    plot1->positionPhaseLine("", scaled_x, false);
    if(p->reason != AXES_DOUBLE_LINE_DRAG)
    {
	plot2->positionDoubleLine("b", scaled_x-d, scaled_x+d, false);
    }
    if(wp) {
	wp->positionDoubleLine("b", scaled_x-d, scaled_x+d, true);
    }
}

void Spectro::tieLimits(AxesClass *plot, AxesLimitsCallbackStruct *a)
{
    if(ignore_limits_callback) return;
    ignore_limits_callback = true;

    if(a->x_margins) {
	plot->setMargins(false, true, a->left, a->right, 0, 0);
    }
    plot->setXLimits(a->x_min, a->x_max);
    ignore_limits_callback = false;
}

#ifdef _OLD__
void Spectro::save(char *file)
{
    DIR *dirp;
    FILE *fp;
    char spdisc_file[MAXPATHLEN+1], path[MAXPATHLEN+1];
    const char *access = "w";
    int len, foff;
    struct stat buf;

    if(file == NULL || file[0] == '\0' || file[(int)strlen(file)-1] == '/')
    {
	showWarning("saveSpectro: No file specified.");
	return;
    }
    if((dirp = opendir(file)) != NULL) {
	closedir(dirp);
	showWarning("%s is a directory.", file);
	return;
    }
    stringcpy(spdisc_file, file, sizeof(spdisc_file));

    len = (int)strlen(spdisc_file);
    if(len < 8 || strcmp(spdisc_file+len-7, ".spdisc")) {
	strcat(spdisc_file, ".spdisc");
    }

    if(!stat(spdisc_file, &buf)) {
	char ques[MAXPATHLEN+100];
	snprintf(ques, sizeof(ques), "%s exists.", spdisc_file);
	int answer = Question::askQuestion("Save Spectrogram Warning", this,
				ques, "Append", "Overwrite", "Cancel");
	if(answer == 3) return;
	if(answer == 1) access = "a";
    }

    stringcpy(path, spdisc_file, sizeof(path));
    len = (int)strlen(path);
    path[len-4] = '\0';	/* make suffix ".sp" */
    if((fp = fopen(path, access)) == NULL) {
//	fileWarning(path);
	return;
    }
    foff = ftell(fp);
    writeData(fp);
    fclose(fp);

    writeSpdisc(spdisc_file, foff, access);
}

void Spectro::writeSpdisc(const char *spdisc_file, int foff, const char *acc)
{
    char dir[MAXPATHLEN+1], prefix[MAXPATHLEN+1];
    int i, len;
    FILE *fp;
    DateTime dt;
    SPDISC30 spdisc, spdisc_null30 = SPDISC_NULL30;
	
    if((fp = fopen(spdisc_file, acc)) == NULL) {
//	fileWarning(spdisc_file);
	return;
    }
    memcpy(&spdisc, &spdisc_null30, sizeof(SPDISC30));

    len = (int)strlen(spdisc_file);
    for(i = len-8; i >= 0 && spdisc_file[i] != '/'; i--);
    strncpy(prefix, spdisc_file+i+1, len-8-i);
    prefix[len-8-i] = '\0';
    if(i >= 0) {
	strncpy(dir, spdisc_file, i+1);
	dir[i+1] = '\0';
    }
    else {
	stringcpy(dir, ".", sizeof(dir));
    }
    spdisc.time = time;
    timeEpochToDate(spdisc.time, &dt);
    spdisc.jdate = timeJDate(&dt);
    spdisc.tlen = tlen;
    stringcpy(spdisc.sta, sta, sizeof(spdisc.sta));
    stringcpy(spdisc.chan, chan, sizeof(spdisc.chan));
    spdisc.winpts = winpts;
    spdisc.overlap = overlap;
    spdisc.nwin = n_windows;
    spdisc.lofreq = lofreq;
    spdisc.hifreq = hifreq;
    spdisc.nf = nf;
    spdisc.spid = cssGetNextId(NULL, "spid", prefix);
    stringcpy(spdisc.datatype, "b#", sizeof(spdisc.datatype));
    stringcpy(spdisc.dir, dir, sizeof(spdisc.dir));
    spdisc.foff = foff;
    snprintf(spdisc.dfile, 33, "%s.sp", prefix);
    stringcpy(spdisc.lddate, timeLoadDate(), sizeof(spdisc.lddate));

    if(cssioWriteSpdisc(fp, &spdisc)) {
	showWarning("Error writing to %s\n%s", spdisc_file, cssioGetErrorMsg());
    }
    fclose(fp);
}

void Spectro::writeData(FILE *fp)
{
    int i, n;
    GTimeSeries *ts;
    gvector<Waveform *> wvec;
//  int file_format_version = 11052002;
//  int file_format_version = 20040322;
//  int file_format_version = 20040323;
    int file_format_version = 20040324;
	
    if(!colors_window) {
	colors_window =
		new ColorSelection("Spec Color Selection", this, this);
    }
    fwrite(&file_format_version, sizeof(int), 1, fp);
    n = colors_window->numColors();
    fwrite(&n, sizeof(int), 1, fp);
    n = colors_window->numStipples();
    fwrite(&n, sizeof(int), 1, fp);
    fwrite(colors_window->red(), sizeof(int), 256, fp);
    fwrite(colors_window->green(), sizeof(int), 256, fp);
    fwrite(colors_window->blue(), sizeof(int), 256, fp);
    n = colors_window->numLines();
    fwrite(&n, sizeof(int), 1, fp);
    fwrite(colors_window->dist(), sizeof(float), n, fp);

    fwrite(&n_windows, sizeof(int), 1, fp);
    fwrite(x, sizeof(double), n_windows, fp);
    fwrite(&nf, sizeof(int), 1, fp);
    fwrite(y, sizeof(double), nf, fp);
    fwrite(data, sizeof(float), n_windows*nf, fp);
    fwrite(&window_length, sizeof(double), 1, fp);
    fwrite(&window_overlap, sizeof(double), 1, fp);
    fwrite(&f_min, sizeof(double), 1, fp);
    fwrite(&f_max, sizeof(double), 1, fp);
    fwrite(&samprate, sizeof(double), 1, fp);
    fwrite(&min, sizeof(double), 1, fp);
    fwrite(&max, sizeof(double), 1, fp);
    fwrite(&normalize, sizeof(bool), 1, fp);
    fwrite(&auto_color_limits, sizeof(bool), 1, fp);
    fwrite(&bin_average, sizeof(bool), 1, fp);
    fwrite(&instrument_corr, sizeof(bool), 1, fp);
    fwrite(sta, 1, 10, fp);
    fwrite(chan, 1, 9, fp);
    fwrite(net, 1, 9, fp);
    fwrite(&time, sizeof(double), 1, fp);
    fwrite(&tlen, sizeof(float), 1, fp);
    fwrite(&winpts, sizeof(int), 1, fp);
    fwrite(&overlap, sizeof(int), 1, fp);
    fwrite(&lofreq, sizeof(float), 1, fp);
    fwrite(&hifreq, sizeof(float), 1, fp);
    fwrite(ts_label, 1, 20, fp);

    if(plot2->getWaveforms(wvec, false) != 1) {
	showWarning("SpectroWrite: error getting waveform.");
	return;
    }
    ts = wvec[0]->ts;
    i = ts->size();
    fwrite(&i, sizeof(int), 1, fp);
    for(i = 0; i < ts->size(); i++) {
	GSegment *s = ts->segment(i);
	double d = s->tbeg();
	fwrite(&d, sizeof(double), 1, fp);
	d = s->tdel();
	fwrite(&d, sizeof(double), 1, fp);
	int n = s->length();
	fwrite(&n, sizeof(int), 1, fp);
	fwrite(s->data, sizeof(float), s->length(), fp);
    }
    gvector<CssArrivalClass *> &arrivals = plot2->getArrivals(&arrivals);
    int narrivals = arrivals.size();
    fwrite(&narrivals, sizeof(int), 1, fp);
    for(i = 0; i < narrivals; i++) {
	fwrite(arrivals[i]->sta, sizeof(arrivals[i]->sta), 1, fp);
	fwrite(arrivals[i]->chan, sizeof(arrivals[i]->chan), 1, fp);
	fwrite(&arrivals[i]->arid, sizeof(long), 1, fp);
    }
}

void Spectro::readData(FILE *fp)
{
    char msg[100];
    int i, len, version, nseg, narrivals = 0;
    int num_colors, num_stipples, num_lines, selected_box;
    unsigned int r[MAX_COLORS], g[MAX_COLORS], b[MAX_COLORS];
    float distribution[MAX_COLORS+1];

    GTimeSeries *ts = NULL;
    CssArrivalClass **arrivals = NULL;

    if(!colors_window) {
	colors_window =
		new ColorSelection("Spec Color Selection", this, this);
    }

    fread(&version, sizeof(int), 1, fp);
    if(version >= 11052002 && version < 20040324) {
	fread(&len, sizeof(int), 1, fp);
    }
    else {
	len = version;
    }
    if(version < 20040324) {
	char *name = (char *)mallocWarn(len);
	fread(name, 1, len, fp);
	free(name);
    }
    fread(&num_colors, sizeof(int), 1, fp);
    fread(&num_stipples, sizeof(int), 1, fp);
    if(version < 20040324) fread(&selected_box, sizeof(int), 1, fp);
    fread(r, sizeof(int), 256, fp);
    fread(g, sizeof(int), 256, fp);
    fread(b, sizeof(int), 256, fp);
    fread(&num_lines, sizeof(int), 1, fp);
    if(version >= 11052002) {
	fread(distribution, sizeof(float), num_lines, fp);
    }
    else {
	int i;
	double percentage, lines[257];
	fread(lines, sizeof(double), num_lines, fp);
	if(num_colors > 0) {
	    min = lines[0];
	    max = lines[num_colors];
	    percentage = 1./(double)num_colors;
	    for(i = 0; i < num_colors; i++) {
	        distribution[i] = i*percentage;
	    }
	    distribution[num_colors] = 1.;
	}
    }
    colors_window->setup(num_colors, r, g, b, distribution);
    fread(&n_windows, sizeof(int), 1, fp);

    Free(x);
    x = (double *)mallocWarn(n_windows*sizeof(double));
    fread(x, sizeof(double), n_windows, fp);

    fread(&nf, sizeof(int), 1, fp);

    Free(y);
    y = (double *)mallocWarn(nf*sizeof(double));
    fread(y, sizeof(double), nf, fp);

    Free(data);
    data = (float *)mallocWarn(n_windows*nf*sizeof(float));
    fread(data, sizeof(float), n_windows*nf, fp);

    fread(&window_length, sizeof(double), 1, fp);
    fread(&window_overlap, sizeof(double), 1, fp);
    fread(&f_min, sizeof(double), 1, fp);
    fread(&f_max, sizeof(double), 1, fp);
    fread(&samprate, sizeof(double), 1, fp);

    if(version >= 11052002) {
	fread(&min, sizeof(double), 1, fp);
	fread(&max, sizeof(double), 1, fp);
	fread(&normalize, sizeof(bool), 1, fp);
	fread(&auto_color_limits, sizeof(bool), 1, fp);
	fread(&bin_average, sizeof(bool), 1, fp);
	fread(&instrument_corr, sizeof(bool), 1, fp);
    }

    fread(sta, 1, 10, fp);
    fread(chan, 1, 9, fp);
    if(version >= 11052002) {
	fread(net, 1, 9, fp);
    }
    fread(&time, sizeof(double), 1, fp);
    fread(&tlen, sizeof(float), 1, fp);
    fread(&winpts, sizeof(int), 1, fp);
    fread(&overlap, sizeof(int), 1, fp);
    fread(&lofreq, sizeof(float), 1, fp);
    fread(&hifreq, sizeof(float), 1, fp);
    fread(ts_label, 1, 20, fp);

    ts = new GTimeSeries();
    if(version >= 20040322)
    {
	int j, narr, n_main;
	CssArrivalClass *a, **a_main = NULL;
	fread(&nseg, sizeof(int), 1, fp);
	for(i = 0; i < nseg; i++) {
	    int data_length;
	    double tbeg, tdel;
	    GSegment *seg;

	    fread(&tbeg, sizeof(double), 1, fp);
	    fread(&tdel, sizeof(double), 1, fp);
	    fread(&data_length, sizeof(int), 1, fp);
	    seg = new GSegment(data_length, tbeg, tdel, 1., 1.);
	    fread(seg->data, sizeof(float), seg->length(), fp);
	    ts->addSegment(seg);
	}
	fread(&narr, sizeof(int), 1, fp);
	if(narr < 0 || narr > 1000) {
	    showWarning("SpectroRead: bad read, narrivals=%d", narr);
	    narr = 0;
	}
	narrivals = 0;
	a = new CssArrivalClass();
	arrivals = (CssArrivalClass **)mallocWarn(narr*sizeof(CssArrivalClass *));
	n_main = data_source->getTable(cssArrival, (CssTableClass ***)&a_main);
	for(i = 0; i < narr; i++) {
	    fread(a->sta, sizeof(a->sta), 1, fp);
	    fread(a->chan, sizeof(a->chan), 1, fp);
	    fread(&a->arid, sizeof(long), 1, fp);
	    for(j = 0; j < n_main; j++) {
		if(!strcasecmp(a_main[j]->sta, a->sta) &&
		   !strcasecmp(a_main[j]->chan, a->chan) &&
			a_main[j]->arid == a->arid)
		{
		    arrivals[narrivals++] = a_main[j];
		}
	    }
	}
	Free(a_main);
	GObject_free((GObject)a);
    }
    else {
	int i1, npts;
	double tbeg, tdel;
	float *fx, *fy;
	GSegment *seg;
	fread(&npts, sizeof(int), 1, fp);
	if(npts < 0) {
	    showWarning("SpectroRead: bad waveform nsamp: %d", npts);
	}
	else {
	    fx = (float *)mallocWarn(npts*sizeof(float));
	    fy = (float *)mallocWarn(npts*sizeof(float));
	    fread(fx, sizeof(float), npts, fp);
	    fread(fy, sizeof(float), npts, fp);
	    tbeg = fx[0];
	    tdel = fx[1] - fx[0];
	    i1 = 0;
	    for(i = 2; i < npts; i++) {
		if((fabs(fx[i]-fx[i-1]) - tdel)/tdel > .01) {
		    seg = new GSegment(fy+i1, i-i1, tbeg, tdel, 1., 1.);
		    ts->addSegment(seg);
		    i1 = i;
		}
	    }
	    if(npts - i1 > 1) {
		seg = new GSegment(fy+i1, i-i1, tbeg, tdel, 1., 1.);
		ts->addSegment(seg);
	    }
	    Free(fx);
	    Free(fy);
	}
    }
    w_id = -1;

    drawSpectro(ts, narrivals, arrivals);

    Free(arrivals);

    ftoa(window_length, 2, 0, msg, 50);
    parameter_window->setString("window_length_text", msg, false);

    ftoa(window_overlap, 2, 0, msg, 50);
    parameter_window->setString("window_overlap_text", msg, false);

    ftoa(f_min, 2, 0, msg, 50);
    parameter_window->setString("lo_freq_text", msg, false);

    ftoa(f_max, 2, 0, msg, 50);
    parameter_window->setString("hi_freq_text", msg, false);
}
#endif
