/** \file FormDialog.cpp
 *  \brief Defines class FormDialog.
 *  \author Ivan Henson
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "config.h"
#include "widget/Table.h"
#include "motif++/FormDialog.h"
#include "motif++/Question.h"
#include "motif++/ListDialog.h"
#include "motif++/Application.h"
#include <Xm/Protocols.h>
#include <X11/Xmu/Editres.h>

static void deleteWindowCB(Widget, XtPointer, XtPointer);

/** Constructor.
 *  @param[in] name name given to this FormDialog instance.
 *  @param[in] parent the Component parent.
 *  @param[in] title the title displayed on the window border.
 *  @param[in] pointer_focus If true, set keyboardFocusPolicy to XmPOINTER,
 * 		otherwise it is XmEXPLICIT
 *  @param[in] independent if true, the widget parent is the display parent
 *		instead of the base widget of the Component parent.
 */
FormDialog::FormDialog(const string &name, Component *parent,
		const string &title, bool pointer_focus, bool independent) :
		Component(name, parent)
{
    init(title, pointer_focus, independent);
}

/** Constructor.
 *  @param[in] name name given to this FormDialog instance.
 *  @param[in] parent the Component parent.
 *  @param[in] pointer_focus If true, set keyboardFocusPolicy to XmPOINTER,
 * 		otherwise it is XmEXPLICIT
 *  @param[in] independent if true, the widget parent is the display parent
 *		instead of the base widget of the Component parent.
 */
FormDialog::FormDialog(const string &name, Component *parent,
		bool pointer_focus, bool independent) : Component(name, parent)
{
    string title;
    init(title, pointer_focus, independent);
}

/** Initialize the class.
 *  @param[in] title the title displayed on the window border.
 *  @param[in] independent
 */
void FormDialog::init(const string &title, bool pointer_focus, bool independent)
{
    int n;
    char *ctitle;
    Arg args[10];
    XmString xm;
    Widget shell;

    Application::getApplication()->registerWindow(this);

    indep = independent;
    positioned = false;
    if( !title.empty() ) {
	ctitle = (char *)title.c_str();
    }
    else {
	ctitle = (char *)getName();
    }

    if(independent) {
	Widget parent = Application::displayParent(getName());
	char shell_name[200];
	snprintf(shell_name, sizeof(shell_name), "%s_shell", getName());

	n = 0;
//	XtSetArg(args[n], XmNdialogTitle, xm); n++;
	XtSetArg(args[n], XmNautoUnmanage, False); n++;
	XtSetArg(args[n], XmNallowShellResize, True); n++;
	XtSetArg(args[n], XmNmwmDecorations, MWM_DECOR_ALL); n++;
	XtSetArg(args[n], XmNmappedWhenManaged, False); n++;
	XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
//	XtSetArg(args[n], XmNwindowGroup, XtUnspecifiedWindowGroup); n++;
	if(pointer_focus) {
	    XtSetArg(args[n], XmNkeyboardFocusPolicy, XmPOINTER); n++;
	}
	shell = XtCreatePopupShell(ctitle, vendorShellWidgetClass,
				parent, args, n);
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNautoUnmanage, False); n++;
	base_widget = XtCreateWidget(getName(), xmFormWidgetClass,
				shell, args, n);
    }
    else {
	xm = createXmString(ctitle);
	n = 0;
	XtSetArg(args[n], XmNdialogTitle, xm); n++;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNautoUnmanage, False); n++;
	if(pointer_focus) {
	    XtSetArg(args[n], XmNkeyboardFocusPolicy, XmPOINTER); n++;
	}
	base_widget = XmCreateFormDialog(comp_parent->baseWidget(),
				(char *)getName(), args, n);
	XmStringFree(xm);

	n = 0;
	XtSetArg(args[n], XmNmappedWhenManaged, False); n++;
	XtSetValues(widgetParent(), args, n);
	shell = widgetParent();
    }
    XmAddWMProtocolCallback(shell, XmInternAtom(XtDisplay(shell),
		(char *)"WM_DELETE_WINDOW", False), deleteWindowCB,
		(XtPointer)this);
    XtAddEventHandler(shell, (EventMask) 0, True,
		(XtEventHandler) _XEditResCheckMessages, NULL);

    installDestroyHandler();

    XtAddCallback(base_widget, XmNmapCallback,
		FormDialog::mapCallback, (XtPointer)this);

    XtAddCallback(widgetParent(), XmNpopdownCallback,popdownCB,(XtPointer)this);

    enableCallbackType(XmNpopdownCallback);

    dialog_state = DIALOG_CANCEL;
    XtVaGetValues(base_widget, XmNx, &initial_x, XmNy, &initial_y,
		XmNwidth, &initial_width, XmNheight, &initial_height, NULL);

    Pixmap pixmap = Application::getApplication()->getIconPixmap(base_widget);
    if(pixmap != (Pixmap)NULL) {
//	XtVaSetValues(base_widget, XmNiconPixmap, pixmap, NULL);
	XtVaSetValues(widgetParent(), XmNiconPixmap, pixmap, NULL);
    }
}

static void deleteWindowCB(Widget widget, XtPointer client, XtPointer data)
{
    ((FormDialog *)client)->windowManagerClose();
}

/** Destructor.
 */
FormDialog::~FormDialog(void)
{
    Application::getApplication()->unregisterWindow(this);
}

/** Delete the FormDialog. Call this function to delete a FormDialog, instead
 *  of deleting the object directly.  It is not necessary to delete or
 *  destroy the children of a FormDialog that is being destroyed. The children
 *  will be destroyed automatically by the Xt library.
 */
void FormDialog::destroy(void)
{
    XtRemoveAllCallbacks(base_widget, XmNmapCallback);
    XtRemoveAllCallbacks(widgetParent(), XmNpopdownCallback);
    Component::destroy();
}

/** Set the FormDialog visible with a position offset from the parent.
 *  @param[in] visible if true, position the FormDialog to the side of the
 *	parent.
 */
void FormDialog::setVisible(bool visible)
{
    setVisible(visible, OFFSET_DIALOG);
}

/** Set the FormDialog visible with a position argument. If the position
 *  argument is OFFSET_DIALOG, the FormDialog is positioned to the side of the
 *  parent. It the position argument is CENTER_DIALOG, position the FormDialog
 *  in the center of the parent window.
 *  @param[in] visible if true, position the FormDialog according to the
 *	position argument
 *  @param[in] position either OFFSET_DIALOG or CENTER_DIALOG.
 */
void FormDialog::setVisible(bool visible, DialogPosition position)
{
    bool get_initial_values = false;
    if(visible) {
	if( !positioned && indep) {
	    Dimension w, h;

	    if( (w = getProperty(getName()+string(".width"), 0)) )
	    {
		XtVaSetValues(base_widget, XtNwidth, w, NULL);
	    }
	    if(	(h = getProperty(getName()+string(".height"), 0)) )
	    {
		XtVaSetValues(base_widget, XtNheight, h, NULL);
	    }
	    get_initial_values = true;
	}

	if(position == OFFSET_DIALOG) {
	    positionOffset();
	}
	else if(position == CENTER_DIALOG) {
	    positionCenter();
	}
 	if(!Application::parseOnly()) {
	    XtVaSetValues(widgetParent(), XmNmappedWhenManaged, True, NULL);
	}
    }

    Component::setVisible(visible);
    if(visible) {
	Widget w = XtParent(base_widget);
	XtManageChild(w);
 	if(!Application::parseOnly()) {
	    if(XtWindow(w)) {
		XMapRaised(XtDisplay(w), XtWindow(w));
		XRaiseWindow(XtDisplay(w), XtWindow(w));
	    }
	}
    }
    else {
	XtUnmanageChild(widgetParent());
    }

    if(get_initial_values) {
	XtVaGetValues(base_widget, XtNwidth, &initial_width,
			XtNheight, &initial_height, NULL);
    }
}

/** Returns true if the FormDialog is visible.
 */
bool FormDialog::isVisible(void)
{
    Boolean mapped = True;
    if(!Application::parseOnly()) {
	XtVaGetValues(widgetParent(), XtNmappedWhenManaged, &mapped, NULL);
    }
    return (XtIsManaged(base_widget) && mapped);
}

/** Offset the FormDialog to the side of the parent.
 */
void FormDialog::positionOffset(void)
{
    Arg args[4];
    int n, screen_width, screen_height;
    Position x, y, popup_x, popup_y;
    Dimension width, height, popup_width, popup_height;
    Widget w, parent_widget = widgetParent();

    // if already positioned once, then don't change the position
    if( positioned ) {
	return;
    }
    positioned = true;

    if(!XtIsManaged(base_widget)) {
	XtVaSetValues(parent_widget, XmNmappedWhenManaged, False, NULL);
	Component::setVisible(true);
	XtManageChild(parent_widget);
    }

    XtVaGetValues(base_widget, XmNwidth, &popup_width,
		XmNheight, &popup_height, NULL);

    screen_width = WidthOfScreen(XtScreen(parent_widget));
    screen_height = HeightOfScreen(XtScreen(parent_widget));
    n = 0;
    XtSetArg(args[n], XmNx, &x); n++;
    XtSetArg(args[n], XmNy, &y); n++;
    XtSetArg(args[n], XmNwidth, &width); n++;
    XtSetArg(args[n], XmNheight, &height); n++;
    TopWindow *tp = comp_parent->topWindowParent();
    if(tp) {
	if(tp->isVisible()) {
	    XtGetValues(tp->baseWidget(), args, n);
	}
	else if(Application::getApplication()->getFirstWindow()) {
	    w = Application::getApplication()->getFirstWindow()->baseWidget();
	    XtGetValues(w, args, n);
	}
	else {
	    XtGetValues(tp->baseWidget(), args, n);
	}
    }
    else {
	XtGetValues(comp_parent->baseWidget(), args, n);
    }

    if(x > (int)popup_width) {
        popup_x = x - popup_width;
        if(popup_x > 30) popup_x -= 30;
        if(popup_x <  0) popup_x = 0;
    }
    else if(x + (int)width < screen_width - (int)popup_width) {
        popup_x = x + width;
	if(popup_x + 30 + popup_width < screen_width) popup_x += 30;
    }
    else {
        popup_x = x + (Position)(.5*width - .5*(int)popup_width);
    }
    popup_y = y;
    if(popup_y < 0) {
        popup_y = 0;
    }
    else if(popup_y + (int)popup_height > screen_height) {
        popup_y = 1020 - popup_height;
    }

//    if(this == Application::getApplication()->windows.front())
    if( !Application::getApplication()->first_window )
    {
	Application::getApplication()->first_window = this;
	// if this is the first window, set the position from properties.
	string name;
	char value[20];
	name = this->getName()+string(".x");
	popup_x = getProperty(name, 100);
	snprintf(value, sizeof(value), "%d", popup_x);
	putProperty(name, value);

	name = this->getName()+string(".y");
	popup_y = getProperty(name, 100);
	snprintf(value, sizeof(value), "%d", popup_y);
	putProperty(name, value);

	// after this widget is realized and mapped with the window manager
	// decoration, the position coordinates will be different.
	// Get these altered position coordinates in getPositionCB().
	XtAppAddTimeOut(Application::getAppContext(), 1000, getPositionCB,
		(XtPointer)this);
    }
    XtVaSetValues(base_widget, XmNx, popup_x, XmNy, popup_y, NULL);
    if(!Application::parseOnly()) {
	XtVaSetValues(parent_widget, XmNmappedWhenManaged, True, NULL);
    }
}

void FormDialog::getPositionCB(XtPointer data, XtIntervalId *id)
{
    FormDialog *fd = (FormDialog *)data;
    XtVaGetValues(fd->baseWidget(), XmNx, &fd->initial_x, XmNy, &fd->initial_y,
		NULL);
}


/** Place the FormDialog in the center of the parent.
 */
void FormDialog::positionCenter(void)
{
    Arg args[4];
    int n, screen_width, screen_height;
    Position x, y, popup_x, popup_y;
    Dimension width, height, popup_width, popup_height;
    Widget w, parent_widget = widgetParent();

    if(!XtIsManaged(base_widget)) {
	XtVaSetValues(parent_widget, XmNmappedWhenManaged, False, NULL);
	Component::setVisible(true);
	XtManageChild(parent_widget);
    }

    XtVaGetValues(parent_widget, XmNx, &popup_x, XmNy, &popup_y,
		XmNwidth, &popup_width, XmNheight, &popup_height, NULL);

    screen_width = WidthOfScreen(XtScreen(parent_widget));
    screen_height = HeightOfScreen(XtScreen(parent_widget));
    n = 0;
    XtSetArg(args[n], XmNx, &x); n++;
    XtSetArg(args[n], XmNy, &y); n++;
    XtSetArg(args[n], XmNwidth, &width); n++;
    XtSetArg(args[n], XmNheight, &height); n++;
    TopWindow *tp = comp_parent->topWindowParent();
    if(tp) {
	if(tp->isVisible()) {
	    XtGetValues(tp->baseWidget(), args, n);
	}
	else if(Application::getApplication()->getFirstWindow()) {
	    w = Application::getApplication()->getFirstWindow()->baseWidget();
	    XtGetValues(w, args, n);
	}
	else {
	    XtGetValues(tp->baseWidget(), args, n);
	}
    }
    else {
	XtGetValues(comp_parent->baseWidget(), args, n);
    }
    popup_x = (x + width/2) - popup_width/2;
    popup_y = (y + height/2) - popup_height/2;

    if(popup_x + popup_width > screen_width) {
	popup_x = screen_width - popup_width;
    }
    if(popup_x < 0) popup_x = 0;

    if(popup_y + popup_height > screen_height) {
	popup_y = screen_height - popup_height;
    }
    if(popup_y < 0) popup_y = 0;

    XtVaSetValues(parent_widget, XmNx, popup_x, XmNy, popup_y, NULL);
    if(!Application::parseOnly()) {
	XtVaSetValues(parent_widget, XmNmappedWhenManaged, True, NULL);
    }
}

/** Set the autoUnmanage resource. If the autoUnmanage resource is true, the
 *  FormDialog will be hidden after any child button is activated.
 *  @param[in] state the new state of the autoUnmanage resource.
 */
void FormDialog::setAutoUnmanage(bool state)
{
    XtVaSetValues(base_widget, XmNautoUnmanage, state, NULL);
}

/** Motif XmNmapCallback callback.
 */
void FormDialog::mapCallback(Widget w, XtPointer clientData, XtPointer calldata)
{
    FormDialog *form = (FormDialog *)clientData;

    XtRemoveCallback(form->base_widget, XmNmapCallback,
			FormDialog::mapCallback, clientData);

    Atom protocols[1];
    Display *d = XtDisplay(form->base_widget);
    protocols[0] = XInternAtom(d, "WM_DELETE_WINDOW", False);

    XSetWMProtocols(d, XtWindow(form->base_widget), protocols, 1);
    XtAddEventHandler(form->base_widget, NoEventMask, True, HandleWindowDel,
			clientData);
}

void FormDialog::HandleWindowDel(Widget w, XtPointer client, XEvent *event,
                Boolean *dispatch)
{
cout << "inside HandleWindowDel\n";
//    *dispatch = False;
}

/** Motif XmNpopdownCallback callback.
 */
void FormDialog::popdownCB(Widget w, XtPointer clientData, XtPointer calldata)
{
    FormDialog *form = (FormDialog *)clientData;
    form->doCallbacks(w, calldata, (const char *)XmNpopdownCallback);
}

/** Set an X-resource.
 *  @param[in] res
 */
void FormDialog::setResource(const char *res)
{
    setResource("%s", res);
}

/** Set an X-resource.
 *  @param[in] format
 *  @param[in] res
 */
void FormDialog::setResource(const char *format, const char *res)
{
    char s[1000];
    int n;
    XrmDatabase db = NULL, display_db;

    snprintf(s, sizeof(s), "%s",
		Application::getApplication()->applicationClass());
    n = strlen(s);

    snprintf(s+n, sizeof(s)-n, format, res);
    XrmPutLineResource(&db, s);
    display_db = XrmGetDatabase(XtDisplay(base_widget));
    XrmCombineDatabase(db, &display_db, False); 
    XrmSetDatabase(XtDisplay(base_widget), display_db);
}

/** Block X-events to all other windows except this FormDialog as long as the
 *  dialog_state is DIALOG_WAITING.
 */
bool FormDialog::waitForAnswer(void)
{
    dialog_state = DIALOG_WAITING;

    Application *theApplication = Application::getApplication();

    while(dialog_state == DIALOG_WAITING)
    {
	if(!isVisible()) {
	    // the last event closed the window but did not set dialog_state
	    dialog_state = DIALOG_CANCEL;
	    return false;
	}
	XEvent event;
	XtAppNextEvent(theApplication->appContext(), &event);

	if(event.xany.type == ButtonPress || event.xany.type == ButtonRelease)
	{
	    Widget warnings_w = theApplication->warningsWidget();
	    /* only dispatch button events in the FormDialog
	     */
	    Widget w =
		    XtWindowToWidget(XtDisplay(base_widget), event.xany.window);
	    while(w != NULL)
	    {
		if(w == base_widget || w == warnings_w) {
		    XtDispatchEvent(&event); 
		    break;
		}
		w = XtParent(w);
	    }
	}
	else {
	    XtDispatchEvent(&event);
	}
    }   
    return (dialog_state != DIALOG_CANCEL) ? true : false;
}

/** Change the FormDialog window title.
 *  @param[in] title the new window title.
 */
void FormDialog::setTitle(const string &title)
{
    XtVaSetValues(XtParent(base_widget), XmNtitle, (char *)title.c_str(), NULL);
}

/** Display help text. If the input messages begins with "file:" or "http:", a
 *  browser used, otherwise a MessageDialog window displays the message.
 *  For "file:" input, the path should be relative to the program installation
 *  directory.
 *  @param[in] message
 */
void FormDialog::displayHelp(const string &message)
{
    if(!message.compare(0, 5, "http:") || !message.compare(0, 5, "file:"))
    {
	string cmd;
	const char *install_dir =
		Application::getApplication()->installationDir();
	string browser;
	if( !getProperty("browser", browser) ) {
	    browser.assign("firefox");
	}
	if(!message.compare(0, 5, "file:")) {
	    cmd = browser + " -new-tab file:" + install_dir + "/"
			+ message.substr(5) + " &";
	}
	else {
	    cmd = browser + " -new-tab " + message + " &";
	}
	system(cmd.c_str());
    }
    else {
	string title;
	title = string(getName()) + " Help";
	MessageDialog *m = new MessageDialog(title, this, message);
	m->setVisible(true);
	// the MessageDialog destroys itself when it is closed.
    }
}

/** Display help information. The help can be displayed in a MessageDialog
 *  window or a browser. If help_key_file is not NULL, then the file should
 *  contain the help for each key name. The format of the help_key_file is
 *
 *  key = {type, message}
 *
 *  where type is 'text', 'file', or 'url'
 * - text: the message is displayed in a popup window.
 * - file: the message is an html file, relative to the installation directory.
 * - url: the message is a url.
 *
 * The 'key = {type' must be on one line. The message can be on multiple lines.
 *
 * If help_key_file is NULL, then the key is the message, which is displayed
 * in a MessageDialog.
 * @param[in] key the help key name, or the help text if help_key_file=NULL.
 * @param[in] help_key_file the key file path relative to the installation
 *  	directory.
 * @returns ShowHelpRet enum: HELP_SUCCESS, NO_HELP_KEY_FILE,
 *	HELP_KEY_NOT_FOUND, HELP_FORMAT_ERROR, INVALID_HELP_TYPE,
 *	NO_HELP_FOUND, HELP_FILE_NOT_FOUND
 */
ShowHelpRet FormDialog::showHelp(const string &key, const string &help_key_file)
{
    MessageDialog *m;
    string cmd, browser;
    char line[5000];
    const char *install_dir = Application::getApplication()->installationDir();
    char *beg, *end, *pos, *msg;
    int format_error, line_no, type=0, n, ret;
    FILE *fp;

    if(help_key_file.empty()) {
	m = new MessageDialog(string(getName()) + " Help", this, key);
	m->setVisible(true);
	// the MessageDialog destroys itself when it is closed.
	return HELP_SUCCESS;
    }

    cmd = string(install_dir) + "/" + help_key_file;
    if( !(fp = fopen(cmd.c_str(), "r")) ) {
	m = new MessageDialog("Help Error", this, string("Cannot open ") + cmd);
	m->setVisible(true);
	return NO_HELP_KEY_FILE;
    }

    line_no = 0;
    format_error = 1;
    msg = NULL;
    memset(line, 0, sizeof(line));

    while( !(ret = stringGetLine(fp, line, sizeof(line)-1)) )
    {
	line_no++;
	if(line[0] != '\0' && line[0] != '#')
	{
	    // skip white space before the key
	    for(beg = line; *beg != '\0' && isspace((int)(*beg)); beg++);
	    // if no key before '=' continue;
	    if(*beg == '\0' || *beg == '=') continue;
	    // find the '=' character
	    for(end = beg+1; *end != '\0' && *end != '='; end++);
	    // if not found continue to the next line
	    if(*end != '=') continue;
	    pos = end;
	    // backup from '=' skipping white space
	    while(--end > beg && isspace((int)(*end)));
	    end++;
	    *end = '\0';

	    if( strcasecmp(key.c_str(), beg) ) continue; // key doesn't match
	
	    // found key. skip white space after the '='
	    for(beg = pos+1; *beg != '\0' && isspace((int)(*beg));beg++);
	    if(*beg != '{') break;

	    // skip white space after the '{'
	    for(++beg; *beg != '\0' && isspace((int)(*beg)); beg++);
	    if(*beg == '\0' || *beg == ',') break;

	    // find ','
	    for(end = beg+1; *end != ','; end++);
	    if(*end != ',') break;
	    pos = end;

	    // backup from ',' skipping white space
	    while(--end > beg && isspace((int)(*end)));
	    end++;
	    *end = '\0';
	    if(!strcasecmp(beg, "text")) type = 0;
	    else if(!strcasecmp(beg, "file")) type = 1;
	    else if(!strcasecmp(beg, "url")) type = 2;
	    else {
		format_error = 2;
		break;
	    }

	    // found the key. skip white space after the ','
	    for(beg = pos+1; *beg != '\0' && isspace((int)(*beg));beg++);
	    if(*beg == '}') { format_error = 3; break; }
	    if(*beg == '\0') {
		// Read lines until the first non-white character of the text
		// is found.
		while( !stringGetLine(fp, line, sizeof(line)-1) ) {
		    line_no++;
		    for(beg=line; *beg != '\0' && isspace((int)(*beg)); beg++);
		    if(*beg != '\0') break;
		}
	    }
	    if(*beg == '\0') break;

	    // find the '}' character
	    for(end = beg+1; *end != '\0' && *end != '}'; end++);
	    n = (int)(end-beg);
	    msg = (char *)malloc(n+2);
	    strncpy(msg, beg, n);
	    msg[n] = '\n';
	    msg[n+1] = '\0';

	    if(*end == '\0') {
		// if not found, save what we have and read more lines until
		// the '}' is found.
		while( !stringGetLine(fp, line, sizeof(line)-1) )
		{
		    line_no++;
		    for(end = line; *end != '}' && *end != '\0'; end++);
		    n = (int)strlen(msg) + (int)(end - line) + 1;
		    msg = (char *)realloc(msg, n+2);
		    strncat(msg, line, (int)(end-line)+1);
		    msg[n-1] = '\n';
		    msg[n] = '\0';
		    if(*end == '}') break;
		}
	    }
	    format_error = 0;
	    break;
	}
    }
    fclose(fp);

    if(ret == EOF && !msg) {
	cerr << "key: '" << key << "' not found in " << cmd << endl;
	return HELP_KEY_NOT_FOUND;
    }
    else if( format_error == 1 ) {
	cerr << "Format error: line " << line_no << " in " << cmd << endl;
	Free(msg);
	return HELP_FORMAT_ERROR;
    }
    else if(format_error == 2) {
	cerr << "Invalid type " << line_no << " in " << cmd << endl;
	Free(msg);
	return INVALID_HELP_TYPE;
    }
    else if(format_error == 3) {
	cerr << "No help available for " << key << endl;
	Free(msg);
	return NO_HELP_FOUND;
    }
    n = (int)strlen(msg);
    if(n > 0 && msg[n-1] == '\n') msg[n-1] = '\0';

    if(type == 0) {
	m = new MessageDialog(key, this, msg);
	m->setVisible(true);
    }
    else if(type == 1) {
	struct stat buf;
	cmd = string(install_dir) + "/" + msg;
	if(stat(cmd.c_str(), &buf) < 0) {
	    showWarning("%s not found.", cmd.c_str());
	    return HELP_FILE_NOT_FOUND;
	}
	if( !getProperty("browser", browser) ) {
	    browser.assign("firefox");
	}
	cmd = browser + " -new-tab file:" + install_dir + "/" + msg + " &";
	system(cmd.c_str());
    }
    else { // url
	if( !getProperty("browser", browser) ) {
	    browser.assign("firefox");
	}
	cmd = browser + " -new-tab " + msg + " &";
	system(cmd.c_str());
    }
    Free(msg);
    return HELP_SUCCESS;
}

void FormDialog::manage(void)
{
    XtManageChild(base_widget);
    XtManageChild(widgetParent());
}

/** Set the window size. Check for width and height properties that override
 *  the input width and height.
 *  @param[in] width window width
 *  @param[in] height window height
 */
void FormDialog::setSize(int width, int height)
{
    int w, h;
    int screen_width = WidthOfScreen(XtScreen(widgetParent()));
    int screen_height = HeightOfScreen(XtScreen(widgetParent()));

    if( (w = getProperty(getName() + string(".width"), 0)) == 0) {
        w = width;
    }

    if( (h = getProperty(getName() + string(".height"), 0)) == 0) {
        h = height;
    }

    if(w <= 0) w = 20;
    else if(w >= screen_width) w = screen_width;
    if(h <= 0) h = 20;
    else if(h >= screen_height) h = screen_height;

    Component::setSize(w, h);
}

ParseCmd FormDialog::parseCmd(const string &cmd, string &msg)
{
    string s;

    if(parseCompare(cmd, "open")) {
	setVisible(true);
    }
    else if(parseCompare(cmd, "close")) {
	setVisible(false);
    }
    else if(parseCompare(cmd, "quit") || parseCompare(cmd, "exit")) {
	setVisible(false);
        Application::getApplication()->stop(0);
    }
    else if(parseCompare(cmd, "choiceQuestion")) {
	msg.assign("choiceQuestion: missing 'question' argument.");
	return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "choiceQuestion", s)) {
	return parseChoiceQuestion(s, msg);
    }
    else if(parseCompare(cmd, "textQuestion")) {
	msg.assign("textQuestion: missing 'question' argument.");
	return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "textQuestion", s)) {
	return parseTextQuestion(s, msg);
    }
    else if(parseCompare(cmd, "listQuestion")) {
	msg.assign("listQuestion: missing 'items' argument.");
	return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "listQuestion", s)) {
	return parseListDialog(s, msg);
    }
    else if(parseCompare(cmd, "showWarning")) {
	msg.assign("showWarning: missing warning text.");
	return ARGUMENT_ERROR;
    }
    else if(parseArg(cmd, "showWarning", s)) {
	showWarning(s);
    }
    else if(parseCompare(cmd, "showMessage")) {
	msg.assign("showMessage: missing 'message' argument.");
	return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "showMessage", s)) {
	return parseShowMessage(s, msg);
    }
    else if(parseCompare(cmd, "getFile") || parseCompare(cmd,"getFile ",8)) {
	return parseGetFile(true, cmd, msg);
    }
    else if(parseCompare(cmd, "getDir")) {
	return parseGetFile(false, cmd, msg);
    }
    else {
	return Component::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd FormDialog::parseChoiceQuestion(const string &s, string &msg)
{
    string title, answer, question, button1, button2, button3;
    ostringstream ans;
    int num;

    if( !parseGetArg(s, "question", question) ) {
	msg.assign("choiceQuestion: missing 'question' argument.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(s, "title", title) ) {
	title.assign("Question");
    }
    if( !parseGetArg(s, "button1", button1) ) {
	button1.assign("OK");
    }
    parseGetArg(s, "button2", button2);
    parseGetArg(s, "button3", button3);
    if( !parseGetArg(s, "answer", answer) ) {
	answer.assign("answer");
    }

    num = Question::askQuestion(title, this, question, button1, button2,
			button3);
    ans << num;
    Application::putParseProperty(answer, ans.str());

    return COMMAND_PARSED;
}

ParseCmd FormDialog::parseTextQuestion(const string &s, string &msg)
{
    string title, question, button1, button2, answer, default_ans, ans;

    if( !parseGetArg(s, "question", question) ) {
	msg.assign("text_question: missing 'question' argument.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(s, "title", title) ) {
        title.assign("Input");
    }
    if( !parseGetArg(s, "button1", button1) ) {
	button1.assign("Apply");
    }
    if( !parseGetArg(s, "button2", button2) ) {
	button2.assign("Cancel");
    }
    parseGetArg(s, "default_response", default_ans);
    if( !parseGetArg(s, "answer", answer) ) {
        answer.assign("answer");
    }

    if(TextQuestion::askTextQuestion(title, this, question, ans,
				default_ans, button1, button2))
    {
	Application::putParseProperty(answer, ans);
    }
    else {
	Application::putParseProperty(answer, "");
    }

    return COMMAND_PARSED;
}

void FormDialog::parseHelp(const char *prefix)
{
    printf("%sopen\n", prefix);
    printf("%sclose\n", prefix);
    printf("%squit\n", prefix);
}

void FormDialog::getParsePrefix(char *prefix, int prefix_len)
{
    Component *comp[100];
    int i;
    for(i = 0, comp[0] = this; i < 99 && comp[i]->getParent(); i++) {
	comp[i+1] = comp[i]->getParent();
    }

    memset(prefix, 0, prefix_len);
    for(i -= 2; i >= 0; i--) {
	int n = strlen(prefix);
	snprintf(prefix+n, prefix_len-n, "%s.", comp[i]->getName());
    }
    for(i = 0; prefix[i] != '\0'; i++) {
	if(isspace((int)prefix[i])) prefix[i] = '_';
	else prefix[i] = tolower((int)prefix[i]);
    }
}

ParseCmd FormDialog::parseListDialog(const string &cmd, string &msg)
{
    string answer, title, sitems;
    char *c, *s, *tok, **items, **selected_items=NULL, *last;
    int len, n, num, num_selected;

    if( !parseGetArg(cmd, "items", sitems) ) {
	msg.assign("list_dialog: missing 'items' argument.");
	return ARGUMENT_ERROR;
    }

    if( !parseGetArg(cmd, "title", title) ) {
        title.assign("List");
    }
    if( !parseGetArg(cmd, "answer", answer) ) {
        answer.assign("answer");
    }

    items = (char **)malloc(sizeof(char *));
    s = strdup(sitems.c_str());
    num = 0;
    tok = s;
    while( (c = strtok_r(tok, ",", &last)) ) {
	tok = NULL;
	items = (char **)realloc(items, (num+1)*sizeof(char *));
	items[num++] = c;
    }
    num_selected = ListDialog::getSelectedItems(title, this, num, items,
					&selected_items);
    Free(s);

    if(num_selected > 0) {
	len = 0;
	for(int i = 0; i < num_selected; i++) {
	    len += (int)strlen(selected_items[i]) + 1;
	}
	len += 1;

	s = (char *)malloc(len);
	s[0] = '\0';

	for(int i = 0; i < num_selected; i++) {
	    n = (int)strlen(s);
	    snprintf(s+n, len-n, "%s,", selected_items[i]);
	}
	n = (int)strlen(s);
	s[n-1] = '\0'; // the last ','
	Application::putParseProperty(answer, s);
    }
    else {
	Application::putParseProperty(answer, "");
    }

    Free(items);
    for(int i = 0; i < num_selected; i++) Free(selected_items[i]);
    Free(selected_items);

    return COMMAND_PARSED;
}

ParseCmd FormDialog::parseShowMessage(const string &cmd, string &msg)
{
    string title, message, button1;

    if( !parseGetArg(cmd, "message", message) ) {
	msg.assign("show_message: missing 'message'.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(cmd, "title", title) ) {
        title.assign("Message");
    }
    if( !parseGetArg(cmd, "button", button1) ) {
	button1.assign("Close");
    }

    Question *q = new Question(title, this, message, button1);
    q->setVisible(true);

    return COMMAND_PARSED;
}

ParseCmd FormDialog::parseGetFile(bool get_file, const string &cmd, string &msg)
{
    string title, dir, filter, file, variable;
    bool append, ret;
    int write_file=0;

    if( !parseGetArg(cmd, "title", title) ) {
	title.assign("Select File");
    }
    if( !parseGetArg(cmd, "dir", dir) ) {
	dir.assign("./");
    }
    if( !parseGetArg(cmd, "filter", filter) ) {
	filter.assign("*");
    }

    if(get_file) {
	stringGetBoolArg(cmd.c_str(), "write", &write_file);
	if(write_file) {
	    ret = FileDialog::getFile(title, this, file, dir, filter, "Open",
				&append);
	    if(append) {
		Application::putParseProperty("append", "true");
	    }
	    else {
		Application::putParseProperty("append", "false");
	    }
	}
	else {
	    ret = FileDialog::getFile(title, this, file, dir, filter, "Open");
	}
	variable.assign("file");
    }
    else {
	ret = FileDialog::getDir(title, this, file, dir, filter, "Open");
	variable.assign("directory");
    }

    if(ret) {
	Application::putParseProperty(variable, file);
    }
    else {
	Application::putParseProperty(variable, "");
    }

    return COMMAND_PARSED;
}
