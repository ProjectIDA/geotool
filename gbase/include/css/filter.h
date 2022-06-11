/*	SccsId:	%W%	%G%	*/
/**
 *	Filter relation.
 */
#ifndef _FILTER__H
#define _FILTER__H

#define FILTER_LEN	137

/** 
 *  Filter structure.
 *  @member sta Station. Initial value = "-".
 *  @member chan Channel. Initial value = "-".
 *  @member chanid Channel id. Initial value = -1.
 *  @member arid Arrival id. Initial value = -1.
 *  @member wfid Waveform id. Initial value = -1.
 *  @member band lp, hp, bp, or br. Initial value = "-".
 *  @member ftype 'z': zero phase, 'c': causal. Initial value = "-".
 *  @member forder Filter order. Initial value = -1.
 *  @member lofreq Low cut frequency (Hz). Initial value = -1.
 *  @member hifreq High cut frequency (Hz). Initial value = -1.
 *  @member algo Filter algorithm. Initial value = "-".
 *  @member program Program which uses filter. Initial value = "-".
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	char	sta[7];		/* station			*/
	char	chan[9];	/* channel			*/
	long	chanid;		/* channel id			*/
	long	arid;		/* arrival id			*/
	long	wfid;		/* waveform id			*/
	char	band[3];	/* lp, hp, bp, or br		*/
	char	ftype[2];	/* 'z': zero phase, 'c': causal	*/
	int	forder;		/* filter order			*/
	float	lofreq;		/* low cut frequency (Hz)	*/
	float	hifreq;		/* high cut frequency (Hz)	*/
	char	algo[31];	/* filter algorithm		*/
	char	program[16];	/* program which uses filter	*/
	char	lddate[18];	/* load date			*/
} FILTER;

#define FILTER_RCS "%6c%*c%8c%*c%8ld%*c%8ld%*c%8ld%*c%2c%*c%c%*c%4d%*c%9f%*c%9f%*c%30%*c%15c%*c%17c%"

#define FILTER_RVL(SP) \
(SP)->sta, (SP)->chan, &(SP)->chanid, &(SP)->arid, &(SP)->wfid, (SP)->band, \
(SP)->ftype, &(SP)->forder, &(SP)->lofreq, &(SP)->hifreq, (SP)->algo, \
(SP)->program, (SP)->lddate

#define FILTER_WCS "%-6.6s %-8.8s %8ld %8ld %8ld %-2.2s %-1.1s %4d %9.4f %9.4f %-30.30s %-15.15s %-17.17s\n"

#define FILTER_WVL(SP) \
(SP)->sta, (SP)->chan, (SP)->chanid, (SP)->arid, (SP)->wfid, (SP)->band, \
(SP)->ftype, (SP)->forder, (SP)->lofreq, (SP)->hifreq, (SP)->algo, \
(SP)->program, (SP)->lddate

#define FILTER_NULL \
{ \
"-",			/* sta			*/ \
"-",			/* chan			*/ \
-1,			/* chanid 		*/ \
-1,			/* arid			*/ \
-1,			/* wfid 		*/ \
"-",			/* band			*/ \
"-",			/* ftype		*/ \
-1,			/* forder		*/ \
-1.,			/* lofreq		*/ \
-1.,			/* hifreq		*/ \
"-",			/* algo			*/ \
"-",			/* program 		*/ \
"-"			/* load date		*/ \
}

#endif
