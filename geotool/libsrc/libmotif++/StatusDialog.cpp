/** \file StatusDialog.cpp
 *  \brief Defines class StatusDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "motif++/StatusDialog.h"
#include "motif++/MotifComp.h"
#include "motif++/Scale.h"
#include "motif++/Button.h"
#include "motif++/Application.h"

/** Constructor.
 *  @param[in] name the name given to this StatusDialog instance.
 *  @param[in] parent the Component parent.
 *  @param[in] label_format a label containing an integer format (like %d)
 *  @param[in] total the total number of computations needed.
 *  @param[in] listener a Component listener for the XmNactivateCallback action.
 */
StatusDialog::StatusDialog(const string &name, Component *parent,
		const string &label_format, int total, ActionListener *listener)
		: FormDialog(name, parent, false, false)
{
    createInterface(label_format, total);
    enableCallbackType(XmNactivateCallback);
    if(listener) addActionListener(listener);
}

/** Constructor.
 *  @param[in] label_format a label containing an integer format (like %d)
 *  @param[in] total the total number of computations needed.
 */
void StatusDialog::createInterface(const string &label_format, int total)
{
    char str[100];
    int n;
    Arg args[20];

    snprintf(str, sizeof(str), label_format.c_str(), total);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    label = new Label(str, this, args, n);

    if(total > 0)
    {
	n = 0;
	XtSetArg(args[n], XmNminimum, 0); n++;
	XtSetArg(args[n], XmNmaximum, total); n++;
	XtSetArg(args[n], XmNvalue, 0); n++;
	XtSetArg(args[n], XmNshowValue, True); n++;
	XtSetArg(args[n], XmNresizable, True); n++;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
	slider = new Scale("slider", this, NULL, args, n);
    }
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    if(total > 0) {
	XtSetArg(args[n], XmNtopWidget, slider->baseWidget()); n++;
    }
    else {
	XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    }
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNtopWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;

    stop = new Button("Stop", this, args, n, this);

    continue_working = true;

//    XFlush(XtDisplay(base_widget));
}

/** Destructor */
StatusDialog::~StatusDialog(void)
{
}

/** Process the action form the Stop button.
 */
void StatusDialog::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Stop")) {
	continue_working = false;
	doCallbacks(comp->baseWidget(), NULL,(const char *)XmNactivateCallback);
    }
}

/** Update the StatusDialog scale. This function returns true to indicate that
 *  the computation should continue. After the Stop button has been selected,
 *  this function will returns false.
 *  @param[in] count the number of computations completed. 0 <= count <= total.
 *  @returns true or false true means continue the computation. false means
 *	stop the computation.
 */
bool StatusDialog::update(int count)
{   
    XEvent event;
    XtAppContext app = Application::getApplication()->appContext();

    if(!continue_working) return false;

    if(slider) {
	slider->setValue(count);
	XFlush(XtDisplay(base_widget));
    }
    int check =0;
    while((XtAppPending(app) & XtIMXEvent) && ++check < 1000)
    {
	XtAppNextEvent(app, &event);
	XtDispatchEvent(&event);
    }
    return continue_working;
}

static StatusDialog *status_dialog = NULL;
static Component *status_parent = NULL;

/** A C interface to this class. If this function is called with a status of 0,
 *  a new StatusDialog is created and displayed. The function setParent must
 *  be called before this function is called with a status equal to 0. When this
 *  function is called with a status of 1, the StatusDialog scale value
 *  indicator is repositioned to the value of the count argument.
 *  @param[in] w this argument is ignored
 *  @param[in] count if status is 0, this is the total number of computations.
 *   if status is 1, this is the current count. if status is 2, the window is
 *   closed.
 *  @param[in] status 0, 1 or 2.
 *  @param[in] label_format a label containing an integer format (like %d)
 *  @returns 0 or 1. 0 means stop the computation. 1 means continue the
 *	computation.
 */
extern "C" int WorkingDialog(void *w, int count, int status,
			const char *label_format)
{
    if(status == 0) {
	if(status_dialog) status_dialog->destroy();
	if(!status_parent) {
	    fprintf(stderr, "StatusDialog: parent not set.\n");
	    return 0;
	}
	status_dialog = new StatusDialog("Working", status_parent, label_format,
				count);
	status_dialog->setVisible(true, CENTER_DIALOG);
	return 1;
    }
    else if(status == 1) {
	if(status_dialog) {
	    return (int)status_dialog->update(count);
	}
    }
    else if(status == 2 && status_dialog) {
	status_dialog->setVisible(false);
	status_dialog->destroy();
	status_dialog = NULL;
   }
   return 0;
}

/** Set the StatusDialog parent before the C interface function WorkingDialog
 *  is called.
 *  @param[in] parent
 */
void StatusDialog::setParent(Component *parent)
{
    status_parent = parent;
}
