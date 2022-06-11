
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * NAME
 *	initialize_mag_params -- Initialize mag_params structure to N/A values

 * FILE
 *	mag_params.c

 * SYNOPSIS 
 *	Mag_params
 *	initialize_mag_params ()

 * DESCRIPTION
 *	Functions.  Most all manipulation and storage of mag_params control
 *	structure information is handled here.  This structure contains
 *	information global to the magnitude process used external to this
 *	library.  A description of individual attributes of this structure
 *	is provided within this section immediately after a description of
 *	all functions contained in this file.

 *	-- initialize_mag_params() initializes mag_params structure (type,
 *	Mag_params) to N/A values.

 *	Magnitude control parameters (mag_params->)
 *	-----------------------------------------------------------------------
 *	verbose			%s	Degree of magnitude verbose output
 *					 = 0: No output to be displayed
 *					 = 1: Only network magnitude info to be
 *					      displayed
 *					 = 2: Both network and station magnitude
 *					      info to be displayed
 *	magtype_to_origin_mb	%s	Specify mb magtype to be written to 
 *					origin.mb field
 *	magtype_to_origin_ms	%s	Specify Ms magtype to be written to 
 *					origin.ms field
 *	magtype_to_origin_ml	%s	Specify ML magtype to be written to 
 *					origin.ml field
 *	list_of_mb_magtypes	%s[]	List of mb magtypes for which to apply
 *					control screening for min/max distance
 *					and determining network mb mmodel.
 *	num_mb_magtypes		%d	Number of mb magtypes 
 *	num_boots		%d	Specifies number of bootstrap resamples 
 *					for MLE computations.  Specifically,
 *					number of iteration of data resampling
 *					for MLE bootstrap estimation.  If non-
 *					zero, num_boots randomly selected data
 *					sets will be constructed, computed and 
 *					averaged.
 *	use_only_sta_w_corr	%b	Use only amplitude data with source-
 *					dependent corrections available.
 *					Currently, this only means test-site
 *					corrections, therefore, only meaningful
 *					when use_ts_corr is set TRUE.
 *	sub_sta_list_only	%b	If TRUE, use only magnitude data
 *					contained in sub-station list below.
 *	sub_sta_list		%s[]	If sub_sta_list_only setting is TRUE,
 *					then use only amplitude data for
 *					stations specified in this list.  If
 *					you want to make sure you get the same
 *					results, as say between ARS and EvLoc, 
 *					then this list better be the same in 
 *					both applications.  The list itself is
 *					not passed in the event_control table.
 *	num_sub_sta_list	%d	Number of entries in sub_sta_list
 *	ignore_large_res	%b	Switch to control whether or not to
 *					censure output stamag records based on
 *					their length from the mean network-
 *					averaged magnitude.  Specifically,
 *					should stamag data with stamag.magres >
 *					large_res_mult * s.d. of network
 *					magnitude, be ignored?
 *	large_res_mult		%F	Large (big) residual multiplier applied
 *					if above setting is set to TRUE.
 *	use_ts_corr		%b	Use (apply) a specified test-site
 *					magnitude correction.  A test-site
 *					correction will be applied as defined
 *					by variable, ts_region.
 *	ts_region		%s	Magnitude correction for this test-site
 *					region will be applied, if use_ts_corr
 *					is set TRUE.
 *	outfile_name		%s	Directory pathway and filename where
 *					output magnitude information should be
 *					written.  If empty, then simply write
 *					to stdout.
 *	-----------------------------------------------------------------------

 * DIAGNOSTICS
 *	-- initialize_mag_params(), returns a default mag_params structure
 *	(type, Mag_params) populated with N/A values.

 * FILES
 *	None.

 * NOTES
 *	None.

 * SEE ALSO
 *	Setting of mag_cntrl structure in local file, mag_access.c.

 * AUTHOR
 *	Walter Nagy,  9/ 8/97,	Created.
 */



#include <stdio.h>
#include "libmagnitude.h"


Mag_Params
#ifdef	__STDC__
initialize_mag_params ()
#else
initialize_mag_params ()
#endif
{
	Mag_Params	Default_Mag_Params;

	Default_Mag_Params.verbose		= 0;
	strcpy (Default_Mag_Params.magtype_to_origin_mb, "");
	strcpy (Default_Mag_Params.magtype_to_origin_ms, "");
	strcpy (Default_Mag_Params.magtype_to_origin_ml, "");
	Default_Mag_Params.list_of_mb_magtypes	= (char **) NULL;
	Default_Mag_Params.num_mb_magtypes	= 0;
	Default_Mag_Params.num_boots		= 20;
	Default_Mag_Params.use_only_sta_w_corr	= FALSE;
	Default_Mag_Params.sub_sta_list_only	= FALSE;
	Default_Mag_Params.sub_sta_list		= (char **) NULL;
	Default_Mag_Params.num_sub_sta_list	= 0;
	Default_Mag_Params.ignore_large_res	= FALSE;
	Default_Mag_Params.large_res_mult	= 3.0;
	Default_Mag_Params.use_ts_corr		= FALSE;
	strcpy (Default_Mag_Params.ts_region, "");
	Default_Mag_Params.outfile_name		= (char *) NULL;

	return (Default_Mag_Params);
}


