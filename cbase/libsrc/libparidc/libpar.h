/*
 *  libpar.h include file.
 *
 */
    
#ifndef LIBPAR_H
#define LIBPAR_H

#define GETPAR_ERROR	100	/* exit status for getpar error */
#define GETPAR_STOP	101	/* exit status for STOP or mstpar */
extern int	libpar_errno;	/* exported error code; equal to 0,
				   GETPAR_ERROR, or GETPAR_STOP */
#define LIBPAR_BUFSIZ   8192    /* size of string buffers within libpar */

int	countarg (const char *name, const char *type);
void	endarg   (void);
void	endpar   (void);
int	getarg   (const char *name, const char *type, void *ptr_to_some_type);
int	getpar   (const char *name, const char *type, 
                          /*@out@*/void *ptr_to_some_type);
int	lenarg   (const char *name);
int	mstpar   (const char *name, const char *type, void *ptr_to_some_type);
void	setarg   (const char *list, const char *subname);
int	setpar   (const int argc, const char **av);
char   *getspar  (const char *name, const char *def);
char   *mstspar  (const char *name);
char   *getsarg  (const char *name, const char *def);
int     getbpar  (const char *parname, const int defvalue);
int     getdpar  (const char *parname, const int defvalue);
float   getfpar  (const char *parname, const double defvalue);
double  getffpar (const char *parname, const double defvalue);

/* The next functions are added for the CD receiver */
int setfpar (const char *progname, const char *name);
int getparline (int line, /*@out@*/ char **name, /*@out@*/ char **value);
int getparlistlen (void);
/* End added functions for CD receiver */

#endif	/* LIBPAR_H */
