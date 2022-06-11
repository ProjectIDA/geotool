
/*
 * Copyright (c) 1993-1995 Science Applications International Corporation.
 *

 * FILENAME
 *	ts_cor.h

 * DESCRIPTION
 *	C structure declarations for the handling test-site corrections used
 *	by AFTAC.

 * SccsId:  @(#)libloc/ts_cor.h	125.1	06/29/98	SAIC.
 */


#ifndef	TS_COR_H
#define	TS_COR_H


typedef struct ts_cor {
	int	num_sta;	/* Number of stations for given region */
	int	reg_num;	/* Should be able to remove in future */
	char	reg_name_id[9];	/* Test-site name (ID) */
	char	phase[9];	/* Phase type */
	char	**sta;		/* List of stations for given region */
	double	*ts_corr;	/* Test-site correction for sta/phase/region */
} TS_Cor;

#endif	/* TS_COR_H */
