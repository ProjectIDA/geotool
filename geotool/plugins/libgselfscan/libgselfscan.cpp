/** \file libgselfscan.cpp
 *  \brief Defines libgselfscan plugins.
 */
#include "config.h"

#include "motif++/PlugIn.h"
#include "SelfScan.h"

using namespace libgselfscan;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Self Scanning Correlation", "Self Scanning Correlation", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gselfscanPlugin : public PlugIn
{
    public :
	gselfscanPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gselfscan = NULL;
	}
	~gselfscanPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gselfscan) gselfscan = new SelfScan("Self Scanning Correlation", tw_parent, ds);
	    gselfscan->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *a)
	{
	    if(!gselfscan) gselfscan = new SelfScan("Self Scanning Correlation", tw_parent, ds);
	    gselfscan->setVisible(true);
	}

	Frame *getFrame(void) { return gselfscan; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gselfscan) gselfscan = new SelfScan("Self Scanning Correlation", tw_parent, ds);
	    return gselfscan->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gselfscan) gselfscan = new SelfScan("Self Scanning Correlation", tw_parent, ds);
	    return gselfscan->parseVar(name, value);
	}
	Button *button;

    protected :
	SelfScan *gselfscan;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gselfscanPlugin *p = new gselfscanPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *cor_menu = option_menu->getMenu("Correlation");
	    if(!cor_menu) {
		cor_menu = new Menu("Correlation", option_menu, -1);
	    }
	    p->button = new Button("Self Scanning Correlation...",
				cor_menu, -1, p);
	}
	else {
	    cerr << "libgselfscan: cannot create Self Scanning Correlation Button. " << endl;
	}
    }
    return p;
}
