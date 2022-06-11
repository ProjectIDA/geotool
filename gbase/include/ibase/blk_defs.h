/* 
 *      Copyright (c) 1996 Science Applications Internaional Corporation.
 *                 

 * NAME
 *

 * FILE
 *	blk_defs.h
 * 
 * SYNOPSIS

 *
 *
 * DESCRIPTION

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	None.

 * SEE ALSO
 *	

 * AUTHORS 
 *	Header and prototype            RLB 09/12/96
 */  

#ifndef BLK_DEFS_H
#define BLK_DEFS_H

#include <math.h>

#include "aesir.h"
#include "site_Astructs.h"

#define DEG_TO_RAD	(M_PI/180.0)
#define RAD_TO_DEG	(180.0/M_PI)
#define FILE_PREF  "BLK."
#define FILE_SUFF  ".T"

typedef struct radplot Radplot ;

struct radplot
{
	double azimuth  ;
	double max_dist ;
};



typedef struct site_blk Site_Blk ;

struct site_blk
{
	Site    *site ;
	int     naz   ;
	Radplot *rad  ;
};


typedef struct blk_inf Blk_Inf ;

struct blk_inf
{
	Site_Blk *site ;
	int num_sites  ;
};




#else

#endif          /*         blk_defs.h header file      */






