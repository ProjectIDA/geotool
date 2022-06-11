
/*
 * Copyright (c) 1994-1996 Science Applications International Corporation.
 *

 * libloc.h include file.  Prototype definitions for routines called outside of 
 * library, libloc.  Local library prototypes are defined in include file, 
 * locp.h.  Hydro_saic prototypes are in tt_info.h
 *
 * SccsId:  @(#)libloc/libloc.h	125.1	06/29/98	SAIC.
 */

#ifndef _LIBLOC_H_
#define _LIBLOC_H_

#include "aesir.h"	/* For Proto definition. */

#ifdef __svr4__


#endif

#include "db_site.h"
#include "db_origin.h"
#include "db_origerr.h"
#include "db_arrival.h"
#include "db_assoc.h"
#include "loc_params.h"
#include "loc_info.h"
#include "tt_info.h"


extern int locate_event(Site		*sites,
			int		num_sites,
			Arrival		*arrival,
			Assoc		*assoc,
			Origin		*origin,
			Origerr		*origerr,
			Locator_params	*locator_params,
			Ar_Info		*ar_info,
			int		num_obs);

extern void predsat(Locator_params	*locator_params,
		    Site		*sites,
		    int		num_sta,
		    Origin		*origin,
		    Origerr		*origerr,
		    char		*data_sta_id,
		    char		*data_phase_type,
		    int		num_data,
		    int		slow_flag,
		    int		verbose,
		    char		*out_file,
		    double		*dist_min,
		    double		*dist_max,
		    double		*dist_center,
		    double		*tt_min,
		    double		*tt_max,
		    double		*tt_center,
		    double		*az_min,
		    double		*az_max,
		    double		*az_center,
		    double		*slow_min,
		    double		*slow_max,
		    double		*slow_center,
		    int		*tt_err_min,
		    int		*tt_err_max,
		    int		*tt_err_center,
		    int		*slow_err_min,
		    int		*slow_err_max,
		    int		*slow_err_center);

extern void ellips(Locator_info	*locator_info,
		   Origerr		**w_origerr,
		   double		conf_level,
		   int		num_dof,
		   double		est_std_error);

extern int  setup_tt_facilities(	
				char		*vmodel_filename,
				char		**phase_types,
				int		num_phase_types,
				Site		*sites,
				int		num_sites);

extern int  setup_tttables(	char		*vmodel_filename,
				char		**phase_types,
				int		num_phase_types);

extern int get_srst_region_number(void);

extern int get_tt_indexes(char		*phase,
			  int		sta_index,
			  int		*spm_index);

extern int get_default_phase_index(char	*phase);

extern double get_model_error(int		phase_index,
			      double		delta,
			      double		depth,
			      double          azimuth, 
			      char            *sta);

extern double total_travel_time(	
				Locator_params	*locator_params,
				Site		*sites,
				Ar_Info		*ar_info,
				Bool		extrapolate,
				Bool		do_we_need_z_derivs,
				double		ev_lat,
				double		ev_lon,
				double		ev_depth,
				double		delta,
				double		esaz,
				char		*phase,
				int		sta_index,
				int		phase_index,
				int		spm_index,
				double		data_std_err,
				double		*prin_deriv,
				double		*tt_deriv,
				double		*slow_deriv,
				double		*az_deriv,
				int		*error_code);

char *get_vmodel(void);

extern double compute_ttime_w_corrs(	
				    Locator_params	*locator_params,
				    Site		*sites,
				    Bool		extrapolate,
				    double		ev_lat,
				    double		ev_lon,
				    double		ev_depth,
				    double		delta,
				    double		azimuth,
				    char		*phase,
				    int		sta_index,
				    Ar_Info		*ar_info,
				    double		*horiz_slow);

extern double trv_time_w_ellip_elev(Bool		extrapolate, 
				    Bool		do_we_need_z_derivs,
				    double		ev_lat,
				    double		delta,
				    double		depth, 
				    double		esaz,
				    char		*phase, 
				    double		sta_elev, 
				    int		phase_index,
				    int		spm_index,
				    double		*prin_deriv, 
				    int		*interp_err);

extern char *loc_error_msg(int error_code);

extern int last_leg(char *phase);

extern int set_sta_pt(Site *site,
		      int  num_sites);

extern int get_sta_index(char *sta);

extern Ar_Info initialize_ar_info(void);

extern Locator_params initialize_loc_params(void);

extern int read_sasc(char *sasc_pathway);

extern int load_single_sasc(int	i);

extern int read_single_sasc(char		*sasc_pathway,
			    char		*sta);

extern void correct_az_slow(int		arid,
			    char		*sta,
			    double		*azimuth,
			    double		*slow,
			    double		delaz,
			    double		delslo,
			    double		*tot_az_err,
			    double		*tot_slow_err);

extern void make_sasc_adj_before_locator(void);

extern Bool apply_sasc_adj_in_locator(void);

extern void free_active_ar_sasc(void);

extern double crude_tt_w_only_phase(
				    char		*phase,
				    double		delta,
				    double		depth);

/*
 * The following prototypes should only be used in conjunction with one
 * another when only the default set of travel-time tables are desired
 * (i.e., no station/phase-dependent knowledge is to be incorporated).
 * All these functions are contained in file, trv_time_default.c, and
 * their use for all but crude travel-time estimates is discouraged.
 */

extern int  setup_default_tt_tables_only(
					 char		*vmodel_filename,
					 char		**phase_types,
					 int		num_phase_types);

extern double fast_travel_time(
			       Bool		extrapolate, 
			       char		*phase, 
			       double		ev_lat,
			       double		ev_lon,
			       double		sta_lat,
			       double		sta_lon,
			       double		delta,
			       double		depth, 
			       double		esaz,
			       double		sta_elev, 
			       double		*horiz_slow, 
			       int		*interp_code);

extern double travel_time_wo_corr(Bool	extrapolate, 
				  char	*phase, 
				  double	delta,
				  double	depth, 
				  double	sta_elev, 
				  double	*horiz_slow, 
				  int		*interp_code);

extern double super_fast_travel_time(
				     char		*phase, 
				     double		distance,
				     double		depth);

extern int get_phase_index(char *phase);

extern int setup_blk_facilities 
      (char     *blk_prefix , 
       Site     *site       , 
       int      num_sites   );

extern Bool blocked_path
      (char *sta         ,
       double lat_source ,
       double lon_source ,
       Bool     *back_branch );

extern Bool blocked_ellipse
      (char *sta , 
       double lat_source, 
       double lon_source, 
       double smajax, 
       double sminax, 
       double strike, 
       Bool *back_branch );

extern Bool blocked_path_max_dist
      (char *sta , double lat_source, 
       double lon_source, double *dist,
       double *az );

extern Bool blocked_ellipse_dist
      (char *sta , double lat_source, 
       double lon_source, double smajax, double sminax, 
       double strike, double *dist_center,
       double *dist_min, double *dist_max,
       double *az_min,   double *az_max );

extern int num_blocked_stations
       (void);

extern Site *get_blocked_site_pt
       (int sta);

extern double get_blk_radius(void);

extern int set_blk_radius(double rad);

extern double compute_deltim 
       (double snr, double max_snr, double min_snr, 
	double max_deltim, double min_deltim);

extern void dswap
	(int n, double *dx, int incx, double *dy, int incy);

extern void dsvdc
	(double *px, int ldx, int n, int p, double *ps, double *pe, double *pu,
	 int ldu, double *pv, int ldv, double *pwork, int job, int *info);

extern void daxpy
	(int n, double da, double *dx, int incx, double *dy, int incy);

extern void dscal
	(int n, double da, double *dx, int incx);

extern void drotg
	(double *da, double *db, double *c, double *s);

extern void drot
	(int n, double *dx, int incx, double *dy, int incy, double c,
	double s);

#endif /* _LIBLOC_H_ */

