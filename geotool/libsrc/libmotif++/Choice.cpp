/** \file Choice.cpp
 *  \brief Defines class Choice.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>
#include <iostream>
#include <X11/IntrinsicP.h> // for core.being_destroyed

#include "motif++/Choice.h"
#include "motif++/Parse.h"
using namespace Parse;

/** Constructor.
 *  @param[in] name the name given to this Choice instance.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNactivateCallback actions.
 *  @param[in] args X-resource structures.
 *  @param[in] num_args the number of X-resource structures.
 */
Choice::Choice(const string &name, Component *parent, ActionListener *listener,
			Arg *args, int num_args) : Component(name, parent)
{
    init(parent, args, num_args);
    if((!args || !num_args) && !parent->getFormDialogInstance()) parent->layout();
    if(listener) addActionListener(listener);
}

/** Constructor.
 *  @param[in] name the name given to this Choice instance.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] num_args the number of X-resource structures.
 */
Choice::Choice(const string &name, Component *parent, Arg *args, int num_args)
		: Component(name, parent)
{
    init(parent, args, num_args);
    if((!args || !num_args) && !getFormDialogInstance()) parent->layout();
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] num_args the number of X-resource structures.
 */
void Choice::init(Component *parent, Arg *args, int num_args)
{
    Arg margs[100];

    pulldown = XmCreatePulldownMenu(parent->baseWidget(),
			(char *)getName(), NULL, 0);

    XtSetArg(margs[0], XmNsubMenuId, pulldown);
    XtSetArg(margs[1], XmNmarginHeight, 0);
    for(int i = 0; i < num_args && i < 98; i++) {
	margs[2+i] = args[i];
    }

    base_widget = XmCreateOptionMenu(parent->baseWidget(), (char *)getName(),
				margs, num_args+2);
    installDestroyHandler();

    XtManageChild(base_widget);

    first_item = true;

    enableCallbackType(XmNactivateCallback);
}

/** Destructor.
 */
Choice::~Choice(void)
{
}

void Choice::destroy(void)
{
    setVisible(false);
    removeAllChoices();
    XtDestroyWidget(pulldown);
    Component::destroy();
}

/** Add another choice to the menu.
 *  @param[in] name the name of the choice.
 */
void Choice::addItem(const string &name)
{
    Widget w = XtVaCreateManagedWidget((char *)name.c_str(),
			xmPushButtonWidgetClass, pulldown,
			XmNmarginHeight, 0,
			XmNhighlightThickness, 0,
			NULL);
    XtAddCallback(w, XmNactivateCallback, Choice::activateCallback,
			(XtPointer)this);
    if(first_item) {
	first_item = false;
	XtVaSetValues(base_widget, XmNmenuHistory, w, NULL);
    }
}

/** Add a Separator.
 */
void Choice::addSeparator(void)
{
    XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, pulldown,
				XmNorientation, XmHORIZONTAL,
				NULL);
}

void Choice::activateCallback(Widget w, XtPointer clientData,
				XtPointer calldata)
{
    Choice *choice = (Choice *)clientData;

    choice->doCallbacks(w, calldata);
}

/** Get the choice.
 *  @returns the selected choice.
 */
const char * Choice::getChoice(void)
{
    Arg args[1];
    Widget w;

    XtSetArg(args[0], XmNmenuHistory, &w);
    XtGetValues(base_widget, args, 1);

    return (const char *)XtName(w);
}

/** Set the choice and optionally do callbacks.
 *  @param[in] name the name of the choice to select.
 *  @param[in] do_callbacks
 *  @param[in] ignore_case
 *  @returns true if the input name is a valid choice.
 */
bool Choice::setChoice(const string &name, bool do_callbacks, bool ignore_case)
{
    Arg args[2];
    Widget *children=NULL;
    int i, num_children;

    XtSetArg(args[0], XmNnumChildren, &num_children);
    XtSetArg(args[1], XmNchildren, &children);
    XtGetValues(pulldown, args, 2);

    if(ignore_case) {
	for(i = 0; i < num_children && (children[i]->core.being_destroyed ||
		strcasecmp(XtName(children[i]), name.c_str())); i++);
    }
    else {
	for(i = 0; i < num_children && (children[i]->core.being_destroyed ||
		strcmp(XtName(children[i]), name.c_str())); i++);
    }
    if(i < num_children) {
	XtSetArg(args[0], XmNmenuHistory, children[i]);
	XtSetValues(base_widget, args, 1);
	if(do_callbacks) {
	    XmPushButtonCallbackStruct c = {XmCR_ACTIVATE, NULL, 0};
	    doCallbacks(children[i], (XtPointer)&c);
	}
	return true;
    }
    return false;
}

/** Remove all choices.
 */
void Choice::removeAllChoices(void)
{
    Arg args[2];
    int num_children;
    Widget *children;

    XtSetArg(args[0], XmNnumChildren, &num_children);
    XtSetArg(args[1], XmNchildren, &children);
    XtGetValues(pulldown, args, 2);

    for(int i = num_children-1; i >= 0; i--) {
	XtRemoveAllCallbacks(children[i], XmNactivateCallback);
	XtDestroyWidget(children[i]);
    }
}

ParseCmd Choice::parseCmd(const string &cmd, string &msg)
{
    string s;

    if(parseArg(cmd, "value", s)) {
	if( setChoice(s, true) ) {
	    return COMMAND_PARSED;
	}
	return ARGUMENT_ERROR;
    }
    return Component::parseCmd(cmd, msg);
}

ParseVar Choice::parseVar(const string &name, string &value)
{
    if(parseCompare(name, "value")) {
	getChoice(value);
	return STRING_RETURNED;
    }
    return Component::parseVar(name, value);
}

void Choice::parseHelp(const char *prefix)
{
    Widget *children=NULL;
    int num_children;

    XtVaGetValues(pulldown, XmNnumChildren, &num_children, XmNchildren,
		&children, NULL);

    for(int i = 0; i < num_children; i++) {
	char *s = strdup(XtName(children[i]));
//	for(int j = 0; s[j] != '\0'; j++) s[j] = tolower((int)s[j]);
	printf("%s%s\n", prefix, s);
	Free(s);
    }
}
