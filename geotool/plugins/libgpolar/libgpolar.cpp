/** \file libgpolar.cpp
 *  \brief Defines libgpolar plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Polarization.h"

using namespace libgpolar;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Polarization", "Polarization Analysis",
		createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gpolarPlugin : public PlugIn
{
    public :
	gpolarPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gpolar = NULL;
	}
	~gpolarPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gpolar) gpolar = new Polarization("Polarization", tw_parent,ds);
	    gpolar->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gpolar) gpolar = new Polarization("Polarization", tw_parent,ds);
	    gpolar->setVisible(true);
	}

	Frame *getFrame(void) { return gpolar; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gpolar) gpolar = new Polarization("Polarization", tw_parent,ds);
	    return gpolar->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gpolar) gpolar = new Polarization("Polarization", tw_parent,ds);
	    return gpolar->parseVar(name, value);
	}
	Button *button;

    protected :
	Polarization *gpolar;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gpolarPlugin *p = new gpolarPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *cp_menu = option_menu->getMenu("Three-Component Analysis");
	    if(!cp_menu) {
		cp_menu = new Menu("Three-Component Analysis", option_menu, -1);
	    }
	    p->button = new Button("Polarization...", cp_menu, -1, p);
	}
	else {
	    cerr << "libgpolar: cannot create Polarization Button. " << endl;
	}
    }
    return p;
}
