/*
 *NAME
 *	present time
 *
 *FILE
 *
 *	stdtime_present.c
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
 *
 *	PIDC additions - nolten - 12/1998
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <string.h>

#include "libstdtimeP.h"

double
stdtime_now ( )
{
	double now;

	stdtime_errno = STDTIME_NOERR;
	now = (double)time( 0 );
	return now;
}

epoch_t
stdtime_rnd_epoch (epoch_t epoch, int ndec)
{
	double decimals;

	stdtime_errno = STDTIME_NOERR;

	if (ndec < 0 || ndec > 6)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}
		
	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

	decimals = pow((double)10.0, (double)ndec);
	return floor(decimals * epoch + 0.5) / decimals;
}


epoch_t
stdtime_diff_epoch (epoch_t epoch1, epoch_t epoch2, int ndec,
		    char *diff_a, int diff_len)
{
        epoch_t   epoch_diff;
	char      format[32];
	
	stdtime_errno = STDTIME_NOERR;

        if ((check_epoch (epoch1) != STDTIME_NOERR)
	    || (check_epoch (epoch2) != STDTIME_NOERR))
        {
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
        }

	if (epoch2 < epoch1)
        {
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
        }

	if (ndec < 0 || ndec > 6)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}
		
        if (check_buffer (diff_a) != STDTIME_NOERR)
        {
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
        }

	epoch_diff = epoch2 - epoch1;

	/*
	 * Use stdtime_format to retrieve the epoch time in 
	 * format (days hh:mm:ss[.n+], %K %T.xN)
	 */
	strcpy(format, "%K %T");
	if (ndec > 0 && ndec <= 6)
	{
		strcat(format, ".%1N");
		format[7] = '0' + ndec;
	}

	if (stdtime_format_r (epoch_diff, format, diff_a, diff_len) !=
		STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_DOUBLE_ERR;
	}
        return epoch_diff;
}


epoch_t
stdtime_get_epoch (void)
{
	epoch_t 	d_now;
	int		ret;

	struct timeval	tp;

	stdtime_errno = d_now = STDTIME_NOERR;
	
#ifndef HAVE_GETTIMEOFDAY_1
        ret = gettimeofday (&tp, NULL);
#else
        ret = gettimeofday (&tp);
#endif /* HAVE_GETTIMEOFDAY_1 */
	if (ret == -1)			/* Error. */
	{
		stdtime_errno = STDTIME_INVALID;
		d_now = STDTIME_DOUBLE_ERR;
	}
	else
	{
		d_now = (epoch_t) tp.tv_sec + 1.0e-6 * (epoch_t) tp.tv_usec;
	}

	return d_now;
}

double
stdtime_fnow (void)
{
	double 		d_now;
	int		ret;

	struct timeval	tp;

	stdtime_errno = d_now = STDTIME_NOERR;
	
#ifndef HAVE_GETTIMEOFDAY_1
        ret = gettimeofday (&tp, NULL);
#else
        ret = gettimeofday (&tp);
#endif /* HAVE_GETTIMEOFDAY_1 */
	if (ret == -1)			/* Error. */
	{
		stdtime_errno = STDTIME_INVALID;
		d_now = STDTIME_DOUBLE_ERR;
	}
	else
	{
		d_now = (double) tp.tv_sec + 1.0e-6 * (double) tp.tv_usec;
	}

	return d_now;
}

#ifdef OLD
double
stdtime_fnow_old ( )nn
{
	double 		d_now;

	struct timeb 	now; 	   /* Present time from ftime() */

	ftime (&now);
		/* Save seconds */
	d_now = now.time;
		/* Add milliseconds to value */
	d_now += (double)now.millitm / 1000.0;

	return d_now;
}

#endif

time_t
stdtime_time (void)
{
	stdtime_errno = STDTIME_NOERR;
	return(time(NULL));
}

int
stdtime_gettimeofday (struct timeval *tp)
{
	int ret;

        stdtime_errno = STDTIME_NOERR;
#ifndef HAVE_GETTIMEOFDAY_1
        ret = gettimeofday (tp, NULL);
#else
        ret = gettimeofday (tp);
#endif /* HAVE_GETTIMEOFDAY_1 */

	return ret;
}
