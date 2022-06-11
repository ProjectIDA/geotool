#ifndef _FK_GRAM_H
#define _FK_GRAM_H

#include "gobject++/Gobject.h"
#include "gobject++/gvector.h"
#include "FKData.h"

typedef bool (*WorkingCB)(int count, int status, const char *label_format);

class Waveform;
class FKData;
class GSegmentArray;

namespace libgfk {

/** Temporary array space.
 *  @ingroup libgFK
 */
class FKWorkSpace : public Gobject
{
    public:
	FKWorkSpace(void);
	~FKWorkSpace(void);
	void freeSpace(void);

	int	*f_lo;
	int	*f_hi;
	int	t_size;
	int	taper_size;
	int	fsum_size;
	int	f_size;
	int	t0_size;
	int	csx_size;
	int	lags_size;

	float	*taper;
	float	*fsum;
	float	*fsum1, *fsum2, *fsum3, *fsum4, *fsum5, *fsum6, *fsum7, *fsum8;
	float	*complete;
	float	*fk;
	FComplex *f;
	double	*lat;
	double	*lon;
	double	*t0;
	double	*csx;
	double	*snx;
	double	*csy;
	double	*sny;
	double	*lags;
	double	*t;
	gvector<Waveform *> wvec;
};

/** This class computes a time series of FKData objects. FKData objects are
 *  computed for a sliding data window.
 *  @ingroup libgFK
 */
class FKGram : public Gobject
{
    public:
	FKGram(gvector<Waveform *> &wvec, int windows,
		double max_slowness, int num_slowness, int num_bands,
		double *f_min, double *f_max, double win_length,
		double win_overlap, WorkingCB working_cb, int taper_type,
		double taper_beg, double taper_end, bool scan_frequencies=false,
		double scan_bandwidth=0.);
	FKGram(gvector<Waveform *> &wvec, int windows,
		double max_slowness, int num_slowness, int num_bands,
		double *f_min, double *f_max, double win_length,
		double win_overlap, WorkingCB working_cb,
		double sig_slow_min, double sig_slow_max,
		double sig_az_min, double sig_az_max, int taper_type,
		double taper_beg, double taper_end,
		bool scan_frequencies=false, double scan_bandwidth=0.);
	~FKGram(void);

	int compute(gvector<Waveform *> &wvec,
			bool append=false, double save_time_secs=0.);
	int fullCompute(gvector<Waveform *> &wvec, int index);

	gvector<Waveform *> waveforms;
	int		windowed;
	int		n_slowness;
	double		d_slowness;
	double		slowness_max;
	int		nbands;
	double		fmin[MAX_NBANDS];
	double		fmax[MAX_NBANDS];
	double		total_power[MAX_NBANDS];
	double		scaling[MAX_NBANDS];
	double		signal_slow_min;
	double		signal_slow_max;
	double		signal_az_min;
	double		signal_az_max;
	double		window_length;
	double		window_overlap;

	int		n;
	int		nf;
	int		if1;
	int		if2;
	int		dk;
	int		window_width;
	int		window_npts;
	int		taper;
	double		beg_taper;
	double		end_taper;
	double		time0;
	double		t0_min;
	double		dt;
	double		df;
	double		domega;
	double		center_lat;
	double		center_lon;
	double		tstart;
	double		tend;
	char		center_sta[10];
	WorkingCB	working_callback;
	bool		*peak_mask;
	bool		full_compute;
	bool		scan_spectrum;
	double		scan_bandw;

	int		num_fks;
	FKData		**fkdata;

    protected:
	FKWorkSpace	ws;

	void init(gvector<Waveform *> &wvec, int windows,
		double max_slowness, int num_slowness, int num_bands,
		double *f_min, double *f_max, double win_length,
		double win_overlap, WorkingCB working_cb,
		double sig_slow_min, double sig_slow_max,
		double sig_az_min, double sig_az_max, int taper_type,
		double taper_beg, double taper_end, bool scan_frequencies,
		double scan_bandwidth);
	int setup(gvector<GSegmentArray*> *v, int window_overlap_npts);
	void getBands(void);
	bool computeFKData(gvector<GSegmentArray*> *v, int *nfks,
		FKData **fkdata);
	bool computeArray(GSegmentArray *sa, int windowed, int *nfks,
		FKData **fkdata, int *nwork);
	FKData **allocateSpace(gvector<Waveform *> &wvec, int nfks);
	void computeTaper(int window_npts, float *tp);
	void computeSines(FKData *fkd);
	void slownessLoop(FKData *fkd);
	void slownessLoopSearch(GSegmentArray *sa, FKData *fkd);
	void computeScaling(GSegmentArray *sa);

    private:
	void initMembers(void);
};

} // namespace libgfk

#endif
