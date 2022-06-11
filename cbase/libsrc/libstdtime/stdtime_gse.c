/*
 *NAME
 *	gse format time function
 *
 *FILE
 *
 *	stdtime_gse.c
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
 *	Tad Glines
 *	SAIC
 *	Patrick AFB, FL
 */

#include <string.h>

#include "libstdtimeP.h"

double
stdtime_gsetoe (gse_a)
char	*gse_a;
{
	stdtime_errno = STDTIME_NOERR;

	if (check_gse (gse_a) == STDTIME_NOERR)
	{
		/* return epoch */
		return stdtime_unformat (gse_a, "%Y/%m/%d %H:%M:%S.%1N");
	}
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}
}


char	*
stdtime_etogse (epoch, ndec, gse_a, gse_len)
epoch_t	epoch;
int     ndec;
char	*gse_a;
int	gse_len;
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

	strcpy(format, "%Y/%m/%d %T");
	if (ndec > 0 && ndec <= 3)
	{
		/*
		 * add decimal format ".%<ndec>N"
		 */
	        len = strlen (format);
		sprintf (format+len, ".%%%1dN", (int) ndec);
	}

	if ( stdtime_format_r(epoch, format, gse_a, gse_len) 
	    != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return gse_a;
}


char	*
stdtime_etogsed (epoch, gse_a, gse_len)
epoch_t	epoch;
char	*gse_a;
int	gse_len;
{
	stdtime_errno = STDTIME_NOERR;

	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}

	if (stdtime_format_r (epoch, "%Y/%m/%d", gse_a,
			      gse_len) != STDTIME_SUCCESS)
	{
	     /* stdtime_format_r() sets stdtime_errno */
	     return STDTIME_CHAR_ERR;
	}

	return gse_a;
}


char	*
stdtime_etogset (epoch, ndec, gse_a, gse_len)
epoch_t	epoch;
int     ndec;
char	*gse_a;
int	gse_len;
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

	strcpy(format, "%T");
	if (ndec > 0 && ndec <= 3)
	{
		/*
		 * add decimal format ".%<ndec>N"
		 */
	        len = strlen (format);
		sprintf (format+len, ".%%%1dN", (int) ndec);
	}

	if ( stdtime_format_r(epoch, format, gse_a, gse_len) 
	    != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return gse_a;
}


int
stdtime_etogse_r (epoch, gse_ap, n)
double	epoch;
char	*gse_ap;
int	n;
{
	stdtime_errno = STDTIME_NOERR;

	if (check_epoch (epoch) == STDTIME_NOERR &&
	    gse_ap != (char *)NULL)
	{
	        /* Get formated string */
		return (stdtime_format_r( epoch, "%Y/%m/%d %H:%M:%S.%1N",
					  gse_ap, n));
	}
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}
}
