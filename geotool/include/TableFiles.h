#ifndef _TABLE_FILES_H
#define _TABLE_FILES_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"

/** 
 *  @ingroup libgx
 */
class TableFiles : public FormDialog
{
    public:
	TableFiles(const string &, Component *);
	~TableFiles(void);

	void actionPerformed(ActionEvent *action_event);
	void filesInit(void);

    protected:
	RowColumn	*controls;
	Button		*close_button, *view_button, *edit_button, *help_button;
	Separator	*sep;
	Table		*table;
	vector<TableViewer *>	windows;
	int		edit_file;

	void createInterface(void);
	void list(void);
	void selectFile(MmTableSelectCallbackStruct *c);
	void view(void);
	void saveFile(void);
	void propertyChange(char *name);

    private:
};

#endif
