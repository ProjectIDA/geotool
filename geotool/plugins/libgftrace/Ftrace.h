#ifndef _FTRACE_H
#define _FTRACE_H

#include "motif++/MotifDecs.h"
#include "WaveformWindow.h"
#include "Beam.h"


/** @defgroup libgftrace plugin Ftrace
 */

namespace libgftrace {

class FtraceParam;

/** Ftrace window.
 *  @ingroup libgftrace
 */
class Ftrace : public WaveformWindow
{
    public:
	Ftrace(const char *, Component *, DataSource *ds);
	~Ftrace(void);

	virtual void setDataSource(DataSource *ds);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	// File Menu
	Button *compute_button;

	// Option Menu
	Toggle *auto_beam_loc;
	Button *param_button;

	// Help Menu
	Button *help_button;

	Label *lat_label, *lon_label;
	TextField *az_text, *slo_text, *lat_text, *lon_text;
	TextField *len_text, *snr_text, *flo_text, *fhi_text;

	FtraceParam *param_window;
	WaveformPlot *wp;
	string cursor;
	Beam *gbeam;

	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	void compute(void);
	void newWindow(void);
	int getArrayElements(gvector<Waveform *> &wlist, double *tmin,
			double *tmax);

	ParseCmd parseCompute(const string &cmd, string &msg);
	ParseCmd parseParams(const string &cmd, string &msg);
};

} // namespace libgftrace


#endif
