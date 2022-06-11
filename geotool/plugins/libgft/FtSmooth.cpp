/** \file FtSmooth.cpp
 *  \brief Defines class FtSmooth.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "FtSmooth.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libgmath.h"
}

using namespace libgft;

FtSmooth::FtSmooth(const char *name, Component *parent) :
			ParamDialog(name, parent)
{
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

void FtSmooth::createInterface(void)
{
    int n;
    Arg args[15];

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    label = new Label("Width (Hz)", rc);

    XtSetArg(args[0], XmNcolumns, 8);
    width_text = new TextField("width_text", rc, this, args, 1);
    width_text->setString("0");
    width_text->addActionListener(this);
    width = 0.;
}

FtSmooth::~FtSmooth(void)
{
}

void FtSmooth::setWidth(double smoothing_width)
{
    char buf[20];
    width = smoothing_width;
    ftoa(width, 2, 0, buf, sizeof(buf));
    width_text->setString(buf);
}

void FtSmooth::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Widget w = comp->baseWidget();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "width_text")) {
	double d;
	if(width_text->getDouble(&d) && d != width) {
	    apply_button->setSensitive(true);
	}
	else {
	    apply_button->setSensitive(false);
	}
    }
    else if(!strcmp(cmd, "Apply")) {
	if(width_text->getDouble(&width)) {
	    doCallbacks(w, NULL, XmNactivateCallback);
	    apply_button->setSensitive(false);
	}
    }
}

ParseCmd FtSmooth::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "width", c)) {
	width_text->setString(c, true);
    }
    else if(parseCompare(cmd, "Apply")) {
	if(width_text->getDouble(&width)) {
	    doCallbacks(baseWidget(), (XtPointer)&width, XmNactivateCallback);
	    apply_button->setSensitive(false);
	}
    }
    else {
	return ParamDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}
