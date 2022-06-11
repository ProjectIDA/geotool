/** \file Polarization.cpp
 *  \brief Defines class Polarization.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h> // for finite()
#endif /* HAVE_IEEEFP_H */
using namespace std;

#include "Polarization.h"
#include "PolarParam.h"
#include "RotateData.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GCoverage.h"
#include "WaveformView.h"
#include "TaperData.h"
#include "IIRFilter.h"
#include "Demean.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}
using namespace libgpolar;

namespace libgpolar {

class PolarWaveformView : public WaveformView
{
    public:
	PolarWaveformView(const char *name, Component *parent, InfoArea *ia,
		Arg *args, int n) : WaveformView(name, parent, ia, args, n)
	{
	    rect = -1.;
	    azimuth = -1.;
	    incidence = -1.;
	}
	double azimuth;
	double rect;
	double incidence;

    protected:
	CssArrivalClass * addArrivalFromKey(CPlotArrivalCallbackStruct *p)
	{
	    Password password = getpwuid(getuid());
	    CssArrivalClass *arrival=NULL;
	    double deltim = -1., delaz = -1., delslo = -1.;
	    double slow = -1., snr = -1.;

	    p->w->channel.assign(p->w->ts->zChan());
	    arrival = makeArrival(p->w, password, p->time, p->name, deltim,
			azimuth, delaz, slow, delslo, incidence, rect, snr,
			false);
	    p->w->channel.assign(p->w->ts->chan());
	    if(arrival) {
		putArrivalWithColor(arrival, stringToPixel("black"));
	    }
	    return arrival;
	}
};

} // namespace libgpolar

static void scaleWaveforms(int npts, float *z, float *n, float *e);

Polarization::Polarization(const char *name, Component *parent, DataSource *ds)
		: Frame(name, parent, true), DataReceiver(ds)
{
    if(data_source) {
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;

    createInterface();
}

void Polarization::createInterface()
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    compute_button = new Button("Compute", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    new_polar_button = new Button("New Polarization Window...", file_menu,this);
    close_button = new Button("Close", file_menu, this);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    auto_toggle = new Toggle("Auto Cursor", option_menu, this);
    param_button = new Button("Parameters...", option_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Polarization Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNwidth, 540); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNverticalScroll, False); n++;
    XtSetArg(args[n], XtNhorizontalScroll, False); n++;
    XtSetArg(args[n], XtNyLabel, "Rectilinearity"); n++;
    XtSetArg(args[n], XtNleftMargin, 20); n++;
    XtSetArg(args[n], XtNbottomMargin, 2); n++;
    XtSetArg(args[n], XtNheight, 120); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_HMS); n++;
    XtSetArg(args[n], XtNsaveSpace, True); n++;
    XtSetArg(args[n], XtNdisplayXLabel, False); n++;
    plot[0] = new CPlotClass("plot1", pane, info_area, args, n);
    plot[0]->setYLimits(-1., 1., true);
    plot[0]->setCurveMargins(.03, .03, .1, .1);

    n = 0;
    XtSetArg(args[n], XtNverticalScroll, False); n++;
    XtSetArg(args[n], XtNhorizontalScroll, False); n++;
    XtSetArg(args[n], XtNyLabel, "Incidence"); n++;
    XtSetArg(args[n], XtNheight, 120); n++;
    XtSetArg(args[n], XtNleftMargin, 20); n++;
    XtSetArg(args[n], XtNbottomMargin, 2); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_HMS); n++;
    XtSetArg(args[n], XtNsaveSpace, True); n++;
    XtSetArg(args[n], XtNdisplayXLabel, False); n++;
    plot[1] = new CPlotClass("plot2", pane, info_area, args, n);
    plot[1]->setYLimits(-1., 1., true);
    plot[1]->setCurveMargins(.03, .03, .1, .1);

    n = 0;
    XtSetArg(args[n], XtNverticalScroll, False); n++;
    XtSetArg(args[n], XtNhorizontalScroll, False); n++;
    XtSetArg(args[n], XtNyLabel, "P Azimuth"); n++;
    XtSetArg(args[n], XtNheight, 120); n++;
    XtSetArg(args[n], XtNleftMargin, 20); n++;
    XtSetArg(args[n], XtNbottomMargin, 2); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_HMS); n++;
    XtSetArg(args[n], XtNsaveSpace, True); n++;
    XtSetArg(args[n], XtNdisplayXLabel, False); n++;
    plot[2] = new CPlotClass("plot3", pane, info_area, args, n);
    plot[2]->setYLimits(-1., 1., true);
    plot[2]->setCurveMargins(.03, .03, .1, .1);

    n = 0;
    XtSetArg(args[n], XtNverticalScroll, False); n++;
    XtSetArg(args[n], XtNhorizontalScroll, False); n++;
    XtSetArg(args[n], XtNyLabel, "Slowness(s/deg)"); n++;
    XtSetArg(args[n], XtNheight, 120); n++;
    XtSetArg(args[n], XtNleftMargin, 20); n++;
    XtSetArg(args[n], XtNbottomMargin, 2); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_HMS); n++;
    XtSetArg(args[n], XtNsaveSpace, True); n++;
    XtSetArg(args[n], XtNdisplayXLabel, False); n++;
    plot[3] = new CPlotClass("plot4", pane, info_area, args, n);
    plot[3]->setYLimits(-1., 1., true);
    plot[3]->setCurveMargins(.03, .03, .1, .1);

    n = 0;
    XtSetArg(args[n], XtNheight, 170); n++;
    XtSetArg(args[n], XtNleftMargin, 20); n++;
    XtSetArg(args[n], XtNverticalScrollPosition, LEFT); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    wplot = new PolarWaveformView("plot5", pane, info_area, args, n);
    plot[4] = wplot;
    wplot->addActionListener(this, XtNdoubleLineCallback);
    wplot->addActionListener(this, XtNdoubleLineDragCallback);
    wplot->addActionListener(this, XtNlimitsCallback);

    for(int i = 0; i < 4; i++) {
	plot[i]->addActionListener(this, XtNphaseLineDragCallback);
	plot[i]->addActionListener(this, XtNphaseLineCallback);
	plot[i]->addActionListener(this, XtNlimitsCallback);
    }
    ignore_limits_callback = false;

    memset(polar_sta, 0, sizeof(polar_sta));
    memset(polar_chan, 0, sizeof(polar_chan));
    memset(polar_net, 0, sizeof(polar_net));

    param_window = new PolarParam("Polarization Parameters", this, this);

    print_window = NULL;

    addPlugins("Polarization", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button);
	tool_bar->add(compute_button);
	tool_bar->add(param_button);
    }
    polar_tbeg = 0.;
    polar_window_width = 0.;
    samprate = 0.;
    npts = 0;
    last_window_length = 0.;
    memset(last_inc_label, 0, sizeof(last_inc_label));
    memset(last_az_label, 0, sizeof(last_az_label));
}

Polarization::~Polarization(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void Polarization::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp && auto_toggle->state()) {
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
	else wp = NULL;
    }
}


void Polarization::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Auto Cursor")) {
	if(wp) {
	    if(auto_toggle->state()) {
		wp->addActionListener(this, XtNdoubleLineDragCallback);
		wp->addActionListener(this, XtNdoubleLineScaleCallback);
	    }
	    else {
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
    }
    else if(comp == wp) {
	if( (!strcmp(reason, XtNdoubleLineScaleCallback) ||
		!strcmp(reason, XtNdoubleLineDragCallback))
		&& auto_toggle->state())
	{
	    AxesCursorCallbackStruct *a = (AxesCursorCallbackStruct *)
                                action_event->getCalldata();
            if(a->label[0] == 'a') {
		compute();
	    }
	}
    }
    else if( !strcmp(reason, XtNphaseLineDragCallback) ||
	!strcmp(reason, XtNphaseLineCallback) ||
	!strcmp(reason, XtNdoubleLineCallback) ||
	!strcmp(reason, XtNdoubleLineDragCallback))
    {
	lineDrag((CPlotClass *)comp,
		(AxesCursorCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNlimitsCallback)) {
	tieLimits((CPlotClass *)comp,
		(AxesLimitsCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Compute") || comp == param_window) {
	compute();
    }
    else if(!strcmp(cmd, "Parameters...")) {
	param_window->setVisible(true);
    }
    else if(!strcmp(cmd, "New Polarization Window...")) {
	newWindow();
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Polarization Help")) {
	showHelp(cmd);
    }
}

ParseCmd Polarization::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;
    string c;

    if(parseCompare(cmd, "Compute")) {
	compute_button->activate();
    }
    else if(parseArg(cmd, "auto_window_parameters", c)) {
	return param_window->auto_param_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "window_length", c)) {
	param_window->window_length_text->setString(c, true);
    }
    else if(parseArg(cmd, "save_file", c)) {
	return save(c, msg) ? COMMAND_PARSED : ARGUMENT_ERROR;
    }
    else if(parseArg(cmd, "print_window", c)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print Polarization", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = param_window->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
        return ret;
    }
    else if((ret = wplot->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
        return ret;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return ret;
}

ParseVar Polarization::parseVar(const string &name, string &value)
{
    string c;
    int numpts;
    float *y;
    double *x;
    vector<CPlotCurve> s;
    ParseVar ret;

    if(parseString(name, "rectilinearity", c)) {
        return plot[0]->parseVar(c, value);
    }
    else if(parseString(name, "incidence", c)) {
        return plot[1]->parseVar(c, value);
    }
    else if(parseString(name, "azimuth", c)) {
        return plot[2]->parseVar(c, value);
    }
    else if(parseString(name, "slowness", c)) {
        return plot[3]->parseVar(c, value);
    }
    else if(parseString(name, "wplot", c)) {
        return wplot->parseVar(c, value);
    }
    else if(parseCompare(name, "recti", 5)) {
	if(plot[0]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "recti", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "inc", 3)) {
	if(plot[1]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "inc", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "paz", 3)) {
	if(plot[2]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "paz", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "slow", 3)) {
	if(plot[3]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "slow", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "time", 4)) {
	if(plot[0]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	x = s[0].x;
	if( parseArray(name, "time", numpts, x, value, &ret) ) {
	    return ret;
	}
    }
    return Frame::parseVar(name, value);
}

bool Polarization::save(const string &file, string &msg)
{
    FILE *fp;

    vector<CPlotCurve> c1, c2, c3;
    if(plot[0]->getCurves(c1) <= 0 || plot[1]->getCurves(c2) <= 0 ||
	plot[2]->getCurves(c3) <= 0)
    {
	msg.assign("No polarization results.");
	return false;
    }
    if( !(fp = fopen(file.c_str(), "w")) ) {
	msg.assign(string("Cannot open file ") + file + "\n" +strerror(errno));
	return false;
    }
// write sta/chan time of first point, samprate, nsamp
// write rect, incidence, azimuth
    Arg args[1];
    char *title;
    XtSetArg(args[0], XtNtitle, &title);
    wplot->getValues(args, 1);
    fprintf(fp, "%s  samprate=%.2f nsamp=%d\n", title, samprate, npts);
    fprintf(fp, "window length=%.2f num_windows=%d\n",
	last_window_length, c1[0].npts);
    fprintf(fp, "rectilinearity incidence   azimuth\n");

    for(int i = 0; i < c1[0].npts; i++) {
	fprintf(fp, "%10.4f   %10.4f %10.4f\n", c1[0].y[i], c2[0].y[i],
		c3[0].y[i]);
    }
    fclose(fp);

    return true;
}

void Polarization::parseHelp(const char *prefix)
{
    printf("%scompute\n", prefix);
    printf("%sauto_window_parameters=(true,false)\n", prefix);
    printf("%swindow_length=SECONDS\n", prefix);
    printf("%sprint.help\n", prefix);
}

void Polarization::print(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print Polarization", this, this);
    }
    print_window->setVisible(true);
}

void Polarization::setVisible(bool visible)
{
    Frame::setVisible(visible);
    if(!visible) {
	auto_toggle->set(false);
	// if this is not the first Polarization window, clear it
	for(int i = 0; i < (int)windows.size(); i++) {
	    if(this == windows[i]) {
		for(int j = 0; j < 5; j++) plot[i]->clear();
	    }
	}
    }
}

void Polarization::tieLimits(CPlotClass *cplot, AxesLimitsCallbackStruct *a)
{
    if(ignore_limits_callback) return;
    ignore_limits_callback = true;

    for(int i = 0; i < 5; i++) if(plot[i] != cplot)
    {
	if(a->x_margins) {
	    plot[i]->setMargins(false, true, a->left, a->right, 0, 0);
	}
	plot[i]->setXLimits(a->x_min, a->x_max);
    }
    ignore_limits_callback = false;
}

void Polarization::compute(void)
{
    int i, j, k, l, window_pts, window_width, windowed, ngroups,
		n_windows, order, zp;
    vector<int> ncmpts;
    gvector<GSegmentArray*> *cov;
    gvector<GTimeSeries *> ts;
    GTimeSeries *t[3];
    float *recti = NULL, *az = NULL, *slow = NULL, *incidence = NULL;
    double *x = NULL;
    char msg[1024], tlab[50], type[3];
    double s[9], d[3], o[3], v[9], c[3][3];
    double a, x_min, x_max, diff, cursor_x, alpha, beta, gamma;
    double vx, vy, vz, lo_cut, hi_cut;
    bool auto_param;
    gvector<Waveform *> wvec;
    Arg args[1];
    bool rotated;

    if(!data_source) return;

    memset(last_inc_label, 0, sizeof(last_inc_label));
    memset(last_az_label, 0, sizeof(last_az_label));

    windowed = 1;
    ngroups = data_source->getSelectedComponents("a", ncmpts, wvec);
    if(ngroups <= 0 && !data_source->dataWindowIsDisplayed("a"))
    {
	windowed = 0;
	ngroups = data_source->getSelectedComponents(ncmpts, wvec);
    }
    if(ngroups <= 0 || ncmpts[0] != 3) {
	showWarning("No 3-component waveform set selected.");
	return;
    }
    else if(ngroups > 1) {
	showWarning("More than one 3-component set selected.");
	return;
    }

    if(windowed)
    {
	GDataPoint *d1 = wvec[0]->dw[0].d1;
	GDataPoint *d2 = wvec[0]->dw[0].d2;
	GDataPoint *dlast;

	samprate = 1./d1->segment()->tdel();
	npts = d2->index() - d1->index() + 1;

	dlast = wvec[0]->dw[0].d1;
	for(i = 1; i < 3; i++)
	{
	    d1 = wvec[i]->dw[0].d1;
	    diff = fabs(dlast->segment()->tdel() - d1->segment()->tdel());
	    dlast = d1;

	    if(diff/d1->segment()->tdel() > .01) {
		showWarning("variable sample rate.");
		return;
	    }
	    d2 = wvec[i]->dw[0].d2;
	}
	t[0] = wvec[0]->ts->subseries(d1->time(), d2->time());
	npts = t[0]->length();
	d1 = wvec[1]->dw[0].d1;
	d2 = wvec[1]->dw[0].d2;
	t[1] = wvec[1]->ts->subseries(d1->time(), d2->time());
	if(npts > t[1]->length()) npts = t[1]->length();
	d1 = wvec[2]->dw[0].d1;
	d2 = wvec[2]->dw[0].d2;
	t[2] = wvec[2]->ts->subseries(d1->time(), d2->time());
	if(npts > t[2]->length()) npts = t[2]->length();
    }
    else {
	samprate = 1./wvec[0]->segment(0)->tdel();
	npts = wvec[0]->length();
	for(i = 1; i < 3; i++)
	{
	    diff = fabs(wvec[i]->segment(0)->tdel() -
			wvec[0]->segment(0)->tdel());
	    if(diff/wvec[i]->segment(0)->tdel() > .01) {
		showWarning("variable sample rate.");
		return;
	    }
	    if(npts > wvec[i]->length()) {
		npts = wvec[i]->length();
	    }
	}
	t[0] = new GTimeSeries(wvec[0]->ts);
	t[1] = new GTimeSeries(wvec[1]->ts);
	t[2] = new GTimeSeries(wvec[2]->ts);
    }
    if(param_window->filterOn() && param_window->getFilter(type, &order,
			&lo_cut, &hi_cut, &zp))
    {
	double tdel = t[0]->segment(0)->tdel();
	DataMethod *dm[2];
	IIRFilter *iir = new IIRFilter(order, type, lo_cut, hi_cut, tdel, zp);
	TaperData *taper = new TaperData("cosine", 5, 5, 200);

	dm[0] = taper;
	dm[1] = iir;
	for(i = 0; i < 3; i++) {
	    DataMethod::changeMethods(2, dm, 1, &t[i]);
	}
    }
    for(i = 0; i < 3; i++) {
	gvector<DataMethod *> *methods = wvec[i]->dataMethods();
	if((int)methods->size()==0 || !methods->back()->getDemeanInstance()) {
	    Demean *dm = new Demean();
	    dm->apply(t[i]);
	}
	ts.push_back(t[i]);
    }

    param_window->getBool("Auto Window Parameters", &auto_param);

    if(auto_param || !param_window->getDouble("window_length", &a) || 
		(int)(a*samprate+.5) < 4)
    {
	a = (npts > 20) ? 10.*samprate : 4;
	if(a < 4) a = 4;
	a /= samprate;
	ftoa((double)a, 2, 0, msg, 50);
	param_window->setString("window_length", msg);
    }
    last_window_length = a;

    window_width = (int)(a*samprate+.5);

    polar_window_width = window_width/samprate;

    window_pts = window_width + 1;

    cov = GCoverage::getArrays(ts, ts[0]->tbeg(), ts[0]->tend());

    l = 0;
    for(j = 0; j < (int)cov->size(); j++) {
	for(k = 0; k + window_width < cov->at(j)->npts; k++, l++);
	l++; // space for the gap indicator set_fnan()
    }
    n_windows = l;

    if( !n_windows ) {
	showWarning("data length < window length");
	delete cov;
        Free(recti); Free(az); Free(slow); Free(incidence); Free(x);
	return;
    }

    if(	!(recti = (float *)mallocWarn(n_windows*sizeof(float))) ||
	!(az = (float *)mallocWarn(n_windows*sizeof(float))) ||
	!(slow = (float *)mallocWarn(n_windows*sizeof(float))) ||
	!(incidence = (float *)mallocWarn(n_windows*sizeof(float))) ||
	!(x = (double *)mallocWarn(n_windows*sizeof(double))))
    {
	delete cov;
        Free(recti); Free(az); Free(slow); Free(incidence); Free(x);
	return;
    }

    cde_id = wvec[0]->getId();
    cdn_id = wvec[1]->getId();
    cdz_id = wvec[2]->getId();

    alpha = wvec[0]->currentAlpha();
    beta = wvec[0]->currentBeta();
    gamma = wvec[0]->currentGamma();

    if(alpha != 0. || beta != 0. || gamma != 0.) {
	alpha *= M_PI/180.;
	beta *= M_PI/180.;
	gamma *= M_PI/180.;
	rotation_matrix(alpha, beta, gamma, c);
	rotated = true;
    }
    else {
	rotated = false;
    }

    stringcpy(polar_sta, ts[0]->sta(), sizeof(polar_sta));
    stringcpy(polar_chan, ts[0]->chan(), sizeof(polar_chan));
    stringcpy(polar_net, ts[0]->net(), sizeof(polar_net));
    polar_tbeg = ts[0]->tbeg();

    ArrivalParams *ap = WaveformPlot::arrivalParams();

    l = 0;
    for(j = 0; j < (int)cov->size(); j++)
    {
	GSegmentArray *sa = cov->at(j);
	double dt = sa->segments[0]->tdel();

	float *e = sa->segments[0]->data + sa->beg_index[0];
	float *n = sa->segments[1]->data + sa->beg_index[1];
	float *z = sa->segments[2]->data + sa->beg_index[2];

	scaleWaveforms(sa->npts, z, n, e); // not really needed

	for(k = 0; k + window_width < sa->npts; k++, l++)
	{
	    // time of the middle of the window
	    x[l] = sa->tmin + (k + .5*window_width)*dt;

	    covar(window_pts, e+k, n+k, z+k, s);

	    tred2(3, s, d, o, v);
	    tql2(3, d, o, v);
	    for(i = 0; i < 3; i++) if(d[i] < 0.) d[i] = 0.;
/*
	    d[0] = sqrt(d[0]);	// make units amp instead of power
	    d[1] = sqrt(d[1]);
	    d[2] = sqrt(d[2]);
*/
	    if(d[2] == 0.) {
		recti[l] = 0.;
	    }
	    else {
		recti[l] = 1. - .5*(d[0] + d[1])/d[2];
	    }
	    // If the stations Components are not E,N,UP, then find the
	    // coordinates of the eignvector v[6],v[7],v[8] in the E,N,UP
	    // coordinate system.
	    if(rotated) {
		vx = c[0][0]*v[6] + c[0][1]*v[7] + c[0][2]*v[8];
		vy = c[1][0]*v[6] + c[1][1]*v[7] + c[1][2]*v[8];
		vz = c[2][0]*v[6] + c[2][1]*v[7] + c[2][2]*v[8];
		v[6] = vx;
		v[7] = vy;
		v[8] = vz;
		vx = c[0][0]*v[0] + c[0][1]*v[1] + c[0][2]*v[2];
		vy = c[1][0]*v[0] + c[1][1]*v[1] + c[1][2]*v[2];
		vz = c[2][0]*v[0] + c[2][1]*v[1] + c[2][2]*v[2];
		v[0] = vx;
		v[1] = vy;
		v[2] = vz;
	    }

	    i = (v[8] > 0.) ? -1 : 1;
	    az[l] = atan2(i*v[6], i*v[7])/DEG_TO_RADIANS;
	    i = (v[2] > 0.) ? -1 : 1;
	    incidence[l] = acos(fabs(v[8]))/DEG_TO_RADIANS;
	    slow[l] = ap->polar_alpha * sin(DEG_TO_RADIANS*incidence[l]/2.)
			* DEG_TO_KM;
	}
	if(j < (int)cov->size()-1) {
	    x[l] = sa->tmin + (k+1 + .5*window_width)*dt;
	    set_fnan(recti[l]);
	    set_fnan(incidence[l]);
	    set_fnan(az[l]);
	    set_fnan(slow[l]);
	    l++;
	}
    }
    n_windows = l;
    delete cov;

    plot[0]->clearWaveforms();
    plot[0]->addCurve(n_windows, x, recti, "rect", true, 0);

    x_min = x[0];
    x_max = x[n_windows-1];
    cursor_x = x_min + .2*(x_max - x_min);
    cursor_x = positionLine(NULL, plot[0], cursor_x, 0);

    timeEpochToString(polar_tbeg, tlab, 50, GSE20);
    snprintf(msg, 1024, "%s %s", polar_sta, tlab);
    XtSetArg(args[0], XtNtitle, msg);
    wplot->setValues(args, 1);

    wplot->clear();

    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    cvector<CssOriginClass> origins;
    data_source->getArrivalsOnWaveform(wvec[2], arrivals);
    data_source->getTable(assocs);
    data_source->getTable(origins);
    for(i = 0; i < arrivals.size(); i++) {
	wplot->putArrivalWithColor(arrivals[i], stringToPixel("black"));
    }
    for(i = 0; i < assocs.size(); i++) {
        wplot->putTable(assocs[i], false);
    }
    for(i = 0; i < origins.size(); i++) {
        wplot->putTable(origins[i], false);
    }

    wplot->addWaveform(ts[0], wvec[0]->fg);
    wplot->addWaveform(ts[1], wvec[1]->fg);
    wplot->addWaveform(ts[2], wvec[2]->fg);
    wplot->copyAllTables(data_source);

    wplot->positionDoubleLine("b", cursor_x-.5*polar_window_width,
		    cursor_x+.5*polar_window_width, true);

    plot[1]->clearWaveforms();
    plot[1]->addCurve(n_windows, x, incidence, "inc", true, 0);
    positionLine(NULL, plot[1], cursor_x, 0);

    plot[2]->clearWaveforms();
    plot[2]->addCurve(n_windows, x, az, "az", true, 0);
    positionLine(NULL, plot[2], cursor_x, 0);

    plot[3]->clearWaveforms();
    plot[3]->addCurve(n_windows, x, slow, "slow", true, 0);
    positionLine(NULL, plot[3], cursor_x, 0);

    if(wp) {
	wp->positionDoubleLine("b", cursor_x-.5*polar_window_width,
				cursor_x+.5*polar_window_width, true);
    }

    rotate();

    Free(recti); Free(az); Free(slow); Free(incidence); Free(x);
}

static void
scaleWaveforms(int npts, float *z, float *n, float *e)
{
    int i;
    double d, scale;

    scale = 0.;
    for(i = 0; i < npts; i++)
    {
	d = fabs(z[i]);
	if(d > scale) scale = d;
	d = fabs(e[i]);
	if(d > scale) scale = d;
	d = fabs(n[i]);
	if(d > scale) scale = d;
    }
    if(scale != 0.) {
	scale = 1./scale;
	for(i = 0; i < npts; i++) {
	    z[i] *= scale;
	    n[i] *= scale;
	    e[i] *= scale;
	}
    }
}

void Polarization::lineDrag(CPlotClass *cp, AxesCursorCallbackStruct *c)
{
    Waveform *w;
    double cursor_x;

    if(cp == wplot) {
	cursor_x = c->scaled_x1 + .5*polar_window_width;
    }
    else {
	cursor_x = c->scaled_x;
    }

    for(int i = 0; i < 4; i++) {
	cursor_x = positionLine(cp, plot[i], cursor_x, c->reason);
    }

    if(cp != wplot || c->reason != AXES_DOUBLE_LINE_DRAG)
    {
	wplot->positionDoubleLine("b", cursor_x-.5*polar_window_width,
		cursor_x+.5*polar_window_width, false);
    }

    if(wp && (w = wp->getWaveform(cdz_id)) != NULL)
    {
	wp->positionDoubleLine("b", cursor_x-.5*polar_window_width,
			cursor_x+.5*polar_window_width, true);
    }

    char tlab[100], lab[100];
    double x = polar_tbeg + cursor_x - .5*polar_window_width;
    timeEpochToString(x, tlab, 100, GSE20);
    snprintf(lab, sizeof(lab), "%s %s", polar_sta, tlab);
    info_area->setText(INFO_AREA_LEFT, lab);

    rotate();
}

double Polarization::positionLine(CPlotClass *p, CPlotClass *plot2,
			double x, int reason)
{
    int i;
    char label[50];
    double min, d, minx;
    float y;
    CPlotCurve c;
    vector<CPlotCurve> curves;

    if(plot2->getCurves(curves) > 0 && curves[0].npts > 0)
    {
	c = curves[0];

	label[0] = '\0';
	if(x >= c.x[0] && x <= c.x[c.npts-1])
	{
	    min = fabs(x - c.x[0]);
	    y = c.y[0];
	    minx = c.x[0];
	    for(i = 0; i < c.npts; i++) {
		d = fabs(x - c.x[i]);
		if(d < min && finite(c.y[i]) && !fNaN(c.y[i])) {
		    min = d;
		    y = c.y[i];
		    minx = c.x[i];
		}
	    }
	    snprintf(label, 50, "%.2f", y);
	    x = minx;
	}
	if(plot2 != p || reason != AXES_PHASE_LINE_DRAG) {
	    plot2->positionPhaseLine(label, x, false);
	}
	else {
	    plot2->renamePhaseLine(label);
	}
    }
    return x;
}

void Polarization::rotate(void)
{
    string errstr;
    char *rect_label, *inc_label, *az_label;
    double rect, inc, az, t1, t2;
    vector<int> ncmpts;
    gvector<Waveform *> wvec;

    if(plot[1]->getPhaseLine(&t1, &inc_label)
	&& stringToDouble(inc_label, &inc)
	&& plot[0]->getPhaseLine(&t1, &rect_label)
	&& stringToDouble(rect_label, &rect)
	&& plot[2]->getPhaseLine(&t2, &az_label)
	&& stringToDouble(az_label, &az))
    {
	if(strcmp(inc_label, last_inc_label)
		|| strcmp(az_label, last_az_label))
	{
	    double reverse_azimuth = az + 180.;

	    if( wplot->getComponents(ncmpts, wvec) == 1
		&& ncmpts[0] == 3 && RotateData::checkComponents(true,
			inc, ncmpts, wvec, errstr) == 1)
	    {
		RotateData::rotateWaveforms(reverse_azimuth, inc,
                                wvec[0], wvec[1], wvec[2], errstr);
		gvector<Waveform *> v(3);
		for(int i = 0; i < 3; i++) v.push_back(wvec[i]);
		wplot->modifyWaveforms(v);
		wplot->rect = rect;
		wplot->azimuth = az;
		wplot->incidence = inc;
	    }
	    if(errstr.empty()) putWarning(errstr.c_str());
	    strncpy(last_inc_label, inc_label, sizeof(last_inc_label));
	    strncpy(last_az_label, az_label, sizeof(last_az_label));
	}
    }
}

void Polarization::newWindow(void)
{
    int i;

    for(i = 0; i < (int)windows.size(); i++) {
	if( !windows[i]->isVisible() ) {
	    windows[i]->setVisible(true);
	    break;
	}
    }
    if(i == (int)windows.size()) {
	Polarization *p = new Polarization(getName(), this, data_source);
	windows.push_back(p);
	p->setVisible(true);
    }
}

void Polarization::print(FILE *fp, PrintParam *p)
{
//    char title[200], tlab[100];
    int total_height;
    AxesParm *a, *a2;
    double f, bottom, top;
    Dimension height;
    Arg args[1];

    XtSetArg(args[0], XtNheight, &height);

    total_height = 0;
    for(int i = 0; i < 5; i++) {
	plot[i]->getValues(args, 1);
	total_height += height;
    }

/*
    snprintf(title, 200,
	"%s  %s\nwindow-length %.1fsecs  window-overlap %.1fsecs",
	fk->p.net, tlab, fk->p.window_length, fk->p.window_overlap);
    p->top_title = title;
*/
    p->top_title = NULL;

    plot[0]->getValues(args, 1);
    f = (double)height/(double)total_height;
    bottom = p->bottom;
    top = p->top;
    p->bottom = top - f*(top - bottom);

    if( !(a = plot[0]->hardCopy(fp, p, NULL)) ) {
	return;
    }

    p->top_title = NULL;
    a->auto_x = False;

    for(int i = 1; i < 5; i++)
    {
	p->top = p->bottom;
	plot[i]->getValues(args, 1);
	f += height/(double)total_height;
	p->bottom = (i < 3) ? top - f*(top - bottom) : bottom;
	if(i == 3) {
	    p->x_axis_label = (char *)"time (seconds)";
	}
	if((a2 = plot[i]->hardCopy(fp, p, a)) == NULL)
	{
	    free(a);
	    return;
	}
        free(a2);
    }
    free(a);
}
