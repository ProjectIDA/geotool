/** \file GCepstrum.cpp
 *  \brief Defines class GCepstrum
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>

using namespace std;

#include "GCepstrum.h"
#include "CepstrumParams.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"

extern "C" {
#include "libstring.h"
#include "cepstrum.h"
}

using namespace libgcepstrum;


GCepstrum::GCepstrum(const char *name, Component *parent, DataSource *ds) :
		Frame(name, parent, true), DataReceiver(ds)
{
    createInterface();
    init();
}

void GCepstrum::createInterface()
{
    int n;
    Arg args[30];

    setSize(600, 500);
    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    compute_button = new Button("Compute", file_menu, this);
    cursor_menu = new Menu("Cursor", file_menu, false);
    cursor_toggle[0] = new Toggle("a: signal", cursor_menu, this);
    cursor_toggle[0]->set(true);
    cursor_toggle[1] = new Toggle("b: noise", cursor_menu, this);
    cursor_toggle[1]->set(true);
    cursor_toggle[2] = new Toggle("c", cursor_menu, this);
    cursor_toggle[3] = new Toggle("d", cursor_menu, this);
    
    print_button = new Button("Print...", file_menu, this);
    new_cep_button = new Button("New Cepstrum Window...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    option_menu = new Menu("Option", menu_bar);
    parameters_button = new Button("Parameters...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    cepstrum_help_button = new Button("Cepstrum Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    tab = new TabClass("tab", frame_form, args, n);
    tab->addActionListener(this, XtNtabCallback);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    form[0] = new Form("Spectrum", tab, args, n);
    form[1] = new Form("Smoothed", tab, args, n);
    form[2] = new Form("Minus Noise", tab, args, n);
    form[3] = new Form("Detrended/Tapered", tab, args, n);
    form[4] = new Form("Inverse FFT", tab, args, n);
    form[5] = new Form("Cepstrum", tab, args, n);
    tab->setOnTop("Spectrum");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtopMargin, 10); n++;
    XtSetArg(args[n], XtNbottomMargin, 5); n++;
    XtSetArg(args[n], XtNborderWidth, 1); n++;

    XtSetArg(args[n], XtNxLabel, "Frequency (Hz)"); n++;
    plot[0] = new CPlotClass("plot0", form[0], info_area, args, n);

    plot[1] = new CPlotClass("plot1", form[1], info_area, args, n);
    plot[1]->addActionListener(this, XtNphaseLineCallback);
    plot[1]->addActionListener(this, XtNphaseLineDragCallback);

    plot[2] = new CPlotClass("plot2", form[2], info_area, args, n);
    plot[2]->addActionListener(this, XtNphaseLineCallback);
    plot[2]->addActionListener(this, XtNphaseLineDragCallback);

    plot[3] = new CPlotClass("plot3", form[3], info_area, args, n);
    n--;
    XtSetArg(args[n], XtNxLabel, "Time (secs)"); n++;
    plot[4] = new CPlotClass("plot4", form[4], info_area, args, n);
    n--;
    XtSetArg(args[n], XtNxLabel, "Delay Time (secs)"); n++;
    plot[5] = new CPlotClass("plot5", form[5], info_area, args, n);

    param_window = new CepstrumParams("Cepstrum Parameters", this, this);
    print_window = NULL;

    addPlugins("GCepstrum", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
	tool_bar->add(parameters_button, "Parameters...");
    }
}

GCepstrum::~GCepstrum(void)
{
}

void GCepstrum::init(void)
{
    memset(compute_tab, 0, sizeof(compute_tab));
}

void GCepstrum::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    const char *reason = action_event->getReason();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "New Cepstrum Window..."))
    {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    GCepstrum *cep = new GCepstrum(getName(), this, data_source);
            windows.push_back(cep);
            cep->setVisible(true);
        }
    }
    else if(!strcmp(cmd, "Parameters...")) {
	param_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Compute")) {
	compute();
    }
    else if(!strcmp(cmd, "Cepstrum Help")) {
	showHelp(cmd);
    }
    else if(comp == param_window) {
	if(!action_event->getCalldata()) {
	    compute();
	}
	else {
	    frequencyLimits((char *)action_event->getCalldata());
	}
    }
    else if(comp == tab) {
	selectTab((char *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNphaseLineCallback) || 
	    !strcmp(reason, XtNphaseLineDragCallback))
    {
	cepstrumLimits((CPlotClass *)comp,
		(AxesCursorCallbackStruct *)action_event->getCalldata());
    }
    else if(comp == cursor_toggle[0] || comp == cursor_toggle[1] ||
	comp == cursor_toggle[2] || comp == cursor_toggle[3])
    {
	Toggle *t = comp->getToggleInstance();
	if(t) selectCursor(t);
    }
}

ParseCmd GCepstrum::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "compute")) {
	compute_button->activate();
    }
    else if(parseString(cmd, "print_window", c)) {
	if(!print_window) {
	    print_window = new PrintDialog("Print Cepstrum", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if(!parseSetParam(cmd)) {
	return Frame::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar GCepstrum::parseVar(const string &name, string &value)
{
    int numpts;
    float *y;
    double *x;
    vector<CPlotCurve> s;
    ParseVar ret;

    if(parseCompare(name, "spectrum", 8)) {
	if(plot[0]->getCurves(s) <= 0) {
	    value.assign("no cepstrum results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "spectrum", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "smoothed", 8)) {
	if(plot[1]->getCurves(s) <= 0) {
	    value.assign("no cepstrum results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "smoothed", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "minus_noise", 11)) {
	if(plot[2]->getCurves(s) <= 0) {
	    value.assign("no cepstrum results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "minus_noise", numpts, y, value, &ret)) {
	    return ret;
	}
    }
    else if(parseCompare(name, "detrended", 9)) {
	if(plot[3]->getCurves(s) <= 0) {
	    value.assign("no cepstrum results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "detrended", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "inverse", 7)) {
	if(plot[4]->getCurves(s) <= 0) {
	    value.assign("no cepstrum results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "inverse", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "cepstrum", 8)) {
	if(plot[5]->getCurves(s) <= 0) {
	    value.assign("no cepstrum results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	y = s[0].y;
	if( parseArray(name, "cepstrum", numpts, y, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "frequency", 9)) {
	if(plot[0]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	x = s[0].x;
	if( parseArray(name, "frequency", numpts, x, value, &ret) ) {
	    return ret;
	}
    }
    else if(parseCompare(name, "delay", 5)) {
	if(plot[5]->getCurves(s) <= 0) {
	    value.assign("no polarization results");
	    return VARIABLE_ERROR;
	}
	numpts = s[0].npts;
	x = s[0].x;
	if( parseArray(name, "delay", numpts, x, value, &ret) ) {
	    return ret;
	}
    }
    return Frame::parseVar(name, value);
}

void GCepstrum::parseHelp(const char *prefix)
{
    printf("%scompute\n", prefix);
    printf("%sprint.help\n", prefix);
}

void GCepstrum::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Cepstrum", this, this);
    }
    print_window->setVisible(true);
}

void GCepstrum::compute(void)
{
    int i, i1, i2, num;
    double *x=NULL;
    CepstrumStruct cs[2], *signal=NULL, *noise=NULL;
    CepstrumParam cp = CEPSTRUM_PARAM_NULL;
    CepstrumOut co;

    if( !getParam(&cp) ) return;

    if(!(num = getData(cs))) return;

    compute_tab[0] = '\0';

    for(i = 0; i < num; i++) {
	if(!i) {
	    if(!cs[i].signal_type) signal = &cs[i];
	    else if(cs[i].signal_type == 1) noise = &cs[i];
	}
	else if(!strcmp(cs[i].sta, cs[i-1].sta) &&
		cs[i].signal_type != cs[i-1].signal_type)
	{
	    if(!signal && !cs[i].signal_type) signal = &cs[i];
	    else if(!noise && cs[i].signal_type==1) noise= &cs[i];
	}
    }
    if(!signal) {
	noise = NULL;
	signal = &cs[0];
    }
    if(noise && fabs(noise->dt - signal->dt) > .001) {
	showWarning("Noise and signal sample rates are different.");
	return;
    }
    if(signal->dt != 0. && cp.fhi > 1./(2.*signal->dt)) {
	cp.fhi = 1./(2.*signal->dt);
    }

    if(noise) {
	tab->setSensitive("Minus Noise", true);
	if(Cepstrum(signal->data, signal->npts, noise->data, noise->npts,
			signal->dt, &cp, &co)) return;
    }
    else {
	tab->setSensitive("Minus Noise", false);
	if(Cepstrum(signal->data, signal->npts, NULL, 0, signal->dt, &cp, &co))
	    return;
    }
    for(i = 0; i < num; i++) {
	Free(cs[i].data);
    }

    x = (double *)malloc(co.nf*sizeof(double));
    for(i = 0; i < co.nf; i++) x[i] = i*co.df;

/*  tab->setOnTop("Cepstrum"); */

    plot[0]->clear();
    plot[0]->addCurve(co.nf, x, co.data1, stringToPixel("black"));
    if(co.noise1) {
	plot[0]->addCurve(co.nf, x, co.noise1, stringToPixel("red"));
    }

    plot[1]->clear();
    plot[1]->addCurve(co.nf, x, co.data2, stringToPixel("black"));
    if(co.noise2) {
	plot[1]->addCurve(co.nf, x, co.noise2, stringToPixel("red"));
    }
    plot[1]->addPhaseLine2("low");
    plot[1]->positionPhaseLine2("low", cp.flo, false);
    plot[1]->addPhaseLine2("high");
    plot[1]->positionPhaseLine2("high", cp.fhi, false);
    drawTrendLine(plot[1], x, co.data2, co.nf, true);

    if(noise)
    {
	plot[2]->clear();
	plot[2]->addCurve(co.nf, x, co.data3, stringToPixel("black"));
	plot[2]->addPhaseLine2("low");
	plot[2]->positionPhaseLine2("low", cp.flo, false);
	plot[2]->addPhaseLine2("high");
	plot[2]->positionPhaseLine2("high", cp.fhi, false);
	drawTrendLine(plot[2], x, co.data3, co.nf, true);
    }

    plot[3]->clear();
    plot[3]->addCurve(co.nf, x, co.data4, stringToPixel("black"));

    i1 = (int)rint(cp.pulse_delay_min/co.dt);
    if(i1 < 0) i1 = 0;
    if(i1 > co.nf-1) i1 = co.nf-1;

    i2 = (int)rint(cp.pulse_delay_max/co.dt) + 1;
    if(i2 < 0) i2 = 0;
    if(i2 > co.nf-1) i2 = co.nf-1;
    num = i2-i1+1;

    if(num <= 0) {
	i1 = 0;
	i2 = co.nf-1;
    }

    num = 0;
    for(i = i1; i <= i2; i++) x[num++] = i*co.dt;

    plot[4]->clear();
    plot[4]->addCurve(num, x, co.data5+i1, stringToPixel("black"));

    plot[5]->clear();
    plot[5]->addCurve(num, x, co.data6+i1, stringToPixel("black"));

    Free(x);
    Free(co.data1);
    Free(co.data2);
    Free(co.data3);
    Free(co.data4);
    Free(co.data5);
    Free(co.data6);
}

int GCepstrum::getData(CepstrumStruct *cs)
{
    int i, j, k, num, npts, pts, minpts, total, num_waveforms;
    gvector<Waveform *> wvec;
    const char *double_lines[] = {"a", "b", "c", "d"};
    int num_double_lines = 4, wfid;

    if(!data_source) return 0;
    num = 0;

    num_waveforms = data_source->getSelectedWaveforms(wvec);

    pts = -1;
    minpts = -1;
    for(i = total = 0; i < num_waveforms; i++)
    {
	for(j = 0; j < num_double_lines; j++) if(cursor_toggle[j]->state())
	{
	    GDataPoint *d1, *d2;
	    for(k = 0; k < wvec[i]->num_dw; k++)
		if(wvec[i]->dw[k].label == double_lines[j][0])
	    {
		break;
	    }
	    if(k == wvec[i]->num_dw) continue;

	    d1 = wvec[i]->dw[k].d1;
	    d2 = wvec[i]->dw[k].d2;

	    if(d1->segmentIndex() != d2->segmentIndex()) continue;

	    total++;

	    npts = d2->index() - d1->index() + 1;

	    if(pts == -1) {
		pts = npts;
		minpts = pts;
	    }
	    else if(abs(npts-pts) > 2) {
		minpts = -1;
		break;
	    }
	    else if(npts < minpts) {
		minpts = npts;
	    }
	}
    }
    if(total > 0 && minpts)
    {
	for(i = 0; i < num_waveforms; i++)
	{
	    wfid = wvec[i]->ts->getWfid();

	    for(j = 0; j < num_double_lines; j++) if(cursor_toggle[j]->state())
	    {
		GDataPoint *d1, *d2;
		for(k = 0; k < wvec[i]->num_dw; k++)
		    if(wvec[i]->dw[k].label == double_lines[j][0])
		{
		    break;
		}
		if(k == wvec[i]->num_dw) continue;

		d1 = wvec[i]->dw[k].d1;
		d2 = wvec[i]->dw[k].d2;

		if(d1->segmentIndex() != d2->segmentIndex()) continue;

		npts = (minpts > 0) ?  minpts : d2->index() - d1->index() + 1;

		strcpy(cs[num].sta, wvec[i]->sta());
		strcpy(cs[num].chan, wvec[i]->chan());
		cs[num].npts = npts;
		cs[num].dt = d1->segment()->tdel();
		cs[num].time = d1->time();
		cs[num].wfid = wfid;
		char *c;
		if((c = cursor_toggle[j]->getLabel()) && strstr(c, "noise")) {
		    cs[num].signal_type = 1;
		}
		else {
		    cs[num].signal_type = 0;
		}
		Free(c);

		cs[num].data = (float *)mallocWarn(npts*sizeof(float));
		memcpy(cs[num].data, d1->segment()->data+d1->index(),
				npts*sizeof(float));
		num++;
		if(num == 2) {
		    return 2;
		}
	    }
	}
	return num;
    }

    for(j = 0; j < num_double_lines; j++)
    {
	if(cursor_toggle[j]->state() &&
		data_source->dataWindowIsDisplayed(double_lines[j])) break;
    }
    if(j == num_double_lines)
    {
	total = num_waveforms;
	for(i = 0; i < num_waveforms; i++)
	{
	    if(wvec[i]->size() > 1)
	    {
		showWarning("%s/%s: data gap.", wvec[i]->sta(),wvec[i]->chan());
		continue;
	    }
			
	    npts = wvec[i]->length();
	    wfid = wvec[i]->ts->getWfid();

	    strcpy(cs[0].sta, wvec[i]->sta());
	    strcpy(cs[0].chan, wvec[i]->chan());
	    cs[0].npts = npts;
	    cs[0].dt = wvec[i]->segment(0)->tdel();
	    cs[0].time = wvec[i]->tbeg();
	    cs[0].wfid = wfid;
	    cs[0].signal_type = j;
	    cs[0].data = (float *)mallocWarn(npts*sizeof(float));
	    memcpy(cs[0].data, wvec[i]->segment(0)->data,
			npts*sizeof(float));
	    return 1;
	}
    }
    return 0;
}

bool GCepstrum::getParam(struct CepstrumParam_s *cp)
{
    if(!param_window->getInt("smoothing npass", &cp->smoothing_npass)) {
	showWarning("invalid smoothing npass");
	return false;
    }
    if(!param_window->getDouble("smoothing width", &cp->smoothing_width)) {
	showWarning("invalid smoothing width");
	return false;
    }
    if(!param_window->getDouble("low frequency", &cp->flo)) {
	showWarning("invalid low frequency");
	return false;
    }
    if(!param_window->getDouble("high frequency", &cp->fhi)) {
	showWarning("invalid high frequency");
	return false;
    }
    if(!param_window->getDouble("guard1", &cp->guard1)) {
	showWarning("invalid guard1");
	return false;
    }
    if(!param_window->getDouble("average bandwidth1", &cp->aveband1)) {
	showWarning("invalid average bandwidth1");
	return false;
    }
    if(!param_window->getDouble("tpass", &cp->tpass)) {
	showWarning("invalid tpass");
	return false;
    }
    if(!param_window->getInt("detrend", &cp->noise_flag)) {
	showWarning("invalid detrend flag");
	return false;
    }
    if(!param_window->getDouble("pulse delay min", &cp->pulse_delay_min)) {
	showWarning("invalid pulse delay min");
	return false;
    }
    if(!param_window->getDouble("pulse delay max", &cp->pulse_delay_max)) {
	showWarning("invalid pulse delay max");
	return false;
    }
    if(!param_window->getDouble("guard2", &cp->guard2)) {
	showWarning("invalid guard2");
	return false;
    }
    if(!param_window->getDouble("average bandwidth2", &cp->aveband2)) {
	showWarning("invalid average bandwidth2");
	return false;
    }

    return true;
}

bool GCepstrum::parseSetParam(const string &cmd)
{
    string c;

    if(parseGetArg(cmd, "smoothing_npass", c)) {
	param_window->setString("smoothing npass", c);
    }
    else if(parseGetArg(cmd, "smoothing_width", c)) {
	param_window->setString("smoothing width", c);
    }
    else if(parseGetArg(cmd, "low_frequency", c)) {
	param_window->setString("low frequency", c);
    }
    else if(parseGetArg(cmd, "high_frequency", c)) {
	param_window->setString("high frequency", c);
    }
    else if(parseGetArg(cmd, "guard1", c)) {
	param_window->setString("guard1", c);
    }
    else if(parseGetArg(cmd, "average_bandwidth1", c)) {
	param_window->setString("average bandwidth1", c);
    }
    else if(parseGetArg(cmd, "tpass", c)) {
	param_window->setString("tpass", c);
    }
    else if(parseGetArg(cmd, "detrend", c)) {
	param_window->setString("detrend", c);
    }
    else if(parseGetArg(cmd, "pulse_delay_min", c)) {
	param_window->setString("pulse delay min", c);
    }
    else if(parseGetArg(cmd, "pulse_delay_max", c)) {
	param_window->setString("pulse delay max", c);
    }
    else if(parseGetArg(cmd, "guard2", c)) {
	param_window->setString("guard2", c);
    }
    else if(parseGetArg(cmd, "average_bandwidth2", c)) {
	param_window->setString("average bandwidth2", c);
    }
    else {
	return false;
    }
    return true;
}

void GCepstrum::selectCursor(Toggle *toggle)
{
    int i, j;
    char label[100];
    int num, num_double_lines = 4;
    const char *double_lines[] =
    {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};

    /* Find this toggle.
     */
    for(j = 0; j < num_double_lines && toggle != cursor_toggle[j]; j++);
    if(j == num_double_lines) return; /* should not happen */

    /* Only allow two toggles to be on. The first selected will be the
     * signal and the second selected will be the noise.
     */
    if(!toggle->state()) {
	/* reset the label of this toggle to its letter only
	 */
	toggle->setLabel(double_lines[j]);

	/* If another toggle is set, it must be the signal.
	 */
	for(i = 0; i < num_double_lines; i++) if(cursor_toggle[i]->state())
	{
	    snprintf(label, sizeof(label), "%s: signal", double_lines[i]);
	    cursor_toggle[i]->setLabel(label);
	}
    }
    else {
	/* If two toggles are already set, then unset this one.
	 */
	num = 0;
	for(i = 0; i < num_double_lines; i++)
	    if(toggle != cursor_toggle[i] && cursor_toggle[i]->state())
	{
	    num++;
	    if(num >= 2) {
		toggle->set(false);
		return;
	    }
	}

	if(!num) {
	    snprintf(label, sizeof(label), "%s: signal", double_lines[j]);
	}
	else {
	    snprintf(label, sizeof(label), "%s: noise", double_lines[j]);
	}
	toggle->setLabel(label);
    }
}

void GCepstrum::cepstrumLimits(CPlotClass *cp, AxesCursorCallbackStruct *p)
{
    AxesCursorCallbackStruct *q;

    setText(p->label, p->scaled_x);

    if(p->reason == AXES_PHASE_LINE_POSITION)
    {
	char *s;
	double df, min, max;
	vector<CPlotCurve> curves;

	if(cp->getCurves(curves) < 1 || curves[0].npts < 2) return;
	df = curves[0].x[1] - curves[0].x[0];
	min = curves[0].x[0];
	max = curves[0].x[curves[0].npts-1];

	if(p->scaled_x < min) {
	    p->scaled_x = min;
	    cp->positionPhaseLine2(p->label, p->scaled_x, false);
	    setText(p->label, p->scaled_x);
	}
	else if(p->scaled_x > max) {
	    p->scaled_x = max;
	    cp->positionPhaseLine2(p->label, p->scaled_x, false);
	    setText(p->label, p->scaled_x);
	}
	if(!strcmp(p->label, "low"))  {
	    if((cp->phaseLine("high", &q))) {
		if(p->scaled_x > q->scaled_x - 4*df) {
		    p->scaled_x = min;
		    cp->positionPhaseLine(p->label, p->scaled_x, false);
		    setText(p->label, p->scaled_x);
		}
	    }
	}
	else {
	    if((cp->phaseLine("low", &q))) {
		if(p->scaled_x < q->scaled_x + 4*df) {
		    p->scaled_x = max;
		    cp->positionPhaseLine2(p->label, p->scaled_x, false);
		    setText(p->label, p->scaled_x);
		}
	    }
	}

	drawTrendLine(cp, curves[0].x, curves[0].y, curves[0].npts, False);

	if((s = tab->labelOnTop())) {
	    strncpy(compute_tab, s, sizeof(compute_tab));
	}
    }
}

void GCepstrum::setText(char *label, double x)
{
    char s[20];

    if(!strcmp(label, "low")) {
	snprintf(s, sizeof(s), "%.3lf", x);
	param_window->setString("low frequency", s);
    }
    else if(!strcmp(label, "high")) {
	snprintf(s, sizeof(s), "%.3lf", x);
	param_window->setString("high frequency", s);
    }
}

void GCepstrum::frequencyLimits(char *name)
{
    int npts;
    float *data, xmin, xmax;
    double *x, f;
    CPlotClass *cplot;
    vector<CPlotCurve> curves;

    if(!strcmp(name, "low")) {
	if(!param_window->getDouble("low frequency", &f)) return;
    }
    else if(!strcmp(name, "high")) {
	if(!param_window->getDouble("high frequency", &f)) return;
    }
    else return;

    if(plot[1]->getCurves(curves) == 2 && curves[1].npts == 2) {
	cplot = plot[1];
    }
    else {
	if(plot[2]->getCurves(curves) == 2 && curves[1].npts == 2) {
	    cplot = plot[2];
	}
	else {
	    return;
	}
    }
    npts = curves[0].npts;
    if(npts <= 0) {
	return;
    }
    data = curves[0].y;
    x = curves[0].x;
    xmin = curves[0].x[0];
    xmax = curves[0].x[npts-1];

    if(f < xmin || f > xmax) return;

    cplot->positionPhaseLine2((char *)name, f, false);

    drawTrendLine(cplot, x, data, npts, false);
}

void GCepstrum::drawTrendLine(CPlotClass *cplot, double *f, float *data, int nf,
		bool add)
{
    int i, if1, if2, npts;
    float y[2];
    double x[2], df, flo, fhi, slope, intercept, nyquist;
    double sumx, sumy, xmean, t, sumt2;

    if(	!param_window->getDouble("low frequency", &flo) ||
	!param_window->getDouble("high frequency", &fhi)) return;

    df = f[1] - f[0];

    nyquist = (nf-1)*df;
    if(fhi > nyquist) fhi = nyquist;

    if1 = (int)(flo/df + .5);
    if(if1 < 0) if1 = 0;
    if(if1 > nf-1) if1 = nf-1;
    if2 = (int)(fhi/df + .5);
    if(if2 < 0) if1 = 0;
    if(if2 > nf-1) if2 = nf-1;

    npts = if2 - if1 + 1;

    /* Compute slope and intercept of linear least square fit
     */
    sumx = 0.;
    sumy = 0.;
    for(i = 0; i < npts; i++) {
	sumx += f[if1+i];
	sumy += data[if1+i];
    }
    xmean = sumx/(double)npts;
    sumt2 = 0.;
    slope = 0.;
    for(i = 0; i < npts; i++) {
	t = f[if1+i] - xmean;
	sumt2 += t*t;
	slope += t*data[if1+i];
    }
    slope /= sumt2;
    intercept = (sumy - sumx*slope)/(double)npts;

    x[0] = flo;
    x[1] = fhi;
    y[0] = slope*x[0] + intercept;
    y[1] = slope*x[1] + intercept;

    if(add) {
	cplot->addCurve(2, x, y, stringToPixel("red"));
    }
    else {
	cplot->changeCurve(1, 2, x, y);
    }
}

void GCepstrum::selectTab(char *label)
{
    int j, n;
    char **tab_labels=NULL;

    /* If a tab to the right of the compute_tab is selected, then recompute.
     */
    if(compute_tab[0] != '\0' && strcmp(compute_tab, label))
    {
	n = tab->getLabels(&tab_labels);
	for(j = 0; j < n && strcmp(tab_labels[j], label)
		&& strcmp(tab_labels[j], compute_tab); j++);
	if(j < n && !strcmp(tab_labels[j], compute_tab)) {
	    setCursor("hourglass");
	    compute();
	    setCursor("default");
	}
	Free(tab_labels);
    }
}

void GCepstrum::print(FILE *fp, PrintParam *p)
{
    Widget w;

    if((w = tab->getTabOnTop()))
    {
	int i;
	for(i = 0; i < 6 && w != form[i]->baseWidget(); i++);
	if(i < 6) {
	    plot[i]->hardCopy(fp, p, NULL);
	}
    }
}
