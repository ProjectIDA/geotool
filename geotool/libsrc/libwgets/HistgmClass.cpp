/** \file HistgmClass.cpp
 *  \brief Defines class HistgmClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>
using namespace std;

#include "widget/HistgmClass.h"

HistgmClass::HistgmClass(const char *name, Component *parent, Arg *args, int n)
		: AxesClass(histgmWidgetClass, name, parent, args, n)
{
    initWidget();
}

HistgmClass::HistgmClass(const char *name, Component *parent, Arg *args, int n,
			ActionListener *listener)
		: AxesClass(histgmWidgetClass, name, parent, args, n)
{
    initWidget();
    addActionListener(listener, XtNselectCallback);
    addActionListener(listener, XtNstretchCallback);
}

// use this constructor in a "Widget" subclass
HistgmClass::HistgmClass(WidgetClass widget_class, const char *name,
		Component *parent, Arg *args, int n) :
		AxesClass(widget_class, name, parent, args, n)
{
    initWidget();
}

void HistgmClass::initWidget(void)
{
    histgm = (HistgmWidget)base_widget;
   
    XtAddCallback(base_widget, XtNselectCallback,
		HistgmClass::selectCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNstretchCallback,
		HistgmClass::stretchCallback, (XtPointer)this);

    enableCallbackType(XtNselectCallback);
    enableCallbackType(XtNstretchCallback);
}

HistgmClass::~HistgmClass(void)
{
}

void HistgmClass::destroy(void)
{
    if(base_widget) {
	XtRemoveCallback(base_widget, XtNselectCallback,
		HistgmClass::selectCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNstretchCallback,
		HistgmClass::stretchCallback, (XtPointer)this);
    }
    AxesClass::destroy();
}

void HistgmClass::selectCallback(Widget w, XtPointer client, XtPointer data)
{
    HistgmClass *histgm = (HistgmClass *)client;
    histgm->doCallbacks(w, data, XtNselectCallback);
}
void HistgmClass::stretchCallback(Widget w, XtPointer client, XtPointer data)
{
    HistgmClass *histgm = (HistgmClass *)client;
    histgm->doCallbacks(w, data, XtNstretchCallback);
}
