/** \file GError.cpp
 *  \brief Defines class GError.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "gobject++/GError.h"

static char error_message[5000];
static bool new_message = false;
static FILE *fp_out = NULL;

#ifdef __STDC__
/** Set the error message.
 *  @param[in] format a form suitable for printf.
 *  @param[in] ... variable length argument list suitable for printf.
 */
void GError::setMessage(const char *format, ...)
#else
/** Set the error message. This functions takes a variable length
 *  argument list suitable for printf. The first argument is the format.
 */
void GError::setMessage(va_alist) va_dcl
#endif
{
    va_list     va;
#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    error_message[0] = '\0';

    if(format == NULL || (int)strlen(format) <= 0) return;

    vsnprintf(error_message, sizeof(error_message), format, va);
    va_end(va);

    new_message = true;

    if(fp_out) {
	fprintf(fp_out, "%s\n", error_message);
    }
}

/** The function getMessage() returns the most recent error message set by
 *  the function setMessage() since the last call to getMessage(). It returns
 *  NULL if no errors have occurred since initialization or since it was last
 *  called.
 */
char * GError::getMessage(void)
{
    if(new_message) {
	new_message = false;
	return error_message;
    }
    return NULL;
}

/** Set the error message output FILE pointer.
 *  @param[in] fpout a FILE pointer to be used to print all error messages.
 */
void GError::printErrorMessages(FILE *fpout)
{
    fp_out = fpout;
}
