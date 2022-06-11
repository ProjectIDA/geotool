/** \file KeyActions.cpp
 *  \brief Defines class KeyActions.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "KeyActions.h"
#include "motif++/MotifClasses.h"
#include "widget/CPlotClass.h"
extern "C" {
#include "libstring.h"
}

KeyActions::KeyActions(const string &name, Component *parent)
	: FormDialog(name, parent, false, false)
{
    Arg args[30];
    int n, ncols, alignment[2];
    const char *labels[2];
    bool col_editable[2];

    ncols = 2;
    labels[0] = "key code";
    labels[1] = "procedure call";
    alignment[0] = LEFT_JUSTIFY;
    alignment[1] = LEFT_JUSTIFY;
    col_editable[0] = true;
    col_editable[1] = true;

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    close_button = new Button("Close", rc, this);
    apply_button = new Button("Apply", rc, this);
    apply_button->setSensitive(false);
    remove_button = new Button("Remove", rc, this);
    remove_button->setSensitive(false);
    defaults_button = new Button("Set Defaults", rc, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    key_plus_button = new Button("+", form, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, key_plus_button->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    key_minus_button = new Button("-", form, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNtableTitle, getName()); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XtNcolumns, ncols); n++;
    XtSetArg(args[n], XtNcolumnLabels, labels); n++;
    XtSetArg(args[n], XtNwidth, 340); n++;
    XtSetArg(args[n], XtNheight, 500); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNvalueChangedCallback);

    table->setAlignment(ncols, alignment);
    table->setColumnEditable(col_editable);

    loadTable();
}

KeyActions::~KeyActions(void)
{
}

void KeyActions::loadTable(void)
{
    const char *row[2];
    CPlotKeyAction *keys;
    int num;

    num = CPlotClass::getKeyActions(&keys);

    table->removeAllRows();

    for(int i = 0; i < num; i++)
    {
	row[0] = keys[i].key_code;
	row[1] = keys[i].proc_call;
	table->addRow(row, false);
    }
    table->adjustColumns();

    Free(keys);
}

void KeyActions::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
	loadTable();
    }
    else if(!strcmp(reason, XtNselectRowCallback))
    {
	remove_button->setSensitive(table->rowSelected());
    }
    else if(!strcmp(reason, XtNvalueChangedCallback))
    {
	buttonsSensitive();
    }
    else if(!strcmp(cmd, "Remove")) 
    {
	remove();
    }
    else if(!strcmp(cmd, "Apply")) 
    {
	apply();
    }
    else if(!strcmp(cmd, "Set Defaults")) 
    {
	setDefaults();
    }
    else if(!strcmp(cmd, "+"))
    {
	const char *row[2] = {"", ""};
	table->addRow(row, true);
	buttonsSensitive();
    }
    else if(!strcmp(cmd, "-"))
    {
	int nrows = table->numRows();

	if(nrows > 1) {
	    table->removeRow(nrows-1);
	}
	buttonsSensitive();
    }
}

void KeyActions::remove(void)
{
    vector<int> rows;
    table->getSelectedRows(rows);

    if((int)rows.size() > 0) {
	table->removeRows(rows);
	apply_button->setSensitive(true);
	remove_button->setSensitive(false);
    }
}

void KeyActions::setVisible(bool visible)
{
    FormDialog::setVisible(visible);

    if(visible) {
	apply_button->setSensitive(false);
    }
}

void KeyActions::apply(void)
{
    char key_code[20], proc_call[50];
    CPlotKeyAction *keys = NULL;
    int n, num = table->numRows();

    keys = new CPlotKeyAction[num];
    n = 0;
    for(int i = 0; i < num; i++) {
	const char **row = table->getRow(i);
	stringTrimCopy(key_code, row[0], sizeof(key_code));
	stringTrimCopy(proc_call, row[1], sizeof(proc_call));
	if((int)strlen(key_code) > 0 && (int)strlen(proc_call) > 0) {
	    strncpy(keys[n].key_code, key_code, sizeof(keys[n].key_code));
	    strncpy(keys[n].proc_call, proc_call, sizeof(keys[n].proc_call));
	    n++;
	}
	Free(row);
    }
    CPlotClass::setKeyActions(n, keys);
    if(keys) delete keys;
    apply_button->setSensitive(false);
}

void KeyActions::buttonsSensitive(void)
{
    char key_code[20], proc_call[50];
    CPlotKeyAction *keys=NULL;
    int num_keys = CPlotClass::getKeyActions(&keys);

    int num = table->numRows();
    if(num != num_keys) {
	Free(keys);
	apply_button->setSensitive(true);
	return;
    }
    bool state = false;
    for(int i = 0; i < num_keys; i++) {
	const char **row = table->getRow(i);
	stringTrimCopy(key_code, row[0], sizeof(key_code));
	stringTrimCopy(proc_call, row[1], sizeof(proc_call));
	if(key_code[0] != '\0' || proc_call[0] != '\0') {
	    if(strcmp(keys[i].key_code, key_code)) state = true;
	    if(strcmp(keys[i].proc_call, proc_call)) state = true;
	}
	Free(row);
    }
    Free(keys);
    apply_button->setSensitive(state);
}

void KeyActions::setDefaults(void)
{
    CPlotClass::setDefaultKeyActions();
    loadTable();
    apply_button->setSensitive(false);
}
