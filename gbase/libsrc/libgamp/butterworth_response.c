/*
 * NAME
 *	butterworth_response
 * 
 * FILE 
 *	butterworth_response.c
 *
 * SYNOPSIS
 *
 *	int
 *	butterworth_response (filt_type, filt_order, zero_phase, freq, hicut, lowcut, resp)
 *	char	*filt_type;	(i) filter type
 *	int	filt_order;	(i) filter order
 *	int	zero_phase;	(i) true for non causal
 *	double	freq;		(i) frequency in cycles/sec
 *	double	hicut;		(i) high frequency cutoff of filter
 *	double	lowcut;		(i) low frequency cutoff of filter
 *	double	*resp;		(o) filter response
 *
 * DESCRIPTION
 *
 *  Computes the butterworth filter response corresponding to the input
 *	frequency and filter band frequencies.  Returns -1 on error, 0 otherwise.
 *
 * DIAGNOSTICS
 *
 *	None.
 *
 * AUTHOR
 *
 * 	Darrin Wahl (from Petr Firbas, CMR)
 *
 */

#include "config.h"
#include <string.h>
#include <math.h>
#include "libampP.h"

int
butterworth_response( char *filt_type, int filt_order, int zero_phase, double freq, 
					  double hicut, double lowcut, double samprate, double *resp   )
{
	double	nyquist;		    /* Nyquist frequency */
	double 	sampper;		    /* sample interval */
	double	butterworth = 1.0;	/* filter correction */
	double 	freq_a;			    /* frequencies transformed to analog */
	double	hicut_a;	
	double	lowcut_a;
	double 	omega_norm_LP;		/* freq. normalized to standard LP */
	
	/* Error checks */
	nyquist = 0.5 * samprate;

	if (samprate <= 0.0 || freq <= 0.0 || freq > nyquist ||
	    hicut > nyquist || lowcut > nyquist || lowcut >= hicut)
	  {
		return (-1);
	  }
	
	if( !BANDPASS_FILTER(filt_type)   && 
		!HIGHPASS_FILTER(filt_type)   &&
	    !LOWPASS_FILTER(filt_type)    && 
		!BANDREJECT_FILTER(filt_type)    )
	{
		return (-1);
	}
	if ((filt_order > MAX_FILTER_ORDER) || (filt_order < 1))
	{
		return (-1);	
	}

	/* Use sampling period */
	sampper = 1.0 / samprate;

	/* Convert the "digital frequencies to "analog" ones */
	hicut_a  = tan (M_PI*hicut*sampper)  / (M_PI*sampper);
	lowcut_a = tan (M_PI*lowcut*sampper) / (M_PI*sampper);
	freq_a   = tan (M_PI*freq*sampper)   / (M_PI*sampper);

	/* 
	 * Convert the signal "analog" frequency to that of standard 
	 * (normalized) LP filter 
	 */ 
	omega_norm_LP = 0.0;
	if (BANDPASS_FILTER (filt_type))
	{
		omega_norm_LP = (freq_a*freq_a - hicut_a*lowcut_a) / freq_a / 
			(hicut_a - lowcut_a);
	}
	else if (HIGHPASS_FILTER (filt_type))
	{
		omega_norm_LP = lowcut_a / freq_a; 
	}
	else if (LOWPASS_FILTER (filt_type))
	{
		omega_norm_LP = freq_a / hicut_a;
	}
	else if (BANDREJECT_FILTER (filt_type))
	{
		omega_norm_LP =  (hicut_a - lowcut_a) * 
			(freq_a / (freq_a*freq_a - hicut_a*lowcut_a));
	}		

	/* Compute the amplitude adjustment factor */
	butterworth = pow (omega_norm_LP, (double) 2.0*filt_order);
	butterworth = 1.0 / (1.0 + butterworth);
	butterworth = sqrt (fabs (butterworth));

	/* For zero phase filter use the square of the correction coefficient*/
	if (zero_phase)
	{
		butterworth *= butterworth;
	}

	/* Return amplitude adjustment */
	*resp =  butterworth;

	return(0);
}
