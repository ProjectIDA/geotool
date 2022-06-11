/** \file libgrot.cpp
 *  \brief Defines libgrot plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/MotifClasses.h"
#include "Rotate.h"

using namespace libgrot;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Rotation", "Component Rotation Analysis",
		createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class grotPlugin : public PlugIn
{
    public :
	grotPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    grot = NULL;
	}
	~grotPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!grot) grot = new Rotate("Rotation", tw_parent, ds);
	    grot->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!grot) grot = new Rotate("Rotation", tw_parent, ds);
	    grot->setVisible(true);
	}

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!grot) grot = new Rotate("Rotation", tw_parent, ds);
	    return grot->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!grot) grot = new Rotate("Rotation", tw_parent, ds);
	    return grot->parseVar(name, value);
	}
	Button *button;

    protected :
	Rotate *grot;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    grotPlugin *p = new grotPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *edit_menu = frame->editMenu();
	    p->button = new Button("Rotation...", edit_menu, -1, p);
	    Menu *option_menu = frame->optionMenu();
	    Menu *cp_menu = option_menu->getMenu("Three-Component Analysis");
	    if(!cp_menu) {
		cp_menu = new Menu("Three-Component Analysis", option_menu, -1);
	    }
	    p->button = new Button("Rotation...", cp_menu, -1, p);
	}
	else {
	    cerr << "libgrot: cannot create Rotation Button. " << endl;
	}
    }
    return p;
}
