/** \file Example4.cpp
 *  \brief Defines class Example4.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>
using namespace std;

#include "Example4.h"
#include "RotateData.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GTimeSeries.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define sameSamprates(rate1,rate2) (fabs(rate1-rate2) < rate1*0.0001)

PLUGIN_NAME::PLUGIN_NAME(const char *name, Component *parent, DataSource *ds)
		: FormDialog(name, parent, false), DataReceiver(ds)
{
    createInterface();
}

void PLUGIN_NAME::createInterface(void)
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
    rotate_button = new Button("Rotate", controls, this);
    unrotate_button = new Button("Unrotate", controls, this);
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
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 2); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 2); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    rc = new RowColumn("rc", this, args, n);
    azimuth_label = new Label("Station to Source Azimuth", rc);
    incidence_label = new Label("Incidence", rc);

    azimuth_text = new TextField("azimuth", rc);
    incidence_text = new TextField("azimuth", rc);
}

PLUGIN_NAME::~PLUGIN_NAME(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void PLUGIN_NAME::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Rotate")) {
	rotate();
    }
    else if(!strcmp(cmd, "Unrotate")) {
	unrotate();
    }
    else if(!strcmp(cmd, "Help")) {
	displayHelp("Enter an azimuth value, select three-component waveforms \
and click Apply to rotate the waveforms.");
    }
}

ParseCmd PLUGIN_NAME::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseString(cmd, "azimuth", c)) {
	azimuth_text->setString(c, true);
    }
    else if(parseString(cmd, "incidence", c)) {
	incidence_text->setString(c, true);
    }
    else if(parseCompare(cmd, "rotate")) {
	rotate();
    }
    else if(parseCompare(cmd, "urotate")) {
	unrotate();
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }

    return COMMAND_PARSED;
}

ParseVar PLUGIN_NAME::parseVar(const string &name, string &value)
{
    if(parseCompare(name, "azimuth")) {
	azimuth_text->getString(value);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "incidence")) {
	incidence_text->getString(value);
	return STRING_RETURNED;
    }
    else {
	return FormDialog::parseVar(name, value);
    }
}

void PLUGIN_NAME::parseHelp(const char *prefix)
{
    printf("%sazimuth=AZIMUTH\n", prefix);
    printf("%sincidence=INCIDENCE\n", prefix);
    printf("%srotate\n", prefix);
    printf("%sunrotate\n", prefix);
}

void PLUGIN_NAME::rotate(void)
{
    int i, n;
    string errmsg;
    vector<int> ncmpts;
    double sta_to_src_azimuth, reverse_azimuth, incidence;
    gvector<Waveform *> wvec;

    if(!azimuth_text->getDouble(&sta_to_src_azimuth)) {
	return;
    }
    reverse_azimuth = sta_to_src_azimuth + 180.;
    if(reverse_azimuth > 360.) reverse_azimuth -= 360.;

    if(!incidence_text->getDouble(&incidence)) {
	return;
    }
    if(incidence < 0.) {
	incidence_text->setString("0.");
	incidence = 0.;
    }
    else if(incidence > 90.) {
	incidence_text->setString("90.");
	incidence = 90.;
    }

    if( data_source->getSelectedComponents(ncmpts, wvec) <= 0)
    {
	putWarning("No orthogonal waveform components selected.");
	return;
    }

    if( !RotateData::checkComponents(false, incidence, ncmpts, wvec, errmsg) )
    {
	if( !errmsg.empty() ) cerr << errmsg << endl;
	return;
    }

//    setCursor("hourglass");

    n = 0;
    for(i = 0; i < (int)ncmpts.size(); i++) {
	if(ncmpts[i] == 2) {
	    RotateData::rotateWaveforms(reverse_azimuth, wvec[n],
				wvec[n+1], errmsg);
	}
	else if(ncmpts[i] == 3) {
	    RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wvec[n], wvec[n+1], wvec[n+2], errmsg);
	}
	if( !errmsg.empty() ) cerr << errmsg << endl;
	n += ncmpts[i];
    }

    data_source->modifyWaveforms(wvec);

//    setCursor("default");
}

void PLUGIN_NAME::unrotate(void)
{
    vector<int> ncmpts;
    gvector<Waveform *> wvec;

    azimuth_text->setString("");
    incidence_text->setString("");

    if( data_source->getSelectedComponents(ncmpts, wvec) <= 0)
    {
	showWarning("No orthogonal waveform components selected.");
	return;
    }
    setCursor("hourglass");

    DataMethod::remove("RotateData", wvec);

    for(int i = 0; i < wvec.size(); i++) {
	wvec[i]->ts->getChan(wvec[i]->channel);
    }

    data_source->modifyWaveforms(wvec);

    setCursor("default");
}
