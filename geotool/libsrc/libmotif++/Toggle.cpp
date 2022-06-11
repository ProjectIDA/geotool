/** \file Toggle.cpp
 *  \brief Defines class Toggle.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/Toggle.h"
#include "motif++/Parse.h"
using namespace Parse;

/** Constructor with one-of-many option, X-resources and listener.
 *  @param[in] name the name given to this Toggle instance. The name is
 *              the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNvalueChangedCallback actions.
 *  @param[in] one_of_many if true, this toggle is part of a group in which
 * 	one and only one toggle is always selected.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
Toggle::Toggle(const string &name, Component *parent, ActionListener *listener,
		bool one_of_many, Arg *args, int n) : Component(name, parent)
{
    init(parent, one_of_many, args, n);
    if(listener) addActionListener(listener, XmNvalueChangedCallback);
}

/** Constructor X-resources and listener.
 *  @param[in] name the name given to this Toggle instance. The name is
 *              the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNvalueChangedCallback actions.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
Toggle::Toggle(const string &name, Component *parent, ActionListener *listener,
		Arg *args, int n) : Component(name, parent)
{
    init(parent, false, args, n);
    if(listener) addActionListener(listener, XmNvalueChangedCallback);
}

/** Constructor X-resources.
 *  @param[in] name the name given to this Toggle instance. The name is
 *              the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
Toggle::Toggle(const string &name, Component *parent, Arg *args, int n)
		: Component(name, parent)
{
    init(parent, false, args, n);
}

/** Constructor position and listener.
 *  @param[in] comp_name the name given to this Toggle instance. The name is
 *              the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] position the position of the Toggle
 *  @param[in] listener for XmNvalueChangedCallback actions.
 */
Toggle::Toggle(const string &comp_name, Component *parent, int position,
		ActionListener *listener) : Component(comp_name, parent)
{
    short pos = (short)position;
    Arg args[1];
    int i;

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

    XtSetArg(args[0], XmNpositionIndex, pos);

    init(parent, false, args, 1);
    if(listener) addActionListener(listener, XmNvalueChangedCallback);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] one_of_many if true, this toggle is part of a group in which
 * 	one and only one toggle is always selected.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
void Toggle::init(Component *parent, bool one_of_many, Arg *args, int n)
{
    if(!one_of_many) {
	if(!args) {
	    base_widget = XtVaCreateManagedWidget(getName(),
			xmToggleButtonWidgetClass, parent->baseWidget(), NULL);
	}
	else {
	    base_widget = XtCreateManagedWidget(getName(),
			xmToggleButtonWidgetClass, parent->baseWidget(),args,n);
	}
    }
    else {
	if(!args) {
	    base_widget = XtVaCreateManagedWidget(getName(),
			xmToggleButtonWidgetClass, parent->baseWidget(),
			XmNindicatorType, XmONE_OF_MANY,
			NULL);
	}
	else {
	    base_widget = XtVaCreateManagedWidget(getName(),
			xmToggleButtonWidgetClass, parent->baseWidget(),
			XmNindicatorType, XmONE_OF_MANY,
			NULL);
	    XtSetValues(base_widget, args, n);
	}
    }
    installDestroyHandler();
    installAccelerators();

    XtAddCallback(base_widget, XmNvalueChangedCallback,
		Toggle::valueChangedCallback, (XtPointer)this);

    enableCallbackType(XmNvalueChangedCallback);
    enableCallbackType(XtNsetLabelCallback);
}

/** Destructor. */
Toggle::~Toggle(void)
{
    if(base_widget) {
	XtRemoveAllCallbacks(base_widget, XmNvalueChangedCallback);
    }
}

void Toggle::valueChangedCallback(Widget w, XtPointer clientData,
				XtPointer calldata)
{
    Toggle *toggle = (Toggle *)clientData;

    toggle->doCallbacks(w, calldata, XmNvalueChangedCallback);
}

/** Set the state of the Toggle with optional callbacks.
 *  @param[in] state
 *  @param[in] do_callbacks it true, the XmNvalueChangedCallback callbacks are
 *	called.
 */
void Toggle::set(bool set_state, bool do_callbacks)
{
    if(base_widget) {
	XmToggleButtonSetState(base_widget, set_state, do_callbacks);
    }
}

/** Get the state.
 *  @returns the state of the Toggle.
 */
bool Toggle::state(void)
{
    if(base_widget) {
	return XmToggleButtonGetState(base_widget);
    }
    return false;
}

/** Change the Toggle label. Do the XtNsetLabelCallback callbacks.
 *  @param[in] label the new label.
 */
void Toggle::setLabel(const string &label)
{
    XmString xm = createXmString(label);
    XtVaSetValues(base_widget, XmNlabelString, xm, NULL);
    XmStringFree(xm);
    doCallbacks(base_widget, (XtPointer)label.c_str(),
		(const char *)XtNsetLabelCallback);
}

/** Get the Toggle label.
 *  @returns the Toggle label. Free the pointer when it is no longer needed.
 */
char * Toggle::getLabel(void)
{
    Arg args[1];
    XmString xm;
    XtSetArg(args[0], XmNlabelString, &xm);
    XtGetValues(base_widget, args, 1);
    char *s =  getXmString(xm);
    XmStringFree(xm);
    return s;
}

ParseCmd Toggle::parseCmd(const string &cmd, string &msg)
{
    string s;

    if(parseCompare(cmd, "true")) {
	set(true, true);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "false")) {
	set(false, true);
	return COMMAND_PARSED;
    }
    else if(parseArg(cmd, "state", s)) {
	if(parseCompare(s, "true") || parseCompare(s, "1")) {
	    set(true, true);
	}
	else if(parseCompare(s, "false") || parseCompare(s, "0")) {
	    set(false, true);
	}
	else {
	    msg.assign(string(getName()) + ".state: unrecognized value: " + s);
	    return ARGUMENT_ERROR;
	}
	return COMMAND_PARSED;
    }
    return Component::parseCmd(cmd, msg);
}

ParseVar Toggle::parseVar(const string &name, string &value)
{
    if(parseCompare(name, "state")) {
	if(state()) {
	    value.assign("true");
	    return VARIABLE_TRUE;
	}
	else {
	    value.assign("false");
	    return VARIABLE_FALSE;
	}
    }
    return Component::parseVar(name, value);
}

void Toggle::parseHelp(const char *prefix)
{
    printf("%s=(true,false)\n", prefix);
}
