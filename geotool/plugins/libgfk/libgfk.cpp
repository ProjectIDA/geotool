/** \file libgfk.cpp
 *  \brief Defines libgfk plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "FK.h"

using namespace libgfk;

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);
static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "FK", "Single Frequency Band FK",
		createPlugin1},
    {NULL, "WaveformWindow", NULL, "FK Multi-Band",
		"Multiple Frequency Band FK", createPlugin2},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The FK PlugIn subclass.
 *  @ingroup libgfk
 */
class gfkPlugin : public PlugIn
{
    public :
	gfkPlugin(TopWindow *tw, DataSource *s, DataReceiver *r,
		const char *title, FKType type) : PlugIn(tw, s, r)
	{
	    button = NULL;
	    fk = NULL;
	    fk_type = type;
	    stringcpy(frame_title, title, sizeof(frame_title));

	}
	~gfkPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!fk) fk = new FK(frame_title, tw_parent, fk_type, ds);
	    fk->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!fk) fk = new FK(frame_title, tw_parent, fk_type, ds);
	    fk->setVisible(true);
	}

	Frame *getFrame(void) { return fk; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!fk) fk = new FK(frame_title, tw_parent, fk_type, ds);
	    return fk->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!fk) fk = new FK(frame_title, tw_parent, fk_type, ds);
	    return fk->parseVar(name, value);
	}
	Button *button;

    protected :
	FK *fk;
	FKType fk_type;
	char frame_title[50];
};

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gfkPlugin *p = new gfkPlugin(tw_parent, ds, dr, "FK", FK_SINGLE_BAND);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *array_menu;
	    if( !(array_menu = option_menu->findMenu("Array Analysis")) ) {
		array_menu = new Menu("Array Analysis", option_menu, -1);
	    }
	    p->button = new Button("FK...", array_menu, -1, p);
	}
	else {
	    cerr << "libgfk: cannot create FK Button. " << endl;
	}
    }
    return p;
}

static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gfkPlugin *p = new gfkPlugin(tw_parent, ds, dr, "FK Multi-Band",
				FK_MULTI_BAND);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *array_menu;
	    if( !(array_menu = option_menu->findMenu("Array Analysis")) ) {
		array_menu = new Menu("Array Analysis", option_menu, -1);
	    }
	    p->button = new Button("FK Multi-Band...", array_menu, -1, p);
	}
	else {
	    cerr << "libgfk: cannot create FT Multi-Band Button. " << endl;
	}
    }
    return p;
}
