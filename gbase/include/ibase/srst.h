
/*
 * Copyright (c) 1994-1995 Science Applications International Corporation.
 *

 * FILENAME
 *	srst.h

 * DESCRIPTION
 *	C structure declarations for the handling SRST corrections used
 *	by AFTAC.

 * SccsId:  @(#)libloc/srst.h	125.1	06/29/98	SAIC.
 */


#ifndef	SRST_H
#define	SRST_H

static	int	num_srst_regions = 0;

typedef struct srst {
	int	num_poly_pairs;
	int	num_sta_w_srst;
	int	reg_num;
	char	**sta;
	int	*type;
	double	*poly_lat_data;
	double	*poly_lon_data;
	double	ref_coord[3];
	double	*weight;
	double	*c0;
	double	*c1;
	double	*c2;
	double	*c3;
} Srst;

#endif	/* SRST_H */
