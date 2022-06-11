#ifndef _LIBDRAW_X_H_
#define	_LIBDRAW_X_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "libgplot.h"

/** @defgroup libdrawx library libdrawx
*/
typedef struct
{
	Boolean		selected;
	short		display;
	short		type;	
	Pixel		fg;
	int		size;
} SymbolInfo;

#define SYMBOL_INFO_INIT \
{ \
	False,			/* selected */ \
	MAP_ON,			/* display */ \
	FILLED_TRIANGLE,	/* type  */ \
	(Pixmap)1,		/* fg */ \
	8,			/* size */ \
}

typedef struct
{
	Boolean		selected;
	short		display;
	Boolean		xorr;
	Pixel		bg;
	Pixel		fg;
	int		line_width;
	int		line_style;
	int		cap_style;
	int		join_style;
	char		*dashes;
	int		n_dashes;
	int		dash_offset;
} LineInfo;

#define LINE_INFO_INIT \
{ \
	False,		/* selected */ \
	MAP_ON,		/* display */ \
	False,		/* xorr */ \
	(Pixel)0,	/* bg */ \
	(Pixel)1,	/* fg */ \
	0,		/* line_width */ \
	LineSolid,	/* line_style */ \
	CapButt,	/* cap_style */ \
	JoinMiter,	/* join_style */ \
	(char *)NULL,	/* *dashes */ \
	0,		/* n_dashes */ \
	0,		/* dash_offset */ \
}

/* ****** GetStrImage.c ********/
XImage *GetStrImage(Display *display, Window window, GC gc, XFontStruct *font,
			GC erase_gc, char *str, int length);


/* ****** SetCursor.c   ********/
void CreateAllCursors(Widget w, Pixel fg);
Cursor CreateCursor(Widget w, const char *type, Pixel fg);
Cursor GetCursor(Widget w, const char *type);


/* ****** StrToPixel.c ********/
Pixel StringToPixel(Widget widget, const char *color_name);
Pixel pixelBrighten(Widget w, Pixel pixel, double percent);



/* ****** shade.c ********/
void shade(Display *display, Drawable window, GC gc, DrawStruct *d,
			int num_colors, Pixel *colors, double *lines,
			int num_stipples, Pixmap *stipples, Matrx *a);
void shadeImage(Screen *screen, XImage *image, DrawStruct *d,
	Pixel no_data_color, Boolean *select_val, Boolean *dim_val,
	int num_colors, Pixel *colors, double *lines, Matrx *a, int *x0,
	int *y0, int *width, int *height, Boolean apply_anti_alias,
	Boolean distinctOutliers, Pixel Select_color);

/* ****** BitmapFont.c ********/
int BitmapFont(Widget w, XFontStruct *font, char *str, char **bitmap,
		int *bitmap_width, int *bitmap_height);

/* ****** plotxyd.c ********/
void plotxyd(DrawStruct *d, int n, double *x, float *y, double xmin,
	double xmax, double ymin, double ymax);
void plotsymd(DrawStruct *d, int n, double *x, float *y, int symbol,
	int symbol_size, double xmin, double xmax, double ymin, double ymax);

#endif /* _LIBDRAW_X_H_ */
