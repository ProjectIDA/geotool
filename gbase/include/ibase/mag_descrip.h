
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * FILENAME
 *	mag_descrip.h

 * DESCRIPTION
 *	C structure declarations for the handling various magnitude-specific 
 *	structure needed by SAIC magnitude software. 

 * SccsId:  @(#)mag_descrip.h	4.2	03/06/98	SAIC.
 */


#ifndef	MAG_DESCRIP_H
#define	MAG_DESCRIP_H

#include "aesir.h"

#include "db_netmag.h"
#include "db_stamag.h"
#include "db_amplitude.h"
#include "tl_table.h"

typedef	struct	mag_descrip {
	char	magtype[7];
	char	TLtype[9];
	char	det_amptype[9];
	char	ev_amptype[9];
	int	algo_code;
	float	dist_min;
	float	dist_max;
	float	sglim1;
	float	sglim2;
	float	sgbase;
	Bool	apply_wgt;
	float	def_sta_corr;
	float	def_sta_corr_error;
	char	orig_det_amptype[9];
	char	orig_ev_amptype[9];
	int	orig_algo_code;
	float	orig_dist_min;
	float	orig_dist_max;
	float	orig_sglim1;
	float	orig_sglim2;
	float	orig_sgbase;
	Bool	orig_apply_wgt;
} Mag_Descrip;

typedef struct mag_sta_tltype
{
	char	sta[7];
	char	TLtype[9];
	float	bulk_sta_corr;
	float	bulk_sta_corr_error;
} Mag_Sta_TLType;

typedef struct mag_cntrl
{
	char	magtype[7];
	char	TLtype[9];
	char	det_amptype[9];
	char	ev_amptype[9];
	int	algo_code;
	double	dist_min;
	double	dist_max;
	double	sglim1;
	double	sglim2;
	double	sgbase;
	Bool	apply_wgt;
} Mag_Cntrl;

typedef struct sm_info
{
	int	mag_error_code;
	double	sta_magnitude;
	int	src_dpnt_corr_type;
	double	total_mag_corr;
	double	mc_table_value;
	double	bulk_static_sta_corr;
	double	bulk_sta_corr_error;
	double	src_dpnt_corr;
	double	model_error;
	double	meas_error;
	double	model_plus_meas_error;
	double	mag_cor_deriv[4];
	char	mmodel[16];
	char	lddate[18];
} SM_Info;

typedef	struct	sm_aux
{
	Bool	detect_based;		/* F: Event-based; T: Detection-based */
	Bool	manual_override;	/* F: Automatic; T: Manual override */
	Bool	clipped;		/* F: Not clipped; T: Clipped measure */
	int	sig_type;		/* Derivable (call get_sig_type() */
	double	wt;			/* Station weighting */
} SM_Aux;

typedef struct magnitude
{
	Bool		mag_computed;	/* Sucessful network mag. computed? */
	Bool		mag_write;	/* Network mag. could not be computed, 
					 * but write to DB to retain stamags? */
	Mag_Cntrl	mag_cntrl;	/* Local control settings for magtype */
	Netmag		netmag;		/* Singular netmag record */
	Stamag		**stamag;	/* Array of ptrs to stamag records */
	Amplitude	**amplitude;	/* Array of ptrs to amplitude records */
	SM_Aux		*sm_aux;	/* Station magnitude auxilary info */
	int		count;		/* Number of stamag/amplitude entries */
} MAGNITUDE, *Magnitude;

#endif	/* MAG_DESCRIP_H */

