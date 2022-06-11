/*
 * libampP.h	
 */

#ifndef LIBAMPP_H
#define LIBAMPP_H

/* for 'free' */
#include <stdlib.h>

/* Define peak,trough indices */
#define PEAK    2		/* Search mode looking for peak */
#define TROUGH  1		/* Search mode looking for trough */
#define NEITHER 0		/* Search mode looking for either one */

/* Define possible measurement type values */
#define AMP_PEAK_TROUGH "peak_trough"
#define AMP_ZERO_PEAK	"zero_peak"
#define AMP_FM		"fm"
#define AMP_FM_SIGN	"fm_sign"
#define AMP_FM_A	"fm_a"
#define AMP_FM_B	"fm_b"
#define AMP_RMS		"rms"
#define AMP_MEAN_SQR	"mean_sqr"
#define AMP_ABS_MAX	"abs_max"
#define AMP_AVG_MAX	"avg_max" /* For Sonseca */
#define AMP_STAV	"stav"

/* Macros to determine filter type - from libfilter.h */
#define BANDPASS_FILTER(ftype)   (strncmp (ftype,"BP",2)==0)
#define BANDREJECT_FILTER(ftype) (strncmp (ftype,"BR",2)==0)
#define HIGHPASS_FILTER(ftype)   (strncmp (ftype,"HP",2)==0)
#define LOWPASS_FILTER(ftype)    (strncmp (ftype,"LP",2)==0)
#define NO_FILTER(ftype)         (strncmp (ftype,"NO",2)==0)

/* Maximum filter order, coefficents allowed */
#define MAX_FILTER_ORDER  10
#define MAX_FILTER_COEFS  MAX_FILTER_ORDER*3	

/* at present (1Q2005) the logErrorMsg doesn't distinguish 
 * syslog levels.  We still want to use the symbols.  
 */

#define DFX_VERBOSE0  LOG_WARNING
#define DFX_VERBOSE1  LOG_WARNING
#define DFX_VERBOSE2  LOG_WARNING
#define DFX_VERBOSE3  LOG_WARNING
#define DFX_VERBOSE4  LOG_WARNING

/* used widely */
#define FREE(a) if(a) {free(a); a = NULL; }

/* from peak_trough.c */
int peak_trough( float *beam, int npts, int *pts,
			double btime, double samprate, 
			char *type, double *amp, double *per,
						double *time);

/* from first_motion.c */
int first_motion( float *beam, int npts, int *pts,
			 double btime, double samprate, 
			 char *type, double *amp, double *per,
			 double *time);

/* from find_peak_trough.c */
int find_peak_trough( float *beam, int npts, int *pts, double *maxdiff );


/* from smooth_peak_trough.c */
int smooth_peak_trough( float *beam, int npts, int *pts, 
			   double maxdiff, double thresh);


/* from max_peak_trough.c */
int max_peak_trough( float *beam, int npts, int *pts, 
			double *max_amp, int *max_0,int *max_1 );


/* from amp.c */
int measure_amplitude( float *beam, int npts, double beam_time, 
			  double samprate, char *type, double value, 
			  double thresh, double flo, double fhi,
			  int ford, int zp, char *ftype, int interp_per,
			  double *amp, double *per, double *time );

/* from remove_filt_resp.c */
int remove_filt_resp( char *filt_type, int filt_order, int zero_phase, 
			 double freq, double hicut, double lowcut,
			 double samprate, double filt_rolloff, double *amp );

/* from avg_max.c */
int avg_max( float *beam, int npts, int nmax, double *amp, int *imax );


/* from max_stav.c */
int max_stav( float *data, int npts, int nstav, double *maxstav, int *imax );

/* from interp_period.c */
int interp_period( float *beam, int npts, int *pt_vals, int max_0, int max_1,
		   double samprate, double flo, double fhi, int ford, int zp,
		   double *period);

/* from butterworth_response.c */
int butterworth_response( char *filt_type, int filt_order,
			  int zero_phase, double freq,
			  double hicut, double lowcut,
			  double samprate, double *resp );

#endif /* LIBAMPP_H */

