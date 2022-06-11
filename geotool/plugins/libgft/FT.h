#ifndef _FT_H
#define _FT_H

#include "motif++/Frame.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "motif++/MotifDecs.h"

/** @defgroup libgft plugin FT
 */

class WaveformPlot;
class AxesLabels;
class DataSource;
class AxesClass;
class RefSta;
class PrintDialog;

namespace libgft {

class FtPlotClass;
class FtSmooth;
class FtWindows;
class TaperPopup;

/** FT window.
 *  @ingroup libgft
 */
class FT : public Frame, public DataReceiver, public PrintClient
{
    friend class FkSignal;

    public:
	FT(const char *, Component *, DataSource *);
	FT(const char *, Component *, DataSource *, bool);
	~FT(void);

	virtual void print(FILE *fp, PrintParam *p);
	void setDataSource(DataSource *ds);
	void compute(void) { compute(true); }
	void compute(bool warn);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd parseDisplayData(const string &cmd, string &msg);
	ParseCmd parseTaper(const string &c, string &msg);
	void parseHelp(const char *prefix);

    protected:
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	FtPlotClass	*plot1;
	AxesLabels	*labels_window;
	TaperPopup	*taper_window;
	PrintDialog	*print_window;
	FtSmooth	*smoothing_window;
	FtWindows	*windows_dialog;
	WaveformPlot	*wp;
	bool		real_time;
	vector <FT *>	windows;

//	RTDWindow rtd;

	// File menu
	Button *compute_button, *input_button, *output_button;
	Button *print_button, *new_ft_button, *close_button;
	Menu *cursor_menu;
	Toggle *cursor_a_toggle, *cursor_b_toggle, *cursor_c_toggle;
	Toggle *cursor_d_toggle;

	// Edit menu
	Button *clear_button, *delete_button, *save_button, *smooth_button;

	// View menu
	Menu *x_axis_menu;
	Toggle *freq_toggle, *log_freq_toggle;
	Toggle *period_toggle, *log_period_toggle;
	Menu *y_axis_menu;
	Toggle *amp_toggle, *power_toggle;

	Menu *displacement_menu;
	Toggle *disp_dB_rel_nm, *disp_dB_rel_m, *disp_nm, *disp_m, *disp_log_nm;
	Toggle *disp_log_m;

	Menu *velocity_menu;
	Toggle *vel_dB_rel_nm, *vel_dB_rel_m, *vel_nm, *vel_m, *vel_log_nm;
	Toggle *vel_log_m;

	Menu *accel_menu;
	Toggle *accel_dB_rel_nm, *accel_dB_rel_m, *accel_nm, *accel_m;
	Toggle *accel_log_nm, *accel_log_m;

	Menu *display_menu;
	Toggle *input_traces_toggle, *mean_toggle, *median_toggle;
	Toggle *std_dev_toggle, *percentiles_toggle, *nlnm_toggle;
	Toggle *nhnm_toggle;

	Toggle *draw_dc_toggle;
	Button *fill_button;
	Toggle *grid_toggle;
	Button *labels_button;
	Button *limits_button;
	Button *unzoom_button;

	// Option menu
	Toggle *auto_cursor_toggle, *rtd_compute_toggle, *demean_toggle;
	Menu *taper_menu;
	Toggle *hanning_toggle, *hamming_toggle, *cosine_toggle, *parzen_toggle;
	Toggle *welch_toggle, *blackman_toggle, *none_toggle;
	Button *taper_percent_button;
	Toggle *instrument_toggle;
	Button *windows_button;

	// Help menu
	Button *ft_help_button;

	Toggle *toggles[24];

	void createInterface(void);

	void init(void);
	void setButtonsSensitive(bool set);
	void setAxes(const char *code, bool save);
	void actionPerformed(ActionEvent *action_event);
	void print(void);
	ParseCmd parseYAxis(const string &cmd, string &msg);

    private:

};

} // namespace libgft

#endif
