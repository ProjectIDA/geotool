#ifndef _MMTABLEP_H_
#define _MMTABLEP_H_

#include <Xm/Xm.h>
#include <Xm/ManagerP.h>

#include "widget/MmTable.h"
#include "widget/TCanvasP.h"
#include "widget/HCanvasP.h"
extern "C" {
#include "libtime.h"
}

/// @cond

typedef struct
{
	int	row;
	int	col;
	char	*choice;
} CellChoice;

typedef struct
{
	int	row;
	int	col;
} CellIndex;

typedef struct
{
	int	*cols;
	int	cols_length;
} SortUnique;

typedef struct
{
	Table		*table_class;
	TCanvasWidget	canvas;	/* TCanvas */
	HCanvasWidget	header;	/* HCanvas */
	Widget		hbar;	/* horizontal Scrollbar */
	Widget		vbar;	/* vertical Scrollbar */
	Widget		label;	/* title */
	Widget		menu_button; /* optional Menu pulldown */
	Widget		cascade_button;
	Widget		row_column;

	char		***columns;	/* String[][] */
	Boolean		**highlight;	/* boolean[][] */
	char		***backup;	/* String[][] */
	int		*row_order;
	int		*display_order;
	Boolean		*row_state;
	char		**column_labels;	/* String[] */
	char		**column_choice;	/* String[] */
	char		**row_labels;		/* String[] */
	char		**cells;		/* String[] */
	Boolean		*column_state;
	char		**columnEditable;	/* String[] */
	enum TimeFormat	*col_time;		/* time code */
	int		incremental_capacity;
	int		capacity;
	int		visible_rows;
	int		min_col_width;
	int		bottom_margin;
	int		increases;
	int		margin;
	int		cellMargin;
	long		nrows, ncols;
	int		nhidden;
	int		max_ascent, max_descent;
	int		row_height;
	int		last_vbar_value;
	int		last_hbar_value;
	int		*col_width, *col_beg, *col_end, *col_order, *col_nchars;
	int		top_row;
	Pixel		fg;	/* foreground */
	Pixel		table_background;	/* table background */
	Pixel		*col_color;	/* Color[] */
	Pixel		highlight_color; /*  Color.red; */
	Pixel		canvas_background;
	Pixel		**cell_fill;
	XFontStruct	*font;
	Boolean		*col_editable;
	char		**col_choice;
	Boolean		*row_editable;
	int		*col_alignment;
	int		top, bottom, left, right;
	int		edit_col, edit_row, edit_pos, edit_x;
	int		select_row, select_col;
	int		select_char1, select_char2;
	int		select_x1, select_x2;
	int		row_label_width;
	int		toggle_label_width, toggle_label_y;
	int		cursor_row;
	int		cursor_col;
	char		*edit_string;
	int		needAdjustColumns;
	int		*charWidths;
	int		num_cell_choices;
	int		num_cell_non_editable;
	CellIndex	*cell_non_editable;
	SortUnique	sort_unique;
	CellChoice	*cell_choice;
	Boolean		edit_row_state;
	Boolean		start_edit_cursor;
	Boolean		have_focus;
	Boolean		displayVerticalScrollbar;
	Boolean		initialDisplayVerticalScrollbar;
	Boolean		displayHorizontalScrollbar;
	Boolean		initialDisplayHorizontalScrollbar;
	Boolean		editable;
	Boolean		selectable;
	Boolean		rowSelectable;
	Boolean		columnSelectable;
	Boolean		columnSingleSelect;
	Boolean		singleSelect;
	Boolean		radioSelect;
	Boolean		cntrlSelect;
	Boolean		dragSelect;
	Boolean		displayRowLabels;
	Boolean		newRowLabels;
	Boolean		select_toggles;
	Boolean		centerHorizontally;
	Boolean		need_layout;
	Boolean		ignore_changed_managed;
	Boolean		display_menu_button;
	Boolean		pixmap_created;
	Boolean		color_only;
	Boolean		do_internal_select_rowCB;
	Widget		cursor_info;
	Widget		table_info;
	String		table_info_text;
	String		field_selection;
	char		*table_title;
	char		*name;
	char		*toggle_label;
	char		*table_message;
	char		*cursor_message;
	XtAppContext	app_context;

	double		epoch_time;
	int		n_small_tdel;
	double		small_tdel;
	double		large_tdel;
	/* FontMetrics	fontMetrics; */
	/* Clipboard	systemClipboard; */
	/* Vector	tableListeners = new Vector(); */

	GC		gc;
	GC		highlightGC;

	XtCallbackList	selectRowCallbacks;
	XtCallbackList	selectColumnCallbacks;
	XtCallbackList	columnMovedCallbacks;
	XtCallbackList	editModeCallbacks;
	XtCallbackList	modifyVerifyCallbacks;
	XtCallbackList	valueChangedCallbacks;
	XtCallbackList	choiceChangedCallbacks;
	XtCallbackList	warningCallbacks;
	XtCallbackList	cellEnterCallbacks;
	XtCallbackList	cellSelectCallbacks;
	XtCallbackList	leaveWindowCallbacks;
	XtCallbackList	rowChangeCallbacks;
	XtCallbackList	copyPasteCallbacks;
} MmTablePart;

typedef struct
{
	int	screen;
	Pixmap	pixmap;
} ScreenPixmap;

typedef struct
{
	ScreenPixmap sp[10];
} MmTableClassPart;

typedef struct _MmTableRec
{
	CorePart	core;
	CompositePart	composite;
	ConstraintPart	constraint;
	XmManagerPart	manager;
	MmTablePart	mmTable;
} MmTableRec;

typedef struct _MmTableClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	MmTableClassPart	mmTable_class;
} MmTableClassRec;

extern MmTableClassRec	mmTableClassRec;

void _MmTableResetInfo(MmTableWidget w);

/// @endcond

#endif
