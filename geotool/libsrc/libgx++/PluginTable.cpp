/** \file PluginTable.cpp
 *  \brief Defines class PluginTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

using namespace std;

#include "PluginTable.h"
#include "motif++/Application.h"
#include "motif++/PlugInManager.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"

PluginTable::PluginTable(const string &name, Component *parent)
                        : FormDialog(name, parent)
{
    createInterface();
}

void PluginTable::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
    setValues(args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
//    help_button = new Button("Help", controls, this);
//    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNcolumns, 4); n++;
    XtSetArg(args[n], XtNvisibleRows, 30); n++;
    XtSetArg(args[n], XtNwidth, 450); n++;
    const char *column_labels[] = {"name", "description", "library", "path"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    XtSetArg(args[n], XtNcolumnSelectable, False); n++;
    XtSetArg(args[n], XtNtableTitle, "Plugins"); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNeditSaveCallback);
    table->addActionListener(this, XtNeditCancelCallback);

    list();
}

PluginTable::~PluginTable(void)
{
}

void PluginTable::list(void)
{
    Application *app = Application::getApplication();
    PlugInManager *manager = app->plugInManager();
    const char *last_name=NULL;
    const char *row[4];

    table->removeAllRows();

    for(int i = 0; i < manager->numPlugins(); i++) {
	PlugInList p = manager->pluginList(i);

	if(!p.p.application_name ||
		!strcmp(app->getName(), p.p.application_name))
	{
	    // only show unique names. there can be multiple plugins with the
	    // same name/library but with different parent_class's
	    if( !last_name || strcmp(p.p.name, last_name) ) {
		row[0] = p.p.name;
		row[1] = p.p.description;
		row[2] = p.library_name;
		row[3] = p.library_path;

		for(int j = 0; j < 4; j++) if(row[j] == NULL) row[j] = "-";
		table->addRow(row, true);
		last_name = p.p.name;
	    }
	}
    }
    table->adjustColumns();
}

void PluginTable::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == close_button) {
	setVisible(false);
    }
}
