#ifndef _LIB_TAPERS_H_
#define _LIB_TAPERS_H_

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
