#ifndef _COLORSELECTON_H
#define _COLORSELECTON_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"
#include "motif++/Toggle.h"
#include "motif++/TextField.h"

#define RESET_COLORS		0
#define SAVE_COLORS		1
#define ADD_COLOR		2
#define REMOVE_COLOR		3
#define DEFAULT_COLORS		4
#define DISTRIBUTE_COLORS	5
#define LIMITS_APPLY		6

#define MAX_COLORS	256

#define XtNcolorLimitsApply	(char *)"colorLimitsApply"

typedef struct
{
	int	reason;
} ColorSelectionStruct;

class HistgmClass;
class ConPlotClass;

/**
 *  @ingroup libgx
 */
class ColorSelection : public ParamDialog
{
    public:
	ColorSelection(const string &name, Component *parent,
		ActionListener *listener=NULL, const string &rgb_list="");
	ColorSelection(const string &name, Component *parent,
		ConPlotClass *conplot, const string &rgb_list="");
	~ColorSelection(void);

	void actionPerformed(ActionEvent *action_event);

	void colorLinesReset(void);
	void addColor(void);
	void removeColor(void);
	void resetDistribution(void);
	void colorLinesDefault(void);
	void colorLinesSave(void);
	void ignoreTextInput(bool set) { ignore_set_text = set; }
	void setAutoLimits(bool set);
	void setMinimum(char *min) { minimum_text->setString(min, false); }
	void setMaximum(char *max) { maximum_text->setString(max, false); }
	bool getMinimum(double *min) { return minimum_text->getDouble(min); }
	bool getMaximum(double *max) { return maximum_text->getDouble(max); }
	bool autoLimits(void) { return auto_limits_toggle->state(); }
	void computeBins(int num_bins, int npts, float *data, float exc);
	void computeBins(int nbins, int npts, float *data, float no_data_flag,
			double min, double max);
	void computeBins(int nbins, int npts, double *data);

	int numColors(void) { return num_colors; }
	int numLines(void) { return num_lines; }
	int numStipples(void) { return num_stipples; }
	double *colorLines(void) { return lines; }
	Pixel *getPixels(void) { return pixels; }
	unsigned int *red(void) { return r; }
	unsigned int *green(void) { return g; }
	unsigned int *blue(void) { return b; }
	float *dist(void) { return distribution; }
	void setup(int num_colors, unsigned int *red, unsigned int *green,
		unsigned int *blue, float *distribution);
	void updateLimits(double *min, double *max, int ndeci);
	void setInitialColors(int num_colors, unsigned int *red,
		unsigned int *green, unsigned int *blue);


    protected:

	Button		*reset_button;
	Button		*save_button;
	Button		*default_button;
	Button		*apply_button;
	Button		*close_button;
	Button		*limits_button;
	Button		*plus_button;
	Button		*minus_button;
	Button		*bar_button;
	Button		*help_button;

	Separator	*sep;
	Form		*form;
	RowColumn	*rc, *controls;
	Scale		*red_scale, *green_scale, *blue_scale;
	Scale		*hue_scale, *sat_scale, *val_scale;
	HistgmClass	*plot1;
	FormDialog	*limits_window;
	RowColumn	*limits_rc, *limits_controls;
	Button		*limits_close_button, *limits_help_button;
	Button		*limits_apply_button;
	Separator	*limits_sep;
	Toggle		*auto_limits_toggle;
	Label		*minimum_label, *maximum_label;
	TextField	*minimum_text, *maximum_text;

	Colormap	cmap;
	Display		*display;

	unsigned int	r[MAX_COLORS], g[MAX_COLORS], b[MAX_COLORS];
	unsigned int 	r0[MAX_COLORS], g0[MAX_COLORS], b0[MAX_COLORS];
	unsigned int	r_init[MAX_COLORS], g_init[MAX_COLORS],
			b_init[MAX_COLORS];
	XColor          colors[MAX_COLORS];
	unsigned int	plane_masks[MAX_COLORS];
	Pixel		pixels[MAX_COLORS];
	double		lines[MAX_COLORS+1];
	float		distribution[MAX_COLORS+1];
	float		original_distribution[MAX_COLORS+1];
	int		original_num_colors;
	int		selected_box;
	int		num_colors, num_stipples, num_lines, num_colors_init;
	int		num_bins;
	float		bins[2*MAX_COLORS+1];
	bool		normalize;
	double		data_min, data_max;
	ConPlotClass	*conplot;

	void createInterface(void);
	void createLimitsDialog(void);
	void init(void);
	void initColors(const string &rgb_list);
	void setRgbControls(int index);
	void setHsvControls(int index);
	void setColors(int index, int new_num_colors);
	int getColorPixels();
	void makeLines(void);
	void rgbCB(ActionEvent *action_event);
	void hsvCB(ActionEvent *action_event, double hue,double sat,double val);
	void colorStretchCB(ActionEvent *action_event);
	void updateConplot(void);
	void callback(Widget w, XtPointer data);

    private:
	int last_theme_id;
	int last_shape_index;
	bool ignore_set_text;
	bool private_cells;
};

#endif
