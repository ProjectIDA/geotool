/** \file BitmapFont.c
 *  \brief Defines routine BitmapFont.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      BitmapFont
 */

#include "stdlib.h"
#include <Xm/Xm.h>

#include "libdrawx.h"
/*
 *  Return a bitmap of the string for the input font.
 */

int
BitmapFont(Widget w, XFontStruct *font, char *string,
		char **bitmap, int *bitmap_width, int *bitmap_height)
{
	Display *display = XtDisplay(w);
	Pixmap pixmap = (Pixmap)NULL;
	GC gc = (GC)NULL;
	XCharStruct overall;
	Pixel background, foreground;
	int i, j, ascent, descent, direction, width, height, pixmap_x, pixmap_y;
	int n_width, nbytes;
	XImage *image;
	char *bytes;

	XTextExtents(font, string, (int)strlen(string), &direction, &ascent,
		&descent, &overall);

	width = overall.width;
	height = overall.descent + overall.ascent;

	pixmap = XCreatePixmap(display, XtWindow(w), width, height,
			DefaultDepth(display, DefaultScreen(display)));

	gc 	= XCreateGC(display, pixmap, 0, 0);
	background = StringToPixel(w, "white");
	foreground = StringToPixel(w, "black");
	XSetBackground(display, gc, background);

	pixmap_x = 0;
	pixmap_y = overall.ascent;
	
	XSetForeground(display, gc, background);
	XFillRectangle(display, pixmap, gc, 0, 0, width, height);

	image = XGetImage(display, pixmap, 0, 0, width, height, AllPlanes,
			XYPixmap);

	XSetForeground(display, gc, foreground);
	XDrawImageString(display, pixmap, gc, pixmap_x, pixmap_y, string,
			(int)strlen(string));

	image = XGetImage(display, pixmap, 0, 0, width, height,
				AllPlanes, XYPixmap);

	n_width = (width-1)/8 + 1;
	bytes = (char *)malloc(n_width * height);
	for(i = 0; i < n_width*height; i++) bytes[i] = 0;

	/* Create a bitmap from the image
	 */
	nbytes = -1;
	for(j = 0; j < height; j++)
	{
	    for(i = 0; i < width; i++)
	    {
		int bit = i - 8*(i/8);
		if(bit == 0) nbytes++;

		if(XGetPixel(image, i, j) == foreground) 
		{
		    bytes[nbytes] |= (0x01)<<(7-bit);
		}
	    }
	}

	XFreePixmap(display, pixmap);
	XFreeGC(display, gc);
	XDestroyImage(image);

	*bitmap = bytes;
	*bitmap_width = width;
	*bitmap_height = height;
	return nbytes;
}
