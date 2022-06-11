/** \file BGStations.cpp
 *  \brief Defines class BGStations
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>

using namespace std;

#include "BGStations.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"

using namespace libgbm;

BGStations::BGStations(const char *name, Component *parent) :
		FormDialog(name, parent)
{
    createInterface();
}

void BGStations::createInterface()
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XtNwidth, 250); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNtableTitle, "Beam Groups"); n++;
    XtSetArg(args[n], XtNcolumns, 3); n++;
    table = new Table("Beam Group Stations", this, args, n);
    table->setAttributes("Station,%s,Channel,%s,Weight,%.2f");
 
    table->addActionListener(this, XtNattributeChangeCallback);
}

BGStations::~BGStations(void)
{
}

void BGStations::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
}

void BGStations::list(BeamGroup *bg)
{
    char title[200];
    const char *row[3];
    char weight[20];
    bool editable[3];
    TAttribute a[3];

    snprintf(title, sizeof(title), "%s %s", bg->net, bg->group);
    table->removeAllRows();
    table->setTitle(title);
    int num_columns = table->numColumns();

    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
    }

    for(int i = 0; i < (int)bg->sta.size(); i++)
    {
	for(int j = 0; j < num_columns; j++)
	{
	    editable[j] = false;
	    row[j] = "-";

	    if(!strcmp(a[j].name, "Station")) {
		row[j] = bg->sta[i].sta;
	    }
	    else if(!strcmp(a[j].name, "Channel")) {
		row[j] = bg->sta[i].chan;
	    }
	    else if(!strcmp(a[j].name, "Weight")) {
		snprintf(weight, 20, a[j].format, bg->sta[i].wgt);
		row[j] = weight;
	    }
	}
	table->addRow(row, false);
    }

    table->adjustColumns();
    table->sortByColumnLabels("Station,Channel");

    table->setColumnEditable(editable);
}
