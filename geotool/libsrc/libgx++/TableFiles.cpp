/** \file TableFiles.cpp
 *  \brief Defines class TableFiles.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

using namespace std;

#include "TableFiles.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "TableViewer.h"
#include "TableSource.h"

typedef struct
{
    bool viewable;
    const char *table_name;
    const char *name;
    const char *filename;
} TableName;

//add priorityFile, mapOverlapDir

static TableName tables[] =
{
    {false, NULL,	"iaspeiTable",		"models/iasp91"},
    {false, NULL,	"jbTable",		"models/jbtable"},
    {false, NULL,	"crustModels",		"models/crust_models"},
    {false, NULL,	"recipeDir",		"recipes"},
    {false, NULL,	"recipeDir2",		"recipes2"},
    {false, NULL,	"mapDir",		"map"},
    {false, NULL,	"tlModelFile",	 	"mag/tlsf/idc_tlsf.defs"},
    {false, NULL,	"magDescripFile",	"mag/mdf/idc_mdf_ars.defs"},
    {true,"affiliation","affiliationTable",	"static/global.affiliation"},
    {true,"affiliation","affiliationTable2",	"static/local.affiliation"},
    {true, "gregion",	"gregionTable",		"static/global.gregion"},
    {true, "instrument","instrumentTable",	"static/global.instrument"},
    {true, "instrument","instrumentTable2",	"static/local.instrument",},
    {true, "lastid", 	"lastidTable",		"dynamic/global.lastid"},
    {true, "sensor",	"sensorTable",		"static/global.sensor",},
    {true, "sensor",	"sensorTable2",		"static/local.sensor",},
    {true, "site",	"siteTable",		"static/global.site"},
    {true, "site",      "siteTable2",		"static/local.site"},
    {true, "sitechan",	"sitechanTable",	"static/global.sitechan"},
    {true, "sitechan",	"sitechanTable2",	"static/local.sitechan"},
    {false, NULL, NULL, NULL},
};

static int num_tables=0;

static bool files_init = true;



TableFiles::TableFiles(const string &name, Component *parent)
                        : FormDialog(name, parent)
{
    createInterface();
}

void TableFiles::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
    setValues(args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    edit_button = new Button("Edit Path", controls, this);
    edit_button->setSensitive(false);
    view_button = new Button("View", controls, this);
    view_button->setSensitive(false);
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
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNcolumns, 2); n++;
    XtSetArg(args[n], XtNvisibleRows, 30); n++;
    XtSetArg(args[n], XtNwidth, 450); n++;
    const char *column_labels[] = {"table", "path"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    XtSetArg(args[n], XtNcolumnSelectable, False); n++;
    XtSetArg(args[n], XtNtableTitle, "Table Files"); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNeditSaveCallback);
    table->addActionListener(this, XtNeditCancelCallback);

    if(files_init) {
	filesInit();
    }

    list();

    Application::addPropertyListener(this);
}

TableFiles::~TableFiles(void)
{
}

void TableFiles::list(void)
{
    string name;
    const char *row[2];

    table->removeAllRows();

    for(int i = 0; i < 100 && tables[i].name; i++) {
	row[0] = tables[i].name;
	if( getProperty(tables[i].name, name) ) {
	    row[1] = name.c_str();
	}
	else {
	    row[1] = "";
	}
	table->addRow(row, true);
    }
    table->adjustColumns();
}

void TableFiles::actionPerformed(ActionEvent *action_event)
{
//    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(comp == close_button) {
	setVisible(false);
    }
    else if(comp == view_button) {
	view();
    }
    else if(comp == edit_button) {
        table->editModeOn();
	close_button->setSensitive(false);
    }
    else if(comp == help_button) {
	showHelp("Table Files Help");
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	selectFile((MmTableSelectCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(action_event->getReason(), XtNeditSaveCallback)) {
	saveFile();
	close_button->setSensitive(true);
    }
    else if(!strcmp(action_event->getReason(), XtNeditCancelCallback)) {
	close_button->setSensitive(true);
    }
    else if(!strcmp(action_event->getReason(), XtNchangePropertyCallback)) {
	if(comp != this) {
	    propertyChange((char *)action_event->getCalldata());
	}
    }
}

void TableFiles::filesInit(void)
{
    const char *c;
    string prop, geo_table_dir;

    if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	putProperty("geoTableDir", c + string("/tables"), false, this);
    }
    else if(getProperty("geoTableDir", prop)) {
	putProperty("geoTableDir", prop, false, this);
    }

    getProperty("geoTableDir", geo_table_dir);

    for(int i = 0; i < 100 && tables[i].name; i++)
    {
	num_tables++;
	if(!geo_table_dir.empty()) // check if file exists in geo_table_dir
	{
	    struct stat buf;
	    string gpath = geo_table_dir + string("/") + tables[i].filename;

	    // only put this property if it does not already exist and it points to a file.
	    if((!getProperty(tables[i].name, prop) || !stat(prop.c_str(), &buf)) &&
		(!stat(gpath.c_str(), &buf) || !strcmp(tables[i].name, "lastid")
			|| !strcmp(tables[i].name, "iaspeiTable")) )
	    {
		putProperty(tables[i].name, gpath, false, this);
	    }
	}
    }

    files_init = false;
}

void TableFiles::selectFile(MmTableSelectCallbackStruct *c)
{
    int i;
    for(i = 0; i < c->nrows && !c->states[i]; i++);
    if(i == c->nrows) {
	view_button->setSensitive(false);
	edit_button->setSensitive(false);
	edit_file = -1;
	return;
    }
    if(i < 0 || i >= num_tables) return;

    edit_button->setSensitive(true);
    edit_file = i;

    vector<const char *> row;
    table->getRow(i, row);

    if(*row[1] != '\0' && tables[i].viewable) {
	view_button->setSensitive(true);
    }
    else {
	view_button->setSensitive(false);
    }
}

void TableFiles::view(void)
{
    vector<int> selected;
    vector<const char *> row;
    char query[100];
    TableViewer *tv = NULL;

    if(table->getSelectedRows(selected) <= 0) return;
    int i = selected[0];

    table->getRow(i, row);
    
    if( *row[1] != '\0' ) {
	int j;
	for(j = 0; j < (int)windows.size(); j++) {
	    if( !windows[j]->isVisible() ) {
		windows[j]->setTitle(row[1]);
		windows[j]->removeAllTabs();
		tv = windows[j];
/*
		char **labels;
		int n = tv->getLabels(&labels);
		if(n > 0) {
		    tv->setLabel(labels[0], tables[i].table_name);
		}
		Free(labels);
*/

// turn edit off
		windows[j]->setVisible(true);
		break;
	    }
	}
        if(j == (int)windows.size()) {
            tv = new TableViewer("TableViewer", this, row[1]);
	    tv->removeAllTabs();
            windows.push_back(tv);
            tv->setVisible(true);
        }

	TableSource *ds = new TableSource("local");
	ds->openPrefix(row[1]);
	snprintf(query, sizeof(query), "%s select * from %s",
			tables[i].table_name, tables[i].table_name);
	ds->query(query);
	gvector<CssTableClass *> v;
	ds->getTable(tables[i].table_name, v);
	tv->addRecords(v);
	delete ds;
    }
}

void TableFiles::saveFile(void)
{
    if(edit_file < 0) return;

    vector<const char *> row;
    table->getRow(edit_file, row);
    putProperty(tables[edit_file].name, row[1], true);
    FFDatabase::clearStaticTables(); // force TableSource to read static tables
}

void TableFiles::propertyChange(char *name)
{
    string prop;
    for(int i = 0; i < num_tables; i++) {
	if(!strcmp(name, tables[i].name)) {
	    const char *row[2];
	    row[0] = tables[i].name;
	    if( getProperty(tables[i].name, prop) ) {
		row[1] = prop.c_str();
	    }
	    else {
		row[1] = "";
	    }
	    table->setRow(i, row);
	    table->adjustColumns();
	    return;
	}
    }
}
