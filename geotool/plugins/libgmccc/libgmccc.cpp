/** \file libgmccc.cpp
 *  \brief Defines libgmccc plugins.
 */
#include "config.h"

#ifdef HAVE_GSL

#include "motif++/PlugIn.h"
#include "MultiChannelCC.h"

using namespace libgmccc;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Multi Channel Cross Correlation",
     "Waveform Multi Channel Cross Correlation", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gmcccPlugin : public PlugIn
{
    public :
	gmcccPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gmccc = NULL;
	}
	~gmcccPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gmccc) {
		gmccc =new MultiChannelCC("Multi Channel Cross Correlation",
					tw_parent, ds);
	    }
	    gmccc->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *a)
	{
	    if(!gmccc) {
		gmccc =new MultiChannelCC("Multi Channel Cross Correlation",
					tw_parent, ds);
	    }
	    gmccc->setVisible(true);
	}

	Frame *getFrame(void) { return gmccc; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gmccc) {
		gmccc =new MultiChannelCC("Multi Channel Cross Correlation",
					tw_parent, ds);
	    }
	    return gmccc->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gmccc) {
		gmccc =new MultiChannelCC("Multi Channel Cross Correlation",
					tw_parent, ds);
	    }
	    return gmccc->parseVar(name, value);
	}
	Button *button;

    protected :
	MultiChannelCC *gmccc;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gmcccPlugin *p = new gmcccPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *cor_menu = option_menu->getMenu("Correlation");
	    if(!cor_menu) {
		cor_menu = new Menu("Correlation", option_menu, -1);
	    }
	    p->button = new Button("Multi Channel Cross Correlation...",
				cor_menu, -1, p);
	}
	else {
	    cerr << "libgmccc: cannot create Button. " << endl;
	}
    }
    return p;
}

#endif /* HAVE_GSL */
