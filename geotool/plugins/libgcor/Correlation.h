#ifndef _CORRELATION_H
#define _CORRELATION_H

#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "TableMenu.h"
#include "gobject++/gvector.h"
#include "motif++/Frame.h"
#include "gobject++/GTimeSeries.h"

/** @defgroup libgcor plugin Correlation
 */

class WaveformView;
class DataSource;
class Table;
class GTimeSeries;
class Waveform;
class FileDialog;

namespace libgcor {

class MinCorrOverlap;

enum NormType {
    LOCAL_MEAN,
    GLOBAL_MEAN,
    TOTAL_AMP
};

/** Correlation window.
 *  @ingroup libgcor
 */
class Correlation : public Frame, public DataReceiver, public PrintClient
{
    public:
	Correlation(const char *, Component *, DataSource *);
	~Correlation(void);

	virtual void print(FILE *fp, PrintParam *p);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

	static void correl(float *data1, float *data2, int n, float *c);
	static void norm1(float *data1, int nr, float *data2, int nt, float *c);
	static void norm2(float *data1, int nr, float *data2, int nt, float *c);
	static void norm3(float *data1, int nr, float *data2, int nt, float *c);
	static void fftCorrelate(float *r, int nr, float *t, int nt, float *c,
			NormType norm_type);

	static GTimeSeries * timeCorrelate(GTimeSeries *ref,
			GTimeSeries *target, int ends, NormType norm_type);
	static GTimeSeries * fftCorrelate(GTimeSeries *ref, GTimeSeries *target,
			int rank_order, NormType norm_type);


    protected:
	// File menu
	Button *print_button, *close_button, *output_traces_button;
	Button *output_table_button;

	// Edit menu
	Button *clear_button, *delete_button;

	// View menu
	TableMenu *table_menu;

	// Option menu
	Button *align_button, *copy_button, *correlate_button, *set_ref_button;
	Menu *max_corr_menu;
	Toggle *pos_max_toggle, *abs_max_toggle;
	Menu *norm_type_menu;
	Toggle *local_mean_toggle, *global_mean_toggle, *total_amp_toggle;
	Button *min_overlap_button;

	// Help menu
	Button *help_button;

	Pane *pane;
	Table *table;
	Form *form;
	WaveformView *plot1, *plot2;
	FileDialog *file_dialog;

	PrintDialog *print_window;
	MinCorrOverlap *min_corr_overlap;

	class CorClass : public Gobject
	{
	    public :
	    CorClass(const char *sta_name, const char *chan_name,
			GDataPoint *d_min, GDataPoint *d_max)
	    {
		strcpy(sta, sta_name);
		strcpy(chan, chan_name);
		dmin = d_min;
		dmax = d_max;
	    }
	    char sta[10];
	    char chan[10];
	    GDataPoint *dmin;
	    GDataPoint *dmax;
	    cvector<CssArrivalClass> arrivals;
	    ~CorClass(void) {
		dmin->deleteObject();
		dmax->deleteObject();
	    }
	};

	gvector<CorClass *> *cor;

	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	void print(void);
	void setReference(void);
	void compute(void);
	void correlate(GTimeSeries *ref_ts, Waveform *w, GTimeSeries *ts,
		NormType norm_type, double *duration);
	void correlateAdd(Waveform *w, GTimeSeries *cor_ts);
	void setButtonsSensitive(void);
	void copyWaveform(void);
	void clear(void);
	void remove(void);
	void selectWaveform(void);
	void selectRow(void);
	void align(void);
	void list(void);
	void setupTable(CorClass *cc);
	void getRow(int num_columns, double t0, CorClass *cc0, CorClass *cc,
			const char **row);
	void save(void);

    private:

};

} // namespace libgcor

#endif
