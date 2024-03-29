/*
 *    Copyright (c) 1993-1997 Science Applications International Corporation.
 */

/*
 * NAME
 *    db_parrival.h
 *
 * SYNOPSIS
 *    The "Parrival" structure to be used with the GDI Array Structs.
 *
 * AUTHOR
 *    Generated by gdi_gen_Astructs 110.2 08/20/97.
 *
 */

#ifndef DB_PARRIVAL_H
#define DB_PARRIVAL_H

#pragma ident "@(#)array_structs/db_parrival.h	110.1 10/16/97 Copyright (c) 1993-1997 SAIC"


/*
 *  The query used to create the "Parrival" structures:
 *       SELECT * from parrival
 *
 */
typedef struct parrival {
	long	parid;
	long	orid;
	long	evid;
	char	sta[7];
	double	time;
	double	azimuth;
	double	slow;
	char	phase[9];
	double	delta;
	char	vmodel[16];
	char	lddate[18];
}Parrival;

#endif /* DB_PARRIVAL_H */
