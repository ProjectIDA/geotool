#ifndef	_FTPLOTP_H_
#define	_FTPLOTP_H_

#include <Xm/Xm.h>
#include <vector>

extern "C" {
#include "libgplot.h"
#include "libdrawx.h"
}
#include "widget/AxesP.h"
#include "FtPlot.h"
#include "Response.h"

/** 
 * 
 */
class FtData
{
    public :
    FtData(const char *station, const char *channel, int num, float *data,
		double tbeg, double tdel, bool remove_calib, double Calib,
		double Calper, Pixel color, vector<Response *> *response)
    {
	stringcpy(sta, station, sizeof(sta));
	stringcpy(chan, channel, sizeof(chan));
	npts = num;
	t = (float *)malloc(num*sizeof(float));
	for(int i = 0; i < npts; i++) {
	    t[i] = finite(data[i]) ? data[i] : 0;
	}
	// remove calib
	if(remove_calib) {
	    for(int i = 0; i < npts; i++)  t[i] /= Calib;
	}
	mean = 0.;
	for(int i = 0; i < npts; i++)  mean += t[i];
	if(npts) mean /= (double)npts;

	time = tbeg;
	dt = tdel;
	calib = Calib;
	calper = Calper;
	fg = color;
	rsp = (response) ? new vector<Response *>(*response) : NULL;

	winpts = -1;
	np2 = -1;
	nf = -1;
	nfft = -1;
	overlap = -1;
	taper_length = 0.;
	taper_min_pts = 0;
	taper_max_pts = 0;
	xPow = NULL;
	phase = NULL;
	memset(taper, 0, sizeof(taper));
    }
    ~FtData(void) {
	Free(t);
	Free(xPow);
	Free(phase);
	if(rsp) delete rsp;
    }

    char	sta[8];
    char	chan[9];
    char	taper[9];
    double	calper;
    double	calib;
    double 	dt;
    double	df;
    double	time;
    double	mean;
    int		npts;
    int		np2;
    int		winpts;
    int		nf;
    int		nfft;
    int		overlap;
    double	taper_length;
    int		taper_min_pts, taper_max_pts;
    float	*t;
    float	*xPow;
    float	*phase;
    Pixel	fg;
    vector<Response *> *rsp;
};

class FtEntry
{
    public :

    FtEntry(FtPlotWidget widget);
    FtEntry(FtPlotWidget widget, FtData *ftd);
    FtEntry(FtPlotWidget widget, FtData *ftd, const char *name, float *xp,
		float *phase, Pixel fg);
    virtual ~FtEntry(void);

    void init(FtPlotWidget widget);
    void setUp(FtData *ftd);
    virtual void scaleFt(void);
    virtual void scaleX(void);
    virtual void scaleY(void);

    FtPlotWidget w;
    int		id;
    Pixel	data_fg;
    bool	on;
    bool	selected;
    bool	selectable;
    bool	fill;
    bool	derived_spectrum;
    int		i1, i2;
    int		npts, np2;
    int		nf;
    int		type;
    int		t_length;
    float	*t;
    float	*amp;
    float	*phase;
    float	*x;
    float	*y;
    float	*xPow;
    SegArray 	s;
    RSegArray 	r;
    double	time;
    double 	df;
    double 	dt;
    double	x_min, x_max;
    double	y_min, y_max;
    double	amp_y_min, amp_y_max;
    double	phase_y_min, phase_y_max;
    double	calper;
    double	calib;
    vector<Response *> *rsp;

    char	sta[8];
    char	chan[9];
    char	taper[9];
    float	smoothvalue;
    bool	do_rsp;
};

/** 
 * 
 */
typedef struct
{
	bool		wait;
	bool		instrument_corr;
	bool		demean;
	bool		draw_dc;
	bool		xaxis_period;
	bool		display_array_traces;
	bool		display_array_mean;
	bool		display_array_median;
	bool		display_array_percentile;
	bool		display_array_std_dev;
	bool		display_array_nlnm;
	bool		display_array_nhnm;
	int             windows;
	int             overlap;
	int		mode;			/* FT_AMP, FT_POWER */
	int		units;			/* FT_DB_RE_NM, ... FT_LOG_M */
	int		der;			/* FT_DISP, FT_VEL, FT_ACC */
	int		grouping;
	int		taper;
	int             percentile1, percentile2;
	XRectangle	point_recs[2];
	int		point_x;
	int		point_y;
	int		overlay_npts;
	float		*overlay_x;
	float		*overlay_y;
	bool		overlay_display;
	Pixel		white, black;
	bool		first_call;
	vector<FtData *>  ftdata;
	vector<FtEntry *> entries;
	FtEntry		*select_entry;
	double		taper_length;
	int		taper_min_pts, taper_max_pts;
	double		smoothing_width;
	Gobject		*object_owner;
	GC		gc;
	GC		gc_invert;
	XtCallbackList	select_callbacks;
} FtPlotPart;

/** 
 * 
 */
typedef struct
{
	int	nextid;
} FtPlotClassPart;

/** 
 * 
 */
typedef struct _FtPlotRec
{
	CorePart	core;
	CompositePart	composite;
	AxesPart	axes;
	FtPlotPart	ft_plot;
} FtPlotRec;

/** 
 * 
 */
typedef struct _FtPlotClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	AxesClassPart		axes_class;
	FtPlotClassPart		ft_plot_class;
} FtPlotClassRec;

extern FtPlotClassRec ftPlotClassRec;

/* ******** from FtPlot.c for use in libwgets only ********/

void _FtPlotRedisplay(FtPlotWidget w, Boolean copy_area);
void _FtPlotRedraw(FtPlotWidget w);


#endif	/* _FTPLOTP_H_ */
