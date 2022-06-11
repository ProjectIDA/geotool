/*	SccsId:	%W%	%G%	*/
/*
 *	Pick relation.
 */
#ifndef _PICK__H
#define _PICK__H

#define PICK_LEN	142

/** 
 *  Pick structure.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member chanid Channed id. Initial value = -1.
 *  @member time Epoch time of measurement. Initial value = -9999999999.999.
 *  @member arid Arrival id. Initial value = -1.
 *  @member amptype pp, zp or rms. Initial value = "-".
 *  @member amp Amplitude in counts. Initial value = -1.
 *  @member per Period. Initial value = -1.
 *  @member calib Calibration factor at calper. Initial value = 0.
 *  @member calper Calibration period. Initial value = -1.
 *  @member ampcalib Calibration at per. Initial value = 0.
 *  @member ampmin Min data value(cnts) for pp measurement. Initial value = -1.
 *  @member commid Comment id. Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	sta[7];		/* station			*/
	char	chan[9];	/* channel			*/
	long	chanid;		/* channed id			*/
	double	time;		/* epoch time of measurement	*/
	long	arid;		/* arrival id			*/
	char	amptype[4];	/* pp, zp or rms		*/
	float	amp;		/* amplitude in counts		*/
	float	per;		/* period			*/
	float	calib;		/* calibration factor at calper	*/
	float	calper;		/* calibration period		*/
	float	ampcalib;	/* calibration at per		*/
	float	ampmin;		/* min data value(cnts) for pp measurement */	
	long	commid;		/* comment id			*/
	char	lddate[18];	/* load date			*/
} PICK;

#define PICK_NULL \
{ \
"-",			/* sta				*/ \
"-",			/* cha				*/ \
-1,			/* chanid			*/ \
-9999999999.999,	/* time				*/ \
-1,			/* arid				*/ \
"-",			/* amptype			*/ \
-1.,			/* amp				*/ \
-1.,			/* per				*/ \
0.,			/* calib			*/ \
-1.,			/* calper			*/ \
0.,			/* ampcalib			*/ \
-1.,			/* ampmin			*/ \
-1,			/* comment id			*/ \
"-",			/* load date			*/ \
}

#define PICK_WCS "%-6.6s %-8.8s %8ld %17.5lf %8ld %-3.3s %10f %10f %10f \
%10.1f %8ld %-17.17s\n"

#endif /* _PICK__H */
