/*	SccsId:	%W%	%G%	*/
/**
 *	Sensor relation from CSS 3.0 table definitions.
 *	Calibration information for specific sensor channels. This table exists
 *	to provide a record of updates in the calibration factor or clock error
 *	for each instrument, and to link a sta/chan/time to a complete
 *	instrument response in the relation instrument. Waveform data are
 *	converted into physical units through multiplication by the calib
 *	attribute located in wfdisc. It can happen that the correct value of
 *	calib is not accurately known when the wfdisc record is entered into the
 *	database. The sensor relation provides the mechanism (calratio and
 *	calper) to "update" calib, without requiring that hundreds, possibly, of
 *	wfdisc records be updated. 
 *<p>	Through the foreign key inid this table is linked to instrument which,
 *	has fields that point to flat files holding detailed calibration
 *	information in a variety of formats.
 *
 *	@see WFDISC30
 *	@see INSTRUMENT30
 *	
 */
#ifndef _SENSOR_3_0_H
#define _SENSOR_3_0_H

#define SENSOR30_LEN	139

/** 
 *  Sensor structure.
 *  @member sta Station code. Initial value = "-".
 *  @member chan Channel code. Initial value = "-".
 *  @member time Epoch time of start of recording period. Initial value = -9999999999.999.
 *  @member endtime Epoch time of end of recording period. Initial value = -9999999999.999.
 *  @member inid Instrument id. Initial value = -1.
 *  @member chanid Channel id. Initial value = -1.
 *  @member jdate Julian date. Initial value = -1.
 *  @member calratio Calibration. Initial value = -999.
 *  @member calper Calibration period. Initial value = -999.
 *  @member tshift Correction of data processing time. Initial value = -999.
 *  @member instant (y,n) discrete/continuing snapshot. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char    sta[7];		/* station code		*/
	char    chan[9];	/* channel code		*/
	double  time;		/* epoch time of start of recording period */
	double  endtime;	/* epoch time of end of recording period */
	long	inid;		/* instrument id	*/
	long	chanid;		/* channel id		*/
	long	jdate;		/* julian date		*/
	float	calratio;	/* calibration		*/
	float	calper;		/* calibration period	*/
	float	tshift;		/* correction of data processing time */
	char	instant[2];	/* (y,n) discrete/continuing snapshot */
	char	lddate[18];	/* load date		*/
} SENSOR30;

#define SENSOR_RCS30  "%6c%*c%8c%*c%17lf%*c%17lf%*c%8ld%*c%8ld%*c%8ld%*c%16f%*c%16f%*c%6f%*c%1c%*c%17c"

#define SENSOR_RVL30(SP) \
(SP)->sta, (SP)->chan, &(SP)->time, &(SP)->endtime, &(SP)->inid, &(SP)->chanid, \
&(SP)->jdate, &(SP)->calratio, &(SP)->calper, &(SP)->tshift, (SP)->instant, \
(SP)->lddate

#define SENSOR_WCS30  "%-6.6s %-8.8s %17.5lf %17.5lf %8ld %8ld %8ld %16.6f %16.6f %6.2f %1s %-17.17s\n"

#define SENSOR_WVL30(SP) \
(SP)->sta, (SP)->chan, (SP)->time, (SP)->endtime, (SP)->inid, (SP)->chanid, \
(SP)->jdate, (SP)->calratio, (SP)->calper, (SP)->tshift, (SP)->instant, \
(SP)->lddate

#define SENSOR_NULL30 \
{ \
"-",		/* station code		*/ \
"-",		/* channel code		*/ \
-9999999999.999, /* epoch time of start of recording period */ \
-9999999999.999, /* epoch time of end of recording period */ \
-1,		/* instrument id	*/ \
-1,		/* channel id		*/ \
-1,		/* julian date		*/ \
-999.0,		/* calibration		*/ \
-999.0,		/* calibration period	*/ \
-999.0,		/* correction of data processing time */ \
"-",		/* (y,n) discrete/continuing snapshot */ \
"-"		/* load date		*/ \
}

#endif
