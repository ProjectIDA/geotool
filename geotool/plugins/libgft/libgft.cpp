/** \file libgft.cpp
 *  \brief Defines libgft plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/MotifClasses.h"
#include "FT.h"

using namespace libgft;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL,  "WaveformWindow", NULL, "FT", "Spectral Analysis", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The FT PlugIn subclass.
 *  @ingroup libgft
 */
class gftPlugin : public PlugIn
{
    public :
	gftPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    ft = NULL;
	}
	~gftPlugin(void) { }

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!ft) ft = new FT("FT", tw_parent, ds);
	    ft->setDataSource(ds);
	}

	Frame *getFrame(void) { return ft; }

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!ft) ft = new FT("FT", tw_parent, ds);
	    ft->setVisible(true);
	}

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!ft) ft = new FT("FT", tw_parent, ds);
	    return ft->parseCmd(cmd, msg);
	}

	ParseVar parseVar(const string &name, string &value) {
	    if(!ft) ft = new FT("FT", tw_parent, ds);
	    return ft->parseVar(name, value);
	}
	Button *button;

    protected :
	FT *ft;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
		bool create_button, DataSource *ds, DataReceiver *dr)
{
    gftPlugin *p = new gftPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if((frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *sp_menu = option_menu->getMenu("Spectral Analysis");
	    if(!sp_menu) {
		sp_menu = new Menu("Spectral Analysis", option_menu, -1);
	    }
	    p->button = new Button("FT...", sp_menu, -1, p);
	}
	else {
	    cerr << "libgft: cannot create FT Button. " << endl;
	}
    }
    return p;
}
