/*
 * NAME
 *      ftoa:	convert double to nice string
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "libgmath.h"

#define MAXC 15		/* the maximum number of output characters */
#define MAXE  8		/* force an e-format if fabs(f) < 10**(-MAXE) */

/** 
 *  Convert a double to a character string.
 *  @param f (input) floating point number to convert to characters.
 *  @param ndeci (input) desired # of digits to the right of the decimal.
 *  @param fixLength (input) if 0, the trailing '0's or '.' are removed.
 *  @param s (output) character string representing f.
 *  @param len (input) The length of <b>s</b>.
 */
void
ftoa(double f, int ndeci, int fixLength, char *s, int len)
{
	int i, n, ndc, lg;
	double af;
	char t[31], *p;

	if(f == 0.) /* special case */
	{ 
	    if(len >= 2) {
		s[0] = '0';
		s[1] = '\0';
	    }
	    return;
	}
	ndc = (ndeci > 0) ? ndeci : 0;
	if(ndc >= 30) ndc = 29;
	af = fabs(f);
	/* get the number of characters needed */
	lg = (int)(.999999*log10(af));
	if((f > 0. && lg < -(MAXE-1)) || (f < 0. && lg < -(MAXE-2)))
	{
	    /* f is too small for MAXE characters force an e-format print */
	    snprintf(s, len, "%8.1e", f);
	    if(len > 0) s[len-1] = '\0';
	    return;
	}
	else
	{
		if(lg >= 0)
		{
			i = lg + 1 + 1;
			if(f < 0.) i++;
			if(i <= MAXC + 1 && i + ndc > MAXC)
			{
				ndc = MAXC - i;
				if(f < 0.) ndc--;
				if(ndc < 0) ndc = 0;
				i = MAXC;
			}
		}
		else
		{
			if(lg <= -ndc)
			{
				ndc = abs(lg) + 1;
			}
			i = 1 + ndc;
			if(f < 0.) i++;
		}
	}
	t[30] = '\0';
	if(i > MAXC)
	{
	    /* too many characters needed. use the e format */
	    snprintf(t, 30, "%8.1e", f);
	    n = strlen(t);
	}
	else
	{
	    /* shift the digits of af to the right ndeci places,
	     * then round off the last digit
	     */
	    for(i = 0; i < ndc; i++) af *= 10;

	    af = floor(af + .5);
	    if(f < 0.) af = -af;   /* add a minus sign if required */
	    snprintf(t, 30, "%f", af);   /* print af to t */
	    /* find the decimal point */
	    if((p = strchr(t, '.')) != NULL)
	    {
		*p = '\0';
	    }
	    n = strlen(t);

	    /* add a decimal point before the last ndc digits */
	    if(ndc > 0)
	    {
		if(t[0] != '-' && ndc > n)
		{
		    for(i = 1; i <= n; i++) t[ndc-i] = t[n-i];
		    for(i = 0; i < ndc-n; i++) t[i] = '0';
		    n = ndc;
		}
		else if(t[0] == '-' && ndc >= n)
		{
		    for(i = 0; i < n-1; i++) t[ndc-i] = t[n-i-1];
		    for(i = 1; i <= ndc-n+1; i++) t[i] = '0';
		    n = ndc + 1;
		}
		for(i = 0; i < ndc && i < n; i++) t[n-i] = t[n-i-1];
		t[n-ndc] = '.';
		n = n + 1;
		if(!fixLength) {
		    /* remove trailing 0's after the decimal point */
		    while(t[n-1] == '0') n--;
		}
	    }
	    if(!fixLength) {
		/* if the last character is a decimal, remove it. */
		if(t[n-1] == '.') n--;
	    }
	}
	if(n > 0)
	{
	    for(i = 0; i < n && i < len; i++) s[i] = t[i];
	    s[n] = '\0';
	}
	else if(len >= 2)
	{
	    s[0] = '0';
	    s[1] = '\0';
	}
	return;
}
