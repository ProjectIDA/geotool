/** \file TopWindow.cpp
 *  \brief Defines class TopWindow.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/TopWindow.h"
#include "motif++/Application.h"
#include "motif++/PlugIn.h"

using namespace std;

/** Constructor.
 *  @param[in] name the name given to this TopWindow instance.
 *  @param[in] parent the Component parent.
 *  @param[in] title the title displayed on the top window border.
 *  @param[in] pointer_focus If true, set keyboardFocusPolicy to XmPOINTER,
 *		otherwise it is XmEXPLICIT
 *  @param[in] independent if true, the widget parent is the display parent
 * 		instead of the parent base widget.
 */
TopWindow::TopWindow(const string &name, Component *parent, const string &title,
		bool pointer_focus, bool independent) :
		FormDialog(name, parent, title, pointer_focus, independent)
{
    warn = NULL;
}

/** Constructor.
 *  @param[in] name the name given to this TopWindow instance.
 *  @param[in] parent the Component parent.
 *  @param[in] pointer_focus If true, set keyboardFocusPolicy to XmPOINTER,
 *		otherwise it is XmEXPLICIT
 *  @param[in] independent if true, the widget parent is the display parent
 * 		instead of the parent base widget.
 */
TopWindow::TopWindow(const string &name, Component *parent, bool pointer_focus,
		bool independent) : FormDialog(name, parent, pointer_focus,
		independent)
{
    warn = NULL;
}

/** Destructor. */
TopWindow::~TopWindow(void)
{
}

/** Add all PlugIns that are intended for this TopWindow.
 *  @param[in] receiver_name the intended TopWindow parent name.
 *  @param[in] ds the DataSource that will be passed to the PlugIn.
 *  @param[in] dr the DataReceiver that will be passed to the PlugIn.
 */
void TopWindow::addPlugins(const string &receiver_name, DataSource *ds,
			DataReceiver *dr)
{
    Application *application = Application::getApplication();
    PlugInManager *pm = application->plugInManager();
    PlugIn *plugin;

    if(!pm) return;

    PlugInStruct *ps = NULL;
    int num = pm->getPlugins(receiver_name, this, &ps);
    for(int i = 0; i < num; i++)
    {
	// check if we already have it
	int j;
	for(j = 0; j < (int)plugin_structs.size() &&
		plugin_structs[j].create_plugin != ps[i].create_plugin; j++);
	if(j == (int)plugin_structs.size())
	{
	    plugin = (*ps[i].create_plugin)(application, this, true, ds, dr);
	    if(plugin)
	    {
		plugins.push_back(plugin);
		plugin_structs.push_back(ps[i]);
	    }
	}
    }
    Free(ps);
}

ParseCmd TopWindow::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;

    if((ret = FormDialog::parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
        return ret;
    }
    return pluginParse(cmd, msg);
}

/**
 */
ParseCmd TopWindow::pluginParse(const string &cmd, string &msg)
{
    ParseCmd ret;
    size_t n;

    if((n = cmd.find('.')) == string::npos) return COMMAND_NOT_FOUND;

    for(int i = 0; i < (int)plugins.size(); i++)
    {
	if( sameName(cmd, plugin_structs[i].name, (int)n) )
	{
	    msg.clear();
	    ret = plugins[i]->parseCmd(cmd.substr(n+1), msg);
	    if( ret != COMMAND_NOT_FOUND ) {
		return ret;
	    }
	}
    }
    return COMMAND_NOT_FOUND;
}

ParseVar TopWindow::parseVar(const string &name, string &value)
{
    ParseVar ret;

    if((ret = FormDialog::parseVar(name, value))
		!= VARIABLE_NOT_FOUND)
    {
        return ret;
    }
    return pluginParseVar(name, value);
}

/**
 */
ParseVar TopWindow::pluginParseVar(const string &name, string &value)
{
    ParseVar ret;
    size_t n;

    if((n = name.find('.')) == string::npos) return VARIABLE_NOT_FOUND;

    for(int i = 0; i < (int)plugins.size(); i++)
    {
	if( sameName(name, plugin_structs[i].name, (int)n) )
	{
	    value.clear();
	    ret = plugins[i]->parseVar(name.substr(n+1), value);
	    if( ret != VARIABLE_NOT_FOUND ) {
		return ret;
	    }
	}
    }
    return VARIABLE_NOT_FOUND;
}

Frame *TopWindow::getPluginFrame(const string &plugin_name)
{
    int n = (int)plugin_name.length();
    const char *c = plugin_name.c_str();

    for(int i = n-1; i >= 0; i--) {
	if(plugin_name[i] == '.') {
	    c = plugin_name.c_str()+i+1;
	    break;
	}
    }

    for(int i = 0; i < (int)plugin_structs.size(); i++) {
	if(sameName(plugin_structs[i].name, c)) {
	    return plugins[i]->getFrame();
	}
    }
    vector<Component *> *windows = Application::getApplication()->getWindows();

    for(int i = 0; i < (int)windows->size(); i++) {
	if(sameName(windows->at(i)->getName(), c)) {
	    return windows->at(i)->getFrameInstance();
	}
    }
    return NULL;
}

FormDialog *TopWindow::getFormDialog(const string &window_name)
{
    int n = (int)window_name.length();
    const char *c = window_name.c_str();

    for(int i = n-1; i >= 0; i--) {
	if(window_name[i] == '.') {
	    c = window_name.c_str()+i+1;
	    break;
	}
    }
    vector<Component *> *windows = Application::getApplication()->getWindows();

    for(int i = 0; i < (int)windows->size(); i++) {
	if(sameName(windows->at(i)->getName(), c)) {
	    return windows->at(i)->getFormDialogInstance();
	}
    }
    return NULL;
}
