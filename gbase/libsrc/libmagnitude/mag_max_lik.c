
/*
 * Copyright (c) 1994-1997 Science Applications International Corporation.
 *

 * NAME
 *	mag_max_lik -- Maximum likelihood estimation routine for magnitude.

 * FILE
 *	mag_max_lik.c

 * SYNOPSIS
 *	int
 *	mag_max_lik (sm_sub, mcntrl, sm_count, ave, net_mag, sigma, verbose)
 *	SM_Sub	*sm_sub;
 *	Mag_Cntrl	*mcntrl;
 *	int	sm_count;
 *	double	ave;
 *	double	*net_mag;
 *	double	*sigma;
 *	int	verbose;

 * DESCRIPTION
 *	Function.  Find the (mu, sigma) pair which maximizes the likelihood
 *	function.  Do this by optimizing the (mu, sigma) estimator by the
 *	EM ALGORITHM.

 * DIAGNOSTICS
 *	If maximum number of iterations is exceeded an error 1 condition
 *	will be returned upon compeletion.

 * FILES
 *	None.

 * NOTES
 * 	None.

 * SEE ALSO
 *	None.

 * AUTHORS
 *	K. McLaughlin,		Created.
 *	Hans Israelsson, 3/89,	Updating of original subroutine.
 *	Walter Nagy,	 5/94,	Converted from FORTRAN to C, May 1994.
 *	Walter Nagy,	10/97,	Reflects changes required to accomodate new
 *				amplitude schema and magnitude model.  Change
 *				of file name to mag_max_lik.c
 */



#include <stdio.h>
#include <math.h>
#include "magp.h"

#define	MAX_ITER 200


int
#ifdef __STDC__
mag_max_lik (SM_Sub *sm_sub, Mag_Cntrl *mcntrl, int sm_count, double ave,
	     double *net_mag, double *sigma, int verbose)
#else
mag_max_lik (sm_sub, mcntrl, sm_count, ave, net_mag, sigma, verbose)

SM_Sub	*sm_sub;
Mag_Cntrl	*mcntrl;
int	sm_count;
double	ave;
double	*net_mag;
double	*sigma;
int	verbose;
#endif
{
	int	i;
	int	iter = 0;
	int	iterer = 0;
	int	nclips, nnodet, nobs, num_amps;
	double	wtot = 0.0;
	double	dsig, dx, p1, p2, s1, sigma1, sigma2, test;
	double	x, xm, x1, y, y1, z;


	p1 = 1.0/sqrt(2.0*M_PI);
	p2 = M_SQRT1_2;

	for (i = 0, nobs = 0; i < sm_count; i++)
	{
	    if (STREQ (sm_sub[i].magdef, "d") && 
		sm_sub[i].sig_type == MEAS_SIGNAL && sm_sub[i].wt != 0.0)
	    {
		wtot += 1.0/(sm_sub[i].wt*sm_sub[i].wt);
		++nobs;
	    }
	}
	if (wtot > 0.0) 
	    wtot = (double)nobs/wtot;
	if (wtot == 0.0) 
	    wtot = 1.0;

	/*
	 * "x" is mu, "y" is sigma
	 * pick initial values from which to begin the iterative procedure
	 */

	if (mcntrl->sgbase >= mcntrl->sglim1 &&
	    mcntrl->sgbase <= mcntrl->sglim2)
	    y = mcntrl->sgbase;
	else
	    y = 0.5 * (mcntrl->sglim1 + mcntrl->sglim2);

	x  = ave;
	y1 = *sigma;	/* For local use */

	/*
	 * jg
	 * jg  This requires net_mag to be initialized to something  !!!
	 * jg 
	 */

	if (fabs(ave - *net_mag) <= 1.0) 
	    x = *net_mag;
	if (y1 <= 0.0 || y1 >= mcntrl->sglim2) 
	    y1 = y;
	if (y1 < mcntrl->sglim1) 
	    y1 = mcntrl->sglim1;

	/* EM loop starts here */

	for (iter = 0; iter < MAX_ITER; iter++)
	{
	    /*
	     * Update mean estimate EQUATIONS (5), (6), and (7) on PAGE I-7 
	     * from VSC-TR-82-12
	     */

	    x1 = 0.0;
	    nobs = nclips = nnodet = 0;
	    for (i = 0; i < sm_count; i++)
	    {
		if (STREQ (sm_sub[i].magdef, "d"))
		{
		    if (sm_sub[i].sig_type == MEAS_SIGNAL)
		    {
			++nobs;
			if (sm_sub[i].wt != 0.0) 
			{
			    x1 += sm_sub[i].magnitude*wtot/
				      (sm_sub[i].wt*sm_sub[i].wt);
			}
			else
			    x1 += sm_sub[i].magnitude;
		    }
		    else if (sm_sub[i].sig_type == CLIPPED)
		    {
			++nclips;
			if (sm_sub[i].wt != 0.0) 
			    s1 = sm_sub[i].wt;
			else
			    s1 = y;
			z = (sm_sub[i].magnitude-x)/s1;

			/*
			 * jg   Use limits for  stability on bad data
			 */
	
			if (z > 4)
			    x1 += x+y1*z;
			else
			    x1 += x+y1*p1*exp(-0.5*z*z)/(0.5*(1.0+erf(-p2*z)));
		    }
		    else if (sm_sub[i].sig_type == NON_DETECT)
		    {
			++nnodet;
			if (sm_sub[i].wt != 0.0)
			    s1 = sm_sub[i].wt;
			else
			    s1 = y;
			z = (sm_sub[i].magnitude-x)/s1;

			/*
			 * jg   "z" is negative...
			 */

			if (z < -4)
			    x1 += x+y1*z;
			else
			    x1 += x-y1*p1*exp(-0.5*z*z)/(0.5*(1.0+erf(p2*z)));
		    }
		}
	    }

	    num_amps = nobs + nclips + nnodet;
	    x1 = x1/(double)num_amps;
	    dx = x1 - x;

	    /*
	     *  Update sigma (y1) EQUATION A16 from VSC-TR-82-12
	     *  Blandford and Shumway (1982)
	     */

	    sigma2 = y1*y1;
	    sigma1 = sigma2*(double)(num_amps-nobs);

	    for (i = 0; i < sm_count; i++)
	    {
		if (STREQ (sm_sub[i].magdef, "d"))
		{
		    if (sm_sub[i].sig_type == MEAS_SIGNAL)
		    {
			xm = sm_sub[i].magnitude-x;
			if (sm_sub[i].wt != 0.0)
			{
			    sigma1 += xm*xm * wtot /
				      (sm_sub[i].wt*sm_sub[i].wt);
			}
			else
			    sigma1 += xm*xm;
		    }
		    else if (sm_sub[i].sig_type == CLIPPED)
		    {
			if (sm_sub[i].wt != 0.0) 
			    s1 = sm_sub[i].wt;
			else
			    s1 = y;
			z = (sm_sub[i].magnitude-x)/s1;
	
			/*
			 * jg   Use limits for  stability on bad data
			 */
	
			if (z > 4.0)
			    sigma1 += sigma2*z*z;
			else
			    sigma1 += sigma2*z*p1*exp(-0.5*z*z)/
						 (0.5*(1.0+erf(-p2*z)));
		    }
		    else if (sm_sub[i].sig_type == NON_DETECT)
		    {
			if (sm_sub[i].wt != 0.0) 
			    s1 = sm_sub[i].wt;
			else
			    s1 = y;
			z = (sm_sub[i].magnitude-x)/s1;

			/*
			 * jg   NEW:
			 */

			if (z < -4.0)
			    sigma1 += sigma2*z*z;
			else
			    sigma1 += -sigma2*z*p1*exp(-0.5*z*z)/
						  (0.5*(1.0+erf(p2*z)));
		    }
		}
	    }

	    /*
	     *  Use the baseline value as the standard deviation
	     *  for the case where there is only one amplitude
	     */
	    if (num_amps <= 1)
		sigma1 = mcntrl->sgbase;
	    else
	        sigma1 = sqrt (sigma1 / ((double) num_amps - 1.0));

	    /* Limit sigma (y1) to interval (sglim1,sglim2) */

	    if (sigma1 < mcntrl->sglim1) 
		sigma1 = mcntrl->sglim1;
	    if (sigma1 > mcntrl->sglim2) 
		sigma1 = mcntrl->sglim2;

	    /* 
	     * Test for convergence!  If successful, break from loop. 
	     * Make sure at least 10 iterations have been performed.
	     */

	    dsig = sigma1 - y1;
	    test = fabs(dsig) + fabs(dx);

	    x = x1;
	    y1 = sigma1;
	    y = y1;

	    if (iter > 10 && test < 0.0001)
		break;		/* Successful convergence */
	}

	*net_mag = x;
	*sigma   = y;

	if (iter >= MAX_ITER)
	{
	    iterer = -2;
	    if (verbose > 0)
		fprintf (stderr, " EM ESTIMATOR HAS NOT CONVERGED AFTER %d ITERATIONS!\n", MAX_ITER);
	}

	return (iterer);
}


