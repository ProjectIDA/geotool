/** \file libgdataqc.cpp
 *  \brief Defines libgdataqc plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/PlugIn.h"
#include "QCParam.h"
#include "RmsWindow.h"

using namespace libgdataqc;

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);
static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr);

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL, "WaveformWindow", NULL, "Data QC", "Data QC", createPlugin1},
    {NULL, "WaveformWindow", NULL, "RMS", "RMS Averaging", createPlugin2},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}

/** The Data QC PlugIn subclass.
 *  @ingroup libgdataqc
 */
class dataqcPlugin : public PlugIn
{
    public :
	dataqcPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    qcparam = NULL;
	}
	~dataqcPlugin(void) {
	}

	void actionPerformed(ActionEvent *action_event)
	{
	    if(!qcparam) qcparam = new QCParam("Data QC", tw_parent, ds);
	    qcparam->setVisible(true);
	}

	Frame *getFrame(void) { return qcparam; }

	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!qcparam) {
		qcparam = new QCParam("Data QC", tw_parent, ds);
	    }
	    return qcparam->parseCmd(cmd, msg);
	}
	ParseVar parseVar(const string &name, string &value) {
	    if(!qcparam) {
		qcparam = new QCParam("Data QC", tw_parent, ds);
	    }
	    return qcparam->parseVar(name, value);
	}
	Button *button;

    protected :
	QCParam *qcparam;
};

static PlugIn *createPlugin1(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    dataqcPlugin *p = new dataqcPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *edit_menu = frame->editMenu();
	    p->button = new Button("QC...", edit_menu, -1, p);
	}
	else {
	    cerr << "libgdataqc: cannot create QC Button. " << endl;
	}
    }
    return p;
}

/** The RMS PlugIn subclass.
 *  @ingroup libgdataqc
 */
class rmsPlugin : public PlugIn
{
    public :
	rmsPlugin(TopWindow *tw, DataSource *s, DataReceiver *r)
		: PlugIn(tw, s, r)
	{
	    button = NULL;
	    rms_window = NULL;
	}
	~rmsPlugin(void) {
	}
	void actionPerformed(ActionEvent *action_event)
	{
	    if(!rms_window) rms_window = new RmsWindow("Root Mean Square",
						tw_parent, ds);
	    rms_window->setVisible(true);
	}
	ParseCmd parseCmd(const string &cmd, string &msg) {
	    if(!rms_window) rms_window = new RmsWindow("Root Mean Square",
						tw_parent, ds);
	    return rms_window->parseCmd(cmd, msg);
	}
	Button *button;

    protected :
	RmsWindow *rms_window;

};

static PlugIn *createPlugin2(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    rmsPlugin *p = new rmsPlugin(tw_parent, ds, dr);

    if( create_button ) {
	Frame *frame;
	if( (frame = tw_parent->getFrameInstance()) )
	{
	    Menu *edit_menu = frame->editMenu();
	    p->button = new Button("RMS...", edit_menu, -1, p);
	}
	else {
	    cerr << "libgdataqc: cannot create RMS Button. " << endl;
	}
    }
    return p;
}
