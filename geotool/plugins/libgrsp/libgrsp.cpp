/** \file libgrsp.cpp
 *  \brief Defines libgrsp plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Resp.h"

using namespace libgrsp;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Instrument Response","Instrument Responses",
		createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Instrument Response PlugIn subclass.
 * @ingroup libgrsp
 */
class grspPlugin : public PlugIn
{
    public :
	grspPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
			PlugIn(tw, s, r)
	{
	    button = NULL;
	    rsp = NULL;
	}
	~grspPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!rsp) rsp = new Resp("Instrument Response", tw_parent, ds);
	    rsp->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!rsp) rsp = new Resp("Instrument Response", tw_parent, ds);
	    rsp->setVisible(true);
	}

	Frame *getFrame(void) { return rsp; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!rsp) rsp = new Resp("Instrument Response", tw_parent, ds);
	    return rsp->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!rsp) rsp = new Resp("Instrument Response", tw_parent, ds);
	    return rsp->parseVar(name, value);
	}
	Button *button;

    protected :
	Resp *rsp;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    grspPlugin *p = new grspPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("Response...", option_menu, -1, p);
	}
	else {
	    cerr << "libgrsp: cannot create Response Button. " << endl;
	}
    }
    return p;
}
