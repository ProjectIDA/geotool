#ifndef _CLUSTER_H
#define _CLUSTER_H

#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "motif++/Frame.h"
#include <mccc.h>

class TabClass;
class WaveformView;
class WaveformPlot;
class DataSource;
class Table;
class GTimeSeries;
class Waveform;

namespace libgcluster {

typedef struct GClusterParam_s
{
    int		nclusters;
    double	min_xcorr_th;
} GClusterParam;

class GClusterParams;
class GClusterSummary;

class GCluster : public Frame, public DataReceiver, public PrintClient
{
    public:
	GCluster(const char *, Component *, DataSource *);
	~GCluster(void);

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
	Button *align_button, *copy_button,
          *vandecar_crosson_cluster_s_button, *shearer_cluster_s_button,
          *vandecar_crosson_cluster_m_button, *shearer_cluster_m_button;

	// Help menu
	Button *help_button;

	Pane *pane;
	Table *table;
	TabClass *tab;
	WaveformView **plot;
	WaveformPlot *wp;

	GClusterParams *param_window;
        GClusterSummary *summary_window;
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
	int num_clusters;
	int *clusterids;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	bool getParam(struct GClusterParam_s *cp);
	void print(void);
	void compute(const Method mccc_method, const char cluster_method);
	void clusterAdd(Waveform *w, GTimeSeries *mccc_ts,
			int clusterid);
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

} // namespace libgcluster

#endif /* !_CLUSTER_H */
