/*	SccsId:	%W%	%G%	*/

#define ALLOC(new_size, old_size, ptr, type, extra)	\
	if((new_size) > (unsigned int)(old_size) )				\
	{							\
		if(ptr == (type *)0)				\
		{						\
			ptr = (type *)malloc(new_size+extra);\
		}						\
		else						\
		{						\
			ptr = (type *)realloc(ptr, new_size+extra);\
		}						\
		if(ptr == (type *)0)				\
		{						\
			perror("malloc error");			\
			exit(1);				\
		}						\
		else						\
		{						\
			old_size = new_size+extra;		\
		}						\
	}
