/** \file MapMeasureForm.cpp
 *  \brief Defines class MapMeasureForm.
 *  \author Ivan Henson
 */
#include "config.h"
#include "MapMeasureForm.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "widget/Map.h"

extern "C" {
#include "libstring.h"
}
using namespace libgmap;

#define DEG_TO_KM       111.19492664            /* for R = 6371 */

MapMeasureForm::MapMeasureForm(const char *name, Component *parent,
		Map *map_src) : Frame(name, parent, false, false)
{
    map = map_src;
    createInterface();
    init();
}

void MapMeasureForm::createInterface()
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    view_menu = new Menu("View", menu_bar);
    clear_selected = new Button("Clear Selected", view_menu, this);
    clear_selected->setSensitive(false);
    clear_all = new Button("Clear All", view_menu, this);
    attributes_button = new Button("Attributes...", view_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Map Measure Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNwidth, 500); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
//    XtSetArg(args[n], XtNtableTitle, "Beam Groups"); n++;
    XtSetArg(args[n], XtNcolumns, 6); n++;
    table = new Table("Map Measure", frame_form, info_area, args, n);
    table->setAttributes(
"label,%s,lat,%.2f,lon,%.2f,distance(deg),%.2f,distance(km),%.2f,azimuth,%.2f");
 
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    addPlugins("MapMeasureForm", NULL, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(clear_selected, "Clear Selected");
	tool_bar->add(clear_all, "Clear All");
    }
}

MapMeasureForm::~MapMeasureForm(void)
{
    Free(ids);
}

void MapMeasureForm::setVisible(bool visible)
{
    Frame::setVisible(visible);
    Arg args[1];
    if(visible) {
	added_crosshair = false;
	if( !map->hasCrosshair() ) {
	    added_crosshair = true;
	    map->addCrosshair();
	}
	XtSetArg(args[0], XtNmeasure, True);
	map->setValues(args, 1);
	map->addActionListener(this, XtNmapMeasureCallback);
	list();
    }
    else {
	XtSetArg(args[0], XtNmeasure, False);
	map->setValues(args, 1);
	map->removeActionListener(this, XtNmapMeasureCallback);
	map->measureDeleteAll();
	num_ids = 0;
	if(added_crosshair) {
	    map->deleteCrosshair();
        }
    }
}

void MapMeasureForm::init(void)
{
    num_ids = 0;
    ids = NULL;
    added_crosshair = false;
}

void MapMeasureForm::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Clear Selected")) {
	deleteSelected();
    }
    else if(!strcmp(cmd, "Clear All")) {
	map->measureDeleteAll();
	num_ids = 0;
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	clear_selected->setSensitive(table->rowSelected());
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(action_event->getReason(), XtNattributeChangeCallback)) {
	list();
    }
    else if(!strcmp(action_event->getReason(), XtNmapMeasureCallback)) {
	measure((MapMeasure *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Map Measure Help")) {
	showHelp("Map Measure Help");
    }
}

void MapMeasureForm::list(void)
{
    const char *row[6];
    bool editable[6];
    TAttribute a[6];

    table->removeAllRows();
    int num_columns = table->numColumns();

    Free(ids);
    num_ids = 0;

    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
	editable[i] = false;
    }
    table->setColumnEditable(editable);

    MapMeasure *m = NULL;
    int num = map->getMeasurements(&m);

    ids = (int *)mallocWarn(num*sizeof(int));
    num_ids = 0;

    for(int i = 0; i < num; i++)
    {
	getRow(&m[i], num_columns, (TAttribute *)a, (char **)row);
	table->addRow(row, false);
	ids[num_ids++] = m[i].id;
	for(int j = 0; j < num_columns; j++) Free(row[j]);
    }

    table->adjustColumns();
//    table->sortByColumnLabels("net,group");

    Free(m);
}

void MapMeasureForm::getRow(MapMeasure *m, int num_columns, TAttribute *a,
			char **row)
{
    char buf[20];

    for(int j = 0; j < num_columns; j++)
    {
	buf[0] = '\0';

	if(!strcmp(a[j].name, "label")) {
	  if (m->label != NULL) {
	    row[j] = strdup(m->label);
	  }
	  else {
	    row[j] = strdup("");
	  }
	}
	else if(!strcmp(a[j].name, "lat")) {
	    snprintf(buf, sizeof(buf), a[j].format, m->lat);
	    if(buf[0] == '\0') {
		snprintf(buf, sizeof(buf), "%.2f", m->lat);
	    }
	    row[j] = strdup(buf);
	}
	else if(!strcmp(a[j].name, "lon")) {
	    snprintf(buf, sizeof(buf), a[j].format, m->lon);
	    if(buf[0] == '\0') {
		snprintf(buf, sizeof(buf), "%.2f", m->lon);
	    }
	    row[j] = strdup(buf);
	}
	else if(!strcmp(a[j].name, "distance(deg)")) {
	    if(m->type == MAP_DELTA) {
		snprintf(buf, sizeof(buf), a[j].format, m->delta);
		if(buf[0] == '\0') {
		    snprintf(buf, sizeof(buf), "%.2f", m->delta);
		}
		row[j] = strdup(buf);
	    }
	    else row[j] = strdup("-");
	}
	else if(!strcmp(a[j].name, "distance(km)")) {
	    if(m->type == MAP_DELTA) {
		double d = m->delta*DEG_TO_KM;
		snprintf(buf, sizeof(buf), a[j].format, d);
		if(buf[0] == '\0') {
		    snprintf(buf, sizeof(buf), "%.2f", d);
		}
		row[j] = strdup(buf);
	    }
	    else row[j] = strdup("-");
	}
	else if(!strcmp(a[j].name, "azimuth")) {
	    if(m->type == MAP_ARC) {
		snprintf(buf, sizeof(buf), a[j].format, m->az);
		if(buf[0] == '\0') {
		    snprintf(buf, sizeof(buf), "%.2f", m->az);
		}
		row[j] = strdup(buf);
	    }
	    else row[j] = strdup("-");
	}
	else row[j] = strdup("-");
    }
}

void MapMeasureForm::measure(MapMeasure *m)
{
    if(m->type == -1) {
	list();
	return;
    }
    const char *row[6];
    TAttribute a[6];
    int num_columns = table->numColumns();
    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
    }

    int i;
    for(i = 0; i < num_ids && ids[i] != m->id; i++);
    getRow(m, num_columns, (TAttribute *)a, (char **)row);
    if(i < num_ids) {
	table->setRow(i, row);
    }
    else {
	table->addRow(row, true);
	if(!ids) ids = (int *)mallocWarn(sizeof(int));
	else ids = (int *)realloc(ids, (num_ids+1)*sizeof(int));
	ids[num_ids++] = m->id;
    }
    for(int j = 0; j < num_columns; j++) Free(row[j]);
    table->adjustColumns();
}

void MapMeasureForm::deleteSelected(void)
{
    vector<int> rows;
    table->getSelectedRows(rows);

    for(int i = 0; i < (int)rows.size(); i++) {
	map->measureDelete(ids[rows[i]]);
    }
    list();
}
