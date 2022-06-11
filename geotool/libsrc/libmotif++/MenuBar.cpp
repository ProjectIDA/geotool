/** \file MenuBar.cpp
 *  \brief Defines class MenuBar.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/MenuBar.h"
#include "motif++/Frame.h"
#include "motif++/Parse.h"
using namespace Parse;

/** Constructor.
 *  @param[in] name the name given to this MenuBar instance.
 *  @param[in] parent the Component parent.
 */
MenuBar::MenuBar(const string &name, Component *parent) : Component(name, parent)
{
    base_widget = XmCreateMenuBar(parent->baseWidget(), (char *)getName(),
				NULL,0);
    installDestroyHandler();
}

/** Constructor with Frame parent
 *  @param[in] name the name given to this MenuBar instance.
 *  @param[in] frame the Frame parent.
 */
MenuBar::MenuBar(const string &name, Frame *frame) : Component(name, frame)
{
    base_widget = XmCreateMenuBar(frame->mainWindow()->baseWidget(),
			(char *)getName(), NULL, 0);
    installDestroyHandler();
    frame->setMenuBar(this);
}

/** Set the help menu. The Help menu is positioned to the far right of the
 *  MenuBar.
 *  @param[in] help_menu a child Menu of this MenuBar.
 */
void MenuBar::setHelpMenu(Menu *help_menu)
{
    if(help_menu) {
	Arg args[1];
	XtSetArg(args[0], XmNmenuHelpWidget, help_menu->cascadeWidget());
	XtSetValues(base_widget, args, 1);
    }
}

Menu * MenuBar::getMenu(const string &menu_name)
{
    char *s, *e;
    Menu *menu=NULL;

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

Button * MenuBar::getButton(const string &button_name)
{
    Button *b;

    for(int i = 0; i < (int)comp_children.size(); i++) {
        if(comp_children[i]->getMenuInstance()) {
	    if((b =comp_children[i]->getMenuInstance()->getButton(button_name)))
	    {
		return b;
	    }
        }
	else if(comp_children[i]->getButtonInstance()) {
	    if(sameName(comp_children[i]->getName(), button_name)) {
		return comp_children[i]->getButtonInstance();
	    }
	}
    }
    return NULL;
}

Toggle * MenuBar::getToggle(const string &toggle_name)
{
    Toggle *t;

    for(int i = 0; i < (int)comp_children.size(); i++) {
        if(comp_children[i]->getMenuInstance()) {
	    if((t =comp_children[i]->getMenuInstance()->getToggle(toggle_name)))
	    {
		return t;
	    }
        }
	else if(comp_children[i]->getToggleInstance()) {
	    if(sameName(comp_children[i]->getName(), toggle_name)) {
		return comp_children[i]->getToggleInstance();
	    }
	}
    }
    return NULL;
}

ParseCmd MenuBar::parseCmd(const string &cmd, string &msg)
{
    Menu *menu;
    size_t n;

    if((n = cmd.find('.')) == string::npos) {
	return Component::parseCmd(cmd, msg);
    }

    for(int i = 0; i < (int)comp_children.size(); i++)
    {
	if( (menu = comp_children[i]->getMenuInstance()) )
	{
	    if( sameName(cmd, menu->getName(), (int)n) )
	    {
		return menu->parseCmd(cmd.substr(n+1), msg);
	    }
	}
    }
    return Component::parseCmd(cmd, msg);
}

ParseVar MenuBar::parseVar(const string &name, string &value)
{
    Menu *menu;
    size_t n;

    if((n = name.find('.')) == string::npos) {
	return Component::parseVar(name, value);
    }

    for(int i = 0; i < (int)comp_children.size(); i++)
    {
	if( (menu = comp_children[i]->getMenuInstance()) )
	{
	    if( sameName(name, menu->getName(), (int)n) )
	    {
		return menu->parseVar(name.substr(n+1), value);
	    }
	}
    }
    return Component::parseVar(name, value);
}
