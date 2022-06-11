/** \file libgcal.cpp
 *  \brief Defines libgcal plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Calibration.h"

using namespace libgcal;


static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Calibration", "Instrument Calibration",
		createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Calibration PlugIn subclass.
 *  @ingroup libgcal
 */
class gcalPlugin : public PlugIn
{
    public :
	gcalPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    gcal = NULL;
	}
	~gcalPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gcal) gcal = new Calibration("Calibration", tw_parent);
	    gcal->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gcal) gcal = new Calibration("Calibration", tw_parent);
	    gcal->setVisible(true);
	}

	Frame *getFrame(void) { return gcal; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gcal) gcal = new Calibration("Calibration", tw_parent);
	    return gcal->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gcal) gcal = new Calibration("Calibration", tw_parent);
	    return gcal->parseVar(name, value);
	}
	Button *button;

    protected :
	Calibration *gcal;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gcalPlugin *p = new gcalPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("Calibration...", option_menu, -1, p);
	}
	else {
	    cerr << "libgcal: cannot create Calibration Button. " << endl;
	}
    }
    return p;
}
