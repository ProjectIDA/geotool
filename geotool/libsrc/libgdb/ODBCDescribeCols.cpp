/* ODBCDescribeCols.c

    ODBCDescribeCols()
*/
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include "libgdb.h"

#ifdef HAVE_LIBODBC

/**
 *  This routine is used to obtain information about the columns that are
 *  returned by an arbitrary SQL query. The information is placed
 *  in the ODBC_ColumnResult strcuture, which is defined as:
 *  <pre>
 *  typedef struct
 *  {
 *      SQLCHAR         columnName[ODBC_STRLEN];
 *      SQLSMALLINT     nameLength;
 *      SQLSMALLINT     dataType;
 *      SQLUINTEGER     columnSize;
 *      SQLSMALLINT     decimalDigits;
 *      SQLSMALLINT     nullable;
 *      SQLPOINTER      value;
 *      union {
 *          char                 sql_String[ODBC_STRLEN];
 *          SQLDOUBLE            sql_Double;
 *          SQLINTEGER           sql_Long;
 *          SQLSMALLINT          sql_Int;
 *          SQL_TIMESTAMP_STRUCT sql_DateTime;
 *      } bindTarget;
 *      SQLINTEGER      indicator;
 *      SQLINTEGER      numRows;
 *  } ODBC_ColumnResult;
 * </pre>
 * <p>
 * This routine is used together with ODBCGetResultCols to retrieve the
 * results of an arbitrary SQL query, where the names of columns and the
 * data types of columns are not known beforehand. The following code
 * shows the routine ODBCGeneralQuery, which uses ODBC_ColumnResult and
 * ODBCGetResultCols to retrieve the data from an arbitrary query.
 *
 * <pre>
 *  int
 *  ODBCGeneralQuery(SQLHDBC hdbc, char *query, ODBC_Result *result)
 *  {
 *      int i;
 *      SQLHSTMT hstmt;
 *      SQLRETURN ret;
 *  
 *      result->column = NULL;
 *  
 *      ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
 *      if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
 *      {
 *          ODBCSetErrorMsg("ODBCGeneralQuery", SQL_HANDLE_DBC, hdbc);
 *          return -1;
 *      }
 * 
 *      /\* Execute the query *\/
 *      ret = SQLExecDirect(hstmt, query, SQL_NTS);
 *      if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
 *      {
 *          ODBCSetErrorMsg("ODBCGeneralQuery", SQL_HANDLE_STMT, hstmt);
 *          if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
 *            logErrorMsg(LOG_WARNING,"ODBCGeneralQuery:SQLFreeHandle failed.");
 *          }
 *          return -1;
 *      }
 *  
 *      /\* Get the column descriptions of the rows that are returned. *\/
 *      if(ODBCDescribeCols(hstmt, &result->numColumns, &result->column) < 0)
 *      {
 *          return -1;
 *      }
 * 
 *      /\* Using the column data types, copy the results into the appropriate
 *       * variables.
 *       *\/
 *      ret = ODBCGetResultCols(hstmt, result->numColumns, result->column);
 *  
 *      /\* Check for consistent results *\/
 *      if(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
 *      {
 *          ret = 0;
 *          if(result->numColumns > 0) {
 *              result->numRows = result->column[0].numRows;
 *          }
 *          for(i = 1; i < result->numColumns &&
 *              result->column[i].numRows == result->column[0].numRows; i++);
 *          if(i < result->numColumns) {
 *              ODBCSetErrorMsg("ODBCGeneralQuery: column lengths vary.",0,0);
 *              ret = -1;
 *          }
 *      }
 *  
 *      if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
 *           logErrorMsg(LOG_WARNING,"ODBCGeneralQuery: SQLFreeHandle failed.");
 *      }
 *  
 *      return ret;
 *  }
 * </pre>
 */


/** Get information about the columns returned from a query.
 *  Fill an ODBC_Column structure for each column.
 * @param hstmt A statement handle obtained from SQLAllocHandle.
 * @param numColumns The number of columns in the result.
 * @param columns An array of numColumns ODBC_ColumnResult structures.
 * Returns zero for success. Returns -1 for an error flag. Retrieve the
 * error number and error message with ODBCErrno() and ODBCErrMsg().
 * @see ODBCGetResultCols
 * @see ODBCGeneralQuery
 */
int
ODBCDescribeCols(SQLHSTMT hstmt, int *numColumns, ODBC_ColumnResult **columns)
{
	int i;
	SQLRETURN ret;
	SQLSMALLINT num;
	ODBC_ColumnResult *col;

	ret = SQLNumResultCols(hstmt, &num);
	*numColumns = (int)num;

	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCDescribeResultCols", SQL_HANDLE_STMT, hstmt);
	    return -1;
	}

	col = (ODBC_ColumnResult *)malloc(
			*numColumns*sizeof(ODBC_ColumnResult));
	if(col == NULL) {
	    ODBCSetErrorMsg("ODBCDescribeResultCols malloc error", 0, 0);
	    return -1;
	}

	for(i = 0; i < *numColumns; i++)
	{
	    ret = SQLDescribeCol(hstmt, (SQLSMALLINT) i+1, col[i].columnName,
			(SQLSMALLINT) sizeof(col[i].columnName),
			&col[i].nameLength, &col[i].dataType,
			&col[i].columnSize, &col[i].decimalDigits,
			&col[i].nullable);
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		ODBCSetErrorMsg("ODBCDescribeResultCols", SQL_HANDLE_STMT,
				hstmt);
		return -1;
	    }
	    col[i].value = malloc(1);
	    if(col[i].value == NULL) {
		ODBCSetErrorMsg("ODBCSescribeResultCols malloc error", 0, 0);
		return -1;
	    }
	}
	*columns = col;
	return 0;
}
#endif /* HAVE_LIBODBC */
