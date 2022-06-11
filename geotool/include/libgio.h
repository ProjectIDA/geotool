#ifndef _LIB_GIO_H
#define	_LIB_GIO_H

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#include "gobject++/CssTables.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "gobject++/DataSource.h"

/** @defgroup libgio library libgio
 */

bool cssAddArrival(CssArrivalClass *arrival, GTimeSeries *ts, Password password,
		const string &output_prefix="", long max_arid=-1);
bool cssChangeArrival(CssArrivalClass *arrival, GTimeSeries *ts, int mask);
bool cssCreateOrigin(CssOriginClass *origin, CssOrigerrClass *origerr, CssTableClass *any,
		Password password, const string &output_prefix="");
CssTableClass * cssCreateTable(CssTableClass *table, const string &table_name,
		const string &id_name, Password password,
		const string &output_prefix="");
bool cssWriteCDC(const char *prefix, const char *wa, gvector<Waveform *> &wvec,
		const char *remark, int raw);
bool sacWriteCDC(const char *prefix, const char *wa, gvector<Waveform *> &wvec,
		const char *remark, int raw, CssOriginClass **origins);
bool asciiWriteCDC(const char *prefix, const char *wa,
		gvector<Waveform *> &wvec, const char *remark, int raw);
void GParseWriteTimeSeries(const char *prefix,  const char *sta,
		const char *chan, GTimeSeries *ts);
bool cssDeleteArrival(Component *caller, CssArrivalClass *arrival);

FILE * cssOpenBackupFile(void);
bool cssAddAssoc(CssAssocClass *assoc);
CssOrigerrClass * cssCreateOrigerr(CssOriginClass *origin);
bool cssDeleteOrigin(Component *caller, CssOriginClass *origin);

bool cssDeleteTable(Component *caller, CssTableClass *table);
int cssWriteDotw(CssTableClass *t, const char *wfdisc_file, const char *wa,
		const char *dotw, const char *da, float *y,
		const char *sta, const char *chan, double time, long chanid,
		int nsamp, double samprate, double calib, double calper,
		const char *remark, long wfid);
bool cssAddNetmag(CssNetmagClass *netmag, CssTableClass *table);
bool cssAddStamag(CssStamagClass *stamag, CssTableClass *table);
int cssGetNextId(CssTableClass *css, const char *keyname, const char *prefix, long maxid=-1);
int cssUndoFileModification(const char *backup_file);
bool cssDeleteIds(Component *caller, const char *file,
		const char *css_table_name, const char *id_name,
		long id_value, FILE *fp_backup);
bool cssDelete(Component *caller, const char *file, CssTableClass *table,
		int num_compare, const char **member_names, FILE *fp_backup);

#endif
