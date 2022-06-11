/*
 *NAME
 *	wrapper for C time function ctime
 *
 *FILE
 *
 *	stdtime_ctime.c
 *
 *SYNOPSIS
 *
 *DESCRIPTION
 *
 *	The function stdtime_ctime() takes an epoch time and converts it to
 *	a character format following the C function ctime()
 *	format, which is %a %b %c %T %Y (Thu Aug  5  09:31:51 1998)
 *
 *	The function stdtime_strtime() gets the current time and
 *		calls the stdtime_strdtime() function.
 *
 *	The function stdtime_strdtime() converts the current time to
 *		the format %a %b %e %T %Y (Thu Aug  5  09:21:31 1998)
 *		via libstdtime function stdtime_ctime().  However,
 *		stdtime_ctime() includes a lagging \n, which
 *		strdtime_strtime() removes, replacing
 *		it with the \0 null terminating character.
 *DIAGNOSTICS
 *
 *FILES
 *
 *NOTES
 *
 *SEE ALSO
 *
 *AUTHOR
 *	Doug A. Brumbaugh
 *	SAIC
 *	San Diego, CA
 */


#include "config.h"
#include "libstdtimeP.h"

char *
stdtime_ctime (epoch_time )
time_t *epoch_time;
{
	char *ret_val;

	stdtime_errno = STDTIME_NOERR;

	if (check_epoch_ptr (epoch_time) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		ret_val = STDTIME_CHAR_ERR;
	}
	else
	{
		ret_val = ctime (epoch_time);
	} 

	return ( ret_val );
}

char *
stdtime_ctime_r (epoch_time, buf, buflen)
time_t *epoch_time;
char	*buf;
int	buflen;
{
	char *ret_val;

	stdtime_errno = STDTIME_NOERR;

	if (check_epoch_ptr (epoch_time) != STDTIME_NOERR ||
	    buf == (char *)NULL || buflen <= 0)
	{
		stdtime_errno = STDTIME_INVALID;
		ret_val = STDTIME_CHAR_ERR;
	}
	else
	{
#ifdef HAVE_CTIME_R_3
                ret_val = ctime_r (epoch_time, buf, buflen);
#else
                ret_val = ctime_r (epoch_time, buf);
#endif
	} 

	return ( ret_val );
}

char *
stdtime_strdtime (thetime)
double	thetime;
{
	time_t		now;
	char		*buf;
	extern char	*stdtime_ctime ();

	stdtime_errno = STDTIME_NOERR;
	
	if (check_epoch (thetime) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
	
	now = (time_t) thetime;
	buf = stdtime_ctime (&now);
	buf[24] = '\0';
	return buf;
}

char *
stdtime_strtime ()
{
	time_t	now;			/* current time */

	now = stdtime_now ();
	return stdtime_strdtime ((double) now);
}
