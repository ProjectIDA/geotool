/*
 * NAME
 *	nap.c
 *
 * SYNOPSIS
 *	A sub-second sleep routine.  Also works for > 1 second.
 *
 * DESCRIPTION
 *	Use select() to sleep on no file descriptors.
 *
 * DIAGNOSTICS
 *	None.  Void function.  Immediately returns if arguments sum to
 *	negative time.
 *
 * FILES
 *	None.
 *
 * NOTES
 *	The granularity is that of the system probably 1/60 second.
 *
 * SEE ALSO
 *
 * AUTHOR
 *	Jim Wang, May 1990.
 */


/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<sys/types.h>
#include	<sys/time.h>
#include	"libaesir.h"

#define	MILLION	1000000

void
nap(seconds, microseconds)
int	seconds, microseconds;
{
	struct	timeval	timeout;

	if (MILLION*seconds + microseconds < 0)
		return;

	timeout.tv_sec = seconds + microseconds/MILLION;
	timeout.tv_usec = microseconds % MILLION;

		/* a cheap sub-second delay */
	(void) select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &timeout);
}
