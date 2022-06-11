#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libgmath.h"
#include "logErrorMsg.h"

#define BINARY_TABLE_HOLD

static FILE	*fp = NULL;
static char	*jbtable = NULL;
static int	new_table = 1;

static int get_record(FILE *fp, int record, float *delta, unsigned short *d);
static void flip2(char *c);
static void flip4(char *c);

/**
 * Open a JB model file.
 * @param table The filename.
 * @return 0 for success, CANNOT_OPEN_JBTABLE for failure and call logErrorMsg.
 */
int
jbopen(const char *table)
{
#ifndef TABLE_INCLUDED
	if(fp != NULL)
	{
		fclose(fp);
	}
	new_table = 1;
	if(jbtable != NULL) free(jbtable);
	jbtable = (char *)malloc(strlen(table)+1);
	if(jbtable == NULL) {
	    logErrorMsg(LOG_ERR, "jbopen: malloc failed.");
	    return -1;
	}
	strcpy(jbtable, table);
	if((fp = fopen(jbtable, "r")) == NULL)
	{
	    char msg[1500];
	    snprintf(msg, 1500, "jbopen: cannot open %s", table);
	    logErrorMsg(LOG_WARNING, msg);
	    return(CANNOT_OPEN_JBTABLE);
	}
#endif
	return(0);
}

/** 
 * Compute JB travel time and derivatives. The travel time derivates computed
 * are:
 *  <pre>
 *	dd->dtdh : d(t)/d(depth) : sec/km
 *	dd->dtdd : d(t)/d(delta) : sec/deg
 *	dd->dpdd : d(dtdd)/d(delta) : (sec/deg)/deg
 *	dd->dpdh : d(dtdd)/d(depth) : (sec/deg)/km
 * </pre>
 * <p>
 * The phase_code is one of:
 * <pre>
 *     1 P       8 ScS
 *     2 PP      9 PKPab
 *     3 S      10 PKPbc
 *     4 SS     11 PKPdf
 *     5 PcP    15 SKSac
 *     6 pP     17 SKSdf
 *     7 sP
 * </pre>
 *
 * The return codes are:
 * <pre>
 *  0                     success
 *  JBPHASE_UNKNOWN       table not found
 *  DELTA_OUTSIDE_RANGE   delta outside range
 *  DEPTH_OUTSIDE_RANGE   depth outside range
 *  CANNOT_OPEN_JBTABLE   can't open jbtable
 *  JBTABLE_READ_ERROR    read error. (file corrupted?)
 * </pre>
 * Calls logErrorMsg for malloc errors and file format errors.
 * <p>
 * Algorithm from subroutine jbtab.f, June 1977 by D.W. McCowan (llab)
 *
 *  @param phase_code Phase code.
 *  @param delta  Epicentral distance in degrees.
 *  @param depth  Focal depth in km.
 *  @param *ttime Returned travel time in secconds.
 *  @param *dd Returned travel time derivatives.
 *  @return 0 for success, nonzero return code for failure (see above).
 */
int
jbsim(int phase_code, double delta, double depth, float *ttime, Derivatives *dd)
{
	int i, j, ipos, record, rec2;
	unsigned short d[14], ilow[14], ihih[14];
	double dtdd2, delta_mid, delta_mid2;
	double deltd, delth, depthl, f00, f01, f10, f11;
	float delta_lo, delta_hi, delta2;

	static double depths[] =
	{
      		  0.00,  33.0 ,  96.38, 159.76, 223.14, 286.52, 349.90,
		413.28, 476.66, 540.04, 603.42, 666.80, 730.18, 793.56
	};

	static struct
	{
		int	id;
		int	loc;
		float	delta_min;
		float	delta_max;
	} *p, phase[] =
	{
		{  1,    2,   0.0, 105.0},	/* P     */
		{  2,  119,   0.0, 210.0},	/* PP    */
		{  3,  331,   0.0, 107.0},	/* S     */
		{  4,  450,   0.0, 214.0},	/* SS    */
		{  5,  666,   0.0, 100.0},	/* PcP   */
		{  8,  768,   0.0, 100.0},	/* ScS   */
		{  9,  870, 141.0, 180.0},	/* PKPab */
		{ 10,  911, 141.0, 147.0},	/* PKPbc */
		{ 11,  919, 109.0, 180.0},	/* PKPdf */
		{ 15,  992,  62.0, 133.0},	/* SKSac */
		{ 17, 1065,  99.0, 180.0},	/* SKSdf */
		{  6, 1148,   0.0, 105.0},	/* pP    */
		{  7, 1265,   0.0, 105.0},	/* sP    */
	};

	*ttime = 0.0;
	for(ipos = 0; ipos < 13; ipos++)
	{
		if(phase_code == phase[ipos].id) break;
	}
	if(ipos == 13) return(JBPHASE_UNKNOWN);

	if(delta < phase[ipos].delta_min || delta > phase[ipos].delta_max)
	{
		return(DELTA_OUTSIDE_RANGE);
	}
	if(depth < -250.0 || depth > depths[13])
	{
		return(DEPTH_OUTSIDE_RANGE);
	}
	if(fp == NULL)
	{
		logErrorMsg(LOG_WARNING, "jbsim: jbtable not open");
		return(CANNOT_OPEN_JBTABLE);
	}

	p = &phase[ipos];

	if(phase_code == 1 || phase_code == 3 || /* P or S */
	   phase_code == 6 || phase_code == 7)	 /* pP or sP */
	{
		if(delta > 10.)
		{
			record = p->loc + 10 + (int)(delta - p->delta_min);
		}
		else
		{
			record = p->loc + (int)(2.0*(delta - p->delta_min));
		}
	}
	else
	{
		record = p->loc + (int)(delta - p->delta_min);
	}
	if(delta == p->delta_max) record -= 1;

	rec2 = (record > p->loc) ? record-1 : record+2;

	if(get_record(fp, record,   &delta_lo, ilow) || 
	   get_record(fp, record+1, &delta_hi, ihih) ||
	   get_record(fp, rec2, &delta2, d))
	{
		fclose(fp);
		fp = NULL;
		return(JBTABLE_READ_ERROR);
	}

	if(delta < delta_lo || delta > delta_hi)
	{
		return(DELTA_OUTSIDE_RANGE);
	}
	/*
	 * find correct depth values
	 */
	if(depth <= 0.)
	{
		i = 0;
	}
	else
	{
		for(i = 0; i < 13; i++)
		{
			if(depth >= depths[i] && depth <= depths[i+1]) break;
		}
	}
	/*
	 * check limits and interpolate
	 */
	if(ilow[i] == 0 || ilow[i+1] == 0 || ihih[i] == 0 || ihih[i+1] == 0)
	{
		return(DELTA_OUTSIDE_RANGE);
	}
	f00 = (double)ilow[i]/10.0;
	f01 = (double)ihih[i]/10.0;
	f10 = (double)ilow[i+1]/10.0;
	f11 = (double)ihih[i+1]/10.0;
	delth = depths[i+1] - depths[i];
	deltd = delta_hi - delta_lo;
	depthl = depths[i];

	*ttime = (f00*delth*deltd + (depth-depthl)*deltd*(f10-f00) +
		 (delta-delta_lo)*delth*(f01-f00) +
		 (depth-depthl)*(delta-delta_lo)
		*(f00-f01-f10+f11))/(delth*deltd);
	dd->dtdh = (deltd*(f10-f00) + (delta-delta_lo)*(f00-f01-f10+f11))/
			(delth*deltd);
	dd->dtdd = (delth*(f01-f00) + (depth-depthl)*(f00-f01-f10+f11))/
			(delth*deltd);
	dd->dpdh = (f00-f01-f10+f11)/(delth*deltd);

	/* compute dpdd = d(dtdd)/d(delta), using another delta below delta_lo
	 * or above delta_hi
	 */
	delta_mid = .5*(delta_lo + delta_hi);
	if(rec2 < record)
	{
		delta_hi = delta_lo;
		delta_lo = delta2;
		for(j = 0; j < 14; j++)
		{
			ihih[j] = ilow[j];
			ilow[j] = d[j];
		}
	}
	else
	{
		delta_lo = delta_hi;
		delta_hi = delta2;
		for(j = 0; j < 14; j++)
		{
			ilow[j] = ihih[j];
			ihih[j] = d[j];
		}
	}
	delta_mid2 = .5*(delta_lo + delta_hi);

	f00 = (double)ilow[i]/10.0;
	f01 = (double)ihih[i]/10.0;
	f10 = (double)ilow[i+1]/10.0;
	f11 = (double)ihih[i+1]/10.0;
	delth = depths[i+1] - depths[i];
	deltd = delta_hi - delta_lo;
	depthl = depths[i];

	dtdd2 = (delth*(f01-f00) + (depth-depthl)*(f00-f01-f10+f11))/
			(delth*deltd);
	
	dd->dpdd = (dd->dtdd - dtdd2)/(delta_mid - delta_mid2);

	return(0);
}

#ifdef TABLE_INCLUDED

#include "jbtable.h"

static int
get_record(FILE *fp, int record, float *delta, unsigned short *d)
{
	char *pos;

	pos = (char *)table + (record-1)*(sizeof(float) + 14*sizeof(short));

	memcpy(delta, pos, sizeof(float));
	pos += sizeof(float);
	memcpy(d, pos, 14*sizeof(short));

	return(0);
}
#endif

#define MAGIC 857035143

#ifdef BINARY_TABLE_HOLD

static int
get_record(FILE *fp, int record, float *delta, unsigned short *d)
{
	static char *table = NULL;
	int i, magic;
	char *pos;
	static int flipbytes = 0;

	if(new_table)
	{
	    new_table = 0;
	    if(table != NULL) free(table);
	    if((table = (char *)malloc(44160)) == NULL)
	    {
		logErrorMsg(LOG_ERR, "jbsim: malloc error.");
		return(1);
	    }
	    if(fread(&magic, sizeof(int), 1, fp) != 1) return(1);

	    if(magic != 857035143)
	    {
		/* check for PC-ordered words */
		flip4((char*)&magic);
		if(magic == 857035143)
		{
			flipbytes = 1;
		}
		else
		{
		    char msg[1500];
		
		    snprintf(msg, 1500, "jbsim: file format error: %s",jbtable);
		    logErrorMsg(LOG_WARNING, jbtable);
		    return(1);
		}
	    }
	    if(fread(table, 1, 44160, fp) != 44160) return(1);
	    fclose(fp);
	}
	if(table == NULL) return(1);

	pos = table + (record-1)*(sizeof(float) + 14*sizeof(short));

	memcpy(delta, pos, sizeof(float));
	pos += sizeof(float);
	memcpy(d, pos, 14*sizeof(short));

	if(flipbytes)
	{
	    flip4((char*)delta);
	    for(i = 0; i < 14; i++) flip2((char *)&d[i]);
	}

	return(0);
}

#endif

#ifdef BINARY_TABLE

static int
get_record(FILE *fp, int record, float *delta, unsigned short *d)
{

	if(fseek(fp, (record-1)*(sizeof(float) + 14*sizeof(short)), 0) == -1)
	{
		return(1);
	}
	if(fread(delta, sizeof(float), 1, fp) != 1) return(1);
	if(fread(d, sizeof(short), 14, fp) != 14) return(1);

	return(0);
}

#endif

#ifdef ASCII_TABLE

static int
get_record(FILE *fp, int record, float *delta, unsigned short *d)
{
	int	i;
	char	a[3], b[4], del[6];
	short	ia, ib;

	if(fseek(fp, (record-1)*81, 0) == -1)
	{
		return(1);
	}
	del[5] = '\0';
	if(fscanf(fp, "%5c", del) != 1) return(1);
	if(sscanf(del, "%f", delta) != 1) return(1);

	for(i = 0; i < 5; i++) if(fgetc(fp) == EOF) return(1);

	a[2] = b[3] = '\0';
	for(i = 0; i < 14; i++)
	{
		if(fscanf(fp, "%2c%3c",a, b) != 2) return(1);
		ia = (a[0] != ' ' || a[1] != ' ') ? atoi(a) : 0;
		ib = (b[0] != ' ' || b[1] != ' ' || b[2] != ' ') ? atoi(b) : 0;

		d[i] = 600*ia + ib;
	}
	return(0);
}

#endif

static void
flip2(char *c)
{
	char a;

	a = c[0];
	c[0] = c[1];
	c[1] = a;
}

static void
flip4(char *c)
{
	char a[4];

	memcpy(a, c, 4);
	c[0] = a[3];
	c[1] = a[2];
	c[2] = a[1];
	c[3] = a[0];
}
