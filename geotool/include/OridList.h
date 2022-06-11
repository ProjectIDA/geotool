#ifndef _ORID_LIST_H
#define _ORID_LIST_H

#include "DataReceiver.h"
#include "motif++/MotifDecs.h"
#include "motif++/MotifComp.h"
#include "gobject++/DataSource.h"

/**
 */

class Table;

/** OridList
 *  @ingroup libgxx
 */
class OridList : public Form, public DataReceiver
{
    public:
	OridList(const string &name, Component *parent, Arg *args, int n,
		DataSource *s);
	~OridList(void);

	void setDataSource(DataSource *ds);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	Table *orid_list;

	void actionPerformed(ActionEvent *action_event);
	void makeOridList(void);
	void selectWorkingOrid(void);
	void changeWorkingOrid(void);

    private:

};

#endif
