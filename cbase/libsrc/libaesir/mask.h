/*
 *	SccsId:	@(#)mask.h	105.1	19 May 1995
 */

/*
 * $Header: mask.h,v 1.2 87/06/03 17:39:34 jim Locked $
 *
 * $Log:	mask.h,v $
 * Revision 1.2  87/06/03  17:39:34  pete
 * Changed definition of Mask and added some macros for accessing
 * the structure.
 * 
 * Revision 1.1  87/05/20  10:28:24  pete
 * Initial revision
 * 
 *
 * Define the structures and functions needed to manipulate
 * bitmasks.
 */

#ifndef _MASK_H_
#define _MASK_H_

#include <stdio.h>
#include "aesir.h"

/*
 * One hopes that sizeof() returns a value that is used by
 * malloc.  This is unnecessary paranoia, but it does not
 * cost me anything.
 */

typedef struct mask {	/* definition of a bitmask */
	int	size;		/* number of bits in mask */
	int	*bits;		/* the array of bits */
} *Mask;

#define UNIT_IN_INT	(sizeof(int))
#define NUM_BITS_UNIT	8	/* number of bits per unit from sizeof */
#define NUM_BITS_INT	(NUM_BITS_UNIT * UNIT_IN_INT)

/*
 * Round the number up to the nearest number of ints needed to
 * represent that number of bits.
 */

#define BIT_TO_INT(bits)	(((bits)/NUM_BITS_INT)+1)

/* Round the number up to give the number of units (bytes) needed */
#define BIT_TO_UNIT(bits)	(BIT_TO_INT(bits)*UNIT_IN_INT)

/* Return which integer the specified posistion falls into */
#define POS_TO_INDEX(pos)	(BIT_TO_INT(pos)-1)

/* Return which bit is desired in the word */
#define POS_TO_LOC(pos)		((pos) % NUM_BITS_INT)

/* Return a mask suitable for turning on or off a bit withan an int */
#define BIT_TO_MASK(pos)		(1 << POS_TO_LOC(pos))


/*
 * Return the bits or the size of a mask -- especially useful for select(2),
 * hence sel_ for the name
 */

#define sel_mask(mask)	((mask)->bits)
#define sel_size(mask)	((mask)->size)


	/* function which duplicates a bitmask */
extern Mask copy_mask (Mask src);

	/* function which returns the first bit set in the argument. */
extern int find_first_bit (Mask src);

	/* function which frees the space made by make_mask(). */
extern void free_mask (Mask mask);

	/* function which returns a bitmask */
extern Mask make_mask (int size);

	/* Clear the bit in the argument. */
extern int clear_bit (Mask src, int where);

	/* Print a binary representation of the mask. */
extern void print_bits (Mask mask, FILE *fptr);

	/* Set the bit in the argument. */
extern int set_bit (Mask src, int where);

	/* Tests if bit is set */
extern int test_bit (Mask src, int where);

#endif /* _MASK_H_ */
