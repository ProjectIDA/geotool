/** \file RespTapers.cpp
 *  \brief Defines class RespTapers.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include <math.h>
#include "RespTapers.h"
#include "motif++/MotifClasses.h"

using namespace libgrsp;

RespTapers::RespTapers(const char *name, Component *parent,
	double taper_secs, double low, double high, double cutoff) :
		FormDialog(name, parent, false)
{
    data_taper = taper_secs;
    low_pass = low;
    high_pass = high;
    amp_cutoff = cutoff;
    createInterface();
    addActionListener(parent, XmNactivateCallback);
}

void RespTapers::createInterface(void)
{
    int n;
    char s[50];
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    warn = new Label("", controls);
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
    label = new Label("Response Tapers", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    rc = new RowColumn("rc", this, args, n);

    label1 = new Label("Data Taper (secs)", rc);
    label2 = new Label("Low Pass (Hz)", rc);
    label3 = new Label("High Pass (Hz)", rc);
    label4 = new Label("Low Amp Cutoff (log)", rc);

    n = 0;
    XtSetArg(args[0], XmNcolumns, 10); n++;
    XtSetArg(args[1], XmNeditable, True); n++;

    params_ok = true;

    data_taper_text = new TextField("data_taper_text", rc, this, args, n);
    snprintf(s, sizeof(s), "%.2g", data_taper);
    data_taper_text->setString(s);

    low_pass_text = new TextField("low_pass_text", rc, this, args, n);
    snprintf(s, sizeof(s), "%.2g", low_pass);
    low_pass_text->setString(s);

    high_pass_text = new TextField("low_pass_text", rc, this, args, n);
    if(high_pass <= 0.) {
	high_pass_text->setString("");
    }
    else {
	snprintf(s, sizeof(s), "%.2g", high_pass);
	high_pass_text->setString(s);
    }

    amp_cutoff_text = new TextField("amp_cutoff_text", rc, this, args, n);
    snprintf(s, sizeof(s), "%.2g", amp_cutoff);
    amp_cutoff_text->setString(s);

    enableCallbackType(XmNactivateCallback);
}

RespTapers::~RespTapers(void)
{
}

void RespTapers::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(comp == data_taper_text || comp == low_pass_text
		|| comp == high_pass_text || comp == amp_cutoff_text)
    {
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
        doCallbacks(baseWidget(), (XtPointer)NULL, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Response Tapers Help");
    }
}

void RespTapers::setButtonsSensitive(void)
{
    bool change = false;
    double d;
    char *s;

    warn->setLabel("");
    params_ok = true;

    if(!data_taper_text->getDouble(&d) || d < 0.) {
	warn->setLabel("Invalid Data Taper");
	apply_button->setSensitive(false);
	params_ok = false;
	return;
    }
    if(d != data_taper) change = true;

    if(!low_pass_text->getDouble(&d) || d < 0.) {
	warn->setLabel("Invalid Low pass");
	apply_button->setSensitive(false);
	params_ok = false;
	return;
    }
    if(d != low_pass) change = true;

    s = high_pass_text->getString();
    if(s[0] == '\0') {
	d = 0.;
    }
    else if((!high_pass_text->getDouble(&d) || d < 0.)) 
    {
	warn->setLabel("Invalid High pass");
	apply_button->setSensitive(false);
	free(s);
	params_ok = false;
	return;
    }
    free(s);

    if(d != high_pass) change = true;

    if(!amp_cutoff_text->getDouble(&d) || d > 0.) {
	warn->setLabel("Invalid High pass");
	apply_button->setSensitive(false);
	params_ok = false;
	return;
    }
    if(d != amp_cutoff) change = true;

    if(change) {
	apply_button->setSensitive(true);
    }
}

void RespTapers::apply(void)
{
    char *s;
    double d;

    if(data_taper_text->getDouble(&d) && d >= 0.) {
	data_taper = d;
    }

    if(low_pass_text->getDouble(&d) && d >= 0.) {
	low_pass = d;
    }

    s = high_pass_text->getString();
    if(s[0] == '\0') {
	high_pass = 0.;
    }
    else if(high_pass_text->getDouble(&d) && d >= 0.) {
	high_pass = d;
    }
    free(s);

    if(amp_cutoff_text->getDouble(&d) && d < 0) {
	amp_cutoff = d;
    }
    apply_button->setSensitive(false);
}

void RespTapers::getParams(double *taper_secs, double *flo, double *fhi,
			double *cutoff)
{
    *taper_secs = data_taper;
    *flo = low_pass;
    *fhi = high_pass;
    *cutoff = pow(10., amp_cutoff);
}
