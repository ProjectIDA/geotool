#ifndef _FRAME_H
#define _FRAME_H

#include "motif++/ToolBar.h"
#include "motif++/InfoArea.h"
#include "motif++/MotifComp.h"
#include "motif++/Menu.h"
#include "motif++/TopWindow.h"

class Separator;

/** A subclass of TopWindow that provides a MenuBar, ToolBar and InfoArea.
 *  The layout of the Frame window Components is show below. The Frame attribute
 *  names are shown in parentheses next to the class name. The work_form,
 *  menu_bar and tool_bar are all children of the main_window. The frame_form
 *  and the info_area are children of the work_form. The Frame constructors
 *  create the main_window, the work_form and the frame_form. The menu_bar,
 *  tool_bar, and info_area are not created by default. Frame subclasses
 *  should use the frame_form as the parent of their Components.
 *  \image html Frame_small.gif "The Frame Components"
 *  \image latex Frame.eps "The Frame Components" width=6in
 *
 *  The example code below illustrates the creation of a Frame window with
 *  a MenuBar, ToolBar and InfoArea. The MenuBar contains a File Menu and a
 *  View Menu. The File menu contains a Close Button. The View Menu contains
 *  a Date Button. Note that the menu_bar, tool_bar, info_area, file_menu,
 *  and view_menu attributes are inherited from the Frame class. There is also
 *  an edit_menu, an option_menu and a help_menu, but they are not created in
 *  this example. A screen image of this example is also shown below.
 *  \include motif++/FrameExample.cpp
 *  \image html FrameExample.gif "Frame Example"
 *  @ingroup libmotif
 */
class Frame : public TopWindow
{
    public:

	Frame(const string &name, Component *parent, const string &title,
			bool pointer_focus=false, bool independent=true);
	Frame(const string &name, Component *parent, bool pointer_focus=false,
			bool independent=true);
	~Frame(void);

	virtual void iconify(void);
	void setVisible(bool);
	void setMenuBar(MenuBar *menuBar);
	void setToolBar(ToolBar *toolBar);
	void setInfoArea(InfoArea *infoArea);
	void quit(bool confirm=false);

	/** Get the MainWindow child of the Frame.
	 *  @returns the MainWindow child
	 */
	MainWindow *mainWindow(void) { return main_window; }
	/** Get the frame Form child of the Frame.
	 *  @returns the frame Form
	 */
	Form *frameForm(void) { return frame_form; }
	/** Get the work Form child of the Frame.
	 *  @returns the work Form
	 */
	Form *workForm(void) { return work_form; }
	/** Get the MenuBar child of the Frame.
	 *  @returns the MenuBar
	 */
	MenuBar *menuBar(void) { return menu_bar; }
	/** Get the File Menu child of the Frame MenuBar
	 *  @returns the File Menu
	 */
	Menu *fileMenu(void) { 
	    if(!file_menu) file_menu = new Menu("File", menu_bar);
	    return file_menu;
	}
	/** Get the Edit Menu child of the Frame MenuBar
	 *  @returns the Edit Menu
	 */
	Menu *editMenu(void) {
	    if(!edit_menu) edit_menu = new Menu("Edit", menu_bar);
	    return edit_menu;
	}
	/** Get the View Menu child of the Frame MenuBar
	 *  @returns the View Menu
	 */
	Menu *viewMenu(void) {
	    if(!view_menu) view_menu = new Menu("View", menu_bar);
	    return view_menu;
	}
	/** Get the Option Menu child of the Frame MenuBar
	 *  @returns the Option Menu
	 */
	Menu *optionMenu(void) {
	    if(!option_menu) option_menu = new Menu("Option", menu_bar);
	    return option_menu;
	}
	/** Get the Help Menu child of the Frame MenuBar
	 *  @returns the Help Menu
	 */
	Menu *helpMenu(void) {
	    if(!help_menu) help_menu = new Menu("Help", menu_bar);
	    return help_menu;
	}
	/** Get the ToolBar child of the Frame
	 *  @returns the ToolBar
	 */
	ToolBar *toolBar(void) {
	    if(!tool_bar) tool_bar = new ToolBar("ToolBar", this, menu_bar);
	    return tool_bar;
	}
	/** Get the InfoArea child of the Frame
	 *  @returns the InfoArea
	 */
	InfoArea *infoArea(void) { return info_area; };

	virtual Frame *getFrameInstance(void) { return this; }

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseCmd addToToolbar(const string &s, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void createInterface(void);

    protected:
	MainWindow	*main_window;	//!< The MainWindow child
	Form		*work_form;	//!< The work Form child
	/** The frame Form child. Components created be subclasses should be
	 *  children of the frame_form. */
	Form		*frame_form;
	//! The separator between the ToolBar and the frame Form
	Separator	*separator;
	MenuBar		*menu_bar;	//!< The MenuBar child
	ToolBar		*tool_bar;	//!< The ToolBar child
	InfoArea	*info_area;	//!< The InfoArea child

	Menu	*file_menu;	//!< The File Menu
	Menu	*edit_menu;	//!< The Edit Menu
	Menu	*view_menu;	//!< The View Menu
	Menu	*option_menu;	//!< The Option Menu
	Menu	*help_menu;	//!< The Help Menu

	void init(void);
	bool popup_activate(XtGrabKind grab_kind, unsigned int source_indication);
	virtual void setFileMenu(void) {}
	virtual void setEditMenu(void) {}
	virtual void setViewMenu(void) {}
	virtual void setOptionMenu(void) {}
	virtual void setHelpMenu(void) {}
	static void HandleWindowDel(Widget, XtPointer, XEvent *, Boolean *);

};
#endif
