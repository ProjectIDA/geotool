#ifndef _SEGMENT_INFO_H
#define _SEGMENT_INFO_H

#include <string.h>
#include <strings.h>
#include <vector>
using namespace std;

#include "gobject++/Gobject.h"
#include "gobject++/CssTables.h"

class WaveformConstraint;

/**
 *  @ingroup libgobject
 */
class ArrivalInfo
{
    public:
	ArrivalInfo() : arid(0), orid(0), time(0.) {
	    memset(net, 0, sizeof(net));
	    memset(chan, 0, sizeof(chan));
	    memset(phase, 0, sizeof(phase));
	}
	~ArrivalInfo(void){}

	int	arid;
	int	orid;
	double	time;
	char	net[10];
	char	chan[20];
	char	phase[9];
};

/**
 *  @ingroup libgobject
 */
class SegmentInfo : public Gobject
{
    public:
	SegmentInfo();
	SegmentInfo(const SegmentInfo &s);
	SegmentInfo & operator=(const SegmentInfo &s) {
	    if(this != &s) {
		arrivals.clear();
		copy(&s);
	    }
	    return *this;
	}
	virtual ~SegmentInfo(void);

	virtual Gobject * clone(void);
	void copy(const SegmentInfo *from);

        void setWfdisc(CssWfdiscClass *wf) {
            if(wf != w) {
                if(w) w->removeOwner(this);
                w = wf;
                if(w) w->addOwner(this);
            }
        }
        CssWfdiscClass *wfdisc(void) { return w; }

	int		path;
	int		id;
	int		format;
	int		file_order;
	int		read_order;
	char		sta[10];
	char		chan[20];
	char		net[10];
	char		refsta[10];
	double		start;
	double		end;
	double		input_start;
	double		input_end;
	int		nsamp;
	double		samprate;
	int		jdate;
	int		nass;
	int		ndef;
	double		station_lat;
	double		station_lon;
	double		station_elev;
	double		station_depth;
	double		dnorth;
	double		deast;
	bool		selected;

	int		origin_id;
	double		origin_time;
	double		origin_lat;
	double		origin_lon;
	double		origin_depth;
	double		origin_delta;
	double		origin_azimuth;

	int		component; // 1: x, 2: y, 3: z
	double		hang;
	double		vang;
		// Euler rotation angles from geocentric to the
		// component coordinate system.
	double		alpha;
	double		beta;
	double		gamma;
	char		x_chan[20];
	char		y_chan[20];
	char		z_chan[20];

	vector <ArrivalInfo> arrivals;
	WaveformConstraint *wc;

        gvector<SegmentInfo *> array_elements;

    protected:
	CssWfdiscClass *w;

};

class WfdiscPeriod
{
    public:
    WfdiscPeriod() {
	tbeg = 0.;
	tend = 0.;
	dir = 0;
	prefix = 0;
	wfdisc_file = 0;
	wfdisc_index = 0;
	pts_needed = 0;
    }
    WfdiscPeriod(const WfdiscPeriod &w) {
	tbeg = w.tbeg;
	tend = w.tend;
	dir = w.dir;
	prefix = w.prefix;
	wfdisc_file = w.wfdisc_file;
	wfdisc_index = w.wfdisc_index;
	pts_needed = w.pts_needed;
	wf = w.wf;
    }
    double	tbeg;		//!< Start time of period.
    double	tend;		//!< End time of period.
    int		dir;		//!< Directory containing the wfdiscs.
    int		prefix;		//!< Prefix for .wfdisc,.arrival,files.
    int		wfdisc_file;	//!< wfdisc filename (complete path).
    int		wfdisc_index;	//!< Index of wfdisc record in .wfdisc
    int		pts_needed;	//!< Argument to BasicSource::readSegment().
    CssWfdiscClass	wf;		//!< wfdisc
};

class WaveformIO : public Gobject
{
    public:
	WaveformIO(void) : wp() {}
	~WaveformIO(void) { }

	virtual Gobject *clone(void) {
	    WaveformIO *css = new WaveformIO();
	    for(int i = 0; i < (int)wp.size(); i++) {
		css->wp.push_back(wp[i]);
	    }
	    return (Gobject *)css;
	}

	vector<WfdiscPeriod> wp;
};

#endif
