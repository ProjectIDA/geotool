#ifndef _POLARIZATION_H
#define _POLARIZATION_H

#include "motif++/MotifDecs.h"
#include "motif++/Frame.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"

/** @defgroup libgpolar plugin Polarization
 */

class DataSource;
class CPlotClass;

namespace libgpolar {

class PolarParam;
class PolarWaveformView;

/** Polarization window.
 *  @ingroup libgpolar
 */
class Polarization : public Frame, public DataReceiver, public PrintClient
{
    public:
	Polarization(const char *, Component *, DataSource *ds);
	~Polarization(void);

	virtual void setDataSource(DataSource *ds);
	void setVisible(bool visible);
	void print(FILE *fp, PrintParam *p);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);
	bool save(const string &file, string &msg);

    protected:
	// File Menu
	Button *compute_button, *print_button, *new_polar_button;
	Button *close_button;

	// Option Menu
	Toggle *auto_toggle;
	Button *param_button;

	// Help Menu
	Button *help_button;

	Pane *pane;
	CPlotClass *plot[5];
	PolarWaveformView *wplot;
	PolarParam *param_window;
	PrintDialog *print_window;

	WaveformPlot *wp;
	vector<Polarization *> windows;
	bool ignore_limits_callback;
	char polar_sta[10], polar_chan[10], polar_net[10];
	char last_inc_label[20], last_az_label[20];
	int cdz_id, cdn_id, cde_id, npts;
	double polar_tbeg, polar_window_width, samprate;
	double last_window_length;

	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
//	void print(void);
	void tieLimits(CPlotClass *plot, AxesLimitsCallbackStruct *a);
	void compute(void);
	void lineDrag(CPlotClass *cp, AxesCursorCallbackStruct *c);
	double positionLine(CPlotClass *p, CPlotClass *plot, double x,
			int reason);
	void newWindow(void);
	void print(void);
	void rotate(void);

    private:

};

} // namespace libgpolar

#endif
