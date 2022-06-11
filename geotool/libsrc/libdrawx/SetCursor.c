/** \file SetCursor.c
 *  \brief Defines X-cursor routines.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      CreateCursor: 	creates some common cursors
 *
 * AUTHOR
 *      I. Henson
 */

#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>

#include "libdrawx.h"

#define hour_glass_width 16	/* hourglass cursor */
#define hour_glass_height 16
static unsigned char hour_glass_bits[] = {
   0xff, 0x7f, 0x21, 0x42, 0x11, 0x44, 0x39, 0x4e, 0x79, 0x4f, 0xf9, 0x4f,
   0xf1, 0x47, 0xe1, 0x43, 0xe1, 0x43, 0x91, 0x44, 0x89, 0x48, 0x89, 0x48,
   0xc9, 0x49, 0xf1, 0x47, 0xe1, 0x43, 0xff, 0x7f};

#define hand_width 19		/* hand cursor */
#define hand_height 14
static  unsigned char hand_bits[] = {
 0x00,0x00,0xf8,0xfc,0x07,0xf8,0x02,0x08,0xf8,0xfc,0x11,0xf8,0x20,0x20,0xf8,
 0x10,0x20,0xf8,0xe0,0x21,0xf8,0x10,0x30,0xf8,0xe0,0x49,0xf8,0x40,0x84,0xf8,
 0x80,0x03,0xf9,0x00,0x92,0xf8,0x00,0x44,0xf8,0x00,0x28,0xf8};

#define move_width 18		/* move window cursor */
#define move_height 18
static unsigned char move_bits[] = {
 0x00,0x00,0xfc,0x00,0x00,0xfc,0x00,0x03,0xfc,0x80,0x07,0xfc,0xc0,0x0f,0xfc,
 0x00,0x03,0xfc,0x10,0x23,0xfc,0x18,0x63,0xfc,0xfc,0xff,0xfc,0xfc,0xff,0xfc,
 0x18,0x63,0xfc,0x10,0x23,0xfc,0x00,0x03,0xfc,0xc0,0x0f,0xfc,0x80,0x07,0xfc,
 0x00,0x03,0xfc,0x00,0x00,0xfc,0x00,0x00,0xfc};

#define plus_width 16		/* crosshair */
#define plus_height 18
static unsigned char plus_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
 0x01,0xfc,0x7f,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x00,0x00,0x00,0x00,0x00};

#define resize_up_width 16	/* resize up cursor */
#define resize_up_height 17
static unsigned char resize_up_bits[] = {
 0x00,0x00,0xfe,0x7f,0xfe,0x7f,0x00,0x00,0x00,0x01,0x80,0x03,0x40,0x05,0x20,
 0x09,0x10,0x11,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x00,0x00,0x00};

#define resize_down_width 16	/* resize down cursor */
#define resize_down_height 17
static unsigned char resize_down_bits[] = {
 0x00,0x00,0x00,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,
 0x00,0x88,0x08,0x90,0x04,0xa0,0x02,0xc0,0x01,0x80,0x00,0x00,0x00,0xfe,0x7f,
 0xfe,0x7f,0x00,0x00};

#define resize_right_width 17	/* resize right cursor */
#define resize_right_height 16
static unsigned char resize_right_bits[] = {
 0x00,0x00,0xfe,0x00,0xc0,0xfe,0x00,0xc0,0xfe,0x00,0xc0,0xfe,0x00,0xc1,0xfe,
 0x00,0xc2,0xfe,0x00,0xc4,0xfe,0x00,0xc8,0xfe,0xfc,0xdf,0xfe,0x00,0xc8,0xfe,
 0x00,0xc4,0xfe,0x00,0xc2,0xfe,0x00,0xc1,0xfe,0x00,0xc0,0xfe,0x00,0xc0,0xfe,
 0x00,0x00,0xfe};

#define resize_left_width 17	/* resize left cursor */
#define resize_left_height 16
static unsigned char resize_left_bits[] = {
 0x00,0x00,0xfe,0x06,0x00,0xfe,0x06,0x00,0xfe,0x06,0x01,0xfe,0x86,0x00,0xfe,
 0x46,0x00,0xfe,0x26,0x00,0xfe,0xf6,0x7f,0xfe,0x26,0x00,0xfe,0x46,0x00,0xfe,
 0x86,0x00,0xfe,0x06,0x01,0xfe,0x06,0x00,0xfe,0x06,0x00,0xfe,0x06,0x00,0xfe,
 0x00,0x00,0xfe};

#define arrow_width 19		/* arrow pointing north west */
#define arrow_height 18
static unsigned char arrow_bits[] = {
 0x00,0x00,0xf8,0x00,0x00,0xf8,0x0c,0x00,0xf8,0x3c,0x00,0xf8,0xf8,0x00,0xf8,
 0xf8,0x03,0xf8,0xf0,0x0f,0xf8,0xf0,0x31,0xf8,0xe0,0x01,0xf8,0x60,0x02,0xf8,
 0x40,0x04,0xf8,0x40,0x08,0xf8,0x80,0x10,0xf8,0x80,0x20,0xf8,0x00,0x40,0xf8,
 0x00,0x80,0xf8,0x00,0x00,0xf8,0x00,0x00,0xf8};

static Cursor hourglass_cursor = None;
static Cursor hand_cursor = None;
static Cursor move_cursor = None;
static Cursor crosshair_cursor = None;
static Cursor resize_up_cursor = None;
static Cursor resize_down_cursor = None;
static Cursor resize_right_cursor = None;
static Cursor resize_left_cursor = None;
static Cursor arrow_cursor = None;


void
CreateAllCursors(Widget w, Pixel fg)
{
	hourglass_cursor = CreateCursor(w, "hourglass", fg);
	hand_cursor = CreateCursor(w, "hand", fg);
	move_cursor = CreateCursor(w, "move", fg);
	crosshair_cursor = CreateCursor(w, "crosshair", fg);
	resize_up_cursor = CreateCursor(w, "resize up", fg);
	resize_down_cursor = CreateCursor(w, "resize down", fg);
	resize_right_cursor = CreateCursor(w, "resize right", fg);
	resize_left_cursor = CreateCursor(w, "resize left", fg);
	arrow_cursor = CreateCursor(w, "arrow", fg);
}

Cursor
GetCursor(Widget w, const char *type)
{
	Cursor cursor;

	if(hourglass_cursor == None) {
	    Pixel fg = 0;
	    Arg args[1];

	    XtSetArg(args[0], XtNforeground, &fg);
	    XtGetValues(w, args, 1);
	    CreateAllCursors(w, fg);
	}
	    
	if(!strcasecmp(type, "hourglass")) {
	    cursor = hourglass_cursor;
	}
	else if(!strcasecmp(type, "hand")) {
	    cursor = hand_cursor;
	}
	else if(!strcasecmp(type, "move")) {
	    cursor = move_cursor;
	}
	else if(!strcasecmp(type, "crosshair") || !strcasecmp(type, "plus")) {
	    cursor = crosshair_cursor;
	}
	else if(!strcasecmp(type, "resize up")) {
	    cursor = resize_up_cursor;
	}
	else if(!strcasecmp(type, "resize down")) {
	    cursor = resize_down_cursor;
	}
	else if(!strcasecmp(type, "resize right")) {
	    cursor = resize_right_cursor;
	}
	else if(!strcasecmp(type, "resize left")) {
	    cursor = resize_left_cursor;
	}
	else if(!strcasecmp(type, "arrow")) {
	    cursor = arrow_cursor;
	}
	else {
	    fprintf(stderr, "GetCursor: unknown cursor type: %s\n", type);
	    return None;
	}
	if(cursor == None) {
	    fprintf(stderr, "GetCursor: cursor not created.\n");
	    fprintf(stderr, "Call CreateAllCursors before GetCursor.\n");
	    return None;
	}
	return cursor;
}

Cursor
CreateCursor(Widget w, const char *type, Pixel fg)
{
	int cursor_width, cursor_height;
	int cursor_x_hot, cursor_y_hot;
	unsigned char *bits;
	Pixmap w_pixmap;
	XColor	cfore, cback;
	Colormap cmap;
	Cursor cursor;

	if(!strcasecmp(type, "hourglass")) {
	    cursor_width = hour_glass_width;
	    cursor_height = hour_glass_height;
	    cursor_x_hot = 7;
	    cursor_y_hot = 7;
	    bits = hour_glass_bits;
	}
	else if(!strcasecmp(type, "hand")) {
	    cursor_width = hand_width;
	    cursor_height = hand_height;
	    cursor_x_hot = 0;
	    cursor_y_hot = 1;
	    bits = hand_bits;
	}
	else if(!strcasecmp(type, "move")) {
	    cursor_width = move_width;
	    cursor_height = move_height;
	    cursor_x_hot = 8;
	    cursor_y_hot = 8;
	    bits = move_bits;
	}
	else if(!strcasecmp(type, "crosshair") || !strcasecmp(type, "plus")) {
	    cursor_width = plus_width;
	    cursor_height = plus_height;
	    cursor_x_hot = 7;
	    cursor_y_hot = 8;
	    bits = plus_bits;
	}
	else if(!strcasecmp(type, "resize up")) {
	    cursor_width = resize_up_width;
	    cursor_height = resize_up_height;
	    cursor_x_hot = 8;
	    cursor_y_hot = 1;
	    bits = resize_up_bits;
	}
	else if(!strcasecmp(type, "resize down")) {
	    cursor_width = resize_down_width;
	    cursor_height = resize_down_height;
	    cursor_x_hot = 8;
	    cursor_y_hot = 16;
	    bits = resize_down_bits;
	}
	else if(!strcasecmp(type, "resize right")) {
	    cursor_width = resize_right_width;
	    cursor_height = resize_right_height;
	    cursor_x_hot = 16;
	    cursor_y_hot = 7;
	    bits = resize_right_bits;
	}
	else if(!strcasecmp(type, "resize left")) {
	    cursor_width = resize_left_width;
	    cursor_height = resize_left_height;
	    cursor_x_hot = 1;
	    cursor_y_hot = 7;
	    bits = resize_left_bits;
	}
	else if(!strcasecmp(type, "arrow")) {
	    cursor_width = arrow_width;
	    cursor_height = arrow_height;
	    cursor_x_hot = 1;
	    cursor_y_hot = 1;
	    bits = arrow_bits;
	}
	else {
	    fprintf(stderr, "CreateCursor: unknown cursor type: %s\n", type);
	    return None;
	}
	w_pixmap = XCreateBitmapFromData(XtDisplay(w), XtWindow(w),
			(char *)bits, cursor_width, cursor_height);

	cmap = XDefaultColormapOfScreen(XtScreen(w));

	cfore.pixel = fg;
	XQueryColor(XtDisplay(w), cmap, &cfore);

	cursor = XCreatePixmapCursor(XtDisplay(w), w_pixmap, w_pixmap,
			&cfore, &cback, cursor_x_hot, cursor_y_hot);

	return(cursor);
}
