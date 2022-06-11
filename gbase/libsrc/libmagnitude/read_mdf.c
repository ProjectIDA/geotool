
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * NAME
 *	read_mdf -- Read magnitude description info from MDF

 * FILE
 *	read_mdf.c

 * SYNOPSIS
 *	int
 *	read_mdf (mdf_filename, list_of_magtypes, num_req_magtypes,
 *		  mag_descrip_ptr, num_md, mag_sta_tltype_ptr, num_mst,
 *		  list_of_TLtypes_ptr)
 *	char	*mdf_filename;		(i) Full filename w/ directory pathway 
 *					    prepended, containing location of
 *					    Magnitude Description File (MDF)
 *	char	**list_of_magtypes;	(i) Array of magtypes to limit number of
 *					    MC tables that need to be read
 *	int	num_req_magtypes;	(i) Number of requested magtypes
 *					    (# of elements in list_of_magtypes)
 *	Mag_Descrip **mag_descrip_ptr;	(o) Populated magnitude description
 *					    info as linked to critical TL info
 *	int	*num_md;		(o) Number of valid elements in 
 *					    structure, mag_descrip_ptr
 *	Mag_Sta_TLType **mag_sta_tltype_ptr;
 *					(o) Populated station/TLtype-specific
 *					    magnitude info 
 *	int	*num_mst;		(o) Number of valid elements in 
 *					    structure, mag_sta_tltype_ptr
 *	TL_Pt	**list_of_TLtypes_ptr;	(o) Pointer to array of TLtypes to limit
 *					    # of MC tables that need to be read

 * DESCRIPTION
 *	Function.  Employed to read magnitude-specific description information
 *	stored in Magnitude Description File (MDF), dependent on the successful
 *	reading of the transmission loss (TL) info access via functions,
 *	read_tlsf() and read_tl_table().  This dependency is one way.  The 
 *	TL info is not dependent of any information created here, except for
 *	a potential restriction based on list of TLtypes that may be specified
 *	here.  This latter restriction is only meaningful if a restricted list
 *	of magtypes is passed into this routine.  This capability is provided
 *	for efficiency purposes to limit the ammount of information that 
 *	actually needs to be read.  In such cases, it is imperative that this
 *	function be called prior to read_tlsf().

 * DIAGNOSTICS
 *	See mag_error_table[] in file, mag_error_msg.c, for specific global
 *      magnitude error code descriptions.  Codes that can be returned here
 *      include:
 *	    OK (0):	Everything read correctly
 *	    MDreadErr1:	Could not open OR re-open MDF
 *	    MDreadErr2:	MDF was found to be empty.
 *	    MDreadErr3:	No matching TLtype found to info specified in TLSF.
 *	    MDreadErr4:	Error allocating memory while reading MD info.

 * FILES
 *	Magnitude Description File (MDF) is read here. 

 * NOTES
 *	The acronym, TL, stands for the transmission loss.  MDF is an acronym
 *	acronym for Magnitude Description File.

 * SEE ALSO
 *	read_tlsf() for reading of transmission loss specification file (TLSF)
 *	info.  The list of TLtypes argument required by read_tlsf() is
 *	established here.

 * AUTHOR
 *	Walter Nagy, 9/12/97,	Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libmagnitude.h"


/*
#ifndef	lint
static	char	SccsId[] = "@(#)read_mdf.c	4.1	10/30/97	SAIC.";
#endif
 */

/* Error report */
#define	READ_ERR(a)	{ fprintf (stderr, "\nread_mdf: Error reading %s in file: %s\n", (a), mdf_filename); \
			}
#define	CALLOC_ERR(a)	{ fprintf (stderr, "\nread_mdf: Error allocating for %s in file: %s\n", (a), mdf_filename); \
			}

int
#ifdef	__STDC___
read_mdf (char *mdf_filename, char **list_of_magtypes, int num_req_magtypes,
	  Mag_Descrip **mag_descrip_ptr, int *num_md, 
	  Mag_Sta_TLType **mag_sta_tltype_ptr, int *num_mst,
	  char ***list_of_TLtypes_ptr)
#else
read_mdf (char *mdf_filename, char **list_of_magtypes, int num_req_magtypes, 
Mag_Descrip **mag_descrip_ptr, 
	int *num_md, Mag_Sta_TLType **mag_sta_tltype_ptr, 
int *num_mst, TL_Pt **list_of_TLtypes_ptr)

/*char		*mdf_filename;*/		/* MDF filename */
     /*char		**list_of_magtypes;*/	/* Restrict magtypes read to those
					   included in this list only */
     /*int	num_req_magtypes;*/	/* Number of requested magtypes in
					   list_of_magtypes array */
     /*Mag_Descrip	**mag_descrip_ptr;*/	/* Pointer to magnitude/TL/tl_model
					   info structure */
     /*int		*num_md;*/		/* length of mag_descrip_ptr */
     /*Mag_Sta_TLType	**mag_sta_tltype_ptr;*/	/* Pointer to mag_sta_tltype info */
     /*int		*num_mst;*/		/* length of mag_sta_tltype_ptr */
     /*TL_Pt		**list_of_TLtypes_ptr;*/	/* List of TLtypes found that map from
					   list_of_magtypes passed in here */
#endif
{

	FILE	*mdf_fp;
	Bool	TLtype_duplicate_found = FALSE;
	Bool	def_sta_corr_found = FALSE;
	Bool	apply_wgt;
	int	i, j, k;
	int	algo_code;
	int	num_MDS = 0;
	int	num_MSTS = 0;
	int	num_mdf_args = 11;
	int	num_tlsf_args = 4;
	int	ierr;
	float	dist_min, dist_max;
	float	sglim1, sglim2, sgbase;
	char	magtype[9], TLtype[9];
	char	det_amptype[9], ev_amptype[9];
	char	input_string[BUFSIZ];

	Mag_Descrip	*mag_descrip = (Mag_Descrip *) NULL;
	Mag_Sta_TLType	*mag_sta_tltype = (Mag_Sta_TLType *) NULL;
	TL_Pt		*list_of_TLtypes = (TL_Pt *) NULL;
	TL_Pt		*prev = (TL_Pt *) NULL;
	TL_Pt		*tl = (TL_Pt *) NULL;

	static	char	routine[] = "read_mdf";


	mag_descrip = *mag_descrip_ptr;
	mag_sta_tltype = *mag_sta_tltype_ptr;
	list_of_TLtypes = *list_of_TLtypes_ptr;
	*num_md = 0;
	*num_mst = 0;

	/*
	 * Open Magnitude Description File (MDF) once here to determine the
	 * required memory that will need to be allocated for the mag_descrip
	 * structure.  Then close file, re-open, and do actual filling of
	 * structures.  A blank space or '#' in the first column can be
	 * used anywhere within this file for comments.
	 */

	if ((mdf_fp = fopen (mdf_filename, "r")) == NULL)
	{
	    fprintf (stderr, "\n%s: MDF: %s will not open!\n", routine,
				mdf_filename);
	    return (MDreadErr1);
	}

	while (fgets (input_string, BUFSIZ, mdf_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (strncmp (input_string, "#", 1))
	    {
		if (!strncmp (input_string, " ", 1) ||
		    !strncmp (input_string, "\n", 1))
		    break;
		sscanf (input_string, "%s%s", magtype, TLtype);
		for (i = 0; i < num_req_magtypes; i++)
		{
		    if (STREQ (magtype, list_of_magtypes[i]))
		    {
			++num_MDS;     /* # of TLtype/tl_model relationships */
			break;
		    }
		}
	    }
	}

	while (fgets (input_string, BUFSIZ, mdf_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (!strncmp (input_string, "#", 1) ||
		!strncmp (input_string, "\n", 1))
		continue;
	    else
		++num_MSTS;     /* Number of mag_sta_tltype relationships */
	}
	fclose (mdf_fp);

	if (num_MDS == 0)
	{
	    fprintf (stderr, "MDF is empty, so no magnitude info can be computed\n");
	    *num_md = num_MDS;
	    return (MDreadErr2);
	}

	/*
	 * Allocate memory for mag_descrip and mag_sta_tltype structures.
	 */

	if ((mag_descrip = (Mag_Descrip *)
			calloc (num_MDS, sizeof (Mag_Descrip))) == NULL)
	{
	    CALLOC_ERR ("structure, mag_descrip");
	    return (MDreadErr4);
	}

	if ((mag_sta_tltype = (Mag_Sta_TLType *)
			calloc (num_MSTS, sizeof (Mag_Sta_TLType))) == NULL)
	{
	    CALLOC_ERR ("structure, mag_sta_tltype");
	    return (MDreadErr4);
	}

	/*
	 * Re-open MDF!   Now we desire to do actual reading of magnitude-
	 * specific information that is independent of the requirements
	 * for employing generic transmission loss (TL) information.
	 */

	if ((mdf_fp = fopen (mdf_filename, "r")) == NULL)
	{
	    fprintf (stderr, "\n%s: MDF: %s cannot be re-opened!\n", routine,
				mdf_filename);
	    return (MDreadErr1);
	}

	/*
	 * Next, define magtype/TLtype/tl_model/amptype definitions.  If 
	 * given tl_model has not already been defined, complain loudly.
	 * NOTE: The last fgets() call here will have read the first line 
	 * after all the magtype/TLtype pairs have been specified.  A blank
	 * line seperates these definitions from the station/TLtype-specific
	 * magnitude specification section to be read subsequently.
	 */

	j = 0;
	while (fgets (input_string, BUFSIZ, mdf_fp) != NULL)
	{
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		ierr = sscanf (input_string, "%s%s%s%s%d%f%f%f%f%f%d",
					magtype, TLtype, det_amptype, 
					ev_amptype, &algo_code, &dist_min, 
					&dist_max, &sglim1, &sglim2, &sgbase,
					&apply_wgt);

		/*
		 *  Check for appropriate number of arguments
		 */
		if (ierr != num_mdf_args)
		{
		    fclose (mdf_fp);
		    fprintf (stderr, "Incorrect number of arguments per magtype in MDF: expecting %d, found %d\n", num_mdf_args, ierr);
		    return (MDreadErr2);
		}

		/*
		 *  Check that sglim1 <= sgbase <= sglim2
		 */
		if (sglim1 > sgbase || sgbase > sglim2)
		{
		    fclose (mdf_fp);
		    fprintf (stderr, "Incorrect LB, UB, or BL value for magtype %s in MDF:\n    Must satisfy LB <= BL <= UB\n", magtype);
		    return (MDreadErr2);
		}

		for (i = 0; i < num_req_magtypes; i++)
		{
		    if (STREQ (magtype, list_of_magtypes[i]))
		    {
			strcpy (mag_descrip[j].magtype, magtype);
			strcpy (mag_descrip[j].TLtype, TLtype);
			strcpy (mag_descrip[j].det_amptype, det_amptype);
			strcpy (mag_descrip[j].ev_amptype, ev_amptype);
			mag_descrip[j].algo_code = algo_code;
			mag_descrip[j].dist_min = dist_min;
			mag_descrip[j].dist_max = dist_max;
			mag_descrip[j].sglim1 = sglim1;
			mag_descrip[j].sglim2 = sglim2;
			mag_descrip[j].sgbase = sgbase;
			mag_descrip[j].apply_wgt = apply_wgt;
			strcpy (mag_descrip[j].orig_det_amptype, det_amptype);
			strcpy (mag_descrip[j].orig_ev_amptype, ev_amptype);
			mag_descrip[j].orig_algo_code = algo_code;
			mag_descrip[j].orig_dist_min = dist_min;
			mag_descrip[j].orig_dist_max = dist_max;
			mag_descrip[j].orig_sglim1 = sglim1;
			mag_descrip[j].orig_sglim2 = sglim2;
			mag_descrip[j].orig_sgbase = sgbase;
			mag_descrip[j].orig_apply_wgt = apply_wgt;

			/* MOVE
			if ((mag_descrip[j].tlmd_index = 
				get_TLMD_index (mag_descrip[j].TLtype)) < 0)
			{
			    fclose (mdf_fp);
			    fprintf (stderr, "%s: TLtype not specified for magtype/TLtype listing:\n   %s/%s\n", routine, mag_descrip[j].magtype, mag_descrip[j].TLtype);
			    return (MDreadErr3);
			}
			 */

			++j;
			break;
		    }
		}
	    }
	}
	*num_md = j;

	/* 
	 * Loop over number of valid magtype mag_descrip records to establish 
	 * a list of unique TLtypes that will need to be read from TLSF.  As
	 * identified, allocate memory for link list, list_of_TLtypes, of
	 * type, TL_Pt (as defined in include file, tl_table.h).
	 */

	tl = list_of_TLtypes;
	for (j = 0; j < *num_md; j++)
	{
	    TLtype_duplicate_found = FALSE;
	    for (i = 0; i < j; i++)
	    {
		if (STREQ (mag_descrip[j].TLtype, mag_descrip[i].TLtype))
		{
		    TLtype_duplicate_found = TRUE;
		    break;
		}
	    }
	    if (!TLtype_duplicate_found)
	    {
		if ((tl = (TL_Pt *) calloc (1, sizeof (TL_Pt))) == NULL)
		{
		    CALLOC_ERR ("structure, list_of_TLtypes");
		    return (MDreadErr4);
		}
		strcpy (tl->TLtype, mag_descrip[j].TLtype);
		tl->next = (TL_Pt *) NULL;
		if (prev != (TL_Pt *) NULL)
		    prev->next = tl;
		else
		    list_of_TLtypes = tl;
		prev = tl;
	    }
	}

	/*
	 * Now read station/TLtype-dependent magnitude information.  This is
	 * primarily used to store and retrieve the bulk static magnitude
	 * station correction and station correction error.
	 */

	j = 0;
	while (fgets (input_string, BUFSIZ, mdf_fp) != NULL)
	{
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		ierr = sscanf (input_string, "%s%s%f%f", 
					mag_sta_tltype[j].sta,
					mag_sta_tltype[j].TLtype,
					&mag_sta_tltype[j].bulk_sta_corr,
					&mag_sta_tltype[j].bulk_sta_corr_error);

		/*
		 *  Check for appropriate number of arguments
		 */
		if (ierr != num_tlsf_args)
		{
		    fclose (mdf_fp);
		    fprintf (stderr, "Incorrect number of arguments per sta/TLtype in MDF file: expecting %d, found %d\n", num_tlsf_args, ierr);
		    return (MDreadErr2);
		}

		/* MOVE
		if ((mag_sta_tltype[j].tlmd_index = 
				get_TLMD_index (mag_sta_tltype[j].TLtype)) < 0)
		{
		    fclose (mdf_fp);
		    fprintf (stderr, "%s: Error: TLtype: %s not specified for station: %s in MDF\n", routine, mag_sta_tltype[j].TLtype, mag_sta_tltype[j].sta);
		    fprintf (stderr, "  MST line: %s\n\n", input_string);
		    return (MDreadErr3);
		}
		 */

		++j;
	    }
	}
	*num_mst = j;

	/*
	 *  Determine whether or not default station corrections
	 *  and station correction errors exist for each unique
	 *  TLtype that has station corrections associated with it.
	 *  If a TLtype does not have a set of default station
	 *  corrections and errors (station name DFAULT) and a
	 *  weighted average is desired, then exit the program.
	 */

	for (k = 0; k < *num_md; k++)
	{
	    def_sta_corr_found = FALSE;
	    for (j = 0; j < *num_mst; j++)
	    {
		if (STREQ (mag_sta_tltype[j].sta, "DFAULT") &&
		    STREQ (mag_sta_tltype[j].TLtype, mag_descrip[k].TLtype))
		{
		    mag_descrip[k].def_sta_corr =
			mag_sta_tltype[j].bulk_sta_corr;
		    mag_descrip[k].def_sta_corr_error = 
			mag_sta_tltype[j].bulk_sta_corr_error;
		    def_sta_corr_found = TRUE;
		    break;
	        }
	    }

	    if (!def_sta_corr_found && mag_descrip[k].apply_wgt)
	    {
		fclose (mdf_fp);
		fprintf (stderr, "Station DFAULT not found in MDF file station correction list for TLtype %s.\n", mag_descrip[k].TLtype);
		return (MDreadErr2);
	    }
	}

	*mag_descrip_ptr = &mag_descrip[0];
	*mag_sta_tltype_ptr = &mag_sta_tltype[0];
	*list_of_TLtypes_ptr = &list_of_TLtypes[0];

	fclose (mdf_fp);

	return (OK);
}


