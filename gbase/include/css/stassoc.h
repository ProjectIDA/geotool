/*	SccsId:	%W%	%G%	*/
/**
 *	Stassoc relation from CSS 3.0 table definitions.
 *	Summary information on groups of related arrivals. This table defines
 *	the group of phases seen at a single station which all came from the
 *	same event.
 */
#ifndef _STASSOC_3_0_H
#define _STASSOC_3_0_H

#define STASSOC30_LEN	187

/** 
 *  Stassoc structure.
 *  @member stassid Stassoc id. Initial value = -1.
 *  @member sta Station code. Initial value = "-".
 *  @member etype Event type. Initial value = "-".
 *  @member location Apparent location description. Initial value = "-".
 *  @member dist Estimated distance. Initial value = -1.
 *  @member azimuth Observed azimuth. Initial value = -1.
 *  @member lat Estimated latitude. Initial value = -999.
 *  @member lon Estimated longitude. Initial value = -999.
 *  @member depth Estimated depth. Initial value = -999.
 *  @member time Estimated origin time. Initial value = -9999999999.999.
 *  @member imb Initial estimated mb. Initial value = -999.
 *  @member ims Initial estimated ms. Initial value = -999.
 *  @member iml Initial estimated ml. Initial value = -999.
 *  @member auth Source/originator. Initial value = "-".
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	stassid;	/* stassoc id			*/
	char	sta[7];		/* station code			*/
	char	etype[8];	/* event type			*/
	char	location[33];	/* apparent location description */
	float	dist;		/* estimated distance		*/
	float	azimuth;	/* observed azimuth		*/
	float	lat;		/* estimated latitude		*/
	float	lon;		/* estimated longitude		*/
	float	depth;		/* estimated depth		*/
	double	time;		/* estimated origin time	*/
	float	imb;		/* initial estimated mb		*/
	float	ims;		/* initial estimated ms		*/
	float	iml;		/* initial estimated ml		*/
	char	auth[16];	/* source/originator		*/
	long	commid;		/* comment id			*/
	char	lddate[18];	/* load date			*/
} STASSOC30;

#define STASSOC_NULL30 \
{ \
-1,			/* stassid	*/ \
"-",			/* sta 		*/ \
"-",			/* etype	*/ \
"-",			/* location	*/ \
-1.0,			/* dist		*/ \
-1.0,			/* azimuth	*/ \
-999.0,			/* lat		*/ \
-999.0,			/* lon		*/ \
-999.0,			/* depth	*/ \
-9999999999.999,	/* time		*/ \
-999.0,			/* imb		*/ \
-999.0,			/* ims		*/ \
-999.0,			/* iml		*/ \
"-",			/* auth		*/ \
-1,			/* commid	*/ \
"-",			/* lddate	*/ \
}
 
#endif /* _STASSOC_3_0_H */
