/*	SccsId:	%W%	%G%	*/
/**
 *	Site relation from CSS 3.0 table definitions.
 *	Station location information. Site names and describes a point on the
 *	earth where seismic measurements are made (e.g the location of a
 *	seismic instrument or array). It contains information that normally
 *	changes infrequently, such as location. In addition, site contains
 *	fields to describe the offset of a station relative to an array
 *	reference location. Global data integrity implies that the sta/ondate
 *	in site be consistent with the sta/chan/ondate in sitechan.
 */
#ifndef _SITE_3_0_H
#define _SITE_3_0_H

#define SITE30_LEN	155

/** 
 *  Site structure.
 *  @member sta Station identifier. Initial value = "-".
 *  @member ondate Julian start date. Initial value = -1.
 *  @member offdate Julian off date. Initial value = -1.
 *  @member lat Latitude. Initial value = -999.
 *  @member lon Longitude. Initial value = -999.
 *  @member elev Elevation. Initial value = -999.
 *  @member staname Station description. Initial value = "-".
 *  @member statype Station type: single station, virt array, etc.. Initial value = "-".
 *  @member refsta Reference station for array members. Initial value = "-".
 *  @member dnorth Offset from array reference (km). Initial value = 0.
 *  @member deast Offset from array reference (km). Initial value = 0.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	sta[7];		/* station identifier	*/
	long	ondate;		/* Julian start date	*/
	long	offdate;	/* Julian off date	*/
	double	lat;		/* latitude		*/
	double	lon;		/* longitude		*/
	double	elev;		/* elevation		*/
	char	staname[51];	/* station description	*/
	char	statype[5];	/* station type: single station, virt array,
					etc. */
	char	refsta[7];	/* reference station for array members */
	double	dnorth;		/* offset from array reference (km) */
	double	deast;		/* offset from array reference (km) */
	char	lddate[18];	/* load date		*/
} SITE30;

#define SITE_RCS30 "%6c%*c%8ld%*c%8ld%*c%9lf%*c%9lf%*c%9lf%*c%50c%*c%4c%*c%6c%*c%9lf%*c%9lf%*c%17c"

#define SITE_RVL30(SP) \
(SP)->sta, &(SP)->ondate, &(SP)->offdate, &(SP)->lat, &(SP)->lon, &(SP)->elev, \
(SP)->staname, (SP)->statype, (SP)->refsta, &(SP)->dnorth, &(SP)->deast, \
(SP)->lddate

#define SITE_WCS30  "%-6.6s %8d %8d %9.4f %9.4f %9.4f %-50.50s %-4.4s %-6.6s %9.4f %9.4f %-17.17s\n"

#define SITE_WVL30(SP) \
(SP)->sta, (SP)->ondate, (SP)->offdate, (SP)->lat, (SP)->lon, (SP)->elev, \
(SP)->staname, (SP)->statype, (SP)->refsta, (SP)->dnorth, (SP)->deast, \
(SP)->lddate

#define SITE_NULL30 \
{ \
"-",		/* sta */ \
-1,		/* ondate */ \
-1,		/* offdate */ \
-999.0,		/* lon */ \
-999.0,		/* lat */ \
-999.0,		/* elev */ \
"-",		/* staname */ \
"-",		/* statype */ \
"-",		/* refsta */ \
0.0,		/* dnorth */ \
0.0,		/* deast */ \
"_"		/* lddate */ \
}

#endif
