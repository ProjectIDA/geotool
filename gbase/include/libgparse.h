#ifndef _LIB_GPARSE_H_
#define	_LIB_GPARSE_H_

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "gobject/cssObjects.h"
#include "gobject/TimeSeries.h"
#include "libgnet.h"
#include "libFK.h"
#include "libFT2.h"

typedef struct GParseEnv_struct *GParseEnv;

typedef struct
{
	GObjectPart	core;
	CPlotData	cd;
	char		color_name[32];
	char		tag[200];
} _PSWaveform, *PSWaveform;

#define GPARSE_SOURCE_NONE	0
#define GPARSE_SOURCE_PREFIX	1
#define GPARSE_SOURCE_FFDB	2
#define GPARSE_SOURCE_ODBC	3


GParseEnv GParseInitialize(int argc, char **argv);
void GParseDestroy(GParseEnv gpe);
void GParseClose(GParseEnv gpe);
int GParseCmd(GParseEnv gpe, const char *cmd, char *prompt, int len_prompt);
char *GParseErrMsg(void);
int GParseErrno(void);
void GParseDrawWaveforms(GParseEnv gpe);
void GParseMTinversion(GParseEnv gpe);
void GParseStaNoise(GParseEnv gpe);
void GParseQCMask(GParseEnv gpe);
void GParseEvQuality(GParseEnv gpe);
void GParseStandardSensor(GParseEnv gpe);
Vector GParseGetTable(GParseEnv gpe, char *cssTableName);
int iaspeiOpen(GParseEnv gpe);
int iaspeiPhase(GParseEnv gpe, const char *phase_name, float delta, float depth,
	float *_tt, float *_ray_p, float *_dtdd, float *_dtdh, float *_dddp);
char *GParseGetString(GParseEnv gpe, const char *name);
bool GParseGetTime(GParseEnv gpe, const char *name, double *time);
bool GParseFillString(GParseEnv gpe, const char *name, char *value, int length);
bool GParseGetInt(GParseEnv gpe, const char *name, int *i);
bool GParseGetDouble(GParseEnv gpe, const char *name, double *d);
bool GParseGetBool(GParseEnv gpe, const char *name, bool *b);
bool GParseImportTables(GParseEnv gpe, const char *line, bool new_tables);
bool GParseImportFile(GParseEnv gpe, const char *filename);
bool GParseStartImport(GParseEnv gpe, const char *line);
bool GParseImportLine(GParseEnv gpe, const char *line);
void GParseBeam(GParseEnv gpe, const char *recipe_name);
void GParseFK(GParseEnv gpe, bool fk_signal);
void GParseFT(GParseEnv gpe);
void GParseWriteFK(GParseEnv gpe);
void GParseWriteFT(GParseEnv gpe);
void GParseReadFT(GParseEnv gpe);
void GParseCorrelate(GParseEnv gpe);
void GParseDeconvolve(GParseEnv gpe);
void GParseOpen(GParseEnv gpe, const char *cmd);
int GParseOpenPrefix(GParseEnv gpe, char *prefix);
void GParseQuery(GParseEnv gpe, const char *query);
void GParseQueryFFDBTables(GParseEnv gpe, const char *cssTableName,
	const char *query);
void GParseQueryPrefixTables(GParseEnv gpe, const char *cssTableName,
	const char *query);
void GParseQueryODBCTables(GParseEnv gpe, const char *cssTableName,
	const char *query);
void GParseAddTables(GParseEnv gpe, Vector table);
void GParseAddTable(GParseEnv gpe, CssTable table);
void GParseReadWaveforms(GParseEnv gpe);
void GParseStoreRecords(GParseEnv gpe, Vector v);
void GParseApplyMethod(GParseEnv gpe, const char *method_string);
void GParsePrint(GParseEnv gpe, const char *cssTableName);
void GParseClear(GParseEnv gpe, const char *cssTableName);
void GParseList(GParseEnv gpe);
void GParseSet(GParseEnv gpe, const char *param);
void GParseRead(GParseEnv gpe, const char *filename);
void GParseWriteWaveforms(GParseEnv gpe);
void GParseWriteWaveform(GParseEnv gpe, FILE *fp, FILE *fpw,
	PSWaveform pw, char *prefix, char *dfile, char *date);
bool GParseInsertTables(GParseEnv gpe, const char *cssTableName);
bool GParseInsertTable(GParseEnv gpe, CssTable t);
void GParseGetId(GParseEnv gpe, char *lastidTable, char *keyname,
	long *address);
long GParsePrefixGetId(GParseEnv gpe, char *lastidTable, char *keyname);
bool GParseWriteTable(GParseEnv gpe, CssTable t);
void GParseUpdateTable(GParseEnv gpe, const char *cssTableName,
                CssTable told, CssTable tnew);
void GParseSetWindow(PSWaveform w, double tbeg, double tend);
void GParseSetNoWindow(PSWaveform w);
PSWaveform GParseGetWaveform(GParseEnv gpe, const char *prop_name);
Vector GParseGetWaveforms(GParseEnv gpe, const char *prop_name);
Vector GParseGetSelectedWaveforms(GParseEnv gpe);
int GParseSelectWaveforms(GParseEnv gpe, const char *name, bool only);
Vector GParseGetArrivalsOnWaveforms(GParseEnv gpe, CPlotData *cd);
void GParseSortWaveforms(GParseEnv gpe);
double GParseStartTime(GParseEnv gpe);
double GParseEndTime(GParseEnv gpe);
int GParseSourceType(GParseEnv gpe);


#ifdef HAVE_STDARG_H
void GParseSetErrorMsg(int err, const char *format, ...);
#else
void GParseSetErrorMsg();
#endif

#define GPARSE_MALLOC_ERR 1

/* ******* ffdb.c ********/
CssTable ffdbTableCreate(CssTable table, const char *table_name,
		const char *id_name, Password password);
int ffdbArrivalAdd(CssArrival arrival, TimeSeries ts, Password password);
int ffdbOriginCreate(CssOrigin origin, CssOrigerr origerr, CssTable any,
                Password password);


/* ******* odbc.c ********/
CssTable odbcTableCreate(CssTable table, const char *table_name,
		const char *id_name, Password password);
int odbcTableList(int num_tables, CssTable *tables, int num_extra,
		CssTable *extra, int num_attributes, Attribute **attributes,
		char **row, const char *rw, bool reopen);
int odbcTableChange(CssTable old_table, CssTable new_table);
int odbcTableDelete(int n_tables, CssTable *tables, int delete_index);
int odbcArrivalAdd(CssArrival arrival, TimeSeries ts, Password password);
int odbcArrivalChange(CssArrival arrival, TimeSeries ts, int  mask);
int odbcArrivalDelete(int n_arrivals, CssArrival *arrivals, int iarr,
		int n_assocs, CssAssoc *assocs);
int odbcUndoArrivalDelete(CssArrival arrival, int n_arrivals,
		CssArrival *arrivals, int n_assocs, CssAssoc *assocs);
int odbcOriginCreate(CssOrigin origin, CssOrigerr origerr, CssTable any,
		Password password);
int odbcOriginDelete(int n_origins, CssOrigin *origins, int iorg,
        int n_origerrs, CssOrigerr *origerrs, int n_netmags,CssNetmag *netmags);
int odbcUndoOriginDelete(CssOrigin origin, int n_origins, CssOrigin *origins,
	int n_origerrs, CssOrigerr *origerrs,
	int n_netmags, CssNetmag *netmags, int n_wftags, CssWftag *wftags,
	int n_assocs, CssAssoc *assocs, int n_stamags, CssStamag *stamags);
int odbcAssocAdd(CssAssoc assoc);
CssOrigerr odbcOrigerrAdd(CssOrigin origin);
int odbcStamagAdd(CssStamag stamag, CssArrival arrival, CssOrigin origin,
	Password password);
int odbcNetmagAdd(CssStamag stamag, CssArrival arrival, CssOrigin origin,
	Password password);
int odbcAmplitudeAdd(CssArrival arrival, CssAmplitude amplitude);

/* ******* GParseDetect.c ********/
void GParseDetect3cInit(FKParam *p);
void GParseDetect(GParseEnv gpe);
void GParseWriteDetect3c(GParseEnv gpe);



#endif /* _LIB_GPARSE_H_ */
