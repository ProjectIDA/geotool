#ifndef _FFDATABASE_H_
#define _FFDATABASE_H_

#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>

using namespace std;

#include "gobject++/cvector.h"
#include "gobject++/CssTables.h"

#define MAX_Q_LEN	8192
#define MAX_N_EXPR	220
#define MAX_TABLES      20

enum FFDB_ERROR {
    FFDB_NO_ERROR,
    FFDB_NO_PARAM_ROOT,
    FFDB_NO_SEG_ROOT,
    FFDB_BAD_STRUCTURE,
    FFDB_CREATE_DIR_ERR,
    FFDB_OPEN_DIR_ERR,
    FFDB_STAT_FILE_ERR,
    FFDB_OPENR_FILE_ERR,
    FFDB_OPENW_FILE_ERR,
    FFDB_AUTHOR_ERR,
    FFDB_TABLE_READ_ERR,
    FFDB_TABLE_WRITE_ERR,
    FFDB_AUTHOR_WRITE_ERR,
    FFDB_INVALID_QUERY,
    FFDB_INVALID_UPDATE,
    FFDB_NO_UPDATE_FILE,
    FFDB_NO_DELETE_FILE,
    FFDB_NO_TIME,
    FFDB_IMPORT_ERR,
    FFDB_PTHREAD_ERR,
    FFDB_SEM_INIT_ERR,
    FFDB_SEM_POST_ERR,
    FFDB_JOIN_ERR,
    FFDB_MALLOC_ERR,
    FFDB_NO_LASTID,
    FFDB_BAD_INCREMENT
};

class AuthorStruct
{
    public:
    string	name;
    double	tmin;
    double	tmax;
    bool	writable;
    AuthorStruct(const string &filename) {
	name = filename;
	tmin = NULL_TIME;
	tmax = NULL_TIME;
	writable = true;
    }
    ~AuthorStruct(void) { }
};

class StaticTable
{
    public:
    string	name;
    string	path;
    string	path2;
    struct stat	stat;
    struct stat	stat2;
    gvector<CssTableClass *> records;
    gvector<CssTableClass *> records2;

    StaticTable(const string &table_name, const string &table_path) {
	name = table_name;
	path = table_path;
	stat.st_ino = 0;
        stat.st_dev = 0;
	stat2.st_ino = 0;
        stat2.st_dev = 0;
    }
    StaticTable(const string &table_name, const string &table_path,
		const string &table_path2) {
	name = table_name;
	path = table_path;
	path2 = table_path2;
	stat.st_ino = 0;
        stat.st_dev = 0;
	stat2.st_ino = 0;
        stat2.st_dev = 0;
    }
    ~StaticTable(void) { }
};

class FFDatabase;
class FFDB_FILE;

class QTable
{
    public:
    char	*author;	/* table author (or account) */
    char	*name;		/* table name (from the query) */
    char	*ref;		/* a query reference to the table */
    int		num_members;	/* number of members in the table */
    CssClassDescription *des;	/* member descriptions */
    gvector<CssTableClass *>	all;		/* record list */
    gvector<CssTableClass *>	constrained;	/* constrained record list */
    double	tmin;		/* time constraints on this table */
    double	tmax;		/* time constraints on this table */
    QTable() {
	author = NULL;
	name = NULL;
	ref = NULL;
	num_members = 0;
	des = NULL;
	tmin = 0.;
	tmax = 0.;
    }
    ~QTable() {
//	Free(author);
//	Free(name);
//	Free(ref);
    }
};

/* The QTerm structure contains information about a single term of a "where"
 * constraint clause
 */
typedef struct
{
    char	*ref;	/* table reference ("a" in "affiliation a") */
    char	*member;/* member name ("sta" in "a.sta") */
    int		sign;	/* 1 or -1 for the sign of the term */
    double	value;	/* numerical terms are converted to double */
    int		index;	/* index of the member the term refers to */
    QTable	*table; /* table containing the member */
    CssTableClass 	*t;	/* an instance of the table during the search */
} QTerm;

typedef struct
{
    const char *operation;	/* '=', '<=', '>=', "like", "in", etc */
    QTerm	a;		/* the left side of a single constraint */
    int		nb;		/* the number of terms on the right side */
    QTerm	b[MAX_TABLES];		/* each term on the right side */
    char	*like[MAX_N_EXPR];	/* string(s) for the "like" operator */
    char	*in[MAX_N_EXPR + 1];	/* string(s) for the "in" operator */
    long	inl[MAX_N_EXPR];
    int		nl;
} QConstraint;

class QParseStruct
{
    public:

    QParseStruct()
    {
	QTerm qterm_null = {NULL, NULL, 1, 0., 0, NULL, NULL};

	qbuf = NULL;
	distinct = false;
	num_ref = 0;
	num_tables = 0;
	num_constraints = 0;

	for(int i = 0; i < MAX_TABLES; i++)
	{
	    refs[i] = qterm_null;
	    tables[i] = NULL;

	    constraints[i].operation = NULL;
	    constraints[i].a = qterm_null;
	    constraints[i].nb = 0;
	    for(int j = 0; j < MAX_TABLES; j++) {
		constraints[i].b[j] = qterm_null;
	    }
	    for(int j = 0; j < MAX_N_EXPR; j++) {
		constraints[i].like[j] = NULL;
		constraints[i].in[j] = NULL;
		constraints[i].inl[j] = 0;
	    }
	    constraints[i].nl = 0;
	}
    }
    char *qbuf;
    bool distinct;
    int num_ref, num_tables, num_constraints;
    QTerm refs[MAX_TABLES];
    QTable *tables[MAX_TABLES];
    QConstraint constraints[MAX_TABLES];
};

class FFDBQuery
{
    public:
	FFDBQuery(FFDatabase *ffdb);
	~FFDBQuery(void);
	int getResults(int numToFetch, int *numFetched,
			gvector<CssTableClass *> *records);

	FFDatabase	*ffdb;
	QParseStruct	parse_struct;
	string		table_name;
	string		prefix;
	int		data_source_q;
	int		param_root_q;
	int		seg_root_q;
	int		format_q;
	int		buffer_limit;
	bool		reading_secondary;
	bool		search_error;
	int		num_fetched;
	gvector<CssTableClass *> *records;
	sem_t		search_sem;
	sem_t		results_sem;
	pthread_t	thread;

	bool checkTables(void);
	bool wfdiscSearch(int num_constraints, QConstraint *con, int num_tables,
		QTable **tables, gvector<CssTableClass *> &r);
	bool searchFlatFileDB(const string &path, int num_constraints,
		QConstraint *c, int num_tables, QTable **t,
		const string &tableName, gvector<CssTableClass *> &records);
	bool doStaticPrimary(int num_c, QConstraint *c, int num_tables,
		QTable **tables, const string &tableName,
		gvector<CssTableClass *> &r);
	bool getRecords(const string &root, const char *name, int num_c,
		QConstraint *c, int num_tables, QTable **tables,
		const string &tableName, gvector<CssTableClass *> &r);
	bool readFile(const char *path, int num_c, QConstraint *c,
		int num_tables, QTable **tables, const string &tableName,
		gvector<CssTableClass *> &r);
	bool searchPrefix(void);
	bool prefixInit(const string &query, const string &cssTableName);
	bool checkPrefixTables(void);
	bool readSecondaryPrefixTables(int *num_constraints, QConstraint *con,
		int num_tables, QTable **tables);
	bool readSecondaryTables(int *num_c, QConstraint *c, int num_tables,
		QTable **tables);
	int processFile(FFDB_FILE *fp, const char *path, int num_c,
		QConstraint *c, int num_tables, QTable **tables,
		const string &tableName, gvector<CssTableClass *> &r,
		CssTableClass **pcss);
	bool searchFlatDB(const string &root, const char *name,int num_levels,
		int *level, int num_c, QConstraint *c, int num_tables,
		QTable **tables, const string &tableName,
		gvector<CssTableClass *> &r);
};

/*
 *  @ingroup libgio
 */
class FFDatabase
{
    friend class FFDBQuery;

    public:
        ~FFDatabase(void);

	static FFDatabase *FFDBOpen(const string &param_root,
		const string &seg_root, const string &directory_structure,
		double directory_duration);
	static FFDatabase *FFDBOpenPrefix(const string &prefix);

	int getAuthors(char ***authors);
	bool setDefaultAuthor(const string &author);
/**
 * Get the default author. Get the default author that is used in queries where
 * an author is not prefixed to a table name.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @return The default author. Free after use.
 */
	const char *defaultAuthor(void) {
	    if(default_author >= 0 && default_author < (int)authors.size()) {
		return authors[default_author]->name.c_str();
	    }
	    return NULL;
	}

	bool getAuthorWritable(const string &author);
	bool setAuthorWritable(const string &author, int mode);
	double directoryDuration(void) { return directory_duration; }
	void setDirectoryDuration(double duration) {
	    directory_duration = duration; }
	void setMaxMemRecords(int max_records) {
		max_mem_file_records = max_records; }
	const char *paramRoot(void) { return param_root.c_str(); }

	bool queryTable(const string &query, const string &tableName,
		gvector<CssTableClass *> *r);
	FFDBQuery *startQuery(const string &query, const string &cssTableName);
/**
 * Get query time limits. Return the time limits, if there were any, for the
 * most recent query on the database. If no time limit was used, tmin and/or
 * tmax will be returned as NULL_TIME.
 * @param ffdb
 * @param tmin The returned minimum time limit or NULL_TIME.
 * @param tmin The returned maximum time limit or NULL_TIME.
 * @return true if both tmin and tmax are available. Returns false if one or both are set to NULL_TIME.
 */
	bool timeLimits(double *tbeg, double *tend) {
	    *tbeg = tmin;
	    *tend = tmax;
	    return tmin>NULL_TIME_CHECK && tmax>NULL_TIME_CHECK ? true:false;
	}

	bool queryPrefix(const string &query, const string &cssTableName,
		gvector<CssTableClass *> *table);
	bool update(CssTableClass *t);

	int insertTable(CssTableClass *table, const string &author);
	int insertTables(int num, CssTableClass **tables, const string &author);
	int insertPrefixTable(CssTableClass *table);

	StaticTable *getStaticTable(const string &cssTableName);
	StaticTable *addStaticTable(const string &cssTableName,
		const string &path, const string &path2="");
	int readStaticTable(const string &cssTableName, gvector<CssTableClass *> &v,
		char **static_table_path);
/**
 * Global static tables control. Global static tables (global.site, etc) are
 * normally read in addition to any local static tables. Use this function to
 * control this behavior.
 * @param ffdb An FFDatabase pointer obtained from FFDBOpen().
 * @param read_globals if true, global table files are read. If false,
 *  no global files are read. Only local static files will be read.
 */
	void setReadGlobalTables(bool read_globals) {
		read_global_tables = read_globals; }
	void defineStaticTable(const string &cssTableName);
	void defineStaticTable2(const string &cssTableName);
	bool setStaticTable(const string &cssTableName, const string &path,
			const string &path2="");
	const char *getStaticTablePath(const string &cssTableName);
	const char *getNetwork(const char *sta);
	const char *getLikeNet(const char *sta);
	void clearTables(void);

	static char *FFDBErrMsg(void);
	static int FFDBErrno(void);

	static int setRequestIdIncrement(const string &keyname,
		int idIncrement);
	int getNextId(const string &tableName, const string &keyname);
	int requestIds(const string &tableName, const string &keyname,
		int numRequested, int consecutive, int *ids, int *numReturned);
	int updateLastid(const string &tableName,const string &keyname,
		int numRequested, int *keyvalue);
	static int recycleIds(const string &keyname, int *ids,
		int numReturned);
	static int numberIDsAvailable(const string &keyname);

	FFDB_FILE *openFile(const string &tableName, const char *path);
	static void clearStaticTables(void);

    protected:
	string			param_root;
	string			seg_root;
	string			directory_structure;
	vector<StaticTable *>	static_tables;
	vector<AuthorStruct *>	authors;
	int			default_author;
	double			directory_duration;
	double			tmin;
	double			tmax;
	vector<FFDB_FILE *>	mem_files;
	int			mem_file_records;
	int			max_mem_file_records;
	bool			read_global_tables;
	bool			prefix_files;

        FFDatabase(void);
	int updateFile(char *path, CssTableClass *table);
	int writeStaticTable(CssTableClass *table);
	int writeTable(CssTableClass *table,const string &authorOrStation,
			double time);
	int createAuthor(const string &author);
	int getRecycledIds(const string &tableName, const string &keyname,
		int numRequested, int consecutive, int *ids, int *numReturned);
	int getLastId(const string &tableName, const string &keyname,
		int numRequested, int *nextid, int consecutive);
};

#endif
