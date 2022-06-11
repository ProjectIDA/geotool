/** \file PrintOptions.cpp
 *  \brief Defines class PrintOptions.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sstream>

#include "widget/PrintOptions.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}

PrintOptions::PrintOptions(const char *name, Component *parent) :
		FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    fonts = new Label("Printer Fonts", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, fonts->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    rc = new RowColumn("rc", this, args, n);

    const char *label[5] = {"Axes Labels", "Tick Labels", "Tag Labels",
			"Arrival Labels", "Main Title"};

    for(int i = 0; i < 5; i++)
    {
	n = 0;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	RowColumn *form = new RowColumn("form", rc, args, n);

	n = 0;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNisAligned, True); n++;
	XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
	XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNnumColumns, 1); n++;
	RowColumn *r = new RowColumn("r", form, args, n);

	new Label(label[i], r);

	font_menu[i] = new Choice("font_menu", r, NULL, 0);
	font_menu[i]->addItem("Helvetica");
	font_menu[i]->addItem("Courier");
	font_menu[i]->addItem("Times");
	
 	n = 0;
	XtSetArg(args[n], XmNcolumns, 15); n++;
	fontText[i] = new TextField("text", form, args, n);

	size_menu[i] = new Choice("size_menu", form, NULL, 0);
	size_menu[i]->addItem("5");
	size_menu[i]->addItem("7");
	size_menu[i]->addItem("9");
	size_menu[i]->addItem("10");
	size_menu[i]->addItem("12");
	size_menu[i]->addItem("14");
	size_menu[i]->addItem("16");
	size_menu[i]->setChoice("12");

 	n = 0;
	XtSetArg(args[n], XmNcolumns, 15); n++;
	sizeText[i] = new TextField("sizeText", form, args, n);

	type_menu[i] = new Choice("type_menu", form, NULL, 0);
	type_menu[i]->addItem("Plain");
	type_menu[i]->addItem("Bold");
	type_menu[i]->addItem("Italic");
	type_menu[i]->setChoice("Bold");

 	n = 0;
	XtSetArg(args[n], XmNcolumns, 15); n++;
	typeText[i] = new TextField("typeText", form, args, n);
    }

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    rc2 = new RowColumn("rc2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc_top", rc2, args, n);

    new Label("Ink", rc);

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass); n++;
    ink_rb = new RadioBox("Ink", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_bw = new Toggle("B_&_W", ink_rb, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_color = new Toggle("Color", ink_rb, args, n);
    rb_color->set(true);

    new Space("space", rc, XmVERTICAL, 5);

    new Label("Arrival Placement", rc);

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass); n++;
    arrival_rb = new RadioBox("Arrival Placement", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_normal = new Toggle("Normal", arrival_rb, args, n);
    rb_normal->set(true);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_close = new Toggle("Close", arrival_rb, args, n);

    new Space("space", rc, XmVERTICAL, 5);

    new Label("Line Width", rc);

    line_width = new Choice("line_width", rc, NULL, 0);
    line_width->addItem("0");
    line_width->addItem("1");
    line_width->addItem("2");
    line_width->addItem("3");
    line_width->addItem("4");
    line_width->addItem("5");

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc_bot = new RowColumn("rc_bot", rc2, args, n);
    new Label("Waveform Scale", rc_bot);

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass); n++;
    scale_rb = new RadioBox("Waveform Scale", rc_bot, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_full = new Toggle("Full", scale_rb, args, n);
    rb_full->set(true);
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_half = new Toggle("Half", scale_rb, args, n);

    new Space("space", rc_bot, XmVERTICAL, 5);

    new Label("Tag Placement", rc_bot);

    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass); n++;
    tag_rb = new RadioBox("Tag Placement", rc_bot, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_standard = new Toggle("Standard", tag_rb, args, n);
    rb_standard->set(true);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    rb_left = new Toggle("Left", tag_rb, args, n);
}

PrintOptions::~PrintOptions(void)
{
}

void PrintOptions::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Print Options Help");
    }
}

ParseCmd PrintOptions::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;
    char s[100];
    const char *label[5] = {"axes_labels", "tick_labels", "tag_labels",
			"arrival_labels", "main_title"};

    for(int i = 0; i < 5; i++) {
	snprintf(s, sizeof(s), "%s_font", label[i]);
	if(parseGetArg(cmd, s, c)) {
	    if(!font_menu[i]->setChoice(c, true, true)) {
		fontText[i]->setString(c);
	    }
	}
	snprintf(s, sizeof(s), "%s_size", label[i]);
	if(parseGetArg(cmd, s, c)) {
	    if(!size_menu[i]->setChoice(c, true, true)) {
		sizeText[i]->setString(c);
	    }
	}
	snprintf(s, sizeof(s), "%s_type", label[i]);
	if(parseGetArg(cmd, s, c)) {
	    if(!type_menu[i]->setChoice(c, true, true)) {
		typeText[i]->setString(c);
	    }
	}
    }
    if(parseGetArg(cmd, "ink", c)) {
	if((ret = ink_rb->parseCmd(c, msg)) == ARGUMENT_ERROR) {
	    msg.assign("ink: expecting 'B_&_W' or 'Color'");
	    return ret;
	}
    }
    else if(parseGetArg(cmd, "arrival_placement", c)) {
	if((ret = arrival_rb->parseCmd(c, msg)) == ARGUMENT_ERROR) {
	    msg.assign("arrival_placement: expecting 'Normal' or 'Close'");
	    return ret;
	}
    }
    else if(parseGetArg(cmd, "line_width", c)) {
	if(!line_width->setChoice(c, true, true)) {
	    msg.assign("line_width: expecting 0-5");
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseGetArg(cmd, "waveform_scale", c)) {
	if((ret = scale_rb->parseCmd(c, msg)) == ARGUMENT_ERROR) {
	    msg.assign("waveform_scale: expecting 'Full' or 'Half'");
	    return ret;
	}
    }
    else if(parseGetArg(cmd, "tag_placement", c)) {
	if((ret = tag_rb->parseCmd(c, msg)) == ARGUMENT_ERROR) {
	    msg.assign("tag_placement: expecting 'Standard' or 'Left'");
	    return ret;
	}
    }
    else if(parseCompare(cmd, "Help")) {
        char prefix[200], *p;
        getParsePrefix(prefix, sizeof(prefix));
        // make "print waveforms.print options" just "print.options"
        if( (p = strstr(prefix, "print ")) ) {
	    strcpy(p, "print.options.");
        }
        parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void PrintOptions::parseHelp(const char *prefix)
{
    printf("%saxes_labels=FONT SIZE TYPE\n", prefix);
    printf("%stick_labels=FONT SIZE TYPE\n", prefix);
    printf("%stag_labels=FONT SIZE TYPE\n", prefix);
    printf("%sarrival_labels=FONT SIZE TYPE\n", prefix);
    printf("%smain_title=FONT SIZE TYPE\n", prefix);
    printf("%sink=(B_&_W,Color)\n", prefix);
    printf("%sarrival_placement=(Normal,Close)\n", prefix);
    printf("%sline_width=(0,1,2,3,4,5)\n", prefix);
    printf("%swaveform_scale=(Full,Half)\n", prefix);
    printf("%stag_placement=(Standard,Left)\n", prefix);
}

void PrintOptions::fillPrintStruct(PrintParam *p)
{
    if(!fontText[0]->getString(p->fonts.label_font,sizeof(p->fonts.label_font)))
    {
	strcpy(p->fonts.label_font, font_menu[0]->getChoice());
    }
    if( !fontText[1]->getString(p->fonts.axis_font,sizeof(p->fonts.axis_font)) )
    {
	strcpy(p->fonts.axis_font, font_menu[1]->getChoice());
    }
    if( !fontText[2]->getString(p->fonts.tag_font, sizeof(p->fonts.tag_font)) )
    {
	strcpy(p->fonts.tag_font, font_menu[2]->getChoice());
    }
    if( !fontText[3]->getString(p->fonts.arr_font, sizeof(p->fonts.arr_font)) )
    {
	strcpy(p->fonts.arr_font, font_menu[3]->getChoice());
    }
    if(!fontText[4]->getString(p->fonts.title_font,sizeof(p->fonts.title_font)))
    {
	strcpy(p->fonts.title_font, font_menu[4]->getChoice());
    }

    if( !typeText[0]->getString(p->labelsFontStyle,sizeof(p->labelsFontStyle)) )
    {
	strcpy(p->labelsFontStyle, type_menu[0]->getChoice());
    }
    if( !typeText[1]->getString(p->axesFontStyle, sizeof(p->axesFontStyle)) )
    {
	strcpy(p->axesFontStyle, type_menu[1]->getChoice());
    }
    if( !typeText[2]->getString(p->tagFontStyle, sizeof(p->tagFontStyle)) )
    {
	strcpy(p->tagFontStyle, type_menu[2]->getChoice());
    }
    if( !typeText[3]->getString(p->arrFontStyle, sizeof(p->arrFontStyle)) )
    {
	strcpy(p->arrFontStyle, type_menu[3]->getChoice());
    }
    if( !typeText[4]->getString(p->mainFontStyle, sizeof(p->mainFontStyle)) )
    {
	strcpy(p->mainFontStyle, type_menu[4]->getChoice());
    }

    if( !sizeText[0]->getInt(&p->fonts.label_fontsize) ) {
	stringToInt(size_menu[0]->getChoice(), &p->fonts.label_fontsize);
    }
    if( !sizeText[1]->getInt(&p->fonts.axis_fontsize) ) {
	stringToInt(size_menu[1]->getChoice(), &p->fonts.axis_fontsize);
    }
    if( !sizeText[2]->getInt(&p->fonts.tag_fontsize) ) {
	stringToInt(size_menu[2]->getChoice(), &p->fonts.tag_fontsize);
    }
    if( !sizeText[3]->getInt(&p->fonts.arr_fontsize) ) {
	stringToInt(size_menu[3]->getChoice(), &p->fonts.arr_fontsize);
    }
    if( !sizeText[4]->getInt(&p->fonts.title_fontsize) ) {
	stringToInt(size_menu[4]->getChoice(), &p->fonts.title_fontsize);
    }

    addFontStyle(p->fonts.title_font, p->mainFontStyle);
    addFontStyle(p->fonts.label_font, p->labelsFontStyle);
    addFontStyle(p->fonts.axis_font, p->axesFontStyle);
    addFontStyle(p->fonts.tag_font, p->tagFontStyle);
    addFontStyle(p->fonts.arr_font, p->arrFontStyle);

    p->color = rb_color->state();
    p->arrPlace = rb_normal->state();
    p->full_scale = rb_full->state();
    p->tagPlace = rb_standard->state();

    p->data_linewidth = 2;
    stringToInt(line_width->getChoice(), &p->data_linewidth);

    p->min_xlab = -1;
    p->max_xlab = -1;
    p->min_ylab = -1;
    p->max_ylab = -1;
    p->min_xsmall = -1;
    p->max_xsmall = -1;
    p->min_ysmall = -1;
    p->max_ysmall = -1;
}

void PrintOptions::addFontStyle(char *font, char *fontStyle)
{
    if(!strcmp(fontStyle, "Plain"))
    {
	if(!strcmp(font, "Times")) strcpy(font, "Times-Roman");
    }
    else if(!strcmp(fontStyle, "Bold"))
    {
	strcat(font, "-Bold");
    }
    else if(!strcmp(fontStyle, "Italic") && !strcmp(font, "Times"))
    {
	strcat(font, "-Italic");
    }
    else if(!strcmp(fontStyle, "Italic"))
    {
	strcat(font, "-Oblique");
    }
}
