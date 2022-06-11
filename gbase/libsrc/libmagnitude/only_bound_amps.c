
/*
 * Copyright (c) 1994-1997 Science Applications International Corporation.
 *

 * NAME
 *	only_bound_amps -- At what conf. level can hypothesis be rejected.

 * FILE
 *	only_bound_amps.c

 * SYNOPSIS
 *	int
 *	only_bound_amps (sm_sub, mcntrl, sub_count, ave, isign, sigma, 
 *			 net_mag, sigmax)
 *	SM_Sub	*sm_sub;
 *	Mag_Cntrl	*mcntrl;
 *	int	sub_count;
 *	double	ave;
 *	int	isign;
 *	double	sigma;
 *	double	*net_mag;
 *	double	*sigmax;

 * DESCRIPTION
 *	Function.  When all the data are lower (or upper) bounds, do a 
 *	hypothesis test calculation : at what confidence level can I
 *	reject the hypothesis that no stations were able to detect the
 *	signal, given that the event magnitude is a particular (assumed)
 *	value?

 * DIAGNOSTICS
 *	If maximum number of iterations is exceeded, -2, will be returned
 *	upon compeletion.

 * FILES
 *	None.

 * NOTES
 * 	None.

 * SEE ALSO
 *	None.

 * AUTHORS
 *	Unknown.		Created.
 *	Walter Nagy,  5/94,	Converted from FORTRAN 4 to C, May 1994.
 *	Walter Nagy, 10/97,	Reflects changes required to accomodate new
 *				amplitude schema and magnitude model.  Change
 *				of file name to only_bound_amps.c
 */



#include <stdio.h>
#include <math.h>
#include "magp.h"

#define	MAX_ITER 200


int
#ifdef __STDC__
only_bound_amps (SM_Sub *sm_sub, Mag_Cntrl *mcntrl, int sub_count, double ave,
		 int isign, double sigma, double *net_mag, double *sigmax)
#else
only_bound_amps (sm_sub, mcntrl, sub_count, ave, isign, sigma, net_mag, sigmax)

SM_Sub	*sm_sub;
Mag_Cntrl	*mcntrl;
int	sub_count;
double	ave;
int	isign;
double	sigma;
double	*net_mag;
double	*sigmax;
#endif
{
	int	i;
	int	iter = 0;
	int	isig, isig2;
	double	siginc = 0.05;
	double	coef, coef1, mu, mu0, prob, prob0, tmp;

	/*
	 * The probability calculation will be performed for a suite of
	 * possible widths of the Gaussian distribution of signals
	 */

	*sigmax	 = -1.0;
	*net_mag = 0.0;
//	isig1	= (int)(mcntrl->sglim1/siginc);
	isig2	= (int)(mcntrl->sglim2/siginc);

	if (mcntrl->sglim1 == mcntrl->sglim2)
	{
//	    isig1 = 1;
	    isig2 = 1;
	    siginc = mcntrl->sglim1;
	}
	if (mcntrl->sglim2 > isig2*siginc) 
	    ++isig2;

	for (isig = 0; isig < isig2; isig++)
	{
	    sigma = siginc*(double)(isig+1);
	    coef1 = (double)isign*M_SQRT1_2/sigma;

	    /*
	     * The probability calculation will be performed for a suite of
	     * assumed event magnitudes; start just below (or above) the
	     * average noise (or clip) magnitude.
	     */

	    mu = 0.05*((int)(20.0*ave) - 5 * isign);
	    mu0 = mu;
	    prob0 = 1.0;
	    mu = mu + 0.05*(double)isign;
	    for (iter = 0; iter < MAX_ITER; iter++)
	    {
		prob = 1.0;
		for (i = 0; i < sub_count; i++)
		{
		    coef = coef1;
		    if (sm_sub[i].wt != 0.0)
			coef = (double)isign*M_SQRT1_2 /
				   sqrt(sigma*sigma + 
						sm_sub[i].wt*sm_sub[i].wt);
		    prob = 
			prob*0.5*(1.0 + erf((mu - sm_sub[i].magnitude)*coef));
		    if (prob < 1.0E-20)
			prob = 0.0;
		}
		if (prob > 0.9475 && prob < 0.9525)
		    break;
		tmp = mu;
		if (prob < 0.90)
		    mu = mu + 0.1*(double)isign;
		else
		    mu = mu - (mu-mu0)*(prob-0.95)/(prob-prob0);
		mu0 = tmp;
		prob0 = prob;
	    }

	    if (sigma < *sigmax)
		continue;
	    *net_mag = mu;
	    *sigmax  = sigma;
	}
	if (iter > MAX_ITER)
	    return (-2);
	else
	    return (0);

}


