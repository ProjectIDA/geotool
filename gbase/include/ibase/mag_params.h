
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * FILENAME
 *	mag_params.h

 * DESCRIPTION
 *	C structure declarations for the handling magnitude control 
 *	(parameter) structure information needed for SAIC software. 

 * SccsId:  @(#)mag_params.h	4.1	10/30/97	SAIC.
 */


#ifndef	MAG_PARAMS_H
#define	MAG_PARAMS_H


typedef struct mag_params
{
	int	verbose;		/* Verbosity level [0/1/2] */
	char	net[9];			/* Magnitude network name */
	char	magtype_to_origin_mb[7];/* Write this magtype to origin.mb */
	char	magtype_to_origin_ms[7];/* Write this magtype to origin.ms */
	char	magtype_to_origin_ml[7];/* Write this magtype to origin.ml */
	char	**list_of_mb_magtypes;	/* List of mb magtypes to limit effect
					   of distance and mmodel controls */
	int	num_mb_magtypes;	/* Number of mb magtypes */
	int	num_boots;		/* Number of bootstrap re-samples */
	Bool	use_only_sta_w_corr;	/* Use only stamags associated with a
					   source-dependent correction? [T/F] */
	Bool	sub_sta_list_only;	/* Use only stamags with stations in
					   sub_sta_list? [T/F] */
	char	**sub_sta_list;		/* List of sub stations to be used when
					   sub_sta_list_only is TRUE */
	int	num_sub_sta_list;	/* Number of elements in sub_sta_list */
	Bool	ignore_large_res;	/* Censure output stamag records when
					   fabs(magres) > large_res_mult * 
					   a posteriori std dev.? [T/F] */
	double	large_res_mult;		/* Multiplier factor used when above
					   ignore_large_res boolean is TRUE */
	Bool	use_ts_corr;		/* Apply test-site correction, if
					   available? [T/F] */
	char	ts_region[9];		/* Test-site region name */
	char	*outfile_name;		/* Output file name for mag results to
					   be written if verbose > 0 */
} Mag_Params;

#endif	/* MAG_PARAMS_H */

