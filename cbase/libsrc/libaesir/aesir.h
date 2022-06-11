/*
 * SccsId:  @(#)aesir.h	105.1	19 May 1995
 */

#ifndef _AESIR_H_
#define _AESIR_H_	1

#include <stdlib.h>
#include <string.h>

#include "cdefs.h"

#define AESIR_VERSION		"3.2"	/* string for printing */

#ifdef Bool
#undef Bool
#endif
typedef int	Bool;

#ifndef NULL
#define NULL	(void *)0
#endif

#ifndef FALSE
#define FALSE		0
#define TRUE		1
#endif

#ifndef False
#define False		0
#define True		1
#endif

#define ERR		-1		/* error return value */
#define OK		0		/* ok return value */

#define REG		register
#define EOS		'\0'		/* end of string */
#define EOL		'\n'		/* end of line */
#define EOP		'\14'		/* end of page (form feed) */

#define FILENAMELEN	255		/* longest file name */
#define	MAXHOSTNAME	256
#define MAXPATH		FILENAMELEN	/* longest file name */
#define NBITS		8		/* number of bits in a byte */
#define BUFLENGTH	256		/* convenient size for getting input */
#define	ID_SIZE		32
#define MAX_MESSSIZE	1024
#define	PPOID_SIZE	32
#define LPOID_SIZE	PPOID_SIZE
#define LONGTIME	(60*60*24*7)	/* seven days */


/*
 * The following make the use of malloc much easier.  You generally want
 * to use UALLOC() to dynamically allocate space and UFREE() to free
 * the space up.
 *
 * UALLOC	- Allocate permanent memory.
 * UCALLOC	- Allocate permanent memory, and initalize to zero.
 * UALLOCA	- Allocate temporary memory that is automatically free'd
 *		  when calling procedure returns.
 * UREALLOC	- Change the size of allocated memory from a previous
 *		  UALLOC() call (only on permanent memory).
 * STRALLOC	- Returns a pointer to the string copied into permanent
 *		  memory.
 * STRALLOCA	- Same as STRALLOC except uses temporary memory.
 * UFREE	- Free permanent memory and set pointer to NULL.
 */

#ifndef UALLOC
#define UALLOC(type, count)	(type *) malloc ((unsigned) (count) * \
						              sizeof (type))
#endif

#ifndef UCALLOC
#define UCALLOC(type, count)    (type *) calloc ( (unsigned) (count), sizeof (type))
#endif

#ifndef UALLOCA
#define UALLOCA(type, count)	(type *) alloca ((count) * sizeof (type))
#endif

/*
 *  Please excuse this ternary statement but it allows us to use REALLOC
 *  even if ptr is NULL.
 */
#ifndef UREALLOC
#define UREALLOC(ptr,type,count) ( (ptr) ? \
				    (type *) realloc ((char *)ptr, \
						       (unsigned) sizeof (type) \
						       * (count))\
				  : \
				    UALLOC(type,(count)) \
				 )
#endif

#ifndef STRALLOC
#define STRALLOC(string)	strcpy (UALLOC (char, strlen (string)+1), string)
#endif

#ifndef STRALLOCA
#define STRALLOCA(string)	strcpy (UALLOCA (char, strlen (string)+1), string)
#endif

/*
 *  Common use would be MALLOC_CHECK ( ptr = UALLOC( float, numOfPoints));
 */
#ifndef MALLOC_CHECK
#define MALLOC_CHECK(a)	if ( ! (a)) \
                        { \
			 fprintf (stderr, "malloc() error in %s:\n", fname); \
			 exit (-1); \
		        }
#endif

	/*
	 *  This sort of wierdness is so it becomes one statement.
	 */
#ifndef UFREE
#define UFREE(ptr)		do \
				{	if (ptr)\
					{	(void) free ((char *) (ptr));\
						(ptr) = NULL;\
					} \
				} while (0)
#endif

/* Return the number of elements in an array. */
#ifndef DIM
#define DIM(ar)		(sizeof (ar) / sizeof (*(ar)))
#endif

/* Are two strings equal? */
#ifndef STREQ
#define STREQ(a,b)		(strcmp ((a), (b)) == 0)
#endif

#if 0
#ifdef DO_DEBUG

/*
 * Provide varying levels of debug information.  You should define your
 * the areas you are interested in debugging (depends on the application
 * what those areas may be) up to a maximum of MAX_DEBUG.  Within each of
 * the areas of interest, you can various levels of debugging information.
 *
 * Typically invoked as:
 *	DEBUG(BUG_INIT, 3, ("Opening file %s\n", filename));
 * and if you have set aDebug[BUG_INIT] >= 3, you will see
 *	"Opening file foo"
 * printed on the screen.  If you set aDebug[BUG_INIT] = 0, you won't
 * see the message.  If you do not define DO_DEBUG, then the debugging
 * statements are not compiled into the code.
 */

#define DEBUG(what,lev,mess)	do {if (aDebug[what] >= lev) printf mess;fflush (stdout);} while(0)
#define MAX_DEBUG	4
extern int		aDebug[MAX_DEBUG+1];
#else
#define DEBUG(lev,what,mess)
#endif	/* DO_DEBUG */
#endif  /* 0 */
#endif  /*_AESIR_H_ */
