/* ODBCGetTableCols.c

    ODBCGetTableCols()
*/
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "libgdb.h"
#include "libstring.h"

#ifdef HAVE_LIBODBC

#define ERROR_CHECK \
    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) { \
	ODBCSetErrorMsg("ODBCGetTableCols", SQL_HANDLE_STMT, hstmt); \
	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) { \
	    logErrorMsg(LOG_WARNING,"ODBCGetTableCols: SQLFreeHandle failed.");\
	} \
	return -1; \
    }
/**
 * ODBCGetTableCols is used by ODBCGetSchema to retrieve information
 * about tables that are available to the current ODBC connection.
 * Information about tables that are synonyms may not be retrievable.
 * The ODBC_Column structure is defined as:
 * <pre>
 *  typedef struct
 *  {
 *      SQLCHAR         catalog[ODBC_STRLEN];
 *      SQLCHAR         schema[ODBC_STRLEN];
 *      SQLCHAR         tableName[ODBC_STRLEN];
 *      SQLCHAR         columnName[ODBC_STRLEN];
 *      SQLSMALLINT     dataType;
 *      SQLCHAR         typeName[ODBC_STRLEN];
 *      SQLINTEGER      columnSize;
 *      SQLINTEGER      bufferLength;
 *      SQLSMALLINT     decimalDigits;
 *      SQLSMALLINT     numPrecRadix;
 *      SQLSMALLINT     nullable;
 *      SQLCHAR         remarks[ODBC_REMLEN];
 *      SQLCHAR         columnDefault[ODBC_STRLEN];
 *      SQLSMALLINT     sqlDataType;
 *      SQLSMALLINT     datetimeSubtypeCode;
 *      SQLINTEGER      charOctetLength;
 *      SQLINTEGER      ordinalPosition;
 *      SQLCHAR         isNullable[ODBC_STRLEN];
 *  } ODBC_Column;
 * </pre>
 */
/**
 * Get information about the columns of a table. 
 * @param hdbc A connection handle obtained from ODBCConnect().
 * @param catalogName The name of the catalog containing this table.
 * @param schemaName The name of the schema containing this table. 
 *                   If NULL the current username is used.  
 * @param tableName The name of the database table.
 * @param numColumns The number of columns in the table (returned).
 * @param columns An array of numColumns ODBC_Column structures that \
 *		describe the columns.
 * @return  0 for success, -1 for an error flag. Retrieve the \
 * error number and error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCGetTableCols(SQLHDBC hdbc, const string &catalogName, 
		 const string &schemaName, const string &tableName, 
		 int *numColumns, ODBC_Column **columns)
{
	int		num, i;
	ODBC_Column	*c;
	SQLRETURN	ret;
	SQLHSTMT	hstmt;
        string		qSchema;

	/* Declare buffers for result set data */
	SQLCHAR	szCatalog[ODBC_STRLEN], szSchema[ODBC_STRLEN];
	SQLCHAR	szTableName[ODBC_STRLEN], szColumnName[ODBC_STRLEN];
	SQLCHAR	szTypeName[ODBC_STRLEN], szRemarks[ODBC_REMLEN];
	SQLCHAR	szColumnDefault[ODBC_STRLEN], szIsNullable[ODBC_STRLEN];
	SQLINTEGER ColumnSize, BufferLength, CharOctetLength, OrdinalPosition;
	SQLSMALLINT DataType, DecimalDigits, NumPrecRadix, Nullable;
	SQLSMALLINT SQLDataType, DatetimeSubtypeCode;

	/* Declare buffers for bytes available to return */
	SQLLEN cbCatalog, cbSchema, cbTableName, cbColumnName;
	SQLLEN cbDataType, cbTypeName, cbColumnSize, cbBufferLength;
	SQLLEN cbDecimalDigits, cbNumPrecRadix, cbNullable, cbRemarks;
	SQLLEN cbColumnDefault, cbSQLDataType, cbDatetimeSubtypeCode;
	SQLLEN cbCharOctetLength;
	SQLLEN cbOrdinalPosition, cbIsNullable;

	ret = ODBCAllocStmt(hdbc, &hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCGetTableCols", SQL_HANDLE_DBC, hdbc);
	    return -1;
	}

	/* Bind columns in result set to buffers
	 */

	szCatalog[0] = '\0';
	szSchema[0] = '\0';
	szTableName[0] = '\0';
	szColumnName[0] = '\0';
	szTypeName[0] = '\0';
	szRemarks[0] = '\0';
	szColumnDefault[0] = '\0';
	szIsNullable[0] = '\0';


        /* Prepare the schema name */
	if( !schemaName.empty() ) {
	   qSchema = schemaName;
	}
	else {
	   qSchema.assign(ODBCGetUser(hdbc));
	} 
        /* Convert schema name to upper case because some versions
         * of the free ODBC Oracle driver are case sensitive on the
         * schema name.
         */
       for (i = 0; i < (signed int)qSchema.length(); i++) {
	  qSchema[i] = (char)toupper(qSchema[i]);
       }
       	
       ret = SQLColumns(hstmt,
		(SQLCHAR *) catalogName.c_str(), SQL_NTS, /* catalog name */
		(SQLCHAR *) qSchema.c_str(), SQL_NTS,	  /* schema name */
		(SQLCHAR *) tableName.c_str(), SQL_NTS,	  /* table name */
		NULL, 0);			  /* All Columns */
       ERROR_CHECK;
       
       *numColumns = 0;
       c = NULL;
       
       if(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	  {
	    c = (ODBC_Column *)malloc(sizeof(ODBC_Column));
	    if(c == NULL) {
		ODBCSetErrorMsg("ODBCGetTableCols malloc error", 0, 0);
		if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		    logErrorMsg(LOG_WARNING,
			"ODBCGetTableCols: SQLFreeHandle failed.");
		}
		return -1;
	    }


            /* Bind columns in result set to buffers */

	    ret = SQLBindCol(hstmt, 1, SQL_C_CHAR,szCatalog,
			     ODBC_STRLEN,&cbCatalog);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 2, SQL_C_CHAR, szSchema,
			     ODBC_STRLEN, &cbSchema);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 3, SQL_C_CHAR, szTableName, 
			     ODBC_STRLEN, &cbTableName);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 4, SQL_C_CHAR, szColumnName, ODBC_STRLEN,
			     &cbColumnName);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 5, SQL_C_SSHORT, (SQLPOINTER)&DataType, 
			     sizeof(DataType), &cbDataType);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt,6,SQL_C_CHAR,szTypeName,
			     ODBC_STRLEN,&cbTypeName);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 7, SQL_C_SLONG, (SQLPOINTER) &ColumnSize, 
			     sizeof(ColumnSize), &cbColumnSize);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 8, SQL_C_SLONG, (SQLPOINTER)&BufferLength,
			     sizeof(BufferLength),&cbBufferLength);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 9, SQL_C_SSHORT, (SQLPOINTER)&DecimalDigits, 
			     sizeof(DecimalDigits), &cbDecimalDigits);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 10, SQL_C_SSHORT, (SQLPOINTER)&NumPrecRadix, 
			     sizeof(NumPrecRadix), &cbNumPrecRadix);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 11, SQL_C_SSHORT, (SQLPOINTER) &Nullable, 
			     sizeof(Nullable), &cbNullable);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 12, SQL_C_CHAR, (SQLPOINTER) szRemarks, 
			     ODBC_REMLEN, &cbRemarks);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 13, SQL_C_CHAR, (SQLPOINTER) szColumnDefault, 
			     ODBC_STRLEN, &cbColumnDefault);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 14, SQL_C_SSHORT, (SQLPOINTER)&SQLDataType, 
			     sizeof(SQLDataType), &cbSQLDataType);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 15, SQL_C_SSHORT, (SQLPOINTER) &DatetimeSubtypeCode, 
			     sizeof(DatetimeSubtypeCode), &cbDatetimeSubtypeCode);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 16, SQL_C_SLONG, (SQLPOINTER) &CharOctetLength, 
			     sizeof(CharOctetLength), &cbCharOctetLength);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 17, SQL_C_SLONG, (SQLPOINTER) &OrdinalPosition, 
			     sizeof(OrdinalPosition), &cbOrdinalPosition);
	    ERROR_CHECK;
	    ret = SQLBindCol(hstmt, 18, SQL_C_CHAR, (SQLPOINTER) szIsNullable, 
			     ODBC_STRLEN, &cbIsNullable);
	    ERROR_CHECK;
 
	    num = 0;

	    while((ret = SQLFetch(hstmt)) == SQL_SUCCESS
			|| ret == SQL_SUCCESS_WITH_INFO)
	    {
		c = (ODBC_Column *)realloc(c, (num+1)*sizeof(ODBC_Column));
		if(c == NULL) {
		    ODBCSetErrorMsg("ODBCGetTableCols malloc error", 0, 0);
		    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
			logErrorMsg(LOG_WARNING,
			    "ODBCGetTableCols: SQLFreeHandle failed.");
		    }
		    return -1;
		}
		stringcpy((char *)c[num].catalog, (const char *)szCatalog,
			ODBC_STRLEN);
		stringcpy((char *)c[num].schema, (const char *)szSchema,
			ODBC_STRLEN);
		stringcpy((char *)c[num].tableName, (const char *)szTableName,
			ODBC_STRLEN);
		stringcpy((char *)c[num].columnName,(const char *)szColumnName,
			ODBC_STRLEN);
		c[num].dataType = DataType;
		stringcpy((char *)c[num].typeName, (const char *)szTypeName,
			ODBC_STRLEN);
		c[num].columnSize =  ColumnSize;
		c[num].bufferLength = BufferLength;
		c[num].decimalDigits = DecimalDigits;
		c[num].numPrecRadix = NumPrecRadix;
		c[num].nullable = Nullable;
		stringcpy((char *)c[num].remarks, (const char *)szRemarks,
			ODBC_REMLEN);
		stringcpy((char *)c[num].columnDefault,
			(const char *)szColumnDefault, ODBC_STRLEN);
		c[num].sqlDataType = SQLDataType;
		c[num].datetimeSubtypeCode = DatetimeSubtypeCode;
		c[num].charOctetLength = CharOctetLength;
		c[num].ordinalPosition = OrdinalPosition;
		stringcpy((char *)c[num].isNullable,
			(const char *)szIsNullable, ODBC_STRLEN);
		num++;

		szCatalog[0] = '\0';
		szSchema[0] = '\0';
		szTableName[0] = '\0';
		szColumnName[0] = '\0';
		szTypeName[0] = '\0';
		szRemarks[0] = '\0';
		szColumnDefault[0] = '\0';
		szIsNullable[0] = '\0';
	    }
	}

	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	    logErrorMsg(LOG_WARNING, "ODBCGetTableCols: SQLFreeHandle failed.");
	}

	*columns = c;
	*numColumns = num;
	return 0;
}
#endif /* HAVE_LIBODBC */
