/*	SccsId:	%W%	%G%	*/
/**
 *	Origin relation from CSS 3.0 table definitions.
 *	Summary of hypocentral parameters. Information describing a derived
 *	or reported origin for a particular event is stored in this table.
 */
#ifndef _ORIGIN_3_0_H
#define _ORIGIN_3_0_H

#define ORIGIN30_LEN	237

/** 
 *  Origin structure.
 *  @member lat estimated latitude. Initial value = -999.
 *  @member lon estimated longitude. Initial value = -999.
 *  @member depth estimated depth. Initial value = -999.
 *  @member time epoch time. Initial value = -9999999999.999.
 *  @member orid origin id. Initial value = -1.
 *  @member evid event id. Initial value = -1.
 *  @member jdate julian date. Initial value = -1.
 *  @member nass number of associated phases. Initial value = -1.
 *  @member ndef number of locating phases. Initial value = -1.
 *  @member ndp number of depth phases. Initial value = -1.
 *  @member grn geographic region number. Initial value = -1.
 *  @member srn seismic region number. Initial value = -1.
 *  @member etype event type. Initial value = -1.
 *  @member depdp estimated depth from depth phases. Initial value = -999.
 *  @member dtype depth method used. Initial value = "-".
 *  @member mb body wave magnitude. Initial value = -1.
 *  @member mbid mb magid. Initial value = -1.
 *  @member ms surface wave magnitude. Initial value = -1.
 *  @member msid ms magid. Initial value = -1.
 *  @member ml local magnitude. Initial value = -1.
 *  @member mlid ml magid. Initial value = -1.
 *  @member algorithm location algorithm used. Initial value = "-".
 *  @member auth source/originator. Initial value = "-".
 *  @member commid comment id. Initial value = -1.
 *  @member lddate load date. Initial value = "-".
 */

typedef struct
{
	float	lat;		/* estimated latitude		*/
	float	lon;		/* estimated longitude		*/
	float	depth;		/* estimated depth		*/
	double	time;		/* epoch time			*/
	long	orid;		/* origin id			*/
	long	evid;		/* event id			*/
	long	jdate;		/* julian date			*/
	long	nass;		/* number of associated phases	*/
	long	ndef;		/* number of locating phases	*/
	long	ndp;		/* number of depth phases	*/
	long	grn;		/* geographic region number	*/
	long	srn;		/* seismic region number	*/
	char	etype[8];	/* event type			*/
	float	depdp;		/* estimated depth from depth phases */
	char	dtype[2];	/* depth method used		*/
	float	mb;		/* body wave magnitude		*/
	long	mbid;		/* mb magid			*/
	float	ms;		/* surface wave magnitude	*/
	long	msid;		/* ms magid			*/
	float	ml;		/* local magnitude		*/
	long	mlid;		/* ml magid			*/
	char	algorithm[16];	/* location algorithm used	*/
	char	auth[16];	/* source/originator		*/
	long	commid;		/* comment id			*/
	char	lddate[18];	/* load date			*/
} ORIGIN30;

#define ORIGIN_RCS30 "%9f%*c%9f%*c%9f%*c%17lf%*c%8ld%*c%8ld%*c%8ld%*c%4ld%*c%4ld%*c%4ld%*c%8ld%*c%8ld%*c%7c%*c%9f%*c%1c%*c%7f%*c%8ld%*c%7f%*c%8ld%*c%7f%*c%8ld%*c%15c%*c%15c%*c%8ld%*c%17c"

#define ORIGIN_RVL30(SP) \
&(SP)->lat, &(SP)->lon, &(SP)->depth, &(SP)->time, &(SP)->orid, \
&(SP)->evid, &(SP)->jdate, &(SP)->nass, &(SP)->ndef, &(SP)->ndp,\
&(SP)->grn, &(SP)->srn, (SP)->etype, &(SP)->depdp, (SP)->dtype,\
&(SP)->mb, &(SP)->mbid, &(SP)->ms, &(SP)->msid, &(SP)->ml, &(SP)->mlid,\
(SP)->algorithm, (SP)->auth, &(SP)->commid, (SP)->lddate

#define ORIGIN_WCS30 "%9.4f %9.4f %9.4f %17.5f %8ld %8ld %8ld %4ld %4ld %4ld %8ld %8ld %-7.7s %9.4f %1s %7.2f %8ld %7.2f %8ld %7.2f %8ld %-15.15s %-15.15s %8ld %-17.17s\n"

#define ORIGIN_WVL30(SP) \
(SP)->lat, (SP)->lon, (SP)->depth, (SP)->time, (SP)->orid, \
(SP)->evid, (SP)->jdate, (SP)->nass, (SP)->ndef, (SP)->ndp,\
(SP)->grn, (SP)->srn, (SP)->etype, (SP)->depdp, (SP)->dtype,\
(SP)->mb, (SP)->mbid, (SP)->ms, (SP)->msid, (SP)->ml, (SP)->mlid,\
(SP)->algorithm, (SP)->auth, (SP)->commid, (SP)->lddate

#define ORIGIN_NULL30 \
{ \
-999.,			/* lat		*/ \
-999.,			/* lon		*/ \
-999.,			/* depth	*/ \
-9999999999.999,	/* time		*/ \
-1,			/* orid		*/ \
-1,			/* evid		*/ \
-1,			/* jdate	*/ \
-1,			/* nass		*/ \
-1,			/* ndef		*/ \
-1,			/* ndp		*/ \
-1,			/* grn		*/ \
-1,			/* srn		*/ \
"-",			/* etype	*/ \
-999.,			/* depdp	*/ \
"-",			/* dtype	*/ \
-1.,			/* mb		*/ \
-1,			/* mbid		*/ \
-1.,			/* ms		*/ \
-1,			/* msid		*/ \
-1.,			/* ml		*/ \
-1,			/* mlid		*/ \
"-",			/* algorithm	*/ \
"-",			/* auth		*/ \
-1,			/* commid	*/ \
"-"			/* ldate	*/ \
}

#endif /* _ORIGIN_3_0_H */

#ifndef _ORIGIN_2_8_H
#define _ORIGIN_2_8_H

#define ORIGIN28_LEN	220

/** 
 *	Origin relation version 2.8
 *	@private
 */
typedef struct
{
	long	date;
	double	time;
	float	lat;
	float	lon;
	float	depth;
	float	mb;
	float	ms;
	float	mo;
	long	maxint;
	long	nass;
	long	ndef;
	long	ndp;
	long	nmb;
	long	nms;
	float	depdp;
	long	orid;
	long	evid;
	long	grn;
	long	srn;
	char	ltype[8];
	char	dtype[4];
	char	etype[8];
	char	auth[16];
	char	moauth[16];
	char	intscl[4];
	char	remark[32];
} ORIGIN28;

#define ORIGIN_RCS28 "%8ld%*c%15lf%*c%9f%*c%9f%*c%9f%*c%6f%*c%6f%*c%7f%*c%2ld%*c%4ld%*c%4ld%*c%4ld%*c%4ld%*c%4ld%*c%9f%*c%8ld%*c%8ld%*c%3ld%*c%3ld%*c%4c%*c%1c%*c%7c%*c%15c%*c%15c%*c%1c%*c%30c"

#define ORIGIN_RVL28(SP) \
&(SP)->date,&(SP)->time,&(SP)->lat,&(SP)->lon,&(SP)->depth,&(SP)->mb,\
&(SP)->ms,&(SP)->mo,&(SP)->maxint,&(SP)->nass,&(SP)->ndef,&(SP)->ndp,\
&(SP)->nmb,&(SP)->nms,&(SP)->depdp,&(SP)->orid,&(SP)->evid,&(SP)->grn,\
&(SP)->srn,(SP)->ltype,(SP)->dtype,(SP)->etype,(SP)->auth,(SP)->moauth,\
(SP)->intscl,(SP)->remark

#define ORIGIN_WCS28 "%8ld %15.3f %9.4f %9.4f %9.4f %6.2f %6.2f %7.2f %2ld %4ld %4ld %4ld %4ld %4ld %9.4f %8ld %8ld %3ld %3ld %-4.4s %1s %-7.7s %-15.15s %-15.15s %1s %-30.30s\n"

#define ORIGIN_WVL28(SP) \
(SP)->date,(SP)->time,(SP)->lat,(SP)->lon,(SP)->depth,(SP)->mb,\
(SP)->ms,(SP)->mo,(SP)->maxint,(SP)->nass,(SP)->ndef,(SP)->ndp,\
(SP)->nmb,(SP)->nms,(SP)->depdp,(SP)->orid,(SP)->evid,(SP)->grn,\
(SP)->srn,(SP)->ltype,(SP)->dtype,(SP)->etype,(SP)->auth,(SP)->moauth,\
(SP)->intscl,(SP)->remark

#define ORIGIN_NULL28 \
{ \
-1,			/* date */ \
-9999999999.999,	/* time */ \
-999.,			/* lat */ \
-999.,			/* lon */ \
-999.,			/* depth */ \
-1.,			/* mb */ \
-1.,			/* ms */ \
-999.,			/* mo */ \
-1,			/* maxint */ \
-1,			/* nass */ \
-1,			/* ndef */ \
-1,			/* ndp */ \
-1,			/* nmb */ \
-1,			/* nms */ \
-999.,			/* depdp */ \
-1,			/* orid */ \
-1,			/* evid */ \
-1,			/* grn */ \
-1,			/* srn */ \
"_",			/* ltype */ \
"_",			/* dtype */ \
"_",			/* etype */ \
"_",			/* auth */ \
"_",			/* moauth */ \
"_",			/* intscl */ \
"_"			/* remark */ \
}

#endif	/* _ORIGIN_2_8_H */
