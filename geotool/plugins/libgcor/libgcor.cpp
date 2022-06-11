/** \file libgcor.cpp
 *  \brief Defines libgcor plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Correlation.h"

using namespace libgcor;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Correlation", "Waveform Correlation",
		createPlugin}
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Correlation PlugIn subclass.
 *  @ingroup libgcor
 */
class gcorPlugin : public PlugIn
{
    public :
	gcorPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    gcor = NULL;
	}
	~gcorPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gcor) gcor = new Correlation("Correlation", tw_parent, ds);
	    gcor->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gcor) gcor = new Correlation("Correlation", tw_parent, ds);
	    gcor->setVisible(true);
	}

	Frame *getFrame(void) { return gcor; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gcor) gcor = new Correlation("Correlation", tw_parent, ds);
	    return gcor->parseCmd(cmd, msg);
	}

	ParseVar parseVar(const string &name, string &value) {
	    if(!gcor) gcor = new Correlation("Correlation", tw_parent, ds);
	    return gcor->parseVar(name, value);
	}
	Button *button;

    protected :
	Correlation *gcor;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gcorPlugin *p = new gcorPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *cor_menu = option_menu->getMenu("Correlation");
	    if(!cor_menu) {
		cor_menu = new Menu("Correlation", option_menu, -1);
	    }
	    p->button = new Button("Basic Correlation...", cor_menu, -1, p);
	}
	else {
	    cerr << "libgcor: cannot create Correlation Button. " << endl;
	}
    }
    return p;
}
