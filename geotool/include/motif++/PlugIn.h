#ifndef _PLUG_IN_H
#define _PLUG_IN_H

#include "gobject++/Gobject.h"
#include "motif++/ActionListener.h"

class Component;
class Application;
class TopWindow;
class Frame;
class DataSource;
class DataReceiver;

/** A class that provides a shared library plugin interface. A plugin shared
 *  library must contain a C function named "plugInIndex" defined by the 
 *  PlugInIndex typedef. The Application will attempt to obtain a pointer to
 *  this function with dlsym and call the function with code like:
 \code
void *handle;
PlugInIndex index_method;
PlugInStruct *plugin_structs;

dlerror(); // clear error messages
if( !(handle = dlopen(shared_lib_path, RTLD_LAZY | RTLD_GLOBAL)) ) {
    fprintf(stderr, "dlopen failed for: %s\n%s\n", shared_lib_path, dlerror());
    return;
}
if( !(index_method = (PlugInIndex)dlsym(handle, "plugInIndex")) ) {
    fprintf(stderr, "dlsym failed for library: %s\n", shared_lib_path);
    fprintf(stderr, " and symbol \"plugInIndex\"\n%s\n", dlerror());
    dlclose(handle);
    return;
}
int num_plugins = (*index_method)( &plugin_structs );
\endcode

 *  The plugInIndex function returns information about the plugins contained
 *  in the shared library. The information about each plugin includes the
 *  intended recipient, which is specifed by any combination of the three
 *  identifiers, application name, parent name, and parent class. The
 *  plugInIndex function also returns a pointer to a creation routine for each
 *  plugin. The creation routine creates a Button subclass. The creation routine
 *  syntax is:
 \code
PlugIn *createPlugin(Application *app, Component *plugin_parent, TopWindow *top_window_parent, DataSource *ds, DataReceiver *dr);
\endcode
 *
 *  When this routine is called by the Application, a new Button will be created
 *  with plugin_parent as its parent. When the Button is activated, the
 *  plugin window will be created and displayed with top_window_parent as its
 *  parent. If the plugin_parent argument is NULL, then the plugin will choose
 *  the parent of the plugin Button from the children of the TopWindow.
 *
 *  The following example code shows how the shared library defines the PlugIn
 *  subclass (ftPlugin) and the createPlugin function. In this case, the library
 *  contains one PlugIn called "FT". FT is defined in another file. There are
 *  two elements in the PlugInStruct array, because there are two intended
 *  recipient types. The first recipient type is any TopWindow subclass of
 *  WaveformWindow. The second recipient type is any TopWindow subclass of tq
 *  in the gtq Application.
 *
 *  The ftPlugin class is a subclass of the Button class. If plugin_parent is
 *  NULL, the createPlugin function creates the ftPlugin instance as a child
 *  of the parent's Option menu. When the Button is first activated, the
 *  ftPlugin.actionPerformed function creates the FT window and sets it visible.
 *  The ftPlugin class also sets the DataSource for the FT window. The
 *  DataSource instance is passed to createPlugin and then to the ftPlugin
 *  constructor.
 \code
#include "motif++/PlugIn.h"

static PlugIn *createPlugin(Application *app, Component *parent, TopWindow *tw_parent, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "FT", "Spectral Analysis",  createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gftPlugin : public Button, public PlugIn
{
    public :
	gftPlugin(const char *name, Component *parent, TopWindow *tw_parent,
		DataSource *ds, DataReceiver *dr) :
		Button(name, parent, -1, this), PlugIn(tw_parent, ds, dr)
	{
	    ft = NULL;
	}
	~gftPlugin(void) { }

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(ft) ft->setDataSource(ds);
	}

	Frame *getFrame(void) { return ft; }

	void actionPerformed(ActionEvent *action_event)
	{
	    if(action_event->getSource() == this) {
		if(!ft) ft = new FT("FT", tw_parent, ds);
		ft->setVisible(true);
	    }
	}
	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!ft) ft = new FT("FT", tw_parent, ds);
	    return ft->parseCmd(cmd, msg);
        }
	ParseVar parseVar(const string &name, string &value) {
	    if(!ft) ft = new FT("FT", tw_parent, ds);
	    return ft->parseVar(name, value);
        }

    protected :
	FT *ft;
};

static PlugIn *createPlugin(Application *app, Component *parent,
			TopWindow *tw_parent, DataSource *ds, DataReceiver *dr)
{
    Frame *frame;

    if(parent) {
	return new gftPlugin("FT...", parent, tw_parent, ds, dr);
    }
    else if( tw_parent && (frame = tw_parent->getFrameInstance()) ) {
	Menu *option_menu = frame->optionMenu();
	return new gftPlugin("FT...", option_menu, tw_parent, ds, dr);
    }
    else {
	cerr << "libgft: cannot add FT plugin. " << endl;
	return NULL;
    }
}
\endcode
 *  @ingroup libmotif
 */
class PlugIn : public Gobject, public ActionListener
{
    protected:
	/** Constructor (for subclasses only).
	 *  @param[in] top_window the parent
	 *  @param[in] data_source a DataSource or NULL.
	 *  @param[in] data_receiver a DataReceiver or NULL.
	 */
	PlugIn(TopWindow *top_window, DataSource *datasource,
		DataReceiver *datareceiver) : tw_parent(top_window),
		ds(datasource), dr(datareceiver)
	{ }

    public:

	/** Destructor. */
	~PlugIn(void){};

	/** Set the plugin's DataSource. */
	virtual void setDataSource(DataSource *datasource) { ds = datasource;}
	/** Set the plugin's DataReceiver. */
	virtual void setDataReceiver(DataReceiver *datareceiver) { dr = datareceiver;}
	/** Get the plugin's DataSource. */
	virtual DataSource *getDataSource(void) { return ds; }
	/** Get the plugin's DataReceiver. */
	virtual DataReceiver *getDataReceiver(void) { return dr; }
	/** Get the plugin's TopWindow parent. */
	virtual TopWindow *getTopWindow(void) { return tw_parent; }
	/** Get the plugin's Frame. */
	virtual Frame *getFrame(void) { return NULL; }

    protected:
	TopWindow *tw_parent;
	DataSource *ds;
	DataReceiver *dr;
};

/** Shared libraries implement this function for each PlugIn. If the Application
 *  does not specify the parent of the PlugIn (plugin_parent == NULL), then
 *  the Plugin is free to create the plugin in any TopWindow menu.
 *  @param[in] app the Application receiving the PlugIn
 *  @param[in] top_window_parent the TopWindow parent of the PlugIn
 *  @param[in] create_button if true, create the PlugIn's Button or Toggle in
 *		the top_window_parent
 *  @param[in] ds the DataSource for the PlugIn (can be NULL)
 *  @param[in] dr the DataReceiver for the PlugIn (can be NULL)
 */
typedef PlugIn * (*CreatePlugIn)(Application *app, TopWindow *top_window_parent,
			bool create_button, DataSource *datasource,
			DataReceiver *datareceiver);

/** The specification of a single PlugIn. The shared library plugInIndex
 *  function returns this information for each of its PlugIn classes. The three
 *  identifiers, application_name, parent_class and parent_name can be used to
 *  specify the intended receiver of the PlugIn. Any or all of these
 *  identifiers can be NULL, in which case, they are ignored. If
 *  application_name is not NULL, then the plugin is only intended for
 *  Application instances with that name. If parent_class is not NULL, then the
 *  plugin is only intended for parents of that class. If parent_name is not
 *  NULL, then the plugin is only intended for parents with that name.
 */
typedef struct plugInStruct
{
    const char	 *application_name; //!< Intended Application name or NULL
    const char	 *parent_class;	    //!< Intended parent class or NULL
    const char	 *parent_name;	    //!< Intended parent name or NULL
    const char	 *name;		    //!< Plugin name
    const char	 *description;	    //!< Plugin description
    CreatePlugIn create_plugin;	    //!< Creates the PlugIn instance.
} PlugInStruct;

/** A plugin shared library must have a function named "plugInIndex" that is
 *  defined with this function typedef.
 *  @param[out] p a pointer to an array of PlugInStruct's.
 *  @returns the number of plugins in the library.
 */
extern "C" {
typedef int (*PlugInIndex)(PlugInStruct **p);
};

extern "C" {
#include "libstring.h"
}

#endif
