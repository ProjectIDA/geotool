/*	SccsId:	%W%	%G%	*/
/**
 *	Xtag relation from CSS 3.0 table definitions.
 */
#ifndef _XTAG_3_0_H
#define _XTAG_3_0_H

#define XTAG30_LEN	81

/** 
 *  Xtag structure.
 *  @member thisid Initial value = -1.
 *  @member thatid Initial value = -1.
 *  @member thisname key (arid, orid, etc.). Initial value = "-".
 *  @member thatname key (arid, orid, etc.). Initial value = "-".
 *  @member dbname dbname or [S|C]/sta/stadate. Initial value = "-".
 *  @member lddate load date. Initial value = "-".
 */
typedef struct
{
	long	thisid;	
	long	thatid;	
	char    thisname[9];	/* key (arid, orid, etc.)	*/
	char    thatname[9];	/* key (arid, orid, etc.)	*/
	char	dbname[26];	/* dbname or [S|C]/sta/stadate  */
	char	lddate[18];	/* load date			*/
} XTAG30;

#define XTAG_RCS30 "%8ld%*c%8ld%*c%8c%*c%8c%*c%25c%*c%17c%"

#define XTAG_WCS30 "%8ld %8ld %-8.8s %-8.8s %-25.25s %-17.17s\n"

#define XTAG_WVL30(SP) (SP)->thisid, (SP)->thatid, (SP)->thisname, (SP)->thatname, (SP)->dbname, (SP)->lddate

#define XTAG_NULL30 \
{ \
-1,                     /* thisid	*/ \
-1,                     /* thatid	*/ \
"-",                    /* thisname	*/ \
"-",                    /* thatname	*/ \
"-",                    /* dbname	*/ \
"-",                    /* lddate       */ \
}
#endif
