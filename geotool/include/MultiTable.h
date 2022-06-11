#ifndef _MULTI_TABLE_H
#define _MULTI_TABLE_H

#include "widget/Table.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"
#include "gobject++/gvector.h"
#include "gobject++/CssTableClass.h"

class MultiTable;

/**
 *  @ingroup libgx
 */
class TableRow : public Gobject
{
    friend class MultiTable;
    public:
	TableRow(CssTableClass *t1);
	TableRow(CssTableClass *t1, int num_aux, CssTableClass **aux);
	TableRow(CssTableClass *t1, CssTableClass *t2);
	TableRow(CssTableClass *t1, CssTableClass *t2, int num_aux, CssTableClass **aux);
	TableRow(CssTableClass *t1, CssTableClass *t2, CssTableClass *t3);
	TableRow(CssTableClass *t1, CssTableClass *t2, CssTableClass *t3,
		int num_aux, CssTableClass **aux);
	~TableRow(void);
	TableRow *copy(void);
	void copyTo(TableRow *o);
	void addTable(CssTableClass *t);
	void addListener(Component *comp);
	void removeListener(Component *comp);
	int numTables(void) { return num_tables; }

	CssTableClass *tables[3];

    protected:
	int num_tables;
	gvector<CssTableClass *> aux;
};

/**
 *  @ingroup libgx
 */
class MultiTableEdit : public Gobject
{
    public:
	MultiTableEdit(TableRow *oldrow, TableRow *newrow) {
	    old_row = oldrow;
	    new_row = newrow;
	}
	~MultiTableEdit(void){}
	TableRow *old_row, *new_row;
};

/**
 *  @ingroup libgx
 */
class MultiTable : public Table, public DataReceiver
{
    public:

	MultiTable(const string &name, Component *parent, Arg *args=NULL,
			int n=0);
	MultiTable(const string &, Component *, DataSource *, Arg *, int);
	~MultiTable(void);

	void actionPerformed(ActionEvent *action_event);

	void setType(const string &cssTableName);
	void setType(int num_tables, const char **cssTableNames);
	void setType(const string &cssTableName, const string &display_list);
	void setType(int num_tables, const char **cssTableNames,
		const string &display_list);
	void setType(int num_tables, const char **cssTableNames, int num_extra,
		const char **extra, const char **extra_formats);
	void setType(int num_tables, const char **cssTableNames, int num_extra,
		const char **extra, const char **extra_formats,
		const string &display_list);
	void setType(int num_tables, const char **cssTableNames, int num_aux,
		const string &auxTableName, int num_extra, const char **extra,
		const char **extra_formats, const string &display_list);
	void setAlternateNames(int num, char **alt_names);
        void list(const string &cssTableName);
	void listKeepOrder(void);
	void removeAllRecords(void);
        void addRecords(gvector<CssTableClass *> &v);
	void addRecord(CssTableClass *css, bool redisplay);
	void addRecord(TableRow *row, bool redisplay);
	void setRecord(int index, CssTableClass *css, bool redisplay);
	void insertRecord(int index, CssTableClass *css, bool redisplay);
	void removeRecords(vector<int> &rows);

        void list(void);
	void list(int record_no);
	int getRecords(gvector<TableRow *> &v);
	int getSelectedRecords(gvector<TableRow *> &v);
	TableRow *getRecord(int i) {
	    return (i>=0 && i < records.size()) ? (TableRow *)records[i] : NULL;
	}
//	bool selectRecord(CssTableClass css, bool select);
	int recordIndex(CssTableClass *css) {
	    for(int i = 0; i < records.size(); i++) {
		for(int j = 0; j < records[i]->num_tables; j++) {
		    if(css == records[i]->tables[j]) return i;
		}
	    }
	    return -1;
	}
	void clear(void);
	void editModeOn(void);
	void editCancel(void);
	void editSave(void);
	void setSelected(void);

    protected:
	int num_types;
	int num_aux;
	char **table_types;
	string aux_type;
	int num_alt_names;
	char **alt_names;
	CssClassDescription **des, *aux_des;
	int *num_members, num_aux_members;
	gvector<TableRow *> records;
	gvector<TableRow *> backup_copy;
	gvector<TableRow *> backup_order;

	void modifyVerify(Widget w, XtPointer calldata);
	void getTok(char *value, int maxlen, int j, int num_members,
			CssClassDescription *d, char *format, CssTableClass *css);
	virtual bool getExtra(int, TableRow *, TAttribute *, char *, int ) {
		return false; }
};

#endif
