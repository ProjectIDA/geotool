/*      SCMS Include File SCCS Header
 *  "@(#)SCCSID: libstdtime.h 82.1"
 *  "@(#)SCCSID: Version Created: 08/27/98 12:26:19"
 */

#ifndef _LIBSTDTIME_H_
#define _LIBSTDTIME_H_

#include <sys/time.h>	/* for struct timeval */
#include <time.h>	/* for time_t and struct tm */
#include <limits.h>	/* for LONG_MIN and LONG_MAX */

#ifdef __cplusplus
extern "C"
{
#endif

#define STDTIME_MAX_FORMAT_SIZE	256
#define STDTIME_LDDATE_SIZE	18	/* for stdtime_get_lddate calls */

/*
 * Valid range for epoch times and years
 */
#define	STDTIME_EPOCH_MIN	LONG_MIN
#define	STDTIME_EPOCH_MAX	LONG_MAX
#define	STDTIME_YEAR_MIN	1901
#define	STDTIME_YEAR_MAX	2038

#define STDTIME_GMT_TIME	 1	/* for deprecated function */
#define STDTIME_LOCAL_TIME	 0	/* for deprecated function */

#define TIME_NA			-9999999999.999 /* from db_na.h */

/*
 * Define the return values that flag the calling program that an error
 * may have occurred.  The calling program should then check the global
 * variable stdtime_errno.  If it equals STDTIME_NOERR, then the returned
 * value is correct (rare), otherwise it is the indicated error.
 */
#define STDTIME_SUCCESS		 0
#define	STDTIME_DOUBLE_ERR	-1.0
#define	STDTIME_INT_ERR		-1
#define	STDTIME_CHAR_ERR	(char *) NULL
#define STDTIME_TM_ERR		(struct tm *) NULL	/* deprecated */

#define STDTIME_STRING_ERR	"Err"	                /* deprecated */
#define STDTIME_TIME_NONE	"None"	                /* deprecated */

/*
 * Possible values for stdtime_errno.
 */
#define	STDTIME_NOERR		 0
#define	STDTIME_PARSE		-1     /* Couldn't parse the input */
#define STDTIME_INVALID		-2     /* Input invalid            */
#define STDTIME_AMBIGUOUS	-3     /* Input ambiguous          */
#define	STDTIME_MALLOC		-4     /* Routine couldn't malloc  */

typedef double epoch_t;         /* seconds since 1970-01-01 GMT    */
typedef int   jdate_t;         /* jdate or "ordinal date" yyyydoy */
typedef int   julian_t;        /* number of days since year 0 AD  */

typedef struct {
     int     year;              /* 4-digit year      */
     int     mon;               /* month number 1-12 */
     int     day;               /* day of month 1-31 */
     int     hour;              /* 0-23   */
     int     min;               /* 0-59   */
     double  sec;               /* 0-61.0 */
} calendar_t;


typedef struct {		/* for deprecated function */
	struct tm tm_s;		/* C time structure */
	int	  milli;	/* milliseconds */
} std_time_st;


/*
 * Multi-Threaded stdtime_errno define
 */

#ifdef _REENTRANT
extern int *___stdtime_errno();
#define stdtime_errno (*(___stdtime_errno()))
#else
extern int stdtime_errno;
#endif  /* _REENTRANT */


/*
 * These are the main functions in the standard time library.
 */
char *stdtime_get_lddate(char *lddate_a);
epoch_t stdtime_get_epoch(void);
epoch_t stdtime_rnd_epoch(epoch_t epoch_time, int ndec);
epoch_t stdtime_diff_epoch(epoch_t epoch1, epoch_t epoch2, int ndec,
                           char *diff_a, int diff_len);
epoch_t stdtime_expr_time(char *expr_a);
calendar_t *stdtime_expr_time_c(char *expr_a, calendar_t *calendar);
int stdtime_format_r(epoch_t epoch_time, char *format_a, char *human_a,
                     int human_len);
epoch_t stdtime_unformat(char *human_a, char *format_a);

/*
 * These are the preferred "wrapper" functions
 */
epoch_t stdtime_htoe(char *human_a);
char *stdtime_etos(epoch_t epoch_time, int ndec, char *std_a, int std_len);
char *stdtime_etosd(epoch_t epoch_time, char *std_a, int std_len);
char *stdtime_etost(epoch_t epoch_time, int ndec, char *std_a, int std_len);
char *stdtime_etol(epoch_t epoch_time, char *lddate_a, int lddate_len);
char *stdtime_etogse(epoch_t epoch_time, int ndec, char *gse_a, int gse_len);
char *stdtime_etogsed(epoch_t epoch_time, char *gse_a, int gse_len);
char *stdtime_etogset(epoch_t epoch_time, int ndec, char *gse_a, int gse_len);
char *stdtime_etocd(epoch_t epoch_time, int ndec, char *cd_a, int cd_len);
char *stdtime_etodir(epoch_t epoch_time, char *fmt_a, char *dir_a,
                     int dir_len);
jdate_t stdtime_etoj(epoch_t epoch_time);
epoch_t stdtime_jtoe(jdate_t yyyydoy);
calendar_t *stdtime_etoc(epoch_t epoch, calendar_t *calendar);
epoch_t stdtime_ctoe(calendar_t *calendar);
julian_t stdtime_etojulian(epoch_t epoch);
epoch_t stdtime_juliantoe(julian_t julian);
julian_t stdtime_htojulian(char *human_a);
calendar_t *stdtime_juliantoc(julian_t julian, calendar_t *calendar);
julian_t stdtime_ctojulian(calendar_t *calendar);


/*
 * These are the deprecated functions.  They are subject to removal.
 */
char* stdtime_format(double epoch_time, char *format_a);
double stdtime_now(void);
double stdtime_fnow(void);
time_t stdtime_time(void);
int stdtime_gettimeofday(struct timeval *tp);
double stdtime_convert_struct(std_time_st time_s);
std_time_st stdtime_build_struct(double epoch_time);
char *stdtime_format_gol(double epoch_time, char *format_a, int gmt_or_local);

/*
 * Prototype definitions for the "wrapper" functions.
 */
double stdtime_gmttoe(char *gmt_a);
char *stdtime_etogmt(double epoch);
int stdtime_etogmt_r(double epoch, char *human_ap, int n);
double stdtime_shtoe(char *sh_a);
char *stdtime_etosh(double epoch);
int stdtime_etosh_r(double epoch, char *human_ap, int n);
double stdtime_sshtoe(char *sh_a);
char *stdtime_etossh(double epoch);
int stdtime_etossh_r(double epoch, char *human_ap, int n);
char *stdtime_etou(double epoch);
int stdtime_etou_r(double epoch, char *human_ap, int n);
double stdtime_gsetoe(char *gse_a);
int stdtime_etogse_r(double epoch, char *gse_ap, int n);
int stdtime_format_gol_r(double epoch_time, char *format_a, char *human_a,
                         int human_len, int gmt_or_local);
/*
 * Prototype definitions for functions in stdtime_get_lddate
 */
char *stdtime_gol_lddate(int gmt_or_local);
int stdtime_gol_lddate_r(int gmt_or_local, char *lddate);
int stdtime_etoyyyymmdd(double epoch);
int stdtime_etol_r(double epoch, char *l_ap, int n);

/*
 * Prototype definitions for functions in stdtime_asctime
 */
char *stdtime_asctime(struct tm *tm);
char *stdtime_asctime_r(struct tm *tm, char *buf, int buflen);

/*
 * Prototype definintions for functions in stdtime_ctime
 */
char *stdtime_ctime(time_t *time);
char *stdtime_ctime_r(time_t *time, char *buf, int buflen);
char *stdtime_strdtime(double thetime);
char *stdtime_strtime(void);

/*
 * Prototype definitions for functions in stdtime_get_lddate
 */
char *stdtime_log_lddate(int gmt_or_local);

/*
 * Functions in stdtime_localtime.c
 */
struct tm *stdtime_localtime(time_t *time);
struct tm *stdtime_localtime_r(time_t *time, struct tm *tm);

/*
 * Functions in stdtime_yyyydoy_format.c:
 */
int stdtime_htoj(char *human);

/*
 * Functions in human_format.c:
 */
char *stdtime_etoh(double epoch);
int stdtime_etoh_r(double epoch, char *human_ap, int n);
char *stdtime_jtoh(int yyyydoy);
int stdtime_jtoh_r(int yyyydoy, char *human_ap, int n);

/*
 * Functions in stdtime_string.c
 */
double stdtime_dstrtoe(char *human_a);
char *stdtime_etodstr(double epoch);
char *stdtime_etotstr(double epoch);

/*
 * Functions in stdtime_milliTime.c
 */
int stdtime_getMilliTime(void);
int stdtime_getMilliElapse(int start);
int stdtime_MillitoSeconds(int milli);

#ifdef __cplusplus
}
#endif

#endif /* _LIBSTDTIME_H_ */
