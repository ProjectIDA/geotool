/** \file TabClass.cpp
 *  \brief Defines class TabClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include "widget/TabClass.h"
#include "motif++/Application.h"


TabClass::TabClass(const string &name, Component *parent) :
			Component(name,parent)
{
    base_widget = XtVaCreateManagedWidget(getName(), tabWidgetClass,
			parent->baseWidget(), NULL);
    installDestroyHandler();
    init();
}

TabClass::TabClass(const string &name, Component *parent, Arg *args, int n) :
		Component(name, parent)
{
    base_widget = XtCreateManagedWidget(getName(), tabWidgetClass,
			parent->baseWidget(), args, n);
    installDestroyHandler();
    init();
}

void TabClass::init(void)
{
    tw = (TabWidget)base_widget;
    XtAddCallback((Widget)tw, XtNtabCallback, tabCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNtabMenuCallback, tabMenuCallback,
			(XtPointer)this);
    XtAddCallback((Widget)tw, XtNinsensitiveTabCallback,
			insensitiveTabCallback, (XtPointer)this);
    enableCallbackType(XtNtabCallback);
    enableCallbackType(XtNtabMenuCallback);
    enableCallbackType(XtNinsensitiveTabCallback);

    TabSetAppContext(tw, Application::getApplication()->appContext());
}

TabClass::~TabClass(void)
{
}

void TabClass::destroy(void)
{
    if(base_widget) {
	XtRemoveCallback(base_widget, XtNtabCallback, tabCallback,
		(XtPointer)this);
	XtRemoveCallback(base_widget, XtNtabMenuCallback, tabMenuCallback,
		(XtPointer)this);
	XtRemoveCallback(base_widget, XtNinsensitiveTabCallback,
		insensitiveTabCallback, (XtPointer)this);
    }
    Component::destroy();
}

void TabClass::tabCallback(Widget w, XtPointer client, XtPointer calldata)
{
    TabClass *tab = (TabClass *)client;
    tab->doCallbacks(w, calldata, XtNtabCallback);
}

void TabClass::tabMenuCallback(Widget w, XtPointer client, XtPointer calldata)
{
    TabClass *tab = (TabClass *)client;
    tab->doCallbacks(w, calldata, XtNtabMenuCallback);
}

void TabClass::insensitiveTabCallback(Widget w, XtPointer client,
		XtPointer calldata)
{
    TabClass *tab = (TabClass *)client;
    tab->doCallbacks(w, calldata, XtNinsensitiveTabCallback);
}
