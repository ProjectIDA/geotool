#ifndef _LIB_FT_H_
#define _LIB_FT_H_

typedef struct CepstrumParam_s
{
	int	npass;
	int	noise_flag;
	int	smoothing_npass;
	int	return_data1;
	int	return_noise1;
	int	return_data2;
	int	return_noise2;
	int	return_data3;
	int	return_data4;
	int	return_data5;
	int	return_data6;
	double	smoothing_width;
        double	flo, fhi;
	double	guard1, aveband1;
	double	guard2, aveband2;
	double	shift, tpass;
	double	pulse_delay_min, pulse_delay_max;
} CepstrumParam;

#define CEPSTRUM_PARAM_NULL \
{ \
	1,	/* npass */ \
	0,	/* noise_flag */ \
	2,	/* smoothing_npass */ \
	1,	/* return_data1 */ \
	1,	/* return_noise1 */ \
	1,	/* return_data2 */ \
	1,	/* return_noise2 */ \
	1,	/* return_data3 */ \
	1,	/* return_data4 */ \
	1,	/* return_data5 */ \
	1,	/* return_data6 */ \
	0.05,	/* smoothing_width */ \
	0.2,	/* flo */ \
	20.0,	/* fhi */ \
	.1,	/* guard1 */ \
	.1,	/* aveband1 */ \
	.1,	/* guard2 */ \
	.1,	/* aveband2 */ \
	0.0,	/* shift */ \
	1.0,	/* tpass */ \
	.1,	/* pulse_delay_min */ \
	10.,	/* pulse_delay_max */ \
}

typedef struct CepstrumOut_s
{
	int	nf;
	float	*data1;
	float	*noise1;
	float	*data2;
	float	*noise2;
	float	*data3;
	float	*data4;
	float	*data5;
	float	*data6;
	double	dt;
	double	df;
	double	delay_time;
	double	variance;
	double	peak_std;
} CepstrumOut;

#define CEPSTRUM_OUT_NULL \
{ \
	0, 	/* nf */ \
	NULL, 	/* data1 */ \
	NULL, 	/* noise1 */ \
	NULL, 	/* data2 */ \
	NULL, 	/* noise2 */ \
	NULL, 	/* data3 */ \
	NULL, 	/* data4 */ \
	NULL, 	/* data5 */ \
	NULL, 	/* data6 */ \
	0.,	/* dt */ \
	0.,	/* df */ \
	0.,	/* delay_time */ \
	0.,	/* variance */ \
	0.,	/* peak_std */ \
}


/* ****** cepstrum.c ********/
int Cepstrum(float *signal, int sig_npts, float *noise, int noise_npts,
		double dt, CepstrumParam *cp, CepstrumOut *co);
int Cepstrum_step1(float *signal, int sig_npts, float *noise, int noise_npts,
		double dt, CepstrumParam *cp, CepstrumOut *co);
int Cepstrum_step2(CepstrumParam *cp, CepstrumOut *co);
int Cepstrum_step3(CepstrumParam *cp, CepstrumOut *co);
int Cepstrum_step4(CepstrumParam *cp, CepstrumOut *co);
int Cepstrum_step5(CepstrumParam *cp, CepstrumOut *co);


/* ****** cefft.c ********/
#ifndef _FCOMPLEX_
#define _FCOMPLEX_
typedef struct fcomplex {
        float re;
        float im;
} FComplex;
#endif
void fftr(register FComplex *x, int n, int sign);
void refft(FComplex *x, int n, int sign, int mode);
void cefft(FComplex *x, int n, int sign);

double Taper_hann(float *x, int n);
double Taper_hamm(float *x, int n);
double Taper_parzen(float *x, int n);
double Taper_welch(float *x, int n);
double Taper_blackman(float *x, int n);
double Taper_cosine(float *x, int n, double beg_len, double end_len);

#ifndef HANN_TAPER
#define HANN_TAPER      0
#endif
#define HAMM_TAPER      1
#define COSINE_TAPER    2
#define PARZEN_TAPER    3
#define WELCH_TAPER     4
#define BLACKMAN_TAPER  5
#define NO_TAPER        6

#endif
