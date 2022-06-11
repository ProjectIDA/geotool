
#ifdef	SCCSID
static char SccsId[] = "@(#)libtable/tree.c	104.1 07/08/97 Copyright 1992 SAIC";
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
 *	tree -- manage binary tree implementation of table.
 *
 * FILE
 *	tree.c
 *
 * SYNOPSIS
 *	Implement binary tree data structure for table interface.
 *
 * DESCRIPTION
 *
 *	GLOBAL FUNCTIONS:
 *
 *	table_tree_create() -- Initialize table structure with tree functions.
 *
 *	TABLE	*
 *	table_tree_create(compar, release, destroy)
 *	int	(*compar)();	(i) Function for data comparison.
 *	void	(*release)();	(i) Function for releasing element's memory
 *				    to the system.
 *	void	(*destroy)();	(i) Table overhead self-free function.
 *
 *
 *	LOCAL FUNCTIONS:
 *
 *	tree_add() -- Add a node to a tree.
 *
 *	static int
 *	tree_add(datum, tree, compar)
 *	char	*datum;		(i) Pointer to datum to be added to tree.
 *	char	**tree;		(i) A pointer to the pointer to the tree.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 *
 *	tree_delete() -- Delete a node from a tree.
 *
 *	static int
 *	tree_delete(datum, tree, compar)
 *	char	*datum;		(i) Pointer to datum to be deleted from tree.
 *	char	**tree;		(i) A pointer to the pointer to the tree.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 *
 *	tree_find() -- Find a node in a tree.
 *
 *	static char	*
 *	tree_find(datum, tree, compar)
 *	char	*datum;		(i) Pointer to datum to be found in tree.
 *	char	**tree;		(i) A pointer to the pointer to the tree.
 *	int	(*compar)();	(i) Function for data comparison.
 *
 * DIAGNOSTICS
 *	table_tree_create returns a pointer to an empty table if successful,
 *	NULL if failure (typically "out of memory").
 *
 *	tree_add and tree_delete return TABLE_ERR if any problems
 *	are encountered, typically "out of memory" for tree_add and
 *	a NULL tree for both tree_add and tree_delete.  TABLE_OK is 
 *	returned upon success.
 *
 *	tree_find returns a pointer to the tree user's data if found in
 *	the tree ("found" being determined by the tree's compar()
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
 * BUGS:
 *	The ability to traverse the tree is not currently implemented.  Use 
 *	of twalk(3) would provide such an interface.  A private data structure 
 *	similar to the list and hash implementations would be necessary to 
 *	keep track of the current node being traversed.
 *
 * SEE ALSO
 *	table, tsearch(3).
 *
 * AUTHOR
 *	Brian Smithey, SAIC Geophysics Division, 1989
 */


#include <stdio.h>
#include <stdlib.h>
#include <search.h>

#include "table.h"

#ifdef __STDC__

static int	tree_add (char *datum, char *tree_p,
			  TABLE_COMPARE compar);
static int	tree_delete (char *datum, char *tree_p, 
			     TABLE_COMPARE compar,
			     TABLE_RELEASE release);
static char	*tree_find (char *datum, char *tree_p, 
			    TABLE_COMPARE compar);
static void	tree_first (char *);
static char	*tree_next (char *);

#else	/* ! __STDC__  */

static int	tree_add ();
static int	tree_delete ();
static char	*tree_find ();
static void	tree_first ();
static char	*tree_next ();

#endif

        /*
	 *  Create a tree accessible using the table_add(), table_delete(), 
	 *  table_find () etc functions.
	 */

TABLE	*
table_tree_create(compar, release, destroy)
int	(*compar)();
void	(*release)();
void	(*destroy)();
{

	TABLE		*table;
	
	if (!compar)
		compar = default_compar;

	if ((table = (TABLE *) malloc ((unsigned) sizeof(TABLE)))
		!= NULL)
	{
		table->add = tree_add;
		table->delet = tree_delete;
		table->find = tree_find;
		table->first = tree_first;
		table->next = tree_next;
		table->compar = compar;
		table->release = release;

		/*
		 *  Since the table_destroy() routine really only calls 
		 *  destroy, destroy should call release too!
		 */
		table->destroy = destroy;
		table->privat = (char *) NULL;	/* Currently not applicable. */
		table->private_destroy = NULL;	/* Currently not applicable. */
		table->count = NULL;		/* Currently unsupported. */
	}

	return table;
}



/************************  Local Functions  ************************************/


static int
tree_add(datum, tree_p, compar)
char	*datum;
char	*tree_p;
int	(*compar)();
{

/*	extern char	*tsearch(); */
	

	if ((char *) tsearch((void *)datum, (void **) &tree_p, 
			     (SEARCH_COMPARE) compar) == (char *)NULL)
	{
		/* Couldn't insert datum, tree_p probably NULL */
		return(TABLE_ERR);
	}
	else
	{
		return(TABLE_OK);
	}
}


static int
tree_delete(datum, tree_p, compar, release)
char	*datum;
char	*tree_p;
int	(*compar)();
void	(*release)();
{

	char		*user_datum;
/*	extern char	*tfind(); */
/*	extern char	*tdelete();*/
	

	/* get a handle on actual user datum */
	user_datum = (char *) tfind((void *) datum, (void **) &tree_p, 
				    (SEARCH_COMPARE) compar);

	if ((char *) tdelete((void *) datum, (void **) &tree_p, 
			      (SEARCH_COMPARE) compar) == (char *)NULL)
	{
		/* datum not found in tree */
		return(TABLE_ERR);
	}
	else
	{
		/*
		 * see comments below for tree_find() regarding indirect
		 * through tfind() return value to access user datum.
		 */
		if (release)
		{
			(*release)(*((char **)user_datum));
		}
		return(TABLE_OK);
	}
}


static char	*
tree_find(datum, tree_p, compar)
char	*datum;
char	*tree_p;
int	(*compar)();
{

	char		*retval;
/*	extern char	*tfind(); */
	

	/*
	 * tfind (as well as tsearch) actually returns a pointer to the
	 * pointer that was passed to tsearch for insertion, so we must
	 * indirect before returning so that we pass back to the caller
	 * the expected pointer (the pointer that actually points to the
	 * caller's data).
	 */

	if ((retval = (char *) tfind((void *) datum, (void **) &tree_p, 
				     (SEARCH_COMPARE) compar)) 
		!= NULL)
	{
		return (*((char **)retval));
	}
	else
	{
		return ((char *)NULL);
	}
}


static void
tree_first(t)
char	*t;
{
	return;
}


static char *
tree_next(t)
char	*t;
{
	return ((char *) NULL);
}
