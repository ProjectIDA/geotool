
/*
 * Copyright (c) 1994-1997 Science Applications International Corporation.
 *

 * NAME
 *	mag_boot_strap -- Compute MLE network magnitude via boot strap method.

 * FILE
 *	mag_boot_strap.c

 * SYNOPSIS
 *	int
 *	mag_boot_strap (sm_sub, mcntrl, sm_count, num_boots, net_mag, sigma,
 *		        fmag1, sigmu, sig1, sigsig, verbose)
 *	SM_Sub	*sm_sub;
 *	Mag_Cntrl	*mcntrl;
 *	int	sm_count;
 *	int	num_boots;
 *	double	net_mag;
 *	double	sigma;
 *	double	*fmag1;
 *	double	*sigmu;
 *	double	*sig1;
 *	double	*sigsig;
 *	int	verbose;

 * DESCRIPTION
 *	Function.  Use bootstraping method to find many (mu, sigma) pairs 
 *	which individual maximize the likelihood function obtained by random
 *	samplings of the input amplitude data.  Better error statistics are
 *	obtained in this way, although it is best not to use the network 
 *	magnitude that results from here as a general rule.  This is because
 *	one typically does not want the MLE magnitude to change between calls
 *	using the exact same data.

 * DIAGNOSTICS
 *	If maximum number of iterations is exceeded in mag_max_lik() then an 
 *	error -2 condition will be returned upon compeletion.

 * FILES
 *	None.

 * NOTES
 * 	If fewer than num_boots re-samples are needed for convergence, 
 *	then this routine will break from main loop early.

 * SEE ALSO
 *	Local function, mag_max_lik().

 * AUTHORS
 *	K. McLaughlin,		Created.
 *	Hans Israelsson, 3/89,	Updating of original subroutine.
 *	Walter Nagy,	 5/94,	Converted from FORTRAN to C, May 1994.
 *	Walter Nagy,	10/97,	Reflects changes required to accomodate new
 *				amplitude schema and magnitude model.  Change
 *				of file name to mag_boot_strap.c
 *	Doug Brumbaugh,	10/99,  Modified random number generator.
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "magp.h"

extern	double	drand48();
extern	void	srand48();
extern	long	lrand48();

static	Bool	first_call = FALSE;


int
#ifdef __STDC__
mag_boot_strap (SM_Sub *sm_sub, Mag_Cntrl *mcntrl, int sm_count, int num_boots,
		double net_mag, double sigma, double *fmag1, double *sigmu,
		double *sig1, double *sigsig, int verbose)
#else
mag_boot_strap (sm_sub, mcntrl, sm_count, num_boots, net_mag, sigma,
		fmag1, sigmu, sig1, sigsig, verbose)

SM_Sub	*sm_sub;
int	sm_count;
int	num_boots;
double	net_mag;
double	sigma;
double	*fmag1;
double	*sigmu;
double	*sig1;
double	*sigsig;
int	verbose;
#endif
{
	int	i, j;
	int	index, isig, num_data, num_signals;
	int	mlk_code = 0;
	long int ran1;
	double	ave, chk, fmag0, fmag2, num_boot_resamples, sig0, sig2;
	SM_Sub	*t_sm_sub = (SM_Sub *) NULL;
	SM_Sub	*b_sm_sub = (SM_Sub *) NULL;
	struct timeval  t;
	struct timezone tz;



	fmag0  = 0.0;
	*fmag1 = 0.0;
	fmag2  = 0.0;
	sig0   = 0.0;
	*sig1  = 0.0;
	sig2   = 0.0;
	ave    = 0.0;

	/*
	 * Compute the average of the valid signal magnitudes (i.e., not 
	 * clipped or noise measurements).
	 */

	for (isig = 0, i = 0; i < sm_count; i++)
	{
	    if (STREQ (sm_sub[i].magdef, "d") && 
		sm_sub[i].sig_type == MEAS_SIGNAL)
	    {
		++isig;
		ave += sm_sub[i].magnitude;
	    }
	}
	if (isig != 0) 
	    ave /= (double)isig;
	else
	    ave = 0.0;

	t_sm_sub = UALLOCA (SM_Sub, sm_count);
	for (num_data = 0, i = 0; i < sm_count; i++)
	{
	    if (STREQ (sm_sub[i].magdef, "d"))
	    {
		strcpy (t_sm_sub[num_data].magdef, "d");
		t_sm_sub[num_data].magnitude = sm_sub[i].magnitude;
		t_sm_sub[num_data].sig_type = sm_sub[i].sig_type;
		t_sm_sub[num_data].wt = sm_sub[i].wt;
		++num_data;
	    }
	}
	b_sm_sub = UALLOCA (SM_Sub, num_data);

	if (! first_call)
	{
	    /*
	     * Setting up random generators
	     */
/*
	    if ((ran1 = (time_t) stdtime_get_epoch()) == STDTIME_DOUBLE_ERR)
	    {
		if (stdtime_errno != STDTIME_NOERR)
		{
		    fprintf (stderr, "mag_boot_strap: Cannot generate epoch time for random number generation!\n");
		    return (ERR);
		}
 	    }
*/
	    if(gettimeofday(&t,&tz) == -1)
	    {
		    fprintf (stderr, "mag_boot_strap: Cannot generate epoch time for random number generation!\n");
		    return (ERR);
	    }
	    ran1 = (long int) t.tv_sec;
	    srand48(ran1);
	    srand48(lrand48());
	    first_call = TRUE;
	}

	for (num_boot_resamples = 0.0, j = 0; j < num_boots; j++)
	{
	    /*
	     * Resample the data randomly, but pick up at least 1 valid 
	     * data point, if any valid signals are available.  
	     * See Sanity check below.
	     */

try_rand_num_gen_again:

	    for (num_signals = 0, i = 0; i < num_data; i++)
	    {
		index = (int)(drand48()*(double)num_data);
		if (index == num_data)    /* Memory check */
		    --index;
		b_sm_sub[i] = t_sm_sub[index];
		if (b_sm_sub[i].sig_type == MEAS_SIGNAL)
		    ++num_signals;
	    }

	    /*
	     * Sanity check: Make sure at least 1 valid signal was 
	     * 		 found during random number generation
	     *		 loop immediately above.
	     */

	    if (isig > 0 && num_signals == 0)
		goto try_rand_num_gen_again;

	    /*
	     * Call the maximum likelihood estimation routine, mag_max_lik(),
	     * without printing.
	     */

	    mlk_code = mag_max_lik (b_sm_sub, mcntrl, num_data, ave, &net_mag,
				    &sigma, verbose);

	    if (verbose > 0)
		fprintf (stdout, " MLE Magnitude: %6.3f / Std. Dev.: %6.3f\n",
				 net_mag, sigma);

	    *fmag1 += net_mag;
	    fmag2  += net_mag*net_mag;
	    *sig1  += sigma;
	    sig2   += sigma*sigma;

	    /*
	     * Consider convergence reached if > 10 iterations and either
	     * the network Mb or the S.D. of the network Mb change by
	     * < 0.01 magnitude units.
	     */

	    num_boot_resamples = (double)j+1;
	    if (j > 10 && (fabs(*fmag1-fmag0)/num_boot_resamples) < 0.01)
		break;
	    fmag0 = *fmag1;
	    if (j > 10 && (fabs(*sig1-sig0)/num_boot_resamples) < 0.01)
		break;
	    sig0 = *sig1;
	}

	/*
	 * Compute final averages and standard deviations.
	 */

	*fmag1 /= num_boot_resamples;
	fmag2  /= num_boot_resamples;
	*sig1  /= num_boot_resamples;
	sig2   /= num_boot_resamples;
	chk = fmag2-(*fmag1*(*fmag1));
	if (chk > 0.0)
	    *sigmu = sqrt(chk);
	else
	    *sigmu = 0.0;

	/*
	 * If sglim1 = sglim2, then sigsig will be 0.0 and sqrt may
	 * over/underflow on some machines due to numerical round-off.
	 */

	*sigsig = sig2 - (*sig1*(*sig1));
	if (mcntrl->sglim1 - mcntrl->sglim2 == 0.0) 
	    *sigsig = 0.0;
	if (*sigsig > 0.0) 
	    *sigsig = sqrt(*sigsig);

	if (verbose > 0)
	{
	    fprintf (stdout, "\nBootstrap MLE Mag: %6.3f (%6.3f)\n",
			     *fmag1, *sigmu);
	    fprintf (stdout, "Bootstrap MLE Sigma: %6.3f (%6.3f)\n",
			     *sig1, *sigsig);
	    fprintf (stdout, "Number of Bootstrap resamples: %d\n", j);
	}

	return (mlk_code);
}


