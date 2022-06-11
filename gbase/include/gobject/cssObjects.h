/*	SccsId:	%W%	%G%	*/
#ifndef _CSS_OBJECTS_H
#define _CSS_OBJECTS_H

#include <sys/stat.h>

#include "libtime.h"
#include "gobject/GObject.h"
#include "gobject/CssTable.h"
#include "gobject/Vector.h"

/******************************************************************************
	To add a table called "mytable", do each of the following:

	1) In this file, add the CssMytable typedef struct.
	2) In this file, add the prototype statements:
		CssMytable new_CssMytable(void);
		int new_CssMytables(int num, CssMytable *tables);

	3) In cssObjects.c, add static bool defineMytable()
	4) In cssObjects.c, add static bool mytableDefined = False;
	5) In cssObjects.c, add new_CssMytable()
	6) In cssObjects.c, add new_CssMytables()
*******************************************************************************/


/**
 *  Arrival structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssArrival_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station			*/
	double	time;		/* epoch time			*/
	long	arid;		/* arrival id			*/
	long	jdate;		/* julian date			*/
	long	stassid;	/* stassoc id			*/
	long	chanid;		/* instrument id		*/
	char	chan[9];	/* channel			*/
	char	iphase[9];	/* reported phase		*/
	char	stype[2];	/* signal type			*/
	double	deltim;		/* delta time			*/
	double	azimuth;	/* observed azimuth		*/
	double	delaz;		/* delta azimuth		*/
	double	slow;		/* observed slowness (s/deg)	*/
	double	delslo;		/* delta slowness		*/
	double	ema;		/* emergence angle		*/
	double	rect;		/* rectilinearity		*/
	double	amp;		/* amplitude, instrument corrected, nm */
	double	per;		/* period			*/
	double	logat;		/* log(amp/per)			*/
	char	clip[2];	/* clipped flag			*/
	char	fm[3];		/* first motion			*/
	double	snr;		/* signal to noise		*/
	char	qual[2];	/* signal onset quality		*/
	char	auth[16];	/* source/originator		*/
	long	commid;		/* comment id			*/
	DateTime lddate;	/* load date			*/

	/* extra members for geotool internal use */
	int	ts_dc;		/* associated waveform dc */
	int	ts_id;		/* associated waveform id */
	int	ts_copy;	/* associated waveform copy */
	char	phase[9];	/* associated phase (from an assoc) */
	double	amp_cnts;	/* measured amplitude */
	double	amp_Nnms;	/* measured amplitude (nominal) */
	double	amp_nms;	/* measured amplitude (corrected) */
	double	zp_Nnms;	/* zp measured amplitude (nominal) */
	double	period;		/* measured period */
	bool	box_location;
	double	boxtime;	/* position (time) of the left side of the box*/
	double	boxmin;		/* position (counts) of the bottom side */
	int	atype;		/* if > -1, points to ArrType */
	double	value;		/* value to use if using ArrType */
	int	pick_file;	/* pick filename (complete path) */
	int	wftag_file;	/* wftag filename (complete path) */
	int	filter_file;	/* filter filename (complete path) */
	int	pick_index;	/* index of arrival in pick_file */
	int	wftag_index;	/* index of arrival in wftag_file */
	int	filter_index;	/* index of arrival in filter_file */
	int	sta_quark;
	int	chan_quark;
	int	net_quark;
} _CssArrival, *CssArrival;

/**
 *  Wfdisc structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssWfdisc_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station 				*/
	char	chan[9];	/* channel 				*/
	double	time;		/* epoch time of first sample in file 	*/
	long	wfid;		/* waveform id 				*/
	long	chanid;		/* channel operation id 		*/
	long	jdate;		/* julian date 				*/
	double	endtime;	/* time + (nsamp-1)/samprate 		*/
	long	nsamp;		/* number of samples 			*/
	double	samprate;	/* sampling rate in samples/second	*/
	double	calib;		/* nominal calibration			*/
	double	calper;		/* nominal calibration period		*/
	char	instype[7];	/* instrument code			*/
	char	segtype[2];	/* indexing method			*/
	char	datatype[3];	/* numeric storage			*/
	char	clip[2];	/* clipped flag				*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* byte offset				*/
	long	commid;		/* comment id				*/
	DateTime lddate;	/* load date				*/

	int	sta_quark;
	int	chan_quark;
} _CssWfdisc, *CssWfdisc;

/**
 *  History structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member foff File offset. Initial value = 0.
 *  @member foffdff Offset of the data format frame. Initial value = 0.
 *  @member time start of the last processed frame. Initial value = -9999999999.999.
 *  @member pid Process id. Initial value = -1.
 *  @member host Machine where process runs. Initial value = "-".
 *  @member lddate Most recent update time. Initial value = "-".
 *
 */
typedef struct CssHistory_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	binfoff;	/* bin file offset			*/
	long	clffoff;	/* clf file offset			*/
	long	foffdff;	/* offset of the data format frame	*/
	double	time;		/* start of the last processed frame	*/
	long	pid;		/* process id				*/
	char	host[33];	/* machine where process runs		*/
	DateTime lddate;	/* most recent update time		*/
} _CssHistory, *CssHistory;

/**
 *  Channame structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member station       Station name. Initial value = "-".
 *  @member stream        Stream name. Initial value = "-".
 *  @member extern_sta    'external' station. Initial value = "-".
 *  @member extern_chan   'external' channel. Initial value = "-".
 *  @member intern_sta    'internal' station. Initial value = "-".
 *  @member intern_chan   'internal' channel. Initial value = "-".
 *  @member capability    If the channel is capable or not. Initial value = 0.
 *  @member position      Position of this channel in mask. Initial value = 0.
 *  @member revision      Revision of mask. Initial value = 0.
 *  @member lddate Most recent update time. Initial value = "-".
 *
 */
typedef struct CssChanname_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	station[7];	/* station name				*/
	char	stream[7];	/* stream name				*/
	char	extern_sta[7];	/* external station (site) name		*/
	char	extern_chan[9];	/* external channel name		*/
	char	intern_sta[7];	/* internal station (site) name		*/
	char	intern_chan[9];	/* internal channel name		*/
	long	capability;	/* value that specifies if the channel
				    is considered capable or not.	*/
	long	position;	/* position of this channel in the
				    channel mask (starting with 0)	*/
	long	revision;	/* revision of this channel mask
				   (starting with revision 0) */
	DateTime lddate;	/* most recent update time		*/
} _CssChanname, *CssChanname;

/**
 *  Frameproduct structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member frid 'frameproduct ID'. Initial value = 0.
 *  @member type_id type ID. Initial value = 0.
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member foff File offset. Initial value = 0.
 *  @member dsize data size. Initial value = 0.
 *  @member time XXX. Initial value = -9999999999.999.
 *  @member endtime XXX. Initial value = -9999999999.999.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member auth Source/originator. Initial value =  "-".
 *  @member revision XXX. Initial value = 0.
 *  @member obsolete XXX. Initial value = 0.
 *  @member lddate Most recent update time. Initial value = "-".
 *
 */
typedef struct CssFrameproduct_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	frid;		/* Frameproduct ID			*/
	long	type_id;	/* type ID				*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* file offset				*/
	long	dsize;		/* offset of the data format frame	*/
	double	time;		/* start of the last processed frame	*/
	double	endtime;	/* XXX					*/
	char	sta[7];		/* station				*/
	char	chan[9];	/* channel				*/
	char	author[17];	/* author				*/
	double	version;	/* version				*/
	long	revision;	/* reversion				*/
	long	obsolete;	/* obsolete				*/
	DateTime lddate;	/* most recent update time		*/
} _CssFrameproduct, *CssFrameproduct;

/**
 *  Fpdescription structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member type_id ID for the product type descr. Initial value = 0.
 *  @member prodtype Name of the product. Initial value = "-".
 *  @member name Descriptive listing of the product name. Initial value = "-".
 *  @member msgdtype Type of data. Initial value = "-".
 *  @member msgdformat Format of the data. Initial value = "-".
 *  @member header_fpid fpid pointing to the header row for this product type. Initial value = "-".
 *
 */
typedef struct CssFpdescription_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	type_id;	/* typeid ID for the product type descr	*/
	char	prodtype[13];	/* Name of the product			*/
	char	name[65];	/* Descriptive listing of the product name */
	char	msgdtype[17];	/* Type of data				*/
	char	msgdformat[9];	/* Descriptive listing of the product name  */
	long	header_fpid;	/* fpid pointing to the header row for this */
				/* product type				*/
	DateTime lddate;	/* most recent update time		*/
} _CssFpdescription, *CssFpdescription;

/**
 *  md5_digest structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member dir Directory. Initial value = "-".
 *  @member dfile Data file. Initial value = "-".
 *  @member inode Inode. Initial value = 0.
 *  @member digest XXX . Initial value = "-".
 *  @member lddate Most recent update time. Initial value = "-".
 *
 */
typedef struct CssMd5_digest_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	dir[65];	/* directory			*/
	char	dfile[33];	/* data file			*/
	long	inode;		/* inode XXX			*/
	char	digest[33];	/* XXXX				*/
	DateTime lddate;	/* most recent update time	*/
} _CssMd5_digest, *CssMd5_digest;

/**
 *  clf structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member fpid File pointer ID. Initial value = 0.
 *  @member frameid Frame ID. Initial value = "-".
 *  @member time Receipt time of the frame. Initial value = -9999999999.999.
 *  @member timestamp Nominal time stamp of the frame. Initial value = -9999999999.999.
 *  @member foff File offset of the start of the frame in the file. Initial value = 0.
 *  @member length Length of the frame in bytes. Initial value = 0.
 *  @member duration Duration of frame in seconds. Initial value = 0.
 *  @member frametype Type of frame Frame type. Initial value = "-".
 *  @member verifstatus Verification status. Initial value = "-".
 *  @member pcode Parsing code (see cdrecv documentation). Initial value = "-".
 *  @member mask Channel mask, e.g., '0011101' 1-channel chanel is present,
 *          0 - channel is not present. Initial value = "-".
 *  @member revision Revision of the channel mask (id in the Chanxxx table). Initial value = -1.
 *  @member lddate Most recent update time. Initial value = "-".
 *
 */
typedef struct CssClf_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	frid;		/* Frameproduct ID for this clf file	*/
	char	frameid[39];	/* Frame ID	*/
	double	time;		/* Receipt time of the frame	*/
	double	timestamp;	/* Nominal time stamp of the frame	*/
 	double	receivespan;	/* XXX					*/
	long	foff;	        /* File offset of the start of the frame in the file */
	long	length;		/* Length of the frame in bytes	*/
	long	duration;	/* frame duration	*/
	char	frametype[3];	/* Type of frame	*/
	char	verifstatus[5]; /* Verification status	*/
	char	pcode[9];	/* Parsing code (see cdrecv documentation) */
	char	mask[65];	/* Channel mask, e.g., '0011101' 1-channel */
				/* chanel is present, 0 - channel is not   */
				/* present	*/
        long	revision;	/* Revision of the channel mask (id in the */
				/* Chanxxx table)	*/
	DateTime lddate;	/* Most recent update time	*/
} _CssClf, *CssClf;


/**
 *  waveinterval structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member wintid Unique index for this table. Initial value = 0.
 *  @member station Station name. Initial value = "-".
 *  @member time Start time of the interval. Initial value = -9999999999.999.
 *  @member endtime End time of the interval (seismic intervals are normally 10 min, infra are 30 min). Initial value = -9999999999.999.
 *  @member percentdata Percent of data available for a given station for a given interval. Initial value = 0..
 *  @member percentchans Percent of channels available for a given station for a given interval. Initial value = 0..
 *  @member percentusable Percent of usable data for a given interval. Initial value = 0..
 *  @member percenttimely Percent of data received within 5 minutes. Initial value = 0..
 *  @member status Always set to "new" by cd2w. Initial value = "new".
 *  @member mask "or" of all masks in clf entries. Initial value = "-".
 *  @member createdate Create date of this record. Initial value = "-".
 *  @member lddate Last update time for this record. Initial value = "-".
 *
 */
typedef struct CssWaveinterval_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	wintid;    	/* Unique ID  for this table	*/
	char	station[7];	/* Station name			*/
	double	time;		/* start of the interval	*/
	double	endtime;	/* end time of the interval	*/
	float	percentdata;	/* Percent of data available for a	*/
				/* given station for a given interval	*/
	float	percentchans;	/* Percent of channels available for a	*/
				/* given station for a given interval	*/
	float	percentusable;	/* Percent of usable data*/
	float	percenttimely;	/* Percent of data received within 5 minutes	*/
	char	status[7];	/* initialized to "new" by cd2w	*/
	char	mask[65];	/* "or" of all masks in clf entries */
	DateTime createdate;	/* Create date of this record		*/
	DateTime lddate;	/* Last update time for this record	*/
} _CssWaveinterval, *CssWaveinterval;

/**
 *  outage structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *
 *  @member otgid .
 *  @member sta Station name. Initial value = "-".
 *  @member chan Channel name. Initial value = "-".
 *  @member auxid .
 *  @member time .
 *  @member endtime .
 *  @member status .
 *  @member auth .
 *  @member available .
 *  @member commid .
 *  @member lddate .
 *
 */
typedef struct CssOutage_s
{
	GObjectPart     core;
	CssTablePart    css;

	long	otgid;		/* */
	char	sta[7];		/* */
	char	chan[9];	/* */
	char	auxid[5];	/* */
	double	time;		/* */
	double	endtime;	/* */
	char	status[33];	/* */
	char	auth[16];	/* */
	char	available[2];	/* */
	long	commid;		/*  */
	DateTime lddate;	/* most recent update time */
} _CssOutage, *CssOutage;

/**
 *  Origin structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member lat estimated latitude. Initial value = -999.
 *  @member lon estimated longitude. Initial value = -999.
 *  @member depth estimated depth. Initial value = -999.
 *  @member time epoch time. Initial value = -9999999999.999.
 *  @member orid origin id. Initial value = -1.
 *  @member evid event id. Initial value = -1.
 *  @member jdate julian date. Initial value = -1.
 *  @member nass number of associated phases. Initial value = -1.
 *  @member ndef number of locating phases. Initial value = -1.
 *  @member ndp number of depth phases. Initial value = -1.
 *  @member grn geographic region number. Initial value = -1.
 *  @member srn seismic region number. Initial value = -1.
 *  @member etype event type. Initial value = -1.
 *  @member depdp estimated depth from depth phases. Initial value = -999.
 *  @member dtype depth method used. Initial value = "-".
 *  @member mb body wave magnitude. Initial value = -1.
 *  @member mbid mb magid. Initial value = -1.
 *  @member ms surface wave magnitude. Initial value = -1.
 *  @member msid ms magid. Initial value = -1.
 *  @member ml local magnitude. Initial value = -1.
 *  @member mlid ml magid. Initial value = -1.
 *  @member algorithm location algorithm used. Initial value = "-".
 *  @member auth source/originator. Initial value = "-".
 *  @member commid comment id. Initial value = -1.
 *  @member lddate load date. Initial value = "-".
 */

typedef struct CssOrigin_s
{
	GObjectPart	core;
	CssTablePart	css;

	double	lat;		/* estimated latitude		*/
	double	lon;		/* estimated longitude		*/
	double	depth;		/* estimated depth		*/
	double	time;		/* epoch time			*/
	long	orid;		/* origin id			*/
	long	evid;		/* event id			*/
	long	jdate;		/* julian date			*/
	long	nass;		/* number of associated phases	*/
	long	ndef;		/* number of locating phases	*/
	long	ndp;		/* number of depth phases	*/
	long	grn;		/* geographic region number	*/
	long	srn;		/* seismic region number	*/
	char	etype[8];	/* event type			*/
	double	depdp;		/* estimated depth from depth phases */
	char	dtype[2];	/* depth method used		*/
	double	mb;		/* body wave magnitude		*/
	long	mbid;		/* mb magid			*/
	double	ms;		/* surface wave magnitude	*/
	long	msid;		/* ms magid			*/
	double	ml;		/* local magnitude		*/
	long	mlid;		/* ml magid			*/
	char	algorithm[16];	/* location algorithm used	*/
	char	auth[16];	/* source/originator		*/
	long	commid;		/* comment id			*/
	DateTime lddate;	/* load date			*/

	/* extra members for geotool internal use */
	int	wftag_file;	/* wftag filename (complete path) */
	int	origerr_file;	/* origerr filename (complete path) */
	int	wftag_index;	/* index of wftag in wftag_file */
	int	origerr_index;	/* index of origerr in origerr_file */
} _CssOrigin, *CssOrigin;

/**
 *  Origerr structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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

typedef struct CssOrigerr_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	orid;		/* origerr id			*/
	double	sxx;		/* covariance matrix element	*/
	double	syy;		/* covariance matrix element	*/
	double	szz;		/* covariance matrix element	*/
	double	stt;		/* covariance matrix element	*/
	double	sxy;		/* covariance matrix element	*/
	double	sxz;		/* covariance matrix element	*/
	double	syz;		/* covariance matrix element	*/
	double	stx;		/* covariance matrix element	*/
	double	sty;		/* covariance matrix element	*/
	double	stz;		/* covariance matrix element	*/
	double	sdobs;		/* std err of obs 		*/
	double	smajax;		/* semi-major axis of error	*/
	double	sminax;		/* semi-minor axis of error	*/
	double	strike;		/* strike of semi-major axis	*/
	double	sdepth;		/* depth error			*/
	double	stime;		/* origin time error		*/
	double	conf;		/* confidence			*/
	long	commid;		/* comment id			*/
	DateTime lddate;	/* load date			*/
} _CssOrigerr, *CssOrigerr;

/**
 *  Origaux structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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

typedef struct CssOrigaux_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	event[9];	/* Unique event identification (string) */
	char	otfixf[2]; /* Flag designating that an origin time is fixed */
	char	epfixf[2]; /* Flag designating that an epicenter is fixed */
	long	nsta;		/* number of defining stations */
	long	gap;		/* Gap in azimuthal coverage (0<=gap<=360) */
	char	ident[9];	/* unique ident (string)	*/
	double	mindist;	/* distance to closest station (degrees) */
	double	maxdist;	/* distance to farthest station (degrees) */
	char	antype[2];	/* analysis type	*/
	long	evid;		/* event identifier	*/
	long	orid;		/* origin idenfitier	*/
	DateTime lddate;	/* load date		*/
} _CssOrigaux, *CssOrigaux;

/**
 *  Lastid structure.
 *
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member keyname Id name (arid, orid, etc.).
 *  @member keyvalue Last value used for that id.
 *  @member lddate Load date.
 */

typedef struct CssLastid_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	keyname[16];	/* id name (arid, orid, etc.)	*/
	long	keyvalue;	/* last value used for that id	*/
	DateTime lddate;	/* load date			*/
} _CssLastid, *CssLastid;

/////

/**
 *  Gregion structure.
 *
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member keyname Id name (arid, orid, etc.).
 *  @member keyvalue Last value used for that id.
 *  @member lddate Load date.
 */

typedef struct CssGregion_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	grn;	/* last value used for that id	*/
	char	grname[39];	/* id name (arid, orid, etc.)	*/
	char    lddate[10];	/* load date			*/
	
} _CssGregion, *CssGregion;




/**
 *  Sensor structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssSensor_s
{
	GObjectPart	core;
	CssTablePart	css;

	char    sta[7];		/* station code		*/
	char    chan[9];	/* channel code		*/
	double  time;		/* epoch time of start of recording period */
	double  endtime;	/* epoch time of end of recording period */
	long	inid;		/* instrument id	*/
	long	chanid;		/* channel id		*/
	long	jdate;		/* julian date		*/
	double	calratio;	/* calibration		*/
	double	calper;		/* calibration period	*/
	double	tshift;		/* correction of data processing time */
	char	instant[2];	/* (y,n) discrete/continuing snapshot */
	DateTime lddate;	/* load date		*/

	/* extra members for geotool internal use */
	int	sta_quark;
	int	chan_quark;
} _CssSensor, *CssSensor;

/**
 *  Instrument structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssInstrument_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	inid;		/* instrument id	*/
	char	insname[51];	/* instrument name	*/
	char	instype[7];	/* instrument type	*/
	char	band[2];	/* frequency band	*/
	char	digital[2];	/* (d,a) analog		*/
	double	samprate;	/* sampling rate in samples/second */
	double	ncalib;		/* nominal calibration	*/
	double	ncalper;	/* nominal calibration period */
	char	dir[65];	/* directory		*/
	char	dfile[33];	/* data file		*/
	char	rsptype[7];	/* response type	*/
	DateTime lddate;	/* load date		*/
} _CssInstrument, *CssInstrument;

/**
 *  Sitechan structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member sta Station identifier. Initial value = "-".
 *  @member chan Channel identifier. Initial value = "-".
 *  @member ondate Julian start date. Initial value = -1.
 *  @member chanid Channel id. Initial value = -1.
 *  @member offdate Julian off date. Initial value = -1.
 *  @member ctype Channel type. Initial value = "-".
 *  @member edepth Emplacement depth. Initial value = -999.
 *  @member hang Horizontal angle. Initial value = -999.
 *  @member vang Vertical angle. Initial value = -999.
 *  @member descrip Channel description. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssSitechan_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station identifier	*/
	char    chan[9];	/* channel identifier	*/
	long	ondate;		/* Julian start date	*/
	long	chanid;		/* channel id		*/
	long	offdate;	/* Julian off date	*/
	char	ctype[5];	/* channel type		*/
	double	edepth;		/* emplacement depth	*/
	double	hang;		/* horizontal angle	*/
	double	vang;		/* vertical angle	*/
	char	descrip[51];	/* channel description	*/
	DateTime lddate;	/* load date		*/

	/* extra members for geotool internal use */
	int	sta_quark;
	int	chan_quark;
	int	chan_alt;
} _CssSitechan, *CssSitechan;

/**
 *  Site structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member sta Station identifier. Initial value = "-".
 *  @member ondate Julian start date. Initial value = -1.
 *  @member offdate Julian off date. Initial value = -1.
 *  @member lat Latitude. Initial value = -999.
 *  @member lon Longitude. Initial value = -999.
 *  @member elev Elevation. Initial value = -999.
 *  @member staname Station description. Initial value = "-".
 *  @member statype Station type: single station, virt array, etc.. Initial value = "-".
 *  @member refsta Reference station for array members. Initial value = "-".
 *  @member dnorth Offset from array reference (km). Initial value = 0.
 *  @member deast Offset from array reference (km). Initial value = 0.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssSite_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station identifier	*/
	long	ondate;		/* Julian start date	*/
	long	offdate;	/* Julian off date	*/
	double	lat;		/* latitude		*/
	double	lon;		/* longitude		*/
	double	elev;		/* elevation		*/
	char	staname[51];	/* station description	*/
	char	statype[5];	/* station type: single station, virt array,
					etc. */
	char	refsta[7];	/* reference station for array members */
	double	dnorth;		/* offset from array reference (km) */
	double	deast;		/* offset from array reference (km) */
	DateTime lddate;	/* load date		*/

	/* extra members for geotool internal use */
	int	sta_quark;
	int	refsta_quark;
} _CssSite, *CssSite;

/**
 *  Wftag structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member tagname Key (arid, orid, etc.). Initial value = "-".
 *  @member tagid Tagname value. Initial value = -1.
 *  @member wfid Waveform id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssWftag_s
{
	GObjectPart	core;
	CssTablePart	css;

	char    tagname[9];	/* key (arid, orid, etc.)	*/
	long    tagid;		/* tagname value		*/
	long    wfid;		/* waveform id			*/
	DateTime lddate;	/* load date			*/
} _CssWftag, *CssWftag;

/**
 *  Xtag structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member thisid Initial value = -1.
 *  @member thatid Initial value = -1.
 *  @member thisname key (arid, orid, etc.). Initial value = "-".
 *  @member thatname key (arid, orid, etc.). Initial value = "-".
 *  @member dbname dbname or [S|C]/sta/stadate. Initial value = "-".
 *  @member lddate load date. Initial value = "-".
 */
typedef struct CssXtag_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	thisid;
	long	thatid;
	char    thisname[9];	/* key (arid, orid, etc.)	*/
	char    thatname[9];	/* key (arid, orid, etc.)	*/
	char	dbname[26];	/* dbname or [S|C]/sta/stadate  */
	DateTime lddate;	/* load date			*/

	/* extra members for geotool internal use */
	long	tagid;
	long	wfid;
	double	mintime;
	double	maxtime;
	char	sta[7];
	char	dir[256];
} _CssXtag, *CssXtag;

/**
 *  fsdisc structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member jdate Julian date. Initial value = -1.
 *  @member time Epoch time of first sample in file. Initial value = -9999999999.999.
 *  @member tlen Time window. Initial value = -1.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member fstype Fourier spectrum type. Initial value = "-".
 *  @member arid Arrival id. Initial value = -1.
 *  @member maxf Maximum frequency. Initial value = -1.
 *  @member nf Number of frequency values. Initial value = -1.
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
typedef struct CssFsdisc_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	jdate;		/* julian date				*/
	double	time;		/* epoch time of first sample in file	*/
	double	tlen;		/* time window				*/
	char	sta[7];		/* station				*/
	char	chan[9];	/* channel				*/
	char	fstype[5];	/* Fourier spectrum type		*/
	long	arid;		/* arrival id				*/
	double	maxf;		/* max frequency			*/
	long	nf;		/* number of frequency values		*/
	double	samprate;	/* sampling rate in samples/second	*/
	long	chanid;		/* channel id				*/
	long	wfid;		/* waveform id				*/
	long	fsrid;		/* fs recipe id				*/
	long	fsid;		/* fs id				*/
	char	datatype[3];	/* numeric storage			*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* byte offset				*/
	long	commid;		/* comment id				*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
	int	chan_quark;
} _CssFsdisc, *CssFsdisc;

/**
 *  fsave structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssFsave_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station				*/
	char	avtype[9];	/* fs channel (ave, med, 95perct, etc)	*/
	char	fstype[5];	/* Fourier spectrum type		*/
	double	maxf;		/* max frequency			*/
	long	nf;		/* number of frequency values		*/
	long	nave;		/* number of spcetra used in this average */
	long	afsid;		/* ave fs id				*/
	double	noissd;		/* std dev of log noise			*/
	char	datatype[3];	/* numeric storage			*/
	char	dir[65];	/* directory				*/
	char	dfile[33];	/* data file				*/
	long	foff;		/* byte offset				*/
	long	commid;		/* comment id				*/
	DateTime lddate;	/* load date				*/

	int	sta_quark;
	int	chan_quark;
} _CssFsave, *CssFsave;

/**
 *  fsrecipe structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssFsrecipe_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	fsrid;		/* fs recipe id			*/
	char	fsdesc[16];	/* desc of fs, med/ave day/night, etc */
	char	taper[9];	/* type of taper		*/
	long	taperstart;	/* starting percent for cosine taper */
	long	taperend;	/* ending percent for cosine taper	*/
	double	winlen;		/* number of seconds in windows	*/
	long	overlap;	/* percent of overlap between adjacent windows*/
	long	nfft;		/* number of points in fft	*/
	double	smoothvalue;	/* amount of smoothing (Hz)	*/
	char	response[2];	/* flag if corrected for instrument response */
	DateTime lddate;	/* load date			*/
} _CssFsrecipe, *CssFsrecipe;

/**
 *  Fstag structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member afsid fs id. Initial value = -1.
 *  @member fsid fs id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssFstag_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	afsid;		/* fs id	*/
	long	fsid;		/* fs id	*/
	DateTime lddate;	/* load date	*/
} _CssFstag, *CssFstag;

/**
 *  Spdisc structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssSpdisc_s
{
	GObjectPart	core;
	CssTablePart	css;

	long    jdate;          /* julian date                          */
	double  time;           /* epoch time of first sample in file   */
	double	tlen;		/* overall time of spectrogram		*/
	char	sta[7];		/* station				*/
	char	chan[9];	/* channel				*/
	int     winpts;         /* number of points in window	  	*/
	int     overlap;        /* percent of overlap between adjacent windows*/
	int	nwin;		/* number of windows			*/
	double	lofreq;		/* lo frequency				*/
	double	hifreq;		/* hi frequency				*/
	long	nf;             /* number of frequency values           */
	long	spid;		/* sp id				*/
	char    datatype[3];    /* numeric storage                      */
	char    dir[65];        /* directory                            */
	char    dfile[33];      /* data file                            */
	long    foff;           /* byte ofspet                          */
	long	commid;		/* comment id				*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
	int	chan_quark;
} _CssSpdisc, *CssSpdisc;

/**
 *  CssDervdisc structure.
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
typedef struct CssDervdisc_s
{
	GObjectPart	core;
	CssTablePart	css;

	long    jdate;          /* julian date                          */
	double  time;           /* epoch time of first sample in file   */
	double	tlen;		/* overall time of spectrogram		*/
	char	net[7];		/* station				*/
	char	sta[7];		/* station				*/
	char	chan[9];	/* station				*/
	long	dervid;		/* pmcc id				*/
	char	method[16];	/* station				*/
	long	recid;		/* recipe id			*/
	char    datatype[3];    /* numeric storage                      */
	char    dervtype[5];    /* derived type                         */
	char    dir[65];        /* directory                            */
	char    dfile[33];      /* data file                            */
	long    foff;           /* byte ofspet                          */
	long	commid;		/* comment id				*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
	int	chan_quark;
} _CssDervdisc, *CssDervdisc;

typedef struct CssPmccRecipe_s
{
        GObjectPart     core;
        CssTablePart    css;

        long    pmccrecid;      /* pmcc recipe id                       */
        long    deflt;          /* default */
	long	fGroup;		/* filter group 		        */
        double	winlen;         /* number of points in window           */
        double	wingap;         /* number of points in window           */
        double	threshCons;     /* number of points in window           */
        long	threshNsens;    /* number of points in window           */
        double	qFactor;        /* number of points in window           */
	long	pmcc3D;
	double	sound_speed;
	double	elevation_angle;
        double	threshFamLen;	/* number of points in window           */
        long	threshFamMin;	/* number of points in window           */
        long	threshFamMax;	/* number of points in window           */
        double	speedTransition;     /* number of points in window           */
        double	timeTol;
        double	freqTol;
        double	speedTol1;
        double	speedTol2;
        double	azTol1;
        double	azTol2;
        char    auth[16];
        DateTime lddate;     /* load date                    */

	int	sta_quark;
} _CssPmccRecipe, *CssPmccRecipe;


typedef struct CssPmccFeatures_s
{
        GObjectPart     core;
        CssTablePart    css;

        char	sta[7];         /* station                      */
        long 	arid;           /* arrival id                   */
        double	initial_time;
        double	duration;
        double	consistency;
	double	correlation;
        long	famsize;
        double	minfreq;
        double	maxfreq;
	double	cfreq;
	double	sigmafreq;
	double	rmsamp;
        char    auth[16];
        long	commid;
        DateTime lddate;     /* load date                    */

	int	sta_quark;
} _CssPmccFeatures, *CssPmccFeatures;

/**
 *  Pick structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member chanid Channed id. Initial value = -1.
 *  @member time Epoch time of measurement. Initial value = -9999999999.999.
 *  @member arid Arrival id. Initial value = -1.
 *  @member amptype pp, zp or rms. Initial value = "-".
 *  @member amp Amplitude in counts. Initial value = -1.
 *  @member per Period. Initial value = -1.
 *  @member calib Calibration factor at calper. Initial value = 0.
 *  @member calper Calibration period. Initial value = -1.
 *  @member ampcalib Calibration at per. Initial value = 0.
 *  @member ampmin Min data value(cnts) for pp measurement. Initial value = -1.
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssPick_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station			*/
	char	chan[9];	/* channel			*/
	long	chanid;		/* channed id			*/
	double	time;		/* epoch time of measurement	*/
	long	arid;		/* arrival id			*/
	char	amptype[4];	/* pp, zp or rms		*/
	double	amp;		/* amplitude in counts		*/
	double	per;		/* period			*/
	double	calib;		/* calibration factor at calper	*/
	double	calper;		/* calibration period		*/
	double	ampcalib;	/* calibration at per		*/
	double	ampmin;		/* min data value(cnts) for pp measurement */
	long	commid;		/* comment id			*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
	int	chan_quark;
} _CssPick, *CssPick;

/**
 *  Filter structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member chanid Channel id. Initial value = -1.
 *  @member arid Arrival id. Initial value = -1.
 *  @member wfid Waveform id. Initial value = -1.
 *  @member band lp, hp, bp, or br. Initial value = "-".
 *  @member ftype 'z': zero phase, 'c': causal. Initial value = "-".
 *  @member forder Filter order. Initial value = -1.
 *  @member lofreq Low cut frequency (Hz). Initial value = -1.
 *  @member hifreq High cut frequency (Hz). Initial value = -1.
 *  @member algo Filter algorithm. Initial value = "-".
 *  @member program Program which uses filter. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssFilter_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	sta[7];		/* station			*/
	char	chan[9];	/* channel			*/
	long	chanid;		/* channel id			*/
	long	arid;		/* arrival id			*/
	long	wfid;		/* waveform id			*/
	char	band[3];	/* lp, hp, bp, or br		*/
	char	ftype[2];	/* 'z': zero phase, 'c': causal	*/
	long	forder;		/* filter order			*/
	double	lofreq;		/* low cut frequency (Hz)	*/
	double	hifreq;		/* high cut frequency (Hz)	*/
	char	algo[31];	/* filter algorithm		*/
	char	program[16];	/* program which uses filter	*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
	int	chan_quark;
} _CssFilter, *CssFilter;

/**
 *  Assoc structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
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
typedef struct CssAssoc_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	arid;		/* arrival id			*/
	long	orid;		/* origin id			*/
	char	sta[7];		/* station code			*/
	char	phase[9];	/* associated phase		*/
	double	belief;		/* phase confidence		*/
	double	delta;		/* station to event distance	*/
	double	seaz;		/* station to event azimuth	*/
	double	esaz;		/* event to station azimuth	*/
	double	timeres;	/* time residual		*/
	char	timedef[2];	/* time = defining,non-defining */
	double	azres;		/* azimuth residual		*/
	char	azdef[2];	/* azimuth = defining,non-defining */
	double	slores;		/* slowness residual		*/
	char	slodef[2];	/* slowness = defining,non-defining */
	double	emares;		/* incidence angle residual	*/
	double	wgt;		/* location weight		*/
	char	vmodel[16];	/* velocity model		*/
	long	commid;		/* comment id			*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
} _CssAssoc, *CssAssoc;

/**
 *  Stassoc structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member stassid Stassoc id. Initial value = -1.
 *  @member sta Station code. Initial value = "-".
 *  @member etype Event type. Initial value = "-".
 *  @member location Apparent location description. Initial value = "-".
 *  @member dist Estimated distance. Initial value = -1.
 *  @member azimuth Observed azimuth. Initial value = -1.
 *  @member lat Estimated latitude. Initial value = -999.
 *  @member lon Estimated longitude. Initial value = -999.
 *  @member depth Estimated depth. Initial value = -999.
 *  @member time Estimated origin time. Initial value = -9999999999.999.
 *  @member imb Initial estimated mb. Initial value = -999.
 *  @member ims Initial estimated ms. Initial value = -999.
 *  @member iml Initial estimated ml. Initial value = -999.
 *  @member auth Source/originator. Initial value = "-".
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssStassoc_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	stassid;	/* stassoc id			*/
	char	sta[7];		/* station code			*/
	char	etype[8];	/* event type			*/
	char	location[33];	/* apparent location description */
	double	dist;		/* estimated distance		*/
	double	azimuth;	/* observed azimuth		*/
	double	lat;		/* estimated latitude		*/
	double	lon;		/* estimated longitude		*/
	double	depth;		/* estimated depth		*/
	double	time;		/* estimated origin time	*/
	double	imb;		/* initial estimated mb		*/
	double	ims;		/* initial estimated ms		*/
	double	iml;		/* initial estimated ml		*/
	char	auth[16];	/* source/originator		*/
	long	commid;		/* comment id			*/
	DateTime lddate;	/* load date			*/

	int	sta_quark;
} _CssStassoc, *CssStassoc;

/**
 *  Affiliation struct.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member net Unique network identifier. Initial value = "-".
 *  @member sta Station identifier. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssAffiliation_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	net[9];		/* unique network identifier	*/
	char	sta[7];		/* station identifier		*/
	DateTime lddate;	/* load date			*/

	/* extra members for geotool internal use */
	int	net_quark;
	int	sta_quark;
} _CssAffiliation, *CssAffiliation;

/**
 *  Stanet struct.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member net Unique network identifier. Initial value = "-".
 *  @member sta Station identifier. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssStanet_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	net[9];		/* unique network identifier	*/
	char	sta[7];		/* station identifier		*/
	DateTime lddate;	/* load date			*/

	int	net_quark;
	int	sta_quark;
} _CssStanet, *CssStanet;

/**
 *  Hydro_features structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member arid Arrival id. Initial value = -1.
 *  @member peak_time Initial value = -999.
 *  @member peak_level Initial value = -999.
 *  @member total_energy Initial value = -999.
 *  @member mean_arrival_time Initial value = -999.
 *  @member time_spread Initial value = -999.
 *  @member onset_time Initial value = -999.
 *  @member termination_time Initial value = -999.
 *  @member total_time Initial value = -999.
 *  @member num_cross Initial value = -1.
 *  @member ave_noise Initial value = -999.
 *  @member skewness Initial value = -999.
 *  @member kurtosis Initial value = -999.
 *  @member cep_var_signal Initial value = -999.
 *  @member cep_delay_time_signal Initial value = -999.
 *  @member cep_peak_std_signal Initial value = -999.
 *  @member cep_var_trend Initial value = -999.
 *  @member cep_delay_time_trend Initial value = -999.
 *  @member cep_peak_std_trend Initial value = -999.
 *  @member low_cut Initial value = -999.
 *  @member high_cut Initial value = -999.
 *  @member ford Initial value = -1.
 *  @member ftype Initial value = "-".
 *  @member fzp Initial value = -1.
 *  @member prob_weight_time Initial value = -999.
 *  @member sigma_time Initial value = -999.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssHydroFeatures_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	arid;		/* arrival id			*/
	double	peak_time;
	double	peak_level;
	double	total_energy;
	double	mean_arrival_time;
	double	time_spread;
	double	onset_time;
	double	termination_time;
	double	total_time;
	long	num_cross;
	double	ave_noise;
	double	skewness;
	double	kurtosis;
	double	cep_var_signal;
	double	cep_delay_time_signal;
	double	cep_peak_std_signal;
	double	cep_var_trend;
	double	cep_delay_time_trend;
	double	cep_peak_std_trend;
	double	low_cut;
	double	high_cut;
	long	ford;
	char	ftype[3];
	long	fzp;
	double	prob_weight_time;
	double	sigma_time;
	DateTime lddate;	/* load date			*/
} _CssHydroFeatures, *CssHydroFeatures;

/**
 *  Infra_features structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member arid Arrival id. Initial value = -1.
 *  @member eng_time Initial value = -9999999999.999.
 *  @member eng_dur Initial value = -999.
 *  @member eng_deldur Initial value = -1.
 *  @member coh_time Initial value = -9999999999.999.
 *  @member coh_dur Initial value = -999.
 *  @member coh_deldur Initial value = -1.
 *  @member coinc_time Initial value = -9999999999.999.
 *  @member coinc_dur Initial value = -999.
 *  @member ninc_deldur Initial value = -1.
 *  @member ford Initial value = 0.
 *  @member zrcr_freq Initial value = -1.
 *  @member zrcr_delfreq Initial value = -1.
 *  @member crnr_freq Initial value = -1.
 *  @member crnr_delfreq Initial value = -1.
 *  @member coh_per Initial value = -999.
 *  @member coh_snr Initial value = -1.
 *  @member total_energy Initial value = -1.
 *  @member auth Initial value = "-".
 *  @member commid Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct CssInfraFeatures_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	arid;		/* arrival id			*/
	double	eng_time;
	double 	eng_dur;
	double 	eng_deldur;
	double	coh_time;
	double 	coh_dur;
	double 	coh_deldur;
	double	coinc_time;
	double 	coinc_dur;
	double 	coinc_deldur;
	int	ford;
	double 	zrcr_freq;
	double 	zrcr_delfreq;
	double 	crnr_freq;
	double 	crnr_delfreq;
	double 	coh_per;
	double 	coh_snr;
	double 	total_energy;
	char	auth[16];
	long   	commid;
	DateTime lddate;	/* load date			*/
} _CssInfraFeatures, *CssInfraFeatures;

/**
 *  Stamag structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member magid Magnitude id. Initial value = -1.
 *  @member ampid Amplitude id. Initial value = -1.
 *  @member sta Station identifier. Inital value = "-".
 *  @member arid Arrival id. Inital value = -1.
 *  @member orid Origin id. Inital value = -1.
 *  @member evid Event id. Inital value = -1.
 *  @member phase Associated phase. Inital value = "-".
 *  @member delta Station-to-event dist. Inital value = -999.
 *  @member magtype Magnitude type (mb). Inital value = "-".
 *  @member magnitude Magnitude. Inital value = -999.
 *  @member uncertainty Magnitude uncertainty. Inital value = -999.
 *  @member magres Magnitude residual. Inital value = -999.
 *  @member magdef Flag if mag is defining. Inital value = "-".
 *  @member mmodel Magnitude model. Inital value = "-".
 *  @member auth Author. Inital value = "-".
 *  @member commid Comment id. Inital value = "-".
 *  @member lddate Load date. Inital value = "-".
 */
typedef struct CssStamag_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	magid;		/* magnitude id     	*/
	long	ampid;		/* amplitude id     	*/
	char	sta[7];		/* station identifier	*/
	long	arid;		/* arrival id     	*/
	long	orid;		/* origin id		*/
	long	evid;		/* event id		*/
	char	phase[9];	/* associated phase 	*/
	double	delta;		/* station-to-event dist*/
	char	magtype[7];	/* magnitude type (mb)	*/
	double	magnitude;	/* magnitude 		*/
	double	uncertainty;	/* magnitude uncertainty */
	double	magres;		/* magnitude residual   */
	char	magdef[2];	/* flag if mag is defining */
	char	mmodel[15];	/* magnitude model	*/
	char	auth[16];	/* author 		*/
	long	commid;		/* comment id		*/
	DateTime lddate;	/* load date		*/

	int	sta_quark;
} _CssStamag, *CssStamag;

/**
 *  Netmag structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member magid Magnitude id. Initial value = -1.
 *  @member net Network identifier. Initial value = "-".
 *  @member orid Origin id. Initial value = -1.
 *  @member evid Event id. Initial value = -1.
 *  @member magtype Magnitude type (mb). Initial value = "-".
 *  @member nsta Number of stations used. Initial value = -1.
 *  @member magnitude Magnitude. Initial value = -999.
 *  @member uncertainty Magnitude uncertainty. Initial value = -999.
 *  @member auth author. Initial value = "-".
 *  @member commid comment id. Initial value = -1.
 *  @member lddate load date. Initial value = "-".
 */
typedef struct CssNetmag_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	magid;		/* magnitude id     	*/
	char	net[9];		/* network identifier	*/
	long	orid;		/* origin id		*/
	long	evid;		/* event id		*/
	char	magtype[7];	/* magnitude type (mb)	*/
	long	nsta;		/* number of stations used */
	double	magnitude;	/* magnitude 		*/
	double	uncertainty;	/* magnitude uncertainty */
	char	auth[16];	/* author 		*/
	long	commid;		/* comment id		*/
	DateTime lddate;	/* load date		*/

	int	net_quark;
} _CssNetmag, *CssNetmag;

/**
 *  Ampdescript structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member amptype Amplitude measure descriptor. Initial value = "-".
 *  @member toff Offset from theoretical or observed arrival time. Initial value = -999.
 *  @member tlen Duration of measurement window. Initial value = -1.
 *  @member gvlo Low group velocity for measurement window (km/sec). Initial value = -999.
 *  @member gvhi High group velocity for measurement window (km/sec). Initial value = -999.
 *  @member mtype Measurement type. Initial value = "-".
 *  @member descr Description.
 *  @member lddate Load date.
 */
typedef struct CssAmpdescript_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	amptype[9];
	double	toff;
	double	tlen;
	double	gvlo;
	double	gvhi;
	char	mtype[9]; /* "peak", "stav", "rms", "peak2tr" or "1stpeak" */
	char	descr[256];
	DateTime lddate;
} _CssAmpdescript, *CssAmpdescript;

/* peak : maximum amplitude
 * stav : maximum short-term average amplitude
 * rms  : root-mean squared amplitude
 * 1stpeak : first motion amplitude
 */

/**
 *  Amplitude structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.
 *  @member ampid  Amplitude identifier. Every amplitude measure is assigned a unique positive integer that identifies it in the database. If an associated stamag record exists, then ampid links it to amplitude
 *  @member arid Arrival identifier. Each arrival is assigned a unique positive integer identifying it with a unique sta, chan and time.
 *  @member parid Every event-based parrival measure is assigned a unique positive integer that identifies it in the database. If an associated amplitude record exists, the parid links it to parrival.
 *  @member chan Channel identifier.
 *  @member amp Measured amplitude defined by amptype.
 *  @member per Measured period at the time of the amplitude measurement.
 *  @member snr Signal-to-noise ratio. This is an estimate of the ration of the amplitude of the signal to the amplitude of the noise immediately preceding it.
 *  @member amptime Epoch time of amplitude measure.
 *  @member start_time Epoch start time of the data interval.
 *  @member duration Total duration of amplitude window.
 *  @member bandw Frequency bandwidth.
 *  @member amptype Amplitude measure descriptor.
 *  @member units Units of amplitude measure.
 *  @member clip Clipped data flag. The value is a single-character flag to indicate whether (c) of not (n) the data were clipped.
 *  @member inarrival Flag to indicate whether of not amp is the same as it is in the arrival table.
 *  @member auth Author.
 *  @member lddate Load date.
 */

typedef struct CssAmplitude_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	ampid;		/* amplitude itentifier */
	long	arid;		/* arrival identifier */
	long	parid;		/* predicted arrival identifier */
	char	chan[9];	/* channel code */
	double	amp;		/* amplitude (nm) */
	double	per;		/* period (s) */
	double	snr;		/* signal-to-noise ratio */
	double	amptime;	/* time of amplitude measure */
	double	start_time;	/* start time of measurement window */
	double	duration;	/* duration of measurement window */
	double	bandw;		/* bandwidth */
	char	amptype[9];	/* amplitude measure descriptor */
	char	units[16];	/* units */
	char	clip[2];	/* clipping flag */
	char	inarrival[2];	/* "y" or "n" flag indicating if amp is the
				 * same as the amp in arrival table */
	char	auth[16];	/* author */
	DateTime lddate;	/* load date */

	int	chan_quark;
	double	amp_cnts;	/* measured amplitude */
	double	amp_Nnms;	/* measured amplitude (nominal) */
	double	amp_nms;	/* measured amplitude (corrected) */
	double	zp_Nnms;	/* zp measured amplitude (nominal) */
	bool	box_location;
	double	boxtime;	/* position (time) of the left side of the box*/
	double	boxmin;		/* position (counts) of the bottom side */
} _CssAmplitude, *CssAmplitude;

typedef struct CssStaconf_s
{
	GObjectPart	core;
	CssTablePart	css;

	char	staname[7];	/* station code */
	char	statype[9];	/* station type:  h-hydro, h-tphase, i-array,
				   s-3c, s-array */
	char	refsite[7];	/* reference station */
	char	refchan[9];	/* reference channel */
	char	threecsite[7];	/* three component site */
	char	threecband[9];	/* 3-c channel prefix: "B", "s", etc. */
	char	lpsite[7];	/* long period site */
	char	lpband[9];	/* long period channel prefix */
	DateTime lddate;	/* load date */

	int	staname_quark;
	int	statype_quark;
	int	refsite_quark;
	int	refchan_quark;
	int	threecsite_quark;
	int	threecband_quark;
	int	lpsite_quark;
	int	lpband_quark;
} _CssStaconf, *CssStaconf;

typedef struct CssParrival_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	parid;		/* predicted arrival identifier */
	long	orid;		/* origin identifier */
	long	evid;		/* event identifier */
	char	sta[7];		/* station code			*/
	double	time;		/* time of amplitude measure */
	double	azimuth;	/* observed azimuth		*/
	double	slow;		/* observed slowness (s/deg)	*/
	char	phase[9];	/* associated phase		*/
	double	delta;		/* station-to-event dist*/
	char	vmodel[16];	/* velocity model		*/
	DateTime lddate;	/* load date */

	int	sta_quark;
} _CssParrival, *CssParrival;

typedef struct CssDynamic5_s
{
	GObjectPart	core;
	CssTablePart	css;
	CssDescription	*des;

	union {
	    double	d_data;
	    double	f_data;
	    long	l_data;
	    int		i_data;
	} data[5];
} _CssDynamic5, *CssDynamic5;

typedef struct CssDynamic10_s
{
	GObjectPart	core;
	CssTablePart	css;
	CssDescription	*des;

	union {
	    double	d_data;
	    double	f_data;
	    long	l_data;
	    int		i_data;
	} data[10];
} _CssDynamic10, *CssDynamic10;

typedef struct CssDynamic20_s
{
	GObjectPart	core;
	CssTablePart	css;
	CssDescription	*des;

	union {
	    double	d_data;
	    double	f_data;
	    long	l_data;
	    int		i_data;
	} data[20];
} _CssDynamic20, *CssDynamic20;

typedef struct CssDynamic40_s
{
	GObjectPart	core;
	CssTablePart	css;
	CssDescription	*des;

	union {
	    double	d_data;
	    double	f_data;
	    long	l_data;
	    int		i_data;
	} data[40];
} _CssDynamic40, *CssDynamic40;

typedef struct CssDynamic_s
{
	GObjectPart	core;
	CssTablePart	css;
	CssDescription	*des;

	union {
	    double	d_data;
	    double	f_data;
	    long	l_data;
	    int		i_data;
	} data[60];
} _CssDynamic, *CssDynamic;




/*
 * CSS structs added by Teleca System Design for bg_analyze
 */



/**
 *
 * gards_bg_energy_cal structure.
 *
 */
typedef struct CssGards_bg_energy_cal_s
{
   GObjectPart   core;
   CssTablePart  css;

   long	     sample_id;
   double    beta_coeff1;
   double    beta_coeff2;
   double    beta_coeff3;
   double    gamma_coeff1;
   double    gamma_coeff2;
   double    gamma_coeff3;
   DateTime  moddate;
} _CssGards_bg_energy_cal, *CssGards_bg_energy_cal;

/**
 *
 * gards_bg_energy_cal_orig structure.
 *
 */
typedef struct CssGards_bg_energy_cal_orig_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   double    beta_coeff1;
   double    beta_coeff2;
   double    beta_coeff3;
   double    gamma_coeff1;
   double    gamma_coeff2;
   double    gamma_coeff3;
   DateTime  moddate;
} _CssGards_bg_energy_cal_orig, *CssGards_bg_energy_cal_orig;



/**
 *
 *  Gards_bg_proc_params structure.
 *
 */
typedef struct CssGards_bg_proc_params_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   double    lc_abscissa;
   long      method;
   long      det_bkgnd_used;
   long      gas_bkgnd_used;
   long      beta_ecr_order;
   long      gamma_ecr_order;
   long      max_qc_dev;
   long      qc_id;
   double    xe_in_air;
   long      det_bkgnd_id;
   long      gas_bkgnd_id;
   long      qc_b_threshold;
   long      bin_rows;
   long      bin_min_count;
   long      bin_gamma_start;
   long      bin_beta_start;
   long      bin_max_vector_size;
   DateTime  moddate;
} _CssGards_bg_proc_params, *CssGards_bg_proc_params;


/**
 *
 *  Gards_bg_proc_params_roi structure.
 *
 */
typedef struct CssGards_bg_proc_params_roi_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      roi;
   long      halflife_sec;
   double    abundance;
   long      nuclide_id;
   DateTime  moddate;
} _CssGards_bg_proc_params_roi, *CssGards_bg_proc_params_roi;


/*
 *
 *  gards_roi_channels structure.
 */
typedef struct CssGards_roi_channels_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      roi;
   long      b_chan_start;
   long      b_chan_stop;
   long      g_chan_start;
   long      g_chan_stop;
   DateTime  moddate;
} _CssGards_roi_channels, *CssGards_roi_channels;



/**
 *
 *  Gards_bg_roi_counts structure.
 */
typedef struct CssGards_bg_roi_counts_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      roi;
   double    gross;
   double    gross_err;
   double    gas_bkgnd_gross;
   double    gas_bkgnd_count;
   double    gas_bkgnd_count_err;
   double    det_bkgnd_count;
   double    det_bkgnd_count_err;
   double    net_count;
   double    net_count_err;
   double    critical_lev_samp;
   double    critical_lev_gas;
   DateTime  moddate;
} _CssGards_bg_roi_counts, *CssGards_bg_roi_counts;

/**
 *
 *  CssGards_bg_roi_concs structure.
 */
typedef struct CssGards_bg_roi_concs_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      roi;
   double    conc;
   double    conc_err;
   double    mdc;
   long      nid_flag;
   double    lc;
   double    ld;
   DateTime  moddate;
} _CssGards_bg_roi_concs, *CssGards_bg_roi_concs;


/**
 *
 *  CssGards_bg_isotope_concs structure.
 */
typedef struct CssGards_bg_isotope_concs_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      nuclide_id;
   double    conc;
   double    conc_err;
   double    mdc;
   long      nid_flag;
   double    lc;
   double    ld;
   DateTime  moddate;
} _CssGards_bg_isotope_concs, *CssGards_bg_isotope_concs;



/**
 *
 *  gards_sample_status structure.
 */
typedef struct CssGards_sample_status_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   DateTime  entry_date;
   DateTime  cnf_begin_date;
   DateTime  cnf_end_date;
   DateTime  review_date;
   long      review_time;
   char      analyst[30];
   char      status[1];
   long      category;
   long      auto_category;
   DateTime  release_date;
   DateTime  moddate;
} _CssGards_sample_status, *CssGards_sample_status;

/**
 *
 *  gards_sample_data structure.
 */
typedef struct CssGards_sample_data_s
{
   GObjectPart   core;
   CssTablePart  css;

   char      site_det_code[15];
   long      sample_id;
   long      station_id;
   long      detector_id;
   char      input_file_name[256];
   char      sample_type[1];
   char      data_type[1];
   char      geometry[17];
   char      spectral_qualifier[5];
   DateTime  transmit_dtg;
   DateTime  collect_start;
   DateTime  collect_stop;
   DateTime  acquisition_start;
   DateTime  acquisition_stop;
   double    acquisition_real_sec;
   double    acquisition_live_sec;
   double    quantity;
   DateTime  moddate;
} _CssGards_sample_data, *CssGards_sample_data;


/**
 *
 * gards_bg_std_spectra_result structure.
 *
 */
typedef struct CssGards_bg_std_spectra_result_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      std_spectra_id;
   double    beta_coeff1;
   double    beta_coeff2;
   double    beta_coeff3;
   double    gamma_coeff1;
   double    gamma_coeff2;
   double    gamma_coeff3;
   double    estimate;
   double    error;
   DateTime  moddate;
} _CssGards_bg_std_spectra_result, *CssGards_bg_std_spectra_result;

/**
 *
 * gards_bg_qc_result structure.
 *
 */
typedef struct CssGards_bg_qc_result_s
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   double    amplitude;
   double    fwhm;
   double    centroid;
   double    offset;
   double    slope;
   DateTime  moddate;
} _CssGards_bg_qc_result, *CssGards_bg_qc_result;

/**
 *
 * gards_roi_limits structure.
 *
 */
typedef struct
{
   GObjectPart   core;
   CssTablePart  css;

   long      sample_id;
   long      roi;
   double    b_energy_start;
   double    b_energy_stop;
   double    g_energy_start;
   double    g_energy_stop;
   DateTime  moddate;
} _CssGards_roi_limits, *CssGards_roi_limits;

/**
 *
 * gards_efficiency_pairs structure used by the noble gas gui
 *
 */
typedef struct 
{
   GObjectPart   core;
   CssTablePart  css;
   
   long      sample_id;
   double    effic_energy;
   double    efficiency;
   double    effic_error;
} _CssGards_efficiency_pairs, *CssGards_efficiency_pairs;


/**
 *  Qcdata structure.
 *  @member core GobjectCorePart.
 *  @member css CssTablePart.

 *  @member stime Epoch time of cell start time. Initial value = -9999999999.999.
 *  @member etime Epoch time of cell end time. Initial value = -9999999999.999.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member sensor Sensor. Initial value = "-".
 *  @member dtype data type. Initial value = "-1".
 *  @member dstat data status. Initial value = "-1".
 *  @member ldtime Epoch time cell was loaded. Initial value = -9999999999.999.
 */
typedef struct CssQcdata_s
{
	GObjectPart	core;
	CssTablePart	css;

	long	stime;		/* epoch time of cell start time        */
	long	etime;		/* epoch time of cell end time    	*/
	char	sta[7];		/* station 				*/
	char	chan[9];	/* channel 				*/
	char	sens[21];	/* sensor 				*/
	long	dtype;		/* data type of cell                    */
	long	dstat;		/* status of cell data                  */
	long	ldtime;		/* epoch load time of row into db       */
} _CssQcdata, *CssQcdata;


/**
 *  Qcmaskdef structure.
 */
typedef struct CssQcmaskdef_s
{
	GObjectPart	core;
	CssTablePart	css;

        long qcdefid;
        long perform_qc;
        double max_mask_fraction;
        char type_multi_component[16];
        long min_multi_component;
        long apply_extended;
        long demean;
        long fix;
        long prefilter;
        double fhigh;
        long gap_samples;
        double gap_taper_fraction;
        long interval_samples;
        double interval_overlap_fraction;
        long niter;
        double single_trace_spike_thresh;
        double spike_thresh;
        char spike_statistic[16];
        double spike_statistic_value;
        char spike_dataset[16];
        long spike_window_samples;
        long ondate;
        long offdate;
        char auth[32];
        DateTime lddate;
} _CssQcmaskdef, *CssQcmaskdef;



/**
 *  Qcmaskinfo structure.
 */
typedef struct CssQcmaskinfo_s
{
	GObjectPart	core;
	CssTablePart	css;

        long   qcmaskid;
        char   sta[7];
        char   chan[9];
        double time;
        double endtime;
        double samprate;
        long   nseg;
        long   qcdefid;
        char   auth[32];
        DateTime lddate;
} _CssQcmaskinfo, *CssQcmaskinfo;

/**
 *  Qcmaskseg structure.
 */
typedef struct CssQcmaskseg_s
{
	GObjectPart	core;
	CssTablePart	css;

        long   qcmaskid;
        long   startsample;
        long   endsample;
        char   masktype[32];
        char   auth[32];
        DateTime lddate;
} _CssQcmaskseg, *CssQcmaskseg;


/* Start bg_analyze*/
CssGards_bg_energy_cal new_CssGards_bg_energy_cal(void);
CssGards_bg_energy_cal_orig new_CssGards_bg_energy_cal_orig(void);
CssGards_bg_proc_params new_CssGards_bg_proc_params(void);
CssGards_bg_proc_params_roi new_CssGards_bg_proc_params_roi(void);
CssGards_roi_channels new_CssGards_roi_channels(void);
CssGards_bg_roi_concs new_CssGards_bg_roi_concs(void);
CssGards_bg_roi_counts new_CssGards_bg_roi_counts(void);
CssGards_bg_isotope_concs new_CssGards_bg_isotope_concs(void);
CssGards_sample_status new_CssGards_sample_status(void);
CssGards_sample_data new_CssGards_sample_data(void);
CssGards_bg_std_spectra_result new_CssGards_bg_std_spectra_result(void);
CssGards_bg_qc_result new_CssGards_bg_qc_result(void);
CssGards_roi_limits new_CssGards_roi_limits(void);
/* End bg_analyze */


/* Start noble gas gui */
CssGards_efficiency_pairs new_CssGards_efficiency_pairs(void);
/* End noble gas gui */


CssArrival new_CssArrival(void);
CssQcdata  new_CssQcdata(void);
CssQcmaskdef new_CssQcmaskdef(void);
CssQcmaskinfo new_CssQcmaskinfo(void);
CssQcmaskseg new_CssQcmaskseg(void);
CssWfdisc new_CssWfdisc(void);
CssHistory new_CssHistory(void);
CssChanname new_CssChanname(void);
CssFrameproduct new_CssFrameproduct(void);
CssFpdescription new_CssFpdescription(void);
CssMd5_digest new_CssMd5_digest(void);
CssClf new_CssClf(void);
CssWaveinterval new_CssWaveinterval(void);
CssOutage new_CssOutage(void);
CssOrigin new_CssOrigin(void);
CssOrigerr new_CssOrigerr(void);
CssOrigaux new_CssOrigaux(void);
CssLastid new_CssLastid(void);
CssSensor new_CssSensor(void);
CssInstrument new_CssInstrument(void);
CssSitechan new_CssSitechan(void);
CssSite new_CssSite(void);
CssWftag new_CssWftag(void);
CssXtag new_CssXtag(void);
CssFsdisc new_CssFsdisc(void);
CssFsave new_CssFsave(void);
CssFsrecipe new_CssFsrecipe(void);
CssFstag new_CssFstag(void);
CssSpdisc new_CssSpdisc(void);
CssDervdisc new_CssDervdisc(void);
CssPmccRecipe new_CssPmccRecipe(void);
CssPick new_CssPick(void);
CssFilter new_CssFilter(void);
CssAssoc new_CssAssoc(void);
CssStassoc new_CssStassoc(void);
CssAffiliation new_CssAffiliation(void);
CssStanet new_CssStanet(void);
CssHydroFeatures new_CssHydroFeatures(void);
CssInfraFeatures new_CssInfraFeatures(void);
CssStamag new_CssStamag(void);
CssNetmag new_CssNetmag(void);
CssAmpdescript new_CssAmpdescript(void);
CssAmplitude new_CssAmplitude(void);
CssPmccFeatures new_CssPmccFeatures(void);
CssStaconf new_CssStaconf(void);
CssParrival new_CssParrival(void);
CssDynamic5 new_CssDynamic5(CssDescription *des);
CssDynamic10 new_CssDynamic10(CssDescription *des);
CssDynamic20 new_CssDynamic20(CssDescription *des);
CssDynamic40 new_CssDynamic40(CssDescription *des);
CssDynamic new_CssDynamic(CssDescription *des);


/* Start bg_analyze */
int new_CssGards_bg_energy_cals(int num, CssGards_bg_energy_cal *tables);
int new_CssGards_bg_proc_paramss(int num, CssGards_bg_proc_params *tables);
int new_CssGards_bg_proc_params_rois(int num, CssGards_bg_proc_params_roi *tables);
int new_CssGards_roi_channelss(int num, CssGards_roi_channels *tables);
int new_CssGards_bg_roi_concss(int num, CssGards_bg_roi_concs *tables);
int new_CssGards_bg_roi_countss(int num, CssGards_bg_roi_counts *tables);
int new_CssGards_bg_isotope_concss(int num, CssGards_bg_isotope_concs *tables);
int new_CssGards_sample_statuss(int num, CssGards_sample_status *tables);
int new_CssGards_sample_datas(int num, CssGards_sample_data *tables);
int new_CssGards_bg_std_spectra_results(int num, CssGards_bg_std_spectra_result *tables);
int new_CssGards_bg_qc_results(int num, CssGards_bg_qc_result *tables);
int new_CssGards_roi_limitss(int num, CssGards_roi_limits *tables);
/* End bg_analyze */


/* Start noble gas gui */
int new_CssGards_efficiency_pairss(int num, CssGards_efficiency_pairs *tables);
/* End noble gas gui */


int new_CssArrivals(int num, CssArrival *tables);
int new_CssQcdatas(int num, CssQcdata *tables);
int new_CssQcmaskdefs(int num, CssQcmaskdef *tables);
int new_CssQcmaskinfos(int num, CssQcmaskinfo *tables);
int new_CssQcmasksegs(int num, CssQcmaskseg *tables);
int new_CssWfdiscs(int num, CssWfdisc *tables);
int new_CssHistorys(int num, CssHistory *tables);
int new_CssChannames(int num, CssChanname *tables);
int new_CssFrameproducts (int num, CssFrameproduct *tables);
int new_CssFpdescriptions (int num, CssFpdescription *tables);
int new_CssMd5_digests(int num, CssMd5_digest *tables);
int new_CssClfs(int num, CssClf *tables);
int new_CssWaveintervals(int num, CssWaveinterval *tables);
int new_CssOutages(int num, CssOutage *tables);
int new_CssOrigins(int num, CssOrigin *tables);
int new_CssOrigerrs(int num, CssOrigerr *tables);
int new_CssOrigauxs(int num, CssOrigaux *tables);
int new_CssLastids(int num, CssLastid *tables);
int new_CssSensors(int num, CssSensor *tables);
int new_CssInstruments(int num, CssInstrument *tables);
int new_CssSitechans(int num, CssSitechan *tables);
int new_CssSites(int num, CssSite *tables);
int new_CssWftags(int num, CssWftag *tables);
int new_CssXtags(int num, CssXtag *tables);
int new_CssFsdiscs(int num, CssFsdisc *tables);
int new_CssFsaves(int num, CssFsave *tables);
int new_CssFsrecipes(int num, CssFsrecipe *tables);
int new_CssFstags(int num, CssFstag *tables);
int new_CssSpdiscs(int num, CssSpdisc *tables);
int new_CssDervdiscs(int num, CssDervdisc *tables);
int new_CssPmccRecipes(int num, CssPmccRecipe *tables);
int new_CssPmccFs(int num, CssPmccFeatures *tables);
int new_CssPicks(int num, CssPick *tables);
int new_CssFilters(int num, CssFilter *tables);
int new_CssAssocs(int num, CssAssoc *tables);
int new_CssGregions(int num, CssGregion *tables); 
int new_CssStassocs(int num, CssStassoc *tables);
int new_CssAffiliations(int num, CssAffiliation *tables);
int new_CssStanets(int num, CssStanet *tables);
int new_CssHydros(int num, CssHydroFeatures *tables);
int new_CssInfras(int num, CssInfraFeatures *tables);
int new_CssStamags(int num, CssStamag *tables);
int new_CssNetmags(int num, CssNetmag *tables);
int new_CssAmpdescripts(int num, CssAmpdescript *tables);
int new_CssAmplitudes(int num, CssAmplitude *tables);
int new_CssStaconfs(int num, CssStaconf *tables);
int new_CssParrivals(int num, CssParrival *tables);
int new_CssDynamic5s(CssDescription *des, int num, CssDynamic5 *dynamic5s);
int new_CssDynamic10s(CssDescription *des, int num,CssDynamic10 *dynamic10s);
int new_CssDynamic20s(CssDescription *des, int num,CssDynamic20 *dynamic20s);
int new_CssDynamic40s(CssDescription *des, int num,CssDynamic40 *dynamic40s);
int new_CssDynamics(CssDescription *des, int num, CssDynamic *dynamics);

#endif
