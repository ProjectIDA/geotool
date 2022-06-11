/* NAME
 *	getbpar()
 *      getdpar()
 *      getfpar()
 *      getffpar()
 * 
 * FILE 
 *	defpar.c
 *
 * SYNOPSIS
 *      par->duration = getffpar ("duration", 200.0);
 *	
 * DESCRIPTION
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 * 	Rick Jenkins  10/08/92
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libpar.h"

int
getbpar (const char *parname, const int defvalue)
{
	int parvalue = defvalue;
	
	getpar (parname, "b", (char *) &parvalue);

	return (parvalue);
}

int
getdpar (const char *parname, const int defvalue)
{
	int parvalue = defvalue;
	
	getpar (parname, "d", (char *) &parvalue);

	return (parvalue);
}

float
getfpar (const char *parname, const double defvalue)
{
	double parvalue = defvalue;
	
	getpar (parname, "F", (char *) &parvalue);

	return ((float)parvalue);
}

double
getffpar (const char *parname, const double defvalue)
{
	double parvalue = defvalue;
	
	getpar (parname, "F", (char *) &parvalue);

	return (parvalue);
}
