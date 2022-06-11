/*
 * NAME
 *	do_var_write	- perform a variable length write of a string,
 *			  no blocking.
 *	do_write	- write to a socket without blocking.
 * SYNOPSIS
 *	do_var_write()	- writes a strings to a socket.
 *	do_write()	- writes a fixed amount of data to a socket.
 * DESCRIPTION
 *	do_var_write (socket, sendbuf)
 *	int	socket;		(i) the file descriptor to write to
 *	Mbuf	*sendbuf;	(i/o) message being written.
 *
 *	For the first call, sendbuf->m_buf should be initialized to the
 *	string to be sent, sendbuf->m_len == 0.  do_var_write() changes
 *	sendbuf->m_len to be the length of the string and sendbuf->m_done
 *	to be the number of bytes actually written.  Uses do_write to actually
 *	do the writing.
 *
 *	do_write (socket, sendbuf)
 *	int	socket;		(i) the file descriptor to write to
 *	Mbuf	*sendbuf;	(i/o) message being written.
 *
 *	Write data to a socket being sure not to block while doing it.
 *	senduf->m_done is changed to reflect the amount of data
 *	actually written.  The data can be binary data.  It returns
 *	ERR if the write fails; 0 if unable to finish writing the
 *	entire message; and the amount written if it is finished (i.e.
 *	> 0).
 * DIAGNOSTICS
 * AUTHOR
 *	Pete Ware	15 Mar 1988
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<errno.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	"aesir.h"
#include	"dispatcher.h"

extern int     errno;

int
do_var_write (socket, sendbuf)
int	socket;			/* (i) the file descriptor to write to */
Mbuf	*sendbuf;		/* (i/o) message being written */
{
	Mess_len	length;	/* how long the message is */
	int		status;	/* how the write fared */

	/* check the arguments */
	if (sendbuf == NULL)
	{
		errno = EINVAL;
		return ERR;
	}
	/* see if we need to send the length of the data */
	if (sendbuf->m_needlen == TRUE)
	{
		length = NET_LENGTH (sendbuf->m_len);
		status = write (socket, (char *) &length, D_LSIZE);
		if (status < 0 && errno ==  EWOULDBLOCK)
			return OK;
		else if (status < 0)
			return ERR;
		else if (status != D_LSIZE)
			return OK;
		else
		{
			sendbuf->m_done = 0;
			sendbuf->m_needlen = FALSE;
		}
	}
	return (do_write (socket, sendbuf));
}

int
do_write (socket, sendbuf)
int	socket;			/* (i) the file descriptor to write to */
Mbuf	*sendbuf;		/* (i/o) message being written */
{
	int	amount;		/* amount to write */
	int	length;		/* bytes actually written */

	/* check the arguments */
	if (socket < 0 || sendbuf == NULL || sendbuf->m_len <= 0)
	{
		errno = EINVAL;
		return ERR;
	}
	if (sendbuf->m_done < 0)
		sendbuf->m_done = 0;

	amount = sendbuf->m_len  - sendbuf->m_done;
	if (amount > MAX_SOCKETWRITE)
		amount = MAX_SOCKETWRITE;
	if (amount < 0)
		amount = 0;
	length = write (socket, &sendbuf->m_buf[sendbuf->m_done],
			(size_t) amount);
	if (length < 0 && errno == EWOULDBLOCK)
		return OK;
	else if (length < 0)
		return ERR;
	else
		sendbuf->m_done += length;
	return (sendbuf->m_done >= sendbuf->m_len ? sendbuf->m_len : OK);
}

		
			
		
