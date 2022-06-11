/** \file ScrollBar.cpp
 *  \brief Defines class ScrollBar.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/ScrollBar.h"

/** Constructor with X-resources and listener.
 *  @param[in] name the name given to this ScrollBar instance.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNactivateCallback actions.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
ScrollBar::ScrollBar(const string &name, Component *parent,
	ActionListener *listener, Arg *args, int n) : Component(name, parent)
{
    init(parent, args, n);
    if(listener) {
	addActionListener(listener, XmNvalueChangedCallback);
	addActionListener(listener, XmNdragCallback);
	addActionListener(listener, XmNincrementCallback);
	addActionListener(listener, XmNdecrementCallback);
	addActionListener(listener, XmNpageIncrementCallback);
	addActionListener(listener, XmNpageDecrementCallback);
    }
}

/** Destructor. */
ScrollBar::~ScrollBar(void)
{
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
void ScrollBar::init(Component *parent, Arg *args, int n)
{
    base_widget = XtCreateManagedWidget(getName(), xmScrollBarWidgetClass,
			getParent()->baseWidget(), args, n);
    installDestroyHandler();
    XtAddCallback(base_widget, XmNvalueChangedCallback,
		ScrollBar::valueChangedCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNdragCallback,
		ScrollBar::dragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNincrementCallback,
		ScrollBar::incrementCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNdecrementCallback,
		ScrollBar::decrementCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNpageIncrementCallback,
		ScrollBar::pageIncrementCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNpageDecrementCallback,
		ScrollBar::pageDecrementCallback, (XtPointer)this);

    enableCallbackType(XmNvalueChangedCallback);
    enableCallbackType(XmNdragCallback);
    enableCallbackType(XmNincrementCallback);
    enableCallbackType(XmNdecrementCallback);
    enableCallbackType(XmNpageIncrementCallback);
    enableCallbackType(XmNpageDecrementCallback);
}

void ScrollBar::valueChangedCallback(Widget w, XtPointer client, XtPointer data)
{
    ScrollBar *sb = (ScrollBar *)client;
    sb->doCallbacks(w, data, (const char *)XmNvalueChangedCallback);
}

void ScrollBar::dragCallback(Widget w, XtPointer client, XtPointer data)
{
    ScrollBar *sb = (ScrollBar *)client;
    sb->doCallbacks(w, data, (const char *)XmNdragCallback);
}

void ScrollBar::incrementCallback(Widget w, XtPointer client, XtPointer data)
{
    ScrollBar *sb = (ScrollBar *)client;
    sb->doCallbacks(w, data, (const char *)XmNincrementCallback);
}

void ScrollBar::decrementCallback(Widget w, XtPointer client, XtPointer data)
{
    ScrollBar *sb = (ScrollBar *)client;
    sb->doCallbacks(w, data, (const char *)XmNdecrementCallback);
}

void ScrollBar::pageIncrementCallback(Widget w, XtPointer client,XtPointer data)
{
    ScrollBar *sb = (ScrollBar *)client;
    sb->doCallbacks(w, data, (const char *)XmNpageIncrementCallback);
}

void ScrollBar::pageDecrementCallback(Widget w, XtPointer client,XtPointer data)
{
    ScrollBar *sb = (ScrollBar *)client;
    sb->doCallbacks(w, data, (const char *)XmNpageDecrementCallback);
}

/** Get the scrollbar maximum value.
 *  @returns the maximum value.
 */
int ScrollBar::getMaximum(void)
{
    int max;
    Arg args[1];
    XtSetArg(args[0], XmNmaximum, &max);
    getValues(args, 1);
    return max;
}

/** Get the scrollbar minimum value.
 *  @returns the minimum value.
 */
int ScrollBar::getMinimum(void)
{
    int min;
    Arg args[1];
    XtSetArg(args[0], XmNminimum, &min);
    getValues(args, 1);
    return min;
}

/** Get the scrollbar value.
 *  @returns the value.
 */
int ScrollBar::getValue(void)
{
    int value;
    Arg args[1];
    XtSetArg(args[0], XmNvalue, &value);
    getValues(args, 1);
    return value;
}

/** Set the scrollbar value.
 *  @param[in] value
 */
void ScrollBar::setValue(int value)
{
    Arg args[1];
    XtSetArg(args[0], XmNvalue, value);
    setValues(args, 1);
}
