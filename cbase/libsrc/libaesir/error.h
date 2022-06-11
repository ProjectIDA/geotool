/*
SccsId: @(#)libaesir/error.h	109.1	07/08/97
*/

#ifndef _ERROR_H_
#define _ERROR_H_

# include	<ctype.h>
# include	<signal.h>
# include	<stdio.h>

# include	"aesir.h"


#define	INFO		5	/* Informative message only. */
#define	WARNING		10	/* Warning, but with good chance of success. */
#define	SEVERE		50	/* Non-fatal, but little chance of success. */
#define	FATAL		90	/* Fatal, must exit immediately. */

#define	STR_LEN	512		/* Maximum length for strings. */

struct	error
{	int	Xerrno;		/* Error number as in errno.h. */
	int	Xseverity;	/* Severity code as defined above. */
	char	Xname[STR_LEN];	/* Name of routine in which error occurred. */
	char	Xmess[STR_LEN];	/* Additional information about the error. */
};

typedef	struct	error	ERROR;

/*  error return codes */

#define ERR_ERROR             -1
#define ERR_NOERROR            0



/*  Externally Visible Function Prototypes  */

extern int error (int number, int severity, char *name, char *mess);
extern int error_ (int *number_p, int *severity_p, char *name, char *mess,
                   int name_len, int mess_len);

/*
 *  We seperated these out because gcc's preprocessor will not allow 
 *  preprocessor directives inside a macro (Wahl & Wass).
 */
#ifdef __svr4__
int init_error (char *program, void (*handler)(int signo));
#else
extern int init_error (char *program, void (*handler)(int signo, int code,
                                                      struct sigcontext *scp,
                                                      char *addr));
#endif

#ifdef __svr4__
extern int initerror_ (char *program, void (*handler)(int signo),
                       int program_len);
#else
extern int initerror_ (char *program, void (*handler)(int signo, int code,
                                                      struct sigcontext *scp,
                                                      char *addr),
                       int program_len);
#endif

#endif /* _ERROR_H_ */


