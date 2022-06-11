/** \file AxesClass.cpp
 *  \brief Defines class AxesClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "widget/AxesClass.h"
#include "widget/Parser.h"
#include "motif++/Application.h"

static void setParseCursor(int n, AxesCursorCallbackStruct *a,
			AxesCursorCallbackStruct *c);

AxesClass::AxesClass(const string &name, Component *parent, Arg *args, int n) :
		Component(name, parent)
{
    initWidget(axesWidgetClass, name, parent, args, n);
}

// use this constructor in a "Widget" subclass (ie CPlot, ConPlot, etc)
AxesClass::AxesClass(WidgetClass widget_class, const string &name,
		Component *parent, Arg *args, int n) : Component(name, parent)
{
    initWidget(widget_class, name, parent, args, n);
}

void AxesClass::initWidget(WidgetClass widget_class, const string &name,
			Component *parent, Arg *args,int n)
{
    base_widget = XtCreateManagedWidget(getName(), widget_class,
			parent->baseWidget(), args, n);
    installDestroyHandler();
    aw = (AxesWidget)base_widget;
    AxesSetClass(aw, this);
   
    XtAddCallback(base_widget, XtNcrosshairCallback,
		AxesClass::crosshairCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNcrosshairDragCallback,
		AxesClass::crosshairDragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNlineCallback,
		AxesClass::lineCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNlineDragCallback,
		AxesClass::lineDragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNdoubleLineCallback,
		AxesClass::doubleLineCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNdoubleLineDragCallback,
		AxesClass::doubleLineDragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNdoubleLineScaleCallback,
		AxesClass::doubleLineScaleCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNphaseLineCallback,
		AxesClass::phaseLineCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNphaseLineDragCallback,
		AxesClass::phaseLineDragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNlimitsCallback,
		AxesClass::limitsCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNhorizontalScrollCallback,
		AxesClass::horizontalScrollCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNmagnifyCallback,
		AxesClass::magnifyCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNwarningCallback,
		AxesClass::warningCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNresizeCallback,
		AxesClass::resizeCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNkeyPressCallback,
		AxesClass::keyPressCallback, (XtPointer)this);

    enableCallbackType(XtNcrosshairCallback);
    enableCallbackType(XtNcrosshairDragCallback);
    enableCallbackType(XtNlineCallback);
    enableCallbackType(XtNlineDragCallback);
    enableCallbackType(XtNdoubleLineCallback);
    enableCallbackType(XtNdoubleLineDragCallback);
    enableCallbackType(XtNdoubleLineScaleCallback);
    enableCallbackType(XtNphaseLineCallback);
    enableCallbackType(XtNphaseLineDragCallback);
    enableCallbackType(XtNlimitsCallback);
    enableCallbackType(XtNhorizontalScrollCallback);
    enableCallbackType(XtNmagnifyCallback);
    enableCallbackType(XtNwarningCallback);
    enableCallbackType(XtNresizeCallback);
    enableCallbackType(XtNkeyPressCallback);

    if( !inArgs(XtNzoomControls, args, n) ) {
	Arg a[1];
	XtSetArg(a[0], XtNzoomControls, True);
	setValues(a, 1);
    }

    Application *app = Application::getApplication();
    app->addReservedName("line_label");
    app->addReservedName("line_time");
    app->addReservedName("num_crosshairs");
    app->addReservedName("num_lines");
    app->addReservedName("num_time_windows");
    app->addReservedName("plot_xmin");
    app->addReservedName("plot_xmax");
    app->addReservedName("plot_ymin");
    app->addReservedName("plot_tmin");
    app->addReservedName("plot_ymax");
    app->addReservedName("plot_tmax");
}

AxesClass::~AxesClass(void)
{
}

void AxesClass::destroy(void)
{
    if(base_widget) {
	XtRemoveCallback(base_widget, XtNcrosshairCallback,
		AxesClass::crosshairCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNcrosshairDragCallback,
		AxesClass::crosshairDragCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNlineCallback,
		AxesClass::lineCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNlineDragCallback,
		AxesClass::lineDragCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNdoubleLineCallback,
		AxesClass::doubleLineCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNdoubleLineDragCallback,
		AxesClass::doubleLineDragCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNdoubleLineScaleCallback,
		AxesClass::doubleLineScaleCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNphaseLineCallback,
		AxesClass::phaseLineCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNphaseLineDragCallback,
		AxesClass::phaseLineDragCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNlimitsCallback,
		AxesClass::limitsCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNhorizontalScrollCallback,
		AxesClass::horizontalScrollCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNmagnifyCallback,
		AxesClass::magnifyCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNwarningCallback,
		AxesClass::warningCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNresizeCallback,
		AxesClass::resizeCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNkeyPressCallback,
		AxesClass::keyPressCallback, (XtPointer)this);
    }
    Component::destroy();
}

void AxesClass::crosshairCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getCrosshairs(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNcrosshairCallback);
}
void AxesClass::crosshairDragCallback(Widget w, XtPointer client,XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getCrosshairs(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNcrosshairDragCallback);
}
void AxesClass::lineCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNlineCallback);
}
void AxesClass::lineDragCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNlineDragCallback);
}
void AxesClass::doubleLineCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getDoubleLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNdoubleLineCallback);
}
void AxesClass::doubleLineDragCallback(Widget w, XtPointer client,
		XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getDoubleLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNdoubleLineDragCallback);
}
void AxesClass::doubleLineScaleCallback(Widget w, XtPointer client,
		XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getDoubleLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNdoubleLineScaleCallback);
}
void AxesClass::phaseLineCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getPhaseLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNphaseLineCallback);
}
void AxesClass::phaseLineDragCallback(Widget w, XtPointer client,XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    AxesCursorCallbackStruct *a;
    int n = aw->getPhaseLines(&a);
    setParseCursor(n, a, (AxesCursorCallbackStruct *)data);
    aw->doCallbacks(w, data, XtNphaseLineDragCallback);
}
void AxesClass::limitsCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    aw->doCallbacks(w, data, XtNlimitsCallback);
}
void AxesClass::horizontalScrollCallback(Widget w, XtPointer c, XtPointer data)
{
    AxesClass *aw = (AxesClass *)c;
    aw->doCallbacks(w, data, XtNhorizontalScrollCallback);
}
void AxesClass::magnifyCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    aw->doCallbacks(w, data, XtNmagnifyCallback);
}
void AxesClass::warningCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    aw->showWarning((char *)data);
    aw->doCallbacks(w, data, XtNwarningCallback);
}
void AxesClass::resizeCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    aw->doCallbacks(w, data, XtNresizeCallback);
}
void AxesClass::keyPressCallback(Widget w, XtPointer client, XtPointer data)
{
    AxesClass *aw = (AxesClass *)client;
    aw->doCallbacks(w, data, XtNkeyPressCallback);
}

bool AxesClass::inArgs(const char *name, Arg *args, int n)
{
    for(int i = 0; i < n; i++) {
	if( !strcasecmp(args[i].name, name) ) return true;
    }
    return false;
}

ParseCmd AxesClass::parseCmd(const string &cmd, string &msg)
{
    int i;
    bool err;
    const char *c;
    string label;
    bool do_callback=true, draw_label=true;
    double x, y, xmin, xmax, ymin, ymax;

    if(parseFind(cmd, "zoom", msg, &err, "xmin", true, "xmax", true,
		"ymin", true, "ymax", true, "remember", false))
    {
	if(err) return ARGUMENT_ERROR;

	bool remember = false;
        if(!parseGetArg(cmd, "zoom", msg, "xmin", &xmin)) {
	    return ARGUMENT_ERROR;
	}
        if(!parseGetArg(cmd, "zoom", msg, "xmax", &xmax)) {
	    return ARGUMENT_ERROR;
	}
        if(!parseGetArg(cmd, "zoom", msg, "ymin", &ymin)) {
	    return ARGUMENT_ERROR;
	}
        if(!parseGetArg(cmd, "zoom", msg, "ymax", &ymax)) {
	    return ARGUMENT_ERROR;
	}
	parseGetArg(cmd, "zoom", msg, "remember", &remember);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
	zoom(xmin, xmax, ymin, ymax, remember);
    }
    else if(parseFind(cmd, "unzoom", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	unzoom();
    }
    else if(parseFind(cmd, "unzoom_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	unzoomAll();
    }
    else if(parseFind(cmd, "unzoom_vertical_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	unzoomVerticalAll();
    }
    else if(parseFind(cmd, "unzoom_horizontal_all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	unzoomHorizontalAll();
    }
    else if(parseFind(cmd, "position_crosshair", msg, &err,
		"i", false, "x", true, "y", true, "notify", false))
    {
	if(err) return ARGUMENT_ERROR;
	c = "position_crosshair";
	i = 1;
        parseGetArg(cmd, c, msg, "i", &i);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
        if(!parseGetArg(cmd, c, msg, "x", &x)) return ARGUMENT_ERROR;
        if(!parseGetArg(cmd, c, msg, "y", &y)) return ARGUMENT_ERROR;
	parseGetArg(cmd, c, msg, "notify", &do_callback);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
	positionCrosshair(i-1, x, y, do_callback);
    }
    else if(parseFind(cmd, "position_line_cursor", msg, &err,
		"i", false, "x", true, "label", false, "notify", false))
    {
	if(err) return ARGUMENT_ERROR;
	c = "position_line_cursor";
	i = 1;
        parseGetArg(cmd, c, msg, "i", &i);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
        if(!parseGetArg(cmd, c, msg, "x", &x)) return ARGUMENT_ERROR;
	parseGetArg(cmd, "label", label);
	parseGetArg(cmd, c, msg, "notify", &do_callback);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
	if(!label.empty()) {
	    positionLine(label, x, do_callback);
	}
	else {
	    positionLine(i-1, x, do_callback);
	}
    }
    else if(parseFind(cmd, "position_phase_line", msg, &err,
		"x", true, "phase", true, "notify", false))
    {
	if(err) return ARGUMENT_ERROR;
	c = "position_phase_line";
        if(!parseGetArg(cmd, c, msg, "x", &x)) return ARGUMENT_ERROR;
	parseGetArg(cmd, "phase", label);
	parseGetArg(cmd, c, msg, "notify", &do_callback);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
	positionPhaseLine(label, x, do_callback);
    }
    else if(parseFind(cmd, "position_time_window", msg, &err,
	"xmin", true, "xmax", true, "label", false, "notify", false,
	"draw_label", false))
    {
	if(err) return ARGUMENT_ERROR;
	c = "position_time_window";
        if(!parseGetArg(cmd, c, msg, "xmin", &xmin)) {
	    return ARGUMENT_ERROR;
	}
        if(!parseGetArg(cmd, c, msg, "xmax", &xmax)) {
	    return ARGUMENT_ERROR;
	}
	parseGetArg(cmd, "label", label);
	parseGetArg(cmd, c, msg, "notify", &do_callback);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
	parseGetArg(cmd, c, msg, "draw_label", &do_callback);
	if(msg[0] != '\0') return ARGUMENT_ERROR;
	if(!label.empty()) {
	    positionDoubleLine(label, xmin, xmax, do_callback, draw_label);
	}
	else {
	    positionDoubleLine(0, xmin, xmax, do_callback, draw_label);
	}
    }
    else if(parseFind(cmd, "delete_crosshair", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	deleteCrosshair();
    }
    else if(parseFind(cmd, "delete_time_window", msg, &err,
		"label", false))
    {
	if(err) return ARGUMENT_ERROR;
	parseGetArg(cmd, "label", label);
	if(!label.empty()) {
	    deleteDoubleLine(label);
	}
	else {
	    deleteDoubleLine("a");
	}
    }
    else if(parseFind(cmd, "delete_line", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	deleteLine();
    }
    else if(parseFind(cmd, "delete_phase_line", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	deletePhaseLine();
    }
    else if(parseFind(cmd, "delete_all_cursors", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	deleteAllCursors();
    }
    else {
	return Component::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar AxesClass::parseVar(const string &name, string &value)
{
    double min, max;
    AxesCursorCallbackStruct *a;
    int n;

    if(parseCompare(name, "plot_xmin")) {
	getXLimits(&min, &max);
	parsePrintDouble(value, min);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "plot_xmax")) {
	getXLimits(&min, &max);
	parsePrintDouble(value, max);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "plot_ymin")) {
	getYLimits(&min, &max);
	parsePrintDouble(value, min);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "plot_ymax")) {
	getYLimits(&min, &max);
	parsePrintDouble(value, max);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "plot_tmin")) {
	getXLimits(&min, &max);
	parsePrintDouble(value, min);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "plot_tmax")) {
	getXLimits(&min, &max);
	parsePrintDouble(value, max);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "num_time_windows")) {
	n = getDoubleLines(&a);
	parsePrintInt(value, n);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "time_window[", 12)) {
	return parseTimeWindow(name, value);
    }
    else if(parseCompare(name, "num_crosshairs")) {
	n = getCrosshairs(&a);
	parsePrintInt(value, n);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "crosshair[", 10)) {
	return parseCrosshair(name, value);
    }
    else if(parseCompare(name, "num_phase_lines")) {
	n = getPhaseLines(&a);
	parsePrintInt(value, n);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "phase_line[", 12)) {
	return parsePhaseLine(name, value);
    }
    else if(parseCompare(name, "num_line_cursors")) {
	n = getLines(&a);
	parsePrintInt(value, n);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "line_cursor[", 12)) {
	return parseLineCursor(name, value);
    }
    else {
	return Component::parseVar(name, value);
    }
}

ParseVar AxesClass::parseTimeWindow(const string &name, string &value)
{
    char s[30];
    AxesCursorCallbackStruct *a;
    int n = getDoubleLines(&a);

    for(int i = 0; i < n; i++) {
	snprintf(s, sizeof(s), "time_window[%d].tmin", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_x1);
	    return STRING_RETURNED;
	}
	snprintf(s, sizeof(s), "time_window[%d].tmax", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_x2);
	    return STRING_RETURNED;
	}
	snprintf(s, sizeof(s), "time_window[%d].duration", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_x2-a[i].scaled_x1);
	    return STRING_RETURNED;
	}
    }
    return VARIABLE_NOT_FOUND;
}

ParseVar AxesClass::parseCrosshair(const string &name, string &value)
{
    char s[30];
    AxesCursorCallbackStruct *a;
    int n = getCrosshairs(&a);

    for(int i = 0; i < n; i++) {
	snprintf(s, sizeof(s), "crosshair[%d].x", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_x);
	    return STRING_RETURNED;
	}
	snprintf(s, sizeof(s), "crosshair[%d].y", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_y);
	    return STRING_RETURNED;
	}
    }
    return VARIABLE_NOT_FOUND;
}

ParseVar AxesClass::parsePhaseLine(const string &name, string &value)
{
    char s[30];
    AxesCursorCallbackStruct *a;
    int n = getPhaseLines(&a);

    for(int i = 0; i < n; i++) {
	snprintf(s, sizeof(s), "phase_line[%d].x", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_x);
	    return STRING_RETURNED;
	}
	snprintf(s, sizeof(s), "phase_line[%d].label", i+1);
	if(parseCompare(name, s)) {
	    value.assign(a[i].label);
	    return STRING_RETURNED;
	}
    }
    return VARIABLE_NOT_FOUND;
}

ParseVar AxesClass::parseLineCursor(const string &name, string &value)
{
    char s[30];
    AxesCursorCallbackStruct *a;
    int n = getLines(&a);

    for(int i = 0; i < n; i++) {
	snprintf(s, sizeof(s), "line_cursor[%d].x", i+1);
	if(parseCompare(name, s)) {
	    parsePrintDouble(value, a[i].scaled_x);
	    return STRING_RETURNED;
	}
	snprintf(s, sizeof(s), "line_cursor[%d].label", i+1);
	if(parseCompare(name, s)) {
	    value.assign(a[i].label);
	    return STRING_RETURNED;
	}
    }
    return VARIABLE_NOT_FOUND;
}

void AxesClass::parseHelp(const char *prefix)
{
    printf("\n");
    printf("%sdelete_all_cursors\n", prefix);
    printf("%sdelete_crosshair\n", prefix);
    printf("%sdelete_line\n", prefix);
    printf("%sdelete_phase_line\n", prefix);
    printf("%sdelete_time_window [label=LABEL]\n", prefix);
    printf("%sposition_crosshair x=XCOORD y=YCOORD\n", prefix);
    printf("%sposition_line x=XCOORD [label=LABEL] [notify=(true,false)]\n", prefix);
    printf("%sposition_phase_line x=XCOORD phase=PHASE [notify=(true,false)]\n", prefix);
    printf("%sposition_time_window xmin=XMIN xmax=XMAX [label=LABEL] [notify=(true,false)]\n", prefix);
    printf("%sunzoom\n", prefix);
    printf("%sunzoom_all\n", prefix);
    printf("%sunzoom_horizontal_all\n", prefix);
    printf("%sunzoom_vertical_all\n", prefix);
    printf("%szoom xmin=XMIN xmax=XMAX ymin=YMIN ymax=YMAX\n", prefix);

    printf("\nAttributes\n");
    printf("%scrosshair_x\n", prefix);
    printf("%scrosshair_y\n", prefix);
    printf("%scrosshair_label\n", prefix);
    printf("%sline_label\n", prefix);
    printf("%sline_time\n", prefix);
    printf("%snum_crosshairs\n", prefix);
    printf("%snum_lines\n", prefix);
    printf("%snum_time_windows\n", prefix);
    printf("%stime_window_a_min\n", prefix);
    printf("%stime_window_a_max\n", prefix);
    printf("%stime_window_a_width\n", prefix);
    printf("%stime_window_b_min\n", prefix);
    printf("%stime_window_b_max\n", prefix);
    printf("%stime_window_b_width\n", prefix);
    printf("%stime_window_c_min\n", prefix);
    printf("%stime_window_c_max\n", prefix);
    printf("%stime_window_c_width\n", prefix);
    printf("%stmin\n", prefix);
    printf("%stmax\n", prefix);
    printf("%sxmin\n", prefix);
    printf("%sxmax\n", prefix);
    printf("%symin\n", prefix);
    printf("%symax\n", prefix);
}

static void
setParseCursor(int n, AxesCursorCallbackStruct *a, AxesCursorCallbackStruct *c)
{
    int i;
    for(i = 0; i < n && a[i].index != c->index; i++);
    if(i < n) {
	char s[10];
	snprintf(s, sizeof(s), "%d", i+1);
	Application::putParseProperty("cursor_index", s);
    }
}
