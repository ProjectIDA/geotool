/*
 *    Copyright (c) 1993-1997 Science Applications International Corporation.
 */

/*
 * NAME
 *    origerr_ArrayStructs.h
 *
 * SYNOPSIS
 *    Constants and declarations for the "Origerr" structure
 *    to be used with GDI Array Structs.
 *
 * AUTHOR
 *    Generated by gdi_gen_Astructs 110.2 08/20/97.
 *
 */

#ifndef ORIGERR_ASTRUCTS_H
#define ORIGERR_ASTRUCTS_H

#include "gdi_ArrayStructs.h"
#include "db_origerr.h"


/*
 *  "db30" macros to convert libdb30 calls to libgdi calls
 */
#ifdef USE_DB30_MACROS
#define get_origerr(table_name, where, tuples, maxrecs) \
                     gdi_where_ArrayStructs (gdi_get_stdconn(), (table_name), \
                                             (where), (void *) (tuples), \
                                             (maxrecs), &ORIGERR_CONTAINER_DEF)

#define origerr_Aadd(table_name, tuples, ntuples, qa_flag) \
                     gdi_add_ArrayStructs (gdi_get_stdconn(), (table_name), \
                                           (void *) (tuples), (ntuples), \
                                           &ORIGERR_CONTAINER_DEF)

/*
 *    allocate an empty tuple as a constant.
 */
static ArrayStructsArgs ORIGERR_CONTAINER_DEF =
{
      sizeof(Origerr),
      0,
      (int *) NULL,
      (void *) NULL,
      (void *) &Na_Origerr,
      Origerr_Attr,
      0,
      0,
      "origerr"
};


#endif /* USE_DB30_MACROS */


/*
 *  The query used to create the "Origerr" structures:
 *       SELECT * from origerr
 *
 */
#define Na_Origerr_Init \
{ \
	-1,	/*	orid 	*/ \
	-1.0,	/*	sxx 	*/ \
	-1.0,	/*	syy 	*/ \
	-1,	/*	szz 	*/ \
	-1.0,	/*	stt 	*/ \
	-1.0,	/*	sxy 	*/ \
	-1.0,	/*	sxz 	*/ \
	-1.0,	/*	syz 	*/ \
	-1.0,	/*	stx 	*/ \
	-1.0,	/*	sty 	*/ \
	-1.0,	/*	stz 	*/ \
	-1.0,	/*	sdobs 	*/ \
	-1.0,	/*	smajax 	*/ \
	-1.0,	/*	sminax 	*/ \
	-1.0,	/*	strike 	*/ \
	-1.0,	/*	sdepth 	*/ \
	-1.0,	/*	stime 	*/ \
	0.0,	/*	conf 	*/ \
	-1,	/*	commid 	*/ \
	"19700101 00:00:00"	/*	lddate 	*/ \
}

#define Origerr_Attr_Init \
{ \
	{ "orid",	offsetof(Origerr, orid),	sizeof_(Origerr, orid),	ASlong_t }, \
	{ "sxx",	offsetof(Origerr, sxx),	sizeof_(Origerr, sxx),	ASdouble_t }, \
	{ "syy",	offsetof(Origerr, syy),	sizeof_(Origerr, syy),	ASdouble_t }, \
	{ "szz",	offsetof(Origerr, szz),	sizeof_(Origerr, szz),	ASdouble_t }, \
	{ "stt",	offsetof(Origerr, stt),	sizeof_(Origerr, stt),	ASdouble_t }, \
	{ "sxy",	offsetof(Origerr, sxy),	sizeof_(Origerr, sxy),	ASdouble_t }, \
	{ "sxz",	offsetof(Origerr, sxz),	sizeof_(Origerr, sxz),	ASdouble_t }, \
	{ "syz",	offsetof(Origerr, syz),	sizeof_(Origerr, syz),	ASdouble_t }, \
	{ "stx",	offsetof(Origerr, stx),	sizeof_(Origerr, stx),	ASdouble_t }, \
	{ "sty",	offsetof(Origerr, sty),	sizeof_(Origerr, sty),	ASdouble_t }, \
	{ "stz",	offsetof(Origerr, stz),	sizeof_(Origerr, stz),	ASdouble_t }, \
	{ "sdobs",	offsetof(Origerr, sdobs),	sizeof_(Origerr, sdobs),	ASdouble_t }, \
	{ "smajax",	offsetof(Origerr, smajax),	sizeof_(Origerr, smajax),	ASdouble_t }, \
	{ "sminax",	offsetof(Origerr, sminax),	sizeof_(Origerr, sminax),	ASdouble_t }, \
	{ "strike",	offsetof(Origerr, strike),	sizeof_(Origerr, strike),	ASdouble_t }, \
	{ "sdepth",	offsetof(Origerr, sdepth),	sizeof_(Origerr, sdepth),	ASdouble_t }, \
	{ "stime",	offsetof(Origerr, stime),	sizeof_(Origerr, stime),	ASdouble_t }, \
	{ "conf",	offsetof(Origerr, conf),	sizeof_(Origerr, conf),	ASdouble_t }, \
	{ "commid",	offsetof(Origerr, commid),	sizeof_(Origerr, commid),	ASlong_t }, \
	{ "lddate",	offsetof(Origerr, lddate[0]),	sizeof_(Origerr, lddate),	AScarray_t }, \
	{ (char *) 0 } \
}

#endif /* ORIGERR_ASTRUCTS_H */
