#ifndef _LIB_GNET_H_
#define	_LIB_GNET_H_

#include "gobject/cssObjects.h"
#include "css/wfdisc.h"
#include "gobject/TimeSeries.h"
#include "Attribute.h"

#ifndef NULL_TIME
#define NULL_TIME	-9999999999.999
#endif

typedef struct
{
	int		arid;
	int		orid;
	double		time;
	char		net[10];
	char		chan[20];
	char		phase[9];
} Arr;

typedef struct
{
	int		path;
	int		id;
	int		format;
	int		file_order;
	int		read_order;
	char		sta[10];
	char		chan[20];
	char		net[10];
	char		refsta[10];
	double		start;
	double		end;
	double		input_start;
	double		input_end;
	int		nsamp;
	double		samprate;
	int		jdate;
	int		nass;
	int		ndef;
	double		station_lat;
	double		station_lon;
	double		dnorth;
	double		deast;
	bool 		selected;

	int		narr;
	Arr		*arr;

	int		origin_id;
	double		origin_time;
	double		origin_lat;
	double		origin_lon;
	double		origin_depth;
	double		origin_delta;
	double		origin_azimuth;

	GObject		io;
	GObject		elements;
} WaveformList;

#define WAVEFORM_LIST_NULL \
{ \
	-1,		/* path */ \
	-1,		/* id */ \
	-1,		/* format */ \
	-1,		/* file_order */ \
	-1,		/* read_order */ \
	"-",		/* sta */ \
	"-",		/* chan */ \
	"-",		/* net */ \
	"-",		/* refsta */ \
	NULL_TIME,	/* start */ \
	NULL_TIME,	/* end */ \
	0.,		/* input_start */ \
	0.,		/* input_end */ \
	0,		/* nsamp */ \
	-1.,		/* samprate */ \
	-1,		/* jdate */ \
	0,		/* nass */ \
	0,		/* ndef */ \
	-999.,		/* station_lat */ \
	-999.,		/* station_lon */ \
	100001.,	/* dnorth */ \
	100001.,	/* deast */ \
	False,		/* selected */ \
 \
	0,		/* narr */ \
	(Arr *) NULL,	/* arr */ \
 \
	-1,		/* origin_id */ \
	NULL_TIME,	/* origin_time */ \
	-999.,		/* origin_lat */ \
	-999.,		/* origin_lon */ \
	-999.,		/* origin_depth */ \
	-1.,		/* origin_delta */ \
	-1.,		/* origin_azimuth */ \
 \
	NULL,		/* wav_io */ \
	NULL,		/* elements */ \
}

typedef struct
{
	int	sta;
	int	net;
	int	refsta;
	int	*chan;
	int	nchan;
	double	lat;
	double	lon;
} GSta;

enum MemFileType {
	GLOBAL  = 0,
	LOCAL   = 1
};

/**
 * @private
 */
typedef struct
{
	enum MemFileType	type;
	char			*name;
	char			*path;
	struct stat		stat;
	Vector			records;
} MemTable;


typedef struct passwd * Password;

typedef struct
{
	char		label;
	DataPoint	d1;
	DataPoint	d2;
} DataWindow;


#define MAX_COMPONENTS  3

/** 
 *  This structure is a reference to a waveform in a CPlotWidget or a
 *  TtPlotWidget.  Several CPlotWidget and TtPlotWdiget functions return
 *  CPlotData pointers. The CPlotData pointers are also used to specify
 *  waveforms in other CPlotWidget and TtPlotWidget functions. The members of
 *  the CPlotData structure should not be altered outside of the CPlotWidget
 *  or TtPlotWidget.
 *  @see CPlotAddTimeSeries
 *  @see TtPlotAddTimeSeries
 *  @see CPlotGetData
 *  @see CPlotGetSelected
 *  @see TtPlotGetData
 *  @see TtPlotGetSelected
 *  
 */
typedef struct cplot_data
{
	int		id;		/* Identifier for each waveform */
	void		*w;		/* CPlotWidget holding this data */
	TimeSeries	ts;		/* time series structure */
	char		sta[7];		/* station name */
	char		chan[9];	/* channel name */
	char		net[9];		/* network name */
	double		lat;		/* station latitude */
	double		lon;		/* station longitude */
	bool		visible;	/* waveform is currently visible */
	bool		selected;	/* waveform is selected */
	double		scaled_x0;	/* current x position of the waveform */
	double		scaled_y0;	/* current y position of the waveform */
	int		fg;		/* color */
	int		num_dp;		/* num of DataPoint structures */
	DataPoint	*dp;		/* DataPoint structures */
	int		num_dw;		/* num of DataWindow structures */
	DataWindow	*dw;		/* DataWindow structures */
	double		begSelect;	/* partial waveform selection */
	double		endSelect;
	struct cplot_data *c[MAX_COMPONENTS];	/* all component pointers */
} CPlotData;

#define CPLOT_DATA_NULL \
{ \
	0,			/* id */ \
	NULL,			/* w */ \
	(TimeSeries)NULL,	/* ts */ \
	"", "", "",		/* sta, chan, net */ \
	-999., -999.,		/* lat, lon */ \
	True,			/* visible */ \
	False,			/* selected */ \
	0., 0.,			/* scaled_x0, scaled_y0 */ \
	0,			/* color */ \
	0,			/* num_dp */ \
	(DataPoint *)NULL,	/* dp */ \
	0,			/* num_dw */ \
	(DataWindow *)NULL,	/* dw */ \
	0.,0., 			/* begSelect, endSelect */ \
	{ \
		(CPlotData *)NULL,	/* c[0] */ \
		(CPlotData *)NULL,	/* c[1] */ \
		(CPlotData *)NULL	/* c[2] */ \
	}, \
}

/** 
 *  A CPlotData structure pointer.
 */
typedef CPlotData *Waveform;

/** 
 *  Information about a waveform amplitude and period measurement.
 *  @see CPlotGetArrivalMeasureBox
 */
typedef struct
{
	CPlotData *cd;		/* CPlotData structure for measured waveform */
	double  amp_cnts;	/* measured amplitude */
	double  amp_Nnms;	/* measured amplitude (nominal) */
	double  amp_nms;	/* measured amplitude (corrected) */
	double  zp_Nnms;	/* zp measured amplitude (nominal) */
	double  period;		/* measured period */
	double	left_side;	/* position (time) of the left side of the box*/
	double	bottom_side;	/* position (amp) of the bottom side */
} CPlotMeasurement;


typedef int (*FormatSpecific)(const char *lib, void *handle, const char *format,
                                void *callback);

typedef int (*AmpPerListFunc)(TimeSeries ts, CPlotMeasurement *m,
			int num_attributes, Attribute **attributes,
			char **row);
typedef int (*ArrivalAddFunc)(CssArrival arrival, TimeSeries ts,
			Password password);
typedef int (*ArrivalChangeFunc)(CssArrival arrival, TimeSeries ts, int mask);
typedef int (*ArrivalDeleteFunc)(int n_arrivals, CssArrival *arrivals, int iarr,
			int n_assocs, CssAssoc *assocs);
typedef int (*UndoArrivalDeleteFunc)(CssArrival arrival, int n_arrivals,
			CssArrival *arrivals, int n_assocs, CssAssoc *assocs);
typedef CssTable (*TableCreateFunc)(CssTable table, const char *table_name,
			const char *id_name, Password password);
typedef int (*TableListFunc)(int num_tables, CssTable *tables,
			int num_extra, CssTable *extra, int num_attributes,
			Attribute **attribute, char **row,
			const char *rw, bool reopen);
typedef int (*TableChangeFunc)(CssTable old_table,CssTable new_table);
typedef int (*TableDeleteFunc)(int n_tables, CssTable *tables, int i);
typedef int (*AmplitudeAddFunc)(CssArrival arrival, CssAmplitude amplitude);
typedef int (*AssocAddFunc)(CssAssoc assoc);
typedef int (*FormatInitFunc)(void *w, const char *global_dir,
			int *num_global_tables, char ***global_table_names,
			char ***global_table_paths);
typedef int (*ListDataFunc)(const char *read_path, int format,
			int max_num, WaveformList ***pwavlist);
typedef CssOrigerr (*OrigerrAddFunc)(CssOrigin origin);
typedef int (*OriginCreateFunc)(CssOrigin origin,
			CssOrigerr origerr, CssTable table, Password password);
typedef int (*OriginDeleteFunc)(int n_origins,CssOrigin *origins,
			int iorg, int n_origerrs, CssOrigerr *origerrs,
			int n_netmags, CssNetmag *netmags);
typedef int (*UndoOriginDeleteFunc)(CssOrigin origin, int n_origins,
			CssOrigin *origins, int n_origerrs,CssOrigerr *origerrs,
			int n_netmags, CssNetmag *netmags, int n_wftags,
			CssWftag *wftags, int n_assocs, CssAssoc *assocs,
			int n_stamags, CssStamag *stamags);
typedef int (*ReadDataFunc)(int nsegs, WaveformList **wav_input,
			double start_time, double end_time, int pts,
			bool preview_arr, TimeSeries *ts,
			Vector arrivals, Vector origins, Vector origerrs,
			Vector assocs, Vector stassocs, Vector wftags,
			Vector hydro_features, Vector infra_features,
			Vector stamags, Vector netmags, Vector amplitudes,
			Vector ampdescripts, Vector parrivals, const char **err_msg);
typedef int (*RereadDataFunc)(TimeSeries ts, const char **err_msg);
typedef int (*ResponseFunc)(TimeSeries ts, char *insname, int insname_len,
			char *file, int file_len, const char **err_msg);
typedef int (*SetPathFunc)(const char *name, const char *path);
typedef CssAmplitude (*AmplitudeCreateFunc)(CssArrival arrival,Password passwd);
typedef int (*StamagAddFunc)(CssStamag stamag,CssArrival arrival,
			CssOrigin origin, Password password);
typedef int (*NetmagAddFunc)(CssNetmag netmag, CssOrigin origin,
			Password password);
typedef int (*SegmentListFunc)(CPlotData *cd, GObject s, CssOrigin origin,
			int num_attributes, Attribute **attributes,
			char **row, const char *rw,
			bool reopen);

typedef struct
{
	int				quark;
	int				num_tables;
	char				**table_names;
	char				**table_paths;
	TableCreateFunc			TableCreate;
	TableListFunc			TableList;
	TableChangeFunc			TableChange;
	TableDeleteFunc			TableDelete;
	AmpPerListFunc			AmpPerList;
	ArrivalAddFunc			ArrivalAdd;
	ArrivalChangeFunc		ArrivalChange;
	ArrivalDeleteFunc		ArrivalDelete;
	UndoArrivalDeleteFunc		UndoArrivalDelete;
	AmplitudeAddFunc		AmplitudeAdd;
	AssocAddFunc			AssocAdd;
	FormatInitFunc			FormatInit;
	ListDataFunc			ListData;
	OrigerrAddFunc			OrigerrAdd;
	OriginCreateFunc		OriginCreate;
	OriginDeleteFunc		OriginDelete;
	UndoOriginDeleteFunc		UndoOriginDelete;
	ReadDataFunc			ReadData;
	RereadDataFunc			RereadData;
	ResponseFunc			Response;
	SetPathFunc			SetPath;
	AmplitudeCreateFunc		AmplitudeCreate;
	StamagAddFunc			StamagAdd;
	NetmagAddFunc			NetmagAdd;
	SegmentListFunc			SegmentList;
} FormatStruct;

#define FORMAT_STRUCT_NULL \
{ \
	(int)0,  \
	0, (char **)NULL, (char **)NULL, \
	(TableCreateFunc) NULL,\
	(TableListFunc) NULL,\
	(TableChangeFunc) NULL,\
	(TableDeleteFunc) NULL,\
	(AmpPerListFunc) NULL,\
	(ArrivalAddFunc) NULL,\
	(ArrivalChangeFunc) NULL,\
	(ArrivalDeleteFunc) NULL,\
	(UndoArrivalDeleteFunc) NULL,\
	(AmplitudeAddFunc) NULL,\
	(AssocAddFunc) NULL,\
	(FormatInitFunc) NULL,\
	(ListDataFunc) NULL,\
	(OrigerrAddFunc) NULL,\
	(OriginCreateFunc) NULL,\
	(OriginDeleteFunc) NULL,\
	(UndoOriginDeleteFunc) NULL,\
	(ReadDataFunc) NULL,\
	(RereadDataFunc) NULL,\
	(ResponseFunc) NULL,\
	(SetPathFunc) NULL,\
	(AmplitudeCreateFunc) NULL,\
	(StamagAddFunc) NULL,\
	(NetmagAddFunc) NULL,\
	(SegmentListFunc) NULL,\
}

/* ****** net_tables.c ********/
MemTable *gnetGetMemTable(const char *cssTableName, enum MemFileType type);
int gnetReadGlobalTable(const char *cssTableName, Vector v);
void gnetResetLoaded(void);
int gnetSetGlobalTable(const char *cssTableName, const char *path);
char *gnetGetGlobalTable(const char *cssTableName);
void gnetReadOrigin(const char *full_prefix, int num_wavs,
		WaveformList **wavlist);
void gnetReadAssoc(const char *full_prefix, int num_wavs,
		WaveformList **wavlist);
void gnetReadArrival(const char *full_prefix, int num_wavs,
		WaveformList **wavlist);
void gnetChangeStaChan(char *sta, int sta_size, char *chan, int chan_size);
void gnetLoadArrivals(TimeSeries ts, const char *net, Vector global_arrival,
		Vector local_arrival, Vector arrivals);
void gnetReadAssocs(const char *file, Vector arrivals, Vector assocs);
void gnetReadStassocs(const char *file, Vector arrivals, Vector stassocs);
void gnetReadHydroFeatures(const char *file, Vector arrivals,
		Vector hydro_features);
void gnetReadInfraFeatures(const char *file, Vector arrivals,
		Vector infra_features);
void gnetReadAmplitudes(const char *file, TimeSeries ts, Vector arrivals,
		Vector amplitudes, Vector ampdescripts);
void gnetReadStamags(const char *file, Vector arrivals, Vector stamags);
void gnetReadOridWftags(const char *file, TimeSeries ts, Vector wftags);
void gnetReadOrigins(const char *full_prefix, TimeSeries ts, const char *net,
		Vector assocs, Vector origins);
void gnetReadOrigerrs(const char *file, Vector origins, Vector origerrs);
void gnetReadNetmags(const char *file, Vector origins, Vector netmags);
void gnetReadPicks(int pick_file, Vector arrivals);
void gnetReadFilters(int filter_file, Vector arrivals);
int gnetReadArrivalTables(const char *path, struct stat *buf, Vector v);


/* ****** read_wf_data.c ********/

Segment gnetReadSegment(WFDISC30 *wfd, const char *working_dir,
		double start_time, double end_time, int pts_wanted);

/* ******* stations.c ********/
void gnetAddStations(int num_selected, WaveformList **wav);
int gnetGetStations(GSta **sta);

/* ****** net.c ********/
bool gnetReadStaTables(const char *prefix, const char **err_msg);
void gnetSetAffiliations(Vector v);
void gnetSetSites(Vector v);
void gnetSetSitechans(Vector v);
void gnetSetStaconfs(Vector v);
CssSite gnetGetSite(const char *sta, int jdate);
CssStaconf gnetGetStaconf(const char *sta);
const char *gnetGetNet(const char *sta);
bool gnetAffiliated(const char *sta1, const char *sta2);
int gnetNetworkStations(const char *net, const char ***elements);
void gnetVerifyChanid(WFDISC30 *wfdisc);
int gnetGetChannels(const char *sta, const char ***channels);
const char *gnetGetLikeNet(const char *sta);
CssSitechan gnetGetSitechan(int chanid, long jdate);
int gnetGetHang(char *chan, int chanid, long jdate, float *hang, float *vang);
void gnetGetWaveSites(int num_wavs, WaveformList **wavlist);
void gnetSaveRefSta(int num, const char **refsta, const char *sitefile);
Vector gnetGetGlobalSitechans(void);
Vector gnetGetSites(void);



#endif /* _LIB_GNET_H_ */
