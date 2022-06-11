/** \file CepstrumParams.cpp
 *  \brief Defines class CepstrumParam
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "CepstrumParams.h"
#include "motif++/MotifClasses.h"

using namespace libgcepstrum;

CepstrumParams::CepstrumParams(const char *name, Component *parent,
		ActionListener *listener) : ParamDialog(name, parent)
{
    createInterface();

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void CepstrumParams::createInterface()
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
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    rc = new RowColumn("rc", this, args, n);

    label[0] = new Label("smoothing npass", rc);
    label[1] = new Label("low frequency", rc);
    label[2] = new Label("guard1", rc);
    label[3] = new Label("tpass", rc);
    label[4] = new Label("pulse delay min", rc);
    label[5] = new Label("guard2", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    text[0] = new TextField("smoothing npass", rc, args, n);
    text[0]->setString("1");
    text[1] = new TextField("low frequency", rc, this, args, n);
    text[1]->setString("2.0");
    text[2] = new TextField("guard1", rc, args, n);
    text[2]->setString("0.1");
    text[3] = new TextField("tpass", rc, args, n);
    text[3]->setString("1.0");
    text[4] = new TextField("pulse delay min", rc, args, n);
    text[4]->setString("0.04");
    text[5] = new TextField("guard2", rc, args, n);
    text[5]->setString("0.1");

    label[6] = new Label("smoothing width", rc);
    label[7] = new Label("high frequency", rc);
    label[8] = new Label("average bandwidth1", rc);
    label[9] = new Label("detrend", rc);
    label[10] = new Label("pulse delay max", rc);
    label[11] = new Label("average bandwidth2", rc);

    text[6] = new TextField("smoothing width", rc, args, n);
    text[6]->setString("0.2");
    text[7] = new TextField("high frequency", rc, this, args, n);
    text[7]->setString("100.");
    text[8] = new TextField("average bandwidth1", rc, args, n);
    text[8]->setString("4.0");
    text[9] = new TextField("detrend", rc, args, n);
    text[9]->setString("0");
    text[10] = new TextField("pulse delay max", rc, args, n);
    text[10]->setString("3.");
    text[11] = new TextField("average bandwidth2", rc, args, n);
    text[11]->setString("0.1");
}

CepstrumParams::~CepstrumParams(void)
{
}

void CepstrumParams::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute")) {
	doCallbacks(base_widget, (XtPointer)NULL, XmNactivateCallback);
    }
    else if(comp == text[1]) { // low frequency
	doCallbacks(base_widget, (XtPointer)"low", XmNactivateCallback);
    }
    else if(comp == text[7]) { // high frequency
	doCallbacks(base_widget, (XtPointer)"high", XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Cepstrum Parameters");
    }
}
