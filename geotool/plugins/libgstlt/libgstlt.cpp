/** \file libgstlt.cpp
 *  \brief Defines libgstlt plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "StaLta.h"

using namespace libgstlt;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "StaLta", "Sta Lta Detector", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gstltPlugin : public PlugIn
{
    public :
	gstltPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gstalta = NULL;
	}
	~gstltPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gstalta) gstalta = new StaLta("StaLta", tw_parent, ds);
	    gstalta->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gstalta) gstalta = new StaLta("StaLta", tw_parent, ds);
	    gstalta->setVisible(true);
	}

	Frame *getFrame(void) { return gstalta; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gstalta) gstalta = new StaLta("StaLta", tw_parent, ds);
	    return gstalta->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gstalta) gstalta = new StaLta("StaLta", tw_parent, ds);
	    return gstalta->parseVar(name, value);
	}
	Button *button;

    protected :
	StaLta *gstalta;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gstltPlugin *p = new gstltPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *det_menu = option_menu->getMenu("Detection");
	    if(!det_menu) {
		det_menu = new Menu("Detection", option_menu, -1);
	    }
	    p->button = new Button("StaLta...", det_menu, -1, p);
	}
	else {
	    cerr << "libgstlt: cannot create StaLta Button. " << endl;
	}
    }
    return p;
}
