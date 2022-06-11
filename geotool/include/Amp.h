#ifndef _AMP_H
#define _AMP_H

#include <sstream>

#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

/** @defgroup libgx library libgx++
 *  C++ classes for core interface elements.
 */

/** A class with functions for automated amplitude and period measurement.
 *  @ingroup libgx
 */
class Amp
{
    public :
	static bool autoMeasureAmpPer(GTimeSeries *ts,
		const string &amptype, double time, bool mb_allow_counts,
		const string &mb_counts_amptype, CssAmplitudeClass *amp);
	static int measure_SP_pp_amplitude(float *beam, double samprate,
		int nsamples, double waveform_start_time,
		double time_of_the_pick, double *max_amplitude, double *period,
		double *time_of_max_amplitude, double *bandw);
	static int automb_new2(float *beam, int *istart_inp,
		int *nsamples_in_window_inp, int *nsamples_inp,
		int filter_order, int zero_phase, double hicut,
		double lowcut, double samprate, double *amb_SigAmp,
		double *amb_f_sighalfper, double *amb_f_sigtime,
		double *threshold1_inp, double max_allowed_filter_correction);
	static int lagrange3(float y_1, float y0, float y1, float *xmax,
		float *ymax);
	static int cos_parameters(float y_1, float y0, float y1,
		float *shift_samples, float *amp, float *half_period_samples);
	static int apply_filt_resp_new2(int filter_order, int zero_phase,
		double freq, double hicut, double lowcut, double samprate,
		double *amp, double max_allowed_filter_correction);
	static char *getAmpError(void);
};

#endif
