
/*
 * Copyright (c) 1997-1998 Science Applications International Corporation.
 *

 * NAME
 *	read_tlsf -- Read distance/depth transmission loss (TL) tables
 *	interp_for_tl_value -- Do bi-cubic spline interpolation for TL value
 *	get_tl_model_error -- Get TL modelling error for given TLtype
 *	valid_phase_for_TLtype -- Is phase type acceptable for TLtype?
 *	valid_range_for_TLtable -- Is dist/depth range acceptable for TLtable?
 *	get_TL_indexes -- Get TLtype indexes
 *	get_TLMD_indexes -- Get index to tltype_model_descrip given TLtype
 *	set_sta_TL_pt -- Set station pointers for rapid search of TLtype
 *	get_TL_ts_corr -- Get TL test-site correction
 *	free_tl_table -- Free memory associated with individual TL tables
 *	get_delta_for_sta -- Get distance (arc deg) to event for input station
 
 * FILE
 *	TL_manipulation.c

 * SYNOPSIS
 *	int
 *	read_tlsf (tl_model_filename, list_of_TLtypes, sites, num_sites)
 *	char	*tl_model_filename;	(i) TL model file name 
 *	TL_Pt	*list_of_TLtypes;	(i) List of unique TLtypes for which 
 *					    to extract TLSF information
 *	Site	*sites;			(i) Site structure (station info)
 *	int	num_sites;		(i) Number of site table entries

 *	double
 *	interp_for_tl_value (distance, depth, tl_index, extrapolate, tl_deriv,
 *			     interp_code)
 *	double	distance;		(i) Station to event distance (arc deg)
 *	double	depth;			(i) Event depth (km)
 *	int	tl_index;		(i) TL index
 *	Bool	extrapolate;		(i) Permit extrapolation ? [T/F]
 *	double	*tl_deriv;		(o) Prin. radial & depth TL derivatives
 *	int	*interp_code;		(o) Interpolation error code (0 = OK)

 *	double
 *	get_tl_model_error (tl_index, delta, depth, model)
 *	int	tl_index;		(i) Attenuation type (TLtype) index 
 *	double	delta;			(i) Event-to-station distance (deg.)
 *	double	depth;			(i) Depth below Earth's surface (km.)
 *	char	*model			(o) TL model associated with M.E.

 *	Bool
 *	valid_phase_for_TLtype (TLtype, phase)
 *	char	*TLtype;		(i) TL type
 *	char	*phase;			(i) Input phase name

 *	Bool
 *	valid_range_for_TLtable (TLtype, sta, phase, chan, delta, depth)
 *	char	*TLtype;		(i) TL type
 *	char	*sta;			(i) Input station name
 *	char	*phase;			(i) Input phase name
 *	char	*chan;			(i) Input chan(nel) name
 *	double	delta;			(i) Event-to-station distance (deg.)
 *	double	depth;			(i) Depth below Earth's surface (km.)

 *	int
 *	get_TL_indexes (magtype, sta, phase, chan, sta_index, stm_index, 
 *			md_index)
 *	char	*magtype;		(i) Magnitude type (e.g., mb, ms)
 *	char	*sta;			(i) Input station name
 *	char	*phase;			(i) Input phase name
 *	char	*chan;			(i) Input chan(nel) name
 *	int	*sta_index;		(o) Station index of site table
 *	int	*stm_index;		(o) STM index for STM structure access
 *	int	*md_index;		(o) MD index for MD structure access

 *	int
 *	get_TLMD_index (TLtype) 
 *	char	*TLtype;		(i) TL type

 *	int
 *	set_sta_TL_pt (sites, num_sites)
 *	Site	*sites;			(i) Site structure (station info)
 *	int	num_sites;		(i) Number of site table entries

 *	Bool
 *	get_TL_ts_corr (ts_region, sta, TLtype, tl_index, ts_corr)
 *	char	*ts_region;		(i) Requested test-site region name
 *	char	*sta;			(i) Station name
 *	char	*TLtype;		(i) TL type
 *	int	tl_index;		(i) TL index
 *	double	*ts_corr;		(o) TL Test-site correction

 *	void
 *	free_tl_table ()

 *	double
 *	get_delta_for_sta (sta, ev_lat, ev_lon)
 *	char	*sta;			(i) Station name
 *	double	ev_lat;			(i) Event latitude (deg)
 *	double	ev_lon;			(i) Event longitude (deg)

 * DESCRIPTION
 *	Functions.  These routines provide the core transmission loss (TL)
 *	handling facilities for all GSO software.  All reading, accessing
 *	and manipulation of TL-specific information is handled here.

 *	-- read_tlsf(), reads distance/depth-depedent transmission loss (TL) 
 *	tables as based on the input transmission loss specification file 
 *	(TLSF; tl_model_filename).  Individual TL tables are read by single 
 *	calls to function, read_tl_tables().  Only specification information
 *	matching the input list_of_TLtypes with stations included in the input
 *	site table will be read into memory.
 *	The first implementation for TL tables is for reading magnitude 
 *	correction tables.  Yet, this function can be applied to other, more
 *	general, kinds of transmission loss information (e.g., raw attenuation
 *	curves).

 *	-- interp_for_tl_value() performs a bi-cubic spline interpolation, 
 *	given a distance/depth-dependent TL table for a specified distance
 *	and depth, extracting the respective TL value at that point.  This
 *	function also returns the principal first and second radial and depth
 *	deviatives.

 *	-- get_tl_model_error(), as the name implies, is used to extract the
 *	modelling error, which is stored with the TL tables.  Either a
 *	single-valued, distance-dependent, or distance/depth-depth modelling 
 *	error may exist.  These are optimized to expedite the access to 
 *	these errors.  If the input distance and/or depth is outside the 
 *	specified range and domain, then the modelling error returned comes 
 *	from the nearest edge.

 *	-- valid_phase_for_TLtype() determines whether or not the input phase
 *	type is valid for the given TLtype.  Phase type must belong to one
 *	of the phase types specified in the list_of_phz structure tied to
 *	the input TLtype as stored in the structure, tltype_model_descrip.

 *	-- valid_range_for_TLtable() determines whether or not the input
 *	distance (delta) and depth is within a valid range as specified 
 *	within the TL table for the given TLtype/sta/phase/chan. 

 *	-- get_TL_indexes(), as the name implies, is used to determine
 *	the magnitude-related attenuation indexes for a given magtype, and,
 *	if available, station/TLtype/model (STM) info.

 *	-- get_TLMD_index() simply determines the index to the local
 *	tltype_model_descrip structure given a TLtype.

 *	-- set_sta_TL_pt(), links the station pointer structure (type, 
 *	Sta_Pt) with a list of TL types (TLtype), corresponding to a given
 *	station, for rapid access.

 *	-- get_TL_ts_corr() will get the TL test-site correction for a given
 *	input station if it exists. 

 *	-- free_tl_table(), frees all previously allocated memory associated
 *	with the individual 1-D TL tables.  This is done here since all 
 *	handling of these tables is done local to this file and the lower-
 *	level file, read_tl_table.c.

 *	-- get_delta_for_sta() computes the event-to-station distance (arc
 *	deg) given the input station name and event latitude and longitude.

 * DIAGNOSTICS
 *	See mag_error_table[] in file, mag_error_msg.c, for specific global
 *	magnitude error code descriptions.

 *	-- read_tlsf () will return the following error codes that can be
 *	encountered within this routine (see include file, tl_defs.h, for 
 *	error code define statements):
 *	    TLreadErr1:	Cannot open TLSF.
 *	    TLreadErr2:	TLSF incorrectly formatted.
 *	    TLreadErr3:	No transmission loss (TL) tables could be found.
 *	    TLreadErr4:	TL table incorrectly formatted.
 *	    TLreadErr5:	TL modelling error table incorrectly formatted.
 *	    TLreadErr6:	TL test-site corr. file incorrectly formatted.
 *	    TLreadErr7:	Error allocating memory while reading TL info.

 *	-- interp_for_tl_value() will return -999.0 if a serious error was
 *	encountered during the attempted interpolation procedure.  Else, the
 *	interpolated TL value, for the given input station-to-event distance 
 *	and event depth, is returned.

 *	-- get_tl_model_error(), returns the TL modelling error for the given
 *	input TLtype.  If the TLtype does not have an associated modelling 
 *	error, then a NA value is returned, which indicates that the baseline
 *	standard deviation in the MDF for the current magtype should be used.

 *	-- valid_phase_for_TLtype(), returns a condition of TRUE (1) if input
 *	phase type maps to the list of phases attached to the input TLtype 
 *	definition as provided for in the tltype_model_descrip structure.
 *	Else, it returns a condition of FALSE (0)

 *	-- valid_range_for_TLtable(), returns a condition of TRUE (1) if input
 *	distance (delta) and depth has a range specified within its TL table 
 *	Else, it returns a condition of FALSE (0)

 *	-- get_TL_indexes() returns the specific integer index to the array 
 *	of TL table pointer specified in tl_table_ptr[].  If no index is
 *	found, then a -1 will be returned.  Will also return a code of -1 
 *	if the requested TLtype is not specified in the TLSF.  Either case
 *	signifies that no valid TL table exist for the given input arguments.

 *	-- get_TLMD_index() returns the index to the local tltype_model_descrip 
 *	structure given a TLtype.  A return code of -1 indicates that no
 *	TLMD index was found matching requested input TLtype.

 *	-- set_sta_TL_pt() will return an error code of SSgetErr2 if memory
 *	cannot be allocated.  It will return an error code of SSgetErr1 if 
 *	no input site table exists OR the site table is empty.

 *	-- get_TL_ts_corr() will return a condition of TRUE, if the requested
 *	test-site correction is found.  Else, a condition of FALSE is returned.

 *	-- get_delta_for_sta() will return the event-to-station distance (arc
 *	deg) if successful.  If input station is not found in static site
 *	table, then -1.0 will be returned.

 * FILES
 *	Read TLSF and individual TL tables along with their associated 
 *	modelling error and test-site correction tables.  These latter two
 *	tables are only optionally defined.

 * NOTES
 *	If input file will not open, then the structure, tl_table, will be 
 *	set to NULL.  All access to individual 1-D TL tables is localized to
 * 	this file, negating the need for it to be returned to any higher-level
 *	functions.
 
 * SEE ALSO
 *	None. 

 * AUTHOR
 *	Walter Nagy, 8/21/97,	Created as read_tlsf.c.
 *	Walter Nagy, 9/29/97,	Changed file name to TL_manipulation.c.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libinterp.h"
#include "libgeog.h"
#include "libmagnitude.h"
#include "magp.h"
#include "tl_defs.h"


/* Error report */
#define READ_ERR(a)	{ fprintf (stderr, "\nread_tlsf: Error reading %s in file: %s\n", (a), file_name); \
			}
#define CALLOC_ERR(a)	{ fprintf (stderr, "\nread_tlsf: Error allocating space for %s, in file: %s\n", (a), file_name); \
			}

#define	VALID_TL(x)	((x) > -1.0)

static	TL_Table	**tl_table_ptr = (TL_Table **) NULL;
static	int	 	num_TL_tables = 0;
static	TL_Model_Path	*tl_model_path = (TL_Model_Path *) NULL;
static	int		num_TL_models = 0;
static	TLType_Model_Descrip	
			*tltype_model_descrip = (TLType_Model_Descrip *) NULL;
static	int		num_TLMD = 0;
static	Sta_TL_Model	*sta_tl_model = (Sta_TL_Model *) NULL;
static	int		num_STM = 0;

static	Sta_Pt		*sta_pt = (Sta_Pt *) NULL;
static	Site		*site = (Site *) NULL;
static	int		num_site = 0;


int
#ifdef __STDC___
read_tlsf (char *tl_model_filename, TL_Pt *list_of_TLtypes, Site *sites,
	   int num_sites)
#else
read_tlsf (char *tl_model_filename, TL_Pt *list_of_TLtypes, Site *sites, 
int num_sites) 

     /*char	*tl_model_filename;*/	/* TL model file name */
     /*TL_Pt	*list_of_TLtypes;*/	/* Linked list of unique TLtypes */ 
     /*Site	*sites;*/			/* Site (station) table */
     /*int	num_sites;*/		/* Number of site (station) records */
#endif

{
	FILE	*tlsf_fp;
	Bool	TLtype_found = FALSE;
	Bool	tl_model_found = FALSE;
	Bool	tltype_found = FALSE;
	Bool	TLMD_found = FALSE;
	Bool	STM_duplicate_found = FALSE;
	int	i, j, k;	
	int	iret = OK;
	int	tl_index = 0;
	int	phase_dependency;
	char	*ffp, *a_ptr;
	char	sta[7], phase[9], chan[9];
	char	TLtype[9], tl_model[16];
	char	file_name[FILENAMELEN];
	char	file_buf[FILENAMELEN];
	char	dir_ptr[FILENAMELEN];
	char	dir_pathway[FILENAMELEN];
	char	relative_pathway[FILENAMELEN];
	char	input_string[BUFSIZ];
	char	phase_list[BUFLENGTH];

	TL_Table	*tl_table = (TL_Table *) NULL;
	List_of_Phz	*phz = (List_of_Phz *) NULL;
	List_of_Phz	*prev = (List_of_Phz *) NULL;
	TL_Pt		*tl = (TL_Pt *) NULL;

	static	char	routine[] = "read_tlsf";


	if (tl_table_ptr != (TL_Table **) NULL)
	{
	    free_tl_table ();
	    num_TL_tables = 0;
	}
	if (tl_model_path != (TL_Model_Path *) NULL)
	{
	    UFREE (tl_model_path);
	    tl_model_path = (TL_Model_Path *) NULL;
	    num_TL_models = 0;
	}
	if (tltype_model_descrip != (TLType_Model_Descrip *) NULL)
	{
	    UFREE (tltype_model_descrip);
	    tltype_model_descrip = (TLType_Model_Descrip *) NULL;
	    num_TLMD = 0;
	}
	if (sta_tl_model != (Sta_TL_Model *) NULL)
	{
	    UFREE (sta_tl_model);
	    sta_tl_model = (Sta_TL_Model *) NULL;
	    num_STM = 0;
	}

	/*
	 * Open TLSF once here to determine how much space needs to be 
	 * allocated up front (namely, number of attenuation (num_TL_models),
	 * number of magtype/TLtype relationships and number of 
	 * station/atten/model-dependent entries (num_STM) 
	 * that exist.  Then close file, re-open, and do actual filling of 
	 * structures.  A blank line should follow the magnitude model name 
	 * and relative directory pathway information as well as the magtype/
	 * TLtype relationships.  A blank space or '#' in the first 
	 * column can be used anywhere within this file for comments.
	 */

	if ((tlsf_fp = fopen (tl_model_filename, "r")) == NULL)
	{
	    fprintf (stderr, "\n%s: TLSF: %s will not open!\n", 
			routine, tl_model_filename);
	    return (TLreadErr1);
	}

	while (fgets (input_string, BUFSIZ, tlsf_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		++num_TL_models;	/* Count number of TL models */
	    }
	}

	while (fgets (input_string, BUFSIZ, tlsf_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;

		/*
		 * Count only number of TLtype/tl_mode relationships (num_TLMD)
		 * belonging to input list_of_TLtypes argument.
		 */

		sscanf (input_string, "%s", TLtype);
		tl = list_of_TLtypes;
		while (tl != (TL_Pt *) NULL)
		{
		    if (STREQ (TLtype, tl->TLtype))
		    {
			++num_TLMD;
			break;
		    }
		    tl = tl->next;
		}
	    }
	}

	while (fgets (input_string, BUFSIZ, tlsf_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (!strncmp (input_string, "#", 1) ||
		!strncmp (input_string, "\n", 1)) 
		continue;
	    else
	    {
		/*
		 * Count only number of STM relationships (num_STM) only where
		 * TLtype belongs to input list_of_TLtypes argument.  Also,
		 * make sure given station is included in site table.  If not,
		 * simply skip.
		 */

		sscanf (input_string, "%s%s", sta, TLtype);
		tl = list_of_TLtypes;
		while (tl != (TL_Pt *) NULL)
		{
		    if (STREQ (TLtype, tl->TLtype))
		    {
			for (i = 0; i < num_sites; i++)
			{
			    if (STREQ (sta, sites[i].sta))
			    {
				++num_STM;
				break;
			    }
			}
			break;
		    }
		    tl = tl->next;
		}
	    }
	}
	fclose (tlsf_fp);

	/*
	 * Parse out directory prefix part of TLSF filename to be passed
	 * to read_tl_table()>
	 */

	strcpy (dir_pathway, tl_model_filename);
	if ((ffp = strrchr (dir_pathway, '/')) == NULL)
	{
	    fprintf (stderr, "%s: No / in TL model filename!\n", routine);
	    return (TLreadErr2);
	}

	/*
	 * Re-open TLSF!  Now we desire to do actual reading of magnitude 
	 * model information including all station/TL/model-dependent 
	 * knowledge.  This latter knowledge take the form of pointer to 
	 * station/TL 1-D table location.
	 */

	if ((tlsf_fp = fopen (tl_model_filename, "r")) == NULL)
	{
	    fprintf (stderr, "\n%s: TLSF: %s cannot be re-opened!\n", 
			routine, tl_model_filename);
	    return (TLreadErr1);
	}

	/*
	 * Allocate storage space for tl_model_path, tltype_model_descrip and 
	 * sta_tl_model structures.
	 */

	if ((tl_model_path = (TL_Model_Path *) 
		calloc (num_TL_models, sizeof (TL_Model_Path))) == NULL)
	{
	    fclose (tlsf_fp);
	    CALLOC_ERR ("structure, tl_model_path");
	    return (TLreadErr7);
	}
	if ((tltype_model_descrip = (TLType_Model_Descrip *) 
		calloc (num_TLMD, sizeof (TLType_Model_Descrip))) == NULL)
	{
	    fclose (tlsf_fp);
	    CALLOC_ERR ("structure, tltype_model_descrip");
	    return (TLreadErr7);
	}
	if (num_STM > 0)
	{
	    if ((sta_tl_model = (Sta_TL_Model *) 
	       calloc (num_STM, sizeof (Sta_TL_Model))) == NULL)
	    {
		fclose (tlsf_fp);
		CALLOC_ERR ("structure, sta_tl_model");
		return (TLreadErr7);
	    }
	}
	else
	    sta_tl_model = (Sta_TL_Model *) NULL;

	/*
	 * Read TL definitions first.  The list of TL types (TLtype) will 
	 * eventually be attached to this model.  NOTE: The last fgets() 
	 * call here will have read the first line after all the TL models 
	 * have been specified. 
	 */

	i = 0;
	while (fgets (input_string, BUFSIZ, tlsf_fp) != NULL)
	{
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		sscanf (input_string, "%s%s", tl_model_path[i].model,
				  relative_pathway);
		strcpy (ffp+1, "");
		strcpy (file_buf, dir_pathway);
		strcpy (dir_ptr, file_buf);
		strcat (dir_ptr, relative_pathway);
		tl_model_path[i].dir_pathway = STRALLOC (dir_ptr);
		/* printf ("tl_model: %s  pathway: %s\n", tl_model_path[i].model, tl_model_path[i].dir_pathway); */
		++i;
	    }
	}

	/*
	 * Next, define TLtype/tl_model/phase dependency definitions.  If 
	 * given tl_model has not already been defined, complain loudly.
	 * Again, we only need to read into memory TLSF information that
	 * matches a TLtype specified by the input argument, list_of_TLtypes.
	 * NOTE: The last fgets() call here will have read the first line 
	 * after all the TLtype/tl_model pairs have been specified. 
	 */

	j = 0;
	tl_index = 0;
	while (fgets (input_string, BUFSIZ, tlsf_fp) != NULL)
	{
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		sscanf (input_string, "%s%s%d%s",
					TLtype, tl_model, &phase_dependency,
					phase_list);
		tl = list_of_TLtypes;
		TLtype_found = FALSE;
		while (tl != (TL_Pt *) NULL)
		{
		    if (STREQ (TLtype, tl->TLtype))
		    {
			TLtype_found = TRUE;
			break;
		    }
		    tl = tl->next;
		}

		if (!TLtype_found)
		    continue;	/* This TLtype not requested */

		strcpy (tltype_model_descrip[j].TLtype, TLtype);
		strcpy (tltype_model_descrip[j].model, tl_model);
		tltype_model_descrip[j].phase_dependency = phase_dependency;

		/*
		 * Parse out list of applicable phases to a list_of_phz
		 * structure attached to tltype_model_descrip.
		 */

		if ((tltype_model_descrip[j].list_of_phz = (List_of_Phz *)
				calloc (1, sizeof (List_of_Phz))) == NULL)
		{
		    CALLOC_ERR ("structure, tltype_model_descrip[j].list_of_phz");
		    return (TLreadErr7);
		}
		a_ptr = strtok (phase_list, ",");
		phz = tltype_model_descrip[j].list_of_phz;
		strcpy (phz->phase, a_ptr);
		phz->next = (List_of_Phz *) NULL;
		phz->tl_index = -1;
		prev = phz;
		while ((a_ptr = strtok ((char *) NULL, ",")) != NULL)
		{
		    if ((phz = (List_of_Phz *) 
			        calloc (1, sizeof (List_of_Phz))) == NULL)
		    {
			CALLOC_ERR ("structure, tltype_model_descrip[j].list_of_phz");
			return (TLreadErr7);
		    }
		    prev->next = phz;
		    strcpy (phz->phase, a_ptr);
		    phz->next = (List_of_Phz *) NULL;
		    phz->tl_index = -1;
		    prev = phz;
		}

		for (tl_model_found = FALSE, k = 0; k < num_TL_models; k++)
		{
		    if (STREQ (tltype_model_descrip[j].model,
				tl_model_path[k].model))
		    {
			tl_model_found = TRUE;
			tltype_model_descrip[j].model_index = k;
			break;
		    }
		}
		if (! tl_model_found)
		{
		    fclose (tlsf_fp);
		    fprintf (stderr, "%s: TL model not specified for: %s\n for TLtype:%s\n", routine, tltype_model_descrip[j].model, tltype_model_descrip[j].TLtype);
		    return (TLreadErr2);
		}

		/*
		 * If individual phase depedency has been specified for this
		 * TLtype/model pair, then multiple transmission loss files
		 * need to be read on a phase-by-phase basis.  These files
		 * will take the form: dir_pathway/model.TLtype.phase (num_phz
		 * tl_index(es) will need to be set in this case).  If no
		 * phase dependency is specfied, then just a single file of
		 * the form: dir_pathway/model.TLtype need by read and only
		 * 1 tl_index need be added.
		 */

		k = tltype_model_descrip[j].model_index;
		if (tltype_model_descrip[j].phase_dependency)
		{
		    phz = tltype_model_descrip[j].list_of_phz;
		    while (phz != (List_of_Phz *) NULL)
		    {
			/* 
			 * Read a single TL file.  Note no channel dependency.
			 */

			iret =
			read_tl_table (tl_model_path[k].dir_pathway,
				tltype_model_descrip[j].TLtype,
				tltype_model_descrip[j].model,
				phz->phase, "-", &tl_table);
			if (iret != OK)
			{
			    if (iret == TLreadWarn1)
				continue;
			    else
			    {
				fclose (tlsf_fp);
				return (iret);
			    }
			}

			tl_table_ptr = UREALLOC (tl_table_ptr, TL_Table *,
						 tl_index + 1);
			tl_table_ptr[tl_index] = tl_table;

			phz->tl_index = tl_index;
			++tl_index;

			phz = phz->next;
		    }
		}
		else
		{
		    /* 
		     * Read a single TL file.  Note no phase or channel 
		     * dependency.
		     */

		    iret =
		    read_tl_table (tl_model_path[k].dir_pathway,
				   tltype_model_descrip[j].TLtype,
				   tltype_model_descrip[j].model,
				   "-", "-", &tl_table);

		    if (iret != OK && iret != TLreadWarn1)
		    {
			fclose (tlsf_fp);
			return (iret);
		    }

		    if (iret != TLreadWarn1)
		    {
			/*
			 * Since no phase dependency has been specified, 
			 * then tl_index is the same for all phases for 
			 * this TLtype/tl_model pair.
			 */

			phz = tltype_model_descrip[j].list_of_phz;
			while (phz != (List_of_Phz *) NULL)
			{
			    phz->tl_index = tl_index;
			    phz = phz->next;
			}

			tl_table_ptr = UREALLOC (tl_table_ptr, TL_Table *,
						 tl_index + 1);
			tl_table_ptr[tl_index] = tl_table;

			++tl_index;
		    }
		}

		++j;
	    }
	}

	/*
	 * Next, read station/TL/model-dependent (STM) information!  Skip
	 * lines with a '#' in the first column.  Determine TLtype index to
	 * tltype_model_descrip, if available.  If not available, then set
	 * a new TL index to ensure reading of STM-specific TL tables later
	 * on.  Three cases can result in the need for an additional TL
	 * index.  These include a newly encountered TL model for a given
	 * TLtype; a declared phase dependency; a declared phase/chan
	 * dependency.  A chan dependency cannot exist in the absence of
	 * a phase dependency within this specification area.
	 */

	j = 0;
	if (num_STM > 0)
	{
	    while (fgets (input_string, BUFSIZ, tlsf_fp) != NULL)
	    {
		/* Skip lines w/ '#' or blank line ('\n') in 1st column */
		if (strncmp (input_string, "#", 1) &&
		    strncmp (input_string, "\n", 1)) 
		{
		    if (sscanf (input_string, "%s%s%s%s%s", 
				sta, TLtype, tl_model, phase, chan) != 5)
		    {
			fclose (tlsf_fp);
			READ_ERR("station/TLtype/model info");
			return (TLreadErr2);
		    }

		    tl = list_of_TLtypes;
		    TLtype_found = FALSE;
		    while (tl != (TL_Pt *) NULL)
		    {
			if (STREQ (TLtype, tl->TLtype))
			{
			    for (i = 0; i < num_sites; i++)
			    {
				if (STREQ (sta, sites[i].sta))
				{
				    TLtype_found = TRUE;
				    break;
				}
			    }
			    break;
			}
			tl = tl->next;
		    }

		    if (!TLtype_found)
			continue;	/* This TLtype not requested */

		    strcpy (sta_tl_model[j].sta, sta);
		    strcpy (sta_tl_model[j].TLtype, TLtype);
		    strcpy (sta_tl_model[j].model, tl_model);
		    strcpy (sta_tl_model[j].phase, phase);
		    strcpy (sta_tl_model[j].chan, chan);

		    /*
		     * Here we wish to only investigate whether duplicate
		     * information has been specified.  If so, a warning
		     * message should be given.  The processing will continue
		     * unimpeded, but this line of the STM section will be
		     * ignored.  Two kinds of duplication are possible:
		     * (1) a redundancy with information specified within
		     * tltype_model_descrip[]; and (2) a duplication with a
		     * previous STM definition.  Only the first is checked
		     * here.  The second check will be undertaken soon
		     * thereafter.
		     */

		    sta_tl_model[j].tl_index = -1;
		    for (TLMD_found = FALSE, i = 0; i < num_TLMD; i++)
		    {
			if (sta_tl_model[j].chan == (char *) NULL ||
			    STREQ (sta_tl_model[j].chan, "-"))
			{
			    if (STREQ (sta_tl_model[j].model, 
					tltype_model_descrip[i].model) &&
			        STREQ (sta_tl_model[j].TLtype, 
					tltype_model_descrip[i].TLtype))
			    {
				/*
				 * If phase type also maps to a valid
				 * tltype_model_descrip structure, then a
				 * redundant record has been encountered.
				 * Send a warning message and ignore this
				 * STM definition.
				 */

				phz = tltype_model_descrip[i].list_of_phz;
				while (phz != (List_of_Phz *) NULL)
				{
				    if (STREQ (sta_tl_model[j].phase,
						phz->phase))
				    {
					TLMD_found = TRUE;
					break;
				    }
				    phz = phz->next;
				}
				if (TLMD_found)
				    break;
			    }
			}
		    }

		    if (TLMD_found)
		    {
			fprintf (stderr, "%s: Warning! STM: %s/%s/%s line\n", 
					routine, sta_tl_model[j].sta,
					sta_tl_model[j].TLtype,
					sta_tl_model[j].model);
			fprintf (stderr, "  found to be redundant with info specified in tltype_model_descrip!\n"); 
			fprintf (stderr, "STM line: %s\n  will be ignored!\n\n",
					 input_string);
			continue;	/* A redundancy found */
		    }

		    /*
		     * Now check for a clear duplication of an STM record.
		     * If found, skip this piece of STM info and go on.
		     */

		    STM_duplicate_found = FALSE;
		    for (i = 0; i < j; i++)
		    {
			if (STREQ (sta_tl_model[j].sta,
				   sta_tl_model[i].sta) &&
			    STREQ (sta_tl_model[j].TLtype,
				   sta_tl_model[i].TLtype) &&
			    STREQ (sta_tl_model[j].model,
				   sta_tl_model[i].model) &&
			    STREQ (sta_tl_model[j].chan,
				   sta_tl_model[i].chan))
			{
			    STM_duplicate_found = TRUE;
			    break;
			}
		    }

		    if (STM_duplicate_found)
		    {
			fprintf (stderr, "%s: Warning! STM: %s/%s/%s line\n", 
					routine, sta_tl_model[j].sta,
					sta_tl_model[j].TLtype,
					sta_tl_model[j].model);
			fprintf (stderr, "  found to be a duplicate with another STM record!\n"); 
			fprintf (stderr, "STM line: %s\n  will be ignored!\n\n",
					 input_string);
			continue;	/* A duplicate STM record found */
		    }

		    /*
		     * The current STM specified TL model must have been 
		     * defined when the tl_model_path structure was built.
		     * If not, this STM record will be ignored.  This is a
		     * serious blunder which should be remedied ASAP.  For
		     * now, simply ignore current STM record and go on.  Also
		     * check to make sure a TLtype as specified here has been 
		     * defined in tltype_model_descrip section.  This is not 
		     * quite as serious as the missing model, but any 
		     * associated STM record should be ignored as well.
		     */

		    for (tltype_found = FALSE, i = 0; i < num_TLMD; i++)
		    {
			if (STREQ (sta_tl_model[j].TLtype,
				   tltype_model_descrip[i].TLtype))
			{
			    sta_tl_model[j].model_index =
					tltype_model_descrip[i].model_index;
			    tltype_found = TRUE;
			    break;
			}
		    }

		    if (! tltype_found)
		    {
			fprintf (stderr, "%s: Error! STM: %s/%s/%s line\n", 
					routine, sta_tl_model[j].sta,
					sta_tl_model[j].TLtype,
					sta_tl_model[j].model);
			fprintf (stderr, "  not associated with any tltype_model_descrip TLtype definition!\n");
			fprintf (stderr, "STM line: %s\n  will be ignored!\n\n",
					 input_string);
			continue;	/* Missing TLtype specification */
		    }

		    /*
		     * All checks are now done, so now we can finally attempt 
		     * to read a single TL table given the current STM request.
		     */

		    k = sta_tl_model[j].model_index;

		    iret =
		    read_tl_table (tl_model_path[k].dir_pathway,
				   sta_tl_model[j].TLtype,
				   sta_tl_model[j].model, sta_tl_model[j].phase,
				   sta_tl_model[j].chan, &tl_table);
		    if (iret != OK)
		    {
			if (iret == TLreadWarn1)
			    continue;
			else
			{
			    fclose (tlsf_fp);
			    return (iret);
			}
		    }

		    tl_table_ptr = UREALLOC (tl_table_ptr, TL_Table *,
					     tl_index + 1);
		    tl_table_ptr[tl_index] = tl_table;

		    sta_tl_model[j].tl_index = tl_index;
		    ++tl_index;

		    j++;
		}
	    }
	}
	num_STM = j;  /* Reset since some STM records may be skipped */
	fclose (tlsf_fp);

	num_TL_tables = tl_index;

	if (num_TL_tables == 0)	/* No files could be opened */
	{
	    fprintf (stderr, "%s: No TL tables could be opened!\n", routine);
	    return (TLreadErr3);
	}

	return (OK);
} 


double
#ifdef	__STDC__
interp_for_tl_value (double distance, double depth, int tl_index, 
		     Bool extrapolate, double *tl_deriv, int *interp_code)
#else
interp_for_tl_value (distance, depth, tl_index, extrapolate, tl_deriv,
		     interp_code)

double	distance;
double	depth;
int	tl_index;
Bool	extrapolate;
double	*tl_deriv;	/* Principal radial and depth TL derivatives */
int	*interp_code;
#endif
{

	Bool	in_hole;
	int	fatal_err = 0;
	float	value;
	float	x_1st_deriv, z_1st_deriv, x_2nd_deriv, z_2nd_deriv;
	double	dist_depth_corr = -999.0;
	TL_Table *mc_table;


	if (tl_index < 0)
	    return (-999.0);	/* TLtype index not found */
	if (tl_index > num_TL_tables)
	{
	    fprintf (stderr, "interp_for_tl_value: tl_index is too large!!!\n");
	    return (-999.0);	/* TLtype index not found */
	}

	mc_table = tl_table_ptr[tl_index];

	/*
	 * If TL table for this TLtype at input distance has a hole, then
	 * set in_hole flag.
	 */

	if (distance > mc_table->in_hole_dist[0] &&
	    distance < mc_table->in_hole_dist[1])
	    in_hole = TRUE;
	else
	    in_hole = FALSE;

	/*
	 * Grab TL value and related derivative information.  A catastrophic
	 * error in this interpolation function will result in the fatal_err
	 * flag being returned as not OK (i.e., non-zero).  In this case,
	 * any interpolated value is simply to unreliable to be useful.  In
	 * practice, this does not happen very often at all.
	 */

	fatal_err =
	interpolate_table_value (extrapolate, in_hole, TRUE,
				 mc_table->num_dists, mc_table->num_depths,
				 mc_table->dist_samples,
				 mc_table->depth_samples, mc_table->tl,
				 (float)distance, (float)depth, &value,
				 &x_1st_deriv, &z_1st_deriv,
				 &x_2nd_deriv, &z_2nd_deriv, interp_code);

	if (fatal_err != OK)
	    return (-999.0);
	if (!extrapolate && *interp_code != 0)
	    return (-999.0);

	dist_depth_corr = (double) value;

	/*
	 * tl_deriv[] contains distance/depth derivatives measured along
	 * the principal distance/azimuth (radial) direction.
	 */

	tl_deriv[0] = (double)x_1st_deriv;	/* dTL/dr */
	tl_deriv[1] = (double)z_1st_deriv;	/* dTL/dz */
	tl_deriv[2] = (double)x_2nd_deriv;	/* d2TL/dr2 */
	tl_deriv[3] = (double)z_2nd_deriv;	/* d2TL/dz2 */

	return (dist_depth_corr);
}


double
#ifdef	__STDC__
get_tl_model_error (int tl_index, double delta, double depth, char *tl_model)
#else
get_tl_model_error (tl_index, delta, depth, tl_model)
int	tl_index;
double	delta;
double	depth;
char	*tl_model;
#endif
{

	Bool	max_depth_index_found = FALSE;
	int	i, m, n;
	int	num_dists, num_depths;
	double	ratio, zratio = 1.0;
	double	top, bot;
	double	model_error = NA_MODEL_ERROR;
	TL_Table  *mc_table;


	mc_table = tl_table_ptr[tl_index];

	/*
	 *  If no modelling error is specified for this TLtype, then
	 *  return the baseline standard deviation from the MDF for
	 *  the appropriate magtype.
	 */

	strcpy (tl_model, mc_table->model);
	if (mc_table->tl_mdl_err == (TL_Mdl_Err *) NULL)
	    return (model_error);

	num_dists  = mc_table->tl_mdl_err->num_dists;
	num_depths = mc_table->tl_mdl_err->num_depths;

	if (num_depths == 1)
	{
	    if (num_dists == 1)
	    {
		/*
		 * Only a single-valued modelling error exists for this
		 * phase (bulk_var).
		 */

		 model_error = (double)mc_table->tl_mdl_err->bulk_var;
	    }
	    else
	    {
		/*
		 * Only distance-dependent modelling errors exists for this
		 * TLtype.  Obtain modelling error via simple linear 
		 * interpolation.  If out of valid distance range, then
		 * simply return modelling error from nearest end point.
		 */

		n = num_dists - 1;
		if (delta < mc_table->tl_mdl_err->dist_samples[0])
		    model_error = (double)mc_table->tl_mdl_err->dist_var[0];
		else if (delta > mc_table->tl_mdl_err->dist_samples[n])
		    model_error = (double)mc_table->tl_mdl_err->dist_var[n];
		else
		{
		    for (i = 0; i < num_dists; i++)
		    {
			if (delta < mc_table->tl_mdl_err->dist_samples[i])
			{
			    n = i;	/* High end */
			    break;
			}
		    }
		    ratio = (delta-mc_table->tl_mdl_err->dist_samples[n-1])
			    / (mc_table->tl_mdl_err->dist_samples[n] -
			       mc_table->tl_mdl_err->dist_samples[n-1]);
		    model_error = 
			mc_table->tl_mdl_err->dist_var[n-1] +
			ratio * (mc_table->tl_mdl_err->dist_var[n] - 
				 mc_table->tl_mdl_err->dist_var[n-1]);
		}
	    }
	}
	else
	{
	    /*
	     * Complete distance/depth-dependent modelling errors exist
	     * for this TLtype. Obtain modelling error via simple bi-
	     * linear interpolation.  If out of valid distance or depth
	     * range, simply return modelling error from nearest end point.
	     */

	    for (i = 0; i < num_depths; i++)
	    {
		if (mc_table->tl_mdl_err->depth_samples[i] > depth)
		{
		    m = i;		/* m: Deepest depth index */
		    zratio = (depth - 
			      mc_table->tl_mdl_err->depth_samples[i-1]) /
			     (mc_table->tl_mdl_err->depth_samples[i] -
			      mc_table->tl_mdl_err->depth_samples[i-1]);
		    max_depth_index_found = TRUE;
		    break;
		}
	    }
	    if (!max_depth_index_found)
		m = num_depths - 1;

	    n = num_dists - 1;
	    if (delta < mc_table->tl_mdl_err->dist_samples[0])
	    {
		if (max_depth_index_found)
		    model_error = 
			mc_table->tl_mdl_err->dist_depth_var[m-1][0] +
			zratio * 
			(mc_table->tl_mdl_err->dist_depth_var[m][0] -
			 mc_table->tl_mdl_err->dist_depth_var[m-1][0]);
		else
		    model_error = mc_table->tl_mdl_err->dist_depth_var[m][0];
	    }
	    else if (delta > mc_table->tl_mdl_err->dist_samples[n])
	    {
		if (max_depth_index_found)
		    model_error = 
			mc_table->tl_mdl_err->dist_depth_var[m-1][n] +
			zratio * 
			(mc_table->tl_mdl_err->dist_depth_var[m][n] -
			 mc_table->tl_mdl_err->dist_depth_var[m-1][n]);
		else
		    model_error = mc_table->tl_mdl_err->dist_depth_var[m][n];
	    }
	    else if (!max_depth_index_found)
	    {
		for (i = 0; i < num_dists; i++)
		{
		    if (delta < mc_table->tl_mdl_err->dist_samples[i])
		    {
			n = i;
			break;
		    }
		}
		model_error = 
		    (double) mc_table->tl_mdl_err->dist_depth_var[m][n];
	    }
	    else
	    {
		for (i = 0; i < num_dists; i++)
		{
		    if (delta < mc_table->tl_mdl_err->dist_samples[i])
		    {
			ratio = (delta - 
				 mc_table->tl_mdl_err->dist_samples[i-1]) /
				(mc_table->tl_mdl_err->dist_samples[i] -
				 mc_table->tl_mdl_err->dist_samples[i-1]);
			bot = mc_table->tl_mdl_err->dist_depth_var[m][i-1] +
			      ratio *
			      (mc_table->tl_mdl_err->dist_depth_var[m][i] -
			       mc_table->tl_mdl_err->dist_depth_var[m][i-1]);
			top = mc_table->tl_mdl_err->dist_depth_var[m][i] +
			      ratio *
			      (mc_table->tl_mdl_err->dist_depth_var[m-1][i] -
			       mc_table->tl_mdl_err->dist_depth_var[m-1][i-1]);
			model_error = top + zratio * (bot - top);
			break;
		    }
		}
	    }
	}

	return (model_error);
}


Bool
#ifdef	__STDC__
valid_phase_for_TLtype (char *TLtype, char *phase)
#else
valid_phase_for_TLtype (TLtype, phase)
char	*TLtype;
char	*phase;
#endif
{
	int		tlmd_index = -1;
	List_of_Phz	*phz;

	if ((tlmd_index = get_TLMD_index (TLtype)) < 0)
	   return (FALSE);	/* Bad TLtype specified */
	phz = tltype_model_descrip[tlmd_index].list_of_phz;
	while (phz != (List_of_Phz *) NULL)
	{
	    if (STREQ (phase, phz->phase))
		return (TRUE);	/* Valid phase type found */
	    phz = phz->next;
	}
	return (FALSE);	/* Phase type is invalid for given TLtype */
}


Bool
#ifdef	__STDC__
valid_range_for_TLtable (char *TLtype, char *sta, char *phase, char *chan,
			 double delta, double ev_depth)
#else
valid_range_for_TLtable (TLtype, sta, phase, chan, delta, ev_depth)
char	*TLtype;
char	*sta;
char	*phase;
char	*chan;
double	delta;
double	ev_depth;
#endif
{
	int	 tl_index;
	int	 sta_index, stm_index, tlmd_index;
	TL_Table *mc_table;

	if ((tl_index = 
	     get_TL_indexes (TLtype, sta, phase, chan, &sta_index, &stm_index,
			     &tlmd_index)) < 0)
	    return (FALSE);	/* Bad TLtype specified */

	mc_table = tl_table_ptr[tl_index];
	if (mc_table->num_dists > 1 &&
	    (delta < mc_table->dist_samples[0] ||
	     delta > mc_table->dist_samples[mc_table->num_dists - 1]))
	    return (FALSE); 	/* Invalid distance range */
	if (mc_table->num_depths > 1 &&
	    (ev_depth < mc_table->depth_samples[0] ||
	     ev_depth > mc_table->depth_samples[mc_table->num_depths - 1]))
	    return (FALSE);	/* Invalid depth range */

	return (TRUE);	/* Valid distance/depth range */
}


int
#ifdef	__STDC__
get_TL_indexes (char *TLtype, char *sta, char *phase, char *chan,
		int *sta_index, int *stm_index, int *tlmd_index)
#else
get_TL_indexes (TLtype, sta, phase, chan, sta_index, stm_index, tlmd_index)
char	*TLtype;
char	*sta;
char	*phase;
char	*chan;
int	*sta_index;
int	*stm_index;
int	*tlmd_index;
#endif
{
	Bool	TLtype_found = FALSE;
	int	i;
	int	tl_index = -1;
	int	num_depend = 0;
	TL_Pt	*tl_ptr;
	List_of_Phz *ph;

	*stm_index = -1;
	*sta_index = -1;
	*tlmd_index = -1;

	if (!TLtype || !*TLtype)
	    return (ERR);

	/*
	 * First, determine default tl_index from tltype_model_descrip 
	 * definition.  Will need to inspect list_of_phz elements based
	 * on input phase specified to get specific tl_index.
	 */

	for (i = 0; i < num_TLMD; i++)
	{
	    if (STREQ (tltype_model_descrip[i].TLtype, TLtype))
	    {
		TLtype_found = TRUE;
		ph = tltype_model_descrip[i].list_of_phz;
		while (ph != (List_of_Phz *) NULL)
		{
		    if (STREQ (phase, ph->phase))
		    {
			tl_index = ph->tl_index;
			*tlmd_index = i;
			break;
		    }
		    ph = ph->next;
		}
		break;
	    }
	}

	if (!TLtype_found)
	    return (ERR);	/* Input TLtype not defined */

	for (i = 0; i < num_site; i++)
	{
	    if (STREQ (sta, site[i].sta))
	    {
		*sta_index = i;
		break;
	    }
	}
	if (*sta_index < 0)
	    return (ERR);	/* Input station not defined in site table */

	tl_ptr = sta_pt[*sta_index].tl_ptr;
	if (tl_ptr != (TL_Pt *) NULL)
	{
	    /*
	     * If specific station/TLtype information has been specified
	     * set *stm_index and transmission loss (TL) index (i) with
	     * highest degree of dependency given input phase and chan 
	     * info.
	     */

	    while (tl_ptr != (TL_Pt *) NULL)
	    {
		if (STREQ (TLtype, tl_ptr->TLtype))
		{
		    if (STREQ (phase, tl_ptr->phase))
		    {
			if (STREQ (chan, tl_ptr->chan))
			{
			    /*
			     * Highest level of dependency encountered, so
			     * simply return with tl_index.
			     */

			    *stm_index = tl_ptr->stm_index;
			    tl_index = tl_ptr->tl_index;
			    break;
			}
			else if (STREQ (tl_ptr->chan, "-"))
			{
			    /*
			     * Best dependency, thus far, is restricted to
			     * phase.
			     */

			    *stm_index = tl_ptr->stm_index;
			    tl_index = tl_ptr->tl_index;
			    num_depend = 2;
			}
			else
			    ;	/* Incorrect chan match, so just skip */
		    }

		    if (STREQ (tl_ptr->phase, "-") && STREQ (tl_ptr->chan, "-")
			&& num_depend < 2)
		    {
			/*
			 * So far, only TLtype dependency specified.  Thus,
			 * set indexes in case no greater dependency is found
			 * for this station/TLtype pair.  Note that since no
			 * dependency has been specified, it must match the
			 * phase dependency belonging to the default TLtype
			 * as detailed in the tltype_model_descrip structure.
			 * If a match is found, then the tl_index and
			 * tlmd_index fields will already have valid values
			 * (i.e., >= 0).  If the model is different here,
			 * than that set in tltype_model_descrip, we do still
			 * need to set a tl_index that maps to this STM info.
			 */

			if (tl_index > -1 && *tlmd_index > -1)
			{
			    *stm_index = tl_ptr->stm_index;
			    tl_index = tl_ptr->tl_index;
			    num_depend = 1;
			}
		    }
		}
		tl_ptr = tl_ptr->next;
	    }
	}

	if (tl_index > -1)
	    return (tl_index);
	else
	    return (ERR);
}


int
#ifdef	__STDC__
get_TLMD_index (char *TLtype)
#else
get_TLMD_index (TLtype)
char	*TLtype;
#endif
{
	int	i;

	for (i = 0; i < num_TLMD; i++)
	    if (STREQ (TLtype, tltype_model_descrip[i].TLtype))
		return (i);

	return (ERR);	/* No TLMD index found */
}


int
#ifdef	__STDC__
set_sta_TL_pt (Site *sites, int num_sites)
#else
set_sta_TL_pt (sites, num_sites)
Site	*sites;
int	num_sites;
#endif
{
	Bool	refill_sta_pt = FALSE;
	int	i, j;

	TL_Pt	*tlp, *cur, *prev, *next;

	static	int	save_num_sites = -1;
	static	char	**save_site_list = (char **) NULL;
	static	int	first_call = TRUE;

	static	char	routine[] = "set_sta_TL_pt";


	/*
	 * First, make sure an input site table exists!
	 */

	if (sites == (Site *) NULL || num_sites == 0)
	{
	    fprintf (stderr, "\n%s: site table not specified or empty!\n", 
				routine);
	    fprintf (stderr, "       Function, %s(), cannot be called until site table is avaiable!\n", routine);
	    return (SSgetErr1);
	}

	if (sta_pt != (Sta_Pt *) NULL)
	{
	    if (! first_call)
	    {
		if (num_sites != save_num_sites)
		    refill_sta_pt = TRUE;
		else
		{
		    for (i = 0; i < num_sites; i++)
		    {
			if (strcmp (sites[i].sta, save_site_list[i]))
			{
			    refill_sta_pt = TRUE;
			    break;
			}
		    }
		}
		if (!refill_sta_pt)
		    return (OK);
	    }
	}

	/*
	 * If we need to re-allocate new memory, first free up previous
	 * arrays and structures (refill_sta_pt == TRUE).
	 */

	if (refill_sta_pt)
	{
	    for (i = 0; i < save_num_sites; i++)
	    {
		UFREE (save_site_list[i]);
		tlp = sta_pt[i].tl_ptr;
		while (tlp != (TL_Pt *) NULL)
		{
		    next = tlp->next;
		    UFREE (tlp);
		    tlp = next;
		}
	    }
	    UFREE (save_site_list);
	    UFREE (sta_pt);
	    sta_pt = (Sta_Pt *) NULL;
	}

	/*
	 * Allocate memory for sta_pt and tl_ptr structures as well as
	 * save_site_list array based on input site table.
	 */

	if ((sta_pt = (Sta_Pt *) calloc (num_sites, sizeof (Sta_Pt))) == NULL)
	    return (SSgetErr2);
	save_site_list = UALLOC (char *, num_sites);
	for (i = 0; i < num_sites; i++)
	{
	    save_site_list[i] = UALLOC (char, 7);
	    strcpy (save_site_list[i], sites[i].sta);
	    sta_pt[i].tl_ptr = (TL_Pt *) NULL;
	}
	save_num_sites = num_sites;

	/*
	 * Now let's fill sta_pt and attached tl_ptr structures based on the
	 * current contents of the station-specific TLtype/model info
	 * contained within structure, sta_tl_model.  We will do this, one
	 * station at-a-time, until all phases have been processed (i.e.,
	 * added to sta_pt link list of TLtype pointer (tl_ptr)).  If no
	 * TLtype are encountered for a given station, then the sta_pt will
	 * contain a NULL pointer to TL_Pt.  Do not include stations from 
	 * sta_tl_model not contained in the site table.
	 */

	for (i = 0; i < num_sites; i++)
	{
	    for (j = 0; j < num_STM; j++)
	    {
		if (STREQ (sites[i].sta, sta_tl_model[j].sta))
		{
		    tlp = sta_pt[i].tl_ptr;
		    if (tlp == (TL_Pt *) NULL)
		    {
			if ((sta_pt[i].tl_ptr =
				(TL_Pt *) calloc (1, sizeof (TL_Pt))) == NULL)
			    return (SSgetErr2);
			tlp = sta_pt[i].tl_ptr;
		    }
		    else
		    {
			if ((cur =
				(TL_Pt *) calloc (1, sizeof (TL_Pt))) == NULL)
			    return (SSgetErr2);
			prev = tlp;
			while (tlp != (TL_Pt *) NULL)
			{
			    prev = tlp;
			    tlp = tlp->next;
			}
			prev->next = cur;
			tlp = cur;
		    }
		    strcpy (tlp->TLtype, sta_tl_model[j].TLtype);
		    strcpy (tlp->phase, sta_tl_model[j].phase);
		    strcpy (tlp->chan, sta_tl_model[j].chan);
		    tlp->stm_index = j;
		    tlp->tl_index = sta_tl_model[j].tl_index;
		    tlp->next = NULL;
		}
	    }
	}

	site = sites;	/* Set a local pointer copy of site table */
	num_site = num_sites;
	first_call = FALSE;

	return (OK);
}


Bool
#ifdef	__STDC__
get_TL_ts_corr (char *ts_region, char *sta, char *TLtype, int tl_index,
		 double *ts_corr)
#else
get_TL_ts_corr (ts_region, sta, TLtype, tl_index, ts_corr)

char	*ts_region;
char	*sta;
char	*TLtype;
int	tl_index;
double	*ts_corr;
#endif
{
	Bool	ts_region_found = FALSE;
	int	i;
	int	ts_reg = -1;
	TL_Table  *mc_table;
	TL_TS_Cor *mtsc = (TL_TS_Cor *) NULL;


	*ts_corr = 0.0;
	mc_table = tl_table_ptr[tl_index];
	if (ts_region != (char *) NULL && strcmp (ts_region, "") &&
	    strcmp (ts_region, "-") && mc_table->num_ts_regions > 0)
	{
	    for (i = 0; i < mc_table->num_ts_regions; i++)
	    {
		mtsc = &mc_table->tl_ts_cor[i];
		if (STREQ (ts_region, mtsc->reg_name_id))
		{
		    ts_region_found = TRUE;
		    if (STREQ (mtsc->TLtype, TLtype))
		    {
			ts_reg = i;
			break;
		    }
		}
	    }
	    if (!ts_region_found)
		fprintf (stderr, " Requested mag. test-site region: %s, not available!\n", ts_region);
	    if (ts_reg > -1)
	    {
		mtsc = &mc_table->tl_ts_cor[ts_reg];
		for (i = 0; i < mtsc->num_sta; i++)
		{
		    if (STREQ (sta, mtsc->sta[i]))
		    {
			*ts_corr = mtsc->ts_corr[i];
			return (TRUE);
		    }
		}
	    }
	}

	return (FALSE);
}


void
free_tl_table () 
{
	int	 i, j;
	TL_Table *mc_table = NULL;

	for (i = 0; i < num_TL_tables; i++)
	{
	    mc_table = tl_table_ptr[i];
	    for (j = 0; j < mc_table->num_depths; j++)
		UFREE (mc_table->tl[j]);
	    UFREE (mc_table->tl);

	    if (mc_table->tl_mdl_err != NULL)
	    {
		if (mc_table->tl_mdl_err->dist_depth_var != NULL)
		{
		    for (j = 0; j < mc_table->tl_mdl_err->num_depths;j++)
			UFREE (mc_table->tl_mdl_err->dist_depth_var[j]);
		    UFREE (mc_table->tl_mdl_err->dist_depth_var);
		}
		UFREE (mc_table->tl_mdl_err);
	    }
	    if (mc_table->tl_ts_cor != NULL)
	    {
		for (j = 0; j < mc_table->num_ts_regions; j++)
		    UFREE (mc_table->tl_ts_cor[j].ts_corr);
		UFREE (mc_table->tl_ts_cor);
		mc_table->tl_ts_cor = (TL_TS_Cor *) NULL;
	    }
	}
	UFREE (mc_table);
	mc_table = (TL_Table *) NULL;
}


double
#ifdef	__STDC__
get_delta_for_sta (char *sta, double ev_lat, double ev_lon)
#else
get_delta_for_sta (sta, ev_lat, ev_lon)

char	*sta;
double	*ev_lat;
double	*ev_lon;
#endif
{
	int	i;
	double	delta, esaz, seaz;

	for (i = 0; i < num_site; i++)
	{
	    if (STREQ (sta, site[i].sta))
	    {
		dist_azimuth (site[i].lat, site[i].lon, ev_lat, ev_lon,
			      &delta, &seaz, &esaz, 0);
		return (delta);
	    }
	}

	return (-1.0);	/* Station not found in site list */
}


