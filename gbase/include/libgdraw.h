#ifndef _LIBGDRAW_H_
#define	_LIBGDRAW_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gobject/TimeSeries.h"
#include "fnan.h"
#include "PrintStruct.h"

#define SQUARE			3
#define TRIANGLE		4
#define PLUS			5
#define EX			6
#define INV_TRIANGLE		7
#define DIAMOND			8
#define CIRCLE			9

#define MAP_AZIMUTH		10
#define MAP_DELTA		11
#define MAP_GRID		12
#define MAP_INCREMENT		13
#define MAP_QUAD		14
#define MAP_BASE		15
#define MAP_ARC			16
#define MAP_ELLIPSE		17
#define MAP_STATION		18
#define MAP_SOURCE		19

#define FILLED_SQUARE		23
#define FILLED_TRIANGLE		24
#define FILLED_INV_TRIANGLE	27
#define FILLED_DIAMOND		28
#define FILLED_CIRCLE		29

/* the order of these map defs is important */
#define MAP_LOCKED_OFF		0
#define MAP_OFF			1
#define MAP_SELECTED_ON		2
#define MAP_ON			3
#define MAP_LOCKED_ON		4
#define MAP_NONE                5
#define MAP_LESS                6
#define MAP_MORE                7
#define MAP_ALL                 8


typedef struct {
	short x1, y1, x2, y2;
} DSegment;

typedef struct {
	short x, y;
} DPoint;

typedef struct
{
	DSegment	*segs;
	int		size_segs;
	int		nsegs;
	int		start;
	int		nshow;
} SegArray;

#define SEG_ARRAY_INIT \
{ \
	(DSegment *)NULL,	/* segs */ \
	0,			/* size_segs */ \
	0,			/* nsegs */ \
	0,			/* start */ \
	0,			/* nshow */ \
}

typedef struct
{
	DPoint	*points;
	int	size_points;
	int	npoints;
} PointArray;

typedef struct
{
	char 		label[80];
	int		x;
	int		y;
	float		size;
	float		angle;
} LabelStruct;

typedef struct
{
	int		n_ls;
	int		size_ls;
	LabelStruct	*ls;
} LabelArray;

typedef struct
{
	double	x;
	double	y;
	double	dist;
	int	type;
	int	side;
	int	ndex;
	int	npts;
} BorderPoint;

typedef struct
{
	int		ix1, iy1, ix2, iy2;
	double		sx1, sy1, sx2, sy2, 
			scalex, scaley, unscalex, unscaley;

	double		drawxmin, drawymin, drawxmax, drawymax;
	
	SegArray	*s;
	LabelArray	*l;
	
	int		lastin;
	double		lastx, lasty;
	short		last_ix, last_iy;
	bool		moved;
	bool		repeating_x, repeating_y;
	short		repeat_ymin, repeat_ymax;
	short		repeat_xmin, repeat_xmax;

	PointArray	*p;

	int		nbuf;
	int		size_buf;
	DPoint		*point_buf;

	int		nborder;
	BorderPoint	*border;
} DrawStruct;

#define DRAW_STRUCT_INIT \
{\
	0, 0, 0, 0,	/* ix1, iy1, ix2, iy2 */ \
	0.,0.,0.,0.,	/* sx1, sy1, sx2, sy2 */ \
	0.,0.,0.,0.,	/* scalex, scaley, unscalex, unscaley */ \
	0.,0.,0.,0.,	/* drawxmin, drawymin, drawxmax, drawymax */ \
	 \
	(SegArray *)NULL,	/* s */ \
	(LabelArray *)NULL,	/* l */ \
	 \
	0,			/* lastin */ \
	0.,0.,			/* lastx, lasty */ \
	0, 0,			/* last_ix, last_iy */ \
	False,False,False,	/* moved, repeating_x, repeating_y */ \
	0,0,			/* repeat_ymin, repeat_ymax */ \
	0,0,			/* repeat_xmin, repeat_xmax */ \
	(PointArray *)NULL,	/* p */ \
	0,0,			/* nbuf, size_buf */ \
	(DPoint *)NULL,		/* point_buf */ \
	0,			/* nborder */ \
	(BorderPoint *)NULL,	/* border */ \
}

typedef struct
{
	float x1, y1, x2, y2;
} RSeg;

typedef struct
{
	RSeg		*segs;
	int		size_segs;
	int		nsegs;
	int		start;
	int		nshow;
	float		xmin, xmax, ymin, ymax;
} RSegArray;

typedef struct
{
	int             nx, ny;
	double		*x, *y;
	float           *z, z_min, z_max;
	int             imin, jmin, imax, jmax;
	double		x1, x2, y1, y2;
	float		exc;
} Matrx;

#define S_LIM	10000		/* self imposed limit on the absolute
				 * magnitude of segment coordinates given
				 * to XDrawSegments.
				 */

#define MAXLAB 20 	/* maximum number of x or y labels. */

#define NOPREF  -1
#define LEFT	0
#define RIGHT	1
#define TOP	2
#define BOTTOM	3

typedef struct axis_segs
{
	int		n_segs, size_segs;
	DSegment	*segs;
} AxisSegs;

typedef void (*FontSizeMethod)(void *font, char *lab, int *width, int *height,
				int *ascent);

typedef struct axes_parm
{
	int	min_xlab, max_xlab, min_ylab, max_ylab;
	int	min_xsmall, max_xsmall, min_ysmall, max_ysmall;
	int	nxlab, nylab, nxdeci, nydeci, maxcx, maxcy;
	int	title_x, title_y, xlab_x, xlab_y, ylab_x, ylab_y;
	int	ylab_w, ylab_h;
	FontSizeMethod fontMethod;
	int	axis_font_height, axis_font_width;
	int	label_font_height, label_font_width;
	int	title_font_height, title_font_width;
	bool	auto_x, auto_y, y_label_int;
	bool	uniform_scale, center;
	bool	log_x, log_y;
	bool	x_small_log, x_medium_log;
	bool	y_small_log, y_medium_log;
	bool	check_y_cursor;
	int	left, right, top, bottom;
	int	ew, ns;
	double	time_interval, small_interval;
	double	xtic, ytic, xsmall, ysmall;
	double	x_lab[MAXLAB], y_lab[MAXLAB];
	double	x_lab_tran[MAXLAB], y_lab_tran[MAXLAB];
	double	xmin, xmax, ymin, ymax;
	int	x_xlab[MAXLAB], x_xlab2[MAXLAB], y_xlab, y_xlab2, r_ylab;
	int	x_ylab[MAXLAB], y_ylab[MAXLAB];
	int	x_ylab2[MAXLAB], y_ylab2[MAXLAB];
	int	xlab_ascent[MAXLAB], ylab_ascent[MAXLAB];
	int	xlab_width[MAXLAB], ylab_height[MAXLAB];
	int	max_xlab_height, max_ylab_width, max_ylab_height;
	int	xlab_off[MAXLAB], ylab_off[MAXLAB];
	char	*xlab[MAXLAB], *ylab[MAXLAB];
	char	*top_title, *x_axis_label, *y_axis_label;
	void 	*font;

	AxisSegs	axis_segs[4];
} AxesParm;

#ifndef	AXES_PARM_INIT
#ifndef CNULL
#define CNULL (char *)NULL
#endif
#define	AXES_PARM_INIT \
{ \
/*	2, 11, 2, 11,*/	/* min_xlab, max_xlab, min_ylab, max_ylab */ \
	3, 12, 3, 12,	/* min_xlab, max_xlab, min_ylab, max_ylab */ \
	1, 4, 1, 4,	/* min_xsmall, max_xsmall, min_ysmall, max_ysmall */ \
	0, 0, 0, 0, 0, 0,/* nxlab, nylab, nxdeci, nydeci, maxcx, maxcy */ \
	0, 0, 		/* title_x, title_y */ \
	0, 0, 0, 0,	/* lab_x, xlab_y, ylab_x, ylab_y */ \
	0, 0,		/* ylab_w, ylab_h */ \
	NULL,		/* font_method */ \
	0, 0,		/* axis_font_height, axis_font_width */ \
	0, 0,		/* label_font_height, label_font_width */ \
	0, 0,		/* title_font_height, title_font_width */ \
	True, True,	/* auto_x, auto_y */ \
	False,		/* y_label_int */ \
	False,		/* uniform_scale */ \
	True,		/* center */ \
	False, False,	/* log_x, log_y */ \
	False, False,	/* x_small_log, x_medium_log */ \
	False, False,	/* y_small_log, y_medium_log */ \
	True, 		/* check_y_cursor */ \
	0, 0, 0, 0,	/* left, right, top, bottom */ \
	0, 0,		/* ew, ns */ \
	0., 0.,		/* time_interval, small_interval */ \
	0., 0., 0., 0., /* xtic, ytic, xsmall, ysmall */ \
	{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,  \
	 0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,}, /* x_lab */ \
	{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,  \
	 0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,}, /* y_lab */ \
	{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,  \
	 0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,}, /* x_lab_tran */ \
	{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,  \
	 0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,}, /* y_lab_tran */ \
	0.,0.,0.,0., 	/* xmin, xmax, ymin, ymax */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* x_xlab */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* x_xlab2 */ \
	0, 0, 0,	/* y_xlab, y_xlab2, r_ylab */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* x_ylab */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* y_ylab */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* x_ylab2 */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* y_ylab2 */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* xlab_ascent */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* ylab_ascent */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* xlab_width */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* ylab_height */ \
	0, 0, 0,	/* max_xlab_height, max_ylab_width, max_ylab_height */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* xlab_off */ \
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}, /* ylab_off */ \
	{CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL, \
	 CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL},/*xlab*/\
	{CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL, \
	 CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL},/*ylab*/\
	CNULL, CNULL, CNULL,	/* top_title, x_axis_label, y_axis_label */ \
	(void *)NULL,	/* font */ \
	{ \
		{0, 0, (DSegment *)NULL}, \
		{0, 0, (DSegment *)NULL}, \
		{0, 0, (DSegment *)NULL}, \
		{0, 0, (DSegment *)NULL},	/* axis_segs[4] */ \
	}, \
}

#endif

#define	TIME_SCALE_SECONDS	0
#define	TIME_SCALE_VARIABLE	1
#define	TIME_SCALE_HMS		2

#define AXES_NONE	0
#define AXES_X		1
#define AXES_Y		2
#define AXES_XY		3
#define AXES_NO_XY	4

#define DOTS_PER_INCH   300     /* PostScript dots per inch */
#define POINTS_PER_DOT  (72./300.)


/* ****** DrawAxis.c ********/

void gdrawAxis(AxesParm *a, DrawStruct *d, double x1, double x2, int x_pos,
			double y1, double y2, int y_pos, bool ticks_in,
			int display_axes_labels, int time_scale,
			double time_factor);
void gdrawXAxis(AxesParm *A, DrawStruct *d, int ia, double y, double xmin,
			double xmax, double xtic, int x_pos, bool ticks_in,
			int ew, int fnt, double fac);
void gdrawYAxis(AxesParm *A, DrawStruct *d, int ia, double x, double ymin,
			double ymax, double ytic, bool ticks_in, int ns,
			int fnt, double fac);
void gdrawTimeAxis(AxesParm *A, DrawStruct *d, int ia, double y, double xmin,
			double xmax, int nxdeci, double xtic, int x_pos,
			bool ticks_in, bool label_it);
char *gdrawFracSec(int n, int val);
void gdrawCVlon(double xcoord, int ndec, char *lab, int len);
void gdrawCVlat(double ycoord, int ndec, char *lab, int len);


/* ****** DrawSubs.c ********/
void imove(DrawStruct *d, double x, double y);
void idraw(DrawStruct *d, double x, double y);
void iflush(DrawStruct *d);
void SetScale(DrawStruct *d, double x1, double y1, double x2, double y2);
void GetScale(DrawStruct *d, double *x1, double *y1, double *x2, double *y2);
void SetClipArea(DrawStruct *d, double x1, double y1, double x2, double y2);
void GetClipArea(DrawStruct *d, double *x1, double *y1, double *x2, double *y2);
void ResetClipArea(DrawStruct *d);
void SetDrawArea(DrawStruct *d, int ix1, int iy1, int ix2, int iy2);
void GetDrawArea(DrawStruct *d, int *ix1, int *iy1, int *ix2, int *iy2);
int unscale_x(DrawStruct *d, double x);
int unscale_y(DrawStruct *d, double y);
double scale_x(DrawStruct *d, int x);
double scale_y(DrawStruct *d, int y);


/* ****** mapalf.c ********/
void mapalf(DrawStruct *d, int x, int y, float size, float angle, int coord_sys,
			 char *s);


/* ****** plotsym.c ********/
void plotsym(DrawStruct *d, int n, float *x, float *y, int symbol,
			int symbol_size, double xmin, double xmax, double ymin,
			double ymax);
int DrawSymbolFromName(char *name);


/* ****** plotxd.c ********/
void plotxd(int n, float *x, float *y, double mean, double xscale,
			double yscale, double xbeg, double x_incr,RSegArray *r);
void draw_ts(TimeSeries ts, DataPoint beg, DataPoint end, double mean,
			double xscale, double yscale, RSegArray *r);


/* ****** plotxd_dbl.c ********/
void plotxd_dbl(int n, float *x, double *y, double mean, double xscale,
			double yscale, double xbeg, double x_incr,RSegArray *r);


/* ****** plotxy.c ********/
void plotxy(DrawStruct *d, int n, float *x, float *y, double xmin, double xmax,
			double ymin, double ymax);


/* ****** plotxy_dbl.c ********/
void plotxy_dbl(DrawStruct *d, int n, float *x, double *y, double xmin,
			double xmax, double ymin, double ymax);


/* ****** pltcon.c ********/
void condata(DrawStruct *d, float label_size, Matrx *a, bool auto_ci,
			float *ci, float cmin, float cmax);
void contour2(DrawStruct *d, float label_size, int n1, int nx, int ny, float *a,
			double *xg, double *yg, float exc, bool auto_ci,
			float *ci, float cmin, float cmax, int *imin, int *jmin,
			int *imax, int *jmax);


/* ****** shade.c ********/
void shadePS(FILE *fp, DrawStruct *d, int num_colors, double *lines, Matrx *a,
			int xmin, int xmax, int ymin, int ymax);
void fillRectPS(FILE *fp, int x, int y, int width, int height);

/* ****** rgb.c ********/

void rgbToHsv(double r, double g, double b, double *h, double *s, double *v);
void hsvToRgb(double h, double s, double v, double *r, double *g, double *b);
void rgbBrighten(double *r, double *g, double *b, double percent);

/* ****** DrawFill.c ********/
void drawFillPoints(DrawStruct *d, int npts, double *x, double *y);

/* ****** drawps.c ********/
void gdrawInitPS(FILE *fp, bool do_color, bool do_portrait);


#endif /* _LIBGDRAW_H_ */
