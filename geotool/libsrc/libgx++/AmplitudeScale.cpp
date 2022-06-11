/** \file AmplitudeScale.cpp
 *  \brief Defines class AmplitudeScale.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "AmplitudeScale.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
}

AmplitudeScale::AmplitudeScale(const string &name, Component *parent) :
			FormDialog(name, parent, false, false)
{
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

void AmplitudeScale::createInterface(void)
{
    Arg args[10];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    initial_layout_label = new Label("Initial Layout", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, initial_layout_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form1 = new Form("form1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    data_height_text = new TextField("Data Height", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, data_height_text->baseWidget()); n++;
    data_height_label1 = new Label("Data Height (Pixels)", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form2 = new Form("form2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    data_separation_text = new TextField("Data Separation", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, data_separation_text->baseWidget()); n++;
    data_separation_label = new Label("Data Separation (Pixels)", form2,args,n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form2->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    apply_layout_button = new Button("Apply Layout", this, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, apply_layout_button->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    set_current_scale_label = new Label("Set Current Scale", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, set_current_scale_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form3 = new Form("form3", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    scale_data_height_text = new TextField("scaleDataHeight", form3, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget,scale_data_height_text->baseWidget()); n++;
    new Label("Data Height (Pixels)", form3, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form3->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    scale_rb = new RadioBox("scaleRB", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    all_toggle = new Toggle("All", scale_rb, this, args, n);
    all_toggle->set(true, false);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    selected_toggle = new Toggle("Selected", scale_rb, this, args, n);
    selected_toggle->set(false, false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form3->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 15); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, scale_rb->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    waveforms_label = new Label("Waveforms", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, scale_rb->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    visible_rb = new RadioBox("visibleRB", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    visible_toggle = new Toggle("Visible", visible_rb, this, args, n);
    visible_toggle->set(true, false);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    entire_toggle = new Toggle("Entire", visible_rb, this, args, n);
    entire_toggle->set(false, false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, scale_rb->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 15); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, visible_rb->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    apply_scale_button = new Button("Apply Scale", this, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, apply_scale_button->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    set_type_label = new Label("Set Type", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, set_type_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    type_rb = new RadioBox("Type", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    independent_toggle = new Toggle("Independent", type_rb, this, args, n);
    independent_toggle->set(true, false);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    uniform_toggle = new Toggle("Uniform", type_rb, this, args, n);
    uniform_toggle->set(false, false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, type_rb->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep3 = new Separator("sep3", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep3->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);
}

/** Destructor.
 */
AmplitudeScale::~AmplitudeScale(void)
{
}

void AmplitudeScale::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Toggle *t = comp->getToggleInstance();

    if(!strcmp(cmd, "Close")) {
        setVisible(false);
    }
    else if(!strcmp(cmd, "Apply Layout")) {
	dialog_state = DIALOG_APPLY;
	doCallbacks(base_widget, (XtPointer)APPLY_LAYOUT, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Apply Scale")) {
	doCallbacks(base_widget, (XtPointer)APPLY_SCALE, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Independent")) {
	if(t && t->state()) {
	    doCallbacks(base_widget, (XtPointer)SET_TYPE_INDEPENDENT,
			XmNactivateCallback);
	}
    }
    else if(!strcmp(cmd, "Uniform")) {
	if(t && t->state()) {
	    doCallbacks(base_widget, (XtPointer)SET_TYPE_UNIFORM,
			XmNactivateCallback);
	}
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Amplitude Scale");
    }
}

ParseCmd AmplitudeScale::parseCmd(const string &cmd, string &msg)
{
    bool err;
    string c;

    if(parseArg(cmd, "Layout_Height", c)) {
	data_height_text->setString(c, true);
    }
    else if(parseArg(cmd, "Layout_Separation", c)) {
	data_separation_text->setString(c, true);
    }
    else if(parseCompare(cmd, "Apply_Layout")) {
	dialog_state = DIALOG_APPLY;
	doCallbacks(base_widget, (XtPointer)0, XmNactivateCallback);
    }
    else if(parseArg(cmd, "Current_Height", c)) {
	scale_data_height_text->setString(c, true);
    }
    else if(parseArg(cmd, "Waveforms", c)) {
	return visible_rb->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Time_Period", c)) {
	return scale_rb->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Set_Type", c) || parseArg(cmd, "Type", c)) {
	return type_rb->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Apply_Scale")) {
	doCallbacks(base_widget, (XtPointer)1, XmNactivateCallback);
    }
    else if(parseFind(cmd, "set_layout", msg, &err,
		"height", true, "separation", true))
    {
        if(parseGetArg(cmd, "height", c)) {
            data_height_text->setString(c);
        }
        if(parseGetArg(cmd, "separation", c)) {
            data_separation_text->setString(c);
        }
	dialog_state = DIALOG_APPLY;
	doCallbacks(base_widget, (XtPointer)0, XmNactivateCallback);
    }
    else if(parseFind(cmd, "set_current_scale", msg, &err,
		"height", true, "selected_only", false, "visible_only", false))
    {
        bool b;
	
        if(parseGetArg(cmd, "height", c)) {
            scale_data_height_text->setString(c);
        }
	all_toggle->set(true, true);
        if(parseGetArg(cmd, "set_current_scale", msg, "selected_only", &b))
	{
	    selected_toggle->set(true, true);
        }
	else if(!msg.empty()) {
            return ARGUMENT_ERROR;
	}
	entire_toggle->set(true, true);
        if(parseGetArg(cmd, "set_current_scale", msg, "visible_only", &b))
	{
	    visible_toggle->set(true, true);
        }
	else if(!msg.empty()) {
            return ARGUMENT_ERROR;
	}
	    
	dialog_state = DIALOG_APPLY;
	doCallbacks(base_widget, (XtPointer)1, XmNactivateCallback);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
        getParsePrefix(prefix, sizeof(prefix));
        parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

/** Print the string commands excepted by this class.
 *  @param[in] prefix the command prefix
 */
void AmplitudeScale::parseHelp(const char *prefix)
{
    printf("%slayout_height=NUM\n", prefix);
    printf("%slayout_separation=NUM\n", prefix);
    printf("%sapply_layout\n", prefix);
    printf("%scurrent_height\n", prefix);
    printf("%swaveforms.all\n", prefix);
    printf("%swaveforms.selected\n", prefix);
    printf("%stime_period.visible\n", prefix);
    printf("%stime_period.entire\n", prefix);
    printf("%sapply_scale\n", prefix);
    printf("%stype.independent\n", prefix);
    printf("%stype.uniform\n", prefix);
}
