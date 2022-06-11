/*	SccsId:	%W%	%G%	*/
/**
 *	Fsdisc relation from CSS 3.0 table definitions.
 */
#ifndef _FSDISC_3_0_H
#define _FSDISC_3_0_H

#define FSDISC30_LEN	269

/** 
 *  fsdisc structure.
 *  @member jdate Julian date. Initial value = -1.
 *  @member time Epoch time of first sample in file. Initial value = -9999999999.999.
 *  @member tlen Time window. Initial value = -1.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member fstype Fourier spectrum type. Initial value = "-".
 *  @member arid Arrival id. Initial value = -1.
 *  @member maxf Maximum frequency. Initial value = -1.
 *  @member nf Number of frequency values. Initial value = -1.
 *  @member samprate Sample rate of time series. Initial value = -1.
 *  @member chanid Channel id. Initial value = -1.
 *  @member wfid Waveform id. Initial value = -1.
 *  @member fsrid fs recipe id. Initial value = -1.
 *  @member fsid fs id. Initial value = -1.
 *  @member datatype Numeric storage. Initial value = "-".
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member foff Byte offset. Initial value = -1.
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	jdate;		/* julian date				*/
	double	time;		/* epoch time of first sample in file	*/
	float	tlen;		/* time window				*/
	char	sta[7];		/* station				*/
	char	chan[9];	/* channel				*/
	char	fstype[5];	/* Fourier spectrum type		*/
	long	arid;		/* arrival id				*/
	float	maxf;		/* max frequency			*/
	long	nf;		/* number of frequency values		*/
	double	samprate;	/* sample rate 				*/
	long	chanid;		/* channel id				*/
	long	wfid;		/* waveform id				*/
	long	fsrid;		/* fs recipe id				*/
	long	fsid;		/* fs id				*/
	char	datatype[3];	/* numeric storage			*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* byte offset				*/
	long	commid;		/* comment id				*/
	char	lddate[18];	/* load date			*/
} FSDISC30;

#define FSDISC_RCS30 \
"%8ld%*c%17lf%*c%6f%*c%6c%*c%8c%*c%4c%*c%8ld%*c%9f%*c%8ld%*c%9lf%*c%8ld%*c%8ld%*c%8ld%*c%8ld%*c%2c%*c%64c%*c%32c%*c%10ld%*c%8ld%*c%17c"

#define FSDISC_RVL30(SP) \
&(SP)->jdate, &(SP)->time, &(SP)->tlen, (SP)->sta, (SP)->chan, (SP)->fstype, &(SP)->arid, &(SP)->maxf, &(SP)->nf, &(SP)->samprate, &(SP)->chanid, &(SP)->wfid, &(SP)->fsrid, &(SP)->fsid, (SP)->datatype, (SP)->dir, (SP)->dfile, &(SP)->foff, &(SP)->commid, (SP)->lddate

#define FSDISC_WCS30 \
"%8ld %17.5f %6.2f %-6.6s %-8.8s %-4.4s %8ld %9.4f %8ld %9f %8ld %8ld %8ld %8ld %-2.2s %-64.64s %-32.32s %10ld %8ld %-17.17s\n"

#define FSDISC_WVL30(SP) \
(SP)->jdate, (SP)->time, (SP)->tlen, (SP)->sta, (SP)->chan, (SP)->fstype, (SP)->arid, (SP)->maxf, (SP)->nf, (SP)->samprate, (SP)->chanid, (SP)->wfid, (SP)->fsrid, (SP)->fsid, (SP)->datatype, (SP)->dir, (SP)->dfile, (SP)->foff, (SP)->commid, (SP)->lddate

#define FSDISC_NULL30 \
{ \
-1,			/* jdate	*/ \
-9999999999.999,	/* time		*/ \
-1.,			/* tlen		*/ \
"-",			/* sta		*/ \
"-",			/* chan		*/ \
"-",			/* fstype	*/ \
-1,			/* arid		*/ \
-1.,			/* maxf		*/ \
-1,			/* nf		*/ \
-1,			/* samprate     */ \
-1,			/* chanid	*/ \
-1,			/* wfid		*/ \
-1,			/* fsrid	*/ \
-1,			/* fsid		*/ \
"-",			/* datatype	*/ \
"-",			/* dir		*/ \
"-",			/* dfile	*/ \
-1,			/* foff		*/ \
-1,			/* commid	*/ \
"-"			/* lddate	*/ \
}

#endif

/*
 * fsave relation css 3.0
 */
#ifndef _FSAVE_3_0_H
#define _FSAVE_3_0_H

#define FSAVE30_LEN	196

/** 
 *  fsave structure.
 *  @member sta station. Initial value = "-".
 *  @member avtype fs channel (ave, med, 95perct, etc). Initial value = "-".
 *  @member fstype Fourier spectrum type. Initial value = "-".
 *  @member maxf max frequency. Initial value = -1.
 *  @member nf number of frequency values. Initial value = -1.
 *  @member nave number of spcetra used in this average. Initial value = -1.
 *  @member afsid ave fs id. Initial value = -1.
 *  @member noissd std dev of log noise. Initial value = -999.9.
 *  @member datatype numeric storage. Initial value = "-".
 *  @member dir directory. Initial value = "-".
 *  @member dfile data file. Initial value = "-".
 *  @member foff byte offset. Initial value = -1.
 *  @member commid comment id. Initial value = -1.
 *  @member lddate load date. Initial value = "-".
 */
typedef struct
{
	char	sta[7];		/* station				*/
	char	avtype[9];	/* fs channel (ave, med, 95perct, etc)	*/
	char	fstype[5];	/* Fourier spectrum type		*/
	float	maxf;		/* max frequency			*/
	long	nf;		/* number of frequency values		*/
	int	nave;		/* number of spcetra used in this average */
	long	afsid;		/* ave fs id				*/
	float	noissd;		/* std dev of log noise			*/
	char	datatype[3];	/* numeric storage			*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* byte offset				*/
	long	commid;		/* comment id				*/
	char	lddate[18];	/* load date				*/
} FSAVE30;

#define FSAVE_RCS30 \
"%6c%*c%8c%*c%4c%*c%9f%*c%8ld%*c%4d%*c%8ld%*c%5f%*c%2c%*c%64c%*c%32c%*c%10ld%*c%8ld%*c%17c"

#define FSAVE_RVL30(SP) \
(SP)->sta, (SP)->avtype, (SP)->fstype, &(SP)->maxf, &(SP)->nf, &(SP)->nave, &(SP)->afsid, &(SP)->noissd, (SP)->datatype, (SP)->dir, (SP)->dfile, &(SP)->foff, &(SP)->commid, (SP)->lddate

#define FSAVE_WCS30 \
"%-6.6s %-8.8s %-4.4s %9.4f %8ld %4d %8ld %5.2f %-2.2s %-64.64s %-32.32s %10ld %8ld %-17.17s\n"

#define FSAVE_WVL30(SP) \
(SP)->sta, (SP)->avtype, (SP)->fstype, (SP)->maxf, (SP)->nf,(SP)->nave, (SP)->afsid, (SP)->noissd, (SP)->datatype, (SP)->dir, (SP)->dfile, (SP)->foff, (SP)->commid, (SP)->lddate

#define FSAVE_NULL30 \
{ \
"-",			/* sta		*/ \
"-",			/* fschan	*/ \
"-",			/* fstype	*/ \
-1.,			/* maxf		*/ \
-1,			/* nf		*/ \
-1,			/* nave		*/ \
-1,			/* afsid	*/ \
-999.9,			/* noissd	*/ \
"-",			/* datatype	*/ \
"-",			/* dir		*/ \
"-",			/* dfile	*/ \
-1,			/* foff		*/ \
-1,			/* commid	*/ \
"-"			/* lddate	*/ \
}

#endif

/*
 * fsrecipe relation css 3.0
 */
#ifndef _FSRECIPE_3_0_H
#define _FSRECIPE_3_0_H

#define FSRECIPE30_LEN	90

/** 
 *  fsrecipe structure.
 *  @member fsrid fs recipe id. Initial value = -1.
 *  @member fsdesc Description of fs, med/ave day/night, etc. Initial value = "-".
 *  @member taper Type of taper. Initial value = "-".
 *  @member taperstart Starting percent for cosine taper. Initial value = 0.
 *  @member taperend Ending percent for cosine taper. Initial value = 0.
 *  @member winlen Number of seconds in windows. Initial value = -1.
 *  @member overlap Percent of overlap between adjacent windows. Initial value = 0.
 *  @member nfft Number of points in fft. Initial value = -1.
 *  @member smoothvalue Amount of smoothing (Hz). Initial value = 0.
 *  @member response Flag if corrected for instrument response. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	fsrid;		/* fs recipe id			*/
	char	fsdesc[16];	/* desc of fs, med/ave day/night, etc */
	char	taper[9];	/* type of taper		*/
	int	taperstart;	/* starting percent for cosine taper */
	int	taperend;	/* ending percent for cosine taper	*/
	float	winlen;		/* number of seconds in windows	*/
	int	overlap;	/* percent of overlap between adjacent windows*/
	long	nfft;		/* number of points in fft	*/
	float	smoothvalue;	/* amount of smoothing (Hz)	*/
	char	response[2];	/* flag if corrected for instrument response */
	char	lddate[18];	/* load date			*/
} FSRECIPE30;

#define FSRECIPE_RCS30 \
"%8ld%*c%15c%*c%8c%*c%3d%*c%3d%*c%9f%*c%3d%*c%8ld%*c%5f%*c%1c%*c%17c"

#define FSRECIPE_RVL30(SP) &(SP)->fsrid, (SP)->fsdesc, (SP)->taper, &(SP)->taperstart, &(SP)->taperend, &(SP)->winlen, &(SP)->overlap, &(SP)->nfft, &(SP)->smoothvalue, (SP)->response, (SP)->lddate

#define FSRECIPE_WCS30 \
"%8ld %-15.15s %-8.8s %3d %3d %9.2f %3d %8ld %5.2f %1s %-17.17s\n"

#define FSRECIPE_WVL30(SP) (SP)->fsrid, (SP)->fsdesc, (SP)->taper, (SP)->taperstart, (SP)->taperend, (SP)->winlen, (SP)->overlap, (SP)->nfft, (SP)->smoothvalue, (SP)->response, (SP)->lddate

#define FSRECIPE_NULL30 \
{ \
-1,			/* fsrid	*/ \
"-",			/* fsdesc	*/ \
"-",			/* taper	*/ \
0.0,			/* taperstart	*/ \
0.0, 			/* taperend	*/ \
-1.0,			/* winlen 	*/ \
0,			/* overlap	*/ \
-1,			/* nfft		*/ \
0.0,			/* smoothvalue 	*/ \
"-",			/* response	*/ \
"-"			/* lddate	*/ \
}

#endif

/*
 * fstag relation css 3.0
 */
#ifndef _FSTAG_3_0_H
#define _FSTAG_3_0_H

#define FSTAG30_LEN	35

/** 
 *  Fstag structure.
 *  @member afsid fs id. Initial value = -1.
 *  @member fsid fs id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	afsid;		/* fs id	*/
	long	fsid;		/* fs id	*/
	char	lddate[18];	/* load date	*/
} FSTAG30;

#define FSTAG_RCS30 "%8ld%*c%8ld%*c%17c"

#define FSTAG_RVL30(SP) &(SP)->afsid, &(SP)->fsid, (SP)->lddate

#define FSTAG_WCS30 "%8ld %8ld %-17.17s\n"

#define FSTAG_WVL30(SP) (SP)->afsid, (SP)->fsid, (SP)->lddate

#define FSTAG_NULL30 \
{ \
-1,			/* afsid	*/ \
-1,			/* fsid		*/ \
"-",			/* lddate	*/ \
}

#endif
