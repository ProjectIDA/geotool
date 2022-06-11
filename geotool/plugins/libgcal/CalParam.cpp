/** \file CalParam.cpp
 *  \brief Defines class CalParam
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "CalParam.h"
#include "motif++/MotifClasses.h"

using namespace libgcal;


CalParam::CalParam(const char *name, Component *parent,
			ActionListener *listener) : ParamDialog(name, parent)
{
    createInterface();

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void CalParam::createInterface()
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
    compute_button = new Button("Compute", controls, this);
    iteration_button = new Button("Iteration", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    lab1 = new Label("CALEX5a Parameters", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNtopWidget, lab1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    rc = new RowColumn("rc", form, args, n);

    label[0] = new Label("alias", rc);
    label[1] = new Label("m0", rc);
    label[2] = new Label("m2", rc);
    label[3] = new Label("qac", rc);
    label[4] = new Label("ns1", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    text[0] = new TextField("alias", rc, args, n);
    text[0]->setString("1");
    text[1] = new TextField("m0", rc, args, n);
    text[1]->setString("0");
    text[2] = new TextField("m2", rc, args, n);
    text[2]->setString("1");
    text[3] = new TextField("qac", rc, args, n);
    text[3]->setString("1.e-5");
    text[4] = new TextField("ns1", rc, args, n);
    text[4]->setString("1");

    label[5] = new Label("m", rc);
    label[6] = new Label("m1", rc);
    label[7] = new Label("maxit", rc);
    label[8] = new Label("finac", rc);
    label[9] = new Label("ns2", rc);

    text[5] = new TextField("m", rc, args, n);
    text[5]->setString("4");
    text[6] = new TextField("m1", rc, args, n);
    text[6]->setString("0");
    text[7] = new TextField("maxit", rc, args, n);
    text[7]->setString("48");
    text[8] = new TextField("finac", rc, args, n);
    text[8]->setString("1.e-2");
    text[9] = new TextField("ns2", rc, args, n);
    text[9]->setString("0");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    lab2 = new Label("System Parameters", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc2 = new RowColumn("rc2", form, args, n);

    input_label = new Label("Input Signal File", rc2);
    n = 0;
    XtSetArg(args[n], XmNeditMode, XmSINGLE_LINE_EDIT); n++;
    input_text = new TextField("input_file", rc2, args, n);
    output_label = new Label("Output Signal File", rc2);
    output_text = new TextField("output_file", rc2, args, n);
    partitle_text = new TextField("partitle", rc2);
    partitle_text->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, lab2->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 20); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 20); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, True); n++;
    XtSetArg(args[n], XmNscrollVertical, True); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, False); n++;
    XtSetArg(args[n], XmNrows, 10); n++;
    param_text = new TextField("param_text", sw, args, n);
}

CalParam::~CalParam(void)
{
}

void CalParam::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute")) {
	doCallbacks(base_widget, (XtPointer)0, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Iteration")) {
	doCallbacks(base_widget, (XtPointer)1, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Calibration Parameters");
    }
}
