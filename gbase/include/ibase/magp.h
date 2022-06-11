
/*
 * magnitudep.h local include file.  Prototype definitions for routines 
 * local to the library, libmagnitude.

 * SccsId:  @(#)magp.h	4.1	10/30/97	Copyright (c) 1997 SAIC.
 */

#ifndef _MAGP_H_
#define _MAGP_H_


#include "aesir.h"	/* For Proto definition. */
#include "mag_defs.h"
#include "mag_descrip.h"

typedef	struct	sm_sub
{
	char	magdef[2];	/* Magnitude defining state [d/n] */
	int	sig_type;	/* Signal type [1/2/3] */
	double	wt;		/* Weighting for station magnitude data */
	double	magnitude;	/* Station magnitude */
} SM_Sub;


extern int network_mag(	SM_Sub	*sm_sub,
			Mag_Cntrl	*mcntrl,
			int	sm_count,
			int	verbose,
			double	*mag,
			double	*sigma,
			double	*sdav,
			int	*num_amps_used);

extern int mag_boot_strap(SM_Sub	*sm_sub,
			  Mag_Cntrl	*mcntrl,
			  int	sm_count,
			  int	num_boots,
			  double	net_mag,
			  double	sigma,
			  double	*fmag1,
			  double	*sigmu,
			  double	*sig1,
			  double	*sigsig,
			  int	verbose);

extern int mag_max_lik(SM_Sub	*sm_sub,
		       Mag_Cntrl	*mcntrl,
		       int	sm_count,
		       double	ave,
		       double	*net_mag,
		       double	*sigma,
		       int	verbose);

extern int only_bound_amps(SM_Sub	*sm_sub,
			   Mag_Cntrl	*mcntrl,
			   int	sub_count,
			   double	ave,
			   int	isign,
			   double	sigma,
			   double	*net_mag,
			   double	*sigmax);


#endif /* _MAGP_H_ */

