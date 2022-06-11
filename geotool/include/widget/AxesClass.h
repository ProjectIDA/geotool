#ifndef _AXES_CLASS_H_
#define _AXES_CLASS_H_

#include "motif++/Component.h"
#include "widget/Axes.h"

/** @defgroup libwgets library libwgets
 *  C++ class interfaces to custom widgets
 */


/** This class is the interface to the Axes Widget.
 *
 *  AxesClass class callbacks and the callback data are:
 *	- XtNcrosshairCallback with AxesCursorCallbackStruct
 *	- XtNcrosshairDragCallback with AxesCursorCallbackStruct
 *	- XtNlineCallback with AxesCursorCallbackStruct
 *	- XtNlineDragCallback with AxesCursorCallbackStruct
 *	- XtNdoubleLineCallback with AxesCursorCallbackStruct
 *	- XtNdoubleLineDragCallback with AxesCursorCallbackStruct
 *	- XtNdoubleLineScaleCallback with AxesCursorCallbackStruct
 *	- XtNphaseLineCallback with AxesCursorCallbackStruct
 *	- XtNphaseLineDragCallback with AxesCursorCallbackStruct
 *	- XtNlimitsCallback with AxesLimitsCallbackStruct
 *	- XtNmagnifyCallback with no (NULL) callback data
 *	- XtNwarningCallback with a string (char *)
 *  @ingroup libwgets
 */
class AxesClass : public Component
{
    public:

	AxesClass(const string &name, Component *parent, Arg *args=NULL,
		int n=0);
	~AxesClass(void);

	virtual AxesClass *getAxesClassInstance(void) { return this; }

	void initWidget(WidgetClass widget_class, const string &name,
			Component *parent, Arg *args,int n);
	void destroy(void);

	DrawStruct *drawStruct(void);
	void axesExpose(bool exp);
	void clearPlot(void);
	void zoomApply(int mode) {AxesZoomApply(aw, mode);}
	void mag(double x_min, double x_max, double y_min, double y_max);
	void setMagnifyWindow(double xmin,double xmax,double ymin,double ymax);
	void showMagWindow(void);
	void zoomCancel(void);
	void setMinLimits(double xmin, double xmax, double ymin, double ymax);
	void getMinLimits(double *xmin,double *xmax,double *ymin,double *ymax);
	void getLimits(double *xmin,double *xmax,double *ymin,double *ymax);
	void getXLimits(double *xmin, double *xmax) {
	    double ymin, ymax;
	    getLimits(xmin, xmax, &ymin, &ymax);
	}
	void getYLimits(double *ymin, double *ymax) {
	    double xmin, xmax;
	    getLimits(&xmin, &xmax, ymin, ymax);
	}
	void setXLimits(double xmin, double xmax, bool init=false);
	void setYLimits(double ymin, double ymax, bool init=false) {
		AxesSetYLimits(aw, ymin, ymax, init); }
	void setLimits(double xmin, double xmax, double ymin, double ymax,
			bool init=false);
	void zoom(double xmin, double xmax, double ymin, double ymax,
			bool zoom_up);
	bool doZoom(int ix1, int iy1, int ix2, int iy2);
	void unzoom(void);
	void unzoomAll(void) { AxesUnzoomAll(aw); }
	void unzoomVerticalAll(void);
	void unzoomHorizontalAll(void);
	void setMargins(bool auto_x, bool auto_y, int left, int right,
			int top, int bottom);
	void getMargins(bool *auto_x, bool *auto_y, int *left, int *right,
			int *top, int *bottom);
	int pageAxis(XEvent *event, const char **params, Cardinal *num_params) {
	    return AxesPage(aw, event, params, num_params);
	}
	int pageAxis(const string &direction) {
	    int end;
	    Cardinal num_params = 2;
	    const char *params[2];
	    if(!strcasecmp(direction.c_str(), "left")) {
		params[0] = strdup("horizontal");
		params[1] = strdup("DOWN");
	    }
	    else if(!strcasecmp(direction.c_str(), "right")) {
		params[0] = strdup("horizontal");
		params[1] = strdup("UP");
	    }
	    else if(!strcasecmp(direction.c_str(), "up")) {
		params[0] = strdup("vertical");
		params[1] = strdup("UP");
	    }
	    else if(!strcasecmp(direction.c_str(), "down")) {
		params[0] = strdup("vertical");
		params[1] = strdup("DOWN");
	    }
	    else {
		cerr << "page: invalid direction: " << direction << endl;
		return -1;
	    }
	    end = pageAxis(NULL, params, &num_params);
	    Free(params[0]);
	    Free(params[1]);
	    return end;
	}
	int scrollAxis(const string &direction) {
	    int end;
	    Cardinal num_params = 2;
	    const char *params[2];
	    if(!strcasecmp(direction.c_str(), "left")) {
		params[0] = strdup("horizontal");
		params[1] = strdup("down");
	    }
	    else if(!strcasecmp(direction.c_str(), "right")) {
		params[0] = strdup("horizontal");
		params[1] = strdup("up");
	    }
	    else if(!strcasecmp(direction.c_str(), "up")) {
		params[0] = strdup("vertical");
		params[1] = strdup("up");
	    }
	    else if(!strcasecmp(direction.c_str(), "down")) {
		params[0] = strdup("vertical");
		params[1] = strdup("down");
	    }
	    else {
		cerr << "scroll: invalid direction: " << direction << endl;
		return -1;
	    }
	    end = pageAxis(NULL, params, &num_params);
	    Free(params[0]);
	    Free(params[1]);
	    return end;
	}
	int addCrosshair(void);
	char addLine(void);
	char addDoubleLine(bool draw_labels=true);
	void addHorDoubleLine();
	int renameCursor(const string &old_label, const string &new_label);
	int addPhaseLine(const string &phase);
	int addPhaseLine2(const string &phase);
	void deleteAllCursors(void);
	void deleteCrosshair(void);
	void deleteLimitLines(void);
	void deleteDoubleLine(const string &label);
	void deleteLine(void);
	void deletePhaseLine(void);
	bool getPhaseLine(double *x, char **phase);
	int positionCrosshair(int i, double x, double y, bool do_callback);
	int positionLine(int i, double x, bool do_callback);
	int positionLine(const string &label, double x, bool do_callback);
	int positionDoubleLine(int i, double x1, double x2,
			bool do_callback, bool draw_labels=true);
	int positionDoubleLine(const string &label, double x1, double x2,
			bool do_callback, bool draw_labels=true);
	int positionPhaseLine(const string &phase, double x, bool do_callback);
	int positionPhaseLine2(const string &phase, double x, bool do_callback);
	int renamePhaseLine(const string &phase);
	int getCursors(AxesCursorCallbackStruct **a);
	int getCrosshairs(AxesCursorCallbackStruct **a);
	int getLines(AxesCursorCallbackStruct **a);
	int getDoubleLines(AxesCursorCallbackStruct **a);
	int getPhaseLines(AxesCursorCallbackStruct **a);
	bool phaseLine(const string &label, AxesCursorCallbackStruct **p);
	bool crosshairIsDisplayed(void);
	bool lineIsDisplayed(const string &label);
	bool doubleLineIsDisplayed(const string &label);
	bool phaseLineIsDisplayed(void);
	void mark(int x, int y);
	void drawLines(int npts, double *x, double *y);
	AxesParm *hardCopy(FILE *fp, PrintParam *p, AxesParm *a);
	void postColor(FILE *fp, Pixel pixel) { AxesPostColor(aw, fp, pixel); }
	void drawRectangle(GC gc, int x1, int y1, int width, int height);
	int getZooms(int *zoom_min, double **xmin, double **xmax, double **ymin,
			double **ymax);
	void setZooms(int zoom_min, int num, double *xmin, double *xmax,
			double *ymin, double *ymax);
	int getZoomLevel(void);
	char *getError(void);
	void horizontalShift(double shift, bool scroll);
	void setOverlayRect(double x1, double x2, double y1, double y2);
	void removeOverlayRect(void);
	int setOverlayRect2(double ox1, double ox2, double oy1, double oy2,
		Pixel pixel, int width, bool display, int in_id,
		bool redisplay);
	int removeOverlayRect2(Pixel pixel, int width, int in_id,
		bool redisplay);
	void removeAllOverlays(void);
	int getXLabels(double **values, char ***labels);
	void setMinDelx(double delx_min);
	void setMinDely(double dely_min);
	void setHorizontalScroll(Widget outside_hor_scroll);
	Widget getHorizontalScroll(void);
	void setMinZoomLevel(int min_zoom_level);
	int getScrollBarHeight(void) {
	    Widget w = getHorizontalScroll();
	    Dimension h; Arg args[1];
	    XtSetArg(args[0], XtNheight, &h);
	    XtGetValues(w, args, 1);
	    return (int)h;
	}
	void redraw(void);
	void setTimeMarks(int num, double *t);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseVar parseTimeWindow(const string &name, string &value);
	ParseVar parseCrosshair(const string &name, string &value);
	ParseVar parsePhaseLine(const string &name, string &value);
	ParseVar parseLineCursor(const string &name, string &value);
	void parseHelp(const char *prefix);

	static bool inArgs(const char *name, Arg *args, int n);
	static float pointToLine(float ax, float ay, float bx, float by,
				float cx, float cy);

    protected:
	// use this constructor in a "Widget" subclass (ie CPlot, ConPlot, etc)
	AxesClass(WidgetClass widget_class, const string &name,
			Component *parent, Arg *args, int n);

	AxesWidget	aw;

    private:

	static void crosshairCallback(Widget, XtPointer, XtPointer);
	static void crosshairDragCallback(Widget, XtPointer,XtPointer);
	static void lineCallback(Widget, XtPointer, XtPointer);
	static void lineDragCallback(Widget, XtPointer, XtPointer);
	static void doubleLineCallback(Widget, XtPointer, XtPointer);
	static void doubleLineDragCallback(Widget, XtPointer, XtPointer);
	static void doubleLineScaleCallback(Widget, XtPointer, XtPointer);
	static void phaseLineCallback(Widget, XtPointer, XtPointer);
	static void phaseLineDragCallback(Widget, XtPointer,XtPointer);
	static void limitsCallback(Widget, XtPointer, XtPointer);
	static void horizontalScrollCallback(Widget, XtPointer, XtPointer);
	static void magnifyCallback(Widget, XtPointer, XtPointer);
	static void warningCallback(Widget, XtPointer, XtPointer);
	static void resizeCallback(Widget, XtPointer, XtPointer);
	static void keyPressCallback(Widget, XtPointer, XtPointer);
};

#endif
