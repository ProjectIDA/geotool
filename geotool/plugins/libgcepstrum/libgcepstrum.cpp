/** \file libgcepstrum.cpp
 *  \brief Defines libgcepstrum plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "GCepstrum.h"

using namespace libgcepstrum;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Cepstrum", "Cepstrum Analysis",
		createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Cepstrum PlugIn subclass.
 *  @ingroup libgcepstrum
 */
class gcepstrumPlugin : public PlugIn
{
    public :
	gcepstrumPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    cep = NULL;
	}
	~gcepstrumPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!cep) cep = new GCepstrum("Cepstrum", tw_parent, ds);
	    cep->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!cep) cep = new GCepstrum("Cepstrum", tw_parent, ds);
	    cep->setVisible(true);
	}

	Frame *getFrame(void) { return cep; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!cep) cep = new GCepstrum("Cepstrum", tw_parent, ds);
	    return cep->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!cep) cep = new GCepstrum("Cepstrum", tw_parent, ds);
	    return cep->parseVar(name, value);
	}
	Button *button;

    protected :
	GCepstrum *cep;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gcepstrumPlugin *p = new gcepstrumPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *sp_menu = option_menu->getMenu("Spectral Analysis");
	    if(!sp_menu) {
		sp_menu = new Menu("Spectral Analysis", option_menu, -1);
	    }
	    p->button = new Button("Cepstrum...", sp_menu, -1, p);
	}
	else {
	    cerr << "libgcepstrum: cannot create Cepstrum Button. " << endl;
	}
    }
    return p;
}
