/** \file libExample4.cpp
 *  \brief Defines Example4 plugin.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/MotifClasses.h"
#include "motif++/PlugIn.h"

#include "Example4.h"

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, PLUGIN_STRING, PLUGIN_DESCRIPTION, createPlugin}
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class PLUGIN_CLASS : public PlugIn
{
    public :
	PLUGIN_CLASS(TopWindow *tw_parent, DataSource *ds, DataReceiver *dr)
		: PlugIn(tw_parent, ds, dr)
	{
	    button = NULL;
	    plugin_window = NULL;
	}
	~PLUGIN_CLASS(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(plugin_window) plugin_window->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!plugin_window) {
		plugin_window = new PLUGIN_NAME(PLUGIN_STRING, tw_parent, ds);
	    }
	    plugin_window->setVisible(true);
	}

	Frame *getFrame(void) { return plugin_window->getFrameInstance(); }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!plugin_window) {
		plugin_window = new PLUGIN_NAME(PLUGIN_STRING, tw_parent, ds);
	    }
	    return plugin_window->parseCmd(cmd, msg);
	}

	ParseVar parseVar(const string &name, string &value) {
	    if(!plugin_window) {
		plugin_window = new PLUGIN_NAME(PLUGIN_STRING, tw_parent, ds);
	    }
	    return plugin_window->parseVar(name, value);
	}
	Button *button;

    protected :
	PLUGIN_NAME *plugin_window;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
		bool create_button, DataSource *ds, DataReceiver *dr)
{
    PLUGIN_CLASS *p = new PLUGIN_CLASS(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button(PLUGIN_BUTTON_NAME, option_menu, -1, p);
	}
	else {
	    cerr << "Cannot add " << PLUGIN_BUTTON_NAME << " button. " << endl;
	}
    }
    return p;
}
