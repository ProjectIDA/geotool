/*
 *NAME
 *	String format
 *
 *FILE
 *
 *	stdtime_string.c
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

#include <string.h>

#include "libstdtimeP.h"

#define	STR_BUF_LEN	32

double
stdtime_dstrtoe (human_a)
char	*human_a;
{
	double	epoch;

	stdtime_errno = STDTIME_NOERR;
	
	if (check_human (human_a) == STDTIME_NOERR)
	{
		epoch = stdtime_unformat (human_a, "%O");
	}
	else
	{
		epoch = STDTIME_DOUBLE_ERR;
		stdtime_errno = STDTIME_INVALID;
	}

	return epoch;

}

char	*
stdtime_etodstr (epoch )
double	epoch;
{
	static	char	ret_str[STR_BUF_LEN];
	char		*human_a;

	stdtime_errno = STDTIME_NOERR;
	
	if (check_epoch (epoch) == STDTIME_NOERR)
	{
		human_a = stdtime_format (epoch, "%O");

		strncpy (ret_str, human_a, STR_BUF_LEN);

		return ret_str;	
	}
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
}

char	*
stdtime_etotstr (epoch)
double	epoch;
{
	static char	ret_str[STR_BUF_LEN];
	char		*human_a;

	stdtime_errno = STDTIME_NOERR;
	
	if (check_epoch (epoch) == STDTIME_NOERR)
	{
		human_a = stdtime_format (epoch, "%o");

		strncpy (ret_str, human_a, STR_BUF_LEN);

		return ret_str;
	}
	else
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_STRING_ERR;
	}
}
