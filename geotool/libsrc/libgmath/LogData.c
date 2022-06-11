/*
 * NAME
 *      LogData: compute log
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <math.h>

#include "libgmath.h"

/**
 * Convert data to log10. Convert the input float array y[] to log10(y[]). All
 * elements y[i] that are less than or equal to zero are set to the log of
 * the minimum y[i] that is greater than 0.
 */
void
LogData(int n, float *y)
{
	int i;
	double min = 1.;

	if(n <= 0)
	{
		return;
	}

	for(i = 0; i < n; i++) if(y[i] > 0.)
	{
		min = y[i];
	}
	for(i = 0; i < n; i++) if(y[i] > 0.)
	{
		if(min > y[i]) min = y[i];
	}
	min = log10(min);

	for(i = 0; i < n; i++)
	{
		if(y[i] > 0.)
		{
			y[i] = log10((double)y[i]);
		}
		else
		{
			y[i] = min;
		}
	}
}
