/*
 *NAME
 *	get ldddate based on now and the gm time time function
 *
 *	get stdtime_gol_lddate returns the lddate either as gmtime or loclatime
 *	depending on the flag send in!
 *
 *	stdtime_etoyyyymmdd () returns the first 8 chars of the lddate format
 *	as a int
 *
 *FILE
 *
 *	stdtime_get_lddate.c
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
 *	Daria F. Buonassisi - original functions from libdb30 and libdiscrim modified for y2k
 *	SAIC
 *	San Diego, CA.
 */

#include <string.h>

#include "libstdtimeP.h"

char *
stdtime_get_lddate( lddate )
char	*lddate;
{
	epoch_t	time_now;

	stdtime_errno = STDTIME_NOERR;

	if (check_lddate (lddate) != STDTIME_NOERR)
	{
		/* They passed an empty string */
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
	else
	{
		/* Get the current time */
		time_now = stdtime_get_epoch ();

		/* Format the time */
		/* Ensure that the formatting was successful */
		if (stdtime_format_r (time_now, "%L", lddate,
				   STDTIME_LDDATE_SIZE) == STDTIME_SUCCESS)
		{
			return lddate;
		}
		else
		{
			/* stdtime_format_r() sets stdtime_errno */
			strcpy (lddate, STDTIME_STRING_ERR);
			return STDTIME_CHAR_ERR;
		}
	}
}

/*
 * stdtime_gol_lddate_r() is a replacement for the gdi funtion
 * gdi_get_date_now(). It fills in the lddate string with either
 * the gmt or local time based on the value of the gmt_or_local flag.
 * It returns STDTIME_INT_ERR if the lddate pointer is NULL or if
 * stdtime_format() failes.
 *
 * WARNING this function assumes that lddate points to an array that
 *         is at least STDTIME_LDDATE_SIZE bytes in size.
 */

int
stdtime_gol_lddate_r (int gmt_or_local, char *lddate)
{
	double  time_now;

	stdtime_errno = STDTIME_NOERR;

	/* make sure lddate is not NULL */
	if (lddate == (char *) NULL)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}

	/* Get the time structure */
	time_now = stdtime_now ();

	/* Format the time
	 * Retrieve the lddate string
	 */
	stdtime_format_gol_r(time_now, "%L", lddate, STDTIME_LDDATE_SIZE,
                             gmt_or_local);

	/* Ensure that the formatting was successful */
	if ( stdtime_format_gol_r(time_now, "%L", lddate, STDTIME_LDDATE_SIZE,
                                  gmt_or_local) == STDTIME_SUCCESS)
	{
		return STDTIME_SUCCESS;
	}
	else
	{
		/*
		 * An error was received.
		 */
		strcpy (lddate, STDTIME_STRING_ERR);
		return STDTIME_INT_ERR;
	}
}

char *
stdtime_gol_lddate (int gmt_or_local)
{
	static char lddate[STDTIME_LDDATE_SIZE];

	stdtime_errno = STDTIME_NOERR;

	if (stdtime_gol_lddate_r(gmt_or_local, lddate) != STDTIME_SUCCESS)
	{
		return STDTIME_STRING_ERR;
	}

	return lddate;
}

/*
 * Convert an epoch time to a int date (yyyymmdd).
 */

int
stdtime_etoyyyymmdd (epoch)
double epoch;
{
	char	yyyymmdd_str[9];
	int	yyyymmdd_int;

	stdtime_errno = STDTIME_NOERR;

	/* use stdtime to get the load date string which is of
	 * the form YYYYMMDD HH:MM:SS then sscanf the first 8
	 * chars into a int
	 */
	if (stdtime_format_r (epoch, "%Y\045m\045d", yyyymmdd_str,
			      sizeof(yyyymmdd_str)) != STDTIME_SUCCESS)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}

	sscanf (yyyymmdd_str, "%d", &yyyymmdd_int);

	return yyyymmdd_int;

}

/*
 * Convert an epoch time to a lddate date string (%L)
 */

char *
stdtime_etol (epoch, lddate_a, lddate_len)
epoch_t epoch;
char	*lddate_a;
int	lddate_len;
{
        stdtime_errno = STDTIME_NOERR;

        /* this check is included due to a quirk in ARS and included libraries
         * that sometimes the NULL_TIME is sent in and a string of NONE
         * is expected
         */
        if (epoch == TIME_NA)
        {
                stdtime_errno = STDTIME_INVALID;
                return STDTIME_CHAR_ERR;
        }

        if (check_epoch (epoch) != STDTIME_NOERR)
        {
                stdtime_errno = STDTIME_INVALID;
                return STDTIME_CHAR_ERR;
        }

	/*
	 * Use stdtime_format to retrieve the epoch time in %L
	 * format (YYYYMMDD HH:MM:SS)
	 */
	if (stdtime_format_r (epoch, "%L", lddate_a, lddate_len) !=
		STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
        return lddate_a;
}

int
stdtime_etol_r (epoch, sh_a, n)
double  epoch;
char	*sh_a;
int	n;
{
        stdtime_errno = STDTIME_NOERR;

        /* this check is included due to a quirk in ARS and included libraries
         * that sometimes the NULL_TIME is sent in and a string of NONE
         * is expected
         */
        if (epoch == TIME_NA)
        {
		strncpy(sh_a, STDTIME_TIME_NONE, n);
                return STDTIME_SUCCESS;
        }

        if (check_epoch (epoch) != STDTIME_NOERR ||
	    sh_a == (char *)NULL)
        {
                stdtime_errno = STDTIME_INVALID;
                return STDTIME_INT_ERR;
        }

	/*
	 * Use stdtime_format to retrieve the epoch time in %L
	 * format (YYYYMMDD HH:MM:SS)
	 */
        return stdtime_format_r (epoch, "%L", sh_a, n);
}

