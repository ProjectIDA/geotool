#ifndef _LIB_GODBC_H_
#define _LIB_GODBC_H_

#include <errno.h>

#ifndef TINY_GODBC
#include "gobject/Vector.h"
#include "gobject/CssTable.h"
#include "gobject/cssObjects.h"
#include "logErrorMsg.h"
#endif  /* TINY_GODBC */

#ifdef HAVE_LIBODBC

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#define ODBC_STRLEN 128+1
#define ODBC_REMLEN 254+1

/* ROW_ARRAY_SIZE can be > 1 for EasySoft Oracle Driver, but must be equal to 1
 * for the free oracle odbc driver (as of 27-04-04).
 */
#ifdef ODBC_USE_ARRAYS
#define ROW_ARRAY_SIZE 10
#else
#define ROW_ARRAY_SIZE 1
#endif

typedef struct
{
	SQLCHAR		catalog[ODBC_STRLEN];
	SQLCHAR		schema[ODBC_STRLEN];
	SQLCHAR		tableName[ODBC_STRLEN];
	SQLCHAR		columnName[ODBC_STRLEN];
	SQLSMALLINT	dataType;
	SQLCHAR		typeName[ODBC_STRLEN];
	SQLULEN		columnSize;
	SQLINTEGER	bufferLength;
	SQLSMALLINT	decimalDigits;
	SQLSMALLINT	numPrecRadix;
	SQLSMALLINT	nullable;
	SQLCHAR		remarks[ODBC_REMLEN];
	SQLCHAR		columnDefault[ODBC_STRLEN];
	SQLSMALLINT	sqlDataType;
	SQLSMALLINT	datetimeSubtypeCode;
	SQLINTEGER	charOctetLength;
	SQLINTEGER	ordinalPosition;
	SQLCHAR		isNullable[ODBC_STRLEN];
} ODBC_Column;

typedef struct
{
	SQLCHAR		catalog[ODBC_STRLEN];
	SQLCHAR		schema[ODBC_STRLEN];
	SQLCHAR		tableName[ODBC_STRLEN];
	SQLCHAR		tableType[ODBC_STRLEN];
	SQLCHAR		remarks[ODBC_REMLEN];

	int		numColumns;
	ODBC_Column	*columns;
} ODBC_Table;

typedef struct
{
	SQLCHAR		columnName[ODBC_STRLEN];
	SQLSMALLINT	nameLength;
	SQLSMALLINT	dataType;
	SQLULEN		columnSize;
	SQLSMALLINT	decimalDigits;
	SQLSMALLINT	nullable;
	SQLPOINTER	value;
	union {
	    char		sql_String[ODBC_STRLEN];
	    SQLDOUBLE		sql_Double;
	    SQLINTEGER		sql_Long;
	    SQLSMALLINT		sql_Int;
	    SQL_TIMESTAMP_STRUCT sql_DateTime;
	} bindTarget;
	SQLLEN		indicator;
	SQLINTEGER	numRows;
} ODBC_ColumnResult;

typedef struct
{
	int			numColumns;
	int			numRows;
	ODBC_ColumnResult	*column;
} ODBC_Result;

typedef struct
{
	int		index;
	SQLSMALLINT	dataType;
	SQLLEN		indicator[ROW_ARRAY_SIZE];
	union {
	    char		sql_String[ROW_ARRAY_SIZE][ODBC_STRLEN];
	    SQLDOUBLE		sql_Double[ROW_ARRAY_SIZE];
	    SQLINTEGER		sql_Long[ROW_ARRAY_SIZE];
	    SQLSMALLINT		sql_Int[ROW_ARRAY_SIZE];
	    SQL_DATE_STRUCT	sql_Date[ROW_ARRAY_SIZE];
	} bindTarget;
} BindColumn;

#ifndef TINY_GODBC
typedef struct
{
	char		*cssTableName;
	int		tableName;
	int		account;
	int		odbc_numColumns;
	int		k1;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;
	SQLUINTEGER	numRowsFetched;
	SQLUSMALLINT	*rowStatusArray;
	CssDescription	*des;
	BindColumn	*bindCol;
} ODBC_QueryTableStruct;
#endif /* TINY_GODBC */


SQLHENV ODBCOpen(void);
int	ODBCDataSources(int *num, char ***dataSource);
SQLHDBC ODBCConnect(const char *dataSource, const char *user,
			const char *passwd, int autoCommit);
SQLHDBC ODBCConnect2(const char *dataSource, const char *user,
			const char *passwd, int autoCommit, int loginTimeout);
SQLHDBC ODBCConnect3(const char *dataSource, const char *user,
		     const char *passwd, int autoCommit, int loginTimeout,
                     int forceNew);
void	ODBCDisconnect(SQLHDBC hdbc);
void	ODBCCloseDatabase(void);
int	ODBCGetSchema(SQLHDBC hdbc, unsigned char *schemaName, int *numTables,
			ODBC_Table **tables);
int	ODBCGetTableCols(SQLHDBC hdbc, const char *catalogName,
			const char *schemaName, const char *tableName,
			int *numColumns, ODBC_Column **columns);
int	ODBCGeneralQuery(SQLHDBC hdbc, const char *query,
                         /*@out@*/ODBC_Result *result);
int	ODBCDescribeCols(SQLHSTMT hstmt, int *numColumns,
			ODBC_ColumnResult **columns);
int	ODBCGetResultCols(SQLHSTMT hstmt, int numColumns,
			ODBC_ColumnResult *column);
char *	ODBCGetErrorMsg(SQLSMALLINT handleType, SQLHANDLE handle, char *msg);
char *	ODBCSetErrorMsg(const char *header, SQLSMALLINT handleType,
			SQLHANDLE handle);
/*@shared@*/char *	ODBCErrMsg(void);
int	ODBCErrno(void);
#ifndef TINY_GODBC
int	ODBCQueryTable(SQLHDBC hdbc, const char *query, const char *tableName,
			Vector table);
int	ODBCQueryTableInit(SQLHDBC hdbc, const char *query,
			const char *tableName, ODBC_QueryTableStruct **qs);
int	ODBCGetStruct(SQLHDBC hdbc, const char *query, int *numRows, void **t,
			const char *tableName);
int	ODBCQueryTableResults(ODBC_QueryTableStruct *qs, int numToFetch,
			int *numFetched, Vector table);
int	ODBCQueryTableClose(ODBC_QueryTableStruct *qs);
#endif /* TINY_GODBC */
char *	ODBCGetDataSource(SQLHDBC this_hdbc);
char *	ODBCGetUser(SQLHDBC this_hdbc);
char *	ODBCGetPassword(SQLHDBC this_hdbc);
int	ODBCGetAutoCommit(SQLHDBC hdbc);
#ifndef TINY_GODBC
int	ODBCPutTable(SQLHDBC hdbc, const char *account, const char *tableName,
			CssTable t);
int	ODBCInsertTable(SQLHDBC hdbc, const char *tableName, CssTable table);
int	ODBCInsertTables(SQLHDBC hdbc, const char *tableName, int num,
			CssTable *tables);
int	ODBCUpdateTable(SQLHDBC hdbc, const char *tableName, CssTable old_table,
			CssTable new_table);
int	ODBCUpdateTables(SQLHDBC hdbc, const char *tableName, int num,
			CssTable *old_tables, CssTable *new_tables);
int	ODBCUpdateTableWhere(SQLHDBC hdbc, const char *tableName,
			int numMembers, int *memberIndices, int numWhere,
			int *whereIndices, CssTable old_table,
			CssTable new_table);
int	ODBCUpdateTablesWhere(SQLHDBC hdbc, const char *tableName,
			int numMembers, int *memberIndices, int numWhere,
			int *whereIndices, int num, CssTable *old_tables,
			CssTable *new_tables);
int	ODBCCountTable(SQLHDBC hdbc, const char *tableName, int numWhere,
			int *whereIndices, CssTable table, int *count);
int	ODBCCountTables(SQLHDBC hdbc, const char *tableName, int numWhere,
			int *whereIndices, int num, CssTable *tables,
			int *count);
int	ODBCDeleteTable(SQLHDBC hdbc, const char *tableName, CssTable table);
int	ODBCDeleteTables(SQLHDBC hdbc, const char *tableName, int num,
			CssTable *tables);
int	ODBCDeleteTableWhere(SQLHDBC hdbc, const char *tableName, int numWhere,
			int *whereIndices, CssTable table);
int	ODBCDeleteTablesWhere(SQLHDBC hdbc, const char *tableName, int numWhere,
			int *whereIndices, int num, CssTable *tables);
int 	ODBCGetWhereMembers(CssTable css, int *where);
int	ODBCQuery(SQLHDBC hdbc, const char *query, Vector table);
#endif /* TINY_GODBC */
SQLHDBC	ODBCGetLastConnection(void);
void	ODBCFreeResult(ODBC_Result *result);
void	ODBCSetStmtTimeout(int timeout);
SQLRETURN ODBCAllocStmt(SQLHDBC hdbc, /*@out@*/SQLHSTMT *hstmt);

int	ODBCRequestIds(SQLHDBC hdbc, const char *tableName, const char *keyname,
			int numRequested, int consecutive, int *ids,
			int *numReturned);
int	ODBCUpdateLastid(SQLHDBC hdbc, const char *tableName,
			const char *keyname, int numRequested, int *nextid);
int	ODBCSetRequestIdIncrement(const char *keyname, int increment);
int	ODBCGetNextId(SQLHDBC hdbc, const char *tableName, const char *keyname);
int	ODBCRecycleIds(const char *keyname, int *ids, int numReturned);
int	ODBCNumberIDsAvailable(const char *keyname);

long	ODBCGetChanid(SQLHDBC hdbc, const char *sitechanTable, const char *sta,
			const char *chan);


#endif /* HAVE_LIBODBC */

#endif /* _LIB_GODBC_H_ */
