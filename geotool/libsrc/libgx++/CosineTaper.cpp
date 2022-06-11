/** \file CosineTaper.cpp
 *  \brief Defines class CosineTaper.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "CosineTaper.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
}

CosineTaper::CosineTaper(const string &name, Component *parent) :
			ParamDialog(name, parent)
{
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

void CosineTaper::createInterface(void)
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
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 3); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    rc = new RowColumn("rc", this, args, n);

    lab1 = new Label("Percentage", rc);

    XmString xm = createXmString("Beginning Taper");
    n = 0;
    XtSetArg(args[n], XmNtitleString, xm); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 50); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    beg_scale = new Scale("beg_taper", rc, args, n);
    beg_scale->setValue(10);

    XmStringFree(xm);

    xm = createXmString("End Taper");
    n = 0;
    XtSetArg(args[n], XmNtitleString, xm); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 50); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    end_scale = new Scale("end_taper", rc, args, n);
    end_scale->setValue(10);

    XmStringFree(xm);

    lab2 = new Label("Or Seconds", rc);

    form1 = new Form("form1", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 12); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 12); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    beg_seconds_text = new TextField("beg_seconds", form1, this, args, n);

    form2 = new Form("form2", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 12); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 12); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    end_seconds_text = new TextField("end_seconds", form2, this, args, n);

    lab3 = new Label("Or Samples", rc);

    form3 = new Form("form3", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 12); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 12); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    beg_samples_text = new TextField("beg_samples", form3, this, args, n);

    form4 = new Form("form4", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 12); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 12); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    end_samples_text = new TextField("end_samples", form4, this, args, n);
}

CosineTaper::~CosineTaper(void)
{
}

void CosineTaper::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "beg_seconds")) {
	if( !beg_seconds_text->empty() ) {
	    beg_scale->setValue(0);
	    beg_scale->setSensitive(false);
	    beg_samples_text->setSensitive(false);
	}
	else {
	    beg_scale->setSensitive(true);
	    beg_samples_text->setSensitive(true);
	}
    }
    else if(!strcmp(cmd, "end_seconds")) {
	if( !end_seconds_text->empty() ) {
	    end_scale->setValue(0);
	    end_scale->setSensitive(false);
	    end_samples_text->setSensitive(false);
	}
	else {
	    end_scale->setSensitive(true);
	    end_samples_text->setSensitive(true);
	}
    }
    else if(!strcmp(cmd, "beg_samples")) {
	if( !beg_samples_text->empty() ) {
	    beg_scale->setValue(0);
	    beg_scale->setSensitive(false);
	    beg_seconds_text->setSensitive(false);
	}
	else {
	    beg_scale->setSensitive(true);
	    beg_seconds_text->setSensitive(true);
	}
    }
    else if(!strcmp(cmd, "end_samples")) {
	if( !end_samples_text->empty() ) {
	    end_scale->setValue(0);
	    end_scale->setSensitive(false);
	    end_seconds_text->setSensitive(false);
	}
	else {
	    end_scale->setSensitive(true);
	    end_seconds_text->setSensitive(true);
	}
    }
}

ParseCmd CosineTaper::parseCmd(const string &cmd, string &msg)
{
    string c;
    double beg, end;
    int value;

    if(parseGetArg(cmd, "beginning_taper_percentage", &beg)) {
	value = (int)beg;
	if(value < 0) value = 0;
	if(value > 50) value = 50;
	beg_scale->setValue((int)value);
    }
    else if(parseGetArg(cmd, "end_taper_percentage", &end)) {
	value = (int)end;
	if(value < 0) value = 0;
	if(value > 50) value = 50;
	end_scale->setValue(value);
    }
    else if(parseArg(cmd, "beginning_taper_seconds", c)) {
	beg_seconds_text->setString(c, true);
    }
    else if(parseArg(cmd, "end_taper_seconds", c)) {
	end_seconds_text->setString(c, true);
    }
    else if(parseArg(cmd, "beginning_taper_samples", c)) {
	beg_samples_text->setString(c, true);
    }
    else if(parseArg(cmd, "end_taper_samples", c)) {
	end_samples_text->setString(c, true);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return ParamDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void CosineTaper::parseHelp(const char *prefix)
{
    printf("%sbeginning_taper_percentage=NUM\n", prefix);
    printf("%send_taper_percentage=NUM\n", prefix);
    printf("%sbeginning_taper_seconds=NUM\n", prefix);
    printf("%send_taper_seconds=NUM\n", prefix);
    printf("%sbeginning_taper_samples=NUM\n", prefix);
    printf("%send_taper_samples=NUM\n", prefix);
}

CosineTaperUnits CosineTaper::getBegTaper(double *value)
{
    if(beg_seconds_text->getDouble(value)) {
	return SECONDS;
    }
    else if(beg_samples_text->getDouble(value)) {
	return SAMPLES;
    }
    else {
	*value = (double)beg_scale->getValue();
	return PERCENTAGE;
    }
}

CosineTaperUnits CosineTaper::getEndTaper(double *value)
{
    if(end_seconds_text->getDouble(value)) {
	return SECONDS;
    }
    else if(end_samples_text->getDouble(value)) {
	return SAMPLES;
    }
    else {
	*value = (double)end_scale->getValue();
	return PERCENTAGE;
    }
}

void CosineTaper::setBegTaper(CosineTaperUnits units, double value)
{
    if(units == SECONDS) {
	beg_seconds_text->setString("%.2lf", value);
    }
    else if(units == SAMPLES) {
	beg_samples_text->setString((int)(value+.5));
    }
    else {
	beg_scale->setValue((int)(value+.5));
    }
}

void CosineTaper::setEndTaper(CosineTaperUnits units, double value)
{
    if(units == SECONDS) {
	end_seconds_text->setString("%.2lf", value);
    }
    else if(units == SAMPLES) {
	end_samples_text->setString((int)(value+.5));
    }
    else {
	end_scale->setValue((int)(value+.5));
    }
}
