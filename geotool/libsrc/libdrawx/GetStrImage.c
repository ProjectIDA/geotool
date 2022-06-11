/** \file GetStrImage.c
 *  \brief Defines routine GetStrImage.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      GetStrImage
 *
 * AUTHOR
 *      Ivan Henson
 *      Teledyne Geotech
 */

#include "stdlib.h"
#include <Xm/Xm.h>

#include "libdrawx.h"
/*
 *  Return an image of the string rotated vertically.
 *  To display the image at x, y do:
 *	w = overall.width;
 *	h = overall.descent + overall.ascent;
 *	x -= overall.ascent;
 *	y -= overall.width;
 *	XPutImage(display, window, gc, transpose_image, 0, 0, x, y, h, w);
 */

XImage *
GetStrImage(Display *display, Window window, GC gc, XFontStruct *font,
		GC erase_gc, char *string, int length)
{
	Pixmap pixmap;
	XImage *transpose_image;
	static char *data;
	GC local_gc = (GC)NULL, local_erase_gc = (GC)NULL;
	XCharStruct overall;
	int i, j, ascent, descent, direction, width, height, pixmap_x, pixmap_y;
	int depth;
	XImage *image;

	XTextExtents(font, string, length, &direction, &ascent, &descent,
			&overall);

	width = overall.width;
	height = overall.descent + overall.ascent;

	pixmap = XCreatePixmap(display, window, width, height,
			DefaultDepth(display, DefaultScreen(display)));

	local_gc 	= XCreateGC(display, pixmap, 0, 0);
	local_erase_gc 	= XCreateGC(display, pixmap, 0, 0);

	XCopyGC(display, gc, ~(GCClipMask | GCClipXOrigin | GCClipYOrigin),
		local_gc);

	XCopyGC(display, erase_gc, ~(GCClipMask|GCClipXOrigin|GCClipYOrigin),
		local_erase_gc);

	pixmap_x = 0;
	pixmap_y = overall.ascent;
	
	XFillRectangle(display, pixmap, local_erase_gc, 0, 0, width, height);

	XDrawImageString(display, pixmap, local_gc, pixmap_x, pixmap_y,
				string, length);

	image = XGetImage(display, pixmap, 0, 0, width, height,
				AllPlanes, XYPixmap);

	depth = DefaultDepth(display, DefaultScreen(display));
	data = (char *)malloc(width * height * 8);

	transpose_image = XCreateImage(display,
		DefaultVisual(display, DefaultScreen(display)), depth,
		XYPixmap, 0, data, height, width, 8, 0);

	/* Transpose the image.
	 */
	for(j = 0; j < height; j++)
	{
		for(i = 0; i < width; i++)
		{
			XPutPixel(transpose_image, j, width-i-1,
					XGetPixel(image, i, j));
		}
	}

	XFreePixmap(display, pixmap);
	XFreeGC(display, local_gc);
	XFreeGC(display, local_erase_gc);
	XDestroyImage(image);

	return(transpose_image);
}
