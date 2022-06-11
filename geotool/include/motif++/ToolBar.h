#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include "motif++/Component.h"

class Frame;
class MenuBar;

/// @cond
/** For ToolBar internal use. Links a MenuBar item with the widget child of
 *  the Add menu or the widget child of the Set Accelerator menu.
 */
typedef struct
{
	Component	*menubar_item;	//!< the MenuBar child
	Widget		item;		//!< the Add menu child
} WidgetPair;

/** For ToolBar internal use. Links a MenuBar item with the widget child of
 *  the Add menu or the widget child of the Set Accelerator menu.
 */
typedef struct
{
	Component	*menubar_item;	//!< the MenuBar child
	Widget		item;		//!< the Remove menu child
	Widget		remove_item;	//!< the ToolBar child
} WidgetThree;
/// @endcond

/** A container for Buttons, Toggles, and Menus.
 *  When the ToolBar is created with a MenuBar source, it has an option to add
 *  any item to the ToolBar that is in the MenuBar. A small arrow button on
 *  the left side of the ToolBar is a pulldown menu with three child menus, the
 *  Add menu, the Remove menu and the Set Accelerator menu. A ToolBar with the
 *  Add menu activated is shown in the screen image below. An item selected
 *  from the ToolBar Add menu will be inserted into the ToolBar. An item
 *  selected from the Remove menu will be removed from the ToolBar. The Set
 *  Accelerator menu allows accelerator keys to be set for any item in the
 *  MenuBar. The example code below demonstrates how to create a MenuBar and
 *  a ToolBar and process the actions from the Button and Toggle children.
 *  \image html ToolBar.gif "The ToolBar Add menu "
 *  \image latex ToolBar.eps "The ToolBar Add menu" width=6in
 *  \include ToolBarExample.cpp
 *  @ingroup libmotif
 */
class ToolBar : public Component
{
    public:

	ToolBar(const string &name, Component *component);
	ToolBar(const string &name, Frame *frame, MenuBar *menu_bar=NULL);
	~ToolBar(void);

	Widget add(Component *comp, const string &name="",
		int insert_position=(int)XmLAST_POSITION);
	void removeAllItems(void);
	bool loadDefaults(void);
	bool changeLabel(const string &name, const string &label);

	virtual Widget containerWidget(void) { return toolBar; }
	void actionPerformed(ActionEvent *action_event);

    protected:
	/** @name The ToolBar Edit Menu
\code
                                    base_widget
                                   .     .  .   .
                                .        .     .      .
                             .           .         .         .
	              toolBar_edit    toolWindow   arrowLeft  arrowRight
		          .       .
	                 .	     .
                     toolBarEdit     toolBarEditBtn
                  .              .
              .                         .
          .                                    .
  addMenu  addBtn removeMenu removeBtn accelMenu accelBtn
\endcode
	*/
	//@{
	//! the XmMENU_BAR type RowColumn parent of toolBarEdit
	Widget toolBar_edit;
	//! pulldown menu parent of the Add, Remove and Set Accelerator menus
	Widget toolBarEdit;
	Widget toolBarEditBtn; //!< CascadeButton for toolBarEdit
	Widget addMenu; //!< the pulldown Add menu.
	Widget addBtn; //!< CascadeButton for addMenu

	Widget removeMenu; //!< the pulldown Remove menu
	Widget removeBtn; //!< CascadeButton for removeMenu
	Widget accelMenu; //!< the pulldown Set Accelerator menu
	Widget accelBtn;  //!< CascadeButton for accelMenu
	//@}
	Widget toolWindow; //!< the ScrolledWindow parent of toolBar
	Widget toolBar;	   //!< the RowColumn parent of the ToolBar items
	Widget arrow_Right;//!< the right ArrowButton
	Widget arrow_Left; //!< the left ArrowButton

	Widget dialog; //!< the Toolbar Button Label FormDialog
	//! the Text widget in the Toolbar Button Label window
	Widget position_text;
	Widget label_text; //!< the Button Label Text widget.
	Widget ctrl_toggle; //!< the Ctrl ToggleButton in Set Accelerator
	Widget alt_toggle;  //!< the Alt ToggleButton in Set Accelerator
	Widget target_list;  //!< the Target List in Set Accelerator

	MenuBar	*menuBar; //!< the MenuBar that the ToolBar is attached to.
	Component *adding_menu_item; //!< the item being added (or accelerated)
/// @cond
	vector<WidgetPair> add_menu_items; //!< the Add menu items.
	vector<WidgetPair> accel_menu_items;//!< the Set Accelerator menu items.
	vector<WidgetThree> toolbar_children; //!< the Remove menu items.
/// @endcond

	void createInterface(MenuBar *);
	void setButtonPixmap(void);
	void writeToolbarProperty(void);
	int getButtonNames(char *property_string, char ***menu_item_names,
			char ***toolbar_names);
	Widget addButton(Component *comp, const string &name,
			int insert_position, bool write_property);
	void addMenuInit(void);
	void accelMenuInit(void);
	void setAccelerator(void);
	void removeMenuInit(void);
	int makeAddMenu(Component *, Widget);
	int makeAccelMenu(Component *, Widget);
	bool inToolBar(Component *);
	void getAddButton(Widget);
	void getAccelButton(Widget);
	void scale(XmScrollBarCallbackStruct *s);
	void toolbarLabel(void);
	void removeButton(Widget);
	void layout(void);
	void arrowRight(void);
	void arrowLeft(void);
	void showArrows(void);

    private:
	static Cardinal InsertPosition(Widget);
	static char *buttonName(Widget w);
	static void resizeCB(Widget, XtPointer, XEvent *, Boolean *);
	static void editMenuCB(Widget, XtPointer, XtPointer);
	static void arrowRightCB(Widget, XtPointer, XtPointer);
	static void arrowLeftCB(Widget, XtPointer, XtPointer);
	static void addButtonCB(Widget, XtPointer, XtPointer);
	static void setAcceleratorCB(Widget, XtPointer, XtPointer);
	static void accelButtonCB(Widget, XtPointer, XtPointer);
	static void cancelCB(Widget, XtPointer, XtPointer);
	static void scaleCB(Widget, XtPointer, XtPointer);
	static void toolbarLabelCB(Widget, XtPointer, XtPointer);
	static void removeButtonCB(Widget, XtPointer, XtPointer);
	static void Sync2TogglesCB(Widget, XtPointer, XtPointer);
	static void toolButtonActivateCB(Widget, XtPointer, XtPointer);
	static void targetListCB(Widget, XtPointer, XtPointer);
};

#endif
