#ifndef _HISTGM_CLASS_H_
#define _HISTGM_CLASS_H_

#include "widget/AxesClass.h"
#include "widget/Histgm.h"

/** This class is the interface to the Histgm Widget.
 *
 *  @ingroup libwgets
 */
class HistgmClass : public AxesClass
{
    public:

	HistgmClass(const char *name, Component *parent, Arg *args, int n);
	HistgmClass(const char *name, Component *parent, Arg *args, int n,
			ActionListener *listener);
	~HistgmClass(void);

	void destroy(void);

	void initWidget(void);

	void input(int num_colors, Pixel *pixels, int num_stipples,
		Pixmap *stipples, double *l, int num_bins,
		float *bins, float data_min, float data_max)
	{
	    HistgmInput(histgm, num_colors, pixels, num_stipples, stipples,
			l, num_bins, bins, data_min, data_max);
	}
	void inputData(int npts, float *z, float no_data_flag, int num_bins,
		int num_colors, Pixel *pixels, int num_stipples,
		Pixmap *stipples, double *l)
	{
	    HistgmInputData(histgm, npts, z, no_data_flag, num_bins,
		num_colors, pixels, num_stipples, stipples, l);
	}
	void lines(int num_colors, int num_stipples, double *l) {
	    HistgmLines(histgm, num_colors, num_stipples, l);
	}
	void colors(int num_colors, Pixel *pixels) {
	    HistgmColors(histgm, num_colors, pixels);
	}

    protected:
	// use this constructor in a "Widget" subclass
	HistgmClass(WidgetClass widget_class, const char *name,
			Component *parent, Arg *args, int n);

	HistgmWidget	histgm;

    private:
	static void selectCallback(Widget, XtPointer, XtPointer);
	static void stretchCallback(Widget, XtPointer, XtPointer);

};

#endif
