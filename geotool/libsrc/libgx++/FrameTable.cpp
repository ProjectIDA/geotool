/** \file FrameTable.cpp
 *  \brief Defines class FrameTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "FrameTable.h"
#include "widget/Table.h"
#include "motif++/MotifClasses.h"

FrameTable::FrameTable(const string &name, Component *parent) : Frame(name,parent)
{
    createInterface();
}

void FrameTable::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);
    deselect_all_button = new Button("Deselect All", view_menu, this);
    promote_rows_button = new Button("Promote Selected Rows", view_menu, this);
    promote_cols_button = new Button("Promote Selected Columns",view_menu,this);
    sort_button = new Button("Sort by Selected Columns", view_menu, this);
    sort_selected_button = new Button("Sort Selected Rows Only",view_menu,this);
    sort_unique_button = new Button("Sort Unique", view_menu, this);
    expand_button = new Button("Expand", view_menu, this);
    reverse_button = new Button("Reverse Order", view_menu, this);
    show_all_button = new Button("Show All", view_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableTitle, getName()); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XtNwidth, 400); n++;
//XtSetArg(args[n], XtNcntrlSelect, True); n++;
    table = new Table("table", frame_form, info_area, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNselectColumnCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    if(!tool_bar->loadDefaults()) {
	tool_bar->add(close_button);
    }

    setButtonsSensitive();
}

FrameTable::~FrameTable(void)
{
}

void FrameTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Promote Selected Rows")) {
	table->promoteSelectedRows();
    }
    else if(!strcmp(cmd, "Promote Selected Columns")) {
	table->promoteSelectedColumns();
    }
    else if(!strcmp(cmd, "Sort by Selected Columns")) {
	vector<int> columns;
        table->getSelectedColumns(columns);
	table->sortByColumns(columns);
    }
    else if(!strcmp(cmd, "Sort Selected Rows Only")) {
	vector<int> columns;
        table->getSelectedColumns(columns);
	table->sortSelected(columns);
    }
    else if(!strcmp(cmd, "Sort Unique")) {
	vector<int> columns;
        table->getSelectedColumns(columns);
	table->sortUnique(columns);
    }
    else if(!strcmp(cmd, "Expand")) {
	vector<int> columns;
        table->getSelectedColumns(columns);
	if((int)columns.size() > 0) {
	    table->expand(columns[0]);
        }
    }
    else if(!strcmp(cmd, "Reverse Order")) {
	table->reverseOrder();
    }
    else if(!strcmp(cmd, "Show All")) {
	table->showAll();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp(getName());
    }
    else if(!strcmp(reason, XtNattributeChangeCallback)) {
	list();
    }
    else if(comp == table) { // select row or column
	setButtonsSensitive();
	if(!strcmp(reason, XtNselectRowCallback)) {
//	  selectRow((MmTableSelectCallbackStruct *)action_event->getCalldata());
	}
    }
}

void FrameTable::setButtonsSensitive(void)
{
    bool row_selected = table->rowSelected();
    bool col_selected = table->columnSelected();

    deselect_all_button->setSensitive(row_selected);
    promote_rows_button->setSensitive(row_selected);
    promote_rows_button->setSensitive(row_selected);

    promote_cols_button->setSensitive(col_selected);
    sort_button->setSensitive(col_selected);
    sort_unique_button->setSensitive(col_selected);

    sort_selected_button->setSensitive(row_selected && col_selected);
    expand_button->setSensitive(row_selected && col_selected);
}

void FrameTable::list(void)
{
    char cell[5000];
    int num_rows = numRows();
    int num_cols = table->numColumns();
    const char **row = (const char **)malloc(num_cols*sizeof(char *));

    table->removeAllRows();

    for(int i = 0; i < num_rows; i++) {
	for(int j = 0; j < num_cols; j++) {
	    TAttribute a = table->getAttribute(j);
	    cell[0] = '\0';
	    getCell(i, &a, cell, sizeof(cell));
	    row[j] = strdup(cell);
	}
	table->addRow(row, false);
	for(int j = 0; j < num_cols; j++) Free(row[j]);
    }
    Free(row);
    table->adjustColumns();
}

ParseCmd FrameTable::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;
    string c;

    if(parseArg(cmd, "Attributes", c)) {
	TableAttributes *ta = table->showAttributes(false);
	if(ta) {
	    ret = ta->parseCmd(c, msg);
	}
    }
    else if( parseCompare(cmd, "Delete_All") ) {
	deselect_all_button->activate();
    }
    else if( parseCompare(cmd, "Promote_Selected_Rows") ) {
	promote_rows_button->activate();
    }
    else if( parseCompare(cmd, "Promote_Selected_Columns") ) {
	promote_cols_button->activate();
    }
    else if( parseCompare(cmd, "Sort_by_Selected_Columns") ) {
	sort_selected_button->activate();
    }
    else if( parseCompare(cmd, "Sort_Unique") ) {
	sort_unique_button->activate();
    }
    else if( parseCompare(cmd, "Expand") ) {
	expand_button->activate();
    }
    else if( parseCompare(cmd, "Reverse_Order") ) {
	reverse_button->activate();
    }
    else if( parseCompare(cmd, "Show_All") ) {
	show_all_button->activate();
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	if((ret = table->parseCmd(cmd, msg)) == COMMAND_NOT_FOUND) {
	    ret = Frame::parseCmd(cmd, msg);
	}
    }
    return ret;
}

void FrameTable::parseHelp(const char *prefix)
{
    printf("%sdelete_all\n", prefix);
    printf("%spromote_selected_rows\n", prefix);
    printf("%spromote_selected_columns\n", prefix);
    printf("%ssort by selected_columns\n", prefix);
    printf("%ssort_unique\n", prefix);
    printf("%sexpand\n", prefix);
    printf("%sreverse_order\n", prefix);
    printf("%sshow_all\n", prefix);
    printf("%sAttributes.help\n", prefix);
    printf("%stable.help\n", prefix);
}
