/** \file libgorigin.cpp
 *  \brief Defines libgorigin plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Origins.h"

using namespace libgorigin;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Origins", "Origin Analysis", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class goriginPlugin : public PlugIn
{
    public :
	goriginPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    origin = NULL;
	}
	~goriginPlugin(void) { }

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!origin) origin = new Origins("Origins", tw_parent, ds);
	    origin->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!origin) origin = new Origins("Origins", tw_parent, ds);
	    origin->setVisible(true);
	}

	Frame *getFrame(void) { return origin; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!origin) origin = new Origins("Origins", tw_parent, ds);
	    return origin->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!origin) origin = new Origins("Origins", tw_parent, ds);
	    return origin->parseVar(name, value);
	}
	Button *button;

    protected :
	Origins *origin;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    goriginPlugin *p = new goriginPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("Origins...", option_menu, -1, p);
	}
	else {
	    cerr << "libgorigin: cannot create Origins Button. " << endl;
	}
    }
    return p;
}
