/* ODBCGetSchema.c
 *
 *   ODBCGetSchema()
 */
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>

#include "libgdb.h"
#include "libstring.h"

#ifdef HAVE_LIBODBC

/**
 * Get information about available tables. ODBCGetSchema() fills the
 * ODBC_Table structure with information about all available tables for
 * a given ODBC connection. The ODBC_Table structure is defined as:
 * <pre>
 *  typedef struct
 *  {
 *          SQLCHAR         catalog[ODBC_STRLEN];
 *          SQLCHAR         schema[ODBC_STRLEN];
 *          SQLCHAR         tableName[ODBC_STRLEN];
 *          SQLCHAR         columnName[ODBC_STRLEN];
 *          SQLSMALLINT     dataType;
 *          SQLCHAR         typeName[ODBC_STRLEN];
 *          SQLINTEGER      columnSize;
 *          SQLINTEGER      bufferLength;
 *          SQLSMALLINT     decimalDigits;
 *          SQLSMALLINT     numPrecRadix;
 *          SQLSMALLINT     nullable;
 *          SQLCHAR         remarks[ODBC_REMLEN];
 *          SQLCHAR         columnDefault[ODBC_STRLEN];
 *          SQLSMALLINT     sqlDataType;
 *          SQLSMALLINT     datetimeSubtypeCode;
 *          SQLINTEGER      charOctetLength;
 *          SQLINTEGER      ordinalPosition;
 *          SQLCHAR         isNullable[ODBC_STRLEN];
 *  } ODBC_Column;
 *  
 *  typedef struct
 *  {
 *          SQLCHAR         catalog[ODBC_STRLEN];
 *          SQLCHAR         schema[ODBC_STRLEN];
 *          SQLCHAR         tableName[ODBC_STRLEN];
 *          SQLCHAR         tableType[ODBC_STRLEN];
 *          SQLCHAR         remarks[ODBC_REMLEN];
 *  
 *          int             numColumns;
 *          ODBC_Column     *columns;
 *  } ODBC_Table;
 * </pre>
 *
 * <p>
 * The following code illustrates the use of ODBCGetSchema.
 * <pre>
 * #include "libgdb.h"
 * 
 * int
 * main(int argc,char **argv)
 * {
 *     int i, j, numTables, autoCommit=1;
 *     char *dataSource="ORACLE", *user="sel3", *passwd="sel3";
 *     ODBC_Table *tables;
 *     SQLRETURN ret;
 *     SQLHDBC hdbc;
 * 
 *     /\* Connect to the data source
 *      *\/
 *     if((hdbc = ODBCConnect(dataSource, user, passwd, autoCommit)) == NULL) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 * 
 *     ret = ODBCGetSchema(hdbc, &numTables, &tables);
 *     if(ret != 0) {
 *	   char msg[100];
 *         snprintf(msg, 100, "ODBCGetSchema failed: %s\n", ODBCErrMsg());
 *	   logErrorMsg(LOG_WARNING, msg);
 *         exit(1);
 *     }
 * 
 *     for(i = 0; i < numTables; i++)
 *     {
 *         printf("catalog:%s schema:%s tableName:%s tableType:%s remarks:%s\n",
 *            tables[i].catalog, tables[i].schema,
 *            tables[i].tableName, tables[i].tableType,
 *            tables[i].remarks);
 * 
 *         printf("number of columns: %d\n", tables[i].numColumns);
 * 
 *         if(tables[i].numColumns > 0)
 *         {
 *             printf(
 * "     name      type   size buflen decimal radix nullable   remark  default\n");
 *             for(j = 0; j < tables[i].numColumns; j++) {
 *                printf("%10s %10s %3d   %3d    %3d     %3d      %1d   %s   %s\n",
 *                    tables[i].columns[j].columnName,
 *                    tables[i].columns[j].typeName,
 *                    (int)tables[i].columns[j].columnSize,
 *                    (int)tables[i].columns[j].bufferLength,
 *                    tables[i].columns[j].decimalDigits,
 *                    tables[i].columns[j].numPrecRadix,
 *                    tables[i].columns[j].nullable,
 *                    tables[i].columns[j].remarks,
 *                    tables[i].columns[j].columnDefault);
 *             }
 *         }
 *         printf("\n");
 *     }
 * 
 *     ODBCDisconnect(hdbc);
 * 
 *     for(i = 0; i < numTables; i++) free(tables[i].columns);
 *     free(tables);
 * }
 * </pre>
 */

#define ERROR_CHECK \
    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) { \
	ODBCSetErrorMsg("ODBCGetSchema", SQL_HANDLE_STMT, hstmt); \
	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) { \
	    logErrorMsg(LOG_WARNING, "ODBCGetSchema: SQLFreeHandle failed."); \
	} \
	return -1; \
    }

/**
 * Get information about tables. This routine returns a list of tables found
 * for the input connection handle. A structure of schema information is
 * returned for each table.
 * @param hdbc A connection handle obtained from ODBCConnect().
 * @param schemaName The name of the schema. If NULL, tables from all 
 *		     schemas will be returned. 
 * @param numTables The number of tables available.
 * @param tables An array of numTables ODBC_Table structures.
 * @return 0 for success, -1 for failure. Retrieve the error number and \
 *		error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCGetSchema(SQLHDBC hdbc, unsigned char *schemaName, 
	      int *numTables, ODBC_Table **tables)
{
	ODBC_Table	*t;
	int		i, num;
	SQLRETURN	ret;
	SQLHSTMT	hstmt;
        unsigned char	allSchemas[] = "%";  /* ET: default to 
						return all tbls. */
        	
	/* Declare buffers for result set data */
	SQLCHAR	szCatalog[ODBC_STRLEN], szSchema[ODBC_STRLEN],
		szTableName[ODBC_STRLEN], szTableType[ODBC_STRLEN],
		szRemarks[ODBC_REMLEN];

	/* Declare buffers for bytes available to return */
	SQLLEN cbCatalog, cbSchema, cbTableName, cbTableType, cbRemarks;

	/* Set value of schema name */
	if (schemaName == NULL) 
	{
	   schemaName = allSchemas;
	}

	ret = ODBCAllocStmt(hdbc, &hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCGetSchema", SQL_HANDLE_DBC, hdbc);
	    return -1;
	}

	ret = SQLTables(hstmt,
			NULL, 0,	/* All catalogs */
			schemaName, SQL_NTS, /* All schemas */
			NULL, 0,	/* All tables */
			NULL, 0);	/* All table types */
	ERROR_CHECK;

/* could use SQLGetInfo with SQL_MAX_CATALOG_NAME_LEN,
 * SQL_MAX_SCHEMA_NAME_LEN, and SQL_MAX_TABLE_NAME_LEN to make sure there is
 * enough space
 */
	t = (ODBC_Table *)malloc(sizeof(ODBC_Table));
	if(t == NULL) {
	    ODBCSetErrorMsg("ODBCGetSchema malloc error", 0, 0);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,"ODBCGetSchema: SQLFreeHandle failed.");
	    }
	    return -1;
	}

	SQLBindCol(hstmt, 1, SQL_C_CHAR, szCatalog, sizeof(szCatalog),
			&cbCatalog);
	ERROR_CHECK;
	SQLBindCol(hstmt, 2, SQL_C_CHAR, szSchema, sizeof(szSchema),
			&cbSchema);
	ERROR_CHECK;
	SQLBindCol(hstmt, 3, SQL_C_CHAR, szTableName, sizeof(szTableName),
			&cbTableName);
	ERROR_CHECK;
	SQLBindCol(hstmt, 4, SQL_C_CHAR, szTableType, sizeof(szTableType),
			&cbTableType);
	ERROR_CHECK;
	SQLBindCol(hstmt, 5, SQL_C_CHAR, szRemarks, sizeof(szRemarks),
			&cbRemarks);
	ERROR_CHECK;

	szCatalog[0] = '\0';
	szSchema[0] = '\0';
	szTableName[0] = '\0';
	szTableType[0] = '\0';
	szRemarks[0] = '\0';

	*numTables = 0;
	num = 0;
	/* GET RESULTS */
	while((ret = SQLFetch(hstmt)) == SQL_SUCCESS
		|| ret == SQL_SUCCESS_WITH_INFO)
	{
	    if(cbTableName == SQL_NULL_DATA) {
		stringcpy((char *) szTableName, "Unknown", ODBC_STRLEN);
	    }
	    t = (ODBC_Table *)realloc(t, (num+1)*sizeof(ODBC_Table));
	    if(t == NULL) {
		ODBCSetErrorMsg("ODBCGetSchema malloc error", 0, 0);
		if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		    logErrorMsg(LOG_WARNING,
			"ODBCGetSchema: SQLFreeHandle failed.");
		}
		return -1;
	    }

	    stringcpy((char *) t[num].catalog, (const char *) szCatalog,
			ODBC_STRLEN);
	    stringcpy((char *) t[num].schema, (const char *) szSchema,
			ODBC_STRLEN);
	    stringcpy((char *) t[num].tableName, (const char *) szTableName,
			ODBC_STRLEN);
	    stringcpy((char *) t[num].tableType, (const char *) szTableType,
			ODBC_STRLEN);
	    stringcpy((char *) t[num].remarks, (const char *) szRemarks,
			ODBC_REMLEN);
	    t[num].numColumns = 0;
	    t[num].columns = NULL;
	    num++;

	    szCatalog[0] = '\0';
	    szSchema[0] = '\0';
	    szTableName[0] = '\0';
	    szTableType[0] = '\0';
	    szRemarks[0] = '\0';
	}
	*tables = t;

	if(ret != SQL_NO_DATA) {
	    ODBCSetErrorMsg("ODBCGetSchema", SQL_HANDLE_STMT, hstmt);
	}

	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	    logErrorMsg(LOG_WARNING, "ODBCGetSchema: SQLFreeHandle failed.");
	}

	for(i = 0; i < num; i++)
	{
	   if(ODBCGetTableCols(hdbc, (char *)t[i].catalog, 
			       (char *)t[i].schema, (char *) t[i].tableName, 
			       &t[i].numColumns, &t[i].columns) < 0)
	    {
		*numTables = i;
		return -1;
	    }
	}

	*numTables = num;
	return 0;
}
#endif /* HAVE_LIBODBC */
