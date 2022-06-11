#ifndef BASIC_MAP_H
#define BASIC_MAP_H

#include <iostream>
#include "motif++/Frame.h"
#include "motif++/Application.h"
extern "C" {
#include "widget/MapTypes.h"
}

using namespace std;

#define BasicMap_warn(a) cerr <<"BasicMap warning: no "<<a<<" routine\n"

/** A virtual interface to mapping operations.
 *  @ingroup libwgets
 */
class BasicMap : public Frame
{
    public:
	virtual ~BasicMap(void) {}

	virtual void clearArcs(char *label, bool redisplay) {
		BasicMap_warn("clearArcs");}
	virtual void clearDeltas(char *label, bool redisplay) {
		BasicMap_warn("clearDeltas");}
	virtual int getCrosshairs(double **lat, double **lon) {
		BasicMap_warn("getCrosshairs"); return 0;}
	virtual void measureDeleteAll(void) {
		BasicMap_warn("measureDeleteAll");}
	virtual void measureDelete(int id) {
		BasicMap_warn("measureDelete");}
	virtual int getMeasurements(MapMeasure **m) {
		BasicMap_warn("getMeasurements"); return 0;}
	virtual void selectStation(char *name, bool selected,
		bool do_callbacks) { BasicMap_warn("selectStation"); }
	virtual void selectSource(int id, bool selected, bool do_callbacks) {
		BasicMap_warn("selectSource");}
	virtual void rotation(void) { BasicMap_warn("rotation"); }
	virtual void rotateToSta(void) { BasicMap_warn("rotateToSta"); }
	virtual void rotateToOrigin(void) { BasicMap_warn("rotateToOrigin"); }
	virtual void rotate(double lon, double lat, bool redisplay) {
		BasicMap_warn("rotate"); }
	virtual bool getCoordinates(double *lat, double *lon) {
		BasicMap_warn("getCoordinates"); return 0;}
	virtual int addStation(MapPlotStation *station, bool redisplay) {
		BasicMap_warn("addStation"); return -1;}
	virtual int getStations(MapPlotStation **sta) {
		BasicMap_warn("getStations"); return 0;}
	virtual int addSource(MapPlotSource *source, bool redisplay) {
		BasicMap_warn("addSource"); return -1;}
	virtual int getSources(MapPlotSource **src) {
		BasicMap_warn("getSources"); return 0;}
	virtual int addLine(MapPlotLine *line, bool redisplay) {
		BasicMap_warn("addLine"); return -1;}
	virtual int addTheme(MapPlotTheme *theme, bool redisplay) {
		BasicMap_warn("addTheme"); return -1;}
	virtual int getLines(MapPlotLine **line) {
		BasicMap_warn("getLines"); return 0;}
	virtual int addSymbols(int nsymbols, MapPlotSymbol *symbol,
		bool redisplay) { BasicMap_warn("addSymbols"); return -1;}
	virtual int addSymbolGroup(MapPlotSymbolGroup *sym_grp, bool redisplay)
		{ BasicMap_warn("addSymbolGroup"); return -1;}
	virtual int addPolygon(MapPlotPolygon *poly, bool redisplay) {
		BasicMap_warn("addPolygon"); return 0;}
	virtual int addRectangle(MapPlotRectangle *rect, bool redisplay) {
		BasicMap_warn("addRectangle"); return 0;}
	virtual int getSymbolGroups(MapPlotSymbolGroup **sym_grp) {
		BasicMap_warn("getSymbolGroups"); return 0;}
	virtual int addEllipse(MapPlotEllipse *ellipse, bool redisplay) {
		BasicMap_warn("addEllipse"); return -1;}
	virtual int getEllipses(MapPlotEllipse **ellipse) {
		BasicMap_warn("getEllipses"); return 0;}
	virtual int addArc(MapPlotArc *arc, bool redisplay) {
		BasicMap_warn("addArc"); return -1;}
	virtual int getStaArc(char *sta, char *label, MapPlotArc *arc) {
		BasicMap_warn("getStaArc"); return -1;}
	virtual void display(int num_ids, int *id, int display_mode,
		bool redisplay) { BasicMap_warn("display");}
	virtual void displayArcs(char *label, int display_mode) {
		BasicMap_warn("displayArcs"); }
	virtual void displayDeltas(char *label, int display_mode) {
		BasicMap_warn("displayDeltas"); }
	virtual void displayEllipses(char *label, int display_mode) {
		BasicMap_warn("displayEllipses"); }
	virtual void displayStations(char *label, int display_mode) {
		BasicMap_warn("displayStations"); }
	virtual void displaySources(char *label, int display_mode) {
		BasicMap_warn("displaySources"); }
	virtual int getStaDelta(char *sta, char *label, MapPlotDelta *delta) {
		BasicMap_warn("getStaDelta"); return -1;}
	virtual int assoc(int assoc_id, int id) {
		BasicMap_warn("assoc"); return -1;}
	virtual int getArcs(MapPlotArc **arc) {
		BasicMap_warn("getArcs"); return 0;}
	virtual int addDelta(MapPlotDelta *delta, bool redisplay) {
		BasicMap_warn("addDelta"); return 0;}
	virtual int getDeltas(MapPlotDelta **delta) {
		BasicMap_warn("getDeltas"); return 0;}
	virtual int mapDelete(int id, bool redisplay) {
		BasicMap_warn("mapDelete"); return 0;}
	virtual int change(MapObject *obj, bool redisplay) {
		BasicMap_warn("change"); return 0;}
	virtual void update(void) { BasicMap_warn("update"); }
	virtual void circles(MapCircle *circle, int ncircles) {
		BasicMap_warn("circles"); }
	virtual bool assocSta(int src_id, int nsta, int *sta_id, bool remove,
		bool redisplay) { BasicMap_warn("assocSta"); return false;}
	virtual void stationColor(Pixel color, char *sta) {
		BasicMap_warn("stationColor"); }
	virtual void changeStationColor(int id, Pixel fg, bool redisplay) {
		BasicMap_warn("changeStationColor"); }
	virtual void changeImage(int theme_id, float *z, bool redisplay) {
		BasicMap_warn("changeImage"); }
	virtual int displayShape(int theme_id, int shape_index,
		bool display_shape, bool redisplay) {
		BasicMap_warn("displayShape"); return 0;}
	virtual int displayTheme(int theme_id, bool display_theme) {
		BasicMap_warn("displayTheme"); return 0;}
	virtual void themeCursor(int theme_id, bool on) {
		BasicMap_warn("themeCursor"); }
	virtual int themeBndry(int theme_id, bool bndry) {
		BasicMap_warn("themeBndry"); return 0;}
	virtual int themeFill(int theme_id, bool fill) {
		BasicMap_warn("themeFill"); return 0;}
	virtual void setShapeSelected(int theme_id, vector<int> ishape) {
		BasicMap_warn("setShapeSelected"); }
	virtual int themeBndryColor(int theme_id, const string &color_name,
		bool redisplay) {
		BasicMap_warn("themeBndryColor"); return 0;}
	virtual int themeFillColor(int theme_id, const string &color_name,
		bool redisplay) { BasicMap_warn("themeFillColor"); return 0;}
	virtual int shapeLabels(int theme_id, vector<const char *> &labels) {
		BasicMap_warn("shapeLabels"); return 0;}
	virtual int displayShapeLabels(int theme_id, bool redisplay) {
		BasicMap_warn("displayShapeLabels"); return 0;}
	virtual int themeOnOffDelta(int theme_id, double on_delta,
		double off_delta, bool redisplay) {
		BasicMap_warn("themeOnOffDelta"); return 0;}
	virtual int shapeColor(int theme_id, int num, double *values,
			int num_bounds, double *bounds) {
		BasicMap_warn("shapeColor"); return 0;}
	virtual int themeSymbolType(int theme_id, int symbol_type) {
		BasicMap_warn("themeSymbolType"); return 0;}
	virtual int themeSymbolSize(int theme_id, int symbol_size) {
		BasicMap_warn("themeSymbolSize"); return 0;}
	virtual bool shapeIsDisplayed(int theme_id, int shape_index) {
		BasicMap_warn("shapeIsDisplayed"); return false;}
	virtual int themeColorScale(int theme_id, ColorScale *color_scale) {
		BasicMap_warn("themeColorScale"); return 0;}
	virtual int themeColorBar(int theme_id) {
		BasicMap_warn("themeColorBar"); return 0;}
	virtual void themeColorBarOff(void) {
		BasicMap_warn("themeColorBarOff"); }
	virtual int themeLevelDown(int theme_id) {
		BasicMap_warn("themeLevelDown"); return 0;}
	virtual int themeLevelUp(int theme_id) {
		BasicMap_warn("themeLevelUp"); return 0;}
	virtual bool colors(int num_colors, int *r, int *g, int *b) {
		BasicMap_warn("colors"); return false;}
	virtual void colorLines(int num_lines, double *lines) {
		BasicMap_warn("colorLines"); }
	virtual void setProjection(int projection) {
		BasicMap_warn("setProjection"); }
	virtual void displaySourceTags(int display_mode) {
		BasicMap_warn("displaySourceTags"); }
	virtual void displayStationTags(int display_mode) {
		BasicMap_warn("displayStationTags"); }
	virtual void displayGrid(int display_mode) {
		BasicMap_warn("displayGrid"); }
	virtual void displayPaths(int display_mode) {
		BasicMap_warn("displayPaths"); }
	virtual int getOriginDisplay(void) {
		BasicMap_warn("getOriginDisplay"); return 0; }
	virtual int getStationDisplay(void) {
		BasicMap_warn("getStationDisplay"); return 0; }
	virtual int getFKAzDisplay(void) {
		BasicMap_warn("getFKAzDisplay"); return 0; }
	virtual bool setMapLimits(double lonmin, double lonmax, double latmin,
			double latmax) {
		BasicMap_warn("setMapLimits"); return false; }


    protected:
	// can only be constructed by a subclass
	BasicMap(const string &name, Component *parent, const string &title="",
		bool independent=true) :
		Frame(name, parent, title, true, independent)
	{
	    Application::addMap(this);
	}

    private:
};

#endif
