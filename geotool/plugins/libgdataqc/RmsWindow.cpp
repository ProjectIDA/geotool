/** \file RmsWindow.cpp
 *  \brief Defines class RmsWindow.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "RmsWindow.h"
#include "RmsData.h"
#include "Waveform.h"
#include "motif++/MotifClasses.h"

using namespace libgdataqc;


RmsWindow::RmsWindow(const char *name, Component *parent, DataSource *ds)
			: FormDialog(name, parent), DataReceiver(ds)
{
    createInterface();
}

void RmsWindow::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    label = new Label("window length (samples)", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    text = new TextField("window length", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, text->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    unfilter_button = new Button("Unfilter", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);
}

RmsWindow::~RmsWindow(void)
{
}

void RmsWindow::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
    }
    else if(!strcmp(cmd, "Unfilter")) {
	unfilter();
    }
    else if(!strcmp(cmd, "window length")) {
	int i;
	apply_button->setSensitive(text->getInt(&i));
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("RMS Help");
    }
}

ParseCmd RmsWindow::parseCmd(const string &cmd, string &msg)
{
    if(parseCompare(cmd, "Apply")) {
	apply();
    }
    else if(parseCompare(cmd, "Unfilter")) {
	unfilter();
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

void RmsWindow::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
}

void RmsWindow::apply(void)
{
    gvector<Waveform *> wvec;
    double start_time, end_time;
    int window_length, num_waveforms;
    bool windowed=true;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }
    if(!text->getInt(&window_length)) {
	showWarning("Invalid window length.");
	return;
    }
    num_waveforms = data_source->getSelectedWaveforms("a", wvec);
	
    if(num_waveforms <= 0)
    {
    	windowed = false;
        num_waveforms = data_source->getSelectedWaveforms(wvec);
    }

    if(num_waveforms == 0) {
	showWarning("No waveforms selected.");
	return;
    }

    for(int i = 0; i < num_waveforms; i++) {
	if(windowed) {
	    start_time = wvec[i]->dw[0].d1->time();
	    end_time = wvec[i]->dw[0].d2->time();
	}
	else {
	    start_time = wvec[i]->tbeg();
	    end_time = wvec[i]->tend();
	}
	RmsData *rms_data = new RmsData(window_length, start_time, end_time);
	gvector<Waveform *> v(wvec[i]);
	rms_data->apply(v);
    }
    data_source->modifyWaveforms(wvec);
}

void RmsWindow::unfilter(void)
{
    gvector<Waveform *> wvec, ws;
    const char *methods[1];

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }
    data_source->getSelectedWaveforms(wvec);
    if(wvec.size() == 0) {
	showWarning("No waveforms selected.");
	return;
    }

    methods[0] = "RmsData";

    for(int i = 0; i < wvec.size(); i++) {
        if(DataMethod::remove(1, methods, wvec[i])) {
	    ws.push_back(wvec[i]);
        }
    }

    data_source->modifyWaveforms(ws);
}
