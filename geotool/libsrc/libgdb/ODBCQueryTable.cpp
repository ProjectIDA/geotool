/* ODBCQueryTable.c
 *
 *   ODBCQueryTable(), ODBCQueryTableInit(), ODBCQueryTableResults(),
 *   ODBCQueryTableClose()
 */
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include "libgdb.h"
#include "libstring.h"
#include "gobject++/CssTableClass.h"

#ifdef HAVE_LIBODBC

/**
 * Routines for submitting queries that return full table rows. The query
 * results are returned in CssTableClass structures.
 * <p>
 * ODBCQueryTable() submits the query and returns all rows in one step.
 * </p>
 * <p>
 * ODBCQueryTableInit(), ODBCQueryTableResults() and ODBCQueryTableClose()
 * can be used to submit a query and then fetch as many rows as desired.
 * Rows not fetched are discarded. For example:
 * </p>
 * <pre>
 *   #include "libgdb.h"
 *
 *   int i, num, ret, autoCommit=1;
 *   char *dataSource="ORACLE", *user="idcx", *passwd="idcx";
 *   char query[500];
 *   SQLHDBC hdbc;
 *   ODBC_QueryTableStruct *qs;
 *   Vector records;
 *
 *
 *   /\* Connect to the data source *\/
 *   if((hdbc = ODBCConnect(dataSource, user, passwd, autoCommit)) == NULL) {
 *       logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *       exit(1);
 *   }
 *
 *   strcpy(query,
 *       "select * from wfdisc where time > 1067530898 and sta like 'FIA0'");
 * 
 *   /\* Execute the query *\/
 *   if(ODBCQueryTableInit(hdbc, query, "wfdisc", &qs)) {
 *       logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *       exit(1);
 *   }
 *
 *   /\* Allocate a vector to hold the CssTableClass objects *\/
 *   records = new_Vector();
 *
 *   /\* Fetch the results in 1000 row chunks *\/
 *   while((ret = ODBCQueryTableResults(qs, 1000, &num, records)) > 0)
 *   {
 *       ...
 *       /\* Check the total number of rows received. Rows are appended. *\/
 *       if(records->elementCount > 20000) {
 *           break;  /\* If too many, stop *\/
 *       }
 *   }
 *   ODBCQueryTableClose(qs);  /\* Discards any rows not received *\/
 *
 *   if(ret < 0) {
 *       logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *   }
 *   
 *   /\* Process the records *\/
 *   for(i = 0; i < records->elementCount; i++)
 *   {
 *       CssWfdisc w = (CssWfdisc)records->elementData[i];
 *       if(w->calper < .1) {
 *       ...
 *   }
 *
 *   GObject_free((GObject)records);
 *   ODBCDisconnect(hdbc);
 * </pre>
 */

static int bindColumn(SQLHSTMT hstmt, int colIndex, BindColumn *c);

/**
 *  Fetch all table rows for a query. All the table rows are returned for
 *  the input query as CssTableClass objects.
 *  @param hdbc A connection handle obtained from ODBCConnect().
 *  @param query An SQL query that should return full table rows.
 *  @param cssTableName The CSS3.0 table name of the expected rows (input).
 *  @param table A Vector array to receive the CssTableClass objects.
 *  @return 0 for success, -1 for failure.  Retrieve the error number and \
 *		error message with ODBCErrno() and ODBCErrMsg().
 *  @see ODBCConnect
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 *  @see new_Vector
 *  @see CssTableClass
 */
int
ODBCQueryTable(SQLHDBC hdbc, const string &query, const string &cssTableName,
		gvector<CssTableClass *> &table)
{
	ODBC_QueryTableStruct *qs;
	int nrows, ret;

	if(ODBCQueryTableInit(hdbc, query, cssTableName, &qs)) return -1;

	table.clear();

	while((ret = ODBCQueryTableResults(qs, 1000, &nrows, table)) > 0);

	if(ODBCQueryTableClose(qs)) ret = -1;

	return ret;
}

/**
 *  Submit a query that will return full table rows. This routine is used
 *  together with ODBCQueryTableResults() and ODBCQueryTableClose() to
 *  submit a query and fetch the results in chunks. An ODBC_QueryTableStruct
 *  pointer is returned that can be used to retrieve the desired number of
 *  table rows with ODBCQueryTableResults().
 *  @param hdbc A connection handle obtained from ODBCConnect().
 *  @param query An SQL query that should return full table rows.
 *  @param cssTableName The CSS3.0 table name of the expected rows (input).
 *  @param qs A ODBC_QueryTableStruct pointer that is returned.
 *  @return 0 for success, -1 for failure.  Retrieve the error number and \
 *		error message with ODBCErrno() and ODBCErrMsg().
 *  @see ODBCConnect
 *  @see ODBCQueryTableResults
 *  @see ODBCQueryTableClose
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 */
int
ODBCQueryTableInit(SQLHDBC hdbc, const string &query,const string &cssTableName,
			ODBC_QueryTableStruct **qs)
{
	ODBC_QueryTableStruct *q;
	string		account, name;
	int		i, j, numColumns;
	SQLRETURN	ret = SQL_SUCCESS;
	ODBC_Column	*columns;
	CssTableClass	*t;

	if((q = (ODBC_QueryTableStruct *)
		malloc(sizeof(ODBC_QueryTableStruct))) == NULL)
	{
	    ODBCSetErrorMsg("ODBCQueryTableInit malloc error.", 0, 0);
	    return -1;
	}
	q->cssTableName = NULL;
	q->rowStatusArray = NULL;
	q->bindCol = NULL;
	q->hdbc = hdbc;

	if((t = CssTableClass::createCssTable(cssTableName)) == NULL) {
	    char buf[1000];
	    snprintf(buf, sizeof(buf),
		"ODBCQueryTableInit cssTableName %s is not recognized.",
			cssTableName.c_str());
	    ODBCSetErrorMsg(buf, 0, 0);
	    return -1;
	}
	numColumns = t->getNumMembers();
	q->des = t->description();
	delete t;

	q->cssTableName = (char *)malloc(cssTableName.length()+1);
	if(q->cssTableName == NULL) {
	    free(q);
	    ODBCSetErrorMsg("ODBCQueryTableInit malloc error.", 0, 0);
	    return -1;
	}
	strcpy(q->cssTableName, cssTableName.c_str());

	if(!ODBCGetTableName(query, account, name)) {
	    q->account = stringToQuark(account);
	    q->tableName = stringToQuark(name);
	}
	else {
	    q->account = stringToQuark("");
	    q->tableName = stringToQuark("");
	}

	q->k1 = 0;
	q->numRowsFetched = 0;

	q->odbc_numColumns = numColumns;
	columns = (ODBC_Column *)malloc(numColumns*sizeof(ODBC_Column));
	if(columns == NULL) {
	    ODBCSetErrorMsg("ODBCQueryTableInit malloc error.", 0, 0);
	    free(q->cssTableName);
	    free(q);
	    return -1;
	}
	for(i = 0; i < numColumns; i++)
	{
	    stringcpy((char *)columns[i].columnName, q->des[i].name,
			ODBC_STRLEN);
	    if(q->des[i].type == CSS_STRING) {
		columns[i].dataType = SQL_VARCHAR;
	    }
	    else if(q->des[i].type == CSS_TIME || q->des[i].type == CSS_DOUBLE
			|| q->des[i].type == CSS_FLOAT)
	    {
		columns[i].dataType = SQL_DOUBLE;
	    }
	    else if(q->des[i].type == CSS_LONG || q->des[i].type == CSS_JDATE) {
		columns[i].dataType = SQL_INTEGER;
	    }
	    else if(q->des[i].type == CSS_INT) {
		columns[i].dataType = SQL_SMALLINT;
	    }
	    else if(q->des[i].type == CSS_DATE) {
		columns[i].dataType = SQL_C_TYPE_DATE;
	    }
	    else if(q->des[i].type == CSS_LDDATE) {
		columns[i].dataType = SQL_C_TYPE_DATE;
	    }
	}

	ret = ODBCAllocStmt(hdbc, &q->hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCQueryTableInit", SQL_HANDLE_DBC, hdbc);
	    free(columns); free(q->cssTableName); free(q);
	    return -1;
	}

	ret = SQLExecDirect(q->hstmt, (unsigned char *)query.c_str(), SQL_NTS);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCQueryTableInit", SQL_HANDLE_STMT, q->hstmt);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, q->hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCQueryTableInit: SQLFreeHandle failed.");
	    }
	    free(columns); free(q->cssTableName); free(q);
	    return -1;
	}
	q->bindCol =(BindColumn *)malloc(q->odbc_numColumns*sizeof(BindColumn));
	if(q->bindCol == NULL) {
	    ODBCSetErrorMsg("ODBCQueryTableInit malloc error.", 0, 0);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, q->hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCQueryTableInit: SQLFreeHandle failed.");
	    }
	    free(columns); free(q->cssTableName); free(q);
	    return -1;
	}

	/* Set the SQL_ATTR_ROW_BIND_TYPE statement attribute to use
	 * column-wise binding. Declare the rowset size with the
	 * SQL_ATTR_ROW_ARRAY_SIZE statement attribute. Set the
	 * SQL_ATTR_ROW_STATUS_PTR statement attribute to point to the
	 * row status array. Set the SQL_ATTR_ROWS_FETCHED_PTR statement
	 * attribute to point to RowsFetched.
	 */
	if(ROW_ARRAY_SIZE > 1) {
	    SQLSetStmtAttr(q->hstmt, SQL_ATTR_ROW_BIND_TYPE,
				SQL_BIND_BY_COLUMN, 0);
	
	    SQLSetStmtAttr(q->hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
			(SQLPOINTER)ROW_ARRAY_SIZE, 0);

	    q->rowStatusArray = (SQLUSMALLINT *)
			malloc(ROW_ARRAY_SIZE*sizeof(SQLUSMALLINT));
	    if(q->rowStatusArray == NULL) {
		ODBCSetErrorMsg("ODBCQueryTableInit malloc error.", 0, 0);
		if(SQLFreeHandle(SQL_HANDLE_STMT, q->hstmt) != SQL_SUCCESS) {
		    logErrorMsg(LOG_WARNING,
			"ODBCQueryTableInit: SQLFreeHandle failed.");
		}
		free(columns); free(q->cssTableName); free(q->bindCol); free(q);
		return -1;
	    }
	    SQLSetStmtAttr(q->hstmt, SQL_ATTR_ROW_STATUS_PTR,
			q->rowStatusArray, 0);
	}
	SQLSetStmtAttr(q->hstmt, SQL_ATTR_ROWS_FETCHED_PTR,
			&q->numRowsFetched, 0);
	
	for(i = 0; i < q->odbc_numColumns; i++)
	{
	    BindColumn *c = &q->bindCol[i];

	    /* find this column name in the CssTableClass object.
	     */
	    for(j = 0; j < numColumns
		&& strcasecmp((const char *)columns[i].columnName,
				q->des[j].name); j++);

	    if(j < numColumns)
	    {
		/* bind this odbc_column and return in the CssTableClass
		 */
		c->index = j;
		c->dataType = columns[i].dataType;
		if(bindColumn(q->hstmt, i+1, c))
		{
		    if(SQLFreeHandle(SQL_HANDLE_STMT,q->hstmt) != SQL_SUCCESS) {
			logErrorMsg(LOG_WARNING,
			    "ODBCQueryTableInit: SQLFreeHandle failed.");
		    }
		    free(columns); free(q->cssTableName);
		    if(q->rowStatusArray) free(q->rowStatusArray);
		    free(q);
		    return -1;
		}
	    }
	    else {
		/* this odbc_column is not returned. */
		c->index = -1;
	    }
	}
	free(columns);

	*qs = q;
	return 0;
}

/**
 *  Fetch the table rows that result from a query submitted to
 *  ODBCQueryTableInit(). The rows are returned as CssTableClass objects in the
 *  Vector table. The CssTableClass objects are <b>appended</b> to the Vector.
 *  @param qs A ODBC_QueryTableStruct pointer that was obtained from \
 *		ODBCQueryTableInit().
 *  @param numToFetch The maximum number of table rows this routine will return.
 *  @param numFetched The actual number of table rows returned.
 *  @param table A vector to hold the CssTableClass objects. Objects are appended.
 *  @return the number of rows fetched or -1 for failure.  Retrieve the error \
 *		number and error message with ODBCErrno() and ODBCErrMsg().
 *  @see ODBCQueryTableInit
 *  @see ODBCQueryTableClose
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 */
int
ODBCQueryTableResults(ODBC_QueryTableStruct *qs, int numToFetch,
			int *numFetched, gvector<CssTableClass *> &table)
{
	int	i, j, k, l, m, n, numRows;
	int	source_q, user_q, passwd_q, format_q;
	char	*column_address, msg[100];
	SQLRETURN ret = SQL_SUCCESS;
	CssClassDescription	*des = qs->des;
	CssTableClass *t, *tarray[ROW_ARRAY_SIZE];

	format_q = stringToQuark("odbc");
	source_q = stringToQuark(ODBCGetDataSource(qs->hdbc));
	user_q = stringToQuark(ODBCGetUser(qs->hdbc));
	passwd_q = stringToQuark(ODBCGetPassword(qs->hdbc));

	*numFetched = 0;
	numRows = 0;

	while(numRows < numToFetch)
	{
	    /* If the last call to ODBCQueryTableResults requested less than
	     * was returned by SQLFetchScroll, then there are some records
	     * left in the buffer, starting at index qs->k1. If qs->k1 >=
	     * qs->numRowsFetched, then call SQLFetchScroll to fetch more rows.
	     */
	    if(qs->k1 >= (signed int)qs->numRowsFetched)
	    {
		ret = SQLFetchScroll(qs->hstmt, SQL_FETCH_NEXT, 0);
		if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		    break;
		}
		qs->k1 = 0;
	    }

	    /* Count the number of tarray elements that will be returned in
	     * the loop below.
	     */
	    n = 0;
	    for(k = qs->k1; k < (signed int)qs->numRowsFetched &&
		numRows+n < numToFetch; k++) n++;

	    /* Allocate n CssTableClass objects;
	     */
	    for(i = 0; i < n; i++) {
		tarray[i] = CssTableClass::createCssTable(qs->cssTableName);
	    }

	    l = 0;
	    for(k = qs->k1; k < (signed int)qs->numRowsFetched &&
		numRows < numToFetch; k++)
	    {
		qs->k1++;
		t = tarray[l++];
		table.push_back(t);

		t->setFormat(format_q);
		t->setSource(source_q, user_q, passwd_q);
		t->setAccount(qs->account, qs->tableName);

		for(i = 0; i < qs->odbc_numColumns; i++)
			if(qs->bindCol[i].index >= 0)
		{
		    BindColumn *c = &qs->bindCol[i];

		    j = qs->bindCol[i].index;
		    column_address = (char *)t + des[j].offset;

		    switch(c->dataType)
		    {
		    case SQL_CHAR:
		    case SQL_VARCHAR:
			if(des[j].type == CSS_STRING && 
                              c->bindTarget.sql_String[k] != NULL)
                        {
			    n = strlen(c->bindTarget.sql_String[k]);
			    m = des[j].end - des[j].start + 1;
			    if(n <= m) {
				strcpy((char *)column_address,
					c->bindTarget.sql_String[k]);
			    }
			    else {
				strncpy((char *)column_address,
					c->bindTarget.sql_String[k], m);
				((char *)column_address)[m] = '\0';
			    }
			}
			else {
			    ((char *)column_address)[0] = '\0';
			}
			if(des[j].quark_offset > 0) {
			    char *q = (char *)t + des[j].quark_offset;
			    *(int *)q = stringToQuark(column_address);
			}
			break;
		    case SQL_NUMERIC:
		    case SQL_FLOAT:
		    case SQL_REAL:
		    case SQL_DOUBLE:
			if(des[j].type == CSS_TIME || des[j].type ==CSS_DOUBLE){
			  /* ET: changed the following line in the course of 
			     64bit migration
			     memcpy(column_address, &c->bindTarget.sql_Double[k],
			     sizeof(SQLDOUBLE));
			  */
			  *((SQLDOUBLE *)column_address) = c->bindTarget.sql_Double[k];
			}
			else if(des[j].type == CSS_FLOAT) {
			    *((float *)column_address) =
				c->bindTarget.sql_Double[k];
			}
			break;
			/* ET: added next line in the course of 64 bit migration */
		    case SQL_BIGINT: //fall-through	
		    case SQL_INTEGER:
			if(des[j].type == CSS_LONG || des[j].type == CSS_JDATE){
			    /* In sqltypes.h, we have the comment:
 			     * I have just discovered that on win64 sizeof(long) == 4, so its
                             * all smoke and mirrors...
			     * Which makes me wonder how robust ODBC is under 64 bit.
			     * there was a problem when the following statement had
			     * under 64 bit where sizeof(long) was sizeof(SQLINTEGER),
			     * so it was changed to be sizeof(long), and the 64 bit problem 
			     * went away.
			     */
			    /* ET: changed the following line as shown below after 
			       encountering problems on 64 bit Linux */
			    /* memcpy(column_address, &c->bindTarget.sql_Long[k],
			       sizeof(long)); */
			    *((long *)column_address) =c->bindTarget.sql_Long[k];
			}
			else if(des[j].type == CSS_INT) {
			    *((int *)column_address) =c->bindTarget.sql_Long[k];
			}
			break;
		    case SQL_SMALLINT:
			if(des[j].type == CSS_INT) {
			  /*ET: changed the following line in the course of 64 bit 
			    migration 
			    memcpy(column_address, &c->bindTarget.sql_Int[k],
			    sizeof(SQLSMALLINT)); 
			  */
			  *((SQLSMALLINT *)column_address) = c->bindTarget.sql_Int[k];
			}
			else if(des[j].type == CSS_LONG
				|| des[j].type == CSS_JDATE) {
			    *((long *)column_address) =c->bindTarget.sql_Int[k];
			}
			break;
		    case SQL_C_TYPE_DATE:
			if(des[j].type == CSS_DATE || des[j].type == CSS_LDDATE)
			{
			    SQL_DATE_STRUCT *std = &c->bindTarget.sql_Date[k];
			    DateTime *dt = (DateTime *)column_address;
			    dt->year = std->year;
			    dt->month = std->month;
			    dt->day = std->day;
			}
			break;
		    default:
			snprintf(msg, 100,
		      "ODBCQueryTable warning: unknown column data type: %d\n", 
				(int)c->dataType);
			logErrorMsg(LOG_WARNING, msg);
			break;
		    }
		}
		numRows++;
	    }
	}
	*numFetched = numRows;

	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO
		&& ret != SQL_NO_DATA)
	{
	    ODBCSetErrorMsg("ODBCGetResult", SQL_HANDLE_STMT, qs->hstmt);
	    return -1;
	}
	return numRows;
}

/**
 *  Terminate a query that was initiated by ODBCQueryTableInit().
 *  This routine can be called anytime after ODBCQueryTableInit(). All rows
 *  that have not been fetched with ODBCQueryTableResults() will be
 *  discarded. This routine should be called to free the resources allocated
 *  by ODBCQueryTableInit.
 *  @param qs A ODBC_QueryTableStruct pointer that was obtained from \
 *		ODBCQueryTableInit().
 *  @return 0 for success, -1 for failure.  Retrieve the error number and \
 *		error message with ODBCErrno() and ODBCErrMsg().
 *  @see ODBCQueryTableInit
 *  @see ODBCQueryTableResults
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 */
int
ODBCQueryTableClose(ODBC_QueryTableStruct *qs)
{
	if(SQLFreeHandle(SQL_HANDLE_STMT, qs->hstmt) != SQL_SUCCESS) {
	    ODBCSetErrorMsg("ODBCQueryTableClose: SQLFreeHandle failed.", 0, 0);
	    return -1;
	}
	free(qs->cssTableName);
	if(qs->rowStatusArray) free(qs->rowStatusArray);
	free(qs->bindCol);
	free(qs);
	return 0;
}

/**
 *  Bind a column according to the data type.
 */
static int
bindColumn(SQLHSTMT hstmt, int colIndex, BindColumn *c)
{
	SQLRETURN ret;

	switch(c->dataType)
	{
	    case SQL_CHAR:
	    case SQL_VARCHAR:
		ret = SQLBindCol(hstmt, colIndex, SQL_C_CHAR,
			c->bindTarget.sql_String, ODBC_STRLEN, c->indicator);
		break;
	    case SQL_NUMERIC:
	    case SQL_FLOAT:
	    case SQL_REAL:
	    case SQL_DOUBLE:
		ret = SQLBindCol(hstmt, colIndex, SQL_C_DOUBLE,
			c->bindTarget.sql_Double,
			sizeof(c->bindTarget.sql_Double[0]), c->indicator);
		break;
	    case SQL_INTEGER:
		ret = SQLBindCol(hstmt, colIndex, SQL_INTEGER,
			c->bindTarget.sql_Long,
			sizeof(c->bindTarget.sql_Long[0]), c->indicator);
		break;
	    case SQL_SMALLINT:
		ret = SQLBindCol(hstmt, colIndex, SQL_SMALLINT,
			c->bindTarget.sql_Int,
			sizeof(c->bindTarget.sql_Int[0]), c->indicator);
		break;
	    case SQL_C_TYPE_DATE:
		ret = SQLBindCol(hstmt, colIndex, SQL_C_TYPE_DATE,
			c->bindTarget.sql_Date,
			sizeof(c->bindTarget.sql_Date[0]), c->indicator);
		break;
	    default:
		ret = SQL_SUCCESS; /* skip unknown dataType ?? */
	}
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCQueryTable", SQL_HANDLE_STMT, hstmt);
	    return -1;
	}
	return 0;
}

#endif /* HAVE_LIBODBC */
