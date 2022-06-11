#ifndef _PLUGIN_TABLE_
#define _PLUGIN_TABLE_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"

/** 
 *  @ingroup libgx
 */
class PluginTable : public FormDialog
{
    public:
	PluginTable(const string &, Component *);
	~PluginTable(void);

	void actionPerformed(ActionEvent *action_event);

    protected:
	RowColumn	*controls;
	Button		*close_button;
	Separator	*sep;
	Table		*table;

	void createInterface(void);
	void list(void);

    private:
};

#endif
