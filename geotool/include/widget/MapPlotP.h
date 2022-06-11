#ifndef	_MAPPLOTP_H_
#define	_MAPPLOTP_H_

extern "C" {
#include "libgplot.h"
}
#include "widget/AxesP.h"
#include "MapPlot.h"
 
/// @cond
typedef struct
{
	SegArray 	fill_segs;
	SegArray 	bndy_segs;
	PointArray	p;
	Boolean		fill;
	Boolean		selected;
	Boolean		display;
	int		center_x;
	int		center_y;
	int		npts;
	double		*lon;
	double		*lat;
	double		label_x;
	double		label_y;
	Boolean		display_label;
	char		*label;
} ThemeDisplay;

class MapImage
{
    public:
	MapImage(void)
	{
	    label = NULL;
	    copy = False;
	    m.nx = 0;
	    m.ny = 0;
	    m.x = NULL;
	    m.y = NULL;
	    m.z = NULL;
	    m.z_min = 0.;
	    m.z_max = 0.;
	    m.imin = 0;
	    m.jmin = 0;
	    m.imax = 0;
	    m.jmax = 0;
	    m.x1 = 0.;
	    m.x2 = 0.;
	    m.y1 = 0.;
	    m.y2 = 0.;
	    m.exc = 1.e+30;
	    ci = 0.;
	    auto_ci = True;
	    c_min = 0.;
	    c_max = 0.;
	    s.segs = NULL;
	    s.size_segs = 0;
	    s.nsegs = 0;
	    s.start = 0;
	    s.nshow = 0;
	    t.segs = NULL;
	    t.size_segs = 0;
	    t.nsegs = 0;
	    t.start = 0;
	    t.nshow = 0;
	    l.n_ls = 0;
	    l.size_ls = 0;
	    l.ls = NULL;
	    mode = COLOR_ONLY;
	    nparts = 0;
	    for(int i = 0; i < 4; i++) {
		display_x0[i] = 0;
		display_y0[i] = 0;
		display_width[i] = 0;
		display_height[i] = 0;
	    }
	}
	~MapImage(void) {}

	char		*label;
	Boolean		copy;
	Matrx		m;

	float		ci;
	Boolean		auto_ci;
	float		c_min, c_max;
	SegArray 	s, t;
	LabelArray 	l;

	int		mode;
	int		nparts;
	int		display_x0[4];
	int		display_y0[4];
	int		display_width[4];
	int		display_height[4];
};

#define	THEME_SHAPE	1
#define	THEME_IMAGE	2

#define MAX_POLAR_AZ	18
#define MAX_POLAR_DIST	10

typedef struct
{
	int	x;
	int	y;
	double	value;
	Boolean	border;
} PolarLabel;

class MapTheme
{
    public:
	MapTheme(void)
	{
	    id = 0;
	    theme_type = 0;
	    display = True;
	    delta_on = True;
	    display_labels = False;
	    cursor_info = True;
	    color_bar_on = False;
	    display_grid = True;
	    rmax = 0.;
	    grid.segs = NULL;
	    grid.size_segs = 0;
	    grid.nsegs = 0;
	    grid.start = 0;
	    grid.nshow = 0;
	    numaz = 0;
	    numdist = 0;
	    ndeci = 0;
	    for(int i = 0; i < MAX_POLAR_AZ; i++) {
		az[i].x = 0;
		az[i].y = 0;
		az[i].value = 0.;
		az[i].border = False;
	    }
	    for(int i = 0; i < MAX_POLAR_DIST; i++) {
		dist[i].x = 0;
		dist[i].y = 0;
		dist[i].value = 0.;
		dist[i].border = False;
	    }
	    td = NULL;
	}
	int		id;
	int		theme_type;	/* THEME_SHAPE, THEME_IMAGE */
	Boolean		display;
	Boolean		delta_on;
	Boolean		display_labels;
	Boolean		cursor_info;
	Boolean		color_bar_on;
	Boolean		display_grid;
	double		rmax;
	SegArray	grid;
	int		numaz,numdist;
	int		ndeci;
	PolarLabel	az[MAX_POLAR_AZ];
	PolarLabel	dist[MAX_POLAR_DIST];
	MapPlotTheme	theme;
	ThemeDisplay	*td;
	MapImage	map_image;
};

typedef struct
{
	Pixel		map_water_color;
	int		projection;
	double		lon_min, lon_max, lat_min, lat_max;
	double		mercator_max_lat;
	double		c[3][3];
	double		d[3][3];
	double		theta;
	double		phi;
	double		polar_max_radius;
	double		polar_del_az;
	double		polar_del_dist;

	int		display_circles;
	int		display_paths;
	int		display_source_ellipses;
	Boolean		map_display_grid;
	Boolean		display_station_tags;
	Boolean		display_source_tags;
	Boolean		movable_stations;
	Boolean		movable_sources;
	Boolean		measure;
	Boolean		display_map_cil;
	Boolean		display_map_bdy;
	Pixmap		pixmap;
	Boolean		need_redisplay;
	Boolean		select_sta_src;
	Boolean		select_arc_del;
	Boolean		select_symbol;
	Boolean		moving_circle;
	Boolean		moving_station;
	Boolean		rotate_on_zoom;
	Boolean		apply_anti_alias;
	Boolean		display_polar_selection;
	Boolean		polar_select;
	Boolean		moving_polar_select;
	Boolean		size_polar_select;
	Boolean		select_color_bar;
	int		polar_select_side;
	int		polar_select_i3;
	int		moving_index;
	int		moving_x0;
	int		moving_y0;
	int		motion_which;
	int		motion_imin;
	int		motion_jmin;
	int		motion_x;
	int		motion_y;
	int		bar_vertical_margin;
	Widget		info_popup;

	MapTheme	**theme;
	int		nthemes;
	int		size_themes;

	MapLine		**line;
	int		nlines;
	int		size_line;
	MapSymbolGroup	**symgrp;
	int		nsymgrp;
	int		size_symgrp;
	MapPlotSymbol	*symbol;
	int		nsymbol;
	int		size_symbol;
	MapStation	**sta;
	int		nsta;
	int		size_sta;
	MapSource	**src;
	int		nsrc;
	int		size_src;
	MapArc		**arc;
	int		narc;
	int		size_arc;
	MapEllipse	**ell;
	int		nell;
	int		size_ell;
	MapDelta	**del;
	int		ndel;
	int		size_del;
	MapPlotPolygon	**poly;
	int		npoly;
	int		size_poly;
	MapPlotRectangle **rect;
	int		nrect;
	int		size_rect;

	int		ncircles;
	MapCircle	*circle;
	int		*circle_start;
	int		*num_circle_segs;
	int		primary_circle;

	int		n_measure_arcs;
	MapArc		**measure_arc;
	int		n_measure_dels;
	MapDelta	**measure_del;

	GC		symbolGC;
	GC		lineGC;
	GC		dataGC;
	XFontStruct	*label_font;

	SegArray	border;
	SegArray	grid;
	SegArray	circles;

	GC		contour_labelGC;
	GC		colorGC, barGC;
	int		contour_label_size;
	Pixel		contour_label_fg, data_fg;
	Boolean		mark_max;
	Boolean		auto_interval;
	Boolean		private_cells;
	double		contour_interval;
	double		contour_min, contour_max;
	int		mode;

	int		ndeci, bar_width;
	XImage		*image;
	int		image_width;
	int		image_height;

	int		size_points;
	XPoint		*points;
	int		polar_ndeci;
	int		numaz, numdist;
	PolarLabel	az[MAX_POLAR_AZ];
	PolarLabel	dist[MAX_POLAR_DIST];
	XtAppContext	app_context;

	char		utm_letter;
	char		utm_select_letter;
	char		utm_cell_letter;
	Boolean		selecting_utm;
	int		utm_zone;
	int		utm_select_zone;
	int		utm_cell_zone;
	double		utm_center_lon;
	double 		utm_center_lat;
	double		e_Squared;
	double		e_PrimeSquared;
	double		M1;
	double		M2;
	double		M3;
	double		M4;
	double		mu1;
	double		p1;
	double		p2;
	double		p3;
	double		polar_margin;
	XSegment	utm_segs[4];
	MapPlotWidget	utm_map;
	int		next_id;
	int		polar_select_id;
	double		polar_radius;
	double		polar_del_radius;
	double		polar_azimuth;
	double		polar_del_azimuth;
	MapLine		*polar_line;

	XtCallbackList	select_station_callbacks;
	XtCallbackList	drag_station_callbacks;
	XtCallbackList	select_source_callbacks;
	XtCallbackList	drag_source_callbacks;
	XtCallbackList	map_measure_callbacks;
	XtCallbackList	select_arc_callbacks;
	XtCallbackList	select_circle_callbacks;
	XtCallbackList	cursor_motion_callbacks;
	XtCallbackList	shape_select_callbacks;
	XtCallbackList	symbol_select_callbacks;
	XtCallbackList	symbol_info_callbacks;
	XtCallbackList	utm_callbacks;
	XtCallbackList	polar_select_callbacks;
	XtCallbackList	select_bar_callbacks;
} MapPlotPart;

typedef struct
{
	Boolean		have_rivers;
} MapPlotClassPart;

typedef struct _MapPlotRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
	MapPlotPart	map_plot;
} MapPlotRec;

typedef struct _MapPlotClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
	MapPlotClassPart	map_plot_class;
} MapPlotClassRec;
 
extern MapPlotClassRec mapPlotClassRec;

void _MapPlotRedisplay(MapPlotWidget w);
void _MapPlotRedraw(MapPlotWidget w);
void _MapPlotDrawBar(MapPlotWidget w);

/// @endcond

#endif	/* _MAPPLOTP_H_ */
