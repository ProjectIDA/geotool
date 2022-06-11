/*
 * NAME
 *	get_version - return the current version of this software
 *	cmp_version - compare two versions to see if they are compatible
 * FILE
 *	version.c
 * SYNOPSIS
 *	Provide a mechanism to insure two processes are built with
 *	the same version of the software.  This is not the worlds
 *	niftiest way of doing so.
 * DESCRIPTION
 *	char *
 *	get_version()	- return a string containing the current version.
 *			  The string is in static memory.
 *	cmp_version(v1, v2)
 *	char	*v1;
 *	char	*v2;	- compare v1 to v2 and return TRUE if they are the
 *			  same, FALSE otherwise
 * DIAGNOSTICS
 *	None.
 * AUTHOR
 *	Pete Ware	3 Mar 1988
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>

#include	"libaesir.h"

static char	*version = AESIR_VERSION;

char *
get_version ()
{
	return version;
}

int
cmp_version (v1, v2)
CONST char	*v1;
CONST char	*v2;
{
	if (!v1 || !*v1 || !v2 || !*v2)
		return FALSE;
	else
		return (strcmp (v1, v2));
}
