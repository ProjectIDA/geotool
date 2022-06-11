/** \file libgtt.cpp
 *  \brief Defines libgtt plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "motif++/Frame.h"
#include "motif++/Toggle.h"
#include "TravelTimes.h"
#include "WaveformWindow.h"

using namespace libgtt;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Travel Times", "Travel Times",createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gttPlugin : public PlugIn
{
    public :
	gttPlugin(TopWindow *tw, DataSource *s, DataReceiver *r,
		WaveformWindow *w) : PlugIn(tw, s, r)
	{
	    ww = w;
	    button = NULL;
	    tt = NULL;
	}
	~gttPlugin(void) {
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!tt) tt = new TravelTimes("Travel Times", tw_parent, ww);
	    if(action_event->getSource() == button) {
		tt->setVisible(true);
	    }
	    else if(action_event->getSource() == tool_bar_toggle) {
		tt->curves(tool_bar_toggle->state());
	    }
	}

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!tt) tt = new TravelTimes("Travel Times", tw_parent, ww);
	    return tt->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!tt) tt = new TravelTimes("Travel Times", tw_parent, ww);
	    return tt->parseVar(name, value);
	}
	Toggle *tool_bar_toggle;
	Button *button;

    protected :
	TravelTimes *tt;
	WaveformWindow *ww;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    WaveformWindow *ww = tw_parent->getWaveformWindowInstance();
    gttPlugin *p = new gttPlugin(tw_parent, ds, dr, ww);

    if( create_button ) {
	if( ww) {
	    Menu *option_menu = ww->optionMenu();
	    p->button = new Button("Travel Times...", option_menu, -1, p);
	    // add a toggle that can be used in the frame.toolbar
	    p->tool_bar_toggle = new Toggle("TT", option_menu, p);
	    p->tool_bar_toggle->setVisible(false);
	}
	else {
	    cerr << "libgtt: cannot create Travel Times Button. " << endl;
	}
    }
    return p;
}
