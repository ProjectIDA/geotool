
/*
 * Copyright 1990 Science Applications International Corporation.
 */
	
/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<errno.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"aesir.h"
#include	"error.h"

	/*
	 *  Local forward declaration. 
	 */
#ifdef __svr4__
static void bail (int signo);
#else
static void bail (int signo, int code, struct sigcontext *scp, char *addr);
#endif /* __svr4__ */

static void setup_signals (void);

static int trim (char *str, int len);


#if defined(alliant) || defined(tektronix)	/* Alliant errno.h doesn't define errno. */
extern	int	errno;
#endif

#ifndef HAVE_SYSERRLIST
extern	int	sys_nerr;	/* Number of known error messages. */
extern   char    *sys_errlist[]; /* Vector to error message strings. */
#endif /* HAVE_SYSERRLIST */

static	int	n_init = 0;	/* Number of times init_error() been called. */
static	ERROR	err;		/* The error structure. */

				/* Name of main program. */
static	char	program_name[STR_LEN] = "WhoAmI";

				/* User's handling routine. */
#define	MAX_HANDLERS	20	/* Maximum number of user handlers. */
static	void		(*user_handler[MAX_HANDLERS])();
static	int	n_handlers = 0;	/* Number of specified handlers. */

/*
 * error()
 * IMS error handling facility.
 * Basic idea is similar to that of errno, only with a structure
 * so more information can be retained. 
 *
 * Each time error() is called, the error is reported to
	 * standard error.  Additional functionality to log to 
	 * a centralized location is planned for IMS
	 *
	 * Thus the procedure is:
	 *
	 * check all input parameters
	 * decode severity
	 * look up character string for errno
	 *	print error message to standard error;
	 *      send error to central log
	 *	load new error into structure;
	 * return;
	 */

int
error(number, severity, name, mess)
int	number;
int	severity;
char	*name;
char	*mess;
{
	int	ret_val = ERR_NOERROR;
	char	severity_str[STR_LEN], report[STR_LEN];
	char	mess_str[STR_LEN];
	const char	*sys_errmsg;

	if (n_init < 1)
	{	fprintf(stderr, "error: Must call init_error first.\n");
		ret_val = ERR_ERROR;
		return(ret_val);
	}

		/* Determine string to print concerning severity. */
	if (severity == INFO)
		strcpy(severity_str, "INFO");
	if (severity == WARNING)
		strcpy(severity_str, "WARNING");
	else if (severity == SEVERE)
		strcpy(severity_str, "SEVERE");
	else if (severity == FATAL)
		strcpy(severity_str, "FATAL");
	else
		sprintf(severity_str, "Severity %0d", severity);

		/* Check against bogus input parameters. */
	if (name == (char *) NULL)
		name = "unknown routine";

	if (mess == (char *) NULL)
		mess = "no message";

		/* Find out the system error message, if any. */
        errno = 0;  /* Only way to find out if strerror fails */
        sprintf(mess_str, "%s", strerror(number));
	if (number < 0 || errno != 0) {
          sprintf(mess_str, "Errno %0d", number);
	}
        sys_errmsg = mess_str;

		/* Produce report message to print. */
	sprintf(report,
		"%-.30s reports %s error(%s) in \"%-.30s\":%-.230s.",
		program_name, severity_str, sys_errmsg, name, mess);

		/* Always print message to standard error */

	fprintf(stderr, "%s\n", report);

	/* Now save parameters. */
	
	err.Xerrno = number;
	err.Xseverity = severity;
	strncpy(err.Xname, name, STR_LEN);
	err.Xname[STR_LEN - 1] = '\0';
	strncpy(err.Xmess, mess, STR_LEN);
	err.Xmess[STR_LEN - 1] = '\0';

	return(ret_val);
}

	/*
	 * Set up signal vectors.
	 * When a fatal signal is received, branch to bail()
	 * which then calls error().
	 * Also, make initial contact with Manager.  Assumes that the
	 * calling program has already opened a connection.
	 * Should be called once at the start of execution with the
	 * name of the program.
	 */

#include <signal.h>

#ifndef	SIGABRT
#define	SIGABRT	SIGIOT		/* For OS 3.x; moving to SIGABRT in future. */
#endif

#if UsePrototypes
int
init_error(
	   char	*program, 
# ifdef __svr4__
	   void	(*handler)(int signo)
# else
	   void	(*handler)(int signo, 
			   int code, 
			   struct sigcontext *scp, 
			   char *addr)
# endif /* __svr4__ */
	   )
#else
int
init_error(program, handler)
char		*program;
void		(*handler)();
#endif /* UsePrototypes */
{
	int	i, return_value = ERR_NOERROR;

		/* All this stuff gets done only the first time. */
	if (n_init++ == 0)
	{
		strncpy(program_name, program, STR_LEN);
		program_name[STR_LEN - 1] = '\0';

		for (i = 0; i < MAX_HANDLERS; i++)
			user_handler[i] = (void (*)()) NULL;
		
		setup_signals();
	}

		/* Set up handler and signals. */
	if (handler != (void (*)()) NULL && n_handlers < MAX_HANDLERS)
		user_handler[n_handlers++] = handler;
	
	return(return_value);
}

static
void
setup_signals()
{
	(void) signal(SIGHUP, bail);	/* 1 */
	(void) signal(SIGQUIT, bail);	/* 3 */
	(void) signal(SIGILL, bail);	/* 4 */
	(void) signal(SIGTRAP, bail);	/* 5 */
	(void) signal(SIGABRT, bail);	/* 6 */
#ifdef SIGEMT
	(void) signal(SIGEMT, bail);	/* 7 */
#endif /* SIGEMT */
	(void) signal(SIGFPE, bail);	/* 8 */
	(void) signal(SIGBUS, bail);	/* 10 */
	(void) signal(SIGSEGV, bail);	/* 11 */
#ifdef SIGSYS
	(void) signal(SIGSYS, bail);	/* 12 */
#endif /* SIGSYS */
	(void) signal(SIGPIPE, bail);	/* 13 */
	(void) signal(SIGXCPU, bail);	/* 24 */
	(void) signal(SIGXFSZ, bail);	/* 25 */
#ifndef __svr4__
	(void) signal(SIGLOST, bail);	/* 29 */
#endif

	return;
}

	/*
	 * Signal catching routine.
	 */
# ifdef __svr4__
static
void bail (int signo)
# else /* __svr4__ */
static
void bail (int signo, 
           int code, 
           struct sigcontext *scp, 
           char	*addr
           )
# endif /* __svr4__ */
{
	static	char	boo[100];
	static	int	count;

#ifdef __svr4__
	sigset_t	set;
#endif

	(void) signal(signo, SIG_DFL);
	(void) signal(SIGABRT, SIG_DFL); 	/* So abort isn't caught. */

#ifdef __svr4__
	(void) sigprocmask((int) NULL, NULL, 	/* Get signal mask. */
			   &set);
	(void) sigprocmask(SIG_UNBLOCK, &set,	/* Unblock all signals. */
			   (sigset_t *) NULL);
#else
	(void) sigsetmask(0);			/* Unblock all signals. */
#endif

	(void) sprintf(boo, "Signal %0d - Bail() abort", signo);
	(void) error(errno, FATAL, program_name, boo);

		/*
		 * We call the handlers backwards, just like
		 * on_exit(3) does.  Why?  Go ask your Dad.
		 */
	for (count = n_handlers - 1; count >= 0; count--)
		if (user_handler[count] != (void (*)()) NULL)
		{
#ifdef __svr4__
			(void) (*user_handler[count])(signo);
#else
			(void) (*user_handler[count])(signo, code, scp, addr);
#endif
		}
	

	fflush(stdout);
	
	switch (signo)
	{
		case SIGQUIT:
		case SIGILL:
		case SIGTRAP:
		case SIGABRT:
#ifdef SIGEMT
		case SIGEMT:
#endif /* SIGEMT */
		case SIGFPE:
		case SIGBUS:
		case SIGSEGV:
#ifdef SIGSYS
		case SIGSYS:
#endif /* SIGSYS */
#ifndef __svr4__
		case SIGLOST:
#endif
			abort();
			exit(ERR_ERROR);
			break;		/* Don't really need this! */
		default:
			exit(ERR_ERROR);
			break;		/* Don't really need this! */
	}

	exit(ERR_ERROR);	/* Shouldn't get here !! */
}

	/*
	 * Fortran callable version.
	 */

int
error_(number_p, severity_p, name, mess, name_len, mess_len)
int	*number_p;
int	*severity_p;
char	*name;
char	*mess;
int	name_len;
int	mess_len;
{
	int	len, i;
	int	number, severity;
	char			my_name[STR_LEN], my_mess[STR_LEN];

	number = *number_p;
	severity = *severity_p;

	for ( i = 0; i < STR_LEN; i++)
	{	my_name[i] = ' ';
		my_mess[i] = ' ';
	}

	len = (name_len < STR_LEN - 1) ? name_len : STR_LEN - 1;
	strncpy(my_name, name, (size_t) len);
	trim(my_name, STR_LEN);

	len = (mess_len < STR_LEN - 1) ? mess_len : STR_LEN - 1;
	strncpy(my_mess, mess, (size_t) len);
	trim(my_mess, STR_LEN);

	return(error(number, severity, my_name, my_mess));
}

	/*
	 * Fortran callable init_error().
	 */
int
initerror_(char *program, 
# ifdef __svr4__
	   void	(*handler)(int signo), 
# else
	   void	(*handler)(int signo, 
			   int code, 
			   struct sigcontext *scp, 
			   char *addr),
# endif /* __svr4__ */
	   int	program_len
	   )
{
	int	len;
	char	my_program[STR_LEN];

	len = (program_len < STR_LEN) ? program_len : STR_LEN;
	strncpy(my_program, program, (size_t) len);
	my_program[STR_LEN - 1] = '\0';
	trim(my_program, len);

	return(init_error(my_program, handler));
}

	/*
	 * Trim trailing spaces from a string.
	 * Useful mainly for character strings passed into C
	 * from Fortran.
	 */
static
int
trim (char *str, int len)
{	
	int	i;

	for (i = len - 1; i > 0 && isspace(str[i]); i--)
		;		/* Null loop. */
	str[i + 1] = '\0';

	return(ERR_NOERROR);
}
