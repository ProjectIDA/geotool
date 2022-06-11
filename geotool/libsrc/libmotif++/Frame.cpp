/** \file Frame.cpp
 *  \brief Defines class Frame.
 *  \author Ivan Henson
 */
#include "config.h"
#include <assert.h>
#include <iostream>

#include "motif++/Frame.h"
#include "motif++/Question.h"
#include "motif++/Application.h"

#ifdef EWMH_DEBUG
#undef EWMH_DEBUG
#endif

/** Constructor.
 *  @param[in] name name given to this Frame instance.
 *  @param[in] parent the Component parent.
 *  @param[in] title the title displayed on the top window border.
 *  @param[in] pointer_focus If true, set keyboardFocusPolicy to XmPOINTER,
 *		otherwise it is XmEXPLICIT
 *  @param[in] independent if true, the widget parent is the display parent
 *              instead of the parent base widget.
 */
Frame::Frame(const string &name, Component *parent, const string &title,
	bool pointer_focus, bool independent) :
	TopWindow(name, parent, title, pointer_focus, independent)
{
    init();
}

/** Constructor.
 *  @param[in] name name given to this Frame instance.
 *  @param[in] parent the Component parent.
 *  @param[in] pointer_focus If true, set keyboardFocusPolicy to XmPOINTER,
 *		otherwise it is XmEXPLICIT
 *  @param[in] independent if true, the widget parent is the display parent
 *              instead of the parent base widget.
 */
Frame::Frame(const string &name, Component *parent, bool pointer_focus,
	bool independent) : TopWindow(name, parent, pointer_focus, independent)
{
    init();
}

/** Create the interface.
 */
void Frame::init(void)
{
    int n;
    Arg args[4];

    // need XmRESIZE_NONE for correct toolbar behavior
    n = 0;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_NONE);
    setValues(args, 1);

    // Use a Motif XmFrame widget to handle window layout
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    main_window = new MainWindow("mainWindow", this, args, n, false);

    work_form = new Form("workArea", main_window, false);

    // Designate the workArea widget as the XmMainWindow widget's
    // XmNworkArea widget

    XtVaSetValues(main_window->baseWidget(), XmNworkWindow,
			work_form->baseWidget(), NULL);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    frame_form = new Form("frameForm", work_form, args, n, false);

    menu_bar = NULL;
    tool_bar = NULL;
    info_area = NULL;
    file_menu = NULL;
    edit_menu = NULL;
    view_menu = NULL;
    option_menu = NULL;
    help_menu = NULL;
}

/** Destructor.
 */
Frame::~Frame(void)
{
}

void Frame::setVisible(bool visible)
{
    assert(base_widget);

    if(visible) {
	if(!XtIsManaged(main_window->baseWidget())) {
	    XtManageChild(main_window->baseWidget());
	    if(tool_bar) {
		tool_bar->setVisible(true);
	    }
	    if(info_area) {
		info_area->setVisible(true);
	    }
	    XtManageChild(frame_form->baseWidget());
	    XtManageChild(work_form->baseWidget());
	    if(menu_bar) {
		installAccelerators(menu_bar);
	    }
        }
	else {
	  popup_activate(XtGrabNone, 2);
	}
    }
    TopWindow::setVisible(visible);
}

void Frame::HandleWindowDel(Widget w, XtPointer client, XEvent *event,
		Boolean *dispatch)
{
cout << "inside HandleWindowDel\n";
    *dispatch = False;
}

void Frame::iconify(void)
{
    assert(base_widget);

    // Set the widget to have an initial iconic state in case the base
    // widget has not yet been realized

    XtVaSetValues(base_widget, XmNiconic, TRUE, NULL);

    // If the widget has already been realized, iconify it

    if(XtIsRealized(base_widget)) {
	XIconifyWindow(XtDisplay(base_widget), XtWindow(base_widget), 0);
    }
}

/*
 * NAME
 *	popup_activate - Popup and activate (de-iconify, raise, focus) window
 *
 * SYNOPSIS
 *	This method contains routines for window management using
 *      Extended Window Manager Hints (EWMH)
 *
 * DESCRIPTION
 *
 *	popup_activate - popup the shell widget using XtPopup() and send a
 *                       _NET_ACTIVE_WINDOW client message event to activate 
 *                       (de-iconify, raise, focus) the shell's window.
 *
 *	bool
 *	popup_activate (grab_kind, source_indication)
 *      XtGrabKind      grab_kind;
 *      unsigned int    source_indication;   1 for normal application, 2 for pager
 *
 * DIAGNOSTICS
 *	Returns true if successfully sent X Client Event, false otherwise.
 *
 * AUTHOR
 *	Martin Ertl     Jun 22, 2009  Initial version. Tested with GNOME/metacity
 *                                    version 2.16.0 of RHEL 5.2 and 5.3.
 *                                    Caveats: a) does not check WM capabilities, and
 *                                    b) does not send timestamp of the event 
 *                                    triggering the window activation.
 *                                    If GNOME development decides to change things,
 *                                    this implementation may need update, too.
 *      Vera Miljanovic Feb 11, 2011  Converted original C function from
 *                                    ibase/libXcss/window_management.c to C++ method
 *      Vera Miljanovic Feb 15, 2011  Made method more object-oriented by removing
 *                                    popup_shell argument
 */
bool Frame::popup_activate(XtGrabKind grab_kind, unsigned int source_indication)
{
	Widget popup_shell = main_window->shellWidget();

	Display *disp;
	Window win;
	XEvent event;
	int mask = SubstructureRedirectMask | SubstructureNotifyMask;

	/* Map the popup shell */
	XtPopup(popup_shell, grab_kind);

#ifdef EWMH_DEBUG
	fprintf(stderr, "popup_activate: preparing _NET_ACTIVE_WINDOW event.\n");
#endif

	/* Get the display */
        disp = XtDisplay(popup_shell);
	if (!disp) {
	  fprintf(stderr, "popup_activate: failed to get display.\n");
	  return false;
	}

	/* Get the window */
	win = XtWindow(popup_shell);
 	if (!win) {
	  fprintf(stderr, "popup_activate: failed to get window.\n");
	  return false;
	}

	/* Fill XClientMessageEvent structure */
	event.xclient.type = ClientMessage;
	event.xclient.serial = 0;
	event.xclient.send_event = True;
	event.xclient.message_type = XInternAtom(disp, "_NET_ACTIVE_WINDOW", False);
	event.xclient.window = win;
	event.xclient.format = 32;
	event.xclient.data.l[0] = source_indication;
	event.xclient.data.l[1] = 0;
	event.xclient.data.l[2] = 0;
	event.xclient.data.l[3] = 0;
	event.xclient.data.l[4] = 0;

	/* Activate window */
#ifdef EWMH_DEBUG
	fprintf (stderr, "popup_activate: activating window id: 0x%.8lx\n", win);
#endif
	if (!XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event)) {
	  fprintf(stderr, "popup_activate: failed sending _NET_ACTIVE_WINDOW event.\n");
	  return false;
	}

#ifdef EWMH_DEBUG
	fprintf(stderr, "popup_activate: successfully sent _NET_ACTIVE_WINDOW event.\n");
#endif
	return true;
}

/** Used by the MenuBar class.
 */
void Frame::setMenuBar(MenuBar *menubar)
{
    menu_bar = menubar;
    XtVaSetValues(main_window->baseWidget(), XmNmenuBar,
			menubar->baseWidget(), NULL);
    if(!XtIsManaged(menubar->baseWidget())) {
	XtManageChild(menubar->baseWidget());
    }
}

/** Used by the ToolBar class.
 */
void Frame::setToolBar(ToolBar *toolbar)
{
    if(tool_bar) {
	tool_bar->setVisible(false);
    }

    tool_bar = toolbar;

    Arg args[4];
    int n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, tool_bar->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    separator = new Separator("separator", work_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, separator->baseWidget()); n++;
    frame_form->setValues(args, n);
}

/** Used by the InfoArea class.
 */
void Frame::setInfoArea(InfoArea *infoarea)
{
    if(info_area) {
	info_area->setVisible(false);
    }

    info_area = infoarea;

    Arg args[2];
    int n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, info_area->baseWidget()); n++;
    frame_form->setValues(args, n);
}

void Frame::createInterface(void)
{
    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    edit_menu = new Menu("Edit", menu_bar);
    view_menu = new Menu("View", menu_bar);
    option_menu = new Menu("Option", menu_bar);
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
}

/** Stop the application with a confirm dialog.
 *  @param[in] confirm it true, a confirm dialog will be displayed allowing
 * 	the user to cancel the quit.
 */
void Frame::quit(bool confirm)
{
    if(!confirm) {
	setVisible(false);
	Application::getApplication()->stop(0);
    }
    else {
	if(Application::getApplication()->numVisibleWindows() > 1) {
	    int ans = Question::askQuestion("Confirm", this,
			"Close All Windows and Quit?", "Yes", "Cancel");
	    if(!ans || ans== 2) {
		return;
	    }
	    else {
		setVisible(false);
		Application::getApplication()->stop(0);
	    }
	}
	else {
	    setVisible(false);
	    Application::getApplication()->stop(0);
	}
    }
}

ParseCmd Frame::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string s;

    if(parseCompare(cmd, "listChildren") || parseCompare(cmd,"listCallbacks")) {
	return Component::parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "addToToolbar")) {
	msg.assign("addToToolbar: missing arguments.");
	return ARGUMENT_ERROR;
    }
    else if(parseArg(cmd, "addToToolbar", s)) {
	return addToToolbar(s, msg);
    }
    else if(frame_form && (ret = frame_form->parseCmd(cmd, msg))
		!= COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else if(menu_bar && (ret = menu_bar->parseCmd(cmd, msg))
		!= COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else if((ret = TopWindow::parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
	return ret;
    }
    return COMMAND_NOT_FOUND;
}

ParseVar Frame::parseVar(const string &name, string &value)
{
    ParseVar ret;

    if(frame_form && (ret = frame_form->parseVar(name, value))
		!= VARIABLE_NOT_FOUND)
    {
	return ret;
    }
    else if(menu_bar && (ret = menu_bar->parseVar(name, value))
		!= VARIABLE_NOT_FOUND)
    {
	return ret;
    }
    else if((ret = TopWindow::parseVar(name, value)) != VARIABLE_NOT_FOUND)
    {
	return ret;
    }
    return VARIABLE_NOT_FOUND;
}

ParseCmd Frame::addToToolbar(const string &cmd, string &msg)
{
    string child_name, toolbar_name;
    Component *comp;
    int pos = -1;

    if( !tool_bar ) {
	msg.assign("addToToobar: There is no toolbar in this window.");
	return ARGUMENT_ERROR;
    }
    if( !menu_bar ) {
	msg.assign("addToToobar: There is no menubar in this window.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(cmd, "name", child_name) ) {
	msg.assign("addToToobar: missing 'name' argument.");
	return ARGUMENT_ERROR;
    }
    if( (comp = menu_bar->getMenu(child_name)) ) {
    }
    else if( (comp = menu_bar->getButton(child_name)) ) {
    }
    else if( (comp = menu_bar->getToggle(child_name)) ) {
    }
    else
    {
	msg.assign(string("addToToobar: ") + child_name + " not found.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(cmd, "toolbar_name", toolbar_name) ) {
	toolbar_name.assign(child_name);
    }

    stringGetIntArg(cmd.c_str(), "position", &pos);
    pos--; // this is so pos=1 will insert the new item as the first item

    if(pos < 0) {
	pos = (int)XmLAST_POSITION;
    }
    tool_bar->add(comp, toolbar_name, pos);

    return COMMAND_PARSED;
}
