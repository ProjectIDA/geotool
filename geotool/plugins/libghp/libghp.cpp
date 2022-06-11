/** \file libghp.cpp
 *  \brief Defines libghp plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "motif++/Frame.h"
#include "HilbertTransform.h"
#include "PolarFilter.h"

using namespace libghp;

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);
static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Hilbert Transform", "Hilbert Transform",
		createPlugin1},
    {NULL, "WaveformWindow", NULL, "Polarization Filter", "Polarization Filter",
		createPlugin2},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Hilbert Transform PlugIn subclass.
 *  @ingroup libghp
 */
class hilbertPlugin : public PlugIn
{
    public :
	hilbertPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    hilbert = NULL;
	}
	~hilbertPlugin(void) {
	}
	void actionPerformed(ActionEvent *action_event)
	{
	    if(!hilbert) hilbert = new HilbertTransform("Hilbert Transform",
					tw_parent, ds);
	    hilbert->setVisible(true);
	}
	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!hilbert) hilbert = new HilbertTransform("Hilbert Transform",
					tw_parent, ds);
	    return hilbert->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!hilbert) hilbert = new HilbertTransform("Hilbert Transform",
					tw_parent, ds);
	    return hilbert->parseVar(name, value);
	}
	Button *button;

    protected :
	HilbertTransform *hilbert;
};

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    hilbertPlugin *p = new hilbertPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *sp_menu = option_menu->getMenu("Spectral Analysis");
	    if(!sp_menu) {
		sp_menu = new Menu("Spectral Analysis", option_menu, -1);
	    }
	    p->button = new Button("Hilbert Transform...", sp_menu, -1, p);
	}
	else {
	    cerr << "libghp: cannot create Hilbert Transform Button. " << endl;
	}
    }
    return p;
}

/** The Polarization Filter PlugIn subclass.
 *  @ingroup polarFilter
 */
class polarFiltPlugin : public PlugIn
{
    public :
	polarFiltPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    polar = NULL;
	}
	~polarFiltPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!polar) polar = new PolarFilter("Polarization Filter",
					tw_parent, ds);
	    polar->setDataSource(ds);
	}
	void actionPerformed(ActionEvent *action_event)
	{
	    if(!polar) polar = new PolarFilter("Polarization Filter",
					tw_parent, ds);
	    polar->setVisible(true);
	}
	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!polar) polar = new PolarFilter("Polarization Filter",
					tw_parent, ds);
	    return polar->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!polar) polar = new PolarFilter("Polarization Filter",
					tw_parent, ds);
	    return polar->parseVar(name, value);
	}
	Button *button;

    protected :
	PolarFilter *polar;
};

static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    polarFiltPlugin *p = new polarFiltPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *menu;
	    if((menu = frame->findMenu("Filter"))) {
		p->button = new Button("Polarization Filter...", menu, -1, p);
	    }
	    Menu *option_menu = frame->optionMenu();
	    Menu *cp_menu = option_menu->getMenu("Three-Component Analysis");
	    if(!cp_menu) {
		cp_menu = new Menu("Three-Component Analysis", option_menu, -1);
	    }
	    p->button = new Button("Polarization Filter...", cp_menu, -1, p);
	}
	else {
	    cerr << "libghp: cannot create Polarization Filter Button. " <<endl;
	}
    }
    return p;
}
