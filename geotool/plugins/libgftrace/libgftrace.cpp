/** \file libgftrace.cpp
 *  \brief Defines libgftrace plugins.
 *  \author Vera Miljanovic
 */
#include "config.h"

#ifdef HAVE_GSL

#include "motif++/MotifClasses.h"
#include "Ftrace.h"

using namespace libgftrace;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Ftrace", "Ftrace Analysis",
		createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gftracePlugin : public PlugIn
{
    public :
	gftracePlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gftrace = NULL;
	}
	~gftracePlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gftrace) gftrace = new Ftrace("Ftrace", tw_parent, ds);
	    gftrace->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gftrace) gftrace = new Ftrace("Ftrace", tw_parent, ds);
	    gftrace->setVisible(true);
	}

	Frame *getFrame(void) { return gftrace; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gftrace) gftrace = new Ftrace("Ftrace", tw_parent, ds);
	    return gftrace->parseCmd(cmd, msg);
	}
	Button *button;

    protected :
	Ftrace *gftrace;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gftracePlugin *p = new gftracePlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *gbm_menu;
	    if( !(gbm_menu = option_menu->findMenu("Array Analysis")) ) {
		gbm_menu = new Menu("Array Analysis", option_menu, -1);
	    }
	    p->button = new Button("Ftrace...", gbm_menu, -1, p);
	}
	else {
	    cerr << "libgftrace: cannot create Ftrace Button. " << endl;
	}
    }
    return p;
}

#endif /* HAVE_GSL */
