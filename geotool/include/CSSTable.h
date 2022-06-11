#ifndef _CSSTABLE_H
#define _CSSTABLE_H

#include "widget/Table.h"
#include "gobject++/DataSource.h"
#include "DataReceiver.h"
#include "widget/Parser.h"
#include "gobject++/CssTableClass.h"
#include "gobject++/cvector.h"

class UndoTableChange;
class InfoArea;

/**
 *  @ingroup libgx
 */
class CSSTable : public Table, public DataReceiver, public Parser
{
    friend class UndoTableChange;

    public:

	CSSTable(const string &name, Component *parent, Arg *args=NULL,int n=0);
	CSSTable(const string &, Component *, InfoArea *, Arg *, int);
	CSSTable(const string &, Component *, DataSource *, Arg *, int);
	CSSTable(const string &, Component *, InfoArea *, DataSource *,
		Arg *, int);
	~CSSTable(void);

	void actionPerformed(ActionEvent *action_event);

	void setType(const string &cssTableName);
	void setType(const string &cssTableName, const string &display_list);
	void setType(const string &cssTableName, int num_extra,
		const char **extra, const char **extra_formats);
	void setType(const string &cssTableName, int num_extra,
		const char **extra, const char **extra_formats,
		const string &display_list);
        void list(const string &cssTableName);
        void addRecords(gvector<CssTableClass *> &v);
	void addRecords(int num, CssTableClass **t);
        void insertRecords(int index, gvector<CssTableClass *> &v);
	void insertRecords(int index, int num, CssTableClass **t);
	void addRecord(CssTableClass *css, bool redisplay);
	void setRecord(int index, CssTableClass *css, bool redisplay);
	void insertRecord(int index, CssTableClass *css, bool redisplay);
	void removeRecords(vector<int> &rows);
	void removeSelectedFromDB(void);
	void setField(int row, int col, const string &s,
		bool redisplay=true);
	void setFieldWithCB(int row, int col, const string &s,
		bool redisplay=true);
	void setFields(int num, int *rows, int *cols, const char **s);

        void list(void);
	void list(int record_no);
	void listKeepOrder(void);

	gvector<CssTableClass *> * getRecords(void) { return &records; }
	int getRecords(gvector<CssTableClass *> &v) {
	    v.clear();
	    v.load(&records);
	    return v.size();
	}
	int getSelectedRecords(gvector<CssTableClass *> &v);
	CssTableClass *getRecord(int i) {
	    return (i >= 0 && i < records.size()) ? records[i] : NULL;
	}
	bool selectRecord(CssTableClass *css, bool select, bool do_callback=false);

	void removeAllRecords(void);
	void clear(void);
	void editModeOn(void);
	void editCancel(void);
	void editSave(void);
	const char *getType(void) { return table_type.c_str(); }

	void copy(TableSelectionType selection_type, Time time);
	void cut(TableSelectionType selection_type, Time time);
	void paste(Time time);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd parseSetType(const string &s, string &msg);
	ParseCmd parseAddRecord(const string &s, string &msg);
	ParseCmd parseRemoveRecord(const string &s, string &msg);

	static bool readCssTable(char *line, CssTableClass *o);

    protected:
        string table_type;
        gvector<CssTableClass *> records;
	gvector<CssTableClass *> backup_order;
	gvector<CssTableClass *> backup_copy;
	gvector<CssTableClass *> copy_rows;

	void modifyVerify(Widget w, XtPointer calldata);
	bool undoTableChange(UndoTableChange *undo);

	virtual bool getExtra(int i, CssTableClass *, TAttribute *, string &);

    private:
	static Boolean convertSelection(Widget w, Atom *selection, Atom *target,
		Atom *type_return, XtPointer *value_return,
		unsigned long *length_return, int *format_return);
        static void loseOwnership(Widget w, Atom *selection);
        static void transferDone(Widget w, Atom *selection, Atom *target);
        static void requestorCallback(Widget w, XtPointer p, Atom *selection,
		Atom *type, XtPointer value, unsigned long *length,int *format);
};

template <class Type> class ctable : public CSSTable
{
    public:
	ctable(const string &name, Component *parent, Arg *args=NULL, int n=0) :
	    CSSTable(name, parent, args, n) { init(); }
	ctable(const string &name, Component *parent, InfoArea *ia, Arg *args,
		int n) : CSSTable(name, parent, ia, args, n) { init(); }
	ctable(const string &name, Component *parent, DataSource *ds, Arg *args,
		int n) : CSSTable(name, parent, ds, args, n) { init(); }
	ctable(const string &name, Component *parent, InfoArea *ia,
		DataSource *ds, Arg *args, int n) :
		CSSTable(name, parent, ia, ds, args, n) { init(); }
	~ctable(void) { }

	void init(void) {
	    Type r;
	    setType(r.getName());
	}

        void addRecords(cvector<Type> &v) {
	    gv.clear();
	    for(int i = 0; i < v.size(); i++) gv.push_back(v[i]);
	    CSSTable::addRecords(gv);
	}
	void addRecords(int num, Type **t) {
	    gv.clear();
	    for(int i = 0; i < num; i++) gv.push_back(t[i]);
	    CSSTable::addRecords(gv);
	}

        void insertRecords(int index, cvector<Type> &v) {
	    gv.clear();
	    for(int i = 0; i < v.size(); i++) gv.push_back(v[i]);
	    CSSTable::insertRecords(index, gv);
	}
	void insertRecords(int index, int num, Type **t) {
	    gv.clear();
	    for(int i = 0; i < num; i++) gv.push_back(t[i]);
	    CSSTable::insertRecords(index, gv);
	}

	cvector<Type> * getTableRecords(void) {
	    return (cvector<Type> *) &records;
	}
 	int getTableRecords(cvector<Type> &v) {
	    v.clear();
	    v.load((cvector<Type> *)&records);
	    return v.size();
	}
 	int getSelected(cvector<Type> &v) {
	    v.clear();
	    CSSTable::getSelectedRecords(sv);
	    return v.size();
	}
	Type * getRecord(int i) {
	    return (i >= 0 && i < records.size()) ? (Type *)records[i] : NULL;
	}

    protected:
	gvector<CssTableClass *> gv;
	gvector<CssTableClass *> sv;
};

#endif
