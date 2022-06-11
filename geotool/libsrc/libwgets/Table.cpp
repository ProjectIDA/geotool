/** \file Table.cpp
 *  \brief Defines class Table.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <errno.h>
using namespace std;

#include "widget/Table.h"
#include "widget/TableAttributes.h"
#include "motif++/Application.h"
#include "motif++/MotifClasses.h"
#include <X11/Xmu/Xmu.h>

extern "C" {
#include "libstring.h"
#include "libtime.h"
}
static bool stringArgPlus(const string &name, char *row_name, string &col,
			string &fmt);

static int num_tables = 0;
static Table **tables = NULL;

Table::Table(const string &name, Component *parent, Arg *args, int n) :
		Component(name, parent)
{
    init(name, parent, NULL, args, n);
}

Table::Table(const string &name, Component *parent, InfoArea *infoarea,
		Arg *args, int n) : Component(name, parent)
{
    init(name, parent, infoarea, args, n);
}

void Table::init(const string &name, Component *parent, InfoArea *infoarea,
                Arg *args, int n)
{
    Arg a[100];
    int num;

    info_area = infoarea;

    // pass only attachment args to the form
    num = 0;
    for(int i = 0; i < n && num < 100; i++) {
	if(!strcasecmp(args[i].name, XmNtopAttachment) ||
	   !strcasecmp(args[i].name, XmNtopOffset) ||
	   !strcasecmp(args[i].name, XmNtopWidget) ||
	   !strcasecmp(args[i].name, XmNbottomAttachment) ||
	   !strcasecmp(args[i].name, XmNbottomOffset) ||
	   !strcasecmp(args[i].name, XmNbottomWidget) ||
	   !strcasecmp(args[i].name, XmNleftAttachment) ||
	   !strcasecmp(args[i].name, XmNleftOffset) ||
	   !strcasecmp(args[i].name, XmNleftWidget) ||
	   !strcasecmp(args[i].name, XmNrightAttachment) ||
	   !strcasecmp(args[i].name, XmNrightOffset) ||
	   !strcasecmp(args[i].name, XmNrightWidget))
	{
	    a[num++] = args[i];
	}
    }
    base_widget = XtCreateManagedWidget(getName(), xmFormWidgetClass,
				parent->baseWidget(), a, num);
    installDestroyHandler();

    // create an unmanaged rowColumn with the edit control buttons
    num = 0;
    XtSetArg(a[num], XmNorientation, XmHORIZONTAL); num++;
    XtSetArg(a[num], XmNleftAttachment, XmATTACH_FORM); num++;
    XtSetArg(a[num], XmNrightAttachment, XmATTACH_FORM); num++;
    XtSetArg(a[num], XmNbottomAttachment, XmATTACH_FORM); num++;
    controls = new RowColumn("controls", this, a, num, false);
    cancel_button = new Button("Cancel", controls, this);
    save_button = new Button("Save", controls, this);

    // pass all other args to the Table
    num = 0;
    for(int i = 0; i < n && num < 100; i++) {
	if(strcasecmp(args[i].name, XmNtopAttachment) &&
	   strcasecmp(args[i].name, XmNtopOffset) &&
	   strcasecmp(args[i].name, XmNtopWidget) &&
	   strcasecmp(args[i].name, XmNbottomAttachment) &&
	   strcasecmp(args[i].name, XmNbottomOffset) &&
	   strcasecmp(args[i].name, XmNbottomWidget) &&
	   strcasecmp(args[i].name, XmNleftAttachment) &&
	   strcasecmp(args[i].name, XmNleftOffset) &&
	   strcasecmp(args[i].name, XmNleftWidget) &&
	   strcasecmp(args[i].name, XmNrightAttachment) &&
	   strcasecmp(args[i].name, XmNrightOffset) &&
	   strcasecmp(args[i].name, XmNrightWidget))
	{
	    a[num++] = args[i];
	}
    }
    if(info_area) {
	XtSetArg(a[num], XtNtableInfoWidget, info_area->leftInfo()); num++;
	XtSetArg(a[num], XtNcursorInfoWidget, info_area->rightInfo()); num++;
    }
    // add table attachment args (do not attach to controls for now)
    if(num < 96) {
	XtSetArg(a[num], XmNtopAttachment, XmATTACH_FORM); num++;
	XtSetArg(a[num], XmNbottomAttachment, XmATTACH_FORM); num++;
	XtSetArg(a[num], XmNleftAttachment, XmATTACH_FORM); num++;
	XtSetArg(a[num], XmNrightAttachment, XmATTACH_FORM); num++;
    }

    tw = (MmTableWidget)MmTableCreate(base_widget, (char *)getName(), a, num);
    MmTableSetClass(tw, this);
    XtManageChild((Widget)tw);

    XtAddCallback((Widget)tw, XtNselectRowCallback,
		Table::selectRowCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNselectColumnCallback,
		Table::selectColumnCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNcolumnMovedCallback,
		Table::columnMovedCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNeditModeCallback,
		Table::editModeCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNmodifyVerifyCallback,
		Table::modifyVerifyCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNvalueChangedCallback,
		Table::valueChangedCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNchoiceChangedCallback,
		Table::choiceChangedCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNcellEnterCallback,
		Table::cellEnterCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNcellSelectCallback,
		Table::cellSelectCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNleaveWindowCallback,
		Table::leaveWindowCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNrowChangeCallback,
		Table::rowChangeCallback, (XtPointer)this);
    XtAddCallback((Widget)tw, XtNcopyPasteCallback,
		Table::copyPasteCallback, (XtPointer)this);

    enableCallbackType(XtNselectRowCallback);
    enableCallbackType(XtNselectColumnCallback);
    enableCallbackType(XtNcolumnMovedCallback);
    enableCallbackType(XtNeditModeCallback);
    enableCallbackType(XtNmodifyVerifyCallback);
    enableCallbackType(XtNvalueChangedCallback);
    enableCallbackType(XtNchoiceChangedCallback);
    enableCallbackType(XtNcellEnterCallback);
    enableCallbackType(XtNcellSelectCallback);
    enableCallbackType(XtNleaveWindowCallback);
    enableCallbackType(XtNrowChangeCallback);
    enableCallbackType(XtNeditSaveCallback);
    enableCallbackType(XtNeditCancelCallback);
    enableCallbackType(XtNattributeChangeCallback);

/* This does not work with some X-servers due to problems in MmTable.c
    table_menu = MmTableMenuChild(tw);
    w = XtVaCreateManagedWidget("Attributes...", xmPushButtonWidgetClass,
		table_menu, NULL);
    XtAddCallback(w, XmNactivateCallback, Table::attributesCallback,
		(XtPointer)this);

    w = XtVaCreateManagedWidget("Edit", xmPushButtonWidgetClass,
		table_menu, NULL);
    XtAddCallback(w, XmNactivateCallback, Table::editCallback, (XtPointer)this);

    w = XtVaCreateManagedWidget("Select All", xmPushButtonWidgetClass,
		table_menu, NULL);
    XtAddCallback(w, XmNactivateCallback, Table::selectAllCallback,
		(XtPointer)this);

    w = XtVaCreateManagedWidget("Deselect All", xmPushButtonWidgetClass,
		table_menu, NULL);
    XtAddCallback(w, XmNactivateCallback, Table::deselectAllCallback,
		(XtPointer)this);
*/

/*
    new Button("Attributes...", table_menu, this);
    new Button("Edit", table_menu, this);
    new Button("Select All", table_menu, this);
    new Button("Deselect All", table_menu, this);
*/
/*
    Arg a[1];
    XtSetArg(a[0], XtNdisplayMenuButton, true);
    XtSetValues(base_widget, a, 1);
*/
    table_attributes = NULL;
    edit_mode = false;
    getDisplayAttributes(display_attributes);

    setAppContext(Application::getApplication()->appContext());

    // save all tables created
    if( !tables ) {
	tables = (Table **)malloc(sizeof(Table *));
    }
    else {
	tables = (Table **)realloc(tables, (num_tables+1)*sizeof(Table *));
    }
    tables[num_tables++] = this;
    selection_type = TABLE_COPY_ROWS;
    column_index = 0;
    table_row_atom = XInternAtom(XtDisplay(base_widget), "TABLE_ROW", False);
    table_column_atom = XInternAtom(XtDisplay(base_widget), "TABLE_COLUMN",
				False);
    requested = table_row_atom;
    copy_time = CurrentTime;
    copy_value = NULL;
    copy_length = 0;
    copy_string_value = NULL;
    copy_string_length = 0;

    display_edit_controls = true;

    memset(row_name, 0, sizeof(row_name));
    memset(selected_row_name, 0, sizeof(selected_row_name));
    parse_row_index = 0;
    selected_row_index = 0;

    memset(column_name, 0, sizeof(column_name));
    memset(selected_column_name, 0, sizeof(selected_column_name));
    parse_column_index = 0;
    selected_column_index = 0;
}

void Table::destroy(void)
{
    if(base_widget)
    {
	XtRemoveAllCallbacks((Widget)tw, XtNselectRowCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNselectColumnCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNcolumnMovedCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNeditModeCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNmodifyVerifyCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNvalueChangedCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNchoiceChangedCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNcellEnterCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNcellSelectCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNleaveWindowCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNrowChangeCallback);
	XtRemoveAllCallbacks((Widget)tw, XtNcopyPasteCallback);
    }
    Component::destroy();
}

Table::~Table(void)
{
    Free(copy_value);
    Free(copy_string_value);

    for(int i = 0; i < num_tables; i++) {
	if(tables[i] == this) {
	    for(int j = i; j < num_tables-1; j++) tables[j] = tables[j+1];
	    num_tables--;
	    break;
	}
    }
}

// static
Table * Table::getTableFromWidget(Widget w)
{
    for(int i = 0; i < num_tables; i++) {
	if(tables[i]->baseWidget() == w) return tables[i];
    }
    return NULL;
}

void Table::setValues(Arg *args, int n, bool set)
{
    Arg a[100];
    int num;

    // pass only attachment args to the form
    num = 0;
    for(int i = 0; i < n && num < 100; i++) {
	if(!strcasecmp(args[i].name, XmNtopAttachment) ||
	   !strcasecmp(args[i].name, XmNtopOffset) ||
	   !strcasecmp(args[i].name, XmNtopWidget) ||
	   !strcasecmp(args[i].name, XmNbottomAttachment) ||
	   !strcasecmp(args[i].name, XmNbottomOffset) ||
	   !strcasecmp(args[i].name, XmNbottomWidget) ||
	   !strcasecmp(args[i].name, XmNleftAttachment) ||
	   !strcasecmp(args[i].name, XmNleftOffset) ||
	   !strcasecmp(args[i].name, XmNleftWidget) ||
	   !strcasecmp(args[i].name, XmNrightAttachment) ||
	   !strcasecmp(args[i].name, XmNrightOffset) ||
	   !strcasecmp(args[i].name, XmNrightWidget))
	{
	    a[num++] = args[i];
	}
    }
    if(num) {
	if(set) {
	    XtSetValues(base_widget, a, num);
	}
	else {
	    XtGetValues(base_widget, a, num);
	}
    }
    // pass all other args to the Table
    num = 0;
    for(int i = 0; i < n && num < 100; i++) {
	if(strcasecmp(args[i].name, XmNtopAttachment) &&
	   strcasecmp(args[i].name, XmNtopOffset) &&
	   strcasecmp(args[i].name, XmNtopWidget) &&
	   strcasecmp(args[i].name, XmNbottomAttachment) &&
	   strcasecmp(args[i].name, XmNbottomOffset) &&
	   strcasecmp(args[i].name, XmNbottomWidget) &&
	   strcasecmp(args[i].name, XmNleftAttachment) &&
	   strcasecmp(args[i].name, XmNleftOffset) &&
	   strcasecmp(args[i].name, XmNleftWidget) &&
	   strcasecmp(args[i].name, XmNrightAttachment) &&
	   strcasecmp(args[i].name, XmNrightOffset) &&
	   strcasecmp(args[i].name, XmNrightWidget))
	{
	    a[num++] = args[i];
	}
    }
    if(num) {
	if(set) {
	    XtSetValues((Widget)tw, a, num);
	}
	else {
	    XtGetValues((Widget)tw, a, num);
	}
    }
}

void Table::attributesCallback(Widget w, XtPointer clientData, XtPointer data)
{
    Table *table = (Table *)clientData;
    table->showAttributes(true);
}

void Table::editCallback(Widget w, XtPointer clientData, XtPointer data)
{
    Table *table = (Table *)clientData;
    table->editModeOn();
}

void Table::selectAllCallback(Widget w, XtPointer clientData, XtPointer data)
{
    Table *table = (Table *)clientData;
    table->selectAllRows(true);
}

void Table::deselectAllCallback(Widget w, XtPointer clientData, XtPointer data)
{
    Table *table = (Table *)clientData;
    table->selectAllRows(false);
}

void Table::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Attributes...")) {
	showAttributes(true);
    }
    else if(!strcmp(cmd, "Edit")) {
	editModeOn();
    }
    else if(!strcmp(cmd, "Select All")) {
        selectAllRows(true);
    }
    else if(!strcmp(cmd, "Deselect All")) {
        selectAllRows(false);
    }
    else if(!strcmp(cmd, "Cancel")) {
	editCancel();
	doCallbacks((Widget)tw, NULL, XtNeditCancelCallback);
    }
    else if(!strcmp(cmd, "Save")) {
	editSave();
	doCallbacks((Widget)tw, NULL, XtNeditSaveCallback);
	backup();
    }
    else if(!strcmp(cmd, "Apply Attributes")) {
	setColumns();
	doCallbacks((Widget)tw, NULL, XtNattributeChangeCallback);
	saveAttributes();
    }
}

ParseCmd Table::parseCmd(const string &cmd, string &msg)
{
    string c;
    vector<int> cols;

    if(parseArg(cmd, "select_row", c)) {
	return parseSelectRow(c, msg, true);
    }
    else if(parseArg(cmd, "deselect_row", c)) {
	return parseSelectRow(c, msg, false);
    }
    else if(parseArg(cmd, "select_column", c)) {
	return parseSelectColumns(c, msg, true);
    }
    else if(parseArg(cmd, "deselect_column", c)) {
	return parseSelectColumns(c, msg, false);
    }
    else if(parseCompare(cmd, "deselect_all")) {
	selectAllRowsWithCB(false);
    }
    else if(parseCompare(cmd, "select_all")) {
	selectAllRowsWithCB(true);
    }
    else if(parseCompare(cmd, "promote_rows")) {
	promoteSelectedRows();
    }
    else if(parseCompare(cmd, "promote_columns")) {
	promoteSelectedColumns();
    }
    else if(parseCompare(cmd, "sort") || parseCompare(cmd, "sort_up")) {
	getSelectedColumns(cols);
	sortByColumns(cols);
    }
    else if(parseCompare(cmd, "sort_down")) {
	getSelectedColumns(cols);
	sortByColumns(cols);
	reverseOrder();
    }
    else if(parseCompare(cmd, "sort ", 5)) {
	return parseSort(cmd.substr(5), "sort", true, false, msg);
    }
    else if(parseCompare(cmd, "sort_up ", 8)) {
	return parseSort(cmd.substr(8), "sort_up", true, false, msg);
    }
    else if(parseCompare(cmd, "sort_down ", 10)) {
	return parseSort(cmd.substr(10), "sort_down", false, false, msg);
    }
    else if(parseCompare(cmd, "sort_selected")) {
	getSelectedColumns(cols);
	sortSelected(cols);
    }
    else if(parseCompare(cmd, "sort_selected ", 14)) {
	return parseSort(cmd.substr(5), "sort", true, true, msg);
    }
    else if(parseCompare(cmd, "sort_selected_up ", 17)) {
	return parseSort(cmd.substr(8), "sort_up", true, true, msg);
    }
    else if(parseCompare(cmd, "sort_selected_down ", 19)) {
	return parseSort(cmd.substr(10), "sort_down", false, true, msg);
    }
    else if(parseCompare(cmd, "sort_unique")) {
	getSelectedColumns(cols);
	sortUnique(cols);
    }
    else if(parseCompare(cmd, "remove_all_rows")) {
	removeAllRows();
    }
    else if(parseCompare(cmd, "expand")) {
	getSelectedColumns(cols);
	if((int)cols.size() > 0) expand(cols[0]);
    }
    else if(parseCompare(cmd, "reverse_order")) {
	reverseOrder();
    }
    else if(parseCompare(cmd, "show_all")) {
	showAll();
    }
    else if(parseArg(cmd, "set_cell", c)) {
	return parseSetCell(c, msg);
    }
    else if(parseArg(cmd, "set_selected_cell", c)) {
	printf("not implemented\n");
//	return parseSetSelectedCell(c, msg);
    }
    else if(parseString(cmd, "save_all_rows", c)) {
	string file;
	if( !parseGetArg(c, "file", file) ) {
	    msg.assign("save_all_rows: missing file argument.");
	}
	bool b = false;
	parseGetArg(c, "save_all_rows", msg, "append", &b);
	if(!msg.empty()) return ARGUMENT_ERROR;
	const char *access = b ? "a" : "w";
	return saveTable(file,access,0,msg) ? COMMAND_PARSED : ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "save_selected_rows", c)) {
	string file;
	if( !parseGetArg(c, "file", file) ) {
	    msg.assign("save_selected_rows: missing file argument.");
	}
	bool b = false;
	parseGetArg(c, "save_selected_rows", msg, "append", &b);
	if(!msg.empty()) return ARGUMENT_ERROR;
	const char *access = b ? "a" : "w";
	return saveTable(file,access,1,msg) ? COMMAND_PARSED : ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "save_selected_columns", c)) {
	string file;
	if( !parseGetArg(c, "file", file) ) {
	    msg.assign("save_selected_columns: missing file argument.");
	}
	bool b = false;
	parseGetArg(c, "save_selected_rows", msg, "append", &b);
	if(!msg.empty()) return ARGUMENT_ERROR;
	const char *access = b ? "a" : "w";
	return saveTable(file,access,2,msg) ? COMMAND_PARSED : ARGUMENT_ERROR;
    }
    else if(parseCompare(cmd, "copy_selected_rows")) {
	copy(TABLE_COPY_ROWS, CurrentTime);
    }
    else if(parseCompare(cmd, "copy_all")) {
	copy(TABLE_COPY_ALL, CurrentTime);
    }
    else if(parseCompare(cmd, "copy_selected_columns")) {
	copy(TABLE_COPY_COLUMNS, CurrentTime);
    }
    else if(parseCompare(cmd, "paste")) {
	paste((Time)CurrentTime);
    }
    else if(parseArg(cmd, "add_row", c)) {
	int i, ncols = numColumns();
	char *s, *tok, *last, *p;
	vector<char *> row;
	p = strdup(c.c_str());
	s = p;
	while(*s != '\0') {
	    if(*s == '"' || *s == '\'' || *s == '`') {
		char a = *s++;
		while(*s != '\0' && *s != a) {
		    // temporarily change commas inside of quotes
		    if(*s == ',') *s = '\f';
		    s++;
		}
	    }
	    s++;
	}
	i = 0;
	tok = p;
	while(i < ncols && (s = strtok_r(tok, ",", &last))) {
	    tok = NULL;
	    for(int j = 0; s[j] != '\0'; j++) if(s[j] == '\f') s[j] = ',';
	    stringTrim(s);
	    row.push_back(s);
	    i++;
	}
	addRow(row, true);
	free(p);
	adjustColumns();
    }
    else if(parseCompare(cmd, "print_selected_rows")) {
	printSelectedRows(stdout);
    }
    else if(parseCompare(cmd, "print_selected_columns")) {
	printSelectedColumns(stdout);
    }
    else if(parseCompare(cmd, "print_all")) {
	printAll(stdout);
    }
    else if(parseCompare(cmd, "help")) {
	parseHelp("table.");
    }
    else {
	return Component::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd Table::parseSort(const string &c, const string &cmd, bool up,
			bool sort_selected_rows, string &msg)
{
    string s;
    char *tok, *col, *last, *p;

    if( !parseGetArg(c, "column", s) ) {
	msg.assign(cmd + ": missing column argument.");
	return ARGUMENT_ERROR;
    }
    vector<const char *> labels;
    int i, ncol=0;
    vector<int> colindex;

    getColumnLabels(labels);

    p = strdup(s.c_str());
    tok = p;
    while( ncol < (int)labels.size() && (col = strtok_r(tok, ",", &last)) ) {
	tok = NULL;
	stringTrim(col);
	for(i = 0; i < (int)labels.size() && strcasecmp(labels[i], col); i++);
	if(i == numColumns()) {
	    msg.assign(cmd + ": invalid column name: " + col);
	    return ARGUMENT_ERROR;
	}
	colindex.push_back(i);
	ncol++;
    }
    free(p);

    if(ncol == 0) {
	msg.assign(cmd + ": invalid column argument");
	return ARGUMENT_ERROR;
    }

    if(sort_selected_rows) {
	sortSelected(colindex);
    }
    else {
	sortByColumns(colindex);
    }

    if(!up) reverseOrder(); // ! need a reverseOrder for only selected rows
    return COMMAND_PARSED;
}

ParseVar Table::parseVar(const string &name, string &value)
{
    ParseVar ret;
    string col, fmt;
    char s[1000];
    int i, j;
    vector<int> order;

    if(parseCompare(name, "num_rows")) {
	parsePrintInt(value, numRows());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "num_sel_rows")) {
	parsePrintInt(value, numSelectedRows());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "num_columns")) {
	parsePrintInt(value, numColumns());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "num_sel_columns")) {
	parsePrintInt(value, numSelectedColumns());
	return STRING_RETURNED;
    }
    else if((ret = getRowRequest(name, value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if((ret=getColumnRequest(name, value)) != VARIABLE_NOT_FOUND){
	return ret;
    }
    else if(row_name[0] != '\0' && stringArgPlus(name, row_name, col, fmt))
    {
	vector<const char *> labels, row;

	getRowOrder(order);
	getColumnLabels(labels);
	j = order[parse_row_index];
	for(i = 0; i < (int)labels.size(); i++) {
	    if(sameName(labels[i], col)) {
		getRow(j, row);
		snprintf(s, sizeof(s), fmt.c_str(), row[i]);
		value.assign(s);
		return STRING_RETURNED;
	    }
	}
	if(parseCompare(col, "index")) {
	    parsePrintInt(value, parse_row_index+1);
	    return STRING_RETURNED;
	}
	value.assign(string("column ") + col + " not found.");
	return VARIABLE_ERROR;
    }
    else if(selected_row_name[0] != '\0' &&
		stringArgPlus(name, selected_row_name, col, fmt))
    {
	vector<const char *> labels, row;

	getColumnLabels(labels);
	for(i = 0; i < (int)labels.size(); i++) {
	    if(sameName(labels[i], col, (int)col.length())) {
		getRow(selected_rows[selected_row_index], row);
		snprintf(s, sizeof(s), fmt.c_str(), row[i]);
		value.assign(s);
		return STRING_RETURNED;
	    }
	}
	if(parseCompare(col, "index")) {
	    parsePrintInt(value, selected_rows[selected_row_index]+1);
	    return STRING_RETURNED;
	}
	value.assign(string("column ") + col + " not found.");
	return VARIABLE_ERROR;
    }
    else if(parseCompare(name, "row[", 4)) {
	return getRowWithIndex(name.c_str(), value);
    }
    else if(parseCompare(name, "sel_row[", 8)) {
	return getRowWithIndex(name.c_str(), value);
    }
    else if(parseCompare(name, "column[", 7)) {
	return getColumnWithIndex(name.c_str(), value);
    }
    else if(parseCompare(name, "cell[", 5)) {
	return getCellWithIndex(name.c_str(), value);
    }
    else if(parseCompare(name, "find_indices_", 13)) {
	return parseFindIndices(name, value);
    }
    return Component::parseVar(name, value);
}

ParseVar Table::parseFindIndices(const string &name, string &value)
{
    int i, j, n, num_col, nrows;
    string column_names[100], values[100];
    ostringstream os;
    vector<const char *> v;
    vector< vector<const char *> > columns;

    j = 1;
    for(i = 0; i < 100; i++) {
	os.str("");
	os << "arg" << j++;
	if( !parseGetArg(name, os.str(), column_names[i]) ) break;

	os.str("");
	os << "arg" << j++;
	if( !parseGetArg(name, os.str(), values[i]) ) {
	    value.assign(string("find_indices: missing value for ") +
			column_names[i]);
	    return VARIABLE_ERROR;
	}
    }
    num_col = i;

    if(num_col == 0) {
	value.assign("find_indices: missing arguments");
	return VARIABLE_ERROR;
    }

    for(j = 0; j < num_col; j++) {
	if( !getColumnByLabel(column_names[j], v) ) {
	    value.assign(string("find_indices: name ") + column_names[j]
			+ " not found");
	    return VARIABLE_ERROR;
	}
	columns.push_back(v);
    }

    os.str("");
    nrows = numRows();
    for(i = 0; i < nrows; i++) {
	for(j = 0; j < num_col && parseCompare(values[j], columns[j][i]); j++);
	if(j == num_col) {
	    os << i+1 << ",";
	}
    }
    value.assign(os.str());
    n = (int)value.length();
    if(n > 0) value.erase(n-1); // remove trailing ','

    return STRING_RETURNED;
}

void Table::parseHelp(const char *prefix)
{
    printf("%sselect_all\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%spromote_rows\n", prefix);
    printf("%spromote_columns\n", prefix);
    printf("%ssort\n", prefix);
    printf("%ssort_selected\n", prefix);
    printf("%ssort_unique\n", prefix);
    printf("%sexpand\n", prefix);
    printf("%sreverse_order\n", prefix);
    printf("%sshow_all\n", prefix);
    printf("%sselect_row or deselect_row NUM\n", prefix);
    printf("%sselect_row or deselect_row COLUMN_LABEL=VALUE COLUMN_LABEL=VALUE...\n",
	prefix);
    printf("%sselect_row or deselect_row COLUMN_LABEL=[MIN_VALUE,MAX_VALUE]...\n",
	prefix);
    printf("%sselect_row or deselect_row COLUMN_LABEL=(MIN_VALUE,MAX_VALUE)...\n",
	prefix);
    printf("%sselect_row or deselect_row COLUMN_LABEL={VALUE1,VALUE2...}...\n",
	prefix);
    printf("%sselect_column or deselect_column COLUMN_LABEL COLUMN_LABEL...\n",
	prefix);
    printf("%sset_cell row=\"COLUMN_LABEL=VALUE...\" column=COLUMN_LABEL value=CELL_VALUE\n",
	prefix);
    printf("%sset_selected_cell value=CELL_VALUE\n", prefix);
    printf("%ssave_selected_rows file=FILENAME [append=(true,false)]\n",prefix);
    printf("%ssave_selected_columns file=FILENAME [append=(true,false)]\n",
	prefix);
}

ParseCmd Table::parseSelectRow(const string &c, string &msg, bool select)
{
    vector<int> ri;
    int n = parseGetSelectRows(c, msg, ri);

    for(int i = 0; i < n-1; i++) {
	selectRow(ri[i], select);
    }
    if(n > 0) selectRowWithCB(ri[n-1], select);
    return (n >= 0) ? COMMAND_PARSED : ARGUMENT_ERROR;
}

int Table::parseGetSelectRows(const string &rc, string &msg, vector<int> &ri)
{
    int i, row;
    vector<const char *> col_labels;

    ri.clear();
    if(stringToInt(rc.c_str(), &row)) {
	vector<int> order;
	getRowOrder(order);
	for(i = 0; i < (int)order.size() && order[i] != row-1; i++);
	if(i < (int)order.size()) {
	    ri.push_back(i);
	    return 1;
	}
	return -1;
    }
    int n=0, ncols = numColumns();
    struct ColStruct {
	char *s;
	char **in;
	int  num_in;
	bool region;
	char *s_min;
	char *s_max;
	double min;
	double max;
	bool inc_min;
	bool inc_max;
    } *cols = (struct ColStruct *)malloc(ncols*sizeof(struct ColStruct));

    getColumnLabels(col_labels);

    for(i = 0; i < ncols; i++)
    {
	string s;
	parseGetArg(rc, col_labels[i], s);
	cols[i].s = ((int)s.length() > 0) ? strdup(s.c_str()) : NULL;
	cols[i].in = NULL;
	cols[i].num_in = 0;
	cols[i].region = false;
	cols[i].s_min = NULL;
	cols[i].s_max = NULL;
	cols[i].inc_min = false;
	cols[i].inc_max = false;
	cols[i].min = 0.;
	cols[i].max = 0.;
	if(cols[i].s)
	{
	    if(cols[i].s[0] == '[' || cols[i].s[0] == '(')
	    {
		if(cols[i].s[0] == '[') cols[i].inc_min = true;
		if(stringEndsWith(cols[i].s, "]")) cols[i].inc_max = true;

		char *c, *last, s[100];
		stringcpy(s, cols[i].s, 100);
		c = strtok_r(s, "([, ])", &last);
		if(c && stringToDouble(c, &cols[i].min)) {
		    c = strtok_r(NULL, "([, ])", &last);
		    if(c && stringToDouble(c, &cols[i].max)) {
			cols[i].region = true;
			n++;
		    }
		    else {
			msg.assign(string("Table.select row: cannot parse: ")
					+ cols[i].s);
			break;
		    }
		}
		else if(c) {
		    cols[i].s_min = strdup(c);
		    c = strtok_r(NULL, "([, ])", &last);
		    if(c) {
			cols[i].s_max = strdup(c);
			cols[i].region = true;
			n++;
		    }
		    else {
			msg.assign(string("Table.select row: cannot parse: ")
					+ cols[i].s);
			break;
		    }
		}
		else {
		    msg.assign(string("Table.select row: cannot parse: ")
				+ cols[i].s);
		    break;
		}
	    }
	    else if(cols[i].s[0] == '{')
	    {
		char *c, *tok, *last, s[1000];
		stringcpy(s, cols[i].s, 1000);
		cols[i].in = (char **)malloc(sizeof(char *));
		cols[i].num_in = 0;
		tok = s;
		while( (c = strtok_r(tok, "{, }", &last)) ) {
		    tok = NULL;
		    cols[i].in = (char **)realloc(cols[i].in,
					(cols[i].num_in+1)*sizeof(char *));
		    cols[i].in[cols[i].num_in++] = strdup(c);
		}
		n++;
	    }
	    else n++;
	}
    }
    if(i < ncols) {
	while(i >= 0) {
	    Free(cols[i].s);
	    Free(cols[i].in);
	    Free(cols[i].s_min);
	    Free(cols[i].s_max);
	    i--;
	}
	Free(cols);
	return -1;
    }
    if( !n ) {
	Free(cols);
	msg.assign(string("Table.select row: cannot parse: ") + rc);
	return -1;
    }

    int nrows = numRows();
    vector<const char *> r;
    for(i = n = 0; i < nrows; i++)
    {
	getRow(i, r);

	int j = 0;
	for(j = 0; j < ncols; j++) if(cols[j].s)
	{
	    if( cols[j].in ) {
		int k = 0;
		for(k = 0; k < cols[j].num_in; k++) {
		    if(stringCaseMatch(r[j], cols[j].in[k])) break;
		}
		if(k == cols[j].num_in) break; // failed
	    }
	    else if(!cols[j].region) {
		if(!stringCaseMatch(r[j], cols[j].s)) break; // failed
	    }
	    else if(cols[j].s_min) {
		if(!cols[j].inc_min) {
		    if(strcasecmp(cols[j].s_min, r[j]) >= 0) break; // failed
		}
		else if(strcasecmp(cols[j].s_min, r[j]) > 0) break; // failed

		if(!cols[j].inc_max) {
		    if(strcasecmp(r[j], cols[j].s_max) >= 0) break; // failed
		}
		else if(strcasecmp(r[j], cols[j].s_max) > 0) break; // failed
	    }
	    else {
		double value;
		if(!stringToDouble(r[j], &value)) {
		    msg.assign(string("Table.select row: not a numeric: ")
				+ r[j]);
		    for(int k = 0; k < ncols; k++) Free(cols[k].s);
		    Free(cols);
		    return -1;
		}
		if(!cols[j].inc_min) {
		    if(cols[j].min >= value) break; // failed
		}
		else if(cols[j].min > value) break; // failed

		if(!cols[j].inc_max) {
		    if(value >= cols[j].max) break; // failed
		}
		else if(value > cols[j].max) break; // failed
	    }
	}
	if(j == ncols) { // row passed all column comparisons
	    ri.push_back(i);
	    n++;
	}
    }

    for(i = 0; i < ncols; i++) {
	Free(cols[i].s);
	Free(cols[i].in);
	Free(cols[i].s_min);
	Free(cols[i].s_max);
    }
    Free(cols);

    return n;
}

ParseCmd Table::parseSelectColumns(const string &c, string &msg, bool select)
{
    vector<int> ci;
    vector<bool> b;
    int n = parseGetSelectColumns(c, msg, ci);

    for(int i = 0; i < n; i++) b.push_back(select);

    selectColumns(ci, b);

    return (n >= 0) ? COMMAND_PARSED : ARGUMENT_ERROR;
}

int Table::parseGetSelectColumns(const string &c, string &msg, vector<int> &ci)
{
    vector<const char *> col_labels;

    ci.clear();
    getColumnLabels(col_labels);
    for(int i = 0; i < (int)col_labels.size(); i++) {
	if(stringCaseMatch(col_labels[i], c.c_str())) {
	    ci.push_back(i);
	}
    }
    return (int)ci.size();
}

ParseCmd Table::parseSetCell(const string &c, string &msg)
{
    string row_string, col_string, value;
    vector<int> ri, ci;

    if( !parseGetArg(c, "row", row_string) ) {
	msg.assign("Table.set cell: row not specified.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(c, "column", col_string) ) {
	msg.assign("Table.set cell: column not specified.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(c, "value", value) ) {
	msg.assign("Table.set cell: value not specified.");
	return ARGUMENT_ERROR;
    }
    if(	parseGetSelectRows(row_string, msg, ri) < 0 ||
	parseGetSelectColumns(col_string, msg, ci) < 0) return ARGUMENT_ERROR;

    for(int i = 0; i < (int)ri.size(); i++) {
	for(int j = 0; j < (int)ci.size(); j++) {
	    setFieldWithCB(ri[i], ci[j], value, false);
	}
    }
    if((int)ri.size() > 0 && (int)ci.size() > 0) adjustColumns();
    return COMMAND_PARSED;
}

bool Table::saveTable(const string &file, const string &access, int save_mode,
			string &msg)
{
    char *full_path=NULL;
    const char *acc;
    FILE *fp;

    full_path = Application::fullPath(file);

    acc = access.empty() ? "a" : access.c_str();

    if( !(fp = fopen(full_path, acc)) ) {
	msg.assign(string("Cannot open file: ") + full_path + "\n"
		+ strerror(errno));
	return false;
    }

    if(acc[0] == 'a') {
	struct stat file_stat;
	if(!stat(full_path, &file_stat) && file_stat.st_size > 0) {
	    fprintf(fp, "\n");
	}
    }
    if(save_mode == 0) {
	printAll(fp);
    }
    else if(save_mode == 1) {
	printSelectedRows(fp);
    }
    else {
	printSelectedColumns(fp);
    }
    fclose(fp);
    Free(full_path);
    return true;
}

void Table::printSelectedRows(FILE *fp)
{
    XtPointer value=NULL;
    long unsigned length;
    TableSelectionType select_type = selection_type;
    selection_type = TABLE_COPY_ROWS;
    if( rowSelection(&value, &length, false) ) {
	fprintf(fp, "%s\n", (char *)value);
    }
    if(value) free(value);
    selection_type = select_type;
}

void Table::printSelectedColumns(FILE *fp)
{
    XtPointer value=NULL;
    long unsigned length;
    TableSelectionType select_type = selection_type;
    selection_type = TABLE_COPY_COLUMNS;
    if( columnSelection(&value, &length, false) ) {
	fprintf(fp, "%s\n", (char *)value);
    }
    if(value) free(value);
    selection_type = select_type;
}

void Table::printAll(FILE *fp)
{
    XtPointer value=NULL;
    long unsigned length;
    TableSelectionType select_type = selection_type;
    selection_type = TABLE_COPY_ALL;
    if( rowSelection(&value, &length, false) ) {
	fprintf(fp, "%s\n", (char *)value);
    }
    if(value) free(value);
    selection_type = select_type;
}

TableAttributes * Table::showAttributes(bool show)
{
    if(table_attributes) {
	table_attributes->setVisible(show);
    }
    return table_attributes;
}

void Table::setAttributes(const string &attribute_list)
{
    setAttributes(attribute_list, attribute_list);
}

void Table::setAttributes(const string &attribute_list,
				const string &display_list)
{
    char *name;
    int len;
    if(table_attributes) {
	table_attributes->setVisible(false);
	table_attributes->destroy();
    }
    len = strlen(getName()) + strlen(" Attributes") + 1;
    name = (char *)malloc(len);
    snprintf(name, len, "%s Attributes", getName());
    table_attributes = new TableAttributes(name, this, attribute_list,
				display_list);
    free(name);
    table_attributes->addActionListener(this);
    table_attributes->setCommandString("Apply Attributes");
    setColumns();

    display_attributes = display_list;
}

void Table::setDisplayAttributes(const string &display_list)
{
    if(!table_attributes) {
	setAttributes(display_attributes, display_list);
    }
    else {
	table_attributes->setDisplayAttributes(display_list);
    }
    setColumns();
}

TAttribute Table::getAttribute(int i)
{
    if(table_attributes) {
	return table_attributes->getAttribute(i);
    }
    else {
	TAttribute a = {NULL, "", -1};
	return a;
    }
}

TAttribute Table::getAttribute(const string &name)
{
    if(table_attributes) {
	return table_attributes->getAttribute(name);
    }
    else {
	TAttribute a = {NULL, "", -1};
	return a;
    }
}

int Table::setColumns(void)
{
    Arg     args[2];
    char    **column_labels = NULL;
    int     *alignment = (int *)NULL;
    int num_columns = getLabelsAlignment(&column_labels, &alignment);

    XtSetArg(args[0], XtNcolumns, num_columns);
    XtSetArg(args[1], XtNcolumnLabels, column_labels);
    XtSetValues((Widget)tw, args, 2);
    Free(column_labels);
    setAlignment(num_columns, alignment);
    Free(alignment);
    return num_columns;
}

int Table::getLabelsAlignment(char ***column_labels, int **alignment)
{
    int num_display_attributes;
    TAttribute **a;

    *column_labels = NULL;
    *alignment = NULL;
    if(!table_attributes || (num_display_attributes =
		table_attributes->displayAttributes(&a)) == 0) return 0;

    *column_labels = (char **)mallocWarn(
			num_display_attributes*sizeof(char *));
    *alignment = (int *)mallocWarn(
			(num_display_attributes+1)*sizeof(int));

    for(int i = 0; i < num_display_attributes; i++)
    {
	(*column_labels)[i] = a[i]->name;

	if(!strcmp(a[i]->format, "%s")) {
	    (*alignment)[i] = LEFT_JUSTIFY;
	}
	else {
	    (*alignment)[i] = RIGHT_JUSTIFY;
	}
    }
    return num_display_attributes;
}

void Table::selectRowCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNselectRowCallback);
}

void Table::selectColumnCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNselectColumnCallback);
}

void Table::columnMovedCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;

    if(table->table_attributes) {
	vector<int> order;
        table->getColumnOrder(order);
        table->table_attributes->setOrder(order);
        table->resetColumnOrder();
	table->saveAttributes();
    }
    table->doCallbacks(w, calldata, XtNcolumnMovedCallback);
}

void Table::editModeCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNeditModeCallback);
}

void Table::modifyVerifyCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->modifyVerify(w, calldata);
}

void Table::modifyVerify(Widget w, XtPointer calldata)
{
    doCallbacks(w, calldata, XtNmodifyVerifyCallback);
}

void Table::valueChangedCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNvalueChangedCallback);
}

void Table::choiceChangedCallback(Widget w, XtPointer client,XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNchoiceChangedCallback);
}

void Table::cellEnterCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNcellEnterCallback);
}

void Table::cellSelectCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNcellSelectCallback);
}

void Table::leaveWindowCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNleaveWindowCallback);
}

void Table::rowChangeCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    table->doCallbacks(w, calldata, XtNrowChangeCallback);
}

void Table::copyPasteCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Table *table = (Table*)client;
    MmTableCopyPasteCallbackStruct *cp =
		(MmTableCopyPasteCallbackStruct *)calldata;
    if(	cp->selection_type == TABLE_COPY_ROWS ||
	cp->selection_type == TABLE_COPY_ALL ||
	cp->selection_type == TABLE_COPY_COLUMNS ||
	cp->selection_type == TABLE_COPY_THIS_COLUMN)
    {
	table->column_index = cp->column;
	table->copy(cp->selection_type, cp->time);
    }
    else if(cp->selection_type == TABLE_PASTE) {
	table->paste(cp->time);
    }
}

int Table::numSelectedRows(void)
{
    vector<int> rows;
    getSelectedRows(rows);
    return (int)rows.size();
}

int Table::numSelectedColumns(void)
{
    vector<int> cols;
    getSelectedColumns(cols);
    return (int)cols.size();
}

void Table::editModeOn(void)
{
    Arg args[2];
    int horizontal_pos, vertical_pos;
    vector<int> row;

    if(edit_mode) return;

    int nselected = getSelectedRows(row);
    if(!nselected) return;

    edit_mode = true;

    int nrows = numRows();
    bool *ed = NULL;
    const char **row_labels = NULL;
    if(!(ed = (bool *)mallocWarn(nrows*sizeof(bool)))) return;
    if(!(row_labels = (const char **)mallocWarn(nrows*sizeof(const char *))))
	return;

    for(int i = 0; i < nrows; i++) {
	ed[i] = False;
	row_labels[i] = NULL;
    }
    for(int i = 0; i < nselected; i++) {
	ed[row[i]] = True;
	row_labels[row[i]] = "edit";
    }

    getScrolls(&horizontal_pos, &vertical_pos);

    selectAllRows(false);
    setRowEditable(ed);
    setRowLabels(row_labels);

    Free(ed);
    Free(row_labels);

    XtSetArg(args[0], XtNrowSelectable, False);
    XtSetArg(args[1], XtNeditable, True);
    XtSetValues((Widget)tw, args, 2);

    if(display_edit_controls) {
	XtUnmanageChild(base_widget);
	XtUnmanageChild((Widget)tw);
	XtSetArg(args[0], XmNbottomAttachment, XmATTACH_WIDGET);
	XtSetArg(args[1], XmNbottomWidget, controls->baseWidget());
	XtSetValues((Widget)tw, args, 2);
	XtManageChild(controls->baseWidget());
	XtManageChild((Widget)tw);
	XtManageChild(base_widget);
    }
    setScrolls(horizontal_pos, vertical_pos);
}

void Table::editCancel(void)
{
    Arg args[2];
    int horizontal_pos, vertical_pos;
    vector<bool> ed;

    if(!edit_mode) return;

    XtUnmanageChild(base_widget);
    XtUnmanageChild((Widget)tw);
    getScrolls(&horizontal_pos, &vertical_pos);

    getRowEditable(ed);
    XtSetArg(args[0], XtNrowSelectable, True);
    XtSetArg(args[1], XtNeditable, False);
    XtSetValues((Widget)tw, args, 2);

    editModeOff();
    restore();
    displayRowLabels(false, false);
    setRowStates(ed);

    XtSetArg(args[0], XmNbottomAttachment, XmATTACH_FORM);
    XtSetValues((Widget)tw, args, 1);
    XtUnmanageChild(controls->baseWidget());
    setScrolls(horizontal_pos, vertical_pos);
    XtManageChild((Widget)tw);
    XtManageChild(base_widget);

    edit_mode = false;
}

void Table::editSave(void)
{
    Arg args[2];
    int horizontal_pos, vertical_pos;
    vector<bool> ed;

    XtUnmanageChild(base_widget);
    XtUnmanageChild((Widget)tw);
    getScrolls(&horizontal_pos, &vertical_pos);

    getRowEditable(ed);
    XtSetArg(args[0], XtNrowSelectable, True);
    XtSetArg(args[1], XtNeditable, False);
    XtSetValues((Widget)tw, args, 2);

    editModeOff();
    displayRowLabels(false, false);
    setRowStates(ed);

    XtSetArg(args[0], XmNbottomAttachment, XmATTACH_FORM);
    XtSetValues((Widget)tw, args, 1);
    XtUnmanageChild(controls->baseWidget());
    setScrolls(horizontal_pos, vertical_pos);
    XtManageChild((Widget)tw);
    XtManageChild(base_widget);

    edit_mode = false;
}

void Table::saveAttributes(void)
{
    TAttribute **a;
    int n, num;
    char nam[200], value[2000];

    if(table_attributes && (num = table_attributes->displayAttributes(&a)) > 0)
    {
	TopWindow *tp = topWindowParent();
	FormDialog *fd = formDialogParent();
	if(tp) {
	    snprintf(nam, sizeof(nam), "%s.%s.attributes",
		tp->getName(), getName());
	}
	else if(fd) {
	    snprintf(nam, sizeof(nam), "%s.%s.attributes",
		fd->getName(), getName());
	}
	else {
	    snprintf(nam, sizeof(nam), "%s.attributes", getName());
	}
	n = 0;
	for(int i = 0; i < num; i++) {
	    snprintf(value+n, sizeof(value)-n, "%s,%s,",
			a[i]->name, a[i]->format);
	    n = (int)strlen(value);
	}
	if(n > 0) value[n-1] = '\0';  // the last ','

	putProperty(nam, value);
    }
}

bool Table::getDisplayAttributes(string &prop)
{
    string nam;

    TopWindow *tp = topWindowParent();
    FormDialog *fd = formDialogParent();
    if(tp) {
	nam = tp->getName() + string(".") + getName() + string(".attributes");
    }
    else if(fd) {
	nam = fd->getName() + string(".") + getName() + string(".attributes");
    }
    else {
	nam = getName() + string(".attributes");
    }

    return getProperty(nam, prop);
}

void Table::copy(TableSelectionType select_type, Time selection_time)
{
    selection_type = select_type;

    Free(copy_value);
    copy_length = 0;
    Free(copy_string_value);
    copy_string_length = 0;

    if(XtOwnSelection(baseWidget(), XA_PRIMARY, selection_time,
                (XtConvertSelectionProc)convertSelection,
                (XtLoseSelectionProc)loseOwnership,
                (XtSelectionDoneProc)transferDone) == FALSE)
    {
        fprintf(stderr, "Table: failed to become selection owner.\n");
    }
}

void Table::cut(TableSelectionType select_type, Time selection_time)
{
    // not ready.
}

Boolean Table::convertSelection(Widget w, Atom *selection, Atom *target,
		Atom *type, XtPointer *value, unsigned long *length,
		int *format)
{
    Table *t = (Table *)getTableFromWidget(w);
    if( !t ) {
	fprintf(stderr, "Table.convertSelection: t == NULL! \n");
	return False;
    }
    Display *d = XtDisplay(w);
    Boolean success = False;

    *type = *target;
    *format = 8;

    if(*target == t->table_row_atom)
    {
	if(t->selection_type == TABLE_COPY_ROWS ||
		t->selection_type == TABLE_COPY_ALL)
	{
	    if(t->copy_value) { // already converted for the initial copy call
		*length = t->copy_length;
		*value = (void *)malloc(*length);
		memcpy(*value, t->copy_value, *length);
		return True;
	    }
	    else if( t->rowSelection(value, length, true) ) {
		t->copy_length = *length;
		t->copy_value = (void *)malloc(t->copy_length);
		memcpy(t->copy_value, *value, t->copy_length);
		return True;
	    }
	}
    }
    else if(*target == t->table_column_atom)
    {
	if(t->selection_type == TABLE_COPY_COLUMNS ||
		t->selection_type == TABLE_COPY_THIS_COLUMN)
	{
	    if(t->copy_value) { // already converted for the initial copy call
		*length = t->copy_length;
		*value = (void *)malloc(*length);
		memcpy(*value, t->copy_value, *length);
		return True;
	    }
	    else if( t->columnSelection(value, length, true) ) {
		t->copy_length = *length;
		t->copy_value = (void *)malloc(t->copy_length);
		memcpy(t->copy_value, *value, t->copy_length);
		return True;
	    }
	}
    }
    else if(*target == XA_STRING ||
	    *target == XA_TEXT(d) ||
	    *target == XA_COMPOUND_TEXT(d))
    {
	if(t->copy_string_value) {// already converted for the initial copy call
	    *length = t->copy_string_length;
	    *value = (void *)malloc(*length);
	    memcpy(*value, t->copy_string_value, *length);
	    return True;
	}
	else  if(t->selection_type == TABLE_COPY_ROWS ||
		t->selection_type == TABLE_COPY_ALL)
	{
	    success = t->rowSelection(value, length, false);
	}
	else {
	    success = t->columnSelection(value, length, false);
	}
	if(success) {
	    t->copy_string_length = *length;
	    t->copy_string_value = (void *)malloc(t->copy_string_length);
	    memcpy(t->copy_string_value, *value, t->copy_string_length);
	    return True;
	}
    }
    else {
	success = XmuConvertStandardSelection(w, CurrentTime, selection,
			target, type, (char **)value, length, format);
	
	if(success && *target == XA_TARGETS(d))
	{
	    Atom *tmp;
	    tmp = (Atom *) realloc(*value, (*length + 4) * sizeof(Atom));
	    tmp[(*length)++] = XInternAtom(XtDisplay(w), "MULTIPLE", False);
	    tmp[(*length)++] = t->table_row_atom;
	    tmp[(*length)++] = t->table_column_atom;
	    tmp[(*length)++] = XA_COMPOUND_TEXT(d);
	    *value = (XtPointer) tmp;
	}
	return success;
    }

    return False;
}

bool Table::rowSelection(XtPointer *value, unsigned long *length,
			bool rows_target)
{
    int nrows;
    vector<int> rows;

    if(selection_type == TABLE_COPY_ROWS) {
	nrows = getSelectedRows(rows);
    }
    else {
	nrows = getRowOrder(rows);
    }
    if( !nrows ) return false;

    if(rows_target) // return the selection in TABLE_COPY_ROWS format
    {
	vector<const char *> row;
 	unsigned int size = 128, total_length = 0;
	char *buf = (char *)malloc(size);
	int name = stringToQuark("unformatted");
	int ncols = numColumns();

	for(int i = 0; i < nrows; i++) {
	    if(total_length + 2*sizeof(int) > size) {
		size += 128;
		buf = (char *)realloc(buf, size);
	    }
	    // the row starts with two ints: the row length and the row type
	    int row_length = 2*sizeof(int);

	    int row_length_n = total_length; // save the row length spot
	    // skip the row length and store row format type
	    total_length += sizeof(int);
	    memcpy(buf+total_length, &name, sizeof(int));
	    total_length += sizeof(int);

	    getRow(rows[i], row);
	    for(int j = 0; j < ncols; j++) {
		int len = strlen(row[j]) + 1;
		while(size < total_length + len) {
		    size += 128;
		    buf = (char *)realloc(buf, size);
		}
		strncpy(buf+total_length, row[j], len); // copy the '\0' also
		row_length += len;
		total_length += len;
	    }
	    memcpy(buf+row_length_n, &row_length, sizeof(int));
	}
	*value = (XtPointer)buf;
	*length = total_length;
    }
    else // return the selection in XA_STRING format
    {
	if( !(*value = (XtPointer)getTableString(rows, length)) ) {
	    return false;
	}
    }
    return true;
}

char * Table::getTableString(vector<int> &rows, unsigned long *length)
{
    int ncols;
    vector<int> col_nchars, alignment, col_order;
    vector<const char *> row;
    char *value=NULL;

    if( !(ncols = getColumnNChars(col_nchars)) ) {
	return NULL;
    }
    if( !getAlignment(alignment) ) {
	return NULL;
    }
    if( !getColumnOrder(col_order) ) {
	return NULL;
    }

    int row_length = 0;
    for(int i = 0; i < ncols; i++) row_length += (col_nchars[i]+3);
    int total_length = (int)rows.size()*(row_length+1);
    value = (char *)mallocWarn(total_length+1);
    *length = total_length+1;

    char *p = value;

    for(int i = 0; i < (int)rows.size(); i++)
    {
	getRow(rows[i], row);

	for(int j = 0; j < ncols; j++)
	{
	    int co = col_order[j];
	    int n = strlen(row[co]);
	    if(alignment[co] == LEFT_JUSTIFY) {
		strcpy(p, row[co]);
		p += n;
		for(int k = n; k < col_nchars[co]; k++) { *p++ = ' '; }
	    }
	    else if(alignment[co] == RIGHT_JUSTIFY) {
		for(int k = 0; k < col_nchars[co]-n; k++) { *p++ = ' '; }
		strcpy(p, row[co]);
		p += n;
	    }
	    else { // CENTER
		for(int k = 0; k < col_nchars[co]; k++) p[k] = ' ';
		int half = (col_nchars[co]-n)/2;
		strcpy(p+half, row[co]);
		p += col_nchars[co];
	    }
	    if(j < ncols-1) {
		*p++ = ' ';
	    }
	}
	if(i < (int)rows.size()-1) {
	    *p++ = '\n';
	}
    }
    *p = '\0';

    return value;
}

bool Table::columnSelection(XtPointer *value, unsigned long *length,
			bool column_target)
{
    int ncols = 0, nrows = 0;
    vector<int> cols, rows;

    if(selection_type == TABLE_COPY_COLUMNS) {
	ncols = getSelectedColumns(cols);
    }
    else if(column_index >= 0 && column_index < numColumns()) {
	ncols = 1;
	cols.push_back(column_index);
    }
    if( !ncols ) return false;

    nrows = getRowOrder(rows);

    if(column_target) // return the selection in TABLE_COPY_COLUMNS format
    {
 	unsigned int size = 128, total_length = 0;
	char *buf = (char *)malloc(size);

	for(int i = 0; i < ncols; i++)
	{
	    if(total_length + sizeof(int) > size) {
		size += 128;
		buf = (char *)realloc(buf, size);
	    }
	    // the column starts with one int: the column length
	    int col_length = sizeof(int);

	    int col_length_n = total_length; // save the col length spot
	    // skip the column length
	    total_length += sizeof(int);

	    const char **col = getColumn(cols[i]);
	    for(int j = 0; j < nrows; j++) {
		int len = strlen(col[rows[j]]) + 1;
		while(size < total_length + len) {
		    size += 128;
		    buf = (char *)realloc(buf, size);
		}
		// copy the '\0' also
		strncpy(buf+total_length, col[rows[j]], len);
		col_length += len;
		total_length += len;
	    }
	    Free(col);
	    memcpy(buf+col_length_n, &col_length, sizeof(int));
	}
	*value = (XtPointer)buf;
	*length = total_length;
    }
    else // return the selection in XA_STRING format
    {
	vector<int> col_nchars, alignment, col_order;
	vector<const char *> row;

	if( !getColumnNChars(col_nchars) || !getAlignment(alignment) ||
		!getColumnOrder(col_order) )
	{
	    return false;
	}

	int row_length = 0;
	for(int i = 0; i < ncols; i++) {
	    row_length += (col_nchars[cols[i]]+1);
	}
	int total_length = nrows*(row_length+1);
	*value = (XtPointer)mallocWarn(total_length+1);
	*length = total_length+1;

	char *p = (char *)*value;

	for(int i = 0; i < nrows; i++)
	{
	    getRow(rows[i], row);

	    for(int j = 0; j < ncols; j++)
	    {
		int co = cols[j];
		int n = strlen(row[co]);
		if(alignment[co] == LEFT_JUSTIFY) {
		    strcpy(p, row[co]);
		    p += n;
		    for(int k = n; k < col_nchars[co]; k++) { *p++ = ' '; }
		}
		else if(alignment[co] == RIGHT_JUSTIFY) {
		    for(int k = 0; k < col_nchars[co]-n; k++) { *p++ = ' '; }
		    strcpy(p, row[co]);
		    p += n;
		}
		else { // CENTER
		    for(int k = 0; k < col_nchars[co]; k++) p[k] = ' ';
		    int half = (col_nchars[co]-n)/2;
		    strcpy(p+half, row[co]);
		    p += col_nchars[co];
		}
		if(j < ncols-1) {
		    *p++ = ' ';
		}
	    }
	    if(i < nrows-1) {
		*p++ = '\n';
	    }
	}
	*p = '\0';
    }
    return true;
}

void Table::loseOwnership(Widget w, Atom *selection)
{
    Table *t = (Table *)getTableFromWidget(w);
    if( t ) {
	Free(t->copy_value);
	t->copy_length = 0;
    }
}

void Table::transferDone(Widget w, Atom *selection, Atom *target)
{
    /* Nothing to do here. Don't free, so the selection can be pasted
	somewhere more than once.
    Free(w->mmTable.field_selection);
     */
}

void Table::paste(Time selection_time)
{
    copy_time = selection_time;
    requested = table_row_atom;
    XtGetSelectionValue(baseWidget(), XA_PRIMARY, table_row_atom,
	(XtSelectionCallbackProc)requestorCallback, (XtPointer)this, copy_time);
}

void Table::requestorCallback(Widget w, XtPointer param, Atom *selection,
	Atom *type, XtPointer value, unsigned long *length, int *format)
{
    Table *t = (Table *)param;
    if( !t ) return;

    if((*type == 0 /* XT_CONVERT_FAIL */) || (*length == 0)) {
	if(t->requested == t->table_row_atom)    // try requesting a column
	{
	    t->requested = t->table_column_atom;
	    XtGetSelectionValue(w, XA_PRIMARY, t->table_column_atom,
		(XtSelectionCallbackProc)requestorCallback, param,t->copy_time);
	}
	else if(t->requested == t->table_column_atom) // try requesting a string
	{
	    t->requested = XA_STRING;
	    XtGetSelectionValue(w, XA_PRIMARY, XA_STRING,
		(XtSelectionCallbackProc)requestorCallback, param,t->copy_time);
	}
	else {
	    XBell(XtDisplay(w), 100);
	    fprintf(stderr,
		"TableViewer: no selection or selection timed out.\n");
	}
	return;
    }
    else if(*type == XA_STRING || *type == XA_COMPOUND_TEXT(XtDisplay(w)))
    {
	int ncols;
	if(t->numRows() > 0) {
	    ncols = t->numColumns();
	}
	else {
	    char *c = (char *)value;
	    ncols = 0;
	    while(*c != '\0' && isspace((int)*c)) c++;
	    while(*c != '\0' && *c != '\n') {
		ncols++;
		while(*c != '\0' && *c != '\n' && !isspace((int)*c)) c++;
		if(*c == '\0' || *c == '\n') break;
		while(*c != '\0' && *c != '\n' && isspace((int)*c)) c++;
	    }
	}
	if(ncols > 0) {
	    Arg args[1];
	    XtSetArg(args[0], XtNcolumns, ncols);
	    t->setValues(args, 1);

	    const char **row = (const char **)malloc(ncols*sizeof(char *));

	    char *c, *tok = (char *)value, *last;
	    int n = 0;
	    while( (c = (char *)strtok_r(tok, " \t\n", &last)) )
	    {
		tok = NULL;
	 	row[n++] = c;
		if(n == ncols) {
		    n = 0;
		    t->addRow(row, false);
		}
	    }
	    if(n > 0) {
		for(int i = n; i < ncols; i++) row[i] = (char *)"";
		t->addRow(row, false);
	    }
	    Free(row);
	    t->adjustColumns();
	}
    }
    else if(*type == t->table_row_atom)
    {
	int ncols = t->numColumns();
	bool is_editable = t->editable();

	const char **row=NULL;
	char *p, *c = (char *)value;
	int row_length, name;
	int nc, n = 0;

	while(n < (int)(*length))
	{
	    if(*length - n < 2*sizeof(int)) {
		break;
	    }
	    memcpy(&row_length, c+n, sizeof(int)); n += sizeof(int);
	    memcpy(&name, c+n, sizeof(int)); n += sizeof(int);

	    // must be unformatted rows
	    if(name != stringToQuark("unformatted")) break;

	    row_length -= 2*sizeof(int);

	    p = c+n;
	    nc = 0;
	    for(int i = 0; i < row_length; i++) if(p[i] == '\0') nc++;

	    if( nc )
	    {
		int ns = (nc > ncols) ? nc : ncols;
		row = (const char **)malloc(ns*sizeof(char *));

		row[0] = p;
		nc = 1;
		for(int i = 0; i < row_length; i++) {
		    if(p[i] == '\0' && i < row_length-1) {
			row[nc++] = p+i+1;
		    }
		}
		if(nc > ncols) {
		    const char **col=NULL;
		    int nrows = t->numRows();
		    col = (const char **)malloc(nrows*sizeof(char *));
		    for(int i = 0; i < nrows; i++) col[i] = "";
		    for(int i = ncols; i < nc; i++) {
			t->addColumn(col, "", is_editable, LEFT_JUSTIFY);
		    }
		    Free(col);
		    ncols = t->numColumns();
                }
		for(int i = nc; i < ncols; i++) {
		    row[i] = strdup("");
		}
		t->addRow(row, false);
		Free(row);
	    }
	    n += row_length;
	}
	t->adjustColumns();
    }
    else if(*type == t->table_column_atom)
    {
	int nrows = t->numRows();
	bool is_editable = t->editable(); 

	char *p, **col=NULL, *c = (char *)value;
	int col_length;
	int nr, n = 0;

	while(n < (int)(*length))
	{
	    if(*length - n < sizeof(int)) {
		break;
	    }
	    memcpy(&col_length, c+n, sizeof(int)); n += sizeof(int);
	    col_length -= sizeof(int);

	    p = c+n;
	    nr = 0;
	    for(int i = 0; i < col_length; i++) if(p[i] == '\0') nr++;

	    if( nr )
	    {
		int ns = (nr > nrows) ? nr : nrows;
		col = (char **)malloc(ns*sizeof(char *));

		col[0] = p;
		nr = 1;
		for(int i = 0; i < col_length; i++) {
		    if(p[i] == '\0' && i < col_length-1) {
			col[nr++] = p+i+1;
		    }
		}
		if(nr > nrows) {
		    const char **row=NULL;
		    int ncols = t->numColumns();
		    row = (const char **)malloc(ncols*sizeof(char *));
		    for(int i = 0; i < ncols; i++) row[i] = (char *)"";
		    for(int i = nrows; i < nr; i++) {
			t->addRow(row, false);
		    }
		    Free(row);
		    nrows = t->numRows();
		}
		for(int i = nr; i < nrows; i++) {
		    col[i] = strdup("");
		}
		t->addColumn((const char **)col, "", is_editable, LEFT_JUSTIFY);
		Free(col);
	    }
	    n += col_length;
	}
	t->adjustColumns();
    }
    Free(value);
}

void Table::printTime(double time, const string &format, char *value,
			int maxlen)
{
    if(!format.compare("%t")) {
	timeEpochToString(time, (char *)value, maxlen, YMONDHMS);
    }
    else if(!format.compare("%2t")) {
	timeEpochToString(time, (char *)value, maxlen, YMONDHMS3);
    }
    else if(!format.compare("%3t")) {
	timeEpochToString(time, (char *)value, maxlen, GSE20);
    }
    else if(!format.compare("%4t")) {
	timeEpochToString(time, (char *)value, maxlen, GSE21);
    }
    else if(!format.compare("%5t")) {
	timeEpochToString(time, (char *)value, maxlen, YMOND);
    }
    else {
	snprintf(value, maxlen, format.c_str(), time);
    }
}

void Table::save(const string &file, const string &access)
{
    FILE *fp;
    int nrows;
    vector<int> r;
    unsigned long length;
    char *s;

    if( !(fp = fopen(file.c_str(), access.c_str())) ) {
	if(errno > 0) {
	    showWarning("Cannot open: %s\n%s", file.c_str(), strerror(errno));
	}
	else {
	    showWarning("Cannot open: %s", file.c_str());
	}
	return;
    }

    nrows = numRows();
    for(int i = 0; i < nrows; i++) r.push_back(i);

    if( (s = getTableString(r, &length)) ) {
	fprintf(fp, "%s\n", s);
    }
    Free(s);
    fclose(fp);
}

ParseVar Table::getRowRequest(const string &input, string &msg)
{
    char *vname;
    const char *cmd="start", *nam;
    vector<const char *> row;
    int len;
    vector<int> order;
    bool selected=false;

    if(parseCompare(input, "foreach_start_row:", 18)) {
        nam = input.c_str()+18;
    }
    else if(parseCompare(input, "foreach_start_sel_row:", 22)) {
        nam = input.c_str()+22;
        selected = true;
    }
    else if(parseCompare(input, "foreach_next_row:", 17)) {
        nam = input.c_str()+17;
	cmd = "next";
    }
    else if(parseCompare(input, "foreach_next_sel_row:", 21)) {
        nam = input.c_str()+21;
        selected = true;
	cmd = "next";
    }
    else if(parseCompare(input, "foreach_stop_row:", 17)) {
        nam = input.c_str()+17;
	cmd = "stop";
    }
    else if(parseCompare(input, "foreach_stop_sel_row:", 21)) {
        nam = input.c_str()+21;
        selected = true;
	cmd = "stop";
    }
    else {
        return VARIABLE_NOT_FOUND;
    }

    if(!selected) {
	len = (int)sizeof(row_name)-1;
	vname = row_name;
    }
    else {
	len = (int)sizeof(selected_row_name)-1;
	vname = selected_row_name;
    }
    strncpy(vname, nam, len);
    if(vname[0] == '\0') return VARIABLE_ERROR;

    if(!strcasecmp(cmd, "start"))
    {
        if(!selected) {
	    parse_row_index = 0;
	    if(numRows() > 0) {
		getRowOrder(order);
		getRow(order[0], row);
		putParseString(vname, row);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
        }
        else {
	    selected_row_index = 0;
	    getSelectedRows(selected_rows);
	    if((int)selected_rows.size() > 0) {
		getRow(selected_rows[0], row);
		putParseString(vname, row);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
	}
    }
    else if(!strcasecmp(cmd, "next"))
    {
        if(!selected) {
	    parse_row_index++;
	    if(parse_row_index < numRows()) {
		getRowOrder(order);
		getRow(order[parse_row_index], row);
		putParseString(vname, row);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
        }
        else {
	    selected_row_index++;
	    if(selected_row_index < (int)selected_rows.size()) {
		getRow(selected_rows[selected_row_index], row);
		putParseString(vname, row);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
	}
    }
    else if(!strcasecmp(cmd, "stop"))
    {
        if(selected) {
	    selected_rows.clear();
	}
	parse_row_index = 0;
	selected_row_index = 0;
	Application::putParseProperty(vname, "");
        return STRING_RETURNED;
    }
    else {
	return VARIABLE_ERROR;
    }
}

ParseVar Table::getColumnRequest(const string &input, string &msg)
{
    char *vname;
    const char *cmd="start", *nam;
    vector<const char *> column;
    int len;
    bool selected=false;

    if(parseCompare(input, "foreach_start_column:", 21)) {
        nam = input.c_str()+21;
    }
    else if(parseCompare(input, "foreach_start_sel_column:", 25)) {
        nam = input.c_str()+25;
        selected = true;
    }
    else if(parseCompare(input, "foreach_next_column:", 20)) {
        nam = input.c_str()+20;
	cmd = "next";
    }
    else if(parseCompare(input, "foreach_next_sel_column:", 24)) {
        nam = input.c_str()+24;
        selected = true;
	cmd = "next";
    }
    else if(parseCompare(input, "foreach_stop_column:", 20)) {
        nam = input.c_str()+20;
	cmd = "stop";
    }
    else if(parseCompare(input, "foreach_stop_sel_column:", 24)) {
        nam = input.c_str()+24;
        selected = true;
	cmd = "stop";
    }
    else {
        return VARIABLE_NOT_FOUND;
    }

    if(!selected) {
	len = (int)sizeof(column_name)-1;
	vname = column_name;
    }
    else {
	len = (int)sizeof(selected_column_name)-1;
	vname = selected_column_name;
    }
    strncpy(vname, nam, len);
    if(vname[0] == '\0') return VARIABLE_ERROR;

    if(!strcasecmp(cmd, "start"))
    {
        if(!selected) {
	    parse_column_index = 0;
	    if(numColumns() > 0) {
		getColumn(0, column);
		putParseString(vname, column);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
        }
        else {
	    selected_column_index = 0;
	    getSelectedColumns(selected_columns);
	    if((int)selected_columns.size() > 0) {
		getColumn(selected_columns[0], column);
		putParseString(vname, column);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
	}
    }
    else if(!strcasecmp(cmd, "next"))
    {
        if(!selected) {
	    parse_column_index++;
	    if(parse_column_index < numColumns()) {
		getColumn(parse_column_index, column);
		putParseString(vname, column);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
        }
        else {
	    selected_column_index++;
	    if(selected_column_index < (int)selected_columns.size()) {
		getColumn(selected_columns[selected_column_index], column);
		putParseString(vname, column);
		return FOREACH_MORE;
	    }
	    else {
		return FOREACH_NO_MORE;
	    }
	}
    }
    else if(!strcasecmp(cmd, "stop"))
    {
        if(selected) {
	    selected_columns.clear();
	}
	parse_column_index = 0;
	selected_column_index = 0;
	Application::putParseProperty(vname, "");
        return STRING_RETURNED;
    }
    else {
	return VARIABLE_ERROR;
    }
}

void Table::putParseString(const string &vname, vector<const char *> &row)
{
    char *s=NULL;
    int len=0, n;

    for(int i = 0; i < (int)row.size(); i++) {
	len += (int)strlen(row[i]) + 1;
    }
    len++;

    s = (char *)malloc(len);
    memset(s, 0, len);

    for(int i = 0; i < (int)row.size(); i++) {
	n = (int)strlen(s);
	snprintf(s+n, len-n, "%s,", row[i]);
    }
    n = (int)strlen(s);
    if(n > 0) s[n-1] = '\0'; // the last ','

    Application::putParseProperty(vname, s);
    Free(s);
}

ParseVar Table::getCellWithIndex(const char *input, string &msg)
{
    int i, j, oi, oj;
    vector<int> order;
    const char *s = strstr(input, "[");

    if(!s) return VARIABLE_ERROR;

    s++;
    if(sscanf(s, "%d][%d", &i, &j) != 2) return VARIABLE_ERROR;

    if(i < 1 || i > numRows()) {
	msg.assign("Table.cell[][]: invalid row index.");
	return VARIABLE_ERROR;
    }
    if(j < 1 || j > numColumns()) {
	msg.assign("Table.cell[][]: invalid column index.");
	return VARIABLE_ERROR;
    }
    getRowOrder(order);
    oi = order[i-1];

    getColumnOrder(order);
    oj = order[j-1];

    s = getField(oi, oj);
    msg.assign(s);
    Free(s);
    return STRING_RETURNED;
}

ParseVar Table::getRowWithIndex(const char *input, string &value)
{
    int i, oi, n, num;
    vector<int> sel_rows;
    vector<const char *> row, labels;
    char col[100];
    const char *e, *s = strstr(input, "[");
    const char *cmd;

    if(!s) return VARIABLE_ERROR;

    s++;
    if(sscanf(s, "%d", &i) != 1) return VARIABLE_ERROR;

    if(!strncmp(input, "row[", 4)) {
	if(i < 1 || i > numRows()) {
	    value.assign("Table.row[]: invalid row index.");
	    return VARIABLE_ERROR;
	}
//	getRowOrder(&order);
//	oi = order[i-1];
	oi = i-1;
//	Free(order);
	cmd = "row";
    }
    else {
	num = getSelectedRows(sel_rows);
	if(i < 1 || i > num) {
	    value.assign("Table.sel_row[]: invalid row index.");
	    return VARIABLE_ERROR;
	}
	oi = sel_rows[i-1];
	cmd = "sel_row";
    }
    if( !(s = strstr(s, "]")) ) {
	value.assign(string("Table.") + cmd + "[]: syntax error.");
	return VARIABLE_ERROR;
    }

    s++;
    if(*s == '.') {
	s++;
	e = s;
	while(*e != '\0' && *e != '(') e++;
	if(e == s) {
	    value.assign(string("Table.") + cmd + "[]: syntax error.");
	    return VARIABLE_ERROR;
	}
	n = e-s;
	if(n > (int)sizeof(col)-1) n = (int)sizeof(col)-1;
	strncpy(col, s, n);
	col[n] = '\0';

	getColumnLabels(labels);
	for(int j = 0; j < (int)labels.size(); j++) {
	    if(sameName(labels[j], col, strlen(col))) {
		getRow(oi, row);
		value.assign(row[j]);
		return STRING_RETURNED;
	    }
	}
	if(!strcasecmp(col, "index")) {
	    ostringstream os;
	    if((int)sel_rows.size()==0) {
		os << i;
		value.assign(os.str());
	    }
	    else {
		os << sel_rows[i-1]+1;
		value.assign(os.str());
	    }
	    return STRING_RETURNED;
	}
	value.assign(string("column ") + col + " not found.");
	return VARIABLE_ERROR;
    }

    getRow(oi, row);

    value.clear();
    for(i = 0; i < (int)row.size(); i++) {
	value.append(row[i]);
	if(i < (int)row.size() - 1) value.append(",");
    }
    return STRING_RETURNED;

}

ParseVar Table::getColumnWithIndex(const char *name, string &value)
{
    int j, oj;
    vector<int> order;
    const char *s = strstr(name, "[");
    vector<const char *> column;

    if(!s) return VARIABLE_ERROR;

    s++;
    if(sscanf(s, "%d", &j) != 1) return VARIABLE_ERROR;

    if(j < 1 || j > numColumns()) {
	value.assign("Table.column[]: invalid column index.");
	return VARIABLE_ERROR;
    }
    getColumnOrder(order);
    oj = order[j-1];

    getColumn(oj, column);

    value.clear();
    for(j = 0; j < (int)column.size(); j++) {
	value.append(column[j]);
	if(j < (int)column.size()-1) value.append(",");
    }
    return STRING_RETURNED;
}

static bool
stringArgPlus(const string &name, char *row_name, string &col, string &fmt)
{
    size_t n;

    if((n = name.find('.')) == string::npos) return false;

    if(n != strlen(row_name) || !parseCompare(name, row_name, n)) {
	return false;
    }

    n++;
    if(name.length() == n || isspace((int)name[n])) return false;

    while(n < name.length() && !isspace((int)name[n]) && name[n] != '(') {
	col.append(1, name[n++]);
    }

    fmt.clear();

    if(name[n] == '(') {
	n++;
	while(n < name.length() && name[n] != ')') {
	    fmt.append(1, name[n++]);
	}
    }
    if(fmt.empty()) fmt.assign("%s");
    return true;
}
