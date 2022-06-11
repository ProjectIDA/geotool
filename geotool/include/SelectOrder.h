#ifndef _SELECTORDER_H
#define _SELECTORDER_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "motif++/Button.h"
#include "motif++/Toggle.h"

/**
 *  @ingroup libgx
 */
class SelectOrder : public FormDialog
{
    public:
	SelectOrder(const string &name, Component *parent, int num,
		const char **list, int num_selected, int *selected,
		int width=200, int height=500);
	~SelectOrder(void);

	void actionPerformed(ActionEvent *action_event);

	int getNum(void) { return num; }
	int getNumSelected(void);
	int getOrder(int **order);
	bool applyChange(void);
	char **getNames(void);
	void resetNames(int num_names, const char **select_names,
			int num_selected, int *selected);
	void deselectAll(void);
	void select(const string &name);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

	bool close_when_apply;

    protected:

	RowColumn	*rc;
	RowColumn	*contents;
	ScrolledWindow	*sw;
	RowColumn	*controls1;
	RowColumn	*controls2;
	Button		*deselect_all;
	Button		*cancel;
	Button		*apply;
	RowColumn	**rco;
	Label		**labels;
	Toggle		**toggles;

	char		**names;
	char		**orderedNames;
	int		*order;
	int		num;

	void createInterface(int num, const char **list, int num_selected,
			int *selected, int width, int height);
	void changeOrder(Component *);

    private:
};

#endif
