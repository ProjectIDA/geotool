/*	SccsId:	%W%	%G%	*/
/**
 *	Stamag relation from CSS 3.0 table definitions.
 *	Station magnitude. This table summarizes station magnitude estimates
 *	based upon measurements made on specific seismic phases.
 *
 *	@see NETMAG30
 */
#ifndef _STAMAG_3_0_H
#define _STAMAG_3_0_H

#define STAMAG30_LEN	161

/** 
 *  Stamag structure.
 *  @member magid Magnitude id. Initial value = -1.
 *  @member ampid Amplitude id. Initial value = -1.
 *  @member sta Station identifier. Inital value = "-".
 *  @member arid Arrival id. Inital value = -1.
 *  @member orid Origin id. Inital value = -1.
 *  @member evid Event id. Inital value = -1.
 *  @member phase Associated phase. Inital value = "-".
 *  @member delta Station-to-event dist. Inital value = -999.
 *  @member magtype Magnitude type (mb). Inital value = "-".
 *  @member magnitude Magnitude. Inital value = -999.
 *  @member uncertainty Magnitude uncertainty. Inital value = -999.
 *  @member magres Magnitude residual. Inital value = -999.
 *  @member magdef Flag if mag is defining. Inital value = "-".
 *  @member mmodel Magnitude model. Inital value = "-".
 *  @member auth Author. Inital value = "-".
 *  @member commid Comment id. Inital value = "-".
 *  @member lddate Load date. Inital value = "-".
 */
typedef struct
{
	long	magid;		/* magnitude id     	*/
	long	ampid;		/* amplitude id     	*/
	char	sta[7];		/* station identifier	*/
	long	arid;		/* arrival id     	*/
	long	orid;		/* origin id		*/
	long	evid;		/* event id		*/
	char	phase[9];	/* associated phase 	*/
	float	delta;		/* station-to-event dist*/
	char	magtype[7];	/* magnitude type (mb)	*/
	float	magnitude;	/* magnitude 		*/
	float	uncertainty;	/* magnitude uncertainty */
	float	magres;		/* magnitude residual   */
	char	magdef[2];	/* flag if mag is defining */
	char	mmodel[15];	/* magnitude model	*/
	char	auth[16];	/* author 		*/
	long	commid;		/* comment id		*/
	char	lddate[18];	/* load date		*/
} STAMAG30;

#define STAMAG_RCS30 "%6c%*c%8ld%*c%8ld%*c%9f%*c%9f%*c%9f%*c%50c%*c%4c%*c%6c%*c%9f%*c%9f%*c%17c"

#define STAMAG_RVL30(SP) \
(SP)->sta, &(SP)->ondate, &(SP)->offdate, &(SP)->lat, &(SP)->lon, &(SP)->elev, \
(SP)->staname, (SP)->statype, (SP)->refsta, &(SP)->dnorth, &(SP)->deast, \
(SP)->lddate

#define STAMAG_WCS30  "%-6.6s %8d %8d %9.4f %9.4f %9.4f %-50.50s %-4.4s %-6.6s %9.4f %9.4f %-17.17s\n"

#define STAMAG_WVL30(SP) \
(SP)->sta, (SP)->ondate, (SP)->offdate, (SP)->lat, (SP)->lon, (SP)->elev, \
(SP)->staname, (SP)->statype, (SP)->refsta, (SP)->dnorth, (SP)->deast, \
(SP)->lddate

#define STAMAG_NULL30 \
{ \
-1,		/* magid  */ \
-1,		/* ampid  */ \
"-",		/* sta */ \
-1,		/* arid  */ \
-1,		/* orid  */ \
-1,		/* evid  */ \
"-",		/* phase */ \
-999.0,		/* delta */ \
"-",		/* magtype */ \
-999.0,		/* magnitude */ \
-999.0,		/* uncertainty */ \
-999.0,		/* magres */ \
"-",		/* magdef */ \
"-",		/* mmodel */ \
"-",		/* auth */ \
-1,		/* commid  */ \
"_"		/* lddate */ \
}

#endif
