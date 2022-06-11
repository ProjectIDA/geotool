/*      SCMS Include File SCCS Header
 @(#)SCCSID: libstdtimeP.h 82.1
 @(#)SCCSID: Version Created: 08/27/98 12:26:26
 */


#ifndef LIBSTDTIMEP_H
#define LIBSTDTIMEP_H

#include <stdio.h>
#include <stdlib.h>			/* putenv proto. */
#include <ctype.h>			/* isspace proto. */
#include "libstdtime.h"

#define YY_NO_UNPUT 1

/* true if leap year else false */
#define stdtime_isleap(yr) (((yr)%4 == 0 && (yr)%100 != 0) || (yr)%400 == 0)

#define EPOCH_YEAR_ZERO		1970

#define SECONDS_PER_DAY         86400

static int stdtime_dim[] = {31,28,31,30,31,30,31,31,30,31,30,31,31};

/* Check routines. */
int check_C(char *str);
int check_epoch(double epoch);
int check_epoch_ptr(time_t *epoch);
int check_buffer(char *buffer);
int check_fmt(char *fmt);
int check_gmt(char *gmt);
int check_gse(char *gse);
int check_human(char *human);
int check_lddate(char *lddate);
int check_sh(char *sh);
int check_std_time_st(std_time_st *time_st);
int check_tm(struct tm *tm_p);
int check_yyyydoy(int yyyydoy);
int check_year(int year);
int check_year_mon_day(int year, int mon, int day);
int check_hour_min_sec(int hour, int min, double sec);


/* Fortran interface routines */
void stdtimeetoh_(double *epoch, char *human, int human_len);
void stdtimeetoj_(double *epoch, int *yyyydoy);
void stdtimeetos_(double *epoch, int *ndec, char *human, int human_len);
void stdtimeetosh_(double *epoch, char *human, int human_len);
void stdtimeetou_(double *epoch, char *human, int human_len);
void stdtimehtoe_(char *human, double *epoch, int human_len);
void stdtimehtoj_(char *human, int *yyyydoy, int human_len);
void stdtimejtoh_(int *yyyydoy, char *human, int human_len);
void stdtimeshtoe_(char *human, double *epoch, int human_len);

int __replace_expandable_formats(char *in_fmt, char *out_fmt, int n);
int __find_special_format(char *in_fmt, char *fmt_c, int *fmt_op);

time_t stdtime_timegm(struct tm *tm);

#ifdef __GNUC__
/*
 * This is included because the prototype is not included in the
 * standard header files.
 char*, strptime, (const	char	*buf,
			  const	char	*fmt,
			  struct tm 	*tm);
 */

#endif /* __GNUC__ */

#ifdef __svr4__
	/* Put Solaris-only code here. */
		/*  strptime is in <time.h>, but guarded by ifdef's ?? */
char *strptime(const char *, const char *, struct tm *);

#else
	/* SunOS only here. */

/*
 * Some system routines don't have prototypes on SunOS.
 */
time_t time(time_t *t);
size_t strftime(const char *s, size_t maxsize, const char *format,
                const struct tm *timeptr);
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);

#ifdef VARARGS
int fprintf(FILE *strm, const char *format, ... );
int printf(FILE *strm, const char *format, ... );
int sscanf(const char *, const char *, ...);
int sprintf(char *s, const char * format, ...);
#endif

#endif

#endif /* LIBSTDTIMEP_H */
