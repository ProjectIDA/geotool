
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * NAME
 *	read_tl_table -- Read a single transmission loss (TL) table
 
 * FILE
 *	read_tl_table.c

 * SYNOPSIS
 *	int
 *	read_tl_table (dir_pathway, TLtype, tl_model, phase, chan, tl_table_ptr)
 *	char	 *dir_pathway		(i) Directory pathway for current 
 *					    desired TL table
 *	char	 *TLtype		(i) Desired TL type 
 *	char	 *tl_model		(i) Desired TL model name 
 *	char	 *phase			(i) Phase type (not required dependency)
 *	char	 *chan			(i) Chan type (not required dependency)
 *	TL_Table *tl_table_ptr;		(o) Pointer to TL table struct

 * DESCRIPTION
 *	Function.  Read a single distance/depth-dependent transmission loss 
 *	(TL) table and return a tl_table structure with its contents.  The
 *	first implementation for TL tables is for reading magnitude correction 
 *	tables.  Yet, this function can be applied to other, more general, 
 *	kinds of transmission loss information (e.g., raw attenuation curves).

 * DIAGNOSTICS
 *	Return argument will inform user about the following errors encountered
 *	in this routine (see include file, tl_defs.h, for error code define 
 *	statements):
 *	    TLreadErr4:	TL table incorrectly formatted.
 *	    TLreadErr5:	TL modelling error table incorrectly formatted.
 *	    TLreadErr6:	TL test-site corr. file incorrectly formatted.
 *	    TLreadErr7:	Error allocating memory while reading TL info.
 *	    TLreadWarn1:Requested single TL file not found.

 * FILES
 *	Read TL and associated modelling error tables.  Also read test-site
 *	correction information, if available.

 * NOTES
 *	If input file will not open, then the structure, tl_table, will be 
 *	set to NULL. 
 
 * SEE ALSO
 *	None. 

 * AUTHOR
 *	Walter Nagy, 8/21/97,	Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libmagnitude.h"
#include "tl_defs.h"


/* Error report */
#define READ_ERR(a)	{ fprintf (stderr, "\nread_tl_table: Error reading %s in file: %s\n", (a), file_name); \
			}
#define CALLOC_ERR(a)	{ fprintf (stderr, "\nread_tl_table: Error allocating space for %s, in file: %s\n", (a), file_name); \
			}

#define	VALID_TL(x)	((x) > -1.0)


int
#ifdef __STDC___
read_tl_table (char *dir_pathway, char *TLtype, char *tl_model, char *phase,
	       char *chan, TL_Table **tl_table_ptr) 
#else
read_tl_table (char *dir_pathway, char *TLtype, char *tl_model, char *phase,
char *chan,TL_Table **tl_table_ptr) 

     /*char	 *dir_pathway;*/
     /*char	 *TLtype;*/
     /*char	 *tl_model;*/
     /*char	 *phase;*/
     /*char	 *chan;*/
     /*TL_Table **tl_table_ptr;*/
#endif

{
	FILE	*tl_fp, *ts_fp;
	Bool	ok_so_far;
	int	i, j, k;	
	int	ntbd, ntbz;
	int	sta_len;
	char	file_name[FILENAMELEN];
	char	file_buf[FILENAMELEN];
	char	tmp[128];

	TL_Table	*tl_table = (TL_Table *) NULL;
	TL_TS_Cor	*tsc = (TL_TS_Cor *) NULL;

	k = 0;
	tl_table = *tl_table_ptr;

	/*
	 * Build filename from desired variable elements, tl_model, TLtype,
	 * phase and chan.  The latter two arguments are not explicitly
	 * required, yet may be used to extend any phase- and channel-
	 * dependencies, respectively.  A phase-dependency can exist w/o
	 * a chan dependency, but not the other way around.
	 */

	strcpy (file_name, dir_pathway);
	strcat (file_name, "/");
	strcat (file_name, tl_model);
	strcat (file_name, ".");
	strcat (file_name, TLtype);
	if (phase != (char *) NULL && strcmp (phase, "-"))
	{
	    strcat (file_name, ".");
	    strcat (file_name, phase);
	    if (chan != (char *) NULL && strcmp (chan, "-"))
	    {
		strcat (file_name, ".");
		strcat (file_name, chan);
	    }
	}

	/* 
	 * Open transmission loss (TL) file given input tl_filename
	 */

	if ((tl_fp = fopen (file_name, "r")) == NULL)
	{
	    fprintf (stderr, "\nWarning: File %s will not open!\n", file_name);
	    return (TLreadWarn1);
	}

	/*
	 * Allocate a single tl_table structure
	 */

	if ((tl_table = (TL_Table *) calloc (1, sizeof (TL_Table))) == NULL)
	{
	    CALLOC_ERR ("structure, tl_table");
	    return (TLreadErr7);
	}

	/* Initialize tl_table structure for this TLtype */

	strcpy (tl_table->TLtype, TLtype);
	strcpy (tl_table->model, tl_model);
	strcpy (tl_table->phase, phase);
	strcpy (tl_table->chan, chan);
	tl_table->num_dists = 0;
	tl_table->num_depths = 0;
	tl_table->in_hole_dist[0] = 181.0;
	tl_table->in_hole_dist[1] = -1.0;
	tl_table->dist_samples = (float *) NULL;
	tl_table->depth_samples = (float *) NULL;
	tl_table->tl = (float **) NULL;
	tl_table->tl_mdl_err = (TL_Mdl_Err *) NULL;
	tl_table->num_ts_regions = 0;
	tl_table->tl_ts_cor = (TL_TS_Cor *) NULL;


	/* 
	 * Begin reading TL info.  Just skip first comment line, then read
	 * number of depth samples available.
	 */

	if (fgets (file_buf, FILENAMELEN, tl_fp) == NULL) return (TLreadErr4);
	if (fscanf (tl_fp, "%d%*[^\n]", &ntbz) != 1)
	{
	    READ_ERR("number of depth samples");
	    fclose (tl_fp);
	    return (TLreadErr4);
	}
	tl_table->num_depths = ntbz;

	if ((tl_table->depth_samples = 
			(float *) calloc (ntbz, sizeof (float))) == NULL)
	{
	    CALLOC_ERR ("tl_table->depth_samples");
	    fclose (tl_fp);
	    return (TLreadErr7);
	}

	for (i = 0; i < ntbz; i++)
	{
	    if (fscanf (tl_fp, "%f", &tl_table->depth_samples[i]) != 1)
	    {
		READ_ERR("depth sample value");
		fclose (tl_fp);
		return (TLreadErr4);
	    }
	}

	/* 
	 * Next, read distance sampling info
	 */

	if (fscanf (tl_fp, "%d%*[^\n]", &ntbd) != 1)
	{
	    READ_ERR("number of distance samples");
	    fclose (tl_fp);
	    return (TLreadErr4);
	}
	tl_table->num_dists = ntbd;

	if ((tl_table->dist_samples = 
		(float *) calloc (ntbd, sizeof (float))) == NULL)
	{
	    CALLOC_ERR ("tl_table->dist_samples");
	    fclose (tl_fp);
	    return (TLreadErr7);
	}

	for (i = 0; i < ntbd; i++)
	{
	    if (fscanf (tl_fp, "%f", &tl_table->dist_samples[i]) != 1)
	    {
		READ_ERR("distance sample value");
		fclose (tl_fp);
		return (TLreadErr4);
	    }
	}

	/* 
	 * Now read actual transmission loss (TL) tables 
	 */
 
	if ((tl_table->tl = (float **) calloc (ntbz, sizeof (float *))) == NULL)
	{
	    CALLOC_ERR ("tl_table->tl");
	    fclose (tl_fp);
	    return (TLreadErr7);
	}
	for (i = 0; i < ntbz; i++)
	{
	    if ((tl_table->tl[i] = 
			(float *) calloc (ntbd, sizeof (float))) == NULL)
	    {
		CALLOC_ERR ("tl_table->tl[]");
		fclose (tl_fp);
		return (TLreadErr7);
	    }
	}

	for (j = 0; j < ntbz; j++)
	{
	    /* skip the comment line */
	    while (getc(tl_fp) != '#');
	    while (getc(tl_fp) != '\n');
	    for (i = 0; i < ntbd; i++)
	    {
		if (fscanf (tl_fp,"%f", &tl_table->tl[j][i]) != 1)
		{
		    READ_ERR("transmission-loss value");
		    fclose (tl_fp);
		    return (TLreadErr4);
		}
	    }
	}

	/*
	 * Look for any holes in TL table.  If one is discovered, then set
	 * min/max distances.  Only inspect 1st depth node.  Only meaningful 
	 * if both in_hole_dist[0] and in_hole_dist[1] are set here.
	 */

	for (i = 1, ok_so_far = TRUE; i < ntbd; i++)
	{
	    if (VALID_TL(tl_table->tl[0][i-1]) && 
	        ! VALID_TL(tl_table->tl[0][i]))
	    {
		tl_table->in_hole_dist[0] = tl_table->dist_samples[i-1];
		ok_so_far = FALSE;
	    }
	    else if (!ok_so_far && VALID_TL(tl_table->tl[0][i]))
	    {
		tl_table->in_hole_dist[1] = tl_table->dist_samples[i];
		break;
	    }
	}

	/*
	 * Next read data modelling information!  This can take the form of
	 * a single-value, a distance-dependent set of values and distance/
	 * depth-dependent values.  Each is optimized for fastest access.
	 * The first thing we need to do is verify the existence of such 
	 * information within the TL tables.  If an EOF has been reached, 
	 * then simply set a NULL pointer for the tl_mdl_err structure 
	 * hanging off of the tl_table structure.  In this case, a default 
	 * modelling error of 1.0 magnitude unit will be employed.  Else 
	 * this part of the table will contain a comment line instead of 
	 * an EOF.  A comment line must exist betwen distance and/or depth 
	 * sample specification and modelling errors themselves.  Additional 
	 * comment lines exist between separate sets of modelling errors for 
	 * different depths (note that this latter case is only valid when
	 * dist_depth_var needs to be filled.  i.e., when ntbd > 0)
	 */

	if (fscanf (tl_fp, "%s%*[^\n]", file_buf) != EOF)
	{
	  if (fscanf (tl_fp, "%d%d%*[^\n]", &ntbd, &ntbz) != EOF)
	  {
	    if ((tl_table->tl_mdl_err = (TL_Mdl_Err *) 
				calloc (1, sizeof (TL_Mdl_Err))) == NULL)
	    {
		CALLOC_ERR ("tl_table->tl_mdl_err");
		fclose (tl_fp);
		return (TLreadErr7);
	    }

	    tl_table->tl_mdl_err->bulk_var       = 1.0;
	    tl_table->tl_mdl_err->num_dists      = ntbd;
	    tl_table->tl_mdl_err->num_depths     = ntbz;
	    tl_table->tl_mdl_err->dist_samples   = (float *) NULL;
	    tl_table->tl_mdl_err->depth_samples  = (float *) NULL;
	    tl_table->tl_mdl_err->dist_var       = (float *) NULL;
	    tl_table->tl_mdl_err->dist_depth_var = (float **) NULL;

	    if (ntbz == 1)
	    {
		if (ntbd == 1)
		{
		    /*
		     * Only a single modelling error is available, so we only
		     * need to fill variable, bulk_var.  Don't forget to skip 
		     * comment line!
		     */

		    if (fscanf (tl_fp, "%s%*[^\n]", tmp) == EOF)
		    {
			READ_ERR("premature EOF");
			fclose (tl_fp);
			return (TLreadErr5);
		    }
		    if (fscanf (tl_fp, "%f%*[^\n]", 
				   &tl_table->tl_mdl_err->bulk_var) != 1)
		    {
			READ_ERR("single bulk modelling error");
			fclose (tl_fp);
			return (TLreadErr5);
		    }
		}
		else
		{
		    /*
		     * Only distance-dependent modelling errors exist, so
		     * allocate only enough memory for single-dimensioned 
		     * arrays, dist_samples && dist_var.
		     */

		    if ((tl_table->tl_mdl_err->dist_samples = 
			(float *) calloc (ntbd, sizeof (float))) == NULL)
		    {
			CALLOC_ERR ("tl_table->tl_mdl_err->dist_samples");
			fclose (tl_fp);
			return (TLreadErr7);
		    }
		    if ((tl_table->tl_mdl_err->dist_var = 
			(float *) calloc (ntbd, sizeof (float))) == NULL)
		    {
			CALLOC_ERR ("tl_table->tl_mdl_err->dist_var");
			fclose (tl_fp);
			return (TLreadErr7);
		    }

		    /*
		     * Read distance-dependent variance information.
		     */

		    for (i = 0; i < ntbd; i++)
		    {
			if (fscanf (tl_fp, "%f", &tl_table->tl_mdl_err->dist_samples[i]) == EOF)
			{
			    READ_ERR("modelling error distance sample value");
			    fclose (tl_fp);
			    return (TLreadErr5);
			}
		    }
		    if (fscanf (tl_fp, "%s%*[^\n]", tmp) == EOF)
		    {
			READ_ERR("premature EOF");
			fclose (tl_fp);
			return (TLreadErr5);
		    }
		    for (i = 0; i < ntbd; i++)
		    {
			if (fscanf (tl_fp, "%f", &tl_table->tl_mdl_err->dist_var[i]) == EOF)
			{
			    READ_ERR("distance-dependent modelling error");
			    fclose (tl_fp);
			    return (TLreadErr5);
			}
		    }
		}
	    }
	    else
	    {
		/*
		 * Modelling errors are distance and depth dependent here.
		 * Allocate memory for dist_samples, depth_samples and 
		 * dist_depth_var.  Note, you don't need to set array 
		 * element, dist_var, in this case.
		 */

		if ((tl_table->tl_mdl_err->dist_samples = 
			(float *) calloc (ntbd, sizeof (float))) == NULL)
		{
		    CALLOC_ERR ("tl_table->tl_mdl_err->dist_samples");
		    fclose (tl_fp);
		    return (TLreadErr7);
		}
		if ((tl_table->tl_mdl_err->depth_samples = 
			(float *) calloc (ntbz, sizeof (float))) == NULL)
		{
		    CALLOC_ERR ("tl_table->tl_mdl_err->depth_samples");
		    fclose (tl_fp);
		    return (TLreadErr7);
		}
		if ((tl_table->tl_mdl_err->dist_depth_var = 
			(float **) calloc (ntbz, sizeof (float *))) == NULL)
		{
		    CALLOC_ERR ("tl_table->tl_mdl_err->dist_depth_var");
		    fclose (tl_fp);
		    return (TLreadErr7);
		}
		for (i = 0; i < ntbz; i++)
		{
		    if ((tl_table->tl_mdl_err->dist_depth_var[i] = 
			      (float *) calloc (ntbd, sizeof (float))) == NULL)
		    {
			CALLOC_ERR ("tl_table->tl_mdl_err->dist_depth_var[]");
			fclose (tl_fp);
			return (TLreadErr7);
		    }
		}

		/*
		 * Read distance and depth sampling info 
		 */

		for (i = 0; i < ntbd; i++)
		{
		    if (fscanf (tl_fp, "%f", &tl_table->tl_mdl_err->dist_samples[i]) == EOF)
		    {
			READ_ERR("modelling error distance sample value");
			fclose (tl_fp);
			return (TLreadErr5);
		    }
		}
		for (i = 0; i < ntbz; i++)
		{
		    if (fscanf (tl_fp, "%f", &tl_table->tl_mdl_err->depth_samples[i]) == EOF)
		    {
			READ_ERR("modelling error depth sample value");
			fclose (tl_fp);
			return (TLreadErr5);
		    }
		}

		/*
		 * Finally, read the actual distance/depth-dependent
		 * modelling errors.
		 */

		for (j = 0; j < ntbz; j++)
		{
		    /* skip the comment line */
		    while (getc(tl_fp) != '#');
		    while (getc(tl_fp) != '\n');
		    for (i = 0; i < ntbd; i++)
		    {
			if (fscanf (tl_fp,"%f", &tl_table->tl_mdl_err->dist_depth_var[j][i]) == EOF)
			{
			    READ_ERR("distance/depth modelling error");
			    fclose (tl_fp);
			    return (TLreadErr5);
			}
		    }
		}
	    }
	  }
	}
	fclose (tl_fp);

	/*
	 * Finally, is there a directory pointer for test-site
	 * corrections associated with this TL table ?  If so,
	 * read the contents here.  A file directory pointer must
	 * specify precise location of test-site correction info.
	 * This file must follow the file naming convention, 
	 * "file_name.ts_dir", where file_name is the current
	 * directory pathway file name of the TL table currently
	 * being accessed.  This file (e.g., 'gr.mb.ts_dir') must
	 * contain the directory pathway and file name which stores
	 * the actual test-site correction contents.
	 */

	strcat (file_name, ".ts_dir");
	if ((ts_fp = fopen (file_name, "r")) != NULL)
	{
	    /* 
	     * Read single line of this file to determine full 
	     * pathway and filename of test-site correction info.
	     */

	  if (fscanf (ts_fp, "%s%*[^\n]", file_buf) != 1) return (TLreadErr5);
	    fclose (ts_fp);	/* Close to re-use pointer */
	    if ((ts_fp = fopen (file_buf, "r")) == NULL)
	    {
		fprintf (stderr, "\nTest-site correction file: %s\n specified in file: %s\n does NOT exist!\n", file_buf, file_name);
		fprintf (stderr, "\nNO test-site corrections can be applied for this TL model\n");
	    }
	    else
	    {
		/*
		 * How many test-site regions are there for this
		 * TL type (TLtype).  With this info,
		 * allocate memory for tl_table[].ts_cor.
		 */

	      if (fscanf (ts_fp, "%d%*[^\n]",&tl_table[k].num_ts_regions) != 1) return (TLreadErr7);
		if ((tl_table[k].tl_ts_cor = 
		    (TL_TS_Cor *) calloc (tl_table[k].num_ts_regions, 
					  sizeof (TL_TS_Cor))) == NULL)
		{
		    CALLOC_ERR ("tl_table[].ts_cor");
		    fclose (tl_fp);
		    return (TLreadErr7);
		}

		/*
		 * Read test-site corrections themselves
		 */

		for (i = 0; i < tl_table[k].num_ts_regions; i++)
		{
		    tsc = &tl_table[k].tl_ts_cor[i];
		    if ((fscanf (ts_fp, "%d%s%s%d%*[^\n]", 
				&tsc->reg_num, tsc->reg_name_id,
				tsc->TLtype, &tsc->num_sta)) != 4)
		    {
			fprintf (stderr, "Error encountered while attempting to read mag. test-site corr. file:\n  %s\n\n", file_buf);
			UFREE (tl_table[k].tl_ts_cor);
			tl_table[k].tl_ts_cor = (TL_TS_Cor *) NULL;
			fclose (ts_fp);
			return (TLreadErr6);
		    }
		    tsc->sta = UALLOC (char *, tsc->num_sta);
		    tsc->ts_corr = UALLOC (double, tsc->num_sta);
		    for (j = 0; j < tsc->num_sta; j++)
		    {
			tsc->sta[j] = UALLOC (char, 7);
			if (fscanf (ts_fp, "%6s %lf", tsc->sta[j], &tsc->ts_corr[j]) != 2) return (TLreadErr6);
			if ((strchr (tsc->sta[j], '*')) != NULL)
			{
			    sta_len = strlen (tsc->sta[j]);
			    strcpy (&tsc->sta[j][sta_len-1], "\0");
			}
		    }
		}
		fclose (ts_fp);
	    }
	}

	*tl_table_ptr = &tl_table[0];

	return (OK);
} 

