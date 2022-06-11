/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *
 * NAME
 * 
 *	compute_deltim
 *
 * FILE 
 *
 *	compute_deltim.c
 *
 * SYNOPSIS
 *
 *	double
 *	compute_deltim (snr, max_snr, min_snr, max_deltim, min_deltim)
 *	double	snr;		   (i)	signal to noise ratio
 *	double	max_snr;	   (i)	maximum SNR value to be used as an
 *					upper limit for setting deltim
 *	double	min_snr;	   (i)	minimum SNR value to be used as a
 *					lower limit for setting deltim
 *	double	max_deltim; 	   (i)	maximum deltim to be used as a
 *					(worst case) measurement error limit
 *	double	min_deltim;	   (i)	minimum deltim to be used as a
 *					(best case) measurement error limit
 *	
 * DESCRIPTION
 *
 *	compute_deltim() computes the error in the time estimate for a 
 *	detection.  This estimate assumes only the measurement error is 
 *	represented by deltim as:
 *
 *	    max [min_deltim, 
 *		 max_deltim - 
 *		   (deltim_diff * log10(snr/min_snr) / log10(max_snr/min_snr))]
 *
 *	    where
 *
 *		deltim_diff = max_deltim - min_deltim
 *
 *	This function transitions logarithmically from max_deltim at
 *	min_snr to min_deltim at max_snr.
 *
 * DIAGNOSTICS
 *
 *	Returns deltim attribute of arrival table as the measurement error 
 *	only as defined by equation defined above.  Returns a value of
 *	of -1.0 if a serious error was encountered (eg, min_deltim >
 *	max_deltim). 
 *
 * FILES
 *
 *	None.
 *
 * NOTES
 * 
 *	Suggested values for the four latter arguments at this time (i.e.,
 *	May 1996) are:
 *
 *		max_snr = 50.0
 *		min_snr = 5.0
 *		max_deltim = 1.0
 *		min_deltim = 0.1
 *
 * SEE ALSO
 *
 *	libloc(3) library function, get_model_error(), for access to modelling
 *	error component of total error used by locator and other travel-time
 *	handling facilities.
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *	Walter Nagy, May 24, 1996	Added capability limiting deltim
 *					contribution to measurement error only.
 *
 */

#include "config.h"
#include <math.h>	/* For log10() prototype */
#include "libloc.h"	/* For compute_deltim() prototype */

double
compute_deltim (double snr, double max_snr, double min_snr, 
		 double max_deltim, double min_deltim)
{
	double	deltim = -1.0;
	
	if ((max_deltim < min_deltim) ||
	    (max_snr < min_snr))
	{
		deltim = -1.0;
	}
	else if (snr < min_snr)
	{
		deltim = max_deltim;
	}
	else if (snr > max_snr)
	{
		deltim = min_deltim;
	}
	else
	{
		deltim = max_deltim - 
			((max_deltim - min_deltim) * 
			 log10 (snr/min_snr) / log10 (max_snr/min_snr));

		deltim = (deltim > min_deltim) ? deltim : min_deltim;
	}

	return (deltim);
}

		
