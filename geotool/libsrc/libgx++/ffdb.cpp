/** \file ffdb.cpp
 *  \brief TableQuery routines for a Flat File Database.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <time.h>
#include <strings.h>

#include "TableQuery.h"
#include "gobject++/GSourceInfo.h"
#include "libgio.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

static CssTableClass *ffdbCreate(OpenDB *open_db, CssTableClass *table,
		const string &table_name, const string &id_name,
		Password password, bool output_source);
static bool ffdbCreateOrigin(OpenDB *open_db, CssOriginClass *origin,
		CssOrigerrClass *origerr, CssTableClass *any, bool output_source);


/* table_name: stassoc, etc.
 * id_name: stassid, etc
 */
CssTableClass * TableQuery::ffdbTableCreate(CssTableClass *table,const string &table_name,
		const string &id_name, Password password, bool output_source)
{
    CssTableClass *t;
    /* If the "reference table" is not a wfdisc, it will have the correct
     * parameter directory information and cssCeateTable will work.
     */
    if(!table->nameIs(cssWfdisc)) {
	t = cssCreateTable(table, table_name, id_name, password);
    }
    else {
	/* Need to find or make the correct directory under the param root,
	 * using the author and time.
	 */
	t = ffdbCreate(open_db, table, table_name, id_name, password,
			output_source);
    }
    if(t) {
	Application::getApplication()->addTableCB(t);
    }
    return t;
}

static CssTableClass *
ffdbCreate(OpenDB *open_db, CssTableClass *table, const string &table_name,
		const string &id_name, Password password, bool output_source)
{
    const char *author;
    CssTableClass *t;
    FFDatabase *ffdb;

    if(output_source) {
	ffdb = FFDatabase::FFDBOpen(open_db->outputParamRoot(),
			open_db->outputSegRoot(), open_db->directoryStructure(),
			open_db->directoryDuration());
	author = ffdb->defaultAuthor();
    }
    else
    {
	int data_source, param_root, seg_root, dir_struct;
	double duration;
	table->getSource(&data_source, &param_root, &seg_root);
	table->getDirectoryStructure(&dir_struct, &duration);

	if( !(author = open_db->getWritableAuthor(quarkToString(param_root))) )
	{
	    logErrorMsg(LOG_WARNING, "ffdbCreate: writable author not found");
	    return NULL;
	}
	ffdb = FFDatabase::FFDBOpen(quarkToString(param_root),
		quarkToString(seg_root), quarkToString(dir_struct), duration);
    }

    if( !ffdb ) {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	return NULL;
    }

    t = CssTableClass::createCssTable(table_name);

    if( ffdb->insertTable(t, author) ) {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	delete t;
	return NULL;
    }

    delete ffdb;

    if(t) {
	Application::getApplication()->addTableCB(t);
    }
    return t;
}

bool TableQuery::ffdbAddArrival(CssArrivalClass *arrival, GTimeSeries *ts,
			Password password, bool output_source)
{
    const char *author;
    string sta;
    FFDatabase *ffdb;
    DateTime dt;
    int arid;

    if(output_source) {
	ffdb = FFDatabase::FFDBOpen(open_db->outputParamRoot(),
			open_db->outputSegRoot(), open_db->directoryStructure(),
			open_db->directoryDuration());
	author = ffdb->defaultAuthor();
    }
    else
    {
	GSourceInfo *s = &ts->source_info;
	if( !(author = open_db->getWritableAuthor(s->getParamRoot())) )
	{
	    logErrorMsg(LOG_WARNING, "No writable author.");
	    return false;
	}
	ffdb = FFDatabase::FFDBOpen(s->getParamRoot(), s->getSegRoot(),
			s->getDirStruct(), s->directory_duration);
    }

    if( !ffdb ) {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	return false;
    }
    if((arid = ffdb->getNextId("lastid", "arid")) >= 0) {
	arrival->arid = arid;
    }
    else {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
    }
    timeEpochToDate(arrival->time, &dt);
    arrival->jdate = timeJDate(&dt);
    stringcpy(arrival->auth, password->pw_name, sizeof(arrival->auth));

    arrival->per = arrival->period;

    if(ts->getSta(sta)) {
	getChanid(sta, arrival->chan, &arrival->chanid);
    }

    if( ffdb->insertTable(arrival, author) ) {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	return false;
    }
    delete ffdb;
    Application::getApplication()->addTableCB(arrival);

    return true;
}

bool TableQuery::ffdbOriginCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *any, Password password, bool output_source)
{
    /* If the "reference table" is not a wftag, it will have the correct
     * parameter directory information and cssCreateOrigin will work.
     */
    if(!any->nameIs("wftag")) {
	return !cssCreateOrigin(origin, origerr, any, password) ? true : false;
    }
    else {
	/* Need to find or make the correct directory under the param root,
	 * using the author and time.
	 */
	return ffdbCreateOrigin(open_db, origin, origerr, any, output_source);
    }
}

static bool
ffdbCreateOrigin(OpenDB *open_db, CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *any, bool output_source)
{
    const char *author;
    FFDatabase *ffdb;

    if(output_source) {
	ffdb = FFDatabase::FFDBOpen(open_db->outputParamRoot(),
			open_db->outputSegRoot(), open_db->directoryStructure(),
			open_db->directoryDuration());
	author = ffdb->defaultAuthor();
    }
    else
    {
	int data_source, param_root, seg_root, dir_struct;
	double duration;
	any->getSource(&data_source, &param_root, &seg_root);
	any->getDirectoryStructure(&dir_struct, &duration);
	ffdb = FFDatabase::FFDBOpen(quarkToString(param_root),
		quarkToString(seg_root), quarkToString(dir_struct), duration);

	if( !(author = open_db->getWritableAuthor(quarkToString(param_root))) )
	{
	    logErrorMsg(LOG_WARNING,
			"ffdbCreateOrigin: writable author not found");
	    return false;
	}
    }

    if( !ffdb ) {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	return false;
    }

    if( ffdb->insertTable(origin,author)  || ffdb->insertTable(origerr,author) )
    {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	return false;
    }
    delete ffdb;
    Application::getApplication()->addTableCB(origin);
    Application::getApplication()->addTableCB(origerr);

    return true;
}
