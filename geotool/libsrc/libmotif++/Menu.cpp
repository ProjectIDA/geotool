/** \file Menu.cpp
 *  \brief Defines class Menu.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/Menu.h"
#include "motif++/MotifComp.h"
#include "motif++/Button.h"
#include "motif++/Toggle.h"
#include "motif++/Application.h"


/** Constructor with MenuBar parent.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu_bar the MenuBar parent.
 */
Menu::Menu(const string &name, MenuBar *menu_bar) : Component(name, menu_bar)
{
    base_widget = XmCreatePulldownMenu(menu_bar->baseWidget(),
			(char *)getName(), NULL, 0);
    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass,
			menu_bar->baseWidget(),
			XmNsubMenuId, base_widget,
			NULL);
    installDestroyHandler();
    XtAddCallback(cascade, XmNcascadingCallback,
		Menu::cascadingCallback, (XtPointer)this);
    enableCallbackType(XmNcascadingCallback);
}

/** Constructor with RowColumn parent.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] rc the RowColumn parent.
 */
Menu::Menu(const string &name, RowColumn *rc) : Component(name, rc)
{
    base_widget = XmCreatePulldownMenu(rc->baseWidget(), (char *)getName(), NULL, 0);
    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass,
			rc->baseWidget(),
			XmNsubMenuId, base_widget,
			NULL);
    installDestroyHandler();
    XtAddCallback(cascade, XmNcascadingCallback,
		Menu::cascadingCallback, (XtPointer)this);
    enableCallbackType(XmNcascadingCallback);
}

/** Constructor with MenuBar parent and position.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu_bar the MenuBar parent.
 *  @param[in] position the position of the Menu in the MenuBar. 0 is the top
 * 	of the menu. If position < 0, position the Menu alphabetically in the
 *	MenuBar.
 */
Menu::Menu(const string &name, MenuBar *menu_bar, int position) :
		Component(name, menu_bar)
{
    int i;
    short pos = (short)position;
    if(position < 0) {
	// get the alphabetical position
	for(i = 0; i < (int)menu_bar->comp_children.size()-1; i++) {
	    if(strcmp(name.c_str(), menu_bar->comp_children[i]->getName()) <= 0) break;
	}
	pos = (short)i;
    }
    for(i = (int)menu_bar->comp_children.size()-1; i > pos; i--) {
	menu_bar->comp_children[i] = menu_bar->comp_children[i-1];
    }
    menu_bar->comp_children[pos] = (Component *)this;
    base_widget = XmCreatePulldownMenu(menu_bar->baseWidget(),(char *)getName(),
			NULL, 0);
    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass,
			menu_bar->baseWidget(),
			XmNsubMenuId, base_widget,
			XmNpositionIndex, pos,
			NULL);
    installDestroyHandler();
    XtAddCallback(cascade, XmNcascadingCallback,
		Menu::cascadingCallback, (XtPointer)this);
    enableCallbackType(XmNcascadingCallback);
}

/** Constructor with Menu parent.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu the Menu parent.
 */
Menu::Menu(const string &name, Menu *menu) : Component(name, menu)
{
    initMenu(menu, False);
}

/** Constructor with Menu parent and position.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu the MenuBar parent.
 *  @param[in] position the position of the Menu in the MenuBar. 0 is the top
 * 	of the menu. If position < 0, position the Menu alphabetically in the
 *	MenuBar.
 */
Menu::Menu(const string &name, Menu *menu, int position) : Component(name, menu)
{
    int i;
    short pos = (short)position;
    if(position < 0) {
	// get the alphabetical position
	for(i = 0; i < (int)menu->comp_children.size()-1; i++) {
	    if(strcmp(name.c_str(), menu->comp_children[i]->getName()) <= 0) break;
	}
	pos = (short)i;
    }
    for(i = (int)menu->comp_children.size()-1; i > pos; i--) {
	menu->comp_children[i] = menu->comp_children[i-1];
    }
    menu->comp_children[pos] = (Component *)this;

    base_widget = XmCreatePulldownMenu(menu->baseWidget(), (char *)getName(), NULL, 0);

    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass,
			menu->baseWidget(),
			XmNsubMenuId, base_widget,
			XmNpositionIndex, pos,
			NULL);
    installDestroyHandler();
    XtAddCallback(cascade, XmNcascadingCallback,
		Menu::cascadingCallback, (XtPointer)this);
    enableCallbackType(XmNcascadingCallback);
}

/** Constructor with Menu parent and radio behavoir.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu the Menu parent.
 *  @param[in] radio_behavior if true, Toggle children have radio button
 *	behavior.
 */
Menu::Menu(const string &name, Menu *menu, bool radio_behavior) :
		Component(name, menu)
{
    initMenu(menu, radio_behavior);
}

/** Constructor with PopupMenu parent and radio behavoir.
 *  @param[in] name the name given to this PopupMenu instance.
 *  @param[in] menu the PopupMenu parent.
 *  @param[in] radio_behavior if true, Toggle children have radio button
 *	behavior.
 */
Menu::Menu(const string &name, PopupMenu *menu, bool radio_behavior) :
		Component(name, menu)
{
    Arg args[1];

    XtSetArg(args[0], XmNradioBehavior, radio_behavior);
    base_widget = XmCreatePulldownMenu(menu->baseWidget(), (char *)getName(), args, 1);

    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass, menu->baseWidget(),
			XmNsubMenuId, base_widget,
			NULL);
    installDestroyHandler();
}

/** Constructor with PopupMenu parent and X-resources.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu the PopupMenu parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
Menu::Menu(const string &name, PopupMenu *menu, Arg *args, int n) :
		Component(name, menu)
{
    base_widget = XmCreatePulldownMenu(menu->baseWidget(), (char *)getName(), args, n);

    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass, menu->baseWidget(),
			XmNsubMenuId, base_widget,
			NULL);
    installDestroyHandler();
}

/** Initialize.
 *  @param[in] name the name given to this Menu instance.
 *  @param[in] menu the Menu parent.
 *  @param[in] radio_behavior if true, Toggle children have radio button
 *	behavior.
 */
void Menu::initMenu(Menu *menu, bool radio_behavior)
{
    Arg args[1];

    XtSetArg(args[0], XmNradioBehavior, radio_behavior);

    base_widget = XmCreatePulldownMenu(menu->baseWidget(), (char *)getName(), args, 1);

    cascade = XtVaCreateManagedWidget((char *)getName(),
			xmCascadeButtonWidgetClass,
			menu->baseWidget(),
			XmNsubMenuId, base_widget,
			NULL);
    installDestroyHandler();
    XtAddCallback(cascade, XmNcascadingCallback,
		Menu::cascadingCallback, (XtPointer)this);
    enableCallbackType(XmNcascadingCallback);
}

/** Destructor.
 */
Menu::~Menu(void)
{
}

void Menu::destroy(void)
{
    XtRemoveAllCallbacks(cascade, XmNcascadingCallback);
    Component::destroy();
    XtDestroyWidget(cascade);
}

void Menu::cascadingCallback(Widget w, XtPointer client, XtPointer data)
{
    Menu *menu = (Menu *)client;
    menu->doCallbacks(w, data, (const char *)XmNcascadingCallback);
}

/** Add a Separator with type argument. Valid types are:
 *  - XmSINGLE_LINE
 *  - XmDOUBLE_LINE
 *  - XmSINGLE_DASHED_LINE
 *  - XmDOUBLE_DASHED_LINE
 *  - XmNO_LINE
 *  - XmSHADOW_ETCHED_IN
 *  - XmSHADOW_ETCHED_OUT
 *  - XmSHADOW_ETCHED_IN_DASH
 *  - XmSHADOW_ETCHED_OUT_DASH
 *  @param[in] name the name given to the Separator instance.
 *  @param[in] type the XmNseparatorType
 *  @returns the Separator instance.
 */
Separator * Menu::addSeparator(const string &name, unsigned char type)
{
    Arg args[1];
    XtSetArg(args[0], XmNseparatorType, type);
    Separator *sep = new Separator(name, this, args, 1);
    return sep;
}


void Menu::setVisible(bool visible)
{
    if(visible) {
	XtManageChild(cascade);
    }
    else {
	XtUnmanageChild(cascade);
    }
}

bool Menu::isVisible(void) {
    return XtIsManaged(cascade);
}

void Menu::parseHelp(const char *prefix)
{
    char *s;
    for(int i = 0; i < (int)comp_children.size(); i++) {
	if(comp_children[i]->getToggleInstance() ||
		comp_children[i]->getButtonInstance())
	{
	    s = strdup(comp_children[i]->getName());
	    for(int j = 0; s[j] != '\0'; j++) {
		if(isspace((int)s[j])) s[j] = '_';
		else s[j] = tolower((int)s[j]);
	    }
	    printf("%s%s\n", prefix, s);
	    Free(s);
	}
    }
    for(int i = 0; i < (int)comp_children.size(); i++) {
	if(comp_children[i]->getMenuInstance()) {
	    comp_children[i]->getMenuInstance()->parseHelp(prefix);
	}
    }
}

Menu * Menu::getMenu(const string &menu_name)
{
    if(sameName(getName(), menu_name)) return this;

    char *s, *e;
    Menu *menu = NULL;

    s = strdup(menu_name.c_str());

    if( (e = strchr(s, (int)'.')) ) {
	*e = '\0';
	if( (menu = getMenu((const char *)s)) && *(e+1) != '\0') {
	    menu = menu->getMenu((const char *)(e+1));
	}
    }
    else {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    if(comp_children[i]->getMenuInstance() &&
		(menu = comp_children[i]->getMenuInstance()->getMenu(s))) break;
	}
    }
    free(s);
    return menu;
}

Button * Menu::getButton(const string &button_name)
{
    char *s, *e;
    Button *button = NULL;
    Menu *menu = NULL;

    s = strdup(button_name.c_str());

    if( (e = strchr(s, (int)'.')) ) {
	*e = '\0';
	if( (menu = getMenu((const char *)s)) && *(e+1) != '\0') {
	    button = menu->getButton((const char *)(e+1));
	}
    }
    else {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    if(comp_children[i]->getButtonInstance() &&
		sameName(comp_children[i]->getName(), button_name))
	    {
		button = comp_children[i]->getButtonInstance();
		break;
	    }
	}
    }
    free(s);
    return button;
}

Toggle * Menu::getToggle(const string &toggle_name)
{
    char *s, *e;
    Toggle *toggle = NULL;
    Menu *menu = NULL;

    s = strdup(toggle_name.c_str());

    if( (e = strchr(s, (int)'.')) ) {
	*e = '\0';
	if( (menu = getMenu((const char *)s)) && *(e+1) != '\0') {
	    toggle = menu->getToggle((const char *)(e+1));
	}
    }
    else {
	for(int i = 0; i < (int)comp_children.size(); i++) {
	    if(comp_children[i]->getToggleInstance() &&
		sameName(comp_children[i]->getName(), toggle_name))
	    {
		toggle = comp_children[i]->getToggleInstance();
		break;
	    }
	}
    }
    free(s);
    return toggle;
}
