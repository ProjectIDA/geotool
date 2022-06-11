#ifndef LIST_DIALOG_H
#define LIST_DIALOG_H

#include "motif++/FormDialog.h"
#include "motif++/List.h"
#include "motif++/RowColumn.h"
#include "motif++/Button.h"
#include "motif++/MotifComp.h"

/** A class for the XmList with a dialog window.
 *  @ingroup libmotif
 */
class ListDialog : public FormDialog
{
    public:

	ListDialog(const string &name, Component *parent, int num,char **items);
	~ListDialog(void);

	void actionPerformed(ActionEvent *action_event);
	int getSelection(char ***selected_items);
	static int getSelectedItems(const string &name, Component *parent,
			int num, char **items, char ***selected_items);

    protected:
	Label *title;
	List *list;
	RowColumn *controls;
	Separator *sep;
	Button *apply_button;
	bool answer;
};

#endif
