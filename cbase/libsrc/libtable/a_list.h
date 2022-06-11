/*
 *	SccsId:	@(#)libtable/a_list.h	104.1	07/08/97
 */
#ifndef A_LIST_H
#define	A_LIST_H


	/* Structure for holding list nodes. */
typedef struct list_node
{
	caddr_t			data;
	struct list_node	*prev, *next;
} LIST_NODE;

	/* Structure to hold list information. */
typedef struct list_head
{
	LIST_NODE	*first, *current, *last;
	int		count;
} LIST_HEAD;


#ifdef __STDC__

typedef   int   (*LIST_COMPARE) (const void *, const void *);
typedef   void  (*LIST_FREE)(caddr_t data);
typedef   void	(*LIST_PRINT)(caddr_t data);

extern void	add_element (LIST_HEAD *list, caddr_t element, 
				     LIST_COMPARE compare);
extern void     append_element (LIST_HEAD *list, caddr_t element);
extern void		prepend_element (LIST_HEAD *list, caddr_t element);
extern char 		*find_element (LIST_HEAD *list, caddr_t key, 
				     LIST_COMPARE compare);
extern void 		delete_element (LIST_HEAD *list, caddr_t data, 
				     LIST_COMPARE compare);
extern LIST_HEAD 	*create_list (void);
extern void 		start_list (LIST_HEAD *list);
extern void 		end_list (LIST_HEAD *list);
extern char		*next_list (LIST_HEAD *list);
extern char 		*prev_list (LIST_HEAD *list);
extern void 		destroy_list (LIST_HEAD *list, LIST_FREE free_data);
extern void 		print_list (LIST_HEAD *list, LIST_PRINT print);
extern int		list_count(LIST_HEAD *list);

#else /* ! __STDC__ */

typedef   int      (*LIST_COMPARE) ();
typedef   void     (*LIST_FREE)();
typedef   void	   (*LIST_PRINT)();

extern void		append_element();
extern void		prepend_element();
extern void 		add_element();
extern char 		*find_element();
extern void 		delete_element();
extern LIST_HEAD 	*create_list();
extern void 		start_list();
extern void 		end_list();
extern char 		*next_list();
extern char 		*prev_list();
extern void 		destroy_list();
extern void 		print_list();
extern int		list_count();

#endif	/* __STDC__ */	
#endif
