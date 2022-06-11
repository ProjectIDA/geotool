/** \file FK.cpp
 *  \brief Defines class FK.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
#include <math.h>
#include <sys/types.h>
#include <pwd.h>
using namespace std;

#include "motif++/MotifClasses.h"
#include "FK.h"
#include "FKData.h"
#include "FKParam.h"
#include "FKGram.h"
#include "FKGram3C.h"
#include "FkSignal.h"
#include "FkParamDialog.h"
#include "TaperWindow.h"
#include "tapers.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"
#include "libgx++.h"
#include "Demean.h"
#include "TaperData.h"
#include "IIRFilter.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}
#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

using namespace libgfk;

static FK *computing_fk=NULL;

static bool get_resources = true;

typedef struct
{
    Pixel	azimuth_color;
    int		fk_units;
} SubData, *SubDataPtr;

static SubData  sub_data;

#define XtNazimuthColor	"azimuthColor"
#define XtNfkUnits	"fkUnits"

#define XtCAzimuthColor	"AzimuthColor"
#define XtCFkUnits	"FkUnits"

#define offset(field)	XtOffset(SubDataPtr, field)
static XtResource	resources[] =
{
    {(char *)XtNazimuthColor, (char *)XtCAzimuthColor, XtRPixel, sizeof(Pixel),
	offset(azimuth_color), XtRString, (XtPointer)"red"},
    {(char *)XtNfkUnits, (char *)XtCFkUnits, XtRInt, sizeof(int),
	offset(fk_units), XtRImmediate, (XtPointer)FK_SEC_PER_DEG},
};
#undef offset

namespace libgfk {

class BeamWaveformView : public WaveformView
{
    public:
        BeamWaveformView(const char *name, Component *parent, FK *Fk,
		InfoArea *ia, Arg *args, int n) :
		WaveformView(name, parent, ia, args, n)
        {
	    fk = Fk;
	    for(int i = 0; i < 4; i++) {
		azimuth[i] = -1.;
		slow[i] = -1.;
		fmin[i] = -1.;
		fmax[i] = -1.;
		fstat[i] = -1.;
	    }
	}
	FK *fk;
	double azimuth[4];
	double slow[4];
	double fmin[4];
	double fmax[4];
	double fstat[4];

    protected:
	CssArrivalClass * addArrivalFromKey(CPlotArrivalCallbackStruct *p)
	{
	    Password password = getpwuid(getuid());
	    bool made_it=false;
	    CssArrivalClass *arrival=NULL;
	    gvector<Waveform *> ws;
	    double cfreq, deltim = -1., delaz = -1., delslo = -1.;
	    double ema = -1., rect = -1., snr = -1.;
	    double fk_lead, fk_lag, fk_dk;
	    int i, id = -1, num;
	    string ch;

	    if(p->w->ts->getValue("beam_id", &id) && id >= 0 && id < 4)
	    {
		getFkParams(&fk_lead, &fk_lag, &fk_dk);
		cfreq = .5*(fmin[id] + fmax[id]);
		if(fstat[id] != 0. && cfreq != 0.) {
		    delslo = fk_dk/(sqrt(fstat[id])*cfreq);
		    delslo *= DEG_TO_KM;
		}
		if(slow[id] != 0.) {
		    delaz = delslo/(2.*slow[id]);
		    if(delaz < 1.0) {
			delaz = 2.*asin(delaz)*180./M_PI;
		    }
		    else {
			delaz = 180;
		    }
		}
		if(fk->wp) {
		    num = fk->wp->getWaveforms(ws);
		    for(i = 0; i < num; i++) {
			if(!strcasecmp(ws[i]->net(), p->w->net()) &&
			    ws[i]->segment(p->time) ) break;
		    }
		    if(i < num) {
			ch = ws[i]->channel;
			ws[i]->channel.assign("fk");
			arrival = fk->wp->makeArrival(ws[i], password, p->time,
				p->name, deltim, azimuth[id], delaz, slow[id],
				delslo, ema, rect, snr, true);
			ws[i]->channel = ch;
			if(arrival) {
				fk->wp->putArrivalWithColor(arrival,
					stringToPixel("black"));
			}
			made_it = true;
		    }
		}
		if(!made_it) {
		    ch = p->w->channel;
		    p->w->channel.assign("fk");
		    arrival = makeArrival(p->w, password, p->time, p->name,
				deltim, azimuth[id], delaz, slow[id], delslo,
				ema, rect, snr, true);
		    p->w->channel = ch;
		    if(arrival) {
			putArrivalWithColor(arrival, stringToPixel("black"));
		    }
		}
            }
	    return arrival;
        }
};

} // namespace libgfk

FK::FK(const char *name, Component *parent, DataSource *ds) :
		Frame(name, parent, true), DataReceiver(ds)
{
    data_source = NULL;
    createInterface(FK_SINGLE_BAND);
    setDataSource(ds);
    init(FK_SINGLE_BAND);
}

FK::FK(const char *name, Component *parent, FKType fktype, DataSource *ds) :
		Frame(name, parent, true), DataReceiver(NULL)
{
    createInterface(fktype);
    setDataSource(ds);
    init(fktype);
}

void FK::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
        if(data_source) {
	    data_source->removeDataListener(this);
            data_source->removeDataReceiver(this);
            if(wp) {
		wp->removeActionListener(this, XtNdoubleLineCallback);
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, "RTDUpdate");
            }
        }
        data_source = ds;
        if(data_source) {
            data_source->addDataReceiver(this);
	    data_source->addDataListener(this);
            wp = data_source->getWaveformPlotInstance();
            if(wp) {
		wp->addActionListener(this, XtNdoubleLineCallback);
		wp->addActionListener(this, XtNdoubleLineDragCallback);
		if(data_source->isRealTime()) {
		    WaveformWindow *ww=data_source->getWaveformWindowInstance();
		    if(ww) ww->addActionListener(this, "RTDUpdate");
		    auto_compute_button->setVisible(false);
		    rtd_compute_toggle->setVisible(true);
		    tool_bar->add(rtd_compute_toggle, "RTD Compute", 2);
		    rtd_save_button->setVisible(true);
		    tool_bar->add(rtd_save_button, "RTD Save", 3);
		}
            }
        }
        else wp = NULL;
    }
}

void FK::createInterface(FKType fktype)
{

    if(get_resources)
    {
	GetResources((XtPointer)&sub_data);
    }

//    fileDialog = NULL;
//    saveDialog = NULL;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    compute_button = new Button("Compute", file_menu, this);
    auto_compute_button = new Button("Auto Compute", file_menu, this);
    print_button = new Button("Print...", file_menu, this);
    new_fk_button = new Button("New FK Window...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    undo_button = new UndoButton("Undo", edit_menu);
    undo_button->setSensitive(false);

    view_menu = new Menu("View", menu_bar);
    grid_toggle = new Toggle("Grid", view_menu, this, false);
    fine_grid_toggle = new Toggle("Fine Grid", view_menu, this, false);
    wide_grid_toggle = new Toggle("Wide Grid", view_menu, this, false);
    units_menu = new Menu("Units", view_menu, true);
    units_km_toggle = new Toggle("sec per km", units_menu, this, true);
    units_km_toggle->set(true);
    units_deg_toggle = new Toggle("sec per deg", units_menu, this, true);
    predicted_toggle = new Toggle("Predicted Arrivals", view_menu, this);
    predicted_toggle->set(false);
    labels_button = new Button("Labels...", view_menu, this);
    start_button = new Button("Start", view_menu, this);
    start_button->setVisible(false);
    start_button->setSensitive(false);
    back_button = new Button("Back", view_menu, this);
    back_button->setVisible(false);
    back_button->setSensitive(false);
    forward_button = new Button("Forward", view_menu, this);
    forward_button->setVisible(false);
    forward_button->setSensitive(false);

    option_menu = new Menu("Option", menu_bar);
    auto_cursor_toggle = new Toggle("Auto Cursor", option_menu);
    rtd_compute_toggle = new Toggle("RTD Compute", option_menu, this);
    rtd_compute_toggle->setVisible(false);
    rtd_save_button = new Button("RTD Save...", option_menu, this);
    rtd_save_button->setVisible(false);

    align_toggle = new Toggle("Align Waveforms", option_menu, this);
    beam_button = new Button("Beam Selected", option_menu, this);
    location_menu = new Menu("Beam Location", option_menu);
    dnorth_deast_toggle = new Toggle("dnorth/deast", location_menu, this, true);
    dnorth_deast_toggle->set(true);
    ref_sta_toggle = new Toggle("reference station", location_menu, this, true);
    array_center_toggle = new Toggle("array center", location_menu, this, true);
    ftrace_button = new Button("Ftrace...", option_menu, this);
    ref_sta_button = new Button("Reference Stations...", option_menu, this);
    param_button = new Button("Parameters...", option_menu, this);
    save_slow_button = new Button("Save Slowness", option_menu, this);
    sig_meas_button = new Button("Signal Measurements...", option_menu, this);
    taper_menu = new Menu("Taper", option_menu, true);
    hanning_toggle = new Toggle("Hanning", taper_menu, this, true);
    hamming_toggle = new Toggle("Hamming", taper_menu, this, true);
    cosine_toggle = new Toggle("Cosine", taper_menu, this, true);
    cosine_toggle->set(true);
    parzen_toggle = new Toggle("Parzen", taper_menu, this, true);
    welch_toggle = new Toggle("Welch", taper_menu, this, true);
    blackman_toggle = new Toggle("Blackman", taper_menu, this, true);
    none_toggle = new Toggle("None", taper_menu, this, true);
    taper_per_button = new Button("Taper Percent...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    fk_help_button = new Button("FK Help", help_menu, this);

    if(fktype == FK_SINGLE_BAND) {
	createSingleInterface();
	form = NULL;
    }
    else {
	createMultiInterface();
/*
	rc = NULL; az_rc = NULL; slow_rc = NULL; app_rc = NULL;
	az_label = NULL; slow_label = NULL; app_label = NULL;
	az_text = NULL; slow_text = NULL; app_text = NULL;
*/
    }

    labels_window = NULL;
    print_window = NULL;
    ref_sta_window = NULL;
    taper_window = NULL;

    parameter_window = new FkParamDialog("FK Parameters", this, fktype);
    parameter_window->setCommandString("Compute");
    parameter_window->addActionListener(this);
    parameter_window->window_length_text->addActionListener(this,
			XmNvalueChangedCallback);
    parameter_window->window_overlap_text->addActionListener(this,
			XmNvalueChangedCallback);
    parameter_window->stav_length_text->addActionListener(this,
			XmNvalueChangedCallback);
    parameter_window->ltav_length_text->addActionListener(this,
			XmNvalueChangedCallback);
    if(fktype == FK_SINGLE_BAND) {
	parameter_window->bandwidth_text->addActionListener(this,
			XmNvalueChangedCallback);
	parameter_window->scan_frequencies_toggle->addActionListener(this,
			XmNvalueChangedCallback);
    }

    if(sub_data.fk_units == FK_SEC_PER_DEG) {
	char s[50];
	ftoa((float)(.36*DEG_TO_KM), 3, 0, s, sizeof(s));
	parameter_window->setString("s_max_text", s);
	parameter_window->setLabel("slow_min_label", "slowness min (s/deg)");
	parameter_window->setLabel("slow_max_label", "slowness max (s/deg)");
	parameter_window->setString("slow_max_text", s);
    }
    else {
	parameter_window->setString("s_max_text", ".36");
	parameter_window->setLabel("slow_min_label", "slowness min (s/km)");
	parameter_window->setLabel("slow_max_label", "slowness max (s/km)");
	parameter_window->setString("slow_max_text", ".23");
    }
    if(fktype == FK_SINGLE_BAND) {
	parameter_window->setString("num_s_text", "81");
    }
    else {
	parameter_window->setString("num_s_text", "61");
    }
    parameter_window->setString("slow_min_text", "0.");
    parameter_window->setString("az_min_text", "0.");
    parameter_window->setString("az_max_text", "360.");

    memset((void *)net, 0, sizeof(net));

    enableCallbackType(XmNactivateCallback);

    addPlugins("FK", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
	tool_bar->add(auto_compute_button, "Auto Compute");
	tool_bar->add(start_button, "Start");
	tool_bar->add(back_button, "-");
	tool_bar->add(forward_button, "+");
	tool_bar->add(sig_meas_button, "Signal");
	tool_bar->add(beam_button, "Beam");
	tool_bar->add(ftrace_button, "Ftrace");
    }
}

void FK::createSingleInterface(void)
{
    int n;
    Arg args[30];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, false); n++;
    XtSetArg(args[n], XtNcolumns, 7); n++;
    const char *col_labels[] = {"Min(Hz)", "Max(Hz)", "AppVel(km/s)",
        "Slowness(s/km)", "Az(deg)", "Fstat", "SNR"};
    if(sub_data.fk_units == FK_SEC_PER_DEG) {
        col_labels[3] = "Slowness(s/deg)";
    }
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    Boolean col_editable[] = {true, true, true, true, true, false, false};
    XtSetArg(args[n], XtNcolumnEditable, col_editable); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, false); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, false); n++;
    const char *cells[] = {
        "0.5", "6.0", "0.0", "0.0", "0.0", "0.0", "0.0", (char *)NULL};
    fmin[0] = getProperty("fkmin", 0.5);
    fmax[0] = getProperty("fkmax", 6.0);

    char fn[10], fx[10];
    snprintf(fn, sizeof(fn), "%.1f", fmin[0]);
    cells[0] = fn;
    snprintf(fx, sizeof(fx), "%.1f", fmax[0]);
    cells[1] = fx;

    XtSetArg(args[n], XtNcells, cells); n++;
    table = new Table("table", frame_form, args, n);
    table->addActionListener(this, XmNvalueChangedCallback);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, table->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    freq_form = new Form("freq_form", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    lo_label[0] = new Label("Low Freq.", freq_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    lo_text[0] = new TextField("low text", freq_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, lo_label[0]->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, lo_text[0]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 100); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    lo_scale[0] = new Scale("low scale0", freq_form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    hi_label[0] = new Label("High Freq.", freq_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    hi_text[0] = new TextField("high text", freq_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, hi_label[0]->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, hi_text[0]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 100); n++;
    XtSetArg(args[n], XmNvalue, 100); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    hi_scale[0] = new Scale("high scale0", freq_form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, freq_form->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNmode, 0); n++;
    XtSetArg(args[n], XtNtickmarksInside, false); n++;
    XtSetArg(args[n], XtNmarkMax, True); n++;
    XtSetArg(args[n], XtNxLabel, "Sx (s/deg)"); n++;
    XtSetArg(args[n], XtNyLabel, "Sy (s/deg)"); n++;
    XtSetArg(args[n], XtNuniformScale, True); n++;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNplotBackgroundColor, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNwidth, 550); n++;
    XtSetArg(args[n], XtNheight, 550); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
    plot1 = new ConPlotClass("plot1", pane, args, n);
    plot1->setLimits(-1., 1., -1., 1., true);

    n = 0;
    XtSetArg(args[n], XmNheight, 150); n++;
    XtSetArg(args[n], XmNwidth, 600); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    beam_plot = new BeamWaveformView("Beams", pane, this, info_area, args, n);
    beam_plot->addActionListener(this, XtNdoubleLineCallback);
    beam_plot->addActionListener(this, XtNdoubleLineDragCallback);
    beam_plot->selectIaspeiPhases(true);

    plot1->addActionListener(this, XtNcrosshairDragCallback);
    plot1->addActionListener(this, XtNcrosshairCallback);

    plot2 = plot3 = plot4 = NULL;
}

void FK::createMultiInterface(void)
{
    int n;
    Arg args[30];
    char fn[4][10], fx[4][10];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XtNradioSelect, True); n++;
//    XtSetArg(args[n], XtNtoggleLabel, "Select Row"); n++;
    XtSetArg(args[n], XtNcolumns, 7); n++;
    const char *col_labels[] = {"Min(Hz)", "Max(Hz)", "AppVel(km/s)",
	"Slowness(s/km)", "Az(deg)", "Fstat", "SNR"};
    if(sub_data.fk_units == FK_SEC_PER_DEG) {
	col_labels[3] = "Slowness(s/deg)";
    }
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    Boolean col_editable[] = {true, true, true, true, true, false, false};
    XtSetArg(args[n], XtNcolumnEditable, col_editable); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, false); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, false); n++;
    const char *cells[] = {
	"0.6", "3.0", "0.0", "0.0", "0.0", "0.0", "0.0",
	"2.0", "4.0", "0.0", "0.0", "0.0", "0.0", "0.0",
	"3.0", "5.0", "0.0", "0.0", "0.0", "0.0", "0.0",
	"4.0", "6.0", "0.0", "0.0", "0.0", "0.0", "0.0", (char *)NULL};
    fmin[0] = getProperty("fkmin1", 0.6);
    fmin[1] = getProperty("fkmin2", 2.0);
    fmin[2] = getProperty("fkmin3", 3.0);
    fmin[3] = getProperty("fkmin4", 4.0);

    fmax[0] = getProperty("fkmax1", 3.0);
    fmax[1] = getProperty("fkmax2", 4.0);
    fmax[2] = getProperty("fkmax3", 5.0);
    fmax[3] = getProperty("fkmax4", 6.0);

    for(int i = 0; i < 4; i++) {
	snprintf(fn[i], sizeof(fn[i]), "%.1f", fmin[i]);
	cells[i*7] = fn[i];
	snprintf(fx[i], sizeof(fx[i]), "%.1f", fmax[i]);
	cells[i*7+1] = fx[i];
    }

    XtSetArg(args[n], XtNcells, cells); n++;
    table = new Table("table", frame_form, args, n);

    table->selectRow(0, true);
    table->addActionListener(this, XmNvalueChangedCallback);
    table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, table->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    form = new Form("form", pane, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    form1 = new Form("form1", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    lo_text[0] = new TextField("low text", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, lo_text[0]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    lo_scale[0] = new Scale("low scale0", form1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    hi_text[0] = new TextField("high text", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, hi_text[0]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    hi_scale[0] = new Scale("high scale0", form1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNbottomPosition, 1); n++;
    XtSetArg(args[n], XtNmode, 0); n++;
    XtSetArg(args[n], XtNtickmarksInside, false); n++;
    XtSetArg(args[n], XtNmarkMax, True); n++;
    XtSetArg(args[n], XtNuniformScale, True); n++;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNscrollbarAsNeeded, True); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNplotBackgroundColor, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNdataColor, stringToPixel("medium blue")); n++;
    XtSetArg(args[n], XtNwidth, 250); n++;
    XtSetArg(args[n], XtNheight, 250); n++;
    XtSetArg(args[n], XtNbottomMargin, 5); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
    plot1 = new ConPlotClass("plot1", form, args, n);
    plot1->setLimits(-1., 1., -1., 1., true);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    form2 = new Form("form2", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    lo_text[1] = new TextField("low text", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, lo_text[1]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    lo_scale[1] = new Scale("low scale1", form2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    hi_text[1] = new TextField("high text", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, hi_text[1]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    hi_scale[1] = new Scale("high scale1", form2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form2->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNbottomPosition, 1); n++;
    XtSetArg(args[n], XtNmode, 0); n++;
    XtSetArg(args[n], XtNtickmarksInside, false); n++;
    XtSetArg(args[n], XtNmarkMax, True); n++;
//    XtSetArg(args[n], XtNtitle, "Frequency 2. - 4. Hz"); n++;
    XtSetArg(args[n], XtNuniformScale, True); n++;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNscrollbarAsNeeded, True); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNplotBackgroundColor, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNdataColor, stringToPixel("forest green")); n++;
    XtSetArg(args[n], XtNwidth, 250); n++;
    XtSetArg(args[n], XtNheight, 250); n++;
    XtSetArg(args[n], XtNbottomMargin, 5); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
    plot2 = new ConPlotClass("plot2", form, args, n);
    plot2->setLimits(-1., 1., -1., 1., true);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNtopPosition, 1); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    form3 = new Form("form3", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    lo_text[2] = new TextField("low text", form3, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, lo_text[2]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    lo_scale[2] = new Scale("low scale2", form3, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    hi_text[2] = new TextField("high text", form3, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, hi_text[2]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    hi_scale[2] = new Scale("high scale2", form3, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form3->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNmode, 0); n++;
    XtSetArg(args[n], XtNtickmarksInside, false); n++;
    XtSetArg(args[n], XtNmarkMax, True); n++;
//    XtSetArg(args[n], XtNtitle, "Frequency 3. - 5. Hz"); n++;
    XtSetArg(args[n], XtNuniformScale, True); n++;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNscrollbarAsNeeded, True); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNplotBackgroundColor, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNdataColor, stringToPixel("sky blue")); n++;
    XtSetArg(args[n], XtNwidth, 250); n++;
    XtSetArg(args[n], XtNheight, 250); n++;
    XtSetArg(args[n], XtNbottomMargin, 5); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
    plot3 = new ConPlotClass("plot3", form, args, n);
    plot3->setLimits(-1., 1., -1., 1., true);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNtopPosition, 1); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    form4 = new Form("form4", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    lo_text[3] = new TextField("low text", form4, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, lo_text[3]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    lo_scale[3] = new Scale("low scale3", form4, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    hi_text[3] = new TextField("high text", form4, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, hi_text[3]->baseWidget()); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNminimum, 1); n++;
    XtSetArg(args[n], XmNmaximum, 128); n++;
    XtSetArg(args[n], XmNvalue, 1); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNshowArrows, XmEACH_SIDE); n++;
    hi_scale[3] = new Scale("high scale3", form4, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form4->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNmode, 0); n++;
    XtSetArg(args[n], XtNtickmarksInside, false); n++;
    XtSetArg(args[n], XtNmarkMax, True); n++;
//    XtSetArg(args[n], XtNtitle, "Frequency 4. - 6. Hz"); n++;
    XtSetArg(args[n], XtNuniformScale, True); n++;
    XtSetArg(args[n], XtNusePixmap, True); n++;
    XtSetArg(args[n], XtNscrollbarAsNeeded, True); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNplotBackgroundColor, stringToPixel("white")); n++;
    XtSetArg(args[n], XtNdataColor, stringToPixel("orange")); n++;
    XtSetArg(args[n], XtNwidth, 250); n++;
    XtSetArg(args[n], XtNheight, 250); n++;
    XtSetArg(args[n], XtNbottomMargin, 5); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
    plot4 = new ConPlotClass("plot4", form, args, n);
    plot4->setLimits(-1., 1., -1., 1., true);

    n = 0;
    XtSetArg(args[n], XmNheight, 170); n++;
    XtSetArg(args[n], XmNwidth, 600); n++;
    XtSetArg(args[n], XtNdataHeight, 40); n++;
    XtSetArg(args[n], XtNautoYScale, True); n++;
    beam_plot = new BeamWaveformView("Beams", pane, this, info_area, args, n);
    beam_plot->addActionListener(this, XtNdoubleLineDragCallback);
    beam_plot->addActionListener(this, XtNdoubleLineCallback);
    beam_plot->selectIaspeiPhases(true);

    plot1->addActionListener(this, XtNcrosshairDragCallback);
    plot1->addActionListener(this, XtNcrosshairCallback);
    plot2->addActionListener(this, XtNcrosshairDragCallback);
    plot2->addActionListener(this, XtNcrosshairCallback);
    plot3->addActionListener(this, XtNcrosshairDragCallback);
    plot3->addActionListener(this, XtNcrosshairCallback);
    plot4->addActionListener(this, XtNcrosshairDragCallback);
    plot4->addActionListener(this, XtNcrosshairCallback);
}

void FK::init(FKType fktype)
{
//    addDOwner(this);

    nbands = (fktype == FK_SINGLE_BAND) ? 1 : 4;

    line_drag = NULL;
    ignore_callback = false;
    for(int i = 0; i < 8; i++) ids[i] = 0;
    auto_display = false;
    p = new FKParam();
    p->three_component = false;
    p->nbands = nbands;
    count = 0;
    rtd_mode = RTD_OFF;
    memset(rtd_sta, 0, sizeof(rtd_sta));
    memset(rtd_chan, 0, sizeof(rtd_chan));
    memset(status_msg, 0, sizeof(status_msg));
    rtd_data_source = NULL;
    rtd_save_secs = 0.;
    update_ftrace = true;

    for(int i = 0; i < 4; i++) {
	conplot[i] = NULL;
	snr[i] = 0.;
	fkmax_snr[i] = 0.;
    }
    current_fk = NULL;

    redraw = false;
    for(int i = 0; i < 2; i++) {
	display_data[i] = false;
	display_grid[i] = false;
    }
    show_single = false;
    slowness = NULL;
    for(int i = 0; i < 4; i++) fkmin[i] = fkmax[i] = 0.;
    p->beam_input = false;

    p->fk_units = sub_data.fk_units;

    /* need to fix the label in the popup */
    setLabels();

    if(nbands == 1) {
	conplot[0] = plot1;
    }

    getBands();
    int value;
    int nf = 128;
//    double f;
    double df = 1./(128.*.025);
    
    for(int i = 0; i < nbands; i++)
    {
	if(lo_scale[i]->getMaximum() != nf) {
		lo_scale[i]->setMaximum(nf);
	}
	value = (int)(fmin[i]/df + .5);
	if(value < 1) value = 1;
	else if(value > nf) value = nf;
	lo_scale[i]->setValue(value);
//	f = value*df;
//	lo_text[i]->setString("%.2f", f);
	lo_text[i]->setString("%.2f", fmin[i]);

	if(hi_scale[i]->getMaximum() != nf) {
		hi_scale[i]->setMaximum(nf);
	}
	value = (int)(fmax[i]/df + .5);
	if(value < 1) value = 1;
	else if(value > nf) value = nf;
	hi_scale[i]->setValue(value);
//	f = value*df;
//	hi_text[i]->setString("%.2f", f);
	hi_text[i]->setString("%.2f", fmax[i]);
    }

    start_button->setSensitive(false);
    back_button->setSensitive(false);
    forward_button->setSensitive(false);

    if(nbands == 4) {
	plot1->addActionListener(this, XtNlimitsCallback);
	plot2->addActionListener(this, XtNlimitsCallback);
	plot3->addActionListener(this, XtNlimitsCallback);
	plot4->addActionListener(this, XtNlimitsCallback);
    }

//    signal_window = new FkSignal("FK Signal Measurements", this, this);
    signal_window = new FkSignal("FK Signal Measurements",
			Application::getApplication(), this);
    signal_window->manage();

    map = Application::getMap(this);
    if(map) {
	map->addActionListener(this, XtNsetVisibleCallback);
    }
    else {
	Application::addMapListener(this, this->getParent());
    }

    string recipe_dir, recipe_dir2;
    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    recipe_dir.assign(c + string("/tables/recipes"));
	}
    }
    getProperty("recipeDir2", recipe_dir2);

    gbeam = new Beam(recipe_dir, recipe_dir2);
}

void FK::GetResources(XtPointer subdata)
{
    XtGetSubresources(base_widget, subdata, "data", "Data", resources,
		XtNumber(resources), NULL, 0);
    get_resources = false;
}

FK::~FK(void)
{
    if(data_source) data_source->removeDataReceiver(this);
    delete p;
    delete gbeam;
}

void FK::actionPerformed(ActionEvent *action_event)
{
    if(ignore_callback) return;

    string s;
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(reason, XtNlimitsCallback) && nbands == 4)
    {
	tieLimits(comp,(AxesLimitsCallbackStruct *)action_event->getCalldata());
    }
    else if(!strncmp(cmd, "signal_plot", 11) || comp == beam_plot ||
	comp == signal_window->beam_plot)
    {
	lineDrag((AxesClass *)comp,
		(AxesCursorCallbackStruct *)action_event->getCalldata());
    }
    else if(comp == wp) {
	if(rtd_mode == RTD_COMPUTING) {
	    return;
	}
	else if(!strcmp(reason, XtNdataChangeCallback)) {
	    setButtonsSensitive();
	    wvec.clear();
	}
	else if(auto_cursor_toggle->state()) {
	    if(!strcmp(reason, XtNdoubleLineCallback)) {
		doubleLine((AxesCursorCallbackStruct *)action_event->getCalldata());
	    }
	}
	else {
	    if(!strcmp(reason, XtNdoubleLineCallback) ||
		!strcmp(reason, XtNdoubleLineDragCallback))
	    {
		windowDrag((AxesCursorCallbackStruct *)action_event->getCalldata());
	    }
	}
    }
    else if(!strcmp(reason, "RTDUpdate")) {
	if(rtd_mode == RTD_WAITING) {
	    rtd_mode = RTD_COMPUTING;
	    rtdContinue();
	    rtd_mode = RTD_WAITING;
	}
    }
    else if(!strcmp(cmd, "plot1") || !strcmp(cmd, "plot2") ||
		!strcmp(cmd, "plot3") || !strcmp(cmd, "plot4"))
    {
	if(!strcmp(reason, XtNcrosshairDragCallback) ||
	    !strcmp(reason, XtNcrosshairCallback))
	{
	    crosshairCB((ConPlotClass *)comp,
		(AxesCursorCallbackStruct *)action_event->getCalldata());
	}
    }
    else if(!strcmp(cmd, "table")) {
	if(!strcmp(reason, XmNvalueChangedCallback)) {
	    tableEdit((MmTableEditCallbackStruct *)action_event->getCalldata());
	}
	else if(!strcmp(reason, XtNselectRowCallback)) {
	    tableSelect();
	}
    }
    else if(!strcmp(cmd, "Compute")) {
//	stopDisplay();
	compute(true, false);
    }
    else if(!strcmp(cmd, "Auto Compute")) {
	stopDisplay();
	computeAll(true, false);
    }
    else if(!strcmp(cmd, "RTD Compute")) {
	if(rtd_compute_toggle->state()) {
	    rtd_mode = RTD_START;
	    computeButtonsOn(false);
//	    stopDisplay();
	    rtd_mode = RTD_COMPUTING;
	    computeAll(true, false);
	    rtd_mode = RTD_WAITING;
//	    count = p->num_fkdata - 5;
//	    if(count < 0) count = 0;
//	    startDisplay();
	}
	else {
	    rtd_mode = RTD_OFF;
	    computeButtonsOn(true);
	}
    }
    else if(!strcmp(cmd, "RTD Save...")) {
	char current_value[50];
	snprintf(current_value, sizeof(current_value), "%d",
			(int)(rtd_save_secs+.5));
	char *ans = TextQuestion::askTextQuestion("FK Save Time (secs)",
			this, "Enter FK Save Time (secs)", current_value,
			"Apply", "Cancel");
	if(ans) {
	    stringToDouble(ans, &rtd_save_secs);
	    if(rtd_save_secs < 300.) rtd_save_secs = 300.;
	    free(ans);
	}
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
    else if(!strcmp(cmd, "Save Slowness")) {
	saveAzSlow();
    }
    else if(!strcmp(cmd, "Predicted Arrivals")) {
	Arg args[1];
	if(predicted_toggle->state()) {
	    XtSetArg(args[0], XtNdisplayTtLabels, True);
	}
	else {
	    XtSetArg(args[0], XtNdisplayTtLabels, False);
	}
	beam_plot->setValues(args, 1);
    }
    else if(!strcmp(cmd, "Labels...")) {
	if(!labels_window) {
	    labels_window = new AxesLabels("FK Labels", this, plot1);
	}
	labels_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Signal Measurements...")) {
	signal_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "New FK Window..."))
    {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    FK *fk;
	    if(p->nbands == 1) {
		fk = new FK(getName(), this, FK_SINGLE_BAND, data_source);
	    }
	    else {
		fk = new FK(getName(), this, FK_MULTI_BAND, data_source);
	    }
	    windows.push_back(fk);
	    fk->setVisible(true);
	}
    }
    else if(!strcmp(cmd, "Parameters...")) {
	parameter_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Reference Stations...")) {
// the data_source should be from the data. This button should be insensitive
// when there is no selected data?
	showRefSta(true);
    }
    else if(!strncmp(cmd, "sec per", 7)) {
	Toggle *t = comp->getToggleInstance();
	if(t && t->state()) changeUnits();
    }
    else if(!strcmp(cmd, "Wide Grid")) {
	wideGrid();
    }
    else if(!strcmp(cmd, "Grid")) {
	grid();
    }
    else if(!strcmp(cmd, "Fine Grid")) {
	fineGrid();
    }
    else if(!strcmp(cmd, "Taper Percent...")) {
	if(!taper_window) {
	    taper_window = new TaperWindow("FK Cosine Taper", this, 5, 5, 200);
	}
	taper_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Start")) {
	autoStart();
    }
    else if(!strcmp(cmd, "Back")) {
	autoStep(false);
    }
    else if(!strcmp(cmd, "Forward")) {
	autoStep(true);
    }
    else if(!strcmp(cmd, "Beam Selected")) {
	fkBeam();
    }
    else if(!strcmp(cmd, "Ftrace...")) {
	doFtrace();
    }
    else if(!strcmp(cmd, "FK Help")) {
	showHelp(cmd);
    }
    else if(!strncmp(cmd, "low scale", 9)) {
	if(current_fk) {
	    int i = atoi(cmd+9);
	    int lo_value = lo_scale[i]->getValue();
	    int hi_value = hi_scale[i]->getValue();
	    double df =  1./((double)current_fk->nt*current_fk->dt);
	    double f = lo_value*df;
	    lo_text[i]->setString("%.2f", f);
	    if(lo_value > hi_value) {
		hi_scale[i]->setValue(lo_value);
		hi_text[i]->setString("%.2f", f);
	    }
	    makeFK(i, lo_scale[i]->getValue(), hi_scale[i]->getValue());
	}
    }
    else if(!strncmp(cmd, "high scale", 10)) {
	if(current_fk) {
	    int i = atoi(cmd+10);
	    int lo_value = lo_scale[i]->getValue();
	    int hi_value = hi_scale[i]->getValue();
	    double df =  1./((double)current_fk->nt*current_fk->dt);
	    double f = hi_value*df;
	    hi_text[i]->setString("%.2f", f);
	    if(hi_value < lo_value) {
		lo_scale[i]->setValue(hi_value);
		lo_text[i]->setString("%.2f", f);
	    }
	    makeFK(i, lo_scale[i]->getValue(), hi_scale[i]->getValue());
	}
    }
    else if(comp == align_toggle) {
	vector<int> rows;
	if(align_toggle->state() && (nbands == 1 ||
		table->getSelectedRows(rows) > 0))
	{
	    double vel, slow, az;
	    if(nbands == 1) rows.push_back(0);
	    if(	table->getDouble(rows[0], 2, &vel) &&
		table->getDouble(rows[0], 4, &az))
	    {
		slow = 1./vel;
		align(slow, az);
	    }
	}
    }
    else if(comp == parameter_window->window_length_text) {
	parameter_window->window_length_text->getString(s);
	signal_window->window_length_text->setString(s);
    }
    else if(comp == parameter_window->window_overlap_text) {
	parameter_window->window_overlap_text->getString(s);
	signal_window->window_overlap_text->setString(s);
    }
    else if(comp == parameter_window->stav_length_text) {
	parameter_window->stav_length_text->getString(s);
	signal_window->stav_length_text->setString(s);
    }
    else if(comp == parameter_window->ltav_length_text) {
	parameter_window->ltav_length_text->getString(s);
	signal_window->ltav_length_text->setString(s);
    }
    else if(comp == parameter_window->bandwidth_text) {
	parameter_window->bandwidth_text->getString(s);
	signal_window->bandwidth_text->setString(s);
    }
    else if(comp == parameter_window->scan_frequencies_toggle) {
	parameter_window->bandwidth_label->setSensitive(
			parameter_window->scan_frequencies_toggle->state());
	parameter_window->bandwidth_text->setSensitive(
			parameter_window->scan_frequencies_toggle->state());
	signal_window->scan_frequencies_toggle->set(
		parameter_window->scan_frequencies_toggle->state(), false);
	signal_window->bandwidth_label->setSensitive(
			parameter_window->scan_frequencies_toggle->state());
	signal_window->bandwidth_text->setSensitive(
			parameter_window->scan_frequencies_toggle->state());
    }
    else if(comp == signal_window->window_length_text) {
	signal_window->window_length_text->getString(s);
	parameter_window->window_length_text->setString(s);
    }
    else if(comp == signal_window->window_overlap_text) {
	signal_window->window_overlap_text->getString(s);
	parameter_window->window_overlap_text->setString(s);
    }
    else if(comp == signal_window->stav_length_text) {
	signal_window->stav_length_text->getString(s);
	parameter_window->stav_length_text->setString(s);
    }
    else if(comp == signal_window->ltav_length_text) {
	signal_window->ltav_length_text->getString(s);
	parameter_window->ltav_length_text->setString(s);
    }
    else if(type() == FK_SINGLE_BAND && comp == signal_window->bandwidth_text) {
	signal_window->bandwidth_text->getString(s);
	parameter_window->bandwidth_text->setString(s);
    }
    else if(type() == FK_SINGLE_BAND &&
		comp == signal_window->scan_frequencies_toggle)
    {
	signal_window->bandwidth_label->setSensitive(
			signal_window->scan_frequencies_toggle->state());
	signal_window->bandwidth_text->setSensitive(
			signal_window->scan_frequencies_toggle->state());
	parameter_window->scan_frequencies_toggle->set(
		signal_window->scan_frequencies_toggle->state(), false);
	parameter_window->bandwidth_label->setSensitive(
			signal_window->scan_frequencies_toggle->state());
	parameter_window->bandwidth_text->setSensitive(
			signal_window->scan_frequencies_toggle->state());
    }
    else if(!strcmp(reason, XtNmapCallback)) {
	if(!map) {
	    map = (BasicMap *)comp;
	    map->addActionListener(this, XtNsetVisibleCallback);
	    redrawMap();
	}
    }
    else if(comp == map) { // XtNsetVisibleCallback from the map
	if(map->isVisible()) redrawMap();
    }
    else if(!strcmp(cmd, "Close")) { // free data?
	setVisible(false);
    }
}

void FK::tableSelect(void)
{
    vector<int> rows;
    int nrows = table->getSelectedRows(rows);
    string msg;
    ostringstream os;

    beam_plot->selectWaveforms(DESELECT_ALL);
    if(nrows > 0) {
	os << "select " << rows[0]+1;
	beam_plot->parseCmd(os.str(), msg);
    }
}

ParseCmd FK::parseCmd(const string &cmd, string &msg)
{
    string s;
    ostringstream os;

    for(int i = 0; i < 10; i++) {
	os.str("");
	os << i+2;
	if(parseString(cmd, os.str(), s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(s, msg);
	    }
            for(int j = (int)windows.size(); j <= i; j++) {
		FK *fk;
		if(p->nbands == 1) {
		    fk = new FK(getName(), this, FK_SINGLE_BAND, data_source);
		}
		else {
		    fk = new FK(getName(), this, FK_MULTI_BAND, data_source);
		}
		windows.push_back(fk);
		if(j == i) {
		    return fk->parseCmd(s, msg);
		}
	    }
	}
    }
    os.str("");

    if(parseCompare(cmd, "Compute")) {
	compute(true, false);
    }
    else if(parseCompare(cmd, "Auto_Compute")) {
	auto_compute_button->activate();
    }
    else if(parseArg(cmd, "RTD_Compute", s) || parseArg(cmd, "RTD", s))
    {
	return rtd_compute_toggle->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Save", s)) {
    }
    else if(parseString(cmd, "Labels", s)) {
	if(!labels_window) {
	    labels_window = new AxesLabels("FK Labels", this, plot1);
	}
	return labels_window->parseCmd(s, msg);
    }
    else if(parseString(cmd, "Signal", s))
    {
	return signal_window->parseCmd(s, msg);
    }
    else if(parseString(cmd, "print_window", s)) {
	if(print_window == NULL) {
	    print_window = new PrintDialog("Print FK", this, this);
	}
	return print_window->parseCmd(s, msg);
    }
    else if(parseCompare(cmd, "Auto_Cursor")) {
	return auto_cursor_toggle->parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "Align_Waveforms")) {
	return align_toggle->parseCmd(cmd, msg);
    }
    else if(parseString(cmd, "Parameters", s)) {
	return parameter_window->parseCmd(s, msg);
    }
    else if(parseString(cmd, "slowness_max", s) ||
	parseString(cmd, "num_slowness", s) ||
	parseString(cmd, "signal_slowness_min", s) ||
	parseString(cmd, "signal_slowness_max", s) ||
	parseString(cmd, "azimuth_min", s) ||
	parseString(cmd, "azimuth_max", s) ||
	parseString(cmd, "window_length", s) ||
	parseString(cmd, "window_overlap", s) ||
	parseString(cmd, "stav_length", s) ||
	parseString(cmd, "ltav_length", s))
    {
	return parameter_window->parseCmd(cmd, msg);
    }
    else if(parseString(cmd, "Reference_Stations", s)) {
	showRefSta(false);
	return ref_sta_window->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Units", s)) {
	return units_menu->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Wide_Grid", s)) {
	return wide_grid_toggle->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Grid", s)) {
	return grid_toggle->parseCmd(s, msg);
    }
    else if(parseArg(cmd, "Fine_Grid", s)) {
	return fine_grid_toggle->parseCmd(s, msg);
    }
    else if(parseString(cmd, "Taper_Percent", s)) {
	if(!taper_window) {
	    taper_window = new TaperWindow("FK Cosine Taper", this, 5, 5, 200);
	}
	return taper_window->parseCmd(s, msg);
    }
    else if(parseString(cmd, "Beam_Location", s)) {
	return location_menu->parseCmd(s, msg);
    }
    else if(parseCompare(cmd, "Start")|| parseCompare(cmd, "Stop")) {
	autoStart();
    }
    else if(parseCompare(cmd, "Back") || parseCompare(cmd, "-")) {
	autoStep(false);
    }
    else if(parseCompare(cmd, "Forward") || parseCompare(cmd, "+")) {
	autoStep(true);
    }
    else if(parseCompare(cmd, "Beam_Selected") || parseCompare(cmd, "Beam")) {
	fkBeam();
    }
    else if(parseArg(cmd, "low_freq", s) ||
	    parseArg(cmd, "low_freq1", s) ||
	    parseArg(cmd, "low_freq2", s) ||
	    parseArg(cmd, "low_freq3", s) ||
	    parseArg(cmd, "low_freq4", s))
    {
	double f;
	if(!stringToDouble(s.c_str(), &f)) return ARGUMENT_ERROR;
	if(current_fk) {
	    int i=0;
	    if(parseArg(cmd, "low_freq", s)) i = 0;
	    else if(parseArg(cmd, "low_freq1", s)) i = 0;
	    else if(parseArg(cmd, "low_freq2", s)) i = 1;
	    else if(parseArg(cmd, "low_freq3", s)) i = 2;
	    else if(parseArg(cmd, "low_freq4", s)) i = 3;
	    if(i > nbands-1) {
		msg.assign("Only one frequency band.");
		return ARGUMENT_ERROR;
	    }
	    double df =  1./((double)current_fk->nt*current_fk->dt);
	    int lo_value = (int)(f/df + .5);
	    int nf = hi_scale[i]->getMaximum();
	    if(lo_value < 1) lo_value = 1;
	    else if(lo_value > nf) lo_value = nf;
	    lo_scale[i]->setValue(lo_value);
	    lo_text[i]->setString("%.2f", f);
	    int hi_value = hi_scale[i]->getValue();
	    if(lo_value > hi_value) {
		hi_scale[i]->setValue(lo_value);
		hi_text[i]->setString("%.2f", f);
	    }
	    makeFK(i, lo_scale[i]->getValue(), hi_scale[i]->getValue());
	}
    }
    else if(parseArg(cmd, "high_freq", s) ||
	    parseArg(cmd, "high_freq1", s) ||
	    parseArg(cmd, "high_freq2", s) ||
	    parseArg(cmd, "high_freq3", s) ||
	    parseArg(cmd, "high_freq4", s))
    {
	double f;
	if(!stringToDouble(s.c_str(), &f)) return ARGUMENT_ERROR;
	if(current_fk) {
	    int i=0;
	    if(parseArg(cmd, "high_freq", s)) i = 0;
	    else if(parseArg(cmd, "high_freq1", s)) i = 0;
	    else if(parseArg(cmd, "high_freq2", s)) i = 1;
	    else if(parseArg(cmd, "high_freq3", s)) i = 2;
	    else if(parseArg(cmd, "high_freq4", s)) i = 3;
	    if(i > nbands-1) {
		msg.assign("Only one frequency band.");
		return ARGUMENT_ERROR;
	    }
	    double df =  1./((double)current_fk->nt*current_fk->dt);
	    int hi_value = (int)(f/df + .5);
	    int nf = hi_scale[i]->getMaximum();
	    if(hi_value < 1) hi_value = 1;
	    else if(hi_value > nf) hi_value = nf;
	    hi_scale[i]->setValue(hi_value);
	    hi_text[i]->setString("%.2f", f);
	    int lo_value = lo_scale[i]->getValue();
            if(hi_value < lo_value) {
                lo_scale[i]->setValue(hi_value);
                lo_text[i]->setString("%.2f", f);
            }
	    makeFK(i, lo_scale[i]->getValue(), hi_scale[i]->getValue());
	}
    }
    else if(parseString(cmd, "flo1", s)) {
	os << "set_cell row=1 column=\"Min(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "flo2", s)) {
	os << "set_cell row=2 column=\"Min(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "flo3", s)) {
	os << "set_cell row=3 column=\"Min(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "flo4", s)) {
	os << "set_cell row=4 column=\"Min(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "fhi1", s)) {
	os << "set_cell row=1 column=\"Max(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "fhi2", s)) {
	os << "set_cell row=2 column=\"Max(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "fhi3", s)) {
	os << "set_cell row=3 column=\"Max(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "fhi4", s)) {
	os << "set_cell row=4 column=\"Max(Hz)\" value=" << s;
	return table->parseCmd(os.str(), msg);
    }
    else if(parseString(cmd, "display", s)) {
	int k = -1;
	if(parseGetArg(s, "window", &k)) {
	    display(k);
	}
	else {
	    msg.assign(string("Invalid display argument: ") + s);
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseString(cmd, "plot1", s)) {
	return plot1->parseCmd(s, msg);
    }
    else if(parseString(cmd, "plot2", s)) {
	if(plot2) return plot2->parseCmd(s, msg);
	else return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "plot3", s)) {
	if(plot3) return plot3->parseCmd(s, msg);
	else return ARGUMENT_ERROR;
    }
    else if(parseString(cmd, "plot4", s)) {
	if(plot4) return plot4->parseCmd(s, msg);
	else return ARGUMENT_ERROR;
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if(parseString(cmd, "table", s)) {
	return table->parseCmd(s, msg);
    }
    else if(parseString(cmd, "beam_plot", s)) {
	return beam_plot->parseCmd(s, msg);
    }
    else {
	return Frame::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar FK::parseVar(const string &name, string &value)
{
    int i, j, n, nextc;
    ParseVar ret;
    double d;
    ostringstream os;
    string s;

    for(i = 0; i < 10; i++) {
	os.str("");
	os << i+2;
	if(parseString(name, os.str(), s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseVar(s, value);
	    }
            for(j = (int)windows.size(); j <= i; j++) {
		FK *fk;
		if(p->nbands == 1) {
		    fk = new FK(getName(),this,FK_SINGLE_BAND,data_source);
		}
		else {
		    fk = new FK(getName(),this,FK_MULTI_BAND,data_source);
		}
		windows.push_back(fk);
		if(j == i) {
		    return fk->parseVar(s, value);
		}
	    }
	}
    }
    os.str("");

    if(current_fk && parseArrayIndex(name, "fk", nbands, &i, &nextc,value,&ret))
    {
        if(ret != STRING_RETURNED) return ret;
	return parseFK(current_fk, i, name.c_str()+nextc, value);
    }
    else if(parseCompare(name, "nbands")) {
	os << nbands;
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "net")) {
	value.assign(net);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "count")) {
	os << count+1;
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if( parseArray(name, "flo", nbands, fmin, value, &ret) ) {
	return ret;
    }
    else if( parseArray(name, "fhi", nbands, fmax, value, &ret) ) {
	return ret;
    }
    else if( parseArrayIndex(name, "sig", nbands, &i, &nextc, value, &ret) )
    {
        if(ret != STRING_RETURNED) return ret;

	const char *c = name.c_str() + nextc;
	n = p->num_fkdata;
        if( parseArray(c, ".snr", n, p->sig[i].snr, value, &ret) ) return ret;
        else if( parseArray(c, ".azimuth", n, p->sig[i].azimuth,
				value, &ret) ) return ret;
        else if( parseArray(c, ".fstat", n, p->sig[i].fstat, value,
                                &ret) ) return ret;
        else if( parseArray(c, ".appvel", n, p->sig[i].appvel, value,
                                &ret) ) return ret;
        else if( parseArray(c, ".slowness", n, p->sig[i].slowness,
				value, &ret) ) return ret;
        else if( parseArray(c, ".delaz", n, p->sig[i].delaz, value,
                                &ret) ) return ret;
        else if( parseArray(c, ".delslo", n, p->sig[i].delslo, value,
                                &ret) ) return ret;
        else if( parseArray(c, ".time", n, p->x, value, &ret) ) return ret;
	else if( parseArrayIndex(c, ".fk", n, &j, &nextc, value, &ret) ) {
	    if(ret != STRING_RETURNED) return ret;

	    return parseFK(p->fkdata[j], i, c+nextc, value);
/*
	    n = p->fkdata[j]->args.num_slowness;
	    if( parseArray(b, ".matrix", n*n, p->fkdata[j]->fk[i],
			value, value_len, &ret) ) return ret;
	    else if(!strcasecmp(b, ".tbeg")) {
		snprintf(value, value_len, "%.15g", p->fkdata[j]->tbeg);
		return STRING_RETURNED;
	    }
	    else if(!strcasecmp(b, ".tend")) {
		snprintf(value, value_len, "%.15g", p->fkdata[j]->tend);
		return STRING_RETURNED;
	    }
	    return VARIABLE_NOT_FOUND;
*/
	}
	return VARIABLE_NOT_FOUND;
    }
    else if(parseCompare(name, "window_length")) {
	d = 5.;
	if(!parameter_window->window_length_text->getDouble(&d)) {
	    parameter_window->window_length_text->setString("5.0");
	    if(signal_window) signal_window->window_length_text->setString("5.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "window_overlap")) {
	d = 3.;
	if(!parameter_window->window_overlap_text->getDouble(&d)) {
	    parameter_window->window_overlap_text->setString("3.0");
	    if(signal_window) signal_window->window_overlap_text->setString("3.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "stav_length")) {
	d = 1.;
	if(!parameter_window->stav_length_text->getDouble(&d)) {
	    parameter_window->stav_length_text->setString("1.0");
	    if(signal_window) signal_window->stav_length_text->setString("1.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "ltav_length")) {
	d = 60.;
	if(!parameter_window->ltav_length_text->getDouble(&d)) {
	    parameter_window->ltav_length_text->setString("60.");
	    if(signal_window) signal_window->ltav_length_text->setString("60.");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "signal_slowness_min")) {
	d = 0.;
	if(!parameter_window->slow_min_text->getDouble(&d)) {
	    parameter_window->slow_min_text->setString("0.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "signal_slowness_max")) {
	d = 40.;
	if(!parameter_window->slow_max_text->getDouble(&d)) {
	    parameter_window->slow_max_text->setString("40.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "azimuth_min")) {
	d = 0.;
	if(!parameter_window->az_min_text->getDouble(&d)) {
	    parameter_window->az_min_text->setString("0.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "azimuth_max")) {
	d = 0.;
	if(!parameter_window->az_max_text->getDouble(&d)) {
	    parameter_window->az_max_text->setString("360.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "slowness_max")) {
	d = 40.;
	if(!parameter_window->s_max_text->getDouble(&d)) {
	    parameter_window->s_max_text->setString("40.0");
	}
	parsePrintDouble(value, d);
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "num_slowness")) {
	i = 61;
	if(!parameter_window->s_max_text->getInt(&i)) {
	    parameter_window->s_max_text->setString("61");
	}
	parsePrintInt(value, i);
	return STRING_RETURNED;
    }
    else if((ret = dataParse(name.c_str(), value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if(parseString(name, "table", s)) {
	return table->parseVar(s, value);
    }
    else if(parseString(name, "plot1", s)) {
	return plot1->parseVar(s, value);
    }
    else if(parseString(name, "plot2", s)) {
	if(plot2) return plot2->parseVar(s, value);
	else return VARIABLE_ERROR;
    }
    else if(parseString(name, "plot3", s)) {
	if(plot3) return plot3->parseVar(s, value);
	else return VARIABLE_ERROR;
    }
    else if(parseString(name, "plot4", s)) {
	if(plot4) return plot4->parseVar(s, value);
	else return VARIABLE_ERROR;
    }
    else if(parseString(name, "beam_plot", s)) {
	return beam_plot->parseVar(s, value);
    }
    else if(parseString(name, "signal", s)) {
	return signal_window->parseVar(s, value);
    }
    return Frame::parseVar(name, value);
}

ParseVar FK::parseFK(FKData *fkd, int i, const char *a, string &value)
{
    ParseVar ret;
    double sec_per_km, az;
    ostringstream os;
    int n;

    n = current_fk->args.num_slowness;
    if( parseArray(a, ".matrix", n*n, fkd->fk[i], value, &ret) ) {
	return ret;
    }
    if(parseCompare(a, ".nwaveforms")) {
	os << fkd->nwaveforms;
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".net")) {
	value.assign(fkd->net);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".tbeg")) {
	parsePrintDouble(value, fkd->tbeg);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".tend")) {
	parsePrintDouble(value, fkd->tend);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".dt")) {
	parsePrintDouble(value, fkd->dt);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".df")) {
	parsePrintDouble(value, fkd->df);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".nt")) {
	parsePrintInt(value, fkd->nt);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".nf")) {
	parsePrintInt(value, fkd->nf);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".if1")) {
	parsePrintInt(value, fkd->if1);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".if2")) {
	parsePrintInt(value, fkd->if2);
	return STRING_RETURNED;
    }
    else if( parseArray(a, ".lon", fkd->nwaveforms, fkd->lon, value, &ret) ) {
	return ret;
    }
    else if( parseArray(a, ".lat", fkd->nwaveforms, fkd->lat, value, &ret) ) {
	return ret;
    }
    else if(parseCompare(a, ".num_slowness")) {
	parsePrintInt(value, fkd->args.num_slowness);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".slowness_max")) {
	parsePrintDouble(value, fkd->args.slowness_max);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".taper_type")) {
	if(fkd->args.taper_type == HANN_TAPER) {
	    value.assign("hanning");
	}
	else if(fkd->args.taper_type == HAMM_TAPER) {
	    value.assign("hamming");
	}
	else if(fkd->args.taper_type == COSINE_TAPER) {
	    value.assign("cosine");
	}
	else if(fkd->args.taper_type == PARZEN_TAPER) {
	    value.assign("parzen");
	}
	else if(fkd->args.taper_type == WELCH_TAPER) {
	    value.assign("welch");
	}
	else if(fkd->args.taper_type == BLACKMAN_TAPER) {
	    value.assign("blackman");
	}
	else if(fkd->args.taper_type == NO_TAPER) {
	    value.assign("none");
	}
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".taper_beg")) {
	parsePrintDouble(value, fkd->args.taper_beg);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".taper_end")) {
	parsePrintDouble(value, fkd->args.taper_end);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".signal_slow_min")) {
	parsePrintDouble(value, fkd->args.signal_slow_min);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".signal_slow_max")) {
	parsePrintDouble(value, fkd->args.signal_slow_max);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".signal_az_min")) {
	parsePrintDouble(value, fkd->args.signal_az_min);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".signal_az_max")) {
	parsePrintDouble(value, fkd->args.signal_az_max);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".scaling")) {
	parsePrintDouble(value, fkd->scaling[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".local_average")) {
	parsePrintDouble(value, fkd->local_average[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".fstat")) {
	parsePrintDouble(value, fkd->fstat[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".fkmax")) {
	parsePrintDouble(value, fkd->fk_max[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".restricted_fkmax")) {
	parsePrintDouble(value, fkd->restricted_fkmax[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".jmax")) {
	parsePrintInt(value, fkd->jmax[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".kmax")) {
	parsePrintInt(value, fkd->kmax[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".xmax")) {
	parsePrintDouble(value, fkd->xmax[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".ymax")) {
	parsePrintDouble(value, fkd->ymax[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".fmin")) {
	parsePrintDouble(value, fkd->args.fmin[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".fmax")) {
	parsePrintDouble(value, fkd->args.fmax[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".total_power")) {
	parsePrintDouble(value, fkd->total_power[i]);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".slowkm")) {
	FKParam::crosshair_to_slow_az(FK_SEC_PER_KM, fkd->xmax[i], fkd->ymax[i],
				&sec_per_km, &az);
	parsePrintDouble(value, sec_per_km);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".slowdeg")) {
	FKParam::crosshair_to_slow_az(FK_SEC_PER_KM, fkd->xmax[i], fkd->ymax[i],
				&sec_per_km, &az);
	parsePrintDouble(value, sec_per_km*DEG_TO_KM);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".appvel")) {
	FKParam::crosshair_to_slow_az(FK_SEC_PER_KM, fkd->xmax[i], fkd->ymax[i],
				&sec_per_km, &az);
	parsePrintDouble(value, 1./sec_per_km);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".az")) {
	FKParam::crosshair_to_slow_az(FK_SEC_PER_KM, fkd->xmax[i], fkd->ymax[i],
				&sec_per_km, &az);
	parsePrintDouble(value, az);
	return STRING_RETURNED;
    }
    else if(parseCompare(a, ".snr")) {
	parsePrintDouble(value, fkmax_snr[i]);
	return STRING_RETURNED;
    }
    return VARIABLE_NOT_FOUND;
}

void FK::parseHelp(const char *prefix)
{
    char s[200];

    printf("%sauto_compute\n", prefix);
    printf("%scompute\n", prefix);

    memset(s, 0, sizeof(s));
    snprintf(s, sizeof(s), "%slabels.", prefix);
    AxesLabels::parseHelp(s);

    snprintf(s, sizeof(s), "%sparameters.", prefix);
    FkParamDialog::parseHelp(s);

//    printf("%sparameters.reference stations.\n", prefix);
    printf("%sgrid=(true,false)\n", prefix);
    printf("%sfine_grid=(true,false)\n", prefix);
    printf("%slog_amplitude=(true,false)\n", prefix);
    printf("%swide_grid=(true,false)\n", prefix);
    printf("%sunits=(sec per km,sec per deg)\n", prefix);

    printf("%sauto_cursor=(true,false)\n", prefix);
    printf("%salign_waveforms=(true,false)\n", prefix);
    printf("%sbeam_selected\n", prefix);

    snprintf(s, sizeof(s), "%staper percent.", prefix);
    if(!taper_window) {
	taper_window = new TaperWindow("FK Cosine Taper", this, 5, 5, 200);
    }
    taper_window->parseHelp(s);

    printf("%sbeam_location=(dnorth/deast,reference station,array center)\n",
		prefix);
    printf("%sstart\n", prefix);
    printf("%sback\n", prefix);
    printf("%sforward\n", prefix);

    printf("%slow_freq=FREQ\n", prefix);
    printf("%slow_freq1=FREQ\n", prefix);
    printf("%slow_freq2=FREQ\n", prefix);
    printf("%slow_freq3=FREQ\n", prefix);
    printf("%slow_freq4=FREQ\n", prefix);
    printf("%shigh_freq=FREQ\n", prefix);
    printf("%shigh_freq1=FREQ\n", prefix);
    printf("%shigh_freq2=FREQ\n", prefix);
    printf("%shigh_freq3=FREQ\n", prefix);
    printf("%shigh_freq4=FREQ\n", prefix);

    printf("%ssignal.auto_compute\n", prefix);
    printf("%ssignal.+\n", prefix);
    printf("%ssignal.-\n", prefix);
    printf("%ssignal.start\n", prefix);
    printf("%ssignal.stop\n", prefix);
    printf("%ssignal.print.help\n", prefix);

    printf("%sreference_stations.help\n", prefix);
    printf("%sprint.help\n", prefix);
}

void FK::setButtonsSensitive(void)
{
    if(!data_source) return;
    gvector<Waveform *> ws;
    AxesCursorCallbackStruct *a;

    bool selected = (data_source->getSelectedWaveforms(ws) > 0);
    bool slowness_cursor = getBeamCursor(&a);

    if(rtd_mode == RTD_OFF) {
	compute_button->setSensitive(selected);
	auto_compute_button->setSensitive(selected);
    }
    beam_button->setSensitive(selected & slowness_cursor);
}

void FK::computeButtonsOn(bool on)
{
    if(on && rtd_mode != RTD_OFF) return;

    compute_button->setSensitive(on);
    auto_compute_button->setSensitive(on);
    if(signal_window) {
	signal_window->auto_compute_button->setSensitive(on);
    }
}

void FK::setVisible(bool visible)
{
    Frame::setVisible(visible);   
    setButtonsSensitive();

    if(visible) {
//	plot1->unzoomAll();
	return;
    }

    if(auto_cursor_toggle->state()) {
	auto_cursor_toggle->set(false, false);
    }

    if(wp) {
	wp->removeActionListener(this, XtNdoubleLineDragCallback);
	wp->removeActionListener(this, XtNdoubleLineCallback);
    }

    if(rtd_compute_toggle->state()) {
	rtd_compute_toggle->set(false, true);
    }
    if(p->single_fk != NULL) {
	p->single_fk->deleteObject();
	p->single_fk = NULL;
    }
    if(p->fg) {
	p->fg->deleteObject();
	p->fg = NULL;
    }
    if(p->fg3C) {
	p->fg3C->deleteObject();
	p->fg3C = NULL;
    }
    p->freeSpace();
}

void FK::rtdContinue(void)
{
    FKData *fk = NULL;

    if(count >= 0 && count < p->num_fkdata) {
	fk = p->fkdata[count];
    }

    if( addToGram() && !auto_display ) {
	double shift;
	count = p->num_fkdata-1;
	if(fk) {
	    // find the current fk in the new array.
	    int i;
            for(i = 0; i < p->num_fkdata && fk != p->fkdata[i]; i++);
	    if(i < p->num_fkdata) {
		count = i;
	    }
	}
	drawFK(true);

	shift = getDataShift(p->fkdata[count]);

	if(wp) {
	    wp->positionDoubleLine("b", p->fkdata[count]->tbeg - shift,
				p->fkdata[count]->tend - shift, false);
	}
	beam_plot->positionDoubleLine("a", p->fkdata[count]->tbeg,
				p->fkdata[count]->tend, false);
	signal_window->beam_plot->positionDoubleLine("a",p->fkdata[count]->tbeg,
				p->fkdata[count]->tend, false);
    }
}

int FK::addToGram(void)
{
    bool dis_data[2], dis_grid[2], scan_freqs;
    gvector<Waveform *> ws;
    int	num_bands, nwaveforms, new_fks=0;
    double slowness_max, tmin=0., bandwidth;
    double dt, window_length, window_overlap;
    double signal_slow_min, signal_slow_max, signal_az_min, signal_az_max;
    int	i, n_slowness;
    int	taper;
    double beg_taper, end_taper, stav_len, ltav_len;

    if(!isVisible()) return 0;

    if(!getBands()) return 0;
    num_bands = p->nbands;

    if(num_bands == 0) return 0;

    if(!(nwaveforms = addToGramGetWaveforms(ws)))
    {
	return 0;
    }
    dt = ws[0]->segment(0)->tdel();

    stringcpy(p->net, ws[0]->net(), sizeof(p->net));

    if(!allGetParam(num_bands, dt, &slowness_max, &n_slowness,
		dis_data, dis_grid, &window_length, &window_overlap,
		&signal_slow_min, &signal_slow_max, &signal_az_min,
		&signal_az_max, &taper, &beg_taper, &end_taper, &stav_len,
		&ltav_len, &scan_freqs, &bandwidth))
    {
	return 0;
    }

    setCursor("hourglass");
    StatusDialog::setParent(this);
    computing_fk = this;

    try {
	if(!p->three_component) {
	    if(!p->fg) {
		p->fg = new FKGram(ws, 0, slowness_max, n_slowness,
			num_bands, fmin, fmax, window_length, window_overlap,
			workingUpdate, signal_slow_min, signal_slow_max,
			signal_az_min, signal_az_max, taper, beg_taper,
			end_taper, scan_freqs, bandwidth);
		new_fks = p->fg->num_fks;
	    }
	    else {
		new_fks = p->fg->compute(ws, true, rtd_save_secs);
	    }
	    p->num_fkdata = p->fg->num_fks;
	    p->fkdata = p->fg->fkdata;
	}
	else {
	    if(!p->fg3C) {
		p->fg3C = new FKGram3C(ws, 0, slowness_max,
			n_slowness, num_bands, fmin, fmax, window_length,
			window_overlap, workingUpdate, taper, beg_taper,
			end_taper);
		new_fks = p->fg3C->num_fks;
	    }
	    else {
		new_fks = p->fg3C->recompute(ws);
	    }
	    p->num_fkdata = p->fg3C->num_fks;
	    p->fkdata = p->fg3C->fkdata;
	}
    }
    catch (int n) {
	showWarning(GError::getMessage());
	p->num_fkdata = 0;
    }

    setCursor("default");

    if(p->num_fkdata <= 0) {
	return 0;
    }

    if(nwaveforms > 0) tmin = ws[0]->tbeg();
    for(i = 1; i < nwaveforms; i++) {
	double t0 = ws[i]->tbeg();
	if(t0 < tmin) tmin = t0;
    }

    p->window_length = window_length;
    p->window_overlap = window_overlap;
    display_data[0] = dis_data[0];
    display_data[1] = dis_data[1];
    display_grid[0] = dis_grid[0];
    display_grid[1] = dis_grid[1];

    if(new_fks) {
	p->findMinMax();

	bool unwrapAz = signal_window->unwrap_toggle->state();
	BeamLocation beam_location;
	if(dnorth_deast_toggle->state()) beam_location = DNORTH_DEAST;
	else if(ref_sta_toggle->state()) beam_location = REFERENCE_STATION;
	else beam_location = ARRAY_CENTER;

	p->updateSignalMeasurements(data_source, unwrapAz, beam_location,
			stav_len, ltav_len, wvec);
	updateSignalMeasurements();
    }

    setGramButtonsOn(true);

    return new_fks;
}

int FK::addToGramGetWaveforms(gvector<Waveform *> &wlist)
{
    int i, j;
    gvector<Waveform *> ws;

    wlist.clear();
    if(!data_source) return 0;

    if(p->beam_input) {
	return rtd_data_source->getBeamElements(rtd_sta, rtd_chan, wlist);
    }

// for data sources that do not own the GTimeSeries objects, we
// must own them before the call to FKComputeGram, or they will be freed in
// Coverage.getArrays

    data_source->getWaveforms(ws, false);

    for(i = 0; i < p->num_waveforms; i++) {
	for(j = 0; j < ws.size() && p->waveform_ids[i] !=ws[j]->getId(); j++);
	if(j < ws.size()) {
	    wlist.push_back(ws[j]);
	}
    }
    return wlist.size();
}

void FK::updateSignalMeasurements(void)
{
    Pixel fg;
    char label[100];
    int j1, m, n = p->num_fkdata;
    float *y;
    Arg args[1];

    if(!p->fkdata) return;

    signal_window->clear();

    if(n <= 0) return;

    y = (float *)malloc(n*sizeof(float));

    for(int i = 0; i < p->nbands; i++)
    {
	XtSetArg(args[0], XtNdataColor, &fg);
	conplot[i]->getValues(args, 1);

	snprintf(label, sizeof(label), "%.1f-%.1f", p->fkdata[0]->args.fmin[i],
                        p->fkdata[0]->args.fmax[i]);

	for(j1 = 0; j1 < n; j1++) {
	    if(p->sig[i].snr[j1] >= 0.) break;
	}
	if(j1 < n) {
	    m = n - j1;
	    for(int j = 0; j < m; j++) {
		if(p->sig[i].snr[j1+j] >= 0.) {
		    y[j] = p->sig[i].snr[j1+j];
		}
		else {
		    set_fnan(y[j]);
		}
	    }
	    signal_window->addCurve(0, m, p->x+j1, y, label, fg);
	}
	signal_window->addCurve(1, n, p->x, p->sig[i].fstat, label, fg);
	signal_window->addCurve(2, n, p->x, p->sig[i].appvel, label, fg);
	signal_window->addCurve(3, n, p->x, p->sig[i].slowness, label, fg);
	signal_window->addCurve(4, n, p->x, p->sig[i].azimuth, label, fg);
	signal_window->addCurve(5, n, p->x, p->sig[i].delaz, label, fg);
	signal_window->addCurve(6, n, p->x, p->sig[i].delslo, label,fg);
    }

    free(y);
}

void FK::setGramButtonsOn(bool on)
{
    tool_bar->changeLabel("Start", "Start");
    start_button->setSensitive(on);
    back_button->setSensitive(on);
    forward_button->setSensitive(on);

    if(signal_window) {
	signal_window->start_button->setLabel("Start");
	signal_window->back_button->setSensitive(on);
	signal_window->forward_button->setSensitive(on);
    }
}

void FK::tieLimits(Component *comp, AxesLimitsCallbackStruct *s)
{
    ConPlotClass *plots[3];
    if(comp == plot1) {
	plots[0] = plot2; plots[1] = plot3; plots[2] = plot4;
    }
    else if(comp == plot2) {
	plots[0] = plot1; plots[1] = plot3; plots[2] = plot4;
    }
    else if(comp == plot3) {
	plots[0] = plot1; plots[1] = plot2; plots[2] = plot4;
    }
    else if(comp == plot4) {
	plots[0] = plot1; plots[1] = plot2; plots[2] = plot3;
    }
    else return;

    ignore_callback = true;
    for(int i = 0; i < 3; i++) {
	if(s->x_margins || s->y_margins) {
	    plots[i]->setMargins(true,true,s->left,s->right,s->top,s->bottom);
        }
	plots[i]->setLimits(s->x_min, s->x_max, s->y_min, s->y_max);
    }
    ignore_callback = false;
}

bool FK::getBands(void)
{
    int nrows;
    char name[100];
    vector<const char *> f1, f2;

    table->getColumn(0, f1);
    table->getColumn(1, f2);
    nrows = table->numRows();
    if(nrows > 4) nrows = 4;
    p->nbands = nrows;

    conplot[0] = plot1;
    conplot[1] = plot2;
    conplot[2] = plot3;
    conplot[3] = plot4;

    for(int i = 0; i < nrows; i++)
    {
	if(!stringToDouble(f1[i], &fmin[i])) {
	    showWarning("FK: invalid minimum frequency: %s", f1[i]);
	    return false;
	}
	if(!stringToDouble(f2[i], &fmax[i])) {
	    showWarning("FK: invalid maximum frequency: %s", f2[i]);
	    return false;
	}
	snprintf(name, sizeof(name), "fkmin%d", i+1);
	putProperty(name, f1[i], false);
	snprintf(name, sizeof(name), "fkmax%d", i+1);
	putProperty(name, f2[i], false);
    }
    return true;
}

void FK::print(void)
{
    if(print_window == NULL) {
        print_window = new PrintDialog("Print FK", this, this);
    }
    print_window->setVisible(true);
}

void FK::print(FILE *fp, PrintParam *ps)
{
    char title[200], tlab[100], phase_lab[100], label[100];
    AxesParm *a;
    double az, slow, length, font_scale, title_font_height;
    double label_font_height;
    FKData *fk = NULL;
    Arg args[1];

    tlab[0] = '\0';
    if(show_single) {
	if(p->single_fk != NULL) {
	    timeEpochToString(p->single_fk->tbeg, tlab, 100, GSE20);
	    fk = p->single_fk;
	}
    }
    else if(count >= 0 && count < p->num_fkdata)
    {
	FKData *f = p->fkdata[count];
	timeEpochToString(f->tbeg, tlab, 100, GSE20);
	fk = p->fkdata[count];
    }
    if(fk == NULL) {
	showWarning("Error in FK:print: fk = NULL.");
	return;
    }

    getPhase(fk, phase_lab, sizeof(phase_lab));

    font_scale = 1./POINTS_PER_DOT;
    title_font_height = DOTS_PER_INCH*ps->fonts.title_fontsize/72.;
    label_font_height = DOTS_PER_INCH*ps->fonts.label_fontsize/72.;

    if(p->nbands == 1)
    {
	int x, y;

//	getAzSlow(&az, &slow);
	vector<const char *> row;
	table->getRow(0, row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);

	ps->top_title_user = True;
	ps->top_title_lines = 4;
	ps->top_title = NULL;

	a = plot1->hardCopy(fp, ps, NULL);
	free(a);

	x = ps->top_title_x;
	y = ps->top_title_y;
	fprintf(fp, "%d %d M\n", x, y);
	length = fk->tend - fk->tbeg;
	fprintf(fp, "(%s  %s  window %.2f s) %.5f ",
	    p->net, tlab, length, font_scale);
	fprintf(fp, "dup dup scale exch dup stringwidth pop 2 div neg\n");
	fprintf(fp, "dup 0 rmoveto exch show exch 1 exch div dup scale\n");
	
	y -= (int)(1.1*title_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "dup 0 rmoveto\n");
	if(phase_lab[0] != '\0') {
	    snprintf(title, 200, "(%s Slowness: %.4f s/deg  Azimuth: %.1f deg)",
		phase_lab, slow*DEG_TO_KM, az);
	}
	else {
	    snprintf(title, 200,"(Slowness: %.4f s/deg  Azimuth: %.1f deg)",
		slow*DEG_TO_KM, az);
	}
	fprintf(fp, "%s show\n", title);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);

	y -= (int)(1.1*title_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "dup 0 rmoveto\n");
	if(phase_lab[0] != '\0') fprintf(fp, "(%s Slowness: ) ", phase_lab);
	else fprintf(fp, "(Slowness: ) ");
	fprintf(fp,"stringwidth pop 0 rmoveto\n");
	fprintf(fp, "(%.4f s/km  Apparent Velocity: %.3f km/s) show\n",
		slow, (slow != 0.) ? 1./slow : 0.);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);

	y -= (int)(1.1*title_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "0 rmoveto\n");
	fprintf(fp, "(Frequencies: %.1f to %.1f  samples: %d) show\n", 
		    fk->args.fmin[0], fk->args.fmax[0], fk->args.num_slowness);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);
    }
    else
    {
	int x, y;
	double left, right, top, bottom, margin;
	vector<const char *> row;

	if(ps->portrait) {
	    margin = 0;
	}
	else {
	    margin =  label_font_height/DOTS_PER_INCH;
	}
	ps->x_axis_label = NULL;
	ps->center = True;
	left = ps->left;
	right = ps->right;
	top = ps->top;
	bottom = ps->bottom;
	
	table->getRow(0, row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);
	snprintf(title, 200, "Slowness: %.4f s/deg  Azimuth: %.1f deg",
		slow*DEG_TO_KM, az);

	ps->top = top - margin;
	ps->right = .5*(left + right);
	ps->bottom = .5*(bottom + top) + 2*margin;
	snprintf(label, sizeof(label), "Frequencies %0.2f to %0.2f", 
		fmin[0], fmax[0]);
	XtSetArg(args[0], XtNtitle, label);
	plot1->setValues(args, 1);
	a = plot1->hardCopy(fp, ps, NULL);
	free(a);
	XtSetArg(args[0], XtNtitle, "");
	plot1->setValues(args, 1);

	x = (int)(ps->x_axis_x - .35*DOTS_PER_INCH);
	y = ps->x_axis_y;
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		ps->fonts.label_font, ps->fonts.label_fontsize);
 	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "(%s) %.5f PC\n", title, font_scale);
	y -= (int)(1.1*label_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "(%s) stringwidth pop 2 div neg 0 rmoveto\n", title);
	fprintf(fp, "(Slowness: )  stringwidth pop 0 rmoveto\n");
	fprintf(fp, "(%.4f s/km  App Vel: %.3f km/s) show\n",
		slow, (slow != 0.) ? 1./slow : 0.);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);

	table->getRow(1, row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);
	snprintf(title, 200, "Slowness: %.4f s/deg Azimuth: %.1fdeg",
		slow*DEG_TO_KM, az);

	ps->left = .5*(left + right);
	ps->right = right;
	ps->bottom = .5*(bottom + top) + 2*margin;
	snprintf(label, sizeof(label), "Frequencies %0.2f to %0.2f", 
		fmin[1], fmax[1]);
	XtSetArg(args[0], XtNtitle, label);
	plot2->setValues(args, 1);
	a = plot2->hardCopy(fp, ps, NULL);
	free(a);
	XtSetArg(args[0], XtNtitle, "");
	plot2->setValues(args, 1);

	x = (int)(ps->x_axis_x - .35*DOTS_PER_INCH);
	y = ps->x_axis_y;
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		ps->fonts.label_font, ps->fonts.label_fontsize);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "(%s) %.5f PC\n", title, font_scale);
	y = ps->x_axis_y;
	y -= (int)(1.1*label_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "(%s) stringwidth pop 2 div neg 0 rmoveto\n", title);
	fprintf(fp, "(Slowness: )  stringwidth pop 0 rmoveto\n");
	fprintf(fp, "(%.4f s/km  App Vel: %.3f km/s) show\n",
			slow, (slow != 0.) ? 1./slow : 0.);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);

	table->getRow(2, row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);
	snprintf(title, 200, "Slowness: %.4f s/deg Azimuth: %.1fdeg",
			slow*DEG_TO_KM, az);

	ps->left = left;
	ps->right = .5*(left + right);
	ps->bottom = bottom + 2*margin;
	ps->top = .5*(bottom + top) - margin;
	snprintf(label, sizeof(label), "Frequencies %0.2f to %0.2f", 
		fmin[2], fmax[2]);
	XtSetArg(args[0], XtNtitle, label);
	plot3->setValues(args, 1);
	a = plot3->hardCopy(fp, ps, NULL);
	free(a);
	XtSetArg(args[0], XtNtitle, "");
	plot3->setValues(args, 1);

	x = (int)(ps->x_axis_x - .35*DOTS_PER_INCH);
	y = ps->x_axis_y;
        fprintf(fp, "/%s findfont %d scalefont setfont\n",
		ps->fonts.label_font, ps->fonts.label_fontsize);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "(%s) %.5f PC\n", title, font_scale);
	y = ps->x_axis_y;
	y -= (int)(1.1*label_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "(%s) stringwidth pop 2 div neg 0 rmoveto\n", title);
	fprintf(fp, "(Slowness: )  stringwidth pop 0 rmoveto\n");
	fprintf(fp, "(%.4f s/km  App Vel: %.3f km/s) show\n",
			slow, (slow != 0.) ? 1./slow : 0.);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);

	table->getRow(3, row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);
	snprintf(title, 200, "Slowness: %.4f s/deg Azimuth: %.1fdeg",
			slow*DEG_TO_KM, az);

	ps->left = .5*(left + right);
	ps->right = right;
	ps->bottom = bottom + 2*margin;
	ps->top = .5*(bottom + top) - margin;
	snprintf(label, sizeof(label), "Frequencies %0.2f to %0.2f", 
		fmin[3], fmax[3]);
	XtSetArg(args[0], XtNtitle, label);
	plot4->setValues(args, 1);
	a = plot4->hardCopy(fp, ps, NULL);
	free(a);
	XtSetArg(args[0], XtNtitle, "");
	plot4->setValues(args, 1);

	x = (int)(ps->x_axis_x - .35*DOTS_PER_INCH);
	y = ps->x_axis_y;
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		ps->fonts.label_font, ps->fonts.label_fontsize);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "(%s) %.5f PC\n", title, font_scale);
	y = ps->x_axis_y;
	y -= (int)(1.1*label_font_height);
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "%.5f %.5f scale\n", font_scale, font_scale);
	fprintf(fp, "(%s) stringwidth pop 2 div neg 0 rmoveto\n", title);
	fprintf(fp, "(Slowness: )  stringwidth pop 0 rmoveto\n");
	fprintf(fp, "(%.4f s/km  App Vel: %.3f km/s) show\n",
			slow, (slow != 0.) ? 1./slow : 0.);
	fprintf(fp, "%.5f %.5f scale\n", 1./font_scale, 1./font_scale);

	x = (int)(.5*(left + right)*DOTS_PER_INCH);
	y = (int)(top*DOTS_PER_INCH);
	snprintf(title, 200, "%s  %s", p->net, tlab);
	fprintf(fp, "/%s findfont %d scalefont setfont\n",
		ps->fonts.label_font, ps->fonts.label_fontsize);
	fprintf(fp, "%d %d M\n", x, y);
	length = fk->tend - fk->tbeg;
	fprintf(fp, "(%s  window %.2f s) %.5f PC\n", title, length, font_scale);

	y -= (int)(1.1*DOTS_PER_INCH*ps->fonts.title_fontsize/72.);
	if(phase_lab[0] != '\0') {
	    snprintf(title, 200, "Phase: %s  Slowness samples: %d\n",
			phase_lab, fk->args.num_slowness);
	}
	else {
	    snprintf(title, 200,"Slowness samples: %d\n",fk->args.num_slowness);
	}
	fprintf(fp, "%d %d M\n", x, y);
	fprintf(fp, "(%s) %.5f PC\n", title, font_scale);
    }
}

void FK::getPhase(FKData *fk, char *phase_lab, int len)
{
    int i, j;
    Waveform *w;

    phase_lab[0] = '\0';
    if(fk == NULL || !wp) return;

    cvector<CssArrivalClass> &arrs = *wp->getArrivalRef();
    phase_lab[0] = '\0';
    for(i = 0; i < arrs.size(); i++) if(wp->isSelected(arrs[i]))
    {
	for(j = 0; j < fk->nwaveforms && phase_lab[0] == '\0'; j++)
	{
	    if((w = wp->getWaveform(fk->waveformId(j))) != NULL &&
		!strcmp(w->net(), arrs[i]->sta) &&
		!strcmp(w->chan(), arrs[i]->chan) &&
		fk->tbeg <= arrs[i]->time && arrs[i]->time <= fk->tend)
	    {
		stringcpy(phase_lab, arrs[i]->iphase, len);
		break;
	    }
	}
    }
}

void FK::showRefSta(bool visible)
{
    gvector<Waveform *> ws;
    char net_name[10];
    DataSource *ds=NULL;
    data_source->getSelectedWaveforms(ws);

    for(int i = 0; i < ws.size(); i++) {
	if((ds = ws[i]->getDataSource())) {
	    stringcpy(net_name, ws[i]->net(), sizeof(net_name));
	    break;
	}
    }
    if(!ds) {
	showWarning("No data source found for the selected waveforms.");
	return;
    }
    if(!ref_sta_window) {
	ref_sta_window = new RefSta("Reference Stations", this,
					net_name, ds);
    }
    else {
	vector<int> v;
	v.push_back(stringUpperToQuark(net_name));
	ref_sta_window->setDataSource(ds, v);
    }
    if(visible) ref_sta_window->setVisible(true);
    ref_sta_window->setDataSource(ds);
}

void FK::tableEdit(MmTableEditCallbackStruct *c)
{
    char str[100];
    double app_vel, azimuth, sec_per_km;
    double az, xmax, ymax;
    vector<const char *> row;

    if(c->column < 2 || c->column > 4) {
	return;
    }
    table->getRow(c->row, row);

    tableGetAzSlow(p->fk_units, row[3], row[4], &azimuth, &sec_per_km);

    if(azimuth < -9999. || sec_per_km < -9999.) {
	return;
    }

    if(c->column == 2) {
	if(stringToDouble(row[2], &app_vel) && app_vel > 0.) {
	    sec_per_km = 1./app_vel;
	    if(p->fk_units == FK_SEC_PER_KM) {
		ftoa(sec_per_km, 4, 0, str, 100);
	    }
	    else {
		ftoa(DEG_TO_KM*sec_per_km, 3, 0, str, 100);
	    }
	    table->setField(c->row, 3, str, false);
	}
	else {
	    table->setField(c->row, 3, "inf", false);
	}
    }
    else if(c->column == 3) {
	if(sec_per_km > 0.) {
	    app_vel = 1./sec_per_km;
	    ftoa(app_vel, 3, 0, str, 100);
	    table->setField(c->row, 2, str, false);
	}
	else {
	    table->setField(c->row, 2, "inf", false);
	}
    }

    /* move cursor to proper az */
    az = azimuth*M_PI/180.;
    if(p->fk_units == FK_SEC_PER_KM) {
	xmax = sec_per_km * sin(az);
	ymax = sec_per_km * cos(az);
    }
    else {
	xmax = DEG_TO_KM*sec_per_km * sin(az);
	ymax = DEG_TO_KM*sec_per_km * cos(az);
    }

    conplot[c->row]->positionCrosshair(0, xmax, ymax, false);

    crosshairAction(sec_per_km, azimuth, 0, c->row);

    ignore_callback = false;
}

void FK::tableGetAzSlow(int fk_units, const char *slow_text,const char *az_text,
		double *azimuth, double *sec_per_km)
{
    double az, slow;

    *azimuth = -9999.9;
    *sec_per_km = -9999.9;

    if(stringToDouble(slow_text, &slow)) {
	*sec_per_km = (fk_units == FK_SEC_PER_KM) ?  slow : slow/DEG_TO_KM;
    }

    if(stringToDouble(az_text, &az)) {
	*azimuth = az;
    }
}

void FK::setLabels()
{
    Arg args[2];

    if(p->fk_units == FK_SEC_PER_KM)
    {
	if(nbands == 1) {
	    XtSetArg(args[0], XtNxLabel, "Sx (s/km)");
	    XtSetArg(args[1], XtNyLabel, "Sy (s/km)");
	    plot1->setValues(args, 2);
	}
	table->setColumnLabel(3, "Slowness(s/km)");
	units_km_toggle->set(true);
	units_deg_toggle->set(false);
    }
    else
    {
	if(nbands == 1) {
	    XtSetArg(args[0], XtNxLabel, "Sx (s/deg)");
	    XtSetArg(args[1], XtNyLabel, "Sy (s/deg)");
	    plot1->setValues(args, 2);
	}
	table->setColumnLabel(3, "Slowness(s/deg)");
	units_km_toggle->set(false);
	units_deg_toggle->set(true);
    }
}

void FK::changeUnits(void)
{
    double slow;

    ignore_callback = 1;

    if(units_deg_toggle->state() && p->fk_units == FK_SEC_PER_KM)
    {
	Arg args[2];
	p->fk_units = FK_SEC_PER_DEG;
	table->setColumnLabel(3, "Slowness(s/deg)");
	if(p->nbands == 1)
	{
	    XtSetArg(args[0], XtNxLabel, "Sx (s/deg)");
	    XtSetArg(args[1], XtNyLabel, "Sy (s/deg)");
	    plot1->setValues(args, 2);
	}

	/* change any value already there */
	if(parameter_window->s_max_text->getDouble(&slow)) {
	    slow *= DEG_TO_KM;
	}
	else {
	    slow = .36*DEG_TO_KM;
	}
	parameter_window->slowMaxLabel()->setLabel("slowness max (s/deg)");
	parameter_window->s_max_text->setString("%lf", slow);
    }
    else if(units_km_toggle->state() && p->fk_units == FK_SEC_PER_DEG)
    {
	Arg args[2];
	p->fk_units = FK_SEC_PER_KM;
	table->setColumnLabel(3, "Slowness(s/km)");
	if(p->nbands == 1)
	{
	    XtSetArg(args[0], XtNxLabel, "Sx (s/km)");
	    XtSetArg(args[1], XtNyLabel, "Sy (s/km)");
	    plot1->setValues(args, 2);
	}

	/* change any value already there */

	if(parameter_window->s_max_text->getDouble(&slow)) {
	    slow /= DEG_TO_KM;
	}
	else {
	    slow = .36;
	}
	parameter_window->slowMaxLabel()->setLabel("slowness max (s/km)");
	parameter_window->s_max_text->setString("%lf", slow);
    }

    ignore_callback = 0;
    compute(false, false);
}

void FK::wideGrid(void)
{
    double slow;

    if(!wide_grid_toggle) return;

    if(wide_grid_toggle->state()) {
	slow = (p->fk_units == FK_SEC_PER_KM) ? .50 : .50*DEG_TO_KM;
    }
    else {
	slow = (p->fk_units == FK_SEC_PER_KM) ? .36 : .36*DEG_TO_KM;
    }
    parameter_window->s_max_text->setString("%lf", slow);

    compute(false, true);
}

void FK::grid(void)
{
    bool dis_data[2], dis_grid[2];

    if(grid_toggle->state() && fine_grid_toggle->state()) {
	fine_grid_toggle->set(false);
    }
    dis_data[0] = !fine_grid_toggle->state();
    dis_data[1] = fine_grid_toggle->state();
    dis_grid[0] = grid_toggle->state();
    dis_grid[1] = fine_grid_toggle->state();

    for(int i = 0; i < p->nbands; i++) {
	conplot[i]->display(2, ids+2*i, dis_data, dis_grid);
    }
    display_grid[0] = dis_grid[0];
    display_grid[1] = dis_grid[1];
}

void FK::fineGrid(void)
{
    bool dis_data[2], dis_grid[2];

    if(grid_toggle->state() && fine_grid_toggle->state()) {
	grid_toggle->set(false);
    }
    dis_data[0] = !fine_grid_toggle->state();
    dis_data[1] = fine_grid_toggle->state();
    dis_grid[0] = grid_toggle->state();
    dis_grid[1] = fine_grid_toggle->state();

    for(int i = 0; i < p->nbands; i++) {
	conplot[i]->display(2, ids+2*i, dis_data, dis_grid);
    }
}

void FK::autoStart(void)
{
    if(auto_display) {   // already running, stop.
	stopDisplay();
    }
    else if(p->num_fkdata > 0) { // not running, start it.
	startDisplay();
    }
}

void FK::autoStep(bool forward)
{
    FKData **fkdata = p->fkdata;
    double shift;
    int k;

    if(p->num_fkdata > 0)
    {
	if(forward) {
	    count++;
	    if(count >= p->num_fkdata) {
		count = 0;
	    }
	}
	else {
	    count--;
	    if(count < 0) {
		count = p->num_fkdata-1;
	    }
	}
	drawFK(true);
	k = count;
	shift = getDataShift(fkdata[k]);

	if(wp) {
	    wp->positionDoubleLine("b", fkdata[k]->tbeg - shift,
			fkdata[k]->tend - shift, false);
	}
    }
}

void FK::display(int i)
{
    if(p->num_fkdata <= 0) {
	cerr << "num fkdata == 0" << endl;
	return;
    }
    if(i <= 0 || i > p->num_fkdata) {
	cerr << "display: Invalid fk index: "<< i << endl;
	return;
    }
    count = i-1;
    drawFK(true);
    int k = count;
    double shift = getDataShift(p->fkdata[k]);

    if(wp) {
	wp->positionDoubleLine("b", p->fkdata[k]->tbeg - shift,
                        p->fkdata[k]->tend - shift, false);
    }
}

double FK::getDataShift(FKData *fk)
{
    Waveform *w;

    if(fk->nwaveforms > 0 && data_source &&
		(w = data_source->getWaveform(fk->waveformId(0))) != NULL)
    {
	return w->tbeg() - w->scaled_x0;
    }
    else {
	return fk->shift;
    }
}

void FK::stopDisplay(void)
{
    auto_display = false;

    tool_bar->changeLabel("Start", "Start");
    start_button->setSensitive(true);
    back_button->setSensitive(true);
    forward_button->setSensitive(true);
    menu_bar->setSensitive(true);
    close_button->setSensitive(true);

    if(rtd_mode == RTD_OFF) {
	compute_button->setSensitive(true);
	auto_compute_button->setSensitive(true);
    }
    beam_button->setSensitive(true);

    if(signal_window) {
	signal_window->start_button->setLabel("Start");
	signal_window->back_button->setSensitive(true);
	signal_window->forward_button->setSensitive(true);
    }
}

void FK::startDisplay(void)
{
    auto_display = true;

    start_button->setSensitive(true);
    tool_bar->changeLabel("Start", "Stop");
    back_button->setSensitive(false);
    forward_button->setSensitive(false);
    menu_bar->setSensitive(false);
    close_button->setSensitive(false);
    compute_button->setSensitive(false);
    auto_compute_button->setSensitive(false);
    beam_button->setSensitive(false);

    if(signal_window) {
	signal_window->start_button->setLabel("Stop");
	signal_window->start_button->setSensitive(true);
	signal_window->back_button->setSensitive(false);
	signal_window->forward_button->setSensitive(false);
    }

    XtAppAddTimeOut(Application::getApplication()->appContext(), 100,
		(XtTimerCallbackProc)nextFK, (XtPointer)this);
}

//static
void FK::nextFK(XtPointer client_data, XtIntervalId id)
{
    FK *fk = (FK *)client_data;
    FKData **fkdata = fk->p->fkdata;
    double shift;
    int k;

    if(fk->auto_display)
    {
	fk->drawFK(true);
	k = fk->count;

	shift = fk->getDataShift(fkdata[k]);

	if(fk->wp) {
	    fk->wp->positionDoubleLine("b", fkdata[k]->tbeg - shift,
				fkdata[k]->tend - shift, false);
	}
	fk->beam_plot->positionDoubleLine("a", fkdata[k]->tbeg,
				fkdata[k]->tend, false);
	fk->signal_window->beam_plot->positionDoubleLine("a", fkdata[k]->tbeg,
				fkdata[k]->tend, false);
	fk->count++;

	if(fk->count < fk->p->num_fkdata) {
	    XtAppAddTimeOut(Application::getApplication()->appContext(), 100,
			(XtTimerCallbackProc)nextFK, client_data);
	}
	else {
	    fk->stopDisplay();
	}
    }
}

void FK::displayFirst(void)
{
    start_button->setSensitive(true);
    back_button->setSensitive(true);
    forward_button->setSensitive(true);

    if(signal_window) {
	signal_window->start_button->setSensitive(true);
	signal_window->back_button->setSensitive(true);
	signal_window->forward_button->setSensitive(true);
    }

    FKData **fkdata = p->fkdata;
    double shift;
    int k;

    drawFK(true);
    k = 0;
    shift = getDataShift(fkdata[k]);
    if(wp) {
	wp->positionDoubleLine("b", fkdata[k]->tbeg - shift,
				fkdata[k]->tend - shift, false);
    }
    count++;
}

bool FK::compute(bool show_warning, bool redisplay)
{
    bool dis_data[2], dis_grid[2];
    int	i, windowed, num_bands, n_slowness;
    double slowness_max, tmin = 0., tmax = 0.;
    double signal_slow_min, signal_slow_max;
    double signal_az_min, signal_az_max;
    double beg_taper, end_taper;
    int taper;

    if(ignore_callback) return false;

    if(p->single_fk != NULL) {
	p->single_fk->deleteObject();
	p->single_fk = NULL;
    }
    auto_display = false;

    if(p->nbands == 0) return false;
    num_bands = (p->nbands > 1) ? p->nbands : 1;

    if( !FkGetWaveforms(&windowed, &tmin, &tmax, show_warning) )
    {
	return false;
    }
    gvector<GTimeSeries *> *ts_v = new gvector<GTimeSeries *>(wvec.size());
    for(i = 0; i < wvec.size(); i++) {
	ts_v->add(wvec[i]->ts);
    }

    getParams(&slowness_max, &n_slowness, dis_data, dis_grid, &signal_slow_min,
		&signal_slow_max, &signal_az_min, &signal_az_max,
		&taper, &beg_taper, &end_taper);

    for(i = 0; i < num_bands; i++) {
	conplot[i]->clear();
	ids[2*i] = ids[2*i+1] = 0;
    }
    show_single = false;
 
/*
    if(windowed && wvec[0]->num_dw > 0) {
	dt = wvec[0]->segment(wvec[0]->dw[0].d1->segmentIndex())->tdel();
    }
    else {
	dt = wvec[0]->segment(0)->tdel();
    }
*/

    if(!getBands()) {
	delete ts_v;
	return false;
    }

    setCursor("hourglass");

    try {
	FKArgs args;
	args.slowness_max = slowness_max;
	args.num_slowness = n_slowness;
	args.num_bands = num_bands;
	for(i = 0; i < num_bands; i++) {
	    args.fmin[i] = fmin[i];
	    args.fmax[i] = fmax[i];
	}
	args.signal_slow_min = signal_slow_min;
	args.signal_slow_max = signal_slow_max;
	args.signal_az_min = signal_az_min;
	args.signal_az_max = signal_az_max;
	args.taper_type = taper;
	args.taper_beg = beg_taper;
	args.taper_end = end_taper;
	args.three_component = p->three_component;

	if(tmax > tmin) {
	    p->single_fk = new FKData(wvec, tmin, tmax, args);
	}
	else {
	    p->single_fk = new FKData(wvec, windowed, args);
	}
	if( !p->single_fk )
	{
	    delete ts_v;
	    setCursor("default");
	    return false;
	}
    }
    catch(...) {
	showWarning(GError::getMessage());
	delete ts_v;
	setCursor("default");
	return false;
    }

    setFreqScales(p->single_fk);

    show_single = true;

    beam_plot->clear();
    beam_plot->unzoomAll();

    displayFKs(dis_data, dis_grid, &p->single_fk->fine, wvec[0]->sta(),
		redisplay);

    stringcpy(p->net, wvec[0]->net(), sizeof(p->net));

    delete ts_v;

    display_data[0] = dis_data[0];
    display_data[1] = dis_data[1];
    display_grid[0] = dis_grid[0];
    display_grid[1] = dis_grid[1];

    windowInfo();

    for(i = 0; i < num_bands; i++) fkmax_snr[i] = snr[i];

    setCursor("default");

    setButtonsSensitive();

    return true;
}

void FK::setFreqScales(FKData *fkdata)
{
    bool set = !fkdata->args.three_component;

    for(int i = 0; i < nbands; i++) {
	lo_scale[i]->setSensitive(set);
	hi_scale[i]->setSensitive(set);
    }

    if(fkdata->args.three_component) return;

    int value;
    int nf = fkdata->nt/2;
    double df = 1./((double)fkdata->nt*fkdata->dt), f;
    
    for(int i = 0; i < nbands; i++)
    {
	if(lo_scale[i]->getMaximum() != nf) {
	    lo_scale[i]->setMaximum(nf);
	}
	value = (int)(fkdata->args.fmin[i]/df + .5);
	if(value < 1) value = 1;
	else if(value > nf) value = nf;
	lo_scale[i]->setValue(value);
	f = value*df;
	lo_text[i]->setString("%.2f", f);

	if(hi_scale[i]->getMaximum() != nf) {
	    hi_scale[i]->setMaximum(nf);
	}
	value = (int)(fkdata->args.fmax[i]/df + .5);
	if(value < 1) value = 1;
	else if(value > nf) value = nf;
	hi_scale[i]->setValue(value);
	f = value*df;
	hi_text[i]->setString("%.2f", f);
    }
}

void FK::displayFKs(bool *displaydata, bool *displaygrid, FKFine *fine,
			const char *sta, bool redisplay)
{
    double d_slowness, xmax[4], ymax[4];
    double *slow_x = NULL, *slow_y = NULL;
    int i, b;
    FKData *fk = p->single_fk;

    for(b = 0; b < fk->args.num_bands; b++) {
	xmax[b] = fk->xmax[b];
	ymax[b] = fk->ymax[b];
    }
    double slowness_max = fk->args.slowness_max;

    d_slowness = 2*slowness_max/(double)(fk->args.num_slowness-1);

    if(p->fk_units == FK_SEC_PER_DEG) {
	slowness_max *= DEG_TO_KM;
	d_slowness = 2*slowness_max/(double)(fk->args.num_slowness-1);
	for(b = 0; b < nbands; b++) {
	    xmax[b] *= DEG_TO_KM;
	    ymax[b] *= DEG_TO_KM;

	    if(fine->n_fine[b] > 0)
	    {
		double slowfine_max = fine->slowfine_xmin[b]
			+ (fine->n_fine[b]-1)*fine->d_slowfine[b];
		slowfine_max *= DEG_TO_KM;
		fine->slowfine_xmin[b] *= DEG_TO_KM;
		fine->slowfine_ymin[b] *= DEG_TO_KM;
		fine->d_slowfine[b] = (slowfine_max - fine->slowfine_xmin[b])/
					(double)(fine->n_fine[b]-1);
	    }
	}
    }

    Free(slowness);
    if(!(slowness =(double *)mallocWarn(fk->args.num_slowness*sizeof(double))))
	return;

    for(i = 0; i < fk->args.num_slowness; i++) {
	slowness[i] = -slowness_max + i*d_slowness;
    }
    if(nbands == 4) {
	plot1->removeActionListener(this, XtNlimitsCallback);
	plot2->removeActionListener(this, XtNlimitsCallback);
	plot3->removeActionListener(this, XtNlimitsCallback);
	plot4->removeActionListener(this, XtNlimitsCallback);
    }
    for(b = 0; b < fk->args.num_bands; b++)
    {
	float *f_k = fk->fk[b];

	ids[b*2] = conplot[b]->input(sta, fk->args.num_slowness,
			fk->args.num_slowness, slowness, slowness, f_k, -1.,
			0, NULL, xmax[b], ymax[b], false, false, redisplay,
			false, 0);
	float fk_max = f_k[0];
	float fk_min = f_k[0];
	for(i = 0; i < fk->args.num_slowness*fk->args.num_slowness; i++)
	{
	    if(fk_max < f_k[i]) fk_max = f_k[i];
	    if(fk_min > f_k[i]) fk_min = f_k[i];
	}
	fkmin[b] = fk_min;
	fkmax[b] = fk_max;
    }
    if(nbands == 4) {
	plot1->addActionListener(this, XtNlimitsCallback);
	plot2->addActionListener(this, XtNlimitsCallback);
	plot3->addActionListener(this, XtNlimitsCallback);
	plot4->addActionListener(this, XtNlimitsCallback);
    }

    if(!(slow_x = (double *)mallocWarn(fine->n_slowfine*sizeof(double))))
	    return;
    if(!(slow_y = (double *)mallocWarn(fine->n_slowfine*sizeof(double))))
	    return;

    for(b = 0; b < fk->args.num_bands; b++)
    {
	if(fine->n_fine[b] > 0)
	{
	    for(i = 0; i < fine->n_slowfine; i++)
	    {
		slow_x[i] = fine->slowfine_xmin[b] + i*fine->d_slowfine[b];
		slow_y[i] = fine->slowfine_ymin[b] + i*fine->d_slowfine[b];
	    }
	    ids[b*2+1] = conplot[b]->input(sta, fine->n_slowfine,
			fine->n_slowfine, slow_x, slow_y, fine->fk_fine[b],
			-1., 0, NULL, xmax[b], ymax[b], false, false, false,
			false, 0);
	}

	conplot[b]->display(2, ids+b*2, displaydata, displaygrid);

	conplot[b]->positionCrosshair(0, xmax[b], ymax[b], true);
    }

    Free(slow_x);
    Free(slow_y);

    current_fk = fk;
    doCallbacks(base_widget, (XtPointer)(-1), XmNactivateCallback);
}

int FK::FkGetWaveforms(int *windowed, double *tmin, double *tmax,
			bool show_warning)
{
    int from=0;

    *tmin = *tmax = 0.;
    auto_tbeg = auto_tend = 0.;

    if(!data_source) return 0;
    p->three_component = false;

    wvec.clear();

    if(data_source->dataWindowIsDisplayed("b")) {
	data_source->getSelectedWaveforms("b", wvec);
	from = 1;
    }
    else {
	data_source->getSelectedWaveforms("a", wvec);
	from = 2;
    }
    p->beam_input = false;
    *windowed = wvec.size();

    if(wvec.size() <= 0 && !data_source->dataWindowIsDisplayed("a"))
    {
	*windowed = 0;
	data_source->getSelectedWaveforms(wvec);
	from = 3;
    }
    if(wvec.size() <= 0)
    {
	// only warn if this came from the Compute button
	if(show_warning) {
	    showWarning("No waveforms selected.");
	}
	wvec.clear();
	return 0;
    }
    else if(wvec.size() == 1)
    {
	getArrayElements(wvec, tmin, tmax, show_warning);
	if(wvec.size() > 1) p->beam_input = true;
    }
    else if(wvec.size() == 3)
    {
	vector<int> ncmpts;
	gvector<Waveform *> ws;
	wvec.clear();
	if(from == 1 || from == 2) {
	    const char *cursor = (from == 1) ? "b" : "a";
	    data_source->getSelectedComponents(cursor, ncmpts, ws);
	}
	else {
	    data_source->getSelectedComponents(ncmpts, ws);
	}
	if((int)ncmpts.size() != 1 || ncmpts[0] != 3) {
	    if(show_warning) {
		showWarning(
	    "3 Selected waveforms must be vertical and horizontal components.");
	    }
	    return 0;
	}
	for(int i = 0; i < 3; i++) wvec.push_back(ws[i]);
	p->three_component = true;
    }
    data_source->getTable(arrivals);
    data_source->getTable(assocs);
    return wvec.size();
}

int FK::getArrayElements(gvector<Waveform *> &wlist, double *tmin,
				double *tmax, bool show_warning)
{
    gvector<Waveform *> ws;
    Waveform *w;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    int i, j, nsta;

    *tmin = *tmax = 0.;

    if(!data_source) {
	wlist.clear();
	return 0;
    }

    w = wlist[0];
    wlist.clear();

// For data sources that do not own the GTimeSeries objects, we
// must own them before the call to FKComputeGram, or they will be freed in
// Coverage.getArrays

    if(wp) {
	DataSource *ds = w->getDataSource();
	if( w->ts->array_elements.size() > 0 )
	{
	    if( !w->ts->getValue("elements_loaded") )
	    {
		wp->inputData(&w->ts->array_elements, ds, false);
		w->ts->putValue("elements_loaded", true);
	    }
	}
	else if( ds->getBeamElements(w->sta(), w->chan(), wlist) > 0) {
	    strcpy(rtd_sta, w->sta());
	    strcpy(rtd_chan, w->chan());
	    rtd_data_source = ds;
	    if(w->num_dw > 0 && wlist[0]->num_dw == 0) {
		// this happens when you select a beam and the elements are
		// loaded but not displayed. For example, the RTDisplay.
		*tmin = w->dw[0].d1->time();
		*tmax = w->dw[0].d2->time();
	    }
	    return wlist.size();
	}
    }

    data_source->getWaveforms(ws, false);

    if( !gbeam->getSelectedOriginRecipe(w->net(), recipe) ) {
	if( gbeam->beamRecipe(w->net(), "cb", recipe) ) {
	  putWarning("No %s beam recipe selected. Using cb stations.",w->net());
	}
	else if(show_warning) {
	    showWarning(
		"No origin beam recipe selected for network %s", w->net());
	    return 0;
	}
    }
    if((nsta = Beam::getGroup(recipe, beam_sta)) <= 0)
    {
	if(show_warning) {
	    showWarning(GError::getMessage());
	}
	return 0;
    }

    CssOriginClass *origin = data_source->getPrimaryOrigin(w);

    for(j = 0; j < nsta; j++)
    {
	for(i = 0; i < ws.size(); i++)
	{
	    if( !strcasecmp(ws[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(ws[i]->chan(), beam_sta[j].chan) &&
		    origin == data_source->getPrimaryOrigin(ws[i]))
	    {
		wlist.push_back(ws[i]);
		break;
	    }
	}
    }
    if(w->num_dw > 0) {
	*tmin = w->dw[0].d1->time();
	*tmax = w->dw[0].d2->time();

	// discard waveforms that are completely out of the window.
	gvector<Waveform *> v(wlist);
	wlist.clear();
	for(i = 0; i < v.size(); i++) {
	    if( !(v[i]->tbeg() > *tmax || v[i]->tend() < *tmin) )
	    {
		wlist.push_back(v[i]);
	    }
	}
    }

    return wlist.size();
}

void FK::getParams(double *slowness_max, int *n_slowness,
		bool *dis_data, bool *dis_grid,
		double *signal_slow_min, double *signal_slow_max,
		double *signal_az_min, double *signal_az_max,
		int *taper, double *beg_taper, double *end_taper)
{
    double smax;
    char str[50];

    if(!parameter_window->getDouble("s_max_text", &smax)) {
	smax = (p->fk_units == FK_SEC_PER_KM) ? .36 : .36*DEG_TO_KM;
	ftoa((float)smax, 3, 0, str, sizeof(str));
	parameter_window->setString("s_max_text", str);
    }
    if(p->fk_units == FK_SEC_PER_DEG) smax /= DEG_TO_KM;
    *slowness_max =  smax; // currently always in sec/km

    if(!parameter_window->getInt("num_s_text", n_slowness)) {
	if(nbands == 1) {
	    *n_slowness = 81;
	    parameter_window->setString("num_s_text", "81");
	}
	else {
	    *n_slowness = 61;
	    parameter_window->setString("num_s_text", "61");
	}
    }
    else if(*n_slowness % 2 == 0) {
	(*n_slowness)++; // must be odd
	snprintf(str, sizeof(str), "%d", *n_slowness);
	parameter_window->setString("num_s_text", str);
    }
    *signal_slow_min = -1.;
    *signal_slow_max = -1.;
    *signal_az_min = -1.;
    *signal_az_max = -1.;
    parameter_window->getDouble("slow_min_text", signal_slow_min);
    parameter_window->getDouble("slow_max_text", signal_slow_max);
    parameter_window->getDouble("az_min_text", signal_az_min);
    parameter_window->getDouble("az_max_text", signal_az_max);
    if(p->fk_units == FK_SEC_PER_DEG) {
	if(*signal_slow_min != -1.) *signal_slow_min /= DEG_TO_KM;
	if(*signal_slow_max != -1.) *signal_slow_max /= DEG_TO_KM;
    }

    dis_data[0] = true;
    dis_grid[0] = false;
    if(grid_toggle->state()) {
	dis_grid[0] = true;
    }
    dis_data[1] = false;
    dis_grid[1] = false;

    if(fine_grid_toggle->state()) {
	dis_grid[1] = true;
	dis_data[1] = true;
	dis_data[0] = false;
    }

    returnTaper(taper, beg_taper, end_taper);
}

/*
for a good signal at BRTR, the slowness and azimuth values
   with num s == 20 are
	taper	slowness    azimuth
	hanning	   3.578    133.251  min_slow
	hamming    3.586    132.992
	parzen     3.604    133.242
	cosine     3.844    130.539
	welch      3.630    131.584
	blackman   3.589    134.321              max_az
	none       3.923    130.376   max_slow   min_az

   with num s == 40 are
	taper	slowness    azimuth
	hanning	   3.579    133.254
	hamming    3.587    132.995
	parzen     3.605    133.247
	cosine     3.845    130.552
	welch      3.631    131.586
	blackman   3.590    134.329
	none       3.924    130.385   max_slow   min_az

for a marginal signal
	           13.549    358.911
	hanning	   12.959    358.288
        hamming    13.279    358.581
	parzen     12.631      0.801
	cosine     14.204      6.782
	welch      13.921      0.920
	blackman   12.123    357.984
	none       17.383      6.040

*/

void FK::returnTaper(int *taper, double *beg_taper, double *end_taper)
{
    if(hanning_toggle->state()) {
	*taper = HANN_TAPER;
    }
    else if(hamming_toggle->state()) {
	*taper = HAMM_TAPER;
    }
    else if(parzen_toggle->state()) {
	*taper = PARZEN_TAPER;
    }
    else if(welch_toggle->state()) {
	*taper = WELCH_TAPER;
    }
    else if(blackman_toggle->state()) {
	*taper = BLACKMAN_TAPER;
    }
    else if(cosine_toggle->state()) {
	*taper = COSINE_TAPER;
    }
    else if(none_toggle->state()) {
	*taper = NO_TAPER;
    }

    if(!taper_window) {
	taper_window = new TaperWindow("FK Cosine Taper", this, 5, 5, 200);
	*beg_taper = .05;
	*end_taper = .05;
    }
    else {
	int width;
	if(!taper_window->width->getInt(&width) || width < 0 || width > 100) {
	    width = 5;;
            taper_window->width->setString("5");
        }
	*beg_taper = width/100;
	*end_taper = width/100;
    }
}

void FK::drawFK(bool draw_conplot)
{
    int		i, k, b, n_slowness;
    const char	*sta;
    FKData	**fkdata = p->fkdata;
    double 	slowness_max, d_slowness;
    double	xmax[4], ymax[4], x;
    Waveform	*w;

    if(fkdata == NULL || p->num_fkdata <= 0) return;

    if(count >= p->num_fkdata) {
	count = 0;
    }

    if(draw_conplot && signal_window)
    {
	x = fkdata[count]->tbeg;

	for(i = 0; i < NUM_SIGNAL_PLOTS; i++) {
	    positionLine(signal_window->plots[i], x);
	}
	beam_plot->removeActionListener(this, XtNdoubleLineCallback);
	signal_window->beam_plot->removeActionListener(this,
					XtNdoubleLineCallback);
	beam_plot->positionDoubleLine("a", fkdata[count]->tbeg,
				fkdata[count]->tend, true);
	signal_window->beam_plot->positionDoubleLine("a", fkdata[count]->tbeg,
				fkdata[count]->tend, true);
	beam_plot->addActionListener(this, XtNdoubleLineCallback);
	signal_window->beam_plot->addActionListener(this,XtNdoubleLineCallback);
    }

    k = count;
    slowness_max = fkdata[k]->args.slowness_max;
    n_slowness = fkdata[k]->args.num_slowness;
    d_slowness = fkdata[k]->d_slowness;
    for(b = 0; b < p->nbands; b++) {
	if(draw_conplot) conplot[b]->clear();
	xmax[b] = fkdata[k]->xmax[b];
	ymax[b] = fkdata[k]->ymax[b];
    }
    if(fkdata[k]->nwaveforms > 0 &&
	(w = data_source->getWaveform(fkdata[k]->waveformId(0))) != NULL)
    {
	sta = w->sta();
    }
    else {
	sta = fkdata[k]->net;
    }

    if(p->fk_units == FK_SEC_PER_DEG) {
	slowness_max *= DEG_TO_KM;
	d_slowness = 2*slowness_max/(double)(n_slowness-1);
	for(b = 0; b < p->nbands; b++)
	{
	    xmax[b] *= DEG_TO_KM;
	    ymax[b] *= DEG_TO_KM;
	}
    }

    Free(slowness);
    if(!(slowness = (double *)mallocWarn(n_slowness*sizeof(double)))) {
	return;
    }

    for(i = 0; i < n_slowness; i++) {
	slowness[i] = -slowness_max + i*d_slowness;
    }
    if(nbands == 4) {
	plot1->removeActionListener(this, XtNlimitsCallback);
	plot2->removeActionListener(this, XtNlimitsCallback);
	plot3->removeActionListener(this, XtNlimitsCallback);
	plot4->removeActionListener(this, XtNlimitsCallback);
    }
    for(b = 0; b < p->nbands; b++)
    {
	if(draw_conplot) {
	    ids[2*b] = conplot[b]->input(sta, n_slowness, n_slowness, slowness,
		slowness, fkdata[k]->fk[b], -1., 0, NULL, xmax[b], ymax[b],
		false, false, redraw, false, 0);

	    conplot[b]->display(2, ids+b*2, display_data, display_grid);
	}
	conplot[b]->positionCrosshair(0, xmax[b], ymax[b], true);

	fkmin[b] = p->min_fk[b];
	fkmax[b] = p->max_fk[b];
    }
    if(nbands == 4) {
	plot1->addActionListener(this, XtNlimitsCallback);
	plot2->addActionListener(this, XtNlimitsCallback);
	plot3->addActionListener(this, XtNlimitsCallback);
	plot4->addActionListener(this, XtNlimitsCallback);
    }
    current_fk = fkdata[k];
    setFreqScales(current_fk);
    doCallbacks(base_widget, (XtPointer)(-1), XmNactivateCallback);

    windowInfo();
}

void FK::makeFK(int plot_index, int if1, int if2)
{
    long	i, b, n_slowness;
    const char	*sta;
    double 	slowness_max, d_slowness;
    float	*f = NULL;
    Waveform	*w;

    if(current_fk == NULL) return;

    if(!current_fk->complete) {
      try {
	current_fk->fullCompute(wvec, current_fk->tbeg, current_fk->tend,
				current_fk->args);
      } catch (int n)  {
	showWarning(GError::getMessage());
	return;
      };
    }

    b = plot_index;

    if(b < 0 || b >= p->nbands) return;

    slowness_max = current_fk->args.slowness_max;
    n_slowness = current_fk->args.num_slowness;
    d_slowness = current_fk->d_slowness;

    conplot[b]->clear();

    if(current_fk->nwaveforms > 0 &&
	(w = data_source->getWaveform(current_fk->waveformId(0))) != NULL)
    {
	sta = w->sta();
    }
    else {
	sta = current_fk->net;
    }

    if(p->fk_units == FK_SEC_PER_DEG) {
	slowness_max *= DEG_TO_KM;
	d_slowness = 2*slowness_max/(double)(n_slowness-1);
    }

    Free(slowness);
    if(!(slowness = (double *)mallocWarn(n_slowness*sizeof(double)))) {
	return;
    }
    for(i = 0; i < n_slowness; i++) {
	slowness[i] = -slowness_max + i*d_slowness;
    }

    if(!(f = (float *)mallocWarn(n_slowness*n_slowness*sizeof(float)))) {
	return;
    }
    // compute scaling factor.
    double total_power = 0.;
    for(int l = if1-1; l < if2; l++) {
	total_power += current_fk->fcomplete[l];
    }
    double scaling = 1./(total_power*current_fk->nwaveforms);

    int m = n_slowness*n_slowness;
    for(i = 0; i < m; i++) f[i] = 0.;
    for(int l = if1-1; l < if2; l++) {
	for(i = 0; i < m; i++) f[i] += current_fk->complete[l*m+i]*scaling;
    }
    // find maximum (what about peak mask?)
    double fk_max = 0.;
    int imax = 0;
    for(i = 0; i < m; i++) {
	if(fk_max < f[i]) {
	    imax = i;
	    fk_max = f[i];
	}
    }
    int jmax = imax/n_slowness;
    int kmax = imax - jmax*n_slowness;
    double xmax = -slowness_max + kmax*d_slowness;
    double ymax = -slowness_max + jmax*d_slowness;
    if( !current_fk->args.output_power ) {
	fk_max = fk_max / (1. - fk_max + 1.e-06);
	for(i = 0; i < m; i++) {
	    f[i] = f[i] / (1. - f[i] + 1.e-06);
	}
    }

    for(i = 0; i < m; i++) {
	f[i] = 10. - 10. * log10(fk_max/f[i]);
    }
/*
    if(log_amp_toggle->state() && !p->three_component) {
	for(i = 0; i < m; i++) {
	    if(f[i] > 0.) f[i] = log10(f[i]);
	}
    }
*/

    ids[2*b] = conplot[b]->input(sta, n_slowness, n_slowness, slowness,
//		slowness, f, -1., 0, NULL, xmax[b], ymax[b],
		slowness, f, -1., 0, NULL, xmax, ymax,
		false, false, true, false, 0);
    Free(f);

//    conplot[b]->display(2, ids+b*2, display_data, display_grid);
    display_data[0] = true;
    conplot[b]->display(1, ids+b*2, display_data, display_grid);
    conplot[b]->positionCrosshair(0, xmax, ymax, true);

/*
	fkmin[b] = p->min_fk[b];
	fkmax[b] = p->max_fk[b];
*/

    doCallbacks(base_widget, (XtPointer)b, XmNactivateCallback);

    windowInfo();
}

void FK::windowInfo(void)
{
    int n;
    char tlab[100], text[20];
    FKData *fk = NULL;
	
    if(!info_area) return;

    tlab[0] = '\0';
    if(show_single) {
	if(p->single_fk != NULL) {
	    timeEpochToString(p->single_fk->tbeg, tlab, 100, GSE20);
	    fk = p->single_fk;
	}
    }
    else if(count >= 0 && count < p->num_fkdata)
    {
	FKData *f = p->fkdata[count];
	timeEpochToString(f->tbeg, tlab, 100, GSE20);
	fk = p->fkdata[count];
    }
    if(fk) {
	for(int i  = 0; i < fk->args.num_bands; i++) {
	    snprintf(text, sizeof(text), "%.1f", fk->fstat[i]);
	    table->setField(i, 5, text, true);
	    beam_plot->fstat[i] = fk->fstat[i];
	}
    }
    if(fk != NULL && (n = strlen(tlab)) > 0) {
	double length = fk->tend - fk->tbeg;
	snprintf(tlab+n, 100-n, "    %.2f", length);
    }
    info_area->setText(INFO_AREA_LEFT, tlab);

    if(signal_window) {
	signal_window->info_area->setText(INFO_AREA_LEFT, tlab);
    }
}

void FK::positionFK(double time)
{
    if(show_single) return;

    for(int i = 0; i < p->num_fkdata; i++) {
	FKData *f = p->fkdata[i];
	if(time > f->tbeg && time <= f->tend) {
	    count = i;
	    drawFK(true);
	    return;
	}
    }
}

void FK::positionLine(CPlotClass *cp, double x)
{
    int i, nc;
    char label[50];
    double min, d;
    float y;
    CPlotCurve c = {0, NULL, NULL, 0, NULL, NULL, (Pixel)0};
    vector<CPlotCurve> curves;

    if((nc = cp->getCurves(curves)) > 0)
    {
	if(p->nbands > 1) {
	    for(i = 0; i < p->nbands &&
		!signal_window->show_toggles[i]->state(); i++);
		
	    if(i < nc) {
		c = curves[i];
	    }
	}
	else {
	    c = curves[0];
	}
	    
	label[0] = '\0';
	if(c.npts > 0 && x >= c.x[0] && x <= c.x[c.npts-1])
	{
	    min = fabs(x - c.x[0]);
	    y = c.y[0];
	    for(i = 0; i < c.npts; i++) {
		d = fabs(x - c.x[i]);
		if(d < min) {
		    min = d;
		    y = c.y[i];
		}
	    }
	    if(y >= .1 || y == 0.0) {
		snprintf(label, 50, "%.2f", y);
	    }
	    else if(y >= .001) {
		snprintf(label, 50, "%.4f", y);
	    }
	    else {
		snprintf(label, 50, "%.6f", y);
	    }
	}
	if(cp != line_drag) {
	    cp->positionPhaseLine(label, x, false);
	}
	else {
	    cp->renamePhaseLine(label);
	}
    }
}

bool FK::workingUpdate(int count, int status, const char *label_format)
{
    if(!computing_fk->isVisible()) return true;

    if(!computing_fk || computing_fk->rtd_mode == RTD_OFF) {
	return WorkingDialog(NULL, count, status, label_format);
    }
    else if(computing_fk) {
	char s[200];
	if(status == 0) {
	    snprintf(computing_fk->status_msg, sizeof(computing_fk->status_msg),
		label_format, count);
	    computing_fk->info_area->setText(INFO_AREA_LEFT,
			computing_fk->status_msg);
	    computing_fk->signal_window->info_area->setText(INFO_AREA_LEFT,
			computing_fk->status_msg);
	}
	if(status == 1) {
	    snprintf(s, sizeof(s), "%s: %d", computing_fk->status_msg, count);
	    computing_fk->info_area->setText(INFO_AREA_LEFT, s);
	    computing_fk->signal_window->info_area->setText(INFO_AREA_LEFT, s);

	    XEvent event;
	    XtAppContext app = Application::getApplication()->appContext();

	    XFlush(XtDisplay(computing_fk->base_widget));
	    int check =0;
	    while((XtAppPending(app) & XtIMXEvent) && ++check < 1000)
	    {
		XtAppNextEvent(app, &event);
		XtDispatchEvent(&event);
	    }
	}
	return true;
    }
    return true;
}

bool FK::computeAll(bool show_warning, bool re_draw)
{
    bool dis_data[2], dis_grid[2], scan_freqs;
    int	windowed, num_bands;
    double slowness_max;
    double dt, window_length, window_overlap, bandwidth;
    double signal_slow_min, signal_slow_max, signal_az_min, signal_az_max;
    int	n_slowness, taper;
    double beg_taper, end_taper, stav_len, ltav_len;
//    bool unwrapAz;

//    if(!isVisible()) return false;

    auto_display = false;

    p->freeSpace();

    setGramButtonsOn(false);

    if(!getBands()) return false;
    num_bands = p->nbands;
    if(num_bands == 0) return false;

// for data sources that do not own the GTimeSeries objects, we
// must own them before the call to FKComputeGram, or they will be freed in
// Coverage.getArrays

    if((allGetWaveforms(&windowed, show_warning)) == 0) {
	return false;
    }

    gvector<GTimeSeries *> *ts_v = new gvector<GTimeSeries *>(p->num_waveforms);
    for(int i = 0; i < p->num_waveforms; i++) {
	ts_v->add(wvec[i]->ts);
    }

    if(windowed) {
	dt = wvec[0]->segment(wvec[0]->dw[0].d1->segmentIndex())->tdel();
    }
    else {
	dt = wvec[0]->segment(0)->tdel();
    }
    stringcpy(p->net, wvec[0]->net(), sizeof(p->net));

    if(!allGetParam(num_bands, dt, &slowness_max, &n_slowness,
		dis_data, dis_grid, &window_length, &window_overlap,
		&signal_slow_min, &signal_slow_max, &signal_az_min,
		&signal_az_max, &taper, &beg_taper, &end_taper, &stav_len,
		&ltav_len, &scan_freqs, &bandwidth))
    {
	delete ts_v;
	return false;
    }

    setCursor("hourglass");
    StatusDialog::setParent(this);
    computing_fk = this;

    try {
	if(!p->three_component) {
	    if(p->fg) {
		p->fg->deleteObject();
		p->fg = NULL;
	    }
	    p->fg = new FKGram(wvec, windowed,
		slowness_max, n_slowness, num_bands, fmin, fmax, window_length,
		window_overlap, workingUpdate, signal_slow_min, signal_slow_max,
		signal_az_min, signal_az_max, taper, beg_taper, end_taper,
		scan_freqs, bandwidth);
	    p->num_fkdata = p->fg->num_fks;
	    p->fkdata = p->fg->fkdata;
	}
	else {
	    if(p->fg3C) {
		p->fg3C->deleteObject();
		p->fg3C = NULL;
	    }
	    p->fg3C = new FKGram3C(wvec, windowed,
		slowness_max, n_slowness, num_bands, fmin, fmax, window_length,
		window_overlap, workingUpdate, taper, beg_taper, end_taper);
	    p->num_fkdata = p->fg3C->num_fks;
	    p->fkdata = p->fg3C->fkdata;
	}
    }
    catch (int n) {
	showWarning(GError::getMessage());
	setCursor("default");
	return false;
    }

    if(p->num_fkdata <= 1) {
	if(p->fg) { p->fg->deleteObject(); p->fg = NULL; }
	if(p->fg3C) { p->fg3C->deleteObject(); p->fg3C = NULL; }
	showWarning("Data window too short.");
	setCursor("default");
	delete ts_v;
	p->num_fkdata = 0;
	p->fkdata = NULL;
	return false;
    }
    show_single = false;

/*
    findMinMax(fk_window, num_bands, num_fkdata, fkdata);
*/

//    auto_display = true;
    count = 0;
    redraw = re_draw;
    p->window_length = window_length;
    p->window_overlap = window_overlap;
    display_data[0] = dis_data[0];
    display_data[1] = dis_data[1];
    display_grid[0] = dis_grid[0];
    display_grid[1] = dis_grid[1];

    p->findMinMax();

    bool unwrapAz = signal_window->unwrap_toggle->state();
    BeamLocation beam_location;
    if(dnorth_deast_toggle->state()) beam_location = DNORTH_DEAST;
    else if(ref_sta_toggle->state()) beam_location = REFERENCE_STATION;
    else beam_location = ARRAY_CENTER;

    p->updateSignalMeasurements(data_source, unwrapAz, beam_location,
		stav_len, ltav_len, wvec);

    updateSignalMeasurements();

    beam_plot->clear();
    beam_plot->unzoomAll();
    signal_window->beam_plot->clear();
    signal_window->beam_plot->unzoomAll();

//    startDisplay();
    displayFirst();

    delete ts_v;

    setCursor("default");

// this causes a segmentation fault in the program gfk
//    setButtonsSensitive();

    return true;
}

int FK::allGetWaveforms(int *windowed, bool show_warning)
{
    if(!data_source) return 0;

    wvec.clear();

    p->beam_input = false;
    p->num_waveforms = 0;
    Free(p->waveform_ids);

    *windowed = data_source->getSelectedWaveforms("a", wvec);

    if(*windowed < 0) // there is no "a" window displayed
    {
	*windowed = 0;
	data_source->getSelectedWaveforms(wvec);
    }

    if(wvec.size() == 0)
    {
	// only warn if this came from the Compute button
	if(show_warning) {
	    showWarning("No waveforms selected.");
	}
	return 0;
    }
    else if(wvec.size() == 1) {
	stringcpy(p->sta, wvec[0]->sta(), sizeof(p->sta));
	stringcpy(p->chan, wvec[0]->chan(), sizeof(p->chan));
	p->num_waveforms = getArrayElements(wvec, show_warning);
	if(p->num_waveforms > 1) p->beam_input = true;
    }
    else if(wvec.size() == 3)
    {
	vector<int> ncmpts;
	gvector<Waveform *> ws;
	wvec.clear();
	if(*windowed) {
	    data_source->getSelectedComponents("a", ncmpts, ws);
	}
	else {
	    data_source->getSelectedComponents(ncmpts, ws);
	}
	if((int)ncmpts.size() != 1 || ncmpts[0] != 3) {
	    if(show_warning) {
		showWarning(
	    "3 Selected waveforms must be vertical and horizontal components.");
	    }
	    return 0;
	}
	for(int i = 0; i < 3; i++) wvec.push_back(ws[i]);
	p->three_component = True;
    }
    p->num_waveforms = wvec.size();
    if(!(p->waveform_ids = (int *)mallocWarn(wvec.size()*sizeof(int)))) {
	return 0;
    }
    if(*windowed) {
	auto_tbeg = wvec[0]->dw[0].d1->time();
	auto_tend = wvec[0]->dw[0].d2->time();
    }
    else {
	auto_tbeg = auto_tend = 0.;
    }

    for(int i = 0; i < wvec.size(); i++) {
	p->waveform_ids[i] = wvec[i]->getId();
    }
    data_source->getTable(arrivals);
    data_source->getTable(assocs);
    return wvec.size();
}

// The ids in this routine will not work as expected on a data source other
// than WaveformPlot.
bool FK::haveData(void)
{
    int i, j, nwaveforms;
    gvector<Waveform *> ws;

    if(!data_source) return false;
    nwaveforms = data_source->getSelectedWaveforms(ws);
    gvector<GTimeSeries *> *ts_v = new gvector<GTimeSeries *>(nwaveforms);
    for(i = 0; i < p->num_waveforms; i++) {
	ts_v->add(ws[i]->ts);
    }

    if(p->num_waveforms > 0) {
	/* Check if the input data is the same.
	 */
	if(nwaveforms == 1 && p->beam_input
		&& !strcmp(p->sta, ws[0]->sta())
		&& !strcmp(p->chan, ws[0]->chan()))
	{
	    delete ts_v;
	    return true;	// Same input data
	}
	else if(nwaveforms == p->num_waveforms)
	{
	    for(i = 0; i < p->num_waveforms; i++) {
		for(j = 0; j < nwaveforms &&
			p->waveform_ids[i] != ws[j]->getId(); j++);
		if(j == nwaveforms) break;
	    }
	    if(i == p->num_waveforms) {
		delete ts_v;
		return true; /* Same input data */
	    }
	}
    }

    /* New input data. Free existing fk's.
     */
    for(i = 0; i < p->num_fkdata; i++) {
	p->fkdata[i]->deleteObject();
    }
    p->freeSpace();

    p->beam_input = false;
    if(nwaveforms <= 0)
    {
	showWarning("No waveforms selected.");
	delete ts_v;
	return false;
    }
    else if(nwaveforms == 1) {
	nwaveforms = getArrayElements(ws, true);
	if(nwaveforms > 1) p->beam_input = True;
    }
    if(!nwaveforms) {
	delete ts_v;
	return false;
    }
    p->num_waveforms = nwaveforms;
    if(!(p->waveform_ids = (int *)mallocWarn(nwaveforms*sizeof(int))))
    {
	delete ts_v;
	return false;
    }
    for(i = 0; i < nwaveforms; i++) {
	p->waveform_ids[i] = ws[i]->getId();
    }
    delete ts_v;
    return (nwaveforms > 0) ? true : false;
}

bool FK::allGetParam(int num_bands, double dt, double *slowness_max,
		int *n_slowness, bool *dis_data,
		bool *dis_grid, double *window_length,
		double *window_overlap, double *signal_slow_min,
		double *signal_slow_max, double *signal_az_min,
		double *signal_az_max, int *taper, double *beg_taper,
		double *end_taper, double *stav_length, double *ltav_length,
		bool *scan_freqs, double *bandwidth)
{
    double smax;
    char str[50];

    if(!parameter_window->getDouble("s_max_text", &smax)) {
	smax = (p->fk_units == FK_SEC_PER_KM) ? .36 : .36*DEG_TO_KM;
	ftoa((float)smax, 3, 0, str, sizeof(str));
	parameter_window->setString("s_max_text", str);
    }
    if(p->fk_units == FK_SEC_PER_DEG) smax /= DEG_TO_KM;

    *slowness_max =  smax; // currently always in sec/km

    if(!parameter_window->getInt("num_s_text", n_slowness)) {
	if(num_bands == 1) {
	    *n_slowness = 81;
	    parameter_window->setString("num_s_text", "81");
	}
	else {
	    *n_slowness = 61;
	    parameter_window->setString("num_s_text", "61");
	}
    }
    else if(*n_slowness % 2 == 0) {
	(*n_slowness)++; // must be odd
	snprintf(str, sizeof(str), "%d", *n_slowness);
	parameter_window->setString("num_s_text", str);
    }

    *signal_slow_min = -1.;
    *signal_slow_max = -1.;
    *signal_az_min = -1.;
    *signal_az_max = -1.;
    parameter_window->getDouble("slow_min_text", signal_slow_min);
    parameter_window->getDouble("slow_max_text", signal_slow_max);
    parameter_window->getDouble("az_min_text", signal_az_min);
    parameter_window->getDouble("az_max_text", signal_az_max);
    if(p->fk_units == FK_SEC_PER_DEG) {
	if(*signal_slow_min != -1.) *signal_slow_min /= DEG_TO_KM;
	if(*signal_slow_max != -1.) *signal_slow_max /= DEG_TO_KM;
    }

    dis_data[0] = true;
    dis_grid[0] = false;

    if(grid_toggle->state()) {
	dis_grid[0] = true;
    }
    dis_data[1] = false;
    dis_grid[1] = false;

    if(fine_grid_toggle->state()) {
	dis_grid[1] = true;
	dis_data[1] = true;
	dis_data[0] = false;
    }
    for(int i = 0; i < num_bands; i++) {
	if(rtd_mode == RTD_OFF) conplot[i]->clear();
	ids[2*i] = ids[2*i+1] = 0;
    }
    show_single = false;

    *window_length =  5.;
    *window_overlap =  3.;

    if(!parameter_window->window_length_text->getDouble(window_length)) {
	parameter_window->window_length_text->setString("5.");
	if(signal_window) signal_window->window_length_text->setString("5.");
    }
    if(*window_length <= 0.) {
	showWarning("Invalid window length: %f", *window_length);
	return false;
    }
    if(!parameter_window->window_overlap_text->getDouble(window_overlap)) {
	char buf[20];
	*window_overlap = 3.*(*window_length)/5.;
	ftoa(*window_overlap, 3, 0, buf, 100);
	parameter_window->window_overlap_text->setString(buf);
	if(signal_window) signal_window->window_overlap_text->setString(buf);
    }
    if(*window_overlap < 0 || *window_overlap >= *window_length) {
	showWarning("Invalid window overlap: %f", *window_overlap);
	return false;
    }

    *stav_length = 1.;
    *ltav_length = 60.;
    if(!parameter_window->stav_length_text->getDouble(stav_length)) {
	parameter_window->stav_length_text->setString("1.0");
	if(signal_window) signal_window->stav_length_text->setString("1.0");
    }
    if(*stav_length <= 0.) {
	showWarning("Invalid stav length: %f", *stav_length);
	return false;
    }
    if(!parameter_window->ltav_length_text->getDouble(ltav_length)) {
	parameter_window->ltav_length_text->setString("60.0");
	if(signal_window) signal_window->ltav_length_text->setString("60.0");
    }
    if(*ltav_length <= *stav_length) {
	showWarning("Invalid ltav length: %f", *ltav_length);
	return false;
    }

    returnTaper(taper, beg_taper, end_taper);

    if(type() == FK_SINGLE_BAND) {
	*scan_freqs = parameter_window->scan_frequencies_toggle->state();
	if(!parameter_window->getDouble("bandwidth_text", bandwidth)
	    || bandwidth <= 0)
	{
	    parameter_window->scan_frequencies_toggle->set(false, true);
	    *scan_freqs = false;
	}
    }
    else {
	*scan_freqs = false;
    }

    return true;
}

void FK::setWindowLength(double window_length)
{
    char buf[20];
    ftoa(window_length, 3, 0, buf, 100);
    signal_window->window_length_text->setString(buf);
}

void FK::setWindowOverlap(double window_overlap)
{
    char buf[20];
    ftoa(window_overlap, 3, 0, buf, 100);
    signal_window->window_overlap_text->setString(buf);
}

void FK::doubleLine(AxesCursorCallbackStruct *axes_cursor)
{
    if(isVisible() && auto_cursor_toggle->state() &&
	axes_cursor->type == AXES_VER_DOUBLE_LINE)
    {
	compute(false, true);
    }
}

void FK::crosshairCB(ConPlotClass *cp, AxesCursorCallbackStruct *c)
{
    double az, sec_per_km, fk_slowness;
    char text[32];
    int row = 0;

    /* need to fix the label in the popup */

    FKParam::crosshair_to_slow_az(p->fk_units, c->scaled_x, c->scaled_y,
					&sec_per_km, &az);

    ignore_callback = 1;

    fk_slowness = sec_per_km;

    ftoa(1./fk_slowness, 3, 0, text, 32);
    if(p->nbands == 1)
    {
//	app_text->setString(text);
	table->setField(0, 2, text, false);
    }
    else {
	for(row = 0; row < p->nbands && !(conplot[row] == cp); row++);
	table->setField(row, 2, text, false);
    }

    if(p->fk_units == FK_SEC_PER_KM) {
	ftoa(sec_per_km, 4, 0, text, 32);
    }
    else {
	ftoa(sec_per_km*DEG_TO_KM, 3, 0, text, 32);
    }
    table->setField(row, 3, text, false);

    ftoa(az, 3, 0, text, 32);
    if(p->nbands == 1)
    {
	table->setField(0, 4, text, true);
    }
    else if(table != NULL) {
	table->setField(row, 4, text, true);
    }
    ignore_callback = 0;

    if(p->nbands == 1) row = -1;
    crosshairAction(sec_per_km, az, c->reason, row);
}

void FK::windowDrag(AxesCursorCallbackStruct *c)
{
    FKData **fkdata = p->fkdata;
    double shift, x1, x2, dmin=0., d;
    int k, kmin=0;

    if(ignore_callback || auto_display || c->label[0] !='b' || p->num_fkdata<=0)
    {
	return;
    }

    for(k = 0; k < p->num_fkdata; k++)
    {
	shift = getDataShift(fkdata[k]);

	x1 = fkdata[k]->tbeg - shift;
	x2 = fkdata[k]->tend - shift;

	if(k == 0) {
	    kmin = 0;
	    dmin = fabs(c->scaled_x1 - x1) + fabs(c->scaled_x2 - x2);
	}
	else {
	    d = fabs(c->scaled_x1 - x1) + fabs(c->scaled_x2 - x2);
	    if(d < dmin) {
		kmin = k;
		dmin = d;
	    }
	}
    }
    if(kmin != count)
    {
	count = kmin;
	drawFK(true);
    }
    if(c->reason != AXES_DOUBLE_LINE_DRAG) {
	count = kmin;
	shift = getDataShift(fkdata[kmin]);
	ignore_callback = true;
	if(wp) {
	    wp->positionDoubleLine("b", fkdata[kmin]->tbeg - shift,
			fkdata[kmin]->tend - shift, true);
	}
	ignore_callback = false;
    }
}

void FK::lineDrag(AxesClass *comp, AxesCursorCallbackStruct *c)
{
    FKData **fkdata = p->fkdata;
    double shift, d, dmin;
    int k, kmin;

    if(show_single && c->reason == AXES_DOUBLE_LINE_DRAG) {
	if(wp) {
	    ignore_callback = true;
	    wp->positionDoubleLine("a", c->scaled_x1, c->scaled_x2, false);
	    ignore_callback = false;
	}
	return;
    }

    if(ignore_callback || auto_display || p->num_fkdata <= 0)
    {
	return;
    }

    kmin = 0;
    dmin = fabs(c->scaled_x);
    for(k = 1; k < p->num_fkdata; k++)
    {
	d = fabs(c->scaled_x - fkdata[k]->tbeg);
	if(d < dmin) {
	    dmin = d;
	    kmin = k;
	}
    }

    if((c->reason == AXES_PHASE_LINE_DRAG || c->reason == AXES_DOUBLE_LINE_DRAG)
		&& kmin != count)
    {
	update_ftrace = false;
	count = kmin;
	shift = getDataShift(fkdata[kmin]);
	if(wp) {
	    ignore_callback = true;
	    wp->positionDoubleLine("b", fkdata[kmin]->tbeg - shift,
			fkdata[kmin]->tend - shift, true);
	    ignore_callback = false;
	}
	line_drag = comp;
	drawFK(true);
	line_drag = NULL;
	update_ftrace = true;
    }
    else if(c->reason == AXES_DOUBLE_LINE_POSITION) {
	update_ftrace = true;
	drawFK(false);
    }
    else if(c->reason == AXES_PHASE_LINE_POSITION) {
	double x = fkdata[kmin]->tbeg;
	update_ftrace = true;
	ignore_callback = true;
	comp->positionPhaseLine("", x, true);
	ignore_callback = false;
	drawFK(false);
    }
}

void FK::crosshairAction(double slow, double az, int reason, int row)
{
    /* reason == 0 for button release, = 1 for button drag.
     */
    if(reason == 0)
    {
	if(align_toggle->state() && wp) {
	    align(slow, az);
	}
	displayBeams(slow, az, (row >= 0) ? row : 0);
    }

    if(map)
    {
	drawOnMap(az, reason, row);
    }
}

void FK::align(double slow, double az)
{
    int num;
    gvector<Waveform *> ws;

    if((num = data_source->getSelectedWaveforms(ws)) == 1) {
	num = getArrayElements(ws, false);
    }
    if(num > 0)
    {
	vector<double> t_lags;
	if( getLags(ws, az, slow, t_lags) ) {
	    wp->alignWaveforms(ws, t_lags);
	}
    }
}

void FK::doFtrace()
{
    char line[1000];
    string msg;
    Application *app = Application::getApplication();
    vector<int> rows;

    if(nbands == 1 || table->getSelectedRows(rows) > 0)
    {
	double flo, fhi, vel, slow, az;
	if(nbands == 1) rows.push_back(0);

	if(table->getDouble(rows[0], 0, &flo) &&
	   table->getDouble(rows[0], 1, &fhi) &&
	   table->getDouble(rows[0], 2, &vel) &&
	   table->getDouble(rows[0], 4, &az))
	{
	    slow = 1./vel;
	    align(slow, az);

	    app->parseLine("ftrace.clear", msg);
	    app->parseLine("ftrace.open", msg);
	    snprintf(line, sizeof(line), "ftrace.compute flo=%.6f fhi=%.6f \
slow=%.6f az=%.6f snr=3. wlen=2. autoloc=true cursor=0",
		flo, fhi, slow, az);
	    app->parseLine(line, msg);
	}
    }
}

void FK::displayBeams(double sec_per_km, double az, int row)
{
    vector<double> t_lags;
    char chan[10], text[50];
    double beam_lat, beam_lon, stav_len, ltav_len, tmin=0., tmax=0., raw_snr;
    GTimeSeries *ts = NULL, *ftrace = NULL;
    GTimeSeries beam_ts, semb_ts, prob_ts;
    gvector<Waveform *> ws, v;
    int i, j, num, npols=2, spts=20;

    if( !getLags(wvec, az, sec_per_km, t_lags, &beam_lat, &beam_lon) ) {
      showWarning("Beam: failed to get station time lags for selected waveforms. Make sure that you selected the same station/channel.");
      return;
    }
    else {
        if(auto_tend > auto_tbeg) {  // Windowed
	  tmin = auto_tbeg;
	  tmax = auto_tend;
        }
	/*  It doesn't work to get tmin/tmax from waveformplot if the plot has been aligned */
	/*
        else if (wp) {  // Waveformplot
	  double ymin, ymax;
	  wp->getLimits(&tmin, &tmax, &ymin, &ymax)
	  //wp->getTimeLimits(&tmin, &tmax);
	}
	*/

	if (tmax > tmin) {  // If windowed
	  try { 
	    ts = Beam::BeamSubSeries(wvec, t_lags, tmin, tmax, true);
	  }
	  catch (...) {
	    showWarning(GError::getMessage());
	    return;
	  }
	}
	else {
	  try {
	    ts = Beam::BeamTimeSeries(wvec, t_lags, true);
	  }
	  catch (...) {
	    showWarning(GError::getMessage());
	    return;
	  }
	  tmin = wvec[0]->tbeg();
	  tmax = wvec[0]->tend();
	}
	ts->makeCopy();
    }
    if(!ts) {
      showWarning("Beam: no time series");
      return;
    }
    if (ts->size() <= 0) {        
      showWarning("Beam: no segments in time series");
      return;
    }

    ts->addOwner(this); // need this for getSnr, which deletes GDataPoints

    beam_plot->azimuth[row] = az;
    beam_plot->slow[row] = sec_per_km*DEG_TO_KM;
    beam_plot->fmin[row] = fmin[row];
    beam_plot->fmax[row] = fmax[row];

    ts->setDataSource(wvec[0]->getDataSource());
    ts->copyWfdiscPeriods(wvec[0]->ts); // to addArrivals
    ts->putValue("beam_id", row);
    ts->setSta(wvec[0]->net());
    ts->setNet(wvec[0]->net());
    ts->setLat(beam_lat);
    ts->setLon(beam_lon);
    ts->setComponent(1);

    // filter waveform
    IIRFilter *iir = NULL;
    try {
	iir = new IIRFilter(3, "BP", fmin[row], fmax[row],
			ts->segment(0)->tdel(), 0);
    }
    catch(...) {
	cerr << GError::getMessage() << endl;
    }
    if(iir) {
	Demean *demean = new Demean();
	TaperData *taper = new TaperData("cosine", 10, 5, 200);
	demean->apply(ts);
	taper->apply(ts);
	iir->apply(ts);
    }
    if(!parameter_window->stav_length_text->getDouble(&stav_len)) {
	stav_len = 1.;
	parameter_window->stav_length_text->setString("1.0");
    }
    if(!parameter_window->ltav_length_text->getDouble(&ltav_len)) {
	ltav_len = 60.;
	parameter_window->ltav_length_text->setString("60.0");
    }

    snr[row] = 0.;
    if(show_single) {
	if(p->single_fk != NULL) {
	    FKData *fk = p->single_fk;
	    double len = fk->tend - fk->tbeg;
	    double ltav = ltav_len;
	    if(ltav_len > fk->tbeg - ts->tbeg()) {
		ltav = .99*(fk->tbeg - ts->tbeg());
	    }
	    snr[row] = FKParam::getSnr(ts, fk->tbeg, len, stav_len, ltav);
	    ltav = ltav_len;
	    if(ltav_len > fk->tbeg - wvec[0]->tbeg()) {
		ltav = .99*(fk->tbeg - wvec[0]->tbeg());
	    }
	    raw_snr = FKParam::getSnr(wvec[0]->ts, fk->tbeg, len,stav_len,ltav);
	}
    }
    else if(count >= 0 && count < p->num_fkdata) {
	FKData *fk = p->fkdata[count];
	double len = p->window_length;
	snr[row] = FKParam::getSnr(ts, fk->tend-len, len, stav_len, ltav_len);
    }
    snprintf(text, sizeof(text), "%.2f", snr[row]);
    table->setField(row, 6, text, true);

    if(update_ftrace) {
	ftrace = new GTimeSeries();
	raw_snr=6.0;
	Beam::ftrace(wvec, tmin, tmax, t_lags, spts, npols, fmin[row],fmax[row],
			true, raw_snr, &beam_ts, &semb_ts, ftrace, &prob_ts);
	ftrace->putValue("beam_id", row);
	ftrace->setLat(beam_lat);
	ftrace->setLon(beam_lon);
    }

    for(i = 0; i < (int)windows.size() && this != windows[i]; i++);
    if(i == (int)windows.size()) {
	snprintf(chan, sizeof(chan), "fk010001");
    }
    else {
	snprintf(chan, sizeof(chan), "fk%02d0001", i+2);
    }
    if(!show_single) {
	snprintf(chan+4, sizeof(chan)-4, "%04d", count+1);
    }

    ts->setChan(chan);

    if(update_ftrace) ftrace->setChan(chan);

    num = beam_plot->getWaveforms(ws)/2;
    if(num <= row) {
	Arg args[1];
	Pixel color;
	XtSetArg(args[0], XtNdataColor, &color);
	conplot[row]->getValues(args, 1);
	ts->tag.members.push_back(0);
	ts->tag.ud_string.assign("beam");
	beam_plot->addWaveform(ts, color);
	if(update_ftrace) {
	    ftrace->tag.members.push_back(0);
	    ftrace->tag.ud_string.assign("ftrace");
	    beam_plot->addWaveform(ftrace, color);
	}
	if(i == 0) {
	    Pixel fg = stringToPixel("Black");
	    for(j = 0; j < arrivals.size(); j++) {
		beam_plot->putArrivalWithColor(arrivals[j], fg);
	    }
	}
    }
    else {
	ws[2*row]->ts = ts;
	v.push_back(ws[2*row]);
	if(update_ftrace) {
	    ws[2*row+1]->ts = ftrace;
	    v.push_back(ws[2*row+1]);
	}
	beam_plot->modify(v, NULL, true);
    }

    if(show_single) {
	beam_plot->positionDoubleLine("a", p->single_fk->tbeg,
				p->single_fk->tend, false);
    }
    
    for(i = 0; i < assocs.size(); i++) {
	beam_plot->putTable(assocs[i]);
    }

    CssOriginClass *origin = data_source->getPrimaryOrigin(wvec[0]);
    if(origin) {
	ws.clear();
	num = beam_plot->getWaveforms(ws);
	for(i = 0; i < num; i++) {
	    beam_plot->setPrimaryOrigin(ws[i], origin);
	}
	beam_plot->updatePredicted();
    }

    ws.clear();
    num = signal_window->beam_plot->getWaveforms(ws);
    if(num <= row) {
	Arg args[1];
	Pixel color;
	XtSetArg(args[0], XtNdataColor, &color);
	conplot[num]->getValues(args, 1);
	signal_window->beam_plot->addWaveform(ts, color);
	if(num == 0) {
	    Pixel fg = stringToPixel("Black");
	    for(j = 0; j < arrivals.size(); j++) {
		signal_window->beam_plot->putArrivalWithColor(arrivals[j], fg);
	    }
	}
    }
    else {
	ws[row]->ts = ts;
	gvector<Waveform *> v(ws[row]);
	signal_window->beam_plot->modify(v, NULL, true);
    }

    for(i = 0; i < assocs.size(); i++) {
	signal_window->beam_plot->putTable(assocs[i]);
    }

    if(origin) {
	ws.clear();
	num = signal_window->beam_plot->getWaveforms(ws);
	for(i = 0; i < num; i++) {
	    signal_window->beam_plot->setPrimaryOrigin(ws[i], origin);
	}
    }

    ts->removeOwner(this);
    if(update_ftrace) ftrace->removeOwner(this);
}

void FK::redrawMap(void)
{
    AxesCursorCallbackStruct *a;
    for(int i = 0; i < p->nbands; i++) {
	if(conplot[i]->getCrosshairs(&a) > 0) {
	    double sec_per_km, az;
	    FKParam::crosshair_to_slow_az(p->fk_units, a->scaled_x, a->scaled_y,
		&sec_per_km, &az);
	    if(p->nbands == 1) drawOnMap(az, 1, -1);
	    else drawOnMap(az, 0, i);
	}
    }
}

void FK::drawOnMap(double az, int reason, int row)
{
    int		i, id, nsta, display_mode;
    Pixel	fg;
    bool	redisplay;
    const char	*arc_lab[4] = {"fk1", "fk2", "fk3", "fk4"};
    FKData	*fk_data;
    MapPlotArc arc, arc_init = MAP_PLOT_ARC_INIT;
    MapPlotStation *sta = NULL;
    LineInfo line_info_init = LINE_INFO_INIT;

    if(show_single) {
	if(p->single_fk == NULL) return;
	fk_data = p->single_fk;
    }
    else {
	if(p->fkdata == NULL || p->num_fkdata <= 0 || count >= p->num_fkdata) {
	    return;
	}
	fk_data = p->fkdata[count];
    }
    if(map)
    {
	if(!map->isVisible()) {
	    display_mode = MAP_OFF;
	}
	else {
	    display_mode = map->getFKAzDisplay();
	}
//	display_mode = map->getFKAzDisplay();
	redisplay = (display_mode == MAP_OFF) ? false : true;
	arc = arc_init;

	if(row == -1) {
	    arc.label = (char *)"fk";
	    fg = sub_data.azimuth_color;
	}
	else {
	    arc.label = (char *)arc_lab[row];
	    fg = (Pixel)-1;
	    Arg args[1];
	    XtSetArg(args[0], XtNdataColor, &fg);
	    conplot[row]->getValues(args, 1);
	    if(fg == (Pixel)-1) fg = sub_data.azimuth_color;
	}

	if(!map->getStaArc(fk_data->center_sta, arc.label, &arc))
	{
	    arc.lat = fk_data->center_lat;
	    arc.lon = fk_data->center_lon;
	    arc.az = az;
	    arc.del = 180.;
	    memcpy(&arc.line, &line_info_init, sizeof(LineInfo));

	    arc.line.fg = fg;
	    arc.line.xorr = false;
	    arc.line.display = display_mode;
	    id = map->addArc(&arc, redisplay);
	    nsta = map->getStations(&sta);
	    for(i = 0; i < nsta; i++) {
		if(sta[i].label != NULL &&
			!strcmp(sta[i].label, fk_data->center_sta)) break;
	    }
	    if(i < nsta) {
		map->assoc(id, sta[i].id);
	    }
	    Free(sta);
	}
	arc.az = az;
	arc.line.fg = fg;
	arc.line.xorr = false;
	arc.line.display = display_mode;
	map->change((MapObject *)&arc, redisplay);
    }
}

/*
static void
redrawFK(FKWindow *fk_window, FKData *fkdata)
{
	int	i, b, n_slowness;
	char	*sta;
	double  slowness_max, d_slowness;
	double	xmax[4], ymax[4];
	double	*slow_x = NULL;
	Waveform *w;

	slowness_max = fkdata->slowness_max;
	n_slowness = fkdata->n_slowness;
	d_slowness = fkdata->d_slowness;
	for(b = 0; b < fk_window->p->nbands; b++) {
	    ConPlotClear(fk_window->conplot[b]);
	    xmax[b] = fkdata->xmax[b];
	    ymax[b] = fkdata->ymax[b];
	}
	if(fkdata->num_waveforms > 0 &&
		(w = CPlotGetWaveform((CPlotWidget)fk_window->main_plot,
		fkdata->waveform_id[0])) != NULL)
	{
	    sta = w->sta;
	}
	else sta = fkdata->sta;

	if(fk_window->p->fk_units == FK_SEC_PER_DEG) {
	    slowness_max *= DEG_TO_KM;
	    d_slowness = 2*slowness_max/(double)(n_slowness-1);
	    for(b = 0; b < fk_window->p->nbands; b++)
	    {
		xmax[b] *= DEG_TO_KM;
		ymax[b] *= DEG_TO_KM;
	    }
	}

	if(!(slow_x = (double *)mallocWarn(n_slowness*sizeof(double)))) {
	    return;
	}

	for(i = 0; i < n_slowness; i++) {
	    slow_x[i] = -slowness_max + i*d_slowness;
	}
	setCursor("hourglass");
	for(b = 0; b < fk_window->p->nbands; b++)
	{
	    fk_window->ids[2*b] = ConPlotInput(fk_window->conplot[b],
		sta, n_slowness, n_slowness, slow_x, slow_x,
		fkdata->fk[b], -1., 0, NULL, xmax[b], ymax[b],
		false, false, fk_window->redraw, false, 0);

	    ConPlotDisplay(fk_window->conplot[b], 2, fk_window->ids+b*2,
		fk_window->display_data, fk_window->display_grid);
	    AxesPositionCrosshair((AxesWidget)fk_window->conplot[b],
		xmax[b], ymax[b], true);
	}
	Free(slow_x);
	setCursor("default");

	windowInfo(fk_window);
}
*/

bool FK::getLags(gvector<Waveform *> &ws, double az, double slow,
		vector<double> &tlags, double *beam_lat, double *beam_lon)
{
    BeamLocation beam_location;

    if(dnorth_deast_toggle->state()) {
	beam_location = DNORTH_DEAST;
    }
    else if(ref_sta_toggle->state()) {
	beam_location = REFERENCE_STATION;
    }
    else {
	beam_location = ARRAY_CENTER;
    }

    return Beam::getTimeLags(data_source, ws, az, slow, beam_location, tlags,
			beam_lat, beam_lon);
}

void FK::fkBeam(void)
{
    double az, sec_per_km;
    AxesCursorCallbackStruct *a;

    if(!getBeamCursor(&a)) {
	showWarning("No crosshair on FK plot.");
	return;
    }

//    slow = sqrt(a->scaled_x*a->scaled_x + a->scaled_y*a->scaled_y);
    az = atan2(a->scaled_x, a->scaled_y);
    az *= (180./M_PI);
    FKParam::crosshair_to_slow_az(p->fk_units, a->scaled_x, a->scaled_y,
			&sec_per_km,&az);

    makeBeam(az, sec_per_km, "fkb");
}

bool FK::getBeamCursor(AxesCursorCallbackStruct **a)
{
    ConPlotClass *plot;

    if(p->nbands == 1) {
	plot = plot1;
    }
    else
    {
	vector<int> rows;

	if(table->getSelectedRows(rows) > 0) {
	    plot = conplot[rows[0]];
	}
	else {
	    table->selectRow(0, true);
	    plot = conplot[0];
	}
    }
    return (plot->getCrosshairs(a) > 0) ? true : false;
}


void FK::makeBeam(double az, double sec_per_km, const char *chan)
{
    gvector<Waveform *> ws;
    vector<double> t_lags;
    double beam_lat, beam_lon;
    int num;

    if( !wp ) return;

    if((num = data_source->getSelectedWaveforms(ws)) <= 0)
    {
	showWarning("No waveforms selected.");
	return;
    }
    p->beam_input = false;
    if(num == 1) {
	num = getArrayElements(ws, true);
	if(num > 1) p->beam_input = true;
    }

    if ( !getLags(ws, az, sec_per_km, t_lags, &beam_lat, &beam_lon) ) {
      showWarning("Beam: failed to get station time lags for selected waveforms. Make sure that you selected the same station/channel.");
      return;
    }

    wp->computeBeam(ws, t_lags, chan, beam_lat, beam_lon, true, false, wp);
}

void FK::makeBeam(double az, double sec_per_km, const char *chan, double time,
		double endtime)
{
    gvector<Waveform *> ws;
    vector<double> t_lags;
    double beam_lat, beam_lon;
    int num;

    if( !wp ) return;

    if((num = data_source->getSelectedWaveforms(ws)) <= 0)
    {
	showWarning("No waveforms selected.");
	return;
    }
    p->beam_input = false;
    if(num == 1) {
	num = getArrayElements(ws, true);
	if(num > 1) p->beam_input = true;
    }

    if( !getLags(ws, az, sec_per_km, t_lags, &beam_lat, &beam_lon) ) {
      showWarning("Beam: failed to get station time lags for selected waveforms. Make sure that you selected the same station/channel.");
      return;
    }

    wp->computeBeam(ws, t_lags, chan, beam_lat, beam_lon, time, endtime,
		    true, false, wp);
}

Matrx * FK::getData(int b)
{
    if(b < 0 || b >= p->nbands) return NULL;

    return conplot[b]->getMatrix();
}

double FK::getFMin(int b)
{
    if(b < 0 || b >= p->nbands) return -1.;
    double f = 0.;
    lo_text[b]->getDouble(&f);
    return f;
}

double FK::getFMax(int b)
{
    if(b < 0 || b >= p->nbands) return -1.;
    double f = 0.;
    hi_text[b]->getDouble(&f);
    return f;
}

void FK::saveSignals(const char *prefix)
{
    char path[MAXPATHLEN+1];
    double dt, t0, samprate;
    FILE *fp;

    if(p->num_fkdata <= 1) return;

    if(stringCaseEndsWith(prefix, ".asc")) {
	strncpy(path, prefix, sizeof(path));
	path[sizeof(path)-1] = '\0';
    }
    else {
	snprintf(path, sizeof(path), "%s.asc", prefix);
    }
    if( !(fp = fopen(path, "w")) ) {
	showWarning("Cannot open %s", path);
	return;
    }

    t0 = p->x[0];
    dt = p->x[1] - p->x[0];
    samprate = (dt != 0.) ? 1./dt : 1.;

    for(int i = 0; i < p->nbands; i++) {
	fprintf(fp, "wfdisc sta=%s chan=snr%d time=%.4f samprate=%.3f\n",
		p->net, i+1, t0, samprate);
	fprintf(fp, "lofreq=%.2f hifreq=%.2f\ndata\n", fmin[i], fmax[i]);
	for(int j = 0; j < p->num_fkdata; j++) {
	    fprintf(fp, "%.6g\n", p->sig[i].snr[j]);
	}
    }
    for(int i = 0; i < p->nbands; i++) {
	fprintf(fp, "wfdisc sta=%s chan=fstat%d time=%.4f samprate=%.3f\n",
		p->net, i+1, t0, samprate);
	fprintf(fp, "lofreq=%.2f hifreq=%.2f\ndata\n", fmin[i], fmax[i]);
	for(int j = 0; j < p->num_fkdata; j++) {
	    fprintf(fp, "%.6g\n", p->sig[i].fstat[j]);
	}
    }
    for(int i = 0; i < p->nbands; i++) {
	fprintf(fp, "wfdisc sta=%s chan=slow%d time=%.4f samprate=%.3f\n",
		p->net, i+1, t0, samprate);
	fprintf(fp, "lofreq=%.2f hifreq=%.2f\ndata\n", fmin[i], fmax[i]);
	for(int j = 0; j < p->num_fkdata; j++) {
	    fprintf(fp, "%.6g\n", p->sig[i].slowness[j]);
	}
    }
    for(int i = 0; i < p->nbands; i++) {
	fprintf(fp, "wfdisc sta=%s chan=az%d time=%.4f samprate=%.3f\n",
		p->net, i+1, t0, samprate);
	fprintf(fp, "lofreq=%.2f hifreq=%.2f\ndata\n", fmin[i], fmax[i]);
	for(int j = 0; j < p->num_fkdata; j++) {
	    fprintf(fp, "%.6g\n", p->sig[i].azimuth[j]);
	}
    }
    fclose(fp);
}

void FK::saveAzSlow(void)
{
    vector<int> sel;
    int changed[3] = {6, 10, 12};
    vector<const char *> row;
    double az, slow;
    cvector<CssArrivalClass> arr;
    CssArrivalClass *a;
    DataSource *ds;

    if(!data_source) return;

    data_source->getSelectedTable(arr);
    if(arr.size() <= 0) {
	showWarning("No arrivals selected.");
	return;
    }
    else if(arr.size() > 1) {
	showWarning("More than one arrival selected.");
	return;
    }
    if( !(ds = arr[0]->getDataSource()) ) {
	showWarning("Cannot save changes.");
	return;
    }
    if(p->nbands > 1) {
	if(table->getSelectedRows(sel) <= 0) {
	    showWarning("No frequency band selected.");
	    return;
	}
	table->getRow(sel[0], row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);
    }
    else {
	table->getRow(0, row);
	tableGetAzSlow(p->fk_units, row[3], row[4], &az, &slow);
    }
    a = new CssArrivalClass(*arr[0]);

    a->azimuth = az;
    a->slow = slow*DEG_TO_KM;
    strcpy(a->chan, "fk");
    ds->changeTable(arr[0], a);
    arr[0]->azimuth = a->azimuth;
    arr[0]->slow = a->slow;
    strcpy(arr[0]->chan, "fk");
    TableListener::doCallbacks(arr[0], this, 3, changed);
}
