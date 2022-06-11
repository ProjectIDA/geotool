/*	SccsId: %W%	%G%	*/

#ifndef _LOG_ERROR_MSG_H_
#define	_LOG_ERROR_MSG_H_

#include <syslog.h>
#include "idcsyslog.h"

void logErrorMsg(int priority, const char *msg);
int WorkingDialog(void *w, int num, int status, const char *string);

#endif /* _LOG_ERROR_MSG_H_ */
