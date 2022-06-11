/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_GCSS_H_
#define	_LIB_GCSS_H_

#include "libgnet.h"
#include "gobject/cssObjects.h"
#include "Attribute.h"
#include "gobject/TimeSeries.h"

#define CHANGE_TIME		1
#define CHANGE_AMP_PER		2
#define CHANGE_PHASE_NAME	4
#define CHANGE_STASSID		8
#define CHANGE_AZIMUTH		16
#define CHANGE_SLOW		32

typedef struct
{
	int	num_params;
	char	**names;
	char	**values;
} ParamStruct;

typedef struct
{
	int	sta;
	int	refsta;
	float	lat;
	float	lon;
       	float	dnorth;
	float	deast;
	float	elev;
} Sites;

typedef struct
{
	int	sta;
	int	chan;
	long	chanid;
	int	inid;
	double	time;
	double	endtime;
} Se;

typedef struct
{
	int	inid;
	char	insname[51];
	char	dfile[33];
	char	dir[65];
} In;

/* ****** css/ParseMsg.c ********/
/*
int parse_db_waveforms(char *msg_data, char *tmp_file, bool append);
void parse_disk_waveforms(char *msg_data, char *tmp_file, bool append);
void parse_arrival_msg(char *msg_data, char *tmp_file, bool append);
void parse_assoc_msg(char *msg_data, char *tmp_file, bool append);
void parse_origin_msg(char *msg_data, char *tmp_file, bool append);
void parse_origerr_msg(char *msg_data, char *tmp_file, bool append);
void parse_wftag_msg(char *msg_data, char *tmp_file, bool append);
*/


/* ****** libgcss/css_input.c ********/
int cssReadData(int nsegs, WaveformList **wav_input, double start_time,
			double end_time, int pts, bool preview_arr,
			TimeSeries *ts, Vector arrivals,
			Vector origins, Vector origerrs, Vector assocs,
			Vector stassocs, Vector wftags, Vector hydro_features,
			Vector infra_features, Vector stamags, Vector netmags,
			Vector amplitudes, Vector ampdescripts, Vector parrivals,
			const char **err_msg);
int cssListData(const char *read_path, int format, int max_num,
			WaveformList ***pwavlist);
int cssRereadData(TimeSeries ts, const char **err_msg);
int cssArrivalAdd(CssArrival arrival, TimeSeries ts, Password password);
int cssArrivalChange(CssArrival arrival, TimeSeries ts, int mask);
int cssArrivalDelete(int n_arrivals, CssArrival *arrivals,
			int iarr, int n_assocs, CssAssoc *assocs);
int cssUndoArrivalDelete(CssArrival arrival, int n_arrivals,
			CssArrival *arrivals, int n_assocs, CssAssoc *assocs);
int cssTableList(int num_tables, CssTable *tables, int num_extra,
			CssTable *extra, int num_attributes,
			Attribute **attributes, char **row,
			const char *rw, bool reopen);
int cssTableChange(CssTable old_table, CssTable new_table);
int cssTableDelete(int n_tables, CssTable *tables, int i);
CssTable cssTableCreate(CssTable table, const char *table_name,
			const char *id_name, Password password);

int cssAmplitudeAdd(CssArrival arrival, CssAmplitude amplitude);
int cssAssocAdd(CssAssoc assoc);
CssOrigerr cssOrigerrAdd(CssOrigin origin);
int cssOriginCreate(CssOrigin origin, CssOrigerr origerr, CssTable table,
			Password password);
int cssOriginDelete(int n_origins, CssOrigin *origins, int iorg,
			int n_origerrs, CssOrigerr *origerrs, int n_netmags,
			CssNetmag *netmags);
int cssUndoOriginDelete(CssOrigin, int n_origins, CssOrigin *origins,
			int n_origerrs, CssOrigerr *origerrs, int n_netmags,
			CssNetmag *netmags, int n_wftags, CssWftag *wftags,
			int n_assocs, CssAssoc *assocs, int n_stamags,
			CssStamag *stamags);
int cssSegmentList(CPlotData *cd, GObject s, CssOrigin origin,
			int num_attributes, Attribute **attributes,
			char **row, const char *rw, bool reopen);
int cssAmpPerList(TimeSeries ts, CPlotMeasurement *m, int num_attributes,
			Attribute **attributes, char **row);
int cssWriteData(CssTable t, const char *wfdisc_file, const char *wa,
			const char *dotw, const char *da, float *y,
			const char *sta, const char *chan, double time,
			int chanid, int nsamp, double samprate, double calib,
			double calper, const char *remark, int wfid);
int cssStamagAdd(CssStamag stamag, CssArrival arrival,
			CssOrigin origin, Password password);
int cssNetmagAdd(CssNetmag netmat, CssOrigin origin, Password password);

int send_arrivals(void);
int cssWriteCD(const char *prefix, const char *wa, int num_waveforms,
		CPlotData **cd_list, const char *remark, int raw);
double getCalib(TimeSeries ts, Segment s);
double getCalper(TimeSeries ts, Segment s);
int startBackup(void);
void cssCloseFile(const char *table_name);
char *css_get_tok(CssTable tables, char *name, char *spec);
void css_put_tok(CssTable table, char *name, char *spec, char *cell);
char *css_get_extra_tok(int num_extra, CssTable *extra, char *name,
			char *spec);
void get_pref_amp(char *sta, char *chan, double amp_Nnms,
                        double amp_nms, double *amp);
bool cssCalibApplied(TimeSeries ts, double *calib);
bool cssSavePick(CssArrival arrival, int mask, int arid, const char *sta,
			const char *chan, WfdiscIO wfdisc_io, DateTime *date,
			FILE *fp, const char *backup_file);
FILE *openBackupFile(void);


/* ****** libgcss/load_tables.c ********/
void LoadAllTables(TimeSeries ts, const char *dir, const char *prefix,
	WaveformList *wav, Vector arrivals, Vector origins, Vector origerrs,
	Vector assocs, Vector stassocs, Vector wftags, Vector hydro_features,
	Vector infra_features, Vector stamags, Vector netmags,
	Vector amplitudes, Vector ampdescripts, Vector parrivals);


/* ****** libgcss/get_id.c ********/
int cssGetId(CssTable css, const char *keyname, const char *prefix);
int getCssLastid(CssTable css, const char *keyname, const char *prefix);


/* ****** libgcss/undo.c ********/
int undo_file_modification(const char *backup_file, ParamStruct *par);


/* ******* libgcss/get_hangvang.c ********/
int get_hangvang(CssTable s);


#endif /* _LIB_GCSS_H_ */
