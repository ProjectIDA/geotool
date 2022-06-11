#ifndef TABLE_MENU_H
#define TABLE_MENU_H

#include "motif++/Menu.h"
#include "motif++/MenuBar.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"

/** A menu for Table operations.
 *  @ingroup libgx
 */
class TableMenu : public Menu
{
    public:

	TableMenu(const string &, MenuBar *);
	TableMenu(const string &, Menu *);
	~TableMenu(void);

	void actionPerformed(ActionEvent *action_event);
	void setTable(Table *table);

    protected:
	Table *table;

	Button *attributes_button;
	Button *copy_rows_button;
	Button *copy_all_button;
	Button *copy_columns_button;
	Button *sort_button;
	Button *sort_selected_button;
	Button *sort_unique_button;
	Button *expand_button;
	Button *reverse_button;
	Button *show_all_button;

	void init(void);
};

#endif
