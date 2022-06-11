/* $Log:	reg.c,v $
 * Revision 1.6  86/08/12  19:09:18  brennan
 * changed abs((short) to abs((int) in nmreg()
 * 
 * Revision 1.5  86/03/10  15:35:14  css
 * updated for removal of symbolic links
 * 
 * Revision 1.4  83/12/08  10:42:46  ncss
 * Changed name of f77 callable gtosf_ to gtos_ for consistency
 * within library and with documentation. This corresponds with
 * calling sequence in outbul.f version 1.3 -- MAB
 * 
 * Revision 1.3  83/12/08  09:35:40  ncss
 * fixed misquoted rcsid string
 * 
 * Revision 1.2  83/12/08  09:10:29  ncss
 * Modified by M.A. Brennan in response to aa bug report #1
 * Added f77 callable function gtos to convert grn to srn
 * Replaced bad installed version of gtos with version that 
 * handles highest region numbers correctly.
 * 
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>

#include "libgeog.h"
#include "reg.h"
#include "regonindex.h"


/* GREG
 * NAME: greg
 *	 greg_		-- f77 callable subroutine --
 *
 * INPUT:	int grn
 *		integer*4 grn			-- f77
 * OUTPUT:	char rgnname[37]
 *		character*37 rgnname		-- f77
 *
 * ABSTRACT:
 * Given a geographic region number and a pointer to a buffer at least 37 bytes
 * long, this subroutine reads a null-terminated 36 byte region name into the 
 * buffer.
 *
 * MODIFIED BY:				DATE:
 * ---------------------------------------------------------------------
 *
 */

int
greg(int number, char *rgnname)
/* int number;*/
/*char *rgnname;*/
{

	if((number < 1) || (number > 729)) {
		strcpy(rgnname,"UNKNOWN GEOGRAPHIC REGION");
		return(FALSE);
	}
	strcpy(rgnname,gregs[number-1]);
	return(TRUE);
}

void
greg_(long int *number,char *rgnname, long int nmlnth)
/*  long int *number, nmlnth;*/
/*  char *rgnname;*/
{
	greg( (int)(*number), rgnname);
}

/* SREG
 * NAME: sreg
 *	 sreg_		-- f77 callable subroutine --
 *
 * INPUT:	int srn
 *		integer*4 srn		-- f77
 * OUTPUT:	char srname[37]
 *		character*37 srname	-- f77
 *
 * ABSTRACT:
 * Given a seismic region number and a pointer to a buffer at least 37 bytes
 * long, this subroutine reads a null-terminated 36 byte region name into the 
 * buffer.
 *
 * MODIFIED BY:				DATE:
 * ---------------------------------------------------------------------
 *
 */

int
sreg(int number, char *rgnname)
     /*	int number;*/
     /*	char *rgnname;*/
{

	if((number < 1) || (number > 50)) {
		strcpy(rgnname,"UNKNOWN SEISMIC REGION");
		return(FALSE);
	}
	strcpy(rgnname,sregs[number-1]);
	return(TRUE);
}

void
sreg_(long int *number,char *rgnname,long int nmlnth)
     /*	long int *number, nmlnth;*/
     /*	char *rgnname;*/
{
	sreg( (int)(*number), rgnname);
}
#define NE	0
#define NW	1
#define SE	2
#define SW	3
#define ERROR	0
int nqrd[4] = { 2, 93, 184, 275 };  /* table offsets for each geographic quadrant */

/* NMREG
 * NAME: nmreg
 *	 nmreg_		-- f77 callable function --
 *
 * INPUT:	float latitude, longitude
 *		real*4 latitude longitude	-- f77
 * OUTPUT:	int grn
 *		integer*4 grn			-- f77
 * 
 * ABSTRACT:
 * Given a latitude and longitude, this function returns the corresponding
 * geographic region number
 *
 * MODIFIED BY:				DATE:
 * ---------------------------------------------------------------------
 *
 */

int
nmreg(double latit,double longit)
     /*	double latit, longit;*/
{
	int offset;
	int i, ilongit;

	if((longit < -180.0) || (longit > 180.0))
		return(ERROR);
	if((latit < -90.0) || (latit > 90.0))
		return(ERROR);

	/* determine which quadrant the supplied location is in and determine
	   the offset to that quadrant in the table */

	if(latit >= 0.0)  {
		if(longit >= 0.0) offset = nqrd[NE];
		else            offset = nqrd[NW];
	}
	else {
		if(longit >= 0.0) offset = nqrd[SE];
		else            offset = nqrd[SW];
	}
	offset = (offset + abs((int)latit) - 1) * 36;

	ilongit = abs((int) longit);
	for(i=0; ilongit > regonindex_table[i + offset].longtd; i++)
		;
	
	if(ilongit < regonindex_table[i + offset].longtd)
		return(regonindex_table[--i + offset].regnumb);
	else
		return(regonindex_table[i + offset].regnumb);
}
long 
nmreg_(float *latit,float *longit)
     /*	float *latit, *longit;*/
{
	return( (long)nmreg((double) *latit,(double) *longit) ) ;
}

/*
 * NAME: gtos	(grn to srn conversion routine)
 *	 gtos_		-- f77 callable function --
 *
 * ABSTRACT: Given geographic region number, this function
 *	     performs a table look up of the corresponding srn in
 *	     the structure gstab defined in file reg.h and returns the
 *	     corresponding srn (seismic region number).
 *
 * INPUT:	int grn -- geographic region number
 *		integer*4 grn			-- f77
 *
 * OUTPUT:	int srn
 *		integer*4			-- f77
 *		function value is srn -- seismic region number
 *		if grn value is invalid, a null value of -1 is
 *		returned in accordance with version 2.6 of the
 *		CSS Database specification.
 *
 * AUTHOR: Mary Ann Brennan		DATE: gtos  Mar. 1983
 *					(gtos_  Nov. 18 1983)
 * MODIFIED BY:				DATE:
 *	Mary Ann Brennan			Nov. 18 1983
 *	incorrect version of gtos which mishandled highest region numbers
 *	had been installed, installed correct version.
 *	added f77 callable function gtos_
 * ---------------------------------------------------------------------
 */

int
gtos (int grn)	/* find srn in gstab from grn */
     /*int grn;*/	/* modified binary search */

{
  int low, high, mid, val;
  
  low = 1;
  high = 51;
  
  if (grn < mingrn)
    return (-1);
  if (grn > maxgrn)
    return (-1);
  
  if (grn >= gstab[high - 1].grn)
    return (high - 1);
  
  while (low <= high) {
    mid = (low + high)/2;
    if (grn < (val = gstab[mid].grn)) {
      if (grn > gstab[mid - 1].grn)
	return (mid - 1);
      else
	high = mid - 1;
    }
    else if (grn > val)
      if (grn < gstab[mid + 1].grn)
	return (mid);
      else
	low = mid + 1;
    else
      return (mid);
  }
  return (-1);
}

long
gtos_(int *grn)
     /*int *grn;*/
{
	int srn;
	srn = gtos(*grn);
	return ((long) srn);
}

