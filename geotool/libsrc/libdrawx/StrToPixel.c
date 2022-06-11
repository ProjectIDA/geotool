/** \file StrToPixel.c
 *  \brief Defines routines StrToPixel and pixelBrighten.
 */
#include "config.h"
#include <string.h>
#include <Xm/Xm.h>

#include "libdrawx.h"

Pixel
StringToPixel(Widget w, const char *color_name)
{
	XColor c;

	if(sscanf(color_name, "%hd.%hd.%hd", &c.red, &c.green, &c.blue) == 3
	|| sscanf(color_name, "%hd %hd %hd", &c.red, &c.green, &c.blue) == 3)
	{
	    c.red *= 256;
	    c.green *= 256;
	    c.blue *= 256;
	    c.flags = DoRed | DoGreen | DoBlue;
	    if(XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &c))
	    {
		return c.pixel;
	    }
	}
	else {
	    XrmValue from, to;
	    to.addr = NULL;
	    to.size = 0;
	    from.addr = (XPointer)color_name;
	    from.size = strlen(color_name) + 1;

	    if(XtConvertAndStore(w, XtRString, &from, XtRPixel, &to) && to.addr)
	    {
		return( *(Pixel *) to.addr );
	    }
	}

	return(BlackPixelOfScreen(XtScreen (w)));
}

Pixel
pixelBrighten(Widget w, Pixel pixel, double percent)
{
        XColor color;
        double r, g, b;

        color.pixel = pixel;
        XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
                DefaultScreen(XtDisplay(w))), &color);

        r = color.red;
        g = color.green;
        b = color.blue;
        rgbBrighten(&r, &g, &b, percent);
        color.red = (unsigned int)r;
        color.green = (unsigned int)g;
        color.blue = (unsigned int)b;

        XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
                DefaultScreen(XtDisplay(w))), &color);

        return color.pixel;
}
