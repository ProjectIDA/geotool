/** \file ColorSelection.cpp
 *  \brief Defines class ColorSelection.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "ColorSelection.h"
#include "motif++/MotifClasses.h"
#include "widget/HistgmClass.h"
#include "widget/ConPlotClass.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

ColorSelection::ColorSelection(const string &name, Component *parent,
		ActionListener *listener, const string &rgb_list)
		: ParamDialog(name, parent)
{
    createInterface();
    init();
    initColors(rgb_list);
    enableCallbackType(XmNvalueChangedCallback);
    enableCallbackType(XtNcolorLimitsApply);
    if(listener) {
	addActionListener(listener, XmNvalueChangedCallback);
	addActionListener(listener, XtNcolorLimitsApply);
    }
}

ColorSelection::ColorSelection(const string &name, Component *parent,
		ConPlotClass *conplot_class, const string &rgb_list)
		: ParamDialog(name, parent)
{
    createInterface();
    init();
    initColors(rgb_list);
    enableCallbackType(XmNvalueChangedCallback);
    enableCallbackType(XtNcolorLimitsApply);
    conplot = conplot_class;
}

void ColorSelection::createInterface(void)
{
    Arg args[10];
    int n;

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    limits_button = new Button("Limits...", controls, this);
    reset_button = new Button("Reset", controls, this);
    reset_button->setSensitive(false);
    save_button = new Button("Save", controls, this);
    save_button->setSensitive(false);
    default_button = new Button("Default", controls, this);
    default_button->setSensitive(false);
    plus_button = new Button("+", controls, this);
    minus_button = new Button("-", controls, this);
    bar_button = new Button("|", controls, this);
    help_button = new Button("Help", controls, this);
    help_button->setSensitive(false);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, true); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginWidth, 50); n++;
    rc = new RowColumn("rc", form, args, n);

    RowColumn *scale_rc;
    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    scale_rc = new RowColumn("vrc", rc, args, n);
    new Label("R", scale_rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 255); n++;
    XtSetArg(args[n], XmNvalue, 128); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    red_scale = new Scale("Red", scale_rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    scale_rc = new RowColumn("vrc", rc, args, n);
    new Label("G", scale_rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 255); n++;
    XtSetArg(args[n], XmNvalue, 128); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    green_scale = new Scale("Green", scale_rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    scale_rc = new RowColumn("vrc", rc, args, n);
    new Label("B", scale_rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 255); n++;
    XtSetArg(args[n], XmNvalue, 128); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    blue_scale = new Scale("Blue", scale_rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    scale_rc = new RowColumn("vrc", rc, args, n);
    new Label("H", scale_rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, -1); n++;
    XtSetArg(args[n], XmNmaximum, 359); n++;
    XtSetArg(args[n], XmNvalue, 180); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    hue_scale = new Scale("Hue", scale_rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    scale_rc = new RowColumn("vrc", rc, args, n);
    new Label("S", scale_rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 255); n++;
    XtSetArg(args[n], XmNvalue, 128); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    sat_scale = new Scale("Saturation", scale_rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    scale_rc = new RowColumn("vrc", rc, args, n);
    new Label("V", scale_rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    XtSetArg(args[n], XmNdecimalPoints, 0); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 255); n++;
    XtSetArg(args[n], XmNvalue, 128); n++;
    XtSetArg(args[n], XmNshowValue, true); n++;
    val_scale = new Scale("Value", scale_rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XtNheight, 200); n++;
    XtSetArg(args[n], XtNwidth, 350); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XtNusePixmap, false); n++;
    plot1 = new HistgmClass("plot1", form, args, n, this);

    createLimitsDialog();
}

void ColorSelection::createLimitsDialog(void)
{
    Arg args[20];
    int n;

    limits_window = new FormDialog("Color Limits", this);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    limits_controls = new RowColumn("limits_controls", limits_window, args, n);

    limits_close_button= new Button("Close", limits_controls, this);
    limits_close_button->setCommandString("Limits Close");
    limits_apply_button = new Button("Apply", limits_controls, this);
    limits_help_button = new Button("Help", limits_controls, this);
    limits_help_button->setSensitive(false);
    limits_controls->setHelp(limits_help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, limits_controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    limits_sep = new Separator("sep", limits_window, args, n);

    XmString xm = createXmString("Auto Color Limits");
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    auto_limits_toggle = new Toggle("auto_limits", limits_window, args, n);

    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, true); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, auto_limits_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, limits_sep->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    limits_rc = new RowColumn("limits_rc", limits_window, args, n);

    minimum_label = new Label("Minimum", limits_rc);
    XtSetArg(args[0], XmNcolumns, 10);
    minimum_text = new TextField("minimum_text", limits_rc, this, args, 1);
    maximum_label = new Label("Maximum", limits_rc);
    maximum_text = new TextField("maximum_text", limits_rc, this, args, 1);

    ignore_set_text = false;
}

typedef struct
{
    unsigned int r;
    unsigned int g;
    unsigned int b;
} RGBColor;

void ColorSelection::init(void)
{
    int num1 = 51;
    RGBColor rgb_init1[] = {
	{255,0,0}, {255,56,0}, {255,76,0}, {255,93,0}, {255,110,0},
	{255,123,0}, {255,144,0}, {255,161,0}, {255,170,0}, {255,187,0},
	{255,199,0}, {255,208,0}, {255,222,0}, {255,255,0}, {218,255,0},
	{208,255,0}, {191,255,0}, {170,255,0}, {148,255,0}, {102,255,0},
	{0,255,0}, {0,255,119}, {0,255,144}, {0,255,157}, {0,255,187},
	{0,255,255}, {0,216,255}, {0,204,255}, {0,178,255}, {0,161,255},
	{0,144,255}, {0,131,255}, {0,114,255}, {0,97,255}, {0,80,255},
	{0,56,255}, {0,28,255}, {0,0,255}, {60,0,255}, {93,0,255}, {119,0,255},
	{144,0,255}, {170,0,255}, {195,0,255}, {212,0,255}, {255,0,255},
	{255,0,208}, {255,0,123}, {255,0,140}, {255,0,106}, {255,0,55},
    };

    display = XtDisplay(base_widget);

    if((cmap = DefaultColormap(display, DefaultScreen(display)))
		== (Colormap)NULL)
    {
	cerr << "Cannot get colormap." << endl;
    }

    original_num_colors = 0;
    selected_box = -1;   /* for rgb boxes */
    num_colors = 0;
    num_stipples = 0;
    num_lines = 0;
    num_colors_init = 0;
    num_bins = 0;
    normalize = false;
    data_min = 0.;
    data_max = 0.;
    for(int i = 0; i < MAX_COLORS; i++) {
	r[i] = 0;
	g[i] = 0;
	b[i] = 0;
        r0[i] = 0;
	g0[i] = 0;
	b0[i] = 0;
        plane_masks[i] = 0;
        lines[i] = 0.;
        distribution[i] = 0.;
        original_distribution[i] = 0.;
    }

    num_colors_init = num1;
    for(int i = 0; i < num_colors_init; i++) {
	r_init[i] = rgb_init1[i].r;
	g_init[i] = rgb_init1[i].g;
	b_init[i] = rgb_init1[i].b;
    }
    conplot = NULL;
}

void ColorSelection::initColors(const string &rgb_list)
{
    int i;
    char *prop;
    bool new_colors;
    double percentage;

    if(!limits_window) {
	createLimitsDialog();
    }

    num_stipples = 0;

    new_colors = false;

    if((prop = getProperty(getName())) == NULL) {
	prop = strdup(rgb_list.c_str());
    }
    if(prop)
    {
	char *c, *tok, *last, *p;
	bool new_format = false;

	p = strdup(prop);

	if((c = strtok_r(p, ",", &last)) != NULL &&
	    sscanf(c, "%d %d %d %f",&r[0],&g[0],&b[0],&distribution[0]) == 4)
	{
	    new_format = true;
	}
	Free(p);

	i = 0;
	tok = prop;
	while((c = strtok_r(tok, ",", &last)) != NULL && i < MAX_COLORS)
	{
	    if(new_format) {
		if(sscanf(c, "%d %d %d %f", &r[i], &g[i], &b[i],
			&distribution[i]) != 4) break;
		if(distribution[i] < 0.) distribution[i] = 0.;
		if(distribution[i] > 1.) distribution[i] = 1.;
	    }
	    else {
		if(sscanf(c, "%d %d %d", &r[i], &g[i], &b[i]) != 3) break;
	    }
	    tok = NULL;
	    if(i < num_colors_init) {
		if(r[i] != r_init[i] || g[i] != g_init[i] || b[i] != b_init[i]){
		    new_colors = true;
		}
	    }
	    r[i] *= 256;
	    g[i] *= 256;
	    b[i] *= 256;
	    r0[i] = r[i];
	    g0[i] = g[i];
	    b0[i] = b[i];
	    i++;
	}
	num_colors = i;
	if(!new_format && num_colors > 0) {
	    percentage = 1./(double)num_colors;
	    for(i = 0; i < num_colors; i++) {
		distribution[i] = i*percentage;
	    }
	}
	distribution[num_colors] = 1.;
	if(num_colors && num_colors != num_colors_init) new_colors = true;
	Free(prop);
    }
    if(new_colors) {
	default_button->setSensitive(true);
    }

    if(num_colors == 0)
    {
	num_colors = num_colors_init;
	percentage = 1./(double)num_colors;
	for(i = 0; i < num_colors; i++) {
	    r[i] = r_init[i]*256;
	    g[i] = g_init[i]*256;
	    b[i] = b_init[i]*256;
	    r0[i] = r[i];
	    g0[i] = g[i];
	    b0[i] = b[i];
	    distribution[i] = i*percentage;
	}
	distribution[num_colors] = 1.;
    }

    if(getColorPixels() != num_colors)
    {
	/* can't get all num_colors colors. Try default num_colors_init */
	if(num_colors > num_colors_init)
	{
	    num_colors = num_colors_init;
	    percentage = 1./(double)num_colors;
	    for(i = 0; i < num_colors_init; i++) {
		r[i] = r_init[i]*256;
		g[i] = g_init[i]*256;
		b[i] = b_init[i]*256;
		r0[i] = r[i];
		g0[i] = g[i];
		b0[i] = b[i];
		distribution[i] = i*percentage;
	    }
	    distribution[num_colors] = 1.;
	    if((i = getColorPixels()) != num_colors)
	    {
		cerr << "Can't allocate all colors for " << getName() << endl;
		num_colors = i;
	    }
	}
#ifdef _ZZ_ZZ
	if(!num_colors)
	{
	    /* can't get any colors. use stipples */
	    s->num_colors = 0;
	    s->num_stipples = 10;
	    ConPlotStipples(conplot, s->num_stipples, (char *)stipples,
                                STIPPLE_WIDTH, STIPPLE_HEIGHT);
	    return;
	}
#endif
    }

    original_num_colors = num_colors;
    for(i = 0; i <= num_colors; i++) {
	original_distribution[i] = distribution[i];
    }

    setRgbControls(0);

    for(i = 0; i <= num_colors; i++) lines[i] = distribution[i];
    float data[2];
    data[0] = lines[0];
    data[1] = lines[num_colors];
    computeBins(2, 2, data, -1.);
}

void ColorSelection::setInitialColors(int ncolors, unsigned int *Red,
		unsigned int *Green, unsigned int *Blue)
{
    if(ncolors > MAX_COLORS) ncolors = MAX_COLORS;
    num_colors = ncolors;
    double percentage = 1./(double)num_colors;
    for(int i = 0; i < num_colors; i++) {
	r[i] = Red[i]*256;
	g[i] = Green[i]*256;
	b[i] = Blue[i]*256;
	r0[i] = r[i];
	g0[i] = g[i];
	b0[i] = b[i];
	distribution[i] = i*percentage;
    }
    distribution[num_colors] = 1.;

    getColorPixels();

    setRgbControls(0);
}

void ColorSelection::computeBins(int nbins, int npts, float *data,
				float no_data_flag)
{
    int i;

    data_min = 0.;
    data_max = 0.;

    for(i = 0; i < npts && data[i] == no_data_flag; i++);
    if(i < npts) {
	data_min = data[i];
	data_max = data[i];
    }

    for(i = 0; i < npts; i++) if(data[i] != no_data_flag)
    {
	if(data_min > data[i]) data_min = data[i];
	if(data_max < data[i]) data_max = data[i];
    }

    computeBins(nbins, npts, data, no_data_flag, data_min, data_max);
}

void ColorSelection::computeBins(int nbins, int npts, float *data,
				float no_data_flag, double min, double max)
{
    int i, j;
    double bin_width;

    num_bins = (nbins <= 2*MAX_COLORS) ? nbins : 2*MAX_COLORS;
    data_min = min;
    data_max = max;
    for(i = 0; i < num_bins; i++) bins[i] = 0.;

    bin_width = (data_max - data_min)/num_bins;

    if(bin_width == 0.) return;

    for(i = 0; i < npts; i++) if(data[i] != no_data_flag)
    {
	j = (int)((data[i] - data_min)/bin_width);
        if(j < 0) j = 0;
        else if(j >= num_bins) j = num_bins-1;
	bins[j] += 1.;
    }

    for(i = 0; i < num_bins; i++) if(bins[i] != 0.)
    {
	bins[i] = log10(bins[i]);
    }
    makeLines();
    plot1->input(num_colors, pixels, 0, NULL, lines, num_bins, bins,
		data_min, data_max);
}

void ColorSelection::computeBins(int nbins, int npts, double *data)
{
    int i, j;
    double bin_width;

    data_min = 0.;
    data_max = 0.;

    if(npts > 0) {
	data_min = data[0];
	data_max = data[0];
    }

    for(i = 0; i < npts; i++) {
	if(data_min > data[i]) data_min = data[i];
	if(data_max < data[i]) data_max = data[i];
    }

    num_bins = (nbins <= 2*MAX_COLORS) ? nbins : 2*MAX_COLORS;
    for(i = 0; i < num_bins; i++) bins[i] = 0.;

    bin_width = (data_max - data_min)/num_bins;

    if(bin_width == 0.) return;

    for(i = 0; i < npts; i++)
    {
	j = (int)((data[i] - data_min)/bin_width);
        if(j < 0) j = 0;
        else if(j >= num_bins) j = num_bins-1;
	bins[j] += 1.;
    }

    for(i = 0; i < num_bins; i++) if(bins[i] != 0.)
    {
	bins[i] = log10(bins[i]);
    }
    makeLines();
    plot1->input(num_colors, pixels, 0, NULL, lines, num_bins, bins,
		data_min, data_max);
}

int ColorSelection::getColorPixels()
{
    if(!cmap || XDisplayCells(display, DefaultScreen(display)) < num_colors) {
	return 0;
    }

    for(int i = 0; i < num_colors; i++) {
	colors[i].red   = r[i];
	colors[i].green = g[i];
	colors[i].blue  = b[i];

	if(!XAllocColor(display, cmap, &colors[i])) {
	    return i;
	}
	pixels[i] = colors[i].pixel;
    }
    return num_colors;
}

ColorSelection::~ColorSelection(void)
{
}

void ColorSelection::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    XmScaleCallbackStruct *s =
		(XmScaleCallbackStruct *)action_event->getCalldata();
    double hue, sat, val;
    ColorSelectionStruct data;

    if(!strcmp(cmd, "Red")) {
	r[selected_box] = s->value*256;
	colors[selected_box].red = r[selected_box];
	rgbCB(action_event);
    }
    else if(!strcmp(cmd, "Green")) {
	g[selected_box] = s->value*256;
	colors[selected_box].green = g[selected_box];
	rgbCB(action_event);
    }
    else if(!strcmp(cmd, "Blue")) {
	b[selected_box] = s->value*256;
	colors[selected_box].blue = b[selected_box];
	rgbCB(action_event);
    }
    else if(!strcmp(cmd, "Hue")) {
	// convert the existing color to hsv
	rgbToHsv((double)(r[selected_box]/256), (double)(g[selected_box]/256),
		(double)(b[selected_box]/256), &hue, &sat, &val);
	hue = (double)s->value;
	hsvCB(action_event, hue, sat, val);
    }
    else if(!strcmp(cmd, "Saturation")) {
	// convert the existing color to hsv
	rgbToHsv((double)(r[selected_box]/256), (double)(g[selected_box]/256),
		(double)(b[selected_box]/256), &hue, &sat, &val);
	sat = (double)s->value;
	hsvCB(action_event, hue, sat, val);
    }
    else if(!strcmp(cmd, "Value")) {
	// convert the existing color to hsv
	rgbToHsv((double)(r[selected_box]/256), (double)(g[selected_box]/256),
		(double)(b[selected_box]/256), &hue, &sat, &val);
	val = (double)s->value;
	hsvCB(action_event, hue, sat, val);
    }
    else if(!strcmp(cmd, "plot1")) {
	if(!strcmp(action_event->getReason(), XtNselectCallback)) {
	    selected_box = *(int *)action_event->getCalldata();
	    if(selected_box >= 0) setRgbControls(selected_box);
	}
	else {
	    colorStretchCB(action_event);
	}
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Limits Close")) {
	if(limits_window) limits_window->setVisible(false);
    }
    else if(!strcmp(cmd, "Limits...")) {
	if(!limits_window) {
	    createLimitsDialog();
	}
	limits_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Reset")) {
	colorLinesReset();
	data.reason = RESET_COLORS;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "Save")) {
	colorLinesSave();
	data.reason = SAVE_COLORS;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "Default")) {
	colorLinesDefault();
	data.reason = DEFAULT_COLORS;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "+")) {
	addColor();
	data.reason = ADD_COLOR;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "-")) {
	removeColor();
	data.reason = REMOVE_COLOR;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "|")) {
	setColors(num_colors-1, num_colors);
	resetDistribution();
	plot1->colors(num_colors, pixels);
	data.reason = DISTRIBUTE_COLORS;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "Apply")) {
	resetDistribution();
	data.reason = LIMITS_APPLY;
	callback(comp->baseWidget(), &data);
    }
    else if(!strcmp(cmd, "minimum_text") || !strcmp(cmd, "maximum_text")) {
	if(!ignore_set_text) {
	    auto_limits_toggle->set(false, false);
	}
    }
    else if(comp == help_button) {
	showHelp("Color Selection Help");
    }
    else if(comp == limits_help_button) {
	showHelp("Color Limits Help");
    }
}

void ColorSelection::rgbCB(ActionEvent *a)
{
    if(XAllocColor(display, cmap, &colors[selected_box])) {
	pixels[selected_box] = colors[selected_box].pixel;
    }
    plot1->colors(num_colors, pixels);

    Widget w = a->getSource()->baseWidget();
    callback(w, (XtPointer)a->getCalldata());
    reset_button->setSensitive(true);
    save_button->setSensitive(true);
    default_button->setSensitive(true);
}

void ColorSelection::hsvCB(ActionEvent *a, double hue, double sat, double val)
{
    double Red, Green, Blue;
    /* map the new hsv value to rgb */
    hsvToRgb(hue, sat, val, &Red, &Green, &Blue);

    r[selected_box] = (unsigned int) Red * 256;
    g[selected_box] = (unsigned int) Green * 256;
    b[selected_box] = (unsigned int) Blue  * 256;

    setRgbControls(selected_box);

    colors[selected_box].red = r[selected_box];
    colors[selected_box].green = g[selected_box];
    colors[selected_box].blue = b[selected_box];
    if(XAllocColor(display, cmap, &colors[selected_box])) {
	pixels[selected_box] = colors[selected_box].pixel;
    }
    plot1->colors(num_colors, pixels);

    Widget w = a->getSource()->baseWidget();
    callback(w, (XtPointer)a->getCalldata());
    reset_button->setSensitive(true);
    save_button->setSensitive(true);
    default_button->setSensitive(true);
}

void ColorSelection::colorStretchCB(ActionEvent *a)
{
    double *l = (double *)a->getCalldata(), d;
    int i, n;

    n = (num_colors > 0) ? num_colors : num_stipples;
    num_lines = n+1;

    d = l[num_lines-1] - l[0];
    if(d == 0.) d = 1.;
    for(i = 0; i < num_lines; i++) {
	distribution[i] = (l[i] - l[0])/d;
    }

    makeLines();

    reset_button->setSensitive(true);
    save_button->setSensitive(true);
    default_button->setSensitive(true);

    Widget w = a->getSource()->baseWidget();
    callback(w, (XtPointer)a->getCalldata());
}

void ColorSelection::colorLinesReset(void)
{
    int i;

    num_lines = original_num_colors + 1;
    for(i = 0; i < num_lines; i++) {
	distribution[i] = original_distribution[i];
    }

    num_colors = original_num_colors;
    for(i = 0; i < num_colors; i++) {
	r[i] = r0[i];
	g[i] = g0[i];
	b[i] = b0[i];
	colors[i].red   = r[i];
	colors[i].green = g[i];
	colors[i].blue  = b[i];

	if(XAllocColor(display, cmap, &colors[i])) {
	    pixels[i] = colors[i].pixel;
	}
    }

    plot1->colors(num_colors, pixels);
    makeLines();
    plot1->lines(num_colors, num_stipples, lines);
    save_button->setSensitive(false);
    reset_button->setSensitive(false);
}

void ColorSelection::setAutoLimits(bool set)
{
    if(auto_limits_toggle->state() != set) {
	auto_limits_toggle->set(set, false);
    }
}

void ColorSelection::makeLines(void)
{
    int i;
    bool auto_color;

    num_lines = num_colors + 1;
    if(num_lines <= 1) return;

    auto_color = auto_limits_toggle->state();

    if(normalize && auto_color) {
	for(i = 0; i < num_lines; i++) {
	    lines[i] = distribution[i];
	}
    }
    else if(num_lines > 1)
    {
	double diff = data_max - data_min;
	if(diff > 0.) {
	    for(i = 0; i < num_lines; i++) {
		lines[i] = data_min + distribution[i]*diff;
	    }
	}
	else {
	    for(i = 0; i < num_lines; i++) {
		lines[i] = distribution[i];
	    }
	}
    }
    if(num_lines >1 && fabs(lines[num_lines-1]) < .001*fabs(lines[num_lines-2]))
    {
	lines[num_lines-1] = 0.;
    }
    if(num_lines > 1 && fabs(lines[0]) < .001*fabs(lines[1])) {
	lines[0] = 0.;
    }
}

void ColorSelection::addColor(void)
{
    if(num_colors < MAX_COLORS)
    {
	setColors(num_colors-1, num_colors+1);
    }
    /* redistribute evenly */
    resetDistribution();
    plot1->colors(num_colors, pixels);

    save_button->setSensitive(true);
    default_button->setSensitive(true);
    reset_button->setSensitive(true);
}

void ColorSelection::removeColor(void)
{
    if(num_colors > 2)
    {
	setColors(num_colors-1, num_colors-1);
    }
    /* redistribute evenly */
    resetDistribution();
    plot1->colors(num_colors, pixels);

    save_button->setSensitive(true);
    default_button->setSensitive(true);
    reset_button->setSensitive(true);
}

void ColorSelection::setColors(int index, int new_num_colors)
{
    int     i;
    double h0, s0, v0;
    double hN, sN, vN;
    double hStep, sStep, vStep;
    double Red, Green, Blue;

    /* find hsv of first and last color, and
       evenly distribute all colors in between
     */
    rgbToHsv((double) (r[0]/256), (double) (g[0]/256), (double) (b[0]/256),
		&h0, &s0, &v0);

    rgbToHsv((double) (r[index]/256), (double) (g[index]/256),
                 (double) (b[index]/256), &hN, &sN, &vN);

    hStep = (hN-h0)/(new_num_colors - 1);
    sStep = (sN-s0)/(new_num_colors - 1);
    vStep = (vN-v0)/(new_num_colors - 1);

    for(i = 1; i < new_num_colors; i++)
    {
	hsvToRgb(h0+(hStep*i), s0+(sStep*i), v0+(vStep*i),
		&Red, &Green, &Blue);

	r[i] = (unsigned int)Red * 256;
	g[i] = (unsigned int)Green * 256;
	b[i] = (unsigned int)Blue * 256;
	r0[i] = (unsigned int)Red * 256;
	g0[i] = (unsigned int)Green * 256;
	b0[i] = (unsigned int)Blue * 256;
	colors[i].red   = r[i];
	colors[i].green = g[i];
	colors[i].blue  = b[i];

	if(XAllocColor(display, cmap, &colors[i])) {
	    pixels[i] = colors[i].pixel;
	}
    }
    num_colors = new_num_colors;
}

void ColorSelection::resetDistribution(void)
{
    int i;
    double percentage;

    percentage = 1./(double)num_colors;
    for(i = 0; i < num_colors; i++) {
	distribution[i] = i*percentage;
    }
    distribution[num_colors] = 1.;
    num_lines = num_colors + 1;
    makeLines();
    plot1->lines(num_colors, num_stipples, lines);
}

void ColorSelection::colorLinesDefault()
{
    double percentage;
    int i;

    num_colors = num_colors_init;
    num_lines = num_colors + 1;
    percentage = 1./(double)num_colors;
    for(i = 0; i < num_colors; i++) {
	r[i] = r_init[i]*256;
	g[i] = g_init[i]*256;
	b[i] = b_init[i]*256;
	r0[i] = r[i];
	g0[i] = g[i];
	b0[i] = b[i];
	colors[i].red   = r[i];
	colors[i].green = g[i];
	colors[i].blue  = b[i];
	if(XAllocColor(display, cmap, &colors[i])) {
	    pixels[i] = colors[i].pixel;
	}
	distribution[i] = i*percentage;
    }
    distribution[num_colors] = 1.;

    makeLines();

    plot1->input(num_colors, pixels, 0, NULL, lines, num_bins, bins, data_min,
			data_max);

    save_button->setSensitive(true);
    default_button->setSensitive(false);
}

void ColorSelection::colorLinesSave(void)
{
    int     i;
    size_t  n;
    bool    new_colors;
    char    buf[100], value[5000];

    if(num_colors > 0)
    {
	n = 1;
	value[0] = '\0';
	for(i = 0; i < num_colors; i++) {
	    snprintf(buf, 100, "%d %d %d %.3f,",
		r[i]/256,g[i]/256,b[i]/256,distribution[i]);
	    n += strlen(buf);
	    if(n >= sizeof(value)) break;
	    strcat(value, buf);
	}
	putProperty(getName(), value);
	Application::writeApplicationProperties();
    }
    save_button->setSensitive(false);
    reset_button->setSensitive(false);

    for(i = 0; i < num_colors; i++) {
	r0[i] = r[i];
	g0[i] = g[i];
	b0[i] = b[i];
    }
    new_colors = false;
    if(num_colors != num_colors_init)
    {
	new_colors = true;
    }
    for(i = 0; i < num_colors; i++) {
	if(r[i] != r_init[i]*256 || g[i] != g_init[i]*256 ||
		b[i] != b_init[i]*256)
	{
	    new_colors = true;
	    break;
	}
    }
    if(new_colors) {
	default_button->setSensitive(true);
    }
}

void ColorSelection::setRgbControls(int index)
{
    red_scale->setValue(r[index]/256);
    green_scale->setValue(g[index]/256);
    blue_scale->setValue(b[index]/256);

    setHsvControls(index);
}

void ColorSelection::setHsvControls(int index)
{
    int ihue;
    double  hue, sat, val;

    rgbToHsv((double) (r[index]/256), (double) (g[index]/256),
		(double) (b[index]/256), &hue, &sat, &val);
    
    ihue = (int)hue;
    if(ihue < -1) ihue = -1;
    hue_scale->setValue(ihue);
    sat_scale->setValue((int)sat);
    val_scale->setValue((int)val);
}

void ColorSelection::setup(int ncolors, unsigned int *Red, unsigned int *Green,
                unsigned int *Blue, float *Dist)
{
    num_colors = ncolors;
    for(int i = 0; i < ncolors; i++) {
	r[i] = Red[i];
	g[i] = Green[i];
	b[i] = Blue[i];
    }
    num_lines = num_colors+1;
    for(int i = 0; i < num_colors; i++) {
	distribution[i] = Dist[i];
    }
}

void ColorSelection::callback(Widget w, XtPointer data)
{
    updateConplot();
    if( ((ColorSelectionStruct *)data)->reason == LIMITS_APPLY) {
	doCallbacks(w, data, XtNcolorLimitsApply);
    }
    else {
	doCallbacks(w, data, XmNvalueChangedCallback);
    }
}

void ColorSelection::updateConplot(void)
{
    if(conplot) {
	conplot->setColors(getPixels(), numColors());
	conplot->colorLines(numLines(), colorLines());
    }
}

void ColorSelection::updateLimits(double *min, double *max, int ndeci)
{
    ignoreTextInput(true);

    if( !autoLimits() )
    {
	double fixed_min, fixed_max;

	if(minimum_text->getDouble(&fixed_min)) {
	    if(fixed_min > *max) {
		setAutoLimits(true);
	    }
	    else {
		*min = fixed_min;
	    }
	}
	else {
	    setAutoLimits(true);
	}

	if(maximum_text->getDouble(&fixed_max)) {
	    if(fixed_max < *min)
	    {
		setAutoLimits(true);
	    }
	    else {
		*max = fixed_max;
	    }
	}
	else {
	    setAutoLimits(true);
	}
    }
    if( autoLimits() )
    {
	char s[50];
        ftoa(*min, ndeci, 0, s, sizeof(s));
        minimum_text->setString(s, false);
        ftoa(*max, ndeci, 0, s, sizeof(s));
        maximum_text->setString(s, false);
    }

    ignoreTextInput(false);
}
