/* NAME
 *	lowercase, uppercase \-  string functions to change case.
 * SYNOPSIS
 *	.nf
 *	void
 *	lowercase (src, dst)
 *	char		*src;
 *	char		*dst;
 *	.fi
 *	.nf
 *	void
 *	uppercase (src, dst)
 *	char		*src;
 *	char		*dst;
 *	.fi
 * DESCRIPTION
 *	lowerCase \- All characters in src are converted from upper
 *	case to lower case and placed in dst.  The source string must 
 *	be null terminated.
 *	.br
 *	upperCase \- All characters in src are converted from lower 
 *	case to upper case and placed in dst.  The source string must 
 *	be null terminated.
 *	.br
 * AUTHOR
 *	09 Jan 1990	Pete Ware	Documented
 *	15 Jan 1990	Brian Smithey	Added freeStrArray()
 *	02 Jun 1992	Shawn Wass	Moved from ARS to libaesir.a
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
 
#include	<stdio.h>
#include	<ctype.h>
#include	"aesir.h"
#include	"libaesir.h"	/* Function prototype checking */


	/*
	 *  Convert a string to all lower case letters.
	 *  The string must be null terminated.
	 */
void
lowercase (char *src, char *dst)
{
	if (!dst)
		return;
	if (!src)
	{
		*dst = '\0';
		return;
	}
	while (*src)
	{
		*dst++ = (isupper ((int)*src) ? tolower ((int)*src) : *src);
		++src;
	}
	*dst = '\0';
}


	/*
	 *  Convert a string to all upper case letters.
	 *  The string must be null terminated.
	 */
void
uppercase (char *src, char *dst)
{
	if (!dst)
		return;
	if (!src)
	{
		*dst = '\0';
		return;
	}
	while (*src)
	{
		*dst++ = (islower ((int)*src) ? toupper ((int)*src) : *src);
		++src;
	}
	*dst = '\0';
}
