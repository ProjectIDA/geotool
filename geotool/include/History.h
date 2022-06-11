#ifndef _HISTORY_H
#define _HISTORY_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/gvector.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"

class Table;
class GTimeSeries;
class DataMethod;
class Waveform;

/**
 *  @ingroup libgx
 */
class History : public FormDialog, public DataReceiver
{
    public:
	History(const string &, Component *, DataSource *);
	~History(void);

	void setVisible(bool visible);
	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);

    protected:
	RowColumn	*controls;
	Button		*close_button, *remove_button;
	Separator	*sep;
	Label		*label;
	Table		*table;

	struct history_item {GTimeSeries *ts; DataMethod *dm; bool no_method;};
	vector<struct history_item> history;

	void createInterface(void);
	void list(void);
	void remove(gvector<Waveform *> &wvec);

    private:
};

#endif
