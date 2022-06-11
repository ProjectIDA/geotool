#ifndef MENU_H
#define MENU_H

#include "motif++/Component.h"
#include "motif++/MenuBar.h"
#include "motif++/RowColumn.h"

class Button;
class ArrowButton;
class Toggle;
class Separator;
class PopupMenu;

/** A class for the XmPulldownMenu widget.
 *  @ingroup libmotif
 */
class Menu : public Component
{
    friend class Button;
    friend class ArrowButton;
    friend class Toggle;
    friend class Separator;

    public:

	Menu(const string &name, MenuBar *menuBar);
	Menu(const string &name, MenuBar *menuBar, int position);
	Menu(const string &name, Menu *menu);
	Menu(const string &name, RowColumn *rc);
	Menu(const string &name, Menu *menu, int position);
	Menu(const string &name, Menu *menu, bool radioBehavior);
	Menu(const string &name, PopupMenu *menu, bool radioBehavior);
	Menu(const string &name, PopupMenu *menu, Arg *args, int n);
	~Menu(void);

	Separator *addSeparator(const string &name,
			unsigned char type=XmSINGLE_LINE);
	/** Get the XmCascadeButton widget.
 	 *  @returns the XmCascadeButton widget
	 */
	Widget cascadeWidget() { return cascade; }

	virtual Menu *getMenuInstance(void) { return this; }
	virtual void setVisible(bool);
	virtual bool isVisible(void);
	virtual void destroy(void);
	void parseHelp(const char *prefix);

	Menu *getMenu(const string &name);
	Button *getButton(const string &name);
	Toggle *getToggle(const string &name);

    protected:

	Widget cascade;

	void initMenu(Menu *menu, bool radioBehavior);
	static void cascadingCallback(Widget, XtPointer, XtPointer);

};

#endif
