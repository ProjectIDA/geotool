#include "config.h"

#include "libgmath.h"

#ifndef HAVE_NINT

/**
 * Round a double to the nearest integer. This is the UNIX system function 
 * nint for systems that do not have it.
 */
int nint(double f)
{
	if(f > 0.)
	{
		return((int)(f + .5));
	}
	else
	{
		return((int)(f - .5));
	}
}

#endif /* HAVE_NINT */
