
/*
 *	Copyright (c) 1991-1996 Science Applications International Corporation.
 *

 * NAME		
 *	loc_params -- Event location specific structure.

 * FILE
 *	loc_params.h

 * SYNOPSIS
 *	Structure interface between the EvLoc, LocSAT, ARS and GA sub-system 
 *	to the locate_event() function interface.

 * AUTHOR
 *	31 Jan 1991	Cynde K. Smith	Created
 *	 7 Jun 1991	Walt Nagy	Augmented prologue.
 *	16 Dec 1992	Walt Nagy	Added fix_lat_lon, fix_origin_time,
 *					and refill_if_loc_fails variables.
 *	27 Mar 1993	Walt Nagy	Added travel-time correction variables.
 *	 3 Mar 1994	Walt Nagy	Added Default_Locator_params.
 *	26 Sep 1996	Walt Nagy	Added include file, db_ar_info.h", so
 *					we ensure including extended 
 *					association info of ar_info structure
 *

 * SccsId:	@(#)libloc/loc_params.h	125.1	06/29/98	SAIC.
 */


#ifndef LOC_PARAMS_H
#define LOC_PARAMS_H

#include "db_ar_info.h"

#define	NO_SRC_DPNT_CORR	0
#define	SSSC_LEVEL_1_CORR	1
#define	SSSC_LEVEL_2_CORR	2
#define	TEST_SITE_CORR		3
#define	SRST_CORR		4

/*
 * The Locator_params structure will be used to pass parameter values from
 * the Locator GUI, LocSAT, ARS and ESAL to locate_event().  locate_event() 
 * will then use these parameters for it's call to the locatation algorithm,
 * LocSAT.
 */

typedef struct locator_params 
{
				     /* DEFAULT- DESCRIPTION                  */
	int	max_iterations;	     /* 20     - Max # of iterations allowed  */
	int	num_iter;	     /* 0      - # of iterations performed    */
	int	refill_if_loc_fails; /* FALSE  - Re-fill origin & assoc tables
						 if location fails ?          */
	int	fix_depth;	     /* TRUE   - Fix depth ?                  */
	int	fix_lat_lon;	     /* FALSE  - Fix latitude & longitude     */
	int	fix_origin_time;     /* FALSE  - Fix origin time ?            */
	int	use_location;	     /* TRUE   - Use current event location ? */
	int	use_elev_corr;	     /* TRUE   - Apply elevation corrections  */
	int	ellip_cor_type;      /* 2      - Ellipticity correction type
						 (0=None; 1=AFTAC; 2=IASPEI)  */
	int	sssc_level;	     /* 0      - SSSC level to apply (0 = None;
						 1 = Regional; 2 = Local)     */
	int	use_test_site_corr;  /* FALSE  - Apply test-site corrections  */
	int	use_srst;	     /* FALSE  - Apply SRST corrections	      */
	int	srst_var_wgt;	     /* FALSE  - Apply SRST variance weighting*/
	int	use_only_sta_w_corr; /* FALSE  - Use only data w/ SSSC, SRST,
						 or test-site corrections     */
	int	dist_var_wgt;	     /* FALSE  - Apply distance variance
						 weighting		      */
	int	ignore_large_res;    /* FALSE  - Ignore data residuals > 
						 large_res_mult*data_std_err  */
	int	num_dof;	     /* 9999   - Number of degrees of freedom */
	double	est_std_error;	     /* 1.0    - Estimate of data std error   */
	double	conf_level;	     /* 0.9    - Confidence level    	      */
	double	damp;		     /* -1.0   - Percent damping to apply to
						 G matrix (-1.0 = No damping) */
	double	user_var_wgt;	     /* -1.0   - User variance weighting to be
						 applied to sensitivity matrix.
						 Overrides all other var. wgt.*/
	double	lat_init;	     /* -999.0 - Initial latitude (deg)       */
	double	lon_init;	     /* -999.0 - Initial longitude (deg)      */
	double	depth_init;	     /* -999.0 - Initial (fixing) depth (km)  */
	double	origin_time_init;    /* 0.0    - Initial (fixing) origin time 
						 (sec)			      */
	double	large_res_mult;	     /* 3.0    - Muliplier factor to apply to
						 residuals to use in location */
	char	verbose;	     /* 'y'    - Level of verbose output
						   0 = n = f: No output 
						   1: Only final location
						   2: Only final loc & arrivals
						   3: No intermediate steps
						   4 = y = t: All output      */
	char	test_site_region[5]; /* NULL   - Test-site region name        */
	char	*outfile_name;	     /* NULL   - Name of output file          */
	char	*prefix;	     /* NULL   - Directory name & prefix of 
						 travel-time tables           */
} Locator_params;


/*
 * The ar_info (arrival information) structure is used to store extended
 * arrival/assoc-based information computed by the location process.
 * Note that there is duplicity between this structure and old locator_errors
 * structure that use to be defined in this include file.  This was done 
 * to replace the locator_errors structure in the call to locate_event() 
 * with ar_info.

typedef	struct	ar_info {
	long	orid;
	long	arid;
	int	time_error_code;
	int	az_error_code;
	int	slow_error_code;
	int	src_dpnt_corr_type;
	char	vmodel[16];
	double	total_travel_time;
	double	tt_table_value;
	double	ellip_corr;
	double	elev_corr;
	double	bulk_static_sta_corr;
	double	src_dpnt_corr;
	double	model_error;
	double	meas_error;
	double	model_plus_meas_error;
	double	time_import;
	double	az_import;
	double	slow_import;
	double	slow_vec_res;
	char	lddate[18];
} Ar_Info;
 */

#endif /* LOCATOR_PARAMS_H */

