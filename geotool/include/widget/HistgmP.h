#ifndef	_HISTGMP_H_
#define	_HISTGMP_H_

#include <Xm/Xm.h>
extern "C" {
#include "libgplot.h"
}
#include "widget/Histgm.h"
#include "widget/AxesP.h"

/// @cond

typedef struct
{
	Pixel		data_fg;
	GC		dataGC, colorGC, selectGC;

	double		dz;

	SegArray        s;
	
	double		*line;
	double		*ratio;
	int		which_line, last_cursor_x, last_cursor_y;

	int		bar_height;
	int		num_bins, num_colors, num_stipples;
	int		last_x_min, last_x_max;
	float		*bins;
	Pixel		*colors;
	Pixmap		*stipples;
	int		selected;
	Boolean		ctrl_motion;

	XtCallbackList 	select_callback;
	XtCallbackList 	stretch_callback;
} HistgmPart;

typedef struct
{
	int	empty;
} HistgmClassPart;

typedef struct _HistgmRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
	HistgmPart	histgm;
} HistgmRec;

typedef struct _HistgmClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
	HistgmClassPart	histgm_class;
} HistgmClassRec;

extern HistgmClassRec histgmClassRec;

/* ******** from Histgm.c for use in libwgets only ********/

void _HistgmRedisplay(HistgmWidget w);
void _HistgmRedraw(HistgmWidget w);

/// @endcond

#endif	/* _HISTGMP_H_ */
