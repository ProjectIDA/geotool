/*
 * NAME
 *	Ordinal (was incorrectly Julian) format time function
 *
 * FILE
 *
 *	stdtime_yyyydoy_format.c
 *
 * SYNOPSIS
 *
 * int
 * stdtime_etoj ( epoch )
 * double	epoch;
 *
 * Convert an epoch time to an ordinal date of form YYYYDOY.
 * 
 * void
 * stdtimeetoj_ (epoch, yyyydoy)
 * double	*epoch;
 * int	*yyyydoy;
 *
 * Fortran-callable version of above.
 * 
 * int
 * stdtime_htoj ( human )
 * char	*human;
 *
 * Convert a human time to an ordinal date.
 * 
 * void
 * stdtimehtoj_ (human, yyyydoy, human_len)
 * char	*human;
 * int	*yyyydoy;
 * int  human_len;
 *
 * Fortran-callable version of above.
 *
 * DESCRIPTION
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * stdtime_yyyydoy_format.c
 *
 * NOTES
 *
 * SEE ALSO
 *
 * AUTHOR
 *	Jim Wang
 *	SAIC
 *	San Diego, CA
 */

#include "libstdtimeP.h"

/*
 * Convert an epoch time to an ordinal date (yyyydoy).
 */
jdate_t
stdtime_etoj ( epoch )
epoch_t	epoch;
{
	char	ord_str[8];
	jdate_t	ord_int;

	stdtime_errno = STDTIME_NOERR;

	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}

	/* Use stdtime_format_r to get a string for the yyyydoy. */
	if (stdtime_format_r (epoch, "%J", ord_str,
			    sizeof(ord_str)) != STDTIME_SUCCESS)
	{
		/* stdtime_format_r() sets stdtime_errno */
		return STDTIME_INT_ERR;
	}

	/* Convert to a int. */
	sscanf (ord_str, "%d", &ord_int);

	return ord_int;
}

/*
 * Fortran-callable version.
 */
void
stdtimeetoj_ (epoch, yyyydoy)
double	*epoch;
int	*yyyydoy;
{
	*yyyydoy = stdtime_etoj (*epoch);

	return;
}

/*
 * Convert a ordinal date to epoch time.
 */
double
stdtime_jtoe (yyyydoy)
int	yyyydoy;
{
        int i,year,day,days=0;

	stdtime_errno = STDTIME_NOERR;

        day  = yyyydoy % 1000;
	year = (yyyydoy - day) / 1000;

	if (check_yyyydoy (yyyydoy) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

        if( year > EPOCH_YEAR_ZERO ){
                for( i=EPOCH_YEAR_ZERO ; i < year ; i++ ){
                        days += 365;
                        if( stdtime_isleap(i) ) days++;
                }
        }
        if( year < EPOCH_YEAR_ZERO ) {
                for( i=year ; i < EPOCH_YEAR_ZERO ; i++ ){
                        days -= 365;
                        if( stdtime_isleap(i) ) days--;
                }
        }
        days += day - 1;
        return( (double)days * 86400. );
}


/*
 * Convert a human time string to ordinal date.
 * Process: human string -> epoch -> ordinal str -> ordinal int.
 */
int
stdtime_htoj ( human )
char	*human;
{
	double	epoch;

	/* Get epoch. */
	epoch = stdtime_htoe (human);
	if (stdtime_errno != STDTIME_NOERR)
	{
		return STDTIME_DOUBLE_ERR;
	}

	return stdtime_etoj(epoch);
}

/*
 * Fortran-callable version.
 */
void
stdtimehtoj_ (human, yyyydoy, human_len)
char	*human;
int	*yyyydoy;
int	human_len;
{
	*yyyydoy = stdtime_htoj (human);

	return;
}



/*
 * Convert a ordinal date to a human time string.
 */
char *
stdtime_jtoh (yyyydoy)
int	yyyydoy;
{
	stdtime_errno = STDTIME_NOERR;
	
	if (check_yyyydoy (yyyydoy) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}

	return stdtime_format (stdtime_jtoe(yyyydoy), "%h");
}

int
stdtime_jtoh_r (yyyydoy, human_ap, n)
int	yyyydoy;
char	*human_ap;
int	n;
{
	stdtime_errno = STDTIME_NOERR;
	
	if (check_yyyydoy (yyyydoy) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}

	return stdtime_format_r (stdtime_jtoe(yyyydoy), "%h", human_ap, n);
}

/*
 * Fortran-callable version.
 */
void
stdtimejtoh_ (yyyydoy, human, human_len)
int	*yyyydoy;
char	*human;
int     human_len;
{
	if (yyyydoy == (int *) NULL)
		return;

	if (human == (char *) NULL)
		return;
	
	stdtime_jtoh_r (*yyyydoy, human, human_len);

	return;
}


