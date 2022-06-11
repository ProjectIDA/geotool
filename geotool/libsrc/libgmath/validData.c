#include "config.h"
#include <stdlib.h> 
#include <math.h> 
#include <ctype.h>  
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */


#include "libgmath.h"

/**
 * Check a float data array for finite numbers. Replace all elements of the
 * input floating point array that are not valid finite numbers with the
 * input value.
 * @param nsamp The length of the data array.
 * @param data An array of data points to check.
 * @param value A number to substitude for invalid (!finite) data elements.
 * @return The number of data points that are invalid.
 */
int
invalidData(int nsamp, float *data, double value)
{
	int invalid;
	int i;

	invalid = 0;
	for(i = 0; i < nsamp; i++, data++) {
	    if(!finite(*data)) {
		invalid++;
		*data = (float)value;
	    }
	}
	return invalid;
}
