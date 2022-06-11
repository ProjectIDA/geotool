/** \file cssInput.cpp
 *  \brief Css I/O routines.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>
#include <strings.h>
#include <math.h>
#include <vector>

#include "libgio.h"
#include "gobject++/GSourceInfo.h"
#include "DataMethod.h"
#include "IIRFilter.h"
#include "Waveform.h"
#include "widget/TableListener.h"
#include "FFDatabase.h"
#include "motif++/Application.h"
#include "cssio.h"

#ifdef HAVE_LIBODBC
#include "FixBool.h"
#include "libgdb.h"
#endif
#include "gobject++/CssTables.h"
extern "C" {
#include "libstring.h"
}

#define open_warn(path) \
    { \
	if(errno > 0) { \
	    snprintf(error,sizeof(error),"css: cannot open: %s\n%s",\
		path, strerror(errno)); \
	} \
	else { \
	    snprintf(error,sizeof(error),"css: cannot open: %s", path); \
	} \
	logErrorMsg(LOG_WARNING, error); \
    }

#define write_warn(file) \
    { \
	if(errno > 0) { \
	    snprintf(error,sizeof(error),"css: cannot write to: %s\n%s",\
                file, strerror(errno));\
	} \
	else { \
	    snprintf(error, sizeof(error), "css: cannot write to: %s", file); \
	} \
	logErrorMsg(LOG_WARNING, error); \
    }

static bool writeFilter(CssArrivalClass *arrival, GTimeSeries *ts, int arid,
	const char *sta, const char *chan, WfdiscPeriod *wp, DateTime *dt);
static bool writePick(CssArrivalClass *arrival, int mask, int arid, const char *sta,
	const char *chan, WfdiscPeriod *wp, DateTime *load_dt, FILE *fp_backup,
	const string &backup_file);
static bool write_ts(FILE *fp, FILE *fpw, const char *sta, const char *chan,
	GTimeSeries *ts, CssTableClass *t, const char *prefix, const char *dfile,
	const char *dotw_file, int commid, int raw);
static void getTableFile(const char *dir, const char *prefix,
	const char *css_table_name, char *file, int len);
static bool sameMember(int type, char *format, char *member1, char *member2);
static void doDeleteCallbacks(Component *caller, CssTableClass *table,
	const char *file, char *sline, int num_members, CssClassDescription *des);
static bool sameFile(const char *file1, const char *file2);

bool
cssAddArrival(CssArrivalClass *arrival, GTimeSeries *ts, Password password,
		const string &output_prefix, long max_arid)
{
    int		len, dir_q, prefix_q;
    const char	*sta, *chan, *path;
    char	arr_file[MAXPATHLEN+1], file[MAXPATHLEN+1];
    char	prefix[MAXPATHLEN+1], error[MAXPATHLEN+100];
    char	full_prefix[MAXPATHLEN+1];
    const char	*err_msg, *sep, *dir, *pref;
    FILE	*fp;
    DateTime	dt;
    struct stat	buf;
    DateTime	load_dt;
    GSegment	*seg;
    CssWftagClass	tag;
    CssPickClass	pick;
    GSourceInfo *s = NULL;
    WfdiscPeriod *wp=NULL;

    if( !(seg = ts->nearestSegment(arrival->time)) ) {
	    logErrorMsg(LOG_WARNING, "Invalid time. Cannot add arrival.");
	    return false;
    }
    wp = ts->getWfdiscPeriod(seg->tbeg());
    if(!wp) {
	    logErrorMsg(LOG_WARNING,
			"WfdiscPeriod not found. Cannot add arrival.");
	    return false;
    }
    s = &ts->source_info;

    if(!s) {
	    logErrorMsg(LOG_WARNING,
		"No source information. Cannot add arrival.");
	    return false;
    }
    s->copySource(arrival);

    dir = quarkToString(wp->dir);
    len = (dir != NULL) ? strlen(dir) : 0;
    sep = (len > 0 && dir[len-1] != '/') ? "/" : "";

    if(output_prefix.empty()) {
	    pref = quarkToString(wp->prefix);
	    dir_q = wp->dir;
	    prefix_q = wp->prefix;
	    prefix[0] = '\0';
	    snprintf(prefix, sizeof(prefix), "%s%s%s", dir ? dir : "", sep,
			pref ? pref : "");
    }
    else {
	    int i;
	    for(i = (int)output_prefix.length()-1; i >= 0 &&
		output_prefix[i] != '/'; i--);
	    prefix[0] = '\0';
	    snprintf(prefix, sizeof(prefix), "%s", output_prefix.c_str()+i+1);
	    prefix_q = stringToQuark(prefix);
	    snprintf(prefix, sizeof(prefix), "%s", output_prefix.c_str());
	    if(i > 0 && i < MAXPATHLEN) {
		memset(file, 0, sizeof(file));
		strncpy(file, output_prefix.c_str(), i);
		dir_q = stringToQuark(file);
	    }
	    else {
		dir_q = stringToQuark("./");
	    }
    }
		
    if(prefix[0] != '\0') {
	snprintf(arr_file, sizeof(arr_file), "%s.arrival", prefix);
    }
    else {
	stringcpy(arr_file, "dot.arrival", MAXPATHLEN+1);
    }
    if((fp = fopen(arr_file, "r+")) == NULL
		&& (fp = fopen(arr_file, "a+")) == NULL)
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo arrival changes will be saved.",
		arr_file, strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error),
		"Cannot write to: %s\nNo arrival changes will be saved.",
		arr_file);
	}
	logErrorMsg(LOG_WARNING, error);
	return false;
    }
	
    arrival->setDir(dir_q);
    arrival->setPrefix(prefix_q);
    arrival->setFile(stringToQuark(arr_file));
    arrival->setFormat(stringToQuark("css"));
    snprintf(file, sizeof(file), "%s.pick", prefix);
    arrival->pick_file = stringToQuark(file);
    snprintf(file, sizeof(file), "%s.wftag", prefix);
    arrival->wftag_file = stringToQuark(file);
    snprintf(file, sizeof(file), "%s.filter", prefix);
    arrival->filter_file = stringToQuark(file);

    arrival->pick_index = -1;
    arrival->wftag_index = -1;
    arrival->filter_index = -1;

    if(stat(arr_file, &buf) || buf.st_size == 0)
    {
	snprintf(file, sizeof(file), "%s.wfdisc", prefix);
	buf.st_size = 0;
    }
    fseek(fp, 0, 2);
    timeEpochToDate(timeGetEpoch(), &load_dt);

    snprintf(full_prefix, sizeof(full_prefix), "%s/%s",
	    quarkToString(dir_q), quarkToString(prefix_q));
    arrival->arid = cssGetNextId(arrival, "arid", full_prefix, max_arid);
    int dc = stringToQuark(full_prefix);
    arrival->setIds(dc, 1);

    timeEpochToDate(arrival->time, &dt);

    arrival->jdate = timeJDate(&dt);
    arrival->per = arrival->period;

    arrival->lddate = load_dt;
    stringcpy(arrival->auth, password->pw_name, sizeof(arrival->auth));

    if(arrival->write(fp, &err_msg))
    {
	if(err_msg) {
	    snprintf(error, sizeof(error), "write error: %s\n%s",
			arr_file, err_msg);
	}
	else {
	    snprintf(error, sizeof(error), "write error: %s", arr_file);
	}
	logErrorMsg(LOG_WARNING, error);
    }
    fclose(fp);

    sta = arrival->sta;
    chan = arrival->chan;

    path = quarkToString(arrival->wftag_file);

    if(stat(path, &buf) < 0) {
	arrival->wftag_index = 0;
    }
    else {
	arrival->wftag_index = buf.st_size/(tag.getLineLength() + 1);
    }
    if((fp = fopen(path, "a")) == NULL) {
	write_warn(path);
    }
    else
    {
	stringcpy(tag.tagname, "arid", sizeof(tag.tagname));
	tag.lddate = load_dt;
	tag.tagid = arrival->arid;
	tag.wfid = wp->wf.wfid;
	if(tag.write(fp, &err_msg))
	{
	    if(err_msg) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
			path, err_msg);
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", path);
	    }
	    snprintf(error, sizeof(error), "write error: %s", path);
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);
    }

    path = quarkToString(arrival->pick_file);

    if(stat(path, &buf) < 0) {
	arrival->pick_index = 0;
    }
    else {
	arrival->pick_index = buf.st_size/(pick.getLineLength() + 1);
    }
    if((fp = fopen(path, "a")) == NULL) {
	write_warn(path);
    }
    else
    {
	stringcpy(pick.sta, sta, sizeof(pick.sta));
	stringcpy(pick.chan, chan, sizeof(pick.chan));
	pick.chanid = wp->wf.chanid;
	pick.time = arrival->boxtime;
	pick.arid = arrival->arid;
	pick.amp = arrival->amp_cnts;
	pick.per = arrival->period;
	pick.calib = wp->wf.calib;
	pick.calper = wp->wf.calper;
	if(arrival->amp_cnts > 0.00000) {
	    pick.ampcalib = arrival->amp_nms/arrival->amp_cnts;
	}
	else {
	    pick.ampcalib = 1.0;
	}

	pick.ampmin = arrival->boxmin;
	if(wp->wf.calib != 0.) {
	    pick.ampmin /= fabs(wp->wf.calib);
	}
	stringcpy(pick.amptype, "hpp", sizeof(pick.amptype));

	pick.commid = -1;
	pick.lddate = load_dt;

	if(pick.write(fp, &err_msg))
	{
	    if(err_msg) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
			path, err_msg);
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", path);
	    }
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);
    }

    IIRFilter *iir;
    if( (iir = (IIRFilter *)ts->getMethod("IIRFilter")) &&
		strcasecmp(iir->getType(), "NA"))
    {
	CssFilterClass filter;
	path = quarkToString(arrival->filter_file);
	if(stat(path, &buf) < 0) {
	    arrival->filter_index = 0;
	}
	else {
	    arrival->filter_index = buf.st_size/(filter.getLineLength() + 1);
	}
	if((fp = fopen(path, "a")) == NULL) {
	    write_warn(path);
	}
	else
	{
	    stringcpy(filter.sta, sta, sizeof(filter.sta));
	    stringcpy(filter.chan, chan, sizeof(filter.chan));
	    filter.arid = arrival->arid;
	    stringcpy(filter.band,iir->getType(),sizeof(filter.band));

	    if(iir->getZeroPhase()) {
		stringcpy(filter.ftype, "z", sizeof(filter.ftype));
	    }
	    else  {
		stringcpy(filter.ftype, "c", sizeof(filter.ftype));
	    }
	    filter.forder = iir->getOrder();
	    filter.lofreq = iir->getFlow();
	    filter.hifreq = iir->getFhigh();
	    stringcpy(filter.algo, "iir_1988_Dave_Harris_LLNL",
			sizeof(filter.algo));
	    stringcpy(filter.program, "iir", sizeof(filter.program));
	    filter.lddate = load_dt;
	    filter.chanid = wp->wf.chanid;
	    filter.wfid = wp->wf.wfid;
	    if(filter.write(fp, &err_msg))
	    {
		if(err_msg) {
		    snprintf(error, sizeof(error),"write error: %s\n%s",
				path, err_msg);
		}
		else {
		    snprintf(error, sizeof(error), "write error: %s", path);
		}
		logErrorMsg(LOG_WARNING, error);
	    }
	    fclose(fp);
	}
    }
    return true;
}

bool
cssChangeArrival(CssArrivalClass *arrival, GTimeSeries *ts, int mask)
{
	const char	*afile, *err_msg;
	string		backup_file;
	char		error[MAXPATHLEN+50];
	FILE		*fp, *fp_backup;
	CssWftagClass	tag;
	CssArrivalClass	a_tmp;
	WfdiscPeriod	*wp = NULL;
	DateTime	dt;
        Password	password;
	DateTime	load_dt;
	long		pos;
	int		err;

	password = getpwuid(getuid());

	if(ts != NULL) {
	    GSegment *seg;
	    if( !(seg = ts->nearestSegment(arrival->time)) ) {
		snprintf(error, sizeof(error),
			"Invalid time. Cannot change arrival.");
		logErrorMsg(LOG_WARNING, error);
		return false;
	    }
	    wp = ts->getWfdiscPeriod(seg->tbeg());
	}

	if((afile = quarkToString(arrival->getFile())) == NULL ||
		afile[0] == '\0')
	{
	    snprintf(error, sizeof(error), "No arrival file for %s/%s.",
			arrival->sta, arrival->chan);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	if((fp = fopen(afile, "r+")) == NULL)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo arrival changes will be saved.",
				afile, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
	"Cannot write to: %s\nNo arrival changes will be saved.", afile);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}

	if((fp_backup = cssOpenBackupFile()) == NULL) {
	    fclose(fp);
	    return false;
	}

	timeEpochToDate(arrival->time, &dt);

	pos = 0;
	while((err = a_tmp.read(fp, &err_msg)) != EOF) {
	    if(a_tmp.arid == arrival->arid) break;
	    pos = ftell(fp);
	}

	if(!Application::getProperty("backup_file", backup_file)) return false;

	fprintf(fp_backup, "replace %s %ld\nrecord ", afile, pos);

	if(a_tmp.write(fp_backup, &err_msg))
	{
	    snprintf(error, sizeof(error), "write error: %s",
		backup_file.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp); fclose(fp_backup);
	    return false;
	}

	timeEpochToDate(timeGetEpoch(), &load_dt);
	a_tmp.lddate = load_dt;
	if(mask & CHANGE_TIME) {
	    a_tmp.time = arrival->time;
	    a_tmp.jdate = timeJDate(&dt);
	}
	if(mask & CHANGE_AMP_PER && arrival->period >= 0.) {
	    a_tmp.amp = arrival->amp;
	    a_tmp.per = arrival->period;
	}
	if(mask & CHANGE_PHASE_NAME) {
	    stringcpy(arrival->iphase, arrival->phase, sizeof(arrival->iphase));
	    stringcpy(a_tmp.iphase, arrival->phase, sizeof(a_tmp.phase));
	}
	if(mask & CHANGE_STASSID) {
	    a_tmp.stassid = arrival->stassid;
	}
	if(mask & CHANGE_AZIMUTH) {
	    a_tmp.azimuth = arrival->azimuth;
	}
	if(mask & CHANGE_SLOW) {
	    a_tmp.slow = arrival->slow;
	}
	stringcpy(a_tmp.auth, password->pw_name, sizeof(a_tmp.auth));

	fseek(fp, pos, 0);
	if(a_tmp.write(fp, &err_msg)) {
	    snprintf(error, sizeof(error), "write error: %s", afile);
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);

	if( wp ) {
	    tag.tagid = a_tmp.arid;
	    tag.wfid = wp->wf.wfid;
	}

	if(ts == NULL || mask == CHANGE_STASSID)
	{
	    fclose(fp_backup);
	    return true;
	}

	if(mask == CHANGE_PHASE_NAME)
	{
	    fclose(fp_backup);
	    return true;
	}

	const char *pick_file = quarkToString(arrival->pick_file);
	if(wp && pick_file && pick_file[0] != '\0')
	{
	    if(!writePick(arrival, mask, a_tmp.arid, a_tmp.sta, a_tmp.chan, wp,
			&load_dt, fp_backup, backup_file))
	    {
		fclose(fp_backup);
		return true;
	    }
	    fclose(fp_backup);
	}

	if(wp && !writeFilter(arrival, ts, a_tmp.arid, a_tmp.sta,
		a_tmp.chan, wp, &load_dt)) return false;

	return true;
}

static bool
writeFilter(CssArrivalClass *arrival, GTimeSeries *ts, int arid, const char *sta,
		const char *chan, WfdiscPeriod *wp, DateTime *dt)
{
    char error[MAXPATHLEN+50];
    const char *path, *err_msg=NULL;
    struct stat buf;
    DateTime load_dt;
    CssFilterClass filter;
    IIRFilter *iir;
    FILE *fp;

    path = quarkToString(arrival->filter_file);

    if(!path || path[0] == '\0') return true;

    if( (iir = (IIRFilter *)ts->getMethod("IIRFilter"))  &&
	(strcasecmp(iir->getType(), "NA") || arrival->filter_index >= 0))
    {
	if(stat(path, &buf) < 0 || (fp = fopen(path, "r+")) == NULL)
	{
	    if((fp = fopen(path, "w")) == NULL) {
		write_warn(path);
		return False;
	    }
	    buf.st_size = 0;
	}
	stringcpy(filter.sta, sta, sizeof(filter.sta));
	stringcpy(filter.chan, chan, sizeof(filter.chan));
	filter.arid = arid;
	stringcpy(filter.band, iir->getType(), sizeof(filter.band));
	if(iir->getZeroPhase()) {
	    stringcpy(filter.ftype, "z", sizeof(filter.ftype));
	}
	else {
	    stringcpy(filter.ftype, "c", sizeof(filter.ftype));
	}
	filter.forder = iir->getOrder();
	filter.lofreq = iir->getFlow();
	filter.hifreq = iir->getFhigh();
	stringcpy(filter.algo, "iir_1988_Dave_Harris_LLNL",
		sizeof(filter.algo));
	stringcpy(filter.program, "iir", sizeof(filter.program));
	timeEpochToDate(timeGetEpoch(), &load_dt);
	filter.lddate = load_dt;

	if(arrival->filter_index < 0)
	{
	    arrival->filter_index = buf.st_size/(filter.getLineLength()+1);
	    fseek(fp, arrival->filter_index*(filter.getLineLength()+1), 0);
	    filter.chanid = wp->wf.chanid;
	    filter.wfid = wp->wf.wfid;
	    if(filter.write(fp, &err_msg)) {
		if(err_msg) {
		    snprintf(error, sizeof(error), "write error: %s\n%s",
			path, err_msg);
		}
		else {
		    snprintf(error, sizeof(error), "write error: %s", path);
		}
		logErrorMsg(LOG_WARNING, error);
	    }
	}
	else if(fseek(fp, arrival->filter_index*(filter.getLineLength()+1), 0))
	{
	    snprintf(error, sizeof(error), "Error reading %s", path);
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return False;
	}
	else
	{
	    filter.chanid = wp->wf.chanid;
	    filter.wfid = wp->wf.wfid;

	    if(filter.write(fp, &err_msg)) {
		if(err_msg) {
		    snprintf(error, sizeof(error), "write error: %s\n%s",
			path, err_msg);
		}
		else {
		    snprintf(error, sizeof(error), "write error: %s", path);
		}
		logErrorMsg(LOG_WARNING, error);
	    }
	    fclose(fp);
	}
    }
    return true;
}

bool
cssCreateOrigin(CssOriginClass *origin, CssOrigerrClass *origerr, CssTableClass *any,
			Password password, const string &output_prefix)
{
    char	origin_file[MAXPATHLEN+1], file[MAXPATHLEN+1];
    char	error[MAXPATHLEN+100];
    const char	*err_msg, *dir, *prefix, *sep, *origerr_file;
    int		len, dir_q, prefix_q;
    FILE	*fp;
    CssOriginClass	*o = NULL;
    DateTime	dt;

    origin_file[0] = '\0';

    if( output_prefix.empty() )
    {
	if(any->nameIs(cssOrigin))
	{
	    o = (CssOriginClass *)any;
	    dir_q = any->getDir();
	    dir = quarkToString(dir_q);
	    prefix_q = any->getPrefix();
	    prefix = quarkToString(prefix_q);
	    if(!dir || dir[0] == '\0') {
		snprintf(error, sizeof(error),
			"Cannot create origin: No path information.");
		return false;
	    }
	    len = strlen(dir);
	    sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
	    snprintf(origin_file, sizeof(origin_file), "%s%s%s.origin",
				dir, sep, prefix);
	    snprintf(file, sizeof(file), "%s%s%s.origerr", dir, sep, prefix);
	    origin->origerr_file = stringToQuark(file);
	    snprintf(file, sizeof(file), "%s%s%s.wftag", dir, sep, prefix);
	    origin->wftag_file = stringToQuark(file);
	    origin->setDir(o->getDir());
	    origin->setPrefix(o->getPrefix());
	    origin->setFile(stringToQuark(origin_file));
	}
	else if(any->nameIs("wftag"))
	{
	    dir_q = any->getDir();
	    dir = quarkToString(dir_q);
	    prefix_q = any->getPrefix();
	    prefix = quarkToString(prefix_q);
	    if(!prefix) prefix = "";
	    if(dir) {
		len = strlen(dir);
		sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
		snprintf(origin_file, sizeof(origin_file), "%s%s%s.origin",
				dir, sep, prefix);
	    }
	    else if(prefix) {
		snprintf(origin_file, sizeof(origin_file), "%s.origin", prefix);
	    }
	    else {
		snprintf(error, sizeof(error),
			"Cannot create origin: No path information.");
		logErrorMsg(LOG_WARNING, error);
		return false;
	    }
	}
	else if(any->nameIs("arrival"))
	{
	    dir_q = any->getDir();
	    dir = quarkToString(dir_q);
	    prefix_q = any->getPrefix();
	    prefix = quarkToString(prefix_q);
	    if(!prefix) prefix = "";
	    if(dir) {
		len = strlen(dir);
		sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
		snprintf(origin_file, sizeof(origin_file), "%s%s%s.origin",
				dir, sep, prefix);
	    }
	    else if(prefix) {
		snprintf(origin_file, sizeof(origin_file), "%s.origin", prefix);
	    }
	    else {
		snprintf(error, sizeof(error),
			"Cannot create origin: No path information.");
		logErrorMsg(LOG_WARNING, error);
		return false;
	    }
	}
	else
	{
	    snprintf(error, sizeof(error),
			"cssOriginCreate: invalid table type.");
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    else {
	char prefx[MAXPATHLEN+1];
	int i;
	for(i = (int)output_prefix.length()-1; i >= 0 &&
                output_prefix[i] != '/'; i--);
	prefx[0] = '\0';
	snprintf(prefx, sizeof(prefx), "%s", output_prefix.c_str()+i+1);
	prefix_q = stringToQuark(prefx);
	prefix = prefx;
	if(i > 0 && i < MAXPATHLEN) {
	    memset(file, 0, sizeof(file));
	    strncpy(file, output_prefix.c_str(), i);
	    dir_q = stringToQuark(file);
	}
	else {
	    dir_q = stringToQuark("./");
	}
	snprintf(origin_file, sizeof(origin_file), "%s.origin",
			output_prefix.c_str());
    }

    /* don't use a+ first, since on linux, ftell(fp) will give 0 for
     * the file position, even if the file size is not zero.
     */
    if((fp = fopen(origin_file, "r+")) == NULL
		&& (fp = fopen(origin_file, "a+")) == NULL)
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error),
	    "Cannot write to: %s\n%s\nNo origin changes will be saved.",
				origin_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\nNo origin changes will be saved.",
				origin_file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}

	origin->orid = cssGetNextId(any, "orid",
				quarkToString(origin->getDC()));
	origin->setIds(any->getDC(), origin->orid);
	origin->setFormat(any->getFormat());
	origerr->setFormat(any->getFormat());
	origerr->setIds(any->getDC(), origin->orid);
	origerr->orid = origin->orid;

	if(!o)
	{
	    origin->setDir(dir_q);
	    origin->setPrefix(prefix_q);
	    origin->setFile(stringToQuark(origin_file));
	    dir = quarkToString(dir_q);
	    prefix = quarkToString(prefix_q);
	    if(!prefix) prefix = "";
	    if(dir && dir[0] != '\0') {
		len = strlen(dir);
		sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
		snprintf(file, sizeof(file), "%s%s%s.origerr",dir, sep, prefix);
		origin->origerr_file = stringToQuark(file);
		snprintf(file, sizeof(file), "%s%s%s.wftag", dir, sep, prefix);
		origin->wftag_file = stringToQuark(file);
	    }
	    else if(prefix && prefix[0] != '\0') {
		snprintf(file, sizeof(file), "%s.origerr", prefix);
		origin->origerr_file = stringToQuark(file);
		snprintf(file, sizeof(file), "%s.wftag", prefix);
		origin->wftag_file = stringToQuark(file);
	    }
	    else {
		snprintf(error, sizeof(error),
			"cssOriginCreate: missing path information.");
		logErrorMsg(LOG_WARNING, error);
		origin->origerr_file = stringToQuark("");
		origin->wftag_file = stringToQuark("");
	    }
	}
	origin->wftag_index = -1;

	origerr->setDir(origin->getDir());
	origerr->setPrefix(origin->getPrefix());
	origerr->setFile(origin->origerr_file);

	fseek(fp, 0, 2);

	timeEpochToDate(origin->time, &dt);

	origin->setFileOffset(ftell(fp));

	origin->jdate = timeJDate(&dt);
	timeEpochToDate(timeGetEpoch(), &origin->lddate);
	stringcpy(origin->auth, password->pw_name, sizeof(origin->auth));

	if(origin->write(fp, &err_msg))
	{
	    snprintf(error, sizeof(error), "write error: %s", origin_file);
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);

	origerr_file = quarkToString(origerr->getFile());
	/* don't use a+ first, since on linux, ftell(fp) will give 0 for
	 * the file position, even if the file size is not zero.
	 */
	if((fp = fopen(origerr_file, "r+")) == NULL
		&& (fp = fopen(origerr_file, "a+")) == NULL)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo origerr changes will be saved.",
				origerr_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\nNo origerr changes will be saved.",
				origerr_file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	fseek(fp, 0, 2);

	origerr->setFileOffset(ftell(fp));
	timeEpochToDate(timeGetEpoch(), &origerr->lddate);

	if(origerr->write(fp, &err_msg))
	{
	    snprintf(error, sizeof(error), "write error: %s", origerr_file);
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);

	return true;
}

bool
cssWriteCDC(const char *prefix, const char *wa,
	gvector<Waveform *> &wvec, const char *remark, int raw)
{
    int i, j, fd, n, commid;
    FILE *fp, *fpw;
    char error[MAXPATHLEN+100];
    char tmp_wfdisc_file[MAXPATHLEN+1];
    char tmp_dotw_file[MAXPATHLEN+1];
    char wfdisc_file[MAXPATHLEN+1];
    char dotw_file[MAXPATHLEN+1];
    char dfile[33];
    gvector<DataMethod *> methods;
    char remark_file[MAXPATHLEN+1], rmk[81];

    if(wvec.size() <= 0) return true;

    n = (int)strlen(prefix);
    for(i = n-1; i >= 0 && prefix[i] != '/'; i--);

    if(i == n-1)
    {
	logErrorMsg(LOG_WARNING, "zero length prefix filename.");
	return false;
    }
    else if(n-1-i > 30)
    {
	snprintf(error, sizeof(error),"prefix filename > 30 characters: %s",
			prefix+i+1);
	logErrorMsg(LOG_WARNING, error);
	return false;
    }

    snprintf(dfile, sizeof(dfile), "%s.w", prefix+i+1);

    snprintf(wfdisc_file, sizeof(wfdisc_file), "%s.wfdisc", prefix);
    snprintf(dotw_file, sizeof(dotw_file), "%s.w", prefix);

    if(!strcmp(wa, "w")) {
	// write to temporary files in case the output files are input files.
	snprintf(tmp_wfdisc_file, sizeof(tmp_wfdisc_file), "%s.XXXXXX",
		wfdisc_file);
	fd = mkstemp(tmp_wfdisc_file);
	if(fd != -1) close(fd);
    }
    else {
	strcpy(tmp_wfdisc_file, wfdisc_file);
    }
    if((fp = fopen(tmp_wfdisc_file, wa)) == NULL)
    {
	open_warn(tmp_wfdisc_file);
	return false;
    }

    if(!strcmp(wa, "w")) {
	// write to temporary files in case the output files are input files.
	snprintf(tmp_dotw_file, sizeof(tmp_dotw_file), "%s.XXXXXX", dotw_file);
	fd = mkstemp(tmp_dotw_file);
	if(fd != -1) close(fd);
    }
    else {
	strcpy(tmp_dotw_file, dotw_file);
    }

    if((fpw = fopen(tmp_dotw_file, wa)) == NULL)
    {
	fclose(fp);
	open_warn(tmp_dotw_file);
	return false;
    }
    commid = (remark != NULL && remark[0] != '\0') ?
		cssGetNextId(NULL, "commid", prefix) : -1;

    for(i = 0; i < wvec.size(); i++)
    {
	if(wvec[i]->num_dw <= 0)
	{
	    GTimeSeries *ts = new GTimeSeries(wvec[i]->ts);

	    if(raw) {
		ts->reread();
		wvec[i]->ts->getMethods("CutData", methods);
		// apply CutData, if found.
		for(int k = 0; k < methods.size(); k++) {
		    methods[k]->applyMethod(1, &ts);
		}
	    }
	    if(!write_ts(fp, fpw, wvec[i]->sta(), wvec[i]->chan(),
			ts, NULL, prefix, dfile, dotw_file, commid, raw))
	    {
		fclose(fp); fclose(fpw);
		ts->deleteObject();
		return false;
	    }
	    ts->deleteObject();
	}
	for(j = 0; j < wvec[i]->num_dw; j++)
	{
	    GDataPoint *d1 =  wvec[i]->dw[j].d1;
	    GDataPoint *d2 =  wvec[i]->dw[j].d2;

	    GTimeSeries *ts = wvec[i]->ts->subseries(d1->time(), d2->time());
	    if(raw) {
		ts->reread();
		wvec[i]->ts->getMethods("CutData", methods);
		// apply CutData, if found.
		for(int k = 0; k < methods.size(); k++) {
		    methods[k]->applyMethod(1, &ts);
		}
	    }
	    if(!write_ts(fp, fpw, wvec[i]->sta(), wvec[i]->chan(), ts, NULL,
			prefix, dfile, dotw_file, commid, raw))
	    {
		fclose(fp); fclose(fpw);
		ts->deleteObject();
		return false;
	    }
	    ts->deleteObject();
	}
    }
    fclose(fp); fclose(fpw);

    if(!strcmp(wa, "w")) {
	if(!rename(tmp_wfdisc_file, wfdisc_file) == -1) {
	    snprintf(error, sizeof(error), "rename error: %s\n%s",
			strerror(errno), wfdisc_file);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	if(!rename(tmp_dotw_file, dotw_file) == -1) {
	    snprintf(error, sizeof(error), "rename error: %s\n%s",
			strerror(errno), dotw_file);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }

    if(remark != NULL && remark[0] != '\0')
    {
	snprintf(remark_file, sizeof(remark_file), "%s.remark", prefix);

	if((fp = fopen(remark_file, wa)) == NULL) {
	    open_warn(remark_file);
	    return false;
	}
	if((int)strlen(remark) <= 80) {
	    fprintf(fp, "%8d %8d %-80.80s %-17.17s\n", commid, 0, remark,
			timeLoadDate());
	}
	else {
	    strncpy(rmk, remark, 80);
	    rmk[80] = '\0';
	    fprintf(fp, "%8d %8d %-80.80s %-17.17s\n", commid, 0, rmk,
			timeLoadDate());
	}
	fclose(fp);
    }
    return true;
}

static bool
write_ts(FILE *fp, FILE *fpw, const char *sta, const char *chan,
	GTimeSeries *ts, CssTableClass *t, const char *prefix, const char *dfile,
	const char *dotw_file, int commid, int raw)
{
    int j, k, err;
    long nsamp;
    const char *err_msg;
    bool remove_calib = false;
    CssWfdiscClass w;
    DateTime dt;
    string s;
    bool big_endian;
    union
    {
	char	a[4];
	float	f;
	int	i;
	short	s;
    } e1;

    e1.a[0] = 0; e1.a[1] = 0;
    e1.a[2] = 0; e1.a[3] = 1;
    big_endian = (e1.i == 1) ? true : false;

    stringcpy(w.sta, sta, sizeof(w.sta));
    stringcpy(w.chan, chan, sizeof(w.chan));
    stringcpy(w.dir, ".", sizeof(w.dir));
    stringcpy(w.dfile, dfile, sizeof(w.dfile));

    if(!raw) {
	/* Remove calib if is has been applied.
	 */
	remove_calib = ts->getMethod("CalibData") ? true : false;
    }

    for(k = 0; k < ts->size(); k++)
    {
	w.chanid = ts->chanid();
	if(ts->getInstype(s)) {
	    strncpy(w.instype, s.c_str(), sizeof(w.instype));
	}
	else {
	    w.instype[0] = '\0';
	}
	w.calib = (ts->segment(k)->calib() != 0.) ? ts->segment(k)->calib():1.;
	w.calper = ts->segment(k)->calper();
	w.time = ts->segment(k)->tbeg();
	timeEpochToDate(w.time, &dt);
	w.jdate = timeJDate(&dt);
	nsamp = ts->segment(k)->length();
	w.nsamp = nsamp;
	w.endtime = w.time + (w.nsamp-1)*ts->segment(k)->tdel();
	w.samprate = 1./ts->segment(k)->tdel();
	w.wfid = cssGetNextId(t, "wfid", prefix);
	w.foff = ftell(fpw);
	w.commid = commid;
	if(big_endian) {
	    stringcpy(w.datatype, "t4", sizeof(w.datatype));
	}
	else {
	    stringcpy(w.datatype, "f4", sizeof(w.datatype));
	}
	timeEpochToDate(timeGetEpoch(), &w.lddate);

	if(w.write(fp, &err_msg)) {
	    return false;
	}
	if(remove_calib  && w.calib != 1.){
	    for(j = 0; j < w.nsamp; j++) {
		ts->segment(k)->data[j] /= w.calib;
	    }
	}

	err = fwrite(ts->segment(k)->data, sizeof(float), (size_t)nsamp, fpw);
	if(err != nsamp)
	{
	    char error[MAXPATHLEN+100];
	    if(errno > 0) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
				dotw_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", dotw_file);
	    }
	    return false;
	}
    }
    return true;
}

/* table_name: stassoc, etc.
 * id_name: stassid, etc
 */
CssTableClass *
cssCreateTable(CssTableClass *table, const string &table_name, const string &id_name,
		Password password, const string &output_prefix)
{
    char	file[MAXPATHLEN+1];
    char	error[MAXPATHLEN+100];
    const char	*err_msg, *dir, *prefix, *sep;
    int		i, id;
    int		len;
    CssTableClass	*t;
    FILE	*fp;

    t = CssTableClass::createCssTable(table_name);
    id = cssGetNextId(table, id_name.c_str(), quarkToString(table->getDC()));
    t->setIds(table->getDC(), id);
    t->setFormat(table->getFormat());

    if(!output_prefix.empty())
    {
	char prefx[MAXPATHLEN+1];
	int prefix_q, dir_q;
	for(i = (int)output_prefix.length()-1; i >= 0 &&
                output_prefix[i] != '/'; i--);
	prefx[0] = '\0';
	snprintf(prefx, sizeof(prefx), "%s", output_prefix.c_str()+i+1);
	prefix_q = stringToQuark(prefx);
	if(i > 0 && i < MAXPATHLEN) {
	    memset(file, 0, sizeof(file));
	    strncpy(file, output_prefix.c_str(), i);
	    dir_q = stringToQuark(file);
	}
	else {
	    dir_q = stringToQuark("./");
	}
	t->setDir(dir_q);
	t->setPrefix(prefix_q);
	snprintf(file, sizeof(file), "%s.%s", output_prefix.c_str(),
			table_name.c_str());
    }
    else
    {

	t->setDir(table->getDir());
	t->setPrefix(table->getPrefix());

	dir = quarkToString(table->getDir());
	prefix = quarkToString(table->getPrefix());

	if(dir) {
	    len = strlen(dir);
	    sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
	    snprintf(file, sizeof(file), "%s%s%s.%s",
				dir, sep, prefix, table_name.c_str());
	}
	else if(prefix) {
	    snprintf(file, sizeof(file), "%s.%s", prefix, table_name.c_str());
	}
	else {
	    snprintf(error, sizeof(error),
		"cssCreateTable: no path information.");
	    logErrorMsg(LOG_WARNING, error);
	    return t;
	}
    }

    t->setFile(stringToQuark(file));

    /* don't use a+ first, since on linux, ftell(fp) will give 0 for
     * the file position, even if the file size is not zero.
     */
    if((fp = fopen(file, "r+")) == NULL && (fp = fopen(file, "a+")) == NULL)
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo %s changes will be saved.",
			file, strerror(errno), table_name.c_str());
	}
	else {
	    snprintf(error, sizeof(error),
	      "Cannot write to: %s\nNo %s changes will be saved.",
			file, table_name.c_str());
	}
	logErrorMsg(LOG_WARNING, error);
    }
    else
    {
	if((i = t->memberIndex("lddate")) >= 0) {
	    t->setMember(i, timeLoadDate());
	}

	fseek(fp, 0, 2);

	t->setFileOffset(ftell(fp));

	if( !id_name.empty() && (i = table->memberIndex(id_name)) >= 0)
	{
	    long *id = (long *)t->member(i);
	    *id = t->getID();
	}

	if(t->write(fp, &err_msg)) {
	    snprintf(error, sizeof(error), "write error: %s", file);
	    logErrorMsg(LOG_WARNING, error);
	    delete t;
	}
	fclose(fp);
    }
    return t;
}

static bool
writePick(CssArrivalClass *arrival, int mask, int arid, const char *sta,
		const char *chan, WfdiscPeriod *wp, DateTime *load_dt,
		FILE *fp_backup, const string &backup_file)
{
	char error[MAXPATHLEN+50];
	const char *path, *err_msg=NULL;
	CssPickClass pick;
	struct stat buf;
	FILE *fp;

	if(!arrival->pick_file) return true;

	path = quarkToString(arrival->pick_file);

	if(stat(path, &buf) < 0 || (fp = fopen(path, "r+")) == NULL)
	{
	    if((fp = fopen(path, "w")) == NULL) {
		write_warn(path);
		return false;
	    }
	    buf.st_size = 0;
	}
	if(arrival->pick_index < 0)
	{
	    arrival->pick_index = buf.st_size/(pick.getLineLength()+1);
	    stringcpy(pick.sta, sta, sizeof(pick.sta));
	    stringcpy(pick.chan, chan, sizeof(pick.chan));
	    pick.chanid = (wp) ? wp->wf.chanid : -1;
	    pick.arid = arid;
	}
	else if(fseek(fp, arrival->pick_index*(pick.getLineLength()+1), 0) ||
		pick.read(fp, &err_msg))
	{
	    fclose(fp);
	    if(err_msg) {
		snprintf(error, sizeof(error), "Error reading %s\n%s",
			path, err_msg);
	    }
	    else {
		snprintf(error, sizeof(error), "Error reading %s", path);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}

	fprintf(fp_backup, "replace %s %d\nrecord ", path,
			arrival->pick_index*(pick.getLineLength()+1));
	if(pick.write(fp_backup, &err_msg))
	{
	    if(err_msg) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
			backup_file.c_str(), err_msg);
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s",
			backup_file.c_str());
	    }
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return false;
	}

	fseek(fp, arrival->pick_index*(pick.getLineLength()+1), 0);

	if(mask & CHANGE_TIME)
	{
	    pick.time = arrival->time;
	}
	if(mask & CHANGE_AMP_PER)
	{
	    pick.per = arrival->period;
	    pick.time = arrival->boxtime;
	    if(wp)
	    {
		pick.calib = wp->wf.calib;
		if(pick.calib == 0.) pick.calib = 1.;
		pick.calper = wp->wf.calper;
	    }
	    else {
		pick.calib = 1.;
	    }
	    pick.ampmin = arrival->boxmin/pick.calib;
	    pick.amp = arrival->amp_Nnms/fabs(pick.calib);
	    if(arrival->amp_nms > 0. && arrival->amp_Nnms > 0.)
	    {
		pick.ampcalib = arrival->amp_nms/arrival->amp_Nnms;
	    }
	    stringcpy(pick.amptype, "hpp", sizeof(pick.amptype));
	}
	pick.commid = -1;
	timeEpochToDate(timeGetEpoch(), &pick.lddate);

	if(pick.write(fp, &err_msg))
	{
	    snprintf(error, sizeof(error), "write error: %s", path);
	    if(err_msg) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
			path, err_msg);
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", path);
	    }
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);
	return true;
}

bool
cssDeleteArrival(Component *c, CssArrivalClass *arrival)
{
    char	file[MAXPATHLEN+1];
    const char *arrival_names[5] = {"arid", "sta", "chan", "iphase", "time"};
    const char *wftag_names[2] = {"tagname", "tagid"};
    long arid = arrival->arid;
    FILE	*fp_backup;
    const char *dir = quarkToString(arrival->getDir());
    const char *prefix = quarkToString(arrival->getPrefix());

    /* delete the arrival
     */
    if((fp_backup = cssOpenBackupFile()) == NULL) {
	return false;
    }
    if( !cssDelete(c, NULL, arrival, 5, arrival_names, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete assocs
     */
    getTableFile(dir, prefix, cssAssoc, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssAssoc, "arid", arid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete amplitudes
     */
    getTableFile(dir, prefix, cssAmplitude, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssAmplitude, "arid", arid, fp_backup) ){
	fclose(fp_backup);
	return false;
    }

    /* delete stamags
     */
    getTableFile(dir, prefix, cssStamag, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssStamag, "arid", arid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete picks
     */
    getTableFile(dir, prefix, cssPick, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssPick, "arid", arid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete filters
     */
    getTableFile(dir, prefix, cssFilter, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssFilter, "arid", arid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete hydrofeatures
     */
    getTableFile(dir, prefix, cssHydroFeatures, file, sizeof(file));
    if(!cssDeleteIds(c, file, cssHydroFeatures, "arid", arid, fp_backup)){
	fclose(fp_backup);
	return false;
    }

    /* delete infrafeatures
     */
    getTableFile(dir, prefix, cssInfraFeatures, file, sizeof(file));
    if(!cssDeleteIds(c, file, cssInfraFeatures, "arid", arid, fp_backup)){
	fclose(fp_backup);
	return false;
    }

    /* delete fsdiscs
     */
    getTableFile(dir, prefix, cssFsdisc, file, sizeof(file));
    if(!cssDeleteIds(c, file, cssFsdisc, "arid", arid, fp_backup)) {
	fclose(fp_backup);
	return false;
    }

    /* delete wftags
     */
    CssWftagClass wftag;
    strncpy(wftag.tagname, "arid", sizeof(wftag.tagname));
    wftag.tagid = arid;
    getTableFile(dir, prefix, cssWftag, file, sizeof(file));
    if( !cssDelete(c, file, &wftag, 2, wftag_names, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    fclose(fp_backup);

    return true;
}

FILE *
cssOpenBackupFile(void)
{
    FILE *fp;
    string backup_file;

    if( !Application::getProperty("backup_file", backup_file) ) {
	const char *file = cssioGetTmpPrefix("/tmp", "geotl");
	Application::putProperty("backup_file", backup_file, false);
	backup_file.assign(file);
    }

    if((fp = fopen(backup_file.c_str(), "r+")) == NULL &&
       (fp = fopen(backup_file.c_str(), "w")) == NULL)
    {
	char error[MAXPATHLEN+100];
	if(errno > 0)
	{
	    snprintf(error, sizeof(error),
			"Cannot write to: %s\n%s\nCannot update arrival(s).",
			backup_file.c_str(), strerror(errno));
	}
	else
	{
	    snprintf(error, sizeof(error),
			"Cannot write to: %s\nCannot update arrival(s).",
			backup_file.c_str());
	}
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }
    fseek(fp, 0, 2);
    return fp;
}

/* assoc coming into here should already be initialized and filled */

bool
cssAddAssoc(CssAssocClass *assoc)
{
	const char	*err_msg, *dir, *sep, *prefix;
	char		file[MAXPATHLEN+1];
	char		error[MAXPATHLEN+50];
	int		len;
	FILE		*fp;

	dir = quarkToString(assoc->getDir());
	len = (dir != NULL) ? strlen(dir) : 0;
	sep = (len > 0 && dir[len-1] != '/') ? "/" : "";

	prefix = quarkToString(assoc->getPrefix());

	snprintf(file, sizeof(file), "%s%s%s.assoc", dir ? dir : "", sep,
			prefix ? prefix : "");

	assoc->setFile(stringToQuark(file));

	/* don't use a+ first, since on linux, ftell(fp) will give 0 for
	 * the file position, even if the file size is not zero.
	 */
	if((fp = fopen(file, "r+")) == NULL && (fp = fopen(file, "a+")) == NULL)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
		    "Cannot write to: %s\n%s\nNo assoc changes will be saved.",
		    file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
		    "Cannot write to: %s\nNo assoc changes will be saved.",
		    file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	else
	{
	    fseek(fp, 0, 2);

	    assoc->setFileOffset(ftell(fp));

	    timeEpochToDate(timeGetEpoch(), &assoc->lddate);

	    if(assoc->write(fp, &err_msg)) {
		snprintf(error, sizeof(error), "write error: %s", file);
		logErrorMsg(LOG_WARNING, error);
	    }
	    fclose(fp);
	}
	return true;
}

CssOrigerrClass *
cssCreateOrigerr(CssOriginClass *origin)
{
	const char	*err_msg, *dir, *sep, *prefix;
	char		file[MAXPATHLEN+1];
	char		error[MAXPATHLEN+50];
	int		len;
	CssOrigerrClass	*origerr;
	FILE		*fp;

	origerr = new CssOrigerrClass();
	origerr->setIds(origin->getDC(), 1);
	origerr->setFormat(origin->getFormat());
	origerr->orid = origin->orid;

	origerr->setDir(origin->getDir());
	origerr->setPrefix(origin->getPrefix());

	dir = quarkToString(origerr->getDir());
	len = (dir != NULL) ? strlen(dir) : 0;
	sep = (len > 0 && dir[len-1] != '/') ? "/" : "";

	prefix = quarkToString(origerr->getPrefix());

	snprintf(file, sizeof(file), "%s%s%s.origerr", dir ? dir : "", sep,
			prefix ? prefix : "");

	origerr->setFile(stringToQuark(file));

	/* don't use a+ first, since on linux, ftell(fp) will give 0 for
	 * the file position, even if the file size is not zero.
	 */
	if((fp = fopen(file, "r+")) == NULL && (fp = fopen(file, "a+")) == NULL)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo origerr changes will be saved.",
			file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\nNo origerr changes will be saved.", file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	}
	else
	{
	    fseek(fp, 0, 2);

	    origerr->setId(ftell(fp));
	    origerr->setFileOffset(ftell(fp));
	    timeEpochToDate(timeGetEpoch(), &origerr->lddate);

	    if(origerr->write(fp, &err_msg)) {
		snprintf(error, sizeof(error), "write error: %s", file);
		logErrorMsg(LOG_WARNING, error);
	    }
	    fclose(fp);
	}
	return origerr;
}

bool
cssDeleteOrigin(Component *c, CssOriginClass *origin)
{
    char	file[MAXPATHLEN+1];
    const char *origin_names[4] = {"orid", "lat", "lon", "time"};
    const char *wftag_names[2] = {"tagname", "tagid"};
    long orid = origin->orid;
    FILE	*fp_backup;
    const char *dir = quarkToString(origin->getDir());
    const char *prefix = quarkToString(origin->getPrefix());

    /* delete the origin
     */
    if((fp_backup = cssOpenBackupFile()) == NULL) {
	return false;
    }
    if( !cssDelete(c, NULL, origin, 4, origin_names, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete origerrs
     */
    getTableFile(dir, prefix, cssOrigerr, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssOrigerr, "orid", orid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete assocs
     */
    getTableFile(dir, prefix, cssAssoc, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssAssoc, "orid", orid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete stamags
     */
    getTableFile(dir, prefix, cssStamag, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssStamag, "orid", orid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete netmags
     */
    getTableFile(dir, prefix, cssNetmag, file, sizeof(file));
    if( !cssDeleteIds(c, file, cssNetmag, "orid", orid, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    /* delete wftags
     */
    CssWftagClass wftag;
    strncpy(wftag.tagname, "orid", sizeof(wftag.tagname));
    wftag.tagid = orid;
    getTableFile(dir, prefix, cssWftag, file, sizeof(file));
    if( !cssDelete(c, file, &wftag, 2, wftag_names, fp_backup) ) {
	fclose(fp_backup);
	return false;
    }

    fclose(fp_backup);

    return true;
}

static void
getTableFile(const char *dir, const char *prefix, const char *css_table_name,
		char *file, int len)
{
    int n;
    const char *sep;

    if(!prefix) prefix = "";
    if(dir) {
	n = strlen(dir);
	sep = (n > 0 && dir[n-1] != '/') ? "/" : "";
	snprintf(file, len, "%s%s%s.%s", dir, sep, prefix, css_table_name);
    }
    else {
	snprintf(file, len, "%s.%s", prefix, css_table_name);
    }
}

bool
cssDeleteTable(Component *caller, CssTableClass *table)
{
    FILE *fp_backup;
    bool ret;
    CssClassDescription *des = table->description();
    int num_compare, num = table->getNumMembers();
    const char **member_names = NULL;

    /* delete the arrival
     */
    if((fp_backup = cssOpenBackupFile()) == NULL) {
	return false;
    }

    member_names = (const char **)malloc(num*sizeof(const char *));
    num_compare = 0;
    for(int i = 0; i < num; i++) {
	if(des[i].type != CSS_LDDATE) {
	    member_names[num_compare++] = des[i].name;
	}
    }

    ret = cssDelete(caller, NULL, table, num_compare, member_names, fp_backup);

    Free(member_names);

    fclose(fp_backup);
    
    return ret;
}

bool
cssDeleteIds(Component *caller, const char *file, const char *css_table_name,
		const char *id_name, long id_value, FILE *fp_backup)
{
    int		c, fd, n, m, line_length;
    int		member_index, num_members;
    long	id, line_pos;
    char	error[MAXPATHLEN+100], *line=NULL, *s=NULL, *sline=NULL;
    char	tmp_file[MAXPATHLEN+10];
    FILE	*fp, *fp_tmp;
    bool	found_one, found_any;
    CssTableClass	*table;
    CssClassDescription *des;
    struct stat buf;

    if(stat(file, &buf) < 0) return true;

    if( !(table = CssTableClass::createCssTable(css_table_name)) ) {
	snprintf(error, sizeof(error), "cssDeleteIds: invalid table name: %s",
		css_table_name);
	logErrorMsg(LOG_WARNING, error);
	return false;
    }
    if( (member_index = table->memberIndex(id_name)) < 0) {
	snprintf(error, sizeof(error),
		"cssDeleteIds: invalid id_name: %s for table: %s",
		id_name, css_table_name);
	logErrorMsg(LOG_WARNING, error);
	delete table;
	return false;
    }

    des = table->description();
    num_members = table->getNumMembers();

    line_length = table->getLineLength();

    if( !(line = (char *)mallocWarn(line_length+1)) ) return false;
    if( !(sline = (char *)mallocWarn(line_length+1)) ) return false;

    if((fp = fopen(file, "r")) == NULL) {
	snprintf(error, sizeof(error),
	    "Cannot open: %s\nNo %s changes will be saved.",
	    file, css_table_name);
	logErrorMsg(LOG_WARNING, error);
	free(line);
	free(sline);
	delete table;
	return false;
    }

    snprintf(tmp_file, sizeof(tmp_file), "%s.XXXXXX", file);
    fd = mkstemp(tmp_file);
    if(fd != -1) close(fd);

    if((fp_tmp = fopen(tmp_file, "w")) == NULL) {
	snprintf(error, sizeof(error),
	    "Cannot write to: %s\nNo %s changes will be saved.",
	    tmp_file, css_table_name);
	fclose(fp);
	logErrorMsg(LOG_WARNING, error);
	free(line);
	free(sline);
	delete table;
	return false;
    }

    m = des[member_index].end - des[member_index].start + 1;
    s = (char *)mallocWarn(m+1);

    found_any = false;
    c = '\0';
    while(c != EOF)
    {
	line_pos = ftell(fp);
	for(n = 0; (c = getc(fp)) != '\n' && c != EOF && n < line_length; n++)
        {
            line[n] = c;
        }
	if(n == 0) break;

	found_one = false;
	if(n == line_length) {
	    strncpy(s, &line[des[member_index].start-1], m);
	    s[m] = '\0';
	    stringTrim(s);
	    if(stringToLong(s, &id) && id == id_value) found_one = true;
	}
	if(found_one) {
	    found_any = true;
	    if(fp_backup) {
		fprintf(fp_backup,"insert %s %ld\nrecord ", file, line_pos);
		fwrite(line, 1, n, fp_backup);
	    }
	    strncpy(sline, line, n);
	    doDeleteCallbacks(caller, table, file, sline, num_members, des);
	}
	else {
	    fwrite(line, 1, n, fp_tmp);
	}
	if(c == '\n') {
	    if(found_one) {
		if(fp_backup) fputc(c, fp_backup);
	    }
	    else fputc(c, fp_tmp);
	}
	else {
	    // read to the next '\n'
	    while((c = getc(fp)) != '\n' && c != EOF) {
		if(found_one) {
		    if(fp_backup) fputc(c, fp_backup);
		}
		else fputc(c, fp_tmp);
	    }
	    if(found_one) {
		if(fp_backup) fputc((int)'\n', fp_backup);
	    }
	    else if(c == '\n') fputc(c, fp_tmp);
	}
    }

    Free(line);
    Free(sline);
    Free(s);
    delete table;

    fclose(fp);
    fclose(fp_tmp);

    if(found_any) {
	if(rename(tmp_file, file) == -1) {
	    snprintf(error, sizeof(error),
		"cssDeleteIds:rename error: %s\n%s", strerror(errno), tmp_file);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    else if(unlink(tmp_file) == -1) {
	snprintf(error, sizeof(error),
		"cssDeleteIds:unlink error: %s\n%s", strerror(errno), tmp_file);
	logErrorMsg(LOG_WARNING, error);
	return false;
    }

    return true;
}

bool
cssDelete(Component *caller, const char *file, CssTableClass *table, int num_compare,
		const char **member_names, FILE *fp_backup)
{
    int		c, i, k, fd, n, line_length, num_members, *member_index=NULL;
    long	line_pos;
    char	error[MAXPATHLEN+100], *line=NULL, *sline=NULL;
    char	tmp_file[MAXPATHLEN+10];
    char	*member1, *member2;
    FILE	*fp, *fp_tmp;
    CssTableClass	*o;
    bool	found_one, found_any;
    CssClassDescription *des;
    struct stat buf;

    if(!file && ((file = quarkToString(table->getFile())) == NULL ||
	file[0] == '\0') )
    {
	snprintf(error, sizeof(error),
	    "cssDelete: error: %s file not found.", table->getName());
	logErrorMsg(LOG_WARNING, error);
	return false;
    }
    if(stat(file, &buf) < 0) return true;

    des = table->description();
    num_members = table->getNumMembers();
    member_index = (int *)mallocWarn(num_members*sizeof(int));

    for(i = 0; i < num_compare; i++) {
	member_index[i] = table->memberIndex(member_names[i]);
	if(member_index[i] < 0) {
	    snprintf(error, sizeof(error),
		"cssDelete: invalid member_name %s for table %s",
		member_names[i], table->getName());
	    logErrorMsg(LOG_WARNING, error);
	    free(member_index);
	    return false;
	}
    }

    line_length = table->getLineLength();

    if( !(line = (char *)mallocWarn(line_length+1)) ) return false;
    if( !(sline = (char *)mallocWarn(line_length+1)) ) return false;

    if((fp = fopen(file, "r+")) == NULL) {
	snprintf(error, sizeof(error),
	    "Cannot write to: %s\nNo %s changes will be saved.",
	    file, table->getName());
	logErrorMsg(LOG_WARNING, error);
	free(line);
	free(member_index);
	return false;
    }

    snprintf(tmp_file, sizeof(tmp_file), "%s.XXXXXX", file);
    fd = mkstemp(tmp_file);
    if(fd != -1) close(fd);

    if((fp_tmp = fopen(tmp_file, "w")) == NULL) {
	snprintf(error, sizeof(error),
	    "Cannot write to: %s\nNo %s changes will be saved.",
	    tmp_file, table->getName());
	fclose(fp);
	logErrorMsg(LOG_WARNING, error);
	free(line);
	free(member_index);
	return false;
    }

    o = CssTableClass::createCssTable(table->getName());

    found_any = false;
    c = '\0';
    while(c != EOF)
    {
	line_pos = ftell(fp);
	for(n = 0; (c = getc(fp)) != '\n' && c != EOF && n < line_length; n++)
        {
            line[n] = c;
        }
	if(n == 0) break;

	found_one = false;
	if(n == line_length) {
	    strncpy(sline, line, line_length);
	    for(i = 0; i < num_compare; i++) {
		k = member_index[i];
		sline[des[k].end] = '\0';
		stringTrim(&sline[des[k].start-1]);
		if(!o->setMember(k, &sline[des[k].start-1])) break;

		member1 = (char *)table + des[k].offset;
                member2 = (char *)o + des[k].offset;

		if( !sameMember(des[k].type, des[k].format, member1,member2) ) {
		    break;
		}
	    }
	    if(i == num_compare) found_one = true;
	}
	if(found_one) {
	    found_any = true;
	    fprintf(fp_backup,"insert %s %ld\nrecord ", file, line_pos);
	    fwrite(line, 1, n, fp_backup);

	    doDeleteCallbacks(caller, o, file, sline, num_members, des);
	}
	else {
	    fwrite(line, 1, n, fp_tmp);
	}
	if(c == '\n') {
	    if(found_one) fputc(c, fp_backup);
	    else fputc(c, fp_tmp);
	}
	else {
	    // read to the next '\n'
	    while((c = getc(fp)) != '\n' && c != EOF) {
		if(found_one) fputc(c, fp_backup);
		else fputc(c, fp_tmp);
	    }
	    if(found_one) fputc((int)'\n', fp_backup);
	    else if(c == '\n') fputc(c, fp_tmp);
	}
    }
    Free(line);
    Free(sline);
    Free(member_index);
    delete o;

    fclose(fp);
    fclose(fp_tmp);

    if(found_any) {
	if(rename(tmp_file, file) == -1) {
	    snprintf(error, sizeof(error),
		"cssDeleteIds:rename error: %s\n%s", strerror(errno), tmp_file);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    else if(unlink(tmp_file) == -1) {
	snprintf(error, sizeof(error),
		"cssDeleteIds:unlink error: %s\n%s", strerror(errno), tmp_file);
	logErrorMsg(LOG_WARNING, error);
	return false;
    }

    return true;
}

static void
doDeleteCallbacks(Component *caller, CssTableClass *table, const char *file,
		char *sline, int num_members, CssClassDescription *des)
{
    CssTableClass **archive;
    gvector<CssTableClass *> t;
    int i, j, num = CssTableClass::archive(&archive);

     // look for other tables from the same file with the same values.
    for(i = 0; i < num_members; i++) {
	sline[des[i].end] = '\0';
	stringTrim(&sline[des[i].start-1]);
	table->setMember(i, &sline[des[i].start-1]);
    }

    num = CssTableClass::archive(&archive);

    for(i = 0; i < num; i++) {
	if(*table == *archive[i] &&
		sameFile(file, quarkToString(archive[i]->getFile())) )
	{
	    t.push_back(archive[i]);
	}
    }

    for(i = 0; i < (int)t.size(); i++) {
	num = CssTableClass::archive(&archive);
	for(j = 0; j < num && t[i] != archive[j]; j++);
	if(j < num) {
	    TableListener::doCallbacks(t[i], caller, "delete");
	}
    }
    t.clear();
}

static bool
sameMember(int type, char *format, char *member1, char *member2)
{
    char buf1[100], buf2[100];

    switch(type)
    {
	case CSS_STRING:
	    if(strcmp(member1, member2)) return false;
	    break;
	case CSS_DATE:
	{
	    DateTime *t1 = (DateTime *)member1;
	    DateTime *t2 = (DateTime *)member2;
	    if(t1->year != t2->year || t1->month != t2->month
		|| t1->day != t2->day) return false;
	    break;
	}
	case CSS_LDDATE:
	{
	    DateTime *t1 = (DateTime *)member1;
	    DateTime *t2 = (DateTime *)member2;
	    if(t1->year != t2->year || t1->month != t2->month
		|| t1->day != t2->day) return false;
	    break;
	}
	case CSS_LONG:
	case CSS_JDATE:
	    if(*(long *)member1 != *(long *)member2) return false;
	    break;
	case CSS_INT:
	case CSS_QUARK:
	    if(*(int *)member1 != *(int *)member2) return false;
	    break;
	case CSS_DOUBLE:
	case CSS_TIME:
	    snprintf(buf1, sizeof(buf1), format, *((double *)member1));
	    snprintf(buf2, sizeof(buf2), format, *((double *)member2));
	    if(strcmp(buf1, buf2)) return false;
	    break;
	case CSS_FLOAT:
	    snprintf(buf1, sizeof(buf1), format, *((float *)member1));
	    snprintf(buf2, sizeof(buf2), format, *((float *)member2));
	    if(strcmp(buf1, buf2)) return false;
	    break;
	case CSS_BOOL:
	    if(*(bool *)member1 != *(bool *)member2) return false;
	    break;
	default:
	    return false;
    }
    return true;
}

static bool
sameFile(const char *file1, const char *file2)
{
    struct stat s1, s2;

    if(!strcmp(file1, file2)) return true;

    if(stat(file1, &s1) || stat(file2, &s2)) return false;

    return (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino);
}

int
cssWriteDotw(CssTableClass *t, const char *wfdisc_file, const char *wa,
	const char *dotw, const char *da, float *y, const char *sta,
	const char *chan, double time, long chanid, int nsamp, double samprate,
	double calib, double calper, const char *remark, long wfid)
{
	char		rmk[81], error[MAXPATHLEN+100];
	const char	*err_msg;
	int		n;
	long		commid;
	CssWfdiscClass	w;
	FILE		*fp;
	struct stat	buf;
	DateTime	dt;
	bool		big_endian;
	union
        {
		char	a[4];
		float	f;
		int	i;
		short	s;
	} e1;

	e1.a[0] = 0; e1.a[1] = 0;
	e1.a[2] = 0; e1.a[3] = 1;
	big_endian = (e1.i == 1) ? true : false;

	timeLoadDate();

	stringcpy(w.sta, sta, sizeof(w.sta));
	stringcpy(w.chan, chan, sizeof(w.chan));
	w.time = time;
	timeEpochToDate(w.time, &dt);
	w.jdate = timeJDate(&dt);
	w.chanid = chanid;
	w.nsamp = nsamp;
	w.calib = calib;
	w.calper = calper;
	w.samprate = (samprate > 0.) ? samprate : 1.;
	w.endtime = time + (nsamp-1)/w.samprate;
	w.wfid = (wfid > 0) ? wfid : cssGetNextId(t, "wfid", NULL);
	wfid = w.wfid;
	if(big_endian) {
	    stringcpy(w.datatype, "t4", sizeof(w.datatype));
	}
	else {
	    stringcpy(w.datatype, "f4", sizeof(w.datatype));
	}
	timeEpochToDate(timeGetEpoch(), &w.lddate);
	dt = w.lddate;

	w.commid = (remark != NULL && remark[0] != '\0') ?
			cssGetNextId(t, "commid", NULL) : -1;
	commid = w.commid;

	if(wa[0] == 'a' && !stat(dotw, &buf)) {
	    w.foff = buf.st_size;
	}

	n = strlen(dotw);
	while(n > 0 && dotw[n-1] != '/') n--;

	if(n == 0) {
	    stringcpy(w.dir, ".", sizeof(w.dir));
	}
	else {
	    strncpy(w.dir, dotw, n);
	    w.dir[n] = '\0';
	}
	stringcpy(w.dfile, dotw+n, sizeof(w.dfile));

	if((fp = fopen(wfdisc_file, wa)) == NULL)
	{
	    open_warn(wfdisc_file);
	    return(0);
	}
	if(w.write(fp, &err_msg))
	{
	    fclose(fp);
	    return(0);
	}
	fclose(fp);

	if((fp = fopen(dotw, da)) == NULL)
	{
	    open_warn(dotw);
	    return(0);
	}

	if(fwrite(y, sizeof(float), (size_t)nsamp, fp) != (size_t)nsamp)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error), "write error: %s\n%s", dotw,
				strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", dotw);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return(0);
	}
	fclose(fp);

        if(remark != NULL && remark[0] != '\0')
        {
	    char remark_file[MAXPATHLEN+1];

	    stringcpy(remark_file, wfdisc_file, MAXPATHLEN+1);

	    n = strlen(remark_file);

	    stringcpy(remark_file+n-7, ".remark", MAXPATHLEN+1-n-7);

	    if((fp = fopen(remark_file, wa)) == NULL)
	    {
		open_warn(remark_file);
		return(0);
	    }
	    if((int)strlen(remark) <= 80)
	    {
		fprintf(fp, "%8ld %8d %-80.80s %-17.17s\n",
			commid, 0, remark, timeDateString(&dt));
	    }
	    else
	    {
		stringcpy(rmk, remark, sizeof(remark));
		fprintf(fp, "%8ld %8d %-80.80s %-17.17s\n",
			commid, 0, rmk, timeDateString(&dt));
	    }
	    fclose(fp);
        }
	return(wfid);
}

bool
cssAddNetmag(CssNetmagClass *netmag, CssTableClass *table)
{
	char		file[MAXPATHLEN+1], error[MAXPATHLEN+100];
	char		full_prefix[MAXPATHLEN+1];
	const char	*dir, *prefix, *sep, *err_msg;
	int		len;
	FILE		*fp;

	netmag->setIds(table->getDC(), 1);
	netmag->setFormat(table->getFormat());

	netmag->setDir(table->getDir());
	netmag->setPrefix(table->getPrefix());

	dir = quarkToString(table->getDir());
	prefix = quarkToString(table->getPrefix());

	if(dir) {
	    len = strlen(dir);
	    sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
	    snprintf(file, sizeof(file), "%s%s%s.netmag", dir, sep, prefix);
	    snprintf(full_prefix, sizeof(full_prefix), "%s%s%s",
			dir, sep, prefix);
	}
	else if(prefix) {
	    snprintf(file, sizeof(file), "%s.netmag", prefix);
	    snprintf(full_prefix, sizeof(full_prefix), "%s", prefix);
	}
	else {
	    snprintf(error, sizeof(error),
		"cssAddNetmag: no path information.");
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	netmag->magid = cssGetNextId(netmag, "magid", full_prefix);

	netmag->setFile(stringToQuark(file));

	/* don't use a+ first, since on linux, ftell(fp) will give 0 for
	 * the file position, even if the file size is not zero.
	 */
	if((fp = fopen(file, "r+")) == NULL
		&& (fp = fopen(file, "a+")) == NULL)
	{
	    if(errno > 0)
	    {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo netmag changes will be saved.",
				file, strerror(errno));
	    }
	    else
	    {
		snprintf(error, sizeof(error),
		      "Cannot write to: %s\nNo netmag changes will be saved.",
			file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	}
	else
	{
	    fseek(fp, 0, 2);

	    netmag->setFileOffset(ftell(fp));

	    if(netmag->write(fp, &err_msg))
	    {
		snprintf(error, sizeof(error), "write error: %s", file);
		logErrorMsg(LOG_WARNING, error);
	    }
	    fclose(fp);
	}
	return true;
}


/* stamag coming into here should already be initialized and filled */

bool
cssAddStamag(CssStamagClass *stamag, CssTableClass *table)
{
	char		file[MAXPATHLEN+1], error[MAXPATHLEN+100];
	const char	*dir, *prefix, *sep, *err_msg;
	int		len;
	FILE		*fp;

	stamag->setIds(table->getDC(), 1);
	stamag->setFormat(table->getFormat());
	stamag->setDir(table->getDir());
	stamag->setPrefix(table->getPrefix());

	dir = quarkToString(table->getDir());
	prefix = quarkToString(table->getPrefix());

	if(dir) {
	    len = strlen(dir);
	    sep = (len > 0 && dir[len-1] != '/') ? "/" : "";
	    snprintf(file, sizeof(file), "%s%s%s.stamag", dir, sep, prefix);
	}
	else if(prefix) {
	    snprintf(file, sizeof(file), "%s.stamag", prefix);
	}
	else {
	    snprintf(error, sizeof(error),
		"cssAddStamag: no path information.");
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}

	stamag->setFile(stringToQuark(file));

	/* don't use a+ first, since on linux, ftell(fp) will give 0 for
	 * the file position, even if the file size is not zero.
	 */
	if((fp = fopen(file, "r+")) == NULL
		&& (fp = fopen(file, "a+")) == NULL)
	{
	    if(errno > 0)
	    {
		snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo stamag changes will be saved.",
				file, strerror(errno));
	    }
	    else
	    {
		snprintf(error, sizeof(error),
		      "Cannot write to: %s\nNo stamag changes will be saved.",
			file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	}
	else
	{
	    fseek(fp, 0, 2);

	    stamag->setFileOffset(ftell(fp));

	    if(stamag->write(fp, &err_msg))
	    {
		snprintf(error, sizeof(error), "write error: %s", file);
		logErrorMsg(LOG_WARNING, error);
	    }
	    fclose(fp);
	}
	return true;
}

static int get_id(FILE *fp, const char *filename, const char *keyname, int id);

int
cssGetNextId(CssTableClass *css, const char *keyname, const char *prefix, long maxid)
{
	char		filename[MAXPATHLEN+1];
	string		lastid_table;
	FILE		*fp;
	struct stat	buf;
	int		format;
	int		id = 0;

	format = (css != NULL) ? css->getFormat() : 0;
	if(format == stringToQuark("ffdb"))
	{
	    FFDatabase *ffdb;
	    int source_q, param_root_q, seg_root_q, structure_q;
	    const char *param_root, *seg_root, *dir_structure;
	    double duration;

	    css->getSource(&source_q, &param_root_q, &seg_root_q);
	    css->getDirectoryStructure(&structure_q, &duration);
	    param_root = quarkToString(param_root_q);
	    seg_root = quarkToString(seg_root_q);
	    dir_structure = quarkToString(structure_q);
	    ffdb = FFDatabase::FFDBOpen(param_root, seg_root, dir_structure,
				duration);
	    if(!ffdb) {
		logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
		return maxid + 1;
	    }
	    id = ffdb->getNextId("lastid", keyname);
	    delete ffdb;

	    if(id < 0) {
		logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	    }
	    return id;
	}
	else if(format == stringToQuark("odbc"))
	{
#ifdef HAVE_LIBODBC
	    SQLHDBC hdbc;
	    int source_q, user_q, passwd_q;
	    const char *source, *user, *passwd;

	    css->getSource(&source_q, &user_q, &passwd_q);
	    source = quarkToString(source_q);
	    user = quarkToString(user_q);
	    passwd = quarkToString(passwd_q);

	    hdbc = ODBCConnect(source, user, passwd, 0);
	    if(!hdbc) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return maxid + 1;
	    }
	    id = ODBCGetNextId(hdbc, "lastid", keyname);
	    ODBCDisconnect(hdbc);
	    if(id < 0) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
#endif
	    return id;
	}

	if( Application::getProperty("lastidTable", lastid_table) )
	{
	    if(stat(lastid_table.c_str(), &buf) == 0)
	    {
		fp = fopen(lastid_table.c_str(), "r+");
	    }
	    else
	    {
		fp = fopen(lastid_table.c_str(), "w+");
	    }
	    if(fp == NULL)
	    {
		char error[MAXPATHLEN+200];
		if(errno > 0) {
		    snprintf(error, sizeof(error), "Cannot open: %s\n%s",
				lastid_table.c_str(), strerror(errno));
		}
		else {
		    snprintf(error, sizeof(error), "Cannot open: %s",
				lastid_table.c_str());
		}
		logErrorMsg(LOG_WARNING, error);
	    }
	    else
	    {
		return(get_id(fp, "lastid", keyname, id));
	    }
	}

	/* get prefix 
	 */
	if (prefix == NULL)
	{
	    return maxid + 1;
	}
	snprintf(filename, sizeof(filename), "%s.lastid", prefix);

	if(stat(filename, &buf) == 0)
	{
	    fp = fopen(filename, "r+");
	}
	else
	{
	    fp = fopen(filename, "w+");
	}
	if(fp == NULL)
	{
	    char error[MAXPATHLEN+200];
	    if(errno > 0) {
		snprintf(error, sizeof(error),
		    "Cannot open: %s\n%s", filename, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error), "Cannot open: %s", filename);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return(maxid+1);
	}
	else
	{
	    long lid = get_id(fp, filename, keyname, id);
	
	    if(maxid >= 0 && maxid >= lid) return maxid+1;
	    return lid;
	}
}

static int
get_id(FILE *fp, const char *filename, const char *keyname, int id)
{
	const char	*err_msg=NULL;
	int		rec, err, keyvalue;
	CssLastidClass	lastid;

	fseek(fp, 0, 0);

	rec = 0;
	while(!(err = lastid.read(fp, &err_msg)))
	{
	    if(!strcmp(lastid.keyname, keyname)) break;
	    rec++;
	}

	if(err == 0)
	{
	    /* found keyname
	     */
	    fseek(fp, rec*(lastid.getLineLength()+1), 0);	
	    if(id > lastid.keyvalue)
	    {
		lastid.keyvalue = id;
	    }
	}
	else if(err == EOF)
	{
	    lastid.keyvalue = id;
	    stringcpy(lastid.keyname, keyname, sizeof(lastid.keyname));
	}
	else
	{
	    char error[MAXPATHLEN+200];
	    snprintf(error, sizeof(error), "format error in%s\n%s",
			filename, cssioGetErrorMsg());
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return(-1);
	}
	lastid.keyvalue++;

	timeEpochToDate(timeGetEpoch(), &lastid.lddate);
	err_msg = NULL;
	if(lastid.write(fp, &err_msg))
	{
	    char error[MAXPATHLEN+200];
	    if(err_msg) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
			filename, err_msg);
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", filename);
	    }
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);

	keyvalue = lastid.keyvalue;

	return(keyvalue);
}

#define INSERT	1
#define REPLACE	2
#define PARAM	3
#define BACKUP	4
#define RECORD	5

typedef struct
{
	int	pos;
	int	type;
	int	len;
} BackupLine;

static int getLineTypes(FILE *fp, BackupLine **l);
static bool processBackup(FILE *fp, const char *backup_file, int nlines,
		BackupLine *line, int backup);
static int undoLine(FILE *fp, BackupLine *line, int i, const char *backup_file,
		char *fileline, BackupLine *rec);
static int insertRecord(FILE *fpo, const char *file, int j1, int j2,
		int position, char *record, int record_len);
static int replaceRecord(FILE *fpo, const char *file, char *record,
		int record_len);
static int getRecord(FILE *fp, const char *backup_file, char **record,
			BackupLine *rec, int *record_len);

int
cssUndoFileModification(const char *backup_file)
{
	char		error[MAXPATHLEN+100];
	int		i, backup, nlines;
	BackupLine	*line = NULL;
	struct stat 	stat_buf;
	FILE		*fp;

	if(stat(backup_file, &stat_buf)
		|| (fp = fopen(backup_file, "r+")) == NULL)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
			"Cannot open : %s\n%s\nUndo failed.\n",
			backup_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
			"Cannot open : %s\nUndo failed.\n", backup_file);
            }
	    logErrorMsg(LOG_WARNING, error);
	    return(-1);
	}

	if((nlines = getLineTypes(fp, &line)) == 0) {
	    fclose(fp);
	    snprintf(error, sizeof(error),
			"Format error in %s\nUndo failed.\n", backup_file);
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}

	for(i = 0; i < nlines-1; i++) {
	    line[i].len = line[i+1].pos - line[i].pos;
	}
	line[nlines-1].len = stat_buf.st_size - line[nlines-1].pos;

	/* find the beginning of the last backup
	 */
	for(backup = nlines-1; backup >= 0; backup--) {
	    if(line[backup].type == BACKUP) break;
	}
	if(backup == -1) {
	    fclose(fp); Free(line);
	    snprintf(error, sizeof(error),
			"backup not found in %s\nUndo failed.", backup_file);
	    logErrorMsg(LOG_WARNING, error);
	    return 0;
	}

	processBackup(fp, backup_file, nlines, line, backup);

	fclose(fp);

	truncate(backup_file, line[backup].pos);

	Free(line);

	return 1;
}

static int
getLineTypes(FILE *fp, BackupLine **l)
{
	int c, i, lastc, nlines;
	BackupLine *line = NULL;
	char		buf[21];

	/* identify the type of each line in the backup file.
	 */
	*l = NULL;
	nlines = 0;
	lastc = '\n';
	while((c = getc(fp)) != EOF)
	{
	    if(lastc == '\n')
	    {
		if(!(line = (BackupLine *)reallocWarn(line,
			(nlines+1)*sizeof(BackupLine)))) {
		    return 0;
		}
		line[nlines].pos = ftell(fp) - 1;

		buf[0] = c;

		i = 1;
		while((c = getc(fp)) != EOF && !isspace(c) && i < 20) {
		    buf[i++] = c;
		}
		buf[i] = '\0';

		if(!strcmp(buf, "insert")) {
		    line[nlines].type = INSERT;
		}
		else if(!strcmp(buf, "replace")) {
		    line[nlines].type = REPLACE;
		}
		else if(!strcmp(buf, "record")) {
		    line[nlines].type = RECORD;
		}
		else if(buf[0] == '#') {
		    line[nlines].type = BACKUP;
		}
		else {
		    line[nlines].type = 0;
		}
		nlines++;
	    }
	    lastc = c;
	}
	*l = line;
	return nlines;
}

static bool
processBackup(FILE *fp, const char *backup_file, int nlines,
		BackupLine *line, int backup)
{
	int		i, lasti, j;
	BackupLine	*rec;
	char		*fileline = NULL;
	char		error[MAXPATHLEN+100];

	/* process the last backup, in reverse order
	 */
	lasti = nlines;

	for(i = nlines-1; i > backup; i--)
	    if(line[i].type == INSERT || line[i].type == REPLACE)
	{
	    for(j = i+1; j < lasti; j++) {
		if(line[j].type == RECORD) break;
	    }
	    if(j == lasti) {
		snprintf(error, sizeof(error),
			"Format error error: %s\nRecord expected at line %d",
			backup_file, i+2);
		logErrorMsg(LOG_WARNING, error);
		Free(fileline);
		return false;
	    }
	    rec = line + j;
	    lasti = i;

	    if(!(fileline = (char *)reallocWarn(fileline, line[i].len+1))) {
		return false;
	    }

	    if(fseek(fp, line[i].pos, 0) ||
		fread(fileline, 1, line[i].len, fp) != (size_t)line[i].len)
	    {
		snprintf(error, sizeof(error), "Read error: %s", backup_file);
		logErrorMsg(LOG_WARNING, error);
		Free(fileline);
		return false;
	    }
	    fileline[line[i].len] = '\0';

	    if((line[i].type == INSERT && !strncmp(fileline, "insert ", 7)) ||
	       (line[i].type == REPLACE && !strncmp(fileline, "replace ", 8)))
	    {
		if(undoLine(fp, line, i, backup_file, fileline, rec)) {
		    Free(fileline);
		    return false;
		}
	    }

	}
	Free(fileline);
	return true;
}

static int
undoLine(FILE *fp, BackupLine *line, int i, const char *backup_file,
		char *fileline, BackupLine *rec)
{
	int		j, j1, j2;
	int		position;
	int		record_len;
	char	*record = NULL;
	char 		*file, error[MAXPATHLEN+100];
	struct stat 	stat_buf;
	FILE		*fpo;

	for(j = 7; fileline[j] != '\0' && isspace((int)fileline[j]); j++);
	file = fileline + j;

	if(*file == '\0') {
	    snprintf(error, sizeof(error),
		"Read error: %s\nNo file on line %d", backup_file, i+1);
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}

	for(j = 0; file[j] != '\0' && !isspace((int)file[j]); j++);
	if(file[j] == '\0' || sscanf(file+j, "%d", &position) != 1) {
	    snprintf(error, sizeof(error), "Formar error: %s\nLine %d",
			backup_file, i+1);
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}
	file[j] = '\0';

	if(getRecord(fp, backup_file, &record, rec, &record_len)) {
	    Free(record);
	    return -1;
	}
	
	if(stat(file, &stat_buf) < 0 || (fpo = fopen(file, "r+")) == NULL)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
			"Cannot open : %s\n%s\nUndo failed.\n",
			file, strerror(errno));
		logErrorMsg(LOG_WARNING, error);
	    }
	    else {
		snprintf(error, sizeof(error),
			"Cannot open : %s\nUndo failed.\n", file);
		logErrorMsg(LOG_WARNING, error);
	    }
	    Free(record);
	    return -1;
	}

	if(fseek(fpo, position, 0))
	{
	    snprintf(error, sizeof(error), "Read error: %s", file);
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fpo);
	    Free(record);
	    return -1;
	}
	if(line[i].type == INSERT)
	{
	    j2 = stat_buf.st_size;
	    j1 = j2 - 512;
	    if(j1 < position) j1 = position;

	    if(insertRecord(fpo, file, j1, j2, position, record, record_len))
	    {
		fclose(fpo);
		Free(record);
		return -1;
	    }
	}
	else if(line[i].type == REPLACE)
	{
	    if(replaceRecord(fpo, file, record, record_len)) {
		fclose(fpo);
		Free(record);
		return -1;
	    }
	}
	fclose(fpo);
	Free(record);

	return 0;
}

static int
getRecord(FILE *fp, const char *backup_file, char **record,
		BackupLine *rec, int *record_len)
{
	char error[MAXPATHLEN+100];

	if(!(*record = (char *)mallocWarn(rec->len+1))) {
	    return -1;
	}
	if(fseek(fp, rec->pos, 0) || fread(*record, 1, rec->len,fp) !=
		(size_t)rec->len)
	{
	    snprintf(error, sizeof(error), "Read error: %s", backup_file);
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}
	(*record)[rec->len] = '\0';
	if(strncmp(*record, "record ", 7)) {
	    snprintf(error, sizeof(error), "Format error: %s\n", backup_file);
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}
	*record_len = rec->len - 7;
	return 0;
}

static int
insertRecord(FILE *fpo, const char *file, int j1, int j2, int position,
		char *record, int record_len)
{
	int	n;
	char	tmp[5000];
	char	error[MAXPATHLEN+100];

	while(j2 >= j1)
	{
	    n = j2 - j1;
	    if(fseek(fpo, j1, 0) || fread(tmp, 1, n, fpo) != (size_t)n)
	    {
		if(errno > 0) {
		    snprintf(error, sizeof(error),
			"Read error: %s\n%s\nUndo failed.\n",
			file, strerror(errno));
		}
		else {
		    snprintf(error, sizeof(error),
				"Read error: %s\nUndo failed.\n", file);
		}
		logErrorMsg(LOG_WARNING, error);
		return -1;
	    }
	    if(fseek(fpo, (long)(j1+record_len), 0) ||
		fwrite(tmp,1,n,fpo) != (size_t)n)
	    {
		if(errno > 0) {
		    snprintf(error, sizeof(error),
			"Write error: %s\n%s\nUndo failed.\n",
			file, strerror(errno));
		}
		else {
		    snprintf(error, sizeof(error),
				"Write error: %s\nUndo failed.\n", file);
		}
		logErrorMsg(LOG_WARNING, error);
		return -1;
	    }
	    fflush(fpo);
	    j2 -= 512;
	    j1 -= 512;
	    if(j1 < position) j1 = position;
	}
	if(fseek(fpo, position, 0) ||
	    fwrite(record+7, 1, record_len, fpo) != (size_t)record_len)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
			"Write error: %s\n%s\nUndo failed.\n",
			file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
			"Write error: %s\nUndo failed.\n", file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}
	return 0;
}


static int
replaceRecord(FILE *fpo, const char *file, char *record, int record_len)
{
	if(fwrite(record+7, 1, record_len, fpo) != (size_t)record_len)
	{
	    char error[MAXPATHLEN+100];
	    if(errno > 0) {
		snprintf(error, sizeof(error),
			"Write error: %s\n%s\nUndo failed.\n",
			file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error),
			"Write error: %s\nUndo failed.\n", file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    return -1;
	}
	return 0;
}
