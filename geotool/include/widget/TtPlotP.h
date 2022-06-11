#ifndef	_TTPLOTP_H_
#define	_TTPLOTP_H_

#include <Xm/Xm.h>

#include "widget/TtPlot.h"
#include "widget/CPlotP.h"
#include "TravelTime.h"

/// @cond

typedef struct
{
	const char 	*phase;
	bool		made;
	bool		selected;
} PhaseList;

typedef struct
{
	Boolean		on;
	double		scaled_x0;
	double		scaled_y0;
	double		drag_yscale;
	double		begSelect, endSelect;
	int		tag_x, tag_y;
	int		lab_x, lab_y;
	int		n_measure_segs;
	XSegment	measure_segs[6];
	double		left_side, bottom_side, zp_ts;
	double		period, amp_cnts, amp_ts, amp_nms;
	DataEntry	*entry;
} SavedEntry;

typedef struct
{
	double 		dataSpacing;
	int		dataHeight;
	int		dataSeparation;
	Boolean		y_label_int;
	int		zoom;
	int		zoom_max;
	double		tmin, tmax;
	double		x1[MAX_ZOOM], x2[MAX_ZOOM], y1[MAX_ZOOM], y2[MAX_ZOOM];
	int		num_saved;
	SavedEntry	*saved;
} CPlotState;

typedef struct
{
	char		*phase;
	int		width;
	int		height;
	int		ascent;
	int		npoints;
	XPoint		*points;
} PredictedLabel;


typedef struct
{
	TravelTime	*travel_time;
	char		*geo_table_dir;
	char		**selected_phases;
	PhaseList	*iaspei_phases;
	PhaseList	*jb_phases;
	PhaseList	*regional_phases;
	PhaseList	*surface_phases;
	double		source_depth;
	double		iaspei_depth;
	Boolean		display_tt_curves;
	Boolean		display_tt_labels;
	Boolean		plot_ray;
	Boolean		align_pred_selected;
	Boolean		reduced_time;
	double		reduction_velocity;
	Pixel		iaspei_color;
	Pixel		iaspei_curve_color;
	Pixel		jb_color;
	Pixel		regional_color;
	Widget		ray_widget;
	int		which;
	int		ray_x, ray_y;
	double		ray_par;
	double		last_ray_par;
	double		stop_Pdiff;
	double		lg_vel;
	double		lq_vel;
	double		lr_vel;
	double		rg_vel;
	double		t_vel;
	double		infra_vel;
	double		infra_tt;
	float		last_depth;
	CrustModel	crust;
	char		*jb_table;
	char		*iaspei_table;
	int		distance_units;

	Boolean		restore_last_state;
	Boolean		predicted_first_warn;
	Boolean		predictedPhase_first_warn;
	Boolean		jb_first_warn;
	Boolean		use_celerity;
	CPlotState	state;
	CPlotState	last_state;

	int		num_labels;
	PredictedLabel	*pred_labels;

	int		prev_n;
	InputPredArr	prev_pred_arr[200];
} TtPlotPart;

typedef struct
{
	char		*jb_table;
	char		*iaspei_table;
} TtPlotClassPart;

typedef struct _TtPlotRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
	CPlotPart	c_plot;
	TtPlotPart	tt_plot;
} TtPlotRec;

typedef struct _TtPlotClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
	CPlotClassPart		c_plot_class;
	TtPlotClassPart		tt_plot_class;
} TtPlotClassRec;

extern TtPlotClassRec ttPlotClassRec;

/// @endcond

#endif	/* _TTPLOTP_H_ */
