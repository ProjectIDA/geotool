#ifndef _TABLE_VIEWER_H
#define _TABLE_VIEWER_H

#include "sys/param.h"
#include <pthread.h>
#include <semaphore.h>
#include <vector>

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "motif++/Menu.h"
#include "motif++/TextField.h"
#include "BasicSource.h"
#include "DataReceiver.h"
#include "widget/TabClass.h"
#include "FFDatabase.h"

#ifdef HAVE_LIBODBC
#include "FixBool.h"
#include "libgdb.h"
#endif
extern "C" {
#include "libstring.h"
#include "libtime.h"
#include "widget/Tab.h"
}

enum ConnectionType {
    NO_CONNECTION,
    ODBC_CONNECTION,
    FFDB_CONNECTION,
    PREFIX_CONNECTION
};

typedef struct
{
    ConnectionType connection_type;
#ifdef HAVE_LIBODBC
    SQLHDBC hdbc;
    string data_source;
    string user;
    string data_passwd;
#endif
    FFDatabase *ffdb;
    string prefix;
} ConnectionHandle;

class CSSTable;
class CssFileDialog;
class SelectOrder;
class Working;
class Import;
class AddStation;

class RecentPath : public Gobject
{
    public :
	RecentPath(const string &p) {
	    path = p;
	    button = NULL;
	}
	~RecentPath(void) { }
	string path;
	Button *button;
};

/**
 *  @ingroup libgx
 */
class TableViewer : public Frame, public BasicSource
{
    friend class Import;

    public:
	TableViewer(const string &name, Component *parent,
			const string &title="", bool independent=true);
	~TableViewer(void);

	virtual TableViewer *getTableViewerInstance(void) { return this; }
	void actionPerformed(ActionEvent *action_event);

	void addRecords(gvector<CssTableClass *> &records);
	void insertRecords(int index, gvector<CssTableClass *> &records);
	CSSTable *getCSSTable(const string &cssTableName);
	gvector<CssTableClass *> * getTableRecords(const string &cssTableName,
			bool warning=false);
	int getTableRecords(const string &cssTableName, gvector<CssTableClass *> &v,
			bool warning=false) {
		gvector<CssTableClass *> *t = getTableRecords(cssTableName, warning);
		v.clear();
		if (t) v.load(t);
		return v.size();
	}
	int getSelectedTableRecords(const string &tableName,
			gvector<CssTableClass *> &v, bool warning=false);

	void setQueryText(const string &tableName, const string &query);
        void saveQueryText(const string &tableName, const string &query);
	void appendQueryText(const string &tableName, const string &query);
	string getTabName(void);
	bool emptyTab(const string &cssTableName);
	void setWaveformReceiver(DataReceiver *dr) { waveform_receiver = dr; }
	void tabSetOnTop(const string &cssTableName);
	CSSTable *tableOnTop(void);
	void clearTab(const string &tableName);
	void clearAllTabs(void);
	void removeTableTab(const string &tableName);
	void removeAllTabs(void);
	void deleteSelectedRows(void);
	virtual void addRow(void);
        virtual CSSTable *addTableTab(const string &tableName);
        virtual CSSTable *addTableTab(const string &tableName,
			bool display_text_area);
        virtual void tabSelect(const string &tab_name);
	int getLabels(char ***labels) { return tab->getLabels(labels); }
	void setLabel(const string &old_label, const string &new_label) {
		tab->setLabel(old_label, new_label);
	}

	bool selectTableRecord(const string &tableName, CssTableClass *css,
		bool select);
	void displayAllTables(DataSource *ds);
	void displayTable(DataSource *ds, const string &tableName);

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseCmd parseSave(const string &cmd, const string &c, bool selected,
		string &msg);
	ParseVar parseVar(const string &name, string &value);
	static void parseHelp(const char *prefix);

	//  DataSource interface

	cvector<CssAffiliationClass> * getAffiliationTable(void) {
	    return (cvector<CssAffiliationClass> *)getTableRecords(cssAffiliation);
	}
	cvector<CssSiteClass> * getSiteTable(void) {
	    return (cvector<CssSiteClass> *)getTableRecords(cssSite);
	}
	cvector<CssSitechanClass> * getSitechanTable(void) {
	    return (cvector<CssSitechanClass> *)getTableRecords(cssSitechan);
	}

	int getTable(const string &cssTableName, gvector<CssTableClass *> &v);

	int getTable(cvector<CssAffiliationClass> &v) { return getTable(cssAffiliation, v); }
	int getTable(cvector<CssAmpdescriptClass> &v) { return getTable(cssAmpdescript, v); }
	int getTable(cvector<CssAmplitudeClass> &v) { return getTable(cssAmplitude, v); }
	int getTable(cvector<CssArrivalClass> &v) { return getTable(cssArrival, v); }
	int getTable(cvector<CssAssocClass> &v) { return getTable(cssAssoc, v); }
	int getTable(cvector<CssHydroFeaturesClass> &v) { return getTable(cssHydroFeatures, v); }
	int getTable(cvector<CssInfraFeaturesClass> &v) { return getTable(cssInfraFeatures, v); }
	int getTable(cvector<CssNetmagClass> &v) { return getTable(cssNetmag, v); }
	int getTable(cvector<CssOrigerrClass> &v) { return getTable(cssOrigerr, v); }
	int getTable(cvector<CssOriginClass> &v) { return getTable(cssOrigin, v); }
	int getTable(cvector<CssParrivalClass> &v) { return getTable(cssParrival, v); }
	int getTable(cvector<CssStamagClass> &v) { return getTable(cssStamag, v); }
	int getTable(cvector<CssStassocClass> &v) { return getTable(cssStassoc, v); }
	int getTable(cvector<CssSiteClass> &v) { return getTable(cssSite, v); }
	int getTable(cvector<CssSitechanClass> &v) { return getTable(cssSitechan, v); }
	int getTable(cvector<CssWfdiscClass> &v) { return getTable(cssWfdisc, v); }
	int getTable(cvector<CssWftagClass> &v) { return getTable(cssWftag, v); }
	int getTable(cvector<CssXtagClass> &v) { return getTable(cssXtag, v); }

	int getSelectedTable(const string &cssTableName, gvector<CssTableClass *> &v);

	int getSelectedTable(cvector<CssAffiliationClass> &v) { return getSelectedTable(cssAffiliation, v); }
	int getSelectedTable(cvector<CssAmpdescriptClass> &v) { return getSelectedTable(cssAmpdescript, v); }
	int getSelectedTable(cvector<CssAmplitudeClass> &v) { return getSelectedTable(cssAmplitude, v); }
	int getSelectedTable(cvector<CssArrivalClass> &v) { return getSelectedTable(cssArrival, v); }
	int getSelectedTable(cvector<CssAssocClass> &v) { return getSelectedTable(cssAssoc, v); }
	int getSelectedTable(cvector<CssHydroFeaturesClass> &v) { return getSelectedTable(cssHydroFeatures, v); }
	int getSelectedTable(cvector<CssInfraFeaturesClass> &v) { return getSelectedTable(cssInfraFeatures, v); }
	int getSelectedTable(cvector<CssNetmagClass> &v) { return getSelectedTable(cssNetmag, v); }
	int getSelectedTable(cvector<CssOrigerrClass> &v) { return getSelectedTable(cssOrigerr, v); }
	int getSelectedTable(cvector<CssOriginClass> &v) { return getSelectedTable(cssOrigin, v); }
	int getSelectedTable(cvector<CssParrivalClass> &v) { return getSelectedTable(cssParrival, v); }
	int getSelectedTable(cvector<CssStamagClass> &v) { return getSelectedTable(cssStamag, v); }
	int getSelectedTable(cvector<CssStassocClass> &v) { return getSelectedTable(cssStassoc, v); }
	int getSelectedTable(cvector<CssSiteClass> &v) { return getSelectedTable(cssSite, v); }
	int getSelectedTable(cvector<CssSitechanClass> &v) { return getSelectedTable(cssSitechan, v); }
	int getSelectedTable(cvector<CssWfdiscClass> &v) { return getSelectedTable(cssWfdisc, v); }
	int getSelectedTable(cvector<CssWftagClass> &v) { return getSelectedTable(cssWftag, v); }
	int getSelectedTable(cvector<CssXtagClass> &v) { return getSelectedTable(cssXtag, v); }

	void addDataListener(Component *comp) {
	    addActionListener(comp, XtNdataChangeCallback);
	}
	void removeDataListener(Component *comp) {
	    removeActionListener(comp, XtNdataChangeCallback);
	}
	void doDataChangeCallbacks(void) {
	    if(dataChange()) {
		doCallbacks(base_widget, (XtPointer)&change,
			XtNdataChangeCallback);
		resetDataChange(false);
	    }
        }
	// for Locate Event window. Don't remove any tables.
        void removeTable(CssTableClass *table, bool do_callback=true) {
	}

	void open(const string &file);
	void saveToFile(CSSTable *table);
	void saveAllToFile(void);
	void saveAll(const string &file, bool append);

	bool insertTable(CssTableClass *t, const string &tableName) {
	    return insertTable(&conn_handle, t, tableName);
	}
	bool writeTable(CssTableClass *t, const string &tableName) {
	    return writeTable(&conn_handle, t, tableName);
	}
	long getId(const string &lastid_table, const string &id_name) {
	    return getId(&conn_handle, lastid_table, id_name);
	}

	sem_t vi_sem;
	char vi_cmd[MAXPATHLEN+100];
	ConnectionHandle conn_handle;

	// File menu
	Button *open_button;
	Menu *recent_files_menu;
	Button *clear_recent_button;
	Button *import_button;
	Button *save_to_file_button;
	Button *save_all_button;
	Button *new_tv_button;
	Button *update_global_button;
	Separator *file_sep;
	Button *close_button, *quit_button;

	// Edit menu
	Button *add_row_button;
	Button *clear_button;
	Button *copy_rows_button;
	Button *copy_all_button;
	Button *copy_columns_button;
	Button *paste_button;
	Button *delete_button;
	Button *edit_button;
	Button *vi_edit_button;
	Button *vi_cmd_button;
	Button *remove_from_file_button;
	Button *remove_tab;
	Button *undo_edit_button;

	// View menu
	Button *attributes_button;
	Button *deselect_all_button;
	Button *select_all_button;
	Button *promote_rows_button;
	Button *promote_columns_button;
	Button *sort_button;
	Button *sort_selected_button;
	Button *sort_unique_button;
	Button *expand_button;
	Button *reverse_order_button;
	Button *show_all_button;
	Button *select_tables_button;

	// Option menu
	PlugIn *map_plugin;
	Button *add_station;

	Button *help_button;

    protected:
	bool display_text_area;

        TabClass *tab;

	typedef struct
	{
	    Form	*tab_form;
	    TextField	*query_text;
	    CSSTable	*table;
	    ArrowButton	*up_arrow;
	    ArrowButton	*dn_arrow;
	    vector<string *> *history;
	    int		history_index;
	} TabForm;
	int		num_tabs;
#define MAX_TABS 50
	TabForm		tab_forms[MAX_TABS];

        SelectOrder     *select_order_window;

	vector<TableViewer *> windows;

	gvector<RecentPath *> recent_input;

	typedef struct
	{
	    CSSTable	*table;
	    int		type;
	    int		row;
	    CssTableClass	*previous;
	    gvector<CssTableClass *>	*cut_records;
	    int		*cut_rows;
	    int		num_paste;
	    Boolean	saved;
	} UndoRecord;

	int num_undo;
	UndoRecord *undo_records;
	Atom table_row_atom;
	Atom requested;
	Time time;
	char vi_path[MAXPATHLEN+1];
	vector<bool> vi_states;
	vector<int> vi_rows;
	vector<int> vi_cols;

        pthread_t thread;
 	bool get_sem;

	CssFileDialog *open_file;

	Import *import_window;

	// set by Import
        vector<char *> table_names;
        vector<char *> member_names;
        vector<char *> id_names;
        vector<char *> unique_names;
        string ondate;
        string offdate;

	string tab_menu_label;
	PopupMenu *tab_popup;
	Button **tab_menu_buttons;
	int num_tab_menu_buttons;
/*
	Atom target_atom;
	Atom table_row_atom;
	bool requested_row;
*/
	static long last_arid;
	static long last_orid;
	static long last_ampid;

	DataReceiver *waveform_receiver;

	AddStation *add_station_window;

	void createInterface(void);
	void setFileMenu(void);
	void setEditMenu(void);
	void setViewMenu(void);
	void setOptionMenu(void);
	void setHelpMenu(void);

        void init(void);
        void createSelectTables(void);

	void selectTablesApply(void);
	void showWorking(int num, const char **labels);
	void closeWorking(void);
	bool updateWorking(const string &tableName, int num_records);
	void putTabProperty(void);
	void setButtonsSensitive(void);
	void setFileButtonsSensitive(void);
	void editTable(void);
	void copyOrCut(CSSTable *table, XmPushButtonCallbackStruct *c,bool cut);
	void paste(XmPushButtonCallbackStruct *c);
	void undoEdit(void);
	void open(void);
	void paste(Time time);
	void upArrow(CSSTable *table);
	void downArrow(CSSTable *table);
	void writeFile(CSSTable *table, const string &file, const string &access);
	void writeAllToFile(CSSTable *table, const string &file, const string &access);
	void viEdit(void);
	void finishViEdit(void);
	bool updateGlobalFile(void);
	void tabMenu(TabMenuCallbackStruct *c);
	PopupMenu *createTabPopup(void);

	virtual bool insertTable(ConnectionHandle *c, CssTableClass *t,
		const string &tableName);
	virtual bool writeTable(ConnectionHandle *c, CssTableClass *t,
		const string &tableName);
	virtual long getId(ConnectionHandle *c, const string &lastid_table,
			const string &id_name);
	long prefixGetNextId(const string &prefix, const string &lastid_table,
			const string &id_name);
	virtual bool updateTable(ConnectionHandle *c, const string &css_table_name,
			CssTableClass *told, CssTableClass *tnew);
	ParseCmd parseSelectRecord(const char *name, string &msg);
	void appendRecentInput(const string &path);
	void addRecentInput(RecentPath *r);
	void getRecentInput(void);
	void addRecentButton(RecentPath *r, int position);
	void clearRecentMenu(void);
	void saveRecentInput(void);

	static void checkViEdit(XtPointer data, XtIntervalId *id);

	//  DataSource interface
#ifdef __STDC__
	void ShowWarning(const char *format, ...);
	void PutWarning(const char *format, ...);
#else
	void ShowWarning(va_alist);
	void PutWarning(va_alist);
#endif


    private:
	Working *working;

	static void requestorCallback(Widget w, XtPointer p, Atom *selection,
		Atom *type, XtPointer value, unsigned long *length,int *format);
	static bool readCssTable(char *line, CssTableClass *o);

};

#endif
