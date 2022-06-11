/** \file CSSTable.cpp
 *  \brief Defines class CSSTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include <X11/Xmu/Xmu.h>

#include "CSSTable.h"
#include "widget/TableAttributes.h"
#include "BasicSource.h"
#include "motif++/MotifClasses.h"
#include "widget/TableListener.h"
#include "gobject++/CssTables.h"
extern "C" {
#include "libstring.h"
}

class UndoTableChange : public UndoAction
{
    public:
	UndoTableChange(CSSTable *css_table) {
	    css = css_table;
	}
	~UndoTableChange() {
	}
	CSSTable *css;
	gvector<CssTableClass *> orig_table, new_table;

	bool undo(void) {
	    return css->undoTableChange(this);
	}
	void getLabel(string &label) {
	    if(orig_table.size() > 1) {
		label.assign("Undo Change Records");
	    }
	    else label.assign("Undo Change Record");
	}
};

CSSTable::CSSTable(const string &name, Component *parent, Arg *args, int n)
	: Table(name, parent, args, n), DataReceiver(NULL), Parser()
		
{
}

CSSTable::CSSTable(const string &name, Component *parent, InfoArea *infoarea,
	Arg *args, int n) : Table(name, parent, infoarea, args, n),
	DataReceiver(NULL), Parser()
		
{
}

CSSTable::CSSTable(const string &name, Component *parent, DataSource *ds,
                Arg *args, int n) : Table(name, parent, args, n),
		DataReceiver(ds), Parser()
		
{
}

CSSTable::CSSTable(const string &name, Component *parent, InfoArea *infoarea,
		DataSource *ds, Arg *args, int n) :
		Table(name, parent, infoarea, args, n), DataReceiver(ds),
		Parser()
		
{
}

CSSTable::~CSSTable(void)
{
    removeAllRecords();
}

void CSSTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    int horizontal_pos, vertical_pos;

    if(!strcmp(reason, CssTableChange)) {
	TableListenerCallback *tc =
		(TableListenerCallback *)action_event->getCalldata();
	int i;
	for(i = 0; i < records.size() && tc->table != records[i]; i++);
	if(i < records.size()) {
	    if( !strcmp(cmd, "delete") ) {
		records.removeAt(i);
		getScrolls(&horizontal_pos, &vertical_pos);
		removeRow(i);
		setScrolls(horizontal_pos, vertical_pos);
	    }
	    else {
		getScrolls(&horizontal_pos, &vertical_pos);
		list(i);
		setScrolls(horizontal_pos, vertical_pos);
	    }
	}
	return;
    }
    if(!strcmp(cmd, "Save")) {
	editSave();
	return;
    }

    Table::actionPerformed(action_event);

    if(!strcmp(cmd, "Apply Attributes")) {
	vector<int> row_order;
	getRowOrder(row_order);
	if((int)row_order.size() > 0) {
	    list();
	    setRowOrder(row_order);
	}
    }
}

void CSSTable::editModeOn(void)
{
    if(edit_mode) return;

    vector<int> row;
    getSelectedRows(row);
    if((int)row.size() == 0) return;

    const char *tname = records[0]->getName();
    for(int i = 0; i < (int)row.size(); i++) {
	CssTableClass *o1 = records[row[i]];
	CssTableClass *o2 = CssTableClass::createCssTable(tname);
	o1->copyTo(o2);
	backup_order.push_back(o1);
	backup_copy.push_back(o2);
    }
    Table::editModeOn();
}

void CSSTable::editCancel(void)
{
    int num, horizontal_pos, vertical_pos;
    vector<bool> edit;
    char **row_labels=NULL;

    num = getRowEditable(edit);

    row_labels = getRowLabels();

    for(int i = 0; i < num; i++) if(row_labels[i])
    {
	for(int j = 0; j < backup_order.size(); j++) {
	    if(backup_order[j] == records[i]) {
		CssTableClass *o2 = records[i];
		CssTableClass *o1 = backup_copy[j];
		o1->copyTo(o2);
		TableListener::doCallbacks(o2, this, "edit");
	    }
	}
    }
    backup_order.clear();
    backup_copy.clear();
    Free(row_labels);

    getScrolls(&horizontal_pos, &vertical_pos);

    Table::editCancel();

//    listKeepOrder();

    setRowStates(edit);

    setScrolls(horizontal_pos, vertical_pos);
}

void CSSTable::editSave(void)
{
    vector<bool> edit;
    vector<int> row_order, col_order;
    int horizontal_pos, vertical_pos;

    getScrolls(&horizontal_pos, &vertical_pos);
    getRowOrder(row_order);
    getColumnOrder(col_order);

    Table::editSave();

    UndoTableChange *undo = new UndoTableChange(this);

    for(int i = 0; i < (int)edit.size(); i++) if(edit[i])
    {
	for(int j = 0; j < backup_order.size(); j++) {
	    if(backup_order[j] == records[i]) {
		CssTableClass *o = records[i];
		CssTableClass *orig = backup_copy[j];

		DataSource *ds = o->getDataSource();
		if(ds) {
		    if( !o->strictlyEquals(*orig) ) { // strict comparison
			ds->changeTable(orig, o);
			undo->orig_table.push_back(orig);
			undo->new_table.push_back(o);
			TableListener::doCallbacks(orig, o, this);
		    }
		}
		break;
	    }
	}
    }

    if(undo->orig_table.size() == 0) {
	delete undo;
    }
    else {
	Application::getApplication()->addUndoAction(undo);
    }

    backup_order.clear();
    backup_copy.clear();

//    listKeepOrder();

    setRowStates(edit);

    setRowOrder(row_order);
    setColumnOrder(col_order);

    setScrolls(horizontal_pos, vertical_pos);
}

bool CSSTable::undoTableChange(UndoTableChange *undo)
{
    for(int i = 0; i < undo->orig_table.size(); i++)
    {
	if( BasicSource::undoFileModification() ) {
	    CssTableClass *o1 = undo->orig_table[i];
	    CssTableClass *o2 = undo->new_table[i];
	    o1->copyTo(o2);
	}
    }
    return true;
}

void CSSTable::modifyVerify(Widget w, XtPointer calldata)
{
    MmTableEditCallbackStruct *c = (MmTableEditCallbackStruct *)calldata;
    if(c->row < 0 || c->row >= numRows()) {
	cerr << "CSSTable.verify: row=" <<c->row<<" num_rows=" << numRows() <<endl;
        return;
    }
    CssTableClass *o = records[c->row];

    TAttribute a = getAttribute(c->column);
    int css_column = a.order;

    if(!o->setMember(css_column, c->new_string)) {
        showWarning("CSSTable: %s", CssTableClass::getError());
        restore();
    }
    backup();
}

void CSSTable::setField(int row, int col, const string &s, bool redisplay)
{
    vector<bool> edit;
    int num = getRowEditable(edit);
    if(row < 0 || row >= num) {
	cerr << "CSSTable.setField: row=" << row <<" num_rows=" << num << endl;
        return;
    }
    bool row_editable = edit[row];
    if( !row_editable ) {
	fprintf(stderr, "CSSTable: attempt to setField of non-editable row.\n");
	return;
    }
    CssTableClass *o = records[row];

    TAttribute a = getAttribute(col);
    int css_column = a.order;

    if(!o->setMember(css_column, s)) {
        showWarning("CSSTable: %s", CssTableClass::getError());
        restore();
    }
    else {
	Table::setField(row, col, s, redisplay);
    }
    backup();
}

void CSSTable::setFieldWithCB(int row, int col, const string &s, bool redisplay)
{
    vector<bool> edit;
    int num = getRowEditable(edit);
    if(row < 0 || row >= num) {
	cerr << "CSSTable.setField: row=" << row <<" num_rows=" << num << endl;
        return;
    }
    bool row_editable = edit[row];
    if( !row_editable ) {
	fprintf(stderr, "CSSTable: attempt to setField of non-editable row.\n");
	return;
    }
    CssTableClass *o = records[row];

    TAttribute a = getAttribute(col);
    int css_column = a.order;

    if(!o->setMember(css_column, s)) {
        showWarning("CSSTable: %s", CssTableClass::getError());
        restore();
    }
    else {
	Table::setFieldWithCB(row, col, s, redisplay);
    }
    backup();
}

void CSSTable::setFields(int n, int *rows, int *cols, const char **s)
{
    vector<bool> edit;
    CssTableClass *o;
    TAttribute a;
    int row, col, css_column;
    int num = getRowEditable(edit);

    for(int i = 0; i < n; i++)
    {
	row = rows[i];
	col = cols[i];
	if(row < 0 || row >= num) {
	    cerr << "CSSTable.setField: row=" << row <<" num_rows="
			<< num << endl;
	    continue;
	}
	if( !edit[row] ) {
	    cerr << "CSSTable: attempt to setField of non-editable row." <<endl;
	    continue;
	}
	o = records[row];

	a = getAttribute(col);
	css_column = a.order;

	if(!o->setMember(css_column, s[i])) {
	    showWarning("CSSTable: %s", CssTableClass::getError());
	    restore();
	}
	else {
	    Table::setField(row, col, s[i], true);
	}
	backup();
    }
}

void CSSTable::setType(const string &cssTableName)
{
    if(!display_attributes.empty()) {
	setType(cssTableName, 0, NULL, NULL, display_attributes);
    }
    else {
	setType(cssTableName, 0, NULL, NULL, "");
    }
}

void CSSTable::setType(const string &cssTableName, int num_extra,
		const char **extra, const char **extra_formats)
{
    if(!display_attributes.empty()) {
	setType(cssTableName, num_extra, extra, extra_formats,
		display_attributes);
    }
    else {
	setType(cssTableName, num_extra, extra, extra_formats, "");
    }
}

void CSSTable::setType(const string &cssTableName, int num_extra,
	const char **extra, const char **extra_formats,
	const string &display_list)
{
    char *name;
    int len;
    if(table_attributes) {
	table_attributes->setVisible(false);
	table_attributes->destroy();
    }
    CssClassDescription *des;
    int num_members = CssTableClass::getDescription(cssTableName, &des);

    if(num_members > 0) {
	table_type = cssTableName;
	len = (int)cssTableName.length() + strlen(" Attributes") + 1;
	name = (char *)malloc(len);
	snprintf(name, len, "%s Attributes", cssTableName.c_str());
	table_attributes = new TableAttributes(name, this, num_members,
			des, num_extra, extra, extra_formats, display_list);
	free(name);
	table_attributes->addActionListener(this);
	table_attributes->setCommandString("Apply Attributes");
	setColumns();
    }
}

void CSSTable::list(const string &cssTableName)
{
    if(table_type.compare(cssTableName)) {
        setType(cssTableName);
    }
    if(!data_source) {
        cerr << "CSSTable: list() called with NULL data_source." << endl;
        return;
    }

    gvector<CssTableClass *> v;
    data_source->getTable(cssTableName, v);
    removeAllRows();
    if(v.size() > 0) {
	addRecords(v);
    }
}

void CSSTable::addRecords(gvector<CssTableClass *> &v)
{
    if(!v.size()) return;

    if(!v[0]->nameIs(table_type)) {
        setType(v[0]->getName());
    }
    for(int i = 0; i < v.size(); i++) {
        records.push_back(v[i]);
	TableListener::addListener(v[i], this);
    }
    list();
}

void CSSTable::addRecords(int num, CssTableClass **t)
{
    if(num <= 0) return;
    if(!t[0]->nameIs(table_type)) {
        setType(t[0]->getName());
    }
    for(int i = 0; i < num; i++) {
        records.push_back(t[i]);
	TableListener::addListener(t[i], this);
    }
    list();
}

void CSSTable::insertRecords(int index, gvector<CssTableClass *> &v)
{
    if(!v.size()) return;

    if(!v[0]->nameIs(table_type)) {
        setType(v[0]->getName());
    }
    for(int i = 0; i < v.size(); i++) {
        records.insert(v[i], index++);
	TableListener::addListener(v[i], this);
    }
    list();
}

void CSSTable::insertRecords(int index, int num, CssTableClass **t)
{
    if(num <= 0) return;
    if(!t[0]->nameIs(table_type)) {
        setType(t[0]->getName());
    }
    for(int i = 0; i < num; i++) {
        records.insert(t[i], index++);
	TableListener::addListener(t[i], this);
    }
    list();
}

void CSSTable::setRecord(int index, CssTableClass *css, bool redisplay)
{
    if(index < 0 || index >= records.size()) {
	cerr << "CSSTable::setRecord: bad record index: " << index << endl;
	return;
    }
    if(css->getType() != records[index]->getType()) {
	cerr << "CSSTable::setRecord: bad record type." << endl;
	return;
    }
    TableListener::removeListener(records[index], this);
    records.set(css, index);
    TableListener::addListener(css, this);

    if(redisplay) listKeepOrder();
}

void CSSTable::addRecord(CssTableClass *css, bool redisplay)
{
    records.push_back(css);
    TableListener::addListener(css, this);
    if(redisplay) list();
}

void CSSTable::insertRecord(int index, CssTableClass *css, bool redisplay)
{
    records.insert(css, index);
    TableListener::addListener(css, this);
    if(redisplay) list();
}

void CSSTable::removeRecords(vector<int> &rows)
{
    removeRows(rows);
    for(int i = (int)rows.size()-1; i >= 0; i--) {
	if(rows[i] >= 0 && rows[i] < records.size()) {
	    TableListener::removeListener(records[rows[i]], this);
	}
	records.removeAt(rows[i]);
    }
}

void CSSTable::removeSelectedFromDB(void)
{
    DataSource *ds;
    bool found_one = false;
    gvector<CssTableClass *> v;

    getSelectedRecords(v);

    for(int i = 0; i < v.size(); i++) {
	CssTableClass *t = v[i];
	if(( ds = t->getDataSource()) )
	{
	    found_one = true;
	    TableListener::removeListener(t, this);
	    ds->deleteTable(this, t);
	    records.remove(t);
	}
	else {
	    cerr <<
	    "CSSTable: attempting to remove from file/db record with no source."
		<< endl;
	}
    }
    if(found_one) {
	listKeepOrder();
    }
}

void CSSTable::listKeepOrder(void)
{
    vector<int> row_order, col_order;

    getRowOrder(row_order);
    getColumnOrder(col_order);

    list();

    setRowOrder(row_order);
    setColumnOrder(col_order);
}

void CSSTable::list(void)
{
    int i, j, k, num_columns, maxlen=200;
    int num_members;
    int *index;
    vector<enum TimeFormat> tc;
    const char **row = NULL;
    char *value;
    string sval;
    double time;
    CssTableClass *o;
    CssClassDescription *des;
    TAttribute **a;
    string svalue;

    removeAllRows();

    if(records.size() <= 0) {
//8/31        clear();
        return;
    }

    o = records[0];

    num_members =  o->getNumMembers();
    des = o->description();

    if(!numColumns() || !o->nameIs(table_type)) {
	setType(o->getName());
    }

    table_attributes->displayAttributes(&a);

    num_columns = numColumns();

    index = new int[num_columns];
    row = new const char * [num_columns];

    for(i = 0; i < num_columns; i++)
    {
	index[i] = -1;
	for(j = 0; j < num_members && strcmp(des[j].name, a[i]->name); j++);
	if(j < num_members)
	{
	    index[i] = j;
            if(des[j].type == CSS_TIME)
            {
                if(!strcmp(a[i]->format, "%t"))	      tc.push_back(YMONDHMS);
                else if(!strcmp(a[i]->format, "%2t")) tc.push_back(YMONDHMS3);
                else if(!strcmp(a[i]->format, "%3t")) tc.push_back(GSE20);
                else if(!strcmp(a[i]->format, "%4t")) tc.push_back(GSE21);
                else if(!strcmp(a[i]->format, "%5t")) tc.push_back(YMOND);
            }
        }
	else {
	    if(!strcmp(a[i]->format, "%t"))	  tc.push_back(YMONDHMS);
	    else if(!strcmp(a[i]->format, "%2t")) tc.push_back(YMONDHMS3);
	    else if(!strcmp(a[i]->format, "%3t")) tc.push_back(GSE20);
	    else if(!strcmp(a[i]->format, "%4t")) tc.push_back(GSE21);
	    else if(!strcmp(a[i]->format, "%5t")) tc.push_back(YMOND);
	}
        if(!(row[i] = (char *)mallocWarn(maxlen))) return;

	if((int)tc.size() <= i) tc.push_back(NOT_TIME);
    }

    for(k = 0; k < records.size(); k++)
    {
	o = records[k];

	for(i = 0; i < num_columns; i++)
	{
	    value = (char *)row[i];
	    memset((void *)value, 0, maxlen);
	    j = index[i];
	    if(j >= 0 && j < num_members)
	    {
                char *member_address = (char *)o + des[j].offset;
                char *format = a[i]->format;

                switch(des[j].type)
                {
		    case CSS_STRING:
			stringcpy(value, member_address, maxlen);
			break;
		    case CSS_QUARK:
			stringcpy(value,
				quarkToString(*(int *)member_address), maxlen);
			break;
		    case CSS_DATE:
		    case CSS_LDDATE:
			stringcpy(value, timeDateString(
				(DateTime *)member_address), maxlen);
			break;
		    case CSS_LONG:
		    case CSS_JDATE:
			snprintf(value, maxlen, format,*(long *)member_address);
			break;
		    case CSS_INT:
			snprintf(value, maxlen, format, *(int *)member_address);
			break;
		    case CSS_DOUBLE:
			snprintf(value, maxlen, format,
					*(double *)member_address);
			    break;
		    case CSS_FLOAT:
			snprintf(value, maxlen,format,*(float *)member_address);
			    break;
		    case CSS_TIME:
			time = *(double *)member_address;
			if(!strcmp(format, "%t")) {
			    timeEpochToString(time, value, maxlen,YMONDHMS);
			}
			else if(!strcmp(format, "%2t")) {
			    timeEpochToString(time, value,maxlen,YMONDHMS3);
			}
			else if(!strcmp(format, "%3t")) {
			    timeEpochToString(time, value, maxlen, GSE20);
			}
			else if(!strcmp(format, "%4t")) {
			    timeEpochToString(time, value, maxlen, GSE21);
			}
			else if(!strcmp(format, "%5t")) {
			    timeEpochToString(time, value, maxlen, YMOND);
			}
			else {
			    snprintf(value, maxlen, format, time);
			}
			break;
		    default:
			stringcpy(value, "na", maxlen);
		}
	    }
	    else if(getExtra(k, o, a[i], sval)) {
		stringcpy(value, sval.c_str(), maxlen);
	    }
	    else if(!strcmp(a[i]->name, "file")) {
		snprintf(value, maxlen, "%s", quarkToString(o->getFile()) );
	    }
	    else if(!strcmp(a[i]->name, "file_index")) {
		int ndx = o->getFileOffset()/(o->getLineLength()+1);
		snprintf(value, maxlen, "%d", ndx);
	    }
	    else if(o->getValue(a[i]->name, svalue)) {
		stringcpy(value, svalue.c_str(), maxlen);
	    }
	    else {
		stringcpy(value, "na", maxlen);
	    }
            value[maxlen-1] = '\0';
        }
        addRow(row, false);
    }
    for(i = 0; i < num_columns; i++) free((char *)row[i]);

    setColumnTime(tc);
    adjustColumns();

    delete [] row;
    delete [] index;

    char text[200];
    snprintf(text, sizeof(text), "%d rows,  %d columns",numRows(),numColumns());
    info(text);
}

#ifdef HAVE_SPRINTF_RET_CHAR
#define ChkSprintf(str,len,spec,val)   strlen(snprintf(str,len,spec,val))
#else
#define ChkSprintf(str,len,spec,val)   snprintf(str,len,spec,val)
#endif

bool CSSTable::getExtra(int i, CssTableClass *o, TAttribute *a, string &value)
{
    char s[1000];
    string sval;
    long l = -1;
    double d = 0.;

    if(o->getValue(a->name, sval)) {
	if(ChkSprintf(s, sizeof(s), a->format, sval.c_str()) < 1) {
	    value.assign(sval);
	}
	else {
	    value.assign(s);
	}
    }
    else if(o->getValue(a->name, &l)) {
	if(ChkSprintf(s, sizeof(s), a->format, l) < 1) {
	    snprintf(s, sizeof(s), "%ld", l);
	}
	value.assign(s);
    }
    else if(o->getValue(a->name, &d)) {
	if(!strcmp(a->format, "%t")) {
	    timeEpochToString(d, s, sizeof(s), YMONDHMS);
	}
	else if(!strcmp(a->format, "%T")) {
	    timeEpochToString(d, s, sizeof(s), YMONDHMS3);
	}
	else if(!strcmp(a->format, "%g")) {
	    timeEpochToString(d, s, sizeof(s), GSE20);
	}
	else if(!strcmp(a->format, "%G")) {
	    timeEpochToString(d, s, sizeof(s), GSE21);
	}
	else if(ChkSprintf(s, sizeof(s), a->format, d) < 1) {
	    snprintf(s, sizeof(s), "%.2f", d);
	}
	value.assign(s);
    }
    else {
	value.assign("-");
	return false;
    }
    return true;
}

void CSSTable::list(int record_no)
{
    int i, j, num_columns, maxlen=200;
    int num_members;
    int *index;
    const char **row = NULL;
    char *value;
    string sval;
    double time;
    CssTableClass *o;
    CssClassDescription *des;
    TAttribute **a;

    if(record_no < 0 || record_no >= records.size()) {
        return;
    }

    o = records[record_no];

    num_members =  o->getNumMembers();
    des = o->description();

    if(!numColumns() || !o->nameIs(table_type)) {
	setType(o->getName());
    }

    table_attributes->displayAttributes(&a);

    num_columns = numColumns();

    if(!(index = (int *)mallocWarn(num_columns*sizeof(int)))) return;
    if(!(row = (const char **)mallocWarn(num_columns*sizeof(char *)))) return;

    for(i = 0; i < num_columns; i++)
    {
	index[i] = -1;
	for(j = 0; j < num_members && strcmp(des[j].name, a[i]->name); j++);
	if(j < num_members)
	{
	    index[i] = j;
        }
        if(!(row[i] = (char *)mallocWarn(maxlen))) return;
    }

    for(i = 0; i < num_columns; i++)
    {
	value = (char *)row[i];
	memset((void *)value, 0, maxlen);
	j = index[i];
	if(j >= 0 && j < num_members)
	{
	    char *member_address = (char *)o + des[j].offset;
	    char *format = a[i]->format;

	    switch(des[j].type)
	    {
		case CSS_STRING:
		    stringcpy(value, member_address, maxlen);
		    break;
		case CSS_QUARK:
		    stringcpy(value, quarkToString(*(int *)member_address),
				maxlen);
			break;
		case CSS_DATE:
		case CSS_LDDATE:
		    stringcpy(value, timeDateString((DateTime *)member_address),
				maxlen);
		    break;
		case CSS_LONG:
		case CSS_JDATE:
		    snprintf(value, maxlen, format,*(long *)member_address);
		    break;
		case CSS_INT:
		    snprintf(value, maxlen, format, *(int *)member_address);
		    break;
		case CSS_DOUBLE:
		    snprintf(value, maxlen, format, *(double *)member_address);
		    break;
		case CSS_FLOAT:
		    snprintf(value, maxlen,format,*(float *)member_address);
		    break;
		case CSS_TIME:
		    time = *(double *)member_address;
		    if(!strcmp(format, "%t")) {
			timeEpochToString(time, value, maxlen,YMONDHMS);
		    }
		    else if(!strcmp(format, "%2t")) {
			timeEpochToString(time, value,maxlen,YMONDHMS3);
		    }
		    else if(!strcmp(format, "%3t")) {
			timeEpochToString(time, value, maxlen, GSE20);
		    }
		    else if(!strcmp(format, "%4t")) {
			timeEpochToString(time, value, maxlen, GSE21);
		    }
		    else if(!strcmp(format, "%5t")) {
			timeEpochToString(time, value, maxlen, YMOND);
		    }
		    else {
			snprintf(value, maxlen, format, time);
		    }
		    break;
		default:
		    stringcpy(value, "na", maxlen);
	    }
	}
	else if(getExtra(record_no, o, a[i], sval)) {
	    stringcpy(value, sval.c_str(), maxlen);
	}
	else if(!strcmp(a[i]->name, "file")) {
	    snprintf(value, maxlen, "%s", quarkToString(o->getFile()) );
	}
	else if(!strcmp(a[i]->name, "file_index")) {
	    int ndx = o->getFileOffset()/(o->getLineLength()+1);
	    snprintf(value, maxlen, "%d", ndx);
	}
	else if(o->getValue(a[i]->name, sval)) {
	    stringcpy(value, sval.c_str(), maxlen);
	}
	else {
	    stringcpy(value, "na", maxlen);
	}
        value[maxlen-1] = '\0';
    }
    setRow(record_no, row);

    for(i = 0; i < num_columns; i++) free((char *)row[i]);

    adjustColumns();

    Free(row);
    Free(index);
}

int CSSTable::getSelectedRecords(gvector<CssTableClass *> &v)
{
    vector<int> rows;

    v.clear();
    getSelectedRows(rows);
    for(int i = 0; i < (int)rows.size(); i++) v.add(records[rows[i]]);

    return v.size();
}

void CSSTable::removeAllRecords(void)
{
    removeAllRows();
    for(int i = 0; i < records.size(); i++) {
	TableListener::removeListener(records[i], this);
    }
    records.clear();
}

void CSSTable::clear(void)
{
    Table::clear();
    for(int i = 0; i < records.size(); i++) {
	TableListener::removeListener(records[i], this);
    }
    records.clear();
    table_type.clear();
}

bool CSSTable::selectRecord(CssTableClass *css, bool select, bool do_callback)
{
    int i;
    if( (i = records.indexOf(css)) >= 0) {
	if(do_callback) {
	    selectRowWithCB(i, select);
	}
	else {
	    selectRow(i, select);
	}
	return true;
    }
    return false;
}

void CSSTable::copy(TableSelectionType select_type, Time time)
{
    if(select_type == TABLE_COPY_COLUMNS ||
	select_type == TABLE_COPY_THIS_COLUMN)
    {
	Table::copy(select_type, time);
	return;
    }
    else if(select_type == TABLE_COPY_ROWS)
    {
	vector<int> rows;
	getSelectedRows(rows);
	gvector<CssTableClass *> *r = getRecords();

	copy_rows.clear();

	for(int i = 0; i < (int)rows.size(); i++) {
	    copy_rows.push_back(r->at(rows[i]));
	}
    }
    else if(select_type == TABLE_COPY_ALL)
    {
	vector<int> rows;
	getRowOrder(rows);
	gvector<CssTableClass *> *r = getRecords();

	copy_rows.clear();

	for(int i = 0; i < (int)rows.size(); i++) {
	    copy_rows.push_back(r->at(rows[i]));
	}
    }
    else {
	fprintf(stderr, "CSSTable.copy: unknown selection_type\n");
	return;
    }

    if(XtOwnSelection(baseWidget(), XA_PRIMARY, time,
		(XtConvertSelectionProc)convertSelection,
		(XtLoseSelectionProc)loseOwnership,
		(XtSelectionDoneProc)transferDone) == FALSE)
    {
	fprintf(stderr, "Table.copy: failed to become selection owner.\n");
    }
}

Boolean CSSTable::convertSelection(Widget w, Atom *selection, Atom *target,
		Atom *type, XtPointer *value, unsigned long *length,
		int *format)
{
    CSSTable *t = (CSSTable *)getTableFromWidget(w);
    if( !t ) {
        fprintf(stderr, "CSSTable.convertSelection: t == NULL! \n");
        return False;
    }
    Display *d = XtDisplay(w);

    *type = *target;
    *format = 8;

    if(*target == t->table_row_atom)
    {
	if(t->copy_rows.size() <= 0) return False;
	char *b=NULL, *p;
	int nbytes = t->copy_rows[0]->toBytes(&b);
	int n = t->copy_rows.size() * nbytes;
	*value = (XtPointer)mallocWarn(n*sizeof(unsigned char));
	p = (char *)*value;
	memcpy(p, b, nbytes);
	p += nbytes;
	Free(b);
	// assumes all CssTableClasss in copy_rows are the same type.
	for(int i = 1; i < t->copy_rows.size(); i++) {
	    t->copy_rows[i]->toBytes(&b);
	    memcpy(p, b, nbytes);
	    p += nbytes;
	    Free(b);
	}
	*length = t->copy_rows.size()*nbytes;
	return True;
    }
    else if(*target == XA_STRING || *target == XA_TEXT(d) ||
	    *target == XA_COMPOUND_TEXT(d))
    {
	if(t->copy_rows.size() == 0) return False;
	int line_len = t->copy_rows[0]->getLineLength();
	int n = t->copy_rows.size() * (line_len+1);
	*value = (XtPointer)mallocWarn((n+1)*sizeof(unsigned char));
	char *val = (char *)*value;
	int j = 0;
	for(int i = 0; i < t->copy_rows.size(); i++)
	{
	    char *s = t->copy_rows[i]->toString();
	    strncpy(val+j, s, line_len);
	    Free(s);
	    j += line_len;
	    if(i < t->copy_rows.size()-1) {
		val[j] = '\n';
		j++;
	    }
	}
	val[j] = '\0';
	*length = n;
	return True;
    }
   else {
	Boolean success = XmuConvertStandardSelection(w, CurrentTime, selection,
			target, type, (char **)value, length, format);

	if(success && *target == XA_TARGETS(d))
	{
	    Atom *tmp;
	    tmp = (Atom *) realloc(*value, (*length + 4) * sizeof(Atom));
	    tmp[(*length)++] = XInternAtom(XtDisplay(w), "MULTIPLE", False);
	    tmp[(*length)++] = t->table_row_atom;
	    tmp[(*length)++] = t->table_column_atom;
	    tmp[(*length)++] = XA_COMPOUND_TEXT(d);
	    *value = (XtPointer) tmp;
	}
	return success;
    }
}

void CSSTable::loseOwnership(Widget w, Atom *selection)
{
    CSSTable *t = (CSSTable *)getTableFromWidget(w);
    if( t ) {
	t->copy_rows.clear();
    }
}

void CSSTable::transferDone(Widget w, Atom *selection, Atom *target)
{
    /* Nothing to do here. Don't free, so the selection can be pasted
        more than once.
     */
}


void CSSTable::cut(TableSelectionType type, Time time)
{
}

void CSSTable::paste(Time paste_time)
{
    copy_time = paste_time;
    requested = table_row_atom;
    XtGetSelectionValue(baseWidget(), XA_PRIMARY, table_row_atom,
        (XtSelectionCallbackProc)requestorCallback, (XtPointer)this, copy_time);
}

void CSSTable::requestorCallback(Widget w, XtPointer p, Atom *selection,
        Atom *type, XtPointer value, unsigned long *length, int *format)
{
    CSSTable *t = (CSSTable *)p;
    if( !t ) return;

    if((*type == 0 /* XT_CONVERT_FAIL */) || (*length == 0)) {
	if(t->requested == t->table_row_atom)    // try requesting a string
	{
            t->requested = XA_STRING;
            XtGetSelectionValue(w, XA_PRIMARY, XA_STRING,
                (XtSelectionCallbackProc)requestorCallback, p, t->copy_time);
        }
        else {
            XBell(XtDisplay(w), 100);
            fprintf(stderr,
                "CSSTable: no selection or selection timed out.\n");
        }
        return;
    }
    else if(*type == t->table_row_atom)
    {
	if(*length < 2*sizeof(int)) {
	    Free(value);
	    return;
	}
	int nbytes, name, n;
	char *b = (char *)value;

	n = 0;
	memcpy(&nbytes, b, sizeof(int)); n += sizeof(int);
	memcpy(&name, b+n, sizeof(int));

	if(t->records.size() > 0) {
	    int q_name = stringToQuark(t->records[0]->getName());
	    if(name != q_name) {
		fprintf(stderr,"CSSTable.paste: wrong type of table record.\n");
		Free(value);
		return;
	    }
	}
	// assumes all the table records in the byte stream are the same type.
	for(int i = 0; i < (int)*length; i += nbytes) {
	    CssTableClass *c = CssTableClass::fromBytes(nbytes, b+i);
	    t->addRecord(c, false);
	}
	t->list();
    }
    else if(*type == XA_STRING || *type == XA_COMPOUND_TEXT(XtDisplay(w)))
    {
	const char *name = NULL;
	if(t->records.size() > 0) {
	    name = t->records[0]->getName();
	}
	else if( !t->table_type.empty() ) {
	    name = t->table_type.c_str();
	}
	if(!name) {
	    fprintf(stderr, "CSSTable.paste: unknown table type.\n");
	}
	else {
	    CssTableClass *css = CssTableClass::createCssTable(name);
	    if(css) {
		char *c = (char *)value;
		int len = css->getLineLength() + 1;
		for(int i = 0; i < (int)(*length); i += len) {
		    if( readCssTable(c+i, css) ) {
			t->addRecord(css, false);
			css = CssTableClass::createCssTable(name);
		    }
		}
		delete css;
		t->list();
	    }
	}
    }

    Free(value);
}

bool CSSTable::readCssTable(char *line, CssTableClass *o)
{
    int num_members = o->getNumMembers();
    CssClassDescription *des = o->description();

    for(int i = 0; i < num_members; i++)
    {
	line[des[i].end] = '\0';
	stringTrim(&line[des[i].start-1]);

	if( !o->setMember(i, &line[des[i].start-1]) ) return false;
    }
    return true;
}

ParseCmd CSSTable::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if(parseArg(cmd, "select_record", c)) {
	int i;
	if(sscanf(c.c_str(), "%d", &i) == 1) {
	    gvector<CssTableClass *> *v = getRecords();
	    if(i > 0 && v && i <= v->size()) {
		selectRecord(v->at(i-1), true);
	    }
	    return COMMAND_PARSED;
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseString(cmd, "set_type", c)) {
	return parseSetType(c, msg);
    }
    else if(parseString(cmd, "add_record", c)) {
	return parseAddRecord(c, msg);
    }
    else if(parseString(cmd, "remove_record", c)) {
	return parseRemoveRecord(c, msg);
    }
    else if(parseCompare(cmd, "clear")) {
	clear();
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "list")) {
	list();
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "remove_all_records")) {
	removeAllRecords();
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "paste")) {
	paste(CurrentTime);
	return COMMAND_PARSED;
    }
    else if((ret = dataParseCmd(cmd.c_str(), msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else {
	return Table::parseCmd(cmd, msg);
    }
}

ParseCmd CSSTable::parseSetType(const string &s, string &msg)
{
    string table, ex, display_list;
    char **extra=NULL, **extra_formats=NULL, *a, *b, *tok, *last;
    int num=0;

    if( !parseGetArg(s, "table", table) ) {
	msg.assign("setType: missing table argument");
	return ARGUMENT_ERROR;
    }
    if( parseGetArg(s, "extra", ex) ) {
	char *c;
	extra = (char **)malloc(sizeof(char *));
	extra_formats = (char **)malloc(sizeof(char *));
	num = 0;
	c = strdup(ex.c_str());
	tok = c;
	while( (a = strtok_r(tok, ",", &last)) ) {
	    tok = NULL;
	    if( !(b = strtok_r(NULL, ",", &last)) ) {
		msg.assign(string("setType: missing format for extra ") + a);
		for(int i = 0; i < num; i++) {
		    free(extra[i]); free(extra_formats[i]);
		}
		free(extra);
		free(extra_formats);
		return ARGUMENT_ERROR;
	    }
	    extra = (char **)realloc(extra, (num+1)*sizeof(char *));
	    extra_formats = (char **)realloc(extra_formats,
					(num+1)*sizeof(char *));
	    extra[num] = strdup(a);
	    extra_formats[num] = strdup(b);
	    num++;
	}
	free(c);
    }
    parseGetArg(s, "display_list", display_list);

    setType(table, num, (const char **)extra, (const char **)extra_formats,
		display_list);

    for(int i = 0; i < num; i++) {
	free(extra[i]);
	free(extra_formats[i]);
    }
    free(extra);
    free(extra_formats);
    return COMMAND_PARSED;
}

ParseCmd CSSTable::parseAddRecord(const string &s, string &msg)
{
    long addr, l;
    double d;
    const char **table_names=NULL;
    string c, name, table;
    CssTableClass *css;
    int num_css = CssTableClass::getAllNames(&table_names);

    for(int i = 0; i < num_css; i++) {
	name.assign(string("_") + table_names[i] + "_");
	if(parseGetArg(s, name, &addr)) {
	    table.assign(table_names[i]);
	    break;
	}
    }
    free(table_names);

    if(table.empty()) {
	msg.assign("add_record: unrecognized object.");
	return ARGUMENT_ERROR;
    }
    css = (CssTableClass *)addr;

    if(!table_attributes) {
	CssClassDescription *des = css->description();
	int num_members = css->getNumMembers();
	if(num_members <= 0) {
	    msg.assign("add_record: unrecognized object.");
	    return ARGUMENT_ERROR;
	}
	name.assign(table + " Attributes");
	table_attributes = new TableAttributes(name, this, num_members, des);
    }

    int n = table_attributes->numAttributes();
    for(int i = 0; i < n; i++) {
	TAttribute ta = table_attributes->getAttribute(i);
	if(ta.table == -1 && parseGetArg(s, ta.name, c)) {
	    if(stringToLong(c.c_str(), &l)) {
		css->putValue(ta.name, l);
	    }
	    else if(stringToDouble(c.c_str(), &d)) {
		css->putValue(ta.name, d);
	    }
	    else {
		css->putValue(ta.name, c);
	    }
	}
    }
    addRecord(css, true);
    return COMMAND_PARSED;
}

ParseCmd CSSTable::parseRemoveRecord(const string &s, string &msg)
{
    string name, table;
    long addr;
    const char **table_names=NULL;
    CssTableClass *css;
    int i, num_css = CssTableClass::getAllNames(&table_names);

    for(i = 0; i < num_css; i++) {
	name.assign(string("_") + table_names[i] + "_");
	if(parseGetArg(s, name, &addr)) {
	    table.assign(table_names[i]);
	    break;
	}
    }
    free(table_names);

    if(table.empty()) {
	msg.assign("add_record: unrecognized object.");
	return ARGUMENT_ERROR;
    }
    css = (CssTableClass *)addr;

    for(i = 0; i < records.size() && css != records[i]; i++);
    if(i == records.size()) {
	msg.assign("remove_record: object not found");
	return ARGUMENT_ERROR;
    }

    vector<int> v;
    v.push_back(i);
    removeRows(v);

    TableListener::removeListener(records[i], this);
    records.removeAt(i);

    return COMMAND_PARSED;
}

ParseVar CSSTable::parseVar(const string &name, string &value)
{
    ParseVar ret;

    if(parseCompare(name, "num_selected")) {
	gvector<CssTableClass *> v;
	if(getSelectedRecords(v) > 0) {
	    ostringstream os;
	    os << v.size();
	    value.assign(os.str());
	}
	else {
	    value.assign("0");
	}
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "selected_records")) {
	gvector<CssTableClass *> v;
	int num = getSelectedRecords(v);
	return (num > 0) ? VARIABLE_TRUE : VARIABLE_FALSE;
    }
    else if(parseCompare(name, "getType")) {
	value.assign(getType());
	return STRING_RETURNED;
    }
    else if((ret = getTableLoop(name.c_str())) != VARIABLE_NOT_FOUND){
        return ret;
    }
    else if((ret = getTableMember(name.c_str(), value)) != VARIABLE_NOT_FOUND){
	return ret;
    }
    else if((ret = getTableCmd(name.c_str(), value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if(!strncmp(name.c_str(), "find_indices_", 13)) {
	gvector<CssTableClass *> *v = getRecords();
	if(v) {
	    return Parse::tableFindIndices(name, *v, value);
	}
	else {
	    return VARIABLE_NOT_FOUND;
	}
    }
    return Table::parseVar(name, value);
}
