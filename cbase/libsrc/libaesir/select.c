/*
 * Name
 *	ok_write - see if it is safe to write to a file descriptor
 *	ok_read - see if it is ok to read from a file descriptor
 *	ok_except - see if there are any exceptional conditions on a descriptor
 *	ok_read_mask - return a bitmask of descriptors ready for reading
 * SYNOPSIS
 *	These routines provide a simpler interface to 4.2bsd's select(2)
 *	system call.  They are also intended to avoid some small
 *	guard against portablility problems.
 * DESCRIPTION
 *	ok_write (fd, size)
 *	int	fd	(i) the file descriptor
 *	int	size	(i) the number of bytes to write
 *
 *	ok_read (fd)
 *	int	fd	(i) the file descriptor
 *
 *	ok_except (fd)
 *	int	fd	(i) the file descriptor
 *
 *	ok_read_mask (mask)
 *	Mask	*mask	(i/o) returned mask of file descriptors ready for
 *			reading.
 * DIAGNOSTICS
 *	Returns TRUE if it is ok the asked for condition is true on the
 *	indicated file descriptor (or one of the file descriptors in the
 *	mask).  Returns FALSE if the operation would block or there was
 *	an error.  If there are no errors and the operation would block,
 *	then errno is set to 0.
 *
 *	Note that ok_read_mask() modifies the past in mask.
 * NOTES
 *	Should probably provide some mechanism for changing the delay.  It
 *	is currently set to 100000 microseconds (1/10 second).  I should
 *	also implement ok_(write,except)_mask () routines.

 *	Pete Ware	15 Mar 1988
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	"aesir.h"
#include	"mask.h"
#include	"dispatcher.h"

static struct timeval waittime = {0, 50000};

int
ok_write (fd, size)
int	fd;			/* (i) the file descriptor */
int	size;			/* (i) amount to write (unused) */
{
	Mask	mask;
	int	status;

	if (fd < 0 || size < 0)
	{
		errno = EINVAL;
		return FALSE;
	}

	mask = make_mask (fd + 1);
	set_bit (mask, fd);
	errno = 0;
	status = select (sel_size (mask), (fd_set *) NULL, 
			 (fd_set *) sel_mask (mask), (fd_set *) NULL, 
			 &waittime);
	free_mask (mask);
	return (status > 0 ? TRUE : FALSE);
}

int
ok_read (fd)
int	fd;			/* (i) the file descriptor */
{
	Mask	mask;
	int	status;

	if (fd < 0)
	{
		errno = EINVAL;
		return FALSE;
	}

	mask = make_mask (fd + 1);
	set_bit (mask, fd);
	errno = 0;
	status = select (sel_size (mask), (fd_set *) sel_mask (mask), 
			 (fd_set *) NULL, (fd_set *) NULL, &waittime);
	free_mask (mask);
	return (status > 0 ? TRUE : FALSE);
}

int
ok_except (fd)
int	fd;			/* (i) the file descriptor */
{
	Mask	mask;
	int	status;

	if (fd < 0)
	{
		errno = EINVAL;
		return FALSE;
	}

	mask = make_mask (fd + 1);
	set_bit (mask, fd);
	errno = 0;
	status = select (sel_size (mask), (fd_set *) NULL, (fd_set *) NULL,
			 (fd_set *) sel_mask (mask), &waittime);
	free_mask (mask);
	return (status > 0 ? TRUE : FALSE);
}

int
ok_read_mask (mask)
Mask	mask;		/* (i/o) the mask to test and returned set of ready
			   descriptros */
{
	int	status;

	if (!mask)
	{
		errno = EINVAL;
		return FALSE;
	}
	errno = 0;
	status = select (sel_size (mask), (fd_set *) sel_mask (mask), 
			 (fd_set *) NULL, (fd_set *) NULL, &waittime);
	return (status > 0 ? TRUE : FALSE);
}

	
