#ifndef _WAVEFORM_H_
#define _WAVEFORM_H_

#include "gobject++/Gobject.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"

typedef struct
{
    char	label;
    GDataPoint	*d1;
    GDataPoint	*d2;
} GDataWindow;

class DataMethod;
class CPlotClass;

#define MAX_COMPONENTS  3

/** This class is a container for GTimeSeries object in libwgets
 *  @ingroup libgmethod
 */
class Waveform : public Gobject
{
    public:

	Waveform(GTimeSeries *timeseries);
	Waveform(void);
        Waveform(const Waveform &w);
        Waveform & operator=(const Waveform &w);
	~Waveform(void);

	int getId(void) { return id; }
	bool changeMethod(DataMethod *dm);
	bool changeMethods(int num, DataMethod **dm);
	bool applyMethod(DataMethod *dm);
	bool applyMethods(int num, DataMethod **dm);

	static void sortByY0(gvector<Waveform *> &wvec);
	static void sortBySta(gvector<Waveform *> &wvec);
	static void sortByNet(gvector<Waveform *> &wvec);
	static void sortByChan(gvector<Waveform *> &wvec);
	static void sortByChan2(gvector<Waveform *> &wvec);
	static void sortByTime(gvector<Waveform *> &wvec);
	static void sortByDistance(gvector<Waveform *> &wvec,
			double lat, double lon);
	static void sortByDistance(gvector<Waveform *> &wvec,
			double *lat, double *lon);
	static void sortByBaz(gvector<Waveform *> &wvec,
			double lat, double lon);
	static void sortByBaz(gvector<Waveform *> &wvec,
			double *lat, double *lon);
	static void sortByFileOrder(gvector<Waveform *> &wvec);
	static void sortByDefaultOrder(gvector<Waveform *> &wvec);
	static void initialize(Waveform *w);

	static void setChanSortOrder0(const string &chan_order);
	static void setChanSortOrder1(const string &chan_order);
	static void setChanSortOrder2(const string &chan_order);
	static const char ** memberNames();
	static char *getTag(WaveformTag &tag, const string &sta,
		const string &chan, CssOriginClass *origin, double tbeg, double lat,
		double lon, GTimeSeries *ts);
        static int numMemberNames();
	static void getPieceOfTag(WaveformTag &tag, int member_id,
		const string &sta, const string &chan, CssOriginClass *origin,
		double tbeg, double lat, double lon, GTimeSeries *ts,
		char *str, int str_size);

	const char *sta() { return ts->sta(); }
	const char *net() { return ts->net(); }
	const char *chan() { return channel.c_str(); }
	const char *instype() { return ts->instype(); }
	double lat() { return ts->lat(); }
	double lon() { return ts->lon(); }
	double elev() { return ts->elev(); }
	double tbeg() { return ts->tbeg(); }
	double tend() { return ts->tend(); }
	double dnorth() { return ts->dnorth(); }
	double deast() { return ts->deast(); }
	double hang() { return ts->hang(); }
	double vang() { return ts->vang(); }
	double currentAlpha(void) { return ts->currentAlpha(); }
	double currentBeta(void) { return ts->currentBeta(); }
	double currentGamma(void) { return ts->currentGamma(); }
	double mean() { return ts->mean(); }
	double tdelTolerance() { return ts->tdelTolerance(); }
        DataSource * getDataSource() { return ts->getDataSource(); }
	GSegment *segment(int i) { return ts->segment(i); }
	GDataPoint *lowerBound(double time) { return ts->lowerBound(time); }
	GDataPoint *upperBound(double time) { return ts->upperBound(time); }
	GSegment *segment(double time) { return ts->segment(time); }
	GDataPoint *nearest(double time) { return ts->nearest(time); }
        void setOriginalStart(double start) { ts->setOriginalStart(start); }
        void setOriginalEnd(double end) { ts->setOriginalEnd(end); }
	int length() { return ts->length(); }
	int size() { return ts->size(); }
	gvector<DataMethod *> *dataMethods(void) { return ts->dataMethods(); }
        void setDataMethods(gvector<DataMethod *> *new_methods) {
		ts->setDataMethods(new_methods); }
	bool reread(void) { return ts->reread(); }

	CPlotClass	*cplot;	//!< CPlotWidget holding this data (or NULL)
	GTimeSeries	*ts;		//!< time series structure
	string		channel;	//!< channel name
	bool		visible;	//!< waveform is currently visible
	bool		selected;	//!< waveform is selected
	double		scaled_x0;	//!< current x position of the waveform
	double		scaled_y0;	//!< current y position of the waveform
	int		fg;		//!< color
	int		num_dp;		//!< num of DataPoint structures
	GDataPoint	**dp;		//!< DataPoint structures
	int		num_dw;		//!< num of DataWindow structures
	GDataWindow	*dw;		//!< DataWindow structures
	double		begSelect;	//!< partial waveform begin selection
	double		endSelect;	//!< partial waveform end selection
	Waveform	*c[MAX_COMPONENTS];	//!< all component pointers

	int		file_order;
	int		default_order;
	double		distance;

    protected:
	void init(void);

    private:
	int		id;		// Identifier for each waveform
};

#endif
