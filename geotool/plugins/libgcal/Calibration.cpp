/** \file Calibration.cpp
 *  \brief Defines class Calibration
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

#include "Calibration.h"
#include "motif++/MotifClasses.h"
#include "widget/PrintDialog.h"
#include "widget/Table.h"
#include "CalParam.h"
#include "Iteration.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "libcalib.h"
}

using namespace libgcal;

Calibration::Calibration(const char *name, Component *parent) :
		WaveformWindow(name, parent)
{
    createInterface();
    init();
}

void Calibration::createInterface()
{
    int n;
    Arg args[20];

    // redo the WaveformWindow menubar and toolbar

    tool_bar->removeAllItems();

    if(file_menu) file_menu->setVisible(false);
    if(edit_menu) edit_menu->setVisible(false);
    if(view_menu) view_menu->setVisible(false);
    if(option_menu) option_menu->setVisible(false);
    if(help_menu) help_menu->setVisible(false);

    file_menu = new Menu("File", menu_bar);
    open_button = new Button("Open...", file_menu, this);
    compute_button = new Button("Compute", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    view_menu = new Menu("View", menu_bar);
    align_button = new Button("Align On First Point", view_menu, this);
    data_movement_menu = new Menu("Data Movement", view_menu, true);
    wt.no_movement = new Toggle("no movement", data_movement_menu, this);
    wt.no_movement->set(true);
    wt.xy_movement = new Toggle("xy movement", data_movement_menu, this);
    wt.x_movement = new Toggle("x movement", data_movement_menu, this);
    wt.y_movement = new Toggle("y movement", data_movement_menu, this);
    scale_menu = new Menu("Scale", view_menu, false);
    space_more  = new Button("Space More", scale_menu, this);
    space_less  = new Button("Space Less", scale_menu, this);
    scale_up  = new Button("Scale Up", scale_menu, this);
    scale_down  = new Button("Scale Down", scale_menu, this);
    wt.uniform_scale = new Toggle("Uniform Scale", scale_menu, this);
    unzoom_all = new Button("Unzoom All", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    parameters_button = new Button("Parameters...", option_menu, this);
    interation_button = new Button("Iteration...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    cal_help_button = new Button("Calibration Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableTitle, "Final System Parameters"); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 1); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    table = new Table("table", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, table->baseWidget()); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_SECONDS); n++;
    XtSetArg(args[n], XtNxLabel, "Seconds"); n++;
    XtSetArg(args[n], XtNdisplayTags, True); n++;
    XtSetArg(args[n], XtNdisplayAmplitudeScale, True); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    XtSetArg(args[n], XtNallowPartialSelect, False); n++;
    XtSetArg(args[n], XtNwidth, 520); n++;
    XtSetArg(args[n], XtNheight, 500); n++;
    wplot->setValues(args, n);

    cal_param_window = new CalParam("Calibration Parameters", this, this);
    iteration_window = new Iteration("Calibration Iteration", this);

//    print_window = NULL;

//    addPlugins("Calibration", NULL, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(open_button, "Open");
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
	tool_bar->add(space_more, ">");
	tool_bar->add(space_less, "<");
	tool_bar->add(scale_up, "+");
	tool_bar->add(scale_down, "-");
    }
}

Calibration::~Calibration(void)
{
    Free(p);
}

void Calibration::init(void)
{
    p = (CalibParam *)mallocWarn(sizeof(CalibParam));
}

void Calibration::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Open...")) {
	open();
	return;
    }
    WaveformWindow::actionPerformed(action_event);

    if(!strcmp(cmd, "Parameters...")) {
	cal_param_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Iteration...")) {
	iteration_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Compute")) {
	p->parfile[0] = '\0';
	if(getInterfaceParams() && !CalibGetStartParam(p)) {
	    setCursor("hourglass");
	    compute();
	    setCursor("default");
	}
    }
    else if(comp == cal_param_window) {
	if((long)action_event->getCalldata() == 0) {
	    p->parfile[0] = '\0';
	    if(getInterfaceParams() && !CalibGetStartParam(p)) {
		setCursor("hourglass");
		compute();
		setCursor("default");
	    }
	}
	else {
	    iteration_window->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "Calibration Help")) {
	showHelp(cmd);
    }
}

ParseCmd Calibration::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "compute")) {
	compute_button->activate();
    }
    else if(parseCompare(cmd, "Align_On_First_Point")) {
	align_button->activate();
    }
    else if(parseArg(cmd, "Data_Movement", c)) {
	return data_movement_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "open file", c)) {
	readParFile(c);
    }
    else if(parseArg(cmd, "alias", c)) {
	cal_param_window->setString("alias", c);
    }
    else if(parseArg(cmd, "m", c)) {
	cal_param_window->setString("m", c);
    }
    else if(parseArg(cmd, "m0", c)) {
	cal_param_window->setString("m0", c);
    }
    else if(parseArg(cmd, "m1", c)) {
	cal_param_window->setString("m1", c);
    }
    else if(parseArg(cmd, "m2", c)) {
	cal_param_window->setString("m2", c);
    }
    else if(parseArg(cmd, "maxit", c)) {
	cal_param_window->setString("maxit", c);
    }
    else if(parseArg(cmd, "qac", c)) {
	cal_param_window->setString("qac", c);
    }
    else if(parseArg(cmd, "finac", c)) {
	cal_param_window->setString("finac", c);
    }
    else if(parseArg(cmd, "ns1", c)) {
	cal_param_window->setString("ns1", c);
    }
    else if(parseArg(cmd, "ns2", c)) {
	cal_param_window->setString("ns2", c);
    }
    else if(parseArg(cmd, "input_file", c)) {
	cal_param_window->setString("input_file", c);
    }
    else if(parseArg(cmd, "output file", c)) {
	cal_param_window->setString("output_file", c);
    }
    else if(parseArg(cmd, "title", c)) {
	cal_param_window->setString("partitle", c);
    }
    else if(parseCompare(cmd, "Space_More")) {
	space_more->activate();
    }
    else if(parseCompare(cmd, "Space_Less")) {
	space_less->activate();
    }
    else if(parseCompare(cmd, "Scale_Up")) {
	scale_up->activate();
    }
    else if(parseCompare(cmd, "Scale_Down")) {
	scale_down->activate();
    }
    else if(parseArg(cmd, "Uniform_Scale", c)) {
	return wt.uniform_scale->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Unzoom_All")) {
	unzoom_all->activate();
    }
    else if(parseArg(cmd, "print_window", c)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print Waveforms", this, this);
	}
	return print_window->parseCmd(c, msg);
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

ParseVar Calibration::parseVar(const string &name, string &value)
{
    string c;

/* also return final system parameters by name */
    if(parseString(name, "table", c)) {
        return table->parseVar(c, value);
    }
    return WaveformWindow::parseVar(name, value);
}

void Calibration::parseHelp(const char *prefix)
{
    printf("%scompute\n", prefix);
    printf("%salign_on_first_point\n", prefix);

    char s[200];
    snprintf(s, sizeof(s), "%sdata_movement.", prefix);
    data_movement_menu->parseHelp(s);

    printf("%sopen file=FILEMAME\n", prefix);

    printf("%salias=NUM\n", prefix);
    printf("%sm=NUM\n", prefix);
    printf("%sm0=NUM\n", prefix);
    printf("%sm1=NUM\n", prefix);
    printf("%sm2=NUM\n", prefix);
    printf("%smaxit=NUM\n", prefix);
    printf("%sqac=NUM\n", prefix);
    printf("%sfinac=NUM\n", prefix);
    printf("%sns1=NUM\n", prefix);
    printf("%sns2=NUM\n", prefix);
    printf("%sns2=input file=FILENAME\n", prefix);
    printf("%sns2=output file=FILENAME\n", prefix);
    printf("%stitle=TITLE\n", prefix);

    printf("%sspace_more\n", prefix);
    printf("%sspace_less\n", prefix);
    printf("%sscale_up\n", prefix);
    printf("%sscale_down\n", prefix);
    printf("%suniform_scale down\n", prefix);
    printf("%sunzoom_all\n", prefix);
    printf("%sprint.help\n", prefix);
}

void Calibration::printWindow(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print Calibration", this, this);
    }
    print_window->setVisible(true);
}

void Calibration::compute(void)
{
    int npts;
    CalibSignal in_sig, out_sig;
    CalibOut co;

    if(CalibReadSignal(p->eing, INPUT_SIGNAL, &in_sig)) return;
    if(CalibReadSignal(p->ausg, OUTPUT_SIGNAL, &out_sig)) return;
    if(in_sig.dt != out_sig.dt) {
       showWarning("Sampling rates of input and output data are inconsistent.");
	Free(in_sig.x); Free(out_sig.x);
	return;
    }
    if(fabs(60.*(in_sig.tmin-out_sig.tmin)+in_sig.tsec-out_sig.tsec) > 0.01)
    {
	showWarning("Time tags of input and output data are inconsistent.");
	Free(in_sig.x); Free(out_sig.x);
	return;
    }
    if(p->alias < 4.0*out_sig.dt) {
	showWarning("Specified anti-alias corner period is too small: %.3lf",
			p->alias);
	Free(in_sig.x); Free(out_sig.x);
	return;
    }

    npts = (in_sig.npts < out_sig.npts) ? in_sig.npts : out_sig.npts;

    if(p->ns1 <= 0) p->ns1 = 1;
    if(p->ns2 <= 0 || p->ns2 > npts) p->ns2 = npts;
    if(p->ns2 <= p->ns1) {
	showWarning("incorrect time window - check ns1, ns2");
	Free(in_sig.x); Free(out_sig.x);
	return;
    }

    if(!CalibCompute(p, in_sig.dt, npts, in_sig.x, out_sig.x, &co)) {
	displayData(in_sig.x, out_sig.x, &co);
    }

    Free(in_sig.x);
    Free(out_sig.x);
    Free(co.ausf);
    Free(co.einf);
    Free(co.synt1);
    Free(co.synt2);
    Free(co.rest);
}

//void Calibration::displayData(double *in, double *out, CalibOut *co)
void Calibration::displayData(double *in, double *out, struct CalibOut_s *co)
{
    const char **c=NULL;
    Arg args[2];
    int i, j, k, n, num_waveform_colors, *alignment=NULL;
    float *data=NULL;
    double tmin, tmax, duration;
    CPlotInputStruct input;
    GTimeSeries *ts;
    Pixel *waveform_fg;

    data = (float *)malloc(co->npts*sizeof(float));

    input.color = stringToPixel("black");

    wplot->clear();

    XtSetArg(args[0], XtNtitle, p->partitle);
    wplot->setValues(args, 1);

    duration = (co->npts-1)*co->dt;
    tmin = -.15*duration;
    tmax = 1.05*duration;

    wplot->setTimeLimits(tmin, tmax);
    wplot->setMaxTagWidth(25);

    num_waveform_colors = WaveformPlot::getWaveformColors(this, &waveform_fg);

    ts = new GTimeSeries();
    for(i = 0; i < co->npts; i++) data[i] = in[i];
    ts->addSegment(new GSegment(data, co->npts, 0., co->dt, 1., 0.));
    input.color = waveform_fg[0];
    ts->setSta("input");
    input.tag.assign("input");
    wplot->addTimeSeries(ts, &input);

    ts = new GTimeSeries();
    for(i = 0; i < co->npts; i++) data[i] = out[i];
    ts->addSegment(new GSegment(data, co->npts, 0., co->dt, 1., 0.));
    input.color = waveform_fg[1% num_waveform_colors];
    ts->setSta("output");
    input.tag.assign("output");
    wplot->addTimeSeries(ts, &input);

    ts = new GTimeSeries();
    for(i = 0; i < co->npts; i++) data[i] = co->einf[i];
    ts->addSegment(new GSegment(data, co->npts, 0., co->dt, 1., 0.));
    input.color = waveform_fg[2% num_waveform_colors];
    ts->setSta("f-in");
    input.tag.assign("filtered\ninput");
    wplot->addTimeSeries(ts, &input);

    ts = new GTimeSeries();
    for(i = 0; i < co->npts; i++) data[i] = co->ausf[i];
    ts->addSegment(new GSegment(data, co->npts, 0., co->dt, 1., 0.));
    input.color = waveform_fg[3% num_waveform_colors];
    ts->setSta("f-out");
    input.tag.assign("filtered\noutput");
    wplot->addTimeSeries(ts, &input);

    ts = new GTimeSeries();
    for(i = 0; i < co->npts; i++) data[i] = co->synt2[i];
    ts->addSegment(new GSegment(data, co->npts, 0., co->dt, 1., 0.));
    input.color = waveform_fg[4% num_waveform_colors];
    ts->setSta("synt");
    input.tag.assign("synthetic");
    wplot->addTimeSeries(ts, &input);

    ts = new GTimeSeries();
    for(i = 0; i < co->npts; i++) data[i] = co->rest[i];
    ts->addSegment(new GSegment(data, co->npts, 0., co->dt, 1., 0.));
    input.color = waveform_fg[5% num_waveform_colors];
    ts->setSta("resid");
    input.tag.assign("residual");
    wplot->addTimeSeries(ts, &input);

    Free(data);

    table->clear();
    iteration_window->table->clear();

    if(!(c = (const char **)mallocWarn((co->npar+2)*sizeof(char *)))) {
	return;
    }

    c[0] = "iter";
    c[1] = "RMS";
    for(i = 0; i < co->npar; i++) {
	c[2+i] = p->name[i];
    }
    XtSetArg(args[0], XtNcolumns, co->npar+2);
    XtSetArg(args[1], XtNcolumnLabels, c);
    table->setValues(args, 2);

    if(!(alignment = (int *)mallocWarn((co->npar+2)*sizeof(int)))) {
	return;
    }
    for(i = 0; i < co->npar+2; i++) {
	alignment[i] = RIGHT_JUSTIFY;
    }
    table->setAlignment(co->npar+2, alignment);

    iteration_window->table->setValues(args, 2);
    iteration_window->table->setAlignment(co->npar+2, alignment);
    Free(alignment);

    for(i = 0; i < co->npar+2; i++) {
	c[i] = (char *)mallocWarn(25);
    }
    snprintf((char *)c[0], 20, "%2d", co->iter);
    snprintf((char *)c[1], 20, "%11.6f", co->final_rms);

    for(i = 0; i < co->npar; i++) {
	snprintf((char *)c[2+i], 20, "%11.6f", co->final_par[i]);
    }
    table->addRow(c, false);
    table->adjustColumns();


    snprintf((char *)c[0], 20, "0");
    snprintf((char *)c[1], 20, "%11.6f", co->initial_rms);

    for(i = 0; i < co->npar; i++) {
	snprintf((char *)c[2+i], 20, "%11.6f", p->x00[i]);
    }
    iteration_window->table->addRow(c, false);

    snprintf((char *)c[0], 20, "%s", "");
    snprintf((char *)c[1], 20, "+-");
    for(i = 0; i < co->npar; i++) {
	snprintf((char *)c[2+i], 20, "%11.6f", p->r00[i]);
    }
    iteration_window->table->addRow(c, false);

    n = co->npar + 1;
    for(i = k = 0; i < co->iter; i++) {
	snprintf((char *)c[0], 20, "%d", i+1);
	for(j = 0; j < n; j++) {
	    snprintf((char *)c[1+j], 20, "%11.6f", co->x[i*n+j]);
	}
	iteration_window->table->addRow(c, false);
    }

    iteration_window->table->adjustColumns();

    for(i = 0; i < co->npar+2; i++) free((char *)c[i]);
    Free(c);
}

void Calibration::open(void)
{
    string file;

    if( FileDialog::getFile("Calibration Open", this, file, "./", "*.par") )
    {
	if(file.empty() || file[(int)file.length()-1] == '/')
	{
	    showWarning("Calibration.open: No file selected.");
	    return;
	}
	readParFile(file);
    }
}

bool Calibration::readParFile(const string &file)
{
    DIR *dirp;

    if((dirp = opendir(file.c_str())) != NULL) {
	closedir(dirp);
	showWarning(file + " is a directory.");
	return false;
    }
    setCursor("hourglass");

    if(!CalibReadPar((char *)file.c_str(), p)) {
	putInterfaceParams();
	cal_param_window->setVisible(true);
	setCursor("default");
	return true;
    }
    setCursor("default");
    return false;
}

void Calibration::putInterfaceParams(void)
{
    char s[20];

    ftoa(p->alias, 6, 0, s, sizeof(s));
    cal_param_window->setString("alias", s);

    snprintf(s, sizeof(s), "%d", p->m);
    cal_param_window->setString("m", s);

    snprintf(s, sizeof(s), "%d", p->m0i);
    cal_param_window->setString("m0", s);

    snprintf(s, sizeof(s), "%d", p->m1i);
    cal_param_window->setString("m1", s);

    snprintf(s, sizeof(s), "%d", p->m2i);
    cal_param_window->setString("m2", s);

    snprintf(s, sizeof(s), "%d", p->maxit);
    cal_param_window->setString("maxit", s);

    ftoa(p->qac, 6, 0, s, sizeof(s));
    cal_param_window->setString("qac", s);

    ftoa(p->finac, 6, 0, s, sizeof(s));
    cal_param_window->setString("finac", s);

    snprintf(s, sizeof(s), "%d", p->ns1);
    cal_param_window->setString("ns1", s);

    snprintf(s, sizeof(s), "%d", p->ns2);
    cal_param_window->setString("ns2", s);

    int n = strlen(p->raw);
    int j = 1;
    for(int i = 1; i < n; i++) {
	if(p->raw[i] == ' ' || p->raw[i] == '\t') {
	    if(p->raw[i-1] != ' ' && p->raw[i-1] != '\t') {
		p->raw[j++] = '\t';
	    }
	}
	else {
	    p->raw[j++] = p->raw[i];
	}
    }
    p->raw[j] = '\0';
    cal_param_window->setString("param_text", p->raw);
    cal_param_window->setString("input_file", p->eing);
    cal_param_window->setString("output_file", p->ausg);
    cal_param_window->setString("partitle", p->partitle);
}

bool Calibration::getInterfaceParams(void)
{
    if(!cal_param_window->getDouble("alias", &p->alias)) {
	showWarning("Invalid alias.");
	return false;
    }
    if(!cal_param_window->getInt("m", &p->m) || p->m <= 0) {
	showWarning("Invalid m.");
	return false;
    }
    if(!cal_param_window->getInt("m0", &p->m0i) || p->m0i < 0) {
	showWarning("Invalid m0.");
	return false;
    }
    if(!cal_param_window->getInt("m1", &p->m1i) || p->m1i < 0) {
	showWarning("Invalid m1.");
	return false;
    }
    if(!cal_param_window->getInt("m2", &p->m2i) || p->m2i < 0) {
	showWarning("Invalid m2.");
	return false;
    }
    if(!cal_param_window->getInt("maxit", &p->maxit) || p->maxit <= 0) {
	showWarning("Invalid maxit.");
	return false;
    }
    if(!cal_param_window->getDouble("maxit", &p->qac)) {
	showWarning("Invalid qac.");
	return false;
    }
    if(!cal_param_window->getDouble("finac", &p->finac)) {
	showWarning("Invalid finac.");
	return false;
    }
    if(!cal_param_window->getInt("ns1", &p->ns1) || p->ns1 < 1) {
	showWarning("Invalid ns1.");
	return false;
    }
    if(!cal_param_window->getInt("ns2", &p->ns2) || p->ns2 < 0
			|| (p->ns2 != 0 && p->ns2 < p->ns1))
    {
	showWarning("Invalid ns2.");
	return false;
    }

    cal_param_window->getString("param_text", p->raw, sizeof(p->raw));
    cal_param_window->getString("input_file", p->eing, sizeof(p->eing));
    cal_param_window->getString("output_file", p->ausg, sizeof(p->ausg));
    cal_param_window->getString("partitle", p->partitle,
			sizeof(p->partitle));

    return true;
}
