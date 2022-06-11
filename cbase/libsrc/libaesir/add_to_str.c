/*
 * NAME
 *	add_to_str \- add one string onto another
 * SYNOPSIS
 *	.nf
 *	add_to_str (add_string, add_to)
 *	char           *add_string;		(i) string to add
 *	char		**add_to;		(o) and place to put it
 *	.fi
 * DESCRIPTION
 *	This can continually concatenate one string on to another string.
 *	Memory is dynamically allocated for each additional string.
 *	Initially, add_to should point to NULL.  For example:
 *	.nf
 *	.sp
 *	char *add_to;
 *	add_to = NULL;
 *	add_to_str ("This is a test", &add_to);
 *	.sp
 *	.fi
 *	Will allocate space for the string "This is a test"
 * AUTHOR
 *	08/03/89	Pete Ware	Documented
 *	.br
 *	08/04/89	Pete Ware	Added missing DBUG_VOID_RETURN
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"aesir.h"
#include	"libaesir.h"
#include	"dbug.h"

void
add_to_str (char *add_string, char **add_to)
{
	int             add;
	static char    *routine = "add_to_str";

	DBUG_ENTER (routine);
	if (!add_string || !add_to)
		DBUG_VOID_RETURN;
	add = strlen (add_string);
	if (*add_to == NULL)
	{
		*add_to =  UALLOC (char, add + 1);
		if (*add_to == (char *) NULL)
		{
			perror (routine);
		}
		else
		{
			(void) strcpy (*add_to, add_string);
		}
	}
	else
	{
		*add_to = (char *)realloc (*add_to, (unsigned) (strlen (*add_to) + add + 1));
		if (*add_to == NULL)
		{
			perror (routine);
		}
		else
		{
			(void) strcat (*add_to, add_string);
		}
	}
	DBUG_VOID_RETURN;
}
