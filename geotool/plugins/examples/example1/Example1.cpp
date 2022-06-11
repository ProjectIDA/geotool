/** \file Example1.cpp
 *  \brief Defines class Example1.
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "Example1.h"		  // For this class definition
#include "motif++/MotifClasses.h" // For MenuBar, Menu, Button, etc.
#include "widget/Table.h"	  // For the Table class
#include "Waveform.h"	  	  // For the Waveform
#include "gobject++/DataSource.h" // For the DataSource class

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
    setSize(600, 300);

    // create a menu bar, tool bar, and info area (at the bottom)
    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // create a File menu with two buttons
    file_menu = new Menu("File", menu_bar);
    output_button = new Button("Output Table...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // create an Edit menu with two buttons
    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);
    remove_row_button = new Button("Remove Row", edit_menu, this);

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
    XtSetArg(args[n], XtNtableTitle, "Waveform Info"); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNcolumns, 6); n++;
    const char *col_labels[] =
	{"sta", "chan", "time", "endtime", "nsamp", "segments"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;

    // create a Table with 6 columns
    table = new Table("table", frame_form, info_area, args, n);

    int alignment[6] = {LEFT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY,
			RIGHT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY};
    table->setAlignment(6, alignment);

    file_dialog = NULL;

    if(!tool_bar->loadDefaults()) {
	// Add Close and Compute to the tool bar
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
    }
}

// Plug-in window destructor
PLUGIN_NAME::~PLUGIN_NAME(void)
{
}

/* Handle button events
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
	clear();
    }
    else if(!strcmp(cmd, "Output Table...")) {
	save();
    }
    else if(!strcmp(cmd, "Remove Row")) {
	removeRow();
    }
    else if(comp == help_button) {
	displayHelp("Select one or more waveforms in the waveform window.\n\
Click Compute to update the table");
    }
}

/* Handle script string commands
 */
ParseCmd PLUGIN_NAME::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    if(parseCompare(cmd, "clear")) {
	clear_button->activate();
    }
    else if(parseCompare(cmd, "compute")) {
	compute_button->activate();
    }
    else if(parseCompare(cmd, "remove_row")) {
	remove_row_button->activate();
    }
    else if(parseString(cmd, "output_table", c)) {
	table->save(c);
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
    ParseVar ret;

    if((ret = table->parseVar(name, value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    return Frame::parseVar(name, value);
}

void PLUGIN_NAME::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scompute\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%soutput_table FILENAME\n", prefix);
    Table::parseHelp(prefix);
}

/* Fill the table.
 */ 
void PLUGIN_NAME::compute(void)
{
    const char *row[6];
    char beg_time[30], end_time[30], samples[20], nseg[20];
    gvector<Waveform *> wvec;

    if( !data_source ) {
	showWarning("No DataSource.");
	return;
    }
    table->removeAllRows();

    if(data_source->getSelectedWaveforms(wvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }

    for(int i = 0; i < wvec.size(); i++)
    {
	row[0] = wvec[i]->sta();
	row[1] = wvec[i]->chan();
	timeEpochToString(wvec[i]->tbeg(), beg_time, sizeof(beg_time),
				YMONDHMS);
	row[2] = beg_time;
	timeEpochToString(wvec[i]->tend(), end_time, sizeof(end_time),
				YMONDHMS);
	row[3] = end_time;
	snprintf(samples, sizeof(samples), "%d", wvec[i]->ts->length());
	row[4] = samples;
	snprintf(nseg, sizeof(nseg), "%d", wvec[i]->ts->size());
	row[5] = nseg;

	table->addRow(row, false);
    }

    table->adjustColumns();
}

/* Remove all table rows.
 */
void PLUGIN_NAME::clear(void)
{
    table->removeAllRows();
}

/* Remove selected table rows.
 */
void PLUGIN_NAME::removeRow(void)
{
    vector<int> rows;

    if(table->getSelectedRows(rows) <= 0) {
	showWarning("No rows selected.");
	return;
    }
    table->removeRows(rows);
}

/* Save the table contents to a file.
 */
void PLUGIN_NAME::save(void)
{
    char *file;

    if(file_dialog == NULL) {
	file_dialog = new FileDialog("Save Table", this, FILE_ONLY, ".",
				(char *)"*", "Save");
    }
    file_dialog->setVisible(true);

    if((file = file_dialog->getFile()) != NULL) {
	table->save(file);
	XtFree(file);
    }
    file_dialog->setVisible(false);
}
