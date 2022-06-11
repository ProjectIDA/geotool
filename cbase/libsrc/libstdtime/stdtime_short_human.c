/*
 *NAME
 *	short human format time function
 *
 *FILE
 *
 *	stdtime_short_human.c
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

double
stdtime_shtoe (sh_a)
char	*sh_a;
{
	return stdtime_htoe(sh_a);
}

/*
 * Fortran-callable version.
 */
void
stdtimeshtoe_ (char *human, double *epoch, int human_len)
{
	if (epoch == (double *) NULL)
		return;

	if (human == (char *) NULL)
		return;
	
	*epoch = stdtime_shtoe (human);
}
	
char *
stdtime_etos (epoch_t epoch, int ndec, char *std_a, int std_len)
/*  converts time from epoch seconds to an ISO standard format string
 *  (YYYY-MM-DD hh:mm:ss.s). The number of decimal seconds is specified
 *  by ndec (0, 1, 3, 6).  Additional decimal digits are truncated.
 */
{
	int ret_val;
	char format[32];

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
	
	strcpy(format, "%Y-%m-%d %T");
	if (ndec > 0 && ndec <= 6)
	{
		strcat(format, ".%1N");
		format[13] = '0' + ndec;
	}
	/*
	 * Disallow decimal values outside the [0,6] range.
	 * Note that ndec=0 case serves as the default format.
	 */
	else if (ndec < 0 || ndec > 6)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}

	ret_val = stdtime_format_r(epoch, format, std_a, std_len);
	if ( ret_val != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return std_a;
}

/*
 * Fortran-callable version.
 */
void
stdtimeetos_ (double *epoch, int *ndec, char *human, int human_len)
{
	if (epoch == (double *) NULL)
		return;

	if (human == STDTIME_CHAR_ERR)
		return;

	stdtime_etos (*epoch, *ndec, human, human_len);

	return;
}

char *
stdtime_etosd (epoch_t epoch, char *std_a, int std_len)
/*  converts time from epoch seconds to an ISO standard format string
 *  (YYYY-MM-DD).
 */
{
	int ret_val;

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
	
	ret_val = stdtime_format_r(epoch, "%Y-%m-%d", std_a, std_len);
	if ( ret_val != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return std_a;
}

char *
stdtime_etost (epoch_t epoch, int ndec, char *std_a, int std_len)
/*  converts time from epoch seconds to an ISO standard format string
 *  (hh:mm:ss.s). The number of decimal seconds is specified by ndec
 *  (0, 1, 3, 6). Additional decimal digits are truncated.
 */
{
	int ret_val;
	char format[32];

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
	
	strcpy(format, "%T");
	if (ndec > 0 && ndec <= 6)
	{
		strcat(format, ".%1N");
		format[4] = '0' + ndec;
	}
	/*
	 * Disallow decimal values outside the [0,6] range.
	 * Note that ndec=0 case serves as the default format.
	 */
	else if (ndec < 0 || ndec > 6)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}

	ret_val = stdtime_format_r(epoch, format, std_a, std_len);
	if ( ret_val != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return std_a;
}

char*
stdtime_etodir (epoch_t epoch, char *fmt_a, char *dir_a, int dir_len)
/*  converts time from epoch seconds to an ISO standard format string
 *  (hh:mm:ss.s). The number of decimal seconds is specified by ndec
 *  (0, 1, 3, 6). Additional decimal digits are truncated.
 */
{
	int i, ret_val;
	char lastc;

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
	
	
	for (ret_val=0, i=0, lastc=0; i<strlen(fmt_a); i++)
	{
		if (lastc == '%')
		{
			switch (fmt_a[i])
			{
			/* list of allowable format chars */
			case 'd':
			case 'H':
			case 'j':
			case 'J':
			case 'm':
			case 'M':
			case 'S':
			case 'T':
			case 'Y':
				break;
		        default:
				ret_val = 1;
			}
		}
		else switch (fmt_a[i])
		{
		/* list of other allowable chars */
		case '/':
		case '%':
		case ':':
			break;
		default :
			ret_val = 1;
		}
		if (ret_val)
		{
			stdtime_errno = STDTIME_INVALID;
			return STDTIME_CHAR_ERR;
		}
		lastc = fmt_a[i];
	}

	ret_val = stdtime_format_r(epoch, fmt_a, dir_a, dir_len);
	if ( ret_val != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_CHAR_ERR;
	}
	return dir_a;
}

char *
stdtime_etosh (epoch)
double	epoch;
{
	char	*sh_a;

	stdtime_errno = STDTIME_NOERR;

	/* this check is included due to a quirk in ARS and included libraries
	 * that sometimes the NULL_TIME is sent in and a string of NONE 
	 * is expected
	 */
	if (epoch == TIME_NA) 
	{
		return STDTIME_TIME_NONE;
	}


	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
	
	sh_a = stdtime_format (epoch, "%g");

	return sh_a;	
}

int
stdtime_etosh_r (epoch, sh_a, n)
double	epoch;
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

	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}
	
	return stdtime_format_r (epoch, "%g", sh_a, n);
}

/*
 * Fortran-callable version.
 */
void
stdtimeetosh_ (double *epoch, char *human, int human_len)
{
	if (epoch == (double *) NULL)
		return;

	if (human == (char *) NULL)
		return;

	stdtime_etosh_r (*epoch, human, human_len);

	return;
}

/*
 * Functions to handle short-short human format, which is just
 * like short-human except without the milliseconds.
 */
double
stdtime_sshtoe (sh_a)
char	*sh_a;
{
	/*
	 * Just pass it on to stdtime_shtoe since it handles the
	 * the lack of milliseconds.
	 */

	return stdtime_htoe (sh_a);
}

char *
stdtime_etossh (epoch)
double	epoch;
{
	char	*sh_a;

	stdtime_errno = STDTIME_NOERR;

	/* this check is included due to a quirk in ARS and included libraries
	 * that sometimes the NULL_TIME is sent in and a string of NONE 
	 * is expected
	 */
	if (epoch == TIME_NA) 
	{
		return STDTIME_TIME_NONE;
	}

	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
	
	sh_a = stdtime_format ( epoch , "%f %T");

	return sh_a;	
}

int
stdtime_etossh_r (epoch, sh_a, n)
double	epoch;
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

	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}
	
	return stdtime_format_r ( epoch , "%f %T", sh_a, n);
}

/*  stdtime_etou() converts an epoch time to a "universal" time string */

char *
stdtime_etou (epoch)
double  epoch;
{
        char    *sh_a;
 
        stdtime_errno = STDTIME_NOERR;
 
        /* this check is included due to a quirk in ARS and included libraries
         * that sometimes the NULL_TIME is sent in and a string of NONE
         * is expected
         */
        if (epoch == TIME_NA)
        {
                return STDTIME_TIME_NONE;
        }
 
 
        if (check_epoch (epoch) != STDTIME_NOERR)
        {
                stdtime_errno = STDTIME_INVALID;
                return STDTIME_CHAR_ERR;
        }
 
        sh_a = stdtime_format (epoch, "%Y-%j:%H.%M.%S.%N");
 
        return sh_a;
}

int
stdtime_etou_r (epoch, sh_a, n)
double  epoch;
char    *sh_a;
int     n;
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
 
        if (check_epoch (epoch) != STDTIME_NOERR)
        {
                stdtime_errno = STDTIME_INVALID;
                return STDTIME_INT_ERR;
        }
 
        return stdtime_format_r (epoch, "%Y-%j:%H.%M.%S.%N", sh_a, n);
}
 
/*
 * Fortran-callable version.
 */
void
stdtimeetou_ (double *epoch, char *human, int human_len)
{
        if (epoch == (double *) NULL)
                return;
 
        if (human == (char *) NULL)
                return;
 
        stdtime_etou_r (*epoch, human, human_len);
 
        return;
} 
