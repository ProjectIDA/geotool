/** \file AmplitudeParams.cpp
 *  \brief Defines class AmplitudeParams.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

using namespace std;

#include "AmplitudeParams.h"
#include "motif++/MotifClasses.h"
#include "widget/TabClass.h"
#include "widget/Table.h"

static int getPhases(const char *col, bool set, int *num_phases,char ***phases);

AmplitudeParams::AmplitudeParams(const string &name, Component *parent)
                        : FormDialog(name, parent)
{
    init();
    createInterface();
}

void AmplitudeParams::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
    setValues(args, n);

    setSize(650, 440);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    save_button = new Button("Save", controls, this);
    save_button->setSensitive(false);
    defaults_button = new Button("Defaults", controls, this);
    defaults_button->setSensitive(false);
    auto_measure_toggle = new Toggle("Auto Measure", controls, this);
    auto_measure_toggle->set(auto_measure);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    label = new Label("   ", controls);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    Label *lab = new Label("Amplitude Parameters", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, lab->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    tab = new TabClass("tab", this, args, n);
    tab->addActionListener(this, XtNtabCallback);

    n = 0;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNcolumns, 4); n++;
    XtSetArg(args[n], XtNvisibleRows, 23); n++;
    const char *column_labels[] = {
	"parameter", "default", "value", "description"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNrowSelectable, False); n++;
    XtSetArg(args[n], XtNcolumnSelectable, False); n++;
    a5_table = new Table("A5/2", tab, args, n);

    bool editable[4] = {false, false, true, false};
    a5_table->setColumnEditable(editable);
    int alignment[4] = {
	LEFT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY, LEFT_JUSTIFY};
    a5_table->setAlignment(4, alignment);

    a5_table->addActionListener(this, XtNmodifyVerifyCallback);
    a5_table->addActionListener(this, XtNvalueChangedCallback);
    a5_table->addActionListener(this, XtNchoiceChangedCallback);

    n = 0;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNcolumns, 4); n++;
    XtSetArg(args[n], XtNvisibleRows, 17); n++;
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNrowSelectable, False); n++;
    XtSetArg(args[n], XtNcolumnSelectable, False); n++;
    sbsnr_table = new Table("SBSNR", tab, args, n);
    tab->setOnTop("A5/2");

    sbsnr_table->setColumnEditable(editable);
    sbsnr_table->setAlignment(4, alignment);

    sbsnr_table->addActionListener(this, XtNmodifyVerifyCallback);
    sbsnr_table->addActionListener(this, XtNvalueChangedCallback);
    sbsnr_table->addActionListener(this, XtNchoiceChangedCallback);

    list();
}

void AmplitudeParams::init(void)
{
    char *prop, *c, *tok, *last;

    auto_measure = getProperty("amp_auto_measure", true);
    auto_measure_default = true;

    if(!getProperty("mb_amptype", mb_amptype)) {
	mb_amptype.assign("A5/2");
    }

    mb_amptype_default.assign("A5/2");

    mb_filter_margin = getProperty("mb_filter_margin", 10.);
    mb_filter_margin_default = 10.;

    mb_lead = getProperty("mb_lead", 0.5);
    mb_lead_default = 0.5;

    mb_length = getProperty("mb_length", 7.0);
    mb_length_default = 7.0;

    mb_taper_frac = getProperty("mb_taper_frac", 0.05);
    mb_taper_frac_default = 0.05;

    if(!getProperty("mb_filter_type", mb_filter_type)) {
	mb_filter_type.assign("BP");
    }
    mb_filter_type_default.assign("BP");

    mb_filter_locut = getProperty("mb_filter_locut", 0.8);
    mb_filter_locut_default = 0.8;

    mb_filter_hicut = getProperty("mb_filter_hicut", 4.5);
    mb_filter_hicut_default = 4.5;

    mb_filter_order = getProperty("mb_filter_order", 3);
    mb_filter_order_default = 3;

    mb_filter_zp = getProperty("mb_filter_zp", true);
    mb_filter_zp_default = true;

    mb_amp_threshold1 = getProperty("mb_amp_threshold1", 15.0);
    mb_amp_threshold1_default = 15.0;

    mb_amp_threshold2 = getProperty("mb_amp_threshold2", 25.0);
    mb_amp_threshold2_default = 25.0;

    mb_amp_threshold3 = getProperty("mb_amp_threshold3", 80.0);
    mb_amp_threshold3_default = 80.0;

    mb_amp_threshold4 = getProperty("mb_amp_threshold4", 2.0);
    mb_amp_threshold4_default = 2.0;

    mb_amp_threshold5 = getProperty("mb_amp_threshold5", 10.0);
    mb_amp_threshold5_default = 10.0;

    mb_amp_threshold6 = getProperty("mb_amp_threshold6", 97.0);
    mb_amp_threshold6_default = 97.0;

    allowed_hp_ratio = getProperty("allowed_hp_ratio", 2.0);
    allowed_hp_ratio_default = 2.0;

    allowed_lp_ratio = getProperty("allowed_lp_ratio", 1.2);
    allowed_lp_ratio_default = 1.2;

    mb_allow_counts = getProperty("mb_allow_counts", false);
    mb_allow_counts_default = false;

    if(!getProperty("mb_counts_amptype", mb_counts_amptype)) {
	mb_counts_amptype.assign("hpp_cnt");
    }
    mb_counts_amptype_default.assign("hpp_cnt");

    if((prop = getProperty("mb_phases"))) {
	mb_num_phases = 0;
	mb_phases = (char **)malloc(sizeof(char *));
	tok = prop;
	while( (c = strtok_r(tok, " ,", &last)) ) {
	    tok = NULL;
	    mb_phases = (char **)realloc(mb_phases,
				(mb_num_phases+1)*sizeof(char *));
	    mb_phases[mb_num_phases++] = strdup(c);
	}
	free(prop);
    }
    else {
	mb_num_phases = 1;
	mb_phases = (char **)malloc(mb_num_phases*sizeof(char *));
 	mb_phases[0] = strdup("P");
    }
    mb_num_phases_default = 1;
    mb_phases_default = (char **)malloc(mb_num_phases_default*sizeof(char *));
    mb_phases_default[0] = strdup("P");

    mb_dist_min = getProperty("mb_dist_min", 20.);
    mb_dist_min_default = 20.;

    mb_dist_max = getProperty("mb_dist_max", 105.);
    mb_dist_max_default = 105.;

    if(!getProperty("ml_amptype", ml_amptype)) {
	ml_amptype.assign("SBSNR");
    }
    ml_amptype_default.assign("SBSNR");

    if((prop = getProperty("ml_phases"))) {
	ml_num_phases = 0;
	ml_phases = (char **)malloc(sizeof(char *));
	tok = prop;
	while( (c = strtok_r(tok, " ,", &last)) ) {
	    tok = NULL;
	    ml_phases = (char **)realloc(ml_phases,
				(ml_num_phases+1)*sizeof(char *));
	    ml_phases[ml_num_phases++] = strdup(c);
	}
	free(prop);
    }
    else {
	ml_num_phases = 2;
	ml_phases = (char **)malloc(ml_num_phases*sizeof(char *));
 	ml_phases[0] = strdup("P");
 	ml_phases[1] = strdup("Pn");
    }
    ml_num_phases_default = 2;
    ml_phases_default = (char **)malloc(ml_num_phases_default*sizeof(char *));
    ml_phases_default[0] = strdup("P");
    ml_phases_default[1] = strdup("Pn");

    ml_dist_min = getProperty("ml_dist_min", 0.);
    ml_dist_min_default = 0.;

    ml_dist_max = getProperty("ml_dist_max", 20.);
    ml_dist_max_default = 20.;

    ml_depth_min = getProperty("ml_depth_min", 0.);
    ml_depth_min_default = 0.;

    ml_depth_max = getProperty("ml_depth_max", 40.);
    ml_depth_max_default = 40.;


    ml_sta_lead = getProperty("ml_sta_lead", 0.);
    ml_sta_lead_default = 0.;

    ml_sta_length = getProperty("ml_sta_length", 1.);
    ml_sta_length_default = 1.;

    ml_sta_window = getProperty("ml_sta_window", 4.);
    ml_sta_window_default = 4.;

    ml_lta_lead = getProperty("ml_lta_lead", 31.);
    ml_lta_lead_default = 31.;

    ml_lta_length = getProperty("ml_lta_length", 30.);
    ml_lta_length_default = 30.;

    ml_filter_margin = getProperty("ml_filter_margin", 10.);
    ml_filter_margin_default = 10.;

    if(!getProperty("ml_filter_type", ml_filter_type)) {
	ml_filter_type.assign("BP");
    }
    ml_filter_type_default.assign("BP");

    ml_filter_locut = getProperty("ml_filter_locut", 2.0);
    ml_filter_locut_default = 2.0;

    ml_filter_hicut = getProperty("ml_filter_hicut", 4.0);
    ml_filter_hicut_default = 4.0;

    ml_filter_order = getProperty("ml_filter_order", 3);
    ml_filter_order_default = 3;

    ml_filter_zp = getProperty("ml_filter_zp", true);
    ml_filter_zp_default = true;
}

AmplitudeParams::~AmplitudeParams(void)
{
    for(int i = 0; i < mb_num_phases; i++) {
	free(mb_phases[i]);
    }
    Free(mb_phases);

    for(int i = 0; i < ml_num_phases; i++) {
	free(ml_phases[i]);
    }
    Free(ml_phases);
}

void AmplitudeParams::list(void)
{
    listA5();
    listSbsnr();
}

void AmplitudeParams::listA5(void)
{
    char s[20];
    const char *row[4];
    Pixel colors[4];
    Arg args[1];
    XtSetArg(args[0], XmNbackground, &colors[0]);
    getValues(args, 1);
    colors[1] = colors[0];
    colors[2] = stringToPixel("white");
    colors[3] = colors[0];

    a5_table->removeAllRows();

    row[2] = s;

    row[0] = "mb_amptype";
    row[1] = "A5/2";
    snprintf(s, sizeof(s), "%s", mb_amptype.c_str());
    row[3] = "Amplitude measure descriptor.";
    a5_table->addRow(row, false);

    row[0] = "mb_phases";
    row[1] = "P";
    s[0] = '\0';
    for(int i = 0; i < mb_num_phases; i++) {
	int n = (int)strlen(s);
	if(i == 0) snprintf(s+n, sizeof(s)-n, "%s", mb_phases[i]);
	else snprintf(s+n, sizeof(s)-n, ",%s", mb_phases[i]);
    }
    row[3] = "Acceptable phases for mb measurement.";
    a5_table->addRow(row, false);

    row[0] = "mb_dist_min";
    row[1] = "20.0";
    snprintf(s, sizeof(s), "%.1f", mb_dist_min);
    row[3] = "Minimum allowed distance(degrees) for mb measurement.";
    a5_table->addRow(row, false);

    row[0] = "mb_dist_max";
    row[1] = "105.0";
    snprintf(s, sizeof(s), "%.1f", mb_dist_max);
    row[3] = "Maximum allowed distance(degrees) for mb measurement.";
    a5_table->addRow(row, false);

    row[0] = "mb_lead";
    row[1] = "0.5";
    snprintf(s, sizeof(s), "%.1f", mb_lead);
    row[3] = "The measurement window starts at arrival.time - mb_lead.";
    a5_table->addRow(row, false);

    row[0] = "mb_length";
    row[1] = "7.0";
    snprintf(s, sizeof(s), "%.1f", mb_length);
    row[3] = "The length of the measurement window.";
    a5_table->addRow(row, false);

    row[0] = "mb_taper_frac";
    row[1] = "0.050";
    snprintf(s, sizeof(s), "%.3f", mb_taper_frac);
    row[3] = "The fraction of the data window tapered";
    a5_table->addRow(row, false);

    row[0] = "mb_filter_margin";
    row[1] = "10.0";
    snprintf(s, sizeof(s), "%.1f", mb_filter_margin);
    row[3] = "No measurements are made within mb_filter_margin seconds of the edges of the data segment.";
    a5_table->addRow(row, false);

    row[0] = "mb_filter_type";
    row[1] = "BP";
    snprintf(s, sizeof(s), "%s", mb_filter_type.c_str());
    row[3] = "filter type";
    a5_table->addRow(row, false);
    a5_table->setCellChoice(a5_table->numRows()-1, 2, "BP:BR:LP:HP:NA");

    row[0] = "mb_filter_locut";
    row[1] = "0.80";
    snprintf(s, sizeof(s), "%.2f", mb_filter_locut);
    row[3] = "filter low cut frequency";
    a5_table->addRow(row, false);

    row[0] = "mb_filter_hicut";
    row[1] = "4.50";
    snprintf(s, sizeof(s), "%.2f", mb_filter_hicut);
    row[3] = "filter high cut frequency";
    a5_table->addRow(row, false);

    row[0] = "mb_filter_order";
    row[1] = "3";
    snprintf(s, sizeof(s), "%d", mb_filter_order);
    row[3] = "filter order";
    a5_table->addRow(row, false);
    a5_table->setCellChoice(a5_table->numRows()-1, 2, "1:2:3:4:5:6:7:8:9:10");

    row[0] = "mb_filter_zp";
    row[1] = "true";
    snprintf(s, sizeof(s), "%s", mb_filter_zp ? "true":"false");
    row[3] = "true for zero phase non-causal filter";
    a5_table->addRow(row, false);
    a5_table->setCellChoice(a5_table->numRows()-1, 2, "true:false");

    row[0] = "mb_amp_threshold1";
    row[1] = "15.0";
    snprintf(s, sizeof(s), "%.1f", mb_amp_threshold1);
    row[3] = "Percent of max signal peak to trough amplitude to use as threshold to omit small swings";
    a5_table->addRow(row, false);

    row[0] = "mb_amp_threshold2";
    row[1] = "25.0";
    snprintf(s, sizeof(s), "%.1f", mb_amp_threshold2);
    row[3] = "Value to recognize substantial side peaks.";
    a5_table->addRow(row, false);

    row[0] = "mb_amp_threshold3";
    row[1] = "80.0";
    snprintf(s, sizeof(s), "%.1f", mb_amp_threshold3);
    row[3] = "Percent of (0.0 to Nyquist frequency) band used by the digitizer";
    a5_table->addRow(row, false);

    row[0] = "mb_amp_threshold4";
    row[1] = "2.0";
    snprintf(s, sizeof(s), "%.1f", mb_amp_threshold4);
    row[3] = "Maximum ratio between found half periods allowed before hypothesis of missed peak/trough pair prevails.";
    a5_table->addRow(row, false);

    row[0] = "mb_amp_threshold5";
    row[1] = "10.0";
    snprintf(s, sizeof(s), "%.1f", mb_amp_threshold5);
    row[3] = "Maximum allowed mean value of the signal in the window compared to the found peak-trough difference (as %).";
    a5_table->addRow(row, false);

    row[0] = "mb_amp_threshold6";
    row[1] = "97.0";
    snprintf(s, sizeof(s), "%.1f", mb_amp_threshold6);
    row[3] = "In maximum peak-trough pair selection, used in selection of slightly lower amplitude.";
    a5_table->addRow(row, false);

    row[0] = "allowed_hp_ratio";
    row[1] = "2.0";
    snprintf(s, sizeof(s), "%.1f", allowed_hp_ratio);
    row[3] = "Allowed ratio between the determined signal frequency and the high cut frequency.";
    a5_table->addRow(row, false);

    row[0] = "allowed_lp_ratio";
    row[1] = "1.2";
    snprintf(s, sizeof(s), "%.1f", allowed_lp_ratio);
    row[3] = "Allowed ratio between the low cut frequency and the determined signal frequency.";
    a5_table->addRow(row, false);

    row[0] = "mb_allow_counts";
    row[1] = "false";
    snprintf(s, sizeof(s), "%s", mb_allow_counts ? "true":"false");
    row[3] = "Allow amplitude.amp to be counts when the instrument response is not found.";
    a5_table->addRow(row, false);
    a5_table->setCellChoice(a5_table->numRows()-1, 2, "true:false");

    row[0] = "mb_counts_amptype";
    row[1] = "hpp_cnt";
    snprintf(s, sizeof(s), "%s", mb_counts_amptype.c_str());
    row[3] = "Amplitude measure descriptor when units are counts.";
    a5_table->addRow(row, false);

    int num = a5_table->numRows();
    for(int i = 0; i < num; i++) {
	a5_table->fillRow(i, colors, false);
    }
    a5_table->adjustColumns();
}

void AmplitudeParams::setA5Defaults(void)
{
    const char *col[] = {
	"A5/2",
	"P",
	"20.0",
	"105.0",
	"0.5",
	"7.0",
	"0.050",
	"10.0",
	"BP",
	"0.80",
	"4.50",
	"3",
	"true",
	"15.0",
	"25.0",
	"80.0",
	"2.0",
	"10.0",
	"97.0",
	"2.0",
	"1.2",
	"false",
	"hpp_cnt"
    };
    a5_table->setColumn(2, col, 23);
    a5_table->adjustColumns();
}

void AmplitudeParams::listSbsnr(void)
{
    char s[50];
    const char *row[4];
    Pixel colors[4];
    Arg args[1];
    XtSetArg(args[0], XmNbackground, &colors[0]);
    getValues(args, 1);
    colors[1] = colors[0];
    colors[2] = stringToPixel("white");
    colors[3] = colors[0];

    sbsnr_table->removeAllRows();

    row[2] = s;

    row[0] = "ml_amptype";
    row[1] = "SBSNR";
    snprintf(s, sizeof(s), "%s", ml_amptype.c_str());
    row[3] = "Amplitude measure descriptor.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_phases";
    row[1] = "P,Pn";
    s[0] = '\0';
    for(int i = 0; i < ml_num_phases; i++) {
	int n = (int)strlen(s);
	if(i == 0) snprintf(s+n, sizeof(s)-n, "%s", ml_phases[i]);
	else snprintf(s+n, sizeof(s)-n, ",%s", ml_phases[i]);
    }
    row[3] = "Acceptable phases for ml measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_dist_min";
    row[1] = "0.0";
    snprintf(s, sizeof(s), "%.1f", ml_dist_min);
    row[3] = "Minimum allowed distance(degrees) for ml measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_dist_max";
    row[1] = "20.0";
    snprintf(s, sizeof(s), "%.1f", ml_dist_max);
    row[3] = "Maximum allowed distance(degrees) for ml measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_depth_min";
    row[1] = "0.0";
    snprintf(s, sizeof(s), "%.1f", ml_depth_min);
    row[3] = "Minimum allowed depth(km) for ml measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_depth_max";
    row[1] = "40.0";
    snprintf(s, sizeof(s), "%.1f", ml_depth_max);
    row[3] = "Maximum allowed depth(km) for ml measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_sta_lead";
    row[1] = "0.00";
    snprintf(s, sizeof(s), "%.2f", ml_sta_lead);
    row[3] ="Lead (secs) before arrival for ML short-term-average measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_sta_length";
    row[1] = "1.00";
    snprintf(s, sizeof(s), "%.2f", ml_sta_length);
    row[3] = "short-term-average measurement length (secs)";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_sta_window";
    row[1] = "4.00";
    snprintf(s, sizeof(s), "%.2f", ml_sta_window);
    row[3] = "short-term-average window length(secs): find max STA in the ml_sta_window";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_lta_lead";
    row[1] = "31.00";
    snprintf(s, sizeof(s), "%.2f", ml_lta_lead);
    row[3] = "Lead (secs) before arrival for ML long-term-average measurement.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_lta_length";
    row[1] = "30.00";
    snprintf(s, sizeof(s), "%.2f", ml_lta_length);
    row[3] = "long-term-average measurement length (secs)";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_filter_margin";
    row[1] = "10.0";
    snprintf(s, sizeof(s), "%.1f", ml_filter_margin);
    row[3] = "No measurements are made within ml_filter_margin seconds of the edges of the data segment.";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_filter_type";
    row[1] = "BP";
    snprintf(s, sizeof(s), "%s", ml_filter_type.c_str());
    row[3] = "filter type";
    sbsnr_table->addRow(row, false);
    sbsnr_table->setCellChoice(sbsnr_table->numRows()-1, 2, "BP:BR:LP:HP:NA");

    row[0] = "ml_filter_locut";
    row[1] = "2.00";
    snprintf(s, sizeof(s), "%.2f", ml_filter_locut);
    row[3] = "filter low cut frequency";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_filter_hicut";
    row[1] = "4.00";
    snprintf(s, sizeof(s), "%.2f", ml_filter_hicut);
    row[3] = "filter high cut frequency";
    sbsnr_table->addRow(row, false);

    row[0] = "ml_filter_order";
    row[1] = "3";
    snprintf(s, sizeof(s), "%d", ml_filter_order);
    row[3] = "filter order";
    sbsnr_table->addRow(row, false);
    sbsnr_table->setCellChoice(sbsnr_table->numRows()-1, 2,"1:2:3:4:5:6:7:8:9:10");

    row[0] = "ml_filter_zp";
    row[1] = "true";
    snprintf(s, sizeof(s), "%s", ml_filter_zp ? "true":"false");
    row[3] = "true for zero phase non-causal filter";
    sbsnr_table->addRow(row, false);
    sbsnr_table->setCellChoice(sbsnr_table->numRows()-1, 2, "true:false");

    int num = sbsnr_table->numRows();
    for(int i = 0; i < num; i++) {
	sbsnr_table->fillRow(i, colors, false);
    }
    sbsnr_table->adjustColumns();
}

void AmplitudeParams::setSbsnrDefaults(void)
{
    const char *col[] = {
	"SBSNR",
	"P,Pn",
	"0.0",
	"20.0",
	"0.0",
	"40.0",
	"0.00",
	"1.00",
	"4.00",
	"31.00",
	"30.00",
	"10.0",
	"BP",
	"2.00",
	"4.00",
	"3",
	"true"
    };
    sbsnr_table->setColumn(2, col, 17);
    sbsnr_table->adjustColumns();
}

void AmplitudeParams::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Save")) {
	save();
    }
    else if(comp == a5_table || comp == sbsnr_table || comp == tab) {
	setButtonsSensitive();
    }
    else if(comp == auto_measure_toggle) {
	auto_measure = auto_measure_toggle->state();
    }
    else if(!strcmp(cmd, "Defaults")) {
	if(tab->getTabOnTop() == a5_table->baseWidget()) {
	    setA5Defaults();
	}
	else {
	    setSbsnrDefaults();
	}
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Amplitude Parameters Help");
    }
}

void AmplitudeParams::save(void)
{
    bool change, a5_defaults, sbsnr_defaults;
    vector<const char *> col;

    if(!auto_measure) {
	putProperty("amp_auto_measure", "false");
    }
    else {
	putProperty("amp_auto_measure", "");
    }
    a5_table->getColumn(2, col);
    getA5Params(col, true, &change, &a5_defaults);

    sbsnr_table->getColumn(2, col);
    getSbsnrParams(col, true, &change, &sbsnr_defaults);

    save_button->setSensitive(false);

    if(tab->getTabOnTop() == a5_table->baseWidget()) {
	defaults_button->setSensitive(a5_defaults);
    }
    else {
	defaults_button->setSensitive(sbsnr_defaults);
    }
}

void AmplitudeParams::setButtonsSensitive(void)
{
    vector<const char *> col;
    bool a5_defaults, sbsnr_defaults;
    bool a5_change, sbsnr_change;

    label->setLabel("    ");

    a5_table->getColumn(2, col);
    getA5Params(col, false, &a5_change, &a5_defaults);
    sbsnr_table->getColumn(2, col);
    getSbsnrParams(col, false, &sbsnr_change, &sbsnr_defaults);

    save_button->setSensitive(a5_change || sbsnr_change);
    if(tab->getTabOnTop() == a5_table->baseWidget()) {
	defaults_button->setSensitive(a5_defaults);
    }
    else {
	defaults_button->setSensitive(sbsnr_defaults);
    }
}

bool AmplitudeParams::getA5Params(vector<const char *> &col, bool set,
		bool *change, bool *defaults)
{
    int i, ret;
    double d, dmin, dmax;
    bool b;

    *change = false;
    *defaults = false;

    if(strcasecmp(col[0], mb_amptype.c_str())) {
	*change = true;
	if(set) {
	    mb_amptype.assign(col[0]);
	    putProperty("mb_amptype", mb_amptype);
	}
    }
    if(strcasecmp(col[0], mb_amptype_default.c_str())) *defaults = true;

    ret = getPhases(col[1], set, &mb_num_phases, &mb_phases);
    if( ret < 0 ) {
	label->setLabel("Cannot interpret mb_phases");
	return false;
    }
    if(ret > 0) *change = true;
    if(mb_num_phases != mb_num_phases_default ||
	strcmp(mb_phases[0], mb_phases_default[0]))
    {
	*defaults = true;
    }

    if(!stringToDouble(col[2], &d) || d < 0.) {
	label->setLabel("Cannot interpret mb_dist_min");
	return false;
    }
    if(d != mb_dist_min) {
	*change = true;
	if(set) {
	    mb_dist_min = d;
	    putProperty("mb_dist_min", col[2]);
	}
    }
    if(d != mb_dist_min_default) *defaults = true;

    if(!stringToDouble(col[3], &d) || d <= 0.) {
	label->setLabel("Cannot interpret mb_dist_max");
	return false;
    }
    if(d != mb_dist_max) {
	*change = true;
	if(set) {
	    mb_dist_max = d;
	    putProperty("mb_dist_max",  col[3]);
	}
    }
    if(d != mb_dist_max_default) *defaults = true;

    if(!stringToDouble(col[4], &d) || d <= 0.) {
	label->setLabel("Cannot interpret mb_lead");
	return false;
    }
    if(d != mb_lead) {
	*change = true;
	if(set) {
	    mb_lead = d;
	    putProperty("mb_lead",  col[4]);
	}
    }
    if(d != mb_lead_default) *defaults = true;

    if(!stringToDouble(col[5], &d) || d < 0.) {
	label->setLabel("Cannot interpret mb_length");
	return false;
    }
    if(d != mb_length) {
	*change = true;
	if(set) {
	    mb_length = d;
	    putProperty("mb_length", col[5]);
	}
    }
    if(d != mb_length_default) *defaults = true;

    if(!stringToDouble(col[6], &d) || d <= 0.) {
	label->setLabel("Cannot interpret mb_taper_frac");
	return false;
    }
    if(d != mb_taper_frac) {
	*change = true;
	if(set) {
	    mb_taper_frac = d;
	    putProperty("mb_taper_frac", col[6]);
	}
    }
    if(d != mb_taper_frac_default) *defaults = true;

    if(!stringToDouble(col[7], &d) || d < 0.) {
	label->setLabel("Cannot interpret mb_filter_margin");
	return false;
    }
    if(d != mb_filter_margin) {
	*change = true;
	if(set) {
	    mb_filter_margin = d;
	    putProperty("mb_filter_margin", col[7]);
	}
    }
    if(d != mb_filter_margin_default) *defaults = true;

    if(strcasecmp(col[8], "BP") && strcasecmp(col[8], "BR") &&
	strcasecmp(col[8], "LP") && strcasecmp(col[8], "HP") &&
	strcasecmp(col[8], "NA"))
    {
	label->setLabel("Cannot interpret mb_filter_type");
	return false;
    }
    if(strcasecmp(col[8], mb_filter_type.c_str())) {
	*change = true;
	if(set) {
	    mb_filter_type.assign(col[8]);
	    putProperty("mb_filter_type", mb_filter_type);
	}
    }
    if(strcasecmp(col[8], mb_filter_type_default.c_str())) *defaults = true;

    if(!stringToDouble(col[9], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret mb_filter_locut");
	return false;
    }
    if(dmin != mb_filter_locut) {
	*change = true;
	if(set) {
	    mb_filter_locut = dmin;
	    putProperty("mb_filter_locut", col[9]);
	}
    }
    if(dmin != mb_filter_locut_default) *defaults = true;

    if(!stringToDouble(col[10], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret mb_filter_hicut");
	return false;
    }
    if(dmax != mb_filter_hicut) {
	*change = true;
	if(set) {
	    mb_filter_hicut = dmax;
	    putProperty("mb_filter_hicut", col[10]);
	}
    }
    if(dmax != mb_filter_hicut_default) *defaults = true;

    if(!stringToInt(col[11], &i) || i < 1 || i > 10) {
	label->setLabel("Cannot interpret mb_filter_order");
	return false;
    }
    if(i != mb_filter_order) {
	*change = true;
	if(set) {
	    mb_filter_order = i;
	    putProperty("mb_filter_order", col[11]);
	}
    }
    if(i != mb_filter_order_default) *defaults = true;

    b = !strcmp(col[12], "true");
    if(b != mb_filter_zp) {
	*change = true;
	if(set) {
	    mb_filter_zp = b;
	    putProperty("mb_filter_zp", col[12]);
	}
    }
    if(b != mb_filter_zp_default) *defaults = true;

    if(!stringToDouble(col[13], &d) || d <= 0.) {
	label->setLabel("Cannot interpret mb_amp_threshold1");
	return false;
    }
    if(d != mb_amp_threshold1) {
	*change = true;
	if(set) {
	    mb_amp_threshold1 = d;
	    putProperty("mb_amp_threshold1", col[13]);
	}
    }
    if(d != mb_amp_threshold1_default) *defaults = true;

    if(!stringToDouble(col[14], &d) || d <= 0.) {
	label->setLabel("Cannot interpret mb_amp_threshold2");
	return false;
    }
    if(d != mb_amp_threshold2) {
	*change = true;
	if(set) {
	    mb_amp_threshold2 = d;
	    putProperty("mb_amp_threshold2", col[14]);
	}
    }
    if(d != mb_amp_threshold2_default) *defaults = true;

    if(!stringToDouble(col[15], &d) || d <= 0.) {
	label->setLabel("Cannot interpret mb_amp_threshold3");
	return false;
    }
    if(d != mb_amp_threshold3) {
	*change = true;
	if(set) {
	    mb_amp_threshold3 = d;
	    putProperty("mb_amp_threshold3", col[15]);
	}
    }
    if(d != mb_amp_threshold3_default) *defaults = true;

    if(!stringToDouble(col[16], &d) || d < 0.) {
	label->setLabel("Cannot interpret mb_amp_threshold4");
	return false;
    }
    if(d != mb_amp_threshold4) {
	*change = true;
	if(set) {
	    mb_amp_threshold4 = d;
	    putProperty("mb_amp_threshold4", col[16]);
	}
    }
    if(d != mb_amp_threshold4_default) *defaults = true;

    if(!stringToDouble(col[17], &d) || d < 0.) {
	label->setLabel("Cannot interpret mb_amp_threshold5");
	return false;
    }
    if(d != mb_amp_threshold5) {
	*change = true;
	if(set) {
	    mb_amp_threshold5 = d;
	    putProperty("mb_amp_threshold5", col[17]);
	}
    }
    if(d != mb_amp_threshold5_default) *defaults = true;

    if(!stringToDouble(col[18], &d) || d < 0.) {
	label->setLabel("Cannot interpret mb_amp_threshold6");
	return false;
    }
    if(d != mb_amp_threshold6) {
	*change = true;
	if(set) {
	    mb_amp_threshold6 = d;
	    putProperty("mb_amp_threshold6", col[18]);
	}
    }
    if(d != mb_amp_threshold6_default) *defaults = true;

    if(!stringToDouble(col[19], &d) || d < 0.) {
	label->setLabel("Cannot interpret allowed_hp_ratio");
	return false;
    }
    if(d != allowed_hp_ratio) {
	*change = true;
	if(set) {
	    allowed_hp_ratio = d;
	    putProperty("allowed_hp_ratio", col[19]);
	}
    }
    if(d != allowed_hp_ratio_default) *defaults = true;

    if(!stringToDouble(col[20], &d) || d < 0.) {
	label->setLabel("Cannot interpret allowed_lp_ratio");
	return false;
    }
    if(d != allowed_lp_ratio) {
	*change = true;
	if(set) {
	    allowed_lp_ratio = d;
	    putProperty("allowed_lp_ratio", col[20]);
	}
    }
    if(d != allowed_lp_ratio_default) *defaults = true;

    b = !strcmp(col[21], "true");
    if(b != mb_allow_counts) {
	*change = true;
	if(set) {
	    mb_allow_counts = b;
	    putProperty("mb_allow_counts", col[21]);
	}
    }
    if(b != mb_allow_counts_default) *defaults = true;

    if(strcasecmp(col[22], mb_counts_amptype.c_str())) {
	*change = true;
	if(set) {
	    mb_counts_amptype.assign(col[22]);
	    putProperty("mb_counts_amptype", mb_counts_amptype);
	}
    }
    if(strcasecmp(col[22], mb_counts_amptype_default.c_str())) *defaults = true;

    return true;
}

bool AmplitudeParams::getSbsnrParams(vector<const char *> &col, bool set,
		bool *change, bool *defaults)
{
    int i, ret;
    double d, dmin, dmax;
    bool b;

    *change = false;
    *defaults = false;

    if(strcasecmp(col[0], ml_amptype.c_str())) {
	*change = true;
	if(set) {
	    ml_amptype.assign(col[0]);
	    putProperty("ml_amptype", ml_amptype);
	}
    }
    if(strcasecmp(col[0], ml_amptype_default.c_str())) *defaults = true;

    ret = getPhases(col[1], set, &ml_num_phases, &ml_phases);
    if( ret < 0 ) {
	label->setLabel("Cannot interpret ml_phases");
	return false;
    }
    if(ret > 0) *change = true;
    if(ml_num_phases != ml_num_phases_default ||
	strcmp(ml_phases[0], ml_phases_default[0]) ||
	strcmp(ml_phases[1], ml_phases_default[1])) *defaults = true;

    if(!stringToDouble(col[2], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret ml_dist_min");
	return false;
    }
    if(dmin != ml_dist_min) {
	*change = true;
	if(set) {
	    ml_dist_min = dmin;
	    putProperty("ml_dist_min", col[2]);
	}
    }
    if(dmin != ml_dist_min_default) *defaults = true;

    if(!stringToDouble(col[3], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret ml_dist_max");
	return false;
    }
    if(dmax != ml_dist_max) {
	*change = true;
	if(set) {
	    ml_dist_max = dmax;
	    putProperty("ml_dist_max", col[3]);
	}
    }
    if(dmax != ml_dist_max_default) *defaults = true;

    if(!stringToDouble(col[4], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret ml_depth_min");
	return false;
    }
    if(dmin != ml_depth_min) {
	*change = true;
	if(set) {
	    ml_depth_min = dmin;
	    putProperty("ml_depth_min", col[4]);
	}
    }
    if(dmin != ml_depth_min_default) *defaults = true;

    if(!stringToDouble(col[5], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret ml_depth_max");
	return false;
    }
    if(dmax != ml_depth_max) {
	*change = true;
	if(set) {
	    ml_depth_max = dmax;
	    putProperty("ml_depth_max", col[5]);
	}
    }
    if(dmax != ml_depth_max_default) *defaults = true;

    if(!stringToDouble(col[6], &d) || d < 0.) {
	label->setLabel("Cannot interpret ml_sta_lead");
	return false;
    }
    if(d != ml_sta_lead) {
	*change = true;
	if(set) {
	    ml_sta_lead = d;
	    putProperty("ml_sta_lead", col[6]);
	}
    }
    if(d != ml_sta_lead_default) *defaults = true;

    if(!stringToDouble(col[7], &d) || d < 0.) {
	label->setLabel("Cannot interpret ml_sta_length");
	return false;
    }
    if(d != ml_sta_length) {
	*change = true;
	if(set) {
	    ml_sta_length = d;
	    putProperty("ml_sta_length", col[7]);
	}
    }
    if(d != ml_sta_length_default) *defaults = true;

    if(!stringToDouble(col[8], &d) || d < 0.) {
	label->setLabel("Cannot interpret ml_sta_window");
	return false;
    }
    if(d != ml_sta_window) {
	*change = true;
	if(set) {
	    ml_sta_window = d;
	    putProperty("ml_sta_window", col[8]);
	}
    }
    if(d != ml_sta_window_default) *defaults = true;

    if(!stringToDouble(col[9], &d) || d < 0.) {
	label->setLabel("Cannot interpret ml_lta_lead");
	return false;
    }
    if(d != ml_lta_lead) {
	*change = true;
	if(set) {
	    ml_lta_lead = d;
	    putProperty("ml_lta_lead", col[9]);
	}
    }
    if(d != ml_lta_lead_default) *defaults = true;

    if(!stringToDouble(col[10], &d) || d < 0.) {
	label->setLabel("Cannot interpret ml_lta_length");
	return false;
    }
    if(d != ml_lta_length) {
	*change = true;
	if(set) {
	    ml_lta_length = d;
	    putProperty("ml_lta_length", col[10]);
	}
    }
    if(d != ml_lta_length_default) *defaults = true;

    if(!stringToDouble(col[11], &d) || d < 0.) {
	label->setLabel("Cannot interpret ml_filter_margin");
	return false;
    }
    if(d != ml_filter_margin) {
	*change = true;
	if(set) {
	    ml_filter_margin = d;
	    putProperty("ml_filter_margin", col[11]);
	}
    }
    if(d != ml_filter_margin_default) *defaults = true;

    if(strcasecmp(col[12], "BP") && strcasecmp(col[12], "BR") &&
	strcasecmp(col[12], "LP") && strcasecmp(col[12], "HP") &&
	strcasecmp(col[12], "NA"))
    {
	label->setLabel("Cannot interpret ml_filter_type");
	return false;
    }
    if(strcasecmp(col[12], ml_filter_type.c_str())) {
	*change = true;
	if(set) {
	    ml_filter_type.assign(col[12]);
	    putProperty("ml_filter_type", col[12]);
	}
    }
    if(strcasecmp(col[12], ml_filter_type_default.c_str())) *defaults = true;

    if(!stringToDouble(col[13], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret ml_filter_locut");
	return false;
    }
    if(dmin != ml_filter_locut) {
	*change = true;
	if(set) {
	    ml_filter_locut = dmin;
	    putProperty("ml_filter_locut", col[13]);
	}
    }
    if(dmin != ml_filter_locut_default) *defaults = true;

    if(!stringToDouble(col[14], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret ml_filter_hicut");
	return false;
    }
    if(dmax != ml_filter_hicut) {
	*change = true;
	if(set) {
	    ml_filter_hicut = dmax;
	    putProperty("ml_filter_hicut", col[14]);
	}
    }
    if(dmax != ml_filter_hicut_default) *defaults = true;

    if(!stringToInt(col[15], &i) || i < 1 || i > 10) {
	label->setLabel("Cannot interpret ml_filter_order");
	return false;
    }
    if(i != ml_filter_order) {
	*change = true;
	if(set) {
	    ml_filter_order = i;
	    putProperty("ml_filter_order", col[15]);
	}
    }
    if(i != ml_filter_order_default) *defaults = true;

    b = !strcmp(col[16], "true");
    if(b != ml_filter_zp) {
	*change = true;
	if(set) {
	    ml_filter_zp = b;
	    putProperty("ml_filter_zp", col[16]);
	}
    }
    if(b != ml_filter_zp_default)  *defaults = true;

    return true;
}

static int
getPhases(const char *col, bool set, int *num_phases, char ***phases)
{
    bool change = false;
    char **ph=NULL, *tok, *c, *last;
    int i, nphases = 0;
    char *p = strdup(col);

    if(p) {
	ph = (char **)malloc(sizeof(char *));
	tok = p;
	while( (c = strtok_r(tok, " ,", &last)) ) {
	    tok = NULL;
	    ph = (char **)realloc(ph, (nphases+1)*sizeof(char *));
	    ph[nphases++] = strdup(c);
	}
	free(p);
    }
    if( !nphases ) {
	return -1;
    }
    i = 0;
    if(nphases == *num_phases) {
	for(i = 0; i < nphases && !strcmp(ph[i], (*phases)[i]); i++);
    }
    if(nphases != *num_phases || i < nphases) {
	change = true;
	if(set) {
	    for(i = 0; i < *num_phases; i++) Free((*phases)[i]);
	    Free(*phases);
	    *phases = (char **)malloc(nphases*sizeof(char *));
	    for(i = 0; i < nphases; i++) (*phases)[i] = strdup(ph[i]);
	    *num_phases = nphases;
	}
    }
    for(i = 0; i < nphases; i++) Free(ph[i]);
    Free(ph);
    return change ? 1 : 0;
}

ParseCmd AmplitudeParams::parseCmd(const string &cmd, string &msg)
{
    char *c, *s;
    bool change, defaults;
    ParseCmd ret = COMMAND_NOT_FOUND;
    vector<char *> v;
    vector<const char *> names, col;
    int i, num = a5_table->numRows();

    a5_table->getColumn(0, names);
    a5_table->getColumn(2, col);

    for(i = 0; i < num; i++) {
        if( (c = stringGetArg(cmd.c_str(), names[i])) ) {
            v.push_back(c);
            col[i] = c;
        }
    }

    if((int)v.size() > 0) {
        if(!getA5Params(col, true, &change, &defaults)) {
            if((s = label->getLabel())) {
                msg.assign(s);
                free(s);
            }
            for(i = 0; i < (int)v.size(); i++) free(v[i]);
            return ARGUMENT_ERROR;
        }
        a5_table->setColumn(2, col);
        a5_table->adjustColumns();
	setButtonsSensitive();
        ret = COMMAND_PARSED;
    }
    for(i = 0; i < (int)v.size(); i++) free(v[i]);
    v.clear();

    if(ret != COMMAND_NOT_FOUND) return ret;

    sbsnr_table->getColumn(0, names);
    sbsnr_table->getColumn(2, col);
    num = sbsnr_table->numRows();

    for(i = 0; i < num; i++) {
        if( (c = stringGetArg(cmd.c_str(), names[i])) ) {
            v.push_back(c);
            col[i] = c;
        }
    }

    if((int)v.size() > 0) {
        if(!getSbsnrParams(col, true, &change, &defaults)) {
            if((s = label->getLabel())) {
                msg.assign(s);
                free(s);
            }
            for(i = 0; i < (int)v.size(); i++) free(v[i]);
            return ARGUMENT_ERROR;
        }
        sbsnr_table->setColumn(2, col);
        sbsnr_table->adjustColumns();
	setButtonsSensitive();
        ret = COMMAND_PARSED;
    }

    if(ret != COMMAND_NOT_FOUND) return ret;

    if(parseCompare(cmd, "defaults")) {
	setA5Defaults();
	setSbsnrDefaults();
	setButtonsSensitive();
	ret = COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "save")) {
	save();
	ret = COMMAND_PARSED;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }

    return ret;
}
