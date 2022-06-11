#ifndef _LIB_GDB_H_
#define _LIB_GDB_H_

#include <errno.h>
#include <string>
using namespace std;

#include "gobject++/CssTables.h"
#include "logErrorMsg.h"

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
	SQLULEN  	columnSize;
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
	SQLULEN 	columnSize;
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
	SQLLEN	indicator;
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
	CssClassDescription	*des;
	BindColumn	*bindCol;
} ODBC_QueryTableStruct;


SQLHENV ODBCOpen(void);
int	ODBCDataSources(int *num, char ***dataSource);
SQLHDBC ODBCConnect(const string &dataSource, const string &user,
			const string &passwd, int autoCommit);
SQLHDBC ODBCConnect2(const string &dataSource, const string &user,
			const string &passwd, int autoCommit, int loginTimeout);
void	ODBCDisconnect(SQLHDBC hdbc);
void	ODBCCommit(SQLHDBC hdbc);
void	ODBCCloseDatabase(void);
int	ODBCGetSchema(SQLHDBC hdbc, unsigned char *schemaName, int *numTables,
			ODBC_Table **tables);
int	ODBCGetTableCols(SQLHDBC hdbc, const string &catalogName, 
			const string &schemaName, const string &tableName, 
			int *numColumns, ODBC_Column **columns);
int	ODBCGeneralQuery(SQLHDBC hdbc, const string &query,ODBC_Result *result);
int	ODBCDescribeCols(SQLHSTMT hstmt, int *numColumns,
			ODBC_ColumnResult **columns);
int	ODBCGetResultCols(SQLHSTMT hstmt, int numColumns,
			ODBC_ColumnResult *column);
char *	ODBCGetErrorMsg(SQLSMALLINT handleType, SQLHANDLE handle, char *msg);
char *	ODBCSetErrorMsg(const char *header, SQLSMALLINT handleType,
			SQLHANDLE handle);
char *	ODBCErrMsg(void);
int	ODBCErrno(void);
int	ODBCQueryTable(SQLHDBC hdbc, const string &query,
			const string &tableName, gvector<CssTableClass *> &table);
int	ODBCQueryTableInit(SQLHDBC hdbc, const string &query,
			const string &tableName, ODBC_QueryTableStruct **qs);
int	ODBCGetStruct(SQLHDBC hdbc, const string &query, int *numRows, void **t,
			const string &tableName);
int	ODBCQueryTableResults(ODBC_QueryTableStruct *qs, int numToFetch,
			int *numFetched, gvector<CssTableClass *> &table);
int	ODBCQueryTableClose(ODBC_QueryTableStruct *qs);
const char * ODBCGetDataSource(SQLHDBC this_hdbc);
const char * ODBCGetUser(SQLHDBC this_hdbc);
const char * ODBCGetPassword(SQLHDBC this_hdbc);
int	ODBCGetAutoCommit(SQLHDBC hdbc);
int	ODBCPutTable(SQLHDBC hdbc, const string &account,
		const string &tableName, CssTableClass *t);
int	ODBCInsertTable(SQLHDBC hdbc, const string &tableName, CssTableClass *table);
int	ODBCInsertTables(SQLHDBC hdbc, const string &tableName, int num,
			CssTableClass **tables);
int	ODBCUpdateTable(SQLHDBC hdbc, const string &tableName,
			CssTableClass *old_table, CssTableClass *new_table);
int	ODBCUpdateTables(SQLHDBC hdbc, const string &tableName, int num,
			CssTableClass **old_tables, CssTableClass **new_tables);
int	ODBCUpdateTableWhere(SQLHDBC hdbc, const string &tableName,
			int numMembers, int *memberIndices, int numWhere,
			int *whereIndices, CssTableClass *old_table,
			CssTableClass *new_table);
int	ODBCUpdateTablesWhere(SQLHDBC hdbc, const string &tableName,
			int numMembers, int *memberIndices, int numWhere,
			int *whereIndices, int num, CssTableClass **old_tables,
			CssTableClass **new_tables);
int	ODBCCountTable(SQLHDBC hdbc, const string &tableName, int numWhere,
			int *whereIndices, CssTableClass *table, int *count);
int	ODBCCountTables(SQLHDBC hdbc, const string &tableName, int numWhere,
			int *whereIndices, int num, CssTableClass **tables,
			int *count);
int	ODBCDeleteTable(SQLHDBC hdbc, const string &tableName, CssTableClass *table);
int	ODBCDeleteTables(SQLHDBC hdbc, const string &tableName, int num,
			CssTableClass **tables);
int	ODBCDeleteTableWhere(SQLHDBC hdbc, const string &tableName,int numWhere,
			int *whereIndices, CssTableClass *table);
int	ODBCDeleteTablesWhere(SQLHDBC hdbc, const string &tableName,
			int numWhere, int *whereIndices, int num,
			CssTableClass **tables);
int 	ODBCGetWhereMembers(CssTableClass *css, int *where);
SQLHDBC	ODBCGetLastConnection(void);
int	ODBCQuery(SQLHDBC hdbc, const string &query,gvector<CssTableClass *> &table);
void	ODBCFreeResult(ODBC_Result *result);
void	ODBCSetStmtTimeout(int timeout);
SQLRETURN ODBCAllocStmt(SQLHDBC hdbc, SQLHSTMT *hstmt);

int	ODBCRequestIds(SQLHDBC hdbc, const string &tableName,
			const string &keyname, int numRequested,
			int consecutive, int *ids,
			int *numReturned);
int	ODBCUpdateLastid(SQLHDBC hdbc, const string &tableName,
			const string &keyname, int numRequested, int *nextid);
int	ODBCSetRequestIdIncrement(const string &keyname, int increment);
int	ODBCGetNextId(SQLHDBC hdbc, const string &tableName,
			const string &keyname);
int	ODBCRecycleIds(const string &keyname, int *ids, int numReturned);
int	ODBCNumberIDsAvailable(const string &keyname);

long	ODBCGetChanid(SQLHDBC hdbc, const string &sitechanTable,
			const string &sta, const string &chan);
int	ODBCGetTableName(const string &query, string &author, string &name);

#endif /* HAVE_LIBODBC */

#endif /* _LIB_GODBC_H_ */
