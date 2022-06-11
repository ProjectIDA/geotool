/** \file ShapeTable.cpp
 *  \brief Defines class ShapeTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "ShapeTable.h"
#include "motif++/MotifClasses.h"
#include "MapThemeFile.h"
#include "MapWindow.h"
#include "widget/Table.h"

extern "C" {
#include "libstring.h"
#ifdef HAVE_NETCDF
#include "libgCDF.h"
#endif /* HAVE_NETCDF */
}

using namespace libgmap;

ShapeTable::ShapeTable(const char *name, Component *parent,
		MapThemeFile *theme_file, DBFHandle dbf_handle, int num_cols)
		: FormDialog(name, parent)
{
    Arg args[20];
    int i, j, n, *alignment=NULL, width, decimals;
    char **labels=NULL;
    const char **choice=NULL;
    const char **row=NULL;
    char col_label[12], s[100];
    bool *col_editable=NULL;

    tf = theme_file;
    dbf = dbf_handle;
    num_cols += 2;
    ncols = num_cols;
    irow = 0;

    if(!(labels = (char **)mallocWarn(ncols*sizeof(char *))) ||
        !(alignment = (int *)mallocWarn(ncols*sizeof(int))) ||
        !(choice = (const char **)mallocWarn(ncols*sizeof(char *))) ||
        !(col_editable = (bool *)mallocWarn(ncols*sizeof(bool))) ||
        !(row = (const char **)mallocWarn(ncols*sizeof(char *))))
    {
	return;
    }
    labels[0] = strdup("Index");
    alignment[0] = RIGHT_JUSTIFY;
    choice[0] = "";
    col_editable[0] = False;
    labels[1] = strdup("Display");
    alignment[1] = LEFT_JUSTIFY;
    choice[1] = "on:off";
    col_editable[1] = True;

    for(i = 2; i < ncols; i++) {
        DBFFieldType type;

        type = DBFGetFieldInfo(dbf, i-2, col_label, &width, &decimals);
        labels[i] = strdup(col_label);
        if(type == FTString) {
            alignment[i] = LEFT_JUSTIFY;
        }
        else {
            alignment[i] = RIGHT_JUSTIFY;
        }
        choice[i] = "";
        col_editable[i] = False;
    }

    info_area = new InfoArea("info_area", this);
    info_area->setVisible(true);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, info_area->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    close_button = new Button("Close", rc, this);
    apply_button = new Button("Apply", rc, this);
    apply_button->setSensitive(false);
    turn_on_button = new Button("Turn On", rc, this);
    turn_on_button->setSensitive(false);
    turn_off_button = new Button("Turn Off", rc, this);
    turn_off_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNcolumns, ncols); n++;
    XtSetArg(args[n], XtNcolumnLabels, labels); n++;
    XtSetArg(args[n], XtNcolumnChoice, choice); n++;
    XtSetArg(args[n], XtNtableTitle, name); n++;
    XtSetArg(args[n], XtNwidth, 500); n++;
    XtSetArg(args[n], XtNheight, 400); n++;
    table = new Table("table", this, info_area, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNvalueChangedCallback);
    table->addActionListener(this, XtNchoiceChangedCallback);

    table->setAlignment(ncols, alignment);
    Free(alignment);
    table->setColumnEditable(col_editable);
    Free(col_editable);
    for(i = 0; i < ncols; i++) free((char *)labels[i]);
    Free(labels);
    Free(choice);

    nrows = DBFGetRecordCount(dbf);

    tf->mw->setCursor("hourglass");
    if(nrows < 1000)
    {
	for(i = 0; i < nrows; i++)
	{
	    char index[20];
	    snprintf(index, 20, "%d", i+1);
	    row[0] = strdup(index);
	    row[1] = strdup("on");
	    for(j = 2; j < ncols; j++)
	    {
		DBFFieldType type;
		type = DBFGetFieldInfo(dbf, j-2,col_label,&width,&decimals);

		if(DBFIsAttributeNULL(dbf, i, j-2))
		{
		    row[j] = strdup("(NULL)");
		}
		else
		{
		    switch(type)
		    {
		    case FTString:
			row[j] = strdup(DBFReadStringAttribute(dbf, i,j-2));
			break;
		    case FTInteger:
			snprintf(s, sizeof(s), "%d",
                                DBFReadIntegerAttribute(dbf, i, j-2));
			row[j] = strdup(s);
			break;
		    case FTDouble:
			snprintf(s, sizeof(s), "%lf",
                                DBFReadDoubleAttribute(dbf, i, j-2));
			row[j] = strdup(s);
			break;
		    default:
			row[j] = strdup("");
			break;
		    }
                }
	    }
	    table->addRow(row, False);
	    for(j = 0; j < ncols; j++) free((char *)row[j]);
	}
	DBFClose(dbf);
	table->adjustColumns();
    }
    else {
	for(i = 0; i < nrows; i++)
	{
	    char index[20];
	    snprintf(index, 20, "%d", i+1);
	    row[0] = index;
	    row[1] = "on";
	    for(j = 2; j < ncols; j++) row[j] = "";
	    table->addRow(row, False);
	}
	XtAppAddWorkProc(Application::getAppContext(),workProc,(XtPointer)this);
    }

    Free(row);
    tf->mw->setCursor("default");
}

ShapeTable::~ShapeTable(void)
{
}

Boolean ShapeTable::workProc(XtPointer client)
{
    ShapeTable *s = (ShapeTable *)client;
    return s->fillTable();
}

bool ShapeTable::fillTable(void)
{
    int j, width, decimals;
    char col_label[12], s[100];

    const char **row = table->getRow(irow);

    int i = irow;

    for(j = 2; j < ncols; j++)
    {
	DBFFieldType type;
	type = DBFGetFieldInfo(dbf, j-2, col_label, &width, &decimals);

	if(DBFIsAttributeNULL(dbf, i, j-2))
	{
	    row[j] = strdup("(NULL)");
	}
	else
	{
	    switch(type)
	    {
	    case FTString:
		row[j] = strdup(DBFReadStringAttribute(dbf, i,j-2));
		break;
	    case FTInteger:
		snprintf(s, sizeof(s), "%d",DBFReadIntegerAttribute(dbf,i,j-2));
		row[j] = strdup(s);
		break;
	    case FTDouble:
		snprintf(s, sizeof(s), "%lf",DBFReadDoubleAttribute(dbf,i,j-2));
		row[j] = strdup(s);
		break;
	    default:
		row[j] = strdup("");
		break;
	    }
	}
    }
    table->setRow(irow, row);
    for(j = 2; j < ncols; j++) free((char *)row[j]);
    Free(row);
    if(++irow >= nrows) {
	DBFClose(dbf);
	table->adjustColumns();
	return true;
    }
    return false;
}

void ShapeTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if(!strcmp(reason, XtNselectRowCallback)) {
	vector<int> rows;
	int num = table->getSelectedAndHiddenRows(rows);
	tf->mw->map->setShapeSelected(tf->getId(), rows);
	turn_on_button->setSensitive((num > 0) ? true : false);
	turn_off_button->setSensitive((num > 0) ? true : false);
    }
    else if(!strcmp(reason, XtNvalueChangedCallback) ||
	    !strcmp(reason, XtNchoiceChangedCallback))
    {
	MmTableEditCallbackStruct *c =
		(MmTableEditCallbackStruct *)action_event->getCalldata();
	if(c->column == 1)
	{
	    bool change = false;
	    bool displayed = tf->mw->map->shapeIsDisplayed(tf->getId(), c->row);
	    if((!strcmp(c->new_string, "off") && displayed) ||
		(!strcmp(c->new_string, "on") && !displayed)) change = true;
	    apply_button->setSensitive(change);
	}
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
    }
    else if(!strcmp(cmd, "Turn On")) {
	turnOn();
    }
    else if(!strcmp(cmd, "Turn Off")) {
	turnOff();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
}

ParseCmd ShapeTable::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;

    if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseCompare(cmd, "apply")) {
	apply();
    }
    else if(parseCompare(cmd, "turn_on")) {
	turnOn();
    }
    else if(parseCompare(cmd, "turn_off")) {
	turnOff();
    }
    else {
        return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void ShapeTable::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%sturn_on\n", prefix);
    printf("%sturn_off\n", prefix);
    Table::parseHelp(prefix);
}

void ShapeTable::apply(void)
{
    vector<const char *> col;
    int i, num_rows;

    if(!tf || !tf->mw->map) return;

    num_rows = table->getColumn(1, col);

    for(i = 0; i < num_rows; i++) {
	bool display_shape = !strcmp(col[i], "on") ? true : false;
	tf->mw->map->displayShape(tf->getId(), i, display_shape, false);
    }
    tf->mw->map->update();
    apply_button->setSensitive(false);
}

void ShapeTable::turnOn(void)
{
    int i, num;
    vector<int> selected;

    if(!(num = table->getSelectedRows(selected))) return;

    for(i = 0; i < num; i++) {
	table->setField(selected[i], 1, "on", false);
    }
    table->adjustColumns();

    checkForChange();
}

void ShapeTable::turnOff(void)
{
    int i, num;
    vector<int> selected;

    if(!(num = table->getSelectedRows(selected))) return;

    for(i = 0; i < num; i++) {
	table->setField(selected[i], 1, "off", false);
    }
    table->adjustColumns();

    checkForChange();
}

void ShapeTable::checkForChange(void)
{
    int i, num_rows;
    vector<const char *> col;
    bool change, table_state;

    num_rows = table->getColumn(1, col);

    change = false;
    for(i = 0; i < num_rows; i++) {
	table_state = !strcmp(col[i], "on") ? true : false;
	if(table_state != tf->mw->map->shapeIsDisplayed(tf->getId(), i)) {
	    change = true;
	    break;
	}
    }
    apply_button->setSensitive(change);
}
