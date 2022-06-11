/*
 * NAME
 *  remove_filt_resp -- Remove butterworth filter response
 *
 * FILE
 *	remove_filt_resp.c
 *
 * SYNOPSIS
 *
 *	int
 *	remove_filt_resp (filt_type, filt_order, zero_phase, freq, hicut, 
 *		lowcut, samprate, filt_rolloff, amp)
 *	char	*filt_type;		(i) Filter type ("BP","LP","HP","BR")
 *	int	filt_order;		(i) Filter order (number of poles)
 *	int	zero_phase;		(i) Zero phase if 1
 *	double	freq;			(i) Frequency for response correction
 *	double	hicut;			(i) High cut frequency of filter
 *	double	lowcut;			(i) Low cut frequency of filter
 *	double 	samprate;		(i) Sampling rate of data (samples/sec)
 *	double	filt_rolloff;		(i) Filter roll-off beyond which a 
 *					    filter correction cannot be made
 *					    (decibels)
 *	double	*amp;			(i/o) Amplitude (nm) adjusted for
 *					      filter response
 *
 * DESCRIPTION
 *
 *	remove_filt_resp () removes the butterworth filter response 
 *	according to the filter specifications given using the
 *	butterworth_response() routine from libfilter.
 *	Error checks are made for filter bands relative to the Nyquist 
 *	frequency.  If the calculated amplitude correction is too 
 *	far outside the filter band as determined by the filter roll-off,
 *	an error occurs.  If an error occurs, the amplitude is left 
 *	unchanged, and -1 is returned.  
 *
 * SEE ALSO
 *	      
 *	libfilter.a
 *
 * AUTHOR
 *	Darrin Wahl
 *	
 */

#include "config.h"
#include <math.h>	/* For sqrt() */
#include <strings.h>
#include <string.h>     /* for strncmp() */
#include "logErrorMsg.h"
#include "libstring.h"  /* for mallocWarn   */
#include "libampP.h"	/* For protos */

int
remove_filt_resp( char *filt_type, int filt_order, int zero_phase, 
				  double freq, double hicut, double lowcut, 
				  double samprate, double filt_rolloff, double *amp )
{
  double	nyquist;
  double	min_resp;
  double	resp = 1.0;
  int	ret = -1;
  char		msg[128];

  /* Error checks */
  if (NO_FILTER (filt_type))
	{
	  ret = 0;
	  logErrorMsg( DFX_VERBOSE2, "remove_filt_resp: No filter applied. No removal of filter response\n" );
	  goto RETURN;
	}
  if (samprate <= 0.0) 
	{
	  snprintf( msg, sizeof(msg),"remove_filt_resp: Bad sample rate %f\n", samprate );
	  logErrorMsg( DFX_VERBOSE0, msg );
	  goto RETURN;
	}
  if (freq <= 0.0) 
	{
	  snprintf( msg, sizeof(msg), "remove_filt_resp: Bad frequency %f\n", freq);
	  logErrorMsg( DFX_VERBOSE0, msg );
	  goto RETURN;
	}

  /* Test on reasonable frequency band wrt to Nyquist frequency */
  nyquist = 0.5 * samprate;
  if( BANDPASS_FILTER (filt_type)  || 
	  LOWPASS_FILTER  (filt_type)  || 
	  BANDREJECT_FILTER (filt_type)  )
	{
	  if (hicut >= nyquist)
		{
		  snprintf( msg,sizeof(msg),"remove_filt_resp: High frequency cutoff %f >= Nyquist %f\n", hicut, nyquist);
		  logErrorMsg( DFX_VERBOSE0, msg );
		  ret = -1;
		  goto RETURN;
		}
	}
  else if (HIGHPASS_FILTER (filt_type))
	{
	  if (lowcut >= nyquist)
		{
		  snprintf( msg,sizeof(msg),"remove_filt_resp: Low frequency cutoff %f >= Nyquist %f\n", lowcut, nyquist);
		  logErrorMsg( DFX_VERBOSE0, msg );
		  ret = -1;
		  goto RETURN;
		}
	}		


  /* Get filter response */
  ret = butterworth_response (filt_type, filt_order, zero_phase, 
							  freq, hicut, lowcut, samprate, &resp);
  if (ret < 0)
	{
	  logErrorMsg( DFX_VERBOSE0, "remove_filt_resp: Error computing filter response\n" );
	  goto RETURN;
	}


  /* 
   * Determine if frequency is close enough to filter band
   * to allow filter correction.  If it is too far away from the 	
   * filter band, the amplitude will be magnified too much upon
   * correction.
   */
  min_resp = pow (10.0, -1.0 * filt_rolloff / 20.0);
  if (resp < min_resp)
	{
	  snprintf( msg, sizeof(msg),
				"remove_filt_resp: Measured period %f too far outside filter band (> -%f dB) to correct for filter response\n" ,
				1.0 / freq, filt_rolloff );
	  logErrorMsg( DFX_VERBOSE0, msg );
	  ret = -1;
	  goto RETURN;
	}
		

  /* Correct amplitude */
  *amp /= resp;

  ret = 0;

 RETURN:
	
  return (ret);

}	/* remove_filt_resp () */



