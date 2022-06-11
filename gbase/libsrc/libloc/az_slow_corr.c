
/*
 * Copyright (c) 1997-2004 Science Applications International Corporation.
 *

 * NAME
 *	read_sasc -- Read slowness/azimuth station correction (SASC) files
 *      load_single_sasc -- load single SASC file into memory
 *	read_single_sasc -- Read single SASC file (e.g., StaPro)
 *	correct_az_slow -- Correct various azimuth and slowness info
 *	get_ar_sasc -- Get a single ar_sasc structure given arid
 *	free_active_ar_sasc -- Free current array of ar_sasc structures
 *	make_sasc_adj_before_locator - Make SASC adjustments before location
 *	apply_sasc_adj_in_locator - Apply SASC adjustment in event locator?

 * FILE
 *	az_slow_corr.c

 * SYNOPSIS
 *	int
 *	read_sasc (sasc_pathway)
 *	char	*sasc_pathway;	(i) Directory pathway and filename prefix of
 *				    slowness/azimuth station correction files

 *	int
 *	load_single_sasc (sta_index)
 *	int	sta_index	(i) Station index in SASC_Tables structure

 *	int
 *	read_single_sasc (sasc_pathway, sta)
 *	char	*sasc_pathway;	(i) Directory pathway and filename prefix of
 *				    single SASC file
 *	char	*sta;		(i) Station name 

 *	void
 *	correct_az_slow (arid, sta, azimuth, slow, delaz, delslo, 
 *		 	 tot_az_err, tot_slow_err)

 *	int	arid;		(i)   Unique arrival ID
 *	char	*sta;		(i)   Station name 
 *	double	*azimuth;	(i/o) Raw azimuth (i); Corrected azimuth (o)
 *	double	*slow;		(i/o) Raw slowness (i); Corrected slowness (o)
 *	double	delaz;		(i)   Azimuth measurement error (from DFX)
 *	double	delslo;		(i)   Slowness measurement error (from DFX)
 *	double	*tot_az_err;	(o)   Total azimuth error (deg)
 *	double	*tot_slow_err;	(o)   Total slowness error (sec/deg)

 *	Ar_SASC
 *	get_ar_sasc (arid)
 *	int	arid;		(i) Unique arrival ID

 *	void
 *	free_active_ar_sasc ()

 *	void
 *	make_sasc_adj_before_locator () 

 *	Bool
 *	apply_sasc_adj_in_locator () 

 * DESCRIPTION
 *	-- read_sasc() puts paths to slowness/azimuth station correction
 *	files into SASC_Tables structure. Every path to file in the specified
 *	directory with the defined filename prefix will be put into SASC_Tables 
 *	structure.  For example, assuming sasc_pathway is,
 *	/prj/idc/ops/static/SASC/sasc , which includes SASC tables for 
 *	the stations, ASAR, NORES and GERES, then paths to 3 files would be saved:

 *	    /prj/idc/ops/static/SASC/sasc.ASAR
 *	    /prj/idc/ops/static/SASC/sasc.NORES
 *	    /prj/idc/ops/static/SASC/sasc.GERES

 *	-- load_single_sasc() reads a single slowness/azimuth station 
 *	correction (SASC) table from file into memory using path saved in
 *	SASC_Tables structure by read_sasc(). 

 *	-- read_single_sasc() reads a single slowness/azimuth station 
 *	correction (SASC) table from file into memory.  A single file in 
 *	the specified directory with the defined filename prefix will be 
 *	read.  For example, assuming we wish to only read SASC table for
 *	station, GERES, and sasc_pathway is, /prj/idc/ops/static/SASC/sasc, 
 *	then only the one file, /prj/idc/ops/static/SASC/sasc.GERES, would 
 *	be read.  This is currently only employed in StaPro.

 *	-- correct_az_slow() corrects for the station-specific slowness/
 *	azimuth field specified in the SASC table.  Unlike a source-specific 
 *	station correction (SSSC) which is applied to a theoretical 
 *	calculation, an SASC is applied to an observed slowness/azimuth 
 *	measure.  A correction is made to the raw azimuth and slowness as 
 *	well as their respective modeling errors.  An affine transform is
 *	applied prior to looking for a bin-corrected azimuth and slowness
 *	correction.  In most cases this transform will not apply any
 *	horizontal rotation, but provides a mechanism by which such as can
 *	be employed.  The affine transform contains a rotation element,
 *	defined by coefficients, a11, a12, a21 and a22, and a default sx/sy
 *	correction.  If a bin correction exists, then the coefficients and
 *	default slowness vector is combined to the value specified in the
 *	appropriate bin.

 *	-- get_ar_sasc() gets an azimuth and slowness corrected structure
 *	for a given arid from the array of actively stored Ar_SASC structures
 *	currently stored in memory.  At present this function is only
 * 	declared locally (privately) to libloc.  If external access is
 *	desired, make include file, ar_sasc.h, public in Makefile.
 
 *	-- free_active_ar_sasc() frees all current SASC information that was
 *	originally allocated in calls to correct_az_slow().  In general,
 *	external access to correct_az_slow() is limited a single instanti-
 *	ation when arrival records are read.  If the arrival records have 
 *	not yet been corrected, function, locate_event(), will make sure
 *	the SASCs are applied.  In this case, the static Ar_SASC structure,
 *	active_ar_sasc (local to this file), must be freed upon completion
 *	of event location (see bottom of locate_event.c).  This function is
 *	also recommended if new arrival records are introduced by a calling
 *	application/function.

 *	-- make_sasc_adj_before_locator() sets the internal static variable,
 *	make_sasc_adj_in_locator to FALSE if called; else it is initialized 
 *	to TRUE.  This is called when SASC adjustments are needed at the 
 *	start of a calling application so that they are not re-applied in 
 *	the event location process itself.  On the other hand, some 
 *	applications (e.g., ARS) use raw azimuth and slowness measures
 *	throughout their processing, but require SASC's be applied in the
 *	event location process alone.

 *	-- apply_sasc_adj_in_locator() informs the locator whether or not
 *	the SASC adjustments have already been applied.  If so, they should
 *	not be doublely applied.

 * DIAGNOSTICS
 *	-- read_sasc() will return an error code of ERR (-1) if a fatal
 *	error is encountered; else it will return OK (0).

 *	-- read_single_sasc() will return an error code of ERR (-1) if a
 *	fatal error is encountered; (1) is no SASC directory prefix is
 *	specified; (2) if no SASC info exists for input station; else it 
 *	will return OK (0).

 * FILES
 *	Function, read_sasc(), reads all station-specific SASC files.
 *	Function, read_single_sasc(), reads only a single SASC file.

 * NOTES
 *	None.

 * SEE ALSO
 *	None. 

 * AUTHOR
 *	Walter Nagy,  2/21/97,	Created.
 *	Walter Nagy,  3/10/97,	Added function, read_single_sasc().
 *	Walter Nagy,  8/26/99,	Added functionality employing full azimuth/
 *				slowness adjustments by employing a new affine
 *				transformation.
 *	Walter Nagy, 11/14/03,	Extended station/phase-dependent azimuth/slowness 
 *				capabilities, including SASCSs, in support of
 *				IDC CR P1 (AWST).  New functions, get_ar_sasc()
 *				and free_active_ar_sasc(), facilitated this
 *				upgrade.
 */


#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include "libloc.h"
#include "loc_defs.h"
#include "dyn_array.h"
#include "ar_sasc.h"
#include "arrival_Astructs.h"

typedef	struct	polar {
	double	slow;		/* Slowness measure (sec/deg) */
	double	az;		/* Azimuth measure (deg) */
} Polar;

typedef	struct	sasc {
	char	sta[7];		  /* Station name */
	double	def_sx_corr;	  /* Default x-vector slowness (sx) corr */
	double	def_sy_corr;	  /* Default y-vector slowness (sy) corr */
	double	def_slow_mdl_err; /* Default slowness modeling error */
	double	a11; 		  /* A11 x-vector affine slowness coefficient */
	double	a12; 		  /* A12 y-vector affine slowness coefficient */
	double	a21; 		  /* A21 x-vector affine slowness coefficient */
	double	a22; 		  /* A22 y-vector affine slowness coefficient */
	int	num_bins;	  /* Number of slowness/azimuth bin pairs */
	Polar	*bin_lb;	  /* Lower slowness/azimuth bound on bin */
	Polar	*bin_ub;	  /* Upper slowness/azimuth bound on bin */
	Polar	*corr;		  /* Binned slowness/azimuth correction */
	Polar	*mdl_err;	  /* Binned slowness/azimuth modeling error */
} SASC_Tables;


#define	NO_SASC_DIR_SPECIFIED	1
#define	NO_STA_FOUND		2

#define	READ_ERR(a)	{ fprintf (stderr, "\nread_sasc: Error reading %s in file: %s\n", (a), file_name); \
			}
#define	CALLOC_ERR(a)	{ fprintf (stderr, "\nread_sasc: Error allocating space for %s in file: %s\n", (a), file_name); \
			}


static	SASC_Tables	*sasc = (SASC_Tables *) NULL;
static	int		num_sta_w_sasc = 0;
static	Bool		make_sasc_adj_in_locator = TRUE;
static	Ar_SASC		*active_ar_sasc = (Ar_SASC *) NULL;
static	int		ar_sasc_cnt = 0;

int
#ifdef UsePrototypes
load_single_sasc (int i)
#else
load_single_sasc (i)
int	i;
#endif
{

	FILE	*sasc_fp;
	int	j;
	char	file_name[FILENAMELEN];
	char	sasc_file_name[FILENAMELEN];
	char	input_string[BUFSIZ];

	static	char	routine[] = "load_sasc_single";

	strcpy (sasc_file_name, (char *) sasc[i].corr);
	free(sasc[i].corr);
	sasc[i].corr     = (Polar *) NULL;
	sasc[i].num_bins = 0;

	if ((sasc_fp = fopen (sasc_file_name, "r")) == NULL)
	{
	    fprintf (stderr, "Error trying to open SASC file: %s\n", sasc_file_name);
	    exit (ERR);
	}

	/*
	 * First, read default vector slowness (sx,sy) corrections along
	 * along with the default slowness modeling error.  Also, read
	 * additional affine transform coefficients, if provided.  This
	 * source code is backward compatiable so that only the original
	 * 3 argument required as necessary.  Note: simply skip lines with 
	 * a '#' in the first column. 
	 */

	while (fgets (input_string, BUFSIZ, sasc_fp) != NULL)
	{
	    /* Skip lines w/ '#' or new-line ('\n') in 1st column */
	    if (strncmp (input_string, "#", 1) && strncmp (input_string, "\n", 1))
	    {
		if (sscanf (input_string, "%lf%lf%lf%lf%lf%lf%lf", 
		 	&sasc[i].def_sx_corr, &sasc[i].def_sy_corr,
			&sasc[i].def_slow_mdl_err, &sasc[i].a11, 
			&sasc[i].a12, &sasc[i].a21, &sasc[i].a22) < 4)
		{
		    sasc[i].a11 = 1.0;
		    sasc[i].a12 = 0.0;
		    sasc[i].a21 = 0.0;
		    sasc[i].a22 = 1.0;
		}
		break;
	    }
	}

	/*
	 * Next, read number of slowness/azimuth bins which contain
	 * corrections.  Again, simply skip lines with a '#' in the 
	 * first column.
	 */

	while (fgets (input_string, BUFSIZ, sasc_fp) != NULL)
	{
	    /* Skip lines w/ '#' or new-line ('\n') in 1st column */
	    if (strncmp (input_string, "#", 1) && strncmp (input_string, "\n", 1))
	    {
	        sscanf (input_string, "%d", &sasc[i].num_bins);
		break;
	    }
	}

	/*
	 * Use only default corrections and modeling errors if no user-
	 * specified bins are defined.
	 */

	if (sasc[i].num_bins == 0)
	{
	    fclose (sasc_fp);
	    return (OK);
	}

	/*
	 * Allocate memory for slowness/azimuth lower and upper bin 
	 * bounds, corrections and modeling errors within structure,
	 * sasc.
	 */

	if ((sasc[i].bin_lb = (Polar *) calloc (sasc[i].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.bin_lb");
	    exit (ERR);
	}
	if ((sasc[i].bin_ub = (Polar *) calloc (sasc[i].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.bin_ub");
	    exit (ERR);
	}
	if ((sasc[i].corr = (Polar *) calloc (sasc[i].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.corr");
	    exit (ERR);
	}
	if ((sasc[i].mdl_err = (Polar *) calloc (sasc[i].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.mdl_err");
	    exit (ERR);
	}

	/*
	 * Finally, read binned slowness/azimuth station corrections
	 * and modeling error organized in bins of min/max slowness
	 * (sec/deg; first two columns); min/max azimuth (deg; next
	 * two columns); slowness correction (sec/deg; 5th column); 
	 * azimuth correction (deg; 6th column); corrected slowness 
	 * modeling error (sec/deg; 7th column); and corrected
	 * azimuth modeling error (deg; 8th column).
	 */

	j = 0;
	while (fgets (input_string, BUFSIZ, sasc_fp) != NULL)
	{
	    /* Skip lines w/ '#' or new-line ('\n') in 1st column */
	    if (strncmp (input_string, "#", 1) && strncmp (input_string, "\n", 1))
	    {
		if (j >= sasc[i].num_bins)
		{
		    fprintf (stderr, "%s: Number of az/slow corr bins found is > specified in file: \n   %s\nThis will overrun memory, so bailing out!!!!\n", routine, sasc_file_name);
		    fclose (sasc_fp);
		    exit (ERR);
		}

		sscanf (input_string, "%lf%lf%lf%lf%lf%lf%lf%lf", 
			&sasc[i].bin_lb[j].slow, &sasc[i].bin_ub[j].slow,
			&sasc[i].bin_lb[j].az, &sasc[i].bin_ub[j].az,
			&sasc[i].corr[j].slow, &sasc[i].corr[j].az,
			&sasc[i].mdl_err[j].slow, &sasc[i].mdl_err[j].az);
		++j;
	    }
	}
	if (j != sasc[i].num_bins)
	{
	    fprintf (stderr, "%s: Number of az/slow corr bins found: %d\n           disagrees with the %d specified in file: \n          %s\nWill continue, but this should be fixed!!!!\n", routine, j, sasc[i].num_bins, sasc_file_name);
	}

	fclose (sasc_fp);

	return (OK);
}


int
#ifdef UsePrototypes
read_sasc (char *sasc_pathway)
#else
read_sasc (sasc_pathway)
char	*sasc_pathway;
#endif
{

        int     i;
	char	sasc_prefix[FILENAMELEN];
	char	file_name[FILENAMELEN];
	char	dir_pathway[FILENAMELEN];
	char	sasc_file_name[FILENAMELEN];

	Array	list_of_files;
	char	*ffp, *ptr;
	char	**ptr_to_files = (char **) NULL;

	DIR	*dirp;
	struct	dirent	*direntp;

	static	char	routine[] = "read_sasc";


	/*
	 * If no directory pathway and prefix is specified, then simply
	 * return.
	 */

	if (sasc_pathway == (char *) NULL ||
	    STREQ (sasc_pathway, "\0") || STREQ (sasc_pathway, "NULL"))
	{
	    fprintf (stderr, "Message: No SASC tables can be read since no directory/prefix is specified!\n");
	    return (OK);
	}

	/*
	 * Save list of all files found in each SASC directory.  Restrict 
	 * search to valid SASC files beginning with the prefix portion of
	 * input char string, sasc_pathway.
	 */

	strcpy (dir_pathway, sasc_pathway);
	if ((ffp = strrchr (dir_pathway, '/')) == NULL)
	{
	    fprintf (stderr, "Message: No SASC tables can be read since no / in SASC directory name!\n");
	    return (OK);
	}
	strcpy (sasc_prefix, ffp+1);
	strcat (sasc_prefix, ".");
	strcpy (ffp, "\0");

	if ((dirp = opendir (dir_pathway)) != NULL)
	{
	    list_of_files = array_create (sizeof (char *));
	    while ((direntp = readdir (dirp)) != NULL)
	    {
		/*
		 * Allocate local list_of_files array here.  Note that
		 * this array is only used in this function, then
		 * memory is freed.
		 */

		ptr = STRALLOCA (direntp->d_name);
		if (!strncmp (direntp->d_name, sasc_prefix, strlen (sasc_prefix)))
		    array_add (list_of_files, (caddr_t) &ptr);
	    }
	    (void) closedir (dirp);

	    ptr_to_files = (char **) array_list (list_of_files);
	    num_sta_w_sasc = array_count (list_of_files);
	    array_free (list_of_files);
	}
	else
	    return (OK);	/* No SASC information available */

	/*
	 * Allocate memory for slowness/azimuth correction tables in 
	 * structure, sasc.
	 */

	if ((sasc = (SASC_Tables *) 
			calloc (num_sta_w_sasc, sizeof (SASC_Tables))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc");
	    return (ERR);
	}

	/*
	 * Now process each file (set of azimuth/correction tables) on a
	 * station-by-station basis.
	 */

	for (i = 0; i < num_sta_w_sasc; i++)
	{
	    sasc[i].bin_lb   = (Polar *) NULL;
	    sasc[i].bin_ub   = (Polar *) NULL;
	    sasc[i].mdl_err  = (Polar *) NULL;
	    sasc[i].num_bins = -999;

	    strcpy (sasc_file_name, dir_pathway);
	    strcat (sasc_file_name, "/");
	    strcat (sasc_file_name, ptr_to_files[i]);

	    if ((ffp = strrchr (sasc_file_name, '.')) == NULL)
	    {
		fprintf (stderr, "%s: No . in SASC file name!\n", routine);
		return (ERR);
	    }
	    strcpy (sasc[i].sta, ffp+1);
	    sasc[i].corr = (Polar *) strdup(sasc_file_name);
		    }

	    /*
	 * Lastly, free temporary filename arrays!
	 */

	UFREE (ptr_to_files);

	return (OK);
}


int
#ifdef UsePrototypes
read_single_sasc (char *sasc_pathway, char *sta)
#else
read_single_sasc (sasc_pathway, sta)
char	*sasc_pathway;
char	*sta;
#endif
{

	FILE	*sasc_fp;
	int	j;
	char	sasc_file_name[FILENAMELEN];
	char	file_name[FILENAMELEN];
	char	input_string[BUFSIZ];

	static	char	routine[] = "read_single_sasc";


	/*
	 * If no directory pathway and prefix is specified, then simply
	 * return.
	 */

	if (sasc_pathway == (char *) NULL ||
	    STREQ (sasc_pathway, "\0") || STREQ (sasc_pathway, "NULL"))
	{
	    fprintf (stderr, "Message: No SASC table can be read since no directory/prefix is specified!\n");
	    return (NO_SASC_DIR_SPECIFIED);
	}

	strcpy (sasc_file_name, sasc_pathway);
	strcat (sasc_file_name, ".");
	strcat (sasc_file_name, sta);

	if ((sasc_fp = fopen (sasc_file_name, "r")) == NULL)
	{
	    fprintf (stderr, "Message: No SASC file exists for sta: %s\n", sta);
	    return (NO_STA_FOUND);
	}

	/*
	 * If num_sta_w_sasc > 0 we know SASC table has already been read,
	 * so free previous memory.
	 */

	if (num_sta_w_sasc != 0)
	{
	    UFREE (sasc[0].bin_lb);
	    UFREE (sasc[0].bin_ub);
	    UFREE (sasc[0].corr);
	    UFREE (sasc[0].mdl_err);
	    UFREE (sasc);
	}

	num_sta_w_sasc = 1;

	/*
	 * Allocate memory for slowness/azimuth correction table in 
	 * structure, sasc.
	 */

	if ((sasc = (SASC_Tables *) 
			calloc (1, sizeof (SASC_Tables))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc");
	    return (ERR);
	}

	/*
	 * Now process single SASC file (set of azimuth/correction tables)
	 */

	sasc[0].bin_lb   = (Polar *) NULL;
	sasc[0].bin_ub   = (Polar *) NULL;
	sasc[0].corr     = (Polar *) NULL;
	sasc[0].mdl_err  = (Polar *) NULL;
	sasc[0].num_bins = 0;
	strcpy (sasc[0].sta, sta);

	/*
	 * First, read default vector slowness (sx,sy) corrections along
	 * along with the default slowness modeling error.  Also, read
	 * additional affine transform coefficients, if provided.  This
	 * source code is backward compatiable so that only the original
	 * 3 argument required as necessary.  Note: simply skip lines with 
	 * a '#' in the first column. 
	 */

	while (fgets (input_string, BUFSIZ, sasc_fp) != NULL)
	{
	    /* Skip lines w/ '#' or new-line ('\n') in 1st column */
	    if (strncmp (input_string, "#", 1) &&
	        strncmp (input_string, "\n", 1))
	    {
		if (sscanf (input_string, "%lf%lf%lf%lf%lf%lf%lf", 
			    &sasc[0].def_sx_corr, &sasc[0].def_sy_corr,
			    &sasc[0].def_slow_mdl_err, &sasc[0].a11, 
			    &sasc[0].a12, &sasc[0].a21, &sasc[0].a22) < 4)
		{
		    sasc[0].a11 = 1.0;
		    sasc[0].a12 = 0.0;
		    sasc[0].a21 = 0.0;
		    sasc[0].a22 = 1.0;
		}
		break;
	    }
	}

	/*
	 * Next, read number of slowness/azimuth bins which contain
	 * corrections.  Again, simply skip lines with a '#' in the 
	 * first column.
	 */

	while (fgets (input_string, BUFSIZ, sasc_fp) != NULL)
	{
	    /* Skip lines w/ '#' or new-line ('\n') in 1st column */
	    if (strncmp (input_string, "#", 1) &&
		strncmp (input_string, "\n", 1))
	    {
		sscanf (input_string, "%d", &sasc[0].num_bins);
		break;
	    }
	}

	/*
	 * Use only default corrections and modeling errors if no user-
	 * specified bins are defined.
	 */

	if (sasc[0].num_bins == 0)
	{
	    fclose (sasc_fp);
	    return (OK);
	}

	/*
	 * Allocate memory for slowness/azimuth lower and upper bin 
	 * bounds, corrections and modeling errors within structure,
	 * sasc.
	 */

	if ((sasc[0].bin_lb = (Polar *) 
			calloc (sasc[0].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.bin_lb");
	    return (ERR);
	}
	if ((sasc[0].bin_ub = (Polar *) 
			calloc (sasc[0].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.bin_ub");
	    return (ERR);
	}
	if ((sasc[0].corr = (Polar *) 
			calloc (sasc[0].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.corr");
	    return (ERR);
	}
	if ((sasc[0].mdl_err = (Polar *) 
			calloc (sasc[0].num_bins, sizeof (Polar))) == NULL)
	{
	    CALLOC_ERR ("structure, sasc.mdl_err");
	    return (ERR);
	}

	/*
	 * Finally, read binned slowness/azimuth station corrections
	 * and modeling error organized in bins of min/max slowness
	 * (sec/deg; first two columns); min/max azimuth (deg; next
	 * two columns); slowness correction (sec/deg; 5th column); 
	 * azimuth correction (deg; 6th column); corrected slowness 
	 * modeling error (sec/deg; 7th column); and corrected
	 * azimuth modeling error (deg; 8th column).
	 */

	j = 0;
	while (fgets (input_string, BUFSIZ, sasc_fp) != NULL)
	{
	    /* Skip lines w/ '#' or new-line ('\n') in 1st column */
	    if (strncmp (input_string, "#", 1) &&
		strncmp (input_string, "\n", 1))
	    {
		if (j >= sasc[0].num_bins)
		{
		    fprintf (stderr, "%s: Number of az/slow corr bins found is > specified in file: \n   %s\nThis will overrun memory, so bailing out!!!!\n", routine, sasc_file_name);
		    fclose (sasc_fp);
		    return (ERR);
		}

		sscanf (input_string, "%lf%lf%lf%lf%lf%lf%lf%lf", 
				&sasc[0].bin_lb[j].slow, &sasc[0].bin_ub[j].slow,
				&sasc[0].bin_lb[j].az, &sasc[0].bin_ub[j].az,
				&sasc[0].corr[j].slow, &sasc[0].corr[j].az,
				&sasc[0].mdl_err[j].slow, &sasc[0].mdl_err[j].az);
		++j;
	    }
	}
	if (j != sasc[0].num_bins)
	{
	    fprintf (stderr, "%s: Number of az/slow corr bins found: %d\n           disagrees with the %d specified in file: \n          %s\nWill continue, but this should be fixed!!!!\n", routine, j, sasc[0].num_bins, sasc_file_name);
	}

	fclose (sasc_fp);

	return (OK);
}


void
#ifdef UsePrototypes
correct_az_slow (int arid, char *sta, double *azimuth, double *slow,
		 double delaz, double delslo, double *tot_az_err, 
		 double *tot_slow_err)
#else
correct_az_slow (arid, sta, azimuth, slow, delaz, delslo, 
		 tot_az_err, tot_slow_err)

int	arid;		/* Unique arrival ID */
char	*sta;		/* Station name */
double	*azimuth;	/* Raw azimuth on input; Corrected azimuth on output */
double	*slow;		/* Raw slowness on input; Corrected slowness on output */
double	delaz;		/* Azimuth measurement error as determined by DFX */
double	delslo;		/* Slowness measurement error as determined by DFX */
double	*tot_az_err;	/* Total azimuth error */
double	*tot_slow_err;	/* Total slowness error */
#endif
{
	int	i;
	int	sta_index = -1;
	Bool	station_found = FALSE;
	double	azr, sx, sy, adj_sx, adj_sy;

	Ar_SASC	ar_sasc;


	/*
	 * Initialize ar_sasc structure.
	 */

	strcpy (ar_sasc.sta, sta);
	ar_sasc.arid		= arid;
	ar_sasc.azimuth		= *azimuth;
	ar_sasc.slow		= *slow;
	ar_sasc.raw_azimuth	= *azimuth;
	ar_sasc.raw_slow	= *slow;
	ar_sasc.delaz		= delaz;
	ar_sasc.delslo		= delslo;
	ar_sasc.tot_az_err	= delaz;
	ar_sasc.tot_slow_err	= delslo;
	ar_sasc.azimuth_corr	= 0.0;
	ar_sasc.slow_corr	= 0.0;
	ar_sasc.azimuth_mdl_err	= -1.0;
	ar_sasc.slow_mdl_err	= -1.0;

	*tot_az_err = delaz;
	*tot_slow_err = delslo;

	/*
	 * If no valid azimuth or slow, simply return
	 */

	if (*azimuth < 0.0 && *slow < 0.0)
	    return;

	if (delaz < 0.0 && delslo < 0.0)
	    return;

	/*
	 * The input delaz and delslo fields assumes only measurement error
	 * is inlcuded.  This is done in DFX.  Then, determine if any SASC 
	 * tables exist.  If not, your done!  If so, determine if any SASC 
	 * table exists for the given input station.
	 */

	if (num_sta_w_sasc == 0)
	    return;		/* No SASC tables exist */

	for (sta_index = 0; sta_index < num_sta_w_sasc; sta_index++)
	{
	    if (STREQ (sta, sasc[sta_index].sta))
	    {
		station_found = TRUE;

		/* check if SASC was loaded into memory */

		if (sasc[sta_index].num_bins == -999)
		    load_single_sasc(sta_index);

		break;
	    }
	}
	if (! station_found)
	    return;		/* No SASC table exists for this station */

	/*
	 * Now that we know an SASC table exists for this station, let's
	 * first convert default vector slowness corrections to a localized
	 * slowness/azimuth correction.  Also store slowness modeling error.
	 */

	ar_sasc.slow_mdl_err	= sasc[sta_index].def_slow_mdl_err;

	/*
	 * Determine default modeling error for azimuth as a function of
	 * the default slowness modeling error.
	 */

	ar_sasc.azimuth_mdl_err = ar_sasc.slow_mdl_err / (2.0 * ar_sasc.slow);
	if (ar_sasc.azimuth_mdl_err < 1.0)
	    ar_sasc.azimuth_mdl_err = 
		2.0 * asin (ar_sasc.azimuth_mdl_err) * RAD_TO_DEG;
	else
	    ar_sasc.azimuth_mdl_err = 180.0;

	/*
	 * Loop over all bins for this station to determine if there is a 
	 * bin corrected slowness/azimuth correction that needs to be applied.
	 * Also obtain bin-dependent modeling error.
	 */

	for (i = 0; i < sasc[sta_index].num_bins; i++)
	{
	    if (ar_sasc.slow < sasc[sta_index].bin_ub[i].slow &&
		ar_sasc.slow >= sasc[sta_index].bin_lb[i].slow &&
		ar_sasc.azimuth < sasc[sta_index].bin_ub[i].az &&
		ar_sasc.azimuth >= sasc[sta_index].bin_lb[i].az)
	    {
		ar_sasc.azimuth_mdl_err = sasc[sta_index].mdl_err[i].az;
		ar_sasc.slow_mdl_err = sasc[sta_index].mdl_err[i].slow;
		ar_sasc.azimuth_corr = sasc[sta_index].corr[i].az;
		ar_sasc.slow_corr = sasc[sta_index].corr[i].slow;

		/* 
		 * Update azimuth and slowness with corrected values. 
		 */

		ar_sasc.azimuth -= ar_sasc.azimuth_corr;
		ar_sasc.slow -= ar_sasc.slow_corr;
		if (ar_sasc.azimuth < 0.0)
		    ar_sasc.azimuth += 360.0;
		if (ar_sasc.azimuth > 360.0)
		    ar_sasc.azimuth -= 360.0;

		break;
	    }
	}

	/*
	 * Apply affine and default slowness vector corrections here.  First 
	 * decompose the original azimuth and slowness into vector slowness 
	 * componenets (sx, sy), and then, apply affine transform.  Then 
	 * apply default slowness vector corrections in x- and y-directions.
	 * Finally, adjust input azimuth and slow based on these updated
	 * slowness vector component adjustments.

	 * Affine transform: corrected_sx = (a11*sx + a12*sy) - def_sx_corr
	 *                   corrected_sy = (a21*sx + a22*sy) - def_sy_corr
	 * where,
	 *	sx and sy have already been bin corrected.
	 */

	if (ar_sasc.slow > 0.0)
	  {
	    azr = ar_sasc.azimuth*DEG_TO_RAD;
	    sx = ar_sasc.slow * sin (azr);
	    sy = ar_sasc.slow * cos (azr);
	    adj_sx = sasc[sta_index].a11*sx + sasc[sta_index].a12*sy;
	    adj_sy = sasc[sta_index].a21*sx + sasc[sta_index].a22*sy;
	    sx = adj_sx;
	    sy = adj_sy;

	/*
	 * Apply default sx and sy corrections here to get the total affine
	 * transformed slowness vector positiions.
	 */

	    sx -= sasc[sta_index].def_sx_corr; /* Apply default sx corr. here */
	    sy -= sasc[sta_index].def_sy_corr; /* Apply default sy corr. here */

	/*
	 * Revert back to azimuth/slowness space
	 */

	ar_sasc.azimuth = atan2 (sx, sy) * RAD_TO_DEG;
	if (ar_sasc.azimuth < 0.0)
	    ar_sasc.azimuth += 360.0;
	ar_sasc.slow = hypot (sx, sy);

	  }
	/*
	 * Store total azimuth/slowness corrections in ar_sasc structure
	 */

	ar_sasc.azimuth_corr = ar_sasc.raw_azimuth - ar_sasc.azimuth;
	if (ar_sasc.azimuth_corr > 180.0)
	    ar_sasc.azimuth_corr = SIGN( (360.0-fabs(ar_sasc.azimuth_corr)),
						     ar_sasc.azimuth_corr );
	ar_sasc.slow_corr = ar_sasc.raw_slow - ar_sasc.slow;

	*azimuth = ar_sasc.azimuth;
	*slow = ar_sasc.slow;

	/*
	 * Total azimuth and slowness errors are an RMS measure of the
	 * combined measurement error (delaz and delslo) and the modeling
	 * error (ar_sasc.azimuth_mdl_err and ar_sasc.slow_mdl_err).
	 * The inverse of the total errors for azimuth and slowness are
	 * employed as weights in the event location process.
	 */

	if (ar_sasc.azimuth_mdl_err < 180.0)
	    *tot_az_err = 
		delaz*delaz + ar_sasc.azimuth_mdl_err*ar_sasc.azimuth_mdl_err;
	else
	    *tot_az_err = 180.0;

	*tot_slow_err = 
	delslo*delslo + ar_sasc.slow_mdl_err*ar_sasc.slow_mdl_err;

	ar_sasc.tot_az_err = sqrt (*tot_az_err);
	ar_sasc.tot_slow_err = sqrt (*tot_slow_err);
	*tot_az_err = ar_sasc.tot_az_err;
	*tot_slow_err = ar_sasc.tot_slow_err;

	if ((active_ar_sasc = 
		(Ar_SASC *) realloc (active_ar_sasc, (ar_sasc_cnt+1) * sizeof (Ar_SASC))) == (Ar_SASC *) NULL)
	{
	    fprintf (stderr, "Memory reallocation failure in correct_az_slow()\n");
	    return;
	}
	MCOPY (&active_ar_sasc[ar_sasc_cnt], &ar_sasc, sizeof (Ar_SASC));
	ar_sasc_cnt++;

/*
	if (num_active_ar_sasc != 0)
	{
	    if (ar_sasc_cnt > num_active_ar_sasc)
		fprintf (stderr, "Error: Incorrect implementation of SASC functionality!!\n  correct_az_slow() attempting to write beyond space allocated for active_ar_sasc[]\n");
	    MCOPY (&active_ar_sasc[ar_sasc_cnt], &ar_sasc, sizeof (Ar_SASC));
	    ar_sasc_cnt++;
	}
 */

	/*
	fprintf (stdout, "\nStation: %-6s\n", sta);
	fprintf (stdout, "Orig Az/Sl:%7.2f/%5.2f  Adj Az/Sl:%7.2f/%5.2f  Az/Sl Corr:%7.2f/%5.2f\n",
		 ar_sasc.raw_azimuth, ar_sasc.raw_slow, ar_sasc.azimuth,
		 ar_sasc.slow, ar_sasc.azimuth_corr, ar_sasc.slow_corr);
	fprintf (stdout, "Meas Az/Sl:%7.2f/%5.2f  Mdl Az/Sl:%7.2f/%5.2f  Tot Az/Sl:%7.2f/%5.2f\n",
		 ar_sasc.delaz, ar_sasc.delslo, ar_sasc.azimuth_mdl_err,
		 ar_sasc.slow_mdl_err, *tot_az_err, *tot_slow_err);
	 */

	return;
}


/*
int
#ifdef UsePrototypes
ar_sasc_alloc (int num_ar_sascs)
#else
ar_sasc_alloc (num_ar_sascs)
int	num_ar_sascs;
#endif
{
	if (active_ar_sasc != (Ar_SASC *) NULL || num_active_ar_sasc != 0)
	{
	    ar_sasc_cnt = 0;
	    UFREE (active_ar_sasc);
	    active_ar_sasc = (Ar_SASC *) NULL;
	}

	if ((active_ar_sasc = (Ar_SASC *) 
			calloc (num_ar_sasc, sizeof (Ar_SASC))) == NULL)
	{
	    CALLOC_ERR ("structure, active_ar_sasc");
	    return (ERR);
	}
	num_active_ar_sasc = num_ar_sascs;

	return (OK);
}
*/


void
#ifdef UsePrototypes
free_active_ar_sasc ()
#else
free_active_ar_sasc ()
#endif
{
	ar_sasc_cnt = 0;	/* Reset ar_sasc counter */
	UFREE (active_ar_sasc);
	active_ar_sasc = (Ar_SASC *) NULL;
}


Ar_SASC
#ifdef UsePrototypes
get_ar_sasc (int arid)
#else
get_ar_sasc (arid)
int     arid;
#endif
{
         int     i;
	 Arrival Na_Arrival = Na_Arrival_Init;
         Ar_SASC ar_sasc;

         ar_sasc.arid = Na_Arrival.arid;

         for (i = 0; i < ar_sasc_cnt; i++)
             if (active_ar_sasc[i].arid == arid)
                 return (active_ar_sasc[i]);

         return (ar_sasc);
}


void
make_sasc_adj_before_locator ()
{
	make_sasc_adj_in_locator = FALSE;
	return;
}


Bool
apply_sasc_adj_in_locator ()
{
	return (make_sasc_adj_in_locator);
}


