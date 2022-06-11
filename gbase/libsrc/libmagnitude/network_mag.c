
/*
 * Copyright (c) 1994-1998 Science Applications International Corporation.
 *

 * NAME
 *	network_mag -- Calculate a network magnitude.

 * FILE
 *	network_mag.c

 * SYNOPSIS
 *	int
 *	network_mag (sm_sub, mcntrl, sm_count, verbose, mag, sigma, sdav,
 *		     num_amps_used)
 *	SM_Sub	*sm_sub;
 *	Mag_Cntrl	*mcntrl;
 *	int	sm_count;
 *	int	verbose;
 *	double	*mag;
 *	double	*sigma;
 *	double	*sdav;
 *	int	*num_amps_used;

 * DESCRIPTION
 *	Function.  Calculate network-average or maximum-likelihood estimated
 *	(MLE) magnitude as requested by the calling routine.

 * DIAGNOSTICS
 *	The following MLE conditions may be returned (mlk_code):

 *	  -1:	No signal, noise or clipped data available to continue.
 *	  -2:	Maximum number of iterations exceeded in MLE routine, 
 *		mag_max_lik().
 *	  -3:	Maximum number of iterations exceeded in routine, 
 *		only_bound_amps().  This indicates that the only event-based
 *		"noise" amplitude data existed for computing this potential 
 *		magnitude, and further, that these amplitudes could not be 
 *		likely associated with one another.
 *	  -4:	Maximum number of iterations exceeded in routine, 
 *		only_bound_amps().  This indicates that the only "clipped" 
 *		amplitude data existed for computing this potential 
 *		magnitude, and further, that these amplitudes could not be 
 *		likely associated with one another.
 *	   0:	MLE calculation was successful.
 *	   1:	This code indicates that the only "noise" amplitude data 
 *		existed for computing this magnitude.
 *	   2:	This code indicates that the only "clipped" amplitude data 
 *		existed for computing this magnitude.

 *	Negative codes are errors while positive codes only indicate the
 *	absence of signal amplitudes (1: Noise only; 2: Clipped only).

 * FILES
 *	None.

 * NOTES
 * 	There are a number of 3 element arrays used here.  In all cases, 
 *	measured amplitude (signal) data is stored in the 1st element;
 *	clipped amplitudes in the 2nd element; and noise amplitudes in
 *	the 3rd element.

 * SEE ALSO
 *	None.

 * AUTHORS
 *	K. McLaughlin,		Created.
 *	Hans Israelsson, 3/89,	Updating of original subroutine.
 *	Walter Nagy,	 5/94,	Converted from FORTRAN to C, May 1994.
 *	Walter Nagy,	10/97,	Reflects changes required to accomodate new
 *				amplitude schema and magnitude model.  Change
 *				of file name to network_mag.c
 *	Doug Brumbaugh,	 9/1999	Standard deviation of network magnitudes
 *				now calculated with station weights.
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "magp.h"


int
#ifdef __STDC__
network_mag (SM_Sub *sm_sub, Mag_Cntrl *mcntrl, int sm_count, int verbose,
	     double *mag, double *sigma, double *sdav, int *num_amps_used)
#else
network_mag (sm_sub, mcntrl, sm_count, verbose, mag, sigma, sdav,
	     num_amps_used)

SM_Sub	*sm_sub;
Mag_Cntrl	*mcntrl;
int	sm_count;
int	verbose;
double	*mag;
double	*sigma;
double	*sdav;
int	*num_amps_used;
#endif
{
	int	i, j;
	int	mlk_code = 0;
	int	sub_count = 0;
	int	num[NUM_AMP_TYPES];
	double	ave[NUM_AMP_TYPES], sig[NUM_AMP_TYPES], stdev[NUM_AMP_TYPES];
//	double	stdevm[NUM_AMP_TYPES];
	double	sum[NUM_AMP_TYPES], sumsq[NUM_AMP_TYPES], wave[NUM_AMP_TYPES];
	double	mag_wgt[NUM_AMP_TYPES];
	double	arg, mag_sqrd, dnum, sigma_sqrd, sigmax, wt_sqrd, y1;

	SM_Sub	*sm_bound = (SM_Sub *) NULL;


	*num_amps_used = 0;
	*sigma = 0.0;
	*sdav  = 0.0;
	*mag   = 0.0;

	if (mcntrl->sglim1 == mcntrl->sglim2) 
	    *sigma = mcntrl->sgbase;

	y1 = *sigma;
	sigma_sqrd = y1*y1;

	/* Initialize for counting types of data flags */

	for (i = 0; i < NUM_AMP_TYPES; i++)
	{
	    num[i]    = 0;
	    sum[i]    = 0.0;
	    sumsq[i]  = 0.0;
	    sig[i]    = 0.0;
	    wave[i]   = 0.0;
	    ave[i]    = 0.0;
	    stdev[i]  = 0.0;
//	    stdevm[i] = 0.0;
	    mag_wgt[i] = 0.0;
	}

	/*
	 * Count the number of each type of data (i.e., measured signal,
	 * clipped, or non-detected.  Also, compute several sums which
	 * will be used to compute the average and standard deviations.
	 */

	for (i = 0; i < NUM_AMP_TYPES; i++)
	{
	    for (j = 0; j < sm_count; j++)
	    {
		if (STREQ (sm_sub[j].magdef, "d") && sm_sub[j].sig_type == i)
		{
		    ++num[i];
		    sum[i] += sm_sub[j].magnitude;
		    mag_sqrd = sm_sub[j].magnitude * sm_sub[j].magnitude;
		    sumsq[i] += mag_sqrd;
		    wt_sqrd = sm_sub[j].wt * sm_sub[j].wt;
		    if (sm_sub[j].wt != 0.0)
		    {
			/*
			 *  These sums are used to compute the weighted
			 *  average network magnitude and associated
			 *  standard deviation
			 */
			if (y1 != 0.0)  /* Fixed a priori network stdev */
			{
			    wave[i] += sm_sub[j].magnitude / sigma_sqrd;
			    mag_wgt[i] += mag_sqrd / sigma_sqrd;
			    sig[i]  += 1.0/sigma_sqrd;
			}
			else		/* Variable a priori network stdev */
			{
			    wave[i] += sm_sub[j].magnitude / wt_sqrd;
			    mag_wgt[i] += mag_sqrd / wt_sqrd;
			    sig[i]  += 1.0/wt_sqrd;
			}
		    }
		}
	    }
	}

	if (num[MEAS_SIGNAL] == 0 && num[NON_DETECT] == 0 && num[CLIPPED] == 0)
	    return (-1);	/* No data available to continue */

	/*
	 * Compute arithmetic mean and standard deviation for each data type.
	 * Make provision for negative variance due to round-off error if
	 * variance is zero (set it equal to 10**-20).
	 */

	for (i = 0; i < NUM_AMP_TYPES; i++)
	{

	    /*
	     *  For the case when there is only one measurement for a given
	     *  data type, then the standard deviation is simply the
	     *  baseline a priori standard deviation from the mdf file.
	     */
	    if (num[i] == 1)
	    {
		ave[i] = sum[i];
		if (sig[i] != 0.0)
		    wave[i] /= sig[i];
		else
		    wave[i] = ave[i];
		stdev[i] = mcntrl->sgbase;
	    }

	    else if (num[i] > 1)
	    {
		dnum = (double)num[i];
		ave[i] = sum[i]/dnum;
		if (sig[i] != 0.0)
		{
		    /*
		     *  Compute the weighted average network magnitude (wave)
		     *  and the variance of the weighted average network
		     *  magnitude (arg) when station standard deviations,
		     *  or weights (sm_sub[j].wt), are available.
		     */
		    wave[i] /= sig[i];
		    arg = (mag_wgt[i]/sig[i] - wave[i]*wave[i]) *
			   dnum / (dnum - 1);
		}
		else
		{
		    /*
		     *  If no station standard deviations are available
		     *  (or are zero), then compute the average network
		     *  magnitude (wave) and variance (arg) using 
		     *  standard, time-honored formulas.
		     */
		    wave[i] = ave[i];
		    arg = (sumsq[i] - ave[i]*ave[i]*dnum) / (dnum - 1);
		}
		if (arg <= 0.0)
		    arg = 1.0E-20;

		/*
		 *  Compute standard deviation (stdev) and standard
		 *  deviation of the mean (stdevm)
		 */
		stdev[i]  = sqrt(arg);
//		stdevm[i] = stdev[i]/sqrt(dnum);
	    }
	}

	if (mcntrl->algo_code == NET_AVG)
	{
	    /*
	     * Store average value and associated standard deviation in mag
	     * and sigma, respectively.  No maximum-likelihood estimate (MLE) 
	     * requested.
	     */

	    *num_amps_used = num[MEAS_SIGNAL];
	    *mag   = wave[MEAS_SIGNAL];

            /*
             *  Constrain the computed standard deviation within
             *  the upper and lower bounds specified in the mdf file
             *  when network averages are computed, unless the upper
	     *  and lower bounds are equal.
             */ 
	    *sigma = stdev[MEAS_SIGNAL];
	    if (mcntrl->sglim1 != mcntrl->sglim2)
	    {
		if (*sigma < mcntrl->sglim1)
		{
		    if (verbose > 0)
			fprintf (stdout, "Warning: Network stdev = %.3f < lower bound in mdf file = %.2f ->\nSetting network sigma = %.2f\n", *sigma, mcntrl->sglim1, mcntrl->sglim1);
		    *sigma = mcntrl->sglim1;
		}
		else if (*sigma > mcntrl->sglim2)
		{
		    if (verbose > 0)
			fprintf (stdout, "Warning: Network stdev = %.3f > upper bound in mdf file = %.2f ->\nSetting network sigma = %.2f\n", *sigma, mcntrl->sglim2, mcntrl->sglim2);
		    *sigma = mcntrl->sglim2;
		}
	    }

	    if (*num_amps_used > 0)
		*sdav = *sigma/sqrt((double) *num_amps_used);
	    else
		*sdav = *sigma;
	}
	else
	{
	    /*
	     * Attempt to calculate a maximum-likelihood estimate (MLE) for
	     * given input data.  If only non-detecting (mlk_code = 1) or 
	     * clipped (mlk_code = 2) data exists, then this is not an 
	     * error, but calling program should know.
	     */

	    if (num[MEAS_SIGNAL] == 0) 
	    {
		if (num[CLIPPED] == 0) 
		{
		    mlk_code = 1;	/* Only non-detecting amps available */
		    sm_bound = UALLOC (SM_Sub, sm_count);
		    sub_count = 0;
		    for (i = 0; i < sm_count; i++)
		    {
			if (STREQ (sm_sub[i].magdef, "d") && 
			    sm_sub[i].sig_type == NON_DETECT)
			{
			    strcpy (sm_bound[sub_count].magdef, "d");
			    sm_bound[sub_count].sig_type = sm_sub[i].sig_type;
			    sm_bound[sub_count].wt = sm_sub[i].wt;
			    sm_bound[sub_count].magnitude = sm_sub[i].magnitude;
			    ++sub_count;
			}
		    }
		    if ((only_bound_amps (sm_bound, mcntrl, sub_count,
					  ave[NON_DETECT], -1, *sigma,
					  mag, &sigmax)) != OK)
		    {
			UFREE (sm_bound);
			return (-3);
		    }
		    UFREE (sm_bound);
		    *sigma = sigmax;
		}
		else if (num[NON_DETECT] == 0) 
		{
		    mlk_code = 2;	/* Only clipped amps available */
		    sm_bound = UALLOC (SM_Sub, sm_count);
		    sub_count = 0;
		    for (i = 0; i < sm_count; i++)
		    {
			if (STREQ (sm_sub[i].magdef, "d") && 
			    sm_sub[i].sig_type == CLIPPED)
			{
			    strcpy (sm_bound[sub_count].magdef, "d");
			    sm_bound[sub_count].sig_type = sm_sub[i].sig_type;
			    sm_bound[sub_count].wt = sm_sub[i].wt;
			    sm_bound[sub_count].magnitude = sm_sub[i].magnitude;
			    ++sub_count;
			}
		    }
		    if ((only_bound_amps (sm_bound, mcntrl, sub_count,
					  ave[CLIPPED], 1, *sigma,
					  mag, &sigmax)) != OK)
		    {
			UFREE (sm_bound);
			return (-4);
		    }
		    UFREE (sm_bound);
		    *sigma = sigmax;
		}
		else
		{
		    /*
		     * A mixture of noise and clipped data only (a very
		     * unlikely scenario, but we will consider it anyway).
		     */

		    ave[MEAS_SIGNAL]   = (ave[CLIPPED]+ave[NON_DETECT]) / 2.0;
		    stdev[MEAS_SIGNAL] = 
				(stdev[CLIPPED]+stdev[NON_DETECT]) / 2.0;
		    *mag = ave[NON_DETECT];
		    mlk_code = 
		    mag_max_lik (sm_sub, mcntrl, sm_count, ave[NON_DETECT],
				 mag, sigma, verbose); 
		}
	    }
	    else
	    {
		/*
		 * If any measured signal amplitudes exists, we will come
		 * here.
		 */

		*mag = ave[MEAS_SIGNAL];
		mlk_code = 
		mag_max_lik (sm_sub, mcntrl, sm_count, ave[MEAS_SIGNAL],
			     mag, sigma, verbose);
	    }

	    *num_amps_used = num[MEAS_SIGNAL] + num[CLIPPED] + num[NON_DETECT];
	    if (*num_amps_used > 0.0)
		*sdav = *sigma/sqrt((double) *num_amps_used);
	    else
		*sdav = *sigma;
	}

	return (mlk_code);	/* Only meaningful for MLE calculations */
}


