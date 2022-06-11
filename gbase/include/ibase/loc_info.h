
/*
 * Copyright (c) 1992-1996 Science Applications International Corporation.
 *

 * FILENAME
 *	loc_info.h

 * DESCRIPTION
 *	C structure declarations for the handling local locator library 
 *	variables.

 * SccsId:  @(#)libloc/loc_info.h	121.2	10/08/97	SAIC.
 */


#ifndef	_LOCATOR_INFO_H_
#define	_LOCATOR_INFO_H_

typedef struct locator_info {
	int	nd_used;
	int	num_params;
	int	np;
	int	tot_ndf;
	double	azimuthal_gap;
	double	condition_number;
	double	sigma_hat;
	double	snssd;
	double	sum_sqrd_res;
	double	applied_damping;
	double	rank;
	int	*sta_index;
	int	*phase_index;
	int	*spm_index;
} Locator_info;

/* Define statements for handling last leg of a given phase */
#define	BOGUS_PHASE_TYPE	-1
#define	JUST_IGNORE_THIS_PHASE	0
#define	LAST_LEG_IS_P		1
#define	LAST_LEG_IS_S		2

/* Define statements for long-period LR/LQ phases */
#define	LR		0
#define	LQ		1
#define	LR_PHASE_INDEX	99998
#define	LQ_PHASE_INDEX	99999
 
#define T_PHASE_INDEX  99997 /*JG*/
#define H_O_PHASE_INDEX  99996 /*JG*/
#define I_PHASE_INDEX  99995 /*JG*/
 

/* Locator Error Table Values */
#define	GLerror1	 1
#define	GLerror2	 2
#define	GLerror3	 3
#define	GLerror4	 4
#define	GLerror5	 5
#define	GLerror6	 6
#define	GLerror7	 7
#define	GLerror8	 8
#define	GLerror9	 9
#define	GLerror10	10
#define	GLerror11	11
#define	GLerror12	12
#define	GLerror13	13
#define	GLerror14	14
#define	GLerror15	15
#define	GLerror16	16
#define	TTerror1	17
#define	TTerror2	18
#define	TTerror3	19
#define	TTerror4	20
#define	TTerror5	21
#define	SSerror1	22
#define	SCerror1	23
#define	SRerror1	24
#define	TSerror1	25
#define	TSerror2	26
#define	ECerror1	27
#define	LPerror1	28
#define	TTerror6	29
#define HYerror1        30  /*JG*/
#define HYerror2        31  /*JG*/
#define HYerror3        32  /*JG*/
#define HYerror4        33  /*JG*/


#endif	/* _LOCATOR_INFO_H_ */
