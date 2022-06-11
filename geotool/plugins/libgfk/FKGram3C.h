#ifndef _FK_GRAM_3C_H
#define _FK_GRAM_3C_H

#include "gobject++/Gobject.h"
#include "gobject++/gvector.h"
#include "FKData.h"
#include "FKGram.h"

class Waveform;
class GSegmentArray;

namespace libgfk {

/** This class computes a time series of FKData objects for three-component
 *  data. FKData objects are computed for a sliding data window along three
 *  components.
 *  @ingroup libgFK
 */
class FKGram3C : public Gobject
{
    public:

	FKGram3C(gvector<Waveform *> &wvec, int windowed,
		double slowness_max, int n_slowness,
		int nbands, double *fmin, double *fmax, double window_length,
		double window_overlap, WorkingCB working_cb, int taper,
		double beg_taper, double end_taper);
	~FKGram3C(void);

	int recompute(gvector<Waveform *> &wvec);

	int		waveform_ids[3];
	int		windowed;
	int		n_slowness;
	double		d_slowness;
	double		slowness_max;
	int		nbands;
	double		fmin[MAX_NBANDS];
	double		fmax[MAX_NBANDS];
	double		window_length;
	double		window_overlap;

	double		p_site[3][3];
	float		*fptr[3];
	double		hang[3];
	double		vang[3];
	int		f_size;
	float		*f;
	int		dk;
	int		window_width;
	int		window_npts;
	double		total_power;
	double		tlen;
	double		time0;
	double		t0_min;
	double		dt;
	double		center_lat;
	double		center_lon;
	char		center_sta[10];
	WorkingCB	working_callback;
	gvector<Waveform *> wvec;

	int		num_fks;
	FKData		**fkdata;

    protected:

	void init(gvector<Waveform *> &wvec, int windowed,
		double slowness_max, int n_slowness,
		int nbands, double *fmin, double *fmax, double window_length,
		double window_overlap, WorkingCB working_cb, int taper,
		double beg_taper, double end_taper);
	int setup(gvector<GSegmentArray*> *v, int window_overlap_npts);
	bool computeFKData(gvector<GSegmentArray*> *v, int *nfks,
		FKData **fkdata);
	bool computeArray(GSegmentArray *sa, int windowed, int *nfks,
		FKData **fkdata, int *nwork);
	FKData ** allocateSpace(gvector<Waveform *> &wvec, int nfks);
	void slownessLoop(float *fk_b);
};

} // namespace libgfk

#endif
