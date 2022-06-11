/* Author: Pete Ware, SAIC, Geophysics Division / San Diego CA / (619) 458-2520
 *
 * Development funded by the Nuclear Monitoring Research Office of DARPA
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>
#include	<sys/time.h>

#include	"libaesir.h"

static int	start_seconds = -1;	/* when  we started getting time */
static int	last_time = -1;	/* last value returned */

int
getMilliTime (void)
{
	struct timeval	tv;
	struct timezone	tz;
	int		milli;

	(void) gettimeofday (&tv, &tz);
	if (start_seconds < 0)
		start_seconds = tv.tv_sec;
	milli = (tv.tv_sec - start_seconds)*1000;
	milli += (tv.tv_usec/1000);
	if (milli <= last_time)
		milli = last_time + 1;
	last_time = milli;
	return milli;
}

int
MillitoSeconds (int milli)
{
	return start_seconds + (milli/1000);
}

int
getMilliElapse (int start)
{
	return getMilliTime () - start;
}
