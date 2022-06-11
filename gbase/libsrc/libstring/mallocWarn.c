/*
 * NAME
 *      mallocWarn
 *      reallocWarn
 *
 * AUTHOR
 *      I. Henson
 */

#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "libstring.h"
#include "logErrorMsg.h"

/** 
 *  @private
 */
void *
mallocWarn(int nbytes)
{
	void *ptr;

	if(nbytes <= 0) {
	    return (void *)NULL;
	}
	else if((ptr = malloc(nbytes)) == (void *)0)
	{
	    char error[200];
	    if(errno > 0) {
		snprintf(error, 200, "malloc error: %s", strerror(errno));
	    }
	    else {
		snprintf(error, 200, "malloc error on %d bytes", nbytes);
	    }
	    logErrorMsg(LOG_ERR, error);
	    return (void *)NULL;
	}
	return ptr;
}

/** 
 *  @private
 */
void *
reallocWarn(void *ptr, int nbytes)
{
	if(nbytes <= 0) {
	    return ptr;
	}
	else if(ptr == (void *)NULL) {
	    return(mallocWarn(nbytes));
	}
	else if((ptr = realloc(ptr, nbytes)) == (void *)0)
	{
	    char error[200];
	    if(errno > 0) {
		snprintf(error, 200, "malloc error: %s", strerror(errno));
	    }
	    else {
		snprintf(error, 200, "malloc error on %d bytes", nbytes);
	    }
	    logErrorMsg(LOG_ERR, error);
	    return (void *)NULL;
	}
	return ptr;
}
