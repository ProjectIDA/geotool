#ifndef MENU_BAR_H
#define MENU_BAR_H

#include "motif++/Component.h"

class Frame;
class Menu;
class Button;
class Toggle;
class Menu;

/** A menubar class that uses XmCreateMenuBar.
 *  @ingroup libmotif
 */
class MenuBar : public Component
{
    friend class Menu;

    public:

	MenuBar(const string &, Component *);
	MenuBar(const string &, Frame *frame);

	Menu *getMenu(const string &name);
	Button *getButton(const string &name);
	Toggle *getToggle(const string &name);

	void setHelpMenu(Menu *help_menu);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
};

#endif
