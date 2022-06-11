#ifndef	_CONPLOTP_H_
#define	_CONPLOTP_H_

#include <Xm/Xm.h>
extern "C" {
#include "libgplot.h"
}
#include "widget/ConPlot.h"
#include "widget/AxesP.h"

/// @cond

typedef struct
{
	char		*label;
	int		id;
	Matrx		m;
	Boolean		display_data;
	Boolean		display_grid;
	Boolean		*select_val;
	Boolean		*dim_val;
	double		xmax, ymax;
	float		ci;
	Boolean		auto_ci;
	float		c_min, c_max;
	SegArray 	s, t;
	LabelArray 	l;
        Boolean         distinctOutliers;
        Pixel           select_color;
} ConEntry;

#ifndef CON_ENTRY_INIT
#define CON_ENTRY_INIT \
{ \
	(char *)NULL,				/* label */ \
	0,					/* id */ \
	{0, 0, (double *)NULL, (double *)NULL, (float *)NULL, 0., 0., \
	 0, 0, 0, 0, 0., 0., 0., 0., 1.e+30},	/* m */ \
	True,					/* display_data */ \
	False,					/* display_grid */ \
	(Boolean *)NULL,			/* select_val */ \
	(Boolean *)NULL,			/* dim_val */ \
	0., 0.,					/* xmax, ymax   */ \
	0.,					/* ci */ \
	True,					/* auto_ci */ \
	0., 0.,					/* c_min, c_max */ \
	{(DSegment *)NULL, 0, 0, 0, 0},		/* s */ \
	{(DSegment *)NULL, 0, 0, 0, 0},		/* t */ \
	{0, 0, (LabelStruct *)NULL},		/* l */ \
        False, 0,			/* distinctOutliers, select_color */ \
}

#endif

typedef struct
{
	Pixel		data_fg, contour_label_fg;
	GC		dataGC, contour_labelGC;
	GC		colorGC, barGC;
	int		contour_label_size;

	Boolean		mark_max;
	Boolean		auto_interval;
	Boolean		private_cells;
	Boolean		apply_anti_alias;
	double		contour_interval;
	double		contour_min, contour_max;
	char		*contour_colors;
	int		mode;
	float		dim_percent;
	Colormap	cmap;
	int		num_colors;
	int		num_cells;
	XColor		*colors;
	int		num_stipples;
	Pixmap		*stipples;
	int		num_lines;
	int		ndeci, bar_x, bar_width;
	char		*bar_title;
	char		*short_bar_title;
	int		bar_title_x, bar_title_y;
	int		bar_title_w, bar_title_h;
	XImage		*bar_title_image;
	XImage		*long_bar_title_image;
	XImage		*short_bar_title_image;
	double		*lines;
	Pixel		*pixels;
	Pixel		plot_background_pixel;
	unsigned int	*plane_masks;
	XImage		*image;
	int		image_width;
	int		image_height;
	int		image_x0;
	int		image_y0;
	int		image_display_width;
	int		image_display_height;

	int		num_entries;
	int		size_entry;
	ConEntry	**entry;

	int		ctrl_select_reason;
	int		num_bar_labels;
	double		*bar_values;
	char		**bar_labels;
	ConPlotSelectCallbackStruct select_callback;

        XtCallbackList  select_data_callbacks;
        XtCallbackList  select_bar_callbacks;
        XtCallbackList  mouse_over_callbacks;
} ConPlotPart;

typedef struct
{
	int	empty;
} ConPlotClassPart;

typedef struct _ConPlotRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
	ConPlotPart	con_plot;
} ConPlotRec;

typedef struct _ConPlotClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
	ConPlotClassPart	con_plot_class;
} ConPlotClassRec;

extern ConPlotClassRec conPlotClassRec;

/* ******** from ConPlot.c for use in libwgets only ********/

void _ConPlotRedisplayData(ConPlotWidget w);
void _ConPlotRedraw(ConPlotWidget w);
void _ConPlotDrawBar(ConPlotWidget w);

/// @endcond

#endif	/* _CONPLOTP_H_ */
