#ifndef	_AXESP_H_
#define	_AXESP_H_

#include <Xm/Xm.h>
#include <time.h>
#include "widget/Axes.h"
extern "C" {
#include "motif++/Info.h"
}

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define MAX_ZOOM	20
#define MAX_CURSORS	10
#define MAX_RECTS	20

#define DATA_REDISPLAY			0
#define DATA_REDRAW			1
#define DATA_SCROLL_HOR			2
#define DATA_SCROLL_VERT		3
#define DATA_JUMP_HOR			4
#define DATA_JUMP_VERT			5
#define DATA_ZOOM_ZERO			6
#define DATA_ZOOM_VERTICAL_ZERO		7
#define DATA_ZOOM_HORIZONTAL_ZERO	8

#define AXES_ZOOM		0
#define AXES_SELECT_CURSOR	1
#define AXES_MOVE_MAGNIFY	2
#define AXES_STRETCH_MAGNIFY	3

#define ALLOC(new_size, old_size, ptr, type, extra)	\
	if((new_size) > (unsigned int)(old_size) )	\
	{							\
		if(ptr == (type *)0)				\
		{						\
			ptr = (type *)malloc(new_size+extra);\
		}						\
		else						\
		{						\
			ptr = (type *)realloc(ptr, new_size+extra);\
		}						\
		if(ptr == (type *)0)				\
		{						\
			perror("malloc error");			\
			exit(1);				\
		}						\
		else						\
		{						\
			old_size = new_size+extra;		\
		}						\
	}
/**
 *  structure to used to draw the overlay rectangles
 */
/// @cond
typedef struct
{
	double  *ovr_scaled_x1;
	double  *ovr_scaled_x2;
	double  *ovr_scaled_y1;
	double  *ovr_scaled_y2;
	Boolean *display;
	int     nrecs;
	Pixel   pixel;
	int     width;
} OverlayGrp;

typedef struct
{
	AxesClass	*axes_class;
	char		*title, *x_label, *y_label, *err_msg;
	char		cursor_type[20];
	XImage		*y_label_image;

	AxesParm	a;
	XRectangle 	clip_rect, old_clip;

	Pixel		fg, mag_fg, select_fg;
	XFontStruct 	*font;

	double		x1[MAX_ZOOM], x2[MAX_ZOOM], y1[MAX_ZOOM], y2[MAX_ZOOM];
	double		x_min, x_max, y_min, y_max;
	double		x_spacing, y_spacing;
	int		left_margin, right_margin, top_margin, bottom_margin;
	int		zoom_x1, zoom_y1, zoom_x2, zoom_y2;
	int		clipx1, clipy1, clipx2, clipy2;
	int		rt_margin, bm_margin;
	double		mag_scaled_x1, mag_scaled_x2;
	double		mag_scaled_y1, mag_scaled_y2;
	double		zoom_horizontal, zoom_vertical;
        double          ovr_scaled_x1, ovr_scaled_x2;
        double          ovr_scaled_y1, ovr_scaled_y2;
	double		delx_min, dely_min;
	int		last_x_mark, last_y_mark;
	int		x_axis, y_axis;
	int		last_cursor_x, last_cursor_y;
	int		last_width, last_height;
	int		wait_state;
	int		vertical_scroll_position;
	int		title_x, title_y, title_width;
        Boolean         overlay_rect;
	int		n_ovy_grps;
	OverlayGrp	ovy_grp[MAX_RECTS];
	Boolean		undraw;
	Boolean		reverse_y, reverse_x;
	Boolean		unmark;
	Boolean		zooming;
	Boolean		horizontal_scroll;
	Boolean		vertical_scroll;
	Boolean		magnify;
	Boolean		magnify_rect;
	Boolean		tickmarks_inside;
	Boolean		clear_on_scroll;
	Boolean		clear_on_redisplay;
	Boolean		zoom_controls;
	Boolean		magnify_only;
	Boolean		allow_cursor_change;
	Boolean		label_mode;
	Boolean		redisplay_pending;
	Boolean		redisplay;
	Boolean		moving_cursor;
	Boolean		expose;
	Boolean		use_pixmap;
	Boolean		use_screen;
	Boolean		setvalues_redisplay;
	Boolean		display_axes;
	Boolean		uniform_scale;
	Boolean		auto_y_scale;
	Boolean		manual_scroll;
	Boolean		check_expose_region;
	Boolean		scrollbar_as_needed;
	Boolean		draw_y_labels;
	Boolean		double_line_anchor;
	Boolean		double_line_anchor_on;
	Boolean		display_grid;
	Boolean		skip_limits_callback;
	Boolean		save_space;
	Boolean		cursor_label_always;
	Boolean		extra_y_tics;
	Boolean		extra_x_tics;
	Boolean		display_x_label;
	Boolean		display_y_label;
	int		time_scale;
	int		display_axes_labels;
	int		scaling_double_line;
	int		num_cursors;
	AxesCursor	cursor[MAX_CURSORS];
	double		double_line_width;
	int		num_crosshairs;
	int		num_lines;
	int		num_double_lines;
	int		num_phase_lines;
	AxesCursorCallbackStruct all_cursors[MAX_CURSORS];
	AxesCursorCallbackStruct *crosshairs;
	AxesCursorCallbackStruct *lines;
	AxesCursorCallbackStruct *double_lines;
	AxesCursorCallbackStruct *phase_lines;

	int		label_xpos, label_ypos;

	DrawStruct	d;
	Pixmap		pixmap;
	int		depth;
	int		zoom;
	int		zoom_min;
	int		zoom_max;
	int		min_zoom_level;
	int		m;
	int		cursor_mode;
	int		stretch_magnify;

        int		anchor_x, anchor_y;
	int		last_x, last_y;
	int		no_motion, hor_only;
	int		num_time_marks;
	double		*time_marks;

	int		hor_scroll_value;
	int		vert_scroll_value;
	Widget		hor_scroll;
	Widget		outside_hor_scroll;
	Widget		vert_scroll;
	Widget		mag_to;
	Widget		mag_from;
	Widget		cursor_info;
	Widget		cursor_info2;
	Widget		width_form;
	Widget		width_text;
	XtCallbackList	zoom_callbacks;
	XtCallbackList	crosshair_callbacks;
	XtCallbackList	crosshair_drag_callbacks;
	XtCallbackList	line_callbacks;
	XtCallbackList	line_drag_callbacks;
	XtCallbackList	double_line_callbacks;
	XtCallbackList	double_line_drag_callbacks;
	XtCallbackList	double_line_scale_callbacks;
	XtCallbackList	phase_line_callbacks;
	XtCallbackList	phase_line_drag_callbacks;
	XtCallbackList	limits_callbacks;
	XtCallbackList	hor_scroll_callbacks;
	XtCallbackList	magnify_callbacks;
	XtCallbackList	warning_callbacks;
	XtCallbackList	keypress_callbacks;
	XtCallbackList	resize_callbacks;

	AxesRedrawFunction	redraw_data_func;
	AxesResizeFunction	resize_func;
	AxesCursorFunction	cursor_func;
	AxesHardCopyFunction	hard_copy_func;
	AxesSelectDataFunction	select_data_func;
	AxesTransformFunction	transform;

	GC		axesGC;
	GC		xorGC;
	GC		invertGC;
	GC		invert_clipGC;
	GC		mag_invertGC;
	GC		eraseGC;
	GC		selectGC;
} AxesPart;

typedef struct
{
	int		empty;
} AxesClassPart;

typedef struct _AxesRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
} AxesRec;

typedef struct _AxesClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
} AxesClassRec;

extern AxesClassRec axesClassRec;


typedef struct
{
	int x;
	int y;
	int width;
	int height;
} Rectangle;

/* ******** from Axes.c for use in libwgets only ********/

Rectangle _AxesLayout(AxesWidget w);
void _AxesRedisplayXor(AxesWidget w);
Boolean _AxesRedisplayAxes(AxesWidget w);
void _AxesRedisplayCursors(AxesWidget w);
void _AxesRedisplayMarks(AxesWidget w);
void _AxesRedisplayMagRect(AxesWidget w);
void _AxesRedraw(AxesWidget w);
void _AxesMagnify(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesUnzoomPercentage(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesUnzoomHorizontal(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesUnzoomVertical(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesZoom(AxesWidget w, XEvent *event, const char **params,
			Cardinal *num_params);
void _AxesZoomBack(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesZoomHorizontal(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesZoomVertical(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesZoomPercentage(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesKeyCommand(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesBtnDown(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesAddCrosshair(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesAddDoubleLine(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesAddLine(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesDeleteCursor(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
void _AxesMove(AxesWidget w, XEvent *event, const char **params,
			Cardinal *num_params);
void _AxesScale(AxesWidget w, XEvent *event, String *params,
			Cardinal *num_params);
Boolean _AxesMotion(AxesWidget w, XEvent *event, String *params,
                        Cardinal *num_params);
int _AxesWhichCursor(AxesWidget w, int cursor_x, int cursor_y, float *d_min,
			int *grab);
void _AxesDoClipping(AxesWidget w, int nsegs, RSeg *segs, float x0, float y0,
			SegArray *m);
char *_Axes_check_parentheses(char *s);
void _AxesClearArea(AxesWidget w, int x, int y, int width, int height);
void _AxesClearArea2(AxesWidget w, int x, int y, int width, int height);
void _AxesDrawSegments(AxesWidget w, GC gc, DSegment *segs, int nsegs);
void _AxesDrawSegments2(AxesWidget w, GC gc, DSegment *segs, int nsegs);
void _AxesDrawString(AxesWidget w, GC gc, int x, int y, const char *str);
void _AxesDrawString2(AxesWidget w, GC gc, int x, int y, const char *str);
double _AxesTimeFactor(AxesWidget w, double d, Boolean setLabel);
void _Axes_AdjustScrollBars(AxesWidget w);
void _AxesSetCursor(AxesWidget w, const char *type);
Boolean _AxesDragScroll(AxesWidget w, XEvent *event);
void AxesWaitCursor(Widget w, Boolean on);
void * AxesMalloc(Widget w, int nbytes);
void * AxesRealloc(Widget w, void *ptr, int nbytes);
void _AxesRedisplayOverlays(AxesWidget w);
void _AxesRedisplayOverlays2(AxesWidget w);

#ifdef __STDC__
void _AxesWarn(AxesWidget widget, const char *format, ...);
void _AxesWarning(AxesWidget widget, const char *format, ...);
#else
void _AxesWarn(va_alist);
void _AxesWarning(va_alist);
#endif
char *AxesGetError(AxesWidget w);

/// @endcond

#endif	/* _AXESP_H_ */
