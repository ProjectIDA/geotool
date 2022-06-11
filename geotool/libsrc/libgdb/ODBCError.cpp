/* ODBCError.c

    ODBCGetErrorMsg() and ODBCSetErrorMsg()
*/
/* Author: Ivan Henson, Scientific Computing
 */
/**
 *  Routines for handling error messages. ODBCGetErrorMsg can be used after
 *  direct calls to libodbc routines (the SQL prefix) to get the libodbc
 *  error message.
 *  <p>
 *  ODBCSetErrorMsg is used mainly inside libgdb to retrieve the libodbc
 *  error message, prefix a libgdb message and store the message in a static
 *  location that will be returned by ODBCErrMsg.
 *  <p>
 *  ODBCErrMsg and ODBCErrno can be used after calls to libgdb routines
 *  to retrieve the most recent error message and libodbc error number.
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "libgdb.h"

#ifdef HAVE_LIBODBC


#define MSG_SIZE 1024
static char ODBC_error_msg[MSG_SIZE] = "";
static SQLINTEGER ODBC_error_no;

/**
 * Get the error message for the most recent libodbc library error.
 * @param handleType the handle type of the second argument.
 * @param handle the handle on which the error occurred.
 * @param error_msg the message.
 * @return error_msg pointer.
 */
char *
ODBCGetErrorMsg(SQLSMALLINT handleType, SQLHANDLE handle, char *error_msg)
{
	SQLINTEGER	err = 0;
	SQLSMALLINT	mlen;
	SQLRETURN	ret;
	unsigned char	stat[200]; /* Status SQL */
	unsigned char	msg[200];

	msg[0] = '\0';
	ret = SQLGetDiagRec(handleType, handle, 1, stat, &err, msg,
				sizeof(msg), &mlen);
	if(ret == SQL_SUCCESS) {
	    snprintf(error_msg, MSG_SIZE, "%s %d", msg, (int)err);
	}
	return error_msg;
}

/**
 * Set the ODBC error message that will be returned by ODBCErrMsg.
 * @param header A text preface to the error message.
 * @param handleType the handle type of the third argument.
 * @param handle the handle on which the error occurred.
 * @return the error message.
 */
char *
ODBCSetErrorMsg(const char *header, SQLSMALLINT handleType, SQLHANDLE handle)
{
	SQLINTEGER	err=0;
	SQLSMALLINT	mlen;
	SQLRETURN	ret;
	unsigned char	stat[200]; /* Status SQL */
	unsigned char	msg[200];

	if(handleType == 0) {
	    if(errno > 0) {
		snprintf(ODBC_error_msg, MSG_SIZE, "%s: %s",
			header, strerror(errno));
            }
            else {
		snprintf(ODBC_error_msg, MSG_SIZE, "%s", header);
            }
	    ODBC_error_no = 0;
	}
	else {
	    msg[0] = '\0';
	    stat[0] = '\0';
	    ret = SQLGetDiagRec(handleType, handle, 1, stat, &err, msg,
				sizeof(msg), &mlen);
	    ODBC_error_no = err;
	    if(ret == SQL_SUCCESS) {
		snprintf(ODBC_error_msg, MSG_SIZE, " %s: %s %d", header, msg,
			(int)err);
	    }
	    else if(ret == SQL_SUCCESS_WITH_INFO) {
		snprintf(ODBC_error_msg, MSG_SIZE,
                        "%s: %s (truncated)", header, msg);
	    }
	    else if(ret == SQL_INVALID_HANDLE) {
		snprintf(ODBC_error_msg, MSG_SIZE,
		    "%s: Invalid handleType input to ODBCSetErrorMsg.", header);
	    }
	    else if(ret == SQL_ERROR) {
		snprintf(ODBC_error_msg, MSG_SIZE,
			"%s: SQLGetDiagRec failed.", header);
	    }
	    else if(ret == SQL_NO_DATA) {
		snprintf(ODBC_error_msg, MSG_SIZE,
			"%s: No diagnostic records.", header);
	    }
		
	}
	return ODBC_error_msg;
}

/**
 * Get a libgdb error message.
 * @return the most recent ODBC error message.
 */
char *
ODBCErrMsg(void)
{
	return ODBC_error_msg;
}

/**
 * Get a libgdb error number.
 * @return the most recent ODBC error number.
 */
int
ODBCErrno(void)
{
	return ODBC_error_no;
}
#endif /* HAVE_LIBODBC */
