/** \file FFDatabase.cpp
 *  \brief Defines classes FFDatabase and FFDBQuery
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>
#include <fstream>

#include "FFDatabase.h"
#include "gobject++/DataSource.h"
#include "motif++/Application.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
}

/**
 * A Flat-File database is a user defined directory
 * structure of CSS table files and data files. The directory structure is
 * based on the time associated with the table records. The date and time
 * formatting UNIX routines strftime and strptime are used to convert from
 * time to a table path and back. The FFDB library contains routines that
 * have some of the SQL querying functionality of a relational database.
 * <p>
 * There are several restrictions on the top two directory levels of the
 * database. All parameter files (CSS table files) except the wfdisc table and
 * the static tables are located under "author" subdirectories, under a
 * parameter root directory. The wfdisc tables and the data files are located
 * under "station" subdirectories, under a segment root directory. The static
 * tables (affiliation, site, sitechan, sensor, instrument, lastid, and
 * gregion) are located in a subdirectory called "static", under the parameter
 * root directory. For example, a database of "year/month/day" directories
 * could be created with an strftime date/time format of "%Y/%m/%d". It would
 * look like:
 * <pre>
 *        (param_root)/
 *            static/
 *                global.affiliation
 *                global.site
 *                global.sitechan
 *                global.sensor
 *                global.instrument
 *                global.lastid
 *                global.gregion
 *
 *            author1/
 *                2004/
 *                    01/
 *                       05/
 *                          20040105.origin
 *                          20040105.arrival
 *                          20040105.assoc
 *                          ...
 *                       06/
 *                          ...
 *            author2/
 *                ...
 *
 *        (seg_root)/
 *            station1/
 *                2004/
 *                    01/
 *                       05/
 *                          20040105.wfdisc
 *                          20040105.w
 *            station2/
 *                ...
 *
 * </pre>
 */

#define Free(a) {if(a) free((void *)a); a = NULL; }

#define MSG_SIZE 4096
static int FFDB_error_no = 0;
static char FFDB_error_msg[MSG_SIZE] = "";

static vector<StaticTable *> all_static_tables;

#ifdef __STDC__
static void FFDBSetErrorMsg(int err, const char *format, ...);
#else
static void FFDBSetErrorMsg(va_alist);
#endif

class FFDB_FILE : public Gobject
{
    public:
    int		filename_q;
    struct stat	file_stat;
    FILE	*fp;
    gvector<CssTableClass *>	*records;
    int		pos;
    FFDB_FILE(int path_q, FILE *_fp) {
	filename_q = path_q;
	memset((void *)&file_stat, 0, sizeof(struct stat));
	fp = _fp;
	records = NULL;
	pos = 0;
    }
    FFDB_FILE(int path_q, struct stat buf, gvector<CssTableClass *> *v) {
	filename_q = path_q;
	file_stat = buf;
	fp = NULL;
	records = v;
	records->addOwner(this);
	pos = 0;
    }
    FFDB_FILE(FFDB_FILE *f) {
	filename_q = f->filename_q;
	file_stat = f->file_stat;
	fp = f->fp;
	records = f->records;
	records->addOwner(this);
	pos = f->pos;
    }
    ~FFDB_FILE(void) {
	if(records) records->removeOwner(this);
    }
};

static bool dirExists(const string &dir, const char *name);
static bool convertDirToDate(const string &ds, const char *name, DateTime *dt);
static bool sameFile(char *path1, char *path2);
static int FFDBCloseFile(FFDB_FILE *mf);
static int FFDBReadFile(CssTableClass *css, FFDB_FILE *mf, const char **err_msg);
static int FFDBCreateDir(const char *path);
static int FFDBConvertDateToDir(const string &directory_structure,int param_dir,
		const string &authorOrStation, double time, char *dir);
static void addChannelAlternate(gvector<CssTableClass *> &sitechan);


FFDBQuery::FFDBQuery(FFDatabase *ffdatabase)
{
    ffdb = ffdatabase;

    data_source_q = 0;
    param_root_q = 0;
    seg_root_q = 0;
    format_q = 0;
    buffer_limit = 0;
    reading_secondary = false;
    search_error = false;
    num_fetched = 0;

    thread = (pthread_t)NULL;

    if(sem_init(&search_sem, 0, 0)) {
	FFDBSetErrorMsg(FFDB_SEM_INIT_ERR,
		"FFDBQuery: sem_init failed: %s", strerror(errno));
    }
    if(sem_init(&results_sem, 0, 0)) {
	FFDBSetErrorMsg(FFDB_SEM_INIT_ERR,
		"FFDBQuery: sem_init failed: %s", strerror(errno));
    }
}

/**
 * These are routines for opening and closing a Flat-File database, and
 * setting the default author and the directory duration.
 */
/**
 * Open a Flat-File database.
 * An FFDatabase pointer is returned, or NULL is returned if the database cannot
 * be opened. The environment variable GEOTOOL_HOME is used as an alternate
 * location for static table files that are not found under (param_root)/static.
 * Use FFDBErrMsg() and FFDBErrno() to obtain the error message and
 * error number. Possible errors are:
 * <pre>
 *    FFDB_NO_PARAM_ROOT  The parameter root does not exist or will not open.
 *    FFDB_NO_SEG_ROOT    The segment root does not exist or will not open.
 *    FFDB_MALLOC_ERR     A call to malloc failed.
 * </pre>
 * @param param_root The parameter directory root for the database.
 * @param seg_rooot The segment directory root for the database.
 * @param directory_structure The strftime/strptime format of the directories.
 * @param directory_duration The time duration of the lowest directories.
 * @return The FFDatabase pointer or NULL if the database cannot be opened.
 */
FFDatabase::FFDatabase(void)
{
    default_author = 0;
    directory_duration = 0.;
    tmin = NULL_TIME;
    tmax = NULL_TIME;
    mem_file_records = 0;
    max_mem_file_records = 5000;
    read_global_tables = 1;
}

FFDatabase * FFDatabase::FFDBOpen(const string &param_root,
	const string &seg_root, const string &directory_structure,
	double directory_duration)
{
    FFDatabase *ffdb;
    struct stat buf;
    DIR *dirp;
    struct dirent *dp;
//    enum FFDB_ERROR err;

    if(param_root.empty() || stat(param_root.c_str(), &buf)
		|| !S_ISDIR(buf.st_mode))
    {
//	err = FFDB_NO_PARAM_ROOT;
	return NULL;
    }
    if(seg_root.empty() || stat(seg_root.c_str(),&buf) || !S_ISDIR(buf.st_mode))
    {
//	err = FFDB_NO_SEG_ROOT;
	return NULL;
    }

    if( !(dirp = opendir(param_root.c_str())) ) {
	char error[MAXPATHLEN+100];
	snprintf(error, sizeof(error), "FFDBOpen: Cannot open %s.\n%s",
		param_root.c_str(), strerror(errno));
	logErrorMsg(LOG_WARNING, error);
//	err = FFDB_NO_PARAM_ROOT;
	return NULL;
    }

    ffdb = new FFDatabase();

    ffdb->prefix_files = false;
    ffdb->param_root = param_root;
    ffdb->seg_root = seg_root;
    ffdb->directory_structure = directory_structure;
    ffdb->directory_duration = (directory_duration > 0.) ?
					directory_duration : 1.e+30;

    ffdb->defineStaticTable(cssAffiliation);
    ffdb->defineStaticTable(cssSite);
    ffdb->defineStaticTable(cssSitechan);
    ffdb->defineStaticTable(cssSensor);
    ffdb->defineStaticTable(cssInstrument);
    ffdb->defineStaticTable(cssLastid);
    ffdb->defineStaticTable(cssGregion);

    while((dp = readdir(dirp)) != NULL)
    {
	if(dirExists(ffdb->param_root, dp->d_name)) {
	    ffdb->authors.push_back(new AuthorStruct(dp->d_name));
	}
    }
    closedir(dirp);

    return ffdb;
}

FFDatabase * FFDatabase::FFDBOpenPrefix(const string &prefix)
{
    FFDatabase *ffdb;

    ffdb = new FFDatabase();

    ffdb->prefix_files = true;
    ffdb->param_root = prefix;

    ffdb->defineStaticTable2(cssAffiliation);
    ffdb->defineStaticTable2(cssSite);
    ffdb->defineStaticTable2(cssSitechan);
    ffdb->defineStaticTable2(cssSensor);
    ffdb->defineStaticTable2(cssInstrument);
    ffdb->defineStaticTable2(cssLastid);
    ffdb->defineStaticTable2(cssGregion);

    ffdb->authors.push_back(new AuthorStruct(prefix.c_str()));

    return ffdb;
}

void FFDatabase::defineStaticTable(const string &cssTableName)
{
    char *c;
    string path;
    struct stat buf;

    path.assign(param_root + "/static/global." + cssTableName);

    if(stat(path.c_str(), &buf))
    {
	if((c = (char *)getenv("GEOTOOL_HOME")) != NULL)
	{
	    if(!cssTableName.compare("lastid")) {
		path.assign(string(c) + "/tables/dynamic/global."+cssTableName);
	    }
	    else {
		path.assign(string(c) + "/tables/static/global." +cssTableName);
	    }
	    if(!stat(path.c_str(), &buf)) {
		setStaticTable(cssTableName, path);
	    }
	}
    }
}

void FFDatabase::defineStaticTable2(const string &cssTableName)
{
    string name, file, file2;
    struct stat buf;

    name.assign(cssTableName + "Table");
    Application::getProperty(name, file);

    name.assign(cssTableName + "Table2");
    Application::getProperty(name, file2);

    if( (!file.empty() && !stat(file.c_str(), &buf)) ||
	(!file2.empty() && !stat(file2.c_str(), &buf)) )
    {
	setStaticTable(cssTableName, file, file2);
    }
}

static bool
dirExists(const string &dir, const char *name)
{
    char fullpath[MAXPATHLEN+1];
    struct stat buf;

    if (name[0] == '.' || (name[0] == ' ')) return(0);

    snprintf(fullpath, MAXPATHLEN+1, "%s/%s", dir.c_str(), name);

    if(stat(fullpath, &buf) != 0 || !S_ISDIR(buf.st_mode)) return false;

    return true;
}

FFDatabase::~FFDatabase(void)
{
    int i;

    static_tables.clear();
    for(i = 0; i < (int)authors.size(); i++) delete authors[i];
    authors.clear();

    for(i = 0; i < (int)mem_files.size(); i++) {
	delete mem_files[i];
    }
    mem_files.clear();
}

void FFDatabase::clearTables(void)
{
    for(int i = 0; i < (int)mem_files.size(); i++) {
	delete mem_files[i];
    }
    mem_files.clear();
    static_tables.clear();
}

/**
 * Get the Flat-File database authors. Get all authors found in the database.
 * Each author has a directory under the parameter root.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param authors The returned array of author names. Free each name and \
 *  the array.
 * @return The number of authors.
 */
int FFDatabase::getAuthors(char ***a)
{
    char **c = (char **)malloc((int)authors.size()*sizeof(char *));

    for(int i = 0; i < (int)authors.size(); i++) {
	c[i] = strdup(authors[i]->name.c_str());
    }
    *a = c;
    return (int)authors.size();
}

/**
 * Set the default author. Set the default author that will be used in
 * queries where an author is not prefixed to a table name.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param author The default author.
 * @return true for success, false if the input author does not exist in the database.
 */
bool FFDatabase::setDefaultAuthor(const string &author)
{
    int i;

    for(i=0; i < (int)authors.size() && author.compare(authors[i]->name); i++);
    if(i < (int)authors.size()) {
	default_author = i;
	return true;
    }
    return false;
}

/**
 * Get the author access mode. Returns 1, if the author can be read from and
 * written to. Returns 0, if the author's mode is read only.  Returns -1, if
 * the author does not exist. The default access mode for all authors after
 * FFDBOpen is 1, writable. Use setAuthorWritable to change the mode.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param author The author name.
 * @return 0 or 1 for success, -1 if author does not exist.
 */
bool FFDatabase::getAuthorWritable(const string &author)
{
    int i;

    for(i=0; i < (int)authors.size() && author.compare(authors[i]->name); i++);
    if(i < (int)authors.size()) {
	return authors[i]->writable;
    }
    return false;
}

/**
 * Set the author access mode.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param author The author name.
 * @param mode input 0 for read only. Input 1 for read and write access.
 * @return false if author does not exist
 */
bool FFDatabase::setAuthorWritable(const string &author, int mode)
{
    int i;

    for(i=0; i < (int)authors.size() && author.compare(authors[i]->name); i++);
    if(i < (int)authors.size()) {
	authors[i]->writable = mode;
	return true;
    }
    return false;
}

/*
    The QConstraint structure holds information from a constraint equation of
    the general form
	s.s operator sign s.s sign s.s ...
    
    where 's' can be a table name or reference, a member name, or a number.

    QConstraint c[1];

    (c[0].a.ref).(c[0].a.member) (c[0].operation) (c[0].b[0].sign) \
		(c[0].b[0].ref).(c[0].b[0].member) \
		(c[0].b[1].ref).(c[1].b[0].member) ...


    Some example queries and their structures are shown below.

    QTable tables[MAX_TABLES]; QConstraint c[MAX_TABLES];

    -----------------------------------------------------------------------
    select * from idc.origin where time >= 1064580839.206888 and
		time <= 1064582039.206959

	num_tables = 1
	tables[0].author = "idc"
	tables[0].name = "origin"
	tables[0].ref = (null)
	tables[0].num_members = 25
	tables[0].des = originDes (from libgobject/cssObjects.c)
	tables[0].tmin = 1064580839.206888
	tables[0].tmax = 1064582039.206959

	primary_table = &tables[0]

	num_constraints = 2
	c[0].operation = ">="
	c[0].a.ref = (null)
	c[0].a.member = "time"
	c[0].a.sign = 1
	c[0].a.value = 0
	c[0].a.index = 3
	c[0].a.table = &tables[0]
	c[0].nb = 1
	c[0].b[0].ref = 1064580839
	c[0].b[0].member = 206888
	c[0].b[0].sign = 1
	c[0].b[0].value = 1064580839.206888
	c[0].b[0].index = -1
	c[0].b[0].table = (null)

	c[1].operation = "<="
	c[1].a.ref = (null)
	c[1].a.member = "time"
	c[1].nb = 1
	c[1].b[0].ref = 1064582039
	c[1].b[0].member = 206959
	c[1].b[0].sign = 1
	c[1].b[0].value = 1064582039.206959
	c[1].b[0].index = -1
	c[1].b[0].table = (null)
	
    -----------------------------------------------------------------------
    select distinct r.* from idc.arrival r, static.affiliation a where
	r.sta = a.net and  a.sta like ('FIA0') and time >= 1064580839.207353
	and time <= 1064582039.207415

	num_tables = 2
	tables[0].author = "idc"
	tables[0].name = "arrival"
	tables[0].ref = "r"
	tables[0].num_members = 25
	tables[0].des = arrivalDes (from libgobject/cssObjects.c)
	tables[0].tmin = 1064580839.207353
	tables[0].tmax = 1064582039.207415

	tables[1].author = "static"
	tables[1].name = "affiliation"
	tables[1].ref = "a"
	tables[1].num_members = 3
	tables[1].des = affiliationDes (from libgobject/cssObjects.c)

	primary_table = &tables[0]
	
	num_constraints = 4
	c[0].operation = "="
	c[0].a.ref = "r"
	c[0].a.member = "sta"
	c[0].a.sign = 1
	c[0].a.value = 0
	c[0].a.index = 0
	c[0].a.table = &tables[0]
	c[0].nb = 1
	c[0].b[0].ref = "a"
	c[0].b[0].member = "net"
	c[0].b[0].sign = 1
	c[0].b[0].value = 0.
	c[0].b[0].index = 0
	c[0].b[0].table = &tables[1]

	c[1].operation = "like"
	c[1].a.ref = "a"
	c[1].a.member = "sta"
	c[1].a.sign = 1
	c[1].a.value = 0
	c[1].a.index = 1
	c[1].a.table = &tables[1]
	c[1].nb = 0
	c[1].like = {"FIA0", (null), ...}

	c[2].operation = ">="
	c[2].a.ref = (null)
	c[2].a.member = "time"
	c[2].a.sign = 1
	c[2].a.value = 0
	c[2].a.index = 1
	c[2].a.table = &tables[0]
	c[2].nb = 1
	c[2].b[0].ref = "1064580839"
	c[2].b[0].member = "207353"
	c[2].b[0].sign = 1
	c[2].b[0].value = 1064580839.207353
	c[2].b[0].index = -1
	c[2].b[0].table = (null)

	c[3].operation = "<="
	c[3].a.ref = (null)
	c[3].a.member = "time"
	c[3].a.sign = 1
	c[3].a.value = 0
	c[3].a.index = 1
	c[3].a.table = &tables[0]
	c[3].nb = 1
	c[3].b[0].ref = "1064582039"
	c[3].b[0].member = "207415"
	c[3].b[0].sign = 1
	c[3].b[0].value = 1064582039.207415
	c[3].b[0].index = -1
	c[3].b[0].table = (null)

    -----------------------------------------------------------------------
	QConstraint.nb will be greate than 1 for constraints such as:

	time between 1064582039.352787 - 14400.000000 and 1064582039.352787

	This would be translated to the following 2 constraints:

	c[0].operation = ">="
	c[0].a.ref = (null)
	c[0].a.member = "time"
	c[0].a.sign = 1
	c[0].a.value = 0
	c[0].a.index = 1
	c[0].a.table = &tables[0]
	c[0].nb = 2
	c[0].b[0].ref = "1064582039"
	c[0].b[0].member = "352787"
	c[0].b[0].sign = 1
	c[0].b[0].value = 1064582039.352787
	c[0].b[0].index = -1
	c[0].b[0].table = (null)
	c[0].b[1].ref = "14400"
	c[0].b[1].member = "000000"
	c[0].b[1].sign = -1
	c[0].b[1].value = 14400.000000
	c[0].b[1].index = -1
	c[0].b[1].table = (null)

	c[0].operation = "<="
	c[0].a.ref = (null)
	c[0].a.member = "time"
	c[0].a.sign = 1
	c[0].a.value = 0
	c[0].a.index = 1
	c[0].a.table = &tables[0]
	c[0].nb = 1
	c[0].b[0].ref = "1064582039"
	c[0].b[0].member = "352787"
	c[0].b[0].sign = 1
	c[0].b[0].value = 1064582039.352787
	c[0].b[0].index = -1
	c[0].b[0].table = (null)

*/

static int parseQuery(const char *query, QParseStruct *p);
static bool beforeWhere(QParseStruct *p, char **last, char **pos);
static bool getPrimaryTable(QParseStruct *p);
static bool get_selection(char *c, char **ref, char **member);
static bool check_more(char *p);
static bool preprocessQuery(const char *query, int n, char *q);
static bool parseWhere(char **last, char **pos, int *num, QConstraint *qc);
extern "C" {
static void *FFDBSearch(void *arg);
}
static bool getRHS(char **last, char **pos, QConstraint *q);
static bool evaluateTerm(QTerm *a, int num_tables, QTable **tables);
static bool getTimeConstraint(QTable *table, int num_constraints,
		QConstraint *c);
static void constrainTable(QTable *t, int *num_constraints, QConstraint *con);
static int getLocalConstraints(QTable *t, int *num_constraints,
		QConstraint *con, QConstraint *c);
static bool applyConstraints(int num_c, QConstraint *con);
static bool passed(double a, QConstraint *c);
static void setConstraintTable(int numc, QConstraint *c, QTable *table,
		CssTableClass *css);


/**
 * Fetch all table rows for a query. All the table rows are returned for
 *  the input query as CssTableClass objects.
 *  @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 *  @param query An SQL query that should return full table rows.
 *  @param cssTableName The CSS table name of the expected rows (input).
 *  @param table A Vector array to receive the CssTableClass objects.
 *  @return 0 for success, -1 for failure.  Retrieve the error number and \
 *              error message with FFDBErrno() and FFDBErrMsg().
 *  @see FFDBOpen
 *  @see FFDBErrno
 *  @see FFDBErrMsg
 *  @see new_Vector
 *  @see CssTableClass
 */
bool FFDatabase::queryTable(const string &query, const string &cssTableName,
		gvector<CssTableClass *> *table)
{
    FFDBQuery *qs;
    int nrows, ret;

    if( !(qs = startQuery(query, cssTableName)) ) return false;

    table->clear();

    while((ret = qs->getResults(1000, &nrows, table)) > 0);

    delete qs;

    return (ret == 0) ? true : false;
}

/**
 *  Submit a query that will return full table rows. This routine is used
 *  together with FFDBQueryTableResults() and FFDBQueryTableClose() to
 *  submit a query and fetch the results in chunks. An FFDB_QueryTableStruct
 *  pointer is returned that can be used to retrieve the desired number of
 *  table rows with FFDBQueryTableResults().
 *  @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 *  @param query An SQL query that should return full table rows.
 *  @param cssTableName The CSS table name of the expected rows (input). If \
 *	NULL, the routine will try to determine the table name from the query.
 *  @param qs A FFDB_QueryTableStruct pointer that is returned.
 *  @return 0 for success, -1 for failure.  Retrieve the error number and \
 *              error message with FFDBErrno() and FFDBErrMsg().
 *  @see FFDBConnect
 *  @see FFDBQueryTableResults
 *  @see FFDBQueryTableClose
 *  @see FFDBErrno
 *  @see FFDBErrMsg
 */
FFDBQuery * FFDatabase::startQuery(const string &query,
				const string &cssTableName)
{
    FFDBQuery *q = new FFDBQuery(this);
    CssClassDescription *des;

    q->data_source_q = stringToQuark("Flat-File-DB");
    q->param_root_q = stringToQuark(param_root);
    q->seg_root_q = stringToQuark(seg_root);
    q->format_q = stringToQuark("ffdb");
    q->buffer_limit = 1000;

    if(!parseQuery(query.c_str(), &q->parse_struct)) {
	delete q;
	return NULL;
    }
    if(q->parse_struct.num_tables == 0) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"FFDBQueryTableInit: no tables found in query.");
	delete q;
	return NULL;
    }

    if(!q->checkTables()) {
	delete q;
	return NULL;
    }

    if(!cssTableName.empty()) {
	q->table_name = cssTableName;
    }
    else {
	q->table_name = string(q->parse_struct.tables[0]->name);
    }
    if(CssTableClass::getDescription(q->table_name, &des) <= 0) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
	    "FFDBQueryTableInit: unknown table name: %s",q->table_name.c_str());
	delete q;
	return NULL;
    }

    if(pthread_create(&q->thread, NULL, FFDBSearch, (void *)q))
    {
	FFDBSetErrorMsg(FFDB_PTHREAD_ERR,
			"FFDBQueryTableInit: pthread_create failed.");
	delete q;
	return NULL;
    }
    return q;
}

/**
 *  Fetch the table rows that result from a query submitted to
 *  FFDBQueryTableInit(). The rows are returned as CssTableClass objects in the
 *  Vector table. The CssTableClass objects are <b>appended</b> to the Vector.
 *  @param qs A FFDB_QueryTableStruct pointer that was obtained from \
 *              FFDBQueryTableInit().
 *  @param numToFetch The maximum number of table rows this routine will return.
 *  @param numFetched The actual number of table rows returned.
 *  @param v A vector to hold the CssTableClass objects. Objects are appended.
 *  @return the number of rows fetched or -1 for failure.  Retrieve the error \
 *              number and error message with FFDBErrno() and FFDBErrMsg().
 *  @see FFDBQueryTableInit
 *  @see FFDBQueryTableClose
 *  @see FFDBErrno
 *  @see FFDBErrMsg
 */
int FFDBQuery::getResults(int numToFetch, int *numFetched,
		gvector<CssTableClass *> *v)
{
    int search_value;

    *numFetched = 0;
    num_fetched = 0;
    records = v;

    sem_getvalue(&search_sem, &search_value);
	
    if(search_value == 0)
    {
	buffer_limit = numToFetch;

	if(sem_post(&search_sem)) {	// run the search thread
	    FFDBSetErrorMsg(FFDB_SEM_POST_ERR,
		    "FFDBQueryTableResults: sem_post failed: %s",
		    strerror(errno));
	    return -1;
	}
	sem_wait(&results_sem);	// wait for the search thread
    }

    *numFetched = num_fetched;

    if(!num_fetched)
    {
	// all done
	return 0;
    }

    return (search_error) ? -1 : *numFetched;
}

static int
parseQuery(const char *query, QParseStruct *p)
{
    char *c = NULL, *last = NULL;
    int i, n;

    if(query[0] == '\0' || query[0] == ',') {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"parseQuery: Parse error at %s", query);
	return false;
    }

    n = (int)strlen(query);
    p->qbuf = (char *)malloc(2*n);
    if(!p->qbuf) {
	FFDBSetErrorMsg(FFDB_MALLOC_ERR, "parseQuery: malloc failed.");
	return false;
    }
    memset(p->qbuf, 0, 2*n);

    /* Preprocess the query string to remove or insert spaces that will
     * make the parsing easier.
     */
    if(!preprocessQuery(query, n, p->qbuf)) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"parseQuery: Parse error at %s", query);
	Free(p->qbuf);
	return false;
    }
    for(i = 0; i < MAX_TABLES; i++) {
	p->tables[i] = new QTable();
    }

    /* Parse the query up to the "where" clause.
     */
    if(!beforeWhere(p, &last, &c))
    {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
				"parseQuery: parse error at %s", c);
	Free(p->qbuf);
	for(i = 0; i < MAX_TABLES; i++) delete p->tables[i];
	return false;
    }
    if(p->num_tables == 0) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY, "parseQuery: Parse error.");
	Free(p->qbuf);
	for(i = 0; i < MAX_TABLES; i++) delete p->tables[i];
    }
	
    if(p->num_tables > 4) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
	    "parseQuery: Max number(4) of different table types exceeded.");
	Free(p->qbuf);
	for(i = 0; i < MAX_TABLES; i++) delete p->tables[i];
	return false;
    }
    p->num_constraints = 0;

    if(c != NULL) {
	/* Parse the Where clause. It must start with "where"
	 */
	if(!parseWhere(&last, &c, &p->num_constraints, p->constraints)) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY,
				"parseQuery: Parse error at %s", c);
	    Free(p->qbuf);
	    for(i = 0; i < MAX_TABLES; i++) delete p->tables[i];
	    return false;
	}
    }

    return getPrimaryTable(p);
}

/* Preprocess the query string to remove or insert spaces that will
 * make the parsing easier. Fails only if the query starts with an operator.
 * Take care of ';'
 */
static bool
preprocessQuery(const char *query, int n, char *q)
{
    int i, j;

    if( query[0] == '\0' || query[0] == ',' || query[0] == '=' ||
	query[0] == '<' || query[0] == '>' || query[0] == '+' ||
	query[0] == '-') return false;

    /* Remove spaces before commas, insure a space after each comma.
     * Offset operators (=,>=,<=) with spaces.
     */
    q[0] = query[0];
    for(i = j = 1; i < n; i++)
    {
	if(query[i-1] == ',' && !isspace((int)query[i])) {
	    q[j++] = ' ';
	}
	else if(query[i-1] == '=' && !isspace((int)query[i])) {
	    q[j++] = ' ';
	}
	else if(query[i-1] == '<' && query[i] != '=' && !isspace((int)query[i]))
	{
	    if(query[i] != '=') q[j++] = ' ';
	}
	else if(query[i-1] == '>' && query[i] != '=' && !isspace((int)query[i]))
	{
	    if(query[i] != '=') q[j++] = ' ';
	}
	else if(query[i-1] == '+' && !isspace((int)query[i])) {
	    q[j++] = ' ';
	}
	else if(query[i-1] == '-' && !isspace((int)query[i])) {
	    q[j++] = ' ';
	}

	if(query[i] == '\'' || query[i] == '"' || query[i] == '`') {
	    const char quote = query[i];
	    // skip over character string
	    q[j++] = query[i++];
	    while(i < n && query[i] != quote) {
		q[j++] = query[i++];
	    }
	    if(i < n) q[j++] = query[i];
	}
	else if(query[i] == ',') {
	    while(isspace((int)q[j-1])) j--;
	    q[j++] = ',';
	}
	else if(query[i] == '=' && !isspace((int)query[i-1])) {
	    if(query[i-1] != '<' && query[i-1] != '>') q[j++] = ' ';
	    q[j++] = query[i];
	}
	else if(query[i] == '>' && !isspace((int)query[i-1])) {
	    q[j++] = ' ';
	    q[j++] = query[i];
	}
	else if(query[i] == '<' && !isspace((int)query[i-1])) {
	    q[j++] = ' ';
	    q[j++] = query[i];
	}
	else if(query[i] == '+' && !isspace((int)query[i-1])) {
	    q[j++] = ' ';
	    q[j++] = query[i];
	}
	else if(query[i] == '-' && !isspace((int)query[i-1])) {
	    q[j++] = ' ';
	    q[j++] = query[i];
	}
	else if(query[i] == '(' && !isspace((int)query[i-1])) {
	    q[j++] = ' ';
	    q[j++] = query[i];
	}
	else {
	    q[j++] = query[i];
	}
    }
    q[j] = '\0';
    if(q[j-1] == ';') q[j-1] = '\0';

    return true;
}

static bool
beforeWhere(QParseStruct *p, char **last, char **pos)
{
    char *c = NULL;
    bool more;
    int n;
	
    *pos = p->qbuf;
    if((c = (char *)strtok_r(p->qbuf, " \t\n", last)) == NULL) return false;

    if(strcmp(c, "select")) return false;

    if((c = (char *)strtok_r(NULL, " \t\n", last)) == NULL) return false;
    *pos = c;

    p->distinct = false;
    if(!strcmp(c, "distinct")) {
	p->distinct = true;
	if((c = (char *)strtok_r(NULL, " \t\n",last)) == NULL) return false;
	*pos = c;
    }
    if(!strcmp(c, "*")) {
	// "select *"
	p->num_ref = 0;
	if((c = (char *)strtok_r(NULL," \t\n",last)) == NULL) return false;
	*pos = c;
    }
    else
    {
	// "select r.*" or "r.member, s.member"
	more = true;
	for(n = 0; n < MAX_TABLES && more; n++)
	{
	    more = get_selection(c, &p->refs[n].ref, &p->refs[n].member);
	    if((c=(char *)strtok_r(NULL," \t\n",last)) ==NULL) return false;
	    *pos = c;
	}
	p->num_ref = n;
    }
    // "select distinct r.* from" 
    if(strcmp(c, "from")) return false;

    if((c = (char *)strtok_r(NULL, " \t\n", last)) == NULL) return false;
    *pos = c;

    more = true;
    for(n = 0; n < MAX_TABLES && more; n++)
    {
	more = get_selection(c, &p->tables[n]->author, &p->tables[n]->name);
	p->tables[n]->ref = NULL;

	if(!more)
	{
	    if((c = (char *)strtok_r(NULL," \t\n",last)) != NULL)
	    {
		*pos = c;
		if(strcasecmp(c, "where")) {
		    // not equal to "where", must be a reference
		    p->tables[n]->ref = c;
		    more = check_more(c);
		    if((c = (char *)strtok_r(NULL," \t\n",last)) == NULL) {
			*pos = c;
			more = false;
		    }
		}
	    }
	}
	else {
	    if((c = (char *)strtok_r(NULL," \t\n",last)) == NULL) {
		more = false;
	    }
	    else {
		*pos = c;
		if(!strcasecmp(c, "where")) return false;
	    }
	}
    }
    p->num_tables = n;
    *pos = c;
    return true;
}

static bool
getPrimaryTable(QParseStruct *p)
{
    char *r;
    int i, i_primary=0;
    QTable *primary_table;

    if(p->num_tables == 0)  {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
				"Invalid query: no tables specified.");
	return false;
    }
    if(p->num_ref == 0) /* no secondary tables */
    {
	if(p->num_tables > 1) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		    "Invalid query: more that one primary table specified.");
	    return false;
	}
	i_primary = 0;
    }
    else if(p->num_ref == 1)
    {
	if((r = p->refs[0].ref) == NULL) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"getPrimaryTable: null table reference.");
	    return false;
	}
	// find the primary table
	for(i = 0; i < p->num_tables; i++) {
	    if(p->tables[i]->ref != NULL && !strcmp(p->tables[i]->ref, r))
		    break;
	}
	if(i == p->num_tables) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		    "getPrimaryTable: cannot find table for reference %s.", r);
	    return false;
	}
	i_primary = i;
    }
    else if(p->num_ref > 1) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		"Invalid query: more that one primary table specified.");
	return false;
    }

    // put the primary table first in tables[]
    primary_table = p->tables[i_primary];
    for(i = i_primary; i > 0; i--) {
	p->tables[i] = p->tables[i-1];
    }
    p->tables[0] = primary_table;

    return true;
}

static const char *less_than = "<=";
static const char *greater_than = ">=";

static bool
parseWhere(char **last, char **pos, int *num_constraints, QConstraint *qc)
{
    int i, n;
    char *c = NULL, *s = NULL, **r = NULL, *p = NULL, q, quote;

    c = *pos;

    if(strcasecmp(c, "where")) return false;

    *num_constraints = 0;

    for(n = 0; n < MAX_TABLES && (c = (char *)strtok_r(NULL," \t\n",last)); n++)
    {
	*pos = c;
	if(n > 0 && !strcasecmp(c, "and")) {
	    if((c = (char *)strtok_r(NULL, " \t\n", last)) == NULL) {
		return false;
	    }
	    *pos = c;
	}
	get_selection(c, &qc[n].a.ref, &qc[n].a.member);

	if((c = (char *)strtok_r(NULL, " \t\n", last)) == NULL) {
	    return false;
	}
	*pos = c;
	qc[n].operation = c;
	qc[n].nb = 0;
	qc[n].nl = 0;
	qc[n].b[0].ref = NULL;
	qc[n].b[0].member = NULL;
	for(i = 0; i < MAX_N_EXPR; i++) {
	    qc[n].like[i] = NULL;
	    qc[n].in[i] = NULL;
	}

	if(!strcasecmp(c, "like") || !strcasecmp(c, "in"))
	{
	    r = !strcasecmp(c, "like") ? qc[n].like : qc[n].in;
	    s = *last;
	    while(*s != '(' && *s != '\'' && *s != '"' && *s != '`'&& *s != '\0') {
		s++;
	    }
	    if(*s == '\0') return false;
	    if(*s == '\'' || *s == '"' || *s == '`') {
		quote = *s;
		s++;
		r[0] = s;
		while(*s != quote && *s != '\0') s++;
		if(*s == '\0') return false;
		*last = (*s != '\0') ? s+1 : s;
		*s = '\0';
	    }
	    else /* *s == '(' */
	    { 
		// first remove spaces about '+' and '-'
		for(p = s; *p != '\0' && *p != ')';)
		{
		    if(*p == ' ' && (*(p-1) == '+' || *(p-1) == '-'
			|| *(p+1) == '+' || *(p+1) == '-'))
		    {
			char *h;
			for(h = p; *(h+1) != '\0'; h++) {
			    *h = *(h+1);
			}
			*h = '\0';
		    }
		    else {
			   p++;
		    }
		}

		i = 0;
		while(*s != ')' && i < MAX_N_EXPR)
		{
		    s++;
		    while(*s != '\0' && isspace((int)(*s))) s++;
		    *last = (*s != '\0') ? s+1 : s;
		    if(*s == '\0') return false;
		    if(*s == '\'' || *s == '"' || *s == '`') {
			quote = *s;
			s++;
			r[i++] = s;
			while(*s != quote && *s != '\0') s++;
			if(*s == '\0') return false;
			*last = (*s != '\0') ? s+1 : s;
			*s = '\0';
		    }
		    else if(*s != ')') {
			*last = s;
			p = s+1;
			while(*p != '\0' && !isspace((int)(*p)) && *p != ','
				&& *p != ')') p++;
			if(*p == '\0') return false;
		 	q = *p;
			*p = '\0';
			qc[n].in[i++] = s;
			if(stringToLong(s, &qc[n].inl[qc[n].nl])) {
				qc[n].nl++;
			}
			s = p;
			if(q == ')') break;
		    }
		}
		*last = (*s != '\0') ? s+1 : s;
	    }
	}
	else if(!strcasecmp(c, "between"))
	{
	    qc[n].operation = greater_than;
	    if(!getRHS(last, pos, &qc[n])) return false;

	    if(strcasecmp(*pos, "and")) return false;

	    if(++n == MAX_TABLES) return false;
	    qc[n].a.ref = qc[n-1].a.ref;
	    qc[n].a.member = qc[n-1].a.member;
	    qc[n].operation = less_than;
	    qc[n].nb = 0;
	    qc[n].nl = 0;
	    qc[n].b[0].ref = NULL;
	    qc[n].b[0].member = NULL;
	    for(i = 0; i < MAX_N_EXPR; i++) {
		    qc[n].like[i] = NULL;
		    qc[n].in[i] = NULL;
	    }

	    if(!getRHS(last, pos, &qc[n])) return false;
	}
	else {
	    if(!getRHS(last, pos, &qc[n])) return false;
	}
    }
    *num_constraints = n;
    *pos = c;
    return true;
}

static bool
getRHS(char **last, char **pos, QConstraint *q)
{
    int i, j;
    char *c = NULL;
    bool sign=false;

    for(i = j = 0; (c = (char *)strtok_r(NULL, " \t\n", last)) != NULL
	    && strcasecmp(c, "and") && j < MAX_TABLES; i++)
    {
	*pos = c;
	if(i == 0)
	{
	    if(*c == '+') { // before first term
		q->b[j].sign = 1;
		sign = true;
	    }
	    else if(*c == '-') { // before first term
		q->b[j].sign = -1;
		sign = true;
	    }
	    else {
		q->b[j].sign = 1;
		get_selection(c, &q->b[j].ref, &q->b[j].member);
		j++;
	  	sign = false;
	    }
	}
	else if(sign) {
	    if(*c == '+') {
		q->b[j].sign = 1;
	    }
	    else if(*c == '-') {
		q->b[j].sign = -1;
	    }
	    else {
		return false;
	    }
	}
	else {
	    get_selection(c, &q->b[j].ref, &q->b[j].member);
	    j++;
	}
	sign = !sign;
    }
    *pos = c;
    q->nb = j;
    return true;
}

static bool
get_selection(char *c, char **ref, char **member)
{
    char *p = NULL, quote;

    // "select distinct r.member"

    if(c[0] == '\'' || c[0] == '"' || c[0] == '`') { // string constant
	int n = strlen(c);
	quote = c[0];
	*ref = NULL;
	*member = c;
	if(c[n-1] == quote) c[n-1] = '\0';
	return false;	// can't have multiple string constants
    }
    else if((p = strstr(c, ".")) != NULL) {
	*p = '\0';
	*ref = c;
	*member = p+1;
	return check_more(p+1);
    }
    else {
	*ref = NULL;
	*member = c;
	return check_more(c);
    }
}

static bool
check_more(char *p)
{
    if(p[(int)strlen(p)-1] == ',') {
	p[(int)strlen(p)-1] = '\0';
	return true;
    }
    return false;
}

bool FFDBQuery::checkTables(void)
{
    int i, j;
    QParseStruct *p = &parse_struct;
    QTable **t = p->tables;

    /* check for a valid author.
     */
    if((int)ffdb->authors.size() <= 0) {
	FFDBSetErrorMsg(FFDB_AUTHOR_ERR, "No authors.");
	return false;
    }
    /* make sure all tables have an author
     */
    for(i = 0; i < p->num_tables; i++) {
	if(t[i]->author == NULL) {
	    t[i]->author = (char *)ffdb->authors[ffdb->default_author]->name.c_str();
	}
    }

    /* Check that all secondary tables have a reference string(or char),
     * as in "r" in "arrival r" or the "a" in "static.affiliation a"
     */
    for(i = 1; i < p->num_tables; i++)
    {
	if(t[i]->ref == NULL) break;
    }
    if(i < p->num_tables) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		"Invalid query: no reference after table %s.%s",
		t[i]->author, t[i]->name);
	return false;
    }

    // Get the description of each table
    for(i = 0; i < p->num_tables; i++)
    {
	t[i]->num_members = CssTableClass::getDescription(t[i]->name, &t[i]->des);
	if(t[i]->num_members <= 0) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"UUnknown table name: %s", t[i]->name);
	    return false;
	}
    }

    /* Evaluate each term in the constraints as either a numerical constant
     * or a reference to a table member. If the latter, determine the table
     * and the member offset. Fails for an unknown member name.
     */
    for(i = 0; i < p->num_constraints; i++) {
	if(!evaluateTerm(&p->constraints[i].a, p->num_tables, t)) {
	    return false;
	}
	for(j = 0; j < p->constraints[i].nb; j++) {
	    if(!evaluateTerm(&p->constraints[i].b[j], p->num_tables, t)) {
		return false;
	    }
	}
    }
    ffdb->tmin = NULL_TIME;
    ffdb->tmax = NULL_TIME;

    /* Check for a simple time constraint (with only numerical terms) for
     * each table.
     */
    for(i = 0; i < p->num_tables; i++) {
	getTimeConstraint(t[i], p->num_constraints, p->constraints);
	if(!strcasecmp(t[i]->name, cssWfdisc)) {
	    if(t[i]->tmin > -1.e+50) {
		ffdb->tmin = t[i]->tmin;
	    }
	    if(t[i]->tmax < 1.e+50) {
		ffdb->tmax = t[i]->tmax;
	    }
	}
    }

    // Read all secondary table files needed.
    if(!readSecondaryTables(&p->num_constraints,p->constraints,p->num_tables,t))
    {
	return false;
    }

    // Check if there are records for all secondary tables.
    for(i = 1; i <p->num_tables; i++) {
	if(t[i]->constrained.size() == 0) return true;
    }

    return true;
}

static void *
FFDBSearch(void *arg)
{
    FFDBQuery *q = (FFDBQuery *)arg;
    char path[MAXPATHLEN+1], *static_path;
    int err;
    QParseStruct *p = &q->parse_struct;
    QTable **t = p->tables;

    q->search_error = false;

    // Wait for FFDBQueryTableResults to request tables.
    sem_wait(&q->search_sem);

    // Check if the primary table is a static table
    if((err = q->ffdb->readStaticTable(t[0]->name, t[0]->all, &static_path)) < 0)
    {	// error
	q->search_error = true;
    }
    else if(err) {
	q->doStaticPrimary(p->num_constraints, p->constraints, p->num_tables,
			t, q->table_name, *q->records);
    }
    else if(strcasecmp(t[0]->name, cssWfdisc))
    {
	/* If the primary table is not a wfdisc, search under a param/author
	 * directory.
	 */
	snprintf(path, MAXPATHLEN+1, "%s/%s",q->ffdb->paramRoot(),t[0]->author);

	q->searchFlatFileDB(path, p->num_constraints, p->constraints,
			p->num_tables, t, q->table_name, *q->records);
    }
    else
    {
	/* The primary table is a wfdisc. Search under the seg/station
	 * directories. Limit the search to one station directory if
	 * a station constraint is available.
	 */
	q->wfdiscSearch(p->num_constraints, p->constraints, p->num_tables,
			t, *q->records);
    }
    sem_post(&q->search_sem);
    sem_post(&q->results_sem);

    return NULL;
}

/**
 *  Terminate a query that was initiated by FFDBQueryTableInit().
 *  This routine can be called anytime after FFDBQueryTableInit(). All rows
 *  that have not been fetched with FFDBQueryTableResults() will be
 *  discarded. This routine should be called to free the resources allocated
 *  by FFDBQueryTableInit.
 *  @param qs A FFDB_QueryTableStruct pointer that was obtained from \
 *              FFDBQueryTableInit().
 *  @return 0 for success, -1 for failure.  Retrieve the error number and \
 *              error message with FFDBErrno() and FFDBErrMsg().
 *  @see FFDBQueryTableInit
 *  @see FFDBQueryTableResults
 *  @see FFDBErrno
 *  @see FFDBErrMsg
 */
FFDBQuery::~FFDBQuery(void)
{
    // signal the search thread to stop
    buffer_limit = 0;

    if(sem_post(&search_sem)) {	// run the search thread
	char error[1000];
	snprintf(error, sizeof(error), "FFDBQuery: sem_post failed: %s",
		strerror(errno));
	logErrorMsg(LOG_ERR, error);
    }
    if( thread && pthread_join(thread, NULL) ) {
	logErrorMsg(LOG_ERR, "FFDBQuery: pthread_join failed.");
    }

    Free(parse_struct.qbuf);
    for(int i = 0; i < MAX_TABLES; i++) {
	if(parse_struct.tables[i]) delete parse_struct.tables[i];
    }

    sem_destroy(&search_sem);
    sem_destroy(&results_sem);
}

bool FFDBQuery::wfdiscSearch(int num_constraints, QConstraint *con,
		int num_tables, QTable **tables, gvector<CssTableClass *> &r)
{
    int i, j, n;
    DIR *dirp;
    struct dirent *dp;
    char path[MAXPATHLEN+1];
    const char *net;
    bool sta_constraint;

    if((dirp = opendir(ffdb->seg_root.c_str())) == NULL) {
	FFDBSetErrorMsg(FFDB_NO_SEG_ROOT, "Cannot open %s",
			ffdb->seg_root.c_str());
	return false;
    }

    while(buffer_limit > 0 && (dp = readdir(dirp)) != NULL)
	if(dp->d_name[0] != '.' && dp->d_name[0] != ' ')
    {
	// check the constraint list for a constraint on station
	sta_constraint = false;
	for(i = 0; i < num_constraints; i++)
	{
	    QConstraint *c = &con[i];

	    for(j = 0; j < c->nb && c->b[j].index < 0; j++);

	    if(j == c->nb && (!strcasecmp(c->a.member, "sta") ||
			!strcasecmp(c->a.member, "net")))
	    {
		sta_constraint = true;
		if(!strcasecmp(c->operation, "like")) {
		    for(j = 0; c->like[j] != NULL; j++) {
			n = (int)strlen(c->like[j]);
			if(c->like[j][n-1] == '%') n--;
			if(!strncasecmp(dp->d_name, c->like[j], n)) break;
			net = ffdb->getNetwork(c->like[j]);
			if(net == NULL) break;
			n = (int)strlen(net);
			if(!strncasecmp(dp->d_name, net, n)) break;
		    }
		    if(c->like[j] != NULL) break; /* match found */
		}
		else if(!strcasecmp(c->operation, "in")) {
		    for(j = 0; j < MAX_N_EXPR && c->in[j] != NULL; j++) {
			if(!strcmp(dp->d_name, c->in[j])) break;
			net = ffdb->getNetwork(c->in[j]);
			if(net == NULL) break;
			if(!strcasecmp(dp->d_name, net)) break;
		    }
		    if(c->in[j] != NULL) break; /* match found */
		}
		else if(!strcmp(c->operation, "=")) {
		    if(c->nb == 1 && c->b[0].index < 0) {
			/* a string constant.
			 * use member+1 to skip over the initial '\'' 
			 * the trailing '\'' has already been nulled
			 */
			if(!strcasecmp(dp->d_name, c->b[0].member+1)) break;
			net = ffdb->getNetwork(c->b[0].member+1);
			if(net == NULL) break;
			if(!strcasecmp(dp->d_name, net)) break;
		    }
		}
	    }
	}
	if(!sta_constraint || i < num_constraints) {
	    snprintf(path, MAXPATHLEN, "%s/%s", ffdb->seg_root.c_str(),
			dp->d_name);

	    searchFlatFileDB(path, num_constraints, con, num_tables, tables,
			cssWfdisc, r);
	}
    }
    closedir(dirp);
    return true;
}

bool FFDBQuery::searchFlatFileDB(const string &path, int num_constraints,
		QConstraint *c, int num_tables, QTable **t,
		const string &tableName, gvector<CssTableClass *> &recs)
{
    DIR *dirp;
    char *ds = (char *)ffdb->directory_structure.c_str();
    struct dirent *dp;
    int i, num_levels, level;
    bool ret;

    // determine the number of directory levels in the database
    for(i = 0, num_levels = 1; ds[i] != '\0'; i++) {
	if(ds[i] == '/') num_levels++;
    }

    if((dirp = opendir(path.c_str())) == NULL) {
	if(errno != ENOTDIR) {
	    FFDBSetErrorMsg(FFDB_OPEN_DIR_ERR, "Cannot open %s", path.c_str());
	    search_error = true;
	}
	return false;
    }

    /* Search all directories under the primary author (or as restricted
     * by tmin and tmax) for records of the primary table that pass all
     * of the constraints.
     */
    ret = true;
    level = 0;
    while(buffer_limit > 0 && (dp = readdir(dirp)) != NULL)
	if(dp->d_name[0] != '.' && dp->d_name[0] != ' ')
    {
	if(!searchFlatDB(path, dp->d_name, num_levels, &level, num_constraints,
			c, num_tables, t, tableName, recs))
	{
	    ret = false;
	    break;
	}
    }
    closedir(dirp);

    return ret;
}

static bool
getTimeConstraint(QTable *table, int num_constraints, QConstraint *con)
{
    int i, j;

    // initialize time limits so that all records will be read.
    table->tmin = -1.e+60;
    table->tmax =  1.e+60;

    for(j = 0; j < table->num_members &&
			strcasecmp(table->des[j].name, "time"); j++);
    if(j == table->num_members) {
	// The table does not contain a "time" member.
	return false;
    }

    // Look for a time constraint
    for(i = 0; i < num_constraints; i++)
	if((con[i].a.ref == NULL || con[i].a.table == table)
		&& !strcasecmp(con[i].a.member, "time"))
    {
	QConstraint *c = &con[i];
	double b = 0.;

	for(j = 0; j < c->nb; j++)
	{
	    if(c->b[j].table != NULL)
	    {
		/* This constraint will not work since it depends on
		 * members of the table.  Need a time constraint with
		 * only numerical terms.
		 */
		break;
	    }
	    b += c->b[j].sign*c->b[j].value;
	}
	if(j == c->nb)
	{
	    /* Found a constraint on time with only numerical terms. Use
	     * it to narrow the directory search for this table.
	     */
	    if(c->operation[0] == '>') {
		    table->tmin = b;
	    }
	    else if(c->operation[0] == '<') {
		    table->tmax = b;
	    }
	    else if(c->operation[0] == '=') {
		// An unlikely constraint. Expand it a little.
		table->tmin = b - 60.;
		table->tmax = b + 60.;
	    }
	    else {
		    continue; // Should not happen
	    }
	}
    }
    return false; // No time constraint on this table
}

static bool
evaluateTerm(QTerm *a, int num_tables, QTable **tables)
{
    int i, j;
    char s[100], quote;
    double d;

    s[0] = '\0';
    if(a->ref != NULL) strcat(s, a->ref);
    if(a->member != NULL) {
	if(a->ref != NULL) strcat(s, ".");
	strcat(s, a->member);
    }
    // test if the term is a number
    if(stringToDouble(s, &d)) {
	a->value = d;
	a->table = NULL;
	a->index = -1;	// indicates that the term is a contant
	return true;
    }
    else if(s[0] == '\'' || s[0] == '"' || s[0] == '`') { // a string constant
	// try reading as a time field
	quote = s[0];
	for(i = 1; s[i] != '\0' && s[i] != quote; i++);
	s[i] = '\0';
	timeParseString(s+1, &a->value);
	a->table = NULL;
	a->index = -1;
	return true;
    }

    if(a->ref != NULL)
    {
	for(i = 0; i < num_tables && strcmp(a->ref, tables[i]->ref); i++);
	if(i == num_tables) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		    "Unknown table reference %s.%s", a->ref, a->member);
	    return false;
	}
	a->table = tables[i];
    }
    else {
	/* There is no specific table reference, such as a.member, so
	 * check which table has the member. Look first in the secondary
	 * tables.
	 */
	for(j = 1; j < num_tables; j++) {
	    for(i = 0; i < tables[j]->num_members; i++) {
		if(!strcasecmp(a->member, tables[j]->des[i].name)) break;
	    }
	    if(i < tables[j]->num_members) break;
	}
	if(j < num_tables) {
	    a->table = tables[j];
	}
	else {
	    a->table = tables[0];
	}
    }

    for(i = 0; i < a->table->num_members; i++) {
	if(!strcasecmp(a->member, a->table->des[i].name)) break;
    }
    if(i == a->table->num_members) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY, "%s not found in table %s",
			a->member, a->table->name);
	return false;
    }
    a->index = i;
    return true;
}

bool FFDBQuery::readSecondaryTables(int *num_constraints, QConstraint *con,
			int num_tables, QTable **tables)
{
    int i, ret, num_c;
    char path[MAXPATHLEN+1], *static_path;
    QConstraint c[MAX_TABLES];

/* should order the tables[] by putting the ones first that have constraints
 that can be applied to one table only and then to two tables only, etc.
 */

    for(i = 1; i < num_tables; i++)
    {
	num_c = getLocalConstraints(tables[i], num_constraints, con, c);

	if((ret = ffdb->readStaticTable(tables[i]->name, tables[i]->all,
			&static_path)) > 0)
	{
	    // Constrain the table if possible.
	    if(num_c > 0) {
		constrainTable(tables[i], &num_c, c);
	    }

/* then could look for constraints involving two secondary tables, and
 * use them to constrain one of the tables.
 */
	}
	else if(ret < 0) {
	    return false;
	}
	else {
	    /* Search the database
	     * Open the directory under the param root for the author
	     */
	    snprintf(path, MAXPATHLEN, "%s/%s", ffdb->param_root.c_str(),
				tables[i]->author);
	    reading_secondary = true;
	    if(!searchFlatFileDB(path, num_c, c, num_tables, tables,
				tables[i]->name, tables[i]->constrained))
	    {
		    return false;
	    }
	    reading_secondary = false;
	}
    }
    return true;
}

static void
constrainTable(QTable *t, int *num_constraints, QConstraint *con)
{
    int i, num_c;
    QConstraint c[MAX_TABLES];

    t->constrained.clear();

    /* If there are some constraints that involve only the input table
     * and a constant, apply them here and remove them from the contraints
     * array.
     */
    if((num_c = getLocalConstraints(t, num_constraints, con, c)) > 0)
    {
	for(i = 0; i < t->all.size(); i++) {
	    setConstraintTable(num_c, c, t, t->all[i]);
	    if(applyConstraints(num_c, c)) {
		t->constrained.push_back(t->all[i]);
	    }
	}
    }
    else {
	// Cannot constrain this table by itself.
	for(i = 0; i < t->all.size(); i++) {
	    t->constrained.push_back(t->all[i]);
	}
    }
}

static int
getLocalConstraints(QTable *t, int *num_constraints, QConstraint *con,
			QConstraint *c)
{
    int i, j, num_c, num, num_used, used[MAX_TABLES];
    bool specific;

    /* Collect the constraints that involve only the input table
     * and constants.
     */
    num_used = 0;
    for(i = num_c = 0; i < *num_constraints; i++)
	    if(con[i].a.ref == NULL || con[i].a.table == t)
    {
	specific = (con[i].a.table == t) ? true : false;
	// Check the left side of the constrain.
	if(con[i].a.ref == NULL) {
	    // check if the left side member is in this table.
	    for(j = 0; j < t->num_members &&
			strcasecmp(t->des[j].name, con[i].a.member); j++);
	    if(j == t->num_members) continue; /* no good */
	}

	// Check the right side of the constraint.
	for(j = 0; j < con[i].nb; j++)
	{
	    if(con[i].b[j].index == -1) continue; // constant term, good

	    if(con[i].b[j].ref != NULL && con[i].b[j].table != t) {
		// a reference to another table
		break;
	    }
	    if(con[i].b[j].ref == NULL) {
		// check if the member is in this table.
		for(j = 0; j < t->num_members &&
		    strcasecmp(t->des[j].name, con[i].b[j].member); j++);
		if(j == t->num_members) break;
	    }
	    else specific = true;
	}
	if(j == con[i].nb) { // all terms of this constraint are ok
	    c[num_c++] = con[i];
	    if(specific) {
		/* if this constraint has a specific reference to the
		 * table, it will not be needed after this routine.
		 */
		used[num_used++] = i;
	    }
	}
    }

    /* The constraints that involve only the input table (a specific
     * reference) and constants will not be needed again after this
     * routine, so remove them from the list.
     */
    num = 0;
    for(i = 0; i < *num_constraints; i++) {
	for(j = 0; j < num_used && i != used[j]; j++);
	if(j == num_used) {
	    con[num++] = con[i];
	}
    }
    *num_constraints = num;

    return num_c;
}

bool FFDBQuery::searchFlatDB(const string &root, const char *name,
	int num_levels, int *level, int num_c, QConstraint *c,
	int num_tables, QTable **tables, const string &tableName,
	gvector<CssTableClass *> &r)
{
    char fullpath[MAXPATHLEN+1];
    char fullname[MAXPATHLEN+1];
    struct stat buf;
    bool ret = true;
    DIR *dirp;
    struct dirent *dp;
    DateTime dt;

    if(name[0] == '.' || name[0] == ' ') return true;

    snprintf(fullpath, MAXPATHLEN, "%s/%s", root.c_str(), name);

    if(stat(fullpath, &buf) != 0 || !S_ISDIR(buf.st_mode)) return true;

    *level += 1;
    if(*level < num_levels)
    {
	if((dirp = opendir(fullpath)) == NULL) {
	    if(errno != ENOTDIR) {
		FFDBSetErrorMsg(FFDB_OPEN_DIR_ERR, "Cannot open %s\n%s",
				fullpath, strerror(errno));
		search_error = true;
	    }
	    *level -= 1;
	    return false;
	}
	while(buffer_limit > 0 && (dp = readdir(dirp)) != NULL)
		if(dp->d_name[0] != '.' && dp->d_name[0] != ' ')
	{
	    snprintf(fullname, MAXPATHLEN, "%s/%s", name, dp->d_name);
	    searchFlatDB(root, fullname, num_levels, level, num_c, c,
				num_tables, tables, tableName, r);
	}
	closedir(dirp);
    }
    else if(convertDirToDate(ffdb->directory_structure, name, &dt))
    {
	// good directory
	double epoch = timeDateToEpoch(&dt);

	// Check if time constraints can eliminate this directory
	if(epoch + ffdb->directory_duration >= tables[0]->tmin
			&& epoch <= tables[0]->tmax)
	{
	    // search this directory
	    if(!getRecords(root, name, num_c, c, num_tables, tables,
				tableName, r))
	    {
		ret = false;
	    }
	}
    }
    *level -= 1;
    return ret;
}

bool FFDBQuery::getRecords(const string &root, const char *name,
		int num_c, QConstraint *c, int num_tables, QTable **tables,
		const string &tableName, gvector<CssTableClass *> &r)
{
    char fullpath[MAXPATHLEN+1];
    char path[MAXPATHLEN+1];
    bool ret = true;
    int i, n;
    DIR *dirp;
    struct dirent *dp;
    struct stat buf;

    snprintf(fullpath, MAXPATHLEN, "%s/%s", root.c_str(), name);

    if((dirp = opendir(fullpath)) == NULL) {
	if(errno != ENOTDIR) {
	    FFDBSetErrorMsg(FFDB_OPEN_DIR_ERR, "Cannot open %s\n%s",
			fullpath, strerror(errno));
	    search_error = true;
	}
	return ret;
    }

    while(buffer_limit > 0 && (dp = readdir(dirp)) != NULL)
	    if(dp->d_name[0] != '.' && dp->d_name[0] != ' ')
    {
	n = (int)strlen(dp->d_name);
	for(i = n-1; i >= 0 && dp->d_name[i] != '.'; i--);
	if(i > 0 && !strcasecmp(tableName.c_str(), dp->d_name+i+1))
	{
	    snprintf(path, MAXPATHLEN, "%s/%s", fullpath, dp->d_name);
	    if(!stat(path, &buf) && !S_ISDIR(buf.st_mode)) {
		if(!readFile(path, num_c, c, num_tables, tables,
				tableName, r))
		{
		    ret = false;
		}
	    }
	}
    }
    closedir(dirp);
    return ret;
}

bool FFDBQuery::readFile(const char *path, int num_c, QConstraint *c,
	int num_tables, QTable **tables, const string &tableName,
	gvector<CssTableClass *> &r)
{
    int ret;
    FFDB_FILE *fp;
    struct stat buf;
    CssTableClass *css;

    if(num_tables > 4) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		"Max number(4) of different table types exceeded.");
	search_error = true;
	return false;
    }
    if(stat(path, &buf) || S_ISDIR(buf.st_mode))
    {
	return true;
    }
    else if(!(fp = ffdb->openFile(tableName, path))) {
	search_error = true;
	return false;
    }

    css = CssTableClass::createCssTable(tableName);

    ret = processFile(fp, path, num_c, c, num_tables, tables, tableName,
			r, &css);

    delete css;
    FFDBCloseFile(fp);

    if(ret && ret != EOF) {
	FFDBSetErrorMsg(FFDB_TABLE_READ_ERR, "Error reading %s\n%s",
			path, CssTableClass::getError());
	search_error = true;
	return false;
    }
    return true;
}

FFDB_FILE * FFDatabase::openFile(const string &tableName, const char *path)
{
    struct stat buf;
    const char *err_msg;
    FFDB_FILE *mf;
    FILE *fp;
    int i, path_q, num_members, line_length, num_records;
    CssClassDescription *des;

    if((num_members = CssTableClass::getDescription(tableName, &des)) <= 0) {
	FFDBSetErrorMsg(FFDB_MALLOC_ERR,
		"FFDBOpenFile: unkown tablename: %s", tableName.c_str());
	return NULL;
    }
    line_length = des[num_members-1].end;

    path_q = stringToQuark(path);
    for(i = 0; i < (int)mem_files.size() &&
		path_q != mem_files[i]->filename_q; i++);
    if(i < (int)mem_files.size())
    {
	/* this will read only if the file stat has changed (file updated)
	 */
	if(CssTableClass::readFile(path, &mem_files[i]->file_stat,
			tableName, *mem_files[i]->records, &err_msg) < 0)
	{
	    FFDBSetErrorMsg(FFDB_MALLOC_ERR, "FFDBOpenFile: %s", err_msg);
	    return NULL;
	}

	mem_files[i]->pos = 0;
	return new FFDB_FILE(mem_files[i]);
    }

    if(stat(path, &buf) || S_ISDIR(buf.st_mode))
    {
	if(errno > 0) {
	    FFDBSetErrorMsg(FFDB_OPENR_FILE_ERR, "Cannot open %s\n%s",
			path, strerror(errno));
	}
	else {
	    FFDBSetErrorMsg(FFDB_OPENR_FILE_ERR, "Cannot open %s", path);
	}
	return NULL;
    }

    num_records = buf.st_size/(line_length+1);
    if(mem_file_records + num_records < max_mem_file_records)
    {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	// read the file into memory
	if(CssTableClass::readFile(path, NULL, tableName, *v, &err_msg) < 0)
	{
	    FFDBSetErrorMsg(FFDB_MALLOC_ERR, "FFDBOpenFile: %s", err_msg);
	    delete v;
	    return NULL;
	}
	mf = new FFDB_FILE(path_q, buf, v);
	mem_files.push_back(mf);
	mem_file_records += num_records;

	return new FFDB_FILE(mf);
    }
    else {
	if((fp = fopen(path, "r")) == NULL) {
	    FFDBSetErrorMsg(FFDB_OPENR_FILE_ERR, "Cannot open %s\n%s",
			path, strerror(errno));
	    return NULL;
	}
	return new FFDB_FILE(path_q, fp);
    }
}

static int
FFDBCloseFile(FFDB_FILE *mf)
{
    int ret = 0;
    if(mf->fp) {
	ret = fclose(mf->fp);
    }
    delete mf;
    return ret;
}

static int
FFDBReadFile(CssTableClass *css, FFDB_FILE *mf, const char **err_msg)
{
    if(mf->fp) {
	return css->read(mf->fp, err_msg);
    }
    else if(mf->pos < mf->records->size())  {
	mf->records->at(mf->pos)->copyTo(css);
	mf->pos++;
	return 0;
    }
    else {
	return EOF;
    }
}

int FFDBQuery::processFile(FFDB_FILE *fp, const char *path, int num_c,
		QConstraint *c, int num_tables, QTable **tables,
		const string &tableName, gvector<CssTableClass *> &r, CssTableClass **pcss)
{
    int j, k, l, ret = 0;
    int file_prefix, dir, file_q, author_q, name_q, structure_q;
    double duration;
    const char *err_msg;
    CssTableClass *css;
    gvector<CssTableClass *> *u, *v, *w;

    css = *pcss;
    file_prefix = stringGetPrefix(path);
    dir = stringGetDir(path);
    file_q = stringToQuark(path);
    author_q = stringToQuark(tables[0]->author);
    name_q = stringToQuark(tables[0]->name);
    structure_q = stringToQuark(ffdb->directory_structure);
    duration = ffdb->directory_duration;

    /* the primary search table is tables[0]
     */
    if(num_tables == 1)
    {
	while(buffer_limit > 0 && !(ret = FFDBReadFile(css, fp, &err_msg)))
	{
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    if(applyConstraints(num_c, c)) {
		if(!reading_secondary && num_fetched >= buffer_limit)
		{
		    sem_post(&results_sem);
		    sem_wait(&search_sem);
		}
		if(buffer_limit <= 0) return 0;

		r.push_back(css);
		css->setDir(dir);
		css->setPrefix(file_prefix);
		css->setFile(file_q);
		css->setFormat(format_q);
		css->setAccount(author_q, name_q);
		css->setSource(data_source_q, param_root_q, seg_root_q);
		css->setDirectoryStructure(structure_q, duration);

		num_fetched++;
		*pcss = css = CssTableClass::createCssTable(tableName);
	    }
	}
    }
    else if(num_tables == 2)
    {
	while(buffer_limit > 0 && !(ret = FFDBReadFile(css, fp, &err_msg)))
	{
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    // loop over all records of the secondary table
	    u = &tables[1]->constrained;
	    for(j = 0; j < u->size() && css != NULL; j++)
	    {
		setConstraintTable(num_c, c, tables[1], u->at(j));

		if(applyConstraints(num_c, c)) {
		    if(!reading_secondary && num_fetched >= buffer_limit)
		    {
			sem_post(&results_sem);
			sem_wait(&search_sem);
		    }
		    if(buffer_limit <= 0) return true;
		    r.push_back(css);
		    css->setDir(dir);
		    css->setPrefix(file_prefix);
		    css->setFile(file_q);
		    css->setFormat(format_q);
		    css->setAccount(author_q, name_q);
		    css->setSource(data_source_q, param_root_q, seg_root_q);
		    css->setDirectoryStructure(structure_q, duration);
		    num_fetched++;
		    css = NULL;
		}
	    }
	    if(css == NULL) {
		*pcss = css = CssTableClass::createCssTable(tableName);
	    }
	}
    }
    else if(num_tables == 3)
    {
	while(buffer_limit > 0 && !(ret = FFDBReadFile(css, fp, &err_msg)))
	{
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    // loop over all records of the first secondary table
	    u = &tables[1]->constrained;
	    for(j = 0; j < u->size() && css != NULL; j++)
	    {
		setConstraintTable(num_c, c, tables[1], u->at(j));

		// loop over all records of the 2nd secondary table
		v = &tables[2]->constrained;
		for(k = 0; k < v->size() && css != NULL; k++)
		{
		    setConstraintTable(num_c, c, tables[2], v->at(k));

		    if(applyConstraints(num_c, c)) {
			if(!reading_secondary && num_fetched >= buffer_limit)
			{
			    sem_post(&results_sem);
			    sem_wait(&search_sem);
			}
			if(buffer_limit <= 0) return true;
			r.push_back(css);
			css->setDir(dir);
			css->setPrefix(file_prefix);
			css->setFile(file_q);
			css->setFormat(format_q);
			css->setAccount(author_q, name_q);
			css->setSource(data_source_q, param_root_q, seg_root_q);
			css->setDirectoryStructure(structure_q, duration);
			num_fetched++;
			css = NULL;
		    }
		}
	    }
	    if(css == NULL) {
		*pcss = css = CssTableClass::createCssTable(tableName);
	    }
	}
    }
    else if(num_tables == 4)
    {
	while(buffer_limit > 0 && !(ret = FFDBReadFile(css, fp, &err_msg)))
	{
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    // loop over all records of the first secondary table
	    u = &tables[1]->constrained;
	    for(j = 0; j < u->size() && css != NULL; j++)
	    {
		setConstraintTable(num_c, c, tables[1], u->at(j));

		// loop over all records of the 2nd secondary table
		v = &tables[2]->constrained;
		for(k = 0; k < v->size() && css != NULL; k++)
		{
		    setConstraintTable(num_c, c, tables[2], v->at(k));

		    // loop over all records of the 3nd secondary table
		    w = &tables[3]->constrained;
		    for(l = 0; l < w->size() && css != NULL; l++)
		    {
			setConstraintTable(num_c, c, tables[3], w->at(l));

			if(applyConstraints(num_c, c)) {
			    if(!reading_secondary && num_fetched >=buffer_limit)
			    {
				sem_post(&results_sem);
				sem_wait(&search_sem);
			    }
			    if(buffer_limit <= 0) return true;
			    r.push_back(css);
			    css->setDir(dir);
			    css->setPrefix(file_prefix);
			    css->setFile(file_q);
			    css->setFormat(format_q);
			    css->setAccount(author_q, name_q);
			    css->setSource(data_source_q, param_root_q,
					seg_root_q);
			    css->setDirectoryStructure(structure_q,
					duration);
			    num_fetched++;
			    css = NULL;
			}
		    }
		}
	    }
	    if(css == NULL) {
		*pcss = css = CssTableClass::createCssTable(tableName);
	    }
	}
    }

    return ret;
}

bool FFDBQuery::doStaticPrimary(int num_c, QConstraint *c, int num_tables,
			QTable **tables, const string &tableName,
			gvector<CssTableClass *> &r)
{
    int j, k, l, n;
    CssTableClass *css;
    gvector<CssTableClass *> *u, *v, *w, *x;

    if(num_tables > 4) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"Max number(4) of different table types exceeded.");
	return false;
    }

    // the primary search table is tables[0]
    if(num_tables == 1)
    {
	x = &tables[0]->all;
	for(n = 0; n < x->size(); n++)
	{
	    css = x->at(n);
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    if(applyConstraints(num_c, c)) {
		if(num_fetched >= buffer_limit) {
		    sem_post(&results_sem);
		    sem_wait(&search_sem);
		}
		if(buffer_limit > 0) {
		    r.push_back(css);
		    num_fetched++;
		}
	    }
	    if(!buffer_limit) return true;
	}
    }
    else if(num_tables == 2)
    {
	x = &tables[0]->all;
	for(n = 0; n < x->size(); n++)
	{
	    css = x->at(n);
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    // loop over all records of the secondary table
	    u = &tables[1]->constrained;
	    for(j = 0; j < u->size(); j++)
	    {
		setConstraintTable(num_c, c, tables[1], u->at(j));

		if(applyConstraints(num_c, c)) {
		    if(num_fetched >= buffer_limit) {
			sem_post(&results_sem);
			sem_wait(&search_sem);
		    }
		    if(buffer_limit > 0) {
			r.push_back(css);
			num_fetched++;
		    }
		}
		if(!buffer_limit) return true;
	    }
	}
    }
    else if(num_tables == 3)
    {
	x = &tables[0]->all;
	for(n = 0; n < x->size(); n++)
	{
	    css = x->at(n);
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    // loop over all records of the first secondary table
	    u = &tables[1]->constrained;
	    for(j = 0; j < u->size(); j++)
	    {
		setConstraintTable(num_c, c, tables[1], u->at(j));

		// loop over all records of the 2nd secondary table
		v = &tables[2]->constrained;
		for(k = 0; k < v->size(); k++)
		{
		    setConstraintTable(num_c, c, tables[2], v->at(k));

		    if(applyConstraints(num_c, c)) {
			if(num_fetched >= buffer_limit) {
			    sem_post(&results_sem);
			    sem_wait(&search_sem);
			}
			if(buffer_limit > 0) {
			    r.push_back(css);
			    num_fetched++;
			}
		    }
		    if(!buffer_limit) return true;
		}
	    }
	}
    }
    else if(num_tables == 4)
    {
	x = &tables[0]->all;
	for(n = 0; n < x->size(); n++)
	{
	    css = x->at(n);
	    // set the search table
	    setConstraintTable(num_c, c, tables[0], css);

	    // loop over all records of the first secondary table
	    u = &tables[1]->constrained;
	    for(j = 0; j < u->size(); j++)
	    {
		setConstraintTable(num_c, c, tables[1], u->at(j));

		// loop over all records of the 2nd secondary table
		v = &tables[2]->constrained;
		for(k = 0; k < v->size(); k++)
		{
		    setConstraintTable(num_c, c, tables[2], v->at(k));

		    // loop over all records of the 3nd secondary table
		    w = &tables[3]->constrained;
		    for(l = 0; l < w->size(); l++)
		    {
			setConstraintTable(num_c, c, tables[3], w->at(l));

			if(applyConstraints(num_c, c)) {
			    if(num_fetched >= buffer_limit) {
				sem_post(&results_sem);
				sem_wait(&search_sem);
			    }
			    if(buffer_limit > 0) {
				r.push_back(css);
				num_fetched++;
			    }
			}
			if(!buffer_limit) return true;
		    }
		}
	    }
	}
    }
    return true;
}

static void
setConstraintTable(int num_c, QConstraint *c, QTable *table, CssTableClass *css)
{
    int i, j;

    for(i = 0; i < num_c; i++)
    {
	if(c[i].a.table == table) {
	    c[i].a.t = css;
	}
	for(j = 0; j < c[i].nb; j++) if(c[i].b[j].table == table)
	{
	    c[i].b[j].t = css;
	}
    }
}

static bool
applyConstraints(int num_c, QConstraint *con)
{
    int i, j, n, a_type, b_type, offset;
    double a=0.;
    char *r, *s;

    for(i = 0; i < num_c; i++)
    {
	QConstraint *c = &con[i];

	a_type = c->a.table->des[c->a.index].type;
	offset = c->a.table->des[c->a.index].offset;

	if(a_type == CSS_STRING)
	{
	    s = (char *)c->a.t + offset;
	    if(!strcasecmp(c->operation, "like"))
	    {
		for(j = 0; c->like[j] != NULL; j++) {
		    n = (int)strlen(c->like[j]);
		    if(c->like[j][n-1] == '%') n--;
		    if(!strncasecmp(s, c->like[j], n)) break;
		}
		if(c->like[j] == NULL) break; /* no match found */
	    }
	    else if(!strcasecmp(c->operation, "in"))
	    {
/* use quarks if possible? */
		for(j = 0; j < MAX_N_EXPR && c->in[j] != NULL
				&& strcmp(s, c->in[j]); j++);
		if(c->in[j] == NULL) break; /* no match found */
	    }
	    else if(!strcasecmp(c->operation, "="))
	    {
/* use quarks */
		/* this is r.sta = a.net or r.sta='ARA0'
		 * must have c->nb = 1
		 */
		if(c->nb != 1) return false;
		if(c->b[0].index < 0) {
		    /* a string constant.
		     * use member+1 to skip over the initial '\'' 
		     * the trailing ''\' has already been nulled
		     */
		    if(strcmp(s, c->b[0].member+1)) break;
		}
		else {
		    if(c->b[0].table == NULL) return false;

		    b_type = c->b[0].table->des[c->b[0].index].type;
		    if(b_type != CSS_STRING)
		    {
			return false;
		    }
		    offset = c->b[0].table->des[c->b[0].index].offset;
		    r = (char *) c->b[0].t +  offset;
		    if(strcasecmp(s, r)) break;
		}
	    }
	}
	else if(!strcasecmp(c->operation, "in"))
	{
	    if(a_type == CSS_INT) {
		a = (double)(*(int *)((char *)c->a.t +  offset));
	    }
	    else if(a_type == CSS_LONG || a_type == CSS_JDATE) {
		a = (double)(*(long *)((char *)c->a.t +  offset));
	    }
	    else return false;

	    for(j = 0; j < MAX_N_EXPR && j < c->nl && a != c->inl[j]; j++);
	    if(j == c->nl) return false;
	}
	else
	{
	    if(a_type == CSS_INT) {
		a = (double)(*(int *)((char *)c->a.t +  offset));
	    }
	    else if(a_type == CSS_LONG || a_type == CSS_JDATE) {
		a = (double)(*(long *)((char *)c->a.t +  offset));
	    }
	    else if(a_type == CSS_FLOAT) {
		a = (double)(*(float *)((char *)c->a.t +  offset));
	    }
	    else if(a_type == CSS_DOUBLE || a_type == CSS_TIME) {
		a = *(double *)((char *)c->a.t +  offset);
	    }
	    else if(a_type == CSS_DATE || a_type == CSS_LDDATE) {
		DateTime *da = (DateTime *)((char *)c->a.t + offset);
		a = timeDateToEpoch(da);
	    }

	    if(!passed(a, c)) return false;
	}
    }
    return (i == num_c) ? true : false;
}

static bool
passed(double a, QConstraint *c)
{
    const char *o = c->operation;
    int i, b_type, offset;
    double b, d=0.;

    b = 0.;
    for(i = 0; i < c->nb; i++)
    {
	if(c->b[i].index >= 0)
	{
	    b_type = c->b[i].table->des[c->b[i].index].type;
	    offset = c->b[i].table->des[c->b[i].index].offset;
	    if(b_type == CSS_INT) {
		d = (double)(*(int *)((char *)c->b[i].t + offset));
	    }
	    else if(b_type == CSS_LONG || b_type == CSS_JDATE) {
		d = (double)(*(long *)((char *)c->b[i].t + offset));
	    }
	    else if(b_type == CSS_FLOAT) {
		d = (double)(*(float *)((char *)c->b[i].t + offset));
	    }
	    else if(b_type == CSS_DOUBLE || b_type == CSS_TIME) {
		d = (double)(*(double *)((char*)c->b[i].t + offset));
	    }
	    else if(b_type == CSS_DATE || b_type == CSS_LDDATE) {
		DateTime *dt = (DateTime *)((char *)c->b[i].t + offset);
		d = timeDateToEpoch(dt);
	    }
	}
	else {
	    d = c->b[i].value; /* numerical input */
	}
	b += c->b[i].sign*d;
    }

    if(o[0] == '=') {
	if(a != b) return false;
    }
    else if(o[0] == '>')
    {
	if(o[1] == '=') {
	    if(a < b) return false;
	}
	else if(a <= b) return false;
    }
    else if(o[0] == '<')
    {
	if(o[1] == '=') {
	    if(a > b) return false;
	}
	else if(a >= b) return false;
    }

    return true;
}

char *strptime(const char *s, const char *format, struct tm *tm);

/**
 * Convert a database directory name to a DateTime.
 * @param ds The directory_structure format string.
 * @param name The directory name.
 * @param dt The output DateTime structure.
 * @return true for success, false if the conversion fails.
 */
static bool
convertDirToDate(const string &ds, const char *name, DateTime *dt)
{
    struct tm tm = {0, 0, 0, 1, 0, 0, 0, 0, 0};
    char *c = NULL;

    /* if ds (directory structure) is empty, then use the format:
     * stationyyyymmdd or authoryyyymmdd
     */
    if(ds.empty()) {
	int i = (int)strlen(name) - 8;
	if(i < 0 || !(c = strptime(name+i, "%Y%m%d", &tm)) || *c != '\0') {
	    return false;
	}
    }
    else {
	if( !(c = strptime(name, ds.c_str(), &tm)) || *c != '\0') return false;
    }

    dt->year = 1900 + tm.tm_year;
    dt->month = tm.tm_mon + 1;
    dt->day = tm.tm_mday;
    dt->hour = tm.tm_hour;
    dt->minute = tm.tm_min;
    dt->second = tm.tm_sec;

    return true;
}

bool FFDatabase::queryPrefix(const string &query, const string &cssTableName,
		gvector<CssTableClass *> *table)
{
    FFDBQuery *q = new FFDBQuery(this);
    int format_q;
    bool ret;

    if(!q->prefixInit(query, cssTableName)) {
	delete q;
	return false;
    }

    table->clear();
    q->records = table;

    ret = q->searchPrefix();
    delete q;

    format_q = stringToQuark("css");
    for(int i = 0; i < table->size(); i++) {
	table->at(i)->setFormat(format_q);
    }
    return ret;
}

bool FFDBQuery::prefixInit(const string &query, const string &cssTableName)
{
    CssClassDescription *des;

    prefix = ffdb->param_root;
    buffer_limit = 100000000;
    data_source_q = stringToQuark("Flat-File-DB");
    param_root_q = stringToQuark(ffdb->param_root);
    seg_root_q = stringToQuark(ffdb->seg_root);
    format_q = stringToQuark("ffdb");

    if(!parseQuery(query.c_str(), &parse_struct)) {
	return false;
    }
    if(parse_struct.num_tables == 0) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"FFDBQuery::prefixInit: no tables found in query.");
	return false;
    }
    if( !checkPrefixTables() ) {
	return false;
    }

    if(!cssTableName.empty()) {
	table_name = cssTableName;
    }
    else {
	table_name.assign(parse_struct.tables[0]->name);
    }
    if(CssTableClass::getDescription(table_name, &des) <= 0) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
		"FFDBQuery::prefixInit: unknown table name: %s",
		table_name.c_str());
	return false;
    }

    return true;
}

bool FFDBQuery::checkPrefixTables(void)
{
    int i, j;
    QParseStruct *p = &parse_struct;
    QTable **t = p->tables;

    // check for a valid author.
    if((int)ffdb->authors.size() <= 0) {
	FFDBSetErrorMsg(FFDB_AUTHOR_ERR, "No authors.");
	return false;
    }
    // make sure all tables have an author
    for(i = 0; i < p->num_tables; i++) {
	if(t[i]->author == NULL) {
	    t[i]->author =
		(char *)ffdb->authors[ffdb->default_author]->name.c_str();
	}
    }

    /* Check that all secondary tables have a reference string(or char),
     * as in "r" in "arrival r" or the "a" in "static.affiliation a"
     */
    for(i = 1; i < p->num_tables; i++)
    {
	if(t[i]->ref == NULL) break;
    }
    if(i < p->num_tables) {
	FFDBSetErrorMsg(FFDB_INVALID_QUERY,
			"Invalid query: no reference after table %s.%s",
			t[i]->author, t[i]->name);
	return false;
    }

    // Get the description of each table
    
    for(i = 0; i < p->num_tables; i++)
    {
	t[i]->num_members = CssTableClass::getDescription(t[i]->name, &t[i]->des);
	if(t[i]->num_members <= 0) {
	    FFDBSetErrorMsg(FFDB_INVALID_QUERY, "Unknown table name: %s", t[i]->name);
	    return false;
	}
    }

    /* Evaluate each term in the constraints as either a numerical constant
     * or a reference to a table member. If the latter, determine the table
     * and the member offset. Fails for an unknown member name.
     */
    for(i = 0; i < p->num_constraints; i++) {
	if(!evaluateTerm(&p->constraints[i].a, p->num_tables, t)) {
	    return false;
	}
	for(j = 0; j < p->constraints[i].nb; j++) {
	    if(!evaluateTerm(&p->constraints[i].b[j], p->num_tables, t)) {
		return false;
	    }
	}
    }
    ffdb->tmin = NULL_TIME;
    ffdb->tmax = NULL_TIME;

    /* Check for a simple time constraint (with only numerical terms) for
     * each table.
     */
    for(i = 0; i < p->num_tables; i++) {
	getTimeConstraint(t[i], p->num_constraints, p->constraints);
	if(!strcasecmp(t[i]->name, cssWfdisc)) {
	    if(t[i]->tmin > -1.e+50) {
		ffdb->tmin = t[i]->tmin;
	    }
	    if(t[i]->tmax < 1.e+50) {
		ffdb->tmax = t[i]->tmax;
	    }
	}
    }

    // Read all secondary table files needed.
    if(!readSecondaryPrefixTables(&p->num_constraints, p->constraints,
				p->num_tables, t)) {
	return false;
    }

    // Check if there are records for all secondary tables.
    for(i = 1; i <p->num_tables; i++) {
	if(t[i]->constrained.size() == 0) {
//	    return true;
	    return false; // 2006-07-14
	}
    }

    return true;
}

bool FFDBQuery::searchPrefix(void)
{
    char path[MAXPATHLEN+1], *static_path;
    int err;
    QParseStruct *p = &parse_struct;
    QTable **t = p->tables;

    search_error = false;

    // Check if the primary table is a static table
    if((err = ffdb->readStaticTable(t[0]->name, t[0]->all, &static_path)) < 0)
    {	// error
	search_error = true;
	return false;
    }
    else if(err) {
	doStaticPrimary(p->num_constraints, p->constraints, p->num_tables, t,
			table_name, *records);
	snprintf(path, MAXPATHLEN+1, "%s.%s", prefix.c_str(), t[0]->name);
	if( !sameFile(path, static_path) )
	{ // search local file also
		readFile(path, p->num_constraints, p->constraints,
			p->num_tables, t, table_name, *records);
	}
    }
    else
    {
	snprintf(path, MAXPATHLEN+1, "%s.%s", prefix.c_str(), t[0]->name);
	readFile(path, p->num_constraints, p->constraints, p->num_tables, t,
		table_name, *records);
    }
    return !search_error;
}

static bool
sameFile(char *path1, char *path2)
{
    struct stat s1, s2;

    if(strcmp(path1, path2) || stat(path1, &s1) || stat(path2, &s2)) {
	return false;
    }
    return (s1.st_size == s2.st_size && s1.st_ino == s2.st_ino) ? true : false;
}

bool FFDBQuery::readSecondaryPrefixTables(int *num_constraints,
			QConstraint *con, int num_tables, QTable **tables)
{
    int i, ret, num_c;
    char path[MAXPATHLEN+1], *static_path;
    QConstraint c[MAX_TABLES];

/* should order the tables[] by putting the ones first that have constraints
 that can be applied to one table only and then to two tables only, etc.
 */

    for(i = 1; i < num_tables; i++)
    {
	num_c = getLocalConstraints(tables[i], num_constraints, con, c);

	if((ret = ffdb->readStaticTable(tables[i]->name, tables[i]->all,
				&static_path)) > 0)
	{
	    // Constrain the table if possible.
	    if(num_c > 0) {
		constrainTable(tables[i], &num_c, c);
	    }

/* then could look for constraints involving two secondary tables, and
 * use them to constrain one of the tables.
 */
	}
	else if(ret < 0) {
	    return false;
	}
	else {
	    /* Search the database
	     * Open the directory under the param root for the author
	     */
	    snprintf(path, MAXPATHLEN, "%s.%s", prefix.c_str(),tables[i]->name);
	    reading_secondary = true;
	    if(!readFile(path, num_c, c, num_tables, tables, tables[i]->name,
			tables[i]->constrained))
	    {
		return false;
	    }
	    reading_secondary = false;
	}
    }
    return true;
}

/* FFDBError.c

    FFDBSetErrorMsg(), FFDBErrMsg()
*/
/* Author: Ivan Henson, Scientific Computing
 */
/**
 *  Routines for handling libFFDB error messages.
 *  <p>
 *  FFDBSetErrorMsg is used mainly inside libgFFDB to store messages in a
 *  static location that will be returned by FFDBErrMsg.
 *  <p>
 *  FFDBErrMsg is used after calls to libgFFDB routines to retrieve the most
 *  recent error message.
 *  <p>
 *  The following errors can be generated by libFFDB:
* <pre>
 *     Error-Code                         Error
 * FFDB_NO_PARAM_ROOT   The parameter root directory cannot be opened.
 * FFDB_NO_SEG_ROOT     The segment root directory cannot be opened.
 * FFDB_BAD_STRUCTURE   Invalid directory structure format.
 * FFDB_CREATE_DIR_ERR  Cannot create a database directory.
 * FFDB_OPEN_DIR_ERR    Cannot open a database directory.
 * FFDB_STAT_FILE_ERR   Cannot stat a file in the database.
 * FFDB_OPENR_FILE_ERR  Cannot open a database file for reading.
 * FFDB_OPENW_FILE_ERR  Cannot open a database file for writing.
 * FFDB_AUTHOR_ERR      The default author directory does not exist.
 * FFDB_TABLE_READ_ERR  Error reading a CSS table file record.
 * FFDB_TABLE_WRITE_ERR Error writing a CSS table file record.
 * FFDB_AUTHOR_WRITE_ERR The author is not writable.
 * FFDB_INVALID_QUERY   An invalid query was submitted to FFDBQueryTable or
 *                              FFDBQueryTableInit.
 * FFDB_INVALID_UPDATE  Invalid input to FFDBUpdateTablesWhere.
 * FFDB_NO_UPDATE_FILE  A table object input to FDBUpdate does not have an
 *                              associated file.
 * FFDB_NO_DELETE_FILE  A table object input to FDBDeleteTable does not have
 *                              an associated file.
 * FFDB_NO_TIME         Cannot determine the time for a table creation.
 * FFDB_IMPORT_ERR      Cannot open an import file or directory.
 * FFDB_PTHREAD_ERR     A call to pthread_create failed.
 * FFDB_SEM_INIT_ERR    A call to sem_init failed.
 * FFDB_SEM_POST_ERR    A call to sem_post failed.
 * FFDB_JOIN_ERR        A call to pthread_join failed.
 * FFDB_MALLOC_ERR      A call to malloc failed.
 * FFDB_NO_LASTID	No lastid table file has been specified.
 * </pre>
 */

/**
 * Set the FFDB error message that will be returned by FFDBErrMsg.
 * @param err The libFFDB error number.
 * @param format The error message or format string followed by arguments.
 */
#ifdef HAVE_STDARG_H
static void
FFDBSetErrorMsg(int err, const char *format, ...)
#else
static void
FFDBSetErrorMsg(va_alist) va_dcl
#endif
{
	va_list va;

#ifdef HAVE_STDARG_H
	va_start(va, format);
#else
	char *format;
	int err;
	va_start(va);
	err = va_arg(va, int);
	format = va_arg(va, char *);
#endif
	if(format == NULL) return;

	FFDB_error_no = err;
	vsnprintf(FFDB_error_msg, MSG_SIZE, format, va);
}

/**
 * Get a libFFDB error message.
 * @return the most recent libFFDB error message.
 */
char * FFDatabase::FFDBErrMsg(void)
{
    return FFDB_error_msg;
}

/**
 * Get a libFFDB error number.
 * @return the most recent FFDB error number.
 */
int FFDatabase::FFDBErrno(void)
{
    return FFDB_error_no;
}

/**
 * Routines for setting the location of and reading static CSS table files.
 */

static int readStaticFile(StaticTable *t);

/**
 * Read a Flat-File static table. The default location for all
 * static tables is (parameter_root)/static/global.cssTableName or
 * $GEOTOOL_HOME/tables/static/global.cssTableName. Use
 * the routine setStaticTable to override the default location before
 * calling this routine. This routine returns 0, if a static file is
 * not found in the static directory, and an alternate location has
 * not been set. It returns 1, if the file is found and successfully read.
 * It returns -1, if an error occurs while opening or reading the file.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param v A vector to receive the CssTableClass objects.
 * @return -1, 0, or 1. Use FFDBErrno() and FFDBErrMsg() to retrieve the error \
 * 	number and message.
 */
int FFDatabase::readStaticTable(const string &cssTableName,
		gvector<CssTableClass *> &v, char **static_table_path)
{
    int i, ret;
    StaticTable *t;

    *static_table_path = NULL;

    if( !read_global_tables ) return 0;

    if(!(t = getStaticTable(cssTableName))) return 0;

    if((ret = readStaticFile(t)) >= 0)
    {
	*static_table_path = (char *)t->path.c_str();
	if(!strcasecmp(cssTableName.c_str(), "sitechan"))
	{
	    // add the alternate channels
	    addChannelAlternate(t->records);

	    // sort the list by station
	    CssTableClass::sort(t->records, "sta");
	}
	for(i = 0; i < t->records.size(); i++) {
	    v.push_back(t->records[i]);
	}
    }
    return (ret < 0) ? false : true;
}

StaticTable * FFDatabase::getStaticTable(const string &cssTableName)
{
    int i;
    char *c;
    string file;
    struct stat buf;

    for(i = 0; i < (int)static_tables.size() &&
	strcasecmp(cssTableName.c_str(), static_tables[i]->name.c_str()); i++);
    if(i < (int)static_tables.size()) {
	return static_tables[i];
    }

    if(!prefix_files) {
	file.assign(param_root + "/static/global." + cssTableName);

	if(!stat(file.c_str(), &buf)) {
	    return addStaticTable(cssTableName, file);
	}
	else if((c = (char *)getenv("GEOTOOL_HOME")) != NULL) {
	    file.assign(string(c) + "/tables/static/global." + cssTableName);
	    if(!stat(file.c_str(), &buf)) {
		return addStaticTable(cssTableName, file);
	    }
	}
    }
    else {
	string name, file2;

	name.assign(cssTableName + "Table");
	Application::getProperty(name, file);

	name.assign(cssTableName + "Table2");
	Application::getProperty(name, file2);

	if( (!file.empty() && !stat(file.c_str(), &buf)) ||
	    (!file2.empty() && !stat(file2.c_str(), &buf)) )
	{
	    addStaticTable(cssTableName, file, file2);
	}
    }
    return NULL;
}

/**
 * Set a static table filename. Set the name of a static table file and override
 * the default locations of (param_root)/static/global.cssTableName or
 * $GEOTOOL_HOME/tables/static/global.cssTableName.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param cssTableName The name of the table.
 * @param path The name of the table file.
 * @return 0 for success, -1 for an error. Use FFDBErrno() and FFDBErrMsg() \
 *	to retrieve the error number and message.
 */
bool FFDatabase::setStaticTable(const string &cssTableName, const string &path,
			const string &path2)
{
    StaticTable *t;

    if((t = getStaticTable(cssTableName)) != NULL)
    {
	if(t->path.compare(path) || t->path2.compare(path2)) {
	    t->path = path;
	    t->path2 = path2;
	    t->records.clear();
	    t->stat.st_ino = 0;
	    t->stat.st_dev = 0;
	    t->stat.st_mtime = 0;
	    t->stat2.st_ino = 0;
	    t->stat2.st_dev = 0;
	    t->stat2.st_mtime = 0;
	}
    }
    else {
	if(!addStaticTable(cssTableName, path, path2)) return false;
    }
    return true;
}

/**
 * Get a static table filename. Get the name of the file currently being used
 * for a static CSS table. Returns NULL, if no table file exits. The
 * default location of a static file is
 * (param_root)/static/global.cssTableNameo or
 * $GEOTOOL_HOME/tables/static/global.cssTableName.
 * Use setStaticTable to override this default with another filename.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param cssTableName The name of the table.
 * @return the filename. Do not free.
 */
const char * FFDatabase::getStaticTablePath(const string &cssTableName)
{
    StaticTable *t;

    if((t = getStaticTable(cssTableName)) != NULL)
    {
	return t->path.c_str();
    }
    return NULL;
}

StaticTable * FFDatabase::addStaticTable(const string &cssTableName,
				const string &path, const string &path2)
{
    int i;

    for(i = 0; i < (int)all_static_tables.size()
	&& all_static_tables[i]->path.compare(path); i++);
    if(i == (int)all_static_tables.size()) {
	all_static_tables.push_back(new StaticTable(cssTableName, path, path2));
    }
    static_tables.push_back(all_static_tables[i]);
    return all_static_tables[i];
}

static int
readStaticFile(StaticTable *t)
{
    const char *err_msg = NULL;
    int i, ret;

    ret = CssTableClass::readFile(t->path, &t->stat, t->name, t->records, &err_msg);
    if(ret < 0 && err_msg != NULL) {
	FFDBSetErrorMsg(FFDB_TABLE_READ_ERR, err_msg);
    }
    if(ret == 1) { // actually read the file
	for(i = 0; i < t->records.size(); i++) {
	    t->records[i]->setSaveDS(false);
	}
	// force read for the second file, if there is one
	t->stat2.st_ino = 0;
	t->stat2.st_dev = 0;
    }
    if( !t->path2.empty() ) {
	int ret2;
	gvector<CssTableClass *> t2;
	ret2 = CssTableClass::readFile(t->path2, &t->stat2, t->name, t2, &err_msg);
	if(ret2 < 0 && err_msg != NULL) {
	    FFDBSetErrorMsg(FFDB_TABLE_READ_ERR, err_msg);
	}
	if(ret2 == 1) { // actually read the file
	    for(i = 0; i < t2.size(); i++) {
		t2[i]->setSaveDS(false);
		t->records.push_back(t2[i]);
	    }
	}
	if(ret2 > 0) ret = ret2;
    }
    return ret;
}

void FFDatabase::clearStaticTables(void)
{
    for(int i = 0; i < (int)all_static_tables.size(); i++) {
	delete all_static_tables[i];
    }
    all_static_tables.clear();
}

/**
 * Get the network that contains a station.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param sta The input station name.
 * @return The network name or NULL, if the station is uknown.
 */
const char * FFDatabase::getNetwork(const char *sta)
{
    int	i, s;
    char *net;
    StaticTable *t;

    if(!(t = getStaticTable(cssAffiliation))) return NULL;
    if(readStaticFile(t) < 0) return NULL;

    s = stringUpperToQuark(sta);
    net = NULL;

    // if sta is also a network name, return sta
    for(i = 0; i < t->records.size(); i++) {
	if(s == ((CssAffiliationClass *)t->records[i])->net_quark) return sta;
    }

    for(i = 0; i < t->records.size(); i++) {
	if(s == ((CssAffiliationClass *)t->records[i])->sta_quark) {
	    net = ((CssAffiliationClass *)t->records[i])->net;
	    break;
	}
    }

    return(net != NULL ? net : sta);
}

/**
 * Get the network name. If an affiliated network is not found, return
 * the network name that starts with the station name.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param sta The input station name.
 * @return The network name or sta, if the network name is not found.
 */
const char * FFDatabase::getLikeNet(const char *sta)
{
    int i, n;
    StaticTable *t;

    if(!(t = getStaticTable(cssAffiliation))) return NULL;
    if(readStaticFile(t) < 0) return NULL;

    for(i = 0; i < t->records.size(); i++) {
	if(!strcasecmp(((CssAffiliationClass *)t->records[i])->sta, sta)) {
	    return ((CssAffiliationClass *)t->records[i])->net;
	}
    }

    n = (int)strlen(sta);

    for(i = 0; i < t->records.size(); i++) {
	if(!strncasecmp(((CssAffiliationClass *)t->records[i])->sta, sta, n)) {
	    return ((CssAffiliationClass *)t->records[i])->net;
	}
    }

    return sta;
}

/**
 * Update a table.
 * @param ffdb An FFDatabase pointer from FFDBOpen().
 * @param t The CssTableClass object.
 * @return true for success,  false for failure.  Retrieve the  error message with \
 *	FFDBErrMsg().
 */
bool FFDatabase::update(CssTableClass *t)
{
    const char *file, *err_msg;
    FILE *fp;

    file = quarkToString(t->getFile());
    if(file == NULL || file[0] == '\0')  {
	FFDBSetErrorMsg(FFDB_NO_UPDATE_FILE, "FFDBUpdate: no file.");
	return false;
    }

    if((fp = fopen(file, "r+")) == NULL)
    {
	if(errno > 0) {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
			"FFDBUpdate: Cannot write to: %s\n%s",
			file, strerror(errno));
	}
        else {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
			"FFDBUpdate: Cannot write to: %s", file);
	}
	return false;
    }
    if(fseek(fp, t->getFileOffset(), 0))
    {
	FFDBSetErrorMsg(FFDB_TABLE_WRITE_ERR,
			"FFDBUpdate: %s write error.", file);
	fclose(fp);
	return false;
    }
    if(t->write(fp, &err_msg)) {
	FFDBSetErrorMsg(FFDB_TABLE_WRITE_ERR,
			"FFDBUpdate: %s write error.\n%s", file, err_msg);
	fclose(fp);
	return false;
    }
    fclose(fp);
    return true;
}

/**
 *  These routines insert one or more records into the database.
 */

static bool getTimeFromOrid(FFDatabase *ffdb, CssTableClass *table,
		const string &author, CssClassDescription *des, double *time);
static bool getTimeFromArid(FFDatabase *ffdb, CssTableClass *table,
		const string &author, CssClassDescription *des, double *time);
static bool getWftagId(FFDatabase *ffdb, CssTableClass *table, const string &author,
		double *time);
static int getTablePrefix(const char *path, const char *dir,
		const string &tableName, char *prefix);

/**
 * Insert one table record. Insert a table record into the appropriate directory
 * and file of the database. Database directories and files are created as
 * needed. The time associated with the record and the database directory
 * structure determine the destination of the input record. The third argument
 * <b>authorOrStation</b> is the author of the input record for all tables
 * except "wfdisc". For wfdisc tables, the third argument is the station or
 * network name.
 * <p>
 * If the input table record contains a "time" member, the value of that member
 * is used to place the record. Otherwise, if the input table record contains
 * an "orid" or an "arid" member, or the input table is an orid or arid wftag,
 * the database is searched for the corresponding origin or arrival record from
 * which the time can be obtained.
 * <p>
 * If the input record does not have a "time" member and cannot be associated
 * with an origin or arrival, then it will not be written to the database,
 * and the return code will be FFDB_NO_TIME.
 * <p>
 * The following tables that are not time dependent are automatically placed
 * in a directory named "static" under the parameter root directory:
 * "affiliation", "site", "sitechan", "instrument", "sensor", and "lastid".
 * <p>
 * If the author does not exist, it will be created. It the author exist, but
 * is not writable, FFDB_AUTHOR_WRITE_ERR will be returned.
 * <p>
 * FFDBInsertTable returns 0 for success or a nonzero error code.
 * The following errors can occur:
 * <pre>
 *   FFDB_NO_TIME           Cannot determine the time for the input record.
 *   FFDB_CREATE_DIR_ERR    Cannot create the database directory.
 *   FFDB_OPEN_DIR_ERR      Cannot open the database directory.
 *   FFDB_BAD_STRUCTURE     The directory structure format is invalid.
 *   FFDB_OPENW_FILE_ERR    Cannot open the output file for writing.
 *   FFDB_STAT_FILE_ERR     Cannot stat() the output file.
 *   FFDB_TABLE_WRITE_ERR   An error occurred while writing the record.
 *   FFDB_AUTHOR_WRITE_ERR  The author is not writable.
 * </pre>
 *   
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param table The CssTableClass object whose elements will be inserted.
 * @param authorOrStation The author or the station directory name.
 * @return 0 for success,  nonzero for failure.  Retrieve the error number \
 * and the error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::insertTable(CssTableClass *table, const string &authorOrStation)
{
    const char *name;
    int i, index;
    double time = 0.0;
    CssClassDescription *des;
    int num_static = 7;
    const char *staticTables[] = {
	cssAffiliation, cssSite, cssSitechan, cssInstrument, cssSensor,
	cssLastid, cssGregion,
    };

    if((des = table->description()) == NULL) return -1;

    // check for a static table
    name = table->getName();
    for(i = 0; i < num_static && strcasecmp(name, staticTables[i]); i++);
    if(i < num_static) {
	return writeStaticTable(table);
    }
	
    // Need a time.
    if((index = table->memberIndex("time")) >= 0 &&
	    (des[index].type == CSS_TIME || des[index].type == CSS_DOUBLE))
    {
	time = *(double *)((char *)table + des[index].offset);
    }
    else if(!getTimeFromOrid(this, table, authorOrStation, des, &time) &&
	    !getTimeFromArid(this, table, authorOrStation, des, &time) &&
	    !getWftagId(this, table, authorOrStation, &time))
    {
	FFDBSetErrorMsg(FFDB_NO_TIME,
		"FFDB::insertTable: Cannot determine time for %s.%s",
		authorOrStation.c_str(), table->getName());
	return FFDB_NO_TIME;
    }

    return writeTable(table, authorOrStation, time);
}

static bool
getTimeFromOrid(FFDatabase *ffdb, CssTableClass *table, const string &author,
		CssClassDescription *des, double *time)
{
    char query[100];
    int index;
    long orid;
    gvector<CssTableClass *> v;

    if((index = table->memberIndex("orid")) < 0) return false;

    if(des[index].type == CSS_LONG) {
	orid = *(long *)((char *)table + des[index].offset);
    }
    else if(des[index].type == CSS_INT) {
	orid = *(int *)((char *)table + des[index].offset);
    }
    else {
	 return false;
    }
    if(orid < 0) return false;

    snprintf(query, 100, "select * from %s.origin where orid=%ld",
		author.c_str(), orid);
			
    if(ffdb->queryTable(query, cssOrigin, &v) && v.size() > 0)
    {
	CssOriginClass *o = (CssOriginClass*)v.at(0);
	*time = o->time;
	return true;
    }
    return false;
}

static bool
getTimeFromArid(FFDatabase *ffdb, CssTableClass *table, const string &author,
		CssClassDescription *des, double *time)
{
    char query[100];
    int index;
    long arid;
    gvector<CssTableClass *> v;

    if((index = table->memberIndex("arid")) < 0) return false;

    if(des[index].type == CSS_LONG) {
	arid = *(long *)((char *)table + des[index].offset);
    }
    else if(des[index].type == CSS_INT) {
	    arid = *(int *)((char *)table + des[index].offset);
    }
    else {
	return false;
    }
    if(arid < 0) return false;

    snprintf(query, 100, "select * from %s.arrival where arid=%ld",
		author.c_str(), arid);
			
    if(ffdb->queryTable(query, cssArrival, &v) && v.size() > 0)
    {
	CssArrivalClass *a = (CssArrivalClass *)v.at(0);
	*time = a->time;
	return true;
    }

    return false;
}

static bool
getWftagId(FFDatabase *ffdb, CssTableClass *table, const string &author,
		double *time)
{
    char query[100];
    long orid = -1;
    long arid = -1;
    CssWftagClass *w;
    gvector<CssTableClass *> v;

    if(!table->nameIs("wftag")) return false;

    w = (CssWftagClass *)table;
    if(!strcasecmp(w->tagname, "orid")) {
	orid = w->tagid;
    }
    else if(!strcasecmp(w->tagname, "arid")) {
	arid = w->tagid;
    }

    if(orid >= 0) {
	snprintf(query, 100, "select * from %s.origin where orid=%ld",
			author.c_str(), orid);
	if(ffdb->queryTable(query, cssOrigin, &v) && v.size() > 0)
	{
	    CssOriginClass *o = (CssOriginClass *)v.at(0);
	    *time = o->time;
	    return true;
	}
    }
    else if(arid >= 0) {
	snprintf(query, 100, "select * from %s.arrival where arid=%ld",
			author.c_str(), arid);
	if(ffdb->queryTable(query, cssArrival, &v) && v.size() > 0)
	{
	    CssArrivalClass *a = (CssArrivalClass *)v.at(0);
	    *time = a->time;
	    return true;
	}
    }

    return false;
}

/**
 * Insert more than one table record. 
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param num The number of records to insert from tables[].
 * @param tables The CssTableClass objects whose elements will be inserted.
 * @param author The author or station directory name.
 * @return 0 for success,  nonzero for failure.  Retrieve the error number \
 * and the error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::insertTables(int num, CssTableClass **tables,
		const string &authorOrStation)
{
    int i, err;

    for(i = 0; i < num; i++) {
	if((err = insertTable(tables[i], authorOrStation))) return err;
    }
    return 0;
}

/**
 * Write a table record to the database. Write a table record to the database
 * using the input author or station name and the input time. If the table
 * is a wfdisc, the <b>authorOrStation</b> argument should be a station name.
 * For all others tables, the <b>authorOrStation</b> argument should be an
 * author name. If the author does not exist, it will be created. If the
 * author exists, but is not writable, FFDB_AUTHOR_WRITE_ERR will be returned.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param table The CssTableClass object whose elements will be written.
 * @param authorOrStation The author or the station directory name.
 * @param time The epochal time associated with the record.
 * @return 0 for success,  nonzero for failure.  Retrieve the error number \
 * and the error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::writeTable(CssTableClass *table, const string &authorOrStation,
		double time)
{
    char dir[MAXPATHLEN+1], path[MAXPATHLEN+1], prefix[MAXPATHLEN+1];
    const char *table_name;
    const char *err_msg;
    struct stat buf;
    int index, err, param_dir;
    FILE *fp;

    /* Make a directory under the param root (or seg root for wfdiscs)
     * using the author (or station) and time.
     */
    param_dir = !table->nameIs(cssWfdisc);
    if((err = FFDBConvertDateToDir(directory_structure, param_dir,
			authorOrStation, time, dir)))
    {
	return err;
    }

    if( !param_dir ) {
	snprintf(path, MAXPATHLEN+1, "%s/%s/%s", seg_root.c_str(),
		authorOrStation.c_str(), dir);
    }
    else {
	int ret;
	ret = getAuthorWritable(authorOrStation);
	if(ret == -1) {
	    if((err = createAuthor(authorOrStation))) return err;
	}
	else if(!ret) {
	    FFDBSetErrorMsg(FFDB_AUTHOR_WRITE_ERR,
			"FFDBWriteTable: author is not writable.");
	    return FFDB_AUTHOR_WRITE_ERR;
	}
	snprintf(path, MAXPATHLEN+1, "%s/%s/%s", param_root.c_str(),
			authorOrStation.c_str(), dir);
    }

    if((err = FFDBCreateDir(path))) return err;

    table->setSource(stringToQuark("ffdb"),
		stringToQuark(param_root), stringToQuark(seg_root));
    table->setFormat(stringToQuark("css"));
    table->setDir(stringToQuark(path));

    table_name = table->getName();
    if((err = getTablePrefix(path, dir, table_name, prefix))) return err;
    table->setPrefix(stringToQuark(prefix));

    if(path[(int)strlen(path)-1] != '/') {
	strcat(path, "/");
    }
    strcat(path, prefix);
    strcat(path,".");
    strcat(path, table_name);
    table->setFile(stringToQuark(path));

    if((fp = fopen(path, "r+")) == NULL && (fp = fopen(path, "a+")) == NULL)
    {
	if(errno > 0) {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
		"FFDBWriteTable: Cannot write to: %s\n%s",
		path, strerror(errno), table_name);
	}
	else {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
		"FFDBWriteTable: Cannot write to: %s",
		path, table_name);
	}
	return FFDB_OPENW_FILE_ERR;
    }

    buf.st_size = 0;
    if(stat(path, &buf) < 0) {
	FFDBSetErrorMsg(FFDB_STAT_FILE_ERR,
			"FFDBWriteTable: Cannot stat %s", path);
	fclose(fp);
	return FFDB_STAT_FILE_ERR;
    }

    if((index = table->memberIndex("lddate")) >= 0) {
	table->setMember(index, timeLoadDate());
    }

    fseek(fp, 0, 2);

    table->setFileOffset(buf.st_size);

    if(table->write(fp, &err_msg)) {
	FFDBSetErrorMsg(FFDB_TABLE_WRITE_ERR,
			"FFDBWriteTable: write error: %s\n%s", path, err_msg);
	fclose(fp);
	return FFDB_TABLE_WRITE_ERR;
    }
    fclose(fp);

    updateFile(path, table);

    return 0;
}

/**
 * Write a static table record to the database. Write a static table record to
 * the database in the "static" directory under the parameter root.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param table The CssTableClass object whose elements will be written.
 * @return 0 for success,  nonzero for failure.  Retrieve the error number \
 * and the error message with FFDBErrMsg().
 */
int FFDatabase::writeStaticTable(CssTableClass *table)
{
    char path[MAXPATHLEN+1], prefix[MAXPATHLEN+1];
    const char *table_name;
    const char *err_msg;
    struct stat buf;
    int index, err;
    FILE *fp;

    // Make a directory named "static" under the param root.
    snprintf(path, MAXPATHLEN+1, "%s/static", param_root.c_str());

    if((err = FFDBCreateDir(path))) return err;

    table->setSource(stringToQuark("ffdb"),
		stringToQuark(param_root), stringToQuark(seg_root));
    table->setFormat(stringToQuark("css"));
    table->setDir(stringToQuark(path));

    table_name = table->getName();
    strcpy(prefix, "global");
    table->setPrefix(stringToQuark(prefix));

    if(path[(int)strlen(path)-1] != '/') {
	    strcat(path, "/");
    }
    strcat(path, prefix);
    strcat(path,".");
    strcat(path, table_name);
    table->setFile(stringToQuark(path));

    if((fp = fopen(path, "r+")) == NULL && (fp = fopen(path, "a+")) == NULL)
    {
	if(errno > 0) {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
			"FFDBWriteStaticTable: Cannot write to: %s\n%s",
			path, strerror(errno), table_name);
	}
	else {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
			"FFDBWriteStaticTable: Cannot write to: %s",
			path, table_name);
	}
	return FFDB_OPENW_FILE_ERR;
    }

    buf.st_size = 0;
    if(stat(path, &buf) < 0) {
	FFDBSetErrorMsg(FFDB_STAT_FILE_ERR,
			"FFDBWriteStaticTable: Cannot stat %s", path);
	fclose(fp);
	return FFDB_STAT_FILE_ERR;
    }

    if((index = table->memberIndex("lddate")) >= 0) {
	table->setMember(index, timeLoadDate());
    }

    fseek(fp, 0, 2);

    table->setFileOffset(buf.st_size);

    if(table->write(fp, &err_msg)) {
	FFDBSetErrorMsg(FFDB_TABLE_WRITE_ERR,
		"FFDBWriteStaticTable: write error: %s\n%s", path, err_msg);
	fclose(fp);
	return FFDB_TABLE_WRITE_ERR;
    }
    fclose(fp);

    updateFile(path, table);

    return 0;
}

int FFDatabase::updateFile(char *path, CssTableClass *table)
{
    int i, path_q;

    path_q = stringToQuark(path);
    for(i = 0; i < (int)mem_files.size() &&
		path_q != mem_files[i]->filename_q; i++);

    if(i == (int)mem_files.size()) return 0;

    mem_files[i]->records->push_back(table->clone());

    if(stat(path, &mem_files[i]->file_stat)) {
	FFDBSetErrorMsg(FFDB_STAT_FILE_ERR,
			"FFDBUpdateFile: Cannot stat %s", path);
	return FFDB_STAT_FILE_ERR;
    }
    return 0;
}

/**
 * Create an author directory.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param author An author name.
 * @return 0 for success, FFDB_CREATE_DIR_ERR for failure.  Retrieve the \
 * error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::createAuthor(const string &author)
{
    char dir[MAXPATHLEN+1];

    snprintf(dir, MAXPATHLEN, "%s/%s", param_root.c_str(), author.c_str());
    return FFDBCreateDir(dir);
}

/**
 * Create a database directory. Create all directories in the input path.
 * @param path A database path.
 * @return 0 for success,  FFDB_CREATE_DIR_ERR for failure.  Retrieve  the \
 * error message with FFDBErrMsg().
 */
static int
FFDBCreateDir(const char *path)
{
	int i;
	char c, dir[MAXPATHLEN+1];
	struct stat buf;

	stringcpy(dir, path, MAXPATHLEN+1);
	dir[MAXPATHLEN] = '\0';

	for(i = 0; dir[i] == '/'; i++);

	while(dir[i] != '\0')
	{
	    while(dir[i] != '\0' && dir[i] != '/') i++;
	    c = dir[i];
	    dir[i] = '\0';
	    if(stat(dir, &buf) && mkdir(dir, 0775) != 0) {
		FFDBSetErrorMsg(FFDB_CREATE_DIR_ERR,
			"FFDBCreateDir: Cannot create directory %s", dir);
		return FFDB_CREATE_DIR_ERR;
	    }
	    dir[i] = c;
	    while(dir[i] == '/') i++;
	}
	return 0;
}

static int
getTablePrefix(const char *path, const char *dir, const string &tableName,
		char *prefix)
{
    int i, j;
    char *c;
    DIR *dirp;
    struct dirent *dp;

    // look for a table file
    if((dirp = opendir(path)) == NULL) {
	FFDBSetErrorMsg(FFDB_OPEN_DIR_ERR,
			"FFDBWriteTable: Cannot open %s", path);
	return FFDB_OPEN_DIR_ERR;
    }

    while((dp = readdir(dirp)) != NULL)
	if(dp->d_name[0] != '.' && dp->d_name[0] != ' ')
    {
	if((c = rindex(dp->d_name, '.')) != NULL &&
		!strcasecmp(c+1, tableName.c_str()))
	{
	    strncpy(prefix, dp->d_name, c - dp->d_name);
	    prefix[c - dp->d_name] = '\0';
	    closedir(dirp);
	    return 0;
	}
    }
    closedir(dirp);

    // Use the directory structure as the prefix
    for(i = j = 0; dir[i] != '\0'; i++) {
	if(!isspace((int)dir[i]) && dir[i] != '/' && dir[i] != ':'
			&& dir[i] != ';') prefix[j++] = dir[i];
    }
    prefix[j] = '\0';
    return 0;
}

char *strptime(const char *s, const char *format, struct tm *tm);

/**
 * Convert a time to a database directory path. Using the input
 * <b>directory_structure</b> as an strptime format, return the directory path.
 * @param directory_structure An strftime/strptime format for the database
 *	directories.
 * @param param_dir 1 for the parameter directory, 0 for the segment directory.
 * @param authorOrStation The author or the station directory name.
 * @param time An epochal time.
 * @param dir The output directory path.
 * @return 0 for success,  FFDB_BAD_STRUCTURE for failure.  Retrieve the  \
 * error message with FFDBErrMsg().
 */
static int
FFDBConvertDateToDir(const string &directory_structure, int param_dir,
		const string &authorOrStation, double time, char *dir)
{
    char format[MAXPATHLEN+1];
    DateTime dt;
    char buf[100], *c;
    struct tm tm;

    timeEpochToDate(time, &dt);

    tm.tm_sec = (int)dt.second;
    tm.tm_min = dt.minute;
    tm.tm_hour = dt.hour;
    tm.tm_year = dt.year - 1900;
    tm.tm_mon = dt.month - 1;
    tm.tm_mday = dt.day;
    tm.tm_wday = 0; // get this below
    tm.tm_yday = 0; // get this below
    tm.tm_isdst = 0;

    /* Print the year/month/day/hour/minute/second and call
     * strftime to get all of the tm fields (including day of week,
     * and day of year).
     */
    strftime(buf, 100, "%Y/%m/%d/%H/%M/%S", &tm);

    // Convert back to struct tm with all fields filled.
    if((c = strptime(buf, "%Y/%m/%d/%H/%M/%S", &tm)) == NULL || *c != '\0')
    {
	FFDBSetErrorMsg(FFDB_BAD_STRUCTURE,
			"FFDBConvertDateToDir: strptime failed.");
	return FFDB_BAD_STRUCTURE;
    }

    if(directory_structure.empty()) {
	/* if ds is empty, then use the format:
	 * station/stationyyyymmdd for the segment directory
	 * authoryyyymmdd for the parameter directory.
	 */
	if(param_dir) {
	    snprintf(format, sizeof(format), "%s%%Y%%m%%d",
			authorOrStation.c_str());
	}
	else {
	    snprintf(format, sizeof(format), "%s/%s%%Y%%m%%d",
			authorOrStation.c_str(), authorOrStation.c_str());
	}
    }
    else {
	snprintf(format, sizeof(format), "%s", directory_structure.c_str());
    }

    // Print again using the directory_structure format.
    strftime(dir, MAXPATHLEN, format, &tm);

    return 0;
}

int FFDatabase::insertPrefixTable(CssTableClass *table)
{
    char path[MAXPATHLEN+1];
    const char *cssTableName = table->getName(), *err_msg;
    struct stat buf;
    FILE *fp;
    int ret, index;

    snprintf(path, sizeof(path), "%s.%s", param_root.c_str(), cssTableName);

    table->setSource(stringToQuark("ffdb"),
		stringToQuark(param_root), stringToQuark(seg_root));
    table->setFormat(stringToQuark("css"));
    table->setDir(stringToQuark(path));

    table->setPrefix(stringToQuark(param_root));

    table->setFile(stringToQuark(path));

    if((fp = fopen(path, "r+")) == NULL && (fp = fopen(path, "a+")) == NULL)
    {
	if(errno > 0) {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
		"FFDBInsertPrefixTable: Cannot write to: %s\n%s",
		path, strerror(errno), cssTableName);
	}
	else {
	    FFDBSetErrorMsg(FFDB_OPENW_FILE_ERR,
		"FFDBInsertPrefixTable: Cannot write to: %s",
		path, cssTableName);
	}
	return FFDB_OPENW_FILE_ERR;
    }

    buf.st_size = 0;
    if(stat(path, &buf) < 0) {
	FFDBSetErrorMsg(FFDB_STAT_FILE_ERR,
			"FFDBInsertPrefixTable: Cannot stat %s", path);
	fclose(fp);
	return FFDB_STAT_FILE_ERR;
    }

    if((index = table->memberIndex("lddate")) >= 0) {
	    table->setMember(index, timeLoadDate());
    }

    fseek(fp, 0, 2);

    table->setFileOffset(buf.st_size);

    ret = table->write(fp, &err_msg);

    fclose(fp);

    updateFile(path, table);

    return ret;
}

/**
 * Routines to request ids from the lastid table.
 */

typedef struct
{
	int	startValue;
	int	numValues;
} IdSegment;

typedef struct
{
	string		keyname;
	int 		num_segments;
	IdSegment	*segment;
	int		idIncrement;
} RecycledIds;

static int numKeys = 0;
static RecycledIds *recycled;

static RecycledIds *getRecycledList(const string &keyname);
static int addRecycleSegment(RecycledIds *r, int startValue, int numValues);
extern "C" {
static int sortIdSegment(const void *A, const void *B);
}

/**
 * Set the lastid request increment for keyname. For the input keyname, set
 * the minimum number of ids that will actually be requested from the lastid
 * table each time a request is made with FFDBGetNextId or FFDBRequestIds.
 * Extra ids are saved in the recycle list.
 * @param keyname The keyname of the lastid record ("arid", "orid", etc.).
 * @param increment The minimum number of ids to request from the database.
 * @return 0 for success, -1 if increment <= 0 or malloc fails. Retrieve the \
 * 	error number and error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::setRequestIdIncrement(const string &keyname, int idIncrement)
{
    RecycledIds *r;

    if(idIncrement <= 0) {
	FFDBSetErrorMsg(FFDB_BAD_INCREMENT,
		"setRequestIdIncrement: invalid idIncrement.");
	return -1;
    }

    if((r = getRecycledList(keyname)) == NULL) return -1;

    r->idIncrement = idIncrement;

    return 0;
}

static RecycledIds *
getRecycledList(const string &keyname)
{
    int i;

    // Get the recycled id list for the input keyname.
    for(i=0; i < numKeys && recycled[i].keyname.compare(keyname.c_str()); i++);
    if(i < numKeys) {
	return &recycled[i];
    }
    else
    {
	if(!numKeys) {
		recycled = (RecycledIds *)malloc(sizeof(RecycledIds));
	    }
	    else {
		recycled = (RecycledIds *)realloc(recycled,
					(numKeys+1)*sizeof(RecycledIds));
	    }
	    if(!recycled) {
		FFDBSetErrorMsg(FFDB_MALLOC_ERR,
			"getRecycledList: malloc failed.");
		return NULL;
	    }
	    recycled[numKeys].keyname.assign(keyname);
	    recycled[numKeys].num_segments = 0;
	    recycled[numKeys].segment = NULL;
	    recycled[numKeys].idIncrement = 1;
	    return &recycled[numKeys++];
	}
}

/**
 * Get an id (keyvalue) from the lastid table and increment it. The id is taken
 * from the recycled-id-list, if available. Otherwise, the keyvalue of the
 * lastid record corresponding to the <b>tableName</b> and <b>keyname</b> is
 * returned and increased by the <b>idIncrement</b> value. The
 * <b>idIncrement</b> defaults to 1 for all keynames. Use
 * setRequestIdIncrement to set the <b>idIncrement</b>.
 * @param ffdb An FFDatabase pointer from FFDBOpen().
 * @param tableName The name of the lastid table including the account, \
 *	if necessary.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @return The keyvalue (next id) or -1 for failure. Retrieve the error number \
 *	and error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::getNextId(const string &tableName, const string &keyname)
{
    int nextid[1], numReturned;

    if( !requestIds(tableName, keyname, 1, 0, nextid, &numReturned) ) {
	return nextid[0];
    }
    return -1;
}

/**
 * Request one or more ids from the lastid table. The requested number of
 * ids are taken from the recycled-id-list, if available. If necessary,
 * ids are taken from the lastid table. The keyvalue of the lastid table
 * is incremented by the larger of the number of ids needed or the
 * <b>idIncrement</b>.
 * @param ffdb An FFDatabase pointer from FFDBOpen().
 * @param tableName The name of the lastid table including the account, \
 *	 if necessary.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @param numRequested The number of ids requested.
 * @param consecutive Set to 1 to request consecutive ids. Set to 0 to request \
 *	any available ids.
 * @param ids An array to hold the returned <b>numRequested</b> ids.
 * @param numReturned The actual number of ids returned in <b>ids</b>.
 * @return 0 for success, -1 for failure. Retrieve the error number \
 *      and error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::requestIds(const string &tableName, const string &keyname,
		int numRequested, int consecutive, int *ids, int *numReturned)
{
    int nextid=0, need;
    int i, n, ret;

    // First check the recycled ids.
    ret = getRecycledIds(tableName, keyname, numRequested, consecutive, ids,
		numReturned);
    if(ret < 0) return ret;
    if(numRequested == *numReturned) return 0;

    // Need more ids from the lastid table
    need = numRequested - *numReturned;
    ret = updateLastid(tableName, keyname, need, &nextid);
    if(ret) {
	return ret;
    }
    n = *numReturned;
    for(i = 0; i < need; i++) {
	ids[n++] = nextid++;
    }
    *numReturned = numRequested;
    return 0;
}

int FFDatabase::getRecycledIds(const string &tableName, const string &keyname,
		int numRequested, int consecutive, int *ids, int *numReturned)
{
    int i, j, ret;
    RecycledIds *r;

    *numReturned = 0;

    // Get the recycled id list for the input keyname.
    for(j = 0; j < numKeys && recycled[j].keyname.compare(keyname); j++);
    if(j == numKeys) return 0;

    r = &recycled[j];

    if(consecutive)
    {
	// search for numRequested consecutive ids in the recycled list.
	for(i = 0; i < r->num_segments &&
		r->segment[i].numValues < numRequested; i++);
	if(i < r->num_segments) 
	{
	    // found a segment with >= numRequested ids.
	    for(j = 0; j < numRequested; j++) {
		ids[j] = r->segment[i].startValue++;
		r->segment[i].numValues--;
	    }
	    if(r->segment[i].numValues == 0) {
		for(j = i+1; j < r->num_segments; j++) {
		    r->segment[j-1] = r->segment[j];
		}
		r->num_segments--;
	    }
	    *numReturned = numRequested;
	}
	else if(r->num_segments)
	{
	    int lastid=0, need;
	    /* Did not find numRequested consecutive ids in the recycled
	     * list. Check if the database lastid is consecutive with the 
	     * last recycled segment.
	     */
	    j = r->num_segments - 1;
	    lastid = r->segment[j].startValue + r->segment[j].numValues - 1;
	    need = numRequested - r->segment[j].numValues;
	    ret = getLastId(tableName, keyname, need, &lastid, 1);
	    if(ret < 0) return ret; /* error return */
	    if(!ret) {
		/* The database lastid was consecutive with the last
		 * recycle segment. Use the last recycle segment plus
		 * some ids from the database.
		 */
		for(j = 0; j < numRequested; j++) {
		    ids[j] = r->segment[j].startValue++;
		}
		for(j = i+1; j < r->num_segments; j++) {
		    r->segment[j-1] = r->segment[j];
		}
		r->num_segments--;
		*numReturned = numRequested;
	    }
	}
    }
    else {
	/* ids do not need to be consecutive. Simple return numRequested
	 * ids from the recycled list, or all that there is.
	 */
	int n = 0;
	for(i = 0; i < r->num_segments && n < numRequested; i++)
	{
	    for(j = 0; j < r->segment[i].numValues && n < numRequested; j++)
	    {
		ids[n++] = r->segment[i].startValue++;
	    }
	    r->segment[i].numValues -= j;
	}
	for(i = j = 0; i < r->num_segments; i++) {
	    if(r->segment[i].numValues) {
		r->segment[j++] = r->segment[i];
	    }
	}
	r->num_segments = j;
	*numReturned = n;
    }
    return 0;
}

/**
 * Update the lastid table. The keyvalue of the lastid table is returned in
 * <b>keyvalue</b> and incremented by <b>numRequested</b>.
 * @param ffdb An FFDatabase pointer from FFDBOpen().
 * @param tableName The name of the lastid table including account if necessary.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @param numRequested The number of ids requested.
 * @return 0 for success, -1 for error. Retrieve the error number \
 *	and error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::updateLastid(const string &tableName, const string &keyname,
		int numRequested, int *keyvalue)
{
    return getLastId(tableName, keyname, numRequested, keyvalue, 0);
}

int FFDatabase::getLastId(const string &tableName, const string &keyname,
		int numRequested, int *nextid, int consecutive)
{
    const char *filename;
    const char *err_msg;
    int offset, err;
    long keyvalue;
    FILE *fp;
    RecycledIds *r;
    CssLastidClass lastid;

    if((r = getRecycledList(keyname)) == NULL) return -1;

    if((filename = getStaticTablePath(tableName)) == NULL) {
	FFDBSetErrorMsg(FFDB_NO_LASTID, "getLastId: no %s table file.",
				tableName.c_str());
	return -1;
    }
    if((fp = fopen(filename, "r+")) == NULL) {
	if(errno > 0) {
	    FFDBSetErrorMsg(FFDB_STAT_FILE_ERR, "Cannot open %s\n%s",
			filename, strerror(errno));
	}
	else {
	    FFDBSetErrorMsg(FFDB_STAT_FILE_ERR, "Cannot open %s", filename);
	}
	return -1;
    }

    offset = 0;
    while(!(err = lastid.read(fp, &err_msg)))
    {
	if(!keyname.compare(lastid.keyname)) break;
	offset = ftell(fp);
    }

    if(err == 0)
    {
	// found keyname
	keyvalue = lastid.keyvalue;
    }
    else if(err == EOF)
    {
	keyvalue = 0;
	strncpy(lastid.keyname, keyname.c_str(), sizeof(lastid.keyname));
	lastid.keyname[sizeof(lastid.keyname)-1] = '\0';
    }
    else
    {
	FFDBSetErrorMsg(FFDB_TABLE_READ_ERR,
			"getLastId: format error in %s\n%s", filename, err_msg);
	fclose(fp);
	return(-1);
    }

    // If consecutive, the database keyvalue must = *nextid
    if(consecutive && keyvalue != *nextid)
    {
	return 1;
    }
    *nextid = keyvalue + 1;

    if(numRequested >= r->idIncrement) {
	lastid.keyvalue = keyvalue + numRequested;
    }
    else {
	lastid.keyvalue = keyvalue + r->idIncrement;
    }

    fseek(fp, offset, 0);
    timeEpochToDate(timeGetEpoch(), &lastid.lddate);
    err = lastid.write(fp, &err_msg);

    fclose(fp);

    // add extra ids to the recycled list
    if(numRequested < r->idIncrement) {
	if(addRecycleSegment(r, keyvalue + numRequested + 1,
		r->idIncrement - numRequested)) return -1;
    }
    return (err);
}

/**
 * Recycle ids from the lastid table. Give ids to the recycle list to be used
 * by FFDBGetNextId and FFDBRequestIds.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @param ids An array of ids.
 * @param numReturned The number <b>ids</b> to put in the recycle list.
 * @return 0 for success, -1 if malloc fails. Retrieve the error number \
 *	and error message with FFDBErrno() and FFDBErrMsg().
 */
int FFDatabase::recycleIds(const string &keyname, int *ids, int numReturned)
{
    int i, j;
    RecycledIds *r;

    if((r = getRecycledList(keyname)) == NULL) return -1;

    for(i = 0; i < numReturned; ) {
    for(j = i+1; j < numReturned && ids[j] == ids[j-1]+1; j++);
	if(addRecycleSegment(r, ids[i], j-i)) return -1;
	i = j;
    }
    return 0;
}

static int
addRecycleSegment(RecycledIds *r, int startValue, int numValues)
{
    int i, j;

    if(r->segment == NULL) {
	r->segment = (IdSegment *)malloc(sizeof(IdSegment));
    }
    else {
	r->segment = (IdSegment *)realloc(r->segment,
				(r->num_segments+1)*sizeof(IdSegment));
    }
    if(!r->segment) {
	FFDBSetErrorMsg(FFDB_MALLOC_ERR,
			"addRecycleSegment: malloc failed.");
	return -1;
    }
    r->segment[r->num_segments].startValue = startValue;
    r->segment[r->num_segments].numValues = numValues;
    r->num_segments++;

    // Sort segments.
    qsort(r->segment, r->num_segments, sizeof(IdSegment), sortIdSegment);
	
    // Join segments that are consecutive.
    for(i = r->num_segments-1; i > 0; i--)
    {
	if(r->segment[i].startValue ==
		r->segment[i-1].startValue + r->segment[i-1].numValues)
	{
	    r->segment[i-1].numValues += r->segment[i].numValues;
	    for(j = i; j < r->num_segments-1; j++) {
		r->segment[j] = r->segment[j+1];
	    }
	    r->num_segments--;
	}
    }
    return 0;
}

static int
sortIdSegment(const void *A, const void *B)
{
	const IdSegment *a = (const IdSegment *)A;
	const IdSegment *b = (const IdSegment *)B;

	return (a->startValue < b->startValue ? -1 : 1);
}

/**
 * Get the number of recycled ids available. For the input keyname, return the
 * number of ids in the recycled-id-list.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @return The number of ids in the recycled-id-list.
 */
int FFDatabase::numberIDsAvailable(const string &keyname)
{
    int i, num;
    RecycledIds *r;

    if((r = getRecycledList(keyname)) == NULL) return 0;

    num = 0;
    for(i = 0; i < r->num_segments; i++) {
	num += r->segment[i].numValues;
    }
    return num;
}

/* 
This is the table I made with the current IDC sitechan list. The first 
column is all the channel names, the second column has an entry for the 
2 letter channel names which may have a 3 letter equivalent. Do you see
anything missing?

idc029: awk '{print $2}' *chan | sort -u | tr "[:lower:]" "[:upper:]" 

BE      BHE
BEL
BN      BHN
BNL
BZ      BHZ
BZL
CB              (coherent beam)
ED      EDH     (WK30)
EE      EHE     (YKW3)
EN      EHN
EZ      EHZ
FKB
HB              horizontal incoherent beam
HE              GEC2 - only station
HN              GEC2 - only station
HZ              GEC2 - only station
IB              vertical incoherent beam
LE      LHE
LN      LHN
LZ      LHZ
MD              mid-period pressure WRAIx only (infrasound)
ME      MHE     medium-period
MN      MHN
MX              cross correlation beam (infrasound)
MZ      MHZ
SD              short-period pressure (infrasound)
SE      SHE
SN      SHN
SP              short-period pressure (infrasound, PSUR0 only)
SZ      SHZ
SZL
WKO
WWD
WWS
*/

typedef struct
{
	char	c[9];
	char	c_alt[9];
	int	q;
	int	q_alt;
} ChanAlt;

#define	NCHAN	16

static void
addChannelAlternate(gvector<CssTableClass *> &sitechan)
{
    int	i, j;
    int	done;
    CssSitechanClass *sc;
    static ChanAlt chan[NCHAN] = {
	{"BZ", "BHZ", 0, 0}, 
	{"BN", "BHN", 0, 0},
	{"BE", "BHE", 0, 0},
	{"ED", "EDH", 0, 0},
	{"EE", "EHE", 0, 0},
	{"EN", "EHN", 0, 0},
	{"EZ", "EHZ", 0, 0},
	{"LE", "LHE", 0, 0},
	{"LN", "LHN", 0, 0},
	{"LZ", "LHZ", 0, 0},
	{"ME", "MHE", 0, 0},
	{"MN", "MHN", 0, 0},
	{"MZ", "MHZ", 0, 0},
	{"SE", "SHE", 0, 0},
	{"SN", "SHN", 0, 0},
	{"SZ", "SHZ", 0, 0}
    };

    // if the quark for the first chan is empty, it means that 
    // we need to initialize all quarks

    if(chan[0].q == 0) {
	for(i = 0; i < NCHAN; i++) {
	    chan[i].q = stringUpperToQuark(chan[i].c);
	    chan[i].q_alt = stringUpperToQuark(chan[i].c_alt);
	}
    }

    for(i = 0; i < sitechan.size(); i++)
    {
	sc = (CssSitechanClass *)sitechan[i];
	done = 0;
	for(j = 0; j < NCHAN; j++)
	{
	    if(sc->chan_quark == chan[j].q) {
		sc->chan_alt = chan[j].q_alt;
		done = 1;
		break;
	    }
	    else if (sc->chan_quark == chan[j].q_alt) {
		sc->chan_alt = chan[j].q;
		done = 1;
		break;
	    }
	}
		
	// if this chan (sc->chan) was not on the list,
	// just set the chan_alt to be the same as chan
	if(!done) {
	    sc->chan_alt = sc->chan_quark;
	}
    }
}
