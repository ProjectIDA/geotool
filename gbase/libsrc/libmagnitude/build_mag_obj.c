/*
 * Copyright (c) 1997-2000 Science Applications International Corporation.
 *

 * NAME
 *	build_mag_obj -- Build magnitude object(s)

 * FILE 
 *	build_mag_obj.c

 * SYNOPSIS
 *	MAGNITUDE *
 *	build_mag_obj (list_of_magtypes, num_magtypes, origin, in_netmag, 
 *			num_netmags, in_stamag, num_stamags, det_amplitude,
 *			num_det_amps, ev_amplitude, num_ev_amps, in_assoc,
 *			num_assocs, in_parrival, num_parrivals)
 *	char		**list_of_magtypes;	(i) List of desired magtypes.
 *	int		num_magtypes;		(i) Number magtypes desired
 *	Origin		*origin;		(i) Singular origin table
 *	Netmag		*in_netmag;		(i) Available netmag records
 *	int		num_netmags;		(i) Number of available input 
 *						    in_netmag records
 *	Stamag		*in_stamag;		(i) Available stamag records
 *	int		num_stamags;		(i) Number of available input 
 *						    in_stamag records
 *	Amplitude	*det_amplitude;		(i) Detection-based amplitude 
 *						    records
 *	int		num_det_amps;		(i) Number of available input 
 *						    detection-based amplitude 
 *						    records
 *	Amplitude	*ev_amplitude;		(i) Event-based amplitude 
 *						    records
 *	int		num_ev_amps;		(i) Number of available input 
 *						    detection-based amplitude 
 *						    records
 *	Assoc		*in_assoc;		(i) Available assoc records
 *	int		num_assocs;		(i) Number of in_assoc records
 *	Parrival	*in_parrival;		(i) Available parrival records
 *	int		num_parrivals;		(i) Number of in_parrival records

 * DESCRIPTIONS
 *	Function.  Initialize (build) magnitude object (of type, MAGNITUDE,
 *	as described in the include file, mag_descrip.h).  Takes as input a
 *	list of desired magtypes and all netmag, stamag and amplitude 
 *	records associated with a given event (origin).  This function will
 *	build a magnitude object (of length num_magtypes) where a NULL 
 *	record will be constructed if no associated information can be
 *	group with the requested magtype.  Input amplitude information is
 *	disseminated into detection-based (det_amplitude) and event-based
 *	(ev_amplitude) structures since this is the logical way with which
 *	to extract these records from the database.  

 *	If available, pre-existing stamag and netmag records will be 
 *	exploited.  When no pre-existing records can be found, new stamag, 
 *	and if needed, netmag records will be constructed.  If no previous 
 *	netmag record existed, then a new magid will be created as well.
 *	Whenever new stamag records are required, we ensure the given phase 
 *	is contained within the list of phases specified within the TLSF for 
 *	the given TLtype.  (The TLtype is linked to magtype within the 
 *	mag_descrip structure).  We will also verify that a valid magnitude 
 *	correction exists at the given distance and depth ranges before 
 *	producing a new stamag record.

 *	This function needs to be called before network magnitude calculations
 *	can be undertaken.

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	Perhaps replace function call to valid_phase_for_TLtype() with a
 *	single call to something like get_list_of_phases (TLtype) which would
 *	return a pointer to the list of acceptable phases.  Then, one would
 *	simply loop over this list w/o other overhead required by function,
 *	valid_phase_for_TLtype().  Perhaps, this function call is not even
 *	needed with introduction of new function, valid_range_for_TLtable().

 *	stamag records that are newly produced within this function will be
 *	identified by filling their auth attribute field with the character
 *	string, "build_mag_obj".

 * SEE ALSO
 *	Include file, mag_descrip.h, for description of MAGNITUDE structure.

 * AUTHORS
 *	Walter Nagy,  9/ 9/97	Created file.
 *	Walter Nagy,  1/ 7/98	Added function, valid_range_for_TLtable(),
 *				so as not to produce new stamag records where
 *				the distance/depth range will not permit
 *				a station magnitude to be computed.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netmag_Astructs.h"
#include "stamag_Astructs.h"
#include "libmagnitude.h"


static	int	magid = -1;


MAGNITUDE *
#ifdef __STDC__
build_mag_obj (char **list_of_magtypes, int num_magtypes, Origin *origin,
		Netmag *in_netmag, int num_netmags, Stamag *in_stamag, 
		int num_stamags, Amplitude *det_amplitude, int num_det_amps,
		Amplitude *ev_amplitude, int num_ev_amps, Assoc *in_assoc,
		int num_assocs, Parrival *in_parrival, int num_parrivals)
#else
build_mag_obj (list_of_magtypes, num_magtypes, origin, in_netmag, num_netmags, 
		in_stamag, num_stamags, det_amplitude, num_det_amps, 
		ev_amplitude, num_ev_amps, in_assoc, num_assoc, in_parrival,
		num_parrivals)
char		**list_of_magtypes;
int		num_magtypes;
Origin		*origin;
Netmag		*in_netmag;
int		num_netmags;
Stamag		*in_stamag;
int		num_stamags;
Amplitude	*det_amplitude;
int		num_det_amps;
Amplitude	*ev_amplitude;
int		num_ev_amps;
Assoc		*in_assoc;
int		num_assocs;
Parrival	*in_parrival;
int		num_parrivals;
#endif
{
	Bool		stamag_found = FALSE;
	int		i, j, k, n;
	int		indx;
	int		num_cnt = 0;
	int		num_det_based = 0;
	int		pre_cnt = 0;
	int		num_existing_stamags = 0;
	double		delta;
	char		TLtype[9];
	MAGNITUDE	*out_magnitude = (MAGNITUDE *) NULL;
	MAGNITUDE	*magn = (MAGNITUDE *) NULL;
	Stamag		*stamag = (Stamag *) NULL;
	Amplitude	*amplitude = (Amplitude *) NULL;
        Netmag          Na_Netmag_rec = Na_Netmag_Init;
        Stamag          Na_Stamag_rec = Na_Stamag_Init;


	if ((out_magnitude = UALLOC (MAGNITUDE, num_magtypes)) == NULL)
	    return (MAGNITUDE *) NULL;

	for (i = 0; i < num_magtypes; i++)
	{
	    magn = &out_magnitude[i];
	    magn->mag_computed = FALSE;
	    magn->mag_write = FALSE;
	    magn->netmag = Na_Netmag_rec;
	    magn->stamag = (Stamag **) NULL;
	    magn->amplitude = (Amplitude **) NULL;
	    magn->sm_aux = (SM_Aux *) NULL;
	    magn->count = 0;

	    /*
	     * Set mag_cntrl structure for given magtype, if included in MDF.
	     */

	    if (get_magtype_features (list_of_magtypes[i], 
				      &magn->mag_cntrl) != TRUE)
	    {
		fprintf (stderr, "Magtype: %s is not specified within MDF\n",
				 list_of_magtypes[i]);
		fprintf (stderr, "Hence, this magnitude cannot be computed!\n");
		continue;
	    }
	    strcpy (TLtype, magn->mag_cntrl.TLtype);

	    /*
	     * Starting from det_amplitude, link existing stamags by evid,
	     * ampid and magtype (Should already be grouped by evid?).  
	     * Ensure magids are all the same for a given grouping.  Where 
	     * no existing stamag is found, create a NULL stamag record with 
	     * evid, magid, ampid and magtype populated (also set magdef to 
	     * defining).

 	     * Whenever new stamag records are required, we ensure the given 
	     * phase is contained within the list of phases specified within 
	     * the TLSF for the given TLtype (by invoking function, 
	     * valid_phase_for_TLtype()).  We will also verify that a valid 
	     * magnitude correction exists at the given distance and depth 
	     * ranges before producing a new stamag record (by invoking 
	     * function, valid_range_for_TLtable()).

	     * For ev_amplitude, first make sure algo_code is not equal to
	     * 0.  If algo_code = 0, then no origin-based amplitude measures
	     * are needed for this magtype.  Else, group existing stamag
	     * records based on ampid and magtype.  Where no stamag record 
	     * is found, create NULL stamag record with magid, ampid and 
	     * magtype populated (also set magdef to defining).
	     */

	    pre_cnt = num_det_amps;
	    if (magn->mag_cntrl.algo_code != NET_AVG)
		pre_cnt += num_ev_amps;
	    stamag = UALLOC (Stamag, pre_cnt);
	    amplitude = UALLOC (Amplitude, pre_cnt);

	    num_cnt = 0;
	    num_existing_stamags = 0;
	    for (k = 0; k < num_det_amps; k++)
	    {
		stamag_found = FALSE;
		for (j = 0; j < num_stamags; j++)
		{
		    if (det_amplitude[k].ampid == in_stamag[j].ampid &&
			STREQ (in_stamag[j].magtype, list_of_magtypes[i]) &&
			STREQ (magn->mag_cntrl.det_amptype, 
			       det_amplitude[k].amptype))
		    {
			stamag_found = TRUE;
			memcpy (&stamag[num_cnt], &in_stamag[j],
				(size_t) (sizeof (Stamag)));
			memcpy (&amplitude[num_cnt], &det_amplitude[k],
				(size_t) (sizeof (Amplitude)));
			++num_cnt;
			++num_existing_stamags;
			break;
		    }
		}

		if (!stamag_found && STREQ (magn->mag_cntrl.det_amptype,
					    det_amplitude[k].amptype))
		{
		    /*
		     * Group in_assoc record to det_amplitude based on arid.
		     */

		    for (indx = -1, n = 0; n < num_assocs; n++)
			    if (in_assoc[n].arid == det_amplitude[k].arid)
				indx = n;

		    if (indx > -1)
		    {
			delta = get_delta_for_sta (in_assoc[indx].sta,
						   origin->lat, origin->lon);

			if (delta > 0.0 &&
			    valid_phase_for_TLtype (TLtype, 
						in_assoc[indx].phase) &&
			    valid_range_for_TLtable (TLtype, 
						in_assoc[indx].sta,
						in_assoc[indx].phase,
						det_amplitude[k].chan,
						in_assoc[indx].delta, 
						origin->depth))
			{
			    stamag[num_cnt] = Na_Stamag_rec;
			    stamag[num_cnt].ampid = det_amplitude[k].ampid;
			    stamag[num_cnt].arid = det_amplitude[k].arid;
			    stamag[num_cnt].orid = origin->orid;
			    stamag[num_cnt].evid = origin->evid;
			    stamag[num_cnt].delta = delta;
			    strcpy (stamag[num_cnt].sta, in_assoc[indx].sta);
			    strcpy (stamag[num_cnt].phase,
				    in_assoc[indx].phase);
			    strcpy (stamag[num_cnt].magdef, "d");
			    strcpy (stamag[num_cnt].magtype, 
				    list_of_magtypes[i]);
			    strcpy (stamag[num_cnt].auth, "build_mag_obj");
			    memcpy (&amplitude[num_cnt], &det_amplitude[k],
				    (size_t) (sizeof (Amplitude)));
			    ++num_cnt;
			}
		    }
		}
	    }
	    num_det_based = num_cnt;

	    /*
	     * Now associated event-based amplitude measures assuming that
	     * they can be used for given magnitude determination (i.e.,
	     * not a network-average magnitude).
	     */

	    if (magn->mag_cntrl.algo_code != NET_AVG)
	    {
		for (k = 0; k < num_ev_amps; k++)
		{
		    stamag_found = FALSE;
		    for (j = 0; j < num_stamags; j++)
		    {
			if (ev_amplitude[k].ampid == in_stamag[j].ampid &&
			    STREQ (in_stamag[j].magtype, list_of_magtypes[i]) &&
			    STREQ (magn->mag_cntrl.ev_amptype,
				   ev_amplitude[k].amptype))
			{
			    stamag_found = TRUE;
			    memcpy (&stamag[num_cnt], &in_stamag[j],
				    (size_t) (sizeof (Stamag)));
			    memcpy (&amplitude[num_cnt], &ev_amplitude[k],
				    (size_t) (sizeof (Amplitude)));
			    ++num_cnt;
			    ++num_existing_stamags;
			    break;
			}
		    }

		    if (!stamag_found && STREQ (magn->mag_cntrl.ev_amptype, 
						ev_amplitude[k].amptype))
		    {
			/*
			 * Group in_parrival record to ev_amplitude based 
			 * on parid.
			 */

			for (indx = -1, n = 0; n < num_parrivals; n++)
			    if (in_parrival[n].parid == ev_amplitude[k].parid)
				indx = n;

			if (indx > -1)
			{
			    delta = 
			    get_delta_for_sta (in_parrival[indx].sta,
						origin->lat, origin->lon);

			    if (delta > 0.0 &&
				valid_phase_for_TLtype (TLtype, 
						in_parrival[indx].phase) &&
				valid_range_for_TLtable (TLtype,
						in_parrival[indx].sta,
						in_parrival[indx].phase,
						ev_amplitude[k].chan,
						delta, origin->depth))
			    {
				stamag[num_cnt] = Na_Stamag_rec;
				stamag[num_cnt].ampid = ev_amplitude[k].ampid;
				stamag[num_cnt].arid = ev_amplitude[k].arid;
				stamag[num_cnt].orid = origin->orid;
				stamag[num_cnt].evid = origin->evid;
				stamag[num_cnt].delta = delta;
				strcpy (stamag[num_cnt].sta, 
					in_parrival[indx].sta);
				strcpy (stamag[num_cnt].phase, 
					in_parrival[indx].phase);
				strcpy (stamag[num_cnt].magdef, "d");
				strcpy (stamag[num_cnt].magtype, 
					list_of_magtypes[i]);
				strcpy (stamag[num_cnt].auth, "build_mag_obj");
				memcpy (&amplitude[num_cnt], &ev_amplitude[k],
				    	(size_t) (sizeof (Amplitude)));
				++num_cnt;
			    }
			}
		    }
		}
	    }

	    /*
	     * No amplitude information exists for this magtype.  Therefore,
	     * simply set current out_magnitude record as a NULL record.
	     */

	    if (num_cnt == 0)
	    {
		magn->count = 0;
		UFREE (stamag);
		UFREE (amplitude);
		continue;
	    }

	    /*
	     * Get pre-existing netmag record, assuming at least one pre-
	     * existing stamag record was found.  If no pre-existing netmag
	     * record is found, then set a new magid and populate as many
	     * attributes of netmag as possible, up front.
	     */

	    if (num_existing_stamags > 0)
	    {
		for (k = 0; k < num_netmags; k++)
		{
		    if (STREQ (in_netmag[k].magtype, list_of_magtypes[i]))
		    {
			memcpy (&magn->netmag, &in_netmag[k],
				(size_t) (sizeof (Netmag)));
			for (j = 0; j < num_cnt; j++)
			    stamag[j].magid = in_netmag[k].magid;
		    }
		}
	    }
	    else
	    {
		--magid;	/* Set new magid */
		magn->netmag = Na_Netmag_rec;
		magn->netmag.evid = origin->evid;
		magn->netmag.orid = origin->orid;
		magn->netmag.magid = magid;
		strcpy (magn->netmag.magtype, list_of_magtypes[i]);
		for (j = 0; j < num_cnt; j++)
		    stamag[j].magid = magid;
	    }

	    /*
	     * Now we have all the information we need to allocate stamag,
	     * amplitude, sm_aux and count fields of out_magnitude 
	     * structure for this magtype.
	     */

	    magn->count = num_cnt;
	    magn->stamag = UALLOC (Stamag *, num_cnt);
	    magn->amplitude = UALLOC (Amplitude *, num_cnt);
	    magn->sm_aux = UALLOC (SM_Aux, num_cnt);

	    /*
	     * Populate sm_aux structure to exploit station magnitude-
	     * based information settings. 
	     */

	    for (j = 0; j < num_cnt; j++)
	    {
		magn->stamag[j] = UALLOC (Stamag, 1);
		magn->amplitude[j] = UALLOC (Amplitude, 1);
		memcpy (magn->stamag[j], &stamag[j],
			(size_t) (sizeof (Stamag)));
		memcpy (magn->amplitude[j], &amplitude[j],
			(size_t) (sizeof (Amplitude)));

		magn->sm_aux[j].manual_override = FALSE;

		if (j < num_det_based)
		    magn->sm_aux[j].detect_based = TRUE;
		else
		    magn->sm_aux[j].detect_based = FALSE;

		if (STREQ (amplitude[j].clip, "c"))
		    magn->sm_aux[j].clipped = TRUE;
		else
		    magn->sm_aux[j].clipped = FALSE;

		if (! magn->sm_aux[j].detect_based)
		    magn->sm_aux[j].sig_type = NON_DETECT;
		else if (! magn->sm_aux[j].clipped)
		    magn->sm_aux[j].sig_type = MEAS_SIGNAL;
		else
		    magn->sm_aux[j].sig_type = CLIPPED;
	    }

	    UFREE (stamag);
	    UFREE (amplitude);

	}   /* End of magtype loop */

	return (out_magnitude);
}


