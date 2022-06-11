/** \file BeamGroups.cpp
 *  \brief Defines class BeamGroups
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
#include <math.h>

using namespace std;

#include "BeamGroups.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "BGStations.h"
#include "AddGroup.h"

extern "C" {
#include "libstring.h"
}

using namespace libgbm;

BeamGroups::BeamGroups(const char *name, Component *parent) : Frame(name,parent)
{
    createInterface();
    init();
}

void BeamGroups::createInterface()
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    add_button = new Button("Add Group...", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);

    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    stations_button = new Button("Stations...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    group_help_button = new Button("Beam Group Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    XtSetArg(args[n], XtNtableTitle, "Beam Groups"); n++;
    XtSetArg(args[n], XtNcolumns, 4); n++;
    table = new Table("Beam Recipes", frame_form, info_area, args, n);
    table->setAttributes("net,%s,group,%s,nsta,%d,path,%s");
 
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    addPlugins("BeamGroups", NULL, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(stations_button, "Stations");
    }
}

BeamGroups::~BeamGroups(void)
{
}

void BeamGroups::init(void)
{
    setButtonsSensitive(false);

    bg_stations = NULL;
    add_group = NULL;

    readGroups();

    list();
}

void BeamGroups::readGroups(void)
{
    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME"))) {
	    recipe_dir.assign(c + string("/tables/recipes"));
	}
    }
    getProperty("recipeDir2", recipe_dir2);

    Beam::readGroups(recipe_dir, recipe_dir2, groups);
}

void BeamGroups::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Add Group...")) {
	if(!add_group) {
	    add_group = new AddGroup("Add Beam Group", this, this);
	}
	add_group->setVisible(true);
    }
    else if(comp == add_group) {
	readGroups();
	list();
    }
    else if(!strcmp(cmd, "Delete")) {
	deleteGroup();
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Stations...")) {
	listStations();
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	selectGroup((MmTableSelectCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(action_event->getReason(), XtNattributeChangeCallback)) {
	list();
    }
    else if(!strcmp(cmd, "Beam Group Help")) {
	showHelp(cmd);
    }
}

void BeamGroups::list(void)
{
    const char *row[4];
    char nsta[20];
    bool editable[4];
    TAttribute a[4];

    setCursor("hourglass");

    table->removeAllRows();
    int num_columns = table->numColumns();

    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
    }

    for(int i = 0; i < (int)groups.size(); i++)
    {
	for(int j = 0; j < num_columns; j++)
	{
	    editable[j] = false;
	    row[j] = "-";

	    if(!strcmp(a[j].name, "net")) {
		row[j] = groups[i].net;
	    }
	    else if(!strcmp(a[j].name, "group")) {
		row[j] = groups[i].group;
	    }
	    else if(!strcmp(a[j].name, "nsta")) {
		snprintf(nsta, 20, "%d", (int)groups[i].sta.size());
		row[j] = nsta;
	    }
	    else if(!strcmp(a[j].name, "path")) {
		row[j] = quarkToString(groups[i].path);
	    }
	}
	table->addRow(row, false);
    }

    table->adjustColumns();
    table->sortByColumnLabels("net,group");

    table->setColumnEditable(editable);

    setButtonsSensitive(false);

    setCursor("default");
}

void BeamGroups::selectGroup(MmTableSelectCallbackStruct *c)
{
    int i;
    bool set;

    for(i = 0; i < c->nrows && !c->states[i]; i++);
    set = (i < c->nrows) ? true : false;

    setButtonsSensitive(set);
}

void BeamGroups::setButtonsSensitive(bool set)
{
    delete_button->setSensitive(set);
    stations_button->setSensitive(set);
}

void BeamGroups::listStations(void)
{
    vector<int> rows;

    if(table->getSelectedRows(rows) <= 0) return;

    BeamGroup *bg = &groups[rows[0]];

    if(!bg_stations) {
	bg_stations = new BGStations("Beam Group Stations", this);
    }
    bg_stations->setVisible(true);
    bg_stations->list(bg);
}

void BeamGroups::deleteGroup(void)
{
    int nrows;
    vector<int> rows;

    if((nrows = table->getSelectedRows(rows)) <= 0) {
	showWarning("No groups selected.");
	return;
    }
    char s[50], s2[20];
    if(nrows == 1) {
	snprintf(s, sizeof(warn), "Delete 1 Beam Group?");
        strcpy(s2, "Delete Group");
    }
    else {
	snprintf(s, sizeof(warn), "Delete %d Beam Groups?", nrows);
        strcpy(s2, "Delete Groups");
    }
    int ans = Question::askQuestion("Confirm Group Delete", this, s, s2,
			"Cancel");
    if(ans == 1) {
	for(int i = 0; i < nrows; i++) {
	    Beam::deleteGroup(&groups[rows[i]]);
	}
	readGroups();
	list();
    }
}
