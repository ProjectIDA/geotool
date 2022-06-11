/*
 * FILE NAME  expand_log_keys.c
 *
 *
 *         			    Copyright 1993
 *		Science Applications International Corporation (SAIC),
 *				 All Rights Reserved
 *
 * SYNOPSIS
 *	int
 *	expand_log_keys( char *src, char *dest, time_t etime )
 *      src			(i) string possibly containing keys
 *	dest			(o) string after expansion of keys
 *	etime			(i) epoch time for jdate computation
 *
 *	int
 *	is_jdate_key( char *s )
 *	s			(i) string to search for jdate key
 *
 *	int
 *	is_host_key( char *s )
 *	s			(i) string to search for host key
 *
 *	int
 *	is_pid_key( char *s )
 *	s			(i) string to search for pid key
 *
 * DESCRIPTION
 *	expand_log_keys() expands any "keys" in the input string
 *	during a copy of the input string to the output string.
 *	The keys currently are:
 *	  %jdate	- expands to Julian date, eg, 1995012
 *	  %host		- expands to host machine, eg, hati
 *	  %pid		- expands to Unix pid, eg, 14599
 *	The function returns an integer count of how many keys
 *	were expanded.
 *
 *	is_jdate_key(), is_host_key() and is_pid_key() are boolean
 *	functions that indicate whether the particular key is
 *	found in the input string.
 *
 * DIAGNOSTICS
 *	For expand_log_keys(), the caller must supply a buffer large
 *	to hold the input string plus any expansions.
 *
 * FILES
 *
 * NOTES
 *	The function expand_log_keys() allows programs to create
 * 	logfile names that embed the Julian date, host and/or pid.
 *	Using libpar, an example logfile specification might be
 *		log=$(logdir)/program-%jdate.log
 *	If logdir was set to /data/logs, the final log filename would be like
 *	/data/logs/program-1995012.log
 *
 *	These functions are coded for Solaris 2/ANSI C.
 *
 * SEE ALSO
 *
 *
 * AUTHOR/DATE(fox) 10:29:51 01/12/95
 *
 *
 *
 * MODIFICATIONS
 *
 *
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include "libaesir.h"
#include "libstdtime.h"

#define JDATE_KEY	"%jdate"
#define	HOST_KEY	"%host"
#define PID_KEY		"%pid"

/*
 * expand_log_keys
 *	This function expands a string (typically a logfile name)
 *	by replacing special keys with julian date (computed from input),
 *	host machine name, and unix process id.  All occurrences
 *	of the keys are replaced while the remaining text is unchanged.
 *
 *	key		replaced with	example
 *	---		-------------	-------
 *	%jdate		YYYYDDD		1994355
 *	%host		hostname	elivagar
 *	%pid		unix pid	18422
 *
 *	The function returns an int signifying the number of
 *	expansions that took place.  The function returns -1
 *	on input error.
 *
 *	Warning, the caller must provide a buffer of sufficient
 *	size to hold the expanded string.
 */
int
expand_log_keys( char *src, char *dest, time_t etime )
{
	char	*p;		/* pointer to key in src */
	char	tail[200];	/* temp hold for src after key */
	int	jdate=0;	/* integer YYYYDDD */
	pid_t	pid;		/* unix process id */
	char	hostname[257];	/* machine name */
	int	cnt=0;		/* count of expansions */
     	struct utsname n_struct;    /* System name structure for hostname */

	/* if bogus input */
	if ( (src == (char *) NULL) || (dest == (char *) NULL) ) return -1;

	strcpy( dest, src );

	/*
	 * General algorithm:
	 * 1) find position of key
	 * 2) copy tail of input (after key) to temp storage
	 * 3) compute replacement value for key
	 * 4) overwrite key with replacement value
	 * 5) tack on tail to end of replacement value
	 * 6) repeat for all occurrences of all keys
	 * 7) return number of replacements
	 */

	/* replace all occurrences of JDATE_KEY */
	while ( (p = strstr( dest, JDATE_KEY )) != NULL )
	{
		strcpy( tail, p+strlen(JDATE_KEY) );
		if ( jdate == 0 )	/* first time thru loop */
		{
			jdate = stdtime_etoj ((epoch_t) etime);
		}
		sprintf( p, "%d", jdate );
		strcat( p, tail );
		cnt++;
	}

	/* replace all occurrences of HOST_KEY */
        hostname[0] = '\0';
	while ( (p = strstr( dest, HOST_KEY )) != NULL )
	{
		strcpy( tail, p+strlen(HOST_KEY) );
		if ( uname( (struct utsname *) &n_struct ) < 0)
		{
			strcpy( hostname, "localhost" );      /* fallback hostname */
		}
		else
		{
			strcpy( hostname, n_struct.nodename); /* Should be the hostname */
		}
		strcpy( p, hostname );
		strcat( p, tail );
		cnt++;
	}

	/* replace all occurrences of PID_KEY */
	while ( (p = strstr( dest, PID_KEY )) != NULL )
	{
		strcpy( tail, p+strlen(PID_KEY) );
		pid = getpid();
		sprintf( p, "%i", (int)pid );
		strcat( p, tail );
		cnt++;
	}
	return cnt;
}

/*
 * is_jdate_key
 * is_host_key
 * is_pid_key
 *	These functions returns a boolean indicating whether the
 *	specified key is in the string.
 */
int
is_jdate_key( char *s )
{
	if ( s )
		return ( strstr(s, JDATE_KEY) != NULL );
	else
		return 0;
}
int
is_host_key( char *s )
{
	if ( s )
		return ( strstr(s, HOST_KEY) != NULL );
	else
		return 0;
}
int
is_pid_key( char *s )
{
	if ( s )
		return ( strstr(s, PID_KEY) != NULL );
	else
		return 0;
}


