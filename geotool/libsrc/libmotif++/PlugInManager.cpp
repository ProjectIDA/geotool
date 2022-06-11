/** \file PlugInManager.cpp
 *  \brief Defines class PlugInManager.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <algorithm>

extern "C" {
#include "libstring.h"
}

using namespace std;

#include "motif++/PlugInManager.h"
#include "motif++/Component.h"
#include "motif++/Application.h"

static bool sort_plugins(PlugInList a, PlugInList b);

/** Constructor with plugin restriction. When the PlugInManager is created, it
 *  searches preset directories for shared libraries that contain plugins and
 *  makes a list of available PlugIn classes. Only the plugins that are in
 *  the only_plugin list will be allowed. TopWindow instances call the
 *  getPlugins function to get a list of plugins that are intended for them.
 *  The TopWindow calls the plugin's createPlugin function, if it decides to
 *  accept the PlugIn.
 *  @param[in] plugin_list a comma delimited list of the plugins that will
 *		be allowed. If it is empty or NULL, no plugins will be allowed.
 * 		Example: "libgtq,libgfk,libgrt"
 */
PlugInManager::PlugInManager(const string &plugin_list, bool set_verbose)
{
    num_only_plugins = -1;
    if(!plugin_list.empty()) {
	char *s = strdup(plugin_list.c_str());
	char *tok, *c, *last;
	num_only_plugins = 0;
	tok = s;
	while( num_only_plugins < 100 && (c = strtok_r(tok, ", ", &last)) ) {
	    tok = NULL;
	    this->only_plugins[num_only_plugins++] = c;
	}
	Free(s);
    }
    verbose = set_verbose;

    readPlugIns();
}

/** Destructor */
PlugInManager::~PlugInManager(void)
{
    for(int i = 0; i < (int)plugins.size(); i++) {
	free(plugins[i].library_path);
	free(plugins[i].library_name);
    }
}

/** Search for shared libraries that contain plugins. The following three
 *  directory locations are searched:
 *	- $HOME/.geotool++/plugins
 *	- $GEOTOOL_PLUGINS
 *	- the installation-directory/lib/plugins
 *
 *  All subdirectories of these directories are also recursively searched.
 *  GEOTOOL_PLUGINS is an optional environment variable that can be used to
 *  point to directory containing plug-in libraries. HOME is an environment
 *  variable set to the user's home directory.
 */
void PlugInManager::readPlugIns(void)
{
    char *c, path[MAXPATHLEN+1];
    const char *install_dir;

    for(int i = 0; i < (int)plugins.size(); i++) {
	free(plugins[i].library_path);
	free(plugins[i].library_name);
    }
    plugins.clear();

    if(num_only_plugins == 0) return;	// don't allow any plugins

    // Search ~/.geotool++/plugins for shared libraries

    if((c = (char *)getenv("HOME"))) {
	snprintf(path, sizeof(path), "%s/.geotool++/plugins", c);
	searchDirectory(path, true);
    }

    // Search installation_dir/lib/plugins for shared libraries

    if ((c = getenv("GEOTOOL_PLUGINS"))) {
      searchDirectory(c, true);
    }

    install_dir = Application::getApplication()->installationDir();
    snprintf(path, sizeof(path), "%s/lib/plugins", install_dir);
    if( !searchDirectory(path, true) ) {
	snprintf(path, sizeof(path), "%s/lib64/plugins", install_dir);
	searchDirectory(path, true);
    }

    sort(plugins.begin(), plugins.end(), sort_plugins);
}

/** Search a directory for shared libraries that contain plugins. Also search
 *  all subdirectories, if search_subdirs is true. Ignores all files and
 *  directories that begin with '.'. Looks for files that end with '.so' only.
 *  A pointer to the PlugInIndex function named "plugInIndex" is retrieved for
 *  each library. Duplicate library links are avoided by comparing the library
 *  names up to the first period ".". For example, the following names would be
 *  considered the same: libgft libgft.so libgft.0 libgft.0.0.0, etc.
 *  @param[in] dir the directory to search.
 *  @param[in] search_subdirs if true, recursively search the subdirectories.
 */
bool PlugInManager::searchDirectory(char *dir, bool search_subdirs)
{
    DIR *dirp, *sub_dirp;
    struct dirent *dp;
    struct stat buf;
    char path[MAXPATHLEN+1];
    bool found_one = false;

    if((dirp = opendir(dir)) == NULL) return false;

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	if(dp->d_name[0] != '.')
    {
	snprintf(path, sizeof(path), "%s/%s", dir, dp->d_name);

	if(search_subdirs && (sub_dirp = opendir(path))) {
	    closedir(sub_dirp);
	    searchDirectory(path, true);
	}
	else if(stringEndsWith(dp->d_name, ".so") ||
		stringEndsWith(dp->d_name, ".dylib"))
	{
	    char *name = strdup(dp->d_name);
	    int i;
	    for(i = 0; name[i] != '\0' && name[i] != '.'; i++);
	    name[i] = '\0'; // the filename up to the first '.'

	    if(num_only_plugins >= 0) { // restrict plugins
		for(i = 0; i < num_only_plugins &&
			strcmp(only_plugins[i], name); i++);
		if(i == num_only_plugins) {
		    Free(name);
		    continue;
		}
	    }

	    // Prevent duplicates. Look for a match up to the first '.'.
	    // libgtq.so matches libgtq.0.0.0, etc. Accept only one of them.
	    for(i = 0; i < (int)plugins.size() &&
			strcmp(name, plugins[i].library_name); i++);
	    if(i < (int)plugins.size()) {
		Free(name);
		continue;
	    }

	    if(!stat(path, &buf) && !S_ISDIR(buf.st_mode))
	    {
		openSharedLib(name, path);
		found_one = true;
	    }
	    Free(name);
	}
    }
    closedir(dirp);
    return found_one;
}

/** Open a shared library and get the pointer to the "plugInIndex" symbol.
 *  @param[in] name the library name
 *  @param[in] path the directory that contains the library.
 *  @throw GERROR_MALLOC_ERROR
 */
void PlugInManager::openSharedLib(char *name, char *path)
{
    struct stat buf;
    void *handle;
    PlugInIndex index_method;
    PlugInStruct *p;

    if(stat(path, &buf) != 0) return;

    dlerror(); // clear error messages
    if( !(handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL)) ) {
	fprintf(stderr, "dlopen failed for: %s\n%s\n", path, dlerror());
	return;
    }
    if( !(index_method = (PlugInIndex)dlsym(handle, "plugInIndex")) ) {
	fprintf(stderr, "dlsym failed for library: %s\n", path);
	fprintf(stderr, " and symbol \"plugInIndex\"\n%s\n", dlerror());
	dlclose(handle);
	return;
    }
    if(verbose) {
	printf("opening: %s\n", path);
    }
    int num = (*index_method)(&p);

    if(num > 0)
    {
	for(int i = 0; i < num; i++) {
	    PlugInList pl;
	    pl.library_name = strdup(name);
	    pl.library_path = strdup(path);
	    pl.p = p[i];
	    plugins.push_back(pl);
	}
    }
}

/** Get the PlugIn classes for a specific TopWindow. A TopWindow instance calls
 *  this function to get all plugins that are intended for it. If the TopWindow
 *  decides to accept a PlugIn, it calls the plugin's createPlugin function.
 *  @param[in] receiver_name the intended TopWindow parent name.
 *  @param[in] parent the intended TopWindow parent of the plugin.
 *  @param[out] p an array of plugin structures for the input TopWindow
 *  @returns the number of plugin structures.
 *  @throw GERROR_MALLOC_ERROR
 */
int PlugInManager::getPlugins(const string &receiver_name, Component *parent,
			PlugInStruct **p) throw(int)
{
    PlugInStruct *ps = NULL;
    int n = 0;
    Application *app = Application::getApplication();

    for(int i = 0; i < (int)plugins.size(); i++)
    {
	PlugInStruct *plugin = &plugins[i].p;

	if( (!plugin->application_name || plugin->application_name[0] == '\0' ||
		!strcmp(plugin->application_name, app->getName())) &&
	    (!plugin->parent_class || plugin->parent_class[0] == '\0' ||
		!receiver_name.compare(plugin->parent_class)) &&
	    (!plugin->parent_name || plugin->parent_name[0] == '\0' ||
		!strcmp(plugin->parent_name, parent->getName()) ) )
	{
	    if(!ps) {
		ps = (PlugInStruct *)malloc(sizeof(PlugInStruct));
	    }
	    else {
		ps = (PlugInStruct *)realloc(ps, (n+1)*sizeof(PlugInStruct));
	    }
	    if( !ps ) {
		GError::setMessage("PlugInManager.getPlugins: malloc failed.");
		throw GERROR_MALLOC_ERROR;
	    }
	    ps[n++] = plugins[i].p;
	}
    }
    *p = ps;
    return n;
}

/** Get the PlugIn classes for a plugin name.
 *  @param[in] plugin_name the name of the plugin.
 *  @param[out] p an array of plugin structures for with the input plugin name.
 *  @returns the number of plugin structures.
 *  @throw GERROR_MALLOC_ERROR
 */
int PlugInManager::getPlugins(const string &plugin_name, PlugInStruct **p)
			throw(int)
{
    PlugInStruct *ps = NULL;
    int n = 0;
    Application *app = Application::getApplication();

    for(int i = 0; i < (int)plugins.size(); i++)
    {
	PlugInStruct *plugin = &plugins[i].p;

	if( (!plugin->application_name || plugin->application_name[0] == '\0' ||
		!strcmp(plugin->application_name, app->getName())) &&
	    (plugin->name && !plugin_name.compare(plugin->name)) )
	{
	    if(!ps) {
		ps = (PlugInStruct *)malloc(sizeof(PlugInStruct));
	    }
	    else {
		ps = (PlugInStruct *)realloc(ps, (n+1)*sizeof(PlugInStruct));
	    }
	    if( !ps ) {
		GError::setMessage("PlugInManager.getPlugins: malloc failed.");
		throw GERROR_MALLOC_ERROR;
	    }
	    ps[n++] = plugins[i].p;
	}
    }
    *p = ps;
    return n;
}

/** Get all PlugIn classes.
 *  @param[out] p an array of plugin structures.
 *  @returns the number of plugin structures.
 *  @throw GERROR_MALLOC_ERROR
 */
int PlugInManager::getPlugins(PlugInStruct **p) throw(int)
{
    PlugInStruct *ps = NULL;
    int n = (int)plugins.size();

    ps = (PlugInStruct *)malloc(n*sizeof(PlugInStruct));
    if( !ps ) {
	GError::setMessage("PlugInManager.getPlugins: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
    for(int i = 0; i < n; i++) ps[n++] = plugins[i].p;
    *p = ps;
    return n;
}

static bool sort_plugins(PlugInList a, PlugInList b)
{
    return (strcmp(a.p.name, b.p.name) < 0) ? true : false;
}

PlugIn * PlugInManager::createPlugin(const string &name, TopWindow *tw_parent)
{
    Application *app = Application::getApplication();
    PlugInStruct *p = NULL;
    PlugIn *plugin = NULL;
    int n = app->getPlugInManager()->getPlugins(name, &p);
    if(n > 0) {
	if(app->getPlugInManager()->verbose) {
	    printf("creating plugin: %s for %s\n",p->name,tw_parent->getName());
	}
	plugin = (*p[0].create_plugin)(app, tw_parent, false, NULL, NULL);
    }
    else {
	cerr << "PlugInManager.createPlugin: cannot find " << name
		<< " plug-in" << endl;
    }
    Free(p);
    return plugin;
}
