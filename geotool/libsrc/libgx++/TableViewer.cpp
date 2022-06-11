/** \file TableViewer.cpp
 *  \brief Defines class TableViewer.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sys/param.h>
#include <iostream>
#include <errno.h>
extern "C" {
#include <unistd.h>
}
using namespace std;

#include "widget/TabClass.h"
#include "widget/Table.h"
#include "CSSTable.h"
#include "TableViewer.h"
#include "SelectOrder.h"
#include "Working.h"
#include "motif++/MotifClasses.h"
#include "CssFileDialog.h"
#include "Import.h"
#include <X11/Xmu/Xmu.h>
#include "gobject++/CssTables.h"
#include "cssio.h"
#include "AddStation.h"

extern "C" {
#include "libgmath.h"
#include "libtime.h"
static void * doSystem(void *client_data);
}

long TableViewer::last_arid = -1;
long TableViewer::last_orid = -1;
long TableViewer::last_ampid = -1;

static bool periodsOverlap(double d_on, double d_off, double e_on,
			double e_off);
static bool getOnOff(CssTableClass *t, const string &on, const string &off,
			double *d_on, double *d_off);

TableViewer::TableViewer(const string &name, Component *parent,
		const string &title, bool independent) :
		Frame(name, parent, title, false, independent),
		BasicSource(name)
{
    createInterface();
    init();
}

void TableViewer::createInterface(void)
{
    Frame::createInterface();
    
    setSize(570, 650);

    setFileMenu();
    setEditMenu();
    setViewMenu();
    setOptionMenu();
    setHelpMenu();

    int n = 0;
    Arg args[10];

    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
//    XtSetArg(args[n], XmNbottomOffset, 5); n++;

    tab = new TabClass("tab", frame_form, args, n);
    tab->addActionListener(this, XtNtabCallback);
    tab->addActionListener(this, XtNtabMenuCallback);

    display_text_area = false;
}

void TableViewer::setFileMenu(void)
{
    file_menu->addActionListener(this, XmNcascadingCallback);
    open_button = new Button("Open...", file_menu, this);
    recent_files_menu = new Menu("Recent Files", file_menu);
    getRecentInput();
    clear_recent_button = new Button("Clear Recent Files", recent_files_menu,
					this);
    import_button = new Button("Import...", file_menu, this);
    save_to_file_button = new Button("Save Selected Rows...", file_menu, this);
    save_to_file_button->setSensitive(false);
    save_all_button = new Button("Save All Rows of All Tabs...",file_menu,this);
    update_global_button = new Button("Update", file_menu, this);
    update_global_button->setSensitive(false);
    new_tv_button = new Button("New TableViewer...", file_menu, this);
    close_button = new Button("Close", file_menu, this);
//    file_sep = file_menu->addSeparator("sep", XmDOUBLE_LINE);
//    quit_button = new Button("Quit", file_menu, this);
}

void TableViewer::setEditMenu(void)
{
    add_row_button = new Button("Add Row", edit_menu, this);
    clear_button = new Button("Clear", edit_menu, this);
    clear_button->setSensitive(false);
    copy_rows_button = new Button("Copy Selected Rows", edit_menu, this);
    copy_rows_button->setSensitive(false);
    copy_all_button = new Button("Copy All Rows", edit_menu, this);
    copy_all_button->setSensitive(false);
    copy_columns_button = new Button("Copy Selected Columns", edit_menu, this);
    copy_columns_button->setSensitive(false);
    paste_button = new Button("Paste", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);
    delete_button->setSensitive(false);
    edit_button = new Button("Edit", edit_menu, this);
    edit_button->setSensitive(false);
    tool_bar->add(edit_button);
    vi_edit_button = new Button("vi Edit", edit_menu, this);
    vi_edit_button->setSensitive(false);
    tool_bar->add(vi_edit_button);
    remove_from_file_button = new Button("Remove Selected Rows from File/DB",
					edit_menu, this);
    remove_from_file_button->setSensitive(false);
    remove_tab = new Button("Remove Tab", edit_menu, this);
    remove_tab->setSensitive(false);
    undo_edit_button = new UndoButton("Undo", edit_menu);
    undo_edit_button->setSensitive(false);
    vi_cmd_button = new Button("vi Edit Command...", edit_menu, this);
}

void TableViewer::setViewMenu(void)
{
    attributes_button = new Button("Attributes...", view_menu, this);
    deselect_all_button = new Button("Deselect All", view_menu, this);
    deselect_all_button->setSensitive(false);
    select_all_button = new Button("Select All", view_menu, this);
    select_all_button->setSensitive(true);
    promote_rows_button = new Button("Promote Rows", view_menu, this);
    promote_rows_button->setSensitive(false);
    promote_columns_button = new Button("Promote Columns", view_menu, this);
    promote_columns_button->setSensitive(false);
    sort_button = new Button("Sort", view_menu, this);
    tool_bar->add(sort_button);
    sort_button->setSensitive(false);
    sort_selected_button = new Button("Sort Selected", view_menu, this);
    sort_selected_button->setSensitive(false);
    sort_unique_button = new Button("Sort Unique", view_menu, this);
    sort_unique_button->setSensitive(false);
    expand_button = new Button("Expand", view_menu, this);
    expand_button->setSensitive(false);
    reverse_order_button = new Button("Reverse Order", view_menu, this);
    show_all_button = new Button("Show All", view_menu, this);
    view_menu->addSeparator("sep");
    select_tables_button = new Button("Select Tables...", view_menu, this);
}

void TableViewer::setOptionMenu(void)
{
    map_plugin = PlugInManager::createPlugin("Map", this);

    if(map_plugin) {
	new Button("Map...", option_menu, -1, map_plugin);
	map_plugin->setDataSource(this);
    }

    add_station = new Button("Add Station...", option_menu, this);
}

void TableViewer::setHelpMenu(void)
{
    help_button = new Button("TableViewer Help", help_menu, this);
}

TableViewer::~TableViewer(void)
{
}

void TableViewer::init(void)
{
    num_tabs = 0;

    working = NULL;
    select_order_window = NULL;

    num_undo = 0;
    undo_records = NULL;

    open_file = NULL;
    waveform_receiver = NULL;
    import_window = NULL;
    conn_handle.connection_type = NO_CONNECTION;
    conn_handle.ffdb = NULL;

    table_row_atom = XInternAtom(XtDisplay(base_widget), "TABLE_ROW", False);
    requested = table_row_atom;
    time = CurrentTime;
    memset(vi_path, 0, sizeof(vi_path));
    memset(vi_cmd, 0, sizeof(vi_cmd));
    get_sem = true;

    tab_popup = NULL;
    tab_menu_buttons = NULL;
    num_tab_menu_buttons = 0;

    add_station_window = NULL;

    enableCallbackType(XtNdataChangeCallback);
}

void TableViewer::setQueryText(const string &tableName,
				const string &query_string)
{
    Widget tab_form;

    if((tab_form = tab->getTab(tableName)) != NULL)
    {
	int i;

	for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);

	if(i < num_tabs) {
	    if(tab_forms[i].query_text) {
		tab_forms[i].query_text->setString(query_string);
	    }
	}
    }
}

void TableViewer::appendQueryText(const string &tableName,
				const string &query_string)
{
    Widget tab_form;

    if((tab_form = tab->getTab(tableName)) != NULL)
    {
	int i;

	for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);

	if(i < num_tabs) {
	    if(tab_forms[i].query_text) {
		tab_forms[i].query_text->appendString(query_string);
	    }
	}
    }
}

void TableViewer::saveQueryText(const string &tableName,
				const string &query_string)
{
    Widget tab_form;

    if(!(tab_form = tab->getTab(tableName))) return;

    int i;
    for(i = 0; i < num_tabs &&
	tab_forms[i].tab_form->baseWidget() != tab_form; i++);
    if(i == num_tabs || !tab_forms[i].history) return;

    bool save = false;
    vector<string *> *history = tab_forms[i].history;
    if((int)history->size() > 0) {
	string *s = history->at(0);
	if(query_string.compare(s->data())) {
	    history->insert(history->begin(), new string(query_string));
	    tab_forms[i].up_arrow->setSensitive(true);
	    save = true;
	}
    }
    else {
	history->push_back(new string(query_string));
	save = true;
    }
    if((int)history->size() > 10) {
	history->erase(history->begin()+10, history->end());
    }
    if(save) {
	char name[100];
	for(int j = 0; j < (int)history->size(); j++) {
	    snprintf(name, sizeof(name), "query.%s.%d", tableName.c_str(), j);
	    putProperty(name, history->at(j)->data());
	}
    }
}

void TableViewer::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();
    CSSTable *table = tableOnTop();

    Frame::actionPerformed(action_event);

    if(!strcmp(reason, XmNcascadingCallback)) {
	setFileButtonsSensitive();
    }
    else if(!strcmp(reason, XtNtabCallback)) {
	tabSelect((char *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNtabMenuCallback)) {
	tabMenu((TabMenuCallbackStruct *)action_event->getCalldata());
    }
    else if(!strncmp(cmd, "popup-menu ", 11)) {
	addTableTab(cmd+11, display_text_area);
	tab->setOnTop(tab->numTabs()-1);
	remove_tab->setSensitive(true);
    }
    else if(!strcmp(cmd, "Open...")) {
	open();
    }
    else if(!strcmp(cmd, "Recent Input")) {
	for(int i = 0; i < recent_input.size(); i++) {
	    RecentPath *r = recent_input[i];
	    if(comp == r->button) {
		addRecentInput(r); // promotes to the top of the list
		setCursor("hourglass");
		open(r->path);
		setCursor("default");
		break;
	    }
	}
    }
    else if(!strcmp(cmd, "Clear Recent Files")) {
	clearRecentMenu();
    }
    else if(!strcmp(cmd, "Import...")) {
	if( !import_window ) {
	    import_window = new Import("Import Tables", this, this);
	}
	import_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Add Row")) {
	addRow();
    }
    else if(!strcmp(cmd, "Delete")) {
	deleteSelectedRows();
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Clear")) {
	if(table) table->removeAllRecords();
	setButtonsSensitive();
    }
    else if(comp == remove_from_file_button) {
	if(table) {
	    int n = table->numSelectedRows();
	    if(n > 0) {
		char s[50], b[20];
		if(n == 1) {
		    snprintf(s, sizeof(s), "Remove 1 record from file/DB?");
		    strncpy(b, "Remove Record", sizeof(b));
		}
		else {
		    snprintf(s, sizeof(s), "Remove %d records from file/DB?",n);
		    strncpy(b, "Remove Records", sizeof(b));
		}
		if(Question::askQuestion("Confirm", this, s, b, "Cancel") == 1)
		{
		    table->removeSelectedFromDB();
		}
	    }
	    else {
		showWarning("No rows selected.");
	    }
	}
    }
    else if(!strcmp(cmd, "Remove Tab")) {
	if(table) {
	    removeTableTab(table->getType());
	    if(tab->numTabs() == 0) {
		remove_tab->setSensitive(false);
	    }
	}
    }
    else if(!strcmp(cmd, "Copy Selected Rows")) {
	XmPushButtonCallbackStruct *c =
		(XmPushButtonCallbackStruct *)action_event->getCalldata();
	XButtonEvent *e = (XButtonEvent *)c->event;
	if(table) table->copy(TABLE_COPY_ROWS, e->time);
    }
    else if(!strcmp(cmd, "Copy All Rows")) {
	XmPushButtonCallbackStruct *c =
		(XmPushButtonCallbackStruct *)action_event->getCalldata();
	XButtonEvent *e = (XButtonEvent *)c->event;
	if(table) table->copy(TABLE_COPY_ALL, e->time);
    }
    else if(!strcmp(cmd, "Copy Selected Columns")) {
	XmPushButtonCallbackStruct *c =
		(XmPushButtonCallbackStruct *)action_event->getCalldata();
	XButtonEvent *e = (XButtonEvent *)c->event;
	if(table) table->copy(TABLE_COPY_COLUMNS, e->time);
    }
    else if(!strcmp(cmd, "Paste")) {
	XmPushButtonCallbackStruct *c =
		(XmPushButtonCallbackStruct *)action_event->getCalldata();
	XButtonEvent *e = (XButtonEvent *)c->event;
	paste(e->time);
    }
    else if(!strcmp(cmd, "Select Tables...")) {
	if(select_order_window) {
	    select_order_window->destroy();
	}
	createSelectTables();
        select_order_window->setVisible(true);
	if(select_order_window->applyChange()) {
	   selectTablesApply();
	}
	remove_tab->setSensitive(tab->numTabs() > 0);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	if(table) {
	    table->selectAllRowsWithCB(false);
	    setButtonsSensitive();
	    change.unknown_select = true;
	}
    }
    else if(!strcmp(cmd, "Display Author")) {
    }
    else if(!strcmp(cmd, "Select All")) {
	if(table) {
	    table->selectAllRowsWithCB(true);
	    setButtonsSensitive();
	    change.unknown_select = true;
	}
    }
    else if(!strcmp(cmd, "Promote Rows")) {
	if(table) table->promoteSelectedRows();
    }
    else if(!strcmp(cmd, "Promote Columns")) {
	if(table) table->promoteSelectedColumns();
    }
    else if(!strcmp(cmd, "Sort")) {
	if(table) {
	    vector<int> cols;
	    table->getSelectedColumns(cols);
	    table->sortByColumns(cols);
	}
    }
    else if(!strcmp(cmd, "Sort Selected")) {
	if(table) {
	    vector<int> cols;
	    table->getSelectedColumns(cols);
	    table->sortSelected(cols);
	}
    }
    else if(!strcmp(cmd, "Sort Unique")) {
	if(table) {
	    vector<int> cols;
	    table->getSelectedColumns(cols);
	    table->sortUnique(cols);
	}
    }
    else if(!strcmp(cmd, "Expand")) {
	if(table) {
	    vector<int> cols;
	    table->getSelectedColumns(cols);
	    if((int)cols.size() > 0) table->expand(cols[0]);
	}
    }
    else if(!strcmp(cmd, "Reverse Order")) {
	if(table) table->reverseOrder();
    }
    else if(!strcmp(cmd, "Show All")) {
	if(table) table->showAll();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Quit")) {
	quit(true);
    }
    else if(!strcmp(reason, XtNselectRowCallback)) {
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNselectColumnCallback)) {
	if(table) {
	    bool col_selected = table->numSelectedColumns();
	    copy_columns_button->setSensitive(col_selected);
	    promote_columns_button->setSensitive(col_selected);
	    sort_button->setSensitive(col_selected);
	    bool row_selected = table->numSelectedRows();
	    sort_selected_button->setSensitive(row_selected && col_selected);
	    sort_unique_button->setSensitive(col_selected);
	    expand_button->setSensitive(col_selected);
	    setButtonsSensitive();
	}
    }
    else if(!strcmp(cmd, "Attributes...")) {
	if(table) table->showAttributes(true);
    }
    else if(!strcmp(cmd, "New TableViewer...")) {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->removeAllTabs();
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    TableViewer *tv = new TableViewer(getName(), this);
            tv->removeAllTabs();
	    windows.push_back(tv);
	    tv->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "Edit")) {
	if(table) table->editModeOn();
    }
    else if(comp == vi_edit_button) {
	viEdit();
    }
    else if(!strcmp(cmd, "vi Edit Command...")) {
	string prop, ans;
	 if(getProperty("vi_edit.command", prop)) {
	    TextQuestion::askTextQuestion("vi Edit Command", this,
		"Enter vi Edit Command", ans, prop, "Apply", "Cancel");
	}
	else {
	    TextQuestion::askTextQuestion("vi Edit Command", this,
		"Enter vi Edit Command", ans, "xterm -e vi", "Apply", "Cancel");
	}
	if(!ans.empty()) {
	    putProperty("vi_edit.command", ans);
	}
    }
    else if(!strcmp(cmd, "up_arrow")) {
	upArrow(table);
    }
    else if(!strcmp(cmd, "dn_arrow")) {
	downArrow(table);
    }
    else if(!strcmp(reason, XtNrowChangeCallback)) {
	change.unknown = true;
    }
    else if(comp == save_to_file_button) {
	saveToFile(table);
    }
    else if(comp == save_all_button) {
	saveAllToFile();
    }
    else if(comp == update_global_button) {
	updateGlobalFile();
    }
    else if(!strcmp(cmd, "TableViewer Help")) {
	showHelp("TableViewer Help");
    }
    else if(!strcmp(cmd, "Add Station...")) {
      if(!add_station_window) {
	add_station_window = new AddStation("Add Station", this, this, this);
      }
      add_station_window->setVisible(true);
    }
}

void TableViewer::getRecentInput(void)
{
    int num = 0;
    char *prop = getProperty("tableviewer_recent_input");

    if(prop) {
	char *s, *tok, *last;
	tok = prop;
	while(num < 10 && (s = strtok_r(tok, ":", &last))) {
	    tok = NULL;
	    appendRecentInput(s);
	    num++;
	}
	free(prop);
    }
}

void TableViewer::appendRecentInput(const string &path)
{
    for(int i = 0; i < recent_input.size(); i++) {
	if(!path.compare(recent_input[i]->path)) return;
    }
    RecentPath *r = new RecentPath(path);
    recent_input.add(r);
    addRecentButton(r, -1);
}

void TableViewer::addRecentInput(RecentPath *r)
{
    int i;
    string path;

    if( !TextField::getAbsolutePath(r->path, path) ) {
	delete r;
	return;
    }
    r->path = path;

    for(i = recent_input.size()-1; i >= 0; i--) {
	if(!r->path.compare(recent_input[i]->path)) {
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

void TableViewer::addRecentButton(RecentPath *r, int position)
{
    if(position >= 0) {
	r->button = new Button(r->path, recent_files_menu, position, this);
    }
    else {
	r->button = new Button(r->path, recent_files_menu, this);
    }
    r->button->setCommandString("Recent Input");
}

void TableViewer::clearRecentMenu(void)
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

void TableViewer::saveRecentInput(void)
{
    int n;
    char *s = (char *)malloc(1);
    s[0] = '\0';

    for(int i = 0; i < recent_input.size(); i++) {
	if(i == 0) {
	    n = (int)recent_input[i]->path.length() + 1;
	    s = (char *)realloc(s, n);
	    snprintf(s, n, "%s", recent_input[i]->path.c_str());
	}
	else {
	    n = (int)strlen(s) + (int)recent_input[i]->path.length() + 2;
	    s = (char *)realloc(s, n);
	    snprintf(s+strlen(s), n, ":%s", recent_input[i]->path.c_str());
	}
    }
    putProperty("tableviewer_recent_input", s);
    free(s);
}

void TableViewer::upArrow(CSSTable *table)
{
    int i;
    for(i = 0; i < num_tabs && tab_forms[i].table != table; i++);
    if(i < num_tabs && tab_forms[i].history) {
	if(tab_forms[i].history_index < (int)tab_forms[i].history->size()-1) {
	    tab_forms[i].history_index++;
	    string *s = tab_forms[i].history->at(tab_forms[i].history_index);
	    tab_forms[i].query_text->setString(s->data());
	    if(tab_forms[i].history_index > 0) {
		    tab_forms[i].dn_arrow->setSensitive(true);
	    }
	    if(tab_forms[i].history_index ==(int)tab_forms[i].history->size()-1)
	    {
		tab_forms[i].up_arrow->setSensitive(false);
	    }
	}
    }
}

void TableViewer::downArrow(CSSTable *table)
{
    int i;
    for(i = 0; i < num_tabs && tab_forms[i].table != table; i++);
    if(i < num_tabs && tab_forms[i].history) {
	if(tab_forms[i].history_index > 0) {
	    tab_forms[i].history_index--;
	    string *s = tab_forms[i].history->at(tab_forms[i].history_index);
	    tab_forms[i].query_text->setString(s->data());
	    tab_forms[i].up_arrow->setSensitive(true);
	    if(tab_forms[i].history_index == 0)
	    {
		tab_forms[i].dn_arrow->setSensitive(false);
	    }
	}
    }
}

ParseCmd TableViewer::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    Widget tab_form;
    string c;
    ostringstream os;
    CSSTable *table = tableOnTop();
    const char *cmds[] = {
	"select_tab", "open_file", "select", "select_tables", "attributes",
	"save_selected_rows", "save_all_rows", "save_all_tables", "import",
    };
    int i, num = (int)sizeof(cmds)/(int)sizeof(const char *);
    size_t n;

    for(i = 0; i < num; i++) if(parseCompare(cmd, cmds[i])) {
	msg.assign(string(cmds[i]) + ": missing argument(s)");
	return ARGUMENT_ERROR;
    }

    for(i = 0; i < 10; i++) {
	os.str("");
	os << i+2;
	if(parseString(cmd, os.str(), c)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(c, msg);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		TableViewer *tv = new TableViewer(getName(), this);
		tv->removeAllTabs();
		windows.push_back(tv);
		if(j == i) {
		    return tv->parseCmd(c, msg);
		}
	    }
	}
    }

    if((n = cmd.find('.')) != string::npos) {
	string s = cmd.substr(0, n);
	if((tab_form = tab->getTab(s)) != NULL)
	{
	    for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);
	    if(i < num_tabs) {
		tabSetOnTop(s);
		return tab_forms[i].table->parseCmd(cmd.substr(n+1), msg);
	    }
	}
    }

    if(parseArg(cmd, "select_tab", c)) {
	tabSetOnTop(c);
    }
    else if(parseArg(cmd, "open_file", c)) {
	open(c);
    }
    else if(parseCompare(cmd, "delete_selected_rows")) {
	deleteSelectedRows();
    }
    else if(parseCompare(cmd, "clear")) {
	if(table) table->removeAllRecords();
    }
    else if(parseCompare(cmd, "remove_all_tabs")) {
	removeAllTabs();
    }
    else if(parseCompare(cmd, "remove_tab")) {
	if(table) removeTableTab(table->getType());
    }
    else if(parseCompare(cmd, "copy_selected_rows")) {
	if(table) table->copy(TABLE_COPY_ROWS, CurrentTime);
    }
    else if(parseCompare(cmd, "copy_all_rows")) {
	if(table) table->copy(TABLE_COPY_ALL, CurrentTime);
    }
    else if(parseCompare(cmd, "copy_selected_columns")) {
	if(table) table->copy(TABLE_COPY_COLUMNS, CurrentTime);
    }
    else if(parseCompare(cmd, "paste")) {
	paste((long unsigned int)CurrentTime);
    }
    else if(parseArg(cmd, "select", c)) {
	return parseSelectRecord(c.c_str(), msg);
    }
    else if(parseArg(cmd, "select_tables", c)) {
	if(select_order_window) {
	    select_order_window->destroy();
	}
	createSelectTables();
	if( !select_order_window->parseCmd(c, msg) ) {
	    if(select_order_window->applyChange()) {
		selectTablesApply();
	    }
	    return COMMAND_PARSED;
	}
	return ARGUMENT_ERROR;
    }
    else if(parseCompare(cmd, "deselect_all")) {
	deselect_all_button->activate();
    }
    else if(parseCompare(cmd, "display_author")) {
    }
    else if(parseCompare(cmd, "select_all")) {
	select_all_button->activate();
    }
    else if(parseCompare(cmd, "promote_rows")) {
	promote_rows_button->activate();
    }
    else if(parseCompare(cmd, "promote_columns")) {
	promote_columns_button->activate();
    }
    else if(parseCompare(cmd, "sort")) {
	sort_button->activate();
    }
    else if(parseCompare(cmd, "sort_selected")) {
	sort_selected_button->activate();
    }
    else if(parseCompare(cmd, "sort_unique")) {
	sort_unique_button->activate();
    }
    else if(parseCompare(cmd, "expand")) {
	expand_button->activate();
    }
    else if(parseCompare(cmd, "reverse_order")) {
	reverse_order_button->activate();
    }
    else if(parseCompare(cmd, "show_all")) {
	show_all_button->activate();
    }
    else if(parseCompare(cmd, "close")) {
	setVisible(false);
    }
    else if(parseCompare(cmd, "quit")) {
	quit(false);
    }
    else if(parseArg(cmd, "attributes", c)) {
	if(table) {
	    TableAttributes *ta = table->showAttributes(false);
	    if(ta) {
		return ta->parseCmd(c, msg);
	    }
	}
    }
    else if(parseCompare(cmd, "import ", 7)) {
	if( !import_window ) {
	    import_window = new Import("Import Tables", this, this);
	}
	return import_window->parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "edit")) {
	edit_button->activate();
    }
    else if(parseCompare(cmd, "up_arrow")) {
	if(table) upArrow(table);
    }
    else if(parseCompare(cmd, "down_arrow")) {
	if(table) downArrow(table);
    }
    else if(parseArg(cmd, "save_selected_rows", c)) {
	return parseSave("save_selected_rows", c, true, msg);
    }
    else if(parseArg(cmd, "save_all_rows", c)) {
	return parseSave("save_all_rows", c, false, msg);
    }
    else if(parseArg(cmd, "save_all_tables", c)) {
	string file;
	if( !parseGetArg(c, "file", file) ) {
	    msg.assign("save_all_tables: missing file argument.");
	    return ARGUMENT_ERROR;
	}
	bool append = true;
	parseGetArg(c, "save_all_tables", msg, "append", &append);
	if(!msg.empty()) return ARGUMENT_ERROR;
	saveAll(file, append);
    }
    else if(parseCompare(cmd, "update_affiliation")) {
	tab->setOnTop((char *)"affiliation");
	updateGlobalFile();
    }
    else if(parseCompare(cmd, "update_site")) {
	tab->setOnTop((char *)"site");
	updateGlobalFile();
    }
    else if(parseCompare(cmd, "update_sitechan")) {
	tab->setOnTop((char *)"sitechan");
	updateGlobalFile();
    }
    else if(parseCompare(cmd, "update_sensor")) {
	tab->setOnTop((char *)"sensor");
	updateGlobalFile();
    }
    else if(parseCompare(cmd, "update_gregion")) {
	tab->setOnTop((char *)"gregion");
	updateGlobalFile();
    }
    else if(parseCompare(cmd, "update_instrument")) {
	tab->setOnTop((char *)"instrument");
	updateGlobalFile();
    }
    else if((ret = dataParseCmd(cmd.c_str(), msg)) != COMMAND_NOT_FOUND) {
        return ret;
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if(table && (ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else {
	return Frame::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd TableViewer::parseSave(const string &cmd, const string &c,
			bool selected, string &msg)
{
    int i;
    const char *s = selected ? "save_selected_rows" : "save_all_rows";
    string file, tname;

    if( !parseGetArg(c, "file", file) ) {
	msg.assign(cmd + ": missing file argument.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(c, "table", tname) ) {
	msg.assign(cmd + ": missing table argument.");
	return ARGUMENT_ERROR;
    }
    for(i = 0; i < num_tabs; i++) {
	if(parseCompare(tname, tab_forms[i].table->getType())) break;
    }
    if(i == num_tabs) {
	msg.assign(cmd + ": table " + tname + " not found.");
	return ARGUMENT_ERROR;
    }

    bool append = true;
    parseGetArg(c, s, msg, "append", &append);
    if(!msg.empty()) return ARGUMENT_ERROR;

    string access = (append) ? string("a") : string("w");
    if(selected) {
	writeFile(tab_forms[i].table, file, access);
    }
    else {
	writeAllToFile(tab_forms[i].table, file, access);
    }

    return COMMAND_PARSED;
}

ParseCmd TableViewer::parseSelectRecord(const char *name, string &msg)
{
    int i, n;
    long addr;
    CSSTable *table;
    const char *c;
    char table_name[100];

    // _arrival_=10595232
    if(name[0] != '_') {
	msg.assign(string("select: invalid argument: ") + name);
	return ARGUMENT_ERROR;
    }

    c = name+1;
    while(*c != '\0' && *c != '_') c++;
    if(strncmp(c, "_=", 2) || !stringToLong(c+2, &addr)) {
	msg.assign(string("select: invalid argument: ") + name);
	return ARGUMENT_ERROR;
    }

    n = c-name-1;
    if(n > 99) {
	msg.assign(string("select: invalid argument: ") + name);
	return ARGUMENT_ERROR;
    }

    strncpy(table_name, name+1, n);
    table_name[n] = '\0';

    if((table = getCSSTable(table_name)) == NULL) {
	msg.assign(string("select: invalid argument: ") + name);
	return ARGUMENT_ERROR;
    }

    gvector<CssTableClass *> v;
    table->getRecords(v);

    for(i = 0; i < v.size() && (long)v[i] != addr; i++);

    if(i < v.size()) {
	table->selectRecord(v[i], true);
	return COMMAND_PARSED;
    }

    msg.assign(string("select: invalid argument: ") + name);
    return ARGUMENT_ERROR;
}

ParseVar TableViewer::parseVar(const string &name, string &value)
{
    char c[1000];
    string s;
    int i, n;
    ParseVar ret;

    for(i = 0; i < 10; i++) {
	snprintf(c, sizeof(c), "%d", i+2);
	if(parseString(name, c, s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseVar(s, value);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		TableViewer *tv = new TableViewer(getName(), this);
		windows.push_back(tv);
		if(j == i) {
		    return tv->parseVar(s, value);
		}
	    }
	}
    }

    if((ret = getTableLoop(name.c_str())) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if((ret = getTableMember(name.c_str(), value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if((ret = getTableCmd(name.c_str(), value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }

    for(i = 0; i < num_tabs; i++) {
	snprintf(c, sizeof(c), "%s.",  tab_forms[i].table->getType());
	n = (int)strlen(c);
	if(parseCompare(name, c, n)) {
	    return tab_forms[i].table->parseVar(name.substr(n), value);
	}
    }
    return Frame::parseVar(name, value);
}

void TableViewer::parseHelp(const char *prefix)
{
    char p[200];
    printf("%sselect_tab TAB_NAME\n", prefix);
    printf("%sopen FILENAME\n", prefix);
//    printf("%sadd row \n", prefix);
    printf("%sclear\n", prefix);
    printf("%sremove_tab\n", prefix);
    printf("%scopy_selected_rows\n", prefix);
    printf("%scopy_all_rows\n", prefix);
    printf("%scopy_selected_columns\n", prefix);
    printf("%spaste\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%sdisplay_author\n", prefix);
    printf("%sselect_all\n", prefix);
    printf("%spromote_rows\n", prefix);
    printf("%spromote_columns\n", prefix);
    printf("%ssort\n", prefix);
    printf("%ssort_selected\n", prefix);
    printf("%ssort_unique\n", prefix);
    printf("%sexpand\n", prefix);
    printf("%sreverse_order\n", prefix);
    printf("%sshow_all\n", prefix);
    printf("%sedit\n", prefix);
    printf("%sup_arrow\n", prefix);
    printf("%sdown_arrow\n", prefix);

    Table::parseHelp(prefix);

    snprintf(p, sizeof(p), "%sattributes.", prefix);
    TableAttributes::parseHelp(p);

    printf("%sselect_tables.help\n", prefix);

    Frame::parseHelp(prefix);
}

void TableViewer::open(void)
{
    if(!open_file) {
	open_file = new CssFileDialog("TableViewer Open", this, FILE_ONLY, "./",
					(char *)"All Tables");
    }
    open_file->setVisible(true);

    char *file;
    if( !(file = open_file->getFile()) ) return;

    RecentPath *r = new RecentPath(file);
    addRecentInput(r);

    open(file);

    Free(file);
}

void TableViewer::open(const string &file)
{
    char suffix[100];
    char *filename = strdup(file.c_str());
    int n = (int)strlen(filename);

    suffix[0] = '\0';
    // get prefix and suffix
    for(int i = n-1; i >= 0 && filename[i] != '/'; i--) {
	if(filename[i] == '.') {
	    stringcpy(suffix, filename+i+1, sizeof(suffix));
	    filename[i] = '\0';
	    break;
	}
    }
    if(conn_handle.ffdb) delete conn_handle.ffdb;
    conn_handle.ffdb = FFDatabase::FFDBOpenPrefix(filename);
    conn_handle.prefix.assign(filename);
    conn_handle.connection_type = PREFIX_CONNECTION;
    conn_handle.ffdb->setReadGlobalTables(false);
    char query_string[100];
    const char **table_nams = NULL;
    int num_table_names = CssTableClass::getAllNames(&table_nams);
    CSSTable *table;
    gvector<CssTableClass *> v;

    clearAllTabs();

    for(int i = 0; i < num_table_names; i++)
    {
	char *tableName = (char *)table_nams[i];

	snprintf(query_string, sizeof(query_string), "select * from %s",
		 tableName);
	v.clear();
	conn_handle.ffdb->queryPrefix(query_string, tableName, &v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(tableName))) {
		table = addTableTab(tableName, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }
    // if no local site table, read global site table for Map lat,lon
    if((table = getCSSTable(cssSite)) == NULL || table->numRows() == 0) {
	conn_handle.ffdb->setReadGlobalTables(true);
	v.clear();
	conn_handle.ffdb->queryPrefix("select * from site", cssSite, &v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(cssSite))) {
		table = addTableTab(cssSite, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }
    // if no local sitechan table, read global sitechan table
    if((table = getCSSTable(cssSitechan)) == NULL || table->numRows() == 0) {
	conn_handle.ffdb->setReadGlobalTables(true);
	v.clear();
	conn_handle.ffdb->queryPrefix("select * from sitechan", cssSitechan,&v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(cssSitechan))) {
		table = addTableTab(cssSitechan, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }
    // if no local affiliation table, read global affiliation table
    if((table = getCSSTable(cssAffiliation)) == NULL || table->numRows() == 0) {
	conn_handle.ffdb->setReadGlobalTables(true);
	v.clear();
	conn_handle.ffdb->queryPrefix("select * from affiliation", cssAffiliation, &v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(cssAffiliation))) {
		table = addTableTab(cssAffiliation, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }
    // if no local instrument table, read global instrument table
    if((table = getCSSTable(cssInstrument)) == NULL || table->numRows() == 0) {
	conn_handle.ffdb->setReadGlobalTables(true);
	v.clear();
	conn_handle.ffdb->queryPrefix("select * from instrument", cssInstrument, &v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(cssInstrument))) {
		table = addTableTab(cssInstrument, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }
    // if no local sensor table, read global sensor table
    if((table = getCSSTable(cssSensor)) == NULL || table->numRows() == 0) {
	conn_handle.ffdb->setReadGlobalTables(true);
	v.clear();
	conn_handle.ffdb->queryPrefix("select * from sensor", cssSensor, &v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(cssSensor))) {
		table = addTableTab(cssSensor, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }
    // if no local gregion table, read global gregion table
    if((table = getCSSTable(cssGregion)) == NULL || table->numRows() == 0) {
	conn_handle.ffdb->setReadGlobalTables(true);
	v.clear();
	conn_handle.ffdb->queryPrefix("select * from gregion", cssGregion, &v);
	if(v.size() > 0) {
	    for(int j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    if(!(table = getCSSTable(cssGregion))) {
		table = addTableTab(cssGregion, false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);
        }
    }

    char **tabs = NULL;
    int numtabs = tab->getLabels(&tabs);

    for(int i = 0; i < numtabs; i++) {
        if((table = getCSSTable(tabs[i])) != NULL && table->numRows() == 0) {
	    removeTableTab(table->getType());
        }
    }
    Free(tabs);

    tab->setOnTop(0);

    Free(table_nams);

    setTitle(filename);

    Free(filename);

    setButtonsSensitive();

    change.waveform = true;
    change.arrival = true;
    change.assoc = true;
    change.origin = true;
    change.origerr = true;
    change.stassoc = true;
    change.stamag = true;
    change.netmag = true;
    change.hydro = true;
    change.infra = true;
    change.wftag = true;
    change.amplitude = true;
    change.ampdescript = true;
    change.filter = true;
    change.parrival = true;

    doDataChangeCallbacks();
}

void TableViewer::displayAllTables(DataSource *ds)
{
    const char **table_nams = NULL;
    int num_table_names = CssTableClass::getAllNames(&table_nams);
    CSSTable *table;
    gvector<CssTableClass *> v;
    bool first=true;

    clearAllTabs();

    for(int i = 0; i < num_table_names; i++)
    {
	v.clear();
	ds->getTable(table_nams[i], v);

	if( v.size() > 0) {
	    if(!(table = getCSSTable((char *)table_nams[i]))) {
		table = addTableTab((char *)table_nams[i], false);
	    }
	    else {
		table->removeAllRecords();
	    }
	    table->addRecords(v);

	    if(first) {
		tab->setOnTop((char *)table_nams[i]);
		first = false;
	    }
        }
    }
    Free(table_nams);
}

void TableViewer::displayTable(DataSource *ds, const string &tableName)
{
    CSSTable *table;
    gvector<CssTableClass *> v;

    ds->getTable(tableName, v);
    if( v.size() > 0) {
	if(!(table = getCSSTable(tableName))) {
	    table = addTableTab(tableName, false);
	}
	else {
	    table->removeAllRecords();
	}
	table->addRecords(v);
    }
}

void TableViewer::undoEdit(void)
{
    if(num_undo <= 0) return;

    UndoRecord *undo = &undo_records[num_undo-1];
    CSSTable *table = undo->table;

    table->editModeOff();

    if(undo->type == 1) /* edit */
    {
	if(undo->row > table->numRows()) {
	    showWarning("Cannot undo operation.");
	    return;
	}
	table->setRecord(undo->row, undo->previous, true);
    }
    else if(undo->type == 2)    /* cut */
    {
	for(int i = 0; i < undo->cut_records->size(); i++)
	{
	    table->insertRecord(undo->cut_rows[i],
			undo->cut_records->at(i), false);
	}
	delete undo->cut_records;
	Free(undo->cut_rows);
	table->list();
    }
    else if(undo->type == 3)    /* paste or add row */
    {
	vector<int> rows;

	for(int i = 0; i < undo->num_paste; i++) {
	    rows.push_back(table->numRows() - undo->num_paste + i);
	}
	table->removeRecords(rows);
    }
    num_undo--;
    if(num_undo == 0) {
	undo_edit_button->setSensitive(false);
    }
    else {
	if(undo_records[num_undo-1].type == 1) {
	    undo_edit_button->setLabel("Undo Edit");
	}
	else if(undo_records[num_undo-1].type == 2) {
	    undo_edit_button->setLabel("Undo Cut");
	}
	else if(undo_records[num_undo-1].type == 3) {
	    undo_edit_button->setLabel("Undo Paste");
	}
    }
    change.unknown = true;
    doDataChangeCallbacks();
}

void TableViewer::setFileButtonsSensitive(void)
{
    int i;
    CSSTable *table = tableOnTop();

    // File menu
    if(table && table->numRows() > 0) {
	if(!strcmp(table->getType(), "site")) {
	    update_global_button->setLabel("Update global.site");
	    update_global_button->setSensitive(true);
	}
	else if(!strcmp(table->getType(), "sitechan")) {
	    update_global_button->setLabel("Update global.sitechan");
	    update_global_button->setSensitive(true);
	}
	else if(!strcmp(table->getType(), "sensor")) {
	    update_global_button->setLabel("Update global.sensor");
	    update_global_button->setSensitive(true);
	}
	else if(!strcmp(table->getType(), "gregion")) {
	    update_global_button->setLabel("Update global.gregion");
	    update_global_button->setSensitive(true);
	}
	else if(!strcmp(table->getType(), "affiliation")) {
	    update_global_button->setLabel("Update global.affliation");
	    update_global_button->setSensitive(true);
	}
	else if(!strcmp(table->getType(), "instrument")) {
	    update_global_button->setLabel("Update global.instrument");
	    update_global_button->setSensitive(true);
	}
	else if(!strcmp(table->getType(), "network")) {
	    update_global_button->setLabel("Update global.network");
	    update_global_button->setSensitive(true);
	}
	else {
	    update_global_button->setSensitive(false);
	}
    }
    else {
	update_global_button->setLabel("Update");
	update_global_button->setSensitive(false);
    }
    for(i = 0; i < num_tabs; i++) {
	if(tab_forms[i].table->numRows() > 0) break;
    }
    save_all_button->setSensitive(i < num_tabs);
}

void TableViewer::setButtonsSensitive(void)
{
    char s[100];
    CSSTable *table = tableOnTop();
    if(!table) {
	remove_tab->setSensitive(false);
	return;
    }
    remove_tab->setSensitive(true);
    int num = table->numRows();

    // Edit menu
    int row_selected = table->numSelectedRows();
    int col_selected = table->numSelectedColumns();

    clear_button->setSensitive(num > 0 ? true : false);
    copy_rows_button->setSensitive(row_selected);
    copy_all_button->setSensitive(num > 0 ? true : false);
    delete_button->setSensitive(row_selected);
    remove_from_file_button->setSensitive(row_selected);
    edit_button->setSensitive(row_selected);
    if(row_selected && col_selected) {
	if(row_selected == 1 && col_selected == 1)
	    vi_edit_button->setLabel("vi Edit Cell");
	else vi_edit_button->setLabel("vi Edit Cells");
    }
    else if(row_selected) {
	if(row_selected == 1) vi_edit_button->setLabel("vi Edit Row");
	else vi_edit_button->setLabel("vi Edit Rows");
    }
    else if(col_selected) {
	if(col_selected == 1) vi_edit_button->setLabel("vi Edit Column");
	else vi_edit_button->setLabel("vi Edit Columns");
    }
    else {
	vi_edit_button->setLabel("vi Edit");
    }
    vi_edit_button->setSensitive(row_selected || col_selected);
 
    snprintf(s, sizeof(s), "Save Selected %s Rows...", table->getType());
    save_to_file_button->setLabel(s);
    save_to_file_button->setSensitive(row_selected);

    // View menu
    deselect_all_button->setSensitive(row_selected);
    promote_rows_button->setSensitive(row_selected);

    copy_columns_button->setSensitive(col_selected);
    promote_columns_button->setSensitive(col_selected);
    sort_button->setSensitive(col_selected);
    sort_selected_button->setSensitive(row_selected && col_selected);
    sort_unique_button->setSensitive(col_selected);
    expand_button->setSensitive(col_selected);
}

void TableViewer::createSelectTables(void)
{
    const char **cssTableNames=NULL;
    int num = CssTableClass::getAllNames(&cssTableNames);
    int *order = (int *)mallocWarn(num*sizeof(int));

    char **tab_labels = NULL;
    int i, j, ntabs = tab->getLabels(&tab_labels);

    for(i = 0; i < num; i++) order[i] = -1;

/*
    int n = 0;
    for(i = 0; i < ntabs; i++) {
        for(j = 0; j < num && strcmp(tab_labels[i], cssTableNames[j]); j++);
        if(j < num) {
            order[j] = n++;
        }
    }
*/
    int num_selected = 0;
    for(i = 0; i < ntabs; i++) {
        for(j = 0; j < num && strcmp(tab_labels[i], cssTableNames[j]); j++);
        if(j < num) {
            order[num_selected++] = j;
        }
    }
    Free(tab_labels);

    select_order_window = new SelectOrder("Select Table Tabs", this, num,
				cssTableNames, num_selected, order);
    Free(cssTableNames);
    Free(order);
}

CSSTable *TableViewer::addTableTab(const string &tableName)
{
    return addTableTab(tableName, true);
}

CSSTable *TableViewer::addTableTab(const string &tableName, bool display_ta)
{
    Separator *sep=NULL;
    ScrolledWindow *sw;
    Form *form, *topform;
    int n;
    Arg args[20];
    char name[200];

    if(num_tabs == MAX_TABS) {
	cerr << "addTable: too many tabs." << endl;
	return NULL;
    }

    n = 0;
    XtSetArg(args[n], XtNtitle, tableName.c_str()); n++;
    topform = new Form(tableName, tab, args, n, false);
    tab_forms[num_tabs].tab_form = topform;

    if(display_ta)
    {
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	form = new Form("form", topform, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	Form *form2 = new Form("form2", form, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNarrowDirection, XmARROW_UP); n++;
	XtSetArg(args[n], XmNshadowThickness, 0); n++;
	XtSetArg(args[n], XmNhighlightThickness, 0); n++;
	tab_forms[num_tabs].up_arrow =
			new ArrowButton("up_arrow", form2, args, n, this);
	tab_forms[num_tabs].up_arrow->setSensitive(false);

	n = 0;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNarrowDirection, XmARROW_DOWN); n++;
	XtSetArg(args[n], XmNshadowThickness, 0); n++;
	XtSetArg(args[n], XmNhighlightThickness, 0); n++;
	tab_forms[num_tabs].dn_arrow =
			new ArrowButton("dn_arrow", form2, args, n, this);
	tab_forms[num_tabs].dn_arrow->setSensitive(false);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget, form2->baseWidget()); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
	XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
	sw = new ScrolledWindow("sw", form, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg(args[n], XmNwordWrap, true); n++;
	XtSetArg(args[n], XmNrows, 5); n++;
	XtSetArg(args[n], XmNscrollHorizontal, false); n++;
	XtSetArg(args[n], XmNscrollVertical, true); n++;

	tab_forms[num_tabs].query_text = new TextField("queryText", sw, args,n);
	tab_forms[num_tabs].query_text->addActionListener(this,
			XmNactivateCallback);
	tab_forms[num_tabs].query_text->setActivationKeys(";");
	tab_forms[num_tabs].query_text->tabKeyFocus();

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, form->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 5); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep =  new Separator("sep", topform, args, n);

	tab_forms[num_tabs].history = new vector<string *>;
	tab_forms[num_tabs].history_index = 0;
	string prop;
	for(int i = 0; i < MAX_TABS; i++) {
	    snprintf(name, sizeof(name), "query.%s.%d", tableName.c_str(), i);
	    if( getProperty(name, prop) ) {
		tab_forms[num_tabs].history->push_back(new string(prop));
	    }
	    else {
		break;
	    }
	}
	if((int)tab_forms[num_tabs].history->size() > 0) {
	    tab_forms[num_tabs].up_arrow->setSensitive(true);
	    tab_forms[num_tabs].history_index = -1;
	}
    }
    else {
	tab_forms[num_tabs].query_text = NULL;
	tab_forms[num_tabs].up_arrow = NULL;
	tab_forms[num_tabs].dn_arrow = NULL;
	tab_forms[num_tabs].history = NULL;
	tab_forms[num_tabs].history_index = 0;
    }

    snprintf(name, 200, "%s table", tableName.c_str());
/*                XtNtableTitle, name, */
    n = 0;
    if(display_ta) {
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, sep->baseWidget()); n++;
    }
    else {
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    }
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNeditable, false); n++;
    XtSetArg(args[n], XtNdoInternalSelectRowCB, True); n++;

    CSSTable *table = new CSSTable(tableName, topform, info_area, args, n);

    table->setVisible(true);
    table->setType(tableName);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNselectColumnCallback);
    table->addActionListener(this, XtNrowChangeCallback);

    tab_forms[num_tabs++].table = table;

    tab->update();

    return table;
}

void TableViewer::tabSelect(const string &tab_name)
{
    setButtonsSensitive();
}

gvector<CssTableClass *> * TableViewer::getTableRecords(const string &tableName,
			bool show_warning)
{
    CSSTable *table;

    if((table = getCSSTable(tableName)) == NULL) {
	if(show_warning) {
	    showWarning("No %s tab found.", tableName.c_str());
	}
	return NULL;
    }
    return table->getRecords();
}

int TableViewer::getSelectedTableRecords(const string &tableName,
		gvector<CssTableClass *> &v, bool show_warning)
{
    CSSTable *table;

    if((table = getCSSTable(tableName)) == NULL) {
	if(show_warning) {
	    showWarning("No %s tab found.", tableName.c_str());
	}
	return 0;
    }
    return table->getSelectedRecords(v);
}

int TableViewer::getTable(const string &cssTableName,gvector<CssTableClass *> &table)
{
    CSSTable *css;

    if((css = getCSSTable(cssTableName)) == NULL) {
	return 0;
    }
    return css->getRecords(table);
}

int TableViewer::getSelectedTable(const string &cssTableName,
				gvector<CssTableClass *> &table)
{
    CSSTable *css;

    if((css = getCSSTable(cssTableName)) == NULL) {
	return 0;
    }
    return css->getSelectedRecords(table);
}

bool TableViewer::selectTableRecord(const string &tableName, CssTableClass *css,
					bool select)
{
    CSSTable *table;

    if( (table = getCSSTable(tableName)) ) {
	bool ret = table->selectRecord(css, select);
	setButtonsSensitive();
	return ret;
    }
    return false;
}

string TableViewer::getTabName(void)
{
    Widget tab_form;
    if((tab_form = tab->getTabOnTop()) == NULL) return string("");

    int i;
    for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);
    if(i == num_tabs) return string("");
    return string(XtName(tab_forms[i].tab_form->baseWidget()));
}


CSSTable * TableViewer::getCSSTable(const string &tableName)
{
    int i;
    Widget tab_form;

    if((tab_form = tab->getTab(tableName)) == NULL) return NULL;

    for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);

    return (i < num_tabs) ? tab_forms[i].table : NULL;
}

void TableViewer::selectTablesApply(void)
{
    int i, j, num_selected;
    char **tabs = NULL, **orderedNames = NULL;
    char prop[2000], name[100];

    int numTabs = tab->getLabels(&tabs);
        
    tab->setVisible(false);

    num_selected = select_order_window->getNumSelected();
    orderedNames = select_order_window->getNames();
        
    for(i = 0; i < num_selected; i++) {
	for(j = 0; j < numTabs && strcmp(orderedNames[i], tabs[j]); j++);
	if(j == numTabs) {
	    addTableTab(orderedNames[i], display_text_area);
	}
    }
    for(j = 0; j < numTabs; j++) {
	for(i = 0; i < num_selected && strcmp(orderedNames[i], tabs[j]); i++);
	if(i == num_selected) {
	    removeTableTab(tabs[j]);
	}   
    }
    Free(tabs);

    tab->orderTabs(num_selected, orderedNames);
        
    prop[0] = '\0';
    for(i = 0; i < num_selected; i++) {
	if(strlen(prop)+1 + strlen(orderedNames[i]) > sizeof(prop)-1) break;
	if(i > 0) strcat(prop, ",");
	strcat(prop, orderedNames[i]);
    }
    snprintf(name, sizeof(name), "%s.tableTabs", getName());
    putProperty(name, prop);
    Application::writeApplicationProperties();
                
    tab->setVisible(true);
}

void TableViewer::removeTableTab(const string &tableName)
{
    Widget tab_form;

    if((tab_form = tab->getTab(tableName)) != NULL)
    {
	int i;
	for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);
	if(i < num_tabs) {
	    tab_forms[i].table->removeActionListener(this,XtNselectRowCallback);
	    tab_forms[i].table->removeActionListener(this,XtNselectColumnCallback);
	    tab_forms[i].table->removeActionListener(this,XtNrowChangeCallback);
	    tab_forms[i].table->removeAllRecords();
	    if(tab_forms[i].history) {
		for(int j = 0; j < (int)tab_forms[i].history->size(); j++) {
		    delete tab_forms[i].history->at(j);
		}
		delete tab_forms[i].history;
	    }
	    for(int j = i; j < num_tabs-1; j++) {
		tab_forms[j] = tab_forms[j+1];
	    }
	   num_tabs--;
	}
	tab->deleteTab(tableName);
    }
    change.unknown = true;
    doDataChangeCallbacks();
}

void TableViewer::removeAllTabs(void)
{
    for(int i = 0; i < num_tabs; i++) {
	tab_forms[i].table->removeActionListener(this, XtNselectRowCallback);
	tab_forms[i].table->removeActionListener(this, XtNselectColumnCallback);
	tab_forms[i].table->removeActionListener(this, XtNrowChangeCallback);

	tab_forms[i].table->removeAllRecords();

	if(tab_forms[i].history) {
	    for(int j = 0; j < (int)tab_forms[i].history->size(); j++) {
		delete tab_forms[i].history->at(j);
	    }
	    delete tab_forms[i].history;
	}
    }
    num_tabs = 0;
    tab->deleteAllTabs();
    change.unknown = true;
    doDataChangeCallbacks();
}

void TableViewer::clearTab(const string &tableName)
{
    Widget tab_form;

    if((tab_form = tab->getTab(tableName)) != NULL)
    {
	int i;
	for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);

	if(i < num_tabs) {
	    tab_forms[i].table->removeAllRecords();
	    if(tab_forms[i].query_text) {
		tab_forms[i].query_text->setString("");
	    }
	}
    }
}

void TableViewer::showWorking(int num, const char **labels)
{
    if(working) {
	closeWorking();
    }
    if(num > 0) {
	working = new Working("Working", tab, num, labels);
	if( isVisible() ) {
	    working->setVisible(true, CENTER_DIALOG);
	}
    }
}

void TableViewer::closeWorking(void)
{
    if(working) {
	working->setVisible(false);
	working->destroy();
    }
    working = NULL;
}

bool TableViewer::updateWorking(const string &tableName, int num_records)
{
    if(working) return working->update(tableName, num_records);
    return true;
}

void TableViewer::putTabProperty(void)
{
    char **tab_labels = NULL, name[100];
    int ntabs = tab->getLabels(&tab_labels);

    if(!ntabs) {

	putProperty(name, "origin,arrival,wfdisc");
    }
    else {
	char prop[2000];
	prop[0] = '\0';
	for(int i = 0; i < ntabs; i++) {
	    if(strlen(prop)+1 + strlen(tab_labels[i]) > sizeof(prop)-1) break;
	    if(i > 0) strcat(prop, ",");
	    strcat(prop, tab_labels[i]);
	}
	snprintf(name, sizeof(name), "%s.tableTabs", getName());
	putProperty(name, prop);
    }
    Free(tab_labels);
    Application::writeApplicationProperties();
}

CSSTable *TableViewer::tableOnTop(void)
{
    Widget tab_form;

    if((tab_form = tab->getTabOnTop()) == NULL) return NULL;

    int i;
    for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);
    if(i == num_tabs) return NULL;

    return tab_forms[i].table;
}

void TableViewer::addRecords(gvector<CssTableClass *> &records)
{
    if(records.size() <= 0) return;

    const char *tableName = records[0]->getName();
    CSSTable *table = getCSSTable(tableName);

    if(!table) {
        table = addTableTab(tableName, display_text_area);
        putTabProperty();
    }

    tab->setOnTop(tableName);
    table->addRecords(records);
    tabSelect(tableName);
}

void TableViewer::insertRecords(int index, gvector<CssTableClass *> &records)
{
    if(records.size() <= 0) return;

    const char *tableName = records[0]->getName();
    CSSTable *table = getCSSTable(tableName);

    if(!table) {
        table = addTableTab(tableName, display_text_area);
        putTabProperty();
    }

    tab->setOnTop(tableName);
    table->insertRecords(index, records);
    tabSelect(tableName);
}

bool TableViewer::emptyTab(const string &cssTableName)
{
    CSSTable *table;
    if( !(table = getCSSTable(cssTableName)) || !table->numRows() ) return true;
    return false;
}

void TableViewer::tabSetOnTop(const string &cssTableName)
{
   if(tab->getTab(cssTableName)) tab->setOnTop(cssTableName);
}

void TableViewer::clearAllTabs(void)
{
    char **labels = NULL, *prop, name[100];
    int n = tab->getLabels(&labels);

    for(int i = 0; i < n; i++) labels[i] = strdup(labels[i]);

    for(int i = 0; i < n; i++) {
	clearTab(labels[i]);
    }

    snprintf(name, sizeof(name), "%s.tableTabs", getName());
    if( (prop = getProperty(name)) ) {
	char **tabs = (char **)mallocWarn(sizeof(char *));
	char *c, *tok = prop, *last;
	int num = 0;
	while((c = (char *)strtok_r(tok, " ,;:\t", &last)) != NULL)
        {
	    tok = NULL;
	    tabs = (char **)realloc(tabs, (num+1)*sizeof(char *));
	    tabs[num++] = c;
	}

	for(int i = n-1; i >= 0; i--) {
	    int j;
	    for(j = 0; j < num && strcmp(labels[i], tabs[j]); j++);
	    if(j == num) {
		removeTableTab(labels[i]);
	    }
	}
	free(tabs);
        free(prop);
    }
    for(int i = 0; i < n; i++) Free(labels[i]);
    Free(labels);
    change.unknown = true;
    doDataChangeCallbacks();
}

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void TableViewer::
#ifdef __STDC__
ShowWarning(const char *format, ...)
#else
ShowWarning(va_alist) va_dcl
#endif
{
    va_list     va;
#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif
    char warning[1000];
    vsnprintf(warning, sizeof(warning), format, va);
    va_end(va);
    showWarning(warning);
    fprintf(stderr, "%s\n", warning);
}

void TableViewer::
#ifdef __STDC__
PutWarning(const char *format, ...)
#else
PutWarning(va_alist) va_dcl
#endif
{
    va_list     va;
#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif
    char warning[1000];
    vsnprintf(warning, sizeof(warning), format, va);
    va_end(va);
    putWarning(warning);
    fprintf(stderr, "%s\n", warning);
}

void TableViewer::saveToFile(CSSTable *table)
{
    char *file, filter[100];
    const char *access;
    bool append;

    if(!table) return;

    table->editModeOff();

    if(table->numSelectedRows() <= 0) {
	showWarning("No rows selected.");
	return;
    }

    snprintf(filter, sizeof(filter), "*.%s", getTabName().c_str());

    if( (file = CssFileDialog::getFile("Save to File", this, "./", filter,
		"Open", &append)) )
    {
	access = (append) ? "a" : "w";

	writeFile(table, file, access);
	Free(file);
    }
}

void TableViewer::saveAllToFile(void)
{
    char *file;
    bool append;
    int i;

    for(i = 0; i < num_tabs; i++) {
	if(tab_forms[i].table->numRows() > 0) break;
    }
    if(i == num_tabs) return;

    if( (file = CssFileDialog::getFile("Save all tables", this, "./", "*",
		"Open", &append)) )
    {
	saveAll(file, append);
	free(file);
    }
}

void TableViewer::saveAll(const string &file, bool append)
{
    string prefix;
    char tname[100], path[MAXPATHLEN+1];
    const char *access, **tablenames=NULL;
    int i, len, n, num_css;

    access = (append) ? "a" : "w";

    num_css = CssTableClass::getAllNames(&tablenames);
    len = (int)file.length();

    prefix.assign(file);
    // if file = "something.table_name", make it "something"
    for(i = 0; i < num_css; i++) {
	snprintf(tname, sizeof(tname), ".%s", tablenames[i]);
	n = (int)strlen(tname);
	if(len > n && !strcasecmp(file.c_str()+len-n, tname)) {
	    
	    prefix.assign(file.substr(0, len-n));
	    break;
	}
    }
    free(tablenames);

    for(i = 0; i < num_tabs; i++) if(tab_forms[i].table->numRows() > 0)
    {
	tab_forms[i].table->editModeOff();
	snprintf(path, sizeof(path), "%s.%s", prefix.c_str(),
			tab_forms[i].table->getType());
	writeAllToFile(tab_forms[i].table, path, access);
    }
}

void TableViewer::writeFile(CSSTable *table, const string &file,
				const string &access)
{
    FILE *fp;
    char *s;
    vector<int> rows;
    int nselected;
    gvector<CssTableClass *> v;

    table->getRecords(v);

    if((nselected = table->getSelectedRows(rows)) == 0) {
	showWarning("No rows selected.");
	return;
    }

    if( !(fp = fopen(file.c_str(), access.c_str())) ) {
	showWarning("Cannot write to %s", file.c_str(), strerror(errno));
    }
    else {
	for(int i = 0; i < nselected; i++) {
	    CssTableClass *css = (CssTableClass *)v[rows[i]];
	    DateTime *dt = (DateTime *)css->memberAddress("lddate");
	    if(dt && css->memberType("lddate") == CSS_LDDATE) {
		timeEpochToDate(timeGetEpoch(), dt);
	    }
	    s = css->toString();
	    fprintf(fp, "%s\n", s);
	    Free(s);
	}
	fclose(fp);
    }
}

void TableViewer::writeAllToFile(CSSTable *table, const string &file,
				const string &access)
{
    FILE *fp;
    char *s;
    vector<int> order;
    int num = table->getRowOrder(order);
    gvector<CssTableClass *> *v = table->getRecords();

    if( !(fp = fopen(file.c_str(), access.c_str())) ) {
	showWarning("Cannot write to %s", file.c_str(), strerror(errno));
    }
    else {
	for(int i = 0; i < num && v && i < v->size(); i++) {
	    CssTableClass *css = (CssTableClass *)v->at(order[i]);
	    DateTime *dt = (DateTime *)css->memberAddress("lddate");
	    if(dt && css->memberType("lddate") == CSS_LDDATE) {
		timeEpochToDate(timeGetEpoch(), dt);
	    }
	    s = css->toString();
	    fprintf(fp, "%s\n", s);
	    Free(s);
	}
	fclose(fp);
    }
}

void TableViewer::paste(Time selection_time)
{
    time = selection_time;
    requested = table_row_atom;
    XtGetSelectionValue(baseWidget(), XA_PRIMARY, table_row_atom,
        (XtSelectionCallbackProc)requestorCallback, (XtPointer)this, time);
}

void TableViewer::requestorCallback(Widget w, XtPointer p, Atom *selection,
        Atom *type, XtPointer value, unsigned long *length, int *format)
{
    TableViewer *t = (TableViewer *)p;
    if( !t ) return;

    if((*type == 0 /* XT_CONVERT_FAIL */) || (*length == 0)) {
	if(t->requested == t->table_row_atom)    // try requesting a string
	{
            t->requested = XA_STRING;
            XtGetSelectionValue(w, XA_PRIMARY, XA_STRING,
                (XtSelectionCallbackProc)requestorCallback, p, t->time);
        }
        else {
            XBell(XtDisplay(w), 100);
            fprintf(stderr,
                "TableViewer: no selection or selection timed out.\n");
        }
        return;
    }
    else if(*type == t->table_row_atom)
    {
	if(*length < 2*sizeof(int)) {
	    Free(value);
	    return;
	}
	int nbytes, name, n;
	char *b = (char *)value;

	n = 0;
	memcpy(&nbytes, b, sizeof(int)); n += sizeof(int);
	memcpy(&name, b+n, sizeof(int));

	CSSTable *table = NULL;
	for(int i = 0; i < (int)*length; i += nbytes) {
	    CssTableClass *c = CssTableClass::fromBytes(nbytes, b+i);
	    if(c) {
		if(!(table = t->getCSSTable(c->getName()))) {
		    table = t->addTableTab(c->getName(), false);
		}
		table->addRecord(c, false);
	    }
	}
	if(table) table->list();
    }
    else if(*type == XA_STRING || *type == XA_COMPOUND_TEXT(XtDisplay(w)))
    {
	const char *name = NULL;
	CSSTable *table = t->tableOnTop();
	CssTableClass *css;
	if(table && (css = table->getRecord(0))) {
	    name = css->getName();
	}
	else if(table) {
	    name = table->getType();
	}
	if(!name || name[0] == '\0') {
	    fprintf(stderr, "CSSTable.paste: unknown table type.\n");
	}
	else {
	    css = CssTableClass::createCssTable(name);
	    if(css) {
		char *c = (char *)value;
		int len = css->getLineLength() + 1;
		for(int i = 0; i < (int)(*length); i += len) {
		    if( readCssTable(c+i, css) ) {
			table->addRecord(css, false);
			css = CssTableClass::createCssTable(name);
		    }
		}
		delete css;
		table->list();
	    }
	}
    }
    t->setButtonsSensitive();

    Free(value);
    t->change.unknown = true;
    t->doDataChangeCallbacks();
}

bool TableViewer::readCssTable(char *line, CssTableClass *o)
{
    int num_members = o->getNumMembers();
    CssClassDescription *des = o->description();

    for(int i = 0; i < num_members; i++)
    {
	line[des[i].end] = '\0';
	stringTrim(&line[des[i].start-1]);

	if( !o->setMember(i, &line[des[i].start-1]) ) return false;
    }
    return true;
}

void TableViewer::addRow(void)
{
    CSSTable *table = tableOnTop();
    if( !table ) return;
    const char *name = table->getType();
    CssTableClass *css = CssTableClass::createCssTable(name);
    if(conn_handle.connection_type != NO_CONNECTION) {
	if( !writeTable(&conn_handle, css, name) ) {
	    if(conn_handle.connection_type == ODBC_CONNECTION) {
#ifdef HAVE_LIBODBC
		showWarning(ODBCErrMsg());
#endif
	    }
	    else  {
		showWarning(FFDatabase::FFDBErrMsg());
	    }
	}
	else {
	    css->setDataSource(this);
	}
    }
    table->addRecord(css, true);
}

void TableViewer::deleteSelectedRows(void)
{
    CSSTable *table = tableOnTop();
    if( !table ) return;
    vector<int> rows;

    table->getSelectedRows(rows);
    if((int)rows.size() <= 0) {
	showWarning("No rows selected.");
	return;
    }
    table->removeRecords(rows);
    change.unknown = true;
    doDataChangeCallbacks();
}

void TableViewer::viEdit(void)
{
    CSSTable *table = tableOnTop();
    if( !table ) return;
    vector<const char *> row;
    string prop;
    FILE *fp;
    int i, j;

    vi_states.clear();
    table->getRowStates(vi_states);

    vi_rows.clear();
    vi_cols.clear();
    table->getSelectedRows(vi_rows);
    table->getSelectedColumns(vi_cols);

    if((int)vi_rows.size() > 0 && (int)vi_cols.size() == 0) {
	int n = table->numColumns();
	for(i = 0; i < n; i++) vi_cols.push_back(i);
    }
    else if((int)vi_cols.size() > 0 && (int)vi_rows.size() == 0) {
	int n = table->numRows();
	for(i = 0; i < n; i++) vi_rows.push_back(i);
	table->selectAllRows(true);
    }
    else if(!(int)vi_rows.size() && !(int)vi_cols.size()) {
	showWarning("No rows or columns selected.");
	return;
    }

    snprintf(vi_path, sizeof(vi_path), cssioGetTmpPrefix("/tmp", "geotl"));

    if( !(fp = fopen(vi_path, "w")) ) {
	showWarning("Cannot open temporary file %s\n%s", vi_path,
			strerror(errno));
	vi_rows.clear();
	vi_cols.clear();
	return;
    }

    for(i = 0; i < (int)vi_rows.size(); i++) {
	table->getRow(vi_rows[i], row);
	for(j = 0; j < (int)vi_cols.size(); j++) {
	    if(j < (int)vi_cols.size()-1) fprintf(fp, "%s | ", row[vi_cols[j]]);
	    else fprintf(fp, "%s\n", row[vi_cols[j]]);
	}
    }
    fclose(fp);

    if( getProperty("vi_edit.command", prop) ) {
	snprintf(vi_cmd, sizeof(vi_cmd), "%s %s", prop.c_str(), vi_path);
    }
    else {
	snprintf(vi_cmd, sizeof(vi_cmd), "xterm -e vi %s", vi_path);
    }

    if( get_sem && sem_init(&vi_sem, 0, 0) ) {
	fprintf(stderr, "sem_init failed: %s\n", strerror(errno));
	thread = (pthread_t)NULL;
	return;
    }
    get_sem = false;

    if(pthread_create(&thread, NULL, doSystem, (void *)this)) {
	fprintf(stderr, "pthread_create failed.\n");
    }
    topWindowParent()->setSensitive(false);
    XtAppAddTimeOut(Application::getAppContext(), 100, checkViEdit,
			(XtPointer)this);
}

static void * doSystem(void *client_data)
{
    TableViewer *tv = (TableViewer *)client_data;
    system(tv->vi_cmd);
    sem_post(&tv->vi_sem);
    return NULL;
}

void TableViewer::checkViEdit(XtPointer data, XtIntervalId *id)
{
    TableViewer *tv = (TableViewer *)data;
    int value;

    sem_getvalue(&tv->vi_sem, &value);
    if(value > 0) { // vi_edit thread has finished
	sem_wait(&tv->vi_sem);
	tv->topWindowParent()->setSensitive(true);
	tv->finishViEdit();
    }
    else {
	XtAppAddTimeOut(Application::getAppContext(), 100, checkViEdit,
			(XtPointer)tv);
    }
}

void TableViewer::finishViEdit(void)
{
    CSSTable *table = tableOnTop();
    if( !table ) return;
    char line[1000];
    char *c, *tok, *last, *s;
    int i, j;
    bool changed;
    FILE *fp;

    gvector<CssTableClass *> *v = table->getRecords();
    if( !v ) return;

    if( !(fp = fopen(vi_path, "r")) ) {
	showWarning("Cannot open temporary file %s\n%s", vi_path,
			strerror(errno));
	vi_rows.clear(); vi_cols.clear();
	return;
    }

    table->editModeOn();

    changed = false;
    i = 0;
    while(i < (int)vi_rows.size() && stringGetLine(fp, line, sizeof(line)) != EOF) {
	j = 0;
	CssTableClass *o = v->at(vi_rows[i]);
	tok = line;
	while(j < (int)vi_cols.size() && (c = strtok_r(tok, "|", &last)) ) {
	    tok = NULL;
	    TAttribute a = table->getAttribute(vi_cols[j]);
	    if( !o->setMember(a.order, stringTrim(c)) ) {
		showWarning("row %d column %d %s", vi_rows[i]+1, vi_cols[j]+1,
				CssTableClass::getError());
	    }
	    else {
		s = table->getField(vi_rows[i], vi_cols[j]);
		if(strcmp(s, c)) {
		    table->setField(vi_rows[i], vi_cols[j], c, false);
		    changed = true;
		}
		Free(s);
	    }
	    j++;
	}
	i++;
    }

    table->adjustColumns();

    unlink(vi_path);

    vi_rows.clear();
    vi_cols.clear();

    if( !changed ) {
	table->editCancel();
	table->setRowStates(vi_states);
	vi_states.clear();
    }
    change.unknown = true;
    doDataChangeCallbacks();
}

bool TableViewer::updateGlobalFile(void)
{
    CSSTable *table = tableOnTop();
    if( !table ) return false;
    char prefix[MAXPATHLEN+1];
    ConnectionHandle c;

    c.connection_type = PREFIX_CONNECTION;
    snprintf(prefix, sizeof(prefix), "%s/tables/static/global",
		Application::getInstallDir("GEOTOOL_HOME"));
    c.prefix = prefix;

    if( !(c.ffdb = FFDatabase::FFDBOpenPrefix(prefix)) ) {
	showWarning(FFDatabase::FFDBErrMsg());
    }

    gvector<CssTableClass *> *v = table->getRecords();

    for(int i = 0; i < (int)unique_names.size(); i++) {
	free(unique_names[i]);
    }
    unique_names.clear();

    for(int i = 0; i < (int)id_names.size(); i++) {
	free(id_names[i]);
    }
    id_names.clear();

    if(v && v->size() > 0) {
	const char *table_name = v->at(0)->getName();
	if(!strcasecmp(table_name, cssSite)) {
	    unique_names.push_back(strdup("sta"));
//	    unique_names.push_back(strdup("staname"));
	    unique_names.push_back(strdup("lat"));
	    unique_names.push_back(strdup("lon"));
	}
	else if(!strcasecmp(table_name, cssSitechan)) {
	    unique_names.push_back(strdup("chanid"));
	}
	else if(!strcasecmp(table_name, cssSensor)) {
//	    unique_names.push_back(strdup("chanid"));
	    id_names.push_back(strdup("inid"));
	}
	else if(!strcasecmp(table_name, cssGregion)) {
	    unique_names.push_back(strdup("grn"));
//	    id_names.push_back(strdup("grn"));
	}
	else if(!strcasecmp(table_name, cssInstrument)) {
	    id_names.push_back(strdup("inid"));
	}
    }

    setCursor("hourglass");
    if(v) {
	for(int i = 0; i < v->size(); i++) {
	    if(!insertTable(&c, v->at(i), table->getType())) {
		delete c.ffdb;
		return false;
	    }
	}
    }
    setCursor("default");
    delete c.ffdb;
    return true;
}

bool TableViewer::insertTable(ConnectionHandle *c, CssTableClass *t,
				const string &name)
{
    char *lastid_table = strdup(cssLastid); // get this from Import.cpp

    for(int i = 0; i < (int)id_names.size(); i++)
    {
	long *id = (long *)t->memberAddress(id_names[i]);
	int type = t->memberType(id_names[i]);

	if(!id) continue;

	if(type != CSS_LONG) {
	    showWarning("insertTable: invalid data type for id: %s, table: %s",
			id_names[i], t->getName());
	    free(lastid_table);
	    return false;
	}
	// Check for valid id.
	if( *id < 0 ) {
	    *id = getId(c, lastid_table, id_names[i]);
	}
    }
    free(lastid_table);

    return writeTable(c, t, name);
}

bool TableViewer::writeTable(ConnectionHandle *c, CssTableClass *t,
				const string &tableName)
{
    const char *cssTableName;
    int i, n, m, err = -1;
    bool ret = false;
    CssClassDescription *des = t->description();

    cssTableName = t->getName();

    if((int)unique_names.size() > 0)
    {
	gvector<CssTableClass *> v;
	char query_string[5000], format[100];
	snprintf(query_string, sizeof(query_string), "select * from %s where ",
		tableName.c_str());
	m = sizeof(query_string);
	for(i = 0; i < (int)unique_names.size(); i++)
	{
	    int k = t->memberIndex(unique_names[i]);

	    if(k < 0) break;

	    if(i == 0) {
		snprintf(format, sizeof(format), "%%s=%s", des[k].format);
	    }
	    else {
		snprintf(format, sizeof(format), " and %%s=%s", des[k].format);
	    }

	    n = strlen(query_string);

	    if(des[k].type == CSS_STRING) {
		char *a = (char *)t->memberAddress(unique_names[i]);
		if(i == 0) {
		    snprintf(query_string+n, m-n-1, "%s='%s'", unique_names[i], a);
		}
		else {
		    snprintf(query_string+n, m-n-1, " and %s='%s'", unique_names[i],a);
		}
	    }
	    else if(des[k].type == CSS_INT) {
		int a = *(int *)t->memberAddress(unique_names[i]);
		snprintf(query_string+n, m-n-1, format, unique_names[i], a);
	    }
	    else if(des[k].type == CSS_LONG || des[k].type == CSS_JDATE) {
		long a = *(long *)t->memberAddress(unique_names[i]);
		snprintf(query_string+n, m-n-1, format, unique_names[i], a);
	    }
	    else if(des[k].type == CSS_FLOAT) {
		float a = *(float *)t->memberAddress(unique_names[i]);
		snprintf(query_string+n, m-n-1, format, unique_names[i], a);
	    }
	    else if(des[k].type == CSS_DOUBLE || des[k].type == CSS_TIME) {
		double a = *(double *)t->memberAddress(unique_names[i]);
		snprintf(query_string+n, m-n-1, format, unique_names[i], a);
	    }
	    else if(des[k].type == CSS_DATE || des[k].type == CSS_LDDATE) {
		DateTime *dt = (DateTime *)t->memberAddress(unique_names[i]);
		double a = timeDateToEpoch(dt);
		snprintf(query_string+n, m-n-1, "%s=%.0lf", unique_names[i], a);
	    }
	}
	if(c->connection_type == FFDB_CONNECTION) {
	    if((err = !c->ffdb->queryTable(query_string, cssTableName, &v))) {
		showWarning(FFDatabase::FFDBErrMsg());
	    }
	}
#ifdef HAVE_LIBODBC
	else if(c->connection_type == ODBC_CONNECTION) {
	    if((err = ODBCQueryTable(c->hdbc, query_string, cssTableName, v))) {
		showWarning(ODBCErrMsg());
	    }
	}
#endif /* HAVE_LIBODBC */
	else if(c->connection_type == PREFIX_CONNECTION) {
	    if((err = !c->ffdb->queryPrefix(query_string, cssTableName, &v))) {
		showWarning(FFDatabase::FFDBErrMsg());
	    }
	}
	if(err) {
	    return false;
	}
	if(!ondate.empty() && !offdate.empty() && t->memberIndex(ondate) >= 0
		&& t->memberIndex(offdate) >= 0
		&& t->memberType(ondate) == t->memberType(offdate))
	{
	    double d_on, d_off;
	    if(!getOnOff(t, ondate, offdate, &d_on, &d_off)) {
		logErrorMsg(LOG_ERR, "Bad 'on' member type");
		return false;
	    }
	    for(i = v.size()-1; i >= 0; i--) {
		double e_on, e_off;
		if(getOnOff(v[i], ondate, offdate, &e_on, &e_off) &&
			!periodsOverlap(d_on, d_off, e_on, e_off))
		{
		    v.removeAt(i);
		}
	    }
	}

	if(v.size() == 1) {
	    updateTable(c, tableName, v[0], t);
	    return true;
	}
	else if(v.size() > 1) {
	    showWarning("%d %s records found with query=%s",
			v.size(), tableName.c_str(), query_string);
	    return true;
	}
    }

    if(c->connection_type == FFDB_CONNECTION)
    {
	const char *author = c->ffdb->defaultAuthor();
	ret = c->ffdb->insertTable(t, author) ? false : true;
	TableListener::doCallbacks(t, this, "add");
    }
#ifdef HAVE_LIBODBC
    else if(c->connection_type == ODBC_CONNECTION)
    {
	if( Application::writeToDB() ) {
	    ret = ODBCInsertTable(c->hdbc, tableName, t) ? true : false;
	    ODBCDisconnect(c->hdbc);
	    if( !(c->hdbc=ODBCConnect(c->data_source,c->user,c->data_passwd,0)))
	    {
		showWarning("%s", ODBCErrMsg());
		return false;
	    }
	}
	else {
	    ret = true;
	}
	TableListener::doCallbacks(t, this, "add");
    }
#endif /* HAVE_LIBODBC */
    else if(c->connection_type == PREFIX_CONNECTION)
    {
	ret = c->ffdb->insertPrefixTable(t) ? false : true;
	TableListener::doCallbacks(t, this, "add");
    }

    return ret;
}

bool TableViewer::updateTable(ConnectionHandle *c, const string &css_table_name,
		CssTableClass *told, CssTableClass *tnew)
{
    if(c->connection_type == FFDB_CONNECTION ||
	c->connection_type == PREFIX_CONNECTION)
    {
	tnew->copyTo(told, false);
	if(!c->ffdb->update(told)) {
	    showWarning(FFDatabase::FFDBErrMsg());
	    return false;
	}
	TableListener::doCallbacks(told, tnew, this);
    }
#ifdef HAVE_LIBODBC
    else if(c->connection_type == ODBC_CONNECTION)
    {
	if( Application::writeToDB() ) {
	    if(ODBCUpdateTable(c->hdbc, css_table_name, told, tnew)) {
		showWarning(ODBCErrMsg());
		return false;
	    }
	}
	TableListener::doCallbacks(told, tnew, this);
    }
#endif /* HAVE_LIBODBC */
    return true;
}

static bool
periodsOverlap(double d_on, double d_off, double e_on, double e_off)
{
    if(d_on == e_on && d_off == e_off) return true;
    if(d_off < 0 && e_off < 0) return true;
    if(d_off < 0 || e_off < 0) return false;
    if(d_off > e_on && d_off <= e_off) return true;
    if(d_on >= e_on && d_on < e_off) return true;
    if(e_off > d_on && e_off <= d_off) return true;
    if(e_on >= d_on && e_on < d_off) return true;
    return false;
}

static bool
getOnOff(CssTableClass *t, const string &on, const string &off, double *d_on,
		double *d_off)
{
    int type = t->memberType(on);

    if(type == CSS_DOUBLE || type == CSS_TIME) {
	*d_on = *(double *)t->memberAddress(on);
	*d_off = *(double *)t->memberAddress(off);
    }
    else if(type == CSS_FLOAT) {
	*d_on = *(float *)t->memberAddress(on);
	*d_off = *(float *)t->memberAddress(off);
    }
    else if(type == CSS_LONG || type == CSS_JDATE) {
	*d_on = *(long *)t->memberAddress(on);
	*d_off = *(long *)t->memberAddress(off);
    }
    else if(type == CSS_INT) {
	*d_on = *(int *)t->memberAddress(on);
	*d_off = *(int *)t->memberAddress(off);
    }
    else if(type == CSS_DATE) {
	DateTime *dt;
	dt = (DateTime *)t->memberAddress(on);
	*d_on = timeDateToEpoch(dt);
	dt = (DateTime *)t->memberAddress(off);
	*d_off = timeDateToEpoch(dt);
    }
    else {
	return false;
    }
    return true;
}

long TableViewer::getId(ConnectionHandle *c, const string &lastid_table,
			const string &id_name)
{
    long id = -1;

    if(c->connection_type == FFDB_CONNECTION)
    {
	id = c->ffdb->getNextId(lastid_table, id_name);
    }
    else if(c->connection_type == ODBC_CONNECTION)
    {
#ifdef HAVE_LIBODBC
	if( Application::writeToDB() ) {
	    id = ODBCGetNextId(c->hdbc, lastid_table, id_name);
	}
	else {
	    if(!strcasecmp(id_name.c_str(), "arid")) {
		id = last_arid;
		last_arid--;
	    }
	    else if(!strcasecmp(id_name.c_str(), "orid")) {
		id = last_orid;
		last_orid--;
	    }
	    else if(!strcasecmp(id_name.c_str(), "ampid")) {
		id = last_ampid;
		last_ampid--;
	    }
	}
#endif
    }
    else if(c->connection_type == PREFIX_CONNECTION)
    {
	id = prefixGetNextId(c->prefix, lastid_table, id_name);
    }
    return id;
}

long TableViewer::prefixGetNextId(const string &prefix,
			const string &lastid_table, const string &id_name)
{
    char file[MAXPATHLEN+1];
    const char *err_msg=NULL;
    int  rec, err, keyvalue;
    CssLastidClass lastid;
    struct stat file_buf;
    FILE *fp;

    snprintf(file, sizeof(file), "%s.%s", prefix.c_str(), lastid_table.c_str());

    if(stat(file, &file_buf) == 0) {
	fp = fopen(file, "r+");
    }
    else {
	snprintf(file, sizeof(file), "%s/tables/dynamic/global.%s",
	    Application::getInstallDir("GEOTOOL_HOME"), lastid_table.c_str());

	if( !stat(file, &file_buf) ) {
	    fp = fopen(file, "r+");
	}
	else {
	    fp = fopen(file, "w+");
	}
    }
    if(fp == NULL)
    {
	if(errno > 0) {
	    showWarning("Cannot open %s\n%s", file, strerror(errno));
	}
	else {
	    showWarning("Cannot open %s", file);
	}
	return -1;
    }

    fseek(fp, 0, 0);

    rec = 0;
    while(!(err = lastid.read(fp, &err_msg)))
    {
	if(!id_name.compare(lastid.keyname)) break;
	rec++;
    }

    if(err == 0)
    {
	// found id_name
	fseek(fp, rec*(lastid.getLineLength()+1), 0);
    }
    else if(err == EOF)
    {
	stringcpy(lastid.keyname, id_name.c_str(), sizeof(lastid.keyname));
	lastid.keyvalue = 0;
    }
    else
    {
	if(err_msg) {
	    showWarning("format error in%s\n%s", file, err_msg);
	}
	else {
	    showWarning("format error in%s", file);
	}
	fclose(fp);
	return(-1);
    }
    lastid.keyvalue++;

    timeEpochToDate(timeGetEpoch(), &lastid.lddate);
    if(lastid.write(fp, &err_msg)) {
	if(err_msg) {
	    showWarning("write error: %s/%s", file, err_msg);
	}
	else {
	    showWarning("write error: %s", file);
	}
    }
    fclose(fp);
    keyvalue = lastid.keyvalue;

    return(keyvalue);
}

void TableViewer::tabMenu(TabMenuCallbackStruct *c)
{
    if(tab_popup && tab_popup->isVisible()) return;

    if(tab_popup == NULL) {
        tab_popup = createTabPopup();
    }
    if(c->tab_label) tab_menu_label.assign(c->tab_label);

    char **tabs = NULL;
    int i, j, ntabs = tab->getLabels(&tabs);

    for(i = 0; i < num_tab_menu_buttons; i++) {
	for(j = 0; j < ntabs && strcmp(tabs[j],tab_menu_buttons[i]->getName());
		j++);
	bool visible = (j == ntabs) ? true : false;
	tab_menu_buttons[i]->setVisible(visible);
    }
		
    tab_popup->position((XButtonPressedEvent *)c->event);
    tab_popup->setVisible(true);
}

PopupMenu *TableViewer::createTabPopup(void)
{
    char s[100];
    Arg args[20];
    int i, j, n;
    PopupMenu *tabpopup;
    const char **cssTableNames=NULL;
    int num = CssTableClass::getAllNames(&cssTableNames);
    XtTranslations translations;
    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

    /* the default is button3, which will inactivate button3
     * in the AxesWidget, so set to 6 > all buttons
     */
    n = 0;
#if(XmVersion >= 2)
#ifndef XmNtearOffTitle
#define XmNtearOffTitle "tearOffTitle"
#endif
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
#endif
    XtSetArg(args[n], XmNwhichButton, 6); n++;
    XtSetArg(args[n], XmNradioBehavior, False); n++;
    tabpopup = new PopupMenu("tabPopup", this, args, n);

    new Label("Add Tab", tabpopup);
    new Separator("sep", tabpopup);

    translations = XtParseTranslationTable(trans);
    n = 0;
    XtSetArg(args[n], XmNtranslations, translations); n++;

    tab_menu_buttons = (Button **)malloc(num*sizeof(Button *));

    for(i = j = 0; i < num; i++)
	if(strncmp(cssTableNames[i], "gards_", 6) &&
	    strncmp(cssTableNames[i], "dynamic", 7))
    {
	tab_menu_buttons[j] = new Button(cssTableNames[i], tabpopup, args,
				n, this);
	snprintf(s, sizeof(s), "popup-menu %s", cssTableNames[i]);
	tab_menu_buttons[j]->setCommandString(s);
	j++;
    }
    tab_menu_buttons[j] = new Button("dynamic", tabpopup, args, n, this);
    snprintf(s, sizeof(s), "popup-menu %s", "dynamic");
    tab_menu_buttons[j]->setCommandString(s);
    j++;
    num_tab_menu_buttons = j;
    return tabpopup;
}
