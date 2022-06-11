/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include        <stdlib.h>
#include	<stdio.h>
#include        <string.h>
#include	<errno.h>
#include	<sys/types.h>

#include	"libaesir.h"

static int      (*message_func)(caddr_t msg_data, char * buf);
static caddr_t	message_data;
static int	(*info_func)();
static caddr_t	info_data;

void
MessageInit (func, to_private)
int		(*func) ();		/* function to call for messages */
caddr_t		to_private;		/* private data to call */
{
	message_func = func;
	message_data = to_private;
}

void
InfoInit (func, to_private)
int		(*func) ();		/* function to call for messages */
caddr_t		to_private;		/* private data to call */
{
	info_func = func;
	info_data = to_private;
}

void
Werror (string)
char	*string;
{
	char	buf[BUFSIZ];

        int errnum = errno;
	if (errno > 0) {
		strcpy (buf, strerror(errno));
	}
	else {
		sprintf (buf, "Unknown error (%d)", errno);
	}
	Warning ("%s:%s\n", string?string:"", buf);

} /* End Werror. */


#if UseVarargsPrototypes
/*VARARGS1*/
void
Warning (char *fmt, ...)
#else
/*VARARGS0*/
void
Warning (fmt, va_alist)
char	*fmt;
va_dcl			/* No, I didn't forget the arg name or semi-colon! */
#endif /* UseVarargsPrototypes */
{
	va_list 	args;	/* Points to the variable arguments. */
	char		buf[BUFSIZ*2];
	static int	reporting_error = 0;

	va_start(args, fmt);
	if (reporting_error)
	{
		va_end (args);
		return;
	}
	else
		reporting_error = 1;

	(void) vsprintf (buf, fmt, args);
	if (!message_func)
		fprintf (stderr, "%s", buf);
	else
		(*message_func) (message_data, buf);
	reporting_error = 0;
	va_end (args);

} /* End Warning. */


#if UseVarargsPrototypes
/*VARARGS1*/
void
Info (char *fmt, ...)
#else
/*VARARGS0*/
void
Info  (fmt, va_alist)
char	*fmt;
va_dcl			/* No, I didn't forget the arg name or semi-colon! */
#endif /* UseVarargsPrototypes */
{
	va_list 	args;
	char		buf[BUFSIZ*2];
	static int	reporting_error = 0;

	VA_START (args, fmt);
	if (reporting_error)
	{
		va_end (args);
		return;
	}
	else
		reporting_error = 1;

	(void) vsprintf (buf, fmt, args);
	if (!message_func)
		fprintf (stderr, "%s", buf);
	else
		(*message_func) (message_data, buf);
	reporting_error = 0;
	va_end (args);
}


#ifdef UseVarargsPrototypes
/*VARARGS1*/
void
Fatal (char *fmt, ...)
#else
/*VARARGS0*/
void
Fatal  (fmt, va_alist)
char	*fmt;
va_dcl			/* No, I didn't forget the arg name or semi-colon! */
#endif /* UseVarargsPrototypes */
{
	va_list 	args;
	char		buf[BUFSIZ*2];
	static int	reporting_error = 0;

	va_start(args, fmt);
	if (reporting_error)
	{
		va_end (args);
		return;
	}
	else
		reporting_error = 1;

	(void) vsprintf (buf, fmt, args);
	fprintf (stderr, "%s", buf);
	reporting_error = 0;
	va_end (args);
	exit (1);
}
