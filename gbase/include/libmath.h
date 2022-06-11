/*	SccsId:	%W%	%G%	*/

#ifndef _LIBMATH_H_
#define	_LIBMATH_H_

#include <stdio.h>

#include "crust.h"


/* ****** Find.c ********/
int Find(double *x, int n, double findx);


/* ****** LogData.c ********/
void LogData(int n, float *y);


/* ****** validData.c ********/
int invalidData(int nsamp, float *data, double value);


/* ****** correlate.c ********/
float correlate(float *x, double fx, float *y, double fy, int n);


/* ****** covar.c ********/
void covar(int npts, float *e, float *n, float *z, double *s);


/* ****** crust.c ********/
int readCrustModels(FILE *fp, CrustModel **cmp, int *n_models);
int crust_model(int num_crusts, CrustModel *cm, const char *crust_name,
			CrustModel *crust);


/* ****** deltaz.c ********/
void deltaz(double slat, double slon, double rlat, double rlon, double *delta,
			double *az, double *baz);


/* ****** euler.c ********/
void euler(double *theta, double *phi, double a, double b, double g);
void euler2(double *theta, double *phi, double a1, double b1, double g1,
			double a2, double b2, double g2);
void rotation_matrix(double alpha, double beta, double gamma, double c[3][3]);


/* ****** fft.c ********/
int factor(int n);
int fftl(float *x, int n, int ndir);


/* ****** ftoa.c ********/
void ftoa(double f, int ndeci, int fixLength, char *s, int len);


/* ****** geocentric.c ********/
double geocentric(double geographic_lat);
double geographic(double geocentric_lat);


/* ****** get_regional.c ********/
void get_regional(CrustModel *crust, const char *phase, double depth, int *npts,
			float *x, float *y);

/* ****** hilbert.c ********/
int Hilbert_data(int npts, float *data);


/* ****** jbsim.c ********/
int jbopen(const char *table);
int jbsim(int phase_code, double delta, double depth, float *ttime,
			Derivatives *dd);


/* ****** nicex.c ********/
void nicex(double ax1, double ax2, int minlb, int maxlb, int *nlab, double *x,
			int *ndigit, int *ndeci);


/* ****** nint.c ********/
int nint(double f);


/* ****** polint.c ********/
int polint(float *xa, float *ya, int n, double x, float *y, float *dy);
int polint2(float *x1a, float *x2a, float *ya, int m, int n, double x1,
			double x2, float *y, float *dy);


/* ****** regional.c ********/
int regional(CrustModel *crust, const char *phase, double delta, double depth,
			float *ttime, Derivatives *dd);


/* ****** sortup.c ********/
void sortup(double *values, int n, int *sort_order);
void sortupi(int *values, int n, int *sort_order);


/* ****** spline.c ********/
void cspline(float *x, float *y, int n, float *y2, float *u);
void csplint(float *xa, float *ya, float *y2a, int n, double x, float *y);


/* ****** toeplitz.c ********/
int toeplitz(int m, float *r, float *g, float *f);
int toepinv(int m, float *r, float *rinv);


/* ****** tql2.c ********/
int tql2(int n, double *d, double *e, double *z);


/* ****** tred2.c ********/
void tred2(int n, double *a, double *d, double *e, double *z);


/* ****** ttup.c ********/
int ttup(CrustModel *crust, const char *phase, double delta, double depth,
			double *tt, Derivatives *dd);


/* ****** xcorr_lag.c ********/
int xcorr_lag(float *x, int nx, double fx, float *y, int ny, double fy,
			double *rmax);


#endif /* _LIBMATH_H_ */
