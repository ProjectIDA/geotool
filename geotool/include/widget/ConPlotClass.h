#ifndef _CONPLOT_CLASS_H_
#define _CONPLOT_CLASS_H_

#include "widget/AxesClass.h"
#include "widget/ConPlot.h"

/** This class is the interface to the ConPlot Widget.
 *
 *  @ingroup libwgets
 */
class ConPlotClass : public AxesClass
{
    public:

	ConPlotClass(const string &name, Component *parent, Arg *args=NULL,
		int n=0) : AxesClass(conPlotWidgetClass, name, parent, args, n)
	{
	    cw = (ConPlotWidget)base_widget;
	    XtAddCallback(base_widget, XtNselectDataCallback,
		ConPlotClass::selectDataCallback, (XtPointer)this);
	    XtAddCallback(base_widget, XtNselectBarCallback,
		ConPlotClass::selectBarCallback, (XtPointer)this);
	    XtAddCallback(base_widget, XtNmouseOverCallback,
		ConPlotClass::mouseOverCallback, (XtPointer)this);
	    enableCallbackType(XtNselectDataCallback);
	    enableCallbackType(XtNselectBarCallback);
	    enableCallbackType(XtNmouseOverCallback);
	}

	~ConPlotClass(void) {}

	void destroy(void)
	{
	    if(base_widget) {
		XtRemoveCallback(base_widget, XtNselectDataCallback,
			ConPlotClass::selectDataCallback, (XtPointer)this);
		XtRemoveCallback(base_widget, XtNselectBarCallback,
			ConPlotClass::selectBarCallback, (XtPointer)this);
		XtRemoveCallback(base_widget, XtNmouseOverCallback,
			ConPlotClass::mouseOverCallback, (XtPointer)this);
	    }
	    this->AxesClass::destroy();
	}

	void clear(void) { ConPlotClear(cw); }
	void clearEntries(void) { ConPlotClearEntries(cw); }
	int input(const char *label, int nx, int ny, double *x, double *y,
		float *z, float no_data_flag, int num_lines, double *lines,
		double xmax, double ymax, bool display_data, bool display_grid,
		bool redraw_axes, bool distinctOutliers, Pixel Select_color);
	int input(const char *label, int nx, int ny, double *x, double *y,
		float *z) {
	    return input(label, nx, ny, x, y, z, 1.e+30, 0, NULL, 0., 0., true,
			false, true, false, 0);
	}
	void dimPixels(int nx, int ny, bool *dim);
	void selectCells(int nx, int ny, bool *select);
 	void selectAllCells(bool select, bool redisplay=true);
	void dimEntry(int x, int y, bool value);
	void dimOff(void);
	void display(int num, int *ids, bool *display_data,bool *display_grid);
	bool output(int *nx, int *ny, double **x, double **y, float **z);
	bool colors(int num_colors, int *r, int *g,int *b,float dim_percent);
	bool setColors(Pixel *pixels, int num_colors);
	void setStipples(int num_stipples, char *stipple_bits,
		int stipple_width, int stipple_height);
	void colorLines(int num_lines, double *lines);
	int getPixels(Pixel **pixels);
	int getStipples(Pixmap **stipples);
	bool privateCells(void);
	Matrx *getMatrix(void);
	void setBarValues(int num, double *values, char **labels,
		bool redisplay);
	void setParams(double contour_interval, double contour_min,
		double contour_max);

        ParseCmd parseCmd(const string &cmd, string &msg) {
	    return AxesClass::parseCmd(cmd, msg);
	}

    protected:

	ConPlotWidget	cw;

    private:

    static void selectDataCallback(Widget w, XtPointer client, XtPointer data) {
	ConPlotClass *cp = (ConPlotClass *)client;
	cp->doCallbacks(w, data, XtNselectDataCallback);
    }
    static void selectBarCallback(Widget w, XtPointer client, XtPointer data) {
	ConPlotClass *cp = (ConPlotClass *)client;
	cp->doCallbacks(w, data, XtNselectBarCallback);
    }
    static void mouseOverCallback(Widget w, XtPointer client, XtPointer data) {
	ConPlotClass *cp = (ConPlotClass *)client;
	cp->doCallbacks(w, data, XtNmouseOverCallback);
    }
};

#endif
