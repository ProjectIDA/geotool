/** \file FkSignal.cpp
 *  \brief Defines class FkSignal.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "FkSignal.h"
#include "FK.h"
#include "FKParam.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"

using namespace libgfk;

FkSignal::FkSignal(const char *name, Component *parent, FK *fk_src) :
		FormDialog(name, parent, true, true)
{
    fk = fk_src;
    createInterface();
    ignore_callback = false;
}

void FkSignal::createInterface()
{
    Arg args[30];
    int n, num_signal_plots;

    info_area = new InfoArea("infoArea", this);
    info_area->setVisible(true);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, info_area->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    print_button = new Button("Print...", controls, this);
    param_button = new Button("Parameters...", controls, fk);

    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    unwrap_toggle = new Toggle("Unwrap Azimuth", controls, args, n);
    unwrap_toggle->setVisible(false);
//    unwrap_toggle->set(true);

    auto_compute_button = new Button("Auto Compute", controls, fk);
    start_button = new Button("Start", controls, fk);
    start_button->setSensitive(false);
    back_button = new Button("-", controls, this);
    back_button->setSensitive(false);
    forward_button = new Button("+", controls, this);
    forward_button->setSensitive(false);
    if(fk->nbands > 1)
    {
	new Space("space", controls, XmHORIZONTAL, 40);
	new Label("Display Curve", controls);
	n = 0;
	XtSetArg(args[n], XmNselectColor, stringToPixel("medium blue")); n++;
	XtSetArg(args[n], XmNunselectColor, stringToPixel("medium blue")); n++;
	show_toggles[0] = new Toggle(" ", controls, this, args, n);
	n = 0;
	XtSetArg(args[n], XmNselectColor, stringToPixel("forest green")); n++;
	XtSetArg(args[n], XmNunselectColor, stringToPixel("forest green")); n++;
	show_toggles[1] = new Toggle(" ", controls, this, args, n);
	n = 0;
	XtSetArg(args[n], XmNselectColor, stringToPixel("sky blue")); n++;
	XtSetArg(args[n], XmNunselectColor, stringToPixel("sky blue")); n++;
	show_toggles[2] = new Toggle(" ", controls, this, args, n);
	n = 0;
	XtSetArg(args[n], XmNselectColor, stringToPixel("orange")); n++;
	XtSetArg(args[n], XmNunselectColor, stringToPixel("orange")); n++;
	show_toggles[3] = new Toggle(" ", controls, this, args, n);

	for(int i = 0; i < 4; i++) show_toggles[i]->set(true, false);
    }

    save_button = new Button("Save...", controls, this);

    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    rc = new RowColumn("rc", this, args, n);

    window_length_label = new Label("Window Length (secs)", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    window_length_text = new TextField("window_length", rc, args, n);
    window_length_text->addActionListener(fk, XmNvalueChangedCallback);

    new Space("space", rc, XmHORIZONTAL, 10);

    window_overlap_label = new Label("Window Overlap (secs)", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    window_overlap_text = new TextField("window_overlap", rc, args, n);
    window_overlap_text->addActionListener(fk, XmNvalueChangedCallback);

    stav_length_label = new Label("stav (secs)", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    stav_length_text = new TextField("stav_length", rc, args, n);
    stav_length_text->addActionListener(fk, XmNvalueChangedCallback);

    ltav_length_label = new Label("ltav (secs)", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    ltav_length_text = new TextField("ltav_length", rc, args, n);
    ltav_length_text->addActionListener(fk, XmNvalueChangedCallback);

    if(fk->nbands == 1) {
	new Space("space", rc, XmHORIZONTAL, 10);
	scan_frequencies_toggle = new Toggle("Scan Frequencies", rc);
	scan_frequencies_toggle->addActionListener(fk, XmNvalueChangedCallback);

	bandwidth_label = new Label("Bandwidth(Hz)", rc);
	bandwidth_label->setSensitive(false);
	n = 0;
	XtSetArg(args[n], XmNcolumns, 6); n++;
	bandwidth_text = new TextField("bandwidth", rc, args, n);
	bandwidth_text->addActionListener(fk, XmNvalueChangedCallback);
	bandwidth_text->setSensitive(false);
	bandwidth_text->setString(".5");
    }

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    pane = new Pane("pane", this, args, n);

    num_signal_plots = (fk->nbands==1) ? NUM_SIGNAL_PLOTS : NUM_SIGNAL_PLOTS-1;

    int height = getProperty("fksignal_plot_height", 80);
    int num_visible_plots = getProperty("fksignal_plot_visible",
				num_signal_plots);
    if(num_visible_plots < 0) {
	num_visible_plots = num_signal_plots;
    }
    else if(num_visible_plots > num_signal_plots) {
	num_visible_plots = num_signal_plots;
    }

    int h = num_visible_plots*height;
    n = 0;
    XtSetArg(args[n], XmNheight, h); n++;
    XtSetArg(args[n], XmNallowResize, True); n++;
    scrolled_pane = new ScrolledPane("scrolled_pane", pane, num_visible_plots,
                                0, args, n);
    scrolled_pane->addActionListener(this, XmNresizeCallback);

    const char *ylabels[NUM_SIGNAL_PLOTS] =
	{"SNR","Fstat","App. Vel.","Slowness","Azimuth","Delaz","Delslo"};
    int order[NUM_SIGNAL_PLOTS];

    getPlotOrder(order);

    for(int i = 0; i < NUM_SIGNAL_PLOTS; i++)
    {
	char name[30];
	int j = order[i];
	snprintf(name, sizeof(name), "signal_plot%d", j+1);

	n = 0;
	XtSetArg(args[n], XtNverticalScroll, false); n++;
	XtSetArg(args[n], XtNhorizontalScroll, false); n++;
//	XtSetArg(args[n], XtNtickmarksInside, false); n++;
	XtSetArg(args[n], XtNtimeScale, TIME_SCALE_HMS); n++;
//	XtSetArg(args[n], XtNclearOnScroll, false); n++;
	XtSetArg(args[n], XtNsaveSpace, True); n++;
	XtSetArg(args[n], XtNcursorLabelAlways, True); n++;
	XtSetArg(args[n], XtNminYLab, 2); n++;
	XtSetArg(args[n], XtNmaxYLab, 5); n++;
	XtSetArg(args[n], XtNminYSmall, 0); n++;
	XtSetArg(args[n], XtNmaxYSmall, 0); n++;
	XtSetArg(args[n], XtNdisplayAxesLabels, AXES_Y); n++;
	XtSetArg(args[n], XtNdisplayXLabel, false); n++;
	XtSetArg(args[n], XtNscrollbarAsNeeded, True); n++;
	XtSetArg(args[n], XtNyLabelInt, false); n++;
	XtSetArg(args[n], XtNautoYScale, True); n++;
	XtSetArg(args[n], XtNyLabel, ylabels[j]); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XtNleftMargin, 15); n++;
	XtSetArg(args[n], XtNtopMargin, 2); n++;
	XtSetArg(args[n], XmNborderWidth, 0); n++;
	plots[j] = new CPlotClass(name, scrolled_pane->getForm(), info_area,
					args, n);
	plots[j]->setYLimits(-1., 1., true);
	plots[j]->setCurveMargins(.03, .03, .1, .1);
	plots[j]->addActionListener(fk, XtNphaseLineDragCallback);
	plots[j]->addActionListener(fk, XtNphaseLineCallback);
	plots[j]->addActionListener(this, XtNlimitsCallback);
    }

    n = 0;
    XtSetArg(args[n], XmNheight, 150); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    XtSetArg(args[n], XtNdisplayTtLabels, True); n++;
    beam_plot = new WaveformView("signal_plot0", pane, info_area, args, n);
    beam_plot->selectIaspeiPhases(true);
//    beam_plot->addActionListener(fk, XtNphaseLineDragCallback);
//    beam_plot->addActionListener(fk, XtNphaseLineCallback);
    beam_plot->addActionListener(this, XtNlimitsCallback);
    beam_plot->addActionListener(fk, XtNdoubleLineDragCallback);
    beam_plot->addActionListener(fk, XtNdoubleLineCallback);

    plots[NUM_SIGNAL_PLOTS] = beam_plot;

    print_window = NULL;
}

FkSignal::~FkSignal(void)
{
}

void FkSignal::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(ignore_callback) return;

    if(!strcmp(reason, XtNlimitsCallback))
    {
	tieLimits(comp,(AxesLimitsCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Save...")) {
	char *prefix = FileDialog::getFile("FK-Measurements", this, "./",
				"*.asc");
	if(prefix) {
	    fk->saveSignals(prefix);
	    free(prefix);
	}
    }
    else if(!strcmp(cmd, "-")) {
	fk->autoStep(false);
    }
    else if(!strcmp(cmd, "+")) {
	fk->autoStep(true);
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("FK Signal Help");
    }
    else if(comp == scrolled_pane) {
	char s[20];
	int form_height = scrolled_pane->getForm()->getHeight();
	int child_height = scrolled_pane->getChildHeight();
	snprintf(s, sizeof(s), "%d", child_height);
	putProperty("fksignal_plot_height", s);

	int num_visible_plots = (int)((double)form_height/child_height + .5);
	snprintf(s, sizeof(s), "%d", num_visible_plots);
	putProperty("fksignal_plot_visible", s);
    }
    else if(fk->nbands > 1) {
	for(int i = 0; i < 4; i++) {
	    if(show_toggles[i] == comp) {
		for(int j = 0; j < NUM_SIGNAL_PLOTS; j++) {
		    plots[j]->setCurveVisible(i, show_toggles[i]->state(),true);
		}
	    }
	}
    }
}

ParseCmd FkSignal::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseString(cmd, "print_window", c)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print FK Signal", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "+")) {
	fk->autoStep(true);
    }
    else if(parseCompare(cmd, "-")) {
	fk->autoStep(false);
    }
    else if(parseCompare(cmd, "Start")|| parseCompare(cmd, "Stop")) {
	fk->autoStart();
    }
    else if(parseCompare(cmd, "Auto_Compute")) {
	fk->auto_compute_button->activate();
    }
    else if(parseString(cmd, "beam_plot", c)) {
	return beam_plot->parseCmd(c, msg);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar FkSignal::parseVar(const string &name, string &value)
{
    int i, n, nextc;
    ParseVar ret;
    string s;

    n = fk->p->num_fkdata;

    if(parseString(name, "beam_plot", s)) {
	return beam_plot->parseVar(s, value);
    }
    else if(parseArrayIndex(name, "sig", fk->p->nbands, &i, &nextc, value,&ret))
    {
	if(ret != STRING_RETURNED) return ret;

	const char *c = name.c_str() + nextc;
	if( parseArray(c, ".snr", n, fk->p->sig[i].snr, value, &ret) )
		return ret;
	if( parseArray(c, ".azimuth", n, fk->p->sig[i].azimuth, value, &ret) )
		return ret;
	if( parseArray(c, ".fstat", n, fk->p->sig[i].fstat, value, &ret) )
		return ret;
	if( parseArray(c, ".appvel", n, fk->p->sig[i].appvel, value, &ret) )
		return ret;
	if( parseArray(c, ".slowness", n, fk->p->sig[i].slowness, value, &ret) )
		return ret;
	if( parseArray(c, ".delaz", n, fk->p->sig[i].delaz, value, &ret) )
		return ret;
	if( parseArray(c, ".delslo", n, fk->p->sig[i].delslo, value, &ret) )
		return ret;
	if( parseArray(c, ".time", n, fk->p->x, value, &ret) ) return ret;
    }
    return FormDialog::parseVar(name, value);
}

void FkSignal::clear(void)
{
    for(int i = 0; i < NUM_SIGNAL_PLOTS+1; i++) {
	plots[i]->clearWaveforms();
    }
}

void FkSignal::addCurve(int curve_index, int num, double *x, float *y,
			char *label, Pixel fg)
{
    if(curve_index < 0 || curve_index >= NUM_SIGNAL_PLOTS) return;

    int n = plots[curve_index]->numCurves();
    bool on  = (fk->nbands == 1) ? true : show_toggles[n]->state();
    plots[curve_index]->addCurve(num, x, y, label, on, fg);
    if(curve_index == 2) // limit app vel y scale
    {
	double xmin, xmax;
	plots[3]->getXLimits(&xmin, &xmax);
	plots[3]->zoom(xmin, xmax, 0., 50., false); // limit app vel y scale
    }
    else if(curve_index == 5)
    {
	double xmin, xmax;
	plots[5]->getXLimits(&xmin, &xmax);
	plots[5]->zoom(xmin, xmax, 0., .1, false); // limit delaz y scale
    }
}

void FkSignal::tieLimits(Component *comp, AxesLimitsCallbackStruct *s)
{
    ignore_callback = true;
    for(int i = 0; i < NUM_SIGNAL_PLOTS+1; i++) if(comp != plots[i])
    {
        if(s->x_margins) {
            plots[i]->setMargins(false, true, s->left, s->right, 0, 0);
        }
	plots[i]->setXLimits(s->x_min, s->x_max);
//	plots[i]->setLimits(s->x_min, s->x_max, s->y_min, s->y_max);
    }
    ignore_callback = false;
}

void FkSignal::print(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print FK Signal", this, this);
    }
    print_window->setVisible(true);
}

void FkSignal::print(FILE *fp, PrintParam *p)
{
    char title[200], tlab[100];
    int total_height;
    AxesParm *a, *a2;
    double f, bottom, top;
    Dimension height;
    Arg args[1];

    XtSetArg(args[0], XtNheight, &height);

    total_height = 0;
    for(int i = 0; i < NUM_SIGNAL_PLOTS+1; i++) {
	plots[i]->getValues(args, 1);
	total_height += height;
    }

    tlab[0] = '\0';
    if(fk->p->num_fkdata > 0)
    {
	FKData *fkd = fk->p->fkdata[0];
	timeEpochToString(fkd->tbeg, tlab, 100, GSE20);
    }
    snprintf(title, 200,
	"%s  %s\nwindow-length %.1fsecs  window-overlap %.1fsecs",
	fk->p->net, tlab, fk->p->window_length, fk->p->window_overlap);
    p->top_title = title;

    plots[0]->getValues(args, 1);
    f = (double)height/(double)total_height;
    bottom = p->bottom;
    top = p->top;
    p->bottom = top - f*(top - bottom);

    if( !(a = plots[0]->hardCopy(fp, p, NULL)) ) {
	return;
    }

    p->top_title = NULL;
    a->auto_x = false;

    for(int i = 1; i < NUM_SIGNAL_PLOTS+1; i++)
    {
	p->top = p->bottom;
	plots[i]->getValues(args, 1);
	f += height/(double)total_height;
	p->bottom = (i < 3) ? top - f*(top - bottom) : bottom;
	if(i == 3) {
	    p->x_axis_label = (char *)"time (seconds)";
	}
	if((a2 = plots[i]->hardCopy(fp, p, a)) == NULL)
	{
	    free(a);
	    return;
	}
        free(a2);
    }
    free(a);
}

void FkSignal::getPlotOrder(int *order)
{
    const char *ylabels[NUM_SIGNAL_PLOTS] =
	{"Fstat", "App. Vel.", "Slowness", "Azimuth", "Delaz", "Delslo"};
    const char *short_ylabels[NUM_SIGNAL_PLOTS] =
	{"fstat", "vel", "slow", "az", "delaz", "delslo"};
    char *tok, *last, *name, *prop;
    int i, n;

    if( !(prop = getProperty("fksignal_plot_order")) ) {
	for(i = 0; i < NUM_SIGNAL_PLOTS; i++) order[i] = i;
	return;
    }

    for(i = 0; i < NUM_SIGNAL_PLOTS; i++) order[i] = -1;

    n = 0;
    tok = prop;
    while((name = (char *)strtok_r(tok, ",", &last))) {
	tok = NULL;
	for(i = 0; i < NUM_SIGNAL_PLOTS; i++) {
	    if(!strncasecmp(name, ylabels[i], 3) ||
		!strcasecmp(name, short_ylabels[i]))
	    {
		order[n++] = i;
	    }
	}
    }
    Free(prop);

    // remove duplicates
    for(i = NUM_SIGNAL_PLOTS-1; i >= 0; i--) {
	if(order[i] >= 0) {
	    for(int j = i-1; j >= 0; j--) {
		if(order[j] == order[i]) {
		    for(int k = j; k < NUM_SIGNAL_PLOTS-1; k++) {
			order[k] = order[k+1];
		    }
		    order[NUM_SIGNAL_PLOTS-1] = -1;
		}
	    }
	}
    }
    // add missing
    for(int j = 0; j < NUM_SIGNAL_PLOTS; j++)
    {
	for(i = 0; i < NUM_SIGNAL_PLOTS && order[i] != j; i++);
	if(i == NUM_SIGNAL_PLOTS) { // 'j' not found
	    for(i = 0; i < NUM_SIGNAL_PLOTS; i++) { // look for available spot
		if(order[i] == -1) {
		    order[i] = j;
		    break;
		}
	    }
	}
    }
}
