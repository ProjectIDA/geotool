/** \file Stassocs.cpp
 *  \brief Defines class Stassocs
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>

using namespace std;

#include "Stassocs.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

using namespace libgarrival;


Stassocs::Stassocs(const char *name, Component *parent, DataSource *ds) :
			Frame(name, parent), DataReceiver(ds)
{
    createInterface();
    init();
}

void Stassocs::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    open_button = new Button("Open", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    delete_button = new Button("Delete", edit_menu, this);
    delete_button->setSensitive(false);
    edit_button = new Button("Edit", edit_menu, this);
    edit_button->setSensitive(false);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Stassocs Help", help_menu, this);

//    tool_bar->add(close_button, "Close");

    n = 0;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNvisibleRows, 15); n++;
    XtSetArg(args[n], XtNwidth, 450); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    table = new ctable<CssStassocClass>("Stassocs", frame_form, args, n);
//    table->setType(cssStassoc);
    table->setVisible(true);
    table->addActionListener(this, XtNselectRowCallback);
// don't need this for CSSTable ???
//    table->addActionListener(this, XtNattributeChangeCallback);

//    fileDialog = NULL;
//    saveDialog = NULL;
//    print_window = NULL;
//    table_query = NULL;

    addPlugins("Stassocs", data_source, NULL);

    tool_bar->loadDefaults();  // load toolbar after plugins.
}

void Stassocs::init(void)
{
    list();
}

Stassocs::~Stassocs(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void Stassocs::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
//    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Attributes...")) {
        table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Edit")) {
	table->editModeOn();
    }
    else if(comp == table) {
	bool set = (table->numSelectedRows() > 0) ? true : false;
	delete_button->setSensitive(set);
	edit_button->setSensitive(set);
    }
/*
    else if(!strcmp(reason, XtNattributeChangeCallback)) {
	list();
    }
*/
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Stassocs Help");
    }
}

void Stassocs::list(void)
{
    cvector<CssStassocClass> stassocs;

    if(!data_source) return;
    data_source->getTable(stassocs);

    table->removeAllRecords();
    table->addRecords(stassocs);
    table->adjustColumns();
}
