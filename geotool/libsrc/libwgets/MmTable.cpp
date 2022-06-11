/** \file MmTable.c
 *  \brief Defines widget MmTable.
 *  \author Ivan Henson
 */
#include "config.h"
/*
 * NAME
 *      MmTable Widget -- widget for displaying tabular data.
 *
 * SYNOPSIS
 *      #include "MmTable.h"
 *      Widget
 *      MmTableCreate(parent, name, arglist, argcount)
 *      Widget parent;          (i) parent widget
 *      String name;            (i) name of widget
 *      ArgList arglist;        (i) arguments
 *      Cardinal argcount       (i) number of arguments
 *
 * FILES
 *      MmTable.h
 *
 * AUTHOR
 *      I. Henson -- February 2000
 *	Multimax, Inc.
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <strings.h>
#include <errno.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Xmu.h>
#include "Xm/XmAll.h"
extern "C" {
#include "libgplot.h"
#include "libtime.h"
#include "libstring.h"
#include "motif++/Info.h"
}
#include "widget/MmTableP.h"
#include "widget/Table.h"

#define XtRBooleanArray		(char *)"BooleanArray"

#define offset(field)	XtOffset(MmTableWidget, mmTable.field)
static XtResource	resources[] =
{
    {XtNdisplayMenuButton, XtCDisplayMenuButton, XtRBoolean,
	sizeof(Boolean), offset(display_menu_button), XtRString,
	(XtPointer)"False"},
    {XtNtableTitle, XtCTableTitle, XtRString, sizeof(String),
	offset(table_title), XtRImmediate, (XtPointer)NULL},
    {XtNcolumns, XtCColumns, XtRInt, sizeof(int),
	offset(ncols), XtRImmediate, (XtPointer)0},
    {XtNcolumnLabels, XtCColumnLabels, XmRStringArray, sizeof(String *),
	offset(column_labels), XmRImmediate, (XtPointer)NULL},
    {XtNcolumnChoice, XtCColumnChoice, XmRStringArray, sizeof(String *),
	offset(column_choice), XmRImmediate, (XtPointer)NULL},
    {XtNrowLabels, XtCRowLabels, XmRStringArray, sizeof(String *),
	offset(row_labels), XmRImmediate, (XtPointer)NULL},
/*
    {XtNcolumnEditable, XtCColumnEditable, XmRStringArray, sizeof(String *),
	offset(columnEditable), XmRImmediate,(XtPointer) NULL},
*/
    {XtNcolumnEditable, XtCColumnEditable, XtRBooleanArray, sizeof(Boolean *),
	offset(col_editable), XmRImmediate,(XtPointer) NULL},

    {XtNcells, XtCCells, XmRStringArray, sizeof(String *),
	offset(cells), XmRImmediate, (XtPointer)NULL},
    {XtNvisibleRows, XtCVisibleRows, XtRInt, sizeof(int),
	offset(visible_rows), XtRImmediate, (XtPointer)0},
    {XtNincrementalCapacity, XtCIncrementalCapacity, XtRInt, sizeof(int),
	offset(incremental_capacity), XtRImmediate, (XtPointer)100},
    {XtNmargin, XtCMargin, XtRInt, sizeof(int),
	offset(margin), XtRImmediate, (XtPointer)4},
    {XtNbottomMargin, XtCMargin, XtRInt, sizeof(int),
	offset(bottom_margin), XtRImmediate, (XtPointer)0},
    {XtNminimumColumnWidth, XtCMargin, XtRInt, sizeof(int),
	offset(min_col_width), XtRImmediate, (XtPointer)0},
    {XtNcellMargin, XtCCellMargin, XtRInt, sizeof(int),
	offset(cellMargin), XtRImmediate, (XtPointer)12},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(fg), XtRString,(XtPointer) XtDefaultForeground},
    {XtNtableBackground, XtCTableBackground, XtRPixel, sizeof(Pixel),
	offset(table_background), XtRImmediate, (Pixel)NULL},
    {XtNhighlighColor, XtCHighlighColor, XtRPixel, sizeof(Pixel),
	offset(highlight_color), XtRString, (XtPointer)"red"},
    {XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(font), XtRString, (XtPointer)"*courier-bold-r-*--14-*"},
    {XtNcenterHorizontally, XtCCenterHorizontally, XtRBoolean,
	sizeof(Boolean), offset(centerHorizontally), XtRString,(XtPointer) "False"},
    {XtNdisplayVerticalScrollbar, XtCDisplayVerticalScrollbar, XtRBoolean,
	sizeof(Boolean), offset(displayVerticalScrollbar), XtRString,(XtPointer) "False"},
    {XtNdisplayHorizontalScrollbar, XtCDisplayHorizontalScrollbar, XtRBoolean,
	sizeof(Boolean), offset(displayHorizontalScrollbar), XtRString,(XtPointer)"False"},
    {XtNeditable, XtCEditable, XtRBoolean, sizeof(Boolean),
	offset(editable), XtRString,(XtPointer) "True"},
    {XtNselectable, XtCSelectable, XtRBoolean, sizeof(Boolean),
	offset(selectable), XtRString, (XtPointer)"True"},
    {XtNrowSelectable, XtCRowSelectable, XtRBoolean, sizeof(Boolean),
	offset(rowSelectable), XtRString, (XtPointer)"True"},
    {XtNcolumnSelectable, XtCColumnSelectable, XtRBoolean, sizeof(Boolean),
	offset(columnSelectable), XtRString,(XtPointer) "True"},
    {XtNcolumnSingleSelect, XtCColumnSingleSelect, XtRBoolean, sizeof(Boolean),
	offset(columnSingleSelect), XtRString, (XtPointer)"True"},
    {XtNsingleSelect, XtCSingleSelect, XtRBoolean, sizeof(Boolean),
	offset(singleSelect), XtRString, (XtPointer)"False"},
    {XtNradioSelect, XtCRadioSelect, XtRBoolean, sizeof(Boolean),
	offset(radioSelect), XtRString,(XtPointer) "False"},
    {XtNcntrlSelect, XtCCntrlSelect, XtRBoolean, sizeof(Boolean),
	offset(cntrlSelect), XtRString,(XtPointer) "False"},
    {XtNdragSelect, XtCDragSelect, XtRBoolean, sizeof(Boolean),
	offset(dragSelect), XtRString,(XtPointer) "True"},
    {XtNselectToggles, XtCSelectToggles, XtRBoolean, sizeof(Boolean),
	offset(select_toggles), XtRString,(XtPointer) "False"},
    {XtNname, XtCName, XtRString, sizeof(String),
	offset(name), XtRString, (XtPointer)""},
    {XtNtoggleLabel, XtCToggleLabel, XtRString, sizeof(String),
	offset(toggle_label), XtRString, (XtPointer)""},
    {XtNcolorOnly, XtCColorOnly, XtRBoolean, sizeof(Boolean),
	offset(color_only), XtRString, (XtPointer)"False"},
    {XtNdoInternalSelectRowCB, XtCDoInternalSelectRowCB, XtRBoolean,
	sizeof(Boolean), offset(do_internal_select_rowCB), XtRString,(XtPointer) "False"},
    {XtNselectRowCallback, XtCSelectRowCallback, XtRCallback,
	sizeof(XtCallbackList), offset(selectRowCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNselectColumnCallback, XtCSelectColumnCallback, XtRCallback,
	sizeof(XtCallbackList), offset(selectColumnCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNcolumnMovedCallback, XtCColumnMovedCallback, XtRCallback,
	sizeof(XtCallbackList), offset(columnMovedCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNeditModeCallback, XtCEditModeCallback, XtRCallback,
	sizeof(XtCallbackList), offset(editModeCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNmodifyVerifyCallback, XtCModifyVerifyCallback, XtRCallback,
	sizeof(XtCallbackList), offset(modifyVerifyCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNvalueChangedCallback, XtCValueChangedCallback, XtRCallback,
	sizeof(XtCallbackList), offset(valueChangedCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNchoiceChangedCallback, XtCChoiceChangedCallback, XtRCallback,
	sizeof(XtCallbackList), offset(choiceChangedCallbacks), XtRCallback,
	(XtPointer)NULL},
    {XtNwarningCallback, XtCWarningCallback, XtRCallback,
	sizeof(XtCallbackList), offset(warningCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNcellEnterCallback, XtCCellEnterCallback, XtRCallback,
	sizeof(XtCallbackList), offset(cellEnterCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNcellSelectCallback, XtCCellSelectCallback, XtRCallback,
	sizeof(XtCallbackList), offset(cellSelectCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNleaveWindowCallback, XtCLeaveWindowCallback, XtRCallback,
	sizeof(XtCallbackList), offset(leaveWindowCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNrowChangeCallback, XtCRowChangeCallback, XtRCallback,
	sizeof(XtCallbackList), offset(rowChangeCallbacks),
	XtRCallback, (XtPointer)NULL},
    {XtNcopyPasteCallback, XtCCopyPasteCallback, XtRCallback,
	sizeof(XtCallbackList), offset(copyPasteCallbacks), XtRCallback,
	(XtPointer)NULL},
    {XtNcursorInfoWidget, XtCCursorInfoWidget, XtRWidget, sizeof(Widget),
	offset(cursor_info), XtRImmediate,(XtPointer) NULL},
    {XtNtableInfoWidget, XtCTableInfoWidget, XtRWidget, sizeof(Widget),
	offset(table_info), XtRImmediate,(XtPointer) NULL},
};

/* Private functions */
static void ClassInitialize(void);
static Boolean CvtStringToStringArray(Display *dpy, XrmValuePtr args,
		Cardinal *num_args, XrmValuePtr from, XrmValuePtr to,
		XtPointer *data);
static void StringArrayDestructor(XtAppContext app, XrmValuePtr to,
		XtPointer converter_data, XrmValuePtr args, Cardinal *num_args);
static Boolean CvtStringToBooleanArray(Display *dpy, XrmValuePtr args,
		Cardinal *num_args, XrmValuePtr from, XrmValuePtr to,
		XtPointer *data);
static void BooleanArrayDestructor(XtAppContext app, XrmValuePtr to,
		XtPointer converter_data, XrmValuePtr args, Cardinal *num_args);
static void Initialize(Widget req, Widget nou);
static void Realize(Widget widget, XtValueMask *valueMask,
		XSetWindowAttributes *attrs);
static Boolean SetValues(MmTableWidget cur,MmTableWidget req,MmTableWidget nou);
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
		XtWidgetGeometry *reply);
static void ChangeManaged(Widget w);
static void Destroy(Widget widget);
static void Resize(Widget widget);
static void doLayout(MmTableWidget widget, Boolean vertScrollOnly);
static void Redisplay(Widget widget, XEvent *event, Region region);
static void format(MmTableWidget w, int col);
static void increaseCapacity(MmTableWidget w);
static void sort(MmTableWidget w, vector<int> &col, int i0, int row1, int row2);
static void sortByString(int lo0, int hi0, String *s, int *row_order);
static void sortByValue(int lo0, int hi0, double *values, int *row_order);
static void verticalScroll(Widget scroll, XtPointer client_data,
		XmScrollBarCallbackStruct *call_data);
static void horizontalScroll(Widget scroll, XtPointer client_data,
		XmScrollBarCallbackStruct *call_data);
static void sortInts(vector<int> &values, int lo0, int hi0);
static Boolean convertSelection(Widget w, Atom *selection, Atom *target,
		Atom *type, XtPointer *value, unsigned long *length,
		int *format);
static void loseOwnership(MmTableWidget w, Atom *selection);
static void transferDone(MmTableWidget w, Atom *selection, Atom *target);
static void requestorCallback(MmTableWidget w, XButtonEvent *event,
		Atom *selection, Atom *type, XtPointer value,
		unsigned long *length, int *format);
static void *MallocIt(MmTableWidget w, int nbytes);
static void *ReallocIt(MmTableWidget w, void *ptr, int nbytes);
static int getWidth(Widget w);
static void setBarValues(Widget w, int value, int visible, int min, int max);
static void getBarValues(Widget w, int *value, int *visible,int *min, int *max);
static int stringWidth(MmTableWidget w, const char *s);
static String substring(MmTableWidget w, String s, int i1, int i2);
static XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *proposed,
		XtWidgetGeometry *answer);
static void mousePressedButton1(MmTableWidget w, XEvent *event, String *params,
                        Cardinal *num_params);
static Boolean getNewColumnWidths(MmTableWidget w, int j1, int j2);
static void updateDisplayOrder(MmTablePart *t);
static void MmTable_drawTitle(MmTableWidget nou);
extern "C" {
static int sortDown(const void *A, const void *B);
}
static int getChoiceMaxWidth(MmTableWidget w, char *choice);
static void setButtonPixmap(MmTableWidget widget);
static void doSelectRowCallback(MmTableWidget w, int row);
static void doSelectRowCB(MmTableWidget w);
static void MmTableAddRow(MmTableWidget w, char **row, Boolean redraw);
static void MmTableAddRowWithLabel(MmTableWidget w, char **row, char *label,
		bool redraw);


static char defTrans[] =
" <Btn1Down>:          mousePressedButton1()";


static XtActionsRec actionsList[] =
{
    {(char *)"mousePressedButton1",	(XtActionProc)mousePressedButton1},
};



MmTableClassRec	mmTableClassRec = 
{
    {	/* core fields */
	(WidgetClass)(&xmManagerClassRec),	/* superclass */
	(char *)"MmTable",		/* class_name */
	sizeof(MmTableRec),		/* widget_size */
	ClassInitialize,		/* class_initialize */
	NULL,				/* class_part_initialize */
	FALSE,				/* class_inited */
	(XtInitProc)Initialize,		/* initialize */
	NULL,				/* initialize_hook */
	Realize,			/* realize */
        actionsList,			/* actions */
        XtNumber(actionsList),		/* num_actions */
	resources,			/* resources */
	XtNumber(resources),		/* num_resources */
	NULLQUARK,			/* xrm_class */
	TRUE,				/* compress_motion */
	TRUE,				/* compress_exposure */
	TRUE,				/* compress_enterleave */
	TRUE,				/* visible_interest */
	Destroy,			/* destroy */
	Resize,				/* resize */
	Redisplay,			/* expose */
	(XtSetValuesFunc)SetValues,	/* set_values */
	NULL,				/* set_values_hook */
	XtInheritSetValuesAlmost,	/* set_values_almost */
	NULL,				/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,			/* version */
	NULL,				/* callback_private */
        defTrans,			/* tm_table */
	query_geometry,			/* query_geometry */
	XtInheritDisplayAccelerator,	/* display_accelerator */
	NULL				/* extension */
    },
    {	/* composite_class fields */
	GeometryManager,		/* geometry_manager */
	ChangeManaged,			/* change_managed */
	XtInheritInsertChild,		/* insert_child */
	XtInheritDeleteChild,		/* delete_child */
	NULL				/* extension */
    },
    {   /* constraint_class fields */
	NULL,				/* constraint resource list */
	0,				/* number of constraints in list */
	0,				/* size of constraint record */
	NULL,				/* constraint initialization */
	NULL,				/* constraint destroy proc */
	NULL,				/* constraint set_values proc */
	NULL,				/* pointer to extension record */
    },
    {   /* manager_class fields */
	XtInheritTranslations,		/* translations */
	NULL,				/* syn_resources */
	0,				/* num_syn_resources */
	NULL,				/* syn_constraint_resources */
	0,				/* num_syn_constraint_resources */
	XmInheritParentProcess,		/* parent_process */
	NULL,				/* extension */
    },
    {	/* MmTableClass fields */
	{
	    {-1,0},{-1,0},{-1,0},{-1,0},{-1,0},
	    {-1,0},{-1,0},{-1,0},{-1,0},{-1,0}
	},				/* sp */
    },
};

WidgetClass mmTableWidgetClass = (WidgetClass)&mmTableClassRec;

static void
ClassInitialize(void)
{
    XtSetTypeConverter(XmRString, XmRStringArray,
	(XtTypeConverter)CvtStringToStringArray,
	NULL, 0, XtCacheAll | XtCacheRefCount, StringArrayDestructor);
    XtSetTypeConverter(XmRString, XtRBooleanArray,
	(XtTypeConverter)CvtStringToBooleanArray,
	NULL, 0, XtCacheAll | XtCacheRefCount, BooleanArrayDestructor);
}

static Boolean
CvtStringToStringArray(Display *dpy, XrmValuePtr args, Cardinal *num_args,
		XrmValuePtr from, XrmValuePtr to, XtPointer *data)
{
    static String *array;
    String s = from->addr;

    if(s == NULL || *s== '\0') {
	array = NULL;
    }
    else {
	String tok, c, buf;
	int n;

	buf = strdup(s);
	array = (String *)malloc(sizeof(String));
	n = 0;
	tok = buf;
	while((c = strtok(tok, ",")) != (String)NULL) {
	    tok = (String)NULL;
	    array = (String *)realloc(array, (n+1)*sizeof(String));
	    array[n++] = strdup(stringTrim(c));
	}
	array = (String *)realloc(array, (n+1)*sizeof(String));
	array[n++] = (String)NULL;
	free(buf);
    }
    if (to->addr == NULL) to->addr = (char*)(XtPointer) &array;
    else *(String **) to->addr = array;
    to->size = sizeof(String *);

    return True;
}

/*
 * Free the string array allocated by the String to StringArray converter
 */
static void
StringArrayDestructor(
    XtAppContext app,
    XrmValuePtr to,
    XtPointer converter_data,
    XrmValuePtr args,
    Cardinal *num_args)
{
    String *array = *(String **) to->addr;
    String *entry;

    if(array == NULL) return;

    for (entry = array; *entry != NULL; entry++) XtFree((char*)(XtPointer) *entry);

    XtFree((char*)(XtPointer) array);
}

static Boolean
CvtStringToBooleanArray(Display *dpy, XrmValuePtr args, Cardinal *num_args,
		XrmValuePtr from, XrmValuePtr to, XtPointer *data)
{
    static Boolean *array;
    String s = from->addr;

    if(s == NULL || *s== '\0') {
	array = NULL;
    }
    else {
	String tok, c, buf;
	int n;

	buf = strdup(s);
	array = (Boolean *)malloc(sizeof(Boolean));
	n = 0;
	tok = buf;
	while((c = strtok(tok, ", ")) != (String)NULL) {
	    tok = (String)NULL;
	    array = (Boolean *)realloc(array, (n+1)*sizeof(Boolean));
	    array[n++] = (!strcasecmp(c, "true")) ? True : False;
	}
	free(buf);
    }
    if (to->addr == NULL) to->addr = (char*)(XtPointer) &array;
    else *(Boolean **) to->addr = array;
    to->size = sizeof(Boolean *);

    return True;
}

/*
 * Free the string array allocated by the String to StringArray converter
 */
static void
BooleanArrayDestructor(XtAppContext app, XrmValuePtr to,
		XtPointer converter_data, XrmValuePtr args, Cardinal *num_args)
{
    Boolean *array = *(Boolean **) to->addr;

    if(array) XtFree((char*)(XtPointer) array);
}


static void
Initialize(Widget w_req, Widget w_new)
{
    MmTableWidget req = (MmTableWidget)w_req;
    MmTableWidget nou = (MmTableWidget)w_new;
    MmTablePart *t = &nou->mmTable;
    Arg	args[20];
    int	i, j, n, ncols;
    Size s;
    static XtCallbackRec hor_callbacks[] = {
	{(XtCallbackProc)horizontalScroll, (XtPointer)NULL},
	{(XtCallbackProc) NULL, (XtPointer)NULL}
    };
    static XtCallbackRec vert_callbacks[] = {
	{(XtCallbackProc)verticalScroll, (XtPointer)NULL},
	{(XtCallbackProc) NULL, (XtPointer)NULL}
    };

/*
    if(req->core.width == 0)
	nou->core.width = XWidthOfScreen(XtScreen(req))/2.5;
    if(req->core.height == 0)
	nou->core.height = XHeightOfScreen(XtScreen(req))/2;
*/

    if(t->radioSelect) t->singleSelect = True;
    /* no data displayed yet.
     */
    t->canvas =  (TCanvasWidget)XtVaCreateWidget("TCanvas", tCanvasWidgetClass,
	(Widget)nou,
        XtNtableWidget, nou,
/*        XtNbackground, t->table_background, */
        XtNsingleSelect, t->singleSelect,
        XtNradioSelect, t->radioSelect,
        XtNcntrlSelect, t->cntrlSelect,
        XtNdragSelect, t->dragSelect,
        NULL);

    if(!t->color_only) {
	t->header =  (HCanvasWidget)XtVaCreateWidget("HCanvas",
		hCanvasWidgetClass,
		(Widget)nou,
		XtNtableWidget, nou,
		XtNcanvasWidget, t->canvas,
		NULL);
    }
    else {
	t->header = NULL;
    }
    t->label =  XtVaCreateWidget("Title", xmLabelWidgetClass, (Widget)nou,
		XmNmarginHeight, 1,
		XmNshadowThickness, 0,
		XmNhighlightThickness, 0,
		NULL);
    t->row_column = XtVaCreateWidget("rc", xmRowColumnWidgetClass, (Widget)nou,
		XmNrowColumnType, XmMENU_BAR,
		XmNmarginTop, 0,
		XmNmarginBottom, 0,
		XmNmarginHeight, 0,
		XmNmarginWidth, 0,
		XmNborderWidth, 0,
		XmNspacing, 0,
		XmNshadowThickness, 2,
		XmNadjustMargin, False,
		XmNadjustLast, False,
		XmNresizePolicy, XmRESIZE_ANY,
		NULL);

t->cascade_button = NULL;
t->menu_button = NULL;
/* This does not work on some X-servers
    XtSetArg(args[0], XmNmarginWidth, 0);
    XtSetArg(args[1], XmNtearOffModel, XmTEAR_OFF_DISABLED);
    t->menu_button =  XmCreatePulldownMenu(t->row_column, "menu_button",
		args, 2);

    t->cascade_button = XtVaCreateManagedWidget("cascade",
		xmCascadeButtonWidgetClass, t->row_column,
		XmNmarginHeight, 0,
		XmNmarginWidth, 0,
		XmNmarginTop, 1,
		XmNmarginLeft, 1,
		XmNspacing, 0,
		XmNborderWidth, 0,
		XmNshadowThickness, 0,
		XmNlabelType, XmPIXMAP,
		XmNlabelPixmap, XmUNSPECIFIED_PIXMAP,
		XmNalignment, XmALIGNMENT_CENTER,
		XmNtearOffModel, XmTEAR_OFF_DISABLED,
		XmNresizePolicy, XmRESIZE_ANY,
		XmNsubMenuId, t->menu_button,
		NULL);
*/

    if(t->table_title != NULL) {
	t->table_title = strdup(t->table_title);
    }
    else {
	t->table_title = strdup("");
    }
    if(t->table_title[0] != '\0') {
	MmTable_drawTitle(nou);
    }

    t->hbar  = (Widget)NULL;
    t->vbar  = (Widget)NULL;
    t->nrows = 0;
    t->increases = 0;
    t->nhidden = 0;
    t->capacity = 0;
    t->max_ascent = t->max_descent = 0;
    t->row_height = 0;
    t->last_vbar_value = -1;
    t->last_hbar_value = -1;
    t->top = t->bottom = t->left = t->right = 0;
    t->edit_col = t->edit_row = t->edit_pos = t->edit_x = -1;
    t->select_row = t->select_col = -1;
    t->select_char1 = t->select_char2 = -1;
    t->select_x1 = t->select_x2 = -1;
    t->edit_string = strdup("");
    t->needAdjustColumns = 0;
    t->charWidths = (int *)NULL;
    t->edit_row_state = False;
    t->start_edit_cursor = False;
    t->have_focus = False;
    t->canvas_background = (Pixel)NULL;
    t->field_selection = (String)NULL;
    t->initialDisplayVerticalScrollbar = t->displayVerticalScrollbar;
    t->initialDisplayHorizontalScrollbar = t->displayHorizontalScrollbar;
    t->table_message = NULL;
    t->cursor_message = NULL;
    t->top_row = 0;
    t->need_layout = True;
    t->ignore_changed_managed = False;
    t->num_cell_choices = 0;
    t->cell_choice = NULL;
    t->pixmap_created = False;

    if(t->table_info != NULL)
    {
	t->table_info_text = InfoGetText(t->table_info);
    }
    else {
	t->table_info_text = strdup("");
    }

    if(t->cells != NULL && t->ncols <= 0) t->ncols = 1;

    ncols = (t->ncols > 0) ? t->ncols : 1;

    if( !(t->column_labels =(char **)MallocIt(nou,(t->ncols+1)*sizeof(char *))))
	return;

    if(req->mmTable.column_labels != (String *)NULL) {
    	for(j = 0; j < t->ncols; j++) {
	    if(req->mmTable.column_labels[j] != (String)NULL) {
		t->column_labels[j] =strdup(req->mmTable.column_labels[j]);
		stringTrim(t->column_labels[j]);
	    }
	    else {
		while(j < t->ncols) {
		    t->column_labels[j++] = strdup("");
		}
	    }
	}
    }
    else {
	for(j = 0; j < t->ncols; j++) {
	    t->column_labels[j] = strdup("");
	}
    }
    t->column_labels[t->ncols] = (String)NULL;

    if( !(t->column_choice =(char **)MallocIt(nou,(t->ncols+1)*sizeof(char *))))
	return;

    if(req->mmTable.column_choice != (String *)NULL) {
    	for(j = 0; j < t->ncols; j++) {
	    if(req->mmTable.column_choice[j] != (String)NULL) {
		t->column_choice[j] =strdup(req->mmTable.column_choice[j]);
		stringTrim(t->column_choice[j]);
	    }
	    else {
		while(j < t->ncols) {
		    t->column_choice[j++] = strdup("");
		}
	    }
	}
    }
    else {
	for(j = 0; j < t->ncols; j++) {
	    t->column_choice[j] = strdup("");
	}
    }
    t->column_choice[t->ncols] = (String)NULL;

    if( !(t->col_editable =
	(Boolean *)MallocIt(nou, (t->ncols+1)*sizeof(Boolean)))) return;

    if(req->mmTable.col_editable) {
    	for(j = 0; j < t->ncols; j++) {
	    t->col_editable[j] = req->mmTable.col_editable[j];
	}
    }
    else {
    	for(j = 0; j < t->ncols; j++) t->col_editable[j] = True;
    }

    t->displayRowLabels = False;
    t->newRowLabels = False;

    t->displayRowLabels = False;
    t->newRowLabels = False;

    if(!(t->col_width = (int *)MallocIt(nou, ncols*sizeof(int)))) return;
    if(!(t->col_order = (int *)MallocIt(nou, ncols*sizeof(int)))) return;
    if(!(t->column_state = (Boolean *)MallocIt(nou, ncols*sizeof(Boolean))))
	return;
    if(!(t->col_time = (enum TimeFormat *)MallocIt(nou,
		ncols*sizeof(enum TimeFormat)))) return;
    if(!(t->col_color = (Pixel *)MallocIt(nou, ncols*sizeof(Pixel)))) return;
    if(!(t->col_alignment = (int *)MallocIt(nou, ncols*sizeof(int)))) return;
    if(!(t->col_beg = (int *)MallocIt(nou, ncols*sizeof(int)))) return;
    if(!(t->col_end = (int *)MallocIt(nou, ncols*sizeof(int)))) return;
    if(!(t->columns = (char ***)MallocIt(nou, ncols*sizeof(String *)))) return;
    if(!(t->highlight = (Boolean **)MallocIt(nou, ncols*sizeof(Boolean *))))
	return;
    if(!(t->cell_fill = (Pixel **)MallocIt(nou, ncols*sizeof(Pixel *)))) return;
    if(!(t->backup = (char ***)MallocIt(nou, ncols*sizeof(String *)))) return;
    if(!(t->col_nchars = (int *)MallocIt(nou, ncols*sizeof(int)))) return;

    n = t->incremental_capacity;

    for(j = 0; j < t->ncols; j++) {
	t->col_width[j] = (t->min_col_width > 0) ? t->min_col_width : 50;
	t->col_order[j] = j;
	t->column_state[j] = False;
	t->col_time[j] = NOT_TIME;
	t->col_color[j] = (Pixel)NULL;
	t->col_alignment[j] = LEFT_JUSTIFY;

	if(!(t->columns[j] = (char **)MallocIt(nou, n*sizeof(String)))) return;
	if(!(t->highlight[j] = (Boolean *)MallocIt(nou, n*sizeof(Boolean))))
		return;
	if(!(t->cell_fill[j] = (Pixel *)MallocIt(nou, n*sizeof(Pixel)))) return;
	if(!(t->backup[j] = (char **)MallocIt(nou, n*sizeof(String)))) return;

	for(i = 0; i < n; i++) {
	    t->highlight[j][i] = False;
	    t->cell_fill[j][i] = CELL_NO_FILL_FLAG;
	    t->columns[j][i] = (String)NULL;
	    t->backup[j][i] = (String)NULL;
	}
	t->col_nchars[j] = 0;
    }
    t->capacity = t->incremental_capacity;

/*
    if(req->mmTable.columnEditable != (String *)NULL)
    {
	int m = 0;
	while(m < req->mmTable.ncols && m < t->ncols && 
	    req->mmTable.columnEditable[m] != (String)NULL)
	{
	    nou->mmTable.col_editable[m] = (!strcasecmp(
		req->mmTable.columnEditable[m], "True")) ? True : False;
	    m++;
	}
    }
*/

    MmTableResetColumnLimits(nou);

    if(!(t->row_order = (int *)MallocIt(nou, n*sizeof(int)))) return;
    if(!(t->display_order = (int *)MallocIt(nou, n*sizeof(int)))) return;
    if(!(t->row_state = (Boolean *)MallocIt(nou, n*sizeof(Boolean)))) return;
    if(!(t->row_editable = (Boolean *)MallocIt(nou, n*sizeof(Boolean)))) return;
    if(!(t->row_labels = (char **)MallocIt(nou, n*sizeof(String)))) return;
    for(i = 0; i < n; i++) t->row_labels[i] = NULL;
		
    /* commented out until cut/copy/paste is working. Causes a memory leak.
    XmuInternAtom(XtDisplay(nou), XmuMakeAtom("NULL"));
    */

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNincrementCallback, hor_callbacks); n++;
    XtSetArg(args[n], XmNdecrementCallback, hor_callbacks); n++;
    XtSetArg(args[n], XmNpageIncrementCallback, hor_callbacks); n++;
    XtSetArg(args[n], XmNpageDecrementCallback, hor_callbacks); n++;
    XtSetArg(args[n], XmNdragCallback, hor_callbacks); n++;
    XtSetArg(args[n], XmNvalueChangedCallback, hor_callbacks); n++;

    t->hbar =  XmCreateScrollBar(w_new, (char *)"scrollHoriz", args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNincrementCallback, vert_callbacks); n++;
    XtSetArg(args[n], XmNdecrementCallback, vert_callbacks); n++;
    XtSetArg(args[n], XmNpageIncrementCallback, vert_callbacks); n++;
    XtSetArg(args[n], XmNpageDecrementCallback, vert_callbacks); n++;
    XtSetArg(args[n], XmNdragCallback, vert_callbacks); n++;
    XtSetArg(args[n], XmNvalueChangedCallback, vert_callbacks); n++;

    t->vbar =  XmCreateScrollBar(w_new, (char *)"scrollVert", args, n);

    doLayout(nou, False);

    if(t->cells != NULL) {
	int i, nrows;
	if(t->ncols <= 0) t->ncols = 1;
	nrows = 0;
	for(i = 0; t->cells[i] != NULL; i++) nrows++;
	nrows /= t->ncols;
	for(i = 0; i < nrows; i++) {
	    MmTableAddRow(nou, t->cells+t->ncols*i, False);
	}

	if(nrows > 0 && req->mmTable.row_labels != (String *)NULL)
	{
	    const char **labels = NULL;
	    if(!(labels = (const char **)MallocIt(nou,
				nrows*sizeof(const char *)))) return;

	    for(i = 0; req->mmTable.row_labels[i] != NULL && i < nrows; i++) {
		labels[i] = strdup(stringTrim(req->mmTable.row_labels[i]));
	    }
	    for(; i < nrows; i++) labels[i] = strdup("");
	    MmTableSetRowLabels(nou, labels);
	    for(i = 0; i < nrows; i++) free((char *)labels[i]);
	    Free(labels);
	}
    }
    MmTableAdjustColumns(nou);

    s = MmTableGetPreferredSize(nou);
    if(nou->core.width == 0) nou->core.width = s.width;
    if(nou->core.height == 0) nou->core.height = s.height;

    t->toggle_label = (req->mmTable.toggle_label != NULL) ?
		strdup(req->mmTable.toggle_label) : strdup("");
    t->toggle_label_width = 0;

    t->gc = NULL;
    t->highlightGC = NULL;

    t->sort_unique.cols = NULL;
    t->sort_unique.cols_length = 0;

    t->num_cell_non_editable = 0;
    t->cell_non_editable = NULL;

    t->cursor_row = -1;
    t->cursor_col = -1;

    if(t->color_only) {
	t->epoch_time = timeGetEpoch();
	t->large_tdel = 60.*60.; // 1 hour
	t->n_small_tdel = 4;
	t->small_tdel = t->large_tdel/t->n_small_tdel; // 15 minutes
    }
    else {
	t->epoch_time = 0.;
	t->large_tdel = 0.;
	t->n_small_tdel = 1;
	t->small_tdel = 0.;
    }
    t->app_context = NULL;
}

void Table::setAppContext(XtAppContext app)
{
    tw->mmTable.app_context = app;
}


Widget
MmTableMenuChild(MmTableWidget w)
{
    return w->mmTable.menu_button;
}

static void
Realize(Widget widget, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    MmTableWidget w = (MmTableWidget)widget;
    Widget  XmCreateScrollBar();
    Widget  children[10];
    int     num_children;
    Pixel   white, black;
    MmTablePart *t = &w->mmTable;

    (*((mmTableWidgetClass->core_class.superclass)->core_class.realize))
	((Widget)w,  valueMask, attrs);

    t->gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);
    t->highlightGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, 0);

    XSetFont(XtDisplay(w), t->gc, t->font->fid);
    XSetFont(XtDisplay(w), t->highlightGC, t->font->fid);

    num_children = 0;

    if(t->displayHorizontalScrollbar) {
	children[num_children++] = t->hbar;
    }

    if(t->displayVerticalScrollbar) {
	children[num_children++] = t->vbar;
    }

    children[num_children++] = (Widget)t->canvas;

    if(t->header) {
	children[num_children++] = (Widget)t->header;
    }

    w->core.background_pixel = t->canvas->core.background_pixel;

    XtManageChildren(children, num_children);

//    w->core.background_pixel = t->header->core.background_pixel;
    w->core.background_pixel = t->canvas->core.background_pixel;
    XSetWindowBackground(XtDisplay(w), XtWindow(w), w->core.background_pixel);
    /*
     * check that no foreground colors are same as background
     */
    white = string_to_pixel(widget, "White");
    black = string_to_pixel(widget, "Black");
    if(t->fg == w->core.background_pixel) {
	t->fg = (w->core.background_pixel != black) ?  black : white;
    }
    if(t->highlight_color == w->core.background_pixel) {
	t->highlight_color = (w->core.background_pixel != black) ?
		black : white;
    }

    XSetForeground(XtDisplay(w), t->gc, t->fg);
    XSetBackground(XtDisplay(w), t->gc, w->core.background_pixel);
    XSetForeground(XtDisplay(w), t->highlightGC, t->highlight_color);
    XSetBackground(XtDisplay(w), t->highlightGC, w->core.background_pixel);

/*    systemClipboard = getToolkit().getSystemClipboard(); */

    if(t->top_row > 0) {
	MmTableSetBarValue(w->mmTable.vbar, t->top_row);
	_TCanvasVScroll(w->mmTable.canvas);
    }
}

static Boolean
SetValues(MmTableWidget cur, MmTableWidget req, MmTableWidget nou)
{
    int i, j;
    MmTablePart *t = &nou->mmTable;
    Boolean redisplay = False;

    if(cur->mmTable.display_menu_button != req->mmTable.display_menu_button) {
 	doLayout(nou, False);
	redisplay = True;
    }
    else if(cur->mmTable.table_title !=NULL && req->mmTable.table_title==NULL) {
	if(cur->mmTable.table_title[0] != '\0') {
	    nou->mmTable.table_title = strdup("");
	    doLayout(nou, False);
	    redisplay = True;
	}
	Free(cur->mmTable.table_title);
    }
    else if(cur->mmTable.table_title == NULL
		&& req->mmTable.table_title != NULL) {
	nou->mmTable.table_title = strdup(req->mmTable.table_title);
	if(req->mmTable.table_title[0] !='\0') {
	    MmTable_drawTitle(nou);
	    doLayout(nou, False);
	    redisplay = True;
	}
    }
    else if(cur->mmTable.table_title != NULL
		&& req->mmTable.table_title != NULL) {
	if(strcmp(cur->mmTable.table_title, req->mmTable.table_title)) {
	    nou->mmTable.table_title = strdup(req->mmTable.table_title);
	    if(	(cur->mmTable.table_title[0] != '\0'
			&& req->mmTable.table_title[0] =='\0')
	     || (cur->mmTable.table_title[0] == '\0'
			&& req->mmTable.table_title[0] !='\0'))
	    {
		MmTable_drawTitle(nou);
		doLayout(nou, False);
		redisplay = True;
	    }
	    else {
		MmTable_drawTitle(nou);
	    }
	    Free(cur->mmTable.table_title);
	}
    }
    if(cur->mmTable.font != req->mmTable.font) {
	nou->mmTable.font = req->mmTable.font;
        XSetFont(XtDisplay(nou), t->gc, t->font->fid);
        XSetFont(XtDisplay(nou), t->highlightGC, t->font->fid);
	XSetFont(XtDisplay(nou), t->canvas->tCanvas.gc, t->font->fid);
	if(t->header) {
	    XSetFont(XtDisplay(nou), t->header->hCanvas.gc, t->font->fid);
	}
	/* force row_height recompute */
	t->row_height = 0;
	doLayout(nou, False);
	redisplay = True;
    }
    if(cur->mmTable.ncols > req->mmTable.ncols)
    {
	for(j = req->mmTable.ncols; j < cur->mmTable.ncols; j++) {
	    for(i = 0; i < cur->mmTable.nrows; i++) {
		Free(cur->mmTable.columns[j][i]);
		Free(cur->mmTable.backup[j][i]);
	    }
	    Free(cur->mmTable.columns[j]);
	    Free(cur->mmTable.highlight[j]);
	    Free(cur->mmTable.cell_fill[j]);
	    Free(cur->mmTable.backup[j]);
	    Free(cur->mmTable.column_labels[j]);
	    Free(cur->mmTable.column_choice[j]);
	}
    	MmTableResetColumnLimits(nou);
    }
    else if(cur->mmTable.ncols < req->mmTable.ncols)
    {
	Boolean b;
	MmTableEditModeOff(nou);

	t->col_order = (int *)ReallocIt(nou, t->col_order,t->ncols*sizeof(int));
	t->col_beg = (int *)ReallocIt(nou, t->col_beg, t->ncols*sizeof(int));
	t->col_end = (int *)ReallocIt(nou, t->col_end, t->ncols*sizeof(int));
	t->col_width = (int *)ReallocIt(nou, t->col_width,t->ncols*sizeof(int));
	t->col_nchars =(int *)ReallocIt(nou,t->col_nchars,t->ncols*sizeof(int));

	b = (req->mmTable.column_labels == cur->mmTable.column_labels)
		? True : False;
	nou->mmTable.column_labels = cur->mmTable.column_labels;
	t->column_labels = (char **)ReallocIt(nou, t->column_labels,
				(t->ncols+1)*sizeof(String));
	cur->mmTable.column_labels = t->column_labels;
	t->column_labels[t->ncols] = NULL;
	if(b) req->mmTable.column_labels = cur->mmTable.column_labels;

	b = (req->mmTable.column_choice == cur->mmTable.column_choice)
		? True : False;
	nou->mmTable.column_choice = cur->mmTable.column_choice;
	t->column_choice = (char **)ReallocIt(nou, t->column_choice,
				(t->ncols+1)*sizeof(String));
	cur->mmTable.column_choice = t->column_choice;
	t->column_choice[t->ncols] = NULL;
	if(b) req->mmTable.column_choice = cur->mmTable.column_choice;

	t->column_state = (Boolean *)ReallocIt(nou, t->column_state,
				t->ncols*sizeof(Boolean));
	t->col_time = (enum TimeFormat *)ReallocIt(nou, t->col_time,
				t->ncols*sizeof(enum TimeFormat));
	t->col_color = (Pixel *)ReallocIt(nou, t->col_color,
				t->ncols*sizeof(Pixel));
	t->col_editable = (Boolean *)ReallocIt(nou, t->col_editable,
				t->ncols*sizeof(Boolean));
	t->col_alignment = (int *)ReallocIt(nou, t->col_alignment,
				t->ncols*sizeof(int));
	t->columns = (char ***)ReallocIt(nou, t->columns,
				t->ncols*sizeof(char **));
	t->highlight = (Boolean **)ReallocIt(nou, t->highlight,
				t->ncols*sizeof(Boolean *));
	t->cell_fill = (Pixel **)ReallocIt(nou, t->cell_fill,
				t->ncols*sizeof(Pixel *));
	t->backup = (char ***)ReallocIt(nou, t->backup,
				t->ncols*sizeof(char **));

	for(j = cur->mmTable.ncols; j < req->mmTable.ncols; j++) {
	    t->column_labels[j] = strdup("");
	    t->column_choice[j] = strdup("");
	    t->col_width[j] = (t->min_col_width > 0) ? t->min_col_width : 50;
    	    t->col_order[j] = j;
    	    t->col_color[j] = (Pixel)NULL;
	    t->column_state[j] = False;
	    t->col_time[j] = NOT_TIME;
	    t->col_editable[j] = True;
    	    t->col_alignment[j] = LEFT_JUSTIFY;
	    t->col_nchars[j] = 0;

	    t->columns[j] = (char **)MallocIt(nou, t->capacity*sizeof(String));
	    t->highlight[j] = (Boolean *)MallocIt(nou,
			t->capacity*sizeof(Boolean));
	    t->cell_fill[j] = (Pixel *)MallocIt(nou, t->capacity*sizeof(Pixel));
	    t->backup[j] = (char **)MallocIt(nou, t->capacity*sizeof(String));

	    for(i = 0; i < cur->mmTable.nrows + cur->mmTable.nhidden; i++) {
		t->columns[j][i] = strdup("");
		t->highlight[j][i] = False;
		t->cell_fill[j][i] = CELL_NO_FILL_FLAG;
		t->backup[j][i] = (String)NULL;
	    }
	}
    	MmTableResetColumnLimits(nou);
    }
    if(cur->mmTable.column_labels != req->mmTable.column_labels)
    {
	int nlabels = 0;
	for(j = 0; j < nou->mmTable.ncols; j++) {
	    t->col_width[j] = (t->min_col_width > 0) ? t->min_col_width : 50;
    	    t->col_order[j] = j;
    	    t->col_color[j] = (Pixel)NULL;
	    t->column_state[j] = False;
	    t->col_time[j] = NOT_TIME;
	    t->col_editable[j] = True;
    	    t->col_alignment[j] = LEFT_JUSTIFY;
	}
	if(req->mmTable.column_labels != (String *)NULL) {
	    while(nlabels < req->mmTable.ncols  &&
		    req->mmTable.column_labels[nlabels] != (String)NULL) {
		nlabels++;
	    }
	}
	for(j = cur->mmTable.ncols; j < req->mmTable.ncols && j < nlabels; j++)
	{
	    free(t->column_labels[j]);
	    t->column_labels[j] = NULL;
	}
	for(j =  0; j < req->mmTable.ncols && j < nlabels; j++) {
	    if(j < cur->mmTable.ncols) Free(cur->mmTable.column_labels[j]);
	    cur->mmTable.column_labels[j] =
		strdup(req->mmTable.column_labels[j]);
	    stringTrim(cur->mmTable.column_labels[j]);
	}
	nou->mmTable.column_labels = cur->mmTable.column_labels;
	redisplay = True;
    }
    if(cur->mmTable.col_editable != req->mmTable.col_editable)
    {
	for(j = 0; j < nou->mmTable.ncols; j++) {
	    t->col_editable[j] = req->mmTable.col_editable[j];
	}
    }
    if(cur->mmTable.column_choice != req->mmTable.column_choice)
    {
	int num = 0;
	if(req->mmTable.column_choice != (String *)NULL) {
	    while(num < req->mmTable.ncols  &&
		    req->mmTable.column_choice[num] != (String)NULL) {
		num++;
	    }
	}
	for(j = cur->mmTable.ncols; j < req->mmTable.ncols && j < num; j++)
	{
	    free(t->column_choice[j]);
	    t->column_choice[j] = NULL;
	}
	for(j =  0; j < req->mmTable.ncols && j < num; j++) {
	    if(j < cur->mmTable.ncols) Free(cur->mmTable.column_choice[j]);
	    cur->mmTable.column_choice[j] =
		strdup(req->mmTable.column_choice[j]);
	    stringTrim(cur->mmTable.column_choice[j]);
	}
	nou->mmTable.column_choice = cur->mmTable.column_choice;
    }
/*
    if(cur->mmTable.columnEditable != req->mmTable.columnEditable)
    {
	int n = 0;
	if(req->mmTable.columnEditable != (String *)NULL) {
	    while(n < nou->mmTable.ncols  &&
		    req->mmTable.columnEditable[n] != (String)NULL) {
		nou->mmTable.col_editable[n] = (!strcasecmp(
		    req->mmTable.columnEditable[n], "True")) ? True : False;
		n++;
	    }
	}
    }
*/
    if(cur->mmTable.table_info != req->mmTable.table_info)
    {
	if(req->mmTable.table_info != NULL)
	{
	    free(nou->mmTable.table_info_text);
	    nou->mmTable.table_info_text = InfoGetText(req->mmTable.table_info);
	}
    }
    if(req->mmTable.toggle_label != NULL
	&& strcmp(cur->mmTable.toggle_label, req->mmTable.toggle_label))
    {
	free(cur->mmTable.toggle_label);
	nou->mmTable.toggle_label = strdup(req->mmTable.toggle_label);
	doLayout(nou, False);
	redisplay = True;
    }
    if(req->mmTable.singleSelect != cur->mmTable.singleSelect ||
	req->mmTable.dragSelect != cur->mmTable.dragSelect ||
	req->mmTable.cntrlSelect != cur->mmTable.cntrlSelect)
    {
	MmTablePart *tc = &nou->mmTable;
	tc->singleSelect = req->mmTable.singleSelect;
	tc->dragSelect = req->mmTable.dragSelect;
	tc->cntrlSelect = req->mmTable.cntrlSelect;
	tc->canvas->tCanvas.singleSelect = req->mmTable.singleSelect;
	tc->canvas->tCanvas.dragSelect = req->mmTable.dragSelect;
	tc->canvas->tCanvas.cntrlSelect = req->mmTable.cntrlSelect;
	_TCanvasSetTranslations(nou->mmTable.canvas);
    }
    if(redisplay) {
        t->needAdjustColumns = 1;
    }
    return redisplay;
}

static void
MmTable_drawTitle(MmTableWidget nou)
{
	XmString xm;
	Arg args[1];

	xm = XmStringCreateLtoR(nou->mmTable.table_title,
			XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[0], XmNlabelString, xm);
	XtSetValues(nou->mmTable.label, args, 1);
	XmStringFree(xm);
}

static XtGeometryResult
query_geometry(Widget wid, XtWidgetGeometry *proposed, XtWidgetGeometry *answer)
{
    MmTableWidget widget = (MmTableWidget)wid;
    Size s;

    proposed->request_mode &= CWWidth | CWHeight;
    if(proposed->request_mode == 0) {
	return XtGeometryYes;
    }

    s = MmTableGetPreferredSize(widget);

    answer->width = s.width;
    answer->height = s.height;
    answer->request_mode = CWWidth | CWHeight;

    if((proposed->request_mode & CWWidth) && proposed->width != s.width)
    {
	return XtGeometryNo;
    }
    else if((proposed->request_mode & CWHeight) && proposed->height != s.height)
    {
	return XtGeometryNo;
    }
    return XtGeometryYes;
}

Size
MmTableGetPreferredSize(MmTableWidget widget)
{
    MmTablePart *t = &widget->mmTable;
    Size hd, cn, s;
    int label_width, title_height, title_width, menu_button_height;

    title_height = (t->table_title && t->table_title[0] != '\0') ?
			t->label->core.height + 2 : 0;
    title_width = (t->table_title && t->table_title[0] != '\0') ?
			t->label->core.width + 2 : 0;
    menu_button_height = (t->display_menu_button) ?
			t->row_column->core.height : 0;
    if(title_height < menu_button_height) title_height = menu_button_height;

    MmTableAdjustColumns(widget);

    if(t->header) {
	hd = HCanvasGetPreferredSize(t->header);
    }
    else {
	hd.width = hd.height = 0;
    }
    cn = TCanvasGetPreferredSize(t->canvas);

    s.width = (hd.width > cn.width) ? hd.width : cn.width;
    if(title_width > s.width) s.width = title_width;
    s.height = title_height + hd.height + cn.height;
    if(t->hbar != NULL) {
	s.height += t->hbar->core.height;
    }
    if(t->vbar != NULL) {
	s.width += t->vbar->core.width;
    }

    label_width = 0;
    if(t->displayRowLabels) {
        int i;
    	for(i = 0; i < t->nrows; i++) {
	    String label = t->row_labels[t->row_order[i]];
	    if(label && label[0] != '\0') {
		int label_w = stringWidth(widget, label);
		if(label_w > label_width) label_width = label_w;
	    }
	}
	if(label_width > 0) label_width += 4;
    }
    if(t->select_toggles) {
	label_width += 16;
    }

    s.width += label_width;
    return s;
}

static XtGeometryResult
GeometryManager(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *reply)
{
    return XtGeometryNo;
}

static void
ChangeManaged(Widget widget)
{
    MmTableWidget w = (MmTableWidget)widget;

    if(!w->mmTable.ignore_changed_managed) {
	doLayout(w, False);
    }
}

static void
Destroy(Widget widget)
{
    MmTableWidget w = (MmTableWidget)widget;
    MmTablePart *t = &w->mmTable;
    int i, j;

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	for(j = 0; j < t->ncols; j++) {
	    free(t->columns[j][i]);
	    Free(t->backup[j][i]);
	}
	Free(t->row_labels[i]);
    }

    for(j = 0; j < t->ncols; j++) {
	free(t->column_labels[j]);
	free(t->column_choice[j]);
    }
    Free(t->column_labels);
    Free(t->column_choice);
    Free(t->table_title);

    Free(t->col_width);
    Free(t->col_order);
    Free(t->column_state);
    Free(t->col_time);
    Free(t->col_color);
    Free(t->col_editable);
    Free(t->col_alignment);
    Free(t->col_beg);
    Free(t->col_end);
    Free(t->col_nchars);
    for(j = 0; j < t->ncols; j++) {
	Free(t->columns[j]);
	Free(t->highlight[j]);
	Free(t->cell_fill[j]);
	Free(t->backup[j]);
    }
    Free(t->columns);
    Free(t->highlight);
    Free(t->cell_fill);
    Free(t->backup);

    Free(t->row_order);
    Free(t->display_order);
    Free(t->row_state);
    Free(t->row_editable);
    Free(t->row_labels);
    Free(t->table_info_text);
    Free(t->toggle_label);


    Free(t->edit_string);

    if(t->gc != NULL) XFreeGC(XtDisplay(w), t->gc);
    if(t->highlightGC != NULL) XFreeGC(XtDisplay(w), t->highlightGC);
}

static void
Resize(Widget widget)
{
    MmTableWidget w = (MmTableWidget)widget;
    if(!XtIsRealized((Widget)w)) return;
    if(widget->core.width <= 0 || widget->core.height <= 0) return;
    doLayout(w, False);
    MmTableAdjustColumns(w);
}

static void
doLayout(MmTableWidget widget, Boolean vertScrollOnly)
{
    Size hd, cn;
    Dimension hbar_height, vbar_width, w, h, width, height, label_width;
    MmTablePart *t = &widget->mmTable;
    int hbar_x, hbar_y, hbar_width, title_height, menu_button_height;
    int previous_hbar_width, x_margin;

    if(widget->core.width <= 0 || widget->core.height <= 0) return;

    t->ignore_changed_managed = True;
    t->need_layout = False;
    hbar_height = t->hbar->core.height;
    vbar_width = t->vbar->core.width;
    title_height = (t->table_title != NULL && t->table_title[0] != '\0') ?
			t->label->core.height : 0;
    setButtonPixmap(widget);
    menu_button_height = (t->display_menu_button) ?
			t->row_column->core.height : 0;
    if(title_height < menu_button_height) title_height = menu_button_height;

    if(t->header) {
	hd = HCanvasGetPreferredSize(t->header);
    }
    else {
	hd.width = hd.height = 0;
    }
/*    cn = TCanvasGetPreferredSize(t->canvas); */
    if(t->row_height == 0) {
        int direction;
        XCharStruct overall;
        XTextExtents(t->font, "M", 1, &direction, &t->max_ascent,
                &t->max_descent, &overall);
        t->row_height = t->max_ascent + t->max_descent + 4;
    }
    cn.width = (t->ncols > 0) ? t->margin + t->col_end[t->ncols-1] + 1 : 50;
    cn.height = t->nrows*t->row_height + 1 + t->bottom_margin;


    w = (hd.width > cn.width) ? hd.width : cn.width;
    h = hd.height + cn.height;

    label_width = 0;
    t->row_label_width = 0;
    if(t->displayRowLabels) {
        int i;
    	for(i = 0; i < t->nrows; i++) {
	    String label = t->row_labels[t->row_order[i]];
	    if(label && label[0] != '\0') {
		int label_w = stringWidth(widget, label);
		if(label_w > t->row_label_width) t->row_label_width = label_w;
	    }
	}
	if(t->row_label_width > 0) t->row_label_width += 4;
	label_width += t->row_label_width;
    }

    t->toggle_label_width = 0;

    if(t->select_toggles) {
	label_width += 16;
	if(t->toggle_label != NULL && strlen(t->toggle_label) > 0)
	{
	    int ascent, descent, direction;
	    XCharStruct overall;
	    XTextExtents(t->font, t->toggle_label, strlen(t->toggle_label),
			&direction, &ascent, &descent, &overall);
	    t->toggle_label_width = overall.width;
	    t->toggle_label_y = ascent + 2;
	    if(label_width < overall.width+2) {
		label_width = overall.width+2;
	    }
	}
    }

    w += label_width;

    /* w and h are the width and height needed if no scrollbars are displayed.
     * Check if scrollbars are needed.
     */
    if(!t->initialDisplayHorizontalScrollbar)
    {
	if(!t->initialDisplayVerticalScrollbar) {
	    if(t->ncols <= 0 || widget->core.width >= w) {
		t->displayHorizontalScrollbar = False;
	    }
	    else {
		t->displayHorizontalScrollbar = True;
	    }
	}
	else if(widget->core.width >= w + vbar_width) {
	    t->displayHorizontalScrollbar = False;
	}
	else {
	    t->displayHorizontalScrollbar = True;
	}
    }
    if(!t->initialDisplayVerticalScrollbar)
    {
	if(t->nrows <= 0) {
	    t->displayVerticalScrollbar = False;
	}
	else if(!t->displayHorizontalScrollbar)
	{
	    if(widget->core.height - title_height >= h) {
		t->displayVerticalScrollbar = False;
	    }
	    else {
		t->displayVerticalScrollbar = True;
		if(widget->core.width < w + vbar_width) {
		    t->displayHorizontalScrollbar = True;
		}
	    }
	}
	else if(widget->core.height - title_height >= h + hbar_height) {
	    t->displayVerticalScrollbar = False;
	}
	else {
	    t->displayVerticalScrollbar = True;
	}
    }

    if(t->displayVerticalScrollbar) {
	width = widget->core.width - vbar_width - label_width;
    }
    else {
	width = widget->core.width - label_width;
    }
    if(t->displayHorizontalScrollbar) {
	if(widget->core.height < title_height + hbar_height) return;
	height = widget->core.height - title_height - hbar_height;
    }
    else {
	if(widget->core.height < title_height) return;
	height = widget->core.height - title_height;
    }
    if(t->color_only) {
	height -= 20;
    }

    if(t->displayVerticalScrollbar) w += vbar_width;

    if(t->centerHorizontally && !t->displayHorizontalScrollbar
	&& widget->core.width > w)
    {
	x_margin = (widget->core.width - w)/2;
	width -= 2*x_margin;
    }
    else {
	x_margin = 0;
    }
    if(t->display_menu_button) {
	XtConfigureWidget(t->row_column, 0, 0, t->row_column->core.width,
				t->row_column->core.height, 0);
    }

    if(!vertScrollOnly && t->table_title && t->table_title[0] != '\0') {
	int x = (t->display_menu_button) ? t->row_column->core.width : 0;
	XtConfigureWidget(t->label, x, 0, widget->core.width,
				t->label->core.height, 0);
    }

    if(t->displayVerticalScrollbar) w -= vbar_width;

    if(!vertScrollOnly) {
	if(width > w - label_width) width = w - label_width;
	if(t->header) {
	    XtConfigureWidget((Widget)t->header, x_margin + label_width,
		title_height, width, hd.height, 0);
	}
	if(height > hd.height) {
	    XtConfigureWidget((Widget)t->canvas, x_margin + label_width,
		hd.height + title_height, width,
		height - hd.height, 0);
	}
    }

    if(t->displayVerticalScrollbar)
    {
	if(t->displayHorizontalScrollbar) {
	    XtConfigureWidget((Widget)t->vbar, widget->core.width-vbar_width,
		title_height, vbar_width, height+hbar_height, 0);
	}
	else {
	    int x = x_margin + w;
	    XtConfigureWidget((Widget)t->vbar, x, title_height,
			vbar_width, height, 0);
	}
    }
    previous_hbar_width = t->hbar->core.width;

    hbar_x = label_width;
    hbar_y = widget->core.height - hbar_height;
    if(t->displayVerticalScrollbar) {
	hbar_width = widget->core.width - vbar_width - label_width;
    }
    else {
	hbar_width = widget->core.width - label_width;
    }

    if(!vertScrollOnly && t->displayHorizontalScrollbar) {
	XtConfigureWidget(t->hbar, hbar_x, hbar_y, hbar_width, hbar_height, 0);
    }

    if(t->row_height == 0 || t->nrows == 0) {
	t->top = 0;
	t->bottom = 0;
	setBarValues(t->vbar, t->top, 1, 0, 1);
    }
    else {
	int value, min, max;
	int visible = (height-hd.height)/t->row_height;
	if(visible > t->nrows) visible = t->nrows;
	setBarValues(t->vbar, t->top, visible, 0, t->nrows);

	getBarValues(t->vbar, &value, &visible, &min, &max);
	t->top = value;
	t->bottom = t->top + visible;
    }

    if(!vertScrollOnly)
    {
	if(t->display_menu_button && !XtIsManaged(t->row_column)) {
	    XtManageChild(t->row_column);
	}
	else if(!t->display_menu_button && XtIsManaged(t->row_column)) {
	    XtUnmanageChild(t->row_column);
	}
	if(t->table_title && t->table_title[0] != '\0' &&
		!XtIsManaged(t->label)) {
	    XtManageChild(t->label);
	}
	else if((!t->table_title || t->table_title[0] == '\0') &&
		XtIsManaged(t->label)) {
	    XtUnmanageChild(t->label);
	}
	if(t->displayHorizontalScrollbar && !XtIsManaged(t->hbar)) {
	    XtManageChild(t->hbar);
	}
	else if(!t->displayHorizontalScrollbar && XtIsManaged(t->hbar)) {
	    XtUnmanageChild(t->hbar);
	}
    }
    if(t->displayVerticalScrollbar && !XtIsManaged(t->vbar)) { 
	XtManageChild(t->vbar);
    }
    else if(!t->displayVerticalScrollbar && XtIsManaged(t->vbar)) {
	XtUnmanageChild(t->vbar);
    }

    if(!vertScrollOnly)
    {
	if(t->ncols == 0) {
	    t->left = 0;
	    t->right = 0;
	    setBarValues(t->hbar, 0, 1, 0, 1);
	}
	else if(previous_hbar_width != hbar_width)
	{
	    t->left = 0;
	    MmTableAdjustHScroll(widget);
	}
    }
    t->ignore_changed_managed = False;
}

static void
setButtonPixmap(MmTableWidget widget)
{
    MmTablePart *t = &widget->mmTable;
    Widget toplevel;
    MmTableClassPart *mc = &mmTableClassRec.mmTable_class;
    Arg args[2];
    Pixel bg, fg;
    int i, screen;
    unsigned int depth;
//    unsigned char bits[] = {0x18, 0x00, 0x38, 0x00, 0x78, 0x00, 0xf8, 0x00,
//                0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0x38, 0x00, 0x18, 0x00};
    unsigned char bits[] = {0x10, 0x00, 0x38, 0x00, 0x44, 0x00, 0xba, 0x00,
		  0xbb, 0x01, 0xba, 0x00, 0x44, 0x00, 0x38, 0x00, 0x10, 0x00};

    if(!t->display_menu_button || t->pixmap_created) return;

    for(toplevel = XtParent(widget); XtParent(toplevel) != NULL;
		toplevel = XtParent(toplevel));

    screen = DefaultScreen(XtDisplay(toplevel));

    for(i = 0; i < 10 && mc->sp[i].screen != -1 &&
		mc->sp[i].screen != screen; i++);
    if(i < 10 && mc->sp[i].screen == -1 && XtWindow(toplevel) != 0) {
        mc->sp[i].screen = screen;

        XtSetArg(args[0], XmNbackground, &bg);
        XtSetArg(args[1], XmNforeground, &fg);
        XtGetValues(t->cascade_button, args, 2);
        depth = DefaultDepth(XtDisplay(toplevel), screen);
        mc->sp[i].pixmap = XCreatePixmapFromBitmapData(XtDisplay(toplevel),
                XtWindow(toplevel), (char *)bits, 9, 9, fg, bg, depth);
    }
    if(i < 10 && mc->sp[i].pixmap != 0) {
	t->pixmap_created = True;
        XtSetArg(args[0], XmNlabelType, XmPIXMAP);
        XtSetArg(args[1], XmNlabelPixmap, mc->sp[i].pixmap);
        XtSetValues(t->cascade_button, args, 2);
    }
}


void
MmTableAdjustHScroll(MmTableWidget w)
{
    MmTablePart *t = &w->mmTable;
    int visible, max;
    int width = getWidth((Widget)t->canvas);
    int i;

    for(i = t->left; i < t->ncols; i++) {
	int x = t->col_end[t->ncols-1] - t->col_beg[i] + t->margin;
	if(x < width) break;
    }
    visible = t->ncols-i;
    if(visible == 0) visible = 1;
/*    int max = (use_1_0_scrollbar) ? ncols-visible : ncols; */
    max = (t->ncols > 0) ? t->ncols : 1;
    setBarValues(t->hbar, t->left, visible, 0, max);
}

static void
Redisplay(Widget widget, XEvent *event, Region region)
{
    MmTableWidget w = (MmTableWidget)widget;
    MmTablePart *t = &w->mmTable;
    TCanvasWidget tc = t->canvas;
    if(widget->core.width <= 0 || widget->core.height <= 0) return;

    if(t->needAdjustColumns == 1) {
	MmTableAdjustColumns(w);
    }
/*
    else if(t->needAdjustColumns == 2) {
	quickAdjustColumns(w);
    }
*/
    _TCanvasDrawRowLabels(tc);
    _TCanvasDrawXLabels(tc);
}

void Table::addFilledRow(Pixel *pixel, bool redraw)
{
    int i;
    char **row;
    MmTablePart *t = &tw->mmTable;

    if(!(row = (char **)MallocIt(tw, t->ncols*sizeof(String)))) return;
    for(i = 0; i < t->ncols; i++) row[i] = NULL;
    addRow((const char **)row, redraw);
    i = t->nrows + t->nhidden - 1;
    fillRow(i, pixel, redraw);
    Free(row);
}

void Table::addRow(vector<const char *> &row, bool redraw)
{
    char **s = (char **)MallocIt(tw, (int)row.size()*sizeof(char *));
    for(int i = 0; i < (int)row.size(); i++) s[i] = (char *)row[i];
    MmTableAddRowWithLabel(tw, s, NULL, redraw);
    Free(s);
}

void Table::addRow(vector<char *> &row, bool redraw)
{
    char **s = (char **)MallocIt(tw, (int)row.size()*sizeof(char *));
    for(int i = 0; i < (int)row.size(); i++) s[i] = row[i];
    MmTableAddRowWithLabel(tw, s, NULL, redraw);
    Free(s);
}

void Table::addRow(vector<string> &row, bool redraw)
{
    char **s = (char **)MallocIt(tw, (int)row.size()*sizeof(char *));
    for(int i = 0; i < (int)row.size(); i++) s[i] = (char *)row[i].c_str();
    MmTableAddRowWithLabel(tw, s, NULL, redraw);
    Free(s);
}

void Table::addRowWithLabel(const char **row, const char *label, bool redraw)
{
    MmTableAddRowWithLabel(tw, (char **)row, (char *)label, redraw);
}

static void
MmTableAddRow(MmTableWidget w, char **row, Boolean redraw)
{
    MmTableAddRowWithLabel(w, row, NULL, (bool)redraw);
}

static void
MmTableAddRowWithLabel(MmTableWidget w, char **row, char *label, bool redraw)
{
    int j, n, len;
    MmTablePart *t = &w->mmTable;

    MmTableEditModeOff(w);

    n = t->nrows + t->nhidden;
    if(n == t->capacity) { /* increase capacity */
	increaseCapacity(w);
    }

    for(j = 0; j < t->ncols; j++) {
	if(row[j] != (String)NULL) {
	    /* first remove bank spaces
	     */
	    String c = strdup(row[j]);
	    t->columns[j][n] = strdup(stringTrim(c));
	    free(c);
	}
	else {
	    t->columns[j][n] = strdup("");
	}
	t->highlight[j][n] = False;
	t->cell_fill[j][n] = CELL_NO_FILL_FLAG;
	t->backup[j][n] = (String)NULL;
	len = strlen(t->columns[j][n]);
	if(t->col_nchars[j] < len) t->col_nchars[j] = len;
    }
    t->row_state[n] = False;
    t->row_editable[n] = t->editable;
    /* move hidden rows down one */
    for(j = t->nrows + t->nhidden; j > t->nrows; j--) {
	t->row_order[j] = t->row_order[j-1];
    }
    t->row_order[t->nrows] = n;
    if(t->nhidden > 0) {
	updateDisplayOrder(t);
    }
    else {
	t->display_order[t->nrows] = t->nrows;
    }
    Free(t->row_labels[n]);
    if(label) {
	t->row_labels[n] = strdup(label);
	t->displayRowLabels = True;
    }
    t->nrows++;
    if(!redraw) {
	t->need_layout = True;
	return;
    }

    if(t->row_height == 0) {
	doLayout(w, False);
    }
    else {
	/* check if this row will be visible. */
	int value, visible, min, max, top, bottom;
	getBarValues(t->vbar, &value, &visible, &min, &max);
	top = value;
	bottom = top + visible;
	if((t->nrows-1) <= bottom) {
	    doLayout(w, False);	/* full layout */
	    _TCanvasRedisplay(t->canvas);
	}
	else {
	    int value, visible, min, max;
	    getBarValues(t->hbar, &value, &visible, &min, &max);
	    doLayout(w, True); /* adjust vertical scroll only */
	    setBarValues(t->hbar, value, visible, min, max);
	}
    }
    XtCallCallbacks((Widget)w, XtNrowChangeCallback, (XtPointer)t->nrows);
}

int Table::getRowHeight(void)
{
    MmTablePart *t = &tw->mmTable;

    if(t->row_height == 0) {
	int direction;
	XCharStruct overall;

	XTextExtents(t->font, "M", 1, &direction, &t->max_ascent,
			&t->max_descent, &overall);
	t->row_height = t->max_ascent + t->max_descent + 4;
    }
    return t->row_height;
}

void Table::moveRow(int row, int dir)
{
    MmTablePart *t = &tw->mmTable;
    char *c;
    Boolean b;
    int j;
    Pixel p;

    if(row < 0 || row >= t->nrows) return;
    if((dir == -1 && row == 0) || (dir == 1 && row == t->nrows-1)) return;

    for(j = 0; j < t->ncols; j++)
    {
	c = t->columns[j][row+dir];
	t->columns[j][row+dir] = t->columns[j][row];
	t->columns[j][row] = c;

	c = t->backup[j][row+dir];
	t->backup[j][row+dir] = t->backup[j][row];
	t->backup[j][row] = c;

	b = t->highlight[j][row+dir];
	t->highlight[j][row+dir] = t->highlight[j][row];
	t->highlight[j][row] = b;

	p = t->cell_fill[j][row+dir];
	t->cell_fill[j][row+dir] = t->cell_fill[j][row];
	t->cell_fill[j][row] = p;
    }

    b = t->row_state[row+dir];
    t->row_state[row+dir] = t->row_state[row];
    t->row_state[row] = b;

    c = t->row_labels[row+dir];
    t->row_labels[row+dir] = t->row_labels[row];
    t->row_labels[row] = c;

    b = t->row_editable[row+dir];
    t->row_editable[row+dir] = t->row_editable[row];
    t->row_editable[row] = b;

    for(j = 0; j < t->num_cell_choices; j++) {
	if(t->cell_choice[j].row == row) {
	    t->cell_choice[j].row += dir;
	}
	else if(t->cell_choice[j].row == row+dir) {
	    t->cell_choice[j].row -= dir;
	}
    }

    for(j = 0; j < t->num_cell_non_editable; j++) {
	if(t->cell_non_editable[j].row == row) {
	    t->cell_non_editable[j].row += dir;
	}
	else if(t->cell_non_editable[j].row == row+dir) {
	    t->cell_non_editable[j].row -= dir;
	}
    }

    if(t->header) _HCanvasRedisplay(t->header);
    _TCanvasRedisplay(t->canvas);
}

void Table::removeRow(int i)
{
    vector<int> v;
    v.push_back(i);
    removeRows(v);
}

void Table::removeRows(vector<int> &rows)
{
    int i, j, n, nrows, nhidden, *rows_sorted=NULL;
    int value, visible, min, max, top, bottom;
    MmTablePart *t = &tw->mmTable;

    if(t->ncols == 0 || (int)rows.size() <= 0) return;

    MmTableEditModeOff(tw);

    rows_sorted = new int[rows.size()];
    for(i = 0; i < (int)rows.size(); i++) rows_sorted[i] = rows[i];
    qsort(rows_sorted, (int)rows.size(), sizeof(int), sortDown);

    nrows = t->nrows;
    nhidden = t->nhidden;
    for(i = 0; i < (int)rows.size(); i++) {
	if(rows_sorted[i] >= 0 && rows_sorted[i] < t->nrows + t->nhidden)
	{
	    for(j = 0; j < t->ncols; j++) Free(t->columns[j][rows_sorted[i]]);
	    Free(t->row_labels[rows_sorted[i]]);
	    t->row_order[t->display_order[rows_sorted[i]]] = -1;
	    t->display_order[rows_sorted[i]] = -1;
	    if(rows_sorted[i] < t->nrows) nrows--;
	    else nhidden--;

	    for(j = 0; j < t->nrows + t->nhidden; j++) {
		if(t->row_order[j] > rows_sorted[i]) t->row_order[j]--;
	    }
	}
    }
    delete [] rows_sorted;

    n = 0;
    for(i = 0; i < t->nrows + t->nhidden; i++) {
	if(t->display_order[i] >= 0 ) {
	    for(j = 0; j < t->ncols; j++) {
		t->columns[j][n] = t->columns[j][i];
	    }
	    t->row_state[n] = t->row_state[i];
	    t->row_labels[n] = t->row_labels[i];
	    t->row_editable[n++] = t->row_editable[i];
	}
    }
    n = 0;
    for(i = 0; i < t->nrows + t->nhidden; i++) {
	if(t->row_order[i] >= 0 ) {
	    t->row_order[n++] = t->row_order[i];
	}
    }
    t->nrows = nrows;
    t->nhidden = nhidden;

    updateDisplayOrder(t);

    /* check if this row will be visible. */
    getBarValues(t->vbar, &value, &visible, &min, &max);
    top = value;
    bottom = top + visible;
    if((t->nrows-1) <= bottom) {
	doLayout(tw, False);	/* full layout */
	_TCanvasRedisplay(t->canvas);
    }
    else {
	doLayout(tw, True); /* adjust vertical scroll only */
    }
    XtCallCallbacks((Widget)tw, XtNrowChangeCallback, (XtPointer)t->nrows);
}

static int
sortDown(const void *A, const void *B)
{
    int *a = (int *)A;
    int *b = (int *)B;

    return (*a < *b) ? 1 : -1;
}


void Table::clear(void)
{
    Arg args[1];

    removeAllRows();
    XtSetArg(args[0], XtNcolumns,0);
    XtSetValues((Widget)tw, args, 1);
    MmTableAdjustColumns(tw);
}

void Table::removeAllRows(void)
{
    int i, j;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	for(j = 0; j < t->ncols; j++) {
	    Free(t->columns[j][i]);
	    Free(t->backup[j][i]);
	}
	Free(t->row_labels[i]);
    }

    t->nrows = 0;
    t->nhidden = 0;
    MmTableAdjustColumns(tw);
    XtCallCallbacks((Widget)tw, XtNrowChangeCallback, (XtPointer)t->nrows);
}

void MmTableSetFieldWithCB(MmTableWidget w, int row, int col, const char *s,
				bool redisplay, bool do_callback)
{
    MmTablePart *t = &w->mmTable;
    MmTableEditCallbackStruct callback;
    char *c, *choice=NULL;
    char *buf, *tok;
    bool valid_choice;

    if(row < 0 || row > t->nrows+t->nhidden || col < 0 || col >t->ncols) return;

/*
    if(!XtIsRealized((Widget)w)) return;

    if(t->row_editable[row] && t->col_editable[col]
		&& strcmp(s, t->columns[col][row]))
    {
	callback.column = col;
	callback.row = row;
	callback.old_string = strdup(t->columns[col][row]);
	callback.new_string = (char *)s;
	XtCallCallbacks((Widget)w, XtNmodifyVerifyCallback, &callback);
	free(callback.old_string);
    }
*/

    if((choice = MmTableGetCellChoice(w, row, col)) != NULL ||
		t->column_choice[col][0] != '\0')
    {
	if( !choice ) choice = t->column_choice[col];

	buf = strdup(choice);
	tok = buf;
	valid_choice = False;
	while((c = strtok(tok, ":")) != NULL) {
	    tok = NULL;
	    if(!strcmp(c, s)) { valid_choice = True; break; }
	}
	Free(buf);
	if( !valid_choice ) {
	    fprintf(stderr,
		"MmTableSetField: invalid choice row=%d column=%d s=%s\n",
			row, col, s);
	    return;
	}
    }
    if(do_callback) {
	callback.old_string = strdup(t->columns[col][row]);
	callback.column = col;
	callback.row = row;
	callback.new_string = (char *)s;
    }

    c = (s != (String)NULL) ? strdup(s) : strdup("");
    if(t->backup[col][row] == NULL) {
	t->backup[col][row] = t->columns[col][row];
    }
    else {
	Free(t->columns[col][row]);
    }
    t->columns[col][row] = c;
    if(redisplay) {
	MmTableAdjustColumnWidths(w, col, col);
    }

    if(do_callback) {
	if(choice) {
	    XtCallCallbacks((Widget)w, XtNchoiceChangedCallback, &callback);
	}
	else {
	    XtCallCallbacks((Widget)w, XtNvalueChangedCallback, &callback);
	}
	free(callback.old_string);
    }
}

void MmTableSetFields(MmTableWidget w, int num, int *rows, int *cols,
			const char **s)
{
    MmTablePart *t = &w->mmTable;
    Boolean need_col_adjust = False;
    char *c, *choice, *buf, *tok;
    int i;
    bool valid_choice;

    if(num <= 0) return;

    for(i = 0; i < num; i++)
    {
	if(rows[i] < 0 || rows[i] > t->nrows+t->nhidden ||
	    cols[i] < 0 || cols[i] >t->ncols) continue;

	if((choice = MmTableGetCellChoice(w, rows[i], cols[i])) != NULL
		|| t->column_choice[cols[i]][0] != '\0')
	{
	    if( !choice ) choice = t->column_choice[cols[i]];

	    buf = strdup(choice);
	    tok = buf;
	    valid_choice = False;
	    while((c = strtok(tok, ":")) != NULL) {
		tok = NULL;
		if(!strcmp(c, s[i])) { valid_choice = True; break; }
	    }
	    Free(buf);
	    if( !valid_choice ) {
		fprintf(stderr,
		    "MmTableSetFields: invalid choice row=%d column=%d s=%s\n",
			rows[i], cols[i], s[i]);
		continue;
	    }
	}

	c = (s[i] != (String)NULL) ? strdup(s[i]) : strdup("");
	if(t->backup[cols[i]][rows[i]] == NULL) {
	    t->backup[cols[i]][rows[i]] = t->columns[cols[i]][rows[i]];
	}
	else {
	    Free(t->columns[cols[i]][rows[i]]);
	}
	t->columns[cols[i]][rows[i]] = c;
	if(getNewColumnWidths(w, cols[i], cols[i])) need_col_adjust = True;
    }
    if(need_col_adjust)
    {
	MmTableResetColumnLimits(w);
	t->left = 0;
	MmTableAdjustHScroll(w);
	doLayout(w, False);
    }
    if(XtIsRealized((Widget)w)) {
	if(t->header) _HCanvasRedisplay(t->header);
	_TCanvasRedisplay(t->canvas);
    }
}

char * Table::getField(int row, int col)
{
    MmTablePart *t = &tw->mmTable;

    if(row < 0 || row > t->nrows+t->nhidden || col < 0 || col >t->ncols) {
	return (char *)NULL;
    }
    return strdup(t->columns[col][row]);
}

bool Table::findAndSetRow(int num_columns, int *col, const char **names,
			const char **row)
{
    char *c;
    int i, j, k;
    MmTablePart *t = &tw->mmTable;

    if(num_columns >= t->ncols) return False;

    MmTableEditModeOff(tw);

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	int j;
	for(j = 0; j < num_columns; j++) {
	    if(!strcmp(t->columns[col[j]][i], names[j])) break;
	}
	if(j == num_columns) break;
    }
    if(i == t->nrows + t->nhidden) return False;  /* couldn't find row */

    if(!XtIsRealized((Widget)tw)) return True;

    for(k = 0; k < t->ncols; k++) {
	int width = stringWidth(tw, row[k]) + t->cellMargin;
	if(width > t->col_width[k]) break;
    }
    for(j = 0; j < t->ncols; j++) {
	c = (row[j] != (String)NULL) ? strdup(row[j]) : strdup("");
	Free(t->columns[j][i]);
	t->columns[j][i] = c;
    }
    if(k < t->ncols) {
	MmTableAdjustColumns(tw);
    }
    else if(i >= t->top && i <= t->bottom) {
	_TCanvasUpdateRow(t->canvas, i);
    }
    return True;
}

void Table::addColumn(const char **col, const string &label, bool isEditable,
			int alignment)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    t->ncols++;
    t->col_order = (int *)ReallocIt(tw, t->col_order, t->ncols*sizeof(int));
    t->col_order[t->ncols-1] = t->ncols-1;

    t->col_beg = (int *)ReallocIt(tw, t->col_beg, t->ncols*sizeof(int));
    t->col_end = (int *)ReallocIt(tw, t->col_end, t->ncols*sizeof(int));
    t->col_width = (int *)ReallocIt(tw, t->col_width, t->ncols*sizeof(int));

    t->column_labels = (char **)ReallocIt(tw, t->column_labels,
				t->ncols*sizeof(String));
    t->column_labels[t->ncols-1] = strdup(label.c_str());

    t->column_choice = (char **)ReallocIt(tw, t->column_choice,
				t->ncols*sizeof(String));
    t->column_choice[t->ncols-1] = strdup("");

    t->column_state = (Boolean *)ReallocIt(tw, t->column_state,
				t->ncols*sizeof(Boolean));
    t->column_state[t->ncols-1] = False;

    t->col_time = (enum TimeFormat *)ReallocIt(tw, t->col_time,
				t->ncols*sizeof(enum TimeFormat));
    t->col_time[t->ncols-1] = NOT_TIME;

    t->col_color = (Pixel *)ReallocIt(tw, t->col_color, t->ncols*sizeof(Pixel));
    t->col_color[t->ncols-1] = (Pixel)NULL;

    t->col_editable = (Boolean *)ReallocIt(tw, t->col_editable,
				t->ncols*sizeof(Boolean));
    t->col_editable[t->ncols-1] = isEditable;

    t->col_alignment = (int *)ReallocIt(tw, t->col_alignment,
				t->ncols*sizeof(int));
    t->col_alignment[t->ncols-1] = alignment;

    t->col_nchars = (int *)ReallocIt(tw, t->col_nchars, t->ncols*sizeof(int));
    t->col_nchars[t->ncols-1] = 0;

    t->columns = (char ***)ReallocIt(tw, t->columns, t->ncols*sizeof(String *));
    if(!(t->columns[t->ncols-1] = (char **)MallocIt(tw,
		t->capacity*sizeof(String)))) return;
    for(i = 0; i < t->nrows; i++) {
	t->columns[t->ncols-1][i] = strdup(col[i]);
    }

    t->highlight = (Boolean **)ReallocIt(tw, t->highlight,
				t->ncols*sizeof(Boolean *));
    if(!(t->highlight[t->ncols-1] = (Boolean *)MallocIt(tw,
		t->capacity*sizeof(Boolean)))) return;
    for(i = 0; i < t->nrows; i++) {
	t->highlight[t->ncols-1][i] = False;
    }
    t->cell_fill = (Pixel **)ReallocIt(tw, t->cell_fill,
				t->ncols*sizeof(Pixel *));
    if(!(t->cell_fill[t->ncols-1] = (Pixel *)MallocIt(tw,
		t->capacity*sizeof(Pixel)))) return;
    for(i = 0; i < t->nrows; i++) {
	t->cell_fill[t->ncols-1][i] = CELL_NO_FILL_FLAG;
    }
    t->backup = (char ***)ReallocIt(tw, t->backup, t->ncols*sizeof(String *));
    if(!(t->backup[t->ncols-1] = (char **)MallocIt(tw,
		t->capacity*sizeof(String)))) return;

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	t->backup[t->ncols-1][i] = (String)NULL;
    }

    getNewColumnWidths(tw, t->ncols-1, t->ncols-1);
    MmTableResetColumnLimits(tw);
    MmTableAdjustHScroll(tw);

    if(t->hbar && XtIsManaged(t->hbar)) {
	int value, visible, min, max;
	getBarValues(t->hbar, &value, &visible, &min, &max);
	if(value == max-visible-1) {
	    value++;
	    MmTableSetBarValue(t->hbar, value);
	    _TCanvasHScroll(t->canvas);
	    getBarValues(t->hbar, &value, &visible, &min, &max);
	    t->last_hbar_value = value;
	}
    }
    else {
	doLayout(tw, False);
	_TCanvasRedisplay(t->canvas);
    }
}

void Table::rollColumn(const char **col, const string &label,
		bool isEditable, int alignment)
{
    int i, j;
    MmTablePart *t = &tw->mmTable;

    if(t->ncols <= 0) {
	addColumn(col, label, isEditable, alignment);
	return;
    }
	
    MmTableEditModeOff(tw);

    for(j = 0; j < t->ncols; j++) {
	if(t->col_order[j] > 0) t->col_order[j]--;
    }
    for(i = 0; i < t->nrows + t->nhidden; i++) {
	Free(t->columns[0][i]);
	Free(t->backup[0][i]);
    }
    Free(t->columns[0]);
    Free(t->highlight[0]);
    Free(t->cell_fill[0]);
    Free(t->backup[0]);
    Free(t->column_labels[0]);
    Free(t->column_choice[0]);

    for(j = 0; j < t->ncols-1; j++) {
	t->col_order[j] = t->col_order[j+1];
	t->col_beg[j] = t->col_beg[j+1];
	t->col_end[j] = t->col_end[j+1];
	t->col_width[j] = t->col_width[j+1];
	t->col_nchars[j] = t->col_nchars[j+1];
	t->column_labels[j] = t->column_labels[j+1];
	t->column_choice[j] = t->column_choice[j+1];
	t->column_state[j] = t->column_state[j+1];
	t->col_time[j] = t->col_time[j+1];
	t->col_color[j] = t->col_color[j+1];
	t->col_editable[j] = t->col_editable[j+1];
	t->col_alignment[j] = t->col_alignment[j+1];
	t->columns[j] = t->columns[j+1];
	t->highlight[j] = t->highlight[j+1];
	t->cell_fill[j] = t->cell_fill[j+1];
	t->backup[j] = t->backup[j+1];
    }

    t->col_order[t->ncols-1] = t->ncols-1;
    t->column_labels[t->ncols-1] = strdup(label.c_str());
    t->column_choice[t->ncols-1] = strdup("");
    t->column_state[t->ncols-1] = False;
    t->col_time[t->ncols-1] = NOT_TIME;
    t->col_color[t->ncols-1] = (Pixel)NULL;
    t->col_editable[t->ncols-1] = isEditable;
    t->col_alignment[t->ncols-1] = alignment;
    if(!(t->columns[t->ncols-1] = (char **)MallocIt(tw,
		t->capacity*sizeof(String)))) return;
    for(i = 0; i < t->nrows; i++) {
	t->columns[t->ncols-1][i] = strdup(col[i]);
    }

    if(!(t->highlight[t->ncols-1] = (Boolean *)MallocIt(tw,
		t->capacity*sizeof(Boolean)))) return;
    for(i = 0; i < t->nrows; i++) {
	t->highlight[t->ncols-1][i] = False;
    }
    if(!(t->cell_fill[t->ncols-1] = (Pixel *)MallocIt(tw,
		t->capacity*sizeof(Pixel)))) return;
    for(i = 0; i < t->nrows; i++) {
	t->cell_fill[t->ncols-1][i] = CELL_NO_FILL_FLAG;
    }
    if(!(t->backup[t->ncols-1] = (char **)MallocIt(tw,
		t->capacity*sizeof(String)))) return;

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	t->backup[t->ncols-1][i] = (String)NULL;
    }

    if(t->color_only) {
	t->epoch_time += t->small_tdel;
    }

    getNewColumnWidths(tw, t->ncols-1, t->ncols-1);
    MmTableResetColumnLimits(tw);
    MmTableAdjustHScroll(tw);

    if(t->hbar && XtIsManaged(t->hbar)) {
	int value, visible, min, max;
	getBarValues(t->hbar, &value, &visible, &min, &max);
	if(value == max-visible-1) {
	    value++;
	    MmTableSetBarValue(t->hbar, value);
	    _TCanvasHScroll(t->canvas);
	    getBarValues(t->hbar, &value, &visible, &min, &max);
	    t->last_hbar_value = value;
	}
    }
    else {
	doLayout(tw, False);
	_TCanvasRedisplay(t->canvas);
    }
}

void Table::removeColumn(int col)
{
    int i, j;
    MmTablePart *t = &tw->mmTable;

    if(col < 0 || col >= t->ncols || t->ncols == 0) return;
    MmTableEditModeOff(tw);

    for(j = 0; j < t->ncols; j++) {
	if(t->col_order[j] > col) t->col_order[j]--;
    }
    for(i = 0; i < t->nrows + t->nhidden; i++) {
	Free(t->columns[col][i]);
	Free(t->backup[col][i]);
    }
    Free(t->columns[col]);
    Free(t->highlight[col]);
    Free(t->cell_fill[col]);
    Free(t->backup[col]);
    Free(t->column_labels[col]);
    Free(t->column_choice[col]);

    for(j = col; j < t->ncols-1; j++) {
	t->col_order[j] = t->col_order[j+1];
	t->col_beg[j] = t->col_beg[j+1];
	t->col_end[j] = t->col_end[j+1];
	t->col_width[j] = t->col_width[j+1];
	t->col_nchars[j] = t->col_nchars[j+1];
	t->column_labels[j] = t->column_labels[j+1];
	t->column_choice[j] = t->column_choice[j+1];
	t->column_state[j] = t->column_state[j+1];
	t->col_time[j] = t->col_time[j+1];
	t->col_color[j] = t->col_color[j+1];
	t->col_editable[j] = t->col_editable[j+1];
	t->col_alignment[j] = t->col_alignment[j+1];
	t->columns[j] = t->columns[j+1];
	t->highlight[j] = t->highlight[j+1];
	t->cell_fill[j] = t->cell_fill[j+1];
	t->backup[j] = t->backup[j+1];
    }
    t->ncols--;

    t->col_order = (int *)ReallocIt(tw, t->col_order, t->ncols*sizeof(int));
    t->col_beg = (int *)ReallocIt(tw, t->col_beg, t->ncols*sizeof(int));
    t->col_end = (int *)ReallocIt(tw, t->col_end, t->ncols*sizeof(int));
    t->col_width = (int *)ReallocIt(tw, t->col_width, t->ncols*sizeof(int));
    t->col_nchars = (int *)ReallocIt(tw, t->col_nchars, t->ncols*sizeof(int));
    t->column_labels = (char **)ReallocIt(tw, t->column_labels,
				t->ncols*sizeof(String));
    t->column_choice = (char **)ReallocIt(tw, t->column_choice,
				t->ncols*sizeof(String));
    t->column_state = (Boolean *)ReallocIt(tw, t->column_state,
				t->ncols*sizeof(Boolean));
    t->col_time = (enum TimeFormat *)ReallocIt(tw, t->col_time,
				t->ncols*sizeof(enum TimeFormat));
    t->col_color = (Pixel *)ReallocIt(tw, t->col_color, t->ncols*sizeof(Pixel));
    t->col_editable = (Boolean *)ReallocIt(tw, t->col_editable,
				t->ncols*sizeof(Boolean));
    t->col_alignment = (int *)ReallocIt(tw, t->col_alignment,
				t->ncols*sizeof(int));
    t->columns = (char ***)ReallocIt(tw, t->columns, t->ncols*sizeof(char **));
    t->highlight = (Boolean **)ReallocIt(tw, t->highlight,
				t->ncols*sizeof(Boolean *));
    t->cell_fill = (Pixel **)ReallocIt(tw, t->cell_fill,
				t->ncols*sizeof(Pixel *));
    t->backup = (char ***)ReallocIt(tw, t->backup, t->ncols*sizeof(String *));

    MmTableAdjustColumns(tw);
}

void
MmTableResetColumnLimits(MmTableWidget w)
{
    int i, x;
    MmTablePart *t = &w->mmTable;

    x = 0;
    for(i = 0; i < t->ncols; i++) {
	t->col_beg[i] = x;
	x += t->col_width[t->col_order[i]];
	t->col_end[i] = x;
    }
}

void Table::setColumn(int col, const char **values, int values_length)
{
    char *c;
    int i, j;
    MmTablePart *t = &tw->mmTable;

    if(col < 0 || col > t->ncols) return;

    MmTableEditModeOff(tw);

    for(i = 0; i < t->nrows + t->nhidden && i < values_length; i++) {
	c = strdup(values[i]);
	Free(t->columns[col][i]);
	t->columns[col][i] = c;
    }
    for(j = 0; j < t->ncols; j++) {
	if(t->col_order[j] == col) {
	    MmTableAdjustColumnWidths(tw, j, j);
	    break;
	}
    }
}

void Table::setColumn(int col, vector<const char *> &values)
{
    char *c;
    int i, j;
    MmTablePart *t = &tw->mmTable;

    if(col < 0 || col > t->ncols) return;

    MmTableEditModeOff(tw);

    for(i = 0; i < t->nrows + t->nhidden && i < (int)values.size(); i++) {
	c = strdup(values[i]);
	Free(t->columns[col][i]);
	t->columns[col][i] = c;
    }
    for(j = 0; j < t->ncols; j++) {
	if(t->col_order[j] == col) {
	    MmTableAdjustColumnWidths(tw, j, j);
	    break;
	}
    }
}

void Table::setColumnByLabel(const string &label, const char **values,
			int values_length)
{
    int j;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	if(!label.compare(t->column_labels[j])) {
	    setColumn(j, values, values_length);
	}
    }
}

void Table::setColumnByLabel(const string &label, vector<const char *> &values)
{
    int j;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	if(!label.compare(t->column_labels[j])) {
	    setColumn(j, values);
	}
    }
}

void
MmTableSetRowLabels(MmTableWidget w, const char **labels)
{
    int i, t_left;
    char *c;
    MmTablePart *t = &w->mmTable;

    for(i = 0; i < t->nrows + t->nhidden; i++)
    {
        if(labels[i] != NULL) {
	    c = strdup(labels[i]);
	    free(t->row_labels[i]);
	    t->row_labels[i] = c;
	}
	else if(t->row_labels[i] && t->row_labels[i][0] != '\0') {
	    t->row_labels[i][0] = '\0';
	}
    }
    t->displayRowLabels = True;
    t->newRowLabels = True;
    t_left = t->left;
    MmTableAdjustColumns(w);
    /* reset the horizontal scroll to its position before MmTableAdjustColumns*/
    t->left = t_left;
    MmTableAdjustHScroll(w);
    _TCanvasHScroll(w->mmTable.canvas);
    if(w->mmTable.header) _HCanvasRedisplay(w->mmTable.header);
}

char ** Table::getRowLabels()
{
    char **labels = (char **)NULL;
    MmTablePart *t = &tw->mmTable;
    int i, n = t->nrows + t->nhidden;

    if(!(labels = (char **)MallocIt(tw, n*sizeof(char **))))
	return (char **)NULL;
    for(i = 0; i < n; i++) labels[i] = t->row_labels[i];
    return labels;
}

void Table::displayRowLabels(bool display, bool redisplay)
{
    MmTablePart *t = &tw->mmTable;
    int t_left;

    if( !redisplay ) {
        t->displayRowLabels = display;
    }
    else if(t->displayRowLabels != display) {
        t->displayRowLabels = display;
        t->newRowLabels = True;
	t_left = t->left;
	MmTableAdjustColumns(tw);
    /* reset the horizontal scroll to its position before MmTableAdjustColumns*/
	t->left = t_left;
	MmTableAdjustHScroll(tw);
	_TCanvasHScroll(tw->mmTable.canvas);
	if(tw->mmTable.header) _HCanvasRedisplay(tw->mmTable.header);
    }
}

void Table::formatColumn(int col)
{
    MmTablePart *t = &tw->mmTable;

    if(t->nrows + t->nhidden <= 0 || col < 0 || col >= t->ncols) return;
    format(tw, col);
/*  MmTableAdjustColumnWidths(w, col, col); */
}

void Table::formatColumns(int *col, int col_length)
{
    int j;
    for(j = 0; j < col_length; j++) format(tw, col[j]);
    MmTableAdjustColumns(tw);
}

static void
format(MmTableWidget w, int col)
{
    int i, j, ndeci, period;
    int *nd = NULL;
    MmTablePart *t = &w->mmTable;

    if(!(nd = (int *)MallocIt(w, (t->nrows + t->nhidden)*sizeof(int)))) return;

    ndeci = -1;
    for(i = 0; i < t->nrows + t->nhidden; i++) {
	String s = t->columns[col][i];
	for(period = (int)strlen(s)-1; period >= 0; period--)
			if(s[period] == '.') break;
	nd[i] = (period != -1) ? (int)strlen(s) - 1 - period : -1;
	if(nd[i] > ndeci) ndeci = nd[i];
    }

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	String s = t->columns[col][i];
	int len = strlen(s);
	if(len > 0 && nd[i] != ndeci) {
	    int n = (nd[i] < 0) ? ndeci + 1 : ndeci;
	    s = (String)realloc(s, n);
	    if(nd[i] < 0) {
		strcat(s, ".");
		nd[i]++;
	    }
	    for(j = nd[i]; j < ndeci; j++) strcat(s, "0");
	    t->columns[col][i] = s;
	}
    }
    Free(nd);
}

void Table::insert(int row_index, const char **row)
{
    int i, j, n;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    n = t->nrows + t->nhidden;
    if(n == t->capacity) { /* increase capacity */
	increaseCapacity(tw);
    }
    for(j = 0; j < t->ncols; j++) {
	t->columns[j][n] = (row[j] != (String)NULL) ? strdup(row[j]) :
			strdup("");
	t->highlight[j][n] = False;
	t->cell_fill[j][n] = CELL_NO_FILL_FLAG;
	t->backup[j][n] = (String)NULL;
    }
    t->row_state[n] = False;
    t->row_labels[n] = strdup("");
    t->row_editable[n] = t->editable;

    for(i = t->nrows + t->nhidden; i > row_index; i--) {
	t->row_order[i] = t->row_order[i-1];
    }
    t->row_order[row_index] = n;
    t->nrows++;
    updateDisplayOrder(t);
    XtCallCallbacks((Widget)tw, XtNrowChangeCallback, (XtPointer)t->nrows);
}

void Table::displayVerticalScrollbar(bool set)
{
    tw->mmTable.displayVerticalScrollbar = set;
}

void Table::displayHorizontalScrollbar(bool set)
{
    tw->mmTable.displayHorizontalScrollbar = set;
}

void Table::setEditable(bool set)
{
    tw->mmTable.editable = set;
}

void Table::setSelectable(bool set)
{
    tw->mmTable.selectable = set;
}

void Table::setRowSelectable(bool set)
{
    tw->mmTable.rowSelectable = set;
}

void Table::setColumnSelectable(bool set)
{
    tw->mmTable.columnSelectable = set;
}

static void
increaseCapacity(MmTableWidget w)
{
    int j, n;
    MmTablePart *t = &w->mmTable;

    n = t->capacity;
    t->capacity += t->incremental_capacity;

    for(j = 0; j < t->ncols; j++) {
	t->columns[j] = (char **)ReallocIt(w, t->columns[j],
				t->capacity*sizeof(String));
	t->highlight[j] = (Boolean *)ReallocIt(w, t->highlight[j],
				t->capacity*sizeof(Boolean));
	t->cell_fill[j] = (Pixel *)ReallocIt(w, t->cell_fill[j],
				t->capacity*sizeof(Pixel));
	t->backup[j] = (char **)ReallocIt(w, t->backup[j],
				t->capacity*sizeof(String));
    }
    t->row_order = (int *)ReallocIt(w, t->row_order, t->capacity*sizeof(int));
    t->display_order = (int *)ReallocIt(w, t->display_order,
				t->capacity*sizeof(int));
    t->row_state = (Boolean *)ReallocIt(w, t->row_state,
				t->capacity*sizeof(Boolean));
    t->row_labels = (char **)ReallocIt(w, t->row_labels,
				t->capacity*sizeof(String));
    t->row_editable = (Boolean *)ReallocIt(w, t->row_editable,
				t->capacity*sizeof(Boolean));
    for(j = 0; j < t->incremental_capacity; j++) t->row_labels[n+j] = NULL;
	
    if(t->incremental_capacity < 400 && ++(t->increases) == 3) {
	t->increases = 0;
	t->incremental_capacity *= 2;
    }
}

void Table::setTableBackground(Pixel bg)
{
    tw->mmTable.canvas->core.background_pixel = bg;
}

/*
public synchronized void addTableListener(TableListener tableListener) {
    tableListeners.addElement(tableListener);
}
public synchronized void removeTableListener(TableListener tableListener) {
    tableListeners.removeElement(tableListener);
}
**/

void
MmTableAdjustColumns(MmTableWidget w)
{
    if(w->mmTable.ncols > 0) {
	MmTableAdjustColumnWidths(w, 0, w->mmTable.ncols-1);
    }
    else {
	MmTablePart *t = &w->mmTable;
	doLayout(w, False);
	if(t->header) _HCanvasRedisplay(t->header);
	_TCanvasRedisplay(t->canvas);
    }
}

void Table::highlightField(int row, int col, bool highlight)
{
    _TCanvasHighlightField(tw->mmTable.canvas, row, col, highlight);
}

void Table::fillCell(int row, int col, Pixel pixel, bool redisplay)
{
    MmTablePart *t = &tw->mmTable;

    if(row >= 0 && row < t->nrows + t->nhidden &&
	col >= 0 && col < t->ncols)
    {
	t->cell_fill[col][row] = pixel;
    }
    if(redisplay) {
/*
	_TCanvasDrawRows(w->mmTable.canvas, row, row, t->left, t->right);
	XCopyArea(XtDisplay(w), tc->image, XtWindow(w), tc->gc, 0, 0,
	   w->core.width, w->core.height, 0, 0);
*/
	if(t->header) _HCanvasRedisplay(t->header);
	_TCanvasRedisplay(t->canvas);
    }
}

Pixel Table::getCellPixel(int row, int col)
{
    MmTablePart *t = &tw->mmTable;

    if(row >= 0 && row < t->nrows + t->nhidden &&
	col >= 0 && col < t->ncols)
    {
	return t->cell_fill[col][row];
    }
    return WhitePixelOfScreen(XtScreen((Widget)tw));
}

bool Table::isHighlighted(int row, int col)
{
    return tw->mmTable.highlight[col][row];
}

void Table::highlightOff(void)
{
    int i, j;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	for(i = 0; i < t->nrows + t->nhidden; i++) {
	    t->highlight[j][i] = False;
	}
    }
    _TCanvasRedisplay(t->canvas);
}

void
MmTableAdjustColumnWidths(MmTableWidget w, int j1, int j2)
{
    MmTablePart *t = &w->mmTable;

    if(getNewColumnWidths(w, j1, j2) || t->need_layout)
    {
	MmTableResetColumnLimits(w);
	t->left = 0;
	MmTableAdjustHScroll(w);
	doLayout(w, False);
    }
    if(XtIsRealized((Widget)w)) {
	if(t->header) _HCanvasRedisplay(t->header);
	_TCanvasRedisplay(t->canvas);
    }
}

static Boolean
getNewColumnWidths(MmTableWidget w, int j1, int j2)
{
    int i, j;
    char *c;
    Boolean need_column_adjust = False;
    MmTablePart *t = &w->mmTable;

    if(!XtIsRealized((Widget)w)) {
	t->needAdjustColumns = 1;
/*	return False; */
    }
    else t->needAdjustColumns = 0;

    for(j = j1; j <= j2; j++)
    {
	int max_w = 0;
	int width = stringWidth(w, t->column_labels[t->col_order[j]])
			+ t->cellMargin;
	if(max_w < width) max_w = width;

	if(t->column_choice[t->col_order[j]][0] != '\0')
	{
	    width = getChoiceMaxWidth(w, t->column_choice[t->col_order[j]])
				+ t->cellMargin;
	    if(max_w < width) max_w = width;
	}
	t->col_nchars[j] = 0;
	for(i = 0; i < t->nrows + t->nhidden; i++)
	{
	    int len;
	    if((c = MmTableGetCellChoice(w, i, t->col_order[j])) != NULL) {
		width = getChoiceMaxWidth(w, c) + t->cellMargin;
	    }
	    else {
		width = stringWidth(w, t->columns[t->col_order[j]][i])
			+ t->cellMargin;
	    }
	    if(max_w < width) max_w = width;

	    len = (int)strlen(t->columns[t->col_order[j]][i]);
	    if(t->col_nchars[j] < len) t->col_nchars[j] = len;
	}
	if(t->col_width[t->col_order[j]] != max_w) {
	    t->col_width[t->col_order[j]] = max_w;
	    if(t->col_width[t->col_order[j]] < t->min_col_width) {
		t->col_width[t->col_order[j]] = t->min_col_width;
	    }
	    need_column_adjust = True;
	}
    }
    if(t->newRowLabels) {
	t->newRowLabels = False;
	need_column_adjust = True;
    }
    return need_column_adjust;
}

static int
getChoiceMaxWidth(MmTableWidget w, char *choice)
{
    char *s, *tok, *buf;
    int max_width, width;

    buf = strdup(choice);

    max_width = 0;
    tok = buf;
    while((s = strtok(tok, ":")) != NULL) {
        tok = NULL;
	width = stringWidth(w, s);
	if(width > max_width) max_width = width;
    }
    Free(buf);
    return max_width;
}

/*
void quickAdjustColumns(MmTableWidget w)
{

    if(fontMetrics == NULL) {
	Font font = getFont();
	if(font == null || (fontMetrics = getFontMetrics(font)) == null) {
	    needAdjustColumns = 2;
	    return;
	}
    }
    MmTableEditModeOff(w);
    needAdjustColumns = 0;
    StringBuffer buf = nou StringBuffer();
    for(j = 0; j < t->ncols; j++) {
	int max_w, w;

	max_w = column_labels[j].length();
	for(i = 0; i < nrows+nhidden; i++) {
	    w = columns[j][i].length();
	    if(max_w < w) max_w = w;
	}
	buf.setLength(0);
	for(i = 0; i < max_w; i++) buf.append("A");
			
	max_w = fontMetrics.stringWidth(buf.toString()) + 5;
	w = fontMetrics.stringWidth(column_labels[j]) + 5;
	if(max_w < w) max_w = w;

	col_width[col_order[j]] = max_w;
    }
    MmTableResetColumnLimits(w);
    doLayout();
    if(t->header) _HCanvasRedisplay(t->header);
    _TCanvasRedisplay(t->canvas);
}
**/

const char * Table::tableName(void)
{
    return tw->mmTable.name;
}

void Table::sortByColumn(int col)
{
    vector<int> v;
    MmTableEditModeOff(tw);
    v.push_back(col);
    sortByColumns(v);
}

void Table::sortByColumns(vector<int> &col)
{
    MmTablePart *t = &tw->mmTable;
    MmTableEditModeOff(tw);
    sort(tw, col, 0, 0, tw->mmTable.nrows-1);
    updateDisplayOrder(t);
    _TCanvasRedisplay(tw->mmTable.canvas);
}

void Table::sortByColumnLabels(const string &col_labels)
{
    MmTablePart *t = &tw->mmTable;
    char *s = NULL, *tok, *c;
    int i, j, n;
    vector<int> col;

    if(col_labels.empty()) return;

    n = 0;
    s = strdup(col_labels.c_str());
    tok = s;
    while((c = strtok(tok, ",")) != NULL)
    {
	tok = NULL;
	stringTrim(c);
	for(i = 0; i < t->ncols; i++) {
	    if(!strcmp(c, t->column_labels[i])) {
		for(j = 0; j < n && col[j] != i; j++);
		if(j == n) { col.push_back(i); n++; }
	    }
	}
    }
    free(s);

    sortByColumns(col);
}

void Table::sortSelected(vector<int> &col)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    for(i = 0; i < t->nrows; i++) {
	if(t->row_state[t->row_order[i]]) {
	    int j;
	    for(j = i+1; j < t->nrows; j++) {
		if(!t->row_state[t->row_order[j]]) break;
	    }
	    sort(tw, col, 0, i, j-1);
	}
    }
    updateDisplayOrder(t);
    _TCanvasRedisplay(t->canvas);
}

void Table::reverseOrder()
{
    int i, n;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    n = t->nrows/2;
    for(i = 0; i < n; i++) {
	int ro = t->row_order[i];
	t->row_order[i] = t->row_order[t->nrows-1-i];
	t->row_order[t->nrows-1-i] = ro;
    }
    updateDisplayOrder(t);
    _TCanvasRedisplay(t->canvas);
}

static void
sort(MmTableWidget w, vector<int> &col, int i0, int row1, int row2)
{
    int i, j, len;
    Boolean do_values;
    double *values = (double *)NULL;
    String *column, endptr;
    String *s = (String *)NULL;
    MmTablePart *t = &w->mmTable;

    if(row2-row1 <= 0 || (int)col.size() <= 0) return;

    if(!(values = (double *)MallocIt(w, t->nrows*sizeof(double)))) return;

    column = t->columns[col[i0]];

    if(t->col_time[col[i0]] != NOT_TIME) {
	enum TimeFormat time_code = t->col_time[col[i0]];
	for(i = row1; i <= row2; i++) {
	    values[i] = timeStringToEpoch(column[t->row_order[i]], time_code);
	}
	do_values = True;
    }
    else {
	do_values = True;
	for(i = row1; i <= row2; i++) {
	    values[i] = strtod(column[t->row_order[i]], &endptr);
	    len = (int)strlen(column[t->row_order[i]]);
	    if(endptr - column[t->row_order[i]] != len) {
		do_values = False;
		break;
	    }
	}
    }
    if(do_values) {
	sortByValue(row1, row2, values, t->row_order);
    }
    else {
        if(!(s = (char **)MallocIt(w, t->nrows*sizeof(String)))) return;
	for(i = row1; i <= row2; i++) s[i] = column[t->row_order[i]];
	sortByString(row1, row2, s, t->row_order);
    }

    /* do a sub-sort */
    if(i0 < (int)col.size()-1) {
	if(do_values) {
	    for(i = row1+1; i <= row2; i++) {
		if(values[i] == values[i-1]) {
		    for(j = i+1; j <= row2; j++) {
			if(values[j] != values[i]) break;
		    }
		    sort(w, col, i0+1, i-1, j-1);
		    i = j;
		}
	    }
	}
	else {
	    for(i = row1+1; i <= row2; i++) {
		if(!strcmp(s[i], s[i-1])) {
		    for(j = i+1; j <= row2; j++) {
			if(strcmp(s[j], s[i])) break;
		    }
		    sort(w, col, i0+1, i-1, j-1);
		    i = j;
		}
	    }
	}
    }
    Free(s);
    Free(values);
}

static void
sortByString(int lo0, int hi0, String *s, int *row_order)
{
    int lo = lo0;
    int hi = hi0;
    String mid = s[(lo0+hi0)/2];

    if(lo0 >= hi0) return;

    while(lo <= hi) {
	while(lo < hi0 && strcmp(s[lo], mid) < 0) lo++;
	while(hi > lo0 && strcmp(s[hi], mid) > 0) hi--;

	if(lo <= hi){
	    if(lo != hi) {
		String g = s[lo];
		int i = row_order[lo];
		s[lo] = s[hi];
		s[hi] = g;
		row_order[lo] = row_order[hi];
		row_order[hi] = i;
	    }
	    lo++;
	    hi--;
	}
    }
    if(lo0 < hi) sortByString(lo0, hi, s, row_order);
    if(lo < hi0) sortByString(lo, hi0, s, row_order);
}

static void
sortByValue(int lo0, int hi0, double *values, int *row_order)
{
    int lo = lo0;
    int hi = hi0;
    double mid = values[(lo0+hi0)/2];

    if(lo0 >= hi0) return;


    while(lo <= hi) {
	while(lo < hi0 && values[lo] < mid) lo++;
	while(hi > lo0 && values[hi] > mid) hi--;

	if(lo <= hi){
	    if(lo != hi) {
		double d = values[lo];
		int i = row_order[lo];
		values[lo] = values[hi];
		values[hi] = d;
		row_order[lo] = row_order[hi];
		row_order[hi] = i;
	    }
	    lo++;
	    hi--;
	}
    }
    if(lo0 < hi) sortByValue(lo0, hi, values, row_order);
    if(lo < hi0) sortByValue(lo, hi0, values, row_order);
}

void Table::promoteSelectedRows()
{
    int i, n, *ro = (int *)NULL;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    if(!(ro = (int *)MallocIt(tw, t->nrows*sizeof(int)))) return;
    memcpy(ro, t->row_order, t->nrows*sizeof(int));

    n = 0;
    for(i = 0; i < t->nrows; i++) {
	if(t->row_state[ro[i]]) {
	    t->row_order[n++] = ro[i];
	}
    }
    for(i = 0; i < t->nrows; i++) {
	if(!t->row_state[ro[i]]) {
	    t->row_order[n++] = ro[i];
	}
    }
    Free(ro);
    updateDisplayOrder(t);
    _TCanvasRedisplay(t->canvas);
}

void Table::promoteSelectedColumns()
{
    int j, n, *co = (int *)NULL;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    if(!(co = (int *)MallocIt(tw, t->ncols*sizeof(int)))) return;
    memcpy(co, t->col_order, t->ncols*sizeof(int));

    n = 0;
    for(j = 0; j < t->ncols; j++) {
	if(t->column_state[co[j]]) {
	    t->col_order[n++] = co[j];
	}
    }
    for(j = 0; j < t->ncols; j++) {
	if(!t->column_state[co[j]]) {
	    t->col_order[n++] = co[j];
	}
    }
    Free(co);
    MmTableResetColumnLimits(tw);
    _TCanvasRedisplay(t->canvas);
    if(t->header) _HCanvasRedisplay(t->header);
}

void Table::setColumnColors(Pixel *colors)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    for(i = 0; i < t->ncols; i++) t->col_color[i] = colors[i];
    if(t->header) _HCanvasRedisplay(t->header);
}

void Table::setColumnEditable(bool *editable)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    for(i = 0; i < t->ncols; i++) t->col_editable[i] = editable[i];
}

void Table::setColumnTime(vector<enum TimeFormat> &time_code)
{
    MmTablePart *t = &tw->mmTable;

    for(int i = 0; i < t->ncols && i < (int)time_code.size(); i++) {
	t->col_time[i] = time_code[i];
    }
}

enum TimeFormat Table::getColumnTime(int col)
{
    MmTablePart *t = &tw->mmTable;
    if(col >= 0 && col < t->ncols) return t->col_time[col];
    return NOT_TIME;
}

void Table::setRowEditable(bool *editable)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    for(i = 0; i < t->nrows; i++) t->row_editable[i] = editable[i];
}

int Table::getRowEditable(vector<bool> &editable)
{
    MmTablePart *t = &tw->mmTable;

    editable.clear();
    for(int i = 0; i < t->nrows; i++) {
	editable.push_back((bool)t->row_editable[i]);
    }
    return (int)editable.size();
}

void Table::setAlignment(int ncols, int *alignment)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    for(i = 0; i < ncols && i < t->ncols; i++) {
	t->col_alignment[i] = alignment[i];
    }
}

int Table::getAlignment(vector<int> &alignment)
{
    MmTablePart *t = &tw->mmTable;

    alignment.clear();
    for(int i = 0; i < t->ncols; i++) alignment.push_back(t->col_alignment[i]);
    return (int)alignment.size();
}

void Table::setColumnOrder(vector<int> &order)
{
    int j, n;
    Boolean *b = (Boolean *)NULL;
    MmTablePart *t = &tw->mmTable;

    if(!(b = (Boolean *)MallocIt(tw, t->ncols*sizeof(Boolean)))) return;

    for(j = 0; j < t->ncols; j++) b[j] = False;

    n = 0;
    for(j = 0; j < t->ncols && j < (int)order.size(); j++) {
	if(!b[order[j]]) {
	    t->col_order[n++] = order[j];
	    b[order[j]] = True;
	}
    }
    for(j = 0; j < t->ncols; j++) {
	if(!b[j]) {
	    t->col_order[n++] = j;
	    if(n == t->ncols) break;
	}
    }
    Free(b);
    MmTableResetColumnLimits(tw);
    MmTableAdjustHScroll(tw);
    _TCanvasRedisplay(t->canvas);
    if(t->header) _HCanvasRedisplay(t->header);
}

void Table::setRowOrder(vector<int> &order)
{
    int j, n;
    Boolean *b = (Boolean *)NULL;
    MmTablePart *t = &tw->mmTable;

    if(!(b = (Boolean *)MallocIt(tw, t->nrows*sizeof(Boolean)))) return;

    for(j = 0; j < t->nrows; j++) b[j] = False;

    n = 0;
    for(j = 0; j < t->nrows && j < (int)order.size(); j++) {
	if(!b[order[j]]) {
	    t->row_order[n++] = order[j];
	    b[order[j]] = True;
	}
    }
    for(j = 0; j < t->nrows; j++) {
	if(!b[j]) {
	    t->row_order[n++] = j;
	    if(n == t->nrows) break;
	}
    }
    Free(b);

    updateDisplayOrder(t);
    _TCanvasRedisplay(t->canvas);
}

void Table::sortUnique(vector<int> &cols)
{
    int i, n, col, len, nhide, *hide = (int *)NULL, *hidden_order = NULL;
    String *column, endptr;
    double *values = (double *)NULL;
    Boolean do_values;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    if((int)cols.size() == 0) return;

    sort(tw, cols, 0, 0, t->nrows-1);

    col = cols[0];
    if(col < 0 || col > t->ncols) return;

    if(!(values = (double *)MallocIt(tw, t->nrows*sizeof(double)))) return;

    Free(t->sort_unique.cols);
    if(!(t->sort_unique.cols = (int *)MallocIt(tw, cols.size()*sizeof(int))))
	return;
    for(i = 0; i < (int)cols.size(); i++) t->sort_unique.cols[i] = cols[i];
    t->sort_unique.cols_length = (int)cols.size();

    column = t->columns[col];
    do_values = True;
    for(i = 0; i < t->nrows; i++) {
	values[i] = strtod(column[t->row_order[i]], &endptr);
	len = (int)strlen(column[t->row_order[i]]);
	if(endptr - column[t->row_order[i]] != len) {
	    do_values = False;
	    break;
	}
    }

    if(!(hide = (int *)MallocIt(tw, t->nrows*sizeof(int)))) return;
    if(!(hidden_order = (int *)MallocIt(tw,
		(t->nrows+t->nhidden)*sizeof(int)))) return;
    nhide = 0;
    if(do_values) {
	for(i = 1; i < t->nrows; i++) {
	    if(values[i] == values[i-1]) {
		hide[nhide++] = i;
	    }
	}
    }
    else {
	String *s = (String *)NULL;

	if(!(s = (char **)MallocIt(tw, t->nrows*sizeof(String)))) return;

	for(i = 0; i < t->nrows; i++) {
	    s[i] = column[t->row_order[i]];
	}
	for(i = 1; i < t->nrows; i++) {
	    if(!strcmp(s[i], s[i-1])) hide[nhide++] = i;
	}
        Free(s);
    }
    if(nhide > 0) {
	/* remove rows to hidden[] */
	for(i = 0; i < t->nhidden; i++) {	/* previously hidden */
	    hidden_order[i] = t->row_order[t->nrows+i];
	}
	for(i = 0; i < nhide; i++) {
	    hidden_order[t->nhidden++] = t->row_order[hide[i]];
	    /* deselect when hiding */
	    t->row_state[t->row_order[hide[i]]] = False;
	    t->row_order[hide[i]] = -1;
	}
	n = 0;
	for(i = 0; i < t->nrows; i++) {
	    if(t->row_order[i] >= 0) t->row_order[n++] = t->row_order[i];
	}
	t->nrows -= nhide;
	for(i = 0; i < t->nhidden; i++) {
	    t->row_order[t->nrows+i] = hidden_order[i];
	}
    }
    updateDisplayOrder(t);

    Free(values);
    Free(hide);
    Free(hidden_order);

    if(nhide > 0) {
	doLayout(tw, False);
	_TCanvasRedisplay(t->canvas);
    }
}

static void
updateDisplayOrder(MmTablePart *t)
{
    int i;
    for(i = 0; i < t->nrows + t->nhidden; i++) {
	t->display_order[t->row_order[i]] = i;
    }
}

void Table::showAll()
{
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);
    if(t->nhidden > 0) {
	t->nrows += t->nhidden;
	t->nhidden = 0;
	doLayout(tw, False);
	_TCanvasRedisplay(t->canvas);
	Free(t->sort_unique.cols);
	t->sort_unique.cols_length = 0;
    }
}

void Table::expand(int col)
{
    int i, j, k, len, n, nshow, *show = (int *)NULL, *hidden_order =(int *)NULL;
    String *column, endptr;
    Boolean do_values, show_one;
    double *values = (double *)NULL;
    double *hidden_values = (double *)NULL;
    MmTablePart *t = &tw->mmTable;

    if(col < 0 || col > t->ncols || t->nhidden == 0) return;

    MmTableEditModeOff(tw);

    column = t->columns[col];

    if(!(values = (double *)MallocIt(tw, t->nrows*sizeof(double)))) return;
    if(!(hidden_values = (double *)MallocIt(tw, t->nhidden*sizeof(double))))
		return;
    if(!(hidden_order = (int *)MallocIt(tw, t->nhidden*sizeof(int)))) return;

    memcpy(hidden_order, t->row_order+t->nrows, t->nhidden*sizeof(int));

    do_values = True;
    for(i = 0; i < t->nrows; i++) {
	values[i] = strtod(column[t->row_order[i]], &endptr);
	len = (int)strlen(column[t->row_order[i]]);
	if(endptr - column[t->row_order[i]] != len) {
	    do_values = False;
	    break;
	}
    }
    if(do_values) {
	for(i = 0; i < t->nhidden; i++) {
	    hidden_values[i] = strtod(column[hidden_order[i]], &endptr);
	    len = (int)strlen(column[hidden_order[i]]);
	    if(endptr - column[hidden_order[i]] != len) {
		do_values = False;
		break;
	    }
	}
    }

    if(!(show = (int *)MallocIt(tw, t->nhidden*sizeof(int)))) return;
    show_one = False;

    k = 0;
    for(i = 0; i < t->nrows; i++, k++) {
	if(t->row_state[t->row_order[i]]) {
	    nshow = 0;
	    if(do_values)
	    {
		for(j = 0; j < t->nhidden; j++) {
		    if(values[k] == hidden_values[j]) {
			show[nshow++] = j;
		    }
		}
	    }
	    else {
		String s = column[t->row_order[k]];
		for(j = 0; j < t->nhidden; j++) {
		    if(!strcmp(s, column[hidden_order[j]])) {
			show[nshow++] = j;
		    }
		}
	    }
	    if(nshow > 0) {
		show_one = True;
		/* move rows up to make space for nshow rows after i */
		for(j = t->nrows-1; j > i; j--) {
		    t->row_order[j+nshow] = t->row_order[j];
		}
		/* insert hidden rows */
		for(j = 0; j < nshow; j++) {
		    t->row_order[i+1+j] = hidden_order[show[j]];
		    hidden_order[show[j]] = -1;
		    t->row_state[t->row_order[i+1+j]] = True;
		}
		n = 0;
		for(j = 0; j < t->nhidden; j++) {
		    if(hidden_order[j] >= 0) {
			hidden_order[n++] = hidden_order[j];
		    }
		}
		t->nhidden = n;
		t->nrows += nshow;
		i += nshow;
	    }
	}
    }
    for(i = 0; i < t->nhidden; i++) {
	t->row_order[t->nrows+i] = hidden_order[i];
    }
    Free(values);
    Free(hidden_order);
    Free(hidden_values);

    updateDisplayOrder(t);

    if(show_one) {
	doLayout(tw, False);
	_TCanvasRedisplay(t->canvas);
    }
}

static void
verticalScroll(
    Widget		scroll,
    XtPointer		client_data,
    XmScrollBarCallbackStruct *call_data)
{
    int value, visible, min, max;
    MmTableWidget w = (MmTableWidget) scroll->core.parent;

    if(call_data->value != w->mmTable.last_vbar_value) {
	_TCanvasVScroll(w->mmTable.canvas);
    }
    getBarValues(scroll, &value, &visible, &min, &max);
    w->mmTable.last_vbar_value = value;

    _MmTableResetInfo(w);
}

static void
horizontalScroll(
    Widget		scroll,
    XtPointer		client_data,
    XmScrollBarCallbackStruct *call_data)
{
    int value, visible, min, max;
    MmTableWidget w = (MmTableWidget) scroll->core.parent;

    if(call_data->value != w->mmTable.last_hbar_value) {
	_TCanvasHScroll(w->mmTable.canvas);
	if(w->mmTable.header) _HCanvasRedisplay(w->mmTable.header);
    }
    getBarValues(scroll, &value, &visible, &min, &max);
    w->mmTable.last_hbar_value = value;

    _MmTableResetInfo(w);
}


int Table::numRows()
{
    return tw->mmTable.nrows + tw->mmTable.nhidden;
}
int Table::numColumns()
{
    return tw->mmTable.ncols;
}

const char ** Table::getRow(int i)
{
    int j;
    const char **row = (const char **)NULL;
    MmTablePart *t = &tw->mmTable;

/*    MmTableEditModeOff(w); */

    if(i < 0 || i >= t->nrows + t->nhidden) return (const char **)NULL;

    if(!(row = (const char **)MallocIt(tw, t->ncols*sizeof(const char **))))
	return (const char **)NULL;
    for(j = 0; j < t->ncols; j++) row[j] = t->columns[j][i];
    return row;
}

int Table::getRow(int i, vector<const char *> &row)
{
    MmTablePart *t = &tw->mmTable;

    row.clear();
    if(i < 0 || i >= t->nrows + t->nhidden) return 0;

    for(int j = 0; j < t->ncols; j++) {
	row.push_back((const char *)t->columns[j][i]);
    }
    return (int)row.size();
}

void Table::setRow(int i, const char **row)
{
    int j;
    char *c;
    MmTablePart *t = &tw->mmTable;

    if(i < 0 || i >= t->nrows + t->nhidden) return;

    for(j = 0; j < t->ncols; j++) {
	c = strdup(row[j]);/* do this first in case row points to columns */
	Free(t->columns[j][i]);
	t->columns[j][i] = c;
    }
}

void Table::setRow(int i, vector<const char *> &row)
{
    int j;
    char *c;
    MmTablePart *t = &tw->mmTable;

    if(i < 0 || i >= t->nrows + t->nhidden) return;

    for(j = 0; j < t->ncols && j < (int)row.size(); j++) {
	c = strdup(row[j]);/* do this first in case row points to columns */
	Free(t->columns[j][i]);
	t->columns[j][i] = c;
    }
}

void Table::setRow(int i, vector<string> &row)
{
    int j;
    char *c;
    MmTablePart *t = &tw->mmTable;

    if(i < 0 || i >= t->nrows + t->nhidden) return;

    for(j = 0; j < t->ncols && j < (int)row.size(); j++) {
	// do this first in case row points to columns 
	c = strdup(row[j].c_str());
	Free(t->columns[j][i]);
	t->columns[j][i] = c;
    }
}

void Table::setRow(int i, vector<char *> &row)
{
    int j;
    char *c;
    MmTablePart *t = &tw->mmTable;

    if(i < 0 || i >= t->nrows + t->nhidden) return;

    for(j = 0; j < t->ncols && j < (int)row.size(); j++) {
	c = strdup(row[j]);/* do this first in case row points to columns */
	Free(t->columns[j][i]);
	t->columns[j][i] = c;
    }
}

void Table::fillRow(int i, Pixel *row, bool redisplay)
{
    int j;
    MmTablePart *t = &tw->mmTable;
    TCanvasPart *tc = &t->canvas->tCanvas;

    if(i < 0 || i >= t->nrows + t->nhidden) return;

    for(j = 0; j < t->ncols; j++) {
	t->cell_fill[j][i] = row[j];
    }
    if(redisplay) {
	_TCanvasDrawRows(tw->mmTable.canvas, i, i, t->left, t->right);
	XCopyArea(XtDisplay(tw), tc->image, XtWindow(tw), tc->gc, 0, 0,
	   tw->core.width, tw->core.height, 0, 0);
    }
}

const char **Table::getColumnByLabel(const string &label)
{
    int j;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	if(!label.compare(t->column_labels[j])) {
	    return getColumn(j);
	}
    }
    return (const char **)NULL;
}

int Table::getColumnByLabel(const string &label, vector<const char *> &col)
{
    int j;
    MmTablePart *t = &tw->mmTable;

    col.clear();
    for(j = 0; j < t->ncols; j++) {
	if(!label.compare(t->column_labels[j])) {
	    return getColumn(j, col);
	}
    }
    return 0;
}

const char **Table::getColumn(int j)
{
    int i, n;
    const char **col = (const char **)NULL;
    MmTablePart *t = &tw->mmTable;

    if(j < 0 || j > t->ncols) return (const char **)NULL;

    n = t->nrows + t->nhidden;
    if(!(col = (const char **)MallocIt(tw, n*sizeof(const char *))))
	return (const char **)NULL;
    for(i = 0; i < n; i++) col[i] = t->columns[j][i];
    return col;
}

int Table::getColumn(int j, vector<const char *> &col)
{
    int n;
    MmTablePart *t = &tw->mmTable;

    col.clear();
    if(j < 0 || j > t->ncols) return 0;

    n = t->nrows + t->nhidden;
    for(int i = 0; i < n; i++) col.push_back((const char *)t->columns[j][i]);
    return (int)col.size();
}

void Table::moveToTop(int row)
{
    MmTablePart *t = &tw->mmTable;

    if(row >= 0 && row < t->nrows)
    {
	int i;
	for(i = 0; i < t->nrows && row != t->row_order[i]; i++);
	if(i < t->nrows) {
	    if(tw->mmTable.vbar != NULL) {
		MmTableSetBarValue(tw->mmTable.vbar, i);
		_TCanvasVScroll(tw->mmTable.canvas);
	    }
	    else {
		tw->mmTable.top_row = i;
	    }
	}
    }
}

int Table::getRowOrder(vector<int> &row_order)
{
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);
    row_order.clear();
    for(int i = 0; i < t->nrows; i++) row_order.push_back(t->row_order[i]);
    return (int)row_order.size();
}

int Table::getColumnNChars(vector<int> &nchars)
{
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);
    nchars.clear();
    for(int i = 0; i < t->ncols; i++) nchars.push_back(t->col_nchars[i]);
    return (int)nchars.size();
}

int Table::getColumnOrder(vector<int> &co)
{
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);
    co.clear();
    for(int i = 0; i < t->ncols; i++) co.push_back(t->col_order[i]);
    return (int)co.size();
}

void Table::resetColumnOrder()
{
    MmTablePart *t = &tw->mmTable;
    int i, *col = NULL;
    enum TimeFormat *tf = NULL;
    Boolean **b = NULL, *bo = NULL;
    String **s = NULL, *lab = NULL;
    Pixel *p = NULL, **cellp;

    if(!(col = (int *)MallocIt(tw, t->ncols*sizeof(int)))) return;

    for(i = 0; i < t->ncols; i++) col[i] = t->col_width[i];
    for(i = 0; i < t->ncols; i++) t->col_width[i] = col[t->col_order[i]];

    for(i = 0; i < t->ncols; i++) col[i] = t->col_nchars[i];
    for(i = 0; i < t->ncols; i++) t->col_nchars[i] = col[t->col_order[i]];

    for(i = 0; i < t->ncols; i++) col[i] = t->col_alignment[i];
    for(i = 0; i < t->ncols; i++) t->col_alignment[i] = col[t->col_order[i]];

    for(i = 0; i < t->ncols; i++) col[i] = t->col_beg[i];
    for(i = 0; i < t->ncols; i++) t->col_beg[i] = col[t->col_order[i]];

    for(i = 0; i < t->ncols; i++) col[i] = t->col_end[i];
    for(i = 0; i < t->ncols; i++) t->col_end[i] = col[t->col_order[i]];
    Free(col);

    if(!(tf = (enum TimeFormat *)MallocIt(tw,
		t->ncols*sizeof(enum TimeFormat)))) return;

    for(i = 0; i < t->ncols; i++) tf[i] = t->col_time[i];
    for(i = 0; i < t->ncols; i++) t->col_time[i] = tf[t->col_order[i]];
    Free(tf);

    if(!(bo = (Boolean *)MallocIt(tw, t->ncols*sizeof(Boolean)))) return;

    for(i = 0; i < t->ncols; i++) bo[i] = t->column_state[i];
    for(i = 0; i < t->ncols; i++) t->column_state[i] = bo[t->col_order[i]];

    for(i = 0; i < t->ncols; i++) bo[i] = t->col_editable[i];
    for(i = 0; i < t->ncols; i++) t->col_editable[i] = bo[t->col_order[i]];
    Free(bo);

    if(!(b = (Boolean **)MallocIt(tw, t->ncols*sizeof(Boolean *)))) return;

    for(i = 0; i < t->ncols; i++) b[i] = t->highlight[i];
    for(i = 0; i < t->ncols; i++) t->highlight[i] = b[t->col_order[i]];
    Free(b);

    if(!(cellp = (Pixel **)MallocIt(tw, t->ncols*sizeof(Pixel *)))) return;

    for(i = 0; i < t->ncols; i++) cellp[i] = t->cell_fill[i];
    for(i = 0; i < t->ncols; i++) t->cell_fill[i] = cellp[t->col_order[i]];
    Free(cellp);

    if(!(s = (char ***)MallocIt(tw, t->ncols*sizeof(char **)))) return;

    for(i = 0; i < t->ncols; i++) s[i] = t->columns[i];
    for(i = 0; i < t->ncols; i++) t->columns[i] = s[t->col_order[i]];

    for(i = 0; i < t->ncols; i++) s[i] = t->backup[i];
    for(i = 0; i < t->ncols; i++) t->backup[i] = s[t->col_order[i]];
    Free(s);

    if(!(p = (Pixel *)MallocIt(tw, t->ncols*sizeof(Pixel)))) return;

    for(i = 0; i < t->ncols; i++) p[i] = t->col_color[i];
    for(i = 0; i < t->ncols; i++) t->col_color[i] = p[t->col_order[i]];
    Free(p);

    if(!(lab = (char **)MallocIt(tw, t->ncols*sizeof(char *)))) return;

    for(i = 0; i < t->ncols; i++) lab[i] = t->column_labels[i];
    for(i = 0; i < t->ncols; i++) t->column_labels[i] = lab[t->col_order[i]];
    for(i = 0; i < t->ncols; i++) lab[i] = t->column_choice[i];
    for(i = 0; i < t->ncols; i++) t->column_choice[i] = lab[t->col_order[i]];
    Free(lab);

    for(i = 0; i < t->ncols; i++) t->col_order[i] = i;

    MmTableResetColumnLimits(tw);
}

int Table::getRowStates(vector<bool> &states)
{
    MmTablePart *t = &tw->mmTable;

    states.clear();
    for(int i = 0; i < t->nrows; i++) states.push_back((bool)t->row_state[i]);
    return (int)states.size();
}

int Table::setRowStates(vector<bool> &states)
{
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    int n = (int)states.size();
    if(n > t->nrows) n = t->nrows;

    int *rows = new int[n];
    Boolean *s = new Boolean[n];

    for(int i = 0; i < n; i++) {
	rows[i] = i;
	s[i] = (Boolean)states[i];
    }

    _TCanvasSelectRows(tw->mmTable.canvas, n, rows, s);

    delete [] rows;
    delete [] s;

    _MmTableResetInfo(tw);

    if(t->do_internal_select_rowCB) {
	doSelectRowCallback(tw, -1);
    }

    return t->nrows;
}

bool Table::rowSelected()
{
    MmTablePart *t = &tw->mmTable;
    for(int i = 0; i < t->nrows; i++) {
	if(t->row_state[t->row_order[i]]) return true;
    }
    return false;
}

bool Table::columnSelected()
{
    MmTablePart *t = &tw->mmTable;
    for(int i = 0; i < t->ncols; i++) {
	if(t->column_state[t->col_order[i]]) return true;
    }
    return false;
}

int Table::getSelectedRows(vector<int> &rows)
{
    MmTablePart *t = &tw->mmTable;

    rows.clear();

    for(int i = 0; i < t->nrows; i++) {
	if(t->row_state[t->row_order[i]]) rows.push_back(t->row_order[i]);
    }
    return (int)rows.size();
}

int Table::getDeselectedRows(vector<int> &rows)
{
    MmTablePart *t = &tw->mmTable;

    rows.clear();

    for(int i = 0; i < t->nrows; i++) {
	if(!t->row_state[t->row_order[i]] ) rows.push_back(t->row_order[i]);
    }
    return (int)rows.size();
}

int Table::getSelectedRowsOrdered(vector<int> &rows)
{
    rows.clear();
    int n = getSelectedRows(rows);
    if(n > 0) sortInts(rows, 0, n-1);
    return n;
}

const char **Table::getColumnLabels()
{
    MmTablePart *t = &tw->mmTable;
    int j;
    const char **labels = (const char **)NULL;

    if(!(labels = (const char **)MallocIt(tw, t->ncols*sizeof(const char *)))) {
	return (const char **)NULL;
    }
    for(j = 0; j < t->ncols; j++) labels[j] = t->column_labels[j];
    return labels;
}

int Table::getColumnLabels(vector<const char *> &labels)
{
    MmTablePart *t = &tw->mmTable;

    labels.clear();
    for(int j = 0; j < t->ncols; j++) {
	labels.push_back((const char *)t->column_labels[j]);
    }
    return (int)labels.size();
}

void Table::setColumnLabel(int col, const string &label)
{
    MmTablePart *t = &tw->mmTable;
    char *c;

    if(col < 0 || col >= t->ncols) return;

    c = strdup(label.c_str());
    Free(t->column_labels[col]);
    t->column_labels[col] = c;
    MmTableAdjustColumnWidths(tw, col, col);
}

void Table::setColumnChoice(int col, const string &choice)
{
    MmTablePart *t = &tw->mmTable;
    char *c;

    if(col < 0 || col >= t->ncols) return;

    c = strdup(choice.c_str());
    Free(t->column_choice[col]);
    t->column_choice[col] = c;
}

void Table::setCellChoice(int row, int col, const string &choice)
{
    MmTablePart *t = &tw->mmTable;
    CellChoice *c;
    int i;

    if(row < 0 || row >= t->nrows || col < 0 || col >= t->ncols) return;

    for(i = 0; i < t->num_cell_choices &&
	(t->cell_choice[i].row != row || t->cell_choice[i].col != col); i++);
    if(i < t->num_cell_choices) {
	c = &t->cell_choice[i];
	Free(c->choice);
    }
    else {
	t->cell_choice = (CellChoice *)ReallocIt(tw, t->cell_choice,
		(t->num_cell_choices+1)*sizeof(CellChoice));
	c = &t->cell_choice[t->num_cell_choices++];
    }
    c->row = row;
    c->col = col;
    c->choice = strdup(choice.c_str());
}

void Table::selectRow(int row, bool select)
{
    MmTableEditModeOff(tw);
    _TCanvasSelectRow(tw->mmTable.canvas, row, select);
    _MmTableResetInfo(tw);
    if(tw->mmTable.do_internal_select_rowCB) {
	doSelectRowCallback(tw, -1);
    }
}

void Table::selectRowWithCB(int row, bool select)
{
    MmTablePart *t = &tw->mmTable;
    TCanvasPart *tc = &t->canvas->tCanvas;
    int i;

    tc->select_data.states = (Boolean *)ReallocIt(tw, tc->select_data.states,
                (t->nrows+t->nhidden)*sizeof(Boolean));
    tc->select_data.changed_rows = (int *)ReallocIt(tw,
                tc->select_data.changed_rows,(t->nrows+t->nhidden)*sizeof(int));

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	tc->select_data.states[i] = t->row_state[i];
    }

    MmTableEditModeOff(tw);
    _TCanvasSelectRow(tw->mmTable.canvas, row, select);
    _MmTableResetInfo(tw);
    doSelectRowCB(tw);
}

static void
doSelectRowCB(MmTableWidget w)
{
    MmTablePart *t = &w->mmTable;
    TCanvasPart *tc = &t->canvas->tCanvas;
    MmTableSelectCallbackStruct *c;
    int i;

    c = &tc->select_data;
    c->nrows = t->nrows;
    c->nchanged_rows = 0;

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	if(c->states[i] != t->row_state[i]) {
	    c->changed_rows[c->nchanged_rows++] = i;
	}
	c->states[i] = t->row_state[i];
    }
    if(c->nchanged_rows > 0) {
	XtCallCallbacks((Widget)w, XtNselectRowCallback, c);
    }
}

void Table::selectAllRows(bool state)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    if(!t->select_toggles) MmTableEditModeOff(tw);
    for(i = 0; i < t->nrows + t->nhidden; i++) t->row_state[i] = state;
    _TCanvasRedisplay(t->canvas);
    _MmTableResetInfo(tw);
    if(t->do_internal_select_rowCB) {
	doSelectRowCallback(tw, -1);
    }
}

void Table::selectAllRowsWithCB(bool state)
{
    int i;
    MmTablePart *t = &tw->mmTable;
    TCanvasPart *tc = &t->canvas->tCanvas;

    tc->select_data.states = (Boolean *)ReallocIt(tw, tc->select_data.states,
                (t->nrows+t->nhidden)*sizeof(Boolean));
    tc->select_data.changed_rows = (int *)ReallocIt(tw,
                tc->select_data.changed_rows,(t->nrows+t->nhidden)*sizeof(int));

    for(i = 0; i < t->nrows + t->nhidden; i++) {
	tc->select_data.states[i] = t->row_state[i];
	t->row_state[i] = state;
    }
    MmTableEditModeOff(tw);
    _TCanvasRedisplay(t->canvas);
    _MmTableResetInfo(tw);
    doSelectRowCB(tw);
}

void Table::selectRows(int num_rows, int *rows, bool *states)
{
    MmTableEditModeOff(tw);
    if(num_rows > 0) {
	_TCanvasSelectRows(tw->mmTable.canvas, num_rows,rows,(Boolean *)states);
	_MmTableResetInfo(tw);
    }
    if(tw->mmTable.do_internal_select_rowCB) {
	doSelectRowCallback(tw, -1);
    }
}

void Table::selectRows(vector<int> &rows, vector<bool> &states)
{
    MmTableEditModeOff(tw);
    if((int)rows.size() > 0) {
	int *r = new int[rows.size()];
	bool *b = new bool[rows.size()];
	for(int i = 0; i < (int)rows.size(); i++) {
	    r[i] = rows[i];
	    b[i] = (i < (int)states.size()) ? states[i] : false;
	}
	_TCanvasSelectRows(tw->mmTable.canvas, (int)rows.size(),r,(Boolean *)b);
	delete [] r;
	delete [] b;
	_MmTableResetInfo(tw);
    }
    if(tw->mmTable.do_internal_select_rowCB) {
	doSelectRowCallback(tw, -1);
    }
}

void Table::selectColumns(vector<int> &columns, vector<bool> &states)
{
    int i;
    MmTablePart *t = &tw->mmTable;

    for(i = 0; i < (int)columns.size() && i < (int)states.size(); i++) {
	if(columns[i] >= 0 && columns[i] < t->ncols) {
	    t->column_state[t->col_order[columns[i]]] = states[i];
	}
    }
    if(t->header) _HCanvasRedisplay(t->header);
    _MmTableResetInfo(tw);
}

void Table::selectAllColumns(bool state)
{
    int i;
    MmTablePart *t = &tw->mmTable;
    for(i = 0; i < t->ncols; i++) t->column_state[i] = state;
    if(t->header) _HCanvasRedisplay(t->header);
    _MmTableResetInfo(tw);
}

static void
sortInts(vector<int> &values, int lo0, int hi0)
{
    int lo = lo0;
    int hi = hi0;
    int mid;

    if(lo0 >= hi0) return;

    mid = values[(lo0+hi0)/2];

    while(lo <= hi) {
	while(lo < hi0 && values[lo] < mid) lo++;
	while(hi > lo0 && values[hi] > mid) hi--;

	if(lo <= hi){
	    if(lo != hi) {
		int i = values[lo];
		values[lo] = values[hi];
		values[hi] = i;
	    }
	    lo++;
	    hi--;
	}
    }
    if(lo0 < hi) sortInts(values, lo0, hi);
    if(lo < hi0) sortInts(values, lo, hi0);
}

int Table::getSelectedColumns(vector<int> &col_indices)
{
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    col_indices.clear();

    for(int i = 0; i < t->ncols; i++) {
	if(t->column_state[t->col_order[i]]) {
	    col_indices.push_back(t->col_order[i]);
	}
    }
    return (int)col_indices.size();
}

/*
bool Table::editMode()
{
    MmTablePart *t = &tw->mmTable;
    return (t->edit_row >= 0 && t->edit_pos >= 0) ? true : false;
}
*/

/*
void lostOwnership(Clipboard clip, Transferable transferable)
{
}
*/

bool Table::fieldSelected()
{
    MmTablePart *t = &tw->mmTable;
    return (t->select_row >= 0 && t->select_char1 != t->select_char2) ?
			true : false;
}

void
MmTableCut(MmTableWidget w, XButtonEvent *event)
{
    MmTablePart *t = &w->mmTable;
    MmTableCopy(w, event);

    if(t->select_row >= 0 && t->select_char1 >= 0 && t->select_char2 >= 0) {
	int c1, c2, n, col;
	String s, s1, s2;
	c1 = (t->select_char1 < t->select_char2) ?
		t->select_char1 : t->select_char2;
	c2 = (t->select_char2 > t->select_char1) ?
		t->select_char2 : t->select_char1;
	s= t->columns[t->col_order[t->select_col]][t->row_order[t->select_row]];
	s1 = substring(w, s, 0, c1);
	s2 = substring(w, s, c2, strlen(s));
	n = strlen(s1) + strlen(s2);
	Free(s);
	if(!(s = (char *)MallocIt(w, n+1))) return;
	snprintf(s, n+1, "%s%s", s1, s2);
	Free(s1);
	Free(s2);
	t->columns[t->col_order[t->select_col]][t->row_order[t->select_row]]= s;
	t->edit_x = (t->select_x1 < t->select_x2) ? t->select_x1 : t->select_x2;
	t->edit_pos = (t->select_char1 < t->select_char2) ?
		t->select_char1 : t->select_char2;
	col = t->select_col;
	t->select_row = t->select_col = t->select_char1 = t->select_char2 = -1;
	MmTableAdjustColumnWidths(w, col, col);
    }
}

void
MmTableCopy(MmTableWidget w, XButtonEvent *event)
{
    MmTablePart *t = &w->mmTable;

    if(t->select_row >= 0 && t->select_char1 != t->select_char2) {
	int c1, c2;
	String s;
	c1 = (t->select_char1 < t->select_char2) ?
		t->select_char1 : t->select_char2;
	c2 = (t->select_char2 > t->select_char1) ?
		t->select_char2 : t->select_char1;
	s= t->columns[t->col_order[t->select_col]][t->row_order[t->select_row]];
	Free(t->field_selection);
	t->field_selection = substring(w, s, c1, c2);
	if(XtOwnSelection((Widget)w, XA_PRIMARY, event->time,
		(XtConvertSelectionProc)convertSelection,
		(XtLoseSelectionProc)loseOwnership,
		(XtSelectionDoneProc)transferDone) == False)
	{
	    fprintf(stderr, "MmTable: failed to become selection owner.\n");
	}
    }
}

static Boolean
convertSelection(Widget w, Atom *selection, Atom *target, Atom *type,
		XtPointer *value, unsigned long *length, int *format)
{
    Display *d = XtDisplay((Widget)w);

    *type= *target;
    *format= 8;

    if(*target == XA_STRING ||
            *target == XA_TEXT(d) ||
            *target == XA_COMPOUND_TEXT(d))
    {
	MmTablePart *t = &((MmTableWidget)w)->mmTable;
	if(t->field_selection != (String)NULL) {
	    *length = (int)strlen(t->field_selection) + 1;
	    *value = (XtPointer)malloc(*length);
	     memcpy(*value, t->field_selection, *length);
	}
	else {
	    *length= 1;
	    *value= strdup("");
	}
	return True;
    }
    else
    {
	Boolean success =  XmuConvertStandardSelection(w, CurrentTime,
		selection, target, type, (char **)value, length, format);
	if(success && *target == XA_TARGETS(d))
        {
            Atom *tmp;
            tmp = (Atom *) realloc(*value, (*length + 2) * sizeof(Atom));
            tmp[(*length)++] = XInternAtom(XtDisplay(w), "MULTIPLE", False);
            tmp[(*length)++] = XA_COMPOUND_TEXT(d);
            *value = (XtPointer) tmp;
        }
	return success;
    }
}

static void
loseOwnership(MmTableWidget w, Atom *selection)
{
    Free(w->mmTable.field_selection);
}

static void
transferDone(MmTableWidget w, Atom *selection, Atom *target)
{
    /* Nothing to do here. Don't free, so the selection can be pasted
	somewhere more than once.
    Free(w->mmTable.field_selection);
     */
}

void
MmTablePaste(MmTableWidget w, XButtonEvent *event)
{
    MmTablePart *t = &w->mmTable;

    if(!t->editable) return;

    XtGetSelectionValue((Widget)w, XA_PRIMARY, XA_STRING,
		(XtSelectionCallbackProc)requestorCallback, event, event->time);
}

static void
requestorCallback(MmTableWidget w, XButtonEvent *event, Atom *selection,
		Atom *type, XtPointer value, unsigned long *length, int *format)
{
    MmTablePart *t = &w->mmTable;

    if((*type == 0 /* XT_CONVERT_FAIL */) || (*length == 0)) {
	XBell(XtDisplay(w), 100);
	fprintf(stderr, "MmTable: no selection or selection timed out.\n");
	return;
    }

    if( *type == XA_STRING || *type == XA_COMPOUND_TEXT(XtDisplay(w)) )
    {
	if(t->edit_row >= 0 && t->edit_pos >= 0) {
	    int n;
    	    MmTablePart *t = &w->mmTable;
	    String s1 = NULL, s2 = NULL;
	    String s =
	       t->columns[t->col_order[t->edit_col]][t->row_order[t->edit_row]];
	    s1 = substring(w, s, 0, t->edit_pos);
	    s2 = substring(w, s, t->edit_pos, (int)strlen(s));
	    if(t->backup[t->col_order[t->edit_col]][t->row_order[t->edit_row]]
		== (String)NULL)
	    {
	      t->backup[t->col_order[t->edit_col]][t->row_order[t->edit_row]]=s;
	    }
	    else {
		Free(s);
	    }
	    n = (s1 != NULL) ? (int)strlen(s1) : 0;
	    n += *length;
	    if(s2 != NULL) n += (int)strlen(s2);
	    n += 1;
	    if(!(s = (char *)MallocIt(w, n))) return;
	    s[0] = '\0';
	    if(s1 != NULL) stringcpy(s, s1, n);
	    strncat(s, (char *)value, *length);
	    if(s2 != NULL) strcat(s, s2);
	    Free(s1);
	    Free(s2);
	    t->columns[t->col_order[t->edit_col]][t->row_order[t->edit_row]] =s;
	    MmTableAdjustColumnWidths(w, t->edit_col, t->edit_col);
	}
    }
    free(value);
}

void
MmTableEditModeOff(MmTableWidget w)
{
    MmTablePart *t = &w->mmTable;
    Boolean on = False;
    MmTableEditCallbackStruct c;

    if(!XtIsRealized((Widget)w)) return;

    if(t->edit_row >= 0) {
	int row = t->row_order[t->edit_row];
	int col = t->col_order[t->edit_col];
	String s = t->columns[col][row];
	if(!t->select_toggles) {
	    t->row_state[row] = t->edit_row_state;
	}
	c.column = col;
	c.row = row;
	c.old_string = t->edit_string;
	c.new_string = s;
	t->edit_row = -1;
	_TCanvasRedisplay(t->canvas);
	XtCallCallbacks((Widget)w, XtNeditModeCallback, &on);

	if(strcmp(s, t->edit_string)) {
	    XtCallCallbacks((Widget)w, XtNmodifyVerifyCallback, &c);
	}
    }
}

bool Table::fieldEdited(int row, int col)
{
    MmTablePart *t = &tw->mmTable;

    if(row < 0 || row > t->nrows+t->nhidden || col < 0 || col >t->ncols) {
	return false;
    }
    return (t->backup[col][row] != (String)NULL &&
	strcmp(t->backup[col][row], t->columns[col][row])) ? true : false;
}

bool Table::rowEdited(int row)
{
    MmTablePart *t = &tw->mmTable;
    
    if(row < 0 || row > t->nrows+t->nhidden) return false;
    for(int j = 0; j < t->ncols; j++) if(fieldEdited(row, j)) return true;
    return false;
}

bool Table::edited()
{
    int i, j;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	for(i = 0; i < t->nrows + t->nhidden; i++) {
    	    if(t->backup[j][i] != (String)NULL &&
		strcmp(t->backup[j][i], t->columns[j][i])) return true;
	}
    }
    return false;
}

void Table::cancelEdit()
{
    MmTablePart *t = &tw->mmTable;
    if(t->edit_row >= 0) {
	Free(t->columns[t->edit_col][t->edit_row]);
	t->columns[t->edit_col][t->edit_row] = t->edit_string;
	t->edit_row = -1;
	MmTableAdjustColumnWidths(tw, t->edit_col, t->edit_col);
    }
}

void Table::backup()
{
    int i, j;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	for(i = 0; i < t->nrows + t->nhidden; i++) {
	    if(t->backup[j][i] != (String)NULL) {
		Free(t->backup[j][i]);
	    }
	}
    }
}

void Table::restore()
{
    int i, j;
    Boolean found_one = False;
    MmTablePart *t = &tw->mmTable;

    for(j = 0; j < t->ncols; j++) {
	for(i = 0; i < t->nrows + t->nhidden; i++) {
	    if(t->backup[j][i] != (String)NULL) {
		Free(t->columns[j][i]);
		t->columns[j][i] = t->backup[j][i];
	    	t->backup[j][i] = (String)NULL;
		found_one = True;
	    }
	}
    }
    if(found_one) {
	MmTableAdjustColumns(tw);
    }
}

void Table::restoreField(int row, int col)
{
    MmTablePart *t = &tw->mmTable;

    if(row >= 0 && row < t->nrows + t->nhidden && col >= 0 && col < t->ncols)
    {
	if(t->backup[col][row] != (String)NULL) {
	    Free(t->columns[col][row]);
	    t->columns[col][row] = t->backup[col][row];
	    t->backup[col][row] = (String)NULL;
	    MmTableAdjustColumns(tw);
	}
    }
}

static void *
MallocIt(MmTableWidget w, int nbytes)
{
    void *ptr;

    if(nbytes <= 0) {
	return (void *)NULL;
    }
    else if((ptr = malloc(nbytes)) == (void *)0)
    {
        char error[200];
        if(errno > 0) {
            snprintf(error, sizeof(error), "malloc error: %s", strerror(errno));
        }
        else {
            snprintf(error, sizeof(error), "malloc error on %d bytes", nbytes);
        }
	if(XtHasCallbacks((Widget)w, XtNwarningCallback) != XtCallbackHasNone) {
	    XtCallCallbacks((Widget)w, XtNwarningCallback, error);
	}
	else {
	    fprintf(stderr, "%s\n", error);
	}
	return (void *)NULL;
    }
    return ptr;
}

static void *
ReallocIt(MmTableWidget w, void *ptr, int nbytes)
{
    if(nbytes <= 0) {
	return ptr;
    }
    else if(ptr == (void *)NULL) {
	return MallocIt(w, nbytes);
    }
    else if((ptr = realloc(ptr, nbytes)) == (void *)0)
    {
	char error[200];
	if(errno > 0) {
	    snprintf(error, sizeof(error), "malloc error: %s", strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error), "malloc error on %d bytes", nbytes);
	}
	if(XtHasCallbacks((Widget)w, XtNwarningCallback) != XtCallbackHasNone) {
	    XtCallCallbacks((Widget)w, XtNwarningCallback, error);
	}
	else {
	    fprintf(stderr, "%s\n", error);
	}
	return (void *)NULL;
    }
    return ptr;
}

static int
getWidth(Widget w)
{
    Dimension width;
    Arg args[1];

    XtSetArg(args[0], XmNwidth, &width);
    XtGetValues(w, args, 1);
    return width;
}

int
MmTableGetBarValue(Widget w)
{
    int value;
    Arg args[1];
    XtSetArg(args[0], XmNvalue, &value);
    XtGetValues(w, args, 1);
    return value;
}

void Table::getScrolls(int *horizontal_pos, int *vertical_pos)
{
    MmTablePart *t = &tw->mmTable;
    *horizontal_pos = MmTableGetBarValue(t->hbar);
    *vertical_pos = MmTableGetBarValue(t->vbar);
}

void Table::setScrolls(int horizontal_pos, int vertical_pos)
{
    MmTablePart *t = &tw->mmTable;
   int value, visible, min, max;

    MmTableSetBarValue(t->hbar, horizontal_pos);
    MmTableSetBarValue(t->vbar, vertical_pos);

    _TCanvasHScroll(t->canvas);
    _TCanvasVScroll(t->canvas);
    if(t->header) _HCanvasRedisplay(t->header);

    getBarValues(t->hbar, &value, &visible, &min, &max);
    t->last_hbar_value = value;
    getBarValues(t->vbar, &value, &visible, &min, &max);
    t->last_vbar_value = value;

    _MmTableResetInfo(tw);
}

void
MmTableSetBarValue(Widget w, int value)
{
    int visible, min, max;
    Arg args[3];

    XtSetArg(args[0], XmNsliderSize, &visible);
    XtSetArg(args[1], XmNminimum, &min);
    XtSetArg(args[2], XmNmaximum, &max);
    XtGetValues(w, args, 3);

    if(value > max-visible) value = max-visible;
    else if(value < min) value = min;
    XtSetArg(args[0], XmNvalue, value);
    XtSetValues(w, args, 1);
}

static void
setBarValues(Widget w, int value, int visible, int min, int max)
{
    int n;
    Arg args[5];

    if(value > max-visible) value = max-visible;
    if(value < min) value = min;
    n = 0;
    XtSetArg(args[n], XmNvalue, value); n++;
    if(visible < 1) visible = 1;
    XtSetArg(args[n], XmNsliderSize, visible); n++;
    XtSetArg(args[n], XmNminimum, min); n++;
    XtSetArg(args[n], XmNmaximum, max); n++;
    XtSetArg(args[n], XmNincrement, 1); n++;
    XtSetValues(w, args, n);
}

static void
getBarValues(Widget w, int *value, int *visible, int *min, int *max)
{
    int n;
    Arg args[4];

    n = 0;
    XtSetArg(args[n], XmNvalue, value); n++;
    XtSetArg(args[n], XmNsliderSize, visible); n++;
    XtSetArg(args[n], XmNminimum, min); n++;
    XtSetArg(args[n], XmNmaximum, max); n++;
    XtGetValues(w, args, n);
}

static int
stringWidth(MmTableWidget w, const char *s)
{
    if(s == (String)NULL || (int)strlen(s) == 0) return 0;
    return XTextWidth(w->mmTable.font, s, (int)strlen(s));
}

static String
substring(MmTableWidget w, String s, int i1, int i2)
{
    String sub;
    if(i2 < i1 || !(sub = (char *)MallocIt(w, i2-i1+1))) return (String)NULL;

    strncpy(sub, s+i1, i2-i1);
    sub[i2-i1] = '\0';

    return sub;
}

int
MmTableGetBarVisibleAmount(Widget w)
{
    int visible;
    Arg args[1];

    XtSetArg(args[0], XmNsliderSize, &visible);
    XtGetValues(w, args, 1);
    return visible;
}

Pixel
string_to_pixel(Widget widget, const char *color_name)
{
	XrmValue from, to;

	from.size = sizeof(String);
	from.addr = XtMalloc(50);
	stringcpy(from.addr, color_name, 50);

	XtConvert(widget, XtRString, &from, XtRPixel, &to);

	XtFree(from.addr);
	return( *(Pixel *) to.addr );
}

void Table::warn(const string &message)
{
    MmTablePart *t = &tw->mmTable;
    if(t->table_info) InfoSetText(t->table_info, (char *)message.c_str());
}

void Table::info(const string &message)
{
    MmTablePart *t = &tw->mmTable;
    char *c;

    c = strdup(message.c_str());
    free(t->table_info_text);
    t->table_info_text = c;

    if(t->table_info) InfoSetText(t->table_info, (char *)message.c_str());
}

void
_MmTableResetInfo(MmTableWidget w)
{
    if(w->mmTable.table_info != NULL)
    {
	InfoSetText(w->mmTable.table_info, w->mmTable.table_info_text);
    }
}

static void
mousePressedButton1(MmTableWidget w, XEvent *event, String *params,
                        Cardinal *num_params)
{
    int i, top, row;
    MmTablePart *t = &w->mmTable;
    int cursor_x = ((XButtonEvent *)event)->x;
    int cursor_y = ((XButtonEvent *)event)->y;

    if(!t->select_toggles || t->nrows <= 0
	|| cursor_x > t->canvas->core.x) return;

    cursor_y -= t->canvas->core.y;

    top = MmTableGetBarValue(t->vbar);
    row = cursor_y/t->row_height + top;
    if(row > t->nrows-1) row = t->nrows-1;

    if(t->singleSelect && t->radioSelect && t->row_state[t->row_order[row]]) {
	/* cannot turn off a selection */
	return;
    }
    t->row_state[t->row_order[row]] = !t->row_state[t->row_order[row]];

    _TCanvas_drawToggle(t->canvas, row);

    if(t->singleSelect && t->row_state[t->row_order[row]])
    {
	for(i = 0; i < t->nrows; i++)
	    if(i != t->row_order[row] && t->row_state[i])
	{
	    t->row_state[i] = False;
	    _TCanvas_drawToggle(t->canvas, i);
	}
    }

    doSelectRowCallback(w, row);

    _MmTableResetInfo(w);
}

static void
doSelectRowCallback(MmTableWidget w, int row)
{
    MmTablePart *t = &w->mmTable;
    TCanvasPart *tc = &t->canvas->tCanvas;
    MmTableSelectCallbackStruct *c;

    tc->select_data.states = (Boolean *)ReallocIt(w, tc->select_data.states,
				t->nrows*sizeof(Boolean));
    tc->select_data.changed_rows = (int *)ReallocIt(w,
			tc->select_data.changed_rows, t->nrows*sizeof(int));

    c = &tc->select_data;
    c->nrows = t->nrows;
    if(row >= 0) {
	c->nchanged_rows = 1;
	c->changed_rows[0] = t->row_order[row];
    }
    else  {
	c->nchanged_rows = -1;
    }
    memcpy(c->states, t->row_state, t->nrows*sizeof(Boolean));

    XtCallCallbacks((Widget)w, XtNselectRowCallback, c);

    Free(tc->select_data.states);
    Free(tc->select_data.changed_rows);
}

Widget
MmTableCreate(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
        return (XtCreateWidget(name, mmTableWidgetClass, parent,
			arglist, argcount));
}

char * MmTableGetCellChoice(MmTableWidget w, int row, int col)
{
    MmTablePart *t = &w->mmTable;
    int i;
	
    for(i = 0; i < t->num_cell_choices &&
	(t->cell_choice[i].row != row || t->cell_choice[i].col != col); i++);
    if(i < t->num_cell_choices) {
	return t->cell_choice[i].choice;
    }
    return NULL;
}

int Table::getSortUnique(vector<int> &cols)
{
    MmTablePart *t = &tw->mmTable;

    cols.clear();
    if(t->sort_unique.cols_length > 0) {
	for(int i = 0; i < t->sort_unique.cols_length; i++) {
	    cols.push_back(t->sort_unique.cols[i]);
	}
    }
    return (int)cols.size();
}

int Table::getColumnIndex(const string &label)
{
    MmTablePart *t = &tw->mmTable;
    int i;

    for(i = 0; i < t->ncols && label.compare(t->column_labels[i]); i++);
    if(i < t->ncols) {
	return i;
    }
    return -1;
}

int Table::getSelectedAndHiddenRows(vector<int> &rows)
{
    int i, m, n, key_col = -1;
    char *key=NULL;
    MmTablePart *t = &tw->mmTable;

    MmTableEditModeOff(tw);

    if(t->sort_unique.cols_length > 0 && t->nhidden > 0)
    {
	key_col = t->sort_unique.cols[t->sort_unique.cols_length-1];
    }
    if(key_col >= t->ncols) key_col = -1;

    n = 0;
    for(i = 0; i < t->nrows; i++) {
	if(t->row_state[t->row_order[i]]) {
	    if(!key && key_col >= 0) key = t->columns[key_col][t->row_order[i]];
	    n++;
	}
    }
    if(key)
    {
	m = 0;
	for(i = 0; i < t->nhidden; i++) {
	    if(!strcmp(key, t->columns[key_col][t->nrows+i])) m++;
	}
	n += m;
    }

    rows.clear();

    for(i = 0; i < t->nrows; i++) {
	if(t->row_state[t->row_order[i]]) rows.push_back(t->row_order[i]);
    }
    if(key)
    {
	for(i = 0; i < t->nhidden; i++) {
/*
	    if(!strcmp(key, t->columns[key_col][t->nrows+i])) {
		row_indices[n++] = t->row_order[t->nrows+i];
	    }
*/
	    if(!strcmp(key, t->columns[key_col][t->nrows+i])) {
		rows.push_back(t->nrows+i);
	    }
	}
    }
    return (int)rows.size();
}

void Table::setCellEditable(int row, int column, bool state)
{
    MmTablePart *t = &tw->mmTable;
    int i;

    for(i = 0; i < t->num_cell_non_editable
	&& (t->cell_non_editable[i].row != row ||
		t->cell_non_editable[i].col != column); i++);
    if(!state) {
	if(i == t->num_cell_non_editable) {
	    t->cell_non_editable = (CellIndex *)ReallocIt(tw,
		t->cell_non_editable,
		(t->num_cell_non_editable+1)*sizeof(CellIndex));
	    t->cell_non_editable[i].row = row;
	    t->cell_non_editable[i].col = column;
	    t->num_cell_non_editable++;
	}
    }
    else {
	if(i < t->num_cell_non_editable) {
	   if(i < t->num_cell_non_editable-1) {
		memmove(t->cell_non_editable+i, t->cell_non_editable+i+1,
			(t->num_cell_non_editable-1-i)*sizeof(CellIndex));
		t->num_cell_non_editable--;
	   }
	}
    }
}

bool Table::cellEditable(int row, int column)
{
    MmTablePart *t = &tw->mmTable;
    int i;

    for(i = 0; i < t->num_cell_non_editable
	&& (t->cell_non_editable[i].row != row ||
		t->cell_non_editable[i].col != column); i++);
    return (i == t->num_cell_non_editable) ? true : false;
}

void Table::displayVerticalLines(bool display)
{
    Arg args[1];
    MmTablePart *t = &tw->mmTable;
    TCanvasWidget tc = t->canvas;

    XtSetArg(args[0], XtNverticalLines, display);
    XtSetValues((Widget)tc, args, 1);
}

void Table::displayHorizontalLines(bool display)
{
    Arg args[1];
    MmTablePart *t = &tw->mmTable;
    TCanvasWidget tc = t->canvas;

    XtSetArg(args[0], XtNhorizontalLines, display);
    XtSetValues((Widget)tc, args, 1);
}

void Table::setTime(double epoch)
{
    MmTablePart *t = &tw->mmTable;
    TCanvasWidget tc = t->canvas;

    if(!t->color_only) {
	fprintf(stderr, "MmTableSetTime: must set XtNcolorOnly=true.\n");
	return;
    }
    if(epoch != t->epoch_time) {
	t->epoch_time = epoch;
	if(XtIsRealized((Widget)tw)) {
	    _TCanvasDrawXLabels(tc);
	}
    }
}

void Table::setTdels(double large_tdel, int n_small_tdel)
{
    MmTablePart *t = &tw->mmTable;

    if(!t->color_only) {
	fprintf(stderr, "MmTableSetTdels: must set XtNcolorOnly=true.\n");
	return;
    }
    if(large_tdel <= 0. || n_small_tdel < 1) {
	fprintf(stderr, "MmTableSetTdels: invalid arguments.\n");
	return;
    }

    if(large_tdel != t->large_tdel || n_small_tdel != t->n_small_tdel) {
	t->n_small_tdel = n_small_tdel;
	t->small_tdel = large_tdel/n_small_tdel;
	t->large_tdel = large_tdel;
	if(XtIsRealized((Widget)tw)) {
	    _TCanvasRedisplay(t->canvas);
	    _TCanvasDrawXLabels(t->canvas);
	}
    }
}

void Table::getTime(double *epoch, double *small_tdel, double *large_tdel)
{
    MmTablePart *t = &tw->mmTable;

    *epoch = t->epoch_time;
    *small_tdel = t->small_tdel;
    *large_tdel = t->large_tdel;
}

XFontStruct * Table::getFont()
{
    MmTablePart *t = &tw->mmTable;
    return t->font;
}

void MmTableSetClass(MmTableWidget w, Table *t)
{
    w->mmTable.table_class = t;
}
