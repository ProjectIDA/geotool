#ifndef _FK_SIGNAL_H
#define _FK_SIGNAL_H

#include "motif++/FormDialog.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "widget/AxesClass.h"
#include "motif++/MotifDecs.h"

class CPlotClass;
class WaveformView;
class ScrolledPane;

namespace libgfk {

class FK;

#define NUM_SIGNAL_PLOTS 7

class FkSignal : public FormDialog, public PrintClient
{
    friend class FK;

    public:
	FkSignal(const char *name, Component *parent, FK *fk);
	~FkSignal(void);

	void print(FILE *fp, PrintParam *p);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);

	void clear(void);
	void addCurve(int curve_index, int num, double *x, float *y,
			char *label, Pixel fg);

    protected:

	InfoArea	*info_area;
	RowColumn	*controls, *rc;
	Button		*close_button, *print_button, *param_button;
	Toggle		*unwrap_toggle, *show_toggles[4];
	Button		*auto_compute_button, *start_button, *back_button;
	Button		*forward_button, *save_button, *help_button;
	Separator	*sep;
	Label		*window_length_label, *window_overlap_label;
	TextField	*window_length_text, *window_overlap_text;
	Label		*stav_length_label, *ltav_length_label;
	TextField	*stav_length_text, *ltav_length_text;
	TextField	*bandwidth_text;
	Label		*bandwidth_label;
	Toggle		*scan_frequencies_toggle;
	Pane		*pane;
	ScrolledPane	*scrolled_pane;
	CPlotClass	*plots[NUM_SIGNAL_PLOTS+1];
	WaveformView	*beam_plot;
	PrintDialog	*print_window;

	FK	*fk;
	bool	ignore_callback;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void tieLimits(Component *comp, AxesLimitsCallbackStruct *s);
	void print(void);
	void getPlotOrder(int *order);

    private:
};

} // namespace libgfk

#endif
