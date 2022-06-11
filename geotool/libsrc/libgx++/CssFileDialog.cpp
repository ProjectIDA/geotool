/** \file CssFileDialog.cpp
 *  \brief Defines class CssFileDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>

using namespace std;

#include "CssFileDialog.h"
#include "motif++/MotifClasses.h"
#include "WaveformWindow.h"
#include "TableQuery.h"
#include "gobject++/CssTables.h"

CssFileDialog::CssFileDialog(const string &name, Component *parent,
		FileType filetype, const string &dir, const string &table)
		: FileDialog(name, parent, filetype, dir, 1, NULL,
			table, "Open")
{
    list_button = NULL;
    getFileSuffixes(table);
}

CssFileDialog::CssFileDialog(const string &name, Component *parent,
		FileType filetype, const string &dir, const string &table,
		const string &open_button_name) : FileDialog(name, parent,
		filetype, dir, 1, NULL, table, open_button_name)
{
    list_button = NULL;
    getFileSuffixes(table);
    addListButton(NULL);
}

CssFileDialog::CssFileDialog(const string &name, WaveformWindow *parent,
		FileType filetype, const string &dir, const string &table)
		: FileDialog(name, parent, filetype, dir, 1, NULL,
			table, "Open")
{
    list_button = NULL;
    getFileSuffixes(table);
    addListButton(parent);
}

CssFileDialog::CssFileDialog(const string &name, WaveformWindow *parent,
		FileType filetype, const string &dir, const string &table,
		const string &open_button_name) : FileDialog(name, parent,
		filetype, dir, 1, NULL, table, open_button_name)
{
    list_button = NULL;
    getFileSuffixes(table);
    addListButton(parent);
}

void CssFileDialog::addListButton(DataReceiver *data_receiver)
{
    list_button = new Button("List Contents...", getControls(), this);
    list_button->setSensitive(false);
    tq = new TableQuery("TableQuery", this, false);

    tq->optionMenu()->setVisible(false);
    tq->editMenu()->setVisible(false);
    tq->connection_button->setVisible(false);
    tq->open_button->setVisible(false);
    tq->import_button->setVisible(false);
    tq->new_tq_button->setVisible(false);
    tq->review_origins->setVisible(false);
    tq->update_global_button->setVisible(false);
    tq->edit_button->setVisible(false);
    tq->vi_edit_button->setVisible(false);

    dr = data_receiver;
    tq->setWaveformReceiver(dr);
    getOpenButton()->addActionListener(this, XtNsetSensitiveCallback);
    addActionListener(this, XtNfileSelectCallback);
}

void CssFileDialog::getFileSuffixes(const string &table)
{
    const char **table_names = NULL;
    int num_table_names = CssTableClass::getAllNames(&table_names);

    setFileSuffixes(num_table_names, table_names, table);

    Free(table_names);
}

void CssFileDialog::actionPerformed(ActionEvent *action_event)
{
    if(action_event->getSource() == list_button) {
	char *file = getFile(false);
	tq->removeAllTabs();
	tq->setVisible(true);
	if(file)
	{
	    // get prefix
	    int n = strlen(file);
	    for(int i = n-1; i >= 0 && file[i] != '/'; i--) {
		if(file[i] == '.') {
		    file[i] = '\0';
		    break;
		}
	    }
	    putProperty("file_prefix", file, false);
	    tq->open_db->disconnect();
	    tq->open_db->autoConnectFilePrefix();
	    tq->getAllTables();
	    XtFree(file);
	    tq->tabSetOnTop(cssWfdisc);
	    tq->setWaveformReceiver(dr);
	}
    }
    else if( !strcmp(action_event->getReason(), XtNsetSensitiveCallback) ) {
	list_button->setSensitive((bool)action_event->getCalldata());
    }
    else if( !strcmp(action_event->getReason(), XtNfileSelectCallback) ) {
	Widget w = (Widget)action_event->getCalldata();
	int nitems=0;
	Arg args[1];
	XtSetArg(args[0], XmNselectedItemCount, &nitems);
	XtGetValues(w, args, 1);
	list_button->setSensitive(nitems == 1);
    }
    else {
	FileDialog::actionPerformed(action_event);
    }
}
