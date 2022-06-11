/** \file Scale.cpp
 *  \brief Defines class Scale.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/Scale.h"

/** Constructor.
 *  @param[in] name the name given to this Scale instance.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNactivateCallback actions.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
Scale::Scale(const string &name, Component *parent, ActionListener *listener,
		Arg *args, int n) : Component(name, parent)
{
    init(parent, args, n);
    if(listener) {
	addActionListener(listener, XmNvalueChangedCallback);
	addActionListener(listener, XmNdragCallback);
    }
}

/** Constructor.
 *  @param[in] name the name given to this Scale instance.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
Scale::Scale(const string &name, Component *parent, Arg *args, int n)
		: Component(name, parent)
{
    init(parent, args, n);
}

/** Destructor.  */
Scale::~Scale(void)
{
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
void Scale::init(Component *parent, Arg *args, int n)
{
    base_widget = XtCreateManagedWidget(getName(), xmScaleWidgetClass,
			parent->baseWidget(), args, n);
    installDestroyHandler();
    XtAddCallback(base_widget, XmNvalueChangedCallback,
		Scale::valueChangedCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNdragCallback,
		Scale::dragCallback, (XtPointer)this);

    enableCallbackType(XmNvalueChangedCallback);
    enableCallbackType(XmNdragCallback);
}

void Scale::valueChangedCallback(Widget w, XtPointer client, XtPointer data)
{
    Scale *scale = (Scale *)client;

    scale->doCallbacks(w, data, (const char *)XmNvalueChangedCallback);
}

void Scale::dragCallback(Widget w, XtPointer client, XtPointer data)
{
    Scale *scale = (Scale *)client;

    scale->doCallbacks(w, data, (const char *)XmNdragCallback);
}

/** Get the scale maximum value.
 *  @returns the maximum value.
 */
int Scale::getMaximum(void)
{
    int max;
    Arg args[1];
    XtSetArg(args[0], XmNmaximum, &max);
    getValues(args, 1);
    return max;
}

/** Get the scale minimum value.
 *  @returns the minimum value.
 */
int Scale::getMinimum(void)
{
    int min;
    Arg args[1];
    XtSetArg(args[0], XmNminimum, &min);
    getValues(args, 1);
    return min;
}

/** Set the scale maximum value.
 */
void Scale::setMaximum(int maximum)
{
    Arg args[3];
    int min = getMinimum();
    int value = getValue();

    if(maximum < min) {
	XtSetArg(args[0], XmNminimum, maximum);
	XtSetArg(args[1], XmNvalue, maximum);
	XtSetArg(args[2], XmNmaximum, maximum);
	setValues(args, 3);
    }
    else if(value > maximum) {
	XtSetArg(args[0], XmNvalue, maximum);
	XtSetArg(args[1], XmNmaximum, maximum);
	setValues(args, 2);
    }
    else {
	XtSetArg(args[0], XmNmaximum, maximum);
	setValues(args, 1);
    }
}

/** Set the scale minimum value.
 */
void Scale::setMinimum(int minimum)
{
    Arg args[3];
    int max = getMaximum();
    int value = getValue();

    if(minimum > max) {
	XtSetArg(args[0], XmNminimum, minimum);
	XtSetArg(args[1], XmNvalue, minimum);
	XtSetArg(args[2], XmNmaximum, minimum);
	setValues(args, 3);
    }
    else if(value < minimum) {
	XtSetArg(args[0], XmNvalue, minimum);
	XtSetArg(args[1], XmNminimum, minimum);
	setValues(args, 2);
    }
    else {
	XtSetArg(args[0], XmNminimum, minimum);
	setValues(args, 1);
    }
}
