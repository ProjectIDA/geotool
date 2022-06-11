#ifndef _SELFSCAN_H
#define _SELFSCAN_H

#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "motif++/Frame.h"

#define MAX_SEGMENTS 200

class TabClass;
class WaveformView;
class WaveformPlot;
class DataSource;
class Table;
class GTimeSeries;
class Waveform;

namespace libgselfscan {

typedef struct SelfScanParam_s
{
    double	windowlength;
    int		nclusters;
    double	min_corr_th;
} SelfScanParam;

class SelfScanParams;
class SelfScanSummary;

class SelfScan : public Frame, public DataReceiver, public PrintClient
{
    public:
	SelfScan(const char *, Component *, DataSource *);
	~SelfScan(void);

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
        Button *summary_button;   

	// Option menu
	Button *parameters_button;
	Button *align_button, *copy_button, *cluster_s_button, *cluster_m_button;

	// Help menu
	Button *help_button;

	Pane *pane;
	Table *table;
	TabClass *tab;
	WaveformView **plot;
	WaveformPlot *wp;

	SelfScanParams *param_window;
        SelfScanSummary *summary_window;
	PrintDialog *print_window;

	typedef struct {
	  char sta[10];
	  char chan[10];
	  double max_coef;
	  double time;
	  double time_lag;
	  double abs_corr_lag;
	  double rel_corr_lag;
	} SelfScanStruct;

	int num_selfscan;
	SelfScanStruct *selfscan;
	int num_clusters;
	int *clusterids;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	bool getParam(struct SelfScanParam_s *cp);
	void print(void);
	void compute(const char cluster_method);
	void clusterAdd(Waveform *cd, GTimeSeries *selfscan_ts, int clusterid);
	void setButtonsSensitive(void);
	void copyWaveform(void);
	void clear_clusters(void);
	void clear(void);
	void remove(void);
	void selectWaveform(void);
	void selectRow(void);
	void align(void);
	void list(void);

    private:
	char *cluster_title_format;
	char * getClusterTitle(int clusterid);

	float * getData(GTimeSeries *ts, int *npts);


};

} // namespace libgselfscan

#endif /* !_SELFSCAN_H */
