/*
 * NAME
 *	new formats
 *
 * FILE
 *
 *	stdtime_new_formats.c
 *
 * SYNOPSIS
 *
 *	
 *
 * DESCRIPTION
 *
 *	
 *	These functions are used by the format_time and unformat_time
 *	functions to override or add time/date formats to the standard
 *	C time/date conversion library.
 *	
 *
 *FILES
 *	None
 *
 *NOTES
 *
 *	None
 *
 *SEE ALSO
 *	
 *	None
 *
 *AUTHOR
 *	Tad Glines
 *	SAIC
 *	Patrick AFB, FL
 *
 */

#include <string.h>

#include "libstdtimeP.h"

/*
 * This function replaces special formats with an expanded form
 * that contains only standard strptime() recognized formats.
 *
 * All new expandable formats should be added here.
 */
int
__replace_expandable_formats(char *old_fmt, char *new_fmt, int n)
{
    /* 
     * The \045 is the octal representation of '%' and is used
     * to prevent SCCS from messing with the string value.
     */
    static char	*fmt_C = "%a %b %e %T %Z %Y";
    static char	*fmt_f = "%m/%d/%Y";
    static char *fmt_F = "%a %b %e %X %Z %Y";
    static char *fmt_g = "%m/%d/%Y %T.%N";
    static char *fmt_G = "%Y\045j\045H\045M\045S";
    static char *fmt_J = "%Y\045j";
    static char *fmt_L = "%Y\045m\045d %T";
    static char *fmt_o = "%H:%M:%S.%2N";
    static char *fmt_O = "%m/%d/%Y %H:%M:%S.%2N";
    char	*in    = old_fmt;
    char	*out   = new_fmt;

    while(*in != '\0')
    {
	if (*in == '%')
	{
	    in++;
	    switch(*in)
	    {
	    case 'C':
		if (&out[strlen(fmt_C)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_C);
		out += strlen(fmt_C);
		in++;
		break;
	    case 'f':
		if (&out[strlen(fmt_f)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_f);
		out += strlen(fmt_f);
		in++;
		break;
	    case 'F':
		if (&out[strlen(fmt_F)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_F);
		out += strlen(fmt_F);
		in++;
		break;
	    case 'g':
		if (&out[strlen(fmt_g)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_g);
		out += strlen(fmt_g);
		in++;
		break;
	    case 'G':
		if (&out[strlen(fmt_G)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_G);
		out += strlen(fmt_G);
		in++;
		break;
	    case 'J':
		if (&out[strlen(fmt_J)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_J);
		out += strlen(fmt_J);
		in++;
		break;
	    case 'L':
		if (&out[strlen(fmt_L)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_L);
		out += strlen(fmt_L);
		in++;
		break;
	    case 'o':
		if (&out[strlen(fmt_o)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_o);
		out += strlen(fmt_o);
		in++;
		break;
	    case 'O':
		if (&out[strlen(fmt_O)] > &new_fmt[n-1]) return (-1);
		strcpy(out, fmt_O);
		out += strlen(fmt_O);
		in++;
		break;
	    default:
		if (&out[2] > &new_fmt[n-1]) return (-1);
		*out++ = '%';
		*out++ = *in++;
		break;
	    }
	}
	else
	{
	    if (&out[1] > &new_fmt[n-1]) return (-1);
	    *out++ = *in++;
	}
    }
    *out = '\0';
    return(0);
}

/*
 * This function returns the index of the first occurance of
 * any of the special formats (i, E, and N);
 *
 * All new unexpandable formats should be handled here.
 */
int
__find_special_format(char *in_fmt, char *fmt_c, int *fmt_op)
{
    int	i;
    int n = strlen(in_fmt);

    for(i = 0; i < n; i++)
    {
	if(in_fmt[i] == '%')
	{
	    switch(in_fmt[i+1])
	    {
	    case 'i':
		*fmt_c = 'i';
		return(i);
	    case 'h':
		*fmt_c = 'h';
		return(i);
	    case 'K':
		*fmt_c = 'K';
		return(i);
	    case 'x':
		*fmt_c = 'x';
		return(i);
	    case 'y':
		*fmt_c = 'y';
		return(i);
	    case 'D':
		*fmt_c = 'D';
		return(i);
	    case 'E':
		*fmt_c = 'E';
		return(i);
	    case 'N':
		*fmt_op = 3;
		*fmt_c = 'N';
		return(i);
	    default:
		if (in_fmt[i+1] >= '1' &&
		    in_fmt[i+1] <= '9' &&
		    in_fmt[i+2] == 'N')
		{
		    *fmt_op = in_fmt[i+1] - '0';
		    *fmt_c = 'N';
		    return(i);
		}
		break;
	    }
	}
    }
    return(-1);
}
