/* ODBCRequestIds.c

    ODBCSetRequestIdIncrement()
    ODBCGetNextId()
    ODBCRequestIds()
    ODBCUpdateLastid()
    ODBCRecycleIds()
    ODBCNumberIDsAvailable()
*/
/* Author: Ivan Henson, Scientific Computing
 */

/**
 * Routines to request ids from the lastid table.
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libgdb.h"
#include "libstring.h"
#include "gobject++/CssTables.h"

#ifdef HAVE_LIBODBC

/** @private
 */
typedef struct
{
	int	startValue;
	int	numValues;
} IdSegment;

/** @private
 */
typedef struct
{
	char		keyname[16];
	int 		num_segments;
	IdSegment	*segment;
	int		idIncrement;
} RecycledIds;

static int numKeys = 0;
static RecycledIds *recycled;

static RecycledIds *getRecycledList(const char *keyname);
static int getRecycledIds(SQLHDBC hdbc, const char *tableName,
		const char *keyname, int numRequested, int consecutive,
		int *ids, int *numReturned);
static int getLastId(SQLHDBC hdbc, const char *tableName, const char *keyname,
		int numRequested, int *nextid, int consecutive);
static int addRecycleSegment(RecycledIds *r, int startValue, int numValues);
static int sortIdSegment(const void *A, const void *B);

/**
 * Set the lastid request increment for keyname. For the input keyname, set
 * the minimum number of ids that will actually be requested from the lastid
 * table each time a request is made with ODBCGetNextId or ODBCRequestIds.
 * Extra ids are saved in the recycle list.
 * @param keyname The keyname of the lastid record ("arid", "orid", etc.).
 * @param increment The minimum number of ids to request from the database.
 * @return 0 for success, -1 if increment <= 0 or malloc fails. Retrieve the \
 * 	error number and error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCSetRequestIdIncrement(const string &keyname, int idIncrement)
{
	RecycledIds *r;

	if(idIncrement <= 0) {
	    ODBCSetErrorMsg("ODBCSetRequestIdIncrement: invalid idIncrement.",
				0, 0);
	    return -1;
	}

	if((r = getRecycledList(keyname.c_str())) == NULL) return -1;

	r->idIncrement = idIncrement;

	return 0;
}

static RecycledIds *
getRecycledList(const char *keyname)
{
	int i;

	/* Get the recycled id list for the input keyname.
	 */
	for(i = 0; i < numKeys && strcasecmp(recycled[i].keyname,keyname); i++);
	if(i < numKeys) {
	    return &recycled[i];
	}
	else
	{
	    if(!numKeys) {
		recycled = (RecycledIds *)malloc(sizeof(RecycledIds));
	    }
	    else {
		recycled = (RecycledIds *)realloc(recycled,
					(numKeys+1)*sizeof(RecycledIds));
	    }
	    if(!recycled) {
		ODBCSetErrorMsg("getRecycledList: malloc failed.", 0, 0);
		return NULL;
	    }
	    stringcpy(recycled[numKeys].keyname, keyname,
			sizeof(recycled[numKeys].keyname));
	    recycled[numKeys].keyname[15] = '\0';
	    recycled[numKeys].num_segments = 0;
	    recycled[numKeys].segment = NULL;
	    recycled[numKeys].idIncrement = 1;
	    return &recycled[numKeys++];
	}
}

/**
 * Get an id (keyvalue) from the lastid table and increment it. The id is taken
 * from the recycled-id-list, if available. Otherwise, the keyvalue of the
 * lastid record corresponding to the <b>tableName</b> and <b>keyname</b> is
 * returned and increased by the <b>idIncrement</b> value. The
 * <b>idIncrement</b> defaults to 1 for all keynames. Use
 * ODBCSetRequestIdIncrement to set the <b>idIncrement</b>.
 * @param hdbc The SQLHDBC connection handle obtained from ODBCConnect()
 * @param tableName The name of the lastid table including the account, \
 *	if necessary.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @return The keyvalue (next id) or -1 for failure. Retrieve the error number \
 *	and error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCGetNextId(SQLHDBC hdbc, const string &tableName, const string &keyname)
{
	int nextid[1], numReturned;

	if(!ODBCRequestIds(hdbc, tableName, keyname, 1, 0, nextid,
				&numReturned))
	{
	    return nextid[0];
	}
	return -1;
}

/**
 * Request one or more ids from the lastid table. The requested number of
 * ids are taken from the recycled-id-list, if available. If necessary,
 * ids are taken from the lastid table. The keyvalue of the lastid table
 * is incremented by the larger of the number of ids needed or the
 * <b>idIncrement</b>.
 * @param hdbc The SQLHDBC connection handle obtained from ODBCConnect().
 * @param tableName The name of the lastid table including the account, \
 *	 if necessary.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @param numRequested The number of ids requested.
 * @param consecutive Set to 1 to request consecutive ids. Set to 0 to request \
 *	any available ids.
 * @param ids An array to hold the returned <b>numRequested</b> ids.
 * @param numReturned The actual number of ids returned in <b>ids</b>.
 * @return 0 for success, -1 for failure. Retrieve the error number \
 *      and error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCRequestIds(SQLHDBC hdbc, const string &tableName, const string &keyname,
		int numRequested, int consecutive, int *ids, int *numReturned)
{
	int nextid=0, need;
	int i, n, ret;

	/* First check the recycled ids.
	 */
	ret = getRecycledIds(hdbc, tableName.c_str(), keyname.c_str(),
			numRequested, consecutive, ids, numReturned);
	if(ret < 0) return ret;
	if(numRequested == *numReturned) return 0;

	/* Need more ids from the lastid table
	 */
	need = numRequested - *numReturned;
	ret = ODBCUpdateLastid(hdbc, tableName, keyname, need, &nextid);
	if(ret) {
	    return ret;
	}
	n = *numReturned;
	for(i = 0; i < need; i++) {
	    ids[n++] = nextid++;
	}
	*numReturned = numRequested;
	return 0;
}

static int
getRecycledIds(SQLHDBC hdbc, const char *tableName, const char *keyname,
		int numRequested, int consecutive, int *ids, int *numReturned)
{
	int i, j, ret;
	RecycledIds *r;

	*numReturned = 0;

	/* Get the recycled id list for the input keyname.
	 */
	for(j = 0; j < numKeys && strcasecmp(recycled[j].keyname,keyname); j++);
	if(j == numKeys) return 0;

	r = &recycled[j];

	if(consecutive)
	{
	    /* search for numRequested consecutive ids in the recycled list.
	     */
	    for(i = 0; i < r->num_segments &&
		r->segment[i].numValues < numRequested; i++);
	    if(i < r->num_segments) 
	    {
		/* found a segment with >= numRequested ids.
		 */
		for(j = 0; j < numRequested; j++) {
		    ids[j] = r->segment[i].startValue++;
		    r->segment[i].numValues--;
		}
		if(r->segment[i].numValues == 0) {
		    for(j = i+1; j < r->num_segments; j++) {
			r->segment[j-1] = r->segment[j];
		    }
		    r->num_segments--;
		}
		*numReturned = numRequested;
	    }
	    else if(r->num_segments)
	    {
		int lastid=0, need;
		/* Did not find numRequested consecutive ids in the recycled
		 * list. Check if the database lastid is consecutive with the 
		 * last recycled segment.
		 */
		j = r->num_segments - 1;
		lastid = r->segment[j].startValue + r->segment[j].numValues - 1;
		need = numRequested - r->segment[j].numValues;
		ret = getLastId(hdbc, tableName, keyname, need, &lastid, 1);
		if(ret < 0) return ret; /* error return */
		if(!ret) {
		    /* The database lastid was consecutive with the last
		     * recycle segment. Use the last recycle segment plus
		     * some ids from the database.
		     */
		    for(j = 0; j < numRequested; j++) {
			ids[j] = r->segment[j].startValue++;
		    }
		    for(j = i+1; j < r->num_segments; j++) {
			r->segment[j-1] = r->segment[j];
		    }
		    r->num_segments--;
		    *numReturned = numRequested;
		}
	    }
	}
	else {
	    /* ids do not need to be consecutive. Simple return numRequested
	     * ids from the recycled list, or all that there is.
	     */
	    int n = 0;
	    for(i = 0; i < r->num_segments && n < numRequested; i++)
	    {
		for(j = 0; j < r->segment[i].numValues && n < numRequested; j++)
		{
		    ids[n++] = r->segment[i].startValue++;
		}
		r->segment[i].numValues -= j;
	    }
	    for(i = j = 0; i < r->num_segments; i++) {
		if(r->segment[i].numValues) {
		    r->segment[j++] = r->segment[i];
		}
	    }
	    r->num_segments = j;
	    *numReturned = n;
	}
	return 0;
}

/**
 * Update the lastid table. The keyvalue of the lastid table is returned in
 * <b>keyvalue</b> and incremented by <b>numRequested</b>.
 * @param hdbc The SQLHDBC connection handle obtained from ODBCConnect().
 * @param tableName The name of the lastid table including account if necessary.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @param numRequested The number of ids requested.
 * @return 0 for success, -1 for error. Retrieve the error number \
 *	and error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCUpdateLastid(SQLHDBC hdbc, const string &tableName, const string &keyname,
		int numRequested, int *keyvalue)
{
	return getLastId(hdbc, tableName.c_str(), keyname.c_str(),
			numRequested, keyvalue, 0);
}

static int
getLastId(SQLHDBC hdbc, const char *tableName, const char *keyname,
		int numRequested, int *nextid, int consecutive)
{
	char query[200];
	SQLHSTMT hstmt;
	SQLRETURN ret;
	SQLINTEGER keyvalue;
	SQLLEN ind;
	CssLastidClass lastid;
	int memberIndices[1];
	int whereIndices[1];
	RecycledIds *r;

	if((r = getRecycledList(keyname)) == NULL) return -1;

	if(ODBCGetAutoCommit(hdbc) != 0) {
	    ODBCSetErrorMsg("ODBCRequestIds: connection autoCommit must be 0.",
			0, 0);
	    return -1;
	}
	    
	snprintf(query, 200,
"select keyvalue from %s where keyname='%s' for update of keyvalue wait 3",
		tableName, keyname);

	ret = ODBCAllocStmt(hdbc, &hstmt);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCRequestIds", SQL_HANDLE_DBC, hdbc);
	    return -1;
	}

	/* Execute the query */
	ret = SQLExecDirect(hstmt, (unsigned char *) query, SQL_NTS);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCRequestIds", SQL_HANDLE_STMT, hstmt);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	       logErrorMsg(LOG_WARNING,"ODBCRequestIds: SQLFreeHandle failed.");
	    }
	    SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
	    return -1;
	}

	/* bind the keyvalue */
	ret = SQLBindCol(hstmt, 1, SQL_INTEGER, (SQLPOINTER)&keyvalue,
			sizeof(long), &ind);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCRequestIds", SQL_HANDLE_STMT, hstmt);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	       logErrorMsg(LOG_WARNING,"ODBCRequestIds: SQLFreeHandle failed.");
	    }
	    return -1;
	}

	/* Fetch the result */
	ret = SQLFetch(hstmt);

	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
	    ODBCSetErrorMsg("ODBCRequestIds", SQL_HANDLE_STMT, hstmt);
	    if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	       logErrorMsg(LOG_WARNING,"ODBCRequestIds: SQLFreeHandle failed.");
	    }
	    return -1;
	}
        if(SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS) {
	    logErrorMsg(LOG_WARNING, "ODBCRequestIds: SQLFreeHandle failed.");
        }

	/* If consecutive, the database keyvalue must = *nextid
	 */
	if(consecutive && keyvalue != *nextid)
	{
	    SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
	    return 1;
	}
	*nextid = keyvalue + 1;

	stringcpy(lastid.keyname, keyname, sizeof(lastid.keyname));
	if(numRequested >= r->idIncrement) {
	    lastid.keyvalue = keyvalue + numRequested;
	}
	else {
	    lastid.keyvalue = keyvalue + r->idIncrement;
	}

	memberIndices[0] = lastid.memberIndex("keyvalue");
	whereIndices[0]  = lastid.memberIndex("keyname");

	ret = ODBCUpdateTableWhere(hdbc, tableName, 1, memberIndices, 1,
				whereIndices, &lastid, &lastid);

	if(ret) {
	    SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
	    return -1;
	}
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);

	/* add extra ids to the recycled list
	 */
	if(numRequested < r->idIncrement) {
	    if(addRecycleSegment(r, keyvalue + numRequested + 1,
				r->idIncrement - numRequested)) return -1;
	}
	return ret;
}

/**
 * Recycle ids from the lastid table. Give ids to the recycle list to be used
 * by ODBCGetNextId and ODBCRequestIds.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @param ids An array of ids.
 * @param numReturned The number <b>ids</b> to put in the recycle list.
 * @return 0 for success, -1 if malloc fails. Retrieve the error number \
 *	and error message with ODBCErrno() and ODBCErrMsg().
 */
int
ODBCRecycleIds(const string &keyname, int *ids, int numReturned)
{
	int i, j;
	RecycledIds *r;

	if((r = getRecycledList(keyname.c_str())) == NULL) return -1;

	for(i = 0; i < numReturned; ) {
	    for(j = i+1; j < numReturned && ids[j] == ids[j-1]+1; j++);
	    if(addRecycleSegment(r, ids[i], j-i)) return -1;
	    i = j;
	}
	return 0;
}

static int
addRecycleSegment(RecycledIds *r, int startValue, int numValues)
{
	int i, j;

	if(r->segment == NULL) {
	    r->segment = (IdSegment *)malloc(sizeof(IdSegment));
	}
	else {
	    r->segment = (IdSegment *)realloc(r->segment,
				(r->num_segments+1)*sizeof(IdSegment));
	}
	if(!r->segment) {
	    ODBCSetErrorMsg("addRecycleSegment: malloc failed.", 0, 0);
	    return -1;
	}
	r->segment[r->num_segments].startValue = startValue;
	r->segment[r->num_segments].numValues = numValues;
	r->num_segments++;

	/* Sort segments.
 	 */
	qsort(r->segment, r->num_segments, sizeof(IdSegment), sortIdSegment);
	
	/* Join segments that are consecutive.
	 */
	for(i = r->num_segments-1; i > 0; i--)
	{
	    if(r->segment[i].startValue ==
		r->segment[i-1].startValue + r->segment[i-1].numValues)
	    {
		r->segment[i-1].numValues += r->segment[i].numValues;
		for(j = i; j < r->num_segments-1; j++) {
		    r->segment[j] = r->segment[j+1];
		}
		r->num_segments--;
	    }
	}
	return 0;
}

static int
sortIdSegment(const void *A, const void *B)
{
	const IdSegment *a = (const IdSegment *)A;
	const IdSegment *b = (const IdSegment *)B;

	return (a->startValue < b->startValue ? -1 : 1);
}

/**
 * Get the number of recycled ids available. For the input keyname, return the
 * number of ids in the recycled-id-list.
 * @param keyname The lastid keyname ("arid", "orid", etc.).
 * @return The number of ids in the recycled-id-list.
 */
int
ODBCNumberIDsAvailable(const char *keyname)
{
	int i, num;
	RecycledIds *r;
	
	if((r = getRecycledList(keyname)) == NULL) return 0;

	num = 0;
	for(i = 0; i < r->num_segments; i++) {
	    num += r->segment[i].numValues;
	}
	return num;
}
#endif /* HAVE_LIBODBC */
