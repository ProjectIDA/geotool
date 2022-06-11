/*
 * NAME    libaesir
 *	 
 *
 * FILE    libaesir.h
 *
 * SYNOPSIS
 *       
 *       Contains function declarations for DAOS clients.
 *
 * SEE ALSO
 *
 *
 * AUTHOR                                     (pw) 02/07/88
 *
 *
 * MODIFICATIONS
 *
 *                    S. Wass - created file.
 */

/*
 *	SccsId:  @(#)libaesir.h	105.1	19 May 1995	SAIC
 */

#ifndef _LIBAESIR_H_
#define _LIBAESIR_H_	1

#include 	"aesir.h"
#include        <inttypes.h>
#include	<sys/types.h>
#include	<stdarg.h>

#ifndef BETWEEN
#define BETWEEN(a,x,y)  ((a) >= (x) && (a) <= (y))
#endif

#ifndef EQUAL_TOL
#define EQUAL_TOL(x,val,tol)    (((x) >= ((val)-(tol))) && \
                                 ((x) <= ((val)+(tol))))
#endif

extern char *get_version (void);
extern int cmp_version (const char *v1, const char *v2);
extern void lowercase (char *src, char *dst);
extern void uppercase (char *src, char *dst);

	/*
	 *  3 routines for gathering timing information.
	 */
extern int getMilliElapse (int start);
extern int getMilliTime (void);
extern int MillitoSeconds (int milli);

	/*
	 *  A sub-second sleep.
	 */
extern void nap (int seconds, int microseconds);

        /*
	 *  Miscellaneous string manipulation functions
	 */
extern void add_to_str (char *add_string, char **add_to);
extern char *stringcat (char *old, char *add);
extern char *sql_char_list (char *str);
extern char *xstrtok (char *str, char *delims, char *pair_delims);
extern int vstring (char *str, char ***strarray, char *delims);
extern char *skip_space (char *ptr);
extern char *skip_nonspace (char *ptr);

	/*
	 *  Prototypes for various message functions. 
	 */
extern void MessageInit (int (*func) (caddr_t, char *), caddr_t private);
extern void InfoInit (int (*func) (), caddr_t private);
extern void Warning (char *fmt, ...);
extern void Info (char *fmt, ...);
extern void Fatal (char *fmt, ...);
extern void Werror (char *str);

	/*
	 *  Prototypes for various time functions. 
	 */
extern char *strdtime (double thetime);
extern char *strtime (void);
extern char *gmdate (double epoch);
extern char *strgmtime (void);
extern char *etosh (double epoch);

	/*
	 *  Prototypes for various data format conversion functions. 
	 */
extern void g2toint (unsigned char *from, int *to, int num);
extern void inttog2 (int *from, unsigned char *to, int num);
extern void floattog2 (float *from, unsigned char *to, int num);
extern void s3tos4 (unsigned char *s3, int *s4, int n);
extern void s4tos3 (unsigned char *s4, unsigned char *s3, int n);
extern void a2tot4 (uint16_t *a2, float	*t4, int n);
extern void t4toa2 (float *t4, int16_t *a2, int n);
extern void i2toint (int16_t *from, int *to, int num);
extern void t4toint (float *from, int *to, int num);
extern void i4toint (char *from, char *to, int num);
extern void s4toint (char *from, char *to, int num);
extern void g2tofloat (unsigned char *from, float *to, int num);
extern void i2tofloat (int16_t *from, float *to, int num);
extern void i4tofloat (int *from, float *to, int num);
extern void s4tofloat (int *from, float *to, int num);
extern void t4tofloat (float *from, float *to, int num);
int endian_revert (char *buff, int numwds, int len);

	/*
	 * Fortran interfaces to above routines
	 */

extern void g2toint_ (unsigned char *from, int *to, int *num);
extern void g2tofloat_ (unsigned char *from, float *to, int *num);
extern void inttog2_ (int *from, unsigned char *to, int *num);
extern void floattog2_ (float *from, unsigned char *to, int *num);
extern void i2toint_ (int16_t *from, int *to, int *num);
extern void i2tofloat_ (int16_t *from, float *to, int *num);
extern void i4tofloat_ (int *from, float *to, int *num);
extern void s4tofloat_ (int *from, float *to, int *num);
extern void t4tofloat_ (float *from, float *to, int *num);
extern void a2tot4_ (int16_t *from, float *to, int *num);
extern void t4toa2_ (float *from, int16_t *to, int *num);
extern void s3tos4_ (char *from, int *to, int *num);
extern void s4tos3_ (int *from, char *to, int *num);

	/*
	 *  Prototypes for expand_log_keys functions (in expand_log_keys.c)
	 */
extern int expand_log_keys (char *str, char *dest, time_t etime);
extern int is_jdate_key (char *src);
extern int is_host_key (char *src);
extern int is_pid_key (char *src);

/* from fvec_funcs.c */
extern int fvec_scal_init (float *x, int n, double scal);
extern int fvec_scal_mult (float *x, int n, double scal);
extern int fvec_scal_add (float *x, int n, double scal);
extern int fvec_scal_div (float *x, int n, double scal);
extern int fvec_scal_sub (float *x, int n, double scal);
extern int fvec_add (float *x, float *y, float *z, int n);
extern int fvec_mult (float *x, float *y, float *z, int n);
extern int fvec_div (float *x, float *y, float *z, int n);
extern int fvec_abs (float *x, float *z, int n);
extern int fvec_sum (float *x, int n, double *sum);
extern int fvec_add_dly (float *x, float *z, int xn, int zn, double xtime,
                         double ztime, double xdly, double samprate);
extern int fvec_rot2d (float *x, float *y, double angd, int npts);
extern int fvec_rotrt (float *n, float *e, double azd,  int npts);
extern int fvec_rotrtz (float *n, float *e, float *z, double azd, double incd,
                        int npts);
extern int fvec_sq (float *x, int n);
extern int fvec_sqrt (float *x, int n);
extern int fvec_mean_var (float *x, int n, double *mean, double *var);
extern int fvec_mean (float *x, int n, double *mean);
extern int fvec_rms (float *x, int n, double *rms );
extern int fvec_min_max (float *x, int n, double *min, double *max);
extern int fvec_max (float *x, int n, double *max);
extern int fvec_abs_min_max (float *x, int n, double *min, double *max);
extern int fvec_abs_max (float *x, int n, double *max);

#endif /*_LIBAESIR_H_ */
