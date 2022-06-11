/*
 *  Copyright 1992 Science Applications International Corporation.
 */

/*
 * NAME    
 *	gdi_ArrayStructs.h
 *
 * DESCRIPTION
 *      Contains defines and structures used by css style arrays
 *	for the Generic Database Interface Library.
 *
 * AUTHOR
 *	Jeff Given 1993
 *
 */

#ifndef _GDI_ARRAYSTRUCTS_
#define _GDI_ARRAYSTRUCTS_

#include <sys/types.h>
#include <stddef.h>

#include "libgdi.h"

#ifdef __cplusplus
extern "C" {
#endif     

/*
 *    offsetof is a macro that is supposedly supported by
 *    ANSI C.  The following gives the right results on the SUN,
 *    but there may be compiler warnings.
 */

#ifndef offsetof
#define offsetof(type, member)   ((size_t) (&(((type *)0)->member)))
#endif

#define sizeof_(str, elem)    sizeof(((str *)0)->elem)

/*
 *    The supported structure element types
 */

typedef enum {AScarray_t, AScptr_t, ASdouble_t, ASfloat_t, 
		   ASint_t, ASlong_t, ASchar_t
	 } AsElem_t; 

/*
 *    The following defines a stucture that holds pertinent information
 *    about each element in a structure.
 */

typedef struct
{
     char     *name;
     size_t   offset;
     int      size;
     AsElem_t type;
} AsElemAttr;

/*
 *    The following structure bundles up all the necessary
 *    information for an array container.
 */
typedef struct
{
     int                size;
     int                nrows;
     int                *col_nums;
     void               *blank_tuple;
     void               *null_tuple;
     AsElemAttr         *elem_attr;
     double             double_val;
     int                alloced_copy;
     char               *name;
} ArrayStructsArgs;

/*
 * prototypes for ArrayStructs constructor function
 * which returns a pointer to an array of structures.
 * (emulates libdb30)
 */

int gdi_get_ArrayStructs(
	dbConn           *dbconn,      /* (i) database connector                  */
	char             *query,       /* (i) database query                      */
	void            **array,       /* (o) pointer to hold array of structures */
	int               maxrecs,     /* (i) max number of rows to return        */
	ArrayStructsArgs *type         /* (i) struct describing data container    */
	);


int gdi_where_ArrayStructs(
	dbConn           *dbconn,      /* (i) database connector                  */
	char             *table_name,  /* (i) database table to select from       */
	char             *where_clause,/* (i) where clause, if any                */
	void            **array,       /* (o) pointer to hold array of structures */
	int               maxrecs,     /* (i) max number of rows to return        */
	ArrayStructsArgs *type         /* (i) struct describing data container    */
	);


int gdi_add_ArrayStructs(
	dbConn           *dbconn,      /* (i) database connector                  */
	char             *table_name,  /* (i) database table name                 */
	void             *array,       /* (i) pointer to array of structures      */
	int               ntuples,     /* (i) number of rows in array             */
	ArrayStructsArgs *type         /* (i) struct describing data container    */
	);


int gdi_print_ArrayStructs(
	dbConn           *dbconn,      /* (i) database connector                  */
	void             *array,       /* (i) pointer to array of structures      */
	int               ntuples,     /* (i) number of rows in array             */
	ArrayStructsArgs *type         /* (i) struct describing data container    */
	);



int gdi_updel_ArrayStructs(
	dbConn           *dbconn,      /* (i) database connector              */
	char             *reserved,    /* (i) unused argument                 */
	char             *updelquery,  /* (i) query with bind variables       */
	void             *array,       /* (i) pointer to array of structures  */
	int               ntuples,     /* (i) number of rows in array         */
	ArrayStructsArgs *type         /* (i) struct describing data container*/
	);


int gdi_var_ArrayStructs(
	dbConn           *dbconn,      /* (i) database connector                   */
	char             *varquery,    /* (i) query with i/o bind variables        */
	void             *in_array,    /* (i) array of structs for input variables */
	int               in_ntuples,  /* (i) number of rows in input array        */
	ArrayStructsArgs *in_type,     /* (i) struct describing input array        */
	void            **out_array,   /* (o) array of structs for output vars     */
				       /*     or select list                       */
	int               out_maxrecs, /* (i) maximum number of rows to return     */
	ArrayStructsArgs *out_type     /* (i) struct describing output array       */
	);


#ifdef __cplusplus
}
#endif     

#endif  /* _GDI_ARRAYSTRUCTS_ */


