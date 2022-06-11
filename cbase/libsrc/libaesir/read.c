/*
 * NAME
 *	do_var_read	- perform a variable length read of a string,
 *			  no blocking.
 *	do_read		- read from a socket without blocking.
 * SYNOPSIS
 *	do_var_read()	- reads a string from a socket.
 *	do_read()	- reads a fixed amount of data from a socket.
 * DESCRIPTION
 *	do_var_read (socket, getbuf)
 *	int	socket;		(i) the file descriptor to read from
 *	Mbuf	*getbuf;	(i/o) message being written.
 *
 *	For the first call getbuf->m_len == 0.  do_var_read() changes
 *	getbuf->m_len to be the length of the string as read from the
 *	socket.  getbuf->m_done is set the number of bytes actually
 *	read.  Uses do_read to actually do the writing.  Terminates
 *	data with a EOS (no guarantee it is an ascii string or that
 *	there is not another EOS in the string.
 *
 *	do_read (socket, getbuf)
 *	int	socket;		(i) the file descriptor to read from
 *	Mbuf	*getbuf;	(i/o) message being written.
 *
 *	Read data to a socket being sure not to block while doing it.
 *	senduf->m_done is changed to reflect the amount of data
 *	actually read.  The data can be binary data.  It returns
 *	ERR if the read fails; 0 if unable to finish reading the
 *	entire message; and the amount read if it is finished (i.e.
 *	> 0).
 * NOTES
 *	Does not verify that the message length is reasonable.
 * AUTHOR
 *	Pete Ware	15 Mar 1988
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>
#include	<unistd.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<netinet/in.h>
#include	"aesir.h"
#include        "libaesir.h"
#include	"dispatcher.h"

extern int     errno;

int
do_var_read (socket, getbuf)
int	socket;			/* (i) the file descriptor to read from */
Mbuf	*getbuf;		/* (i/o) message being written */
{
	int	length;		/* how much was read */

	if (getbuf == NULL)
	{
		errno = EINVAL;
		return ERR;
	}

	/* See how much data the client is planning on sending. */
	if (getbuf->m_needlen == TRUE)
	{
		length = read (socket, (char *) &getbuf->m_len, D_LSIZE);
		if (length < 0)
		{
			if (errno == EWOULDBLOCK)
				return OK;
			else
				return ERR;
		}
		else if (length == 0)
		{
			/*
			 * check if connection is closed by seeing of
			 * select() indicates there is data and read returns
			 * none.  Seems to be the only way of telling when
			 * SYSV and BSD meet
			 */
			if (ok_read (socket))
			{
				length = read (socket, (char *) &getbuf->m_len,
					       D_LSIZE);
				if (length <= 0)
					return ERR;
			}
			else
			{
				return OK;
			}
		}
		if (length != D_LSIZE)
		{	/* unable to read whole length. TROUBLE! */
			/*
			 * NOTE:
			 *	With new scheme of using m_needlen flag
			 *	this could be handled.
			 */
			fprintf (stderr, "do_var_read: unable to read the message length");
			return ERR;
		}
		else
		{	/* succesfully got the length. */
			getbuf->m_len = HOST_LENGTH(getbuf->m_len);
			getbuf->m_done = 0;
			getbuf->m_needlen = FALSE;
		}
	}
	return do_read (socket, getbuf);
}

int
do_read (socket, getbuf)
int	socket;			/* (i) the file descriptor to read */
Mbuf	*getbuf;		/* (i/o) message being written */
{
	int	length;		/* the amount of data read */
	int	amount;		/* bytes to read */

	/* check the arguments */
	if (socket < 0 || getbuf == NULL || getbuf->m_len <= 0)
	{
		errno = EINVAL;
		return ERR;
	}
	if (getbuf->m_done < 0)
		getbuf->m_done = 0;
	amount = getbuf->m_len  - getbuf->m_done;
	if (amount > MAX_SOCKETWRITE) /* a misnomer */
		amount = MAX_SOCKETWRITE;
	/*
	 * NOTE:
	 *	It's impossible to read zero bytes and return success!
	 */

	length = read (socket, &getbuf->m_buf[getbuf->m_done],
		       (size_t) amount);
	if (length < 0)
	{
		if (errno == EWOULDBLOCK)
			return OK;	/* BSD style no data available */
		else
			return ERR;
	}
	else if (length == 0)
	{
		/*
		 * check if connection is closed by seeing if
		 * select() indicates there is data and read returns
		 * none.  Seems to be the only way of telling when
		 * SYSV and BSD meet
		 */
		if (ok_read (socket))
		{
			length = read (socket, (char *) &getbuf->m_len,
				       D_LSIZE);
			if (length <= 0)
				return ERR;
		}
		else
		{
			return OK;
		}
	}
	getbuf->m_done += length;
	length = (getbuf->m_done >= getbuf->m_len ? getbuf->m_len : OK);
	return length;
}

		
/*******************  endian_revert ***************************************/
char    *buff;                  /* input buffer containing data */
int     numwds;                 /* number of words of length len in input buffer */
double  len;                    /* length of one word
The function reverts the order of every len words in the input buffer in place. 
Intended for supporting casting of input data on little endian platforms. 
*/
int
endian_revert(buff, numwds, len)
     char *buff;
     int numwds;
     int len;
{
   int i;
   int j;
   char *temp;

   temp = (char *)malloc(len * sizeof(char));

   for (i=0; i <= numwds*len - len; i+=len) {
      /* revert order of len buff elements starting from pos. i. Write into temp.*/
      for (j=0; j<len; j++) {
         temp[j] = buff[i+len-1-j];
      }
      /* copy the reverted bytes back into buff */
      for (j=0; j<len; j++) {
         buff[i+j] = temp[j];
      }
   }
   return 0;
}
