/*	SccsId:	%W%	%G%	*/
/**
 *	Dervdisc relation from CSS 3.0 table definitions.
 */
#ifndef _DERVDISC_3_0_H
#define _DERVDISC_3_0_H

#define DERVDISC30_LEN	244

/** 
 *  Dervdisc structure.
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
        char    sta[7];         /* station                              */
        char    chan[9];        /* channel                              */
	double  time;           /* epoch time of first sample in file   */
	float	tlen;		/* overall time of spectrogram		*/
	char	net[9];		/* network				*/
        long    dervid;         /* derived id (unique for this table    */
        long    recid;          /* recipe id (points to recipe table)   */
        char    method[18];     /* description of the processing method */
	char    datatype[3];    /* numeric storage                      */
	char    dervtype[5];    /* derived type                         */
	long    jdate;          /* julian date                          */
	char    dir[65];        /* directory                            */
	char    dfile[33];      /* data file                            */
	long    foff;           /* byte ofspet                          */
	long	commid;		/* comment id				*/
	char	lddate[18];	/* load date			*/
} DERVDISC30;

#define DERVDISC_RCS30 \
"%8ld%*c%17lf%*c%6f%*c%6c%*c%8c%*c%8d%*c%3d%*c%5d%*c%6f%*c%6f%*c%5d%*c%8ld%*c%2c%*c%64c%*c%32c%*c%10ld%*c%8ld%17c"

#define DERVDISC_RVL30(PMCC) \
&(PMCC)->jdate, &(PMCC)->time, &(PMCC)->tlen, (PMCC)->sta, (PMCC)->chan, &(PMCC)->winpts, &(PMCC)->overlap, &(PMCC)->nwin, &(PMCC)->lofreq, &(PMCC)->hifreq, &(PMCC)->nf, &(PMCC)->pmccid, (PMCC)->datatype, (PMCC)->dir, (PMCC)->dfile, &(PMCC)->foff, &(PMCC)->commid, (PMCC)->lddate

#define DERVDISC_WCS30 \
"%8ld %17.5f %6.2f %-6.6s %-8.8s %8d %3d %5d %6.2f %6.2f %5d %8ld %-2.2s %-64.64s %-32.32s %10ld %8ld %-17.17s\n"

#define DERVDISC_WVL30(PMCC) \
(PMCC)->jdate, (PMCC)->time, (PMCC)->tlen, (PMCC)->sta, (PMCC)->chan, (PMCC)->winpts, (PMCC)->overlap, (PMCC)->nwin, (PMCC)->lofreq, (PMCC)->hifreq, (PMCC)->nf, (PMCC)->pmccid, (PMCC)->datatype, (PMCC)->dir, (PMCC)->dfile, (PMCC)->foff, (PMCC)->commid, (PMCC)->lddate

#define DERVDISC_NULL30 \
{ \
"-",                    /* sta		*/ \
"-",                    /* chan		*/ \
-9999999999.999,        /* time         */ \
-1.,			/* tlen		*/ \
"-",                    /* net		*/ \
-1,			/* dervid	*/ \
-1,			/* recid	*/ \
"-",                    /* method	*/ \
"-",                    /* datatype	*/ \
"-",                    /* dervtype     */ \
-1,                     /* jdate        */ \
"-",                    /* dir		*/ \
"-",                    /* dfile	*/ \
-1,			/* foff		*/ \
-1,			/* commid	*/ \
"-"                     /* lddate       */ \
}

#endif
