/*
 * .TH STRTIME "10 March 1989"
 * .SH NAME
 *	strtime \- return the local time as a date string.
 *	.br
 *	strdtime \- return a double precision epoch time to a local date.
 *	.br
 *	strgmtime \- return the current UT as a string.
 *	.br
 *	gmdate \- return a double precisions epoch time as a gm date.
 *	.br
 *	etosh \- epoch to short human time interpreted as a local time
 * .SH FILE
 *	strtime.c(libaesir.a)
 * .SH SYNOPSIS
 *	.nf
 *	char *
 *	strtime ()
 *
 *	char *
 *	strdtime (epoch)
 *	double	epoch;			(i) seconds since start of 1/1/1970
 *
 *	char *
 *	strgmtime ()
 *
 *	char *
 *	gmdate (epoch)
 *	double	epoch;			(i) seconds since start of 1/1/1970
 *
 *	char *
 *	etosh (epoch)
 *	double epoch;			(i) seconds since start of 1/1/1970
 *
 * .SH DESCRITPION
 *	.LP
 *	\fIstrtime\fP () returns the current time as a NULL terminated
 *	string.  The string is in the same format as date(1) prints.
 *	Uses asctime () to return the string but deletes the trailing
 *	newline.
 *	.LP
 *	\fIstrdtime\fP () returns the same formated string as \fIstrtime\fP ()
 *	except one passes in the epochal time.
 *	.LP
 *	\fIstrgmtime\fP () returns the current time at Greenwich as a string.
 *	.LP
 *	\fIgmdate\fP () returns an  epochal time interpreted as Greenwich time.
 *	.LP
 *	\fIetosh\fP () returns an epochal time in short human format.  Short
 *	human format is "mm/dd/yy hh:mm:ss.sss" and always is 21 characters int.
 * .SH AUTHOR
 *	Pete Ware
 * .SH BUGS
 *	All routines use \fIasctime\fP (3) to format the string and then
 *	delete the newline.  The string is stored in a  static buffer
 *	and is overwritten with each call.
 *	.LP
 *	\fIstrdtime\fP and \fIgmdate\fP are accurate only to a second.
 *	Fractions of a second are truncated.
 *	.LP
 *	\fIgmdate\fP is so named for compatability with previous
 *	usage.
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>	
#include	<stdlib.h>	
/****
#include	<sys/types.h>	
#include	<sys/time.h>	
****/
#include	<time.h>	
#include	"aesir.h"
#include	"libaesir.h"

char *
strdtime (thetime)
double	thetime;			/* (i) current time */
{
	time_t		now;		/* current time */
	char		*buf;

	now = (time_t) thetime;
	buf = ctime (&now);
	buf[24] = '\0';
	return buf;
}

char *
strtime ()
{
	time_t		now;		/* current time */

	now = time ((time_t *) NULL);
	return strdtime ((double) now);
}

char *
gmdate (epoch)
double	epoch;				/* (i) seconds since start of 1/1/1970 */
{
	time_t		clock;
	char		*cp;

	clock = (time_t) epoch;
	cp = asctime (gmtime(&clock));
	cp[24] = '\0';		/* Char 24 is a newline. */
	return(cp);
}

char *
strgmtime ()
{
	time_t		now;

	now = time ((time_t *) NULL);
	return gmdate ((double) now);
}


char *
etosh (epoch)
double		epoch;		/* (i) seconds since start of 1/1/1970 */
{
	time_t		clock;
	struct	tm	*tm;
	static char	buf[24];
	int		hundredths;

		/*
		 * The time will get stored in a 4-byte int for conversion
		 * purposes.  Check to see if it will fit.  On the sun, the
		 * actual value is around +/- 2147483647.
		 */
	if (epoch < -2.0E9 || epoch > 2.0E9)
	{
		strcpy(buf, "None");
		return buf;
	}
		
		/*
		 * We have to handle fractional seconds for negative
		 * times correctly.
		 */
	if (epoch >= 0.0)
	{
		/*
		 *  We use to calculate the hundredths of a second with 
		 *  (int) ((epoch - clock) * 1000. + .5) but this meant 
		 *  that if epoch - clock >= .9995 then you should actually 
		 *  add one to seconds.
		 */
		clock = (time_t) epoch;
		if (epoch - clock >= .9995)
		{
			clock +=1;
			hundredths = 0;
		}
		else
		{
			hundredths = (int) ((epoch - clock) * 1000.0 + .5);
		}
		
		tm = gmtime (&clock);

		sprintf (buf, "%02d/%02d/%02d %02d:%02d:%02d.%03d",
			 tm->tm_mon + 1, tm->tm_mday, tm->tm_year,
			 tm->tm_hour, tm->tm_min, tm->tm_sec,
			 hundredths);
	}
	else
	{
			/* Check for fractional seconds. */
		if (epoch == (double)(time_t) epoch)
		{
			clock = (time_t) epoch;
			tm = gmtime (&clock);
			sprintf (buf, "%02d/%02d/%02d %02d:%02d:%02d.%03d",
				 tm->tm_mon + 1, tm->tm_mday, tm->tm_year,
				 tm->tm_hour, tm->tm_min, tm->tm_sec, 0);
		}
		else
		{
			/* Round away from zero. */
			clock = -1 - ((time_t) (0.0 - epoch));

			/*
			 *  Worry about the correct hundredths of a second.
			 */
			if (epoch - clock >= .9995)
			{
				clock -=1;
				hundredths = 0;
			}
			else
			{
				hundredths = (int) ((epoch - clock) * 1000.0 
						    + .5);
			}
			tm = gmtime (&clock);

			sprintf
				(buf, "%02d/%02d/%02d %02d:%02d:%02d.%03d",
				 tm->tm_mon + 1, tm->tm_mday, tm->tm_year,
				 tm->tm_hour, tm->tm_min, tm->tm_sec,
				 hundredths);
		}
	}
	
	return buf;
}

