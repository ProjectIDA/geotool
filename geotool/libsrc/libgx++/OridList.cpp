/** class OridList.
 *  \brief Defines class OridList.
 *  \author Ivan Henson
 */
#include "config.h"
#include <map>

#include "OridList.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "widget/TableListener.h"

OridList::OridList(const string &name, Component *parent, Arg *args, int n,
		DataSource *s) : Form(name, parent, args, n), DataReceiver(s)
{
    int m;
    Arg a[20];

    s->addDataListener(this);

    m = 0;
    XtSetArg(a[m], XtNcolumns, 1); m++;
    XtSetArg(a[m], XtNcolumnSelectable, False); m++;
    XtSetArg(a[m], XtNeditable, False); m++;
    XtSetArg(a[m], XtNsingleSelect, True); m++;
    XtSetArg(a[m], XmNtopAttachment, XmATTACH_FORM); m++;
    XtSetArg(a[m], XmNbottomAttachment, XmATTACH_FORM); m++;
    XtSetArg(a[m], XmNleftAttachment, XmATTACH_FORM); m++;
    XtSetArg(a[m], XmNrightAttachment, XmATTACH_FORM); m++;

    const char *labels[1] = {"Working Orid"};
    XtSetArg(a[m], XtNcolumnLabels, labels); m++;
    orid_list = new Table("Working Orid", this, a, m);
    int alignment[1] = {RIGHT_JUSTIFY};
    orid_list->setAlignment(1, alignment);
    orid_list->addActionListener(this, XtNselectRowCallback);
    makeOridList();
}

OridList::~OridList(void)
{
}

void OridList::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
        if(data_source) {
            data_source->removeDataListener(this);
            data_source->removeDataReceiver(this);
        }
        data_source = ds;
        if(data_source) {
            data_source->addDataReceiver(this);
            data_source->addDataListener(this);
        }
    }
    makeOridList();
}

void OridList::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if(!strcmp(reason, XtNdataChangeCallback)) {
        DataChange *c = (DataChange *)action_event->getCalldata();
        if(c->origin || c->working_orid) makeOridList();
    }
    else if(!strcmp(reason, CssTableChange)) {
        if(!strcmp(cmd, cssOrigin)) {
            TableListenerCallback *tc = (TableListenerCallback *)action_event->getCalldata();

            if(	!strcasecmp(tc->table->getName(), cssOrigin) ||
		!strcasecmp(tc->table->getName(), cssOrigerr) ||
		!strcasecmp(tc->table->getName(), cssAssoc) ||
		!strcasecmp(tc->table->getName(), cssStamag) ||
		!strcasecmp(tc->table->getName(), cssNetmag) ||
		!strcasecmp(tc->table->getName(), cssParrival))
	    {
		changeWorkingOrid();
            }
        }
    }
    else if(action_event->getSource() == orid_list) {
	selectWorkingOrid();
    }
}

void OridList::makeOridList(void)
{
    const char *row[1];
    int k, select_pos= -1;
    long working_orid;
    cvector<CssOriginClass> origins;
    map<long, long> orids;
    map<long, long>::iterator it;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssAssocClass> assocs;
    cvector<CssStamagClass> stamags;
    cvector<CssNetmagClass> netmags;
    cvector<CssParrivalClass> parrivals;

    data_source->getTable(origins);
    data_source->getTable(origerrs);
    data_source->getTable(assocs);
    data_source->getTable(stamags);
    data_source->getTable(netmags);
    data_source->getTable(parrivals);
    for(int i = 0; i < origins.size(); i++) {
	orids[origins[i]->orid] = 1;
	TableListener::addListener(origins[i], this);
    }
    for(int i = 0; i < origerrs.size(); i++) {
	orids[origerrs[i]->orid] = 1;
	TableListener::addListener(origerrs[i], this);
    }
    for(int i = 0; i < assocs.size(); i++) {
	orids[assocs[i]->orid] = 1;
	TableListener::addListener(assocs[i], this);
    }
    for(int i = 0; i < stamags.size(); i++) {
	orids[stamags[i]->orid] = 1;
	TableListener::addListener(stamags[i], this);
    }
    for(int i = 0; i < netmags.size(); i++) {
	orids[netmags[i]->orid] = 1;
	TableListener::addListener(netmags[i], this);
    }
    for(int i = 0; i < parrivals.size(); i++) {
	orids[parrivals[i]->orid] = 1;
	TableListener::addListener(parrivals[i], this);
    }

    working_orid = data_source->getWorkingOrid();

    orid_list->removeAllRows();

    k = 0;
    for(it = orids.begin(); it != orids.end(); it++) {
	char orid[50];
	snprintf(orid, sizeof(orid), "%ld", (*it).first);
	row[0] = (const char *)orid;
	orid_list->addRow(row, False);
	if((*it).first == working_orid) select_pos = k;
	k++;
    }

    if(select_pos != -1) {
	orid_list->selectRow(select_pos, true);
    }
    orid_list->adjustColumns();
}

void OridList::selectWorkingOrid(void)
{
    vector<int> rows;

    orid_list->getSelectedRows(rows);

    if((int)rows.size() > 0) {
	long working_orid;
	vector<const char *> col;
	orid_list->getColumn(0, col);
	if(stringToLong(col[rows[0]], &working_orid)) {
            data_source->removeDataListener(this);
	    data_source->setWorkingOrid(working_orid);
            data_source->addDataListener(this);
	}
    }
    else {
	data_source->removeDataListener(this);
	data_source->setWorkingOrid(-1);
	data_source->addDataListener(this);
    }
}

void OridList::changeWorkingOrid(void)
{
    vector<int> rows;
    orid_list->getSelectedRows(rows);
    makeOridList();
    if(rows.size() > 0) {
	orid_list->selectRowWithCB(rows[0], true);
    }
}

ParseCmd OridList::parseCmd(const string &cmd, string &msg)
{
    return Form::parseCmd(cmd, msg);
}

void OridList::parseHelp(const char *prefix)
{
}
