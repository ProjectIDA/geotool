
#ifdef	SCCSID
static	char SccsId[] = "@(#)libtable/table.c	104.1	07/08/97 Copyright 1992 SAIC";
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
 *	table -- manage insertion, search, and deletion of data.
 *
 * FILE
 *	table.c
 *
 * SYNOPSIS
 *	Allow creation of a table and insertion, searching, and deletion
 *	of data in the table.  Different types of data structures are
 *	supported with identical interface.
 *
 * DESCRIPTION
 *	table_destroy() -- Destroy a table include each entry in table.
 *
 *	void
 *	table_destroy(table)
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *	table_add() -- Add a node to a table.
 *
 *	int
 *	table_add(datum, table)
 *	char	*datum;		(i) Pointer to datum to be added to table.
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_append() -- Append a node to a table.
 *
 *	int
 *	table_append(datum, table)
 *	char	*datum;		(i) Pointer to datum to be added to table.
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_prepend() -- Prepend a node to a table.
 *
 *	int
 *	table_prepend(datum, table)
 *	char	*datum;		(i) Pointer to datum to be added to table.
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_delete() -- Delete a node from a table.
 *
 *	int
 *	table_delete(datum, table)
 *	char	*datum;		(i) Pointer to datum to be deleted from table.
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_unlink() -- Unlink a node from a table.
 *
 *	char	*
 *	table_unlink(datum, table)
 *	char	*datum;		(i) Pointer to datum to be unlinked from table.
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_find() -- Find a node in a table.
 *
 *	char	*
 *	table_find(datum, table)
 *	char	*datum;		(i) Pointer to datum to be found in table.
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_first() -- Prepare table for traversal.
 *
 *	void
 *	table_first(table)
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_next() --  Get "next" node in table traversal.
 *
 *	char	*
 *	table_next(table)
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 *
 *	table_count() --  Get the number of entries currently in the table.
 *
 *	int 
 *	table_count (table)
 *	TABLE	*table;		(i) A pointer to a previously created table.
 *
 * DIAGNOSTICS
 *	table_create returns a pointer to an empty table if successful,
 *	NULL if failure (typically "out of memory").
 *
 *	table_add and table_delete return TABLE_ERR if any problems
 *	are encountered, typically "out of memory" for table_add and
 *	a NULL table for both table_add and table_delete.
 *
 *	table_find returns a pointer to the table user's data if found
 *	in the table ("found" being determined by the table's compar()
 *	function) and NULL if not found.
 *
 * FILES
 *	table.h
 *
 * NOTES
 *	The "compar()" function is a user supplied function that will
 *	be called with two arguments, the pointers to the elements being
 *	compared.  The function should return an integer less than,
 *	equal to, or greater than 0, according to whether the first
 *	argument is to be considered less than, equal to, or greater
 *	than the second.
 *
 *	Currently the following data structures are implemented:
 *
 *		TABLE_LIST	-- (linear) linked list
 *		TABLE_HASH	-- hash table (similar to hsearch(3)).
 *		TABLE_TREE	-- binary tree (no traversal)
 *
 * SEE ALSO
 *	None.
 *
 * AUTHOR
 *	Brian Smithey, SAIC Geophysics Division, 1989
 *	Shannon Torrey, SAIC Geophysics, 1991  Added append and prepend funcs.
 *	Shawn Wass	February 1992	Added private_destroy() for cleaning 
 *					up the internal data structure.  Also 
 *					added means of obtaining # of elements.
 */


#include <stdio.h>
#include "table.h"
#include "aesir.h"

int
default_compar(item1, item2)
char    *item1;
char    *item2;
{
	return ((int)((char *) item1 - (char *) item2));
}

void
default_release(datum)
char	*datum;
{
	UFREE (datum);
}

void
default_destroy(table)
TABLE	*table;
{
	UFREE (table);
}


int
table_add(datum, table)
char	*datum;
TABLE	*table;
{
	if (table)
	{
		return ((*table->add)(datum, table->privat, table->compar));
	}
	else
	{
		return (TABLE_ERR);
	}
}

int
table_append(datum, table)
char	*datum;
TABLE	*table;
{
	if (table)
	{
		return ((*table->append)(datum, table->privat, table->compar));
	}
	else
	{
		return (TABLE_ERR);
	}
}

int
table_prepend(datum, table)
char	*datum;
TABLE	*table;
{
	if (table)
	{
		return ((*table->prepend)(datum, table->privat, 
					  table->compar));
	}
	else
	{
		return (TABLE_ERR);
	}
}

int
table_delete(datum, table)
char	*datum;
TABLE	*table;
{
	if (table)
	{
		return ((*table->delet)(datum, table->privat,
					 table->compar, table->release));
	}
	else
	{
		return (TABLE_ERR);
	}
}

char	*
table_unlink(datum, table)
char	*datum;
TABLE	*table;
{
	char	*ret_elt = (char *)NULL;
	
	if (table)
	{
		ret_elt = ((*table->find)(datum, table->privat,
					  table->compar));
		
		(void) ((*table->delet)(datum, table->privat,
					table->compar, (void(*)())NULL));
	}

	return (ret_elt);
}

char	*
table_find(datum, table)
char	*datum;
TABLE	*table;
{
	if (table)
	{
		return ((*table->find)(datum, table->privat, table->compar));
	}
	else
	{
		return ((char *)NULL);
	}
}

	
void
table_first(table)
TABLE	*table;
{
	if (table)
	{
		(*table->first)(table->privat);
	}
}

	
char	*
table_next(table)
TABLE	*table;
{
	if (table)
	{
		return ((*table->next)(table->privat));
	}
	else
	{
		return ((char *)NULL);
	}
}


void
table_destroy(table)
TABLE	*table;
{
	char	*elt;
	
	if (table)
	{
		if (table->release)
		{
			table_first(table);
			for (elt = table_next(table); elt; 
			     elt = table_next(table))
			{
				/*
				 *  Call the release function rather than 
				 *  table_delete.  This is to insure the 
				 *  structure of the table remains intact 
				 *  over the course of the current for loop; 
				 *  otherwise the indexing may be reset in 
				 *  the release function.
				 */
				if (table->release)
					(*table->release) (elt);
			}
		}

		if (table->destroy)
		{
			if (table->private_destroy)
			{
				/*
				 *  Deallocate the table's private data 
				 *  structure.  This may be a linked list 
				 *  or a hash table.
				 */
				(*table->private_destroy)(table->privat);
			}
			(*table->destroy)(table);
		}
	}
}

int 
table_count (table)
TABLE	*table;
{
	if (table && 
	    table->count)
	{
		return ((*table->count)(table->privat));
	}
	else
	{
		return (-1);
	}
}
