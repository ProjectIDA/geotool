/** \file TaperWindow.cpp
 *  \brief Defines class TaperWindow.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>

#include "TaperWindow.h"
#include "motif++/MotifClasses.h"

TaperWindow::TaperWindow(const string &name, Component *parent,
		int width_percent, int min_pts, int max_pts)
		: FormDialog(name, parent, false, false)
{
    int n;
    char s[50];
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    rc = new RowColumn("rc", this, args, n);

    width_label = new Label("Width (%)", rc);
    min_label = new Label("Min Points", rc);
    max_label = new Label("Max Points", rc);

    if(width_percent < 0 || width_percent > 50) width_percent = 10;
    snprintf(s, sizeof(s), "%d", width_percent);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNvalue, s); n++;
    width = new TextField("width", rc, args, n);

    snprintf(s, sizeof(s), "%d", min_pts);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNvalue, s); n++;
    min_points = new TextField("min_points", rc, args, n);

    snprintf(s, sizeof(s), "%d", max_pts);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNvalue, s); n++;
    max_points = new TextField("max_points", rc, args, n);
}


void TaperWindow::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
}

ParseCmd TaperWindow::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "Width", c)) {
	width->setString(c, true);
    }
    else if(parseArg(cmd, "Min_Points", c)) {
	min_points->setString(c, true);
    }
    else if(parseArg(cmd, "Max_Points", c)) {
	max_points->setString(c, true);
    }
    else if( parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
	return COMMAND_PARSED;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar TaperWindow::parseVar(const string &name, string &value)
{
    if(parseCompare(name, "width")) {
	width->getString(value);
    }
    else if(parseCompare(name, "min_points")) {
	min_points->getString(value);
    }
    else if(parseCompare(name, "max_points")) {
	max_points->getString(value);
    }
    else {
	return FormDialog::parseVar(name, value);
    }
    return STRING_RETURNED;
}

void TaperWindow::parseHelp(const char *prefix)
{
    printf("\n");
    printf("%swidth PERCENT\n", prefix);
    printf("%smin_points NUM\n", prefix);
    printf("%smax_points NUM\n\n", prefix);
}
