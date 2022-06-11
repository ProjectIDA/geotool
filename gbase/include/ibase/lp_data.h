
/*
 * Copyright (c) 1995 Science Applications International Corporation.
 *

 * FILENAME
 *	lp_data.h

 * DESCRIPTION
 *	C structure declarations for the handling LP grid and phase 
 *	velocity table used by GSO staff for SAIC geophysical software.

 * SccsId:  %W%	%G%	Copyright (c) 1995 SAIC.
 */


#ifndef	_LP_DATA_H_
#define	_LP_DATA_H_

typedef struct lp_data {
	int	num_lat_grids;
	int	num_lon_grids;
	double	latlon_spacing;
	short	**grid_indice;
	int	num_periods;
	int	num_indexes;
	double	*period_samples;
	double	**velocity;
} LP_Data;

#endif	/* _LP_DATA_H_ */

