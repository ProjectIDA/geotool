#ifndef _REF_STA_H
#define _REF_STA_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "widget/Table.h"

class DataSource;

/**
 *  @ingroup libgx
 */
class RefSta : public FormDialog, public DataReceiver
{
    public:
	RefSta(const string &name, Component *parent, const string &net,
		DataSource *ds);
	RefSta(const string &name, Component *parent, vector<int> &nets,
		DataSource *ds);

	~RefSta(void);

	void actionPerformed(ActionEvent *action_event);

	void setDataSource(DataSource *ds);
	void setDataSource(DataSource *ds, vector<int> &nets);

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:

	RowColumn	*controls;
	Button		*close_button, *apply_button, *make_perm_button;
	Separator	*sep;
	Table		*table;
	vector<int>	network;
	bool		applied;

	void createInterface(void);
	void selectStation(MmTableSelectCallbackStruct *c);
	void apply(bool permanent);

    private:
};

#endif
