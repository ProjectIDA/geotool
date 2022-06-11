#ifndef _TOP_WINDOW_H
#define _TOP_WINDOW_H

#include "motif++/FormDialog.h"

class PlugIn;
class Warn;
class DataSource;
class DataReceiver;
class Frame;
class WaveformWindow;
class WaveformPlot;
class TableViewer;
class CssFileDialog;
struct plugInStruct;

/** A FormDialog subclass that supports PlugIns.
 *  Plugins can only bee added to windows that are subclasses of TopWindow.
 *  The Frame class is a subclass of TopWindow. There is only one Warn window
 *  for each TopWindow, so two FormDialog windows might share the same Warn
 *  window that belongs to their TopWindow parent.
 *
 *  The Application instance exits the program when there are no visible
 *  (managed) TopWindow instances.
 *  @ingroup libmotif
 */
class TopWindow : public FormDialog
{
    friend class Component;
    public:
	TopWindow(const string &, Component *parent, const string &title,
			bool pointer_focus=false, bool independent=true);
	TopWindow(const string &, Component *parent, bool pointer_focus=false,
			bool independent=true);
	~TopWindow(void);

	virtual TopWindow *getTopWindowInstance(void) { return this; }
	Frame *getPluginFrame(const string &plugin_name);
	FormDialog *getFormDialog(const string &window_name);
        ParseCmd parseCmd(const string &cmd, string &msg);
        ParseVar parseVar(const string &name, string &value);

    protected:
	Warn *warn; //!< the Warn window for this TopWindow.
	vector<PlugIn *> plugins; //!< the plugins.
	vector<struct plugInStruct> plugin_structs; //!< plugin information.

	void addPlugins(const string &receiver_name, DataSource *ds,
			DataReceiver *dr);
	ParseCmd pluginParse(const string &cmd, string &msg);
	ParseVar pluginParseVar(const string &name, string &value);
};

#endif
