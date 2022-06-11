#ifndef _SPECTRO_H
#define _SPECTRO_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "gobject++/CssTables.h"

/** @defgroup libgspectro plugin Spectrogram
 */

class ColorSelection;
class AxesLabels;
class DataSource;
class ConPlotClass;
class WaveformView;
class WaveformPlot;
class GTimeSeries;
class AxesClass;
class Waveform;
class Response;

namespace libgspectro { 

class SpectroParam;
class SpectroWaveformView;

/** Spectrogram window.
 *  @ingroup libgspectro
 */
class Spectro : public Frame, public DataReceiver, public PrintClient
{
    friend class SpectroWaveformView;

    public:
	Spectro(const char *name, Component *parent, DataSource *ds);
	Spectro(const char *name, Component *parent, DataSource *ds,
		bool mainWindow);
	~Spectro(void);

	PrintDialog	*print_window;

	virtual void print(FILE *fp, PrintParam *p);
	virtual void setDataSource(DataSource *ds);

	void setProjection(const char *name);
	void setColorMap(int num, Pixel *pixels);
	Pixel getColor(const char *name);
	void readFile(const char *path);
	void compute(bool warn=true);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

	static int get_line(FILE *fp, char *line, int len, int *line_no);

    protected:
	Pane		*pane;
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	ConPlotClass	*plot1;
	SpectroWaveformView	*plot2;
        WaveformPlot	*wp;
	SpectroParam	*parameter_window;
 	ColorSelection	*colors_window;
	AxesLabels	*labels_window;
	vector<Spectro *> windows;

	Button	*compute_button, *open_button, *save_button, *print_button;
	Button	*close_button, *new_spectro, *colors, *labels, *parameters;
	Button	*help;

	Toggle	*bin_average_toggle;
	Toggle	*log_data_toggle;
	Toggle	*instrument_toggle;
	Toggle	*normalize_toggle;
	Toggle	*auto_cursor;
	Toggle	*rtd_compute;

	bool	init_colors;
	int	n_windows;
	int	nf;
	double	*x;
	double	*y;
	float	*data;

	double	window_length;
	double	window_overlap;
	double	f_min;
	double	f_max;
	double	min;
	double	max;
	double	samprate;
	float	exc;
	bool	normalize;
	bool	auto_color_limits;
	bool	bin_average;
	bool	instrument_corr;
	bool	add_callbacks;

	char	sta[10];
	char	chan[9];
	char	net[9];
	double	time;
	float	tlen;
	int	winpts;
	int	overlap;
	float	lofreq;
	float	hifreq;

	char	ts_label[20];
	Pixel	fg;
	int	w_id;
	int	selection_warning;
	bool	ignore_limits_callback;


	void createInterface(void);
	void init(void);
	void subMenuChoice(const char *, Toggle **t);
	void actionPerformed(ActionEvent *action_event);
	void print(void);
	void initColors(void);
	void setColorBar(double, double);
	void open(void);
	bool spectro(Waveform *w, GTimeSeries *ts, bool windowed,
		vector<Response *> *rsp);
	void drawSpectro(Waveform *w, GTimeSeries *ts);

	void cursorPlot1(AxesCursorCallbackStruct *p);
	void cursorPlot2(AxesCursorCallbackStruct *p);
	void tieLimits(AxesClass *plot, AxesLimitsCallbackStruct *a);

	static void sMinMax(float *s, int npts, float exc, double *min,
			double *max);

    private:
	int last_theme_id;
	int last_shape_index;
};

} // namespace libgpsectro

#endif
