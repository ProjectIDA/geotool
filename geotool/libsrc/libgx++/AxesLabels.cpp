/** \file AxesLabels.cpp
 *  \brief Defines class AxesLabels.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "motif++/MotifClasses.h"
#include "widget/AxesClass.h"
#include "AxesLabels.h"

extern "C" {
#include "libstring.h"
}

AxesLabels::AxesLabels(const string &name, Component *parent,
	AxesClass *axes_class) : FormDialog(name, parent, false, false)
{
    axes = axes_class;
    createInterface();
    setTextFields();
}

void AxesLabels::createInterface(void)
{
    Arg args[10];
    int n;

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    help_button = new Button("Help", controls, this);
    help_button->setSensitive(false);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    rc = new RowColumn("rc", this, args, n);

    title_label = new Label("Title", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 20); n++;
    title_text = new TextField("title_text", rc, this, args, n);

    x_axis_label = new Label("x-axis label", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 20); n++;
    x_axis_text = new TextField("x_axis_text", rc, this, args, n);

    y_axis_label = new Label("y-axis label", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 20); n++;
    y_axis_text = new TextField("y_axis_text", rc, this, args, n);
}

AxesLabels::~AxesLabels(void)
{
}

void AxesLabels::setTextFields(void)
{
    Arg args[1];
    char *s;

    if(!axes) return;

    XtSetArg(args[0], XtNtitle, &s);
    axes->getValues(args, 1);
    last_title_string.assign(s);
    title_text->setString(last_title_string);

    XtSetArg(args[0], XtNxLabel, &s);
    axes->getValues(args, 1);
    last_x_axis_string.assign(s);
    x_axis_text->setString(last_x_axis_string);

    XtSetArg(args[0], XtNyLabel, &s);
    axes->getValues(args, 1);
    last_y_axis_string.assign(s);
    y_axis_text->setString(last_y_axis_string);
}

void AxesLabels::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    if(!strcmp(cmd, "title_text") || !strcmp(cmd, "x_axis_text")
		|| !strcmp(cmd, "y_axis_text"))
    {
	if(!title_text->equals(last_title_string) ||
	   !x_axis_text->equals(last_x_axis_string) ||
	   !y_axis_text->equals(last_y_axis_string))
	{
	    apply_button->setSensitive(true);
	}
	else {
	    apply_button->setSensitive(false);
	}
    }
    else if(!strcmp(cmd, "Apply"))
    {
	apply();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Axes Labels");
    }
}

void AxesLabels::apply(void)
{
    title_text->getString(last_title_string);
    x_axis_text->getString(last_x_axis_string);
    y_axis_text->getString(last_y_axis_string);

    Arg args[3];
    XtSetArg(args[0], XtNtitle, last_title_string.c_str());
    XtSetArg(args[1], XtNxLabel, last_x_axis_string.c_str());
    XtSetArg(args[2], XtNyLabel, last_y_axis_string.c_str());
    axes->setValues(args, 3);
    apply_button->setSensitive(false);
}

ParseCmd AxesLabels::parseCmd(const string &cmd, string &msg)
{
    bool err;
    string c;

    if(parseArg(cmd, "Title", c)) {
	title_text->setString(c);
	apply_button->setSensitive(true);
    }
    else if(parseArg(cmd, "x_axis_label", c)) {
	x_axis_text->setString(c);
	apply_button->setSensitive(true);
    }
    else if(parseArg(cmd, "y_axis_label", c)) {
	y_axis_text->setString(c);
	apply_button->setSensitive(true);
    }
    else if(parseCompare(cmd, "Apply")) {
	apply();
    }
    else if(parseFind(cmd, "set_axes_labels", msg, &err,
		"x_label", false, "y_label", false, "title", false))
    {
	bool found_one=false;
	if(parseGetArg(cmd, "x_label", c)) {
	    x_axis_text->setString(c);
	    found_one = true;
	}
	if(parseGetArg(cmd, "y_label", c)) {
	    y_axis_text->setString(c);
	    found_one = true;
	}
	if(parseGetArg(cmd, "title", c)) {
	    title_text->setString(c);
	    found_one = true;
	}
	if(!found_one) {
	    msg.assign("axes_labels set: missing argument");
	    return ARGUMENT_ERROR;
	}
	apply();
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

void AxesLabels::parseHelp(const char *prefix)
{
    printf("%stitle=TITLE\n", prefix);
    printf("%sx_axis_label=LABEL\n", prefix);
    printf("%sy_axis_label=LABEL\n", prefix);
    printf("%sapply\n", prefix);
}
