/*	SccsId:	%W%	%G%	*/
/**
 *	Spdisc relation from CSS 3.0 table definitions.
 */
#ifndef _SPDISC_3_0_H
#define _SPDISC_3_0_H

#define SPDISC30_LEN	236

/** 
 *  Spdisc structure.
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
 *  @member spid sp id. Initial value = -1.
 *  @member datatype Numeric storage. Initial value = "-".
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member foff Byte ofspet. Initial value = -1.
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long    jdate;          /* julian date                          */
	double  time;           /* epoch time of first sample in file   */
	float	tlen;		/* overall time of spectrogram		*/
	char	sta[7];		/* station				*/
	char	chan[9];	/* channel				*/
	int     winpts;         /* number of points in window	  	*/
	int     overlap;        /* percent of overlap between adjacent windows*/
	int	nwin;		/* number of windows			*/
	float	lofreq;		/* lo frequency				*/
	float	hifreq;		/* hi frequency				*/
	int     nf;             /* number of frequency values           */
	long	spid;		/* sp id				*/
	char    datatype[3];    /* numeric storage                      */
	char    dir[65];        /* directory                            */
	char    dfile[33];      /* data file                            */
	long    foff;           /* byte ofspet                          */
	long	commid;		/* comment id				*/
	char	lddate[18];	/* load date			*/
} SPDISC30;

#define SPDISC_RCS30 \
"%8ld%*c%17lf%*c%6f%*c%6c%*c%8c%*c%8d%*c%3d%*c%5d%*c%6f%*c%6f%*c%5d%*c%8ld%*c%2c%*c%64c%*c%32c%*c%10ld%*c%8ld%17c"

#define SPDISC_RVL30(SP) \
&(SP)->jdate, &(SP)->time, &(SP)->tlen, (SP)->sta, (SP)->chan, &(SP)->winpts, &(SP)->overlap, &(SP)->nwin, &(SP)->lofreq, &(SP)->hifreq, &(SP)->nf, &(SP)->spid, (SP)->datatype, (SP)->dir, (SP)->dfile, &(SP)->foff, &(SP)->commid, (SP)->lddate

#define SPDISC_WCS30 \
"%8ld %17.5f %6.2f %-6.6s %-8.8s %8d %3d %5d %6.2f %6.2f %5d %8ld %-2.2s %-64.64s %-32.32s %10ld %8ld %-17.17s\n"

#define SPDISC_WVL30(SP) \
(SP)->jdate, (SP)->time, (SP)->tlen, (SP)->sta, (SP)->chan, (SP)->winpts, (SP)->overlap, (SP)->nwin, (SP)->lofreq, (SP)->hifreq, (SP)->nf, (SP)->spid, (SP)->datatype, (SP)->dir, (SP)->dfile, (SP)->foff, (SP)->commid, (SP)->lddate

#define SPDISC_NULL30 \
{ \
-1,                     /* jdate        */ \
-9999999999.999,        /* time         */ \
-1.,			/* tlen		*/ \
"-",                    /* sta          */ \
"-",                    /* chan         */ \
-1,                     /* winpts       */ \
0,                      /* overlap      */ \
0,			/* nwin		*/ \
-1.,			/* lofreq	*/ \
-1.,			/* hifreq	*/ \
-1,			/* nf		*/ \
-1,			/* spid		*/ \
"-",                    /* datatype	*/ \
"-",                    /* dir		*/ \
"-",                    /* dfile	*/ \
-1,			/* foff		*/ \
-1,			/* commid	*/ \
"-"                     /* lddate       */ \
}

#endif
