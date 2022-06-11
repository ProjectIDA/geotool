/*
 *NAME
 *
 *	get stdtime_log_lddate returns the lddate format (%L), compressed -
 *	without the space or colons, either as gmtime or
 *	localtime depending on the flag sent in.
 *
 *FILE
 *
 *	stdtime_log_date.c
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

#include "libstdtimeP.h"


/*
 * This function returns either the local or gmt
 * formatted load date string compressed for logging.
 */

char *
stdtime_log_lddate (int gmt_or_local)
{
	double  time_now;

	time_now = stdtime_now ();

	/* Format the time
	 * Retrieve the lddate string
	 */
	return stdtime_format_gol (time_now, "%Y\045m\045d\045H\045M\045S", gmt_or_local);
}
