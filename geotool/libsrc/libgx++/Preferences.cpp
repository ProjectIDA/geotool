/** \file Preferences.cpp
 *  \brief Defines class Preferences.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/MotifClasses.h"
#include "libgio.h"
#include "Preferences.h"
#include "ArrivalKeyTable.h"
#include "WaveformPlot.h"
#include "Fonts.h"
#include "KeyActions.h"

extern "C" {
#include "libstring.h"
}


Preferences::Preferences(const string &name, Component *parent,
	WaveformPlot *wave_plot) : FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    wp = wave_plot;

    wp->addDataListener(this); // listen for waveform selection

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);
    cancel_button = new Button("Close", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    label = new Label("Preferences", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc1 = new RowColumn("rc1", rc, args, n);

    waveform_label = new Label("Waveform Info Popup", rc1);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb1 = new RadioBox("rb1", rc1, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    waveform_on_toggle = new Toggle("On", rb1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    waveform_off_toggle = new Toggle("Off", rb1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc2 = new RowColumn("rc2", rc, args, n);

    arrival_label = new Label("Arrival Info Popup", rc2);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb2 = new RadioBox("rb2", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    arrival_on_toggle = new Toggle("On", rb2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    arrival_off_toggle = new Toggle("Off", rb2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc3 = new RowColumn("rc3", rc, args, n);

    scrollbar_label = new Label("Vertical Scrollbar", rc3);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb3 = new RadioBox("rb3", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    left_toggle = new Toggle("Left", rb3, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    right_toggle = new Toggle("Right", rb3, this, args, n);

    string prop;
    if( getProperty("verticalScrollPosition", prop) ) {
	if(!strcasecmp(prop.c_str(), "RIGHT")) {
	    left_toggle->set(false);
	    right_toggle->set(true);
	}
    }

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    sep3 = new Separator("sep3", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep3->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    load_origins = new Choice("origin_load", this, this, args, n);
    load_origins->addItem("Load Associated Origins Only");
    load_origins->addItem("Load Origins within Time Limits");
    load_origins->addItem("Load All Origins");

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, load_origins->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    Separator *sep4 = new Separator("sep4", this, args, n);

    n =  0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep4->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    button_rc = new RowColumn("button_rc", this, args, n);

    arrival_keys_button = new Button("Arrival Keys...", button_rc, this);
    arrival_param_button = new Button("Arrival Parameters...", button_rc, this);
    amp_param_button = new Button("Amplitude Parameters...", button_rc, this);
    fonts_button = new Button("Fonts...", button_rc, this);
    key_action_button = new Button("Key Actions...", button_rc, this);

    arrival_keys = NULL;
    key_actions = NULL;
}

Preferences::~Preferences(void)
{
}

void Preferences::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Toggle *t = comp->getToggleInstance();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Preferences Help");
    }
    else if( t ) {
	if( t->state() ) {
	    if(t == waveform_on_toggle) {
		putProperty("showWaveformInfo", "true");
	    }
	    else if(t == waveform_off_toggle) {
		putProperty("showWaveformInfo", "false");
	    }
	    else if(t == arrival_on_toggle) {
		putProperty("showArrivalInfo", "true");
	    }
	    else if(t == arrival_off_toggle) {
		putProperty("showArrivalInfo", "false");
	    }
	    else if(t == left_toggle) {
		scrollBarLeft(true);
	    }
	    else if(t == right_toggle) {
		scrollBarLeft(false);
	    }
	}
    }
    else if(comp == load_origins) {
	const char *s = load_origins->getChoice();
	if(!strcmp(s, "Load Associated Origins Only")) {
	    putProperty("origin_load", "0");
	}
	else if(!strcmp(s, "Load Origins within Time Limits")) {
	    putProperty("origin_load", "1");
	}
	else { // load all origins
	    putProperty("origin_load", "2");
	}
    }
    else if(!strcmp(cmd, "Arrival Keys...")) {
	if(!arrival_keys) {
	    arrival_keys = new ArrivalKeyTable("Arrival Key Table", this, wp);
	}
	else {
	    arrival_keys->loadTable();
	}
	arrival_keys->setVisible(true);
    }
    else if(!strcmp(cmd, "Arrival Parameters...")) {
	WaveformPlot::showArrivalParams();
    }
    else if(!strcmp(cmd, "Amplitude Parameters...")) {
	WaveformPlot::showAmplitudeParams();
    }
    else if(!strcmp(cmd, "Fonts...")) {
	Fonts::showFonts();
    }
    else if(!strcmp(cmd, "Key Actions...")) {
	if(!key_actions) {
	    key_actions = new KeyActions("Key Actions", this);
	}
	else {
	    key_actions->loadTable();
	}
	key_actions->setVisible(true);
    }
}

void Preferences::setVisible(bool visible)
{
    FormDialog::setVisible(visible);

    if(visible) {
	if( getProperty("showArrivalInfo", true) ) {
	    arrival_on_toggle->set(true, true);
	}
	else {
	    arrival_off_toggle->set(true, true);
	}
	if( getProperty("showWaveformInfo", true) ) {
	    waveform_on_toggle->set(true, true);
	}
	else {
	    waveform_off_toggle->set(true, true);
	}
	int origin_load = getProperty("origin_load", 1);
	if(origin_load == 0) {
	    load_origins->setChoice("Load Associated Origins Only");
	}
	else if(origin_load == 1) {
	    load_origins->setChoice("Load Origins within Time Limits");
	}
	else {
	    load_origins->setChoice("Load All Origins");
	}
    }
}

ParseCmd Preferences::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "Waveform_Info_Popup", c)) {
	if(parseCompare(c, "On") || parseCompare(c, "True")) {
	    waveform_on_toggle->set(true, true);
	}
	else {
	    waveform_off_toggle->set(true, true);
	}
    }
    else if(parseArg(cmd, "Arrival_Info_Popup", c)) {
	if(parseCompare(c, "On") || parseCompare(c, "True")) {
	    arrival_on_toggle->set(true);
	}
	else {
	    arrival_off_toggle->set(true);
	}
    }
    else if(parseArg(cmd, "Vertical_Scrollbar", c) ||
	    parseArg(cmd, "Scrollbar", c))
    {
	if(parseCompare(c, "Left")) {
	    scrollBarLeft(true);
	}
	else if(parseCompare(c, "Right")) {
	    scrollBarLeft(false);
	}
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

void Preferences::parseHelp(const char *prefix)
{
    printf("%swaveform_info_popup=[on,off]\n", prefix);
    printf("%sarrival_info_popup=[on,off]\n", prefix);
    printf("%sclose\n", prefix);
}

void Preferences::scrollBarLeft(bool left)
{
    const char *line;
    Arg args[1];
    XrmDatabase database;

    if(left) {
        line = "*verticalScrollPosition: LEFT";
        putProperty("verticalScrollPosition", "LEFT");
        XtSetArg(args[0], XtNverticalScrollPosition, 0);
    }
    else {
        line = "*verticalScrollPosition: RIGHT";
        putProperty("verticalScrollPosition", "RIGHT");
        XtSetArg(args[0], XtNverticalScrollPosition, 1);
    }
    setValues(args, 1);
    Application::writeApplicationProperties();

    database = XrmGetDatabase(XtDisplay(base_widget));
    XrmPutLineResource(&database, line);

    vector<Component *> *v = Application::getApplication()->getComponents();
    for(int i = 0; i < (int)v->size(); i++) {
        AxesClass *a = v->at(i)->getAxesClassInstance();
        if(a) a->setValues(args, 1);
    }
    delete v;
}
