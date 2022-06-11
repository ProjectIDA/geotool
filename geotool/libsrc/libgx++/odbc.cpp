/** \file odbc.cpp
 *  \brief TableQuery ODBC routines.
 *  \author Ivan Henson
 */
#include "config.h"
#ifdef HAVE_LIBODBC

#include <stdlib.h>
#include <pwd.h>

#include "TableQuery.h"
#include "gobject++/GSourceInfo.h"
#include "libgio.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

#define PERIOD(a) (a[0] != '\0' ? "." : "")


/** table_name: stassoc, etc.
 * id_name: stassid, etc
 */
CssTableClass *TableQuery::odbcCreateTable(CssTableClass *table, const string &table_name,
			const string &id_name, Password password,
			bool output_source)
{
    CssTableClass *t = NULL;
    int acc, ret = 1;
    string tableName, account;
    SQLHDBC hdbc = NULL;

    if(Application::writeToDB())
    {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	}
	else {
	    if((hdbc = connectToODBC(table, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return NULL;
	    }
	    account.assign(quarkToString(acc));
	}
    }
    t = CssTableClass::createCssTable(table_name); // need new table_id
	
    if(Application::writeToDB()) {
	tableName.assign(account +  PERIOD(account)
		+ open_db->getMapping(table_name));

	ret = ODBCInsertTable(hdbc, tableName, t);
	ODBCDisconnect(hdbc);
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(t);
	return t;
    }
    else {
	delete t;
	return NULL;
    }
}

/**
 */
SQLHDBC TableQuery::connectToODBC(CssTableClass *table, int *account)
{
    int s, u, p, name, autoCommit=0;
    const char *dataSource, *user, *passwd;
    SQLHDBC hdbc;

    table->getSource(&s, &u, &p);
    table->getAccount(account, &name);
    dataSource = quarkToString(s);
    user = quarkToString(u);
    passwd = quarkToString(p);

    if((hdbc = ODBCConnect(dataSource, user, passwd, autoCommit)) == NULL) {
	return NULL;
    }
    return hdbc;
}

/**
 */
SQLHDBC TableQuery::connectToODBC(GTimeSeries *ts, int *account)
{
    int auto_commit = 0;
    SQLHDBC hdbc;
    GSourceInfo *s = &ts->source_info;

    *account = s->account;

    if((hdbc = ODBCConnect(s->getODBCDataSource(), s->getUser(),
			s->getPassword(), auto_commit)) == NULL)
    {
	return NULL;
    }
    return hdbc;
}

/**
 */
bool TableQuery::odbcChangeTable(CssTableClass *old_table, CssTableClass *new_table)
{
    string tableName;
    SQLHDBC hdbc;
    int ret;
    
    if( !Application::writeToDB() ) return true;

    if((hdbc = odbcConnect(old_table, tableName)) == NULL) {
	logErrorMsg(LOG_WARNING, ODBCErrMsg());
	return false;
    }

    ret = ODBCUpdateTable(hdbc, tableName, old_table, new_table);
    if(ret) {
	logErrorMsg(LOG_WARNING, ODBCErrMsg());
    }

    ODBCDisconnect(hdbc);

    return (!ret) ? true : false;
}

/**
 */
SQLHDBC TableQuery::odbcConnect(CssTableClass *table, string &tableName)
{
    int data_source, user, passwd, account, table_name;
    const char *name;
    SQLHDBC hdbc;

    table->getSource(&data_source, &user, &passwd);
    table->getAccount(&account, &table_name);
    name = quarkToString(table_name);
    if(name[0] == '\0') {
	name = table->getName();
    }

    if((hdbc = connectToODBC(table, &account)) == NULL) {
	fprintf(stderr, "odbcTableChange failed: %s\n", ODBCErrMsg());
	return 0;
    }
    if(user != account) {
	const char *acc = quarkToString(account);
	tableName.assign(string(acc) + PERIOD(acc) + name);
    }
    else {
	tableName.assign(name);
    }
    return hdbc;
}

/**
 */
bool TableQuery::odbcDeleteTable(CssTableClass *table, const string &table_name)
{
    string tableName;
    SQLHDBC hdbc;
    int ret=0;

    if( Application::writeToDB() )
    {
	if((hdbc = odbcConnect(table, tableName)) == NULL) {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    return false;
	}
	if(!table_name.empty()) {
	    tableName.assign(table_name);
	}
	else if(tableName.empty()) {
	    tableName.assign(open_db->getMapping(table->getName()));
	}

	ret = ODBCDeleteTable(hdbc, tableName, table);
	if(ret) {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	}

	ODBCDisconnect(hdbc);
    }

    if(!ret) TableListener::doCallbacks(table, this, "delete");

    return (!ret) ? true : false;
}

/**
 */
bool TableQuery::odbcAddArrival(CssArrivalClass *arrival, GTimeSeries *ts,
				bool output_source)
{
    int ret = 1, arid, autoCommit=0;
    const char *account;
    string tableName;
    SQLHDBC hdbc;

    arrival->per = arrival->period;

    if( !Application::writeToDB() ) {
	arrival->arid = last_arid;
	last_arid -= 1;
	Application::getApplication()->addTableCB(arrival);
	return true;
    }

    if(output_source) {
//	if((hdbc = ODBCConnect(open_db->outputSource(), open_db->outputUser(),
	if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	{
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    return false;
	}
	account = "";
    }
    else {
	int acc;
	if((hdbc = connectToODBC(ts, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	}
	account = quarkToString(acc);
    }

    tableName.assign(string(account) + PERIOD(account)
		+ open_db->getMapping(cssArrival));

    if((arid = ODBCGetNextId(hdbc, "lastid", "arid")) >= 0) {
	arrival->arid = arid;
    }
    else {
	logErrorMsg(LOG_WARNING, ODBCErrMsg());
	ODBCDisconnect(hdbc);
	return false;
    }

    ret = ODBCInsertTable(hdbc, tableName, (CssTableClass *)arrival);

    ODBCDisconnect(hdbc);
    if(ret == 1) {
	Application::getApplication()->addTableCB(arrival);
    }
    return (ret == 1) ? true : false;
}

/**
 */
bool TableQuery::odbcChangeArrival(CssArrivalClass *arrival, GTimeSeries *ts,
			int mask)
{
    string tableName;
    SQLHDBC hdbc;
    int num, ret=0, indices[100], num_indices, where[1];
    DateTime dt;
    Password password;
    CssClassDescription *des;

    if(mask & CHANGE_TIME) {
	timeEpochToDate(arrival->time, &dt);
	arrival->jdate = timeJDate(&dt);
    }
    if(mask & CHANGE_AMP_PER && arrival->period >= 0.) {
	arrival->per = arrival->period;
    }
    password = getpwuid(getuid());
    stringcpy(arrival->auth, password->pw_name, sizeof(arrival->auth));

    if( Application::writeToDB() ) {
	if((hdbc = odbcConnect(arrival, tableName)) == NULL) {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    return false;
	}

	des = arrival->description();
	num = arrival->getNumMembers();
	num_indices = 0;
	for(int i = 0; i < num; i++) {
	    if(des[i].type != CSS_LDDATE) indices[num_indices++] = i;
	}
	where[0] = arrival->memberIndex("arid");

	ret = ODBCUpdateTableWhere(hdbc, tableName, num_indices, indices,
			1, where, (CssTableClass *)arrival, (CssTableClass *)arrival);
	if(ret) {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	}

	ODBCDisconnect(hdbc);
    }

    return (!ret) ? true : false;
}

/**
 */
bool TableQuery::odbcDeleteArrival(CssArrivalClass  *arrival,
	cvector<CssAssocClass> &assocs, cvector<CssAmplitudeClass> &amps,
	cvector<CssStamagClass> &stamags, cvector<CssHydroFeaturesClass> &hydros,
	cvector<CssInfraFeaturesClass> &infras)
{
    string tableName;
    SQLHDBC hdbc;
    int i;

    // delete the arrival
    if(!odbcDeleteTable(arrival, "")) {
	return false;
    }

    // delete the assocs
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < assocs.size(); i++) if(arrival->arid == assocs[i]->arid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(assocs[i], tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, assocs[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(assocs[i], this, "delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < assocs.size(); i++) if(arrival->arid == assocs[i]->arid)
	{
	    TableListener::doCallbacks(assocs[i], this, "delete");
	}
    }

    // delete the amplitudes
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < amps.size(); i++) if(arrival->arid == amps[i]->arid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(amps[i], tableName)) ) {
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, amps[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(amps[i], this, "delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < amps.size(); i++) if(arrival->arid == amps[i]->arid)
	{
	    TableListener::doCallbacks(amps[i], this, "delete");
	}
    }

    // delete the stamags
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < stamags.size(); i++) if(arrival->arid ==stamags[i]->arid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(stamags[i], tableName)))
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, stamags[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(stamags[i],this,"delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < stamags.size(); i++) if(arrival->arid ==stamags[i]->arid)
	{
	    TableListener::doCallbacks(stamags[i], this, "delete");
	}
    }

    // delete the hydros
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < hydros.size(); i++) if(arrival->arid == hydros[i]->arid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(hydros[i], tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, hydros[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(hydros[i], this, "delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < hydros.size(); i++) if(arrival->arid == hydros[i]->arid)
	{
	    TableListener::doCallbacks(hydros[i], this, "delete");
	}
    }

    // delete the infras
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < infras.size(); i++) if(arrival->arid == infras[i]->arid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(infras[i], tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, infras[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(infras[i], this, "delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < infras.size(); i++) if(arrival->arid == infras[i]->arid)
	{
	    TableListener::doCallbacks(infras[i], this, "delete");
	}
    }

    return true;
}

/*
int
odbcUndoArrivalDelete(CssArrivalClass arrival, int n_arrivals,
		CssArrivalClass *arrivals, int n_assocs, CssAssocClass *assocs)
{
	return 1;
}
*/

/**
 */
bool TableQuery::odbcOriginCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *any, Password password, bool output_source)
{
    int ret=1, orid;
    string tableName;
    const char *account;
    SQLHDBC hdbc;

    if( Application::writeToDB() )
    {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = "";
	}
	else {
	    int acc;

	    if((hdbc = connectToODBC(any, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = quarkToString(acc);
	}

	if((orid = ODBCGetNextId(hdbc, "lastid", "orid")) >= 0) {
	    origin->orid = orid;
	    origin->evid = orid;
	    if(origerr) origerr->orid = orid;
	}
	else {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    ODBCDisconnect(hdbc);
	    return false;
	}

	stringcpy(origin->auth, password->pw_name, sizeof(origin->auth));

	tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(cssOrigin));
	ret = ODBCInsertTable(hdbc, tableName, origin);

	if(origerr) {
	    tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(cssOrigerr));
	    ret = ODBCInsertTable(hdbc, tableName, origerr);
	}

	ODBCDisconnect(hdbc);
    }
    else {
	origin->orid = last_orid;
	origin->evid = last_orid;
	if(origerr) origerr->orid = last_orid;
	last_orid -= 1;
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(origin);
	Application::getApplication()->addTableCB(origerr);
    }
    return (ret == 1) ? true : false;
}

/**
 */
bool TableQuery::odbcDeleteOrigin(CssOriginClass *origin,
	cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
	cvector<CssWftagClass> &wftags, cvector<CssNetmagClass> &netmags,
	cvector<CssStamagClass> &stamags)

{
    string tableName;
    SQLHDBC hdbc;
    int i;

    // delete the origin
    if(!odbcDeleteTable(origin, "")) {
	return false;
    }

    // delete the origerrs
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < origerrs.size(); i++)
		if(origin->orid == origerrs[i]->orid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(origerrs[i], tableName)))
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, origerrs[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(origerrs[i],this,"delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < origerrs.size(); i++)
		if(origin->orid == origerrs[i]->orid)
	{
	    TableListener::doCallbacks(origerrs[i], this, "delete");
	}
    }

    // delete the assocs
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < assocs.size(); i++) if(origin->orid == assocs[i]->orid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(assocs[i],tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, assocs[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(origerrs[i],this,"delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < assocs.size(); i++) if(origin->orid == assocs[i]->orid)
	{
	    TableListener::doCallbacks(origerrs[i], this, "delete");
	}
    }

    // delete the wftags
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < wftags.size(); i++) if(!strcmp(wftags[i]->tagname,"orid")
		&& origin->orid == wftags[i]->tagid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(wftags[i], tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, wftags[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(origerrs[i],this,"delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < wftags.size(); i++) if(!strcmp(wftags[i]->tagname,"orid")
		&& origin->orid == wftags[i]->tagid)
	{
	    TableListener::doCallbacks(origerrs[i], this, "delete");
	}
    }

    // delete the stamags
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < stamags.size(); i++) if(origin->orid == stamags[i]->orid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(stamags[i], tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, stamags[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(origerrs[i],this,"delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < stamags.size(); i++) if(origin->orid == stamags[i]->orid)
	{
	    TableListener::doCallbacks(origerrs[i], this, "delete");
	}
    }

    // delete the netmags
    if( Application::writeToDB() ) {
	hdbc = NULL;
	for(i = 0; i < netmags.size(); i++) if(origin->orid == netmags[i]->orid)
	{
	    if( !hdbc ) {
		if( !(hdbc = odbcConnect(netmags[i], tableName)) )
		{
		    logErrorMsg(LOG_WARNING, ODBCErrMsg());
		    return false;
		}
	    }
	    if( ODBCDeleteTable(hdbc, tableName, netmags[i]) ) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    }
	    else {
		TableListener::doCallbacks(origerrs[i], this, "delete");
	    }
	}
	if(hdbc) ODBCDisconnect(hdbc);
    }
    else {
	for(i = 0; i < netmags.size(); i++) if(origin->orid == netmags[i]->orid)
	{
	    TableListener::doCallbacks(origerrs[i], this, "delete");
	}
    }

    return true;
}

/*
int
odbcUndoOriginDelete(CssOriginClass origin, int n_origins, CssOriginClass *origins,
	int n_origerrs, CssOrigerrClass *origerrs, int n_netmags, CssNetmagClass *netmags,
	int n_wftags, CssWftagClass *wftags, int n_assocs, CssAssocClass *assocs,
	int n_stamags, CssStamagClass *stamags)
{
	return 1;
}
*/

/**
 */
bool TableQuery::odbcAddAssoc(CssAssocClass *assoc, bool output_source)
{
    int ret = 1;
    string tableName;
    const char *account;
    SQLHDBC hdbc;

    if( Application::writeToDB() ) {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = "";
	}
	else {
	    int acc;
	    if((hdbc = connectToODBC(assoc, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = quarkToString(acc);
	}
	tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(cssAssoc));

	ret = ODBCInsertTable(hdbc, tableName, assoc);

	ODBCDisconnect(hdbc);
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(assoc);
    }
    return (ret == 1) ? true : false;
}

/**
 */
bool TableQuery::odbcAddTable(CssTableClass *table, bool output_source)
{
    int ret = 1;
    string tableName;
    const char *account;
    SQLHDBC hdbc;

    if( Application::writeToDB() ) {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = "";
	}
	else {
	    int acc;
	    if((hdbc = connectToODBC(table, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = quarkToString(acc);
	}
	tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(table->getName()));

	ret = ODBCInsertTable(hdbc, tableName, table);

	ODBCDisconnect(hdbc);
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(table);
    }
    return (ret == 1) ? true : false;
}

/**
 */
CssOrigerrClass * TableQuery::odbcAddOrigerr(CssOriginClass *origin, bool output_source)
{
    int ret = 1;
    string tableName;
    const char *account;
    SQLHDBC hdbc;
    CssOrigerrClass *origerr;

    origerr = new CssOrigerrClass();
    origerr->setIds(origin->getDC(), 1);
    origerr->setFormat(origin->getFormat());
    origerr->orid = origin->orid;

    if( Application::writeToDB() ) {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    return false;
	    }
	    account = "";
	}
	else {
	    int acc;
	    if((hdbc = connectToODBC(origin, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return NULL;
	    }
	    account = quarkToString(acc);
	}
	tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(cssOrigerr));

	ret = ODBCInsertTable(hdbc, tableName, origerr);
	if(ret != 1) delete origerr;

	ODBCDisconnect(hdbc);
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(origerr);
	return origerr;
    }
    else {
	delete origerr;
	return NULL;
    }
}

/**
 */
bool TableQuery::odbcAddStamag(CssStamagClass *stamag, CssTableClass *table,
				bool output_source)
{
    int ret = 1;
    string tableName;
    const char *account;
    SQLHDBC hdbc;

    stamag->setIds(table->getDC(), 1);
    stamag->setFormat(table->getFormat());

    if( Application::writeToDB() ) {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = "";
	}
	else {
	    int acc;
	    if((hdbc = connectToODBC(table, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = quarkToString(acc);
	}
	tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(cssStamag));

	ret = ODBCInsertTable(hdbc, tableName, stamag);

	ODBCDisconnect(hdbc);
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(stamag);
    }
    return (ret == 1) ? true : false;
}

/**
 */
bool TableQuery::odbcAddAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude,
				bool output_source)
{
    int ret = 1, ampid;
    string tableName;
    const char *account;
    Password password;
    SQLHDBC hdbc;

    amplitude->setIds(arrival->getDC(), 1);
    amplitude->setFormat(arrival->getFormat());

    password = getpwuid(getuid());
    stringcpy(amplitude->auth, password->pw_name, sizeof(amplitude->auth));

    if( Application::writeToDB() ) {
	if(output_source) {
	    int autoCommit = 0;
	    if((hdbc = ODBCConnect(open_db->outputSource(),
			open_db->outputAccount(),
			open_db->outputPasswd(), autoCommit)) == NULL)
	    {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = "";
	}
	else {
	    int acc;
	    if((hdbc = connectToODBC(arrival, &acc)) == NULL) {
		logErrorMsg(LOG_WARNING, ODBCErrMsg());
		return false;
	    }
	    account = quarkToString(acc);
	}
	if((ampid = ODBCGetNextId(hdbc, "lastid", "ampid")) >= 0) {
	    amplitude->ampid = ampid;
	}
	else {
	    char msg[1000];
	    snprintf(msg, sizeof(msg),
		"Cannot get new ampid from lastid table.\n%s", ODBCErrMsg());
	    logErrorMsg(LOG_WARNING, msg);
	    ODBCDisconnect(hdbc);
	    return false;
	}
	tableName.assign(string(account) + PERIOD(account)
			+ open_db->getMapping(cssAmplitude));

	ret = ODBCInsertTable(hdbc, tableName, amplitude);

	ODBCDisconnect(hdbc);
    }
    else {
	amplitude->ampid = last_ampid;
	last_ampid -= 1;
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(amplitude);
    }
    return (ret == 1) ? true : false;
}

/**
 */
bool TableQuery::odbcAddNetmag(CssNetmagClass *netmag, CssTableClass *table,
			bool output_source)
{
    int ret = 1;
    string tableName;
    const char *acc;
    int account, name;
    SQLHDBC hdbc;

    netmag->setIds(table->getDC(), netmag->magid);
    netmag->setFormat(table->getFormat());

    table->getAccount(&account, &name);

    if( Application::writeToDB() ) {
	acc = quarkToString(account);
	tableName.assign(string(acc) + PERIOD(acc)
			+ open_db->getMapping(cssNetmag));

	if((hdbc = connectToODBC(table, &account)) == NULL) {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	    return false;
	}

	ret = ODBCInsertTable(hdbc, tableName, netmag);

	ODBCDisconnect(hdbc);
    }
    if(ret == 1) {
	Application::getApplication()->addTableCB(netmag);
    }
    return (ret == 1) ? true : false;
}

#endif /* HAVE_LIBODBC */
