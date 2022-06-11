/*	SccsId:	%W%	%G%	*/
/**
 *	Sitechan relation from CSS 3.0 table definitions.
 *	Station-Channel information. This relation describes the orientation of
 *	a recording channel at the site referenced by sta. This relation
 *	provides information about the various channels (e.g. sz, lz, iz) that
 *	are available at a station and maintains a record of the physical
 *	channel configuration at a site.
 */
#ifndef _SITECHAN_3_0_H
#define _SITECHAN_3_0_H

#define SITECHAN30_LEN	140

/** 
 *  Sitechan structure.
 *  @member sta Station identifier. Initial value = "-".
 *  @member chan Channel identifier. Initial value = "-".
 *  @member ondate Julian start date. Initial value = -1.
 *  @member chanid Channel id. Initial value = -1.
 *  @member offdate Julian off date. Initial value = -1.
 *  @member ctype Channel type. Initial value = "-".
 *  @member edepth Emplacement depth. Initial value = -999.
 *  @member hang Horizontal angle. Initial value = -999.
 *  @member vang Vertical angle. Initial value = -999.
 *  @member descrip Channel description. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	sta[7];		/* station identifier	*/
	char    chan[9];	/* channel identifier	*/
	long	ondate;		/* Julian start date	*/
	long	chanid;		/* channel id		*/
	long	offdate;	/* Julian off date	*/
	char	ctype[5];	/* channel type		*/
	float	edepth;		/* emplacement depth	*/
	float	hang;		/* horizontal angle	*/
	float	vang;		/* vertical angle	*/
	char	descrip[51];	/* channel description	*/
	char	lddate[18];	/* load date		*/
} SITECHAN30;

#define SITECHAN_RCS30  "%6c%*c%8c%*c%8d%*c%8d%*c%8d%*c%4c%*c%9f%*c%6f%*c%6f%*c%50%*c%17c"

#define SITECHAN_RVL30(SP) \
(SP)->sta, (SP)->chan, &(SP)->ondate, &(SP)->chanid, &(SP)->offdate, \
(SP)->ctype, &(SP)->edepth, &(SP)->hang, &(SP)->vang, (SP)->descrip, \
(SP)->lddate

#define SITECHAN_WCS30  "%-6.6s %-8.8s %8d %8d %8d %-4.4s %9.4f %6.1f %6.1f %-50.50s %-17.17s\n"

#define SITECHAN_WVL30(SP) \
(SP)->sta, (SP)->chan, (SP)->ondate, (SP)->chanid, (SP)->offdate, \
(SP)->ctype, (SP)->edepth, (SP)->hang, (SP)->vang, (SP)->descrip, \
(SP)->lddate

#define SITECHAN_NULL30 \
{ \
"-",            /* sta */ \
"-",            /* chan */ \
-1,             /* ondate */ \
-1,             /* chanid */ \
-1,             /* offdate */ \
"-",            /* ctype */ \
-999.0,         /* edepth */ \
-999.0,         /* hang */ \
-999.0,         /* vang */ \
"_",            /* descrip */ \
"_"             /* lddate */ \
}

#endif
