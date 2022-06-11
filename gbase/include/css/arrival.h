/*	SccsId:	%W%	%G%	*/
/**
 *	Arrival relation from CSS 3.0 table definitions.
 *	Summary information on a seismic arrival. Information characterizing a
 *	"seismic phase" observerd at a particular station is save here. Many
 *	of the attributes conform to seismological convention and are listed
 *	in earthquake catalogs.
 */
#ifndef _ARRIVAL_3_0_H
#define _ARRIVAL_3_0_H

#define ARRIVAL30_LEN	223

/** 
 *  Arrival structure.
 *  @member sta Station name. Initial value = "-".
 *  @member time Epoch time. Initial value = -9999999999.999.
 *  @member arid Arrival id. Initial value = -1.
 *  @member jdate Julian date. Initial value = -1.
 *  @member stassid Stassoc id. Initial value = -1.
 *  @member chanid Instrument id. Initial value = -1.
 *  @member chan Channel name. Initial value = "-"
 *  @member iphase Reported phase. Initial value = "-"
 *  @member stype Signal type. Initial value = "-"
 *  @member deltim Delta time. Initial value = -1.
 *  @member azimuth Observed azimuth. Initial value = -1.
 *  @member delaz Delta azimuth. Initial value = -1.
 *  @member slow Observed slowness (s/deg). Initial value = -1.
 *  @member delslo Delta slowness. Initial value = -1.
 *  @member ema Emergence angle. Initial value = -1.
 *  @member rect Rectilinearity. Initial value = -1.
 *  @member amp Amplitude, instrument corrected, nm. Initial value = -1.
 *  @member per Period. Initial value = -1.
 *  @member logat Log(amp/per). Initial value = -1.
 *  @member clip Clipped flag. Initial value = "-".
 *  @member fm First motion. Initial value = "-".
 *  @member snr Signal to noise. Initial value = -1.
 *  @member qual Signal onset quality. Initial value =  "-".
 *  @member auth Source/originator. Initial value =  "-".
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	sta[7];		/* station			*/
	double	time;		/* epoch time			*/
	long	arid;		/* arrival id			*/
	long	jdate;		/* julian date			*/
	long	stassid;	/* stassoc id			*/
	long	chanid;		/* instrument id		*/
	char	chan[9];	/* channel			*/
	char	iphase[9];	/* reported phase		*/
	char	stype[2];	/* signal type			*/
	float	deltim;		/* delta time			*/
	float	azimuth;	/* observed azimuth		*/
	float	delaz;		/* delta azimuth		*/
	float	slow;		/* observed slowness (s/deg)	*/
	float	delslo;		/* delta slowness		*/
	float	ema;		/* emergence angle		*/
	float	rect;		/* rectilinearity		*/
	float	amp;		/* amplitude, instrument corrected, nm */
	float	per;		/* period			*/
	float	logat;		/* log(amp/per)			*/
	char	clip[2];	/* clipped flag			*/
	char	fm[3];		/* first motion			*/
	float	snr;		/* signal to noise		*/
	char	qual[2];	/* signal onset quality		*/
	char	auth[16];	/* source/originator		*/
	long	commid;		/* comment id			*/
	char	lddate[18];	/* load date			*/
} ARRIVAL30;

#define ARRIVAL_RCS30 "%6c%*c%17lf%*c%8ld%*c%8ld%*c%8ld%*c%8ld%*c%8c%*c%8c%*c%c%*c%6f%*c%7f%*c%7f%*c%7f%*c%7f%*c%7f%*c%7f%*c%10f%*c%7f%*c%7f%*c%c%*c%2c%*c%10f%*c%c%*c%15c%*c%8ld%*c%17c%"

#define ARRIVAL_RVL30(SP) \
(SP)->sta, &(SP)->time, &(SP)->arid, &(SP)->jdate, &(SP)->stassid, \
&(SP)->chanid, (SP)->chan, (SP)->iphase, (SP)->stype, &(SP)->deltim,\
&(SP)->azimuth, &(SP)->delaz, &(SP)->slow, &(SP)->delslo, &(SP)->ema,\
&(SP)->rect, &(SP)->amp, &(SP)->per, &(SP)->logat, (SP)->clip, (SP)->fm,\
&(SP)->snr, (SP)->qual, (SP)->auth, &(SP)->commid, (SP)->lddate

#define ARRIVAL_WCS30 "%-6.6s %17.5f %8ld %8ld %8ld %8ld %-8.8s %-8.8s %1s %6.3f %7.2f %7.2f %7.2f %7.2f %7.2f %7.3f %10.1f %7.2f %7.2f %1s %-2.2s %10.2f %1s %-15.15s %8ld %-17.17s\n"

#define ARRIVAL_WVL30(SP) \
(SP)->sta, (SP)->time, (SP)->arid, (SP)->jdate, (SP)->stassid, (SP)->chanid,\
(SP)->chan, (SP)->iphase, (SP)->stype, (SP)->deltim, (SP)->azimuth,\
(SP)->delaz, (SP)->slow, (SP)->delslo, (SP)->ema, (SP)->rect, (SP)->amp,\
(SP)->per, (SP)->logat, (SP)->clip, (SP)->fm, (SP)->snr, (SP)->qual,\
(SP)->auth, (SP)->commid, (SP)->lddate

#define ARRIVAL_NULL30 \
{ \
"-",			/* station			*/ \
-9999999999.999,	/* epoch time			*/ \
-1,			/* arrival id			*/ \
-1,			/* julian date			*/ \
-1,			/* stassoc id			*/ \
-1,			/* instrument id		*/ \
"-",			/* channel			*/ \
"-",			/* reported phase		*/ \
"-",			/* signal type			*/ \
-1.,			/* delta time			*/ \
-1.,			/* observed azimuth		*/ \
-1.,			/* delta azimuth		*/ \
-1.,			/* observed slowness (s/deg)	*/ \
-1.,			/* delta slowness		*/ \
-1.,			/* emergence angle		*/ \
-1.,			/* rectilinearity		*/ \
-1.,			/* amplitude, instrument corrected, nm */ \
-1.,			/* period			*/ \
-1.,			/* log(amp/per)			*/ \
"-",			/* clipped flag			*/ \
"-",			/* first motion			*/ \
-1.,			/* signal to noise		*/ \
"-",			/* signal onset quality		*/ \
"-",			/* source/originator		*/ \
-1,			/* comment id			*/ \
"-"			/* load date			*/ \
}

#endif /* _ARRIVAL_3_0_H */

#ifndef _ARRIVAL_2_8_H
#define _ARRIVAL_2_8_H

#define ARRIVAL28_LEN	203

/** 
 *	Arrival relation version 2.8
 *	@private
 */
typedef struct
{
	long	date;		/* julian date			*/
	double	time;		/* epoch time			*/
	char	sta[7];		/* station			*/
	char	chan[3];	/* channel			*/
	char	dig[2];		/* digital data flag		*/
	char	qual[2];	/* signal onset quality		*/
	char	phase[9];	/* reported phase		*/
	char	fm[3];		/* first motion			*/
	float	amp;		/* amplitude 			*/
	float	per;		/* period			*/
	float	logat;		/* log(amp/per)			*/
	float	coda;		/* signal duration		*/
	float	seaz;		/* observed azimuth		*/
	float	slow;		/* observed slowness (s/deg)	*/
	float	ema;		/* emergence angle		*/
	float	dist;		/* estimated distance		*/
	char	stype[2];	/* signal type			*/
	char	clip[2];	/* clipped flag			*/
	long	arid;		/* arrival id			*/
	long	ftid;		/* related arrival id		*/
	long	chid;		/* instrument id		*/
	long	wfid;		/* waveform id			*/
	char	auth[16];	/* source/originator		*/
	char	remark[31];	/* comment 			*/
} ARRIVAL28;

#define ARRIVAL_RCS28 "%8ld%*c%15lf%*c%6c%*c%2c%*c%1c%*c%1c%*c%8c%*c%2c%*c%10f%*c%7f%*c%7f%*c%6f%*c%7f%*c%7f%*c%7f%*c%7f%*c%1c%*c%1c%*c%8ld%*c%8ld%*c%8ld%*c%8ld%*c%15c%*c%30c%"

#define ARRIVAL_RVL28(SP) \
&(SP)->date, &(SP)->time, (SP)->sta, (SP)->chan, (SP)->dig, \
(SP)->qual, (SP)->phase, (SP)->fm, &(SP)->amp, &(SP)->per,\
&(SP)->logat, &(SP)->coda, &(SP)->seaz, &(SP)->slow, &(SP)->ema,\
&(SP)->dist, (SP)->stype, (SP)->clip, &(SP)->arid, &(SP)->ftid,\
&(SP)->chid, &(SP)->wfid, (SP)->auth, (SP)->remark

#define ARRIVAL_WCS28 "%8ld %15.3f %-6.6s %-2.2s %1s %1s %-8.8s %-2.2s %10.1f %7.2f %7.2f %6.1f %7.2f %7.2f %7.2f %7.2f %1s %1s %8ld %8ld %8ld %8ld %-15.15s %-30.30s\n"

#define ARRIVAL_WVL28(SP) \
(SP)->date, (SP)->time, (SP)->sta, (SP)->chan, (SP)->dig,\
(SP)->qual, (SP)->phase, (SP)->fm, (SP)->amp, (SP)->per,\
(SP)->logat, (SP)->coda, (SP)->seaz, (SP)->slow, (SP)->ema,\
(SP)->dist, (SP)->stype, (SP)->clip, (SP)->arid, (SP)->ftid,\
(SP)->chid, (SP)->wfid, (SP)->auth, (SP)->remark

#define ARRIVAL_NULL28 \
{ \
-1,			/* julian date			*/ \
-9999999999.999,	/* epoch time			*/ \
"_",			/* station			*/ \
"_",			/* channel			*/ \
"_",			/* dig				*/ \
"_",			/* qual				*/ \
"_",			/* phase			*/ \
"_",			/* fmt				*/ \
-1.,			/* amplitude			*/ \
-1.,			/* period			*/ \
-1.,			/* log(amp/per)			*/ \
-1.,			/* coda				*/ \
-1.,			/* observed azimuth		*/ \
-1.,			/* observed slowness (s/deg)	*/ \
-1.,			/* emergence angle		*/ \
-1.,			/* estimated distance		*/ \
"_",			/* signal type			*/ \
"_",			/* clipped flag			*/ \
-1,			/* arrival id			*/ \
-1,			/* related arrival id		*/ \
-1,			/* instrument id		*/ \
-1,			/* waveform id			*/ \
"_",			/* source/originator		*/ \
"_"			/* comment 			*/ \
}

#endif /* _ARRIVAL_2_8_H */
