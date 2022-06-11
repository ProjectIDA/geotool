/** \file libgbm.cpp
 *  \brief Defines libgbm plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "Recipes.h"

using namespace libgbm;

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);
static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Origin Beams", "Origin Beam Recipes",
		createPlugin1},
    {NULL, "WaveformWindow", NULL, "Detection Beams", "Detection Beam Recipes",
		createPlugin2},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Origin and Detection Beams PlugIn subclass.
 *  @ingroup libgbm
 */
class gbmPlugin : public PlugIn
{
    public :
	gbmPlugin(TopWindow *tw, DataSource *s, DataReceiver *r,
		const char *title, RecipeType type) : PlugIn(tw, s, r)
	{
	    button = NULL;
	    gbm = NULL;
	    recipe_type = type;
	    stringcpy(frame_title, title, sizeof(frame_title));
	}
	~gbmPlugin(void) {
	}

	void setDataSource(DataSource *dsrc) {
	    ds = dsrc;
	    if(!gbm) gbm = new Recipes(frame_title, tw_parent, recipe_type, ds);
	    gbm->setDataSource(ds);
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!gbm) gbm = new Recipes(frame_title, tw_parent, recipe_type, ds);
	    gbm->setVisible(true);
	}

	Frame *getFrame(void) { return gbm; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!gbm) gbm = new Recipes(frame_title, tw_parent, recipe_type, ds);
	    return gbm->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!gbm) gbm = new Recipes(frame_title, tw_parent, recipe_type, ds);
	    return gbm->parseVar(name, value);
	}
	Button *button;

    protected :
	Recipes *gbm;
	RecipeType recipe_type;
	char frame_title[50];
};

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gbmPlugin *p = new gbmPlugin(tw_parent, ds, dr, "Origin Beams",
			ORIGIN_RECIPES);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *array_menu;
	    if( !(array_menu = option_menu->findMenu("Array Analysis")) ) {
		array_menu = new Menu("Array Analysis", option_menu, -1);
	    }
	    p->button = new Button("Origin Beams...", array_menu, -1, p);
	}
	else {
	    cerr << "libggbm: cannot create Origin Beams Button. " << endl;
	}
    }
    return p;
}

static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    gbmPlugin *p = new gbmPlugin(tw_parent, ds, dr, "Detection Beams",
			DETECTION_RECIPES);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *option_menu = frame->optionMenu();
	    Menu *array_menu;
	    if( !(array_menu = option_menu->findMenu("Array Analysis")) ) {
		array_menu = new Menu("Array Analysis", option_menu, -1);
	    }
	    p->button = new Button("Detection Beams...", array_menu, -1, p);
	}
	else {
	    cerr << "libggbm: cannot create Detection Beams Button. " << endl;
	}
    }
    return p;
}
