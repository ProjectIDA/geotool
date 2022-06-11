/*
 * Copyright 1990 Science Applications International Corporation.
 */

/*
 * NAME
 *      dyn_array_ext
 *
 * FILE
 *	dyn_array.c(libaesir.a)
 *      dyn_array_ext.c(libaesir.a)
 *
 * DESCRIPTION
 *      Extensions to original dyn_array.  See description in dyn_array.c.
 *
 *	array_add_n   - add n elements to a dynamic array
 *	array_destroy - free the contents of a dynamic array and free the dynamic array
 *	array_copy    - copy the contents of a dynamic array
 *      array_init    - initialize dynamic array
 *
 * AUTHOR
 *	Pete Ware
 *	Jeff Given, Shawn Wass, Darrin Wahl
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<memory.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>

#include	"libaesir.h"
#include	"dyn_array.h"


caddr_t
array_add_n (array, elements, n)
Array		array;			/* the dynamic array */
caddr_t		elements;		/* address of elements to add */
int		n;			/* number of elements to add */
{
	caddr_t	p = elements;
	int 	i;

	if (array == (Array) NULL)
	{
		Warning ("array_add_n: array is NULL\n");
		return ((caddr_t) NULL);
	}
	if ((elements == (caddr_t) NULL) && (n > (int) 0))
	{
		Warning ("array_add_n: elements is NULL and n > 0\n");
		return ((caddr_t) NULL);
	}


	/* Add each element of size array->ar_size to the array */
	for (i = 0; i < n; i++, p += array->ar_size)
	{
		(void) array_add (array, p);
	}

	return (array->ar_list);

}

void
array_destroy (a)
Array a;
{
	caddr_t c;

	if (a == (Array) NULL)
	{
		return;
	}

	if ((c = array_list (a)) != (caddr_t) NULL)
	{
		(void) free (c);
	}

	array_free (a);
}


Array
array_copy (a)
Array a;
{
	Array   b = (Array) NULL;
	caddr_t p  = (caddr_t) NULL;
	caddr_t c;

	if (a == (Array) NULL)
	{
		Warning ("array_copy: array is NULL\n");
		return ((Array) NULL);
	}

	if ((b = array_create (a->ar_size)) == (Array) NULL)
	{
		Warning ("array_copy: could not create array\n");
		return ((Array) NULL);
	}

	b->ar_size = a->ar_size;
	b->ar_list = (caddr_t) NULL;
	b->ar_count = a->ar_count;
	b->ar_allocated = a->ar_count;
	b->ar_allocate_count = a->ar_allocate_count;

	if ((c = array_list (a)) != (caddr_t) NULL)
	{
		p = (caddr_t) malloc ((size_t) b->ar_size * b->ar_count);
	}

	if (!p)
	{
		Werror ("array_copy");
		array_free (b);
		return ((Array) NULL);
	}

	memcpy ((void *) p, (const void *) a->ar_list,
		(size_t) b->ar_count * b->ar_size);

	b->ar_list = p;

	return (b);

}



Array
array_init(a, n, size)
caddr_t	a;		/* dynamically allocated array */
int     n;
int     size;
{
	Array 	array;
	caddr_t b;

	array = array_create (size);

	if (array == (Array) NULL)
	{
		Warning ("array_init: could not create array\n");
		return ((Array) NULL);
	}


	/*
	 *   presumably this will crash "malloc" if a isn't a correct
	 *   heap pointer
	 */
	b = (caddr_t) realloc ((void *) a, (size_t) n * size);

	if (b == (caddr_t) NULL)
	{
		Werror ("array_init");
		array_free (array);
		return ((Array) NULL);
	}

	array->ar_size  = size;
	array->ar_count = n;
	array->ar_list  = b;
	array->ar_allocated = n;

	return(array);
}



