/** \file TableSource.cpp
 *  \brief Defines class TableSource.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>

#include "motif++/Application.h"
#include "TableSource.h"
#include "FFDatabase.h"
#include "gobject++/CssTables.h"

static TableSource *default_source = NULL;

TableSource::TableSource(const string &name) : BasicSource(name)
{
    source_type = DATA_SOURCE_NONE;
    ffdb = NULL;
#ifdef HAVE_LIBODBC
    hdbc = NULL;
#endif
    verbose = false;
}

TableSource::~TableSource(void)
{
    closeConnection();
}

bool TableSource::openPrefix(const string &file_prefix)
{
    closeConnection();
    char *prefix = checkPrefix(file_prefix);
    source_type = DATA_SOURCE_NONE;
    if((ffdb = FFDatabase::FFDBOpenPrefix(prefix)) == NULL)
    {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	Free(prefix);
	return false;
    }
    source_type = DATA_SOURCE_PREFIX;
    Free(prefix);
    return true;
}

char * TableSource::checkPrefix(const string &file_prefix)
{
    struct stat buf;
    char *prefix = strdup(file_prefix.c_str());

    /* check if the prefix is a full file path
     */
    if(!stat(prefix, &buf)) {
	int i, n;
	// check if the the file ends in .tableName
	n = (int)strlen(prefix);
	for(i = n-1; i >= 0; i--) {
	    if(prefix[i] == '.') {
		if(CssTableClass::isTableName(prefix+i+1)) {
		    prefix[i] = '\0';
		    return prefix;
		}
		return prefix;
	    }
	    else if(prefix[i] == '/') {
		return prefix;
	    }
	}
    }
    return prefix;
}

bool TableSource::openFFDB(const string &parameter_root,
		const string &segment_root, const string &directory_structure,
		double directory_duration)
{
    closeConnection();
    source_type = DATA_SOURCE_NONE;
    if((ffdb = FFDatabase::FFDBOpen(parameter_root, segment_root,
			directory_structure, directory_duration)) == NULL)
    {
	logErrorMsg(LOG_WARNING, FFDatabase::FFDBErrMsg());
	return false;
    }
    source_type = DATA_SOURCE_FFDB;
    return true;
}

bool TableSource::openODBC(const string &data_source, const string &user,
		const string &password)
{
#ifdef HAVE_LIBODBC
    char *source = stringTrim(strdup(data_source.c_str()));
    char *acct = stringTrim(strdup(user.c_str()));
    closeConnection();
    source_type = DATA_SOURCE_NONE;
    /* remove trailing blanks */
    if((hdbc = ODBCConnect(source, acct, password.c_str(), 0)) == NULL)
    {
	Free(source); Free(acct);
	logErrorMsg(LOG_WARNING, ODBCErrMsg());
	return false;
    }
    Free(source); Free(acct);
    source_type = DATA_SOURCE_ODBC;
    return true;
#else
    logErrorMsg(LOG_WARNING,
	"TableSource::openODBC: not compiled with libodbc.");
    return false;
#endif /* HAVE_LIBODBC */
}

void TableSource::closeConnection(void)
{
    if(source_type == DATA_SOURCE_PREFIX)
    {
	delete ffdb;
	ffdb = NULL;
    }
    else if(source_type == DATA_SOURCE_FFDB)
    {
	delete ffdb;
	ffdb = NULL;
    }
    else if(source_type == DATA_SOURCE_ODBC)
    {
#ifdef HAVE_LIBODBC
	ODBCDisconnect(hdbc);
	hdbc = NULL;
#endif /* HAVE_LIBODBC */
    }
    source_type = DATA_SOURCE_NONE;
}

void TableSource::clearTable(const string &css_table_name)
{
    gvector<CssTableClass *> *v;

    if(!strcasecmp(css_table_name.c_str(), "all"))
    {
	for(int i = 0; i < tables.size(); i++) {
	    tables[i]->clear();
	}
	if(ffdb) ffdb->clearTables();
    }
    else if(!CssTableClass::isTableName(css_table_name)) {
	char error[1000];
	snprintf(error, sizeof(error),
		"TableSource::clearTable: table %s is not recognized.",
		css_table_name.c_str());
	logErrorMsg(LOG_WARNING, error);
	return;
    }
    else if((v = findTable(css_table_name))) {
	v->clear();
    }
}

void TableSource::query(const string &query_string)
{
    int i, j, n;
    string css_table_name;

    if(source_type == DATA_SOURCE_NONE) {
	logErrorMsg(LOG_ERR,
	    "TableSource::query: no data source specified before query.");
            return;
    }

    n = (int)query_string.length();
    for(i = 0; i < n &&  isspace((int)query_string[i]); i++);
    for(j = 0; i < n && !isspace((int)query_string[i]); i++, j++)
    {
	css_table_name.append(1, query_string[i]);
    }

    while(i < n && isspace((int)query_string[i])) i++;

    if(source_type == DATA_SOURCE_PREFIX) {
	queryPrefixTables(css_table_name, query_string.c_str()+i);
    }
    else if(source_type == DATA_SOURCE_FFDB) {
	queryFFDBTables(css_table_name, query_string.c_str()+i);
    }
    else if(source_type == DATA_SOURCE_ODBC) {
	queryODBCTables(css_table_name, query_string.c_str()+i);
    }
}

void TableSource::queryAllPrefixTables(const string &wfdisc_query)
{
    if( !wfdisc_query.empty() ) {
	char q[5000];
	snprintf(q, sizeof(q), "wfdisc %s", wfdisc_query.c_str());
	query(q);
    }
    else {
        query("wfdisc select * from wfdisc");
    }
    query("arrival select * from arrival");
    query("assoc select * from assoc");
    query("origin select * from origin");
    query("origerr select * from origerr");
    query("amplitude select * from amplitude");
    query("stassoc select * from stassoc");
    query("stamag select * from stamag");
    query("netmag select * from netmag");
    query("wftag select * from wftag");
    query("xtag select * from xtag");
    query("parrival select * from parrival");
    query("hydrofeatures select * from hydrofeatures");
    query("infrafeatures select * from infrafeatures");
}

bool TableSource::queryPrefixTables(const string &css_table_name,
			const string &query_string)
{
    gvector<CssTableClass *> v;

    if( !ffdb ) {
	logErrorMsg(LOG_ERR,
	    "TableSource::queryPrefixTables: no data source opened for query.");
	return false;
    }

    if(!ffdb->queryPrefix(query_string, css_table_name, &v)) {
	logErrorMsg(LOG_ERR, FFDatabase::FFDBErrMsg());
    }

    setFFDBIds(v);
    storeRecords(v);

    return true;
}

bool
TableSource::queryFFDBTables(const string &css_table_name,
				const string &query_string)
{
    gvector<CssTableClass *> v;

    if(!ffdb) {
	logErrorMsg(LOG_ERR,
	    "TableSource::queryFFDBTables: no data source opened for query.");
	return false;
    }

    if( !ffdb->queryTable(query_string, css_table_name, &v) ) {
	logErrorMsg(LOG_ERR, FFDatabase::FFDBErrMsg());
    }

    setFFDBIds(v);
    storeRecords(v);

    return true;
}

bool TableSource::queryODBCTables(const string &css_table_name,
				const string &query_string)
{
#ifdef HAVE_LIBODBC
    gvector<CssTableClass *> v;

    if(!hdbc) {
	logErrorMsg(LOG_ERR,
	    "TableSource::queryODBCTables: no data source opened for query.");
	return false;
    }

    if(ODBCQueryTable(hdbc, query_string, css_table_name, v)) {
	logErrorMsg(LOG_ERR, ODBCErrMsg());
    }

    setODBCIds(v);
    storeRecords(v);
#endif /* HAVE_LIBODBC */
    return true;
}

void TableSource::setFFDBIds(gvector<CssTableClass *> &records)
{
    const char *name;
    char path[MAXPATHLEN+1];
    int i;

    if(records.size() <= 0) return;
    name = records[0]->getName();

    if(!strcmp(name, cssArrival)) {
	for(i = 0; i < records.size(); i++) {
	    CssArrivalClass *a = (CssArrivalClass *)records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), a->arid);
	}
    }
    else if(!strcmp(name, cssAssoc)) {
	for(i = 0; i < records.size(); i++) {
	    CssAssocClass *a = (CssAssocClass *)records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), a->arid);
	}
    }
    else if(!strcmp(name, cssOrigin)) {
	for(i = 0; i < records.size(); i++) {
	    CssOriginClass *a = (CssOriginClass *)records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), a->orid);
	}
    }
    else if(!strcmp(name, cssOrigerr)) {
	for(i = 0; i < records.size(); i++) {
	    CssOrigerrClass *a = (CssOrigerrClass *)records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), a->orid);
	}
    }
    else if(!strcmp(name, cssWfdisc)) {
	for(i = 0; i < records.size(); i++) {
	    CssWfdiscClass *a = (CssWfdiscClass *)records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), a->wfid);
	}
    }
    else if(!strcmp(name, cssAmplitude)) {
	for(i = 0; i < records.size(); i++) {
	    CssAmplitudeClass *a = (CssAmplitudeClass *)records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), a->ampid);
	}
    }
    else {
	for(i = 0; i < records.size(); i++) {
	    CssTableClass *a = records[i];
	    snprintf(path, sizeof(path), "%s/%s",
		    quarkToString(a->getDir()), quarkToString(a->getPrefix()));
	    a->setIds(stringToQuark(path), 1);
	}
    }
}

void TableSource::setODBCIds(gvector<CssTableClass *> &records)
{
    const char *name;
    int i, dc, source, user, passwd;
    char s[1000];

    if(records.size() <= 0) return;
    name = records[0]->getName();
    records[0]->getSource(&source, &user, &passwd);
    snprintf(s, sizeof(s), "%s/%s", quarkToString(source), quarkToString(user));
    dc = stringToQuark(s);

    if(!strcmp(name, cssArrival)) {
	for(i = 0; i < records.size(); i++) {
	    CssArrivalClass *a = (CssArrivalClass *)records[i];
	    a->setIds(dc, a->arid);
	}
    }
    else if(!strcmp(name, cssAssoc)) {
	for(i = 0; i < records.size(); i++) {
	    CssAssocClass *a = (CssAssocClass *)records[i];
	    a->setIds(dc, a->arid);
	}
    }
    else if(!strcmp(name, cssOrigin)) {
	for(i = 0; i < records.size(); i++) {
	    CssOriginClass *a = (CssOriginClass *)records[i];
	    a->setIds(dc, a->orid);
	}
    }
    else if(!strcmp(name, cssOrigerr)) {
	for(i = 0; i < records.size(); i++) {
	    CssOrigerrClass *a = (CssOrigerrClass *)records[i];
	    a->setIds(dc, a->orid);
	}
    }
    else if(!strcmp(name, cssWfdisc)) {
	for(i = 0; i < records.size(); i++) {
	    CssWfdiscClass *a = (CssWfdiscClass *)records[i];
	    a->setIds(dc, a->wfid);
	}
    }
}

void TableSource::copyFrom(TableSource *from)
{
    int i, j, k;

    // remove empty vectors
    for(i = tables.size()-1; i >= 0; i--)
    {
	if(tables[i]->size() == 0) {
	    tables.removeAt(i);
	}
    }

    for(i = 0; i < from->tables.size(); i++) {
	gvector<CssTableClass *> &v = *from->tables[i];

	if(v.size() > 0) {
	    for(j = 0; j < v.size(); j++) {
		v[j]->setDataSource(this);
	    }
	    for(k = 0; k < tables.size(); k++) {
		if(v[0]->getType() == tables[k]->at(0)->getType()) break;
	    }
	    if(k < tables.size()) {
		for(j = 0; j < v.size(); j++) {
		    tables[k]->push_back(v[j]);
		}
	    }
	    else {
		// Add the entire vector as a new table type.
		gvector<CssTableClass *> *t = new gvector<CssTableClass *>(v);
		tables.push_back(t);
	    }
	}
    }
}

void TableSource::storeRecords(gvector<CssTableClass *> &v)
{
    if(v.size() > 0) {
	for(int i = 0; i < v.size(); i++) {
	    v[i]->setDataSource(this);
	}
	addTables(v);
	if(verbose) {
	    if(v.size() == 1) {
		printf("1 %s record retrieved.\n", v[0]->getName());
	    }
	    else {
		printf("%d %s records retrieved.\n", v.size(), v[0]->getName());
	    }
	}
    }
    else {
	if(verbose) {
	    printf("No records found.\n");
	}
    }
}

bool TableSource::addTables(gvector<CssTableClass *> &v)
{
    int i, name;

    if(v.size() == 0) return false;

    name = v[0]->getType();

    // remove empty vectors
    for(i = tables.size()-1; i >= 0; i--)
    {
	if(tables[i]->size() == 0) {
	    tables.removeAt(i);
	}
    }

    for(i = tables.size()-1; i >= 0; i--)
    {
	if(tables[i]->size() > 0 && name == tables[i]->at(0)->getType())
	{
	    // remove current vector
	    tables.removeAt(i);
	}
    }
    // Add the entire vector as a new table type.
    gvector<CssTableClass *> *t = new gvector<CssTableClass *>(v);
    tables.push_back(t);
    return true;
}

gvector<CssTableClass *> * TableSource::findTable(const string &css_table_name)
{
    int i, name;

    name = stringToQuark(css_table_name);

    for(i = 0; i < tables.size(); i++) {
	if(tables[i]->size() > 0 && name == tables[i]->at(0)->getType()) {
	    return tables[i];
	}
    }

    // table has not been read. If it is a static table, read it.
    if(!css_table_name.compare(cssSite)) {
	query("site select * from site");
    }
    else if(!css_table_name.compare(cssSitechan)) {
	query("sitechan select * from sitechan");
    }
    else if(!css_table_name.compare(cssAffiliation)) {
	query("affiliation select * from affiliation");
    }

    for(i = 0; i < tables.size(); i++) {
	if(tables[i]->size() > 0 && name == tables[i]->at(0)->getType()) {
	    return tables[i];
	}
    }
    return NULL;
}

int TableSource::getTable(const string &css_table_name,
				gvector<CssTableClass *> &table)
{
    gvector<CssTableClass *> *v = findTable(css_table_name);
    table.clear();
    if(!v || !v->size()) return 0;
    table.load(*v);
    return table.size();
}

int TableSource::getSelectedTable(const string &css_table_name,
			gvector<CssTableClass *> &table)
{
    gvector<CssTableClass *> *v = findTable(css_table_name);
    table.clear();
    if(!v || !v->size()) return 0;
    for(int i = 0; i < v->size(); i++) {
	if(v->at(i)->getSelected()) table.add(v->at(i));
    }
    return table.size();
}

void TableSource::selectTables(const string &cssTableName, vector<int> &indices)
{
    gvector<CssTableClass *> *v = findTable(cssTableName);
    if(!v || !v->size()) return;

    for(int i = 0; i < (int)indices.size(); i++) {
        if(indices[i] >= 0 && indices[i] < v->size()) {
	    v->at(indices[i])->setSelected(true);
        }
    }
}

// this is faster than the BasicSource routine, since it bypasses the
// vector copy.
string TableSource::getNet(const string &sta)
{
    gvector<CssTableClass *> *v;

    if( !(v = findTable("affiliation")) ) {
	/* Need affiliations
	 */
	getNetworkTables();

	if( !(v = findTable("affiliation")) ) {
	    cerr << "No affiliation table." << endl;
	    return sta;
	}
    }

    int sta_q = stringUpperToQuark(sta);

    /* if sta is also a network name, return sta
     */
    int i;
    for(i = 0; i < v->size() &&
		sta_q != ((CssAffiliationClass *)v->at(i))->net_quark; i++);
    if(i < v->size()) {
	return sta;
    }

    const char *net = NULL;
    for(i = 0; i < v->size() && 
		sta_q != ((CssAffiliationClass *)v->at(i))->sta_quark; i++);
    if(i < v->size()) {
	net = ((CssAffiliationClass *)v->at(i))->net;
    }

    return((net != NULL && net[0] != '\0' && strcmp(net, "-"))
		? string(net) : sta);
}

// this is faster than the BasicSource routine, since it bypasses the
// vector copy.
CssSiteClass * TableSource::getSite(const string &sta, int jdate)
{
    gvector<CssTableClass *> *v;

    if( !(v = findTable(cssSite)) ) {
	/* Need sites
	 */
	getNetworkTables();

	if( !(v = findTable(cssSite)) ) {
	    cerr << "No site table." << endl;
	    return NULL;
	}
    }

    int sta_q = stringUpperToQuark(sta);

    CssSiteClass *s = NULL;
    for(int i = 0; i < v->size(); i++) {
	CssSiteClass *site = (CssSiteClass *)v->at(i);
	if(sta_q == site->sta_quark) {
	    if(jdate < 0) { s = site; break; }
	    if(site->ondate <= jdate && (site->offdate == -1 ||
		site->offdate > jdate)) { s = site; break; }
	}
    }
    return s;
}

// this is faster than the BasicSource routine, since it bypasses the
// vector copy.
CssSitechanClass *TableSource::getSitechan(const string &sta, const string &chan,
                                int jdate)
{
    gvector<CssTableClass *> *v;

    if( !(v = findTable(cssSitechan)) ) {
	/* Need sitechans
	 */
	getNetworkTables();

	if( !(v = findTable(cssSitechan)) ) {
	    cerr << "No sitechan table." << endl;
	    return NULL;
	}
    }

    int sta_q = stringUpperToQuark(sta);
    int chan_q = stringUpperToQuark(chan);

    CssSitechanClass *s = NULL;
    for(int i = 0; i < v->size(); i++) {
	CssSitechanClass *sc = (CssSitechanClass *)v->at(i);
	if(sta_q == sc->sta_quark && chan_q == sc->chan_quark)
	{
	    if(jdate < 0) { s = sc; break; }
	    if(sc->ondate <= jdate && (sc->offdate == -1 ||
		sc->offdate > jdate)) { s = sc; break; }
	}
    }
    return s;
}

void TableSource::removeDataReceiver(DataReceiver *owner)
{
    DataSource::removeDataReceiver(owner);
    if((int)receivers.size() == 0) {
	for(int i = 0; i < tables.size(); i++) {
	    tables[i]->clear();
	}
	tables.clear();
    }
}

// static
TableSource * TableSource::defaultSource(void)
{
    if( !default_source )
    {
	default_source = new TableSource("default");
	default_source->openPrefix("tmp");
    }
    return default_source;
}

