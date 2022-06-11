/*
 *NAME
 *	human format time function
 *
 *FILE
 *
 *	stdtime_human_format.C
 *
 *SYNOPSIS
 *
 *DESCRIPTION
 *
 *DIAGNOSTICS
 *
 *FILES
 *
 *NOTES
 *
 *SEE ALSO
 *
 *AUTHOR
 *	Joseph Wehlen
 *	SAIC
 *	Patrick AFB, FL
 */

#include <string.h>

#include "libstdtimeP.h"

/*
 * Converts a number of human readable date/time formats to epoch time
 */
epoch_t
stdtime_htoe (human_a)
char	*human_a;
{
	char	buf[256];

	stdtime_errno = STDTIME_NOERR;

	if (check_human (human_a) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}

	/* Zero out the buffer */
	memset((void*)buf, 0, sizeof(buf));

	/* place a '[' at the begining of the buffer */
	buf[0] = '[';

	/* 
	 * Copy the time string into the buffer, making sure not to over flow
	 * the buffer
	 */
	strncat(buf, human_a, sizeof(buf)-3);

	/* place a ']' at the end of the string */
	buf[strlen(buf)] = ']';

	/*
	 * Call stdtime_expr_time() and return whatever it returns
	 */
	return (stdtime_expr_time (buf));
}

/*
 * Fortran-callable version.
 */
void
stdtimehtoe_ (human, epoch, human_len)
char	*human;
double	*epoch;
int	human_len;
{
	if (epoch == (double *) NULL)
		return;

	if (human == (char *) NULL)
		return;
	
	*epoch = stdtime_htoe (human);
}
	
char	*
stdtime_etoh (epoch)
double	epoch;
{
	char	*human_a;

	stdtime_errno = STDTIME_NOERR;
	
	if (check_epoch (epoch) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_CHAR_ERR;
	}
	
	human_a = stdtime_format (epoch, "%h");

	return human_a;	
}

int
stdtime_etoh_r (epoch, human_ap, n)
double	epoch;
char	*human_ap;
int	n;
{
	stdtime_errno = STDTIME_NOERR;
	
	if (human_ap == (char *)NULL)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_INT_ERR;
	}
	
	return (stdtime_format_r (epoch, "%h", human_ap, n));
}

/*
 * Fortran-callable version.
 */
void
stdtimeetoh_ (epoch, human, human_len)
double	*epoch;
char	*human;
int	human_len;
{
	if (epoch == (double *) NULL)
		return;

	if (human == (char *) NULL)
		return;
	
	stdtime_etoh_r (*epoch, human, human_len);

	return;
}
	
