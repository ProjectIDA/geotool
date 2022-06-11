#ifndef _IDCSYSLOG_H
#define _IDCSYSLOG_H

/**
 * @author    Gerald Klinkl, Siemens Inc. Austria
 * @version   $Revision: 1.5 $
 * @date      $Date: 2005/06/03 12:30:15 $
 */

#include <syslog.h>

/* ==== Some defines and macros ================================ */

#define LOGSIZE         196     /* Don't set LOGSIZE to high. The Program can
                                   run in a multithreaded environment and
                                   stack size is limited there  */

#define DEBUG_FP  stderr
#define IF_DEBUG(level)        if (getDebugLevel() >= level)
#define DEBUG_HDR    (void)fprintf(DEBUG_FP,"(Debug:%.2hu)", getDebugLevel())
#define DEBUG(level,msg)       IF_DEBUG(level)\
        { DEBUG_HDR;\
          (void)fprintf(DEBUG_FP,"%s\n", msg); (void)fflush(DEBUG_FP);}
#define DEBUG2(level,fmt,p1)   IF_DEBUG(level)\
        { DEBUG_HDR;\
          (void)fprintf(DEBUG_FP,fmt, p1); (void)fprintf(DEBUG_FP,"\n");\
          (void)fflush(DEBUG_FP);}
#define DEBUG3(level,fmt,p1, p2)  IF_DEBUG(level)\
        { DEBUG_HDR;\
          (void)fprintf(DEBUG_FP,fmt, p1, p2); (void)fprintf(DEBUG_FP,"\n");\
          (void)fflush(DEBUG_FP);}
#define DEBUG4(level,fmt,p1, p2, p3)  IF_DEBUG(level)\
        { DEBUG_HDR;\
          (void)fprintf(DEBUG_FP,fmt, p1, p2, p3);\
          (void)fprintf(DEBUG_FP,"\n");\
          (void)fflush(DEBUG_FP);}
#define DEBUG5(level,fmt,p1, p2, p3, p4)  IF_DEBUG(level)\
        { DEBUG_HDR;\
          (void)fprintf(DEBUG_FP,fmt, p1, p2, p3, p4);\
          (void)fprintf(DEBUG_FP,"\n");\
          (void)fflush(DEBUG_FP);}
#define DEBUG6(level,fmt,p1, p2, p3, p4, p5)  IF_DEBUG(level)\
        { DEBUG_HDR;\
          (void)fprintf(DEBUG_FP,fmt, p1, p2, p3, p4, p5);\
          (void)fprintf(DEBUG_FP,"\n");\
          (void)fflush(DEBUG_FP);}
#define LOG2STDOUT(p1) (void)printf("%s\n", p1);\
          (void)fflush(stdout);
#define LOG2STDOUT2(fmt,p1) (void)printf(fmt, p1); (void)printf("\n");\
          (void)fflush(stdout);

/* ==== Prototypes ========================================================== */

extern void setDebugLevel(unsigned short newDebugLevel);
extern unsigned short getDebugLevel();
extern void setSyslogFilterLevel(unsigned short newSyslogLevel);
extern unsigned short getSyslogFilterLevel();
extern void logInit (char *appName, unsigned int startPriority);
extern void longLogWrite(int logLevel, const char *msgToLog );
extern void logWrite ( int logLevel, const char *msgToLog );
extern void logWarning(const char *message, const char *detail);
extern void logError(const char *message, /*@null@*/const char *detail);
extern void log2Stdout( char *msgToLog);

#ifdef IMS
extern void printTimeStamp(unsigned short debugLevel);

/* Rewrite DEBUG_HDR macro */
#undef DEBUG_HDR
#define DEBUG_HDR    printTimeStamp(getDebugLevel())

#endif /* IMS */

#endif /* _IDCSYSLOG_H */
