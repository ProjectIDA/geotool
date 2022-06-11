/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<ctype.h>
#include	"libaesir.h"

#ifndef NULL
#define         NULL 0
#endif /* NULL */

char *
skip_space (ptr)
char	*ptr;
{
	if (ptr == NULL)
		return (char *) NULL;
	while (*ptr && isspace ((int)(*ptr)))
		++ptr;
	return ptr;
}

char *
skip_nonspace (ptr)
char	*ptr;
{
	if (ptr == NULL)
		return (char *) NULL;
	while (*ptr && !isspace ((int)(*ptr)))
		++ptr;
	return ptr;
}

