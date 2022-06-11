/** \file libgmap.cpp
 *  \brief Defines libgmap plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "MapWindow.h"

using namespace libgmap;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Map", "Map", createPlugin},
    {NULL, "Locate", NULL, "Map", "Map", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gmapPlugin : public PlugIn
{
    public :
	gmapPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    map = NULL;
	}
	~gmapPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    tw_parent->setCursor("hourglass");
	    if(!map) map = new MapWindow("Map", tw_parent, ds);
	    map->setVisible(true);
	    tw_parent->setCursor("default");
	}

	Frame *getFrame(void) { return map; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!map) map = new MapWindow("Map", tw_parent, ds);
	    return map->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!map) map = new MapWindow("Map", tw_parent, ds);
	    return map->parseVar(name, value);
	}
	Button *button;

    protected :
	MapWindow *map;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gmapPlugin *p = new gmapPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if((frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("Map...", option_menu, -1, p);
	}
	else {
	    cerr << "libgmap: cannot create Map Button. " << endl;
	}
    }
    return p;
}
