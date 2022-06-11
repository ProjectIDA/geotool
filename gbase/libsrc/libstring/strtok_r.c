#include "config.h"
#include <stdio.h>
#include <strings.h>

#include "libstring.h"

#ifndef HAVE_STRTOK_R

/**
 * Extract tokens from a string. This is the UNIX strtok_r for system that do
 * not have it.
 */
char *
strtok_r(char *str, const char *tok, char **last)
{
	extern char *strtok();
	char *c, *d, *e;

	if(str != NULL)
	{
		for(e = str; *e != '\0'; e++);
		c = strtok(str, tok);
	}
	else
	{
		for(e = *last; *e != '\0'; e++);
		c = strtok(*last, tok);
	}
	if(c != NULL)
	{
		for(d = c; *d != '\0'; d++);
		if(d < e) d++;
		*last = d;
	}
	return(c);
}

#endif /* HAVE_STRTOK_R */
