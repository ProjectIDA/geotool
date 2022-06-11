/** \file Example2.cpp
 *  \brief Defines class Example2.
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "Example2.h"		  // For this class definition
#include "motif++/MotifClasses.h" // For MenuBar, Menu, Button, etc.
#include "gobject++/DataSource.h" // For the DataSource class
#include "WaveformView.h"	  // For the WaveformView class

// Plug-in window constructor
PLUGIN_NAME::PLUGIN_NAME(const char *name, Component *parent, DataSource *ds)
			: Frame(name, parent), DataReceiver(ds)
{
    createInterface();
}

void PLUGIN_NAME::createInterface()
{
    int n;
    Arg args[20];

    // Set the initial width and height of the window.
    setSize(600, 400);

    // create a menu bar, tool bar, and info area (at the bottom)
    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // create a File menu with two buttons
    file_menu = new Menu("File", menu_bar);
    output_button = new Button("Output...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // create an Edit menu with two buttons
    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);
    delete_button = new Button("Delete Selected", edit_menu, this);

    // create an Option menu with one button
    option_menu = new Menu("Option", menu_bar);
    compute_button = new Button("Compute", option_menu, this);

    // create a Help menu with one button
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button(PLUGIN_STRING, help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtitle, "Processed Waveforms"); n++;

    // create a WaveformView object.
    wplot = new WaveformView("wplot", frame_form, info_area, args, n);

    if(!tool_bar->loadDefaults()) {
	// Add Close and Compute to the tool bar
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
    }
}

/* Plug-in window destructor
 */
PLUGIN_NAME::~PLUGIN_NAME(void)
{
}

/* Handle ActionEvents (From Button, Toggle, etc.)
 */
void PLUGIN_NAME::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute")) {
	compute();
    }
    else if(!strcmp(cmd, "Clear")) {
	wplot->clear();
    }
    else if(!strcmp(cmd, "Output...")) {
	wplot->output();
    }
    else if(!strcmp(cmd, "Delete Selected")) {
	wplot->deleteSelectedWaveforms();
    }
    else if(comp == help_button) {
	displayHelp("Select one or more waveforms in the waveform window.\n\
Click Compute to process the selected waveforms.");
    }
}

/* Handle script string commands
 */
ParseCmd PLUGIN_NAME::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;

    if(parseCompare(cmd, "clear")) {
	clear_button->activate();
    }
    else if(parseCompare(cmd, "compute")) {
	compute_button->activate();
    }
    else if(parseCompare(cmd, "delete_selected")) {
	delete_button->activate();
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = wplot->parseCmd(cmd, msg)) == COMMAND_NOT_FOUND)
    {
	return FormDialog::parseCmd(cmd, msg);
    }
    return ret;
}

/* Handle script variable references
 */
ParseVar PLUGIN_NAME::parseVar(const string &name, string &value)
{
    ParseVar ret;

    if((ret = wplot->parseVar(name, value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    return Frame::parseVar(name, value);
}

/* Print string commands.
 */
void PLUGIN_NAME::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scompute\n", prefix);
    printf("%sdelete_selected\n", prefix);
    wplot->parseHelp(prefix);
}

/* This example method simply copies the waveforms and multiplies the data
 * values by -1, thus flipping the polarity. The waveforms are then displayed
 * in the plug-in window.
 */
void PLUGIN_NAME::compute(void)
{
    gvector<GTimeSeries *> tvec;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }
    wplot->clear();

    if(data_source->copySelectedTimeSeries(tvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }
    wplot->copyAllTables(data_source);

    // Multiply the waveform data values by -1.
    for(int i = 0; i < tvec.size(); i++)
    {
	// Loop over the segments of the waveform.
	for(int j = 0; j < tvec[i]->size(); j++) {
	    int npts = tvec[i]->segment(j)->length();
	    float *data = tvec[i]->segment(j)->data;
	    // Loop over all data values in each segment.
	    for(int k = 0; k < npts; k++) {
		data[k] *= -1.;
	    }
	}
	wplot->addWaveform(tvec[i], stringToPixel("grey60"));
    }
}
