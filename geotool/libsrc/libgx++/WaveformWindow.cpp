/** \file WaveformWindow.cpp
 *  \brief Defines class WaveformWindow.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
using namespace std;

#include "WaveformWindow.h"
#include "WaveformView.h"
#include "WaveformColor.h"
#include "motif++/MotifClasses.h"
#include "OpenFile.h"
#include "TableSource.h"
#include "SacSource.h"
#include "GseSource.h"
#include "AscSource.h"
#include "SeedSource.h"
#include "TableFiles.h"
#include "Preferences.h"
#include "Fonts.h"
#include "TableViewer.h"
#include "TableQuery.h"
#include "PluginTable.h"
#include "motif++/Warnings.h"

extern "C" {
#include "libstring.h"
}

static TableFiles *table_files = NULL;

class MouseHelp : public FormDialog
{
    public :
	MouseHelp(const char *name, Component *parent);
    protected :
	void actionPerformed(ActionEvent *action_event);
};

WaveformWindow::WaveformWindow(const string &name, Component *parent,
		const string &title, bool independent)
		: Frame(name, parent, title, true, independent),
		DataReceiver(), DataSource()
{
    createInterface();
}

void WaveformWindow::createInterface(void)
{
    setSize(600, 680);

    open_file = NULL;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // Create File Menu
    file_menu = new Menu("File", menu_bar);
    open_db_button = new Button("Open Database...", file_menu, this);
    open_file_button = new Button("Open File...", file_menu, this);
    recent_files_menu = new Menu("Recent Files", file_menu);

    getRecentInput();
    clear_recent_button = new Button("Clear Recent Files", recent_files_menu,
					this);
    file_menu->addSeparator("separator");

//    messages_button = new Button("Log...", file_menu, this);
//    output_button = new Button("Output...", file_menu, this);
    output_button = new Button("Save as...", file_menu, this);
    preferences_button = new Button("Preferences...", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    plugins_button = new Button("Plugins...", file_menu, this);
    tables_menu = new Menu("Tables", file_menu);
    waveform_tables_button = new Button("Loaded Tables...", tables_menu, this);
    table_files_button = new Button("Table Files...", tables_menu, this);
    tv_button = new Button("TableViewer...", tables_menu, this);
    waveform_setup_button = new Button("Waveform Setup...", file_menu, this);
    warnings_button = new Button("Warnings...", file_menu, this);
    new_window_button = new Button("New Window...", file_menu, this);

    close_button = new Button("Close", file_menu, this);
    file_menu->addSeparator("separator", XmDOUBLE_LINE);
    quit_button = new Button("Quit", file_menu, this);

    // Create Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    edit_menu->addActionListener(this, XmNcascadingCallback);
    clear_button = new Button("Clear", edit_menu, this);
    copy_button = new Button("Copy", edit_menu, this);
    cut_button = new Button("Cut", edit_menu, this);
    paste_button = new Button("Paste", edit_menu, this);
    delete_data_button = new Button("Delete Data", edit_menu, this);
    wt.partial_select_toggle = new Toggle("Partial Select", edit_menu, this);
    calib_menu = new Menu("Calib", edit_menu);
    display_calib_button = new Button("Display counts*calib", calib_menu, this);
    display_counts_button = new Button("Display counts", calib_menu, this);
    demean_menu = new Menu("Demean", edit_menu);
    remove_mean_button = new Button("Remove Mean", demean_menu, this);
    restore_mean_button = new Button("Restore Mean", demean_menu, this);
    polarity_menu = new Menu("Polarity", edit_menu);
    normal_polarity_button = new Button("Normal", polarity_menu, this);
    reverse_polarity_button = new Button("Reverse", polarity_menu, this);

    filter_menu = new Menu("Filter", edit_menu);
    bfilter_button = new Button("Butterworth Filter...", filter_menu, this);
    unfilter_button = new Button("Unfilter All", filter_menu, this);
    undo_button = new UndoButton("Undo", edit_menu);

    // Create View Menu
    view_menu = new Menu("View", menu_bar);
    view_menu->addActionListener(this, XmNcascadingCallback);

    align_menu = new Menu("Align", view_menu, true);
    wt.align_first_point = new Toggle("On First Point", align_menu, this, true);
    wt.align_first_point->set(false, false);
    wt.align_true_time = new Toggle("On True Time", align_menu, this, true);
    wt.align_true_time->set(true, false);
    wt.align_origin = new Toggle("Time Minus Origin", align_menu, this, true);
    wt.align_predicted = new Toggle("Time Minus Predicted FirstP", align_menu,
				this, true);
    wt.align_observed = new Toggle("Time Minus Observed FirstP", align_menu,
				this, true);

    components_menu = new Menu("Components", view_menu, true);
    wt.all_components = new Toggle("All Components", components_menu,this,true);
    wt.all_components->set(true);
    wt.z_only = new Toggle("Z Only", components_menu, this, true);
    wt.n_and_e_only = new Toggle("N and E Only", components_menu, this, true);
    wt.n_only = new Toggle("N Only", components_menu, this, true);
    wt.e_only = new Toggle("E Only", components_menu, this, true);

    cursors_menu = new Menu("Cursors", view_menu, true);
    add_time_line = new Button("Add Time Line", "L", cursors_menu, this);
    add_time_limits = new Button("Add Time Limits", "l", cursors_menu, this);
    add_crosshair = new Button("Add Crosshair", "c", cursors_menu, this);
    remove_all_cursors = new Button("Remove All Cursors","D",cursors_menu,this);

    data_movement_menu = new Menu("Data Movement", view_menu, true);
    wt.no_movement = new Toggle("no movement", data_movement_menu, this);
    wt.no_movement->set(true);
    wt.xy_movement = new Toggle("xy movement", data_movement_menu, this);
    wt.x_movement = new Toggle("x movement", data_movement_menu, this);
    wt.y_movement = new Toggle("y movement", data_movement_menu, this);
    data_values_button = new Button("Data Values...", view_menu, this);

//    diff_button = new Button("Diff...", view_menu, this);

    wt.display_amp_scale = new Toggle("Display Amplitude Scale",view_menu,this);
    wt.display_amp_scale->set(true);

    display_arrivals_menu = new Menu("Display Arrivals", view_menu, true);
    wt.arrivals_off = new Toggle("Off", display_arrivals_menu, this);
    wt.on_one_channel = new Toggle("On One Channel",display_arrivals_menu,this);
    wt.on_all_channels = new Toggle("On All Channels", display_arrivals_menu,
				this);
    wt.on_all_channels->set(true);

    display_data_menu = new Menu("Display Data", view_menu, true);
    all_data  = new Button("All Data", display_data_menu, this);
    selected_data  = new Button("Selected Only", display_data_menu, this);

    labels_button = new Button("Labels...", view_menu, this);

    magnify_button = new Button("Magnify Window...", view_menu, this);

    promote_selected  = new Button("Promote Selected Waveforms",view_menu,this);

    scale_menu = new Menu("Scale", view_menu, false);
    space_more  = new Button("Space More", scale_menu, this);
    space_less  = new Button("Space Less", scale_menu, this);
    scale_up  = new Button("Scale Up", scale_menu, this);
    scale_down  = new Button("Scale Down", scale_menu, this);
    set_scale  = new Button("Set Scale...", scale_menu, this);
    wt.uniform_scale = new Toggle("Uniform Scale", scale_menu, this);
    wt.auto_scale = new Toggle("Auto Scale", scale_menu, this);

    select_menu = new Menu("Select", view_menu, false);
    select_all = new Button("Select All", select_menu, this);
    deselect_all = new Button("Deselect All", select_menu, this);
    select_visible = new Button("Select Visible", select_menu, this);
    select_visible_z = new Button("Select Visible Z", select_menu, this);
    select_visible_horz = new Button("Select Visible Horz", select_menu, this);

    set_default_order = new Button("Set Default Order", view_menu, this);

    sort_menu = new Menu("Sort", view_menu, true);
    wt.default_order = new Toggle("Default Order", sort_menu, this);
    wt.default_order->set(true);
    wt.file_order = new Toggle("File Order", sort_menu, this);
    wt.distance = new Toggle("Distance", sort_menu, this);
    wt.distance_from = new Toggle("Distance from", sort_menu, this);
    wt.distance_from->setVisible(false);
    wt.time_sta_chan = new Toggle("Time/Sta/Chan", sort_menu, this);
    wt.sta_chan = new Toggle("Sta/Chan", sort_menu, this);
    wt.chan_sta = new Toggle("Chan/Sta", sort_menu, this);
    wt.back_azimuth = new Toggle("Back Azimuth", sort_menu, this);

    scroll_button = new Button("Scroll...", view_menu, this);

    tags_menu = new Menu("Tags", view_menu, false);
    wt.display_tags = new Toggle("Display Tags", tags_menu, this);
    wt.display_tags->set(true, false);
    new Button("Tag Contents...", tags_menu, this);

    time_scale_menu = new Menu("Time Scale", view_menu, true);
    wt.time_scale_seconds = new Toggle("Seconds", time_scale_menu, this, true);
    wt.time_scale_seconds->set(false, false);
    wt.time_scale_variable = new Toggle("Variable", time_scale_menu, this,true);
    wt.time_scale_variable->set(false, false);
    wt.time_scale_hms = new Toggle("HMS", time_scale_menu, this, true);
    wt.time_scale_hms->set(true, false);
    wt.time_scale_epoch_toggle = new Toggle("Epoch", time_scale_menu,this,true);
    wt.time_scale_epoch_toggle->set(false, false);

    unzoom_all = new Button("Unzoom All", view_menu, this);
    waveform_color_button = new Button("Waveform Color...", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    array_menu = new Menu("Array Analysis", option_menu);
    create_beam_button = new Button("Create Beam...", array_menu, this);
    history_button = new Button("History...", option_menu, this);
    waveforms_button = new Button("Waveforms...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Mouse Buttons", help_menu, this);
    doc_button = new Button("Documentation", help_menu, this);
    version_button = new Button("Version", help_menu, this);
    new Separator("sep", help_menu);
    browser_button = new Button("Set Browser...", help_menu, this);

    int n = 0;
    Arg args[10];

    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNdisplayAddArrival, True); n++;

    wplot = new WaveformView("wave_plot", frame_form, info_area, args, n);
    wplot->setSyncMenus(edit_menu, view_menu);
    wplot->addActionListener(this, XtNalignCallback);

    wt.align_predicted_phase = NULL;
    mouse_help_window = NULL;
    plugin_table = NULL;
    tableviewer_window = NULL;
    plugin_table = NULL;
    preferences = NULL;
    vector<Component *> *w =Application::getApplication()->getWindows();
    table_files = new TableFiles("Table Files", w->at(0));

    align_toggle = NULL;
    time_scale_toggle = NULL;
    travel_times_toggle = NULL;

//    addPlugins("WaveformWindow", wplot, this);
    addPlugins("WaveformWindow", this, this);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(open_db_button, "DB", 0);
	tool_bar->add(open_file_button, "Open");
	tool_bar->add(select_all, "Select All");
	tool_bar->add(deselect_all, "Deselect All");
	tool_bar->add(unzoom_all, "Unzoom All");
	tool_bar->add(space_more, ">");
	tool_bar->add(space_less, "<");
	tool_bar->add(scale_up, "+");
	tool_bar->add(scale_down, "-");
	tool_bar->add(wt.auto_scale, "|");
	// it we have the TravelTime plugin, add a button to the toolbar
	travel_times_toggle = findToggle("TT");
	if(travel_times_toggle) {
	    tool_bar->add(travel_times_toggle, "TT");
	}
    }
    else {
	travel_times_toggle = findToggle("TT");
    }

    table_query = new TableQuery("TableQuery", this);
    table_query->setWaveformReceiver(this);

    putProperty("last_data_dir", ".");
}

void WaveformWindow::destroy(void)
{
/*
    if( wplot->removeDataReceiver(wplot, false) ) {
	// only destroy if there are no other DOwners
	Frame::destroy();
    }
*/
	Frame::destroy();
}

WaveformWindow::~WaveformWindow(void)
{
}

void WaveformWindow::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Open Database...")) {
	table_query->setVisible(true);
    }
    else if(!strcmp(cmd, "Open File...")) {
	open();
    }
    else if(!strcmp(reason, XmNcascadingCallback)) {
	if(comp == edit_menu) {
	    setEditMenuSensitive();
	}
	else if(comp == view_menu) {
	    setViewMenuSensitive();
	}
	// The purpose of this is to synchronize the toggle states with
	// the WaveformPlot popup menu, but this will not synchronize toggles
	// in the toolbar.
	wplot->setToggleStates(&wt);
    }
    else if(!strcmp(cmd, "Preferences...")) {
	if(!preferences) {
	    preferences = new Preferences("Preferences", this, wplot);
	}
	preferences->setVisible(true);
    }
    else if(comp == table_files_button) {
	table_files->setVisible(true);
    }
    else if(!strcmp(cmd, "Plugins...")) {
	if(!plugin_table) {
	    plugin_table = new PluginTable("Plugins", this);
	}
	plugin_table->setVisible(true);
    }
    else if(!strcmp(cmd, "Log...")) {
	Application::displayLog();
    }
    else if(!strcmp(cmd, "Warnings...")) {
	Application::displayWarnings();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Quit")) {
	setVisible(false);
	FFDatabase::clearStaticTables();
	Application::getApplication()->stop(0);
    }
    else if(!strcmp(cmd, "New Window...")) {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    WaveformWindow *w = new WaveformWindow(getName(), this);
	    windows.push_back(w);
	    w->setVisible(true);
	    w->addActionListener(this, XtNsetVisibleCallback);
	}
    }
    else if(comp->getWaveformWindowInstance() &&
		!strcmp(reason, XtNsetVisibleCallback))
    {
	comp->getWaveformWindowInstance()->clear();
    }
    else if(comp == open_file) {
	FileDialogStruct *c = (FileDialogStruct *)action_event->getCalldata();
	if(c->state == DIALOG_APPLY && (int)c->files.size() > 0) {
	    setVisible(true);
	    readFiles(c->files);
	}
    }
    else if(!strcmp(cmd, "Recent Input")) {
	for(int i = 0; i < recent_input.size(); i++) {
	    RecentInput *r = recent_input[i];
	    if(comp == r->button) {
		addRecentInput(r); // promotes to the top of the list
		setCursor("hourglass");
		wplot->readRecentInput(r);
		setCursor("default");
		break;
	    }
	}
    }
    else if(!strcmp(cmd, "Clear Recent Files")) {
	clearRecentMenu();
    }
    else if(!strcmp(cmd, "Mouse Buttons")) {
	if(!mouse_help_window) {
	    mouse_help_window = new MouseHelp("Mouse Buttons Help", this);
	}
	mouse_help_window->setVisible(true);
    }
    else if(!strcmp(cmd, "TableViewer...")) {
	if(!tableviewer_window) {
	    tableviewer_window = new TableViewer("TableViewer", this);
	    tableviewer_window->removeAllTabs();
	}
	tableviewer_window->setVisible(true);
    }
    else if(!strcmp(reason, XtNalignCallback)) {
	setAlignToggles(wplot->alignment());
    }
    // these are from the plugin main window (see changeWindowMenu())
    else if(!strcmp(cmd, "Open Database...")) {
	putCmd("tablequery.open");
    }
    else if(!strcmp(cmd, "Waveform Window...")) {
	setVisible(true);
    }
    else if(!strcmp(cmd, "Tutorial")) {
	showHelp("Tutorial");
    }
    else if(!strcmp(cmd, "User's Manual")) {
	showHelp("Users Manual");
    }
    else if(!strcmp(cmd, "Documentation")) {
	showHelp("Documentation");
    }
    else if(!strcmp(cmd, "Programmer's Manual")) {
	if(showHelp("Programmers Manual") == HELP_FILE_NOT_FOUND) {
	    showWarning(
    "Documentation not found.\nExecute doxygen in the top source directory.");
	}
    }
    else if(!strcmp(cmd, "Version")) {
	showHelp(Application::getVersion(), "");
    }
    else if(!strcmp(cmd, "Set Browser...")) {
	string cur_browser, browser;
	if(!getProperty("browser", cur_browser)) cur_browser.assign("firefox");
	if( TextQuestion::askTextQuestion("Set Browser", this,
                "Enter the browser executable", browser, cur_browser, "Apply",
		"Cancel") )
	{
	    putProperty("browser", browser);
	}
    }
    else {
	wplot->actionPerformed(action_event);
    }
}

ParseCmd WaveformWindow::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_NOT_FOUND;
    bool err;
    string c;
    char s[1000];
    const char *p;
    const char *cmds[] = {
	"read", "tableviewer", "connect", "mapping", "tablequery", "import",
	"preferences", "table_files", "fonts", "plugins", "warnings",
	"arrival_parameters", "amplitude_parameters", "edit", "remove_from_db"
    };
    int num = (int)sizeof(cmds)/(int)sizeof(const char *);

    if(missingArgs(cmd, num, cmds, msg)) {
	return ARGUMENT_ERROR;
    }

    for(int i = 0; i < 10; i++) {
	snprintf(s, sizeof(s), "%d", i+2);
	if(parseString(cmd, s, c)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(c, msg);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		WaveformWindow *w = new WaveformWindow(getName(), this);
		windows.push_back(w);
		if(j == i) {
		    return w->parseCmd(c, msg);
		}
	    }
	}
    }

    if(parseString(cmd, "read", c))
    {
	string file;
	if( !parseGetArg(c, "file", file) ) {
	    int i, j;
	    p = c.c_str();
	    for(i = 0; p[i] != '\0' &&  isspace(p[i]); i++);
	    if(p[i] == '"' || p[i] == '\'' || p[i] == '`') {
		for(j = i+1; p[j] != '\0' && p[j] != p[i]; j++);
		i++;
	    }
	    else {
		for(j = i; p[j] != '\0' && !isspace(p[j]); j++);
	    }
	    file.assign(p+i, j-i);
	}
	if( !file.empty() ) {
	    string query;
	    parseGetArg(c, "query", query);
	    char *full_path = Application::fullPath(file);
	    readFile(full_path, query, false);
	    Free(full_path);
	    return COMMAND_PARSED;
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseFind(cmd, "quit", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	setVisible(false);
	FFDatabase::clearStaticTables();
	Application::getApplication()->stop(0);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "recent_input") ||
		parseString(cmd, "recent_input", c)) {
	int i;
	if(!c.empty()) {
	    if( !stringToInt(c.c_str(),&i) || i < 1 || i > recent_input.size()){
		msg.assign(string("Recent_Input: invalid index: ") + c);
		return ARGUMENT_ERROR;
	    }
	}
	else {
	    i = 1;
	    if(recent_input.size() == 0) {
		msg.assign("No recent input.");
		return ARGUMENT_ERROR;
	    }
	}
	RecentInput *r = recent_input[i-1];
	addRecentInput(r); // promotes to the top of the list
	setCursor("hourglass");
	wplot->readRecentInput(r);
	setCursor("default");
	return COMMAND_PARSED;
    }
    else if(parseString(cmd, "tableviewer", c)) {
	if(!tableviewer_window) {
	    tableviewer_window = new TableViewer("TableViewer", this);
	    tableviewer_window->removeAllTabs();
	}
	return tableviewer_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "connect") || parseCompare(cmd, "connect ", 8)
	  || parseCompare(cmd, "get_aao") || parseCompare(cmd, "get_aao ", 8)
	  || parseCompare(cmd, "get_aaow") || parseCompare(cmd,"get_aaow ",9)
	  || parseCompare(cmd, "get_all_tables")
	  || parseCompare(cmd, "get_all_tables ", 15)
	  || parseCompare(cmd, "get_wfdiscs")
	  || parseCompare(cmd, "get_wfdiscs ", 12)
	  || parseCompare(cmd, "mapping") || parseCompare(cmd, "mapping ", 8)
	  || parseCompare(cmd, "query") || parseCompare(cmd, "query ", 6)
	  || parseCompare(cmd, "read_waveforms")
	  || parseCompare(cmd, "read_waveforms ", 15)
	  || parseCompare(cmd, "disconnect") )
    {
	return table_query->parseCmd(cmd, msg);
    }
    else if(parseString(cmd, "tablequery", c)) {
	return table_query->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "convolve") || parseCompare(cmd, "convolve ",9)
	  || parseCompare(cmd, "deconvolve")
	  || parseCompare(cmd, "deconvolve ", 11)
	  || parseCompare(cmd, "remove_con_decon")
	  || parseCompare(cmd, "remove_con_decon ", 17))
    {
	snprintf(s, sizeof(s), "instrument_response.%s", cmd.c_str());
	return pluginParse(s, msg);
    }
    else if(parseString(cmd, "preferences", c)) {
	if(!preferences) {
	    preferences = new Preferences("Preferences", this, wplot);
	}
	return preferences->parseCmd(c, msg);
    }
    else if(parseString(cmd, "table_files", c)) {
	return table_files->parseCmd(c, msg);
    }
    else if(parseString(cmd, "fonts", c)) {
	return Fonts::parse(c, msg);
    }
    else if(parseString(cmd, "plugins", c)) {
	if(!plugin_table) {
	    plugin_table = new PluginTable("Plugins", this);
	}
	return plugin_table->parseCmd(c, msg);
    }
    else if(parseString(cmd, "warnings", c)) {
	Warnings *warnings = Application::getApplication()->warningsWindow();
	if(warnings) return warnings->parseCmd(c, msg);
    }
    else if(parseString(cmd, "arrival_parameters", c)) {
	return wplot->arrivalParams()->parseCmd(c, msg);
    }
    else if(parseString(cmd, "amplitude_parameters", c)) {
	return wplot->amplitudeParams()->parseCmd(c, msg);
    }
    else if(parseString(cmd, "open_file", c)) {
	if(open_file == NULL) {
	    open_file = new OpenFile("Open File", this, EXISTING_FILE, ".",
				cssWfdisc);
	    open_file->addActionListener(this);
	}
	return open_file->parseCmd(c, msg);
    }
    else if(!parseCompare(cmd, "edit.", 5) && parseString(cmd, "edit", c)) {
	return parseEditOrRemove(true, c.c_str(), msg);
    }
    else if(parseString(cmd, "remove_from_db", c)) {
	return parseEditOrRemove(false, c.c_str(), msg);
    }
    else if(parseCompare(cmd, "import ", 7) ||
		parseCompare(cmd, "update_affiliation") ||
		parseCompare(cmd, "update_site") ||
		parseCompare(cmd, "update_sitechan") ||
		parseCompare(cmd, "update_sensor") ||
		parseCompare(cmd, "update_instrument"))
    {
	if(!tableviewer_window) {
	    tableviewer_window = new TableViewer("TableViewer", this);
	}
//	tableviewer_window->removeAllTabs();
	return tableviewer_window->parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
	return COMMAND_PARSED;
    }
    else if((parseCompare(cmd, "listChildren") ||
	     parseCompare(cmd, "listCallbacks") ||
	     parseCompare(cmd, "addToToolbar ", 13) ||
	     parseCompare(cmd,"create ",7) || strstr(cmd.c_str(),".create ")) &&
	(ret = Frame::parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else if( (ret=wplot->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if((ret = Frame::parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
	return ret;
    }
    return COMMAND_NOT_FOUND;
}

ParseCmd WaveformWindow::parseEditOrRemove(bool edit, const char *s,string &msg)
{
    char name[100], table[100];
    const char **table_names=NULL;
    CssClassDescription *des;
    DataSource *ds;
    string c;
    long addr;
    CssTableClass *css, *orig;
    int i, num_members;
    int num_css = CssTableClass::getAllNames(&table_names);

    memset(table, 0, sizeof(table));
    for(i = 0; i < num_css; i++) {
        snprintf(name, sizeof(name), "_%s_", table_names[i]);
        if(parseGetArg(s, "", msg, name, &addr)) {
            strncpy(table, table_names[i], sizeof(table));
            break;
        }
    }
    free(table_names);
    msg.clear();

    if(table[0] == '\0') {
        msg.assign("edit: unrecognized object.");
        return ARGUMENT_ERROR;
    }
    css = (CssTableClass *)addr;

    if( !(ds = css->getDataSource()) ) {
	if(edit) msg.assign("edit: no data source for object.");
	else msg.assign("remove_from_db: no data source for object.");
	return ARGUMENT_ERROR;
    }

    if(edit) {
	ostringstream os;
	num_members = CssTableClass::getDescription(table, &des);

	orig = CssTableClass::createCssTable(table);
	BasicSource::copyTable(orig, css);

	for(i = 0; i < num_members; i++) {
	    if( parseGetArg(s, des[i].name, c) ) {
		if( !css->setMember(i, c) ) {
		    os << "edit: invalid value for " << table << "."
			<< des[i].name << ": " << c;
		    msg.assign(os.str());
		    *css = *orig;
		    delete orig;
		    return ARGUMENT_ERROR;
		}
	    }
	}

	if( css->strictlyEquals(*orig) ) {
	    ds->changeTable(orig, css);
	    TableListener::doCallbacks(orig, css, this);
	}
	delete orig;
    }
    else {
	ds->deleteTable(this, css);
    }
    return COMMAND_PARSED;
}

ParseVar WaveformWindow::parseVar(const string &name, string &value)
{
    ParseVar ret;
    char s[100];
    string c;

    for(int i = 0; i < 10; i++) {
	snprintf(s, sizeof(s), "%d", i+2);
	if(parseString(name, s, c)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseVar(c, value);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		WaveformWindow *w = new WaveformWindow(getName(), this);
		windows.push_back(w);
		if(j == i) {
		    return w->parseVar(c, value);
		}
	    }
	}
    }
    if(parseString(name, "tableviewer", c)) {
	if(!tableviewer_window) {
	    tableviewer_window = new TableViewer("TableViewer", this);
	    tableviewer_window->removeAllTabs();
	}
	return tableviewer_window->parseVar(c, value);
    }
    else if(parseString(name, "tablequery", c)) {
	return table_query->parseVar(c, value);
    }
    else if(parseString(name, "preferences", c)) {
	if(!preferences) {
	    preferences = new Preferences("Preferences", this, wplot);
	}
	return preferences->parseVar(c, value);
    }
    else if(parseString(name, "table_files", c)) {
	return table_files->parseVar(c, value);
    }
/*
    else if(parseString(name, "Fonts", &c)) {
	return Fonts::parse(c, value, value_len);
    }
*/
    else if(parseString(name, "plugins", c)) {
	if(!plugin_table) {
	    plugin_table = new PluginTable("Plugins", this);
	}
	return plugin_table->parseVar(c, value);
    }
    else if(parseString(name, "warnings", c)) {
	Warnings *warnings = Application::getApplication()->warningsWindow();
	if(warnings) return warnings->parseVar(c, value);
    }
    else if(parseString(name, "arrival_parameters", c)) {
	return wplot->arrivalParams()->parseVar(c, value);
    }
    else if(parseString(name, "amplitude_parameters", c)) {
	return wplot->amplitudeParams()->parseVar(c, value);
    }
    else if(parseCompare(name, "instrument.", 11) ||
		parseCompare(name, "instrument[", 11))
    {
	snprintf(s, sizeof(s), "instrument_response.%s", name.c_str());
	return pluginParseVar(s, value);
    }
    else if((ret=wplot->parseVar(name, value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    return Frame::parseVar(name, value);
}

void WaveformWindow::parseHelp(const char *prefix)
{
    char *s;
    printf("\n");
    printf("%sread FILENAME [query=]\n", prefix);
    printf("%slist FILENAME\n", prefix);
    printf("%srecent_input [NUM]\n", prefix);

    wplot->parseHelp(prefix);

    printf("%sfonts.help\n", prefix);
    printf("%stableviewer.help\n", prefix);
    printf("%stablequery.help\n", prefix);
    printf("%spreferences.help\n", prefix);
    printf("%splugins.help\n", prefix);
    printf("%stable_files.help\n", prefix);
    printf("%swarnings.help\n", prefix);

    for(int i = 0; i < (int)plugin_structs.size(); i++) {
	if(plugin_structs[i].name) {
	    s = strdup(plugin_structs[i].name);
	    for(int j = 0; s[j] != '\0'; j++) {
		if(isspace((int)s[j])) s[j] = '_';
		else s[j] = tolower((int)s[j]);
	    }
	    printf("%s%s.help\n", prefix, s);
	    free(s);
	}
    }
    printf("\n");
}

void WaveformWindow::setEditMenuSensitive(void)
{
    bool waveforms_selected = (wplot->numSelected() > 0) ? true : false;

    clear_button->setSensitive(!wplot->empty());
    delete_data_button->setSensitive(waveforms_selected);
    copy_button->setSensitive(waveforms_selected);
    cut_button->setSensitive(waveforms_selected);
    display_calib_button->setSensitive(waveforms_selected);
    display_counts_button->setSensitive(waveforms_selected);
    remove_mean_button->setSensitive(waveforms_selected);
    restore_mean_button->setSensitive(waveforms_selected);
    normal_polarity_button->setSensitive(waveforms_selected);
    reverse_polarity_button->setSensitive(waveforms_selected);
    paste_button->setSensitive(wplot->numClipBoard() > 0 ? true : false);
}

void WaveformWindow::setViewMenuSensitive(void)
{
    int i;
    bool set;
    CssOriginClass *origin;
    gvector<Waveform *> wvec;

    set = (wplot->numSelected() > 0) ? true : false;
    promote_selected->setSensitive(set);
    selected_data->setSensitive(set);

    getWaveforms(wvec);
    for(i = 0; i < wvec.size(); i++) {
	if((origin=getPrimaryOrigin(wvec[i])) &&
		origin->time > NULL_TIME_CHECK) break;
    }
    set = (i < wvec.size()) ? true : false;

    wt.align_origin->setSensitive(set);
    wt.align_predicted->setSensitive(set);
}

void WaveformWindow::open(void)
{
    string dir;

    if(open_file == NULL) {
	if(!getProperty("last_data_dir", dir)) dir.assign(".");
	open_file = new OpenFile("Open File", this, EXISTING_FILE, dir,
				cssWfdisc);
	open_file->setVisible(true);
	open_file->addActionListener(this);
    }
    else {
	open_file->setVisible(true);
    }
}

void WaveformWindow::readFiles(vector<string> &files, const string &query,
				bool update_recent_input)
{
    if((int)files.size() == 0) return;

    struct stat file_stat;
    string dir, s;
    const char **table_names=NULL;
    int i, j, nt=0, num_table_names = CssTableClass::getAllNames(&table_names);
    vector<bool> table_suffix;
    TableSource *ts=NULL;

    for(i = 0; i < (int)files.size(); i++) {
	table_suffix.push_back(false);
	if( stat(files[i].c_str(), &file_stat) ) {
	    for(j = 0; j < num_table_names; j++) {
		s.assign(files[i] + "." + table_names[j]);
		if(!stat(s.c_str(), &file_stat) && !S_ISDIR(file_stat.st_mode)){
		    table_suffix[i] = true;
		    nt++;
		    break;
		}
	    }
	    if(j == num_table_names) {
		cerr << "read: " << files[i] << " not found." << endl;
		Free(table_names);
		return;
	    }
	}
	else if(S_ISDIR(file_stat.st_mode)) {
	    cerr << "read: " << files[i] << " is a directory." << endl;
	    Free(table_names);
	    return;
	}
	else {
	    for(j = 0; j < num_table_names; j++) {
		s.assign(string(".") + table_names[j]);
		if( stringCaseEndsWith(files[i].c_str(), s.c_str()) ) {
		    table_suffix[i] = true;
		    nt++;
		    break;
		}
	    }
	}
    }
    Free(table_names);

    int num_wavs = wplot->numWaveforms();
    const char *format = "";

    setCursor("hourglass");

    for(i = j = 0; i < (int)files.size(); i++) if(table_suffix[i])
    {
	if( !wplot->table_source ) {
	    wplot->table_source = new TableSource("WaveformPlot");
	}
	TableSource *ds = wplot->table_source;

	if(j == 0) {
	    ds->openPrefix(files[i]);
	    ds->queryAllPrefixTables(query);
	}
	else {
	    if(!ts) ts = new TableSource("ts");
	    ts->openPrefix(files[i]);
	    ts->queryAllPrefixTables(query);
	    ds->copyFrom(ts);
	    ts->clearTable("all");
	}

	if(j == nt-1)
	{
	    // ds is saved in wplot
	    wplot->setDataSource(ds);
	    ds->clearTable("all");
	    format= "css";
	}
	j++;
    }
    if(ts) delete ts;

    for(i = 0; i < (int)files.size(); i++) if(!table_suffix[i])
    {
	if(stringCaseEndsWith(files[i].c_str(), ".sac")) {
	    SacSource *s = new SacSource("sac_source", files[i]);
	    wplot->setDataSource(s);
	    format= "sac";
	}
	else if(stringCaseEndsWith(files[i].c_str(), ".seed")) {
	    SeedSource *ss = new SeedSource("seed_source", files[i]);
	    wplot->setDataSource(ss);
	    format= "seed";
	}
	else if(stringCaseEndsWith(files[i].c_str(), ".msg") ||
		stringCaseEndsWith(files[i].c_str(),".gse"))
	{
	    GseSource *gs = new GseSource("gse_source", files[i]);
	    wplot->setDataSource(gs);
	    format= "gse";
	}
	else if(stringCaseEndsWith(files[i].c_str(), ".asc")) {
	    AscSource *asc = new AscSource("asc_source", files[i]);
	    wplot->setDataSource(asc);
	    format= "asc";
	}
	else if(SeedSource::isSeedFile(files[i])) {
	    SeedSource *s = new SeedSource("seed_source", files[i]);
	    wplot->setDataSource(s);
	    format= "seed";
	}
	else if(AscSource::isAscFile(files[i])) {
	    AscSource *asc = new AscSource("asc_source", files[i]);
	    wplot->setDataSource(asc);
	    format= "asc";
	}
	else {
	    int num = wplot->numWaveforms();
	    setPrintError(false);
	    // try GSE message
	    GseSource *gs = new GseSource("gse_source", files[i]);
	    wplot->setDataSource(gs);
	    format= "gse";
	    if( num == wplot->numWaveforms() ) {
		// try SAC
		SacSource *ss = new SacSource("sac_source", files[i]);
		wplot->setDataSource(ss);
		format= "sac";
	    }
	    setPrintError(true);
	}
    }
  
    if((int)files.size() == 1) {
	if(update_recent_input && wplot->numWaveforms() > num_wavs) {
	    RecentInput *r = new RecentInput();
	    r->file = files[0];
	    r->read_all = true;
	    r->format.assign(format);
	    addRecentInput(r);
	}
    }

    size_t n = files[0].find_last_of("/");
    if(n != string::npos) {
	putProperty("last_data_dir", files[0].substr(0, n), false);
    }

    setCursor("default");
}

void WaveformWindow::alignWaveforms(AlignType type, const string &phase)
{
    if(type == ALIGN_PREDICTED_ARRIVAL) {
	wplot->phase_align.assign(phase);
    }
    alignWaveforms(type);
}

void WaveformWindow::alignWaveforms(AlignType type)
{
    setAlignToggles(type);
    wplot->removeActionListener(this, XtNalignCallback);
    wplot->alignWaveforms(type);
    wplot->addActionListener(this, XtNalignCallback);
}

void WaveformWindow::setAlignToggles(AlignType type)
{
    if(type != ALIGN_TRUE_TIME)
    {
        Arg args[1];
        int time_scale;

        XtSetArg(args[0], XtNtimeScale, &time_scale);
        wplot->getValues(args, 1);
        if(time_scale != TIME_SCALE_SECONDS && time_scale != TIME_SCALE_VARIABLE
                && time_scale != TIME_SCALE_HMS)
        {
	    wt.time_scale_variable->set(true, false);
	    wt.time_scale_seconds->set(false, false);
	    wt.time_scale_hms->set(false, false);
	    wt.time_scale_epoch_toggle->set(false, false);
        }
	wt.time_scale_epoch_toggle->setSensitive(false);
    }
    else {
	wt.time_scale_epoch_toggle->setSensitive(true);
    }

    if(type == ALIGN_FIRST_POINT) {
	if(!wt.align_first_point->state()) {
	    wt.align_first_point->set(true, false);
	    wt.align_true_time->set(false, false);
	    wt.align_origin->set(false, false);
	    wt.align_predicted->set(false, false);
	    wt.align_observed->set(false, false);
	}
    }
    else if(type == ALIGN_TRUE_TIME) {
	if(!wt.align_true_time->state()) {
	    wt.align_true_time->set(true, false);
	    wt.align_first_point->set(false, false);
	    wt.align_origin->set(false, false);
	    wt.align_predicted->set(false, false);
	    wt.align_observed->set(false, false);
	}
    }
    else if(type == ALIGN_ORIGIN) {
	if(!wt.align_origin->state()) {
	    wt.align_origin->set(true, false);
	    wt.align_first_point->set(false, false);
	    wt.align_true_time->set(false, false);
	    wt.align_predicted->set(false, false);
	    wt.align_observed->set(false, false);
	}
    }
    else if(type == ALIGN_PREDICTED_P || !wplot->phase_align.compare("FirstP")){
	if(!wt.align_predicted->state()) {
	    wt.align_predicted->set(true, false);
	    wt.align_first_point->set(false, false);
	    wt.align_true_time->set(false, false);
	    wt.align_origin->set(false, false);
	    wt.align_observed->set(false, false);
	}
    }
    else if(type == ALIGN_OBSERVED_P) {
	if(!wt.align_observed->state()) {
	    wt.align_observed->set(true, false);
	    wt.align_first_point->set(false, false);
	    wt.align_true_time->set(false, false);
	    wt.align_origin->set(false, false);
	    wt.align_predicted->set(false, false);
	}
    }
    else if(type == ALIGN_PREDICTED_ARRIVAL) {
	char label[100];
	snprintf(label, sizeof(label), "Time Minus Predicted %s",
		wplot->phase_align.c_str());
	if(!wt.align_predicted_phase) {
	    wt.align_predicted_phase = new Toggle(label, align_menu, this,true);
	}
	else {
	    wt.align_predicted_phase->setLabel(label);
	}
	
	if(!wt.align_predicted_phase->state()) {
	    wt.align_predicted_phase->set(true, false);
	    wt.align_observed->set(false, false);
	    wt.align_first_point->set(false, false);
	    wt.align_true_time->set(false, false);
	    wt.align_origin->set(false, false);
	}
    }
}

void WaveformWindow::print(FILE *fp, PrintParam *p)
{
    AxesParm *a;

    p->right -= 1.;
    if((a = wplot->hardCopy(fp, p, NULL))) {
        free(a);
    }
}

void WaveformWindow::clear(void)
{
    wplot->clear();

/*
    clearUndo(g);
*/
}

void WaveformWindow::deleteSelectedWaveforms(void)
{
    wplot->deleteSelectedWaveforms();

//  updateAfterDelete();
}

void WaveformWindow::copyData(void)
{
    gvector<Waveform *> wvec;
    wplot->getSelectedWaveforms(wvec);
    wplot->copyWaveforms(wvec);
}

void WaveformWindow::cutData(void)
{
    gvector<Waveform *> wvec;
    wplot->getSelectedWaveforms(wvec);
    wplot->cutWaveforms(wvec);
}

void WaveformWindow::pasteData(void)
{
    wplot->pasteData();
}

void WaveformWindow::travelTimeCurves(bool on)
{
    wplot->hideMagWindow();

    travel_times_toggle->set(on, true);

    if(on)
    {
	Toggle *t;
	vector<Component *> *children = align_menu->getChildren();

	align_toggle = NULL;
	for(int i = 0; i < (int)children->size(); i++) {
	    if((t = children->at(i)->getToggleInstance()) && t->state())
	    {
		align_toggle = t;
		break;
	    }
	}
	delete children;

	children = time_scale_menu->getChildren();
	time_scale_toggle = NULL;

	for(int i = 0; i < (int)children->size(); i++) {
	    if((t = children->at(i)->getToggleInstance()) && t->state())
	    {
		time_scale_toggle = t;
		break;
	    }
	}
	delete children;

	if(!wt.align_origin->state()) {
	    wt.align_origin->set(true, true);
	}
	if(!wt.time_scale_variable->state()) {
	    wt.time_scale_variable->set(true, true);
	}
	wplot->setTTCurves(true);
    }
    else {
	wplot->setTTCurves(false);

	if(align_toggle && !align_toggle->state()) {
	    align_toggle->set(true, true);
	}
	if(time_scale_toggle && !time_scale_toggle->state()) {
	    time_scale_toggle->set(true, true);
	}
    }

    sort_menu->setSensitive(!on);
    promote_selected->setSensitive(!on);
    set_default_order->setSensitive(!on);
/*
	    if((w = localWidget(widget, "*_geotool_Form*Scroll")) != NULL) {
		XtSetSensitive(w, !set);
	    }
*/
    space_more->setSensitive(!on);
    space_less->setSensitive(!on);
    set_scale->setSensitive(!on);
    wt.auto_scale->setSensitive(!on);
}

// this makes the new sort button label, but doesn't do the sort and
// there's nothing in actionPerformed to do the sort when the distance_from
// button is activated!?
void WaveformWindow::sortByDistanceFrom(double lat, double lon)
{
    wplot->sort_from_lat = lat;
    wplot->sort_from_lon = lon;

    char slat[20], slon[20], label[100];
    if(lon < 0) {
	int ilon = (int)(-lon+.5);
	if(ilon > 180) ilon = 180;
	if(ilon == 0) strcpy(slon, "0");
	else snprintf(slon, sizeof(slon), "%dW", ilon);
    }
    else {
	int ilon = (int)(lon+.5);
	if(ilon > 180) ilon = 180;
	if(ilon == 0) strcpy(slon, "0");
	else snprintf(slon, sizeof(slon), "%dE", ilon);
    }
    if(lat < 0) {
	int ilat = (int)(-lat+.5);
	if(ilat > 90) ilat = 90;
	if(ilat == 0) strcpy(slat, "0");
	else snprintf(slat, sizeof(slat), "%dS", ilat);
    }
    else {
	int ilat = (int)(lat+.5);
	if(ilat > 90) ilat = 90;
	if(ilat == 0) strcpy(slat, "0");
	else snprintf(slat, sizeof(slat), "%dN", ilat);
    }
    snprintf(label, sizeof(label), "Distance from %s,%s", slon, slat);
    wt.distance_from->setLabel(label);
    wt.distance_from->setVisible(true);
    wt.distance_from->set(true, true);
}

void WaveformWindow::getRecentInput(void)
{
    string s;
    char name[1000], *prop;
    int i, num_recent_input, max_recent_input = 10;

    current_input = NULL;

    num_recent_input = getProperty("num_recent_input", 0);

    if(num_recent_input > max_recent_input) {
	num_recent_input = max_recent_input;
    }

    for(i = 0; i < num_recent_input; i++)
    {
	RecentInput *r = new RecentInput();

	snprintf(name, sizeof(name), "recent_input%d.read_all", i);
	r->read_all = getProperty(name, false);

	snprintf(name, sizeof(name), "recent_input%d.format", i);
	if(!getProperty(name, r->format)) {
	    r->format.assign("css");
	}

	snprintf(name, sizeof(name), "recent_input%d.file", i);
	if( !getProperty(name, r->file) ) {
	    delete r;
	    continue;
	}

	snprintf(name, sizeof(name), "recent_input%d.chan_list", i);
	if((prop = getProperty(name)) != NULL)
	{
	    char *c, *last, *tok = prop;
	    while((c = strtok_r(tok, ", \t", &last)) != NULL) {
		tok = NULL;
		r->chan_list.push_back(string(c));
	    }
	}
	Free(prop);

	snprintf(name, sizeof(name), "recent_input%d.wavs", i);
	if((prop = getProperty(name)) != NULL)
	{
	    char *c, *last, *tok = prop;
	    while((c = strtok_r(tok, ", \t", &last)) != NULL) {
		int wav_index, j, j1, j2;
		char *b;
		tok = NULL;
		if((b = strstr(c, "-")) != NULL) {
		    *b = '\0';
		    if(sscanf(c, "%d", &j1) == 1 && sscanf(b+1, "%d", &j2) == 1)
		    {
			for(j = j1; j <= j2; j++) {
			    r->wavs.push_back(j);
			}
		    }
		}
		else if(sscanf(c, "%d", &wav_index) == 1) {
		    r->wavs.push_back(wav_index);
		}
	    }
	}
	Free(prop);

	if(!r->read_all && (int)r->chan_list.size() == 0
		&& (int)r->wavs.size() == 0)
	{
	    delete r;
	    continue;
	}

	appendRecentInput(r);
    }
}

void WaveformWindow::appendRecentInput(RecentInput *r)
{
    for(int i = 0; i < recent_input.size(); i++) {
	if(r->equals(recent_input[i])) {
	    return;
	}
    }
    recent_input.add(r);
    addRecentButton(r, -1);
}

void WaveformWindow::addRecentInput(RecentInput *r)
{
    int i;
    string path;

    if( !TextField::getAbsolutePath(r->file, path) ) {
	delete r;
	return;
    }
    r->file = path;

    for(i = recent_input.size()-1; i >= 0; i--) {
	if(r->equals(recent_input[i])) {
	    recent_input[i]->button->destroy();
	    if(r != recent_input[i]) {
		recent_input.removeAt(i);
		recent_input.insert(r, 0);
	    }
	    else {
		recent_input.promote(recent_input[i]);
	    }
	    break;
	}
    }
    if(i < 0) {
	recent_input.insert(r, 0);
    }
    addRecentButton(r, 0);
    saveRecentInput();
}

void WaveformWindow::addRecentButton(RecentInput *r, int position)
{
    string name;

    if(r->read_all) {
	name.assign(r->file + string("(all)"));
    }
    else if((int)r->chan_list.size() > 0) {
	name = r->file + string("(");
	for(int j = 0; j < (int)r->chan_list.size() && j < 10; j++) {
	    name.append(r->chan_list[j]);
	    if(j < (int)r->chan_list.size()-1) name.append(",");
	    else if(j == 9 && (int)r->chan_list.size() > 10) {
		name.append(",...");
	    }
	}
	name.append(")");
    }
    else {
	ostringstream os;
	os << r->file << "(" << (int)r->wavs.size() << ")";
	name = os.str();
    }

    if(position >= 0) {
	r->button = new Button(name, recent_files_menu, position, this);
    }
    else {
	r->button = new Button(name, recent_files_menu, this);
    }
    r->button->setCommandString("Recent Input");
}

void WaveformWindow::clearRecentMenu(void)
{
    vector<Component *> *comps = recent_files_menu->getChildren();
    Button *b;

    for(int i = (int)comps->size()-1; i >= 0; i--) {
	if((b = comps->at(i)->getButtonInstance()) && b != clear_recent_button)
	{
	    b->destroy();
	}
    }
    delete comps;

    recent_input.removeAll();

    saveRecentInput();
}

void WaveformWindow::saveRecentInput(void)
{
    int i, j;
    ostringstream name, value;

    value << recent_input.size();
    putProperty("num_recent_input", value.str());

    // clear properties
    for(i = 0; i < 20; i++) {
	name.str("");
	name << "recent_input" << i << ".read_all";
	putProperty(name.str(), "");
	name.str("");
	name << "recent_input" << i << ".format";
	putProperty(name.str(), "");
	name.str("");
	name << "recent_input" << i << ".chan_list";
	putProperty(name.str(), "");
	name.str("");
	name << "recent_input" << i << ".wavs";
	putProperty(name.str(), "");
    }

    for(i = 0; i < recent_input.size(); i++)
    {
	RecentInput *r = recent_input[i];

	name.str("");
	name << "recent_input" << i << ".read_all";
	value.str("");
	value << r->read_all;
	putProperty(name.str(), value.str());

	name.str("");
	name << "recent_input" << i << ".format";
	putProperty(name.str(), r->format);

	name.str("");
	name << "recent_input" << i << ".file";
	putProperty(name.str(), r->file);

	if((int)r->chan_list.size() > 0) {
	    name.str("");
	    name << "recent_input" << i << ".chan_list";
	    value.str("");
	    for(j = 0; j < (int)r->chan_list.size(); j++) {
		value << r->chan_list[j];
		if(j < (int)r->chan_list.size() - 1) value << ",";
	    }
	    putProperty(name.str(), value.str());
	}
	else {
	    name.str("");
	    name << "recent_input" << i << ".chan_list";
	    putProperty(name.str(), " ");
	}

	if((int)r->wavs.size() > 0 && !r->read_all)
	{
	    int j1 = 0, j2 = 0;
	    name.str("");
	    name << "recent_input" << i << ".wavs";
	    value.str("");
	    for(j = 0; j < (int)r->wavs.size(); j++)
	    {
		if(j < (int)r->wavs.size()-1 && r->wavs[j+1] == r->wavs[j] + 1)
		{
		    j2 = j+1;
		}
		else {
		    if(j2 > j1) {
			value << r->wavs[j1] << "-" << r->wavs[j2];
		    }
		    else {
			value << r->wavs[j];
		    }
		    if(j < (int)r->wavs.size()-1) {
			value << ",";
		    }
		    j1 = j+1;
		}
	    }
	    putProperty(name.str(), value.str());
	}
	else {
	    name.str("");
	    name << "recent_input" << i << ".wavs";
	    putProperty(name.str(), " ");
	}
    }
//    Application::writeApplicationProperties();
}

MouseHelp::MouseHelp(const char *name, Component *parent) :
		FormDialog(name, parent)
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    RowColumn *rc = new RowColumn("rc", this, args, n);

    new Button("Close", rc, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    Separator *sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    Label *label = new Label(
"(^) Select waveform segments with Edit/Partial Select on.\n\
(*) Movement must be turned on with View/Data Movement.", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 36); n++;
    XtSetArg(args[n], XtNcolumns, 2); n++;
    const char *column_labels[] = {"Action", "Mouse Button or Key"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    const char *cells[] = {
	"Select an Arrival",			"Left Button Click",
	"Select a Waveform and Deselect Others","Left Button Click",
	"Select a Single Waveform",		"Ctrl Left Button",
	"Select Multiple Waveforms(^)",		"Left Button Drag",
	"Extend Waveform Selection",		"Shift Left Button Click",
	"Scale Waveform",			"Ctrl Right Button Drag",
	"Zoom In",				"Middle Button Drag",
	"Zoom In Horizontally Only",		"Right Button Drag",
	"Zoom Out",				"Middle Button Click",
	"Repeat Zoom In",			"Shift Middle Button Click",
	"Magnify",				"Crtl Middle Button Drag",
	"Expand Double-Line Cursor",		"Right Button Drag",
	"Expand Double-Line Cursor",		"Ctrl Left Button Drag",
	"Move Waveform(*)",			"Left Button Drag",
	"Arrival Menu Popup",			"Middle Button on Arrival",
	"Waveform Menu Popup",			"Middle Button on Label",
	"Cursor Menu Popup",			"Middle Button on Cursor",
	"Add Arrival to Net",			"Right Button Hold",
	"Add Arrival to Sta",			"Shift Right Button Hold",
	"View Menu",				"Ctrl Shift Right Button Hold",
	NULL};

    XtSetArg(args[n], XtNcells, cells); n++;

    Table *table = new Table("table", this, args, n);

    const char *row[2];
    char proc_call[50], key_code[50];
    CPlotKeyAction *keys;
    int num = CPlotClass::getKeyActions(&keys);

    for(int i = 0; i < num; i++) {
	if(!strncmp(keys[i].proc_call, "MoveEntry", 9)) {
	    snprintf(proc_call, sizeof(proc_call), "%s(*)", keys[i].proc_call);
	}
	else {
	    snprintf(proc_call, sizeof(proc_call), "%s", keys[i].proc_call);
	}
	row[0] = proc_call;

	snprintf(key_code, sizeof(key_code), "key: %s", keys[i].key_code);
	row[1] = key_code;
	table->addRow(row, false);
    }
    table->adjustColumns();

    Free(keys);
}

void MouseHelp::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
}

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef __STDC__
ParseCmd WaveformWindow::putCmd(const char *format, ...)
#else
ParseCmd WaveformWindow::putCmd(va_alist) va_dcl
#endif
{
    va_list     va;
#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif
    char cmd[10000];
    string msg;

    vsnprintf(cmd, sizeof(cmd), format, va);
    va_end(va);

    ParseCmd ret;
    if((ret = parseCmd(cmd, msg)) != COMMAND_PARSED)
    {
	if(!msg.empty()) logErrorMsg(LOG_WARNING, msg.c_str());
	return ret;
    }
    return COMMAND_PARSED;
}

void WaveformWindow::changeWindowMenu(const string &window_name)
{
    int i, j;
    Frame *frame = getPluginFrame(window_name);

    if(frame)
    {
	Menu *menu = frame->fileMenu();
	if(menu) {
	    vector<Component *> *children = menu->getChildren();
	    for(i = 0; i < (int)children->size(); i++) {
		Button *b;
		if(!strcmp(children->at(i)->getName(), "Close") && 
		    (b = children->at(i)->getButtonInstance()))
		{
		    b->setLabel("Quit");
		    b->setCommandString("Quit");
		    b->removeAllListeners(XmNactivateCallback);
		    b->addActionListener(this);
		    break;
		}
	    }
	    if(getProperty("add_ww_button", true)) {
		new Button("Waveform Window...", menu, i, this);
	    }
	    if(strcasecmp(frame->getName(), "TableQuery")
			&& getProperty("add_db_button", true)) {
		for(j = 0; j < (int)children->size(); j++) {
		    if(!strncmp(children->at(j)->getName(), "Open Database", 13)
			&& children->at(i)->getButtonInstance()) break;
		}
		if(j == (int)children->size()) {
		    new Button("Open Database...", menu, i+1, this);
		}
	    }
	    delete children;
	}
	menu = frame->helpMenu();
	if(menu) {
	    new Button("Version", menu, this);
	}
    }
    else if(strcasecmp(window_name.c_str(), "open_file") &&
	strcasecmp(window_name.c_str(), "tablequery.connection"))
    {
	FormDialog *form;
	vector<Component *> *w = Application::getApplication()->getWindows();

	if((i = (int)w->size()) > 0 &&
		(form = w->at(i-1)->getFormDialogInstance()))
	{
	    vector<Component *> *children = form->getChildren();
	    for(i = 0; i < (int)children->size()
		&& strcasecmp(children->at(i)->getName(), "controls"); i++);
	    if(i < (int)children->size()) {
		Component *c = children->at(i);
		if(c->getRowColumnInstance()) {
		    Button *b = new Button("Waveforms...", c, this);
		    b->setCommandString("Waveform Window...");
		}
	    }
	    delete children;
	}
    }
}

void WaveformWindow::windowManagerClose(void)
{
    Application *app = Application::getApplication();

    if( strcmp(app->getName(), "geotool") ) {
	setVisible(false);
	return; // if not geotool
    }

    vector<Component *> *wins = app->getWindows();
    bool another_one = false;
    int num = 0;
    for(int i = 0; i < (int)wins->size(); i++) {
        if(wins->at(i)->isVisible()) {
	    num++;
	    if(wins->at(i) !=this && wins->at(i)->getWaveformWindowInstance()) {
		another_one = true;
	    }
	}
    }
    if(!another_one && num > 1) {
        int ans = Question::askQuestion("Confirm", this,
                "Close the Waveform Window?", "Yes", "No");
        if(ans == 1) {
            setVisible(false);
        }
    }
    else {
        setVisible(false);
    }
}

// static
void WaveformWindow::makeIcon(Display *display, Drawable d, int size)
{
    Application *app = Application::getApplication();
    GC gc = XCreateGC(display, d, 0, 0);
    XSetForeground(display, gc, app->stringToPixel("grey90"));

    if(size == 16) // drawble is 16 x 16
    {
	XFillRectangle(display, d, gc, 0, 0, 15, 15);
	int nsegs=15;
	XSegment segs[15];

	segs[0].x1 = 0; segs[0].y1 = 15;
	segs[0].x2 = 15; segs[0].y2 = 15;
	segs[1].x1 = 15; segs[1].y1 = 0;
	segs[1].x2 = 15; segs[1].y2 = 15;
	XSetForeground(display, gc, app->stringToPixel("black"));
	XDrawSegments(display, d, gc, segs, 2);

	int amp1[15] = {0, 0, 0, 0, 1, 1, 2, 3, 2, 1, 2, 3, 1, 2, 0};
	int offset = 4;

	for(int j = 0; j < 15; j++) {
	    segs[j].x1 = j;
	    segs[j].x2 = j;
	    segs[j].y1 = offset - amp1[j];
	    segs[j].y2 = offset + amp1[j];
	}
	XSetForeground(display, gc, app->stringToPixel("blue"));
	XDrawSegments(display, d, gc, segs, nsegs);

	int amp2[15] = {0, 0, 0, 0, 0, 0, 1, 1, 3, 1, 3, 2, 3, 1, 0};
	offset = 11;
	for(int j = 0; j < 15; j++) {
	    segs[j].x1 = j;
	    segs[j].x2 = j;
	    segs[j].y1 = offset - amp2[j];
	    segs[j].y2 = offset + amp2[j];
	}
	XDrawSegments(display, d, gc, segs, nsegs);
    }
    else if(size == 32) {
    }
    else if(size == 48) {
	XFillRectangle(display, d, gc, 0, 0, 45, 45);
	int nsegs=46;
	XSegment segs[46];

	segs[0].x1 =  0; segs[0].y1 = 47;
	segs[0].x2 = 47; segs[0].y2 = 47;
	segs[1].x1 =  1; segs[1].y1 = 46;
	segs[1].x2 = 47; segs[1].y2 = 46;
	segs[2].x1 = 47; segs[2].y1 =  0;
	segs[2].x2 = 47; segs[2].y2 = 47;
	segs[3].x1 = 46; segs[3].y1 =  1;
	segs[3].x2 = 46; segs[3].y2 = 46;

	XSetForeground(display, gc, app->stringToPixel("black"));
	XDrawSegments(display, d, gc, segs, 4);

	int amp1[15] = {0, 0, 0, 0, 1, 1, 2, 3, 2, 1, 2, 3, 1, 2, 0};
	int offset = 4;

	for(int j = 0; j < 15; j++) {
	    segs[j].x1 = j;
	    segs[j].x2 = j;
	    segs[j].y1 = offset - amp1[j];
	    segs[j].y2 = offset + amp1[j];
	}
	XSetForeground(display, gc, app->stringToPixel("blue"));
	XDrawSegments(display, d, gc, segs, nsegs);

	int amp2[15] = {0, 0, 0, 0, 0, 0, 1, 1, 3, 1, 3, 2, 3, 1, 0};
	offset = 11;
	for(int j = 0; j < 15; j++) {
	    segs[j].x1 = j;
	    segs[j].x2 = j;
	    segs[j].y1 = offset - amp2[j];
	    segs[j].y2 = offset + amp2[j];
	}
	XDrawSegments(display, d, gc, segs, nsegs);
    }
    XFreeGC(display, gc);
}
