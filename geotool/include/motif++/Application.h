#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "motif++/AppParse.h"
#include "motif++/Warnings.h"
#include "motif++/PlugInManager.h"
#include "motif++/UndoManager.h"
#include "motif++/MotifClasses.h"

typedef void (*MakeIconRoutine)(Display *display, Drawable d, int size);
typedef void (*CleanUpMethod)(void);

#define XtNchangePropertyCallback (char *)"changePropertyCallback"
#define XtNformDialogCallback (char *)"formDialogCallback"
#define XtNmapCallback (char *)"mapCallback"

class MapListener
{
    public:
	MapListener(ActionListener *listener, Component *parent) {
	    _listener = listener;
	    _parent = parent;
	}
	ActionListener *_listener;
	Component *_parent;
};

class Component;
class BasicMap;
class FormDialog;
class ResourceManager;
class TextDialog;
class Warnings;

/** The top-level class for an application.
 *  An instance of the Application class is normally the top-level parent of
 *  an application. It creates and realizes a toplevel widget with
 *  XtAppInitialize and XtRealizeWidget. It creates a PlugInManager and an
 *  UndoManager. It sets the default X-resources and reads the program
 *  properties.
 *
 *  Application class callbacks:
 *     - XtNchangePropertyCallback called when any program property is changed.
 *		The callback data is the name of the property that was changed.
 *     - XtNformDialogCallback called when a FormDialog is created or destroyed.
 *		The callback data is either 1 for created or 0 for destroyed.
 *
 *  The following lines of code illustrate the usage of the Application class.
 * \code
   #include "motif++/Application.h"
   #include "WaveformWindow.h"
  
   int main(int argc, const char **argv)
   {
  	char version = "geotool++ version 1.1";
  
  	Application *app = new Application("geotool", &argc, argv, version);
  
  	app->setDefaultResources();

  	app->readApplicationProperties(".geotool++/geotool");
  
  	WaveformWindow *mw = new WaveformWindow("geotool", app);
  	mw->setVisible(true);
  
  	app->handleEvents();
  
  	return 0;
   }\endcode
 * @ingroup libmotif
 */
class Application : public AppParse
{
    // Allow FormDialog, Component and Warnings to access protected functions
    friend class FormDialog;
    friend class Component;
    friend class Warnings;
    friend class ResourceManager;

    public:

	Application(const string &name, int *argc, const char **argv,
			const string &version_str="No version information.",
			const string &installation_dir="",
			const string &only_plugins="");
	Application(const string &name, int *argc, const char **argv,
			CleanUpMethod clean_up_method,
			const string &version_str="No version information.",
			const string &installation_dir="",
			const string &only_plugins="");
	~Application(void);

	virtual Application *getApplicationInstance(void) { return this; }

	void setVisible(bool);
	void iconify(void);
	virtual void handleEvents(void);

	/** Make the program icon */
	void setIconRoutine(MakeIconRoutine makeIcon);

	/** Set the default X-resources. */
	void setDefaultResources(void);

	/** Get the primary X Display for the application. */
	Display	*display(void) { return XtDisplay(base_widget); }
	/** Get the Application Context. */
	XtAppContext appContext(void) { return (app_context); }
	/** Get the Application Class name. */
	const char *applicationClass(void) {return(application_class.c_str());}

	/** Set the cursor type for all windows. */
	void setCursor(const string &cursor_type);
	/** Get the Warnings window */
	Warnings *warningsWindow(void);
	/** Get the Message window */
	TextDialog *logWindow(void);
	/** Get the base widget for the Warnings popup window */
	Widget warningsWidget(void) {
	    return (warnings) ? warnings->baseWidget() : NULL; }
	/** Get the FormDialog with the specified name */
	Component *getWindow(const string &name);
	/** Get the first window that was opened. */
	Component *getFirstWindow(void) { return first_window; }
	/** Get all TopWindows */
	vector<Component *> *getWindows(void) { return &windows; }
	/** Get the FormDialog with the specified baseWidget */
	Component *getWindow(Widget base_widget);
	/** Get all instances of the specified class. */
	vector<Component *> *getComponents(void);

	/** Get the PlugInManager instance. */
	PlugInManager *plugInManager(void) { return plug_in_manager; }
	/** Get the UndoManager instance. */
	UndoManager *undoManager(void) { return undo_manager; }
	/** Add an UndoAction. */
	void addUndoAction(UndoAction *undo) {
	    undo_manager->addUndoAction(undo);
	}
	/** Return the number of visible FormDialog instances. */
	int numVisibleWindows(void);
	/** Get the top shell Widget for the display name. */
	Widget getDisplayParent(const string &name);
	/** Get the program version information */
	const char *versionInfo(void) { return version_info.c_str(); }
	/** Get the program release information */
	const char *releaseInfo(void) { return release_info.c_str(); }

	void addBasicMap(BasicMap *map);
	BasicMap *getBasicMap(Component *comp);
//	BasicMap *getAnyBasicMap(Component *comp);
        void addMapActionListener(ActionListener *listener, Component *parent) {
	    map_listeners.push_back(new MapListener(listener, parent));
	}

	Pixmap getIconPixmap(Widget w);

	/** Read program properties from a file. */
	void readApplicationProperties(const string &filename);

	/** Write program properties to the properties file. */
	static void writeApplicationProperties(void);

	static Application *getApplication(void);
	/** Get the X Application Context */
	static XtAppContext getAppContext(void) {
	    return getApplication()->app_context;
	}
	static void displayWarnings(void);
	static void displayLog(void);
	/** Add a listener for the XtNchangePropertyCallback action. */
	static void addPropertyListener(ActionListener *listener) {
		getApplication()->addActionListener(listener,
			XtNchangePropertyCallback);
	}
	/** Add a listener for the XtNformDialogCallback action. */
	static void addFormDialogListener(ActionListener *listener) {
		getApplication()->addActionListener(listener,
			XtNformDialogCallback);
	}
	/** Add a listener for the XtNmapCallback action. */
	static void addMapListener(ActionListener *listener, Component *parent){
		getApplication()->addMapActionListener(listener, parent);
	}
	/** Add a listener for the XtNanyMapCallback action. */
/*
	static void addAnyMapListener(ActionListener *listener) {
		getApplication()->addActionListener(listener,XtNanyMapCallback);
	}
*/
	/** Return the top-level Widget for the input display specification */
	static Widget displayParent(const string &name) {
		return getApplication()->getDisplayParent(name);
	}
	/** Get the version information string. */
	static const char *getVersion(void) { return getApplication()->versionInfo();}

	/** Get the release information string. */
	static const char *getRelease(void) { return getApplication()->releaseInfo();}

	/** Get the installation directory. */
	static const char *getInstallDir(void) {
		return getApplication()->installationDir();}
	/** Get an environment variable, or the installation directory */
	static const char *getInstallDir(const string &env_variable);

	/** Get the PlugInManager instance. */
	static PlugInManager *getPlugInManager(void) {
		return getApplication()->plug_in_manager; }

	/** Get a ResourceManager instance. */
	static ResourceManager *getResourceManager(Component *comp);

	static void addMap(BasicMap *map) {getApplication()->addBasicMap(map);} 
	static BasicMap *getMap(Component *c) {
		return getApplication()->getBasicMap(c); } 
//	static BasicMap *getAnyMap(Component *c) {
//		return getApplication()->getAnyBasicMap(c); } 
	void doMapCallback(BasicMap *map);

	static bool writeToDB(void) {
	    return getApplication()->write_to_db;
	}
	static void setWriteToDB(bool set) {
	    getApplication()->write_to_db = set;
	}
	void readProgramProperties(int argc, const char **argv,
		const string &filename);

    protected:

	vector<Component *> windows; //!< a list of all FormDialog instances
	Component *first_window; //!< the first window to be realized.
	string version_info; //!< program version information.
	string release_info; //!< program release information.
	bool write_to_db; //!< permission to write to ODBC interface
	string first_window_name; //!< the first window name.

	/** redirect-warnings flag. If true, warnings go to stdout. */
	bool redirect_warnings;
	Warnings *warnings; //!< the Warnings popup window
	TextDialog *log_window; //!< the Log popup window

	PlugInManager *plug_in_manager; //!< a PluginManager instance.

	vector<BasicMap *> maps;
	vector<MapListener *> map_listeners;

	string properties_file;
	ghashtable<string *> properties;
	ghashtable<string *> tmp_properties;

	UndoManager *undo_manager; //!< an UndoManager instance.

	void init(const string &name, int *argcp, const char **argv,
			const string &version, const string &only_plugins);
	void initMembers(void);
	void makePropertiesDir(void);
	void cleanUp(void);
	void getDescendants(vector<Component *> *v, Component *comp);
	Cursor getCursor(Widget w, const string &type);
	void readPropertiesFile(const string &file);

    private:

#define MAX_NUM_DISPLAYS 10
	//! display_list contains members that are dependent on the X-Display.
	struct {
	    Display *display;	//!< the X-Display pointer
	    char *name;		//!< the Display specification (example: "0.1")
	    Widget parent;	//!< the top-level Window for the Display
	    Pixmap icon_pixmap;

		//! display dependent "hourglass" cursor
	    Cursor hourglass_cursor;
		//! display dependent "hand" cursor
	    Cursor hand_cursor;
		//! display dependent "move" cursor
	    Cursor move_cursor;
		//! display dependent "crosshair" cursor
	    Cursor crosshair_cursor;
		//! display dependent "resize up" cursor
	    Cursor resize_up_cursor;
		//! display dependent "resize down" cursor
	    Cursor resize_down_cursor;
		//! display dependent "resize right" cursor
	    Cursor resize_right_cursor;
		//! display dependent "resize left" cursor
	    Cursor resize_left_cursor;
		//! display dependent "arrow" cursor
	    Cursor arrow_cursor;
	} display_list[MAX_NUM_DISPLAYS];

	int num_displays; //!< the number of active X-Display's

	MakeIconRoutine make_icon; //!< routine that draws the program icon

	ResourceManager *rm;

    private:

	CleanUpMethod clean_up;
	void createDisplayCursors(int i);
	Cursor createCursor(const char *type, Pixel fg);
	static void redirectCallback(Widget, XtPointer, XtPointer);
	static Cursor createCursor(Widget w, const char *type, Pixel fg);
	void putResourceLine(XrmDatabase *db, const char *name,
				const char *line);

	// Functions for registering and unregistering top-level windows

	void registerWindow(Component *);
	void unregisterWindow(Component *);
};

class ResourceManager
{
    public :
	ResourceManager(Application *app);
	~ResourceManager(void);
	void putResource(const char *res_line);
	void close(void);

	XrmDatabase db, display_db;
	Display *display;
	char *name;
};

#endif
