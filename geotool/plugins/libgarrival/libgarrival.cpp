/** \file libgarrival.cpp
 * \brief Defines libgarrival plugins.
 * \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Arrivals.h"

using namespace libgarrival;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
		bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Arrivals", "Arrival Analysis", createPlugin}
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Arrivals PlugIn subclass.
 * @ingroup libgarrival
 */
class garrivalPlugin : public PlugIn
{
    public :
	garrivalPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    amp_button = NULL;
	    arrival = NULL;
	}

	~garrivalPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!arrival) arrival = new Arrivals("Arrivals", tw_parent, ds);
	    arrival->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    const char *cmd = action_event->getActionCommand();
	    if(!arrival) arrival = new Arrivals("Arrivals", tw_parent, ds);
	    if(!strcmp(cmd, "Arrivals...")) {
		arrival->setVisible(true);
	    }
	    else {
		string msg;
		arrival->parseCmd("amplitudes_magnitudes.open", msg);
	    }
	}

	Frame *getFrame(void) { return arrival; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!arrival) arrival = new Arrivals("Arrivals", tw_parent, ds);
	    return arrival->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!arrival) arrival = new Arrivals("Arrivals", tw_parent, ds);
	    return arrival->parseVar(name, value);
	}
	Button *button;
	Button *amp_button;

    protected :
	Arrivals *arrival;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
		bool create_button, DataSource *ds, DataReceiver *dr)
{
    garrivalPlugin *p = new garrivalPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("Arrivals...", option_menu, -1, p);
	    p->amp_button = new Button("Amplitudes/Magnitudes...", option_menu,
					-1, p);
	}
	else {
	    cerr << "libgarrival: cannot create Buttons. " << endl;
	}
    }
    return p;
}
