/*
 * Copyright 1989 Science Applications International Corporation
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of S.A.I.C. not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  S.A.I.C. makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * S.A.I.C. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * S.A.I.C.  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Pete Ware, SAIC, Geophysics Division / San Diego CA / (619) 458-2520
 *
 * Development funded by the Nuclear Monitoring Research Office of DARPA
 */

#include        "config.h"
#include	"libstdtimeP.h"

static int	start_seconds = -1;	/* when  we started getting time */
static int	last_time = -1;	/* last value returned */

int
stdtime_getMilliTime ()
{
	struct timeval	tv;
	int		milli;

#ifndef HAVE_GETTIMEOFDAY_1
        gettimeofday (&tv, NULL);
#else
        (void) gettimeofday (&tv);
#endif /* HAVE_GETTIMEOFDAY_1 */
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
stdtime_MillitoSeconds (milli)
int		milli;
{
	return start_seconds + (milli/1000);
}

int
stdtime_getMilliElapse (start)
int		start;
{
	return stdtime_getMilliTime () - start;
}
