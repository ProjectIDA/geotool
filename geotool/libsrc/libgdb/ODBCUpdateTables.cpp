/* ODBCUpdateTables.c
 *
 *  ODBCUpdateTable()
 *  ODBCUpdateTables()
 *  ODBCUpdateTableWhere()
 *  ODBCUpdateTablesWhere()
 *  ODBCCountTable()
 *  ODBCCountTables()
 *  ODBCDeleteTable()
 *  ODBCDeleteTables()
 *  ODBCDeleteTableWhere()
 *  ODBCDeleteTablesWhere()
 */
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libgdb.h"
#include "libstring.h"
#include "gobject++/CssTableClass.h"

#ifdef HAVE_LIBODBC

/**
 *  These routines update specific members (columns) of database tables.
 *  Four routines, ODBCUpdateTable(), ODBCUpdateTables(),
 *  ODBCUpdateTableWhere() and ODBCUpdateTablesWhere() update one or more
 *  table records. Two other routines count records, ODBCCountTable() and
 *  ODBCCountTables(). And there are four routines that delete records from
 *  the database, ODBCDeleteTable(), ODBCDeleteTables(), ODBCDeleteTableWhere(),
 *  and ODBCDeleteTablesWhere().
 *  <p>
 *  All data is transfered to the ODBC server by binding to memory locations.
 *  This includes data values inserted into the table and also data values
 *  used in the "where clause". (Queries are formed using the '?' character
 *  to denote bound parameters.) Columns of type CSS_LDDATE are automatically
 *  updated using name=CURRENT_TIMESTAMP.
 *  <p>
 *  SQL "where clauses" are constructed using members of the original records.
 *  Some routines allow the specification of which members to use in the where
 *  clause and other routines automatically use all members, except those of
 *  type CSS_LDDATE. Routines that use all table members in the where clause
 *  use ODBCGetWhereMembers() to order the members. Id members (arid,orid, etc)
 *  are place first followed by all other integer members, all string members
 *  and then all float and double members. An SQL between operator is used
 *  to specify a small range for the float and double members. For float
 *  members the between interval is (f +/- 1.e-06*fabs(f)). For double
 *  members the interval is (d +/- 1.e-10*fabs(d)).
 * <p>
 * The following example shows the use of ODBCUpdateTableWhere to update the
 * depth member of the origin table. The orid and evid are included in the
 * where clause.
 * </p>
 * <pre>
 * #include "libgdb.h"
 * 
 * int
 * main(int argc, char **argv)
 * {
 *     int ret, autoCommit=1;
 *     int where[2], memberIndices[1];
 *     char *dataSource="ORACLE", *user="sel3", *passwd="sel3";
 *     SQLHDBC hdbc;
 *     CssOrigin old, new;
 * 
 *     /\* Connect to the data source
 *      *\/
 *     if((hdbc = ODBCConnect(dataSource, user, passwd, autoCommit)) == NULL) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 * 
 *     /\* Create a CssOrigin structure with null values
 *      *\/
 *     old = (CssOrigin)new_CssTableClass("origin");
 * 
 *     /\* Set the orid and evid values.
 *      *\/
 *     old->orid = 296229;
 *     old->evid = 296229;
 * 
 *     /\* Create another CssOrigin structure.
 *      *\/
 *     new = (CssOrigin)new_CssTableClass("origin");
 * 
 *     /\* Set the depth value.
 *      *\/
 *     new->depth = 50.;
 * 
 *     /\* Set the "where" indices.
 *      *\/
 *     where[0] = CssTable_memberIndex(old, "orid");
 *     where[1] = CssTable_memberIndex(old, "evid");
 * 
 *     /\* Set the indices of the members that will be updated.
 *      *\/
 *     memberIndices[0] = CssTable_memberIndex(old, "depth");
 * 
 *     /\* Update the table. The effect of this call is the same as the SQL
 *      * command "update sel3.origin set depth=50.,lddate=CURRENT_TIMESTAMP
 *      * where orid=296229 and evid=296229".
 *      *\/
 *     ret = ODBCUpdateTableWhere(hdbc, "sel3.origin", 1, memberIndices, 2,
 *                         where, (CssTableClass)old, (CssTableClass)new);
 * 
 *     if(ret !=  0) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *     }
 * 
 *     ODBCDisconnect(hdbc);
 * 
 *     /\* Free tables allocated.
 *      *\/
 *     GObject_free((GObject)old);
 *     GObject_free((GObject)new);
 * }
 * </pre>
 * A second example below contains some lines that demonstrate the use of
 * ODBCUpdateTable. Notice that it requires neither "where" indices nor
 * memberIndices. It will use all members (except CssDate members) in the
 * where clause and it will update all members of table2 that are different
 * from table1. ODBCUpdateTable is convenient when you are updating a table
 * row that was recently obtained from the database.
 * </p>
 * <pre>
 *
 *     SQLHDBC hdnc;
 *     char query[1000];
 *     CssOrigin o1, o2;
 *     Vector v;
 *     ...
 *
 *     v = new_Vector();
 *
 *     strcpy(query, "select * from sel3.origin where orid=296229");
 *
 *     ret = ODBCQueryTable(hdbc, query, "origin", v);
 *
 *     if(ret !=  0) {
 *         logErrorMsg(LOG_WARNING, ODBCErrMsg());
 *         exit(1);
 *     }
 *     if(v->elementCount == 0)
 *     {
 *         logErrorMsg(LOG_WARNING, "No records returned.");
 *         exit(0);
 *     }
 *
 *     o1 = (CssOrigin)v->elementData[0];
 *
 *     /\* make a copy
 *      *\/
 *     o2 = (CssOrigin)GObject_clone((GObject)o1);
 *
 *    /\* change some members
 *     *\/
 *     o2.lat = 46.7;
 *     o2.depth = 10.3;
 *     o2.evid = 44363;
 *
 *     /\* update the table. All members of o1 will be used in the where clause.
 *      * All members of o2 that differ from o1 (lat, depth and evid) will be
 *      * updated.
 *      *\/
 *     ODBCUpdateTable(hdbc, "sel3.origin", o1, o2);
 *
 *     ODBCDisconnect(hdbc);
 *
 *     GObject_free((GObject)v);
 *     GObject_free((GObject)o2);
 * </pre>
 */

#define COUNT 	1
#define DELETE	2

/**
 * @private
 */
/* ET: The datatype of sql_Long was changed from SQLINTEGER to long in the course of the 
   64 bit migration to eliminate problems related to the fact that 
   sizeof(long) > sizeof(SQLINTEGER) on 64 bit.
 */
typedef struct
{
	SQLLEN	length;
	SQLLEN	length2;
	SQLDOUBLE	sql_Begin;
	SQLDOUBLE	sql_End;
	union {
	    char		*sql_String;
	    SQLDOUBLE		sql_Double;
	    long		sql_Long;
	    SQL_DATE_STRUCT	sql_Date;
        } bindTarget;
} BindParam;

static int countOrDelete(SQLHDBC hdbc, const char *tableName, int numWhere,
		int *whereIndices, int num, CssTableClass **tables,
		int count_or_delete, int *count);
static int bindTargets(SQLHSTMT hstmt, int nb, int *bindIndices, BindParam *bp,
		CssClassDescription *des, int *bindNumber, int use_between);
static int copyMembers(int nb, int *bindIndices, BindParam *bp,
		CssClassDescription *des, CssTableClass *t, int use_between);
static void freeBP(int nb, BindParam *bp, CssClassDescription *des,int *bindIndices);

/**
 * Update num database records. Update each member (column) of each record in
 * new_tables that differs from the corresponding member and record in
 * old_tables. Columns of type CSS_LDDATE are automatically updated using
 * name=CURRENT_TIMESTAMP. All of the members of the old_tables records
 * (except CSS_LDDATE members) are used in the where clause.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param num The number of records to update.
 * @param old_tables The original CssTableClass objects whose elements will be used \
 *		in the where clause.
 * @param new_tables The CssTableClass objects with new values.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCUpdateTables(SQLHDBC hdbc, const string &tableName, int num,
		CssTableClass **old_tables, CssTableClass **new_tables)
{
	int i;

	for(i = 0; i < num; i++) {
	    if(ODBCUpdateTable(hdbc, tableName, old_tables[i], new_tables[i]))
		return -1;
	}
	return 0;
}

/**
 * Update one database record. Update each member (column) of old_table
 * that differs from the corresponding member in new_table. Columns of type
 * CSS_LDDATE are automatically updated using name=CURRENT_TIMESTAMP. All of the
 * members of old_table (except CSS_LDDATE members) are used in the where clause.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param old_table The original CssTableClass object whose elements will be used \
 *		in the where clause.
 * @param new_table The CssTableClass object with new values.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCUpdateTable(SQLHDBC hdbc, const string &tableName, CssTableClass *old_table,
		CssTableClass *new_table)
{
	int i, num_members, num_indices, indices[100], num_where, where[100];
	CssClassDescription *des;

	/* find members that have been changed.
	 */
	num_members = old_table->getNumMembers();
	des = old_table->description();

	num_indices = 0;
	for(i = 0; i < num_members; i++)
	{
	    char *member1 = (char *)old_table + des[i].offset;
	    char *member2 = (char *)new_table + des[i].offset;

	    switch(des[i].type)
            {
		case CSS_STRING:
		    if(strcmp(member1, member2)) indices[num_indices++] = i;
		    break;
		case CSS_JDATE:
		case CSS_LONG:
		    if(*(long *)member1 != *(long *)member2)
			indices[num_indices++] = i;
		    break;
		case CSS_INT:
		    if(*(int *)member1 != *(int *)member2)
			indices[num_indices++] = i;
		    break;
		case CSS_DOUBLE:
		    if(*(double *)member1 != *(double *)member2)
			indices[num_indices++] = i;
		    break;
		case CSS_FLOAT:
		    if(*(float *)member1 != *(float *)member2)
			indices[num_indices++] = i;
		    break;
		case CSS_TIME:
		    if(*(double *)member1 != *(double *)member2)
			indices[num_indices++] = i;
		    break;
	    }
	}
	if(num_indices == 0) return 0;

	num_where = ODBCGetWhereMembers(old_table, where);

	return ODBCUpdateTableWhere(hdbc, tableName, num_indices, indices,
			num_where, where, old_table, new_table);
}

/**
 * Update one database record. The members to update are specified by
 * the memberIndices array. (Columns of type CSS_LDDATE are automatically
 * updated using name=CURRENT_TIMESTAMP.) The members to use in the where
 * clause are specified by the whereIndices array. The updated values are
 * taken from the new_table object and the where values are taken from the
 * old_table object.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param numMembers The number of members of the new_table object to update.
 * @param memberIndices The indices of the members in the CssTableClass structure.
 * @param numWhere The number of members to use in the where clause.
 * @param whereIndices The indices of the where members in the CssTableClass \
 *		structure.
 * @param old_table The original CssTableClass object whose members will be used \
 *		in the where clause.
 * @param new_table The CssTableClass object with new values.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCUpdateTableWhere(SQLHDBC hdbc, const string &tableName, int numMembers,
		int *memberIndices, int numWhere, int *whereIndices,
		CssTableClass *old_table, CssTableClass *new_table)
{
	return ODBCUpdateTablesWhere(hdbc, tableName, numMembers, memberIndices,
			numWhere, whereIndices, 1, &old_table, &new_table);
}

/**
 * Update num database records. The members to update are specified by
 * the memberIndices array. (Columns of type CSS_LDDATE are automatically
 * updated using name=CURRENT_TIMESTAMP.) The members to use in the where
 * clause are specified by the whereIndices array. The updated values are
 * taken from the new_tables objects and the where values are taken from the
 * old_tables objects.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param numMembers The number of members of each record to update.
 * @param memberIndices The indices of the members in the CssTableClass structure.
 * @param numWhere The number of members to use in the where clause.
 * @param whereIndices The indices of the where members in the CssTableClass \
 *		structure.
 * @param num The number of records to update.
 * @param old_tables The original CssTableClass objects whose members will be used \
 *		in the where clause.
 * @param new_tables The CssTableClass objects with new values.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCUpdateTablesWhere(SQLHDBC hdbc, const string &tableName, int numMembers,
		int *memberIndices, int numWhere, int *whereIndices,
		int num, CssTableClass **old_tables, CssTableClass **new_tables)
{
	SQLRETURN ret;
	SQLHSTMT hstmt;
	char update[5000];
	int i, j, k, numColumns, *bind_old, nb_old, *bind_new, nb_new;
	BindParam *bp_old, *bp_new;
	CssClassDescription *des;

	if(num <= 0 || numMembers <= 0) return 0;

	des = old_tables[0]->description();
	numColumns = old_tables[0]->getNumMembers();

	if(numMembers > numColumns) {
	    ODBCSetErrorMsg("ODBCUpdateTablesWhere: numMembers > numColumns.",
			0, 0);
	    return -1;
	}
	if(numWhere > numColumns) {
	    ODBCSetErrorMsg(
		"ODBCUpdateTablesWhere error: numWhere > numColumns.", 0, 0);
	    return -1;
	}

	if((bind_old = (int *)malloc(numColumns*sizeof(int))) == NULL) {
	    ODBCSetErrorMsg("ODBCUpdateTablesWhere: malloc failed.", 0, 0);
	    return -1;
	}
	if((bind_new = (int *)malloc(numColumns*sizeof(int))) == NULL) {
	    ODBCSetErrorMsg("ODBCUpdateTablesWhere: malloc failed.", 0, 0);
	    return -1;
	}

	snprintf(update, 5000, "update %s set ", tableName.c_str());
	for(i = j = 0; i < numColumns; i++) {
	    if(des[i].type == CSS_LDDATE) {
		if(j > 0) strcat(update, ",");
		strcat(update, des[i].name);
		strcat(update, "=CURRENT_TIMESTAMP");
		j++;
	    }
	}
	nb_new = 0;
	for(i = 0; i < numMembers; i++) {
	    k = memberIndices[i];
	    if(k >= 0 && k < numColumns && des[k].type != CSS_LDDATE) {
		if(j > 0) strcat(update, ",");
		strcat(update, des[k].name);
		strcat(update, "=?");
		bind_new[nb_new++] = k;
		j++;
	    }
	}
	nb_old = 0;
	for(i = j = 0; i < numWhere; i++) {
	    k = whereIndices[i];
	    if(k >= 0 && k < numColumns && des[k].type != CSS_LDDATE) {
		if(j == 0) strcat(update, " where ");
		else strcat(update, " and ");
		strcat(update, des[k].name);
		if(des[k].type == CSS_TIME || des[k].type == CSS_DOUBLE ||
			des[k].type == CSS_FLOAT)
		{
		    strcat(update, " between ? and ?");
		}
		else {
		    strcat(update, "=?");
		}
		bind_old[nb_old++] = k;
		j++;
	    }
	}

	ret = ODBCAllocStmt(hdbc, &hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCUpdateTables", SQL_HANDLE_DBC, hdbc);
	    free(bind_old); free(bind_new);
	    return -1;
	}

	ret = SQLPrepare(hstmt, (unsigned char *) update, SQL_NTS);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCUpdateTables", SQL_HANDLE_DBC, hdbc);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	     logErrorMsg(LOG_WARNING,"ODBCUpdateTables: SQLFreeHandle failed.");
	    }
	    free(bind_old); free(bind_new);
	    return -1;
	}
	if((bp_old = (BindParam *)malloc(numColumns*sizeof(BindParam))) == NULL
	|| (bp_new = (BindParam *)malloc(numColumns*sizeof(BindParam))) == NULL)
	{
	    ODBCSetErrorMsg("ODBCUpdateTables malloc error.", 0, 0);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	     logErrorMsg(LOG_WARNING,"ODBCUpdateTables: SQLFreeHandle failed.");
	    }
	    return -1;
	}

	i = 1;
	ret = bindTargets(hstmt, nb_new, bind_new, bp_new, des, &i, 0);

	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	     logErrorMsg(LOG_WARNING,"ODBCUpdateTables: SQLFreeHandle failed.");
	    }
	    freeBP(nb_new, bp_new, des, bind_new);
	    free(bind_old); free(bind_new);
	    return -1;
	}

	ret = bindTargets(hstmt, nb_old, bind_old, bp_old, des, &i, 1);

	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	     logErrorMsg(LOG_WARNING,"ODBCUpdateTables: SQLFreeHandle failed.");
	    }
	    freeBP(nb_old, bp_old, des, bind_old);
	    freeBP(nb_new, bp_new, des, bind_new);
	    free(bind_old); free(bind_new);
	    return -1;
	}

	for(i = 0; i < num; i++)
	{
	    copyMembers(nb_old, bind_old, bp_old, des, old_tables[i], 1);
	    copyMembers(nb_new, bind_new, bp_new, des, new_tables[i], 0);

	    ret = SQLExecute(hstmt);

	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	    {
		ODBCSetErrorMsg("ODBCUpdateTables", SQL_HANDLE_STMT, hstmt);
		break;
	    }
	    if(SQLFreeStmt(hstmt, SQL_CLOSE) != SQL_SUCCESS) {
	       logErrorMsg(LOG_WARNING,"ODBCUpdateTables: SQLFreeStmt failed.");
		break;
	    }
	}
	ret = (i == num) ? 0 : -1;

	if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	    logErrorMsg(LOG_WARNING,"ODBCUpdateTables: SQLFreeHandle failed.");
	}

	freeBP(nb_old, bp_old, des, bind_old);
	freeBP(nb_new, bp_new, des, bind_new);
	free(bind_old); free(bind_new);

	return ret;
}

/**
 * Count database records. Count the number of records that partially match the
 * input record. The table members to compare are specified by the whereIndices.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param numWhere The number of members to use in the where clause.
 * @param whereIndices The indices of the where members in the CssTableClass \
 *		structure.
 * @param table The CssTableClass object whose members will be used the \
		where clause.
 * @param count The number of records that match.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCCountTable(SQLHDBC hdbc, const string &tableName, int numWhere,
		int *whereIndices, CssTableClass *table, int *count)
{
	return ODBCCountTables(hdbc, tableName, numWhere, whereIndices,
			1, &table, count);
}

/**
 * Count database records. Count the number of records that partially match the
 * input records. The table members to compare are specified by the
 * whereIndices.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param numWhere The number of members to use in the where clause.
 * @param whereIndices The indices of the where members in the CssTableClass \
 *		structure.
 * @param num The number of different records to use in the where clause.
 * @param tables The CssTableClass objects whose members will be used the \
		where clause.
 * @param count The number of records that match.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCCountTables(SQLHDBC hdbc, const string &tableName, int numWhere,
		int *whereIndices, int num, CssTableClass **tables, int *count)
{
	return countOrDelete(hdbc, tableName.c_str(), numWhere, whereIndices,
				num, tables, COUNT, count);
}

/**
 * Delete a database table record. All of the members (except type CSS_LDDATE)
 * of input CssTableClass are used in the where clause.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param table The CssTableClass object whose members will be used the \
		where clause.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCDeleteTable(SQLHDBC hdbc, const string &tableName, CssTableClass *table)
{
	int num_where, where[100];

	num_where = ODBCGetWhereMembers(table, where);

	return ODBCDeleteTableWhere(hdbc, tableName, num_where, where, table);
}

/**
 * Delete database table records. All of the members (except type CSS_LDDATE)
 * of input CssTableClasss are used in the where clause.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param num The number of table records to delete.
 * @param tables The CssTableClass objects whose members will be used the \
		where clause.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCDeleteTables(SQLHDBC hdbc, const char *tableName, int num, CssTableClass **tables)
{
	int num_where, where[100];

	if(num <= 0) return 0;

	num_where = ODBCGetWhereMembers(tables[0], where);

	return ODBCDeleteTablesWhere(hdbc, tableName, num_where, where,
			num, tables);
}

/**
 * Delete a database table record. The members that will be used in the
 * where clause are specified by the whereIndices array.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param numWhere The number of members to use in the where clause.
 * @param whereIndices The indices of the members to use in the where clause.
 * @param table The CssTableClass object whose members will be used the \
		where clause.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCDeleteTableWhere(SQLHDBC hdbc, const string &tableName, int numWhere,
			int *whereIndices, CssTableClass *table)
{
	return ODBCDeleteTablesWhere(hdbc, tableName, numWhere, whereIndices,
			1, &table);
}

/**
 * Delete database table records. The members that will be used in the
 * where clause are specified by the whereIndices array.
 * @param hdbc The ODBC connection handle.
 * @param tableName The database table name, with an account prefix if needed.
 * @param numWhere The number of members to use in the where clause.
 * @param whereIndices The indices of the members to use in the where clause.
 * @param num The number of table records to delete.
 * @param tables The CssTableClass objects whose members will be used the \
		where clause.
 * @return 0 for success,  -1 for failure.  Retrieve the error number and the \
 * error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCDeleteTablesWhere(SQLHDBC hdbc, const string &tableName, int numWhere,
		int *whereIndices, int num, CssTableClass **tables)
{
	int count;

	return countOrDelete(hdbc, tableName.c_str(), numWhere, whereIndices,
				num, tables, DELETE, &count);
}

/**
 *  Count or delete records that match the where clause.
 */
static int
countOrDelete(SQLHDBC hdbc, const char *tableName, int numWhere,
		int *whereIndices, int num, CssTableClass **tables,
		int count_or_delete, int *count)
{
	SQLRETURN ret;
	SQLHSTMT hstmt;
	char delete_index[5000];
	int i, j, k, numColumns, *bind, nb;
	BindParam *bp;
	CssClassDescription *des;

	if(num <= 0 || numWhere <= 0) return 0;

	des = tables[0]->description();
	numColumns = tables[0]->getNumMembers();

	if(numWhere > numColumns) {
	   ODBCSetErrorMsg("ODBCUpdateTablesWhere: numWhere > numColumns.",0,0);
	    return -1;
	}

	if((bind = (int *)malloc(numColumns*sizeof(int))) == NULL) {
	    ODBCSetErrorMsg("ODBCDeleteTables malloc error.", 0, 0);
	    return -1;
	}

	if(count_or_delete == DELETE) {
	    snprintf(delete_index, 5000, "delete from %s where ", tableName);
	}
	else {
	    snprintf(delete_index, 5000, "select * from %s where ", tableName);
	}

	nb = 0;
	for(i = j = 0; i < numWhere; i++) {
	    k = whereIndices[i];
	    if(k >= 0 && k < numColumns && des[k].type != CSS_LDDATE) {
		if(j > 0) strcat(delete_index, " and ");
		strcat(delete_index, des[k].name);
		if(des[k].type == CSS_TIME || des[k].type == CSS_DOUBLE ||
			des[k].type == CSS_FLOAT)
		{
		    strcat(delete_index, " between ? and ?");
		}
		else {
		    strcat(delete_index, "=?");
		}
		bind[nb++] = k;
		j++;
	    }
	}

	if((bp = (BindParam *)malloc(numColumns*sizeof(BindParam))) == NULL)
	{
	    ODBCSetErrorMsg("ODBCDeleteTables malloc error.", 0, 0);
	    return -1;
	}

	*count = 0;
	for(i = 0; i < num; i++)
	{
	    ret = ODBCAllocStmt(hdbc, &hstmt);
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		ODBCSetErrorMsg("ODBCDeleteTables", SQL_HANDLE_DBC, hdbc);
		free(bind);
		return -1;
	    }

	    ret = SQLPrepare(hstmt, (unsigned char *) delete_index, SQL_NTS);
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		ODBCSetErrorMsg("ODBCDeleteTables", SQL_HANDLE_DBC, hdbc);
		if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		    logErrorMsg(LOG_WARNING,
				"ODBCDeleteTables: SQLFreeHandle failed.");
		}
		free(bind);
		return -1;
	    }

	    j = 1;
	    ret = bindTargets(hstmt, nb, bind, bp, des, &j, 1);

	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		    logErrorMsg(LOG_WARNING,
			"ODBCDeleteTables: SQLFreeHandle failed.");
		}
		freeBP(nb, bp, des, bind);
		free(bind);
		return -1;
	    }

	    copyMembers(nb, bind, bp, des, tables[i], 1);

	    ret = SQLExecute(hstmt);

	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	    {
		ODBCSetErrorMsg("ODBCDeleteTables", SQL_HANDLE_STMT, hstmt);
		break;
	    }
	    if(count_or_delete == COUNT) {
		while((ret = SQLFetch(hstmt)) == SQL_SUCCESS
			|| ret == SQL_SUCCESS_WITH_INFO) (*count)++;
		if(ret != SQL_NO_DATA) break;
	    }
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
			"ODBCDeleteTables: SQLFreeHandle failed.");
		break;
	    }
	    hstmt = NULL;
	}
	if(hstmt) {
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
		logErrorMsg(LOG_WARNING,
		    "ODBCDeleteTables: SQLFreeHandle failed.");
	    }
	}

	ret = (i == num) ? 0 : -1;

	freeBP(nb, bp, des, bind);
	free(bind);

	return ret;
}

/**
 * Free memory allocated as bind targets.
 */
static void
freeBP(int nb, BindParam *bp, CssClassDescription *des, int *bindIndices)
{
	int i, k;

	for(i = 0; i < nb; i++) {
	    k = bindIndices[i];
	    if(des[k].type == CSS_STRING) free(bp[k].bindTarget.sql_String);
	}
	free(bp);
}

/**
 * Bind memory parameter locations for the ODBC routines. When SQLExecute()
 * is called, the values in the update or delete statement will be obtained from
 * the bp[].bindTarget.
 */
static int
bindTargets(SQLHSTMT hstmt, int nb, int *bindIndices, BindParam *bp,
		CssClassDescription *des, int *bindNumber, int use_between)
{
	int i, k, len;
	SQLRETURN ret;

	for(i = 0; i < nb; i++)
	{
	    k = bindIndices[i];
	    bp[k].bindTarget.sql_String = NULL;

	    switch(des[k].type)
	    {
		case CSS_STRING:
		    len = des[k].end - des[k].start + 1;
		    if((bp[k].bindTarget.sql_String = (char *)malloc(len+1))
				== NULL)
		    {
			ODBCSetErrorMsg("bindTargets malloc error.", 0, 0);
			return -1;
		    }
		    ret = SQLBindParameter(hstmt, *bindNumber, SQL_PARAM_INPUT,
				SQL_C_CHAR, SQL_CHAR, len, 0,
				bp[k].bindTarget.sql_String, len+1,
				&bp[k].length);
		    bp[k].length = SQL_NTS;
		    (*bindNumber)++;
		    break;
		case CSS_DOUBLE:
		case CSS_TIME:
		case CSS_FLOAT:
		    if(use_between) {
			ret = SQLBindParameter(hstmt, *bindNumber,
				SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
				&bp[k].sql_Begin, 0, &bp[k].length);
			bp[k].length = 0;
			(*bindNumber)++;
			ret = SQLBindParameter(hstmt, *bindNumber,
				SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
				&bp[k].sql_End, 0, &bp[k].length2);
			bp[k].length2 = 0;
			(*bindNumber)++;
		    }
		    else {
			ret = SQLBindParameter(hstmt, *bindNumber,
				SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
				&bp[k].bindTarget.sql_Double, 0, &bp[k].length);
			bp[k].length = 0;
			(*bindNumber)++;
		    }
		    break;
		case CSS_INT:
		case CSS_JDATE:
		case CSS_LONG:
		  /* ET: The following line was changed in the course of 64 bit 
		     migration to solve problems resulting from different sizes of 
		     long and SQLInteger on 64 bit.    
		  
		     ret = SQLBindParameter(hstmt, *bindNumber, SQL_PARAM_INPUT,
		     SQL_C_LONG, SQL_INTEGER, 0, 0,
		     &bp[k].bindTarget.sql_Long, 0, &bp[k].length);
		  */
		  ret = SQLBindParameter(hstmt, *bindNumber, SQL_PARAM_INPUT,
				SQL_C_SLONG, SQL_BIGINT, 0, 0,
				&bp[k].bindTarget.sql_Long, 0, &bp[k].length);
		  bp[k].length = 0;
		  (*bindNumber)++;
		    break;
		case CSS_DATE:
		    ret = SQLBindParameter(hstmt, *bindNumber, SQL_PARAM_INPUT,
				SQL_C_TYPE_DATE, SQL_TYPE_DATE, 0, 0,
				&bp[k].bindTarget.sql_Date, 0, &bp[k].length);
		    bp[k].length = 0;
		    (*bindNumber)++;
		    break;
		default:
		    ret = SQL_SUCCESS; /* skip unknown type */
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
 * (bd[i].bindTarget).
 */
static int
copyMembers(int nb, int *bindIndices, BindParam *bp, CssClassDescription *des,
		CssTableClass *t, int use_between)
{
	char *member;
	int i, k;
	double fac;
	SQL_DATE_STRUCT *ts;
	DateTime *dt;

	for(i = 0; i < nb; i++)
	{
	    k = bindIndices[i];
	    member = (char *)t + des[k].offset;
	    switch(des[k].type)
	    {
		case CSS_STRING:
		    strcpy(bp[k].bindTarget.sql_String, member);
		    break;

		case CSS_DOUBLE:
		case CSS_TIME:
		case CSS_FLOAT:
		    if(des[k].type == CSS_DOUBLE || des[k].type == CSS_TIME) {
			bp[k].bindTarget.sql_Double = *(double *)member;
		    }
		    else {
			bp[k].bindTarget.sql_Double = *(float *)member;
		    }
		    fac = (des[k].type == CSS_TIME) ? 1.e-10 : 1.e-6;
		    if(use_between)
		    {
			SQLDOUBLE d = bp[k].bindTarget.sql_Double;
			if(d != 0.) {
			    bp[k].sql_Begin = d - fac*fabs(d);
			    bp[k].sql_End = d + fac*fabs(d);
			}
			else {
			    bp[k].sql_Begin = - fac;
			    bp[k].sql_End = fac;
			}
		    }
		    break;
		case CSS_INT:
		    bp[k].bindTarget.sql_Long = *(int *)member;
		    break;
		case CSS_JDATE:
		case CSS_LONG:
		    bp[k].bindTarget.sql_Long = *(long *)member;
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

/**
 *  Return the members indices of a CssTableClass ordered for a where clause.
 *  Id members (arid,orid, etc) are place first followed by all other integer
 *  members, all string members and then all float and double members.
 *  Members of type CSS_LDDATE are not included.
 *  @param css A CssTableClass object.
 *  @param where The ordered member indices.
 *  @return the number of indices returned in where[].
 */
int
ODBCGetWhereMembers(CssTableClass *css, int *where)
{
	int i, j, k, num_where, num;
	CssClassDescription *des;

	num = css->getNumMembers();
	des = css->description();

	/* If the table has "ids" (arid, orid, etc), put then first in the
	 * where clause.
	 */
	num_where = 0;
	for(i = 0; i < num; i++) {
	    if(stringEndsWith(des[i].name, "id") && (des[i].type == CSS_LONG
		|| des[i].type == CSS_INT))
	    {
		where[num_where++] = i;
	    }
	}
	/* Put all other int's and long's next in the where clause.
	 */
	k = num_where;
	for(i = 0; i < num; i++) {
	    for(j = 0; j < k && where[j] != i; j++);
	    if(j == k && (des[i].type == CSS_LONG || des[i].type == CSS_INT
		|| des[i].type == CSS_JDATE || des[i].type == CSS_STRING
		|| des[i].type == CSS_DATE))
	    {
		where[num_where++] = i;
	    }
	}
	/* Put double's and float's last. Don't include any CSS_LDDATE's
	 */
	k = num_where;
	for(i = 0; i < num; i++) {
	    for(j = 0; j < k && where[j] != i; j++);
	    if(j == k && (des[i].type == CSS_TIME || des[i].type == CSS_DOUBLE
		|| des[i].type == CSS_FLOAT))
	    {
		where[num_where++] = i;
	    }
	}
	return num_where;
}
#endif /* HAVE_LIBODBC */
