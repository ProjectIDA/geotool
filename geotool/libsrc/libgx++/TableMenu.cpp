/** \file TableMenu.cpp
 *  \brief Defines class TableMenu.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "TableMenu.h"
#include "widget/Table.h"
#include "motif++/MotifClasses.h"

TableMenu::TableMenu(const string &name, MenuBar *menu_bar) :
		Menu(name, menu_bar)
{
    init();
}

TableMenu::TableMenu(const string &name, Menu *menu) : Menu(name, menu)
{
    init();
}

void TableMenu::init()
{
    attributes_button = new Button("Attributes...", this, this);
    copy_rows_button = new Button("Copy Selected Rows", this, this);
    copy_all_button = new Button("Copy All Rows", this, this);
    copy_columns_button = new Button("Copy Selected Columns", this, this);
    sort_button = new Button("Sort by Selected Columns", this, this);
    sort_selected_button = new Button("Sort Selected Rows Only", this, this);
    sort_unique_button = new Button("Sort Unique", this, this);
    expand_button = new Button("Expand", this, this);
    reverse_button = new Button("Reverse Order", this, this);
    show_all_button = new Button("Show All", this, this);

    addActionListener(this, XmNcascadingCallback);

    table = NULL;
}

TableMenu::~TableMenu(void)
{
}

void TableMenu::setTable(Table *new_table)
{
    table = new_table;
}

void TableMenu::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if( !table ) return;

    if(!strcmp(reason, XmNcascadingCallback)) {
	bool row_selected = table->rowSelected();
	bool col_selected = table->columnSelected();

	copy_rows_button->setSensitive(row_selected);
	copy_columns_button->setSensitive(col_selected);
	sort_button->setSensitive(col_selected);
	sort_selected_button->setSensitive(col_selected && row_selected);
	sort_unique_button->setSensitive(col_selected);
	expand_button->setSensitive(col_selected && row_selected);
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
    else if(!strcmp(cmd, "Copy Selected Rows")) {
	XmPushButtonCallbackStruct *c =
		(XmPushButtonCallbackStruct *)action_event->getCalldata();
	XButtonEvent *e = (XButtonEvent *)c->event;
	table->copy(TABLE_COPY_ROWS, e->time);
    }
    else if(!strcmp(cmd, "Copy All Rows")) {
        XmPushButtonCallbackStruct *c =
                (XmPushButtonCallbackStruct *)action_event->getCalldata();
        XButtonEvent *e = (XButtonEvent *)c->event;
        table->copy(TABLE_COPY_ALL, e->time);

    }
    else if(!strcmp(cmd, "Copy Selected Columns")) {
        XmPushButtonCallbackStruct *c =
                (XmPushButtonCallbackStruct *)action_event->getCalldata();
        XButtonEvent *e = (XButtonEvent *)c->event;
        table->copy(TABLE_COPY_COLUMNS, e->time);
    }
}
