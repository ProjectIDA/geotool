/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_FK_H_
#define _LIB_FK_H_

#include "libgcss.h"
#include "libstalta.h"
#include "gobject/GObjectP.h"
#include "libFT.h"

#define FK_SEC_PER_DEG	0
#define FK_SEC_PER_KM	1

#define MAXNUMFK      16


typedef struct FKStaLta
{
        double  beamSecBefore;
        double  beamSecAfter;
        double  peakHeightThreshold0;
        double  FKSlopeThreshold0;
        double  peakHeightThreshold1;
        double  FKSlopeThreshold1;
        double  peakHeightThreshold2;
        double  FKSlopeThreshold2;
        double  slowThreshold2;
        double  avgVThreshold2;

        double  peakHeightThreshold3;
        double  FKSlopeThreshold3;
        double  slowThreshold3;
        double  freqThreshold3;

        double  detTimeBeforePeak;
        double  detTimeBeforeLargePeak;
        double  detTimeAfterPeak;
        double  slowForHalfSTA;
        double  snrThreshold;
        double  peakGroupDuration;

        double  slowThreshold;
        double  slowSnrThreshold;

        double  heightForHigherTrigger;
        double  heightForLargePeak;
        double  slopeForLargePeak;
        int     writeTimeSeries;
} FKStaLta_t;

typedef struct
{
	GObjectPart	core;

	int		num_waveforms;
	int		*waveform_id;
	double		shift;
	char		sta[10];
	int		windowed;
	int		taper;
	int		nbands;
	float		begTaper;
	float		endTaper;
	double		tbeg;
	double		tend;
	double		slowness_min;
	double		slowness_max;
	int		n_slowness;
	double		d_slowness;
	double		f_min[MAXNUMFK];
	double		f_max[MAXNUMFK];
	char		center_sta[10];
	double		center_lat;
	double		center_lon;
	double		local_average[MAXNUMFK];
	double		xmax[MAXNUMFK];
	double		ymax[MAXNUMFK];
	double		fk_max[MAXNUMFK];
        double          fstat[MAXNUMFK];
	float		*fk[MAXNUMFK];
} _FKData, *FKData;

typedef struct
{
	int		n_slowfine;
	int		n_fine[MAXNUMFK];
	float		*fk_fine[MAXNUMFK];
	double		slowfine_xmin[MAXNUMFK];
	double		slowfine_ymin[MAXNUMFK];
	double		d_slowfine[MAXNUMFK];
} FKFine;

typedef struct FKGram_struct *FKGram;
typedef struct FKGram3C_struct *FKGram3C;

typedef struct
{
	float		*fk_max;
	float		*appvel;
	float		*az;
	float		*slowness;

	int		*slope;		/* derived values */
} FKSignal;

typedef struct
{
	float	az;
	float	slow;
	double	peakTime;	/* time of the peak */
	double	dStart;		/* start of sta/lta detection */
	double	dEnd;		/* end of sta/lta detection */
	int	band;		/* index of the filter band used to find the peak */
	int	minpos;
	int	maxpos;
	float	durSec;		/* duration of the peak in seconds */
	float	relativeHeight;
	float	absoluteHeight;
	float	snr;		/* maximum ration value returned by sta/lta detector */
	int	detection;
	float	meanFKSlope;

        float   htrig;          /* htrig used by the sta/lta detector */
        float   ltrig;          /* ltrig used by the sta/lta detector */
        float   staSec;         /* short term avg (sec) used by the sta/lta detector */
        float   ltaSec;         /* long term avg (sec) used by the sta/lta detector */

	int	nV;
	float	avgV;
	float	maxV;
	float	v[20];
} FKPeak;

typedef struct
{
	char		net[10];
	char		sta[10];
	char		chan[10];
	bool		three_component;
	int		nbands;
	int		fk_units;

	int		num_fkdata;
	FKData		*fkdata;
	FKData		single_fk;
	int		num_waveforms;
	int		*waveform_ids;
	float		min_fk[MAXNUMFK];
	float		max_fk[MAXNUMFK];
	double		window_length;
	double		window_overlap;
	bool		beam_input;

        /* signal measurement parameters */
	StaLtaDef	staltadef;
	FKStaLta_t	f;

        /* signal measurements */
        FKSignal        sig[MAXNUMFK];
	float		*x;
	int		nPeak;
	FKPeak		*peak;

	FKGram		fg;
	FKGram3C	fg3C;
} FKParam;


/* ****** FKCompute.c ********/
FKData FKCompute(int num_waveforms, CPlotData **cd_list, int windowed,
		double slowness_min, double slowness_max, int n_slowness,
		double d_slowness, int nbands, double *f_min, double *f_max,
		FKFine *fine, int taper, float begTaper, float endTaper);
void FKFit2d(double h[3][3], double *x, double *y);

void recordTaper(FKData fk_data, int taper, float begTaper, float endTaper);
void applyTaper(FKData fk_data, float *t, int npts);

/* ****** FKCompute3C.c ********/
FKData FKCompute3C(int num_waveforms, CPlotData **cd_list, int windowed,
		double slowness_min, double slowness_max, int n_slowness,
		double d_slowness, int nbands, double *f_min, double *f_max,
		FKFine *fine, int taper, float begTaper, float endTaper);

/* ****** FKGram.c ********/
int FKComputeGram(int num_waveforms, CPlotData **cd_list, int windowed,
	double slowness_min, double slowness_max, int n_slowness,
	double d_slowness, int nbands, double *f_min, double *f_max,
	double window_length, double window_overlap, bool show_working,
	int *num_fks, FKData **fkdata, int taper, float begTaper, float endTaper);

FKGram new_FKGram(int num_waveforms, CPlotData **cd_list, int windowed,
	double slowness_min, double slowness_max, int n_slowness,
	double d_slowness, int nbands, double *fmin, double *fmax,
	double window_length, double window_overlap, bool show_working, int taper, float begTaper, float endTaper);

int FKGram_compute(FKGram fg, int num_waveforms, CPlotData **cd_list,
                int *num_fks, FKData **p_fkdata);

int FKComputeGram3C(int num_waveforms, CPlotData **cd_list, int windowed,
	double slowness_min, double slowness_max, int n_slowness,
	double d_slowness, int nbands, double *fmin, double *fmax,
	double window_length, double window_overlap, bool show_working,
	int *num_fks, FKData **fkdata, int taper, float begTaper, float endTaper);

FKGram3C new_FKGram3C(int num_waveforms, CPlotData **cd_list, int windowed,
	double slowness_min, double slowness_max, int n_slowness,
	double d_slowness, int nbands, double *fmin, double *fmax,
	double window_length, double window_overlap, bool show_working, int taper, float begTaper, float endTaper);

int FKGram3C_compute(FKGram3C fg, int num_waveforms, CPlotData **cd_list,
	int *num_fks, FKData **p_fkdata);

FKData new_FKData(int num_waveforms, CPlotData **cd_list);

/* ****** FKio.c ********/
bool FKWrite(FILE *fp, FKParam *p);
bool FKWriteData(FILE *fp, FKData f);
bool FKRead(FILE *fp, FKParam *p);
bool FKReadData(FILE *fp, FKData f);

/* ****** FKSignal.c ********/
void FKProcessSignal(FKParam *p);
void FKReportPeaks(FKParam *p);

void FKLog(FKParam *p);
void FKFindMinMax(FKParam *p);
void FKComputeAzSlow(int fk_units, double x, double y, float *az, float *app_vel,
	float *slowness);
void crosshair_to_slow_az(int fk_units, double scaled_x, double scaled_y,
	double *sec_per_km, double *az);
void FKAllocSignal(FKParam *p);
void FKUpdateSignalMeasurements(FKParam *p, bool unwrapAz);
void FKAzSlowToSxSy(float obsAzimuth, float obsSlowness, float * sx, float *sy);


/* ****** FKPeaks.c ********/
int FKProcessPeaks(FKParam *p, CPlotData **cd_list, char *prefix);

/* ****** timeSeries.c ********/
void GParseWriteTimeSeries(char *prefix,  const char *sta, const char *chan,
			TimeSeries ts);

#endif
