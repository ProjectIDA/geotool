/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_FFDB_H_
#define	_LIB_FFDB_H_

#include "gobject/CssTable.h"
#include "gobject/Vector.h"
#include "libtime.h"

#define MAX_Q_LEN       8192
#define MAX_N_EXPR      220
#define MAX_TABLES      20

#define FFDB_NO_PARAM_ROOT	1
#define FFDB_NO_SEG_ROOT	2
#define FFDB_BAD_STRUCTURE	3
#define FFDB_CREATE_DIR_ERR	4
#define FFDB_OPEN_DIR_ERR	5
#define FFDB_STAT_FILE_ERR	6
#define FFDB_OPENR_FILE_ERR	7
#define FFDB_OPENW_FILE_ERR	8
#define FFDB_AUTHOR_ERR		9
#define FFDB_TABLE_READ_ERR	10
#define FFDB_TABLE_WRITE_ERR	11
#define FFDB_AUTHOR_WRITE_ERR	12
#define FFDB_INVALID_QUERY	13
#define FFDB_INVALID_UPDATE	14
#define FFDB_NO_UPDATE_FILE	15
#define FFDB_NO_DELETE_FILE	16
#define FFDB_NO_TIME		17
#define FFDB_IMPORT_ERR		18
#define FFDB_PTHREAD_ERR	19
#define FFDB_SEM_INIT_ERR	20
#define FFDB_SEM_POST_ERR	21
#define FFDB_JOIN_ERR		22
#define FFDB_MALLOC_ERR		23
#define FFDB_NO_LASTID		24
#define FFDB_BAD_INCREMENT	25

typedef struct
{
	char		*name;
	double		tmin;
	double		tmax;
	bool		writable;
} AuthorStruct;

typedef struct
{
	char		*author;	/* table author (or account) */
	char		*name;		/* table name (from the query) */
	char		*ref;		/* a query reference to the table */
	int		num_members;	/* number of members in the table */
	CssDescription	*des;		/* member descriptions */
	Vector		all;		/* record list */
	Vector		constrained;	/* constrained record list */
	double		tmin;		/* time constraints on this table */
	double		tmax;		/* time constraints on this table */
} QTable;

/* The QTerm structure contains information about a single term of a "where"
 * constraint clause
 */
typedef struct
{
	char		*ref;	/* table reference ("a" in "affiliation a") */
	char		*member;/* member name ("sta" in "a.sta") */
	int		sign;	/* 1 or -1 for the sign of the term */
	double		value;	/* numerical terms are converted to double */
	int		index;	/* index of the member the term refers to */
	QTable		*table; /* table containing the member */
	CssTable 	t;	/* an instance of the table during the search */
} QTerm;

typedef struct
{
	const char *operation;	/* '=', '<=', '>=', "like", "in", etc */
	QTerm	a;		/* the left side of a single constraint */
	int	nb;		/* the number of terms on the right side */
	QTerm	b[MAX_TABLES];		/* each term on the right side */
	char	*like[MAX_N_EXPR];	/* string(s) for the "like" operator */
	char	*in[MAX_N_EXPR + 1];	/* string(s) for the "in" operator */
	long	inl[MAX_N_EXPR];
	int	nl;
} QConstraint;

typedef struct
{
	char *qbuf;
	bool distinct;
	int num_ref, num_tables, num_constraints;
	QTerm refs[MAX_TABLES];
	QTable *tables[MAX_TABLES];
	QConstraint constraints[MAX_TABLES];
} QParseStruct;

typedef struct FFDB_Database *FFDatabase;
typedef struct FFDB_QueryTable_Struct *FFDB_QueryTableStruct;


/* ******* FFDBOpen.c ********/
FFDatabase FFDBOpen(const char *param_root, const char *seg_root,
		const char *directory_structure, double directory_duration);
FFDatabase FFDBOpenPrefix(const char *prefix);
void FFDBClose(FFDatabase ffdb);
int FFDBAuthors(FFDatabase ffdb, char ***authors);
int FFDBSetDefaultAuthor(FFDatabase ffdb, const char *author);
char *FFDBDefaultAuthor(FFDatabase ffdb);
int FFDBGetAuthorWritable(FFDatabase ffdb, const char *author);
int FFDBSetAuthorWritable(FFDatabase ffdb, const char *author, int mode);
double FFDBDirectoryDuration(FFDatabase ffdb);
void FFDBSetDirectoryDuration(FFDatabase ffdb, double directory_duration);
void FFDBSetMaxMemRecords(FFDatabase ffdb, int max_records);


/* ******* FFDBQueryTable.c ********/
int FFDBQueryTable(FFDatabase ffdb, const char *query, const char *tableName,
		Vector r);
int FFDBQueryTableInit(FFDatabase ffdb, const char *query,
		const char *cssTableName, FFDB_QueryTableStruct *qs);
int FFDBQueryTableResults(FFDB_QueryTableStruct qs, int numToFetch,
                int *numFetched, Vector records);
int FFDBQueryTableClose(FFDB_QueryTableStruct qs);
bool FFDBConvertDirToDate(const char *ds, const char *name, DateTime *dt);
int FFDBTimeLimits(FFDatabase ffdb, double *tmin, double *tmax);
int FFDBGetQueryTable(const char *query, char *author, int author_len,
		char *name, int name_len);
int FFDBQueryPrefix(FFDatabase ffdb, const char *query,
		const char *cssTableName, Vector table);


/* ******* FFDBUpdateTables.c ********/
int FFDBUpdateTables(FFDatabase ffdb, const char *tableName, int num,
		CssTable *old_tables, CssTable *new_tables);
int FFDBUpdateTable(FFDatabase ffdb, const char *tableName, CssTable old_table,
		CssTable new_table);
int FFDBUpdateTableWhere(FFDatabase ffdb, const char *tableName, int numMembers,
		int *memberIndices, int numWhere, int *whereIndices,
		CssTable old_table, CssTable new_table);
int FFDBUpdateTablesWhere(FFDatabase ffdb, const char *tableName,int numMembers,
		int *memberIndices, int numWhere, int *whereIndices,
		int num, CssTable *old_tables, CssTable *new_tables);
int FFDBGetWhereMembers(CssTable css, int *where);
int FFDBUpdateWhere(FFDatabase ffdb, int numMembers, int *memberIndices,
		CssTable t, const char *select_clause);
int FFDBUpdate(FFDatabase ffdb, CssTable t);
int FFDBDeleteTable(FFDatabase ffdb, CssTable t);

/* ******* FFDBInsertTables.c ********/
int FFDBInsertTable(FFDatabase ffdb, CssTable table, const char *author);
int FFDBInsertTables(FFDatabase ffdb, int num, CssTable *tables,
		const char *author);
int FFDBWriteTable(FFDatabase ffdb, CssTable table, const char *author,
		double time);
int FFDBWriteStaticTable(FFDatabase ffdb, CssTable table);
int FFDBCreateAuthor(FFDatabase ffdb, const char *author);
int FFDBCreateDir(const char *path);
int FFDBConvertDateToDir(const char *directory_structure, int param_dir,
                const char *authorOrStation, double time, char *dir);
int FFDBInsertPrefixTable(FFDatabase ffdb, CssTable table);

/* ******* FFDBStatic.c ********/
int FFDBReadStaticTable(FFDatabase ffdb, const char *cssTableName, Vector v,
		char **static_table_path);
void FFDBSetReadGlobalTables(FFDatabase ffdb, bool read_globals);
int FFDBSetStaticTable(FFDatabase ffdb, const char *cssTableName,
		const char *path);
char *FFDBGetStaticTable(FFDatabase ffdb, const char *cssTableName);
const char *FFDBGetNetwork(FFDatabase ffdb, const char *sta);
const char *FFDBGetLikeNet(FFDatabase ffdb, const char *sta);
void FFDBAddChannelAlternate(Vector v);

/* ******* FFDBImport.c ********/
int FFDBImport(FFDatabase ffdb, const char *path);

/* ******* FFDBError.c ********/
char *FFDBErrMsg(void);
int FFDBErrno(void);

/* ******* FFDBRequestIds.c ********/
int FFDBSetRequestIdIncrement(const char *keyname, int idIncrement);
int FFDBGetNextId(FFDatabase ffdb, const char *tableName, const char *keyname);
int FFDBRequestIds(FFDatabase ffdb, const char *tableName, const char *keyname,
		int numRequested, int consecutive, int *ids, int *numReturned);
int FFDBUpdateLastid(FFDatabase ffdb, const char *tableName,const char *keyname,
		int numRequested, int *keyvalue);
int FFDBRecycleIds(const char *keyname, int *ids, int numReturned);
int FFDBNumberIDsAvailable(const char *keyname);


#endif /* _LIB_FFDB_H_ */
