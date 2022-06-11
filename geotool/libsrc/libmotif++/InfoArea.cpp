/** \file InfoArea.cpp
 *  \brief Defines class InfoArea.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/InfoArea.h"
#include "motif++/Frame.h"
#include "motif++/Application.h"

/** Constructor.
 *  @param[in] name the name given to this InfoArea instance.
 *  @param[in] parent the Component parent.
 *  @param[in] type INFO_NORMAL (two InfoClass children) or INFO_LEFT_ONLY
 *	(one InfoClass child)
 */
InfoArea::InfoArea(const string &name, Component *parent, InfoType type) :
			Component(name, parent)
{
    init(parent, type);
}

/** Constructor for a Frame parent.
 *  @param[in] name the name given to this InfoArea instance.
 *  @param[in] frame the Frame parent.
 *  @param[in] type INFO_NORMAL (two InfoClass children) or INFO_LEFT_ONLY
 *	(one InfoClass child)
 */
InfoArea::InfoArea(const string &name, Frame *frame, InfoType type) :
			Component(name, frame)
{
    init(frame->workForm(), type);
    frame->setInfoArea(this);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] type INFO_NORMAL (two InfoClass children) or INFO_LEFT_ONLY
 *	(one InfoClass child)
 */
void InfoArea::init(Component *parent, InfoType type)
{
    info_type = type;

    base_widget = XtVaCreateWidget(getName(),
				xmFormWidgetClass, parent->baseWidget(),
				XmNbottomAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNfractionBase, 2,
				XmNhighlightThickness, 0,
				XmNheight, 20,
				NULL);
    installDestroyHandler();

    Arg args[10];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_IN); n++;
    left_area = new Form("left_area", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 2); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 2); n++;
    left_info = new InfoClass("left_info", left_area, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, left_area->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_IN); n++;
    right_area = new Form("right_area", this, args, n);

    if(info_type == INFO_NORMAL)
    {
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopOffset, 2); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomOffset, 2); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 2); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 2); n++;
	right_info = new InfoClass("right_info", right_area, args, n);
    }
    else {
	right_info = NULL;
    }

    warning = NULL;
}

/** Display a warning Button. A button labelled " ! " is displayed on the
 *  right side of the InfoArea. The Application Warning dialog is displayed
 *  when the button is selected. After it is selected, the warning button is
 *  hidden until this function is called again.
 */
void InfoArea::warningButtonOn()
{
    Arg args[10];
    int n;

    if(!warning) {
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNmarginHeight, 0); n++;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	warning = new Button(" ! ", this, args, n, this);
    }
    warning->setVisible(true);

    n = 0;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, warning->baseWidget()); n++;
    right_area->setValues(args, n);
}

/** Process the action from the warning button.
 */
void InfoArea::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == warning) {
	Application::displayWarnings();
	warning->setVisible(false);
	Arg args[1];
	XtSetArg(args[0], XmNrightAttachment, XmATTACH_FORM);
	right_area->setValues(args, 1);
    }
}

