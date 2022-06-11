/*	SccsId:	%W%	%G%	*/
/**
 *	Instrument relation from CSS 3.0 table definitions.
 *	Ancillary calibration information. This table serves three purposes. It
 *	holds nominal on-frequency calibration factors for each instrument. It
 *	holds pointers to the nominal frequency-dependent calibration for an
 *	instrument. Finally, it holds pointers to the exact calibrations
 *	obtained by direct measurement on a particular instrument.
 *
 *	@see SENSOR30
 */
#ifndef _INSTRUMENT_3_0_H
#define _INSTRUMENT_3_0_H

#define INSTRUMENT30_LEN	239

/** 
 *  Instrument structure.
 *  @member inid instrument id. Initial value = -1.
 *  @member insname instrument name. Initial value = "-".
 *  @member instype instrument type. Initial value = "-".
 *  @member band frequency band. Initial value = "-".
 *  @member digital (d,a) analog. Initial value = "-".
 *  @member samprate sampling rate in samples/second. Initial value = -999.
 *  @member ncalib nominal calibration. Initial value = -999.
 *  @member ncalper nominal calibration period. Initial value = -999.
 *  @member dir directory. Initial value = "-".
 *  @member dfile data file. Initial value = "-".
 *  @member rsptype response type. Initial value = "-".
 *  @member lddate load date. Initial value = "-".
 */
typedef struct
{
	long	inid;		/* instrument id	*/
	char	insname[51];	/* instrument name	*/
	char	instype[7];	/* instrument type	*/
	char	band[2];	/* frequency band	*/
	char	digital[2];	/* (d,a) analog		*/
	float	samprate;	/* sampling rate in samples/second */
	float	ncalib;		/* nominal calibration	*/
	float	ncalper;	/* nominal calibration period */
	char	dir[65];	/* directory		*/
	char	dfile[33];	/* data file		*/
	char	rsptype[7];	/* response type	*/
	char	lddate[18];	/* load date		*/
} INSTRUMENT30;

#define INSTRUMENT_WCS30 "%8ld %-50.50s %-6.6s %1s %1s %11.7f %16.6f %16.6f %-64.64s %-32.32s %-6.6s %-17.17s\n"

#define INSTRUMENT_WVL30(SP) \
(SP)->inid, (SP)->insname, (SP)->instype, (SP)->band, (SP)->digital, \
(SP)->samprate, (SP)->ncalib, (SP)->ncalper, (SP)->dir, (SP)->dfile, \
(SP)->rsptype, (SP)->lddate

#define INSTRUMENT_NULL30 \
{ \
-1,		/* instrument id	*/ \
"-",		/* instrument name	*/ \
"-",		/* instrument type	*/ \
"-",		/* frequency band	*/ \
"-",		/* (d,a) analog		*/ \
-999.0,		/* sampling rate in samples/second */ \
-999.0,		/* nominal calibration	*/ \
-999.0,		/* nominal calibration period */ \
"-",		/* directory		*/ \
"-",		/* data file		*/ \
"-",		/* response type	*/ \
"-",		/* load date		*/ \
}

#endif
