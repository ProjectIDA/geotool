
/*
 * Copyright (c) 1994-1996 Science Applications International Corporation.
 *

 * NAME
 *	read_tt_tables -- Read travel-time tables
 
 * FILE
 *	read_tt_tables.c

 * SYNOPSIS
 *	int
 *	read_tt_tables (only_read_default_tt_tables, vmodel_filename, 
 *			phase_list, num_phases, tt_table_ptr, 
 *			total_num_phases, model_descrip_ptr, num_md, 
 *			sta_phase_model_ptr, num_spm)
 *	Bool	only_read_default_tt_tables;
 *					(i) Only read default set of travel-
 *					    time tables (i.e., ignore 
 *					    station/phase-dependent info)
 *	char	*vmodel_filename;	(i) Velocity model file name location
 *	char	**phase_list;		(i) Pointer to list of phases
 *	int	num_phases;		(i) Number of phases in above list
 *	TT_Table **tt_table_ptr;	(o) Pointer to T-T table structure
 *	int	*total_num_phases;	(o) Total number of all phases
 *					    (length of tt_table_ptr structure)
 *	Model_Descrip **model_descrip_ptr; 
 *					(o) Pointer to model description info
 *	int	*num_md;		(o) Number of velocity models (length
 *					    of model_descrip_ptr structure)
 *	Sta_Phase_Model **sta_phase_model_ptr; 
 *					(o) Pointer to station-specific info
 *	int	*num_spm;		(o) Number of station-specific phase/
 *					    velocity models (length of 
 *					    sta_phase_model_ptr structure)

 * DESCRIPTION
 *	Function.  Read travel-time tables as based on the input velocity
 *	model specification file (vmodel_filename) and the input list of 
 *	phase types.  Create travel-time table structure, tt_table, model
 *	description structure, model_descrip, and station-specific phase/
 *	model handling structure, sta_phase_model, by reading from a the
 *	velocity model specification file and sets of travel-time tables 
 *	denoted by this file.  If only default travel time tables are desired
 *	(i.e., only_read_default_tt_tables = TRUE), then ignore station/phase-
 *	dependent information.

 * DIAGNOSTICS
 *	Return argument will inform user about the following errors 
 *	encountered in this routine (see include file, loc_defs.h, for
 *	error code define statements):
 *	    TTerror2:	No travel-time tables could be found.
 *	    TTerror3:	Bogus input travel-time table format encountered.
 *	    TTerror4:	Error allocating memory.
 *	    TTerror5:	Cannot open velocity model specification file.
 *	    LPerror1:	Error encountered while trying to read LP tables.
 *          HYerror1:   Error encountered while trying to read radial_2D station table.
 *          HYerror2:   Bad parameter passed to radial_2D function.
 *          HYerror3:   Could not find a radial_2D station file.
 *          HYerror4:   Could not allocate memory while in a radial_2D function.
 *	    ECerror1:	Error encountered while reading ellip. corr. table.

 * FILES
 *	Read travel-time, modelling error and ellipticity correction tables.

 * NOTES
 *	If input file will not open, then the structure, tt_table, will be 
 *	set to NULL.  The input list of phases is only used for the default
 *	set of travel-time tables, namely, model_descrip[0].
 
 * SEE ALSO
 *	Calling routine, setup_tttables(). 

 * AUTHOR
 *	Walter Nagy, 5/25/94,	Created.
 *	Walter Nagy, 9/8/95,	Now phase-dependent ellipticity corrections 
 *				are directly attached to travel-time tables 
 *				using tt_table structure (tt_table.h).
 *	Walter Nagy, 9/20/95,	Added T-T modelling error functionality by 
 *				including these variances with travel-time 
 *				tables via new function, get_model_error().
 *	Walter Nagy, 10/27/95,	Added special handling of default SPM info as
 *				represented by phase designators, *P and *S.
 *	Walter Nagy,  5/ 7/96,	Added argument, only_read_default_tt_tables,
 *				to ignore station/phase-dependent information
 *				if this is desired.
 */


#include "config.h"
#include <stdio.h>
#include <string.h>
#include "locp.h"
#include "loc_defs.h"
#include "libLP.h"
#include "tt_info.h"

/* Error report */
#define READ_ERR(a)	{ fprintf (stderr, "\nread_tt_tables: Error reading %s in file: %s\n", (a), file_name); \
			}
#define CALLOC_ERR(a)	{ fprintf (stderr, "\nread_tt_tables: Error allocating space for %s, in file: %s\n", (a), file_name); \
			}

#define	VALID_TIME(x)	((x) > -1.0)

/*
 * vmodel_filename:	Velocity model file name location
 * phase_list:		Pointer to list of phases
 * num_phases:		Number of phases in above list
 * tt_table_ptr:	Pointer to T-T table structure
 * total_num_phases:	Total # of phases in tt_table_ptr
 * model_descrip_ptr:	Pointer to model description struct
 * num_md:		length of model_descrip_ptr
 * sta_phase_model_ptr: Pointer to station-specific phase model info structure
 * num_spm:		length of sta_phase_model_ptr
 */
int
read_tt_tables (Bool only_read_default_tt_tables, char *vmodel_filename, 
		char **phase_list, int num_phases, TT_Table **tt_table_ptr, 
		int *total_num_phases, Model_Descrip **model_descrip_ptr, 
		int *num_md, Sta_Phase_Model **sta_phase_model_ptr, 
		int *num_spm)
{
	FILE	*tfp, *tmp_fp, *vm_fp;
	Bool	model_found;
	Bool	phase_found;
	Bool	ok_so_far;
	Bool	separate_ec_tables_exist = FALSE;
	Bool	phase_already_exists_in_list = FALSE;
	int	i, j, k, n;	
	int	status;
	int	ntbz;			/* Number of depth samples to read    */
	int	ntbd;			/* Number of distance samples to read */
	int	open_cnt, num_files;	/* File counters		      */
	int	num_models = 0;
	int	phase_index;
	int	num_sta_phase_models = 0;
	char	*ffp;
	char	file_name[FILENAMELEN];
	char	file_buf[FILENAMELEN];
	char	dir_ptr[FILENAMELEN];
	char	el_prefix[FILENAMELEN];
	char	dir_pathway[FILENAMELEN];
	char	relative_pathway[FILENAMELEN];
	char	vmodel_name[16], vm1[16], vm2[16];
	char	input_string[BUFSIZ];
	Bool	use_2D_tables;

	TT_Table	*tt_table = (TT_Table *) NULL;
	EC_Table	*ec_table_ptr = (EC_Table *) NULL;
	Model_Descrip	*model_descrip = (Model_Descrip *) NULL;
	Sta_Phase_Model	*sta_phase_model = (Sta_Phase_Model *) NULL;
	List_of_Phases	*ph, *ph_anch, *ph_end, *ph_tst, *prev;

	static	char	routine[] = "read_tt_tables";


	tt_table = *tt_table_ptr;
	model_descrip = *model_descrip_ptr;
	sta_phase_model = *sta_phase_model_ptr;

	/*
	 * Open velocity model once here to determine how much space 
	 * needs to be allocated up front (namely, number of velocity 
	 * models (num_models) and number of station/phase/model-dependent
	 * entries (num_sta_phase_models) that exist.  Then close file, 
	 * re-open, and do actual filling of structures.  A blank line
	 * should follow the velocity model name and relative directory
	 * pathway information.  A blank space or '#' in the first column
	 * can be used as well.
	 */

	if ((vm_fp = fopen (vmodel_filename, "r")) == NULL)
	{
	    fprintf (stderr, "\n%s: VMSF: %s will not open!\n", 
			routine, vmodel_filename);
	    return (TTerror5);
	}

	while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (!strncmp (input_string, "#", 1)) 
		continue;
	    else
		break;
	}

 
/* Skip a second line, the radial_2D path*/
        while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
        {
            /* Skip lines w/ '#' in 1st column */
            if (!strncmp (input_string, "#", 1))
                continue;
            else
                break;
        }


	while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		++num_models;		/* Count number of velocity models */
	    }
	}

	/*
	 * If only default travel-time tables are specified, then ignore
	 * and and all station/phase-dependent information (i.e., SPM).
	 * In such a case, num_models = 1 and num_sta_phase_models = 0, by
	 * definition.
	 */

	if (only_read_default_tt_tables)
	{
	    num_models = 1;
	    num_sta_phase_models = 0;
	}
	else
	{
	    while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
	    {
		/* Skip lines w/ '#' in 1st column */
		if (!strncmp (input_string, "#", 1) ||
		    !strncmp (input_string, "\n", 1)) 
		    continue;
		else
		    ++num_sta_phase_models;	/* Count lines of sta-specific info */ 
	    }
	}
	fclose (vm_fp);

	/*
	 * Re-open velocity model specification file!  Now we desire to do
	 * actual reading of velocity model information including all station/
	 * phase/model-dependent knowledge.  This latter knowledge take the
	 * form of pointer to station/phase 1-D travel-time table location,
	 * the sedimentary velocity (sed_vel) assigned as well as the bulk
	 * station corr (bulk_sta_corr).
	 */

	if ((vm_fp = fopen (vmodel_filename, "r")) == NULL)
	{
	    fprintf (stderr, "\n%s: VMSF: %s will not open!\n", 
			routine, vmodel_filename);
	    return (TTerror5);
	}

	while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
	{
	    /* Skip lines w/ '#' in 1st column */
	    if (!strncmp (input_string, "#", 1)) 
		continue;
	    else
	    {
		sscanf (input_string, "%s", dir_ptr);
		break;
	    }
	}

	/*
	 * Read long-period (LP) path-dependent travel-time tables for 
	 * LR and LQ phases.
	 */

	strcpy (dir_pathway, vmodel_filename);
	if ((ffp = strrchr (dir_pathway, '/')) == NULL)
	{
	    fclose (vm_fp);
	    fprintf (stderr, "%s: No / in LP directory name!\n", routine);
	    return (LPerror1);
	}
	strcpy (ffp+1, dir_ptr);

	if ((read_LP_info (dir_pathway)) != OK)
	{
	    fclose (vm_fp);
	    fprintf (stderr, "%s: Problems encountered while trying to read LP tables!\n", routine);
	    return (LPerror1);
	}


 
 
while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
        {
            /* Skip lines w/ '#' in 1st column */
            if (!strncmp (input_string, "#", 1))
                continue;
            else
            {
                sscanf (input_string, "%s", dir_ptr);
                break;
            }
        }
 
        /*
         * Read path-dependent radial_2D travel-time tables for
         * hydro-acoustic phases. 
         */

	use_2D_tables = FALSE;
	for(i=0; i<num_phases; i++)
	    if ( STREQ(phase_list[i],"H") || STREQ(phase_list[i],"O") 
	        || STREQ(phase_list[i],"T") || STREQ(phase_list[i],"I") ) 
		use_2D_tables = TRUE;
		
        if(use_2D_tables && STREQ(dir_ptr,"NONE"))
	    fprintf(stderr,"NONE specified as radial_2D file path.  Default travel times will be used.\n");
        else if(use_2D_tables)
        {
                strcpy (dir_pathway, vmodel_filename);
                if ((ffp = strrchr (dir_pathway, '/')) == NULL)
                {
                        fclose (vm_fp);
                        fprintf (stderr, "%s: No / in radial_2D directory name!\n", routine);
                        return (HYerror1);
                }
                strcpy (ffp+1, dir_ptr);
                if ((read_HY_info (dir_pathway)) != OK)
                {
                        fclose (vm_fp);
                        fprintf (stderr, "%s: Problems encountered while trying to read radial_2D tables!\n", routine);
                        return (HYerror1);
                }
        }
 


	/*
	 * Allocate storage space for model_descrip and sta_phase_model
	 * structures.
	 */

	if ((model_descrip = (Model_Descrip *) 
			calloc (num_models, sizeof (Model_Descrip))) == NULL)
	{
	    fclose (vm_fp);
	    CALLOC_ERR ("structure, model_descrip");
	    return (TTerror4);
	}
	if (num_sta_phase_models > 0)
	{
	    if ((sta_phase_model = (Sta_Phase_Model *) 
	       calloc (num_sta_phase_models, sizeof (Sta_Phase_Model))) == NULL)
	    {
		fclose (vm_fp);
		CALLOC_ERR ("structure, sta_phase_model");
		return (TTerror4);
	    }
	}
	else
	    sta_phase_model = (Sta_Phase_Model *) NULL;

	/*
	 * Read default velocity model information first.  The input list
	 * of phases will be attached to this model.  Further, all ellipticity
	 * corrections will be relative, by definition, to this set of travel-
	 * time tables.  NOTE: The last fgets() call here will have read 
	 * the first line after all the velocity models have been specified. 
	 */

	i = 0;
	while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
	{
	    if (strncmp (input_string, "#", 1)) 
	    {
		if (!strncmp (input_string, " ", 1) || 
		    !strncmp (input_string, "\n", 1))
		    break;
		sscanf (input_string, "%s%s", model_descrip[i].vmodel,
				  relative_pathway);
		strcpy (ffp+1, "");
		strcpy (file_buf, dir_pathway);
		strcpy (dir_ptr, file_buf);
		strcat (dir_ptr, relative_pathway);
		model_descrip[i].dir_pathway = STRALLOC (dir_ptr);
		model_descrip[i].list_of_phases = (List_of_Phases *) NULL;
		model_descrip[i].sssc_dir_exists = FALSE;
		/*
		printf ("vmodel: %s  pathway: %s\n", model_descrip[i].vmodel,
					model_descrip[i].dir_pathway);
		 */
		++i;
	    }
	    if (i == 1 && only_read_default_tt_tables)
		break;
	}

	/*
	 * Next, read station/phase/model-dependent information!  First,
	 * skip lines with a '#' in the first column.  Find velocity index 
	 * (sta_phase_model[].vel_index) associated with specified velocity
	 * model.  If none is found, return with an error code.
	 * f
	 */

	if (num_sta_phase_models > 0)
	{
	    j = 0;
	    while (fgets (input_string, BUFSIZ, vm_fp) != NULL)
	    {
		/* Skip lines w/ '#' or blank line ('\n') in 1st column */
		if (strncmp (input_string, "#", 1) &&
		    strncmp (input_string, "\n", 1)) 
		{
		    if (sscanf (input_string, "%s%s%s%f%f%s", /* new style of vmsf - additional column with sssc file suffix (sorce region) */
			    sta_phase_model[j].sta, sta_phase_model[j].phase, 
			    vmodel_name, &sta_phase_model[j].sed_vel,
			    &sta_phase_model[j].bulk_sta_corr, sta_phase_model[j].source_region) != 6)
		    {
		      if (sscanf (input_string, "%s%s%s%f%f", 
			    sta_phase_model[j].sta, sta_phase_model[j].phase, 
			    vmodel_name, &sta_phase_model[j].sed_vel,
			    &sta_phase_model[j].bulk_sta_corr) == 5)
		      {
		      	strcpy(sta_phase_model[j].source_region, "*"); /* no source_region in vmsf */
		      }
		      else
		      {
			fclose (vm_fp);
			READ_ERR("station/phase/model info");
			return (TTerror3);
		      }
		    }
		    else
		    {
		    	if (!strncmp (sta_phase_model[j].source_region, "#", 1))
			  strcpy(sta_phase_model[j].source_region, "*"); /* comment in vmsf */
		    }
		    
		    sta_phase_model[j].sssc = (Sssc *) NULL;
		    for (model_found = FALSE, i = 0; i < num_models; i++)
		    {
			if (STREQ (vmodel_name, model_descrip[i].vmodel))
			{
			    model_found = TRUE;
			    sta_phase_model[j].vel_index = i;
			    break;
			}
		    }
		    if (!model_found)
		    {
			fclose (vm_fp);
			fprintf (stderr, "%s: Velocity model not specified for: %s in station/phase/model listing\n", routine, vmodel_name);
			return (TTerror3);
		    }
		    j++;
		}
	    }
	}
	fclose (vm_fp);

	/*
	 * Open file pointer to ellipticity correction directory, if one
	 * exists.  If not, simply return FALSE (i.e., no ellipticity
	 * correction tables exist).
	 */

	strcpy (el_prefix, model_descrip[0].dir_pathway);
	strcat (el_prefix, "/");
	strcat (el_prefix, model_descrip[0].vmodel);
	strcat (el_prefix, ".elcor_dir");
	if ((tmp_fp = fopen (el_prefix, "r")) == NULL)
	{
	    fprintf (stdout, "No ellipticity correction tables exist for given T-T tables! Will use defaults!\n");
	}
	else
	{
	    /*
	     * Back-track to find current directory location
	     */

	    if ((ffp = strrchr (el_prefix, '/')) == NULL)
	    {
		fclose (tmp_fp);
		fprintf (stderr, "%s: No / in ellipticity correction directory name!\n", routine);
		return (ECerror1);
	    }

	    /*
	     * Now extract ellipticity correction directory and prefix info
	     */

	    if (fscanf (tmp_fp, " %s ", dir_ptr) != 1) return(ECerror1);
	    strcat (dir_ptr, ".");
	    strcpy (ffp+1, dir_ptr);
	    fclose (tmp_fp);
	    separate_ec_tables_exist = TRUE;
	}

	/*
	 * Fill default table (vel_index = 0) with complete list of 
	 * input phases (phase_list).
	 */

	if ((model_descrip[0].list_of_phases = (List_of_Phases *) 
				calloc (1, sizeof (List_of_Phases))) == NULL)
	{
	    CALLOC_ERR ("structure, list_of_phases");
	    return (TTerror4);
	}
	ph_anch = model_descrip[0].list_of_phases;
	model_descrip[0].list_of_phases->phase_index = 0;
	model_descrip[0].list_of_phases->next = (List_of_Phases *) NULL;
	strcpy (model_descrip[0].list_of_phases->phase, phase_list[0]);
	prev = ph_anch;
	for (i = 1; i < num_phases; i++)
	{
	    if ((ph = (List_of_Phases *) 
				calloc (1, sizeof (List_of_Phases))) == NULL)
	    {
		    CALLOC_ERR ("structure, list_of_phases");
		    return (TTerror4);
	    }
	    ph->phase_index = i;
	    ph->next = (List_of_Phases *) NULL;
	    strcpy (ph->phase, phase_list[i]);
	    prev->next = ph;
	    prev = prev->next;
	}

	/*
	 * For remaining velocity models, add to list_of_phases only those
	 * phases belonging to given model.  Do not attempt to read T-T
	 * tables for station/phase/models with a default P (*P) or S (*S)
	 * phase designator.
	 */

	for (i = 0; i < num_sta_phase_models; i++)
	{
	    j = sta_phase_model[i].vel_index;
	    ph_anch = model_descrip[j].list_of_phases;
	    if (STREQ (sta_phase_model[i].phase, "*P") ||
		STREQ (sta_phase_model[i].phase, "*S"))
		continue;	/* Just skip over this general case */
	    else if (j == 0)
	    {
		/*
		 * Make sure phase type exists for SPM info given default
		 * (j = 0) model!  If not, complain with a warning message
		 * and skip over this phase type!
		 */

		for (phase_found = FALSE, k = 0; k < num_phases; k++)
		{
		    if (STREQ (sta_phase_model[i].phase, phase_list[k]))
		    {
			phase_found = TRUE;
			break;
		    }
		}
		if (!phase_found)
		{
/* we don't want warning messages any more (must be solve in the future) !!! */
/*
		    fprintf (stderr, "\nWarning: Phase %s missing from phase list for default %s model\n", sta_phase_model[i].phase, model_descrip[j].vmodel);
		    fprintf (stderr, "         SPM info for Sta/Phz/Model: %s/%s/%s will be ignored\n", sta_phase_model[i].sta, sta_phase_model[i].phase, model_descrip[j].vmodel);
*/		    
		    continue;
		}
	    }

	    if (ph_anch == (List_of_Phases *) NULL)
	    {
		if ((model_descrip[j].list_of_phases = (List_of_Phases *) 
				calloc (1, sizeof (List_of_Phases))) == NULL)
		{
		    CALLOC_ERR ("structure, list_of_phases");
			return (TTerror4);
		}
		ph_anch = model_descrip[j].list_of_phases;
		model_descrip[j].list_of_phases->next = (List_of_Phases *) NULL;
		strcpy (model_descrip[j].list_of_phases->phase, 
			sta_phase_model[i].phase);
	    }
	    else
	    {
		/*
		 * Does this phase already exist in list_of_phases for given
		 * velocity model?  If so, move onto next sta_phase_model[]
		 * sample.  If this is a new phase, then add phase to list.
		 */

		ph = ph_anch;
		phase_already_exists_in_list = FALSE;
		while (ph != (List_of_Phases *) NULL)
		{
		    if (STREQ (ph->phase, sta_phase_model[i].phase))
		    {
			phase_already_exists_in_list = TRUE;
			break;
		    }
		    ph = ph->next;
		}
		if (!phase_already_exists_in_list)
		{
		    if ((ph = (List_of_Phases *) 
				calloc (1, sizeof (List_of_Phases))) == NULL)
		    {
			CALLOC_ERR ("structure, list_of_phases");
			return (TTerror4);
		    }
		    ph->next = (List_of_Phases *) NULL;
		    strcpy (ph->phase, sta_phase_model[i].phase);
		    ph_end = ph_anch;
		    while (ph_end != (List_of_Phases *) NULL)
		    {
			if (ph_end->next == (List_of_Phases *) NULL)
			{
			    ph_end->next = ph;
			    break;
			}
			ph_end = ph_end->next;
		    }
		}
	    }
	}

	/*
	 * Now set phase indexes (model_descrip[].list_of_phases->phase_index)
	 * such that each phase/model has its own index.  This is ensured
	 * now by the above loop which file list_of_phases for velocity models
	 * (vel_index) 1 thru num_models-1.  Note our loop here starts at
	 * 1, since we already filled the default velocity model phase_index
	 * field (vel_index = 0).  Our first phase index, is therefore,
	 * phase_index = num_phases, from input.
	 */

	phase_index = num_phases;
	for (j = 1; j < num_models; j++)
	{
	    ph = model_descrip[j].list_of_phases;
	    while (ph != (List_of_Phases *) NULL)
	    {
		ph->phase_index = phase_index;
		++phase_index;
		ph = ph->next;
	    }
	}
	*total_num_phases = phase_index;

	/*
	 * Now fill phase_index field of sta_phase_model structure for phases
	 * inlcuded in list_of_phases for varying models.
	 */

	for (i = 0; i < num_sta_phase_models; i++)
	{
	    sta_phase_model[i].phase_index = -1;
	    ph = model_descrip[sta_phase_model[i].vel_index].list_of_phases;
	    while (ph != (List_of_Phases *) NULL)
	    {
		if (STREQ (sta_phase_model[i].phase, ph->phase))
		{
		    sta_phase_model[i].phase_index = ph->phase_index;
		    break;
		}
		ph = ph->next;
	    }
	}

	/*
	 * Allocate tt_table structure based on number of input phases.
	 */

	if ((tt_table = (TT_Table *) calloc (*total_num_phases,
						sizeof (TT_Table))) == NULL)
	{
	    CALLOC_ERR ("structure, tt_table");
	    return (TTerror4);
	}

	/* Initialize error code and counters */

	open_cnt = 0; 
	num_files = 0;

	/* Read the files */

	for (n = 0; n < num_models; n++)
	{
	    ph = model_descrip[n].list_of_phases;
	    while (ph != (List_of_Phases *) NULL)
	    {
		k = ph->phase_index;
		strcpy (file_name, model_descrip[n].dir_pathway);
		strcat (file_name, "/");
		strcat (file_name, model_descrip[n].vmodel);
		strcat (file_name, ".");
		strcat (file_name, ph->phase);

		/* Initialize tt_table structure for this phase */

		strcpy (tt_table[k].phase, ph->phase);
		strcpy (tt_table[k].vmodel, model_descrip[n].vmodel);
		tt_table[k].num_dists = 0;
		tt_table[k].num_depths = 0;
		tt_table[k].in_hole_dist[0] = 181.0;
		tt_table[k].in_hole_dist[1] = -1.0;
		tt_table[k].dist_samples = (float *) NULL;
		tt_table[k].depth_samples = (float *) NULL;
		tt_table[k].trv_time = (float **) NULL;
		tt_table[k].ec_table = (EC_Table *) NULL;
		tt_table[k].model_error = (Model_Error *) NULL;

		/*
		 * Is the last leg of given phase of type P or S ?
		 */

		if ((tt_table[k].last_leg = last_leg (ph->phase)) < OK)
		    fprintf (stdout, "Warning: Last leg of phase: %s has no definition in function, last_leg()!\n", ph->phase);

		/* Open travel-time file */
 
		if ((tfp = fopen (file_name, "r")) == NULL)
		{
		    fprintf (stderr, "\nFile %s will not open!\n", file_name);
		    ph = ph->next;
		    continue;
		}
		++open_cnt;	/* Successful opening of 1 file */

		/* Begin reading info -- Start with velocity model */

		if (fscanf (tfp, "%s%s%*[^\n]", vm1, vm2) != 2)
		{
		    READ_ERR("1st line of travel-time table");
		    fclose (tfp);
		    return (TTerror3);
		}
		if (STREQ (vm1, "#"))
		    strcpy (tt_table[k].vmodel, vm2);
		else
		    strcpy (tt_table[k].vmodel, vm1);

		/* Read depth sampling */
 
		if (fscanf (tfp, "%d%*[^\n]", &ntbz) != 1)
		{
		    READ_ERR("number of depth samples");
		    fclose (tfp);
		    return (TTerror3);
		}
		tt_table[k].num_depths = ntbz;

		if ((tt_table[k].depth_samples = 
			(float *) calloc (ntbz, sizeof (float))) == NULL)
		{
		    CALLOC_ERR ("tt_table[].depth_samples");
		    fclose (tfp);
		    return (TTerror4);
		}

		for (i = 0; i < ntbz; i++)
		{
		    if (fscanf (tfp, "%f", &tt_table[k].depth_samples[i]) != 1)
		    {
			READ_ERR("depth sample value");
			fclose (tfp);
			return (TTerror3);
		    }
		}
 
		/* Read distance sampling */
 
		if (fscanf (tfp, "%d%*[^\n]", &ntbd) != 1)
		{
		    READ_ERR("number of distance samples");
		    fclose (tfp);
		    return (TTerror3);
		}
		tt_table[k].num_dists = ntbd;

		if ((tt_table[k].dist_samples = 
			(float *) calloc (ntbd, sizeof (float))) == NULL)
		{
		    CALLOC_ERR ("tt_table[].dist_samples");
		    fclose (tfp);
		    return (TTerror4);
		}

		for (i = 0; i < ntbd; i++)
		{
		    if (fscanf (tfp, "%f", &tt_table[k].dist_samples[i]) != 1)
		    {
			READ_ERR("distance sample value");
			fclose (tfp);
			return (TTerror3);
		    }
		}

		/* Read travel-time tables */
 
		if ((tt_table[k].trv_time = 
			(float **) calloc (ntbz, sizeof (float *))) == NULL)
		{
		    CALLOC_ERR ("tt_table[].trv_time");
		    fclose (tfp);
		    return (TTerror4);
		}
		for (i = 0; i < ntbz; i++)
		{
		    if ((tt_table[k].trv_time[i] = 
			(float *) calloc (ntbd, sizeof (float))) == NULL)
		    {
			CALLOC_ERR ("tt_table[].trv_time[]");
			fclose (tfp);
			return (TTerror4);
		    }
		}

		for (j = 0; j < ntbz; j++)
		{
		    /* skip the comment line */
		    while (getc(tfp) != '#');
		    while (getc(tfp) != '\n');
		    for (i = 0; i < ntbd; i++)
		    {
			if (fscanf (tfp,"%f", &tt_table[k].trv_time[j][i]) != 1)
			{
			    READ_ERR("travel-time value");
			    fclose (tfp);
			    return (TTerror3);
			}
		    }
		}

		/*
		 * Look for hole in travel-time table.  If one is discovered 
		 * then set min/max distances.  Only inspect 1st depth node.
		 * Only meaningful if both in_hole_dist[0] and in_hole_dist[1]
		 * are set here.
		 */

		for (i = 1, ok_so_far = TRUE; i < ntbd; i++)
		{
		    if (VALID_TIME(tt_table[k].trv_time[0][i-1]) && 
		        ! VALID_TIME(tt_table[k].trv_time[0][i]))
		    {
			tt_table[k].in_hole_dist[0] = 
						tt_table[k].dist_samples[i-1];
			ok_so_far = FALSE;
		    }
		    else if (!ok_so_far && 
			     VALID_TIME(tt_table[k].trv_time[0][i]))
		    {
			tt_table[k].in_hole_dist[1] = 
						tt_table[k].dist_samples[i];
			break;
		    }
		}

		/*
		 * Next read data modelling information!  This can take the
		 * form of a single-value, a distance-dependent set of values
		 * and distance/depth-dependent values.  Each is optimized
		 * for fastest access.  The first thing we need to do is
		 * verify the existence of such information within the 
		 * travel-time tables.  If this is an old-style travel-time
		 * table, backwards compatiability is maintained by
		 * determining whether or not the EOF has been reached. 
		 * If so, a NULL pointer will be set for the model_error
		 * structure hanging off of the tt_table structure.  In
		 * this case, a default modelling error of 1.0 sec. will
		 * be employed.  If a new-style travel-time table exists,
		 * it will contain a comment line instead of an EOF.
		 * A comment line must exist betwen distance and/or
		 * depth sample specification and modelling errors 
		 * themselves.  Additional comment lines exist between
		 * separate sets of modelling errors for different depths
		 * (note that this latter case is only valid when
		 *  dist_depth_var needs to be filled.  i.e., when ntbd > 0)
		 */

		if (fscanf (tfp, "%s%*[^\n]", vm1) == EOF)
		    goto eof_found;
		if (fscanf (tfp, "%d%d%*[^\n]", &ntbd, &ntbz) == EOF)
		    goto eof_found;

		if ((tt_table[k].model_error = (Model_Error *) 
				calloc (1, sizeof (Model_Error))) == NULL)
		{
		    CALLOC_ERR ("tt_table[].model_error");
		    fclose (tfp);
		    return (TTerror4);
		}

		tt_table[k].model_error->bulk_var       = 1.0;
		tt_table[k].model_error->num_dists      = ntbd;
		tt_table[k].model_error->num_depths     = ntbz;
		tt_table[k].model_error->dist_samples   = (float *) NULL;
		tt_table[k].model_error->depth_samples  = (float *) NULL;
		tt_table[k].model_error->dist_var       = (float *) NULL;
		tt_table[k].model_error->dist_depth_var = (float **) NULL;

		if (ntbz == 1)
		{
		    if (ntbd == 1)
		    {
			/*
			 * Only a single modelling error is available, so
			 * we only need to fill variable, bulk_var.  Don't
			 * forget to skip comment line!
			 */

			if (fscanf (tfp, "%s%*[^\n]", vm1) == EOF)
			{
			    READ_ERR("premature EOF");
			    fclose (tfp);
			    return (TTerror3);
			}
			if (fscanf (tfp, "%f%*[^\n]", 
				   &tt_table[k].model_error->bulk_var) != 1)
			{
			    READ_ERR("single bulk modelling error");
			    fclose (tfp);
			    return (TTerror3);
			}
		    }
		    else
		    {
			/*
			 * Only distance-dependent modelling errors exist,
			 * so allocate only enough memory for single-
			 * dimensioned arrays, dist_samples && dist_var_wgt.
			 */

			if ((tt_table[k].model_error->dist_samples = 
				(float *) calloc(ntbd, sizeof (float))) == NULL)
			{
			    CALLOC_ERR ("tt_table[].model_error->dist_samples");
			    fclose (tfp);
			    return (TTerror4);
			}
			if ((tt_table[k].model_error->dist_var = 
				(float *) calloc(ntbd, sizeof (float))) == NULL)
			{
			    CALLOC_ERR ("tt_table[].model_error->dist_var");
			    fclose (tfp);
			    return (TTerror4);
			}

			/*
			 * Read distance-dependent variance information.
			 */

			for (i = 0; i < ntbd; i++)
			{
			    if (fscanf (tfp, "%f", &tt_table[k].model_error->dist_samples[i]) == EOF)
			    {
				READ_ERR("modelling error distance sample value");
				fclose (tfp);
				return (TTerror3);
			    }
			}
			if (fscanf (tfp, "%s%*[^\n]", vm1) == EOF)
			{
			    READ_ERR("premature EOF");
			    fclose (tfp);
			    return (TTerror3);
			}
			for (i = 0; i < ntbd; i++)
			{
			    if (fscanf (tfp, "%f", &tt_table[k].model_error->dist_var[i]) == EOF)
			    {
				READ_ERR("distance-dependent modelling error");
				fclose (tfp);
				return (TTerror3);
			    }
			}
		    }
		}
		else
		{
		    /*
		     * Modelling errors are distance and depth dependent
		     * here.  Allocate memory for dist_samples,
		     * depth_samples and dist_depth_var.  Note, you
		     * don't need to set array element, dist_var, in
		     * this case.
		     */

		    if ((tt_table[k].model_error->dist_samples = 
			    (float *) calloc (ntbd, sizeof (float))) == NULL)
		    {
			CALLOC_ERR ("tt_table[].model_error->dist_samples");
			fclose (tfp);
			return (TTerror4);
		    }
		    if ((tt_table[k].model_error->depth_samples = 
			    (float *) calloc (ntbz, sizeof (float))) == NULL)
		    {
			CALLOC_ERR ("tt_table[].model_error->depth_samples");
			fclose (tfp);
			return (TTerror4);
		    }
		    if ((tt_table[k].model_error->dist_depth_var = 
			    (float **) calloc (ntbz, sizeof (float *))) == NULL)
		    {
			CALLOC_ERR ("tt_table[].model_error->dist_depth_var");
			fclose (tfp);
			return (TTerror4);
		    }
		    for (i = 0; i < ntbz; i++)
		    {
			if ((tt_table[k].model_error->dist_depth_var[i] = 
				(float *) calloc (ntbd, sizeof (float))) == NULL)
			{
			    CALLOC_ERR ("tt_table[].model_error->dist_depth_var[]");
			    fclose (tfp);
			    return (TTerror4);
			}
		    }

		    /*
		     * Read distance and depth sampling info 
		     */

		    for (i = 0; i < ntbd; i++)
		    {
			if (fscanf (tfp, "%f", &tt_table[k].model_error->dist_samples[i]) == EOF)
			{
			    READ_ERR("modelling error distance sample value");
			    fclose (tfp);
			    return (TTerror3);
			}
		    }
		    for (i = 0; i < ntbz; i++)
		    {
			if (fscanf (tfp, "%f", &tt_table[k].model_error->depth_samples[i]) == EOF)
			{
			    READ_ERR("modelling error depth sample value");
			    fclose (tfp);
			    return (TTerror3);
			}
		    }

		    /*
		     * Finally, read the actual distance/depth-dependent
		     * modelling errors (variances).
		     */

		    for (j = 0; j < ntbz; j++)
		    {
			/* skip the comment line */
			while (getc(tfp) != '#');
			while (getc(tfp) != '\n');
			for (i = 0; i < ntbd; i++)
			{
			    if (fscanf (tfp,"%f", &tt_table[k].model_error->dist_depth_var[j][i]) == EOF)
			    {
				READ_ERR("distance/depth modelling error");
				fclose (tfp);
				return (TTerror3);
			    }
			}
		    }
		}

eof_found:
		++num_files;
		fclose (tfp);

		/*
		 * If separate distance/depth-dependent ellipticity 
		 * corrections exist, then read each file one phase 
		 * at-a-time for "default" model only.  Other models
		 * will point to these tables.
		 */

		if (separate_ec_tables_exist)
		{
		    if (n == 0)
		    {
			strcpy (file_name, el_prefix);
			strcat (file_name, tt_table[k].phase);

			/*
			 * Does ellipticity correction file exist for 
			 * given phase?  If so, a return message of 0 (OK) 
			 * will be come back.  If return value (status) is 
			 * negative, then a fatal error was encountered.
			 */

			status = read_ec_table (file_name, &ec_table_ptr);
			if (status == OK)
			    tt_table[k].ec_table = ec_table_ptr;
			else if (status < OK)
	    		    return (ECerror1);
		    }
		    else
		    {
			/*
			 * Link with appropriate EC_Table pointer from default
			 * velocity model.  If none found, then simply use
			 * default NULL pointer (already set).
			 */

			ph_tst = model_descrip[0].list_of_phases;
			while (ph_tst != (List_of_Phases *) NULL)
			{
			    if (STREQ (ph->phase, ph_tst->phase))
				tt_table[k].ec_table =
					tt_table[ph_tst->phase_index].ec_table;
			    ph_tst = ph_tst->next;
			}
		    }
		}

		ph = ph->next;
	    }
	}

	*tt_table_ptr = &tt_table[0];
	*model_descrip_ptr = &model_descrip[0];
	*sta_phase_model_ptr = &sta_phase_model[0];

	*num_spm = num_sta_phase_models;
	*num_md = num_models;

	if (num_files == 0 && open_cnt == 0)	/* No files could be opened */
	{
	    fprintf (stderr, "%s: No tables could be opened!\n", routine);
	    return (TTerror2);
	}

	return (OK);
} 

