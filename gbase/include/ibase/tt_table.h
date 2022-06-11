
/*
 * Copyright (c) 1994-1995 Science Applications International Corporation.
 *

 * FILENAME
 *	tt_table.h

 * DESCRIPTION
 *	C structure declarations for the handling travel-time, modelling
 *	error and new-style elliptiticity correction tables as needed for
 *	use by GSO staff for SAIC geophysical software.

 * SccsId:  @(#)libloc/tt_table.h	125.1	06/29/98	SAIC.
 */


#ifndef	TT_TABLE_H
#define	TT_TABLE_H

#include "sssc.h"

typedef	struct	ec_table {
	int	num_dists;
	int	num_depths;
	float	*dist_samples;
	float	*depth_samples;
	float	**t0;
	float	**t1;
	float	**t2;
} EC_Table;

typedef	struct	model_error {
	float	bulk_var;
	int	num_dists;
	int	num_depths;
	float	*dist_samples;
	float	*depth_samples;
	float	*dist_var;
	float	**dist_depth_var;
} Model_Error;

typedef	struct	tt_table	TT_Table;

struct tt_table {
	char		phase[9];
	char		vmodel[16];
	int		last_leg;
	int		num_dists;
	int		num_depths;
	float		in_hole_dist[2];     /* 0 = min. dist; 1 = max. dist */
	float		*dist_samples;
	float		*depth_samples;
	float		**trv_time;
	EC_Table	*ec_table;
	Model_Error	*model_error;
} ;

typedef	struct	list_of_phases	List_of_Phases;

struct	list_of_phases {
	char		phase[9];
	int		phase_index;
	List_of_Phases	*next;
} ;

typedef	struct	model_descrip {
	char		vmodel[16];
	char		*dir_pathway;
	List_of_Phases	*list_of_phases;
	Bool		sssc_dir_exists;
} Model_Descrip;

typedef	struct	sta_phase_model	Sta_Phase_Model;

struct	sta_phase_model {
	char		sta[7];
	char		phase[9];
	int		phase_index;
	int		vel_index;
	float		sed_vel;
	float		bulk_sta_corr;
	Sssc		*sssc;
	char		source_region[16]; /* suffix of SSSC file from vmsf */
} ;

typedef	struct	phase_ptr	Phz_Pt;

struct	phase_ptr {
	char	phase[9];
	int	spm_index;
	Phz_Pt	*next;
} ;

typedef	struct	sta_pt {
	char    sta[7];  /*JG*/
	Phz_Pt	*phase_ptr;
} Sta_Pt;

#endif	/* TT_TABLE_H */

