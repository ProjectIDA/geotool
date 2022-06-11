/** \file FkParamDialog.cpp
 *  \brief Defines class FkParamDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "FkParamDialog.h"
#include "motif++/MotifClasses.h"

using namespace libgfk;

FkParamDialog::FkParamDialog(const char *name, Component *parent, FKType type)
			: ParamDialog(name,parent)
{
    createInterface(type);
    enableCallbackType(XmNactivateCallback);
}

void FkParamDialog::createInterface(FKType type)
{
    int n;
    RowColumn *controls, *rc2, *rc3;
    Separator *sep2;
    Label *plot_label, *signal_label;
    Arg args[15];

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    compute_button = new Button("Compute", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    plot_label = new Label("FK Plot Parameters", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, plot_label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    rc = new RowColumn("rc", this, args, n);

    s_max_label = new Label("slowness max (s/deg)", rc);
    XtSetArg(args[0], XmNcolumns, 10);
    s_max_text = new TextField("s_max_text", rc, this, args, 1);
    string s;
    if( getProperty("fk_smax", s) ) {
	s_max_text->setString(s);
    }

    num_s_label = new Label("num slowness", rc);
    num_s_text = new TextField("num_s_text", rc, this, args, 1);
    if( getProperty("fk_nums", s) ) {
	num_s_text->setString(s);
    }

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 20); n++;
    signal_label = new Label("FK Signal Parameters", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, signal_label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    if( type != FK_SINGLE_BAND ) {
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
	XtSetArg(args[n], XmNbottomOffset, 4); n++;
    }
    rc2 = new RowColumn("rc2", this, args, n);

    slow_min_label = new Label("slowness min (s/deg)", rc2);
    az_min_label = new Label("azimuth min (deg)", rc2);
    window_length_label = new Label("window length (sec)", rc2);
    stav_length_label = new Label("stav length (sec)", rc2);
    XtSetArg(args[0], XmNcolumns, 10);
    slow_min_text = new TextField("slow_min_text", rc2, this, args, 1);
    az_min_text = new TextField("az_min_text", rc2, this, args, 1);
    window_length_text = new TextField("window_length_text", rc2, this, args,1);
    stav_length_text = new TextField("stav_length_text", rc2, this, args,1);

    slow_max_label = new Label("slowness max (s/deg)", rc2);
    az_max_label = new Label("azimuth max (deg)", rc2);
    window_overlap_label = new Label("window overlap (sec)", rc2);
    ltav_length_label = new Label("ltav length (sec)", rc2);
    slow_max_text = new TextField("slow_max_text", rc2, this, args, 1);
    az_max_text = new TextField("az_max_text", rc2, this, args, 1);
    window_overlap_text = new TextField("window_overlap_text", rc2,this,args,1);
    ltav_length_text = new TextField("ltav_length_text", rc2, this, args,1);

    if( type == FK_SINGLE_BAND )
    {
	n = 0;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, rc2->baseWidget()); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 4); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
	XtSetArg(args[n], XmNbottomOffset, 4); n++;
	rc3 = new RowColumn("rc2", this, args, n);

	scan_frequencies_toggle = new Toggle("Scan Frequencies", rc3);

	bandwidth_label = new Label("Bandwidth(Hz)", rc3);
	bandwidth_label->setSensitive(false);
	n = 0;
	XtSetArg(args[n], XmNcolumns, 6); n++;
	bandwidth_text = new TextField("bandwidth_text", rc3, args, n);
	bandwidth_text->setSensitive(false);
	bandwidth_text->setString(".5");
    }
    else {
	scan_frequencies_toggle = NULL;
	bandwidth_label = NULL;
	bandwidth_text = NULL;
    }
}

FkParamDialog::~FkParamDialog(void)
{
}

void FkParamDialog::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute")) {
	compute();
    }
    else if(!strcmp(cmd, "Scan Frequencies")) {
	bandwidth_text->setSensitive(scan_frequencies_toggle->state());
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("FK Parameters");
    }
}

void FkParamDialog::compute(void)
{
    FkParamDialogStruct data;
    
    data.s_max_text = s_max_text->getString();
    data.num_s_text = num_s_text->getString();
    data.slow_min_text = slow_min_text->getString();
    data.slow_max_text = slow_max_text->getString();
    data.az_min_text = az_min_text->getString();
    data.az_max_text = az_max_text->getString();

    doCallbacks(baseWidget(), (XtPointer)&data, XmNactivateCallback);

    Free(data.s_max_text);
    Free(data.num_s_text);
    Free(data.slow_min_text);
    Free(data.slow_max_text);
    Free(data.az_min_text);
    Free(data.az_max_text);
}

ParseCmd FkParamDialog::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "slowness_max", c)) {
	s_max_text->setString(c, true);
    }
    else if(parseArg(cmd, "num_slowness", c)) {
	num_s_text->setString(c, true);
    }
    else if(parseArg(cmd, "signal_slowness_min", c)) {
	slow_min_text->setString(c, true);
    }
    else if(parseArg(cmd, "signal_slowness_max", c)) {
	slow_max_text->setString(c, true);
    }
    else if(parseArg(cmd, "azimuth_min", c)) {
	az_min_text->setString(c, true);
    }
    else if(parseArg(cmd, "azimuth_max", c)) {
	az_max_text->setString(c, true);
    }
    else if(parseArg(cmd, "window_length", c)) {
	window_length_text->setString(c, true);
    }
    else if(parseArg(cmd, "window_overlap", c) ||
		parseArg(cmd, "overlap", c))
    {
	window_overlap_text->setString(c, true);
    }
    else if(parseArg(cmd, "stav_length", c))
    {
	stav_length_text->setString(c, true);
    }
    else if(parseArg(cmd, "ltav_length", c))
    {
	ltav_length_text->setString(c, true);
    }
    else if(parseCompare(cmd, "Compute")) {
	compute();
    }
    else {
	return ParamDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void FkParamDialog::parseHelp(const char *prefix)
{
    printf("%sslowness_max=NUM\n", prefix);
    printf("%snum_slowness=NUM\n", prefix);
    printf("%ssignal_slowness_min=NUM\n", prefix);
    printf("%ssignal_slowness_max=NUM\n", prefix);
    printf("%ssignal_azimuth_min=NUM\n", prefix);
    printf("%ssignal_azimuth_max=NUM\n", prefix);
    printf("%swindow_length=NUM\n", prefix);
    printf("%swindow_overlap=NUM\n", prefix);
}
