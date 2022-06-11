/** \file libgfk3d.cpp
 *  \brief Defines libfk3d plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#ifdef HAVE_OPENGL

#include "motif++/PlugIn.h"
#include "Fk3D.h"
extern "C" {
#include "libstring.h"
}

using namespace libgfk;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "FK", NULL, "FK3D", "3-D FK Display", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return 1;
}

/** The FK3D PlugIn subclass.
 *  @ingroup libgfk3d
 */
class fk3dPlugin : public PlugIn
{
    public :
	fk3dPlugin(TopWindow *tw, DataSource *s, DataReceiver *r) :
		PlugIn(tw, s, r)
	{
	    fk = (FK *)tw;
	    fk3d = NULL;
	}
	~fk3dPlugin(void) { }

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!strcmp(action_event->getActionCommand(), "3DView...") ||
		action_event->getSource() == button)
	    {
		show3D();
		fk3d->setVisible(true);
	    }
	    else if(action_event->getSource() == fk) {
		if(fk3d && fk3d->isVisible())
		{
		    fk3d->drawFK(1., (long)action_event->getCalldata());
		}
	    }
	}

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!fk3d) {
		show3D();
		fk3d->setVisible(true);
	    }
	    return fk3d->parseCmd(cmd, msg);
	}
	Button *button;

    protected :
	FK *fk;
	Fk3D *fk3d;

	void show3D(void)
	{
	    if(!fk3d) {
		fk3d = new Fk3D("3-D FK", tw_parent, fk, fk->type());
	    }
	    fk3d->draw();
	}
};

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    fk3dPlugin *p = new fk3dPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) ) {
	    Menu *option_menu = frame->optionMenu();
	    p->button = new Button("3DView...", option_menu, p);
	    tw_parent->addActionListener(p);
	}
	else {
	    cerr << "libgfk3d: cannot create 3DView Button. " << endl;
	}
    }
    return p;
}

#endif /* HAVE_OPENGL */
