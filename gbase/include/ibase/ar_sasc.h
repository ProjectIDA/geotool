
/*
 * Copyright (c) 1997-2003 Science Applications International Corporation.
 *

 * FILENAME
 *	ar_sasc.h

 * DESCRIPTION
 *	C structure declaration for handling slowness/azimuth station
 *	corrections (SASC) on an arrival-by-arrival basis.  This 
 *	structure preserves the raw azimuth and slowness calculations 
 *	and also stores specifics of the new (corrected) azimuths and
 *	slownesses.

 * NOTES
 *	This structure can be thought of as a rough blueprint for an
 *	arrival-based database structure.  Need to get some policy about 
 *	units for slowness.  Do not have to force a standard (although 
 *	that would be nice), but do need some set of rules (WCN).

 * AUTHOR
 *	Walter Nagy,	 2/21/97
 */


#ifndef	AR_SASC_H
#define	AR_SASC_H

typedef	struct	ar_sasc {
	int	arid;		/* Unique arrival identifier */
	char	sta[7];		/* Station name */
	double	raw_azimuth;	/* Raw azimuth (deg; w/o any corrections) */
	double	raw_slow;	/* Raw slowness (sec/deg; w/o any corrections) */
	double	delaz;		/* Azimuth measurement error (deg) */
	double	delslo;		/* Slowness measurement error (sec/deg) */
	double	azimuth_corr;	/* Azimuth correction from SASC table, if any */
	double	slow_corr;	/* Slowness correction from SASC table, if any */
	double	azimuth_mdl_err;/* Azimuth modeling error (deg) */
	double	slow_mdl_err;	/* Slowness modeling error (sec/deg) */
	double	azimuth;	/* Final azimuth (deg; w or w/o SASC applied) */
	double	slow;		/* Final slow (s/deg; w or w/o SASC applied) */
	double	tot_az_err;	/* Total azimuth error (deg)0 */
	double	tot_slow_err;	/* Total slowness error (sec/deg) */
} Ar_SASC;

#endif	/* AR_SASC_H */

extern Ar_SASC get_ar_sasc(int arid);

