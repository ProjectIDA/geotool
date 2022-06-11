#ifndef	_CPLOTP_H_
#define	_CPLOTP_H_

#include <Xm/Xm.h>
#include <vector>

extern "C" {
#include "libtime.h"
#include "libgplot.h"
}
#include "widget/AxesP.h"
#include "gobject++/CssTables.h"

#include "CPlot.h"
#include "CPlotClass.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"
#include "Waveform.h"

/// @cond

#define MAX_DECI	3

typedef struct deci_array
{
	int		span;
	GTimeSeries	*ts;
} DeciArray;

class ArrivalEntry : public Gobject
{
    public:
	CssArrivalClass	*a;
	int		npoints;
	XPoint		*points;
	int		width, height, ascent;
	Pixel		fg;
	bool		selected;
	bool		associated;
	bool		on;
	bool		retime;
	bool		modified;
	ArrivalEntry(CssArrivalClass *arrival) {
	    a = arrival;
	    npoints = 0;
	    points = (XPoint *)NULL;
	    width = height = ascent = 0;
	    fg = (Pixel)NULL;
	    selected = false;
	    associated = false;
	    on = True;
	    retime = false;
	    modified = false;
	}
	~ArrivalEntry() {
	    Free(points);
	}
};

typedef struct
{
	ArrivalEntry	*a_entry;

	Boolean		done;
	double		x, y;
	int		y1, y2;
} ArrivalLabel;

typedef struct
{
	char	phase[9];
	Pixel	fg;
	double	time;
	double	x, y;
	int	width, height, ascent;
	int	y1, y2;
	int	npoints;
	XPoint	*points;
	Boolean	done;
} CPlotPredArr;

#define CPlot_Pred_Arr_NULL \
{ \
	"-",	/* phase[9] */ \
	1,	/* fg */ \
	-1.0,	/*time */ \
	-1.0,	/* x */ \
	-1.0,	/* y */ \
	0,	/* width */ \
	0,	/* height */ \
	0,	/* ascent */ \
	0,	/* y1 */ \
	0,	/* y2 */ \
	0,	/* npoints */ \
	NULL,	/* *points */ \
	False	/* done */ \
}

typedef struct data_entry
{
	struct data_entry *comps[MAX_COMPONENTS];
	int		num_deci;
	DeciArray	p[MAX_DECI];
	double		xscale, yscale, length, mean;
	double		ymin, ymax;
	double		visible_ymin, visible_ymax;
	double		display_time;
	Boolean		on;
	int		type;
	double		drag_yscale;
	double		tbeg, tend;
	int		scale_width;
	int		scale_height;
	Boolean		modified;
	Boolean		calib_applied;
	RSegArray	r;
	Waveform	*w;
	GTimeSeries	*ts;
	GTimeSeries	*use_ts;
	GDataPoint	**dp;
	GDataWindow	*dw;
	int		range;
	GDataPoint	*beg, *end;
	int		n_arrlab;
	ArrivalLabel	*arrlab;
	int		npred_arr;
	CPlotPredArr	*pred_arr;
	Boolean		need_find;
	Boolean		need_pred_find;
	Boolean		matched;
	CssOriginClass	*origin;
	double		depth;
	char		tag[100];
	int		tag_x, tag_y;
	int		tag_width;
	int		tag_height;
	int		lab_x, lab_y;
	int		amp_width;
	int		n_measure_segs;
	XSegment	measure_segs[6];
	double		left_side;
			/* the units of amp_ts, bottom_side and zp_ts are the
			 * same as the GTimeSeries (either counts or
			 * Nnms (counts*calib).
			 */
	double		bottom_side, zp_ts;
	double		period, amp_cnts, amp_ts, amp_nms;

	int		n_tag_points;
	XPoint		*tag_points;

	GTimeSeries	*last_ts;
	SegArray	sel;
	SegArray	unsel;
	void		*mag_entry;
	int		component;
} DataEntry;

#define	DATA_ENTRY_INIT \
{ \
	{(DataEntry *)NULL,(DataEntry *)NULL,(DataEntry *)NULL}, /* comps */ \
	0,				/* num_deci */ \
	{ \
		{0, (GTimeSeries *)NULL}, \
		{0, (GTimeSeries *)NULL}, \
		{0, (GTimeSeries *)NULL}, \
	},				/* p */ \
	0., 0., 0., 0.,			/* xscale, yscale, length, mean */ \
	0., 0.,				/* ymin, ymax */ \
	0., 0.,				/* visible_ymin, visible_ymax */ \
	0.,				/* display_time */ \
	False, 				/* on */ \
	CPLOT_DATA,			/* type */ \
	1.,				/* drag_yscale */ \
	0.,				/* tbeg */ \
	0.,				/* tend */ \
	0, 0,				/* scale_width, scale_height */ \
	True,				/* modified */ \
	True,				/* calib_applied */ \
	{(RSeg *)NULL, 0, 0, 0, 0, 0., 0., 0., 0.},	/* r */ \
	(Waveform *)NULL,		/* w */ \
	(GTimeSeries *)NULL,		/* ts */ \
	(GTimeSeries *)NULL,		/* use_ts */ \
	(GDataPoint **)NULL,		/* dp */ \
	(GDataWindow *)NULL,		/* dw */ \
	5,				/* range */ \
	(GDataPoint *)NULL,		/* beg */ \
	(GDataPoint *)NULL,		/* end */ \
	0,				/* n_arrlab */ \
	(ArrivalLabel *)NULL,		/* arrlab */ \
	0,				/* npred_arr */ \
	(CPlotPredArr *)NULL,		/* pred_arr */ \
	False, False,			/* need_find, need_pred_find */ \
	False, 				/* matched */ \
	NULL,				/* origin */ \
	0.,				/* depth */ \
	"",				/* tag */ \
	0, 0,				/* tag_x, tag_y */ \
	0, 0, 				/* tag_width, tag_height */ \
	0, 0,				/* lab_x, lab_y */ \
	0,				/* amp_width */ \
	0,				/* n_measure_segs */ \
	{{0,0,0,0},{0,0,0,0}, \
	 {0,0,0,0},{0,0,0,0}, \
	 {0,0,0,0},{0,0,0,0}},		/* measure_segs[6] */ \
	0., 0., 0.,		/* left_side, bottom_side, zp_ts */ \
	0., 0., 0., 0.,		/* period, amp_cnts, amp_ts, amp_nms */ \
	0, (XPoint *)NULL,	/* n_tag_points, tag_points */ \
	(GTimeSeries *)NULL,	/* last_ts */ \
	{(DSegment *)NULL, 0, 0, 0, 0},	/* sel */ \
	{(DSegment *)NULL, 0, 0, 0, 0},	/* unsel */ \
	(void *)NULL, 			/* mag_entry */ \
	0,				/* component code */ \
}

class DataCurve
{
    public:
    bool	on;
    SegArray	s;
    int		type;
    double	*x;
    float	*y, *x_orig, *y_orig, *ray_p;
    int		npts;
    double	xmin, xmax, ymin, ymax;
    string	mouse_label;
    string	lab;
    int		lab_x, lab_y;
    float	lab_angle;
    Pixel	fg;

    DataCurve() {
	on = false;
	s.segs=(DSegment *)NULL;
	s.size_segs=0; s.nsegs=0; s.start=0; s.nshow=0;
	type = CPLOT_DATA;
	x = (double *)NULL;
	y = x_orig = y_orig = ray_p = (float *)NULL;
	npts = 0;
	xmin = xmax = ymin = ymax = 0.;
	lab_x = lab_y = 0;
	lab_angle = 0.;
	fg = 0;
    }
    ~DataCurve() {
	Free(x);
	Free(y);
	Free(x_orig);
	Free(y_orig);
	Free(ray_p);
	Free(s.segs);
    }
};

typedef struct
{
	Boolean moved, scaled;
	int cutx1, cutx2, npts;
	GDataPoint *beg, *end;
	double xbeg, drag_yscale, y_diff;
} MoveStruct;

class Vectors
{
    public:
    gvector<ArrivalEntry *>	arrival_entries;
    cvector<CssArrivalClass>		arrivals;
    gvector<CPlotArrivalType *>	arrival_types;
    cvector<CssOriginClass>		origins;
    cvector<CssOrigerrClass>		origerrs;
    cvector<CssAssocClass>		assocs;
    cvector<CssStassocClass>		stassocs;
    cvector<CssHydroFeaturesClass>	hydro_features;
    cvector<CssInfraFeaturesClass>	infra_features;
    cvector<CssStamagClass>		stamags;
    cvector<CssNetmagClass>		netmags;
    cvector<CssAmplitudeClass>	amplitudes;
    cvector<CssAmpdescriptClass> 	ampdescripts;
    cvector<CssParrivalClass>	parrivals;
    cvector<CssFilterClass>		filters;
    cvector<CssWftagClass>		wftags;
    gvector<CssTableClass *>		selected_tables;
    Vectors() {}
    ~Vectors() {
	arrival_entries.clear();
	arrivals.clear();
	arrival_types.clear();
	origins.clear();
	origerrs.clear();
	assocs.clear();
	stassocs.clear();
	hydro_features.clear();
	infra_features.clear();
	stamags.clear();
	netmags.clear();
	amplitudes.clear();
	ampdescripts.clear();
	parrivals.clear();
	filters.clear();
	wftags.clear();
	selected_tables.clear();
    }
};

typedef struct
{
	CPlotClass	*cplot_class;
	int		num_entries;
	DataEntry	**entry;
	int		size_entry;
	int		num_curves;
	DataCurve	**curve;
	int		size_curve;
	Gobject		*owner_object;
	Vectors		*v;
	DataEntry	*ref;
	DataEntry	*measure_entry;
	DataEntry	*label_entry;
	DataEntry	*info_entry;
	DataEntry	*select_entry;
	DataEntry	*move_entry;
	MoveStruct	mov;

	XFontStruct 	*arrival_font;
	XFontStruct 	*associated_arrival_font;
	XFontStruct 	*tag_font;
	XFontStruct 	*amp_font;

	long		working_orid;
	int		drag_scale_initial_y;
	int		data_movement;
	Boolean		scale_independently;
	Boolean		first_position;
	double		uniform_scale;
	double		tmin, tmax;
	double		dataSpacing;
	int		numPositions;
	int		dataHeight;
	int		dataSeparation;
	int		resize_measure_box;
	int		arrival_i, arrival_j;
	int		info_arrival_i, info_arrival_j;
	int		arrival_rec_x;
	int		arrival_rec_y;
	int		arrival_rec_width;
	int		arrival_rec_height;
	int		point_rec_x;
	int		point_rec_y;
	int		point_rec_width;
	int		point_rec_height;
	int		point_x;
	int		point_y;
	int		add_arrival_x;
	int		add_arrival_y;
	int		motion_cursor_x;
	int		motion_cursor_y;
	Boolean		arrival_moving;
	int		max_tag_width;
	int		display_components;
	int		display_arrivals;
	Boolean		display_assoc_only;
	int		find_arrivals;
	Boolean		allow_partial_select;
	Boolean		display_predicted_arrivals;
	Boolean		display_amplitude_scale;
	Boolean		display_tags;
	Boolean		scroll_data;
	Boolean		adjust_after_move;
	Boolean		scaling;
	Boolean		selecting;
	Boolean		deselecting;
	Boolean		adding_arrival;
	Boolean		align_arr_selected;
	Boolean		amp_interpolation;
	Boolean		three_half_cycles;
	Boolean		limit_select;
	Boolean		measure_box;
	Boolean		single_arr_select;
	Boolean		last_arr_select;
	Boolean		key_modifier;
	Boolean		need_sort;
	Boolean		redraw_selected_data;
	Boolean		join_timeSeries;
	Boolean		*tmp_selected;
	Boolean		curves_only;
	Boolean		data_shift;
	Boolean		highlight_points;
	Boolean		adjust_time_limits;
	Boolean		display_add_arrival;
	double		data_min;
	double		data_max;
	double		component_time_limit;
	double		join_time_limit;
	double		overlap_limit;
	double		begSelect;
	double		info_delay;
	double		curve_left_margin;
	double		curve_right_margin;
	double		curve_bottom_margin;
	double		curve_top_margin;
	char		last_key_str[10];
	char		arrival_name[50];

	XButtonEvent	add_arrival_event;

	vector<ArrivalKey> *arrival_keys;
	Boolean		retime_arrival;

	int		draw_label_call;
	int		y0Select;

	Widget		info_popup;

	XtAppContext	app_context;

	XtCallbackList	single_select_data_callbacks;
	XtCallbackList	select_data_callbacks;
	XtCallbackList	position_callbacks;
	XtCallbackList	position_drag_callbacks;
	XtCallbackList	select_arrival_callbacks;
	XtCallbackList	measure_callbacks;
	XtCallbackList	amp_convert_callbacks;
	XtCallbackList	retime_callbacks;
	XtCallbackList	retime_drag_callbacks;
	XtCallbackList	waveform_menu_callbacks;
	XtCallbackList	arrival_menu_callbacks;
	XtCallbackList	waveform_info_callbacks;
	XtCallbackList	arrival_info_callbacks;
	XtCallbackList	modify_waveform_callbacks;
	XtCallbackList	menu_callbacks;
	XtCallbackList	add_arrival_callbacks;
	XtCallbackList	add_arrival_menu_callbacks;

	GC		labelGC;
	GC		invertGC;
	GC		arrivalGC;
	GC		dataGC;

} CPlotPart;

typedef struct
{
	int		nextid;
} CPlotClassPart;

typedef struct _CPlotRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
	CPlotPart	c_plot;
} CPlotRec;

typedef struct _CPlotClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
	CPlotClassPart		c_plot_class;
} CPlotClassRec;

extern CPlotClassRec cPlotClassRec;


/* ******** from CPlot.c for use in libwgets only ********/

void _CPlotRedisplay(CPlotWidget w);
void _CPlotRedisplayCurve(CPlotWidget w, DataCurve *curve);
void _CPlotRedisplayEntry(CPlotWidget w, DataEntry *entry);
void _CPlotRedraw(CPlotWidget w);
void _CPlotRedrawEntry(CPlotWidget w, DataEntry *entry);
void _CPlotRedrawCurve(CPlotWidget w, DataCurve *curve);
void _CPlotDrawPredArrivals(CPlotWidget w, DataEntry *entry);
void _CPlotMove(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params, int m, double manual_scaled_x0,
		double manual_scaled_y0);
void _CPlotScale(CPlotWidget w, XEvent *event, String *params,
		Cardinal *num_params);
void _CPlotHorizontalScroll(CPlotWidget w, double shift, Boolean jump);
void _CPlotVerticalScroll(CPlotWidget w, double shift, Boolean jump);
void _CPlotPredArrivals(CPlotWidget widget, Waveform *w,
		CPlotPredArr *pred_arr, int npred_arr);
void _CPlotGetXLimits(CPlotWidget w, double *xmin,double *xmax);
Boolean _CPlotVisible(CPlotWidget w, DataEntry *entry);
void _CPlotComponentsOnOff(CPlotWidget w, Boolean place);

/// @endcond

#endif	/* _CPLOTP_H_ */
