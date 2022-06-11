/** \file GClusterParams.cpp
 *  \brief Defines class GClusterParams
 *  \author Vera Miljanovic
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "GClusterParams.h"
#include "motif++/MotifClasses.h"

using namespace libgcluster;


GClusterParams::GClusterParams(const char *name, Component *parent,
		ActionListener *listener) : ParamDialog(name, parent)
{
    createInterface();

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void GClusterParams::createInterface()
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

    label[0] = new Label("number of clusters", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    text[0] = new TextField("number of clusters", rc, args, n);
    text[0]->setString("4");

    label[1] = new Label("minimum cross-correlation threshold", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    text[0] = new TextField("minimum cross-correlation threshold", rc, args, n);
    text[0]->setString("0.75");
}

GClusterParams::~GClusterParams(void)
{
}

void GClusterParams::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("GCluster Parameters");
    }
}
