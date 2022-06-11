
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * FILENAME
 *	mag_defs.h

 * DESCRIPTION
 *	Includes magnitude dependent define statements. 

 * SccsId:  @(#)mag_defs.h	4.1	10/30/97
 */

#ifndef _MAG_DEFS_H_
#define _MAG_DEFS_H_

/* Magnitude Error Table Values */
#define	MDreadErr1	 1
#define	MDreadErr2	 2
#define	MDreadErr3	 3
#define	MDreadErr4	 4
#define	SSgetErr1	 5
#define	SSgetErr2	 6

#define	NET_AVG		 0
#define	MLE		 1
#define	MLE_W_BOOTS	 2
#define	MEAS_SIGNAL	 0
#define	CLIPPED		 1
#define	NON_DETECT	 2
#define	NUM_AMP_TYPES	 3

#define	NO_MAG_SRC_DPNT_CORR	0
#define	MAG_TEST_SITE_CORR	1

#define NA_MODEL_ERROR	 -999.9

#endif /* _MAG_DEFS_H_ */

