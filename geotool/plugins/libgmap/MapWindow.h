#ifndef _MAPWINDOW_H
#define _MAPWINDOW_H

#include "BasicMap.h"
#include "DataReceiver.h"
#include "motif++/MotifDecs.h"
#include "motif++/Menu.h"
#include "motif++/Toggle.h"
#include "widget/PrintDialog.h"
#include "PrintClient.h"
#include "widget/Map.h"
#include "gobject++/DataSource.h"

class AxesLabels;
class TableViewer;

/** @defgroup libgmap plugin Map
 */

namespace libgmap {

class MapOverlay;
class MapOverlayForm;
class MapCursor;
class MapMeasureForm;
class MapThemes;
class UTMWindow;

/** Map window.
 *  @ingroup libgmap
 */
class MapWindow : public BasicMap, public DataReceiver, public PrintClient
{
    friend class MapOverlay;

    public:
	MapWindow(const string &name, Component *parent, DataSource *ds=NULL,
		bool independent=true, const string &title="", bool utm=false);
	~MapWindow(void);

	Map		*map;
	PrintDialog	*print_window;
	UTMWindow	*utm_window;

	virtual void print(FILE *fp, PrintParam *p);
        virtual void addActionListener(ActionListener *listener,
			const string &action_type);
        virtual void addActionListener(ActionListener *listener) {
		this->BasicMap::addActionListener(listener); }
        virtual void removeActionListener(ActionListener *listener,
			const string &action_type);
        virtual void removeActionListener(ActionListener *listener) {
		this->BasicMap::removeActionListener(listener); }
        virtual void removeAllListeners(const string &action_type);
	virtual ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	// DataReceiver interface
	void setDataSource(DataSource *ds);

	ParseCmd parseUTM(const string &c, string &msg);
	void setProjection(const char *name);
	void setColorMap(int num, Pixel *pixels);
	Pixel getColor(const char *name);
	bool readFile(const char *path);
	void displayOverlay(int pos);
	int numOverlays(void);

	// BasicMap interface
        void clearArcs(char *label, bool redisplay) {
		map->clearArcs(label, redisplay); }
        void clearDeltas(char *label, bool redisplay) {
		map->clearDeltas(label, redisplay); }
        int getCrosshairs(double **lat, double **lon) {
		return map->getCrosshairs(lat, lon); }
        void measureDeleteAll(void) { map->measureDeleteAll(); }
        void measureDelete(int id) { map->measureDelete(id); }
        int getMeasurements(MapMeasure **m) {
		return map->getMeasurements(m); }
        void selectStation(char *name, bool selected, bool do_callbacks) {
        	map->selectStation(name, selected, do_callbacks, true); }
        void selectSource(int id, bool selected, bool do_callbacks) {
        	map->selectSource(id, selected, do_callbacks, true); }
        void rotation(void) { map->rotation(); }
        void rotateToSta(void) { map->rotateToSta(); }
        void rotateToOrigin(void) { map->rotateToOrigin(); }
        void rotate(double lon, double lat, bool redisplay) {
		map->rotate(lon, lat, redisplay); }
        bool getCoordinates(double *lat, double *lon, double *delta,
		double *azimuth) {
		return map->getCoordinates(lat, lon, delta, azimuth); }
        bool getCoordinates(double *lat, double *lon) {
		return this->BasicMap::getCoordinates(lat, lon); }
        int addStation(MapPlotStation *station, bool redisplay) {
		return map->addStation(station, redisplay); }
        int getStations(MapPlotStation **sta) { return map->getStations(sta); }
        int addSource(MapPlotSource *source, bool redisplay) {
		return map->addSource(source, redisplay); }
        int getSources(MapPlotSource **src) { return map->getSources(src); }
        int addLine(MapPlotLine *line, bool redisplay) {
		return map->addLine(line, redisplay); }
        int addTheme(MapPlotTheme *theme, bool redisplay) {
		return map->addTheme(theme, redisplay); }
        int getLines(MapPlotLine **lines) { return map->getLines(lines); }
        int addSymbols(int nsymbols, MapPlotSymbol *symbol, bool redisplay) {
		return map->addSymbols(nsymbols, symbol, redisplay); }
        int addSymbolGroup(MapPlotSymbolGroup *sym_grp, bool redisplay) {
		return map->addSymbolGroup(sym_grp, redisplay); }
        int addPolygon(MapPlotPolygon *poly, bool redisplay) {
		return map->addPolygon(poly, redisplay); }
        int addRectangle(MapPlotRectangle *rect, bool redisplay) {
		return map->addRectangle(rect, redisplay); }
        int getSymbolGroups(MapPlotSymbolGroup **sym_grp) {
		return map->getSymbolGroups(sym_grp); }
        int addEllipse(MapPlotEllipse *ellipse, bool redisplay) {
        	return map->addEllipse(ellipse, redisplay); }
        int getEllipses(MapPlotEllipse **ellipse) {
        	return map->getEllipses(ellipse); }
        int addArc(MapPlotArc *arc, bool redisplay) {
        	return map->addArc(arc, redisplay); }
        int getStaArc(char *sta, char *label, MapPlotArc *arc) {
        	return map->getStaArc(sta, label, arc); }
        void display(int num_ids, int *id, int display_mode, bool redisplay) {
        	map->display(num_ids, id, display_mode, redisplay); }
        void displayArcs(char *label, int display_mode) {
        	map->displayArcs(label, display_mode); }
        void displayDeltas(char *label, int display_mode) {
        	map->displayDeltas(label, display_mode); }
        void displayEllipses(char *label, int display_mode) {
        	map->displayEllipses(label, display_mode); }
        void displayStations(char *label, int display_mode) {
        	map->displayStations(label, display_mode); }
        void displaySources(char *label, int display_mode) {
        	map->displaySources(label, display_mode); }
        int getStaDelta(char *sta, char *label, MapPlotDelta *delta) {
        	return map->getStaDelta(sta, label, delta); }
        int assoc(int assoc_id, int id) { return map->assoc(assoc_id, id); }
        int getArcs(MapPlotArc **arc) { return map->getArcs(arc); }
        int addDelta(MapPlotDelta *delta, bool redisplay) {
        	return map->addDelta(delta, redisplay); }
        int getDeltas(MapPlotDelta **delta) { return map->getDeltas(delta); }
        int mapDelete(int id, bool redisplay) {
        	return map->mapDelete(id, redisplay); }
	int change(MapObject *obj, bool redisplay) {
		return map->change(obj, redisplay); }
        void update(void) { map->update(); }
        void circles(MapCircle *circle, int ncircles) {
		map->circles(circle, ncircles); }
        bool assocSta(int src_id, int nsta, int *sta_id, bool remove,
                bool redisplay) {
		return map->assocSta(src_id, nsta, sta_id, remove, redisplay); }
        void stationColor(Pixel color, char *sta) {
        	map->stationColor(color, sta); }
        void changeStationColor(int id, Pixel fg, bool redisplay) {
        	map->changeStationColor(id, fg, redisplay); }
        void changeImage(int theme_id, float *z, bool redisplay) {
        	map->changeImage(theme_id, z, redisplay); }
        int displayShape(int theme_id, int shape_index,
                bool display_shape, bool redisplay) {
		return map->displayShape(theme_id, shape_index, display_shape,			redisplay); }
        int displayTheme(int theme_id, bool display_theme) {
        	return map->displayTheme(theme_id, display_theme); }
        void themeCursor(int theme_id, bool on) {
        	map->themeCursor(theme_id, on); }
        int themeBndry(int theme_id, bool bndry) {
        	return map->themeBndry(theme_id, bndry); }
        int themeFill(int theme_id, bool fill) {
        	return map->themeFill(theme_id, fill); }
        void setShapeSelected(int theme_id, vector<int> ishape) {
        	map->setShapeSelected(theme_id, ishape); }
        int themeBndryColor(int theme_id, const string &color_name,
                bool redisplay) {
        	return map->themeBndryColor(theme_id, color_name, redisplay); }
        int themeFillColor(int theme_id, const string &color_name,
                bool redisplay) {
        	return map->themeFillColor(theme_id, color_name, redisplay); }
        int shapeLabels(int theme_id, vector<const char *> &labels) {
        	return map->shapeLabels(theme_id, labels); }
        int displayShapeLabels(int theme_id, bool redisplay) {
        	return map->displayShapeLabels(theme_id, redisplay); }
        int themeOnOffDelta(int theme_id, double on_delta,
                double off_delta, bool redisplay) {
        	return map->themeOnOffDelta(theme_id, on_delta, off_delta,
		redisplay); }
        int shapeColor(int theme_id, int num, double *values,
                        int num_bounds, double *bounds) {
        	return map->shapeColor(theme_id, num, values, num_bounds,
			bounds); }
	int themeSymbolType(int theme_id, int symbol_type) {
		return map->themeSymbolType(theme_id, symbol_type); }
        int themeSymbolSize(int theme_id, int symbol_size) {
        	return map->themeSymbolSize(theme_id, symbol_size); }
        bool shapeIsDisplayed(int theme_id, int shape_index) {
        	return map->shapeIsDisplayed(theme_id, shape_index); }
        int themeColorScale(int theme_id, ColorScale *cs) {
        	return map->themeColorScale(theme_id, cs); }
        int themeColorBar(int theme_id) { return map->themeColorBar(theme_id); }
        void themeColorBarOff(void) { map->themeColorBarOff(); }
        int themeLevelDown(int theme_id) {return map->themeLevelDown(theme_id);}
        int themeLevelUp(int theme_id) { return map->themeLevelUp(theme_id); }
        void setProjection(int projection) {
        	return map->setProjection(projection); }
        void displaySourceTags(int display_mode) { map->displaySourceTags(display_mode); }
        void displayStationTags(int display_mode) {map->displayStationTags(display_mode);}
        void displayGrid(int display_mode) { map->displayGrid(display_mode); }
        void displayPaths(int display_mode) { map->displayPaths(display_mode); }

	int getOriginDisplay(void) {
	    if(origin_toggle[0]->state()) return MAP_ON;
	    else if(origin_toggle[1]->state()) return MAP_SELECTED_ON;
	    else return MAP_OFF;
	}
	int getStationDisplay(void) {
	    if(stations[0]->state()) return MAP_ON;
	    else if(stations[1]->state()) return MAP_SELECTED_ON;
	    else return MAP_OFF;
	}
	int getFKAzDisplay(void) {
	    if(fkaz[0]->state()) return MAP_ON;
	    else if(fkaz[1]->state()) return MAP_SELECTED_ON;
	    else return MAP_OFF;
	}

	static int get_line(FILE *fp, char *line, int len, int *line_no);

	void print(void);

    protected:
 	bool		init_colors;
	bool		utm;
	ColorScale	color_scale;
	int		map_id;
	FileDialog	*fileDialog;
	AxesLabels	*labels_window;
	MapCursor	*map_cursor_window;
	MapMeasureForm	*map_measure_window;
	TableViewer	*tv;

	gvector<MapOverlay *>	*overlays;

	int		num_color_map;
	Pixel		*color_map;

	MapOverlayForm	*overlay_form;
	MapThemes	*themes;

	TextField *latitude_text, *longitude_text;
	TextField *delta_text, *azimuth_text;
	Label *latitude_label, *longitude_label;
	Label *delta_label, *azimuth_label;
	Toggle *crosshair_toggle;
	bool ignore_crosshair;
	bool ignore_data_change;

	// File menu
	Button *open_button, *overlays_button, *print_button, *close_button;

	// Edit menu
	Menu *stations_menu, *origins_menu, *paths_menu, *ellipses_menu;
	Menu *rotaz_menu, *fkaz_menu, *dist_menu;
	Button *clear_dist_button, *clear_fkaz_button, *clear_rotaz_button;
	Toggle *stations[3], *origin_toggle[3], *paths[3], *ellipses[3];
	Toggle *station_tags, *origin_tags;
	Toggle *rotaz[3], *fkaz[3], *dist[3];

	// View menu
	Toggle *grid_toggle;
	Button *limits_button, *labels_button;

	// Option menu
	Menu *projection_menu;
	Toggle *lc_toggle, *cea_toggle, *mercator_toggle, *ortho_toggle;
	Toggle *ae_toggle, *aea_toggle, *utm_toggle;
	Button *map_cursor_button, *measure_button, *rotate_to_crosshair;
	Button *rotate_to_station, *rotate_to_origin, *themes_button;

	// Help menu
	Button *help_button;

	void createInterface(bool utm);
	void init(bool utm);
	void subMenuChoice(const char *, Menu **menu, Toggle **t);
	void actionPerformed(ActionEvent *action_event);
	void initColors(void);
	void setColorBar(double, double);
	void open(void);
	void overlayWindow(void);
	void mapUpdate(void);
	void mapUpdateSelected(void);
	void crosshairCB(void);
	void positionCrosshair(void);
	bool onlySelect(DataChange *);
	void selectTableRows(void);

    private:
	int last_theme_id;
	int last_shape_index;
};

} // namespace libgmap

#endif
