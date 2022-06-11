/*
 *NAME
 *	stdtime_localtime is a wrapper function for localtime.
 *	stdtime_local_now gets the current local time and
 *	returns it as a double
 *
 *FILE
 *
 *	stdtime_localtime.c
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
 *	Daria F. Buonassisi
 *	SAIC
 *	San Diego, CA.
 */

#include "config.h"
#include "libstdtimeP.h"

struct tm *
stdtime_localtime (epoch_time)
time_t *epoch_time;
{
        struct tm    *ret_val;

	stdtime_errno = STDTIME_NOERR;

	if (check_epoch_ptr (epoch_time) == STDTIME_NOERR)
        {
                ret_val = localtime (epoch_time);
                if ( ret_val == STDTIME_TM_ERR )
                        stdtime_errno = STDTIME_INVALID;
        }
	else
        {
                ret_val = STDTIME_TM_ERR;
                stdtime_errno = STDTIME_INVALID;
        }

        return ( ret_val );
}

struct tm *
stdtime_localtime_r (epoch_time, tm)
time_t 		*epoch_time;
struct tm	*tm;
{
	struct tm    *ret_val;

	stdtime_errno = STDTIME_NOERR;

	if (check_epoch_ptr (epoch_time) == STDTIME_NOERR &&
	    tm != (struct tm *)NULL)
        {
		ret_val = localtime_r(epoch_time, tm);
                if ( ret_val == STDTIME_TM_ERR )
                        stdtime_errno = STDTIME_INVALID;
        }
	else
        {
		ret_val = STDTIME_TM_ERR;
                stdtime_errno = STDTIME_INVALID;
        }

	return ret_val;
}
