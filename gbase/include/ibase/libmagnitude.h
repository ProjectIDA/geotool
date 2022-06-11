
/*
 * Copyright (c) 1997-2000 Science Applications International Corporation.
 *

 * FILENAME
 *	libmagnitude.h

 * DESCRIPTION
 *	Includes prototype definitions for routines called outside library, 
 *	libmagnitude.  Also includes main magnitude object definition of
 *	type, MAGNITUDE (*Magnitude).

 */

#ifndef _LIBMAGNITUDE_H_
#define _LIBMAGNITUDE_H_

#include "aesir.h"	/* For Proto definition. */

#include "db_site.h"
#include "db_origin.h"
#include "db_assoc.h"
#include "db_parrival.h"
#include "mag_defs.h"
#include "mag_descrip.h"
#include "mag_params.h"


extern MAGNITUDE *copy_magnitudes(
				  MAGNITUDE  *in_magnitude,
				  int	   num_mag_records);

extern void free_magnitudes(MAGNITUDE  *magnitude,
			    int	   num_mag_records);

extern int calc_mags(MAGNITUDE  *magn_ptr, 
		     int	   num_mags,
		     Origin	   *origin,
		     Mag_Params *mag_params);

extern MAGNITUDE *build_mag_obj(
				char	**list_of_magtypes, 
				int	num_magtypes,
				Origin	*origin,
				Netmag	*in_netmag,
				int	num_netmags,
				Stamag	*in_stamag,
				int	num_stamags,
				Amplitude *det_amplitude,
				int	num_det_amps,
				Amplitude *ev_amplitude,
				int	num_ev_amps,
				Assoc	*in_assoc,
				int	num_assocs,
				Parrival *in_parrival,
				int	num_parrivals);

extern int setup_mag_facilities(	
				char	*tl_model_filename,
				char	*mdf_filename,
				char	**list_of_magtypes,
				int	num_magtypes,
				Site	*sites,
				int	num_sites);

extern int setup_mc_tables(char	*tl_model_filename,
			   char	*mdf_filename,
			   char	**list_of_magtypes,
			   int	num_magtypes,
			   Site	*sites,
			   int	num_sites);

extern int read_mdf(char *mdf_filename,
		    char		**list_of_magtypes,
		    int		num_req_magtypes,
		    Mag_Descrip	**mag_descrip_ptr,
		    int		*num_md,
		    Mag_Sta_TLType	**mag_sta_tltype_ptr,
		    int		*num_mst,
		    TL_Pt		**list_of_TLtypes);

extern int read_tl_table(char	*dir_pathway,
			 char	*TLtype,
			 char	*tl_model,
			 char	*phase,
			 char	*chan,
			 TL_Table **tl_table_ptr);

extern double station_magnitude(
				char	*magtype,
				char	*sta,
				char	*phase,
				char	*chan,
				Bool	extrapolate,
				char	*ts_region,
				double	distance,
				double	ev_depth,
				double	amp,
				double	per,
				double	duration,
				double	snr,
				SM_Info	*sm_info);

extern double abbrev_sta_mag(char	*magtype,
			     char	*sta,
			     char	*phase,
			     char	*chan,
			     double	distance,
			     double	ev_depth,
			     double	amp,
			     double	per,
			     double	duration);

extern int read_tlsf(char	*tl_model_filename,
		     TL_Pt	*list_of_TLtypes,
		     Site	*sites,
		     int	num_sites);

extern double interp_for_tl_value(
				  double	distance,
				  double	ev_depth,
				  int	tl_index,
				  Bool	extrapolate,
				  double	*tl_deriv,
				  int	*interp_code);

extern char *TL_error_msg(int	error_code);

extern char *mag_error_msg(int	error_code);

extern int get_mag_indexes(char	*magtype,
			   char	*sta,
			   char	*phase,
			   char	*chan,
			   int	*sta_index,
			   int	*stm_index,
			   int	*tlmd_index,
			   int	*md_index,
			   int	*mst_index);

extern int get_TL_indexes(char	*TLtype,
			  char	*sta,
			  char	*phase,
			  char	*chan,
			  int	*sta_index,
			  int	*stm_index,
			  int	*tlmd_index);

extern Bool get_TLMD_index(char	*TLtype);

extern double get_tl_model_error(
				 int	tl_index,
				 double	delta,
				 double	depth,
				 char	*model);

extern double get_meas_error(double	snr);

extern Bool valid_phase_for_TLtype(
					char	*TLtype,
					char	*phase);

extern Bool valid_range_for_TLtable(
				    char	*TLtype,
				    char	*sta,
				    char	*phase,
				    char	*chan,
				    double	delta,
				    double	ev_depth);


extern Bool get_TL_ts_corr(char	*ts_region,
			   char	*sta,
			   char	*TLtype,
			   int	tl_index,
			   double	*ts_corr);

extern int set_sta_TL_pt(Site	*sites,
			 int	num_sites);

extern Mag_Params initialize_mag_params();

extern Bool get_magtype_features(
				 char	   *magtype,
				 Mag_Cntrl  *mag_cntrl);

extern int reset_amptypes(char	*magtype,
			  char	*det_amptype,
			  char	*ev_amptype);

extern int reset_algorithm(char	*magtype,
			   int	algo_code);

extern int reset_min_dist(char	*magtype,
			  double	dist_min);

extern int reset_max_dist(char	*magtype,
			  double	dist_max);

extern int reset_sd_limits(char	*magtype,
			   double	sglim1,
			   double	sglim2);

extern int reset_sd_baseline(char	*magtype,
			     double	sgbase);

extern int reset_wgt_ave_flag(char	*magtype,
			      Bool	apply_wgt);

extern int revert_amptypes(char	*magtype);

extern int revert_algorithm(char *magtype);

extern int revert_min_dist(char	*magtype);

extern int revert_max_dist(char *magtype);

extern int revert_sd_limits(char *magtype);

extern int revert_sd_baseline(char *magtype);

extern int revert_wgt_ave_flag(char *magtype);

extern SM_Info initialize_sm_info(void);

extern void free_tl_table(void);

extern double get_delta_for_sta(char	*sta,
				double	ev_lat,
				double	ev_lon);

extern void mag_set_compute_upper_bounds(Bool	compute_ubs);

extern Bool mag_get_compute_upper_bounds();

#endif /* _LIBMAGNITUDE_H_ */

