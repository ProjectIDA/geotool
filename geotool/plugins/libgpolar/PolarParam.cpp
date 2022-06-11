/** \file PolarParam.cpp
 *  \brief Defines class PolarParam.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "PolarParam.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"

using namespace libgpolar;


PolarParam::PolarParam(const char *name, Component *parent,
		ActionListener *listener) : ParamDialog(name, parent)
{
    createInterface();

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void PolarParam::createInterface()
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    compute_button = new Button("Compute", controls, this);
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
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    auto_param_toggle = new Toggle("Auto Window Parameters", this, args, n);
    auto_param_toggle->set(true);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, auto_param_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;

    rc = new RowColumn("rc", this, args, n);

    window_length_label = new Label("window length (secs)", rc);

    XtSetArg(args[0], XmNcolumns, 10);
    window_length_text = new TextField("window_length", rc, this, args, 1);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    filter_rc = new RowColumn("filter_rc", this, args, n);

    filter_label = new Label("Filter", filter_rc);
    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb = new RadioBox("Filter", filter_rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    filter_on = new Toggle("On", rb, args, n);
    filter_on->set(false);
    filter_off = new Toggle("Off", rb, args, n);
    filter_off->set(true);

    const char *column_labels[] = {"Low", "High", "Order", "Type","Constraint"};
    const char *column_choice[] = {"","","0:1:2:3:4:5:6:7:8:9:10","BP:BR:LP:HP",
                                "zero phase:causal"};
    const char *cells[] = {
        "2.0", "4.0", "3", "BP", "causal", (char *)NULL};

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, filter_rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNcolumns, 5); n++;
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNcolumnChoice, column_choice); n++;
    XtSetArg(args[n], XtNcells, cells); n++;
    XtSetArg(args[n], XtNborderWidth, 0); n++;
    filter_table = new Table("table", this, args, n);
}

PolarParam::~PolarParam(void)
{
}

void PolarParam::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute")) {
	doCallbacks(base_widget, (XtPointer)0, XmNactivateCallback);
    }
    else if(comp == window_length_text) {
	double length;
	if(window_length_text->getDouble(&length) && length > 0.)
	{
	    auto_param_toggle->set(false);
	}
	else {
	    auto_param_toggle->set(true);
	}
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Polarization Parameters Help");
    }
}

bool PolarParam::getFilter(char *type, int *order, double *flo, double *fhi,
			int *zero_phase)
{
    vector<const char *> row;

    filter_table->getRow(0, row);

    if(!strcasecmp(row[3], "BP")) {
	strcpy(type, "BP");
    }
    else if(!strcasecmp(row[3], "LP")) {
	strcpy(type, "LP");
    }
    else if(!strcasecmp(row[3], "HP")) {
	strcpy(type, "HP");
    }
    else if(!strcasecmp(row[3], "BR")) {
	strcpy(type, "BR");
    }
    else {
	showWarning("Cannot interpret filter type: %s", row[3]);
	return false;
    }
    if(!stringToDouble(row[0], flo) || *flo < 0.) {
	showWarning("Cannot interpret Filter Low Frequency.");
	return false;
    }
    if(strcmp(type, "HP"))
    {
	if(!stringToDouble(row[1], fhi) || *fhi < 0.  || *fhi <= *flo)
	{
	    showWarning("Cannot interpret Filter High Frequency.");
	    return false;
	}
    }
    if(!stringToInt(row[2], order) || *order < 1 || *order > 10) {
	showWarning("Cannot interpret Filter Order");
	return false;
    }
    if(!strncasecmp(row[4], "zero", 4)) {
	*zero_phase = 1;
    }
    else if(!strncasecmp(row[4], "causal", 6)) {
	*zero_phase = 0;
    }
    else {
	showWarning("Cannot interpret Filter Constraint");
	return false;
    }
    return true;
}

ParseCmd PolarParam::parseCmd(const string &cmd, string &msg)
{
   ParseCmd ret = COMMAND_PARSED;
   bool zp;
   string c;

   if(parseArg(cmd, "filter", c)) {
	if(parseCompare(c, "on")) {
	    filter_on->set(true, true);
	}
	else if(parseCompare(c, "off")) {
	    filter_off->set(true, true);
	}
	else ret = ARGUMENT_ERROR;
    }
    else if(parseArg(cmd, "low", c)) {
	filter_table->setField(0, 0, c, true);
    }
    else if(parseArg(cmd, "high", c)) {
	filter_table->setField(0, 1, c, true);
    }
    else if(parseArg(cmd, "order", c)) {
	filter_table->setField(0, 2, c, true);
    }
    else if(parseArg(cmd, "type", c)) {
	filter_table->setField(0, 3, c, true);
    }
    else if(parseGetArg(cmd, "zp", msg, "zp", &zp)) {
	if(zp) filter_table->setField(0, 4, "zero phase", true);
	else filter_table->setField(0, 4, "causal", true);
    }
    else {
        return COMMAND_NOT_FOUND;
    }
    return ret;
}
