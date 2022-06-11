/** \file MapThemes.cpp
 *  \brief Defines class MapThemes.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/param.h>

using namespace std;

#include "MapThemes.h"
#include "MapThemeFile.h"
#include "MapWindow.h"
#include "MapOverlay.h"
#include "ShapeTable.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#ifdef HAVE_NETCDF
#include "libgCDF.h"
#endif /* HAVE_NETCDF */
}

using namespace libgmap;

MapThemes::MapThemes(const char *name, Component *parent, MapWindow *mapWindow)
		: FormDialog(name, parent)
{
    mw = mapWindow;

    addCursorCallbacks(mw->map);

    theme_files = new gvector<MapThemeFile *>;
    last_theme_id = -1;
    last_shape_index = -1;

    createInterface();
}

void MapThemes::addCursorCallbacks(Map *map)
{
    XtAddCallback((Widget)map->baseWidget(), XtNcursorMotionCallback,
		MapThemes::cursorMotionCB, (XtPointer)this);
    XtAddCallback((Widget)map->baseWidget(), XtNshapeSelectCallback,
		MapThemes::shapeSelectCB, (XtPointer)this);
}

void MapThemes::createInterface(void)
{
    const char *column_labels[] = {
		"Level", "Name", "Display", "Labels", "On-Delta",
		"Off-Delta", "Cursor Column", "Color Column", "Color Bar",
		"Bndry", "Bndry Color", "Fill", "Fill Color", "Symbol Type",
		"Sym Size", "Nshapes", "ShapeType", "Path"};
    const char *column_choice[] = {
		"","","on:off","","","","","","on:off","on:off","", "on:off","",
"circle:solid-circle:square:solid-square:triangle:solid-triangle:plus:diamond:solid-diamond:inv-triangle:solid-inv-tri",
		"1:2:3:4:5:6:7:8:9:10","","",""};
    Boolean column_editable[] = {False, False, True, True, True, True,
		True, True, True, True, True, True, True, True,
		True, False, False, False};
    Arg args[20];
    int n;

    setSize(850, 300);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    shape_table_button = new Button("Shape Table...", controls, this);
    shape_table_button->setSensitive(false);
    color_table_button = new Button("Color Table...", controls, this);
    color_table_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    updn = new RowColumn("controls", controls, args, n);

    up_arrow = new ArrowButton("up", updn, XmARROW_UP, this);
    up_arrow->setSensitive(false);
    down_arrow = new ArrowButton("down", updn, XmARROW_DOWN, this);
    down_arrow->setSensitive(false);

    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    remove_button = new Button("Remove", controls, this);
    remove_button->setSensitive(false);
    save_button = new Button("Save", controls, this);

    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

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
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    rc = new RowColumn("rc", this, args, n);

    label = new Label("Map Themes", rc);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], (char *)XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], (char *)XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], (char *)XtNcolumns, 18); n++;
    XtSetArg(args[n], (char *)XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], (char *)XtNcolumnChoice, column_choice); n++;
    XtSetArg(args[n], (char *)XtNcolumnEditable, column_editable); n++;
    XtSetArg(args[n], (char *)XtNeditable, True); n++;
    XtSetArg(args[n], (char *)XtNsingleSelect, True); n++;
    XtSetArg(args[n], (char *)XtNselectToggles, True); n++;

    table = new Table("table", this, args, n);
    table->setVisible(true);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNchoiceChangedCallback);
    table->addActionListener(this, XtNvalueChangedCallback);
}

MapThemes::~MapThemes(void)
{
    delete theme_files;

    XtRemoveCallback((Widget)mw->map->baseWidget(), XtNcursorMotionCallback,
		MapThemes::cursorMotionCB, (XtPointer)this);
    XtRemoveCallback((Widget)mw->map->baseWidget(), XtNshapeSelectCallback,
		MapThemes::shapeSelectCB, (XtPointer)this);
}

void MapThemes::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
        setVisible(false);
    }
    else if(!strcmp(cmd, "Shape Table...")) {
	displayShapeTable();
    }
    else if(!strcmp(cmd, "Color Table...")) {
	displayColorTable();
    }
    else if(!strcmp(cmd, "Apply")) {
	applyChange();
    }
    else if(!strcasecmp(cmd, "up")) {
	changeThemeLevel(false);
    }
    else if(!strcasecmp(cmd, "down")) {
	changeThemeLevel(true);
    }
    else if(!strcmp(cmd, "Save")) {
	saveThemes();
    }
    else if(!strcmp(cmd, "Remove")) {
	removeThemes();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Map Themes Help");
    }
    else if(!strcmp(cmd, "table")) {
	if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	    selectRow();
	}
	else if(!strcmp(action_event->getReason(), XtNchoiceChangedCallback)) {
	    themesChoice((MmTableEditCallbackStruct *)action_event->getCalldata());
	}
	else if(!strcmp(action_event->getReason(), XtNvalueChangedCallback)) {
	    themesEdit((MmTableEditCallbackStruct *)action_event->getCalldata());
	}
    }
}

ParseCmd MapThemes::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseArg(cmd, "theme_level", c)) {
	if(!table->rowSelected()) {
	    msg.assign("themes.theme level: no theme selected.");
	    return ARGUMENT_ERROR;
	}
	if(parseCompare(c, "up")) {
	    changeThemeLevel(false);
	}
	else if(parseCompare(c, "down")) {
	    changeThemeLevel(true);
	}
	else {
	    msg.assign(string("themes.theme level: invalid arg: ") + c);
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "Shape_Table", c)) {
	int i;
	vector<int> rows;

	if(!table->getSelectedRows(rows)) {
	    msg.assign("themes.Shape Table: no theme selected.");
	    return ARGUMENT_ERROR;
	}
 	i = rows[0];
	if(i < theme_files->size()) {
	    MapThemeFile *tf = theme_files->at(i);
	    tf->createShapeTable();
	    if(tf->info_window) {
		return tf->info_window->parseCmd(c, msg);
	    }
	}
    }
    else if(parseArg(cmd, "Color_Table", c)) {
	int i;
	vector<int> rows;

	if(!table->getSelectedRows(rows)) {
	    msg.assign("themes.Color Table: no theme selected.");
	    return ARGUMENT_ERROR;
	}
 	i = rows[0];
	if(i < theme_files->size()) {
	    MapThemeFile *tf = theme_files->at(i);
	    tf->showColorTable(this);
	    if(tf->theme_colors_window) {
		return tf->theme_colors_window->parseCmd(c, msg);
	    }
	}
    }
    else if(parseCompare(cmd, "Apply")) {
	applyChange();
    }
    else if(parseCompare(cmd, "Save")) {
	saveThemes();
    }
    else if(parseCompare(cmd, "Remove")) {
	removeThemes();
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200], *s;
	getParsePrefix(prefix, sizeof(prefix));
	// change "map themes" to "themes"
	if((s = strstr(prefix, "map themes"))) {
	    strcpy(s, "themes.");
	}
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void MapThemes::parseHelp(const char *prefix)
{
    printf("%stheme_level up\n", prefix);
    printf("%stheme_level down\n", prefix);
    Table::parseHelp(prefix);
    char p[200];
    snprintf(p, sizeof(p), "%sshape_table", prefix);
    Table::parseHelp(p);
//    printf("%scolor table\n", prefix);
    printf("%sapply\n", prefix);
    printf("%ssave\n", prefix);
    printf("%sremove\n", prefix);
}

void MapThemes::selectRow(void)
{
    int num, nrows;
    vector<int> rows;

    shape_table_button->setSensitive(table->rowSelected());
    color_table_button->setSensitive(table->rowSelected());
    remove_button->setSensitive(table->rowSelected());

    nrows = table->numRows();
    num = table->getSelectedRows(rows);
    if(!num) {
	up_arrow->setSensitive(false);
	down_arrow->setSensitive(false);
    }
    else {
	up_arrow->setSensitive(true);
	down_arrow->setSensitive(true);
        if(rows[0] == 0) {
	    up_arrow->setSensitive(false);
	}
	if(rows[0] == nrows-1) {
	    down_arrow->setSensitive(false);
	}
    }
}

void MapThemes::readInitialThemes(void)
{
    int i, num_themes=0;
    const char *geotool_home;
    char *prop, name[200];

    if((prop = getProperty("num_map_themes")))
    {
	if(!stringToInt(prop, &num_themes) || num_themes < 0) {
	    showWarning("Invalid num_map_themes in .geotool");
	    return;
	}
        Free(prop);
	for(i = 0; i < num_themes; i++)
	{
	    snprintf(name, sizeof(name), "theme%d.path", i+1);
	    if(!(prop = getProperty(name))) {
		showWarning("Invalid %s in .geotool", name);
		return;
	    }
	    mw->readFile(prop);
	    Free(prop);
	}
    }
    if(!num_themes &&
	(geotool_home = Application::getInstallDir("GEOTOOL_HOME")))
    {
	char path[MAXPATHLEN+1];
	struct stat buf;

	putProperty("num_map_themes", "2", false);

	snprintf(path, sizeof(path),
		"%s/tables/map/shapefiles/lowres/cntry00.shp", geotool_home);
	putProperty("theme1.path", path, false);
	putProperty("theme.cntry00.on_delta", "360.", false);
	putProperty("theme.cntry00.off_delta", "50.", false);
	putProperty("theme.cntry00.fill", "true.", false);
	putProperty("theme.cntry00.cursor_column", "6", false);
	mw->readFile(path);

	snprintf(path, sizeof(path),
	    "%s/tables/map/shapefiles/f_pol/vpf/polbnda.shp.gz", geotool_home);
	if( stat(path, &buf) ) {
	    snprintf(path, sizeof(path),
		"%s/tables/map/shapefiles/f_pol/vpf/polbnda.shp", geotool_home);
	}
	putProperty("theme2.path", path, false);
	putProperty("theme.polbnda.on_delta", "49.9", false);
	putProperty("theme.polbnda.off_delta", "0.", false);
	putProperty("theme.polbnda.fill", "true", false);
	putProperty("theme.polbnda.cursor_column", "5", false);
	mw->readFile(path);
    }
}

void MapThemes::changeThemeLevel(bool raise_level)
{
    int row, nrows;
    vector<int> rows;
    char buf[20];

    if((nrows = table->numRows()) <= 1) return;

    if(!table->getSelectedRows(rows)) return;

    row = rows[0];

    if((!raise_level && row == 0) || (raise_level && row == nrows-1)) return;

    MapThemeFile *tf = theme_files->at(row);

    if(!raise_level)
    {
	theme_files->exchange(row-1, row);
	mw->map->themeLevelDown(tf->id);
	table->moveRowUp(row);
	snprintf(buf, sizeof(buf), "%d", row-1);
	table->setField(row-1, 0, buf, true);
	snprintf(buf, sizeof(buf), "%d", row);
	table->setField(row, 0, buf, true);
	if(row == 1) {
	    up_arrow->setSensitive(false);
	}
	if(row == nrows-1) {
	    down_arrow->setSensitive(true);
	}
    }
    else {
	theme_files->exchange(row+1, row);
	mw->map->themeLevelUp(tf->id);
	table->moveRowDown(row);
	snprintf(buf, sizeof(buf), "%d", row+1);
	table->setField(row+1, 0, buf, true);
	snprintf(buf, sizeof(buf), "%d", row);
	table->setField(row, 0, buf, true);
	if(row == nrows-2) {
	    down_arrow->setSensitive(false);
	}
	if(row == 0) {
	    up_arrow->setSensitive(true);
	}
    }
}

void MapThemes::shapeSelectCB(Widget w, XtPointer client, XtPointer data)
{
    MapThemes *map_themes = (MapThemes *)client;
    MapCursorCallbackStruct *m = (MapCursorCallbackStruct *)data;
    int i;
    MapThemeFile *tf=NULL;

    for(i = 0; i < map_themes->theme_files->size(); i++) {
	tf = map_themes->theme_files->at(i);
	if(tf->id == m->shape_theme_id) break;
    }

    if(i == map_themes->theme_files->size()) return;

    if(tf->shape_table) {
	tf->shape_table->selectRow(m->shape_index, m->selected);
	if(m->selected) {
	    tf->shape_table->moveToTop(m->shape_index);
	}
	bool set = tf->shape_table->rowSelected();
	tf->info_window->turn_on_button->setSensitive(set);
	tf->info_window->turn_off_button->setSensitive(set);
    }
}

void MapThemes::cursorMotionCB(Widget w, XtPointer client, XtPointer data)
{
    MapThemes *map_themes = (MapThemes *)client;
    MapCursorCallbackStruct *m = (MapCursorCallbackStruct *)data;
    MapThemeFile *tf=NULL;
    char *s, info[1000];
    int i;

    if(m->info2 == NULL) return;

    if(m->shape_theme_id == map_themes->last_theme_id
	&& m->shape_index == map_themes->last_shape_index) return;

    map_themes->last_theme_id = m->shape_theme_id;
    map_themes->last_shape_index = m->shape_index;

    info[0] = '\0';
    for(i = 0; i < map_themes->theme_files->size(); i++) {
	tf = map_themes->theme_files->at(i);
	if(tf->id == m->shape_theme_id) break;
    }
    if(i < map_themes->theme_files->size())
    {
	if(tf->shape_table && (s = tf->shape_table->getField(m->shape_index,
			tf->cursor_column)) != NULL)
	{
	    snprintf(info, 1000, "%d %d %s", i, m->shape_index, s);
	    InfoSetText(m->info2, info);
	    Free(s);
	}
	else {
	    snprintf(info, 1000, "%d %d", i, m->shape_index);
	    InfoSetText(m->info2, info);
	}
    }
}

void MapThemes::themesChoice(MmTableEditCallbackStruct *c)
{
    int i;
    vector<const char *> labels, lab, col;
    Boolean state;
    MapThemeFile *tf;

    if(!mw->map || c->row < 0 || c->row >= theme_files->size()) return; 

    tf = theme_files->at(c->row);

    if( !table->getColumnLabels(labels) ) return;

    state = isTrue(c->new_string);

    if(!strcasecmp(labels[c->column], "Display"))
    {
	if(state != tf->display) {
	    mw->map->displayTheme(tf->id, state);
	    tf->display = state;
	}
    }
    else if(!strcasecmp(labels[c->column], "Labels"))
    {
	if(tf->shape_table != NULL) {
	    int num = tf->shape_table->getColumnLabels(lab);
	    for(i = 0; i < num && strcmp(c->new_string, lab[i]); i++);
	    if(i < num) {
		if(tf->label_column != i) {
		    tf->label_column = i;
		    tf->shape_table->getColumn(tf->label_column, col);
		    mw->map->shapeLabels(tf->id, col);
		    mw->map->displayShapeLabels(tf->id, true);
		}
	    }
	    else if(tf->label_column != -1) {
		tf->label_column = -1;
		mw->map->displayShapeLabels(tf->id, false);
	    }
	}
	else {
	    tf->label_column = -1;
	}
    }
    else if(!strcasecmp(labels[c->column], "Cursor Column"))
    {
	if(!strcmp(c->new_string, "off")) {
	    tf->cursor_info = false;
	}
	else if(tf->shape_table != NULL) {
	    int num = tf->shape_table->getColumnLabels(lab);
	    for(i = 0; i < num && strcmp(c->new_string, lab[i]); i++);
	    if(i < num) {
		tf->cursor_column = i;
		tf->cursor_info = true;
	    }
	}
	else if(!strcmp(c->new_string, "on")) {
		tf->cursor_info = true;
	}
	mw->map->themeCursor(tf->id, tf->cursor_info);
	last_theme_id = last_shape_index = -1;
    }
    else if(!strcasecmp(labels[c->column], "Color Column"))
    {
	tf->colorColumn(c->new_string);
    }
    else if(!strcasecmp(labels[c->column], "Color Bar"))
    {
	mapColorBar(tf, c->row, state);
    }
    else if(!strcasecmp(labels[c->column], "Bndry"))
    {
	if(state != tf->bndry) {
	    tf->bndry = state;
	    mw->map->themeBndry(tf->id, state);
	}
    }
    else if(!strcasecmp(labels[c->column], "Fill"))
    {
	if(state != tf->fill) {
	    tf->fill = state;
	    mw->map->themeFill(tf->id, state);
	}
    }
    else if(!strcasecmp(labels[c->column], "Symbol Type"))
    {
	int type = DrawSymbolFromName(c->new_string);
	if(type >= 0) {
	    mw->map->themeSymbolType(tf->id, type);
	}
    }
    else if(!strcasecmp(labels[c->column], "Sym Size"))
    {
	int size;
	if(stringToInt(c->new_string, &size)) {
	    mw->map->themeSymbolSize(tf->id, size);
	}
    }
}

void MapThemes::mapColorBar(MapThemeFile *tf, int row, Boolean state)
{
    int i, nrows;
    vector<const char *> col;

    if(state == tf->color_bar) return;

    if(state) {
	nrows = table->getColumnByLabel("Color Bar", col);
	col[row] = "on";
	for(i = 0; i < nrows; i++) {
	    if(i != row) col[i] = "off";
	}
	table->setColumnByLabel("Color Bar", col);
	tf->color_bar = true;
	for(i = 0; i < theme_files->size(); i++) {
	    MapThemeFile *t = theme_files->at(i);
	    if(i != row) t->color_bar = false;
	}
	mw->map->themeColorBar(tf->id);
	tf->setColorScale();
    }
    else {
	tf->color_bar = false;
	mw->map->themeColorBarOff();
    }
}

void MapThemes::themesEdit(MmTableEditCallbackStruct *c)
{
    vector<const char *> labels;
    int i, ncols;
    double d;
    MapThemeFile *tf;

    if(c->row < 0 || c->row >= theme_files->size()) return; 
    tf = theme_files->at(c->row);

    if( !(ncols = table->getColumnLabels(labels)) ) return;

    for(i = 0; i < ncols && strcmp(labels[i], "On-Delta"); i++);
    if(i == ncols) {
	return;
    }
    if(c->column == i)	/* On-Delta has been edited */
    {
	if(stringToDouble(c->new_string, &d) && d != tf->on_delta) {
	    tf->on_delta_changed = true;
	    apply_button->setSensitive(true);
	}
	else {
	    tf->on_delta_changed = false;
	    apply_button->setSensitive(false);
	}
    }

    for(i = 0; i < ncols && strcmp(labels[i], "Off-Delta"); i++);
    if(i == ncols) {
	return;
    }
    if(c->column == i)	/* Off-Delta has been edited */
    {
	if(stringToDouble(c->new_string, &d) && d != tf->off_delta) {
	    tf->off_delta_changed = true;
	    apply_button->setSensitive(true);
	}
	else {
	    tf->off_delta_changed = false;
	    apply_button->setSensitive(false);
	}
    }

    for(i = 0; i < ncols && strcmp(labels[i], "Bndry Color"); i++);
    if(i == ncols) {
	return;
    }
    if(c->column == i)	/* Bndry Color has been edited */
    {
	if(strcmp(c->new_string, tf->bndry_color)) {
	    tf->bndry_color_changed = true;
	    apply_button->setSensitive(true);
	}
	else {
	    tf->bndry_color_changed = false;
	    apply_button->setSensitive(false);
	}
    }

    for(i = 0; i < ncols && strcmp(labels[i], "Fill Color"); i++);
    if(i == ncols) {
	return;
    }
    if(c->column == i)	/* Fill Color has been edited */
    {
	if(strcmp(c->new_string, tf->fill_color)) {
	    tf->fill_color_changed = true;
	    apply_button->setSensitive(true);
	}
	else {
	    tf->fill_color_changed = false;
	    apply_button->setSensitive(false);
	}
    }
}

void MapThemes::applyChange(void)
{
    int i;
    double d;
    vector<const char *> onDelta, offDelta, bndry, fill;
    MapThemeFile *tf=NULL;

    if(!theme_files->size()) return;

    tf = theme_files->at(0);
    if( !table->getColumnByLabel("On-Delta", onDelta) ) return;
    if( !table->getColumnByLabel("Off-Delta", offDelta) ) return;
    if( !table->getColumnByLabel("Bndry Color", bndry) ) return;
    if( !table->getColumnByLabel("Fill Color", fill) ) return;

    for(i = 0; i < theme_files->size(); i++)
    {
	tf = theme_files->at(i);

	if(tf->on_delta_changed) {
	    if(stringToDouble(onDelta[i], &d)) {
		tf->on_delta = d;
	    }
	    else {
		showWarning("Invalid On-Delta: %s", onDelta[i]);
		tf->on_delta_changed = false;
	    }
	}
	if(tf->off_delta_changed) {
	    if(stringToDouble(offDelta[i], &d)) {
		tf->off_delta = d;
	    }
	    else {
		showWarning("Invalid Off-Delta: %s", offDelta[i]);
		tf->off_delta_changed = false;
	    }
	}
	if(tf->on_delta_changed || tf->off_delta_changed) {
	    mw->map->themeOnOffDelta(tf->id, tf->on_delta, tf->off_delta,false);
	    tf->on_delta_changed = false;
	    tf->off_delta_changed = false;
	}
	if(tf->bndry_color_changed) {
	    Free(tf->bndry_color);
	    tf->bndry_color = strdup(bndry[i]);
	    mw->map->themeBndryColor(tf->id, bndry[i], false);
	    tf->bndry_color_changed = false;
	}
	if(tf->fill_color_changed) {
	    Free(tf->fill_color);
	    tf->fill_color = strdup(fill[i]);
	    mw->map->themeFillColor(tf->id, fill[i], false);
	    tf->fill_color_changed = false;
	}
    }

    apply_button->setSensitive(false);

    if(tf) mw->map->update();
}

bool MapThemes::isTrue(char *s)
{
    return (s[0] == '1' || s[0] == 'T' || s[0] == 't' ||
		!strcasecmp(s, "on")) ? true : false;
}

void MapThemes::addThemeRow(MapThemeFile *tf, bool new_theme)
{
    const char *row[18];
    vector<const char *> labels;
    char level[20], nshapes[20], sym_size[20];
    char choice[1000], onDelta[20], offDelta[20];
    int i, j, ncols=0, i_theme;
    int iLevel, iName, iDisplay, iLabels, iOnDelta, iOffDelta;
    int iCursorCol, iColorCol, iColorBar, iBndry, iBndryColor;
    int iFill, iFillColor, iSymType, iSymSize, iNshapes, iShapeType, iPath;

    for(i = 0; i < theme_files->size() && theme_files->at(i) != tf; i++);
    if(i == theme_files->size()) return;
    i_theme = i;

    iLevel =	table->getColumnIndex("Level");
    iName =	table->getColumnIndex("Name");
    iDisplay =	table->getColumnIndex("Display");
    iLabels =	table->getColumnIndex("Labels");
    iOnDelta =	table->getColumnIndex("On-Delta");
    iOffDelta =	table->getColumnIndex("Off-Delta");
    iCursorCol =table->getColumnIndex("Cursor Column");
    iColorCol =	table->getColumnIndex("Color Column");
    iColorBar =	table->getColumnIndex("Color Bar");
    iBndry =	table->getColumnIndex("Bndry");
    iBndryColor=table->getColumnIndex("Bndry Color");
    iFill =	table->getColumnIndex("Fill");
    iFillColor =table->getColumnIndex("Fill Color");
    iSymType =	table->getColumnIndex("Symbol Type");
    iSymSize =	table->getColumnIndex("Sym Size");
    iNshapes = 	table->getColumnIndex("Nshapes");
    iShapeType =table->getColumnIndex("ShapeType");
    iPath = 	table->getColumnIndex("Path");

    snprintf(level, sizeof(level), "%d", i);
    row[iLevel] = level;
    row[iName] = tf->theme_name;
    row[iDisplay] = (char *)(tf->display ? "on" : "off");
    snprintf(onDelta, sizeof(onDelta), "%.1f", tf->on_delta);
    snprintf(offDelta, sizeof(offDelta), "%.1f", tf->off_delta);
    row[iOnDelta] = onDelta;
    row[iOffDelta] = offDelta;

    row[iLabels] = "off";
    row[iCursorCol] = "off";
    row[iColorCol] = "off";

    strcpy(choice, "off:");

    if(tf->shape_table)
    {
	ncols = tf->shape_table->getColumnLabels(labels);
	for(j = 0; j < ncols; j++) if(j != 1) {
	    if(strlen(choice) + strlen(labels[j]) + 2 < sizeof(choice))
	    {
		strcat(choice, labels[j]);
		if(j < ncols-1) strcat(choice,":");
	    }
	}
	if(tf->label_column >= 0 && tf->label_column < ncols) {
	    row[iLabels] = labels[tf->label_column];
	}
	if(tf->cursor_column >= 0 && tf->cursor_column < ncols) {
	    row[iCursorCol] = labels[tf->cursor_column];
	}
	if(tf->color_column >= 0 && tf->color_column < ncols) {
	    row[iColorCol] = labels[tf->color_column];
	}
    }
    snprintf(nshapes, sizeof(nshapes), "%d", tf->nshapes);
    row[iColorBar] = (char *)(tf->color_bar ? "on" : "off");
    row[iBndry] = (char *)(tf->bndry ? "on" : "off");
    row[iBndryColor] = tf->bndry_color;
    row[iFill] = (char *)(tf->fill ? "on" : "off");
    row[iFillColor] = tf->fill_color;
    switch(tf->sym_type)
    {
	case CIRCLE:		row[iSymType] = "circle"; break;
	case FILLED_CIRCLE:	row[iSymType] = "solid-circle"; break;
	case SQUARE:		row[iSymType] = "square"; break;
	case FILLED_SQUARE:	row[iSymType] = "solid-square"; break;
	case TRIANGLE:		row[iSymType] = "triangle"; break;
	case FILLED_TRIANGLE:	row[iSymType] = "solid-triangle"; break;
	case PLUS:		row[iSymType] = "plus"; break;
	case DIAMOND:		row[iSymType] = "diamond"; break;
	case FILLED_DIAMOND:	row[iSymType] = "solid-diamond"; break;
	case INV_TRIANGLE:	row[iSymType] = "inv-triangle"; break;
	case FILLED_INV_TRIANGLE: row[iSymType] = "solid-inv-tri"; break;
	default:		row[iSymType] = "circle"; break;
    }
    snprintf(sym_size, sizeof(sym_size), "%d", tf->sym_size);
    row[iSymSize] = sym_size;
    row[iNshapes] = nshapes;

    if(tf->theme_type != THEME_IMAGE_FILE) {
	row[iShapeType] = (char *)SHPTypeName(tf->shape_type);
    }
    else {
	row[iShapeType] = "";
    }
    row[iPath] = tf->path;

    if(tf->shape_type != SHPT_POINT) {
	row[iSymType] = "";	/* Symbol Type */
	row[iSymSize] = "";	/* Sym Size */
    }
    if(tf->shape_type != SHPT_POLYGON) {
	row[iBndry] = "";	/* Bndry */
	row[iBndryColor] = "";	/* Bndry Color */
	row[iFill] = "";	/* Fill */
    }

    if(tf->theme_type == THEME_IMAGE_FILE) {
	row[iLabels] = "";
	row[iColorCol] = "image";
	row[iBndry] = "";
	row[iBndryColor] = "";
	row[iFill] = "";
	row[iFillColor] = "";
	row[iSymType] = "";
	row[iSymSize] = "";
	row[iNshapes] = "";
	row[iShapeType] = "";
    }

    if(new_theme) {
	table->addRow(row, false);
    }
    else {
	table->setRow(i_theme, row);
    }
    if(tf->shape_table) {
	table->setCellChoice(i_theme, iLabels, choice);
	table->setCellChoice(i_theme, iCursorCol, choice);
	if(strlen(choice) + strlen(":sort-order") + 1 < sizeof(choice)) {
	    strcat(choice, ":sort-order");
	}
	table->setCellChoice(i_theme, iColorCol, choice);
    }
    if(tf->shape_type == SHPT_POLYGON) {
	table->setCellEditable(i_theme, iBndry, true);
	table->setCellEditable(i_theme, iBndryColor, true);
	table->setCellEditable(i_theme, iFill, true);
	table->setCellEditable(i_theme, iSymType, false);
	table->setCellEditable(i_theme, iSymSize, false);
    }
    else if(tf->shape_type == SHPT_POINT) {
	table->setCellEditable(i_theme, iBndry, false);
	table->setCellEditable(i_theme, iBndryColor, false);
	table->setCellEditable(i_theme, iFill, false);
	table->setCellEditable(i_theme, iSymType, true);
	table->setCellEditable(i_theme, iSymSize, true);
    }
    else if(tf->shape_table) { /* everything else, including SHPT_ARC */
	table->setCellEditable(i_theme, iBndry, false);
	table->setCellEditable(i_theme, iBndryColor, false);
	table->setCellEditable(i_theme, iFill, false);
	table->setCellEditable(i_theme, iSymType, false);
	table->setCellEditable(i_theme, iSymSize, false);
    }
    else if(tf->theme_type == THEME_IMAGE_FILE) {
	table->setCellChoice(i_theme, iCursorCol, "on:off");
	table->setCellEditable(i_theme, iLabels, false);
	table->setCellEditable(i_theme, iColorCol, false);
	for(i = iBndry; i < iPath; i++) {
	    table->setCellEditable(i_theme, i, false);
	}
    }
}

bool MapThemes::readShapeFile(const char *path)
{
    SHPHandle shp;
    char *theme_name;
    int i, nEntities, nShapeType;
    double adfMinBound[4], adfMaxBound[4];
    MapPlotTheme theme;
    MapThemeFile *tf=NULL;
    bool new_theme;

    if(!(shp = SHPOpen(path, "rb"))) {
	showWarning("Cannot open shapefile %s", path);
	return false;
    }

    SHPGetInfo(shp, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
    theme_name = getThemeName(path);

    for(i = 0; i < theme_files->size(); i++) {
	tf = theme_files->at(i);
	if(!strcmp(tf->path, path)) break;
    }
    if(i < theme_files->size()) {
	mw->map->mapDelete(tf->id, false);
	new_theme = false;
    }
    else {
	bool first = (!theme_files->size()) ? true : false;
	new_theme = true;
	tf = new MapThemeFile(mw, THEME_SHAPE_FILE, (const char *)theme_name,
			path, nShapeType, nEntities, first);
	theme_files->add(tf);
    }
    free(theme_name);

    if(!(theme.shape_fg = (Pixel *)mallocWarn(nEntities*sizeof(Pixel))) ||
      !(theme.shapes = (SHPObject **)mallocWarn(nEntities*sizeof(SHPObject *))))
    {
	SHPClose(shp);
	return false;
    }

    theme.shape_type = nShapeType;
    theme.polar_coords = false;
    theme.center_lat = 0.;
    theme.center_lon = 0.;
    theme.type_name = strdup(SHPTypeName(nShapeType));
    theme.nshapes = nEntities;
    theme.lon_min = adfMinBound[0];
    theme.lat_min = adfMinBound[1];
    theme.lon_max = adfMaxBound[0];
    theme.lat_max = adfMaxBound[1];
    theme.on_delta = tf->on_delta;
    theme.off_delta = tf->off_delta;
    theme.symbol_type = tf->sym_type;
    theme.symbol_size = tf->sym_size;
    theme.bndry = tf->bndry;
    theme.bndry_fg = tf->bndry_fg;
    theme.fill = tf->fill;

    tf->initThemeColorScale(&theme, 0., (double)nEntities);

    for(i = 0; i < theme.nshapes; i++) {
	theme.shapes[i] = SHPReadObject(shp, i);
    }
    SHPClose(shp);

    if(theme.shape_type == SHPT_POLYGON) {
	for(i = 0; i < theme.nshapes; i++) {
	    theme.shape_fg[i] = tf->fill_fg;
	}
    }
    else {
	for(i = 0; i < theme.nshapes; i++) {
	    theme.shape_fg[i] = theme.bndry_fg;
	}
    }

    int id = mw->map->addTheme(&theme, true);
    if(id >= 0)
    {
	tf->setId(id);
	mw->map->update();
    }
    XFlush(XtDisplay(baseWidget()));

    tf->createShapeTable();

    addThemeRow(tf, new_theme);

    table->adjustColumns();

    return true;
}

char *MapThemes::getThemeName(const char *path)
{
    int i, n;
    char name[MAXPATHLEN+1];

    n = (int)strlen(path);
    for(i = n-1; i >= 0 && path[i] != '/'; i--);

    strcpy(name, path+i+1);
    n = (int)strlen(name);
    if(stringCaseEndsWith(name, ".gz")) {
	for(i = n-1; i >= 0 && name[i] != '.'; i--);
	i--;
	while(i >= 0 && name[i] != '.') i--;
	if(i > 0) name[i] = '\0';
    }
    else {
	for(i = n-1; i >= 0 && name[i] != '.'; i--);
	if(i > 0) name[i] = '\0';
    }
    return strdup(name);
}

void MapThemes::displayShapeTable(void)
{
    int i, num;
    vector<int> rows;

    if(!(num = table->getSelectedRows(rows))) return;

    for(i = 0; i < num; i++) {
	if(rows[i] < theme_files->size()) {
	    MapThemeFile *tf = theme_files->at(rows[i]);
	    tf->createShapeTable();
	    if(tf->info_window) {
		tf->info_window->setVisible(true);
	    }
	}
    }
}

void MapThemes::displayColorTable(void)
{
    int num;
    vector<int> rows;

    if(!(num = table->getSelectedRows(rows))) return;

    for(int i = 0; i < num; i++) {
	if(rows[i] < theme_files->size()) {
	    MapThemeFile *tf = theme_files->at(rows[i]);
	    tf->showColorTable(this);
	}
    }
}

void MapThemes::readNetCDF(const char *path)
{
#ifdef HAVE_NETCDF
    cdfData_t cdfData;
    float *x, *y, *z;
    double min=0., max=0.;
    char *theme_name;
    Boolean new_theme;
    int i, verbose = 1, nlat, nlon;
    MapThemeFile *tf=NULL;
    MapPlotTheme theme;

    cdfData.leftLon = -180.;
    cdfData.rightLon = 180.;
    cdfData.bottomLat = -90.0;
    cdfData.topLat = 90.;

    cdfData.reqType = NC_FLOAT;

    cdfData.xReqPix = 0;
    cdfData.yReqPix = 0;

    getCdfData((char *)path, &cdfData, verbose);

    x = (float *)cdfData.Xvals;
    y = (float *)cdfData.Yvals;
    z = (float *)cdfData.data;
    nlon = cdfData.nXdim;
    nlat = cdfData.nYdim;
    if(nlon <= 0 || nlat <= 0) {
	cerr << "MapReadNetCDF: nlon=" << nlon << " nlat=" << nlat << endl;
	return;
    }
    for(i = 0; i < nlon; i++) x[i] = (float)cdfData.Xvals[i];
    for(i = 0; i < nlat; i++) y[i] = (float)cdfData.Yvals[i];
    if(nlon*nlat > 0) {
	min = z[0];
	max = z[0];
    }
    for(i = 0; i < nlon*nlat; i++) {
	if(z[i] < min) min = z[i];
	if(z[i] > max) max = z[i];
    }

x[0] = -180.;

    theme.shape_type = 0;
    theme.polar_coords = false;
    theme.center_lat = 0.;
    theme.center_lon = 0.;
    theme.type_name = 0;
    theme.nshapes = 0;
    theme.lon_min = x[0];
    theme.lat_min = y[0];
    theme.lon_max = x[nlon-1];
    theme.lat_max = y[nlat-1];
    theme.on_delta = 360.;
    theme.off_delta = 0.;
    theme.symbol_type = -1;
    theme.symbol_size = -1;
    theme.bndry = false;
    theme.bndry_fg = 0;
    theme.fill = true;

    for(i = 0; i < theme_files->size(); i++) {
	tf = theme_files->at(i);
	if(!strcmp(tf->path, path)) break;
    }
    if(i < theme_files->size()) {
	mw->map->mapDelete(tf->id, false);
	new_theme = false;
    }
    else {
	new_theme = true;
	theme_name = getThemeName(path);
	bool first = (!theme_files->size()) ? true : false;
	tf = new MapThemeFile(mw, THEME_IMAGE_FILE, (const char *)theme_name,
			path, 0, 0, first);
	theme_files->add(tf);
	free(theme_name);
    }

    tf->initThemeColorScale(&theme, min, max);

    int id = mw->map->addImage((char *)"", nlon, nlat,
			x, y, z, -1.e+30, COLOR_ONLY, &theme, false);
    tf->setId(id);

    addThemeRow(tf, new_theme);
    mapColorBar(tf, i, true);
    table->adjustColumns();
    freeCdfData(&cdfData);
#else
    showWarning("readNetCDF called, but not compiled with netcdf");
#endif /* HAVE_NETCDF */
}

void MapThemes::readGriddedData(const char *path)
{
#ifdef HAVE_NETCDF
    cdfData_t cdfData;
    float *x, *y, *z;
    double min=0., max=0.;
    char *theme_name;
    Boolean new_theme;
    int i, verbose = 1, nlat, nlon;
    MapThemeFile *tf=NULL;
    MapPlotTheme theme;
    int type;

    if(stringEndsWith(path, ".grd"))
    {
	cdfData.leftLon = -180.;
	cdfData.rightLon = 180.;
	cdfData.bottomLat = -90.0;
	cdfData.topLat = 90.;

	type = ETOPO2;
    }
    else if(stringEndsWith(path, "srtm1"))
    {
	cdfData.leftLon = 23.;
	cdfData.rightLon = 24.;
	cdfData.bottomLat = 35.0;
	cdfData.topLat = 36.;

	type = SRTM1;
    }
    else
    {
	/* for g10g */
	cdfData.leftLon = 0.;
	cdfData.rightLon = 50.;
	cdfData.bottomLat = 0.0;
	cdfData.topLat = 50.;

	type = GLOBE;
    }
    cdfData.reqType = NC_FLOAT;

    cdfData.xReqPix = 0;
    cdfData.yReqPix = 0;

    getRawGriddedData((char *)path, type, &cdfData, verbose);

    x = (float *)cdfData.Xvals;
    y = (float *)cdfData.Yvals;
    z = (float *)cdfData.data;
    nlon = cdfData.nXdim;
    nlat = cdfData.nYdim;
    if(nlon <= 0 || nlat <= 0) {
	cerr << "MapReadNetCDF: nlon=" << nlon << " nlat=" << nlat << endl;
	return;
    }
    for(i = 0; i < nlon; i++) x[i] = (float)cdfData.Xvals[i];
    for(i = 0; i < nlat; i++) y[i] = (float)cdfData.Yvals[i];
    if(nlon*nlat > 0) {
	min = z[0];
	max = z[0];
    }
    for(i = 0; i < nlon*nlat; i++) {
	if(z[i] < min) min = z[i];
	if(z[i] > max) max = z[i];
    }

    theme.shape_type = 0;
    theme.type_name = 0;
    theme.nshapes = 0;
    theme.lon_min = x[0];
    theme.lat_min = y[0];
    theme.lon_max = x[nlon-1];
    theme.lat_max = y[nlat-1];
    theme.on_delta = 360.;
    theme.off_delta = 0.;
    theme.symbol_type = -1;
    theme.symbol_size = -1;
    theme.bndry = false;
    theme.bndry_fg = 0;
    theme.fill = true;

    for(i = 0; i < theme_files->size(); i++) {
	tf = theme_files->at(i);
	if(!strcmp(tf->path, path)) break;
    }
    if(i < theme_files->size()) {
	mw->map->mapDelete(tf->id, false);
	new_theme = false;
    }
    else {
	new_theme = true;
	theme_name = getThemeName(path);
	bool first = (!theme_files->size()) ? true : false;
	tf = new MapThemeFile(mw, THEME_IMAGE_FILE, (const char *)theme_name,
			path, 0, 0, first);
	theme_files->add(tf);
	free(theme_name);
    }

    tf->initThemeColorScale(&theme, min, max);

    int id = mw->map->addImage((char *)"", nlon, nlat,
			x, y, z, -1.e+30, COLOR_ONLY, &theme, true);
    tf->setId(id);

    addThemeRow(tf, new_theme);
    mapColorBar(tf, i, true);
    table->adjustColumns();
#endif
}

void MapThemes::saveThemes(void)
{
    int i;
    char name[1000], prop[100];

    snprintf(prop, sizeof(prop), "%d", theme_files->size());
    putProperty("num_map_themes", prop);

    for(i = 0; i < theme_files->size(); i++)
    {
	MapThemeFile *tf = theme_files->at(i);

	snprintf(name, sizeof(name), "theme%d.path", i+1);
	putProperty(name, tf->path);

	snprintf(name, sizeof(name), "theme.%s.on_delta", tf->theme_name);
	snprintf(prop, sizeof(prop), "%.2lf", tf->on_delta);
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.off_delta", tf->theme_name);
	snprintf(prop, sizeof(prop), "%.2lf", tf->off_delta);
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.bndry", tf->theme_name);
	strcpy(prop, tf->bndry ? "true":"false");
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.fill", tf->theme_name);
	strcpy(prop, tf->fill ? "true":"false");
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.bndry_color",tf->theme_name);
	putProperty(name, tf->bndry_color);

	snprintf(name, sizeof(name), "theme.%s.fill_color", tf->theme_name);
	putProperty(name, tf->fill_color);

	snprintf(name, sizeof(name), "theme.%s.sym_type", tf->theme_name);
	snprintf(prop, sizeof(prop), "%d", tf->sym_type);
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.sym_size", tf->theme_name);
	snprintf(prop, sizeof(prop), "%d", tf->sym_size);
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.display", tf->theme_name);
	strcpy(prop, tf->display ? "true":"false");
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.color_bar", tf->theme_name);
	strcpy(prop, tf->color_bar ? "true":"false");
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.labels", tf->theme_name);
	snprintf(prop, sizeof(prop), "%d", tf->label_column);
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.cursor_column", tf->theme_name);
	snprintf(prop, sizeof(prop), "%d", tf->cursor_column);
	putProperty(name, prop);

	snprintf(name, sizeof(name), "theme.%s.color_column", tf->theme_name);
	snprintf(prop, sizeof(prop), "%d", tf->color_column);
	putProperty(name, prop);
    }
    Application::writeApplicationProperties();
}

void MapThemes::removeThemes(void)
{
    MapThemeFile *tf;
    int i, j, nrows;
    vector<int> rows;

    if(!(nrows = table->getSelectedRows(rows))) return;

    for(i = nrows-1; i >= 0; i--) {
	j = rows[i];
	tf = theme_files->at(j);

	mw->map->mapDelete(tf->id, false);
	theme_files->removeAt(j);
	table->removeRow(j);
    }

    mw->map->update();
}

void MapThemes::readPolar(const char *path)
{
#ifdef _ZZZ
    FILE *fp;
    int i;
    Boolean new_theme;
    char *theme_name;
    MapThemeFile *tf;
    MapPlotTheme theme;

    if((fp = fopen(path, "r")) == NULL)
    {
	if(errno > 0) {
	    showWarning("Cannot open: %s\n%s", path, strerror(errno));
	}
	else {
	    showWarning("Cannot open: %s", path);
	}
	return;
    }

    readPolarFile(fp, &theme);

    for(i = 0; i < theme_files->size(); i++) {
	tf = theme_files->at(i);
	if(!strcmp(tf->path, path)) break;
    }
    if(i < theme_files->size()) {
	mw->map->mapDelete(tf->id, false);
	new_theme = false;
    }
    else {
	new_theme = true;
	theme_name = getThemeName(path);
	bool first = (!theme_files->size()) ? true : false;
	tf = new MapThemeFile(mw, THEME_SHAPE_FILE, (const char *)theme_name,
			path, SHPT_POLYGON, theme.nshapes, first);
	theme_files->add(tf);
	free(theme_name);
    }

//    XmToggleButtonSetState(widgetId("*_Map*azEquidist"), true, true);
//    MapPlotRotate(mw->map, 0., 90., false);

//    MapInitThemeFile(tf, THEME_POLAR_FILE, theme_name, path, theme.nshapes,

    theme.shape_type = SHPT_POLYGON;
    theme.polar_coords = true;
    theme.center_lon = 50.;
    theme.center_lat = 30.;
    theme.type_name = strdup("polar");
    theme.on_delta = 360.;
    theme.off_delta = 0.;
    theme.symbol_type = tf->sym_type;
    theme.symbol_size = tf->sym_size;
    theme.bndry = tf->bndry;
    theme.bndry_fg = tf->bndry_fg;
    theme.fill = tf->fill;

    if(!(theme.shape_fg = (Pixel *)mallocWarn(theme.nshapes*sizeof(Pixel))))
    {
	return;
    }
    if(theme.shape_type == SHPT_POLYGON) {
	for(i = 0; i < theme.nshapes; i++) {
	    theme.shape_fg[i] = tf->fill_fg;
	}
    }
    else {
	for(i = 0; i < theme.nshapes; i++) {
	    theme.shape_fg[i] = theme.bndry_fg;
	}
    }

    tf->initThemeColorScale(&theme, 0, theme.nshapes);

    int id = mw->map->addTheme(&theme, true);
    tf->setId(id);

//    MapCreateShapeTable(widget, tf);

    addThemeRow(tf, new_theme);
//    MapColorBar(widget, tf, i, true);
    table->adjustColumns();
#endif

{
MapPlotSymbol s[10], symbol_init = MAP_PLOT_SYMBOL_INIT;

s[0] = symbol_init;
s[0].polar_coords = true;
s[0].sym.display = MAP_ON;
s[0].sym.type = MapOverlay::getSymType("PLUS");
s[0].sym.fg = stringToPixel("red");
s[0].lon = 100.;
s[0].lat = 35.;
s[0].sym.size = 4;
s[1] = symbol_init;
s[1].polar_coords = true;
s[1].sym.display = MAP_ON;
s[1].sym.type = MapOverlay::getSymType("TRIANGLE");
s[1].sym.fg = stringToPixel("black");
s[1].lon = 500.;
s[1].lat = 60.;
s[1].sym.size = 5;
s[2] = symbol_init;
s[2].polar_coords = true;
s[2].sym.display = MAP_ON;
s[2].sym.type = MapOverlay::getSymType("SQUARE");
s[2].sym.fg = stringToPixel("brown");
s[2].lon = 1500.;
s[2].lat = -45.;
s[2].sym.size = 4;
s[3] = symbol_init;
s[3].polar_coords = true;
s[3].sym.display = MAP_ON;
s[3].sym.type = MapOverlay::getSymType("CIRCLE");
s[3].sym.fg = stringToPixel("tan");
s[3].lon = 300.;
s[3].lat = 80.;
s[3].sym.size = 6;
s[4] = symbol_init;
s[4].polar_coords = true;
s[4].sym.display = MAP_ON;
s[4].sym.type = MapOverlay::getSymType("PLUS");
s[4].sym.fg = stringToPixel("forest green");
s[4].lon = 300.;
s[4].lat = -10.;
s[4].sym.size = 4;
s[5] = symbol_init;
s[5].polar_coords = true;
s[5].sym.display = MAP_ON;
s[5].sym.type = MapOverlay::getSymType("FILLED_TRIANGLE");
s[5].sym.fg = stringToPixel("orange");
s[5].lon = 300.;
s[5].lon = 1000.;
s[5].lat = 45.;
s[5].sym.size = 6;
s[6] = symbol_init;
s[6].polar_coords = true;
s[6].sym.display = MAP_ON;
s[6].sym.type = MapOverlay::getSymType("FILLED_SQUARE");
s[6].sym.fg = stringToPixel("cyan");
s[6].lon = 300.;
s[6].lon = 750;
s[6].lat = 70.;
s[6].sym.size = 7;
s[7] = symbol_init;
s[7].polar_coords = true;
s[7].sym.display = MAP_ON;
s[7].sym.type = MapOverlay::getSymType("PLUS");
s[7].sym.fg = stringToPixel("yellow");
s[7].lon = 300.;
s[7].lon = 2000;
s[7].lat = 10.;
s[7].sym.size = 4;
s[8] = symbol_init;
s[8].polar_coords = true;
s[8].sym.display = MAP_ON;
s[8].sym.type = MapOverlay::getSymType("PLUS");
s[8].sym.fg = stringToPixel("purple");
s[8].lon = 300.;
s[8].lon = 1325;
s[8].lat = -75;
s[9] = symbol_init;
s[8].sym.size = 4;
s[9].polar_coords = true;
s[9].sym.display = MAP_ON;
s[9].sym.type = MapOverlay::getSymType("PLUS");
s[9].sym.fg = stringToPixel("red");
s[9].lon = 300.;
s[9].lon = 1700;
s[9].lat = 115;
s[9].sym.size = 4;
mw->map->addSymbols(10, s, true);
}
}

void MapThemes::readPolarFile(FILE *fp, MapPlotTheme *theme)
{
    char cmd[200], buf[200];
    int m, n = 0, line_no, shape_size=50;
    double a, r, theta, x, y, xmin=0., ymin=0., xmax=0., ymax=0.;
    double r0=0., theta0=0.;
    Boolean	warned_once = false;
    SHPObject *s;

    theme->nshapes = 0;
    if(!(theme->shapes = (SHPObject **)
			mallocWarn(shape_size*sizeof(SHPObject *)))) return;
    theme->lon_min  = 1.e+30;
    theme->lon_max = -1.e+30;
    theme->lat_min =  1.e+30;
    theme->lat_max = -1.e+30;

    n = MapOverlay::getLine(fp, buf, 200, &line_no);

    while(n != EOF)
    {
	cmd[0] = '\0';
	sscanf(buf, "%50s", cmd);

	if(!strcmp(cmd, "line") || !strcmp(cmd, "polygon"))
	{
	    int pos = ftell(fp);
	    int line = line_no, nParts, nVertices, num;

	    nParts = 1;
	    nVertices = 0;

	    while((n = MapOverlay::getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(!strncasecmp(buf, "new part", 8)) {
		    nParts++;
		    continue;
		}
		if(sscanf(buf, "%lf %lf", &r,&theta) != 2 ||
			MapOverlay::isCommand(buf)) break;
		if(!nVertices) {
		    r0 = r;
		    theta0 = theta;
		}
		nVertices++;
	    }
	    if(nVertices <= 1) continue;

	    num = nVertices;
	    if(!strcmp(cmd, "polygon") && (r != r0 || theta != theta0)) {
		num = nVertices + 1;
	    }

	    if(!(s = (SHPObject *)mallocWarn(sizeof(SHPObject)))) return;

	    if(!strcmp(cmd, "line")) {
		s->nSHPType = SHPT_ARC;
	    }
	    else {
		s->nSHPType = SHPT_POLYGON;
	    }
	    s->nShapeId = -1;
	    s->nVertices = num;
	    s->nParts = nParts;
	    s->padfX = (double *)mallocWarn(num*sizeof(double));
	    s->padfY = (double *)mallocWarn(num*sizeof(double));
	    s->panPartStart = (int *)mallocWarn(nParts*sizeof(int));
	    s->panPartStart[0] = 0;
	    s->panPartType = NULL;

	    fseek(fp, pos, 0);
	    line_no = line;
	    m = 0;
	    nParts = 1;
	    while((n = MapOverlay::getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(!strncasecmp(buf, "new part", 8)) {
		    nParts++;
		    if(nParts > s->nParts) break;
		    s->panPartStart[nParts-1] = m;
		    continue;
		}
		if(sscanf(buf, "%lf %lf", &r, &theta) != 2 ||
			MapOverlay::isCommand(buf)) {
		    break;
		}
		a = theta*DEG_TO_RADIANS;
		x = r*sin(a);
		y = r*cos(a);
		if(!m) {
		    xmin = xmax = x;
		    ymin = ymax = y;
		}
		else {
		    if(x < xmin) xmin = x;
		    else if(x > xmax) xmax = x;
		    if(y < ymin) ymin = y;
		    else if(y > ymax) ymax = y;
		}
		m++;
		if(m > s->nVertices) break;
		s->padfX[m-1] = r;
		s->padfY[m-1] = theta;
	    }
	    if(m < s->nVertices) {
		s->padfX[m] = s->padfX[0];
		s->padfY[m] = s->padfY[0];
	    }
	    s->dfXMin = xmin;
	    s->dfXMax = xmax;
	    s->dfYMin = ymin;
	    s->dfYMax = ymax;

	    if(theme->lon_min > xmin) theme->lon_min = xmin;
	    if(theme->lon_max < xmax) theme->lon_max = xmax;
	    if(theme->lat_min > ymin) theme->lat_min = ymin;
	    if(theme->lat_max < ymax) theme->lat_max = ymax;

	    s->dfZMin = 0;
	    s->dfZMax = 0;
	    s->dfMMin = 0;
	    s->dfMMax = 0;
	    if(theme->nshapes >= shape_size) {
		shape_size += 50;
		if(!(theme->shapes = (SHPObject **)reallocWarn(theme->shapes,
			shape_size*sizeof(SHPObject *)))) return;
	    }
	    theme->shapes[theme->nshapes++] = s;
	}
/*
	else if(!strcmp(cmd, "symbols") || !strcmp(cmd, "points"))
	{
	    symbol = symbol_init;
	    symbol.sym.display = display;
	    symbol.label = NULL;
	    if((str = stringGetArg(buf, "label")) != NULL)
	    {
		stringcpy(label, str, sizeof(label));
		symbol.label = label;
		free(str);
	    }
	    if((str = stringGetArg(buf, "symbol")) != NULL)
	    {
		symbol.sym.type = MapOverlay::getSymType(str);
		free(str);
	    }
	    symbol.sym.fg = fg;
	    if((str = stringGetArg(buf, "color")) != NULL)
	    {
		symbol.sym.fg = stringToPixel(str);
		free(str);
	    }

	    symbol.npts = m = 0;
	    symbol.lat = (double *)mallocWarn(sizeof(double));
	    symbol.lon = (double *)mallocWarn(sizeof(double));
	    symbol.size = (int *)mallocWarn(sizeof(int));

	    while((n = MapOverlay::getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(sscanf(buf, "%lf %lf %d", &lat, &lon, &siz) != 3 ||
			MapOverlay::isCommand(buf)) break;
		symbol.lat = (double *)reallocWarn(symbol.lat,
						(m+1)*sizeof(double));
		symbol.lon = (double *)reallocWarn(symbol.lon,
						(m+1)*sizeof(double));
		symbol.size = (int *)reallocWarn(symbol.size,
						(m+1)*sizeof(int));
		symbol.lat[m] = lat;
		symbol.lon[m] = lon;
		symbol.size[m] = siz;
		m++;
	    }
	    if(m > 0)
	    {
		symbol.npts = m;
		o->id = (int *)reallocWarn(o->id,
					(o->num_ids+1)*sizeof(int));
		o->id[o->num_ids++] = MapPlotAddSymbols(mw->map, &symbol,false);
	    }
	    free(symbol.lat);
	    free(symbol.lon);
	    free(symbol.size);
	}
*/
	else
	{
	    if(!warned_once)
	    {
		warned_once = true;
		showWarning("format error line %d", line_no);
	    }
	    n = MapOverlay::getLine(fp, buf, 200, &line_no);
	}
    }
}
