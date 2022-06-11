/** \file libgpm.cpp
 *  \brief Defines libgpm plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/MotifClasses.h"
#include "MotionPlot.h"

using namespace libgpm;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Particle Motion",
			"Particle Motion Analysis", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gpmPlugin : public PlugIn
{
    public :
	gpmPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gpm = NULL;
	}
	~gpmPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gpm) gpm = new MotionPlot("Particle Motion", tw_parent, ds);
	    gpm->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gpm) gpm = new MotionPlot("Particle Motion", tw_parent, ds);
	    gpm->setVisible(true);
	}

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gpm) gpm = new MotionPlot("Particle Motion", tw_parent, ds);
	    return gpm->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gpm) gpm = new MotionPlot("Particle Motion", tw_parent, ds);
	    return gpm->parseVar(name, value);
	}
	Button *button;

    protected :
	MotionPlot *gpm;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gpmPlugin *p = new gpmPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *cp_menu = option_menu->getMenu("Three-Component Analysis");
	    if(!cp_menu) {
		cp_menu = new Menu("Three-Component Analysis", option_menu, -1);
	    }
	    p->button = new Button("Particle Motion...", cp_menu, -1, p);
	}
	else {
	    cerr << "libgpm: cannot create Particle Motion Button. " << endl;
	}
    }
    return p;
}
