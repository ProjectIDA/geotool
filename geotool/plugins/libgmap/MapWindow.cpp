/** \file MapWindow.cpp
 *  \brief Defines class MapWindow.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

#include "MapWindow.h"
#include "MapThemes.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "MapOverlayForm.h"
#include "MapOverlay.h"
#include "MapCursor.h"
#include "MapMeasureForm.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

typedef struct
{
	char	*map_overlay_dir;
	Boolean	backing_store;
	char	*source_symbol;
	int	source_size;
	Pixel	distance_color;
} SubData, *SubDataPtr;

#define XtNmapOverlayDir	"mapOverlayDir"
#define XtNmapBackingStore	"mapBackingStore"
#define XtNmapSourceSymbol	"mapSourceSymbol"
#define XtNmapSourceSize	"mapSourceSize"
#define XtNdistanceColor	"distanceColor"

#define XtCMapOverlayDir	"MapOverlayDir"
#define XtCMapBackingStore	"MapBackingStore"
#define XtCMapSourceSymbol	"MapSourceSymbol"
#define XtCMapSourceSize	"MapSourceSize"
#define XtCDistanceColor	"DistanceColor"


#define offset(field)	XtOffset(SubDataPtr, field)
static XtResource resources[] =
{
    {(char *)XtNmapOverlayDir, (char *)XtCMapOverlayDir, XtRString,
	sizeof(String), offset(map_overlay_dir), XtRString, (XtPointer)NULL},
    {(char *)XtNmapBackingStore, (char *)XtCMapBackingStore, XtRBoolean,
	sizeof(Boolean), offset(backing_store), XtRString, (XtPointer)"False"},
    {(char *)XtNmapSourceSymbol, (char *)XtCMapSourceSymbol, XtRString,
	sizeof(String), offset(source_symbol), XtRString, (XtPointer)"PLUS"},
    {(char *)XtNmapSourceSize, (char *)XtCMapSourceSize, XtRInt, sizeof(int),
	offset(source_size), XtRImmediate, (XtPointer)6},
    {(char *)XtNdistanceColor, (char *)XtCDistanceColor, XtRPixel,sizeof(Pixel),
	offset(distance_color), XtRString, (XtPointer)"maroon"},
};
#undef offset

static SubData sub_data;

static bool get_resources = true;

//int map_source_symbol = PLUS;
//int map_source_size = 6;

using namespace libgmap;

MapWindow::MapWindow(const string &name, Component *parent, DataSource *ds,
		bool independent, const string &title, bool utm_flag) :
		BasicMap(name, parent, title, independent), DataReceiver(ds)
{
    createInterface(utm_flag);
    init(utm_flag);
}

namespace libgmap {

class UTMWindow : public MapWindow
{
    public :
	UTMWindow(const char *name, Component *parent) :
			MapWindow(name, parent, NULL, true, "", true)
	{
	    setSize(500, 700);
	}
};

} // namespace libgmap

void MapWindow::init(bool utm_flag)
{
    init_colors = true;
    color_scale.num_colors = 0;
    color_scale.pixels = NULL;
    color_scale.lines = NULL;
    map_id = -1;
    utm = utm_flag;
    print_window = NULL;
    labels_window = NULL;
    map_cursor_window = NULL;
    map_measure_window = NULL;
    ignore_data_change = false;

    overlay_form = new MapOverlayForm("Map Overlays", this, this);
    themes = new MapThemes("Map Themes", this, this);
    if( !utm ) {
	themes->readInitialThemes();
    }

    if(data_source) {
	data_source->addDataListener(this);
    }
    map->addActionListener(this, XtNcrosshairCallback);
    map->addActionListener(this, XtNcrosshairDragCallback);
    map->addActionListener(this, XtNselectSourceCallback);
    ignore_crosshair = false;

    mapUpdate();
}

void MapWindow::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) data_source->removeDataListener(this);
	data_source = ds;
	if(data_source) data_source->addDataListener(this);
    }
}

void MapWindow::createInterface(bool utm_flag)
{
    int n;
    Arg args[20];

    setSize(975, 555);

    num_color_map = 0;
    color_map = NULL;
    overlay_form = NULL;
    fileDialog = NULL;
    utm_window = NULL;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    Component *form = ( utm_flag ) ? frame_form : workForm();
    // make coordinate text fields
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    azimuth_text = new TextField("azimuth_text", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, azimuth_text->baseWidget()); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    azimuth_label = new Label("azimuth", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, azimuth_label->baseWidget()); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    delta_text = new TextField("delta_text", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, delta_text->baseWidget()); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    if( !utm_flag ) {
	delta_label = new Label("delta", form, args, n);
    }
    else {
	delta_label = new Label("meters", form, args, n);
    }

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, delta_label->baseWidget()); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    latitude_text = new TextField("lat_text", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, latitude_text->baseWidget()); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    latitude_label = new Label("latitude", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, latitude_label->baseWidget()); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    longitude_text = new TextField("lon_text", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, longitude_text->baseWidget()); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    longitude_label = new Label("longitude", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, longitude_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XmString xm = createXmString("+");
    XtSetArg(args[n], XmNtitleString, xm); n++;
    crosshair_toggle = new Toggle("+", form, this, args, n);
    XmStringFree(xm);

    if( !utm_flag ) {
	// change InfoArea attachments
	n = 0;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNrightWidget, crosshair_toggle->baseWidget()); n++;
	info_area->setValues(args, n);
    }

    file_menu = new Menu("File", menu_bar);
    if( !utm_flag ) {
	open_button = new Button("Open...", file_menu, this);
	overlays_button = new Button("Overlays...", file_menu, this);
    }
    print_button = new Button("Print...", file_menu, this);
    file_menu->addSeparator("separator");
    close_button = new Button("Close", file_menu, this);

    if( !utm_flag ) {
	edit_menu = new Menu("Edit", menu_bar);
	clear_dist_button = new Button("Clear Distances", edit_menu, this);
	clear_fkaz_button = new Button("Clear FK Azimuths", edit_menu, this);
	clear_rotaz_button = new Button("Clear Rotation Azimuths", edit_menu,
					this);
    }

    view_menu = new Menu("View", menu_bar);

    if( !utm_flag ) {
	grid_toggle = new Toggle("Grid", view_menu, this);
	limits_button = new Button("Limits...", view_menu, this);
	labels_button = new Button("Labels...", view_menu, this);
    }

    subMenuChoice("Stations", &stations_menu, stations);

    station_tags = new Toggle("Station Tags", view_menu, this);

    subMenuChoice("Origins", &origins_menu, origin_toggle);

    origin_tags = new Toggle("Origin Tags", view_menu, this);

    subMenuChoice("Paths", &paths_menu, paths);
    subMenuChoice("Ellipses", &ellipses_menu, ellipses);
    subMenuChoice("Rotation Azimuths", &rotaz_menu, rotaz);
    subMenuChoice("FK Azimuths", &fkaz_menu, fkaz);
    subMenuChoice("Distances", &dist_menu, dist);

    option_menu = new Menu("Option", menu_bar);
    if( !utm_flag ) {
	projection_menu = new Menu("Projection", option_menu, true);
	lc_toggle = new Toggle("Linear Cylindrical", projection_menu, this);
	lc_toggle->set(true, false);
	cea_toggle = new Toggle("Cylindrical Equal-Area", projection_menu,this);
	mercator_toggle = new Toggle("Mercator", projection_menu, this);
	ortho_toggle = new Toggle("Orthographic", projection_menu, this);
	ae_toggle = new Toggle("Azimuthal Equidistant", projection_menu, this);
	aea_toggle = new Toggle("Azimuthal Equal-Area", projection_menu, this);
	utm_toggle = new Toggle("Universal Transverse Mercator",
			projection_menu, this);
    }

    map_cursor_button = new Button("Map Cursor...", option_menu, this);
    measure_button = new Button("Measure...", option_menu, this);
    if( !utm_flag ) {
	rotate_to_crosshair =new Button("Rotate to Crosshair",option_menu,this);
	rotate_to_station = new Button("Rotate to Station", option_menu, this);
	rotate_to_origin = new Button("Rotate to Origin", option_menu, this);
	themes_button = new Button("Themes...", option_menu, this);
    }

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Map Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    if( !utm_flag ) {
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    }
    else {
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomOffset, 2); n++;
	XtSetArg(args[n], XmNbottomWidget, crosshair_toggle->baseWidget()); n++;
    }
    XtSetArg(args[n], XtNuniformScale, True); n++;
    XtSetArg(args[n], XtNtickmarksInside, False); n++;
    XtSetArg(args[n], XtNclearOnScroll, False); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;

    map = new Map("map", frame_form, args, n);
    map->setVisible(true);

    addPlugins("MapWindow", NULL, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(print_button, "Print");
	if( !utm_flag ) {
	    tool_bar->add(overlays_button, "Overlays");
	    tool_bar->add(themes_button, "Themes");
	}
    }
}

void MapWindow::subMenuChoice(const char *name, Menu **menu, Toggle **t)
{
    char cmd[20];

    *menu = new Menu(name, view_menu, true);
    t[0] = new Toggle("All", *menu, this);
    t[0]->set(true);
    snprintf(cmd, sizeof(cmd), "%s All", name);
    t[0]->setCommandString(cmd);

    t[1] = new Toggle("Selected", *menu, this);
    snprintf(cmd, sizeof(cmd), "%s Selected", name);
    t[1]->setCommandString(cmd);

    t[2] = new Toggle("None", *menu, this);
    snprintf(cmd, sizeof(cmd), "%s None", name);
    t[2]->setCommandString(cmd);
}

MapWindow::~MapWindow(void)
{
    if(color_scale.lines) free(color_scale.lines);
    if(color_scale.pixels) free(color_scale.pixels);
    if(color_map) free(color_map);
}

void MapWindow::setColorMap(int num, Pixel *pixels)
{
    if(color_map) free(color_map);
    num_color_map = 0;
    color_map = NULL;
    if(num > 0) {
	color_map = (Pixel *)mallocWarn(num*sizeof(Pixel));
	for(int i = 0; i < num; i++) color_map[i] = pixels[i];
	num_color_map = num;
    }
}

Pixel MapWindow::getColor(const char *name)
{
    int index;

    if(num_color_map > 0 && stringToInt(name, &index)) {
	return color_map[index % num_color_map];
    }
    return stringToPixel((char *)name);
}

void MapWindow::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();
    Toggle *t = comp->getToggleInstance();
    bool on;

    if(!strcmp(cmd, "Open...")) {
	open();
    }
    else if(!strcmp(cmd, "Overlays...")) {
	overlayWindow();
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Linear Cylindrical")) {
	if(t && t->state()) {
	    map->setProjection(MAP_LINEAR_CYLINDRICAL);
	}
    }
    else if(!strcmp(cmd, "Cylindrical Equal-Area")) {
	if(t && t->state()) {
	    map->setProjection(MAP_CYLINDRICAL_EQUAL_AREA);
	}
    }
    else if(!strcmp(cmd, "Mercator")) {
	if(t && t->state()) {
	    map->setProjection(MAP_MERCATOR);
	}
    }
    else if(!strcmp(cmd, "Orthographic")) {
	if(t && t->state()) {
	    map->setProjection(MAP_ORTHOGRAPHIC);
	}
    }
    else if(!strcmp(cmd, "Azimuthal Equidistant")) {
	if(t && t->state()) {
	    map->setProjection(MAP_AZIMUTHAL_EQUIDISTANT);
	}
    }
    else if(!strcmp(cmd, "Azimuthal Equal-Area")) {
	if(t && t->state()) {
	    map->setProjection(MAP_AZIMUTHAL_EQUAL_AREA);
	}
    }
    else if(!strcmp(cmd, "Universal Transverse Mercator")) {
	if(t && t->state()) {
	    if( !utm_window ) {
		utm_window = new UTMWindow("UTM Map", this);
		utm_window->manage();
		map->addActionListener(this, XtNutmCallback);
		map->linkMap(utm_window->map);
		themes->addCursorCallbacks(utm_window->map);
	    }
	    map->setProjection(MAP_UTM);
	}
    }
    else if(!strcmp(reason, XtNutmCallback) ) {
	UtmStruct *utm_struct = (UtmStruct *)action_event->getCalldata();
	utm_window->map->displayUTM(utm_struct->letter, utm_struct->zone);
	utm_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Map Cursor...")) {
	if(!map_cursor_window) {
	    map_cursor_window = new MapCursor("Map Cursor", this, this,
				data_source);
	}
	map_cursor_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Measure...")) {
	if(!map_measure_window) {
	    map_measure_window = new MapMeasureForm("Map Measure", this, map);
	}
	map_measure_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Origin Tags")) {
	if(t) {
	    on = t->state();
	    map->displaySourceTags(on);
	}
    }
    else if(!strcmp(cmd, "Station Tags")) {
	if(t) {
	    on = t->state();
	    map->displayStationTags(on);
	}
    }
    else if(!strcmp(cmd, "Grid")) {
	if(t) {
	    on = t->state();
	    map->displayGrid(on);
	}
    }
    else if(!strcmp(cmd, "Stations All")) {
	if(t && t->state()) {
	    map->displayStations(NULL, MAP_ON);
	}
    }
    else if(!strcmp(cmd, "Stations Selected")) {
	if(t && t->state()) {
	    map->displayStations(NULL, MAP_SELECTED_ON);
	}
    }
    else if(!strcmp(cmd, "Stations None")) {
	if(t && t->state()) {
	    map->displayStations(NULL, MAP_OFF);
	}
    }
    else if(!strcmp(cmd, "Origins All")) {
	if(t && t->state()) {
	    map->displaySources(NULL, MAP_ON);
	}
    }
    else if(!strcmp(cmd, "Origins Selected")) {
	if(t && t->state()) {
	    map->displaySources(NULL, MAP_SELECTED_ON);
	}
    }
    else if(!strcmp(cmd, "Origins None")) {
	if(t && t->state()) {
	    map->displaySources(NULL, MAP_OFF);
	}
    }
    else if(!strcmp(cmd, "Paths All")) {
	if(t && t->state()) {
	    map->displayPaths(MAP_ON);
	}
    }
    else if(!strcmp(cmd, "Paths Selected")) {
	if(t && t->state()) {
	    map->displayPaths(MAP_SELECTED_ON);
	}
    }
    else if(!strcmp(cmd, "Paths None")) {
	if(t && t->state()) {
	    map->displayPaths(MAP_OFF);
	}
    }
    else if(!strcmp(cmd, "Ellipses All")) {
	if(t && t->state()) {
	    map->displayEllipses(NULL, MAP_ON);
	}
    }
    else if(!strcmp(cmd, "Ellipses Selected")) {
	if(t && t->state()) {
	    map->displayEllipses(NULL, MAP_SELECTED_ON);
	}
    }
    else if(!strcmp(cmd, "Ellipses None")) {
	if(t && t->state()) {
	    map->displayEllipses(NULL, MAP_OFF);
	}
    }
    else if(!strcmp(cmd, "Distances All")) {
	if(t && t->state()) {
	    map->displayDeltas(NULL, MAP_ON);
	}
    }
    else if(!strcmp(cmd, "Distances Selected")) {
	if(t && t->state()) {
	    map->displayDeltas(NULL, MAP_SELECTED_ON);
	}
    }
    else if(!strcmp(cmd, "Distances None")) {
	if(t && t->state()) {
	    map->displayDeltas(NULL, MAP_OFF);
	}
    }
    else if(!strcmp(cmd, "Themes...")) {
	if(!themes) {
	    themes = new MapThemes("Map Themes", this, this);
	    themes->readInitialThemes();
	}
	themes->setVisible(true);
    }
    else if(!strcmp(cmd, "Labels...")) {
	if(!labels_window) {
	    labels_window = new AxesLabels("Axes Labels", this, map);
	}
	labels_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Rotate to Crosshair")) {
	map->rotation();
    }
    else if(!strcmp(cmd, "Rotate to Station")) {
	map->rotateToSta();
    }
    else if(!strcmp(cmd, "Rotate to Origin")) {
	map->rotateToOrigin();
    }
    else if(!strcmp(reason, XtNdataChangeCallback)) {
	if(!ignore_data_change) {
	    if(onlySelect((DataChange *)action_event->getCalldata())) {
		mapUpdateSelected();
	    }
	    else {
		mapUpdate();
		mapUpdateSelected();
	    }
	}
    }
    else if(!strcmp(reason, XtNcrosshairCallback) ||
	    !strcmp(reason, XtNcrosshairDragCallback))
    {
	if(!ignore_crosshair) crosshairCB();
    }
    else if(comp == latitude_text) {
	double lat;
	if(latitude_text->getDouble(&lat)) {
	    positionCrosshair();
        }
    }
    else if(comp == longitude_text) {
	double lon;
	if(longitude_text->getDouble(&lon)) {
	    positionCrosshair();
	}
    }
    else if(comp == delta_text) {
	double delta;
	if(delta_text->getDouble(&delta)) {
	    positionCrosshair();
	}
    }
    else if(comp == azimuth_text) {
	double azimuth;
	if(azimuth_text->getDouble(&azimuth)) {
	    positionCrosshair();
	}
    }
    else if(comp == crosshair_toggle) {
	if(crosshair_toggle->state()) {
/*
	    double xmin, xmax, ymin, ymax;
	    map->getLimits(&xmin, &xmax, &ymin, &ymax);
	    double x = xmin + .3*(xmax - xmin);
	    double y = ymin + .3*(ymax - ymin);
	    map->positionCrosshair(x, y, false);
	    crosshairCB();
*/
	    if(!map->hasCrosshair()) {
		map->addCrosshair();
		crosshairCB();
	    }
	}
	else {
	    map->deleteCrosshair();
	}
    }
    else if(!strcmp(reason, XtNselectSourceCallback)) {
	if(tv) {
	     selectTableRows();
	}
    }
    else if(!strcmp(cmd, "Clear Distances")) {
	map->clearDeltas("distance", true);
    }
    else if(!strcmp(cmd, "Clear FK Azimuths")) {
	map->clearArcs("fk", true);
    }
    else if(!strcmp(cmd, "Clear Rotation Azimuths")) {
	map->clearArcs("rotate", true);
    }
    else if(!strcmp(cmd, "Map Help")) {
	showHelp("Map Help");
    }
}

ParseCmd MapWindow::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if(parseArg(cmd, "set_limits", c) || parseArg(cmd, "limits", c) )
    {
	double lonmin, lonmax, latmin, latmax;
	if( !parseGetArg(c, "limits", msg, "lonmin", &lonmin) ) {
	    msg.assign("set limits: missing lonmin");
	    return ARGUMENT_ERROR;
	}
	if( !parseGetArg(c, "limits", msg, "lonmax", &lonmax) ) {
	    msg.assign("set limits: missing lonmax");
	    return ARGUMENT_ERROR;
	}
	if( !parseGetArg(c, "limits", msg, "latmin", &latmin) ) {
	    msg.assign("set limits: missing latmin");
	    return ARGUMENT_ERROR;
	}
	if( !parseGetArg(c, "limits", msg, "latmax", &latmax) ) {
	    msg.assign("set limits: missing latmax");
	    return ARGUMENT_ERROR;
	}
	map->setLimits(lonmin, lonmax, latmin, latmax);
    }
    else if(parseCompare(cmd, "Rotate_to_Crosshair")) {
	map->rotation();
    }
    else if(parseCompare(cmd, "Rotate_to_Station")) {
	map->rotateToSta();
    }
    else if(parseCompare(cmd, "Rotate_to_Origin")) {
	map->rotateToOrigin();
    }
    else if(parseArg(cmd, "Rotate", c)) {
	double lon, lat;
	string sta;
	if( parseGetArg(c, "Rotate", msg, "lon", &lon)
		&& parseGetArg(c, "Rotate", msg, "lat", &lat) )
	{
	    map->rotate(lon, lat, true);
	}
	else if( parseString(c, "sta", sta) ) {
	    map->rotateToStation((char *)sta.c_str());
	}
    }
    else if(parseArg(cmd, "print_window", c)) {
	print();
	return print_window->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "open_file", c)) {
	readFile(c.c_str());
    }
    else if(parseArg(cmd, "overlays", c)) {
	return overlay_form->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Clear_Distances")) {
	map->clearDeltas("distance", true);
    }
    else if(parseCompare(cmd, "Clear_FK_Azimuths")) {
	map->clearArcs("fk", true);
    }
    else if(parseCompare(cmd, "Clear_Rotation_Azimuths")) {
	map->clearArcs("rotate", true);
    }
    else if(parseArg(cmd, "Origin_Tags", c)) {
	return origin_tags->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Station_Tags", c)) {
	return station_tags->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Grid", c)) {
	return grid_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Stations", c)) {
	return stations_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Origins", c)) {
	return origins_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Paths", c)) {
	return paths_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Ellipses", c)) {
	return ellipses_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Distances", c)) {
	return dist_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "FK_Azimuths", c)) {
	return fkaz_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Rotation_Azimuths", c)) {
	return rotaz_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Projection", c)) {
        if(parseCompare(c, "Linear_Cylindrical") || parseCompare(c, "LC")) {
	    lc_toggle->set(true, true);
	}
	else if(parseCompare(c, "Cylindrical_Equal-Area") ||
		parseCompare(c, "CEA")) {
	    cea_toggle->set(true, true);
	}
	else if(parseCompare(c, "Mercator") || parseCompare(c, "MER")) {
	    mercator_toggle->set(true, true);
	}
	else if(parseCompare(c, "Orthographic") || parseCompare(c, "ORTHO")) {
	    ortho_toggle->set(true, true);
	}
	else if(parseCompare(c, "Azimuthal_Equidistant") ||
		parseCompare(c, "AE")) {
	    ae_toggle->set(true, true);
	}
	else if(parseCompare(c, "Azimuthal_Equal-Area") ||
		parseCompare(c, "AEA")) {
	    aea_toggle->set(true, true);
	}
	else if(parseCompare(c, "Universal_Transverse_Mercator", 29) ||
		parseCompare(c, "UTM", 3))
	{
	    return parseUTM(c, msg);
	}
	else {
	    msg.assign("Projection: \
Linear_Cylindrical or LC,\n\
Cylindrical_Equal-Area or CEA,\n\
Mercator or M,\n\
Orthographic or O,\n\
Azimuthal_Equidistant or AE,\n\
Azimuthal_Equal-Area or AEA,\n\
UTM cell-id");
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "Themes", c)) {
	if(!themes) {
	    themes = new MapThemes("Map Themes", this, this);
	    themes->readInitialThemes();
	}
	return themes->parseCmd(c, msg);
    }
    else if((ret = pluginParse(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return BasicMap::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd MapWindow::parseUTM(const string &cmd, string &msg)
{
    if( !utm_window ) {
	utm_window = new UTMWindow("UTM Map", this);
	utm_window->manage();
	map->addActionListener(this, XtNutmCallback);
	map->linkMap(utm_window->map);
	themes->addCursorCallbacks(utm_window->map);
    }
    map->setProjection(MAP_UTM);

    int i=0, zone;
    char number[10] = "\0";
    char letter = '\0';
    const char *c = cmd.c_str();

    if(!strncasecmp(c, "UTM", 3)) c += 3;
    else c += 29;

    while(*c != '\0' && isspace((int)*c)) c++;
    if(*c >= 'a' && *c <= 'z') {
	letter = 'A' + *c - 'a';
	c++;
	while(*c != '\0' && isspace((int)*c)) c++;
	while(*c >= '0' && *c <= '9' && i < 2) number[i++] = *c++;
	number[i] = '\0';
    }
    else if(*c >= 'A' && *c <= 'Z') {
	letter = *c;
	c++;
	while(*c != '\0' && isspace((int)*c)) c++;
	while(*c >= '0' && *c <= '9' && i < 2) number[i++] = *c++;
	number[i] = '\0';
    }
    else {
	while(*c >= '0' && *c <= '9' && i < 2) number[i++] = *c++;
	number[i] = '\0';

	while(*c != '\0' && isspace((int)*c)) c++;
	if(*c >= 'a' && *c <= 'z') {
	    letter = 'A' + *c - 'a';
	}
	else if(*c >= 'A' && *c <= 'Z') {
	    letter = *c;
	}
    }
    if(letter == '\0' || number[0] == '\0') {
	msg.assign("Projection: UTM cell-id");
	return ARGUMENT_ERROR;
    }
    zone = atoi(number);

    if( !utm_window->map->displayUTM(letter, zone) ) {
	msg.assign("Projection: UTM P23");
	return ARGUMENT_ERROR;
    }
    utm_window->setVisible(true);
    return COMMAND_PARSED;
}

void MapWindow::parseHelp(const char *prefix)
{
    printf("%sclear_distances\n", prefix);
    printf("%sclear_fk_azimuths\n", prefix);
    printf("%sclear_rotation_azimuths\n", prefix);

    printf("%sopen_file=FILENAME\n", prefix);
    printf("%sorigin_tags=(true,false)\n", prefix);

    printf("%sset_limits lonmin=LON lonmax=LON latmin=LAT latmax=LAT\n",prefix);
    printf("%srotate lon=LON lat=LAT\n", prefix);
    printf("%srotate sta=STA\n", prefix);
    printf("%sstation_tags=(true,false)\n", prefix);
    printf("%sgrid=(true,false)\n", prefix);
    printf("%sstations=(all,selected,none)\n", prefix);
    printf("%sorigins=(all,selected,none)\n", prefix);
    printf("%spaths=(all,selected,none)\n", prefix);
    printf("%sellipses=(all,selected,none)\n", prefix);
    printf("%sdistances=(all,selected,none)\n", prefix);
    printf("%sfk_azimuths=(all,selected,none)\n", prefix);
    printf("%srotation_azimuths=(all,selected,none)\n", prefix);

    printf("%sprojection=(LC,CEA,MER,ORTHO,AE,AEA,UTM CELL_ID)\n", prefix);

    printf("%soverlays=OVERLAY,OVERLAY...\n", prefix);
    printf("%soverlays=none\n", prefix);

    printf("%sthemes.help\n", prefix);
    printf("%sprint.help\n", prefix);
}

bool MapWindow::onlySelect(DataChange *c)
{
    if((c->select_waveform || c->select_arrival || c->unknown_select ||
	c->select_origin) &&
	!c->waveform && !c->arrival && !c->assoc && !c->origin && !c->origerr &&
	!c->stassoc && !c->stamag && !c->netmag && !c->hydro && !c->infra &&
	!c->wftag && !c->amplitude && !c->ampdescript && !c->filter &&
	!c->parrival && !c->unknown) return true;
    return false;
}

void MapWindow::setProjection(const char *name)
{
    string msg;

    if( projection_menu->parseCmd("true", msg) != COMMAND_PARSED )
    {
	cerr << msg << endl;
    }
}

void MapWindow::open(void)
{
    const char *c;
    char *file, dir[MAXPATHLEN+1];

    if((c = Application::getInstallDir("GEOTOOL_HOME")))
    {
        snprintf(dir, sizeof(dir), "%s/tables", c);
    }
    else {
	snprintf(dir, sizeof(dir), ".");
    }

    if(fileDialog == NULL) {
	fileDialog = new FileDialog("Open file", this, EXISTING_FILE, dir,
				(char *)"*");
    }
    fileDialog->setVisible(true);

    if((file = fileDialog->getFile()) != NULL)
    {
	readFile(file);
	XtFree(file);
    }
}

bool MapWindow::readFile(const char *path)
{
    if(!themes) {
	themes = new MapThemes("Map Themes", this, this);
    }
    if(	stringCaseEndsWith(path, ".shp") ||
	stringCaseEndsWith(path, ".shp.gz") ||
	stringCaseEndsWith(path, ".shx") ||
	stringCaseEndsWith(path, ".shx.gz") ||
	stringCaseEndsWith(path, ".dbf") ||
	stringCaseEndsWith(path, ".dbf.gz") ||
	stringCaseEndsWith(path, ".sbn") ||
	stringCaseEndsWith(path, ".sbn.gz") ||
	stringCaseEndsWith(path, ".sbx") ||
	stringCaseEndsWith(path, ".sbx.gz") ||
	stringCaseEndsWith(path, ".avl") ||
	stringCaseEndsWith(path, ".avl.gz") ||
	stringCaseEndsWith(path, ".prj") ||
	stringCaseEndsWith(path, ".prj.gz"))
    {
	return themes->readShapeFile(path);
    }
    else if(stringCaseEndsWith(path, ".nc") ||
	stringCaseEndsWith(path, ".nc.gz"))
    {
	themes->readNetCDF(path);
    }
    else if(stringCaseEndsWith(path, ".pol") ||
	    stringCaseEndsWith(path, ".pol.gz"))
    {
	themes->readPolar(path);
    }
    else if(stringEndsWith(path, ".grd") ||
	stringEndsWith(path, ".grd.gz") ||
	stringEndsWith(path, ".glb") ||
	stringEndsWith(path, ".glb.gz") ||
	stringEndsWith(path, "srtm1") ||
	stringEndsWith(path, "srtm1.gz"))
    {
	themes->readGriddedData(path);
    }
//    else if(stringCaseEndsWith(path, ".ovr")) {
    else {
	try {
	    overlay_form->listOverlay(new MapOverlay(path, this));
	}
	catch(...) {
	    showWarning(GError::getMessage());
	    return false;
	}
    }
    return true;
}

void MapWindow::overlayWindow(void)
{
    if(!overlay_form) {
	overlay_form = new MapOverlayForm("Map Overlays", this, this);
    }
    overlay_form->setVisible(true);
}

int MapWindow::numOverlays(void)
{
    if(!overlay_form) {
	overlay_form = new MapOverlayForm("Map Overlays", this, this);
    }
    return overlay_form->getList()->numItems();
}

void MapWindow::displayOverlay(int pos)
{
    overlay_form->getList()->select(pos, true);
}

void MapWindow::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Map", this, this);
    }
    print_window->setVisible(true);
}

void MapWindow::initColors(void)
{
    int num_colors_init = 11;
    int r_init[] = {238, 119,  71,  38,  44,  44,  32, 210, 231, 237, 255};
    int g_init[] = {232, 133, 150, 183, 255, 208, 233, 233, 201, 158,   0};
    int b_init[] = {229, 208, 237, 237, 255,  26,   0,  11,  18,   0,   0};
    XColor color;
    Widget w = (Widget)map->baseWidget();

    if(!init_colors) return;

    init_colors = false;
    color_scale.num_colors = num_colors_init;
    color_scale.lines =
	(double *)malloc((color_scale.num_colors+1)*sizeof(double));
    color_scale.pixels = (Pixel *)malloc(color_scale.num_colors*sizeof(Pixel));

    for(int i = 0; i < num_colors_init; i++) {
	color.red = r_init[i]*256;
	color.green = g_init[i]*256;
	color.blue = b_init[i]*256;
	XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color);
	color_scale.pixels[i] = color.pixel;
    }
}

void MapWindow::setColorBar(double min, double max)
{
    double incre;
    int i;

    if(color_scale.num_colors > 0) {
	incre = (max - min)/color_scale.num_colors;
	for(i = 0; i < color_scale.num_colors+1; i++) {
	    color_scale.lines[i] = (double)(min + (i*incre));
	}
	map->themeColorScale(map_id, &color_scale);
    }
}

void MapWindow::print(FILE *fp, PrintParam *p)
{
    AxesParm *a;

    p->right -= 1.;
    if((a = map->hardCopy(fp, p, NULL))) free(a);
}

void MapWindow::mapUpdate(void)
{
    int		i, j, k, nsta, nsrc, type, num_paths;
    int		n_assoc_sta, display_mode;
    int		*src_id = NULL, *sta_id = NULL, *assoc_sta_id = NULL;
    int		*sta_gone = NULL;
    long	id;
    char	label[20];
    Pixel	fg;
    PathInfo	*pathinfo;
    bool	found_change = false, do_change = false;
    MapPlotStation *stas = NULL, sta;
    MapPlotSource *srcs = NULL, src;
    cvector<CssOriginClass> origins;
    cvector<CssOriginClass> selected_origins;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssSiteClass> sites;
    CssOriginClass *origin;

    if(get_resources)
    {
	get_resources = false;
	XtGetSubresources(base_widget, &sub_data, "data", "Data", resources,
			XtNumber(resources), NULL, 0);
    }

    if(!data_source) return;

    ignore_data_change = true;

    data_source->getTable(origins);
    data_source->getSelectedTable(selected_origins);
    data_source->getTable(origerrs);

    fg = stringToPixel("black");

    type = MapOverlay::getSymType(sub_data.source_symbol);

//    map_source_symbol = type;
//    map_source_size = sub_data.source_size;

    src_id = new int[origins.size()];

    for(i = 0; i < origins.size(); i++) {
	src_id[i] = origins[i]->getValue("mapid", &id) ? id : 0;
    }

    nsrc = map->getSources(&srcs);

    /* delete all map sources not in origin[] and not in overlay[]
     */
    for(i = 0; i < nsrc; i++)
    {
	for(j = 0; j < origins.size(); j++)
	{
	    if(src_id[j] > 0 && src_id[j] == srcs[i].id) break;
	}
	if(j == origins.size())
	{
	    if(!overlay_form->isOverlayId(srcs[i].id)) {
		map->mapDelete(srcs[i].id, false);
		found_change = true;
	    }
	}
    }
    /* check all existing sources for changes
     */
    display_mode = getOriginDisplay();

    for(i = 0; i < origins.size(); i++) if(src_id[i] > 0)
    {
	int dc = origins[i]->getDC();
	long orid = origins[i]->orid;

	for(j = 0; j < nsrc; j++) {
	    if(src_id[i] == srcs[j].id) break;
	}
	if(j == nsrc) continue;

	do_change = false;
	if( srcs[j].lat != origins[i]->lat ||
	    srcs[j].lon != origins[i]->lon ||
	    srcs[j].depth != origins[i]->depth ||
	    srcs[j].time != origins[i]->time)
	{
	    srcs[j].lat = origins[i]->lat;
	    srcs[j].lon = origins[i]->lon;
	    srcs[j].depth = origins[i]->depth;
	    srcs[j].time = origins[i]->time;
	    do_change = true;
	}
	for(k = 0; k < origerrs.size(); k++)
	{
	    if(dc == origerrs[k]->getDC() && orid == origerrs[k]->orid) break;
	}
	if(k < origerrs.size())
	{
	    if(srcs[j].smajax != origerrs[k]->smajax ||
		srcs[j].sminax != origerrs[k]->sminax ||
		srcs[j].strike != origerrs[k]->strike)
	    {
		srcs[j].smajax = origerrs[k]->smajax;
		srcs[j].sminax = origerrs[k]->sminax;
		srcs[j].strike = origerrs[k]->strike;
		do_change = true;
	    }
	}
	else if(srcs[j].smajax != -1.)
	{
	    srcs[j].smajax = -1.;
	    srcs[j].sminax = -1.;
	    srcs[j].strike = -1.;
	    do_change = true;
	}
	if(do_change) {
	    map->change((MapObject *)&srcs[j], false);
	}
    }
		
    /* add all new sources
     */
    for(i = 0; i < origins.size(); i++) if(src_id[i] == 0 &&
		origins[i]->lat > -900. && origins[i]->lon > -900.)
    {
	int dc = origins[i]->getDC();
	int orid = origins[i]->orid;
	snprintf(label, sizeof(label), "%d", orid);
	src.orid = orid;
	src.label = label;
	src.lat = origins[i]->lat;
	src.lon = origins[i]->lon;
	src.depth = origins[i]->depth;
	src.time = origins[i]->time;
	src.tag_loc = NOPREF;
	src.sym.type = type;
	src.sym.size = sub_data.source_size;
	src.sym.fg = fg;
	src.sym.display = display_mode;
	for(j = 0; j < selected_origins.size()
		&& selected_origins[j] != origins[i]; j++);
	src.sym.selected = (j < selected_origins.size()) ? true : false;

	for(j = 0; j < origerrs.size(); j++) {
	    if(dc == origerrs[j]->getDC() && orid == origerrs[j]->orid) break;
	}
	if(j < origerrs.size())
	{
	    src.smajax = origerrs[j]->smajax;
	    src.sminax = origerrs[j]->sminax;
	    src.strike= origerrs[j]->strike;
	}
	else
	{
	    src.smajax = -1.0;
	    src.sminax = -1.0;
	    src.strike = -1.0;
	}
	src_id[i] = map->addSource(&src, False);
	origins[i]->putValue("mapid", (long)src_id[i]);
	found_change = true;
    }
    Free(srcs);

    num_paths = data_source->getPathInfo(&pathinfo);

    nsta = map->getStations(&stas);

    if(num_paths) {
	sta_id = new int[num_paths];
	assoc_sta_id = new int[num_paths];
	for(i = 0; i < num_paths; i++) sta_id[i] = 0;
    }

    /* delete all map stations not in pathinfo[];
     */
    if(nsta) sta_gone = new int[nsta];

    for(i = 0; i < nsta; i++)
    {
	sta_gone[i] = 0;
	for(j = 0; j < num_paths; j++) {
	    if(!strcmp(stas[i].label, pathinfo[j].sta)) break;
	}
	if(j < num_paths) {
	    sta_id[j] = stas[i].id;
	}
	else
	{
	    if(!overlay_form->isOverlayId(stas[i].id))
	    {
		map->mapDelete(stas[i].id, false);
		sta_gone[i] = 1;
		found_change = true;
	    }
	}
    }
    /* add all new stations
     */
    display_mode = getStationDisplay();

    for(i = 0; i < num_paths; i++) if(pathinfo[i].lat > -900.)
    {
	PathInfo *p = &pathinfo[i];
	/*
	 * check for repeat sta
	 */
	for(j = 0; j < i; j++) {
	    if(!strcmp(pathinfo[j].sta, p->sta)) break;
	}
	if(j < i) {
	    sta_id[i] = sta_id[j];
	    continue;
	}

	for(j = 0; j < nsta; j++)
	{
	    if(!sta_gone[j] && !strcmp(stas[j].label, p->sta)) break;
//	    if(!strcmp(stas[j].label, cd->sta)) break;
	}
	if(j == nsta)
	{
	    sta.label = p->sta;
	    sta.lat = p->lat;
	    sta.lon = p->lon;
	    sta.tag_loc = NOPREF;
	    sta.sym.type = FILLED_TRIANGLE;
	    sta.sym.display = display_mode;
	    sta.sym.size = 6;
	    sta.sym.fg =  p->fg;
	    sta.sym.selected = False;
	    sta_id[i] = map->addStation(&sta, false);
	    found_change = true;
	}
    }
    if(sta_gone) delete [] sta_gone;
    Free(stas);

    data_source->getTable(sites);
    nsta = map->getStations(&stas);

    fg = stringToPixel("sea green");
    for(i = 0; i < sites.size(); i++) {
	for(j = 0; j < nsta && strcasecmp(stas[j].label, sites[i]->sta); j++);
	if(j == nsta) {
	    sta.label = sites[i]->sta;
	    sta.lat = sites[i]->lat;
	    sta.lon = sites[i]->lon;
	    sta.tag_loc = NOPREF;
	    sta.sym.type = FILLED_TRIANGLE;
	    sta.sym.display = display_mode;
	    sta.sym.size = 6;
	    sta.sym.fg =  fg;
	    sta.sym.selected = False;
	    map->addStation(&sta, false);
	    found_change = true;
	}
    }
    Free(stas);

    map->clearArcs(NULL, false);

    /* associate stations with origins
     */
    for(i = 0; i < origins.size(); i++)
	if(origins[i]->lat > -900. && origins[i]->lon > -900.)
    {
	int dc = origins[i]->getDC();
	long orid = origins[i]->orid;
	n_assoc_sta = 0;
	for(j = 0; j < num_paths; j++)
	{
	    if((origin = pathinfo[j].origin) != NULL
		&& origin->getDC() == dc && origin->orid == orid)
	    {
		for(k = 0; k < n_assoc_sta; k++) {
		    if(sta_id[j] == assoc_sta_id[k]) break;
		}
		if(k == n_assoc_sta) {
		    assoc_sta_id[n_assoc_sta++] = sta_id[j];
		}
	    }
	}
	if(map->assocSta(src_id[i], n_assoc_sta, assoc_sta_id, true, false))
	{
	    found_change = true;
	}
    }
    if(found_change) map->update();

    ignore_data_change = false;

    Free(pathinfo);
    delete [] src_id;
    delete [] sta_id;
    delete [] assoc_sta_id;
}

void MapWindow::mapUpdateSelected(void)
{
    MapPlotSource *sources = NULL;
    int *src_id = NULL, i, j;
    long id;
    cvector<CssOriginClass> selected_origins;

    if(!data_source) return;

    data_source->getSelectedTable(selected_origins);

    src_id = new int[selected_origins.size()];

    int nsrc = map->getSources(&sources);

    for(i = 0; i < selected_origins.size(); i++)
    {
	src_id[i] = selected_origins[i]->getValue("mapid", &id) ? id : 0;
    }

    for(i = 0; i < nsrc; i++)
    {
	for(j = 0; j < selected_origins.size(); j++) {
	    if(src_id[j] > 0 && src_id[j] == sources[i].id) break;
	}
	if(j < selected_origins.size()) {
	    if(!sources[i].sym.selected) {
		map->selectSource(sources[i].id, true, false, false);
	    }
	}
	else if(sources[i].sym.selected) {
	    map->selectSource(sources[i].id, false, false, false);
	}
    }
    Free(sources);
    if(src_id) delete [] src_id;
    map->update();
}

void MapWindow::selectTableRows(void)
{
    MapPlotSource *sources = NULL;
    int *src_id = NULL, i, j, k;
    long id;
    cvector<CssOriginClass> origins, selected_origins;
    bool table_row_selected;

    if( !(tv = data_source->getTableViewerInstance()) ) return;

    data_source->getTable(origins);

    src_id = new int[origins.size()];

    for(i = 0; i < origins.size(); i++) {
	src_id[i] = origins[i]->getValue("mapid", &id) ? id : 0;
    }

    data_source->getSelectedTable(selected_origins);

    int nsrc = map->getSources(&sources);

    for(i = 0; i < origins.size(); i++) if(src_id[i] > 0)
    {
	for(j = 0; j < nsrc && src_id[i] != sources[j].id; j++);
	if(j == nsrc) continue;

	for(k = 0; k < selected_origins.size()
		&& selected_origins[k] != origins[i]; k++);
	table_row_selected = (k < selected_origins.size()) ? true : false;

	if(sources[j].sym.selected && !table_row_selected) {
	    tv->selectTableRecord(cssOrigin, origins[i], true);
	}
	else if(!sources[j].sym.selected && table_row_selected) {
	    tv->selectTableRecord(cssOrigin, origins[i], false);
	}
    }
    if(src_id) delete [] src_id;
    Free(sources);
}

void MapWindow::crosshairCB(void)
{
    double lat, lon, az = -999., delta = -1.;

    if(!map->getCoordinates(&lat, &lon, &delta, &az)) return;

    if(lon < -180) lon = -180;
    else if(lon > 180) lon = 180;
    longitude_text->setString("%.2f", lon);

    if(lat < -90) lat = -90;
    else if(lat > 90) lat = 90;
    latitude_text->setString("%.2f",lat);

    if(delta >= 0.) {
	if( !utm ) {
	    delta_text->setString("%.2f", delta);
	}
	else {
	    delta_text->setString("%.0f", delta);
	}
    }
    if(az > -900.) azimuth_text->setString("%.2f", az);
}

void MapWindow::positionCrosshair(void)
{
    double lat, lon, delta=10., az=60.;

    if(latitude_text->getDouble(&lat) && longitude_text->getDouble(&lon))
    {
	ignore_crosshair = true;
	delta_text->getDouble(&delta);
	azimuth_text->getDouble(&az);
        map->positionCrosshair(lat, lon, delta, az);
	ignore_crosshair = false;
    }
}

void MapWindow::addActionListener(ActionListener *listener,
			const string &action_type)
{
    if(	!action_type.compare(XtNselectStationCallback) ||
	!action_type.compare(XtNdragStationCallback) ||
	!action_type.compare(XtNselectSourceCallback) ||
	!action_type.compare(XtNdragSourceCallback) ||
	!action_type.compare(XtNmapMeasureCallback) ||
	!action_type.compare(XtNselectArcCallback) ||
	!action_type.compare(XtNselectCircleCallback) ||
	!action_type.compare(XtNcursorMotionCallback) ||
	!action_type.compare(XtNshapeSelectCallback) ||
	!action_type.compare(XtNsymbolSelectCallback) ||
	!action_type.compare(XtNsymbolInfoCallback) ||
	!action_type.compare(XtNutmCallback))
    {
	map->addActionListener(listener, action_type);
    }
    else {
	Component::addActionListener(listener, action_type);
    }
}

void MapWindow::removeActionListener(ActionListener *listener,
			const string &action_type)
{
    if(	!action_type.compare(XtNselectStationCallback) ||
	!action_type.compare(XtNdragStationCallback) ||
	!action_type.compare(XtNselectSourceCallback) ||
	!action_type.compare(XtNdragSourceCallback) ||
	!action_type.compare(XtNmapMeasureCallback) ||
	!action_type.compare(XtNselectArcCallback) ||
	!action_type.compare(XtNselectCircleCallback) ||
	!action_type.compare(XtNcursorMotionCallback) ||
	!action_type.compare(XtNshapeSelectCallback) ||
	!action_type.compare(XtNsymbolSelectCallback) ||
	!action_type.compare(XtNsymbolInfoCallback) ||
	!action_type.compare(XtNutmCallback))
    {
	map->removeActionListener(listener, action_type);
    }
    else {
	Component::removeActionListener(listener, action_type);
    }
}

void MapWindow::removeAllListeners(const string &action_type)
{
    if(	!action_type.compare(XtNselectStationCallback) ||
	!action_type.compare(XtNdragStationCallback) ||
	!action_type.compare(XtNselectSourceCallback) ||
	!action_type.compare(XtNdragSourceCallback) ||
	!action_type.compare(XtNmapMeasureCallback) ||
	!action_type.compare(XtNselectArcCallback) ||
	!action_type.compare(XtNselectCircleCallback) ||
	!action_type.compare(XtNcursorMotionCallback) ||
	!action_type.compare(XtNshapeSelectCallback) ||
	!action_type.compare(XtNsymbolSelectCallback) ||
	!action_type.compare(XtNsymbolInfoCallback) ||
	!action_type.compare(XtNutmCallback))
    {
	map->removeAllListeners(action_type);
    }
    else {
	Component::removeAllListeners(action_type);
    }
}
