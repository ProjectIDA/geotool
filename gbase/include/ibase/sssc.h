
/*
 * Copyright (c) 1995-1998 Science Applications International Corporation.
 *

 * NAME
 *	sssc.h

 * DESCRIPTION
 *	Modified C structure declarations for the handling source-specific 
 *	station corrections (SSSC).

 * AUTHOR
 *	Walter Nagy,	  2/ 6/95	Created.
 *	Walter Nagy,	 10/16/95	Updated to hang directly off travel-
 *					time table structure, tt_table.
 *	Walter Nagy,	  3/12/98	Added ability to apply source-dependent
 *					modeling error w/ new float**, mdl_err.
 * SccsId:  @(#)libloc/sssc.h	125.1	06/29/98	SAIC.
 */


#ifndef	SSSC_H
#define	SSSC_H

   typedef	struct	sssc	Sssc;

   struct sssc {
      char	source_region[16];
      double	me_factor;
      float	origin_nw_lat;
      float	origin_nw_lon;
      float	se_lat_bound;
      float	se_lon_bound;
      float	depth;
      int	nlats;
      int	nlons;
      float	*lat_grid_ref;
      float	*lon_grid_ref;
      float	**sta_cor;
      float	**sssc_2nd_deriv;
      float	**mdl_err;
      Sssc	*next_src_region;
      Sssc	*finer_detail;
      char	*sssc_path;
   };

#endif	/* SSSC_H */


