
/*******************************************************/
/*  International Data Centre                          */
/*  Comprehensive Test Ban Treaty Organization         */
/*  Vienna                                             */
/*  Austria                                            */
/*******************************************************/

/**
 * @author    Gerald Klinkl, Siemens Inc. Austria
 * @version   $Revision: 1.9 $
 * @date      $Date: 2006/05/24 13:12:57 $
 *
 * $Log: idcsyslog.c,v $
 * Revision 1.9  2007/09/28 10:45:52  Elena Tomuta
 * Added a new function setIDCDebugLevel, with the same
 * functionality as setDebugLevel which clashes with some
 * oracle driver libraries. setDebugLevel is deactivated 
 * after the first call to setIDCDebugLevel. 
 *
 * $Log: idcsyslog.c,v $
 * Revision 1.9  2006/05/24 13:12:57  gerald
 * Swap details and errno string
 *
 * Revision 1.8  2005/06/16 07:28:52  gerald
 * Fix definition of MAXLOGSIZE (add parenthesis)
 *
 * Revision 1.7  2005/06/15 07:05:59  gerald
 * Bug Fix: correct the (define) constant used for buffer allocation by the
 * longLogWrite(). There was a problem with the preprocessor, which caused
 * sizeof operator to return wrong buffer size (idriz.smaili@ctbto.org)
 *
 * Revision 1.6  2005/06/14 15:14:19  gerald
 * Bug Fix: correct the maximum length of messages that can be handled by
 * the longLogWrite()
 *
 * Revision 1.5  2005/06/03 12:30:15  gerald
 * Add new function longLogWrite(). This function should be used only
 * if the output buffer of logWrite() is too small or if the application
 * is not multi threaded
 *
 * Revision 1.4  2005/04/29 11:05:13  gerald
 * Extend max. length of syslog messages
 *
 * Revision 1.3  2004/11/12 14:17:42  gerald
 * Change log2stdout() that static strings could also be logged
 *
 * Revision 1.2  2004/10/18 14:26:59  gerald
 * Fix that syslog messages are written to stdout instead of stderr
 * when debugLevel > 0
 *
 * Revision 1.1  2004/06/08 13:26:23  gerald
 * Move logging functions in onw library (libidcsyslog)
 *
 * Revision 1.13  2004/06/02 09:14:40  gerald
 * Add get and set functions for debuglevel and syslog
 * filtering and and remove using external variables
 * "debugLevel" and "log2syslog"
 *
 * Revision 1.12  2004/04/26 13:37:51  gerald
 * Add more fine control for logging different message types
 * (errors, warnings ) to syslog
 *
 * Revision 1.11  2004/04/21 12:31:37  georgian
 * log2syslog condition modified; it uses SYSLOG flag for better readablity
 *
 * Revision 1.10  2004/04/21 10:05:32  georgian
 * Syslog switch added for setting syslog entries
 *
 * Revision 1.9  2003/11/07 11:13:11  gerald
 * Add date and time to debug messages if compiled with -DIMS
 *
 * Revision 1.8  2003/03/24 14:48:46  gerald
 * Fix splint warnings
 *
 * Revision 1.7  2003/03/17 14:17:17  gerald
 * Remove trailing blanks
 *
 * Revision 1.6  2003/02/12 09:10:34  ctbto
 * Fix warnings that occur when compliling with  -Wall
 *
 * Revision 1.5  2003/01/10 11:07:35  ctbto
 * Change type of debugLevel to unsigned short
 *
 * Revision 1.4  2002/10/30 09:42:20  ctbto
 * cast pthread_t datatype to unsigned long to be compatible
 * to the printf format string on all supported architectures
 *
 * Revision 1.3  2002/10/15 07:43:11  ctbto
 * Fix bug in log2stdout()
 *
 * Revision 1.2  2002/06/20 08:01:43  ctbto
 * Rewrite DEBUG functions (macros) to reduce stack space and make them
 * checkable by SPLINT. Change log functions to reduce stack and save one
 * function call.
 *
 * Revision 1.1.1.1  2002/06/18 07:28:05  ctbto
 * Import sources
 *
 */

/**
 * @short Core Logging Functions
 *
 * @desciption All logging have to be done using these functions
 * @desciption Logging doesn't include generation of .bin .tbs .txt
 * @desciption and .wfdisc files
 *
 * @description Logging is written to syslog instead of a stationLog file
 * @description within CD-1.1. If "debugLevel" is set, all messages are logged
 * @description to DEBUG_FP too (unbuffered).
 *
 * @description A trailer is inserted before each log message
 * @description All data written to DEBUG_FP is written unbuffered!!
 *
 * @description Use LOG_INFO to log all kind of summaries,
 * @description statistics and additional info
 *
 * @description Use LOG_NOTICE to log all kind of normal start/stop of
 * @description application, threads and connections
 *
 * @description Use LOG_WARNING to log all kind of recoverable errors
 *
 * @description Use LOG_ERR to log all kind of non recoverable errors
 * @description which impacts a single connection.
 *
 * @description Use LOG_CRIT to log all kind of non recoverable errors
 * @description which impacts the whole application.
 *
 * @description Use LOG_DEBUG to log all kind of debug messages, but use it
 * @description only if the debug functions don't fit your needs. Avoid to
 * @description flood the syslog deamon with debug messages!
 */

#include <config.h>

#ifdef S_SPLINT_S
 #include "splint_hints.h"
#endif /* S_SPLINT_S */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "idcsyslog.h"

/* file globals */
static unsigned short log2syslog = 2;   /* Syslog switch */
static unsigned short debugLevel = 0;   /* Debug Level */
static unsigned short debugCount = 0;   /* 1 if setDebugLevel has 
                                           been called */

/**
 * @short Set IDC debug level
 *
 * @param newDebugLevel     debug Level
 *
 * @return None
 */

void
setIDCDebugLevel(unsigned short newDebugLevel)
{
   debugLevel = newDebugLevel;
   if (!debugCount) {
      debugCount++;
   }
}


/**
 * @short Set debug level (Deprecated) 
 *
 * @param newDebugLevel     debug Level
 *
 * @description This function has been replaced with setIDCDebugLevel, 
 * @description as it was clashing with some ODBC driver libraries. 
 * @description Please use setIDCDebugLevel instead. 
 * @return None
 */

void
setDebugLevel(unsigned short newDebugLevel)
{
  if (!debugCount) {
    debugLevel = newDebugLevel;
  }
}

/**
 * @short Get debug level
 *
 * @return current debug level
 */

unsigned short
getDebugLevel()
{
   return(debugLevel);
}

/**
 * @short Set syslog filter level
 *
 * @param newSyslogLevel      new syslog level
 *
 * @description  This functions sets the new syslog filter level. With a
 * @description  value lower than 2 logWarning() calls are not logged
 * @description  to syslog.
 *
 * @return None
 */

void
setSyslogFilterLevel(unsigned short newSyslogLevel)
{
   log2syslog = newSyslogLevel;
}

/**
 * @short Get syslog filter level
 *
 * @return current syslog filter level
 */

unsigned short
getSyslogFilterLevel()
{
   return(log2syslog);
}

/**
 * @short Initialize logging
 *
 * @param appName string added to each syslog message (see openlog())
 * @param startPriority priorities lower than startPriorities are ignored but
 *        all messages are logged to DEBUG_FP regardless of their
 *        priority if debugLevel is set
 *
 * @return None
 */

void logInit(char *appName, unsigned int startPriority)
{
   (void) setlogmask (LOG_UPTO (startPriority));
   openlog (appName,  LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
}

/**
 * @short write messages to stdout only
 *
 * @description Use this function to write messages unbuffered to
 * @description stdout (Copyright messages and similar). Terminating
 * @description NIL is replaced with a NL
 *
 * @param msgToLog message to log
 *
 * @return None
 */

void log2Stdout(char *outString)
{
   size_t outLen;

   /* Retrieve string length */
   outLen = strlen(outString);

   /* Write string */
   (void) write(1, outString, outLen);

   /* Write newline here instead */
   (void) write(1, "\n", 1);
}

/**
 * @short Write long message with given priority to syslog
 *
 * @description This function sends a message with its given priority
 * @description to syslog. A standard trailer is inserted before each message
 * @description This function uses a bigger outbut buffer than logWrite(). If
 * @description possible use logWrite() in a multi threaded environment.
 * @description Messages are logged to DEBUG_FP unbuffered if debugLevel is set
 *
 * @param logLevel priority of message (see syslog())
 * @param msgToLog message to log
 *
 * @return None
 */

#define MAXLOGSIZE  (LOGSIZE + 18)

void longLogWrite(int logLevel, const char *msgToLog )
{
   char outString[MAXLOGSIZE * 2];

   /* Insert trailer: Thread_ID Log_Level Origin */

   (void) snprintf(outString, sizeof(outString), "(%04lu) %.392s",
                   (unsigned long)pthread_self(), msgToLog);
   /* Log to syslog only if the syslog flag is set to "ON" */
   if (log2syslog > 0) syslog(logLevel, "%s", outString);
   /* Log to DEBUG_FP too if debuglevel is set */
   DEBUG(1, outString);
}

/**
 * @short Write message with given priority to syslog
 *
 * @description This function sends a message with its given priority
 * @description to syslog. A standard trailer is inserted before each message
 * @description Messages are logged to DEBUG_FP unbuffered if debugLevel is set
 *
 * @param logLevel priority of message (see syslog())
 * @param msgToLog message to log
 *
 * @return None
 */

void logWrite(int logLevel, const char *msgToLog )
{
   char outString[MAXLOGSIZE];

   /* Insert trailer: Thread_ID Log_Level Origin */

   (void) snprintf(outString, sizeof(outString), "(%04lu) %.196s",
                   (unsigned long)pthread_self(), msgToLog);
   /* Log to syslog only if the syslog flag is set to "ON" */
   if (log2syslog > 0) syslog(logLevel,"%s", outString);
   /* Log to DEBUG_FP too if debuglevel is set */
   DEBUG(1, outString);
}

/**
 * @short Log error condition
 *
 * @description This function logs a error condition with an error
 * @description message and additional information to syslog with
 * @description LOG_ERR priority. Messages are also logged to DEBUG_FP
 * @description if debugLevel is set
 *
 * @param message     error message
 * @param info        additional info or NULL
 *
 * @return None
 */

void logError(const char *message, const char *detail)
{
   char outBuffer[MAXLOGSIZE];  /* per Thread or on stack!! */

   /* Here you can add more info like error message as string using
      errno, sys_errlist, sys_nerror etc..) */
   if (detail == NULL)
   {
       (void) snprintf(outBuffer, sizeof(outBuffer), "(%04lu) %.70s [%.30s]",
                      (unsigned long)pthread_self(), message, strerror(errno));
   }
   else
   {
       (void) snprintf(outBuffer, sizeof(outBuffer),
                       "(%04lu) %.70s [%.30s, %.80s]",
                      (unsigned long)pthread_self(), message,
                      strerror(errno), detail);
   }

   if (log2syslog > 0) syslog(LOG_ERR, "%s", outBuffer);
   /* Log to DEBUG_FP too if debuglevel is set */
   DEBUG(1, outBuffer);
}

/**
 * @short Log warning condition
 *
 * @description This function logs a warning condition with an warning
 * @description message and additional information to syslog with
 * @description LOG_WARNING priority. Messages are also logged to DEBUG_FP
 * @description if debugLevel is set
 *
 * @param message     warning message
 * @param info        additional info
 *
 * @return None
 */

void logWarning(const char *message, const char *detail)
{
   char outBuffer[MAXLOGSIZE];  /* per Thread or on stack!! */

   (void) snprintf(outBuffer, sizeof(outBuffer), "(%04lu) %.70s [%.80s]",
                  (unsigned long)pthread_self(), message, detail);
   if (log2syslog > 1) syslog(LOG_WARNING, "%s", outBuffer);
   /* Log to DEBUG_FP too if debuglevel is set */
   DEBUG(1, outBuffer);
}

#ifdef IMS

/**
 * @short Print UTC time stamp
 *
 * @description This function prints the current UTC in form of
 * @description an ASCII string with format "YYYYMMDD HH:MM:SS:ssss"
 * @description to stdout
 *
 * @param debugLevel     debug level
 *
 * @return  Nothing
 */

void
printTimeStamp(unsigned short debugLevel)
{
   struct tm utcTime;
   struct timeval nowTime;

   (void) gettimeofday(&nowTime, NULL);
   /* All times in the receiver are UTC times *NOT* local times */
   (void) gmtime_r(&(nowTime.tv_sec), &utcTime);
   (void) printf("Debug: %.2hu %4d%02d%02d %02d:%02d:%02d.%04d) ",
                 debugLevel,
                 utcTime.tm_year + 1900,
                 utcTime.tm_mon + 1,
                 utcTime.tm_mday,
                 utcTime.tm_hour,
                 utcTime.tm_min, utcTime.tm_sec,
                 (int)((nowTime.tv_usec * 1000)));
}

#endif /* IMS */


