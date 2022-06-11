#ifndef _PLUG_IN_MANAGER_H
#define _PLUG_IN_MANAGER_H

#include <vector>
#include "gobject++/Gobject.h"
#include "motif++/PlugIn.h"

/** PlugInManager information structure. */
typedef struct
{
    char *library_path; //!< The directory that contains the shared library
    char *library_name;	//!< The shared library name.
    PlugInStruct p;   //!< Returned by the shared library plugInIndex function.
} PlugInList;

class Component;

/** Manages shared library PlugIn instances.
 *  @ingroup libmotif
 */
class PlugInManager : public Gobject
{
    public:
	PlugInManager(const string &only_plugins="", bool verbose=false);
	~PlugInManager(void);

	void readPlugIns(void);
	int getPlugins(const string &receiver_name, Component *parent,
			PlugInStruct **p) throw(int);
	int getPlugins(const string &plugin_name, PlugInStruct **p) throw(int);
	int getPlugins(PlugInStruct **p) throw(int);
	int numPlugins() { return (int)plugins.size(); }
	PlugInList pluginList(int i) { return plugins[i]; }

	static PlugIn * createPlugin(const string &name, TopWindow *tw_parent);

	bool verbose;

    protected:
	vector<PlugInList> plugins;
	int num_only_plugins;
	char *only_plugins[100];

	bool searchDirectory(char *dir, bool search_subdirs=false);
	void openSharedLib(char *name, char *path);

    private:
};

#endif
