/*
 * Copyright 1990 Science Applications International Corporation.
 */

/*
 * mask.c contains routines for manipulating bitmasks.
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>
#include	<errno.h>
#include	<stdlib.h>
#include	<memory.h>
#include	<string.h>
#include	"mask.h"

extern int     errno;

/*
 * make_mask()	- create a bitmask with the specified number
 *		  of bits.
 *
 * A mask is implemented as a array of ints.  The actual
 * size of the bitmask then is the smallest number of ints
 * that can contain the specified number of bits.
 */

Mask
make_mask (size)
	int		size;		/* number of bits */
{
	Mask		mask;		/* returned mask */
	unsigned int	unit_size;	/* number of bytes in mask */

	if (size <= 0) {
		return ((Mask) NULL);
	}

	mask = (Mask) malloc (sizeof (struct mask));

	if (mask != (Mask)NULL)
	{
		mask->size = size;
		unit_size = BIT_TO_UNIT(size-1);
		mask->bits = (int *) malloc (unit_size);
		if (mask->bits != (int *)NULL)
		{
			memset ((char *) mask->bits, 0, unit_size);
		}
	}
	return mask;
}

/*
 * free_mask()	- free the space made by make_mask().
 */

void
free_mask (mask)
	Mask		mask;		/* mask to be free'ed */
{
	(void) free ((char *) mask->bits);
	(void) free ((char *) mask);
}

Mask
copy_mask (src)
	Mask		src;		/* where the bits are from */
{
	Mask		dst;		/* where they are going to */

	if (!src) {
		errno = EINVAL;
		return (Mask) 0;
	}

	dst = make_mask (src->size);
	memcpy ((char *) dst->bits, (char *) src->bits,
		BIT_TO_UNIT(src->size-1));
	return dst;
}

/*
 * find_first_bit()	- returns the first bit set in the argument.
 *			  Bits are numbered starting at 0.  -1
 *			  indicates no bits are set.
 */

int
find_first_bit (src)
	Mask		src;		/* where the bit is */
{
	int		*ptr;		/* pointer into src */
	int		where = 0;
	int		pos;		/* bit position */
	int		found = 0;	/* true if we found a bit */


	if (!src) {
		errno = EINVAL;
		return -1;
	}
	for (ptr = src->bits; ptr < &src->bits[BIT_TO_INT(src->size-1)]; ptr++) {
		if (*ptr != 0 && ((pos = ffs (*ptr)) > 0)) {
			where += pos - 1;
			++found;
			break;
		}
		else {
			where += NUM_BITS_INT;
		}
	}
	if (!found)
		return -1;
	else
		return where;
}

/*
 * set_bit()		- set the bit in the argument.
 *			  Bits are numbered starting at 0.
 */

int
set_bit (src, where)
	Mask		src;		/* where we set the bit */
	int		where;		/* position of the bit */
{
	if (!src || where >= src->size ) {
		errno = EINVAL;
		return -1;
	}
	src->bits[POS_TO_INDEX(where)] |= BIT_TO_MASK (where);
	return 0;
}

/*
 * clear_bit()		- set to 0 the given bit.
 *			  Bits are numbered starting at 0.
 */

int
clear_bit (src, where)
	Mask		src;		/* where we clear the bit */
	int		where;		/* position of the bit */
{
	if (!src || where >= src->size) {
		errno = EINVAL;
		return -1;
	}
	src->bits[POS_TO_INDEX(where)] &= ~BIT_TO_MASK (where);
	return 0;
}


/*
 * test_bit()		- returns true if the specified bit is set.
 */

int
test_bit (src, where)
	Mask		src;		/* where we test the bit */
	int		where;		/* position of the bit */
{
	int		themask;
	int		testmask;
	int		index;
	int		value;

	index = POS_TO_INDEX(where);
	themask = src->bits[index];
	testmask = BIT_TO_MASK(where);
	value = themask & testmask;
	return value;
}

/*
 * print_bits()		- print a binary representation of the mask.  Inserts
 *			  a newline at the 64'th character
 *			  mask.  If FILE pointer is null, sends
 *			  it to stderr.
 */

void
print_bits (mask, fptr)
	Mask		mask;	/* the bitmask */
	FILE		*fptr;	/* where to write the data.  NULL=> stderr */
{
	int		i;
	if (!fptr)
		fptr = stderr;
	if (!mask) {
		fprintf (fptr, "print_bits: mask == NULL!!\n");
		return;
	}

	for (i = 0; i < mask->size; i++) {
		if (test_bit(mask, i)) {
			fputc ('1', fptr);
		}
		else {
			fputc ('0', fptr);
		}
		if ((i + 1) % 64 == 0) {
			fputc ('\n', fptr);
		}
		else if ((i+1) % 8 == 0) {
			fputc (' ', fptr);
		}
	}
	if ((i + 1) % 64 != 0)
		fputc ('\n', fptr);
}


#ifdef TEST
#include	<ctype.h>
main (argc, argv)
	int	argc;
	char	**argv;
{
	Mask	mask;
	int	size;
	int	bit;
	char	buf[256];
	char	*reply;
	int	quit;

	for (;;) {
		printf ("size ");
		gets (buf);
		sscanf (buf, "%d", &size);
		mask = make_mask (size);
		print_bits (mask, stdout);
		quit = 0;
		do {
			printf ("Set Clear Print Find Quit? ");
			gets (buf);
			for (reply = buf; *reply && isspace (*reply); ++reply)
				;
			if (!*reply)
				continue;
			switch (*reply) {
			case 's': case 'S':
				printf ("Set bit [%d]? ", size);
				gets (buf);
				bit = 0;    /* Set in case of empty input. */
				sscanf (buf, "%d", &bit);
				set_bit (mask, bit);
				break;
			case 'c': case 'C':
				printf ("Clr bit [%d]? ", size);
				gets (buf);
				bit = 0;    /* Set in case of empty input. */
				sscanf (buf, "%d", &bit);
				clear_bit (mask, bit);
				break;
			case 'p': case 'P':
				print_bits (mask, stdout);
				break;
			case 'f':  case 'F':
				printf ("First set is %d\n", find_first_bit (mask));
				break;
			case 'q': case 'Q':
				quit = 1;
			}
		} while (!quit);
		break;
	}
}
#endif /* TEST */
