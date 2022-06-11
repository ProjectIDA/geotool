/*
 *NAME
 *	wrapper for C time function asctime
 *
 *FILE
 *
 *	stdtime_asctime.c
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

char *
stdtime_asctime( tm )
struct tm *tm;
{
	char	*ret_val;

	stdtime_errno = STDTIME_NOERR;
	
	if (check_tm (tm) == STDTIME_NOERR)
	{
		ret_val = asctime (tm);
		if (ret_val == (char *) NULL)
			stdtime_errno = STDTIME_INVALID;
	}
	else
	{
		ret_val = STDTIME_CHAR_ERR;
		stdtime_errno = STDTIME_INVALID;
	}

	return ( ret_val );
}

char *
stdtime_asctime_r( tm, buf, buflen )
struct tm *tm;
char	*buf;
int	buflen;
{
	char	*ret_val;

	stdtime_errno = STDTIME_NOERR;
	
	if (check_tm (tm) == STDTIME_NOERR &&
	    buf != (char *)NULL && buflen > 0)
	{
#ifdef HAVE_ASCTIME_R_3
                ret_val = asctime_r (tm, buf, buflen);
#else
                ret_val = asctime_r (tm, buf);
#endif
		if (ret_val == (char *) NULL)
			stdtime_errno = STDTIME_INVALID;
	}
	else
	{
		ret_val = STDTIME_CHAR_ERR;
		stdtime_errno = STDTIME_INVALID;
	}

	return ( ret_val );
}
