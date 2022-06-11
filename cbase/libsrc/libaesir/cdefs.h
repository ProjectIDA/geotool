/*
 * SccsId:  @(#)cdefs.h	103.1	29 Mar 1995
 */

#ifndef _CDEFS_H_
#define _CDEFS_H_	1

/*
 *  The following macros shall be avoided and may no longer be used with ANSI
 *  and non-ANSI compilers.
 *
 *  30/06/2010: ANSI C COMPILER IS NOW REQUIRED!
 *
 */

/*
 *  UsePrototypes - macro for checking for strict C/C++ function prototype 
 *                  checking.
 *
 *  Stop using this macro!
 *
 *  30/06/2010: ANSI C NOW ENFORCED!
 *
 */

#ifndef UsePrototypes
# define UsePrototypes 1
#endif /* !UsePrototypes */


/*
 *  Proto - macro for defining ANSI C style prototypes with ANSI & non-ANSI 
 *          compilers.
 *
 *  STOP USING Proto as soon as possible and define proper ANSI C style
 *  prototypes.
 *
 *  30/06/2010: ANSI C NOW ENFORCED!
 *
 */
#ifndef Proto
# define Proto(type,name,args) type name args
#endif /* !Proto */

#ifndef UseVarargsPrototypes
# define UseVarargsPrototypes 1
#endif /* !UseVarargsPrototypes */


/*
 *  VA_START - macro to use rather than va_start().  This 
 *             will allow code to work under either ANSI C 
 *             or K&R C.  The second argument is the last 
 *	       formal argument to the function prior to the 
 *	       start of the varargs list.
 */
#include <stdarg.h>
#define VA_START(a,b) va_start(a,b)



/*
 *  CONST, VOLATILE & SIGNED are all supplied to handle special 
 *  argument handling for ANSI C.
 *
 *  STOP USING these macros as soon as possible and use proper ANSI C style
 *  keywords.
 *
 *  30/06/2010: ANSI C NOW ENFORCED!
 */

#ifndef CONST
# define CONST const
#endif /* !CONST */

#ifndef VOLATILE
# define VOLATILE volatile
#endif /* !VOLATILE */

#ifndef SIGNED
# define SIGNED signed
#endif /* !SIGNED */

#endif /*_CDEFS_H_ */
