/*
 *NAME
 *	unformat_time
 *
 *FILE
 *
 *	unformat_time.c
 *
 *	See libstdtime.info
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
#include <time.h>   /* for strptime */
/* Remember to use -D_XOPEN_SOURCE to get strptime prototype */

#include "libstdtimeP.h"

double
stdtime_unformat(char *time, char *format)
{
    char	fmt1[256];
    char	fmt2[256];
    char	buf1[256];
    char	buf2[256];
    char	tmp_c[64];
    char	fmt_c;
    int		fmt_op;
    struct tm 	tmp_tm;
    int		idx, idx2, idx3;
    char	*ptr;
    double	val_E = 0.0;
    int		have_E = 0;
    double	val_i = 0.0;
    int		have_i = 0;
    double	val_N = 0.0;
    int		val_N_op = 0;
    int		have_N = 0;
    double	ans;

    /* reset stdtime_errno */
    stdtime_errno = STDTIME_NOERR;


    /* Copy the time string into buf1 */
    strncpy(buf1, time, 256); /* Make sure we don't overrun our buffer */
    buf1[255] = '\0';

    /*
     * Copy the format into fmt1 replacing the expandable formats
     * at the same time.
     */
    if (__replace_expandable_formats(format, fmt1, sizeof(fmt1)) != 0)
    {
	stdtime_errno = STDTIME_INVALID;
	return STDTIME_DOUBLE_ERR;
    }

    /* 
     * strptime does not initialize structure, it just fills in the
     * fields specified by the format string (except for %j, %U and %W
     * which default to the current year).  Should the structure be 
     * initialized to Jan. 1, 1970? 
     */
    memset (&tmp_tm, 0, sizeof(tmp_tm));

    /*
     * Find any special formats and extract their values.
     */
    while((idx = __find_special_format(fmt1, &fmt_c, &fmt_op)) != -1)
    {
	/* Copy the non special part of the format into fmt2 */
	strncpy(fmt2, fmt1, idx);
	fmt2[idx] = '\0'; /* Add the nul terminator */

	/*
	 * Use strptime to find the begining of the part of the
	 * time string that need to be unformated using the special format
	 */
	ptr = strptime(buf1, fmt2, &tmp_tm);


	/* Bail if strptime fails */
	if (ptr == NULL)
	{
	    stdtime_errno = STDTIME_PARSE;
	    return STDTIME_DOUBLE_ERR;
	}

	/* Copy the portion recognized by strptime to buf2 */
	strncpy(buf2, buf1, ptr-buf1);
	buf2[ptr-buf1] = '\0';

	/* Unformat the part recognized by the special format */
	switch(fmt_c)
	{
	case 'i':
	    /* Find the %i part of time string */
	    idx2 = strspn(ptr, " ");
	    idx2 += strspn(&ptr[idx2], "+-1234567890");

	    /* If nothing is found then bail */
	    if (idx2 == 0)
	    {
		stdtime_errno = STDTIME_PARSE;
		return STDTIME_DOUBLE_ERR;
	    }

	    /* Copy the %i part into tmp_c */
	    strncpy(tmp_c, ptr, idx2);
	    tmp_c[idx2] = '\0';

	    /* convert to a double */
	    val_i = atof(tmp_c);

	    /* Increment ptr past the %i part */
	    ptr += idx2;

	    /* Copy the rest of the time string int buf2 minus the %i part */
	    strcat(buf2, ptr);

	    /* Copy the rest of the format into fmt2 minus the %i part */
	    strcat(fmt2, &fmt1[idx+2]);

	    /* Set a flag indicating that we found a %i part */
	    have_i = 1;

	    break;
	case 'E':
	    /* Find the %E part of time string, minus the decimal part */
	    idx2 = strspn(ptr, " ");
	    idx2 += strspn(&ptr[idx2], "+-1234567890");
	    if (ptr[idx2] == '.')
		idx3 = strspn(&ptr[idx2+1],"1234567890")+1;
	    else
		idx3 = 0;

	    if (idx2 == 0 || idx3 < 4)
	    {
		stdtime_errno = STDTIME_PARSE;
		return STDTIME_DOUBLE_ERR;
	    }

	    /* Assume a decimal part of '.NNN' */
	    idx2 += 4;

	    /* Copy the %E part into tmp_c */
	    strncpy(tmp_c, ptr, idx2);
	    tmp_c[idx2] = '\0';

	    /* Convert the %E part into a double */
	    val_E = atof(tmp_c);

	    /* Increment ptr past the %E part */
	    ptr += idx2;

	    /* Copy the rest of the time string into buf2 minus the %E part */
	    strcat(buf2, ptr);

	    /* Copy the rest of the format into fmt2 minus the %E part */
	    strcat(fmt2, &fmt1[idx+2]);

	    /* Set a flag indicating that we found a %E part */
	    have_E = 1;

	    /* 
	     * Indicate that we have an %N part (part of %E) that is 3 places
	     * of precision if we don't already have something of greater
	     * precision.
	     */
	    if (val_N_op < 3)
		val_N_op = 3;

	    break;
	case 'N':
	    /* Copy the %N part into tmp_c */
	    strcpy(tmp_c, "0.");

	    strncpy(&tmp_c[2], ptr, fmt_op);

	    tmp_c[fmt_op+2] = '\0';

	    /*
	     * If we don't have something of equal or greater precision then
	     * get the value of %N
	     */
	    if (val_N_op < fmt_op)
	    {
		val_N = atof(tmp_c);
		val_N_op = fmt_op;
	    }

	    /* Increment ptr past the %n part */
	    ptr += fmt_op;

	    /* Copy the rest of the time string into buf2 minus the %N part */
	    strcat(buf2, ptr);

	    /* Copy the rest of the format into fmt2 minus the %N part */
	    if (fmt1[idx+1] != 'N')
		strcat(fmt2, &fmt1[idx+3]);
	    else
		strcat(fmt2, &fmt1[idx+2]);

	    /* Indicate that we have a %N part */
	    have_N = 1;

	    break;
	case 'h':
	    /* Find the %E part of the %h part of time string,
	     * minus the decimal part */
	    idx2 = strspn(ptr, " ");
	    idx2 += strspn(&ptr[idx2], "+-1234567890");
	    if (ptr[idx2] == '.')
		idx3 = strspn(&ptr[idx2+1],"1234567890")+1;
	    else
		idx3 = 0;

	    if (idx2 == 0 || idx3 < 4 ||
		strlen(&ptr[4]) < 30)
	    {
		stdtime_errno = STDTIME_PARSE;
		return STDTIME_DOUBLE_ERR;
	    }

	    /* Assume a decimal part of '.NNN' */
	    idx2 += 4;

	    /* Copy the %E part into tmp_c */
	    strncpy(tmp_c, ptr, idx2);
	    tmp_c[idx2] = '\0';

	    /* Convert the %E part into a double */
	    val_E = atof(tmp_c);

	    /* Increment ptr past the rest of the %h part */
	    ptr += idx2 + 30;

	    /* Copy the rest of the time string into buf2 minus the %h part */
	    strcat(buf2, ptr);

	    /* Copy the rest of the format into fmt2 minus the %h part */
	    strcat(fmt2, &fmt1[idx+2]);

	    /* Set a flag indicating that we found a %h part */
	    have_E = 1;

	    /* 
	     * Indicate that we have an %N part (part of %E) that is 3 places
	     * of precision if we don't already have something of greater
	     * precision.
	     */
	    if (val_N_op < 3)
		val_N_op = 3;

	    break;
	default:
	    /* We should never actualy get here */
	    stdtime_errno = STDTIME_PARSE;
	    return STDTIME_DOUBLE_ERR;
	}

	/* Copy the time string minus the specially formated part into buf1. */
	strcpy(buf1, buf2);

	/* Copy the format minus the special format part into fmt1 */
	strcpy(fmt1, fmt2);
    }

    /*
     * Call strptime with a time string and format that don't have any
     * special formats.
     */
    if (strptime(buf1, fmt1, &tmp_tm) == NULL)
    {
	stdtime_errno = STDTIME_PARSE;
	return STDTIME_DOUBLE_ERR;
    }


    /*
     * If we found an %E part and there was no %N part with greater precision
     * then this is the number we return.
     */
    if (have_E && val_N_op <= 3)
	return(val_E);

    /*
     * If we have an %i part then we have the integer part of the epoch
     * number. Otherwise we convert the values in the tmp_tm struct into
     * the integer part of the epoch number.
     */
    if (have_i)
    {
	ans = val_i;
    }
    else
    {
	ans = (double)stdtime_timegm(&tmp_tm);
	if (stdtime_errno != STDTIME_NOERR)
	{
	     return (STDTIME_DOUBLE_ERR);
	}
	
    }

    /* If we have an %N part, add it to the epoch number */
    if (have_N)
	ans += val_N;

    return(ans);
}

