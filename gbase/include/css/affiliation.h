/*	SccsId:	%W%	%G%	*/
/**
 *	Affiliation relation from CSS 3.0 table definitions.
 *	Network-Station affiations. This is an intermediate realtion by which
 *	seismic stations can be clustered into networks.
 */
#ifndef _AFFILIATION_3_0_H
#define _AFFILIATION_3_0_H

#define AFFILIATION30_LEN	33

/** 
 *  Affiliation struct.
 *  @member net Unique network identifier. Initial value = "-".
 *  @member sta Station identifier. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	net[9];		/* unique network identifier	*/
	char	sta[7];		/* station identifier		*/
	char	lddate[18];	/* load date			*/
} AFFILIATION30;

#define AFFILIATION_RCS30 "%8c%*c%6c%*c%17c%"

#define AFFILIATION_WCS30 "%-8.8s %-6.6s %-17.17s\n"

#define AFFILIATION_WVL30(SP) (SP)->net, (SP)->sta, (SP)->lddate

#define AFFILIATION_NULL30 \
{ \
"-",		/* net	  */ \
"-",		/* sta	  */ \
"-",		/* lddate */ \
}

#endif
