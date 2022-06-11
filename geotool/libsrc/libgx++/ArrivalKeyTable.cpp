/** \file ArrivalKeyTable.cpp
 *  \brief Defines class ArrivalKeyTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "ArrivalKeyTable.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
}

ArrivalKeyTable::ArrivalKeyTable(const string &name, Component *parent,
	WaveformPlot *wp) : FormDialog(name, parent, false, false)
{
    Arg args[30];
    int n, ncols, alignment[2];
    const char *labels[2];
    bool col_editable[2];

    wplot = wp;

    ncols = 2;
    labels[0] = "phase";
    labels[1] = "key";
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
    XtSetArg(args[n], XtNwidth, 100); n++;
    XtSetArg(args[n], XtNheight, 200); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNvalueChangedCallback);

    table->setAlignment(ncols, alignment);
    table->setColumnEditable(col_editable);

    loadTable();
}

ArrivalKeyTable::~ArrivalKeyTable(void)
{
}

void ArrivalKeyTable::loadTable(void)
{
    const char *row[2];
    vector<ArrivalKey> *v = wplot->getArrivalKeys();

    table->removeAllRows();

    for(int i = 0; i < (int)v->size(); i++)
    {
	row[0] = v->at(i).name.c_str();
	row[1] = v->at(i).key.c_str();
	table->addRow(row, false);
    }
    table->adjustColumns();

    delete v;
}

void ArrivalKeyTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
//    Component *comp = action_event->getSource();

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
    else if(!strcmp(cmd, "+"))
    {
	vector<const char *> row;
	int nrows = table->numRows();

	table->getRow(nrows-1, row);
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

void ArrivalKeyTable::remove(void)
{
    vector<int> rows;

    table->getSelectedRows(rows);

    if((int)rows.size() > 0) {
	table->removeRows(rows);
	apply_button->setSensitive(true);
	remove_button->setSensitive(false);
    }
}

void ArrivalKeyTable::setVisible(bool visible)
{
    FormDialog::setVisible(visible);

    if(visible) {
	apply_button->setSensitive(false);
    }
}

void ArrivalKeyTable::apply(void)
{
    char name[10], key[10];
    ArrivalKey a;
    vector<ArrivalKey> arrival_keys;
    vector<const char *> row;
    int num = table->numRows();

    for(int i = 0; i < num; i++) {
	table->getRow(i, row);
	stringTrimCopy(name, row[0], sizeof(name));
	stringTrimCopy(key, row[1], sizeof(key));
	if((int)strlen(name) > 0 && (int)strlen(key) > 0) {
	    a.name.assign(name);
	    a.key.assign(key);
	    arrival_keys.push_back(a);
	}
    }
    wplot->setArrivalKeys(arrival_keys);
    apply_button->setSensitive(false);
}

void ArrivalKeyTable::buttonsSensitive(void)
{
    char name[10], key[10];
    vector<ArrivalKey> *v = wplot->getArrivalKeys();
    vector<const char *> row;

    int num = table->numRows();
    if(num != (int)v->size()) {
	delete v;
	apply_button->setSensitive(true);
	return;
    }
    bool state = false;
    for(int i = 0; i < (int)v->size(); i++) {
	table->getRow(i, row);
	stringTrimCopy(name, row[0], sizeof(name));
	stringTrimCopy(key, row[1], sizeof(key));
	if(v->at(i).name.compare(name)) state = true;
	if(v->at(i).key.compare(key)) state = true;
    }
    delete v;
    apply_button->setSensitive(state);
}
