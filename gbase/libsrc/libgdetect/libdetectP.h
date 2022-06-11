/*
 *
 *	Private header file for DFX libdetect
 *
 */


#ifndef LIBDETECT_H
#define LIBDETECT_H

/* use syslog symbol values */
#include <syslog.h>

#define DEFAULT_SNR 7.9

#define DFX_VERBOSE0   LOG_WARNING

/* to avoid pulling in dfx.h: */
#define DFX_ZERO_VALUE          1.0e-6
#define DFX_NULL_LOG_VALUE      -6.0

/* used widely in libdetect */
#define FREE(a) if(a) { free(a); a = NULL; }


/* from avg.c */
extern int running_avg( float *data, int *state, int npts, 
						int len, int thresh, double (*fp)(double),
						float *avg, int *avg_state );

extern int recursive_avg( float *data, int *state, int npts, 
						  int prev_len, int avg_len, int thresh, 
						  double (*fp)(double),float *avg,int *avg_state );

extern int running_sum( int *data, int npts, int len, int *fdata );

/* from z_stat_snr.c */
extern int z_stat_snr( float *stav, int *stav_state, float *ltav, 
					   int *ltav_state, int npts, int stav_len, 
					   int ltav_len, int ltav_thresh,
					   int log_flag, float **snr );

/* from standard_snr.c */
extern int standard_snr( float *stav, int *stav_state, float *ltav, 
						 int *ltav_state, int npts, int stav_len, 
						 int ltav_len, int sqr_flag, float **snr );

/* from snr.c */
extern int compute_snr( float *data, float *norm, int npts, double comp_thr,
						int stav_len, int ltav_len, char *stav_meas, 
						char *stav_method, int stav_thresh, int ltav_thresh,
						char *snr_method, float **stav, int **stav_state,
						float **ltav, int **ltav_state, float **snr, int **state );

extern int compute_snr_aux( float *stav, int *sstp, float *ltav, int *lstp,
							int npts, int stav_len, int ltav_len, char *stav_meas, 
							char *stav_method, int stav_thresh, int ltav_thresh,
							char *snr_method, float **snr );

/* from ltav.c */
extern int compute_ltav( float *stav, int *stav_state, int npts, int stav_len, 
						 int ltav_len, int ltav_thresh, float **ltav, int **ltav_state );

/* from stav.c */
extern int compute_stav( float *data, int *state, int npts, char *method, char *meas,  
						 int len, int thresh, float **stav, int **stav_state );

/* from state.c */
extern int compute_state( float *norm, int npts, double thr, int **state );

/* from calc_deltime.c */
extern double calc_deltime( double snr, double detsnr,double max_snr, 
							double min_snr, double max_deltim, double min_deltim );

/* from detect_funcs.c */
double squarex( double x);
double samex( double x );

#endif /* LIBDETECT_H */

