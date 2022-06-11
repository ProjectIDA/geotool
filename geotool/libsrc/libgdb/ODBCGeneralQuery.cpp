/* ODBCGeneralQuery.c

    ODBCQuery()
    ODBCGeneralQuery()
*/
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libgdb.h"
#include "libstring.h"
#include "gobject++/CssTables.h"

#ifdef HAVE_LIBODBC

/**
 * The routines ODBCQuery and ODBCGeneralQuery are used to retrieve the results
 * of a general SQL query for which the number of columns in the result and
 * their data types do not need to be specified beforehand. Unlike
 * ODBCQueryTable and ODBCQueryTableResult, which return complete CSS table
 * rows in CssTableClass structures, ODBCQuery and ODBCGeneralQuery can return any
 * combination of table members or table joins.
 * <p>
 * ODBCQuery returns the query results as "dynamic" CssTableClass structures. One
 * of five CssTableClass subclasses will be returned: CssDynamic5Class, CssDynamic10Class,
 * CssDynamic20Class, CssDynamic40Class and CssDynamic that hold up to 5, 10, 20, 40 and
 * 60 columns (members), respectively. (Columns greater that 60 are ignored.)
 * These are "dynamic" CssTableClass structures, since the description of their
 * members is created dynamically instead of being statically defined like
 * the standard CSS tables (wfdisc, arrival, etc).  ODBCQuery creates the
 * dynamic structures as needed.
 * <p>
 * ODBCGeneralQuery returns the query results in an ODBC_Result structure,
 * which is defined as:
 * <pre>
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
 *          SQLINTEGER                 sql_Long;
 *          SQLSMALLINT          sql_Int;
 *          SQL_TIMESTAMP_STRUCT sql_DateTime;
 *      } bindTarget;
 *      SQLINTEGER           indicator;
 *      SQLINTEGER           numRows;
 *  } ODBC_ColumnResult;
 *  
 *  typedef struct
 *  {
 *      int               numColumns;
 *      int               numRows;
 *      ODBC_ColumnResult *column;
 *  } ODBC_Result;
 * </pre>
 * <p>
 *
 * The following example code demonstrates the use of ODBCGeneralQuery and
 * the ODBC_Result structure.
 *<pre>
 *
 * #include "libgdb.h"
 * 
 * int
 * main(int argc, char **argv)
 * {
 *     int i, autoCommit=0;
 *     char *dataSource="ORACLE", *user="sel3", *passwd="sel3";
 *     char **sta, **iphase, **phase;
 *     double *azimuth, *seaz, *delta;
 *     long *orid;
 *     SQLRETURN ret;
 *     SQLHDBC hdbc;
 *     ODBC_Result result;
 * 
 *     /\* Connect to the data source
 *      *\/
 *     if((hdbc = ODBCConnect(dataSource, user, passwd, autoCommit)) == NULL) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 * 
 *     /\* Submit the query and disconnect
 *      *\/
 *     ret = ODBCGeneralQuery(hdbc,
 *             "select r.sta,r.iphase,a.phase,r.azimuth,a.seaz,a.delta,a.orid\\
 *             from arrival r, assoc a where a.orid < 100 and a.arid=r.arid",
 *             &result);
 * 
 *     ODBCDisconnect(hdbc);
 * 
 *     if(ret < 0) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 * 
 *     if(result.numRows == 0) {
 *         logErrorMsg(LOG_WARNING, "No rows returned.");
 *         ODBCFreeResult(&result);
 *         exit(0);
 *     }
 * 
 *     /\* Set pointers to the result columns
 *      *\/
 *     sta     = (char **) result.column[0].value;
 *     iphase  = (char **) result.column[1].value;
 *     phase   = (char **) result.column[2].value;
 *     azimuth = (double *)result.column[3].value;
 *     seaz    = (double *)result.column[4].value;
 *     delta   = (double *)result.column[5].value;
 *     orid    = (long *)  result.column[6].value;
 * 
 *     /\* Print the results
 *      *\/
 *     for(i = 0; i < result.numRows; i++) {
 *         printf("%6s %8s %8s %8.2lf %8.2lf %8.2lf %ld\n", sta[i], iphase[i],
 *             phase[i], azimuth[i], seaz[i], delta[i], orid[i]);
 *     }
 * 
 *     /\* Free the result
 *      *\/
 *     ODBCFreeResult(&result);
 * 
 *     return(0);
 * }
 * </pre>
 * <p>
 * In the code above, we knew the number and types of columns that would
 * be returned, and could directly use the result.column[].value pointer.
 * Alternatively, since the ODBC_Result describes each column type, so we
 * could handle the results in a more general manner. For example, instead of
 * using pointers to the result.column[].value to print, we could make a
 * generic print routine. The printing part of the code would then look like:
 <pre>
 *     ...
 *     if(result.numRows == 0) {
 *         logErrorMsg(LOG_WARNING, "No rows returned.");
 *         ODBCFreeResult(&result);
 *         exit(0);
 *     }
 *
 *     /\* Print the results
 *      *\/
 *     printResults(&result);
 *
 *     /\* Free the result
 *      *\/
 *     ODBCFreeResult(&result);
 *
 *     return(0);
 * }
 *
 * /\* This generic print routine can print the results of any query.
 *  *\/
 * static void
 * printResults(ODBC_Result *result)
 * {
 *     int i, j, yr;
 *     char buf[18];
 *     SQL_TIMESTAMP_STRUCT *time, *ts;
 * 
 * 
 *     for(j = 0; j < result->numColumns; j++) {
 *         printf("%8s ", result->column[j].columnName);
 *     }
 *     printf("\n");
 * 
 *     for(i = 0; i < result->numRows; i++)
 *     {
 *         for(j = 0; j < result->numColumns; j++)
 *         {
 *             ODBC_ColumnResult *c = &result->column[j];
 * 
 *             switch(c->dataType)
 *             {
 *             case SQL_CHAR:
 *             case SQL_VARCHAR:
 *                 printf("%8s ", ((char **)c->value)[i]);
 *                 break;
 *             case SQL_NUMERIC:
 *             case SQL_FLOAT:
 *             case SQL_REAL:
 *             case SQL_DOUBLE:
 *                 printf("%8.2lf ", ((double *)c->value)[i]);
 *                 break;
 *             case SQL_INTEGER:
 *                 printf("%8ld ", ((long *)c->value)[i]);
 *                 break;
 *             case SQL_DECIMAL:
 *             case SQL_SMALLINT:
 *                 printf("%8d ", ((int *)c->value)[i]);
 *                 break;
 *             case SQL_TYPE_TIMESTAMP:
 *                 ts = &((SQL_TIMESTAMP_STRUCT *)c->value)[i];
 *                 yr = (ts->year >= 100)
 *                         ? ts->year - (ts->year/100)*100 : ts->year;
 *                 printf("%02d/%02d/%02d %02d:%02d:%02d", yr, ts->month,
 *                     ts->day, ts->hour, ts->minute, ts->second);
 *                 break;
 *             }
 *         }
 *         printf("\n");
 *     }
 * }
 * </pre>
 * The following code demonstrates the use of ODBCQuery and CssTableClass objects
 * to accomplish the same task.
 *<pre>
 * #include "libgdb.h"
 * 
 * int
 * main(int argc, char **argv)
 * {
 *     int i, autoCommit=0, numColumns;
 *     char *dataSource="ORACLE", *user="sel3", *passwd="sel3";
 *     SQLRETURN ret;
 *     SQLHDBC hdbc;
 *     CssClassDescription *des;
 *     Vector v;
 * 
 *     /\* Connect to the data source
 *      *\/
 *     if((hdbc = ODBCConnect(dataSource, user, passwd, autoCommit)) == NULL) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 * 
 *     /\* Allocate a vector to hold the results
 *      *\/
 *     v = new_Vector();
 * 
 *     /\* Submit the query and disconnect
 *      *\/
 *     ret = ODBCQuery(hdbc,
 *             "select r.sta,r.iphase,a.phase,r.azimuth,a.seaz,a.delta,a.orid\\
 *             from arrival r, assoc a where a.orid < 100 and a.arid=r.arid",
 *             v);
 * 
 *     ODBCDisconnect(hdbc);
 * 
 *     if(ret < 0) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 * 
 *     /\* Check if any rows were returned.
 *      *\/
 *     if(v->elementCount == 0) {
 *         logErrorMsg(LOG_WARNING, "No rows returned.");
 *         GObject_free((GObject)v);
 *         exit(0);
 *     }
 * 
 *     /\* Get the number of columns (members)
 *      *\/
 *     numColumns = CSSTABLE_numMembers((CssTableClass)v->elementData[0]);
 * 
 *     /\* Get the member description structures
 *      *\/
 *     des = CssTable_des((CssTableClass)v->elementData[0]);
 * 
 *     /\* Print the column (member) names
 *      *\/
 *     for(i = 0; i < numColumns; i++) {
 *         printf("%8s ", des[i].name);
 *     }
 *     printf("\n");
 * 
 *     /\* Print the results (rows)
 *      *\/
 *     for(i = 0; i < v->elementCount; i++) {
 *         printf("%s\n", CssTable_toString(v->elementData[i]));
 *     }
 * 
 *     /\* Free the results
 *      *\/
 *     GObject_free((GObject)v);
 * 
 *     return(0);
 * }
* </pre>
*/

static void copyChar5(CssDynamic5Class **tarray, int i, int nrows, char **value);
static void copyChar10(CssDynamic10Class **tarray, int i, int nrows, char **value);
static void copyChar20(CssDynamic20Class **tarray, int i, int nrows, char **value);
static void copyChar40(CssDynamic40Class **tarray, int i, int nrows, char **value);
static void copyChar60(CssDynamic60Class **tarray, int i, int nrows, char **value);

static void copyArray5(CssDynamic5Class **tarray, int i, int n, int siz, void *v);
static void copyArray10(CssDynamic10Class **tarray, int i, int n, int siz, void *v);
static void copyArray20(CssDynamic20Class **tarray, int i, int n, int siz, void *v);
static void copyArray40(CssDynamic40Class **tarray, int i, int n, int siz, void *v);
static void copyArray60(CssDynamic60Class **tarray, int i, int n, int siz, void *v);

static void copyDate5(CssDynamic5Class **tarray, int i, int nrows, void *value);
static void copyDate10(CssDynamic10Class **tarray, int i, int nrows, void *value);
static void copyDate20(CssDynamic20Class **tarray, int i, int nrows, void *value);
static void copyDate40(CssDynamic40Class **tarray, int i, int nrows, void *value);
static void copyDate60(CssDynamic60Class **tarray, int i, int nrows, void *value);

#define offset(a,field) ((unsigned int)(((const char *)&a->field) - ((const char *)a)))


/**
 * Retrieve the results of an arbitrary sql query. The query can request
 * individual columns and table joins.  The ODBC_Result structure is filled
 * with the number of columns, the column descriptions, the number of rows
 * and the values in each column.
 * @param hdbc A connection handle obtained from ODBCConnect().
 * @param query An arbitrary SQL query that does not have to return complete \
 *	CSS table rows, but can return combinations of columns and joins.
 * @table A Vector array to receive the CssTableClass dynamic objects (CssDynamic5Class, \
 *		CssDynamic10Class, CssDynamic20Class, CssDynamic40Class or CssDynamicClass).
 *
 * @return the number of columns in the result or -1 to indicate an error. \
 *	Retrieve the error number and error message with ODBCErrno() and \
 *      ODBCErrMsg().
 */
int
ODBCQuery(SQLHDBC hdbc, const string &query, gvector<CssTableClass *> &table)
{
	ODBC_Result result;
	const char *name;
	int i, j, k, n, m, source_q, user_q, passwd_q, table_q, format_q;
	int size, nrows, ncols;
	CssClassDescription *des;
	CssDynamic5Class **tarray5 = NULL;
	CssDynamic10Class **tarray10 = NULL;
	CssDynamic20Class **tarray20 = NULL;
	CssDynamic40Class **tarray40 = NULL;
	CssDynamic60Class **tarray60 = NULL;
	CssDynamic60Class *d;
	CssTableClass **ta = NULL;

	if(ODBCGeneralQuery(hdbc, query, &result)) {
	    if(result.column) free(result.column);
	    return -1;
	}
	if(result.numColumns <= 0) {
	    ODBCSetErrorMsg("ODBCQuery: No rows returned.", 0, 0);
	    return 0;
	}
	ncols = (result.numColumns <= 60) ? result.numColumns : 60;

	des = (CssClassDescription *)malloc(ncols*sizeof(CssClassDescription));
	if(des == NULL) {
	    ODBCSetErrorMsg("ODBCQuery malloc failed.", 0, 0);
	    return -1;
	}

	d = new CssDynamic60Class();

	nrows = result.numRows;

	m = 1;
	for(i = 0; i < ncols; i++)
	{
	    ODBC_ColumnResult *c = &result.column[i];

	    stringToLower((char *) c->columnName);
	    stringcpy(des[i].name, (char*)c->columnName, sizeof(des[i].name));
	    des[i].quark_offset = -1;
	    des[i].offset = offset(d, data[i]);
	    des[i].size = 0;

	    switch(c->dataType)
	    {
		case SQL_CHAR:
		case SQL_VARCHAR:
		    des[i].type = CSS_QUARK;
		    stringcpy(des[i].format, "%s", sizeof(des[i].format));
		    n = 0;
		    for(j = 0; j < nrows; j++) {
			k = (int)strlen(((char **)c->value)[j]);
			if(n < k) n = k;
		    }
		    des[i].size = n+1;
		    des[i].start = m;
		    des[i].end = m+n-1;
		    stringcpy(des[i].null_value, "-",sizeof(des[i].null_value));
		    m += n + 1;
		    break;
		case SQL_NUMERIC:
		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
		    des[i].type = CSS_DOUBLE;
		    stringcpy(des[i].format, "%10.2lf", sizeof(des[i].format));
		    des[i].size = sizeof(double);
		    des[i].start = m;
		    des[i].end = m+9;
		    stringcpy(des[i].null_value, "-999.",
				sizeof(des[i].null_value));
		    m += 11;
		    break;
		case SQL_BIGINT:
		case SQL_INTEGER:
		    des[i].size = sizeof(long);
		    des[i].type = CSS_LONG;
		    stringcpy(des[i].format, "%8ld", sizeof(des[i].format));
		    des[i].start = m;
		    des[i].end = m+7;
		    stringcpy(des[i].null_value,"-1",sizeof(des[i].null_value));
		    m += 9;
		    break;
		case SQL_DECIMAL:
		case SQL_SMALLINT:
		    des[i].size = sizeof(int);
		    des[i].type = CSS_INT;
		    stringcpy(des[i].format, "%8d", sizeof(des[i].format));
		    stringcpy(des[i].null_value,"-1",sizeof(des[i].null_value));
		    m += 9;
		    break;
		case SQL_TYPE_TIMESTAMP:
		    des[i].size = sizeof(int);
		    des[i].type = CSS_QUARK;
		    stringcpy(des[i].format, "%17s", sizeof(des[i].format));
		    stringcpy(des[i].null_value, "-",sizeof(des[i].null_value));
		    des[i].start = m;
		    des[i].end = m+16;
		    m += 18;
		    break;
	    }
	}

	delete d;

	if(ncols <= 5) {
	    CssTableClass::redefine("dynamic5", ncols, des, 0, NULL,
			CssDynamic5Class::createDynamic5, sizeof(CssDynamic5Class));
	    tarray5 = (CssDynamic5Class **)malloc(nrows*sizeof(CssDynamic5Class *));
	    ta = (CssTableClass **)tarray5;
	    name = "dynamic5";
	}
	else if(ncols <= 10) {
	    CssTableClass::redefine("dynamic10", ncols, des, 0, NULL,
			CssDynamic10Class::createDynamic10, sizeof(CssDynamic10Class));
	    tarray10 = (CssDynamic10Class **)malloc(nrows*sizeof(CssDynamic10Class *));
	    ta = (CssTableClass **)tarray10;
	    name = "dynamic10";
	}
	else if(ncols <= 20) {
	    CssTableClass::redefine("dynamic20", ncols, des, 0, NULL,
			CssDynamic20Class::createDynamic20, sizeof(CssDynamic20Class));
	    tarray20 = (CssDynamic20Class **)malloc(nrows*sizeof(CssDynamic20Class *));
	    ta = (CssTableClass **)tarray20;
	    name = "dynamic20";
	}
	else if(ncols <= 40) {
	    CssTableClass::redefine("dynamic40", ncols, des, 0, NULL,
			CssDynamic40Class::createDynamic40, sizeof(CssDynamic40Class));
	    tarray40 = (CssDynamic40Class **)malloc(nrows*sizeof(CssDynamic40Class *));
	    ta = (CssTableClass **)tarray40;
	    name = "dynamic40";
	}
	else {
	    CssTableClass::redefine("dynamic60", ncols, des, 0, NULL,
			CssDynamic60Class::createDynamic60, sizeof(CssDynamic60Class));
	    tarray60 = (CssDynamic60Class **)malloc(nrows*sizeof(CssDynamic60Class *));
	    ta = (CssTableClass **)tarray60;
	    name = "dynamic60";
	}
	if(ta == NULL) {
	    ODBCSetErrorMsg("ODBCQuery: malloc failed.", 0, 0);
	    free(des);
	    return -1;
	}

	for(i = 0; i < nrows; i++) ta[i] = CssTableClass::createCssTable(name);

	for(i = 0; i < ncols; i++)
	{
	    ODBC_ColumnResult *c = &result.column[i];

	    if(c->dataType == SQL_CHAR || c->dataType == SQL_VARCHAR)
	    {
		n = des[i].end - des[i].start + 2;
		if(tarray5) {
		    copyChar5(tarray5, i, nrows, (char **)c->value);
		}
		else if(tarray10) {
		    copyChar10(tarray10, i, nrows, (char **)c->value);
		}
		else if(tarray20) {
		    copyChar20(tarray20, i, nrows, (char **)c->value);
		}
		else if(tarray40) {
		    copyChar40(tarray40, i, nrows, (char **)c->value);
		}
		else if(tarray60) {
		    copyChar60(tarray60, i, nrows, (char **)c->value);
		}
		for(j = 0; j < nrows; j++) free(((char **)c->value)[j]);
	    }
	    else if(c->dataType == SQL_TYPE_TIMESTAMP)
	    {
		if(tarray5) copyDate5(tarray5, i, nrows, c->value);
		else if(tarray10) copyDate10(tarray10, i, nrows, c->value);
		else if(tarray20) copyDate20(tarray20, i, nrows, c->value);
		else if(tarray40) copyDate40(tarray40, i, nrows, c->value);
		else if(tarray60) copyDate60(tarray60, i, nrows, c->value);
	    }
 	    else
	    {
		if(c->dataType == SQL_NUMERIC || c->dataType == SQL_FLOAT ||
		   c->dataType == SQL_REAL || c->dataType == SQL_DOUBLE)
		{
		    size = sizeof(double);
		}
		else if(c->dataType == SQL_INTEGER || c->dataType == SQL_BIGINT)
		{
		    size = sizeof(long);
		}
		else if(c->dataType == SQL_SMALLINT || c->dataType == SQL_DECIMAL)
		{
		    size = sizeof(int);
		}
		else {
		    ODBCSetErrorMsg("ODBCQuery unknown dataType.", 0, 0);
		    free(des);
		    if(result.column) free(result.column);
		    return -1;
		}
		if(tarray5) copyArray5(tarray5, i, nrows, size, c->value);
		else if(tarray10) copyArray10(tarray10, i, nrows,size,c->value);
		else if(tarray20) copyArray20(tarray20, i, nrows,size,c->value);
		else if(tarray40) copyArray40(tarray40, i, nrows,size,c->value);
		else if(tarray60) copyArray60(tarray60, i, nrows,size,c->value);
	    }
	    if(c->value) free(c->value);
	}

	format_q = stringToQuark("odbc");
	source_q = stringToQuark(ODBCGetDataSource(hdbc));
	user_q = stringToQuark(ODBCGetUser(hdbc));
	passwd_q = stringToQuark(ODBCGetPassword(hdbc));
	table_q = stringToQuark("tmpTable");

	for(k = 0; k < nrows; k++)
	{
	    table.push_back(ta[k]);

	    ta[k]->setFormat(format_q);
	    ta[k]->setSource(source_q, user_q, passwd_q);
	    ta[k]->setAccount(user_q, table_q);
	}

	free(des);

	if(ta) free(ta);
	if(result.column) free(result.column);
	return result.numColumns;
}

/**
 * Free space allocated for a ODBC_Result structure.
 */
void
ODBCFreeResult(ODBC_Result *result)
{
	int i, j;

	for(i = 0; i < result->numColumns; i++)
	{
	    ODBC_ColumnResult *c = &result->column[i];
	    if(c->dataType == SQL_CHAR || c->dataType == SQL_VARCHAR)
	    {
		for(j = 0; j < result->numRows; j++) {
		    free(((char **)c->value)[j]);
		}
	    }
	    free(c->value);
	}
	free(result->column);
}

static void
copyChar5(CssDynamic5Class **tarray, int i, int nrows, char **value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    tarray[j]->data[i].i_data = stringToQuark(value[j]);
	}
}

static void
copyChar10(CssDynamic10Class **tarray, int i, int nrows, char **value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    tarray[j]->data[i].i_data = stringToQuark(value[j]);
	}
}

static void
copyChar20(CssDynamic20Class **tarray, int i, int nrows, char **value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    tarray[j]->data[i].i_data = stringToQuark(value[j]);
	}
}

static void
copyChar40(CssDynamic40Class **tarray, int i, int nrows, char **value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    tarray[j]->data[i].i_data = stringToQuark(value[j]);
	}
}

static void
copyChar60(CssDynamic60Class **tarray, int i, int nrows, char **value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    tarray[j]->data[i].i_data = stringToQuark(value[j]);
	}
}

static void
copyArray5(CssDynamic5Class **tarray, int i, int nrows, int size, void *value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    memcpy(&tarray[j]->data[i], (char *)value+j*size, size);
	}
}

static void
copyArray10(CssDynamic10Class **tarray, int i, int nrows, int size, void *value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    memcpy(&tarray[j]->data[i], (char *)value+j*size, size);
	}
}

static void
copyArray20(CssDynamic20Class **tarray, int i, int nrows, int size, void *value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    memcpy(&tarray[j]->data[i], (char *)value+j*size, size);
	}
}

static void
copyArray40(CssDynamic40Class **tarray, int i, int nrows, int size, void *value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    memcpy(&tarray[j]->data[i], (char *)value+j*size, size);
	}
}

static void
copyArray60(CssDynamic60Class **tarray, int i, int nrows, int size, void *value)
{
	int j;
	for(j = 0; j < nrows; j++) {
	    memcpy(&tarray[j]->data[i], (char *)value+j*size, size);
	}
}

static void
copyDate5(CssDynamic5Class **tarray, int i, int nrows, void *value)
{
	int j, yr;
	char buf[18];
	SQL_TIMESTAMP_STRUCT *time, *ts;

	time = (SQL_TIMESTAMP_STRUCT *)value;
	for(j = 0; j < nrows; j++)
	{
	    ts = &time[j];
	    yr = (ts->year >= 100) ?  ts->year - (ts->year/100)*100 : ts->year;
	    snprintf(buf, 18, "%02d/%02d/%02d %02d:%02d:%02d",
		yr, ts->month, ts->day, ts->hour, ts->minute, ts->second);
	    tarray[j]->data[i].i_data = stringToQuark(buf);
	}
}

static void
copyDate10(CssDynamic10Class **tarray, int i, int nrows, void *value)
{
	int j, yr;
	char buf[18];
	SQL_TIMESTAMP_STRUCT *time, *ts;

	time = (SQL_TIMESTAMP_STRUCT *)value;
	for(j = 0; j < nrows; j++)
	{
	    ts = &time[j];
	    yr = (ts->year >= 100) ?  ts->year - (ts->year/100)*100 : ts->year;
	    snprintf(buf, 18, "%02d/%02d/%02d %02d:%02d:%02d",
		yr, ts->month, ts->day, ts->hour, ts->minute, ts->second);
	    tarray[j]->data[i].i_data = stringToQuark(buf);
	}
}

static void
copyDate20(CssDynamic20Class **tarray, int i, int nrows, void *value)
{
	int j, yr;
	char buf[18];
	SQL_TIMESTAMP_STRUCT *time, *ts;

	time = (SQL_TIMESTAMP_STRUCT *)value;
	for(j = 0; j < nrows; j++)
	{
	    ts = &time[j];
	    yr = (ts->year >= 100) ?  ts->year - (ts->year/100)*100 : ts->year;
	    snprintf(buf, 18, "%02d/%02d/%02d %02d:%02d:%02d",
		yr, ts->month, ts->day, ts->hour, ts->minute, ts->second);
	    tarray[j]->data[i].i_data = stringToQuark(buf);
	}
}

static void
copyDate40(CssDynamic40Class **tarray, int i, int nrows, void *value)
{
	int j, yr;
	char buf[18];
	SQL_TIMESTAMP_STRUCT *time, *ts;

	time = (SQL_TIMESTAMP_STRUCT *)value;
	for(j = 0; j < nrows; j++)
	{
	    ts = &time[j];
	    yr = (ts->year >= 100) ?  ts->year - (ts->year/100)*100 : ts->year;
	    snprintf(buf, 18, "%02d/%02d/%02d %02d:%02d:%02d",
		yr, ts->month, ts->day, ts->hour, ts->minute, ts->second);
	    tarray[j]->data[i].i_data = stringToQuark(buf);
	}
}

static void
copyDate60(CssDynamic60Class **tarray, int i, int nrows, void *value)
{
	int j, yr;
	char buf[18];
	SQL_TIMESTAMP_STRUCT *time, *ts;

	time = (SQL_TIMESTAMP_STRUCT *)value;
	for(j = 0; j < nrows; j++)
	{
	    ts = &time[j];
	    yr = (ts->year >= 100) ?  ts->year - (ts->year/100)*100 : ts->year;
	    snprintf(buf, 18, "%02d/%02d/%02d %02d:%02d:%02d",
		yr, ts->month, ts->day, ts->hour, ts->minute, ts->second);
	    tarray[j]->data[i].i_data = stringToQuark(buf);
	}
}

/**
 * Retrieve the results of an arbitrary sql query. The query can request
 * individual columns and table joins.  The ODBC_Result structure is filled
 * with the number of columns, the column descriptions, the number of rows
 * and the values in each column.
 * @param hdbc A connection handle obtained from ODBCConnect().
 * @param query An arbitrary SQL query that does not have to return complete \
 *	CSS table rows, but can return combinations of columns and joins.
 * @param result An ODBC_Result structure to receive the results.
 * @return 0 for success or -1 to indicate an error. \
 *	Retrieve the error number and error message with ODBCErrno() and \
 *      ODBCErrMsg().
 */
int
ODBCGeneralQuery(SQLHDBC hdbc, const string &query, ODBC_Result *result)
{
	int i;
	SQLHSTMT hstmt;
	SQLRETURN ret;

	result->column = NULL;

	ret = ODBCAllocStmt(hdbc, &hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCGeneralQuery", SQL_HANDLE_DBC, hdbc);
	    return -1;
	}

	/* Execute the query */
	ret = SQLExecDirect(hstmt, (unsigned char *) query.c_str(), SQL_NTS);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCGeneralQuery", SQL_HANDLE_STMT, hstmt);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCGeneralQuery: SQLFreeHandle failed.");
	    }
	    return -1;
	}

	/* Get the column descriptions of the rows that are returned. */
	if(ODBCDescribeCols(hstmt, &result->numColumns, &result->column) < 0)
	{
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCGeneralQuery: SQLFreeHandle failed.");
	    }
	    return -1;
	}

	/* Using the column data types, copy the results into the appropriate
	 * variables.
	 */
	ret = ODBCGetResultCols(hstmt, result->numColumns, result->column);

	/* Check for consistent results */
	if( !ret )
	{
	    if(result->numColumns > 0) {
		result->numRows = result->column[0].numRows;
	    }
	    for(i = 1; i < result->numColumns && 
		result->column[i].numRows == result->column[0].numRows; i++);
	    if(i < result->numColumns) {
		ODBCSetErrorMsg("ODBCGeneralQuery: column lengths vary.",0,0);
		ret = -1;
	    }
	}

	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	    logErrorMsg(LOG_WARNING, "ODBCGeneralQuery: SQLFreeHandle failed.");
	}

	return ret;
}
#endif /* HAVE_LIBODBC */ 
