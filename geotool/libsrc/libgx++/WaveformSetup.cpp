/** \file WaveformSetup.cpp
 *  \brief Defines class WaveformSetup.
 *  \author Ivan Henson
 */
#include "config.h"
#include "WaveformSetup.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
}

WaveformSetup::WaveformSetup(const string &name, Component *parent,
		ActionListener *listener, double jointime_limit,
		double overlaplimit, bool applycalib)
			: FormDialog(name, parent, false, false)
{
    join_time_limit = jointime_limit;
    overlap_limit = overlaplimit;
    apply_calib = applycalib;
    segment_length = -1.;
    createInterface();
    addActionListener(listener);
}

void WaveformSetup::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    segment_label = new Label("Segment Join Parameters", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, segment_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    rc = new RowColumn("rc", this, args, n);

    gap_label = new Label("Gap Limit (secs)", rc);
    overlap_label = new Label("Overlap Limit (secs)", rc);
    length_label = new Label("Segmentation Length (secs)", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    gap_text = new TextField("gap_text", rc, this, args, n);
    overlap_text = new TextField("overlap_text", rc, this, args, n);
    length_text = new TextField("length_text", rc, this, args, n);

    gap_text->setString("%.1f", join_time_limit);
    overlap_text->setString("%.1f", overlap_limit);
    length_text->setString("");

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    calib_label = new Label("Initial Waveform Calib Setting", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, calib_label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNspacing, 30); n++;
    rb = new RadioBox("rb", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNset, apply_calib); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    calib_toggle = new Toggle("Display Counts*Calib", rb, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNset, !apply_calib); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    counts_toggle = new Toggle("Display Counts", rb, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rb->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    cancel_button = new Button("Cancel", controls, this);

    enableCallbackType(XmNactivateCallback);
}

WaveformSetup::~WaveformSetup(void)
{
}

void WaveformSetup::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Cancel")) {
	cancel();
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
    }
    else if(comp == gap_text || comp == overlap_text || comp == length_text ||
		comp == calib_toggle || comp == counts_toggle)
    {
	setButtonsSensitive();
    }
}

void WaveformSetup::setButtonsSensitive(void)
{
    double d;

    if(gap_text->getDouble(&d) && d >= 0. && d != join_time_limit) {
	apply_button->setSensitive(true);
	return;
    }
    if(overlap_text->getDouble(&d) && d >= 0. && d != overlap_limit) {
	apply_button->setSensitive(true);
	return;
    }
    if(length_text->getDouble(&d)) {
	if((d > 0. && d != segment_length) || (d == 0. && segment_length > 0.)){
	    apply_button->setSensitive(true);
	    return;
	}
    }
    else if(segment_length > 0.) {
	apply_button->setSensitive(true);
	return;
    }

    if(apply_calib != calib_toggle->state()) {
	apply_button->setSensitive(true);
	return;
    }
    apply_button->setSensitive(false);
}

void WaveformSetup::cancel(void)
{
    setVisible(false);
    gap_text->setString("%.1f", join_time_limit);
    overlap_text->setString("%.1f", overlap_limit);
    if(segment_length > 0.) {
	length_text->setString("%.1f", segment_length);
    }
    else {
	length_text->setString("");
    }
    calib_toggle->set(apply_calib, true);
}

void WaveformSetup::apply(void)
{
    double d1, d2, d3;

    if(!gap_text->getDouble(&d1) || d1 < 0.) {
	showWarning("Invalid Gap Limit");
	return;
    }
    if(!overlap_text->getDouble(&d2) || d2 < 0.) {
	showWarning("Invalid Overlap Limit");
	return;
    }
    if(length_text->getDouble(&d3)) {
	if( d3 < 0.) {
	    showWarning("Invalid Segmentation Length");
	    return;
	}
	segment_length = d3;
    }
    else {
	segment_length = -1.;
    }
    join_time_limit = d1;
    overlap_limit = d2;
    apply_calib = calib_toggle->state();
    doCallbacks(base_widget, (XtPointer)this, XmNactivateCallback);
    setVisible(false);
    apply_button->setSensitive(false);
}

ParseCmd WaveformSetup::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "gap_limit", c)) {
	gap_text->setString(c, true);
	apply();
    }
    else if(parseArg(cmd, "overlap_limit", c)) {
	overlap_text->setString(c, true);
	apply();
    }
    else if(parseArg(cmd, "segmentation_length", c)) {
	length_text->setString(c, true);
	apply();
    }
    else if(parseCompare(cmd, "calib")) {
	calib_toggle->set(true, true);
	apply();
    }
    else if(parseCompare(cmd, "counts")) {
	counts_toggle->set(true, true);
	apply();
    }
    else if(parseCompare(cmd, "apply")) {
	apply();
    }
    else if(parseCompare(cmd, "cancel")) {
	setVisible(false);
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

void WaveformSetup::parseHelp(const char *prefix)
{
    printf("%sgap_limit=SECONDS\n", prefix);
    printf("%soverlap_limit=SECONDS\n", prefix);
    printf("%scounts*calib\n", prefix);
    printf("%scounts\n", prefix);
    printf("%sapply\n", prefix);
    printf("%scancel\n", prefix);
}
