#include "config.h"
#include <iostream>
using namespace std;

#include "Example3.h"
#include "hilbt.h"
#include "motif++/MotifClasses.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/DataSource.h"


PLUGIN_NAME::PLUGIN_NAME(const char *name, Component *parent,
			DataSource *ds) : FormDialog(name, parent, false),
			DataReceiver(ds)
{
    createInterface();
}

void PLUGIN_NAME::createInterface(void)
{
    int n;
    Arg args[20];

     n = 0;
     XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNleftOffset, 5); n++;
     XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNbottomOffset, 5); n++;
     XtSetArg(args[n], XtNorientation, XmHORIZONTAL); n++;
     controls = new RowColumn("controls", this, args, n);
 
     close_button = new Button("Close", controls, this);
     apply_button = new Button("Apply", controls, this);
     unfilter_button = new Button("Unfilter", controls, this);
     help_button = new Button("Help", controls, this);
     controls->setHelp(help_button);
 
     n = 0;
     XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
     XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
     sep = new Separator("sep", this, args, n);
 
     n = 0;
     XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNtopOffset, 5); n++;
     XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNleftOffset, 5); n++;
     XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNrightOffset, 5); n++;
     XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
     XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
     XtSetArg(args[n], XtNorientation, XmHORIZONTAL); n++;
     XtSetArg(args[n], XmNspacing, 20); n++;
     rc = new RowColumn("rc", this, args, n);
 
     label = new Label("Input", rc);
     n = 0;
     XtSetArg(args[n], XmNborderWidth, 1); n++;
     XtSetArg(args[n], XtNorientation, XmHORIZONTAL); n++;
     input_rb = new RadioBox("input_rb", rc, args, n);
 
     n = 0;
     XtSetArg(args[n], XmNshadowThickness, 0); n++;
     XtSetArg(args[n], XmNset, False); n++;
     all_toggle = new Toggle("All", input_rb, this, args, n);
 
     n = 0;
     XtSetArg(args[n], XmNshadowThickness, 0); n++;
     XtSetArg(args[n], XmNset, True); n++;
     selected_toggle = new Toggle("Selected", input_rb, this, args, n);
}

PLUGIN_NAME::~PLUGIN_NAME(void)
{
}

void PLUGIN_NAME::actionPerformed(ActionEvent *action_event)
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
    else if(!strcmp(cmd, "Help")) {
	showHelp("Hilbert Transform Help");
    }
}

/* Handle script string commands
 */
ParseCmd PLUGIN_NAME::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "apply")) {
	apply_button->activate();
    }
    else if(parseCompare(cmd, "unfilter")) {
	unfilter_button->activate();
    }
    else if(parseString(cmd, "input", c)) {
	if(parseCompare(c, "All")) {
	    all_toggle->set(true, true);
	}
	else if(parseCompare(c, "selected")) {
	    selected_toggle->set(true, true);
	}
	else return ARGUMENT_ERROR;
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

/* Handle script variable references
 */
ParseVar PLUGIN_NAME::parseVar(const string &name, string &value)
{
    return FormDialog::parseVar(name, value);
}

void PLUGIN_NAME::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%sunfilter\n", prefix);
    printf("%sinput=(all,selected)\n", prefix);
}

void PLUGIN_NAME::apply(void)
{
    gvector<Waveform *> wvec;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }

    if(all_toggle->state()) {
	data_source->getWaveforms(wvec);
    }
    else if(data_source->getSelectedWaveforms(wvec) <= 0)
    {
	showWarning("No waveforms selected.");
	return;
    }
    setCursor("hourglass");

    for(int i = 0; i < wvec.size(); i++) {
	DataMethod *dm = new hilbert();
	wvec[i]->changeMethod(dm);
    }
    data_source->modifyWaveforms(wvec);

    setCursor("default");
}

void PLUGIN_NAME::unfilter(void)
{
    gvector<Waveform *> wvec, ws;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }
    if(all_toggle->state()) {
	data_source->getWaveforms(wvec);
    }
    else {
	data_source->getSelectedWaveforms(wvec);
    }

    setCursor("hourglass");

    for(int i = 0; i < wvec.size(); i++) {
	if(wvec[i]->ts->removeMethod("Hilbert")) {
	    ws.push_back(wvec[i]);
	}
    }

    data_source->modifyWaveforms(ws);

    setCursor("default");
}
