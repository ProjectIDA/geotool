/** \file FT.cpp
 *  \brief Defines class FT.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "FT.h"
#include "FtPlotClass.h"
#include "FtSmooth.h"
#include "FtWindows.h"
#include "TaperWindow.h"
#include "WaveformWindow.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"

extern "C" {
#include "libstring.h"
}

using namespace libgft;

namespace libgft {

class TaperPopup : public TaperWindow
{
    public:
	TaperPopup(const char *name, Component *parent, int width_percent,
                int min_pts, int max_pts) : TaperWindow(name, parent,
		width_percent, min_pts, max_pts)
	{
	    enableCallbackType(XmNactivateCallback);
	    apply_button = new Button("Apply", controls, this);
	}
	Button *apply_button;
};

} // namespace libgft


FT::FT(const char *name, Component *parent, DataSource *ds) :
		Frame(name, parent, true), DataReceiver(NULL)
{
    createInterface();
    init();
    setDataSource(ds);
}

void FT::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    compute_button = new Button("Compute", file_menu, this);
    cursor_menu = new Menu("Cursor", file_menu, false);
    cursor_a_toggle = new Toggle("a", cursor_menu);
    cursor_a_toggle->set(true);
    cursor_b_toggle = new Toggle("b", cursor_menu);
    cursor_b_toggle->set(true);
    cursor_c_toggle = new Toggle("c", cursor_menu);
    cursor_d_toggle = new Toggle("d", cursor_menu);
    input_button = new Button("Input...", file_menu, this);
    output_button = new Button("Output Selected...", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    new_ft_button = new Button("New FT Window", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);
    delete_button = new Button("Delete Selected", edit_menu, this);
    delete_button->setSensitive(false);
    save_button = new Button("Save Selected", edit_menu, this);
    save_button->setSensitive(false);
    smooth_button = new Button("Smooth...", edit_menu, this);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    x_axis_menu = new Menu("X Axis", view_menu, true);
    freq_toggle = new Toggle("Frequency", x_axis_menu, this, true);
    freq_toggle->setCommandString("axes 0");
    freq_toggle->set(true);
    log_freq_toggle = new Toggle("Log Frequency", x_axis_menu, this, true);
    log_freq_toggle->setCommandString("axes 1");
    period_toggle = new Toggle("Period", x_axis_menu, this, true);
    period_toggle->setCommandString("axes -1");
    log_period_toggle = new Toggle("Log Period", x_axis_menu, this, true);
    log_period_toggle->setCommandString("axes -2");

    y_axis_menu = new Menu("Y Axis", view_menu);
    amp_toggle = new Toggle("Amp", y_axis_menu, this);
    amp_toggle->setCommandString("axes 2");
    amp_toggle->set(true);
    power_toggle = new Toggle("Power", y_axis_menu, this);
    power_toggle->setCommandString("axes 3");
    y_axis_menu->addSeparator("sep");

    displacement_menu = new Menu("Displacement", y_axis_menu, false);
    disp_dB_rel_nm = new Toggle("Displacement Power (dB rel nm**2/Hz)",
			displacement_menu, this);
    disp_dB_rel_nm->setCommandString("axes 4");
    disp_dB_rel_m = new Toggle("Displacement Power (dB rel m**2/Hz)",
			displacement_menu, this);
    disp_dB_rel_m->setCommandString("axes 5");
    disp_nm = new Toggle("Displacement Power (nm**2/Hz)",
			displacement_menu, this);
    disp_nm->setCommandString("axes 6");
    disp_m = new Toggle("Displacement Power (m**2/Hz)",
			displacement_menu, this);
    disp_m->setCommandString("axes 7");
    disp_log_nm = new Toggle("Displacement Power log (nm**2/Hz)",
			displacement_menu, this);
    disp_log_nm->setCommandString("axes 8");
    disp_log_m = new Toggle("Displacement Power log (m**2/Hz)",
			displacement_menu, this);
    disp_log_m->setCommandString("axes 9");

    velocity_menu = new Menu("Velocity", y_axis_menu, false);
    vel_dB_rel_nm = new Toggle("Velocity Power (dB rel nm**2/Hz)",
			velocity_menu, this);
    vel_dB_rel_nm->setCommandString("axes 10");
    vel_dB_rel_m = new Toggle("Velocity Power (dB rel m**2/Hz)",
			velocity_menu, this);
    vel_dB_rel_m->setCommandString("axes 11");
    vel_nm = new Toggle("Velocity Power (nm**2/Hz)", velocity_menu, this);
    vel_nm->setCommandString("axes 12");
    vel_m = new Toggle("Velocity Power (m**2/Hz)", velocity_menu, this);
    vel_m->setCommandString("axes 13");
    vel_log_nm = new Toggle("Velocity Power log (nm**2/Hz)",
			velocity_menu, this);
    vel_log_nm->setCommandString("axes 14");
    vel_log_m = new Toggle("Velocity Power log (m**2/Hz)", velocity_menu, this);
    vel_log_m->setCommandString("axes 15");

    accel_menu = new Menu("Acceleration", y_axis_menu, false);
    accel_dB_rel_nm = new Toggle("Acceleration Power (dB rel nm**2/Hz)",
			accel_menu, this);
    accel_dB_rel_nm->setCommandString("axes 16");
    accel_dB_rel_m = new Toggle("Acceleration Power (dB rel m**2/Hz)",
			accel_menu, this);
    accel_dB_rel_m->setCommandString("axes 17");
    accel_nm = new Toggle("Acceleration Power (nm**2/Hz)", accel_menu, this);
    accel_nm->setCommandString("axes 18");
    accel_m = new Toggle("Acceleration Power (m**2/Hz)", accel_menu, this);
    accel_m->setCommandString("axes 19");
    accel_log_nm = new Toggle("Acceleration Power log (nm**2/Hz)",
			accel_menu, this);
    accel_log_nm->setCommandString("axes 20");
    accel_log_m = new Toggle("Acceleration Power log (m**2/Hz)",
			accel_menu, this);
    accel_log_m->setCommandString("axes 21");

    toggles[0] = freq_toggle;
    toggles[1] = log_freq_toggle;
    toggles[2] = amp_toggle;
    toggles[3] = power_toggle;
    toggles[4] = disp_dB_rel_nm;
    toggles[5] = disp_dB_rel_m;
    toggles[6] = disp_nm;
    toggles[7] = disp_m;
    toggles[8] = disp_log_nm;
    toggles[9] = disp_log_m;
    toggles[10] = vel_dB_rel_nm;
    toggles[11] = vel_dB_rel_m;
    toggles[12] = vel_nm;
    toggles[13] = vel_m;
    toggles[14] = vel_log_nm;
    toggles[15] = vel_log_m;
    toggles[16] = accel_dB_rel_nm;
    toggles[17] = accel_dB_rel_m;
    toggles[18] = accel_nm;
    toggles[19] = accel_m;
    toggles[20] = accel_log_nm;
    toggles[21] = accel_log_m;
    toggles[22] = period_toggle;
    toggles[23] = log_period_toggle;


    display_menu = new Menu("Display Data", view_menu, false);
    input_traces_toggle = new Toggle("Input Traces", display_menu, this);
    input_traces_toggle->set(true);
    mean_toggle = new Toggle("Mean", display_menu, this);
    median_toggle = new Toggle("Median", display_menu, this);
    std_dev_toggle = new Toggle("Std Dev", display_menu, this);
std_dev_toggle->setVisible(false);
    percentiles_toggle = new Toggle("Percentiles", display_menu, this);
    nlnm_toggle = new Toggle("NLNM", display_menu, this);
    nhnm_toggle = new Toggle("NHNM", display_menu, this);

    XtSetArg(args[0], XmNvisibleWhenOff, True);
    draw_dc_toggle = new Toggle("Draw DC", view_menu, this, args, 1);
    fill_button = new Button("Fill/Unfill Selected", view_menu, this);
    grid_toggle = new Toggle("Grid", view_menu, this, args, 1);
    grid_toggle->set(true);
    labels_button = new Button("Labels...", view_menu, this);
//    limits_button = new Button("Limits...", view_menu, this);
    unzoom_button = new Button("Unzoom All", view_menu, this);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    XtSetArg(args[0], XmNvisibleWhenOff, True);
    auto_cursor_toggle = new Toggle("Auto Cursor", option_menu, this, args, 1);
    rtd_compute_toggle = new Toggle("RTD Compute", option_menu, this, args, 1);
    rtd_compute_toggle->setVisible(false);

    demean_toggle = new Toggle("Demean", option_menu, this, args, 1);
    demean_toggle->set(true);

    taper_menu = new Menu("Taper", option_menu, true);
    hanning_toggle = new Toggle("Hanning", taper_menu, this);
    hanning_toggle->set(true);
    hamming_toggle = new Toggle("Hamming", taper_menu, this);
    cosine_toggle = new Toggle("Cosine", taper_menu, this);
    parzen_toggle = new Toggle("Parzen", taper_menu, this);
    welch_toggle = new Toggle("Welch", taper_menu, this);
    blackman_toggle = new Toggle("Blackman", taper_menu, this);
    none_toggle = new Toggle("None", taper_menu, this);

    taper_percent_button = new Button("Taper Percent...", option_menu, this);
    instrument_toggle = new Toggle("Instrument Corr", option_menu, this);
    windows_button = new Button("Windows...", option_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    ft_help_button = new Button("FT Help", help_menu, this);
    
    n = 0;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNdisplayGrid, True); n++;
    XtSetArg(args[n], XtNtimeScale, TIME_SCALE_SECONDS); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNwidth, 500); n++;
    XtSetArg(args[n], XtNheight, 480); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
    plot1 = new FtPlotClass("plot1", frame_form, args, n);
    plot1->setVisible(true);

    plot1->addActionListener(this, XtNselectCallback);

//    fileDialog = NULL;
//    saveDialog = NULL;
    labels_window = NULL;
    print_window = NULL;
    taper_window = new TaperPopup("FT Cosine Taper", this, 10, 5, 200);
    taper_window->apply_button->addActionListener(this);

    smoothing_window = NULL;
    windows_dialog = NULL;

    addPlugins("FT", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
	tool_bar->add(clear_button, "Clear");
	tool_bar->add(save_button, "Save");
	tool_bar->add(unzoom_button, "Unzoom All");
    }
}

void FT::init(void)
{
    string prop;

    if(freq_toggle->state())		setAxes( "0", false);
    if(log_freq_toggle->state())	setAxes( "1", false);
    if(amp_toggle->state())		setAxes( "2", false);
    if(power_toggle->state())		setAxes( "3", false);
    if(disp_dB_rel_nm->state())		setAxes( "4", false);
    if(disp_dB_rel_m->state())		setAxes( "5", false);
    if(disp_nm->state())		setAxes( "6", false);
    if(disp_m->state())			setAxes( "7", false);
    if(disp_log_nm->state())		setAxes( "8", false);
    if(disp_log_m->state())		setAxes( "9", false);
    if(vel_dB_rel_nm->state())		setAxes("10", false);
    if(vel_dB_rel_m->state())		setAxes("11", false);
    if(vel_nm->state())			setAxes("12", false);
    if(vel_m->state())			setAxes("13", false);
    if(vel_log_nm->state())		setAxes("14", false);
    if(vel_log_m->state())		setAxes("15", false);
    if(accel_dB_rel_nm->state())	setAxes("16", false);
    if(accel_dB_rel_m->state())		setAxes("17", false);
    if(accel_nm->state())		setAxes("18", false);
    if(accel_m->state())		setAxes("19", false);
    if(accel_log_nm->state())		setAxes("20", false);
    if(accel_log_m->state())		setAxes("21", false);
    if(period_toggle->state())		setAxes("-1", false);
    if(log_period_toggle->state())	setAxes("-2", false);

    if(getProperty("FT.xaxis", prop)) {
	if(!strcasecmp(prop.c_str(), "frequency")) {
	    freq_toggle->set(true, true);
	}
	else if(!strcasecmp(prop.c_str(), "logFrequency")) {
	    log_freq_toggle->set(true, true);
	}
	else if(!strcasecmp(prop.c_str(), "period")) {
	    period_toggle->set(true, true);
	}
	else if(!strcasecmp(prop.c_str(), "logPeriod")) {
	    log_period_toggle->set(true, true);
	}
    }

    if(getProperty("FT.power", false)) {
	power_toggle->set(true, true);
	amp_toggle->set(false, false);
    }
    else {
	amp_toggle->set(true, true);
	power_toggle->set(false, false);
    }
    if(getProperty("FT.Y_Axis", prop))
    {
	if(!prop.compare("Displacement.dB_rel_nm")) {
	    disp_dB_rel_nm->set(true, true);
	}
	else if(!prop.compare("Displacement.dB_rel_m")) {
	    disp_dB_rel_m->set(true, true);
        }
	else if(!prop.compare("Displacement.nm")) {
	    disp_nm->set(true, true);
        }
	else if(!prop.compare("Displacement.m")) {
	    disp_m->set(true, true);
        }
	else if(!prop.compare("Displacement.log_nm")) {
	    disp_log_nm->set(true, true);
        }
	else if(!prop.compare("Displacement.log_m")) {
	    disp_log_m->set(true, true);
        }
	else if(!prop.compare("Velocity.dB_rel_nm")) {
	    vel_dB_rel_nm->set(true, true);
	}
	else if(!prop.compare("Velocity.dB_rel_m")) {
	    vel_dB_rel_m->set(true, true);
        }
	else if(!prop.compare("Velocity.nm")) {
	    vel_nm->set(true, true);
        }
	else if(!prop.compare("Velocity.m")) {
	    vel_m->set(true, true);
        }
	else if(!prop.compare("Velocity.log_nm")) {
	    vel_log_nm->set(true, true);
        }
	else if(!prop.compare("Velocity.log_m")) {
	    vel_log_m->set(true, true);
        }
	else if(!prop.compare("Acceleration.dB_rel_nm")) {
	    accel_dB_rel_nm->set(true, true);
	}
	else if(!prop.compare("Acceleration.dB_rel_m")) {
	    accel_dB_rel_m->set(true, true);
        }
	else if(!prop.compare("Acceleration.nm")) {
	    accel_nm->set(true, true);
        }
	else if(!prop.compare("Acceleration.m")) {
	    accel_m->set(true, true);
        }
	else if(!prop.compare("Acceleration.log_nm")) {
	    accel_log_nm->set(true, true);
        }
	else if(!prop.compare("Acceleration.log_m")) {
	    accel_log_m->set(true, true);
        }
    }
    if(getProperty("FT.Taper", prop))
    {
	if(!prop.compare("Hanning")) {
	    hanning_toggle->set(true, true);
        }
	else if(!prop.compare("Hamming")) {
	    hamming_toggle->set(true, true);
        }
	else if(!prop.compare("Cosine")) {
	    cosine_toggle->set(true, true);
        }
	else if(!prop.compare("Parzen")) {
	    parzen_toggle->set(true, true);
        }
	else if(!prop.compare("Welch")) {
	    welch_toggle->set(true, true);
        }
	else if(!prop.compare("Blackman")) {
	    blackman_toggle->set(true, true);
        }
	else if(!prop.compare("None")) {
	    none_toggle->set(true, true);
        }
    }
    draw_dc_toggle->set(getProperty("FT.drawDC", false), true);
    grid_toggle->set(getProperty("FT.grid", true), true);
    instrument_toggle->set(getProperty("FT.instrument", false), true);
    demean_toggle->set(getProperty("FT.demean", true), true);

    setButtonsSensitive(false);
}

void FT::setButtonsSensitive(bool set)
{
    save_button->setSensitive(set);
    delete_button->setSensitive(set);
    output_button->setSensitive(set);
}

void FT::setAxes(const char *code, bool save)
{
    int	n, tag, mode, units, der;
    char label[80], label2[50], label3[8], label4[8];
    Arg	args[10];

    tag = atoi((char *)code);

    if(tag == 2 || tag == 3)
    {
	n = 0;
	XtSetArg(args[n], XtNunits, &units); n++;
	XtSetArg(args[n], XtNder, &der); n++;
        plot1->getValues(args, n);
	if(der == FT_DISP)
	{
	    strcpy(label, "Disp ");
	    if(tag == 2) {
		strcat(label, "Amp ");
		strcpy(label3, "");
		strcpy(label4, "**0.5");
	    }
	    else if(tag == 3) {
		strcat(label, "Power ");
		strcpy(label3, "**2");
		strcpy(label4, "");
	    }

	    if(units == FT_DB_RE_NM) {
		snprintf(label2, 50, "(dB rel nm%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_DB_RE_M) {
		snprintf(label2, 50, "(dB rel m%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_NM) {
		snprintf(label2, 50, "(nm%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_M) {
		snprintf(label2, 50, "(m%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_LOG_NM) {
		snprintf(label2, 50, "log (nm%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_LOG_M) {
		snprintf(label2, 50, "log (m%s/Hz%s)", label3, label4);
	    }
	}
	else if(der == FT_VEL)
	{
	    strcpy(label, "Vel ");
	    if(tag == 2) {
		strcat(label, "Amp ");
		strcpy(label3, "");
		strcpy(label4, "**0.5");
	    }
	    else if(tag == 3) {
		strcat(label, "Power ");
		strcpy(label3, "**2");
		strcpy(label4, "");
	    }
	    if(units == FT_DB_RE_NM) {
		snprintf(label2, 50, "(dB rel (nm/s)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_DB_RE_M) {
		snprintf(label2, 50, "(dB rel (m/s)%s/Hz%s)",label3,label4);
	    }
	    else if(units == FT_NM) {
		snprintf(label2, 50, "((nm/s)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_M) {
		snprintf(label2, 50, "((m/s)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_LOG_NM) {
		snprintf(label2, 50, "log ((nm/s)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_LOG_M) {
		snprintf(label2, 50, "log ((m/s)%s/Hz%s)", label3, label4);
	    }
	}
	else if(der == FT_ACC)
	{
	    strcpy(label, "Acc ");
	    if(tag == 2) {
		strcat(label, "Amp ");
		strcpy(label3, "");
		strcpy(label4, "**0.5");
	    }
	    else if(tag == 3) {
		strcat(label, "Power ");
		strcpy(label3, "**2");
		strcpy(label4, "");
	    }
	    if(units == FT_DB_RE_NM) {
		snprintf(label2, 50, "(dB rel (nm/s**2)%s/Hz%s)",label3,label4);
	    }
	    else if(units == FT_DB_RE_M) {
		snprintf(label2, 50, "(dB rel (m/s**2)%s/Hz%s)", label3,label4);
	    }
	    else if(units == FT_NM) {
		snprintf(label2, 50, "((nm/s**2)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_M) {
		snprintf(label2, 50, "((m/s**2)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_LOG_NM) {
		snprintf(label2, 50, "log ((nm/s**2)%s/Hz%s)", label3, label4);
	    }
	    else if(units == FT_LOG_M) {
		snprintf(label2, 50, "log ((m/s**2)%s/Hz%s)", label3, label4);
	    }
	}
	strcat(label, label2);
    }
    else if(tag > 3)
    {
	n = 0;
	XtSetArg(args[n], XtNmode, &mode); n++;
	XtSetArg(args[n], XtNder, &der); n++;
        plot1->getValues(args, n);
	if(mode == FT_AMP) {
	    strcpy(label2, "Amp");
	    strcpy(label3, "");
	    strcpy(label4, "**0.5");
	}
	else if(mode == FT_POWER) {
	    strcpy(label2, "Power");
	    strcpy(label3, "**2");
	    strcpy(label4, "");
	}
    }

    n = 0;
    switch(tag)
    {
    case -2:
	XtSetArg(args[n], XtNlogX, True); n++;
	XtSetArg(args[n], XtNxAxisPeriod, True); n++;
	XtSetArg(args[n], XtNxLabel, "Log Period"); n++;
	if(save) putProperty("FT.xaxis", "logPeriod");
	break;
    case -1:
	XtSetArg(args[n], XtNlogX, False); n++;
	XtSetArg(args[n], XtNxAxisPeriod, True); n++;
	XtSetArg(args[n], XtNxLabel, "Period (sec)"); n++;
	if(save) putProperty("FT.xaxis", "period");
	break;
    case 0:
	XtSetArg(args[n], XtNlogX, False); n++;
	XtSetArg(args[n], XtNxAxisPeriod, False); n++;
	XtSetArg(args[n], XtNxLabel, "Frequency (Hz)"); n++;
	if(save) putProperty("FT.xaxis", "frequecy");
	break;
    case 1:
	XtSetArg(args[n], XtNlogX, True); n++;
	XtSetArg(args[n], XtNxAxisPeriod, False); n++;
	XtSetArg(args[n], XtNxLabel, "Log Frequency"); n++;
	if(save) putProperty("FT.xaxis","logFrequency");
	break;
    case 2:	/* Amp */
	amp_toggle->set(true, false);
	power_toggle->set(false, false);

	/* disp */
	disp_dB_rel_nm->setLabel("Displacement Amp (dB rel nm/Hz**0.5)");
	disp_dB_rel_m->setLabel("Displacement Amp (dB rel m/Hz**0.5)");
	disp_nm->setLabel("Displacement Amp (nm/Hz**0.5)");
	disp_m->setLabel("Displacement Amp (m/Hz**0.5)");
	disp_log_nm->setLabel("Displacement Amp log (nm/Hz**0.5)");
	disp_log_m->setLabel("Displacement Amp log (m/Hz**0.5)");

	/* vel */
	vel_dB_rel_nm->setLabel("Velocity Amp (dB rel (nm/sec)/Hz**0.5)");
	vel_dB_rel_m->setLabel("Velocity Amp (dB rel (m/sec)/Hz**0.5)");
	vel_nm->setLabel("Velocity Amp ((nm/sec)/Hz**0.5)");
	vel_m->setLabel("Velocity Amp ((m/sec)/Hz**0.5)");
	vel_log_nm->setLabel("Velocity Amp log ((nm/sec)/Hz**0.5)");
	vel_log_m->setLabel("Velocity Amp log ((m/sec)/Hz**0.5)");

	/* acc */
	accel_dB_rel_nm->setLabel(
		"Acceleration Amp (dB rel (nm/sec**2)/Hz**0.5)");
	accel_dB_rel_m->setLabel(
		"Acceleration Amp (dB rel (m/sec**2)/Hz**0.5)");
	accel_nm->setLabel("Acceleration Amp ((nm/sec**2)/Hz**0.5)");
	accel_m->setLabel("Acceleration Amp ((m/sec**2)/Hz**0.5)");
	accel_log_nm->setLabel("Acceleration Amp log ((nm/sec**2)/Hz**0.5)");
	accel_log_m->setLabel("Acceleration Amp log ((m/sec**2)/Hz**0.5)");

	n = 0;
	XtSetArg(args[n], XtNmode, FT_AMP); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;
	if(save) putProperty("FT.power", "false");
	break;
    case 3:	/* Power */
	amp_toggle->set(false, false);
	power_toggle->set(true, false);

	/* disp */
	disp_dB_rel_nm->setLabel("Displacement Power (dB rel nm**2/Hz)");
	disp_dB_rel_m->setLabel("Displacement Power (dB rel m**2/Hz)");
	disp_nm->setLabel("Displacement Power (nm**2/Hz)");
	disp_m->setLabel("Displacement Power (m**2/Hz)");
	disp_log_nm->setLabel("Displacement Power log (nm**2/Hz)");
	disp_log_m->setLabel("Displacement Power log (m**2/Hz)");

	/* vel */
	vel_dB_rel_nm->setLabel("Velocity Power (dB rel (nm/sec)**2/Hz)");
	vel_dB_rel_m->setLabel("Velocity Power (dB rel (m/sec)**2/Hz)");
	vel_nm->setLabel("Velocity Power ((nm/sec)**2/Hz)");
	vel_m->setLabel("Velocity Power ((m/sec)**2/Hz)");
	vel_log_nm->setLabel("Velocity Power log ((nm/sec)**2/Hz)");
	vel_log_m->setLabel("Velocity Power log ((m/sec)**2/Hz)");

	/* acc */
	accel_dB_rel_nm->setLabel(
		"Acceleration Power (dB rel (nm/sec**2)**2/Hz)");
	accel_dB_rel_m->setLabel(
		"Acceleration Power (dB rel (m/sec**2)**2/Hz)"); 
	accel_nm->setLabel("Acceleration Power ((nm/sec**2)**2/Hz)");
	accel_m->setLabel("Acceleration Power ((m/sec**2)**2/Hz)");
	accel_log_nm->setLabel("Acceleration Power log ((nm/sec**2)**2/Hz)");
	accel_log_m->setLabel("Acceleration Power log ((m/sec**2)**2/Hz)");

	n = 0;
	XtSetArg(args[n], XtNmode, FT_POWER); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;
	if(save) putProperty("FT.power", "true");
	break;
    case 4:	/* Displacement dB_rel_nm */
	toggles[4]->set(true);
	for(int i = 5; i < 22; i++) toggles[i]->set(false);

	snprintf(label, 80, "Disp %s (dB rel nm%s/Hz%s)", label2,label3,label4);
	XtSetArg(args[n], XtNunits, FT_DB_RE_NM); n++;
	XtSetArg(args[n], XtNder, FT_DISP); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;

	if(save) putProperty("FT.Y_Axis", "Displacement.dB_rel_nm");
	break;
    case 5:	/* Displacement dB_rel_m */
	toggles[5]->set(true);
	for(int i = 4; i < 22; i++) if(i != 5) toggles[i]->set(false);

	snprintf(label, 80, "Disp %s (dB rel m%s/Hz%s)", label2, label3,label4);
	XtSetArg(args[n], XtNunits, FT_DB_RE_M); n++;
	XtSetArg(args[n], XtNder, FT_DISP); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;
	if(save) putProperty("FT.Y_Axis", "Displacement.dB_rel_m");
	break;
    case 6:	/* Displacement nm */
	toggles[6]->set(true);
	for(int i = 4; i < 22; i++) if(i != 6) toggles[i]->set(false);

	snprintf(label, 80, "Disp %s (nm%s/Hz%s)",label2,label3,label4);
	XtSetArg(args[n], XtNunits, FT_NM); n++;
	XtSetArg(args[n], XtNder, FT_DISP); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;
	if(save) putProperty("FT.Y_Axis","Displacement.nm");
	break;
    case 7:	/* Displacement m */
	toggles[7]->set(true);
	for(int i = 4; i < 22; i++) if(i != 7) toggles[i]->set(false);

	snprintf(label, 80, "Disp %s (m%s/Hz%s)", label2,label3,label4);
	XtSetArg(args[n], XtNunits, FT_M); n++;
	XtSetArg(args[n], XtNder, FT_DISP); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;
	if(save) putProperty("FT.Y_Axis", "Displacement.m");
	break;
    case 8:	/* Displacement log_nm*/
	toggles[8]->set(true);
	for(int i = 4; i < 22; i++) if(i != 8) toggles[i]->set(false);

	snprintf(label, 80, "Disp %s (log nm%s/Hz%s)", label2, label3, label4);
	XtSetArg(args[n], XtNunits, FT_LOG_NM); n++;
	XtSetArg(args[n], XtNder, FT_DISP); n++;
	XtSetArg(args[n], XtNyLabel, label); n++;
	if(save) putProperty("FT.Y_Axis", "Displacement.log_nm");
	break;
    case 9:	/* Displacement log_m */
	toggles[9]->set(true);
	for(int i = 4; i < 22; i++) if(i != 9) toggles[i]->set(false);

	snprintf(label, 80, "Disp %s (log m%s/Hz%s)", label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_LOG_M); n++;
	XtSetArg(args[n], XtNder, FT_DISP); n++;
	if(save) putProperty("FT.Y_Axis", "Displacement.log_m");
	break;
    case 10:	/* Velocity dB_rel_nm */
	toggles[10]->set(true);
	for(int i = 4; i < 22; i++) if(i != 10) toggles[i]->set(false);

	snprintf(label,80,"Vel %s (dB rel (nm/s)%s/Hz%s)",label2,label3,label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_DB_RE_NM); n++;
	XtSetArg(args[n], XtNder, FT_VEL); n++;
	if(save) putProperty("FT.Y_Axis", "Velocity.dB_rel_nm");
	break;
    case 11:	/* Velocity dB_rel_m */
	toggles[11]->set(true);
	for(int i = 4; i < 22; i++) if(i != 11) toggles[i]->set(false);

	snprintf(label, 80,"Vel %s (dB rel (m/s)%s/Hz%s)",label2,label3,label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_DB_RE_M); n++;
	XtSetArg(args[n], XtNder, FT_VEL); n++;
	if(save) putProperty("FT.Y_Axis", "Velocity.dB_rel_m");
	break;
    case 12:	/* Velocity nm */
	toggles[12]->set(true);
	for(int i = 4; i < 22; i++) if(i != 12) toggles[i]->set(false);

	snprintf(label, 80, "Vel %s ((nm/s)%s/Hz%s)", label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_NM); n++;
	XtSetArg(args[n], XtNder, FT_VEL); n++;
	if(save) putProperty("FT.Y_Axis", "Velocity.nm");
	break;
    case 13:	/* Velocity m */
	toggles[13]->set(true);
	for(int i = 4; i < 22; i++) if(i != 13) toggles[i]->set(false);

	snprintf(label, 80, "Vel %s ((m/s)%s/Hz%s)", label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_M); n++;
	XtSetArg(args[n], XtNder, FT_VEL); n++;
	if(save) putProperty("FT.Y_Axis", "Velocity.m");
	break;
    case 14:	/* Velocity log_nm*/
	toggles[14]->set(true);
	for(int i = 4; i < 22; i++) if(i != 14) toggles[i]->set(false);

	snprintf(label, 80, "Vel %s (log (nm/s)%s/Hz%s)", label2,label3,label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_LOG_NM); n++;
	XtSetArg(args[n], XtNder, FT_VEL); n++;
	if(save) putProperty("FT.Y_Axis","Velocity.log_nm");
	break;
    case 15:	/* Velocity log_m */
	toggles[15]->set(true);
	for(int i = 4; i < 22; i++) if(i != 15) toggles[i]->set(false);

	snprintf(label, 80, "Vel %s (log (m/s)%s/Hz%s)", label2, label3,label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_LOG_M); n++;
	XtSetArg(args[n], XtNder, FT_VEL); n++;
	if(save) putProperty("FT.Y_Axis", "Velocity.log_m");
	break;
    case 16:	/* Acceleration dB_rel_nm */
	toggles[16]->set(true);
	for(int i = 4; i < 22; i++) if(i != 16) toggles[i]->set(false);

	snprintf(label, 80, "Acc %s (dB rel (nm/s**2)%s/Hz%s)",
			label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_DB_RE_NM); n++;
	XtSetArg(args[n], XtNder, FT_ACC); n++;
	if(save) putProperty("FT.Y_Axis", "Acceleration.dB_rel_nm");
	break;
    case 17:	/* Acceleration dB_rel_m */
	toggles[17]->set(true);
	for(int i = 4; i < 22; i++) if(i != 17) toggles[i]->set(false);

	snprintf(label, 80, "Acc %s (dB rel (m/s**2)%s/Hz%s)",
			label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_DB_RE_M); n++;
	XtSetArg(args[n], XtNder, FT_ACC); n++;
	if(save) putProperty("FT.Y_Axis", "Acceleration.dB_rel_m");
	break;
    case 18:	/* Acceleration nm */
	toggles[18]->set(true);
	for(int i = 4; i < 22; i++) if(i != 18) toggles[i]->set(false);

	snprintf(label, 80, "Acc %s ((nm/s**2)%s/Hz%s)", label2, label3,label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_NM); n++;
	XtSetArg(args[n], XtNder, FT_ACC); n++;
	if(save) putProperty("FT.Y_Axis","Acceleration.nm");
	break;
    case 19:	/* Acceleration m */
	toggles[19]->set(true);
	for(int i = 4; i < 22; i++) if(i != 19) toggles[i]->set(false);

	snprintf(label, 80, "Acc %s ((m/s**2)%s/Hz%s)", label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_M); n++;
	XtSetArg(args[n], XtNder, FT_ACC); n++;
	if(save) putProperty("FT_Y_Axis", "Acceleration.m");
	break;
    case 20:	/* Acceleration log_nm*/
	toggles[20]->set(true);
	for(int i = 4; i < 22; i++) if(i != 20) toggles[i]->set(false);

	snprintf(label, 80, "Acc %s (log (nm/s**2)%s/Hz%s)",
			label2, label3, label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_LOG_NM); n++;
	XtSetArg(args[n], XtNder, FT_ACC); n++;
	if(save) putProperty("FT.Y_Axis", "Acceleration.log_nm");
	break;
    case 21:	/* Acceleration log_m */
	toggles[21]->set(true);
	for(int i = 4; i < 22; i++) if(i != 21) toggles[i]->set(false);

	snprintf(label, 80,"Acc %s (log (m/s**2)%s/Hz%s)",label2,label3,label4);
	XtSetArg(args[n], XtNyLabel, label); n++;
	XtSetArg(args[n], XtNunits, FT_LOG_M); n++;
	XtSetArg(args[n], XtNder, FT_ACC); n++;
	if(save) putProperty("FT.Y_Axis", "Acceleration.log_m");
	break;
    }
    plot1->setValues(args, n);

    Application::writeApplicationProperties();
}

FT::~FT(void)
{
}

void FT::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    Toggle *t = comp->getToggleInstance();
    Arg args[10];
    int n;

    if(comp == plot1) { // select callback
	setButtonsSensitive((bool)action_event->getCalldata());
    }
    else if(!strncmp(cmd, "axes ", 5)) {
	if( t && t->state() ) {
	    setAxes(cmd+5, true);
	}
    }
    else if(!strcmp(cmd, "Clear")) {
	plot1->clear();
	setButtonsSensitive(false);
    }
    else if(!strcmp(cmd, "Delete Selected")) {
	plot1->deleteSelected();
 	setButtonsSensitive((bool)plot1->numSelected());
    }
    else if(!strcmp(cmd, "Unzoom All")) {
	plot1->unzoomAll();
    }
    else if(!strcmp(cmd, "Draw DC") && t) {
	XtSetArg(args[0], XtNdrawDC, (Boolean)t->state());
	plot1->setValues(args, 1);
	if(t->state()) {
	    putProperty("FT.drawDC", "true");
	}
	else {
	    putProperty("FT.drawDC", "false");
	}
    }
    else if(!strcmp(cmd, "Grid") && t) {
	XtSetArg(args[0], XtNdisplayGrid, (Boolean)t->state());
	plot1->setValues(args, 1);
	if(t->state()) {
	    putProperty("FT.grid", "true");
	}
	else {
	    putProperty("FT.grid", "false");
	}
    }
    else if(!strcmp(cmd, "Demean") && t) {
	XtSetArg(args[0], XtNdemean,(Boolean)t->state());
	plot1->setValues(args, 1);
	if(t->state()) {
	    putProperty("FT.demean", "true");
	}
	else {
	    putProperty("FT.demean", "false");
	}
    }
    else if(!strcmp(cmd, "Instrument Corr") && t) {
	XtSetArg(args[0], XtNinstrumentCorr,(Boolean)t->state());
	plot1->setValues(args, 1);
	if(t->state()) {
	    putProperty("FT.instrument", "true");
	}
	else {
	    putProperty("FT.instrument", "false");
	}
    }
    else if(!strcmp(cmd, "Smooth...")) {
	if(!smoothing_window) {
	    smoothing_window = new FtSmooth("FT Smoothing", this);
	    smoothing_window->addActionListener(this);
	}
	double width = plot1->smoothingWidth();
	smoothing_window->setWidth(width);
	smoothing_window->setVisible(true);
    }
    else if(!strcmp(cmd, "FT Smoothing")) {
	double width = smoothing_window->getWidth();
	plot1->smooth(width);
    }
    else if(!strcmp(cmd, "Windows...")) {
	if(!windows_dialog) {
	    windows_dialog = new FtWindows("FT Windows", this);
	    windows_dialog->addActionListener(this);
	}
	windows_dialog->setVisible(true);
    }
    else if(!strcmp(cmd, "FT Windows")) {
	FtWindowsStruct *s = (FtWindowsStruct *)action_event->getCalldata();
	n = 0;
	XtSetArg(args[n], XtNwindows, s->num_windows); n++;
	XtSetArg(args[n], XtNoverlap, s->overlap); n++;
	plot1->setValues(args, n);
    }
    else if(!strcmp(cmd, "Fill/Unfill Selected")) {
	plot1->fillSelected();
    }
    else if(!strcmp(cmd, "Hanning") && t && t->state() ) {
	XtSetArg(args[0], XtNtaper, HANN_TAPER);
	plot1->setValues(args, 1);
	putProperty("FT.Taper", "Hanning");
    }
    else if(!strcmp(cmd, "Hamming") && t && t->state() ) {
	XtSetArg(args[0], XtNtaper, HAMM_TAPER);
	plot1->setValues(args, 1);
	putProperty("FT.Taper", "Hamming");
    }
    else if(!strcmp(cmd, "Cosine") && t && t->state() ) {
	double length;
	int min_pts, max_pts;
        if(!taper_window->width->getDouble(&length) || length < 0
		|| length > 50) {
	    length = 10;
	    taper_window->width->setString("10");
	}
        if(!taper_window->min_points->getInt(&min_pts) || min_pts < 0) {
	    min_pts = 5;
	    taper_window->min_points->setString("5");
	}
        if(!taper_window->max_points->getInt(&max_pts) || max_pts < 0) {
	    max_pts = 200;
	    taper_window->max_points->setString("200");
	}
	plot1->setCosineTaper(length, min_pts, max_pts);
	putProperty("FT.Taper", "Cosine");
    }
    else if(!strcmp(cmd, "Parzen") && t && t->state() ) {
	XtSetArg(args[0], XtNtaper, PARZEN_TAPER);
	plot1->setValues(args, 1);
	putProperty("FT.Taper", "Parzen");
    }
    else if(!strcmp(cmd, "Welch") && t && t->state() ) {
	XtSetArg(args[0], XtNtaper, WELCH_TAPER);
	plot1->setValues(args, 1);
	putProperty("FT.Taper", "Welch");
    }
    else if(!strcmp(cmd, "Blackman") && t && t->state() ) {
	XtSetArg(args[0], XtNtaper, BLACKMAN_TAPER);
	plot1->setValues(args, 1);
	putProperty("FT.Taper", "Blackman");
    }
    else if(!strcmp(cmd, "None") && t && t->state() ) {
	XtSetArg(args[0], XtNtaper, NO_TAPER);
	plot1->setValues(args, 1);
	putProperty("FT.Taper", "None");
    }
    else if(!strcmp(cmd, "Taper Percent...")) {
	taper_window->setVisible(true);
    }
    else if(comp == taper_window->apply_button) {
	if(!cosine_toggle->state()) {
	    cosine_toggle->set(true, true);
	}
	else {
	    double length;
	    int  min_pts, max_pts;
	    if(!taper_window->width->getDouble(&length) || length < 0
			|| length > 50) {
		showWarning("Invalid cosine taper width");
		length = 10;
	    }
	    if(!taper_window->min_points->getInt(&min_pts) || min_pts < 0) {
		min_pts = 5;
		taper_window->min_points->setString("5");
	    }
	    if(!taper_window->max_points->getInt(&max_pts) || max_pts < 0) {
		max_pts = 200;
		taper_window->max_points->setString("200");
	    }
	    plot1->setCosineTaper(length, min_pts, max_pts);
	}
    }
    else if(comp == input_traces_toggle || comp == mean_toggle ||
	comp == median_toggle || comp == std_dev_toggle ||
	comp == percentiles_toggle || comp == nlnm_toggle ||
	comp == nhnm_toggle)
    {
	n = 0;
	XtSetArg(args[n], XtNdisplayArrayTraces,
		input_traces_toggle->state()); n++;
	XtSetArg(args[n], XtNdisplayArrayMean, mean_toggle->state()); n++;
	XtSetArg(args[n], XtNdisplayArrayMedian, median_toggle->state()); n++;
	XtSetArg(args[n], XtNdisplayArrayStdDev, std_dev_toggle->state()); n++;
	XtSetArg(args[n], XtNdisplayArrayPercentile,
		percentiles_toggle->state()); n++;
	XtSetArg(args[n], XtNdisplayArrayNlnm, nlnm_toggle->state()); n++;
	XtSetArg(args[n], XtNdisplayArrayNhnm, nhnm_toggle->state()); n++;
	plot1->setValues(args, n);
    }
    else if(!strcmp(cmd, "Auto Cursor")) {
	if(wp) {
	    if(auto_cursor_toggle->state()) {
		wp->addActionListener(this, XtNdoubleLineDragCallback);
		wp->addActionListener(this, XtNdoubleLineScaleCallback);
	    }
	    else {
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
    }
    else if(!strcmp(action_event->getReason(), "RTDUpdate")) {
        if(rtd_compute_toggle->state()) {
            compute(false);
        }
    }
    else if(comp == wp) {
	const char *reason = action_event->getReason();
	if( (!strcmp(reason, XtNdoubleLineScaleCallback) ||
		!strcmp(reason, XtNdoubleLineDragCallback))
		&& auto_cursor_toggle->state())
	{
	    AxesCursorCallbackStruct *a = (AxesCursorCallbackStruct *)
			action_event->getCalldata();
	    if( (a->label[0] == 'a' && cursor_a_toggle->state()) ||
		(a->label[0] == 'b' && cursor_b_toggle->state()) ||
		(a->label[0] == 'c' && cursor_c_toggle->state()) ||
		(a->label[0] == 'd' && cursor_d_toggle->state()))
	    {
		compute();
	    }
	}
    }
    else if(!strcmp(cmd, "Compute")) {
	compute();
    }
    else if(!strcmp(cmd, "Save...")) {
/*
	char *file;

	if(fileDialog == NULL) {
	    fileDialog = new FileDialog("FKgram Save", this, FILE_ONLY, ".",
				(char *)"*.spdisc", "Save");
	}
	else {
	    fileDialog->setVisible(true);
	}
	if((file = fileDialog->getFile()) != NULL) {
	    save(file);
	    XtFree(file);
	}
	fileDialog->setVisible(false);
*/
    }
    else if(!strcmp(cmd, "Save Selected")) {
	plot1->save();
    }
    else if(!strcmp(cmd, "Labels...")) {
	if(!labels_window) {
	    labels_window = new AxesLabels("FT Labels", this, plot1);
	}
	labels_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "New FT Window"))
    {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    FT *ft = new FT(getName(), this, data_source);
	    windows.push_back(ft);
	    ft->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
	auto_cursor_toggle->set(false);
	rtd_compute_toggle->set(false);
    }
    else if(!strcmp(cmd, "FT Help")) {
	showHelp("FT Help");
    }
}

ParseCmd FT::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;
    char c[200];
    string s;

    for(int i = 0; i < 10; i++) {
	snprintf(c, sizeof(c), "%d", i+2);
	if(parseString(cmd, c, s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(s, msg);
	    }
	    for(int j = (int)windows.size(); j <= i; j++) {
		FT *ft = new FT(getName(), this, data_source);
		windows.push_back(ft);
		if(j == i) {
		    return ft->parseCmd(s, msg);
		}
	    }
	}
    }

    if(parseArg(cmd, "X_Axis", s) ) {
	if(parseCompare(s, "freq")) {
	    freq_toggle->set(true, true);
	}
	else if(!sameName("log freq", s, 8)) {
	    log_freq_toggle->set(true, true);
	}
	else {
	    msg.assign(string("ft.x_axis: unknown setting: ") + s);
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "Y_Axis", s) ) {
	ret = parseYAxis(s, msg);
    }
    else if(parseArg(cmd, "Display_Data", s)) {
	ret = parseDisplayData(s, msg);
    }
    else if(parseCompare(cmd, "Clear")) {
	plot1->clear();
	setButtonsSensitive(false);
    }
    else if(parseCompare(cmd, "Delete_Selected")) {
	plot1->deleteSelected();
 	setButtonsSensitive((bool)plot1->numSelected());
    }
    else if(parseCompare(cmd, "Unzoom_All")) {
	plot1->unzoomAll();
    }
    else if(parseArg(cmd, "Draw_DC", s)) {
	ret = draw_dc_toggle->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Grid", s)) {
	ret = grid_toggle->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Demean", s)) {
	ret = demean_toggle->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Instrument", s) )
    {
	ret = instrument_toggle->parseCmd(s, msg);
    }
    else if(parseString(cmd, "Smooth", s)) {
	if(!smoothing_window) {
	    smoothing_window = new FtSmooth("FT Smoothing", this);
	    smoothing_window->addActionListener(this);
	}
	ret = smoothing_window->parseCmd(s, msg);
    }
    else if(parseString(cmd, "Windows", s)) {
	if(!windows_dialog) {
	    windows_dialog = new FtWindows("FT Windows", this);
	    windows_dialog->addActionListener(this);
	}
	ret = windows_dialog->parseCmd(s, msg);
    }
    else if(parseCompare(cmd, "Fill_Selected") ||
		parseCompare(cmd, "Unfill Selected"))
    {
	plot1->fillSelected();
    }
    else if(parseArg(cmd, "Taper", s)) {
	ret = parseTaper(s, msg);
    }
    else if(parseCompare(cmd, "Compute")) {
	compute();
    }
    else if(parseCompare(cmd, "Save")) {
    }
    else if(parseString(cmd, "Labels", s)) {
	if(!labels_window) {
	    labels_window = new AxesLabels("FT Labels", this, plot1);
	}
	ret = labels_window->parseCmd(s, msg);
    }
    else if(parseString(cmd, "print_window", s)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print FT", this, this);
	}
	ret = print_window->parseCmd(s, msg);
    }
    else if(parseCompare(cmd, "Close")) {
	setVisible(false);
	auto_cursor_toggle->set(false);
	rtd_compute_toggle->set(false);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	ret = Frame::parseCmd(cmd, msg);
    }
    return ret;
}

ParseCmd FT::parseYAxis(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "Amp_", 4)) {
	amp_toggle->set(true, true);
	power_toggle->set(false, false);
	c = cmd.substr(4);
    }
    else if(parseCompare(cmd, "Power_", 6)) {
	power_toggle->set(true, true);
	amp_toggle->set(false, false);
	c = cmd.substr(6);
    }
    else {
	return ARGUMENT_ERROR;
    }
    if(parseCompare(c, "Displacement_dB_rel_nm")) {
	disp_dB_rel_nm->set(true, true);
    }
    else if(parseCompare(c, "Displacement_dB_rel_m")) {
	disp_dB_rel_m->set(true, true);
    }
    else if(parseCompare(c, "Displacement_nm")) {
	disp_nm->set(true, true);
    }
    else if(parseCompare(c, "Displacement_m")) {
	disp_m->set(true, true);
    }
    else if(parseCompare(c, "Displacement_log_nm")) {
	disp_log_nm->set(true, true);
    }
    else if(parseCompare(c, "Displacement_log_m")) {
	disp_log_m->set(true, true);
    }
    else if(parseCompare(c, "Velocity_dB_rel_nm")) {
	vel_dB_rel_nm->set(true, true);
    }
    else if(parseCompare(c, "Velocity_dB_rel_m")) {
	vel_dB_rel_m->set(true, true);
    }
    else if(parseCompare(c, "Velocity_nm")) {
	vel_nm->set(true, true);
    }
    else if(parseCompare(c, "Velocity_m")) {
	vel_m->set(true, true);
    }
    else if(parseCompare(c, "Velocity_log_nm")) {
	vel_log_nm->set(true, true);
    }
    else if(parseCompare(c, "Velocity_log_m")) {
	vel_log_m->set(true, true);
    }
    else if(parseCompare(c, "Acceleration_dB_rel_nm")) {
	accel_dB_rel_nm->set(true, true);
    }
    else if(parseCompare(c, "Acceleration_dB_rel_m")) {
	accel_dB_rel_m->set(true, true);
    }
    else if(parseCompare(c, "Acceleration_nm")) {
	accel_nm->set(true, true);
    }
    else if(parseCompare(c, "Acceleration_m")) {
	accel_m->set(true, true);
    }
    else if(parseCompare(c, "Acceleration_log_nm")) {
	accel_log_nm->set(true, true);
    }
    else if(parseCompare(c, "Acceleration_log_m")) {
	accel_log_m->set(true, true);
    }
    else {
	return ARGUMENT_ERROR;
    }
    return COMMAND_PARSED;
}

ParseCmd FT::parseDisplayData(const string &cmd, string &msg)
{
    ParseCmd ret = ARGUMENT_ERROR;
    char *tok, *c, *last;
    char *s = strdup(cmd.c_str());

    input_traces_toggle->set(false, true);
    mean_toggle->set(false, true);
    median_toggle->set(false, true);
    std_dev_toggle->set(false, true);
    percentiles_toggle->set(false, true);
    nlnm_toggle->set(false, true);
    nhnm_toggle->set(false, true);

    tok = s;
    while((c = strtok_r(tok, ",", &last))) {
	tok = NULL;
	if(parseCompare(cmd,"input traces") || parseCompare(cmd,"input_traces"))
	{
	    input_traces_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else if(parseCompare(cmd, "mean")) {
	    mean_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else if(parseCompare(cmd, "median")) {
	    median_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else if(parseCompare(cmd, "std dev") || parseCompare(cmd, "std_dev")) {
	    std_dev_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else if(parseCompare(cmd, "percentiles")) {
	    percentiles_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else if(parseCompare(cmd, "nlnm")) {
	    nlnm_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else if(parseCompare(cmd, "nhnm")) {
	    nhnm_toggle->set(true, true);
	    ret = COMMAND_PARSED;
	}
	else {
	    msg.assign(string("FT.display_data: unknown mode: ") + c);
	    ret = ARGUMENT_ERROR;
	    break;
	}
    }
    Free(s);
    return ret;
}

ParseCmd FT::parseTaper(const string &c, string &msg)
{
    if(parseCompare(c, "Hanning")) {
	hanning_toggle->set(true, true);
    }
    else if(parseCompare(c, "Hamming")) {
	hamming_toggle->set(true, true);
    }
    else if(parseCompare(c, "Cosine")) {
	cosine_toggle->set(true, true);
    }
    else if(parseCompare(c, "Parzen")) {
	parzen_toggle->set(true, true);
    }
    else if(parseCompare(c, "Welch")) {
	welch_toggle->set(true, true);
    }
    else if(parseCompare(c, "Blackman")) {
	blackman_toggle->set(true, true);
    }
    else if(parseCompare(c, "None")) {
	none_toggle->set(true, true);
    }
    else {
	msg.assign(string("FT.taper: unknown taper type: ") + c);
	return ARGUMENT_ERROR;
    }
    return COMMAND_PARSED;
}
    

ParseVar FT::parseVar(const string &name, string &value)
{
    ParseVar ret;
    FtPlotData *fts=NULL;
    int i, nextc, num = plot1->numEntries();

    if(parseArrayIndex(name, "entry", num, &i, &nextc, value, &ret)) {
	num = plot1->getData(&fts, false);

	const char *c = name.c_str() + nextc;
	if( parseArray(c, ".pow", fts[i].nf, fts[i].dB, value,
		&ret) ) { Free(fts); return ret; }
	else if( parseArray(c, ".phase", fts[i].nf, fts[i].phase, value,
		&ret) ) { Free(fts); return ret; }
	else if( parseArray(c, ".data", fts[i].npts, fts[i].t, value,
		&ret) ) { Free(fts); return ret; }

	else if(!strcasecmp(c, ".sta")) {
	    value.assign(fts[i].sta);
	}
	else if(!strcasecmp(c, ".chan")) {
	    value.assign(fts[i].chan);
	}
	else if(!strcasecmp(c, ".time")) {
	    parsePrintDouble(value, fts[i].time);
	}
	else if(!strcasecmp(c, ".nf")) {
	    parsePrintInt(value, fts[i].nf);
	}
	else if(!strcasecmp(c, ".npts")) {
	    parsePrintInt(value, fts[i].npts);
	}
	else if(!strcasecmp(c, ".taper")) {
	    value.assign(fts[i].taper);
	}
	else if(!strcasecmp(c, ".tlen")) {
	    parsePrintDouble(value, fts[i].tlen);
	}
	else if(!strcasecmp(c, ".df")) {
	    parsePrintDouble(value, fts[i].df);
	}
	else {
	    Free(fts);
	    return VARIABLE_NOT_FOUND;
	}
	Free(fts);
	return STRING_RETURNED;
    }
    return VARIABLE_NOT_FOUND;
}

void FT::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scompute\n", prefix);
    printf("%sdelete_selected\n", prefix);
    printf("%sdemean=(true,false)\n", prefix);
    printf("%sdisplay_data=\
(Input Traces,Mean,Median,Std Dev,Percentiles,NLNM,NHNM)\n", prefix);
    printf("%sdraw_dc=(true,false)\n", prefix);
    printf("%sfill_selected\n", prefix);
    printf("%sgrid=(true,false)\n", prefix);
    printf("%sinstrument=(true,false)\n", prefix);

    char p[200];
    snprintf(p, sizeof(p), "%slabels.", prefix);
    AxesLabels::parseHelp(p);

    printf("%ssmooth.width=WIDTH\n", prefix);
    printf("%ssmooth.apply\n", prefix);

    printf("%sx_axis=(freq,log freq)\n", prefix);
    printf("%sy_axis=(\n", prefix);
	printf("\tAmp_Displacement_dB_rel_nm\n");;
	printf("\tAmp_Displacement_dB_rel_m\n");
	printf("\tAmp_Displacement_nm\n");
	printf("\tAmp_Displacement_m\n");
	printf("\tAmp_Displacement_log_nm\n");
	printf("\tAmp_Displacement_log_m\n");
	printf("\tAmp_Velocity_dB_rel_nm\n");
	printf("\tAmp_Velocity_dB_rel_m\n");
	printf("\tAmp_Velocity_nm\n");
	printf("\tAmp_Velocity_m\n");
	printf("\tAmp_Velocity_log_nm\n");
	printf("\tAmp_Velocity_log_m\n");
	printf("\tAmp_Acceleration_dB_rel_nm\n");
	printf("\tAmp_Acceleration_dB_rel_m\n");
	printf("\tAmp_Acceleration_nm\n");
	printf("\tAmp_Acceleration_m\n");
	printf("\tAmp_Acceleration_log_nm\n");
	printf("\tAmp_Acceleration_log_m\n");
	printf("\tPower_Displacement_dB_rel_nm\n");
	printf("\tPower_Displacement_dB_rel_m\n");
	printf("\tPower_Displacement_nm\n");
	printf("\tPower_Displacement_m\n");
	printf("\tPower_Displacement_log_nm\n");
	printf("\tPower_Displacement_log_m\n");
	printf("\tPower_Velocity_dB_rel_nm\n");
	printf("\tPower_Velocity_dB_rel_m\n");
	printf("\tPower_Velocity_nm\n");
	printf("\tPower_Velocity_m\n");
	printf("\tPower_Velocity_log_nm\n");
	printf("\tPower_Velocity_log_m\n");
	printf("\tPower_Acceleration_dB_rel_nm\n");
	printf("\tPower_Acceleration_dB_rel_m\n");
	printf("\tPower_Acceleration_nm\n");
	printf("\tPower_Acceleration_m\n");
	printf("\tPower_Acceleration_log_nm\n");
	printf("\tPower_Acceleration_log_m\n");
	printf("\t)\n");

    printf("%swindows.number=NUM\n", prefix);
    printf("%swindows.overlay=NUM\n", prefix);
    printf("%swindows.apply=NUM\n", prefix);

    printf("%staper=(Hanning,Hamming,Cosine,Parzen,Welch,Blackman,None)\n",
		prefix);
    printf("%sunfill_selected\n", prefix);
    printf("%sunzoom_all\n", prefix);

    printf("%sprint.help\n", prefix);
}

void FT::print(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print FT", this, this);
    }
    print_window->setVisible(true);
}

void FT::print(FILE *fp, PrintParam *p)
{
    AxesParm *a = plot1->hardCopy(fp, p, NULL);
    Free(a);
}

void FT::compute(bool warning)
{
    int i, j, k, npts, total, num_waveforms;
    int pts, minpts;
    double dt, time;
    gvector<Waveform *> wvec;
    const char *double_lines = "abcd";
    int num_double_lines = 4;
    Toggle *cursor_toggles[4];

    plot1->clearTmps();

    if(!data_source) return;

    setCursor("hourglass");

    num_waveforms = data_source->getSelectedWaveforms(wvec);

    cursor_toggles[0] = cursor_a_toggle;
    cursor_toggles[1] = cursor_b_toggle;
    cursor_toggles[2] = cursor_c_toggle;
    cursor_toggles[3] = cursor_d_toggle;

    pts = -1;
    minpts = -1;
    for(i = total = 0; i < num_waveforms; i++)
    {
	for(j = 0; j < num_double_lines; j++) if(cursor_toggles[j]->state())
	{
	    GDataPoint *d1, *d2;

	    for(k = 0; k < wvec[i]->num_dw; k++)
		if(wvec[i]->dw[k].label == double_lines[j])
	    {
		break;
	    }
	    if(k == wvec[i]->num_dw) continue;

	    d1 = wvec[i]->dw[k].d1;
	    d2 = wvec[i]->dw[k].d2;
		
	    if(d1->segmentIndex() != d2->segmentIndex()) continue;

	    total++;

	    npts = d2->index() - d1->index() + 1;

	    if(pts == -1) {
		pts = npts;
		minpts = pts;
	    }
	    else if(abs(npts-pts) > 2) {
		minpts = -1;
		break;
	    }
	    else if(npts < minpts) {
		minpts = npts;
	    }
	}
    }

    if(total > 0 && minpts)
    {
	for(i = 0; i < num_waveforms; i++)
	{
	    for(j = 0; j < num_double_lines; j++) if(cursor_toggles[j]->state())
	    {
		GDataPoint *d1, *d2;

		for(k = 0; k < wvec[i]->num_dw; k++)
		    if(wvec[i]->dw[k].label == double_lines[j])
		{
		    break;
		}
		if(k == wvec[i]->num_dw) continue;

		d1 = wvec[i]->dw[k].d1;
		d2 = wvec[i]->dw[k].d2;
		
		if(d1->segmentIndex() != d2->segmentIndex()) continue;

		dt = d1->segment()->tdel();
	
		npts = (minpts > 0) ?  minpts : d2->index() - d1->index() + 1;

		time = d1->time();

		plot1->input(wvec[i]->sta(), wvec[i]->chan(), npts,
		    d1->segment()->data+d1->index(), dt, wvec[i]->fg,
		    wvec[i]->ts, time);
	    }
	}
	plot1->compute();
	setCursor("default");
	return;
    }

    for(j = 0; j < num_double_lines; j++)
    {
	char c[2];
	c[0] = double_lines[j];
	c[1] = '\0';
	if(cursor_toggles[j]->state() && data_source->dataWindowIsDisplayed(c))
	{
	    break;
	}
    }
    if(j == num_double_lines)
    {
	total = num_waveforms;
	for(i = 0; i < num_waveforms; i++)
	{
	    if(wvec[i]->size() > 1)
	    {
		showWarning("%s/%s: data gap.", wvec[i]->sta(),
			wvec[i]->chan());
		continue;
	    }
			
	    dt = wvec[i]->segment(0)->tdel();
	
	    npts = wvec[i]->length();

	    time = wvec[i]->tbeg();

	    plot1->input(wvec[i]->sta(), wvec[i]->chan(), npts,
		    wvec[i]->segment(0)->data, dt,
		    wvec[i]->fg, wvec[i]->ts, time);
	}
    }
    plot1->compute();
    setCursor("default");

    if(num_waveforms <= 0)
    {
	if(warning) showWarning("No selected waveforms.");
	return;
    }
}

void FT::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
		wp->removeActionListener(this, "RTDUpdate");
	    }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp) {
		if(auto_cursor_toggle->state()) {
		    wp->addActionListener(this, XtNdoubleLineDragCallback);
		    wp->addActionListener(this, XtNdoubleLineScaleCallback);
		}
		if(wp->isRealTime()) {
		    wp->addActionListener(this, "RTDUpdate");
		    rtd_compute_toggle->setVisible(true);
		}
	    }
	}
	else wp = NULL;
    }
}
