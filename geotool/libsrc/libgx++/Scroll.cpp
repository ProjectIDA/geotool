/** \file Scroll.cpp
 *  \brief Defines class Scroll.
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "Scroll.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}

Scroll::Scroll(const string &name, Component *parent, WaveformPlot *wave_plot)
                        : FormDialog(name, parent, false, false)
{
    wp = wave_plot;
    createInterface();
}

void Scroll::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNvalue, 500); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    speed_scale = new Scale("speed", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, speed_scale->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XtNorientation, XmHORIZONTAL); n++;
    rb = new RadioBox("rb", this, args, n);

    fb_toggle = new Toggle("fb", rb, this);
    b_toggle = new Toggle("b", rb, this);
    s_toggle = new Toggle("s", rb, this);
    s_toggle->set(true);
    f_toggle = new Toggle("f", rb, this);
    ff_toggle = new Toggle("ff", rb, this);

    action = 0;
    interval = 500;
    memset(direction, 0, sizeof(direction));
}

Scroll::~Scroll(void)
{
}

void Scroll::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Toggle *t = comp->getToggleInstance();

    int previous_action = action;

    if(!strcmp(cmd, "Close"))
    {
	setVisible(false);
    }
    else if(!strcmp(cmd, "fb")) {
	if( t && t->state() ) {
	    strcpy(direction, "UP");
	    action = 1;
	}
    }
    else if(!strcmp(cmd, "b")) {
	if( t && t->state() ) {
	    strcpy(direction, "up");
	    action = 1;
	}
    }
    else if(!strcmp(cmd, "s")) {
	action = 0;
    }
    else if(!strcmp(cmd, "f")) {
	if( t && t->state() ) {
	    strcpy(direction, "down");
	    action = 1;
	}
    }
    else if(!strcmp(cmd, "ff")) {
	if( t && t->state() ) {
	    strcpy(direction, "DOWN");
	    action = 1;
	}
    }
    else if(comp == speed_scale) {
	interval = (unsigned long)speed_scale->getValue();
    }

    if(action > previous_action)
    {
	XtAppAddTimeOut(Application::getAppContext(), interval,
		(XtTimerCallbackProc)doScroll, (XtPointer)this);
    }
}

void Scroll::setVisible(bool visible)
{
    FormDialog::setVisible(visible);
    if(!visible) {
	s_toggle->set(true, true);
    }
}

void Scroll::doScroll(XtPointer client_data, XtIntervalId id)
{
    int end;
    Cardinal num_params = 2;

    Scroll *s = (Scroll *)client_data;

    if(s->action == 0) return;

    const char *params[2];
    params[0] = strdup("horizontal");
    params[1] = strdup(s->direction);

    end = s->wp->pageAxis(NULL, params, &num_params);

    Free(params[0]);
    Free(params[1]);

    if(end) {
	s->s_toggle->set(true, true);
    }
    else
    {
	XtAppAddTimeOut(Application::getAppContext(), s->interval,
		(XtTimerCallbackProc)doScroll, client_data);
    }
}
