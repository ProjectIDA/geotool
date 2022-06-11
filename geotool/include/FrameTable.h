#ifndef _FRAME_TABLE_H
#define _FRAME_TABLE_H

#include "motif++/Frame.h"
#include "widget/Table.h"
#include "widget/TableAttributes.h"

/** FrameTable class
 *  @ingroup libgx
 */
class FrameTable : public Frame
{
    public:
	FrameTable(const string &name, Component *parent);
	~FrameTable(void);

	void actionPerformed(ActionEvent *action_event);
	virtual int numRows(void) { return 0; }
	virtual void getCell(int i, TAttribute *a, char *value, int maxlen) {}
	virtual void list(void);
	Table *getTable(void) { return table; }

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	Table *table;

	// File Menu
	Button *close_button;

	// Edit menu
	
	// View Menu
	Button *attributes_button, *deselect_all_button, *promote_rows_button;
	Button *promote_cols_button, *sort_button, *sort_selected_button;
	Button *sort_unique_button, *expand_button, *reverse_button;
	Button *show_all_button;

	// Help Menu

	void createInterface(void);
	void setButtonsSensitive(void);
};

#endif
