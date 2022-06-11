
/*
 * Copyright (c) 1994-1997 Science Applications International Corporation.
 *

 * NAME
 *	copy_magnitudes -- Copy given array of "MAGNITUDE" structures.
 *	free_magnitudes -- Free memory devoted to "MAGNITUDE" structures.

 * FILE 
 *	mag_utils.c

 * SYNOPSIS
 *	MAGNITUDE *
 *	copy_magnitudes (in_magnitude, num_mag_records)
 *	MAGNITUDE *in_magnitude;	(i) MAGNITUDE structure.
 *	int	  num_mag_records;	(i) Number of magnitude records.

 *	void
 *	free_magnitudes (in_magnitude)
 *	MAGNITUDE *in_magnitude;	(i) Pointer to MAGNITUDE.

 * DESCRIPTIONS
 * 	copy_magntiudes() copies the given set of MAGNITUDE records to
 *	newly allocated memory.

 *	free_magnitudes() frees memory previously allocated.

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 * 	copy_magnitudes() has a second arguement which tells how many 
 *	records in the MAGNITUDE structure should be copied.

 * SEE ALSO
 *	Include file, libmagnitude.h, for description of MAGNITUDE structure.

 * AUTHORS
 *	Walter Nagy, 10/ 7/97,	Created.
 */


#include <stdlib.h>
#include <string.h>
#include "libmagnitude.h"


MAGNITUDE *
#ifdef __STDC__
copy_magnitudes (MAGNITUDE *in_magnitude, int num_mag_records)
#else
copy_magnitudes (in_magnitude, num_mag_records)
MAGNITUDE	*in_magnitude;
int		num_mag_records;
#endif
{
	int		i, j;
	int		cnt;
	MAGNITUDE	 *out_magnitude = (MAGNITUDE *) NULL;
	MAGNITUDE	 *magn = (MAGNITUDE *) NULL;

	if (in_magnitude == (MAGNITUDE *) NULL)
	    return (MAGNITUDE *) NULL;

	if (num_mag_records > 0)
	{
	    if ((out_magnitude = UALLOC (MAGNITUDE, num_mag_records)) == NULL)
	    	return (MAGNITUDE *)NULL;

	    /*
	     * Copy the netmag records and allocate new stamag records.
	     */

	    for (i = 0; i < num_mag_records; i++)
	    {
		magn = &out_magnitude[i];
		magn->stamag = (Stamag **) NULL;
		magn->amplitude = (Amplitude **) NULL;
		magn->sm_aux = (SM_Aux *) NULL;
		if (in_magnitude[i].count == 0)
		{
		    magn->count = 0;
		    continue;
		}

		magn->mag_computed = in_magnitude[i].mag_computed;
		cnt = in_magnitude[i].count;
		magn->netmag = in_magnitude[i].netmag;
		magn->mag_cntrl = in_magnitude[i].mag_cntrl;
		if (cnt > 0)
		{
		    magn->stamag = UALLOC (Stamag *, cnt);
		    magn->amplitude = UALLOC (Amplitude *, cnt);
		    magn->sm_aux = UALLOC (SM_Aux, cnt);
		}
		magn->count = cnt;
		memcpy (magn->sm_aux, in_magnitude[i].sm_aux,
			(size_t) (cnt*(sizeof (SM_Aux))));

		for (j = 0; j < cnt; j++)
		{
		    magn->stamag[j] = UALLOC (Stamag, 1);
		    magn->amplitude[j] = UALLOC (Amplitude, 1);
		    memcpy (magn->stamag[j], in_magnitude[i].stamag[j],
			    (size_t) (sizeof (Stamag)));
		    memcpy (magn->amplitude[j], in_magnitude[i].amplitude[j],
			    (size_t) (sizeof (Amplitude)));
		}
	    }
	}

	return out_magnitude;
}


void
#ifdef __STDC__
free_magnitudes (MAGNITUDE *magnitude, int num_mag_records)
#else
free_magnitudes (magnitude, num_mag_records)
MAGNITUDE	*magnitude;
int		num_mag_records;
#endif
{
	int	  i, j;

	if (magnitude != (MAGNITUDE *) NULL)
	{
	    for (i = 0; i < num_mag_records; i++)
	    {
		for (j = 0; j < magnitude[i].count; j++)
		{
			if (magnitude[i].stamag != (Stamag **) NULL)
				UFREE (magnitude[i].stamag[j]);

			if (magnitude[i].amplitude != (Amplitude **) NULL)
				UFREE (magnitude[i].amplitude[j]);
		}

		UFREE (magnitude[i].stamag);
		UFREE (magnitude[i].amplitude);
		UFREE (magnitude[i].sm_aux);
	    }
	    UFREE (magnitude);
	}
}


