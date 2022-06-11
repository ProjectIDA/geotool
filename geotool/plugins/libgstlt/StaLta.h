#ifndef _STALTA_H
#define _STALTA_H

#include "motif++/Frame.h"
#include "DataReceiver.h"
#include "motif++/MotifDecs.h"
#include "gobject++/DataSource.h"
#include "gobject++/cvector.h"

extern "C" {
#include "libstalta.h"
}
class Table;
class WaveformPlot;
class DataMethod;

/** @defgroup libgstlt plugin StaLta
 */

namespace libgstlt {

extern "C" {
  typedef struct
  {
    double	time;
    double	duration;
    double	snr;
    double	cfreq;
    char	group[16];
    char	sta[8];
    long	arid;
    int		bandIndex;
    int		dataIndex;
    int		state;
  } DetectCandidate, *DetectCandidateP;
} /* extern "C" */

/** StaLta window.
 *  @ingroup libgstlt
 */
class StaLta : public Frame, public DataReceiver
{
    public:
	StaLta(const char *, Component *, DataSource *);
	~StaLta(void);

	virtual void setDataSource(DataSource *ds);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	// File Menu
	Button *close_button;

	// Edit Menu
	Toggle *arrivals_toggle;

	// View Menu
	Menu *select_menu;
	Button *select_all_button, *deselect_all_button;

	// Option Menu
	Button *apply_button;
	Toggle *rtd_compute_toggle;

	// Help Menu
	Button *help_button;

	RowColumn *rc;
	Button *more_button, *less_button;
	Table *table;

	WaveformPlot *wp;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *action_event);
	void setButtonsSensitive(void);
	void more(void);
	void less(void);
	void apply(void);

	void detectOnWaveforms(gvector<Waveform *> &wvec,
		StaLtaDef *staltadef, int nrecipes, DetectCandidate **c,
		int *nC);
	int addDetectArrivals(gvector<Waveform *> &wvec,
		StaLtaDef *staltadef, DetectCandidate *c, int nC);
	bool getStaLtaDef(StaLtaDef **staltadef, int *nrecipes);
	int applyDetector(const char * net, GTimeSeries *ts, int dataIndex,
		StaLtaDef s, int bandIndex, double cfreq, DetectCandidate **c,
		int *nC);
	void showAllCandidates(DetectCandidate *c, int nC);
	bool processCandidates(StaLtaDef *s, DetectCandidate *c, int nC);
	void compareWithArrivals(cvector<CssArrivalClass> &arrivals,
		StaLtaDef *s, DetectCandidate *c, int nC);
	GTimeSeries * returnTS(char * group, gvector<Waveform *> &wvec,
		 int pos, int *nUsed, bool *restoreNeeded);
	void restoreWaveform(Waveform *wvec,
		gvector<DataMethod *> *orig_methods);
	int nearbyCandidates(DetectCandidate *c, int nC, int i,
		double trgsepSec);
	int bestSNR(DetectCandidate *c, int i, int j);

    private:

};

} // namespace libgstlt

#endif
