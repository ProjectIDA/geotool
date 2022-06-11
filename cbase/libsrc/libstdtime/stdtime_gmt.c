/*
 *NAME
 *	gmt format time function
 *
 *FILE
 *
 *	stdtime_gmt.c
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

#include "libstdtimeP.h"

double
stdtime_gmttoe (gmt_a)
char	*gmt_a;
{
	stdtime_errno = STDTIME_NOERR;

	if (check_gmt (gmt_a) == STDTIME_NOERR)
		return stdtime_unformat (gmt_a, "%G");
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}
}

char	*
stdtime_etogmt (epoch)
double	epoch;
{
	stdtime_errno = STDTIME_NOERR;

	if (check_epoch (epoch) == STDTIME_NOERR)
		return stdtime_format( epoch, "%G" );
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
}

int
stdtime_etogmt_r (epoch, gmt_a, n)
double	epoch;
char	*gmt_a;
int	n;
{
	stdtime_errno = STDTIME_NOERR;

	if (check_epoch (epoch) == STDTIME_NOERR && gmt_a != (char *)NULL)
		return stdtime_format_r( epoch, "%G", gmt_a, n);
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}
}
