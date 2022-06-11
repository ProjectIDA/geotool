#ifndef _MULTICHANNELCC_H
#define _MULTICHANNELCC_H

#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "motif++/Frame.h"
#include <mccc.h>

/** @defgroup libgmccc plugin libgmccc
 *  C++ classes for multi-channel cross-correlation
 */

class WaveformView;
class WaveformPlot;
class DataSource;
class Table;
class GTimeSeries;
class Waveform;

namespace libgmccc {

/** @ingroup libgmccc
 */
class MultiChannelCC : public Frame, public DataReceiver, public PrintClient
{
    public:
	MultiChannelCC(const char *, Component *, DataSource *);
	~MultiChannelCC(void);

	virtual void print(FILE *fp, PrintParam *p);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	// File menu
	Button *print_button, *close_button;

	// Edit menu
	Button *clear_button, *delete_button;

	// View menu
	Button *attributes_button;

	// Option menu
	Button *align_button, *copy_button,
          *vandecar_crosson_button, *shearer_button;

	// Help menu
	Button *help_button;

	Pane *pane;
	Table *table;
	Form *form;
	WaveformView *plot1;
	WaveformPlot *wp;

	PrintDialog *print_window;

	typedef struct {
	  char sta[10];
	  char chan[10];
	  double max_coef;
	  double time;
	  double time_lag;
	  double abs_corr_lag;
	  double rel_corr_lag;
	} MCCCStruct;

	int num_mccc;
	MCCCStruct *mccc;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void print(void);
	void compute(Method method);
	void multiChannelCCAdd(Waveform *w, GTimeSeries *mccc_ts);
	void setButtonsSensitive(void);
	void copyWaveform(void);
	void clear(void);
	void remove(void);
	void selectWaveform(void);
	void selectRow(void);
	void align(void);
	void list(void);

    private:

	float *getData(GTimeSeries *ts, int *npts);

};

} // namespace libgmccc

#endif /* !_MULTICHANNELCC_H */
