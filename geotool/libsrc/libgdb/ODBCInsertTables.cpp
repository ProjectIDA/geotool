/* ODBCInsertTables.c

    ODBCInsertTable()
    ODBCInsertTables()
*/
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#ifdef HAVE_LIBODBC

#include "libgdb.h"
#include "libstring.h"
#include "gobject++/CssTableClass.h"

#include "sqltypes.h"

/**
 *  These routines insert one or more records into the database.
 *  <p>
 *  All data is transfered to the ODBC server by binding to memory locations.
 *  (Insert statments use the '?' character to denote bound parameters.)
 */

/**
 * @private
 */

/* ET: The type of sql_Long was changed from SQLINTEGER to long to make the 
   code 64-bit compatible. Defining sql_Long as SQLINTEGER caused an error 
   when executing the insert statement. On 64 bit sizeof(SQLINTEGER) = 4 and
   sizeof(long) = 8;
 */

typedef struct
{
	SQLLEN        	length;
	union {
	    char		*sql_String;
	    SQLDOUBLE		sql_Double;
	    long		sql_Long;
	    SQL_DATE_STRUCT	sql_DateTime;
        } bindTarget;
} BindParam;


static int bindTargets(SQLHSTMT hstmt, int numColumns, BindParam *bp,
		CssClassDescription *des);
static int copyMembers(int numColumns, BindParam *bp, CssClassDescription *des,
		CssTableClass *t);

/**
 * Insert one table record. 
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param table The CssTableClass object whose elements will be inserted.
 * @return 1 for success. -1 for failure. Retrieve the error number \
 * and the error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCInsertTable(SQLHDBC hdbc, const string &tableName, CssTableClass *table)
{
	return ODBCInsertTables(hdbc, tableName, 1, &table);
}

/**
 * Insert more than one table record. 
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param num The number of records to insert from tables[].
 * @param tables The CssTableClass objects whose elements will be inserted.
 * @return the number of records inserted.  If less than num, retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCInsertTables(SQLHDBC hdbc, const string &tableName, int num,
			CssTableClass **tables)
{
	SQLRETURN ret;
	SQLHSTMT hstmt;
	char insert[5000];
	int i, numColumns;
	int source_q, user_q, passwd_q, table_q, format_q;
	BindParam *bp;
	CssClassDescription *des;

	if(num <= 0) return 0;

	des = tables[0]->description();
	numColumns = tables[0]->getNumMembers();

	snprintf(insert, 5000, "insert into %s (", tableName.c_str());
	for(i = 0; i < numColumns; i++) {
	    if(i > 0) strcat(insert, ",");
	    strcat(insert, des[i].name);
	}
	strcat(insert, ") values (");
	for(i = 0; i < numColumns; i++) {
	    if(i > 0) strcat(insert, ",");
	    if(des[i].type != CSS_LDDATE) {
		strcat(insert, "?");
	    }
	    else {
		strcat(insert, "CURRENT_TIMESTAMP"); /* load date */
	    }
	}
	strcat(insert, ")");

	if(!hdbc) {
	    ODBCSetErrorMsg("ODBCInsertTables: hdbc=NULL.", 0, 0);
	    return -1;
	}
	ret = ODBCAllocStmt(hdbc, &hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCInsertTables: ODBCAllocStmt failed.",
			SQL_HANDLE_DBC, hdbc);
	    return -1;
	}

	ret = SQLPrepare(hstmt, (unsigned char *) insert, SQL_NTS);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCInsertTables: SQLPrepare failed.",
			SQL_HANDLE_DBC, hdbc);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCInsertTables: SQLFreeHandle failed.");
	    }
	    return -1;
	}
	if((bp = (BindParam *)malloc(numColumns*sizeof(BindParam))) == NULL) {
	    ODBCSetErrorMsg("ODBCInsertTables malloc error.", 0, 0);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCInsertTables: SQLFreeHandle failed.");
	    }
	    return -1;
	}

	ret = bindTargets(hstmt, numColumns, bp, des);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCInsertTables: SQLFreeHandle failed.");
	    }
	    for(i = 0; i < numColumns; i++) {
		if(des[i].type == CSS_STRING) {
		    free(bp[i].bindTarget.sql_String);
		}
	    }
	    free(bp);
	    return -1;
	}

	source_q = stringToQuark(ODBCGetDataSource(hdbc));
	user_q = stringToQuark(ODBCGetUser(hdbc));
	passwd_q = stringToQuark(ODBCGetPassword(hdbc));
	table_q = stringToQuark(tableName.c_str());
	format_q = stringToQuark("odbc");

	for(i = 0; i < num; i++)
	{
	    copyMembers(numColumns, bp, des, tables[i]);

	    ret = SQLExecute(hstmt);

	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		ODBCSetErrorMsg("ODBCInsertTables", SQL_HANDLE_STMT, hstmt);

		break;
	    }

	    if(SQLFreeStmt(hstmt, SQL_CLOSE) != SQL_SUCCESS) {
	       logErrorMsg(LOG_WARNING,"ODBCInsertTables: SQLFreeStmt failed.");
		break;
	    }
	    tables[i]->setFormat(format_q);
	    tables[i]->setSource(source_q, user_q, passwd_q);
	    tables[i]->setAccount(user_q, table_q);
	}
	ret = i; /* return the number of records successfully inserted */

	for(i = 0; i < numColumns; i++) {
	    if(des[i].type == CSS_STRING) free(bp[i].bindTarget.sql_String);
	}
	free(bp);
	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	    logErrorMsg(LOG_WARNING, "ODBCInsertTables: SQLFreeHandle failed.");
	}

	return ret;
}

/**
 *  Bind memory parameters locations for the ODBC routines. When SQLExecute()
 *  is called, the values of the insert statement will be obtained from
 *  the bp[].bindTarget.
 */
static int
bindTargets(SQLHSTMT hstmt, int numColumns, BindParam *bp, CssClassDescription *des)
{
	int i, len;
	SQLRETURN ret;

	for(i = 0; i < numColumns; i++)
	{
	    bp[i].bindTarget.sql_String = NULL;

	    switch(des[i].type)
	    {
		case CSS_STRING:
		    len = des[i].end - des[i].start + 1;
		    if((bp[i].bindTarget.sql_String = (char *)malloc(len+1))
				== NULL)
		    {
			ODBCSetErrorMsg("bindTargets malloc error.", 0, 0);
			return -1;
		    }
		    bp[i].length = SQL_NTS;
		    ret = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT,
				SQL_C_CHAR, SQL_CHAR, len, 0,
				bp[i].bindTarget.sql_String, len+1,
				&bp[i].length);
		    break;
		case CSS_DOUBLE:
		case CSS_TIME:
		case CSS_FLOAT:
		    bp[i].length = 0;
		    ret = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT,
				SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
				&bp[i].bindTarget.sql_Double, 0, &bp[i].length);
		    break;
		case CSS_INT:
		case CSS_JDATE:
		case CSS_LONG:
		    bp[i].length = 0;
                    /* ET: The following line was changed to cater for the fact that on 64 bit,
                       sizeof(SQL_C_LONG) = sizeof(long) = 8, sizeof(SQL_INTEGER) = 4 and 
		       sizeof(SQL_BIGINT) = 8.
		       ret = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT,
		       SQL_C_LONG, SQL_INTEGER, 0, 0,
		       &bp[i].bindTarget.sql_Long, 0, &bp[i].length); */
		    
		    ret = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT,
					   SQL_C_SLONG, SQL_BIGINT, 0, 0,
					   &bp[i].bindTarget.sql_Long, 0, &bp[i].length); 
		    
		      break;
		case CSS_DATE:
		    bp[i].length = 0;
		    ret = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT,
				SQL_C_TYPE_DATE, SQL_TYPE_DATE, 0, 0,
				&bp[i].bindTarget.sql_DateTime, 0,
				&bp[i].length);
		    break;
		default:
		    ret = SQL_SUCCESS; /* skip unknown dataType?? */
	    }
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		ODBCSetErrorMsg("bindTargets", SQL_HANDLE_STMT, hstmt);
		return -1;
	    }
	}
	return 0;
}

/**
 *  Copy elements of a CssTableClass into the bound memory locations
 *  (bd[i].bindTarget).
 */
static int
copyMembers(int numColumns, BindParam *bp, CssClassDescription *des, CssTableClass *t)
{
	char *member;
	int i, len;
	SQL_DATE_STRUCT *ts;
	DateTime *dt;

	for(i = 0; i < numColumns; i++)
	{
	    member = (char *)t + des[i].offset;
	    switch(des[i].type)
	    {
		case CSS_STRING:
		    len = des[i].end - des[i].start + 1;
		    strncpy(bp[i].bindTarget.sql_String, member, len);
		    bp[i].bindTarget.sql_String[len] = '\0';
		    break;
		case CSS_DOUBLE:
		    bp[i].bindTarget.sql_Double = *(double *)member;
		    break;
		case CSS_TIME:
		    bp[i].bindTarget.sql_Double = *(double *)member;
		    break;
		case CSS_FLOAT:
		    bp[i].bindTarget.sql_Double = *(float *)member;
		    break;
		case CSS_INT:
		    bp[i].bindTarget.sql_Long = *(int *)member;
		    break;
		case CSS_JDATE:
		case CSS_LONG:
		    bp[i].bindTarget.sql_Long = *(long *)member;
		    break;
		case CSS_DATE:
		    dt = (DateTime *)member;
		    ts = (SQL_DATE_STRUCT *)&bp[i].bindTarget;
		    ts->year = dt->year;
		    if(ts->year < 0) ts->year = 0;
		    if(ts->year > 9999) ts->year = 9999;
		    ts->month = dt->month;
		    if(ts->month < 1) ts->month = 1;
		    if(ts->month > 12) ts->month = 12;
		    ts->day = dt->day;
		    if(ts->day < 1) ts->day = 1;
		    if(ts->day > 31) ts->day = 31;
		    break;
	    }
	}
	return 0;
}
#endif /* HAVE_LIBODBC */
