/** \file HilbertTransform.cpp
 *  \brief Defines class HilbertTransform.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "HilbertTransform.h"
#include "Hilbert.h"
#include "TaperData.h"
#include "motif++/MotifClasses.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/DataSource.h"
#include "Working.h"
#include "DataMethod.h"
#include "TaperWindow.h"
#include "Demean.h"

using namespace libghp;


HilbertTransform::HilbertTransform(const char *name, Component *parent,
		DataSource *ds) : FormDialog(name, parent, false, false),
		DataReceiver(ds)
{
    createInterface();
}

void HilbertTransform::createInterface(void)
{
    Arg args[20];
    int n;

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
    taper_button = new Button("Taper Width...", controls, this);
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

    taper_window = new TaperWindow("Hilbert Transform Taper", this, 5, 5, 200);
}

HilbertTransform::~HilbertTransform(void)
{
}

void HilbertTransform::actionPerformed(ActionEvent *action_event)
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
    else if(!strcmp(cmd, "Taper Width...")) {
	taper_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Hilbert Transform Help");
    }
}

ParseCmd HilbertTransform::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "apply")) {
	apply_button->activate();
    }
    else if(parseCompare(cmd, "unfilter")) {
	unfilter_button->activate();
    }
    else if(parseArg(cmd, "Input", c)) {
	if(parseCompare(c, "All")) {
	    all_toggle->set(true, true);
	}
	else if(parseCompare(c, "Selected")) {
	    selected_toggle->set(true, true);
	}
	else return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "hilbert_transform_taper", c)
		|| parseString(cmd, "taper", c))
    {
	return taper_window->parseCmd(c, msg);
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

ParseVar HilbertTransform::parseVar(const string &name, string &value)
{
    string c;

    if(parseCompare(name, "input")) {
	if(all_toggle->state()) {
	    value.assign("All");
	}
	else {
	    value.assign("Selected");
	}
    }
    else if(parseString(name, "taper", c)) {
        return taper_window->parseVar(c, value);
    }
    else {
        return FormDialog::parseVar(name, value);
    }
    return STRING_RETURNED;
}

void HilbertTransform::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%sunfilter\n", prefix);
    printf("%sinput=(all,selected)\n", prefix);
}

void HilbertTransform::apply(void)
{
    int i, num_waveforms, npts;
    gvector<Waveform *> wvec;
    Working *working = NULL;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }

    if(all_toggle->state()) {
	num_waveforms = data_source->getWaveforms(wvec);
    }
    else if((num_waveforms = data_source->getSelectedWaveforms(wvec)) <= 0)
    {
	showWarning("No waveforms selected.");
	return;
    }
    setCursor("hourglass");

    npts = 0;
    for(i = 0; i < num_waveforms; i++) {
	npts += wvec[i]->length();
    }
    int working_threshold = getProperty("working_dialog_threshold", 100000);
    if(npts > working_threshold) {
	char title[100];
	snprintf(title, sizeof(title), "Processing %d waveforms",num_waveforms);
	working = new Working("Working", this, title, "waveforms processed");
	working->setVisible(true);
    }

    for(i = 0; i < num_waveforms; i++) {
	DataMethod *dm[3];
	dm[0] = new Demean();
	dm[1] = getTaper();
	dm[2] = new Hilbert();

	wvec[i]->changeMethods(3, dm);

	if(working && !working->update(i+1)) break;
    }
    if(working) {
	working->destroy();
    }
    data_source->modifyWaveforms(wvec);

    setCursor("default");
}

TaperData * HilbertTransform::getTaper(void)
{
    int width=0, minpts=0, maxpts=0;

    if(!taper_window->width->getInt(&width) || width < 0 || width > 100) {
	width = 5;
	taper_window->width->setString("5");
    }
    if(!taper_window->min_points->getInt(&minpts) || minpts < 0) {
	minpts = 5;
	taper_window->min_points->setString("5");
    }
    if(!taper_window->max_points->getInt(&maxpts) || maxpts <= minpts) {
	maxpts = 0;
	taper_window->max_points->setString("");
    }
    if(width == 0 && minpts == 0) {
	return new TaperData("none", width, minpts, maxpts);
    }
    else {
	return new TaperData("cosine", width, minpts, maxpts);
    }
}

void HilbertTransform::unfilter(void)
{
    int i, num_waveforms;
    const char *methods[3];
    gvector<Waveform *> wvec, ws;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }
    if(all_toggle->state()) {
	num_waveforms = data_source->getWaveforms(wvec);
    }
    else {
	num_waveforms = data_source->getSelectedWaveforms(wvec);
    }

    setCursor("hourglass");

    methods[0] = "Demean";
    methods[1] = "TaperData";
    methods[2] = "Hilbert";

    for(i = 0; i < num_waveforms; i++) {
	if(DataMethod::remove(3, methods, wvec[i])) {
	    ws.push_back(wvec[i]);
	}
    }

    data_source->modifyWaveforms(ws);

    setCursor("default");
}
