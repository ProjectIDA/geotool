/*
 * NAME
 *	Continuous Data System date format functions 
 *
 * FILE
 *
 *	stdtime_cd.c
 *
 * DESCRIPTION
 *      stdtime_etocd() converts an epoch time to a CD 1.1 format string
 *      (yyyyjjj hh:mm:ss[.n+]).  stdtime_htoe() can be used to convert
 *      a CD 1.1 date format string to an epoch time.
 *
 */

#include <string.h>

#include "libstdtimeP.h"

char *
stdtime_etocd (epoch, ndec, cd_a, cd_len)
epoch_t	epoch;
int     ndec;
char	*cd_a;
int	cd_len;
{
        int  len;
	char format[32];

	stdtime_errno = STDTIME_NOERR;

	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}

	if (ndec < 0 || ndec > 3)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}

	strcpy(format, "%J %T");
	if (ndec > 0 && ndec <= 3)
	{
		/*
		 * add decimal format ".%<ndec>N"
		 */
	        len = strlen (format);
		sprintf (format+len, ".%%%1dN", (int) ndec);
	}

	if ( stdtime_format_r(epoch, format, cd_a, cd_len) 
	    != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return cd_a;
}


