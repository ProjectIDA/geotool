/*
 *NAME
 *	time_struct
 *
 *FILE
 *
 *	stdtime_time_struct.c
 *
 *
 *SYNOPSIS
 *
 *DESCRIPTION
 *
 *DIAGNOSTICS
 *
 *FILES
 *
 *NOTES
 *
 *SEE ALSO
 *
 *AUTHOR
 *	Joseph Wehlen 
 *	SAIC
 *	Patrick AFB, FL
 */

#include "config.h"
#include <math.h>
#include "libstdtimeP.h"

/* for TM_YEAR_BASE */
/* on Linux, this needs to be defined=1900.
 * Solaris does this in the tzfile.h
 */

#ifdef HAVE_TZFILE_H
#include <tzfile.h>    
#else
#define TM_YEAR_BASE 1900
#endif /* HAVE_TZFILE_H */

std_time_st
stdtime_build_struct( epoch_time )
double	epoch_time;
{
	std_time_st	tmp;
	time_t		time;

	stdtime_errno = STDTIME_NOERR;

	time = (time_t)epoch_time;

	tmp.tm_s = *gmtime( &time );

	tmp.milli = rint ((epoch_time - (int) epoch_time) * 1000);

	return tmp;
}	

double
stdtime_convert_struct( time_s )
std_time_st	time_s;
{
	double	tmp;

	stdtime_errno = STDTIME_NOERR;

	tmp = (double) stdtime_timegm (&time_s.tm_s);
	if (stdtime_errno != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}
	
	tmp = tmp + ((double)time_s.milli / 1000.0);

	return tmp;
}	


time_t
stdtime_timegm( tm )
struct tm *tm;
{
	epoch_t time;
	int     year;
	int     mon;
	int     doy;
	int     dim;
	int     i;

	stdtime_errno = STDTIME_NOERR;

	if (check_tm(tm) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

	year = TM_YEAR_BASE + tm->tm_year;
	mon  = tm->tm_mon+1;
	
	if (check_year_mon_day(year, mon, tm->tm_mday) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

        doy = 0;
        for( i = 0 ; i < mon-1; i++ ){
                dim = stdtime_dim[i];
                if( i == 1 && stdtime_isleap(year) ) dim++;
                doy += dim;
        }
        doy += tm->tm_mday;

	time = stdtime_jtoe(1000 * year + doy);

	if (stdtime_errno != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

        time += tm->tm_hour*3600 + tm->tm_min*60 + tm->tm_sec;

	if (check_epoch (time) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

	return( time );
}


#if 0				/* Only on Solaris. */
/*
	 * The routine timegm is a routine that converts time from
	 * the struct tm format to a time_t (or epoch).  This routine
	 * was supported under SunOS and dropped in Solaris.
	 *
	 * So this is my attempt to rewrite this routine.  It seems 
	 * simple enough.
	 *
	 * I am using the routine htoe() found in the CSS library libtime
	 * to convert to an epoch.  All I need to do is convert Unix
	 * struct tm to CSS date_time which is pretty close. 
	 *
	 * Jon Jump/SAIC
	 */

time_t
timegm( tm )
struct tm *tm;
{
	time_t time;
	int isdst;

	isdst = tm->tm_isdst;
	time = mktime(tm);

	/*
	 * mktime() returns -1 if an error occurred.
	 * If an error occurred, return -1 w/o correcting it
	 * for timezone.
	 */
	if (time == (time_t) -1)
	{
		stdtime_errno = STDTIME_INVALID;
	}
	else
	{
		if( isdst == 0 )
			time -= timezone;
		else
			time -= altzone;
	}

	return( time );
}
#endif

