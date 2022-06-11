/** \file MinCorrOverlap.cpp
 *  \brief Defines class MinCorrOverlap.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "MinCorrOverlap.h"
#include "motif++/MotifClasses.h"

using namespace libgcor;

MinCorrOverlap::MinCorrOverlap(const char *name, Component *parent) :
		FormDialog(name, parent, false, false)
{
    createInterface();
}

void MinCorrOverlap::createInterface(void)
{
    Arg args[20];
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
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    label = new Label("Minimum Correlation Overlap (Secs)", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 20); n++;
    text = new TextField("text", rc, this, args, n);

    min_overlap = getProperty("min_correlation_overlap", 5.);
    text->setString("%.2f", min_overlap);

    enableCallbackType(XmNactivateCallback);
}

MinCorrOverlap::~MinCorrOverlap(void)
{
}

void MinCorrOverlap::setVisible(bool visible)
{
    if(visible) {
	text->setString("%.2f", min_overlap);
    }
    FormDialog::setVisible(visible);
}

void MinCorrOverlap::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    if( !strcmp(cmd, "text") )
    {
	double d;
	if(text->getDouble(&d) && d != min_overlap) {
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
	showHelp("Minimum Correlation Overlap");
    }
}

void MinCorrOverlap::apply(void)
{
    double d;
    if(text->getDouble(&d) && d != min_overlap) {
	min_overlap = d;
	doCallbacks(base_widget, (XtPointer)&min_overlap, XmNactivateCallback);
    }
    apply_button->setSensitive(false);
}

ParseCmd MinCorrOverlap::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "Minimun_Correlation_Overlap", c)) {
	text->setString(c);
    }
    else if(parseCompare(cmd, "Apply")) {
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

void MinCorrOverlap::parseHelp(const char *prefix)
{
    printf("%sminimum_correlation_overlap=SECS\n", prefix);
    printf("%sapply\n", prefix);
}
