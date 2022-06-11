
/*
 * Copyright (c) 1992-1996 Science Applications International Corporation.
 *

 * FILENAME
 *	loc_defs.h

 * DESCRIPTION
 *	Local locator library define statements.

 * SccsId:  @(#)libloc/loc_defs.h	125.1	06/29/98	SAIC.
 */

#include <math.h>


#ifndef	MAX_PARAM
#define	MAX_PARAM	4
#endif

#ifndef	RAD_TO_DEG			/* Convert radians to degrees */
#define	RAD_TO_DEG	(180.0/M_PI)
#endif

#ifndef	DEG_TO_RAD			/* Convert degrees to radians */
#define	DEG_TO_RAD	(M_PI/180.0)
#endif

#ifndef	DEG_TO_KM			/* Convert degrees to kilometers */
#define	DEG_TO_KM	111.195
#endif

#ifndef	KM_TO_DEG			/* Convert kilometers to degrees */
#define	KM_TO_DEG	0.00899322
#endif

#ifndef	RADIUS_EARTH			/* Average radius of the Earth (km) */
#define	RADIUS_EARTH	6371.0
#endif

#ifndef	MAX_DEPTH			/* Max. credible depth for quake (km) */
#define	MAX_DEPTH	700.0
#endif

#ifndef	COND_NUM_LIMIT			/* Condition number limit */
#define	COND_NUM_LIMIT	30000.0
#endif

#ifndef	MIN
#define	MIN(x, y)	(((x) < (y)) ? (x) : (y))
#endif

#ifndef	MAX
#define	MAX(x, y)	(((x) > (y)) ? (x) : (y))
#endif

#ifndef	SIGN
#define	SIGN(x, y)	((y) >= 0 ? -(x) : (x))
#endif

#ifndef	VALID_SEAZ
#define	VALID_SEAZ(x)	(((x) >= 0.0) && ((x) <= 360.0))
#endif

#ifndef	VALID_SLOW
#define	VALID_SLOW(x)	((x) >= 0.0)
#endif

#ifndef	NUM_ELEMENTS
#define	NUM_ELEMENTS(a)	(sizeof(a)/sizeof(a[0]))
#endif

#ifndef	MCOPY
#ifdef	__svr4__
#include <string.h>
#define	MCOPY(to, from, n) memmove ((void *)(to), (const void *)(from), (n))
#else
#include <memory.h>
#define	MCOPY(to, from, n) bcopy ((char *)(from), (char *)(to), (n))
#endif	/* __svr4__ */
#endif

