/* ODBCGetResultCols.c

    ODBCGetResultCols()
*/
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libgdb.h"

#ifdef HAVE_LIBODBC

/** Retrieve the result of an sql query. Fill the columns of the
 *  ODBC_Result structure with data from SQLExecDirect or SQLFetch.
 * @param hstmt A statement handle obtained from SQLAllocHandle.
 * @param numColumns The number of columns in the result (input).
 * @param column The column descriptions obtained from ODBCDescribeCols.
 * Returns zero for success. Returns -1 for an error flag. Retrieve the
 * error number and error message with ODBCErrno() and ODBCErrMsg().
 * @see ODBCDescribeCols
 * @see ODBCGeneralQuery
 */
int
ODBCGetResultCols(SQLHSTMT hstmt, int numColumns, ODBC_ColumnResult *column)
{
	int		i, n;
	SQLRETURN	ret;
	char		**s, msg[100];

	for(i = 0; i < numColumns; i++)
	{
	    SQLINTEGER	bufferLength = 0;
	    ODBC_ColumnResult *c = &column[i];

	    switch(c->dataType)
	    {
	    case SQL_CHAR:
	    case SQL_VARCHAR:
		bufferLength = sizeof(c->bindTarget.sql_String);
		ret = SQLBindCol(hstmt, i+1, SQL_C_CHAR, &c->bindTarget,
				 bufferLength, &c->indicator);
		break;
	    case SQL_NUMERIC:
	    case SQL_FLOAT:
	    case SQL_REAL:
	    case SQL_DOUBLE:
		ret = SQLBindCol(hstmt, i+1, SQL_DOUBLE, &c->bindTarget,
				sizeof(double), &c->indicator);
		break;
	    case SQL_DECIMAL:  /* GK - Oracle ODBC driver maps number(8) to decimal */
	    case SQL_INTEGER:
		ret = SQLBindCol(hstmt, i+1, SQL_INTEGER, &c->bindTarget,
				sizeof(long), &c->indicator);
		break;
		/* ET: added SQL_BIGINT which is returned by the open source odbc 
		   driver on 64 bit for columns of type NUMBER(k) where k>=10 
		   (or perhaps already 9)
		 */  	
	    case SQL_BIGINT:
		ret = SQLBindCol(hstmt, i+1, SQL_BIGINT, &c->bindTarget,
				sizeof(long), &c->indicator);
		if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		    ret = SQLBindCol(hstmt, i+1, SQL_INTEGER, &c->bindTarget,
				sizeof(long), &c->indicator);
		}
		break;
	    case SQL_SMALLINT:
		ret = SQLBindCol(hstmt, i+1, SQL_SMALLINT, &c->bindTarget,
				sizeof(int), &c->indicator);
		break;
	    case SQL_TYPE_TIMESTAMP:
		ret = SQLBindCol(hstmt, i+1, SQL_TYPE_TIMESTAMP,
			&c->bindTarget, 0, &c->indicator);
		break;
	    default:
		ret = SQL_SUCCESS; /* skip unknown dataType?? */
	    }
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		ODBCSetErrorMsg("ODBCGetResult", SQL_HANDLE_STMT, hstmt);
		return -1;
	    }
	    c->bindTarget.sql_String[0] = '\0';
	    c->numRows = 0;
	}

	while((ret = SQLFetch(hstmt)) == SQL_SUCCESS
                        || ret == SQL_SUCCESS_WITH_INFO)
	{
	    for(i = 0; i < numColumns; i++)
	    {
		ODBC_ColumnResult *c;

		c = &column[i];
		n = c->numRows;

		switch(c->dataType)
		{
		case SQL_CHAR:
		case SQL_VARCHAR:
		    c->value = realloc(c->value, (n+1)*sizeof(char *));
		    if(c->value == NULL) {
			ODBCSetErrorMsg("ODBCGetResult", 0, 0);
			return -1;
		    }
		    s = (char **)c->value;
		    s[n] =(char *)malloc(strlen(c->bindTarget.sql_String)+1);
		    strcpy(s[n], c->bindTarget.sql_String);
		    if(s[n] == NULL) {
			ODBCSetErrorMsg("ODBCGetResult", 0, 0);
			return -1;
		    }
		    break;
		case SQL_NUMERIC:
		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
		  /* ET: replaced memcpy with assignments in the reminder ofthe
		     code below
		   */
		    // c->value = realloc(c->value, (n+1)*sizeof(SQLDOUBLE));
		    c->value = realloc(c->value, (n+1)*sizeof(double));
		    if(c->value == NULL) {
			ODBCSetErrorMsg("ODBCGetResult", 0, 0);
			return -1;
		    }
// 		    memcpy((char *)c->value+n*sizeof(SQLDOUBLE), &c->bindTarget,
// 				sizeof(SQLDOUBLE));
//		    memcpy((char *)c->value+n*sizeof(double), &c->bindTarget,
//				sizeof(double));
		    *((double *)((char *)c->value + n*sizeof(double))) = *((double *)(&c->bindTarget));
		    break;
	        case SQL_DECIMAL:  /* GK - Oracle ODBC driver maps number(8) to decimal */
		case SQL_INTEGER:
		/* ET: following line added during 64 bit migration */  
		case SQL_BIGINT:   
		    c->value = realloc(c->value, (n+1)*sizeof(long));
		    // c->value = realloc(c->value, (n+1)*sizeof(SQLINTEGER));
		    if(c->value == NULL) {
			ODBCSetErrorMsg("ODBCGetResult", 0, 0);
			return -1;
		    }
// 		    memcpy((char *)c->value+n*sizeof(SQLINTEGER),
// 				&c->bindTarget, sizeof(SQLINTEGER));
//		    memcpy((char *)c->value+n*sizeof(long),
//				&c->bindTarget, sizeof(long));
		    *((long *)((char *)c->value +n*sizeof(long))) = *((long *)(&c->bindTarget));
		    break;
		case SQL_SMALLINT:
		    c->value = realloc(c->value, (n+1)*sizeof(SQLSMALLINT));
		    memcpy((char *)c->value+n*sizeof(SQLSMALLINT),
				&c->bindTarget, sizeof(SQLSMALLINT));
		    break;
		case SQL_TYPE_TIMESTAMP:
		    c->value = realloc(c->value,
				(n+1)*sizeof(SQL_TIMESTAMP_STRUCT));
		    if(c->value == NULL) {
			ODBCSetErrorMsg("ODBCGetResult", 0, 0);
			return -1;
		    }
 //		    memcpy((char *)c->value +
 //		     n*sizeof(SQL_TIMESTAMP_STRUCT),
 //		     &c->bindTarget, sizeof(SQL_TIMESTAMP_STRUCT));
                    *((SQL_TIMESTAMP_STRUCT *)((char *)c->value+n*sizeof(SQL_TIMESTAMP_STRUCT))) = 
		      *((SQL_TIMESTAMP_STRUCT *)(&c->bindTarget));
		    break;
		default:
		    snprintf(msg, 100,
		"ODBCGetResultCols warning: unknown column data type: %d",
			(int)c->dataType);
		    logErrorMsg(LOG_WARNING, msg);
		    break;
		}
		c->numRows++;
	    }
	}
	if(ret != SQL_NO_DATA) {
	    ODBCSetErrorMsg("ODBCGetResult", SQL_HANDLE_STMT, hstmt);
	    return -1;
	}
	return 0;
}
#endif /* HAVE_LIBODBC */
