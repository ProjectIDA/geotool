/** \file FtWindows.cpp
 *  \brief Defines class FtWindows.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "FtWindows.h"
#include "motif++/MotifClasses.h"

using namespace libgft;

FtWindows::FtWindows(const char *name, Component *parent) :
			ParamDialog(name, parent)
{
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

void FtWindows::createInterface(void)
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

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
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
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc1 = new RowColumn("rc1", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNvalue, "1"); n++;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    num_windows_text = new TextField("num_windows_text", rc1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 99); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    XmString xm = createXmString("Number of Windows");
    XtSetArg(args[n], XmNtitleString, xm); n++;
    num_windows_scale = new Scale("num_windows_scale", rc1, this, args, n);
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc2 = new RowColumn("rc2", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNvalue, "0"); n++;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    overlap_text = new TextField("overlap_text", rc1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 99); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    xm = createXmString("Overlap (%)");
    XtSetArg(args[n], XmNtitleString, xm); n++;
    overlap_scale = new Scale("overlap_scale", rc1, this, args, n);
    XmStringFree(xm);
}

FtWindows::~FtWindows(void)
{
}

void FtWindows::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Widget w = comp->baseWidget();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	FtWindowsStruct s;
	s.num_windows = num_windows_scale->getValue();
	s.overlap = overlap_scale->getValue();
	doCallbacks(w, (XtPointer)&s, XmNactivateCallback);
    }
    else if(comp == num_windows_scale) {
	XmScaleCallbackStruct *s = (XmScaleCallbackStruct *)action_event->getCalldata();
	num_windows_text->setString(s->value);
    }
    else if(comp == overlap_scale) {
	XmScaleCallbackStruct *s = (XmScaleCallbackStruct *)action_event->getCalldata();
	overlap_text->setString(s->value);
    }
    else if(comp == num_windows_text) {
	int num;
	if(num_windows_text->getInt(&num)) {
	    if(num < 1) {
		num = 1;
		num_windows_text->setString("1");
	    }
	    else if(num > 99) {
		num = 99;
		num_windows_text->setString("99");
	    }
	    num_windows_scale->setValue(num);
	}
    }
    else if(comp == overlap_text) {
	int num;
	if(overlap_text->getInt(&num)) {
	    if(num < 0) {
		num = 0;
		overlap_text->setString("0");
	    }
	    else if(num > 99) {
		num = 99;
		overlap_text->setString("99");
	    }
	    overlap_scale->setValue(num);
	}
    }
}

ParseCmd FtWindows::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "Number", c) )
    {
	num_windows_text->setString(c, true);
    }
    else if(parseArg(cmd, "Overlap", c))
    {
	overlap_text->setString(c, true);
    }
    else if(parseCompare(cmd, "Apply")) {
	FtWindowsStruct s;
	s.num_windows = num_windows_scale->getValue();
	s.overlap = overlap_scale->getValue();
	doCallbacks(baseWidget(), (XtPointer)&s, XmNactivateCallback);
    }
    else {
	return ParamDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}
