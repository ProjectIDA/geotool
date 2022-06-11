/** \file Residuals.cpp
 *  \brief Defines class Residuals.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "Residuals.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"

extern "C" {
#include "libstring.h"
}

using namespace libglc;

Residuals::Residuals(const char *name, Component *parent) : Frame(name, parent)
{
    createInterface();
    init();
}

void Residuals::createInterface(void)
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);


    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("form", frame_form, args, n);

    more_button = new Button("+", rc, this);
    less_button = new Button("-", rc, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XtNtableTitle, "Residual Colors"); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 12); n++;
    XtSetArg(args[n], XtNcolumns, 7); n++;
    const char *labels[] = {"phase", "delta-min", "delta-max", "residual",
		"begin-value", "end-value", "color"};
    XtSetArg(args[n], XtNcolumnLabels, labels); n++;
    table = new Table("table", frame_form, args, n);

    if(!tool_bar->loadDefaults()) {
	tool_bar->add(close_button, "Close");
    }
}

Residuals::~Residuals(void)
{
}

void Residuals::init(void)
{
    int i, j, num = 0;
    string value;
    ostringstream os;
    char *last, *tok, *s, *prop;
    const char *row[7];

    if(getProperty("Geotool.locate.numResiduals", value))
    {
	if(!stringToInt(value.c_str(), &num)) {
	    return;
	}
    }

    for(i = 0; i < num; i++)
    {
	os.str("");
	os << "Geotool.locate.residual" << i;
	if((prop = getProperty(os.str())) != NULL)
	{
	    tok = prop;
	    for(j = 0; j < 7 && (s = strtok_r(tok, ",\t ", &last)) != NULL; j++)
	    {
		tok = NULL;
		row[j] = s;
	    }
	    table->addRow(row, false);
	    Free(prop);
	}
    }
    if(num <= 0) {
	const char *row1[7] = {"P", "30", "100", "timres", "5.","10.","yellow"};
	const char *row2[7] = {"P", "30", "100", "timres", "10.", "", "red"};
	table->addRow(row1, true);
	table->addRow(row2, true);
    }
    table->adjustColumns();
}

void Residuals::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "+")) {
	moreResiduals();
    }
    else if(!strcmp(cmd, "-")) {
	lessResiduals();
    }
}

void Residuals::moreResiduals(void)
{
    int nrows = table->numRows();
    if(nrows > 0) {
	vector<const char *> row;
	if(table->getRow(nrows-1, row) == 0) return;
	table->addRow(row, true);
    }
    else {
	const char *row[7] = {"P", "30", "100", "timres", "5.", "10.", "red"};
	table->addRow(row, true);
    }
}

void Residuals::lessResiduals(void)
{
    int nrows;

    if((nrows = table->numRows()) > 1) {
	table->removeRow(nrows-1);
    }
}

