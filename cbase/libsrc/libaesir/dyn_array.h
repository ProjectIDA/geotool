/*
 * SccsId:	@(#)dyn_array.h	105.1	19 May 1995
 */

#ifndef _TYPES_
#include <sys/types.h>
#endif /* _TYPES_ */

#ifndef _DYN_ARRAY_H_
#define _DYN_ARRAY_H_
typedef struct 
{
	int		ar_size;	/* size of each element */
	caddr_t		ar_list;	/* the list of elements */
	int		ar_count;	/* number of elements in list */
	int		ar_allocated;	/* number of slots in list */
	int		ar_allocate_count; /* how many slots to allocate */
} ArrayRec, *Array;

/* Number of elements to allocated per malloc */
#define ARRAY_DEF_ALLOCATE	5

#define ARRAY_INDEX(a, index)	((a)->ar_list + ((a)->ar_size * (index)))

extern Array array_create (int size);
extern void array_free (Array array);
extern int array_count (Array array);
extern caddr_t array_list (Array array);
extern caddr_t array_add (Array array, caddr_t element);
extern caddr_t array_delete (Array array, caddr_t element);
extern void array_setrate (Array array, int rate);

extern caddr_t array_add_n (Array array, caddr_t elements, int n);
extern void array_destroy (Array a);
extern Array array_copy (Array a);
extern Array array_init (caddr_t a, int n, int size);



#endif /* _DYN_ARRAY_H_ */
