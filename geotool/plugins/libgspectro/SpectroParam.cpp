/** \file SpectroParam.cpp
 *  \brief Defines class SpectroParam.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "SpectroParam.h"
#include "motif++/MotifClasses.h"

using namespace libgspectro;


SpectroParam::SpectroParam(const char *name, Component *parent) :
			ParamDialog(name, parent)
{
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

void SpectroParam::createInterface(void)
{
    int n;
    RowColumn *controls, *rc;
    Separator *sep;
    Arg args[15];

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XmString xm = createXmString("Auto Window Parameters");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    auto_toggle = new Toggle("Auto Window Parameters", this, args, n);

    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    Button *b = new Button("Close", controls, this);
    compute_button = new Button("Compute", controls, this);
    b = new Button("Help", controls, this);
/*
    XtSetArg(args[0], XmNmenuHelpWidget, b->baseWidget());
    controls->setValues(args, 1);
*/
    controls->setHelp(b);

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
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, auto_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    rc = new RowColumn("rc", this, args, n);

    new Label("lo freq", rc);
    new Label("window length", rc);

    XtSetArg(args[0], XmNcolumns, 10);
    lo_freq_text = new TextField("lo_freq_text", rc, this, args, 1);
    window_length_text = new TextField("window_length_text", rc, this, args, 1);

    new Label("hi freq", rc);
    new Label("window overlap", rc);

    hi_freq_text = new TextField("hi_freq_text", rc, this, args, 1);
    window_overlap_text = new TextField("window_overlap_text", rc,this,args,1);

    ignore_set_text = False;
}

SpectroParam::~SpectroParam(void)
{
}

void SpectroParam::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "hi_freq_text") || !strcmp(cmd, "lo_freq_text") ||
	    !strcmp(cmd, "window_length_text") ||
	    !strcmp(cmd, "window_overlap_text"))
    {
	if(!ignore_set_text) {
	    auto_toggle->set(false, false);
	}
    }
    else if(!strcmp(cmd, "Compute")) {
	compute();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Spectrogram Parameters Help");
    }
}

void SpectroParam::compute(void)
{
    SpectroParamStruct data;
    data.lo_freq_text = lo_freq_text->getString();
    data.hi_freq_text = hi_freq_text->getString();
    data.window_length_text = window_length_text->getString();
    data.window_overlap_text =window_overlap_text->getString();
    doCallbacks(baseWidget(), (XtPointer)&data, XmNactivateCallback);
    Free(data.lo_freq_text);
    Free(data.hi_freq_text);
    Free(data.window_length_text);
    Free(data.window_overlap_text);
}

ParseCmd SpectroParam::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "Auto_Window_Parameters", c) || parseArg(cmd, "Auto", c))
    {
	return auto_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "lo_freq", c)) {
	lo_freq_text->setString(c, true);
    }
    else if(parseArg(cmd, "hi_freq", c)) {
	hi_freq_text->setString(c, true);
    }
    else if(parseArg(cmd, "window_length", c) || parseArg(cmd, "length", c))
    {
	window_length_text->setString(c, true);
    }
    else if(parseArg(cmd, "window_overlap", c) || parseArg(cmd, "overlap", c))
    {
	window_overlap_text->setString(c, true);
    }
    else if(parseCompare(cmd, "Compute")) {
	compute();
    }
    else {
	return ParamDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}
