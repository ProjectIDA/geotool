/*	SccsId:	%W%	%G%	*/

#ifndef _NETWORK_3_0_H
#define _NETWORK_3_0_H

/**	Network relation from CSS 3.0 table definitions.
 *	Network description and identification. This relation gives general
 *	information about seismic networks.
 *
 *	@see AFFILIATION30
 */
#define NETWORK30_LEN	137

/** 
 *  Network structure
 *  @member net	Network identifier. Initial value = "-".
 *  @member netname Network name. Initial value = "-".
 *  @member nettype Network type, array, local, world-wide,etc. Initial value = "-".
 *  @member auth Author. Initial value = "-".
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	net[9];		/* network identifier	*/
	char	netname[81];	/* network name	*/
	char	nettype[5];	/* network type, array, local, world-wide,etc.*/
	char	auth[16];	/* author 		*/
	long	commid;		/* comment id		*/
	char	lddate[18];	/* load date		*/
} NETWORK30;

#define NETWORK_WCS30 "%-8.8s %-80.80s %-4.4s %-15.15s %8ld %-17.17s\n"
 
#define NETWORK_WVL30(SP) \
(SP)->net, (SP)->netname, (SP)->nettype, (SP)->auth, (SP)->commid, (SP)->lddate
 
#define NETWORK_NULL30 \
{ \
"-",		/* net */ \
"-",		/* netname */ \
"-",		/* nettype */ \
"-",		/* auth */ \
-1,		/* commid  */ \
"_"		/* lddate */ \
}

#endif
