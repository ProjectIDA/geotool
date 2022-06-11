#ifndef TABLE_H
#define TABLE_H

using namespace std;

#include <vector>
#include "motif++/Component.h"
#include "motif++/MotifDecs.h"
#include "widget/TableAttributes.h"
#include "widget/MmTable.h"
#include "widget/TCanvas.h"
#include "widget/HCanvas.h"

#define XtNeditSaveCallback (char *)"editSaveCallback"
#define XtNeditCancelCallback (char *)"editCancelCallback"
#define XtNattributeChangeCallback (char *)"attributeChangeCallback"

class InfoArea;

/** This class is the interface to the MmTable Widget.
 *  @ingroup libwgets
 */
class Table : public Component
{
    public:

	Table(const string &, Component *, Arg *args=NULL, int num=0);
	Table(const string &, Component *, InfoArea *, Arg *, int);
	~Table(void);

	void actionPerformed(ActionEvent *action_event);

	virtual Table *getTableInstance(void) { return this; }

	void destroy(void);
	void adjustHScroll(void) { MmTableAdjustHScroll(tw);}
	void addRow(vector<char *> &row, bool redraw);
	void addRow(vector<const char *> &row, bool redraw);
	void addRow(vector<string> &row, bool redraw);
	void addRowWithLabel(const char **row, const char *label, bool redraw);
	void addRow(const char **row, bool redraw) {
		addRowWithLabel(row, NULL, redraw);
	}
	void addRowWithLabel(const char **row, const string &label,bool redraw){
		addRowWithLabel(row, label.c_str(), redraw);
	}
	int getRowHeight(void);
	void removeRow(int i);
	void removeRows(int *rows, int num) {
	    vector<int> r;
	    for(int i = 0; i < num; i++) r.push_back(rows[i]);
	    removeRows(r);
	}
	void removeRows(vector<int> &rows);
	void removeAllRows(void);

	void setFieldWithCB(int row, int col,const char *s,bool redisplay=true,
			bool do_callback=true) {
		MmTableSetFieldWithCB(tw, row, col, s, redisplay, do_callback);
	}
	virtual void setField(int row, int col, const string &s,
		bool redisplay=true)
	{
	    setFieldWithCB(row, col, s.c_str(), redisplay);
	}
	void setField(int row, int col, const char *s, bool redisplay=true) {
	    setFieldWithCB(row, col, s, redisplay);
	}
	virtual void setFieldWithCB(int row, int col, const string &s,
		bool redisplay=true)
	{
	   setFieldWithCB(row, col, s.c_str(), redisplay);
	}

	virtual void setFields(int num, int *rows, int *cols, const char **s) {
		return MmTableSetFields(tw, num, rows, cols, s); }
	char *getField(int row, int col);
	bool getDouble(int row, int col, double *d) {
	    char *s = getField(row, col);
	    bool ret = stringToDouble(s, d);
	    Free(s);
	    return ret;
	}
	bool getInt(int row, int col, int *i) {
	    char *s = getField(row, col);
	    bool ret = stringToInt(s, i);
	    Free(s);
	    return ret;
	}
	bool findAndSetRow(int num_columns, int *col, const char **names,
		const char **row);
	void addColumn(const char **col, const string &label, bool isEditable,
		int alignment);
	/** Append a column and remove the first column.
	 *  @param[in] col the cell values for the column.
	 *  @param[in] label the column label.
	 *  @param[in] is_editable true if the column cells are editable.
	 *  @param[in] alignment LEFT_JUSTIFY, RIGHT_JUSTIFY or CENTER
	 */
	void rollColumn(const char **col, const string &label, bool is_editable,
		int alignment);
	void removeColumn(int col);
	void resetColumnLimits(void) { MmTableResetColumnLimits(tw); }
	void setColumn(int col, const char **values, int values_length);
	void setColumn(int col, vector<const char *> &values);
	void formatColumn(int col);
	void formatColumns(int *col, int col_length);
	void insert(int row_index, const char **row);
	void displayVerticalScrollbar(bool set);
	void displayHorizontalScrollbar(bool set);
	void setEditable(bool set);
	void setSelectable(bool set);
	void setRowSelectable(bool set);
	void setColumnSelectable(bool set);
	void setTableBackground(Pixel bg);
	void adjustColumns(void) { MmTableAdjustColumns(tw); }
	void highlightField(int row, int col, bool highlight=true);
	bool isHighlighted(int row, int col);
	void highlightOff(void);
	void adjustColumnWidths(int j1, int j2) {
	    MmTableAdjustColumnWidths(tw, j1, j2); }
	const char *tableName(void);
	void sortByColumn(int col);
	void sortByColumnLabels(const string &col_labels);
	void sortByColumns(vector<int> &col);
	void sortSelected(vector<int> &col);
	void reverseOrder(void);
	void promoteSelectedRows(void);
	void promoteSelectedColumns(void);
	void setColumnColors(Pixel *colors);
	void setColumnEditable(bool *set_editable);
	void setRowEditable(bool *set_editable);
	int getRowEditable(vector<bool> &set_editable);
	void setAlignment(int ncols, int *alignment);
	int getAlignment(vector<int> &alignment);
	void sortUnique(vector<int> &cols);
	void showAll(void);
	void expand(int col);
	int numRows(void);
	int numColumns(void);
	int getRow(int i, vector<const char *> &row);
	const char **getRow(int i);
	void setRow(int i, const char **row);
	void setRow(int i, vector<const char *> &row);
	void setRow(int i, vector<char *> &row);
	void setRow(int i, vector<string> &row);
	const char **getColumnByLabel(const string &label);
	int getColumnByLabel(const string &label, vector<const char *> &col);
	const char **getColumn(int j);
	int getColumn(int j, vector<const char *> &col);
	int getColumnNChars(vector<int> &col_nchars);
	void moveToTop(int row);
	int getRowOrder(vector<int> &order);
	void setRowOrder(vector<int> &order);
	int getColumnOrder(vector<int> &co);
	void setColumnOrder(vector<int> &order);
	int getRowStates(vector<bool> &states);
	int setRowStates(vector<bool> &states);
	int getSelectedRows(vector<int> &rows);
	int getSelectedRows(int **rows) {
	    vector<int> r;
	    getSelectedRows(r);
	    *rows = NULL;
	    if(r.size() > 0) {
		*rows = (int *)malloc(r.size()*sizeof(int));
		for(int i = 0; i < (int)r.size(); i++) (*rows)[i] = r[i];
	    }
	    return r.size();
	}
	int numSelectedRows(void);
	int getDeselectedRows(vector<int> &rows);
	int getSelectedRowsOrdered(vector<int> &rows);
	const char **getColumnLabels(void);
	int getColumnLabels(vector<const char *> &labels);
	void selectRow(int row, bool select);
	void selectRowWithCB(int row, bool select);
	void selectAllRows(bool state);
	void selectAllRowsWithCB(bool state);
	void selectRows(int num_rows, int *rows, bool *states);
	void selectRows(vector<int> &rows, vector<bool> &states);
	void selectColumns(vector<int> &columns, vector<bool> &states);
	void deselectAllRows(void) { selectAllRows(false); }
	void selectAllColumns(bool state);
	int getSelectedColumns(vector<int> &col_indices);
	int numSelectedColumns(void);
	bool editMode(void) { return edit_mode; }
	bool fieldSelected(void);
	void cutEvent(XButtonEvent *event) { MmTableCut(tw, event); }
	void copyEvent(XButtonEvent *event) { MmTableCopy(tw, event); }
	void pasteEvent(XButtonEvent *event) {MmTablePaste(tw, event);}
	void editModeOff(void) { MmTableEditModeOff(tw); }
	bool fieldEdited(int row, int col);
	bool rowEdited(int row);
	bool edited();
	void cancelEdit(void);
	void backup(void);
	void restore(void);
	void restoreField(int row, int col);
	Pixel string_to_pixel(Widget widget, const string &color_name) {
	    return string_to_pixel(widget, color_name.c_str()); }
	void setRowLabels(const char **labels) {MmTableSetRowLabels(tw,labels);}
	char **getRowLabels(void);
	void displayRowLabels(bool display, bool redisplay=true);
	void warn(const string &message);
	void info(const string &message);
	virtual void clear(void);
	Size getPreferredSize(void) { return MmTableGetPreferredSize(tw); }
	void setColumnLabel(int col, const string &label);
	void resetColumnOrder(void);
	bool rowSelected(void);
	bool columnSelected(void);
	void moveRowUp(int row) { moveRow(row, -1); }
	void moveRowDown(int row) { moveRow(row, 1); }
	void moveRow(int row, int dir);
	void setColumnByLabel(const string &label, const char **values,
		int values_length);
	void setColumnByLabel(const string &label,vector<const char *> &values);
	int getColumnIndex(const string &label);
	void setCellChoice(int row, int col, const string &choice);
	void setColumnTime(vector<enum TimeFormat> &time_code);
	void setColumnChoice(int col, const string &choice);
	char * getCellChoice(int row, int col) {
		return MmTableGetCellChoice(tw, row, col); }
	void fillCell(int row, int col, Pixel pixel, bool redisplay);
	Pixel getCellPixel(int row, int col);
	void selectCell(int row, int col, bool select) {
	    fillCell(row, col, select ? CELL_TOGGLE_ON : CELL_TOGGLE_OFF, true);
	}
	bool cellSelected(int row, int col) {
	    return (getCellPixel(row, col) == CELL_TOGGLE_ON) ? true : false;
	}
	void fillRow(int i, Pixel *row, bool redisplay);
	void addFilledRow(Pixel *pixel, bool redraw);
	int getSortUnique(vector<int> &cols);
	int getSelectedAndHiddenRows(vector<int> &rows);
	enum TimeFormat getColumnTime(int col);
	bool cellEditable(int row, int column);
	void setCellEditable(int row, int column, bool state);
	void setTitle(const string &title) {
	    Arg args[1];
	    XtSetArg(args[0], XtNtableTitle, title.c_str());
	    XtSetValues((Widget)tw, args, 1);
	}
	void getScrolls(int *horizontal_pos, int *vertical_pos);
	void setScrolls(int horizontal_pos, int vertical_pos);
	void displayVerticalLines(bool display);
	void displayHorizontalLines(bool display);

	/** Get the time properties of the table.
	 *  @param[out] epoch the time of the left most column.
	 *  @param[out] small_tdel the time width of each column.
	 *  @param[out] large_tdel the time between time-lines.
	 */
	void getTime(double *epoch, double *small_tdel, double *large_tdel);
	/** Set the time of the left edge of the table.
	 *  @param[in] epoch time in seconds since 1970-01-01.
	 */
	void setTime(double epoch);
	/** Set the time properties of the table.
	 *  @param[in] large_tdel the time between time-lines.
	 *  @param[in] n_small_tdel the number of columns between time-lines.
	 */
	void setTdels(double large_tdel, int n_small_tdel);
	bool editable(void) {
	    Arg args[1];
	    Boolean b = True;
	    XtSetArg(args[0], XtNeditable, &b);
	    getValues(args, 1);
	    return (bool)b;
	}
	void displayEditControls(bool display) {
	    display_edit_controls = display; }

	void setAttributes(const string &attribute_list);
	void setAttributes(const string &attribute_list,
			const string &display_list);
	void setDisplayAttributes(const string &display_list);
	TableAttributes *showAttributes(bool show);
	int getLabelsAlignment(char ***column_labels, int **alignment);
	TAttribute getAttribute(int i);
	TAttribute getAttribute(const string &name);
	virtual void editModeOn(void);
	virtual void editCancel(void);
	virtual void editSave(void);
	void setValues(Arg *args, int n) { setValues(args, n, true); }
	void getValues(Arg *args, int n) { setValues(args, n, false); }

	virtual void copy(TableSelectionType select_type, Time selection_time);
	virtual void cut(TableSelectionType select_type, Time selection_time);
	virtual void paste(Time time);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd parseSort(const string &c, const string &cmd, bool up,
			bool sort_selected_rows, string &msg);
	ParseVar parseFindIndices(const string &name, string &value);
	static void parseHelp(const char *prefix);
	bool saveTable(const string &file, const string &access, int save_mode,
			string &msg);
	void save(const string &file) { save(file, "w"); }
	void save(const string &file, const string &access);
	void printSelectedRows(FILE *fp);
	void printSelectedColumns(FILE *fp);
	void printAll(FILE *fp);
	char * getTableString(vector<int> &rows, unsigned long *length);
	XFontStruct *getFont();

	static void printTime(double time, const string &format, char *value,
			int maxlen);

    protected:
	MmTableWidget tw;
	RowColumn *controls;
	Widget table_menu;
	Button *cancel_button;
	Button *save_button;
	TableAttributes	*table_attributes;
	bool edit_mode;
	string display_attributes;
	enum TableSelectionType selection_type;
	int column_index;
	Atom table_row_atom;
	Atom table_column_atom;
	Atom requested;
	Time copy_time;
	void *copy_value;
	void *copy_string_value;
	int copy_length;
	int copy_string_length;
	InfoArea *info_area;
	bool display_edit_controls;

	char row_name[20];
	char selected_row_name[20];
	int parse_row_index;
	int selected_row_index;
	vector<int> selected_rows;

	char column_name[20];
	char selected_column_name[20];
	int parse_column_index;
	int selected_column_index;
	vector<int> selected_columns;

	void init(const string &, Component *, InfoArea *, Arg *, int);
	virtual void modifyVerify(Widget w, XtPointer calldata);

	int setColumns(void);
	void setValues(Arg *args, int n, bool set);
	void saveAttributes(void);
	bool getDisplayAttributes(string &prop);

	void setAppContext(XtAppContext app);
	bool rowSelection(XtPointer *value, unsigned long *length,
			bool rows_target);
	bool columnSelection(XtPointer *value, unsigned long *length,
			bool column_target);
	static Table * getTableFromWidget(Widget w);
	ParseCmd parseSelectRow(const string &c, string &msg, bool select);
	int parseGetSelectRows(const string &c, string &msg, vector<int> &ri);
	ParseCmd parseSelectColumns(const string &c, string &msg, bool select);
	int parseGetSelectColumns(const string &c, string &msg,vector<int> &ci);
	ParseCmd parseSetCell(const string &c, string &msg);

	ParseVar getRowRequest(const string &name, string &value);
	ParseVar getColumnRequest(const string &name, string &value);
	void putParseString(const string &name, vector<const char *> &row);
	ParseVar getCellWithIndex(const char *name, string &value);
	ParseVar getRowWithIndex(const char *name, string &value);
	ParseVar getColumnWithIndex(const char *name, string &value);

    private:
	static void attributesCallback(Widget, XtPointer, XtPointer);
	static void editCallback(Widget, XtPointer, XtPointer);
	static void selectAllCallback(Widget, XtPointer, XtPointer);
	static void deselectAllCallback(Widget, XtPointer, XtPointer);
	static void selectRowCallback(Widget, XtPointer, XtPointer);
	static void selectColumnCallback(Widget, XtPointer, XtPointer);
	static void columnMovedCallback(Widget, XtPointer, XtPointer);
	static void editModeCallback(Widget, XtPointer, XtPointer);
	static void modifyVerifyCallback(Widget, XtPointer, XtPointer);
	static void valueChangedCallback(Widget, XtPointer, XtPointer);
	static void choiceChangedCallback(Widget, XtPointer, XtPointer);
	static void cellEnterCallback(Widget, XtPointer, XtPointer);
	static void cellSelectCallback(Widget, XtPointer, XtPointer);
	static void leaveWindowCallback(Widget, XtPointer, XtPointer);
	static void rowChangeCallback(Widget, XtPointer, XtPointer);
	static void copyPasteCallback(Widget, XtPointer, XtPointer);

	static Boolean convertSelection(Widget w, Atom *selection, Atom *target,
		Atom *type_return, XtPointer *value_return,
		unsigned long *length_return, int *format_return);
	static void loseOwnership(Widget w, Atom *selection);
	static void transferDone(Widget w, Atom *selection, Atom *target);
	static void requestorCallback(Widget w, XtPointer p, Atom *selection,
		Atom *type, XtPointer value, unsigned long *length,int *format);
};

#endif
