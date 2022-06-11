/** \file Button.cpp
 *  \brief Defines class Button.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "motif++/Button.h"
#include "motif++/Parse.h"
using namespace Parse;

using namespace std;

/** Constructor.
 *  @param[in] name the name given to this Button instance. The name is
 * 		the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNactivateCallback actions.
 */
Button::Button(const string &name, Component *parent, ActionListener *listener)
		: Component(name, parent)
{
    init(parent);
    if(listener) addActionListener(listener);
}

/** Constructor with X-resources and listener.
 *  @param[in] name the name given to this Button instance. The name is
 * 		the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 *  @param[in] listener for XmNactivateCallback actions.
 *  @param[in] position the position of the Button.
 */
Button::Button(const string &name, Component *parent, Arg *args, int n,
	    ActionListener *listener, int position) : Component(name, parent)
{
    init(parent, args, n, position);
    if(listener) addActionListener(listener);
}

/** Constructor with accelerator.
 *  @param[in] name the name given to this Button instance. The name is
 * 		the default action command string.
 *  @param[in] accel accelerator name.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNactivateCallback actions.
 */
Button::Button(const string &name, const string &accel, Component *parent,
		ActionListener *listener) : Component(name, parent)
{
    init(parent, accel);
    if(listener) addActionListener(listener);
}

/** Constructor with position and listener.
 *  @param[in] name the name given to this Button instance. The name is
 * 		the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] position the position of the Button.
 *  @param[in] listener for XmNactivateCallback actions.
 */
Button::Button(const string &name, Component *parent, int position,
		ActionListener *listener) : Component(name, parent)
{
    init(parent, position);
    if(listener) addActionListener(listener);
}

/** Constructor with accelerator, position and listener.
 *  @param[in] name the name given to this Button instance. The name is
 * 		the default action command string.
 *  @param[in] accel accelerator name.
 *  @param[in] parent the Component parent.
 *  @param[in] position the position of the Button.
 *  @param[in] listener for XmNactivateCallback actions.
 */
Button::Button(const string &name, const string &accel, Component *parent,
	    int position, ActionListener *listener) : Component(name, parent)
{
    init(parent, accel, position);
    if(listener) addActionListener(listener);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 */
void Button::init(Component *parent)
{
    init(parent, "");
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] accel accelerator name.
 */
void Button::init(Component *parent, const string &accel)
{
    Arg args[2];
    XmString xm=NULL, xms=NULL;
    int n = 0;
    if(!accel.empty()) {
	xm = createXmString((char *)accel.c_str());
	XtSetArg(args[n], XmNacceleratorText, xm); n++;
    }
    // remove leading '.'s
    int len = (int)component_name.length();
    if(len > 0 && component_name.at(len-1) == '.') {
	xms = createXmString(component_name.c_str());
	XtSetArg(args[n], XmNlabelString, xms); n++;
	while(len > 0 && component_name.at(len-1) == '.') len--;
	component_name.assign(component_name.c_str(), len);
    }
    base_widget = XtCreateManagedWidget(getName(), xmPushButtonWidgetClass,
			 parent->baseWidget(), args, n);
    if(xm) XmStringFree(xm);
    if(xms) XmStringFree(xms);

    installDestroyHandler();
    installAccelerators();
    XtAddCallback(base_widget, XmNactivateCallback,
		Button::activateCallback, (XtPointer)this);

    enableCallbackType(XmNactivateCallback);
    enableCallbackType(XtNsetLabelCallback);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] position the position of the Button.
 */
void Button::init(Component *parent, int position)
{
    init(parent, "", position);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] accel accelerator name.
 *  @param[in] position the position of the Button.
 */
void Button::init(Component *parent, const string &accel, int position)
{
    short pos = (short)position;
    Arg args[3];
    XmString xm=NULL, xms=NULL;
    int i, n = 0;

    if(position < 0) {
	vector<Component *> *ch = parent->getChildren();
 	// get the alphabetical position
	for(i = 0; i < (int)ch->size()-1; i++) {
	    if(strcmp(getName(), ch->at(i)->getName()) <= 0) break;
	}
	delete ch;
	pos = (short)i;
    }
    parent->positionLastChild((int)pos);

    XtSetArg(args[n], XmNpositionIndex, pos); n++;

    if(!accel.empty()) {
	xm = createXmString((char *)accel.c_str());
	XtSetArg(args[n], XmNacceleratorText, xm); n++;
    }
    int len = (int)component_name.length();
    if(len > 0 && component_name.at(len-1) == '.') {
	xms = createXmString(component_name.c_str());
	XtSetArg(args[n], XmNlabelString, xms); n++;
	while(len > 0 && component_name.at(len-1) == '.') len--;
	component_name.assign(component_name.c_str(), len);
    }
    base_widget = XtCreateManagedWidget(getName(), xmPushButtonWidgetClass,
			 parent->baseWidget(), args, n);
    if(xm) XmStringFree(xm);
    if(xms) XmStringFree(xms);

    installDestroyHandler();
    installAccelerators();
    XtAddCallback(base_widget, XmNactivateCallback,
		Button::activateCallback, (XtPointer)this);

    enableCallbackType(XmNactivateCallback);
    enableCallbackType(XtNsetLabelCallback);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] num the number of X-resource structures.
 *  @param[in] position the position of the Button.
 */
void Button::init(Component *parent, Arg *args, int num, int position)
{
    if(position != 0) {
	Arg a[20];
	int i, n;
	short pos = (short)position;
	if(position < 0) {
	    vector<Component *> *ch = parent->getChildren();
	    // get the alphabetical position
	    for(i = 0; i < (int)ch->size()-1; i++) {
		if(strcmp(getName(), ch->at(i)->getName()) <= 0) break;
	    }
	    delete ch;
	    pos = (short)i;
	}
	parent->positionLastChild((int)pos);

	n = 0;
	XtSetArg(a[n], XmNpositionIndex, pos); n++;

	for(i = 0; i < num && n < 20; i++, n++) {
	    a[n] = args[i];
	}
	base_widget = XtCreateManagedWidget(getName(),
			xmPushButtonWidgetClass, parent->baseWidget(), a, n);
    }
    else {
	base_widget = XtCreateManagedWidget(getName(),
			xmPushButtonWidgetClass, parent->baseWidget(),args,num);
    }
    installDestroyHandler();
    installAccelerators();
    XtAddCallback(base_widget, XmNactivateCallback,
		Button::activateCallback, (XtPointer)this);

    enableCallbackType(XmNactivateCallback);
    enableCallbackType(XtNsetLabelCallback);
}

/** Destructor.
 */
Button::~Button(void)
{
}

/** Activate the button.
 *  Calls all Button callbacks with NULL calldata.
 */
void Button::activate(void)
{
    doCallbacks(base_widget, NULL, (const char *)XmNactivateCallback);
}

void Button::activateCallback(Widget w, XtPointer clientData,
				XtPointer calldata)
{
    Button *button = (Button *)clientData;

    button->doCallbacks(w, calldata, (const char *)XmNactivateCallback);
}

/** Change the Button label.
 *  @param[in] label the new Button label.
 */
void Button::setLabel(const string &label)
{
    XmString xm = createXmString((char *)label.c_str());
    XtVaSetValues(base_widget, XmNlabelString, xm, NULL);
    XmStringFree(xm);
    doCallbacks(base_widget, (XtPointer)label.c_str(),
		(const char *)XtNsetLabelCallback);
}

ParseCmd Button::parseCmd(const string &cmd, string &msg)
{
    if(parseCompare(cmd, "activate")) {
	activate();
	return COMMAND_PARSED;
    }
    return Component::parseCmd(cmd, msg);
}
