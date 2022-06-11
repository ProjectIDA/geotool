/* ODBCConnect.c

    ODBCOpen(), ODBCConnect(), ODBCDisconnect(), ODBCCloseDatabase()
*/
/* Author: Ivan Henson, Scientific Computing
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "libgdb.h"

#ifdef HAVE_LIBODBC

/**
 * Routines for opening and closing a connection to an ODBC server and
 * retrieving information about connections.
 */

/**
 * @private
 */
class ODBC_Connection
{
    public:
	SQLHDBC hdbc;
	string	dataSource;
	string	user;
	string	passwd;
	int	autoCommit;
	int	num_connection_requests;
};

static SQLHENV env = NULL;
static vector<ODBC_Connection *> connections;

/* This is the most recent open connection
 */
static SQLHDBC last_hdbc = NULL;

/**
 *  Allocate an ODBC environment handle.
 *  @return SQLHENV pointer or NULL if the allocation failed.
 */
SQLHENV
ODBCOpen(void)
{
	SQLRETURN ret;

	if(env == NULL)
	{
	    /* Allocate Environment handle and register version 
	     */
	    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	    {
		ODBCSetErrorMsg(
		    "ODBCOpen: SQLAllocHandle(SQL_HANDLE_ENV, failed.", 0,0);
		env = NULL;
		return env;
	    }
	    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
				(void*)SQL_OV_ODBC3, 0); 
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	    {
		ODBCSetErrorMsg( "ODBCOpen: SQLSetEnvAttr failed.", 0, 0);
		SQLFreeHandle(SQL_HANDLE_ENV, env);
		env = NULL;
		return env;
	    }
	}
	return env;
}

/**
 *  Open a connection to a data source. An SQLHDBC connection handle is
 *  returned. The connection will remain open until ODBCDisconnect() is
 *  called with the SQLHDBC connection handle. This routine first searches
 *  for a prior open connection that was created with the same dataSource,
 *  user, passwd and autoCommit. If a matching prior connection is found,
 *  its connection handle will be returned and no new resources will be
 *  allocated. If no matching open connection is found, than a new connection
 *  will be established and its connection handle will be returned.
 *  @param dataSource The name of the data source.
 *  @param user The account user name.
 *  @param passwd  The account password.
 *  @param autoCommit  If autoCommit=1, then all transactions will be \
 *		automatically committed. Rollbacks will not be possible. If \
 *		autoCommit=0, transactions will not be committed until a call \
 *		to SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT). Rollback with \
 *		SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK).
 *  @return SQLHDBC pointer or NULL if the connection failed. \
 *	Retrieve the error number and error message with ODBCErrno() \
 *	and ODBCErrMsg().
 *  @see ODBCDisconnect
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 */
SQLHDBC
ODBCConnect(const string &dataSource, const string &user, const string &passwd,
		int autoCommit)
{

	return ODBCConnect2(dataSource, user, passwd, autoCommit, 30);
}

/**
 *  Open a connection to a data source and specify login timeout. An SQLHDBC
 *  connection handle is returned. This routine performs the same function
 *  as ODBCConnect, with the additional loginTimeout argument.
 *  @param dataSource The name of the data source.
 *  @param user The account user name.
 *  @param passwd  The account password.
 *  @param autoCommit  If autoCommit=1, then all transactions will be \
 *		automatically committed. Rollbacks will not be possible. If \
 *		autoCommit=0, transactions will not be committed until a call \
 *		to SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT). Rollback with \
 *		SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK).
 *  @param loginTimeout The number of seconds that this routine will wait for \
 *		a connection to the data source.
 *  @return SQLHDBC pointer or NULL if the connection failed. \
 *	Retrieve the error number and error message with ODBCErrno() \
 *	and ODBCErrMsg().
 *  @see ODBCDisconnect
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 */
SQLHDBC
ODBCConnect2(const string &dataSource, const string &user, const string &passwd,
		int autoCommit, int loginTimeout)
{
	int i;
	string c_user, c_passwd;
	SQLHDBC hdbc;
	SQLRETURN ret;

	if(ODBCOpen() == NULL) return NULL;

	if( !passwd.empty() ) {
	    c_user = user;
	    c_passwd = passwd;
	}
	else { /* get password from user string */
	    size_t j;
	    c_user = user;
	    if((j = c_user.find("/")) != string::npos) {
		c_passwd = c_user.substr(j+1);
		c_user.erase(j);
	    }
	}

	for(i = 0; i < (int)connections.size() &&
		(dataSource.compare(connections[i]->dataSource) ||
		 c_user.compare(connections[i]->user) ||
		 c_passwd.compare(connections[i]->passwd) ||
		 autoCommit != connections[i]->autoCommit); i++);
	if(i < (int)connections.size()) {
	    connections[i]->num_connection_requests++;
	    return connections[i]->hdbc;
	}
        /* Allocate connection handle, set timeout
	 */
	ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &hdbc); 
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg(
		"ODBCConnect: SQLAllocHandle(SQL_HANDLE_DBC failed.", 0, 0);
	    return NULL;
	}
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)30, 0);

	/* Connect to the datasource
	 */
	ret = SQLConnect(hdbc, (SQLCHAR *)dataSource.c_str(), SQL_NTS,
				(SQLCHAR *)c_user.c_str(), SQL_NTS,
				(SQLCHAR *)c_passwd.c_str(), SQL_NTS);
	if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
	    ODBCSetErrorMsg("ODBCConnect", SQL_HANDLE_DBC, hdbc);
	    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	    return NULL;
	}
	if(!autoCommit) {
	    ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT,
			(SQLPOINTER) SQL_AUTOCOMMIT_OFF, SQL_NTS);
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	    {
		char msg[100];
		ODBCSetErrorMsg("ODBCConnect", SQL_HANDLE_DBC, hdbc);
		snprintf(msg, 100,
			"ODBCConnect: unable to turn AUTOCOMMIT off: %s",
			    ODBCErrMsg());
		logErrorMsg(LOG_WARNING, msg);
	    }
	}

	ODBC_Connection *c = new ODBC_Connection();
	connections.push_back(c);
	
	c->hdbc = hdbc;
	c->dataSource = dataSource;
	c->user = c_user;
	c->passwd = c_passwd;
	c->autoCommit = autoCommit;
	c->num_connection_requests = 1;
	last_hdbc = hdbc;
	return hdbc;
}

/**
 *  Disconnect from a data source. Call ODBCDisconnect when a connection
 *  handle is no longer needed locally.
 *  @param hdbc The SQLHDBC connection handle obtained from ODBCConnect()
 *  @see ODBCConnect
 */
void
ODBCDisconnect(SQLHDBC hdbc)
{
	int i;

	for(i = 0; i < (int)connections.size() &&
		connections[i]->hdbc != hdbc; i++);
	if(i < (int)connections.size()) {
	    connections[i]->num_connection_requests--;

	    ODBCCommit(hdbc);

	    if(connections[i]->num_connection_requests == 0)
	    {
                SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);

		delete connections[i];
		connections.erase(connections.begin()+i);

		if(hdbc == last_hdbc) {
		    if((int)connections.size() > 0) {
			last_hdbc = connections.back()->hdbc;
		    }
		    else {
			last_hdbc = NULL;
		    }
		}
	    }
	}
}

/**
 *  Commit transactions. Commit all transaction for the handle hdbc.
 *  @param hdbc The SQLHDBC connection handle obtained from ODBCConnect()
 *  @see ODBCConnect
 */
void
ODBCCommit(SQLHDBC hdbc)
{
	int ret;

	/* if autocommit is off, commit the changes */
	if(ODBCGetAutoCommit(hdbc) == 0) {
	    ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
	    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		char msg[100];
		ODBCSetErrorMsg("ODBCConnect", SQL_HANDLE_DBC, hdbc);
		snprintf(msg, 100, "ODBCCommit: unable to commit changes: %s",
			ODBCErrMsg());
		logErrorMsg(LOG_WARNING, msg);
	    }
	}		
}

/**
 *  Disconnect from all data sources. ODBCDisconnect is called to disconnect
 *  all open connections and the ODBC environment resources are released.
 */
void
ODBCCloseDatabase(void)
{
	int i;

	for(i = (int)connections.size()-1; i >= 0; i--) {
	    /* force ODBCDisconnect to really disconnect */
	    connections[i]->num_connection_requests = 1;
	    ODBCDisconnect(connections[i]->hdbc);
	}
	SQLFreeHandle(SQL_HANDLE_ENV, env);
}

/**
 *  Get data sources from the ODBC server. The names of all available
 *  data sources are returned. This routine allocates the array of strings,
 *  dataSource, and each string element in the array. The calling routine
 *  may free this space with code such as:
 *  <pre>
 *  #include "libgdb.h"
 * 
 *  int i, num_sources;
 *  char **dataSources;
 *
 *  num_sources = ODBCDataSources(&num, &dataSources);
 *  ...
 *  for(i = 0; i < num_sources; i++) free(dataSources[i]);
 *  free(dataSources);
 *  </pre>
 *  @param num The number of data sources (returned).
 *  @param dataSource The data source names (returned).
 *  @return 0 for success, -1 for failure. Retrieve the error number \
 *	and error message with ODBCErrno() and ODBCErrMsg().
 *  @see ODBCErrno
 *  @see ODBCErrMsg
 */
int
ODBCDataSources(int *num, char ***dataSource)
{
	SQLHENV this_env;
	int num_sources=0;
	char **dataSources;
	unsigned char dsn[SQL_MAX_DSN_LENGTH+1];
        unsigned char description[256];
        SQLSMALLINT dsnLen, descriptionLen;
        SQLUSMALLINT direction;
        SQLRETURN ret;

	if((this_env = ODBCOpen()) == NULL) {
	    return -1;
	}
	*num = 0;
	dataSources = (char **)malloc(sizeof(char *));
	/* get available data sources
	*/
	direction = SQL_FETCH_FIRST;
        while((ret = SQLDataSources(this_env, direction,
			dsn, sizeof(dsn), &dsnLen,
			description, sizeof(description), &descriptionLen))
		== SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
	    direction = SQL_FETCH_NEXT;
	    dataSources = (char **)realloc(dataSources,
				(num_sources+1)*sizeof(char *));
	    dataSources[num_sources] =
			(char *)malloc(strlen((const char *) dsn) + 1);
	    strcpy(dataSources[num_sources++], (const char *) dsn);
        }
	if(ret != SQL_NO_DATA)
	{
	    ODBCSetErrorMsg("OBDCDataSources", SQL_HANDLE_ENV, env);
	    return -1;
	}
	*num = num_sources;
	*dataSource = dataSources;
	return 0;
}

/**
 * Get the data source name. Returns the data source name for an open
 * connection.
 * @param hdbc The connection handle (input).
 * @return The data source name or NULL if the connection handle is invalid. \
 *		Do not free or alter the returned string.
 *  @see ODBCConnect
 */
const char *
ODBCGetDataSource(SQLHDBC hdbc)
{
	int i;

	for(i = 0; i < (int)connections.size(); i++) {
	    if(connections[i]->hdbc == hdbc) {
		return connections[i]->dataSource.c_str();
	    }
	}
	return NULL;
}

/**
 * Get the data source user name. Returns the data source user name for an open
 * connection.
 * @param hdbc The connection handle (input).
 * @return The user name or NULL if the connection handle is invalid. \
 *		Do not free or alter the returned string.
 *  @see ODBCConnect
 */
const char *
ODBCGetUser(SQLHDBC hdbc)
{
	int i;

	for(i = 0; i < (int)connections.size(); i++) {
	    if(connections[i]->hdbc == hdbc) {
		return connections[i]->user.c_str();
	    }
	}
	return NULL;
}

/**
 * Get the data source password. Returns the data source password for an open
 * connection.
 * @param hdbc The connection handle (input).
 * @return The data source password or NULL if the connection handle is \
 *		invalid. Do not free or alter the returned string.
 *  @see ODBCConnect
 */
const char *
ODBCGetPassword(SQLHDBC hdbc)
{
	int i;

	for(i = 0; i < (int)connections.size(); i++) {
	    if(connections[i]->hdbc == hdbc) {
		return connections[i]->passwd.c_str();
	    }
	}
	return NULL;
}

/**
 * Get the autoCommit state. Returns 1 if auto-commit is turned on, or 0 if
 * it is turned off.
 * @param hdbc The connection handle (input).
 * @return The autoCommit state (0,1) or -1 if the connection handle is invalid.
 *  @see ODBCConnect
 */
int
ODBCGetAutoCommit(SQLHDBC hdbc)
{
	int i;

	for(i = 0; i < (int)connections.size(); i++) {
	    if(connections[i]->hdbc == hdbc) {
		return connections[i]->autoCommit;
	    }
	}
	return -1;
}

/**
 *  Get the most recent open connection. Returns an SQLHDBC connection handle
 *  for the most recent connection. Returns NULL if no connections are open.
 *  @see ODBCConnect
 */
SQLHDBC
ODBCGetLastConnection(void)
{
	return last_hdbc;
}

static int stmt_timeout = 30;
/**
 * Set the query statement timeout. This timeout will be used with all
 * ODBC queries. The ODBC server is asked to wait <b>timeout</b> seconds
 * before giving up on a query.
 * @param timeout The statement timeout in seconds.
 */
void
ODBCSetStmtTimeout(int timeout)
{
	if(timeout > 0) stmt_timeout = timeout;
}

/**
 * Get the timeout of a query statement
 * @param hstmt The statement handle
 * @return timeout or -1 on error
 */
int
ODBCGetStmtTimeout(SQLHSTMT *hstmt)
{
       SQLRETURN ret;  
       int timeout;
       int retvalue = -1;  /* default to error */
  
       ret = SQLGetStmtAttr(hstmt, SQL_ATTR_QUERY_TIMEOUT,
		       	(SQLPOINTER)&timeout, 0, NULL);

       if(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
           retvalue = timeout;
       }

       return retvalue;
}


/**
 * Verify the timeout of a query statement. Returns whether the query timeout
 * of the statement matches the global statement timeout.
 * @param hstmt The statement handle
 * @return True (1) if timeouts match, otherwise false (0)
 */
int
ODBCCheckStmtTimeout(SQLHSTMT *hstmt)
{
     int timeout;

     timeout = ODBCGetStmtTimeout(hstmt);
     
     return ( timeout == stmt_timeout );

}

/**
 * @private
 */
SQLRETURN
ODBCAllocStmt(SQLHDBC hdbc, SQLHSTMT *hstmt)
{
	SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, hstmt);

	if(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
	    SQLSetStmtAttr(hstmt, SQL_ATTR_QUERY_TIMEOUT,
			(SQLPOINTER)&stmt_timeout, 0);
	}
	return ret;
}
#endif /* HAVE_LIBODBC */
