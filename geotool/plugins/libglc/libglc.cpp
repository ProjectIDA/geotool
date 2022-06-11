/** \file libglc.cpp
 *  \brief Defines libglc plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Locate.h"

using namespace libglc;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Locate Event", "Event Location",
		createPlugin},
    {NULL, "TableQuery", NULL, "Locate Event", "Event Location", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

class glcPlugin : public PlugIn
{
    public :
	glcPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    button = NULL;
	    glc = NULL;
	}
	~glcPlugin(void) {
	}
	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!glc) glc = new Locate("Locate Event", tw_parent, ds);
	    glc->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!glc) glc = new Locate("Locate Event", tw_parent, ds);
	    glc->setVisible(true);
	}

	Frame *getFrame(void) { return glc; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!glc) glc = new Locate("Locate Event", tw_parent, ds);
	    return glc->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!glc) glc = new Locate("Locate Event", tw_parent, ds);
	    return glc->parseVar(name, value);
	}
	Button *button;

    protected :
	Locate *glc;
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    glcPlugin *p = new glcPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("Locate Event...", option_menu, -1, p);
	}
	else {
	    cerr << "libglc: cannot create Locate Event Button. " << endl;
	}
    }
    return p;
}
