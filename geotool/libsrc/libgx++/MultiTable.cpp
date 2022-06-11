/** \file MultiTable.cpp
 *  \brief Defines class MultiTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
using namespace std;

#include "MultiTable.h"
#include "widget/TableAttributes.h"
#include "motif++/ActionEvent.h"
#include "motif++/Button.h"
#include "widget/TableListener.h"
#include "BasicSource.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

TableRow::TableRow(CssTableClass *t1)
{
    num_tables = 1;
    tables[0] = t1;
    t1->addOwner(this);
    tables[1] = tables[2] = NULL;
}
TableRow::TableRow(CssTableClass *t1, int num_aux_tables, CssTableClass **aux_tables)
{
    num_tables = 1;
    tables[0] = t1;
    t1->addOwner(this);
    tables[1] = tables[2] = NULL;
    for(int i = 0; i < num_aux_tables; i++) {
	aux.push_back(aux_tables[i]);
    }
}
TableRow::TableRow(CssTableClass *t1, CssTableClass *t2)
{
    num_tables = 2;
    tables[0] = t1;
    tables[1] = t2;
    t1->addOwner(this);
    t2->addOwner(this);
    tables[2] = NULL;
}
TableRow::TableRow(CssTableClass *t1, CssTableClass *t2, int num_aux_tables,
			CssTableClass **aux_tables)
{
    num_tables = 2;
    tables[0] = t1;
    tables[1] = t2;
    t1->addOwner(this);
    t2->addOwner(this);
    tables[2] = NULL;
    for(int i = 0; i < num_aux_tables; i++) {
	aux.push_back(aux_tables[i]);
    }
}
TableRow::TableRow(CssTableClass *t1, CssTableClass *t2, CssTableClass *t3)
{
    num_tables = 3;
    tables[0] = t1;
    tables[1] = t2;
    tables[2] = t3;
    t1->addOwner(this);
    t2->addOwner(this);
    t3->addOwner(this);
}
TableRow::TableRow(CssTableClass *t1, CssTableClass *t2, CssTableClass *t3,
                int num_aux_tables, CssTableClass **aux_tables)
{
    num_tables = 3;
    tables[0] = t1;
    tables[1] = t2;
    tables[2] = t3;
    t1->addOwner(this);
    t2->addOwner(this);
    t3->addOwner(this);
    for(int i = 0; i < num_aux_tables; i++) {
	aux.push_back(aux_tables[i]);
    }
}

TableRow::~TableRow()
{
    for(int i = 0; i < 3; i++) {
	if(tables[i]) tables[i]->removeOwner(this);
    }
}

TableRow *TableRow::copy(void)
{
    CssTableClass **ax=NULL;
    int num_aux = aux.size();

    if(aux.size()) {
	ax = (CssTableClass **)mallocWarn(aux.size()*sizeof(CssTableClass *));
    }
    for(int i = 0; i < aux.size(); i++) {
	ax[i] = aux[i]->clone();
    }

    TableRow *o = NULL;

    if(num_tables == 1) {
	o = new TableRow(tables[0]->clone(), num_aux, ax);
    }
    else if(num_tables == 2) {
	o = new TableRow(tables[0]->clone(), tables[1]->clone(), num_aux, ax);
    }
    else if(num_tables == 3) {
	o = new TableRow(tables[0]->clone(), tables[1]->clone(),
			tables[2]->clone(), num_aux, ax);
    }
    Free(ax);
    return o;
}

void TableRow::copyTo(TableRow *o)
{
    if(num_tables == o->num_tables) {
	for(int i = 0; i < num_tables; i++) {
	    o->tables[i] = tables[i]->clone();
	}
    }
}

void TableRow::addTable(CssTableClass *t)
{
    if(num_tables < 3) {
	tables[num_tables] = t;
	t->addOwner(this);
	num_tables++;
    }
    else {
	fprintf(stderr, "TableRow: adding too many tables.\n");
    }
}

void TableRow::addListener(Component *comp)
{
    for(int i = 0; i < num_tables; i++) {
	if(tables[i]) TableListener::addListener(tables[i], comp);
    }
    for(int i = 0; i < aux.size(); i++) {
	if(aux[i]) {
	    TableListener::addListener(aux[i], comp);
	}
    }
}

void TableRow::removeListener(Component *comp)
{
    for(int i = 0; i < num_tables; i++) {
	if(tables[i]) TableListener::removeListener(tables[i], comp);
    }
    for(int i = 0; i < aux.size(); i++) {
	if(aux[i]) {
	    TableListener::removeListener(aux[i], comp);
	}
    }
}

MultiTable::MultiTable(const string &name, Component *parent, Arg *args, int n)
                : Table(name, parent, args, n), DataReceiver(NULL)
{
    num_types = 0;
    table_types = NULL;
    des = NULL;
    num_members = NULL;
    num_aux = 0;
    aux_des = NULL;
    num_aux_members = 0;
}

MultiTable::MultiTable(const string &name, Component *parent, DataSource *ds,
                Arg *args, int n) : Table(name, parent, args, n),
		DataReceiver(ds)
{
    num_types = 0;
    table_types = NULL;
    des = NULL;
    num_members = NULL;
    num_aux = 0;
    aux_des = NULL;
    num_aux_members = 0;
}

MultiTable::~MultiTable(void)
{
    if(table_attributes) table_attributes->destroy();
    for(int i = 0; i < num_types; i++) Free(table_types[i]);
    Free(table_types);
    Free(des);
    Free(num_members);
}

void MultiTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    int horizontal_pos, vertical_pos;

    if(!strcmp(reason, CssTableChange)) {
	TableListenerCallback *tc =
		(TableListenerCallback *)action_event->getCalldata();
	for(int i = (int)records.size()-1; i >= 0; i--) {
	    for(int j = 0; j < records[i]->num_tables; j++) {
		if(tc->table == records[i]->tables[j]) {
		    if( !strcmp(cmd, "delete") ) {
			getScrolls(&horizontal_pos, &vertical_pos);
			vector<int> v;
			v.push_back(i);
			removeRecords(v);
			setScrolls(horizontal_pos, vertical_pos);
		    }
		    else {
			getScrolls(&horizontal_pos, &vertical_pos);
			list(i);
			setScrolls(horizontal_pos, vertical_pos);
		    }
		}
		break;
	    }
	}
	return;
    }
    else if(!strcmp(cmd, "Save")) {
	editSave();
	return;
    }

    Table::actionPerformed(action_event);

    if(!strcmp(cmd, "Apply Attributes")) {
        list();
    }
}

void MultiTable::editModeOn(void)
{
    if(edit_mode) return;

    vector<int> row;
    getSelectedRows(row);
    if((int)row.size() == 0) return;

    backup_order.ensureCapacity((int)row.size());
    backup_copy.ensureCapacity((int)row.size());

    for(int i = 0; i < (int)row.size(); i++) {
	backup_order.add(records[row[i]]);
	backup_copy.add(records[row[i]]->copy());
    }
    Table::editModeOn();
}

void MultiTable::editCancel(void)
{
    int horizontal_pos, vertical_pos;
    vector<bool> ed;
    char **row_labels=NULL;

    getRowEditable(ed);

    row_labels = getRowLabels();

    for(int i = 0; i < (int)ed.size(); i++) if(row_labels[i])
    {
	for(int j = 0; j < backup_order.size(); j++) {
	    if(backup_order[j] == records[i]) {
		backup_copy[j]->copyTo(records[i]);
		break;
	    }
	}
    }
    backup_order.clear();
    backup_copy.clear();
    Free(row_labels);

    getScrolls(&horizontal_pos, &vertical_pos);

    Table::editCancel();

    setRowStates(ed);

    setScrolls(horizontal_pos, vertical_pos);
}

void MultiTable::editSave(void)
{
    int horizontal_pos, vertical_pos, num;
    vector<bool> ed;
    vector<int> row_order, col_order;
    char **row_labels=NULL;

    num = getRowEditable(ed);
    row_labels = getRowLabels();

    getScrolls(&horizontal_pos, &vertical_pos);
    getRowOrder(row_order);
    getColumnOrder(col_order);

    Table::editSave();

    gvector<MultiTableEdit *> *v = new gvector<MultiTableEdit *>;

    for(int i = 0; i < num; i++) if(row_labels[i] && rowEdited(i)) 
    {
	for(int j = 0; j < backup_order.size(); j++) {
	    if(backup_order[j] == records[i]) {
		v->add(new MultiTableEdit(backup_copy[j], records[i]));
		break;
	    }
	}
    }

    doCallbacks((Widget)tw, (XtPointer)v, XtNeditSaveCallback);
    v->deleteObject();

    errorMsg(); // clear last error message

    for(int i = 0; i < num; i++) if(row_labels[i] && rowEdited(i)) 
    {
	int n;
	for(n = 0; n < backup_order.size(); n++) {
	    if(backup_order[n] == records[i]) break;
	}
	if(n == backup_order.size()) continue; // should not happen

	for(int j = 0; j < records[i]->num_tables; j++) {
	    CssTableClass *o = records[i]->tables[j];
	    CssTableClass *orig = backup_copy[n]->tables[j];
	    if(backup_copy[n]->num_tables <= j || !o->strictlyEquals(*orig))
	    {
		DataSource *ds = o->getDataSource();
		if(ds) {
		    if(backup_copy[n]->num_tables <= j) {
			ds->addTable(o);
		    }
		    else if( !ds->changeTable(orig, o) ) {
			orig->copyTo(o);
		    }
		    else {
			TableListener::doCallbacks(orig, o, this);
		    }
		    showErrorMsg();
		}
	    }
	}
    }

    backup_order.clear();
    backup_copy.clear();

    backup(); // Table class

    setRowStates(ed);

    setRowOrder(row_order);
    setColumnOrder(col_order);

    setScrolls(horizontal_pos, vertical_pos);
}

void MultiTable::modifyVerify(Widget w, XtPointer calldata)
{
    MmTableEditCallbackStruct *c = (MmTableEditCallbackStruct *)calldata;
    if(c->row < 0 || c->row >= numRows()) {
	cerr << "MultiTable.verify: row=" <<c->row<<" num_rows=" << numRows()
		<< endl;
        return;
    }
    TableRow *o = records[c->row];

    TAttribute a = getAttribute(c->column);

    if(a.table >= o->num_tables && a.table < num_types)
    {
	CssTableClass *css = CssTableClass::createCssTable(table_types[a.table]);
	o->addTable(css);
	DataSource *ds = o->tables[0]->getDataSource();
	if(ds) {
	    css->setDataSource(ds);
	}
	DataSource::copySourceInfo(css, o->tables[0]);
    }

    if(a.table >= 0 && a.table < o->num_tables)
    {
	CssTableClass *css = o->tables[a.table];
	if(!css->setMember(a.member, c->new_string)) {
	    showWarning("%s", CssTableClass::getError());
	    restoreField(c->row, c->column);
	}
	else if(!strcmp(a.name, "phase") && a.table == 1
		&& !strcmp(table_types[0], cssArrival)
		&& !strcmp(table_types[1], cssAssoc))
	{
	    if(getProperty("syncPhaseAndIphase", "true")) {
		CssArrivalClass *arrival = (CssArrivalClass *)o->tables[0];
		CssAssocClass *assoc = (CssAssocClass *)o->tables[1];
		strcpy(arrival->iphase, assoc->phase);
		strcpy(arrival->phase, assoc->phase);
	    }
	}
	else if(!strcmp(a.name, "iphase") && a.table == 0 && o->num_tables == 1
		&& !strcmp(table_types[0], cssArrival))
	{
	    CssArrivalClass *arrival = (CssArrivalClass *)o->tables[0];
	    strcpy(arrival->phase, arrival->iphase);
	}
    }
    else if(a.table >= o->num_tables)
    {
	int k = a.table - o->num_tables;
	if(k < o->aux.size()) {
	    CssTableClass *css = o->aux[k];
	    if(!css->setMember(a.member, c->new_string)) {
		showWarning("%s", CssTableClass::getError());
		restoreField(c->row, c->column);
	    }
	}
    }
}

void MultiTable::setType(const string &cssTableName)
{
    const char *c[1];
    c[0] = cssTableName.c_str();
    setType(1, c, 0, "", 0, NULL, NULL, "");
}

void MultiTable::setType(const string &cssTableName, const string &display_list)
{
    const char *c[1];
    c[0] = cssTableName.c_str();
    setType(1, c, 0, "", 0, NULL, NULL, display_list);
}

void MultiTable::setType(int num_tables, const char **cssTableNames)
{
    setType(num_tables, cssTableNames, 0, "", 0, NULL, NULL, "");
}

void MultiTable::setType(int num_tables, const char **cssTableNames,
		int num_extra, const char **extra, const char **extra_formats)
{
    setType(num_tables, cssTableNames, 0, "", num_extra, extra, extra_formats,
		 "");
}

void MultiTable::setType(int num_tables, const char **cssTableNames,
		int num_extra, const char **extra, const char **extra_formats,
		const string &display_list)
{
    setType(num_tables, cssTableNames, 0, "", num_extra, extra, extra_formats,
		 display_list);
}

void MultiTable::setType(int num_tables, const char **cssTableNames,
			const string &display_list)
{
    setType(num_tables, cssTableNames, 0, "", 0, NULL, NULL, display_list);
}

void MultiTable::setType(int num_tables, const char **cssTableNames,
	int num_aux_tables, const string &auxTableName, int num_extra,
	const char **extra, const char **extra_formats,
	const string &display_list)
{
    char *name;
    int i, len, n;

    if(table_attributes) {
	table_attributes->setVisible(false);
	table_attributes->destroy();
    }
    for(i = 0; i < num_types; i++) Free(table_types[i]);
    num_types = 0;
    Free(table_types);
    Free(des);
    Free(num_members);
    num_aux_members = 0;
    num_aux = 0;
    if(num_tables <= 0 && num_aux_tables <= 0 && num_extra <= 0) return;

    des = (CssClassDescription **)mallocWarn(num_tables*sizeof(CssClassDescription *));
    num_members = (int *)mallocWarn(num_tables*sizeof(int));

    n = 0;
    for(i = 0; i < num_tables; i++) {
	num_members[i] = CssTableClass::getDescription(cssTableNames[i], &des[i]);
	if(num_members[i] > 0) n++;
    }

    if(num_aux_tables > 0) {
	if((num_aux_members=CssTableClass::getDescription(auxTableName,&aux_des)) > 0)
	{
	    num_aux = num_aux_tables;
	    aux_type = auxTableName;
	}
    }

    num_types = n;
    len = 0;
    if(num_types > 0) {
	table_types = (char **)mallocWarn(num_types*sizeof(char *));
	n = 0;
	for(i = 0; i < num_tables; i++) if(num_members[i] > 0) {
	    table_types[n] = strdup(cssTableNames[i]);
	    len += strlen(table_types[n]) + 1;
	    num_members[n] = num_members[i];
	    des[n++] = des[i];
	}
    }
    if(num_aux_members > 0) {
	len += (int)aux_type.length() + 1;
    }
    len += strlen("Attributes") + 1;
    name = (char *)mallocWarn(len);
    n = 0;
    for(i = 0; i < num_types; i++) {
	snprintf(name+n, len-n, "%s ", table_types[i]);
	n = strlen(name);
    }
    if(num_aux_members > 0) {
	snprintf(name+n, len-n, "%s ", auxTableName.c_str());
	n = strlen(name);
    }
    snprintf(name+n, len-n, "Attributes");

    table_attributes = new TableAttributes(name, this, num_types,
		num_members, des, num_aux_tables, num_aux_members, aux_des,
		num_extra, extra, extra_formats, display_list);
    free(name);
    table_attributes->addActionListener(this);
    table_attributes->setCommandString("Apply Attributes");
    setColumns();
}

void MultiTable::addRecords(gvector<CssTableClass *> &v)
{
    if(!v.size()) return;

    CssTableClass *t = v[0];
    if(num_types != 1 || !t->nameIs(table_types[0])) {
	clear();
        setType(t->getName());
    }
    for(int i = 0; i < v.size(); i++) {
	records.add(new TableRow(v[i]));
	TableListener::addListener(v[i], this);
    }
    list();
}

void MultiTable::setRecord(int index, CssTableClass *css, bool redisplay)
{
    if(index < 0 || index >= records.size()) {
	cerr << "MultiTable::setRecord: bad record index: " << index << endl;
	return;
    }
    int i;
    for(i=0; i < num_types && !css->nameIs(table_types[i]); i++);
    if(i == num_types)
    {
	cerr << "MultiTable::setRecord: bad record type." << endl;
	return;
    }
    if(i < records[index]->num_tables && records[index]->tables[i] != css) {
	if(records[index]->tables[i]) {
	    TableListener::removeListener(records[index]->tables[i], this);
	    records[index]->tables[i]->removeOwner(records[index]);
	}
	records[index]->tables[i] = css;
	css->addOwner(records[index]);
	TableListener::addListener(css, this);

	if(redisplay) listKeepOrder();
    }
}

void MultiTable::addRecord(CssTableClass *css, bool redisplay)
{
    if(num_types != 1 || !css->nameIs(table_types[0])) {
	clear();
        setType(css->getName());
    }
    records.add(new TableRow(css));
    TableListener::addListener(css, this);

    if(redisplay) list();
}

void MultiTable::addRecord(TableRow *row, bool redisplay)
{
    row->addListener(this);
    records.add(row);
    row->addListener(this);
    if(redisplay) list();
}

void MultiTable::insertRecord(int index, CssTableClass *css, bool redisplay)
{
    if(num_types != 1 || !css->nameIs(table_types[0])) {
	clear();
        setType(css->getName());
    }
    records.insert(new TableRow(css), index);
    TableListener::addListener(css, this);
    if(redisplay) list();
}

void MultiTable::removeRecords(vector<int> &rows)
{
    removeRows(rows);
    for(int i = (int)rows.size()-1; i >= 0; i--) {
	records[i]->removeListener(this);
	records.removeAt(rows[i]);
    }
}

void MultiTable::listKeepOrder(void)
{
    vector<int> row_order, col_order;
    getRowOrder(row_order);
    getColumnOrder(col_order);

    list();

    setRowOrder(row_order);
    setColumnOrder(col_order);
}

void MultiTable::list(void)
{
    int i, j, k, n, num_columns, maxlen=MAXPATHLEN+1;
    int *index=NULL, *oi=NULL, *aux_pos=NULL;
    vector<enum TimeFormat> tc;
    const char **row=NULL;
    char *value;
    string sval;
    Arg args[1];
    TableRow *o;
    TAttribute **a;
    Pixel *colors=NULL, bg[3];

    removeAllRows();

    if(records.size() <= 0) {
        clear();
        return;
    }

    o  = records[0];
    if(o->num_tables > num_types || o->aux.size() > num_aux)
    {
	const char *c[20];
	string ax_type;
	
	n = 0;
	for(i = 0; i < o->num_tables && n < 20; i++) {
	    c[n++] = o->tables[i]->getName();
	}
	if(o->aux.size() > 0) {
	    ax_type.assign(o->aux[0]->getName());
	}
	    
	setType(n, c, o->aux.size(), ax_type, 0, NULL, NULL, "");
    }

    table_attributes->displayAttributes(&a);

    num_columns = numColumns();

    if(!(index = (int *)mallocWarn(num_columns*sizeof(int)))) return;
    if(!(oi = (int *)mallocWarn(num_columns*sizeof(int)))) return;
    if(!(row = (const char **)mallocWarn(num_columns*sizeof(char *)))) return;
    if(!(aux_pos = (int *)mallocWarn(num_columns*sizeof(int)))) return;
    if(!(colors = (Pixel *)mallocWarn(num_columns*sizeof(Pixel)))) return;

    XtSetArg(args[0], XmNbackground, &bg[0]);
    getValues(args, 1);
//    bg[1] = stringToPixel("grey60");
    bg[1] = pixelBrighten(bg[0], .9);
    bg[2] = stringToPixel("white");

    for(i = 0; i < num_columns; i++)
    {
	index[i] = -1;
	aux_pos[i] = -1;
	oi[i] = -1;
	colors[i] = bg[2];
	if(a[i]->table >= 0 && a[i]->table < num_types)
	{
	    k = a[i]->table;
	    for(j = 0; j < num_members[k] && 
		strcmp(des[k][j].name, a[i]->name); j++);
	    if(j < num_members[k])
	    {
		oi[i] = k;
		index[i] = j;
		if(des[k][j].type == CSS_TIME)
		{
		    if(!strcmp(a[i]->format, "%t")) tc.push_back(YMONDHMS);
                    else if(!strcmp(a[i]->format,"%2t"))tc.push_back(YMONDHMS3);
                    else if(!strcmp(a[i]->format, "%3t")) tc.push_back(GSE20);
                    else if(!strcmp(a[i]->format, "%4t")) tc.push_back(GSE21);
                    else if(!strcmp(a[i]->format, "%5t")) tc.push_back(YMOND);
		}
	    }
	    colors[i] = bg[a[i]->table];
	}
	if(index[i] == -1 && (n=strlen(a[i]->name)-2) > 0 &&
		stringToInt(a[i]->name+n, &aux_pos[i]))
	{
	    for(j = 0; j < num_aux_members &&
			strncmp(aux_des[j].name, a[i]->name, n); j++);
	    if(j < num_aux_members)
	    {
		index[i] = j;
		if(aux_des[j].type == CSS_TIME)
		{
		    if(!strcmp(a[i]->format, "%t"))      tc.push_back(YMONDHMS);
                    else if(!strcmp(a[i]->format, "%2t"))tc.push_back(YMONDHMS3);
                    else if(!strcmp(a[i]->format, "%3t")) tc.push_back(GSE20);
                    else if(!strcmp(a[i]->format, "%4t")) tc.push_back(GSE21);
                    else if(!strcmp(a[i]->format, "%5t")) tc.push_back(YMOND);
		}
	    }
	}
	if(index[i] == -1)
	{
	    // extra non-table member
	    if(!strcmp(a[i]->format, "%t"))       tc.push_back(YMONDHMS);
	    else if(!strcmp(a[i]->format, "%2t")) tc.push_back(YMONDHMS3);
	    else if(!strcmp(a[i]->format, "%3t")) tc.push_back(GSE20);
	    else if(!strcmp(a[i]->format, "%4t")) tc.push_back(GSE21);
	    else if(!strcmp(a[i]->format, "%5t")) tc.push_back(YMOND);
	}
	if(!(row[i] = (char *)mallocWarn(maxlen))) return;

	if((int)tc.size() <= i) tc.push_back(NOT_TIME);
    }

    for(int l = 0; l < records.size(); l++)
    {
	o = records[l];
	CssTableClass *css;

	for(i = 0; i < num_columns; i++)
	{
	    value = (char *)row[i];
	    stringcpy(value, "na", maxlen);
	    j = index[i];

	    if(j >= 0) {
		if(oi[i] >= 0) {
		    k = oi[i];
		    if(k >= 0 && k < o->num_tables) {
			css = o->tables[k];
			getTok(value, maxlen, j, num_members[k], des[k],
				a[i]->format, css);
		    }
		}
		else if(aux_pos[i] >= 0) {
		    k = aux_pos[i];
		    if(k < o->aux.size()) {
			css = o->aux[k];
			getTok(value, maxlen, j, num_aux_members, des[k],
				a[i]->format, css);
		    }
		}
	    }
	    else if(o->num_tables > 0)
	    {
		css = o->tables[0];
		if(getExtra(l, o, a[i], value, maxlen)) {
                }
		else if(!strcmp(a[i]->name, "file")) {
		    stringcpy(value, quarkToString(css->getFile()), maxlen);
		}
		else if(!strcmp(a[i]->name, "file_index")) {
		    int ndx = css->filePosition();
		    snprintf(value, maxlen, "%d", ndx);
		}
		else if(css->getValue(a[i]->name, sval)) {
		    stringcpy(value, sval.c_str(), maxlen);
		}
		else {
		    stringcpy(value, "na", maxlen);
		}
	    }
            value[maxlen-1] = '\0';
        }
        addRow(row, False);
    }
    for(i = 0; i < num_columns; i++) free((char *)row[i]);

    setColumnTime(tc);
    setColumnColors(colors);

    adjustColumns();
    setSelected();

    Free(index);
    Free(oi);
    Free(row);
    Free(aux_pos);
    Free(colors);
}

void MultiTable::list(int record_no)
{
    int i, j, k, n, num_columns, maxlen=MAXPATHLEN+1;
    int *index=NULL, *oi=NULL, *aux_pos=NULL;
    const char **row=NULL;
    char *value;
    string sval;
    TableRow *o;
    TAttribute **a;
    CssTableClass *css;

    if(record_no < 0 || record_no >= (int)records.size()) {
        return;
    }

    o = records[record_no];
    if(o->num_tables > num_types || o->aux.size() > num_aux)
    {
	const char *c[20];
	string ax_type;
	
	n = 0;
	for(i = 0; i < o->num_tables && n < 20; i++) {
	    c[n++] = o->tables[i]->getName();
	}
	if(o->aux.size() > 0) {
	    ax_type.assign(o->aux[0]->getName());
	}
	    
	setType(n, c, o->aux.size(), ax_type, 0, NULL, NULL, "");
    }

    table_attributes->displayAttributes(&a);

    num_columns = numColumns();

    if(!(index = (int *)mallocWarn(num_columns*sizeof(int)))) return;
    if(!(oi = (int *)mallocWarn(num_columns*sizeof(int)))) return;
    if(!(row = (const char **)mallocWarn(num_columns*sizeof(char *)))) return;
    if(!(aux_pos = (int *)mallocWarn(num_columns*sizeof(int)))) return;

    for(i = 0; i < num_columns; i++)
    {
	index[i] = -1;
	aux_pos[i] = -1;
	oi[i] = -1;
	if(a[i]->table >= 0 && a[i]->table < num_types)
	{
	    k = a[i]->table;
	    for(j = 0; j < num_members[k] && 
		strcmp(des[k][j].name, a[i]->name); j++);
	    if(j < num_members[k])
	    {
		oi[i] = k;
		index[i] = j;
	    }
	}
	if(index[i] == -1 && (n=strlen(a[i]->name)-2) > 0 &&
		stringToInt(a[i]->name+n, &aux_pos[i]))
	{
	    for(j = 0; j < num_aux_members &&
			strncmp(aux_des[j].name, a[i]->name, n); j++);
	    if(j < num_aux_members)
	    {
		index[i] = j;
	    }
	}
	if(!(row[i] = (char *)mallocWarn(maxlen))) return;
    }

    for(i = 0; i < num_columns; i++)
    {
	value = (char *)row[i];
	stringcpy(value, "na", maxlen);
	j = index[i];

	if(j >= 0) {
	    if(oi[i] >= 0) {
		k = oi[i];
		if(k >= 0 && k < o->num_tables) {
		    css = o->tables[k];
		    getTok(value, maxlen, j, num_members[k], des[k],
				a[i]->format, css);
		}
	    }
	    else if(aux_pos[i] >= 0) {
		k = aux_pos[i];
		if(k < o->aux.size()) {
		    getTok(value, maxlen, j, num_aux_members, des[k],
				a[i]->format, o->aux[k]);
		}
	    }
	}
	else if(o->num_tables > 0)
	{
	    css = o->tables[0];
	    if(getExtra(record_no, o, a[i], value, maxlen)) {
	    }
	    else if(!strcmp(a[i]->name, "file")) {
		stringcpy(value, quarkToString(css->getFile()), maxlen);
	    }
	    else if(!strcmp(a[i]->name, "file_index")) {
		int ndx = css->filePosition();
		snprintf(value, maxlen, "%d", ndx);
	    }
	    else if(css->getValue(a[i]->name, sval)) {
		stringcpy(value, sval.c_str(), maxlen);
	    }
	    else {
		stringcpy(value, "na", maxlen);
	    }
	}
        value[maxlen-1] = '\0';
    }
    setRow(record_no, row);

    for(i = 0; i < num_columns; i++) free((char *)row[i]);

    adjustColumns();
//    setSelected();

    Free(index);
    Free(oi);
    Free(row);
    Free(aux_pos);
}

void MultiTable::getTok(char *value, int maxlen, int j, int n_members,
			CssClassDescription *d, char *format, CssTableClass *css)
{
    if(j >= 0 && j < n_members)
    {
	double t;
	char *member_address = (char *)css + d[j].offset;

	switch(d[j].type)
	{
	    case CSS_STRING:
		stringcpy(value, member_address, maxlen);
		break;
	    case CSS_QUARK:
		stringcpy(value, quarkToString(*(int *)member_address), maxlen);
		break;
	    case CSS_DATE:
	    case CSS_LDDATE:
		stringcpy(value, timeDateString(
			(DateTime *)member_address), maxlen);
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		snprintf(value, maxlen, format, *(long *)member_address);
		break;
	    case CSS_INT:
		snprintf(value, maxlen, format, *(int *)member_address);
		break;
	    case CSS_DOUBLE:
		snprintf(value, maxlen, format, *(double *)member_address);
		break;
	    case CSS_FLOAT:
		snprintf(value, maxlen, format, *(float *)member_address);
		break;
	    case CSS_TIME:
		t = *(double *)member_address;
		if(!strcmp(format, "%t")) {
		    timeEpochToString(t, value, maxlen, YMONDHMS);
		}
		else if(!strcmp(format, "%2t")) {
		    timeEpochToString(t, value, maxlen, YMONDHMS3);
		}
		else if(!strcmp(format, "%3t")) {
		    timeEpochToString(t, value, maxlen, GSE20);
		}
		else if(!strcmp(format, "%4t")) {
		    timeEpochToString(t, value, maxlen, GSE21);
		}
		else if(!strcmp(format, "%5t")) {
		    timeEpochToString(t, value, maxlen, YMOND);
		}
		else {
		    snprintf(value, maxlen, format, t);
		}
		break;
	    default:
		stringcpy(value, "na", maxlen);
		break;
	}
    }
}

void MultiTable::setSelected(void)
{
    vector<bool> states;

    if(!data_source) return;

    for(int i = 0; i < records.size(); i++)
    {
	if(records[i]->num_tables > 0 &&
	    data_source->isSelected(records[i]->tables[0])) {
		states.push_back(true);
	}
	else {
	    states.push_back(false);
	}
    }

    setRowStates(states);
}

int MultiTable::getRecords(gvector<TableRow*> &v)
{
    v.clear();
    for(int i = 0; i < records.size(); i++) {
	v.add(records[i]);
    }
    return v.size();
}

int MultiTable::getSelectedRecords(gvector<TableRow *> &v)
{
    vector<int> rows;

    getSelectedRows(rows);

    v.clear();
    for(int i = 0; i < (int)rows.size(); i++) {
	v.add(records[rows[i]]);
    }

    return v.size();
}

void MultiTable::removeAllRecords(void)
{
    removeAllRows();
    for(int i = 0; i < (int)records.size(); i++) {
	records[i]->removeListener(this);
    }
    records.clear();
}

void MultiTable::clear(void)
{
    Table::clear();

    for(int i = 0; i < (int)records.size(); i++) {
	records[i]->removeListener(this);
    }
    records.clear();
    
    for(int i = 0; i < num_types; i++) Free(table_types[i]);
    Free(table_types);
    num_types = 0;
}
