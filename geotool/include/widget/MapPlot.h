#ifndef	MAPPLOT_H_
#define	MAPPLOT_H_

using namespace std;
#include <vector>

#include <X11/Intrinsic.h>
#include "widget/Axes.h"
#include "widget/MapTypes.h"

#define XtNmapDir			(char *)"mapDir"
#define XtNprojection			(char *)"projection"
#define XtNmapWaterColor		(char *)"mapWaterColor"
#define XtNlabelFont			(char *)"labelFont"
#define XtNlonMin			(char *)"lonMin"
#define XtNlonMax			(char *)"lonMax"
#define XtNlatMin			(char *)"latMin"
#define XtNlatMax			(char *)"latMax"
#define XtNmovableStations		(char *)"movableStations"
#define XtNmovableSources		(char *)"movableSources"
#define XtNdisplayPaths			(char *)"displayPaths"
#define XtNdisplaySourceEllipses	(char *)"displaySourceEllipses"
#define XtNmapDisplayGrid		(char *)"mapDisplayGrid"
#define XtNdisplayCircles		(char *)"displayCircles"
#define XtNdisplayStationTags		(char *)"displayStationTags"
#define XtNdisplaySourceTags		(char *)"displaySourceTags"
#define XtNdisplayMapCil		(char *)"displayMapCil"
#define XtNdisplayMapBdy		(char *)"displayMapBdy"
#define XtNmeasure			(char *)"measure"
#define XtNselectStationCallback	(char *)"selectStationCallback"
#define XtNdragStationCallback		(char *)"dragStationCallback"
#define XtNselectSourceCallback		(char *)"selectSourceCallback"
#define XtNdragSourceCallback		(char *)"dragSourceCallback"
#define XtNmapMeasureCallback		(char *)"mapMeasureCallback"
#define XtNselectArcCallback		(char *)"selectArcCallback"
#define XtNselectCircleCallback		(char *)"selectCircleCallback"
#define XtNcursorMotionCallback		(char *)"cursorMotionCallback"
#define XtNshapeSelectCallback		(char *)"shapeSelectCallback"
#define XtNmercatorMaxLat		(char *)"mercatorMaxLat"
#define XtNrotateOnZoom			(char *)"rotateOnZoom"
#define XtNapplyAntiAlias		(char *)"applyAntiAlias"
#define XtNpolarMaxRadius		(char *)"polarMaxRadius"
#define XtNpolarDelAz			(char *)"polarDelAz"
#define XtNpolarDelDist			(char *)"polarDelDist"
#define XtNbarVerticalMargin		(char *)"barVerticalMargin"
#define XtNsymbolSelectCallback		(char *)"symbolSelectCallback"
#define XtNsymbolInfoCallback		(char *)"symbolInfoCallback"
#define XtNutmCallback			(char *)"utmCallback"
#define XtNpolarSelectCallback		(char *)"polarSelectCallback"
#define XtNselectBarCallback		(char *)"selectBarCallback"

#define XtCMapDir			(char *)"MapDir"
#define XtCProjection			(char *)"Projection"
#define XtCMapWaterColor		(char *)"MapWaterColor"
#define XtCLabelFont			(char *)"LabelFont"
#define XtCLonMin			(char *)"LonMin"
#define XtCLonMax			(char *)"LonMax"
#define XtCLatMin			(char *)"LatMin"
#define XtCLatMax			(char *)"LatMax"
#define XtCMovableStations		(char *)"MovableStations"
#define XtCMovableSources		(char *)"MovableSources"
#define XtCDisplayPaths			(char *)"DisplayPaths"
#define XtCDisplaySourceEllipses	(char *)"DisplaySourceEllipses"
#define XtCMapDisplayGrid		(char *)"MapDisplayGrid"
#define XtCDisplayCircles		(char *)"DisplayCircles"
#define XtCDisplayStationTags		(char *)"DisplayStationTags"
#define XtCDisplaySourceTags		(char *)"DisplaySourceTags"
#define XtCDisplayMapCil		(char *)"DisplayMapCil"
#define XtCDisplayMapBdy		(char *)"DisplayMapBdy"
#define XtCMeasure			(char *)"Measure"
#define XtCSelectStationCallback	(char *)"SelectStationCallback"
#define XtCDragStationCallback		(char *)"DragStationCallback"
#define XtCSelectSourceCallback		(char *)"SelectSourceCallback"
#define XtCDragSourceCallback		(char *)"DragSourceCallback"
#define XtCMapMeasureCallback     	(char *)"MapMeasureCallback"
#define XtCSelectArcCallback		(char *)"SelectArcCallback"
#define XtCSelectCircleCallback		(char *)"SelectCircleCallback"
#define XtCCursorMotionCallback		(char *)"CursorMotionCallback"
#define XtCShapeSelectCallback		(char *)"ShapeSelectCallback"
#define XtCMercatorMaxLat		(char *)"MercatorMaxLat"
#define XtCRotateOnZoom			(char *)"RotateOnZoom"
#define XtCApplyAntiAlias		(char *)"ApplyAntiAlias"
#define XtCPolarMaxRadius		(char *)"PolarMaxRadius"
#define XtCPolarDelAz			(char *)"PolarDelAz"
#define XtCPolarDelDist			(char *)"PolarDelDist"
#define XtCBarVerticalMargin		(char *)"BarVerticalMargin"
#define XtCSymbolSelectCallback		(char *)"SymbolSelectCallback"
#define XtCSymbolInfoCallback		(char *)"SymbolInfoCallback"
#define XtCUtmCallback			(char *)"UtmCallback"
#define XtCPolarSelectCallback		(char *)"PolarSelectCallback"
#define XtCSelectBarCallback		(char *)"SelectBarCallback"

#define XtNdataColor			(char *)"dataColor"
#define XtNcontourLabelColor		(char *)"contourLabelColor"
#define XtNcontourLabelSize		(char *)"contourLabelSize"
#define XtNmarkMax			(char *)"markMax"
#define XtNcontourInterval		(char *)"contourInterval"
#define XtNcontourMin			(char *)"contourMin"
#define XtNcontourMax			(char *)"contourMax"
#define XtNautoInterval			(char *)"autoInterval"
#define XtNmode				(char *)"mode"
#define XtNnumColors			(char *)"numColors"

#define XtCDataColor			(char *)"DataColor"
#define XtCContourLabelColor		(char *)"ContourLabelColor"
#define XtCContourLabelSize		(char *)"ContourLabelSize"
#define XtCMarkMax			(char *)"MarkMax"
#define XtCContourInterval		(char *)"ContourInterval"
#define XtCContourMin			(char *)"ContourMin"
#define XtCContourMax			(char *)"ContourMax"
#define XtCAutoInterval			(char *)"AutoInterval"
#define XtCMode				(char *)"Mode"
#define XtCNumColors			(char *)"NumColors"


#ifndef XtRDouble
#define XtRDouble	(char *)"double"
#endif

/// @cond
typedef struct _MapPlotClassRec	*MapPlotWidgetClass;
typedef struct _MapPlotRec	*MapPlotWidget;

extern WidgetClass		mapPlotWidgetClass;
/// @endcond


typedef struct
{
        int     type;          /* type of measure 0: pt 2 pt, 1: = circle */
	double	start_lat, start_lon;
	double  end_lat, end_lon;	/* only for type = 0 */
	double	delta;
	double	az, baz;		/* only for type = 0 */
} MapMeasureCallbackStruct;

typedef struct
{
	void	*pt;
	Boolean	primary;
} MapCircleCallbackStruct;

typedef struct
{
	int	index;		/* index into MapPlot, not sure if needed */
	int	type;		/* type of object 0: station, 1: event */
	double	lat, lon;	/* new lat lon of object */
	char	name[10];	/* station name */
	Boolean	moving;
} MapMoveObjectStruct;

typedef struct
{
	MapPlotLine	line;
	double		lon_min, lon_max, lat_min, lat_max;
	SegArray 	s;
} MapLine;

#define MAP_LINE_INIT \
{ \
	{ \
		0,				/* id */ \
		0,				/* npts */ \
		NULL,				/* label */ \
		False,				/* polar_coords */ \
		(double *)NULL,			/* lat */ \
		(double *)NULL,			/* lon */ \
		LINE_INFO_INIT,			/* line */ \
	},					/* line */ \
	0., 0., 				/* lon_min, lon_max */ \
	0., 0., 				/* lat_min, lat_max */ \
	SEG_ARRAY_INIT,				/* s */ \
}

typedef struct
{
	int	npoints;
	XPoint	points[6];
} PointsArray;

typedef struct
{
	MapPlotSymbolGroup	group;
	double			lon_min, lon_max, lat_min, lat_max;
	PointsArray		*p;
} MapSymbolGroup;

#define MAP_SYMBOL_GROUP_INIT \
{ \
	{ \
		0,				/* id */ \
		NULL,				/* label */ \
		False,				/* polar_coords */ \
		0,				/* npts */ \
		(int *)NULL,			/* size */ \
		(double *)NULL,			/* lat */ \
		(double *)NULL,			/* lon */ \
		SYMBOL_INFO_INIT,		/* sym */ \
	},					/* symbols */ \
	0., 0., 				/* lon_min, lon_max */ \
	0., 0., 				/* lat_min, lat_max */ \
	(PointsArray *)NULL,			/* p */ \
}

typedef struct
{
	short		position[4];
	int		tag_loc;		/* user can set this */
	int		text_width;
	int		text_height;
	short		symbol_x0;
	short		symbol_x1;
	short		symbol_y0;
	short		symbol_y1;
	short		label_x0;
	short		label_x1;
	short		label_y0;
	short		label_y1;
} Placement;

#define PLACEMENT_INIT \
{ \
	{-1,-1,-1,-1,},	/* placement.position */ \
	-1,		/* placement.tag_loc */ \
	-1,		/* placement.text_width */ \
	-1,		/* placement.text_height */ \
	-1,		/* placement.symbol_x0 */ \
	-1,		/* placement.symbol_x1 */ \
	-1,		/* placement.symbol_y0 */ \
	-1,		/* placement.symbol_y1 */ \
	-1,		/* placement.label_x0 */ \
	-1,		/* placement.label_x1 */ \
	-1,		/* placement.label_y0 */ \
	-1,		/* placement.label_y1 */ \
}

typedef struct
{
	MapPlotEllipse	ellipse;
	int		npts;
	double		*lon, *lat;
	SegArray 	s;
	int		type;	/* MAP_INPUT or MAP_ASSOC */
	int		src_id;
	Boolean		changed;
} MapEllipse;

#define MAP_ELLIPSE_INIT \
{ \
	{ \
		0,				/* id */ \
		NULL,				/* label */ \
		-999., -999.,			/* lat, lon */ \
		-1., -1., -1.,			/* sminax, smajax, strike */ \
		LINE_INFO_INIT,			/* line */ \
	},					/* ellipse */ \
	0,					/* npts */ \
	(double *)NULL,				/* lon */ \
	(double *)NULL,				/* lat */ \
	SEG_ARRAY_INIT,				/* s */ \
	MAP_INPUT,				/* type */ \
	0,					/* src_id */ \
	True,					/* changed */ \
}

typedef struct
{
	MapPlotArc	arc;
	int		npts;
	double		*lon, *lat;
	SegArray 	s;
	int		type;	/* MAP_INPUT, MAP_MEASURE, or MAP_ASSOC */
	int		sta_id;
	int		src_id;
	Boolean		changed;
} MapArc;

#define MAP_ARC_INIT \
{ \
	{ \
		0,				/* id */ \
		NULL,				/* label */ \
		-999., -999.,			/* lat, lon */ \
		-1., -1.,			/* del, az */ \
		LINE_INFO_INIT,			/* line */ \
	},					/* arc */ \
	0,					/* npts */ \
	(double *)NULL,				/* lon */ \
	(double *)NULL,				/* lat */ \
	SEG_ARRAY_INIT,				/* s */ \
	MAP_INPUT,				/* type */ \
	0,					/* sta_id */ \
	0,					/* src_id */ \
	True,					/* changed */ \
}

typedef struct
{
	MapPlotDelta	delta;
	int		npts;
	double		*lon, *lat;
	double		dist;
	SegArray 	s;
	int		type;	/* MAP_INPUT, MAP_MEASURE, or MAP_ASSOC */
	int		sta_id;
	int		src_id;
	Boolean		changed;
} MapDelta;

#define MAP_DELTA_INIT \
{ \
	{ \
		0,				/* id */ \
		NULL,				/* label */ \
		-999., -999.,			/* lat, lon */ \
		-1.,				/* del */ \
		LINE_INFO_INIT,			/* line */ \
	},					/* delta */ \
	0,					/* npts */ \
	(double *)NULL,				/* lon */ \
	(double *)NULL,				/* lat */ \
	0.,					/* dist */ \
	SEG_ARRAY_INIT,				/* s */ \
	MAP_INPUT,				/* type */ \
	0,					/* sta_id */ \
	0,					/* src_id */ \
	True,					/* changed */ \
}

typedef struct
{
	MapPlotStation	station;
	Placement	placement;
	Boolean		visible;
} MapStation;

#define MAP_STATION_INIT \
{ \
	{ \
		0,				/* station.id */ \
		NULL,				/* station.label */ \
		-999.,-999.,			/* station.lat, sta.lon */ \
		0,				/* station.tag_loc */ \
		SYMBOL_INFO_INIT,		/* station.sym */ \
	},					/* station */ \
	PLACEMENT_INIT,		/* placement */ \
	True,			/* visible */ \
}

typedef struct
{
	MapPlotSource	source;
	Boolean		visible;
	int		ell_id;
} MapSource;

#define MAP_SOURCE_INIT \
{ \
	{ \
		0,			/* id */ \
		0,			/* orid */ \
		NULL,			/* label */ \
		-999., -999., -999.,	/* lat, lon, depth */ \
		-9999999999.999, 	/* time */ \
		-1., -1., -1.,		/* smajax, sminax, strike */ \
		0,			/* tag_loc */ \
		SYMBOL_INFO_INIT,	/* sym */ \
	},		/* source */ \
	True,				/* visible */ \
	0, 				/* ell_id */ \
}

typedef struct
{
	int		shape_theme_id;
	int		image_theme_id;
	MapPlotTheme	*theme;
	int		shape_index;
	Widget		info2;
	Boolean		selected;
	int		image_ix;
	int		image_iy;
	double		image_x;
	double		image_y;
	double		image_z;
} MapCursorCallbackStruct;

#define MAP_SYMBOL_SELECT	1
#define MAP_SYMBOL_ENTER	2
#define MAP_SYMBOL_GROUP_SELECT	3
#define MAP_SYMBOL_GROUP_ENTER	4
#define MAP_SYMBOL_CTRL_SELECT	5

typedef struct
{
	int		reason; /* MAP_SYMBOL_GROUP_SELECT,
				 * MAP_SYMBOL_SELECT,
				 * MAP_SYMBOL_GROUP_ENTER
				 * MAP_SYMBOL_ENTER
				 */
	Boolean		selected;
	int		id;
	char		label[500];
	MapPlotSymbol	symbol;
} MapSymbolCallbackStruct;

typedef struct
{
	char	letter;
	int	zone;
} UtmStruct;

typedef struct
{
	double radius;
	double azimuth;
	double del_radius;
	double del_azimuth;
	double avg_radius;
	double avg_azimuth;
} PolarSelectStruct;

int MapPlotAddImage(MapPlotWidget w, char *label, int nx, int ny, float *x,
			float *y, float *z, float no_data_flag,
			int mode, MapPlotTheme *theme, Boolean display);
int MapPlotAddImageCopy(MapPlotWidget w, char *label, Matrx *m, int mode,
			MapPlotTheme *theme, Boolean display);
void MapPlotClearArcs(MapPlotWidget w, char *label, Boolean redisplay);
void MapPlotClearDeltas(MapPlotWidget w, char *label, Boolean redisplay);
int MapPlotGetCrosshairs(MapPlotWidget w, double **lat, double **lon);
void MapMeasureDeleteAll(MapPlotWidget w);
void MapMeasureDelete(MapPlotWidget w, int id);
int MapPlotGetMeasurements(MapPlotWidget w, MapMeasure **m);
void MapPlotSelectStation(MapPlotWidget w, char *name, Boolean selected,
			Boolean callbacks, Boolean redisplay);
void MapPlotSelectSource(MapPlotWidget w, int id, Boolean selected,
			Boolean callbacks, Boolean redisplay);
void MapPlotRotation(MapPlotWidget w);
void MapPlotRotateToSta(MapPlotWidget w);
void MapPlotRotateToStation(MapPlotWidget w, char *sta);
void MapPlotRotateToOrig(MapPlotWidget w);
void MapPlotRotate(MapPlotWidget w, double lon, double lat, Boolean redisplay);
Boolean MapPlotGetCoordinates(MapPlotWidget w, double *lat, double *lon,
			double *delta, double *azimuth);
Boolean MapPlotPositionCrosshair(MapPlotWidget w, double lat, double lon);
Boolean MapPlotPositionCrosshair2(MapPlotWidget w, double lat, double lon,
			bool callback);
Boolean MapPlotPositionCrosshairDA(MapPlotWidget w, double lat, double lon,
			double delta, double azimuth);

int MapPlotAddStation(MapPlotWidget w, MapPlotStation *station,
			Boolean redisplay);
int MapPlotGetStations(MapPlotWidget w, MapPlotStation **sta);
int MapPlotAddSource(MapPlotWidget w, MapPlotSource *source, Boolean redisplay);
int MapPlotGetSources(MapPlotWidget w, MapPlotSource **src);
int MapPlotAddLine(MapPlotWidget w, MapPlotLine *line, Boolean display);
int MapPlotAddTheme(MapPlotWidget w, MapPlotTheme *theme, Boolean redisplay);
int MapPlotGetLines(MapPlotWidget w, MapPlotLine **line);
int MapPlotAddSymbols(MapPlotWidget w, int nsymbols, MapPlotSymbol *symbol,
			Boolean redisplay);
int MapPlotGetSymbols(MapPlotWidget w, MapPlotSymbol **symbols);
int MapPlotAddSymbolGroup(MapPlotWidget w, MapPlotSymbolGroup *sym_grp,
			Boolean redisplay);
int MapPlotAddPolygon(MapPlotWidget w, MapPlotPolygon *poly, Boolean redisplay);
int MapPlotAddRectangle(MapPlotWidget w, MapPlotRectangle *rect,
			Boolean redisplay);
int MapPlotGetSymbolGroups(MapPlotWidget w, MapPlotSymbolGroup **sym_grp);
int MapPlotAddEllipse(MapPlotWidget w, MapPlotEllipse *ellipse,
			Boolean redisplay);
int MapPlotGetEllipses(MapPlotWidget w, MapPlotEllipse **ellipse);
int MapPlotAddArc(MapPlotWidget w, MapPlotArc *arc, Boolean redisplay);
int MapPlotGetStaArc(MapPlotWidget w, char *sta, char *label, MapPlotArc *arc);
void MapPlotDisplay(MapPlotWidget w, int num_ids, int *id, int display,
			Boolean redisplay);
void MapPlotDisplayArcs(MapPlotWidget w, char *label, int display);
void MapPlotDisplayDeltas(MapPlotWidget w, char *label, int display);
void MapPlotDisplayEllipses(MapPlotWidget w, char *label, int display);
void MapPlotDisplayStations(MapPlotWidget w, char *label, int display);
void MapPlotDisplaySources(MapPlotWidget w, char *label, int display);
int MapPlotGetStaDelta(MapPlotWidget w, char *sta, char *label,
			MapPlotDelta *delta);
int MapPlotAssoc(MapPlotWidget w, int assoc_id, int id);
int MapPlotGetArcs(MapPlotWidget w, MapPlotArc **arc);
int MapPlotAddDelta(MapPlotWidget w, MapPlotDelta *delta, Boolean redisplay);
int MapPlotGetDeltas(MapPlotWidget w, MapPlotDelta **delta);
int MapPlotDelete(MapPlotWidget w, int id, Boolean redisplay);
int MapPlotChange(MapPlotWidget w, MapObject *obj, Boolean redisplay);
void MapPlotUpdate(MapPlotWidget w);
void MapPlotCircles(MapPlotWidget w, MapCircle *circle, int ncircles);
Boolean MapPlotAssocSta(MapPlotWidget w, int src_id, int nsta, int *sta_id,
			Boolean delet, Boolean redisplay);
Widget MapPlotCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);
void MapChangeStationColor(MapPlotWidget w, Pixel color, char *sta);
void MapPlotChangeStationColor(MapPlotWidget w, int id, Pixel fg,
			Boolean redisplay);
void MapPlotChangeImage(MapPlotWidget w, int theme_id, float *z,
			Boolean redisplay);
int MapPlotDisplayShape(MapPlotWidget w, int theme_id, int shape_index,
			Boolean display_shape, Boolean redisplay);
int MapPlotDisplayTheme(MapPlotWidget w, int theme_id, Boolean display_theme);
void MapPlotThemeCursor(MapPlotWidget w, int theme_id, Boolean on);
int MapPlotThemeBndry(MapPlotWidget w, int theme_id, Boolean bndry);
int MapPlotThemeFill(MapPlotWidget w, int theme_id, Boolean fill);
void MapPlotSetShapeSelected(MapPlotWidget w, int theme_id, vector<int> ishape);
int MapPlotThemeBndryColor(MapPlotWidget w, int theme_id,
			const char *color_name, Boolean redisplay);
int MapPlotThemeFillColor(MapPlotWidget w, int theme_id, const char *color_name,
			Boolean redisplay);
/*
int MapPlotShapeLabels(MapPlotWidget w, int theme_id, int nlab,
		       const char **labels)
*/
int MapPlotShapeLabels(MapPlotWidget w, int theme_id,
			vector<const char *> &labels);
int MapPlotDisplayShapeLabels(MapPlotWidget w, int theme_id, Boolean display);
int MapPlotThemeOnOffDelta(MapPlotWidget w, int theme_id, double on_delta,
			double off_delta, Boolean display);
int MapPlotShapeColor(MapPlotWidget w, int theme_id, int num, double *values,
			int num_bounds, double *bounds);
int MapPlotThemeSymbolType(MapPlotWidget w, int theme_id, int symbol_type);
int MapPlotThemeSymbolSize(MapPlotWidget w, int theme_id, int symbol_size);
Boolean MapPlotShapeIsDisplayed(MapPlotWidget w, int theme_id, int shape_index);
int MapPlotThemeColorScale(MapPlotWidget w, int theme_id,
			ColorScale *color_scale);
int MapPlotThemeColorBar(MapPlotWidget w, int theme_id);
void MapPlotThemeColorBarOff(MapPlotWidget w);
int MapPlotThemeLevelDown(MapPlotWidget w, int theme_id);
int MapPlotThemeLevelUp(MapPlotWidget w, int theme_id);
void MapPlotClearOverlays(MapPlotWidget w, Boolean redisplay);
void MapPlotClearSymbols(MapPlotWidget w, Boolean redisplay);
void MapPlotSetAppContext(MapPlotWidget w, XtAppContext app);
void MapPlotLinkMap(MapPlotWidget w, MapPlotWidget utm_map);
void MapPlotUnlinkMap(MapPlotWidget w, MapPlotWidget utm_map);
void MapPlotClear(MapPlotWidget w);
Boolean MapPlotDisplayUTM(MapPlotWidget w, char letter, int zone);
Boolean MapPlotSetLimits(MapPlotWidget w, double lonmin, double lonmax,
		double latmin, double latmax);
void MapPlotGetLimits(MapPlotWidget w, double *lonmin, double *lonmax,
		double *latmin, double *latmax);
void MapPlotSetPolarMaxRadius(MapPlotWidget w, double max_radius);
double MapPlotGetPolarMaxRadius(MapPlotWidget w);
void MapPlotSelectSymbol(MapPlotWidget w, MapPlotSymbol *s, Boolean redisplay);
void MapPlotSetPolarMargin(MapPlotWidget w, double polar_margin);
void MapPlotDisplayPolarSelection(MapPlotWidget w, Boolean display,
		double azimuth, double del_azimuth, double radius,
		double del_radius, Boolean do_callback);
void MapPlotPositionPolarSelection(MapPlotWidget w, double azimuth,
		double del_azimuth, double radius, double del_radius,
		Boolean do_callback);
Boolean MapPlotGetPolarSelection(MapPlotWidget w, double *azimuth,
		double *del_azimuth, double *radius, double *del_radius);

#endif	/* MAPPLOT_H_ */
