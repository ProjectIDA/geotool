/*	SccsId:	%W%	%G%	*/
/**
 *	Netmag relation from CSS 3.0 table definitions.
 *	Network magnitude. This table summarizes estimates of network
 *	magnitudes of different types for an event. Each network magnitude
 *	has a unique magid. Station magnitudes used to compute the network
 *	magnitude can be found in the relation stamag.
 *
 *	@see STAMAG30
 */
#ifndef _NETMAG_3_0_H
#define _NETMAG_3_0_H

#define NETMAG30_LEN	110

/** 
 *  Netmag structure.
 *  @member magid Magnitude id. Initial value = -1.
 *  @member net Network identifier. Initial value = "-".
 *  @member orid Origin id. Initial value = -1.
 *  @member evid Event id. Initial value = -1.
 *  @member magtype Magnitude type (mb). Initial value = "-".
 *  @member nsta Number of stations used. Initial value = -1.
 *  @member magnitude Magnitude. Initial value = -999.
 *  @member uncertainty Magnitude uncertainty. Initial value = -999.
 *  @member auth author. Initial value = "-".
 *  @member commid comment id. Initial value = -1.
 *  @member lddate load date. Initial value = "-".
 */
typedef struct
{
	long	magid;		/* magnitude id     	*/
	char	net[9];		/* network identifier	*/
	long	orid;		/* origin id		*/
	long	evid;		/* event id		*/
	char	magtype[7];	/* magnitude type (mb)	*/
	int	nsta;		/* number of stations used */
	float	magnitude;	/* magnitude 		*/
	float	uncertainty;	/* magnitude uncertainty */
	char	auth[16];	/* author 		*/
	long	commid;		/* comment id		*/
	char	lddate[18];	/* load date		*/
} NETMAG30;

#define NETMAG_RCS30 "%6c%*c%8ld%*c%8ld%*c%9f%*c%9f%*c%9f%*c%50c%*c%4c%*c%6c%*c%9f%*c%9f%*c%17c"

#define NETMAG_RVL30(SP) \
(SP)->sta, &(SP)->ondate, &(SP)->offdate, &(SP)->lat, &(SP)->lon, &(SP)->elev, \
(SP)->staname, (SP)->statype, (SP)->refsta, &(SP)->dnorth, &(SP)->deast, \
(SP)->lddate

#define NETMAG_WCS30  "%-6.6s %8d %8d %9.4f %9.4f %9.4f %-50.50s %-4.4s %-6.6s %9.4f %9.4f %-17.17s\n"

#define NETMAG_WVL30(SP) \
(SP)->sta, (SP)->ondate, (SP)->offdate, (SP)->lat, (SP)->lon, (SP)->elev, \
(SP)->staname, (SP)->statype, (SP)->refsta, (SP)->dnorth, (SP)->deast, \
(SP)->lddate

#define NETMAG_NULL30 \
{ \
-1,		/* magid  */ \
"-",		/* network */ \
-1,		/* orid  */ \
-1,		/* evid  */ \
"-",		/* magtype */ \
-1,		/* nsta */ \
-999.0,		/* magnitude */ \
-999.0,		/* uncertainty */ \
"-",		/* auth */ \
-1,		/* commid  */ \
"_"		/* lddate */ \
}

#endif
