/*
 * NAME
 *      timeParseString
 *
 * AUTHOR
 *      J. Coyne
 *      SAIC
 *
 *	I. Henson
 *	Scientific Computing
 */

/**
 * Some basic time parsing and formatting routines.
 */

#include "config.h"
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "libtime.h"
#include "libstring.h"
#include "logErrorMsg.h"

static void correct_60_problem(DateTime *dt, const char *sec_format);
static int hoursInMonth(DateTime *dt );
static int parseInt(const char *str, int *n);
static int ptime(const char *string, const char *format, struct tm *tm,
		double *frac, char **s);
static int getTimeTerm(char *s, char **next, double *d);
static int endsWith(char *s, const char *end, double *d);
static int is_space(char c);

extern char *strptime(const char *s, const char *format, struct tm *tm);

/*
	the goal is to come up with something which returns epoch time, given
	expressions like:

	now
	now - 2d
	now - 36h
	now - 5
	now - 5s
	1992/08/13
	1992/08/13 - 2d
	1993/8/14 12:32
	328443434.
*/

/**
 * Parse a string for a time expression. Return the time as the epochal time,
 * seconds since 1970/1/1. Valid expressions start with either "now" or
 * a time in one of the many formats shown below. The expression can also
 * have additional terms of relative time to be added or subtracted from the
 * first time. Acceptable formats for the relative-time terms include
 * key words and abbreviations for time units:
 * <pre>
 * For years: "years" "year" "yrs" "yr" "y"
 *
 * For months: "months" "month" "mon" 
 *
 * For weeks: "weeks" "week" "w"
 *
 * For days: "days" "day" "d"
 *
 * For hours: "hours" "hrs" "hr" "h"
 *
 * For minutes: "minutes" "minute" "min" "mn" "m"
 *
 * For seconds: "seconds" "second" "secs" "sec" "sc" "s"
 * </pre>
 * Examples:
 * <pre>
 *     now - 3d - 2h
 *     now + 1.5d
 *     1993/8/14 12:32 - 2hrs
 *     1997/03/12  12:23:39.7
 *     97464643.
 *     97464643. + 5s + 1h
 *     2003Jan25 12:23:39.7
 *     1995/123  12:23:39.7
 *     2001-11-12 18:31:01
 * </pre>
 * <p>
 * timeParseString recognizes string in the following strftime/strptime time
 * formats:
 * <pre>
 *
 *      strftime format             example
 *
 *     "%y%b%d %n%H:%M:%S"       03Jan25 hh:mm:ss
 *     "%y%b%d %n%H %M %S"       03Jan25 hh mm ss
 *     "%y-%b-%d %n%H:%M:%S"     03-Jan-25 hh:mm:ss
 *     "%y-%b-%d %n%H %M %S"     03-Jan-25 hh mm ss
 *     "%y/%b/%d %n%H:%M:%S"     03/Jan/25 hh:mm:ss
 *     "%y/%b/%d %n%H %M %S"     03/Jan/25 hh mm ss
 *     "%Y%b%d %n%H:%M:%S"       2003Jan25 hh:mm:ss
 *     "%Y%b%d %n%H %M %S"       2003Jan25 hh mm ss
 *     "%Y-%b-%d %n%H:%M:%S"     2003-Jan-25 hh:mm:ss
 *     "%Y-%b-%d %n%H %M %S"     2003-Jan-25 hh mm ss
 *     "%Y/%b/%d %n%H:%M:%S"     2003/Jan/25 hh:mm:ss
 *     "%Y/%b/%d %n%H %M %S"     2003/Jan/25 hh mm ss
 *     "%Y-%m-%d %n%H:%M:%S"     2003-01-25 hh:mm:ss
 *     "%Y-%m-%d %n%H %M %S"     2003-01-25 hh mm ss
 *     "%Y/%m/%d %n%H:%M:%S"     2003/01/25 hh:mm:ss
 *     "%Y/%m/%d %n%H %M %S"     2003/01/25 hh mm ss
 *     "%Y%m%d %n%H:%M:%S"	20030125 hh:mm:ss
 *     "%y-%m-%d %n%H:%M:%S"     03-01-25 hh:mm:ss
 *     "%y-%m-%d %n%H %M %S"     03-01-25 hh mm ss
 *     "%y/%m/%d %n%H:%M:%S"     03/01/25 hh:mm:ss
 *     "%y/%m/%d %n%H %M %S"     03/01/25 hh mm ss
 *     
 *     "%y%b%d %n%H:%M"          03Jan25 hh:mm
 *     "%y%b%d %n%H %M"          03Jan25 hh mm
 *     "%y-%b-%d %n%H:%M"        03-Jan-25 hh:mm
 *     "%y-%b-%d %n%H %M"        03-Jan-25 hh mm
 *     "%y/%b/%d %n%H:%M"        03/Jan/25 hh:mm
 *     "%y/%b/%d %n%H %M"        03/Jan/25 hh mm
 *     "%Y%b%d %n%H:%M"          2003Jan25 hh:mm
 *     "%Y%b%d %n%H %M"          2003Jan25 hh mm
 *     "%Y-%b-%d %n%H:%M"        2003-Jan-25 hh:mm
 *     "%Y-%b-%d %n%H %M"        2003-Jan-25 hh mm
 *     "%Y/%b/%d %n%H:%M"        2003/Jan/25 hh:mm
 *     "%Y/%b/%d %n%H %M"        2003/Jan/25 hh mm
 *     "%Y-%m-%d %n%H:%M"        2003-01-25 hh:mm
 *     "%Y-%m-%d %n%H %M"        2003-01-25 hh mm
 *     "%Y/%m/%d %n%H:%M"        2003/01/25 hh:mm
 *     "%Y/%m/%d %n%H %M"        2003/01/25 hh mm
 *     "%y-%m-%d %n%H:%M"        03-01-25 hh:mm
 *     "%y-%m-%d %n%H %M"        03-01-25 hh mm
 *     "%y/%m/%d %n%H:%M"        03/01/25 hh:mm
 *     "%y/%m/%d %n%H %M"        03/01/25 hh mm
 *     
 *     "%y%b%d %n%H"             03Jan25 hh
 *     "%y-%b-%d %n%H"           03-Jan-25 hh
 *     "%y/%b/%d %n%H"           03/Jan/25 hh
 *     "%Y%b%d %n%H"             2003Jan25 hh
 *     "%Y-%b-%d %n%H"           2003-Jan-25 hh
 *     "%Y/%b/%d %n%H"           2003/Jan/25 hh
 *     "%Y-%m-%d %n%H"           2003-01-25 hh
 *     "%Y/%m/%d %n%H"           2003/01/25 hh
 *     "%y-%m-%d %n%H"           03-01-25 hh
 *     "%y/%m/%d %n%H"           03/01/25 hh
 *     
 *     "%y%b%d"                 03Jan25
 *     "%d-%b-%Y"               25-Jan-2003
 *     "%y-%b-%d"               03-Jan-25
 *     "%y/%b/%d"               03/Jan/25
 *     "%Y%b%d"                 2003Jan25
 *     "%Y-%b-%d"               2003-Jan-25
 *     "%Y/%b/%d"               2003/Jan/25
 *     "%Y-%m-%d"               2003-01-25
 *     "%Y/%m/%d"               2003/01/25
 *     "%y-%m-%d"               03-01-25
 *     "%y/%m/%d"               03/01/25
 *     
 *     "%y%b"                   03Jan
 *     "%y-%b"                  03-Jan
 *     "%y/%b"                  03/Jan
 *     "%Y%b"                   2003Jan
 *     "%Y-%b"                  2003-Jan
 *     "%Y/%b"                  2003/Jan
 *     
 *     "%Y/%j %n%H:%M:%S"        2003/123 hh:mm:ss
 *     "%Y/%j %n%H %M %S"        2003/123 hh mm ss
 *     "%Y/%j %n%H:%M"           2003/123 hh:mm
 *     "%Y/%j %n%H %M"           2003/123 hh mm
 *     "%Y/%j %n%H"              2003/123 hh
 *     "%Y/%j"                  2003/123
 * </pre>
 * @param string The string time expression.
 * @param time The epochal time returned.
 * @return 1 for success, 0 for failure to parse.
 */
int
timeParseString(const char *string, double *time)
{
    char *c, s[128];
    int i, ret, n_first_term;
    double t, f = 0.;
    DateTime dt;
    struct tm tm = {0,0,0,1,1,0,0,1,0};

    if((int)strlen(string) > 127) return -1;
    stringcpy(s, string, 128);
    stringToLower(s);
    stringTrim(s);
    for(i = 0; s[i] != '\0' && !isspace((int)s[i]); i++);
    n_first_term = i;
    for(i = 0; s[i] != '\0'; i++) {
	if(isspace((int)s[i]) && !(i > 0 && s[i-1] == '#')) s[i] = '#';
    }

    if(ptime(s, "%Y%m%d#%n%H:%M:%S",  &tm,&f,&c) )/* 20030125 hh:mm:ss */
    {
	dt.year = 1900 + tm.tm_year;
	dt.month = tm.tm_mon + 1;
	dt.day = tm.tm_mday;
	dt.hour = tm.tm_hour;
	dt.minute = tm.tm_min;
	dt.second = tm.tm_sec + f;
	
	*time = timeDateToEpoch(&dt);

	while((ret = getTimeTerm(c, &c, &t)) > 0) {
	    *time += t;
	}
	if(ret == 0) return 1;
    }
    else if(!strcmp(s, "none") || !strcmp(s, "null") || !strcmp(s, "-")) {
	*time = NULL_TIME;
	return 1;
    }
    else if(!strncmp(s, "now", 3))
    {
	*time = timeGetEpoch();
	c = s+3;
	while((ret = getTimeTerm(c, &c, &t)) > 0) {
	    *time += t;
	}
	if(ret == 0) return 1;
    }
    else if(n_first_term == 7 && ptime(s, "%Y%j",&tm,NULL,&c))
    {
	/* 2003123 */
	dt.year = 1900 + tm.tm_year;
	dt.month = tm.tm_mon + 1;
	dt.day = tm.tm_mday;
	dt.hour = tm.tm_hour;
	dt.minute = tm.tm_min;
	dt.second = tm.tm_sec + f;
	
	*time = timeDateToEpoch(&dt);

	while((ret = getTimeTerm(c, &c, &t)) > 0) {
	    *time += t;
	}
	if(ret == 0) return 1;
    }
    else if(getTimeTerm(s, &c, time) > 0) {
	while((ret = getTimeTerm(c, &c, &t)) > 0) {
	    *time += t;
	}
	if(ret == 0) return 1;
    }
    else if(
	ptime(s, "%y%b%d#%n%H:%M:%S",  &tm,&f,&c) ||/* 03Jan25 hh:mm:ss */
	ptime(s, "%y%b%d#%n%H %M %S",  &tm,&f,&c) ||/* 03Jan25 hh mm ss */
	ptime(s, "%y-%b-%d#%n%H:%M:%S",&tm,&f,&c) ||/* 03-Jan-25 hh:mm:ss */
	ptime(s, "%y-%b-%d#%n%H %M %S",&tm,&f,&c) ||/* 03-Jan-25 hh mm ss */
	ptime(s, "%y/%b/%d#%n%H:%M:%S",&tm,&f,&c) ||/* 03/Jan/25 hh:mm:ss */
	ptime(s, "%y/%b/%d#%n%H %M %S",&tm,&f,&c) ||/* 03/Jan/25 hh mm ss */
	ptime(s, "%Y%b%d#%n%H:%M:%S",  &tm,&f,&c) ||/* 2003Jan25 hh:mm:ss */
	ptime(s, "%Y%b%d#%n%H %M %S",  &tm,&f,&c) ||/* 2003Jan25 hh mm ss */
	ptime(s, "%Y-%b-%d#%n%H:%M:%S",&tm,&f,&c) ||/* 2003-Jan-25 hh:mm:ss */
	ptime(s, "%Y-%b-%d#%n%H %M %S",&tm,&f,&c) ||/* 2003-Jan-25 hh mm ss */
	ptime(s, "%Y/%b/%d#%n%H:%M:%S",&tm,&f,&c) ||/* 2003/Jan/25 hh:mm:ss */
	ptime(s, "%Y/%b/%d#%n%H %M %S",&tm,&f,&c) ||/* 2003/Jan/25 hh mm ss */
	ptime(s, "%Y-%m-%d#%n%H:%M:%S",&tm,&f,&c) ||/* 2003-01-25 hh:mm:ss */
	ptime(s, "%Y-%m-%d#%n%H %M %S",&tm,&f,&c) ||/* 2003-01-25 hh mm ss */
	ptime(s, "%Y/%m/%d#%n%H:%M:%S",&tm,&f,&c) ||/* 2003/01/25 hh:mm:ss */
	ptime(s, "%Y/%m/%d#%n%H %M %S",&tm,&f,&c) ||/* 2003/01/25 hh mm ss */
	ptime(s, "%y-%m-%d#%n%H:%M:%S",&tm,&f,&c) ||/* 03-01-25 hh:mm:ss */
	ptime(s, "%y-%m-%d#%n%H %M %S",&tm,&f,&c) ||/* 03-01-25 hh mm ss */
	ptime(s, "%y/%m/%d#%n%H:%M:%S",&tm,&f,&c) ||/* 03/01/25 hh:mm:ss */
	ptime(s, "%y/%m/%d#%n%H %M %S",&tm,&f,&c) ||/* 03/01/25 hh mm ss */
	ptime(s, "%m/%d/%y#%n%H:%M:%S",&tm,&f,&c) ||/* 01/25/03 hh:mm:ss */

	ptime(s, "%y%b%d#%n%H:%M",  &tm,NULL,&c) || /* 03Jan25 hh:mm */
	ptime(s, "%y%b%d#%n%H %M",  &tm,NULL,&c) || /* 03Jan25 hh mm */
	ptime(s, "%y-%b-%d#%n%H:%M",&tm,NULL,&c) || /* 03-Jan-25 hh:mm */
	ptime(s, "%y-%b-%d#%n%H %M",&tm,NULL,&c) || /* 03-Jan-25 hh mm */
	ptime(s, "%y/%b/%d#%n%H:%M",&tm,NULL,&c) || /* 03/Jan/25 hh:mm */
	ptime(s, "%y/%b/%d#%n%H %M",&tm,NULL,&c) || /* 03/Jan/25 hh mm */
	ptime(s, "%Y%b%d#%n%H:%M",  &tm,NULL,&c) || /* 2003Jan25 hh:mm */
	ptime(s, "%Y%b%d#%n%H %M",  &tm,NULL,&c) || /* 2003Jan25 hh mm */
	ptime(s, "%Y-%b-%d#%n%H:%M",&tm,NULL,&c) || /* 2003-Jan-25 hh:mm */
	ptime(s, "%Y-%b-%d#%n%H %M",&tm,NULL,&c) || /* 2003-Jan-25 hh mm */
	ptime(s, "%Y/%b/%d#%n%H:%M",&tm,NULL,&c) || /* 2003/Jan/25 hh:mm */
	ptime(s, "%Y/%b/%d#%n%H %M",&tm,NULL,&c) || /* 2003/Jan/25 hh mm */
	ptime(s, "%Y-%m-%d#%n%H:%M",&tm,NULL,&c) || /* 2003-01-25 hh:mm */
	ptime(s, "%Y-%m-%d#%n%H %M",&tm,NULL,&c) || /* 2003-01-25 hh mm */
	ptime(s, "%Y/%m/%d#%n%H:%M",&tm,NULL,&c) || /* 2003/01/25 hh:mm */
	ptime(s, "%Y/%m/%d#%n%H %M",&tm,NULL,&c) || /* 2003/01/25 hh mm */
	ptime(s, "%y-%m-%d#%n%H:%M",&tm,NULL,&c) || /* 03-01-25 hh:mm */
	ptime(s, "%y-%m-%d#%n%H %M",&tm,NULL,&c) || /* 03-01-25 hh mm */
	ptime(s, "%y/%m/%d#%n%H:%M",&tm,NULL,&c) || /* 03/01/25 hh:mm */
	ptime(s, "%y/%m/%d#%n%H %M",&tm,NULL,&c) || /* 03/01/25 hh mm */

	ptime(s, "%y%b%d#%n%H",  &tm,NULL,&c) || /* 03Jan25 hh */
	ptime(s, "%y-%b-%d#%n%H",&tm,NULL,&c) || /* 03-Jan-25 hh */
	ptime(s, "%y/%b/%d#%n%H",&tm,NULL,&c) || /* 03/Jan/25 hh */
	ptime(s, "%Y%b%d#%n%H",  &tm,NULL,&c) || /* 2003Jan25 hh */
	ptime(s, "%Y-%b-%d#%n%H",&tm,NULL,&c) || /* 2003-Jan-25 hh */
	ptime(s, "%Y/%b/%d#%n%H",&tm,NULL,&c) || /* 2003/Jan/25 hh */
	ptime(s, "%Y-%m-%d#%n%H",&tm,NULL,&c) || /* 2003-01-25 hh */
	ptime(s, "%Y/%m/%d#%n%H",&tm,NULL,&c) || /* 2003/01/25 hh */
	ptime(s, "%y-%m-%d#%n%H",&tm,NULL,&c) || /* 03-01-25 hh */
	ptime(s, "%y/%m/%d#%n%H",&tm,NULL,&c) || /* 03/01/25 hh */

	ptime(s, "%y%b%d",  &tm,NULL,&c) ||   /* 03Jan25 */
	ptime(s, "%d-%b-%Y",&tm,NULL,&c) ||   /* 25-Jan-2003 */

	ptime(s, "%d-%b-%y",&tm,NULL,&c) ||   /* 25-Jan-03 */

	ptime(s, "%y-%b-%d",&tm,NULL,&c) ||   /* 03-Jan-25 */
	ptime(s, "%y/%b/%d",&tm,NULL,&c) ||   /* 03/Jan/25 */
	ptime(s, "%Y%b%d",  &tm,NULL,&c) ||   /* 2003Jan25 */
	ptime(s, "%Y-%b-%d",&tm,NULL,&c) ||   /* 2003-Jan-25 */
	ptime(s, "%Y/%b/%d",&tm,NULL,&c) ||   /* 2003/Jan/25 */
	ptime(s, "%Y-%m-%d",&tm,NULL,&c) ||   /* 2003-01-25 */
	ptime(s, "%Y/%m/%d",&tm,NULL,&c) ||   /* 2003/01/25 */
	ptime(s, "%y-%m-%d",&tm,NULL,&c) ||   /* 03-01-25 */
	ptime(s, "%y/%m/%d",&tm,NULL,&c) ||   /* 03/01/25 */

	ptime(s, "%y%b", &tm,NULL,&c) ||   /* 03Jan */
	ptime(s, "%y-%b",&tm,NULL,&c) ||   /* 03-Jan */
	ptime(s, "%y/%b",&tm,NULL,&c) ||   /* 03/Jan */
	ptime(s, "%Y%b", &tm,NULL,&c) ||   /* 2003Jan */
	ptime(s, "%Y-%b",&tm,NULL,&c) ||   /* 2003-Jan */
	ptime(s, "%Y/%b",&tm,NULL,&c) ||   /* 2003/Jan */

	ptime(s, "%Y/%j#%n%H:%M:%S",&tm,&f,&c) || /*2003/123 hh:mm:ss */
	ptime(s, "%Y/%j#%n%H %M %S",&tm,&f,&c) || /*2003/123 hh mm ss */
	ptime(s, "%Y/%j#%n%H:%M", &tm,NULL,&c) || /*2003/123 hh:mm */
	ptime(s, "%Y/%j#%n%H %M", &tm,NULL,&c) || /*2003/123 hh mm */
	ptime(s, "%Y/%j#%n%H",&tm,&f,&c)	      || /*2003/123 hh */
	ptime(s, "%Y/%j",&tm,NULL,&c))		 /*2003/123 */
    {
	dt.year = 1900 + tm.tm_year;
	dt.month = tm.tm_mon + 1;
	dt.day = tm.tm_mday;
	dt.hour = tm.tm_hour;
	dt.minute = tm.tm_min;
	dt.second = tm.tm_sec + f;
	
	*time = timeDateToEpoch(&dt);

	while((ret = getTimeTerm(c, &c, &t)) > 0) {
	    *time += t;
	}
	if(ret == 0) return 1;
    }
    return 0;
}

/**
 * Parse a jdate string. The string that can be in the following formats.
 *     
 *     "%Y%j"			2003123
 *     "%y%b%d"                 03Jan25
 *     "%d-%b-%Y"               25-Jan-2003
 *     "%y-%b-%d"               03-Jan-25
 *     "%y/%b/%d"               03/Jan/25
 *     "%Y%b%d"                 2003Jan25
 *     "%Y-%b-%d"               2003-Jan-25
 *     "%Y/%b/%d"               2003/Jan/25
 *     "%Y-%m-%d"               2003-01-25
 *     "%Y/%m/%d"               2003/01/25
 *     "%y-%m-%d"               03-01-25
 *     "%y/%m/%d"               03/01/25
 * @param string The string jdate.
 * @param jdate The long jdate returned.
 * @return 1 for success, 0 for failure to parse.
 */
int
timeParseJDate(const char *string, long *jdate)
{
    char *c;
    struct tm tm = {0,0,0,1,1,0,0,1,0};

    if(!strcmp(string, "-1") || !strcmp(string, "-")) { /* null */
	*jdate = -1;
	return 1;
    }
    else if(
	ptime(string, "%Y%j",    &tm,NULL,&c) ||	/* 2003123 */
	ptime(string, "%y%b%d",  &tm,NULL,&c) ||	/* 03Jan25 */
	ptime(string, "%d-%b-%Y",&tm,NULL,&c) ||	/* 25-Jan-2003 */
	ptime(string, "%d-%b-%y",&tm,NULL,&c) ||	/* 25-Jan-03 */
	ptime(string, "%y-%b-%d",&tm,NULL,&c) ||	/* 03-Jan-25 */
	ptime(string, "%y/%b/%d",&tm,NULL,&c) ||	/* 03/Jan/25 */
	ptime(string, "%Y%b%d",  &tm,NULL,&c) ||	/* 2003Jan25 */
	ptime(string, "%Y-%b-%d",&tm,NULL,&c) ||	/* 2003-Jan-25 */
	ptime(string, "%Y/%b/%d",&tm,NULL,&c) ||	/* 2003/Jan/25 */
	ptime(string, "%Y-%m-%d",&tm,NULL,&c) ||	/* 2003-01-25 */
	ptime(string, "%Y/%m/%d",&tm,NULL,&c) ||	/* 2003/01/25 */
	ptime(string, "%y-%m-%d",&tm,NULL,&c) ||	/* 03-01-25 */
	ptime(string, "%y/%m/%d",&tm,NULL,&c))		/* 03/01/25 */
    {
	DateTime dt;
	dt.year = 1900 + tm.tm_year;
	dt.month = tm.tm_mon + 1;
	dt.day = tm.tm_mday;
	dt.hour = 0;
	dt.minute = 0;
	dt.second = 0.;
	*jdate = timeJDate(&dt);
	return 1;
    }
    return 0;
}

static int
endsWith(char *s, const char *end, double *d)
{
	char c;
	int n, m, ret;

	if(stringEndsWith(s, end)) {
	    n = strlen(end);
	    m = strlen(s);
	    if(n >= m) return 0;
	    c = s[m-n];
	    s[m-n] = '\0';
	    ret = stringToDouble(s, d);
	    s[m-n] = c;
	    return ret;
	}
	return 0;
}

static int
getTimeTerm(char *s, char **next, double *d)
{
	int i, sign;
	char *c, save;

	for(i = 0; s[i] != '\0' && is_space(s[i]); i++);
	if(s[i] == '\0') {
	    *next = s+i;
	    return 0;
	}

	if(s[i] == '+') {
	    sign = 1;
	    for(++i; s[i] != '\0' && is_space(s[i]); i++);
	}
	else if(s[i] == '-') {
	    sign = -1;
	    for(++i; s[i] != '\0' && is_space(s[i]); i++);
	}
	else {
	    sign = 1;
	}

	s = s+i;
	for(c = s; *c != '\0' && !is_space(*c); c++);
	save = *c;
	*c = '\0';

	if(stringToDouble(s, d)) {
	    *d *= sign;
	}
	else if(endsWith(s, "years", d) ||
		endsWith(s, "year", d) ||
		endsWith(s, "yrs", d) ||
		endsWith(s, "yr", d) ||
		endsWith(s, "y", d))
	{
	    *d *= sign*365*24*60*60;
	}
	else if(endsWith(s, "months", d) ||
		endsWith(s, "month", d) ||
		endsWith(s, "mon", d))
	{
	    *d *= sign*31*24*60*60;
	}
	else if(endsWith(s, "weeks", d) ||
		endsWith(s, "week", d) ||
		endsWith(s, "w", d))
	{
	    *d *= sign*7*24*60*60;
	}
	else if(endsWith(s, "days", d) ||
		endsWith(s, "day", d) ||
		endsWith(s, "d", d))
	{
	    *d *= sign*24*60*60;
	}
	else if(endsWith(s, "hours", d) ||
		endsWith(s, "hrs", d) ||
		endsWith(s, "hr", d) ||
		endsWith(s, "h", d))
	{
	    *d *= sign*60*60;
	}
	else if(endsWith(s, "minutes", d) ||
		endsWith(s, "minute", d) ||
		endsWith(s, "min", d) ||
		endsWith(s, "mn", d) ||
		endsWith(s, "m", d))
	{
	    *d *= sign*60;
	}
	else if(endsWith(s, "seconds", d) ||
		endsWith(s, "second", d) ||
		endsWith(s, "secs", d) ||
		endsWith(s, "sec", d) ||
		endsWith(s, "sc", d) ||
		endsWith(s, "s", d))
	{
	    *d *= sign;
	}
	else {
	    *c = save;
	    return -1;
	}

	*c = save;
	*next = c;
	return 1;
}

static int
ptime(const char *string, const char *format, struct tm *tm, double *frac,
		char **s)
{
	char *c, *p, a;
	int i, n, m = (int)strlen(format);

	for(n = 0; string[n] != '\0' && !isspace((int)string[n]); n++);

	if(!strncmp(format, "%y", 2)) {
	    /* require 2-digit year */
	    for(i = 0; string[i] != '\0' && isdigit((int)string[i]); i++);
	    if(i != 2) return 0;
	}
	else if(!strncmp(format, "%Y", 2)) {
	    /* require 4-digit year */
	    for(i = 0; i < 4 && string[i] != '\0' && isdigit((int)string[i]); i++);
	    if(i != 4) return 0;
	}
	else if(m > 0 && format[m-1] == 'y') {
	    /* require 2-digit year */
	    for(i = n-1; i >= 0 && isdigit((int)string[i]); i--);
	    if(n-1-i != 2) return 0;
	}
	else if(m > 0 && format[m-1] == 'Y') {
	    /* require 4-digit year */
	    for(i = n-1; i >= 0 && isdigit((int)string[i]); i--);
	    if(n-1-i != 4) return 0;
	}

	if((c = strptime(string, format, tm)) == NULL) return 0;

	*s = c;

	if(frac == NULL) return (*c == '\0' || isspace((int)*c)) ? 1 : 0;

	*frac = 0.;
	if(*c == '\0') {
	    return 1;
	}
	else if(*c == '.') {
	    if(c[1] == '\0') {
		*s = c+1;
	    }
	    else {
		for(p = c; *p != '\0' && !is_space(*p); p++);
		a = *p;
		*p = '\0';
		if(!stringToDouble(c, frac)) {
		    return 0;
		}
		*p = a;
		*s = p;
	    }
	}
	else if(!is_space(*c)) {
	    return 0;
	}
	return 1;
}

/**
 * Print a date and time. Print the data and time that an epochal double
 * represents, using the input TimeFormat. The TimeFormat codes are:
 * <pre>
 *
 *   code               format                           args
 *
 * YMONDHMS     "%04d%s%02d %02d:%02d:%06.3f"      yr,mname,day,hr,min,sec
 * YMONDHMS2    "%04d%s%02d %02d:%02d:%04.1f"      yr,mname,day,hr,min,sec
 * YMONDHMS3    "%04d%s%02d %02d:%02d:%02.0f"      yr,mname,day,hr,min,sec
 * YMOND        "%04d%s%02d"                       yr,mname,day
 * HMS          "%02d:%02d:%06.3f"                 hr,min,sec
 * HMS2         "%02d:%02d:%04.1f"                 hr,min,sec
 * HMS3         "%02d:%02d:%02.0f"                 hr,min,sec
 * GSE20        "%04d/%02d/%02d %02d:%02d:%04.1f"  yr,mon,day,hr,min,sec
 * GSE21        "%04d/%02d/%02d %02d:%02d:%06.3f"  yr,mon,day,hr,min,sec
 * MONTH_HOURS  "%03d:%02d:%06.3f"                 epoch_hours,min,sec
 * MONTH_HOURS2 "%03d:%02d:%02.0f"                 epoch_hours,min,sec
 * MONTH_HOURS3 "%03d:%02d:%04.1"		   epoch_hours,min,sec
 * YMD          "%04d-%02d-%02d"                   yr,mon,day
 * DHMS         "%03d %02d:%02d:%02.0f"            day-1,hr,min,sec
 * EPOCH_DAYS   "%04d %02d:%02d:%02.0f"            epoch_days,hr,min,sec
 * EPOCH_DAYS2  "%03d %02d:%02d:%04.1f"            epoch_days,hr,min,sec
 * HHMMSS       "%02d:%02d:%02.0f"                 epoch_hours,min,sec
 * </pre>
 * @param epoch The epochal time to print. If epoch is <= NULL_TIME_CHECK, the \
 *	string str will be set to "none".
 * @param str A string to hold the printed date/time.
 * @param len The size of <b>str</b>.
 * @param format One of the format codes above.
 * @return str
 */
char *
timeEpochToString(double epoch, char *str, int len, enum TimeFormat format)
{
	
	DateTime dt;

	if(epoch < NULL_TIME_CHECK) {
	    stringcpy(str, "none", len);
	    return str;
	}

	timeEpochToDate(epoch, &dt);

	if (format == YMONDHMS)
	{
		correct_60_problem(&dt, "%6.3f");
		snprintf(str, len, "%04d%s%02d %02d:%02d:%06.3f", 
			dt.year, timeMonthName(dt.month), dt.day,
			dt.hour, dt.minute, dt.second);
	}
	else if(format == YMONDHMS3)
	{
		correct_60_problem(&dt, "%2.0f");
		snprintf(str, len, "%04d%s%02d %02d:%02d:%02.0f", 
			dt.year, timeMonthName(dt.month), dt.day,
			dt.hour, dt.minute, dt.second);
	}
	else if (format == YMOND)
	{
		snprintf(str, len, "%04d%s%02d",
			dt.year, timeMonthName(dt.month), dt.day);
	}
	else if (format == HMS)
	{
		correct_60_problem(&dt, "%6.3f");
		snprintf(str, len, "%02d:%02d:%06.3f",
			dt.hour, dt.minute, dt.second);
	}
	else if (format == HMS2)
	{
		correct_60_problem(&dt, "%4.1f");
		snprintf(str, len, "%02d:%02d:%04.1f", 
			dt.hour, dt.minute, dt.second);
	}
	else if (format == HMS3)
	{
		correct_60_problem(&dt, "%2.0f");
		snprintf(str, len, "%02d:%02d:%02.0f", 
			dt.hour, dt.minute, dt.second);
	}
	else if (format == YMONDHMS2)
	{
		correct_60_problem(&dt, "%4.1f");
		snprintf(str, len, "%04d%s%02d %02d:%02d:%04.1f", 
			dt.year, timeMonthName(dt.month), dt.day,
			dt.hour, dt.minute, dt.second);
	}
	else if (format == GSE20) /* GSE2.0 format yyyy/mm/dd hh:mm:ss.s */
	{
		correct_60_problem(&dt, "%4.1f");
		snprintf(str, len, "%04d/%02d/%02d %02d:%02d:%04.1f", 
			dt.year, dt.month, dt.day,
			dt.hour, dt.minute, dt.second);
	}
	else if (format == GSE22 ) /* GSE2.0 modified format for CBT bulletin creation yyyy/mm/dd hh:mm:ss.ss */	
	{	
		correct_60_problem(&dt, "%5.2f");
		snprintf(str, len, "%04d/%02d/%02d %02d:%02d:%05.2f", 
			dt.year, dt.month, dt.day,
			dt.hour, dt.minute, dt.second);
	}

	else if (format == GSE21) /* GSE2.1 format yyyy/mm/dd hh:mm:ss.sss */
	{
		correct_60_problem(&dt, "%6.3f");
		snprintf(str, len, "%04d/%02d/%02d %02d:%02d:%06.3f", 
			dt.year, dt.month, dt.day,
			dt.hour, dt.minute, dt.second);
	}
	
	
	/* hh:mm:ss.sss
	   For use in forming 'gse formatted' times
	 */
	else if (format == MONTH_HOURS)
	{
		correct_60_problem(&dt, "%6.3f");
		snprintf(str, len, "%03d:%02d:%06.3f",
			hoursInMonth(&dt), dt.minute, dt.second);
	}
	else if (format == YMD)
	{
		snprintf(str, len, "%04d-%02d-%02d", dt.year, dt.month, dt.day);
	}
        /* ddd hh:mm:ss  for use in sta_status reports - old*/
        else if (format == DHMS)
        {
		correct_60_problem(&dt, "%02.0f");
		snprintf(str, len, "%03d %02d:%02d:%02.0f", timeDOY(&dt)-1,
			dt.hour, dt.minute, dt.second);
        }
        /* hhh:mm:ss  for use in chan_status reports  */
        else if (format == MONTH_HOURS2)
        {
		correct_60_problem(&dt, "%02.0f");
		snprintf(str, len, "%03d:%02d:%02.0f",
			hoursInMonth(&dt), dt.minute, dt.second);
        }

        /* dddd hh:mm:ss  for use in sta_status reports,
		periods longer than 1 month
	 */
        else if (format == EPOCH_DAYS)
        {
		correct_60_problem(&dt, "%02.0f");
		snprintf(str, len, "%04d %02d:%02d:%02.0f",
			timeEpochDays(&dt), dt.hour, dt.minute, dt.second);
        }
        /* ddd hh:mm:ss.s for use in sta_status reports,
		periods longer than 1 month
	 */
        else if (format == EPOCH_DAYS2)
        {
		correct_60_problem(&dt, "%04.1f");
		snprintf(str, len, "%03d %02d:%02d:%04.1f",
			timeEpochDays(&dt), dt.hour, dt.minute, dt.second);   
        }
	/* hhh:mm:ss.s
	   For use in chan_status reports
	*/
	else if (format == MONTH_HOURS3)
	{
		correct_60_problem(&dt, "%04.1f");
		snprintf(str, len, "%03d:%02d:%04.1f",
			hoursInMonth(&dt), dt.minute, dt.second);
	}
        /* hh:mm:ss  for use in wave_mission report  */
        else if (format == HHMMSS)
        {
		correct_60_problem(&dt, "%02.0f");
		snprintf(str, len, "%02d:%02d:%02.0f",
			hoursInMonth(&dt), dt.minute, dt.second);
        }

	return str;
}

static void
correct_60_problem(DateTime *dt, const char *sec_format)
{
	char	char_sec[16];
	double  sec;

	/* check if the print format rounds the seconds up to 60.
	 */
	/* format the seconds as requested */
	snprintf(char_sec, 16, sec_format, dt->second);

	/* convert the string into a double */
	sec = atof(char_sec);

	/* see if the seconds will come out to be >= 60.0 */
	if (sec >= 60.)
	{
		/* round epoch up to the next whole second. */
		double epoch = timeDateToEpoch(dt);
		epoch = floor(epoch+.5);
		/* recalculate date */
		timeEpochToDate(epoch, dt);
	}
}

static int
hoursInMonth(DateTime *dt )
{
	return dt->hour + (dt->day-1)*24;
}

/**
 *  Parse a string using the input TimeFormat code. Returns NULL_TIME if the
 *  string cannot be parsed. The TimeFormat codes are:
 * <pre>
 *
 *   code               format                           args
 *
 * YMONDHMS     "%04d%s%02d %02d:%02d:%06.3f"      yr,mname,day,hr,min,sec
 * YMONDHMS2    "%04d%s%02d %02d:%02d:%04.1f"      yr,mname,day,hr,min,sec
 * YMONDHMS3    "%04d%s%02d %02d:%02d:%02.0f"      yr,mname,day,hr,min,sec
 * YMOND        "%04d%s%02d"                       yr,mname,day
 * HMS          "%02d:%02d:%06.3f"                 hr,min,sec
 * HMS2         "%02d:%02d:%04.1f"                 hr,min,sec
 * HMS3         "%02d:%02d:%02.0f"                 hr,min,sec
 * GSE20        "%04d/%02d/%02d %02d:%02d:%04.1f"  yr,mon,day,hr,min,sec
 * GSE21        "%04d/%02d/%02d %02d:%02d:%06.3f"  yr,mon,day,hr,min,sec
 * MONTH_HOURS  "%03d:%02d:%06.3f"                 epoch_hours,min,sec
 * MONTH_HOURS2 "%03d:%02d:%02.0f"                 epoch_hours,min,sec
 * MONTH_HOURS3 "%03d:%02d:%04.1"		   epoch_hours,min,sec
 * YMD          "%04d-%02d-%02d"                   yr,mon,day
 * DHMS         "%03d %02d:%02d:%02.0f"            day-1,hr,min,sec
 * EPOCH_DAYS   "%04d %02d:%02d:%02.0f"            epoch_days,hr,min,sec
 * EPOCH_DAYS2  "%03d %02d:%02d:%04.1f"            epoch_days,hr,min,sec
 * </pre>
 * @param str A string date-time representation to be parsed.
 * @param format A TimeFormat code.
 * @return The epochal time. Returns TIME_NULL if the string cannot be parsed.
 */
double
timeStringToEpoch(const char *str, enum TimeFormat format)
{

	static const char *month_name[] = {
	 "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
	};
	double epoch;
	DateTime dt;
	char buf[10], *endptr;
	int i, n;

	if(str == NULL) return NULL_TIME;
	n = (int)strlen(str);
	    
	/* check for epochal time
	 */
	if(format != YMOND)
	{
	    epoch = strtod(str, &endptr);
	    if(*endptr == '\0') {
		return epoch;
	    }
	}

	if(format == YMONDHMS || format == YMONDHMS2 || format == YMONDHMS3)
	{
	    /* sprintf(str, "%04d%s%02d %02d:%02d:%06.3f", 
	     *  year, month_name, day, hour, minute, second);
	     *		or
	     * sprintf(str, "%04d%s%02d %02d:%02d:%04.1f", 
	     *  year, month_name, day, hour, minute, second);
	     *		or
	     * sprintf(str, "%04d%s%02d %02d:%02d:%02.0f", 
	     *  year, month_name, day, hour, minute, second);
	     */
	    if(n < 17) return NULL_TIME;
	    strncpy(buf, str, 4);
	    buf[4] = '\0';
	    if(parseInt(buf, &dt.year)) return NULL_TIME;

	    strncpy(buf, str+4, 3);
	    buf[3] = '\0';
	    for(i = 0; i < 12 && strcasecmp(buf, month_name[i]); i++);
	    if(i == 12) return NULL_TIME;
	    dt.month = i+1;

	    strncpy(buf, str+7, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.day)) return NULL_TIME;

	    strncpy(buf, str+10, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.hour)) return NULL_TIME;

	    strncpy(buf, str+13, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.minute)) return NULL_TIME;

	    dt.second = strtod(str+16, &endptr);
	    if(*endptr != '\0') return NULL_TIME;
	}
	else if(format == YMOND)
	{
	    /* sprintf(str, "%04d%s%02d", year, month_name, day);
	     */
	    if(n != 9) return NULL_TIME;
	    strncpy(buf, str, 4);
	    buf[4] = '\0';
	    if(parseInt(buf, &dt.year)) return NULL_TIME;

	    strncpy(buf, str+4, 3);
	    buf[3] = '\0';
	    for(i = 0; i < 12 && strcasecmp(buf, month_name[i]); i++);
	    if(i == 12) return NULL_TIME;
	    dt.month = i+1;

	    strncpy(buf, str+7, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.day)) return NULL_TIME;

	    dt.hour = 0;
	    dt.minute = 0;
	    dt.second = 0.;
	}
	else if(format == HMS || format == HMS2 || format == HMS3)
	{
	    /* sprintf(str, "%02d:%02d:%06.3f", hour, minute, second);
	     *		or
	     * sprintf(str, "%02d:%02d:%04.1f", hour, minute, second);
	     *		or
	     * sprintf(str, "%02d:%02d:%02.0f", hour, minute, second);
	     */
	    if(n < 7) return NULL_TIME;
	    strncpy(buf, str, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.hour)) return NULL_TIME;

	    strncpy(buf, str+3, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.minute)) return NULL_TIME;

	    dt.second = strtod(str+6, &endptr);
	    if(*endptr != '\0') return NULL_TIME;

	    dt.year = 1970;
	    dt.month = 1;
	    dt.day = 1;
	}
	else if(format == GSE20 || format == GSE21 || format == GSE22 )
	{
	    /* GSE2.0 format yyyy/mm/dd hh:mm:ss.s 
	     * sprintf(str, "%04d/%02d/%02d %02d:%02d:%04.1f", 
	     *  year, month, day, hour, minute, second);
	     *  	or
	     * GSE2.1 format yyyy/mm/dd hh:mm:ss.sss 
	     * sprintf(str, "%04d/%02d/%02d %02d:%02d:%06.3f", 
	     *  year, month, day, hour, minute, second);
	     */
	    if(n < 19) return NULL_TIME;
	    strncpy(buf, str, 4);
	    buf[4] = '\0';
	    if(parseInt(buf, &dt.year)) return NULL_TIME;

	    strncpy(buf, str+5, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.month)) return NULL_TIME;

	    strncpy(buf, str+8, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.day)) return NULL_TIME;

	    strncpy(buf, str+11, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.hour)) return NULL_TIME;

	    strncpy(buf, str+14, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.minute)) return NULL_TIME;

	    dt.second = strtod(str+17, &endptr);
	    if(*endptr != '\0') return NULL_TIME;
	}
	else if (format == YMD)	
	{
	    /* sprintf(str, "%04d-%02d-%02d", year, month, day);
	     */
	    if(n != 10) return NULL_TIME;
	    strncpy(buf, str, 4);
	    buf[4] = '\0';
	    if(parseInt(buf, &dt.year)) return NULL_TIME;

	    strncpy(buf, str+5, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.month)) return NULL_TIME;

	    if(parseInt(str+8, &dt.day)) return NULL_TIME;

	    dt.hour = 0;
	    dt.minute = 0;
	    dt.second = 0.;
	}
        else if (format == DHMS)
        {
	    /* sprintf(str, "%03d %02d:%02d:%02.0f", timeDOY(&dt)-1,
	     *		dt.hour, dt.minute, dt.second);
	     */
	    int doy;
	    for(i = 0; str[i] != '\0' && str[i] != ' '; i++);
	    if(str[i] == '\0') return NULL_TIME;
	    strncpy(buf, str, i);
	    buf[i] = '\0';
	    if(parseInt(buf, &doy)) return NULL_TIME;
	    doy++;
	    strncpy(buf, str+i+1, 10);
	    if((int)strlen(buf) != 8) return NULL_TIME;

	    buf[2] = '\0';
	    if(parseInt(buf, &dt.hour)) return NULL_TIME;

	    strncpy(buf, str+i+4, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.minute)) return NULL_TIME;

	    dt.second = strtod(str+i+7, &endptr);
	    if(*endptr != '\0') return NULL_TIME;

	    dt.year = 1970;
	    timeMonthDay(dt.year, doy, &dt.month, &dt.day);
	}
	else if (format == MONTH_HOURS || format == MONTH_HOURS2)
	{
	    /*
	     *	sprintf(str, "%03d:%02d:%06.3f", 
	     *		hoursInMonth(&dt), dt.minute, dt.second);
	     */
	    int hours;
	    if(n < 9) return NULL_TIME;
	    strncpy(buf, str, 3);
	    buf[3] = '\0';
	    if(parseInt(buf, &hours)) return NULL_TIME;

	    strncpy(buf, str+4, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.minute)) return NULL_TIME;

	    dt.second = strtod(str+7, &endptr);
	    if(*endptr != '\0') return NULL_TIME;

	    dt.day = hours/24 + 1;
	    dt.hour = hours - (dt.day-1)*24;
	    dt.year = 1970;
	    dt.month = 1;
	}
        else if (format == EPOCH_DAYS)
        {
	    /*
	     *	sprintf(str, "%04d %02d:%02d:%02.0f",
	     *		timeEpochDays(&dt), dt.hour, dt.minute, dt.second);   
	     */
	    int days, doy;
	    for(i = 0; str[i] != '\0' && !isspace((int)str[i]); i++);
	    if(i > 8 || n-i != 9) return NULL_TIME;
	    strncpy(buf, str, i);
	    buf[i] = '\0';
	    if(parseInt(buf, &days)) return NULL_TIME;

	    strncpy(buf, str+i+1, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.hour)) return NULL_TIME;

	    strncpy(buf, str+i+4, 2);
	    buf[2] = '\0';
	    if(parseInt(buf, &dt.minute)) return NULL_TIME;

	    dt.second = strtod(str+i+7, &endptr);
	    if(*endptr != '\0') return NULL_TIME;

	    dt.year = 1970 + days/365;
	    doy = days - (dt.year-1970)*365 + 1;
	    timeMonthDay(dt.year, doy, &dt.month, &dt.day);
        }
	else {
	    return NULL_TIME;
	}

	return timeDateToEpoch(&dt);
}

static int
parseInt(const char *str, int *n)
{
	int i;
	char *endptr;

	for(i = 0; str[i] != '\0' && str[i] == ' '; i++);
	for(; str[i] != '\0' && str[i] == '0'; i++);

	if(str[i] == '\0') {
	    *n = 0;
	}
	else {
	    *n = (int)strtol(str+i, &endptr, 0);
	    if(*endptr != '\0') return -1;
	}
	return 0;
}

/**
 * Parse a character string for date and time values. This routine is called
 * recursively to parse the input string. The parse type can be:
 * <pre>
 *
 * "f"                          float value
 * "lf"                         double value
 * "d"                          int value
 * "ld"                         long value
 * "s"                          string value
 * "S"                          string value skip blanks
 * "yymmdd_hhmmss"              date time
 * "yyyymmdd_hhmmssss"          date time
 * "yyyymmdd_hhmmssssss"        date time
 * "yyyymmdd_hhmmssssssss"      date time
 * "hhmmss"                     time
 * @param line The string to parse between s_pos and e_pos.
 * @param s_pos The starting index of line.
 * @param e_pos The ending index of line.
 * @param val A pointer to a memory location of length size bytes.
 * @param size The length (number of bytes) of val. For strings, size must be \
 * 	>= e_pos - s_pos + 2.
 * @param type The parse type.
 */
void
timeParseLine(const char *line, int s_pos, int e_pos, void *val,
		int size, const char *type)
{
	int i;
	DateTime dt;
	int buf_len=256;
	char cbuf[buf_len];

	/*
	  For non character fields you should check that the items are numeric
	  and do not exceed the maximum size that the number can hold, e.g., 
	  199607150013 does not fit into long
	*/

	if ((int) strlen(line) < s_pos)
	{
	    if (!strcmp(type, "f"))
	    {
		if(size != sizeof(float)) {
		    logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		    return;
		}
		*(float *)val = -1.0;
	    }
	    else if ( !strcmp(type, "lf") ||
			!strcmp(type, "yyyymmdd_hhmmssss") ||
			!strcmp(type, "yyyymmdd_hhmmssssss") ||
			!strcmp(type, "yyyymmdd_hhmmssssssss") ||
			!strcmp(type, "hhmmssssss"))
	    {
		if(size != sizeof(double)) {
		    logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		    return;
		}
		*(double *)val = 0.0;
	    }
	    else if (!strcmp(type, "d"))
	    {
		if(size != sizeof(int)) {
		    logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		    return;
		}
		*(int *)val = -1;
	    }
	    else if (!strcmp(type, "ld"))
	    {
		if(size != sizeof(long)) {
		    logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		    return;
		}
		*(long *)val = -1;
	    }
	    else if (!strcmp(type, "s"))
	    {
		strncpy((char *) val, "-", size);
	    }
	    return;
	}
	if(s_pos >= buf_len || e_pos >= buf_len || e_pos < s_pos) {
	    logErrorMsg(LOG_WARNING, "timeParseLine: invalid s_pos, e_pos");
	    return;
	}

	strncpy(cbuf, "                                   ", buf_len);
	strncpy(cbuf, line+s_pos, e_pos-s_pos+1);

	for (i=0; i<e_pos-s_pos+1; i++)
		if (cbuf[i] == '\n')
	{
	    cbuf[i] = '\0';
	}

	if (!strcmp(type, "f"))
	{
	    if(size != sizeof(float)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(float *)val = (float) atof(cbuf);
	}
	else if (!strcmp(type, "lf"))
	{
	    if(size != sizeof(double)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(double *)val = (double) atof(cbuf);
	}
	else if (!strcmp(type, "d"))
	{
	    if(size != sizeof(int)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(int *)val = (int) atoi(cbuf);
	}
	else if (!strcmp(type, "ld"))
	{
	    if(size != sizeof(long)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(long *)val = (long) atoi(cbuf);
	}
	else if (!strcmp(type, "s"))
	{
	    stringcpy((char *)val, cbuf, size);
	}
	else if (!strcmp(type, "S"))
	{
	    int i1;
	    /* skip initial and trailing white space */
	    for(i1 = 0; cbuf[i1] != '\0' && isspace((int)cbuf[i1]); i1++);

	    for(i = i1; cbuf[i] != '\0' && !isspace((int)cbuf[i]); i++);
	    cbuf[i] = '\0';
	    stringcpy((char *)val, cbuf+i1, size-i1);
	}
	/* 1995/10/25 00:02:39.1 */
	/* 012345678901234567890 */
	else if (!strcmp(type, "yyyymmdd_hhmmssss"))
	{
	    timeParseLine(cbuf, 0,  4, &dt.year, sizeof(dt.year), "d");
	    timeParseLine(cbuf, 5,  7, &dt.month, sizeof(dt.month), "d");
	    timeParseLine(cbuf, 8, 10, &dt.day, sizeof(dt.day), "d");

	    timeParseLine(cbuf, 11, 13, &dt.hour, sizeof(dt.hour), "d");
	    timeParseLine(cbuf, 14, 16, &dt.minute, sizeof(dt.minute), "d");
	    timeParseLine(cbuf, 17, 21, &dt.second, sizeof(dt.second), "lf");

	    if(size < (signed int)sizeof(double)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(double *)val = timeDateToEpoch(&dt);
	}
	/* 1995/10/25 00:02:39.192 */
	/* 01234567890123456789012 */
	else if (!strcmp(type, "yyyymmdd_hhmmssssss"))
	{
	    timeParseLine(cbuf,  0,  4, &dt.year, sizeof(dt.year), "d");
	    timeParseLine(cbuf,  5,  7, &dt.month, sizeof(dt.month), "d");
	    timeParseLine(cbuf,  8, 10, &dt.day, sizeof(dt.day), "d");

	    timeParseLine(cbuf, 11, 13, &dt.hour, sizeof(dt.hour), "d");
	    timeParseLine(cbuf, 14, 16, &dt.minute, sizeof(dt.minute), "d");
	    timeParseLine(cbuf, 17, 23, &dt.second, sizeof(dt.second), "lf");

	    if(size < (signed int)sizeof(double)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(double *)val = timeDateToEpoch(&dt);
	}
	/* 1995/10/25 00:02:39.19201 */
	/* 0123456789012345678901234 */
	else if (!strcmp(type, "yyyymmdd_hhmmssssssss"))
	{
	    timeParseLine(cbuf,  0,  4, &dt.year, sizeof(dt.year), "d");
	    timeParseLine(cbuf,  5,  7, &dt.month, sizeof(dt.month), "d");
	    timeParseLine(cbuf,  8, 10, &dt.day, sizeof(dt.day), "d");

	    timeParseLine(cbuf, 11, 13, &dt.hour, sizeof(dt.hour), "d");
	    timeParseLine(cbuf, 14, 16, &dt.minute, sizeof(dt.minute), "d");
	    timeParseLine(cbuf, 17, 25, &dt.second, sizeof(dt.second), "lf");

	    if(size < (signed int)sizeof(double)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(double *)val = timeDateToEpoch(&dt);
	}
	/* 95/10/25 00:02:39 */
	/* 012345678901234567890 */
	else if (!strcmp(type, "yymmdd_hhmmss"))
	{
	    timeParseLine(cbuf,   0,  2, &dt.year, sizeof(dt.year), "d");
	    if (dt.year < 50) {
		dt.year += 2000;
	    }
	    else {
		dt.year += 1900;
	    }
	    timeParseLine(cbuf,  3,  5, &dt.month, sizeof(dt.month), "d");
	    timeParseLine(cbuf,  6,  8, &dt.day, sizeof(dt.day), "d");

	    timeParseLine(cbuf,  9, 11, &dt.hour, sizeof(dt.hour), "d");
	    timeParseLine(cbuf, 12, 14, &dt.minute, sizeof(dt.minute), "d");
	    timeParseLine(cbuf, 15, 17, &dt.second, sizeof(dt.second), "lf");

	    if(size < (signed int)sizeof(double)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(double *)val = timeDateToEpoch(&dt);
	}
	/* 00:32:36.750 */
	/* 012345678901 */
	else if (!strcmp(type, "hhmmss"))
	{
	    dt.year = 1970;
	    dt.month = 1;
	    dt.day = 1;

	    timeParseLine(cbuf, 0,  1, &dt.hour, sizeof(dt.hour), "d");
	    timeParseLine(cbuf, 3,  4, &dt.minute, sizeof(dt.minute), "d");
	    timeParseLine(cbuf, 6, 11, &dt.second, sizeof(dt.second), "lf");

	    if(size < (signed int)sizeof(double)) {
		logErrorMsg(LOG_ERR, "timeParseLine: invalid size.");
		return;
	    }
	    *(double *)val = timeDateToEpoch(&dt);
	}
}

/**
 * Check time limits. Check that the time values are a valid time period.
 * Returns 1 if time1 and time2 are > NULL_TIME_CHECK and time2 > time1 and
 * fabs(time2 - time1) >= 0.001. Otherwise, returns 0 and prints an error
 * message in msg.
 */
int
timeCheckTimes(double time1, double time2, char *msg, int len)
{
	msg[0] = '\0';

	if (time1 < NULL_TIME_CHECK || time2 < NULL_TIME_CHECK)
		return(1);

	if (fabs(time1 - time2) < 0.001)
	{
	    snprintf(msg, len,
"time1 - time2 diff must be > 0.001 sec\ncurrent difference of fabs(%.3f - %.3f) is %.5f",
			time1, time2, fabs(time1 - time2));
		return(0);
	}
	else if (time1 > time2)
	{
	    snprintf(msg, len,
	"time1 must be > time2\ncurrent values are time1=%.3f time2=%.3f",
			time1, time2);
		return(0);
	}
	return(1);
}

static int
is_space(char c)
{
	return (isspace((int)c) || c == '#');
}
