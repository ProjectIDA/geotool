/*
 *    Copyright (c) 1993-1997 Science Applications International Corporation.
 */

/*
 * NAME
 *    db_origin.h
 *
 * SYNOPSIS
 *    The "Origin" structure to be used with the GDI Array Structs.
 *
 * AUTHOR
 *    Generated by gdi_gen_Astructs 110.2 08/20/97.
 *
 */

#ifndef DB_ORIGIN_H
#define DB_ORIGIN_H


/*
 *  The query used to create the "Origin" structures:
 *       SELECT * from origin
 *
 */
typedef struct origin {
	double	lat;
	double	lon;
	double	depth;
	double	time;
	long	orid;
	long	evid;
	long	jdate;
	long	nass;
	long	ndef;
	long	ndp;
	long	grn;
	long	srn;
	char	etype[8];
	double	depdp;
	char	dtype[2];
	double	mb;
	long	mbid;
	double	ms;
	long	msid;
	double	ml;
	long	mlid;
	char	algorithm[16];
	char	auth[16];
	long	commid;
	char	lddate[18];
}Origin;

#endif /* DB_ORIGIN_H */