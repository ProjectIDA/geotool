/** \file MapPlot.cpp
 *  \brief Defines widget MapPlot.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      MapPlot Widget -- widget for plotting continent outlines,
 *		src-rec locations, distance circles, azimuth lines, etc.
 *
 * FILES
 *      MapPlot.h
 *
 * BUGS
 *
 * NOTES
 *
 * AUTHOR
 *		Ivan Henson	1/92
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>

#ifdef HAVE_GSL
#include "gsl/gsl_interp.h"
#endif

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/XmAll.h>

#include "widget/MapPlotP.h"
extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
#include "libgmath.h"
#include "libstring.h"
}
#include "widget/AxesClass.h"

#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962  /* pi/4 */
#endif

#define LON_MIN	-180.
#define LON_MAX	 180.
#define LAT_MIN	 -90.
#define LAT_MAX	  90.

#define BAR_WIDTH 10

#define XtRMapInt      (char *)"MapInt"

#define	offset(field)		XtOffset(MapPlotWidget, map_plot.field)
static XtResource	resources[] = 
{
	{XtNprojection, XtCProjection, XtRMapInt, sizeof(int),
		offset(projection), XtRImmediate,
		(XtPointer)MAP_LINEAR_CYLINDRICAL},
	{XtNdisplayPaths, XtCDisplayPaths, XtRMapInt, sizeof(int),
		offset(display_paths), XtRImmediate, (XtPointer)MAP_ON},
	{XtNdisplaySourceEllipses, XtCDisplaySourceEllipses, XtRMapInt,
		sizeof(int), offset(display_source_ellipses), XtRImmediate,
		(XtPointer)MAP_ON},
	{XtNmapDisplayGrid, XtCMapDisplayGrid, XtRBoolean, sizeof(Boolean),
		offset(map_display_grid), XtRString,(XtPointer) "False"},
	{XtNdisplayCircles, XtCDisplayCircles, XtRMapInt, sizeof(int),
		offset(display_circles), XtRImmediate, (XtPointer)MAP_ON},
	{XtNdisplayStationTags, XtCDisplayStationTags, XtRBoolean,
		sizeof(Boolean), offset(display_station_tags), XtRString,
		(XtPointer)"False"},
	{XtNdisplaySourceTags, XtCDisplaySourceTags, XtRBoolean,sizeof(Boolean),
		offset(display_source_tags), XtRString,(XtPointer) "False"},
	{XtNdisplayMapCil, XtCDisplayMapCil, XtRBoolean, sizeof(Boolean),
		offset(display_map_cil), XtRString, (XtPointer)"True"},
	{XtNdisplayMapBdy, XtCDisplayMapBdy, XtRBoolean, sizeof(Boolean),
		offset(display_map_bdy), XtRString,(XtPointer) "True"},
	{XtNmovableStations, XtCMovableStations, XtRBoolean, sizeof(Boolean),
		offset(movable_stations), XtRString,(XtPointer) "False"},
	{XtNmovableSources, XtCMovableSources, XtRBoolean, sizeof(Boolean),
		offset(movable_sources), XtRString,(XtPointer) "True"},
	{XtNmeasure, XtCMeasure, XtRBoolean, sizeof(Boolean),
		offset(measure), XtRString,(XtPointer) "False"},
	{XtNrotateOnZoom, XtCRotateOnZoom, XtRBoolean, sizeof(Boolean),
		offset(rotate_on_zoom), XtRString,(XtPointer) "True"},
	{XtNapplyAntiAlias, XtCApplyAntiAlias, XtRBoolean, sizeof(Boolean),
		offset(apply_anti_alias), XtRString, (XtPointer)"False"},
	{XtNlonMin, XtCLonMin, XtRDouble, sizeof(double),
		offset (lon_min), XtRString, (XtPointer)"-180."},
	{XtNlonMax, XtCLonMax, XtRDouble, sizeof(double),
		offset (lon_max), XtRString, (XtPointer)"180."},
	{XtNlatMin, XtCLatMin, XtRDouble, sizeof(double),
		offset (lat_min), XtRString, (XtPointer)"-90."},
	{XtNlatMax, XtCLatMax, XtRDouble, sizeof(double),
		offset (lat_max), XtRString, (XtPointer)"90."},
	{XtNmercatorMaxLat, XtCMercatorMaxLat, XtRDouble, sizeof(double),
		offset(mercator_max_lat), XtRString, (XtPointer)"75."},
	{XtNmapWaterColor, XtCMapWaterColor, XtRPixel, sizeof(Pixel),
		offset(map_water_color), XtRString, (XtPointer)"SkyBlue"},
	{XtNlabelFont, XtCLabelFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(label_font), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*"},
	{XtNselectStationCallback, XtCSelectStationCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_station_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNselectCircleCallback, XtCSelectCircleCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_circle_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNdragStationCallback, XtCDragStationCallback, XtRCallback,
		sizeof(XtCallbackList), offset(drag_station_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNselectSourceCallback, XtCSelectSourceCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_source_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNdragSourceCallback, XtCDragSourceCallback, XtRCallback,
		sizeof(XtCallbackList), offset(drag_source_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNmapMeasureCallback, XtCMapMeasureCallback, XtRCallback,
		sizeof(XtCallbackList), offset(map_measure_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNselectArcCallback, XtCSelectArcCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_arc_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNcursorMotionCallback, XtCCursorMotionCallback, XtRCallback,
		sizeof(XtCallbackList), offset(cursor_motion_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNshapeSelectCallback, XtCShapeSelectCallback, XtRCallback,
		sizeof(XtCallbackList), offset(shape_select_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNsymbolSelectCallback, XtCSymbolSelectCallback, XtRCallback,
		sizeof(XtCallbackList), offset(symbol_select_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNsymbolInfoCallback, XtCSymbolInfoCallback, XtRCallback,
		sizeof(XtCallbackList), offset(symbol_info_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNutmCallback, XtCUtmCallback, XtRCallback,
		sizeof(XtCallbackList), offset(utm_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNpolarSelectCallback, XtCPolarSelectCallback, XtRCallback,
		sizeof(XtCallbackList), offset(polar_select_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNselectBarCallback, XtCSelectBarCallback, XtRCallback,
		sizeof(XtCallbackList), offset(select_bar_callbacks),
		XtRCallback, (XtPointer)NULL},
	{XtNcontourLabelSize, XtCContourLabelSize, XtRInt, sizeof(int),
		offset(contour_label_size), XtRImmediate, (caddr_t)5},
	{XtNcontourLabelColor, XtCContourLabelColor, XtRPixel, sizeof(Pixel),
		offset(contour_label_fg), XtRString,(XtPointer) "red"},
	{XtNdataColor, XtCDataColor, XtRPixel, sizeof(Pixel),
		offset(data_fg), XtRString, (XtPointer)"medium blue"},
	{XtNautoInterval, XtCAutoInterval, XtRBoolean, sizeof(Boolean),
		offset(auto_interval), XtRString,(XtPointer) "True"},
	{XtNcontourInterval, XtCContourInterval, XtRDouble, sizeof(double),
		offset(contour_interval), XtRString, (caddr_t)"0."},
	{XtNcontourMin, XtCContourMin, XtRDouble, sizeof(double),
		offset(contour_min), XtRString, (caddr_t)"0."},
	{XtNcontourMax, XtCContourMax, XtRDouble, sizeof(double),
		offset(contour_max), XtRString, (caddr_t)"0."},
	{XtNmarkMax, XtCMarkMax, XtRBoolean, sizeof(Boolean),
		offset(mark_max), XtRString, (XtPointer)"False"},
	{XtNmode, XtCMode, XtRInt, sizeof(int),
		offset(mode), XtRImmediate,(XtPointer) CONTOURS_ONLY},
	{XtNpolarMaxRadius, XtCPolarMaxRadius, XtRDouble, sizeof(double),
		offset(polar_max_radius), XtRString, (caddr_t)"1."},
	{XtNpolarDelAz, XtCPolarDelAz, XtRDouble, sizeof(double),
		offset(polar_del_az), XtRString, (caddr_t)"20."},
	{XtNpolarDelDist, XtCPolarDelDist, XtRDouble, sizeof(double),
		offset(polar_del_dist), XtRString, (caddr_t)"0."},
	{XtNbarVerticalMargin, XtCBarVerticalMargin, XtRInt, sizeof(int),
		offset(bar_vertical_margin), XtRImmediate, (caddr_t)0},

};
#undef offset

typedef struct
{
	int	loop;
	double	lon;
	Boolean	up;
} Longitude;


/* Private functions */
static void MapPlotRedraw(AxesWidget w, int type, double shift);
static void MapPlotHardCopy(AxesWidget w, FILE *fp, DrawStruct *d,
			AxesParm *a, float font_scale, PrintParam *p);
static void XYTransform(AxesWidget w, double x, double y, char *xlab,
			char *ylab);
static void MapPlotResize(AxesWidget w);
extern "C" {
static int sort_lon(const void *A, const void *B);
}

static void CvtStringToMapInt(XrmValuePtr *args, Cardinal *num_args,
			XrmValuePtr fromVal, XrmValuePtr toVal);
static void CvStringToDouble(XrmValuePtr *args, Cardinal *num_args,
				XrmValuePtr fromVal, XrmValuePtr toVal);
static void ClassInitialize(void);
static void Initialize(Widget req, Widget nou);
static void Realize(Widget widget, XtValueMask *valueMask,
			XSetWindowAttributes *attrs);
static void InitImage(MapPlotWidget w);
static Boolean SetValues(MapPlotWidget cur,MapPlotWidget req,MapPlotWidget nou);
static void RedisplayPixmap(MapPlotWidget w, Pixmap pixmap, Window window);
static void RedisplayOverlays(MapPlotWidget w, Window window);
static void DrawSourceTags(MapPlotWidget w, Window window);
static void MapPlotZoom(MapPlotWidget w, XtPointer client_data,
			XtPointer callback_data);
static void OrthoRotate(MapPlotWidget w);
static Boolean RedisplaySymbol(MapPlotWidget w, Window window, double lon,
			double lat, SymbolInfo *sym);
static void RedisplaySymbolGroup(MapPlotWidget w, Window window,
			MapSymbolGroup *s);
static void RedisplayMapSymbol(MapPlotWidget w, Window window,MapPlotSymbol *s);
static void DrawHardSymbols(MapPlotWidget w, FILE *fp, DrawStruct *d,
			MapSymbolGroup *s, Boolean do_color);
static Boolean DrawHardSymbol(MapPlotWidget w, FILE *fp, DrawStruct *d,
			double lon, double lat, SymbolInfo *sym,
			Boolean do_color);
static void HardDrawBar(MapPlotWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
			float font_scale, AxesFontStruct *fonts, ColorScale *c);
static void RedisplayLine(MapPlotWidget w, Window window, LineInfo *line,
			SegArray *s);
static void RedisplayPoly(MapPlotWidget w, Window window, MapPlotPolygon *poly);
static void RedisplayRectangle(MapPlotWidget w, Window window,
			MapPlotRectangle *rect);
static void DrawHardPoly(MapPlotWidget w, FILE *fp, DrawStruct *d,
			MapPlotPolygon *poly, Boolean do_color);
static void DrawCircles(MapPlotWidget w);
static void RedisplayCircles(MapPlotWidget w, Window window);
static void Destroy(Widget w);
static void DrawMapBackground(MapPlotWidget w);
static void DrawLines(MapPlotWidget w, DrawStruct *d);
static void DrawTheme(MapPlotWidget w, DrawStruct *d, MapTheme *t);
static void DrawShapeTheme(MapPlotWidget w, DrawStruct *d, MapTheme *t);
static void DrawImageTheme(MapPlotWidget w, DrawStruct *d, MapTheme *t);
static void DrawThemeImage(MapPlotWidget w, DrawStruct *d, MapTheme *t,
			int *part);
static void DoYCase(MapPlotWidget w, DrawStruct *d, MapTheme *t, MapImage *mi);
static void RedisplayThemes(MapPlotWidget w);
static void RedisplayTheme(MapPlotWidget w, Window window, MapTheme *t);
static void RedisplayShapeTheme(MapPlotWidget w, Window window, MapTheme *t);
static void RedisplayImageTheme(MapPlotWidget w, Window window, MapTheme *t);
static void DrawEllipses(MapPlotWidget w, DrawStruct *d);
static void DrawArcs(MapPlotWidget w, DrawStruct *d);
static void DrawDeltas(MapPlotWidget w, DrawStruct *d);
static void project(MapPlotWidget w, double lon, double lat, double *xp,
			double *yp, double *zp);
static void Btn1Down(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Btn1Motion(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Btn1Up(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Motion(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Measure(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void Delete(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static int WhichCircle(MapPlotWidget w, int cursor_x, int cursor_y);
static int WhichStaSrc(MapPlotWidget w, int cursor_x, int cursor_y,
			Boolean *station);
static int WhichMeasure(MapPlotWidget w, Boolean stasrc, Boolean arcdel,
			int cursor_x, int cursor_y, int *type, int *ic,
			int *delta_id, int *arc_id);
static void DoSelectStation(MapPlotWidget w, MapStation *sta,
			Boolean callbacks, Boolean redisplay);
static void SelectCircle(MapPlotWidget w, int i);
static void DoSelectSource(MapPlotWidget w, MapSource *src, Boolean callbacks,
			Boolean redisplay);
static void DrawLonLat(MapPlotWidget w, DrawStruct *d, int n, double *lon,
			double *lat);
static void DrawLonLatXor(MapPlotWidget	w, DrawStruct *d, int n, double	*lon,
			double *lat);
static void DrawBorder(MapPlotWidget w, DrawStruct *d);
static void DrawGrid(MapPlotWidget w, DrawStruct *d, AxesParm *a,
			int dash_length);
static void grid_x(DrawStruct *d, double x, double ymin, double ymax,
			double del);
static void grid_y(DrawStruct *d, double y, double xmin, double xmax,
			double del);
static void grid_lon(MapPlotWidget w, DrawStruct *d, double lon, double min_lat,
			double max_lat, double del, double *x_lab);
static void grid_lat(MapPlotWidget w, DrawStruct *d, double lat, double min_lon,
			double max_lon, double del, double *y_lab);
static void MapProjection(MapPlotWidget cur, MapPlotWidget nou);
static void RotateToCursor(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static void MapRotate(MapPlotWidget w, double longitude, double latitude);
static void SetZoomZero(MapPlotWidget w);
static void unproject(MapPlotWidget w, double xp, double yp, double *lon,
			double *lat);
static int MapPlotChangeStation(MapPlotWidget w, MapPlotStation *station,
			Boolean redisplay);
static int MapPlotDeleteStation(MapPlotWidget w, int id, Boolean redisplay);
static int MapPlotChangeSource(MapPlotWidget w, MapPlotSource *source,
			Boolean redisplay);
static int MapPlotDeleteSource(MapPlotWidget w, int id, Boolean redisplay);
static int MapPlotDeleteLine(MapPlotWidget w, int j, Boolean redisplay);
static int MapPlotDeleteSymbol(MapPlotWidget w, int j, Boolean redisplay);
static int MapPlotDeleteSymbolGroup(MapPlotWidget w, int j, Boolean redisplay);
static int MapPlotDeletePoly(MapPlotWidget w, int j, Boolean redisplay);
static int MapPlotDeleteRectangle(MapPlotWidget w, int j, Boolean redisplay);
static int MapPlotChangeEllipse(MapPlotWidget w, MapPlotEllipse *ellipse,
			Boolean redisplay);
static int MapPlotDeleteEllipse(MapPlotWidget w, int id, Boolean redisplay);
static void MakeEllipse(MapPlotWidget w, MapEllipse *e);
static int MapPlotChangeArc(MapPlotWidget w, MapPlotArc *arc,Boolean redisplay);
static int MapPlotDeleteArc(MapPlotWidget w, int id, Boolean redisplay);
static void MakeArc(MapPlotWidget w, MapArc *a);
static int MapPlotChangeDelta(MapPlotWidget w, MapPlotDelta *delta,
			Boolean redisplay);
static int MapPlotChangeSymbol(MapPlotWidget w, MapPlotSymbol *sym,
			Boolean redisplay);
static int MapPlotDeleteDelta(MapPlotWidget w, int id, Boolean redisplay);
static int MapPlotDeleteTheme(MapPlotWidget w, int id, Boolean redisplay);
static void MakeDelta(MapPlotWidget w, MapDelta *d);
static void DeleteStation(MapPlotWidget w, int j);
static void DeleteSource(MapPlotWidget w, int j);
static void DeleteArc(MapPlotWidget w, int j);
static void DeleteEllipse(MapPlotWidget w, int j);
static void DeleteDelta(MapPlotWidget w, int j);
static void DeleteTheme(MapPlotWidget w, int i);
static void DrawStationTags(MapPlotWidget w, Window window);
static int CheckLabel(MapPlotWidget w, int orig_x0, int orig_x1, int orig_y0,
			int orig_y1, int ii, int jj);
static void label_position(int center_x, int center_y, int width, int height,
			int position, int half_size, int *x0, int *x1, int *y0,
			int *y1);
static int have_overlap(int ax0, int ax1, int ay0, int ay1, int bx0, int bx1,
			int by0, int by1);
static int inside_limits(int x0, int x1, int y0, int y1, int clipx0, int clipx1,
			int clipy0, int clipy1);
static void PostSegs(MapPlotWidget w, FILE *fp, SegArray *s, Boolean do_color,
			Pixel color, LineInfo *line);
static void DrawHardTag(MapPlotWidget w, FILE *fp, DrawStruct *d,
			MapStation *sta, float font_scale, char *tag_font,
			int tag_fontsize);
static void reduce_lon(double *lon);
static int getMapId(MapPlotPart *mp);
static Boolean ComputeBar(MapPlotWidget w, ColorScale *c);
static Boolean nearArcDel(MapPlotWidget w, int cursor_x, int cursor_y,int dmin);
static Boolean nearStaSrc(MapPlotWidget w, int cursor_x, int cursor_y,int dmin);
/*
static Boolean nearSymbol(MapPlotWidget w, int cursor_x, int cursor_y,int dmin);
*/
static void doMeasure(MapPlotWidget w, XEvent *event);
static void FillBackground(MapPlotWidget w, DrawStruct *d);
static void DrawThemeShape(MapPlotWidget w, DrawStruct *d, MapTheme *t, int i);
static void DrawThemeLCFill(MapPlotWidget w, DrawStruct *d, MapTheme *t,
			int ishape);
static void DrawThemeMercatorFill(MapPlotWidget w, DrawStruct *d, MapTheme *t,
			int ishape);
static void DrawThemeOrthoFill(MapPlotWidget w, DrawStruct *d, MapTheme *t,
			int ishape);
static int WhichShape(MapPlotWidget w, MapTheme *t, int cursor_x, int cursor_y,
			double *dist_min);
static void DisplayShapeSymbol(MapPlotWidget w, Window window, SHPObject *s,
			ThemeDisplay *td, Pixel fg, int sym_type, int sym_size);
static void MapGetLCLongitudes(AxesPart *ax, SHPObject *s, int numlat,
			double *lat, double dlat, Longitude **lon, int *nlon,
			int *sizes);
static void DrawThemeMercatorFill(MapPlotWidget w, DrawStruct *d, MapTheme *t,
			int ishape);
static void MapReverseLoop(SHPObject *s, int loop);
static void MapGetMerLongitudes(MapPlotWidget w, SHPObject *s, int numlat,
			double *lat, double dlat, Longitude **lon, int *nlon,
			int *sizes);
static void DrawThemePts(MapPlotWidget w, DrawStruct *d, MapTheme *t,
			int ishape);
static void DisplayArcShapeLabel(MapPlotWidget w, Window window, GC gc,
			MapTheme *t, int i);
static int WhichThemeShape(MapPlotWidget w, int cursor_x, int cursor_y,
			double x, double y,
			int *jmin, int *imin, int *jmin_image,
			int *ix, int *iy);
static void shadeImageProjected(MapPlotWidget w, XImage *image, DrawStruct *d,
	Pixel no_data_color, int num_colors, Pixel *colors, double *lines,
	Matrx *a, int *x0, int *y0, int *xwidth, int *yheight);
static void MapPlotSizeImage(MapPlotWidget w);
static void MapPlotDrawThemeColorBar(MapPlotWidget w, ColorScale *c);
static void DisplayThemeSymbols(MapPlotWidget w, Window window, MapTheme *t,
			int ishape);
static void DisplayShapeSymbolLabel(MapPlotWidget w, Window window,
			SHPObject *s, ThemeDisplay *td, GC gc, int sym_size);
static void DisplayThemePolygonLabel(MapPlotWidget w, Window window, GC gc,
				ThemeDisplay *td);
static void DrawPolarLonLat(MapPlotWidget w, DrawStruct *d, MapPlotTheme *theme,
			int n, double *r, double *theta);
static void MapGetLCLongPolar(AxesPart *ax, MapPlotTheme *theme, SHPObject *s,
			int numlat, double *lat, double dlat, Longitude **lon,
			int *nlon, int *sizes);
static void MapGetMerLonPolar(MapPlotWidget w, MapPlotTheme *theme,
			SHPObject *s, int numlat, double *lat, double dlat,
			Longitude **lon, int *nlon, int *sizes);
static void MapGetOrthoLon(MapPlotWidget w, SHPObject *s, int numlat,
			double *lat, double dlat, Longitude **lon, int *nlon,
			int *sizes);
static void MapGetOrthoLonPolar(MapPlotWidget w, MapPlotTheme *theme,
			SHPObject *s, int numlat, double *lat, double dlat,
			Longitude **lon, int *nlon, int *sizes);
static void DrawThemeGrid(MapPlotWidget w, DrawStruct *d, MapTheme *t);
static Boolean gridLon(MapPlotWidget w, DrawStruct *d, double longitude,
			double min_lat, double max_lat, double del,
			PolarLabel *p, double c[3][3], double *max_theta);
static Boolean gridLat(MapPlotWidget w, DrawStruct *d, double latitude,
			double min_lon, double max_lon, double del,
			PolarLabel *p, double c[3][3]);
static void MapGetPolar(AxesPart *ax, SHPObject *s, int numlat, double *lat,
			double dlat, Longitude **lon, int *nlon, int *sizes);
static Boolean gridPolarDist(DrawStruct *d, double azimuth, double maxr,
			double del, PolarLabel *p);
static Boolean gridPolarAz(MapPlotWidget w, DrawStruct *d, double r,
			double min_az, double max_az, double del,PolarLabel *p);
static void DrawPolarGrid(MapPlotWidget w, DrawStruct *d, int dash_length);
static Boolean getPolarDistRange(DrawStruct *d, double maxr, double *rmin,
			double *rmax);
static int WhichSymbol(MapPlotWidget w, int cursor_x, int cursor_y, int max,
			int *imin, int *jmin, int nearDist, Boolean *near);
static void polarGridLabels(MapPlotWidget w, Window window, int numaz,
			PolarLabel *az, int numdist, PolarLabel *dist);
static void symbolInfo(XtPointer client_data, XtIntervalId id);
static void destroyInfoPopup(MapPlotWidget w, Boolean reset);
static void HardDrawTheme(MapPlotWidget w, FILE *fp, DrawStruct *d, MapTheme *t,
			float font_scale);
static void HardMapSegs(FILE *fp, SegArray *s);
static void HardDrawImageTheme(MapPlotWidget w, FILE *fp, DrawStruct *d,
			MapTheme *t);
static void HardShadeImageProjected(MapPlotWidget w, FILE *fp, DrawStruct *d,
			Pixel no_data_color, int num_colors, double *lines,
			Matrx *a, int *x0, int *y0, int *xwidth, int *yheight);
static void HardDrawThemeImage(MapPlotWidget w, FILE *fp, DrawStruct *d,
			MapTheme *t, int *part);
static void HardDoYCase(MapPlotWidget w, FILE *fp, DrawStruct *d, MapTheme *t,
			MapImage *mi);
static void HardPolarGridLabels(MapPlotWidget w, FILE *fp, DrawStruct *d,
		AxesParm *a, float font_scale, int numaz, PolarLabel *az,
		int numdist, PolarLabel *dist);
static void CrosshairCB(Widget widget, XtPointer client,
		XtPointer callback_data);
static void drawUTMGrid(MapPlotWidget w, Window window);
static void utmGrid(MapPlotWidget w, Window window);
static void utmNearGrid(MapPlotWidget w, Window window);
static void highlightUTM(MapPlotWidget w, double x, double y, char *utm_label);
static int getUTMZone(double x, double y);
static char getUTMLetter(double lat);
static void LeaveWindow(MapPlotWidget w, XEvent *event, String *params,
			Cardinal *num_params);
static Boolean selectUTMCell(MapPlotWidget w);
static void utmMode(MapPlotWidget w);
static Boolean getUTMCell(char letter, int zone, double *lon_min,
			double *lon_max, double *lat_min, double *lat_max);
static void llToUtm(MapPlotPart *mp, double lon, double lat, double *xp,
			double *yp, double *zp);
static void utmToLL(MapPlotPart *mp, double UTMEasting, double UTMNorthing,
			double *lon, double *lat);
static void utmDelta(MapPlotWidget w, double lon, double lat, int cursor_x,
			int cursor_y, double *del);
static void utmAz(MapPlotWidget w, double lon, double lat, int cursor_x,
			int cursor_y, double *az);
static void DrawDelta(MapPlotWidget w, DrawStruct *d, MapDelta *del);
static void DrawArc(MapPlotWidget w, DrawStruct *d, MapArc *arc);
static Boolean nearPolarSelection(MapPlotWidget w, int cursor_x, int cursor_y,
			int dmin, int *side);
static void drawPolarSelect(MapPlotWidget w, double *radius, double *azimuth);
static void findPolarSelectSegs(MapPlotWidget w, double *lon, double *lat);
static void doPolarSelectCallback(MapPlotWidget w);

static char	defTrans[] =
"~Shift ~Ctrl<Btn2Down>:Zoom() \n\
  <Btn2Up>:		Zoom() \n\
  <Btn2Motion>:		Zoom() \n\
 ~Ctrl Shift<Btn2Down>:	ZoomBack() \n\
  ~Shift<Key>c:		AddCrosshair() \n\
  ~Shift<Key>l:		AddDoubleLine() \n\
  Shift<Key>l:		AddLine() \n\
  ~Shift<Key>d:		DeleteCursor(one) \n\
  Shift<Key>d:		DeleteCursor(all) \n\
  ~Shift<Key>h:		ZoomHorizontal() \n\
  ~Shift<Key>v:		ZoomVertical() \n\
  ~Shift<Key>z:		ZoomPercentage() \n\
  Shift<Key>h:		UnzoomHorizontal() \n\
  Shift<Key>v:		UnzoomVertical() \n\
  Shift<Key>z:		UnzoomPercentage() \n\
  ~Shift<Key>f:		Page(vertical DOWN) \n\
  ~Shift<Key>b:		Page(vertical UP) \n\
  Shift<Key>f:		Page(vertical down) \n\
  Shift<Key>b:		Page(vertical up) \n\
  ~Shift<Key>t:		Page(horizontal DOWN) \n\
  ~Shift<Key>r:		Page(horizontal UP) \n\
  Shift<Key>t:		Page(horizontal down) \n\
  Shift<Key>r:		Page(horizontal up) \n\
  <KeyPress>:		KeyCommand()\n\
~Shift ~Ctrl<Btn1Down>:	Btn1Down()\n\
 <Btn1Motion>:		Btn1Motion()\n\
 <Btn1Up>:		Btn1Up()\n\
~Shift Ctrl<Btn1Down>:	Btn1Down(move)\n\
~Shift Ctrl<Btn2Down>:	Rotate()\n\
<Motion>:		Motion() \n\
~Shift ~Ctrl<Btn3Down>:	Measure(multiple)\n\
~Ctrl Shift<Btn3Down>:	Measure(multiple)\n\
~Shift Ctrl<Btn3Down>:	Delete()\n\
<LeaveWindow>:		LeaveWindow()";

/*** for GAgrid
static char	defTrans[] = "#override\n\
!<Btn1Down>:		Move(select)\n\
 <Btn1Motion>:		Move()\n\
 <Btn1Up>:		Move()\n\
!Ctrl<Btn1Down>:	Move(circle)\n\
!Shift<Btn1Down>:	Move(move)\n\
!Ctrl<Btn2Down>:	Rotate()";
***/
/***	for editing the map
static char	defTrans[] = "#override\n\
!<Btn1Down>:		MovePoint(move)\n\
 <Btn1Motion>:		MovePoint(move)\n\
 <Btn1Up>:		MovePoint(move)\n\
!Ctrl<Btn1Down>:	MovePoint(find)\n\
!<Btn3Down>:		MovePoint(nou)\n\
!Ctrl<Btn3Down>:	MovePoint(finish)";
***/


static XtActionsRec	actionsList[] =
{
	{(char *)"Zoom",		(XtActionProc)_AxesZoom},
	{(char *)"ZoomBack",		(XtActionProc)_AxesZoomBack},
	{(char *)"Magnify",		(XtActionProc)_AxesMagnify},
	{(char *)"AddCrosshair",	(XtActionProc)_AxesAddCrosshair},
	{(char *)"AddLine",		(XtActionProc)_AxesAddLine},
	{(char *)"AddDoubleLine",	(XtActionProc)_AxesAddDoubleLine},
	{(char *)"DeleteCursor",	(XtActionProc)_AxesDeleteCursor},
	{(char *)"UnzoomPercentage",	(XtActionProc)_AxesUnzoomPercentage},
	{(char *)"UnzoomHorizontal",	(XtActionProc)_AxesUnzoomHorizontal},
	{(char *)"UnzoomVertical",	(XtActionProc)_AxesUnzoomVertical},
	{(char *)"ZoomPercentage",	(XtActionProc)_AxesZoomPercentage},
	{(char *)"ZoomHorizontal",	(XtActionProc)_AxesZoomHorizontal},
	{(char *)"ZoomVertical",	(XtActionProc)_AxesZoomVertical},
	{(char *)"Page",		(XtActionProc)AxesPage},
	{(char *)"KeyCommand",		(XtActionProc)_AxesKeyCommand},

	{(char *)"Btn1Down",		(XtActionProc)Btn1Down},
	{(char *)"Btn1Motion",		(XtActionProc)Btn1Motion},
	{(char *)"Btn1Up",		(XtActionProc)Btn1Up},
	{(char *)"Motion",		(XtActionProc)Motion},
	{(char *)"Rotate",		(XtActionProc)RotateToCursor},
	{(char *)"Measure",		(XtActionProc)Measure},
	{(char *)"Delete",		(XtActionProc)Delete},
	{(char *)"LeaveWindow",		(XtActionProc)LeaveWindow},
};

MapPlotClassRec	mapPlotClassRec = 
{
	{	/* core fields */
	(WidgetClass)(&axesClassRec),	/* superclass */
	(char *)"MapPlot",		/* class_name */
	sizeof(MapPlotRec),		/* widget_size */
	ClassInitialize,		/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
	actionsList,			/* actions */
	XtNumber(actionsList),		/* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
        NULLQUARK,		        /* xrm_class */
	TRUE,				/* compress_motion */
	TRUE,				/* compress_exposure */
	TRUE,				/* compress_enterleave */
	TRUE,				/* visible_interest */
	Destroy,			/* destroy */
	XtInheritResize,		/* resize */
	XtInheritExpose,		/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
	defTrans,			/* tm_table */
	NULL,				/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL				/* extension */
	},
	{	/* composite_class fields */
	XtInheritGeometryManager,	/* geometry_manager */
	XtInheritChangeManaged,		/* change_managed */
	XtInheritInsertChild,		/* insert_child */
	XtInheritDeleteChild,		/* delete_child */
	NULL,				/* extension */
	},
	{	/* AxesClass fields */
	0,			/* empty */
	},
};

WidgetClass mapPlotWidgetClass = (WidgetClass)&mapPlotClassRec;

static double	pi = 3.14159265357989, half_pi, two_pi;
static double	rad;

static void
CvtStringToMapInt(XrmValuePtr *args, Cardinal *num_args, XrmValuePtr fromVal,
			XrmValuePtr toVal)
{
	char		*s = fromVal->addr;	/* string to convert */
	static int	val;			/* returned value */

	if(s == NULL)					val = 0;
	else if(!strcasecmp(s, "MAP_LINEAR_CYLINDRICAL"))
		val = MAP_LINEAR_CYLINDRICAL;
	else if(!strcasecmp(s, "MAP_CYLINDRICAL_EQUAL_AREA"))
		val = MAP_CYLINDRICAL_EQUAL_AREA;
	else if(!strcasecmp(s, "MAP_ORTHOGRAPHIC"))	val = MAP_ORTHOGRAPHIC;
	else if(!strcasecmp(s, "MAP_AZIMUTHAL_EQUIDISTANT"))
		val = MAP_AZIMUTHAL_EQUIDISTANT;
	else if(!strcasecmp(s, "MAP_AZIMUTHAL_EQUAL_AREA"))
		val = MAP_AZIMUTHAL_EQUAL_AREA;
	else if(!strcasecmp(s, "MAP_MERCATOR"))		val = MAP_MERCATOR;
	else if(!strcasecmp(s, "MAP_POLAR"))		val = MAP_POLAR;
	else if(!strcasecmp(s, "MAP_UTM")) 		val = MAP_UTM;
	else if(!strcasecmp(s, "MAP_UTM_NEAR")) 	val = MAP_UTM_NEAR;
	else if(!strcasecmp(s, "MAP_LOCKED_OFF"))	val = MAP_LOCKED_OFF;
	else if(!strcasecmp(s, "MAP_OFF"))		val = MAP_OFF;
	else if(!strcasecmp(s, "MAP_SELECTED_ON"))	val = MAP_SELECTED_ON;
	else if(!strcasecmp(s, "MAP_ON"))		val = MAP_ON;
	else if(!strcasecmp(s, "MAP_LOCKED_ON"))	val = MAP_LOCKED_ON;
	else if(!strcasecmp(s, "MAP_NONE"))		val = MAP_NONE;
	else if(!strcasecmp(s, "MAP_LESS"))		val = MAP_LESS;
	else if(!strcasecmp(s, "MAP_MORE"))		val = MAP_MORE;
	else if(!strcasecmp(s, "MAP_ALL"))		val = MAP_ALL;

	else val = 0;

	toVal->size = sizeof(val);
	toVal->addr = (char*)(XtPointer)&val;
}

static void
ClassInitialize(void)
{
	MapPlotClassPart *mc = &mapPlotClassRec.map_plot_class;

	mc->have_rivers = False;

	XtAddConverter (XtRString, XtRDouble, (XtConverter)CvStringToDouble,
		(XtConvertArgList) NULL, (Cardinal) 0);

	XtAddConverter (XtRString, XtRMapInt, (XtConverter)CvtStringToMapInt,
		(XtConvertArgList) NULL, (Cardinal) 0);
}

static void
Initialize(Widget w_req, Widget w_new)
{
	MapPlotWidget	nou = (MapPlotWidget)w_new;
	MapPlotPart *mp = &nou->map_plot;
	AxesPart *ax = &nou->axes;
	int	i;
	/* XtTranslations translations; */

	rad = pi/180.;
	half_pi = pi/2.;
	two_pi = 2.*pi;


	ax->zoom = 0;
	for(i = 0; i < MAX_ZOOM; i++) {
		ax->x1[i] = 0.0;
		ax->x2[i] = 0.0;
		ax->y1[i] = 0.0;
		ax->y2[i] = 0.0;
	}
	ax->zoom = 0;
	SetZoomZero(nou);
	mp->phi = 0.;
	mp->theta = 0.;
	MapRotate(nou, 0., 90.);

	/* this is so DrawBar will be exposed when only it is exposed */
	ax->check_expose_region = False;

	ax->display_axes_labels = AXES_XY;
	ax->moving_cursor = False;
	ax->manual_scroll =
		(mp->projection == MAP_LINEAR_CYLINDRICAL ||
		 mp->projection == MAP_UTM || mp->projection == MAP_UTM_NEAR ||
		 mp->projection == MAP_POLAR) ? False : True;
	if(mp->projection == MAP_POLAR) {
	    ax->display_axes = False;
	}

	mp->nthemes = 0;
	mp->size_themes = 0;
	mp->theme = NULL;
	mp->nlines = 0;
	mp->size_line = 0;
	mp->line = NULL;
	mp->nsymbol = 0;
	mp->size_symbol = 0;
	mp->symbol = NULL;
	mp->nsymgrp = 0;
	mp->size_symgrp = 0;
	mp->symgrp = NULL;
	mp->nsta = 0;
	mp->size_sta = 0;
	mp->sta = NULL;
	mp->nsrc = 0;
	mp->size_src = 0;
	mp->src = NULL;
	mp->narc = 0;
	mp->size_arc = 0;
	mp->arc = NULL;
	mp->nell = 0;
	mp->size_ell = 0;
	mp->ell = NULL;
	mp->ndel = 0;
	mp->size_del = 0;
	mp->del = NULL;
	mp->npoly = 0;
	mp->size_poly = 0;
	mp->poly = NULL;
	mp->nrect = 0;
	mp->size_rect = 0;
	mp->rect = NULL;

	mp->n_measure_arcs = 0;
	mp->measure_arc = (MapArc **)AxesMalloc(w_new, sizeof(MapArc *));
	mp->n_measure_dels = 0;
	mp->measure_del = (MapDelta **)AxesMalloc(w_new, sizeof(MapDelta *));

	mp->border.nsegs = 0;
	mp->border.segs = (DSegment *) NULL;
	mp->border.size_segs = 0;

	mp->grid.nsegs = 0;
	mp->grid.segs = (DSegment *) NULL;
	mp->grid.size_segs = 0;

	mp->circle = NULL;
	mp->ncircles = 0;
	mp->circles.nsegs = 0;
	mp->circles.segs = (DSegment *)NULL;
	mp->circles.size_segs = 0;
	mp->circle_start = (int *)NULL;
	mp->num_circle_segs = (int *)NULL;
	mp->primary_circle = -1;
	mp->polar_margin = .2;

	mp->display_polar_selection = False;
	mp->polar_select_id = -1;
	mp->polar_select = False;
	mp->moving_polar_select = False;
	mp->size_polar_select = False;
	mp->polar_radius = -1.;
	mp->polar_del_radius = 0.;
	mp->polar_azimuth = 100.;
	mp->polar_del_azimuth = 20.;
	mp->polar_line = NULL;
	mp->polar_select_i3 = -1;
	mp->polar_select_side = -1;
	mp->select_color_bar = False;

/**
	translations = XtParseTranslationTable(defTrans);
	XtOverrideTranslations(nou, translations);
**/

	ax->clear_on_redisplay = False;

	if(mp->projection == MAP_POLAR) {
	    double d;
	    ax->a.ew = 0;
	    ax->a.ns = 0;
	    ax->display_axes = False;
	    if(mp->polar_max_radius <= 0.) mp->polar_max_radius = 1.;
	    d = mp->polar_margin*mp->polar_max_radius;
	    ax->zoom = 0;
	    ax->x1[0] = -mp->polar_max_radius - d;
	    ax->x2[0] =  mp->polar_max_radius + d;
	    ax->y1[0] = -mp->polar_max_radius - d;
	    ax->y2[0] =  mp->polar_max_radius + d;
	}
	else if(mp->projection == MAP_UTM_NEAR) {
	    ax->a.ew = 0;
	    ax->a.ns = 0;
	}
	else {
	    ax->a.ew = 1;
	    ax->a.ns = 1;
	}
	ax->redraw_data_func = MapPlotRedraw;
	ax->hard_copy_func = MapPlotHardCopy;
	ax->transform = XYTransform;
	ax->resize_func = MapPlotResize;
	ax->use_pixmap = True;
	mp->pixmap = (Pixmap)NULL;
	mp->need_redisplay = False;

	ax->a.min_xlab = 4;
	ax->a.max_xlab = 20;
	ax->a.min_ylab = 3;
	ax->a.max_ylab = 20;

#ifdef __OLD__
        mp->cmap = (Colormap) NULL;
        mp->plane_masks = NULL;
        mp->num_private_colors = 0;
        mp->num_cells = 0;
        mp->private_colors = NULL;
        mp->private_pixels = (Pixel *)NULL;

        mp->num_colors = 0;
        mp->colors = NULL;
        mp->pixels = (Pixel *)NULL;

        mp->num_stipples = 0;
        mp->stipples = NULL;
        mp->num_lines = 0;
        mp->lines = NULL;
#endif

        mp->ndeci = 0;
        mp->bar_width = 0;
	mp->select_sta_src = False;
	mp->select_arc_del = False;
	mp->select_symbol = False;
	mp->moving_circle = False;
	mp->moving_station = False;
	mp->moving_index = -1;
	mp->moving_x0 = 0;
	mp->moving_y0 = 0;
	mp->motion_which = -1;
	mp->motion_imin = -1;
	mp->motion_jmin = -1;
	mp->info_popup = NULL;

	mp->image = NULL;
	mp->image_width = 0;
	mp->image_height = 0;

	mp->size_points = 0;
	mp->points = NULL;

	mp->polar_ndeci = 0;
	mp->numaz = 0;
	mp->numdist = 0;
	mp->app_context = NULL;

	mp->utm_letter = '\0';
	mp->utm_select_letter = '\0';
	mp->utm_cell_letter = '\0';
	mp->utm_zone = -1;
	mp->utm_select_zone = -1;
	mp->utm_cell_zone = -1;
	mp->selecting_utm = False;
	mp->utm_map = (MapPlotWidget)NULL;

	mp->next_id = -1;

/* to restrict the zoom-in
	ax->delx_min = .4;
	ax->dely_min = .2;
*/

	XtAddCallback(w_new, XtNcrosshairCallback, CrosshairCB, NULL);
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
	MapPlotWidget w = (MapPlotWidget)widget;
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	ax->a.ew = 1;
	ax->a.ns = 1;

	(*((mapPlotWidgetClass->core_class.superclass)->
			core_class.realize))((Widget)w,  valueMask, attrs);

	XSetLineAttributes(XtDisplay(w), ax->mag_invertGC, 2,
		LineSolid, CapNotLast, JoinMiter);

	mp->symbolGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	mp->lineGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
	mp->dataGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

        mp->contour_labelGC= XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
        mp->colorGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
        mp->barGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

	XSetForeground(XtDisplay(w), mp->contour_labelGC,
		mp->contour_label_fg);
	XSetBackground(XtDisplay(w), mp->contour_labelGC,
		w->core.background_pixel);

	XSetForeground(XtDisplay(w), mp->colorGC, w->axes.fg);
	XSetBackground(XtDisplay(w), mp->colorGC,
		w->core.background_pixel);
	XSetForeground(XtDisplay(w), mp->barGC, w->axes.fg);
	XSetBackground(XtDisplay(w), mp->barGC,
		w->core.background_pixel);
	XSetFont(XtDisplay(w), mp->barGC, w->axes.font->fid);
	
	XSetBackground(XtDisplay(w), mp->symbolGC, w->core.background_pixel);
	XSetFont(XtDisplay(w), mp->symbolGC, mp->label_font->fid);
	XSetBackground(XtDisplay(w), mp->lineGC, w->core.background_pixel);
	XSetFont(XtDisplay(w), mp->lineGC, mp->label_font->fid);

	XtAddCallback((Widget)w, XtNzoomCallback, (XtCallbackProc)MapPlotZoom,
			NULL);

	/* important to create pixmap with (width+1,height+1) for calls to
	 * XFillRectangle(... width, height)
	 * Otherwise get bad problems with some X-servers
	 */
	mp->pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
			w->core.width+1, w->core.height+1, ax->depth);
	if(mp->pixmap == (Pixmap)NULL) {
		fprintf(stderr, "%s: XCreatePixmap failed.\n", w->core.name);
		exit(1);
	}

	_AxesRedraw((AxesWidget)w);
	_MapPlotRedraw(w);
	RedisplayThemes(w);
	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	
	if(!ax->redisplay_pending) {
		_AxesRedisplayAxes((AxesWidget)w);
		RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		_AxesRedisplayXor((AxesWidget)w);
	}

	InitImage(w);
	_MapPlotRedisplay(w);
}

static void
InitImage(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	Display	*display;
	char	*data;
	int	num_colors, depth;

	num_colors = XDisplayCells(XtDisplay(w), DefaultScreen(XtDisplay(w)));
	if(mp->image == NULL && num_colors > 20)
	{
	    mp->image_width = w->core.width;
	    mp->image_height = w->core.height;

	    display = XtDisplay(w);
	    depth = DefaultDepth(display, DefaultScreen(display));

	    data = (char *)AxesMalloc((Widget)w,
			mp->image_width*mp->image_height*8);
	    if( !data ) return;

	    mp->image = XCreateImage(display,
				DefaultVisual(display, DefaultScreen(display)),
				depth, ZPixmap, 0, data, mp->image_width,
				mp->image_height,  8, 0);
	}
}

static void
MapPlotResize(AxesWidget widget)
{
	MapPlotWidget w = (MapPlotWidget)widget;
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	if(mp->pixmap != (Pixmap)NULL) {
		XFreePixmap(XtDisplay(w), mp->pixmap);
		mp->pixmap = (Pixmap)NULL;
	}
	mp->pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
			w->core.width+1, w->core.height+1, ax->depth);

	if(mp->pixmap == (Pixmap)NULL) {
		fprintf(stderr, "%s: XCreatePixmap failed.\n", w->core.name);
		exit(1);
	}
}

void
MapPlotSetPolarMargin(MapPlotWidget w, double polar_margin)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double d;

	mp->polar_margin = polar_margin;
	d = mp->polar_margin*mp->polar_max_radius;
	ax->x1[0] = -mp->polar_max_radius - d;
	ax->x2[0] =  mp->polar_max_radius + d;
	ax->y1[0] = -mp->polar_max_radius - d;
	ax->y2[0] =  mp->polar_max_radius + d;
	if(ax->zoom == 0 && XtIsRealized((Widget)w)) {
		_AxesRedraw((AxesWidget)w);
		_MapPlotRedraw(w);
		RedisplayThemes(w);
		RedisplayPixmap(w, mp->pixmap, ax->pixmap);
		RedisplayOverlays(w, ax->pixmap);
		_AxesRedisplayAxes((AxesWidget)w);
		RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		_AxesRedisplayXor((AxesWidget)w);
	}
}

static Boolean
SetValues(MapPlotWidget cur, MapPlotWidget req, MapPlotWidget nou)
{
	MapPlotPart *cur_mp = &cur->map_plot;
	MapPlotPart *req_mp = &req->map_plot;
	MapPlotPart *new_mp = &nou->map_plot;
	AxesPart *new_ax = &nou->axes;
	int		i;
	Boolean		redisplay = False, redraw = False;
	Boolean		clear = False;
	double		x1, x2, y1, y2, z1, z2;
	MapArc		*arc;
	MapEllipse	*ell;
	MapCircleCallbackStruct callback;

	if(!XtIsRealized((Widget)cur))
	{
		return(False);
	}
	if(cur->axes.font != req->axes.font) {
	    XSetFont(XtDisplay(nou), req_mp->barGC, nou->axes.font->fid);
	    redraw = True;
	}
	if(cur_mp->lon_min != req_mp->lon_min ||
	   cur_mp->lon_max != req_mp->lon_max ||
	   cur_mp->lat_min != req_mp->lat_min ||
	   cur_mp->lat_max != req_mp->lat_max)
	{
	    if(new_mp->projection != MAP_LINEAR_CYLINDRICAL &&
		new_mp->projection != MAP_UTM)
	    {
		project(nou, req_mp->lon_min, req_mp->lat_min, &x1, &y1, &z1);
		project(nou, req_mp->lon_max, req_mp->lat_max, &x2, &y2, &z2);
	    }
	    else
	    {
		x1 = req_mp->lon_min;
		while(x1 < new_ax->x1[0]) x1 += 360;
		while(x1 > new_ax->x2[0]) x1 -= 360;
		x2 = req_mp->lon_max;
		while(x2 < new_ax->x1[0]) x2 += 360;
		while(x2 > new_ax->x2[0]) x2 -= 360;
		y1 = req_mp->lat_min;
		y2 = req_mp->lat_max;
		z1 = z2 = 1;
	    }
	    if(z1 < 0 || z2 < 0)
	    {
		new_mp->lon_min = cur_mp->lon_min;
		new_mp->lon_max = cur_mp->lon_max;
		new_mp->lat_min = cur_mp->lat_min;
		new_mp->lat_max = cur_mp->lat_max;
	    }
	    else
	    {
		if(x1 > x2)
		{
		    z1 = x1;
		    x1 = x2;
		    x2 = z1;
		}
		if(y1 > y2)
		{
		    z1 = y1;
		    y1 = y2;
		    y2 = z1;
		}
		new_ax->zoom = 1;
		new_ax->x1[1] = x1;
		new_ax->x2[1] = x2;
		new_ax->y1[1] = y1;
		new_ax->y2[1] = y2;
		redraw = True;
		clear = True;
	    }
	}
	if(cur_mp->projection != req_mp->projection)
	{
	    AxesWaitCursor((Widget)nou, True);
	    new_ax->manual_scroll = (
		new_mp->projection == MAP_LINEAR_CYLINDRICAL ||
		new_mp->projection == MAP_UTM ||
		new_mp->projection == MAP_UTM_NEAR ||
		new_mp->projection == MAP_POLAR) ? False : True;

	    if(req_mp->projection == MAP_POLAR)
	    {
		double d;
		new_ax->a.ew = 0;
		new_ax->a.ns = 0;
		new_ax->display_axes = False;
		if(new_mp->polar_max_radius <= 0.) new_mp->polar_max_radius =1.;
		d = new_mp->polar_margin*new_mp->polar_max_radius;
		new_ax->zoom = 0;
		new_ax->x1[0] = -new_mp->polar_max_radius - d;
		new_ax->x2[0] =  new_mp->polar_max_radius + d;
		new_ax->y1[0] = -new_mp->polar_max_radius - d;
		new_ax->y2[0] =  new_mp->polar_max_radius + d;
	    }
	    else if(req_mp->projection == MAP_UTM_NEAR) {
		new_ax->a.ew = 0;
		new_ax->a.ns = 0;
		new_ax->display_axes = True;
	    }
	    else {
		new_ax->a.ew = 1;
		new_ax->a.ns = 1;
		new_ax->display_axes = True;
	    }
	    clear = True;
	    if( (cur_mp->projection == MAP_LINEAR_CYLINDRICAL &&
		    new_mp->projection == MAP_UTM) || (cur_mp->projection ==
		MAP_UTM && new_mp->projection == MAP_LINEAR_CYLINDRICAL) )
	    {
		redraw = False;
		redisplay = True;
	    }
	    else {
		redraw = True;
		MapProjection(cur, nou);
	    }
	    AxesWaitCursor((Widget)nou, False);
	}
	if(cur_mp->projection == MAP_POLAR &&
              cur_mp->polar_max_radius != req_mp->polar_max_radius)
        {
	    double d;
	    AxesWaitCursor((Widget)nou, True);
	    new_ax->a.ew = 0;
	    new_ax->a.ns = 0;
	    new_ax->display_axes = False;
	    if(new_mp->polar_max_radius <= 0.) new_mp->polar_max_radius = 1.;
	    d = new_mp->polar_margin*new_mp->polar_max_radius;
	    new_ax->zoom = 0;
	    new_ax->x1[0] = -new_mp->polar_max_radius - d;
	    new_ax->x2[0] =  new_mp->polar_max_radius + d;
	    new_ax->y1[0] = -new_mp->polar_max_radius - d;
	    new_ax->y2[0] =  new_mp->polar_max_radius + d;

	    MapProjection(cur, nou);
	    clear = True;
	    redraw = True;
	    AxesWaitCursor((Widget)nou, False);
        }

	if(cur_mp->display_circles != req_mp->display_circles)
	{
	    if(req_mp->display_circles == MAP_SELECTED_ON)
	    {
		for(i = 0; i < new_mp->ncircles; i++)
		{
		    new_mp->circle[i].display = new_mp->circle[i].selected;
		    if(new_mp->circle[i].selected && i !=new_mp->primary_circle)
		    {
			new_mp->circle[i].selected =False;
		    }
		}
		if(new_mp->primary_circle >= 0)
		{
		    i = new_mp->primary_circle;
		    callback.pt = new_mp->circle[i].pt;
		    callback.primary = True;
		    XtCallCallbacks((Widget)nou, XtNselectCircleCallback,
					&callback);
		}
	    }
	    else
	    {
		for(i = 0; i < new_mp->ncircles; i++)
		{
		    new_mp->circle[i].display = True;
		}
	    }
	    redisplay = True;
	    if(req_mp->display_circles != MAP_OFF)
	    {
		DrawCircles(nou);
	    }
	}
	if(cur_mp->display_station_tags != req_mp->display_station_tags)
	{
	    redisplay = True;
	}
	if(cur_mp->display_source_tags != req_mp->display_source_tags)
	{
	    redisplay = True;
	}
	if(cur_mp->display_source_ellipses != req_mp->display_source_ellipses)
	{
	    if(req_mp->display_source_ellipses > MAP_LOCKED_OFF &&
		 req_mp->display_source_ellipses < MAP_LOCKED_ON)
	    {
		for(i = 0; i < new_mp->nell; i++)
		    if(new_mp->ell[i]->type == MAP_SRC_ELLIPSE &&
		       new_mp->ell[i]->ellipse.line.display > MAP_LOCKED_OFF)
		{
		    ell = new_mp->ell[i];
		    ell->ellipse.line.display = req_mp->display_source_ellipses;
		    if(ell->ellipse.line.display > MAP_OFF)
		    {
			if(ell->changed) MakeEllipse(nou, ell);
			Free(ell->s.segs);
			ell->s.size_segs = 0;
			ell->s.nsegs = 0;
			new_ax->d.s  = &ell->s;
			DrawLonLat(nou, &new_ax->d, ell->npts, ell->lon,
				ell->lat);
		    }
		}
		redisplay = True;
	    }
	    else
	    {
		new_mp->display_source_ellipses = 
				cur_mp->display_source_ellipses;
	    }
	}
	if(cur_mp->display_paths != req_mp->display_paths)
	{
	    if(req_mp->display_paths > MAP_LOCKED_OFF &&
		req_mp->display_paths < MAP_LOCKED_ON)
	    {
		for(i = 0; i < new_mp->narc; i++)
		    if(new_mp->arc[i]->type == MAP_PATH &&
		       new_mp->arc[i]->arc.line.display > MAP_LOCKED_OFF)
		{
		    arc = new_mp->arc[i];
		    arc->arc.line.display = req_mp->display_paths;
		    if(arc->arc.line.display > MAP_OFF)
		    {
			if(arc->changed) MakeArc(nou, arc);
			DrawArc(nou, &new_ax->d, arc);
		    }
		}
		redisplay = True;
	    }
	    else
	    {
		new_mp->display_paths = cur_mp->display_paths;
	    }
	}
	if(cur_mp->map_display_grid != req_mp->map_display_grid)
	{
	    redisplay = True;
	}
	if(cur_mp->display_map_bdy != req_mp->display_map_bdy)
	{
	    redraw = True;
	}
	if(cur_mp->auto_interval != req_mp->auto_interval)
	{
	    redraw = True;
	}
	if(cur_mp->contour_interval != req_mp->contour_interval
	    && req_mp->auto_interval)
	{
	    redraw = True;
	}
	if(cur_mp->contour_min != req_mp->contour_min ||
	   cur_mp->contour_max != req_mp->contour_max)
	{
	    redraw = True;
	}
	if(cur_mp->mode != req_mp->mode)
	{
	    InitImage(nou);
	    redraw = True;
	}

	if(redraw)
	{
	    _AxesRedraw((AxesWidget)nou);
	    _MapPlotRedraw(nou);
	    RedisplayThemes(nou);
	    redisplay = True;
	}
	if(redisplay)
	{
	    RedisplayPixmap(nou, new_mp->pixmap, new_ax->pixmap);
	    RedisplayOverlays(nou, new_ax->pixmap);
	    if(clear)
	    {
		_AxesRedisplayAxes((AxesWidget)nou);
	    }
	    RedisplayPixmap(nou, new_ax->pixmap, XtWindow(nou));
	    _AxesRedisplayXor((AxesWidget)nou);
	}
	return(False);
}

void
MapPlotSetPolarMaxRadius(MapPlotWidget w, double max_radius)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	if(max_radius != mp->polar_max_radius && max_radius > 0.)
	{
	    double d;
	    mp->polar_max_radius = max_radius;

	    ax->a.ew = 0;
	    ax->a.ns = 0;
	    ax->display_axes = False;
	    d = mp->polar_margin*mp->polar_max_radius;
	    ax->zoom = 0;
	    ax->x1[0] = -mp->polar_max_radius - d;
	    ax->x2[0] =  mp->polar_max_radius + d;
	    ax->y1[0] = -mp->polar_max_radius - d;
	    ax->y2[0] =  mp->polar_max_radius + d;

	    MapProjection(w, w);

	    if(XtIsRealized((Widget)w))
	    {
		_AxesRedraw((AxesWidget)w);
		_MapPlotRedraw(w);
		RedisplayThemes(w);
		RedisplayPixmap(w, mp->pixmap, ax->pixmap);
		RedisplayOverlays(w, ax->pixmap);
		_AxesRedisplayAxes((AxesWidget)w);
		RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		_AxesRedisplayXor((AxesWidget)w);
	    }
	}
}

double
MapPlotGetPolarMaxRadius(MapPlotWidget w)
{
	return w->map_plot.polar_max_radius;
}

int
MapPlotThemeColorScale(MapPlotWidget w, int theme_id, ColorScale *color_scale)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	ColorScale *c;
	int i, j, n;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	c = &mp->theme[i]->theme.color_scale;
	n = color_scale->num_colors;

	for(j = 0; j < c->num_labels; j++) Free(c->labels[j]);

	if(color_scale->num_colors != c->num_colors)
	{
	    Free(c->pixels);
	    Free(c->lines);
	    if( !(c->pixels = (Pixel *)AxesMalloc((Widget)w,
			n*sizeof(Pixel))) ) return 0;
	    if( !(c->lines = (double *)AxesMalloc((Widget)w,
			(n+1)*sizeof(double))) ) return 0;
	    Free(c->label_values);
	    if( color_scale->num_labels &&
		!(c->label_values = (double *)AxesMalloc((Widget)w,
		color_scale->num_labels*sizeof(double))) ) return 0;
	    Free(c->labels);
	    if( color_scale->num_labels &&
		!(c->labels = (char **)AxesMalloc((Widget)w,
		    color_scale->num_labels*sizeof(char *))) ) return 0;
	}

	memcpy(c->pixels, color_scale->pixels, n*sizeof(Pixel));
	memcpy(c->lines, color_scale->lines, (n+1)*sizeof(double));
	c->num_colors = color_scale->num_colors;
	c->num_labels = color_scale->num_labels;
	memcpy(c->label_values, color_scale->label_values,
			c->num_labels*sizeof(double));
	for(j = 0; j < c->num_labels; j++) {
	    c->labels[j] = strdup(color_scale->labels[j]);
	}

	if(ComputeBar(w, c))
	{
	    if(!XtIsRealized((Widget)w)) return 1;
	    _AxesRedraw((AxesWidget)w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _MapPlotRedraw(w);
	    _MapPlotDrawBar(w);
	    MapPlotUpdate(w);
	}
	else {
	    if(mp->theme[i]->theme_type == THEME_IMAGE) {
		DrawTheme(w, &ax->d, mp->theme[i]);
	    }
	    _MapPlotDrawBar(w);
	    MapPlotUpdate(w);
	}

	if( mp->utm_map ) {
	    MapPlotThemeColorScale(mp->utm_map, theme_id, color_scale);
	}
	return 1;
}

int
MapPlotThemeColorBar(MapPlotWidget w, int theme_id)
{
	MapPlotPart *mp = &w->map_plot;
	int i, j;

        for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
        if(i == mp->nthemes) return 0;

	mp->theme[i]->color_bar_on = True;
	for(j = 0; j < mp->nthemes; j++) {
	    if(j != i) mp->theme[j]->color_bar_on = False;
	}
	if(ComputeBar(w, &mp->theme[i]->theme.color_scale))
	{
	    if(!XtIsRealized((Widget)w)) return 1;
	    _AxesRedraw((AxesWidget)w);
	    _AxesRedisplayAxes((AxesWidget)w);
	    _MapPlotRedraw(w);
	}
	_MapPlotDrawBar(w);
	MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotThemeColorBar(mp->utm_map, theme_id);
	}
	return 1;
}

void
MapPlotThemeColorBarOff(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nthemes; i++) {
	    mp->theme[i]->color_bar_on = False;
	}
	mp->bar_width = 0;
	w->axes.right_margin = 0;
	_AxesRedraw((AxesWidget)w);
	_AxesRedisplayAxes((AxesWidget)w);
	_MapPlotRedraw(w);
	MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotThemeColorBarOff(mp->utm_map);
	}
}

int
MapPlotAddImage(MapPlotWidget w, char *label, int nx, int ny, float *x,
		float *y, float *z, float no_data_flag, int mode,
		MapPlotTheme *theme, Boolean display)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	MapImage	*mi;
	MapTheme	*t;

	if(nx <= 1 || ny <= 1 || x == NULL || y == NULL || z == NULL)
	{
	   _AxesWarn((AxesWidget)w, "MapPlotAddImage: invalid image input");
	    return(0);
	}
	ALLOC((mp->nthemes+1)*sizeof(MapTheme *),
		mp->size_themes, mp->theme, MapTheme *, 0);

	mp->theme[mp->nthemes] = new MapTheme();

	t = mp->theme[mp->nthemes];
	mp->nthemes++;

	t->theme = *theme;

	t->theme_type = THEME_IMAGE;
	t->id = getMapId(mp);

	mi = &t->map_image;

	mi->label = (label != NULL) ? strdup(label) : strdup("");
	mi->copy = False;

	if(!(mi->m.x = (double *)AxesMalloc((Widget)w, nx*sizeof(double))) ||
	   !(mi->m.y = (double *)AxesMalloc((Widget)w, ny*sizeof(double))) ||
	   !(mi->m.z = (float *)AxesMalloc((Widget)w, nx*ny*sizeof(float))) )
	{
	    DeleteTheme(w, mp->nthemes-1);
	    return(0);
	}
	
	for(i = 0; i < nx; i++) mi->m.x[i] = (double)x[i];
	for(i = 0; i < ny; i++) mi->m.y[i] = (double)y[i];
	memcpy(mi->m.z, z, nx*ny*sizeof(float));

	mi->m.exc = no_data_flag;

	mi->m.nx = nx;
	mi->m.ny = ny;
	mi->m.imin = mi->m.jmin = 0;
	mi->m.imax = mi->m.jmax = 0;
	mi->ci = mp->contour_interval;
	mi->auto_ci = mp->auto_interval;
	mi->c_min = mp->contour_min;
	mi->c_max = mp->contour_max;

	mi->mode = mode;

	DrawTheme(w, &ax->d, t);

	if(display)
	{
	    RedisplayThemes(w);
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}

	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = t->id;
	    MapPlotAddImageCopy(mp->utm_map, label, &mi->m, mode,theme,display);
	    utm_mp->next_id = -1;
	}
	    
	return(t->id);
}

int
MapPlotAddImageCopy(MapPlotWidget w, char *label, Matrx *m, int mode,
		MapPlotTheme *theme, Boolean display)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	MapImage	*mi;
	MapTheme	*t;

	if(!m || m->nx <= 1 || m->ny <= 1 || m->x == NULL || m->y == NULL
		|| m->z == NULL)
	{
	   _AxesWarn((AxesWidget)w, "MapPlotAddImageCopy: invalid image input");
	    return(0);
	}
	ALLOC((mp->nthemes+1)*sizeof(MapTheme *),
		mp->size_themes, mp->theme, MapTheme *, 0);

	mp->theme[mp->nthemes] = new MapTheme();

	t = mp->theme[mp->nthemes];
	mp->nthemes++;

	t->theme = *theme;

	t->theme_type = THEME_IMAGE;
	t->id = getMapId(mp);

	mi = &t->map_image;

	mi->label = (label != NULL) ? strdup(label) : strdup("");

	mi->copy = True;
	mi->m = *m;

	mi->m.imin = mi->m.jmin = 0;
	mi->m.imax = mi->m.jmax = 0;
	mi->ci = mp->contour_interval;
	mi->auto_ci = mp->auto_interval;
	mi->c_min = mp->contour_min;
	mi->c_max = mp->contour_max;

	mi->mode = mode;

	DrawTheme(w, &ax->d, t);

	if(display)
	{
	    RedisplayThemes(w);
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}

	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = t->id;
	    MapPlotAddImageCopy(mp->utm_map, label, &mi->m, mode,theme,display);
	    utm_mp->next_id = -1;
	}
	    
	return(t->id);
}

void
MapPlotChangeImage(MapPlotWidget w, int theme_id, float *z, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return;

	t = mp->theme[i];

	memcpy(t->map_image.m.z, z,
		t->map_image.m.nx*t->map_image.m.ny*sizeof(float));
	if(redisplay) {
	    DrawTheme(w, &ax->d, t);
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotChangeImage(mp->utm_map, theme_id, z, redisplay);
	}
}

void
MapPlotClearArcs(MapPlotWidget w, char *label, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;
	Boolean found_one = False;

	for(i = mp->narc-1; i >= 0; i--)
	    if(mp->arc[i]->type != MAP_MEASURE &&
		mp->arc[i]->type != MAP_CURSOR_MEASURE && (label == NULL ||
		(mp->arc[i]->arc.label != NULL &&
		    !strcmp(mp->arc[i]->arc.label, label))))
	{
	    found_one = True;
	    DeleteArc(w, i);
	}
	if(found_one && redisplay) MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotClearArcs(mp->utm_map, label, redisplay);
	}
}

void
MapPlotClearDeltas(MapPlotWidget w, char *label, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;
	Boolean found_one = False;

	for(i = mp->ndel-1; i >= 0; i--)
	    if(mp->del[i]->type != MAP_MEASURE &&
		mp->del[i]->type != MAP_CURSOR_MEASURE && (label == NULL ||
		(mp->del[i]->delta.label != NULL &&
		    !strcmp(mp->del[i]->delta.label, label))))
	{
	    found_one = True;
	    DeleteDelta(w, i);
	}
	if(found_one && redisplay) MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotClearDeltas(mp->utm_map, label, redisplay);
	}
}

static void
MapPlotRedraw(AxesWidget widget, int type, double shift)
{
	MapPlotWidget w = (MapPlotWidget)widget;
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double xmin, xmax, ymin, ymax;

	AxesWaitCursor((Widget)widget, True);
	switch(type)
	{
	    case DATA_REDISPLAY :
		if(mp->need_redisplay)
		{
		    _MapPlotRedisplay(w);
		}
		else
		{
		    _MapPlotDrawBar(w);
		    RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		}
		break;
	    case DATA_REDRAW :
		_MapPlotRedraw(w);
		RedisplayThemes(w);
		RedisplayPixmap(w, mp->pixmap, ax->pixmap);
		RedisplayOverlays(w, ax->pixmap);
		break;
	    case DATA_JUMP_HOR :
	    case DATA_JUMP_VERT :
	    case DATA_SCROLL_HOR :
	    case DATA_SCROLL_VERT :
		if(mp->projection != MAP_POLAR &&
		   mp->projection != MAP_LINEAR_CYLINDRICAL &&
		   mp->projection != MAP_UTM &&
		   mp->projection != MAP_UTM_NEAR)
		{
		    if( mp->projection == MAP_ORTHOGRAPHIC ||
			mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
			mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
		    {
			if(mp->rotate_on_zoom) {
			    OrthoRotate(w);
			}
		    }
		    _AxesRedraw((AxesWidget)w);
		    _MapPlotRedraw(w);
		    RedisplayThemes(w);
		    if(!ax->redisplay_pending)
		    {
			RedisplayPixmap(w, mp->pixmap, ax->pixmap);
			RedisplayOverlays(w, ax->pixmap);
			_AxesRedisplayAxes((AxesWidget)w);
			_MapPlotDrawBar(w);
			RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		    }
		}
		else
		{
		    _MapPlotRedraw(w);
		    RedisplayThemes(w);
		    _MapPlotRedisplay(w);
		}
		break;
	    case DATA_ZOOM_ZERO :
		xmin = ax->x1[0];
		xmax = ax->x2[0];
		ymin = ax->y1[0];
		ymax = ax->y2[0];

		SetZoomZero(w);

		if(xmin != ax->x1[0] || xmax != ax->x2[0] ||
		   ymin != ax->y1[0] || ymax != ax->y2[0])
		{
		    _AxesRedraw((AxesWidget)w);
		    _MapPlotRedraw(w);
		    RedisplayThemes(w);
		    if(!ax->redisplay_pending)
		    {
			RedisplayPixmap(w, mp->pixmap, ax->pixmap);
			RedisplayOverlays(w, ax->pixmap);
			_AxesRedisplayAxes((AxesWidget)w);
			_MapPlotDrawBar(w);
			RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		    }
		}
		break;
	}
	AxesWaitCursor((Widget)widget, False);
}

/** 
 * 
 */
void
_MapPlotRedisplay(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	if(!XtIsRealized((Widget)w)) return;
	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	RedisplayOverlays(w, ax->pixmap);
	RedisplayPixmap(w, ax->pixmap, XtWindow(w));
}

static void
RedisplayPixmap(MapPlotWidget w, Pixmap pixmap, Window window)
{
	AxesPart *ax = &w->axes;
	int x0, y0, width, height;

	if(!XtIsRealized((Widget)w)) return;

	if(window != XtWindow(w)) {
	    x0 = unscale_x(&ax->d, ax->x1[ax->zoom]) + 1;
	    y0 = unscale_y(&ax->d, ax->y2[ax->zoom]) + 1;
	    width = unscale_x(&ax->d, ax->x2[ax->zoom]) - x0;
	    height = unscale_y(&ax->d, ax->y1[ax->zoom]) - y0;
	}
	else {
	    x0 = 0;
	    y0 = 0;
	    width = w->core.width;
	    height = w->core.height;
	}

	XSetClipMask(XtDisplay(w), ax->axesGC, None);
	XCopyArea(XtDisplay(w), pixmap, window, ax->axesGC,
		x0, y0, width, height, x0, y0);
}

static void
RedisplayOverlays(MapPlotWidget w, Window window)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	SymbolInfo	*sym;
	LineInfo	*line;
	MapStation	*sta;
	MapSource	*src;

	if(!XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nlines; i++)
	{
	    line = &mp->line[i]->line.line;
	    if(line->display == MAP_ON || line->display == MAP_LOCKED_ON ||
		(line->display == MAP_SELECTED_ON && line->selected))
	    {
		if(mp->line[i]->line.id != mp->polar_select_id) {
		    RedisplayLine(w, window, line, &mp->line[i]->s);
		}
	    }
	}
	for(i = 0; i < mp->nsymgrp; i++)
	{
	    sym = &mp->symgrp[i]->group.sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		RedisplaySymbolGroup(w, window, mp->symgrp[i]);
	    }
	}
	for(i = 0; i < mp->nsymbol; i++)
	{
            if(mp->symbol[i].sym.display == MAP_ON ||
                mp->symbol[i].sym.display == MAP_LOCKED_ON ||
                (mp->symbol[i].sym.display == MAP_SELECTED_ON &&
                        mp->symbol[i].sym.selected))
            {
                RedisplayMapSymbol(w, window, &mp->symbol[i]);
            }
	}
	
	if(mp->display_circles) RedisplayCircles(w, window);

	for(i = 0; i < mp->nsta; i++)
	{
	    sym = &mp->sta[i]->station.sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		sta = mp->sta[i];
		sta->visible = RedisplaySymbol(w, window, sta->station.lon,
						sta->station.lat, sym);
	    }
	}
	if(mp->display_station_tags)
	{
	    DrawStationTags(w, window);
	}

	for(i = 0; i < mp->nsrc; i++)
	{
	    sym = &mp->src[i]->source.sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		src = mp->src[i];
		src->visible = RedisplaySymbol(w, window, src->source.lon,
						src->source.lat, sym);
	    }
	}
	if(mp->display_source_tags)
	{
	    DrawSourceTags(w, window);
	}

	for(i = 0; i < mp->narc; i++)
	{
	    line = &mp->arc[i]->arc.line;
	    if(line->display == MAP_ON || line->display == MAP_LOCKED_ON ||
		(line->display == MAP_SELECTED_ON && line->selected))
	    {
		RedisplayLine(w, window, line, &mp->arc[i]->s);
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	{
	    line = &mp->del[i]->delta.line;
	    if(line->display == MAP_ON || line->display == MAP_LOCKED_ON ||
		(line->display == MAP_SELECTED_ON && line->selected))
	    {
		RedisplayLine(w, window, line, &mp->del[i]->s);
	    }
	}
	for(i = 0; i < mp->nell; i++)
	{
	    line = &mp->ell[i]->ellipse.line;
	    if(line->display == MAP_ON || line->display == MAP_LOCKED_ON ||
		(line->display == MAP_SELECTED_ON && line->selected))
	    {
		RedisplayLine(w, window, line, &mp->ell[i]->s);
	    }
	}
	for(i = 0; i < mp->npoly; i++)
	{
	    sym = &mp->poly[i]->sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		RedisplayPoly(w, window, mp->poly[i]);
	    }
	}
	for(i = 0; i < mp->nrect; i++)
	{
	    sym = &mp->rect[i]->sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		RedisplayRectangle(w, window, mp->rect[i]);
	    }
	}
	if(mp->border.nsegs > 0)
	{
	    XDrawSegments(XtDisplay(w), window, ax->axesGC,
			(XSegment *)mp->border.segs, mp->border.nsegs);
	}
	if(mp->map_display_grid && mp->grid.nsegs > 0)
	{
	    XDrawSegments(XtDisplay(w), window, ax->axesGC,
			(XSegment *)mp->grid.segs, mp->grid.nsegs);
	    if(mp->projection == MAP_POLAR) {
		polarGridLabels(w, window, mp->numaz, mp->az, mp->numdist,
				mp->dist);
	    }
	}
	if(mp->projection != MAP_POLAR)
	{
	    for(i = 0; i < mp->nthemes; i++)
	    {
		MapTheme *t = mp->theme[i];

		if(!t->display || !t->display_grid || t->grid.nsegs <= 0) {
		    continue;
		}

		XDrawSegments(XtDisplay(w), window, ax->axesGC,
			(XSegment *)t->grid.segs, t->grid.nsegs);

		polarGridLabels(w, window, t->numaz, t->az, t->numdist,t->dist);
	    }
	}
	if(mp->projection == MAP_UTM || mp->projection == MAP_UTM_NEAR) {
	    drawUTMGrid(w, window);
	}
	for(i = 0; i < mp->nlines; i++)
	    if(mp->line[i]->line.id == mp->polar_select_id)
	{
	    line = &mp->line[i]->line.line;
	    if(line->display == MAP_ON || line->display == MAP_LOCKED_ON ||
		(line->display == MAP_SELECTED_ON && line->selected))
	    {
		RedisplayLine(w, window, line, &mp->line[i]->s);
	    }
	}

	mp->need_redisplay = False;
}

static void
drawUTMGrid(MapPlotWidget w, Window window)
{
    MapPlotPart	*mp = &w->map_plot;
    if(mp->projection == MAP_UTM) {
	utmGrid(w, window);
    }
    else if(mp->projection == MAP_UTM_NEAR) {
	utmNearGrid(w, window);
    }
}

static void
utmGrid(MapPlotWidget w, Window window)
{
    AxesPart *ax = &w->axes;
    int    i, ix=0, iy=0, n, lats[30], lons[200], zone[200];
    double l, lon, lon_min, lon_max, lat, lat_min, lat_max;
    double minlon, maxlon, minlat, maxlat, x, y, z;
    int ascent, descent, direction;
    XCharStruct overall;
    char c, letter[30], s[10];
    SegArray grid;
    DrawStruct *d = &ax->d;

    grid.size_segs = 0;
    grid.nsegs = 0;
    grid.segs = NULL;
    d->s = &grid;
    SetClipArea(d, ax->x1[ax->zoom], ax->y1[ax->zoom], ax->x2[ax->zoom],
			ax->y2[ax->zoom]);

    unproject(w, ax->x1[ax->zoom], ax->y1[ax->zoom], &minlon, &minlat);
    unproject(w, ax->x2[ax->zoom], ax->y2[ax->zoom], &maxlon, &maxlat);

    if(minlon > maxlon) {
	i = (int)minlon;
	minlon = maxlon;
	maxlon = i;
    }

    lon_min = 6*(int)(minlon/6) - 6;
    lon_max = 6*(int)(maxlon/6) + 6;
    n = 0;
    for(i = (int)lon_min; i <= (int)lon_max; i += 6)
    {
	lon = (double)i;
	l = (lon+180)-(int)((lon+180.)/360.)*360-180; // -180 < l < 180
	if(lon > minlon && lon < maxlon)
	{
	    zone[n] = (int)((l + 180)/6) + 1;
	    lat_min = -80.;
	    if(l == 6) {
		lat_max = 56;
	    }
	    else if(l >= 12 && l <= 36) {
		lat_max = 72;
	    }
	    else {
		lat_max = 84.;
	    }

	    if(lat_min < minlat) lat_min = minlat;
	    if(lat_max > maxlat) lat_max = maxlat;

	    project(w, lon, lat_min, &x, &y, &z);
	    imove(d, x, y);
	    lons[n++] = unscale_x(d, x);
	    iy = unscale_y(d, y);

	    project(w, lon, lat_max, &x, &y, &z);
	    idraw(d, x, y);
	}

	if(l == 6)
	{
	    lat_min = 56.;
	    lat_max = 64.;
	    project(w, lon-3., lat_min, &x, &y, &z);
	    imove(d, x, y);

	    project(w, lon-3., lat_max, &x, &y, &z);
	    idraw(d, x, y);

	    lat_min = 64.;
	    lat_max = 72.;
	    project(w, lon, lat_min, &x, &y, &z);
	    imove(d, x, y);

	    project(w, lon, lat_max, &x, &y, &z);
	    idraw(d, x, y);
	}
	if(l == 6 || l == 18 || l == 30) {
	    lat_min = 72.;
	    lat_max = 84.;
	    project(w, lon+3., lat_min, &x, &y, &z);
	    imove(d, x, y);

	    project(w, lon+3., lat_max, &x, &y, &z);
	    idraw(d, x, y);
	}
    }

    for(i = 0; i < n-1; i++) {
	int lx;
	snprintf(s, sizeof(s), "%d", zone[i]);
	XTextExtents(ax->font, s, (int)strlen(s), &direction, &ascent,
				&descent, &overall);
	lx = (lons[i] + lons[i+1] - overall.width)/2 ;
	XDrawString(XtDisplay(w), window, ax->axesGC, lx, iy-4,
					s, (int)strlen(s));
    }
    if(n > 0) {
	int x2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
	snprintf(s, sizeof(s), "%d", zone[n-1]);
	XTextExtents(ax->font, s, (int)strlen(s), &direction, &ascent,
				&descent, &overall);
	if(ax->zoom == 0 || x2 - lons[n-1] > 2*overall.width) {
	    int lx = (lons[n-1] + x2 - overall.width)/2 ;
	    XDrawString(XtDisplay(w), window, ax->axesGC, lx, iy-4,
					s, (int)strlen(s));
	}
    }

    n = 0;
    for(i = -80; i <= 72; i += 8) {
	lats[n] = -1;
	lat = (double)i;
	if(lat > minlat && lat < maxlat) {
	    project(w, minlon, lat, &x, &y, &z);
	    imove(d, ax->x1[ax->zoom], y);
	    idraw(d, ax->x2[ax->zoom], y);
	    ix = unscale_x(&ax->d, ax->x1[ax->zoom]);
	    letter[n] = getUTMLetter(lat+1.);
	    lats[n] = unscale_y(&ax->d, y);
	}
	n++;
    }
    lat = 84;
    if(lat > minlat && lat < maxlat) {
	project(w, minlon, lat, &x, &y, &z);
	imove(d, ax->x1[ax->zoom], y);
	idraw(d, ax->x2[ax->zoom], y);
	lats[n++] = unscale_y(&ax->d, y);
    }
    iflush(d);
    if(grid.nsegs) {
	XDrawSegments(XtDisplay(w), window, ax->axesGC,
			(XSegment *)grid.segs, grid.nsegs);
    }

    c = 'M';
    XTextExtents(ax->font, &c, 1, &direction, &ascent, &descent, &overall);
    for(i = 0; i < n-1; i++) {
	if(lats[i] > 0 && lats[i+1] > 0 && letter[i] != '\0') {
	    int ly = (lats[i] + lats[i+1] + ascent)/2 ;
	    XDrawString(XtDisplay(w), window, ax->axesGC, ix+4, ly,
					&letter[i], 1);
	}
    }
    Free(grid.segs);
}

static void
utmNearGrid(MapPlotWidget w, Window window)
{
    MapPlotPart	*mp = &w->map_plot;
    AxesPart *ax = &w->axes;
    int    i, j, n;
    Boolean lastin;
    double l, lon, lon_min, lon_max, lat, lat_min, lat_max;
    double minlon, maxlon, minlat, maxlat, x, y, z, dlon;
    SegArray grid;
    DrawStruct *d = &ax->d;

    grid.size_segs = 0;
    grid.nsegs = 0;
    grid.segs = NULL;
    d->s = &grid;
    SetClipArea(d, ax->x1[ax->zoom], ax->y1[ax->zoom], ax->x2[ax->zoom],
			ax->y2[ax->zoom]);

    if(mp->utm_cell_letter - 'N' >= 0) { // Northern Hemisphere
	unproject(w, ax->x1[ax->zoom], ax->y2[ax->zoom], &minlon, &y);
	unproject(w, ax->x2[ax->zoom], ax->y2[ax->zoom], &maxlon, &y);
    }
    else {
	unproject(w, ax->x1[ax->zoom], ax->y1[ax->zoom], &minlon, &y);
	unproject(w, ax->x2[ax->zoom], ax->y1[ax->zoom], &maxlon, &y);
    }
    l = .5*(ax->x1[ax->zoom] + ax->x2[ax->zoom]);
    unproject(w, l, ax->y1[ax->zoom], &x, &minlat);
    unproject(w, l, ax->y2[ax->zoom], &x, &maxlat);

    if(minlon > maxlon) {
	i = (int)minlon;
	minlon = maxlon;
	maxlon = i;
    }

    lon_min = 6*(int)(minlon/6) - 6;
    lon_max = 6*(int)(maxlon/6) + 6;
    n = 0;
    for(i = (int)lon_min; i <= (int)lon_max; i += 6)
    {
	lon = (double)i;
	l = (lon+180)-(int)((lon+180.)/360.)*360-180; // -180 < l < 180
	if(lon > minlon && lon < maxlon)
	{
	    lat_min = -80.;
	    if(l == 6) {
		lat_max = 56;
	    }
	    else if(l >= 12 && l <= 36) {
		lat_max = 72;
	    }
	    else {
		lat_max = 84.;
	    }

	    if(lat_min < minlat) lat_min = minlat;
	    if(lat_max > maxlat) lat_max = maxlat;

	    project(w, lon, lat_min, &x, &y, &z);
	    if(z > 0.) {
		imove(d, x, y);
		project(w, lon, lat_max, &x, &y, &z);
		if(z > 0.) idraw(d, x, y);
	    }
	}

	if(l == 6)
	{
	    lat_min = 56.;
	    lat_max = 64.;
	    project(w, lon-3., lat_min, &x, &y, &z);
	    if(z > 0.) {
		imove(d, x, y);
		project(w, lon-3., lat_max, &x, &y, &z);
		if(z > 0.) idraw(d, x, y);
	    }

	    lat_min = 64.;
	    lat_max = 72.;
	    project(w, lon, lat_min, &x, &y, &z);
	    if(z > 0.) {
		imove(d, x, y);
		project(w, lon, lat_max, &x, &y, &z);
		if(z > 0.) idraw(d, x, y);
	    }
	}
	if(l == 6 || l == 18 || l == 30) {
	    lat_min = 72.;
	    lat_max = 84.;
	    project(w, lon+3., lat_min, &x, &y, &z);
	    if(z > 0.) {
		imove(d, x, y);
		project(w, lon+3., lat_max, &x, &y, &z);
		if(z > 0.) idraw(d, x, y);
	    }
	}
    }

    dlon = (maxlon - minlon)/199.;

    n = 0;
    for(i = -80; i <= 72; i += 8) {
	lat = (double)i;
	if(lat > minlat && lat < maxlat) {
	    lastin = False;
	    for(j = 0; j < 200; j++) {
		lon = minlon + j*dlon;
		project(w, lon, lat, &x, &y, &z);
		if(z > 0.) {
		    if(!lastin) imove(d, x, y);
		    else idraw(d, x, y);
		    lastin = True;
		}
		else lastin = False;
	    }
	}
	n++;
    }
    lat = 84;
    if(lat > minlat && lat < maxlat) {
	lastin = False;
	for(j = 0; j < 200; j++) {
	    lon = minlon + j*dlon;
	    project(w, lon, lat, &x, &y, &z);
	    if(z > 0.) {
		if(!lastin) imove(d, x, y);
		else idraw(d, x, y);
		lastin = True;
	    }
	    else lastin = False;
	}
    }
    iflush(d);
    if(grid.nsegs) {
	XDrawSegments(XtDisplay(w), window, ax->axesGC,
			(XSegment *)grid.segs, grid.nsegs);
    }

    Free(grid.segs);
}

static void
polarGridLabels(MapPlotWidget w, Window window, int numaz, PolarLabel *az,
			int numdist, PolarLabel *dist)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int j;

	XSetForeground(XtDisplay(w), mp->symbolGC, ax->fg);

	XSetClipRectangles(XtDisplay(w), mp->symbolGC, 0,0,
					&ax->clip_rect, 1, Unsorted);
	for(j = 0; j < numaz; j++)
	{
	    char label[20];
	    int ascent, descent, direction, wd, ht, x, y;
	    double azimuth, d;
	    XCharStruct overall;

	    snprintf(label, sizeof(label), "%d", (int)az[j].value);
	    XTextExtents(ax->font, label, (int)strlen(label),
			&direction, &ascent, &descent, &overall);
	    wd = overall.width;
	    ht = ascent + descent;
	    azimuth = az[j].value*DEG_TO_RADIANS;
	    d = 2 + .5*ht*fabs(cos(azimuth)) + .5*wd*fabs(sin(azimuth));
	    x = (int)(az[j].x + d*sin(azimuth) - .5*wd);
	    y = (int)(az[j].y - d*cos(azimuth) - .5*ht + ascent);

	    XDrawString(XtDisplay(w), window, mp->symbolGC, x, y, label,
				(int)strlen(label));
	}
	for(j = 0; j < numdist; j++)
	{
	    char label[20];
	    int ascent, descent, direction, wd, x, y;
	    XCharStruct overall;

	    ftoa(dist[j].value, mp->polar_ndeci, 1, label, sizeof(label));
	    XTextExtents(ax->font, label, (int)strlen(label),
			&direction, &ascent, &descent, &overall);
	    wd = overall.width;
	    x = (int)(dist[j].x - .5*wd);
	    if(x <= ax->clip_rect.x) x = ax->clip_rect.x+1;
	    else if(x + wd > ax->clip_rect.x + ax->clip_rect.width)
		x = ax->clip_rect.x + ax->clip_rect.width - wd - 1;
	    y = dist[j].y + ascent + 2;

	    XDrawString(XtDisplay(w), window, mp->symbolGC, x, y, label,
				(int)strlen(label));
	}
}

static void
HardPolarGridLabels(MapPlotWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, int numaz, PolarLabel *az,
		int numdist, PolarLabel *dist)
{
	MapPlotPart	*mp = &w->map_plot;
	int j, wd, ht, x, y;
	char label[20];

	for(j = 0; j < numaz; j++)
	{
	    double azimuth, db;

	    snprintf(label, sizeof(label), "%d", (int)az[j].value);
	    wd = (int)strlen(label) * a->axis_font_width;
	    ht = a->axis_font_height;
	    azimuth = az[j].value*DEG_TO_RADIANS;
	    db = 10 + .5*ht*fabs(cos(azimuth)) + .5*wd*fabs(sin(azimuth));
	    x = (int)(az[j].x + db*sin(azimuth) - .5*wd);
	    y = (int)(az[j].y + db*cos(azimuth) - .5*ht);

	    fprintf(fp, "%d %d M (%s) %.5f P\n", x, y, label, font_scale);
	}
	for(j = 0; j < numdist; j++)
	{

	    ftoa(dist[j].value, mp->polar_ndeci, 1, label, sizeof(label));
	    wd = (int)strlen(label) * a->axis_font_width;
	    ht = a->axis_font_height;
	    x = (int)(dist[j].x - .5*wd);
	    y = dist[j].y - ht;

	    fprintf(fp, "%d %d M (%s) %.5f P\n", x, y, label, font_scale);
	}
}

/** 
 * 
 */
static void
RedisplayImageTheme(MapPlotWidget w, Window drawable, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;
	MapImage *mi;

	if(!t->display || !t->delta_on || t->theme_type != THEME_IMAGE) return;

	mi = &t->map_image;

	if(mi->mode != CONTOURS_ONLY)
	{
	    if(mp->image != NULL)
	    {
		int i;
		for(i = 0; i < mi->nparts; i++) {
		    XPutImage(XtDisplay(w), drawable, w->axes.axesGC, mp->image,
			mi->display_x0[i], mi->display_y0[i],
			mi->display_x0[i], mi->display_y0[i],
			mi->display_width[i], mi->display_height[i]);
		}
	    }
	    else if(t->theme.color_scale.num_colors > 0)
	    {
		mi->m.x1 = w->axes.x1[w->axes.zoom];
		mi->m.x2 = w->axes.x2[w->axes.zoom];
		mi->m.y1 = w->axes.y1[w->axes.zoom];
		mi->m.y2 = w->axes.y2[w->axes.zoom];
		shade(XtDisplay(w), drawable, mp->colorGC, &w->axes.d,
			t->theme.color_scale.num_colors,
			t->theme.color_scale.pixels,
			t->theme.color_scale.lines, 0, NULL, &mi->m);
	    }
	}
	if(mi->mode != COLOR_ONLY)
	{
	    XDrawSegments(XtDisplay(w), drawable, w->axes.axesGC,
			(XSegment *)mi->s.segs, mi->s.nsegs);
	    XDrawSegments(XtDisplay(w), drawable, mp->contour_labelGC,
			(XSegment *)mi->t.segs, mi->t.nsegs);
	}
}

static void
DrawSourceTags(MapPlotWidget w, Window window)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, x0, y0, ascent, descent, direction;
	XCharStruct	overall;
	double 		x, y, z;
	MapSource	*src;

	if(!XtIsRealized((Widget)w) || !mp->display_source_tags) return;

	XTextExtents(mp->label_font, "01234", 5, &direction, &ascent,
			&descent, &overall);

	for(i = 0; i < mp->nsrc; i++)
		if(mp->src[i]->source.sym.display >= MAP_ON ||
		  (mp->src[i]->source.sym.display == MAP_SELECTED_ON
		&& mp->src[i]->source.sym.selected))
	{
	    src = mp->src[i];
	    if(src->source.label != NULL)
	    {
		project(w, src->source.lon, src->source.lat, &x, &y,&z);
		if(z > 0.)
		{
		    if(src->source.sym.selected)
		    {
			XSetForeground(XtDisplay(w), mp->symbolGC,
			    ax->select_fg);
		    }
		    else
		    {
			XSetForeground(XtDisplay(w), mp->symbolGC,
			    src->source.sym.fg);
		    }
		    x0 = unscale_x(&ax->d, x) + src->source.sym.size/2 + 5;
		    y0 = (int)(unscale_y(&ax->d, y) +
			.5*(overall.ascent + overall.descent));
		    XDrawString(XtDisplay(w), window, mp->symbolGC,
			x0, y0, src->source.label,
			(int)strlen(src->source.label));
		}
	    }
	}
}

static void
MapPlotZoom(MapPlotWidget w, XtPointer client_data, XtPointer callback_data)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	if(ax->undraw)
	{
	    ax->axes_class->drawRectangle(ax->invert_clipGC,
			ax->zoom_x1, ax->zoom_y1,
			ax->zoom_x2 - ax->zoom_x1,
			ax->zoom_y2 - ax->zoom_y1);
	    ax->undraw = False;
	}
	if(!ax->axes_class->doZoom(ax->zoom_x1, ax->zoom_y1,
			ax->zoom_x2, ax->zoom_y2))
	{
	    return;
	}

	AxesWaitCursor((Widget)w, True);

	if(mp->projection == MAP_ORTHOGRAPHIC ||
	   mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
	   mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    if(mp->rotate_on_zoom) {
		OrthoRotate(w);
	    }
	}

	_AxesRedraw((AxesWidget)w);
	_MapPlotRedraw(w);
	RedisplayThemes(w);
	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	RedisplayOverlays(w, ax->pixmap);
	if(!ax->redisplay_pending)
	{
	    _AxesRedisplayAxes((AxesWidget)w);
	    _MapPlotDrawBar(w);
	    RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	    _AxesRedisplayXor((AxesWidget)w);
	}
	ax->zooming = False;
	AxesWaitCursor((Widget)w, False);
}

static void
OrthoRotate(MapPlotWidget w)
{
	AxesPart *ax = &w->axes;
	double	lat, lon, x0, y0, xdif, ydif;

	x0 = .5*(ax->x1[ax->zoom] + ax->x2[ax->zoom]);
	y0 = .5*(ax->y1[ax->zoom] + ax->y2[ax->zoom]);

	unproject(w, x0, y0, &lon, &lat);

	xdif = .5*(ax->x2[ax->zoom] - ax->x1[ax->zoom]);
	ydif = .5*(ax->y2[ax->zoom] - ax->y1[ax->zoom]);

	MapRotate(w, lon, lat);

	ax->x1[ax->zoom] = -xdif;
	ax->x2[ax->zoom] =  xdif;
	ax->y1[ax->zoom] = -ydif;
	ax->y2[ax->zoom] =  ydif;
}

static Boolean
RedisplaySymbol(MapPlotWidget w, Window window, double lon, double lat,
		SymbolInfo *sym)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		x0, y0;
	double		x, y, z, siz, h, v;
	GC		gc;
	XArc		arc;
	XSegment	segs[2];
	XPoint		points[5];

	if(!XtIsRealized((Widget)w)) return(False);

	project(w, lon, lat, &x, &y, &z);
	if(z <= 0.) return(False);

	gc = mp->symbolGC;

	if(!sym->selected)
	{
	    XSetForeground(XtDisplay(w), gc, sym->fg);
	}
	else
	{
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	}
	XSetClipRectangles(XtDisplay(w), gc, 0,0, &ax->clip_rect, 1, Unsorted);

	x0 = unscale_x(&ax->d, x);
	y0 = unscale_y(&ax->d, y);
	siz = sym->size;

	if(sym->type == CIRCLE || sym->type == FILLED_CIRCLE)
	{
	    arc.x = (short int)(x0 - siz);
	    arc.y = (short int)(y0 - siz);
	    arc.width = (short unsigned int)(2*siz);
	    arc.height = (short unsigned int)(2*siz);
	    arc.angle1 = 0;
	    arc.angle2 = 360*64;

	    if(sym->type == CIRCLE)
	    {
		XDrawArcs(XtDisplay(w), window, gc, &arc, 1);
	    }
	    else
	    {
		XFillArcs(XtDisplay(w), window, gc, &arc, 1);
	    }
	}
	else if(sym->type == SQUARE || sym->type == FILLED_SQUARE)
	{
	    points[0].x = (short int)(x0 - siz);
	    points[0].y = (short int)(y0 - siz);
	    points[1].x = (short int)(x0 + siz);
	    points[1].y = (short int)(y0 - siz);
	    points[2].x = (short int)(x0 + siz);
	    points[2].y = (short int)(y0 + siz);
	    points[3].x = (short int)(x0 - siz);
	    points[3].y = (short int)(y0 + siz);
	    points[4].x = (short int)(x0 - siz);
	    points[4].y = (short int)(y0 - siz);

	    if(sym->type == SQUARE)
	    {
		XDrawLines(XtDisplay(w), window, gc, points, 5,CoordModeOrigin);
	    }
	    else
	    {
		XFillPolygon(XtDisplay(w), window, gc, points, 5, Nonconvex,
				CoordModeOrigin);
	    }
	}
	else if(sym->type == TRIANGLE || sym->type == FILLED_TRIANGLE)
	{
	    h = .57735*siz + .5;	/* tan(30)*siz */
	    v = sqrt(h*h + siz*siz) + .5;

	    points[0].x = (short int)(x0 - siz);
	    points[0].y = (short int)(y0 + h);
	    points[1].x = (short int)(x0 + siz);
	    points[1].y = (short int)(y0 + h);
	    points[2].x = x0;
	    points[2].y = (short int)(y0 - v);
	    points[3].x = (short int)(x0 - siz);
	    points[3].y = (short int)(y0 + h);

	    if(sym->type == TRIANGLE)
	    {
		XDrawLines(XtDisplay(w), window, gc, points, 4,CoordModeOrigin);
	    }
	    else
	    {
		XFillPolygon(XtDisplay(w), window, gc, points, 4, Nonconvex,
				CoordModeOrigin);
	    }
	}
	else if(sym->type == INV_TRIANGLE || sym->type == FILLED_INV_TRIANGLE)
	{
	    h = .57735*siz + .5;	/* tan(30)*siz */
	    v = sqrt(h*h + siz*siz) + .5;

	    points[0].x = (short int)(x0 + siz);
	    points[0].y = (short int)(y0 - h);
	    points[1].x = (short int)(x0 - siz);
	    points[1].y = (short int)(y0 - h);
	    points[2].x = x0;
	    points[2].y = (short int)(y0 + v);
	    points[3].x = (short int)(x0 + siz);
	    points[3].y = (short int)(y0 - h);

	    if(sym->type == INV_TRIANGLE)
	    {
		XDrawLines(XtDisplay(w), window, gc, points, 4,CoordModeOrigin);
	    }
	    else
	    {
		XFillPolygon(XtDisplay(w), window, gc, points, 4, Nonconvex,
				CoordModeOrigin);
	    }
	}
	else if(sym->type == DIAMOND || sym->type == FILLED_DIAMOND)
	{
	    points[0].x = x0;
	    points[0].y = (short int)(y0 - siz);
	    points[1].x = (short int)(x0 - siz);
	    points[1].y = y0;
	    points[2].x = x0;
	    points[2].y = (short int)(y0 + siz);
	    points[3].x = (short int)(x0 + siz);
	    points[3].y = y0;
	    points[4].x = x0;
	    points[4].y = (short int)(y0 - siz);

	    if(sym->type == DIAMOND)
	    {
		XDrawLines(XtDisplay(w), window, gc, points, 5,CoordModeOrigin);
	    }
	    else
	    {
		XFillPolygon(XtDisplay(w), window, gc, points, 5, Nonconvex,
				CoordModeOrigin);
	    }
	}
	else if(sym->type == PLUS)
	{
	    segs[0].x1 = (short int)(x0 - siz);
	    segs[0].y1 = y0;
	    segs[0].x2 = (short int)(x0 + siz);
	    segs[0].y2 = y0;

	    segs[1].x1 = x0;
	    segs[1].y1 = (short int)(y0 - siz);
	    segs[1].x2 = x0;
	    segs[1].y2 = (short int)(y0 + siz);

	    XSetLineAttributes(XtDisplay(w), gc, 2, LineSolid, CapNotLast,
				JoinMiter);
	    XDrawSegments(XtDisplay(w), window, gc, segs, 2);
	    XSetLineAttributes(XtDisplay(w), gc, 0, LineSolid, CapNotLast,
				JoinMiter);
	}
	return(True);
}

static void
RedisplaySymbolGroup(MapPlotWidget w, Window window, MapSymbolGroup *s)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, n, x0, y0, siz, h, v;
	double		x, y, z;
	GC		gc;
	XArc		*arcs = NULL;
	XSegment	*segs = NULL;
	XPoint		points[10];
	SymbolInfo	*sym;

	if(!XtIsRealized((Widget)w)) return;

	if(mp->projection == MAP_POLAR && !s->group.polar_coords) return;

	gc = mp->symbolGC;

	sym = &s->group.sym;
	if(!sym->selected) {
	    XSetForeground(XtDisplay(w), gc, sym->fg);
	}
	else {
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	}

	XSetClipRectangles(XtDisplay(w), gc, 0,0, &ax->clip_rect, 1, Unsorted);

	if(sym->type == CIRCLE || sym->type == FILLED_CIRCLE)
	{
	    arcs = (XArc *)AxesMalloc((Widget)w, s->group.npts*sizeof(XArc));
		
	    for(i = n = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    arcs[n].x = x0 - siz/2;
		    arcs[n].y = y0 - siz/2;
		    arcs[n].width = siz;
		    arcs[n].height = siz;
		    arcs[n].angle1 = 0;
		    arcs[n].angle2 = 360*64;
		    n++;
		}
	    }
	    if(n > 0)
	    {
		if(sym->type == CIRCLE) {
		    XDrawArcs(XtDisplay(w), window, gc, arcs, n);
		}
		else {
		    XFillArcs(XtDisplay(w), window, gc, arcs, n);
		}
	    }
	    Free(arcs);
	}
	else if(sym->type == SQUARE)
	{
	    segs = (XSegment *)AxesMalloc((Widget)w,
				4*s->group.npts*sizeof(XSegment));
		
	    for(i = n = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    segs[n].x1 = x0 - siz;
		    segs[n].y1 = y0 - siz;
		    segs[n].x2 = x0 + siz;
		    segs[n].y2 = y0 - siz;
		    n++;
		    segs[n].x1 = x0 + siz;
		    segs[n].y1 = y0 - siz;
		    segs[n].x2 = x0 + siz;
		    segs[n].y2 = y0 + siz;
		    n++;
		    segs[n].x1 = x0 + siz;
		    segs[n].y1 = y0 + siz;
		    segs[n].x2 = x0 - siz;
		    segs[n].y2 = y0 + siz;
		    n++;
		    segs[n].x1 = x0 - siz;
		    segs[n].y1 = y0 + siz;
		    segs[n].x2 = x0 - siz;
		    segs[n].y2 = y0 - siz;
		    n++;
		}
	    }
	    if(n > 0)
	    {
		XDrawSegments(XtDisplay(w), window, gc, segs,n);
	    }
	    Free(segs);
	}
	else if(sym->type == FILLED_SQUARE)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];

		    points[0].x = x0 - siz;
		    points[0].y = y0 - siz;
		    points[1].x = x0 + siz;
		    points[1].y = y0 - siz;
		    points[2].x = x0 + siz;
		    points[2].y = y0 + siz;
		    points[3].x = x0 - siz;
		    points[3].y = y0 + siz;
		    points[4].x = x0 - siz;
		    points[4].y = y0 - siz;
		    XFillPolygon(XtDisplay(w), window, gc, points, 5,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym->type == TRIANGLE)
	{
	    segs = (XSegment *)AxesMalloc((Widget)w,
				3*s->group.npts*sizeof(XSegment));
		
	    for(i = n = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);
		    segs[n].x1 = x0 - siz;
		    segs[n].y1 = y0 + h;
		    segs[n].x2 = x0 + siz;
		    segs[n].y2 = y0 + h;
		    n++;
		    segs[n].x1 = x0 + siz;
		    segs[n].y1 = y0 + h;
		    segs[n].x2 = x0;
		    segs[n].y2 = y0 - v;
		    n++;
		    segs[n].x1 = x0;
		    segs[n].y1 = y0 - v;
		    segs[n].x2 = x0 - siz;
		    segs[n].y2 = y0 + h;
		    n++;
		}
	    }
	    if(n > 0)
	    {
		XDrawSegments(XtDisplay(w), window, gc, segs,n);
	    }
	    Free(segs);
	}
	else if(sym->type == FILLED_TRIANGLE)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

		    points[0].x = x0 - siz;
		    points[0].y = y0 + h;
		    points[1].x = x0 + siz;
		    points[1].y = y0 + h;
		    points[2].x = x0;
		    points[2].y = y0 - v;
		    points[3].x = x0 - siz;
		    points[3].y = y0 + h;
		    XFillPolygon(XtDisplay(w), window, gc, points, 4,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym->type == INV_TRIANGLE)
	{
	    segs = (XSegment *)AxesMalloc((Widget)w,
				3*s->group.npts*sizeof(XSegment));
		
	    for(i = n = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);
		    segs[n].x1 = x0 + siz;
		    segs[n].y1 = y0 - h;
		    segs[n].x2 = x0 - siz;
		    segs[n].y2 = y0 - h;
		    n++;
		    segs[n].x1 = x0 - siz;
		    segs[n].y1 = y0 - h;
		    segs[n].x2 = x0;
		    segs[n].y2 = y0 + v;
		    n++;
		    segs[n].x1 = x0;
		    segs[n].y1 = y0 + v;
		    segs[n].x2 = x0 + siz;
		    segs[n].y2 = y0 - h;
		    n++;
		}
	    }
	    if(n > 0)
	    {
		XDrawSegments(XtDisplay(w), window, gc, segs,n);
	    }
	    Free(segs);
	}
	else if(sym->type == FILLED_INV_TRIANGLE)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

		    points[0].x = x0 + siz;
		    points[0].y = y0 - h;
		    points[1].x = x0 - siz;
		    points[1].y = y0 - h;
		    points[2].x = x0;
		    points[2].y = y0 + v;
		    points[3].x = x0 + siz;
		    points[3].y = y0 - h;
		    XFillPolygon(XtDisplay(w), window, gc, points, 4,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym->type == DIAMOND)
	{
	    segs = (XSegment *)AxesMalloc((Widget)w,
				4*s->group.npts*sizeof(XSegment));
		
	    for(i = n = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    segs[n].x1 = x0;
		    segs[n].y1 = y0 - siz;
		    segs[n].x2 = x0 - siz;
		    segs[n].y2 = y0;
		    n++;
		    segs[n].x1 = x0 - siz;
		    segs[n].y1 = y0;
		    segs[n].x2 = x0;
		    segs[n].y2 = y0 + siz;
		    n++;
		    segs[n].x1 = x0;
		    segs[n].y1 = y0 + siz;
		    segs[n].x2 = x0 + siz;
		    segs[n].y2 = y0;
		    n++;
		    segs[n].x1 = x0 + siz;
		    segs[n].y1 = y0;
		    segs[n].x2 = x0;
		    segs[n].y2 = y0 - siz;
		    n++;
		}
	    }
	    if(n > 0)
	    {
		XDrawSegments(XtDisplay(w), window, gc, segs,n);
	    }
	    Free(segs);
	}
	else if(sym->type == FILLED_DIAMOND)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];

		    points[0].x = x0;
		    points[0].y = y0 - siz;
		    points[1].x = x0 - siz;
		    points[1].y = y0;
		    points[2].x = x0;
		    points[2].y = y0 + siz;
		    points[3].x = x0 + siz;
		    points[3].y = y0;
		    points[4].x = x0;
		    points[4].y = y0 - siz;
		    XFillPolygon(XtDisplay(w), window, gc, points, 5,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym->type == PLUS)
	{
	    segs = (XSegment *)AxesMalloc((Widget)w,
				2*s->group.npts*sizeof(XSegment));

	    XSetLineAttributes(XtDisplay(w), gc, 2, LineSolid,
					CapNotLast, JoinMiter);
		
	    for(i = n = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    siz = s->group.size[i];
		    segs[n].x1 = x0 - siz;
		    segs[n].y1 = y0;
		    segs[n].x2 = x0 + siz;
		    segs[n].y2 = y0;
		    n++;
		    segs[n].x1 = x0;
		    segs[n].y1 = y0 - siz;
		    segs[n].x2 = x0;
		    segs[n].y2 = y0 + siz;
		    n++;
		}
	    }
	    if(n > 0)
	    {
		XDrawSegments(XtDisplay(w), window, gc, segs, n);
	    }
	    XSetLineAttributes(XtDisplay(w), gc, 0, LineSolid,
						CapNotLast, JoinMiter);
	    Free(segs);
	}
}

static void
RedisplayMapSymbol(MapPlotWidget w, Window window, MapPlotSymbol *s)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		x0, y0, siz, h, v;
	double		x, y, z;
	GC		gc;
	XArc		arc;
	XSegment	segs[4];
	XPoint		points[10];
	SymbolInfo	*sym;

	if(!XtIsRealized((Widget)w)) return;

	if(mp->projection == MAP_POLAR && !s->polar_coords) return;

	gc = mp->symbolGC;

	sym = &s->sym;
	siz = sym->size;
	if(!sym->selected) {
	    XSetForeground(XtDisplay(w), gc, sym->fg);
	}
	else {
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	}

	XSetClipRectangles(XtDisplay(w), gc, 0,0, &ax->clip_rect, 1, Unsorted);

	if(sym->type == CIRCLE || sym->type == FILLED_CIRCLE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		arc.x = x0 - siz/2;
		arc.y = y0 - siz/2;
		arc.width = siz;
		arc.height = siz;
		arc.angle1 = 0;
		arc.angle2 = 360*64;
		if(sym->type == CIRCLE) {
		    XDrawArcs(XtDisplay(w), window, gc, &arc, 1);
		}
		else {
		    XFillArcs(XtDisplay(w), window, gc, &arc, 1);
		}
	    }
	}
	else if(sym->type == SQUARE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		segs[0].x1 = x0 - siz;
		segs[0].y1 = y0 - siz;
		segs[0].x2 = x0 + siz;
		segs[0].y2 = y0 - siz;

		segs[1].x1 = x0 + siz;
		segs[1].y1 = y0 - siz;
		segs[1].x2 = x0 + siz;
		segs[1].y2 = y0 + siz;

		segs[2].x1 = x0 + siz;
		segs[2].y1 = y0 + siz;
		segs[2].x2 = x0 - siz;
		segs[2].y2 = y0 + siz;

		segs[3].x1 = x0 - siz;
		segs[3].y1 = y0 + siz;
		segs[3].x2 = x0 - siz;
		segs[3].y2 = y0 - siz;

		XDrawSegments(XtDisplay(w), window, gc, segs, 4);
	    }
	}
	else if(sym->type == FILLED_SQUARE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);

		points[0].x = x0 - siz;
		points[0].y = y0 - siz;
		points[1].x = x0 + siz;
		points[1].y = y0 - siz;
		points[2].x = x0 + siz;
		points[2].y = y0 + siz;
		points[3].x = x0 - siz;
		points[3].y = y0 + siz;
		points[4].x = x0 - siz;
		points[4].y = y0 - siz;
		XFillPolygon(XtDisplay(w), window, gc, points, 5,
					Nonconvex, CoordModeOrigin);
	    }
	}
	else if(sym->type == TRIANGLE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		v = (int)(sqrt((double)(h*h + siz*siz)) + .5);
		segs[0].x1 = x0 - siz;
		segs[0].y1 = y0 + h;
		segs[0].x2 = x0 + siz;
		segs[0].y2 = y0 + h;

		segs[1].x1 = x0 + siz;
		segs[1].y1 = y0 + h;
		segs[1].x2 = x0;
		segs[1].y2 = y0 - v;

		segs[2].x1 = x0;
		segs[2].y1 = y0 - v;
		segs[2].x2 = x0 - siz;
		segs[2].y2 = y0 + h;

		XDrawSegments(XtDisplay(w), window, gc, segs, 3);
	    }
	}
	else if(sym->type == FILLED_TRIANGLE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

		points[0].x = x0 - siz;
		points[0].y = y0 + h;
		points[1].x = x0 + siz;
		points[1].y = y0 + h;
		points[2].x = x0;
		points[2].y = y0 - v;
		points[3].x = x0 - siz;
		points[3].y = y0 + h;
		XFillPolygon(XtDisplay(w), window, gc, points, 4,
					Nonconvex, CoordModeOrigin);
	    }
	}
	else if(sym->type == INV_TRIANGLE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		v = (int)(sqrt((double)(h*h + siz*siz)) + .5);
		segs[0].x1 = x0 + siz;
		segs[0].y1 = y0 - h;
		segs[0].x2 = x0 - siz;
		segs[0].y2 = y0 - h;

		segs[1].x1 = x0 - siz;
		segs[1].y1 = y0 - h;
		segs[1].x2 = x0;
		segs[1].y2 = y0 + v;

		segs[2].x1 = x0;
		segs[2].y1 = y0 + v;
		segs[2].x2 = x0 + siz;
		segs[2].y2 = y0 - h;
		XDrawSegments(XtDisplay(w), window, gc, segs, 3);
	    }
	}
	else if(sym->type == FILLED_INV_TRIANGLE)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

		points[0].x = x0 + siz;
		points[0].y = y0 - h;
		points[1].x = x0 - siz;
		points[1].y = y0 - h;
		points[2].x = x0;
		points[2].y = y0 + v;
		points[3].x = x0 + siz;
		points[3].y = y0 - h;
		XFillPolygon(XtDisplay(w), window, gc, points, 4,
					Nonconvex, CoordModeOrigin);
	    }
	}
	else if(sym->type == DIAMOND)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		segs[0].x1 = x0;
		segs[0].y1 = y0 - siz;
		segs[0].x2 = x0 - siz;
		segs[0].y2 = y0;

		segs[1].x1 = x0 - siz;
		segs[1].y1 = y0;
		segs[1].x2 = x0;
		segs[1].y2 = y0 + siz;

		segs[2].x1 = x0;
		segs[2].y1 = y0 + siz;
		segs[2].x2 = x0 + siz;
		segs[2].y2 = y0;

		segs[3].x1 = x0 + siz;
		segs[3].y1 = y0;
		segs[3].x2 = x0;
		segs[3].y2 = y0 - siz;
		XDrawSegments(XtDisplay(w), window, gc, segs, 4);
	    }
	}
	else if(sym->type == FILLED_DIAMOND)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);

		points[0].x = x0;
		points[0].y = y0 - siz;
		points[1].x = x0 - siz;
		points[1].y = y0;
		points[2].x = x0;
		points[2].y = y0 + siz;
		points[3].x = x0 + siz;
		points[3].y = y0;
		points[4].x = x0;
		points[4].y = y0 - siz;
		XFillPolygon(XtDisplay(w), window, gc, points, 5,
					Nonconvex, CoordModeOrigin);
	    }
	}
	else if(sym->type == PLUS)
	{
	    project(w, s->lon, s->lat, &x, &y, &z);
	    if(z > 0.)
	    {
		XSetLineAttributes(XtDisplay(w), gc, 2, LineSolid,
					CapNotLast, JoinMiter);
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		segs[0].x1 = x0 - siz;
		segs[0].y1 = y0;
		segs[0].x2 = x0 + siz;
		segs[0].y2 = y0;

		segs[1].x1 = x0;
		segs[1].y1 = y0 - siz;
		segs[1].x2 = x0;
		segs[1].y2 = y0 + siz;
		XDrawSegments(XtDisplay(w), window, gc, segs, 2);
		XSetLineAttributes(XtDisplay(w), gc, 0, LineSolid,
						CapNotLast, JoinMiter);
	    }
	}
}

static void
DisplayThemeSymbols(MapPlotWidget w, Window window, MapTheme *t, int ishape)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, i0, nshapes;
	ThemeDisplay	*td = t->td;
	
	if(!XtIsRealized((Widget)w)) return;

	if(ishape >= 0) {
	    i0 = ishape;
	    nshapes = ishape+1;
	}
	else {
	    i0 = 0;
	    nshapes = t->theme.nshapes;
	}
	XSetClipRectangles(XtDisplay(w), mp->symbolGC, 0,0, &ax->clip_rect,
			1, Unsorted);

/* if fg's are the same, could draw in arrays
 with XDrawArcs, etc for increased speed.
if(nshapes > 1)
for(i = i0+1; i < nshapes; i++) {
	if(t->theme.shape_fg[i] != t->theme.shape_fg[i-1]) break;
}
	etc.
*/
	XSetForeground(XtDisplay(w), mp->dataGC, ax->fg);
	for(i = i0; i < nshapes; i++) if(td[i].display)
	{
	    SHPObject *s = t->theme.shapes[i];
	    DisplayShapeSymbol(w, window, s, &td[i], t->theme.shape_fg[i],
			t->theme.symbol_type, t->theme.symbol_size);

	    if(t->display_labels) {
		DisplayShapeSymbolLabel(w, window, s, &td[i], mp->dataGC,
			t->theme.symbol_size);
	    }
	}
}

static void
DisplayShapeSymbol(MapPlotWidget w, Window window, SHPObject *s,
		ThemeDisplay *td, Pixel fg, int sym_type, int sym_size)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, x0, y0, h, v, n;
	double		x, y, z, lon, lat;
	GC		gc;
	XArc		arc;
	XSegment	segs[4];
	XPoint		points[10];

	gc = mp->symbolGC;

	if(!td->selected) {
	    XSetForeground(XtDisplay(w), gc, fg);
	}
	else {
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	}
	td->center_x = td->center_y = 0;
	n = 0;

	if(sym_type == CIRCLE || sym_type == FILLED_CIRCLE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];
		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    arc.x = x0 - sym_size;
		    arc.y = y0 - sym_size;
		    arc.width = 2*sym_size;
		    arc.height = 2*sym_size;
		    arc.angle1 = 0;
		    arc.angle2 = 360*64;

		    if(sym_type == CIRCLE) {
			XDrawArcs(XtDisplay(w), window, gc, &arc, 1);
		    }
		    else {
			XFillArcs(XtDisplay(w), window, gc, &arc, 1);
		    }
		}
	    }
	}
	else if(sym_type == SQUARE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];
		
		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    segs[0].x1 = x0 - sym_size;
		    segs[0].y1 = y0 - sym_size;
		    segs[0].x2 = x0 + sym_size;
		    segs[0].y2 = y0 - sym_size;

		    segs[1].x1 = x0 + sym_size;
		    segs[1].y1 = y0 - sym_size;
		    segs[1].x2 = x0 + sym_size;
		    segs[1].y2 = y0 + sym_size;

		    segs[2].x1 = x0 + sym_size;
		    segs[2].y1 = y0 + sym_size;
		    segs[2].x2 = x0 - sym_size;
		    segs[2].y2 = y0 + sym_size;

		    segs[3].x1 = x0 - sym_size;
		    segs[3].y1 = y0 + sym_size;
		    segs[3].x2 = x0 - sym_size;
		    segs[3].y2 = y0 - sym_size;

		    XDrawSegments(XtDisplay(w), window, gc, segs, 4);
		}
	    }
	}
	else if(sym_type == FILLED_SQUARE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;

		    points[0].x = x0 - sym_size;
		    points[0].y = y0 - sym_size;
		    points[1].x = x0 + sym_size;
		    points[1].y = y0 - sym_size;
		    points[2].x = x0 + sym_size;
		    points[2].y = y0 + sym_size;
		    points[3].x = x0 - sym_size;
		    points[3].y = y0 + sym_size;
		    points[4].x = x0 - sym_size;
		    points[4].y = y0 - sym_size;
		    XFillPolygon(XtDisplay(w), window, gc, points, 5,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym_type == TRIANGLE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    h = (int)(.57735*sym_size + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + sym_size*sym_size)) + .5);
		    segs[0].x1 = x0 - sym_size;
		    segs[0].y1 = y0 + h;
		    segs[0].x2 = x0 + sym_size;
		    segs[0].y2 = y0 + h;

		    segs[1].x1 = x0 + sym_size;
		    segs[1].y1 = y0 + h;
		    segs[1].x2 = x0;
		    segs[1].y2 = y0 - v;

		    segs[2].x1 = x0;
		    segs[2].y1 = y0 - v;
		    segs[2].x2 = x0 - sym_size;
		    segs[2].y2 = y0 + h;

		    XDrawSegments(XtDisplay(w), window, gc, segs, 3);
		}
	    }
	}
	else if(sym_type == FILLED_TRIANGLE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    h = (int)(.57735*sym_size + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + sym_size*sym_size)) + .5);

		    points[0].x = x0 - sym_size;
		    points[0].y = y0 + h;
		    points[1].x = x0 + sym_size;
		    points[1].y = y0 + h;
		    points[2].x = x0;
		    points[2].y = y0 - v;
		    points[3].x = x0 - sym_size;
		    points[3].y = y0 + h;
		    XFillPolygon(XtDisplay(w), window, gc, points, 4,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym_type == INV_TRIANGLE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    h = (int)(.57735*sym_size + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + sym_size*sym_size)) + .5);
		    segs[0].x1 = x0 + sym_size;
		    segs[0].y1 = y0 - h;
		    segs[0].x2 = x0 - sym_size;
		    segs[0].y2 = y0 - h;

		    segs[1].x1 = x0 - sym_size;
		    segs[1].y1 = y0 - h;
		    segs[1].x2 = x0;
		    segs[1].y2 = y0 + v;

		    segs[2].x1 = x0;
		    segs[2].y1 = y0 + v;
		    segs[2].x2 = x0 + sym_size;
		    segs[2].y2 = y0 - h;

		    XDrawSegments(XtDisplay(w), window, gc, segs, 3);
		}
	    }
	}
	else if(sym_type == FILLED_INV_TRIANGLE)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    h = (int)(.57735*sym_size + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + sym_size*sym_size)) + .5);

		    points[0].x = x0 + sym_size;
		    points[0].y = y0 - h;
		    points[1].x = x0 - sym_size;
		    points[1].y = y0 - h;
		    points[2].x = x0;
		    points[2].y = y0 + v;
		    points[3].x = x0 + sym_size;
		    points[3].y = y0 - h;
		    XFillPolygon(XtDisplay(w), window, gc, points, 4,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym_type == DIAMOND)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    segs[0].x1 = x0;
		    segs[0].y1 = y0 - sym_size;
		    segs[0].x2 = x0 - sym_size;
		    segs[0].y2 = y0;

		    segs[1].x1 = x0 - sym_size;
		    segs[1].y1 = y0;
		    segs[1].x2 = x0;
		    segs[1].y2 = y0 + sym_size;

		    segs[2].x1 = x0;
		    segs[2].y1 = y0 + sym_size;
		    segs[2].x2 = x0 + sym_size;
		    segs[2].y2 = y0;

		    segs[3].x1 = x0 + sym_size;
		    segs[3].y1 = y0;
		    segs[3].x2 = x0;
		    segs[3].y2 = y0 - sym_size;

		    XDrawSegments(XtDisplay(w), window, gc, segs, 4);
		}
	    }
	}
	else if(sym_type == FILLED_DIAMOND)
	{
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;

		    points[0].x = x0;
		    points[0].y = y0 - sym_size;
		    points[1].x = x0 - sym_size;
		    points[1].y = y0;
		    points[2].x = x0;
		    points[2].y = y0 + sym_size;
		    points[3].x = x0 + sym_size;
		    points[3].y = y0;
		    points[4].x = x0;
		    points[4].y = y0 - sym_size;
		    XFillPolygon(XtDisplay(w), window, gc, points, 5,
					Nonconvex, CoordModeOrigin);
		}
	    }
	}
	else if(sym_type == PLUS)
	{
	    XSetLineAttributes(XtDisplay(w), gc, 2, LineSolid,
					CapNotLast, JoinMiter);
	    for(i = 0; i < s->nVertices; i++)
	    {
		lon = s->padfX[i];
		lat = s->padfY[i];

		project(w, lon, lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    td->center_x += x0;
		    td->center_y += y0;
		    n++;
		    segs[0].x1 = x0 - sym_size;
		    segs[0].y1 = y0;
		    segs[0].x2 = x0 + sym_size;
		    segs[0].y2 = y0;

		    segs[1].x1 = x0;
		    segs[1].y1 = y0 - sym_size;
		    segs[1].x2 = x0;
		    segs[1].y2 = y0 + sym_size;

		    XDrawSegments(XtDisplay(w), window, gc, segs, 2);
		}
	    }
	    XSetLineAttributes(XtDisplay(w), gc, 0, LineSolid, CapNotLast,
			JoinMiter);
	}
	if(n > 0) {
	    td->center_x /= n;
	    td->center_y /= n;
	}
}

static void
DisplayShapeSymbolLabel(MapPlotWidget w, Window window, SHPObject *s,
		ThemeDisplay *td, GC gc, int sym_size)
{
	int ascent, descent, direction, x, y;
	XCharStruct overall;

	if(td->label) {
	    XTextExtents(w->axes.font, td->label, (int)strlen(td->label),
			&direction, &ascent, &descent, &overall);
	    x = (int)(td->center_x - .5*overall.width);
	    y = td->center_y - sym_size - descent - 2;
	    XDrawString(XtDisplay(w), window, gc,
			x, y, td->label, (int)strlen(td->label));
	}
}

static void
DisplayThemePolygonLabel(MapPlotWidget w, Window window, GC gc,
			ThemeDisplay *td)
{
	int i, j, width, ymin, ymax, yd, ydmin, imin, ymid, width_max, wd;
	int ascent, descent, direction, x, y, reset;
	XCharStruct overall;
	DSegment *s;

	td->display_label = False;

	if(td->label) {
	    XTextExtents(w->axes.font, td->label, (int)strlen(td->label),
			&direction, &ascent, &descent, &overall);
	    s = td->fill_segs.segs;
	    ymin = w->core.height;
	    ymax = 0;
	    width_max = 0;
	    reset = 0;
	    for(i = 0; i < td->fill_segs.nsegs; i = j)
	    {
		width = s[i].x2 - s[i].x1;
		for(j = i+1; j < td->fill_segs.nsegs && s[j].y1==s[i].y1; j++) {
		    wd = s[j].x2 - s[j].x1;
		    if(wd > width) width = wd;
		}
		if(width > overall.width) {
		    if(reset && width > width_max) {
			ymin = ymax = s[i].y1;
			reset = 0;
		    }
		    else {
			if(s[i].y1 < ymin) ymin = s[i].y1;
			if(s[i].y1 > ymax) ymax = s[i].y1;
		    }
		    if(width > width_max) width_max = width;
		}
		else {
		    reset = 1;
		}
	    }
	    if(ymin == w->core.height || ymax == 0) return;

	    ymid = (ymin + ymax)/2;

	    imin = -1;
	    ydmin = w->core.height;
	    for(i = 0; i < td->fill_segs.nsegs; i++) {
		width = s[i].x2 - s[i].x1;
		if(width > overall.width) {
		    yd = abs(s[i].y1 - ymid);
		    if(yd < ydmin) {
			ydmin = yd;
			imin = i;
		    }
		}
	    }
	    if(imin >= 0) {
		x = (int)(.5*(s[imin].x1 + s[imin].x2) - .5*overall.width);
		y = s[imin].y1 + ascent/2;
		XDrawString(XtDisplay(w), window, gc,
			x, y, td->label, (int)strlen(td->label));
		td->display_label = True;
		x = (int)(.5*(s[imin].x1 + s[imin].x2));
		td->label_x = scale_x(&w->axes.d, x);
		td->label_y = scale_y(&w->axes.d, y);
	    }
	}
}

static void
DrawHardSymbols(MapPlotWidget w, FILE *fp, DrawStruct *d, MapSymbolGroup *s,
		Boolean do_color)
{
	AxesPart	*ax = &w->axes;
	int		i, x0, y0, siz, h, v;
	char		*fill;
	double		x, y, z;
	SymbolInfo	*sym;

	if(!XtIsRealized((Widget)w)) return;

	sym = &s->group.sym;

	if(do_color)
	{
	    if(!sym->selected)
	    {
		AxesPostColor((AxesWidget)w, fp, sym->fg);
	    }
	    else
	    {
		AxesPostColor((AxesWidget)w, fp, ax->select_fg);
	    }
	}

	if(sym->type == CIRCLE || sym->type == FILLED_CIRCLE)
	{
	    fill = (sym->type == CIRCLE) ? (char*)"stroke" : (char*)"fill";

	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(d, x);
		    y0 = unscale_y(d, y);
		    siz = (int)(1.875*s->group.size[i]);
		    fprintf(fp, "N %d %d %d 0 360 arc %s C\n",
					x0, y0, siz, fill);
		}
	    }
	}
	else if(sym->type == SQUARE || sym->type == FILLED_SQUARE)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(d, x);
		    y0 = unscale_y(d, y);
		    siz = (int)(3.75*s->group.size[i]);
		    if(sym->type == SQUARE)
		    {
			fprintf(fp, "%d %d M\n", x0 - siz, y0 - siz);
			fprintf(fp, "%d %d D\n", x0 - siz, y0 + siz);
			fprintf(fp, "%d %d D\n", x0 + siz, y0 + siz);
			fprintf(fp, "%d %d D\n", x0 + siz, y0 - siz);
			fprintf(fp, "%d %d D\n", x0 - siz, y0 - siz);
			fprintf(fp, "stroke\n");
		    }
		    else
		    {
			fprintf(fp, "N %d %d M\n",x0 - siz, y0 - siz);
			fprintf(fp, "%d %d d\n",  x0 - siz, y0 + siz);
			fprintf(fp, "%d %d d\n",  x0 + siz, y0 + siz);
			fprintf(fp, "%d %d d\n",  x0 + siz, y0 - siz);
			fprintf(fp, "%d %d d\n",  x0 - siz, y0 - siz);
			fprintf(fp, "fill\n");
		    }
		}
	    }
	}
	else if(sym->type == TRIANGLE || sym->type == FILLED_TRIANGLE)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(d, x);
		    y0 = unscale_y(d, y);
		    siz = (int)(3.75*s->group.size[i]);
		    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

		    if(sym->type == TRIANGLE)
		    {
			fprintf(fp, "%d %d M\n", x0 - siz, y0 - h);
			fprintf(fp, "%d %d D\n", x0, y0 + v);
			fprintf(fp, "%d %d D\n", x0 + siz, y0 - h);
			fprintf(fp, "%d %d D\n", x0 - siz, y0 - h);
			fprintf(fp, "stroke\n");
		    }
		    else
		    {
			fprintf(fp, "N %d %d M\n",  x0 - siz, y0 - h);
			fprintf(fp, "%d %d d\n", x0, y0 + v);
			fprintf(fp, "%d %d d\n", x0 + siz, y0 - h);
			fprintf(fp, "%d %d d\n", x0 - siz, y0 - h);
			fprintf(fp, "fill\n");
		    }
		}
	    }
	}
	else if(sym->type == INV_TRIANGLE || sym->type == FILLED_INV_TRIANGLE)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(d, x);
		    y0 = unscale_y(d, y);
		    siz = (int)(3.75*s->group.size[i]);
		    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
		    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

		    if(sym->type == INV_TRIANGLE)
		    {
			fprintf(fp, "%d %d M\n", x0, y0 - v);
			fprintf(fp, "%d %d D\n", x0 - siz, y0 + h);
			fprintf(fp, "%d %d D\n", x0 + siz, y0 + h);
			fprintf(fp, "%d %d D\n", x0, y0 - v);
			fprintf(fp, "stroke\n");
		    }
		    else
		    {
			fprintf(fp, "N %d %d M\n", x0, y0 - v);
			fprintf(fp, "%d %d d\n", x0 - siz, y0 + h);
			fprintf(fp, "%d %d d\n", x0 + siz, y0 + h);
			fprintf(fp, "%d %d d\n", x0, y0 - v);
			fprintf(fp, "fill\n");
		    }
		}
	    }
	}
	else if(sym->type == DIAMOND || sym->type == FILLED_DIAMOND)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(d, x);
		    y0 = unscale_y(d, y);
		    siz = (int)(3.75*s->group.size[i]);

		    if(sym->type == DIAMOND)
		    {
			fprintf(fp, "%d %d M\n",  x0, y0 - siz);
			fprintf(fp, "%d %d D\n", x0 - siz, y0);
			fprintf(fp, "%d %d D\n", x0, y0 + siz);
			fprintf(fp, "%d %d D\n", x0 + siz, y0);
			fprintf(fp, "%d %d D\n", x0, y0 - siz);
			fprintf(fp, "stroke\n");
		    }
		    else
		    {
			fprintf(fp, "N %d %d M\n",  x0, y0 - siz);
			fprintf(fp, "%d %d d\n", x0 - siz, y0);
			fprintf(fp, "%d %d d\n", x0, y0 + siz);
			fprintf(fp, "%d %d d\n", x0 + siz, y0);
			fprintf(fp, "%d %d d\n", x0, y0 - siz);
			fprintf(fp, "fill\n");
		    }
		}
	    }
	}
	else if(sym->type == PLUS)
	{
	    for(i = 0; i < s->group.npts; i++)
	    {
		project(w, s->group.lon[i], s->group.lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(d, x);
		    y0 = unscale_y(d, y);
		    siz = (int)(3.75*s->group.size[i]);
		    fprintf(fp, "%d %d M\n", x0 - siz, y0);
		    fprintf(fp, "%d %d D\n", x0 + siz, y0);
		    fprintf(fp, "%d %d M\n", x0, y0 - siz);
		    fprintf(fp, "%d %d D\n", x0, y0 + siz);
		}
	    }
	}
	fprintf(fp, "initclip\n");
}

static Boolean
DrawHardSymbol(MapPlotWidget w, FILE *fp, DrawStruct *d, double lon, double lat,
		SymbolInfo *sym, Boolean do_color)
{
	AxesPart	*ax = &w->axes;
	int		x0, y0, siz, h, v;
	char		*fill;
	double		x, y, z;

	if(!XtIsRealized((Widget)w)) return(False);

	project(w, lon, lat, &x, &y, &z);
	if(z <= 0.) return(False);

	x0 = unscale_x(d, x);
	y0 = unscale_y(d, y);
	siz = (int)(3.75*sym->size);

	if(do_color)
	{
	    if(!sym->selected)
	    {
		AxesPostColor((AxesWidget)w, fp, sym->fg);
	    }
	    else
	    {
		AxesPostColor((AxesWidget)w, fp, ax->select_fg);
	    }
	}

	if(sym->type == CIRCLE || sym->type == FILLED_CIRCLE)
	{
	    fill = (sym->type == CIRCLE) ? (char*)"stroke" : (char*)"fill";

	    siz = (int)(1.875*sym->size);
	    fprintf(fp, "N %d %d %d 0 360 arc %s C\n", x0, y0, siz, fill);
	    /* move for source tag if drawn */
	    fprintf(fp, "%d %d M\n", x0 + siz + 10, y0 - 5);
	    return(True);
	}
	else if(sym->type == SQUARE || sym->type == FILLED_SQUARE)
	{
	    if(sym->type == SQUARE)
	    {
		fprintf(fp, "%d %d M\n", x0 - siz, y0 - siz);
		fprintf(fp, "%d %d D\n", x0 - siz, y0 + siz);
		fprintf(fp, "%d %d D\n", x0 + siz, y0 + siz);
		fprintf(fp, "%d %d D\n", x0 + siz, y0 - siz);
		fprintf(fp, "%d %d D\n", x0 - siz, y0 - siz);
		fprintf(fp, "stroke\n");
	    }
	    else
	    {
		fprintf(fp, "N %d %d M\n",x0- siz, y0 - siz);
		fprintf(fp, "%d %d d\n", x0 - siz, y0 + siz);
		fprintf(fp, "%d %d d\n", x0 + siz, y0 + siz);
		fprintf(fp, "%d %d d\n", x0 + siz, y0 - siz);
		fprintf(fp, "%d %d d\n", x0 - siz, y0 - siz);
		fprintf(fp, "fill\n");
	    }
	}
	else if(sym->type == TRIANGLE || sym->type == FILLED_TRIANGLE)
	{
	    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
	    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

	    if(sym->type == TRIANGLE)
	    {
		fprintf(fp, "%d %d M\n",  x0 - siz, y0 - h);
		fprintf(fp, "%d %d D\n", x0, y0 + v);
		fprintf(fp, "%d %d D\n", x0 + siz, y0 - h);
		fprintf(fp, "%d %d D\n", x0 - siz, y0 - h);
		fprintf(fp, "stroke\n");
	    }
	    else
	    {
		fprintf(fp, "N %d %d M\n",  x0 - siz, y0 - h);
		fprintf(fp, "%d %d d\n", x0, y0 + v);
		fprintf(fp, "%d %d d\n", x0 + siz, y0 - h);
		fprintf(fp, "%d %d d\n", x0 - siz, y0 - h);
		fprintf(fp, "fill\n");
	    }
	}
	else if(sym->type == INV_TRIANGLE || sym->type == FILLED_INV_TRIANGLE)
	{
	    h = (int)(.57735*siz + .5);	/* tan(30)*siz */
	    v = (int)(sqrt((double)(h*h + siz*siz)) + .5);

	    if(sym->type == INV_TRIANGLE)
	    {
		fprintf(fp, "%d %d M\n", x0, y0 - v);
		fprintf(fp, "%d %d D\n", x0 - siz, y0 + h);
		fprintf(fp, "%d %d D\n", x0 + siz, y0 + h);
		fprintf(fp, "%d %d D\n", x0, y0 - v);
		fprintf(fp, "stroke\n");
	    }
	    else
	    {
		fprintf(fp, "N %d %d M\n", x0, y0 - v);
		fprintf(fp, "%d %d d\n", x0 - siz, y0 + h);
		fprintf(fp, "%d %d d\n", x0 + siz, y0 + h);
		fprintf(fp, "%d %d d\n", x0, y0 - v);
		fprintf(fp, "fill\n");
	    }
	}
	else if(sym->type == DIAMOND || sym->type == FILLED_DIAMOND)
	{
	    if(sym->type == DIAMOND)
	    {
		fprintf(fp, "%d %d M\n",  x0, y0 - siz);
		fprintf(fp, "%d %d D\n", x0 - siz, y0);
		fprintf(fp, "%d %d D\n", x0, y0 + siz);
		fprintf(fp, "%d %d D\n", x0 + siz, y0);
		fprintf(fp, "%d %d D\n", x0, y0 - siz);
		fprintf(fp, "stroke\n");
	    }
	    else
	    {
		fprintf(fp, "N %d %d M\n",  x0, y0 - siz);
		fprintf(fp, "%d %d d\n", x0 - siz, y0);
		fprintf(fp, "%d %d d\n", x0, y0 + siz);
		fprintf(fp, "%d %d d\n", x0 + siz, y0);
		fprintf(fp, "%d %d d\n", x0, y0 - siz);
		fprintf(fp, "fill\n");
	    }
	}
	else if(sym->type == PLUS)
	{
	    fprintf(fp, "%d %d M\n", x0 - siz, y0);
	    fprintf(fp, "%d %d D\n", x0 + siz, y0);
	    fprintf(fp, "%d %d M\n", x0, y0 - siz);
	    fprintf(fp, "%d %d D\n", x0, y0 + siz);
	}
	/* move for source tag if drawn */
	fprintf(fp, "%d %d M\n", x0 + siz + 10, y0 - 5);
	return(True);
}

static void
RedisplayLine(MapPlotWidget w, Window window, LineInfo *line, SegArray *s)
{
	AxesPart *ax = &w->axes;
	GC gc;

	if(!XtIsRealized((Widget)w)) return;

	gc = line->xorr ? ax->mag_invertGC : w->map_plot.lineGC;

	if(!line->selected)
	{
	    XSetForeground(XtDisplay(w), gc, line->fg);
	    if(line->xorr) {
		XSetPlaneMask(XtDisplay(w), gc,
			line->fg ^ w->core.background_pixel);
	    }
	}
	else
	{
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	    if(line->xorr) {
		XSetPlaneMask(XtDisplay(w), gc,
			ax->select_fg ^ w->core.background_pixel);
	    }
	}

	XSetLineAttributes(XtDisplay(w), gc, line->line_width, line->line_style,
		line->cap_style, line->join_style);
	XSetClipRectangles(XtDisplay(w), gc,
		0,0, &ax->clip_rect, 1, Unsorted);

	if(line->line_style == LineOnOffDash ||
	   line->line_style == LineDoubleDash)
	{
	    XSetDashes(XtDisplay(w), gc, line->dash_offset, line->dashes, 
			line->n_dashes);
	    if(line->line_style == LineDoubleDash)
	    {
		XSetBackground(XtDisplay(w), gc, line->bg);
	    }
	}
	XDrawSegments(XtDisplay(w), window, gc, (XSegment *)s->segs, s->nsegs);
	XSetLineAttributes(XtDisplay(w), gc, 0, LineSolid, CapNotLast,
				JoinMiter);
}

static void
RedisplayPoly(MapPlotWidget w, Window window, MapPlotPolygon *poly)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i;
	GC	gc;
	XPoint	*points = NULL;
	double	x, y, z;
	SymbolInfo	*sym;

	if(!XtIsRealized((Widget)w)) return;

	gc = mp->symbolGC;

	sym = &poly->sym;
	if(!sym->selected)
	{
	    XSetForeground(XtDisplay(w), gc, sym->fg);
	}
	else
	{
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	}
	XSetClipRectangles(XtDisplay(w), gc, 0,0, &ax->clip_rect, 1,
				Unsorted);

	points = (XPoint *)AxesMalloc((Widget)w, poly->npts*sizeof(XPoint));

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
		mp->projection == MAP_UTM)
	{
	    for(i = 0; i < poly->npts; i++)
	    {
		x = poly->lon[i];
		if(x < ax->x1[0]) x += 360;
		else if(x > ax->x2[0]) x -= 360;
		y = poly->lat[i];
		points[i].x = unscale_x(&ax->d, x);
		points[i].y = unscale_y(&ax->d, y);
	    }
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	        mp->projection == MAP_MERCATOR)
	{
	    for(i = 0; i < poly->npts; i++)
	    {
		project(w, poly->lon[i], poly->lat[i], &x, &y, &z);
		x = poly->lon[i];
		if(x < ax->x1[0]) x += 360;
		else if(x > ax->x2[0]) x -= 360;
		points[i].x = unscale_x(&ax->d, x);
		points[i].y = unscale_y(&ax->d, y);
	    }
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC ||
		mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
		mp->projection == MAP_AZIMUTHAL_EQUAL_AREA ||
		mp->projection == MAP_POLAR ||
		mp->projection == MAP_UTM_NEAR)
	{
	    for(i = 0; i < poly->npts; i++)
	    {
		project(w, poly->lon[i], poly->lat[i], &x, &y, &z);
		if(z <= 0)
		{
		    Free(points);
		    return;
		}
		points[i].x = unscale_x(&ax->d, x);
		points[i].y = unscale_y(&ax->d, y);
	    }
	}

	XFillPolygon(XtDisplay(w), window, gc, points, poly->npts, Nonconvex,
			CoordModeOrigin);
	Free(points);
}

static void
RedisplayRectangle(MapPlotWidget w, Window window, MapPlotRectangle *rect)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	x0, y0, width, height;
	GC	gc;
	XPoint	points[4];
	double	x, y, z;
	SymbolInfo *sym;

	if(!XtIsRealized((Widget)w)) return;

	gc = mp->symbolGC;

	sym = &rect->sym;
	if(!sym->selected) {
	    XSetForeground(XtDisplay(w), gc, sym->fg);
	}
	else {
	    XSetForeground(XtDisplay(w), gc, ax->select_fg);
	}
	XSetClipRectangles(XtDisplay(w), gc, 0,0, &ax->clip_rect, 1,
				Unsorted);

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
		mp->projection == MAP_UTM)
	{
	    x = rect->lon;
	    if(x < ax->x1[0]) x += 360;
	    else if(x > ax->x2[0]) x -= 360;
	    y = rect->lat;
	    x0 = unscale_x(&ax->d, x);
	    y0 = unscale_y(&ax->d, y);
	    width = unscale_x(&ax->d, x+rect->width) - x0;
	    height = unscale_y(&ax->d, y+rect->height) - y0;
	    XFillRectangle(XtDisplay(w), window, gc, x0, y0, width, height);
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	        mp->projection == MAP_MERCATOR)
	{
	    project(w, rect->lon, rect->lat, &x, &y, &z);
	    x = rect->lon;
	    if(x < ax->x1[0]) x += 360;
	    else if(x > ax->x2[0]) x -= 360;
	    x0 = unscale_x(&ax->d, x);
	    y0 = unscale_y(&ax->d, y);
	    project(w, rect->lon+rect->width, rect->lat, &x, &y, &z);
	    width = unscale_x(&ax->d, rect->lon+rect->width) - x0;
	    height = unscale_x(&ax->d, y) - y0;
	    XFillRectangle(XtDisplay(w), window, gc, x0, y0, width, height);
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC ||
		mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
		mp->projection == MAP_AZIMUTHAL_EQUAL_AREA ||
		mp->projection == MAP_POLAR ||
		mp->projection == MAP_UTM_NEAR)
	{
	    project(w, rect->lon, rect->lat, &x, &y, &z);
	    if(z <= 0) return;
	    points[0].x = unscale_x(&ax->d, x);
	    points[0].y = unscale_y(&ax->d, y);

	    project(w, rect->lon+rect->width, rect->lat, &x, &y, &z);
	    if(z <= 0) return;
	    points[1].x = unscale_x(&ax->d, x);
	    points[1].y = unscale_y(&ax->d, y);

	    project(w, rect->lon+rect->width, rect->lat+rect->height,&x,&y,&z);
	    if(z <= 0) return;
	    points[2].x = unscale_x(&ax->d, x);
	    points[2].y = unscale_y(&ax->d, y);

	    project(w, rect->lon, rect->lat+rect->height, &x, &y, &z);
	    if(z <= 0) return;
	    points[3].x = unscale_x(&ax->d, x);
	    points[3].y = unscale_y(&ax->d, y);

	    XFillPolygon(XtDisplay(w), window, gc, points, 4, Nonconvex,
			CoordModeOrigin);
	}
}

static void
DrawHardPoly(MapPlotWidget w, FILE *fp, DrawStruct *d, MapPlotPolygon *poly,
		Boolean do_color)
{ 
	AxesPart *ax = &w->axes;
	int	i, x0=0, y0=0, x1=0, y1=0;
	double	x, y, z;

	if(!XtIsRealized((Widget)w)) return;

	for(i = 0; i < poly->npts; i++)
	{
	    project(w, poly->lon[i], poly->lat[i], &x, &y, &z);
	    if(z <= 0) return;
	}
	if(do_color)
	{
	    if(!poly->sym.selected)
	    {
		AxesPostColor((AxesWidget)w, fp, poly->sym.fg);
	    }
	    else
	    {
		AxesPostColor((AxesWidget)w, fp, ax->select_fg);
	    }
	}
	for(i = 0; i < poly->npts; i++)
	{
	    project(w, poly->lon[i], poly->lat[i], &x, &y, &z);
	    x0 = unscale_x(d, x);
	    y0 = unscale_y(d, y);
	    if(i == 0)
	    {
		fprintf(fp, "N %d %d M\n", x0, y0);
		x1 = x0;
		y1 = y0;
	    }
	    else
	    {
		fprintf(fp, "%d %d d\n", x0, y0);
	    }
	}
	if(poly->npts > 0)
	{
	    if(x0 != x1 || y0 != y1) fprintf(fp, "C ");

	    fprintf(fp, "fill\n");
	}
}

static void
DrawCircles(MapPlotWidget w)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, k, l, n, nsegs;
	Boolean		last_in;
	double		x, y, z, x0, y0, del, cs[2000], sn[2000];
	double		lastx, lasty, theta, phi;
	double		xmin, xmax, ymin, ymax;
	double		x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
	double		m[3][3], m1[3][3], alpha, beta, gamma, r, sum;
	DrawStruct	*d;
	MapCircle	*c;

	if(!XtIsRealized((Widget)w)|| mp->ncircles <= 0) return;

	d = &ax->d;
	Free(mp->circles.segs);
	mp->circles.size_segs = 0;
	mp->circles.nsegs = 0;

	if(mp->projection == MAP_POLAR) return;

	d->s = &mp->circles;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];
	SetClipArea(d, xmin, ymin, xmax, ymax);

	c = mp->circle;
	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    n = unscale_x(d, xmin + (double)c->radius) - unscale_x(d, xmin);
	}
/** fix for MAP_MERCATOR **/
	else
	{
	    n = unscale_x(d, sin(c->radius*rad)) - unscale_x(d, 0.);
	}
	if(n > 2000) n = 2000;
	if(n <= 10) n = 10;
	del = 2.*pi/(n-1);

	for(i = 0; i < n; i++)
	{
	    cs[i] = cos(i*del);
	    sn[i] = sin(i*del);
	}

	for(i = 0; i < mp->ncircles; i++)
	{
	    mp->num_circle_segs[i] = 0;
	}
	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
		mp->projection == MAP_UTM_NEAR)
	{
	    for(i = 0; i < mp->ncircles; i++) if(c[i].display)
	    {
		mp->num_circle_segs[i] = 0;
		nsegs = d->s->nsegs;

		x0 = c[i].lon - c[i].radius;
		if(x0 < ax->x1[0]) x0 += 360.;
		else if(x0 > ax->x2[0]) x0 -= 360.;
		x = c[i].lon + c[i].radius;
		if(x < ax->x1[0]) x += 360.;
		else if(x > ax->x2[0]) x -= 360.;

		if(x < xmin || x0 > xmax || c[i].lat + c[i].radius < ymin ||
		   c[i].lat - c[i].radius > ymax) continue;

		alpha = c[i].lon*rad;
		beta = (90. - c[i].lat)*rad;
		gamma = 0.;
		rotation_matrix(alpha, beta, gamma, m);

		r = sin(c[i].radius*rad);
		z1 = cos(c[i].radius*rad);

		x1 = r;
		y1 = 0.;
		x = m[0][0]*x1 + m[0][1]*y1 + m[0][2]*z1;
		y = m[1][0]*x1 + m[1][1]*y1 + m[1][2]*z1;
		z = m[2][0]*x1 + m[2][1]*y1 + m[2][2]*z1;
		theta = atan2(sqrt(x*x+y*y), z);
		phi = atan2(y, x);
		theta = 90. - theta/rad;
		phi /= rad;
		imove(d, phi, theta);
		lastx = phi;
		lasty = theta;

		for(j = 1; j < n; j++)
		{
		    x1 = r*cs[j];
		    y1 = r*sn[j];
		    x = m[0][0]*x1 + m[0][1]*y1 + m[0][2]*z1;
		    y = m[1][0]*x1 + m[1][1]*y1 + m[1][2]*z1;
		    z = m[2][0]*x1 + m[2][1]*y1 + m[2][2]*z1;
		    theta = atan2(sqrt(x*x+y*y), z);
		    phi = atan2(y, x);
		    theta = 90. - theta/rad;
		    phi /= rad;
		    if(phi < ax->x1[0]) phi += 360.;
		    else if(phi > ax->x2[0]) phi -= 360.;
		    if(fabs(phi - lastx) > 180.)
		    {
			if(lastx > mp->phi)
			{
			    idraw(d, phi+360., theta);
			    imove(d, lastx-360., lasty);
			}
			else
			{
			    idraw(d, phi-360., theta);
			    imove(d, lastx+360., lasty);
			}
		    }
		    idraw(d, phi, theta);
		    lastx = phi;
		    lasty = theta;
		}
		iflush(d);
		if(d->s->nsegs > nsegs)
		{
		    mp->num_circle_segs[i] = d->s->nsegs - nsegs;
		    mp->circle_start[i] = nsegs;
		}
	    }
	}
	else
	{
	    for(i = 0; i < mp->ncircles; i++) if(c[i].display)
	    {
		mp->num_circle_segs[i] = 0;
		nsegs = d->s->nsegs;
		x0 = c[i].lon - c[i].radius;
		x  = c[i].lon + c[i].radius;
		y0 = c[i].lat - c[i].radius;
		y  = c[i].lat + c[i].radius;
		project(w, x0, y0, &x1, &y1, &z1);
		project(w, x0,  y, &x2, &y2, &z2);
		project(w, x,  y0, &x3, &y3, &z3);
		project(w, x,   y, &x4, &y4, &z4);
		if((z1 < 0. || fabs(x1) > xmax || fabs(y1) > ymax) &&
		   (z2 < 0. || fabs(x2) > xmax || fabs(y2) > ymax) &&
		   (z3 < 0. || fabs(x3) > xmax || fabs(y3) > ymax) &&
		   (z4 < 0. || fabs(x4) > xmax || fabs(y4) > ymax))
		{
		    continue;
		}
		alpha = c[i].lon*rad;
		beta = (90. - c[i].lat)*rad;
		gamma = 0.;
		rotation_matrix(alpha, beta, gamma, m1);

		for(l = 0; l < 3; l++)
		{
		    for(j = 0; j < 3; j++)
		    {
			for(k = 0, sum = 0.; k < 3; k++)
			   sum += mp->c[l][k]*m1[k][j];
			m[l][j] = sum;
		    }
		}
		r = sin(c[i].radius*rad);
		z1 = cos(c[i].radius*rad);

		for(j = 0, last_in = False; j < n; j++)
		{
		    x1 = r*cs[j];
		    y1 = r*sn[j];
		    z = m[2][0]*x1 + m[2][1]*y1 + m[2][2]*z1;
		    if(z > 0.)
		    {
			x = m[0][0]*x1 + m[0][1]*y1 +m[0][2]*z1;
			y = m[1][0]*x1 + m[1][1]*y1 +m[1][2]*z1;
			if(!last_in) imove(d, x, y);
			else idraw(d, x, y);
			last_in = True;
		    }
		    else
		    {
			last_in = False;
		    }
		}
		iflush(d);
		if(d->s->nsegs > nsegs)
		{
		    mp->num_circle_segs[i] = d->s->nsegs - nsegs;
		    mp->circle_start[i] = nsegs;
		}
	    }
	}
}

static void
RedisplayCircles(MapPlotWidget w, Window window)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;

	if(!XtIsRealized((Widget)w) || mp->display_circles == MAP_OFF)
	{
		return;
	}

	if(mp->circles.nsegs > 0)
	{
		XDrawSegments(XtDisplay(w), window, ax->axesGC,
			(XSegment *)mp->circles.segs, mp->circles.nsegs);
	}
	for(i = 0; i < mp->ncircles; i++)
		if(mp->circle[i].selected &&
			mp->num_circle_segs[i] > 0)
	{
		XDrawSegments(XtDisplay(w), window, ax->mag_invertGC,
			(XSegment *)mp->circles.segs+mp->circle_start[i],
			mp->num_circle_segs[i]);
	}
}		

void
MapPlotClear(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nsta; i++) {
	    Free(mp->sta[i]->station.label);
	    Free(mp->sta[i]);
	}
	Free(mp->sta);
	mp->nsta = 0;

	for(i = 0; i < mp->nsrc; i++) {
	    Free(mp->src[i]->source.label);
	    Free(mp->src[i]);
	}
	Free(mp->src);
	mp->nsrc = 0;

	for(i = 0; i < mp->narc; i++) {
	    Free(mp->arc[i]->arc.label);
	    Free(mp->arc[i]->lon);
	    Free(mp->arc[i]->lat);
	    Free(mp->arc[i]->s.segs);
	    Free(mp->arc[i]);
	}
	Free(mp->arc);
	mp->narc = 0;

	for(i = 0; i < mp->nell; i++) {
	    Free(mp->ell[i]->ellipse.label);
	    Free(mp->ell[i]->lon);
	    Free(mp->ell[i]->lat);
	    Free(mp->ell[i]->s.segs);
	    Free(mp->ell[i]);
	}
	Free(mp->ell);
	mp->nell = 0;

	for(i = 0; i < mp->ndel; i++) {
	    Free(mp->del[i]->delta.label);
	    Free(mp->del[i]->lon);
	    Free(mp->del[i]->lat);
	    Free(mp->del[i]->s.segs);
	    Free(mp->del[i]);
	}
	Free(mp->del);
	mp->ndel = 0;

	for(i = 0; i < mp->nthemes; i++) {
	    int j;
	    for(j = 0; j < mp->theme[i]->theme.nshapes; j++) {
		ThemeDisplay *td = &mp->theme[i]->td[j];
		MapImage *mi = &mp->theme[i]->map_image;
		SHPDestroyObject(mp->theme[i]->theme.shapes[j]);
		Free(td->fill_segs.segs);
		Free(td->bndy_segs.segs);
		Free(td->p.points);
		Free(td->lon);
		Free(td->lat);
		Free(mi->label);
		if(!mi->copy) {
		    Free(mi->m.x);
		    Free(mi->m.y);
		    Free(mi->m.z);
		}
		Free(mi->s.segs);
		Free(mi->t.segs);
		Free(mi->l.ls);
	    }
	    Free(mp->theme[i]->theme.type_name);
	    Free(mp->theme[i]->theme.shapes);
	    Free(mp->theme[i]->theme.shape_fg);
	    Free(mp->theme[i]->theme.color_scale.pixels);
	    Free(mp->theme[i]->theme.color_scale.lines);
	    Free(mp->theme[i]);
	}
	Free(mp->theme);
	mp->nthemes = 0;

	for(i = 0; i < mp->nlines; i++) {
	    Free(mp->line[i]->line.label);
	    Free(mp->line[i]->line.lon);
	    Free(mp->line[i]->line.lat);
	    Free(mp->line[i]->s.segs);
	    Free(mp->line[i]);
	}
	Free(mp->line);
	mp->nlines = 0;

	for(i = 0; i < mp->npoly; i++) {
	    Free(mp->poly[i]->label);
	    Free(mp->poly[i]->lon);
	    Free(mp->poly[i]->lat);
	    Free(mp->poly[i]);
	}
	Free(mp->poly);
	mp->npoly = 0;

	for(i = 0; i < mp->nrect; i++) {
	    Free(mp->rect[i]->label);
	    Free(mp->rect[i]);
	}
	Free(mp->rect);
	mp->nrect = 0;

	for(i = 0; i < mp->nsymgrp; i++) {
	    Free(mp->symgrp[i]->group.label);
	    Free(mp->symgrp[i]->group.lon);
	    Free(mp->symgrp[i]->group.lat);
	    Free(mp->symgrp[i]->group.size);
	    Free(mp->symgrp[i]->p);
	    Free(mp->symgrp[i]);
	}
	Free(mp->symgrp);
	mp->nsymgrp = 0;

	Free(mp->border.segs);
	Free(mp->grid.segs);

	Free(mp->circle);
	Free(mp->circle_start);
	Free(mp->num_circle_segs);

	Free(mp->circles.segs);

	if( mp->utm_map ) {
	    MapPlotClear(mp->utm_map);
	}
}

static void
Destroy(Widget widget)
{
	MapPlotWidget w = (MapPlotWidget)widget;
	MapPlotPart *mp = &w->map_plot;
	
	MapPlotClear(w);

	if(mp->pixmap != (Pixmap)NULL) {
	    XFreePixmap(XtDisplay(w), mp->pixmap);
	}

	if(mp->symbolGC != NULL) XFreeGC(XtDisplay(w), mp->symbolGC);
	if(mp->lineGC != NULL) XFreeGC(XtDisplay(w), mp->lineGC);
	if(mp->dataGC != NULL) XFreeGC(XtDisplay(w), mp->dataGC);
	if(mp->contour_labelGC != NULL) {
	    XFreeGC(XtDisplay(w), mp->contour_labelGC);
	}
	if(mp->colorGC != NULL) XFreeGC(XtDisplay(w), mp->colorGC);
	if(mp->barGC != NULL) XFreeGC(XtDisplay(w), mp->barGC);
}

/** 
 * 
 */
void
_MapPlotRedraw(MapPlotWidget w)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	DrawStruct	*d;
	
	AxesWaitCursor((Widget)w, True);

	SetClipArea(&ax->d, ax->x1[ax->zoom], ax->y1[ax->zoom], 
			ax->x2[ax->zoom], ax->y2[ax->zoom]);
	d = &ax->d;

	for(i = 0; i < mp->nthemes; i++) {
	    DrawTheme(w, d, mp->theme[i]);
	}
	if(mp->projection != MAP_POLAR) {
	    DrawGrid(w, d, &ax->a, 2);
	}
	else {
	    DrawPolarGrid(w, d, 2);
	}
	DrawLines(w, d);
	DrawEllipses(w, d);
	DrawArcs(w, d);
	DrawDeltas(w, d);
	DrawBorder(w, d);

	if(mp->display_circles != MAP_OFF) {
	    DrawCircles(w);
	}
	mp->utm_letter = '\0';
	AxesWaitCursor((Widget)w, False);
}

static void
DrawMapBackground(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	SegArray s;
	DrawStruct *d;

	if(!XtIsRealized((Widget)w)) return;

	XSetClipMask(XtDisplay(w), mp->dataGC, None);
	XSetForeground(XtDisplay(w), mp->dataGC,
		mp->map_water_color);
	XFillRectangle(XtDisplay(w), mp->pixmap, mp->dataGC,
			0, 0, w->core.width, w->core.height);

	if(mp->projection == MAP_ORTHOGRAPHIC ||
	   mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
	   mp->projection == MAP_AZIMUTHAL_EQUAL_AREA ||
	   mp->projection == MAP_POLAR)
	{
	    s.size_segs = 0;
	    s.nsegs = 0;
	    s.segs = (DSegment *)NULL;
	    d = &ax->d;
	    d->s = &s;

	    FillBackground(w, d);

	    if(s.nsegs > 0)
	    {
		XSetForeground(XtDisplay(w), mp->dataGC,
			w->core.background_pixel);
		XDrawSegments(XtDisplay(w), mp->pixmap,
			mp->dataGC, (XSegment *)s.segs, s.nsegs);
	    }
	    Free(s.segs);
	}
	_MapPlotDrawBar(w);

	XSetClipRectangles(XtDisplay(w), mp->dataGC, 0,0, &ax->clip_rect, 1,
			Unsorted);
}

static void
FillBackground(MapPlotWidget w, DrawStruct *d)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double	xmin, xmax, x, y, lim;
	int i, i1, i2;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];

	if(mp->projection == MAP_ORTHOGRAPHIC)
	{
	    lim = 1.;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	{
	    lim = .95*pi;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    lim = .95*2.;
	}
	else if(mp->projection == MAP_POLAR)
	{
	    lim = mp->polar_max_radius;
	}
	else {
	    return;
	}

	if(d->iy1 < d->iy2) {
	    i1 = d->iy1;
	    i2 = d->iy2;
	}
	else {
	    i1 = d->iy2;
	    i2 = d->iy1;
	}

	for(i = i1; i <= i2; i++)
	{
	    y = scale_y(d, i);
	    if(y <= -lim || y >= lim)
	    {
		imove(d, xmin, y);
		idraw(d, xmax, y);
	    }
	    else
	    {
		x = -sqrt(lim*lim - y*y);
		if(x >= xmin)
		{
		    imove(d, xmin, y);
		    idraw(d, x, y);
		}
		x = -x;
		if(xmax >= x)
		{
		    imove(d, x, y);
		    idraw(d, xmax, y);
		}
	    }
	}
}

static void
DrawImageTheme(MapPlotWidget w, DrawStruct *d, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double x1, y1, x2, y2, xdif;
	MapImage *mi = &t->map_image;

	Free(mi->s.segs);
	Free(mi->l.ls);
	mi->s.size_segs = 0;
	mi->s.nsegs = 0;
	mi->l.size_ls = 0;
	mi->l.n_ls = 0;
	Free(mi->t.segs);
	mi->t.size_segs = 0;
	mi->t.nsegs = 0;

	if(mi->m.nx <= 0 || mi->m.ny <= 0 || mp->projection ==MAP_POLAR) return;

	unproject(w, ax->x1[ax->zoom], ax->y1[ax->zoom], &x1, &y1);
	unproject(w, ax->x2[ax->zoom], ax->y2[ax->zoom], &x2, &y2);

	t->delta_on = False;
/*
	if(t->theme.lat_min > y2 || t->theme.lat_max < y1) return;
	if(t->theme.lon_min > x2 && t->theme.lon_max-180. < x1) return;
	if(t->theme.lon_max < x1 && t->theme.lon_min+180. > x2) return;
*/

	xdif = fabs(x2 - x1);
	if(xdif > t->theme.on_delta || xdif < t->theme.off_delta) return;
	t->delta_on = True;

	MapPlotSizeImage(w);

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
	   mp->projection == MAP_UTM ||
	   mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	   mp->projection == MAP_MERCATOR)
	{
	    int i;
	    double *x = NULL, *y = NULL;
	    x1 = w->axes.x1[w->axes.zoom];
	    x2 = w->axes.x2[w->axes.zoom];
	    y1 = w->axes.y1[w->axes.zoom];
	    y2 = w->axes.y2[w->axes.zoom];

	    /* Check if the image wraps around the left or right limits.
	     * This will occur after a rotation, or simply because the
	     * input data has limits outside of -180 to 180, like 100. to 240.
	     * Also, make this check for latitude. Can have 1,2 or 4 parts.
	     */
	    mi->nparts = 0;
	    if(mi->m.x[0] < x1 && mi->m.x[0] + 360. < x2)
	    {
		DrawThemeImage(w, d, t, &mi->nparts);
		DoYCase(w, d, t, mi);

		if(!(x = (double *)AxesMalloc((Widget)w,
			mi->m.nx*sizeof(double))) ) return;
		memcpy(x, mi->m.x, mi->m.nx*sizeof(double));
		for(i = 0; i < mi->m.nx; i++) mi->m.x[i] = x[i] + 360.;

		DrawThemeImage(w, d, t, &mi->nparts);
		DoYCase(w, d, t, mi);
		memcpy(mi->m.x, x, mi->m.nx*sizeof(double));
		Free(x);
	    }
	    else if(mi->m.x[mi->m.nx-1] > x2 &&
			mi->m.x[mi->m.nx-1] - 360. > x1)
	    {
		DrawThemeImage(w, d, t, &mi->nparts);
		DoYCase(w, d, t, mi);

		if(!(x = (double *)AxesMalloc((Widget)w,
			mi->m.nx*sizeof(double))) ) return;
		memcpy(x, mi->m.x, mi->m.nx*sizeof(double));
		for(i = 0; i < mi->m.nx; i++) mi->m.x[i] = x[i] - 360.;

		DrawThemeImage(w, d, t, &mi->nparts);
		DoYCase(w, d, t, mi);
		memcpy(mi->m.x, x, mi->m.nx*sizeof(double));
		Free(x);
	    }
	    else if(mi->m.y[0] < y1 && mi->m.y[0] + 180. < y2)
	    {
		DrawThemeImage(w, d, t, &mi->nparts);

		if(!(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
		memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
		for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] + 180.;

		DrawThemeImage(w, d, t, &mi->nparts);
		memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
		Free(y);
	    }
	    else if(mi->m.y[mi->m.ny-1] > y2 && mi->m.y[mi->m.ny-1] - 180. > y1)
	    {
		DrawThemeImage(w, d, t, &mi->nparts);

		if(!(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
		memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
		for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] - 180.;

		DrawThemeImage(w, d, t, &mi->nparts);
		memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
		Free(y);
	    }
	    else {
		DrawThemeImage(w, d, t, &mi->nparts);
	    }
	}
	else {
	    shadeImageProjected(w, mp->image, d, w->core.background_pixel,
		t->theme.color_scale.num_colors, t->theme.color_scale.pixels,
		t->theme.color_scale.lines, &mi->m,
		&mi->display_x0[0], &mi->display_y0[0],
		&mi->display_width[0], &mi->display_height[0]);
	    mi->nparts = 1;
	}
}

static void
DoYCase(MapPlotWidget w, DrawStruct *d, MapTheme *t, MapImage *mi)
{
	int i;
	double *y = NULL;
	double y1 = w->axes.y1[w->axes.zoom];
	double y2 = w->axes.y2[w->axes.zoom];

	if(mi->m.y[0] < y1 && mi->m.y[0] + 180. < y2)
	{
	    if(!(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
	    memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
	    for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] + 180.;
	    DrawThemeImage(w, d, t, &mi->nparts);
	    memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
	    Free(y);
	}
	else if(mi->m.y[mi->m.ny-1] > y2 && mi->m.y[mi->m.ny-1] - 180. > y1)
	{
	    if(!(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
	    memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
	    for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] - 180.;
	    DrawThemeImage(w, d, t, &mi->nparts);
	    memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
	    Free(y);
	}
}

static void
DrawThemeImage(MapPlotWidget w, DrawStruct *d, MapTheme *t, int *part)
{
	MapPlotPart *mp = &w->map_plot;
	int i;
	LabelArray *l;
	MapImage *mi = &t->map_image;

	if(!XtIsRealized((Widget)w) || mi->m.nx <= 0 || mi->m.ny <= 0) return;
	
	if(mi->mode != CONTOURS_ONLY)
	{
	    mi->m.x1 = w->axes.x1[w->axes.zoom];
	    mi->m.x2 = w->axes.x2[w->axes.zoom];
	    mi->m.y1 = w->axes.y1[w->axes.zoom];
	    mi->m.y2 = w->axes.y2[w->axes.zoom];

	    if(mp->projection == MAP_LINEAR_CYLINDRICAL
		|| mp->projection == MAP_UTM)
	    {
		shadeImage(XtScreen(w), mp->image, d, w->core.background_pixel,
		    (Boolean *)NULL, (Boolean *)NULL,
		    t->theme.color_scale.num_colors,
		    t->theme.color_scale.pixels, t->theme.color_scale.lines,
		    &mi->m, &mi->display_x0[*part], &mi->display_y0[*part],
		    &mi->display_width[*part], &mi->display_height[*part],
		    mp->apply_anti_alias, False, 0);
		(*part)++;
	    }
	    else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
			mp->projection == MAP_MERCATOR)
	    {
		double *y=NULL, ym;

		if(!(y = (double *)AxesMalloc((Widget)w,
				mi->m.ny*sizeof(double))) ) return;
		memcpy(y, mi->m.y, mi->m.ny*sizeof(double));

		if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
		    for(i = 0; i < mi->m.ny; i++) {
			mi->m.y[i] = sin(y[i]*rad)/rad;
		    }
		}
		else {
		    for(i = 0; i < mi->m.ny; i++) {
			ym = y[i];
			if(ym > mp->mercator_max_lat) {
			    ym = mp->mercator_max_lat;
			}
			else if(ym < -mp->mercator_max_lat) {
			    ym = -mp->mercator_max_lat;
			}
			mi->m.y[i] = log(tan(.5*ym*rad + M_PI_4))/rad;
		    }
		}
		shadeImage(XtScreen(w), mp->image, d, w->core.background_pixel,
		    (Boolean *)NULL, (Boolean *)NULL,
		    t->theme.color_scale.num_colors,
		    t->theme.color_scale.pixels, t->theme.color_scale.lines,
		    &mi->m, &mi->display_x0[*part], &mi->display_y0[*part],
		    &mi->display_width[*part], &mi->display_height[*part],
		    mp->apply_anti_alias, False, 0);
		(*part)++;
		memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
		Free(y);
	    }
	}
	if(mi->mode != COLOR_ONLY)
	{
	    d->s = &mi->s;
	    d->l = &mi->l;
	    mi->m.x1 = w->axes.x1[w->axes.zoom];
	    mi->m.x2 = w->axes.x2[w->axes.zoom];
	    mi->m.y1 = w->axes.y1[w->axes.zoom];
	    mi->m.y2 = w->axes.y2[w->axes.zoom];
	    condata(d, (float)mp->contour_label_size, &mi->m,
			mi->auto_ci, &mi->ci, mi->c_min, mi->c_max);
	    d->s = &mi->t;
	    l = &mi->l;
	    for(i = 0; i < l->n_ls; i++)
	    {
		mapalf(d, l->ls[i].x, l->ls[i].y, l->ls[i].size,
			l->ls[i].angle, 0, l->ls[i].label);
	    }
	}
}

static void
MapPlotSizeImage(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;

	if(mp->image == NULL || mp->image_width != w->core.width ||
		mp->image_height != w->core.height)
	{
	    Display *display = XtDisplay(w);
	    int depth = DefaultDepth(display, DefaultScreen(display));
	    char *data;

	    if(mp->image != NULL) XDestroyImage(mp->image);

	    if( !(data = (char *)AxesMalloc((Widget)w,
			w->core.width*w->core.height*8)) ) return;

	    mp->image = XCreateImage(display,
			DefaultVisual(display, DefaultScreen(display)),
			depth, ZPixmap, 0, data, w->core.width, w->core.height,
			8, 0);
	    mp->image_width = w->core.width;
	    mp->image_height = w->core.height;
	}
}

#ifdef __OLD__
Boolean
MapPlotPrivateColors(MapPlotWidget w, int num_colors, int *r, int *g, int *b)
{
	MapPlotPart *mp = &w->map_plot;
/*	AxesPart *ax = &w->axes; */
	int i;

	if(num_colors <= 0)
	{
	    return(False);
	}
	if(XDisplayCells(XtDisplay(w),DefaultScreen(XtDisplay(w))) < num_colors)
	{
	    return(False);
	}
	mp->mode = COLOR_ONLY;

	if(num_colors > mp->num_cells)
	{
	    if(!GetColorCells(w, num_colors)) return False;
	}
	mp->num_private_colors = num_colors;

	if(mp->private_cells)
	{
	    for(i = 0; i < mp->num_private_colors; i++)
	    {
		mp->colors[i].red   = r[i];
		mp->colors[i].green = g[i];
		mp->colors[i].blue  = b[i];
		XStoreColor(XtDisplay(w), mp->cmap, &mp->private_colors[i]);
	    }
	}
	else
	{
	    for(i = 0; i < mp->num_private_colors; i++)
	    {
		mp->private_colors[i].red   = r[i];
		mp->private_colors[i].green = g[i];
		mp->private_colors[i].blue  = b[i];

		if(!XAllocColor(XtDisplay(w),mp->cmap,&mp->private_colors[i])) {
		    return False;
		}
		mp->private_pixels[i] = mp->private_colors[i].pixel;
	    }
	}
/*
	if(mp->image != NULL) {
	    _MapPlotRedisplayData(w);
	    _AxesRedisplayXor((AxesWidget)w);
	    _MapPlotDrawBar(w);
	}
*/
	return(True);
}

Boolean
MapPlotColors(MapPlotWidget w, int num_colors, int *r, int *g, int *b)
{
	MapPlotPart *mp = &w->map_plot;
	int i;
	Colormap cmap =
		DefaultColormap(XtDisplay(w), DefaultScreen(XtDisplay(w)));

	if(num_colors <= 0) return(False);

	if(XDisplayCells(XtDisplay(w),DefaultScreen(XtDisplay(w))) < num_colors)
	{
	    return(False);
	}

	Free(mp->pixels);
	Free(mp->colors);
	if( !(mp->pixels = (Pixel *)AxesMalloc((Widget)w,
		num_colors*sizeof(Pixel))) ) return False;
	if( !(mp->color = (XColor *)AxesMalloc((Widget)w,
		num_colors*sizeof(XColor)) )) return False;

	mp->num_colors = num_colors;

	for(i = 0; i < mp->num_colors; i++)
	{
	    mp->colors[i].red   = r[i];
	    mp->colors[i].green = g[i];
	    mp->colors[i].blue  = b[i];
	    mp->colors[i].flags = DoRed | DoGreen | DoBlue;

	    if(!XAllocColor(XtDisplay(w), cmap, &mp->colors[i])) {
		    return False;
	    }
	    mp->pixels[i] = mp->colors[i].pixel;
	}
/*
	if(mp->image != NULL) {
	    _MapPlotRedisplayData(w);
	    _AxesRedisplayXor((AxesWidget)w);
	    _MapPlotDrawBar(w);
	}
*/
	return(True);
}

Boolean
MapPlotSetColors(MapPlotWidget w, Pixel *pixels, int num_colors)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;

	if (num_colors < 1) return(False);

	if(mp->private_pixels == NULL) {
	    mp->private_pixels = (Pixel *)malloc(num_colors*sizeof(Pixel));
	}
	else {
	    mp->private_pixels = (Pixel *)realloc(mp->private_pixels,
				num_colors*sizeof(Pixel));
	}
	Free(mp->private_colors);
	if( !(mp->private_colors = (XColor *)AxesMalloc((Widget)w,
		num_colors*sizeof(XColor))) ) return(False);

	Free(mp->plane_masks);
	if( !(mp->plane_masks = (unsigned int *)AxesMalloc((Widget)w,
		num_colors*sizeof(unsigned int))) ) return(False);

	if(mp->cmap == (Colormap) NULL)
	{
	    if((mp->cmap = DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w)))) == (Colormap)NULL)
	    {
		return(False);
	    }
	}

	for(i = 0; i < num_colors; i++)
	{
	    mp->private_colors[i].pixel = mp->private_pixels[i] = pixels[i];
	    mp->private_colors[i].flags = DoRed | DoGreen | DoBlue;
	}
	mp->num_cells = num_colors;
	mp->num_private_colors = num_colors;
	mp->mode = COLOR_ONLY;
	
	return(True);
}

static Boolean 
GetColorCells(MapPlotWidget w, int num_colors)
{
	MapPlotPart *mp = &w->map_plot;
	int i, need;
	
	if(num_colors > XDisplayCells(XtDisplay(w),DefaultScreen(XtDisplay(w))))
	{
	    return False;
	}
	need = num_colors - mp->num_cells;
	if(need <= 0)
	{
	    return True;
	}
	if(mp->private_pixels == NULL) {
	    mp->private_pixels = (Pixel *)malloc(num_colors*sizeof(Pixel));
	}
	else {
	    mp->private_pixels = (Pixel *)realloc(mp->private_pixels,
			num_colors*sizeof(Pixel));
	}
	Free(mp->private_colors);
	if( !(mp->private_colors = (XColor *)AxesMalloc((Widget)w,
		num_colors*sizeof(XColor))) ) return False;

	Free(mp->plane_masks);
	if( !(mp->plane_masks = (unsigned int *)AxesMalloc((Widget)w,
		num_colors*sizeof(unsigned int))) ) return False;

	if(mp->cmap == (Colormap) NULL)
	{
	    if((mp->cmap = DefaultColormap(XtDisplay(w),
		DefaultScreen(XtDisplay(w)))) == (Colormap)NULL)
	    {
		return False;
	    }
	}

	if(XAllocColorCells(XtDisplay(w), mp->cmap, False, 
		(unsigned long *)mp->plane_masks, 0,
		mp->private_pixels+mp->num_cells, need) != 0)
	{
	    for(i = 0; i < need; i++)
	    {
		mp->private_colors[mp->num_cells+i].pixel
			= mp->private_pixels[i];
		mp->private_colors[mp->num_cells+i].flags
			= DoRed | DoGreen | DoBlue;
	    }
	    mp->num_cells = num_colors;
	    mp->private_cells = True;
	}
	else
	{
	    mp->private_cells = False;
	}
	return True;
}

void
MapPlotColorLines(MapPlotWidget w, int num_lines, double *lines)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	if(num_lines <= 0) {
	    return;
	}
	Free(mp->lines);
	if( !(mp->lines = (double *)AxesMalloc((Widget)w,
		num_lines*sizeof(double))) ) return;

	memcpy(mp->lines, lines, num_lines*sizeof(double));
	mp->num_lines = num_lines;
/*
	if(ComputeBar(w) || mp->num_stipples > 0)
	{
*/
ComputeBar(w);
	    _AxesRedraw((AxesWidget)w);
	    _MapPlotRedraw(w);
	    if(!w->axes.redisplay_pending)
	    {
		RedisplayPixmap(w, mp->pixmap, ax->pixmap);
		RedisplayOverlays(w, ax->pixmap);
		_AxesRedisplayAxes((AxesWidget)w);
		_MapPlotDrawBar(w);
		RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	    }
/*
	}
	else
	{
	    int xmin, xmax, ymin, ymax;
	    Drawable drawable = XtWindow(w);
	    entry = mp->entry[0];
	    if(!w->axes.redisplay_pending) {
		_AxesRedisplayXor((AxesWidget)w);
	    }
	    entry->m.x1 = w->axes.x1[w->axes.zoom];
	    entry->m.x2 = w->axes.x2[w->axes.zoom];
	    entry->m.y1 = w->axes.y1[w->axes.zoom];
	    entry->m.y2 = w->axes.y2[w->axes.zoom];
	    xmin = unscale_x(&w->axes.d, w->axes.x1[w->axes.zoom]) + 1;
	    xmax = unscale_x(&w->axes.d, w->axes.x2[w->axes.zoom]) - 1;
	    ymin = unscale_y(&w->axes.d, w->axes.y2[w->axes.zoom]) + 1;
	    ymax = unscale_y(&w->axes.d, w->axes.y1[w->axes.zoom]) - 1;
	    if(mp->image != NULL)
	    {
		shadeImage(mp->image, &w->axes.d, w->core.background_pixel,
			(Boolean *)NULL, (Boolean *)NULL,
			mp->num_colors, mp->pixels, mp->lines,
			&entry->m, xmin, xmax, ymin, ymax, mp->apply_anti_alias);
		if(!w->axes.redisplay_pending)
		{
		    XPutImage(XtDisplay(w), drawable, w->axes.axesGC,
			mp->image, 0, 0, xmin, ymin, mp->image->width,
			mp->image->height);
		}
	    }
	    else if(!w->axes.redisplay_pending)
	    {
		shade(XtDisplay(w), drawable, mp->colorGC,
			&w->axes.d, mp->num_colors, mp->pixels, mp->lines,
			mp->num_stipples, mp->stipples, &entry->m);
	    }
	    if(!w->axes.redisplay_pending) {
		RedisplayPixmap(w, mp->pixmap, ax->pixmap);
		RedisplayOverlays(w, ax->pixmap);
		_AxesRedisplayAxes((AxesWidget)w);
		_MapPlotDrawBar(w);
		RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	    }
	}
*/
}
#endif

static Boolean
ComputeBar(MapPlotWidget w, ColorScale *c)
{
	MapPlotPart *mp = &w->map_plot;
	int i, new_bar_width, max_width;
	int ascent, descent, direction;
	XCharStruct overall;
	char lab[20], last_lab[20];
	double range, dif;

	new_bar_width = 0;
	if(c->num_labels > 0)
	{
	    for(i = max_width = 0; i < c->num_labels; i++) {
		XTextExtents(w->axes.font, c->labels[i],
			(int)strlen(c->labels[i]), &direction,
			&ascent, &descent, &overall);
		if(max_width < overall.width) {
		    max_width = overall.width;
		}
	    }
	    new_bar_width = BAR_WIDTH + 7 + max_width;
	}
	else if(c->num_colors > 0)
	{
	    dif = c->lines[c->num_colors] - c->lines[0];

	    if(dif != 0.)
	    {
		range = log10(fabs(dif));
		mp->ndeci = (int)((range-2 > 0) ? 0 : -range+2);
		last_lab[0] = '\0';
		for(;;)
		{
		    for(i = max_width = 0; i <= c->num_colors; i++)
		    {
			ftoa(c->lines[i], mp->ndeci, 1, lab, 20);
			if(!strcmp(lab, last_lab)) {
			    mp->ndeci++;
			    last_lab[0] = '\0';
			    break;
			}
			stringcpy(last_lab, lab, sizeof(last_lab));
			XTextExtents(w->axes.font, lab, (int)strlen(lab),
				&direction, &ascent, &descent, &overall);
			if(max_width < overall.width) {
			    max_width = overall.width;
			}
		    }
		    if(i > c->num_colors || mp->ndeci >= 10) break;
		}
		new_bar_width = BAR_WIDTH + 7 + max_width;
	    }
	}

	if(new_bar_width > (int)w->core.width) {
	    new_bar_width = 0;
	    fprintf(stderr, "MapPlotWidget: ComputeBar failed.\n");
	}
	if(new_bar_width > mp->bar_width || mp->bar_width - new_bar_width > 10)
	{
	    mp->bar_width = new_bar_width;
	    w->axes.right_margin = mp->bar_width;
	    return(True);
	}
	else
	{
	    return(False);
	}
}

/** 
 * 
 */
void
_MapPlotDrawBar(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nthemes; i++) {
	    if(mp->theme[i]->color_bar_on) {
		MapPlotDrawThemeColorBar(w, &mp->theme[i]->theme.color_scale);
		return;
	    }
	}
	MapPlotDrawThemeColorBar(w, NULL);
}

static void
MapPlotDrawThemeColorBar(MapPlotWidget w, ColorScale *c)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i, x, y, idif, iy1, iy2, last_y, scroll_width;
	int ascent, descent, direction, margin;
	XCharStruct overall;
	char lab[20];
	double dely;
	Drawable drawable = ax->pixmap;

	if( !XtIsRealized((Widget)w) ) return;

	if(ax->vertical_scroll_position != LEFT && ax->vert_scroll &&
		XtIsManaged(ax->vert_scroll))
	{
	    scroll_width = ax->vert_scroll->core.width;
	}
	else {
	    scroll_width = 0;
	}
	if(mp->bar_width > 0) {
	    _AxesClearArea((AxesWidget)w,
			w->core.width - scroll_width - mp->bar_width,
			w->axes.d.iy2, mp->bar_width,
			w->axes.d.iy1 - w->axes.d.iy2 + 1);
	}

	if(!c || c->num_colors <= 0) return;

	XTextExtents(w->axes.font, "0123456789", 10, &direction, &ascent,
			&descent, &overall);
	margin = overall.descent + overall.ascent;
	if(c->num_colors <= 14) {
	    iy1 = unscale_y(&w->axes.d, w->axes.y1[w->axes.zoom]);
	    iy2 = unscale_y(&w->axes.d, w->axes.y2[w->axes.zoom]);
	    if(iy1 < iy2) {
		i = iy1;
		iy1 = iy2;
		iy2 = i;
	    }
	}
	else {
	    iy1 = w->axes.d.iy1 - margin;
	    iy2 = w->axes.d.iy2 + margin;
	}
	idif = iy1 - iy2 - 2*mp->bar_vertical_margin;
	if(idif <= 0) {
	    return;
	}

	dely = (double)idif/c->num_colors;

	XSetFillStyle(XtDisplay(w), mp->barGC, FillSolid);
	last_y = iy1 - mp->bar_vertical_margin;

	x = w->core.width - scroll_width - mp->bar_width;

	for(i = 0; i < c->num_colors; i++)
	{
	    XSetForeground(XtDisplay(w), mp->barGC, c->pixels[i]);
	    y = (int)(iy1 - mp->bar_vertical_margin - (i+1)*dely);
	    XFillRectangle(XtDisplay(w), drawable, mp->barGC, x, y, BAR_WIDTH,
				last_y - y);
	    last_y = y;
	}
	XSetForeground(XtDisplay(w), mp->barGC, w->axes.fg);

	x += BAR_WIDTH + 5;

	if(c->num_labels > 0)
	{
	    dely = (double)idif/(c->lines[c->num_colors] - c->lines[0]);
	    for(i = 0; i < c->num_labels; i++)
	    {
		y = (int)(iy1 - mp->bar_vertical_margin
			- (c->label_values[i] - c->lines[0])*dely);
		XTextExtents(w->axes.font, c->labels[i],
			(int)strlen(c->labels[i]), &direction,
				&ascent, &descent, &overall);
		XDrawString(XtDisplay(w), drawable, mp->barGC,
			x, y + overall.ascent/2, c->labels[i],
			(int)strlen(c->labels[i]));
	    }
	}
	else
	{
	    for(i = 0; i <= c->num_colors; i++)
	    {
		y = (int)(iy1 - mp->bar_vertical_margin - i*dely);
		ftoa(c->lines[i], mp->ndeci, 1, lab, 20);
		XTextExtents(w->axes.font, lab, (int)strlen(lab), &direction,
				&ascent, &descent, &overall);
		XDrawString(XtDisplay(w), drawable, mp->barGC,
				x, y + overall.ascent/2, lab, (int)strlen(lab));
	    }
	}
}

static void
DrawLines(MapPlotWidget w, DrawStruct *d)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;
	MapLine	*l;

	if(!XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nlines; i++)
		if(mp->line[i]->line.line.display >= MAP_ON ||
		   (mp->line[i]->line.line.display == MAP_SELECTED_ON
			&& mp->line[i]->line.line.selected))
	{
	    l = mp->line[i];
	    Free(l->s.segs);
	    l->s.size_segs = 0;
	    l->s.nsegs = 0;
	    if(mp->projection != MAP_POLAR || l->line.polar_coords) {
		d->s  = &l->s;
		DrawLonLat(w, d, l->line.npts, l->line.lon, l->line.lat);
	    }
	}
}

static void
DrawTheme(MapPlotWidget w, DrawStruct *d, MapTheme *t)
{
	if(t->theme_type == THEME_SHAPE) {
	    DrawShapeTheme(w, d, t);
	}
	else if(t->theme_type == THEME_IMAGE) {
	    DrawImageTheme(w, d, t);
	}
}

static void
DrawShapeTheme(MapPlotWidget w, DrawStruct *d, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i;
	double xmin, xmax, ymin, ymax, x1, x2, y1, y2, xdif;
	
	if(!XtIsRealized((Widget)w)) return;

	if(mp->projection == MAP_POLAR && !t->theme.polar_coords) return;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];

	if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
	    ymin = sin(ymin*rad)/rad;
	    ymax = sin(ymin*rad)/rad;
	}
	else if(mp->projection == MAP_MERCATOR) {
	    ymin = log(tan(.5*ymin*rad + M_PI_4))/rad;
	    ymax = log(tan(.5*ymax*rad + M_PI_4))/rad;
	}

	for(i = 0; i < t->theme.nshapes; i++) {
	    ThemeDisplay *td = &t->td[i];
	    Free(td->fill_segs.segs);
	    td->fill_segs.size_segs = 0;
	    td->fill_segs.nsegs = 0;
	    Free(td->bndy_segs.segs);
	    td->bndy_segs.size_segs = 0;
	    td->bndy_segs.nsegs = 0;
	    Free(td->p.points);
	    td->p.size_points = 0;
	    td->p.npoints = 0;
	}

	if(mp->projection != MAP_POLAR)
	{
	    unproject(w, xmin, ymin, &x1, &y1);
	    unproject(w, xmax, ymax, &x2, &y2);

	    t->delta_on = False;
	    if(t->theme.lat_min > y2 || t->theme.lat_max < y1) return;
	    if(t->theme.lon_min > x2 && t->theme.lon_max-180. < x1) return;
	    if(t->theme.lon_max < x1 && t->theme.lon_min+180. > x2) return;

	    xdif = fabs(x2 - x1);
	    if(xdif > t->theme.on_delta || xdif < t->theme.off_delta) return;
	    t->delta_on = True;
	}

	if(t->theme.shape_type == SHPT_POLYGON) {
	    if(mp->projection == MAP_LINEAR_CYLINDRICAL
		|| mp->projection == MAP_UTM
		|| mp->projection == MAP_POLAR)
	    {
		DrawThemeLCFill(w, d, t, -1);	/* draw all shapes */
	    }
	    else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
			mp->projection == MAP_MERCATOR ||
			mp->projection == MAP_UTM_NEAR)
	    {
		DrawThemeMercatorFill(w, d, t, -1);
	    }
	    else {
		DrawThemeOrthoFill(w, d, t, -1);
	    }
	}
	else if(t->theme.shape_type == SHPT_ARC) {
	    DrawThemePts(w, d, t, -1);
	}
	if(mp->projection != MAP_POLAR && t->theme.polar_coords) {
	    DrawThemeGrid(w, d, t);
	}
}

static void
DrawThemeShape(MapPlotWidget w, DrawStruct *d, MapTheme *t, int i)
{
	MapPlotPart *mp = &w->map_plot;
	ThemeDisplay *td = &t->td[i];

	Free(td->fill_segs.segs);
	td->fill_segs.size_segs = 0;
	td->fill_segs.nsegs = 0;
	Free(td->bndy_segs.segs);
	td->bndy_segs.size_segs = 0;
	td->bndy_segs.nsegs = 0;
	Free(td->p.points);
	td->p.size_points = 0;
	td->p.npoints = 0;

	if(!t->display) return;

	if(t->theme.shape_type == SHPT_POLYGON)
	{
	    if(mp->projection == MAP_LINEAR_CYLINDRICAL
		|| mp->projection == MAP_UTM
		|| mp->projection == MAP_POLAR)
	    {
		DrawThemeLCFill(w, d, t, i);
	    }
	    else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		mp->projection == MAP_MERCATOR)
	    {
		DrawThemeMercatorFill(w, d, t, i);
	    }
	    else {
		DrawThemeOrthoFill(w, d, t, i);
	    }
	}
	else if(t->theme.shape_type == SHPT_ARC) {
	    DrawThemePts(w, d, t, i);
	}
}

static void
RedisplayThemes(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	DrawMapBackground(w);
	for(i = 0; i < mp->nthemes; i++) {;
	    if(mp->projection != MAP_POLAR || mp->theme[i]->theme.polar_coords)
	    {
		RedisplayTheme(w, mp->pixmap, mp->theme[i]);
	    }
	}
}

static void
RedisplayTheme(MapPlotWidget w, Window window, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;

	if(mp->projection != MAP_POLAR || t->theme.polar_coords) {
	    if(t->theme_type == THEME_SHAPE) {
		RedisplayShapeTheme(w, window, t);
	    }
	    else if(t->theme_type == THEME_IMAGE) {
		RedisplayImageTheme(w, window, t);
	    }
	}
}

static void
RedisplayShapeTheme(MapPlotWidget w, Window window, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i;
	double xmin, xmax, ymin, ymax, x1, x2, y, xdif, on_delta;
	GC gc;

	if(!XtIsRealized((Widget)w)) return;

	if(!t->display) return;

	if(mp->projection != MAP_POLAR)
	{
	    xmin = ax->x1[ax->zoom];
	    xmax = ax->x2[ax->zoom];
	    ymin = ax->y1[ax->zoom];
	    ymax = ax->y2[ax->zoom];

	    unproject(w, xmin, ymin, &x1, &y);
	    unproject(w, xmax, ymax, &x2, &y);
	    xdif = fabs(x2 - x1);
	    t->delta_on = False;
	    on_delta = (t->theme.on_delta > 359.) ? 1.e+10 : t->theme.on_delta;
	    if(xdif > on_delta || xdif < t->theme.off_delta) return;
	    t->delta_on = True;
	}

/*	gc = line->xorr ? ax->mag_invertGC : w->map_plot.lineGC; */
	gc = w->map_plot.lineGC;

	XSetClipRectangles(XtDisplay(w), gc, 0,0, &ax->clip_rect, 1, Unsorted);

	if(t->theme.shape_type == SHPT_POLYGON) {
	    Pixel black = BlackPixelOfScreen(XtScreen(w));
	    Pixel white = WhitePixelOfScreen(XtScreen(w));
	    if(t->theme.fill)
	    {
		for(i = 0; i < t->theme.nshapes; i++) if(t->td[i].display)
		{
		    if(!t->td[i].selected) {
			XSetForeground(XtDisplay(w), gc, t->theme.shape_fg[i]);
		    }
		    else {
			if(ax->select_fg != t->theme.shape_fg[i]) {
			    XSetForeground(XtDisplay(w), gc, ax->select_fg);
			}
			else if(black != t->theme.shape_fg[i]) {
			    XSetForeground(XtDisplay(w), gc, black);
			}
			else {
			    XSetForeground(XtDisplay(w), gc, white);
			}
		    }
		    XDrawSegments(XtDisplay(w), window, gc,
				(XSegment *)t->td[i].fill_segs.segs,
				t->td[i].fill_segs.nsegs);
		}
	    }
	    if(t->theme.bndry)
	    {
		XSetForeground(XtDisplay(w), gc, t->theme.bndry_fg);
		for(i = 0; i < t->theme.nshapes; i++) {
		    XDrawSegments(XtDisplay(w), window, gc,
				(XSegment *)t->td[i].bndy_segs.segs,
				t->td[i].bndy_segs.nsegs);
		}
	    }
	    if(t->display_labels)
	    {
		XSetForeground(XtDisplay(w), mp->dataGC, ax->fg);
		for(i = 0; i < t->theme.nshapes; i++) if(t->td[i].display)
		{
		    DisplayThemePolygonLabel(w, window, mp->dataGC, &t->td[i]);
		}
	    }
	}
	else if(t->theme.shape_type == SHPT_POINT) {
	    DisplayThemeSymbols(w, window, t, -1);
	}
	else if(t->theme.shape_type == SHPT_ARC)
	{
	    if(!t->display_labels)
	    {
		for(i = 0; i < t->theme.nshapes; i++) if(t->td[i].display)
		{
		    if(!t->td[i].selected) {
			XSetForeground(XtDisplay(w), gc, t->theme.shape_fg[i]);
		    }
		    else {
			XSetForeground(XtDisplay(w), gc, ax->select_fg);
		    }
		    XDrawSegments(XtDisplay(w), window, gc,
			(XSegment *)t->td[i].bndy_segs.segs,
			t->td[i].bndy_segs.nsegs);
		}
	    }
	    else
	    {
		for(i = 0; i < t->theme.nshapes; i++) if(t->td[i].display) {
		    if(!t->td[i].selected) {
			XSetForeground(XtDisplay(w), gc, t->theme.shape_fg[i]);
		    }
		    else {
			XSetForeground(XtDisplay(w), gc, ax->select_fg);
		    }
		    DisplayArcShapeLabel(w, window, gc, t, i);
		}
	    }
	}
}

int
MapPlotThemeOnOffDelta(MapPlotWidget w, int theme_id, double on_delta,
		double off_delta, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i < mp->nthemes) {
	    t = mp->theme[i];
	    t->theme.on_delta = on_delta;
	    t->theme.off_delta = off_delta;
	    DrawTheme(w, &ax->d, t);
	    if(redisplay) {
		RedisplayTheme(w, XtWindow(w), t);
	    }
	    if( mp->utm_map ) {
		MapPlotThemeOnOffDelta(mp->utm_map, theme_id, on_delta,
					off_delta, redisplay);
	    }
	    return 0;
	}
	return -1;
}

int
MapPlotThemeBndryColor(MapPlotWidget w, int theme_id, const char *color_name,
		Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i < mp->nthemes) {
	    t = mp->theme[i];
	    t->theme.bndry_fg = StringToPixel((Widget)w, (char *)color_name);
	    if(redisplay) {
		MapPlotUpdate(w);
	    }
	    if( mp->utm_map ) {
		MapPlotThemeBndryColor(mp->utm_map, theme_id, color_name,
				redisplay);
	    }
	    return 0;
	}
	return -1;
}

int
MapPlotThemeFillColor(MapPlotWidget w, int theme_id, const char *color_name,
		Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i < mp->nthemes) {
	    MapTheme *t = mp->theme[i];
	    Pixel fg = StringToPixel((Widget)w, (char *)color_name);
	    for(i = 0; i < t->theme.nshapes; i++) {
		t->theme.shape_fg[i] = fg;
	    }
	    if(redisplay) {
		MapPlotUpdate(w);
	    }
	    if( mp->utm_map ) {
		MapPlotThemeFillColor(mp->utm_map, theme_id, color_name,
				redisplay);
	    }
	    return 0;
	}
	return -1;
}

int
MapPlotThemeSymbolType(MapPlotWidget w, int theme_id, int symbol_type)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i < mp->nthemes)
	{
	    t = mp->theme[i];
	    t->theme.symbol_type = symbol_type;
	    MapPlotUpdate(w);
	    if( mp->utm_map ) {
		MapPlotThemeSymbolType(mp->utm_map, theme_id, symbol_type);
	    }
	    return 1;
	}
	return 0;
}

int
MapPlotThemeSymbolSize(MapPlotWidget w, int theme_id, int symbol_size)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i < mp->nthemes)
	{
	    t = mp->theme[i];
	    t->theme.symbol_size = symbol_size;
	    MapPlotUpdate(w);
	    if( mp->utm_map ) {
		MapPlotThemeSymbolSize(mp->utm_map, theme_id, symbol_size);
	    }
	    return 1;
	}
	return 0;
}

static void
DisplayArcShapeLabel(MapPlotWidget w, Window window, GC gc, MapTheme *t, int i)
{
	MapPlotPart *mp = &w->map_plot;

	if(t->td[i].label != NULL && t->td[i].bndy_segs.nsegs > 0)
	{
	    int j, j1, n, ascent, descent, direction, nlines;
	    int ix1, iy1, ix2, iy2, x, y;
	    XCharStruct overall;
	    char *lab = t->td[i].label;

	    ix1 = t->td[i].bndy_segs.segs[0].x1;
	    iy1 = t->td[i].bndy_segs.segs[0].y1;
	    ix2 = t->td[i].bndy_segs.segs[t->td[i].bndy_segs.nsegs-1].x2;
	    iy2 = t->td[i].bndy_segs.segs[t->td[i].bndy_segs.nsegs-1].y2;

	    nlines = 1;
	    n = (int)strlen(lab);
	    for(j = 1; j < n; j++) if(lab[j] == '\n') nlines++;

	    j1 = 0;
	    for(j = 1; j <= n; j++)
	    {
		if(lab[j] == '\n' || lab[j] == '\0')
		{
		    XTextExtents(mp->label_font, lab+j1, j-j1,
				&direction, &ascent, &descent, &overall);

		    /* do not draw if the label is longer than the arc */
/*		    if(!j1 && overall.width > abs(ix2-ix1)) return; */

		    x = (ix1 + ix2)/2 - overall.width/2;
		    y = (iy1 + iy2)/2 - (nlines-1)*(ascent+descent);
		    XDrawString(XtDisplay(w), window, gc, x, y, lab+j1, j-j1);
		    j1 = j+1;
		    nlines--;
		}
	    }
	}
}

static void
DrawEllipses(MapPlotWidget w, DrawStruct *d)
{
	MapPlotPart *mp = &w->map_plot;
	int		i;
	MapEllipse	*e;

	if(!XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nell; i++)
	    if(mp->ell[i]->ellipse.line.display >= MAP_ON ||
		(mp->ell[i]->ellipse.line.display == MAP_SELECTED_ON
		&& mp->ell[i]->ellipse.line.selected))
	{
	    e = mp->ell[i];
	    Free(e->s.segs);
	    e->s.size_segs = 0;
	    e->s.nsegs = 0;
	    if(mp->projection != MAP_POLAR) {
		d->s  = &e->s;
		DrawLonLat(w, d, e->npts, e->lon, e->lat);
	    }
	}
}

static void
DrawArcs(MapPlotWidget w, DrawStruct *d)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;
	MapArc	*a;

	if(!XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->narc; i++)
	    if(mp->arc[i]->arc.line.display >= MAP_ON ||
		(mp->arc[i]->arc.line.display == MAP_SELECTED_ON
		&& mp->arc[i]->arc.line.selected))
	{
	    a = mp->arc[i];
	    Free(a->s.segs);
	    a->s.size_segs = 0;
	    a->s.nsegs = 0;
	    if(mp->projection != MAP_POLAR) {
		d->s  = &a->s;
		DrawLonLat(w, d, a->npts, a->lon, a->lat);
	    }
	}
}

static void
DrawDeltas(MapPlotWidget w, DrawStruct *d)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;

	if(!XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->ndel; i++)
	    if(mp->del[i]->delta.line.display >= MAP_ON ||
		(mp->del[i]->delta.line.display == MAP_SELECTED_ON
		&& mp->del[i]->delta.line.selected))
	{
	    DrawDelta(w, d, mp->del[i]);
	}
}

static void
DrawDelta(MapPlotWidget w, DrawStruct *d, MapDelta *del)
{
	MapPlotPart *mp = &w->map_plot;

	Free(del->s.segs);
	del->s.size_segs = 0;
	del->s.nsegs = 0;
	d->s  = &del->s;
	if(mp->projection == MAP_UTM_NEAR) {
	    int i;
	    if(del->npts > 0) imove(d, del->lon[0], del->lat[0]);
	    for(i = 1; i < del->npts; i++) {
		idraw(d, del->lon[i], del->lat[i]);
	    }
	    iflush(d);
	}
	else if(mp->projection != MAP_POLAR) {
	    DrawLonLat(w, d, del->npts, del->lon, del->lat);
	}
}

static void
DrawThemeLCFill(MapPlotWidget w, DrawStruct *d, MapTheme *t, int ishape)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j, l, i0, npts, nshapes, q0;
	int	i1, i2, ilat, q, numlat;
	int	*sizes = NULL, *nlon = NULL;
	Longitude **lon = NULL;
	double	xmin, xmax, ymin, ymax, x2, x, y, dlat;
	double	*lat = NULL;

	if(!XtIsRealized((Widget)w) || !t->display) return;

/*3-18	if(t->theme.shape_type != SHPT_POLYGON) return;*/

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];

	i1 = unscale_y(d, ymax) + 1;
	i2 = unscale_y(d, ymin) - 1;
	if(i1 > i2) {
	    i = i1;
	    i1 = i2;
	    i2 = i;
	}
	numlat = i2 - i1 + 1;
	if(numlat <= 1) return;
	lon = (Longitude **)malloc(numlat*sizeof(Longitude *));
	sizes = (int *)malloc(numlat*sizeof(int));
	lat = (double *)malloc(numlat*sizeof(double));
	nlon = (int *)malloc(numlat*sizeof(int));
	for(i = 0; i < numlat; i++)
	{
	    lon[i] = (Longitude *)malloc(sizeof(Longitude));
	    sizes[i] = 1;
	    nlon[i] = 0;
	}
	for(ilat = i1, l = 0; ilat <= i2; ilat++, l++)
	{
/*	    lat[l] = scale_y(d, ilat); */
	    lat[l] = scale_y(d, ilat) + .0001;
	}

	dlat = d->scaley;

	if(ishape >= 0) {
	    q0 = ishape;
	    nshapes = ishape+1;
	}
	else {
	    q0 = 0;
	    nshapes = t->theme.nshapes;
	}

	for(q = q0; q < nshapes; q++) if(t->td[q].display)
	{
	    SHPObject *s = t->theme.shapes[q];
	    ThemeDisplay *td = &t->td[q];

	    if(s->nSHPType != SHPT_POLYGON) continue;
	    if(!t->theme.polar_coords || mp->projection == MAP_POLAR) {
		if(s->dfYMin > ymax || s->dfYMax < ymin) continue;
		if(s->dfXMin > xmax && s->dfXMax-180. < xmin) continue;
		if(s->dfXMax < xmin && s->dfXMin+180. > xmax) continue;
	    }

	    d->s = &td->bndy_segs;

	    for(j = 0; j < s->nParts; j++)
	    {
		i0 = s->panPartStart[j];
		if(j < s->nParts-1) {
		    npts = s->panPartStart[j+1] - i0;
		}
		else {
		    npts = s->nVertices - i0;
		}
		if(!t->theme.polar_coords) {
		    DrawLonLat(w, d, npts, s->padfX+i0, s->padfY+i0);
		}
		else {
		    DrawPolarLonLat(w, d, &t->theme, npts, s->padfX+i0,
				s->padfY+i0);
		}
	    }
	    iflush(d);

	    d->s = &td->fill_segs;

	    if(mp->projection == MAP_POLAR) {
		MapGetPolar(ax, s, numlat, lat, dlat, lon, nlon, sizes);
	    }
	    else if(!t->theme.polar_coords) {
		MapGetLCLongitudes(ax, s, numlat, lat, dlat, lon, nlon, sizes);
	    }
	    else {
		MapGetLCLongPolar(ax, &t->theme, s, numlat, lat, dlat, lon,
					nlon, sizes);
	    }

            /* check for counter-clockwise outer loop.
	     */
	    for(l = 0; l < numlat && (nlon[l] == 0 || lon[l][0].up); l++);
	    if(l < numlat) {
		/* redo it in the opposite sense */
		MapReverseLoop(s, lon[l][0].loop);
		if(mp->projection == MAP_POLAR) {
		    MapGetPolar(ax, s, numlat, lat, dlat, lon, nlon, sizes);
		}
		else if(!t->theme.polar_coords) {
		    MapGetLCLongitudes(ax, s, numlat, lat, dlat,lon,nlon,sizes);
		}
		else {
		    MapGetLCLongPolar(ax, &t->theme, s, numlat, lat, dlat, lon,
					nlon, sizes);
		}
		for(l = 0; l < numlat && (nlon[l] == 0 || lon[l][0].up); l++);
		if(l < numlat) continue; /* failed twice */
	    }
	    if(mp->phi != 0.)
	    {
		/* correct for rotation
		 */
		for(l = 0; l < numlat; l++)
		{
		    for(j = 0; j < nlon[l]; j++) {
			if(lon[l][j].lon < ax->x1[0]) lon[l][j].lon += 360;
			else if(lon[l][j].lon > ax->x2[0]) lon[l][j].lon -= 360;
		    }
		    qsort(lon[l], (unsigned)nlon[l],sizeof(Longitude),sort_lon);
		}
	    }

	    for(l = 0; l < numlat; l++) if(nlon[l] != 0)
	    {
/*
for(j = 1; j < nlon[l]; j++) {
    if(lon[l][j].up == lon[l][j-1].up) {
	if(!lon[l][0].up) {
	   printf("found one at lat=%.4f lon=%.4f lon=%.4f and bad direction\n",
		lat[l], lon[l][j-1].lon, lon[l][j].lon);
	}
	else {
	   printf("found one at lat=%.4f lon=%.4f lon=%.4f\n",
		lat[l], lon[l][j-1].lon, lon[l][j].lon);
	}
    }
}
*/
		y = lat[l];
		if(!lon[l][0].up)
		{
		    x = lon[l][0].lon;
		    if(x > xmin) {
			imove(d, xmin, y);
			idraw(d, x, y);
		    }
		}

		for(j = 0; j < nlon[l]-1; j++)
		{
		    if(lon[l][j].up)
		    {
			x = lon[l][j].lon;
			imove(d, x, y);
			x2 = lon[l][j+1].lon;
			idraw(d, x2, y);
		    }
		}
		if(lon[l][nlon[l]-1].up)
		{
		    x = lon[l][nlon[l]-1].lon;
		    if(xmax > x) {
			imove(d, x, y);
			idraw(d, xmax, y);
		    }
		}
	    }
	    iflush(d);
	}

	Free(nlon);
	Free(sizes);
	Free(lat);
	for(i = 0; i < numlat; i++) {
	    Free(lon[i]);
	}
	Free(lon);
}

static void
MapGetLCLongitudes(AxesPart *ax, SHPObject *s, int numlat, double *lat,
	double dlat, Longitude **lon, int *nlon, int *sizes)
{
	int i, i0, j, l, l1, l2, m, num, npts;
	double x1=0., y1=0., x2, y2, fac, mov = .00001;
	Boolean add_end_pt=False;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts) {
		x1 = s->padfX[i0];
		y1 = s->padfY[i0];
		add_end_pt = False;
		if(x1 != s->padfX[i0+npts-1] || y1 != s->padfY[i0+npts-1])
		{
		    npts++;
		    add_end_pt = True;
		}
	    }
	    if(npts <= 3) continue;

	    for(i = 1; i < npts; i++)
	    {
		if(i < npts-1 || !add_end_pt) {
		    x2 = s->padfX[i0+i];
		    y2 = s->padfY[i0+i];
		}
		else {
		    x2 = s->padfX[i0];
		    y2 = s->padfY[i0];
		}
		if(x2 == x1 && y2 == y1) continue;

		if(y1 == y2)
		{
		    if(y1 < -80.) {
			/* special case to handle the line in the Antarctica
			 * boundary that goes to the south pole and back.
			 */
			y2 += .0001;
		    }
		    else {
			x1 = x2;
			continue;
		    }
		}

		l1 = (int)((y1 - lat[0])/dlat + .5);
		l2 = (int)((y2 - lat[0])/dlat + .5);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;

		fac = (x2-x1)/(y2-y1);
		for(l = l1; l <= l2; l++)
		{
		    if(y1 == lat[l]) {
			y1 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y2 == lat[l]) {
			y2 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y1 < lat[l] && y2 > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					(m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = True;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		    else if(y1 > lat[l] && y2 < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = False;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		}
		x1 = x2;
		y1 = y2;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static void
MapGetLCLongPolar(AxesPart *ax, MapPlotTheme *theme, SHPObject *s, int numlat,
	double *lat, double dlat, Longitude **lon, int *nlon, int *sizes)
{
	int i, i0, j, l, l1, l2, m, num, npts;
	double x1=0., y1=0., x2, y2, fac, mov = .00001;
	double alpha, beta, gamma, c[3][3];
	double x, y, z, th, phi, sn, xp, yp, zp, r, theta;
	Boolean add_end_pt=False;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	/*
	 * alpha, beta, gamma from north pole to polar origin
	 */
	alpha = theme->center_lon*rad;
	beta = (90. - theme->center_lat)*rad;
	gamma = 0.;
	rotation_matrix(alpha, beta, gamma, c);

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts) {
		r = s->padfX[i0];
		theta = s->padfY[i0];
		phi = (180. - theta)*DEG_TO_RADIANS;
		th = r/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		x1 = atan2(yp, xp)/DEG_TO_RADIANS;
		y1 = 90. - th/DEG_TO_RADIANS;

		add_end_pt = False;
		if(s->padfX[i0] != s->padfX[i0+npts-1]
			|| s->padfY[i0] != s->padfY[i0+npts-1])
		{
		    npts++;
		    add_end_pt = True;
		}
	    }
	    if(npts <= 3) continue;

	    for(i = 1; i < npts; i++)
	    {
		if(i < npts-1 || !add_end_pt) {
		    r = s->padfX[i0+i];
		    theta = s->padfY[i0+i];
		}
		else {
		    r = s->padfX[i0];
		    theta = s->padfY[i0];
		}
		phi = (180. - theta)*DEG_TO_RADIANS;
		th = r/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		x2 = atan2(yp, xp)/DEG_TO_RADIANS;
		y2 = 90. - th/DEG_TO_RADIANS;

		if(x2 == x1 && y2 == y1) continue;

		if(y1 == y2)
		{
		    if(y1 < -80.) {
			/* special case to handle the line in the Antarctica
			 * boundary that goes to the south pole and back.
			 */
			y2 += .0001;
		    }
		    else {
			x1 = x2;
			continue;
		    }
		}

		l1 = (int)((y1 - lat[0])/dlat + .5);
		l2 = (int)((y2 - lat[0])/dlat + .5);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;

		fac = (x2-x1)/(y2-y1);
		for(l = l1; l <= l2; l++)
		{
		    if(y1 == lat[l]) {
			y1 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y2 == lat[l]) {
			y2 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y1 < lat[l] && y2 > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					(m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = True;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		    else if(y1 > lat[l] && y2 < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = False;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		}
		x1 = x2;
		y1 = y2;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static void
MapGetPolar(AxesPart *ax, SHPObject *s, int numlat, double *lat, double dlat,
		Longitude **lon, int *nlon, int *sizes)
{
	int i, i0, j, l, l1, l2, m, num, npts;
	double x1=0., y1=0., x2, y2, fac, mov = .00001;
	double r, theta;
	Boolean add_end_pt=False;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts) {
		r = s->padfX[i0];
		theta = s->padfY[i0]*DEG_TO_RADIANS;
		x1 = r*sin(theta);
		y1 = r*cos(theta);

		add_end_pt = False;
		if(s->padfX[i0] != s->padfX[i0+npts-1]
			|| s->padfY[i0] != s->padfY[i0+npts-1])
		{
		    npts++;
		    add_end_pt = True;
		}
	    }
	    if(npts <= 3) continue;

	    for(i = 1; i < npts; i++)
	    {
		if(i < npts-1 || !add_end_pt) {
		    r = s->padfX[i0+i];
		    theta = s->padfY[i0+i]*DEG_TO_RADIANS;
		}
		else {
		    r = s->padfX[i0];
		    theta = s->padfY[i0]*DEG_TO_RADIANS;
		}
		x2 = r*sin(theta);
		y2 = r*cos(theta);

		if(x2 == x1 && y2 == y1) continue;

		if(y1 == y2)
		{
		    if(y1 < -80.) {
			/* special case to handle the line in the Antarctica
			 * boundary that goes to the south pole and back.
			 */
			y2 += .0001;
		    }
		    else {
			x1 = x2;
			continue;
		    }
		}

		l1 = (int)((y1 - lat[0])/dlat + .5);
		l2 = (int)((y2 - lat[0])/dlat + .5);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;

		fac = (x2-x1)/(y2-y1);
		for(l = l1; l <= l2; l++)
		{
		    if(y1 == lat[l]) {
			y1 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y2 == lat[l]) {
			y2 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y1 < lat[l] && y2 > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					(m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = True;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		    else if(y1 > lat[l] && y2 < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = False;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		}
		x1 = x2;
		y1 = y2;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static void
MapReverseLoop(SHPObject *s, int loop)
{
	int i0, i, npts;
	double d;

	if(loop < 0 || loop >= s->nParts) return;

	i0 = s->panPartStart[loop];
	if(loop < s->nParts-1) {
	    npts = s->panPartStart[loop+1] - i0;
	}
	else {
	    npts = s->nVertices - i0;
	}

	for(i = 0; i < npts; i++)
	{
	    d = s->padfX[i0+i];
	    s->padfX[i0+i] = s->padfX[i0+npts-1-i];
	    s->padfX[i0+npts-1-i] = d;
	    d = s->padfY[i0+i];
	    s->padfY[i0+i] = s->padfY[i0+npts-1-i];
	    s->padfY[i0+npts-1-i] = d;
	}
}

static void
DrawThemeMercatorFill(MapPlotWidget w, DrawStruct *d, MapTheme *t, int ishape)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j, l, i0, npts, nshapes, q0;
	int	i1, i2, ilat, q, numlat;
	int	*sizes = NULL, *nlon = NULL;
	Longitude **lon = NULL;
	double	xmin, xmax, ymin, ymax, x2, x, y, dlat;
	double x_min, x_max, y_min, y_max;
	double	*lat = NULL;
	Boolean up;

	if(!XtIsRealized((Widget)w) || !t->display) return;

/*3-18	if(t->theme.shape_type != SHPT_POLYGON) return; */

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];

/*
	unproject(w, xmin, ymin, &x_min, &y_min);
	unproject(w, xmax, ymax, &x_max, &y_max);
*/
	unproject(w, xmin, ymin, &x_min, &y_min);
	unproject(w, xmax, ymin, &x_max, &y);
	unproject(w, xmax, ymax, &x, &y_max);

	i1 = unscale_y(d, ymax) + 1;
	i2 = unscale_y(d, ymin) - 1;
	if(i1 > i2) {
	    i = i1;
	    i1 = i2;
	    i2 = i;
	}
	numlat = i2 - i1 + 1;
	if(numlat <= 1) return;
	lon = (Longitude **)malloc(numlat*sizeof(Longitude *));
	sizes = (int *)malloc(numlat*sizeof(int));
	lat = (double *)malloc(numlat*sizeof(double));
	nlon = (int *)malloc(numlat*sizeof(int));
	for(i = 0; i < numlat; i++)
	{
	    lon[i] = (Longitude *)malloc(sizeof(Longitude));
	    sizes[i] = 1;
	    nlon[i] = 0;
	}
	for(ilat = i1, l = 0; ilat <= i2; ilat++, l++)
	{
	    lat[l] = scale_y(d, ilat);
	}
	dlat = d->scaley;

	if(ishape >= 0) {
	    q0 = ishape;
	    nshapes = ishape+1;
	}
	else {
	    q0 = 0;
	    nshapes = t->theme.nshapes;
	}

	for(q = q0; q < nshapes; q++) if(t->td[q].display)
	{
	    SHPObject *s = t->theme.shapes[q];
	    ThemeDisplay *td = &t->td[q];

	    if(s->nSHPType != SHPT_POLYGON) continue;
	    if(!t->theme.polar_coords) {
		if(s->dfYMin > y_max || s->dfYMax < y_min) continue;
		if(s->dfXMin > x_max && s->dfXMax-180. < x_min) continue;
		if(s->dfXMax < x_min && s->dfXMin+180. > x_max) continue;
	    }
	    d->s = &td->bndy_segs;

	    for(j = 0; j < s->nParts; j++)
	    {
		i0 = s->panPartStart[j];
		if(j < s->nParts-1) {
		    npts = s->panPartStart[j+1] - i0;
		}
		else {
		    npts = s->nVertices - i0;
		}
		if(!t->theme.polar_coords) {
		    DrawLonLat(w, d, npts, s->padfX+i0, s->padfY+i0);
		}
		else {
		    DrawPolarLonLat(w, d, &t->theme, npts, s->padfX+i0,
				s->padfY+i0);
		}
	    }
	    iflush(d);

	    d->s = &td->fill_segs;

	    if(mp->projection == MAP_UTM_NEAR) {
		MapGetOrthoLon(w, s, numlat, lat, dlat, lon, nlon, sizes);
	    }
	    else if(!t->theme.polar_coords) {
		MapGetMerLongitudes(w, s, numlat, lat, dlat, lon, nlon, sizes);
	    }
	    else {
		MapGetMerLonPolar(w, &t->theme, s, numlat, lat, dlat, lon,
				nlon, sizes);
	    }

            /* check for counter-clockwise outer loop.
             */
	    for(l = 0; l < numlat && (nlon[l] == 0 || lon[l][0].up); l++);
	    if(mp->projection != MAP_UTM_NEAR && l < numlat) {
		/* redo it in the opposite sense */
		MapReverseLoop(s, lon[l][0].loop);
		if(!t->theme.polar_coords) {
		    MapGetMerLongitudes(w, s, numlat, lat, dlat, lon, nlon,
					sizes);
		}
		else {
		    MapGetMerLonPolar(w, &t->theme, s, numlat, lat, dlat,
					lon, nlon, sizes);
		}
		for(l = 0; l < numlat && (nlon[l] == 0 || lon[l][0].up); l++);
		if(l < numlat) continue; /* failed twice */
	    }
	    if(mp->phi != 0.)
	    {
		/* correct for rotation
		 */
		for(l = 0; l < numlat; l++)
		{
		    for(j = 0; j < nlon[l]; j++) {
			if(lon[l][j].lon < ax->x1[0]) lon[l][j].lon += 360;
			else if(lon[l][j].lon > ax->x2[0]) lon[l][j].lon -= 360;
		    }
		    qsort(lon[l], (unsigned)nlon[l],sizeof(Longitude),sort_lon);
		}
	    }

	    for(l = 0; l < numlat; l++) if(nlon[l] != 0)
	    {
		y = lat[l];
		if(!lon[l][0].up)
		{
		    up = False;
		    x = lon[l][0].lon;
		    if(x > xmin) {
			imove(d, xmin, y);
			idraw(d, x, y);
		    }
		}
		else up = True;

		for(j = 0; j < nlon[l]-1; j++)
		{
		    if(up)
		    {
			x = lon[l][j].lon;
			imove(d, x, y);
			x2 = lon[l][j+1].lon;
			idraw(d, x2, y);
			up = False;
		    }
		    else up = True;
		}
		if(up)
		{
		    x = lon[l][nlon[l]-1].lon;
		    if(xmax > x) {
			imove(d, x, y);
			idraw(d, xmax, y);
		    }
		}
	    }
	    iflush(d);
	}

	Free(nlon);
	Free(sizes);
	Free(lat);
	for(i = 0; i < numlat; i++) {
	    Free(lon[i]);
	}
	Free(lon);
}

static void
MapGetMerLongitudes(MapPlotWidget w, SHPObject *s, int numlat, double *lat,
        double dlat, Longitude **lon, int *nlon, int *sizes)
{
	MapPlotPart *mp = &w->map_plot;
        int i, i0, j, l, l1, l2, m, num, npts;
        double x1=0., y1=0., x2, y2, z,fac, mov = .00001, deg;
        Boolean add_end_pt=False;

	deg = 1./rad;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts) {
		x1 = s->padfX[i0];
		y1 = s->padfY[i0];
		add_end_pt = False;
		if(x1 != s->padfX[i0+npts-1] || y1 != s->padfY[i0+npts-1]) {
		    npts++;
		    add_end_pt = True;
		}
		if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
		    y1 = sin(y1*rad)*deg;
		}
		else if(mp->projection == MAP_UTM_NEAR) {
		    llToUtm(mp, x1, y1, &x1, &y1, &z);
		}
		else {
		    y1 = log(tan(.5*y1*rad + M_PI_4))*deg;
		}
	    }
	    if(npts < 4) continue;

	    for(i = 1; i < npts; i++)
	    {
		if(i < npts-1 || !add_end_pt) {
		    x2 = s->padfX[i0+i];
		    y2 = s->padfY[i0+i];
		}
		else {
		    x2 = s->padfX[i0];
		    y2 = s->padfY[i0];
		}
		if(x2 == x1 && y2 == y1) continue;

		if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
		    y2 = sin(y2*rad)*deg;
		}
		else if(mp->projection == MAP_UTM_NEAR) {
		    llToUtm(mp, x2, y2, &x2, &y2, &z);
		}
		else {
		    y2 = log(tan(.5*y2*rad + M_PI_4))*deg;
		}

		if(y1 == y2) continue;

		l1 = (int)((y1 - lat[0])/dlat + .5);
		l2 = (int)((y2 - lat[0])/dlat + .5);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;
					
		fac = (x2-x1)/(y2-y1);
		for(l = l1; l <= l2; l++)
		{
		    if(y1 == lat[l]) {
			y1 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y2 == lat[l]) {
			y2 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y1 < lat[l] && y2 > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = True;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		    else if(y1 > lat[l] && y2 < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = False;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		}
		x1 = x2;
		y1 = y2;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static void
MapGetMerLonPolar(MapPlotWidget w, MapPlotTheme *theme, SHPObject *s,
	int numlat, double *lat, double dlat, Longitude **lon, int *nlon,
	int *sizes)
{
	MapPlotPart *mp = &w->map_plot;
        int i, i0, j, l, l1, l2, m, num, npts;
        double x1=0., y1=0., x2, y2, fac, mov = .00001;
	double alpha, beta, gamma, c[3][3];
	double x, y, z, th, phi, sn, xp, yp, zp, r, theta, deg;
        Boolean add_end_pt=False;

	deg = 1./rad;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	/*
	 * alpha, beta, gamma from north pole to polar origin
	 */
	alpha = theme->center_lon*rad;
	beta = (90. - theme->center_lat)*rad;
	gamma = 0.;
	rotation_matrix(alpha, beta, gamma, c);

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts)
	    {
		r = s->padfX[i0];
		theta = s->padfY[i0];
		phi = (180. - theta)*DEG_TO_RADIANS;
		th = r/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		x1 = atan2(yp, xp)/DEG_TO_RADIANS;
		y1 = 90. - th/DEG_TO_RADIANS;

		add_end_pt = False;
		if(s->padfX[i0] != s->padfX[i0+npts-1] ||
			s->padfY[i0] != s->padfY[i0+npts-1]) {
		    npts++;
		    add_end_pt = True;
		}
		if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
		    y1 = sin(y1*rad)*deg;
		}
		else if(mp->projection == MAP_UTM_NEAR) {
		    llToUtm(mp, x1, y1, &x1, &y1, &z);
		}
		else {
		    y1 = log(tan(.5*y1*rad + M_PI_4))*deg;
		}
	    }
	    if(npts < 4) continue;

	    for(i = 1; i < npts; i++)
	    {
		if(i < npts-1 || !add_end_pt) {
		    r = s->padfX[i0+i];
		    theta = s->padfY[i0+i];
		}
		else {
		    r = s->padfX[i0+i];
		    theta = s->padfY[i0+i];
		}
		phi = (180. - theta)*DEG_TO_RADIANS;
		th = r/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		x2 = atan2(yp, xp)/DEG_TO_RADIANS;
		y2 = 90. - th/DEG_TO_RADIANS;

		if(x2 == x1 && y2 == y1) continue;

		if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
		    y2 = sin(y2*rad)*deg;
		}
		else if(mp->projection == MAP_UTM_NEAR) {
		    llToUtm(mp, x2, y2, &x2, &y2, &z);
		}
		else {
		    y2 = log(tan(.5*y2*rad + M_PI_4))*deg;
		}

		if(y1 == y2) continue;

		l1 = (int)((y1 - lat[0])/dlat + .5);
		l2 = (int)((y2 - lat[0])/dlat + .5);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;
					
		fac = (x2-x1)/(y2-y1);
		for(l = l1; l <= l2; l++)
		{
		    if(y1 == lat[l]) {
			y1 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y2 == lat[l]) {
			y2 += mov;
			fac = (x2-x1)/(y2-y1);
		    }
		    if(y1 < lat[l] && y2 > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = True;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		    else if(y1 > lat[l] && y2 < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].loop = j;
			lon[l][m].up = False;
			lon[l][m].lon = x1 + (lat[l] - y1)*fac;
			nlon[l]++;
		    }
		}
		x1 = x2;
		y1 = y2;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static void
DrawThemeOrthoFill(MapPlotWidget w, DrawStruct *d, MapTheme *t, int ishape)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j, l, i0, npts, nshapes, q0;
	int	i1, i2, ilat, q, numlat;
	int	*sizes = NULL, *nlon = NULL;
	Longitude **lon = NULL;
	double	xmin, xmax, ymin, ymax, x, y, dlat, lim=0.;
	double	xlo, xhi;
	double	x1, x2, x3, x4;
	double	y1, y2, y3, y4;
	double	z1, z2, z3, z4;
	double	*lat = NULL;
	Boolean up;

	if(!XtIsRealized((Widget)w) || !t->display) return;

/*3-18	if(t->theme.shape_type != SHPT_POLYGON) return; */

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];

	i1 = unscale_y(d, ymax) + 1;
	i2 = unscale_y(d, ymin) - 1;
	if(i1 > i2) {
	    i = i1;
	    i1 = i2;
	    i2 = i;
	}
	numlat = i2 - i1 + 1;
	if(numlat <= 1) return;
	lon = (Longitude **)malloc(numlat*sizeof(Longitude *));
	sizes = (int *)malloc(numlat*sizeof(int));
	lat = (double *)malloc(numlat*sizeof(double));
	nlon = (int *)malloc(numlat*sizeof(int));
	for(i = 0; i < numlat; i++)
	{
	    lon[i] = (Longitude *)malloc(sizeof(Longitude));
	    sizes[i] = 1;
	    nlon[i] = 0;
	}
	for(ilat = i1, l = 0; ilat <= i2; ilat++, l++)
	{
	    lat[l] = scale_y(d, ilat);
	}
	dlat = d->scaley;

	if(ishape >= 0) {
	    q0 = ishape;
	    nshapes = ishape+1;
	}
	else {
	    q0 = 0;
	    nshapes = t->theme.nshapes;
	}

	for(q = q0; q < nshapes; q++) if(t->td[q].display)
	{
	    SHPObject *s = t->theme.shapes[q];
	    ThemeDisplay *td = &t->td[q];

	    if(s->nSHPType != SHPT_POLYGON) continue;

	    if(!t->theme.polar_coords) {
		project(w, s->dfXMin, s->dfYMin, &x1, &y1, &z1);
		project(w, s->dfXMin, s->dfYMax, &x2, &y2, &z2);
		project(w, s->dfXMax, s->dfYMin, &x3, &y3, &z3);
		project(w, s->dfXMax, s->dfYMax, &x4, &y4, &z4);
		if((z1 <= 0. && z2 <= 0. && z3 <= 0. && z4 <= 0.))
/* this was failing for Russia and Azimuthal equal area
		(x1 < xmin && x2 < xmin && x3 < xmin && x4 < xmin) ||
		(x1 > xmax && x2 > xmax && x3 > xmax && x4 > xmax) ||
		(y1 < ymin && y2 < ymin && y3 < ymin && y4 < ymin) ||
		(y1 > ymax && y2 > ymax && y3 > ymax && y4 > ymax))
*/
		{
		    continue;
		}
	    }

	    d->s = &td->bndy_segs;

	    for(j = 0; j < s->nParts; j++)
	    {
		i0 = s->panPartStart[j];
		if(j < s->nParts-1) {
		    npts = s->panPartStart[j+1] - i0;
		}
		else {
		    npts = s->nVertices - i0;
		}
		if(!t->theme.polar_coords) {
		    DrawLonLat(w, d, npts, s->padfX+i0, s->padfY+i0);
		}
		else {
		    DrawPolarLonLat(w, d, &t->theme, npts, s->padfX+i0,
				s->padfY+i0);
		}
	    }

	    d->s = &td->fill_segs;

	    if(!t->theme.polar_coords) {
		MapGetOrthoLon(w, s, numlat, lat, dlat, lon, nlon, sizes);
	    }
	    else {
		MapGetOrthoLonPolar(w, &t->theme, s, numlat, lat, dlat,
				lon, nlon, sizes);
	    }

	    if(mp->projection == MAP_ORTHOGRAPHIC)
	    {
		lim = 1.;
	    }
	    else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	    {
		lim = .95*pi;
	    }
	    else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	    {
		lim = .95*2.;
	    }

	    for(l = 0; l < numlat; l++)
		if(nlon[l] > 0 && lat[l] > -lim && lat[l] < lim)
	    {
		y = lat[l];

		if(!lon[l][0].up)
		{
		    up = False;
		    xlo = -sqrt(lim*lim - y*y);
		    if(xlo < xmin) xlo = xmin;
		    x = lon[l][0].lon;
		    if(x > xlo) {
			imove(d, xlo, y);
			idraw(d, x, y);
		    }
		}
		else up = True;

		for(j = 0; j < nlon[l]-1; j++)
		{
		    if(up)
		    {
			x = lon[l][j].lon;
			imove(d, x, y);
			x2 = lon[l][j+1].lon;
			idraw(d, x2, y);
			up = False;
		    }
		    else up =  True;
		}
		if(up)
		{
		    xhi = sqrt(lim*lim - y*y);
		    if(xhi > xmax) xhi = xmax;
		    x = lon[l][nlon[l]-1].lon;
		    if(xhi > x) {
			imove(d, x, y);
			idraw(d, xhi, y);
		    }
		}
	    }
	    iflush(d);
	}

	Free(nlon);
	Free(sizes);
	Free(lat);
	for(i = 0; i < numlat; i++) {
	    Free(lon[i]);
	}
	Free(lon);
}

static void
MapGetOrthoLon(MapPlotWidget w, SHPObject *s, int numlat, double *lat,
		double dlat, Longitude **lon, int *nlon, int *sizes)
{
	int i, i0, j, l, l1, l2, m, num, npts;
	double	x, y, z, fac, x0, y0, start_x=0., start_y=0.;
	Boolean last_in=False;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts) {
	        x0 = s->padfX[i0];
	        y0 = s->padfY[i0];
		project(w, x0, y0, &x, &y, &z);
		last_in = (z > 0.) ? True : False;
		if(last_in)
		{
		    start_x = x;
		    start_y = y;
		}
	    }

	    for(i = 1; i < npts; i++)
	    {
		x0 = s->padfX[i0+i];
		y0 = s->padfY[i0+i];
		project(w, x0, y0, &x, &y, &z);

		if(z <= 0.) {
		    last_in = False;
		    continue;
		}
		if(!last_in) {
		    start_x = x;
		    start_y = y;
		    last_in = True;
		    continue;
		}

		if(y == start_y)
		{
		    last_in = True;
		    continue;
		}
		l1 = (int)((start_y - lat[0])/dlat);
		l2 = (int)((y - lat[0])/dlat);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;
					
		fac = (x-start_x)/(y-start_y);
		for(l = l1; l <= l2; l++)
		{
		    if(start_y <= lat[l] && y > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].up = True;
			lon[l][m].lon = start_x + (lat[l] - start_y)*fac;
			nlon[l]++;
		    }
		    else if(start_y >= lat[l] && y < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].up = False;
			lon[l][m].lon = start_x + (lat[l] - start_y)*fac;
			nlon[l]++;
		    }
		}
		start_x = x;
		start_y = y;
		last_in = True;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static void
MapGetOrthoLonPolar(MapPlotWidget w, MapPlotTheme *theme, SHPObject *s,
        int numlat, double *lat, double dlat, Longitude **lon, int *nlon,
	int *sizes)
{
	int i, i0, j, l, l1, l2, m, num, npts;
	double	fac, x0, y0;
	double	start_x=0., start_y=0.;
	double x, y, z, xp, yp, zp;
	double alpha, beta, gamma, c[3][3], sn, th, phi, r, theta;
	Boolean last_in=False;

	for(i = 0; i < numlat; i++) nlon[i] = 0;

	/*
	 * alpha, beta, gamma from north pole to polar origin
	 */
	alpha = theme->center_lon*rad;
	beta = (90. - theme->center_lat)*rad;
	gamma = 0.;
	rotation_matrix(alpha, beta, gamma, c);

	for(j = 0; j < s->nParts; j++)
	{
	    i0 = s->panPartStart[j];
	    if(j < s->nParts-1) {
		npts = s->panPartStart[j+1] - i0;
	    }
	    else {
		npts = s->nVertices - i0;
	    }

	    num = numlat-1;

	    if(npts) {
	        r = s->padfX[i0];
		theta = s->padfY[i0];
		phi = (180. - theta)*DEG_TO_RADIANS;
		th = r/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		x0 = atan2(yp, xp)/DEG_TO_RADIANS;
		y0 = 90. - th/DEG_TO_RADIANS;

		project(w, x0, y0, &x, &y, &z);
		last_in = (z > 0.) ? True : False;
		if(last_in)
		{
		    start_x = x;
		    start_y = y;
		}
	    }

	    for(i = 1; i < npts; i++)
	    {
		r = s->padfX[i0+i];
		theta = s->padfY[i0+i];
		phi = (180. - theta)*DEG_TO_RADIANS;
		th = r/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		x0 = atan2(yp, xp)/DEG_TO_RADIANS;
		y0 = 90. - th/DEG_TO_RADIANS;

		project(w, x0, y0, &x, &y, &z);

		if(z <= 0.) {
		    last_in = False;
		    continue;
		}
		if(!last_in) {
		    start_x = x;
		    start_y = y;
		    last_in = True;
		    continue;
		}

		if(y == start_y)
		{
		    last_in = True;
		    continue;
		}
		l1 = (int)((start_y - lat[0])/dlat);
		l2 = (int)((y - lat[0])/dlat);
		if(l1 > l2)
		{
		    l = l1;
		    l1 = l2;
		    l2 = l;
		}
		if(l1 < 0) l1 = 0;
		if(l2 > num) l2 = num;
					
		fac = (x-start_x)/(y-start_y);
		for(l = l1; l <= l2; l++)
		{
		    if(start_y <= lat[l] && y > lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].up = True;
			lon[l][m].lon = start_x + (lat[l] - start_y)*fac;
			nlon[l]++;
		    }
		    else if(start_y >= lat[l] && y < lat[l])
		    {
			m = nlon[l];
			if(nlon[l] == sizes[l])
			{
			    lon[l] = (Longitude *)realloc(lon[l],
					    (m+10)*sizeof(Longitude));
			    sizes[l] += 10;
			}
			lon[l][m].up = False;
			lon[l][m].lon = start_x + (lat[l] - start_y)*fac;
			nlon[l]++;
		    }
		}
		start_x = x;
		start_y = y;
		last_in = True;
	    }
	}
	for(l = 0; l < numlat; l++)
	{
	    qsort(lon[l], (unsigned)nlon[l], sizeof(Longitude), sort_lon);
	}
}

static int
sort_lon(const void *A, const void *B)
{
	Longitude *a = (Longitude *)A;
	Longitude *b = (Longitude *)B;
	if(a->lon != b->lon) return(a->lon < b->lon ? -1 : 1);
	return(0);
}

static void
DrawThemePts(MapPlotWidget w, DrawStruct *d, MapTheme *t, int ishape)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int q, q0, nshapes;
	double xmin, xmax, ymin, ymax;

	if(!XtIsRealized((Widget)w) ||
		(!t->display && t->theme.shape_type != SHPT_ARC)) return;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];

	if(ishape >= 0) {
	    q0 = ishape;
	    nshapes = ishape+1;
	}
	else {
	    q0 = 0;
	    nshapes = t->theme.nshapes;
	}

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
		mp->projection == MAP_UTM ||
		mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		mp->projection == MAP_MERCATOR)
	{
	    double x, y1, y2;

	    unproject(w, xmin, ymin, &x, &y1);
	    unproject(w, xmin, ymax, &x, &y2);

	    for(q = q0; q < nshapes; q++) if(t->td[q].display)
	    {
		SHPObject *s = t->theme.shapes[q];
		ThemeDisplay *td = &t->td[q];

		if(!t->theme.polar_coords) {
		    if(s->dfYMin > y2 || s->dfYMax < y1) continue;
		    if(s->dfXMin > xmax && s->dfXMax-180. < xmin) continue;
		    if(s->dfXMax < xmin && s->dfXMin+180. > xmax) continue;
		}

		d->s = &td->bndy_segs;
		if(!t->theme.polar_coords) {
		    DrawLonLat(w, d, td->npts, td->lon, td->lat);
		}
		else {
		    DrawPolarLonLat(w, d, &t->theme, td->npts, td->lon,td->lat);
		}
	    }
	}
	else 
	{
	    double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

	    for(q = q0; q < nshapes; q++) if(t->td[q].display)
	    {
		SHPObject *s = t->theme.shapes[q];
		ThemeDisplay *td = &t->td[q];

		if(!t->theme.polar_coords) {
		    project(w, s->dfXMin, s->dfYMin, &x1, &y1, &z1);
		    project(w, s->dfXMin, s->dfYMax, &x2, &y2, &z2);
		    project(w, s->dfXMax, s->dfYMin, &x3, &y3, &z3);
		    project(w, s->dfXMax, s->dfYMax, &x4, &y4, &z4);
		    if((z1 <= 0. && z2 <= 0. && z3 <= 0. && z4 <= 0.)) continue;
		}

		d->s = &td->bndy_segs;

		if(!t->theme.polar_coords) {
		    DrawLonLat(w, d, td->npts, td->lon, td->lat);
		}
		else {
		    DrawPolarLonLat(w, d, &t->theme, td->npts, td->lon,td->lat);
		}
	    }
	}
	iflush(d);
}

static void
DrawThemeGrid(MapPlotWidget w, DrawStruct *d, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double  xmin, xmax, ymin, ymax, del, dash_length = 2, theta_max;
	double alpha, beta, gamma, c[3][3], lat, max_theta, rmax;
	double values[MAX_POLAR_DIST];
	int i, j, ndigit;

	Free(t->grid.segs);
	t->grid.size_segs = 0;
	t->grid.nsegs = 0;
	d->s = &t->grid;

	if(!t->theme.polar_coords) return;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];
	SetClipArea(d, xmin, ymin, xmax, ymax);

	del = fabs(scale_x(d, (int)dash_length) - scale_x(d, 0));

	if(mp->projection == MAP_ORTHOGRAPHIC ||
		mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
		mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    if(del > 1.) del = 1.;
	    del = asin(del)/rad;
	}

	/*
	 * alpha, beta, gamma from north pole to polar origin
	 */
	alpha = t->theme.center_lon*rad;
	beta = (90. - t->theme.center_lat)*rad;
	gamma = 0.;
	rotation_matrix(alpha, beta, gamma, c);

	theta_max = t->rmax/6371.;
	theta_max /= DEG_TO_RADIANS;
	if(theta_max > 90.) theta_max = 90.;
	max_theta = 0;

	for(i = j = 0; i < 360; i += 20)
       	{
	    double maxt=0.;
	    if(gridLon(w, d, (double)i, 90.-theta_max, 90., del, &t->az[j], c,
			&maxt))
	    {
		if(j < MAX_POLAR_AZ) {
		    t->az[j++].value = 180. - (double)i;
		}
		if(maxt > max_theta) max_theta = maxt;
	    }
	}
	t->numaz = j;

	rmax = max_theta*6371;

	nicex(0., rmax, 3, MAX_POLAR_DIST, &t->numdist, values, &ndigit,
		&t->ndeci);
	for(i = 1, j = 0; i < t->numdist; i++) {
	    lat = 90. - values[i]/(6371.*DEG_TO_RADIANS);
	    if(gridLat(w, d, lat, -180., 180., 2*del, &t->dist[j], c))
	    {
		t->dist[j++].value = values[i];
	    }
	}
	t->numdist = j;
}

static Boolean
gridLon(MapPlotWidget w, DrawStruct *d, double longitude, double min_lat,
		double max_lat, double del, PolarLabel *p, double c[3][3],
		double *max_theta)
{
	double lat, lat1, lat2, latitude, x1, y1, z1, x2, y2, z2, dl;
	double x, y, z, xp, yp, zp, phi, theta, th, sn, lon, lon1, lon2;
	Boolean drew_one = False;

	dl = (max_lat - min_lat)/1000.;
	for(latitude = min_lat; latitude < max_lat && dl != 0.; latitude += dl)
        {
	    phi = longitude*DEG_TO_RADIANS;
	    theta = (90-latitude)*DEG_TO_RADIANS;
	    sn = sin(theta);
	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    theta = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat = 90. - theta/DEG_TO_RADIANS;
	    lon = atan2(yp, xp)/DEG_TO_RADIANS;

	    project(w, lon, lat, &x1, &y1, &z1);

	    if(z1 > 0.)
	    {
		imove(d, x1, y1);
		if(d->lastin)
		{
		    p->x = unscale_x(d, x1);
		    p->y = unscale_y(d, y1);
		    p->border = (latitude == min_lat) ? False : True;
		    drew_one = True;
		    break;
		}
	    }
	}
	if(!drew_one) return False;

	*max_theta = 0;
	for(latitude = min_lat + 4*del; latitude < max_lat; latitude += 5*del)
	{
	    phi = longitude*DEG_TO_RADIANS;
	    theta = (90-latitude)*DEG_TO_RADIANS;
	    sn = sin(theta);
	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    th = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat1 = 90. - th/DEG_TO_RADIANS;
	    lon1 = atan2(yp, xp)/DEG_TO_RADIANS;

	    th = (90-(latitude+del))*DEG_TO_RADIANS;
	    sn = sin(th);
	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(th);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    th = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat2 = 90. - th/DEG_TO_RADIANS;
	    lon2 = atan2(yp, xp)/DEG_TO_RADIANS;

	    project(w, lon1, lat1, &x1, &y1, &z1);
	    project(w, lon2, lat2, &x2, &y2, &z2);

	    if(z1 > 0. && z2 > 0.)
	    {
		imove(d, x1, y1);
		idraw(d, x2, y2);
		drew_one = True;
		if(*max_theta < theta) *max_theta = theta;
	    }
	}
	return drew_one;
}

static Boolean
gridLat(MapPlotWidget w, DrawStruct *d, double latitude, double min_lon,
		double max_lon, double del, PolarLabel *p, double c[3][3])
{
	double longitude, x1, y1, z1, x2, y2, z2, dl;
	double x, y, z, xp, yp, zp, phi, theta, sn, lon1, lon2, lat1, lat2;
	int iy;

	dl = 2.*sin((90.-latitude)*DEG_TO_RADIANS);
	if(dl != 0.) {
	    del /= dl;
	}

	p->y = 1000000;
	for(longitude = min_lon+4*del; longitude < max_lon; longitude += 5*del)
	{
	    phi = longitude*DEG_TO_RADIANS;
	    theta = (90-latitude)*DEG_TO_RADIANS;
	    sn = sin(theta);
	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    theta = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat1 = 90. - theta/DEG_TO_RADIANS;
	    lon1 = atan2(yp, xp)/DEG_TO_RADIANS;

	    phi = (longitude+del)*DEG_TO_RADIANS;
	    theta = (90-latitude)*DEG_TO_RADIANS;
	    sn = sin(theta);
	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    theta = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat2 = 90. - theta/DEG_TO_RADIANS;
	    lon2 = atan2(yp, xp)/DEG_TO_RADIANS;

	    project(w, lon1, lat1, &x1, &y1, &z1);
	    project(w, lon2, lat2, &x2, &y2, &z2);

	    x = x2 - x1; /* guard against dashes from -180 to 180, etc. */
	    y = y2 - y1;
	    if(z1 > 0. && z2 > 0. && sqrt(x*x + y*y) < 2*del)
	    {
		imove(d, x1, y1);
		idraw(d, x2, y2);
		iy = unscale_y(d, y1);
		if(iy < p->y) {p->y = iy; p->x = unscale_x(d, x1);}
		iy = unscale_y(d, y2);
		if(iy < p->y) {p->y = iy; p->x = unscale_x(d, x2);}
	    }
	}
	return True;
}

static void
DrawPolarGrid(MapPlotWidget w, DrawStruct *d, int dash_length)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double xmin, xmax, ymin, ymax, del;
	double rmax, rmin;
	double values[MAX_POLAR_DIST];
	int i, j, i0=0, ndigit;

        Free(mp->grid.segs);
        mp->grid.size_segs = 0;
        mp->grid.nsegs = 0;
        d->s = &mp->grid;
	mp->numaz = 0;
	mp->numdist = 0;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];
	SetClipArea(d, xmin, ymin, xmax, ymax);

	del = fabs(scale_x(d, dash_length) - scale_x(d, 0));

	rmax = 0;
	rmin = -1;

	if(!getPolarDistRange(d, mp->polar_max_radius, &rmin, &rmax)) {
	    mp->numaz = 0;
	    mp->numdist = 0;
	    return;
	}
	rmax += .01*(rmax-rmin);

	if(mp->polar_del_az > 0.)
	{
	    for(i = j = 0; i < 360; i += (int)(mp->polar_del_az))
	    {
		if(gridPolarDist(d, (double)i, mp->polar_max_radius, del,
				&mp->az[j]))
		{
		    if(j == MAX_POLAR_AZ) break;
		    // to force values between -180 and 180 use the following
//		    mp->az[j++].value = (i<=180) ? (double)i : -(double)(360-i);
		    // for values from 0 to 360 use
		    mp->az[j++].value = (double)i;
		}
	    }
	    mp->numaz = j;
	}
	else {
	    mp->numaz = 0;
	}

	if(rmin < 0.) {
	    mp->numdist = 0;
	    return;
	}

	if(xmin <= 0. && xmax >= 0. && ymin <= 0. && ymax >= 0.) rmin = 0.;

	if(mp->polar_del_dist <= 0.) {
	    nicex(rmin, rmax, 3, MAX_POLAR_DIST, &mp->numdist, values, &ndigit,
		&mp->polar_ndeci);
	}
	else {
	    double r;
	    for(i = 0, j = 1; i < MAX_POLAR_DIST; j++)
	    {
		r = j*mp->polar_del_dist;
		if(r >= rmin && r <= rmax) {
		    values[i++] = r;
		}
		else if(r > rmax) break;
	    }
	    mp->numdist = i;
	}

	if(mp->numdist > 0) i0 = (values[0] > 0.) ? 0 : 1;
	for(i = i0, j = 0; i < mp->numdist; i++) {
	    double r = values[i];
	    double arc_del = (r != 0.) ? del/(r*DEG_TO_RADIANS) : 2.;
	    if(gridPolarAz(w, d, r, -180., 180., arc_del, &mp->dist[j]))
	    {
		mp->dist[j++].value = values[i];
	    }
	}
	mp->numdist = j;
}

static Boolean
getPolarDistRange(DrawStruct *d, double maxr, double *rmin, double *rmax)
{
	int i;
	double azimuth, x, y, r, dl, cs, sn;
	Boolean drew_one = False;

	dl = maxr/200.;

	*rmin = -1.;
	*rmax = 0;

	for(i = 0; i < 360; i += 10)
	{
	    azimuth = i*DEG_TO_RADIANS;
	    sn = sin(azimuth);
	    cs = cos(azimuth);

	    for(r = dl; r < maxr; r += dl)
	    {
		x = r*sn;
		y = r*cs;

		imove(d, x, y);

		if(d->lastin) {
		    drew_one = True;
		    if(*rmin < 0.) *rmin = r;
		    *rmax = r;
		}
	    }
	}
	return drew_one;
}

static Boolean
gridPolarDist(DrawStruct *d, double azimuth, double maxr, double del,
		PolarLabel *p)
{
	double x1, y1, x2, y2, r, dl;
	Boolean drew_one = False;

	if(del <= 0.) return False;

	azimuth *= DEG_TO_RADIANS;

	dl = maxr/1000.;
	for(r = maxr; r > 0; r -= dl)
        {
	    x1 = r*sin(azimuth);
	    y1 = r*cos(azimuth);
	    imove(d, x1, y1);
	    if(d->lastin)
	    {
		p->x = unscale_x(d, x1);
		p->y = unscale_y(d, y1);
		drew_one = True;
		break;
	    }
	}
	if(!drew_one) return False;

	for(r = 4*del; r < maxr; r += 5*del)
	{
	    x1 = r*sin(azimuth);
	    y1 = r*cos(azimuth);

	    x2 = (r+del)*sin(azimuth);
	    y2 = (r+del)*cos(azimuth);

	    imove(d, x1, y1);
	    idraw(d, x2, y2);
	    if(d->lastin) {
		drew_one = True;
	    }
	}
	return drew_one;
}

static Boolean
gridPolarAz(MapPlotWidget w, DrawStruct *d, double r, double min_az,
		double max_az, double del, PolarLabel *p)
{
	double az, x1, y1, x2, y2, dl;
	int iy;
	Boolean drew_one = False;

	if(del <= 0.) return False;

	imove(d, 0, r);
	if(d->lastin) {
	    p->y = unscale_y(d, r);
	    p->x = unscale_x(d, 0.);
	}
	else
	{
	    dl = del/100;
	    p->y = 1000000;
	    for(az = min_az+4*dl; az < max_az; az += 5*dl)
	    {
		x2 = r*sin((az+del)*DEG_TO_RADIANS);
		y2 = r*cos((az+del)*DEG_TO_RADIANS);

		imove(d, x2, y2);
		if(d->lastin) {
		    iy = unscale_y(d, y2);
		    if(iy < p->y) {p->y = iy; p->x = unscale_x(d, x2);}
		}
	    }
	}

	for(az = min_az+4*del; az < max_az; az += 5*del)
	{
	    x1 = r*sin(az*DEG_TO_RADIANS);
	    y1 = r*cos(az*DEG_TO_RADIANS);
	    x2 = r*sin((az+del)*DEG_TO_RADIANS);
	    y2 = r*cos((az+del)*DEG_TO_RADIANS);

	    imove(d, x1, y1);
	    idraw(d, x2, y2);
	    if(d->lastin) {
		drew_one = True;
	    }
	}
	return drew_one;
}

void
MapPlotPositionPolarSelection(MapPlotWidget w, double azimuth,
		double del_azimuth, double radius, double del_radius,
		Boolean do_callback)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	if(!mp->display_polar_selection) {
	    MapPlotDisplayPolarSelection(w, True, azimuth, del_azimuth, radius,
				del_radius, do_callback);
	    return;
	}

	mp->polar_azimuth = azimuth;
	if(mp->polar_azimuth < 0.) mp->polar_azimuth += 360.;
	mp->polar_del_azimuth = del_azimuth;
	mp->polar_radius = radius;
	mp->polar_del_radius = del_radius;

	drawPolarSelect(w, mp->polar_line->line.lon, mp->polar_line->line.lat);
	Free(mp->polar_line->s.segs);
	mp->polar_line->s.size_segs = 0;
	mp->polar_line->s.nsegs = 0;
	ax->d.s = &mp->polar_line->s;
	DrawLonLat(w, &ax->d, mp->polar_line->line.npts,
			mp->polar_line->line.lon, mp->polar_line->line.lat);
	findPolarSelectSegs(w, mp->polar_line->line.lon,
				mp->polar_line->line.lat);
	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	RedisplayOverlays(w, ax->pixmap);
	RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	_AxesRedisplayXor((AxesWidget)w);

	if(do_callback) {
	    doPolarSelectCallback(w);
	}
}

static void
doPolarSelectCallback(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	PolarSelectStruct p;
	double r, az, r1, r2, a1, a2;
	int i, n=0;

	p.radius = mp->polar_radius;
	p.azimuth = mp->polar_azimuth;
	p.del_radius = mp->polar_del_radius;
	p.del_azimuth = mp->polar_del_azimuth;
	p.avg_radius = 0.;
	p.avg_azimuth = 0.;

	r1 = mp->polar_radius;
	r2 = mp->polar_radius + mp->polar_del_radius;
	a1 = mp->polar_azimuth;
	a2 = mp->polar_azimuth + mp->polar_del_azimuth;

	for(i = 0; i < mp->nsymbol; i++) if(mp->symbol[i].polar_coords)
	{
	    r = mp->symbol[i].lon;
	    az = mp->symbol[i].lat;

	    if(r >= r1 && r <= r2 && az >= a1 && az <= a2) {
		p.avg_azimuth += az;
		p.avg_radius += r;
		n++;
	    }
	}
	if( a2 > 360. )
	{
	    a2 -= 360.;
	    for(i = 0; i < mp->nsymbol; i++) if(mp->symbol[i].polar_coords)
	    {
		r = mp->symbol[i].lon;
		az = mp->symbol[i].lat;

		if(az <= a2 && r >= r1 && r <= r2) {
		    p.avg_azimuth += az;
		    p.avg_radius += r;
		    n++;
		}
	    }
	}
	if(n) {
	    p.avg_radius /= (double)n;
	    p.avg_azimuth /= (double)n;
	}

	XtCallCallbacks((Widget)w, XtNpolarSelectCallback, &p);
}

void 
MapPlotDisplayPolarSelection(MapPlotWidget w, Boolean display, double azimuth,
		double del_azimuth, double radius, double del_radius,
		Boolean do_callback)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(display == mp->display_polar_selection) return;

	mp->display_polar_selection = display;
	if(display) {
	    LineInfo line_info_null = LINE_INFO_INIT;
	    MapPlotLine line;
	    line.id = 0;
	    line.label = NULL;
	    line.line = line_info_null;
	    line.polar_coords = True;
	    line.npts = 102;
	    line.lon = (double *)AxesMalloc((Widget)w, // radius
				line.npts*sizeof(double));
	    line.lat = (double *)AxesMalloc((Widget)w, // azimuth
				line.npts*sizeof(double));
mp->polar_radius = radius;
mp->polar_azimuth = azimuth;
mp->polar_del_azimuth = del_azimuth;
mp->polar_del_radius = del_radius;
	    if(mp->polar_radius < 0.) {
		mp->polar_radius = .25*mp->polar_max_radius;
		mp->polar_del_radius = .5*mp->polar_max_radius;
	    }
//	    mp->polar_azimuth = 100.;
//	    mp->polar_del_azimuth = 20.;

	    drawPolarSelect(w, line.lon, line.lat);
	    mp->polar_select_id = MapPlotAddLine(w, &line, True);
	    for(i = 0; i < mp->nlines; i++) {
		if(mp->polar_select_id == mp->line[i]->line.id) {
		    mp->polar_line = mp->line[i];
		}
	    }

	    findPolarSelectSegs(w, line.lon, line.lat);

	    Free(line.lat);
	    Free(line.lon);
	}
	else {
	    for(i = 0; i < mp->nlines; i++)
	    {
		if(mp->polar_select_id == mp->line[i]->line.id)
		{
		    MapPlotDeleteLine(w, i, True);
		}
	    }
	}
	_MapPlotRedisplay(w);

	if(display && do_callback) {
	    doPolarSelectCallback(w);
	}
}

Boolean MapPlotGetPolarSelection(MapPlotWidget w, double *azimuth,
		double *del_azimuth, double *radius, double *del_radius)
{
	MapPlotPart *mp = &w->map_plot;

	if( !mp->display_polar_selection ) return False;
	*radius = mp->polar_radius;
	*del_radius = mp->polar_del_radius;
	*azimuth = mp->polar_azimuth;
	*del_azimuth = mp->polar_del_azimuth;
	return True;
}

static void
findPolarSelectSegs(MapPlotWidget w, double *lon, double *lat)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i;
	double d, dmin, x, y, ix, iy, jx, jy;

	x = lon[50]*sin(lat[50]*DEG_TO_RADIANS);
	y = lon[50]*cos(lat[50]*DEG_TO_RADIANS);
	ix = (double)unscale_x(&ax->d, x);
	iy = (double)unscale_y(&ax->d, y);

	mp->polar_select_i3 = 0;
	dmin = 1.e+10;

	/* check for the segment that is closest to lon[51], lat[51]
	 */
	for(i = 0; i < mp->polar_line->s.nsegs; i++) {
	    jx = mp->polar_line->s.segs[i].x1,
	    jy = mp->polar_line->s.segs[i].y1,

	    d = (jx-ix)*(jx-ix) + (jy-iy)*(jy-iy);
	    if(d < dmin) {
		dmin = d;
		mp->polar_select_i3 = i;
	    }
	}
}

static void
drawPolarSelect(MapPlotWidget w, double *radius, double *azimuth)
{
	MapPlotPart *mp = &w->map_plot;
	double delaz, a, r;
	int i, n;

	delaz = mp->polar_del_azimuth/49.;
	azimuth[0] = mp->polar_azimuth;
	radius[0] = mp->polar_radius;
	r = mp->polar_radius + mp->polar_del_radius;
	n = 1;
	for(i = 0; i < 50; i++) {
	    azimuth[n] = mp->polar_azimuth + i*delaz;
	    radius[n] = r;
	    n++;
	}
	azimuth[n] = mp->polar_azimuth + mp->polar_del_azimuth;
	radius[n] = mp->polar_radius;
	a = mp->polar_azimuth + mp->polar_del_azimuth;
	n++;
	for(i = 0; i < 50; i++) {
	    azimuth[n] = a - i*delaz;
	    radius[n] = mp->polar_radius;
	    n++;
	}
}

static void
project(MapPlotWidget w, double lon, double lat, double *xp, double *yp,
			double *zp)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double	sn, x, y, z, theta, phi, x1, x2, y1, y2;
	
	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	    while(lon < ax->x1[0]) lon += 360;
	    while(lon > ax->x2[0]) lon -= 360;
	    *zp = (lon < x1 || lon > x2 || lat < y1 || lat > y2) ? -1. : 1.;
	    *xp = lon;
	    *yp = lat;
	}
	else if(mp->projection == MAP_UTM_NEAR)
	{
	    llToUtm(mp, lon, lat, xp, yp, zp);
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA)
	{
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	    while(lon < ax->x1[0]) lon += 360;
	    while(lon > ax->x2[0]) lon -= 360;
	    if(lat > mp->mercator_max_lat)
	    {
		lat = mp->mercator_max_lat;
	    }
	    if(lat < -mp->mercator_max_lat)
	    {
		lat = -mp->mercator_max_lat;
	    }
	    lat = sin(lat*rad)/rad;
	    *zp = (lon < x1 || lon > x2 || lat < y1 || lat > y2) ? -1. : 1.;
	    *xp = lon;
	    *yp = lat;
	}
	else if(mp->projection == MAP_MERCATOR)
	{
	    x1 = ax->x1[ax->zoom];
	    x2 = ax->x2[ax->zoom];
	    y1 = ax->y1[ax->zoom];
	    y2 = ax->y2[ax->zoom];
	    while(lon < ax->x1[0]) lon += 360;
	    while(lon > ax->x2[0]) lon -= 360;
	    if(lat > mp->mercator_max_lat)
	    {
		lat = mp->mercator_max_lat;
	    }
	    if(lat < -mp->mercator_max_lat)
	    {
		lat = -mp->mercator_max_lat;
	    }
	    lat = log(tan(.5*lat*rad + M_PI_4))/rad;
	    *zp = (lon < x1 || lon > x2 || lat < y1 || lat > y2) ? -1. : 1.;
	    *xp = lon;
	    *yp = lat;
	    return;
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC)
	{
	    theta = (90. - lat)*rad;
	    phi = lon*rad;
	    sn = sin(theta);

	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);
	
	    *zp = mp->c[2][0]*x + mp->c[2][1]*y + mp->c[2][2]*z;
	    if(*zp < 0.) return;

	    *xp = mp->c[0][0]*x + mp->c[0][1]*y + mp->c[0][2]*z;
	    *yp = mp->c[1][0]*x + mp->c[1][1]*y + mp->c[1][2]*z;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	{
	    theta = (90. - lat)*rad;
	    phi = lon*rad;
	    sn = sin(theta);

	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);
	
	    *zp = mp->c[2][0]*x + mp->c[2][1]*y + mp->c[2][2]*z;
	    *xp = mp->c[0][0]*x + mp->c[0][1]*y + mp->c[0][2]*z;
	    *yp = mp->c[1][0]*x + mp->c[1][1]*y + mp->c[1][2]*z;

	    theta = atan2(sqrt(*xp * *xp + *yp * *yp), *zp);
	    if(fabs(theta) > .95*pi)
	    {
		*zp = -1.;
	    }
	    else
	    {
		phi = atan2(*yp, *xp);
		*xp = theta*cos(phi);
		*yp = theta*sin(phi);
		*zp = 1.;
	    }
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    theta = (90. - lat)*rad;
	    phi = lon*rad;
	    sn = sin(theta);

	    x = sn*cos(phi);
	    y = sn*sin(phi);
	    z = cos(theta);
	
	    *zp = mp->c[2][0]*x + mp->c[2][1]*y + mp->c[2][2]*z;
	    *xp = mp->c[0][0]*x + mp->c[0][1]*y + mp->c[0][2]*z;
	    *yp = mp->c[1][0]*x + mp->c[1][1]*y + mp->c[1][2]*z;

	    theta = atan2(sqrt(*xp * *xp + *yp * *yp), *zp);
	    sn = 2.*sin(.5*theta);
	    if(fabs(sn) > .95*2.)
	    {
		*zp = -1.;
	    }
	    else
	    {
		phi = atan2(*yp, *xp);
		*xp = sn*cos(phi);
		*yp = sn*sin(phi);
		*zp = 1.;
	    }
	}
	else if(mp->projection == MAP_POLAR)
	{
	    *xp = lon*sin(lat*DEG_TO_RADIANS);
	    *yp = lon*cos(lat*DEG_TO_RADIANS);
	    *zp = 1.;
	}
}

static void
llToUtm(MapPlotPart *mp, double lon, double lat, double *xp, double *yp,
		double *zp)
{
//        double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
//        double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
//        Ellipsoid( 1, "Airy", 6377563, 0.00667054),

    double a = 6377563;
//    double eccSquared = 0.00667054;
    double k0 = 0.9996;
//    double eccPrimeSquared;
    double N, T, C, A, M;
    double A2, A3, A4, A5, A6, T2;
    double sin_lat, cos_lat, tan_lat;
    double lat_rad = lat*rad;

    *xp = 0.;
    *yp = 0.;
    if(mp->utm_cell_letter == '\0') {
	*zp = -1.;
	return;
    }

    if(mp->utm_cell_zone != 1 && mp->utm_cell_zone != 60) {
	lon = (lon+180)-(int)((lon+180)/360)*360-180; // -180.00..179.9
    }

    if(fabs(lon - mp->utm_center_lon) > 90.
		|| fabs(lat - mp->utm_center_lat) > 50.)
    {
	*zp = -1.;
	return;
    }

//    eccPrimeSquared = (eccSquared)/(1-eccSquared);

/*
    N = a/sqrt(1-eccSquared*sin(lat_rad)*sin(lat_rad));
    T = tan(lat_rad)*tan(lat_rad);
    C = eccPrimeSquared*cos(lat_rad)*cos(lat_rad);
    A = cos(lat_rad)*(lon - mp->utm_center_lon)*rad;

    M = a*((1 - eccSquared/4 - 3*eccSquared*eccSquared/64
		- 5*eccSquared*eccSquared*eccSquared/256)*lat_rad
		- (3*eccSquared/8 + 3*eccSquared*eccSquared/32
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*lat_rad)
		+ (15*eccSquared*eccSquared/256 +
		    45*eccSquared*eccSquared*eccSquared/1024)*sin(4*lat_rad)
		- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*lat_rad));
    // UTMEasting 
    *xp = (double)(k0*N*(A+(1-T+C)*A*A*A/6
		+ (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120)
			+ 500000.0);

    // UTMNorthing
    *yp = (double)(k0*(M+N*tan(lat_rad)*(A*A/2+
			(5-T+9*C+4*C*C)*A*A*A*A/24
		+ (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
*/
    sin_lat = sin(lat_rad);
    cos_lat = cos(lat_rad);
    tan_lat = tan(lat_rad);

    N = a/sqrt(1-mp->e_Squared*sin_lat*sin_lat);
    T = tan_lat*tan_lat;
    C = mp->e_PrimeSquared*cos_lat*cos_lat;
    A = cos_lat*(lon - mp->utm_center_lon)*rad;

    M = a*(mp->M1*lat_rad - mp->M2*sin(2*lat_rad) + mp->M3*sin(4*lat_rad)
		- mp->M4*sin(6*lat_rad));

    A2 = A*A;
    A3 = A2*A;
    A4 = A3*A;
    A5 = A4*A;
    A6 = A5*A;
    T2 = T*T;

    // UTMEasting 
    *xp = (double)(k0*N*(A + (1 - T + C)*A3/6
		+ (5 - 18*T + T2 + 72*C - 58*mp->e_PrimeSquared)*A5/120)
		+ 500000.0);

    // UTMNorthing
    *yp = (double)(k0*(M + N*tan_lat*(A2/2 + (5 - T + 9*C + 4*C*C)*A4/24
		+ (61 - 58*T + T2 + 600*C - 330*mp->e_PrimeSquared)*A6/720)));
    
    if(mp->utm_cell_letter - 'N' < 0) {
	//10000000 meter offset for southern hemisphere
	*yp += 10000000.0;
    }
    *zp = 1.;
}

static void
utmToLL(MapPlotPart *mp, double UTMEasting, double UTMNorthing, double *lon,
		double *lat)
{
//converts UTM coords to lat/long.  Equations from USGS Bulletin 1532
//East Longitudes are positive, West longitudes are negative.
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees.
// USGS Series  	Professional Paper
// Report Number 	1395
// Title 	Map projections; a working manual
// Edition 	Supersedes Bulletin 1532
// Language 	ENGLISH
// Author(s) 	Snyder, John P.
// Year 	1987
// Tranverse Mercator Projection formulae on page 61.

    double k0 = 0.9996;
    double a = 6377563;
//    double eccSquared = 0.00667054;
//    double eccPrimeSquared;
//    double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
    double N1, T1, C1, R1, D, M;
    double D2, D3, D4, D5, D6;
    double mu, phi1Rad;
//  doduble phi1;
    double sin_phi, cos_phi, tan_phi;
    double x, y;


    x = UTMEasting - 500000.0; //remove 500,000 meter offset for longitude
    y = UTMNorthing;

    if(mp->utm_cell_letter - 'N' < 0) { // point is in southern hemisphere
	//remove 10,000,000 meter offset used for southern hemisphere
	y -= 10000000.0;
    }

/*
    eccPrimeSquared = (eccSquared)/(1-eccSquared);

    M = y / k0;
    mu = M/(a*(1 - eccSquared/4 - 3*eccSquared*eccSquared/64
		- 5*eccSquared*eccSquared*eccSquared/256));

    phi1Rad = mu + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu)
		+ (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
		+(151*e1*e1*e1/96)*sin(6*mu);
    phi1 = phi1Rad/rad;

    N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
    T1 = tan(phi1Rad)*tan(phi1Rad);
    C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
    R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
    D = x/(N1*k0);

    *lat = phi1Rad - (N1*tan(phi1Rad)/R1)*
		(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
               +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)
		*D*D*D*D*D*D/720);
    *lat /= rad;

    *lon = (D-(1+2*T1+C1)*D*D*D/6 +
		(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
			*D*D*D*D*D/120)/cos(phi1Rad);
    *lon = mp->utm_center_lon + *lon / rad;
*/
    M = y / k0;
    mu = M/mp->mu1;

    phi1Rad = mu + mp->p1*sin(2*mu) + mp->p2*sin(4*mu) + mp->p3*sin(6*mu);

    sin_phi = sin(phi1Rad);
    tan_phi = tan(phi1Rad);
    cos_phi = cos(phi1Rad);
    N1 = a/sqrt(1 - mp->e_Squared*sin_phi*sin_phi);
    T1 = tan_phi*tan_phi;
    C1 = mp->e_PrimeSquared*cos_phi*cos_phi;
    R1 = a*(1 - mp->e_Squared)/pow(1 - mp->e_Squared*sin_phi*sin_phi, 1.5);
    D = x/(N1*k0);
    D2 = D*D;
    D3 = D2*D;
    D4 = D2*D2;
    D5 = D4*D;
    D6 = D4*D2;

    *lat = phi1Rad - (N1*tan_phi/R1)*
		(D2/2-(5+3*T1+10*C1-4*C1*C1-9*mp->e_PrimeSquared)*D4/24
               +(61+90*T1+298*C1+45*T1*T1-252*mp->e_PrimeSquared-3*C1*C1)
		*D6/720);
    *lat /= rad;

    *lon = (D-(1+2*T1+C1)*D3/6 +
		(5-2*C1+28*T1-3*C1*C1+8*mp->e_PrimeSquared+24*T1*T1)
			*D5/120)/cos_phi;
    *lon = mp->utm_center_lon + *lon / rad;
}

int
MapPlotGetCrosshairs(MapPlotWidget w, double **lat, double **lon)
{
	AxesPart *ax = &w->axes;
	int	i;
	double 	*x, *y;

	if(w == NULL || !XtIsRealized((Widget)w)) return(0);

	if(ax->num_cursors < 1) return(0);

	if( !(x = (double *)AxesMalloc((Widget)w,
		ax->num_cursors * sizeof(double))) ||
	    !(y = (double *)AxesMalloc((Widget)w,
		ax->num_cursors * sizeof(double))) ) return(0);

	for (i=0; i<ax->num_cursors; i++)
	{
		y[i] = ax->cursor[i].scaled_y;
		x[i] = ax->cursor[i].scaled_x;
	}
	*lat = y;
	*lon = x;
	return(ax->num_cursors);
}

void
MapPlotSetAppContext(MapPlotWidget w, XtAppContext app)
{
    w->map_plot.app_context = app;
}

static void
Motion(MapPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i=0, cursor_x, cursor_y, n1, n2, which;
	int ix1, ix2, iy1, iy2, imin, jmin, scroll_width;
	char label[200], label2[200], xlab[50], ylab[50], c;
	double x, y;
	Boolean	station, near, turn_off_utm = False;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(ax->vertical_scroll_position != LEFT && ax->vert_scroll &&
	    XtIsManaged(ax->vert_scroll))
	{
	    scroll_width = ax->vert_scroll->core.width;
	}
	else {
	    scroll_width = 0;
	}

	if(mp->projection == MAP_POLAR && mp->bar_width > 0 &&
		cursor_x > w->core.width - scroll_width - mp->bar_width &&
		cursor_y > ax->d.iy2 + mp->bar_vertical_margin &&
		cursor_y < ax->d.iy1 - mp->bar_vertical_margin)
	{
	    if(XtHasCallbacks((Widget)w, XtNselectBarCallback)
			!= XtCallbackHasNone)
	    {
		destroyInfoPopup(w, True);
		_AxesSetCursor((AxesWidget)w, "hand");
		mp->select_color_bar = True;
	    }
	    return;
	}
	mp->select_color_bar = False;

	ix1 = unscale_x(&ax->d, ax->x1[ax->zoom]);
	ix2 = unscale_x(&ax->d, ax->x2[ax->zoom]);
	iy1 = unscale_y(&ax->d, ax->y1[ax->zoom]);
	iy2 = unscale_y(&ax->d, ax->y2[ax->zoom]);
	if((cursor_x < ix1 && cursor_x < ix2) ||
	   (cursor_x > ix1 && cursor_x > ix2) ||
	   (cursor_y < iy1 && cursor_y < iy2) ||
	   (cursor_y > iy1 && cursor_y > iy2)) return;

	turn_off_utm = _AxesMotion((AxesWidget)w, event, params, num_params);

	if(mp->projection == MAP_POLAR && mp->display_polar_selection
		&& nearPolarSelection(w, cursor_x, cursor_y, 5,
			&mp->polar_select_side))
	{
	    destroyInfoPopup(w, True);
	    _AxesSetCursor((AxesWidget)w, "move");
	    mp->polar_select = True;
	}
	else if(nearStaSrc(w, cursor_x, cursor_y, 5))
	{
	    destroyInfoPopup(w, True);
	    _AxesSetCursor((AxesWidget)w, "hand");
	    mp->select_sta_src = True;
	    turn_off_utm = True;
	}
	else if(nearArcDel(w, cursor_x, cursor_y, 5))
	{
	    destroyInfoPopup(w, True);
	    _AxesSetCursor((AxesWidget)w, "move");
	    mp->select_arc_del = True;
	    turn_off_utm = True;
	}
	else if((which = WhichSymbol(w, cursor_x, cursor_y, 1, &imin, &jmin,
				10, &near)) >= 0 && near)
	{
	    _AxesSetCursor((AxesWidget)w, "hand");
	    turn_off_utm = True;
	    mp->select_symbol = True;
	    if(which != mp->motion_which || imin != mp->motion_imin
			|| jmin != mp->motion_jmin)
	    {
		unsigned long millisecs = 0;
		mp->motion_which = which;
		mp->motion_imin = imin;
		mp->motion_jmin = jmin;

		if(mp->app_context) {
		    XtAppAddTimeOut(mp->app_context, millisecs,
				(XtTimerCallbackProc)symbolInfo, (XtPointer)w);
		}
		else {
		    XtAddTimeOut(millisecs, (XtTimerCallbackProc)symbolInfo,
				(XtPointer)w);
		}
	    }
	}
	else {
	    destroyInfoPopup(w, True);
	    _AxesSetCursor((AxesWidget)w, "default");
	    mp->select_sta_src = False;
	    mp->select_arc_del = False;
	    mp->select_symbol = False;
	    mp->polar_select = False;
	}

	if(ax->cursor_info == NULL) return;

	x = scale_x(&ax->d, cursor_x);
	y = scale_y(&ax->d, cursor_y);
	if(mp->projection != MAP_UTM_NEAR) {
	    XYTransform((AxesWidget)w, x, y, xlab, ylab);
	}

	if(mp->projection != MAP_POLAR && mp->projection != MAP_UTM_NEAR)
	{
	    /* add '0's to make the strings the same length. */
	    n1 = (int)strlen(xlab);
	    while(xlab[n1-1] == ' ') xlab[--n1] = '\0';
	    n2 = (int)strlen(ylab);
	    while(ylab[n2-1] == ' ') ylab[--n2] = '\0';
	    if(n1 != n2 && n1 > 2 && n2 > 2) {
		if(n1 < n2) {
		    c = xlab[n1-1]; /* the last char, 'N', 'S', 'E' or 'W' */
		    if(c == 'N' || c == 'S' || c == 'E' || c == 'W') {
			while(n1 < n2) {
			    xlab[n1-1] = '0';
			    xlab[n1] = '\0';
			    n1++;
			}
			xlab[n1-1] = c;
			xlab[n1] = '\0';
		    }
		}
		else {
		    c = ylab[n2-1];
		    if(c == 'N' || c == 'S' || c == 'E' || c == 'W') {
			while(n2 < n1) {
			    ylab[n2-1] = '0';
			    ylab[n2] = '\0';
			    n2++;
			}
			ylab[n2-1] = c;
			ylab[n2] = '\0';
		    }
		}
	    }
	}
 	if(mp->projection != MAP_UTM_NEAR) {
	    snprintf(label, 200, "%s  %s", xlab, ylab);
	}
	else {
	    double lat, lon;
	    utmToLL(mp, x, y, &lon, &lat);
	    xlab[0] = '\0';
	    ylab[0] = '\0';
	    gdrawCVlon(lon, ax->a.nxdeci+2, xlab, 20);
	    gdrawCVlat(lat, ax->a.nydeci+2, ylab, 20);
	    if(xlab[0] != '\0') strcat(xlab, " ");
	    snprintf(label, 200, "%d%c %.0f  %.0f   %s  %s",
		mp->utm_cell_zone, mp->utm_cell_letter, x, y, xlab, ylab);
	}

	if(turn_off_utm) {
	    if(mp->utm_letter != '\0') {
		XDrawSegments(XtDisplay(w), XtWindow(w), ax->mag_invertGC,
			mp->utm_segs, 4);
	    }
	    mp->utm_letter = '\0';
	    mp->utm_zone = -1;
	}
	else if(mp->projection == MAP_UTM)
	{
	    char utm_label[20];
	    int n = (int)strlen(label);
	    highlightUTM(w, x, y, utm_label);
	    snprintf(label+n, sizeof(label)-n, "  %s", utm_label);
	}

	if((i = WhichStaSrc(w, cursor_x, cursor_y, &station)) >= 0)
	{
	    if(station)
	    {
		snprintf(label2, 200, "%s %.2f %.2f", mp->sta[i]->station.label,
			mp->sta[i]->station.lat, mp->sta[i]->station.lon);
	    }
	    else
	    {
		snprintf(label2, 200, "%s %.2f %.2f", mp->src[i]->source.label,
			mp->src[i]->source.lat, mp->src[i]->source.lon);
	    }
	    if(ax->cursor_info2) InfoSetText(ax->cursor_info2, label2);
	}
	else if(mp->nthemes > 0)
	{
	    int jmin_image, ix, iy;
	    double lon, lat;

	    unproject(w, x, y, &lon, &lat);

	    if(WhichThemeShape(w, cursor_x, cursor_y, lon, lat, &jmin, &imin,
				&jmin_image, &ix, &iy))
	    {
		MapCursorCallbackStruct m;

		m.shape_theme_id = -1;
		m.selected = False;
		m.shape_index = -1;
		m.theme = NULL;
		m.image_theme_id = -1;
		m.image_ix = -1;
		m.image_iy = -1;
		m.image_x = -1;
		m.image_y = -1;
		m.image_z = -1.;
		m.info2 = ax->cursor_info2;

		if(jmin >= 0) 
		{
		    m.shape_theme_id = mp->theme[jmin]->id;
		    m.selected = mp->theme[jmin]->td[imin].selected;
		    m.shape_index = imin;
		    m.theme = &mp->theme[jmin]->theme;
		}
		if(jmin_image >= 0)
		{
		    MapImage *mi = &mp->theme[jmin_image]->map_image;
		    m.image_theme_id = mp->theme[jmin_image]->id;
		    m.image_ix = ix;
		    m.image_iy = iy;
		    m.image_x = mi->m.x[ix];
		    m.image_y = mi->m.y[iy];
		    m.image_z = mi->m.z[ix+iy*mi->m.nx];
		    if(xlab[0] != '\0') {
			snprintf(label, 200, "%s  %s  %.3lf",
				xlab, ylab, m.image_z);
		    }
		}
		XtCallCallbacks((Widget)w, XtNcursorMotionCallback, &m);
	    }
	}

	InfoSetText(ax->cursor_info, label);
}

static void
highlightUTM(MapPlotWidget w, double lon, double lat, char *utm_label)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int zone = getUTMZone(lon, lat);
        char letter;
	double lon_min, lon_max, lat_min, lat_max, x, y, z;
	XSegment *segs;

	if((letter = getUTMLetter(lat)) != '\0') {
	    sprintf(utm_label, "%c %d", letter, zone);
	}
	else {
	    utm_label[0] = '\0';
	}

	if(letter == mp->utm_letter && zone == mp->utm_zone) return;

	if(mp->utm_letter != '\0') {
	   XDrawSegments(XtDisplay(w), XtWindow(w), ax->mag_invertGC,
			mp->utm_segs, 4);
	}
	mp->utm_letter = letter;
	mp->utm_zone = zone;

	if( !getUTMCell(letter, zone, &lon_min, &lon_max, &lat_min, &lat_max) )
		return;

	segs = mp->utm_segs;

	// this draws a rectangle inside of the utm rectangle. Assumes that
	// the pen width is two. Avoids overlapping pixels.
	project(w, lon_min, lat_min, &x, &y, &z);
	segs[0].x1 = unscale_x(&ax->d, x) + 2;
	segs[0].y1 = unscale_y(&ax->d, y);
	project(w, lon_min, lat_max, &x, &y, &z);
	segs[0].x2 = segs[0].x1;
	segs[0].y2 = unscale_y(&ax->d, y) + 1;

	segs[1].x1 = segs[0].x2 + 1;
	segs[1].y1 = segs[0].y2 + 1;
	project(w, lon_max, lat_max, &x, &y, &z);
	segs[1].x2 = unscale_x(&ax->d, x) - 2;
	segs[1].y2 = segs[1].y1;

	segs[2].x1 = segs[1].x2 + 1;
	segs[2].y1 = segs[1].y2 - 1;
	project(w, lon_max, lat_min, &x, &y, &z);
	segs[2].x2 = segs[2].x1;
	segs[2].y2 = unscale_y(&ax->d, y);

	segs[3].x1 = segs[2].x2 - 1;
	segs[3].y1 = segs[2].y2 - 1;
	segs[3].x2 = segs[0].x1 + 1;
	segs[3].y2 = segs[3].y1;

	XDrawSegments(XtDisplay(w), XtWindow(w), ax->mag_invertGC, segs, 4);
}

static Boolean
getUTMCell(char letter, int zone, double *lon_min, double *lon_max,
		double *lat_min, double *lat_max)
{
	// special cases
	if(letter == 'V' && zone == 31) {
	    *lon_min = 0; *lon_max = 3;
	    *lat_min = 56; *lat_max = 64;
	}
	else if(letter == 'V' && zone == 32) {
	    *lon_min = 3; *lon_max = 12;
	    *lat_min = 56; *lat_max = 64;
	}
	else if(letter == 'X' && zone == 31) {
	    *lon_min = 0; *lon_max = 9;
	    *lat_min = 72; *lat_max = 84;
	}
	else if(letter == 'X' && zone == 33) {
	    *lon_min = 9; *lon_max = 21;
	    *lat_min = 72; *lat_max = 84;
	}
	else if(letter == 'X' && zone == 35) {
	    *lon_min = 21; *lon_max = 33;
	    *lat_min = 72; *lat_max = 84;
	}
	else if(letter == 'X' && zone == 37) {
	    *lon_min = 33; *lon_max = 42;
	    *lat_min = 72; *lat_max = 84;
	}
	else if(letter != '\0') {
	    *lon_min = -180 + (zone-1)*6;
	    *lon_max = *lon_min + 6;
	    if(letter == 'X')      { *lat_min =  72; *lat_max =  84; }
	    else if(letter == 'W') { *lat_min =  64; *lat_max =  72; }
	    else if(letter == 'V') { *lat_min =  56; *lat_max =  64; }
	    else if(letter == 'U') { *lat_min =  48; *lat_max =  56; }
	    else if(letter == 'T') { *lat_min =  40; *lat_max =  48; }
	    else if(letter == 'S') { *lat_min =  32; *lat_max =  40; }
	    else if(letter == 'R') { *lat_min =  24; *lat_max =  32; }
	    else if(letter == 'Q') { *lat_min =  16; *lat_max =  24; }
	    else if(letter == 'P') { *lat_min =   8; *lat_max =  16; }
	    else if(letter == 'N') { *lat_min =   0; *lat_max =   8; }
	    else if(letter == 'M') { *lat_min =  -8; *lat_max =   0; }
	    else if(letter == 'L') { *lat_min = -16; *lat_max =  -8; }
	    else if(letter == 'K') { *lat_min = -24; *lat_max = -16; }
	    else if(letter == 'J') { *lat_min = -32; *lat_max = -24; }
	    else if(letter == 'H') { *lat_min = -40; *lat_max = -32; }
	    else if(letter == 'G') { *lat_min = -48; *lat_max = -40; }
	    else if(letter == 'F') { *lat_min = -56; *lat_max = -48; }
	    else if(letter == 'E') { *lat_min = -64; *lat_max = -56; }
	    else if(letter == 'D') { *lat_min = -72; *lat_max = -64; }
	    else if(letter == 'C') { *lat_min = -80; *lat_max = -72; }
	}
	else {
	    return False;
	}
	return True;
}

static int
getUTMZone(double x, double y)
{
	double lon = (x+180)-(int)((x+180.)/360.)*360-180; // -180 < l < 180
	double lat = y;

	if(lat > 56. && lat <= 64) {
	    if(lon > 0 && lon <= 3) return 31;
	    else if(lon >  3 && lon <= 12) return 32;
	}
	else if(lat > 72 && lat <= 84) {
	    if(lon > 0 && lon <= 9) {
		return 31;
	    }
	    else if(lon > 9 && lon <= 21) {
		return 33;
	    }
	    else if(lon > 21 && lon <= 33) {
		return 35;
	    }
	    else if(lon > 33 && lon <= 42) {
		return 37;
	    }
	}
	return (int)((lon + 180)/6) + 1;
}

static char
getUTMLetter(double lat)
{
        char letter = '\0';

	if(     84 >= lat && lat >=  72) letter = 'X';
	else if( 72 > lat && lat >=  64) letter = 'W';
	else if( 64 > lat && lat >=  56) letter = 'V';
        else if( 56 > lat && lat >=  48) letter = 'U';
        else if( 48 > lat && lat >=  40) letter = 'T';
        else if( 40 > lat && lat >=  32) letter = 'S';
        else if( 32 > lat && lat >=  24) letter = 'R';
        else if( 24 > lat && lat >=  16) letter = 'Q';
        else if( 16 > lat && lat >=   8) letter = 'P';
        else if(  8 > lat && lat >=   0) letter = 'N';
        else if(  0 > lat && lat >=  -8) letter = 'M';
        else if( -8 > lat && lat >= -16) letter = 'L';
        else if(-16 > lat && lat >= -24) letter = 'K';
        else if(-24 > lat && lat >= -32) letter = 'J';
        else if(-32 > lat && lat >= -40) letter = 'H';
        else if(-40 > lat && lat >= -48) letter = 'G';
        else if(-48 > lat && lat >= -56) letter = 'F';
        else if(-56 > lat && lat >= -64) letter = 'E';
        else if(-64 > lat && lat >= -72) letter = 'D';
        else if(-72 > lat && lat >= -80) letter = 'C';
//        else letter = 'Z';

	return letter;
}

static void
symbolInfo(XtPointer client_data, XtIntervalId id)
{
	Widget w = (Widget)client_data;
	MapPlotWidget widget = (MapPlotWidget)client_data;
	MapPlotPart *mp = (widget != NULL) ? &widget->map_plot : NULL;

	if(mp->motion_which >= 0)
	{
	    Position x, y;
	    int imin, jmin;
	    MapSymbolCallbackStruct m;

	    destroyInfoPopup(widget, False);

	    imin = mp->motion_imin;
	    jmin = mp->motion_jmin;
	    m.label[0] = '\0';
	    if(mp->motion_which == 0) {
		if(imin < 0 || imin >= mp->nsymgrp || jmin < 0
			|| jmin >= mp->symgrp[imin]->group.npts) return;
		m.reason = MAP_SYMBOL_GROUP_ENTER;
		m.selected = mp->symgrp[imin]->group.sym.selected;
		m.id = mp->symgrp[imin]->group.id;
		m.symbol.id = mp->symgrp[imin]->group.id;
		m.symbol.polar_coords = mp->symgrp[imin]->group.polar_coords;
		m.symbol.lon = mp->symgrp[imin]->group.lon[jmin];
		m.symbol.lat = mp->symgrp[imin]->group.lat[jmin];
		memcpy(&m.symbol.sym, &mp->symgrp[imin]->group.sym,
				sizeof(SymbolInfo));
	    }
	    else if(mp->motion_which == 1) {
		if(imin < 0 || imin >= mp->nsymbol) return;
		m.reason = MAP_SYMBOL_ENTER;
		m.selected = mp->symbol[imin].sym.selected;
		m.id = mp->symbol[imin].id;
		memcpy(&m.symbol, &mp->symbol[imin], sizeof(MapPlotSymbol));
	    }

	    XtCallCallbacks((Widget)w, XtNsymbolInfoCallback, (XtPointer)&m);

	    if(m.label[0] != '\0') {
		Pixel white = StringToPixel(w, (char *)"White");
		XmString xm =XmStringCreateLtoR(m.label,XmFONTLIST_DEFAULT_TAG);
		mp->info_popup = XtVaCreateWidget("infoShell",
				overrideShellWidgetClass,
				(Widget)widget, XmNborderWidth, 1, NULL);
		XtVaCreateManagedWidget("infoLabel", xmLabelWidgetClass,
				mp->info_popup, XmNlabelString, xm,
				XtNbackground, white, NULL);
		XmStringFree(xm);
		XtTranslateCoords(w, 0, 0, &x, &y);
		x += mp->motion_x + 5;
		y += mp->motion_y + 5;
		XtMoveWidget(mp->info_popup, x, y);
		XtManageChild(mp->info_popup);
	    }
	}
}

static void
destroyInfoPopup(MapPlotWidget w, Boolean reset)
{
	MapPlotPart *mp = &w->map_plot;

	if(mp->info_popup)
	{
	    XtUnmanageChild(mp->info_popup);
	    XtDestroyWidget(mp->info_popup);
	    mp->info_popup = NULL;
	}
	if(reset) {
	    mp->motion_which = -1;
	    mp->motion_imin = -1;
	    mp->motion_jmin = -1;
	}
}

static int
WhichThemeShape(MapPlotWidget w, int cursor_x, int cursor_y, double x, double y,
		int *jmin, int *imin, int *jmin_image, int *ix, int *iy)
{
	MapPlotPart *mp = &w->map_plot;
	int i, j;
	double d, dmin=1.e+30;

	*imin = *jmin = -1;
	*jmin_image = *ix = *iy = -1;
	for(j = 0; j < mp->nthemes; j++)
	    if(mp->theme[j]->display && mp->theme[j]->delta_on
		&& mp->theme[j]->cursor_info)
	{
	    if(mp->projection == MAP_POLAR && !mp->theme[j]->theme.polar_coords)
	    {
		continue;
	    }
	    if(mp->theme[j]->theme_type == THEME_SHAPE)
	    {
		if((i = WhichShape(w, mp->theme[j],cursor_x,cursor_y, &d)) >= 0)
		{
		    if(dmin > 1.e+20) { /* we don't have the distance to a */
			*imin = i;	/* point or symbol yet */
			*jmin = j;
			if(d > 0.) dmin = d;
		    }
		    /* We already have the distance to a point or symbol.
		     * Compare this distance to it. */
		    else if(d > 0. && d < dmin) {
			dmin = d;
			*imin = i;
			*jmin = j;
		    }
		}
	    }
	    else if(mp->theme[j]->theme_type == THEME_IMAGE)
	    {
		MapImage *mi = &mp->theme[j]->map_image;

		if(mi->m.nx > 0 && mi->m.ny > 0)
		{
#ifdef HAVE_GSL
		    *jmin_image = j;
		    *ix = gsl_interp_bsearch(mi->m.x, x, 0, mi->m.nx-1);
		    if(*ix < mi->m.nx-1) {
			if(mi->m.x[*ix+1] - x < x - mi->m.x[*ix]) (*ix)++;
		    }

		    *iy = gsl_interp_bsearch(mi->m.y, y, 0, mi->m.ny-1);
		    if(*iy < mi->m.ny-1) {
			if(mi->m.y[*iy+1] - y < y - mi->m.y[*iy]) (*iy)++;
		    }
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
return 0;
#endif
		}
	    }
	}
	return ((*imin >= 0 && *jmin >= 0) || (*ix >= 0 && *iy >= 0)) ? 1 : 0;
}

static void
Btn1Down(MapPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
    MapPlotPart	*mp = &w->map_plot;
    AxesPart	*ax = &w->axes;
    int		cursor_x, cursor_y;
    double	x, y;
    SymbolInfo	*sym;

    destroyInfoPopup(w, True);

    if(ax->cursor_mode == AXES_SELECT_CURSOR)
    {
	int i, j;
	_AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	ax->moving_cursor = True;
	if(ax->m >= 0) {
	    for(i = 0; i < mp->ndel; i++) {
		if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
		    mp->del[i]->sta_id == ax->m) break;
	    }
	    if(i < mp->ndel) {
		mp->del[i]->delta.line.display = MAP_LOCKED_OFF;
	    }
	    for(j = 0; j < mp->narc; j++) {
		if(mp->arc[j]->type == MAP_CURSOR_MEASURE &&
		    mp->arc[j]->sta_id == ax->m) break;
	    }
	    if(j < mp->narc) {
		mp->arc[j]->arc.line.display = MAP_LOCKED_OFF;
	    }
	    if(i < mp->ndel || j < mp->narc) {
		RedisplayOverlays(w, mp->pixmap);
		for(i = 0; i < ax->num_cursors; i++) {
		    XDrawSegments(XtDisplay(w),mp->pixmap,ax->invert_clipGC,
				ax->cursor[i].segs, ax->cursor[i].nsegs);
		}
	    }
	}
	return;
    }
    if(mp->select_color_bar) {
	XtCallCallbacks((Widget)w, XtNselectBarCallback, NULL);
	return;
    }
    cursor_x = ((XButtonEvent *)event)->x;
    cursor_y = ((XButtonEvent *)event)->y;
    x = scale_x(&ax->d, cursor_x);
    y = scale_y(&ax->d, cursor_y);

    mp->moving_x0 = cursor_x;
    mp->moving_y0 = cursor_y;
    mp->selecting_utm = False;

/*
    if(mp->moving_index == -1)
    {
	_AxesMove((AxesWidget)w, event, params, num_params);
	ax->moving_cursor = True;
	mp->moving_circle = (*num_params > 0 && !strcmp(params[0], "circle"))
				? False : True;
	return;
    }
*/
    if(mp->polar_select)
    {
	if(mp->polar_line) {
	    if(*num_params > 0 && !strcmp(params[0], "move")) {
		mp->size_polar_select = True;
	    }
	    else {
		mp->moving_polar_select = True;
	    }
	}
    }
    else if(mp->select_symbol)
    {	
	MapSymbolCallbackStruct m;
	int imin[10], jmin[10], which;
	Boolean near;

	which = WhichSymbol(w, cursor_x, cursor_y, 10, imin, jmin, 5, &near);
	if(which == 0) {
		mp->symgrp[imin[0]]->group.sym.selected =
			!mp->symgrp[imin[0]]->group.sym.selected;
		MapPlotUpdate(w);
		m.reason = MAP_SYMBOL_GROUP_SELECT;
		m.selected = mp->symgrp[imin[0]]->group.sym.selected;
		m.id = mp->symgrp[imin[0]]->group.id;
		m.symbol.id = mp->symgrp[imin[0]]->group.id;
		m.symbol.polar_coords = mp->symgrp[imin[0]]->group.polar_coords;
		m.symbol.lon = mp->symgrp[imin[0]]->group.lon[jmin[0]];
		m.symbol.lat = mp->symgrp[imin[0]]->group.lat[jmin[0]];
		memcpy(&m.symbol.sym, &mp->symgrp[imin[0]]->group.sym,
				sizeof(SymbolInfo));
		XtCallCallbacks((Widget)w, XtNsymbolSelectCallback, &m);
	}
	else if(which > 0) {
	    for(int i = 0; i < which; i++) {
		mp->symbol[imin[i]].sym.selected =
			!mp->symbol[imin[i]].sym.selected;
	    }
	    MapPlotUpdate(w);
	    if(*num_params > 0 && !strcmp(params[0], "move")) {
		    m.reason = MAP_SYMBOL_CTRL_SELECT;
	    }
	    else {
		    m.reason = MAP_SYMBOL_SELECT;
	    }
	    for(int i = 0; i < which; i++) {
		m.selected = mp->symbol[imin[i]].sym.selected;
		m.id = mp->symbol[imin[i]].id;
		memcpy(&m.symbol, &mp->symbol[imin[i]], sizeof(MapPlotSymbol));
		XtCallCallbacks((Widget)w, XtNsymbolSelectCallback, &m);
	    }
	}
    }
    else if(*num_params > 0 && !strcmp(params[0], "move"))
    {
	if((mp->moving_index = WhichStaSrc(w,cursor_x, cursor_y,
					&mp->moving_station)) == -1) return;

	if(( mp->moving_station && !mp->movable_stations) ||
	   (!mp->moving_station && !mp->movable_sources))
	{
	    mp->moving_index = -1;
	}
	else
	{
	    sym = (mp->moving_station) ?
			&mp->sta[mp->moving_index]->station.sym :
			&mp->src[mp->moving_index]->source.sym;
	    sym->display = MAP_OFF;
	    RedisplayOverlays(w, mp->pixmap);
	}
    }
    else if(mp->projection == MAP_UTM && selectUTMCell(w)) {
	    mp->selecting_utm = True;
    }
    else if(mp->select_sta_src)
    {
	if((mp->moving_index = WhichStaSrc(w,cursor_x, cursor_y,
					&mp->moving_station)) == -1) return;
	if(mp->moving_station) {
	    DoSelectStation(w, mp->sta[mp->moving_index], True, True);
	}
	else {
	    DoSelectSource(w, mp->src[mp->moving_index], True, True);
	}
	mp->moving_index = -1;
    }
    else if(mp->select_arc_del)
    {
	doMeasure(w, event);
    }
    else if(mp->nthemes > 0)
    {
	int imin, jmin, jmin_image, ix, iy;

	if(WhichThemeShape(w, cursor_x, cursor_y, x, y, &jmin, &imin,
			&jmin_image, &ix, &iy))
	{
	    MapCursorCallbackStruct m;

	    m.shape_theme_id = -1;
	    m.selected = False;
	    m.shape_index = -1;
	    m.theme = NULL;
	    m.image_theme_id = -1;
	    m.image_ix = -1;
	    m.image_iy = -1;
	    m.image_x = -1;
	    m.image_y = -1;
	    m.image_z = -1.;
	    m.info2 = ax->cursor_info2;

	    if(jmin >= 0) 
	    {
		mp->theme[jmin]->td[imin].selected =
                        !mp->theme[jmin]->td[imin].selected;
		MapPlotUpdate(w);
		m.shape_theme_id = mp->theme[jmin]->id;
		m.selected = mp->theme[jmin]->td[imin].selected;
		m.shape_index = imin;
		m.theme = &mp->theme[jmin]->theme;
	    }
	    if(jmin_image >= 0)
	    {
		MapImage *mi = &mp->theme[jmin_image]->map_image;
		m.image_theme_id = mp->theme[jmin_image]->id;
		m.image_ix = ix;
		m.image_iy = iy;
		m.image_x = mi->m.x[ix];
		m.image_y = mi->m.y[iy];
		m.image_z = mi->m.z[ix+iy*mi->m.nx];
	    }
	    XtCallCallbacks((Widget)w, XtNshapeSelectCallback, &m);
	}
    }
    ax->last_cursor_x = cursor_x;
    ax->last_cursor_y = cursor_y;
}

static int
WhichShape(MapPlotWidget w, MapTheme *t, int cursor_x, int cursor_y,
		double *dist_min)
{
	int i, j;

	*dist_min = -1;
	if(t->theme.shape_type == SHPT_POLYGON)
	{
	    for(i = 0; i < t->theme.nshapes; i++)
	    {
		if(t->td[i].display) {
		    DSegment *seg = t->td[i].fill_segs.segs;
		    for(j = 0; j < t->td[i].fill_segs.nsegs; j++) {
			if(cursor_y == seg[j].y1 && cursor_x >= seg[j].x1 && 
			    cursor_x <= seg[j].x2)
			{
			    return i;
			}
		    }
		}
	    }
	}
	else if(t->theme.shape_type == SHPT_POINT)
	{
	    int imin = -1;
	    double d, dmin=1.e+30, xdif, ydif;

	    for(i = 0; i < t->theme.nshapes; i++)
		if(t->td[i].display && t->td[i].center_x > 0)
	    {
		xdif = t->td[i].center_x - cursor_x;
		ydif = t->td[i].center_y - cursor_y;
		d = xdif*xdif + ydif*ydif;
		if(d < dmin) {
		    dmin = d;
		    imin = i;
		}
	    }
	    if(imin >= 0) *dist_min = dmin;
	    return imin;
	}
	else if(t->theme.shape_type == SHPT_ARC)
	{
	    int imin = -1;
	    double d, dmin=1.e+30;

	    for(i = 0; i < t->theme.nshapes; i++)
	    {
		if(t->td[i].display) {
		    DSegment *seg = t->td[i].bndy_segs.segs;
		    for(j = 0; j < t->td[i].bndy_segs.nsegs; j++) {
			d = AxesClass::pointToLine(
					(float)seg[j].x1, (float)seg[j].y1,
					(float)seg[j].x2, (float)seg[j].y2,
					(float)cursor_x, (float)cursor_y);
			if(d < dmin) {
			    dmin = d;
			    imin = i;
			}
		    }
		}
	    }
	    if(imin >= 0) *dist_min = dmin;
	    return imin;
	}
	return -1;
}

static void
Btn1Motion(MapPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, cursor_x, cursor_y, delx, dely, half_symbol_size;
	MapCircleCallbackStruct callback;
	MapStation	*sta;
	MapSource	*src;
	SymbolInfo	*sym;
	double		*lat, *lon;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(ax->moving_cursor)
	{
	    MapDelta *d = NULL;
	    MapArc *a= NULL;
	    double cursor_lon, cursor_lat;

	    for(i = 0; i < ax->num_cursors; i++) {
		XDrawSegments(XtDisplay(w),mp->pixmap,ax->invert_clipGC,
			ax->cursor[i].segs, ax->cursor[i].nsegs);
	    }
	    _AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	    for(i = 0; i < ax->num_cursors; i++) {
		XDrawSegments(XtDisplay(w),mp->pixmap,ax->invert_clipGC,
			ax->cursor[i].segs, ax->cursor[i].nsegs);
	    }
	    cursor_x = ((XButtonEvent *)event)->x;
	    cursor_y = ((XButtonEvent *)event)->y;
	    if(ax->moving_cursor && mp->ncircles > 0 && (ax->num_cursors == 0 ||
		(abs(cursor_x-mp->moving_x0) <= 2 &&
			abs(cursor_y-mp->moving_y0) <= 2)) &&
		(j = WhichCircle(w, cursor_x, cursor_y)) >= 0 &&
		!mp->circle[j].selected)
	    {
		if(mp->moving_circle)
		{
		    mp->primary_circle = j;
		    for(i = 0; i < mp->ncircles; i++)
			if(mp->circle[i].selected && i != j)
		    {
			SelectCircle(w, i);
		    }
		}
		else if(mp->primary_circle < 0)
		{
		    mp->primary_circle = j;
		    mp->moving_circle = True;
		}
		SelectCircle(w, j);
		callback.pt = mp->circle[j].pt;
		callback.primary = mp->moving_circle;
		XtCallCallbacks((Widget)w, XtNselectCircleCallback, &callback);
	    }
	    if(ax->m < 0) return;

	    for(i = 0; i < mp->ndel; i++) {
		if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
			mp->del[i]->sta_id == ax->m)
		{
		    d = mp->del[i];
		    break;
		}
	    }
	    for(j = 0; j < mp->narc; j++) {
		if(mp->arc[j]->type == MAP_CURSOR_MEASURE &&
			mp->arc[j]->sta_id == ax->m)
		{
		    a = mp->arc[j];
		    break;
		}
	    }
	    if( !a && !d ) return;

	    unproject(w, scale_x(&ax->d, ax->cursor[ax->m].x),
		scale_y(&ax->d, ax->cursor[ax->m].y), &cursor_lon, &cursor_lat);

	    RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	    if(d != NULL)
	    {
		d->delta.lat = cursor_lat;
		d->delta.lon = cursor_lon;

		MakeDelta(w, d);
		DrawDelta(w, &ax->d, d);
		RedisplayLine(w, ax->pixmap, &d->delta.line, &d->s);
	    }
	    if(a != NULL)
	    {
		a->arc.lat = cursor_lat;
		a->arc.lon = cursor_lon;
		MakeArc(w, a);
		DrawArc(w, &ax->d, a);
		RedisplayLine(w, ax->pixmap, &a->arc.line, &a->s);
	    }
	    RedisplayPixmap(w, ax->pixmap, XtWindow(w));

	    return;
	}

	if(mp->moving_index >= 0)
	{
	    if(mp->moving_station)
	    {
		sta = mp->sta[mp->moving_index];
		sym = &sta->station.sym;
		lat = &sta->station.lat;
		lon = &sta->station.lon;
	    }
	    else
	    {
		src = mp->src[mp->moving_index];
		sym = &src->source.sym;
		lat = &src->source.lat;
		lon = &src->source.lon;
	    }

	    half_symbol_size = (sym->size + 1)/2;
	    if(cursor_x - half_symbol_size <  ax->clipx1) 
			cursor_x = ax->clipx1 + half_symbol_size;
	    if(cursor_x + half_symbol_size >  ax->clipx2) 
			cursor_x = ax->clipx2 - half_symbol_size;
	    if(cursor_y - half_symbol_size <  ax->clipy1) 
			cursor_y = ax->clipy1 + half_symbol_size;
	    if(cursor_y + half_symbol_size >  ax->clipy2) 
			cursor_y = ax->clipy2 - half_symbol_size;
	    /*
	     * get the distance the cursor has moved
	     */	
	    delx = cursor_x - ax->last_cursor_x;
	    dely = cursor_y - ax->last_cursor_y;

	    mp->moving_x0 += delx;
	    mp->moving_y0 += dely;
	    unproject(w, scale_x(&ax->d, mp->moving_x0),
			scale_y(&ax->d, mp->moving_y0), lon, lat);

	    RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	    RedisplaySymbol(w, ax->pixmap, *lon, *lat, sym);
	    RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	    _AxesRedisplayXor((AxesWidget)w);
	}
	else if(mp->select_arc_del)
	{
	    doMeasure(w, event);
	}
	else if(mp->moving_polar_select)
	{
	    int ix, iy;
	    double x, y;

	    /*
	     * get the distance the cursor has moved
	     */	
	    if( mp->polar_line->s.nsegs > 0) {
		delx = cursor_x - ax->last_cursor_x;
		dely = cursor_y - ax->last_cursor_y;
		ix = mp->polar_line->s.segs[0].x1 + delx;
		iy = mp->polar_line->s.segs[0].y1 + dely;
		x = scale_x(&ax->d, ix);
		y = scale_y(&ax->d, iy);

		mp->polar_azimuth = atan2(x, y)/DEG_TO_RADIANS;
		if(mp->polar_azimuth < 0.) mp->polar_azimuth += 360.;
		mp->polar_radius = sqrt(x*x + y*y);

		drawPolarSelect(w, mp->polar_line->line.lon,
				mp->polar_line->line.lat);
		Free(mp->polar_line->s.segs);
		mp->polar_line->s.size_segs = 0;
		mp->polar_line->s.nsegs = 0;
		ax->d.s = &mp->polar_line->s;
		DrawLonLat(w, &ax->d, mp->polar_line->line.npts,
			mp->polar_line->line.lon, mp->polar_line->line.lat);
		findPolarSelectSegs(w, mp->polar_line->line.lon,
				mp->polar_line->line.lat);
		RedisplayPixmap(w, mp->pixmap, ax->pixmap);
		RedisplayOverlays(w, ax->pixmap);
		RedisplayPixmap(w, ax->pixmap, XtWindow(w));
		_AxesRedisplayXor((AxesWidget)w);

		doPolarSelectCallback(w);
	    }
	}
	else if(mp->size_polar_select)
	{
	    double x, y, az1, az2, r1, r2;
	    /*
	     * get the distance the cursor has moved
	     */	
	    x = scale_x(&ax->d, cursor_x);
	    y = scale_y(&ax->d, cursor_y);

	    if(mp->polar_select_side == 0) {
		az1 = atan2(x, y)/DEG_TO_RADIANS;
		if(az1 < 0.) az1 += 360.;
		az2 = mp->polar_azimuth + mp->polar_del_azimuth;
		mp->polar_del_azimuth = az2 - az1;
		mp->polar_azimuth = az1;
	    }
	    else if(mp->polar_select_side == 2) {
		az2 = atan2(x, y)/DEG_TO_RADIANS;
		if(az2 < 0.) az2 += 360.;
		mp->polar_del_azimuth = az2 - mp->polar_azimuth;
	    }
	    else if(mp->polar_select_side == 1) {
		r2 = sqrt(x*x + y*y);
		mp->polar_del_radius = r2 - mp->polar_radius;
	    }
	    else if(mp->polar_select_side == 3) {
		r1 = sqrt(x*x + y*y);
		r2 = mp->polar_radius + mp->polar_del_radius;
		mp->polar_del_radius = r2 - r1;
		mp->polar_radius = r1;
	    }

	    drawPolarSelect(w, mp->polar_line->line.lon,
				mp->polar_line->line.lat);
	    Free(mp->polar_line->s.segs);
	    mp->polar_line->s.size_segs = 0;
	    mp->polar_line->s.nsegs = 0;
	    ax->d.s = &mp->polar_line->s;
	    DrawLonLat(w, &ax->d, mp->polar_line->line.npts,
			mp->polar_line->line.lon, mp->polar_line->line.lat);
	    findPolarSelectSegs(w, mp->polar_line->line.lon,
				mp->polar_line->line.lat);
	    RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	    RedisplayOverlays(w, ax->pixmap);
	    RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	    _AxesRedisplayXor((AxesWidget)w);

	    doPolarSelectCallback(w);
	}
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;
}

static void
Btn1Up(MapPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, cursor_x, cursor_y;
	MapDelta *d=NULL;
	MapArc *a=NULL;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(mp->moving_index >= 0)
	{
	    if(mp->moving_station)
	    {
		mp->sta[mp->moving_index]->station.sym.display = MAP_ON;
		MapPlotChangeStation(w, &mp->sta[mp->moving_index]->station,
				True);
	    }
	    else
	    {
		mp->src[mp->moving_index]->source.sym.display = MAP_ON;
		MapPlotChangeSource(w, &mp->src[mp->moving_index]->source,True);
	    }
	    mp->moving_index = -1;
	}
	else if(mp->select_arc_del)
	{
	    doMeasure(w, event);
	}
	else if(mp->projection == MAP_UTM && mp->selecting_utm &&
		mp->utm_select_letter == mp->utm_letter &&
		mp->utm_select_zone == mp->utm_zone)
	{
	    utmMode(w);
	    ax->moving_cursor = False;
	    ax->last_cursor_x = cursor_x;
	    ax->last_cursor_y = cursor_y;
	    mp->selecting_utm = False;
	    return;
	}
	if(ax->moving_cursor) {
	    for(i = 0; i < mp->ndel; i++) {
		if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
			mp->del[i]->sta_id == ax->m) {
		    d = mp->del[i];
		    break;
		}
	    }
	    for(i = 0; i < mp->narc; i++) {
		if(mp->arc[i]->type == MAP_CURSOR_MEASURE &&
			mp->arc[i]->sta_id == ax->m) {
		    a = mp->arc[i];
		    break;
		}
	    }
	    if(d != NULL) d->delta.line.display = MAP_LOCKED_ON;
	    if(a != NULL) a->arc.line.display = MAP_LOCKED_ON;
	}
	_AxesMove((AxesWidget)w, event, (const char **)params, num_params);
	MapPlotUpdate(w);

	ax->moving_cursor = False;
	mp->moving_polar_select = False;
	mp->size_polar_select = False;
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;
}

static void
CrosshairCB(Widget widget, XtPointer client, XtPointer callback_data)
{
	MapPlotWidget w = (MapPlotWidget)widget;
	AxesCursorCallbackStruct *c = (AxesCursorCallbackStruct *)callback_data;
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	LineInfo line_info_init = LINE_INFO_INIT;
	MapPlotDelta delta;
	MapDelta *d;
	MapPlotArc arc;
	MapArc *a;
	int i;
	double lat, lon;

	if(c->type != AXES_CROSSHAIR) return;

	if(c->reason == AXES_CROSSHAIR_DELETE) {
	    for(i = 0; i < mp->ndel; i++) {
		if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
		    mp->del[i]->sta_id == c->index) break;
	    }
	    if(i < mp->ndel) {
		DeleteDelta(w, i);
	    }
	    for(i = 0; i < mp->narc; i++) {
		if(mp->arc[i]->type == MAP_CURSOR_MEASURE &&
		    mp->arc[i]->sta_id == c->index) break;
	    }
	    if(i < mp->narc) {
		DeleteArc(w, i);
	    }
	    return;
	}
	else if(c->reason != AXES_CROSSHAIR_POSITION)
	{
	    return;
	}

	if( mp->projection == MAP_POLAR) return;

	unproject(w, c->scaled_x, c->scaled_y, &lon, &lat);

	for(i = 0; i < mp->ndel; i++) {
	    if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
		mp->del[i]->sta_id == c->index) break;
	}
	if(i < mp->ndel) {
	    MapPlotUpdate(w);
	    return;
	}

	delta.label = NULL;
	delta.lat = lat;
	delta.lon = lon;
	if(mp->projection == MAP_UTM_NEAR) {
	    delta.del = .1*fabs(ax->x2[ax->zoom] - ax->x1[ax->zoom]);
	}
	else {
	    double x1, x2, y, dif;
	    unproject(w, ax->x1[ax->zoom], ax->y1[ax->zoom], &x1, &y);
	    unproject(w, ax->x2[ax->zoom], ax->y1[ax->zoom], &x2, &y);
	    x1 = (x1+180)-(int)((x1+180)/360)*360-180;
	    x2 = (x2+180)-(int)((x2+180)/360)*360-180;
	    dif = fabs(x2 - x1);
	    if(dif > 0. && dif < 20.) {
		delta.del = .1*dif;
	    }
	    else {
		delta.del = 10.;
	    }
	}
	memcpy(&delta.line, &line_info_init, sizeof(LineInfo));
	delta.line.display = MAP_LOCKED_ON;
	delta.line.fg = ax->fg;
	delta.line.bg = w->core.background_pixel;

	MapPlotAddDelta(w, &delta, False);
	d = mp->del[mp->ndel-1];
	d->type = MAP_CURSOR_MEASURE;
	d->sta_id = c->index;
	d->src_id = 0;

	arc.label = NULL;
	arc.lat = lat;
	arc.lon = lon;
	arc.del = 180.;
	arc.az = 65.;
	memcpy(&arc.line, &line_info_init, sizeof(LineInfo));
	arc.line.display = MAP_LOCKED_ON;
	arc.line.fg = ax->fg;
	arc.line.bg = w->core.background_pixel;
	MapPlotAddArc(w, &arc, False);
	a = mp->arc[mp->narc-1];
	a->type = MAP_CURSOR_MEASURE;
	a->sta_id = c->index;
	a->src_id = 0;

	MapPlotUpdate(w);
}

static void
Measure(MapPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i0, sta_id, src_id, cursor_x, cursor_y, ic, n;
	int		delta_id, arc_id;
	double		cursor_lat, cursor_lon, del, az, baz;
	MapPlotArc	arc;
	MapPlotDelta	delta;
	MapMeasure	m;
	LineInfo line_info_init = LINE_INFO_INIT;
	static MapDelta	*d = NULL;
	static MapArc	*a = NULL;
	static double	lat, lon;
	static int	type = -1;

	if(!mp->measure) return;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	unproject(w, scale_x(&ax->d, cursor_x),
		scale_y(&ax->d, cursor_y), &cursor_lon, &cursor_lat);

	if(event->type == ButtonPress)
	{
	    a = NULL;
	    d = NULL;
	    if((i0 = WhichMeasure(w, True, False, cursor_x, cursor_y,
				&type, &ic, &delta_id, &arc_id)) < 0)
	    {
		return;
	    }
	    if(type == MAP_STATION || type == MAP_SOURCE || ic >= 0)
	    {
		if(type == MAP_STATION)
		{
		    sta_id = mp->sta[i0]->station.id;
		    src_id = 0;
		    lat = mp->sta[i0]->station.lat;
		    lon = mp->sta[i0]->station.lon;
		    m.label = mp->sta[i0]->station.label;
		}
		else if(type == MAP_SOURCE)
		{
		    src_id = mp->src[i0]->source.id;
		    sta_id = 0;
		    lat = mp->src[i0]->source.lat;
		    lon = mp->src[i0]->source.lon;
		    m.label = mp->src[i0]->source.label;
		}
		else
		{
		    src_id = 0;
		    sta_id = 0;
		    unproject(w, ax->cursor[ic].scaled_x,
				ax->cursor[ic].scaled_y, &lon, &lat);
		    m.label = NULL;
		}

		delta.label = m.label;
		delta.lat = lat;
		delta.lon = lon;
		if(mp->projection == MAP_UTM_NEAR) {
		    utmDelta(w, lon, lat, cursor_x, cursor_y, &del);
		}
		else {
		    deltaz(lat, lon, cursor_lat, cursor_lon, &del, &az, &baz);
		}
		delta.del = del;
		memcpy(&delta.line, &line_info_init, sizeof(LineInfo));
		delta.line.display = MAP_LOCKED_ON;
		delta.line.fg = ax->fg;
		delta.line.bg = w->core.background_pixel;
		MapPlotAddDelta(w, &delta, False);
		d = mp->del[mp->ndel-1];
		d->type = MAP_MEASURE;
		d->sta_id = sta_id;
		d->src_id = src_id;
		n = mp->n_measure_dels;
		mp->measure_del = (MapDelta **)AxesRealloc((Widget)w,
				mp->measure_del, (n+1)*sizeof(MapDelta *));
		mp->measure_del[n] = d;
		mp->n_measure_dels++;
		m.id = d->delta.id;
		m.type = MAP_DELTA;
		m.lat = lat;
		m.lon = lon;
		m.delta = del;
		m.az = -999.;
		XtCallCallbacks((Widget)w, XtNmapMeasureCallback, &m);

		arc.label = m.label;
		arc.lat = lat;
		arc.lon = lon;
		arc.del = 180.;
		deltaz(lat, lon, cursor_lat, cursor_lon, &del, &az, &baz);
		arc.az = az;
		memcpy(&arc.line, &line_info_init, sizeof(LineInfo));
		arc.line.display = MAP_LOCKED_ON;
		arc.line.fg = ax->fg;
		arc.line.bg = w->core.background_pixel;
		MapPlotAddArc(w, &arc, False);
		a = mp->arc[mp->narc-1];
		a->type = MAP_MEASURE;
		a->sta_id = sta_id;
		a->src_id = src_id;
		n = mp->n_measure_arcs;
		mp->measure_arc = (MapArc **)AxesRealloc((Widget)w,
				mp->measure_arc, (n+1)*sizeof(MapArc *));
		mp->measure_arc[n] = a;
		mp->n_measure_arcs++;
		m.id = a->arc.id;
		m.type = MAP_ARC;
		m.lat = lat;
		m.lon = lon;
		m.delta = arc.del;
		m.az = arc.az;
		XtCallCallbacks((Widget)w, XtNmapMeasureCallback, &m);
		MapPlotUpdate(w);
	    }
	}
}

static void
doMeasure(MapPlotWidget w, XEvent *event)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, i0, cursor_x, cursor_y, ic;
	int		delta_id, arc_id;
	double		cursor_lat, cursor_lon, del, az, baz;
	MapMeasure	m;
	static MapDelta	*d = NULL;
	static MapArc	*a = NULL;
	static Boolean	measure = False;
	static int	type = -1;

	if(!mp->measure) { // check for a crosshair arc/del
	    for(i=0; i<mp->narc && mp->arc[i]->type != MAP_CURSOR_MEASURE; i++);
	    for(j=0; j<mp->ndel && mp->del[j]->type != MAP_CURSOR_MEASURE; j++);
	    if(i == mp->narc && j == mp->ndel) return;
	}

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	unproject(w, scale_x(&ax->d, cursor_x),
		scale_y(&ax->d, cursor_y), &cursor_lon, &cursor_lat);

	if(event->type == ButtonPress)
	{
	    MapPlotUpdate(w);
	    a = NULL;
	    d = NULL;
	    if((i0 = WhichMeasure(w, False, True, cursor_x, cursor_y,
				&type, &ic, &delta_id, &arc_id)) < 0)
	    {
		measure = False;
		return;
	    }
	    if(arc_id >= 0 && delta_id >= 0)
	    {
		a = mp->arc[arc_id];
		a->arc.line.display = MAP_LOCKED_OFF;
		d = mp->del[delta_id];
		d->delta.line.display = MAP_LOCKED_OFF;
		RedisplayOverlays(w, mp->pixmap);
		for(i = 0; i < ax->num_cursors; i++)
		{
		    XDrawSegments(XtDisplay(w), mp->pixmap, ax->invert_clipGC,
				ax->cursor[i].segs, ax->cursor[i].nsegs);
		}
		measure = True;
	    }
	    else if(type == MAP_ARC)
	    {
		a = mp->arc[i0];
		a->arc.line.display = MAP_LOCKED_OFF;
		RedisplayOverlays(w, mp->pixmap);
		for(i = 0; i < ax->num_cursors; i++)
		{
		    XDrawSegments(XtDisplay(w), mp->pixmap, ax->invert_clipGC,
				ax->cursor[i].segs, ax->cursor[i].nsegs);
		}
		measure = True;
	    }
	    else if(type == MAP_DELTA)
	    {
		d = mp->del[i0];
		d->delta.line.display = MAP_LOCKED_OFF;
		RedisplayOverlays(w, mp->pixmap);
		for(i = 0; i < ax->num_cursors; i++)
		{
		    XDrawSegments(XtDisplay(w), mp->pixmap, ax->invert_clipGC,
				ax->cursor[i].segs, ax->cursor[i].nsegs);
		}
		measure = True;
	    }
	    else
	    {
		measure = False;
	    }
	}
	else if(event->type == MotionNotify && measure)
	{
	    RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	    if(d != NULL)
	    {
		if(mp->projection == MAP_UTM_NEAR) {
		    utmDelta(w, d->delta.lon, d->delta.lat, cursor_x,
				cursor_y, &d->delta.del);
		}
		else {
		    deltaz(d->delta.lat, d->delta.lon, cursor_lat, cursor_lon,
			&d->delta.del, &az, &baz);
		}
		MakeDelta(w, d);
		DrawDelta(w, &ax->d, d);
		RedisplayLine(w, ax->pixmap, &d->delta.line, &d->s);
		m.id = d->delta.id;
		m.label = d->delta.label;
		m.type = MAP_DELTA;
		m.lat = d->delta.lat;
		m.lon = d->delta.lon;
		m.delta = d->delta.del;
		m.az = -999.;
		XtCallCallbacks((Widget)w, XtNmapMeasureCallback, &m);
	    }
	    if(a != NULL)
	    {
		if(mp->projection == MAP_UTM_NEAR) {
		    utmAz(w, a->arc.lon, a->arc.lat, cursor_x, cursor_y,
				&a->arc.az);
		}
		else {
		    deltaz(a->arc.lat, a->arc.lon, cursor_lat, cursor_lon, &del,
			&a->arc.az, &baz);
		}
		MakeArc(w, a);
		DrawArc(w, &ax->d, a);
		RedisplayLine(w, ax->pixmap, &a->arc.line, &a->s);
		m.id = a->arc.id;
		m.label = a->arc.label;
		m.type = MAP_ARC;
		m.lat = a->arc.lat;
		m.lon = a->arc.lon;
		m.delta = a->arc.del;
		m.az = a->arc.az;
		XtCallCallbacks((Widget)w, XtNmapMeasureCallback, &m);
	    }
	    RedisplayPixmap(w, ax->pixmap, XtWindow(w));

	    if(	(d && d->type == MAP_CURSOR_MEASURE) ||
		(a && a->type == MAP_CURSOR_MEASURE) )
	    {
		AxesCursorCallbackStruct p;
		ic = (d) ? d->sta_id : a->sta_id;
		if(ic < 0 || ic >= ax->num_cursors) return;
		p.index	= ic;
		p.type	= ax->cursor[ic].type;
		p.x	= ax->cursor[ic].x;
		p.y	= ax->cursor[ic].y;
		p.x1	= ax->cursor[ic].x1;
		p.x2	= ax->cursor[ic].x2;
		p.scaled_x	= ax->cursor[ic].scaled_x;
		p.scaled_y	= ax->cursor[ic].scaled_y;
		p.scaled_x1	= ax->cursor[ic].scaled_x1;
		p.scaled_x2	= ax->cursor[ic].scaled_x2;
		p.label[0]	= '\0';
		p.reason = AXES_CROSSHAIR_DRAG;
		if(XtHasCallbacks((Widget)w, XtNcrosshairDragCallback) !=
                                        XtCallbackHasNone)
		{
		    XtCallCallbacks((Widget)w, XtNcrosshairDragCallback, &p);
		}
	    }
	}
	else if(event->type == ButtonRelease && measure)
	{
	    measure = False;
	    if(d != NULL)
	    {
		d->delta.line.display = MAP_LOCKED_ON;
	    }
	    if(a != NULL)
	    {
		a->arc.line.display = MAP_LOCKED_ON;
	    }
	    a = NULL;
	    d = NULL;
	}
	ax->last_cursor_x = cursor_x;
	ax->last_cursor_y = cursor_y;
}

void
MapMeasureDeleteAll(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	int i, j;

	for(i = mp->n_measure_dels-1; i >= 0; i--)
	{
	    for(j = 0; j < mp->ndel; j++)
	    {
		if(mp->del[j] == mp->measure_del[i])
		{
		    DeleteDelta(w, j);
		    break;
		}
	    }
	}
	mp->n_measure_dels = 0;

	for(i = mp->n_measure_arcs-1; i >= 0; i--)
	{
	    for(j = 0; j < mp->narc; j++)
	    {
		if(mp->arc[j] == mp->measure_arc[i])
		{
		    DeleteArc(w, j);
		    break;
		}
	    }
	}
	mp->n_measure_arcs = 0;

	_MapPlotRedisplay(w);

	if( mp->utm_map ) {
	    MapMeasureDeleteAll(mp->utm_map);
	}
}

void
MapMeasureDelete(MapPlotWidget w, int id)
{
	MapPlotPart *mp = &w->map_plot;
	int i, j;

	for(i = 0; i < mp->n_measure_dels; i++)
	    if(mp->measure_del[i]->delta.id == id)
	{
	    for(j = 0; j < mp->ndel; j++)
	    {
		if(mp->del[j] == mp->measure_del[i])
		{
		    DeleteDelta(w, j);
		    break;
		}
	    }
	    for(j = i; j < mp->n_measure_dels-1; j++)
	    {
		mp->measure_del[j]=mp->measure_del[j+1];
	    }
	    mp->n_measure_dels--;
	    break;
	}

	for(i = 0; i < mp->n_measure_arcs; i++)
	    if(mp->measure_arc[i]->arc.id == id)
	{
	    for(j = 0; j < mp->narc; j++)
	    {
		if(mp->arc[j] == mp->measure_arc[i])
		{
		    DeleteArc(w, j);
		    break;
		}
	    }
	    for(j = i; j < mp->n_measure_arcs-1; j++)
	    {
		mp->measure_arc[j]=mp->measure_arc[j+1];
	    }
	    mp->n_measure_arcs--;
	    break;
	}

	_MapPlotRedisplay(w);

	if( mp->utm_map ) {
	    MapMeasureDelete(mp->utm_map, id);
	}
}

int
MapPlotGetMeasurements(MapPlotWidget w, MapMeasure **m)
{
	MapPlotPart *mp = &w->map_plot;
	int i, j, n;

	if(w == NULL || !XtIsRealized((Widget)w)) return(0);

	n = mp->n_measure_dels + mp->n_measure_arcs;

	*m = (MapMeasure *)AxesMalloc((Widget)w, n*sizeof(MapMeasure));

	i = j = n = 0;
	
	while(i < mp->n_measure_dels || j < mp->n_measure_arcs)
	{
	    if(i < mp->n_measure_dels && (j >= mp->n_measure_arcs ||
		mp->measure_del[i]->delta.id < mp->measure_arc[j]->arc.id))
	    {
		(*m)[n].id = mp->measure_del[i]->delta.id;
		(*m)[n].label = mp->measure_del[i]->delta.label;
		(*m)[n].type = MAP_DELTA;
		(*m)[n].lat = mp->measure_del[i]->delta.lat;
		(*m)[n].lon = mp->measure_del[i]->delta.lon;
		(*m)[n].delta = mp->measure_del[i]->delta.del;
		(*m)[n].az = -999.;
		n++;
		i++;
	    }
	    else if(j < mp->n_measure_arcs)
	    {
		(*m)[n].id = mp->measure_arc[j]->arc.id;
		(*m)[n].label = mp->measure_arc[j]->arc.label;
		(*m)[n].type = MAP_ARC;
		(*m)[n].lat = mp->measure_arc[j]->arc.lat;
		(*m)[n].lon = mp->measure_arc[j]->arc.lon;
		(*m)[n].delta = mp->measure_arc[j]->arc.del;
		(*m)[n].az = mp->measure_arc[j]->arc.az;
		n++;
		j++;
	    }
	}
	return(n);
}

static void
Delete(MapPlotWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, cursor_x, cursor_y, type, ic;
	int		delta_id, arc_id;
	MapDelta	*d;
	MapArc		*a;
	MapMeasure	m;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	if(WhichMeasure(w, True, True, cursor_x, cursor_y, &type, &ic,
			&delta_id, &arc_id) < 0
			|| type == MAP_STATION || type == MAP_SOURCE)
	{
	    return;
	}
	if(delta_id >= 0)
	{
	    d = mp->del[delta_id];
	    m.id = d->delta.id;
	    m.label = NULL;
	    m.type = -1;	/* to indicate deletion */
	    m.lat = d->delta.lat;
	    m.lon = d->delta.lon;
	    m.delta = d->delta.del;
	    m.az = -999.;
	    for(i = 0; i < mp->n_measure_dels; i++)
		if(mp->measure_del[i] == d)
	    {
		mp->n_measure_dels--;
		for(j = i; j < mp->n_measure_dels; j++)
		{
		    mp->measure_del[j] = mp->measure_del[j+1];
		}
		break;
	    }
	    DeleteDelta(w, delta_id);
	    XtCallCallbacks((Widget)w, XtNmapMeasureCallback, &m);
	}
	if(arc_id >= 0)
	{
	    a = mp->arc[arc_id];
	    m.id = a->arc.id;
	    m.label = NULL;
	    m.type = -1;
	    m.lat = a->arc.lat;
	    m.lon = a->arc.lon;
	    m.delta = a->arc.del;
	    m.az = a->arc.az;
	    for(i = 0; i < mp->n_measure_arcs; i++)
		if(mp->measure_arc[i] == a)
	    {
		mp->n_measure_arcs--;
		for(j = i; j < mp->n_measure_arcs; j++)
		{
		    mp->measure_arc[j] = mp->measure_arc[j+1];
		}
		break;
	    }
	    DeleteArc(w, arc_id);
	    XtCallCallbacks((Widget)w, XtNmapMeasureCallback, &m);
	}
	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	RedisplayOverlays(w, ax->pixmap);
	RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	_AxesRedisplayXor((AxesWidget)w);
}

static int
WhichCircle(MapPlotWidget w, int  cursor_x, int cursor_y)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j;
	double	x, y, lon, lat, dmin, d;

	x = scale_x(&ax->d, cursor_x);
	y = scale_y(&ax->d, cursor_y);

	if(x < ax->x1[ax->zoom] || x > ax->x2[ax->zoom] ||
	   y < ax->y1[ax->zoom] || y > ax->y2[ax->zoom])
	{
	    return(-1);
	}

	unproject(w, x, y, &lon, &lat);

	if(mp->ncircles <= 0) return(-1);

	for(i = 0; i < mp->ncircles; i++)
	{
	    if(mp->circle[i].display) break;
	}
	if(i == mp->ncircles) return(-1);

	x = lon - mp->circle[i].lon;
	y = lat - mp->circle[i].lat;
	dmin = x*x + y*y;

	for(j = i; i < mp->ncircles; i++) if(mp->circle[i].display)
	{
	    x = lon - mp->circle[i].lon;
	    y = lat - mp->circle[i].lat;
	    d = x*x + y*y;
	    if(d < dmin)
	    {
		dmin = d;
		j = i;
	    }
	}
	return(j);
}

static int
WhichStaSrc(MapPlotWidget w, int cursor_x, int cursor_y, Boolean *station)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, which, x0, y0;
	double		x, y, z;
	float		d, dmin;

	/* initialize dmin to a large value before seeking the minimim.
	*/
	dmin = 2*((int)w->core.width*(int)w->core.width +
		  (int)w->core.height*(int)w->core.height);
	which = -1;

	if(_AxesWhichCursor((AxesWidget)w, cursor_x, cursor_y, &d, &i) >= 0)
	{
		dmin = d*d;
	}

	for(i = 0; i < mp->nsta; i++)
	    if( (mp->sta[i]->station.sym.display >= MAP_ON ||
		(mp->sta[i]->station.sym.display == MAP_SELECTED_ON
			&& mp->sta[i]->station.sym.selected)) &&
			mp->sta[i]->visible)
	{
	    project(w, mp->sta[i]->station.lon, mp->sta[i]->station.lat,
			&x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		d = (x0 - cursor_x)*(x0 - cursor_x) + 
		    (y0 - cursor_y)*(y0 - cursor_y);
		if(d < dmin)
		{
		    dmin = d;
		    which = i;
		    *station = True;
		}
	    }
	}
	/* cursor must be within 20 pixels of the station
	 */
	if(sqrt(fabs(dmin)) > 20)
	{
	    *station = False;
	    which = -1;
	}

	for(i = 0; i < mp->nsrc; i++)
	    if( (mp->src[i]->source.sym.display >= MAP_ON ||
		(mp->src[i]->source.sym.display == MAP_SELECTED_ON
			&& mp->src[i]->source.sym.selected)) &&
			mp->src[i]->visible)
	{
	    project(w, mp->src[i]->source.lon, mp->src[i]->source.lat,
			&x, &y, &z);
	    if(z > 0.)
	    {
		x0 = unscale_x(&ax->d, x);
		y0 = unscale_y(&ax->d, y);
		d = (x0 - cursor_x)*(x0 - cursor_x) + 
		    (y0 - cursor_y)*(y0 - cursor_y);
		if(d < dmin)
		{
		    dmin = d;
		    which = i;
		    *station = False;
		}
	    }
	}
	/* cursor must be within 20 pixels of the source
	 */
	if(sqrt(fabs(dmin)) > 20)
	{
	    *station = False;
	    which = -1;
	}
	return(which);
}

static int
WhichSymbol(MapPlotWidget w, int cursor_x, int cursor_y, int max, int *imin,
		int *jmin, int nearDist, Boolean *near)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, which, x0, y0, min_x0 = -1, min_y0 = -1;
	double		x, y, z;
	float		d, dmin;

	/* initialize dmin to a large value before seeking the minimim.
	*/
	dmin = 2*((int)w->core.width*(int)w->core.width +
		  (int)w->core.height*(int)w->core.height);
	which = -1;

	if(_AxesWhichCursor((AxesWidget)w, cursor_x, cursor_y, &d, &i) >= 0)
	{
	    dmin = d*d;
	}
	*near = False;

	for(i = 0; i < mp->nsymgrp; i++)
	{
	    SymbolInfo *sym = &mp->symgrp[i]->group.sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		MapSymbolGroup *s = mp->symgrp[i];

		for(j = 0; j < s->group.npts; j++)
		{
		    project(w, s->group.lon[j], s->group.lat[j], &x, &y, &z);
		    if(z > 0.)
		    {
			x0 = unscale_x(&ax->d, x);
			y0 = unscale_y(&ax->d, y);
			d = (x0 - cursor_x)*(x0 - cursor_x) +
				(y0 - cursor_y)*(y0 - cursor_y);
			if(d < dmin)
			{
			    dmin = d;
			    imin[0] = i;
			    jmin[0] = j;
			    which = 0;
			    mp->motion_x = x0;
			    mp->motion_y = y0;
			}
		    }
		}
	    }
	}
	for(i = 0; i < mp->nsymbol; i++)
	{
	    SymbolInfo *sym = &mp->symbol[i].sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		MapPlotSymbol *s = &mp->symbol[i];

		project(w, s->lon, s->lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    d = (x0 - cursor_x)*(x0 - cursor_x) +
			(y0 - cursor_y)*(y0 - cursor_y);
		    if(d < dmin)
		    {
			dmin = d;
			imin[0] = i;
			which = 1;
			mp->motion_x = x0;
			mp->motion_y = y0;
			min_x0 = x0;
			min_y0 = y0;
		    }
		}
	    }
	}
	if(which >= 0 && dmin < nearDist) {
	    *near = True;
	    if(which == 1) {
		for(i = 0; i < mp->nsymbol; i++) if(i != imin[0])
		{
		    SymbolInfo *sym = &mp->symbol[i].sym;
		    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON
			|| (sym->display == MAP_SELECTED_ON && sym->selected))
		    {
			MapPlotSymbol *s = &mp->symbol[i];

			project(w, s->lon, s->lat, &x, &y, &z);
			if(z > 0.)
			{
			    x0 = unscale_x(&ax->d, x);
			    y0 = unscale_y(&ax->d, y);
			    if(which < max && x0 == min_x0 && y0 == min_y0) {
				imin[which++] = i;
			    }
			}
		    }
		}
	    }
	}

	return which;
}

static int
WhichMeasure(MapPlotWidget w, Boolean stasrc, Boolean arcdel, int cursor_x,
		int cursor_y, int *type, int *ic, int *delta_id, int *arc_id)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, which, x0, y0, d_id, a_id;
	double		x, y, z, delta_d, arc_d;
	float		d, dmin;

	*delta_id = -1;
	*arc_id = -1;
	*type = -1;
	/* initialize dmin to a large value before seeking the minimim.
	*/
	dmin = 2*((int)w->core.width*(int)w->core.width +
		  (int)w->core.height*(int)w->core.height);
	which = -1;

	if(!arcdel &&
	    (*ic = _AxesWhichCursor((AxesWidget)w,cursor_x,cursor_y,&d,&i)) >= 0
		&& ax->cursor[*ic].type == AXES_CROSSHAIR)
	{
	    which = *ic;
	    dmin = d*d;
	}

	if(stasrc)
	{
	    for(i = 0; i < mp->nsta; i++)
		if((mp->sta[i]->station.sym.display >= MAP_ON ||
		   (mp->sta[i]->station.sym.display == MAP_SELECTED_ON
			&& mp->sta[i]->station.sym.selected)) &&
				mp->sta[i]->visible)
	    {
		project(w, mp->sta[i]->station.lon, mp->sta[i]->station.lat,
			&x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    d = (x0 - cursor_x)*(x0 - cursor_x) + 
			(y0 - cursor_y)*(y0 - cursor_y);
		    if(d < dmin)
		    {
			dmin = d;
			which = i;
			*type = MAP_STATION;
			*ic = -1;
		    }
		}
	    } 
	}
	if(stasrc)
	{
	    for(i = 0; i < mp->nsrc; i++)
		if((mp->src[i]->source.sym.display >= MAP_ON ||
		   (mp->src[i]->source.sym.display == MAP_SELECTED_ON
			&& mp->src[i]->source.sym.selected))
			&& mp->src[i]->visible)
	    {
		project(w, mp->src[i]->source.lon, mp->src[i]->source.lat,
			&x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    d = (x0 - cursor_x)*(x0 - cursor_x) + 
			    (y0 - cursor_y)*(y0 - cursor_y);
		    if(d < dmin)
		    {
			dmin = d;
			which = i;
			*type = MAP_SOURCE;
			*ic = -1;
		    }
		}
	    } 
	}

	if(arcdel)
	{
	    arc_d = delta_d = 2*((int)w->core.width*(int)w->core.width +
			  (int)w->core.height*(int)w->core.height);
	    d_id = a_id = -1;
	    for(i = 0; i < mp->ndel; i++) 
		if(mp->del[i]->type == MAP_MEASURE ||
		   mp->del[i]->type == MAP_CURSOR_MEASURE)
	    {
		for(j = 0; j < mp->del[i]->s.nsegs; j++)
	       	{
		    d = AxesClass::pointToLine(
				(float)mp->del[i]->s.segs[j].x1,
				(float)mp->del[i]->s.segs[j].y1,
				(float)mp->del[i]->s.segs[j].x2,
				(float)mp->del[i]->s.segs[j].y2,
				(float)cursor_x, (float)cursor_y);
		    if(d < delta_d)
		    {
			delta_d = d;
	 		d_id = i;
		    }
		}
	    }
	    for(i = 0; i < mp->narc; i++) 
		if(mp->arc[i]->type == MAP_MEASURE ||
		   mp->arc[i]->type == MAP_CURSOR_MEASURE)
	    {
		for(j = 0; j < mp->arc[i]->s.nsegs; j++)
	       	{
		    d = AxesClass::pointToLine(
				(float)mp->arc[i]->s.segs[j].x1,
				(float)mp->arc[i]->s.segs[j].y1,
				(float)mp->arc[i]->s.segs[j].x2,
				(float)mp->arc[i]->s.segs[j].y2,
				(float)cursor_x, (float)cursor_y);
		    if(d < arc_d)
		    {
			arc_d = d;
	 		a_id = i;
		    }
		}
	    }
	    if(dmin < delta_d && dmin < arc_d)
	    {
		return(which);
	    }
	    else if(d_id >= 0 && a_id >= 0)
	    {
		if(sqrt(delta_d) < 10 && sqrt(arc_d) < 10)
		{
		    *type = MAP_DELTA;
		    which = d_id;
		    *arc_id = a_id;
		    *delta_id = d_id;
		}
		else if(delta_d < arc_d)
		{
		    *type = MAP_DELTA;
		    which = d_id;
		    *delta_id = d_id;
		}
		else
		{
		    *type = MAP_ARC;
		    which = a_id;
		    *arc_id = a_id;
		}
		*ic = -1;
	    }
	    else if(d_id >= 0)
	    {
		*type = MAP_DELTA;
		which = d_id;
		*ic = -1;
		*delta_id = d_id;
	    }
	    else if(a_id >= 0)
	    {
		*type = MAP_ARC;
		which = a_id;
		*ic = -1;
		*arc_id = a_id;
	    }
	}
	return(which);
}

static Boolean
nearArcDel(MapPlotWidget w, int cursor_x, int cursor_y, int dmin)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i, j;
	double		delta_d, arc_d;
	float		d;

	arc_d = delta_d = 2*((int)w->core.width*(int)w->core.width +
			  (int)w->core.height*(int)w->core.height);
	for(i = 0; i < mp->ndel; i++)
	    if(mp->del[i]->type == MAP_MEASURE ||
		mp->del[i]->type == MAP_CURSOR_MEASURE)
	{
	    for(j = 0; j < mp->del[i]->s.nsegs; j++)
	    {
		d = AxesClass::pointToLine((float)mp->del[i]->s.segs[j].x1,
				(float)mp->del[i]->s.segs[j].y1,
				(float)mp->del[i]->s.segs[j].x2,
				(float)mp->del[i]->s.segs[j].y2,
				(float)cursor_x, (float)cursor_y);
		if(d < delta_d)
		{
		    delta_d = d;
		    if(delta_d < dmin) return True;
		}
	    }
	}
	for(i = 0; i < mp->narc; i++)
	    if(mp->arc[i]->type == MAP_MEASURE ||
		mp->arc[i]->type == MAP_CURSOR_MEASURE)
	{
	    for(j = 0; j < mp->arc[i]->s.nsegs; j++)
	    {
		d = AxesClass::pointToLine((float)mp->arc[i]->s.segs[j].x1,
				(float)mp->arc[i]->s.segs[j].y1,
				(float)mp->arc[i]->s.segs[j].x2,
				(float)mp->arc[i]->s.segs[j].y2,
				(float)cursor_x, (float)cursor_y);
		if(d < arc_d)
		{
		    arc_d = d;
		    if(arc_d < dmin) return True;
		}
	    }
	}
	return False;
}

static Boolean
nearStaSrc(MapPlotWidget w, int cursor_x, int cursor_y, int dmin)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, x0, y0;
	double		x, y, z, sta_d, d;

	sta_d = 2*((int)w->core.width*(int)w->core.width +
			  (int)w->core.height*(int)w->core.height);
	dmin = dmin*dmin;

	for(i = 0; i < mp->nsta; i++)
	{
	    if(mp->sta[i]->station.sym.display >= MAP_ON && mp->sta[i]->visible)
	    {
		project(w, mp->sta[i]->station.lon, mp->sta[i]->station.lat,
			&x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    d = (x0 - cursor_x)*(x0 - cursor_x) + 
			(y0 - cursor_y)*(y0 - cursor_y);
		    if(d < sta_d)
		    {
			sta_d = d;
			if(sta_d < dmin) return True;
		    }
		}
	    } 
	}
	for(i = 0; i < mp->nsrc; i++)
	{
	    if(mp->src[i]->source.sym.display >= MAP_ON && mp->src[i]->visible)
	    {
		project(w, mp->src[i]->source.lon, mp->src[i]->source.lat,
			&x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    d = (x0 - cursor_x)*(x0 - cursor_x) + 
			    (y0 - cursor_y)*(y0 - cursor_y);
		    if(d < sta_d)
		    {
			sta_d = d;
			if(sta_d < dmin) return True;
		    }
		}
	    } 
	}
	return False;
}

static Boolean
nearPolarSelection(MapPlotWidget w, int cursor_x, int cursor_y, int dmin,
			int *side)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;
	double		d, polar_d;

	if( !mp->polar_line ) return False;

	dmin = dmin*dmin;
	polar_d = 2*((int)w->core.width*(int)w->core.width +
			  (int)w->core.height*(int)w->core.height);

	for(i = 0; i < mp->polar_line->s.nsegs; i++)
	{
	    d = AxesClass::pointToLine((float)mp->polar_line->s.segs[i].x1,
				(float)mp->polar_line->s.segs[i].y1,
				(float)mp->polar_line->s.segs[i].x2,
				(float)mp->polar_line->s.segs[i].y2,
				(float)cursor_x, (float)cursor_y);
	    if(d < polar_d)
	    {
		polar_d = d;
		if(polar_d < dmin) {
		    if(i == 0) *side = 0;
		    else if(i < mp->polar_select_i3) *side = 1;
		    else if(i == mp->polar_select_i3) *side = 2;
		    else *side = 3;
		    return True;
		}
	    }
	}
	*side = -1;
	return False;
}

/*
static Boolean
nearSymbol(MapPlotWidget w, int cursor_x, int cursor_y, int dmin)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, x0, y0;
	double		x, y, z, sym_d, d;

	sym_d = 2*((int)w->core.width*(int)w->core.width +
			  (int)w->core.height*(int)w->core.height);
	dmin = dmin*dmin;

	for(i = 0; i < mp->nsymgrp; i++)
	{
	    SymbolInfo *sym = &mp->symgrp[i]->group.sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		MapSymbolGroup *s = mp->symgrp[i];

		for(j = 0; j < s->group.npts; j++)
		{
		    project(w, s->group.lon[j], s->group.lat[j], &x, &y, &z);
		    if(z > 0.)
		    {
			x0 = unscale_x(&ax->d, x);
			y0 = unscale_y(&ax->d, y);
			d = (x0 - cursor_x)*(x0 - cursor_x) + 
				(y0 - cursor_y)*(y0 - cursor_y);
			if(d < sym_d)
			{
			    sym_d = d;
			    if(sym_d < dmin) return True;
			}
		    }
		}
	    } 
	}
	for(i = 0; i < mp->nsymbol; i++)
	{
	    SymbolInfo *sym = &mp->symbol[i].sym;
	    if(sym->display == MAP_ON || sym->display == MAP_LOCKED_ON ||
		(sym->display == MAP_SELECTED_ON && sym->selected))
	    {
		MapPlotSymbol *s = &mp->symbol[i];

		project(w, s->lon, s->lat, &x, &y, &z);
		if(z > 0.)
		{
		    x0 = unscale_x(&ax->d, x);
		    y0 = unscale_y(&ax->d, y);
		    d = (x0 - cursor_x)*(x0 - cursor_x) + 
			(y0 - cursor_y)*(y0 - cursor_y);
		    if(d < sym_d)
		    {
			sym_d = d;
			if(sym_d < dmin) return True;
		    }
		}
	    } 
	}
	return False;
}
*/

void
MapPlotSelectStation(MapPlotWidget w, char *name, Boolean selected,
			Boolean callbacks, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nsta; i++)
	{
	    if(mp->sta[i]->station.label != NULL &&
		!strcmp(mp->sta[i]->station.label, name) &&
		selected != mp->sta[i]->station.sym.selected)
	    {
		DoSelectStation(w, mp->sta[i], callbacks, redisplay);
		return;
	    }
	}

	if( mp->utm_map ) {
	    MapPlotSelectStation(mp->utm_map, name, selected, False, redisplay);
	}
}

static void
DoSelectStation(MapPlotWidget w, MapStation *sta, Boolean callbacks,
		Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	MapPlotStation	station;
	MapArc		*a;
	MapDelta	*d;

	sta->station.sym.selected = !sta->station.sym.selected;

	for(i = 0; i < mp->narc; i++)
	    if( mp->arc[i]->sta_id == sta->station.id &&
		mp->arc[i]->type != MAP_PATH)
	{
	    a = mp->arc[i];
	    a->arc.line.selected = sta->station.sym.selected;
	    if(a->arc.line.selected && a->arc.line.display == MAP_SELECTED_ON)
	    {
		a = mp->arc[i];
		if(a->changed) MakeArc(w, a);
		DrawArc(w, &ax->d, a);
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	    if(mp->del[i]->sta_id == sta->station.id)
	{
	    d = mp->del[i];
	    d->delta.line.selected = sta->station.sym.selected;
	    if(d->delta.line.selected &&
		d->delta.line.display == MAP_SELECTED_ON)
	    {
		if(d->changed) MakeDelta(w, d);
		DrawDelta(w, &ax->d, d);
	    }
	}

	if(redisplay) {
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}

	if(callbacks)
	{
	    station = sta->station;
	    XtCallCallbacks((Widget)w, XtNselectStationCallback, &station);
	}
}

static void
SelectCircle(MapPlotWidget w, int  i)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	mp->circle[i].selected = mp->circle[i].selected ?  False : True;

	if(i == mp->primary_circle)
	{
	    XSetForeground(XtDisplay(w), ax->mag_invertGC, ax->select_fg);
	}
	else
	{
	    XSetForeground(XtDisplay(w), ax->mag_invertGC, ax->mag_fg);
	}
	if(mp->num_circle_segs[i] > 0)
	{
	    XDrawSegments(XtDisplay(w), XtWindow(w), ax->mag_invertGC,
			(XSegment *)mp->circles.segs+mp->circle_start[i],
			mp->num_circle_segs[i]);
	}
}

void
MapPlotSelectSource(MapPlotWidget w, int id, Boolean selected,
		Boolean callbacks, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;
	MapSource *src;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nsrc; i++)
	{
	    src = mp->src[i];
	    if(id == src->source.id)
	    {
		break;
	    }
	}
	if(i < mp->nsrc && selected != src->source.sym.selected)
	{
	    DoSelectSource(w, src, callbacks, redisplay);
	}

	if( mp->utm_map ) {
	    MapPlotSelectSource(mp->utm_map, id, selected, False, redisplay);
	}
}

static void
DoSelectSource(MapPlotWidget w, MapSource *src, Boolean callbacks,
		Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	MapPlotSource	source;
	MapArc		*a;
	MapDelta	*d;
	MapEllipse	*e;

	src->source.sym.selected = (src->source.sym.selected) ? False : True;

	for(i = 0; i < mp->narc; i++)
	    if(mp->arc[i]->src_id == src->source.id)
	{
	    a = mp->arc[i];
	    a->arc.line.selected = src->source.sym.selected;
	    if(a->arc.line.selected && a->arc.line.display == MAP_SELECTED_ON)
	    {
		if(a->changed) MakeArc(w, a);
		DrawArc(w, &ax->d, a);
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	    if(mp->del[i]->src_id == src->source.id)
	{
	    d = mp->del[i];
	    d->delta.line.selected = src->source.sym.selected;
	    if(d->delta.line.selected &&
			d->delta.line.display == MAP_SELECTED_ON)
	    {
		if(d->changed) MakeDelta(w, d);
		DrawDelta(w, &ax->d, d);
	    }
	}
	for(i = 0; i < mp->nell; i++)
	    if(mp->ell[i]->src_id == src->source.id)
	{
	    e = mp->ell[i];
	    e->ellipse.line.selected = src->source.sym.selected;
	    if(e->ellipse.line.selected &&
			e->ellipse.line.display == MAP_SELECTED_ON)
	    {
		if(e->changed) MakeEllipse(w, e);
		Free(e->s.segs);
		e->s.size_segs = 0;
		e->s.nsegs = 0;
		ax->d.s  = &e->s;
		DrawLonLat(w, &ax->d, e->npts, e->lon, e->lat);
	    }
	}
	if(redisplay) {
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}

	if(callbacks)
	{
	    source = src->source;
	    XtCallCallbacks((Widget)w, XtNselectSourceCallback, &source);
	}
}

static void
DrawLonLat(MapPlotWidget w, DrawStruct *d, int n, double *lon, double *lat)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i;
	Boolean	last_in;
	double	lastx, lasty, x, y, z, x_lon;

	if(!XtIsRealized((Widget)w) || n <= 0) return;

	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    x_lon = lon[0];
	    if(x_lon < ax->x1[0]) x_lon += 360;
	    else if(x_lon > ax->x2[0]) x_lon -= 360;
	    imove(d, x_lon, lat[0]);
	    lastx = x_lon;
	    lasty = lat[0];
	    for(i = 1; i < n; i++)
	    {
		x_lon = lon[i];
		if(x_lon < ax->x1[0]) x_lon += 360;
		else if(x_lon > ax->x2[0]) x_lon -= 360;
		if(fabs(x_lon - lastx) > 180.)
		{
		    if(lastx > mp->phi) {
			idraw(d, x_lon+360., lat[i]);
			imove(d, lastx-360., lasty);
		    }
		    else {
			idraw(d, x_lon-360., lat[i]);
			imove(d, lastx+360., lasty);
		    }
		}
		idraw(d, x_lon, lat[i]);
		lastx = x_lon;
		lasty = lat[i];
	    }
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		mp->projection == MAP_MERCATOR)
	{
	    x_lon = lon[0];
	    if(x_lon < ax->x1[0]) x_lon += 360;
	    else if(x_lon > ax->x2[0]) x_lon -= 360;
	    project(w, x_lon, lat[0], &x, &y, &z);
	    imove(d, x_lon, y);
	    lastx = x_lon;
	    lasty = y;
	    for(i = 1; i < n; i++)
	    {
		x_lon = lon[i];
		project(w, x_lon, lat[i], &x, &y, &z);
		if(x_lon < ax->x1[0]) x_lon += 360;
		else if(x_lon > ax->x2[0]) x_lon -= 360;
		if(fabs(x_lon - lastx) > 180.)
		{
		    if(lastx > mp->phi) {
			idraw(d, x_lon+360., y);
			imove(d, lastx-360., lasty);
		    }
		    else {
			idraw(d, x_lon-360., y);
			imove(d, lastx+360., lasty);
		    }
		}
		idraw(d, x_lon, y);
		lastx = x_lon;
		lasty = y;
	    }
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC ||
		mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
		mp->projection == MAP_AZIMUTHAL_EQUAL_AREA ||
		mp->projection == MAP_UTM_NEAR)
	{
	    project(w, lon[0], lat[0], &x, &y, &z);
	    last_in = (z > 0.) ? True : False;
	    if(last_in) imove(d, x, y);
	    for(i = 1; i < n; i++)
	    {
		project(w, lon[i], lat[i], &x, &y, &z);

		if(z > 0.) {
		    if(!last_in) imove(d, x, y);
		    else idraw(d, x, y);
		    last_in = True;
		}
		else {
		    last_in = False;
		}
	    }
	}
	else if(mp->projection == MAP_POLAR)
	{
	    x = lon[0]*sin(lat[0]*DEG_TO_RADIANS);
	    y = lon[0]*cos(lat[0]*DEG_TO_RADIANS);
	    imove(d, x, y);
	    for(i = 1; i < n; i++)
	    {
		x = lon[i]*sin(lat[i]*DEG_TO_RADIANS);
		y = lon[i]*cos(lat[i]*DEG_TO_RADIANS);
		idraw(d, x, y);
	    }
	}
	iflush(d);
}

static void
DrawPolarLonLat(MapPlotWidget w, DrawStruct *d, MapPlotTheme *theme,
			int n, double *r, double *theta)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i;
	Boolean	last_in;
	double alpha, beta, gamma, c[3][3];
	double	lastx, lasty, x, y, z, x_lon, lat, lon, th, phi, sn, xp, yp, zp;

	if(!XtIsRealized((Widget)w) || n <= 0) return;

	/*
	 * alpha, beta, gamma from north pole to polar origin
	 */
	alpha = theme->center_lon*rad;
	beta = (90. - theme->center_lat)*rad;
	gamma = 0.;
	rotation_matrix(alpha, beta, gamma, c);

	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    phi = (180. - theta[0])*DEG_TO_RADIANS;
	    th = r[0]/6371.;
            sn = sin(th);
            x = sn*cos(phi);
            y = sn*sin(phi);
            z = cos(th);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    th = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat = 90. - th/DEG_TO_RADIANS;
	    lon = atan2(yp, xp)/DEG_TO_RADIANS;

	    x_lon = lon;
	    if(x_lon < ax->x1[0]) x_lon += 360;
	    else if(x_lon > ax->x2[0]) x_lon -= 360;
	    imove(d, x_lon, lat);
	    lastx = x_lon;
	    lasty = lat;
	    for(i = 1; i < n; i++)
	    {
		phi = (180. - theta[i])*DEG_TO_RADIANS;
		th = r[i]/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		lat = 90. - th/DEG_TO_RADIANS;
		lon = atan2(yp, xp)/DEG_TO_RADIANS;

		x_lon = lon;
		if(x_lon < ax->x1[0]) x_lon += 360;
		else if(x_lon > ax->x2[0]) x_lon -= 360;
		if(fabs(x_lon - lastx) > 180.)
		{
		    if(lastx > mp->phi) {
			idraw(d, x_lon+360., lat);
			imove(d, lastx-360., lasty);
		    }
		    else {
			idraw(d, x_lon-360., lat);
			imove(d, lastx+360., lasty);
		    }
		}
		idraw(d, x_lon, lat);
		lastx = x_lon;
		lasty = lat;
	    }
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		mp->projection == MAP_MERCATOR)
	{
	    phi = (180. - theta[0])*DEG_TO_RADIANS;
	    th = r[0]/6371.;
            sn = sin(th);
            x = sn*cos(phi);
            y = sn*sin(phi);
            z = cos(th);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    th = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat = 90. - th/DEG_TO_RADIANS;
	    lon = atan2(yp, xp)/DEG_TO_RADIANS;

	    x_lon = lon;
	    if(x_lon < ax->x1[0]) x_lon += 360;
	    else if(x_lon > ax->x2[0]) x_lon -= 360;
	    project(w, x_lon, lat, &x, &y, &z);
	    imove(d, x_lon, y);
	    lastx = x_lon;
	    lasty = y;
	    for(i = 1; i < n; i++)
	    {
		phi = (180. - theta[i])*DEG_TO_RADIANS;
		th = r[i]/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		lat = 90. - th/DEG_TO_RADIANS;
		lon = atan2(yp, xp)/DEG_TO_RADIANS;

		x_lon = lon;
		project(w, x_lon, lat, &x, &y, &z);
		if(x_lon < ax->x1[0]) x_lon += 360;
		else if(x_lon > ax->x2[0]) x_lon -= 360;
		if(fabs(x_lon - lastx) > 180.)
		{
		    if(lastx > mp->phi) {
			idraw(d, x_lon+360., y);
			imove(d, lastx-360., lasty);
		    }
		    else {
			idraw(d, x_lon-360., y);
			imove(d, lastx+360., lasty);
		    }
		}
		idraw(d, x_lon, y);
		lastx = x_lon;
		lasty = y;
	    }
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC ||
		mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
		mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    phi = (180. - theta[0])*DEG_TO_RADIANS;
	    th = r[0]/6371.;
            sn = sin(th);
            x = sn*cos(phi);
            y = sn*sin(phi);
            z = cos(th);

	    zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
	    xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
	    yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

	    th = atan2(sqrt(xp*xp + yp*yp), zp);
	    lat = 90. - th/DEG_TO_RADIANS;
	    lon = atan2(yp, xp)/DEG_TO_RADIANS;

	    project(w, lon, lat, &x, &y, &z);
	    last_in = (z > 0.) ? True : False;
	    if(last_in) imove(d, x, y);
	    for(i = 1; i < n; i++)
	    {
		phi = (180. - theta[i])*DEG_TO_RADIANS;
		th = r[i]/6371.;
		sn = sin(th);
		x = sn*cos(phi);
		y = sn*sin(phi);
		z = cos(th);

		zp = c[2][0]*x + c[2][1]*y + c[2][2]*z;
		xp = c[0][0]*x + c[0][1]*y + c[0][2]*z;
		yp = c[1][0]*x + c[1][1]*y + c[1][2]*z;

		th = atan2(sqrt(xp*xp + yp*yp), zp);
		lat = 90. - th/DEG_TO_RADIANS;
		lon = atan2(yp, xp)/DEG_TO_RADIANS;

		project(w, lon, lat, &x, &y, &z);

		if(z > 0.) {
		    if(!last_in) imove(d, x, y);
		    else idraw(d, x, y);
		    last_in = True;
		}
		else {
		    last_in = False;
		}
	    }
	}
	else if(mp->projection == MAP_POLAR)
	{
	    x = r[0]*sin(theta[0]*DEG_TO_RADIANS);
	    y = r[0]*cos(theta[0]*DEG_TO_RADIANS);
	    imove(d, x, y);
	    for(i = 1; i < n; i++)
	    {
		x = r[i]*sin(theta[i]*DEG_TO_RADIANS);
		y = r[i]*cos(theta[i]*DEG_TO_RADIANS);
		idraw(d, x, y);
	    }
	}
	iflush(d);
}

static void
DrawLonLatXor(MapPlotWidget w, DrawStruct *d, int n, double *lon, double *lat)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, nsegs;
	Boolean		last_in;
	double		lastx, lasty, x, y, z, x0, y0;
	float		*fx, *fy;
	RSegArray	r;

	if(!XtIsRealized((Widget)w)) return;

	fx = (float *)AxesMalloc((Widget)w, 2*n*sizeof(float));
	fy = (float *)AxesMalloc((Widget)w, 2*n*sizeof(float));

	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    if(lon[0] < ax->x1[0]) lon[0] += 360;
	    else if(lon[0] > ax->x2[0]) lon[0] -= 360;
	    fx[0] = lon[0];
	    fy[0] = lat[0];
	    lastx = lon[0];
	    lasty = lat[0];
	    for(i = j = 1; i < n; i++)
	    {
		if(lon[i] < ax->x1[0]) lon[i] += 360;
		else if(lon[i] > ax->x2[0]) lon[i] -= 360;
		if(fabs(lon[i] - lastx) > 180.)
		{
		    if(lastx > mp->phi) {
			fx[j] = lon[i]+360.;
			fy[j] = lat[i];
			j++;
			set_fnan(fx[j]);
			set_fnan(fy[j]);
			j++;
			fx[j] = lastx-360.;
			fy[j] = lasty;
			j++;
		    }
		    else {
			fx[j] = lon[i]-360.;
			fy[j] = lat[i];
			j++;
			set_fnan(fx[j]);
			set_fnan(fy[j]);
			j++;
			fx[j] = lastx+360.;
			fy[j] = lasty;
			j++;
		    }
		}
		fx[j] = lon[i];
		fy[j] = lat[i];
		j++;
		lastx = lon[i];
		lasty = lat[i];
	    }
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		mp->projection == MAP_MERCATOR)
	{
	    if(mp->projection == MAP_MERCATOR ||
	       mp->projection == MAP_CYLINDRICAL_EQUAL_AREA)
	    {
		project(w, lon[0], lat[0], &x, &y, &z);
	    }
	    if(lon[0] < ax->x1[0]) lon[0] += 360;
	    else if(lon[0] > ax->x2[0]) lon[0] -= 360;
	    fx[0] = lon[0];
	    fy[0] = y;
	    lastx = lon[0];
	    lasty = y;
	    for(i = j = 1; i < n; i++)
	    {
		project(w, lon[i], lat[i], &x, &y, &z);
		if(lon[i] < ax->x1[0]) lon[i] += 360;
		else if(lon[i] > ax->x2[0]) lon[i] -= 360;
		if(fabs(lon[i] - lastx) > 180.)
		{
		    if(lastx > mp->phi)
		    {
			fx[j] = lon[i]+360.;
			fy[j] = y;
			j++;
			set_fnan(fx[j]);
			set_fnan(fy[j]);
			j++;
			fx[j] = lastx-360.;
			fy[j] = lasty;
			j++;
/*
	idraw(d, lon[i]+360., lat[i]);
	imove(d, lastx-360., lasty);
*/
		    }
		    else
		    {
			fx[j] = lon[i]-360.;
			fy[j] = y;
			j++;
			set_fnan(fx[j]);
			set_fnan(fy[j]);
			j++;
			fx[j] = lastx+360.;
			fy[j] = lasty;
			j++;
/*
	idraw(d, lon[i]-360., lat[i]);
	imove(d, lastx+360., lasty);
*/
		    }
		}
		fx[j] = lon[i];
		fy[j] = y;
		j++;
/* idraw(d, lon[i], lat[i]); */
		lastx = lon[i];
		lasty = y;
	    }
	}
	else
	{
	    project(w, lon[0], lat[0], &x, &y, &z);
	    last_in = (z > 0.) ? True : False;
	    j = 0;
	    if(last_in)
	    {
/* imove(d, x, y); */
		fx[j] = x;
		fy[j] = y;
		j++;
	    }
	    for(i = 1; i < n; i++)
	    {
		project(w, lon[i], lat[i], &x, &y, &z);
		if(z > 0.)
		{
		    if(!last_in)
		    {
/* imove(d, x, y); */
			if(j > 0) {
			    set_fnan(fx[j]);
			    set_fnan(fy[j]);
			    j++;
			}
			fx[j] = x;
			fy[j] = y;
			j++;
		    }
		    else {
			fx[j] = x;
			fy[j] = y;
			j++;
/* idraw(d, x, y); */
		    }
		    last_in = True;
		}
		else {
		    last_in = False;
		}
	    }
	}

	r.nsegs = 0;
	r.segs = NULL;
	r.size_segs = 0;

	plotxd(j, fx, fy, 0., d->unscalex, d->unscaley, 0., 0., &r);
	Free(fx);
	Free(fy);

	nsegs = d->s->nsegs + r.nsegs;
	d->s->segs = (DSegment *)AxesRealloc((Widget)w, d->s->segs,
			nsegs * sizeof(DSegment));
	d->s->size_segs = nsegs;
	j = d->s->nsegs;
	d->s->nsegs = nsegs;

	x0 = d->sx1*d->unscalex;
	y0 = d->sy1*d->unscaley;
	for(i = 0; i < r.nsegs; i++)
	{
	    d->s->segs[j+i].x1 = (short int)(d->ix1 + ( r.segs[i].x1-x0));
	    d->s->segs[j+i].x2 = (short int)(d->ix1 + ( r.segs[i].x2-x0));
	    d->s->segs[j+i].y1 = (short int)(d->iy1 + (-r.segs[i].y1-y0));
	    d->s->segs[j+i].y2 = (short int)(d->iy1 + (-r.segs[i].y2-y0));
	}
	Free(r.segs);

	iflush(d);
}

static void
DrawBorder(MapPlotWidget w, DrawStruct *d)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i;
	double phi, dphi, x, y, r, x1, x2, y1, y2;

	if(!XtIsRealized((Widget)w)) return;

	Free(mp->border.segs);
	mp->border.size_segs = 0;
	mp->border.nsegs = 0;

	d->s = &mp->border;

	x1 = ax->x1[ax->zoom];
	x2 = ax->x2[ax->zoom];
	y1 = ax->y1[ax->zoom];
	y2 = ax->y2[ax->zoom];

	if(mp->projection == MAP_ORTHOGRAPHIC)
	{
	    r = 1.;
	    if(ax->zoom == 0)
	    {
		x1 = -1.1;
		x2 =  1.1;
		y1 = -1.1;
		y2 =  1.1;
	    }
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	{
	    r = .95*pi;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    r = .95*2.;
	}
	else if(mp->projection == MAP_POLAR)
	{
	    r = mp->polar_max_radius;
	}
	else
	{
	    return;
	}

	SetClipArea(d, x1, y1, x2, y2);

	dphi = two_pi/499.;
	x = r;
	y = 0.;
	imove(d, x, y);
	for(i = 1; i <= 500; i++)
	{
	    phi = i*dphi;
	    x = r*cos(phi);
	    y = r*sin(phi);
	    idraw(d, x, y);
	}
	iflush(d);
	SetClipArea(d, 0., 0., 0., 0.);
}

static void
DrawGrid(MapPlotWidget w, DrawStruct *d, AxesParm *a, int dash_length)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, ndigit;
	double	xmin, xmax, ymin, ymax, xdif, ydif, x, y, del, xdel, ydel;
	double	lon1, lon2, lon3, lon4, lat1, lat2, lat3, lat4,
		lon, lat, lon_min, lon_max, lim=0.;
	double	lat_min, lat_max, ydash;

	Free(mp->grid.segs);
	mp->grid.size_segs = 0;
	mp->grid.nsegs = 0;
	d->s = &mp->grid;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];
	ymin = ax->y1[ax->zoom];
	ymax = ax->y2[ax->zoom];
	SetClipArea(d, xmin, ymin, xmax, ymax);

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
	   mp->projection ==MAP_UTM ||
	   mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	   mp->projection == MAP_MERCATOR)
	{
	    if( mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		mp->projection == MAP_MERCATOR)
	    {
		unproject(w, xmin, ymin, &lon, &lat1);
		unproject(w, xmin, ymax, &lon, &lat2);
		nicex(lat1, lat2, a->min_ylab, a->max_ylab, &a->nylab,
			a->y_lab_tran, &ndigit, &a->nydeci);

		for(i = 0; i < a->nylab; i++)
		{
		    project(w, xmin, a->y_lab_tran[i], &x, &a->y_lab[i], &del);
		}
		a->ysmall = 0;
		gdrawYAxis(a, d, 2, xmin, ymin, ymax, a->ytic, False, 2, 1, 0.);
		gdrawYAxis(a, d, 3, xmax, ymin, ymax, -a->ytic,False, 2, 0, 0.);
	    }
	    del = fabs(scale_x(d, dash_length) - scale_x(d, 0));
	    if(a->nxlab <= 1 || a->nylab <= 1)
	    {
		for(i = 0; i < a->nxlab; i++)
		{
		    grid_x(d, a->x_lab[i], ymin, ymax, del);
		}
		for(i = 0; i < a->nylab; i++)
		{
		    grid_y(d, a->y_lab[i], xmin, xmax, del);
		}
		iflush(d);
		return;
	    }
	    xdif = fabs(a->x_lab[1] - a->x_lab[0]);
	    ydif = (mp->projection == MAP_LINEAR_CYLINDRICAL ||
			mp->projection == MAP_UTM) ?
				fabs(a->y_lab[1] - a->y_lab[0]) :
				fabs(a->y_lab_tran[1] - a->y_lab_tran[0]);

	    if(xdif < ydif)
	    {
		for(i = 0; i < a->nxlab; i++)
		{
		    grid_x(d, a->x_lab[i], ymin, ymax, del);
		}
		for(y = a->y_lab[0]; y > ymin; y -= xdif)
		{
		    grid_y(d, y, xmin, xmax, del);
		}
		for(y = a->y_lab[0]+xdif; y < ymax; y += xdif)
		{
		    grid_y(d, y, xmin, xmax, del);
		}
	    }
	    else
	    {
		for(i = 0; i < a->nylab; i++)
		{
		    grid_y(d, a->y_lab[i], xmin, xmax, del);
		}
		for(x = a->x_lab[0]; x > xmin; x -= ydif)
		{
		    grid_x(d, x, ymin, ymax, del);
		}
		for(x = a->x_lab[0]+ydif; x < xmax; x += ydif)
		{
		    grid_x(d, x, ymin, ymax, del);
		}
	    }
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC ||
		mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
		mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    del = fabs(scale_x(d, dash_length) - scale_x(d, 0));
	    if(del > 1.) del = 1.;
	    del = asin(del)/rad;

	    unproject(w, 0., ymin, &lon1, &lat);
	    unproject(w, 0., ymax, &lon2, &lat);
	    reduce_lon(&lon1);
	    reduce_lon(&lon2);

	    if(mp->projection == MAP_ORTHOGRAPHIC)
	    {
		lim = 1.;
	    }
	    else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	    {
		lim = .95*pi;
	    }
	    else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	    {
		lim = .95*2.;
	    }
	    if( xmin*xmin + ymin*ymin > lim || xmin*xmin + ymax*ymax > lim ||
		xmax*xmax + ymin*ymin > lim || xmax*xmax + ymax*ymax > lim)
	    {
		a->axis_segs[0].n_segs = 0;
		a->axis_segs[1].n_segs = 0;
		a->axis_segs[2].n_segs = 0;
		a->axis_segs[3].n_segs = 0;
		a->nxlab = 0;
		a->nylab = 0;
		for(i = 0; i < 360; i += 20)
		{
		    grid_lon(w, d, (double)i, -90., 90., del, &x);
		}
		for(i = -80; i <= 80; i += 20)
		{
		    grid_lat(w, d, (double)i, -180., 180., del, &y);
		}
	    }
	    else if(fabs(lon1 - lon2) > 90.)
	    {
		/* a pole is inside limits
		 */
		unproject(w, xmin, ymin, &lon1, &lat1);
		unproject(w, xmax, ymin, &lon2, &lat3);
		unproject(w, xmin, ymax, &lon3, &lat2);
		unproject(w, xmax, ymax, &lon4, &lat4);
		if(lat1 > 0.)
		{
		    lat_min = (lat1 < lat2) ? lat1 : lat2;
		    if(lat_min > lat3) lat_min = lat3;
		    if(lat_min > lat4) lat_min = lat4;
		    lat_max = 90. - 4*del;
		    lon_min = lon1;
		    lon_max = lon2;
		}
		else
		{
		    gdrawAxis(a, d, xmin, xmax, TOP, ymin, ymax, ax->y_axis,
				ax->tickmarks_inside, ax->display_axes_labels,
				ax->time_scale, 1.);
		    SetClipArea(d, xmin, ymin, xmax, ymax);
		    lat_max = (lat1 > lat2) ? lat1 : lat2;
		    if(lat_max < lat3) lat_max = lat3;
		    if(lat_max < lat4) lat_max = lat4;
		    lat_min = -90. + 4*del;
		    lon_min = lon3;
		    lon_max = lon4;
		}
		reduce_lon(&lon_min);
		reduce_lon(&lon_max);
		if(lon_max < lon_min) lon_max += 360;
		nicex(lon_min, lon_max, a->min_xlab, a->max_xlab, &a->nxlab,
			a->x_lab_tran, &ndigit, &a->nxdeci);
		for(i = 0; i < a->nxlab; i++)
		{
		    grid_lon(w, d, a->x_lab_tran[i], lat_min, lat_max, del,
				&a->x_lab[i]);
		}
		a->xsmall = 0;
		if(lat1 > 0.)
		{
		    gdrawXAxis(a, d, 0, ymin, xmin, xmax, a->xtic, BOTTOM,
				False, 2, 1, 0.);
		}
		else
		{
		    gdrawXAxis(a, d, 0, ymax, xmin, xmax, a->xtic, TOP, False,
				2, 1, 0.);
		}
		if(a->nxlab > 1)
		{
		    xdel =  a->x_lab_tran[1] - a->x_lab_tran[0];
		    for(lon = a->x_lab_tran[0]-xdel; lon >= -180.; lon -= xdel)
		    {
			grid_lon(w, d, lon, lat_min, lat_max, del, &x);
		    }
		    for(lon = a->x_lab_tran[a->nxlab-1]+xdel;
					lon <= 180.; lon += xdel)
		    {
			grid_lon(w, d, lon, lat_min, lat_max, del, &x);
		    }
		}
		nicex(lat_min, lat_max, a->min_ylab, a->max_ylab, &a->nylab,
			a->y_lab_tran, &ndigit, &a->nydeci);

		for(i = 0; i < a->nylab; i++)
		{
		    ydash=del/sin((90.-fabs(a->y_lab_tran[i]))*rad);
		    grid_lat(w, d, a->y_lab_tran[i], -180., 180., ydash,
				&a->y_lab[i]);
		}
		a->nylab = 0;
		a->axis_segs[1].n_segs = 1;
		a->axis_segs[2].n_segs = 1;
		a->axis_segs[3].n_segs = 1;
	    }
	    else
	    {
		unproject(w, xmin, ymin, &lon1, &lat1);
		unproject(w, xmin, ymax, &lon, &lat2);
		unproject(w, xmax, ymin, &lon2, &lat);
		reduce_lon(&lon1);
		reduce_lon(&lon2);
		if(fabs(lat1) < fabs(lat2))
		{
		    unproject(w, xmin, ymax, &lon_min, &lat);
		    unproject(w, xmax, ymax, &lon_max, &lat);
		    reduce_lon(&lon_min);
		    reduce_lon(&lon_max);
		    lat_min = lat1;
		    unproject(w, 0., ymax, &lon, &lat_max);
		}
		else
		{
		    lon_min = lon1;
		    lon_max = lon2;
		    lat_max = lat2;
		    unproject(w, 0., ymin, &lon, &lat_min);
		}
		if(lon_max < lon_min) lon_max += 360;
		if(lon2 < lon1) lon2 += 360;
		nicex(lon1, lon2, a->min_xlab, a->max_xlab, &a->nxlab,
			a->x_lab_tran, &ndigit, &a->nxdeci);

		for(i = 0; i < a->nxlab; i++)
		{
		    grid_lon(w, d, a->x_lab_tran[i], lat_min, lat_max, del,
				&a->x_lab[i]);
		}
		a->xsmall = 0;
		gdrawXAxis(a, d, 0, ymin, xmin, xmax, a->xtic, BOTTOM, False,
				2, 1, 0.);
		if(a->nxlab > 1)
		{
		    xdel = a->x_lab_tran[1] - a->x_lab_tran[0];
		    for(lon = a->x_lab_tran[0]-xdel; lon > lon_min; lon -= xdel)
		    {
			grid_lon(w, d, lon, lat_min, lat_max, del, &x);
		    }
		    for(lon = a->x_lab_tran[a->nxlab-1]+xdel;
				lon < lon_max; lon += xdel)
		    {
			grid_lon(w, d, lon, lat_min, lat_max, del, &x);
		    }
		}
		nicex(lat1, lat2, a->min_ylab, a->max_ylab, &a->nylab,
			a->y_lab_tran, &ndigit, &a->nydeci);

		for(i = 0; i < a->nylab; i++)
		{
		    ydash=del/sin((90.-fabs(a->y_lab_tran[i]))*rad);
		    grid_lat(w, d, a->y_lab_tran[i], lon_min, lon_max, ydash,
				&a->y_lab[i]);
		}
		a->ysmall = 0;
		gdrawYAxis(a, d, 2, xmin, ymin, ymax, a->ytic, False, 2, 1, 0.);
		if(a->nylab > 1)
		{
		    ydel = a->y_lab_tran[1] - a->y_lab_tran[0];
		    for(lat = a->y_lab_tran[0]-ydel; lat > lat_min; lat -= ydel)
		    {
			ydash = del/sin((90.-fabs(lat))*rad);
			grid_lat(w, d, lat, lon_min, lon_max, ydash,&y);
		    }
		    for(lat = a->y_lab_tran[a->nylab-1]+ydel;
				lat < lat_max; lat += ydel)
		    {
			ydash = del/sin((90.-fabs(lat))*rad);
			grid_lat(w, d, lat, lon_min, lon_max, ydash,&y);
		    }
		}
		a->axis_segs[1].n_segs = 1;
		a->axis_segs[3].n_segs = 1;
	    }
	}
	iflush(d);
}

static void
grid_x(DrawStruct *d, double x, double ymin, double ymax, double del)
{
	double	y;

	for(y = ymin+4*del; y < ymax; y += 5*del)
	{
	    imove(d, x, y);
	    idraw(d, x, y+del);
	}
}

static void
grid_y(DrawStruct *d, double y, double xmin, double xmax, double del)
{
	double	x;

	for(x = xmin+4*del; x < xmax; x += 5*del)
	{
	    imove(d, x, y);
	    idraw(d, x+del, y);
	}
}

static void
grid_lon(MapPlotWidget w, DrawStruct *d, double lon, double min_lat,
		double max_lat, double del, double *x_lab)
{
	Boolean	first = True;
	double	lat, x1, y1, z1, x2, y2, z2, dl;

	dl = (max_lat - min_lat)/1000.;
	for(lat = min_lat; lat < max_lat && dl != 0.; lat += dl)
	{
	    project(w, lon, lat, &x1, &y1, &z1);

	    if(z1 > 0.)
	    {
		imove(d, x1, y1);
		if(d->lastin && first)
		{
		    *x_lab = x1;
		    break;
		}
	    }
	}
	for(lat = min_lat + 4*del; lat < max_lat; lat += 5*del)
	{
	    project(w, lon, lat, &x1, &y1, &z1);
	    project(w, lon, lat+del, &x2, &y2, &z2);

	    if(z1 > 0. && z2 > 0.)
	    {
		imove(d, x1, y1);
		idraw(d, x2, y2);
	    }
	}
}

static void
grid_lat(MapPlotWidget w, DrawStruct *d, double lat, double min_lon,
		double max_lon, double del, double *y_lab)
{
	Boolean	first = True;
	double	lon, x1, y1, z1, x2, y2, z2, dl;

	dl = (max_lon - min_lon)/1000.;
	for(lon = min_lon; lon < max_lon && dl != 0.; lon += dl)
	{
	    project(w, lon, lat, &x1, &y1, &z1);

	    if(z1 > 0.)
	    {
		imove(d, x1, y1);
		if(d->lastin && first)
		{
		    *y_lab = y1;
		    break;
		}
	    }
	}
	for(lon = min_lon + 4*del; lon < max_lon; lon += 5*del)
	{
	    project(w, lon, lat, &x1, &y1, &z1);
	    project(w, lon+del, lat, &x2, &y2, &z2);

	    if(z1 > 0. && z2 > 0.)
	    {
		imove(d, x1, y1);
		idraw(d, x2, y2);
	    }
	}
}

static void
MapProjection(MapPlotWidget cur, MapPlotWidget nou)
{
	MapPlotPart *cur_mp = &cur->map_plot;
	MapPlotPart *new_mp = &nou->map_plot;
	AxesPart *cur_ax = &cur->axes;
	AxesPart *new_ax = &nou->axes;
	int	i;
	double	lon, lat, lon1, lat1, lon2, lat2, lon_dif, lat_dif;
	double	xdif, ydif, x0, y0, z0;

	if(cur_mp->projection == new_mp->projection)
	{
	    return;
	}

	SetZoomZero(nou);

	i = cur_ax->zoom;
	if(new_mp->projection == MAP_LINEAR_CYLINDRICAL ||
	   new_mp->projection == MAP_UTM ||
	   new_mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	   new_mp->projection == MAP_MERCATOR)
	{
	    if(cur_mp->projection == MAP_ORTHOGRAPHIC)
	    {
		if(cur_ax->x1[i]*cur_ax->x1[i] +
		 cur_ax->y1[i]*cur_ax->y1[i] > 1. ||
		 cur_ax->x1[i]*cur_ax->x1[i] +
		 cur_ax->y2[i]*cur_ax->y2[i] > 1. ||
		 cur_ax->x2[i]*cur_ax->x2[i] +
		 cur_ax->y1[i]*cur_ax->y1[i] > 1. ||
		 cur_ax->x2[i]*cur_ax->x2[i] +
		 cur_ax->y2[i]*cur_ax->y2[i] > 1.)
		{
		    new_ax->zoom = 0;
		}
		else
		{
		    unproject(cur, cur_ax->x1[i],cur_ax->y1[i],&lon1,&lat1);
		    unproject(cur, cur_ax->x2[i],cur_ax->y1[i],&lon2,&lat1);
		    reduce_lon(&lon1);
		    reduce_lon(&lon2);
		    if(lon2 < lon1) lon2 += 360;
		    lon_dif = lon2 - lon1;
		    unproject(cur, cur_ax->x1[i],cur_ax->y2[i],&lon1,&lat2);
		    unproject(cur, cur_ax->x2[i],cur_ax->y2[i],&lon2,&lat2);
		    reduce_lon(&lon1);
		    reduce_lon(&lon2);
		    if(lon2 < lon1) lon2 += 360;
		    if(lon2 - lon1 > lon_dif) lon_dif = lon2 - lon1;
		    if(lon_dif > new_ax->x2[0] - new_ax->x1[0])
		    {
			new_ax->zoom = 0;
		    }
		    else
		    {
			new_ax->x1[1] = cur_mp->phi - .5*lon_dif;
			new_ax->x2[1] = cur_mp->phi + .5*lon_dif;

			lat_dif = (new_ax->y2[0] - new_ax->y1[0])*lon_dif/
				(new_ax->x2[0] - new_ax->x1[0]);

			if(cur_mp->theta - .5*lat_dif < 0.)
			{
			    new_ax->y2[1] = 90.;
			    new_ax->y1[1] = 90. - lat_dif;
			}
			else if(cur_mp->theta + .5*lat_dif > 180.)
			{
			    new_ax->y1[1] = -90.;
			    new_ax->y2[1] = -90. + lat_dif;
			}
			else
			{
			    new_ax->y1[1] = 90. - cur_mp->theta -
						.5*lat_dif;
			    new_ax->y2[1] = 90. - cur_mp->theta +
						.5*lat_dif;
			}
			new_ax->zoom = 1;
	   		if(new_mp->projection == MAP_MERCATOR)
			{
			    if(fabs(new_ax->y1[1]) >
					new_mp->mercator_max_lat ||
				fabs(new_ax->y2[1]) >
					new_mp->mercator_max_lat)
			    {
				new_ax->zoom = 0;
			    }
			    else
			    {
				project(nou, 0., new_ax->y1[1], &x0,
					&new_ax->y1[1], &z0);
				project(nou, 0., new_ax->y2[1], &x0,
					&new_ax->y2[1], &z0);
			    }
			}
	   		else if(new_mp->projection ==
				MAP_CYLINDRICAL_EQUAL_AREA)
			{
			    if(fabs(new_ax->y1[1]) >
					new_mp->mercator_max_lat ||
				fabs(new_ax->y2[1]) >
					new_mp->mercator_max_lat)
			    {
				new_ax->zoom = 0;
			    }
			    else
			    {
				project(nou, 0., new_ax->y1[1], &x0,
					&new_ax->y1[1], &z0);
				project(nou, 0., new_ax->y2[1], &x0,
					&new_ax->y2[1], &z0);
			    }
			}
		    }
		}
	    }
	    else if(new_ax->zoom > 0 && (
		cur_mp->projection == MAP_LINEAR_CYLINDRICAL ||
		cur_mp->projection == MAP_UTM) )
	    {
		if(fabs(cur_ax->y1[i]) > new_mp->mercator_max_lat ||
		   fabs(cur_ax->y2[i]) > new_mp->mercator_max_lat)
		{
		    new_ax->zoom = 0;
		}
		else
		{
		    new_ax->zoom = 1;
		    project(nou, 0., cur_ax->y1[i], &x0,
			&new_ax->y1[1], &z0);
		    project(nou, 0., cur_ax->y2[i], &x0,
			&new_ax->y2[1], &z0);
		}
	    }
	    else if(new_ax->zoom > 0 &&
		cur_mp->projection == MAP_CYLINDRICAL_EQUAL_AREA)
	    {
		if(fabs(cur_ax->y1[i]) > new_mp->mercator_max_lat ||
		   fabs(cur_ax->y2[i]) > new_mp->mercator_max_lat)
		{
		    new_ax->zoom = 0;
		}
		else
		{
		    new_ax->zoom = 1;
		    project(nou, 0., cur_ax->y1[i], &x0,
			&new_ax->y1[1], &z0);
		    project(nou, 0., cur_ax->y2[i], &x0,
			&new_ax->y2[1], &z0);
		}
	    }
	    else if(new_ax->zoom > 0 &&
	    	cur_mp->projection == MAP_MERCATOR)
	    {
	        new_ax->zoom = 1;
		unproject(cur, cur_ax->x1[i],cur_ax->y1[i],&new_ax->x1[1],
			&new_ax->y1[1]);
		unproject(cur, cur_ax->x2[i],cur_ax->y2[i],&new_ax->x2[1],
			&new_ax->y2[1]);
	    }
	}
	else if(new_ax->zoom == 0)
	{
	    if( cur_mp->projection != MAP_ORTHOGRAPHIC &&
		cur_mp->projection != MAP_AZIMUTHAL_EQUIDISTANT &&
		cur_mp->projection != MAP_AZIMUTHAL_EQUAL_AREA)
	    {
		MapRotate(nou, 10., 25.);
	    }
	}
	else
	{
	    x0 = .5*(cur_ax->x1[i] + cur_ax->x2[i]);
	    y0 = .5*(cur_ax->y1[i] + cur_ax->y2[i]);

	    unproject(cur, x0, y0, &lon, &lat);
	    MapRotate(nou, lon, lat);

	    xdif = .5*(cur_ax->x2[i] - cur_ax->x1[i]);
	    ydif = .5*(cur_ax->y2[i] - cur_ax->y1[i]);
	    if( cur_mp->projection == MAP_LINEAR_CYLINDRICAL ||
		cur_mp->projection == MAP_UTM ||
		cur_mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
		cur_mp->projection == MAP_MERCATOR)
	    {
		if(new_mp->projection == MAP_ORTHOGRAPHIC)
		{
		    xdif = (xdif > 0. && xdif < 90.) ? sin(xdif*rad) : 1.;
		    ydif = (ydif > 0. && ydif < 90.) ? sin(ydif*rad) : 1.;
		}
		else if(new_mp->projection ==MAP_AZIMUTHAL_EQUAL_AREA)
		{
		    xdif = (xdif > 0. && xdif < 90.) ? sin(xdif*rad) : pi;
		    ydif = (ydif > 0. && ydif < 90.) ? sin(ydif*rad) : pi;
		}
		else
		{
		    xdif = (xdif > 0. && xdif < 90.) ? sin(xdif*rad) : 2.;
		    ydif = (ydif > 0. && ydif < 90.) ? sin(ydif*rad) : 2.;
		}
	    }
	    new_ax->x1[1] = -xdif;
	    new_ax->x2[1] =  xdif;
	    new_ax->y1[1] = -ydif;
	    new_ax->y2[1] =  ydif;
	    new_ax->zoom = 1;
	}

	/* convert crosshair coordinates from current to new system */
	for(i = 0; i < cur_ax->num_cursors; i++)
	{
	    unproject(cur, cur_ax->cursor[i].scaled_x,
				cur_ax->cursor[i].scaled_y, &lon, &lat);
	    project(nou, lon, lat, &x0, &y0, &z0);

	    if(z0 > 0.)
	    {
		new_ax->cursor[i].scaled_x = x0;
		new_ax->cursor[i].scaled_y = y0;
	    }
	    else
	    {
		new_ax->cursor[i].scaled_x = 0.;
		new_ax->cursor[i].scaled_y = 0.;
	    }
	}
}

void
MapPlotRotation(MapPlotWidget w)
{
	AxesPart *ax = &w->axes;
	double lon, lat;

	if(ax->num_cursors <= 0)
	{
	    _AxesWarn((AxesWidget)w, "Position a crosshair on the map.");
	    return;
	}
	/* first crosshair denotes new z-axis location.
	 */
	unproject(w, ax->cursor[0].scaled_x, ax->cursor[0].scaled_y,&lon,&lat);

	AxesWaitCursor((Widget)w, True);
	MapPlotRotate(w, lon, lat, True);
	AxesWaitCursor((Widget)w, False);
}

void
MapPlotRotateToSta(MapPlotWidget w)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i, n;
	MapStation	*sta=NULL;

	for(i = n = 0; i < mp->nsta; i++)
	{
	    if(mp->sta[i]->station.sym.selected)
	    {
		sta = mp->sta[i];
		n++;
	    }
	}
	if(n == 1)
	{
	    AxesWaitCursor((Widget)w, True);
	    MapPlotRotate(w, sta->station.lon, sta->station.lat, True);
	    AxesWaitCursor((Widget)w, False);
	}
	else if(n == 0)
	{
	    _AxesWarn((AxesWidget)w, "No stations selected.");
	}
	else if(n > 1)
	{
	    _AxesWarn((AxesWidget)w, "More than one station selected.");
	}
}

void
MapPlotRotateToStation(MapPlotWidget w, char *sta)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;

	for(i = 0; i < mp->nsta; i++)
	{
	    if(!strcasecmp(sta, mp->sta[i]->station.label))
	    {
		MapPlotRotate(w, mp->sta[i]->station.lon,
			mp->sta[i]->station.lat, True);
		break;
	    }
	}
}

void
MapPlotRotateToOrig(MapPlotWidget w)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i, n;
	MapSource	*src=NULL;

	for(i = n = 0; i < mp->nsrc; i++)
	{
	    if(mp->src[i]->source.sym.selected)
	    {
		src = mp->src[i];
		n++;
	    }
	}
	if(n == 1)
	{
	    AxesWaitCursor((Widget)w, True);
	    MapPlotRotate(w, src->source.lon, src->source.lat, True);
	    AxesWaitCursor((Widget)w, False);
	}
	else if(n == 0)
	{
	    _AxesWarn((AxesWidget)w, "No sources selected.");
	}
	else if(n > 1)
	{
	    _AxesWarn((AxesWidget)w, "More than one source selected.");
	}
}

static void
RotateToCursor(MapPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params)
{
	AxesPart *ax = &w->axes;
	int	cursor_x, cursor_y;
	double	lon, lat;

	cursor_x = ((XButtonEvent *)event)->x;
	cursor_y = ((XButtonEvent *)event)->y;

	unproject(w, scale_x(&ax->d, cursor_x),
			scale_y(&ax->d, cursor_y), &lon, &lat);

	AxesWaitCursor((Widget)w, True);
	MapPlotRotate(w, lon, lat, True);
	AxesWaitCursor((Widget)w, False);
}

void
MapPlotRotate(MapPlotWidget w, double lon, double lat, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	MapRotate(w, lon, lat);

	_AxesRedraw((AxesWidget)w);
	_MapPlotRedraw(w);
	RedisplayThemes(w);
	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	RedisplayOverlays(w, ax->pixmap);
	
	if(redisplay && !ax->redisplay_pending)
	{
	    _AxesRedisplayAxes((AxesWidget)w);
	    RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	    _AxesRedisplayXor((AxesWidget)w);
	}
}

static void
MapRotate(MapPlotWidget w, double longitude, double latitude)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i;
	double	lon, lat, alpha, beta, gamma;
	double	x1, x2, x, y, z;

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
	   mp->projection == MAP_UTM ||
	   mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	   mp->projection == MAP_MERCATOR)
	{
	    mp->phi = longitude;
	    reduce_lon(&mp->phi);

	    for(i = 0; i <= ax->zoom; i++)
	    {
		x1 = ax->x1[i];
		x2 = ax->x2[i];
		ax->x1[i] = mp->phi - .5*(x2-x1);
		ax->x2[i] = mp->phi + .5*(x2-x1);
	    }
	    for(i = 0; i < ax->num_cursors; i++)
	    {
		if(ax->cursor[i].scaled_x < ax->x1[0])
		{
		    ax->cursor[i].scaled_x += 360.;
		}
		else if(ax->cursor[i].scaled_x > ax->x2[0])
		{
		    ax->cursor[i].scaled_x -= 360.;
		}
	    }
	}
	else
	{
	    mp->theta = 90. - latitude;
	    mp->phi = longitude;

	    for(i = 0; i < ax->num_cursors; i++)
	    {
		/* get crosshair coordinates in the original system.
		 */
		unproject(w, ax->cursor[i].scaled_x, ax->cursor[i].scaled_y,
				&lon, &lat);
		ax->cursor[i].scaled_x = lon;
		ax->cursor[i].scaled_y = lat;
	    }
	    /*
	     * alpha, beta, gamma from original to new system.
	     */
	    alpha = mp->phi*rad;
	    beta = mp->theta*rad;
	    gamma = (beta > 0) ? half_pi : -half_pi;

	    rotation_matrix(-gamma, -beta, -alpha, mp->c);
	    rotation_matrix(alpha, beta, gamma, mp->d);

	    /* convert crosshair coordinates from original to new system 
	     */
	    for(i = 0; i < ax->num_cursors; i++)
	    {
		project(w, ax->cursor[i].scaled_x, ax->cursor[i].scaled_y,
			&x, &y, &z);

		if(z <= 0.)
		{
		    /* move this crosshair to 0.,0.
		     */
		    ax->cursor[i].scaled_x = 0.;
		    ax->cursor[i].scaled_y = 0.;
		}
		else
		{
		    ax->cursor[i].scaled_x = x;
		    ax->cursor[i].scaled_y = y;
		}
	    }
	}
}

Boolean
MapPlotGetCoordinates(MapPlotWidget w, double *lat, double *lon, double *delta,
			double *azimuth)
{
	int i;
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	if(w == NULL || !XtIsRealized((Widget)w)) return(0);

	if(ax->num_cursors <= 0 || ax->cursor[0].type != AXES_CROSSHAIR ||
	    ax->cursor[0].xlab[0] == '\0' || ax->cursor[0].ylab[0] == '\0')
	{
	    return(False);
	}
	unproject(w, ax->cursor[0].scaled_x, ax->cursor[0].scaled_y, lon, lat);

	for(i = 0; i < mp->ndel; i++) {
	    if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
		    mp->del[i]->sta_id == 0) break;
	}
	if(i < mp->ndel) {
	    *delta = mp->del[i]->delta.del;
	}

	for(i = 0; i < mp->narc; i++) {
	    if(mp->arc[i]->type == MAP_CURSOR_MEASURE &&
		    mp->arc[i]->sta_id == 0) break;
	}
	if(i < mp->narc) {
	    *azimuth = mp->arc[i]->arc.az;
	}

	return(True);
}

Boolean
MapPlotPositionCrosshairDA(MapPlotWidget w, double lat, double lon,
				double delta, double azimuth)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double x, y, z;
	int i;

	if(w == NULL || !XtIsRealized((Widget)w)) return False;

	project(w, lon, lat, &x, &y, &z);
	if(z < 0.) return False;

	for(i = 0; i < mp->ndel; i++) {
	    if(mp->del[i]->type == MAP_CURSOR_MEASURE &&
		    mp->del[i]->sta_id == 0) break;
	}
	if(i < mp->ndel) {
	    MapDelta *d = mp->del[i];
	    d->delta.del = delta;
	    Free(d->s.segs);
	    d->s.size_segs = 0;
	    d->s.nsegs = 0;
	    ax->d.s = &d->s;
	    MakeDelta(w, d);
	    DrawDelta(w, &ax->d, d);
	    RedisplayLine(w, ax->pixmap, &d->delta.line, &d->s);
	}

	for(i = 0; i < mp->narc; i++) {
	    if(mp->arc[i]->type == MAP_CURSOR_MEASURE &&
		    mp->arc[i]->sta_id == 0) break;
	}
	if(i < mp->narc) {
	    MapArc *a = mp->arc[i];
	    a->arc.az = azimuth;
	    MakeArc(w, a);
	    DrawArc(w, &ax->d, a);
	    RedisplayLine(w, ax->pixmap, &a->arc.line, &a->s);
	}

	if(ax->num_cursors <= 0) {
	    ax->axes_class->addCrosshair();
	}
	return !ax->axes_class->positionCrosshair(0, x, y, true) ? True : False;
}

Boolean
MapPlotPositionCrosshair(MapPlotWidget w, double lat, double lon)
{
	return MapPlotPositionCrosshair2(w, lat, lon, True);
}

Boolean
MapPlotPositionCrosshair2(MapPlotWidget w, double lat, double lon,bool callback)
{
	AxesPart *ax = &w->axes;
	double x, y, z;
	if(w == NULL || !XtIsRealized((Widget)w)) return False;

	project(w, lon, lat, &x, &y, &z);
	if(z < 0.) return False;

	if(ax->num_cursors <= 0) {
	    ax->axes_class->addCrosshair();
	}
	return !ax->axes_class->positionCrosshair(0, x, y, callback)
			? True : False;
}

static void
XYTransform(AxesWidget widget, double x, double y, char *xlab, char *ylab)
{
	MapPlotWidget	w = (MapPlotWidget)widget;
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double lon, lat;

	xlab[0] = '\0';
	ylab[0] = '\0';

	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    gdrawCVlon(x, ax->a.nxdeci+2, xlab, 20);
	    gdrawCVlat(y, ax->a.nydeci+2, ylab, 20);
	    if(xlab[0] != '\0') strcat(xlab, " ");
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	        mp->projection == MAP_MERCATOR)
	{
	    unproject(w, x, y, &lon, &lat);
	    gdrawCVlon(lon, ax->a.nxdeci+2, xlab, 20);
	    gdrawCVlat(lat, ax->a.nydeci+2, ylab, 20);
	    if(xlab[0] != '\0') strcat(xlab, " ");
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC)
	{
	    if(x*x + y*y < 1.)
	    {
		unproject(w, x, y, &lon, &lat);
		gdrawCVlon(lon, ax->a.nxdeci+2, xlab, 20);
		gdrawCVlat(lat, ax->a.nydeci+2, ylab, 20);
	    }
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	{
	    if(sqrt(x*x + y*y) < .95*pi)
	    {
		unproject(w, x, y, &lon, &lat);
		gdrawCVlon(lon, ax->a.nxdeci+2, xlab, 20);
		gdrawCVlat(lat, ax->a.nydeci+2, ylab, 20);
	    }
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    if(sqrt(x*x + y*y) < .95*2.)
	    {
		unproject(w, x, y, &lon, &lat);
		gdrawCVlon(lon, ax->a.nxdeci+2, xlab, 20);
		gdrawCVlat(lat, ax->a.nydeci+2, ylab, 20);
	    }
	}
	else if(mp->projection == MAP_POLAR)
	{
	    int ndeci;
	    double r, a, lg;
	    a = atan2(x, y)/DEG_TO_RADIANS;
	    r = sqrt(x*x + y*y);
	    lg = log10(fabs(r));
	    ndeci = (int)((lg-4 > 0) ? 0 : -lg+4);
	    ftoa(r, ndeci, 1, xlab, 20);
	    strcat(xlab, " radius");
	    if(a < 0.) a += 360.;
	    lg = log10(fabs(a));
	    ndeci = (int)((lg-4 > 0) ? 0 : -lg+4);
//	    ftoa(a, ndeci, 1, ylab, 20);
	    ftoa(a, 1, 1, ylab, 20);
	    strcat(ylab, " az");
	}
	else if(mp->projection == MAP_UTM_NEAR)
	{
	    snprintf(xlab, 20, "%.0f", x);
	    snprintf(ylab, 20, "%.0f", y);
	}
}

static void
SetZoomZero(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double	theta, phi, lat, fac, lim;

	phi = .5*(LON_MIN + LON_MAX);
	theta = .5*(LAT_MIN + LAT_MAX);
	mp->theta = theta;
	mp->phi = phi;
	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    ax->x1[0] = LON_MIN;
	    ax->x2[0] = LON_MAX;
	    ax->y1[0] = LAT_MIN;
	    ax->y2[0] = LAT_MAX;
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA)
	{
	    ax->x1[0] = LON_MIN;
	    ax->x2[0] = LON_MAX;
	    lat = LAT_MIN;
	    if(lat < -mp->mercator_max_lat)
	    {
		lat = -mp->mercator_max_lat;
	    }
	    else if(lat > mp->mercator_max_lat)
	    {
		lat = mp->mercator_max_lat;
	    }
	    ax->y1[0] = sin(lat*rad)/rad;
	    lat = LAT_MAX;
	    if(lat < -mp->mercator_max_lat)
	    {
		lat = -mp->mercator_max_lat;
	    }
	    else if(lat > mp->mercator_max_lat)
	    {
		lat = mp->mercator_max_lat;
	    }
	    ax->y2[0] = sin(lat*rad)/rad;
	}
	else if(mp->projection == MAP_MERCATOR)
	{
	    ax->x1[0] = LON_MIN;
	    ax->x2[0] = LON_MAX;
	    lat = LAT_MIN;
	    if(lat < -mp->mercator_max_lat)
	    {
		lat = -mp->mercator_max_lat;
	    }
	    else if(lat > mp->mercator_max_lat)
	    {
		lat = mp->mercator_max_lat;
	    }
	    ax->y1[0] = log(tan(.5*lat*rad + M_PI_4))/rad;
	    lat = LAT_MAX;
	    if(lat < -mp->mercator_max_lat)
	    {
		lat = -mp->mercator_max_lat;
	    }
	    else if(lat > mp->mercator_max_lat)
	    {
		lat = mp->mercator_max_lat;
	    }
	    ax->y2[0] = log(tan(.5*lat*rad + M_PI_4))/rad;
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC ||
	        mp->projection == MAP_AZIMUTHAL_EQUIDISTANT ||
	        mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
/**
	    xdif = .5*(LON_MAX - LON_MIN);
	    ydif = .5*(LAT_MAX - LAT_MIN);
	    xdif = (xdif > 0. && xdif < 90.) ? sin(xdif*rad) : 1.02;
	    ydif = (ydif > 0. && ydif < 90.) ? sin(ydif*rad) : 1.02;
	    ax->x1[0] = -xdif;
	    ax->x2[0] =  xdif;
	    ax->y1[0] = -ydif;
	    ax->y2[0] =  ydif;
**/
	    if(mp->projection == MAP_ORTHOGRAPHIC)
	    {
		lim = 1.02;
	    }
	    else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	    {
		lim = pi;
	    }
	    else
	    {
		lim = 2.;
	    }
	    if(w->core.width > w->core.height)
	    {
		fac = (double)w->core.width/w->core.height;
		ax->y1[0] = -lim;
		ax->y2[0] =  lim;
		ax->x1[0] = -.5*fac*(ax->y2[0] - ax->y1[0]);
		ax->x2[0] =  .5*fac*(ax->y2[0] - ax->y1[0]);
	    }
	    else
	    {
		fac = (double)w->core.height/w->core.width;
		ax->x1[0] = -lim;
		ax->x2[0] =  lim;
		ax->y1[0] = -.5*fac*(ax->x2[0] - ax->x1[0]);
		ax->y2[0] =  .5*fac*(ax->x2[0] - ax->x1[0]);
	    }
	}
}

static void
unproject(MapPlotWidget w, double xp, double yp, double *lon, double *lat)
{
	MapPlotPart *mp = &w->map_plot;
	double d, x, y, z, zp=0., theta, phi, sn;
	/* find lat, lon from the projection
	 */
	if(mp->projection == MAP_LINEAR_CYLINDRICAL || mp->projection ==MAP_UTM)
	{
	    *lon = xp;
	    *lat = yp;
	    return;
	}
	else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA)
	{
	    *lon = xp;
	    *lat = asin(yp*rad)/rad;
	    return;
	}
	else if(mp->projection == MAP_MERCATOR)
	{
	    *lon = xp;
	    *lat = 2.*(atan(exp(yp*rad)) - M_PI_4)/rad;
	    return;
	}
	else if(mp->projection == MAP_UTM_NEAR)
	{
	    utmToLL(mp, xp, yp, lon, lat);
	    return;
	}
	else if(mp->projection == MAP_ORTHOGRAPHIC)
	{
	    if((d = xp*xp + yp*yp) > 1.)
	    {
		d = sqrt(d);
		xp /= d;
		yp /= d;
		zp = 0.;
	    }
	    else
	    {
		zp = sqrt(1. - xp*xp - yp*yp);
	    }
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT)
	{
	    if((d = sqrt(xp*xp + yp*yp)) > pi)
	    {
		*lon = 0.;
		*lat = -90.;
		return;
	    }
	    else
	    {
		theta = d;
		phi = atan2(yp, xp);
		sn = sin(theta);
		xp = sn*cos(phi);
		yp = sn*sin(phi);
		zp = cos(theta);
	    }
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA)
	{
	    if((d = sqrt(xp*xp + yp*yp)) > 2.)
	    {
		*lon = 0.;
		*lat = -90.;
		return;
	    }
	    else
	    {
		theta = 2.*asin(d/2.);
		phi = atan2(yp, xp);
		sn = sin(theta);
		xp = sn*cos(phi);
		yp = sn*sin(phi);
		zp = cos(theta);
	    }
	}
	x = mp->d[0][0]*xp + mp->d[0][1]*yp + mp->d[0][2]*zp;
	y = mp->d[1][0]*xp + mp->d[1][1]*yp + mp->d[1][2]*zp;
	z = mp->d[2][0]*xp + mp->d[2][1]*yp + mp->d[2][2]*zp;

	*lon = atan2(y, x)/rad;
	*lat = 90. - atan2(sqrt(x*x + y*y), z)/rad;
}

int
MapPlotAddStation(MapPlotWidget w, MapPlotStation *station, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	MapStation  *sta;
	MapStation map_station_init = MAP_STATION_INIT;

	if(w == NULL) return(-1);

	ALLOC((mp->nsta+1)*sizeof(MapStation *),
		mp->size_sta, mp->sta, MapStation *, 0);

	if( !(mp->sta[mp->nsta] = (MapStation *)AxesMalloc((Widget)w,
		sizeof(MapStation))) ) return(-2);

	sta = mp->sta[mp->nsta];
	mp->nsta++;

	memcpy(sta, &map_station_init, sizeof(MapStation));

	memcpy(&sta->station, station, sizeof(MapPlotStation));
	sta->station.id = getMapId(mp);
	if(station->label != NULL)
	{
	    sta->station.label = strdup(station->label);
	}
	sta->placement.tag_loc = station->tag_loc;

	if(redisplay)
	{
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = sta->station.id;
	    MapPlotAddStation(mp->utm_map, station, redisplay);
	    utm_mp->next_id = -1;
	}
	return(sta->station.id);
}

int
MapPlotGetStations(MapPlotWidget w, MapPlotStation **sta)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*sta = (MapPlotStation *)AxesMalloc((Widget)w,
		mp->nsta*sizeof(MapPlotStation))) ) return(0);

	for(i = 0; i < mp->nsta; i++)
	{
	    (*sta)[i] = mp->sta[i]->station;
	}
	return(mp->nsta);
}

static int
MapPlotChangeStation(MapPlotWidget w, MapPlotStation *station,Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i, j;
	double		slat, slon, rlat, rlon, baz;
	MapStation	*s;
	MapArc		*a;
	MapDelta	*d;

	for(i = 0; i < mp->nsta; i++)
	{
	    if(mp->sta[i]->station.id == station->id) break;
	}
	if(i == mp->nsta) return(0);

	s = mp->sta[i];
	if(s->station.label != station->label)
	{
	    Free(s->station.label);
	    memcpy(&s->station, station, sizeof(MapPlotStation));
	    if(station->label != NULL)
	    {
		s->station.label = strdup(station->label);
	    }
	}
	else
	{
	    memcpy(&s->station, station, sizeof(MapPlotStation));
	}

	for(i = 0; i < mp->narc; i++)
	{
	    a = mp->arc[i];
	    if(a->sta_id == station->id &&
		(station->lat != a->arc.lat || station->lon != a->arc.lon))
	    {
		if(a->type == MAP_PATH)
		{
	 	    for(j = 0; j < mp->nsrc; j++)
		    {
			 if(a->src_id == mp->src[j]->source.id) break;
		    }
		    slat = mp->src[j]->source.lat;
		    slon = mp->src[j]->source.lon;
		    rlat = s->station.lat;
		    rlon = s->station.lon;
		    deltaz(slat, slon, rlat, rlon, &a->arc.del,&a->arc.az,&baz);
		}
		else
		{
		    a->arc.lat = s->station.lat;
		    a->arc.lon = s->station.lon;
		}
		MapPlotChangeArc(w, &a->arc, False);
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	{
	    d = mp->del[i];
	    if(d->sta_id == station->id &&
		 (station->lat != d->delta.lat || station->lon != d->delta.lon))
	    {
		d->delta.lat = station->lat;
		d->delta.lon = station->lon;
		MapPlotChangeDelta(w, &d->delta, False);
	    }
	}

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

static int
MapPlotDeleteStation(MapPlotWidget w, int id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j;

	if(w == NULL) return(0);

	for(i = 0; i < mp->nsta; i++)
	{	
	    if(mp->sta[i]->station.id == id) break;
	}
	if(i == mp->nsta) return(0);

	DeleteStation(w, i);
	
	/* delete all arcs and deltas associated with this station
	 */
	for(j = mp->narc-1; j >= 0; j--)
	{
	    if(mp->arc[j]->sta_id == id)
	    {
		DeleteArc(w, j);
	    }
	}
	for(j = mp->ndel-1; j >= 0; j--)
	{
	    if(mp->del[j]->sta_id == id)
	    {
		DeleteDelta(w, j);
	    }
	}
	if(redisplay && !ax->redisplay_pending)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	return(1);
}

int
MapPlotAddSource(MapPlotWidget w, MapPlotSource *source, Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	MapSource	*src;
	MapPlotEllipse	ell;
	LineInfo line_info_init = LINE_INFO_INIT;
	MapSource map_source_init = MAP_SOURCE_INIT;

	ALLOC((mp->nsrc+1)*sizeof(MapSource *), mp->size_src, mp->src,
		MapSource *, 0);

	if( !(mp->src[mp->nsrc] = (MapSource *)AxesMalloc((Widget)w,
		sizeof(MapSource))) ) return(-2);

	src = mp->src[mp->nsrc];
	mp->nsrc++;

	memcpy(src, &map_source_init, sizeof(MapSource));

	memcpy(&src->source, source, sizeof(MapPlotSource));
	src->source.id = getMapId(mp);
	if(src->source.label != NULL)
	{
	    src->source.label = strdup(src->source.label);
	}
	if(src->source.smajax > 0.0 && src->source.sminax > 0.0)
	{
	    ell.label = NULL;
	    ell.lat = src->source.lat;
	    ell.lon = src->source.lon;
	    ell.sminax = src->source.sminax;
	    ell.smajax = src->source.smajax;
	    ell.strike = src->source.strike;
	    memcpy(&ell.line, &line_info_init, sizeof(LineInfo));
/* fg == 0 is valid pixel 
	    if (source->sym.fg > 0) ell.line.fg = source->sym.fg;
*/
	    ell.line.fg = source->sym.fg;
	    if( src->source.sym.display > MAP_LOCKED_OFF &&
		src->source.sym.display < MAP_LOCKED_ON)
	    {
		ell.line.display = mp->display_source_ellipses;
	    }
	    else
	    {
		ell.line.display = src->source.sym.display;
	    }
	    ell.line.fg = src->source.sym.fg;
	    ell.line.selected = src->source.sym.selected;
	    MapPlotAddEllipse(w, &ell, False);
	    mp->ell[mp->nell-1]->src_id = src->source.id;
	    mp->ell[mp->nell-1]->type = MAP_SRC_ELLIPSE;
	}

	if(redisplay)
	{
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = src->source.id;
	    MapPlotAddSource(mp->utm_map, source, redisplay);
	    utm_mp->next_id = -1;
	}
	return(src->source.id);
}

int
MapPlotGetSources(MapPlotWidget w, MapPlotSource **src)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*src = (MapPlotSource *)AxesMalloc((Widget)w,
		mp->nsrc*sizeof(MapPlotSource))) ) return(0);

	for(i = 0; i < mp->nsrc; i++)
	{
	    (*src)[i] = mp->src[i]->source;
	}
	return(mp->nsrc);
}

static int
MapPlotChangeSource(MapPlotWidget w, MapPlotSource *source, Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i, j;
	double		slat, slon, rlat, rlon, baz;
	MapSource	*s;
	MapArc		*a;
	MapEllipse	*e;
	MapPlotEllipse	ell;
	MapDelta	*d;
	LineInfo line_info_init = LINE_INFO_INIT;

	for(i = 0; i < mp->nsrc; i++)
	{
	    if(mp->src[i]->source.id == source->id) break;
	}
	if(i == mp->nsrc) return(0);

	s = mp->src[i];
	if(s->source.label != source->label)
	{
	    Free(s->source.label);
	    memcpy(&s->source, source, sizeof(MapPlotSource));
	    if(source->label != NULL)
	    {
		s->source.label = strdup(source->label);
	    }
	}
	else
	{
	    memcpy(&s->source, source, sizeof(MapPlotSource));
	}

	for(i = 0; i < mp->nell; i++)
	{
	    if( mp->ell[i]->type == MAP_SRC_ELLIPSE &&
		mp->ell[i]->src_id == source->id) break;
	}
	if(i < mp->nell)
	{
	    e = mp->ell[i];
	    if(source->smajax > 0.0 && source->sminax > 0.0)
	    {
		if(source->lat != e->ellipse.lat ||
		   source->lon != e->ellipse.lon ||
		   source->smajax != e->ellipse.smajax ||
		   source->sminax != e->ellipse.sminax ||
		   source->strike != e->ellipse.strike)
		{
		    e->ellipse.lat = source->lat;
		    e->ellipse.lon = source->lon;
		    e->ellipse.sminax = source->sminax;
		    e->ellipse.smajax = source->smajax;
		    e->ellipse.strike = source->strike;
		    MapPlotChangeEllipse(w, &e->ellipse, False);
		}
	    }
	    else
	    {
		DeleteEllipse(w, i);
	    }
	}
	else if(source->smajax > 0.0 && source->sminax > 0.0)
	{
	    ell.label = NULL;
	    ell.lat = s->source.lat;
	    ell.lon = s->source.lon;
	    ell.sminax = s->source.sminax;
	    ell.smajax = s->source.smajax;
	    ell.strike = s->source.strike;
	    memcpy(&ell.line, &line_info_init, sizeof(LineInfo));
			
	    if( s->source.sym.display > MAP_LOCKED_OFF &&
		s->source.sym.display < MAP_LOCKED_ON)
	    {
		ell.line.display = mp->display_source_ellipses;
	    }
	    else
	    {
		ell.line.display = s->source.sym.display;
	    }
	    ell.line.selected = s->source.sym.selected;
	    MapPlotAddEllipse(w, &ell, False);
	    mp->ell[mp->nell-1]->src_id = s->source.id;
	}
	for(i = 0; i < mp->narc; i++)
	{
	    a = mp->arc[i];
	    if(a->src_id == source->id &&
		(source->lat != a->arc.lat || source->lon != a->arc.lon))
	    {
		if(a->type == MAP_PATH)
		{
		    for(j = 0; j < mp->nsta; j++)
		    {
		        if(a->sta_id == mp->sta[j]->station.id) break;
		    }
		    slat = s->source.lat;
		    slon = s->source.lon;
		    rlat = mp->sta[j]->station.lat;
		    rlon = mp->sta[j]->station.lon;
		    deltaz(slat, slon, rlat, rlon, &a->arc.del,&a->arc.az,&baz);
		}
		a->arc.lat = s->source.lat;
		a->arc.lon = s->source.lon;
		MapPlotChangeArc(w, &a->arc, False);
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	{
	    d = mp->del[i];
	    if(d->src_id == source->id &&
		(source->lat != d->delta.lat || source->lon != d->delta.lon))
	    {
		d->delta.lat = source->lat;
		d->delta.lon = source->lon;
		MapPlotChangeDelta(w, &d->delta, False);
	    }
	}

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

static int
MapPlotDeleteSource(MapPlotWidget w, int id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j;

	for(i = 0; i < mp->nsrc; i++)
	{
	    if(mp->src[i]->source.id == id) break;
	}	
	if(i == mp->nsrc) return(0);

	DeleteSource(w, i);

	/* delete all arcs, deltas and ellipses associated with this source
	 */
	for(j = mp->narc-1; j >= 0; j--)
	{
	    if(mp->arc[j]->src_id == id)
	    {
		DeleteArc(w, j);
	    }
	}
	for(j = mp->ndel-1; j >= 0; j--)
	{
	    if(mp->del[j]->src_id == id)
	    {
		DeleteDelta(w, j);
	    }
	}
	for(j = mp->nell-1; j >= 0; j--)
	{
	    if(mp->ell[j]->src_id == id)
	    {
		DeleteEllipse(w, j);
	    }
	}
	if(redisplay && !ax->redisplay_pending)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	return(1);
}

// what about ColorScale member of MapPlotTheme ??
int
MapPlotAddTheme(MapPlotWidget w, MapPlotTheme *theme, Boolean display)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapTheme *t;
	int i;

	if(theme->shape_type != SHPT_POINT
		&& theme->shape_type != SHPT_ARC
		&& theme->shape_type != SHPT_POLYGON
		&& theme->shape_type != SHPT_POLAR)
	{
	    _AxesWarn((AxesWidget)w, "MapPlotAddTheme: Unknown theme type: %s",
			theme->type_name);
	    return -1;
	}
	ALLOC((mp->nthemes+1)*sizeof(MapTheme *),
		mp->size_themes, mp->theme, MapTheme *, 0);

	mp->theme[mp->nthemes] = new MapTheme();

	t = mp->theme[mp->nthemes];
	mp->nthemes++;

	t->theme_type = THEME_SHAPE;

	t->theme = *theme;

	if( t->theme.nshapes && !(t->td = (ThemeDisplay *)AxesMalloc((Widget)w,
		t->theme.nshapes*sizeof(ThemeDisplay))) ) return -1;

	for(i = 0; i < t->theme.nshapes; i++) {
	    ThemeDisplay *td = &t->td[i];
	    td->fill_segs.segs = NULL;
	    td->fill_segs.nsegs = 0;
	    td->fill_segs.size_segs = 0;
	    td->bndy_segs.segs = NULL;
	    td->bndy_segs.nsegs = 0;
	    td->bndy_segs.size_segs = 0;
	    td->p.points = NULL;
	    td->p.npoints = 0;
	    td->p.size_points = 0;
	    td->fill = True;
	    td->selected = False;
	    td->display = True;
	    td->center_x = 0;
	    td->center_y = 0;
	    td->npts = 0;
	    td->lon = NULL;
	    td->lat = NULL;
	    td->label = NULL;
	}

	if(t->theme.shape_type == SHPT_ARC)
	{
	    double baz;
	    MapArc a;
	    for(i = 0; i < t->theme.nshapes; i++)
	    {
		SHPObject *s = t->theme.shapes[i];
		ThemeDisplay *td = &t->td[i];

		if(s->nVertices != 2) continue;

		a.arc.lon = s->padfX[0];
		a.arc.lat = s->padfY[0];
		deltaz(a.arc.lat, a.arc.lon, s->padfY[1], s->padfX[1],
				&a.arc.del, &a.arc.az, &baz);

		td->npts = (int)(a.arc.del*2 + 1);
		if(td->npts < 2) td->npts = 2;
		if( !(td->lon = (double *)AxesMalloc((Widget)w,
			td->npts*sizeof(double))) ) return -1;
		if( !(td->lat = (double *)AxesMalloc((Widget)w,
			td->npts*sizeof(double))) ) return -1;

		a.npts = td->npts;
		a.lon = td->lon;
		a.lat = td->lat;
		MakeArc(w, &a);
	    }
	}

	if(t->theme.polar_coords)
	{
	    int j;
	    t->rmax = 0.;
	    for(i = 0; i < t->theme.nshapes; i++)
	    {
		SHPObject *s = t->theme.shapes[i];

		for(j = 0; j < s->nVertices; j++) {
		    if(t->rmax < s->padfX[j]) t->rmax = s->padfX[j];
		}
	    }
	    if(mp->polar_max_radius < t->rmax) {
		mp->polar_max_radius = t->rmax;
	    }

	    if(mp->projection == MAP_POLAR)
	    {
		double d = mp->polar_margin*t->rmax;
		ax->zoom = 0;
		ax->x1[0] = -mp->polar_max_radius - d;
		ax->x2[0] =  mp->polar_max_radius + d;
		ax->y1[0] = -mp->polar_max_radius - d;
		ax->y2[0] =  mp->polar_max_radius + d;
		_AxesRedraw((AxesWidget)w);
		_MapPlotRedraw(w);
	    }
	}

	t->id = getMapId(mp);

	DrawTheme(w, &ax->d, t);

	if(display)
	{
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = t->id;
	    MapPlotAddTheme(mp->utm_map, theme, display);
	    utm_mp->next_id = -1;
	}
	return(t->id);
}

int
MapPlotThemeLevelDown(MapPlotWidget w, int theme_id)
{
	MapPlotPart *mp = &w->map_plot;
	int i;
	MapTheme *t;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	if(i > 0) {
	    t = mp->theme[i-1];
	    mp->theme[i-1] = mp->theme[i];
	    mp->theme[i] = t;
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotThemeLevelDown(mp->utm_map, theme_id);
	}
	return 1;
}

int
MapPlotThemeLevelUp(MapPlotWidget w, int theme_id)
{
	MapPlotPart *mp = &w->map_plot;
	int i;
	MapTheme *t;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	if(i < mp->nthemes-1) {
	    t = mp->theme[i+1];
	    mp->theme[i+1] = mp->theme[i];
	    mp->theme[i] = t;
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotThemeLevelUp(mp->utm_map, theme_id);
	}
	return 1;
}

static int
MapPlotDeleteTheme(MapPlotWidget w, int theme_id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	DeleteTheme(w, i);

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

int
MapPlotThemeBndry(MapPlotWidget w, int theme_id, Boolean bndry)
{
	MapPlotPart *mp = &w->map_plot;
	int i, ret;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	if(mp->theme[i]->theme.bndry != bndry) {
	    mp->theme[i]->theme.bndry = bndry;
	    MapPlotUpdate(w);
	    ret = 1;
	}
	ret = 0;
	if( mp->utm_map ) {
	    MapPlotThemeBndry(mp->utm_map, theme_id, bndry);
	}
	return ret;
}

int
MapPlotThemeFill(MapPlotWidget w, int theme_id, Boolean fill)
{
	MapPlotPart *mp = &w->map_plot;
	int i, ret;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	if(mp->theme[i]->theme.fill != fill) {
	    mp->theme[i]->theme.fill = fill;
	    _MapPlotRedraw(w);
	    MapPlotUpdate(w);
	    ret = 1;
	}
	ret = 0;
	if( mp->utm_map ) {
	    MapPlotThemeFill(mp->utm_map, theme_id, fill);
	}
	return ret;
}

int
MapPlotShapeColor(MapPlotWidget w, int theme_id, int num, double *values,
		int num_bounds, double *bounds)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i, k, klower, kupper, kmiddle;
	ColorScale *c;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	t = mp->theme[i];
	c = &t->theme.color_scale;

	if(c->num_colors <= 0) {
	    fprintf(stderr,
	"MapPlotShapeColor: no colors. Call MapPlotThemeColorScale first.\n");
	    return 0;
	}
	for(i = 0; i < num_bounds && i <= c->num_colors; i++) {
	    c->lines[i] = bounds[i];
	}

	for(i = 0; i < num && i < t->theme.nshapes; i++) {
	    if(values[i] <= c->lines[0]) {
		k = 0;
	    }
	    else if(values[i] >= c->lines[c->num_colors]) {
		k = c->num_colors-1;
	    }
	    else {
		klower = -1; kupper = c->num_colors+1;
		while(kupper - klower > 1)
		{
		    kmiddle = (klower + kupper)/2;
		    if(values[i] > c->lines[kmiddle]) {
			klower = kmiddle;
		    }
		    else {
			kupper = kmiddle;
		    }
		}
		k = klower;
	    }
	    t->theme.shape_fg[i] = c->pixels[k];
	}
	MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotShapeColor(mp->utm_map, theme_id, num, values, num_bounds,
				bounds);
	}
	return 1;
}

void
MapPlotSetShapeSelected(MapPlotWidget w, int theme_id, vector<int> ishape)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return;

	t = mp->theme[i];

	for(i = 0; i < t->theme.nshapes; i++) {
	    t->td[i].selected = False;
	}
	for(i = 0; i < (int)ishape.size(); i++) {
	    if(ishape[i] >= 0 && ishape[i] < t->theme.nshapes) {
		t->td[ishape[i]].selected = True;
	    }
	}
	MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotSetShapeSelected(mp->utm_map, theme_id, ishape);
	}
}

void
MapPlotThemeCursor(MapPlotWidget w, int theme_id, Boolean on)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return;

	mp->theme[i]->cursor_info = on;

	if( mp->utm_map ) {
	    MapPlotThemeCursor(mp->utm_map, theme_id, on);
	}
}

int
MapPlotDisplayTheme(MapPlotWidget w, int theme_id, Boolean display_theme)
{
	MapPlotPart *mp = &w->map_plot;
	int i, ret;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	if(mp->theme[i]->display != display_theme) {
	    mp->theme[i]->display = display_theme;
	    _MapPlotRedraw(w);
	    MapPlotUpdate(w);
	    ret = 1;
	}
	ret = 0;
	if( mp->utm_map ) {
	    MapPlotDisplayTheme(mp->utm_map, theme_id, display_theme);
	}
	return ret;
}

int
MapPlotDisplayShape(MapPlotWidget w, int theme_id, int shape_index,
		Boolean display_shape, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapTheme *t;
	
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return(0);

	t = mp->theme[i];

	if(shape_index >= 0 && shape_index < t->theme.nshapes)
	{
	    if(display_shape != t->td[shape_index].display)
	    {
		t->td[shape_index].display = display_shape;
		DrawThemeShape(w, &ax->d, t, shape_index);
		if(redisplay) MapPlotUpdate(w);
	    }
	}
	if( mp->utm_map ) {
	    MapPlotDisplayShape(mp->utm_map, theme_id, shape_index,
				display_shape, redisplay);
	}
	return(1);
}

Boolean
MapPlotShapeIsDisplayed(MapPlotWidget w, int theme_id, int shape_index)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return False;

	t = mp->theme[i];

	if(shape_index >= 0 && shape_index < t->theme.nshapes)
	{
	    return t->td[shape_index].display;
	}
	return False;
}

/*
int
MapPlotShapeLabels(MapPlotWidget w, int theme_id, int nlab, const char **labels)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	t = mp->theme[i];

	for(i = 0; i < nlab && i < t->theme.nshapes; i++) {
	    t->td[i].label = (char *)labels[i];
	}
	while(i < t->theme.nshapes) t->td[i++].label = NULL;
	if(t->display_labels)
	{
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotShapeLabels(mp->utm_map, theme_id, nlab, labels);
	}
	return 1;
}
*/

int
MapPlotShapeLabels(MapPlotWidget w, int theme_id, vector<const char *> &labels)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	t = mp->theme[i];

	for(i = 0; i < (int)labels.size() && i < t->theme.nshapes; i++) {
	    t->td[i].label = (char *)labels[i];
	}
	while(i < t->theme.nshapes) t->td[i++].label = NULL;
	if(t->display_labels)
	{
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotShapeLabels(mp->utm_map, theme_id, labels);
	}
	return 1;
}

int
MapPlotDisplayShapeLabels(MapPlotWidget w, int theme_id, Boolean display)
{
	MapPlotPart *mp = &w->map_plot;
	MapTheme *t;
	int i;

	for(i = 0; i < mp->nthemes && mp->theme[i]->id != theme_id; i++);
	if(i == mp->nthemes) return 0;

	t = mp->theme[i];

	if(t->display_labels != display) {
	    t->display_labels = display;
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplayShapeLabels(mp->utm_map, theme_id, display);
	}
	return 1;
}

int
MapPlotAddLine(MapPlotWidget w, MapPlotLine *line, Boolean display)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapLine	*l;
	MapLine map_line_init = MAP_LINE_INIT;

	ALLOC((mp->nlines+1)*sizeof(MapLine *),
		mp->size_line, mp->line, MapLine *, 0);

	if( !(mp->line[mp->nlines] = (MapLine *)AxesMalloc((Widget)w,
		sizeof(MapLine))) ) return(0);

	l = mp->line[mp->nlines];
	mp->nlines++;

	memcpy(l, &map_line_init, sizeof(MapLine));

	memcpy(&l->line, line, sizeof(MapPlotLine));
	l->line.id = getMapId(mp);
	if(l->line.label != NULL)
	{
	    l->line.label = strdup(l->line.label);
	}
	if(l->line.npts > 0)
	{
	    if( !(l->line.lon = (double *)AxesMalloc((Widget)w,
			l->line.npts*sizeof(double))) ||
		!(l->line.lat = (double *)AxesMalloc((Widget)w,
			l->line.npts*sizeof(double))) )
	    {
		mp->nlines--;
		Free(l->line.lon); Free(l->line.lat);
		return(0);
	    }
	    memcpy(l->line.lon, line->lon, line->npts*sizeof(double));
	    memcpy(l->line.lat, line->lat, line->npts*sizeof(double));
	}

	ax->d.s  = &l->s;
	DrawLonLat(w, &ax->d, l->line.npts, l->line.lon, l->line.lat);

	if(display)
	{
	    RedisplayLine(w, XtWindow(w), &l->line.line, &l->s);
	    mp->need_redisplay = True;
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = l->line.id;
	    MapPlotAddLine(mp->utm_map, line, display);
	    utm_mp->next_id = -1;
	}
	return(l->line.id);
}

int
MapPlotGetLines(MapPlotWidget w, MapPlotLine **line)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*line = (MapPlotLine *)AxesMalloc((Widget)w,
		mp->nlines*sizeof(MapPlotLine))) ) return(0);

	for(i = 0; i < mp->nlines; i++)
	{
	    (*line)[i] = mp->line[i]->line;
	}
	return(mp->nlines);
}

static int
MapPlotDeleteLine(MapPlotWidget w, int j, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->nlines) return 0;

	Free(mp->line[j]->line.label);
	Free(mp->line[j]->line.lon);
	Free(mp->line[j]->line.lat);
	Free(mp->line[j]->s.segs);
	Free(mp->line[j]);
	for(i = j; i < mp->nlines-1; i++) mp->line[i] = mp->line[i+1];
	mp->nlines--;

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

int
MapPlotAddSymbols(MapPlotWidget w, int nsymbols, MapPlotSymbol *symbol,
			Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	MapPlotPart *utm_mp = NULL;
	int i;

	if(nsymbols <= 0) return 0;

	ALLOC((mp->nsymbol+nsymbols)*sizeof(MapPlotSymbol),
		mp->size_symbol, mp->symbol, MapPlotSymbol, 0);

	if( mp->utm_map ) {
	    utm_mp = &mp->utm_map->map_plot;
	    ALLOC((utm_mp->nsymbol+nsymbols)*sizeof(MapPlotSymbol),
		    utm_mp->size_symbol, utm_mp->symbol, MapPlotSymbol, 0);
	}

	for(i = 0; i < nsymbols; i++) {
	    symbol[i].id = getMapId(mp);
	    memcpy(mp->symbol+mp->nsymbol, symbol+i, sizeof(MapPlotSymbol));
	    mp->nsymbol++;

	    if(utm_mp) {
		memcpy(utm_mp->symbol+utm_mp->nsymbol, symbol+i,
			sizeof(MapPlotSymbol));
		utm_mp->nsymbol++;
	    }
	}

	if(redisplay)
	{
	    if(mp->projection == MAP_POLAR)
	    {
		_AxesRedraw((AxesWidget)w);
		_MapPlotRedraw(w);
	    }
	    if(utm_mp && utm_mp->projection == MAP_POLAR)
	    {
		_AxesRedraw((AxesWidget)mp->utm_map);
		_MapPlotRedraw(mp->utm_map);
	    }
	    MapPlotUpdate(w);
	    if(mp->utm_map) MapPlotUpdate(mp->utm_map);
	}

	return (symbol[0].id);
}

int
MapPlotAddSymbolGroup(MapPlotWidget w, MapPlotSymbolGroup *group,
			Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	MapSymbolGroup	*s;
	MapSymbolGroup map_symbol_init = MAP_SYMBOL_GROUP_INIT;

	ALLOC((mp->nsymgrp+1)*sizeof(MapSymbolGroup *),
		mp->size_symgrp, mp->symgrp, MapSymbolGroup *, 0);

	if( !(mp->symgrp[mp->nsymgrp] = (MapSymbolGroup *)AxesMalloc((Widget)w,
		sizeof(MapSymbolGroup))) ) return(0);

	s = mp->symgrp[mp->nsymgrp];
	mp->nsymgrp++;

	memcpy(s, &map_symbol_init, sizeof(MapSymbolGroup));

	memcpy(&s->group, group, sizeof(MapPlotSymbolGroup));
	s->group.id = getMapId(mp);
	if(s->group.label != NULL)
	{
	    s->group.label = strdup(s->group.label);
	}
	if(s->group.npts > 0)
	{
	    if( !(s->group.lon = (double *)AxesMalloc((Widget)w,
			s->group.npts*sizeof(double))) ||
		!(s->group.lat = (double *)AxesMalloc((Widget)w,
			s->group.npts*sizeof(double))) ||
		!(s->group.size = (int *)AxesMalloc((Widget)w,
			s->group.npts*sizeof(int))) ||
		!(s->p = (PointsArray *)AxesMalloc((Widget)w,
			s->group.npts*sizeof(PointsArray))) )
	    {
		mp->nsymgrp--;
		Free(s->group.lon); Free(s->group.lat);
		Free(s->group.size); Free(s->p);
		return(0);
	    }
	    memcpy(s->group.lon, group->lon, group->npts*sizeof(double));
	    memcpy(s->group.lat, group->lat, group->npts*sizeof(double));
	    memcpy(s->group.size, group->size, group->npts*sizeof(int));
	}
	if(group->polar_coords)
	{
	    AxesPart *ax =  &w->axes;
	/* do not change the radius based on the input data
	    int i;
	    for(i = 0; i < s->group.npts; i++) {
		if(group->lon[i] > mp->polar_max_radius) {
		    mp->polar_max_radius = group->lon[i];
		}
	    }
            */
	    if(mp->projection == MAP_POLAR)
	    {
		double d = mp->polar_margin*mp->polar_max_radius;
		ax->zoom = 0;
		ax->x1[0] = -mp->polar_max_radius - d;
		ax->x2[0] =  mp->polar_max_radius + d;
		ax->y1[0] = -mp->polar_max_radius - d;
		ax->y2[0] =  mp->polar_max_radius + d;
		_AxesRedraw((AxesWidget)w);
		_MapPlotRedraw(w);
	    }
	}
	if(redisplay)
	{
	    MapPlotUpdate(w);
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = s->group.id;
	    MapPlotAddSymbolGroup(mp->utm_map, group, redisplay);
	    utm_mp->next_id = -1;
	}
	return(s->group.id);
}

int
MapPlotAddPolygon(MapPlotWidget w, MapPlotPolygon *poly, Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	MapPlotPolygon	*p;

	ALLOC((mp->npoly+1)*sizeof(MapPlotPolygon *),
		mp->size_poly, mp->poly, MapPlotPolygon *, 0);

	if( !(mp->poly[mp->npoly] = (MapPlotPolygon *)AxesMalloc((Widget)w,
			sizeof(MapPlotPolygon))) ) return(0);

	p = mp->poly[mp->npoly];
	mp->npoly++;

	memcpy(p, poly, sizeof(MapPlotPolygon));

	p->id = getMapId(mp);
	if(p->label != NULL)
	{
	    p->label = strdup(p->label);
	}
	if(p->npts > 0)
	{
	    if( !(p->lon = (double *)AxesMalloc((Widget)w,
			p->npts*sizeof(double))) ||
		!(p->lat = (double *)AxesMalloc((Widget)w,
			p->npts*sizeof(double))) )
	    {
		mp->npoly--;
		Free(p->lon); Free(p->lat);
		return(0);
	    }
	    memcpy(p->lon, poly->lon, poly->npts*sizeof(double));
	    memcpy(p->lat, poly->lat, poly->npts*sizeof(double));
	}
	if(redisplay)
	{
	    RedisplayPoly(w, XtWindow(w), p);
	    mp->need_redisplay = True;
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = p->id;
	    MapPlotAddPolygon(mp->utm_map, poly, redisplay);
	    utm_mp->next_id = -1;
	}
	return(p->id);
}

int
MapPlotAddRectangle(MapPlotWidget w, MapPlotRectangle *rect, Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	MapPlotRectangle *p;

	ALLOC((mp->nrect+1)*sizeof(MapPlotRectangle *),
		mp->size_rect, mp->rect, MapPlotRectangle *, 0);

	if( !(mp->rect[mp->nrect] = (MapPlotRectangle *)AxesMalloc((Widget)w,
		sizeof(MapPlotRectangle))) ) return(0);

	p = mp->rect[mp->nrect];
	mp->nrect++;

	memcpy(p, rect, sizeof(MapPlotRectangle));

	p->id = getMapId(mp);
	if(p->label != NULL)
	{
	    p->label = strdup(p->label);
	}
	if(redisplay)
	{
	    RedisplayRectangle(w, XtWindow(w), p);
	    mp->need_redisplay = True;
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = p->id;
	    MapPlotAddRectangle(mp->utm_map, rect, redisplay);
	    utm_mp->next_id = -1;
	}
	return(p->id);
}

/*
int
MapPlotAddMatrix(MapPlotWidget w, MapPlotMatrix *rect, Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	MapPlotMatrix *p;

	ALLOC((mp->nrect+1)*sizeof(MapPlotMatrix *),
		mp->size_rect, mp->rect, MapPlotMatrix *, 0);

	if( !(mp->rect[mp->nrect] = (MapPlotRectangle *)AxesMalloc((Widget)w,
		sizeof(MapPlotRectangle))) ) return(0);

	p = mp->rect[mp->nrect];
	mp->nrect++;

	memcpy(p, rect, sizeof(MapPlotRectangle));

	p->id = getMapId(mp);
	if(p->label != NULL)
	{
	    p->label = strdup(p->label);
	}
	if(redisplay)
	{
	    RedisplayRectangle(w, XtWindow(w), p);
	    mp->need_redisplay = True;
	}
	return(p->id);
}
*/

int
MapPlotGetSymbolGroups(MapPlotWidget w, MapPlotSymbolGroup **symgrp)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*symgrp = (MapPlotSymbolGroup *)AxesMalloc((Widget)w,
		mp->nsymgrp*sizeof(MapPlotSymbolGroup))) ) return(0);

	for(i = 0; i < mp->nsymgrp; i++)
	{
	    (*symgrp)[i] = mp->symgrp[i]->group;
	}
	return(mp->nsymgrp);
}

static int
MapPlotDeleteSymbolGroup(MapPlotWidget w, int j, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->nsymgrp) return(0);

	Free(mp->symgrp[j]->group.label);
	Free(mp->symgrp[j]->group.lon);
	Free(mp->symgrp[j]->group.lat);
	Free(mp->symgrp[j]->group.size);
	Free(mp->symgrp[j]->p);
	Free(mp->symgrp[j]);
	for(i = j; i < mp->nsymgrp-1; i++) mp->symgrp[i] = mp->symgrp[i+1];
	mp->nsymgrp--;

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

int
MapPlotGetSymbols(MapPlotWidget w, MapPlotSymbol **symbols)
{
	MapPlotPart *mp = &w->map_plot;

        if(mp->nsymbol <= 0) {
	    *symbols = NULL;
	    return 0;
	}

	if( !(*symbols = (MapPlotSymbol *)AxesMalloc((Widget)w,
		mp->nsymbol*sizeof(MapPlotSymbol))) ) return(0);

	memcpy(*symbols, mp->symbol, mp->nsymbol*sizeof(MapPlotSymbol));
	return(mp->nsymbol);
}

static int
MapPlotDeleteSymbol(MapPlotWidget w, int j, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;

	if(j < 0 || j >= mp->nsymbol) return(0);

	if(j < mp->nsymbol-1) {
	    memmove(mp->symbol+j, mp->symbol+j+1,
			(mp->nsymbol-j-1)*sizeof(MapPlotSymbol));
	}
	mp->nsymbol--;

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

void
MapPlotClearSymbols(MapPlotWidget w, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;

	mp->nsymbol = 0;
	if(redisplay) MapPlotUpdate(w);

	if( mp->utm_map ) {
	    MapPlotClearSymbols(mp->utm_map, redisplay);
	}
}

static int
MapPlotDeletePoly(MapPlotWidget w, int j, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->npoly) return(0);

	Free(mp->poly[j]->label);
	Free(mp->poly[j]->lon);
	Free(mp->poly[j]->lat);
	Free(mp->poly[j]);
	for(i = j; i < mp->npoly-1; i++) mp->poly[i] = mp->poly[i+1];
	mp->npoly--;

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

static int
MapPlotDeleteRectangle(MapPlotWidget w, int j, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->nrect) return(0);

	Free(mp->rect[j]->label);
	Free(mp->rect[j]);
	for(i = j; i < mp->nrect-1; i++) mp->rect[i] = mp->rect[i+1];
	mp->nrect--;

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

int
MapPlotAddEllipse(MapPlotWidget w, MapPlotEllipse *ellipse, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapEllipse *e;
	MapEllipse map_ellipse_init = MAP_ELLIPSE_INIT;

	ALLOC((mp->nell+1)*sizeof(MapEllipse *),
		mp->size_ell, mp->ell, MapEllipse *, 0);

	if( !(mp->ell[mp->nell] = (MapEllipse *)AxesMalloc((Widget)w,
		sizeof(MapEllipse))) ) return(0);

	e = mp->ell[mp->nell];
	mp->nell++;

	memcpy(e, &map_ellipse_init, sizeof(MapEllipse));

	memcpy(&e->ellipse, ellipse, sizeof(MapPlotEllipse));
	e->ellipse.id = getMapId(mp);
	if(e->ellipse.label != NULL)
	{
	    e->ellipse.label = strdup(e->ellipse.label);
	}

	if(e->ellipse.line.display <= MAP_OFF || (!e->ellipse.line.selected &&
	    e->ellipse.line.display == MAP_SELECTED_ON))
	{
	    return(e->ellipse.id);
	}

	MakeEllipse(w, e);

	Free(e->s.segs);
	e->s.size_segs = 0;
	e->s.nsegs = 0;
	ax->d.s  = &e->s;
	DrawLonLat(w, &ax->d, e->npts, e->lon, e->lat);

	if(redisplay && !ax->redisplay_pending)
	{
	    RedisplayLine(w, XtWindow(w), &e->ellipse.line, &e->s);
	    mp->need_redisplay = True;
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = e->ellipse.id;
	    MapPlotAddEllipse(mp->utm_map, ellipse, redisplay);
	    utm_mp->next_id = -1;
	}
	return(e->ellipse.id);
}

int
MapPlotGetEllipses(MapPlotWidget w, MapPlotEllipse **ellipse)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*ellipse = (MapPlotEllipse *)AxesMalloc((Widget)w,
		mp->nell*sizeof(MapPlotEllipse))) ) return(0);

	for(i = 0; i < mp->nell; i++)
	{
	    (*ellipse)[i] = mp->ell[i]->ellipse;
	}
	return(mp->nell);
}

static int
MapPlotChangeEllipse(MapPlotWidget w, MapPlotEllipse *ellipse,Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i;
	MapEllipse *e;

	for(i = 0; i < mp->nell; i++)
	{
	    if(mp->ell[i]->ellipse.id == ellipse->id) break;
	}
	if(i == mp->nell) return(0);

	e = mp->ell[i];

	if(e->ellipse.label != ellipse->label)
	{
	    Free(e->ellipse.label);
	    memcpy(&e->ellipse, ellipse, sizeof(MapPlotDelta));
	    if(ellipse->label != NULL)
	    {
		e->ellipse.label = strdup(ellipse->label);
	    }
	}
	else
	{
	    memcpy(&e->ellipse, ellipse, sizeof(MapPlotDelta));
	}
	if(e->ellipse.line.display <= MAP_OFF || (!e->ellipse.line.selected
	    && e->ellipse.line.display == MAP_SELECTED_ON))
	{
	    return(0);
	}
	MakeEllipse(w, e);

	Free(e->s.segs);
	e->s.size_segs = 0;
	e->s.nsegs = 0;
	ax->d.s  = &e->s;
	DrawLonLat(w, &ax->d, e->npts, e->lon, e->lat);

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

static int
MapPlotDeleteEllipse(MapPlotWidget w, int id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;

	for(i = 0; i < mp->nell; i++)
	{
	    if(mp->ell[i]->ellipse.id == id) break;
	}
	if(i == mp->nell) return(0);

	DeleteEllipse(w, i);

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

static void
MakeEllipse(MapPlotWidget w, MapEllipse *e)
{	
	int	i;
	double	a, b, phi, theta, dphi, alpha, beta, gamma, sn, cs;

	e->changed = False;
	e->npts = 200;
	Free(e->lat);
	Free(e->lon);
	e->lat = (double *)AxesMalloc((Widget)w, e->npts*sizeof(double));
	e->lon = (double *)AxesMalloc((Widget)w, e->npts*sizeof(double));

	a = e->ellipse.sminax/DEG_TO_KM;
	b = e->ellipse.smajax/DEG_TO_KM;

	/* euler angles for a system rotated to the epicenter with the
	 * x-axis in the direction of the a-axis
	 * (alpha = -gamma, beta = -beta, gamma = -alpha for reverse rotations)
	 */
	alpha = DEG_TO_RADIANS*e->ellipse.lon;
	beta  = DEG_TO_RADIANS*(90. - geocentric(e->ellipse.lat));
	gamma = DEG_TO_RADIANS*(90. - e->ellipse.strike);

	dphi = 360./(e->npts-1);
	for(i = 0; i < e->npts; i++)
	{
	    phi = i*dphi*DEG_TO_RADIANS;
	    sn = sin(phi);
	    cs = cos(phi);
	    theta = DEG_TO_RADIANS*a*b/sqrt(a*a*sn*sn + b*b*cs*cs);
	    euler(&theta, &phi, -gamma, -beta, -alpha);
	    e->lat[i] = geographic(90. - theta/DEG_TO_RADIANS);
	    e->lon[i] = phi/DEG_TO_RADIANS;
	}
}

int
MapPlotAddArc(MapPlotWidget w, MapPlotArc *arc, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapArc *a;
	MapArc map_arc_init = MAP_ARC_INIT;

	ALLOC((mp->narc+1)*sizeof(MapArc *), mp->size_arc, mp->arc, MapArc *,0);

	if( !(mp->arc[mp->narc] = (MapArc *)AxesMalloc((Widget)w,
		sizeof(MapArc))) ) return(0);

	a = mp->arc[mp->narc];
	mp->narc++;

	memcpy(a, &map_arc_init, sizeof(MapArc));

	memcpy(&a->arc, arc, sizeof(MapPlotArc));
	a->arc.id = getMapId(mp);
	if(a->arc.label != NULL)
	{
	    a->arc.label = strdup(a->arc.label);
	}
	a->npts = 200;
	a->lat = (double *)AxesMalloc((Widget)w, a->npts*sizeof(double));
	a->lon = (double *)AxesMalloc((Widget)w, a->npts*sizeof(double));

	if(a->arc.line.display <= MAP_OFF || (!a->arc.line.selected &&
	    a->arc.line.display == MAP_SELECTED_ON))
	{
	    return(a->arc.id);
	}
	if(a->arc.line.fg == w->core.background_pixel)
	{
	    a->arc.line.fg = ax->fg;
	}

	MakeArc(w, a);

	DrawArc(w, &ax->d, a);

	if(redisplay && !ax->redisplay_pending)
	{
_MapPlotRedisplay(w);
//	    RedisplayLine(w, XtWindow(w), &a->arc.line, &a->s);
//	    mp->need_redisplay = True;
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = a->arc.id;
	    MapPlotAddArc(mp->utm_map, arc, redisplay);
	    utm_mp->next_id = -1;
	}
	return(a->arc.id);
}

int
MapPlotGetStaArc(MapPlotWidget w, char *sta, char *label, MapPlotArc *arc)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;
	MapArc		*a;
	MapStation	*s=NULL;

	for(i = 0; i < mp->nsta; i++)
	{
	    if(mp->sta[i]->station.label != NULL && 
		!strcmp(mp->sta[i]->station.label, sta))
	    {
		s = mp->sta[i];
		break;
	    }
	}
	if(i == mp->nsta) return(0);

	for(i = 0; i < mp->narc; i++)
	{
	    a = mp->arc[i];
	    if(a->type != MAP_MEASURE && a->sta_id == s->station.id &&
		a->arc.label != NULL && !strcmp(a->arc.label, label))
			
	    {
		*arc = mp->arc[i]->arc;
		return(1);
	    }
	}
	return(0);
}

void
MapPlotDisplay(MapPlotWidget w, int num_ids, int *id, int display,
			Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, sta_id, src_id;
	Boolean		need_redisplay;
	DrawStruct	*d;
	MapStation	*sta;
	MapSource	*src;
	MapArc		*arc;
	MapDelta	*del;
	MapEllipse	*ell;
	MapLine		*l;
	MapSymbolGroup	*s;

	d = &ax->d;
	need_redisplay = False;
	for(i = 0; i < num_ids; i++)
	{
	    sta_id = -100;
	    for(j = 0; j < mp->nsta; j++)
		if(mp->sta[j]->station.id == id[i])
	    {
		sta = mp->sta[j];
		sta->station.sym.display = display;
		need_redisplay = True;
		sta_id = id[i];
	    }
	    src_id = -100;
	    for(j = 0; j < mp->nsrc; j++)
		if(mp->src[j]->source.id == id[i])
	    {
		src = mp->src[j];
		src->source.sym.display = display;
		need_redisplay = True;
		src_id = id[i];
	    }
	    for(j = 0; j < mp->narc; j++)
		if(mp->arc[j]->arc.id == id[i] ||
		   mp->arc[j]->sta_id == sta_id ||
		   mp->arc[j]->src_id == src_id)
	    {
		arc = mp->arc[j];
		if(display >= MAP_SELECTED_ON)
		{
		    if(arc->changed) MakeArc(w, arc);
		    DrawArc(w, d, arc);
		}
		arc->arc.line.display = display;
		need_redisplay = True;
	    }
	    for(j = 0; j < mp->ndel; j++)
		if(mp->del[j]->delta.id == id[i] ||
		   mp->del[j]->sta_id == sta_id ||
		   mp->del[j]->src_id == src_id)
	    {
		del = mp->del[j];
		if(display >= MAP_SELECTED_ON)
		{
		    if(del->changed) MakeDelta(w, del);
		    DrawDelta(w, d, del);
		}
		del->delta.line.display = display;
		need_redisplay = True;
	    }
	    for(j = 0; j < mp->nell; j++) if(mp->ell[j]->ellipse.id == id[i]
		 || mp->ell[j]->src_id == src_id)
	    {
		ell = mp->ell[j];
		if(display >= MAP_SELECTED_ON)
		{
		    if(ell->changed) MakeEllipse(w, ell);
		    Free(ell->s.segs);
		    ell->s.size_segs = 0;
		    ell->s.nsegs = 0;
		    d->s  = &ell->s;
		    DrawLonLat(w, d, ell->npts, ell->lon, ell->lat);
		}
		ell->ellipse.line.display = display;
		need_redisplay = True;
	    }

	    for(j = 0; j < mp->nlines; j++) if(mp->line[j]->line.id == id[i])
	    {
		l = mp->line[j];
		if(display >= MAP_SELECTED_ON)
		{
		    Free(l->s.segs);
		    l->s.size_segs = 0;
		    l->s.nsegs = 0;
		    d->s  = &l->s;
		    DrawLonLat(w, d, l->line.npts, l->line.lon, l->line.lat);
		}
		l->line.line.display = display;
		need_redisplay = True;
	    }
	    for(j = 0; j < mp->nsymgrp; j++)
		if(mp->symgrp[j]->group.id == id[i])
	    {
		s = mp->symgrp[j];
		s->group.sym.display = display;
		need_redisplay = True;
	    }
	    for(j = 0; j < mp->nsymbol; j++)
		if(mp->symbol[j].id == id[i])
	    {
		mp->symbol[j].sym.display = display;
		need_redisplay = True;
	    }
	    for(j = 0; j < mp->npoly; j++) if(mp->poly[j]->id == id[i])
	    {
		mp->poly[j]->sym.display = display;
		need_redisplay = True;
	    }
	    for(j = 0; j < mp->nrect; j++) if(mp->rect[j]->id == id[i])
	    {
		mp->rect[j]->sym.display = display;
		need_redisplay = True;
	    }
	}

	if(redisplay && need_redisplay)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplay(mp->utm_map, num_ids, id, display, redisplay);
	}
}

void
MapPlotDisplayArcs(MapPlotWidget w, char *label, int display)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	Boolean		redisplay;
	MapArc		*arc;

	redisplay = False;
	for(i = 0; i < mp->narc; i++)
	    if( mp->arc[i]->type != MAP_MEASURE &&
	        mp->arc[i]->type != MAP_CURSOR_MEASURE &&
		mp->arc[i]->arc.line.display != MAP_LOCKED_OFF &&
		mp->arc[i]->arc.line.display != MAP_LOCKED_ON &&
		mp->arc[i]->arc.line.display != display &&
		(label == NULL || (mp->arc[i]->arc.label != NULL &&
			!strcmp(mp->arc[i]->arc.label, label))))
	{
	    arc = mp->arc[i];
	    if(display == MAP_ON || display == MAP_SELECTED_ON)
	    {
		if(arc->changed) MakeArc(w, arc);
		DrawArc(w, &ax->d, arc);
	    }
	    arc->arc.line.display = display;
	    redisplay = True;
	}
	if(redisplay)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplayArcs(mp->utm_map, label, display);
	}
}

void
MapPlotDisplayDeltas(MapPlotWidget w, char *label, int display)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	Boolean		redisplay;
	MapDelta	*del;

	redisplay = False;
	for(i = 0; i < mp->ndel; i++)
	    if( mp->del[i]->type != MAP_MEASURE &&
	        mp->del[i]->type != MAP_CURSOR_MEASURE &&
		mp->del[i]->delta.line.display != MAP_LOCKED_OFF &&
		mp->del[i]->delta.line.display != MAP_LOCKED_ON &&
		mp->del[i]->delta.line.display != display &&
		(label == NULL || (mp->del[i]->delta.label != NULL &&
			!strcmp(mp->del[i]->delta.label, label))))
	{
	    del = mp->del[i];
	    if(display == MAP_ON || display == MAP_SELECTED_ON)
	    {
		if(del->changed) MakeDelta(w, del);
		DrawDelta(w, &ax->d, del);
	    }
	    del->delta.line.display = display;
	    redisplay = True;
	}
	if(redisplay)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplayDeltas(mp->utm_map, label, display);
	}
}

void
MapPlotDisplayEllipses(MapPlotWidget w, char *label, int display)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i;
	Boolean		redisplay;
	MapEllipse	*ell;

	redisplay = False;
	for(i = 0; i < mp->nell; i++)
	    if( mp->ell[i]->type != MAP_MEASURE &&
		mp->ell[i]->ellipse.line.display != MAP_LOCKED_OFF &&
		mp->ell[i]->ellipse.line.display != MAP_LOCKED_ON &&
		mp->ell[i]->ellipse.line.display != display &&
		(label == NULL || (mp->ell[i]->ellipse.label != NULL &&
			!strcmp(mp->ell[i]->ellipse.label, label))))
	{
	    ell = mp->ell[i];
	    if(display == MAP_ON || display == MAP_SELECTED_ON)
	    {
		if(ell->changed) MakeEllipse(w, ell);
		Free(ell->s.segs);
		ell->s.size_segs = 0;
		ell->s.nsegs = 0;
		ax->d.s  = &ell->s;
		DrawLonLat(w, &ax->d, ell->npts, ell->lon,ell->lat);
	    }
	    ell->ellipse.line.display = display;
	    redisplay = True;
	}
	if(redisplay)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplayEllipses(mp->utm_map, label, display);
	}
}

void
MapPlotDisplayStations(MapPlotWidget w, char *label, int display)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;
	Boolean		redisplay;

	redisplay = False;
	for(i = 0; i < mp->nsta; i++)
	    if( mp->sta[i]->station.sym.display != MAP_LOCKED_OFF &&
		mp->sta[i]->station.sym.display != MAP_LOCKED_ON &&
		mp->sta[i]->station.sym.display != display &&
		(label == NULL || (mp->sta[i]->station.label != NULL &&
			!strcmp(mp->sta[i]->station.label, label))))
	{
	    mp->sta[i]->station.sym.display = display;
	    redisplay = True;
	}
	if(redisplay)
	{
	    _MapPlotRedisplay(w);
	    _AxesRedisplayXor((AxesWidget)w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplayStations(mp->utm_map, label, display);
	}
}

void
MapPlotDisplaySources(MapPlotWidget w, char *label, int display)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;
	Boolean		redisplay;

	redisplay = False;
	for(i = 0; i < mp->nsrc; i++)
		if(mp->src[i]->source.sym.display != MAP_LOCKED_OFF &&
		   mp->src[i]->source.sym.display != MAP_LOCKED_ON &&
		   mp->src[i]->source.sym.display != display &&
			(label == NULL ||
			(mp->src[i]->source.label != NULL &&
			!strcmp(mp->src[i]->source.label, label))))
	{
		mp->src[i]->source.sym.display = display;
		redisplay = True;
	}
	if(redisplay)
	{
		_MapPlotRedisplay(w);
		_AxesRedisplayXor((AxesWidget)w);
	}
	if( mp->utm_map ) {
	    MapPlotDisplaySources(mp->utm_map, label, display);
	}
}

int
MapPlotGetStaDelta(MapPlotWidget w, char *sta, char *label, MapPlotDelta *delta)
{
	MapPlotPart	*mp = &w->map_plot;
	int		i;
	MapDelta	*d;
	MapStation	*s=NULL;

	for(i = 0; i < mp->nsta; i++)
	{
		if(mp->sta[i]->station.label != NULL && 
			!strcmp(mp->sta[i]->station.label, sta))
		{
			s = mp->sta[i];
			break;
		}
	}
	if(i == mp->nsta) return(0);

	for(i = 0; i < mp->ndel; i++)
	{
		d = mp->del[i];
		if(d->sta_id == s->station.id && d->delta.label != NULL
			&& !strcmp(d->delta.label,label))
			
		{
			*delta = mp->del[i]->delta;
			return(1);
		}
	}
	return(0);
}

/* associate and arc, delta or ellipse (assoc_id) with a station or source (id)
 */
int
MapPlotAssoc(MapPlotWidget w, int assoc_id, int id)
{
	MapPlotPart	*mp = &w->map_plot;
	int	 	i;
	Boolean		selected;
	MapArc		*a = NULL;
	MapDelta	*d = NULL;
	MapEllipse	*e = NULL;

	if( mp->utm_map ) {
	    MapPlotAssoc(mp->utm_map, assoc_id, id);
	}

	for(i = 0; i < mp->narc; i++)
	{
	    if(mp->arc[i]->arc.id == assoc_id)
	    {
		a = mp->arc[i];
		break;
	    }
	}
	if(i == mp->narc)
	{
	    for(i = 0; i < mp->ndel; i++)
	    {
		if(mp->del[i]->delta.id == assoc_id)
		{
		    d = mp->del[i];
		    break;
		}
	    }
	    if(i == mp->ndel)
	    {
		for(i = 0; i < mp->nell; i++)
		{
		    if(mp->ell[i]->ellipse.id == assoc_id)
		    {
			e = mp->ell[i];
			break;
		    }
		}
	    }
	}
	if(a == NULL && d == NULL && e == NULL) return(0);

	for(i = 0; i < mp->nsta; i++)
	{
	    if(mp->sta[i]->station.id == id)
	    {
		selected = mp->sta[i]->station.sym.selected;
		if(a != NULL)
		{
		    a->sta_id = id;
		    a->arc.line.selected = selected;
		}
		else if(d != NULL)
		{
		    d->sta_id = id;
		    d->delta.line.selected = selected;
		}
		return(1);
	    }
	}
	for(i = 0; i < mp->nsrc; i++)
	{
	    if(mp->src[i]->source.id == id)
	    {
		selected = mp->src[i]->source.sym.selected;
		if(a != NULL)
		{
		    a->src_id = id;
		    a->arc.line.selected = selected;
		}
		else if(d != NULL)
		{
		    d->src_id = id;
		    d->delta.line.selected = selected;
		}
		else
		{
		    e->src_id = id;
		    e->ellipse.line.selected = selected;
		}
		return(1);
	    }
	}
	return(0);
}

int
MapPlotGetArcs(MapPlotWidget w, MapPlotArc **arc)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*arc = (MapPlotArc *)AxesMalloc((Widget)w,
		mp->narc*sizeof(MapPlotArc))) ) return(0);

	for(i = 0; i < mp->narc; i++)
	{
		(*arc)[i] = mp->arc[i]->arc;
	}
	return(mp->narc);
}

static int
MapPlotChangeArc(MapPlotWidget w, MapPlotArc *arc, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, display;
	Boolean	old_xor;
	MapArc	*a;

	for(i = 0; i < mp->narc; i++)
	{
	    if(mp->arc[i]->arc.id == arc->id) break;
	}
	if(i == mp->narc) return(0);

	a = mp->arc[i];
	old_xor = a->arc.line.xorr;

	if(a->arc.label != arc->label)
	{
	    Free(a->arc.label);
	    memcpy(&a->arc, arc, sizeof(MapPlotArc));
	    if(arc->label != NULL)
	    {
		a->arc.label = strdup(arc->label);
	    }
	}
	else
	{
	    memcpy(&a->arc, arc, sizeof(MapPlotArc));
	}

	if(a->arc.line.display <= MAP_OFF || (!a->arc.line.selected
	    && a->arc.line.display == MAP_SELECTED_ON))
	{
	    return(0);
	}

	MakeArc(w, a);

	if(redisplay && a->arc.line.xorr)
	{
	    if(!old_xor)
	    {
		display = a->arc.line.display;
		a->arc.line.display = MAP_OFF;
		MapPlotUpdate(w);
		a->arc.line.display = display;
	    }
	    else
	    {
		RedisplayLine(w, XtWindow(w), &a->arc.line, &a->s);
	    }
	}

	if(!a->arc.line.xorr)
	{
	    DrawArc(w, &ax->d, a);
	}
	else
	{
	    Free(a->s.segs);
	    a->s.size_segs = 0;
	    a->s.nsegs = 0;
	    ax->d.s  = &a->s;
	    DrawLonLatXor(w, &ax->d, a->npts, a->lon, a->lat);
	}

	if(redisplay)
	{
	    if(!a->arc.line.xorr)
	    {
		MapPlotUpdate(w);
	    }
	    else
	    {
		RedisplayLine(w, XtWindow(w), &a->arc.line, &a->s);
	    }
	}
	return(1);
}

static int
MapPlotDeleteArc(MapPlotWidget w, int id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;

	for(i = 0; i < mp->narc; i++)
	{
		if(mp->arc[i]->arc.id == id) break;
	}
	if(i == mp->narc) return(0);

	DeleteArc(w, i);

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

/** 
 */
static void
MakeArc(MapPlotWidget w, MapArc *a)
{
    MapPlotPart *mp = &w->map_plot;
    int i;
    double rlat, rlon;
    double rtheta, rphi, alpha, beta, gamma, pphi, ptheta, theta, phi, dphi;

    a->changed = False;
    if(mp->projection != MAP_UTM_NEAR)
    {
	rlat = rad*geocentric(a->arc.lat);
	rlon = rad*a->arc.lon;
	rtheta = half_pi - rlat;
	rphi = rlon;

	/* Rotate the z-axis to rtheta, rphi, then the y-axis to north.
	 */
	alpha = rphi;
	beta = rtheta;
	gamma = (beta > 0) ? half_pi : -half_pi;
	/*
	 * Get the pole to the great circle specified by the azimuth in the
	 * rotated coordinate system.
	 */
	ptheta = half_pi;
	pphi = pi -  rad*a->arc.az;

	/*  The pole coordinates in the original system:
	 */
	euler(&ptheta, &pphi, -gamma, -beta, -alpha);

	/* now rotate the z-axis to the pole.
	 */
	alpha = pphi;
	beta = ptheta;
	gamma = 0.;
	/*
	 * get the receiver coordinates in this rotated system.
	 */
	theta = rtheta;
	phi = rphi;
	euler(&theta, &phi, alpha, beta, gamma);
	/*
	 * rotate the new x-axis to the receiver.
	 */
	gamma = phi;

	dphi = rad*a->arc.del/(a->npts-1);

	for(i = 0; i < a->npts; i++)
	{
		theta = half_pi;
		phi = i*dphi;
		euler(&theta, &phi, -gamma, -beta, -alpha);
		a->lat[i] = geographic(90. - theta/DEG_TO_RADIANS);
		a->lon[i] = phi/rad;
	}
    }
    else
    {
	double cs = cos(a->arc.az*rad);
	double sn = sin(a->arc.az*rad);
	double x, y, z;
	double meters = a->arc.del*rad*6371000.;

	if(meters > 10000000.) meters = 10000000.;
	dphi = meters/(a->npts-1);

	llToUtm(mp, a->arc.lon, a->arc.lat, &x, &y, &z);

	for(i = 0; i < a->npts; i++)
	{
	    a->lat[i] = y + cs*i*dphi;
	    a->lon[i] = x + sn*i*dphi;
	}
    }
}

static void
DrawArc(MapPlotWidget w, DrawStruct *d, MapArc *arc)
{
	MapPlotPart *mp = &w->map_plot;

	Free(arc->s.segs);
	arc->s.size_segs = 0;
	arc->s.nsegs = 0;
	d->s  = &arc->s;
	if(mp->projection == MAP_UTM_NEAR) {
	    int i;
	    if(arc->npts > 0) imove(d, arc->lon[0], arc->lat[0]);
	    for(i = 1; i < arc->npts; i++) {
		idraw(d, arc->lon[i], arc->lat[i]);
	    }
	    iflush(d);
	}
	else {
	    DrawLonLat(w, d, arc->npts, arc->lon, arc->lat);
	}
}

int
MapPlotAddDelta(MapPlotWidget w, MapPlotDelta *delta, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	MapDelta *d;
	MapDelta map_delta_init = MAP_DELTA_INIT;

	ALLOC((mp->ndel+1)*sizeof(MapDelta *),
		mp->size_del, mp->del, MapDelta *, 0);

	if( !(mp->del[mp->ndel] = (MapDelta *)AxesMalloc((Widget)w,
		sizeof(MapDelta))) ) return(0);

	d = mp->del[mp->ndel];
	mp->ndel++;

	memcpy(d, &map_delta_init, sizeof(MapDelta));

	memcpy(&d->delta, delta, sizeof(MapPlotDelta));
	d->delta.id = getMapId(mp);
	if(d->delta.label != NULL)
	{
		d->delta.label = strdup(d->delta.label);
	}
	d->npts = 200;
	d->lat = (double *)AxesMalloc((Widget)w, d->npts*sizeof(double));
	d->lon = (double *)AxesMalloc((Widget)w, d->npts*sizeof(double));

	if(d->delta.line.display <= MAP_OFF || (!d->delta.line.selected &&
		d->delta.line.display == MAP_SELECTED_ON))
	{
		return(d->delta.id);
	}
	if(d->delta.line.fg == w->core.background_pixel)
	{
		d->delta.line.fg = ax->fg;
	}

	MakeDelta(w, d);

	DrawDelta(w, &ax->d, d);

	if(redisplay && !ax->redisplay_pending)
	{
		RedisplayLine(w, XtWindow(w), &d->delta.line, &d->s);
		mp->need_redisplay = True;
	}
	if( mp->utm_map ) {
	    MapPlotPart *utm_mp = &mp->utm_map->map_plot;
	    utm_mp->next_id = d->delta.id;
	    MapPlotAddDelta(mp->utm_map, delta, redisplay);
	    utm_mp->next_id = -1;
	}
	return(d->delta.id);
}

int
MapPlotGetDeltas(MapPlotWidget w, MapPlotDelta **delta)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL) return(0);

	if( !(*delta = (MapPlotDelta *)AxesMalloc((Widget)w,
		mp->ndel*sizeof(MapPlotDelta))) ) return(0);

	for(i = 0; i < mp->ndel; i++)
	{
		(*delta)[i] = mp->del[i]->delta;
	}
	return(mp->ndel);
}

static int
MapPlotChangeSymbol(MapPlotWidget w, MapPlotSymbol *sym, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;
	
	for(i = 0; i < mp->nsymbol; i++)
	{
	    if(mp->symbol[i].id == sym->id) break;
	}
	if(i == mp->nsymbol) return(0);

	memcpy(mp->symbol+i, sym, sizeof(MapPlotSymbol));

	if(redisplay) {
	    MapPlotUpdate(w);
	}
	return(1);
}

void
MapPlotSelectSymbol(MapPlotWidget w, MapPlotSymbol *s, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	for(i = 0; i < mp->nsymbol; i++) {
	    if(mp->symbol[i].id == s->id) break;
	}
	if(i == mp->nsymbol) return;

	mp->symbol[i].sym.selected = s->sym.selected;
	if(redisplay) {
	    MapPlotUpdate(w);
	}
}

static int
MapPlotChangeDelta(MapPlotWidget w, MapPlotDelta *delta, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, display;
	Boolean	old_xor;
	MapDelta *d;

	for(i = 0; i < mp->ndel; i++)
	{
	    if(mp->del[i]->delta.id == delta->id) break;
	}
	if(i == mp->ndel) return(0);

	d = mp->del[i];
	old_xor = d->delta.line.xorr;

	if(d->delta.label != delta->label)
	{
	    Free(d->delta.label);
	    memcpy(&d->delta, delta, sizeof(MapPlotDelta));
	    if(delta->label != NULL)
	    {
		d->delta.label = strdup(delta->label);
	    }
	}
	else
	{
	    memcpy(&d->delta, delta, sizeof(MapPlotDelta));
	}

	if(d->delta.line.display <= MAP_OFF || (!d->delta.line.selected
		&& d->delta.line.display == MAP_SELECTED_ON))
	{
	    return(0);
	}

	MakeDelta(w, d);

	if(redisplay && d->delta.line.xorr)
	{
	    if(!old_xor)
	    {
		display = d->delta.line.display;
		d->delta.line.display = MAP_OFF;
		MapPlotUpdate(w);
		d->delta.line.display = display;
	    }
	    else
	    {
		RedisplayLine(w, XtWindow(w), &d->delta.line, &d->s);
	    }
	}
	Free(d->s.segs);
	d->s.size_segs = 0;
	d->s.nsegs = 0;
	ax->d.s  = &d->s;
	if(!d->delta.line.xorr) {
	    DrawDelta(w, &ax->d, d);
	}
	else {
	    DrawLonLatXor(w, &ax->d, d->npts, d->lon, d->lat);
	}

	if(redisplay)
	{
	    if(!d->delta.line.xorr)
	    {
		MapPlotUpdate(w);
	    }
	    else
	    {
		RedisplayLine(w, XtWindow(w), &d->delta.line, &d->s);
	    }
	}
	return(1);
}

static int
MapPlotDeleteDelta(MapPlotWidget w, int id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;

	for(i = 0; i < mp->ndel; i++)
	{
		if(mp->del[i]->delta.id == id) break;
	}
	if(i == mp->ndel) return(0);

	DeleteDelta(w, i);

	if(redisplay) MapPlotUpdate(w);
	return(1);
}

static void
MakeDelta(MapPlotWidget w, MapDelta *d)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;
	double	alpha, beta, gamma, theta, phi, dphi;

	d->changed = False;
	if(mp->projection != MAP_UTM_NEAR)
	{
	    /* Rotate the z-axis to circle center, then the y-axis to north.
	     */
	    alpha = DEG_TO_RADIANS*d->delta.lon;
	    beta = DEG_TO_RADIANS*(90. - geocentric(d->delta.lat));
	    gamma = (beta > 0) ? half_pi : -half_pi;

	    d->npts = 200;
	    Free(d->lat);
	    Free(d->lon);
	    d->lat = (double *)AxesMalloc((Widget)w, d->npts*sizeof(double));
	    d->lon = (double *)AxesMalloc((Widget)w, d->npts*sizeof(double));

	    dphi = DEG_TO_RADIANS*360./(d->npts-1);

	    for(i = 0; i < d->npts; i++)
	    {
		phi = i*dphi;
		theta = DEG_TO_RADIANS*d->delta.del;
		euler(&theta, &phi, -gamma, -beta, -alpha);
		d->lat[i] = geographic(90. - theta/DEG_TO_RADIANS);
		d->lon[i] = phi/DEG_TO_RADIANS;
	    }
	}
	else // the units are meters instead of degrees, and the projection is
	     // linear.
	{
	    double x, y, z;
	    llToUtm(mp, d->delta.lon, d->delta.lat, &x, &y, &z);
	    d->npts = 200;
	    Free(d->lat);
	    Free(d->lon);
	    d->lat = (double *)AxesMalloc((Widget)w, d->npts*sizeof(double));
	    d->lon = (double *)AxesMalloc((Widget)w, d->npts*sizeof(double));

	    dphi = 360./(d->npts-1);
	    for(i = 0; i < d->npts; i++)
	    {
		d->lon[i] = x + d->delta.del*cos(i*dphi*rad);
		d->lat[i] = y + d->delta.del*sin(i*dphi*rad);
	    }
	}
}

int
MapPlotDelete(MapPlotWidget w, int id, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if( mp->utm_map ) {
	    MapPlotDelete(mp->utm_map, id, redisplay);
	}

	for(i = 0; i < mp->nsrc; i++)
	{
	    if(id == mp->src[i]->source.id)
	    {
		return(MapPlotDeleteSource(w, id, redisplay));
	    }
	}
	for(i = 0; i < mp->nsta; i++)
	{
	    if(id == mp->sta[i]->station.id)
	    {
		return(MapPlotDeleteStation(w, id, redisplay));
	    }
	}
	for(i = 0; i < mp->narc; i++)
	{
	    if(id == mp->arc[i]->arc.id)
	    {
		return(MapPlotDeleteArc(w, id, redisplay));
	    }
	}
	for(i = 0; i < mp->nell; i++)
	{
	    if(id == mp->ell[i]->ellipse.id)
	    {
		return(MapPlotDeleteEllipse(w, id, redisplay));
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	{
	    if(id == mp->del[i]->delta.id)
	    {
		return(MapPlotDeleteDelta(w, id, redisplay));
	    }
	}
	for(i = 0; i < mp->nsymgrp; i++)
	{
	    if(id == mp->symgrp[i]->group.id)
	    {
		return(MapPlotDeleteSymbolGroup(w, i, redisplay));
	    }
	}
	for(i = 0; i < mp->nlines; i++)
	{
	    if(id == mp->line[i]->line.id)
	    {
		return(MapPlotDeleteLine(w, i, redisplay));
	    }
	}
	for(i = 0; i < mp->npoly; i++)
	{
	    if(id == mp->poly[i]->id)
	    {
		return(MapPlotDeletePoly(w, i, redisplay));
	    }
	}
	for(i = 0; i < mp->nrect; i++)
	{
	    if(id == mp->rect[i]->id)
	    {
		return(MapPlotDeleteRectangle(w, i, redisplay));
	    }
	}
	for(i = 0; i < mp->nthemes; i++)
	{
	    if(id == mp->theme[i]->id)
	    {
		return(MapPlotDeleteTheme(w, id, redisplay));
	    }
	}
	for(i = 0; i < mp->nsymbol; i++)
	{
	    if(id == mp->symbol[i].id)
	    {
		return(MapPlotDeleteSymbol(w, i, redisplay));
	    }
	}
	return(0);
}

void
MapPlotClearOverlays(MapPlotWidget w, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if( mp->utm_map ) {
	    MapPlotClearOverlays(mp->utm_map, redisplay);
	}

	for(i = 0; i < mp->nsrc; i++)
	{
            Free(mp->src[i]->source.label);
            Free(mp->src[i]);
        }
        mp->nsrc = 0;

	for(i = 0; i < mp->nsta; i++)
	{
            Free(mp->sta[i]->station.label);
            Free(mp->sta[i]);
	}
        mp->nsta = 0;

	for(i = 0; i < mp->narc; i++)
	{
            Free(mp->arc[i]->arc.label);
            Free(mp->arc[i]->lon);
            Free(mp->arc[i]->lat);
            Free(mp->arc[i]->s.segs);
            Free(mp->arc[i]);
	}
        mp->narc = 0;

	for(i = 0; i < mp->nell; i++)
	{
            Free(mp->ell[i]->ellipse.label);
            Free(mp->ell[i]->lon);
            Free(mp->ell[i]->lat);
            Free(mp->ell[i]->s.segs);
            Free(mp->ell[i]);
	}
        mp->nell = 0;

	for(i = 0; i < mp->ndel; i++)
	{
            Free(mp->del[i]->delta.label);
            Free(mp->del[i]->lon);
            Free(mp->del[i]->lat);
            Free(mp->del[i]->s.segs);
            Free(mp->del[i]);
	}
        mp->ndel = 0;

	for(i = 0; i < mp->nsymgrp; i++)
	{
            Free(mp->symgrp[i]->group.label);
            Free(mp->symgrp[i]->group.lon);
            Free(mp->symgrp[i]->group.lat);
            Free(mp->symgrp[i]->group.size);
            Free(mp->symgrp[i]->p);
            Free(mp->symgrp[i]);
	}
        mp->nsymgrp = 0;

	for(i = 0; i < mp->nlines; i++)
	{
            Free(mp->line[i]->line.label);
            Free(mp->line[i]->line.lon);
            Free(mp->line[i]->line.lat);
            Free(mp->line[i]->s.segs);
            Free(mp->line[i]);
	}
        mp->nlines = 0;

	for(i = 0; i < mp->npoly; i++)
	{
            Free(mp->poly[i]->label);
            Free(mp->poly[i]->lon);
            Free(mp->poly[i]->lat);
            Free(mp->poly[i]);
	}
        mp->npoly = 0;

	for(i = 0; i < mp->nrect; i++)
	{
            Free(mp->rect[i]->label);
            Free(mp->rect[i]);
	}
        mp->nrect = 0;

        mp->nsymbol = 0;

        if(redisplay) MapPlotUpdate(w);
}

/** 
 */
int
MapPlotChange(MapPlotWidget w, MapObject *obj, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if( mp->utm_map ) {
	    MapPlotChange(mp->utm_map, obj, redisplay);
	}
	for(i = 0; i < mp->nsrc; i++)
	{
	    if(obj->id == mp->src[i]->source.id)
	    {
		return(MapPlotChangeSource(w, &obj->src, redisplay));
	    }
	}
	for(i = 0; i < mp->narc; i++)
	{
	    if(obj->id == mp->arc[i]->arc.id)
	    {
		return(MapPlotChangeArc(w, &obj->arc, redisplay));
	    }
	}
	for(i = 0; i < mp->nell; i++)
	{
	    if(obj->id == mp->ell[i]->ellipse.id)
	    {
		return(MapPlotChangeEllipse(w, &obj->ell, redisplay));
	    }
	}
	for(i = 0; i < mp->ndel; i++)
	{
	    if(obj->id == mp->del[i]->delta.id)
	    {
		return(MapPlotChangeDelta(w, &obj->del, redisplay));
	    }
	}
	for(i = 0; i < mp->nsta; i++)
	{
	    if(obj->id == mp->sta[i]->station.id)
	    {
		return(MapPlotChangeStation(w, &obj->sta, redisplay));
	    }
	}
	for(i = 0; i < mp->nsymbol; i++)
	{
	    if(obj->id == mp->symbol[i].id)
	    {
		return(MapPlotChangeSymbol(w, &obj->sym, redisplay));
	    }
	}
/****
	for(i = 0; i < mp->nsymgrp; i++)
	{
	    if(obj->id == mp->symgrp[i]->group.id)
	    {
		return(MapPlotChangeSymbols(w, &obj->sym, redisplay));
	    }
	}
	for(i = 0; i < mp->nlines; i++)
	{
	    if(obj->id == mp->line[i]->line.id)
	    {
		return(MapPlotChangeLine(w, &obj->lin, redisplay));
	    }
	}
****/
	return(0);
}

void
MapPlotUpdate(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	if(!XtIsRealized((Widget)w)) return;

	RedisplayThemes(w);
	_MapPlotRedisplay(w);
	_AxesRedisplayXor((AxesWidget)w);

	if( mp->utm_map ) {
	    MapPlotUpdate(mp->utm_map);
	}
}

void
MapPlotCircles(MapPlotWidget w, MapCircle *circle, int ncircles)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i;

	Free(mp->circle);
	Free(mp->circle_start);
	Free(mp->num_circle_segs);
	mp->primary_circle = -1;

	if(ncircles > 0)
	{
	    mp->circle = (MapCircle *)AxesMalloc((Widget)w,
				ncircles*sizeof(MapCircle));
	    memcpy(mp->circle, circle, ncircles*sizeof(MapCircle));
	    mp->circle_start = (int *)AxesMalloc((Widget)w,
				ncircles*sizeof(int));
	    mp->num_circle_segs = (int *)AxesMalloc((Widget)w,
				ncircles*sizeof(int));
	}

	for(i = 0; i < ncircles; i++)
	{
	    if(mp->circle[i].lon > 180.)
	    {
		mp->circle[i].lon -=
		(int)((mp->circle[i].lon - 180.)/360. + 1)*360.;
	    }
	    else if(mp->circle[i].lon < -180.)
	    {
		mp->circle[i].lon +=
		(int)((-mp->circle[i].lon - 180.)/360. + 1)*360;
	    }
	}
	if(!XtIsRealized((Widget)w))
	{
	    mp->ncircles = ncircles;
	    return;
	}
	if(mp->display_circles == MAP_SELECTED_ON)
	{
	    for(i = 0; i < ncircles; i++)
	    {
		mp->circle[i].display = mp->circle[i].selected;
		if(mp->circle[i].selected && i != mp->primary_circle)
		{
		    mp->circle[i].selected =False;
		}
	    }
	}
	if(mp->ncircles > 0 && mp->display_circles != MAP_OFF)
	{
	    mp->ncircles = ncircles;
	    DrawCircles(w);
	    if(!ax->redisplay_pending)
	    {
		_MapPlotRedisplay(w);
	    }
	}
	else
	{
	    mp->ncircles = ncircles;
	    DrawCircles(w);
	}
	if( mp->utm_map ) {
	    MapPlotCircles(mp->utm_map, circle, ncircles);
	}
}

Boolean
MapPlotAssocSta(MapPlotWidget w, int src_id, int nsta, int *sta_id,
		Boolean delete_index, Boolean redisplay)
{
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, k;
	Boolean		change = False;
	double		rlat, rlon, slat, slon, delta, az, baz;
	MapSource	*src;
	MapArc		*a;
	MapStation	*sta=NULL;
	MapPlotArc	arc;
	LineInfo line_info_init = LINE_INFO_INIT;

	if( mp->utm_map ) {
	    MapPlotAssocSta(mp->utm_map, src_id, nsta, sta_id, delete_index,
			redisplay);
	}

	for(i = 0; i < mp->nsrc; i++)
	{
//	    if(mp->src[i]->source.orid == src_orid) break;
	    if(mp->src[i]->source.id == src_id) break;
	}
	if(i == mp->nsrc) return False;
	src_id = mp->src[i]->source.id;

	src = mp->src[i];

	/* first delete all associated arcs not in current input.
	 */
	if(delete_index)
	{
	    for(i = 0; i < mp->narc; i++)
		if(mp->arc[i]->src_id == src_id && mp->arc[i]->type == MAP_PATH)
	    {
		a = mp->arc[i];
		for(j = 0; j < nsta; j++)
		{
		    if(sta_id[j] == a->sta_id) break;
		}
		if(j == nsta)
		{
		    DeleteArc(w, i);
		    change = True;
		}
	    }
	}
	/* add new associated stations
	*/
	for(i = 0; i < nsta; i++)
	{
	    for(j = 0; j < mp->narc; j++)
	    {
		if(mp->arc[j]->src_id == src_id &&
		   mp->arc[j]->sta_id == sta_id[i])  break;
	    }
	    if(j == mp->narc)
	    {
		for(k = 0; k < mp->nsta; k++)
		{
		    sta = mp->sta[k];
		    if(sta->station.id == sta_id[i]) break;
		}
		if(k == mp->nsta) continue;
		change = True;

		slat = src->source.lat;
		slon = src->source.lon;
		rlat = sta->station.lat;
		rlon = sta->station.lon;
		deltaz(slat, slon, rlat, rlon, &delta, &az, &baz);

		arc.label = sta->station.label;
		arc.lat = src->source.lat;
		arc.lon = src->source.lon;
		arc.del = delta;
		arc.az = az;
		memcpy(&arc.line, &line_info_init, sizeof(LineInfo));
		if(sta->station.sym.display > MAP_LOCKED_OFF &&
		   sta->station.sym.display < MAP_LOCKED_ON)
		{
		    arc.line.display = mp->display_paths;
		}
		else
		{
		    arc.line.display = sta->station.sym.display;
		}
		arc.line.fg = src->source.sym.fg;
		arc.line.selected = (src->source.sym.selected ||
			sta->station.sym.selected) ? True : False;
		MapPlotAddArc(w, &arc, False);
		a = mp->arc[mp->narc-1];
		a->type = MAP_PATH;
		a->src_id = src->source.id;
		a->sta_id = sta_id[i];
	    }
	}
	if(change)
	{
	    DrawArcs(w, &ax->d);
	    if(redisplay)
	    {
		MapPlotUpdate(w);
	    }
	}
	return(change);
}

static void
DeleteStation(MapPlotWidget w, int j)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->nsta) return;

	Free(mp->sta[j]->station.label);
	Free(mp->sta[j]);
	for(i = j; i < mp->nsta-1; i++)
	{
		mp->sta[i] = mp->sta[i+1];
	}
	mp->nsta--;
}

static void
DeleteSource(MapPlotWidget w, int j)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->nsrc) return;

	Free(mp->src[j]->source.label);
	Free(mp->src[j]);
	for(i = j; i < mp->nsrc-1; i++)
	{
		mp->src[i] = mp->src[i+1];
	}
	mp->nsrc--;
}

static void
DeleteArc(MapPlotWidget w, int j)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->narc) return;

	Free(mp->arc[j]->arc.label);
	Free(mp->arc[j]->lon);
	Free(mp->arc[j]->lat);
	Free(mp->arc[j]->s.segs);
	Free(mp->arc[j]);
	for(i = j; i < mp->narc-1; i++)
	{
		mp->arc[i] = mp->arc[i+1];
	}
	mp->narc--;
}

static void
DeleteEllipse(MapPlotWidget w, int j)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->nell) return;

	Free(mp->ell[j]->ellipse.label);
	Free(mp->ell[j]->lon);
	Free(mp->ell[j]->lat);
	Free(mp->ell[j]->s.segs);
	Free(mp->ell[j]);
	for(i = j; i < mp->nell-1; i++)
	{
		mp->ell[i] = mp->ell[i+1];
	}
	mp->nell--;
}

static void
DeleteDelta(MapPlotWidget w, int j)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(j < 0 || j >= mp->ndel) return;

	Free(mp->del[j]->delta.label);
	Free(mp->del[j]->lon);
	Free(mp->del[j]->lat);
	Free(mp->del[j]->s.segs);
	Free(mp->del[j]);
	for(i = j; i < mp->ndel-1; i++)
	{
		mp->del[i] = mp->del[i+1];
	}
	mp->ndel--;
}

static void
DeleteTheme(MapPlotWidget w, int i)
{
	MapPlotPart *mp = &w->map_plot;
	int j;

	if(i < 0 || i >= mp->nthemes) return;

	for(j = 0; j < mp->theme[i]->theme.nshapes; j++) {
	    ThemeDisplay *td = &mp->theme[i]->td[j];
	    SHPDestroyObject(mp->theme[i]->theme.shapes[j]);
	    Free(td->fill_segs.segs);
	    Free(td->bndy_segs.segs);
	    Free(td->p.points);
	    Free(td->lat);
	    Free(td->lon);
	}
	Free(mp->theme[i]->theme.type_name);
	Free(mp->theme[i]->theme.shapes);
	Free(mp->theme[i]->theme.shape_fg);
	Free(mp->theme[i]->theme.color_scale.pixels);
	Free(mp->theme[i]->theme.color_scale.lines);
	Free(mp->theme[i]->td);

	Free(mp->theme[i]->map_image.label);
	if( !mp->theme[i]->map_image.copy ) {
	    Free(mp->theme[i]->map_image.m.x);
	    Free(mp->theme[i]->map_image.m.y);
	    Free(mp->theme[i]->map_image.m.z);
	}
	Free(mp->theme[i]->map_image.s.segs);
	Free(mp->theme[i]->map_image.t.segs);
	Free(mp->theme[i]->map_image.l.ls);

	delete mp->theme[i];

	for(j = i; j < mp->nthemes-1; j++)
	{
	    mp->theme[j] = mp->theme[j+1];
	}
	mp->nthemes--;
}

Widget
MapPlotCreate(
	Widget		parent,
	String		name,
	ArgList		arglist,
	Cardinal	argcount)
{
	return (XtCreateWidget(name, mapPlotWidgetClass, parent, 
		arglist, argcount));
}

void
MapChangeStationColor(MapPlotWidget w, Pixel color, char *sta)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nsta; i++)
		if(mp->sta[i]->station.label != NULL && 
			!strcmp(mp->sta[i]->station.label, sta))
	{
		mp->sta[i]->station.sym.fg = color;
		_MapPlotRedisplay(w);
		_AxesRedisplayXor((AxesWidget)w);
		break;
	}
	if( mp->utm_map ) {
	    MapChangeStationColor(mp->utm_map, color, sta);
	}
}

void
MapPlotChangeStationColor(MapPlotWidget w, int id, Pixel fg, Boolean redisplay)
{
	MapPlotPart *mp = &w->map_plot;
	int i;

	if(w == NULL || !XtIsRealized((Widget)w)) return;

	for(i = 0; i < mp->nsta; i++)
	{
	    if(id == mp->sta[i]->station.id) break;
	}
	if(i < mp->nsta)
	{
	    mp->sta[i]->station.sym.fg = fg;
	    if(redisplay)
	    {
		_MapPlotRedisplay(w);
		_AxesRedisplayXor((AxesWidget)w);
	    }
	}
	if( mp->utm_map ) {
	    MapPlotChangeStationColor(mp->utm_map, id, fg, redisplay);
	}
}

#define		UNKNOWN		-1
#define		OUTSIDE_LIMITS	0
#define		USED		1
#define		OCCUPIED	10

static void
DrawStationTags(MapPlotWidget w, Window window)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int	i, j, ii, ascent, descent, direction;
	int	x0, x1, y0, y1, orig_x0, orig_x1, orig_y0, orig_y1;
	int	*label_status = NULL, index, jj, kk;
	int	*center_x = NULL, *center_y = NULL;
	double	x, y, z;
	XCharStruct overall;
	MapStation *sta, *sta2;
	SymbolInfo *sym;

	if(!XtIsRealized((Widget)w) || !mp->display_station_tags) return;

	if( !(label_status = (int *)AxesMalloc((Widget)w, mp->nsta*sizeof(int)))
	 || !(center_x = (int *)AxesMalloc((Widget)w, mp->nsta*sizeof(int))) ||
	    !(center_y = (int *)AxesMalloc((Widget)w, mp->nsta*sizeof(int))) )
	{
	    return;
	}

	for(i = 0; i < mp->nsta; i++)
	    if(mp->sta[i]->visible && mp->sta[i]->station.label != NULL
		    && mp->sta[i]->station.sym.display >= MAP_ON)
	{
	    sta = mp->sta[i];
	    sym = &sta->station.sym;

	    sta->placement.position[0] = UNKNOWN;
	    sta->placement.position[1] = UNKNOWN;
	    sta->placement.position[2] = UNKNOWN;
	    sta->placement.position[3] = UNKNOWN;

	    XTextExtents(mp->label_font, sta->station.label,
			(int)strlen(sta->station.label),
			&direction, &ascent, &descent, &overall);

	    project(w, sta->station.lon, sta->station.lat, &x, &y, &z);
	    center_x[i] = unscale_x(&ax->d, x);
	    center_y[i] = unscale_y(&ax->d, y);
	    sta->placement.text_width = overall.width;
	    sta->placement.text_height = overall.ascent + overall.descent+2;
	    sta->placement.symbol_x0 = center_x[i] - sym->size/2;
	    sta->placement.symbol_x1 = center_x[i] + sym->size/2;
	    sta->placement.symbol_y0 = center_y[i] + sym->size/2;
	    sta->placement.symbol_y1 = center_y[i] - sym->size/2;
	}

	for(i = 0; i < mp->nsta; i++)
	    if(mp->sta[i]->visible && mp->sta[i]->station.label != NULL
		&& mp->sta[i]->station.sym.display >= MAP_ON)
	{
	    sta = mp->sta[i];
	    sym = &sta->station.sym;

	    for(j = 0; j < 4; j++)
	    {
		label_position (center_x[i], center_y[i], 
			sta->placement.text_width, sta->placement.text_height,
			j, (int)(sym->size/2), &x0, &x1, &y0, &y1);

		if(inside_limits(x0, x1, y0, y1, ax->clipx1, ax->clipx2,
			ax->clipy1, ax->clipy2))
		{
		    for(ii = 0; ii < i; ii++)
		    {
			if(have_overlap(x0, x1, y0, y1,
				mp->sta[ii]->placement.label_x0,
				mp->sta[ii]->placement.label_x1,
				mp->sta[ii]->placement.label_y0,
				mp->sta[ii]->placement.label_y1))
			{
			    sta->placement.position[j] = OCCUPIED + ii;
			    break;
			}
		    }
		    if(ii == i)
		    {
			sta->placement.position[j] = USED;
		    }
		}
		else
		{
		    sta->placement.position[j] = OUTSIDE_LIMITS;
		}
		if(sta->placement.position[j] == USED) break;
	    }
	    label_status[i] = j;
	    sta->placement.label_x0 = x0;
	    sta->placement.label_x1 = x1;
	    sta->placement.label_y0 = y0;
	    sta->placement.label_y1 = y1;
	}
	for(i = 0; i < mp->nsta; i++)
	    if(mp->sta[i]->visible && mp->sta[i]->station.label != NULL
		&& mp->sta[i]->station.sym.display >= MAP_ON
		&& label_status[i] == 4)
	{
	    sta = mp->sta[i];
	    sym = &sta->station.sym;
	    for(ii=0; ii<4; ii++)
	    {
		if(sta->placement.position[ii] >= OCCUPIED)
		{
		    index = sta->placement.position[ii] - OCCUPIED;
		    sta2 = mp->sta[index];
		    for(jj = 0; jj < 4; jj++)
		    {
			if(sta2->placement.position[jj] == UNKNOWN)
			{
			    label_position(center_x[index], center_y[index],
					sta2->placement.text_width,
					sta2->placement.text_height, jj,
					(int)(sta2->station.sym.size/2),
					&x0, &x1, &y0, &y1);

			    if(inside_limits(x0, x1, y0, y1, ax->clipx1,
					ax->clipx2, ax->clipy1, ax->clipy2))
			    {
				for(kk = 0; kk < mp->nsta; kk++)
				    if(mp->sta[kk]->visible &&
					mp->sta[kk]->station.label != NULL)
				{
				    if(kk != index && have_overlap(x0,x1,y0,y1,
					mp->sta[kk]->placement.label_x0,
					mp->sta[kk]->placement.label_x1,
					mp->sta[kk]->placement.label_y0,
					mp->sta[kk]->placement.label_y1))
				    {
					sta2->placement.position[jj] =
								OCCUPIED + kk;
					break;
				    }	
				}
				if(kk == mp->nsta) 
				{
				    sta2->placement.position[jj] = USED;
				}
			    }
			    else
			    {
				sta2->placement.position[jj] = OUTSIDE_LIMITS;
			    }
			    /*
			     * following condition is passed IF
			     * we appear to have a suitable place.
			     * But now we must see if we're tramping on
			     * another label.  
			     */
			    if(sta2->placement.position[jj] == USED)
			    {
				label_position(center_x[i], center_y[i],
				    sta->placement.text_width,
				    sta->placement.text_height, ii,
				    (int)(sym->size/2),
				    &orig_x0, &orig_x1, &orig_y0, &orig_y1);
				if(CheckLabel(w, orig_x0, orig_x1, 
				  	  orig_y0, orig_y1, i, index))
				{
				    sta2->placement.label_x0 = x0;
				    sta2->placement.label_x1 = x1;
				    sta2->placement.label_y0 = y0;
				    sta2->placement.label_y1 = y1;
				    label_status[index] = jj;
				    label_status[i] = ii;
				    break;
				}
			    }
			}	/* end of if UNKNOWN */
		    }	/* end of jj loop */
		}
		if(label_status[i] == ii)
		{
		    label_position (center_x[i], center_y[i], 
			    sta->placement.text_width,
			    sta->placement.text_height, ii,
			    (int)(sym->size/2), &x0, &x1, &y0, &y1);
		    sta->placement.label_x0 = x0;
		    sta->placement.label_x1 = x1;
		    sta->placement.label_y0 = y0;
		    sta->placement.label_y1 = y1;
		    break;
		}
	    }   /* end of ii loop */
	}
	Free(label_status);
	Free(center_x);
	Free(center_y);

	for(i = 0; i < mp->nsta; i++)
	    if(mp->sta[i]->visible && mp->sta[i]->station.label != NULL
		&& mp->sta[i]->station.sym.display >= MAP_ON)
	{
	    sta = mp->sta[i];
	    if(!sta->station.sym.selected)
	    {
//		XSetForeground(XtDisplay(w), mp->symbolGC, sta->station.sym.fg);
		XSetForeground(XtDisplay(w), mp->symbolGC, ax->fg);
	    }
	    else
	    {
		XSetForeground(XtDisplay(w), mp->symbolGC, ax->select_fg);
	    }
	    XDrawString(XtDisplay(w), window, mp->symbolGC,
			sta->placement.label_x0, sta->placement.label_y1,
			sta->station.label, (int)strlen(sta->station.label));
	}
}

static int
CheckLabel(MapPlotWidget w, int orig_x0, int orig_x1, int orig_y0, int orig_y1,
			int ii, int jj)
{
	MapPlotPart *mp = &w->map_plot;
	int	i;

	for(i = 0; i < mp->nsta; i++) if(mp->sta[i]->visible
	    && mp->sta[i]->station.label != NULL)
	{
	    if(i != ii && i != jj &&
		have_overlap(orig_x0, orig_x1, orig_y0, orig_y1,
		mp->sta[i]->placement.label_x0,
		mp->sta[i]->placement.label_x1,
		mp->sta[i]->placement.label_y0,
		mp->sta[i]->placement.label_y1)) return(False);
	}
	return(True);
}

static void		
label_position(int center_x, int center_y, int width, int height, int position,
		int half_size, int *x0, int *x1, int *y0, int *y1)
{
	if(position == 0)
	{
	    *x0 = center_x - width/2;
	    *x1 = center_x + width/2;
	    *y0 = center_y ;
	    *y1 = center_y + half_size + height;
	}
	else if(position == 1)
	{
	    *x0 = center_x - width/2;
	    *x1 = center_x + width/2;
	    *y0 = center_y - half_size - height;
	    *y1 = center_y - half_size ;
	}
	else if(position == 2)
	{
	    *x0 = center_x + half_size;
	    *x1 = center_x + width + half_size;
	    *y0 = center_y - height/2;
	    *y1 = center_y + height/2;
	}
	else if(position == 3)
	{
	    *x0 = center_x - width - half_size;
	    *x1 = center_x - half_size;
	    *y0 = center_y - height/2;
	    *y1 = center_y + height/2;
	}
}

static int		
have_overlap(int ax0, int ax1, int ay0, int ay1, int bx0, int bx1, int by0,
		int by1)
{
	if(( (ax0 <= bx1 && ax0 >= bx0) ||
	     (ax1 <= bx1 && ax1 >= bx0) ||
	     (ax0 <= bx0 && ax1 >= bx1)) &&
	   ( (ay0 <= by1 && ay0 >= by0) ||
	     (ay1 <= by1 && ay1 >= by0) ||
	     (ay0 <= by0 && ay1 >= by1)) ) return(True);
	return (False);
}

static int		
inside_limits(int x0, int x1, int y0, int y1, int clipx0, int clipx1,
		int clipy0, int clipy1)
{
	if(x0 >= clipx0 && x1 <= clipx1 &&
	   y0 >= clipy0 && y1 <= clipy1) return(True);
	return(False);
}

static void
MapPlotHardCopy(AxesWidget widget, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, PrintParam *p)
{
	MapPlotWidget	w = (MapPlotWidget)widget;
	MapPlotPart	*mp = &w->map_plot;
	AxesPart	*ax = &w->axes;
	int		i, j, xmin, ymin, xmax, ymax;
	SegArray	s;

	AxesWaitCursor((Widget)widget, True);

/*	if(mp->map_color_fill) p->color = True; */
	p->color = True;

	d->s = &s;
	d->s->segs = NULL;
	d->s->size_segs = 0;
	d->s->nsegs = 0;

	SetClipArea(d, ax->x1[ax->zoom], ax->y1[ax->zoom],
		ax->x2[ax->zoom], ax->y2[ax->zoom]);

	if(a->axis_segs[0].n_segs > 0 || a->axis_segs[1].n_segs > 0 ||
	   a->axis_segs[2].n_segs > 0 || a->axis_segs[3].n_segs > 0)
	{
	    xmin = unscale_x(d, ax->x1[ax->zoom]);
	    ymin = unscale_y(d, ax->y1[ax->zoom]);
	    xmax = unscale_x(d, ax->x2[ax->zoom]);
	    ymax = unscale_y(d, ax->y2[ax->zoom]);

	    fprintf(fp, "N %d %d M\n", xmin, ymax);
	    fprintf(fp, "%d %d d\n", xmax, ymax);
	    fprintf(fp, "%d %d d\n", xmax, ymin);
	    fprintf(fp, "%d %d d\n", xmin, ymin);
	    fprintf(fp, "%d %d d\n", xmin, ymax);
	    fprintf(fp, "closepath clip\n");
	    if(p->color) {
		fprintf(fp, "gsave clip ");
		AxesPostColor((AxesWidget)w, fp, mp->map_water_color);
		fprintf(fp, "fill grestore 1 setgray stroke\n");
	    }
	}
	if(p->color) {
	    FillBackground(w, d);
	    if(s.nsegs > 0) {
		fprintf(fp, "1 setgray\n");	/* white */
		fprintf(fp, "%d %d M\n", s.segs[0].x1, s.segs[0].y1);
		fprintf(fp, "%d %d D\n", s.segs[0].x2, s.segs[0].y2);
		for(i = 1; i < s.nsegs; i++)
		{
		    if(	s.segs[i].x1 != s.segs[i-1].x2 ||
			s.segs[i].y1 != s.segs[i-1].y2)
		    {
			fprintf(fp, "%d %d M\n", s.segs[i].x1, s.segs[i].y1);
		    }
		    fprintf(fp, "%d %d D\n", s.segs[i].x2, s.segs[i].y2);
		}
	    }
	    Free(s.segs);
	    d->s = &s;
	    d->s->segs = NULL;
	    d->s->size_segs = 0;
	    d->s->nsegs = 0;
	}

	fprintf(fp, "/%s findfont %d scalefont setfont\n",
				p->fonts.tag_font, p->fonts.tag_fontsize);

	for(i = 0; i < mp->nthemes; i++)
	    if(mp->theme[i]->display && mp->theme[i]->delta_on)
	{
	    MapTheme *t = mp->theme[i];
	    XColor color_info;
	    for(j = 0; j < t->theme.color_scale.num_colors; j++) {
		color_info.pixel = t->theme.color_scale.pixels[j];
		XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color_info);
		fprintf(fp, "/c%d { %f %f %f setrgbcolor } def\n", j,
			(float)color_info.red/65535.,
			(float)color_info.green/65535.,
			(float)color_info.blue/65535.);
	    }
	    color_info.pixel = w->core.background_pixel;
	    XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color_info);

	    fprintf(fp,"/c%d { %f %f %f setrgbcolor } def\n",
			t->theme.color_scale.num_colors,
			(float)color_info.red/65535.,
			(float)color_info.green/65535.,
			(float)color_info.blue/65535.);

	    if(t->theme_type == THEME_SHAPE) {
		DrawShapeTheme(w, d, mp->theme[i]);
		HardDrawTheme(w, fp, d, t, font_scale);
	    }
	    else if(t->theme_type == THEME_IMAGE) {
		HardDrawImageTheme(w, fp, d, mp->theme[i]);
	    }

	    if(t->color_bar_on) {
		HardDrawBar(w, fp, d, a, font_scale, &p->fonts,
				&t->theme.color_scale);
	    }
	}

	if(p->color) {
	    AxesPostColor((AxesWidget)w, fp,
			StringToPixel((Widget)widget, (char *)"Black"));
	}

	if(mp->projection != MAP_POLAR) {
	    DrawGrid(w, d, a, 10);
	}
	else {
	    DrawPolarGrid(w, d, 10);
	    a->axis_segs[0].n_segs = 0;
	    a->axis_segs[1].n_segs = 0;
	    a->axis_segs[2].n_segs = 0;
	    a->axis_segs[3].n_segs = 0;
	    a->nxlab = 0;
	    a->nylab = 0;
	}
	if(mp->map_display_grid)
	{
	    PostSegs(w, fp, d->s, p->color, ax->fg, NULL);
	}
	if(mp->projection == MAP_POLAR) {
	    HardPolarGridLabels(w, fp, d, a, font_scale,
		mp->numaz, mp->az, mp->numdist, mp->dist);
	}

	DrawLines(w, d);
	for(i = 0; i < mp->nlines; i++)
	    if(mp->line[i]->line.line.display >= MAP_ON ||
	      (mp->line[i]->line.line.display == MAP_SELECTED_ON
			&& mp->line[i]->line.line.selected))
	{
	    PostSegs(w, fp, &mp->line[i]->s, p->color,mp->line[i]->line.line.fg,
			&mp->line[i]->line.line);
	}

	for(i = 0; i < mp->nsymgrp; i++)
	    if(mp->symgrp[i]->group.sym.display >= MAP_ON ||
	      (mp->symgrp[i]->group.sym.display == MAP_SELECTED_ON
			&& mp->symgrp[i]->group.sym.selected))
	{
	    DrawHardSymbols(w, fp, d, mp->symgrp[i], p->color);
	}

	for(i = 0; i < mp->nsymbol; i++)
            if( mp->symbol[i].sym.display >= MAP_ON ||
		(mp->symbol[i].sym.display == MAP_SELECTED_ON &&
                        mp->symbol[i].sym.selected))
	{
	    DrawHardSymbol(w, fp, d, mp->symbol[i].lon, mp->symbol[i].lat,
			&mp->symbol[i].sym, p->color);
        }

	DrawEllipses(w, d);
	for(i = 0; i < mp->nell; i++)
	    if(mp->ell[i]->ellipse.line.display >= MAP_ON ||
	      (mp->ell[i]->ellipse.line.display == MAP_SELECTED_ON
			&& mp->ell[i]->ellipse.line.selected))
	{
	    PostSegs(w, fp, &mp->ell[i]->s, p->color,
		mp->ell[i]->ellipse.line.fg, &mp->ell[i]->ellipse.line);
	}

	DrawArcs(w, d);
	for(i = 0; i < mp->narc; i++)
		if(mp->arc[i]->arc.line.display >= MAP_ON ||
		   (mp->arc[i]->arc.line.display == MAP_SELECTED_ON
			&& mp->arc[i]->arc.line.selected))
	{
	    PostSegs(w, fp, &mp->arc[i]->s, p->color,
			mp->arc[i]->arc.line.fg, &mp->arc[i]->arc.line);
	}

	DrawDeltas(w, d);
	for(i = 0; i < mp->ndel; i++)
		if(mp->del[i]->delta.line.display >= MAP_ON ||
		   (mp->del[i]->delta.line.display == MAP_SELECTED_ON
			&& mp->del[i]->delta.line.selected))
	{
	    PostSegs(w, fp, &mp->del[i]->s, p->color,
			mp->del[i]->delta.line.fg, &mp->del[i]->delta.line);
	}

	fprintf(fp, "2 slw\n");	
	d->s = &s;
	for(i = 0; i < mp->nsta; i++)
		if(mp->sta[i]->station.sym.display >= MAP_ON ||
		   (mp->sta[i]->station.sym.display == MAP_SELECTED_ON
			&& mp->sta[i]->station.sym.selected))
	{
	    if(DrawHardSymbol(w, fp, d, mp->sta[i]->station.lon,
		mp->sta[i]->station.lat, &mp->sta[i]->station.sym, p->color)
		&& mp->display_station_tags)
	    {
		DrawHardTag(w, fp, d, mp->sta[i], font_scale, p->fonts.tag_font,
			p->fonts.tag_fontsize);
	    }
	}
	for(i = 0; i < mp->nsrc; i++)
		if(mp->src[i]->source.sym.display >= MAP_ON ||
		   (mp->src[i]->source.sym.display == MAP_SELECTED_ON
			&& mp->src[i]->source.sym.selected))
	{
	    if(DrawHardSymbol(w, fp, d, mp->src[i]->source.lon,
		mp->src[i]->source.lat, &mp->src[i]->source.sym, p->color)
		&& mp->display_source_tags && mp->src[i]->source.label != NULL)
	    {
		fprintf(fp, "%.5f (%s) PL\n", font_scale,
				mp->src[i]->source.label);
	    }
	}
	for(i = 0; i < mp->npoly; i++)
		if(mp->poly[i]->sym.display >= MAP_ON ||
		   (mp->poly[i]->sym.display == MAP_SELECTED_ON
			&& mp->poly[i]->sym.selected))
	{
	    DrawHardPoly(w, fp, d, mp->poly[i], p->color);
	}
	for(i = 0; i < mp->nrect; i++)
		if(mp->rect[i]->sym.display >= MAP_ON ||
		   (mp->rect[i]->sym.display == MAP_SELECTED_ON
			&& mp->rect[i]->sym.selected))
	{
/*	    DrawHardRectangle(w, fp, d, mp->rect[i], p->color); */
	}
	fprintf(fp, "initclip\n");

	Free(s.segs);
	s.size_segs = 0;
	s.nsegs = 0;

	if(p->color) {
	    AxesPostColor((AxesWidget)w, fp,
			StringToPixel((Widget)widget, (char *)"Black"));
	}

	DrawBorder(w, d);
	PostSegs(w, fp, d->s, p->color, ax->fg, NULL);

/*****
	if(mp->display_circles != MAP_OFF)
	{
		DrawCircles(w);
	}
****/

	_MapPlotRedraw(w);

	AxesWaitCursor((Widget)widget, False);
}

static void
HardDrawTheme(MapPlotWidget w, FILE *fp, DrawStruct *d, MapTheme *t,
		float font_scale)
{
	AxesWidget aw = (AxesWidget)w;
	AxesPart *ax = &w->axes;
	int i;

	if(t->theme.shape_type == SHPT_POLYGON) {
	    Pixel black = BlackPixelOfScreen(XtScreen(w));
	    Pixel white = WhitePixelOfScreen(XtScreen(w));
	    if(t->theme.fill)
	    {
		for(i = 0; i < t->theme.nshapes; i++) if(t->td[i].display)
		{
		    if(!t->td[i].selected) {
			AxesPostColor(aw, fp, t->theme.shape_fg[i]);
		    }
		    else {
			if(ax->select_fg != t->theme.shape_fg[i]) {
			    AxesPostColor(aw, fp, ax->select_fg);
			}
			else if(black != t->theme.shape_fg[i]) {
			    AxesPostColor(aw, fp, black);
			}
			else {
			    AxesPostColor(aw, fp, white);
			}
                    }
		    HardMapSegs(fp, &t->td[i].fill_segs);
		}
	    }
	    if(t->theme.bndry)
	    {
		AxesPostColor(aw, fp, t->theme.bndry_fg);
		for(i = 0; i < t->theme.nshapes; i++) {
		    HardMapSegs(fp, &t->td[i].bndy_segs);
		}
	    }
	    if(t->display_labels)
	    {
		AxesPostColor(aw, fp, ax->fg);
		for(i = 0; i < t->theme.nshapes; i++)
		    if(t->td[i].display && t->td[i].display_label)
		{
		    ThemeDisplay *td = &t->td[i];
		    int x = unscale_x(d, td->label_x);
		    int y = unscale_y(d, td->label_y);
		    fprintf(fp, "%d %d M\n", x, y);
		    fprintf(fp, "(%s) %.5f PC\n", td->label, font_scale);
		}
	    }
	}
}

static void
HardDrawBar(MapPlotWidget w, FILE *fp, DrawStruct *d, AxesParm *a,
		float font_scale, AxesFontStruct *fonts, ColorScale *c)
{
	MapPlotPart *mp = &w->map_plot;
	int i, n, x, y, idif, iy1, iy2, last_y, bar_width, skip;
	char lab[200];
	float dely;

	n = c->num_colors;

	if(n <= 0) return;

	iy1 = unscale_y(d, w->axes.y1[w->axes.zoom]);
	iy2 = unscale_y(d, w->axes.y2[w->axes.zoom]);
	if(iy1 > iy2) {
	    i = iy1;
	    iy1 = iy2;
	    iy2 = i;
	}
	idif = iy2 - iy1;
	if(idif <= 0) return;

	x = w->core.width - mp->bar_width;
	x = (int)(d->ix2 + .05*DOTS_PER_INCH);
	bar_width = (int)(.2*DOTS_PER_INCH);

	dely = (float)idif/n;

	last_y = iy1;

	fprintf(fp, "initclip\n");

	for(i = 0; i < n; i++)
	{
	    fprintf(fp, "c%d ", i);

	    y = (int)(iy1 + (i+1)*dely);
	    fprintf(fp, "%d %d %d %d rectfill\n", x, y, bar_width, last_y - y);
	    last_y = y;
	}
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		fonts->axis_font, fonts->axis_fontsize);
	fprintf(fp, "0 setgray\n");

	x += (int)(bar_width + .05*DOTS_PER_INCH);
	if(n > 15) {
	    skip = n/8;
	    if(skip <= 0) skip = 1;
	}
	else skip = 1;

	for(i = 0; i <= n; i += skip)
	{
	    y = (int)(iy1 + i*dely - .5*a->axis_font_height);
	    fprintf(fp, "N %d %d M\n", x, y);
	    ftoa(c->lines[i], mp->ndeci, 1, lab, 200);
	    fprintf(fp, "(%s) %.5f P\n", lab, font_scale);
	}
	if(i != n+skip) {
	    y = (int)(iy1 + n*dely - .5*a->axis_font_height);
	    fprintf(fp, "N %d %d M\n", x, y);
	    ftoa(c->lines[n], mp->ndeci, 1, lab, 200);
	    fprintf(fp, "(%s) %.5f P\n", lab, font_scale);
	}
}

static void
PostSegs(MapPlotWidget w, FILE *fp, SegArray *s, Boolean do_color, Pixel color,
		LineInfo *line)
{
	if(s->nsegs <= 0) return;

	if(line == NULL || line->line_width <= 1) {
	    fprintf(fp, " 1 slw \n");
	}
	else {
	    fprintf(fp, " %d slw \n", line->line_width*5);
	}
	if(do_color) {
	    AxesPostColor((AxesWidget)w, fp, color);
	}
	HardMapSegs(fp, s);
}

static void
HardMapSegs(FILE *fp, SegArray *s)
{
	int i;

	if(s->nsegs > 0) {
	    fprintf(fp, "%d %d M\n", s->segs[0].x1, s->segs[0].y1);
	    fprintf(fp, "%d %d D\n", s->segs[0].x2, s->segs[0].y2);
	}
	for(i = 1; i < s->nsegs; i++)
	{
	    if(s->segs[i].x1 != s->segs[i-1].x2 ||
	       s->segs[i].y1 != s->segs[i-1].y2)
	    {
		fprintf(fp, "%d %d M\n", s->segs[i].x1, s->segs[i].y1);
	    }
	    fprintf(fp, "%d %d D\n", s->segs[i].x2, s->segs[i].y2);
	}
}

static void
DrawHardTag(MapPlotWidget w, FILE *fp, DrawStruct *d, MapStation *sta,
		float font_scale, char *tag_font, int tag_fontsize)
{
	int	type, siz;
	int	center_x, center_y, radius;
	double	x, y, z;

	if(!sta->visible || !sta->station.sym.display
	    || sta->station.label == NULL
	    || (int)strlen(sta->station.label) <= 0) return;

	fprintf(fp, "/%s findfont %d scalefont setfont\n",
				tag_font, tag_fontsize);

	type = sta->station.sym.type;

	project(w, sta->station.lon, sta->station.lat, &x, &y, &z);
	center_x = unscale_x(d, x);
	center_y = unscale_y(d, y);

	if(type == CIRCLE || type == FILLED_CIRCLE)
	{
	    radius = (int)(sta->station.sym.size*1.875);

	    if( sta->placement.tag_loc == NOPREF ||
		sta->placement.tag_loc == BOTTOM )
	    {
		fprintf(fp, "%d %d M\n", center_x,
			center_y-30-tag_fontsize-radius);
		fprintf(fp, "(%s) %.5f PC\n", sta->station.label, font_scale);
	    }
	    else if(sta->placement.tag_loc == TOP)
	    {
		fprintf(fp, "%d %d M\n", center_x, center_y+7+radius);
		fprintf(fp, "(%s) %.5f PC\n", sta->station.label, font_scale);
	    }
	    else if(sta->placement.tag_loc == LEFT)
	    {
		fprintf(fp, "%d %d M\n", center_x-7-radius, center_y);
		fprintf(fp, "%.5f (%s) PR\n", font_scale, sta->station.label);
	    }
	    else if(sta->placement.tag_loc == RIGHT)
	    {
		fprintf(fp, "%d %d M\n", center_x+radius, center_y);
		fprintf(fp, "%.5f (%s) PL\n", font_scale, sta->station.label);
	    }
	    return;
	}

	siz = (int)(3.75*sta->station.sym.size);

	if(sta->placement.tag_loc == NOPREF ||
	   sta->placement.tag_loc == BOTTOM )
	{
	    fprintf(fp, "%d %d M\n", center_x, center_y-siz-30-tag_fontsize);
	    fprintf(fp, "(%s) %.5f PC\n", sta->station.label, font_scale);
	}	
	else if(sta->placement.tag_loc == TOP)
	{
	    fprintf(fp, "%d %d M\n", center_x, center_y+siz+20);
	    fprintf(fp, "(%s) %.5f PC\n", sta->station.label, font_scale);
	}	
	else if(sta->placement.tag_loc == LEFT)
	{
	    fprintf(fp, "%d %d M\n", center_x-siz-5, center_y);
	    fprintf(fp, "%.5f (%s) PR\n", font_scale, sta->station.label);
	}	
	else if(sta->placement.tag_loc == RIGHT)
	{
	    fprintf(fp, "%d %d M\n", center_x+siz+5, center_y);
	    fprintf(fp, "%.5f (%s) PL\n", font_scale, sta->station.label);
	}
}

static void
reduce_lon(double *lon)
{
	if(*lon > 180.)
	{
	    *lon -= (int)((*lon - 180.)/360. + 1)*360.;
	}
	else if(*lon < -180.)
	{
	    *lon += (int)((-*lon - 180.)/360. + 1)*360;
	}
}

static int
getMapId(MapPlotPart *mp)
{
	static int id = 0;

	if(mp->next_id > 0) {
	    return mp->next_id;
	}
	else if(++id <= 0) {
	    id = 1;
	}
	return(id);
}

static void
CvStringToDouble(XrmValuePtr *args, Cardinal *num_args, XrmValuePtr fromVal,
		XrmValuePtr toVal)
{
        char    *s = fromVal->addr;     /* string to convert */
        static double   val;            /* returned value */
	/* need the following, even though we have stdlib.h */

        if (s == NULL) val = 0.0;
        else val = atof (s);
        toVal->size = sizeof(val);
        toVal->addr = (char*)(XtPointer) &val;
}

#define Z(i,j) a->z[i+(j)*a->nx]

static void
shadeImageProjected(MapPlotWidget w, XImage *image, DrawStruct *d,
	Pixel no_data_color, int num_colors, Pixel *colors, double *lines,
	Matrx *a, int *x0, int *y0, int *xwidth, int *yheight)
{
#ifdef HAVE_GSL
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i, j, k, klower, kupper, kmiddle, last_k;
	int i1=0, i2=0, j1, j2, ix, iy, imin, imax, jmin, jmax;
	float aij, last_aij=0.;
	double xmin, xmax, x, y, x1, x2, xp, yp, lim, lim2;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];

	if(mp->projection == MAP_ORTHOGRAPHIC) {
	    lim = 1.;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT) {
	    lim = .95*pi;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA) {
	    lim = .95*2.;
	}
	else if(mp->projection == MAP_UTM_NEAR) {
	    lim = 0.;
	    i1 = d->ix1;
	    i2 = d->ix2;
	}
	else {
	    return;
	}
	lim2 = lim*lim;

	if(d->iy1 < d->iy2) {
	    j1 = d->iy1;
	    j2 = d->iy2;
	}
	else {
	    j1 = d->iy2;
	    j2 = d->iy1;
	}

	last_k = -1;

	imin = 10000000;
	imax = -1;
	jmin = j1;
	jmax = j2;

	for(j = j1; j <= j2; j++)
	{
	    y = scale_y(d, j);
	    if(mp->projection != MAP_UTM_NEAR)
	    {
		if(y <= -lim || y >= lim) continue;

		x1 = -sqrt(lim2 - y*y);
		if(x1 < xmin) x1 = xmin;
		x2 = -x1;
		if(x2 > xmax) x2 = xmax;
		i1 = unscale_x(d, x1);
		i2 = unscale_x(d, x2);
	    }

	    for(i = i1; i <= i2; i++)
	    {
		x = scale_x(d, i);
		unproject(w, x, y, &xp, &yp);

		ix = gsl_interp_bsearch(a->x, xp, 0, a->nx-1);
		if(ix < a->nx-1) {
		    if(a->x[ix+1] - x < x - a->x[ix]) ix++;
		}

		iy = gsl_interp_bsearch(a->y, yp, 0, a->ny-1);
		if(iy < a->ny-1) {
		    if(a->y[iy+1] - y < y - a->y[iy]) iy++;
		}

		aij = Z(ix, iy);

		if(aij == a->exc) {
		    XPutPixel(image, i, j, no_data_color);
		    if(i < imin) imin = i;
		    else if(i > imax) imax = i;
		    if(j < jmin) jmin = j;
		    else if(j > jmax) jmax = j;
		    continue;
		}

		if(last_k >= 0 && aij == last_aij) {
		    k = last_k;
		}
		else
		{
		    k = -1;
		    if(aij <= lines[0]) {
			k = 0;
		    }
		    else if(aij >= lines[num_colors]) {
			k = num_colors-1;
		    }
		    else 
		    {
			klower = -1; kupper = num_colors+1;
			while(kupper - klower > 1)
			{
			    kmiddle = (klower + kupper)/2;
			    if(aij > lines[kmiddle]) {
				klower = kmiddle;
			    }
			    else {
				kupper = kmiddle;
			    }
			}
			k = klower;
		    }
		}
		XPutPixel(image, i, j, colors[k]);
		if(i < imin) imin = i;
		else if(i > imax) imax = i;
		if(j < jmin) jmin = j;
		else if(j > jmax) jmax = j;
		last_k = k;
		last_aij = aij;
	    }
	}
	*x0 = imin;
	*y0 = jmin;
	*xwidth = imax - imin + 1;
	*yheight = jmax - jmin + 1;

	if(mp->projection != MAP_UTM_NEAR)
	{
	    /* fill outside of the globe */
	    for(j = jmin; j <= jmax; j++)
	    {
		y = scale_y(d, j);
		if(y <= -lim || y >= lim)
		{
		    for(i = imin; i <= imax; i++) {
			XPutPixel(image, i, j, w->core.background_pixel);
		    }
		}
		else
		{
		    x = -sqrt(lim2 - y*y);
		    if(x >= xmin)
		    {
			i1 = unscale_x(d, x);
			for(i = imin; i <= i1; i++) {
			    XPutPixel(image, i, j, w->core.background_pixel);
			}
		    }
		    x = -x;
		    if(xmax >= x)
		    {
			i2 = unscale_x(d, x);
			for(i = i2; i <= imax; i++) {
			    XPutPixel(image, i, j, w->core.background_pixel);
			}
		    }
		}
	    }
	}
#else
fprintf(stderr, "Operation unavailable without libgsl.\n");
#endif
}

static void
HardDrawImageTheme(MapPlotWidget w, FILE *fp, DrawStruct *d, MapTheme *t)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	double x1, y1, x2, y2, xdif;
	MapImage *mi = &t->map_image;

	if(mi->m.nx <= 0 || mi->m.ny <= 0 || mp->projection ==MAP_POLAR) return;

	unproject(w, ax->x1[ax->zoom], ax->y1[ax->zoom], &x1, &y1);
	unproject(w, ax->x2[ax->zoom], ax->y2[ax->zoom], &x2, &y2);

	t->delta_on = False;

	xdif = fabs(x2 - x1);
	if(xdif > t->theme.on_delta || xdif < t->theme.off_delta) return;
	t->delta_on = True;

	if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
	   mp->projection ==MAP_UTM ||
	   mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
	   mp->projection == MAP_MERCATOR)
	{
	    int i;
	    double *x = NULL, *y = NULL;
	    x1 = w->axes.x1[w->axes.zoom];
	    x2 = w->axes.x2[w->axes.zoom];
	    y1 = w->axes.y1[w->axes.zoom];
	    y2 = w->axes.y2[w->axes.zoom];

	    /* Check if the image wraps around the left or right limits.
	     * This will occur after a rotation, or simply because the
	     * input data has limits outside of -180 to 180, like 100. to 240.
	     * Also, make this check for latitude. Can have 1,2 or 4 parts.
	     */
	    mi->nparts = 0;
	    if(mi->m.x[0] < x1 && mi->m.x[0] + 360. < x2)
	    {
		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
		HardDoYCase(w, fp, d, t, mi);

		if( !(x = (double *)AxesMalloc((Widget)w,
			mi->m.nx*sizeof(double))) ) return;
		memcpy(x, mi->m.x, mi->m.nx*sizeof(double));
		for(i = 0; i < mi->m.nx; i++) mi->m.x[i] = x[i] + 360.;

		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
		HardDoYCase(w, fp, d, t, mi);
		memcpy(mi->m.x, x, mi->m.nx*sizeof(double));
		Free(x);
	    }
	    else if(mi->m.x[mi->m.nx-1] > x2 &&
			mi->m.x[mi->m.nx-1] - 360. > x1)
	    {
		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
		HardDoYCase(w, fp, d, t, mi);

		if( !(x = (double *)AxesMalloc((Widget)w,
			mi->m.nx*sizeof(double))) ) return;
		memcpy(x, mi->m.x, mi->m.nx*sizeof(double));
		for(i = 0; i < mi->m.nx; i++) mi->m.x[i] = x[i] - 360.;

		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
		HardDoYCase(w, fp, d, t, mi);
		memcpy(mi->m.x, x, mi->m.nx*sizeof(double));
		Free(x);
	    }
	    else if(mi->m.y[0] < y1 && mi->m.y[0] + 180. < y2)
	    {
		HardDrawThemeImage(w, fp, d, t, &mi->nparts);

		if( !(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
		memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
		for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] + 180.;

		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
		memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
		Free(y);
	    }
	    else if(mi->m.y[mi->m.ny-1] > y2 && mi->m.y[mi->m.ny-1] - 180. > y1)
	    {
		HardDrawThemeImage(w, fp, d, t, &mi->nparts);

		if( !(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
		memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
		for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] - 180.;

		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
		memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
		Free(y);
	    }
	    else {
		HardDrawThemeImage(w, fp, d, t, &mi->nparts);
	    }
	}
	else {
	    HardShadeImageProjected(w, fp, d, w->core.background_pixel,
		t->theme.color_scale.num_colors,
		t->theme.color_scale.lines, &mi->m,
		&mi->display_x0[0], &mi->display_y0[0],
		&mi->display_width[0], &mi->display_height[0]);
	    mi->nparts = 1;
	}
}

static void
HardDoYCase(MapPlotWidget w, FILE *fp, DrawStruct *d, MapTheme *t, MapImage *mi)
{
	int i;
	double *y = NULL;
	double y1 = w->axes.y1[w->axes.zoom];
	double y2 = w->axes.y2[w->axes.zoom];

	if(mi->m.y[0] < y1 && mi->m.y[0] + 180. < y2)
	{
	    if( !(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
	    memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
	    for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] + 180.;
	    HardDrawThemeImage(w, fp, d, t, &mi->nparts);
	    memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
	    Free(y);
	}
	else if(mi->m.y[mi->m.ny-1] > y2 && mi->m.y[mi->m.ny-1] - 180. > y1)
	{
	    if( !(y = (double *)AxesMalloc((Widget)w,
			mi->m.ny*sizeof(double))) ) return;
	    memcpy(y, mi->m.y, mi->m.ny*sizeof(double));
	    for(i = 0; i < mi->m.ny; i++) mi->m.y[i] = y[i] - 180.;
	    HardDrawThemeImage(w, fp, d, t, &mi->nparts);
	    memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
	    Free(y);
	}
}

static void
HardDrawThemeImage(MapPlotWidget w, FILE *fp, DrawStruct *d, MapTheme *t,
			int *part)
{
	MapPlotPart *mp = &w->map_plot;
	int i, xmin, xmax, ymin, ymax;
	LabelArray *l;
	MapImage *mi = &t->map_image;

	if(mi->m.nx <= 0 || mi->m.ny <= 0) return;
	
	if(mi->mode != CONTOURS_ONLY)
	{
	    mi->m.x1 = w->axes.x1[w->axes.zoom];
	    mi->m.x2 = w->axes.x2[w->axes.zoom];
	    mi->m.y1 = w->axes.y1[w->axes.zoom];
	    mi->m.y2 = w->axes.y2[w->axes.zoom];
	    xmin = unscale_x(d, w->axes.x1[w->axes.zoom]);
	    xmax = unscale_x(d, w->axes.x2[w->axes.zoom]);
	    ymin = unscale_y(d, w->axes.y1[w->axes.zoom]);
	    ymax = unscale_y(d, w->axes.y2[w->axes.zoom]);

	    if(mp->projection == MAP_LINEAR_CYLINDRICAL ||
		mp->projection == MAP_UTM)
	    {
		shadePS(fp, d, t->theme.color_scale.num_colors,
		    t->theme.color_scale.lines, &mi->m,
		    xmin, xmax, ymin, ymax);
		(*part)++;
	    }
	    else if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA ||
			mp->projection == MAP_MERCATOR)
	    {
		double *y=NULL, ym;

		if( !(y = (double *)AxesMalloc((Widget)w,
				mi->m.ny*sizeof(double))) ) return;
		memcpy(y, mi->m.y, mi->m.ny*sizeof(double));

		if(mp->projection == MAP_CYLINDRICAL_EQUAL_AREA) {
		    for(i = 0; i < mi->m.ny; i++) {
			mi->m.y[i] = sin(y[i]*rad)/rad;
		    }
		}
		else {
		    for(i = 0; i < mi->m.ny; i++) {
			ym = y[i];
			if(ym > mp->mercator_max_lat) {
			    ym = mp->mercator_max_lat;
			}
			else if(ym < -mp->mercator_max_lat) {
			    ym = -mp->mercator_max_lat;
			}
			mi->m.y[i] = log(tan(.5*ym*rad + M_PI_4))/rad;
		    }
		}
		shadePS(fp, d, t->theme.color_scale.num_colors,
		    t->theme.color_scale.lines, &mi->m,
		    xmin, xmax, ymin, ymax);
		(*part)++;
		memcpy(mi->m.y, y, mi->m.ny*sizeof(double));
		Free(y);
	    }
	}
	if(mi->mode != COLOR_ONLY)
	{
	    d->s = &mi->s;
	    d->l = &mi->l;
	    mi->m.x1 = w->axes.x1[w->axes.zoom];
	    mi->m.x2 = w->axes.x2[w->axes.zoom];
	    mi->m.y1 = w->axes.y1[w->axes.zoom];
	    mi->m.y2 = w->axes.y2[w->axes.zoom];
	    condata(d, (float)mp->contour_label_size, &mi->m,
			mi->auto_ci, &mi->ci, mi->c_min, mi->c_max);
	    d->s = &mi->t;
	    l = &mi->l;
	    for(i = 0; i < l->n_ls; i++)
	    {
		mapalf(d, l->ls[i].x, l->ls[i].y, l->ls[i].size,
			l->ls[i].angle, 0, l->ls[i].label);
	    }
	}
}

static void
HardShadeImageProjected(MapPlotWidget w, FILE *fp, DrawStruct *d,
	Pixel no_data_color, int num_colors, double *lines,
	Matrx *a, int *x0, int *y0, int *xwidth, int *yheight)
{
#ifdef HAVE_GSL
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;
	int i, j, k, klower, kupper, kmiddle, last_k, last_color;
	int i1, i2, j1, j2, ix, iy, imin, imax, jmin, jmax;
	float aij, last_aij=0.;
	double xmin, xmax, x, y, x1, x2, xp, yp, lim, lim2;

	xmin = ax->x1[ax->zoom];
	xmax = ax->x2[ax->zoom];

	if(mp->projection == MAP_ORTHOGRAPHIC) {
	    lim = 1.;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUIDISTANT) {
	    lim = .95*pi;
	}
	else if(mp->projection == MAP_AZIMUTHAL_EQUAL_AREA) {
	    lim = .95*2.;
	}
	else {
	    return;
	}
	lim2 = lim*lim;

	if(d->iy1 < d->iy2) {
	    j1 = d->iy1;
	    j2 = d->iy2;
	}
	else {
	    j1 = d->iy2;
	    j2 = d->iy1;
	}

	last_k = -1;
	last_color = -1;

	imin = 10000000;
	imax = -1;
	jmin = j1;
	jmax = j2;

	fprintf(fp, "/p { y 1 1 rectfill } def\n");

	for(j = j1; j <= j2; j++)
	{
	    y = scale_y(d, j);
	    if(y <= -lim || y >= lim) continue;

	    fprintf(fp, "/y %d def\n", j);

	    x1 = -sqrt(lim2 - y*y);
	    if(x1 < xmin) x1 = xmin;
	    x2 = -x1;
	    if(x2 > xmax) x2 = xmax;

	    i1 = unscale_x(d, x1);
	    i2 = unscale_x(d, x2);
	    for(i = i1; i <= i2; i++)
	    {
		x = scale_x(d, i);
		unproject(w, x, y, &xp, &yp);

		ix = gsl_interp_bsearch(a->x, xp, 0, a->nx-1);
		if(ix < a->nx-1) {
		    if(a->x[ix+1] - x < x - a->x[ix]) ix++;
		}

		iy = gsl_interp_bsearch(a->y, yp, 0, a->ny-1);
		if(iy < a->ny-1) {
		    if(a->y[iy+1] - y < y - a->y[iy]) iy++;
		}

		aij = Z(ix, iy);

		if(aij == a->exc) {
		    AxesPostColor((AxesWidget)w, fp, no_data_color);
		    fprintf(fp, "%d %d p\n", i, j);
		    if(i < imin) imin = i;
		    else if(i > imax) imax = i;
		    if(j < jmin) jmin = j;
		    else if(j > jmax) jmax = j;
		    continue;
		}

		if(last_k >= 0 && aij == last_aij) {
		    k = last_k;
		}
		else
		{
		    k = -1;
		    if(aij <= lines[0]) {
			k = 0;
		    }
		    else if(aij >= lines[num_colors]) {
			k = num_colors-1;
		    }
		    else 
		    {
			klower = -1; kupper = num_colors+1;
			while(kupper - klower > 1)
			{
			    kmiddle = (klower + kupper)/2;
			    if(aij > lines[kmiddle]) {
				klower = kmiddle;
			    }
			    else {
				kupper = kmiddle;
			    }
			}
			k = klower;
		    }
		}
		if(k != last_color) {
		    fprintf(fp, "c%d ", k);
		    last_color = k;
		}
		fprintf(fp, "%d p ", i);
		if(i < imin) imin = i;
		else if(i > imax) imax = i;
		if(j < jmin) jmin = j;
		else if(j > jmax) jmax = j;
		last_k = k;
		last_aij = aij;
	    }
	    fprintf(fp, "\n");
	}
	*x0 = imin;
	*y0 = jmin;
	*xwidth = imax - imin + 1;
	*yheight = jmax - jmin + 1;
#endif
}

static void
LeaveWindow(MapPlotWidget w, XEvent *event, String *params,Cardinal *num_params)
{
	MapPlotPart *mp = &w->map_plot;
	AxesPart *ax = &w->axes;

	if(mp->utm_letter != '\0') {
	    XDrawSegments(XtDisplay(w), XtWindow(w), ax->mag_invertGC,
		mp->utm_segs, 4);
	    mp->utm_letter = '\0';
	    mp->utm_zone = -1;
	}
}

static Boolean
selectUTMCell(MapPlotWidget w)
{
	MapPlotPart *mp = &w->map_plot;
	if(mp->projection != MAP_UTM || mp->utm_letter == '\0'
		|| (mp->utm_letter == mp->utm_cell_letter
		&& mp->utm_zone == mp->utm_cell_zone) ) return False;

	if(mp->utm_letter != '\0' && mp->utm_zone > 0) {
	    mp->utm_select_letter = mp->utm_letter;
	    mp->utm_select_zone = mp->utm_zone;
	}
	else {
	    mp->utm_select_letter = '\0';
	    mp->utm_select_zone = -1;
	}
	return True;
}

void
MapPlotLinkMap(MapPlotWidget w, MapPlotWidget utm_map)
{
	MapPlotPart *mp = &w->map_plot;
	MapPlotPart *utm_mp = &utm_map->map_plot;
	int i;

	w->map_plot.utm_map = utm_map;

	MapPlotClear(utm_map);

	for(i = 0; i < mp->nthemes; i++)
	{
	    utm_mp->next_id = mp->theme[i]->id;
	    if(mp->theme[i]->theme_type == THEME_IMAGE)
	    {
		MapImage *mi = &mp->theme[i]->map_image;
		MapPlotAddImageCopy(utm_map, mi->label, &mi->m, mi->mode,
				&mp->theme[i]->theme, False);
	    }
	    else {
		MapPlotAddTheme(utm_map, &mp->theme[i]->theme, False);
	    }
	    MapPlotThemeColorScale(utm_map, mp->theme[i]->id,
			&mp->theme[i]->theme.color_scale);
	    if(mp->theme[i]->color_bar_on) {
		MapPlotThemeColorBar(utm_map, mp->theme[i]->id);
	    }
	}
	for(i = 0; i < mp->nlines; i++) {
	    utm_mp->next_id = mp->line[i]->line.id;
	    MapPlotAddLine(utm_map, &mp->line[i]->line, False);
	}

	if(mp->nsymbol > 0) {

	    ALLOC((utm_mp->nsymbol+mp->nsymbol)*sizeof(MapPlotSymbol),
		    utm_mp->size_symbol, utm_mp->symbol, MapPlotSymbol, 0);

	    for(i = 0; i < mp->nsymbol; i++) {
		memcpy(utm_mp->symbol+utm_mp->nsymbol, mp->symbol+i,
			sizeof(MapPlotSymbol));
		utm_mp->nsymbol++;
	    }
	    if(utm_mp->projection == MAP_POLAR) {
		_AxesRedraw((AxesWidget)utm_map);
		_MapPlotRedraw(utm_map);
	    }
	}

	for(i = 0; i < mp->nsymgrp; i++) {
	    utm_mp->next_id = mp->symgrp[i]->group.id;
	    MapPlotAddSymbolGroup(utm_map, &mp->symgrp[i]->group, False);
	}
	for(i = 0; i < mp->nsta; i++) {
	    utm_mp->next_id = mp->sta[i]->station.id;
	    MapPlotAddStation(utm_map, &mp->sta[i]->station, False);
	}
	for(i = 0; i < mp->nsrc; i++) {
	    utm_mp->next_id = mp->src[i]->source.id;
	    MapPlotAddSource(utm_map, &mp->src[i]->source, False);
	}
	for(i = 0; i < mp->narc; i++) {
	    utm_mp->next_id = mp->arc[i]->arc.id;
	    MapPlotAddArc(utm_map, &mp->arc[i]->arc,  False);
	}
	for(i = 0; i < mp->nell; i++) {
	    utm_mp->next_id = mp->ell[i]->ellipse.id;
	    MapPlotAddEllipse(utm_map, &mp->ell[i]->ellipse, False);
	}
	for(i = 0; i < mp->ndel; i++) {
	    utm_mp->next_id = mp->del[i]->delta.id;
	    MapPlotAddDelta(utm_map, &mp->del[i]->delta, False);
	}
	for(i = 0; i < mp->npoly; i++) {
	    utm_mp->next_id = mp->poly[i]->id;
	    MapPlotAddPolygon(utm_map, mp->poly[i], False);
	}
	for(i = 0; i < mp->nrect; i++) {
	    utm_mp->next_id = mp->rect[i]->id;
	    MapPlotAddRectangle(utm_map, mp->rect[i], False);
	}
	utm_mp->next_id = -1;
}

void
MapPlotUnlinkMap(MapPlotWidget w, MapPlotWidget utm_map)
{
	if( w->map_plot.utm_map == utm_map ) {
	    w->map_plot.utm_map = NULL;
	}
}

static void
utmMode(MapPlotWidget w)
{
    MapPlotPart *mp = &w->map_plot;
    UtmStruct us;

    if(mp->utm_select_letter != mp->utm_letter ||
	mp->utm_select_zone != mp->utm_zone) return;

    if(XtHasCallbacks((Widget)w, XtNutmCallback) != XtCallbackHasNone)
    {
	us.letter = mp->utm_letter;
	us.zone = mp->utm_zone;
	XtCallCallbacks((Widget)w, XtNutmCallback, (XtPointer)&us);
    }
    else {
	MapPlotDisplayUTM(w, mp->utm_letter, mp->utm_zone);
    }
}

Boolean
MapPlotDisplayUTM(MapPlotWidget w, char letter, int zone)
{
    MapPlotPart *mp = &w->map_plot;
    AxesPart *ax = &w->axes;
    double lon_min, lon_max, lat_min, lat_max, x, y, z, margin;
    double a = 6377563;
    double e1;

    if( !getUTMCell(letter, zone, &lon_min, &lon_max, &lat_min, &lat_max) ) {
	return False;
    }

    mp->utm_cell_letter = letter;
    mp->utm_cell_zone = zone;
    mp->utm_center_lon = .5*(lon_min + lon_max);
    mp->utm_center_lat = .5*(lat_min + lat_max);

    mp->e_Squared = 0.00667054;
    mp->e_PrimeSquared = (mp->e_Squared)/(1-mp->e_Squared);
    mp->M1 = 1 - mp->e_Squared/4 - 3*mp->e_Squared*mp->e_Squared/64
                - 5*mp->e_Squared*mp->e_Squared*mp->e_Squared/256;
    mp->M2 = 3*mp->e_Squared/8 + 3*mp->e_Squared*mp->e_Squared/32
                + 45*mp->e_Squared*mp->e_Squared*mp->e_Squared/1024;
    mp->M3 = 15*mp->e_Squared*mp->e_Squared/256 +
                    45*mp->e_Squared*mp->e_Squared*mp->e_Squared/1024;
    mp->M4 = 35*mp->e_Squared*mp->e_Squared*mp->e_Squared/3072;
    mp->e_PrimeSquared = (mp->e_Squared)/(1-mp->e_Squared);
    mp->mu1 = a*(1 - mp->e_Squared/4 - 3*mp->e_Squared*mp->e_Squared/64
		- 5*mp->e_Squared*mp->e_Squared*mp->e_Squared/256);
    e1 = (1 - sqrt(1 - mp->e_Squared))/(1+sqrt(1 - mp->e_Squared));
    mp->p1 = 3*e1/2-27*e1*e1*e1/32;
    mp->p2 = 21*e1*e1/16-55*e1*e1*e1*e1/32;
    mp->p3 = 151*e1*e1*e1/96;

    ax->zoom = 0;
    margin = (mp->utm_cell_zone == 1 || mp->utm_cell_zone == 60) ? 0. : 1.;

    // latitude lines are parallel. longitude lines are not parallel, so
    // detemine x1 and x2 where the longitudinal distance is the greatest.

    if(mp->utm_cell_letter - 'N' >= 0) { // Northern Hemisphere
	llToUtm(mp, lon_min-margin, lat_min-margin, &ax->x1[0], &ax->y1[0], &z);
	llToUtm(mp, lon_max+margin, lat_min+margin, &ax->x2[0], &y, &z);
	llToUtm(mp, lon_min-margin, lat_max+margin, &x, &ax->y2[0], &z);
    }
    else {
	llToUtm(mp, lon_min-margin, lat_max+margin, &ax->x1[0], &ax->y2[0], &z);
	llToUtm(mp, lon_max+margin, lat_max+margin, &ax->x2[0], &y, &z);
	llToUtm(mp, lon_min-margin, lat_min-margin, &x, &ax->y1[0], &z);
    }
    mp->projection = MAP_UTM_NEAR;
    ax->a.ew = 0;
    ax->a.ns = 0;

    _AxesRedraw((AxesWidget)w);
    _MapPlotRedraw(w);
    RedisplayThemes(w);
    RedisplayPixmap(w, mp->pixmap, ax->pixmap);
    RedisplayOverlays(w, ax->pixmap);
    _AxesRedisplayAxes((AxesWidget)w);
    RedisplayPixmap(w, ax->pixmap, XtWindow(w));
    _AxesRedisplayXor((AxesWidget)w);

    return True;
}

static void
utmDelta(MapPlotWidget w, double lon, double lat, int cursor_x, int cursor_y,
		double *del)
{
    MapPlotPart *mp = &w->map_plot;
    AxesPart *ax = &w->axes;
    double utm_x, utm_y, x, y, z;

    llToUtm(mp, lon, lat, &utm_x, &utm_y, &z);
    if(z <= 0.) {
	*del = -1.;
	return;
    }
    x = scale_x(&ax->d, cursor_x);
    y = scale_y(&ax->d, cursor_y);

    *del = sqrt((x-utm_x)*(x-utm_x) + (y-utm_y)*(y-utm_y));
}

static void
utmAz(MapPlotWidget w, double lon, double lat, int cursor_x, int cursor_y,
		double *az)
{
    MapPlotPart *mp = &w->map_plot;
    AxesPart *ax = &w->axes;
    double utm_x, utm_y, x, y, z;

    llToUtm(mp, lon, lat, &utm_x, &utm_y, &z);
    if(z <= 0.) {
	*az = -1.;
	return;
    }
    x = scale_x(&ax->d, cursor_x);
    y = scale_y(&ax->d, cursor_y);

    *az = 90. - atan2((y-utm_y), (x-utm_x))/rad;
}

void
MapPlotGetLimits(MapPlotWidget w, double *lonmin, double *lonmax,
			double *latmin, double *latmax)
{
    MapPlotPart *mp = &w->map_plot;

    *lonmin = mp->lon_min;
    *lonmax = mp->lon_max;
    *latmin = mp->lat_min;
    *latmax = mp->lat_max;
}

Boolean
MapPlotSetLimits(MapPlotWidget w, double lonmin, double lonmax,
			double latmin, double latmax)
{
    MapPlotPart *mp = &w->map_plot;
    AxesPart *ax = &w->axes;
    double x1, y1, z1, x2, y2, z2, lon, lat;

    if(mp->projection != MAP_LINEAR_CYLINDRICAL && mp->projection != MAP_UTM)
    {
	project(w, lonmin, latmin, &x1, &y1, &z1);
	project(w, lonmax, latmax, &x2, &y2, &z2);
	if(z1 < 0 || z2 < 0)
	{
	    // Rotate and try again
	    lon = .5*(lonmin + lonmax);
	    lat = .5*(latmin + latmax);
	    MapRotate(w, lon, lat);
	    project(w, lonmin, latmin, &x1, &y1, &z1);
	    project(w, lonmax, latmax, &x2, &y2, &z2);
	}
    }
    else
    {
	x1 = lonmin;
	while(x1 < ax->x1[0]) x1 += 360;
	while(x1 > ax->x2[0]) x1 -= 360;
	x2 = lonmax;
	while(x2 < ax->x1[0]) x2 += 360;
	while(x2 > ax->x2[0]) x2 -= 360;
	y1 = latmin;
	y2 = latmax;
	z1 = z2 = 1;
    }
    if(z1 > 0 && z2 > 0)
    {
	if(x1 > x2)
	{
	    z1 = x1;
	    x1 = x2;
	    x2 = z1;
	}
	if(y1 > y2)
	{
	    z1 = y1;
	    y1 = y2;
	    y2 = z1;
	}
	ax->zoom = 1;
	ax->x1[1] = x1;
	ax->x2[1] = x2;
	ax->y1[1] = y1;
	ax->y2[1] = y2;

	_AxesRedraw((AxesWidget)w);
	_MapPlotRedraw(w);
	RedisplayThemes(w);

	RedisplayPixmap(w, mp->pixmap, ax->pixmap);
	RedisplayOverlays(w, ax->pixmap);
	_AxesRedisplayAxes((AxesWidget)w);
	RedisplayPixmap(w, ax->pixmap, XtWindow(w));
	_AxesRedisplayXor((AxesWidget)w);
	return True;
    }
    return False;
}
