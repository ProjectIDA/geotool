/** \file Component.cpp
 *  \brief Defines class Component.
 *  \author Ivan Henson
 */
#include "config.h"
#include "stdio.h"
#include "assert.h"
#include <iostream>
#include <algorithm>
#include "motif++/Component.h"
#include "motif++/Application.h"
#include "motif++/TopWindow.h"
#include "motif++/Warn.h"
#include "motif++/Warnings.h"
#include "motif++/InfoArea.h"
#include "motif++/RowColumn.h"

extern "C" {
#include "libstring.h"
}

static double findMin(double r, double g, double b);
static double findMax(double r, double g, double b);
static void rgbToHsv(double r, double g, double b, double *h, double *s,
			double *v);
static void hsvToRgb(double h, double s, double v, double *r, double *g,
			double *b);
static void rgbBrighten(double  *r, double  *g, double  *b, double  percent);


/** The component constructor is accessible only from a subclass.
 *  @param[in] name The name given to this Component instance. The name is the
 *		default action command string.
 *  @param[in] parent The parent of this Component.
 */
Component::Component(const string &name, Component *parent)
{
    comp_parent = parent;
    real_comp_parent = NULL;
    base_widget = NULL;
    component_name.assign(name);
    command_string.assign(name);
    redirect_warnings = false;
    if(comp_parent) comp_parent->addChild(this);
    enableCallbackType(XtNsetSensitiveCallback);
    enableCallbackType(XtNsetVisibleCallback);
}

/** The Component Destructor. The destructor is used internally, but should
 *  not be invoked elsewhere. Do not delete objects that are subclasses
 *  of Component. Use the destroy function.
 */
Component::~Component(void)
{
    if(comp_parent && (comp_parent == real_comp_parent ||
		real_comp_parent->getApplicationInstance())) {
	comp_parent->removeChild(this);
    }
    if(base_widget) {
	cerr <<"Deleting a subclass of Component. Use destroy() instead."<<endl;
    }
    if((int)comp_children.size() > 0) {
//printf("deleting Component with children\n");
    }
    for(int i = 0; i < (int)listeners.elements.size(); i++) {
	delete listeners.elements[i]->second;
    }
    listeners.clear();

    for(int i = 0; i < (int)scripts.elements.size(); i++) {
	vector<ScriptCallback> *v = scripts.elements[i]->second;
	for(int j = 0; j < (int)v->size(); j++) {
	    v->at(j).parse_file.clear();
	}
	delete v;
    }
}

void Component::widgetDestroyedCB(Widget, XtPointer clientData, XtPointer)
{
    Component *o = (Component *)clientData;
    o->base_widget = NULL;
    delete o;
}

/** Delete the Component. Call this function to delete an object that is a
 * subclass of Component, instead of deleting the object directly. The base
 * widget of the Component is destroyed and the Component's destructor is
 * called. It is not necessary to delete or destroy the children of a Component
 * that is being destroyed. The children will be destroyed automatically by
 * the Xt library.
 */
void Component::destroy(void)
{
    XtDestroyWidget(base_widget);
//    if(base_widget) base_widget = NULL;
}

/** Register destroy widget callbacks with the Xt library. This function is
 *  called by subclasses that create Motif widgets. It is used only within
 *  the motif++ library.
 */
void Component::installDestroyHandler(void)
{
    assert(base_widget != NULL);
    XtRemoveCallback(base_widget, XmNdestroyCallback,
		Component::widgetDestroyedCB, (XtPointer)this);
    XtAddCallback(base_widget, XmNdestroyCallback,
                Component::widgetDestroyedCB, (XtPointer)this);
    findRealParent();
}

void Component::findRealParent(void)
{
    Widget w;
    Component *comp = this;
    for(comp = comp->getParent(); comp != NULL; comp = comp->getParent()) {
	for(w = XtParent(base_widget); w && w != comp->base_widget;
		w = XtParent(w));
	if(w == comp->base_widget) {
	    real_comp_parent = comp;
	    return;
	}
    }
}

void Component::addChild(Component *comp) throw(int)
{
    comp_children.push_back(comp);
}

void Component::removeChild(Component *comp)
{
    vector<Component *>::iterator child =
		find(comp_children.begin(), comp_children.end(), comp);

    if( child != comp_children.end() ) {
	comp_children.erase(child);
    }
}

/** Get the Component children of this Component.
 * @returns a vector with the children.
 */
vector<Component *> * Component::getChildren(void)
{
    return new vector<Component *>(comp_children);
}

/** Position the last child added to a Component. Used only by subclasses in
 * libmotif++.
 * @param[in] new_position
 */
void Component::positionLastChild(int new_position)
{
    if((int)comp_children.size() > 1 && new_position >= 0 && 
		new_position < (int)comp_children.size()-1)
    {
	Component *comp = comp_children.back();
	comp_children.insert(comp_children.begin()+new_position, comp);
	comp_children.pop_back();
    }
}

/** Find a Component Button descendent. Search the Component tree, with this
 *  Component at the top, for the Component Button instance.
 *  @param[in] name the instance name of the Button.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the Button with the specified name, or returns NULL if no
 *	Button is found with the specified name.
 */
Button * Component::findButton(const string &name, bool star)
{
    char part[200], tmp[200];
    Button *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getButtonInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findButton(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findButton(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component Toggle descendent. Search the Component tree, with this
 *  Component at the top, for the Component Toggle instance.
 *  @param[in] name the instance name of the Toggle.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the Toggle with the specified name, or returns NULL if no
 *	Toggle is found with the specified name.
 */
Toggle * Component::findToggle(const string &name, bool star)
{
    char part[200], tmp[200];
    Toggle *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getToggleInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findToggle(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findToggle(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component TextField descendent. Search the Component tree, with
 *  this Component at the top, for the Component TextField instance.
 *  @param[in] name the instance name of the TextField.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the TextField with the specified name, or returns NULL if no
 *	TextField is found with the specified name.
 */
TextField * Component::findTextField(const string &name, bool star)
{
    char part[200], tmp[200];
    TextField *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getTextFieldInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findTextField(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findTextField(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component Menu descendent. Search the Component tree, with this
 *  Component at the top, for the Component Menu instance.
 *  @param[in] name the instance name of the Menu.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the Menu with the specified name, or returns NULL if no
 *	Menu is found with the specified name.
 */
Menu * Component::findMenu(const string &name, bool star)
{
    char part[200], tmp[200];
    Menu *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getMenuInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findMenu(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findMenu(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component Label descendent. Search the Component tree, with this
 *  Component at the top, for the Component Label instance.
 *  @param[in] name the instance name of the Label.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the Label with the specified name, or returns NULL if no
 *	Label is found with the specified name.
 */
Label * Component::findLabel(const string &name, bool star)
{
    char part[200], tmp[200];
    Label *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getLabelInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findLabel(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findLabel(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component RowColumn descendent. Search the Component tree, with
 *  this Component at the top, for the Component RowColumn instance.
 *  @param[in] name the instance name of the RowColumn.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the RowColumn with the specified name, or returns NULL if no
 *	RowColumn is found with the specified name.
 */
RowColumn * Component::findRowColumn(const string &name, bool star)
{
    char part[200], tmp[200];
    RowColumn *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getRowColumnInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findRowColumn(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findRowColumn(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component InfoArea descendent. Search the Component tree, with
 *  this Component at the top, for the Component InfoArea instance.
 *  @param[in] name the instance name of the InfoArea.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the InfoArea with the specified name, or returns NULL if no
 *	InfoArea is found with the specified name.
 */
InfoArea * Component::findInfoArea(const string &name, bool star)
{
    char part[200], tmp[200];
    InfoArea *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getInfoAreaInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findInfoArea(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findInfoArea(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}
/** Find a Component Table descendent. Search the Component tree, with
 *  this Component at the top, for the Component Table instance.
 *  @param[in] name the instance name of the Table.
 *  @param[in] star true if the component parent name is '*'
 *  @returns the Table with the specified name, or returns NULL if no
 *	Table is found with the specified name.
 */
Table * Component::findTable(const string &name, bool star)
{
    char part[200], tmp[200];
    Table *comp;
    int n;

    snprintf(tmp, sizeof(tmp), "%s", name.c_str());
    // take care of special case of a name ending with "..."
    for(int i = 0; tmp[i] != '\0'; i++) {
	if(tmp[i] == '.' && tmp[i+1] == '.' && tmp[i+2] == '.') {
	    tmp[i] = tmp[i+1] = tmp[i+2] = '#';
	}
    }
    for(n=0; n<200 && tmp[n] != '\0' && tmp[n] != '.' && tmp[n] != '*'; n++) {
	part[n] = name[n];
    }
    part[n] = '\0';

    if(!strcmp(part, component_name.c_str())) {
	if(tmp[n] == '.') star = false;
	else if(name[n] == '*') star = true;
	else return getTableInstance();
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findTable(name.c_str()+n+1, star);
	    if(comp) return comp;
	}
    }
    else if(star) {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    comp = comp_children[i]->findTable(name);
	    if(comp) return comp;
	}
    }
    return NULL;
}

/** Display or hide the Component. Callbacks will be made to all
 *  XtNsetVisibleCallback listeners. This virtual function can be reimplemented
 *  in subclasses that need to take actions before they are made visible or
 *  after they are hidden.
 *  @param[in] visible True will make the component visible (Managed, Mapped,
 *  and Raised). False will hide the Component.
 */
void Component::setVisible(bool visible)
{
    assert(base_widget != NULL);
    assert(XtHasCallbacks(base_widget,XmNdestroyCallback) == XtCallbackHasSome);

    if(visible) {
	if(!XtIsManaged(base_widget)) {
	    XtManageChild(base_widget);
	    doCallbacks(base_widget, (XtPointer)visible,
		(const char *)XtNsetVisibleCallback);
	}
    }
    else {
	if(XtIsManaged(base_widget)) {
	    XtUnmanageChild(base_widget);
	    doCallbacks(base_widget, (XtPointer)visible,
		(const char *)XtNsetVisibleCallback);
	}
    }
}

/** Returns true if the Component is visible (Managed). This function returns
 *  true if the Component has been managed and mapped to the screen, even if
 *  it is actually hidden by other windows. A call to setVisible(true) will
 *  insure that a Component is raised to the top of a stack of windows.
 */
bool Component::isVisible(void)
{
    return XtIsManaged(base_widget);
}


/** Set the size of a component.
 *  @param[in] width the width in pixels.
 *  @param[in] height the height in pixels.
 */
void Component::setSize(int width, int height)
{
    assert(base_widget != NULL);

    XtVaSetValues(base_widget, XtNwidth, width, XtNheight, height, NULL);
}

/** Get the size of a component.
 *  @param[out] width the width in pixels.
 *  @param[out] height the height in pixels.
 */
void Component::getSize(int *width, int *height)
{
    assert(base_widget != NULL);
    Arg args[2];
    Dimension w, h;

    XtSetArg(args[0], XmNwidth, &w);
    XtSetArg(args[1], XmNheight, &h);
    XtGetValues(base_widget, args, 2);
    *width = (int)w;
    *height = (int)h;
}

/** Get the character string from an XmString.
 *  @param[in] xm an XmString object.
 *  @returns pointer to the character string. Free this pointer when it is no
 *  longer needed.
 */
char * Component::getXmString(XmString xm)
{
    char *text;

    if(XmStringGetLtoR(xm, XmFONTLIST_DEFAULT_TAG, &text)) {
	return text;
    }
    else {
	return NULL;
    }
}

/** Create an XmString object.
 *  @param[in] s the character string.
 *  @returns an XmString object. Free this object with XmStringFree when it is
 *  no longer needed.
 */
XmString Component::createXmString(const string &s)
{
    return XmStringCreateLtoR((char *)s.c_str(), XmFONTLIST_DEFAULT_TAG);
}

/** Get the horizontal position of a component.
 *  @returns the horizontal position in pixels.
 */
int Component::getX(void)
{
    Arg args[1];
    Position x = 0;

    XtSetArg(args[0], XmNx, &x);
    XtGetValues(base_widget, args, 1);
    return (int)x;
}

/** Get the vertical position of a component.
 *  @returns the vertical position in pixels.
 */
int Component::getY(void)
{
    Arg args[1];
    Position y = 0;

    XtSetArg(args[0], XmNy, &y);
    XtGetValues(base_widget, args, 1);
    return (int)y;
}

/** Get the width of a component.
 *  @returns the width in pixels.
 */
int Component::getWidth(void)
{
    Arg args[1];
    Dimension width = 0;

    XtSetArg(args[0], XmNwidth, &width);
    XtGetValues(base_widget, args, 1);
    return (int)width;
}

/** Set the width of a component.
 *  @param[in] width the width in pixels.
 */
void Component::setWidth(int width)
{
    Arg args[1];
    XtSetArg(args[0], XmNwidth, width);
    XtSetValues(base_widget, args, 1);
}

/** Get the height of a component.
 *  @returns the height in pixels.
 */
int Component::getHeight(void)
{
    Arg args[1];
    Dimension height = 0;

    XtSetArg(args[0], XmNheight, &height);
    XtGetValues(base_widget, args, 1);
    return (int)height;
}

/** Set the height of a component.
 *  @param[in] height the width in pixels.
 */
void Component::setHeight(int height)
{
    Arg args[1];
    XtSetArg(args[0], XmNheight, height);
    XtSetValues(base_widget, args, 1);
}

/** Enable action callbacks of the specified type. A subclass of
 *  Component must enable callback types (once only) before it's
 *  addActionListener function is called and before it's doCallbacks
 *  function is called with the specified action type.
 *  @param[in] action_type
 */
void Component::enableCallbackType(const string &action_type)
{
    int q_action_type = stringToQuark(action_type.c_str());
    int i;
    for(i = 0; i < (int)callback_types.size(); i++) {
	if(callback_types[i] == q_action_type) break;
    }
    if(i == (int)callback_types.size()) {
	callback_types.push_back(q_action_type);
    }
}


/** Add an ActionListener that listens for a specific action_type. The
 *  actionPerformed function of the listener object will be called whenever
 *  an action_type action (event) occurs on this Component.
 *  @param[in] listener the ActionCommand listener.
 *  @param[in] action_type the action callback type. It must be a callback
 *	generated by this Component class.
 */
void Component::addActionListener(ActionListener *listener,
		const string &action_type)
{
    int i;
    vector<ActionListener *> *v;

    if( !listener ) return;

    int q_action_type = stringToQuark(action_type.c_str());

    for(i = 0; i < (int)callback_types.size(); i++) {
	if(callback_types[i] == q_action_type) break;
    }
    if(i == (int)callback_types.size()) {
        cerr << "addActionListener: invalid listener action_type: "
		<< action_type << endl;
    }

    if( !listeners.get(action_type, &v) ) {
        v = new vector<ActionListener *>;
        listeners.put(action_type, v);
    }
    for(i = 0; i < (int)v->size(); i++) {
	if(v->at(i) == listener) break;
    }
    if(i == (int)v->size()) {
        v->push_back(listener);
    }
}

/** Remove a Component action listener.
 *  @param[in] listener the Component listener.
 *  @param[in] action_type the action callback type. It must be a callback
 *	generated by this Component subclass.
 */
void Component::removeActionListener(ActionListener *listener,
			const string &action_type)
{
    vector<ActionListener *> *v;

    if( listeners.get(action_type, &v) ) {
	for(int i = 0; i < (int)v->size(); i++) {
	    if(v->at(i) == listener) {
		v->erase(v->begin()+i);
		break;
	    }
	}
    }
}

/** Remove all Component listeners for the specified action type.
 *  @param[in] action_type the action callback type. It must be a callback
 *	generated by this Component subclass.
 */
void Component::removeAllListeners(const string &action_type)
{
    vector<ActionListener*> *v;

    if( listeners.get(action_type, &v) ) {
        v->clear();
    }
}

/** Get a list of all Component listeners for the specified action type.
 *  @param[in] action_type the action callback type. It must be a callback
 *	generated by this Component subclass.
 *  @returns a vector containing all listeners for the specified action
 *	type. The vector can be emtpy. The vector will be NULL if the
 *	action_type is not valid for this Component.
 */
vector<ActionListener*> * Component::getActionListeners(const string &action_type)
{
    vector<ActionListener*> *v;

    if( listeners.get(action_type, &v) ) {
	return v;
    }
    else {
	cerr << "getActionListener: invalid listener type: " << action_type
		<< endl;
	return NULL;
    }
}

/** Add a script for an action type. The script will be executed whenever
 *  an action_type action (event) occurs on this Component.
 *  @param[in] s a script.
 *  @param[in] action_type the action callback type. It must be a callback
 *	generated by this Component class.
 */
void Component::addScriptCallback(const string &script,
			const string &action_type)
{
    int i, n;
    const char *s;
    int q_action_type = stringToQuark(action_type.c_str());
    vector<ScriptCallback> *v;

    for(i = 0; i < (int)callback_types.size(); i++) {
	if(callback_types[i] == q_action_type) break;
    }
    if(i == (int)callback_types.size()) {
        cerr << "addScriptCallback: invalid action_type: "
		<< action_type << endl;
    }

    if( !scripts.get(action_type, &v) ) {
        v = new vector<ScriptCallback>;
        scripts.put(action_type, v);
    }
    n = (int)script.length();
    s = script.c_str();
    if((*s == '"' && s[n-1] == '"') || (*s == '\'' && s[n-1] == '\'') ||
	(*s == '`' && s[n-1] == '`') || (*s == '{' && s[n-1] == '}'))
    {
	s++;
	n -= 2;
    }
    for(i = 0; i < (int)v->size(); i++) {
	if(!strncmp(v->at(i).script.c_str(), s, n)) break;
    }
    if(i == (int)v->size()) {
	Application *app = Application::getApplication();
	ScriptCallback sc;
	sc.script.assign(s, n);
	for(i = 0; i < (int)app->parse_file.size(); i++) {
	    sc.parse_file.push_back(app->parse_file[i]);
	}
        v->push_back(sc);
    }
}

/** Do action callbacks. The callbacks for the specified action_type are
 *  called for all registered listeners. The input arguments, w, calldata and
 *  action_type are recorded in an ActionEvent class object that is delivered
 *  to all Component listeners via their actionPerformed function.
 * @param[in] source the Widget source of the action.
 * @param[in] callback_data the callback data.
 * @param[in] action_type the type of action event.
 */
void Component::doCallbacks(Widget source, XtPointer callback_data,
			const string &action_type)
{
    vector<ActionListener *> *v;
    vector<ScriptCallback> *s;

    if( listeners.get(action_type, &v) ) {
	for(int i = 0; i < (int)v->size(); i++) {
	    ActionListener *comp = v->at(i);
	    ActionEvent *a =
		new ActionEvent(this, source, command_string, callback_data,
				action_type);
	    comp->actionPerformed(a);
	    a->deleteObject();
	}
    }

    if( scripts.get(action_type, &s) ) {
	string msg;
	Application *app = Application::getApplication();
	for(int i = 0; i < (int)s->size(); i++) {
	    msg.clear();
	    app->parseCallback(s->at(i).script.c_str(), s->at(i).parse_file,
			msg);
	    if(!msg.empty()) cout << msg << endl;
	}
    }
}

/** Do action callbacks. The callbacks for the specified action_type are
 *  called for all registered listeners. The input arguments, component,
 *  calldata and action_type are recorded in an ActionEvent class object that
 *  is delivered to all Component listeners via their actionPerformed function.
 * @param[in] source the Component source of the action.
 * @param[in] callback_data the callback data.
 * @param[in] action_type the type of action event.
 */
void Component::doCallbacks(Component *source, XtPointer callback_data,
		const string &action_type)
{
    vector<ActionListener *> *v;
    vector<ScriptCallback> *s;

    if( listeners.get(action_type, &v) ) {
	for(int i = 0; i < (int)v->size(); i++) {
	    ActionListener *comp = v->at(i);
	    ActionEvent *a =
		new ActionEvent(source, source->baseWidget(),
				command_string, callback_data, action_type);
	    comp->actionPerformed(a);
	    a->deleteObject();
	}
    }

    if( scripts.get(action_type, &s) ) {
	string msg;
	Application *app = Application::getApplication();
	for(int i = 0; i < (int)s->size(); i++) {
	    msg.clear();
	    app->parseCallback(s->at(i).script.c_str(), s->at(i).parse_file,
			msg);
	    if(!msg.empty()) cout << msg << endl;
	}
    }
}

/** Set the Component's sensitivity. An insensitive Component is
 *  visible, but it will not respond to user input.
 *  @param[in] sensitive
 */
void Component::setSensitive(bool sensitive)
{
    if(sensitive != XtIsSensitive(base_widget)) {
	XtSetSensitive(base_widget, sensitive);
	doCallbacks(base_widget, (XtPointer)sensitive,
	    (const char *)XtNsetSensitiveCallback);
    }
}

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef __STDC__
/** Display a warning message in a popup window. The message is displayed in
 *  the Warn window for the Component's TopWindow and it is also appended to
 *  the Application Warnings window.
 *  @param[in] format a form suitable for printf.
 *  @param[in] ... variable length argument list suitable for printf.
 */
void Component::showWarning(const char *format, ...) throw(int)
#else
/** Display a warning message in a popup window. The message is displayed in
 *  the Warn window for the Component's TopWindow and it is also appended to
 *  the Application Warnings window. This functions takes a variable length
 *  argument list suitable for printf. The first argument is the format.
 */
void Component::showWarning(va_alist) va_dcl
#endif
{
    va_list	va;
    int		n, len;
    char	*warning = NULL;

#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    if(format == NULL || (n = (int)strlen(format)) <= 0) return;

    Application *app = Application::getApplication();

    if(app->warnings == NULL) {
	if((int)app->getWindows()->size() > 0) {
            app->warnings = new Warnings(app->getWindows()->front());
        }
        else {
            app->warnings = new Warnings(app);
        }
    }
    Warnings *warnings = app->warnings;
    len = n+5000;
    warning = (char *)malloc(len);
    if( !warning ) {
	GError::setMessage("Component.showWarning: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    if(warnings->warn_no == 1) {
	snprintf(warning, len, "%d: ", warnings->warn_no++);
    }
    else {
	snprintf(warning, len, "\n%d: ", warnings->warn_no++);
    }
    n = (int)strlen(warning);
    len -= n;

    vsnprintf(warning+n, len, format, va);
    va_end(va);
    for(int i = (int)strlen(warning)-1; i >= 0 && warning[i] == '\n'; i--) {
	warning[i] = '\0';
    }

    //  find the TopWindow (only one warn window per TopWindow)
    TopWindow *top = topWindowParent();

    if(!top || app->redirect_warnings || redirect_warnings) {
	if(warning[0] == '\n') cerr << warning+1 << endl;
	else cerr << warning << endl;
    }
    else if(top->warn != NULL) {
	/* if a warning is already displayed, save this warning, turn on the
	 * more_warnings button.
	 */
	top->warn->more_warnings->setSensitive(true);
        warnings->append(warning);
    }
    else {
	top->warn = new Warn(warning, top);
	XtAddCallback(top->warn->close->baseWidget(), XmNactivateCallback,
		TopWindow::closeWarnCallback, (XtPointer)top);
	XtAddCallback(top->warn->more_warnings->baseWidget(),
		XmNactivateCallback, TopWindow::warnCallback, (XtPointer)top);
	XtAddCallback(top->warn->redirect->baseWidget(),XmNvalueChangedCallback,
		TopWindow::warnCallback, (XtPointer)top);
	top->warn->setVisible(true, CENTER_DIALOG);

        /* append warning to the list in *_Warnings
         */
        warnings->append(warning);
    }
    free(warning);
}

#ifdef __STDC__
/** Save a warning message. A small warning button labelled "!" will be
 *  displayed in the lower right corner of the TopWindow parent of this
 *  component. Activation of the button will display the warning message in
 *  a popup window.
 *  @param[in] format a form suitable for printf.
 *  @param[in] ... variable length argument list suitable for printf.
 */
void Component::putWarning(const char *format, ...) throw(int)
#else
/** Save a warning message. A small warning button labelled "!" will be
 *  displayed in the lower right corner of the TopWindow parent of this
 *  component. Activation of the button will display the warning message in
 *  a popup window.  This functions takes a variable length argument list
 *  suitable for printf. The first argument is the format.
 */
void Component::putWarning(va_alist) va_dcl
#endif
{
    va_list	va;
    int		n, len;
    char	*warning = NULL;

#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    if(format == NULL || (n = (int)strlen(format)) <= 0) return;

    Application *app = Application::getApplication();

    if(app->warnings == NULL) {
	if((int)app->getWindows()->size() > 0) {
            app->warnings = new Warnings(app->getWindows()->front());
        }
        else {
            app->warnings = new Warnings(app);
        }
    }
    Warnings *warnings = app->warnings;
    len = n+5000;
    warning = (char *)malloc(len);
    if( !warning ) {
	GError::setMessage("Component.putWarning: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    if(warnings->warn_no == 1) {
	snprintf(warning, len, "%d: ", warnings->warn_no++);
    }
    else {
	snprintf(warning, len, "\n%d: ", warnings->warn_no++);
    }
    n = (int)strlen(warning);
    len -= n;

    vsnprintf(warning+n, len, format, va);
    va_end(va);

    /* Save this warning.
     */
    for(int i = (int)strlen(warning)-1; i >= 0 && warning[i] == '\n'; i--) {
	warning[i] = '\0';
    }
    warnings->append(warning);

    free(warning);

    // find the TopWindow (only one warn window per TopWindow)
    // Turn on the warning button in the InfoArea.
    TopWindow *top = topWindowParent();

    if(top) {
	InfoArea *info = top->findInfoArea("infoArea");
	RowColumn *rc;
	if( info )
	{
	    info->warningButtonOn();
	}
	else if( (rc = top->findRowColumn("controls")) )
	{
	    rc->warningButtonOn();
	}
    }
}

#ifdef __STDC__
/** Print a message to the log window. Messages are appended to the log. Use
 *  Application:: displayLog() to display the log window.
 *  @param[in] format a form suitable for printf.
 *  @param[in] ... variable length argument list suitable for printf.
 */
void Component::printLog(const char *format, ...) throw(int)
#else
/** Print a message to a log window. Messages are appended toe the log. Use
 *  Application::displayLog() to display the log window.
 *  This functions takes a variable length argument list suitable for printf.
 *  The first argument is the format.
 */
void Component::printLog(va_alist) va_dcl
#endif
{
    va_list	va;
    int		n, len;
    char	*msg = NULL;

#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    if(format == NULL || (n = (int)strlen(format)) <= 0) return;

    len = n+5000;
    msg = (char *)malloc(len);
    if( !msg ) {
	GError::setMessage("Component.printLog: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }

    vsnprintf(msg, len, format, va);
    va_end(va);

    Application *app = Application::getApplication();
    app->logWindow()->append(msg);

    free(msg);
}

void Component::closeWarnCallback(Widget w, XtPointer client, XtPointer data)
{
    TopWindow *top = (TopWindow *)client;
    top->warn = NULL;
}

void Component::warnCallback(Widget w, XtPointer clientData, XtPointer calldata)
{
    TopWindow *top = (TopWindow *)clientData;

    if(!top->warn) return;

    Application *app = Application::getApplication();

    if(w == top->warn->more_warnings->baseWidget())
    {
	app->warnings->setVisible(true, CENTER_DIALOG);
    }
    else if(w == top->warn->redirect->baseWidget())
    {
	app->redirect_warnings = XmToggleButtonGetState(w);
	XmToggleButtonSetState(app->warnings->redirect,
		app->redirect_warnings, false);
    }
}

/** Get the TopWindow ancester of this Component.
 *  @returns the TopWindow ancester, or returns NULL if the Component
 *	does not have a TopWindow ancester.
 *  @sa TopWindow
 */
TopWindow * Component::topWindowParent(void)
{
    Component *comp = this;
    while(comp && !comp->getTopWindowInstance()) {
	if(!comp->getParent()) break;
	comp = comp->getParent();
    }
    if(comp && comp->getTopWindowInstance()) {
	return comp->getTopWindowInstance();
    }
    else {
	Application *app = Application::getApplication();
	return (TopWindow *)app->getWindows()->at(0);
    }
}

/** Get the FormDialog ancester of this Component.
 *  @returns the FormDialog ancester, or returns NULL if the Component
 *	does not have a FormDialog ancester.
 *  @sa FormDialog
 */
FormDialog * Component::formDialogParent(void)
{
    Component *comp = this;
    while(comp && !comp->getFormDialogInstance()) {
	if(!comp->getParent()) break;
	comp = comp->getParent();
    }
    return ( comp ) ? comp->getFormDialogInstance() : NULL;
}

/** Get the WMShell widget ancester of this Component.
 *  @returns the WMShell widget ancester, or returns NULL if the Component
 * 	does not have a WmShell ancester.
 */
Widget Component::shellWidget(void)
{
    Widget widget = base_widget;

    while(widget && !XtIsWMShell(widget)) {
	widget = XtParent(widget);
    }
    return widget;
}

/** Install accelerators for a Component. Accelerators are installed as
 *  specified by program properties. This function is used only within
 *  libmotif++.
 */
void Component::installAccelerators(void)
{
    char *prop, prop_name[200];
    Component *p;

    // the component (this) is a Button or Toggle.
    for(p = getParent(); p != NULL; p = p->getParent()) {
	if(p->getFormDialogInstance() || p->getFrameInstance()) break;
    }
    if(!p) return;

    // look for properties like:
    // Arrivals:Add:accelerator=Ctrl+a#override :Ctrl<Key>a: ArmAndActivate()
    snprintf(prop_name, sizeof(prop_name), "%s:%s:accelerator",
		p->getName(), getName());
    if( (prop = getProperty(prop_name)) ) {
	char *c = strstr(prop, "#");
	if(c) {
	    Arg args[2];
	    XmString xm = NULL;
	    XtAccelerators a = XtParseAcceleratorTable(c);
	    int n = 0;
	    XtSetArg(args[n], XmNaccelerators, a); n++;
	    if(c > prop) {
		*c = '\0';
		xm = createXmString(prop);
		XtSetArg(args[n], XmNacceleratorText, xm); n++;
	    }
	    XtSetValues(base_widget, args, 2);
	    if(xm) XmStringFree(xm);
	}
	free(prop);
    }

    // Check for accelerator destinations other than this FormDialog.

    // Look for properties like:
    // Arrivals:Add:destinations=geotool,...
    snprintf(prop_name, sizeof(prop_name), "%s:%s:destinations",
		p->getName(), getName());
    if( (prop = getProperty(prop_name)) ) {
	Component *comp;
	char *c, *tok, *last;
	tok = prop;
	while( (c = strtok_r(tok, ",", &last)) ) {
	    tok = NULL;
	    if( (comp = Application::getApplication()->getWindow(c)) ) {
		comp->installAccelerators(this);
	    }
	}
	Free(prop);
    }
}

/** Install accelerators for a Component. Accelerators are installed as
 *  specified by program properties. This function is used only within
 *  libmotif++.
 */
void Component::installAccelerators(Component *comp)
{
    installAccelerators(comp->baseWidget());
}

/** Install accelerators for a Component. Accelerators are installed as
 *  specified by program properties. This function is used only within
 *  libmotif++.
 */
void Component::installAccelerators(Widget source)
{
    installAccelerators(base_widget, source);
}

/** Install accelerators for a Component. Accelerators are installed as
 *  specified by program properties. This function is used only within
 *  libmotif++.
 */
void Component::installAccelerators(Widget destination, Widget source)
{
    Widget *w;
    unsigned char type;
    int i, num;
    Arg args[2];

    // XtInstallAllAccelerators does not like Gadgets. Gives seg fault.
    if(XmIsGadget(destination)) return;

    if(XmIsRowColumn(destination)) {
	XtSetArg(args[0], XmNrowColumnType, &type);
	XtGetValues(destination, args, 1);
	if(type != XmWORK_AREA) return;
    }

    XtInstallAllAccelerators(destination, source);

    num = 0;
    XtSetArg(args[0], XmNnumChildren, &num);
    XtSetArg(args[1], XmNchildren, &w);
    XtGetValues(destination, args, 2);

    for(i = 0; i < num; i++) {
	installAccelerators(w[i], source);
    }
}

/** Get the Pixel value for a color.
 *  @param[in] color_name the name of the color.
 *  @returns the Pixel value.
 */
Pixel Component::stringToPixel(const string &color_name)
{
    Widget w = base_widget;
    XColor c;
    char *last, *t;
    int r, g, b;
    
    char *s = strdup(color_name.c_str());

    stringTrim(s);

    if(isdigit(s[0]) &&
	(t = strtok_r(s, " ,.:;\t()[]{}/", &last)) && stringToInt(t, &r) &&
	(t = strtok_r(NULL, " ,.:;\t()[]{}/", &last)) && stringToInt(t, &g) &&
	(t = strtok_r(NULL, " ,.:;\t()[]{}/", &last)) && stringToInt(t, &b))
    {
	Free(s);
	c.red = r*256;
	c.green = g*256;
	c.blue = b*256;
	c.flags = DoRed | DoGreen | DoBlue;
	if(XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
		DefaultScreen(XtDisplay(w))), &c))
	{
	    return c.pixel;
	}
    }
    else {
	XrmValue from, to;
	Free(s);
	to.addr = NULL;
	to.size = 0;
	from.addr = (char *)color_name.c_str();
	from.size = (int)color_name.length() + 1;

	if(XtConvertAndStore(w, XtRString, &from, XtRPixel, &to) && to.addr)
	{
	    return( *(Pixel *) to.addr );
	}
    }

    return(BlackPixelOfScreen(XtScreen(w)));
}

/** Brighten a Pixel. Brighten the input pixel by the specified percentage.
 *  @param[in] pixel the pixel to brighten.
 *  @param[in] percent the percentage to brighten.
 *  @returns a new Pixel that is brighter than the input Pixel.
 */
Pixel Component::pixelBrighten(Pixel pixel, double percent)
{
    Widget w = base_widget;
    XColor color;
    double r, g, b;

    color.pixel = pixel;
    XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
		DefaultScreen(XtDisplay(w))), &color);

    r = color.red;
    g = color.green;
    b = color.blue;
    rgbBrighten(&r, &g, &b, percent);
    color.red = (unsigned short)r;
    color.green = (unsigned short)g;
    color.blue = (unsigned short)b;

    XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
		DefaultScreen(XtDisplay(w))), &color);

    return color.pixel;
}

static void
rgbToHsv(double r, double g, double b, double *h, double *s, double *v)
{
    double min, max;

    min = findMin(r, g, b);
    max = findMax(r, g, b);

    /* set val to the maximum */
    *v = max;

    /* if max is 0, we have black */
    if (max == 0) {
	*s = 0;
	*h = 0;
    }
    else
    {
	*s = ((max-min)/max)*255.;
	if (r == max) {
	    *h = 60 * ((g - b)/(max - min));
	}
	else if (g == max) {
	    *h = 60 * (2 + (b - r)/(max - min));
	}
	else if (b == max) {
		*h = 60 * (4 + (r - g)/(max - min));
	}

	if (*h < 0) {
		*h = *h + 360;
	}
    }
}
	
static double
findMin(double r, double g, double b)
{
    double min;

    if (r < g)
    {
	if (r < b) {
	    min = r;
	}
	else {
	    min = b;
	}
    }
    else 
    {
	if (g < b) {
	    min = g;
	}
	else {
	    min = b;
	}
    }
    return min;
}

static double
findMax(double r, double g, double b)
{
    double max;

    if (r > g)
    {
	if (r > b) {
	    max = r;
	}
	else
	{
	    max = b;
	}
    }
    else 
    {
	if (g > b) {
	    max = g;
	}
	else
	{
	    max = b;
	}
    }
    return max;
}

static void
hsvToRgb(double h, double s, double v, double *r, double *g, double *b)
{
    int		maxc;
    double	vals[3];
    double	min, thu;

    if (h > 300) {
	h = h - 360;
    }

    if (h < 60) {
	maxc = 0;
    }
    else if (h > 180) {
	maxc = 2;
    }
    else {
	maxc = 1;
    }

    vals[maxc] = v;

    min = (v*(255. - s)) / 255.;

    thu = ((h/60) - (2*maxc)) * (v - min);

    if (thu > 0.0) {
	vals[(maxc+2)%3] = min;
	vals[(maxc+1)%3] = min + thu;
    }
    else {
	vals[(maxc+1)%3] = min;
	vals[(maxc+2)%3] = min - thu;
    }

    *r = vals[0];
    *g = vals[1];
    *b = vals[2];
}

static void
rgbBrighten(double  *r, double  *g, double  *b, double  percent)
{
    double h=0., s=0., v=0.;
    double red, green, blue;

    red = *r;
    green = *g;
    blue = *b;

    rgbToHsv(red, green, blue, &h, &s, &v);
    v = v * percent;
    hsvToRgb(h, s, v, &red, &green, &blue);

    *r = red;
    *g = green;
    *b = blue;
}

/** Set the cursor type.This function will set the cursor
 *  type in all FormDialog instances. Available cursor types are:
 *      - "hourglass"
 *      - "hand"
 *      -  "move"
 *      - "crosshair"
 *      - "resize up"
 *      - "resize down"
 *      - "resize right"
 *      - "resize left"
 *      - "arrow"
 *  @param[in] cursor_type
 */
void Component::setCursor(const string &cursor_type)
{
    Application::getApplication()->setCursor(cursor_type);
}

ParseCmd Component::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string s;
    int i, n, len;
    const char *c, *action_type;

    if(parseCompare(cmd, "show")) {
	setVisible(true);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "hide")) {
	setVisible(false);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "setSensitive")) {
	setSensitive(true);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "setInsensitive")) {
	setSensitive(false);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "create")) {
	msg.assign("create: missing arguments.");
	return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "create", s)) {
	return Application::getApplication()->create(this, s, msg);
    }
    else if(parseCompare(cmd, "listChildren")) {
	for(i = 0; i < (int)comp_children.size(); i++)
	{
	    printf("%s\n", comp_children[i]->component_name.c_str());
	}
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "listCallbacks")) {
	for(i = 0; i < (int)callback_types.size(); i++) {
	    action_type = quarkToString(callback_types[i]);
	    printf("%s\n", action_type);
	}
	return COMMAND_PARSED;
    }

    for(i = 0; i < (int)callback_types.size(); i++) {
	action_type = quarkToString(callback_types[i]);
	if(parseString(cmd, action_type, s)) {
	    addScriptCallback(s, action_type);
	    return COMMAND_PARSED;
	}
    }

    len = (int)cmd.length();
    for(n = 0; n < len && cmd[n] != '.'; n++);
    c = cmd.c_str()+n+1;
    if(n == len || *c == '\0') return COMMAND_NOT_FOUND;

    for(i = 0; i < (int)comp_children.size(); i++)
    {
	if(sameName(cmd, comp_children[i]->component_name.c_str(), n) )
	{
	    ret = comp_children[i]->parseCmd(c, msg);
	    if( ret != COMMAND_NOT_FOUND ) {
		return ret;
	    }
	}
    }
/*
    for(i = 0; i < (int)comp_children.size(); i++)
    {
	ret = comp_children[i]->parseCmd(cmd, msg, msg_len);
	if( ret != COMMAND_NOT_FOUND ) {
	    return ret;
	}
    }
*/
    return COMMAND_NOT_FOUND;
}

ParseVar Component::parseVar(const string &name, string &value)
{
    ParseVar ret;
    int i, n, len;
    const char *s;

    len = (int)name.length();
    for(n = 0; n < len && name[n] != '.'; n++);
    s = name.c_str()+n+1;
    if(n == len || *s == '\0') return VARIABLE_NOT_FOUND;

    for(i = 0; i < (int)comp_children.size(); i++)
    {
	if(sameName(name, comp_children[i]->component_name, n) )
	{
	    ret = comp_children[i]->parseVar(s, value);
	    if( ret != VARIABLE_NOT_FOUND ) {
		return ret;
	    }
	}
    }
    return VARIABLE_NOT_FOUND;
}

void Component::putProperty(const string &name, const string &value,
			bool permanent, Component *comp)
{
    Application *app = Application::getApplication();
    string s(value);
    if(permanent) {
	app->properties.put(name, &s);
    }
    else {
	app->tmp_properties.put(name, &s);
    }
    if(comp) {
	app->doCallbacks(comp, (XtPointer)name.c_str(), XtNchangePropertyCallback);
    }
}

void Component::removeProperty(const string &name)
{
    Application *app = Application::getApplication();
    app->properties.remove(name);
    app->tmp_properties.remove(name);
}

char * Component::getProperty(const string &name)
{
    Application *app = Application::getApplication();
    string *s;
    if( !app->tmp_properties.get(name, &s) ) {
	if( !app->properties.get(name, &s) ) {
	    return (char *)NULL;
	}
    }
    return strdup(s->c_str());
}
