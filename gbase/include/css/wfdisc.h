/*	SccsId:	%W%	%G%	*/
/**
 *	Wfdisc relation from CSS 3.0 table definitions.
 *	Waveform header file and descriptive information. This relation
 *	provides a pointer (or index) to waveforms that are stored on disk.
 *	The waveforms themselves are stored in ordinary disk files called
 *	wfdisc or .w files, which contain only a sequence of sample values
 *	(usually in binary representation).
 *
 */
#ifndef _WFDISCIO_3_0_H
#define _WFDISCIO_3_0_H

#define WFDISC30_LEN	283

/** 
 *  Wfdisc structure.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member time Epoch time of first sample in file. Initial value = -9999999999.999.
 *  @member wfid Waveform id. Initial value = -1.
 *  @member chanid Channel operation id. Initial value = -1.
 *  @member jdate Julian date. Initial value = -1.
 *  @member endtime Time + (nsamp-1)/samprate. Initial value = -9999999999.999.
 *  @member nsamp Number of samples. Initial value = -1.
 *  @member samprate Sampling rate in samples/second. Initial value = -1.
 *  @member calib Nominal calibration. Initial value = 0.
 *  @member calper Nominal calibration period. Initial value = -1.
 *  @member instype Instrument code. Initial value = "-".
 *  @member segtype Indexing method. Initial value = "-".
 *  @member datatype Numeric storage. Initial value = "-".
 *  @member clip Clipped flag. Initial value = "-".
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member foff Byte offset. Initial value = 0.
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char    sta[7];		/* station 				*/
	char    chan[9];	/* channel 				*/
	double  time;		/* epoch time of first sample in file 	*/	
	long    wfid;		/* waveform id 				*/
	long    chanid;		/* channel operation id 		*/
	long    jdate;		/* julian date 				*/
	double	endtime;	/* time + (nsamp-1)/samprate 		*/
	long    nsamp;		/* number of samples 			*/
	float   samprate;	/* sampling rate in samples/second	*/
	float	calib;		/* nominal calibration			*/
	float	calper;		/* nominal calibration period		*/
	char	instype[7];	/* instrument code			*/
	char	segtype[2];	/* indexing method			*/
	char	datatype[3];	/* numeric storage			*/
	char	clip[2];	/* clipped flag				*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* byte offset				*/
	long	commid;		/* comment id				*/
	char	lddate[18];	/* load date				*/
} WFDISC30;

#define WFDISC_RCS30 "%6c%*c%8c%*c%17lf%*c%8ld%*c%8ld%*c%8ld%*c%17lf%*c%8ld%*c%11f%*c%16f%*c%16f%*c%6c%*c%c%*c%2c%*c%c%*c%64c%*c%32c%*c%10ld%*c%8ld%*c%17c"

#define WFDISC_RVL30(SP) \
(SP)->sta, (SP)->chan, &(SP)->time, &(SP)->wfid, &(SP)->chanid, &(SP)->jdate,\
&(SP)->endtime, &(SP)->nsamp, &(SP)->samprate, &(SP)->calib, &(SP)->calper,\
(SP)->instype, (SP)->segtype, (SP)->datatype, (SP)->clip, (SP)->dir,\
(SP)->dfile, &(SP)->foff, &(SP)->commid, (SP)->lddate

#define WFDISC_WCS30 "%-6.6s %-8.8s %17.5f %8ld %8ld %8ld %17.5f %8ld %11.7f %16.6f %16.6f %-6.6s %1s %-2.2s %1s %-64.64s %-32.32s %10ld %8ld %-17.17s\n"

#define WFDISC_WVL30(SP) \
(SP)->sta, (SP)->chan, (SP)->time, (SP)->wfid, (SP)->chanid, (SP)->jdate,\
(SP)->endtime, (SP)->nsamp, (SP)->samprate, (SP)->calib, (SP)->calper,\
(SP)->instype, (SP)->segtype, (SP)->datatype, (SP)->clip, (SP)->dir,\
(SP)->dfile, (SP)->foff, (SP)->commid, (SP)->lddate

#define WFDISC_NULL30 \
	{ \
		"-",			/* sta 		*/ \
		"-",			/* chan 	*/ \
		-9999999999.999,	/* time 	*/ \
		-1,			/* wfid		*/ \
		-1,			/* chanid	*/ \
		-1,			/* jdate	*/ \
		9999999999.999,		/* endtime	*/ \
		-1,			/* nsamp	*/ \
		-1.,			/* samprate	*/ \
		0.,			/* calib	*/ \
		-1.,			/* calper	*/ \
		"-",			/* instype	*/ \
		"-",			/* segtype	*/ \
		"-",			/* datatype	*/ \
		"-",			/* clip		*/ \
		"-",			/* dir		*/ \
		"-",			/* dfile	*/ \
		0,			/* foff		*/ \
		-1,			/* commid	*/ \
		"-",			/* lddate	*/ \
	}

#endif

#ifndef _WFDISCIO_2_8_H
#define _WFDISCIO_2_8_H


#define WFDISC28_LEN	208

/** 
 *  Wfdisc relation / waveform file header version 2.8
 *  @private
 */
typedef struct
{
	long	date;
	double	time;
	char	sta[8];
	char	chan[4];
	long	nsamp;
	float	smprat;
	float	calib;
	float	calper;
	char	instyp[8];
	char	segtyp[4];
	char	dattyp[4];
	char	clip[4];
	long	chid;
	long	wfid;
	char	dir[32];
	char	file[24];
	long	foff;
	long	adate;
	char	remark[32];
} WFDISC28;

#define WFDISC_RCS28 "%ld%lf%*c%6c%*c%2c%ld%f%f%f%*c%6c%*c%c%*c%2c%*c%c%ld%ld%*c%30c%*c%20c%ld%ld%*c%30c"
/* following extra read string is a gift from Keith */
/* NF means Number of Fields, (just like in awk) */
#define WFDISC_NF	19
#define WFDISC_SIZE 209

#define WFDISC_RVL28(SP) \
&(SP)->date,&(SP)->time,(SP)->sta,(SP)->chan,&(SP)->nsamp,\
&(SP)->smprat,&(SP)->calib,&(SP)->calper,(SP)->instyp,(SP)->segtyp,\
(SP)->dattyp,(SP)->clip,&(SP)->chid,&(SP)->wfid,(SP)->dir,\
(SP)->file,&(SP)->foff,&(SP)->adate,(SP)->remark

#define WFDISC_WCS28 "%8ld %15.3f %-6.6s %-2.2s %8ld %11.7f %9.6f %7.4f %-6.6s %1s %-2.2s %1s %8ld %8ld %-30.30s %-20.20s %10ld %8ld %-30.30s\n"

#define WFDISC_WVL28(SP) \
(SP)->date,(SP)->time,(SP)->sta,(SP)->chan,\
(SP)->nsamp,(SP)->smprat,(SP)->calib,(SP)->calper,\
(SP)->instyp,(SP)->segtyp,(SP)->dattyp,(SP)->clip,(SP)->chid,\
(SP)->wfid,(SP)->dir,(SP)->file,(SP)->foff,\
(SP)->adate,(SP)->remark

#endif
