/*	SccsId:	%W%	%G%	*/
/**
 *	Pmccdisc relation from CSS 3.0 table definitions.
 */
#ifndef _PMCC_RECIPE_3_0_H
#define _PMCC_RECIPE_3_0_H

#define PMCC_RECIPE30_LEN	206

/** 
 *  Pmccdisc structure.
 *  @member jdate Julian date. Initial value = -1.
 *  @member time Epoch time of first sample in file. Initial value = -9999999999.999.
 *  @member tlen Overall time of spectrogram. Initial value = -1.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member winpts Number of points in window. Initial value = -1.
 *  @member overlap Percent of overlap between adjacent windows. Initial value = 0.
 *  @member nwin Number of windows. Initial value = 0.
 *  @member lofreq Low frequency. Initial value = -1.
 *  @member hifreq High frequency. Initial value = -1.
 *  @member nf Number of frequency values. Initial value = -1.
 *  @member pmccid pmcc id. Initial value = -1.
 *  @member datatype Numeric storage. Initial value = "-".
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member foff Byte ofspet. Initial value = -1.
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	pmccrecid;	/* pmcc recipe id			*/
	long	deflt;		/* default */
	long	fGroup;		/* filter group */
	float   winlen;         /* number of points in window	  	*/
	float   wingap;         /* number of points in window	  	*/
	float   threshCons;     /* number of points in window	  	*/
	long	threshNsens;    /* number of points in window	  	*/
	float   qFactor;   
	long	pmcc3D;
	float	sound_speed;
	float	elevation_angle;
	float   threshFamLen;    /* number of points in window	  	*/
	long	threshFamMin;   /* number of points in window	  	*/
	long	threshFamMax;   /* number of points in window	  	*/
	float   speedTransition;/* number of points in window	  	*/
	float	timeTol;	
	float	freqTol;	
	float	speedTol1;	
	float	speedTol2;	
	float	azTol1;	
	float	azTol2;	
	char    auth[16];
	char	lddate[18];	/* load date			*/
} PMCC_RECIPE30;

#define PMCC_RECIPE_RCS30 \
"%8ld%*c%17lf%*c%6f%*c%6c%*c%8c%*c%8d%*c%3d%*c%5d%*c%6f%*c%6f%*c%5d%*c%8ld%*c%2c%*c%64c%*c%32c%*c%10ld%*c%8ld%17c"

#define PMCC_RECIPE_RVL30(PMCC_RECIPE) \
&(PMCC_RECIPE)->jdate, &(PMCC_RECIPE)->time, &(PMCC_RECIPE)->tlen, (PMCC_RECIPE)->sta, (PMCC_RECIPE)->chan, &(PMCC_RECIPE)->winpts, &(PMCC_RECIPE)->overlap, &(PMCC_RECIPE)->nwin, &(PMCC_RECIPE)->lofreq, &(PMCC_RECIPE)->hifreq, &(PMCC_RECIPE)->nf, &(PMCC_RECIPE)->pmccid, (PMCC_RECIPE)->datatype, (PMCC_RECIPE)->dir, (PMCC_RECIPE)->dfile, &(PMCC_RECIPE)->foff, &(PMCC_RECIPE)->commid, (PMCC_RECIPE)->lddate

#define PMCC_RECIPE_WCS30 \
"%8ld %17.5f %6.2f %-6.6s %-8.8s %8d %3d %5d %6.2f %6.2f %5d %8ld %-2.2s %-64.64s %-32.32s %10ld %8ld %-17.17s\n"

#define PMCC_RECIPE_WVL30(PMCC_RECIPE) \
(PMCC_RECIPE)->jdate, (PMCC_RECIPE)->time, (PMCC_RECIPE)->tlen, (PMCC_RECIPE)->sta, (PMCC_RECIPE)->chan, (PMCC_RECIPE)->winpts, (PMCC_RECIPE)->overlap, (PMCC_RECIPE)->nwin, (PMCC_RECIPE)->lofreq, (PMCC_RECIPE)->hifreq, (PMCC_RECIPE)->nf, (PMCC_RECIPE)->pmccid, (PMCC_RECIPE)->datatype, (PMCC_RECIPE)->dir, (PMCC_RECIPE)->dfile, (PMCC_RECIPE)->foff, (PMCC_RECIPE)->commid, (PMCC_RECIPE)->lddate

#define PMCC_RECIPE_NULL30 \
{ \
-1,			/* pmccrecid	*/ \
-1,			/* deflt	*/ \
-1,                     /* fGroup       */ \
-1.,			/* winlen	*/ \
-1.,			/* wingap	*/ \
-1.,			/* threshCons	*/ \
-1,                     /* threshNsens	*/ \
-1.,			/* qFactor	*/ \
-1,			/* pmcc3D	*/ \
-1.,			/* sound_speed	*/ \
-1.,			/* elevation_angle */ \
-1.,			/* threshFamLen	*/ \
-1,                     /* threshFamMin	*/ \
-1,                     /* threshFamMax	*/ \
-1.,			/* speedTransition*/ \
-1.,			/* tSigma	*/ \
-1.,			/* fSigma	*/ \
-1.,			/* speedTol1    */ \
-1.,			/* speedTol2    */ \
-1.,			/* azTol1       */ \
-1.,			/* azTol2       */ \
"-"                     /* auth		*/ \
"-"                     /* lddate       */ \
}

#endif
