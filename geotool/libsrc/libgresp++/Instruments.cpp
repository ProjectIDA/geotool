/** \file Instruments.cpp
 *  \brief Defines class ResponseFile.
 *  \author Ivan Henson
 */
/*
 * NAME
 *      Response_getFile, Response_getInstrument
 *
 * AUTHOR
 *      I. Henson		
 *      Teledyne Geotech	
 *
 *      J. Coyne
 *      SAIC
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string>

#include "ResponseFile.h"
#include "gobject++/DataSource.h"
#include "motif++/Application.h"
#include "gobject++/CssTables.h"
#include "libgdb.h"
#include "cssio.h"
extern "C" {
#include "libtime.h"
}

static char	msg[10000];

class GlobalTable
{
    public:
    bool		opened;
    struct stat		sensor_stat;
    string		sensor_file;
    gvector<CssTableClass *>	sensor;
    struct stat		instrument_stat;
    string		instrument_file;
    gvector<CssTableClass *>	instrument;

    GlobalTable(void) {
	opened = false;
    }
};

static GlobalTable local, local2, global, global2;

#ifdef HAVE_LIBODBC 
static int readAllRows(SQLHDBC hdbc, const string &account,
		const string &table_name, const string &css_type,
		gvector<CssTableClass *> &tables, const char **err_msg);
#endif
static void readLocalTables(const string &dir, const string &prefix);
static void readLocalTables2(void);
static void readGlobalTables(void);
static void getNames(string s, CssInstrumentClass *instrument, string &insname,
		string &file, int *inid);

gvector<CssTableClass *> &getATable(void) { return local.sensor; }

/**
 * Get the instrument name and the response filename. The sensor table and the
 * instrument table are searched for the records that link to the input
 * sta,chan,time, and endtime. First, the files obtained from the program
 * resources "sensor_table", "instrument_table", "sensor_table2",
 * "instrument_table2" are searched. If the instrument record is not found, and
 * an ODBC connection has been made, then the program property "dbAccount",
 * "sensorDbTable" and "instrumentDbTable" will be used to search the database.

 * The following program properties are used:
 *	- sensorTable
 *	- sensorTable2
 *	- instrumentTable
 *	- instrumentTable2
 *	- dbAccount
 *	- sensorDbTable
 *	- dbAccount
 *	- instrumentDbTable
 * 
 * @param[in] sta the station name
 * @param[in] chan the channel name
 * @param[in] time the beginning of the time period of interest.
 * @param[in] endtime the end of the time period of interest.
 * @param[out] insname the instrument name for the waveform.
 * @param[out] file the response filename for the waveform.
 * @param[out] inid the inid from the instrument table
 * @param err_msg on error return, *err_msg is set to a static character \
 *	string error message.
 * @return true for success, false for error.
 */
bool
ResponseFile::getFile(const string &sta, const string &chan, double time,
		double endtime, string &insname, string &file, int *inid,
		const char **err_msg)
{
    string dir, prefix;
    return getFile(sta, chan, time, endtime, dir, prefix, insname,
			file, inid, err_msg);
}

/**
 * Get the instrument name and the response filename. The sensor table and the
 * instrument table are searched for the records that link to the input sta,
 * chan,time and endtime. If the directory <b>dir</b> and file <b>prefix</b> are
 * not NULL, then the sensor file dir/prefix.sensor and the instrument file
 * dir/prefix.instrument will be searched. If <b>dir</b> and <b>prefix</b> are
 * NULL, or they do not contain the required records for the input sta,chan,
 * then the file names will be obtained from the program resources
 * "sensor_table", "instrument_table", "sensor_table2", "instrument_table2".
 * If the instrument record is still not found, and an ODBC connection has
 * been made, then the program property "dbAccount", "sensorDbTable" and
 * "instrumentDbTable" will be used to search the database.

 * The following program properties are used:
 *	- sensorTable
 *	- sensorTable2
 *	- instrumentTable
 *	- instrumentTable2
 *	- dbAccount
 *	- sensorDbTable
 *	- dbAccount
 *	- instrumentDbTable
 * 
 * @param[in] sta the station name
 * @param[in] chan the channel name
 * @param[in] time the beginning time
 * @param[in] endtime the end time
 * @param[in] dir the wfdisc directory or NULL.
 * @param[in] prefix the wfdisc prefix or NULL.
 * @param[out] insname the instrument name for the waveform.
 * @param[in] insname_len the size of insname.
 * @param[out] file the response filename for the waveform.
 * @param[out] inid the inid from the instrument table
 * @param err_msg on error return, *err_msg is set to a static character \
 *	string error message.
 * @returns true for success, false for error.
 */
bool
ResponseFile::getFile(const string &sta, const string &chan, double time,
		double endtime, const string &dir, const string &prefix,
		string &insname, string &file, int *inid, const char **err_msg)
{
    int	i, len;
    char tim[20];
    CssSensorClass *s, *sensor = NULL;
    CssInstrumentClass *instrument = NULL;
    int	sta_q;
	
    *err_msg = NULL;
    msg[0] = '\0';
    insname.clear();
    file.clear();

    sta_q = stringUpperToQuark(sta);

    readLocalTables(dir, prefix);

    if(local.opened && local.sensor.size() > 0) {
	for(i = 0; i < local.sensor.size(); i++)
	{
	    s = (CssSensorClass *)local.sensor[i];
	    if(sta_q == s->sta_quark && DataSource::compareChan(chan, s->chan)
			&& time > s->time && endtime < s->endtime) break;
	}
	if(i < local.sensor.size()) {
	    sensor = s;
	    snprintf(msg, sizeof(msg), "%s/%s sensor.inid=%ld found in %s",
		sensor->sta, sensor->chan, sensor->inid,
		local.sensor_file.c_str());
	}
	else {
	    snprintf(msg, sizeof(msg), "%s/%s %s sensor not found in\n%s",
		sta.c_str(), chan.c_str(), timeEpochToString(time,tim,20,YMOND),
		local.sensor_file.c_str());
	}
	if(sensor && local.instrument.size() > 0)
	{
	    for(i = 0; i < local.instrument.size() && sensor->inid !=
		((CssInstrumentClass *)local.instrument[i])->inid; i++);

	    if(i < local.instrument.size()) {
		instrument = (CssInstrumentClass *)local.instrument[i];
	    }
	    else {
		len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len,
		    "\n%s/%s instrument.inid=%ld not found in %s", sensor->sta,
		    sensor->chan, sensor->inid, local.instrument_file.c_str());
	    }
	}
    }

    if(instrument) {
	getNames(local.instrument_file, instrument, insname, file, inid);
	return true;
    }

    readLocalTables2();

    if(!sensor && local2.opened && local2.sensor.size() > 0) {
	for(i = 0; i < local2.sensor.size(); i++)
	{
	    s = (CssSensorClass *)local2.sensor[i];
	    if(sta_q == s->sta_quark && DataSource::compareChan(chan, s->chan)
			&& time > s->time && endtime < s->endtime) break;
	}
	if(i < local2.sensor.size()) {
	    sensor = s;
	    snprintf(msg, sizeof(msg), "%s/%s sensor.inid=%ld found in %s",
		sensor->sta, sensor->chan, sensor->inid,
		local2.sensor_file.c_str());
	}
	else {
	    snprintf(msg, sizeof(msg), "%s/%s %s sensor not found in\n%s",
		sta.c_str(), chan.c_str(), timeEpochToString(time,tim,20,YMOND),
		local2.sensor_file.c_str());
	}
	if(sensor && local2.instrument.size() > 0)
	{
	    for(i = 0; i < local2.instrument.size() && sensor->inid !=
		((CssInstrumentClass *)local2.instrument[i])->inid; i++);

	    if(i < local2.instrument.size()) {
		instrument = (CssInstrumentClass *)local2.instrument[i];
	    }
	    else {
		len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len,
		    "\n%s/%s instrument.inid=%ld not found in %s",
		    sensor->sta, sensor->chan, sensor->inid,
		    local2.instrument_file.c_str());
	    }
	}
    }

    if(instrument) {
	getNames(local2.instrument_file, instrument, insname, file, inid);
	return true;
    }

    readGlobalTables();

    if(!sensor && global.sensor.size() > 0) {
	for(i = 0; i < global.sensor.size(); i++)
	{
	    s = (CssSensorClass *)global.sensor[i];
	    if(sta_q == s->sta_quark && DataSource::compareChan(chan, s->chan)
			&& time > s->time && endtime < s->endtime) break;
	}
	if(i < global.sensor.size()) {
	    sensor = s;
	    snprintf(msg, sizeof(msg), "%s/%s sensor.inid=%ld found in %s",
		sensor->sta, sensor->chan, sensor->inid,
		global.sensor_file.c_str());
	}
	else {
	    snprintf(msg, sizeof(msg), "%s/%s %s sensor not found in\n%s",
		sta.c_str(), chan.c_str(), timeEpochToString(time,tim,20,YMOND),
		global.sensor_file.c_str());
	}
    }

    if(sensor && global.instrument.size() > 0)
    {
	for(i = 0; i < global.instrument.size() && sensor->inid !=
		((CssInstrumentClass *)global.instrument[i])->inid; i++);

	if(i < global.instrument.size()) {
	    instrument = (CssInstrumentClass *)global.instrument[i];
	}
	else {
	    len = (int)strlen(msg);
	    snprintf(msg+len, sizeof(msg)-len,
		"\n%s/%s instrument.inid=%ld not found in %s",
		sensor->sta, sensor->chan, sensor->inid,
		global.instrument_file.c_str());
	}
    }

    if(instrument) {
	getNames(global.instrument_file, instrument, insname, file, inid);
	return true;
    }

#ifdef HAVE_LIBODBC
    if(!instrument)
    {
	bool open_instrument = false;
	SQLHDBC hdbc = ODBCGetLastConnection();

	if( !global2.opened )
	{
	    string dbAccount, table_name;
	    Application::getProperty("dbAccount", dbAccount);
	    Application::getProperty("sensorDbTable", table_name);
	    if(hdbc != NULL && !dbAccount.empty() && !table_name.empty())
	    {
		readAllRows(hdbc, dbAccount, table_name, "SENSOR",
				global2.sensor, err_msg);
	    }
	    global2.opened = true;
	    open_instrument = true;
	}

	for(i = 0; i < global2.sensor.size(); i++)
	{
	    s = (CssSensorClass *)global2.sensor[i];
	    if(sta_q == s->sta_quark && DataSource::compareChan(chan, s->chan)
			&& time > s->time && endtime < s->endtime) break;
	}

	if(i < global2.sensor.size())
	{
	    int j;
	    if(open_instrument)
	    {
		string dbAccount, table_name;
		Application::getProperty("dbAccount", dbAccount);
		Application::getProperty("instrumentDbTable", table_name);
		if(hdbc != NULL && !dbAccount.empty() && !table_name.empty())
		{
		    readAllRows(hdbc, dbAccount, table_name, "INSTRUMENT",
					global2.instrument, err_msg);
		}
	    }

	    s = (CssSensorClass *)global2.sensor[i];
	    for(j = 0; j < global2.instrument.size() && s->inid !=
		((CssInstrumentClass *)global2.instrument[j])->inid;j++);
	    if(j < global2.instrument.size()) {
		 instrument = (CssInstrumentClass *)global2.instrument[j];
	    }
	}
    }
    if(instrument) {
	insname.assign(instrument->insname);
	*inid = instrument->inid;
	return true;
    }
#endif /* HAVE_LIBODBC */

    *err_msg = msg;
    return false;
}

static void
getNames(string s, CssInstrumentClass *instrument, string &insname, string &file,
		int *inid)
{
    char filename[MAXPATHLEN+1], *path=NULL;
    int i, n;

    strncpy(filename, s.c_str(), sizeof(filename));

    /* check for no directory or a relative directory
    */
    if(!strcmp(instrument->dir, "-") || !strcmp(instrument->dir, "_") ||
	    instrument->dir[0] != '/')
    {
	for(i = (int)strlen(filename)-1; i > 0 && filename[i]!='/'; i--);
	filename[i+1] = '\0';
	if(instrument->dir[0] != '/')
	{
	    n = (int)strlen(filename);
	    snprintf(filename+n, sizeof(filename)-n, "%s", instrument->dir);
	}
    }
    else
    {
	snprintf(filename, sizeof(filename), "%s", instrument->dir);
    }
    n = (int)strlen(filename);
    if(filename[n-1] != '/') {
	snprintf(filename+n, sizeof(filename)-n, "/%s", instrument->dfile);
    }
    else {
	snprintf(filename+n, sizeof(filename)-n, "%s", instrument->dfile);
    }

    if((path = TextField::cleanPath(filename))) {
	file.assign(path);
	Free(path);
    }

    insname.assign(instrument->insname);

    *inid = instrument->inid;
}

static void
readLocalTables(const string &dir, const string &prefix)
{
    int i, path_q;
    char dr[MAXPATHLEN+1], path[MAXPATHLEN+1], *s;
    CssInstrumentClass *ins;
    const char *err;
    struct stat buf;

    if(dir.empty() || prefix.empty()) return;

    snprintf(path, sizeof(path), "%s/%s.sensor", dir.c_str(), prefix.c_str());

    if(!stat(path, &buf))
    {
	if(CssTableClass::readFile(path, &local.sensor_stat, "sensor",
			local.sensor, &err) < -1)
	{
	    if(err) {
		int len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len, "\n%s", err);
	    }
	}
	else {
	    local.sensor_file.assign(path);
	}
    }
    snprintf(path, MAXPATHLEN+1, "%s/%s.instrument",dir.c_str(),prefix.c_str());

    if(!stat(path, &buf))
    {
	if(CssTableClass::readFile(path, &local.instrument_stat, "instrument",
			local.instrument, &err) < -1)
	{
	    if(err) {
		int len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len, "\n%s", err);
	    }
	}
	else {
	    s = TextField::getAbsolutePath(path);
	    local.instrument_file.assign(s);
	    Free(s);

	    strncpy(dr, local.instrument_file.c_str(), sizeof(dr));
	    for(i = (int)strlen(dr)-1; i > 0 && dr[i] != '/'; i--);
	    dr[i+1] = '\0';

	    for(i = 0; i < local.instrument.size(); i++) {
		ins = (CssInstrumentClass *)local.instrument[i];
		if(ins->dir[0] != '/')
		{
		    snprintf(path, sizeof(path), "%s/%s/%s",
				dr, ins->dir, ins->dfile);
		}
		else {
		    snprintf(path, sizeof(path), "%s/%s", ins->dir, ins->dfile);
		}
		if((s = TextField::cleanPath(path))) {
		    path_q = stringToQuark(s);
		    ins->putValue("dfile", (long)path_q);
		    Free(s);
		}
	    }
	}
    }
    local.opened = true;
}

static void
readLocalTables2(void)
{
    const char *err;
    int i, path_q;
    char dir[MAXPATHLEN+1], path[MAXPATHLEN+1];
    CssInstrumentClass *ins;
    struct stat buf;
    string sensor_file, instrument_file;
    Application::getProperty("sensorTable2", sensor_file);
    Application::getProperty("instrumentTable2", instrument_file);

    if(!sensor_file.empty() && !stat(sensor_file.c_str(), &buf))
    {
	if(CssTableClass::readFile(sensor_file, &local2.sensor_stat, "sensor",
			local2.sensor, &err) < -1)
	{
	    if(err) {
		int len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len, "\n%s", err);
	    }
	}
	else {
	    local2.sensor_file = sensor_file;
	}
    }

    if(!instrument_file.empty() && !stat(instrument_file.c_str(), &buf))
    {
	if(CssTableClass::readFile(instrument_file, &local2.instrument_stat,
			"instrument", local2.instrument, &err) < -1)
	{
	    if(err) {
		int len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len, "\n%s", err);
	    }
	}
	else {
	    string s;
	    TextField::getAbsolutePath(instrument_file, local2.instrument_file);

	    strncpy(dir, local2.instrument_file.c_str(), sizeof(dir));
	    for(i = (int)strlen(dir)-1; i > 0 && dir[i] != '/'; i--);
	    dir[i+1] = '\0';

	    for(i = 0; i < local2.instrument.size(); i++) {
		ins = (CssInstrumentClass *)local2.instrument[i];
		if(ins->dir[0] != '/')
		{
		    snprintf(path, sizeof(path), "%s/%s/%s",
				dir, ins->dir, ins->dfile);
		}
		else {
		    snprintf(path, sizeof(path), "%s/%s", ins->dir, ins->dfile);
		}
		if(TextField::cleanPath(path, s)) {
		    path_q = stringToQuark(s);
		    ins->putValue("dfile", (long)path_q);
		}
	    }
	}
    }
    local2.opened = true;
}

static void
readGlobalTables(void)
{
    const char *err;
    int i, path_q;
    char dir[MAXPATHLEN+1], path[MAXPATHLEN+1];
    CssInstrumentClass *ins;
    struct stat buf;
    string sensor_file, instrument_file;
    Application::getProperty("sensorTable", sensor_file);
    Application::getProperty("instrumentTable", instrument_file);

    if(!sensor_file.empty() && !stat(sensor_file.c_str(), &buf))
    {
	if(CssTableClass::readFile(sensor_file, &global.sensor_stat, "sensor",
			global.sensor, &err) < -1)
	{
	    if(err) {
		int len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len, "\n%s", err);
	    }
	}
	else {
	    global.sensor_file.assign(sensor_file);
	}
    }

    if(!instrument_file.empty() && !stat(instrument_file.c_str(), &buf))
    {
	if(CssTableClass::readFile(instrument_file, &global.instrument_stat,
			"instrument", global.instrument, &err) < -1)
	{
	    if(err) {
		int len = (int)strlen(msg);
		snprintf(msg+len, sizeof(msg)-len, "\n%s", err);
	    }
	}
	else {
	    string s;
	    TextField::getAbsolutePath(instrument_file, global.instrument_file);

	    strncpy(dir, global.instrument_file.c_str(), sizeof(dir));
	    for(i = (int)strlen(dir)-1; i > 0 && dir[i] != '/'; i--);
	    dir[i+1] = '\0';

	    for(i = 0; i < global.instrument.size(); i++) {
		ins = (CssInstrumentClass *)global.instrument[i];
		if(ins->dir[0] != '/')
		{
		    snprintf(path, sizeof(path), "%s/%s/%s",
				dir, ins->dir, ins->dfile);
		}
		else {
		    snprintf(path, sizeof(path), "%s/%s", ins->dir, ins->dfile);
		}
		if(TextField::cleanPath(path, s)) {
		    path_q = stringToQuark(s);
		    ins->putValue("dfile", (long)path_q);
		}
	    }
	}
    }
    global.opened = true;
}

#ifdef HAVE_LIBODBC

#define MAX_Q_LEN       8192
#define PERIOD(a) (a[0] != '\0' ? "." : "")

static int
readAllRows(SQLHDBC hdbc, const string &account, const string &table_name,
		const string &css_type, gvector<CssTableClass *> &tables,
		const char **err_msg)
{
    int i, account_q, table_name_q;
    char query[MAX_Q_LEN];
    const char *acc = account.c_str();

    *err_msg = NULL;
    msg[0] = '\0';
    tables.clear();

    snprintf(query, MAX_Q_LEN, "select * from %s%s%s ",
			acc, PERIOD(acc), table_name.c_str());

    // css_type = cssObject type, for example SITE 
    if(ODBCQueryTable(hdbc, query, css_type, tables) < 0) {
	*err_msg = msg;
	snprintf(msg, 2047, "readAllRows: %s", ODBCErrMsg());
    }

    if(tables.size() <= 0) {
	*err_msg = msg;
	snprintf(msg, 1024, "No entries in %s%s%s", acc,
			PERIOD(acc), table_name.c_str());
	return(0);
    }
    account_q = stringToQuark(account);
    table_name_q = stringToQuark(table_name);

    for(i = 0; i < tables.size(); i++) {
	tables[i]->setAccount(account_q, table_name_q);
    }

    return(tables.size());
}

#endif

int ResponseFile::getInstruments(cvector<CssInstrumentClass> &v)
{
    int i, n;

    if( !local2.opened ) {
	readLocalTables2();
    }
    if( !global.opened ) {
	readGlobalTables();
    }
  
    n = local.instrument.size() + local2.instrument.size()
		+ global.instrument.size();
    if(!n) n = 1;

    v.clear();

    for(i = 0; i < local.instrument.size(); i++) {
	v.push_back((CssInstrumentClass *)local.instrument[i]);
    }
    for(i = 0; i < local2.instrument.size(); i++) {
	v.push_back((CssInstrumentClass *)local2.instrument[i]);
    }
    for(i = 0; i < global.instrument.size(); i++) {
	v.push_back((CssInstrumentClass *)global.instrument[i]);
    }
    return v.size();
}

/** Get the CssInstrumentClass for a ResponseFile.
 *  @param[in] rf a ResponseFile object.
 *  @param[in] inid inid from the instrument table, -1 if not known
 *  @returns the Instrument for the input ResponseFile. Returns NULL if the
 *  CssInstrumentClass is not found.
 */
CssInstrumentClass * ResponseFile::getInstrument(ResponseFile *rf, int inid)
{
    int i;
    long file_q, q;
    CssInstrumentClass *n;

    if( !local2.opened ) readLocalTables2();
    if( !global.opened ) readGlobalTables();
  
    if(inid > -1) {
	for(i = 0; i < local.instrument.size(); i++) {
	    n = (CssInstrumentClass *)local.instrument[i];
	    if(inid == n->inid) return n;
	}
	for(i = 0; i < local2.instrument.size(); i++) {
	    n = (CssInstrumentClass *)local2.instrument[i];
	    if(inid == n->inid) return n;
	}
	for(i = 0; i < global.instrument.size(); i++) {
	    n = (CssInstrumentClass *)global.instrument[i];
	    if(inid == n->inid) return n;
	}
#ifdef HAVE_LIBODBC 
	for(i = 0; i < global2.instrument.size(); i++) {
	    n = (CssInstrumentClass *)global2.instrument[i];
	    if(inid == n->inid) return n;
	}
#endif
	return NULL;
    }

    file_q = stringToQuark(rf->file.c_str());

    for(i = 0; i < local.instrument.size(); i++) {
	n = (CssInstrumentClass *)local.instrument[i];
	if(n->getValue("dfile", &q) && q == file_q) return n;
    }
    for(i = 0; i < local2.instrument.size(); i++) {
	n = (CssInstrumentClass *)local2.instrument[i];
	if(n->getValue("dfile", &q) && q == file_q) return n;
    }
    for(i = 0; i < global.instrument.size(); i++) {
	n = (CssInstrumentClass *)global.instrument[i];
	if(n->getValue("dfile", &q) && q == file_q) return n;
    }
    return NULL;
}

/** Get the CssInstrumentClass for an inid.
 *  @param[in] inid inid from the instrument table.
 *  @returns the CssInstrumentClass for the input ResponseFile. Returns NULL if the
 *  Instrument is not found.
 */
CssInstrumentClass * ResponseFile::getInstrument(int inid)
{
    int i;
    CssInstrumentClass * n;

    if( !local2.opened ) readLocalTables2();
    if( !global.opened ) readGlobalTables();
  
    for(i = 0; i < local.instrument.size(); i++) {
	n = (CssInstrumentClass *)local.instrument[i];
	if(inid == n->inid) return n;
    }
    for(i = 0; i < local2.instrument.size(); i++) {
	n = (CssInstrumentClass *)local2.instrument[i];
	if(inid == n->inid) return n;
    }
    for(i = 0; i < global.instrument.size(); i++) {
	n = (CssInstrumentClass *)global.instrument[i];
	if(inid == n->inid) return n;
    }
#ifdef HAVE_LIBODBC 
    for(i = 0; i < global2.instrument.size(); i++) {
	n = (CssInstrumentClass *)global2.instrument[i];
	if(inid == n->inid) return n;
    }
#endif
    return NULL;
}

CssInstrumentClass * ResponseFile::addDFile(const string &file)
{
    if( !local2.opened ) {
	readLocalTables2();
    }

    CssInstrumentClass *in = new CssInstrumentClass();
    char *path = TextField::getAbsolutePath(file);
    int i, file_q = stringToQuark(path);
    Free(path);

    for(i = (int)file.length()-1; i >= 0 && file[i] != '/'; i--);
    snprintf(in->dfile, sizeof(in->dfile), file.c_str()+i+1);

    in->putValue("dfile", (long)file_q);
    local2.instrument.push_back(in);
    return in;
}

/** Constructor with Instrument argument. Use the Instrument table to get the
 *  response file name.  Read all the responses from the file, in either CSS or
 *  GSE2.0 format. Use the function ResponseFile instead of this constructor
 *  in situations where the same file could be read multiple times. This
 *  constructor will always read the file each time it is used. This
 *  ResponseFile function will not read the same file twice, but instead will
 *  return the ResponseFile object for a file that has previously been read.
 *  @param[in] n Instrumemt structure.
 *  @throws GERROR_RESPONSE_FILE_ERROR if an error is encountered while reading
 *  the response file.
 */
ResponseFile::ResponseFile(CssInstrumentClass *in) :
	file(), responses(), io_error(false)
{
    long q = -1;
    char path[MAXPATHLEN+1], *p=NULL;

    if(in->getValue("dfile", &q)) {
	file.assign(quarkToString(q));
	if( !open() )  {
	    throw GERROR_RESPONSE_FILE_ERROR;
	}
	return;
    }

    if(in->dir[0] != '\0' && strcmp(in->dir, "-")) {
	snprintf(path, sizeof(path), "%s/%s", in->dir, in->dfile);
    }
    else {
	snprintf(path, sizeof(path), "%s", in->dfile);
    }

    p = TextField::getAbsolutePath(path);
    file.assign(p);
    Free(p);

    if( !open() )  {
	throw GERROR_RESPONSE_FILE_ERROR;
    }
}

/** Create a ResponseFile object for the input Instrument table. All responses
 *  are read from the file in either CSS or GSE2.0 format. Use this function
 *  instead of the ResponseFile constructor in situations where the same file
 *  could be read multiple times. This function will not read the same file
 *  twice, but instead will return the ResponseFile object for a file that has
 *  previously been read. The ResponseFile constructor will always read the
 *  file each time it is used.
 *  @param[in] n Instrumemt structure.
 *  @param[in] warn if true, print a warning message if this function fails.
 *  @returns a ResponseFile object or NULL if the file could not be parsed.
 */
ResponseFile * ResponseFile::readFile(CssInstrumentClass *n, bool warn)
{
    int i, len;
    long q = -1;
    char path[MAXPATHLEN+1], *p;

    if(n->dir[0] != '/')
    {
	if(!n->getValue("dfile", &q)) {
	    cerr << "ResponseFile(CssInstrumentClass n) failed" << endl;
	    throw GERROR_RESPONSE_FILE_ERROR;
	}
	snprintf(path, sizeof(path), "%s", quarkToString(q));
	for(i = (int)strlen(path)-1; i > 0 && path[i] != '/';i--);
	path[i+1] = '\0';
	strncat(path, n->dir, MAXPATHLEN-strlen(path));
    }
    else {
	stringcpy(path, n->dir, MAXPATHLEN+1);
    }
    len = strlen(path);
    if(path[(int)len-1] != '/' && len < MAXPATHLEN) {
	strncat(path, "/", 1);
    }
    strncat(path, n->dfile, MAXPATHLEN-strlen(path));
    if( !(p = TextField::cleanPath(path)) ) {
        cerr << "ResponseFile:readFile bad path: " << path << endl;
	return NULL;
    }
    strncpy(path, p, sizeof(path));
    free(p);

    return readFile(path);
}
