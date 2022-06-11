
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * FILENAME
 *	tl_table.h

 * DESCRIPTION
 *	C structure declarations for the handling various transmission 
 *	loss (TL) and related modelling error tables as needed for use 
 *	by GSO staff for SAIC geophysical software.  The most common, and
 *	first to be implemented, transmission loss table is the magnitude 
 *	correction table.

 * SccsId:  @(#)tl_table.h	4.1	10/30/97	SAIC.
 */


#ifndef	TL_TABLE_H
#define	TL_TABLE_H

typedef	struct	tl_mdl_err {
	float		bulk_var;
	int		num_dists;
	int		num_depths;
	float		*dist_samples;
	float		*depth_samples;
	float		*dist_var;
	float		**dist_depth_var;
} TL_Mdl_Err;

typedef	struct	tl_ts_cor {
	int		num_sta;
	int		reg_num;
	char		reg_name_id[9];
	char		TLtype[9];	/* magtype ? */
	/* char		model[16]; */
	char		**sta;
	double		*ts_corr;
} TL_TS_Cor;


typedef	struct	tl_table	TL_Table;

struct tl_table {
	char		TLtype[9];
	char		model[16];
	char		phase[9];
	char		chan[9];
	int		num_dists;
	int		num_depths;
	float		in_hole_dist[2];     /* 0 = min. dist; 1 = max. dist */
	float		*dist_samples;
	float		*depth_samples;
	float		**tl;
	TL_Mdl_Err	*tl_mdl_err;
	int		num_ts_regions;
	TL_TS_Cor	*tl_ts_cor;
} ;

typedef	struct	list_of_phz	List_of_Phz;

struct	list_of_phz {
	char		phase[9];
	int		tl_index;
	List_of_Phz	*next;
} ;

typedef	struct	tl_model_path {
	char		model[16];
	char		*dir_pathway;
} TL_Model_Path;

typedef	struct	tltype_model_descrip {
	char		TLtype[9];
	char		model[16];
	int		model_index;
	Bool		phase_dependency;
	List_of_Phz	*list_of_phz;
} TLType_Model_Descrip;

typedef	struct	sta_tl_model	Sta_TL_Model;

struct	sta_tl_model {
	char		sta[7];
	char		TLtype[9];
	char		model[16];
	char		phase[9];
	char		chan[9];
	int		tl_index;
	int		model_index;
} ;

typedef	struct	tl_ptr	TL_Pt;

struct	tl_ptr {
	char		TLtype[9];
	char		phase[9];
	char		chan[9];
	int		stm_index;
	int		tl_index;
	TL_Pt		*next;
} ;

typedef	struct	sta_pt {
	TL_Pt		*tl_ptr;
} Sta_Pt;

#endif	/* TL_TABLE_H */

