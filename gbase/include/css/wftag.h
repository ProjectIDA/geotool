/*	SccsId:	%W%	%G%	*/
/**
 *	Wftag relation from CSS 3.0 table definitions.
 *	Waveform mapping file. The wftag relation links various identifiers,
 *	e.g. origin id, arrival id, stassoc id, to waveform id. All of the
 *	linkages could be determined indirectly using sta, chan and time.
 *	However, it is more efficient to predetermine them.
 */
#ifndef _WFTAG_3_0_H
#define _WFTAG_3_0_H

#define WFTAG30_LEN	44

/** 
 *  Wftag structure.
 *  @member tagname Key (arid, orid, etc.). Initial value = "-".
 *  @member tagid Tagname value. Initial value = -1.
 *  @member wfid Waveform id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char    tagname[9];	/* key (arid, orid, etc.)	*/
	long    tagid;		/* tagname value		*/
	long    wfid;		/* waveform id			*/
	char	lddate[18];	/* load date			*/
} WFTAG30;

#define WFTAG_RCS30 "%8c%*c%8ld%*c%8ld%*c%17c%"

#define WFTAG_WCS30 "%-8.8s %8ld %8ld %-17.17s\n"

#define WFTAG_WVL30(SP) (SP)->tagname, (SP)->tagid, (SP)->wfid, (SP)->lddate

#define WFTAG_NULL30 \
{ \
"-",                    /* tagname	*/ \
-1,                     /* tagid        */ \
-1,                     /* wfid         */ \
"-",                    /* lddate       */ \
}

#endif
