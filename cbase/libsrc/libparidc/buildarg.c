/* NAME
 *	buildarg_
 *
 * FILE
 *	buildarg.c
 *
 * SYNOPSIS
 *	This file contains functions that set up fortran command-line arguments
 *	so they can be sent to setpar.  Buildarg takes a character string and
 *	adds it to the structure to be sent to setarg.  flusharg actually
 *	makes the call to setarg.
 *
 * DESCRIPTION
 *	void buildarg_(str, str_len)
 *		char	*str	=> (i) Character string to add to array.
 *		int	*len_str=> (i) Length of string (trailing blanks removed).
 *		int	str_len	=> (i) Length of string passed down by Fortran.
 *
 * DIAGNOSTICS
 *
 *
 * FILES
 *
 *
 * NOTES
 *
 *
 * SEE ALSO
 *		None.
 *
 * AUTHOR
 *	Ethan Brown
 *	Geophysics Division
 *	Science Applications International Corporation
 *	10260 Campus Point Drive
 *	San Diego, CA 92121
 *
 *	28 April 1994
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aesir.h>
#include "libpar.h"

void buildarg_ (char *str, int *len_str, int str_len);
void flusharg_ (void);

char **par_argv = (char **)NULL;
int par_argc = 0;

void
buildarg_ (char *str, int *len_str, int str_len)
{
        if ((par_argv=UREALLOC(par_argv, char *, (par_argc + 1))) == NULL)
	{
		fprintf(stderr, "****ERROR:  URALLOC failed in buildarg_");
		return;
	}

        if ((par_argv[par_argc]=UALLOC(char, (*len_str + 1))) == NULL)
	{
		fprintf(stderr, "****ERROR:  UALLOC failed in buildarg_");
		return;
	}

	strncpy(par_argv[par_argc], str, (size_t)(*len_str));
	par_argv[par_argc][*len_str] = '\0';
	par_argc++;
}


void
flusharg_ (void)
{
        int  i;

        setpar(par_argc, (const char **)par_argv);

        if (par_argv)
        {
             for (i = 0; i < par_argc; i++)
                  UFREE (par_argv[i]);
        }
        par_argc = 0;
        UFREE(par_argv);
}
