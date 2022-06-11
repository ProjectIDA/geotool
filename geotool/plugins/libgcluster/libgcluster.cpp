/** \file libgcluster.cpp
 *  \brief Defines libgcluster plugins.
 */
#include "config.h"

#include "motif++/PlugIn.h"
#include "GCluster.h"

using namespace libgcluster;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Cluster", "Waveform Cluster", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gclusterPlugin : public PlugIn
{
    public :
	gclusterPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    gcluster = NULL;
	}
	~gclusterPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gcluster) gcluster = new GCluster("Cluster", tw_parent, ds);
	    gcluster->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *a)
	{
	    if(!gcluster) gcluster = new GCluster("Cluster", tw_parent, ds);
	    gcluster->setVisible(true);
	}

	Frame *getFrame(void) { return gcluster; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gcluster) gcluster = new GCluster("GCluster", tw_parent, ds);
	    return gcluster->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gcluster) gcluster = new GCluster("GCluster", tw_parent, ds);
	    return gcluster->parseVar(name, value);
	}
	Button *button;

    protected :
	GCluster *gcluster;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gclusterPlugin *p = new gclusterPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *cor_menu = option_menu->getMenu("Correlation");
	    if(!cor_menu) {
		cor_menu = new Menu("Correlation", option_menu, -1);
	    }
	    p->button = new Button("Cluster...", cor_menu, -1, p);
	}
	else {
	    cerr << "libgcluster: cannot create Cluster Button. " << endl;
	}
    }
    return p;
}
