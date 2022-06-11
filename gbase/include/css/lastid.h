/*	SccsId:	%W%	%G%	*/
/**
 *	Lastid relation from CSS 3.0 table definitions.
 */
#ifndef _LASTID_3_0_H
#define _LASTID_3_0_H

#define LASTID30_LEN	42

/** 
 *  Lastid structure.
 *
 *  @member keyname Id name (arid, orid, etc.).
 *  @member keyvalue Last value used for that id.
 *  @member lddate Load date.
 */

typedef struct
{
	char	keyname[16];	/* id name (arid, orid, etc.)	*/
	long	keyvalue;	/* last value used for that id	*/
	char	lddate[18];	/* load date			*/
} LASTID30;

#define LASTID_WCS30 "%-15.15s %8ld %-17.17s\n"

#define LASTID_WVL30(SP) (SP)->keyname, (SP)->keyvalue, (SP)->lddate

#endif
