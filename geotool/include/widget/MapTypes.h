#ifndef	MAP_TYPES_H
#define	MAP_TYPES_H

extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
#include "shapefil.h"
}

#define	MAP_LINEAR_CYLINDRICAL		0
#define	MAP_CYLINDRICAL_EQUAL_AREA	1
#define	MAP_MERCATOR			2
#define	MAP_ORTHOGRAPHIC		3
#define	MAP_AZIMUTHAL_EQUIDISTANT	4
#define	MAP_AZIMUTHAL_EQUAL_AREA	5
#define	MAP_POLAR			6
#define MAP_UTM				7
#define MAP_UTM_NEAR			8

#define MAP_INPUT		0
#define MAP_MEASURE		1
#define	MAP_PATH		2
#define	MAP_SRC_ELLIPSE		3
#define MAP_CURSOR_MEASURE	4

#define CONTOURS_ONLY		0
#define COLOR_ONLY		1
#define CONTOURS_AND_COLOR	2

#define SHPT_VARIABLE		233
#define SHPT_POLAR		234


/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	bool		polar_coords;
	double		lon;	/* or radius */
	double		lat;	/* or azimuth (degrees) */
	SymbolInfo	sym;
} MapPlotSymbol;

#define MAP_PLOT_SYMBOL_INIT \
{ \
	0,			/* id */ \
	false,			/* polar_coords */ \
	0.,			/* lon */ \
	0.,			/* lat */ \
	SYMBOL_INFO_INIT,	/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	bool		polar_coords;
	int		npts;
	int		*size;
	double		*lat;
	double		*lon;
	SymbolInfo	sym;
} MapPlotSymbolGroup;

#define MAP_PLOT_SYMBOL_GROUP_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	False,			/* polar_coords */ \
	0,			/* npts */ \
	(int *)NULL,		/* size */ \
	(double *)NULL,		/* lat */ \
	(double *)NULL,		/* lon */ \
	SYMBOL_INFO_INIT,	/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	int		npts;
	char		*label;
	bool		polar_coords;
	double		*lat;
	double		*lon;
	LineInfo	line;
} MapPlotLine;

#define MAP_PLOT_LINE_INIT \
{ \
	0,			/* id */ \
	0,			/* npts */ \
	(char *)NULL,		/* label */ \
	false,			/* polar_coords */ \
	(double *)NULL,		/* lat */ \
	(double *)NULL,		/* lon */ \
	LINE_INFO_INIT,		/* line */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	double		lat, lon;
	int		tag_loc;
	SymbolInfo	sym;
} MapPlotStation;

#define MAP_PLOT_STATION_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	0.,			/* lat */ \
	0.,			/* lon */ \
	0,			/* tag_loc */ \
	SYMBOL_INFO_INIT,	/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id, orid;
	char		*label;
	double		lat, lon, depth, time, smajax, sminax, strike;
	int		tag_loc;
	SymbolInfo	sym;
} MapPlotSource;

#define MAP_PLOT_SOURCE_INIT \
{ \
	0,			/* id */ \
	0,			/* orid */ \
	(char *)NULL,		/* label */ \
	0.,			/* lat */ \
	0.,			/* lon */ \
	0.,			/* depth */ \
	0.,			/* time */ \
	0.,			/* smajax */ \
	0.,			/* sminax */ \
	0.,			/* strike */ \
	0,			/* tag_loc */ \
	SYMBOL_INFO_INIT,	/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	double		lat, lon, del, az;
	LineInfo	line;
} MapPlotArc;

#define MAP_PLOT_ARC_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	0.,			/* lat */ \
	0.,			/* lon */ \
	0.,			/* del */ \
	0.,			/* az */ \
	LINE_INFO_INIT,		/* line */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	double		lat, lon;
	double		sminax, smajax, strike;
	LineInfo	line;
} MapPlotEllipse;

#define MAP_PLOT_ELLIPSE_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	0.,			/* lat */ \
	0.,			/* lon */ \
	0.,			/* sminax */ \
	0.,			/* smajax */ \
	0.,			/* strike */ \
	LINE_INFO_INIT,		/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	double		lat, lon, del;
	LineInfo	line;
} MapPlotDelta;

#define MAP_PLOT_DELTA_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	0.,			/* lat */ \
	0.,			/* lon */ \
	0.,			/* del */ \
	LINE_INFO_INIT,		/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	int		npts;
	double		*lat;
	double		*lon;
	SymbolInfo	sym;
} MapPlotPolygon;

#define MAP_PLOT_POLYGON_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	0,			/* npts */ \
	(double *)NULL,		/* lat */ \
	(double *)NULL,		/* lon */ \
	SYMBOL_INFO_INIT,	/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef struct
{
	int		id;
	char		*label;
	double		lat;
	double		lon;
	double		width;
	double		height;
	SymbolInfo	sym;
} MapPlotRectangle;

#define MAP_PLOT_RECTANGLE_INIT \
{ \
	0,			/* id */ \
	(char *)NULL,		/* label */ \
	0.,			/* lat */ \
	0.,			/* lon */ \
	0.,			/* width */ \
	0.,			/* height */ \
	SYMBOL_INFO_INIT,	/* sym */ \
}

/**
 *  @ingroup libwgets
 */
typedef union
{
	int			id;
	MapPlotSource		src;
	MapPlotStation		sta;
	MapPlotArc		arc;
	MapPlotEllipse		ell;
	MapPlotDelta		del;
	MapPlotSymbol		sym;
	MapPlotLine		lin;
	MapPlotPolygon		poly;
	MapPlotRectangle	rect;
} MapObject;

/**
 *  @ingroup libwgets
 */
typedef struct
{
	double		lat;
	double		lon;
	double		radius;
	void		*pt;
	bool		selected;
	bool		display;
} MapCircle;

/**
 *  @ingroup libwgets
 */
typedef struct
{
	char		*label;
	int		id;
	int		type;
	double		lat;
	double		lon;
	double		delta;
	double		az;
} MapMeasure;

/**
 *  @ingroup libwgets
 */
class ColorScale
{
    public:
	ColorScale(void)
	{
	    num_colors = 0;
	    pixels = NULL;
	    lines = NULL;
	    num_labels = 0;
	    labels = NULL;
	    label_values = NULL;
	}
	~ColorScale(void)
	{
//	    if(pixels) free(pixels);
//	    if(lines) free(lines);
//fix***	    for(int i = 0; i < num_labels; i++) if(labels[i]) free(labels[i]);
//fix****	    if(labels) free(labels);
//fix****	    if(label_values) free(label_values);
	}
	int		num_colors;
	Pixel		*pixels;	/* num_colors pixels */
	double		*lines;		/* num_colors+1 to color divisions */
	int		num_labels;
	char		**labels;
	double		*label_values;
};

/**
 *  @ingroup libwgets
 */
class MapPlotTheme
{
    public:
	MapPlotTheme(void)
	{
	    shape_type = 0;
	    type_name = NULL;
	    nshapes = 0;
	    shapes = NULL;
	    lon_min = 0.; lon_max = 0.; lat_min = 0.; lat_max = 0.;
	    on_delta = 0.;
	    off_delta = 360.;
	    shape_fg = NULL;
	    symbol_type = -1;
	    symbol_size = -1;
	    bndry = False;
	    fill = False;
	    bndry_fg = (Pixel)0;
	    color_scale.num_colors = 0;
	    color_scale.pixels = NULL;
	    color_scale.lines = NULL;
	    color_scale.num_labels = 0;
	    color_scale.labels = NULL;
	    color_scale.label_values = NULL;
	    polar_coords = False;
	    center_lat = 0.;
	    center_lon = 0.;
	}
	~MapPlotTheme(void) {}

	int		shape_type;
	char		*type_name;
        int             nshapes;
        SHPObject       **shapes;
	double		lon_min, lon_max, lat_min, lat_max;
	double		on_delta, off_delta;
	Pixel		*shape_fg;	/* fill fg for polygons */
	int		symbol_type;	/* for point shapes */
	int		symbol_size;	/* for point shapes */
	bool		bndry;
	bool		fill;
	Pixel		bndry_fg;	/* boundary fg for polygons */
	ColorScale	color_scale;
	bool		polar_coords;
	double		center_lat;
	double		center_lon;
};

#endif	/* MAP_TYPES_H */
