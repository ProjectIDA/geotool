/** \file SearchParam.cpp
 *  \brief Defines class SearchParam
 *  \author Ivan henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

#include "SearchParam.h"
#include "motif++/MotifClasses.h"

using namespace libgarrival;

SearchParam::SearchParam(const char *name, Component *parent) :
			ParamDialog(name, parent)
{
    createInterface();
    enableCallbackType(XmNactivateCallback);
    addActionListener(parent);
}

void SearchParam::createInterface(void)
{
    int n;
    Arg args[15];

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    search_button = new Button("Search", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    rc = new RowColumn("rc", this, args, n);

    min_label = new Label("Minimum Period", rc);
    max_label = new Label("Maximum Period", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "17"); n++;
    min_text = new TextField("Minimum Period", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "23"); n++;
    max_text = new TextField("Maximum Period", rc, args, n);
}

SearchParam::~SearchParam(void)
{
}

void SearchParam::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Search")) {
	doCallbacks(comp->baseWidget(), (XtPointer)NULL, XmNactivateCallback);
    }
}
