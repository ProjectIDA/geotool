#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"a_list.h"


#ifdef	SCCSID
static char SccsId[] = "@(#)libtable/a_list.c	104.1 07/08/97 Copyright 1992 SAIC";
#endif

/*
 * NAME
 * 	liblist - A library of generic list handling functions.
 *
 * SYNOPSIS 
 *	Liblist has a variety of functions to create, maintain, inspect
 * 	and destroy a list.  The list is implemented as a linked list, and 
 *	all internal components are hidden from the usr.
 *
 * DESCRIPTION
 * 	Liblist maintains a double linked list of LIST_NODE's which each 
 *	have a pointer to the data, the next LIST_NODE structure and the
 * 	previous LIST_NODE structure.  A LIST_HEAD structure maintains
 *	the pointers to the first, last and current LIST_NODE structures,
 *	as well as a count of the number of items currently in the list.
 *	The current pointer is used for routines which allow the user to 
 *	sequentionally step through the list.  The user is returned a 
 * 	pointer to a LIST_HEAD structure when a list is created, which is 
 *	used as a handle to the instance of list created.  The user must
 * 	pass in pointers to functions that access the users data in many 
 *	routines.  Such routines must compare two instances of the data where
 *	a positive number is returned if the first instance is greater than
 * 	the second, a negative if it is less than, and a zero if they are
 * 	equal. 
 *
 * DIAGNOSTICS
 * 	What happens to the current element pointer if that element is
 * 	removed is not obvious, however in this implementation the pointer
 *	is updated to point to the next item in the list.  
 *
 * FILES
 *
 * NOTES
 *
 * SEE ALSO
 *
 * AUTHOR
 * 	Alex Olender - Science Applications International Corporation.
 *
 */

#define UALLOC(type, count)	(type*) calloc((unsigned)(count), sizeof(type))
#define UFREE(ptr)		if (ptr)\
				{	(void) free ((char *) (ptr));\
					(ptr) = NULL;\
				}


        /*
	 * data_cmp() - Used to arbitrarily sort data.
   	 */

static
int
data_cmp(t1, t2)
caddr_t t1, t2;
{
	return((int) (t1 - t2));
}



	/*
	 * add_node() - Add a node to a list using the user's compare routine 
	 * as a means of sorting the data.
   	 */

static void
add_node(list, node, compare)
	LIST_HEAD	*list;
	LIST_NODE	*node;
        LIST_COMPARE    compare;
{
	LIST_NODE	*current_node;

	if (list == NULL)
		return;

		/* If list is empty, insert node. */
	if (list->first == NULL)
	{
		list->first = list->last = node;
		list->count++;
		return;
	}

		/* Check if node is less than first in list. */	
	else if ((*compare)(node->data, list->first->data) < 0) 
	{
		node->next = list->first;
		list->first->prev = node;
		list->first = node;
		list->count++;
		return;
	}

		/* Check if node is greater than last in list. */	
	else if ((*compare)(node->data, list->last->data) >= 0) 
	{
		node->prev = list->last;
		list->last->next = node;
		list->last = node;
		list->count++;
		return;
	}

		
		/* Step through list looking for a spot. */
	current_node = list->first;
	while (current_node != NULL) 
	{
		if ((*compare)(node->data, current_node->data) < 0) 
		{
			node->next = current_node;
			node->prev = current_node->prev;
			current_node->prev = node;
			node->prev->next = node;
			current_node = NULL;
		}
		else
			current_node = current_node->next;
	}
	list->count++;

	return;
}


	/*
	 * prepend_node() - Add a node to the beginning of the list.
	 */

static void
prepend_node(list, node)
	LIST_HEAD	*list;
	LIST_NODE	*node;
{
	if (list == NULL)
		return;

		/* If list is empty, insert node */
	if (list->first == NULL)
	{
		list->first = list->last = node;
		list->count++;
		return;
	}

		/* put at beginning of list */	
	else
	{
		node->next = list->first;
		list->first->prev = node;
		list->first = node;
		list->count++;
		return;
	}
}



	/*
	 * append_node() - Add a node to the end of the list.
	 */

static void
append_node(list, node)
	LIST_HEAD	*list;
	LIST_NODE	*node;
{
	if (list == NULL)
		return;

		/* If list is empty, insert node */
	if (list->first == NULL)
	{
		list->first = list->last = node;
		list->count++;
		return;
	}

		/* put at end of list */	
	else
	{
		node->prev = list->last;
		list->last->next = node;
		list->last = node;
		list->count++;
		return;
	}
}



	/*
	 * remove_node() - Remove a node from the list.
	 */

static void
remove_node(list, node)
	LIST_HEAD	*list;
	LIST_NODE	*node;
{
	if (list == NULL)
		return;

	if (list->first == node) 	/* first in list */
	{
		list->first = node->next;
		if (list->first != NULL)
			list->first->prev = NULL;
		if (list->current == node)
			list->current = list->first;
	}	

	else if (list->last == node) 	/* last in list */
	{
		list->last = node->prev;
		if (list->last != NULL)
			list->last->next = NULL;
		if (list->current == node)
			list->current = list->last;
	}	

	else				/* somewhere in the middle */ 
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
		if (list->current == node)
			list->current = node->next;
	}

	list->count--;	/* Decrement the number of items in list. */
	UFREE(node);
}



	/*
	 * find_node() - Find a node which contains the data that has the 
	 * key using the compare function.
	 */

static LIST_NODE *
find_node(list, key, compare)
LIST_HEAD	*list;
caddr_t		key;
LIST_COMPARE compare;
{
	LIST_NODE	*node;
	
	if (list == NULL)
		return(NULL);

		/* Start looking at beginning of list. */
	node = list->first;

		/* Step through list until compare function finds a match. */
	while (node != NULL) {
		if ((*compare)(node->data, key) == 0)
			return(node);
		node = node->next;
	}

	return(NULL);
}



	/*
	 * create_list() - Allocate the memory for and initialize values in
	 * the LIST_HEAD structure.
   	 */

LIST_HEAD *
create_list ()
{
	LIST_HEAD	*list;

	list = UALLOC (LIST_HEAD, 1);
	list->first = list->last = list->current = NULL;
	list->count = 0;

	return(list);
}



	/*
	 * add_element() - Add an element to the list using 'compare' as a sort 
	 * function.
   	 */

void
add_element(list, element, compare)
	LIST_HEAD	*list;
	caddr_t		element;
        LIST_COMPARE    compare;
{
	LIST_NODE	*node; 

	if (list == NULL)
		return;

		/* Allocate and initialize a node. */
	node = UALLOC (LIST_NODE, 1);
	node->next = node->prev = NULL;
	node->data = element;

		/* If user's compare function is null, use data_cmp. */
	if (compare == NULL)
		add_node(list, node, data_cmp);
	else
		add_node(list, node, compare);
}



	/*
	 * prepend_element() - Add an element to the beginning of the list.
	 */

void
prepend_element(list, element)
	LIST_HEAD	*list;
	caddr_t		element;
{
	LIST_NODE	*node; 

	if (list == NULL)
		return;

		/* Allocate and initialize a node. */
	node = UALLOC (LIST_NODE, 1);
	node->next = node->prev = NULL;
	node->data = element;

	prepend_node(list, node);
}



	/*
	 * append_element() - Add a element to the end of the list.
	 */

void
append_element(list, element)
	LIST_HEAD	*list;
	caddr_t		element;
{
	LIST_NODE	*node; 

	if (list == NULL)
		return;

		/* Allocate and initialize a node. */
	node = UALLOC (LIST_NODE, 1);
	node->next = node->prev = NULL;
	node->data = element;

	append_node(list, node);
}



	/*
	 * delete_element() - Delete the element which contains data
	 * using the compare routine.
	 */ 

void
delete_element(list, data, compare)
	LIST_HEAD	*list;
	caddr_t		data;
        LIST_COMPARE    compare;
{
	LIST_NODE	*node;

	if (list == NULL)
		return;

		/* If compare function is NULL, use data_cmp. */
	if (compare == NULL)
		node = find_node(list, data, data_cmp);
	else
		node = find_node(list, data, compare);

	if (node != NULL)
		remove_node(list, node);
}



	/*
	 * find_element() - Find a element which contains the data that has the 
	 * key using the compare function.
	 */

caddr_t
find_element(list, key, compare)
	LIST_HEAD	*list;
	caddr_t		key;
        LIST_COMPARE    compare;
{
	LIST_NODE	*node;

	if (list == NULL)
		return(NULL);

		/* Find node and return the data if node exists. */
	node = find_node(list, key, compare);
	if (node != NULL)
		return(node->data);
	else
		return(NULL);
}



	/*
	 * start_list() - Set the current list pointer to the first element
	 * in the list.
	 */ 

void
start_list(list)
	LIST_HEAD	*list;
{
	if (list == NULL)
		return;

	list->current = list->first;
}



	/*
	 * end_list() - Set the current list pointer to the last element
	 * in the list.
	 */ 

void
end_list(list)
	LIST_HEAD	*list;
{
	if (list == NULL)
		return;

	list->current = list->last;
}



	/*
	 * next_list() - Set the current list pointer to the next element
	 * in the list.
	 */ 

caddr_t
next_list(list)
	LIST_HEAD	*list;
{
	caddr_t	ret = NULL;

	if (list == NULL)
		return(NULL);

	if (list->current != NULL) {
		ret = list->current->data;
		list->current = list->current->next;
	}
	return(ret);
}



	/*
	 * prev_list() - Set the current list pointer to the previous element
	 * in the list.
	 */ 

caddr_t
prev_list(list)
	LIST_HEAD	*list;
{
	caddr_t	ret = NULL;

	if (list == NULL)
		return(NULL);

	if (list->current != NULL) {
		ret = list->current->data;
		list->current = list->current->prev;
	}
	return(ret);
}



	/*
	 * destroy_list() - Free all the data associated with the list using
	 * free data to free the user's data.
	 */ 

void
destroy_list(list, free_data)
	LIST_HEAD	*list;
        LIST_FREE       free_data;
{
	LIST_NODE	*node;

	if (list == NULL)
		return;

	node = list->first;
	while (node != NULL) {
		node = node->next;
		if (free_data != NULL)
			free_data(list->first->data);
		UFREE(list->first);
		list->first = node;
	}
	UFREE(list);
}



	/* 
	 * print_list() - Print the contents of a list using the users print
	 * routine
	 */

void
print_list(list, print)
LIST_HEAD	*list;
LIST_PRINT       print;
{
	caddr_t		data;

	if (list == NULL)
		return;

	start_list(list);

	printf("----\n");
	data = next_list(list);
	while (data != NULL) {
		print(data);
		data = next_list(list);
	}
}

	/*
	 * list_count() - Return the number of items in a list.
	 */

int
list_count(list)
LIST_HEAD	*list;
{
	if (list == NULL)
		return(-1);

	return(list->count);
}
