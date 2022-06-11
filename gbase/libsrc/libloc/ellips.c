
/*
 * Copyright (c) 1992-1996 Science Applications International Corporation.
 *

 * NAME
 *	ellips -- Compute hypocentral event error ellipsoid.

 * FILE
 *	ellips.c

 * SYNOPSIS
 *	void
 *	ellips (locator_info, w_origerr, conf_level, num_dof, est_std_error)
 *	Locator_info	*locator_info;	(i/o) Locator info structure
 *	Origerr		**w_origerr;	(i/o) Local working DB3.0 origerr table
 *	double		conf_level;	(i) Confidence level (0.0 - 1.0)
 *	int		num_dof;	(i) Number of degrees of freedom
 *	double		est_std_error;	(i) Estimated a priori data std. error

 * DESCRIPTION
 *	Function.  Determine event error ellipsoid and normalized confidence 
 *	regions.  Given the covariance matrix of the hypocentral estimate,
 *	calculate the error ellipsoid and confidence regions from the 
 *	appropriate marginal variances with F-distribution factors ignored.
 *	These are normalized in the sense that they are scaled to a 
 *	particular confidence probability, thereby making the marginal 
 *	variances justified.

 *	The following parameters are described:
 *	   1.	The three-dimensional confidence ellipsoid of the hypocenter,
 *		as determined from the 3x3 variance matrix of the hypocenter
 *		(marginal w.r.t. origin time)
 *	   2.	The two-dimensional confidence ellipse of the epicenter, as
 *		determined from the 2x2 variance matrix of the epicenter
 *		(marginal w.r.t. origin time and depth)
 *	   3.	The one-dimensional confidence interval of focal depth, as
 *		determined from the scalar variance of the focal depth
 *		(marginal w.r.t. origin time and epicenter)
 
 *	---- Functions called ----
 *	Local
 *		f_test:	Make an F-test
 
 * DIAGNOSTICS
 *	Three-dimensional ellipsoid parameters not yet inplemented.  Output 
 *	variables hy[...] are currently returned as zero.

 * NOTES
 *	Currently, f_test() only accepts the following conf_level values of
 *	0.80, 0.90, 0.95 and 0.99.

 *	Eventually, a programmer should deal with the three-dimensional 
 *	ellipsoid calculations with perhaps the following variables:
 *	hymaj:	Length of major semi-axis of hypocenter confidence ellipsoid
 *	hymid:	Length of middle semi-axis of hypocenter confidence ellipsoid
 *	hymin:	Length of minor semi-axis of hypocenter confidence ellipsoid
 *	hystr:	Strike of major semi-axis of hypocenter confidence ellipsoid
 *		(degrees clockwise from north)
 *	hyplu:	Plunge of major semi-axis of hypocenter confidence ellipsoid
 *		(degrees downward from horizontal)
 *	hyrak:	Direction of middle semi-axis of hypocenter confidence ellipsoid
 *		(degrees clockwise from up, when looking down-plunge)

 *	Not currently used, so commented out (WCN)
 *	f_test (3, locator_info->tot_ndf, conf_level, &fs);
 *	fac   = sqrt(3.0*fs)*locator_info->sigma_hat;
 *	hymaj = hymaj0*fac;
 *	hymid = hymid0*fac;
 *	hymin = hymin0*fac;

 * SEE ALSO
 *	Bratt and Bache (1988, BSSA, 78, 780-798; and 
 *	Jordan and Sverdrup (1981, BSSA, 71, 1105-1130).

 * AUTHOR
 *	Walter Nagy,  8/18/92,	Created.  Contents largely moved from file,
 *				compute_hypo.c.
 *	Walter Nagy, 11/18/93,	Added 80th percentile ellipse table for 
 *				computing confidence regions.
 */

#include "config.h"
#include <math.h>
#include <stdio.h>
#include "libloc.h"
#include "locp.h"
#include "loc_defs.h"

#define TWO_PI		2.0*M_PI
#define	STRIKE_NULL	-1.0


void
ellips (Locator_info *locator_info, Origerr **w_origerr, double conf_level, 
	int num_dof, double est_std_error)
{
	Origerr	*origerr;
	double	c, cc, fac, fs, epstrr, s, strike, ss;
	double	stt, sxx, sxy, sxytcs, syy, szz, twosxy;
	double	epi_semi_major, epi_semi_minor;
	double	andf, est_sqrd_ndf, snssdden, snssdnum, ssq;


	origerr = *w_origerr;

	/*
	 * First, compute data-related variance estimates.

	 * num_dof, is the K of Jordan & Sverdrup (1981); Bratt & Bache (1988)
	 *	set num_dof = 8; we recommend num_dof = 99999 for most IMS
	 *	applications, i.e., we assume perfect a priori knowledge about
	 *	the data errors (i.e., the coverage ellipse in the AFTAC case).
	 *	We automatically calculate the coverage (num_dof=99999) and
	 *	confidence (num_dof=0) ellipses, as well as, the inputted
	 *	value, which may be the same as one of these end members.
	 * est_std_error, is s-sub-k of Bratt & Bache (1988); it is the a
	 *	priori estimate of the variance scale factor (as determined
	 *	from past runs of the data itself).  We recommend that
	 *	est_std_error = 1.0 for most IMS applications.
	 * tot_ndf, is the total degrees of freedom assuming an F-distribution
	 *	= num_dof + [# of data - # of parameters].  If tot_ndf is
	 *	equal to infinity a chi-squared distribution is obtained.
	 * ssq, is the numerator for the a posteriori estimate of the
	 *	squared variance scale factor; eqn (6) of Bratt & Bache (1988).
	 * sigma_hat is the actual estimate of the variance scale factor
	 *	(eqn. 34 of J&S, 1981), and,
	 * snssd, is the normalized a priori estimate for the estimated
	 *	variance scale factor.
	 */

	locator_info->tot_ndf = num_dof + locator_info->nd_used
					- locator_info->np;
	est_sqrd_ndf = num_dof * est_std_error*est_std_error;
	ssq = locator_info->sum_sqrd_res + est_sqrd_ndf;
	andf = locator_info->tot_ndf;
	if (andf == 0.0)
	    andf = 0.001;
	if (fabs(locator_info->tot_ndf - ssq) < 0.00001)
	    andf = ssq;

	locator_info->sigma_hat = sqrt(ssq/andf);
	snssdnum = ssq-est_sqrd_ndf;
	snssdden = andf-(double)num_dof;
	if (fabs(snssdden) > 0.001 && snssdnum/snssdden >= 0.0)
	    locator_info->snssd = sqrt(snssdnum/snssdden);
	else
	    locator_info->snssd = -1.0;


	/*
	 * Compute two-dimenstional ellipse parameters from marginal 
	 * epicentral variances
	 */
 
	stt = origerr->stt;
	sxx = origerr->sxx;
	sxy = origerr->sxy;
	syy = origerr->syy;
	szz = origerr->szz;
	twosxy = 2.0*sxy;
	if (twosxy != 0.0)
	{
	    epstrr = 0.5*atan2(twosxy, syy-sxx);
	    c      = cos(epstrr);
	    s      = sin(epstrr);
	    cc     = c*c;
	    ss     = s*s;
	    sxytcs = twosxy*c*s;
	    epi_semi_major = sxx*ss + sxytcs + syy*cc;
	    epi_semi_minor = sxx*cc - sxytcs + syy*ss;
	    if (epstrr < 0.0)    epstrr = epstrr + TWO_PI;
	    if (epstrr > TWO_PI) epstrr = epstrr - TWO_PI;
	    if (epstrr > M_PI)   epstrr = epstrr - M_PI;
	    strike = RAD_TO_DEG*epstrr;
	    if (epi_semi_major < 0.0 || epi_semi_minor < 0.0)
	    {
		origerr->smajax = -1.0;
		origerr->sminax = -1.0;
		origerr->strike = -1.0;
	    }
	    else
	    {
		f_test (2, locator_info->tot_ndf, conf_level, &fs);
		fac = sqrt(2.0*fs)*locator_info->sigma_hat;
		origerr->smajax = sqrt(epi_semi_major)*fac;
		origerr->sminax = sqrt(epi_semi_minor)*fac;

		if (strike < 0.0 && strike != STRIKE_NULL)
		    while (strike < 0.0)
			strike += 180;
		origerr->strike = strike;
	    }
	}
	else
	{
	    origerr->smajax = -1.0;
	    origerr->sminax = -1.0;
	    origerr->strike = -1.0;
	}

	/*
	 * Compute the one-dimensional depth and time confidence semi-intervals
	 * from marginal depth and time variances
	 */
 
	f_test (1, locator_info->tot_ndf, conf_level, &fs);
	fac = sqrt(fs)*locator_info->sigma_hat;
 
	if (szz <= 0.0)
	    origerr->sdepth = -1.0;
	else
	    origerr->sdepth = sqrt(szz)*fac;

	if (stt <= 0.0)
	    origerr->stime = -1.0;
	else
	    origerr->stime = sqrt(stt)*fac;
}

