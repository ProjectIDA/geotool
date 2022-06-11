#ifdef	SCCSID
static	char SccsId[] = "@(#)libtable/list.c	104.1	07/08/97 Copyright 1992 SAIC";
#endif
/*
 * --BEGIN--
 * Copyright 1992 Science Applications International Corporation
 *		  All Rights Reserved
 *
 * Author: Pete Ware, SAIC, Geophysics Division / San Diego CA / (619) 458-2520
 * --END--
 */
/*
 * NAME
 *	list -- manage double linked list implementation of table.
 *
 * FILE
 *	list.c
 *
 * SYNOPSIS
 *	Implement linked list data structure for table interface.
 *
 * DESCRIPTION
 *
 *	GLOBAL functions:
 *
 *	table_list_create() -- Initialize table structure with list functions.
 *
 *	TABLE	*
 *	table_list_create(compar, release, destroy)
 *	int	(*compar)();	(i) Function for data comparison.
 *	void	(*release)();	(i) Function for releasing element's memory
 *				    to the system.
 *	void	(*destroy)();	(i) Table overhead self-free function.
 *
 *
 *	LOCAL functions:
 *
 *	list_add() -- Add a node to a list.
 *
 *	static int
 *	list_add(datum, list, compar)
 *	char	*datum;		(i) Pointer to datum to be added to list.
 *	char	**list;		(i) A pointer to the pointer to the list.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 *
 *	list_prepend() -- Add a node to the beginning of a list.
 *
 *	static int
 *	list_prepend(datum, list, compar)
 *	char	*datum;		(i) Pointer to datum to be added to list.
 *	char	**list;		(i) A pointer to the pointer to the list.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 *
 *	list_append() -- Add a node to the end of a list.
 *
 *	static int
 *	list_append(datum, list, compar)
 *	char	*datum;		(i) Pointer to datum to be added to list.
 *	char	**list;		(i) A pointer to the pointer to the list.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 *
 *	list_delete() -- Delete a node from a list.
 *
 *	static int
 *	list_delete(datum, list, compar)
 *	char	*datum;		(i) Pointer to datum to be deleted from list.
 *	char	**list;		(i) A pointer to the pointer to the list.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 *
 *	list_find() -- Find a node in a list.
 *
 *	static char	*
 *	list_find(datum, list, compar)
 *	char	*datum;		(i) Pointer to datum to be found in list.
 *	char	**list;		(i) A pointer to the pointer to the list.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 * DIAGNOSTICS
 *	table_list_create() returns a pointer to an empty table if successful,
 *	NULL if failure (typically "out of memory").
 *
 *	list_add() and list_delete() return TABLE_ERR if any problems
 *	are encountered, typically "out of memory" for list_add() and
 *	if a NULL table is passed to list_add() and list_delete(). TABLE_OK 
 *	is returned on success.
 *
 *	list_find() returns a pointer to the list user's data if found in
 *	the list ("found" being determined by the list's compar()
 *	function) and NULL if not found.
 *
 * FILES
 *	a_list.h, table.h
 *
 * NOTES
 *	The "compar()" function is a user supplied function that will
 *	be called with two arguments, the pointers to the elements being
 *	compared.  The function should return an integer less than,
 *	equal to, or greater than 0, according to whether the first
 *	argument is to be considered less than, equal to, or greater
 *	than the second.
 *
 * SEE ALSO
 *	table, tsearch(3).
 *
 * AUTHOR
 *	Brian Smithey	1989	Created.
 *	Shannon Torrey	1991 	Added append and prepend funcs.
 *	Jim Wang		Add, append, delete, and insert return TABLE_ERR
 *				if object exists for adding or if doesn't on 
 *				delete.
 */


#include <stdio.h>
#include <stdlib.h>

#ifndef	_TYPES_
#include <sys/types.h>
#endif	/* _TYPES_ */

#include <search.h>

#include "table.h"
#include "a_list.h"


	/*
	 *  This function allows the default_destroy() function 
	 *  to successfully clean up the list table.  This will
	 *  deallocate all of the nodes (which pointed to the data).
	 *  This change was required when the table.c default_destroy() 
	 *  function was changed (it was stepping on it's own toes!).
	 */
static void 
list_destroy (list)
char	*list;
{
	if (list)
	{
		destroy_list ((LIST_HEAD *) list, NULL);
	}
}


static int
list_add(datum, list_p, compar)
char	     *datum;
char	     *list_p;
TABLE_COMPARE compar;
{
	if (find_element((LIST_HEAD *)list_p, (caddr_t)datum, 
			 (LIST_COMPARE) compar))
	{
		return (TABLE_ERR);
	}
	else
	{
		add_element((LIST_HEAD *)list_p, (caddr_t)datum, 
			    (LIST_COMPARE) compar);
		return(TABLE_OK);
	}
}

static 
int
list_append(datum, list_p, compar)
char	     *datum;
char	     *list_p;
TABLE_COMPARE compar;
{
	append_element((LIST_HEAD *)list_p, (caddr_t)datum);
	return(TABLE_OK);
}

static int
list_prepend(datum, list_p, compar)
char	*datum;
char	*list_p;
TABLE_COMPARE compar;
{
	prepend_element((LIST_HEAD *)list_p, (caddr_t)datum);
	return(TABLE_OK);
}

static int
list_delete(datum, list_p, compar, release)
char	*datum;
char	*list_p;
TABLE_COMPARE compar;
TABLE_RELEASE release;
{

	char		*user_datum;

	if ((user_datum = (char *) find_element ((LIST_HEAD *) list_p, 
						 (caddr_t) datum, 
						 (LIST_COMPARE) compar))
	    != NULL)
	{
		delete_element((LIST_HEAD *)list_p, (caddr_t)datum, 
			       (LIST_COMPARE) compar);
		if (release)
		{
			(*release)(user_datum);
		}
		return(TABLE_OK);
	}
	else
	{
		return (TABLE_ERR);
	}
}


static char	*
list_find(datum, list_p, compar)
char	*datum;
char	*list_p;
TABLE_COMPARE compar;
{
	return ((char *)find_element((LIST_HEAD *)list_p, (caddr_t)datum,
				     (LIST_COMPARE) compar));
}


static void
list_first(list_p)
char	*list_p;
{
	start_list((LIST_HEAD *)list_p);
}


static char	*
list_next(list_p)
char	*list_p;
{
	return ((char *) next_list((LIST_HEAD *) list_p));
}


static int
list_size(list_p)
char	*list_p;
{
	return (list_count((LIST_HEAD *) list_p));
}

	/*
	 *  Create a list accessible via the TABLE routines table_add(), 
	 *  table_delete(), etc.
	 */

TABLE	*
table_list_create(compar, release, destroy)
TABLE_COMPARE compar;
TABLE_RELEASE release;
TABLE_DESTROY destroy;
{
	TABLE			*table;

	if (!compar)
		compar = (TABLE_COMPARE) default_compar;

	if ((table = (TABLE *) malloc ((unsigned) sizeof(TABLE)))
	    != (TABLE *) NULL)
	{
		table->add = list_add;
		table->append = list_append;
		table->prepend = list_prepend;
		table->delet = list_delete;
		table->find = list_find;
		table->first = list_first;
		table->next = list_next;
		table->compar = compar;
		table->release = release;
		table->destroy = destroy;	/* User destroy function */
		table->privat = (char *) create_list();

		/*
		 *  Private destroy function to deallocate the 
		 *  internal data structure used for the list.
		 */
		table->private_destroy = list_destroy;

		table->count = list_size;
	}

	return table;
}
