/*
 * Copyright 1990 Science Applications International Corporation.
 */

/*
 * .TH DYNARRAY "15 March 1989"
 * .SH NAME
 *	array_create \- create a new dynamic array
 *	.br
 *	array_free \- free a dynamic array
 *	.br
 *	array_count \- return the number of element in an array
 *	.br
 *	array_list \- return the user's array
 *	.br
 *	array_add \- add an element to the array
 *	.br
 *	array_delete \- remove and element from the array
 *	.br
 *	array_setrate \- set the number of elements allocated each time
 *	.br
 *	array_add_n \- add n elements to a dynamic array
 *	.br
 *	array_destroy \-  free the contents of a dynamic array and free the dynamic array
 *	.br
 *	array_copy \-  copy the contents of a dynamic array
 *	.br
 * .SH FILE
 *	dyn_array.c(libaesir.a)
 *	dyn_array_ext.c(libaesir.a)
 * .SH SYNOPSIS
 *	.nf
 *	\fB#include "dyn_array.h"\fP
 *	.sp
 *	Array
 *	array_create (size)
 *	int		size;		(i) size of each element
 *	.sp
 *	void
 *	array_free (array)
 *	Array		array;		(i) the dynamic array
 *	.sp
 *	array_count (array)
 *	Array		array;		(i) the dynamic array
 *	.sp
 *	caddr_t
 *	array_list (array)
 *	Array		array;		(i) the dynamic array
 *	.sp
 *	caddr_t
 *	array_add (array, element)
 *	Array		array;		(i) the dynamic array
 *	caddr_t		element;	(i) address of element to add
 *	.sp
 *	caddr_t
 *	array_delete (array, element)
 *	Array		array;		(i) the dynamic array
 *	caddr_t		element;	(i) element to delete
 *	.sp
 *	void
 *	array_setrate (array, rate)
 *	Array		array;		(i) the dynamic array
 *	int		rate;		(i) how many elements to allocate
 *	.sp
 *	caddr_t
 *	array_add_n (array, elements, n)
 *	Array		array;		(i) the dynamic array
 *	caddr_t		elements;	(i) address of elements to add
 *	int		n;		(i) number of elements
 *	.sp
 *	void
 *	array_destroy (array)
 *	Array 	array;		(i) the dynamic array
 *	.sp
 *	Array
 *	array_copy (a)
 *	Array 	a;  		(i) the dynamic array
 *	.fi
 * .SH DESCRIPTION
 *	.LP
 *	The above functions implement dynamic arrays in the same manner
 *	that one would use malloc(3) and realloc(3) to keep adding
 *	elements to a list.  An arbitrary number of elements can be
 *	added to a list with each new element added to the end of the
 *	list.  When an element is deleted, the list is compacted.
 *	The function \fIarray_destroy\fP will free both the contents
 *	of the dynamic array and the dynamic array itself.  See
 *	description below.
 *	.LP
 *	.I array_create()
 *	is first called to initialize an \fIArray\fP structure.  The
 *	\fIArray\fP data type is opaque and should not be modified.
 *	The \fIsize\fP is the size of each element, typically as
 *	returned by sizeof (i.e. \fIsizeof (float)\fP).  After one is
 *	finished allocating elements, call \fIarray_free\fP to free
 *	the data associated with the dynamic array.  Note that this
 *	does not free up the array itself, just any internal space
 *	that the dynamic array package allocates.  One must call
 *	\fIfree(3)\fP on the list of data when finished.
 *	.LP
 *	.I array_count()
 *	returns the number of elements in the dynamic array.
 *	.LP
 *	.I array_add()
 *	adds an element onto the end of the list and returns a pointer
 *	to the first element of the list.  If unable to allocate memory,
 *	returns a NULL ptr.  \fIarray\fP must be previously allocated by
 *	\fIarray_create()\fP.  \fIelement\fP is the \fBaddress\fP of the
 *	element to be added to the list.  \fIsize\fP (as specified in
 *	\fIarray_create\fP) bytes is copied  from the address to the end
 *	of the list.
 *	.LP
 *	.I array_delete()
 *	removes the specified \fIelement\fP from the list.  The list
 *	is then compacted by copying the rest of the list over the
 *	deleted element.  \fIsize\fP bytes (as specified in the call
 *	to \fIarray_create()\fP) are compared to find the element to
 *	delete.  If multiple copies of the element exist, then only
 *	the first one is deleted.  The address of the object to be
 *	compared must be passed, not the element itself.  The address
 *	of the first element of the list is returned.
 *	.LP
 *	.I array_setrate()
 *	specified the number of elements to pre-allocate.  By default,
 *	five  elements are allocated at a time.  Changing this could
 *	result in minor speed improvements.  If one knows the
 *	approximate number of elements is large, then setting this
 *	to a large value results in fewer calls to \fIrealloc(3)\fP.
 *	If space is a concern, the setting this to one results in
 *	\fIrealloc(3)\fP being called for each new element.
 *	.LP
 *	.I array_add_n()
 *	adds n elements onto the end of the list and returns a pointer
 *	to the first element of the list.  \fIarray\fP must be previously
 *	allocated by \fIarray_create()\fP.  \fIelements\fP is the beginning
 *	\fBaddress\fP of the elements to be added to the list.  Then,
 *	\fIn * size\fP (as specified in \fIarray_create\fP) bytes are
 *	copied from the address to the end of the list using \fIarray_add\fP.
 *	If the array is NULL, or if \fIelements\fP is NULL and \fIn\fP is
 *	greater than zero, or if \fIn\fP is less than or equal to zero,
 *	a warning is given, and a NULL ptr is returned.
 *	.LP
 *	.I array_destroy()
 *	frees the contents of the dynamic array using \fIfree(3)\fP and
 *	then frees the dynamic array using \fIarray_free\fP.  If the
 *	dynamic array is NULL on input, no action is taken.
 *	.LP
 *	.I array_copy()
 *	creates a new dynamic array using \fIarray_create\fP, copies the
 *	data of the input dynamic array into the new dynamic array, and
 *	returns the new dynamic array.  Memory is allocated for the contents
 *	of the new dynamic array using \fImalloc(3)\fP.  The contents of
 *	the input array are copied using \fImemcpy(3)\fP. If the input
 *	dynamic array is NULL, or if a new dynamic array could not be created,
 *	a warning is given, and a NULL pointer is returned. If the memory
 *	for the copy of the contents could not be allocated, an error is
 *	reported, the dynamic array is freed, and a NULL pointer is returned.
 * .SH EXAMPLE
 *	.LP
 *	The following example reads a list of floating point numbers
 *	from stdin and then prints out the list at the end:
 *	.nf
 *	float	float_val
 *	float	*list;
 *	int	i, count;
 *	Array	a;
 *
 *		a = array_create (sizeof (char *));
 *		while (scanf ("%f", &float_val) != NULL)
 *		{
 *			list = (float *) array_add (a, &float_val);
 *		}
 *		count = array_count (a);
 *		array_free (a);
 *		for (i = 0; i < count; i++)
 *		{
 *			printf ("%f\n", list[i]);
 *		}
 *		free ((char *) list)
 *	.fi
 * .SH AUTHOR
 *	Pete Ware
 *	Jeff Given, Shawn Wass, Darrin Wahl
 * .SH BUGS
 *	It might be nice to be able to insert elements in arbitrary positions
 *	and delete elements at arbitrary positions.  For example, if
 *	one wanted to create dynamic strings, it would be nice to be
 *	able to always have a null terminated string by adding an
 *	element just before the end of the string.
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


Array
array_create (size)
int		size;			/* size of each element */
{
	Array	a;

	if (size <= 0)
	{
		Warning ("array_create size (%d) <= 0\n", size);
		return (Array) NULL;
	}

	a = (Array) malloc (sizeof (ArrayRec));
	if (!a)
	{
		Werror ("array_create");
	}
	else
	{
		a->ar_size = size;
		a->ar_list = NULL;
		a->ar_count = 0;
		a->ar_allocated = 0;
		a->ar_allocate_count = ARRAY_DEF_ALLOCATE;
	}
	return a;
}

void
array_free (array)
Array		array;			/* the dynamic array */
{
	if (array)
		(void) free (array);
}

int
array_count (array)
Array		array;			/* the dynamic array */
{
	if (array)
		return array->ar_count;
	else
		return 0;
}

caddr_t
array_list (array)
Array		array;			/* the dynamic array */
{
	if (array)
		return array->ar_list;
	else
		return (caddr_t) NULL;
}

caddr_t
array_add (array, element)
Array		array;			/* the dynamic array */
caddr_t		element;		/* address of element to add */
{
        caddr_t		tmp_ptr;

	if (!array)
	{
		Warning ("array_add: array is NULL\n");
		return (caddr_t) NULL;
	}
	if (!element)
	{
		Warning ("array_add: element is NULL\n");
		return array->ar_list;
	}
	if (array->ar_count < 0)
	{
		array->ar_list = NULL;
		array->ar_count = 0;
	}

	if ((array->ar_count + 1) > array->ar_allocated)
	{
		array->ar_allocated += array->ar_allocate_count;
		if (!array->ar_list)
			tmp_ptr = (caddr_t)
				malloc ((size_t) (array->ar_size *
						  array->ar_allocated));
		else
			tmp_ptr = (caddr_t)
				realloc (array->ar_list,
					 (size_t) (array->ar_size *
						   array->ar_allocated));
		if (tmp_ptr)
		     array->ar_list = tmp_ptr;
		else
		{
		     array->ar_allocated -= array->ar_allocate_count;
		     Werror ("array_add");
		     return (caddr_t) NULL;
		}
	}
	memcpy (ARRAY_INDEX (array, array->ar_count), element,
		(size_t) array->ar_size);
	++array->ar_count;
	return array->ar_list;
}

caddr_t
array_delete (array, element)
Array		array;			/* the dynamic array */
caddr_t		element;		/* element to delete */
{
	int	index;

	if (!array)
	{
		Warning ("array_delete: array is NULL\n");
		return (caddr_t) NULL;
	}
	if (!element)
	{
		Warning ("array_delete: element is NULL\n");
		return array->ar_list;
	}
	for (index = 0; index < array->ar_count; index++)
	{
		if (memcmp ((const void * )ARRAY_INDEX (array, index),
			    (const void * )element, (size_t) array->ar_size)
		    == 0)
			break;
	}
	if (index >= array->ar_count)
	{
		Warning ("array_delete: element %x not found\n", element);
		return array->ar_list;
	}
	while (index + 1 < array->ar_count)
	{
#ifdef __svr4__
		memmove (ARRAY_INDEX (array, index),
			 ARRAY_INDEX (array, index + 1),
			 (size_t) array->ar_size);
#else
		bcopy (ARRAY_INDEX (array, index + 1),
		       ARRAY_INDEX (array, index),
		       array->ar_size);
#endif
		++index;
	}
	if (array->ar_count > 0)
	{
		--array->ar_count;
		memset (ARRAY_INDEX (array, index), 0, (size_t) array->ar_size);
	}
	if (array->ar_count == 0)
	{
		array->ar_allocated = 0;
		if (array->ar_list)
			(void) free (array->ar_list);
		array->ar_list = NULL;
	}
	return array->ar_list;
}

void
array_setrate (array, rate)
Array		array;			/* the dynamic array */
int		rate;			/* how many elements to allocate */
{
	if (!array)
	{
		Warning ("array_setrate: array is NULL\n");
		return;
	}
	if (rate <= 0)
	{
		Warning ("array_setrate: rate (%d) is <= 0\n", rate);
		return;
	}
	array->ar_allocate_count = rate;
}



#ifdef TEST
main (argc, argv)
int		argc;
char		**argv;
{
	char	buf[BUFSIZ];
	int	i, count;
	char	*input;
	Array	a, b;
	char	**list;

	printf ("Enter text followed by returns; enter <Cntrl d> to terminate input.\n");

	a = array_create (sizeof (char *));
	while (gets (buf) != NULL)
	{
		input = strcpy (malloc (strlen (buf) + 1), buf);
		list = (char **)
			array_add (a, (caddr_t) &input);
	}

	/*
	 *  Print out the entries.
	 */
	printf ("\nContents are:\n");
	count = array_count (a);
	for (i = 0; i < count; i++)
	{
		puts (list[i]);
	}

	printf ("\n");


	/* Make a copy of the array and print out entries */
	b = array_copy (a);
	count = array_count (b);
	list = (char **) array_list (b);
	printf ("Copy of contents:\n");
	for (i = 0; i < count; i++)
	{
		puts (list[i]);
	}
	printf ("\n");



	/*
	 *  Delete every other entry and then
	 *  print them out.
	 */
	for (i = 0; i < count; i++)
	{
		printf ("Deleting '%s'\n", list[i]);

		/*
		 *  We need to get our list each time because our
		 *  memory is shifted with each deletion.
		 */
		list = (char **) array_delete (a, (caddr_t) &list[i]);
		count = array_count (a);
	}

	printf ("\nThe final contents are:\n");
	count = array_count (a);
	list = (char **) array_list (a);
	for (i = 0; i < count; i++)
	{
		puts (list[i]);
	}


	/* Add final contents of a to b and print results */
	list = (char **) array_add_n (b, array_list (a),(int) array_count(a));
	count = array_count (b);
	printf ("\nFinal result added to copy:\n");
	for (i = 0; i < count; i++)
	{
		puts (list[i]);
	}
	printf ("\n");


	/* Test array_destroy */
	array_destroy (b);


	array_free (a);
	a = NULL;

}
#endif /* TEST */
