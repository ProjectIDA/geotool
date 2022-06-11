/** \file Warn.cpp
 *  \brief Defines class Warn.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/Warn.h"
#include "motif++/RowColumn.h"
#include "motif++/Button.h"
#include "motif++/Toggle.h"


/** Constructor.
 *  @param[in] msg the name and the message to display.
 *  @param[in] parent the Component parent.
 */
Warn::Warn(const string &msg, Component *parent) :
		FormDialog("Warning", parent, false, false)
{
    Widget w;
    XmString xm = createXmString(msg);

    w = XtVaCreateManagedWidget("label", xmLabelWidgetClass,
			base_widget,
			XmNlabelString, xm,
			XmNtopAttachment, XmATTACH_FORM,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 10,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 10,
			NULL);
    XmStringFree(xm);

    w = XtVaCreateManagedWidget("separator", xmSeparatorWidgetClass,
			base_widget,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, w,
			XmNtopOffset, 10,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			NULL);

    Arg args[7];
    int n = 0;

    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, w); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;

    row_column = new RowColumn("row_column", this, args, n);

    close = new Button("Close", row_column, this);

    redirect = new Toggle("Redirect to Terminal", row_column);
    more_warnings = new Button("More warnings", row_column, this);
    more_warnings->setSensitive(false);
}

/** Process the Close button action.
 */
void Warn::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
	destroy();
    }
}
