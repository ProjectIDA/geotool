/*
 * NAME
 *	Ordinal (incorrectly called Julian) format time function
 *
 * FILE
 *
 *	stdtime_check_value.c
 *
 * SYNOPSIS
 *
 * DESCRIPTION
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
 *   TMH 20-Apr-2006:
 *    - Replaced lots of duplicate checks for NULL char * with a generic
 *      function stdtime_IsNullPointer( char *Ptr ), which returns
 *      either STDTIME_NOERR or STDTIME_INVALID.
 *
 * SEE ALSO
 *
 * AUTHOR
 *	Jim Wang
 *	SAIC
 *	San Diego, CA
 */

#include "libstdtimeP.h"

static int stdtime_IsNullPointer(char *Ptr);

/* Valid range for ordinal dates. */
#define DOY_MIN		001
#define DOY_MAX		365
#define LEAP_DOY_MAX	366
#define YYYYDOY_MIN     1901347
#define YYYYDOY_MAX     2038019

/*
 * check_C ()
 *
 * Check the C format string for validity.
 * The C format mimics the date () output
 */
int
check_C ( char *str )
{
	return( stdtime_IsNullPointer( str ) );
}

/*
 * check_epoch ()
 *
 * Check an epoch time for validity.
 */
int
check_epoch ( epoch_t epoch )
{
	if (epoch > STDTIME_EPOCH_MIN && epoch < STDTIME_EPOCH_MAX)
		return STDTIME_NOERR;
	else
		return STDTIME_INVALID;
}

/*
 * check_epoch_ptr ()
 *
 * Check an epoch time pointer for validity.
 * epoch is a time_t pointer.
 */
int
check_epoch_ptr ( time_t *epoch )
{
	if (epoch == (time_t) NULL)
		return STDTIME_INVALID;
	else if (*epoch > STDTIME_EPOCH_MIN && *epoch < STDTIME_EPOCH_MAX)
		return STDTIME_NOERR;
	else
		return STDTIME_INVALID;
}

/*
 * check_buffer ()
 *
 * Check a buffer pointer for validity
 */
int
check_buffer ( char *buffer )
{
	return( stdtime_IsNullPointer( buffer ) );
}

/*
 * check_fmt ()
 *
 * Check a format time string for validity.
 */
int
check_fmt ( char *format )
{
	return( stdtime_IsNullPointer( format ) );
}

/*
 * check_gmt ()
 *
 * Check a GMT time string for validity.
 */
int
check_gmt ( char *gmt )
{
	return( stdtime_IsNullPointer( gmt ) );
}

/*
 * check_gse ()
 *
 * Check a GSE time string for validity.
 */
int
check_gse ( char *gse )
{
	return( stdtime_IsNullPointer( gse ) );
}

/*
 * check_human ()
 *
 * Check a human time string for validity.
 */
int
check_human ( char *human )
{
	return( stdtime_IsNullPointer( human ) );
}

/*
 * check_lddate ()
 *
 * Check a lddate string for validity.
 */
int
check_lddate ( char *lddate )
{
	return( stdtime_IsNullPointer( lddate ) );
}

/*
 * check_sh ()
 *
 * Check a short-human time string for validity.
 */
int
check_sh ( char *sh )
{
	return( stdtime_IsNullPointer( sh ) );
}

/*
 * check_std_time_st ()
 *
 * Check a std_time_st structure pointer for validity.
 */
int
check_std_time_st ( std_time_st *time_st )
{
	if (time_st != (std_time_st *) NULL)
		return STDTIME_NOERR;
	else
		return STDTIME_INVALID;
}

/*
 * check_tm ()
 *
 * Check a tm structure poniter for validity.
 */
int
check_tm (struct  tm *tm_p )
{
	if (tm_p != (struct tm *) NULL)
		return STDTIME_NOERR;
	else
		return STDTIME_INVALID;
}

/*
 * check_yyyydoy ()
 *
 * Check an ordinal/julian date for validity.
 */
int
check_yyyydoy ( int yyyydoy )
{
	int doy;
	int year;
	int doy_max;
	
	if (yyyydoy < YYYYDOY_MIN || yyyydoy > YYYYDOY_MAX)
	{
	     return STDTIME_INVALID;
	}

	doy = yyyydoy % 1000;
	year = (yyyydoy - doy) / 1000;
	
	if (stdtime_isleap(year))
	     doy_max = LEAP_DOY_MAX;
	else
	     doy_max = DOY_MAX;

	if ( doy < DOY_MIN || doy > doy_max )
	{
	        return STDTIME_INVALID;
	}

	return STDTIME_NOERR;
}

/*
 * check_year ()
 *
 * Check a year for validity.
 */
int
check_year ( int year )
{
	if (year >= STDTIME_YEAR_MIN && year <= STDTIME_YEAR_MAX)
		return STDTIME_NOERR;
	else
		return STDTIME_INVALID;
}


/*
 * check_year_mon_day ()
 *
 * Check a year, mon, day for validity.
 */
int
check_year_mon_day ( int year, int mon, int day )
{
	int dim;

	stdtime_errno = STDTIME_NOERR;

	if (check_year(year) == STDTIME_INVALID)
	{
		return STDTIME_INVALID;
	}

	if ((mon < 1) || (mon > 12))
	{
		return STDTIME_INVALID;
	}
	
	dim = stdtime_dim[mon-1];
        if (stdtime_isleap(year) && (mon == 2))
	     dim++;
	if ((day < 1) || (day > dim))
	{
		return STDTIME_INVALID;
	}

	return STDTIME_NOERR;
}


/*
 * check_hour, min, sec ()
 *
 * Check a hour, min, second for validity.
 */
int
check_hour_min_sec ( int hour, int min, epoch_t sec )
{
	stdtime_errno = STDTIME_NOERR;

	if ((hour < 0) || (hour > 23))
	{
		return STDTIME_INVALID;
	}
	
	if ((min < 0) || (min > 59))
	{
		return STDTIME_INVALID;
	}
	
	if ((sec < (epoch_t) 0) || (sec > (epoch_t) 61))
	{
		return STDTIME_INVALID;
	}
	
	return STDTIME_NOERR;
}

/*
 * Generic Check function for NULL character pointer
 * TMH - 19-Apr-2006 : Code factorisation
 */
static int
stdtime_IsNullPointer( char *Ptr )
{
        if ( Ptr != (char *) NULL)
        {
                return STDTIME_NOERR;
        }

        return STDTIME_INVALID;
}

