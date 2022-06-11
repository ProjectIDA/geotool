/** \file FtPlotClass.cpp
 *  \brief Defines class FtPlotClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include "FtPlotClass.h"

using namespace libgft;

FtPlotClass::FtPlotClass(const char *name, Component *parent, Arg *args, int n)
		: AxesClass(ftPlotWidgetClass, name, parent, args, n)
{
    initWidget();
}

// use this constructor in a "Widget" subclass
FtPlotClass::FtPlotClass(WidgetClass widget_class, const char *name,
		Component *parent, Arg *args, int n) :
		AxesClass(widget_class, name, parent, args, n)
{
    initWidget();
}

void FtPlotClass::initWidget(void)
{
    ftplot = (FtPlotWidget)base_widget;
   
    XtAddCallback(base_widget, XtNselectCallback,
		FtPlotClass::selectCallback, (XtPointer)this);
    enableCallbackType(XtNselectCallback);
}

FtPlotClass::~FtPlotClass(void)
{
}

void FtPlotClass::destroy(void)
{
    if(base_widget) {
	XtRemoveCallback(base_widget, XtNselectCallback,
		FtPlotClass::selectCallback, (XtPointer)this);
    }
    Component::destroy();
}

void FtPlotClass::selectCallback(Widget w, XtPointer client, XtPointer data)
{
    FtPlotClass *ftplot = (FtPlotClass *)client;
    ftplot->doCallbacks(w, data, XtNselectCallback);
}
