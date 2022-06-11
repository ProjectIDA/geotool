#ifndef _FK_DATA_H
#define _FK_DATA_H

#include <stdio.h>

#include "gobject++/gvector.h"
extern "C" {
#include "tapers.h"
}

#ifndef _FCOMPLEX_
#define _FCOMPLEX_
typedef struct fcomplex {
	float re;
	float im;
} FComplex;
#endif

/**
 *  C++ classes that performs frequency-wavenumber (FK) analysis and computes
 *  beamed waveforms.
 *  This library contains classes for performing FK analysis for array data.
 *  The FKData class holds the FK results. The FKGram class has functions for
 *  analyzing the time varying FK properties. The Beam class has functions for
 *  reading beam recipes and computing waveform beams.
 *  @ingroup libgbeam
 */
class Waveform;

/** The maximum number of different frequency bands that the FKData object
 *  can hold.
 */
#define MAX_NBANDS      16

/** Holds the fine-grid FK for all frequency bands. The fine grid is not
 *  necessarily centered at slowness equal to zero. The x slowness values can
 *  be differen from the y slowness values.
 *  @ingroup libgbeam
 */
class FKFine
{
    public:
	FKFine(void) {
	    nbands=0;
	    n_slowfine = 0;
	    for(int i = 0; i < MAX_NBANDS; i++) {
		n_fine[i] = 0;
		jmax[i] = -1;
		kmax[i] = -1;
		fk_fine[i] = NULL;
		slowfine_xmin[i] = 0.;
		slowfine_ymin[i] = 0.;
		d_slowfine[i] = 0.;
	    }
	}
	int		nbands; //!< the number of frequency bands.
	int		n_slowfine; //!< the number of slowness values
	/** the number of FK values for each band (n_slowfine * n_slowfine) */
	int		n_fine[MAX_NBANDS];
	/** the FK values for each frequency band */
	int		jmax[MAX_NBANDS];
	int		kmax[MAX_NBANDS];
	float		*fk_fine[MAX_NBANDS];
	double		slowfine_xmin[MAX_NBANDS]; //!< x-slowness min value
	double		slowfine_ymin[MAX_NBANDS]; //!< y-slowness min value
	double		d_slowfine[MAX_NBANDS]; //!< x and y slowness interval

        ~FKFine()
	{
	    for(int i = 0; i < nbands; i++) Free(fk_fine[i]);
	}
};

/** Class for FK parameters.
 *  @ingroup libgbeam
 */
class FKArgs : public Gobject
{
    public:
	bool output_power; //!< if true, output power db instead of amplitude dB.
	bool full_compute; //!< if true, compute a single FK and save all frequencies.
	bool three_component; //!< if true, compute a three-component FK.
	bool fine_grid; //!< if true, compute fine grid.

	double slowness_max; //!< (sec/km) the maximum slowness value of the FK grid
	int num_slowness; //!< the number of slowness values. (must be odd.)
	int num_bands; //!< the number of frequency bands.
	double fmin[MAX_NBANDS]; //!< an array of minimum frequencies for the bands.
	double fmax[MAX_NBANDS]; //!< an array of maximum frequencies for the bands.
/** (sec/km) the minimum slowness value for the signal max. Ignored if < 0. */
	double signal_slow_min;
/** (sec/km) the maximum slowness value for the signal max. Ignored if < 0. */
	double signal_slow_max;
/** (degrees) the minimum azimuth value for the signal max. Ignored if < 0. */
	double signal_az_min;
/** (degrees) the maximum azimuth value for the signal max. Ignored if < 0. */
	double signal_az_max;
	int taper_type; //!< the taper type
/** for a cosine taper, the percentage of the waveform to taper at the start. */
	double taper_beg;
/** for a cosine taper, the percentage of the waveform to taper at the end. */
	double taper_end;

	FKArgs(void) {
	    output_power = false;
	    full_compute = false;
	    three_component = false;
	    fine_grid = true;

	    slowness_max = .4;
	    num_slowness = 81;
	    num_bands = 0;
	    for(int i = 0; i < MAX_NBANDS; i++) {
		fmin[i] = 0.;
		fmax[i] = 0.;
	    }
	    fmin[0] = 0.5;
	    fmax[0] = 6.0;
	    signal_slow_min = -1.;;
	    signal_slow_max = -1.;;
	    signal_az_min = -1.;;
	    signal_az_max = -1.;;
	    taper_type = COSINE_TAPER;
	    taper_beg = .05;
	    taper_end = .05;
	}
        ~FKArgs() {}
};

/** A class for waveform FK analysis. An instance of this class holds the FK
 *  values for a particular grid of slowness values and for one or more
 *  frequency bands. The FK grid is centered at slowness 0. The x and y
 *  slowness values are the same. Member functions compute a single FK for
 *  each frequency band, or a time series of FKs for a sliding time window.
 *  The time series of FKs is called an FKgram.
 *  @ingroup libgbeam
 */
class FKData : public Gobject
{
    public:
	FKData(gvector<Waveform *> &wvec, int windowed,
		FKArgs args);
	FKData(gvector<Waveform *> &wvec, double tmin,
		double tmax, FKArgs args);
	~FKData(void);
	Gobject *clone(void);

	static void fit2d(double h[3][3], double *x, double *y);

	/** Find the location of the maximum FK value without a grid mask. */
	void findMax(float *fk=NULL, int b=0, bool *peak_mask=NULL);
	bool write(FILE *fp);
	bool read(FILE *fp);
	int waveformId(int i) { return waveform_id[i]; }

	void fullCompute(gvector<Waveform *> &wvec,
		double tmin, double tmax, FKArgs args);
	void searchFBands(double scan_bandw, double flow, double fhigh,
		double sig_slow_min, double sig_slow_max, double sig_az_min,
		double sig_az_max);


	int		nwaveforms;
	double		shift;
	char		net[10];
	int		windowed;
	FKArgs		args;
	int		nt;
	int		nf;
	int		if1;
	int		if2;
	double		dt;
	double 		df;
	double		tbeg;
	double		tend;
	double		d_slowness;

	double		scaling[MAX_NBANDS];
	double		local_average[MAX_NBANDS];
	int		jmax[MAX_NBANDS];
	int		kmax[MAX_NBANDS];
	double		xmax[MAX_NBANDS];
	double		ymax[MAX_NBANDS];
	double		fk_max[MAX_NBANDS];
	double		restricted_fkmax[MAX_NBANDS];
	double		total_power[MAX_NBANDS];
	double		fstat[MAX_NBANDS];
	float		*fk[MAX_NBANDS];

	double		*lon, *lat;
	bool		scan_spectrum;
	double		scan_bandw;
	char		center_sta[10];
	double		center_lat;
	double		center_lon;
	FKFine		fine;
	float		*fcomplete;
	float		*complete;

	int		*waveform_id;

	FKData(gvector<Waveform *> &wvec);
	void init(gvector<Waveform *> &wvec, FKArgs fk_args);
	void checkTaper(int taper);
	void applyTaper(float *t, int npts);

	void compute(gvector<Waveform *> &wvec, int windows,
		FKArgs args);
	void compute(gvector<Waveform *> &wvec, double tmin,
		double tmax, FKArgs args);
	bool getCoordinates(gvector<Waveform *> &wvec,
		double *lat, double *lon);
	void compute3C(gvector<Waveform *> &wvec,
		int windowed, FKArgs args);
	void compute3C(gvector<Waveform *> &wvec,
		double tmin, double tmax, FKArgs args);
	bool *createPeakMask(double signal_slow_min, double signal_slow_max,
		double signal_az_min, double signal_az_max);
	void computeFineGrid(int *f_lo, int *f_hi, FComplex *f,
		double *fsum1, double *fsum2);


    private:
	FKData() {}
};

#endif
