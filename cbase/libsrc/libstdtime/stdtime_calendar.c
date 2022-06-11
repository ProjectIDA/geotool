/*
 * NAME
 *	Calendar date functions
 *
 * FILE
 *
 *	stdtime_calendar.c
 *
 * SYNOPSIS
 *
 * DESCRIPTION
 *
 *      A calendar date structure (calendar_t) contains the components
 *      of a date (year, month, day, hour, min, sec).
 *
 *
 * DIAGNOSTICS
 *
 * AUTHOR
 *	Bonnie MacRitchie, ported from libtime/juldat.c
 */

#include <math.h>

#include "libstdtime.h"
#include "libstdtimeP.h"


calendar_t *
stdtime_etoc (epoch, cal)
epoch_t     epoch;
calendar_t *cal;
{
     int   edays, i;
     int    diy, dim;
     double seconds;
     
     stdtime_errno = STDTIME_NOERR;

     if (cal == (calendar_t *) NULL)
     {
	  stdtime_errno = STDTIME_INVALID;
	  return ((calendar_t *) NULL);
     }

     cal->year = 0;
     cal->mon = 0;
     cal->day = 0;
     cal->hour = 0;
     cal->min = 0;
     cal->sec = 0;
     
     edays = floor (epoch / SECONDS_PER_DAY);
     seconds = epoch - ((double) edays * SECONDS_PER_DAY);

     cal->year = EPOCH_YEAR_ZERO;
     if (edays >= 0)
     {
	  for (cal->year=EPOCH_YEAR_ZERO; ; cal->year++ )
	  {
	       diy = stdtime_isleap(cal->year) ? 366:365;
	       if (edays < diy) break;
	       edays -= diy;
	  }
     } 
     else if (edays < 0)
     {
	  for (cal->year=EPOCH_YEAR_ZERO-1; ; cal->year-- )
	  {
	       diy = stdtime_isleap(cal->year) ? 366:365;
	       edays += diy;
	       if (edays >= 0) break;
	  }
     }
     
     for (i = 0; i < 12; i++)
     {
	  dim = stdtime_dim[i];
	  if ((i == 1) && stdtime_isleap(cal->year)) dim++;
	  if (edays < dim) break;
	  edays -= dim;
     }
     
     cal->mon = i+1;
     cal->day = edays+1;

     cal->hour = (int) seconds/3600;
     seconds -= cal->hour*3600;
     cal->min = (int) seconds/60;
     cal->sec = seconds - cal->min*60;
     
     return (cal);
}



epoch_t
stdtime_ctoe (cal)
calendar_t *cal;
{
	epoch_t epoch;
	int    edays;
        int     i;

	stdtime_errno = STDTIME_NOERR;

	if (cal == (calendar_t *) NULL)
	{
	     stdtime_errno = STDTIME_INVALID;
	     return STDTIME_DOUBLE_ERR;
	}

	if (check_year_mon_day(cal->year, cal->mon, cal->day) 
	    != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

	if (check_hour_min_sec(cal->hour, cal->min, cal->sec) 
	    != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

	edays = 0;
        if (cal->year > EPOCH_YEAR_ZERO )
	{
                for( i=EPOCH_YEAR_ZERO ; i < cal->year ; i++ )
		{
                        edays += 365;
                        if( stdtime_isleap(i) ) edays++;
                }
        }
        else if (cal->year < EPOCH_YEAR_ZERO ) 
	{
                for (i=cal->year ; i < EPOCH_YEAR_ZERO ; i++)
		{
                        edays -= 365;
                        if( stdtime_isleap(i) ) edays--;
                }
        }

        for (i = 0 ; i < cal->mon - 1 ; i++){
                edays += stdtime_dim[i];
                if (i == 1 && stdtime_isleap(cal->year)) edays++;
        }

        edays += (cal->day - 1);

	/* 
	 * use double since edays*SECONDS_PER_DAY is beyond min range of 
	 * a int for 1901-12-13 
	 */
	epoch = ((double)edays)*SECONDS_PER_DAY 
	     + cal->hour*3600 + cal->min*60 + cal->sec;
	
	if (check_epoch (epoch) != STDTIME_NOERR)
	{
	     stdtime_errno = STDTIME_INVALID;
	     return STDTIME_DOUBLE_ERR;
	}

        return (epoch);
}


