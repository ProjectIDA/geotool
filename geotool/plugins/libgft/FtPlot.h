#ifndef	FTPLOT_H_
#define	FTPLOT_H_

#include "widget/Axes.h"
#include "Response.h"

#define XtNtaper 			(char *)"taper"
#define XtNbegLength 			(char *)"begLength"
#define XtNendLength 			(char *)"endLength"
#define XtNmode 			(char *)"mode"
#define XtNunits			(char *)"units"
#define XtNder				(char *)"der"
#define XtNgrouping 			(char *)"grouping"
#define XtNsmoothingWidth		(char *)"smoothingWidth"
#define XtNinstrumentCorr		(char *)"instrumentCorr"
#define XtNdemean			(char *)"demean"
#define XtNdrawDC			(char *)"drawDC"
#define XtNwindows			(char *)"windows"
#define XtNoverlap			(char *)"overlap"
#define XtNpercentile1			(char *)"percentile1"
#define XtNpercentile2			(char *)"percentile2"
#define XtNselectCallback		(char *)"selectCallback"
#define XtNdisplayArrayTraces		(char *)"displayArrayTraces"
#define XtNdisplayArrayMean		(char *)"displayArrayMean"
#define XtNdisplayArrayMedian		(char *)"displayArrayMedian"
#define XtNdisplayArrayPercentile	(char *)"displayArrayPercentile"
#define XtNdisplayArrayStdDev		(char *)"displayArrayStdDev"
#define XtNdisplayArrayNlnm		(char *)"displayArrayNlnm"
#define XtNdisplayArrayNhnm		(char *)"displayArrayNhnm"
#define XtNxAxisPeriod			(char *)"xAxisPeriod"

#define XtCTaper 			(char *)"Taper"
#define XtCBegLength 			(char *)"BegLength"
#define XtCEndLength 			(char *)"EndLength"
#define XtCMode 			(char *)"Mode"
#define XtCUnits			(char *)"Units"
#define XtCDer				(char *)"Der"
#define XtCGrouping 			(char *)"Grouping"
#define XtCSmoothingWidth		(char *)"SmoothingWidth"
#define XtCInstrumentCorr		(char *)"InstrumentCorr"
#define XtCDemean			(char *)"Demean"
#define XtCDrawDC			(char *)"DrawDC"
#define XtCWindows			(char *)"Windows"
#define XtCOverlap			(char *)"Overlap"
#define XtCPercentile1			(char *)"Percentile1"
#define XtCPercentile2			(char *)"Percentile2"
#define XtCSelectCallback		(char *)"SelectCallback"
#define XtCDisplayArrayTraces		(char *)"DisplayArrayTraces"
#define XtCDisplayArrayMean		(char *)"DisplayArrayMean"
#define XtCDisplayArrayMedian		(char *)"DisplayArrayMedian"
#define XtCDisplayArrayPercentile	(char *)"DisplayArrayPercentile"
#define XtCDisplayArrayStdDev		(char *)"DisplayArrayStdDev"
#define XtCDisplayArrayNlnm		(char *)"DisplayArrayNlnm"
#define XtCDisplayArrayNhnm		(char *)"DisplayArrayNhnm"
#define XtCXAxisPeriod			(char *)"XAxisPeriod"

#ifndef XtRDouble
#define XtRDouble	(char *)"double"
#endif

typedef struct _FtPlotClassRec	*FtPlotWidgetClass;
typedef struct _FtPlotRec	*FtPlotWidget;

extern WidgetClass		ftPlotWidgetClass;

#define FT_AMP		0
#define FT_POWER	1
#define FT_PHASE	2

#define	FT_DB_RE_NM	0
#define	FT_DB_RE_M	1
#define	FT_NM		2
#define	FT_M		3
#define	FT_LOG_NM	4
#define	FT_LOG_M	5

#define	FT_DISP		0
#define	FT_VEL		1
#define	FT_ACC		2


#define FT_TMP		0	// entry->type
#define FT_PERM		1	// perm can redo FT
#define FT_STATIC	2	// perm cannot redo FT, read from disk
#define FT_NOISE	3	// perm cannot redo FT, noise spectrum

#define FT_ALL          0
#define FT_STATION      1
#define FT_ARRAY        2

typedef struct
{
	int		id;
	char		sta[7];         /* station name */
	char		chan[9];        /* channel name */
	char		taper[9];
	char		fstype[6];
	char		do_rsp[2];

	double		time;
	double		maxf;
	double		tlen;
	int		nf;
	double		df;
	double		taper_length;
	int		taper_min_pts;
	int		taper_max_pts;
	int		npts;
	int		signal_type;
	double		smoothvalue;
	float		*dB;
	float		*phase;
	float		*t;
} FtPlotData;

void FtPlotClear(FtPlotWidget w);
void FtPlotClearTmps(FtPlotWidget w);
void FtPlotDoAverages(FtPlotWidget w);
void FtPlotWait(FtPlotWidget w);
void FtPlotContinue(FtPlotWidget w);
void FtPlotCompute(FtPlotWidget w, bool redisplay, bool dolimits);
void FtPlotInput(FtPlotWidget w, const char *sta, const char *chan, int npts,
		float *y, double dt, Pixel fg, GTimeSeries *ts, double time);
int FtPlotNumEntries(FtPlotWidget w);
int FtPlotGetData(FtPlotWidget w, FtPlotData **ft_list, Boolean selected_only);
Widget FtPlotCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);
void FtPlotSave(FtPlotWidget w);
void FtPlotWrite(Widget w, FILE *fp);
void FtPlotRead(Widget w, FILE *fp);
Boolean FtPlotWriteEntry(FtPlotWidget w, int id, FILE *fp);
Boolean FtPlotReadEntry(FtPlotWidget w, FILE *fp);
void FtPlotFillSelected(FtPlotWidget w);
void FtPlotDeleteSelected(FtPlotWidget w);
void FtPlotOverlay(FtPlotWidget w, int npts, float *x, float *y,
			Boolean display);
int FtPlotNumSelected(FtPlotWidget w);
void FtPlotSmooth(FtPlotWidget w, double width);
double FtPlotSmoothingWidth(FtPlotWidget w);
void FtPlotSetCosineTaper(FtPlotWidget w, double length, int min_pts,
			int max_pts);
void FtPlotGetCosineTaper(FtPlotWidget w, double *length, int *min_pts,
			int *max_pts);

#endif	/* FTPLOT_H_ */
