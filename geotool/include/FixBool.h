#ifndef _FIX_BOOL_H
#define _FIX_BOOL_H

/*
   when compiling with some versions of ODBC (like iODBC), there were
   conflicting types for `BOOL` between
   /usr/include/iodbcunix.h:136: conflicting types for `BOOL'
   /usr/X11R6/include/X11/Xmd.h:160: previous declaration of `BOOL'

   if we look in iodbcunix.h, there is the following construct:

#if !defined(BOOL) && !defined(_OBJC_OBJC_H_)
typedef int                     BOOL;
#endif

in order to avoid the BOOL conflict, _OBJC_OBJC_H_ is defined below.
 */
#define BYTE BYTE2
#define _OBJC_OBJC_H_

#endif
