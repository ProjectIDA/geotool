/*
 *NAME
 *	format time function - converts the time into any format
 *
 *FILE
 *
 *	stdtime_format_time.c
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

#include <math.h>
#include <string.h>
#include <limits.h>       /* for DBL_DIG, maximum precision of a double */

/* DBL_DIG not defined under some versions of Linux */
#ifndef DBL_DIG
#define DBL_DIG 15
#endif /* DBL_DIG */

#include "libstdtimeP.h"

int
stdtime_format_gol_r ( epoch_time, format_a, human_a, human_len, gmt_or_local )
double	epoch_time;
char	*format_a;
char	*human_a;
int	human_len;
int     gmt_or_local;
{
    char	time_a[STDTIME_MAX_FORMAT_SIZE];
    char	fmt[STDTIME_MAX_FORMAT_SIZE];
    char	tmp[STDTIME_MAX_FORMAT_SIZE];
    /*    char	tmp2[16]; */
    char	*time_ptr;
    char	*fmt_ptr;
    char	fmt_c;
    int		fmt_op;
    time_t	epoch;
    struct tm	tm_s;
    epoch_t	decimal_round;
    epoch_t     decimal_epoch;
    int		ret, idx;
    int         ndays;



    /*
     * Initialize the global error variable
     */
    stdtime_errno = STDTIME_NOERR;

    /* Check validity of arguments */
    if (check_epoch (epoch_time) != STDTIME_NOERR ||
	check_fmt(format_a) != STDTIME_NOERR ||
	(human_a == (char *) NULL) ||
	human_len <= 0)
    {
	stdtime_errno = STDTIME_INVALID;
	return (STDTIME_INT_ERR);
    }

    /* Zero out the result buffer */
    memset((void*)time_a, 0, sizeof(time_a));

    /*
     * get whole epoch seconds as an integer; use floor so
     * that the algorithm works for negative seconds
     */
    epoch = (time_t) floor(epoch_time);

    /*
     * decimal seconds is the difference between the epoch time
     * and whole epoch seconds
     *
     * later on we will print decimal seconds to the maximum
     * precision (DBL_DIG), which will automatically round the
     * last digit; if the decimal rounds up to 1.0, then "epoch,"
     * (whole seconds) will be off by 1; so we calculate whole
     * and decimal seconds here before converting whole seconds
     * to a date
     */

    /* Round off the decimal to DBL_DIG places */
    decimal_round = rint((epoch_time-(double) epoch) *
			  pow(10.0, (double) DBL_DIG));
    decimal_epoch = decimal_round/pow(10.0, (double) DBL_DIG);

    /* if decimal seconds are rounded up to 1.0 */
    /* bump epoch up one and make decimal seconds 0 */
    if (decimal_epoch >= 1.0)
    {
	 epoch += 1.0;
	 decimal_epoch = 0.0;
    }


    /* Fill in the tm struct */
    if (STDTIME_GMT_TIME == gmt_or_local)
    {
       if ( gmtime_r( &epoch, &tm_s ) == NULL ) {
         /* real error is in errno and could be printed with strerror */
         stdtime_errno = STDTIME_INVALID;
         return (STDTIME_INT_ERR);
       }
    }
    else
    {
       if ( localtime_r( &epoch, &tm_s ) == NULL ) {
         /* real error is in errno and could be printed with strerror */
         stdtime_errno = STDTIME_INVALID;
         return (STDTIME_INT_ERR);
       }
    }

    /*
     * Copy the format into fmt replacing the expandable formats
     * at the same time.
     */
    if (__replace_expandable_formats(format_a, fmt, sizeof(fmt)) != 0)
    {
	stdtime_errno = STDTIME_INVALID;
	return (STDTIME_INT_ERR);
    }


    /* Set pointers to beginning of buffers */
    fmt_ptr = fmt;
    time_ptr = time_a;

    /* Find first special format, if any */
    while((idx = __find_special_format(fmt_ptr, &fmt_c, &fmt_op)) != -1)
    {
	if (idx > 0)
	{
	    /* Copy non-special format to tmp */
	    strncpy(tmp, fmt_ptr, idx);
	    tmp[idx] = '\0';

	    /* Make sure there is room in temp_a */
	    if (&time_ptr[1] >= &time_a[sizeof(time_a)])
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    /* Call strftime with format in tmp */
	    ret = strftime(time_ptr, sizeof(time_a) - (time_ptr-time_a),
			   tmp, &tm_s);
	    if (ret <= 0)
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    /* Advance time_ptr and fmt_ptr */
	    time_ptr += ret;
	    fmt_ptr += idx;
	}

	switch(fmt_c)
	{
        case 'x':			/* These are now invalid. */
        case 'y':
        case 'D':
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	case 'i':
	    /* Format %i */
	    sprintf(tmp, "%ld", epoch);

	    /* make sure we won't overflow temp_a */
	    if (&time_ptr[strlen(tmp)] > &time_a[sizeof(time_a)-1])
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    strcpy(time_ptr, tmp);

	    /* Advance time_ptr */
	    time_ptr += strlen(time_ptr);

	    /* Advance fmt_ptr */
	    fmt_ptr += 2;

	    break;

	case 'E':
	    /* Format %E */
	    sprintf(tmp, "%.3f", epoch_time);

	    /* make sure we won't overflow temp_a */
	    if (&time_ptr[strlen(tmp)] > &time_a[sizeof(time_a)-1])
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    strcpy(time_ptr, tmp);

	    /* Advance time_ptr */
	    time_ptr += strlen(time_ptr);

	    /* Advance fpt_ptr */
	    fmt_ptr += 2;
	    break;
	case 'N':
	    /* Format %N */
	    /* Make sure we won't overflow temp_a */
	    if (&time_ptr[fmt_op] > &time_a[sizeof(time_a)-1])
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    /* Create %?N format */
#if 0
	    sprintf(tmp2, "%%.%df", fmt_op);
	    /* Truncate the decimal to the requested number of places */
	    decimal_floor = (int) ((epoch_time-(double) epoch) *
		pow(10.0, (double) fmt_op));
	    decimal_trunc = (double) decimal_floor/pow(10.0, (double) fmt_op);
	    /* Format %N */
	    sprintf(tmp, tmp2, decimal_trunc);

	    strncpy(time_ptr, &tmp[2], fmt_op);
#else
	    /* Print the decimal and truncate the string to */
	    /* the requested number of places */
	    sprintf(tmp, "%.*f", DBL_DIG, decimal_epoch);
	    strncpy(time_ptr, &tmp[2], fmt_op);
#endif

	    /* Advance time_ptr */
	    time_ptr += fmt_op;
	    *time_ptr = '\0';

	    /* Advance fpm_ptr */
	    fmt_ptr += fmt_ptr[1] == 'N' ? 2 : 3;

	    break;
	case 'h':
	    /* Format %E part of %h */
	    /* BGM: why isn't this treated as an expandable format? */
	    sprintf(tmp, "%15.3f ", epoch_time);

	    /* make sure we won't overflow temp_a */
	    if (&time_ptr[strlen(tmp) + 30] > &time_a[sizeof(time_a)-1])
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

            strcpy(time_ptr, tmp);

	    /* Advance time_ptr */
	    time_ptr += strlen(time_ptr);

	    /* Format remainder of %h */
	    if (stdtime_format_r(epoch_time, "%Y %m %e %Y\045j %T.%1N",
				 time_ptr,
				 STDTIME_MAX_FORMAT_SIZE - (time_ptr-time_a))
		!= STDTIME_SUCCESS)
	    {
		stdtime_errno = STDTIME_PARSE;
		return (STDTIME_INT_ERR);
	    }

	    /* Remove leading 0 in month, if any */
	    if (time_ptr[5] == '0')
		time_ptr[5] = ' ';

	    /* Advance time_ptr */
	    time_ptr += strlen(time_ptr);
	    *time_ptr = '\0';

	    /* Advance fmt_ptr */
	    fmt_ptr += 2;

	    break;

	case 'K':
	    /* Format %K, number of days */
	    ndays = epoch / SECONDS_PER_DAY;

	    if ((ndays < 0) || (ndays > 999))
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    sprintf(tmp, "%03d", ndays);

	    /* make sure we won't overflow temp_a */
	    if (&time_ptr[strlen(tmp)] > &time_a[sizeof(time_a)-1])
	    {
		stdtime_errno = STDTIME_INVALID;
		return (STDTIME_INT_ERR);
	    }

	    strcpy(time_ptr, tmp);

	    /* Advance time_ptr */
	    time_ptr += strlen(time_ptr);

	    /* Advance fmt_ptr */
	    fmt_ptr += 2;

	    break;

	default:
	    /* We should never get here */
	    stdtime_errno = STDTIME_PARSE;
	    return (STDTIME_INT_ERR);
	}
    }

    /* If there is any format left, format it */
    if (strlen(fmt_ptr) > 0)
    {
	ret = strftime(time_ptr, sizeof(time_a) - (time_ptr-time_a),
		       fmt_ptr, &tm_s);
	if (ret <= 0)
	{
	    stdtime_errno = STDTIME_PARSE;
	    return (STDTIME_INT_ERR);
	}
    }

    /* Put results in return_a */
    strncpy(human_a, time_a, human_len);
    human_a[human_len-1] = '\0';

    return STDTIME_SUCCESS;
}

int
stdtime_format_r ( epoch_time, format_a, human_a, human_len )
double	epoch_time;
char	*format_a;
char	*human_a;
int	human_len;
{
   return(stdtime_format_gol_r(epoch_time, format_a, human_a,
                               human_len, STDTIME_GMT_TIME));
}

char *
stdtime_format_gol(double epoch_time, char *format_a, int gmt_or_local)
{
	static char	static_return[STDTIME_MAX_FORMAT_SIZE];
	int		ret_val;

	memset((void *)static_return, 0, STDTIME_MAX_FORMAT_SIZE);

	ret_val = stdtime_format_gol_r( epoch_time, format_a, static_return,
				    STDTIME_MAX_FORMAT_SIZE, gmt_or_local);

	if ( ret_val != STDTIME_SUCCESS)
	{
		strcpy( static_return, STDTIME_STRING_ERR);
	}

	return static_return;
}

char *
stdtime_format( epoch_time, format_a )
double epoch_time;
char *format_a;
{
   return(stdtime_format_gol(epoch_time, format_a, STDTIME_GMT_TIME));
}

