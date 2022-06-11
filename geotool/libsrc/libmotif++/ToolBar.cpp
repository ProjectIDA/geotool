/** \file ToolBar.cpp
 *  \brief Defines class ToolBar.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/ToolBar.h"
#include "motif++/Frame.h"
#include "motif++/Button.h"
#include "motif++/Toggle.h"
#include "motif++/Menu.h"
#include "motif++/Application.h"

extern "C" {
#include "libstring.h"
}

static int get_Height(Widget widget);
static void set_Height(Widget widget, int height);

/** Constructor.
 *  @param[in] name the name given to this ToolBar instance.
 *  @param[in] parent the Component parent.
 */
ToolBar::ToolBar(const string &name, Component *parent) :
		Component(name, parent)
{
    base_widget = XtVaCreateWidget(getName(), xmFormWidgetClass,
				parent->baseWidget(),
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNhighlightThickness, 0,
				NULL);
    installDestroyHandler();

    createInterface(NULL);
}

/** Constructor with ToolBar parent and MenuBar source.
 *  @param[in] name the name given to this ToolBar instance.
 *  @param[in] frame the Frame parent.
 *  @param[in] menu_bar the source for ToolBar
 */
ToolBar::ToolBar(const string &name, Frame *frame, MenuBar *menu_bar)
			: Component(name, frame)
{
    base_widget = XtVaCreateWidget(getName(), xmFormWidgetClass,
				frame->workForm()->baseWidget(),
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNhighlightThickness, 0,
				NULL);
    installDestroyHandler();

    createInterface(menu_bar);

    frame->setToolBar(this);
}

/** Destructor */
ToolBar::~ToolBar(void)
{
}

/** Create the interface
 *  @param[in] menu_bar if menu_bar is not NULL, the toolBarEditBtn is
 * 	created.
 */
void ToolBar::createInterface(MenuBar *menu_bar)
{
    Arg args[2];

    menuBar = menu_bar;

    if(menuBar)
    {
	toolBar_edit = XtVaCreateManagedWidget("toolBar_edit",
			xmRowColumnWidgetClass, base_widget,
			XmNrowColumnType, XmMENU_BAR,
			XmNleftAttachment, XmATTACH_FORM,
			XmNtopAttachment, XmATTACH_FORM,
			XmNmarginTop, 0,
			XmNmarginBottom, 0,
			XmNmarginHeight, 0,
			XmNmarginWidth, 0,
			XmNborderWidth, 0,
			XmNspacing, 0,
			XmNshadowThickness, 2,
			XmNadjustMargin, False,
			XmNadjustLast, False,
			XmNresizePolicy, XmRESIZE_ANY,
			NULL);

	XtSetArg(args[0], XmNmarginWidth, 0);
	toolBarEdit = XmCreatePulldownMenu(toolBar_edit, (char *)"toolBarEdit",
			args,1);

	toolBarEditBtn = XtVaCreateManagedWidget("toolBarEditBtn",
			xmCascadeButtonWidgetClass, toolBar_edit,
			XmNmarginHeight, 0,
			XmNmarginWidth, 0,
			XmNmarginTop, 1,
			XmNmarginLeft, 1,
			XmNspacing, 0,
			XmNborderWidth, 0,
			XmNshadowThickness, 0,
// These two resources caused a BadDrawable X Error on a Suse10.2 64bit machine
// The code appears to work correctly without them. IH 2007/9/27
//			XmNlabelType, XmPIXMAP,
//			XmNlabelPixmap, XmUNSPECIFIED_PIXMAP,
			XmNalignment, XmALIGNMENT_CENTER,
			XmNsubMenuId, toolBarEdit,
			XmNtearOffModel, XmTEAR_OFF_DISABLED,
			XmNresizePolicy, XmRESIZE_ANY,
			NULL);
	XtAddCallback(toolBarEditBtn, XmNcascadingCallback, editMenuCB,
                        (XtPointer)this);
	setButtonPixmap();

	addMenu = XmCreatePulldownMenu(toolBarEdit, (char *)"addMenu", NULL, 0);
	addBtn = XtVaCreateManagedWidget("Add", xmCascadeButtonWidgetClass,
			toolBarEdit,
			XmNsubMenuId, addMenu,
			NULL);
	removeMenu = XmCreatePulldownMenu(toolBarEdit, (char *)"removeMenu",
			NULL, 0);
	removeBtn = XtVaCreateManagedWidget("Remove",
			xmCascadeButtonWidgetClass,
			toolBarEdit,
			XmNsubMenuId, removeMenu,
			NULL);
	accelMenu = XmCreatePulldownMenu(toolBarEdit, (char *)"accelMenu",
			NULL, 0);
	accelBtn = XtVaCreateManagedWidget("Set Accelerator",
			xmCascadeButtonWidgetClass,
			toolBarEdit,
			XmNsubMenuId, accelMenu,
			NULL);

	toolWindow = XtVaCreateManagedWidget("toolWindow",
			xmScrolledWindowWidgetClass, base_widget,
			XmNshadowThickness, 0,
			XmNscrollingPolicy, XmAUTOMATIC,
			XmNscrollBarDisplayPolicy, XmSTATIC,
//Not needed? 3/12/07	XmNscrolledWindowChildType, XmSCROLL_HOR,
			XmNtopAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_WIDGET,
			XmNleftWidget, toolBar_edit,
			XmNrightAttachment, XmATTACH_FORM,
			XmNresizePolicy, XmRESIZE_ANY,
			NULL);
    }
    else {
	toolWindow = XtVaCreateManagedWidget("toolWindow",
			xmScrolledWindowWidgetClass, base_widget,
			XmNshadowThickness, 0,
			XmNscrollingPolicy, XmAUTOMATIC,
			XmNscrollBarDisplayPolicy, XmSTATIC,
//			XmNscrolledWindowChildType, XmSCROLL_HOR,
			XmNtopAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNresizePolicy, XmRESIZE_ANY,
			NULL);
    }
    Widget hor = NULL, vert = NULL;
    XtSetArg(args[0], XmNhorizontalScrollBar, &hor);
    XtSetArg(args[1], XmNverticalScrollBar, &vert);
    XtGetValues(toolWindow, args, 2);
    if(hor != NULL) XtUnmanageChild(hor);
    if(vert != NULL) XtUnmanageChild(vert);

    toolBar = XtVaCreateManagedWidget("toolBar", xmRowColumnWidgetClass,
			toolWindow,
			XmNisHomogeneous, False,
			XmNradioBehavior, False,
			XmNradioAlwaysOne, False,
			XmNorientation, XmHORIZONTAL,
			XmNresizePolicy, XmRESIZE_ANY,
			NULL);

    arrow_Right = XtVaCreateWidget("arrowRight", xmArrowButtonWidgetClass,
			base_widget,
			XmNtopAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNarrowDirection, XmARROW_RIGHT,
			XmNshadowThickness, 0,
			NULL);
    XtAddCallback(arrow_Right, XmNactivateCallback, arrowRightCB,
                        (XtPointer)this);

    arrow_Left = XtVaCreateWidget("arrowLeft", xmArrowButtonWidgetClass,
			base_widget,
			XmNtopAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_WIDGET,
			XmNrightWidget, arrow_Right,
			XmNarrowDirection, XmARROW_LEFT,
			XmNshadowThickness, 0,
			NULL);
    XtAddCallback(arrow_Left, XmNactivateCallback, arrowLeftCB,(XtPointer)this);

    layout();

    XtAddEventHandler(base_widget, ExposureMask|VisibilityChangeMask
			|ResizeRedirectMask, False, resizeCB, (XtPointer)this);

    XtManageChild(base_widget);
}

/** Process the actions from the ToolBar edit menus.
 */
void ToolBar::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(!strcmp(action_event->getReason(), XtNsetSensitiveCallback))
    {
	bool set = (bool)action_event->getCalldata();
	for(int i = 0; i < (int)toolbar_children.size(); i++) {
	    if(toolbar_children[i].menubar_item == comp) {
		XtSetSensitive(toolbar_children[i].item, set);
		return;
	    }
	}
    }
    else if(!strcmp(action_event->getReason(), XtNsetVisibleCallback))
    {
	bool set = (bool)action_event->getCalldata();
	for(int i = 0; i < (int)toolbar_children.size(); i++) {
	    if(toolbar_children[i].menubar_item == comp) {
		if(set) {
		    XtManageChild(toolbar_children[i].item);
		}
		else {
		    XtUnmanageChild(toolbar_children[i].item);
		}
		return;
	    }
	}
    }
    else if(!strcmp(action_event->getReason(), XtNsetLabelCallback))
    {
	char *label = (char *)action_event->getCalldata();
	for(int i = 0; i < (int)toolbar_children.size(); i++) {
	    if(toolbar_children[i].menubar_item == comp) {
		XmString xm = createXmString(label);
		XtVaSetValues(toolbar_children[i].item, XmNlabelString,xm,NULL);
		XmStringFree(xm);
		return;
	    }
	}
    }
}

/** The XmNcascadingCallback callback for the ToolBar editMenu.
 */
void ToolBar::editMenuCB(Widget widget, XtPointer client, XtPointer data)
{
    ToolBar *toolbar = (ToolBar *)client;
    toolbar->addMenuInit();
    toolbar->removeMenuInit();
    toolbar->accelMenuInit();
}

/** Set the toolBarEditBtn pixmap.
 */
void ToolBar::setButtonPixmap(void)
{
    typedef struct
    {
	int screen;
	Pixmap	pixmap;
    } ScreenPixmap;
    static ScreenPixmap sp[10] = {
	{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}
    };
    Widget w;
    Arg args[2];
    Pixel bg, fg;
    int i, screen;
    unsigned int depth;
    unsigned char bits[] = {0x18, 0x00, 0x38, 0x00, 0x78, 0x00, 0xf8, 0x00,
		0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0x38, 0x00, 0x18, 0x00};

    w = Application::getApplication()->baseWidget();
    screen = DefaultScreen(XtDisplay(w));

    for(i = 0; i < 10 && sp[i].screen != -1 && sp[i].screen != screen; i++);
    if(i < 10 && sp[i].screen == -1 && XtWindow(w) != 0) {
	sp[i].screen = screen;

	XtSetArg(args[0], XmNbackground, &bg);
	XtSetArg(args[1], XmNforeground, &fg);
	XtGetValues(toolBarEditBtn, args, 2);
	depth = DefaultDepth(XtDisplay(w), screen);
	sp[i].pixmap = XCreatePixmapFromBitmapData(XtDisplay(w),
		XtWindow(w), (char *)bits, 9, 9, fg, bg, depth);
    }
    if(i < 10 && sp[i].pixmap != 0) {
	XtSetArg(args[0], XmNlabelType, XmPIXMAP);
	XtSetArg(args[1], XmNlabelPixmap, sp[i].pixmap);
	XtSetValues(toolBarEditBtn, args, 2);
    }
}

/** Load the initial ToolBar contents from a program property.
 *  The toolBar contents are specified with the property: \verbatim
    getParent()->getName().getName().toolBar
    \endverbatim
 *  For example, if the parent name is geotool and the ToolBar name is
 *  toolbar, then the following property will add three items to the ToolBar:
 *  \verbatim
    geotool.toolbar.toolBar=File.Open Database,DB,View.Select.Select All,Select,View.Scale.Space More,>,View.Scale.Space Less,<
    \endverbatim
 *  Each item is specified by two of the comma-delimited strings. The first
 *  string is the item's name and the second string is the name of the item
 *  in the ToolBar. \verbatim
 	MenuBar item name		ToolBar name
 	File.Open Database		    DB
	View.Select.Select All		Select All
	View.Scale.Space More		    >
	View.Scale.Space Less		    < \endverbatim
 *  @throw GERROR_MALLOC_ERROR
 * 
 */
bool ToolBar::loadDefaults(void)
{
    char **menu_item_names = NULL, **toolbar_names = NULL;
    char *buttons = NULL, prop_name[200];
    Component **comps=NULL;
    int i, j, n;

    /* Get the initial contents of the toolbar from a property
     */
    snprintf(prop_name, sizeof(prop_name), "%s.%s.toolBar",
		getParent()->getName(), getName());

    if((buttons = getProperty(prop_name)) == NULL) return false;

    n = getButtonNames(buttons, &menu_item_names, &toolbar_names);

    free(buttons);

    comps = (Component **)malloc(n*sizeof(Component *));

    for(i = j = 0; i < n; i++)
    {
	if((comps[i] = menuBar->findButton(menu_item_names[i])) != NULL ||
	   (comps[i] = menuBar->findToggle(menu_item_names[i])) != NULL ||
	   (comps[i] = menuBar->findMenu(menu_item_names[i])) != NULL)
	{
	    j++;
	}
	else {
	    fprintf(stderr, "ToolBar.loadDefaults: cannot find Component %s\n",
			menu_item_names[i]);
	}
    }
    if(j > 0)
    {
/*	Uncomment this if you want to remove all buttons before adding the
	default buttons. 
	for(j = 0; j < (int)toolbar_children.size(); j++) {
	    toolbar_children[j].menubar_item->removeActionListener(this,
				XtNsetSensitiveCallback);
	    toolbar_children[j].menubar_item->removeActionListener(this,
				XtNsetVisibleCallback);
	    XtUnmanageChild(toolbar_children[j].item);
	    XtDestroyWidget(toolbar_children[j].item);
	}
	toolbar_children.clear();
*/
	for(i = 0; i < n; i++) {
	    if(comps[i]) {
		for(j = 0; j < (int)toolbar_children.size()
			&& comps[i] != toolbar_children[j].menubar_item; j++);
		if(j == (int)toolbar_children.size()) {
		    addButton(comps[i], toolbar_names[i], i, False);
		}
	    }
	}
    }
    Free(comps);
    for(i = 0; i < n; i++) {
	free(menu_item_names[i]);
	free(toolbar_names[i]);
    }
    free(menu_item_names);
    free(toolbar_names);

    layout();

    return true;
}

/** Get the initial ToolBar items from the program property string.
 *  @param[in] property_string the getParent()->getName().getName().toolBar property
 *  @param[out] menu_item_names an array of menu item names. Free each name
 *	and the array pointer when no longer needed.
 *  @param[out] toolbar_names an array of corresponding toolbar names. Free each
 *	name and the array pointer when no longer needed.
 *  @returns the number of items found in the property string.
 */
int ToolBar::getButtonNames(char *property_string, char ***menu_item_names,
			char ***toolbar_names)
{
    char **m_names = NULL, **t_names = NULL;
    char *tok, *c1, *c2, *last, widget_name[100];
    int n;

    if(property_string == NULL) return 0;

    tok = property_string;
    n = 0;
    while ((c1 = strtok_r(tok, ",", &last)) != NULL &&
	   (c2 = strtok_r(NULL, ",", &last)) != NULL)
    {
	tok = NULL;
	c1 = stringTrim(c1);
	c2 = stringTrim(c2);
/*
	if(c1[0] != '*') {
	    snprintf(widget_name, sizeof(widget_name), "*%s", c1);
	}
	else {
*/
	    stringcpy(widget_name, c1, sizeof(widget_name));
//	}
	m_names = (char **)reallocWarn(m_names, (n+1)*sizeof(char *));
	t_names = (char **)reallocWarn(t_names, (n+1)*sizeof(char *));
	m_names[n] = strdup(widget_name);
	t_names[n] = strdup(c2);
	n++;
    }
    *menu_item_names = m_names;
    *toolbar_names = t_names;

    return n;
}

/** Add a Component to the ToolBar with name and position.
 *  @param[in] comp the Component to add.
 *  @param[in] name the name the Component will have in the Toolbar.
 *  @param[in] insert_position the position of the item in the ToolBar.
 *  @returns the base_widget of the ToolBar Component.
 */
Widget ToolBar::add(Component *comp, const string &name, int insert_position)
{
    Widget w;
    if(!name.empty()) {
	w = addButton(comp, name, insert_position, False);
    }
    else {
	w = addButton(comp, comp->getName(), insert_position, False);
    }
    layout();
    return w;
}

/** Add a Component to the ToolBar with name, position and save option.
 *  @param[in] menubar_item the Component to add.
 *  @param[in] name the name the Component will have in the Toolbar.
 *  @param[in] insert_position the position of the item in the ToolBar.
 *  @param[in] write_property if true, the ToolBar property is saved.
 *  @returns the base_widget of the ToolBar Component.
 */
Widget ToolBar::addButton(Component *menubar_item, const string &name,
			int insert_position, bool write_property)
{
    int n;
    Arg args[6];
    Widget button = NULL;
    Toggle *t;

    if(menubar_item->getButtonInstance())
    {
	button = XtVaCreateManagedWidget(name.c_str(),
			xmPushButtonWidgetClass, toolBar,
			XmNshadowThickness, 2,
			XmNpositionIndex, insert_position,
			NULL);
	XtAddCallback(button, XmNactivateCallback, 
			(XtCallbackProc)toolButtonActivateCB,
			(XtPointer)menubar_item);
    }
    else if( (t = menubar_item->getToggleInstance()) )
    {
	n = 0;
	XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNmarginHeight, 0); n++;
	XtSetArg(args[n], XmNshadowThickness, 2); n++;
	XtSetArg(args[n], XmNpositionIndex, insert_position); n++;
	XtSetArg(args[n], XmNset, t->state()); n++;
	button = XtCreateManagedWidget(name.c_str(),
			xmToggleButtonWidgetClass, toolBar, args, n);
	XtAddCallback(button, XmNvalueChangedCallback, 
			(XtCallbackProc)toolButtonActivateCB, 
			(XtPointer)menubar_item);
	XtAddCallback(menubar_item->baseWidget(), XmNvalueChangedCallback,
			(XtCallbackProc)Sync2TogglesCB, (XtPointer)button);
    }
    else if(menubar_item->getMenuInstance())
    {
	Widget pulldown;
	n = 0;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNshadowThickness, 0); n++;
	XtSetArg(args[n], XmNpositionIndex, insert_position); n++;
	button = XmCreateMenuBar(toolBar, (char *)"_menuBar_", args, n);
	pulldown = menubar_item->baseWidget();

	/* turn off this option to avoid a warning. Turn back on below */
#if(XmVersion >= 2)
	XtSetArg(args[0], XmNtearOffModel, XmTEAR_OFF_DISABLED);
	XtSetValues(pulldown, args, 1);
#endif
	XtVaCreateManagedWidget(name.c_str(), xmCascadeButtonWidgetClass,button,
			XmNsubMenuId, pulldown,
			XmNmarginHeight, 0,
			XmNmarginWidth, 1,
			NULL);
	XtManageChild(button);
#if(XmVersion >= 2)
	XtSetArg(args[0], XmNtearOffModel, XmTEAR_OFF_ENABLED);
	XtSetValues(pulldown, args, 1);
#endif
    }
    if(button)
    {
	if(!menubar_item->isSensitive()) {
	    XtSetSensitive(button, False);
	}
	menubar_item->addActionListener(this, XtNsetSensitiveCallback);
	menubar_item->addActionListener(this, XtNsetVisibleCallback);
	menubar_item->addActionListener(this, XtNsetLabelCallback);

	WidgetThree p;
	p.menubar_item = menubar_item;
	p.item = button;
	p.remove_item = NULL;

	if(insert_position < 0 || insert_position>(int)toolbar_children.size()){
	    insert_position = (int)toolbar_children.size();
	    toolbar_children.push_back(p);
	}
	else {
	    toolbar_children.insert(toolbar_children.begin()+insert_position,p);
	}

	if(write_property) writeToolbarProperty();
    }
    return button;
}

/** Initialize the Add menu. The Add menu contains all of the items in the
 *  MenuBar, except the items that have already been added to the ToolBar.
 */
void ToolBar::addMenuInit(void)
{
    int i, num;
    Widget *children;
    Arg args[2];

    if(!menuBar) return;

    XtSetArg(args[0], XtNnumChildren, &num);
    XtSetArg(args[1], XtNchildren, &children);
    XtGetValues(addMenu, args, 2);

    for(i = 0; i < num; i++) {
	XtDestroyWidget(children[i]);
    }
    add_menu_items.clear();
    makeAddMenu(menuBar, addMenu);
}

/** Initialize the Remove menu. The Remove menu contains all of the items
 *  currently in the ToolBar.
 */
void ToolBar::removeMenuInit(void)
{
    int i, num;
    char *c1, *c2, name[200], new_name[200], *parent;
    Arg args[2];
    XmString xm, xm2;
    Widget *children, button;

    if(!menuBar) return;

    XtSetArg(args[0], XtNnumChildren, &num);
    XtSetArg(args[1], XtNchildren, &children);
    XtGetValues(removeMenu, args, 2);

    for(i = 0; i < num; i++) {
	XtDestroyWidget(children[i]);
    }

    for(i = 0; i < (int)toolbar_children.size(); i++)
    {
	if(XmIsRowColumn(toolbar_children[i].item)) {
	    int nc;
	    Widget *kids;
	    XtSetArg(args[0], XtNnumChildren, &nc);
	    XtSetArg(args[1], XtNchildren, &kids);
	    XtGetValues(toolbar_children[i].item, args, 2);
	    if(nc >= 1) {
		XtSetArg(args[0], XmNlabelString, &xm);
		XtGetValues(kids[0], args, 1);
	    }
	}
	else {
	    XtSetArg(args[0], XmNlabelString, &xm);
	    XtGetValues(toolbar_children[i].item, args, 1);
	}
	c1 = getXmString(xm);
	snprintf(new_name, sizeof(new_name), "_%s_", c1);

	parent = XtName(toolbar_children[i].menubar_item->widgetParent());

	XtSetArg(args[0], XmNlabelString, &xm);
	XtGetValues(toolbar_children[i].menubar_item->baseWidget(), args, 1);
	c2 = getXmString(xm);
	snprintf(name, sizeof(name), "%s", c1);
	xm = createXmString(name);
	snprintf(name, sizeof(name), "(%s/%s)", parent, c2);
	xm2 = createXmString(name);
	XtFree(c1);
	XtFree(c2);
	button = XtVaCreateManagedWidget(new_name,
				xmPushButtonWidgetClass, removeMenu,
				XmNlabelString, xm,
				XmNacceleratorText, xm2,
				NULL);
	toolbar_children[i].remove_item = button;
	XmStringFree(xm);
	XmStringFree(xm2);
	XtAddCallback(button, XmNactivateCallback,
			(XtCallbackProc)removeButtonCB, (XtPointer)this);
    }
}

/** Make the Add Menu. This function is called recusively for all PulldownMenu
 *  children of the MenuBar.
 *  @param[in] menubar the MenuBar to which the ToolBar is attached.
 *  @param[in] addmenu the XmPulldownMenu widget item in the MenuBar.
 *  returns the number of items added to the Add menu.
 */
int ToolBar::makeAddMenu(Component *menubar, Widget addmenu)
{
    int added_child = 0;
    char *name, new_name[100];
    int i, n;
    Arg args[2];
    XmString xm;
    vector<Component *> *children;
    Widget menu, w, button;

    if(!menubar || !addmenu) return 0;

    children = menubar->getChildren();

    for(i = 0; i < (int)children->size(); i++) if(children->at(i)->isVisible())
    {
	if(children->at(i)->getMenuInstance())
	{
	    name = (char *)children->at(i)->getName();
	    if(!strcmp(name, "Recent Files")) {
		continue;
	    }
	    if(!inToolBar(children->at(i)))
	    {
		snprintf(new_name, sizeof(new_name), "_%s_pulldown", name);
		w = XmCreatePulldownMenu(addmenu, new_name, NULL, 0);
		/* alter the name so there is no confusion with the real
		 * menubar children.
		 */
		snprintf(new_name, sizeof(new_name), "_%s_", name);
		xm = createXmString(name);
		menu = XtVaCreateManagedWidget(new_name,
				xmCascadeButtonWidgetClass, addmenu,
				XmNlabelString, xm,
				XmNsubMenuId, w, NULL);
		XmStringFree(xm);
		n = makeAddMenu(children->at(i), w);
		added_child += n;
		if(n > 1) {
		    XtCreateManagedWidget("_sep_", xmSeparatorWidgetClass,
				w, 0, 0);
		    button = XtVaCreateManagedWidget("Add This Menu",
				xmPushButtonWidgetClass, w, NULL);
		    WidgetPair p;
		    p.menubar_item = children->at(i);
		    p.item = button;
		    add_menu_items.push_back(p);

		    XtAddCallback(button, XmNactivateCallback, 
				(XtCallbackProc)addButtonCB, (XtPointer)this);
		}
		else if(!n) {
		    XtSetSensitive(menu, False);
		}
		added_child++;
	    }
	}
	else if(children->at(i)->getButtonInstance() ||
		children->at(i)->getToggleInstance())
	{
	    if(!inToolBar(children->at(i)))
	    {
		XtSetArg(args[0], XmNlabelString, &xm);
		XtGetValues(children->at(i)->baseWidget(), args, 1);
		name = getXmString(xm);
		snprintf(new_name, sizeof(new_name), "_%s_", name);
		XtFree(name);
		button = XtVaCreateManagedWidget(new_name,
				xmPushButtonWidgetClass,
				addmenu, XmNlabelString, xm,
				NULL);
		WidgetPair p;
		p.menubar_item = children->at(i);
		p.item = button;
		add_menu_items.push_back(p);
		XtAddCallback(button, XmNactivateCallback, 
				(XtCallbackProc)addButtonCB, (XtPointer)this);
		added_child++;
	    }
	}
    }
    delete children;
    return added_child;
}

/** Check if an item is currently contained in the ToolBar.
 *  @param[in] item the MenuBar child Component.
 *  @returns true if the item is contained in the ToolBar.
 */
bool ToolBar::inToolBar(Component *item)
{
    for(int i = 0; i < (int)toolbar_children.size(); i++) {
	if(toolbar_children[i].menubar_item == item) {
	    return true;
	}
    }
    return False;
}

/** Initialize the Set Accelerator menu.
 */
void ToolBar::accelMenuInit(void)
{
    int i, num;
    Widget *children;
    Arg args[2];

    if(!menuBar) return;

    XtSetArg(args[0], XtNnumChildren, &num);
    XtSetArg(args[1], XtNchildren, &children);
    XtGetValues(accelMenu, args, 2);

    for(i = 0; i < num; i++) {
	XtDestroyWidget(children[i]);
    }
    accel_menu_items.clear();
    makeAccelMenu(menuBar, accelMenu);
}

/** Make the Accelerator Menu. This function is called recusively for all
 *  PulldownMenu children of the MenuBar. All MenuBar items are added to the
 *  Accelerator Menu.
 *  @param[in] menubar the MenuBar to which the ToolBar is attached.
 *  @param[in] accelmenu the XmPulldownMenu widget item in the MenuBar.
 *  returns the number of items added to the Set Accelerator menu.
 */
int ToolBar::makeAccelMenu(Component *menubar, Widget accelmenu)
{
    int added_child = 0;
    char *name, new_name[100];
    int i, n;
    Arg args[2];
    XmString xm;
    vector<Component *> *children;
    Widget menu, w, button;

    if(!menubar || !accelmenu) return 0;

    children = menubar->getChildren();

    for(i = 0; i < (int)children->size(); i++) if(children->at(i)->isVisible())
    {
	if(children->at(i)->getMenuInstance())
	{
	    name = (char *)children->at(i)->getName();
	    if(!strcmp(name, "Recent Files")) {
		continue;
	    }
	    snprintf(new_name, sizeof(new_name), "_%s_pulldown", name);
	    w = XmCreatePulldownMenu(accelmenu, new_name, NULL, 0);
	    /* alter the name so there is no confusion with the real
	     * menubar children.
	     */
	    snprintf(new_name, sizeof(new_name), "_%s_", name);
	    xm = createXmString(name);
	    menu = XtVaCreateManagedWidget(new_name,
				xmCascadeButtonWidgetClass, accelmenu,
				XmNlabelString, xm,
				XmNsubMenuId, w, NULL);
	    XmStringFree(xm);
	    n = makeAccelMenu(children->at(i), w);
	    added_child += n;
	    if(!n) {
		XtSetSensitive(menu, False);
	    }
	    added_child++;
	}
	else if(children->at(i)->getButtonInstance() ||
		children->at(i)->getToggleInstance())
	{
	    XtSetArg(args[0], XmNlabelString, &xm);
	    XtGetValues(children->at(i)->baseWidget(), args, 1);
	    name = getXmString(xm);
	    snprintf(new_name, sizeof(new_name), "_%s_", name);
	    XtFree(name);
	    button = XtVaCreateManagedWidget(new_name,
				xmPushButtonWidgetClass,
				accelmenu, XmNlabelString, xm,
				NULL);
	    WidgetPair p;
	    p.menubar_item = children->at(i);
	    p.item = button;
	    accel_menu_items.push_back(p);
	    XtAddCallback(button, XmNactivateCallback, 
				(XtCallbackProc)accelButtonCB, (XtPointer)this);
	    added_child++;
	}
    }
    delete children;
    return added_child;
}

/** Change the label of a ToolBar item.
 *  @param[in] name the ToolBar item name.
 *  @param[in] label the new ToolBar item name.
 *  @returns true if the item is found.
 */
bool ToolBar::changeLabel(const string &name, const string &label)
{
    for(int i = 0; i < (int)toolbar_children.size(); i++) {
	if(!name.compare(XtName(toolbar_children[i].item))) {
	    Arg args[1];
	    XmString xm = createXmString(label);
	    XtSetArg(args[0], XmNlabelString, xm);
	    XtSetValues(toolbar_children[i].item, args, 1);
	    XmStringFree(xm);
	    return true;
	}
    }
    return false;
}

/** The callback for all items in the Add menu.
 */
void ToolBar::addButtonCB(Widget widget, XtPointer client, XtPointer callData)  
{
    ToolBar *toolBar = (ToolBar *)client;

    toolBar->getAddButton(widget);
}

/** Create the Add Button interface. This popup window allows the ToolBar item
 *  name and position to be specified. This function is a callback for each
 *  item in the ToolBar Add menu.
 *  @param[in] widget the item in the Add menu that was selected.
 */
void ToolBar::getAddButton(Widget widget)
{
    int i, len, min;
    char *name, *parent, position[10], label_string[200];
    Arg args[3];
    XmString xm;
    Widget label, rc, controls, ad, cancel, rc2, scale_w, sep;

    for(i = 0; i < (int)add_menu_items.size()
		&& add_menu_items[i].item != widget; i++);
    if(i == (int)add_menu_items.size()) return;

    adding_menu_item = add_menu_items[i].menubar_item;

    xm = createXmString("Toolbar Button Label");

    XtSetArg(args[0], XmNdialogTitle, xm);
    dialog = (Widget)XmCreateFormDialog(base_widget,
			(char *)"Toolbar Button Label", args, 1);
    XmStringFree(xm);


    XtSetArg(args[0], XmNlabelString, &xm);
    XtGetValues(adding_menu_item->baseWidget(), args, 1);
    name = getXmString(xm);
    if(name == NULL) name = strdup(XtName(adding_menu_item->baseWidget()));
    parent = XtName(adding_menu_item->widgetParent());
    snprintf(label_string, sizeof(label_string), "Toolbar Label for %s/%s",
			parent, name);
    len = strlen(name);
    for(i = len-1; i > 0 && name[i] == '.'; i--);
    name[i+1] = '\0';

    label = XtVaCreateManagedWidget(label_string, xmLabelWidgetClass,
			dialog,
			XmNtopAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNtopOffset, 5,
			XmNleftOffset, 5,
			XmNrightOffset, 5,
			XmNbottomOffset, 5,
			NULL);
    rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, dialog,
			XmNorientation, XmHORIZONTAL,
			XmNisAligned, True,
			XmNentryAlignment, XmALIGNMENT_END,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, label,
			XmNtopOffset, 5,
			XmNleftOffset, 5,
			XmNrightOffset, 5,
			NULL);

    XtVaCreateManagedWidget("Button Label", xmLabelWidgetClass, rc, NULL);

    label_text = XtVaCreateManagedWidget("label_text", xmTextWidgetClass, rc,
			XmNcolumns, 20,
			XmNvalue, name,
			NULL);
    XtFree(name);

    controls = XtVaCreateManagedWidget("controls", xmRowColumnWidgetClass,
			dialog,
			XmNorientation, XmHORIZONTAL,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			NULL);
    ad = XtVaCreateManagedWidget("Add Button", xmPushButtonWidgetClass,
			controls,
			NULL);
    XtAddCallback(ad, XmNactivateCallback, toolbarLabelCB, (XtPointer)this);

    cancel = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass,
			controls,
			NULL);
    XtAddCallback(cancel, XmNactivateCallback, cancelCB, (XtPointer)this);

    sep = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog,
			XmNorientation, XmHORIZONTAL,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, rc,
			XmNtopOffset, 10,
			XmNbottomAttachment, XmATTACH_WIDGET,
			XmNbottomWidget, controls,
			NULL);

    rc2 = XtVaCreateManagedWidget("rc2", xmRowColumnWidgetClass, rc,
			XmNorientation, XmHORIZONTAL,
			XmNisAligned, True,
			XmNentryAlignment, XmALIGNMENT_END,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, rc,
			XmNbottomAttachment, XmATTACH_WIDGET,
			XmNbottomWidget, sep,
			XmNleftOffset, 5,
			XmNrightOffset, 5,
			XmNtopOffset, 5,
			XmNbottomOffset, 5,
			NULL);
    XtVaCreateManagedWidget("Button Position", xmLabelWidgetClass, rc2, NULL);

    min = ((int)toolbar_children.size() > 0) ? 1 : 0;
    scale_w = XtVaCreateManagedWidget("scale", xmScaleWidgetClass, rc2,
			XmNorientation, XmHORIZONTAL,
			XmNscaleHeight, 20,
			XmNminimum, min,
			XmNmaximum, (int)toolbar_children.size()+1,
			XmNvalue, (int)toolbar_children.size()+1,
			XmNshowValue, False,
			NULL);
    XtAddCallback(scale_w, XmNdragCallback, scaleCB, (XtPointer)this);
    XtAddCallback(scale_w, XmNvalueChangedCallback, scaleCB, (XtPointer)this);

    snprintf(position, sizeof(position), "%d", (int)toolbar_children.size()+1);
    position_text = XtVaCreateManagedWidget("position_text", xmTextWidgetClass,
			rc2, XmNcolumns, 4,
			XmNeditable, True,
			XmNvalue, position,
			NULL);

    XtManageChild(dialog);
}

/** The callback for the Cancel button in the Toolbar Button Label popup window
 *  and also for the Cancel button in the Set Accelerator popup window.
 */
void ToolBar::cancelCB(Widget widget, XtPointer clientData, XtPointer callData)
{
    ToolBar *toolbar = (ToolBar *)clientData;
    XtUnmanageChild(toolbar->dialog);
    XtDestroyWidget(toolbar->dialog);
}

/** The Callback for the items in the Set Accelerator menu.
 */
void ToolBar::accelButtonCB(Widget widget, XtPointer client, XtPointer callData)
{
    ToolBar *toolBar = (ToolBar *)client;

    toolBar->getAccelButton(widget);
}

/** Create the Set Accelerator interface. This popup window allows the MenuBar
 *  item accelerator to be specified. This function is a callback for each
 *  item in the ToolBar Set Accelerator menu.
 *  @param[in] widget the item in the Set Accelerator menu that was selected.
 */
void ToolBar::getAccelButton(Widget widget)
{
    int i, len;
    char *name, lab[200];
    Arg args[3];
    XmString xm;
    Widget label, rc, controls, ad, cancel, sep, sw, label2;

    for(i = 0; i < (int)accel_menu_items.size()
		&& accel_menu_items[i].item != widget; i++);
    if(i == (int)accel_menu_items.size()) return;

    adding_menu_item = accel_menu_items[i].menubar_item;

    xm = createXmString("Set Accelerator");
    XtSetArg(args[0], XmNdialogTitle, xm);
    dialog = (Widget)XmCreateFormDialog(base_widget, (char *)"Set Accelerator",
			args,1);

    XtSetArg(args[0], XmNlabelString, &xm);
    XtGetValues(adding_menu_item->baseWidget(), args, 1);
    name = getXmString(xm);
    if(name == NULL) name = strdup(XtName(adding_menu_item->baseWidget()));
    len = strlen(name);
    for(i = len-1; i > 0 && name[i] == '.'; i--);
    name[i+1] = '\0';
    snprintf(lab, sizeof(lab), "Set Accelerator for: %s", name);
    XtFree(name);

    label = XtVaCreateManagedWidget(lab, xmLabelWidgetClass,
			dialog,
			XmNtopAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNtopOffset, 5,
			XmNleftOffset, 10,
			XmNrightOffset, 10,
			XmNbottomOffset, 5,
			NULL);

    controls = XtVaCreateManagedWidget("controls", xmRowColumnWidgetClass,
			dialog,
			XmNorientation, XmHORIZONTAL,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			NULL);
    ad = XtVaCreateManagedWidget("Set Accelerator", xmPushButtonWidgetClass,
			controls,
			NULL);
    XtAddCallback(ad, XmNactivateCallback, setAcceleratorCB, (XtPointer)this);

    cancel = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass,
			controls,
			NULL);
    XtAddCallback(cancel, XmNactivateCallback, cancelCB, (XtPointer)this);

    sep = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog,
			XmNorientation, XmHORIZONTAL,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_WIDGET,
			XmNbottomWidget, controls,
			NULL);
    rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, dialog,
			XmNorientation, XmHORIZONTAL,
			XmNisAligned, True,
			XmNentryAlignment, XmALIGNMENT_END,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, label,
			XmNtopOffset, 5,
			XmNleftOffset, 5,
			XmNrightOffset, 10,
			NULL);

    ctrl_toggle = XtVaCreateManagedWidget(" Ctrl ",xmToggleButtonWidgetClass,rc,
			XmNindicatorOn, XmINDICATOR_NONE,
			XmNspacing, 0,
			XmNmarginHeight, 0,
			XmNshadowThickness, 2,
			XmNvisibleWhenOff, True,
			NULL);
    alt_toggle = XtVaCreateManagedWidget(" Alt ", xmToggleButtonWidgetClass,rc,
			XmNindicatorOn, XmINDICATOR_NONE,
			XmNspacing, 0,
			XmNmarginHeight, 0,
			XmNshadowThickness, 2,
			XmNvisibleWhenOff, True,
			NULL);
    XtVaCreateManagedWidget("  Key:", xmLabelWidgetClass, rc, NULL); 

    label_text = XtVaCreateManagedWidget("label_text", xmTextWidgetClass, rc,
			XmNcolumns, 2,
			NULL);

    label2 = XtVaCreateManagedWidget("Target Windows", xmLabelWidgetClass,
			dialog,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, rc,
			XmNtopOffset, 5,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			NULL);
    sw = XtVaCreateManagedWidget("sw", xmScrolledWindowWidgetClass, dialog,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, label2,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			XmNbottomAttachment, XmATTACH_WIDGET,
			XmNbottomWidget, sep,
			XmNbottomOffset, 5,
			NULL);

    target_list = XtVaCreateManagedWidget("target_list", xmListWidgetClass, sw,
			XmNselectionPolicy, XmMULTIPLE_SELECT,
			XmNvisibleItemCount, 5,
			NULL);
    XtAddCallback(target_list, XmNmultipleSelectionCallback, targetListCB,
			(XtPointer)this);

    vector<Component *> *v = Application::getApplication()->getWindows();

    XmString *xms = (XmString *)malloc((v->size()+1)*sizeof(XmString));
    xms[0] = createXmString((char *)comp_parent->getName());
    int n = 1;

    for(i = 0; i < (int)v->size(); i++) {
	name = (char *)v->at(i)->getName();
	if(strcasecmp(comp_parent->getName(), name)) {
	    xms[n++] = createXmString(name);
	}
    }
    XmListAddItems(target_list, xms, n, 1);

    XmListSelectItem(target_list, xms[0], false);

    for(i = 0; i < n; i++) XmStringFree(xms[i]);
    Free(xms);

    XtManageChild(dialog);
}

/** The callback for the XmList in the Set Accelerator popup window.
 */
void ToolBar::targetListCB(Widget widget, XtPointer clientData,
			XtPointer callData)  
{
    ToolBar *toolbar = (ToolBar *)clientData;
    int i, *selected=NULL, nselected;

    // Force the parent window (first item) to be selected.


    XmListGetSelectedPos(toolbar->target_list, &selected, &nselected);

    for(i = 0; i < nselected; i++) {
	if(selected[i] == 1) break;
    }
    Free(selected);
    if(i == nselected) {
	XmListSelectPos(toolbar->target_list, 1, False);
    }
}

/** The callback for the XmScale in the Toolbar Button Label popup window.
 */
void ToolBar::scaleCB(Widget widget, XtPointer clientData, XtPointer callData)  
{
    ToolBar *toolbar = (ToolBar *)clientData;
    toolbar->scale((XmScrollBarCallbackStruct *)callData);
}

/** The callback for the XmScale in the Toolbar Button Label popup window.
 *  Update the insert position text.
 */
void ToolBar::scale(XmScrollBarCallbackStruct *s)
{
    char text[50];
    if((int)toolbar_children.size() > 0) {
	snprintf(text, sizeof(text), "%d", (int)s->value);
	XmTextSetString(position_text, text);
    }
    else {
	XmTextSetString(position_text, (char *)"1");
    }
}

/** The callback for the Add Button in the Toolbar Button Label popup window.
 */
void ToolBar::toolbarLabelCB(Widget widget, XtPointer client, XtPointer data)
{
    ToolBar *toolbar = (ToolBar *)client;
    toolbar->toolbarLabel();
}

/** The callback for the Add Button in the Toolbar Button Label popup window.
 *  Add the item to the ToolBar.
 */
void ToolBar::toolbarLabel(void)
{
    char *name, *position;
    int pos;

    name = XmTextGetString(label_text);

    stringTrim(name);
    if(name[0] == '\0') {
	XtFree(name);
	name = strdup(XtName(adding_menu_item->baseWidget()));
    }

    position = XmTextGetString(position_text);
    if(sscanf(position, "%d", &pos) != 1 || pos < 1 ||
	pos > (int)toolbar_children.size()+1)
    {
	pos = (int)toolbar_children.size() + 1;
    }
    XtFree(position);
    pos--;

    addButton(adding_menu_item, name, pos, True);
    XtFree(name);

    XtUnmanageChild(dialog);
    XtDestroyWidget(dialog);

    layout();
}

/** The callback for the Set Accelerator button in the Set Accelerator popup
 *  window.
 */
void ToolBar::setAcceleratorCB(Widget widget, XtPointer client, XtPointer data)
{
    ToolBar *toolbar = (ToolBar *)client;
    toolbar->setAccelerator();
}

/** The callback for the Set Accelerator button in the Set Accelerator popup
 *  window.
 */
void ToolBar::setAccelerator(void)
{
    char *name, s[100], label[100], key;
    Arg args[2];

    name = XmTextGetString(label_text);

    stringTrim(name);
    if(name[0] == '\0') {
	XtFree(name);
	return;
    }
    key = name[0];
    XtFree(name);

    XmString *selectedItems;
    int n, num_selected;

    n = 0;
    XtSetArg(args[n], XmNselectedItems, &selectedItems); n++;
    XtSetArg(args[n], XmNselectedItemCount, &num_selected); n++;
    XtGetValues(target_list, args, n);

    if(num_selected <= 0) return;


    Boolean ctrl = XmToggleButtonGetState(ctrl_toggle);
    Boolean alt = XmToggleButtonGetState(alt_toggle);

    if(ctrl && !alt) {
	snprintf(s, sizeof(s), "#override : ~Alt Ctrl<Key>%c: ArmAndActivate()",
			key);
	snprintf(label, sizeof(label), "Ctrl+%c", key);
    }
    else if(!ctrl && alt) {
	snprintf(s, sizeof(s), "#override :~Ctrl Alt<Key>%c: ArmAndActivate()",
			key);
	snprintf(label, sizeof(label), "Alt+%c", key);
    }
    else if(ctrl && alt) {
	snprintf(s, sizeof(s), "#override :Ctrl Alt<Key>%c: ArmAndActivate()",
			key);
	snprintf(label, sizeof(label), "Ctrl Alt+%c", key);
    }
    else {
	snprintf(s, sizeof(s), "#override :~Ctrl ~Alt<Key>%c: ArmAndActivate()",
			key);
	snprintf(label, sizeof(label), "%c", key);
    }
    XtAccelerators a = XtParseAcceleratorTable(s);
    XtSetArg(args[0], XmNaccelerators, a);
    XmString xm = createXmString(label);
    XtSetArg(args[1], XmNacceleratorText, xm);
    XtSetValues(adding_menu_item->baseWidget(), args, 2);
    XmStringFree(xm);

    comp_parent->installAccelerators(adding_menu_item);

    char prop_name[200], prop[2000];
    snprintf(prop_name, sizeof(prop_name), "%s:%s:accelerator",
                comp_parent->getName(), adding_menu_item->getName());
    snprintf(prop, sizeof(prop), "%s%s",  label, s);

    putProperty(prop_name, prop);

    memset(prop, 0, sizeof(prop));

    for(int i = 0; i < num_selected; i++) {
	char *c = getXmString(selectedItems[i]);
	if( strcasecmp(c, comp_parent->getName()) ) {
	    n = strlen(prop);
	    snprintf(prop+n, sizeof(prop)-n, "%s,", c);

	    Component *comp = Application::getApplication()->getWindow(c);
	    if(comp) {
		comp->installAccelerators(adding_menu_item);
	    }
	}
	XtFree(c);
    }
    n = strlen(prop);
    if(n > 0) prop[n-1] = '\0'; // remove the last ',';
    snprintf(prop_name, sizeof(prop_name), "%s:%s:destinations",
                comp_parent->getName(), adding_menu_item->getName());
    putProperty(prop_name, prop);

    XtUnmanageChild(dialog);
    XtDestroyWidget(dialog);
}

/** The callback for the items in the Remove menu.
 */
void ToolBar::removeButtonCB(Widget widget, XtPointer client, XtPointer data)  
{
    ToolBar *toolbar = (ToolBar *)client;
    toolbar->removeButton(widget);
    toolbar->layout();
}

/** The callback for the items in the Remove menu.
 *  @param[in] widget the item in the Remove menu.
 */
void ToolBar::removeButton(Widget widget)
{
    int i;
    Widget tool_button;

    for(i = 0; i < (int)toolbar_children.size()
	&& toolbar_children[i].remove_item != widget; i++);
    if(i == (int)toolbar_children.size()) return;
    tool_button = toolbar_children[i].item;

    if(toolbar_children[i].menubar_item->getToggleInstance()) {
	XtRemoveCallback(toolbar_children[i].menubar_item->baseWidget(),
		 XmNvalueChangedCallback, Sync2TogglesCB,
			toolbar_children[i].item);
    }

    toolbar_children.erase(toolbar_children.begin()+i);

    writeToolbarProperty();

    XtDestroyWidget(tool_button);
}

/** Remove all items from the ToolBar.
 */
void ToolBar::removeAllItems(void)
{
    for(int i = 0; i < (int)toolbar_children.size(); i++) {
	if(toolbar_children[i].menubar_item->getToggleInstance()) {
	    XtRemoveCallback(toolbar_children[i].menubar_item->baseWidget(),
			XmNvalueChangedCallback, Sync2TogglesCB,
			toolbar_children[i].item);
	}
	XtDestroyWidget(toolbar_children[i].item);
    }
    toolbar_children.clear();
}

/** Resize callback.
 */
void ToolBar::
resizeCB(Widget widget, XtPointer client_data, XEvent *event, Boolean *cont)
{
    ToolBar *toolbar = (ToolBar *)client_data;
    toolbar->showArrows();
}

/** Show the ToolBar arrows. When the ToolBar is not wide enough to display
 *  all of its items, two arrow buttons are drawn on the right side of the
 *  ToolBar. These arrow buttons scroll the ToolBar items left or right.
 */
void ToolBar::showArrows(void)
{
    Widget scroll;
    int value, slider_size, incr, page_incr, maximum;
    Dimension toolBar_width, toolBar_edit_width, width;
    Arg args[3];

    XtSetArg(args[0], XmNhorizontalScrollBar, &scroll);
    XtGetValues(toolWindow, args, 1);

    XtSetArg(args[0], XmNwidth, &toolBar_width);
    XtGetValues(toolBar, args, 1);

    if(menuBar) {
	XtSetArg(args[0], XmNwidth, &toolBar_edit_width);
	XtGetValues(toolBar_edit, args, 1);
    }
    else {
	toolBar_edit_width = 0;
    }

    XtSetArg(args[0], XmNwidth, &width);
    XtGetValues(base_widget, args, 1);

    if(toolBar_width > width - toolBar_edit_width)
    {
	if(!XtIsManaged(arrow_Right) || !XtIsManaged(arrow_Left)) {
	    XtManageChild(arrow_Right);
	    XtManageChild(arrow_Left);
	    XtSetArg(args[0], XmNrightAttachment, XmATTACH_WIDGET);
	    XtSetArg(args[1], XmNrightWidget, arrow_Left);
	    XtSetValues(toolWindow, args, 2);
	    XmScrollBarGetValues(scroll, &value, &slider_size,&incr,&page_incr);
	    value = 0;
	    XmScrollBarSetValues(scroll, value, slider_size, incr,
				page_incr, True);
	    XtSetSensitive(arrow_Left, False);
	}
	XtSetArg(args[0], XmNvalue, &value);
	XtSetArg(args[1], XmNmaximum, &maximum);
	XtSetArg(args[2], XmNsliderSize, &slider_size);
	XtGetValues(scroll, args, 3);
	if(value < maximum - slider_size) {
	    XtSetSensitive(arrow_Right, True);
	}
    }
    else {
	if(XtIsManaged(arrow_Right) || XtIsManaged(arrow_Left)) {
	    XtSetArg(args[0], XmNrightAttachment, XmATTACH_FORM);
	    XtSetValues(toolWindow, args, 1);
	    XtUnmanageChild(arrow_Left);
	    XtUnmanageChild(arrow_Right);
	    XmScrollBarGetValues(scroll, &value, &slider_size,&incr,&page_incr);
	    value = 0;
	    XmScrollBarSetValues(scroll, value, slider_size, incr,
				page_incr, True);
	}
    }
}

/** Update the ToolBar layout.
 */
void ToolBar::layout(void)
{
    Dimension edit_height = (menuBar) ? get_Height(toolBar_edit) : 0;
    Dimension arrow_height = XtIsManaged(arrow_Right) ?
				get_Height(arrow_Right) : 0;
    Dimension margin_height, height, margin;
    Dimension button_height = 0;
    Arg args[1];

    margin = 0;
    margin_height = 0;
    XtSetArg(args[0], XmNmarginHeight, &margin_height);
    XtGetValues(toolBar, args, 1);
    margin += 2*margin_height;
    margin_height = 0;
    XtGetValues(toolWindow, args, 1);
    margin += 2*margin_height;
    margin_height = 0;
    XtGetValues(base_widget, args, 1);
    edit_height += 2*margin_height;
    arrow_height += 2*margin_height;
    margin += 2*margin_height;

    for(int i = 0; i < (int)toolbar_children.size(); i++) {
	Dimension h = get_Height(toolbar_children[i].item);
	if(h > button_height) button_height = h;
    }
    button_height += margin;

    height = edit_height;
    if(arrow_height > height) height = arrow_height;
    if(button_height > height) height = button_height;

    if(height != get_Height(toolWindow) || height != get_Height(base_widget) ||
		height != get_Height(toolBar))
    {
	XtUnmanageChild(base_widget);
	XtUnmanageChild(toolWindow);
	XtUnmanageChild(toolBar);
	set_Height(base_widget, height);
	set_Height(toolWindow, height);
	set_Height(toolBar, height);
	XtManageChild(toolBar);
	XtManageChild(toolWindow);
	XtManageChild(base_widget);
    }
}

/** Callback for all items in the ToolBar.
 */
void ToolBar::toolButtonActivateCB(Widget widget, XtPointer client_data,
			XtPointer data)
{
    Component *menu_item = (Component *)client_data;   
    Toggle *t;

    if(menu_item->getButtonInstance()) {
	XtCallCallbacks(menu_item->baseWidget(), XmNactivateCallback, NULL);
    }
    else if( (t = menu_item->getToggleInstance()) ) {
	t->set(XmToggleButtonGetState(widget), true);
    }
}

/** Callback added to a MenuBar Toggle to synchronize its state with its
 *  ToolBar counterpart.
 */
void ToolBar::Sync2TogglesCB(Widget widget, XtPointer client, XtPointer data)
{
    Widget  w = (Widget)client;

    XmToggleButtonSetState(w, XmToggleButtonGetState(widget), False);
}

/** Scroll right arrow callback.
 */
void ToolBar::arrowRightCB(Widget, XtPointer clientData, XtPointer data)
{
    ToolBar *toolBar = (ToolBar *)clientData;
    toolBar->arrowRight();
}

/** Scroll right arrow callback.
 */
void ToolBar::arrowRight(void)
{
    Arg args[4];
    int value, minimum, maximum, slider_size, incr, page_incr;
    Widget scroll;

    XtSetArg(args[0], XmNhorizontalScrollBar, &scroll);
    XtGetValues(toolWindow, args, 1);

    XtSetArg(args[0], XmNvalue, &value);
    XtSetArg(args[1], XmNminimum, &minimum);
    XtSetArg(args[2], XmNmaximum, &maximum);
    XtSetArg(args[3], XmNsliderSize, &slider_size);
    XtGetValues(scroll, args, 4);

    XmScrollBarGetValues(scroll, &value, &slider_size, &incr, &page_incr);

    if(value < maximum - slider_size) {
	value += 20;
	if(value > maximum - slider_size) {
	    value = maximum - slider_size;
	}
	XmScrollBarSetValues(scroll, value, slider_size, incr, page_incr, True);
	if(value > 0) {
	    XtSetSensitive(arrow_Left, True);
	}
	if(value >= maximum - slider_size) {
	    XtSetSensitive(arrow_Right, False);
	}
    }
}

/** Scroll left arrow callback.
 */
void ToolBar::arrowLeftCB(Widget, XtPointer clientData, XtPointer data)
{
    ToolBar *toolBar = (ToolBar *)clientData;
    toolBar->arrowLeft();
}

/** Scroll left arrow callback.
 */
void ToolBar::arrowLeft(void)
{
    Arg args[4];
    int value, minimum, maximum, slider_size, incr, page_incr;
    Widget scroll;

    XtSetArg(args[0], XmNhorizontalScrollBar, &scroll);
    XtGetValues(toolWindow, args, 1);

    XtSetArg(args[0], XmNvalue, &value);
    XtSetArg(args[1], XmNminimum, &minimum);
    XtSetArg(args[2], XmNmaximum, &maximum);
    XtSetArg(args[3], XmNsliderSize, &slider_size);
    XtGetValues(scroll, args, 4);

    XmScrollBarGetValues(scroll, &value, &slider_size, &incr, &page_incr);

    if(value > 0) {
	value -= 20;
	if(value < 0) value = 0;
	XmScrollBarSetValues(scroll, value, slider_size, incr, page_incr, True);
	if(value <= 0) {
	    XtSetSensitive(arrow_Left, False);
	}
	if(value < maximum - slider_size) {
	    XtSetSensitive(arrow_Right, True);
	}
    }
}

/** Save the current ToolBar contents as a program property.
 */
void ToolBar::writeToolbarProperty(void)
{
    char *value = NULL, prop_name[200];
    Component *w, *tree[100];

    if(!menuBar) return;

    snprintf(prop_name, sizeof(prop_name), "%s.%s.toolBar",
		getParent()->getName(), getName());

    value = (char *)malloc(1);
    value[0] = '\0';
    int n = 0;
    for(int i = 0; i < (int)toolbar_children.size(); i++)
    {
	int m = 0;
	for(w = toolbar_children[i].menubar_item; w != NULL; w = w->getParent())
	{
	    if(w == menuBar) break;
	    n += (int)strlen(w->getName()) + 1;
	    tree[m++] = w;
	}
	n += (int)strlen(buttonName(toolbar_children[i].item)) + 2;

	value = (char *)realloc(value, n+1);

	for(int j = m-1; j >= 0; j--) {
	    strcat(value, tree[j]->getName());
	    if(j > 0) strcat(value, ".");
	}
	strcat(value, ",");
	strcat(value, buttonName(toolbar_children[i].item));
	if(i < (int)toolbar_children.size()-1) strcat(value, ",");
    }
    putProperty(prop_name, value);
//    writeApplicationProperties();
    free(value);
}

/** Get the name of a ToolBar item.
 *  @param[in] the item's widget
 */
char * ToolBar::buttonName(Widget w)
{
    static char name[100];

    if(XmIsRowColumn(w))
    {
	int nc;
	char *c;
	Arg args[2];
	Widget *kids;
	XmString xm;

	XtSetArg(args[0], XtNnumChildren, &nc);
	XtSetArg(args[1], XtNchildren, &kids);
	XtGetValues(w, args, 2);
	if(nc >= 1) {
	    XtSetArg(args[0], XmNlabelString, &xm);
	    XtGetValues(kids[0], args, 1);
	    c = getXmString(xm);
	    if(c != NULL) {
		stringcpy(name, c, sizeof(name));
		XtFree(c);
	    }
	    else {
		stringcpy(name, "-", sizeof(name));
	    }
	}
	else {
	    stringcpy(name, "-", sizeof(name));
	}
	return name;
    }
    else {
	return XtName(w);
    }
}

/** Get the height of a Widget.
 *  @param[in] widget
 *  @returns the height in pixels.
 */
static int get_Height(Widget widget)
{
    Arg args[1];
    Dimension height = 0;

    XtSetArg(args[0], XmNheight, &height);
    XtGetValues(widget, args, 1);
    return (int)height;
}

/** Set the height of a component.
 *  @param[in] widget
 *  @param[in] height the width in pixels.
 */
static void set_Height(Widget widget, int height)
{
    Arg args[1];
    XtSetArg(args[0], XmNheight, height);
    XtSetValues(widget, args, 1);
}
