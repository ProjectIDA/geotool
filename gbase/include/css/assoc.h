/*	SccsId:	%W%	%G%	*/
/**
 *	Assoc relation from CSS 3.0 table definitions.
 *	Data associating arrivals with origins. This table has information that
 *	connects arrivals (i.e., entries in the arrival relation) to a
 *	particular origin. It has a composite key made of arid and orid. There
 *	are two kinds of measurement data: three attributes are related to the
 *	station (delta, seaz, esaz), and the remaining measurement attributes
 *	are jointly determined by the measurements made on the seismic wave
 *	(arrival), and the inferred event's origin. The attribute sta is
 *	intentionally duplicated in this table to eliminate the need for a join
 *	with arrival when doing a lookup on station.
 *
 *	@see ARRIVAl30
 *	@see ORIGIN30
 */
#ifndef _ASSOC_3_0_H
#define _ASSOC_3_0_H

#define ASSOC30_LEN	152

/** 
 *  Assoc structure.
 *  @member arid Arrival id. Initial value = -1.
 *  @member orid Origin id. Initial value = -1.
 *  @member sta Station code. Initial value = "-".
 *  @member phase Associated phase. Initial value = "-".
 *  @member belief Phase confidence. Initial value = -1.
 *  @member delta Station to event distance. Initial value = -1.
 *  @member seaz Station to event azimuth. Initial value = -999.
 *  @member esaz Event to station azimuth. Initial value = -999.
 *  @member timeres Time residual. Initial value = -999.
 *  @member timedef Time = defining,non-defining. Initial value = "d".
 *  @member azres Azimuth residual. Initial value = -999.
 *  @member azdef Azimuth = defining,non-defining. Initial value = "-".
 *  @member slores Slowness residual. Initial value = -999.
 *  @member slodef Slowness = defining,non-defining. Initial value = "-".
 *  @member emares Incidence angle residual. Initial value = -999.
 *  @member wgt Location weight. Initial value = -1.
 *  @member vmodel Velocity model. Initial value = "-".
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	arid;		/* arrival id			*/
	long	orid;		/* origin id			*/
	char	sta[7];		/* station code			*/
	char	phase[9];	/* associated phase		*/
	float	belief;		/* phase confidence		*/
	float	delta;		/* station to event distance	*/
	float	seaz;		/* station to event azimuth	*/
	float	esaz;		/* event to station azimuth	*/
	float	timeres;	/* time residual		*/
	char	timedef[2];	/* time = defining,non-defining */
	float	azres;		/* azimuth residual		*/
	char	azdef[2];	/* azimuth = defining,non-defining */
	float	slores;		/* slowness residual		*/
	char	slodef[2];	/* slowness = defining,non-defining */
	float	emares;		/* incidence angle residual	*/
	float	wgt;		/* location weight		*/
	char	vmodel[16];	/* velocity model		*/
	long	commid;		/* comment id			*/
	char	lddate[18];	/* load date			*/
} ASSOC30;

#define ASSOC_RCS30 "%8ld%*c%8ld%*c%6c%*c%8c%*c%4f%*c%8f%*c%7f%*c%7f%*c%8f%*c%1c%*c%7f%*c%1c%*c%7f%*c%1c%*c%7f%*c%6f%*c%15c%*c%8ld%*c%17c"

#define ASSOC_RVL30(SP) \
&(SP)->arid, &(SP)->orid, (SP)->sta, (SP)->phase, &(SP)->belief, &(SP)->delta,\
&(SP)->seaz, &(SP)->esaz, &(SP)->timeres, (SP)->timedef, &(SP)->azres,\
(SP)->azdef, &(SP)->slores, (SP)->slodef, &(SP)->emares, &(SP)->wgt, \
(SP)->vmodel, &(SP)->commid, (SP)->lddate

#define ASSOC_WCS30 "%8ld %8ld %-6.6s %-8.8s %4.1f %8.3f %7.2f %7.2f %8.3f %1s %7.1f %1s %7.2f %1s %7.1f %6.3f %-15.15s %8ld %-17.17s\n"

#define ASSOC_WVL30(SP) \
(SP)->arid, (SP)->orid, (SP)->sta, (SP)->phase, (SP)->belief, (SP)->delta,\
(SP)->seaz, (SP)->esaz, (SP)->timeres, (SP)->timedef, (SP)->azres,\
(SP)->azdef, (SP)->slores, (SP)->slodef, (SP)->emares, (SP)->wgt, (SP)->vmodel,\
(SP)->commid, (SP)->lddate

#define ASSOC_NULL30 \
{ \
-1,			/* arid		*/ \
-1,			/* orid		*/ \
"-",			/* sta 		*/ \
"-",			/* phase	*/ \
-1.0,			/* belief	*/ \
-1.0,			/* delta	*/ \
-999.0,			/* seaz		*/ \
-999.0,			/* esaz		*/ \
-999.0,			/* timeres	*/ \
"d",			/* timedef	*/ \
-999.0,			/* azres	*/ \
"-",			/* azdef	*/ \
-999.0,			/* slores	*/ \
"-",			/* slodef	*/ \
-999.0,			/* emares	*/ \
-1.0,			/* wgt		*/ \
"-",			/* vmodel	*/ \
-1,			/* commid	*/ \
"-",			/* lddate	*/ \
}
 
#endif /* _ASSOC_3_0_H */


#ifndef _ASSOC_2_8_H
#define _ASSOC_2_8_H

#define ASSOC28_LEN	115

/** 
 *	Arrival relation version 2.8
 *	@private
 */
typedef struct
{
	int	arid;		/* arrival id			*/
	int	orid;		/* origin id			*/
	float	delta;		/* distance			*/
	char	phase[9];	/* associated phase		*/
	float	esaz;		/* event to station azimuth	*/
	float	seaz;		/* station to event azimuth	*/
	float	resid;		/* residual			*/
	char	atype[2];	/* arrival usage		*/
	float	wgt;		/* location weight		*/
	float	mag;		/* location weight		*/
	char	tratbl[5];	/* travel time table		*/
	char	remark[31];	/* comment			*/
} ASSOC28;

#define ASSOC_RCS28 "%8ld%*c%8ld%*c%8f%*c%8c%*c%8f%*c%7f%*c%8f%*c%1c%*c%6f%*c%7f%*c%4c%*c%30c"

#define ASSOC_RVL28(SP) \
&(SP)->arid, &(SP)->orid, &(SP)->delta, (SP)->phase, &(SP)->esaz, &(SP)->seaz, \
&(SP)->resid, (SP)->atype, &(SP)->wgt, &(SP)->mag, (SP)->tratbl, (SP)->remark

#define ASSOC_WCS28 "%8ld %8ld %8.3f %-8.8s %8.3f %7.2f %8.3f %1s %6.3f %7.2f %-4.4s %-30.30\n"

#define ASSOC_WVL28(SP) \
(SP)->arid, (SP)->orid, (SP)->delta, (SP)->phase, (SP)->esaz, (SP)->seaz, \
(SP)->resid, (SP)->atype, (SP)->wgt, (SP)->mag, (SP)->tratbl, (SP)->remark

#define ASSOC_NULL28 \
{ \
-1,			/* arid		*/ \
-1,			/* orid		*/ \
-1.0,			/* delta	*/ \
"-",			/* phase	*/ \
-999.0,			/* esaz		*/ \
-999.0,			/* seaz		*/ \
-999.0,			/* resid	*/ \
"-",			/* atype	*/ \
-1.0,			/* wgt		*/ \
-1.0,			/* mag		*/ \
"-",			/* tratbl	*/ \
"-",			/* remark	*/ \
}

#endif /* _ASSOC_2_8_H */
