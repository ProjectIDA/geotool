/*
 * NAME
 * 
 *	calc_deltime
 *
 * FILE 
 *
 *	calc_deltime.c
 *
 * SYNOPSIS
 *
 *	double
 *	calc_deltime (snr, detsnr, max_snr, min_snr, max_deltim, min_deltim)
 *	double	snr;		   (i)	signal to noise ratio
 *	double	detsnr;		   (i)	detection snr threshold
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
 *	calc_deltime() computes the error in the time estimate for a 
 *	detection in two possible ways.  The first, and old style, assumes
 *	modelling error and measurement error (sec.) are defined as one in
 *	the deltim attribute of the arrival table as:
 *
 *	    max [1.0, 4.0 - (3.0 * log10 (snr / detsnr) / log10 (5.0))]
 *	
 *	This will be used if the min_snr argument is < 1.0.  Note that in
 *	this case deltim will always vary from 1.0 sec. to 4.0 sec.  To
 *	ensure backward compatibility of DFX where the rest of the system
 *	is NOT to employ phase-dependent modelling errors, this mode is
 *	preferred.
 *
 *	The second, and preferred, new style, uses the compute_deltim()
 *	function from libloc(3).
 *	This is the recommended mode when phase-depedent modelling errors
 *	are to be incorporated.
 * 
 * DIAGNOSTICS
 *
 *	Returns deltim attribute of arrival table.  Will return a deltim
 *	of -1.0 if a serious error was encountered (f.e., min_deltim >
 *	max_deltim).  If old style, will return an assumed composite of 
 *	measurement plus modelling error.  If new style, will return 
 *	measurement error only as defined by second equation defined above.
 *
 * FILES
 *
 *	None.
 *
 * NOTES
 * 
 *	Suggested values for the three latter arguments at this time (i.e.,
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
 *	handling facilities.  Only pertinent to new style implementation.
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *	Walter Nagy, May 24, 1996	Added capability limiting deltim
 *					contribution to measurement error only.
 *
 */


#include "config.h"
#include <math.h>	     /* For log10 */
#include "libloc.h"	     /* For compute_deltim() proto */
#include "loc_defs.h"	     /* For MAX */
#include "libdetectP.h"	     /* For proto and symbols      */
#include "logErrorMsg.h"

double
calc_deltime (double snr, double detsnr, double max_snr, double min_snr, 
			  double max_deltim, double min_deltim)
{
	double	deltim = 0.0;

        if (snr == -1.0) {
	  snr = DEFAULT_SNR;
	}

	if (min_snr < 1.0)
	{
		/*
		 * Old style
		 */
		if (detsnr > DFX_ZERO_VALUE)
		{
			deltim = 4.0 - (3.0 * log10 (snr / detsnr)/log10(5.0));
			deltim = MAX (deltim, 1.0);
		}
	}
	else
	{
		/*
		 * New style (measurement error only)
		 */
		deltim = compute_deltim (snr, max_snr, min_snr,
					 max_deltim, min_deltim);
		if (deltim < 0.0)
		{
			logErrorMsg( DFX_VERBOSE0, "calc_deltime: Error in compute_deltim()\n" );
		}
	}
	
	return (deltim);
}

		
