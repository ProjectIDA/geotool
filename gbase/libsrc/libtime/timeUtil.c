#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "libtime.h"

static const int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31,31};
static const char *month_name[] =
{"---","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

/**
 * Convert epochal time to a string as GMT.
 * @param epochal The epochal time.
 * @param num_decimals The number of decimal places to print.
 * @return Returns a pointer to a static string with the GMT time.
 */
char *
timeEpochToGMT(double epochal, int num_decimals)
{
	static char gmt[100];
	char *date;
	time_t clock;
	char buf[50];
	struct tm result;

	if(num_decimals > 0)
	{
	    int decimal;
	    double p = pow(10., (double)num_decimals);
	    double x = floor(epochal*p+.5);
	    clock = (time_t)(x/p);

	    /* if time is neg, need to remove a second */
	    if(epochal < 0.) clock = clock - 1;

	    date = (char *)asctime_r(gmtime_r(&clock, &result), buf);
	    strncpy(gmt, date+11, 8);
	    gmt[8] = '\0';
	    decimal = (int)(x - p*floor(epochal));
	    if(decimal != 0) {
		char s[50];
		int i;
		strcat(gmt, ".");
		snprintf(s, 50, "%d", decimal);
		s[49] = '\0';
		for(i = (int)strlen(s); i < num_decimals; i++) {
		    strcat(gmt, "0");
		}
		strcat(gmt, s);
	    }
	}
	else {
	    clock = (time_t)floor(epochal + .5);
	    if(epochal < 0.) clock = clock - 1;
	    date = (char *)asctime_r(gmtime_r(&clock, &result), buf);
	    strncpy(gmt, date+11, 8);
	    gmt[8] = '\0';
	}
	return gmt;
}

/**
 * Convert a DateTime to epochal time. (seconds since 1970/1/1).
 * The DateTime structure is defined as:
 * <pre>
 *
 * typedef struct {
 *    int year;
 *    int month;
 *    int day;
 *    int hour;
 *    int minute;
 *    double second;
 * } DateTime;
 * </pre>
 * @param dt Input DateTime structure.
 * @return The epochal time.
 */
double
timeDateToEpoch(DateTime *dt)
{
	int doy = timeDOY(dt);
	double days = 0.;
	int year = dt->year;

	if(year > 1970) {
	    while(--year >= 1970)
		days += isLeapYear(year) ? 366 : 365;
	}
	else if(year < 1970) {
	    while(year < 1970) {
		days -= isLeapYear(year) ? 366 : 365;
		year++;
	    }
	}
	return (days + doy-1)*86400. + 3600*dt->hour + 60.*dt->minute
			+ dt->second;
}

/**
 * Return the day of year for a DateTime.
 * @param dt Input DateTime structure.
 * @return The day of year, beginning with 1.
 */
int
timeDOY(DateTime *dt)
{
	return timeDayOfYear(dt->year, dt->month, dt->day);
}

/**
 * Return the day of year for input year, month and day.
 * @param year The year (four digits).
 * @param month The month (1-12)
 * @param day The day of the month (1-31)
 * @return the day of year beginning with 1.
 */
int
timeDayOfYear(int year, int month, int day)
{
	int i, dim, doy;

	doy = 0;
	for(i = 0 ; i < month - 1 ; i++ ){
	    dim = days_in_month[i];
	    if(i == 1 && isLeapYear(year) ) dim++;
	    doy += dim;
	}
	doy += day;
	return doy;
}

#define mod(a,b) ((a) - ((int)((a)/(b))) * (b))

/**
 * Convert an epochal time to a DateTime.
 * @param epoch The input epochal time.
 * @param dt A pointer to a DataTime structure that will be filled.
 */
void
timeEpochToDate(double epoch, DateTime *dt)
{
	int doy, i;
	double secleft;

	dt->hour = dt->minute = 0;
        dt->second = 0;
	doy = (int)(epoch / 86400.);
	secleft = mod(epoch,86400.0);

        if(secleft) {		/* compute hours minutes seconds */
	    if(secleft < 0) {	/* before 1970 */
		doy--;			/* subtract a day */
		secleft += 86400;	/* add a day */
	    }
	    dt->hour = (int)(secleft/3600);
	    secleft = mod(secleft, 3600.);
	    dt->minute = (int)(secleft/60.);
	    dt->second = mod(secleft, 60.);
	}

	if(doy >= 0){
	    for( dt->year = 1970 ; ; dt->year++ ){
		int diy = isLeapYear(dt->year) ? 366 : 365;
		if( doy < diy ) break;
		doy -= diy;
	    }
	}
	else{
	    for( dt->year = 1969 ; ; dt->year-- ){
		int diy = isLeapYear(dt->year) ? 366 : 365;
		doy += diy;
		if( doy >= 0 ) break;
	    }
	}
	doy++;

	dt->day = doy;

	for( i = 0 ; i < 12 ; i ++ ){
	    int dim = days_in_month[i];
	    if( isLeapYear(dt->year) && i == 1 ) dim++;
	    if( dt->day <= dim ) break;
	    dt->day -= dim;
	}
	dt->month = i + 1;
}

/**
 * Return the month name. Return a character string month name for a month
 * index, beginning with 1 for January.
 * @param month The index of the month beginning with 1.
 * @return a static char string containing the month name.
 */
const char *
timeMonthName(int month)
{
	if(month > 0 && month <= 12) return month_name[month];
	return month_name[0];
}

/**
 * Return a standard load date string. Return a pointer to a 9 character
 * string containing the system time in the form: "01-NOV-91".
 */
char *
timeLoadDate(void)
{
	static char buf[25];
	long clock;
	char s[50];
	struct tm result;

	clock = (long) timeGetEpoch();

	strncpy(buf, asctime_r(gmtime_r(&clock, &result), s), 24);
	/*
	 *       012345678901234567890123
	 * buf: "Fri Nov  1 15:16:47 1991"
	 */

	buf[0] = (buf[8] == ' ') ? '0' : buf[8];
	buf[1] = buf[9];
	buf[2] = '-';
	/* uppercase conversion of the month */
	buf[3] = (buf[4] < 'A' || buf[4] > 'Z') ? 'A' + buf[4] - 'a' : buf[4];
	buf[4] = (buf[5] < 'A' || buf[5] > 'Z') ? 'A' + buf[5] - 'a' : buf[5];
	buf[5] = (buf[6] < 'A' || buf[6] > 'Z') ? 'A' + buf[6] - 'a' : buf[6];
	buf[6] = '-';
	buf[7] = buf[22];
	buf[8] = buf[23];
	buf[9] = '\0';

	return(buf);
}

/**
 * Return a date string. Return a pointer to a 11 character date string 
 * in the form: "2004-NOV-15". The string "-" is returned for an invalid date.
 */
char *
timeDateString(DateTime *dt)
{
	static char date_string[12];

	if(dt->year < 0 || dt->year > 9999 || dt->month < 0 || dt->month > 12
		|| dt->day < 1 || dt->day > 31)
	{
	    strcpy(date_string, "-");
	}
	else {
	    snprintf(date_string, sizeof(date_string), "%04d-%s-%02d", dt->year,
			month_name[dt->month], dt->day);
	}
	return date_string;
}

/**
 * Return a date string. Return a pointer to a 11 character date string 
 * in the form: "15-NOV-2004". The string "-" is returned for an invalid date.
 */
char *
timeDateStringR(DateTime *dt)
{
	static char date_string[12];

	if(dt->year < 0 || dt->year > 9999 || dt->month < 0 || dt->month > 12
		|| dt->day < 1 || dt->day > 31)
	{
	    strcpy(date_string, "-");
	}
	else {
	    snprintf(date_string, sizeof(date_string), "%02d-%s-%04d", dt->day,
			month_name[dt->month], dt->year);
	}
	return date_string;
}

/**
 * Return system time as an epochal time. Return the system time in seconds
 * since 00:00:00 1970/1/1. Returns NULL_TIME if cannot get system time.
 */
double
timeGetEpoch(void)
{
	struct timeval  t;
	struct timezone tz;

	if(gettimeofday(&t,&tz) == -1)
	{
	    perror("gettimeofday");
	    return NULL_TIME;
	}

	return((double)t.tv_sec + 1.e-6*(double)t.tv_usec);
}

/**
 * Return epochal days. Return the number of days for the DataTime since
 * 1/1/1970.
 * @param dt The input DateTime structure.
 * @return the number of days since 1970/1/1.
 */
int
timeEpochDays(DateTime *dt)
{
	int doy = timeDOY(dt);

	if(( dt->year - 1970 ) > 0 )
	{
	    return (dt->year-1970)*365 + doy - 1;
        }
	else
	{
	    return doy - 1;
	}
}

/**
 * Convert day of year to month and day of month.
 * @param year Input year.
 * @param day_of_year Input day of year.
 * @param month Output month of year beginning with 1.
 * @param day Output day of month (1-31).
 */
void
timeMonthDay(int year, int day_of_year, int *month, int *day)
{
	int i, dim;

	*day = day_of_year;
	for( i = 0 ; i < 12 ; i ++ ) {
	    dim = days_in_month[i];
	    if( isLeapYear(year) && i == 1 ) dim++;
	    if( *day <= dim ) break;
	    *day -= dim;
	}
	*month = i + 1;
}

/**
 * Return the JDate. Return the JDate for a DateTime. The JDate is
 * 1000*year + day_of_year
 * @param dt Input DateTime structure.
 * @return JDate.
 */
int
timeJDate(DateTime *dt)
{
	return 1000*dt->year + timeDOY(dt);
}


/**
 * Return the JDate. Return the JDate for an epoch time. The JDate is
 * 1000*year + day_of_year
 * @param epoch The epoch time.
 * @return JDate.
 */
int
timeEpochToJDate(double epoch)
{
        DateTime dt;

	timeEpochToDate(epoch, &dt);
	return timeJDate(&dt);
}

