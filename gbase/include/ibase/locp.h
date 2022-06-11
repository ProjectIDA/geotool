
/*
 * Copyright (c) 1994-1998 Science Applications International Corporation.
 *

 * locp.h include file.  Only for local Prototype definitions for routines 
 * inside of library, libloc.
 *
 * SccsId:  @(#)libloc/locp.h	125.1	06/29/98	SAIC.
 */

#ifndef _LOCP_H
#define _LOCP_H

#include <stdio.h>

#include "aesir.h"	/* For Proto definition. */

#include "loc_params.h"
#include "loc_info.h"
#include "tt_table.h"
#include "site_Astructs.h"
#include "origin_Astructs.h"
#include "origerr_Astructs.h"
#include "arrival_Astructs.h"
#include "assoc_Astructs.h"

typedef struct LM_params {
   Origin          *origin;
   Origerr         *origerr;
   Arrival         *arrival;
   Assoc           *assoc;
   Locator_params  *locator_params;
   Locator_info    *locator_info;
   Ar_Info         *ar_info;
   Site            *sites;
   int             num_sites;
   int             num_obs;
   int             num_tt_used;
   double          *torg;
   double          *At;
   double          *resid2;
   double          *dsd2;
   double          *az_used_in_loc;
   double          *wt_rms;
   double          *unwt_rms;
   Bool            fix_depth_this_iter;
   int             np;
   int             nd;
   int             iter;
} lm_params;

typedef struct SOLUTION {
   double          torg;
   double          lat;
   double          lon;
   double          depth;
   double          wt_rms;
   double          unwt_rms;
   double          sdobs;
   int             nd;
} best_solution;


extern int compute_hypo(Origin		**w_origin,
			Origerr		**w_origerr,
			Arrival		*w_arrival,
			Assoc		**w_assoc,
			Locator_params	*locator_params,
			Locator_info	*locator_info,
			Ar_Info		**ar_info_ptr,
			Site		*sites,
			int		num_sites,
			int		num_obs,
			FILE		*ofp,
			int		verbose,
			double		*torg);

extern int compute_hypo_lm(Origin		*origin,
			   Origerr		*origerr,
			   Arrival		*arrival,
			   Assoc		*assoc,
			   Locator_params	*locator_params,
			   Locator_info	*locator_info,
			   Ar_Info		*ar_info,
			   Site		*sites,
			   int		num_sites,
			   int		num_obs,
			   FILE		*ofp,
			   int		verbose,
			   double		*torg,
			   double		*At,
			   double		*resid2,
			   double		*dsd2,
			   double		*az_used_in_loc,
			   double		*init,
			   Bool		svd_conv);

extern int best_guess(Site		*sites,
		      int		num_sites,
		      Arrival		*arrival,
		      Assoc		*assoc,
		      Ar_Info		*ar_info,
		      int		num_obs,
		      Locator_info	*locator_info,
		      FILE		*ofp,
		      int		verbose,
		      double		*lat_init,
		      double		*lon_init);

extern void print_loc_results(Origin		*origin,
			      Origerr		*origerr,
			      Arrival		*arrival,
			      Assoc		*assoc,
			      Locator_params	*locator_params,
			      Locator_info	*locator_info,
			      Ar_Info		*ar_info,
			      int		num_obs,
			      double		torg,
			      FILE		*ofp,
			      int		verbose,
			      int		ierr);

extern int solve_via_svd(int		icov,
			 int		nd,
			 int		np,
			 int		maxp,
			 double		*at,
			 double		*d,
			 double		damp,
			 double		*cnvgtst,
			 double		*condit,
			 double		*xsol,
			 double		covar[][4],
			 double		*data_importances,
			 double		*applied_damping,
			 double		*rank);

extern int read_tt_tables(Bool		only_read_default_tbls,
			  char		*dir_prefix,
			  char		**new_phase_list,
			  int		num_phases,
			  TT_Table	**tt_table_ptr,
			  int		*total_num_phases,
			  Model_Descrip	**model_descrip_ptr,
			  int		*num_models,
			  Sta_Phase_Model	**sta_phase_model_ptr,
			  int		*num_sta_phase_models);

extern int denuis(char		*dtype2,
		  int		nd,
		  int		np,
		  double		*residual,
		  double		*data_std_err,
		  double		*at,
		  double		*dmean);

extern void f_test(int		m,
		   int		n,
		   double		p,
		   double		*x);

extern int read_sssc(Model_Descrip	**model_descrip_ptr,
		     int		num_models,
		     Sta_Phase_Model	**sta_phase_model_ptr,
		     int		num_sta_phase_models);

extern int read_single_sssc_file(
                                 Sta_Phase_Model	spm,
				 int		sssc_level,
				 char		verbose);

extern int apply_sssc(double		ev_lat,
		      double		ev_lon,
		      Sta_Phase_Model	spm,
		      double		*sssc_correct,
		      double		*me_factor,
		      double		*me_sssc,
		      int		sssc_level,
		      char		verbose);

extern int read_srst(char		*vmodel_pathway,
		     char		*vmodel);

extern Bool apply_srst(char		*sta,
		       double		ev_geoc_lat,
		       double		ev_lon,
		       double		ev_geoc_co_lat,
		       double		ev_depth,
		       double		*correct,
		       double		*var_wgt);

extern void set_srst_region_number(void);
extern int get_srst_region_number(void);

extern int read_ts_corr(char		*vmodel_pathway,
			char		*vmodel);

extern double get_ts_corr(char		*ts_region,
			  char		*phase,
			  char		*sta,
			  Bool		*ts_corr_found);

extern double crude_but_quick_dist(
				   int		phase_id,
				   double	tcalc);

extern double crude_but_quick_trv_time(
				       int		phase_id,
				       double		delta,
				       double		depth);

extern int read_ec_table(char		*file_name,
			 EC_Table	**ec_table_ptr);

extern double get_ec_from_table(
				EC_Table	*ec_table,
				double		delta,
				double		esaz,
				double		ev_geoc_co_lat,
				double		ev_depth);

extern int *ivect(int		nl,
		  int		nh);

extern double *dvect(int		nl,
		     int		nh);

extern double **dmatr(int		nrl,
		      int		nrh,
		      int		ncl,
		      int		nch);

extern void free_ivect(int		*v,
		       int		nl,
		       int		nh);

extern void free_dvect(double		*v,
		       int		nl,
		       int		nh);

extern void free_dmatr(double		**m,
		       int		nrl,
		       int		nrh,
		       int		ncl,
		       int		nch);

extern double dfridr(double		x,
		     double		h,
		     double		*err,
		     int		n,
		     lm_params	*p);

extern double get_slow(double		depth,
		       int		n,
		       lm_params	*p);

extern int mrqcof(lm_params	*pars,
		  double		**alpha,
		  double		*beta);

extern int gaussj(double		**a,
		  int		n,
		  double		**b,
		  int		m);

extern int get_resids_and_derivs(lm_params	*params,
				 Bool 		deriv);


extern int last_leg(char *phase);

extern double ddot(int n, double *dx, int incx, double *dy,int incy);

extern double dnrm2(int n, double *dx, int incx);


#endif /* _LOCP_H */

