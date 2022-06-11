/*	SccsId:	%W%	%G%	*/

#ifndef _LIBTIME_H_
#define	_LIBTIME_H_

#include "logErrorMsg.h"

#define isLeapYear(yr) ((yr%4)==0 && ((yr%100)!=0 || (yr%400)==0))

#ifndef NULL_TIME
#define NULL_TIME -9999999999.999
#endif
#ifndef NULL_TIME_CHECK
#define NULL_TIME_CHECK -9999999999.
#endif


enum TimeFormat {
    /* not a time value */
    NOT_TIME = -1,

    /* %04d%s%02d %02d:%02d:%06.3f, year, mname, day, hour, minute, second */
    YMONDHMS = 0,

    /* %04d%s%02d, year, mname, day */
    YMOND = 1,

    /* %02d:%02d:%06.3f, hour, minute, second */
    HMS = 2,

    /* %02d:%02d:%04.1f, hour, minute, second */
    HMS2 = 3,

    /* %02d:%02d:%04.1f, hour, minute, second */
    HMS3 = 4,

    /* %04d%s%02d %02d:%02d:%04.1f, year, mname, day, hour, minute, second */
    YMONDHMS2 = 5,

    /* %04d/%02d/%02d %02d:%02d:%04.1f, year, month, day, hour, minute, second*/
    GSE20 = 6,

    /* %04d/%02d/%02d %02d:%02d:%06.3f, year, month, day, hour, minute, second*/
    GSE21 = 7,

    /* %02d:%02d:%06.3f, epoch_hours, minute, second */
    MONTH_HOURS = 8,

    /* %04d-%02d-%02d, year, month, day */
    YMD = 9,

    /* %03d %02d:%02d:%02.0f, day-1, hour, minute, second */
    DHMS = 10,

    /* %03d:%02d:%02.0f, epoch_hours, minute, second */
    MONTH_HOURS2 = 11,

    /* %04d %02d:%02d:%02.0f, epoch_days, hour, minute, second */
    EPOCH_DAYS = 12,

    /* %03d %02d:%02d:%04.1f, epoch_days, hour, minute, second */
    EPOCH_DAYS2 = 13,

    /* %04d%s%02d %02d:%02d:%06.0f, year, mname, day, hour, minute, second */
    YMONDHMS3 = 14,

    /* %03d:%02d:%04.1f, epoch_hours, minute, second */
    MONTH_HOURS3 = 15,

    /* %02d:%02d:%02.0f, epoch_hours, minute, second */
    HHMMSS = 16,

    /* %04d/%02d/%02d %02d:%02d:%04.2f, year, month, day, hour, minute, second*/
    GSE22 = 17 

};

	
typedef struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	double second;
} DateTime;

/* ****** timeUtil.c ********/
char *	timeEpochToGMT(double epochal, int num_decimals);
double	timeDateToEpoch(DateTime *dt);
int	timeDOY(DateTime *dt);
int	timeDayOfYear(int year, int month, int day);
void	timeEpochToDate(double epoch, DateTime *dt);
const char *timeMonthName(int month);
double	timeGetEpoch(void);
char *	timeLoadDate(void);
int	timeEpochDays(DateTime *dt);
void	timeMonthDay(int year, int day_of_year, int *month, int *day);
int	timeJDate(DateTime *dt);
int	timeEpochToJDate(double epoch);
char *	timeDateString(DateTime *dt);
char *	timeDateStringR(DateTime *dt);

/* ****** timeParse.c ********/
int	timeCheckTimes(double time1, double time2, char *msg, int len);
int	timeParseString(const char *s, double *d);
char *	timeEpochToString(double epoch, char *str, int len,
			enum TimeFormat format);
double	timeStringToEpoch(const char *str, enum TimeFormat format);
void	timeParseLine(const char *line, int s_pos, int e_pos, void *val,
			int size, const char *type);
int	timeParseJDate(const char *string, long *jdate);


#endif /* _LIBTIME_H_ */
