/** \file libgspectro.cpp
 *  \brief Defines libgspectro plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Spectro.h"

using namespace libgspectro;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Spectrogram", "Spectrogram", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class gspectroPlugin : public PlugIn
{
    public :
	gspectroPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    spectro = NULL;
	}
	~gspectroPlugin() {
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!spectro) spectro = new Spectro("Spectrogram", tw_parent, ds);
	    spectro->setVisible(true);
	}
	void setDataSource(DataSource *data_source) {
	    ds = data_source;
	    if(!spectro) spectro = new Spectro("Spectrogram", tw_parent, ds);
	    spectro->setDataSource(ds);
	}

	Frame *getFrame(void) { return spectro; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!spectro) spectro = new Spectro("Spectrogram", tw_parent, ds);
	    return spectro->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!spectro) spectro = new Spectro("Spectrogram", tw_parent, ds);
	    return spectro->parseVar(name, value);
	}
	Button *button;

    protected :
	Spectro *spectro;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gspectroPlugin *p = new gspectroPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    Menu *sp_menu = option_menu->getMenu("Spectral Analysis");
	    if(!sp_menu) {
		sp_menu = new Menu("Spectral Analysis", option_menu, -1);
	    }
	    p->button = new Button("Spectrogram...", sp_menu, -1, p);
	}
	else {
	    cerr << "libgspectro: cannot create Spectrogram Button. " << endl;
	}
    }
    return p;
}
