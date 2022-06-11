#ifndef MAP_H
#define MAP_H

#include "widget/AxesClass.h"
#include "widget/MapPlot.h"

/** Map widget.
 *  @ingroup libwgets
 */
class Map : public AxesClass
{
    public:

	Map(const string &name, Component *parent, Arg *args=NULL, int num=0);
	~Map(void);

	int addImage(char *label, int nx, int ny, float *x, float *y, float *z,
		float no_data_flag, int mode, MapPlotTheme *theme,
		bool redisplay)
	{
	    return MapPlotAddImage(mp, label, nx, ny, x, y, z, no_data_flag,
				mode, theme, (Boolean)redisplay);
	}
	void clearArcs(const char *label, bool redisplay) {
	    MapPlotClearArcs(mp, (char *)label, (Boolean)redisplay);
	}
	void clearDeltas(const char *label, bool redisplay) {
	    MapPlotClearDeltas(mp, (char *)label, (Boolean)redisplay);
	}
	int getCrosshairs(double **lat, double **lon) {
	    return MapPlotGetCrosshairs(mp, lat, lon);
	}
	void measureDeleteAll(void) { MapMeasureDeleteAll(mp); }
	void measureDelete(int id) { MapMeasureDelete(mp, id); }
	int getMeasurements(MapMeasure **m) {
	    return MapPlotGetMeasurements(mp, m);
	}
	void selectStation(char *name, bool selected, bool callbacks,
		bool redisplay) {
	    MapPlotSelectStation(mp, name,(Boolean)selected,
			(Boolean)callbacks, (Boolean)redisplay);
	}
	void selectSource(int id, bool selected, bool callbacks,bool redisplay){
	    MapPlotSelectSource(mp, id, (Boolean)selected, (Boolean)callbacks,
		(Boolean)redisplay);
	}
	void rotation(void) { MapPlotRotation(mp); }
	void rotateToSta(void) { MapPlotRotateToSta(mp); }
	void rotateToStation(char *sta) { MapPlotRotateToStation(mp, sta); }
	void rotateToOrigin(void) { MapPlotRotateToOrig(mp); }
	void rotate(double lon, double lat, bool redisplay) {
	    MapPlotRotate(mp, lon, lat, (Boolean)redisplay);
	}
	bool getCoordinates(double *lat, double *lon, double *delta,
		double *azimuth)
	{
	    return MapPlotGetCoordinates(mp, lat, lon, delta, azimuth);
	}
	bool positionCrosshair(double lat, double lon, double delta,
		double azimuth) {
	    return MapPlotPositionCrosshairDA(mp, lat, lon, delta, azimuth);
	}
	bool positionCrosshair(double lat, double lon) {
	    return MapPlotPositionCrosshair(mp, lat, lon);
	}
	bool positionCrosshair(double lat, double lon, bool do_callback) {
	    return MapPlotPositionCrosshair2(mp, lat, lon, do_callback);
	}
	int addStation(MapPlotStation *station, bool redisplay) {
	    return MapPlotAddStation(mp, station, (Boolean)redisplay);
	}
	int getStations(MapPlotStation **sta) {
	    return MapPlotGetStations(mp, sta);
	}
	int addSource(MapPlotSource *source, bool redisplay) {
	    return MapPlotAddSource(mp, source, (Boolean)redisplay);
	}
	int getSources(MapPlotSource **src) {
	    return MapPlotGetSources(mp, src);
	}
	int addLine(MapPlotLine *line, bool redisplay) {
	    return MapPlotAddLine(mp, line, (Boolean)redisplay);
	}
	int addTheme(MapPlotTheme *theme, bool redisplay) {
	    return MapPlotAddTheme(mp, theme, (Boolean)redisplay);
	}
	int getLines(MapPlotLine **line) {
	    return MapPlotGetLines(mp, line);
	}
	int addSymbols(int nsymbols, MapPlotSymbol *symbol, bool redisplay) {
	    return MapPlotAddSymbols(mp, nsymbols, symbol, (Boolean)redisplay);
	}
	int getSymbols(MapPlotSymbol **symbols) {
	    return MapPlotGetSymbols(mp, symbols);
	}
	int addSymbolGroup(MapPlotSymbolGroup *sym_grp, bool redisplay) {
	    return MapPlotAddSymbolGroup(mp, sym_grp, (Boolean)redisplay);
	}
	int addPolygon(MapPlotPolygon *poly, bool redisplay) {
	    return MapPlotAddPolygon(mp, poly, (Boolean)redisplay);
	}
	int addRectangle(MapPlotRectangle *rect, bool redisplay) {
	    return MapPlotAddRectangle(mp, rect, (Boolean)redisplay);
	}
	int getSymbolGroups(MapPlotSymbolGroup **sym_grp) {
	    return MapPlotGetSymbolGroups(mp, sym_grp);
	}
	int addEllipse(MapPlotEllipse *ellipse, bool redisplay) {
	    return MapPlotAddEllipse(mp, ellipse, (Boolean)redisplay);
	}
	int getEllipses(MapPlotEllipse **ellipse) {
	    return MapPlotGetEllipses(mp, ellipse);
	}
	int addArc(MapPlotArc *arc, bool redisplay) {
	    return MapPlotAddArc(mp, arc, (Boolean)redisplay);
	}
	int getStaArc(char *sta, char *label, MapPlotArc *arc) {
	    return MapPlotGetStaArc(mp, sta, label, arc);
	}
	void display(int num_ids, int *id, int display_mode, bool redisplay) {
	    MapPlotDisplay(mp, num_ids, id, display_mode, (Boolean)redisplay);
	}
	void displayArcs(char *label, int display_mode) {
	    MapPlotDisplayArcs(mp, label, display_mode);
	}
	void displayDeltas(char *label, int display_mode) {
	    MapPlotDisplayDeltas(mp, label, display_mode);
	}
	void displayEllipses(char *label, int display_mode) {
	    MapPlotDisplayEllipses(mp, label, display_mode);
	}
	void displayStations(char *label, int display_mode) {
	    MapPlotDisplayStations(mp, label, display_mode);
	}
	void displaySources(char *label, int display_mode) {
	    MapPlotDisplaySources(mp, label, display_mode);
	}
	int getStaDelta(char *sta, char *label, MapPlotDelta *delta) {
	    return MapPlotGetStaDelta(mp, sta, label, delta);
	}
	int assoc(int assoc_id, int id) {
	    return MapPlotAssoc(mp, assoc_id, id);
	}
	int getArcs(MapPlotArc **arc) { return MapPlotGetArcs(mp, arc); }
	int addDelta(MapPlotDelta *delta, bool redisplay) {
	    return MapPlotAddDelta(mp, delta, (Boolean)redisplay);
	}
	int getDeltas(MapPlotDelta **delta) {
	    return MapPlotGetDeltas(mp, delta);
	}
	int mapDelete(int id, bool redisplay) {
	    return MapPlotDelete(mp, id, (Boolean)redisplay);
	}
	int change(MapObject *obj, bool redisplay) {
	    return MapPlotChange(mp, obj, (Boolean)redisplay);
	}
	void update(void) { MapPlotUpdate(mp); }
	void circles(MapCircle *circle, int ncircles) {
	    MapPlotCircles(mp, circle, ncircles);
	}
	bool assocSta(int src_id, int nsta, int *sta_id, bool delet,
		bool redisplay) {
	    return (bool)MapPlotAssocSta(mp, src_id, nsta,sta_id,(Boolean)delet,
			(Boolean)redisplay);
	}
	void stationColor(Pixel color, char *sta) {
	    MapChangeStationColor(mp, color, sta);
	}
	void changeStationColor(int id, Pixel fg, bool redisplay) {
	    MapPlotChangeStationColor(mp, id, fg, (Boolean)redisplay);
	}
	void changeImage(int theme_id, float *z, bool redisplay) {
	    MapPlotChangeImage(mp, theme_id, z, (Boolean)redisplay);
	}
	int displayShape(int theme_id, int shape_index, bool display_shape,
		bool redisplay) {
	    return MapPlotDisplayShape(mp, theme_id, shape_index,
			(Boolean)display_shape, (Boolean)redisplay);
	}
	int displayTheme(int theme_id, bool display_theme) {
	    return MapPlotDisplayTheme(mp, theme_id, (Boolean)display_theme);
	}
	void themeCursor(int theme_id, bool on) {
	    MapPlotThemeCursor(mp, theme_id, (Boolean)on);
	}
	int themeBndry(int theme_id, bool bndry) {
	    return MapPlotThemeBndry(mp, theme_id, (Boolean)bndry);
	}
	int themeFill(int theme_id, bool fill) {
	    return MapPlotThemeFill(mp, theme_id, (Boolean)fill);
	}
	void setShapeSelected(int theme_id, vector<int> ishape) {
	    MapPlotSetShapeSelected(mp, theme_id, ishape);
	}
	int themeBndryColor(int theme_id, const string &color_name,
		bool redisplay) {
	    return MapPlotThemeBndryColor(mp, theme_id, color_name.c_str(),
			(Boolean)redisplay);
	}
	int themeFillColor(int theme_id, const string &color_name,
		bool redisplay) {
	    return MapPlotThemeFillColor(mp, theme_id, color_name.c_str(),
			(Boolean)redisplay);
	}
	int shapeLabels(int theme_id, vector<const char *> &labels) {
	    return MapPlotShapeLabels(mp, theme_id, labels);
	}
	int displayShapeLabels(int theme_id, bool redisplay) {
	    return MapPlotDisplayShapeLabels(mp, theme_id, (Boolean)redisplay);
	}
	int themeOnOffDelta(int theme_id, double on_delta, double off_delta,
		bool redisplay) {
	    return MapPlotThemeOnOffDelta(mp, theme_id, on_delta, off_delta,
			(Boolean)redisplay);
	}
	int shapeColor(int theme_id, int num, double *values, int num_bounds,
		double *bounds) {
	    return MapPlotShapeColor(mp, theme_id, num, values, num_bounds,
			bounds);
	}
	int themeSymbolType(int theme_id, int symbol_type) {
	    return MapPlotThemeSymbolType(mp, theme_id, symbol_type);
	}
	int themeSymbolSize(int theme_id, int symbol_size) {
	    return MapPlotThemeSymbolSize(mp, theme_id, symbol_size);
	}
	bool shapeIsDisplayed(int theme_id, int shape_index) {
	    return (bool)MapPlotShapeIsDisplayed(mp, theme_id, shape_index);
	}
	int themeColorScale(int theme_id, ColorScale *color_scale) {
	    return MapPlotThemeColorScale(mp, theme_id, color_scale);
	}
	int themeColorBar(int theme_id) {
	    return MapPlotThemeColorBar(mp, theme_id);
	}
	void themeColorBarOff(void) { MapPlotThemeColorBarOff(mp); }
	int themeLevelDown(int theme_id) {
	    return MapPlotThemeLevelDown(mp, theme_id);
	}
	int themeLevelUp(int theme_id) {
	    return MapPlotThemeLevelUp(mp, theme_id);
	}
	void setProjection(int projection) {
	    Arg args[1];
	    XtSetArg(args[0], XtNprojection, projection);
	    XtSetValues((Widget)mp, args, 1);
	}
	void displaySourceTags(int display_mode) {
	    Arg args[1];
	    XtSetArg(args[0], XtNdisplaySourceTags, display_mode);
	    XtSetValues((Widget)mp, args, 1);
	}
	void displayStationTags(int display_mode) {
	    Arg args[1];
	    XtSetArg(args[0], XtNdisplayStationTags, display_mode);
	    XtSetValues((Widget)mp, args, 1);
	}
	void displayGrid(int display_mode) {
	    Arg args[1];
	    XtSetArg(args[0], XtNmapDisplayGrid, display_mode);
	    XtSetValues((Widget)mp, args, 1);
	}
	void displayPaths(int display_mode) {
	    Arg args[1];
	    XtSetArg(args[0], XtNdisplayPaths, display_mode);
	    XtSetValues((Widget)mp, args, 1);
	}
	bool hasCrosshair(void) {
	    double *lat=NULL, *lon=NULL;
	    bool b = (MapPlotGetCrosshairs(mp, &lat, &lon) > 0) ? true : false;
	    Free(lat); Free(lon);
	    return b;
	}
	void deleteCrosshair(void) {
	    this->AxesClass::deleteCrosshair();
	    update();
	}
	bool setMapLimits(double lonmin, double lonmax, double latmin,
		double latmax) {
	    return MapPlotSetLimits(mp, lonmin, lonmax, latmin, latmax);
	}
	void getMapLimits(double *lonmin, double *lonmax, double *latmin,
		double *latmax) {
	    MapPlotGetLimits(mp, lonmin, lonmax, latmin, latmax);
	}

	void linkMap(Map *map) {
	    MapPlotLinkMap(mp, map->mp);
	}
	void unlinkMap(Map *map) {
	    MapPlotUnlinkMap(mp, map->mp);
	}
	bool displayUTM(char letter, int zone) {
	    return MapPlotDisplayUTM(mp, letter, zone);
	}
	void clearSymbols(bool redisplay) {
	    MapPlotClearSymbols(mp, redisplay);
	}
	void setPolarMaxRadius(double max_radius) {
	    MapPlotSetPolarMaxRadius(mp, max_radius);
	}
	double getPolarMaxRadius(void) {
	    return MapPlotGetPolarMaxRadius(mp);
	}
	void selectSymbol(MapPlotSymbol *s, bool redisplay) {
	    MapPlotSelectSymbol(mp, s, (Boolean)redisplay);
	}
	void setPolarMargin(double polar_margin) {
	    MapPlotSetPolarMargin(mp, polar_margin);
	}
	void displayPolarSelection(bool redisplay, double azimuth,
		double del_azimuth, double radius, double del_radius,
		bool do_callback=true) {
	    MapPlotDisplayPolarSelection(mp, (Boolean)redisplay, azimuth,
			del_azimuth, radius, del_radius, (Boolean)do_callback);
	}
	void positionPolarSelection(double azimuth, double del_azimuth,
		double radius, double del_radius, bool do_callback=false) {
	    MapPlotPositionPolarSelection(mp, azimuth, del_azimuth, radius,
			del_radius, (Boolean)do_callback);
	}
	bool getPolarSelection(double *azimuth, double *del_azimuth,
		double *radius, double *del_radius) {
	    return (Boolean)MapPlotGetPolarSelection(mp, azimuth, del_azimuth,
			radius, del_radius);
	}

    protected:
	MapPlotWidget mp;
	void init(Component *parent);

    private:
	static void selectStationCallback(Widget, XtPointer, XtPointer);
	static void dragStationCallback(Widget, XtPointer, XtPointer);
	static void selectSourceCallback(Widget, XtPointer, XtPointer);
	static void dragSourceCallback(Widget, XtPointer, XtPointer);
	static void mapMeasureCallback(Widget, XtPointer, XtPointer);
	static void selectArcCallback(Widget, XtPointer, XtPointer);
	static void selectCircleCallback(Widget, XtPointer, XtPointer);
	static void cursorMotionCallback(Widget, XtPointer, XtPointer);
	static void shapeSelectCallback(Widget, XtPointer, XtPointer);
	static void symbolSelectCallback(Widget, XtPointer, XtPointer);
	static void symbolInfoCallback(Widget, XtPointer, XtPointer);
	static void utmCallback(Widget, XtPointer, XtPointer);
	static void polarSelectCallback(Widget, XtPointer, XtPointer);
	static void selectBarCallback(Widget, XtPointer, XtPointer);
};

#endif
