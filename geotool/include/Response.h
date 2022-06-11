#ifndef _RESPONSE_H
#define _RESPONSE_H

#include "gobject++/Gobject.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libtime.h"
}

/** @defgroup libgresp library libgresp++
 *  C++ classes for handling instrument responses.
 */

#ifndef _FCOMPLEX_
#define _FCOMPLEX_
typedef struct fcomplex {
	float re;
	float im;
} FComplex;
#endif

typedef struct
{
    char	sta[6];
    char	chan[4];
    char	auxid[5];
    char	instype[7];
    double	calib;
    double	calper;
    double	samprat;
    DateTime	ondate;
    DateTime	offdate;
} GSE_CAL;

/** A class for instrument responses. This class holds an instrument response
 *  in one of three formats, FAP(frequency, amplitude, and phase), PAZ (poles
 *  and zeroes), and FIR (finite impulse response). There are functions for
 *  computing the complex response values at specific frequencies. There are
 *  functions that convolve and deconvolve the response with the data in a
 *  GTimeSeries object. And there are convenience functions for converting
 *  waveform amplitude values from counts to nanometers.
 *  @ingroup libgresp
 */
class Response : public Gobject
{
    public:
	Response(void);
        Response(const Response &r);
        Response & operator=(const Response &r);
	~Response(void);

	bool firConvolve(int direction, FComplex *f, int np2, double dt, bool remove_time_shift);
	bool fapConvolve(int direction, FComplex *y, int np2, double dt);
	bool pazConvolve(int direction, FComplex *f, int np2, double dt);
	void computePAZ(double omega, FComplex *poles, int num_poles, FComplex *zeros,
			int num_zeros, double norm, double *real, double *imag);
	void computeFAP(double lofreq, double hifreq, int nf, double *real, double *imag);
	bool computeFIR(double lofreq, double hifreq, int nf, double *real, double *imag);

	void computepaz(double omega, FComplex *poles, int num_poles,
			FComplex *zeros, int num_zeros, double norm,
			double *real, double *imag);
	void computefap(double lofreq, double hifreq, int nf, double *real,
			double *imag);
	bool computefir(double lofreq, double hifreq, int nf, double *real,
			double *imag);

	static bool convolve(vector<Response *> *resp, int direction, float *data,
		int npts, double dt, double calib, double calper,
		double flo, double fhi, double amp_cutoff,
		bool remove_fir_time_shift);
	static bool compute(vector<Response *> *resp, double lofreq,
		double hifreq, double calib, double calper,
		int nf, double *real, double *imag);
	static bool compute(vector<Response *> *resp, double freq, double calib,
		double calper, double *real, double *imag) {
	    return compute(resp, freq, freq, calib, calper, 1, real, imag);
	}
	static void ampCutoff(double *real, double *imag, int nf,
		double amp_cutoff);
	static void taperAmp(double *real, double *imag, double df, int nf,
		double flo, double fhi);
	static void unwrap(double *p, int n);
	static void removeTimeShift(int nf, double *re, double *im);


//    protected:

	string		source;		//!< "theoretical" or "measured"
	int		stage;		//!< stage number
	string		des;		//!< description
	string		type;		//!< fap, paz, or fir
	/** d for displacement, v for velocity, or a for acceleration */
	string		input_units;	//!< from b53,b54,b55,b61 reference to b34
	string		output_units;	//!< from b53,b54,b55,b61 reference to b34
	string		response_units;	//!< from b52 reference to b34
	string		insname;	//!< from b52 Instrument identifier
	string		author;		//!< author or information source
	double		a0;		//!< normalization for paz / gain for fir
	double		b58_sensitivity;//!< calibration
	double		b58_frequency;	//!< calibration frequency
	int		npoles;		//!< number of poles for paz type
	int		nzeros;		//!< number of zeros for paz type
	FComplex	*pole;		//!< the poles for paz type
	FComplex	*pole_err;	//!< the pole errors for paz type
	FComplex	*zero;		//!< the zeros for paz type
	FComplex	*zero_err;	//!< the zero errors for paz type
	int		nfap;		//!< the number of fap triplets
	float		*fap_f;		//!< the frequencies for fap type
	float		*fap_a;		//!< the amplitudes for fap type
	float		*fap_p;		//!< the phases for fap type
	float		*amp_error;	//!< the amplitude errors for fap type
	float		*phase_error;	//!< the phase errors for fap type
	double		input_samprate;	//!< samples/sec ( used in fir type)
	double		output_samprate;//!< samples/sec
	double		delay;		//!< blockette57 estimated time delay
	double		correction;	//!< blockette57 time correction
	/** the number of numerator coefficients for fir */
	int		num_n;
	/** the number of denominator coefficients for fir */
	int		num_d;
	float		*fir_n;		//!< the numerators for fir type
	float		*fir_n_error;	//!< the numerator errors for fir type
	float		*fir_d;		//!< the denominators for fir type
	float		*fir_d_error;	//!< the denominator errors for fir

	GSE_CAL		*cal;	//!< associated CAL record for GSE format

    private:
	void copy(const Response &r);
};

#endif
