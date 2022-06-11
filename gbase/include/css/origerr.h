/*	SccsId:	%W%	%G%	*/
/**
 *	Origerr relation from CSS 3.0 table definitions.
 *	Summary of confidence bounds in origin estimations. The error estimates
 *	associated with the parameters in the origin relation are saved in this
 *	table. The measurement attributes include the elements of the location
 * 	covariance matrix and a pre-computed epicentral error ellipse to a
 *	prescribed confidence.
 */
#ifndef _ORIGERR_3_0_H
#define _ORIGERR_3_0_H

#define ORIGERR30_LEN	257

/** 
 *  Origerr structure.
 *  @member orid origerr id. Initial value = -1.
 *  @member sxx covariance matrix element. Initial value = -1.
 *  @member syy covariance matrix element. Initial value = -1.
 *  @member szz covariance matrix element. Initial value = -1.
 *  @member stt covariance matrix element. Initial value = -1.
 *  @member sxy covariance matrix element. Initial value = -1.
 *  @member sxz covariance matrix element. Initial value = -1.
 *  @member syz covariance matrix element. Initial value = -1.
 *  @member stx covariance matrix element. Initial value = -1.
 *  @member sty covariance matrix element. Initial value = -1.
 *  @member stz covariance matrix element. Initial value = -1.
 *  @member sdobs std err of obs. Initial value = -1.-1.
 *  @member smajax semi-major axis of error. Initial value = -1.
 *  @member sminax semi-minor axis of error. Initial value = -1.
 *  @member strike strike of semi-major axis. Initial value = -1.
 *  @member sdepth depth error. Initial value = -1.
 *  @member stime origin time error. Initial value = -1.
 *  @member conf confidence. Initial value = 0.0.
 *  @member commid comment id. Initial value = -1.
 *  @member lddate load date. Initial value = "-".
 */

typedef struct
{
	long	orid;		/* origerr id			*/
	float	sxx;		/* covariance matrix element	*/
	float	syy;		/* covariance matrix element	*/
	float	szz;		/* covariance matrix element	*/
	float	stt;		/* covariance matrix element	*/
	float	sxy;		/* covariance matrix element	*/
	float	sxz;		/* covariance matrix element	*/
	float	syz;		/* covariance matrix element	*/
	float	stx;		/* covariance matrix element	*/
	float	sty;		/* covariance matrix element	*/
	float	stz;		/* covariance matrix element	*/
	float	sdobs;		/* std err of obs 		*/
	float	smajax;		/* semi-major axis of error	*/
	float	sminax;		/* semi-minor axis of error	*/
	float	strike;		/* strike of semi-major axis	*/
	float	sdepth;		/* depth error			*/
	float	stime;		/* origin time error		*/
	float	conf;		/* confidence			*/
	long	commid;		/* comment id			*/
	char	lddate[18];	/* load date			*/
} ORIGERR30;

#define ORIGERR_RCS30 "%8ld%*c%15f%*c%15f%*c%15f%*c%15f%*c%15f%*c%15f%*c%15f%*c%15f%*c%15f%*c%15f%*c%9f%*c%9f%*c%9f%*c%6f%*c%9f%*c%8f%*c%5f%*c%*c%17c"

#define ORIGERR_RVL30(SP) \
&(SP)->orid, &(SP)->sxx, &(SP)->syy, &(SP)->szz, &(SP)->stt, \
&(SP)->sxy, &(SP)->sxz, &(SP)->syz, &(SP)->stx, &(SP)->sty,\
&(SP)->stz, &(SP)->sdobs, &(SP)->smajax, &(SP)->sminax, &(SP)->strike,\
&(SP)->sdepth, &(SP)->stime, &(SP)->conf, &(SP)->commid, (SP)->lddate

#define ORIGERR_WCS30 "%8ld %15.4f %15.4f %15.4f %15.4f %15.4f %15.4f %15.4f %15.4f %15.4f %15.4f %9.4f %9.4f %9.4f %6.2f %9.4f %8.2f %5.3f %8ld %-17.17s\n"

#define ORIGERR_WVL30(SP) \
(SP)->orid, (SP)->sxx, (SP)->syy, (SP)->szz, (SP)->stt, \
(SP)->sxy, (SP)->sxz, (SP)->syz, (SP)->stx, (SP)->sty,\
(SP)->stz, (SP)->sdobs, (SP)->smajax, (SP)->sminax, (SP)->strike,\
(SP)->sdepth, (SP)->stime, (SP)->conf, (SP)->commid, (SP)->lddate

#define ORIGERR_NULL30 \
{ \
-1,			/* orid		*/ \
-1.0,			/* sxx 		*/ \
-1.0,			/* syy 		*/ \
-1.0,			/* szz 		*/ \
-1.0,			/* stt 		*/ \
-1.0,			/* sxy 		*/ \
-1.0,			/* sxz 		*/ \
-1.0,			/* syz 		*/ \
-1.0,			/* stx 		*/ \
-1.0,			/* sty 		*/ \
-1.0,			/* stz 		*/ \
-1.0,			/* sdobs	*/ \
-1.0,			/* smajax	*/ \
-1.0,			/* sminax	*/ \
-1.0,			/* strike	*/ \
-1.0,			/* sdepth	*/ \
-1.0,			/* stime	*/ \
 0.0,			/* conf		*/ \
-1,			/* commid	*/ \
"-"			/* ldate	*/ \
}

#endif /* _ORIGERR_3_0_H */

#ifndef _ORIGERR_2_8_H
#define _ORIGERR_2_8_H

#define ORIGERR28_LEN	220

/** 
 *	Origerr relation version 2.8
 *	@private
 */
typedef struct
{
	long	orid;		/* origerr id			*/
	float	sdobs;		/* std err of obs 		*/
	float	sxx;		/* covariance matrix element	*/
	float	syy;		/* covariance matrix element	*/
	float	szz;		/* covariance matrix element	*/
	float	stt;		/* covariance matrix element	*/
	float	sxy;		/* covariance matrix element	*/
	float	sxz;		/* covariance matrix element	*/
	float	syz;		/* covariance matrix element	*/
	float	stx;		/* covariance matrix element	*/
	float	sty;		/* covariance matrix element	*/
	float	stz;		/* covariance matrix element	*/
	float	sdmb;		/* std dev of mb		*/
	float	sdms;		/* std dev of ms		*/
	float	sddp;		/* std dev of (pP-P)		*/
	float	sdzdp;		/* std dev of depth (pP-P)	*/
	char	remark[32];
} ORIGERR28;

#define ORIGERR_RCS28 "%8ld%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%9f%*c%30c"

#define ORIGERR_RVL28(SP) \
&(SP)->orid, &(SP)->sdobs, &(SP)->sxx, &(SP)->syy, &(SP)->szz, &(SP)->stt, \
&(SP)->sxy, &(SP)->sxz, &(SP)->syz, &(SP)->stx, &(SP)->sty,\
&(SP)->stz, &(SP)->sdmb, &(SP)->sdms, &(SP)->sddp, &(SP)->sdzdp, (SP)->remark

#define ORIGERR_WCS28 "%8ld %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %-30.30s\n" 

#define ORIGERR_WVL28(SP) \
(SP)->orid,(SP)->sdobs, (SP)->sxx, (SP)->syy, (SP)->szz, (SP)->stt, \
(SP)->sxy, (SP)->sxz, (SP)->syz, (SP)->stx, (SP)->sty,\
(SP)->stz, (SP)->sdmb, (SP)->sdms, (SP)->sddp, (SP)->sdzdp, (SP)->remark

#define ORIGERR_NULL28 \
{ \
-1,			/* orid		*/ \
-1.0,			/* sdobs	*/ \
-1.0,			/* sxx 		*/ \
-1.0,			/* syy 		*/ \
-1.0,			/* szz 		*/ \
-1.0,			/* stt 		*/ \
-1.0,			/* sxy 		*/ \
-1.0,			/* sxz 		*/ \
-1.0,			/* syz 		*/ \
-1.0,			/* stx 		*/ \
-1.0,			/* sty 		*/ \
-1.0,			/* stz 		*/ \
-1.0,			/* sdmb		*/ \
-1.0,			/* sdms		*/ \
-1.0,			/* sddp		*/ \
-1.0,			/* sdzdp	*/ \
"_"			/* remark 	*/ \
}

#endif	/* _ORIGERR_2_8_H */
