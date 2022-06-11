/** \file ArrivalParams.cpp
 *  \brief Defines class ArrivalParams.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

using namespace std;

#include "ArrivalParams.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"

ArrivalParams::ArrivalParams(const string &name, Component *parent)
                        : FormDialog(name, parent)
{
    init();
    createInterface();
    initBands();
}

void ArrivalParams::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
    setValues(args, n);

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
    label = new Label("", controls);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmVERTICAL); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 7); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    single_band_toggle = new Toggle("Single Band", rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    search_bands_toggle = new Toggle("Search Bands", rc, this, args, n);

    n = 0;
    single_fmin_label = new Label("fmin", rc);
    search_fmin_label = new Label("fmin", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    single_fmin_text = new TextField("fmin", rc, this, args, n);
    search_fmin_text = new TextField("fmin", rc, this, args, n);

    single_fmax_label = new Label("fmax", rc);
    search_bandw_label = new Label("bandwidth", rc);

    single_fmax_text = new TextField("fmax", rc, this, args, n);
    search_bandw_text = new TextField("bandwidth", rc, this, args, n);

    new Space("space", rc, XmHORIZONTAL, 10);
    search_fmax_label = new Label("fmax", rc);

    new Space("space", rc, XmHORIZONTAL, 10);
    search_fmax_text = new TextField("fmax", rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    label2 = new Label("Arrival FK Frequency Band", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, label2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNcolumns, 4); n++;
    XtSetArg(args[n], XtNvisibleRows, 27); n++;
    XtSetArg(args[n], XtNwidth, 650); n++;
    const char *column_labels[] = {
	"parameter", "default", "value", "description"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNrowSelectable, False); n++;
    XtSetArg(args[n], XtNcolumnSelectable, False); n++;
    XtSetArg(args[n], XtNtableTitle, "Arrival Parameters"); n++;
    table = new Table("table", this, args, n);

    bool editable[4] = {false, false, true, false};
    table->setColumnEditable(editable);
    int alignment[4] = {
	LEFT_JUSTIFY, RIGHT_JUSTIFY, RIGHT_JUSTIFY, LEFT_JUSTIFY};
    table->setAlignment(4, alignment);

    table->addActionListener(this, XtNmodifyVerifyCallback);
    table->addActionListener(this, XtNvalueChangedCallback);
    table->addActionListener(this, XtNchoiceChangedCallback);

    list();
}

void ArrivalParams::init(void)
{
    string prop;

    stav_len = getProperty("stav_len", 1.);
    stav_len_default = 1.0;

    ltav_len = getProperty("ltav_len", 60.);
    ltav_len_default = 60.0;

    min_snr = getProperty("min_snr", 4.);
    min_snr_default = 4.0;

    max_snr = getProperty("max_snr", 18.);
    max_snr_default = 18.0;

    min_deltim = getProperty("min_deltim", .685);
    min_deltim_default = 0.685;

    max_deltim = getProperty("max_deltim", 1.72);
    max_deltim_default = 1.72;

    fk_lead = getProperty("fk_lead", 4.4);
    fk_lead_default = 4.4;

    fk_lag = getProperty("fk_lag", 6.4);
    fk_lag_default = 6.4;

    fk_dk = getProperty("fk_dk", 0.017);
    fk_dk_default = 0.017;

    signal_slow_min = getProperty("signal_slow_min", 0.0);
    signal_slow_min_default = 0.0;

    signal_slow_max = getProperty("signal_slow_max", .36);
    signal_slow_max_default = 0.36;

    signal_az_min = getProperty("signal_az_min", 0.0);
    signal_az_min_default = 0.0;

    signal_az_max = getProperty("signal_az_max", 360.0);
    signal_az_max_default = 360.0;

    fk_taper_type = COSINE_TAPER;
    fk_taper_type_default = COSINE_TAPER;
    if(getProperty("fk_taper_type", prop)) {
	if(!strncasecmp(prop.c_str(), "hann", 4)) {
	    fk_taper_type = HANN_TAPER;
	}
	else if(!strncasecmp(prop.c_str(), "hamm", 4)) {
	    fk_taper_type = HAMM_TAPER;
	}
	else if(!strncasecmp(prop.c_str(), "cosine", 6)) {
	    fk_taper_type = COSINE_TAPER;
	}
	else if(!strncasecmp(prop.c_str(), "parzen", 6)) {
	    fk_taper_type = PARZEN_TAPER;
	}
	else if(!strncasecmp(prop.c_str(), "welch", 5)) {
	    fk_taper_type = WELCH_TAPER;
	}
	else if(!strncasecmp(prop.c_str(), "blackman", 8)) {
	    fk_taper_type = BLACKMAN_TAPER;
	}
	else if(!strncasecmp(prop.c_str(), "no", 2)) {
	    fk_taper_type = NO_TAPER;
	}
    }

    fk_taper_frac = getProperty("fk_taper_frac", .05);
    fk_taper_frac_default = 0.05;

    polar_signal_lead = getProperty("polar_signal_lead", 1.5);
    polar_signal_lead_default = 1.5;

    polar_signal_len = getProperty("polar_signal_len", 5.5);
    polar_signal_len_default = 5.5;

    polar_window = getProperty("polar_window", 1.5);
    polar_window_default = 1.5;

    polar_alpha = getProperty("polar_alpha", 0.2965);
    polar_alpha_default = 0.2965;

    polar_dk = getProperty("polar_dk", 0.10);
    polar_dk_default = 0.10;

    polar_zp = getProperty("polar_zp", false);
    polar_zp_default = false;

    polar_lofreq = getProperty("polar_lofreq", 2.0);
    polar_lofreq_default = 2.0;

    polar_hifreq = getProperty("polar_hifreq", 4.0);
    polar_hifreq_default = 4.0;

    polar_order = getProperty("polar_order", 3);
    polar_order_default = 3;

    polar_taper_frac = getProperty("polar_taper_frac", 0.05);
    polar_taper_frac_default = 0.05;

    if(!getProperty("polar_filter_type", polar_filter_type)) {
        polar_filter_type.assign("BP");
    }
    polar_filter_type_default.assign("BP");

    if(!getProperty("arrival_beam_recipe", arrival_beam_recipe)) {
        arrival_beam_recipe.assign("cb");
    }
    arrival_beam_recipe_default.assign("cb");
}

void ArrivalParams::initBands(void)
{
    string prop;
    single_fmin = getProperty("fk_single_fmin", 0.5);
    single_fmin_text->setString("%.1f", single_fmin);
    single_fmin_default = 0.5;

    single_fmax = getProperty("fk_single_fmax", 3.0);
    single_fmax_text->setString("%.1f", single_fmax);
    single_fmax_default = 3.0;

    search_fmin = getProperty("fk_search_fmin", 0.0);
    search_fmin_text->setString("%.1f", search_fmin);
    search_fmin_default = 0.0;

    search_fmax = getProperty("fk_search_fmax", 6.0);
    search_fmax_text->setString("%.1f", search_fmax);
    search_fmax_default = 6.0;

    search_bandw = getProperty("fk_search_bandw", 2.0);
    search_bandw_text->setString("%.1f", search_bandw);
    search_bandw_default = 2.0;

    if(getProperty("fk_frequency_band", prop)) {
	if(!strncasecmp(prop.c_str(), "single", 6)) {
	    search_bands = false;
	    single_band_toggle->set(true, true);
	}
	else {
	    search_bands = true;
	    search_bands_toggle->set(true, true);
	}
    }
    else {
	search_bands = false;
	single_band_toggle->set(true, true);
    }
    search_bands_default = false;
}

ArrivalParams::~ArrivalParams(void)
{
}

void ArrivalParams::list(void)
{
    char s[20];
    const char *row[4];
    const char *tapers[] = {"hanning", "hamming", "cosine", "parzen",
			"welch", "blackman", "none"};
    Pixel colors[4];
    Arg args[1];
    XtSetArg(args[0], XmNbackground, &colors[0]);
    getValues(args, 1);
    colors[1] = colors[0];
    colors[2] = stringToPixel("white");
    colors[3] = colors[0];

    table->removeAllRows();

    row[2] = s;

    row[0] = "stav_len";
    row[1] = "1.00";
    snprintf(s, sizeof(s), "%.2f", stav_len);
    row[3] = "the short term average window length (secs)";
    table->addRow(row, false);

    row[0] = "ltav_len";
    row[1] = "60.00";
    snprintf(s, sizeof(s), "%.2f", ltav_len);
    row[3] = "the long term average window length (secs)";
    table->addRow(row, false);

    row[0] = "min_snr";
    row[1] = "4.00";
    snprintf(s, sizeof(s), "%.2f", min_snr);
    row[3] = "the minimum signal to noise ratio";
    table->addRow(row, false);

    row[0] = "max_snr";
    row[1] = "18.00";
    snprintf(s, sizeof(s), "%.2f", max_snr);
    row[3] = "the maximum signal to noise ratio";
    table->addRow(row, false);

    row[0] = "min_deltim";
    row[1] = "0.685";
    snprintf(s, sizeof(s), "%.3f", min_deltim);
    row[3] = "the minimum deltim value";
    table->addRow(row, false);

    row[0] = "max_deltim";
    row[1] = "1.720";
    snprintf(s, sizeof(s), "%.3f", max_deltim);
    row[3] = "the maximum deltim value";
    table->addRow(row, false);


    row[0] = "fk_lead";
    row[1] = "4.40";
    snprintf(s, sizeof(s), "%.2f", fk_lead);
    row[3] = "the FK window starts at arrival.time - fk_lead secs";
    table->addRow(row, false);

    row[0] = "fk_lag";
    row[1] = "6.40";
    snprintf(s, sizeof(s), "%.2f", fk_lag);
    row[3] = "the FK window ends at arrival.time + fk_lag secs";
    table->addRow(row, false);

    row[0] = "fk_dk";
    row[1] = "0.017";
    snprintf(s, sizeof(s), "%.3f", fk_dk);
    row[3] = "FK delslo = fk_dk/(sqrt(fstat)*central_frequency)";
    table->addRow(row, false);

    row[0] = "signal_slow_min";
    row[1] = "0.00";
    snprintf(s, sizeof(s), "%.2f", signal_slow_min);
    row[3] = "the minimum slowness considered for fstat";
    table->addRow(row, false);

    row[0] = "signal_slow_max";
    row[1] = "0.36";
    snprintf(s, sizeof(s), "%.2f", signal_slow_max);
    row[3] = "the maximum slowness considered for fstat";
    table->addRow(row, false);

    row[0] = "signal_az_min";
    row[1] = "0.00";
    snprintf(s, sizeof(s), "%.2f", signal_az_min);
    row[3] = "the minimum azimuth considered for fstat";
    table->addRow(row, false);

    row[0] = "signal_az_max";
    row[1] = "360.00";
    snprintf(s, sizeof(s), "%.2f", signal_az_max);
    row[3] = "the maximum azimuth considered for fstat";
    table->addRow(row, false);

    row[0] = "fk_taper_type";
    row[1] = "cosine";
    snprintf(s, sizeof(s), "%s", tapers[fk_taper_type]);
    row[3] = "the taper type";
    table->addRow(row, false);
    table->setCellChoice(table->numRows()-1, 2,
		"hanning:hamming:cosine:parzen:welch:blackman:none");

    row[0] = "fk_taper_frac";
    row[1] = ".05";
    snprintf(s, sizeof(s), "%.3f", fk_taper_frac);
    row[3] = "the fraction of the data window tapered (Cosine taper only)";
    table->addRow(row, false);

    row[0] = "polar_signal_lead";
    row[1] = "1.5";
    snprintf(s, sizeof(s), "%.1f", polar_signal_lead);
    row[3] = "3c polarization starts at arrival.time - polar_signal_lead";
    table->addRow(row, false);

    row[0] = "polar_signal_len";
    row[1] = "5.5";
    snprintf(s, sizeof(s), "%.1f", polar_signal_len);
    row[3] = "The length of the 3c polarization window.";
    table->addRow(row, false);

    row[0] = "polar_window";
    row[1] = "1.5";
    snprintf(s, sizeof(s), "%.1f", polar_window);
    row[3] = "3c polarization analysis window length.";
    table->addRow(row, false);

    row[0] = "polar_alpha";
    row[1] = "0.2965";
    snprintf(s, sizeof(s), "%.4f", polar_alpha);
    row[3] = "3c slowness = polar_alpha * sin(arrival.ema/2)";
    table->addRow(row, false);

    row[0] = "polar_dk";
    row[1] = "0.100";
    snprintf(s, sizeof(s), "%.3f", polar_dk);
    row[3] = "3c delslo = sqrt(0.5*polar_dk*polar_dk*(1-rectilinearity))";
    table->addRow(row, false);

    row[0] = "polar_zp";
    row[1] = "false";
    snprintf(s, sizeof(s), "%s", polar_zp ? "true" : "false");
    row[3] = "polar filter zero-phase";
    table->addRow(row, false);
    table->setCellChoice(table->numRows()-1, 2, "true:false");

    row[0] = "polar_lofreq";
    row[1] = "2.00";
    snprintf(s, sizeof(s), "%.2f", polar_lofreq);
    row[3] = "polar filter low-cut frequency.";
    table->addRow(row, false);

    row[0] = "polar_hifreq";
    row[1] = "4.00";
    snprintf(s, sizeof(s), "%.2f", polar_hifreq);
    row[3] = "polar filter high-cut frequency.";
    table->addRow(row, false);

    row[0] = "polar_order";
    row[1] = "3";
    snprintf(s, sizeof(s), "%d", polar_order);
    row[3] = "polar filter order.";
    table->addRow(row, false);
    table->setCellChoice(table->numRows()-1, 2, "1:2:3:4:5:6:7:8:9:10");

    row[0] = "polar_taper_frac";
    row[1] = "0.050";
    snprintf(s, sizeof(s), "%.3f", polar_taper_frac);
    row[3] = "The fraction of the data window tapered for the filter.";
    table->addRow(row, false);

    row[0] = "polar_filter_type";
    row[1] = "BP";
    snprintf(s, sizeof(s), "%s", polar_filter_type.c_str());
    row[3] = "The filter type (BP, BR, LP, HP, NA).";
    table->addRow(row, false);
    table->setCellChoice(table->numRows()-1, 2, "BP:BR:LP:HP:NA");

    row[0] = "arrival_beam_recipe";
    row[1] = "cb";
    snprintf(s, sizeof(s), "%s", arrival_beam_recipe.c_str());
    row[3] = "The recipe that specifies elements for array measurements.";
    table->addRow(row, false);

    int num = table->numRows();
    for(int i = 0; i < num; i++) {
	table->fillRow(i, colors, false);
    }
    table->adjustColumns();
}

void ArrivalParams::setDefaults(void)
{
    char s[20];
    const char *col[] = {
	"1.00",
	"60.00",
	"4.00",
	"18.00",
	"0.685",
	"1.720",
	"4.40",
	"6.40",
	"0.017",
	"0.00",
	"0.36",
	"0.00",
	"360.00",
	"cosine",
	".05",
	"1.5",
	"5.5",
	"1.5",
	"0.2965",
	"0.100",
	"false",
	"2.00",
	"4.00",
	"3",
	"0.050",
	"BP",
	"cb"
    };
    table->setColumn(2, col, 27);
    table->adjustColumns();

    single_band_toggle->set(true, true);

    snprintf(s, sizeof(s), "%.1f", single_fmin_default);
    single_fmin_text->setString(s);
    snprintf(s, sizeof(s), "%.1f", single_fmax_default);
    single_fmax_text->setString(s);

    snprintf(s, sizeof(s), "%.1f", search_fmin_default);
    search_fmin_text->setString(s);
    snprintf(s, sizeof(s), "%.1f", search_fmax_default);
    search_fmax_text->setString(s);
    snprintf(s, sizeof(s), "%.1f", search_bandw_default);
    search_bandw_text->setString(s);
}

void ArrivalParams::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Save")) {
	save();
    }
    else if(!strcmp(cmd, "Defaults")) {
	setDefaults();
	setButtonsSensitive();
    }
    else if(comp == single_band_toggle) {
	if(single_band_toggle->state()) {
	    single_fmin_text->setSensitive(true);
	    single_fmax_text->setSensitive(true);
	    single_fmin_label->setSensitive(true);
	    single_fmax_label->setSensitive(true);

	    search_bands_toggle->set(false);
	    search_fmin_text->setSensitive(false);
	    search_bandw_text->setSensitive(false);
	    search_fmax_text->setSensitive(false);
	    search_fmin_label->setSensitive(false);
	    search_bandw_label->setSensitive(false);
	    search_fmax_label->setSensitive(false);
	    setButtonsSensitive();
	}
    }
    else if(comp == search_bands_toggle) {
	if(search_bands_toggle->state()) {
	    search_fmin_text->setSensitive(true);
	    search_bandw_text->setSensitive(true);
	    search_fmax_text->setSensitive(true);
	    search_fmin_label->setSensitive(true);
	    search_bandw_label->setSensitive(true);
	    search_fmax_label->setSensitive(true);

	    single_band_toggle->set(false);
	    single_fmin_text->setSensitive(false);
	    single_fmax_text->setSensitive(false);
	    single_fmin_label->setSensitive(false);
	    single_fmax_label->setSensitive(false);
	    setButtonsSensitive();
	}
    }
    else if(comp == table || comp == search_bandw_text
	|| comp == single_fmin_text || comp == single_fmax_text
	|| comp == search_fmin_text || comp == search_fmax_text)
    {
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Arrival Parameters Help");
    }
}

void ArrivalParams::save(void)
{
    bool change, defaults;
    vector<const char *> col;

    table->getColumn(2, col);
    getParams(col, true, &change, &defaults);
    save_button->setSensitive(false);
    defaults_button->setSensitive(defaults);
}

void ArrivalParams::setButtonsSensitive(void)
{
    bool change, defaults;
    vector<const char *> col;

    table->getColumn(2, col);
    label->setLabel("");
    getParams(col, false, &change, &defaults);
    save_button->setSensitive(change);
    defaults_button->setSensitive(defaults);
}

bool ArrivalParams::getParams(vector<const char *> &col, bool set, bool *change,
			bool *defaults)
{
    const char *tapers[] = {"hanning", "hamming", "cosine", "parzen",
			"welch", "blackman", "none"};
    string c;
    int i, taper_type;
    double d, dmin, dmax;
    bool b;

    *change = false;
    *defaults = false;

    if(!stringToDouble(col[0], &d) || d <= 0.) {
	label->setLabel("Cannot interpret stav_len");
	return false;
    }
    if(d != stav_len) {
	*change = true;
	if(set) { stav_len = d; putProperty("stav_len", col[0]); }
    }
    if(d != stav_len_default) *defaults = true;

    if(!stringToDouble(col[1], &d) || d <= 0. || d <= stav_len) {
	label->setLabel("Cannot interpret ltav_len");
	return false;
    }
    if(d != ltav_len) {
	*change = true;
	if(set) { ltav_len = d; putProperty("ltav_len",  col[1]); }
    }
    if(d != ltav_len_default) *defaults = true;

    if(!stringToDouble(col[2], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret min_snr");
	return false;
    }
    if(dmin != min_snr) {
	*change = true;
	if(set) { min_snr = dmin; putProperty("min_snr", col[2]); }
    }
    if(dmin != min_snr_default) *defaults = true;

    if(!stringToDouble(col[3], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret max_snr");
	return false;
    }
    if(dmax != max_snr) {
	*change = true;
	if(set) { max_snr = dmax; putProperty("max_snr", col[3]); }
    }
    if(dmax != max_snr_default) *defaults = true;

    if(!stringToDouble(col[4], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret min_deltim");
	return false;
    }
    if(dmin != min_deltim) {
	*change = true;
	if(set) { min_deltim = dmin; putProperty("min_deltim", col[4]); }
    }
    if(dmin != min_deltim_default) *defaults = true;

    if(!stringToDouble(col[5], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret max_deltim");
	return false;
    }
    if(dmax != max_deltim) {
	*change = true;
	if(set) { max_deltim = dmax; putProperty("max_deltim", col[5]); }
    }
    if(dmax != max_deltim_default) *defaults = true;

    if(!stringToDouble(col[6], &d) || d < 0.) {
	label->setLabel("Cannot interpret fk_lead");
	return false;
    }
    if(d != fk_lead) { *change = true;
	if(set) { fk_lead = d; putProperty("fk_lead", col[6]); }
    }
    if(d != fk_lead_default) *defaults = true;

    if(!stringToDouble(col[7], &d) || d < 0.) {
	label->setLabel("Cannot interpret fk_lag");
	return false;
    }
    if(d != fk_lag) {
	*change = true;
	if(set) { fk_lag = d; putProperty("fk_lag", col[7]); }
    }
    if(d != fk_lag_default) *defaults = true;

    if(!stringToDouble(col[8], &d) || d <= 0.) {
	label->setLabel("Cannot interpret fk_dk");
	return false;
    }
    if(d != fk_dk) {
	*change = true;
	if(set) { fk_dk = d; putProperty("fk_dk", col[8]); }
    }
    if(d != fk_dk_default) *defaults = true;

    if(!stringToDouble(col[9], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret signal_slow_min");
	return false;
    }
    if(dmin != signal_slow_min) {
	*change = true;
	if(set) {
	    signal_slow_min = dmin;
	    putProperty("signal_slow_min", col[9]);
	}
    }
    if(dmin != signal_slow_min_default) *defaults = true;

    if(!stringToDouble(col[10], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret signal_slow_max");
	return false;
    }
    if(dmax != signal_slow_max) {
	*change = true;
	if(set) {
	    signal_slow_max = dmax;
	    putProperty("signal_slow_max", col[10]);
	}
    }
    if(dmax != signal_slow_max_default) *defaults = true;

    if(!stringToDouble(col[11], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret signal_az_min");
	return false;
    }
    if(dmin != signal_az_min) {
	*change = true;
	if(set) {
	    signal_az_min = dmin;
	    putProperty("signal_az_min", col[11]);
	}
    }
    if(dmin != signal_az_min_default) *defaults = true;

    if(!stringToDouble(col[12], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret signal_az_max");
	return false;
    }
    if(dmax != signal_az_max) {
	*change = true;
	if(set) {
	    signal_az_max = dmax;
	    putProperty("signal_az_max", col[12]);
	}
    }
    if(dmax != signal_az_max_default) *defaults = true;

    if(	    !strcasecmp(col[13], "hanning"))	taper_type = HANN_TAPER;
    else if(!strcasecmp(col[13], "hamming"))	taper_type = HAMM_TAPER;
    else if(!strcasecmp(col[13], "cosine"))	taper_type = COSINE_TAPER;
    else if(!strcasecmp(col[13], "parzen"))	taper_type = PARZEN_TAPER;
    else if(!strcasecmp(col[13], "welch"))	taper_type = WELCH_TAPER;
    else if(!strcasecmp(col[13], "blackman"))	taper_type = BLACKMAN_TAPER;
    else if(!strcasecmp(col[13], "none"))	taper_type = NO_TAPER;
    else {
	label->setLabel("Cannot interpret fk_taper_type");
	return false;
    }
    if(taper_type != fk_taper_type) {
	*change = true;
	if(set) {
	    fk_taper_type = taper_type;
	    putProperty("fk_taper_type", tapers[fk_taper_type]);
	}
    }
    if(taper_type != fk_taper_type_default) *defaults = true;

    if(!stringToDouble(col[14], &d) || d < 0. || d > .50) {
	label->setLabel("Cannot interpret fk_taper_frac");
	return false;
    }
    if(d != fk_taper_frac) {
	*change = true;
	if(set) {
	    fk_taper_frac = d; putProperty("fk_taper_frac", col[14]);
	}
    }
    if(d != fk_taper_frac_default) *defaults = true;
    if(!stringToDouble(col[15], &d) || d < 0.) {
	label->setLabel("Cannot interpret polar_signal_lead");
	return false;
    }
    if(d != polar_signal_lead) {
	*change = true;
	if(set) {
	    polar_signal_lead = d;
	    putProperty("polar_signal_lead", col[15]);
	}
    }
    if(d != polar_signal_lead_default) *defaults = true;

    if(!stringToDouble(col[16], &d) || d <= 0.) {
	label->setLabel("Cannot interpret polar_signal_len");
	return false;
    }
    if(d != polar_signal_len) {
	*change = true;
	if(set) {
	    polar_signal_len = d;
	    putProperty("polar_signal_len", col[16]);
	}
    }
    if(d != polar_signal_len_default) *defaults = true;

    if(!stringToDouble(col[17], &d) || d <= 0.) {
	label->setLabel("Cannot interpret polar_window");
	return false;
    }
    if(d != polar_window) {
	*change = true;
	if(set) {
	    polar_window = d;
	    putProperty("polar_window", col[17]);
	}
    }
    if(d != polar_window_default) *defaults = true;

    if(!stringToDouble(col[18], &d) || d <= 0.) {
	label->setLabel("Cannot interpret polar_alpha");
	return false;
    }
    if(d != polar_alpha) {
	*change = true;
	if(set) {
	    polar_alpha = d;
	    putProperty("polar_alpha", col[18]);
	}
    }
    if(d != polar_alpha_default) *defaults = true;

    if(!stringToDouble(col[19], &d) || d <= 0.) {
	label->setLabel("Cannot interpret polar_dk");
	return false;
    }
    if(d != polar_dk) {
	*change = true;
	if(set) {
	    polar_dk = d;
	    putProperty("polar_dk", col[19]);
	}
    }
    if(d != polar_dk_default) *defaults = true;

    b = !strcmp(col[20], "true");
    if(b != polar_zp) {
	*change = true;
	if(set) {
	    polar_zp = b;
	    putProperty("polar_zp", col[20]);
	}
    }
    if(b != polar_zp_default) *defaults = true;

    if(!stringToDouble(col[21], &dmin) || dmin < 0.) {
	label->setLabel("Cannot interpret polar_lofreq");
	return false;
    }
    if(dmin != polar_lofreq) {
	*change = true;
	if(set) {
	    polar_lofreq = dmin;
	    putProperty("polar_lofreq", col[21]);
	}
    }
    if(dmin != polar_lofreq_default) *defaults = true;

    if(!stringToDouble(col[22], &dmax) || dmax < 0. || dmax <= dmin) {
	label->setLabel("Cannot interpret polar_hifreq");
	return false;
    }
    if(dmax != polar_hifreq) {
	*change = true;
	if(set) {
	    polar_hifreq = dmax;
	    putProperty("polar_hifreq", col[22]);
	}
    }
    if(dmax != polar_hifreq_default) *defaults = true;

    if(!stringToInt(col[23], &i) || i < 1 || i > 10) {
	label->setLabel("Cannot interpret polar_order");
	return false;
    }
    if(i != polar_order) {
	*change = true;
	if(set) {
	    polar_order = i;
	    putProperty("polar_order", col[23]);
	}
    }
    if(i != polar_order_default) *defaults = true;

    if(!stringToDouble(col[24], &d) || d <= 0.) {
	label->setLabel("Cannot interpret polar_taper_frac");
	return false;
    }
    if(d != polar_taper_frac) {
	*change = true;
	if(set) {
	    polar_taper_frac = d;
	    putProperty("polar_taper_frac", col[24]);
	}
    }
    if(d != polar_taper_frac_default) *defaults = true;

    if(strcasecmp(col[25], "BP") && strcasecmp(col[25], "BR") &&
	strcasecmp(col[25], "LP") && strcasecmp(col[25], "HP") &&
	strcasecmp(col[25], "NA"))
    {
	label->setLabel("Cannot interpret polar_filter_type");
	return false;
    }
    if(strcasecmp(col[25], polar_filter_type.c_str())) {
	*change = true;
	if(set) {
	    polar_filter_type.assign(col[25]);
	    putProperty("polar_filter_type", polar_filter_type);
	}
    }
    if(strcasecmp(col[25], polar_filter_type_default.c_str())) *defaults = true;

    if(strcasecmp(col[26], arrival_beam_recipe.c_str())) {
	*change = true;
	if(set) {
	    arrival_beam_recipe.assign(col[26]);
	    putProperty("arrival_beam_recipe", arrival_beam_recipe);
	}
    }
    if(strcasecmp(col[26], arrival_beam_recipe_default.c_str())) *defaults = true;

    if(single_band_toggle->state()) {
	if(search_bands) {
	    *change = true;
	    if(set) {
		search_bands = false;
		putProperty("fk_frequency_band", "single");
	    }
	}
	if(!single_fmin_text->getDouble(&d) || d < 0.) {
	    label->setLabel("Cannot interpret single band fmin");
	    return false;
	}
	if(d != single_fmin) {
	    *change = true;
	    if(set) {
		single_fmin = d;
		single_fmin_text->getString(c);
		putProperty("fk_single_fmin", c);
	    }
	}
	if(d != single_fmin_default) *defaults = true;

	if(!single_fmax_text->getDouble(&d) || d <= 0.) {
	    label->setLabel("Cannot interpret single band fmax");
	    return false;
	}
	if(d != single_fmax) {
	    *change = true;
	    if(set) {
		single_fmax = d;
		single_fmax_text->getString(c);
		putProperty("fk_single_fmax", c);
	    }
	}
	if(d != single_fmax_default) *defaults = true;
    }
    else {
	*defaults = true;
	if(!search_bands) {
	    *change = true;
	    if(set) {
		search_bands = true;
		putProperty("fk_frequency_band", "search");
	    }
	}
	if(!search_fmin_text->getDouble(&d) || d < 0.) {
	    label->setLabel("Cannot interpret search bands fmin");
	    return false;
	}
	if(d != search_fmin) {
	    *change = true;
	    if(set) {
		search_fmin = d;
		search_fmin_text->getString(c);
		putProperty("fk_search_fmin", c);
	    }
	}
	if(!search_fmax_text->getDouble(&d) || d < 0.) {
	    label->setLabel("Cannot interpret search bands fmax");
	    return false;
	}
	if(d != search_fmax) {
	    *change = true;
	    if(set) {
		search_fmax = d;
		search_fmax_text->getString(c);
		putProperty("fk_search_fmax", c);
	    }
	}
	if(!search_bandw_text->getDouble(&d) || d < 0.) {
	    label->setLabel("Cannot interpret search bands bandw");
	    return false;
	}
	if(d != search_bandw) {
	    *change = true;
	    if(set) {
		search_bandw = d;
		search_bandw_text->getString(c);
		putProperty("fk_search_bandw", c);
	    }
	}
    }

    return true;
}

ParseCmd ArrivalParams::parseCmd(const string &cmd, string &msg)
{
    char *c;
    string s;
    bool change, defaults;
    ParseCmd ret = COMMAND_NOT_FOUND;
    vector<char *> v;
    vector<const char *> names, col;
    int i;

    table->getColumn(0, names);
    table->getColumn(2, col);

    for(i = 0; i < (int)names.size(); i++) {
	if( (c = stringGetArg(cmd.c_str(), names[i])) ) {
	    v.push_back(c);
	    col[i] = c;
	}
    }

    if((int)v.size() > 0) {
	if(!getParams(col, true, &change, &defaults)) {
	    label->getLabel(s);
	    msg.assign(s);
	    for(i = 0; i < (int)v.size(); i++) free(v[i]);
	    return ARGUMENT_ERROR;
	}
	table->setColumn(2, col);
	table->adjustColumns();
	setButtonsSensitive();
	ret = COMMAND_PARSED;
    }
    for(i = 0; i < (int)v.size(); i++) free(v[i]);

    if(ret != COMMAND_NOT_FOUND) return ret;

    if(parseCompare(cmd, "defaults")) {
	setDefaults();
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
