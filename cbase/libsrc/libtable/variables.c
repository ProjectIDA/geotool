/*
*
* FILE NAME  variables.c
*
*
*         			    Copyright 1992
*		Science Applications International Corporation (SAIC),
*				 All Rights Reserved
*
*
* SYNOPSIS
*
*	void
*	var_init()
*
*	void
*	var_set (key, value)
*	char		*key;
*	char		*value;
*
*	char *
*	var_get (key)
*	char		*key;
*
*	void
*	var_print()
*
* DESCRIPTION
*
*	These functions create and manage a hash table for implementing an
*	associative database  var_set() inserts the key/value pairs 
*	into the table, deleting any previous value first.  var_get() searches 
*	for a given key and returns either the associated value if found,
*	or NULL.  Both the variables and values are null terminated 
*	character strings.
*
*	var_init - initializes the database.
*
*	var_set - sets a key/value association in the database.  key
*		is a pointer to the key string and value is a pointer
*		to the value string.  If value pointer is NULL, key
*		variable is deleted from database.
*
*	var_get - returns a pointer to the value string for the given key
*		if found in the database, or NULL otherwise.
*
*	var_print - print all key/value associations in the database in sorted 
*		    order.
*
* DIAGNOSTICS
*
*
* FILES
*       
*       
* NOTES
*	This is a module; which means information hiding
*	techniques have been used at the expense of expandability.
*	Only one variable table may be used per process.  For the
*	same reason, this module may not be linked dynamically.
*
* SEE ALSO
*
*
* AUTHOR
*	Pete Ware, October 1989.	Created file.
*
* MODIFICATIONS
*	Shawn Wass, November 20, 1990.	Fixed so hashs values correctly;
*	Warren Fox, May 1991.  		Changed header.
*	Warren Fox, Jan 1992.		Fully documented, and added
*					to libscheme.
*	Shawn Wass, February 18, 1992.	Improved var_set() and added sorting 
*					to var_print().
*       Jeff Given, March 22, 1994.     Moved to libtable from libscheme
*                                       (branched from version 58.1).
*       Jeff Given, March 29, 1994.     Added var_array, so that apps that want
*                                       to print can access the variables.
*	Darrin Wahl, Sept 1, 1995	Added deletion of key variable from
*					database if value is NULL in var_set()
*/

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"aesir.h"
#include	"table.h"

typedef struct
{
	char	*variable;	/* symbol for variable	*/
	char	*value;		/* value of variable 	*/
} Variable;

	/*
	 *  Forward declarations of local functions.
	 */

#ifdef __STDC__
static int	 var_compare (char *elt1, char *elt2);
static void	 var_release(char *elt);
static uintmax_t var_hash (char *elt);

#else	/* ! __STDC__ */
static int	 var_compare ();
static void	 var_release();
static uintmax_t var_hash ();

#endif

static TABLE	*var_table = NULL;	/* pointer to table of variables */

/*
 * var_init
 *	Initialize the datastructure for retaining variable values.
 *	Only one table of variables is allowed per process.
 */
void
var_init ()
{
	if ( var_table == NULL )
		var_table = table_hash_create ((TABLE_COMPARE ) var_compare, 
					       (TABLE_RELEASE ) var_release,
					       (TABLE_DESTROY) default_destroy,
					       (TABLE_HASH)    var_hash, 
					       1);
}

/*
 * var_set
 *	Set the value of a variable.  Overwrite any previous value.
 *	Delete variable from table if value is NULL.
 */
void
var_set (variable, value)
char		*variable;
char		*value;
{
	Variable	tmp_v;
	Variable	*v;

	tmp_v.variable = variable;
	tmp_v.value = value;

	/*
	 *  A table_find() is unnecessary because we can just perform 
	 *  a table_delete() and ignore the return.  This is because 
	 *  table_find() and table_delete() perform the same search.
	 */
	(void) table_delete((char *)&tmp_v, var_table);

	if (value)
	{
		v = UALLOC (Variable, 1);
		v->variable = STRALLOC (variable);
		v->value = STRALLOC (value);
		table_add ((char *) v, var_table);
	}
}

/*
 * var_get
 *	Get the value of a variable, and return the string.  Returns
 *	NULL if not found.
 */
char *
var_get (variable)
char		*variable;
{
	Variable	*v;
	Variable	val;

	val.variable = variable;
	val.value = NULL;
	v = (Variable *) table_find ((char *) &val, var_table);
	if (!v)
		return ((char *) NULL);
	else
		return v->value;
}

/*
 * var_print
 *	Print to stdout all the variables and their values in sorted order.
 */

int var_array(name_list, val_list)
char ***name_list;
char ***val_list;
{
	char           **nlist = NULL;
	char           **vlist = NULL;
	Variable	*var_list = NULL;
	Variable	*v;
	int		count;
	int		i;
	
	/*
	 *  Build array to contain the unsorted variables.
	 */
	count = table_count (var_table);
	nlist = UALLOC (char *, count);
	if(nlist) vlist = UALLOC (char *, count);
	if(vlist) var_list = UALLOC(Variable, count);

	if (var_list == NULL)
	{
		*vlist = NULL;
		return -1;
	}

	/*
	 *  Fill the arrays.
	 */

	i = 0;
	table_first (var_table);
	for (v = (Variable *) table_next (var_table); v;
	     v = (Variable *) table_next (var_table))
	{
		var_list[i].variable = v->variable;
		var_list[i].value = v->value;
		i++;
	}

	/*
	 *  Now sort them.
	 */
	qsort ( var_list, count, sizeof (Variable), 
	        (SEARCH_COMPARE) var_compare );


	for(i = 0; i < count; i++)
	{
		nlist[i] = var_list[i].variable;
		vlist[i] = var_list[i].value;
	}

	*name_list = nlist;
	*val_list  = vlist;
		
	UFREE(var_list);

	return(i);
}

void
var_print (flag)
int	flag;		/* unused, intended to control printing */
{
	int              i, count;
	char             **names, **values;
	
	if ((count = var_array(&names, &values) < 0))
	{
		fprintf(stderr, "Error: var_print unable to malloc for variable list!\n");
		return; 
	}
	
	for (i = 0; i < count; i++)
	{
		fprintf(stdout, "%20s: %s\n", names[i], values[i]);
	}

	UFREE( names );
	UFREE( values );
}

	/*
	 *  Functions local to file.
	 */

/*
 * var_compare
 *	Compare strings, return 0 for true, -1 when first argument
 *	is less than second, and 1 for the reverse situation.
 */
static int
var_compare (elt1, elt2)
char		*elt1;
char		*elt2;
{
	Variable	*v1 = (Variable *) elt1;
	Variable	*v2 = (Variable *) elt2;

	return (strcmp (v1->variable, v2->variable));
}

/*
 * var_release
 *	Release memory occupied by this variable.
 */
static void
var_release(elt)
char	*elt;
{
	Variable	*v = (Variable *) elt;

	free ((char *) v->variable);
	free ((char *) v->value);
	free ((char *) v);
}

/*
 * var_hash
 *	Return a hash value for the given string.  Return 0 if
 *	no string supplied.
 */
static uintmax_t
var_hash (elt)
char	*elt;
{
	Variable	*v = (Variable *) elt;
	register int	i;
	register int	namelen;
	register uintmax_t	retval = 0;

	if (! v || ! v->variable)
	{
		fprintf(stderr, "Warning: NULL variable or key in var_hash\n");
		return 0;
	}

	namelen = strlen(v->variable);
	for (i = 0; i < namelen; i++)
	{
		retval ^= (v->variable[i] << ((i % 4) * 8));
	}
	return (uintmax_t) retval;
}


