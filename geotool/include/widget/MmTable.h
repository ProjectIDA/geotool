#ifndef _MMTABLE_H_
#define _MMTABLE_H_

extern "C" {
#include "libtime.h"
}
#include <string>
#include <vector>
using namespace std;

/// @cond
typedef struct _MmTableClassRec	*MmTableWidgetClass;
typedef struct _MmTableRec	*MmTableWidget;

extern WidgetClass	mmTableWidgetClass;
/// @endcond

#define XtNcolumns			(char *)"columns"
#define XtNrows				(char *)"rows"
#define XtNcolumnLabels			(char *)"columnLabels"
#define XtNrowLabels			(char *)"rowLabels"
#define XtNcells			(char *)"cells"
#define XtNvisibleRows			(char *)"visibleRows"
#define XtNincrementalCapacity		(char *)"incrementalCapacity"
#define XtNmargin			(char *)"margin"
#define XtNbottomMargin			(char *)"bottomMargin"
#define XtNcellMargin			(char *)"cellMargin"
#define XtNhighlighColor		(char *)"highlighColor"
#define XtNdisplayVerticalScrollbar	(char *)"displayVerticalScrollbar"
#define XtNdisplayHorizontalScrollbar	(char *)"displayHorizontalScrollbar"
#define XtNeditable			(char *)"editable"
#define XtNselectable			(char *)"selectable"
#define XtNrowSelectable		(char *)"rowSelectable"
#define XtNcolumnSelectable		(char *)"columnSelectable"
#define XtNcolumnSingleSelect		(char *)"columnSingleSelect"
#define XtNtableBackground		(char *)"tableBackground"
#define XtNsingleSelect			(char *)"singleSelect"
#define XtNradioSelect			(char *)"radioSelect"
#define XtNcntrlSelect			(char *)"cntrlSelect"
#define XtNdragSelect			(char *)"dragSelect"
#define XtNcursorInfoWidget		(char *)"cursorInfoWidget"
#define XtNtableInfoWidget		(char *)"tableInfoWidget"
#define XtNselectToggles		(char *)"selectToggles"
#define XtNcenterHorizontally		(char *)"centerHorizontally"
#define XtNtoggleLabel			(char *)"toggleLabel"
#define XtNcolumnEditable		(char *)"columnEditable"
#define XtNcolorOnly			(char *)"colorOnly"
#define XtNdoInternalSelectRowCB	(char *)"doInternalSelectRowCB"
#define XtNtableTitle			(char *)"tableTitle"
#define XtNdisplayMenuButton		(char *)"displayMenuButton"
#define XtNcolumnChoice			(char *)"columnChoice"
#define XtNminimumColumnWidth		(char *)"minimumColumnWidth"
#define XtNselectRowCallback		(char *)"selectRowCallback"
#define XtNselectColumnCallback		(char *)"selectColumnCallback"
#define XtNcolumnMovedCallback		(char *)"columnMovedCallback"
#define XtNeditModeCallback		(char *)"editModeCallback"
#define XtNmodifyVerifyCallback		(char *)"modifyVerifyCallback"
#define XtNvalueChangedCallback		(char *)"valueChangedCallback"
#define XtNchoiceChangedCallback	(char *)"choiceChangedCallback"
#define XtNwarningCallback		(char *)"warningCallback"
#define XtNcellEnterCallback		(char *)"cellEnterCallback"
#define XtNcellSelectCallback		(char *)"cellSelectCallback"
#define XtNleaveWindowCallback		(char *)"leaveWindowCallback"
#define XtNrowChangeCallback		(char *)"rowChangeCallback"
#define XtNcopyPasteCallback		(char *)"copyPasteCallback"

#define XtCColumns			(char *)"Columns"
#define XtCRows				(char *)"Rows"
#define XtCColumnLabels			(char *)"ColumnLabels"
#define XtCRowLabels			(char *)"RowLabels"
#define XtCCells			(char *)"Cells"
#define XtCVisibleRows			(char *)"VisibleRows"
#define XtCCellMargin			(char *)"CellMargin"
#define XtCBottomMargin			(char *)"BottomMargin"
#define XtCIncrementalCapacity		(char *)"IncrementalCapacity"
#define XtCHighlighColor		(char *)"HighlighColor"
#define XtCDisplayVerticalScrollbar	(char *)"DisplayVerticalScrollbar"
#define XtCDisplayHorizontalScrollbar	(char *)"DisplayHorizontalScrollbar"
#define XtCEditable			(char *)"Editable"
#define XtCSelectable			(char *)"Selectable"
#define XtCRowSelectable		(char *)"RowSelectable"
#define XtCColumnSelectable		(char *)"ColumnSelectable"
#define XtCColumnSingleSelect		(char *)"ColumnSingleSelect"
#define XtCCntrlSelect			(char *)"CntrlSelect"
#define XtCTableBackground		(char *)"TableBackground"
#define XtCSingleSelect			(char *)"SingleSelect"
#define XtCRadioSelect			(char *)"RadioSelect"
#define XtCDragSelect			(char *)"DragSelect"
#define XtCCursorInfoWidget		(char *)"CursorInfoWidget"
#define XtCTableInfoWidget		(char *)"TableInfoWidget"
#define XtCSelectToggles		(char *)"SelectToggles"
#define XtCCenterHorizontally		(char *)"CenterHorizontally"
#define XtCToggleLabel			(char *)"ToggleLabel"
#define XtCName				(char *)"Name"
#define XtCColumnEditable		(char *)"ColumnEditable"
#define XtCColorOnly			(char *)"ColorOnly"
#define XtCDoInternalSelectRowCB	(char *)"DoInternalSelectRowCB"
#define XtCTableTitle			(char *)"TableTitle"
#define XtCColumnChoice			(char *)"ColumnChoice"
#define XtCDisplayMenuButton		(char *)"DisplayMenuButton"
#define XtCMinimumColumnWidth		(char *)"MinimumColumnWidth"
#define XtCSelectRowCallback		(char *)"SelectRowCallback"
#define XtCSelectColumnCallback		(char *)"SelectColumnCallback"
#define XtCColumnMovedCallback		(char *)"ColumnMovedCallback"
#define XtCEditModeCallback		(char *)"EditModeCallback"
#define XtCModifyVerifyCallback		(char *)"ModifyVerifyCallback"
#define XtCValueChangedCallback		(char *)"ValueChangedCallback"
#define XtCChoiceChangedCallback	(char *)"ChoiceChangedCallback"
#define XtCWarningCallback		(char *)"WarningCallback"
#define XtCCellEnterCallback		(char *)"CellEnterCallback"
#define XtCCellSelectCallback		(char *)"CellSelectCallback"
#define XtCLeaveWindowCallback		(char *)"LeaveWindowCallback"
#define XtCRowChangeCallback		(char *)"RowChangeCallback"
#define XtCCopyPasteCallback		(char *)"CopyPasteCallback"

#define LEFT_JUSTIFY	0
#define RIGHT_JUSTIFY	1
#define CENTER		2

#define CELL_NO_FILL_FLAG	999999
#define CELL_TOGGLE_ON		999998
#define CELL_TOGGLE_OFF		999997

enum TableSelectionType {
	TABLE_COPY_ROWS,
	TABLE_COPY_ALL,
	TABLE_COPY_COLUMNS,
	TABLE_COPY_THIS_COLUMN,
	TABLE_PASTE
};

typedef struct {
	enum TableSelectionType	selection_type;
	int	column;
	Time	time;
} MmTableCopyPasteCallbackStruct;


#ifndef _Size_
#define _Size_
typedef struct {
	int	width;
	int	height;
} Size;
#endif

typedef struct {
	int		nrows;
	int		nchanged_rows;
	int		*changed_rows;
	Boolean		*states;
} MmTableSelectCallbackStruct;

typedef struct {
	int		column;
	int		row;
	String		old_string;		
	String		new_string;
} MmTableEditCallbackStruct;

typedef struct {
	int		row;
	int		column;
	char		*string;
	Pixel		pixel;
	XEvent		*event;
} MmTableCellCallbackStruct;

void MmTableAdjustHScroll(MmTableWidget w);
void MmTableResetColumnLimits(MmTableWidget w);
void MmTableAdjustColumns(MmTableWidget w);

void MmTableAdjustColumnWidths(MmTableWidget w, int j1, int j2);

void MmTableCut(MmTableWidget w, XButtonEvent *event);
void MmTableCopy(MmTableWidget w, XButtonEvent *event);
void MmTablePaste(MmTableWidget w, XButtonEvent *event);
void MmTableEditModeOff(MmTableWidget w);

int MmTableGetBarValue(Widget w);
void MmTableSetBarValue(Widget w, int value);
int MmTableGetBarVisibleAmount(Widget w);

Pixel string_to_pixel(Widget widget, const char *color_name);
void MmTableSetRowLabels(MmTableWidget w, const char **labels);

Size MmTableGetPreferredSize(MmTableWidget w);

Widget MmTableCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);

Widget MmTableMenuChild(MmTableWidget w);
char * MmTableGetCellChoice(MmTableWidget w, int row, int col);
void MmTableSetFields(MmTableWidget w, int num, int *rows, int *cols,
			const char **s);
void MmTableSetFieldWithCB(MmTableWidget w, int row, int col, const char *s,
			bool redisplay, bool do_callback);
class Table;
void MmTableSetClass(MmTableWidget w, Table *t);

#endif
