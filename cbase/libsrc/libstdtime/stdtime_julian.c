/*
 * NAME
 *	Julian date functions
 *
 * FILE
 *
 *	stdtime_julian.c
 *
 * SYNOPSIS
 *
 * DESCRIPTION
*
 *      The Julian Day Number begins at noon of the calendar date specified
 *      by mm <month>, dd <day>, yyyy <year>.  yyyy < 0 is BC; yyyy > 0 is AD
 *      yyyy = 0 is an ERROR. It will be interpreted as yyyy = -1
 *
 *
 * DIAGNOSTICS
 *
 * AUTHOR
 *	Bonnie MacRitchie, ported from libtime/juldat.c
 */

#include <math.h>
#include <string.h>

#include "libstdtime.h"
#include "libstdtimeP.h"


#define IGREG (15+31L*(10+12L*1582))   /* the start of the Gregorian */
                                       /* calendar relative to 1 AD  */

#define JGREG  2299161L                /* the start of the Gregorian */
                                       /* calendar in Julian days    */ 



#define JUL_EPOCH_ZERO  2440588.0      /* Julian day for 1970-01-01 */
#define JUL_EPOCH_MAX   JUL_EPOCH_ZERO+((double)STDTIME_EPOCH_MAX/SECONDS_PER_DAY)
#define JUL_EPOCH_MIN   JUL_EPOCH_ZERO+((double)STDTIME_EPOCH_MIN/SECONDS_PER_DAY)

static int julday(int mm, int dd, int yyyy);
static void caldat(int julian, int *mm, int *dd, int *yyyy);


julian_t
stdtime_htojulian (human_a)
char  *human_a;
{
     calendar_t   cal;
     julian_t     julian;
     char	  buf[256];

     stdtime_errno = STDTIME_NOERR;

     if (human_a == NULL)
     {
	  stdtime_errno = STDTIME_INVALID;
	  return STDTIME_DOUBLE_ERR;
     }

     /* Zero out the buffer */
     memset((void*)buf, 0, sizeof(buf));

     /* place a '[' at the begining of the buffer */
     buf[0] = '[';

     /* 
      * Copy the time string into the buffer, making sure not to overflow
      * the buffer
      */
     strncat(buf, human_a, sizeof(buf)-3);

     /* place a ']' at the end of the string */
     buf[strlen(buf)] = ']';

     /*
      * Call stdtime_expr_time_c() 
      */
     if (stdtime_expr_time_c (buf, &cal) == NULL)
     {
	  /* stdtime_expr_time_c() set stdtime_errno */
	  return (STDTIME_INT_ERR);
     }

     julian = stdtime_ctojulian (&cal);
     if (stdtime_errno != STDTIME_NOERR)
     {
	  return (STDTIME_INT_ERR);
     }
     
     return (julian);
}



julian_t
stdtime_ctojulian (cal)
calendar_t  *cal;
{
     julian_t   julian;
     
     stdtime_errno = STDTIME_NOERR;
     
     if (cal == NULL)
     {
	  stdtime_errno = STDTIME_INVALID;
	  return ((julian_t)0);
     }

     julian = julday (cal->mon, cal->day, cal->year);

     return (julian);
}


calendar_t *
stdtime_juliantoc (julian, cal)
julian_t    julian;
calendar_t  *cal;
{
     stdtime_errno = STDTIME_NOERR;
     
     if ((julian == 0) || (cal == NULL))
     {
	  stdtime_errno = STDTIME_INVALID;
	  return ((calendar_t *) NULL);
     }
     
     caldat (julian, &cal->mon, &cal->day, &cal->year);
     cal->hour = 0;
     cal->min = 0;
     cal->sec = 0;

     return (cal);
}     


/*
 *    int julday(int month, int day, int year)
 *
 *    computes the True Julian Day.
 *    julday doesn't check for valid calendar input - days are simply
 *    offsets from the month. (e.g. Mar 45 is intepreted as 44 days after
 *    Mar 1).
 *
 *    void caldat(int julian_day, int *month, int *day, int *year);
 *
 *    brings back the month/day/year.
 */

/*
 * julday() computes the true Julian Day
 * adapted from "Numerical Recipes in C"
 */

static
int julday(mm,dd,yyyy)
int mm;
int dd;
int yyyy;
{
	int jul;
	int  ja, jy, jm;

	if(yyyy < 0) ++yyyy;
	
	if (mm > 2)
	{
		jy = yyyy;
		jm = mm + 1;
	}
	else
	{
		jy = yyyy - 1;
		jm = mm   + 13;
	}
	jul = (int) (floor(365.25*jy) + (floor(30.6001 * jm)) + dd + 
		      1720995);
	
	if(dd + 31L*(mm + 12L*yyyy) >= IGREG)
	{
		ja = jy/100;
		jul += 2 - ja + (int) (ja/4);
	}
	
	return jul;
}

static
void caldat(julian, mm, dd, yyyy)
int julian;
int  *mm;
int  *dd;
int  *yyyy;
{
	int ja, jalpha, jb, jc, jd, je;
	
	if(julian >= JGREG)
	{
		jalpha =(int)(((double) (julian - 1867216) - 0.25)/36524.25);
		ja = julian + 1 + jalpha - (int) ( 0.25 * jalpha);
	}
	else
		ja = julian;
	
	jb = ja + 1524;
	jc = (int) (6680.0 + ((double) (jb - 2439870) - 122.1)/365.25);
	jd = 365 * jc + (jc/4);
	je = (jb - jd)/30.6001;
	*dd = jb - jd - (int) (30.6001 * je);
	*mm = je - 1;
	if (*mm > 12) *mm -= 12;
	*yyyy = jc - 4715;
	if (*mm > 2) --(*yyyy);
	if(*yyyy <= 0) --(*yyyy);
	
	return;
}




/*
 * some interesting factoids will make conversion to other time bases
 * a snap:
 *
 *    May 23, 1968 is Julian day 2440000
 *
 *    Jan  1, 1970 is Julian day 2440588
 *
 *    "Epoch" time is simply  julday(mm, dd, yy) - julday(1,1,1970) * 86400 +
 *    3600 * hh + 60 * min + sec
 */

epoch_t stdtime_juliantoe(julian)
julian_t julian;
{
     epoch_t epoch;
     
     stdtime_errno = STDTIME_NOERR;
     
     if ((julian < JUL_EPOCH_MIN) || (julian > JUL_EPOCH_MAX))
     {
	  stdtime_errno = STDTIME_INVALID;
	  return (STDTIME_DOUBLE_ERR);
     }
     
     epoch = (julian - JUL_EPOCH_ZERO) * SECONDS_PER_DAY;
     
     return (epoch);
}


julian_t stdtime_etojulian(epoch)
epoch_t epoch;
{
     julian_t  julian;
     
     stdtime_errno = STDTIME_NOERR;
     
     julian = floor(epoch/(double)SECONDS_PER_DAY) + JUL_EPOCH_ZERO;
     
     return (julian);
}

