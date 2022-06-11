/** \file Warnings.cpp
 *  \brief Defines class Warnings.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/Warnings.h"
#include "motif++/MotifComp.h"
#include "motif++/Application.h"
#include "motif++/RowColumn.h"
#include "motif++/Button.h"

/** Constructor.
 *  @param[in] parent the Component parent.
 */
Warnings::Warnings(Component *parent) :
		FormDialog("Warnings", parent, false, false)
{
    Widget sep;

    show_warn = NULL;
    warn_no = 1;
    last_n = 0;
    last = NULL;
    pos = 0;

    setSize(600, 200);

    Arg args[7];
    int n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;

    controls = new RowColumn("controls", this, args, n, false);

    new Button("Close", controls, this);

    redirect = XtVaCreateManagedWidget("Redirect to Terminal",
			xmToggleButtonWidgetClass, controls->baseWidget(),
			XmNset, False,
			NULL);

    XtAddCallback(redirect, XmNvalueChangedCallback,
		Application::redirectCallback, (XtPointer)this);

    XtManageChild(controls->baseWidget());

    sep = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, base_widget,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_WIDGET,
			XmNbottomWidget, controls->baseWidget(),
			NULL);

    sw = XtVaCreateWidget("sw", xmScrolledWindowWidgetClass, base_widget,
			XmNtopAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_WIDGET,
			XmNbottomWidget, sep,
			NULL);

    text = XtVaCreateWidget("text", xmTextWidgetClass, sw,
			XmNeditMode, XmMULTI_LINE_EDIT,
			XmNeditable, False,
			XmNblinkRate, 0,
			XmNautoShowCursorPosition, True,
			XmNcursorPositionVisible, False,
			NULL);
    XtManageChild(text);
    XtManageChild(sw);
    XtSetMappedWhenManaged(XtParent(base_widget), False);
    XtManageChild(base_widget);
    XtSetMappedWhenManaged(XtParent(base_widget), True);
    XtUnmanageChild(base_widget);
}

/** Process the Close button action.
 */
void Warnings::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
}

/** The callback for the Warn.redirect Toggle.
 */
void Warnings::redirectWarnings(Widget widget, XtPointer client_data, XtPointer)
{
    Application::getApplication()->redirect_warnings
		= XmToggleButtonGetState(widget);
}

/** Append a message to the list.
 */
void Warnings::append(char *warning)
{
    char *warn_string;

    warn_string = strstr(warning, ":");

    if(XmToggleButtonGetState(redirect))
    {
	if(warning[0] != '\n') fprintf(stderr, "%s\n", warning);
	else fprintf(stderr, "%s\n", warning+1);
	return;
    }
    else if(last != NULL && warn_string != NULL && !strcmp(warn_string, last))
    {
	if(pos == 0) {
	    XmTextReplace(text, pos, pos+last_n, warning+1);
	}
	else {
	    XmTextReplace(text, pos, pos+last_n, warning);
	}
    }
    else
    {
	pos = XmTextGetLastPosition(text);
	XmTextInsert(text, pos, warning);
	if(last) free(last);
	last = (warn_string) ? strdup(warn_string) : NULL;
    }
    last_n = strlen(warning)+1;
    adjustScrollbar();
}   

/** Adjust the scrollbar to display the last warning message.
 */
void Warnings::adjustScrollbar(void)
{
    Widget scroll;
    int	value, slider_size, maximum, incr, page_incr;

    XtVaGetValues(sw, XmNverticalScrollBar, &scroll, NULL);
    if(scroll != NULL) {
	XtVaGetValues(scroll,	XmNsliderSize, &slider_size,
				XmNmaximum, &maximum,
				NULL);
	XmScrollBarGetValues(scroll, &value, &slider_size, &incr, &page_incr);
	value = maximum - slider_size;
	XmScrollBarSetValues(scroll, value, slider_size, incr, page_incr, True);
    }
    XtVaGetValues(sw, XmNhorizontalScrollBar, &scroll, NULL);
    if(scroll != NULL) {
	XtVaGetValues(scroll,	XmNsliderSize, &slider_size,
				XmNmaximum, &maximum,
				NULL);
	XmScrollBarGetValues(scroll, &value, &slider_size, &incr, &page_incr);
	value = 0;
	XmScrollBarSetValues(scroll, value, slider_size, incr, page_incr, True);
    }
}
