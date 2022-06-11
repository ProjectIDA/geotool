/** \file Application.cpp
 *  \brief Defines class Application.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "motif++/IPCClient.h"
#include "motif++/Application.h"
#include "motif++/MotifClasses.h"
#include "BasicMap.h"


extern "C" {
static void registerCB(XtPointer data, XtIntervalId *id);
static void newMapCB(XtPointer data, XtIntervalId *id);
static void stopApplication(int sig);
}

static char *getPropLine(FILE *fp);
static int getProp(char *line, char **name, char **value);

using namespace std;

static Application *theApplication = NULL;

class MessageReceiver : public IPCMessageReceiver
{
    public:
    void parseIPCMessage(const char *msg_id, const char *msg,
		const char *msg_class) throw(runtime_error)
    {
        theApplication->parseIPCMessage(msg_id, msg, msg_class);
    }
};

static MessageReceiver messageReceiver;


/** Class constructor.
 *  @param[in] name The name of the application. This name is used as the
 *		application class name in the specification of X-resources.
 *  @param[in,out] argcp a pointer to the number of command-line arguments.
 *  @param[in,out] argv a pointer to the command-line arguments.
 *  @param[in] version_str program version information.
 *  @param[in] installation_dir program installation directory.
 *  @param[in] only_plugins a comma delimited list of the plugins that will
 *	be allowed. If it is empty, no plugins will be allowed. If only_plugins
 *	is NULL, all plugins will be allowed. Example: "libgtq,libgfk,libgrt"
 */
Application::Application(const string &name, int *argcp, const char **argv,
		const string &version_str, const string &installation_dir,
		const string &only_plugins) :
		AppParse(name, NULL, argcp, argv, installation_dir)
{
    init(name, argcp, argv, version_str, only_plugins);
}

/** Class constructor.
 *  @param[in] name The name of the application. This name is used as the
 *		application class name in the specification of X-resources.
 *  @param[in,out] argcp a pointer to the number of command-line arguments.
 *  @param[in,out] argv a pointer to the command-line arguments.
 *  @param[in] clean_up_method a method that will be called just before the
 *	program exits.
 *  @param[in] version_str program version information.
 *  @param[in] installation_dir program installation directory.
 *  @param[in] only_plugins a comma delimited list of the plugins that will
 *	be allowed. If it is empty, no plugins will be allowed. If only_plugins
 *	is NULL, all plugins will be allowed. Example: "libgtq,libgfk,libgrt"
 */
Application::Application(const string &name, int *argcp, const char **argv,
		CleanUpMethod clean_up_method, const string &version_str,
		const string &installation_dir, const string &only_plugins) :
		AppParse(name, NULL, argcp, argv, installation_dir)
{
    init(name, argcp, argv, version_str, only_plugins);
    clean_up = clean_up_method;
}

/** Initialization. Calls XtAppInitialize and XtRealizeWidget.
 *  @param[in] name The name of the application. This name is used as the
 *		application class name in the specification of X-resources.
 *  @param[in,out] argcp a pointer to the number of command-line arguments.
 *  @param[in,out] argv a pointer to the command-line arguments.
 *  @param[in] version_str program version information.
 *  @param[in] only_plugins a comma delimited list of the plugins that will
 *	be allowed. If it is empty, no plugins will be allowed. If only_plugins
 *	is NULL, all plugins will be allowed. Example: "libgtq,libgfk,libgrt"
 */
void Application::init(const string &name, int *argcp, const char **argv,
		const string &version_str, const string &only_plugins)
{
    int i;
    bool verbose = false;
    char *s, *c;

    redirect_warnings = false;

    if(nargcp > 0 && arg_v) {
	for(i = 1; i < nargcp; i++) {
	    if(!strcmp(argv[i], "-i")) {
		redirect_warnings = true;
	    }
	    else if(!strcmp(argv[i], "-verbose")) {
		verbose = true;
	    }
	}
    }

    if(!version_str.empty()) {
	version_info = version_str;
	c = strdup(version_str.c_str());
	if((s = strstr(c, "\n")) != NULL) *s = '\0';
	release_info.assign(s);
	free(c);
    }

    theApplication = this;

    plug_in_manager = new PlugInManager(only_plugins, verbose);

    undo_manager = new UndoManager("undo manager");

    makePropertiesDir();
//    fontsInit(programProperties());

    enableCallbackType(XtNchangePropertyCallback);
    enableCallbackType(XtNformDialogCallback);
    enableCallbackType(XtNmapCallback);
//    enableCallbackType(XtNanyMapCallback);

    initMembers();

    if(nargcp > 0 && arg_v) {
	for(i = 1; i < nargcp; i++) {
	    if(strstr(argv[i], "version") || !strcmp(argv[i], "-v")) {
		printf("%s\n", version_str.c_str());
	    }
	}
	if(stringGetBoolArgv(nargcp, arg_v, "write_to_db", &i)) {
	    write_to_db = (bool)i;
	}
	if((c = stringGetArgv(nargcp, arg_v, "window"))) {
	    first_window_name.assign(c);
	}
    }
    addReservedName("first_window");

    if(!first_window_name.empty()) {
	putGlobalVariable("first_window", first_window_name);
    }
    else {
	putGlobalVariable("first_window", name);
    }
	

    signal(SIGHUP, stopApplication);
    signal(SIGINT, stopApplication);
    signal(SIGQUIT, stopApplication);
    signal(SIGTRAP, stopApplication);
    signal(SIGABRT, stopApplication);
    signal(SIGFPE, stopApplication);
    signal(SIGUSR1, stopApplication);
    signal(SIGSEGV, stopApplication);
    signal(SIGTERM, stopApplication);

    /* Initiate the IPC Connection */
    IPCClient* ipcc = IPCClient::getInstance();
    ipcc->connect(argcp, argv);
    ipcc->registerReceiver(&messageReceiver);
}

/** Initialize class members.
 * Initialize some of the Application class members.
 */
void Application::initMembers(void)
{
    stop_flag = false;
    warnings = NULL;
    log_window = NULL;

    first_window = NULL;

    num_displays = 1;
    display_list[0].display = XtDisplay(base_widget);
    display_list[0].name = strdup(":0.0");
    display_list[0].parent = base_widget;
    display_list[0].icon_pixmap = (Pixmap)NULL;
    display_list[0].hourglass_cursor = (Cursor)-1;
    display_list[0].hand_cursor = (Cursor)-1;
    display_list[0].move_cursor = (Cursor)-1;
    display_list[0].crosshair_cursor = (Cursor)-1;
    display_list[0].resize_up_cursor = (Cursor)-1;
    display_list[0].resize_down_cursor = (Cursor)-1;
    display_list[0].resize_right_cursor = (Cursor)-1;
    display_list[0].resize_left_cursor = (Cursor)-1;
    display_list[0].arrow_cursor = (Cursor)-1;

    make_icon = NULL;

    rm = NULL;

    clean_up = NULL;
    write_to_db = true;
}

/** Class destructor.
 */
Application::~Application(void)
{
}

/** Event handler.
 *  This function is called once at the beginning of an application to start
 *  the event loop. XtAppNextEvent and XtDispatchEvent are called for each
 *  XEvent. This function will exit the event loop and return if the class
 *  member stop_flag is set to true. Use the stop() routine to set the
 *  stop_flag. This function will also exit the event loop and return if all
 *  windows have been unmanaged.
 */
void Application::handleEvents(void)
{
    while(!stop_flag)
    {
	XEvent event;

	if( !parse_only && !XtAppPending(app_context) )
	{
	    int j;
	    for(j = (int)windows.size()-1; j >= 0; j--) {
		if( !windows.at(j)->baseWidget() ) {
		    windows.erase(windows.begin()+j);
		}
	    }
	    // check if all TopWindow windows are closed
	    for(j = 0; j < (int)windows.size(); j++) {
		Component *comp = windows.at(j);
		if( comp->getFormDialogInstance() && comp->isVisible() ) break;
	    }
	    if(j == (int)windows.size()) break;
	}

	XtAppNextEvent(app_context, &event);

	XtDispatchEvent(&event);
    }
    AppParse::stop(0);
}

static void
stopApplication(int sig)
{
    Application::getApplication()->stop(0);
}

/** Clean up at the end of an application.
 * This routine is called one time after the Application exists the event loop.
 * All temporary files are deleted and program properties are written to disk.
 */
void Application::cleanUp(void)
{
    char name[200], value[20];
    FormDialog *fd;
    bool first_realized = false;

    if(clean_up) (*clean_up)();

    for(int i = 0; i < (int)windows.size(); i++) {
	Component *comp = windows.at(i);
	if( (fd = comp->getFormDialogInstance()) )
	{
	    if( fd->isIndependent() ) {
		Dimension w, h;
		XtVaGetValues(fd->baseWidget(), XmNwidth, &w, XmNheight, &h,
				NULL);

		if(w != fd->initialWidth()) {
		    snprintf(name, sizeof(name), "%s.width", fd->getName());
		    snprintf(value, sizeof(value), "%d", fd->getWidth());
		    putProperty(name, value);
		}

		if(h != fd->initialHeight()) {
		    snprintf(name, sizeof(name), "%s.height", fd->getName());
		    snprintf(value, sizeof(value), "%d", fd->getHeight());
		    putProperty(name, value);
		}
	    }

	    if( !first_realized && XtIsRealized(comp->baseWidget())) {
		Position x, y;
		first_realized = true;
		XtVaGetValues(comp->baseWidget(), XmNx, &x, XmNy, &y, NULL);
		
		if(x != fd->initialX()) {
		    snprintf(name, sizeof(name), "%s.x", comp->getName());
		    snprintf(value, sizeof(value), "%d", x);
		    putProperty(name, value);
		}
		if(y != fd->initialY()) {
		    snprintf(name, sizeof(name), "%s.y", comp->getName());
		    snprintf(value, sizeof(value), "%d", y);
		    putProperty(name, value);
		}
	    }
	}
    }
    writeApplicationProperties();

//    if(thread) pthread_kill(thread, SIGUSR1);
}

/** Register a new window.
 *  This routine is called when a FormDialog class is created to register the
 *  window with the Application class instance. All listeners for the
 *  XtNformDialogCallback action are notified.
 */
void Application::registerWindow(Component *window)
{
    windows.push_back(window);
    // need this timeout to allow the new window to finish it's initialization
    XtAppAddTimeOut(app_context, 0, registerCB, (XtPointer)window);
}

static void registerCB(XtPointer data, XtIntervalId *id)
{
    Application::getApplication()->doCallbacks((Component *)data, (XtPointer)1,
		XtNformDialogCallback);
}

/** Unregister a new window.
 *  This routine is called when a FormDialog class is destroyed to unregister
 *  the window with the Application class instance. All listeners for the
 *  XtNformDialogCallback action are notified.
 *  @param[in] window a FormDialog instance
 */
void Application::unregisterWindow(Component *window)
{
    for(int i = (int)windows.size()-1; i >= 0;i--) {
	if(windows.at(i) == window) windows.erase(windows.begin()+i);
    }
// having problems with this. window can already be destroyed at this point.
//    doCallbacks(window, (XtPointer)0, XtNformDialogCallback);
}

/** Set all windows visibles or hide all windows.
 * This routine sets all toplevel windows visible or hides all toplevel windows.
 * @param[in] visible
 */
void Application::setVisible(bool visible)
{
    // Manage all windows, popping up iconified windows as well.
    for(int i = 0; i < (int)windows.size(); i++) {
	windows.at(i)->setVisible(visible);
    }
}

/** Iconify the application.
 * Iconify all windows of the application.
 */
void Application::iconify(void)
{
    // Iconify all top-level windows
/*
    for(int i = 0; i < (int)windows.size(); i++) {
	windows.at(i)->iconify();
    }
*/
}

/** Get the top shell Widget for the display name.
 *  The Application class supports multiple Displays (monitors) for its
 *  TopWindow class instances. This function returns the top shell Widget
 *  that will be the parent of all TopWindow class instances on the
 *  specified display.
 *  @param name the display specification (for example ":0.1")
 */
Widget Application::getDisplayParent(const string &name)
{
    XrmValue value;
    char *type, *app_class=NULL;
    const char *app_name=NULL;

    if( !num_displays ) {
	num_displays = 1;
	display_list[0].display = XtDisplay(base_widget);
	display_list[0].name = strdup(":0.0");
	display_list[0].parent = base_widget;
	display_list[0].hourglass_cursor = (Cursor)-1;
	display_list[0].hand_cursor = (Cursor)-1;
	display_list[0].move_cursor = (Cursor)-1;
	display_list[0].crosshair_cursor = (Cursor)-1;
	display_list[0].resize_up_cursor = (Cursor)-1;
	display_list[0].resize_down_cursor = (Cursor)-1;
	display_list[0].resize_right_cursor = (Cursor)-1;
	display_list[0].resize_left_cursor = (Cursor)-1;
	display_list[0].arrow_cursor = (Cursor)-1;
    }

    app_name = application_class.c_str();
    app_class = strdup(app_name);
    app_class[0] = toupper(app_class[0]);

    XrmDatabase database = XrmGetDatabase(XtDisplay(base_widget));

    if(XrmGetResource(database, name.c_str(), name.c_str(), &type, &value))
    {
	Arg args[3];
	XtSetArg(args[0], XmNwidth, 1);
	XtSetArg(args[1], XmNheight, 1);
	XtSetArg(args[2], XmNmappedWhenManaged, False);

	int i;
	for(i=0; i<num_displays && strcmp(display_list[i].name,value.addr);i++);
	if(i < num_displays) {
	    return display_list[i].parent;
	}
	if(num_displays == MAX_NUM_DISPLAYS) {
	    char error[50];
	    snprintf(error, sizeof(error),
		"Maximum number of displays (%d) exceeded.", MAX_NUM_DISPLAYS);
	    logErrorMsg(LOG_ERR, error);
	    return display_list[0].parent;
	}
	if( !(display_list[i].display = XtOpenDisplay(app_context, value.addr,
		app_name, app_class, NULL, 0, &nargcp, (char **)arg_v)) ||
	    !(display_list[i].parent = XtAppCreateShell(app_name, app_class,
		applicationShellWidgetClass, display_list[i].display, args, 3)))
	{
	    char error[50];
	    snprintf(error, sizeof(error),"Cannot open display %s.",value.addr);
	    logErrorMsg(LOG_ERR, error);
	    Free(app_class);
	    return display_list[0].parent;
	}
	num_displays++;
	Free(app_class);
	return display_list[i].parent;
    }
    else {
	Free(app_class);
	return display_list[0].parent;
    }
}

void Application::redirectCallback(Widget w, XtPointer clientData,
			XtPointer calldata)
{
    getApplication()->redirect_warnings = XmToggleButtonGetState(w);
}

/** Set the cursor type for all windows. This function will set the cursor
 *  type in all FormDialog instances. Available cursor types are:
 *	- "hourglass"
 *	- "hand"
 *	-  "move"
 *	- "crosshair"
 *	- "resize up"
 *	- "resize down"
 *	- "resize right"
 *	- "resize left"
 *	- "arrow"
 *  @param[in] cursor_type
 */
void Application::setCursor(const string &cursor_type)
{
    Cursor cursor;
    
    if(!cursor_type.compare("default")) {
	for(int i = 0; i < (int)windows.size(); i++) {
	    Widget w = windows.at(i)->baseWidget();
	    if(w && XtIsRealized(w) && XtWindow(w)) {
		XUndefineCursor(XtDisplay(w), XtWindow(w));
		XFlush(XtDisplay(w));
	    }
	    if(!w) {
		cout << "Null widget for " << windows.at(i)->getName() << endl;
	    }
	}
    }
    else {
	for(int i = 0; i < (int)windows.size(); i++) {
	    Widget w = windows.at(i)->baseWidget();
	    if(w && XtIsRealized(w) && XtWindow(w)) {
		cursor = getCursor(w, cursor_type);
		XDefineCursor(XtDisplay(w),  XtWindow(w), cursor);
		XFlush(XtDisplay(w));
	    }
	    if(!w) {
		cout << "Null widget for " << windows.at(i)->getName() << endl;
	    }
	}
    }
}

/** Get the Application instance.
 */
Application *Application::getApplication(void)
{
    if(!theApplication) {
	theApplication = new Application("application", NULL, 0);
    }
    return theApplication;
}

/** Get a FormDialog.
 *  @param name the name given to the FormDialog
 *  @returns the FormDialog instance or NULL if none exists for the input name.
 */
Component * Application::getWindow(const string &name)
{
    for(int i = 0; i < (int)windows.size(); i++) {
	if(!name.compare(windows.at(i)->getName())) return windows.at(i);
    }
    return NULL;
}

/** Get a FormDialog.
 *  @param form_dialog_base_widget the baseWidget of the FormDialog
 *  @returns the FormDialog instance or NULL if none exists for the input name.
 */
Component * Application::getWindow(Widget form_dialog_base_widget)
{
    for(int i = 0; i < (int)windows.size(); i++) {
	if(windows.at(i)->baseWidget() == form_dialog_base_widget) {
	    return windows.at(i);
	}
    }
    return NULL;
}

/** Return all Components.
 * Return all Components in the Application.
 * @return A vector containing the Component instances. The returned vector
 *	can be empty, but it will never be NULL.
 */
vector<Component *> * Application::getComponents(void)
{
    vector<Component *> *v = new vector<Component *>;

    for(int i = 0; i < (int)windows.size(); i++) {
	getDescendants(v, windows.at(i));
    }
    return v;
}

/** Find Component descendants.
 *  Find all Components descendants of the input parent.
 *  @param[out] v the Component instances of class_name.
 *  @param[in] parent the Component parent (tree-top).
 */
void Application::getDescendants(vector<Component *> *v, Component *parent)
{
    v->push_back(parent);
    vector<Component *> *c = parent->getChildren();

    for(int i = 0; i < (int)c->size(); i++) {
	getDescendants(v, c->at(i));
    }
    delete c;
}

/** Get the number of visible FormDialog instances.
 *  @returns the number of visible FormDialog class instances.
 */
int Application::numVisibleWindows(void)
{
    int n = 0;
    for(int i = 0; i < (int)windows.size(); i++) {
	if( windows.at(i)->isVisible() ) n++;
    }
    return n;
}

/** Get the Warnings window
 */
Warnings * Application::warningsWindow(void)
{
    if(!warnings) {
	if(windows.size() > 0) {
	    warnings = new Warnings(windows.at(0));
	}
	else {
	    warnings = new Warnings(this);
	}
    }
    return warnings;
}

/** Display the warnings popup window.
 *  All messages that are input to the functions showWarning and putWarning
 *  are saved in the Application class and can be displayed with this function.
 */
void Application::displayWarnings(void)
{
    Application *app = getApplication();
    if(!app->warnings) {
	if(app->windows.size() > 0) {
	    app->warnings = new Warnings(app->windows.at(0));
	}
	else {
	    app->warnings = new Warnings(app);
	}
    }
    app->warnings->setVisible(true);
}

/** Get the Log window
 */
TextDialog * Application::logWindow(void)
{
    if(!log_window) {
	if(windows.size() > 0) {
	    log_window = new TextDialog("Log", windows.at(0));
	}
	else {
	    log_window = new TextDialog("Log", this);
	}
	Arg args[1];
	XtSetArg(args[0], XtNbackground, log_window->stringToPixel("white"));
	log_window->textField()->setValues(args, 1);
    }
    return log_window;
}

/** Display the log window.
 *  All messages that are input to the functions printLog are saved in
 *  the Application class and can be displayed with this function.
 */
void Application::displayLog(void)
{
    Application *app = getApplication();
    if(!app->log_window) {
	if(app->windows.size() > 0) {
	    app->log_window = new TextDialog("Log", app->windows.at(0));
	}
	else {
	    app->log_window = new TextDialog("Log", app);
	}
	Arg args[1];
	XtSetArg(args[0],XtNbackground,app->log_window->stringToPixel("white"));
	app->log_window->textField()->setValues(args, 1);
    }
    app->log_window->setVisible(true);
}

/** Create the Applications properties directory. The default location of
 *  this directory is $HOME/.geotool++.
 */
void Application::makePropertiesDir(void)
{
    char *home;

    if((home = getenv("HOME")) != NULL && (int)strlen(home) > 0)
    {
	char path[MAXPATHLEN+1];
	struct stat buf;
	snprintf(path, sizeof(path), "%s/.geotool++", home);

	if( stat(path, &buf) ) { // might not exist
	    if(mkdir(path, 0777)) {
		fprintf(stderr, "Cannot create %s\n%s\n", path,strerror(errno));
	    }
	}
	// Also make the .geotool++/plugins directory.
	snprintf(path, sizeof(path), "%s/.geotool++/plugins", home);

	if( stat(path, &buf) ) { // might not exist
	    if(mkdir(path, 0777)) {
		fprintf(stderr, "Cannot create %s\n%s\n", path,strerror(errno));
	    }
	}
    }
}

#define hour_glass_width 16	/* hourglass cursor */
#define hour_glass_height 16
static unsigned char hour_glass_bits[] = {
   0xff, 0x7f, 0x21, 0x42, 0x11, 0x44, 0x39, 0x4e, 0x79, 0x4f, 0xf9, 0x4f,
   0xf1, 0x47, 0xe1, 0x43, 0xe1, 0x43, 0x91, 0x44, 0x89, 0x48, 0x89, 0x48,
   0xc9, 0x49, 0xf1, 0x47, 0xe1, 0x43, 0xff, 0x7f};

#define hand_width 19		/* hand cursor */
#define hand_height 14
static  unsigned char hand_bits[] = {
 0x00,0x00,0xf8,0xfc,0x07,0xf8,0x02,0x08,0xf8,0xfc,0x11,0xf8,0x20,0x20,0xf8,
 0x10,0x20,0xf8,0xe0,0x21,0xf8,0x10,0x30,0xf8,0xe0,0x49,0xf8,0x40,0x84,0xf8,
 0x80,0x03,0xf9,0x00,0x92,0xf8,0x00,0x44,0xf8,0x00,0x28,0xf8};

#define move_width 18		/* move window cursor */
#define move_height 18
static unsigned char move_bits[] = {
 0x00,0x00,0xfc,0x00,0x00,0xfc,0x00,0x03,0xfc,0x80,0x07,0xfc,0xc0,0x0f,0xfc,
 0x00,0x03,0xfc,0x10,0x23,0xfc,0x18,0x63,0xfc,0xfc,0xff,0xfc,0xfc,0xff,0xfc,
 0x18,0x63,0xfc,0x10,0x23,0xfc,0x00,0x03,0xfc,0xc0,0x0f,0xfc,0x80,0x07,0xfc,
 0x00,0x03,0xfc,0x00,0x00,0xfc,0x00,0x00,0xfc};

#define plus_width 16		/* crosshair */
#define plus_height 18
static unsigned char plus_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
 0x01,0xfc,0x7f,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x00,0x00,0x00,0x00,0x00};

#define resize_up_width 16	/* resize up cursor */
#define resize_up_height 17
static unsigned char resize_up_bits[] = {
 0x00,0x00,0xfe,0x7f,0xfe,0x7f,0x00,0x00,0x00,0x01,0x80,0x03,0x40,0x05,0x20,
 0x09,0x10,0x11,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x00,0x00,0x00};

#define resize_down_width 16	/* resize down cursor */
#define resize_down_height 17
static unsigned char resize_down_bits[] = {
 0x00,0x00,0x00,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,
 0x00,0x88,0x08,0x90,0x04,0xa0,0x02,0xc0,0x01,0x80,0x00,0x00,0x00,0xfe,0x7f,
 0xfe,0x7f,0x00,0x00};

#define resize_right_width 17	/* resize right cursor */
#define resize_right_height 16
static unsigned char resize_right_bits[] = {
 0x00,0x00,0xfe,0x00,0xc0,0xfe,0x00,0xc0,0xfe,0x00,0xc0,0xfe,0x00,0xc1,0xfe,
 0x00,0xc2,0xfe,0x00,0xc4,0xfe,0x00,0xc8,0xfe,0xfc,0xdf,0xfe,0x00,0xc8,0xfe,
 0x00,0xc4,0xfe,0x00,0xc2,0xfe,0x00,0xc1,0xfe,0x00,0xc0,0xfe,0x00,0xc0,0xfe,
 0x00,0x00,0xfe};

#define resize_left_width 17	/* resize left cursor */
#define resize_left_height 16
static unsigned char resize_left_bits[] = {
 0x00,0x00,0xfe,0x06,0x00,0xfe,0x06,0x00,0xfe,0x06,0x01,0xfe,0x86,0x00,0xfe,
 0x46,0x00,0xfe,0x26,0x00,0xfe,0xf6,0x7f,0xfe,0x26,0x00,0xfe,0x46,0x00,0xfe,
 0x86,0x00,0xfe,0x06,0x01,0xfe,0x06,0x00,0xfe,0x06,0x00,0xfe,0x06,0x00,0xfe,
 0x00,0x00,0xfe};

#define arrow_width 19		/* arrow pointing north west */
#define arrow_height 18
static unsigned char arrow_bits[] = {
 0x00,0x00,0xf8,0x00,0x00,0xf8,0x0c,0x00,0xf8,0x3c,0x00,0xf8,0xf8,0x00,0xf8,
 0xf8,0x03,0xf8,0xf0,0x0f,0xf8,0xf0,0x31,0xf8,0xe0,0x01,0xf8,0x60,0x02,0xf8,
 0x40,0x04,0xf8,0x40,0x08,0xf8,0x80,0x10,0xf8,0x80,0x20,0xf8,0x00,0x40,0xf8,
 0x00,0x80,0xf8,0x00,0x00,0xf8,0x00,0x00,0xf8};

/** Create cursors for the i'th Display.
  * @param[in] i
 */
void Application::createDisplayCursors(int i)
{
    Pixel fg = stringToPixel("Black");
    Widget w = display_list[i].parent;

    XtVaGetValues(w, XtNforeground, &fg, NULL);

    display_list[i].hourglass_cursor = createCursor(w, "hourglass", fg);
    display_list[i].hand_cursor = createCursor(w, "hand", fg);
    display_list[i].move_cursor = createCursor(w, "move", fg);
    display_list[i].crosshair_cursor = createCursor(w, "crosshair", fg);
    display_list[i].resize_up_cursor = createCursor(w, "resize up", fg);
    display_list[i].resize_down_cursor = createCursor(w, "resize down", fg);
    display_list[i].resize_right_cursor = createCursor(w, "resize right", fg);
    display_list[i].resize_left_cursor = createCursor(w, "resize left", fg);
    display_list[i].arrow_cursor = createCursor(w, "arrow", fg);
}

/** Get a Cursor.
 *  @param[in] w the Widget in which the Cursor will be displayed. This is
 *		used only to determine the X-Display.
 *  @param[in] cursor_type the cursor specification string.
 */
Cursor Application::getCursor(Widget w, const string &cursor_type)
{
    Cursor cursor;

    int i;
    for(i = 0; i < num_displays; i++) {
	if(XtDisplay(w) == display_list[i].display) break;
    }
    if(i == num_displays) {
	cerr << "Application.getCursor cannot find display for "
		<< XtName(w) << endl;
	return (Cursor)-1;
    }

    if(display_list[i].hourglass_cursor == (Cursor)-1) {
	createDisplayCursors(i);
    }

    if(!strcasecmp(cursor_type.c_str(), "hourglass")) {
	cursor = display_list[i].hourglass_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "hand")) {
	cursor = display_list[i].hand_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "move")) {
	cursor = display_list[i].move_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "crosshair") ||
		!strcasecmp(cursor_type.c_str(), "plus")) {
	cursor = display_list[i].crosshair_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "resize up")) {
	cursor = display_list[i].resize_up_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "resize down")) {
	cursor = display_list[i].resize_down_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "resize right")) {
	cursor = display_list[i].resize_right_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "resize left")) {
	cursor = display_list[i].resize_left_cursor;
    }
    else if(!strcasecmp(cursor_type.c_str(), "arrow")) {
	cursor = display_list[i].arrow_cursor;
    }
    else {
	fprintf(stderr, "Application.getCursor: unknown cursor type: %s\n",
		cursor_type.c_str());
	return (Cursor)-1;
    }
    if(cursor == (Cursor)-1) {
	fprintf(stderr, "Application.getCursor: cursor not created.\n");
	return (Cursor)-1;
    }
    return cursor;
}

/** Create a Cursor.
 *  @param[in] w a Widget for determining the Display and Window.
 *  @param[in] cursor_type the cursor specification string.
 *  @param[in] fg the foreground color.
 */
Cursor Application::createCursor(Widget w, const char *cursor_type, Pixel fg)
{
    int cursor_width, cursor_height;
    int cursor_x_hot, cursor_y_hot;
    unsigned char *bits;
    Pixmap w_pixmap;
    XColor	cfore, cback;
    Colormap cmap;
    Cursor cursor;

    if(!strcasecmp(cursor_type, "hourglass")) {
	cursor_width = hour_glass_width;
	cursor_height = hour_glass_height;
	cursor_x_hot = 7;
	cursor_y_hot = 7;
	bits = hour_glass_bits;
    }
    else if(!strcasecmp(cursor_type, "hand")) {
	cursor_width = hand_width;
	cursor_height = hand_height;
	cursor_x_hot = 0;
	cursor_y_hot = 1;
	bits = hand_bits;
    }
    else if(!strcasecmp(cursor_type, "move")) {
	cursor_width = move_width;
	cursor_height = move_height;
	cursor_x_hot = 8;
	cursor_y_hot = 8;
	bits = move_bits;
    }
    else if(!strcasecmp(cursor_type, "crosshair") ||
		!strcasecmp(cursor_type, "plus")) {
	cursor_width = plus_width;
	cursor_height = plus_height;
	cursor_x_hot = 7;
	cursor_y_hot = 8;
	bits = plus_bits;
    }
    else if(!strcasecmp(cursor_type, "resize up")) {
	cursor_width = resize_up_width;
	cursor_height = resize_up_height;
	cursor_x_hot = 8;
	cursor_y_hot = 1;
	bits = resize_up_bits;
    }
    else if(!strcasecmp(cursor_type, "resize down")) {
	cursor_width = resize_down_width;
	cursor_height = resize_down_height;
	cursor_x_hot = 8;
	cursor_y_hot = 16;
	bits = resize_down_bits;
    }
    else if(!strcasecmp(cursor_type, "resize right")) {
	cursor_width = resize_right_width;
	cursor_height = resize_right_height;
	cursor_x_hot = 16;
	cursor_y_hot = 7;
	bits = resize_right_bits;
    }
    else if(!strcasecmp(cursor_type, "resize left")) {
	cursor_width = resize_left_width;
	cursor_height = resize_left_height;
	cursor_x_hot = 1;
	cursor_y_hot = 7;
	bits = resize_left_bits;
    }
    else if(!strcasecmp(cursor_type, "arrow")) {
	cursor_width = arrow_width;
	cursor_height = arrow_height;
	cursor_x_hot = 1;
	cursor_y_hot = 1;
	bits = arrow_bits;
    }
    else {
	fprintf(stderr, "CreateCursor: unknown cursor type: %s\n", cursor_type);
	return (Cursor)-1;
    }
    w_pixmap = XCreateBitmapFromData(XtDisplay(w), XtWindow(w),
			(char *)bits, cursor_width, cursor_height);

    cmap = XDefaultColormapOfScreen(XtScreen(w));

    cfore.pixel = fg;
    cfore.pixel = getApplication()->stringToPixel("black");
    XQueryColor(XtDisplay(w), cmap, &cfore);

    cursor = XCreatePixmapCursor(XtDisplay(w), w_pixmap, w_pixmap,
			&cfore, &cback, cursor_x_hot, cursor_y_hot);

    return(cursor);
}

/** Get an Icon Pixmap.
 *  @param[in] w a Widget for determining the XtScreen and XtDisplay
 *  @returns a Pixmap for use as a Shell XmNiconPixmap
 */
Pixmap Application::getIconPixmap(Widget w)
{
    if( !make_icon ) return (Pixmap)NULL;
	
    int i;
    for(i = 0; i < num_displays; i++) {
	if(XtDisplay(w) == display_list[i].display) break;
    }
    if(i == num_displays) {
	cerr << "Application.getCursor cannot find display for "
		<< XtName(w) << endl;
	return (Pixmap)NULL;
    }

    if(display_list[i].icon_pixmap == (Pixmap)NULL)
    {
	display_list[i].icon_pixmap = XCreatePixmap(XtDisplay(w),
		XtScreen(w)->root, 16, 16,
                DefaultDepth(XtDisplay(w),DefaultScreen(XtDisplay(w))));

	(*make_icon)(XtDisplay(w), display_list[i].icon_pixmap, 16);
    }
    return display_list[i].icon_pixmap;
}

/** Set the routine that makes the program icon.
 *  @param[in] makeIcon
 */
void Application::setIconRoutine(MakeIconRoutine makeIcon)
{
    make_icon = makeIcon;

    for(int i = 0; i < num_displays; i++) {
	if(display_list[i].icon_pixmap) {
	    XFreePixmap(display_list[i].display, display_list[i].icon_pixmap);
	    display_list[i].icon_pixmap = (Pixmap)NULL;
	}
    }
}

/** Set the default X-resources.
 */
void Application::setDefaultResources(void)
{
    char *prop, line[2000];
    XrmDatabase db = NULL, display_db;
    const char *name = resourceName();
    const char *c;

    if((c = getInstallDir("GEOTOOL_HOME")) != NULL)
    {
	snprintf(line, sizeof(line), "*geoTableDir: %s/tables", c);
	putResourceLine(&db, name, line);

	snprintf(line, sizeof(line), "*iaspeiTable: %s/tables/models/iasp91",c);
	putResourceLine(&db, name, line);
    }
    else {
	cerr << 
"Environment variable GEOTOOL_HOME is not set.\nSome options will not be available." << endl;
    }

    putResourceLine(&db, name, "*XmText.marginHeight: 2");
    putResourceLine(&db, name, "*toolBar*fontList: 8x13");
    putResourceLine(&db, name, "*toolBar*marginHeight: 1");
    putResourceLine(&db, name, "*toolBar*highlightThickness: 0");
    putResourceLine(&db, name, "*toolBar*traversalOn: False");
    putResourceLine(&db, name, "*Print Waveforms*marginHeight: 1");
    putResourceLine(&db, name, "*Print Waveforms*highlightThickness: 0");
    putResourceLine(&db, name, "*Print Waveforms*traversalOn: False");
    putResourceLine(&db, name, "*menuBar.marginHeight: 0");
    putResourceLine(&db, name, "*menuBar.marginWidth: 0");
    putResourceLine(&db, name, "*pane.marginHeight: 0");
    putResourceLine(&db, name, "*pane.marginWidth: 0");
    putResourceLine(&db, name,
	"*fontList:-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");
/*
putResourceLine(&db, name,
"*preview*tagFont:-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*");
putResourceLine(&db, name,
"*preview*arrivalFont:-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*");
putResourceLine(&db, name,
"*preview*axesFont:-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*");
*/

    putResourceLine(&db, name, "*MmTable*font: 8x13");
    putResourceLine(&db, name,
	"*Info*font: -adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*");

    putResourceLine(&db, name, "*background: grey85");
    putResourceLine(&db, name, "*foreground: black");
    putResourceLine(&db, name, "*CPlot*background: White");
    putResourceLine(&db, name, "*CPlot*scrollVert*background: grey85");
    putResourceLine(&db, name, "*CPlot*scrollHoriz*background: grey85");
    putResourceLine(&db, name, "*TtPlot*background: White");
    putResourceLine(&db, name, "*TtPlot*scrollVert*background: grey85");
    putResourceLine(&db, name, "*TtPlot*scrollHoriz*background: grey85");
    putResourceLine(&db, name, "*MapPlot*scrollVert*background: grey85");
    putResourceLine(&db, name, "*MapPlot*scrollHoriz*background: grey85");

    putResourceLine(&db, name, "*MmTable*tableBackground: white");
    putResourceLine(&db, name, "*MmTable*foreground: black");
    putResourceLine(&db, name, "*MmTable*borderWidth: 0");
    putResourceLine(&db, name, "*XmToggleButton.visibleWhenOff: true");

    putResourceLine(&db, name,
	"*menuBar*Close.accelerators: #override Ctrl<Key>q: ArmAndActivate()");
    putResourceLine(&db, name, "*menuBar*Close.acceleratorText:  Ctrl+q");
    putResourceLine(&db, name,
	"*menuBar*Print.accelerators: #override Ctrl<Key>p: ArmAndActivate()");
    putResourceLine(&db, name, "*menuBar*Print.acceleratorText:  Ctrl+p");
    putResourceLine(&db, name,
	"*menuBar*Compute.accelerators:#override Ctrl<Key>c:ArmAndActivate()");
    putResourceLine(&db, name, "*menuBar*Compute.acceleratorText:Ctrl+c");
    putResourceLine(&db, name,
	"*menuBar*Apply.accelerators: #override Ctrl<Key>a: ArmAndActivate()");
    // to avoid "Warning: Actions not found: delete-next-character"
    putResourceLine(&db, name, "*Text.translations:");
    // to avoid "Warning: Actions not found: delete-next-character
    putResourceLine(&db, name, "*Command.translations:");
/*
    putResourceLine(&db, name,
    "*Locate_Details_text.fontList: -*-courier-medium-r-*-*-14-*-*-*-*-*-*-*");
*/
    if( (prop = getProperty("verticalScrollPosition")) ) {
	snprintf(line, sizeof(line), "*verticalScrollPosition: %s", prop);
	putResourceLine(&db, name, line);
	Free(prop);
    }

    /* display_db will override db.
     */
    display_db = XrmGetDatabase(XtDisplay(base_widget));
    XrmCombineDatabase(db, &display_db, False);
    XrmSetDatabase(XtDisplay(base_widget), display_db);
}

void Application::putResourceLine(XrmDatabase *db, const char *name,
				const char *line)
{
    char buf[2000];

    snprintf(buf, sizeof(buf), "%s%s", name, line);
    XrmPutLineResource(db, buf);
}

/** Read program properties from a file. Properties are also obtained
 *  from the command-line arguments. These override the properties in
 *  the file.
 *  @param[in] filename the name of a properties file.
 */
void Application::readApplicationProperties(const string &filename)
{
    readProgramProperties(nargcp, arg_v, filename);
}

void Application::readProgramProperties(int argc, const char **argv,
		const string &filename)
{
    char file[MAXPATHLEN+1], *prop_file=NULL, *home=NULL;

    if((prop_file = stringGetArgv(argc, argv, "properties")) != NULL)
    {
	snprintf(file, sizeof(file), "%s", prop_file);
	free(prop_file);
    }
    else
    {
	if((home = getenv("HOME")) != NULL && (int)strlen(home) > 0) {
	    snprintf(file, sizeof(file), "%s/%s", home, filename.c_str());
	}
	else {
	    snprintf(file, sizeof(file), "%s", filename.c_str());
	}
    }

    readPropertiesFile(file);

    for(int i = 1; i < argc; i++) {
	char tmp[1024], *name, *value;
	stringcpy(tmp, argv[i], 1024);
	if(getProp(tmp, &name, &value)) {
	    string s(name);
	    tmp_properties.put(name, &s);
	}
    }
    properties_file.assign(file);
    properties.put("properties_file", properties_file);
}

void Application::readPropertiesFile(const string &file)
{
    char *line, *name, *value;
    FILE *fp=NULL;

    if((fp = fopen(file.c_str(), "r")) == NULL) {
	char msg[2000];
	snprintf(msg, sizeof(msg), "Cannot open %s\n%s\n",
		file.c_str(), strerror(errno));
	logErrorMsg(LOG_ERR, msg);
	return;
    }

    while((line = getPropLine(fp)) != NULL)
    {
	if(getProp(line, &name, &value)) {
	    string s(value);
	    properties.put(name, &s);
	}
	if(line) free(line);
    }
    fclose(fp);
}

static char *
getPropLine(FILE *fp)
{
    int c, n = 0;
    char *line = (char*)malloc(1);

    line[0] = '\0';

    while((c = fgetc(fp)) != EOF)
    {
	if(c == '\n') return line;

	line = (char *)realloc(line, n+2);
	if(line == NULL) {
	    char msg[100];
	    snprintf(msg, sizeof(msg),
		"getPropLine: malloc error. line_length=%d",n);
	    logErrorMsg(LOG_ERR, msg);
	    if(line) free(line);
	    return NULL;
	}
	line[n++] = (char)c;
	line[n] = '\0';
    }

    if(!n && line) {free(line); line = NULL;}
    return line;
}

static int
getProp(char *line, char **name, char **value)
{
    char *a, *b, *equals;

    if((equals = strstr(line, "=")) == NULL) return 0;

    *equals = '\0';

    for(a = line; *a != '\0' && isspace((int)*a); a++);
    if(*a == '\0') return 0;

    for(b = equals-1; b >= a && isspace((int)*b); b--);
    *(b+1) = '\0';
    *name = a;

    for(a = equals+1; *a != '\0' && isspace((int)*a); a++);
    if(*a == '\0') return 0;

    for(b = a+(int)strlen(a)-1; b >= a && isspace((int)*b); b--);
    *(b+1) = '\0';
    *value = a;

    return 1;
}

/** Write program properties to the properties file.
 */
void Application::writeApplicationProperties(void)
{
    Application *app = Application::getApplication();
    FILE *fp;
    char *file = getProperty("properties_file");

    if(!file) {
	char msg[2000];
	snprintf(msg, sizeof(msg),
		"writeApplicationProperties: no properties_file\n");
	logErrorMsg(LOG_ERR, msg);
	return;
    }
    if((fp = fopen(file, "w")) == NULL) {
	char msg[2000];
	snprintf(msg, sizeof(msg),
		"writeApplicationProperties: cannot open %s\n%s",
		file, strerror(errno));
	logErrorMsg(LOG_WARNING, msg);
	free(file);
	return;
    }
    free(file);

    for(int i = 0; i < (int)app->properties.elements.size(); i++) {
	// skip blank keys and blank values.
	if( !app->properties.elements[i]->first->empty() &&
		!app->properties.elements[i]->second->empty())
	{
	    fprintf(fp, "%s=%s\n", app->properties.elements[i]->first->c_str(),
			app->properties.elements[i]->second->c_str());
	}
    }
    fclose(fp);
}

// static
ResourceManager * Application::getResourceManager(Component *comp)
{
    Application *app = Application::getApplication();
    if( !app->rm ) {
	app->rm = new ResourceManager(app);
    }
    app->rm->display = XtDisplay(comp->baseWidget());
    app->rm->display_db = XrmGetDatabase(app->display());
    app->rm->db = NULL;
    return app->rm;
}

ResourceManager::ResourceManager(Application *app)
{
    // get the actual program name to use in the resources lines.
    const char *c = (app->nargcp > 0) ? app->arg_v[0] : "";

    int i;
    for(i = (int)strlen(c)-1; i >= 0 && c[i] != '/'; i--);
    name = (i >= 0) ? strdup(c + i+1) : strdup(c);
    db = NULL;
    display = NULL;
}

ResourceManager::~ResourceManager(void)
{
    Free(name);
}

void ResourceManager::putResource(const char *res_line)
{
    char buf[2000];

    if( !display ) {
	cerr << "ResourceManager::putResource() called after close()." << endl;
	cerr << "Cannot load resources." << endl;
	return;
    }
    snprintf(buf, sizeof(buf), "%s%s", name, res_line);
    XrmPutLineResource(&db, buf);
}

void ResourceManager::close(void)
{
    XrmCombineDatabase(db, &display_db, False);
    XrmSetDatabase(display, display_db);
    display = NULL;
}

// static
/** Return a directory environment variable. If the variable is not found,
 *  or does not point to an existing directory, return the installation
 *  directory.
 *  @param[in] env_variable the name of an environment variable.
 *  @returns the value of the variable or the installation directory.
 */
const char * Application::getInstallDir(const string &env_variable)
{
    const char *dir;

    if( (dir = (char *)getenv(env_variable.c_str())) ) {
	DIR *dirp;
	if( (dirp = opendir(dir)) ) {
	    closedir(dirp);
	    return dir;
	}
    }
    // Must be static and unique for each different env_variable for putenv. 
    // This assumes that this routine is always called with the same argument.
    static char line[MAXPATHLEN+100];

    dir = getApplication()->install_dir.c_str();
    snprintf(line, sizeof(line), "%s=%s", env_variable.c_str(), dir);
    putenv(line);
    
    return dir;
}

void Application::addBasicMap(BasicMap *map)
{
    maps.push_back(map);

    // need this timeout to allow the new window to finish it's initialization
    XtAppAddTimeOut(app_context, 0, newMapCB, (XtPointer)map);
}

static void newMapCB(XtPointer data, XtIntervalId *id)
{
    Application::getApplication()->doMapCallback((BasicMap *)data);
}

void Application::doMapCallback(BasicMap *map)
{
    ActionEvent *a;
    Component *map_parent = map->getParent();

    // Only do the callback for Components with the same parent
    // as the new BasicMap.
    for(int i = 0; i < (int)map_listeners.size(); i++) {
	if(map_parent == map_listeners.at(i)->_parent) {
	    a = new ActionEvent(map, map->baseWidget(), command_string,
				NULL, XtNmapCallback);
	    map_listeners.at(i)->_listener->actionPerformed(a);
	    a->deleteObject();
	}
    }
/*
    if(listeners.get(XtNanyMapCallback, &v)) {
        for(int i = 0; i < (int)v->size(); i++) {
            ActionListener *comp = v->at(i);
	    a = new ActionEvent(map, map->baseWidget(), command_string,
				NULL, XtNanyMapCallback);
	    comp->actionPerformed(a);
	    a->deleteObject();
        }
    }
*/
}

BasicMap * Application::getBasicMap(Component *comp)
{
    Component *cp_parent = comp->getParent();

    for(int i = 0; i < (int)maps.size(); i++) {
	if(maps.at(i)->getParent() == cp_parent) {
	    return maps.at(i);
	}
    }
    return NULL;
}

/*
BasicMap * Application::getAnyBasicMap(Component *comp)
{
    Component *cp_parent = comp->getParent();

    for(int i = 0; i < (int)maps.size(); i++) {
	if(maps.at(i)->getParent() == cp_parent) return maps.at(i);
    }

    if(maps.size() > 0) return maps.at(0);

    return NULL;
}
*/
